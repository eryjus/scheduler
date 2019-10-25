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
//
//===================================================================================================================


#include "cpu.h"
#include "scheduler.h"


//
// -- PUSH a value on the stack for a new process
//    -------------------------------------------
#define PUSH(tos,val) (*(-- tos) = val)


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

    AddReady(rv);

    return rv;
}


//
// -- based on what is currently running (roun-robin style), select the next task
//    ---------------------------------------------------------------------------
void Schedule(void) 
{
    PCB_t *next = NextReady();

    if (next) {
        AddReady(currentPCB);
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


