//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
/*
 *    Copyright (C) 2004, Motorola Inc. All Rights Reserved
 *
 * File:            rtc.h
 * Purpose:         Definitions for RTC module
 *
 */

 
#ifndef _INC_RTC_H
#define _INC_RTC_H

/*********************************************************************
 INCLUDE FILES  
*********************************************************************/


/*********************************************************************
 MACRO DEFINITIONS 
*********************************************************************/

//These macro define the settings of RTC control register
#define RTC_CTL_RTC_EN            (1 << 7)
#define RTC_CTL_CLK32768HZ        (0 << 5)
#define RTC_CTL_CLK32000HZ        (1 << 5)
#define RTC_CTL_CLK38400HZ        (2 << 5)
#define RTC_CTL_SW_RESET          (1 << 0)

//These macro define the settings of RTC Interrupt Enable register
#define RTC_INT_EN_SAMPLE7        0x8000
#define RTC_INT_EN_SAMPLE6        0x4000
#define RTC_INT_EN_SAMPLE5        0x2000
#define RTC_INT_EN_SAMPLE4        0x1000
#define RTC_INT_EN_SAMPLE3        0x0800
#define RTC_INT_EN_SAMPLE2        0x0400
#define RTC_INT_EN_SAMPLE1        0x0200
#define RTC_INT_EN_SAMPLE0        0x0100
#define RTC_INT_EN_2HZ            0x0080
#define RTC_INT_EN_HOUR           0x0020
#define RTC_INT_EN_SECOND         0x0010
#define RTC_INT_EN_DAY            0x0008
#define RTC_INT_EN_ALARM          0x0004
#define RTC_INT_EN_MINUTE         0x0002
#define RTC_INT_EN_STOPWATCH      0x0001

//These macro define the masks of RTC Interrupt Status register
#define RTC_INT_STAT_SAMPLE7      0x8000
#define RTC_INT_STAT_SAMPLE6      0x4000
#define RTC_INT_STAT_SAMPLE5      0x2000
#define RTC_INT_STAT_SAMPLE4      0x1000
#define RTC_INT_STAT_SAMPLE3      0x0800
#define RTC_INT_STAT_SAMPLE2      0x0400
#define RTC_INT_STAT_SAMPLE1      0x0200
#define RTC_INT_STAT_SAMPLE0      0x0100
#define RTC_INT_STAT_2HZ          0x0080
#define RTC_INT_STAT_HOUR         0x0020
#define RTC_INT_STAT_SECOND       0x0010
#define RTC_INT_STAT_DAY          0x0008
#define RTC_INT_STAT_ALARM        0x0004
#define RTC_INT_STAT_MINUTE       0x0002
#define RTC_INT_STAT_STOPWATCH    0x0001


// These macro define the RTC Days, Hours, Minutes and 
// Seconds register bits for both counter and alarm registers
#define RTC_SECOND_MASK        0x0000003FL
#define RTC_SECOND_OFFSET        0

#define RTC_MINUTE_MASK        0x0000003FL
#define RTC_MINUTE_OFFSET        0

#define RTC_HOUR_MASK          0x00001F00L
#define RTC_HOUR_OFFSET          8

#define RTC_DAY_MASK           0x0000FFFFL
#define RTC_DAY_OFFSET           0


//These macro define some default information of RTC
#define ORIGINYEAR       1980                  // the begin year
#define MAXYEAR          (ORIGINYEAR + 100)    // the maxium year
#define JAN1WEEK         2                     // Jan 1 1980 is a Tuesday


/*********************************************************************
 ENUMERATIONS AND STRUCTURES 
*********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

typedef struct RTCRegisters{
    /* 0x10007000: Real Time Clock */
    REG32    RTCHMCnt;        /* 00: RTC Hours & Minutes Counter Register  HOURMIN*/
    REG32    RTCSecCnt;       /* 04: RTC Seconds Counter Register          SECONDS*/
    REG32    RTCHMAlarm;      /* 08: RTC Hours & Minutes Alarm Register    ALRM_HM*/
    REG32    RTCSecAlarm;     /* 0C: RTC Seconds Alarm Register           ALRM_SEC*/
    REG32    RTCControl;      /* 10: RTC Control Register                    RCCTL*/
    REG32    RTCIntStatus;    /* 14: RTC Interrupt Status Register          RTCISR*/
    REG32    RTCIntEnable;    /* 18: RTC Interrupt Enable Register         RTCIENR*/
    REG32    RTCStopWatch;    /* 1C: Stopwatch Minutes Register             STPWCH*/
    REG32    RTCDayCnt;       /* 20: RTC Days Counter Register                DAYR*/
    REG32    RTCDayAlarm;     /* 24: RTC Day Alarm Register               DAYALARM*/
} RTCRegisters_t, *pRTCRegisters_t;

/*********************************************************************
 FUNCTION PROTOTYPES
*********************************************************************/


/*********************************************************************
 EXTERN DECLARATIONS
*********************************************************************/
#ifdef __cplusplus
}
#endif

/*********************************************************************
 CLASS DEFINITIONS
*********************************************************************/

#endif    /* _INC_RTC_H */

/*********************************************************************
 END OF FILE
*********************************************************************/
