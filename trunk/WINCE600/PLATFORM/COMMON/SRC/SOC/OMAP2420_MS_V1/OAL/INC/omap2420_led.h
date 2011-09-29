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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  led.h
//
#ifndef __OMAP2420_LED_H
#define __OMAP2420_LED_H

 //------------------------------------------------------------------------------
//
//  Define:  LED_IDX_xxx
//
//  Following constants are used to specify OALLED indexes for different
//  information displayed on debug LED drivers.
//
#ifndef SHIP_BUILD
#define LED_IDX_TIMERSPIN       0
#define LED_IDX_IDLE            1
#define LED_IDX_SLEEPSTAT       2
#define LED_IDX_CLKREQ1         3
#define LED_IDX_CLKREQ2         4
#define LED_IDX_SOFTREQ         5
#define LED_IDX_DBBSTAT         6
#define LED_IDX_ITSTAT          7
#define LED_IDX_IDLECT1         8
#define LED_IDX_IDLECT2         9
#define LED_IDX_IDLECT3         10
#define LED_IDX_SOFT_DIS_REQ    11
#endif // SHIP_BUILD

//------------------------------------------------------------------------------

#endif // __OMAP2420_LED_H
