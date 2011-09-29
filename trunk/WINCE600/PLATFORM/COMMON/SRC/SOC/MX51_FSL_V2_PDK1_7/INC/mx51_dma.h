//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mx51_dma.h
//
//  This header contains DMA definitions.
//
//------------------------------------------------------------------------------
#ifndef __MX51_DMA_H
#define __MX51_DMA_H

//------------------------------------------------------------------------------
// SDMA EVENT DEFINITIONS
//------------------------------------------------------------------------------
#define DMA_EVENT_VPU               0
#define DMA_EVENT_GPC               1
#define DMA_EVENT_PATA_RX           2
#define DMA_EVENT_PATA_TX           3
#define DMA_EVENT_PATA_TFER_END     4
#define DMA_EVENT_SLIMBUS           5
#define DMA_EVENT_ECSPI1_RX         6
#define DMA_EVENT_ECSPI1_TX         7
#define DMA_EVENT_ECSPI2_RX         8
#define DMA_EVENT_ECSPI2_TX         9
#define DMA_EVENT_HSI2C_TX          10
#define DMA_EVENT_HSI2C_RX          11
#define DMA_EVENT_FIRI_RX           12
#define DMA_EVENT_FIRI_TX           13
#define DMA_EVENT_IOMUX_GPIO1_4     14
#define DMA_EVENT_IOMUX_GPIO1_5     15
#define DMA_EVENT_UART2_RX          16
#define DMA_EVENT_UART2_TX          17
#define DMA_EVENT_UART1_RX          18
#define DMA_EVENT_UART1_TX          19
#define DMA_EVENT_ESDHC1_I2C1       20
#define DMA_EVENT_ESDHC2_I2C2       21
#define DMA_EVENT_SSI2_RX1          22
#define DMA_EVENT_SSI2_TX1_SLM_TX3  23
#define DMA_EVENT_SSI2_RX0_SLM_RX2  24
#define DMA_EVENT_SSI2_TX0_SLM_TX2  25
#define DMA_EVENT_SSI1_RX1          26
#define DMA_EVENT_SSI1_TX1          27
#define DMA_EVENT_SSI1_RX0          28
#define DMA_EVENT_SSI1_TX0          29
#define DMA_EVENT_EMIV2_READ        30
#define DMA_EVENT_CTI2_0            31
#define DMA_EVENT_EMIV2_WRITE       32
#define DMA_EVENT_CTI2_1            33
#define DMA_EVENT_EPIT2             34
#define DMA_EVENT_SSI3_RX1_SLM_RX1  35
#define DMA_EVENT_IPUV3E            36
#define DMA_EVENT_SSI3_TX1_SLM_TX1  37
#define DMA_EVENT_CSPI_RX           38
#define DMA_EVENT_CSPI_TX           39
#define DMA_EVENT_ESDHC3            40
#define DMA_EVENT_ESDHC4            41
#define DMA_EVENT_SLM_TX4           42
#define DMA_EVENT_UART3_RX          43
#define DMA_EVENT_UART3_TX          44
#define DMA_EVENT_SPDIF             45
#define DMA_EVENT_SSI3_RX0_SLM_RX0  46
#define DMA_EVENT_SSI3_TX0_SLM_TX0  47

#endif // __MX51_DMA_H
