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
//  File:  sh4_bsc.h
//
//  This header file defines Bus State Control registers layout and
//  associated constants and types.
//
//  Note:  Base address is defined in sh4_base_regs.h header file.
//
#ifndef __SH4_BSC_H
#define __SH4_BSC_H

//------------------------------------------------------------------------------

typedef struct {
    UINT32  BCR1;
    UINT16  BCR2;
    UINT16  UNUSED6[1];
    UINT32  WCR1;
    UINT32  WCR2;
    UINT32  WCR3;
    UINT32  MCR;
    UINT16  PCR;
    UINT16  UNUSED1A[1];
    UINT16  RTCSR;
    UINT16  UNUSED1D[1];
    UINT16  RTCNT;
    UINT16  UNUSED22[1];
    UINT16  RTCOR;
    UINT16  UNUSED26[1];
    UINT16  RFCR;
    UINT16  UNUSED2A[1];
    UINT32  PCTRA;
    UINT16  PDTRA;
    UINT16  UNUSED32[1];
    UINT32  UNUSED34[3];
    UINT32  PCTRB;
    UINT16  PDTRB;
    UINT16  UNUSED46[1];
    UINT16  GPIOIC;
    UINT16  UNUSED4A[1];
} SH4_BSC_REGS, *PSH4_BSC_REGS;

//------------------------------------------------------------------------------

#define BSC_RTCSR_COOKIE    0xA500
#define BSC_RTCSR_RCMIE     0x40
#define BSC_RTCSR_ROVIE     0x02

//------------------------------------------------------------------------------

#endif
