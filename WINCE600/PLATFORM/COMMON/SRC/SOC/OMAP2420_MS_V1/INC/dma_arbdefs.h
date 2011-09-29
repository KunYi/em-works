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
//
// Copyright <C> Intrinsyc Software International. All rights reserved.
//
// DMA Arbitration driver definitions
// 
// To use arbiter, include dma_arb.h and use the functions there
//

#ifndef __DMA_ARBDEFS_H
#define __DMA_ARBDEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef uint_defined
#define uint_defined
typedef unsigned int    uint;
#endif

#define DMA_CONTROLLER_NAME_STRBUFCHARS   32
typedef struct DMAcontroller 
{
    uint mSysId;
    char mName[DMA_CONTROLLER_NAME_STRBUFCHARS];
} DMACONTROLLER;

typedef enum DMA_seqProperty
{
    DMACP_None=0,

    DMACP_L3IntEnb, /* value = bits to set   in L3 irq enable register */
    DMACP_L3IntDis, /* value = bits to clear in L3 irq enable register */
    DMACP_L3IntAck, /* value = bit  to set   in L3 irq status register */

    DMACP_L2IntEnb, /* value = bits to set   in L2 irq enable register */
    DMACP_L2IntDis, /* value = bits to clear in L2 irq enable register */
    DMACP_L2IntAck, /* value = bit  to set   in L2 irq status register */

    DMACP_L0IntEnb, /* value = bits to set   in L0 irq enable register */
    DMACP_L0IntDis, /* value = bits to clear in L0 irq enable register */
    DMACP_L0IntAck, /* value = bit  to set   in L0 irq status register */
    /* ... */
} DMA_CONT_PROPERTY;

#ifdef __cplusplus
};
#endif

#endif // __DMA_ARBDEFS_H
