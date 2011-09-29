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
#ifndef __PMU_H__
#define __PMU_H__

/* Copyright 2000-2002 Intel Corp.  */
/*++

Module Name: pmu.h

Abstract:
 Contains definitions specific to Intel XScale
 Microarchitecture PMU kernel routines.

--*/

// Crystal Frequency constants for frequency calculations
// PMU API deals in KHZ
//
#define CRYSTAL_KHZ 13000
#define CRYSTAL_KHZDIVTEN    1300

// struct for PMUInterruptCallback function ptr definition
//
typedef struct _SampleInput {
    unsigned long   r14;
    unsigned long   spsr;
    unsigned long   tid;
    unsigned long   pid;
} SampleInput, *PSampleInput;

// OS CallBack definitions
//
typedef BOOL (*PMUInterruptCallback)(SampleInput *);
typedef void (*ReleasePMUCallback)(void);
typedef void (*ReleaseCCFCallback)(void);

// Nomenclature for PMU CP register access (matches spec)
//
#define PMNC 0        // PMU Control Register
#define CCNT 1        // PMU Clock Counter (CCNT)
#define PMN0 2        // PMU Count Register 0 (PMN0)
#define PMN1 3        // PMU Count Register 1 (PMN1)
#define PMN2 4        // PMU Count Register 2 (PMN2)
#define PMN3 5        // PMU Count Register 3 (PMN3)
#define INTEN 6       // PMU Interupt Enable Register (INTEN)
#define FLAG 7        // PMU Overflow Flag Status Register (FLAG)
#define EVTSEL 8      // PMU Event Select Register (EVTSEL)
#define MAXPMUREG 8

#endif // __PMU_H__
