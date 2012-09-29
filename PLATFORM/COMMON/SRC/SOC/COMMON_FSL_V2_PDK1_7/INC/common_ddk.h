//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  common_ddk.h
//
//  This header contains common DDK definitions that are common that are common 
//  to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_DDK_H
#define __COMMON_DDK_H

#include "common_sdma.h"

#if __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
//  Defines
#define DMA_EVENT_NONE      0xFF

//-----------------------------------------------------------------------------
//  Types

//-----------------------------------------------------------------------------
//
//  Type: DDK_GPIO_PORT
//
//  Specifies the GPIO module instance.  Note that not all instances will
//  be available on all platforms.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_GPIO_PORT1      = 0,
    DDK_GPIO_PORT2      = 1,
    DDK_GPIO_PORT3      = 2,
    DDK_GPIO_PORT4      = 3
} DDK_GPIO_PORT;


//-----------------------------------------------------------------------------
//
//  Type: DDK_GPIO_DIR
//
//  Specifies the direction the GPIO pins.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_GPIO_DIR_IN     = 0,
    DDK_GPIO_DIR_OUT    = 1
} DDK_GPIO_DIR;


//-----------------------------------------------------------------------------
//
//  Type: DDK_GPIO_INTR
//
//  Specifies the detection logic used for generating GPIO interrupts.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_GPIO_INTR_LOW_LEV   = 0,
    DDK_GPIO_INTR_HIGH_LEV  = 1,
    DDK_GPIO_INTR_RISE_EDGE = 2,
    DDK_GPIO_INTR_FALL_EDGE = 3,
    DDK_GPIO_INTR_BOTH_EDGE = 4,    // Unavailable on MX31/MX32
    DDK_GPIO_INTR_NONE      = 5
} DDK_GPIO_INTR;

//-----------------------------------------------------------------------------
//
//  Type: DDK_DMA_ACCESS
//
//  Specifies width of the data for a peripheral DMA transfer.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_DMA_ACCESS_32BIT = 0,
    DDK_DMA_ACCESS_8BIT  = 1,
    DDK_DMA_ACCESS_16BIT = 2,
    DDK_DMA_ACCESS_24BIT = 3
} DDK_DMA_ACCESS;

//-----------------------------------------------------------------------------
//
//  Type: DDK_DMA_FLAGS
//
//  Specifies mode flags within the buffer descriptor.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_DMA_FLAGS_BUSY = (1 << SDMA_MODE_DONE_LSH),
    DDK_DMA_FLAGS_WRAP = (1 << SDMA_MODE_WRAP_LSH),
    DDK_DMA_FLAGS_CONT = (1 << SDMA_MODE_CONT_LSH),
    DDK_DMA_FLAGS_INTR = (1 << SDMA_MODE_INTR_LSH), 
    DDK_DMA_FLAGS_ERROR = (1 << SDMA_MODE_ERROR_LSH)
} DDK_DMA_FLAGS;

//-----------------------------------------------------------------------------
//
//  Type: DDK_DMA_REQ
//
//  Specifies DMA request used to trigger SDMA channel execution.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_DMA_REQ_EXT0,
    DDK_DMA_REQ_EXT1,
    DDK_DMA_REQ_EXT2,
    DDK_DMA_REQ_CCM,
    DDK_DMA_REQ_ECT,
    DDK_DMA_REQ_CSPI1_RX,
    DDK_DMA_REQ_CSPI1_TX,
    DDK_DMA_REQ_CSPI2_RX,
    DDK_DMA_REQ_CSPI2_TX,
    DDK_DMA_REQ_CSPI3_RX,
    DDK_DMA_REQ_CSPI3_TX,
    DDK_DMA_REQ_UART1_RX,
    DDK_DMA_REQ_UART1_TX,
    DDK_DMA_REQ_UART2_RX,
    DDK_DMA_REQ_UART2_TX,
    DDK_DMA_REQ_UART3_RX,
    DDK_DMA_REQ_UART3_TX,
    DDK_DMA_REQ_UART4_RX,
    DDK_DMA_REQ_UART4_TX,
    DDK_DMA_REQ_UART5_RX,
    DDK_DMA_REQ_UART5_TX,
    DDK_DMA_REQ_FIRI_RX,
    DDK_DMA_REQ_FIRI_TX,
    DDK_DMA_REQ_MSHC1_RX,
    DDK_DMA_REQ_MSHC1_TX,
    DDK_DMA_REQ_SDHC1_RX,
    DDK_DMA_REQ_SDHC1_TX,
    DDK_DMA_REQ_SDHC2_RX,
    DDK_DMA_REQ_SDHC2_TX,
    DDK_DMA_REQ_SDHC3_RX,
    DDK_DMA_REQ_SDHC3_TX,
    DDK_DMA_REQ_SDHC4_RX,
    DDK_DMA_REQ_SDHC4_TX,
    DDK_DMA_REQ_SSI1_RX0,
    DDK_DMA_REQ_SSI1_TX0,
    DDK_DMA_REQ_SSI1_RX1,
    DDK_DMA_REQ_SSI1_TX1,
    DDK_DMA_REQ_SSI2_RX0,
    DDK_DMA_REQ_SSI2_TX0,
    DDK_DMA_REQ_SSI2_RX1,
    DDK_DMA_REQ_SSI2_TX1,
    DDK_DMA_REQ_SSI3_RX0,
    DDK_DMA_REQ_SSI3_TX0,
    DDK_DMA_REQ_SSI3_RX1,
    DDK_DMA_REQ_SSI3_TX1, 
    //DDK_DMA_REQ_NFC,
    DDK_DMA_REQ_NFC_READ,
    DDK_DMA_REQ_NFC_WRITE,
    DDK_DMA_REQ_SIM1_RX0,
    DDK_DMA_REQ_SIM1_TX0,
    DDK_DMA_REQ_SIM1_RX1,
    DDK_DMA_REQ_SIM1_TX1,
    DDK_DMA_REQ_SIM2_RX0,
    DDK_DMA_REQ_SIM2_TX0,
    DDK_DMA_REQ_SIM2_RX1,
    DDK_DMA_REQ_SIM2_TX1,
    DDK_DMA_REQ_USB_FUNC,
    DDK_DMA_REQ_USB_GROUP,
    DDK_DMA_REQ_ATA_RX,
    DDK_DMA_REQ_ATA_TX,
    DDK_DMA_REQ_ATA_TXEND,
    DDK_DMA_REQ_SPDIF_RX,
    DDK_DMA_REQ_SPDIF_TX,
    DDK_DMA_REQ_ESAI_RX,
    DDK_DMA_REQ_ESAI_TX,
    DDK_DMA_REQ_ASRC_RXA,
    DDK_DMA_REQ_ASRC_TXA,
    DDK_DMA_REQ_ASRC_RXB,
    DDK_DMA_REQ_ASRC_TXB,
    DDK_DMA_REQ_ASRC_RXC,
    DDK_DMA_REQ_ASRC_TXC,
    DDK_DMA_REQ_HSI2C_TX,
    DDK_DMA_REQ_HSI2C_RX,
    DDK_DMA_REQ_I2C1_TX,
    DDK_DMA_REQ_I2C1_RX,
    DDK_DMA_REQ_I2C2_TX,
    DDK_DMA_REQ_I2C2_RX,
    DDK_DMA_REQ_GPU,
    DDK_DMA_REQ_VPU,
    DDK_DMA_REQ_SLM_TX,
    DDK_DMA_REQ_SLM_FIFO0_TX,
    DDK_DMA_REQ_SLM_FIFO0_RX,
    DDK_DMA_REQ_SLM_FIFO1_TX,
    DDK_DMA_REQ_SLM_FIFO1_RX,
    DDK_DMA_REQ_SLM_FIFO2_TX,
    DDK_DMA_REQ_SLM_FIFO3_TX,
    DDK_DMA_REQ_SLM_FIFO4_TX,
    DDK_DMA_REQ_EXTMEM2IPURAM,
    DDK_DMA_REQ_EXTMEM2IRAM,
    DDK_DMA_REQ_EXTMEM2EXTMEM,
    DDK_DMA_REQ_ASRC_TXA_2_ESAI_TX, 
    DDK_DMA_REQ_ASRC_TXB_2_ESAI_TX, 
    DDK_DMA_REQ_ASRC_TXC_2_ESAI_TX
} DDK_DMA_REQ;


//------------------------------------------------------------------------------
// Functions
BOOL DDKGpioSetConfig(DDK_GPIO_PORT port, UINT32 pin, DDK_GPIO_DIR dir, DDK_GPIO_INTR intr);
BOOL DDKGpioWriteData(DDK_GPIO_PORT port, UINT32 portMask, UINT32 data);
BOOL DDKGpioWriteDataPin(DDK_GPIO_PORT port, UINT32 pin, UINT32 data);
BOOL DDKGpioReadData(DDK_GPIO_PORT port, UINT32 portMask, UINT32 *pData);
BOOL DDKGpioReadDataPin(DDK_GPIO_PORT port, UINT32 pin, UINT32 *pData);
BOOL DDKGpioReadIntr(DDK_GPIO_PORT port, UINT32 portMask, UINT32 *pStatus);
BOOL DDKGpioReadIntrPin(DDK_GPIO_PORT port, UINT32 pin, UINT32 *pStatus);
BOOL DDKGpioClearIntrPin(DDK_GPIO_PORT port, UINT32 pin);


UINT8 DDKSdmaOpenChan(DDK_DMA_REQ dmaReq, UINT8 priority);
BOOL DDKSdmaUpdateSharedChan(UINT8 chan, DDK_DMA_REQ dmaReq);
BOOL DDKSdmaCloseChan(UINT8 chan);
BOOL DDKSdmaAllocChain(UINT8 chan, UINT32 numBufDesc);
BOOL DDKSdmaFreeChain(UINT8 chan);
BOOL DDKSdmaSetBufDesc(UINT8 chan, UINT32 index, 
    UINT32 modeFlags, UINT32 memAddr1PA, UINT32 memAddr2PA, 
    DDK_DMA_ACCESS dataWidth, UINT16 numBytes);
BOOL DDKSdmaGetBufDescStatus(UINT8 chan, UINT32 index, UINT32 *pStatus);
BOOL DDKSdmaGetChainStatus(UINT8 chan, UINT32 *pStatus);
BOOL DDKSdmaClearBufDescStatus(UINT8 chan, UINT32 index);
BOOL DDKSdmaClearChainStatus(UINT8 chan);
BOOL DDKSdmaInitChain(UINT8 chan, UINT32 waterMark);
BOOL DDKSdmaStartChan(UINT8 chan);
BOOL DDKSdmaStopChan(UINT8 chan, BOOL bKill);

#if __cplusplus
}
#endif

#endif // __COMMON_DDK_H
