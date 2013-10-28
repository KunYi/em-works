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
//  File:  MX31_irq.h
//
//  This file defines names for IRQ. These names have no other role than make
//  code more readable. For SoC where device - IRQ mapping is defined by
//  silicon and it can't be changed by software.
//
//  This header contains IRQ definitions for the MX31 SoC that are not
//  included as part of the Freescale ARM11 chassis implementation.
//
//------------------------------------------------------------------------------
#ifndef __MX31_IRQ_H
#define __MX31_IRQ_H


//------------------------------------------------------------------------------
// MX31-SPECIFIC IRQ DEFINITIONS
//------------------------------------------------------------------------------
#define IRQ_RESERVED0           0
#define IRQ_RESERVED1           1
#define IRQ_RESERVED2           2
#define IRQ_I2C3                3
#define IRQ_I2C2                4
#define IRQ_MPEG4_ENCODE        5
#define IRQ_RTIC                6
#define IRQ_FIRI                7
#define IRQ_ATA_CTRL            15
#define IRQ_GACC                16
#define IRQ_CSPI3               17
#define IRQ_UART3               18
#define IRQ_IIM                 19
#define IRQ_EVTMON              23
#define IRQ_PWR_FAIL            30
#define IRQ_DVFS                31
#define IRQ_UART2               32
#define IRQ_USB_HOST1           35
#define IRQ_USB_HOST2           36
#define IRQ_USB_OTG             37
#define IRQ_RESERVED38          38
#define IRQ_MSHC1               39
#define IRQ_MSHC2               40
#define IRQ_IPU_ERROR           41
#define IRQ_IPU_GENERAL         42
#define IRQ_RESERVED43          43
#define IRQ_RESERVED44          44
#define IRQ_UART1               45
#define IRQ_UART4               46
#define IRQ_UART5               47
#define IRQ_ECT                 48
#define IRQ_SCC_SCM             49
#define IRQ_SCC_SMN             50
#define IRQ_GPIO2               51
#define IRQ_GPIO1               52
#define IRQ_CCM                 53
#define IRQ_PCMCIA              54
#define IRQ_WDOG                55
#define IRQ_GPIO3               56
#define IRQ_RESERVED57          57
#define IRQ_EXT58               58
#define IRQ_EXT59               59
#define IRQ_EXT60               60
#define IRQ_EXT61               61
#define IRQ_EXT62               62
#define IRQ_EXT63               63


//------------------------------------------------------------------------------

#endif // __MX31_IRQ_H
