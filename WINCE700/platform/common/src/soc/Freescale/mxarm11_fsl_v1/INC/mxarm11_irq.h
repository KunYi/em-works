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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mxarm11_irq.h
//
//  This file defines names for IRQ. These names have no other role than make
//  code more readable. For SoC where device - IRQ mapping is defined by
//  silicon and it can't be changed by software.
//
//  This header contains common IRQ definitions for SoC designs based on the
//  Freescale ARM11 chassis.
//------------------------------------------------------------------------------
#ifndef __MXARM11_IRQ_H
#define __MXARM11_IRQ_H

//------------------------------------------------------------------------------
// ARM11 CHASSIS IRQ DEFINITIONS
//------------------------------------------------------------------------------
#define IRQ_SDHC2               8
#define IRQ_SDHC1               9
#define IRQ_I2C                 10
#define IRQ_SSI2                11
#define IRQ_SSI1                12
#define IRQ_CSPI2               13
#define IRQ_CSPI1               14
#define IRQ_SIM_COMMON          20
#define IRQ_SIM_DATA            21
#define IRQ_RNGA                22
#define IRQ_KPP                 24
#define IRQ_RTC                 25
#define IRQ_PWM                 26
#define IRQ_EPIT2               27
#define IRQ_EPIT1               28
#define IRQ_GPT                 29
#define IRQ_NANDFC              33
#define IRQ_SDMA                34
#define IRQ_NONE                64


//------------------------------------------------------------------------------

#endif // __MXARM11_IRQ_H
