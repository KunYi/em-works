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
//  Header: bulverde_dma.h
//
//  Defines the DMA controller CPU register layout and definitions.
//
#ifndef __BULVERDE_DMA_H
#define __BULVERDE_DMA_H

#if __cplusplus
    extern "C" 
    {
#endif


//
// DMA source enumeration.
//
enum XSC1_DmaSource
{
    XSC1_UDC_Xmit = 0,
    XSC1_UDC_Rcv,
    XSC1_SDLC_Xmit,
    XSC1_SDLC_Rcv,
    XSC1_UART1_Xmit,
    XSC1_UART1_Rcv,
    XSC1_UART2_Xmit,
    XSC1_UART2_Rcv,
    XSC1_HSSP_Xmit,
    XSC1_HSSP_Rcv,
    XSC1_UART3_Xmit,
    XSC1_UART3_Rcv,
    XSC1_MCP_AudioXmit,
    XSC1_MCP_AudioRcv,
    XSC1_MCP_TelecomXmit,
    XSC1_MCP_TelecomRcv,
    XSC1_SSP_Xmit,
    XSC1_SSP_Rcv
};


//
// DMA descriptor - channel type.
//
typedef struct 
{
    UINT32   ddadr;    // descriptor address reg.
    UINT32   dsadr;    // source address register.
    UINT32   dtadr;    // target address register.
    UINT32   dcmd;     // command address register.

} DMADescriptorChannelType;


//------------------------------------------------------------------------------
//  Type: BULVERDE_DMA_REG    
//
//  DMA control registers.
//

typedef struct 
{
    UINT32    dcsr[32];	     // DMA CSRs by channel.
    UINT32    rsvd0[0x8];    
    UINT32    dalign;		        
    UINT32    dpcsr;		        
    UINT32    rsvd1[0xE];
    UINT32    drqsr0;        // Dreq[0] Status.
    UINT32    drqsr1;        // Dreq[1] Status.
    UINT32    drqsr2;        // Dreq[1] Status.
    UINT32    rsvd2[0x1];
    UINT32    dint;          // DMA interrupt Register.
    UINT32    rsvd3[0x3];
    UINT32    drcmr[64];     // On-chip device DMA request --> channel map registers [63:0].
    DMADescriptorChannelType ddg[32];    // 32 channels of descriptor registers.
    UINT32    rsvd4[0x340];
    UINT32    drcmr2[11];    // On-chip device DMA request --> channel map registers [63:0].

} BULVERDE_DMA_REG, *PBULVERDE_DMA_REG;

#if __cplusplus
    }
#endif

#endif 
