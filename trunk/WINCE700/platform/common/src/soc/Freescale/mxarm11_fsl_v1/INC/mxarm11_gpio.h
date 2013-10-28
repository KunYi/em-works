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
//  Header:  mxarm11_gpio.h
//
//  Provides definitions for GPIO module based on Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_GPIO_H
#define __MXARM11_GPIO_H


#if __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define GPIO_INTR_SOURCES_MAX       32
#define GPIO_PINS_PER_PORT          32


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 DR;
    UINT32 GDIR;
    UINT32 PSR;
    UINT32 ICR1;
    UINT32 ICR2;
    UINT32 IMR;
    UINT32 ISR;
} CSP_GPIO_REGS, *PCSP_GPIO_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define GPIO_DR_OFFSET              0x0000
#define GPIO_GDIR_OFFSET            0x0004
#define GPIO_PSR_OFFSET             0x0008
#define GPIO_ICR1_OFFSET            0x000C
#define GPIO_ICR2_OFFSET            0x0010
#define GPIO_IMR_OFFSET             0x0014
#define GPIO_ISR_OFFSET             0x0018


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define GPIO_GDIR_INPUT             0           // GPIO pin is input
#define GPIO_GDIR_OUTPUT            1           // GPIO pin is output

#define GPIO_ICR_LOW_LEVEL          0           // Interrupt is low-level
#define GPIO_ICR_HIGH_LEVEL         1           // Interrupt is high-level
#define GPIO_ICR_RISE_EDGE          2           // Interrupt is rising edge
#define GPIO_ICR_FALL_EDGE          3           // Interrupt is falling edge

#define GPIO_IMR_MASKED             0           // Interrupt is masked
#define GPIO_IMR_UNMASKED           1           // Interrupt is unmasked



//------------------------------------------------------------------------------
// HELPER MACROS
//------------------------------------------------------------------------------
#define GPIO_PIN_MASK(pin)          (1U << (pin))
#define GPIO_PIN_VAL(val, pin)      ((val) << (pin))
#define GPIO_ICR_MASK(pin)          (0x3U << ((pin) << 1))
#define GPIO_ICR_VAL(val, pin)      ((val) << ((pin) << 1))

#ifdef __cplusplus
}
#endif

#endif // __MXARM11_GPIO_H
