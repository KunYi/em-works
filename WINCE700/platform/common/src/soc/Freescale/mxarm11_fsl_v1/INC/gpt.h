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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  gpt.h
//
//  Public definitions for General Purpose Timer Driver
//
//------------------------------------------------------------------------------
#ifndef __GPT_H__
#define __GPT_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

// IOCTL to create GPT timer event handle
#define GPT_IOCTL_TIMER_CREATE_EVENT     1
// IOCTL to close GPT timer event handle
#define GPT_IOCTL_TIMER_RELEASE_EVENT     2
// IOCTL to start the GPT timer
#define GPT_IOCTL_TIMER_START        3
// IOCTL to stop the GPT timer
#define GPT_IOCTL_TIMER_STOP         4
// IOCTL to set the delay between
// interrupts for the GPT timer in ms
#define GPT_IOCTL_TIMER_UPDATE_PERIOD 5
// To obtain GPT main counter value
#define GPT_IOCTL_TIMER_GET_COUNT    6
// To resume(enable)the GPT timer from stop
#define GPT_IOCTL_TIMER_RESUME       7
#define GPT_IOCTL_TIMER_CHANGE_CLKSRC 8
#define GPT_IOCTL_TIMER_SHOW_CLKSRC 9
//------------------------------------------------------------------------------
// Types

// GPT timer mode
typedef enum timerMode{
    timerModePeriodic,
    timerModeFreeRunning
} timerMode_c;

// GPT timer source
typedef enum timerSrc{
    GPT_NOCLK,
    GPT_IPGCLK,
    GPT_HIGHCLK,
    GPT_EXTCLK,
    GPT_32KCLK
} timerSrc_c;

// GPT set timer mode and period 
typedef struct
{
    timerMode_c timerMode;
    UINT32 period;
} GPT_Config, *pGPT_Config;

// GPT timer source packet
typedef struct
{
    timerSrc_c timerSrc;
} GPT_TIMER_SRC_PKT, *PGPT_TIMER_SRC_PKT;
//------------------------------------------------------------------------------
// Functions

HANDLE GptOpenHandle(void);
BOOL GptCloseHandle(HANDLE);
HANDLE GptCreateTimerEvent(HANDLE, LPTSTR);
BOOL  GptReleaseTimerEvent(HANDLE, LPTSTR);
BOOL  GptStart(HANDLE, pGPT_Config);
BOOL  GptResume(HANDLE);
BOOL  GptStop(HANDLE);
BOOL  GptUpdatePeriod(HANDLE,DWORD);
BOOL  GptGetCounterValue(HANDLE,PDWORD);
BOOL  GptShowTimerSrc(HANDLE hGpt);
BOOL  GptSetTimerSrc(HANDLE hGpt, PGPT_TIMER_SRC_PKT pGptTimerSrcPkt);
#ifdef __cplusplus
}
#endif

#endif   // __GPT_H__