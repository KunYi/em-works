;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this sample source code is subject to the terms of the Microsoft
; license agreement under which you licensed this sample source code. If
; you did not accept the terms of the license agreement, you are not
; authorized to use this sample source code. For the terms of the license,
; please see the license agreement between you and Microsoft or, if applicable,
; see the LICENSE.RTF on your install media or the root of your tools installation.
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
;
;//
;// Portions Copyright (c) Texas Instruments.  All rights reserved.
;//
;//------------------------------------------------------------------------------
;//
;//  File:  omap2420_dma.h
;//
;//  This header file is comprised of DMA module register details defined as
;//  structures and macros for configuring and controlling DMA module.
;//
;//------------------------------------------------------------------------------

;// Module      Base
;// Name        Address
;// DDMA        IOMA + 0xC000
;// CamDMA      0x48052800
;// SDMA        0x48056000

;//DMA4_COLOR register is valid only for SDMA module. It is a reserved section
;//in other modules(CAMDMA and DDMA)

OMAP2420_DDMA_DMA4_REVISION                EQU     0x0000
OMAP2420_DDMA_DMA4_IRQSTATUS_L0            EQU     0x0008
OMAP2420_DDMA_DMA4_IRQSTATUS_L1            EQU     0x000C
OMAP2420_DDMA_DMA4_IRQSTATUS_L2            EQU     0x0010
OMAP2420_DDMA_DMA4_IRQSTATUS_L3            EQU     0x0014
OMAP2420_DDMA_DMA4_IRQENABLE_L0            EQU     0x0018
OMAP2420_DDMA_DMA4_IRQENABLE_L1            EQU     0x001C
OMAP2420_DDMA_DMA4_IRQENABLE_L2            EQU     0x0020
OMAP2420_DDMA_DMA4_IRQENABLE_L3            EQU     0x0024
OMAP2420_DDMA_DMA4_SYSSTATUS               EQU     0x0028
OMAP2420_DDMA_DMA4_OCP_SYSCONFIG           EQU     0x002C
OMAP2420_DDMA_DMA4_CAPS_0                  EQU     0x0064
OMAP2420_DDMA_DMA4_CAPS_2                  EQU     0x006C
OMAP2420_DDMA_DMA4_CAPS_3                  EQU     0x0070
OMAP2420_DDMA_DMA4_CAPS_4                  EQU     0x0074
OMAP2420_DDMA_DMA4_GCR                     EQU     0x0078
OMAP2420_DDMA_CHNL_CTRL                    EQU     0x0080

OMAP2420_SDMA_DMA4_REVISION                EQU     0x0000
OMAP2420_SDMA_DMA4_IRQSTATUS_L0            EQU     0x0008
OMAP2420_SDMA_DMA4_IRQSTATUS_L1            EQU     0x000C
OMAP2420_SDMA_DMA4_IRQSTATUS_L2            EQU     0x0010
OMAP2420_SDMA_DMA4_IRQSTATUS_L3            EQU     0x0014
OMAP2420_SDMA_DMA4_IRQENABLE_L0            EQU     0x0018
OMAP2420_SDMA_DMA4_IRQENABLE_L1            EQU     0x001C
OMAP2420_SDMA_DMA4_IRQENABLE_L2            EQU     0x0020
OMAP2420_SDMA_DMA4_IRQENABLE_L3            EQU     0x0024
OMAP2420_SDMA_DMA4_SYSSTATUS               EQU     0x0028
OMAP2420_SDMA_DMA4_OCP_SYSCONFIG           EQU     0x002C
OMAP2420_SDMA_DMA4_CAPS_0                  EQU     0x0064
OMAP2420_SDMA_DMA4_CAPS_2                  EQU     0x006C
OMAP2420_SDMA_DMA4_CAPS_3                  EQU     0x0070
OMAP2420_SDMA_DMA4_CAPS_4                  EQU     0x0074
OMAP2420_SDMA_DMA4_GCR                     EQU     0x0078
OMAP2420_SDMA_CHNL_CTRL                    EQU     0x0080

OMAP2420_CAMDMA_DMA4_REVISION              EQU     0x0000
OMAP2420_CAMDMA_DMA4_IRQSTATUS_L0          EQU     0x0008
OMAP2420_CAMDMA_DMA4_IRQSTATUS_L1          EQU     0x000C
OMAP2420_CAMDMA_DMA4_IRQSTATUS_L2          EQU     0x0010
OMAP2420_CAMDMA_DMA4_IRQSTATUS_L3          EQU     0x0014
OMAP2420_CAMDMA_DMA4_IRQENABLE_L0          EQU     0x0018
OMAP2420_CAMDMA_DMA4_IRQENABLE_L1          EQU     0x001C
OMAP2420_CAMDMA_DMA4_IRQENABLE_L2          EQU     0x0020
OMAP2420_CAMDMA_DMA4_IRQENABLE_L3          EQU     0x0024
OMAP2420_CAMDMA_DMA4_SYSSTATUS             EQU     0x0028
OMAP2420_CAMDMA_DMA4_OCP_SYSCONFIG         EQU     0x002C
OMAP2420_CAMDMA_DMA4_CAPS_0                EQU     0x0064
OMAP2420_CAMDMA_DMA4_CAPS_2                EQU     0x006C
OMAP2420_CAMDMA_DMA4_CAPS_3                EQU     0x0070
OMAP2420_CAMDMA_DMA4_CAPS_4                EQU     0x0074
OMAP2420_CAMDMA_DMA4_GCR                   EQU     0x0078
OMAP2420_CAMDMA_CHNL_CTRL                  EQU     0x0080

    END
