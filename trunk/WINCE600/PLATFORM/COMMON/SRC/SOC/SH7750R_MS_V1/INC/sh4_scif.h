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
//  File:  sh4_scif.h
//
//  This header file defines Serial Communication Interface registers layout and
//  associated constants and types.
//
//  Note:  Base address is defined in sh4_base_regs.h header file.
//
#ifndef __SH4_SCIF_H
#define __SH4_SCIF_H

//------------------------------------------------------------------------------

typedef struct {
    UINT16  SCSMR2;
    UINT16  UNUSED2[1];
    UINT8   SCBRR2;
    UINT8   UNUSED5[3];
    UINT16  SCSCR2;
    UINT16  UNUSEDA[1];
    UINT8   SCFTDR2;
    UINT8   UNUSEDD[3];
    UINT16  SCFSR2;
    UINT16  UNUSED12[1];
    UINT8   SCFRDR2;
    UINT8   UNUSED15[3];
    UINT16  SCFCR2;
    UINT16  UNUSED1A[1];
    UINT16  SCFDR2;
    UINT16  UNUSED1E[1];
    UINT16  SCSPTR2;
    UINT16  UNUSED22[1];
    UINT16  SCLSR2;
    UINT16  UNUSED26[1];
} SH4_SCIF_REGS, *PSH4_SCIF_REGS;

//------------------------------------------------------------------------------
// Bit fields for Serial Status Register SCFSR2
//
#define SCIF_SCFSR2_ER          0x0080
#define SCIF_SCFSR2_TEND        0x0040
#define SCIF_SCFSR2_TDFE        0x0020
#define SCIF_SCFSR2_BRK         0x0010
#define SCIF_SCFSR2_FER         0x0008
#define SCIF_SCFSR2_PER         0x0004
#define SCIF_SCFSR2_RDF         0x0002
#define SCIF_SCFSR2_DR          0x0001

//------------------------------------------------------------------------------
// Bit fields for Serial Control Register SCSCR2
//
#define SCIF_SCSCR2_TIE         0x0080
#define SCIF_SCSCR2_RIE         0x0040
#define SCIF_SCSCR2_BRIE        0x0004
#define SCIF_SCSCR2_REIE        0x0004

//------------------------------------------------------------------------------
// Bit fields for Serial Status Register SCLSR2
//
#define SCIF_SCLSR2_ORER           0x0001

//------------------------------------------------------------------------------

#endif
