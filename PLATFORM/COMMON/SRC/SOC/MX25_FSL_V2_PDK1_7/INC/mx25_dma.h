//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mx25_dma.h
//
//  This header contains DMA definitions for the MX25 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX25_DMA_H
#define __MX25_DMA_H

//------------------------------------------------------------------------------
// SDMA EVENT DEFINITIONS
//------------------------------------------------------------------------------
#define DMA_EVENT_EXTREQ0           0
#define DMA_EVENT_DVFC              1
#define DMA_EVENT_ATA_TXEND         2
#define DMA_EVENT_ATA_TX            3
#define DMA_EVENT_ATA_RX            4
#define DMA_EVENT_RESERVED0         5
#define DMA_EVENT_CSPI2_RX          6
#define DMA_EVENT_CSPI2_TX          7
#define DMA_EVENT_CSPI1_RX          8
#define DMA_EVENT_CSPI1_TX          9
#define DMA_EVENT_UART3_RX          10
#define DMA_EVENT_UART3_TX          11
#define DMA_EVENT_UART4_RX          12
#define DMA_EVENT_UART4_TX          13
#define DMA_EVENT_EXTREQ1           14
#define DMA_EVENT_EXTREQ2           15
#define DMA_EVENT_UART2_RX          16
#define DMA_EVENT_UART2_TX          17
#define DMA_EVENT_UART1_RX          18
#define DMA_EVENT_UART1_TX          19
#define DMA_EVENT_FEC               20
#define DMA_EVENT_RESERVED1         21
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
#define DMA_EVENT_ESAI_RX           32
#define DMA_EVENT_ESAI_TX           33
#define DMA_EVENT_CSPI3_RX          34
#define DMA_EVENT_CSPI3_TX          35
#define DMA_EVENT_SIM2_RX           36
#define DMA_EVENT_SIM2_TX           37
#define DMA_EVENT_SIM1_RX           38
#define DMA_EVENT_SIM1_TX           39
#define DMA_EVENT_RESERVED2         40
#define DMA_EVENT_RESERVED3         41
#define DMA_EVENT_CSI_RX            42
#define DMA_EVENT_CSI_ST            43
#define DMA_EVENT_TCH               44
#define DMA_EVENT_ADCTL             45
#define DMA_EVENT_UART5_RX          46
#define DMA_EVENT_UART5_TX          47
#define DMA_EVENT_RESERVED4         48

#endif // __MX25_DMA_H
