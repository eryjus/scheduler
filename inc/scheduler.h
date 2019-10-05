//===================================================================================================================
//
//  scheduler.h -- Strutures and prototypes for takng care of task scheduling
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2019-Sep-21  Initial  step01   ADCL  Initial version
//  2019-Sep-25  Step 2   step02   ADCL  Add `Schedule()`
//
//===================================================================================================================


#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__


//
// -- This is the Process Control Block -- for controlling processes by the scheduler
//    -------------------------------------------------------------------------------
typedef struct PCB_t {
    unsigned int tos;
    unsigned int virtAddr;
    struct PCB_t *next;
    int state;
    int used;
} PCB_t;


//
// -- We need to keep track of the current task
//    -----------------------------------------
extern PCB_t *currentPCB;


//
// -- an array of PCBs that can be used
//    ---------------------------------
#define MAX_TASKS       256
extern PCB_t pcbArray[MAX_TASKS];


//
// -- Funtion prototypes
//    ------------------
extern "C" 
{
    void SwitchToTask(PCB_t *task) __attribute__((cdecl));
    unsigned int GetCR3(void);

    void InitScheduler(void);
    PCB_t *CreateProcess(void (*ent)());

    void Schedule(void);

    void UpdateTimeUsed(void);
}

#endif


