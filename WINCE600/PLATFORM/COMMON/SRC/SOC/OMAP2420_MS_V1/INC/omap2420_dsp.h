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
// Portions Copyright (c) Texas Instruments.  All rights reserved. 
//
//------------------------------------------------------------------------------
//
//  File:  omap2420_dsp.h
//
//  This header file is comprised of register structures for controllong 
//  the DSP subsystem from ARM.
//

#ifndef __OMAP2420_DSP_H
#define __OMAP2420_DSP_H

//------------------------------------------------------------------------------

// NOTE
// DSP Subsystem has the following modules.
// 1. DSP Core       - Accessible from only DSP IO space
// 2. dINTC          - DSP memory space
// 3. DSP SS Global  - DSP memory space
// 4. dDMA           - DSP memory space
// 5. DSP MMU        - OMAP 24xx memory space
// 6. IPI            - OMAP 24xx memory space

// Since the items 1-4 are accessible only from DSP IO and memory space
// the structures for them will not be defined from the ARM side.
// The item DSP MMU is already defined under MMU section.
// So the structures will be created for the IPI registers alone.

//
// IPI Register - SEE omap2420_base_regs.h base address OMAP2420_DSP_IPI_REGS_PA
//

typedef struct __IPIREGS__
{
   unsigned long ulIPI_REVISION;           //offset 0x00, IP revision code
   unsigned long ulRESERVED_1[3];
   volatile unsigned long ulIPI_SYSCONFIG; //offset 0x10, OCP i/f params
   unsigned long ulRESERVED_2[11];
   volatile unsigned long ulIPI_INDEX;     //offset 0x40, table index 
   volatile unsigned long ulIPI_ENTRY;     //offset 0x44, IPI entry register
   volatile unsigned long ulIPI_ENABLE;    //offset 0x48, IPI enable register
   volatile unsigned long ulIPI_IOMAP;     //offset 0x4C, IOMap base addr for 
                                           //conversion from 16-bit DSP-IO word
                                           //addr to 24-bit DSP-MEM byte addr
   volatile unsigned long ulIPI_DSPBOOTCFG;//offset 0x50,DSP config for boot
}
OMAP2420_DSP_IPI_REGS, *pIPIREGS;

#endif
