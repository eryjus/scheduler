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
//  2019-Sep-29  Step 3   step03   ADCL  Add the timer initialization into the code
//  2019-Oct-05  Step 4   step04   ADCL  Add a process state -- sanity check
//  2019-Oct-24  Step 5   step05   ADCL  Add rudamentary scheduler lock
//  2019-Oct-25  Step 6   step06   ADCL  Add the ability to block/unblock
//  2019-Nov-05  Step 9   step09   ADCL  Add sleeping to the process repetoire
//  2019-Nov-09  Step10   step10   ADCL  Add idle CPU handling 
//  2019-Nov-10  Step11   step11   ADCL  Add preemption
//
//===================================================================================================================


#include "cpu.h"
#include "video.h"
#include "scheduler.h"


//
// -- the kMain prototype
//    -------------------
extern "C" void kMain(void);


//
// -- Current letter to print
//    -----------------------
char pch = 'A';


//
// -- This is a generic process which will output a letter accordingly
//    ----------------------------------------------------------------
void Process(void)
{
    char ch = pch ++;
    while (true) {
        WriteChar(ch);
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
    InitTimer();
    STI();

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

    Process();
}