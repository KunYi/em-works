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
//  Header:  bulverde_memctrl.h
//
//  Defines the memory controller register layout and associated 
//  constants and types.
//
#ifndef __BULVERDE_MEMCTRL_H
#define __BULVERDE_MEMCTRL_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  BULVERDE_MEMCTRL_REG    
//
//  Memory controller register layout.
//

typedef struct
{
    UINT32    mdcnfg;
    UINT32    mdrefr;
    UINT32    msc0;
    UINT32    msc1;
    UINT32    msc2;
    UINT32    mecr;
    UINT32    rsvd0;
    UINT32    sxcnfg;
    UINT32    flycnfg;
    UINT32    rsvd1;
    UINT32    mcmem0;
    UINT32    mcmem1;
    UINT32    mcatt0;
    UINT32    mcatt1;
    UINT32    mcio0;
    UINT32    mcio1;
    UINT32    mdmrs;
    UINT32    boot_def;
    UINT32    arb_cntl;
    UINT32    bscntrp;
    UINT32    bscntrn;
    UINT32    lcdbscntr;
    UINT32    mdmrslp;

} BULVERDE_MEMCTRL_REG, *PBULVERSE_MEMCTRL_REG;

// Auto power down bit
#define MEMCTRL_MDREFR_APD           (0x00100000)

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
