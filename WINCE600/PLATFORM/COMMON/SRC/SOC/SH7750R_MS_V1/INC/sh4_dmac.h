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
//  File:  sh4_uart.h
//
//  This header file defines DMAC register layout and associated constants
//  and types.
//
//  Note:  Unit base address is defined in sh4_base_regs.h header file.
//
#ifndef __SH4_DMAC_H
#define __SH4_DMAC_H

//------------------------------------------------------------------------------

typedef struct {
    UINT32  SAR0;
    UINT32  DAR0;
    UINT32  DMATCR0;
    UINT32  CHCR0;
    UINT32  SAR1;
    UINT32  DAR1;
    UINT32  DMATCR1;
    UINT32  CHCR1;
    UINT32  SAR2;
    UINT32  DAR2;
    UINT32  DMATCR2;
    UINT32  CHCR2;
    UINT32  SAR3;
    UINT32  DAR3;
    UINT32  DMATCR3;
    UINT32  CHCR3;
    UINT32  DMAOR;
} SH4_DMAC_REGS, *PSH4_DMAC_REGS;

typedef struct {
    UINT32  SAR4;
    UINT32  DAR4;
    UINT32  DMATCR4;
    UINT32  CHCR4;
    UINT32  SAR5;
    UINT32  DAR5;
    UINT32  DMATCR5;
    UINT32  CHCR5;
    UINT32  SAR6;
    UINT32  DAR6;
    UINT32  DMATCR6;
    UINT32  CHCR6;
    UINT32  SAR7;
    UINT32  DAR7;
    UINT32  DMATCR7;
    UINT32  CHCR7;
} SH4_DMAC_REGS_EX, *PSH4_DMAC_REGS_EX;


//------------------------------------------------------------------------------
#define	DMAC_CHCR_RL_ACTIVE_HIGH	0x00000000

#define	DMAC_CHCR_DM_INCREMENTED	0x00004000

#define	DMAC_CHCR_SM_INCREMENTED	0x00001000

#define	DMAC_CHCR_TS_32			    0x00000030

#define	DMAC_CHCR_TE_DMATCR_COMP	0x00000002

#define	DMAC_CHCR_DE_DISABLED		0x00000000
#define	DMAC_CHCR_DE_ENABLED		0x00000001

#define	DMAC_CHCR_IE     		    0x00000040
#define	DMAC_CHCR_IE_NOT_GENARATED	0x00000000
#define	DMAC_CHCR_IE_GENERATED		0x00000004

#define	DMAC_DMAOR_PR11			    0x00000300
#define	DMAC_DMAOR_NMFI			    0x00000002
#define	DMAC_DMAOR_DME			    0x00000001
//------------------------------------------------------------------------------

#endif
