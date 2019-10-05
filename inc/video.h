//===================================================================================================================
//
//  video.h -- Video buffer manipulation prototypes
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2019-Sep-21  Initial   0.0.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __VIDEO_H__
#define __VIDEO_H__


extern "C" 
{
    void VideoInit(void);
    void WriteChar(int ch);
    void WriteStr(const char *s);
    void WriteDecimal(const unsigned long val);
}

#endif


