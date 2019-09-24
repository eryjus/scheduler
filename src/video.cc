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

