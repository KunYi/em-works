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
//  Header:  mxarm11_iomux.h
//
//  This header contains common IOMUX definitions that are specific to the
//  Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------

#ifndef __MXARM11_IOMUX_H__
#define __MXARM11_IOMUX_H__


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

// IOMUX pinout groupings for request/release IOCTLs
#define IOMUX_PINOUT_I2C                0
#define IOMUX_PINOUT_UART1              1
#define IOMUX_PINOUT_UART2              2
#define IOMUX_PINOUT_OWIRE              3
#define IOMUX_PINOUT_CSPI1              4
#define IOMUX_PINOUT_KPP                5
#define IOMUX_PINOUT_USBOTG             6
#define IOMUX_PINOUT_SDHC1              7
#define IOMUX_PINOUT_SDHC2              8
#define IOMUX_PINOUT_CSPI2              9
#define IOMUX_PINOUT_FIRI               10
#define IOMUX_PINOUT_GPT                11
#define IOMUX_PINOUT_IPU_SDC            12
#define IOMUX_PINOUT_IPU_ADC            13
#define IOMUX_PINOUT_IPU_CSI            14
#define IOMUX_PINOUT_AUDMUX1            15
#define IOMUX_PINOUT_AUDMUX2            16
#define IOMUX_PINOUT_AUDMUX3            17
#define IOMUX_PINOUT_PWM                18
#define IOMUX_PINOUT_NANDFC             19
#define IOMUX_PINOUT_MXARM11            19

#endif // __MXARM11_IOMUX_H__