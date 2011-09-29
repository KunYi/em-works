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
//  File:  sh4_rtc.h
//
//  This header file defines Real Time Clock registers layout and
//  associated constants and types.
//
//  Note:  Base address is defined in sh4_base_regs.h header file.
//
#ifndef __SH4_RTC_H
#define __SH4_RTC_H

//------------------------------------------------------------------------------

typedef struct {
    UINT8   R64CNT;
    UINT8   UNUSED1[3];
    UINT8   RSECCNT;
    UINT8   UNUSED5[3];
    UINT8   RMINCNT;
    UINT8   UNUSED9[3];
    UINT8   RHRCNT;
    UINT8   UNUSEDD[3];
    UINT8   RWKCNT;
    UINT8   UNUSED11[3];
    UINT8   RDAYCNT;
    UINT8   UNUSED15[3];
    UINT8   RMONCNT;
    UINT8   UNUSED19[3];
    UINT16  RYRCNT;
    UINT16  UNUSED1E[1];
    UINT8   RSECAR;
    UINT8   UNUSED21[3];
    UINT8   RMINAR;
    UINT8   UNUSED25[3];
    UINT8   RHRAR;
    UINT8   UNUSED29[3];
    UINT8   RWKAR;
    UINT8   UNUSED2D[3];
    UINT8   RDAYAR;
    UINT8   UNUSED31[3];
    UINT8   RMONAR;
    UINT8   UNUSED35[3];
    UINT8   RCR1;
    UINT8   UNUSED39[3];
    UINT8   RCR2;
    UINT8   UNUSED3D[3];
} SH4_RTC_REGS, *PSH4_RTC_REGS;

typedef struct {
    UINT8   RCR3;
    UINT8   UNUSED1[3];
    UINT16  RYRAR;
    UINT16  UNUSED6[1];
} SH4_RTC_REGS_EX, *PSH4_RTC_REGS_EX;

//------------------------------------------------------------------------------
// Bit field for Real Time Clock register RCR1
//
#define RTC_RCR1_CF     0x80
#define RTC_RCR1_AF     0x01
#define RTC_RCR1_CIE    0x10
#define RTC_RCR1_AIE    0x08

#define RTC_RSECAR_ENB  0x80
#define RTC_RMINAR_ENB  0x80
#define RTC_RHRAR_ENB   0x80
#define RTC_RWKAR_ENB   0x80
#define RTC_RDAYAR_ENB  0x80
#define RTC_RMONAR_ENB  0x80
#define RTC_RYRAR_ENB   0x80

//------------------------------------------------------------------------------
// Bit field for Real Time Clock register RCR2
//
#define RTC_RCR2_RESET  0x02
#define RTC_RCR2_START  0x01
#define RTC_RCR2_PIE    0x80

//------------------------------------------------------------------------------

#endif
