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
//  File:  sh4_tmu.h
//
//  This header file defines Timer Unit register layout and
//  associated constants and types.
//
//  Note:  Unit base addresses are defined in sh4_base_regs.h header file.
//
#ifndef __SH4_TMU_H
#define __SH4_TMU_H

//------------------------------------------------------------------------------

typedef struct {
    UINT8   TOCR;
    UINT8   UNUSED1[3];
    UINT8   TSTR;
    UINT8   UNUSED5[3];
    UINT32  TCOR0;
    UINT32  TCNT0;
    UINT16  TCR0;
    UINT16  UNUSED12[1];
    UINT32  TCOR1;
    UINT32  TCNT1;
    UINT16  TCR1;
    UINT16  UNUSED1E[1];
    UINT32  TCOR2;
    UINT32  TCNT2;
    UINT16  TCR2;
    UINT16  UNUSED2A[1];
    UINT32  TCPR2;
} SH4_TMU_REGS, *PSH4_TMU_REGS;

typedef struct {
    UINT32  UNUSED0[1];
    UINT8   TSTR2;
    UINT8   UNUSED5[3];
    UINT32  TCOR3;
    UINT32  TCNT3;
    UINT16  TCR3;
    UINT16  UNUSED12[1];
    UINT32  TCOR4;
    UINT32  TCNT4;
    UINT16  TCR4;
    UINT16  UNUSED1E[1];
} SH4_TMU_REGS_EX, *PSH4_TMU_REGS_EX;

//------------------------------------------------------------------------------

#define TMU_TSTR_STR0   0x01
#define TMU_TSTR_STR1   0x02
#define TMU_TSTR_STR2   0x04

#define TMU_TOCR_ENABLE 0x01

#define TMU_TCR_UNF     0x100   // counter underflowed
#define TMU_TCR_UNIE    0x20    // underflow interrupt enable
#define TMU_TCR_ICPIE   0x40    // input capture interrupt enable

#define TMU_TCR_D4      0x00    // PERIPHERAL clock / 4
#define TMU_TCR_D16     0x01    // PERIPHERAL clock / 16
#define TMU_TCR_D64     0x02    // PERIPHERAL clock / 64
#define TMU_TCR_D256    0x03    // PERIPHERAL clock / 256
#define TMU_TCR_D1024   0x04    // PERIPHERAL clock / 1024

//------------------------------------------------------------------------------

#endif
