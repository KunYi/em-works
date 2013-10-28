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
//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//  File:  gpt_priv.h
//
//  Private definitions for General Purpose Timer Driver
//
//------------------------------------------------------------------------------

#ifndef __GPT_PRIV_H__
#define __GPT_PRIV_H__

#include "gpt.h"

//------------------------------------------------------------------------------
// Defines

#define GPT_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define GPT_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT       0
#define ZONEID_DEINIT     1
#define ZONEID_IOCTL      2

#define ZONEID_INFO       12
#define ZONEID_FUNCTION   13
#define ZONEID_WARN       14
#define ZONEID_ERROR      15

// Debug zone masks
#define ZONEMASK_INIT     (1<<ZONEID_INIT)
#define ZONEMASK_DEINIT   (1<<ZONEID_DEINIT)
#define ZONEMASK_IOCTL    (1<<ZONEID_IOCTL)

#define ZONEMASK_INFO     (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN     (1<<ZONEID_WARN)
#define ZONEMASK_ERROR    (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT         DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT       DEBUGZONE(ZONEID_DEINIT)
#define ZONE_IOCTL        DEBUGZONE(ZONEID_IOCTL)

#define ZONE_INFO         DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION     DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN         DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR        DEBUGZONE(ZONEID_ERROR)
#endif


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Functions

BOOL GptInitialize(void);
void GptRelease(void);
BOOL GptTimerCreateEvent(LPTSTR);
BOOL GptTimerReleaseEvent(LPTSTR);
BOOL GptEnableTimer(void);
void GptDisableTimer(void);
void GptEnableTimerInterrupt(void);
void GptDisableTimerInterrupt(void);
void GptClearInterruptStatus(void);
void GptRegInit(void);
BOOL GptResetTimer(void);
BOOL GptGetTimerCount(PDWORD pGptCntValue);
BOOL GptUpdateTimerPeriod(DWORD Period);
BOOL GptSetTimerMode(timerMode_c TimerMode);
void GptShowClkSrc(void);
void GptChangeClkSrc(PGPT_TIMER_SRC_PKT pSetSrcPkt);
void GptRegChangeClkSrc(void);

#endif   // __GPT_PRIV_H__
