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
//  Header: s3c6410_pwm.h
//
//  Defines the PWM Timer register layout and associated types and constants.
//
#ifndef __S3C6410_PWM_H
#define __S3C6410_PWM_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  S3C6410_PWM_REG
//
//  Defines the PWM Timer control register layout. This register bank is
//  located by the constant S3C6410_BASE_REG_XX_PWM in the configuration file
//  s3c6410_base_reg_cfg.h.
//

typedef struct
{
    UINT32 TCFG0;        //0x00
    UINT32 TCFG1;        //0x04
    UINT32 TCON;        //0x08
    UINT32 TCNTB0;        //0x0C

    UINT32 TCMPB0;        //0x10
    UINT32 TCNTO0;        //0x14
    UINT32 TCNTB1;        //0x18
    UINT32 TCMPB1;        //0x1C

    UINT32 TCNTO1;        //0x20
    UINT32 TCNTB2;        //0x24
    UINT32 TCMPB2;        //0x28
    UINT32 TCNTO2;        //0x2C

    UINT32 TCNTB3;        //0x30
    UINT32 TCMPB3;        //0x34
    UINT32 TCNTO3;        //0x38
    UINT32 TCNTB4;        //0x3C

    UINT32 TCNTO4;        //0x40
    UINT32 TINT_CSTAT;    // 0x44
    UINT32 PAD[2];        // 0x48~0x4f
} S3C6410_PWM_REG, *PS3C6410_PWM_REG;

//------------------------------------------------------------------------------

// Do not use OR/AND operation on TINT_CSTAT SFR
// to clear interrupt pending or enable/disable interrupt
// TINT_CSTAT SFR have multiple pending bit of Timer0~4
// You should write "1" on only corresponding pending bit
#define TINT_CSTAT_INTMASK(n)            ((n)&0x1F)    // Remove Pending Status Value from SFR
#define TIMER4_PENDING_CLEAR            (1<<9)
#define TIMER3_PENDING_CLEAR            (1<<8)
#define TIMER2_PENDING_CLEAR            (1<<7)
#define TIMER1_PENDING_CLEAR            (1<<6)
#define TIMER0_PENDING_CLEAR            (1<<5)
#define TIMER4_INTERRUPT_ENABLE        (1<<4)
#define TIMER3_INTERRUPT_ENABLE        (1<<3)
#define TIMER2_INTERRUPT_ENABLE        (1<<2)
#define TIMER1_INTERRUPT_ENABLE        (1<<1)
#define TIMER0_INTERRUPT_ENABLE        (1<<0)

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif    // __S3C6410_PWM_H