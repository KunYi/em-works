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
//  Header: bulverde_mmc.h
//
//  Defines the multi-media card (MMC) register layout and associated 
//  types and constants.
//
#ifndef __BULVERDE_MMC_H__
#define __BULVERDE_MMC_H__

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: BULVERDE_MMC_REG    
//
//  MMC control registers.
//
typedef struct
{
    UINT32    strpc;     
    UINT32    stat;     
    UINT32    clkrt;     
    UINT32    spi;     
    UINT32    cmdat;     
    UINT32    resto;     
    UINT32    rdto;     
    UINT32    blkle;     
    UINT32    nob;     
    UINT32    prtbu;     
    UINT32    imask;     
    UINT32    ireg;     
    UINT32    cmd;     
    UINT32    argh;     
    UINT32    argl;     
    UINT32    res;     
    UINT32    rxfifo;     
    UINT32    txfifo;     

} BULVERDE_MMC_REG, *PBULVERDE_MMC_REG;

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
