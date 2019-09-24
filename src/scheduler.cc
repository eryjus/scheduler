//===================================================================================================================
//
//  scheduler.cc -- These are the scheduler functions
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2019-Sep-22  Initial  step01   ADCL  Initial version
//
//===================================================================================================================


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
// -- This is the array of possible running tasks
//    -------------------------------------------
PCB_t pcbArray[MAX_TASKS];

//
// -- initialize the current task block
//    ---------------------------------
void InitScheduler(void) 
{
    for (int i = 0; i < MAX_TASKS; i ++) pcbArray[i].used = 0;

    pcbArray[0].used = 1;
    pcbArray[0].tos = 0;
    pcbArray[0].virtAddr = GetCR3();
    pcbArray[0].state = 0;
    pcbArray[0].next = (PCB_t *)0;

    currentPCB = &pcbArray[0];
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

    return rv;
}
