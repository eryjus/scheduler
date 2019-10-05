//===================================================================================================================
//
//  video.cc -- Video buffer manipulation
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2019-Sep-21  Initial   0.0.0   ADCL  Initial version
//  2019-Sep-24  Pull#1   step01   ADCL  Correct an issue with the buffer overrunning
//  2019-Oct-05  Step 3   step03   ADCL  Add the `WriteDecimal()` function
//
//===================================================================================================================


#include "video.h"


//
// -- These are the variables for the video
//    -------------------------------------
int row;
int col;
int attr;
unsigned char *buffer = (unsigned char *)0xb8000;



//
// -- video initialization
//    --------------------
void VideoInit(void)
{
    row = 0;
    col = 0;
    attr = 0x07;
}


//
// -- Write a character to the video
//    ------------------------------
void WriteChar(int ch)
{
    if (row >= 25) return;

    if (ch == '\n') {
newline:
        row ++;
        col = 0;
        return;
    }

    if (ch == '\r') {
        col = 0;
        return;
    }

    buffer[(row * 80 + col) * 2] = (unsigned char)ch;
    buffer[(row * 80 + col) * 2 + 1] = attr;

    col ++;

    if (col == 80) goto newline;
}


//
// -- Write a string to the video
//    ---------------------------
void WriteStr(const char *s)
{
    while (*s) {
        WriteChar(*s ++);
    }
}


//
// -- Worker function to write a decimal value
//    ----------------------------------------
static inline void WriteDec(unsigned long val)
{
    if (val == 0) return;
    else {
        WriteDec(val / 10);
        WriteChar('0' + (val % 10));
    }
}


//
// -- Write a decimal value on the screen
//    -----------------------------------
void WriteDecimal(const unsigned long val)
{
    if (val == 0) WriteChar('0');
    else WriteDec(val);
}

