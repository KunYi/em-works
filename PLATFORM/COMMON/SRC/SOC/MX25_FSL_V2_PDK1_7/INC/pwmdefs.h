//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pwmdefs.h
//
//  Definitions for Pulse Width Modulator Driver
//
//------------------------------------------------------------------------------
#ifndef __PWMDEFS_H__
#define __PWMDEFS_H__

#include "pm.h"
//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT             0
#define ZONEID_DEINIT           1
#define ZONEID_IOCTL            2

#define ZONEID_INFO            12
#define ZONEID_FUNCTION        13
#define ZONEID_WARN            14
#define ZONEID_ERROR           15

// Debug zone masks
#define ZONEMASK_INIT         (1<<ZONEID_INIT)
#define ZONEMASK_DEINIT     (1<<ZONEID_DEINIT)
#define ZONEMASK_IOCTL       (1<<ZONEID_IOCTL)

#define ZONEMASK_INFO         (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN        (1<<ZONEID_WARN)
#define ZONEMASK_ERROR        (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT                   DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT               DEBUGZONE(ZONEID_DEINIT)
#define ZONE_IOCTL                 DEBUGZONE(ZONEID_IOCTL)

#define ZONE_INFO                  DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION          DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN                 DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR                DEBUGZONE(ZONEID_ERROR)

#endif


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
// IOCTL to enable reset PWM
#define PWM_IOCTL_RESET                 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1000, METHOD_BUFFERED, FILE_ANY_ACCESS)


/* struct of pwm play sample */
typedef struct {
   UINT32  sample;				// pwm sample value
   UINT32  period;					// pwm period
   UINT32  duration;				// duration of the sample (msec)
   UINT32  pwmctrl;
} PwmSample_t, *pPwmSample_t;


//
// CS&ZHL AUG-16-2011: input parameters for PWM_Write(..)
//
typedef struct {
   DWORD	dwFreq;				// PWM freq in Hz, = 0: stop PWM output
   DWORD	dwDuty;				// PWM duty in %, = 0: stop PWM output
   DWORD	dwDuration;			// duration of PWM output (msec), = 0: -> infinit
   DWORD	dwPwmctrl;			// PWM Control register 
								// dwPwmctrl[1:0]:PWM Output Configuration. This bit field determines the mode of PWM output on the output pin.
								//	   00 Output pin is set at rollover and cleared at comparison
								//	   01 Output pin is cleared at rollover and set at comparison
} PWMINFO, *PPWMINFO;


//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------
DWORD PwmInitialize(LPCTSTR pContext);
void PwmDeinit(PVOID pDeviceContext);
void PwmStart(PVOID pDeviceContext);
void PwmStop(PVOID pDeviceContext);
void PwmReset(PVOID pDeviceContext);
DWORD	 PwmInfo2PwmSample(PPWMINFO pInfo, pPwmSample_t pSample);	// CS&ZHL AUG-17-2011: return = PreScaler
BOOL PwmPlaySample(PVOID pDeviceContext, LPCVOID SampleBuffer, int dwNumWord );
BOOL PwmReadRegister(PVOID pDeviceContext, UINT32* readBuf, DWORD Count);
void SetPwmPower(PPOWER_CAPABILITIES power);

#endif   // __PWMDEFS_H__
