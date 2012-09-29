//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#ifndef DEBUG_ZONES_H
#define DEBUG_ZONES_H

//#include <dbgapi.h>
//#pragma warning(disable: 4091)
extern DBGPARAM dpCurSettings;

// REVIEW: should we establish consistent debug zone across all our drivers? -> If yes, move to <SolutionRoot>/GL/Log

//namespace GlobalLocate
//{
//namespace GPSCtl
//{
//namespace Log
//{

// Note: for really crippled (aka old) compilers either use extern const int declaration
//       and provide definition in .cpp (costs memory though) or replace with #define,
//       e.g. #define ZONEID_INIT 0

// Definitions for our "debug zones", which map to the log facility
// Since we're basically just routing output from our general log facility to WinCE debug log
// the definitions are different from what would normally be expected, in particular 
// the "standard" WARN, FUNCTION and ERROR zones don't really fit.
#define ZONEID_INIT        0
#define ZONEID_OPEN        1
#define ZONEID_IOCTL       2
#define ZONEID_INFO        3
#define ZONEID_RSVD4       4
#define ZONEID_RSVD5       5
#define ZONEID_RSVD6       6
#define ZONEID_RSVD7       7
#define ZONEID_RSVD8       8
#define ZONEID_RSVD9       9
#define ZONEID_RSVD10      10
#define ZONEID_RSVD11      11
#define ZONEID_RSVD12      12
#define ZONEID_FUNCTION    13
#define ZONEID_WARN        14
#define ZONEID_ERROR       15

// These masks are useful for initialization of dpCurSettings
#define ZONEMASK_INIT        (1 << ZONEID_INIT )
#define ZONEMASK_OPEN        (1 << ZONEID_OPEN )
#define ZONEMASK_IOCTL       (1 << ZONEID_IOCTL) 
#define ZONEMASK_INFO        (1 << ZONEID_INFO )
#define ZONEMASK_RSVD4       (1 << ZONEID_RSVD4) 
#define ZONEMASK_RSVD5       (1 << ZONEID_RSVD5) 
#define ZONEMASK_RSVD6       (1 << ZONEID_RSVD6) 
#define ZONEMASK_RSVD7       (1 << ZONEID_RSVD7) 
#define ZONEMASK_RSVD8       (1 << ZONEID_RSVD8) 
#define ZONEMASK_RSVD9       (1 << ZONEID_RSVD9) 
#define ZONEMASK_RSVD10      (1 << ZONEID_RSVD10)
#define ZONEMASK_RSVD11      (1 << ZONEID_RSVD11)
#define ZONEMASK_RSVD12      (1 << ZONEID_RSVD12)
#define ZONEMASK_FUNCTION    (1 << ZONEID_FUNCTION) 
#define ZONEMASK_WARN        (1 << ZONEID_WARN ) 
#define ZONEMASK_ERROR       (1 << ZONEID_ERROR) 

#ifdef DEBUG
    // These macros are used as the first arg to DEUBGMSG
#define ZONE_INIT             DEBUGZONE(ZONEID_INIT) 
#define ZONE_OPEN             DEBUGZONE(ZONEID_OPEN) 
#define ZONE_IOCTL            DEBUGZONE(ZONEID_IOCTL)
#define ZONE_INFO             DEBUGZONE(ZONEID_INFO) 
#define ZONE_RSVD4            DEBUGZONE(ZONEID_RSVD4)
#define ZONE_RSVD5            DEBUGZONE(ZONEID_RSVD5)
#define ZONE_RSVD6            DEBUGZONE(ZONEID_RSVD6)
#define ZONE_RSVD7            DEBUGZONE(ZONEID_RSVD7)
#define ZONE_RSVD8            DEBUGZONE(ZONEID_RSVD8)
#define ZONE_RSVD9            DEBUGZONE(ZONEID_RSVD9)
#define ZONE_RSVD10           DEBUGZONE(ZONEID_RSVD10)
#define ZONE_RSVD11           DEBUGZONE(ZONEID_RSVD11)
#define ZONE_RSVD12           DEBUGZONE(ZONEID_RSVD12)
#define ZONE_FUNCTION         DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN             DEBUGZONE(ZONEID_WARN)
//#define ZONE_ERROR                 DEBUGZONE(ZONEID_ERROR)
/*#else  
    // For RETAIL builds, these conditionals are always 0
    //const int ZONE_INIT      = 0;
    //const int ZONE_OPEN        = 0;
    //const int ZONE_IOCTL       = 0;
    //const int ZONE_INFO      = 0;
    //const int ZONE_RSVD4   = 0;
    //const int ZONE_RSVD5   = 0;
    //const int ZONE_RSVD6   = 0;
    //const int ZONE_RSVD7     = 0;
    //const int ZONE_RSVD8   = 0;
    //const int ZONE_RSVD9   = 0;
    //const int ZONE_RSVD10      = 0;
    //const int ZONE_RSVD11      = 0;
    //const int ZONE_RSVD12      = 0;
    //const int ZONE_FUNCTION  = 0;
    //const int ZONE_WARN          = 0;
    //const int ZONE_ERROR   = 0;
    const int ZONE_INIT      = DEBUGZONE(ZONEID_INIT);
    const int ZONE_OPEN      = DEBUGZONE(ZONEID_OPEN);
    const int ZONE_IOCTL     = DEBUGZONE(ZONEID_IOCTL);
    const int ZONE_INFO        = DEBUGZONE(ZONEID_INFO);
    const int ZONE_RSVD4         = DEBUGZONE(ZONEID_RSVD4);
    const int ZONE_RSVD5         = DEBUGZONE(ZONEID_RSVD5);
    const int ZONE_RSVD6         = DEBUGZONE(ZONEID_RSVD6);
    const int ZONE_RSVD7         = DEBUGZONE(ZONEID_RSVD7);
    const int ZONE_RSVD8         = DEBUGZONE(ZONEID_RSVD8);
    const int ZONE_RSVD9         = DEBUGZONE(ZONEID_RSVD9);
    const int ZONE_RSVD10    = DEBUGZONE(ZONEID_RSVD10);
    const int ZONE_RSVD11    = DEBUGZONE(ZONEID_RSVD11);
    const int ZONE_RSVD12    = DEBUGZONE(ZONEID_RSVD12);
    const int ZONE_FUNCTION      = DEBUGZONE(ZONEID_FUNCTION);
    const int ZONE_WARN      = DEBUGZONE(ZONEID_WARN);
    const int ZONE_ERROR         = DEBUGZONE(ZONEID_ERROR);*/
#endif //DEBUG

//} // end namespace Log
//} // end namespace GPSCtl
//} // end namespace GlobalLocate

#endif // DEBUG_ZONES_H

