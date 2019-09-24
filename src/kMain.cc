//===================================================================================================================
//
//  kMain.cc -- The main entry point and all tasks
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2019-Sep-21  Initial   0.0.0   ADCL  Initial version
//
//===================================================================================================================


#include "video.h"
#include "scheduler.h"


//
// -- the kMain prototype
//    -------------------
extern "C" void kMain(void);


//
// -- Some simple global variables
//    ----------------------------
PCB_t *A;
PCB_t *B;
PCB_t *C;


//
// -- This is process "B" which wll output the letter "B"
//    ---------------------------------------------------
void ProcessB(void)
{
    while (true) {
        WriteChar('B');
        SwitchToTask(C);
    }
}


//
// -- This is process "C" which wll output the letter "C"
//    ---------------------------------------------------
void ProcessC(void)
{
    while (true) {
        WriteChar('C');
        SwitchToTask(A);
    }
}


//
// -- This is the main entry point after fundamental initialization
//    -------------------------------------------------------------
void kMain(void) 
{
    VideoInit();
    WriteStr("Welcome\n");
    InitScheduler();
    A = currentPCB;

    B = CreateProcess(ProcessB);
    C = CreateProcess(ProcessC);

    // -- loop forever rather than return
    while(true) {
        WriteChar('A');
        SwitchToTask(B);
    }
}