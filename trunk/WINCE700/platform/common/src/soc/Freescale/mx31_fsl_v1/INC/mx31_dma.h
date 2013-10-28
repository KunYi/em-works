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
//  File:  MX31_dma.h
//
//  This header contains DMA definitions for the MX31 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX31_DMA_H
#define __MX31_DMA_H

//------------------------------------------------------------------------------
// SDMA EVENT DEFINITIONS
//------------------------------------------------------------------------------
#define DMA_EVENT_EXTREQ0           0
#define DMA_EVENT_CCM               1
#define DMA_EVENT_ATA_TXEND         2
#define DMA_EVENT_ATA_TX            3
#define DMA_EVENT_ATA_RX            4
#define DMA_EVENT_SIM               5
#define DMA_EVENT_CSPI2_RX          6
#define DMA_EVENT_CSPI2_TX          7
#define DMA_EVENT_CSPI1_UART3_RX    8
#define DMA_EVENT_CSPI1_UART3_TX    9
#define DMA_EVENT_CSPI3_UART5_RX    10
#define DMA_EVENT_CSPI3_UART5_TX    11
#define DMA_EVENT_UART4_RX          12
#define DMA_EVENT_UART4_TX          13
#define DMA_EVENT_EXTREQ1           14
#define DMA_EVENT_EXTREQ2           15
#define DMA_EVENT_UART2_FIRI_RX     16
#define DMA_EVENT_UART2_FIRI_TX     17
#define DMA_EVENT_UART1_RX          18
#define DMA_EVENT_UART1_TX          19
#define DMA_EVENT_SDHC1_MSHC1       20
#define DMA_EVENT_SDHC2_MSHC2       21
#define DMA_EVENT_SSI2_RX1          22
#define DMA_EVENT_SSI2_TX1          23
#define DMA_EVENT_SSI2_RX0          24
#define DMA_EVENT_SSI2_TX0          25
#define DMA_EVENT_SSI1_RX1          26
#define DMA_EVENT_SSI1_TX1          27
#define DMA_EVENT_SSI1_RX0          28
#define DMA_EVENT_SSI1_TX0          29
#define DMA_EVENT_NANDFC            30
#define DMA_EVENT_ECT               31

#endif // __MX31_DMA_H
