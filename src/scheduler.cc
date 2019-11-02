//===================================================================================================================
//
//  scheduler.cc -- These are the scheduler functions
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2019-Sep-22  Initial  step01   ADCL  Initial version
//  2019-Sep-25  Step 2   step02   ADCL  Add `Schedule()` and create a circular linked list
//  2019-Oct-05  Step 3   step03   ADCL  Add timer-related functions to the scheduler
//  2019-Oct-05  Step 4   step04   ADCL  Updates for process states
//  2019-Oct-24  Step 5   step05   ADCL  Add rudamentary scheduler lock
//  2019-Oct-25  Step 6   step06   ADCL  Add the ability to block/unblock
//  2019-Nov-01  Step 8   step08   ADCL  Add checks for postponed task changes
//
//===================================================================================================================


#include "cpu.h"
#include "scheduler.h"


//
// -- PUSH a value on the stack for a new process
//    -------------------------------------------
#define PUSH(tos,val) (*(-- tos) = val)


//
// -- The number of times IRQs have been disbled
//    ------------------------------------------
static int irqDisableCounter = 0;

//
// -- The number of times locks that would postpone task swaps have been obtained
//    ---------------------------------------------------------------------------
int postponeTaskCounter = 0;


//
// -- Has a task change been postponed?
//    ---------------------------------
bool taskChangePostponedFlag = false;


//
// -- the implementation of the running task
//    --------------------------------------
PCB_t *currentPCB;


//
// -- The list of ready to run processes
//    ----------------------------------
PCB_t *readyListHead;
PCB_t *readyListTail;


//
// -- This is the array of possible running tasks
//    -------------------------------------------
PCB_t pcbArray[MAX_TASKS];


//
// -- This is the last counter value read
//    -----------------------------------
unsigned long lastCounter;


//
// -- initialize the current task block
//    ---------------------------------
void InitScheduler(void) 
{
    for (int i = 0; i < MAX_TASKS; i ++) pcbArray[i].used = 0;

    pcbArray[0].used = 1;
    pcbArray[0].tos = 0;
    pcbArray[0].virtAddr = GetCR3();
    pcbArray[0].state = RUNNING;
    pcbArray[0].next = (PCB_t *)0;

    currentPCB = &pcbArray[0];

    readyListHead = (PCB_t *)0;
    readyListTail = (PCB_t *)0;

    lastCounter = GetCurrentCounter();
}


//
// -- allocate a PCB and return its address to the calling function
//    -------------------------------------------------------------
PCB_t *PcbAlloc(void)
{
    for (int i = 0; i < MAX_TASKS; i ++) {
        if (pcbArray[i].used == 0) {
            pcbArray[i].used = 1;
            pcbArray[i].tos = ((0x181 + i) << 12);  // also allocate a stack here

            return &pcbArray[i];
        }
    }

    return (PCB_t *)0;
}


//
// -- Free a PCB back to the pool
//    ---------------------------
void PcbFree(PCB_t *pcb)
{
    if (pcb < &pcbArray[0] || pcb >= &pcbArray[MAX_TASKS]) return;
    pcb->used = 0;
}


//
// -- Process startup function which will be called before the actual entry point
//    ---------------------------------------------------------------------------
static void ProcessStartup(void) 
{
    UnlockScheduler();
}


//
// -- Add a process to the list of ready tasks
//    ----------------------------------------
void AddReady(PCB_t *task)
{
    if (!task) return;

    task->state = READY;

    if (readyListHead == (PCB_t *)0) {
        readyListHead = readyListTail = task;
    } else {
        readyListTail->next = task;
        readyListTail = task;
    }
}


//
// -- Get the next ready task, removing it from the list
//    WARNING: Might return NULL!!!
//    --------------------------------------------------
PCB_t *NextReady(void)
{
    if (readyListHead == (PCB_t *)0) return (PCB_t *)0;

    PCB_t *rv = readyListHead;
    readyListHead = readyListHead->next;

    if (readyListHead == (PCB_t *)0) readyListTail = readyListHead;

    return rv;
}


//
// -- Create a new process with its entry point
//    -----------------------------------------
PCB_t *CreateProcess(void (*ent)())
{
    PCB_t *rv = PcbAlloc();

    if (!rv) return rv;

    unsigned int *tos = (unsigned int *)rv->tos;
    PUSH(tos, (unsigned int)ProcessStartup);  // startup function
    PUSH(tos, (unsigned int)ent);        // entry point
    PUSH(tos, 0);           // EBP
    PUSH(tos, 0);           // EDI
    PUSH(tos, 0);           // ESI
    PUSH(tos, 0);           // EBX

    rv->tos = (unsigned int)tos;
    rv->virtAddr = GetCR3();

    LockScheduler();
    AddReady(rv);
    UnlockScheduler();

    return rv;
}


//
// -- based on what is currently running (roun-robin style), select the next task
//
//    !!!! The caller must have called LockScheduler() and must call !!!!
//    !!!! UnlockScheduler() before and after the call to Schedule() !!!!
//    ---------------------------------------------------------------------------
void Schedule(void) 
{
    if (postponeTaskCounter != 0) {
        taskChangePostponedFlag = true;
        return;
    }

    PCB_t *next = NextReady();

    if (next) {
        SwitchToTask(next);
    }
}


//
// -- Update the current process with the amount of time used
//    -------------------------------------------------------
void UpdateTimeUsed(void)
{
    unsigned long c = lastCounter;
    lastCounter = GetCurrentCounter();
    currentPCB->used += (lastCounter - c);
}


//
// -- Lock the scheduler
//    ------------------
void LockScheduler(void) 
{
    CLI();
    irqDisableCounter ++;
}


//
// -- Unlock the scheduler
//    --------------------
void UnlockScheduler(void)
{
    irqDisableCounter --;
    if (irqDisableCounter == 0) STI();
}


//
// -- Obtain a lock (global for now) and postpone task changes
//    --------------------------------------------------------
void LockAndPostpone(void) 
{
    CLI();
    irqDisableCounter ++;
    postponeTaskCounter ++;
}


//
// -- Unlock a lock (global for now) and perform a Schedule if we can
//    ---------------------------------------------------------------
void UnlockAndSchedule(void)
{
    postponeTaskCounter --;
    if (postponeTaskCounter == 0) {
        if (taskChangePostponedFlag) {
            taskChangePostponedFlag = false;
            Schedule();
        }
    }

    irqDisableCounter --;
    if (irqDisableCounter == 0) STI();
}



//
// -- Block the current process
//    -------------------------
void BlockProcess(int reason)
{
    LockScheduler();
    currentPCB->state = reason;
    Schedule();
    UnlockScheduler();
}


//
// -- Unblock a Process
//    -----------------
void UnblockProcess(PCB_t *proc) 
{
    LockScheduler();

    if (readyListHead == (PCB_t *)0) {
        SwitchToTask(proc);
    } else {
        AddReady(proc);
    }

    UnlockScheduler();
}


