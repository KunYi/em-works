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
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx27_aitc.h
//
//  Provides definitions for AITC module based on ARM926EJ-S.
//
//------------------------------------------------------------------------------
#ifndef __MX27_AITC_H__
#define __MX27_AITC_H__

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define AITC_IRQ_SOURCES_MAX    64
#define AITC_IRQ_UNDEFINED      (-1)


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    REG32 INTCNTL;
    REG32 NIMASK;
    REG32 INTENNUM;
    REG32 INTDISNUM;
    REG32 INTENABLEH;
    REG32 INTENABLEL;
    REG32 INTTYPEH;
    REG32 INTTYPEL;
    REG32 NIPRIORITY7;
    REG32 NIPRIORITY6;
    REG32 NIPRIORITY5;
    REG32 NIPRIORITY4;
    REG32 NIPRIORITY3;
    REG32 NIPRIORITY2;
    REG32 NIPRIORITY1;
    REG32 NIPRIORITY0;
    REG32 NIVECSR;
    REG32 FIVECSR;
    REG32 INTSRCH;
    REG32 INTSRCL;
    REG32 INTFRCH;
    REG32 INTFRCL;
    REG32 NIPNDH;
    REG32 NIPNDL;
    REG32 FIPNDH;
    REG32 FIPNDL;
} CSP_AITC_REGS, *PCSP_AITC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define AITC_INTCNTL_OFFSET             0x00000000
#define AITC_NIMASK_OFFSET              0x00000004
#define AITC_INTENNUM_OFFSET            0x00000008
#define AITC_INTDISNUM_OFFSET           0x0000000C
#define AITC_INTENABLEH_OFFSET          0x00000010
#define AITC_INTENABLEL_OFFSET          0x00000014
#define AITC_INTTYPEH_OFFSET            0x00000018
#define AITC_INTTYPEL_OFFSET            0x0000001C
#define AITC_NIPRIORITY7_OFFSET         0x00000020
#define AITC_NIPRIORITY6_OFFSET         0x00000024
#define AITC_NIPRIORITY5_OFFSET         0x00000028
#define AITC_NIPRIORITY4_OFFSET         0x0000002C
#define AITC_NIPRIORITY3_OFFSET         0x00000030
#define AITC_NIPRIORITY2_OFFSET         0x00000034
#define AITC_NIPRIORITY1_OFFSET         0x00000038
#define AITC_NIPRIORITY0_OFFSET         0x0000003C
#define AITC_NIVECSR_OFFSET             0x00000040
#define AITC_FIVECSR_OFFSET             0x00000044
#define AITC_INTSRCH_OFFSET             0x00000048
#define AITC_INTSRCL_OFFSET             0x0000004C
#define AITC_INTFRCH_OFFSET             0x00000050
#define AITC_INTFRCL_OFFSET             0x00000054
#define AITC_NIPNDH_OFFSET              0x00000058
#define AITC_NIPNDL_OFFSET              0x0000005C
#define AITC_FIPNDH_OFFSET              0x00000060
#define AITC_FIPNDL_OFFSET              0x00000064

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

//Interrupt Control Register (INTCNTL)
#define AITC_INTCNTL_POINTER_LSH        2
#define AITC_INTCNTL_MD_LSH             16
#define AITC_INTCNTL_FIAD_LSH           19
#define AITC_INTCNTL_NIAD_LSH           20
#define AITC_INTCNTL_FIDIS_LSH          21
#define AITC_INTCNTL_NIDIS_LSH          22

// Normal Interrupt Mask Register (NIMASK)
#define AITC_NIMASK_NIMASK_LSH          0

// Interrupt Enable Number Register (INTENNUM)
#define AITC_INTENNUM_ENNUM_LSH         0

// Interrupt Disable Number Register (INTDISNUM)
#define AITC_INTDISNUM_DISNUM_LSH       0

// Interrupt Enable Register High (INTENABLEH) and Low(INTENABLEL)
#define AITC_INTENABLEH_INTENABLE_LSH   0
#define AITC_INTENABLEL_INTENABLE_LSH   0

// Interrupt Type Register High (INTTYPEH) and Low (INTTYPEL)
#define AITC_INTTYPEH_INTTYPE_LSH       0
#define AITC_INTTYPEL_INTTYPE_LSH       0

// Normal Interrupt Vector and Status Register (NIVECSR)
#define AITC_NIVECSR_NIPRILVL_LSH       0
#define AITC_NIVECSR_NIVECTOR_LSH       16

// Fast Interrupt Vector and Status Register (FIVECSR)
#define AITC_FIVECSR_FIVECTOR_LSH       0

// Interrupt Source Register High (INTSRCH) and Low (INTSRCL)
#define AITC_INTSRCH_INTIN_LSH          0
#define AITC_INTSRCL_INTIN_LSH          0

// Interrupt Force Register High (INTFRCH) and Low (INTFRCL)
#define AITC_INTFRCH_FORCE_LSH          0
#define AITC_INTFRCL_FORCE_LSH          0

// Normal Interrupt Pending Register High (NIPNDH) and Low (NIPNDL)
#define AITC_NIPNDH_NIPEND_LSH          0
#define AITC_NIPNDL_NIPEND_LSH          0

// Fast Interrupt Pending Register High (FIPNDH) and Low (FIPNDL)
#define AITC_FIPNDH_FIPEND_LSH          0
#define AITC_FIPNDL_FIPEND_LSH          0


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

//Interrupt Control Register (INTCNTL)
#define AITC_INTCNTL_POINTER_WID        10
#define AITC_INTCNTL_MD_WID             1
#define AITC_INTCNTL_FIAD_WID           1
#define AITC_INTCNTL_NIAD_WID           1
#define AITC_INTCNTL_FIDIS_WID          1
#define AITC_INTCNTL_NIDIS_WID          1

// Normal Interrupt Mask Register (NIMASK)
#define AITC_NIMASK_NIMASK_WID          5

// Interrupt Enable Number Register (INTENNUM)
#define AITC_INTENNUM_ENNUM_WID         6

// Interrupt Disable Number Register (INTDISNUM)
#define AITC_INTDISNUM_DISNUM_WID       6

// Interrupt Enable Register High (INTENABLEH) and Low(INTENABLEL)
#define AITC_INTENABLEH_INTENABLE_WID   32
#define AITC_INTENABLEL_INTENABLE_WID   32

// Interrupt Type Register High (INTTYPEH) and Low (INTTYPEL)
#define AITC_INTTYPEH_INTTYPE_WID       32
#define AITC_INTTYPEL_INTTYPE_WID       32

// Normal Interrupt Vector and Status Register (NIVECSR)
#define AITC_NIVECSR_NIPRILVL_WID       16
#define AITC_NIVECSR_NIVECTOR_WID       16

// Fast Interrupt Vector and Status Register (FIVECSR)
#define AITC_FIVECSR_FIVECTOR_WID       32

// Interrupt Source Register High (INTSRCH) and Low (INTSRCL)
#define AITC_INTSRCH_INTIN_WID          32
#define AITC_INTSRCL_INTIN_WID          32

// Interrupt Force Register High (INTFRCH) and Low (INTFRCL)
#define AITC_INTFRCH_FORCE_WID          32
#define AITC_INTFRCL_FORCE_WID          32

// Normal Interrupt Pending Register High (NIPNDH) and Low (NIPNDL)
#define AITC_NIPNDH_NIPEND_WID          32
#define AITC_NIPNDL_NIPEND_WID          32

// Fast Interrupt Pending Register High (FIPNDH) and Low (FIPNDL)
#define AITC_FIPNDH_FIPEND_WID          32
#define AITC_FIPNDL_FIPEND_WID          32

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif // __MX27_AITC_H__
