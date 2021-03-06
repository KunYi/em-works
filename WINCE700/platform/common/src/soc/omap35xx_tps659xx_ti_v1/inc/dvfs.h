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
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: dvfs.h
//
#ifndef __DVFS_H__
#define __DVFS_H__

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// dvfs constants
//

// dvfs opp change notification constants
//
#define DVFS_CORE1_OPP              (0)
#define DVFS_MPU1_OPP               (16)
#define DVFS_RESET_TRANSITION      (0)
#define DVFS_FAIL_TRANSITION        (-1)

// dvfs transition flags
//
#define DVFS_CORE1_PRE_NOTICE       (0x00000001)
#define DVFS_CORE1_POST_NOTICE      (0x00000002)
#define DVFS_CORE1_CANCEL_NOTICE    (0x00000004)

#define DVFS_MPU1_PRE_NOTICE        (0x00000010)
#define DVFS_MPU1_POST_NOTICE       (0x00000020)
#define DVFS_MPU1_CANCEL_NOTICE     (0x00000040)

typedef enum {
    kOpm0 = 0,
    kOpm1,
    kOpm2,
    kOpm3,
    kOpm4,
    kOpm5,
    kOpm6,
    kOpm7,
    kOpm8,
    kOpm9,
    kOpmCount,
    kOpmUndefined = -1
}
Dvfs_OperatingMode_e;

typedef enum {
    kOpp1 = 0,
    kOpp2,
    kOpp3,
    kOpp4,
    kOpp5,
    kOpp6,
    kOpp7,
    kOpp8,
    kOpp9,
    kOppCount,
    kOppUndefined = -1
}
Dvfs_OperatingPoint_e;

//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif
