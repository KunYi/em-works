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
//  File:  vr4131_siu.h
//
//  This header file defines Serial Interface Unit register layout and
//  associated constants and types. Because SoC Debug Serial Interface Unit
//  has similar register layout following structure can be used
//  for this unit also.
//
//  Unit base address is defined in vr4131_base_regs.h header file.
//
#ifndef __VR4131_SIU_H
#define __VR4132_SIU_H

//------------------------------------------------------------------------------

#ifndef __MIPS_ASSEMBLER

typedef struct {
    union {
        UINT8 RB;
        UINT8 TH;
        UINT8 DLL;
    };
    union {
        UINT8 IE;
        UINT8 DLM;
    };
    union {
        UINT8 IID;
        UINT8 FC;
    };
    UINT8 LC;
    UINT8 MC;
    UINT8 LS;
    UINT8 MS;
    UINT8 SC;
    UINT8 IRSEL;
    UINT8 RESET;
    UINT8 CSEL;
} VR4131_SIU_REGS;

#endif // __MIPS_ASSEMBLER

//------------------------------------------------------------------------------

#define SIU_DSIURESET   (1 << 1)
#define SIU_SIURESET    (1 << 0)

#define SIU_FCR0        (1 << 0)        // Receive/Transmit FIFO enable
#define SIU_FCR1        (1 << 1)        // Receive FIFO clear
#define SIU_FCR2        (1 << 2)        // Transmit FIFO clear

#define SIU_LCR7        (1 << 7)        // Switch to divisor latch access
#define SIU_8BIT        (3 << 0)

#define SIU_RTS         (1 << 1)        // Modem Control RTS#
#define SIU_DTR         (1 << 0)        // Modem Control DTR#

#define SIU_ERR         (1 << 7)        // Error Detection
#define SIU_THRE        (1 << 5)        // Transmission Hold Registry Empty
#define SIU_RDR         (1 << 0)        // Receive Data Ready

//------------------------------------------------------------------------------

#endif
