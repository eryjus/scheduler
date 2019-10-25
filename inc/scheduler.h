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
//  2019-Oct-05  Step 3   step03   ADCL  Add a call to `UpdateTimeUsed()`
//
//===================================================================================================================


#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__


typedef enum {
    RUNNING = 0,
    READY = 1,
} ProcessState_t;



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
// -- This is the head/tail list of ready to run processes
//    ----------------------------------------------------
extern PCB_t *readyListHead;
extern PCB_t *readyListTail;


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

    void AddReady(PCB_t *task);
    PCB_t *NextReady(void);
}

#endif


