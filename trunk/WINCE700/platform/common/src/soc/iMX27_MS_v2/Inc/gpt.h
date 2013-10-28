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
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
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

// GPT number ranges from 1 to 5

#define GPT_MINIMUM_NUMBER 1
#define GPT_MAXIMUM_NUMBER 5

// IOCTL to create GPT timer event handle
#define GPT_IOCTL_TIMER_CREATE_EVENT     1
// IOCTL to close GPT timer event handle
#define GPT_IOCTL_TIMER_RELEASE_EVENT     2
// IOCTL to enable GPT interrupts
#define GPT_IOCTL_INT_ENABLE         3
// IOCTL to disable GPT interrupts
#define GPT_IOCTL_INT_DISABLE        4
// IOCTL to start the GPT timer
#define GPT_IOCTL_TIMER_START        5
// IOCTL to stop the GPT timer
#define GPT_IOCTL_TIMER_STOP         6
// IOCTL to set the delay between
// interrupts for the GPT timer
#define GPT_IOCTL_TIMER_SET_DELAY    7


//------------------------------------------------------------------------------
// Types

// GPT timer mode
typedef enum timerMode{
    timerModePeriodic,
    timerModeFreeRunning
} timerMode_c;

// GPT timer period type
typedef enum timerPeriodType {
    SECOND,
    MILLISEC,
    MICROSEC
} timerPeriodType_c;

// GPT set timer delay packet
typedef struct
{
    timerMode_c timerMode;
    UINT32 period;
    timerPeriodType_c periodType;
} GPT_TIMER_SET_PKT, *PGPT_TIMER_SET_PKT;

//------------------------------------------------------------------------------
// Functions

HANDLE GptOpenHandle(UINT8);
BOOL GptCloseHandle(HANDLE);
HANDLE GptCreateTimerEvent(HANDLE, LPTSTR);
BOOL  GptReleaseTimerEvent(HANDLE, LPTSTR);
BOOL  GptSetTimer(HANDLE, PGPT_TIMER_SET_PKT);
BOOL  GptStart(HANDLE);
BOOL  GptStop(HANDLE);

#ifdef __cplusplus
}
#endif

#endif   // __GPT_H__
