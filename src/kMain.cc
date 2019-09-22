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


extern "C" void kMain(void);


//
// -- This is the main entry point after fundamental initialization
//    -------------------------------------------------------------
void kMain(void) 
{
    VideoInit();
    WriteStr("Welcome\n");

    // -- loop forever rather than return
    while(true) {}
}