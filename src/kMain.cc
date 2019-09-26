//===================================================================================================================
//
//  kMain.cc -- The main entry point and all tasks
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2019-Sep-21  Initial   0.0.0   ADCL  Initial version
//  2019-Sep-23  Step 1   step01   ADCL  Complete Step 1 of the tutorial
//  2019-Sep-25  Step 2   step02   ADCL  Add `Schedule()` and update the processes
//
//===================================================================================================================


#include "video.h"
#include "scheduler.h"


//
// -- the kMain prototype
//    -------------------
extern "C" void kMain(void);


char pch = 'A';


//
// -- This is a generic process which will output a letter accordingly
//    ----------------------------------------------------------------
void Process(void)
{
    char ch = pch ++;
    while (true) {
        WriteChar(ch);
        Schedule();
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

    // -- each will not get their "letter" until they start running
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);
    CreateProcess(Process);

    // -- Now, we call process ourselves
    Process();
}