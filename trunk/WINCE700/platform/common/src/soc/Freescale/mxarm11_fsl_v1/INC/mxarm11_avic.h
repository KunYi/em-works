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
//  Header:  mxarm11_avic.h
//
//  Provides definitions for AVIC module based on Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_AVIC_H
#define __MXARM11_AVIC_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define AVIC_IRQ_SOURCES_MAX    64
#define AVIC_IRQ_UNDEFINED      (-1)


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 INTCNTL;
    UINT32 NIMASK;
    UINT32 INTENNUM;
    UINT32 INTDISNUM;
    UINT32 INTENABLEH;
    UINT32 INTENABLEL;
    UINT32 INTTYPEH;
    UINT32 INTTYPEL;
    UINT32 NIPRIORITY7;
    UINT32 NIPRIORITY6;
    UINT32 NIPRIORITY5;
    UINT32 NIPRIORITY4;
    UINT32 NIPRIORITY3;
    UINT32 NIPRIORITY2;
    UINT32 NIPRIORITY1;
    UINT32 NIPRIORITY0;
    UINT32 NIVECSR;
    UINT32 FIVECSR;
    UINT32 INTSRCH;
    UINT32 INTSRCL;
    UINT32 INTFRCH;
    UINT32 INTFRCL;
    UINT32 NIPNDH;
    UINT32 NIPNDL;
    UINT32 FIPNDH;
    UINT32 FIPNDL;
    UINT32 VECTOR[AVIC_IRQ_SOURCES_MAX];
} CSP_AVIC_REGS, *PCSP_AVIC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define AVIC_INTCNTL_OFFSET         0x0000
#define AVIC_NIMASK_OFFSET          0x0004
#define AVIC_INTENNUM_OFFSET        0x0008
#define AVIC_INTDISNUM_OFFSET       0x000C
#define AVIC_INTENABLEH_OFFSET      0x0010
#define AVIC_INTENABLEL_OFFSET      0x0014
#define AVIC_INTTYPEH_OFFSET        0x0018
#define AVIC_INTTYPEL_OFFSET        0x001C
#define AVIC_NIPRIORITY7_OFFSET     0x0020
#define AVIC_NIPRIORITY6_OFFSET     0x0024
#define AVIC_NIPRIORITY5_OFFSET     0x0028
#define AVIC_NIPRIORITY4_OFFSET     0x002C
#define AVIC_NIPRIORITY3_OFFSET     0x0030
#define AVIC_NIPRIORITY2_OFFSET     0x0034
#define AVIC_NIPRIORITY1_OFFSET     0x0038
#define AVIC_NIPRIORITY0_OFFSET     0x003C
#define AVIC_NIVECSR_OFFSET         0x0040
#define AVIC_FIVECSR_OFFSET         0x0044
#define AVIC_INTSRCH_OFFSET         0x0048
#define AVIC_INTSRCL_OFFSET         0x004C
#define AVIC_INTFRCH_OFFSET         0x0050
#define AVIC_INTFRCL_OFFSET         0x0054
#define AVIC_NIPNDH_OFFSET          0x0058
#define AVIC_NIPNDL_OFFSET          0x005C
#define AVIC_FIPNDH_OFFSET          0x0060
#define AVIC_FIPNDL_OFFSET          0x0064
#define AVIC_VECTOR_OFFSET          0x0100


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define AVIC_INTCNTL_NM_LSH         18
#define AVIC_INTCNTL_FIAD_LSH       19
#define AVIC_INTCNTL_NIAD_LSH       20
#define AVIC_INTCNTL_FIDIS_LSH      21
#define AVIC_INTCNTL_NIDIS_LSH      22
#define AVIC_INTCNTL_ABFEN_LSH      24
#define AVIC_INTCNTL_ABFLAG_LSH     25

#define AVIC_NIMASK_NIMASK_LSH      0

#define AVIC_INTENNUM_ENNUM_LSH     0

#define AVIC_INTDISNUM_DISNUM_LSH   0

#define AVIC_INTENABLEH_INTENABLE_LSH   0

#define AVIC_INTENABLEL_INTENABLE_LSH   0

#define AVIC_INTTYPEH_INTTYPE_LSH   0

#define AVIC_INTTYPEL_INTTYPE_LSH   0

#define AVIC_NIVECSR_NIPRILVL_LSH   0
#define AVIC_NIVECSR_NIVECTOR_LSH   16

#define AVIC_FIVECSR_FIVECTOR_LSH   0

#define AVIC_INTSRCH_INTIN_LSH      0

#define AVIC_INTSRCL_INTIN_LSH      0

#define AVIC_INTFRCH_FORCE_LSH      0

#define AVIC_INTFRCL_FORCE_LSH      0

#define AVIC_NIPNDH_NIPEND_LSH      0

#define AVIC_NIPNDL_NIPEND_LSH      0

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define AVIC_INTCNTL_NM_WID         1
#define AVIC_INTCNTL_FIAD_WID       1
#define AVIC_INTCNTL_NIAD_WID       1
#define AVIC_INTCNTL_FIDIS_WID      1
#define AVIC_INTCNTL_NIDIS_WID      1
#define AVIC_INTCNTL_ABFEN_WID      1
#define AVIC_INTCNTL_ABFLAG_WID     1

#define AVIC_NIMASK_NIMASK_WID      5

#define AVIC_INTENNUM_ENNUM_WID     6

#define AVIC_INTDISNUM_DISNUM_WID   6

#define AVIC_INTENABLEH_INTENABLE_WID   32

#define AVIC_INTENABLEL_INTENABLE_WID   32

#define AVIC_INTTYPEH_INTTYPE_WID   32

#define AVIC_INTTYPEL_INTTYPE_WID   32

#define AVIC_NIVECSR_NIPRILVL_WID   16
#define AVIC_NIVECSR_NIVECTOR_WID   16

#define AVIC_FIVECSR_FIVECTOR_WID   32

#define AVIC_INTSRCH_INTIN_WID      32

#define AVIC_INTSRCL_INTIN_WID      32

#define AVIC_INTFRCH_FORCE_WID      32

#define AVIC_INTFRCL_FORCE_WID      32

#define AVIC_NIPNDH_NIPEND_WID      32

#define AVIC_NIPNDL_NIPEND_WID      32


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif // __MXARM11_AVIC_H