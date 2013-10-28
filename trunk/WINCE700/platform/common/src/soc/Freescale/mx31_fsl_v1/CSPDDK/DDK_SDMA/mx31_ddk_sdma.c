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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  ddk_sdma.c
//
//  This file contains the SoC-specific DDK interface for the SDMA module.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"

#define SDMA_ROM_V2

#ifdef SDMA_ROM_V2
#include "sdma_script_code_v2.h"
#define SDMA_EXT_BD                 TRUE
#define SDMA_MEM2MEM_SCRIPT         ap_2_ap_ADDR
#else
#include "sdma_script_code.h"
#define SDMA_EXT_BD                 FALSE
#define SDMA_MEM2MEM_SCRIPT         mcu_2_mcu_ADDR
#endif


//-----------------------------------------------------------------------------
// External Functions
extern BOOL BSPSdmaBufIntMem(DDK_DMA_REQ dmaReq);

//-----------------------------------------------------------------------------
// External Variables 


//-----------------------------------------------------------------------------
// Defines 

// 8-word scratch area is required for each channel using mcu_2_mcu script
#define SDMA_DM_SCRATCH_WORDS       8
#define SDMA_DM_SCRATCH_SIZE        (SDMA_NUM_CHANNELS*SDMA_DM_SCRATCH_WORDS)
#define SDMA_DM_RAM_END             (SDMA_DM_RAM_START + (SDMA_RAM_SIZE/4))
#define SDMA_DM_SCATCH_START        (SDMA_DM_RAM_END - SDMA_DM_SCRATCH_SIZE)

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function:  SdmaLoadRamScripts
//
//  This function loads the SDMA RAM script image into the specified buffer.
//
//  Parameters:
//      pBuf
//          [out] - Points to buffer that will be filled with SDMA RAM 
//          contents.
//
//  Returns:
//      Returns the size in bytes of the SDMA RAM image.
//
//-----------------------------------------------------------------------------
UINT16 SdmaLoadRamScripts(void *pBuf)
{
    memcpy(pBuf, &sdma_code, RAM_CODE_SIZE<<1);
    return RAM_CODE_SIZE<<1;
}


//-----------------------------------------------------------------------------
//
//  Function:  SdmaSetChanDesc
//
//  This function fills the fields of the channel descriptor structure
//  based on the DMA request line specified.
//
//  Parameters:
//      dmaReq
//          [in] Specifies the DMA request line.
//
//      pChanDesc
//          [out] Points to channel descriptor structure to be updated.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SdmaSetChanDesc(DDK_DMA_REQ dmaReq, PSDMA_CHAN_DESC pChanDesc)
{
    BOOL rc = TRUE;

    // Assume the channel has DMA buffers in external memory
    pChanDesc->bBufIntMem = FALSE;
    
    switch(dmaReq)
    {

    case DDK_DMA_REQ_EXT0:
        pChanDesc->dmaMask = 1U << DMA_EVENT_EXTREQ0;
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_ATA_TX:
        pChanDesc->dmaMask = (1U <<  DMA_EVENT_ATA_TX) | (1U <<  DMA_EVENT_ATA_TXEND);
        // pChanDesc->perAddr = CSP_BASE_REG_PA_ATA_CTRL;
        pChanDesc->perAddr = 0x50020018; // ATA FIFO32 address in a way that SDMA will change it to the correct address
        pChanDesc->scriptAddr = mcu_2_ata_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_ATA_RX:
        pChanDesc->dmaMask = (1U <<  DMA_EVENT_ATA_RX) | (1U <<  DMA_EVENT_ATA_TXEND);
        // pChanDesc->dmaMask = (1U <<  DMA_EVENT_ATA_RX);
        // pChanDesc->perAddr = CSP_BASE_REG_PA_ATA_CTRL;
        pChanDesc->perAddr = 0x50020000; // ATA BASE address in a way that SDMA will change it to the correct address
        pChanDesc->scriptAddr = ata_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SIM1_RX0:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SIM;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SIM + 0x1A;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SIM1_TX0:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SIM;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SIM + 0x1C;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SIM1_RX1:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SIM;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SIM + 0x08;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SIM1_TX1:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SIM;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SIM + 0x06;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI2_RX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_CSPI2_RX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI2 + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI2_TX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_CSPI2_TX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI2 + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI1_RX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_CSPI1_UART3_RX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI1 + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI1_TX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_CSPI1_UART3_TX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI1 + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART3_RX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_CSPI1_UART3_RX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART3 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uartsh_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART3_TX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_CSPI1_UART3_TX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART3 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI3_RX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_CSPI3_UART5_RX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI3 + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI3_TX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_CSPI3_UART5_TX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI3 + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART5_RX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_CSPI3_UART5_RX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART5 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uart_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART5_TX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_CSPI3_UART5_TX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART5 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART4_RX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_UART4_RX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART4 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uart_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART4_TX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_UART4_TX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART4 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_EXT2:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_EXTREQ2;
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_EXT1:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_EXTREQ1;
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_UART2_RX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_UART2_FIRI_RX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART2 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uart_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART2_TX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_UART2_FIRI_TX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART2 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_FIRI_RX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_UART2_FIRI_RX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_FIRI;
        pChanDesc->scriptAddr = firi_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_FIRI_TX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_UART2_FIRI_TX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_FIRI + FIRI_TXFIFO_OFFSET;
        pChanDesc->scriptAddr = mcu_2_firi_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART1_RX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_UART1_RX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART1 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uart_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART1_TX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_UART1_TX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART1 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SDHC1_RX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SDHC1_MSHC1;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SDHC1 + SDHC_BUFFER_ACCESS_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SDHC1_TX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SDHC1_MSHC1;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SDHC1 + SDHC_BUFFER_ACCESS_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SDHC2_RX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SDHC2_MSHC2;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SDHC2 + SDHC_BUFFER_ACCESS_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SDHC2_TX:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SDHC2_MSHC2;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SDHC2 + SDHC_BUFFER_ACCESS_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_RX1:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SSI2_RX1;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_SRX1_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_TX1:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SSI2_TX1;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_STX1_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_RX0:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SSI2_RX0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_SRX0_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_TX0:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SSI2_TX0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_STX0_OFFSET;
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_RX1:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SSI1_RX1;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_SRX1_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_TX1:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SSI1_TX1;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_STX1_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_RX0:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SSI1_RX0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_SRX0_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_TX0:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_SSI1_TX0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_STX0_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_NFC:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_NANDFC;
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_ECT:
        pChanDesc->dmaMask = 1U <<  DMA_EVENT_ECT;
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_MEM2MEM:
        pChanDesc->dmaMask = DMA_EVENT_NONE;
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;

    default:
        rc = FALSE;
        break;

    }

#ifdef SDMA_ROM_V2
        // If this is a shared peripheral, call down to the platform code
        // to determine if the DMA buffers are located in internal memory 
        // thus allowing use of a script that does not access the burstDMA.
        if (pChanDesc->scriptAddr == mcu_2_shp_ADDR)
        {
            if (BSPSdmaBufIntMem(dmaReq))
            {
                pChanDesc->scriptAddr = per_2_shp_ADDR;
                pChanDesc->bBufIntMem = TRUE;

            }
        }
        else if (pChanDesc->scriptAddr == shp_2_mcu_ADDR)
        {
            if (BSPSdmaBufIntMem(dmaReq))
            {
                pChanDesc->scriptAddr = shp_2_per_ADDR;
                pChanDesc->bBufIntMem = TRUE;

            }
        }
#endif

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  SdmaUpdateChanDesc
//
//  This function updates the descriptor for a channel with a shared DMA event
//  so that one of the alternate DMA requests becomes active.
//
//  Parameters:
//      dmaReq
//          [in] - Specifies one of the possible DMA requests valid for
//                 a channel with a shared DMA event.  The DMA request
//                 must be one of the set of requests valid for the existing
//                 channel descriptor.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SdmaUpdateChanDesc(DDK_DMA_REQ dmaReq, PSDMA_CHAN_DESC pChanDesc)
{
    BOOL rc = FALSE;

    switch(dmaReq)
    {

    case DDK_DMA_REQ_SDHC1_RX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask & (1U << DMA_EVENT_SDHC1_MSHC1))
        {
            pChanDesc->scriptAddr = shp_2_mcu_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_SDHC1_TX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask & (1U <<  DMA_EVENT_SDHC1_MSHC1))
        {
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_SDHC2_RX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask & (1U << DMA_EVENT_SDHC2_MSHC2))
        {
            pChanDesc->scriptAddr = shp_2_mcu_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_SDHC2_TX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask & (1U << DMA_EVENT_SDHC2_MSHC2))
        {
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_SIM1_RX0:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask & (1U << DMA_EVENT_SIM))
        {
            pChanDesc->perAddr = CSP_BASE_REG_PA_SIM + 0x1A;
            pChanDesc->scriptAddr = shp_2_mcu_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_SIM1_TX0:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask & (1U << DMA_EVENT_SIM))
        {
            pChanDesc->perAddr = CSP_BASE_REG_PA_SIM + 0x1C;
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_SIM1_RX1:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask & (1U << DMA_EVENT_SIM))
        {
            pChanDesc->perAddr = CSP_BASE_REG_PA_SIM + 0x08;
            pChanDesc->scriptAddr = shp_2_mcu_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_SIM1_TX1:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask & (1U << DMA_EVENT_SIM))
        {
            pChanDesc->perAddr = CSP_BASE_REG_PA_SIM + 0x06;
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
            rc = TRUE;
        }
        break;
    }

    return rc;
}




//-----------------------------------------------------------------------------
//
//  Function:  SdmaSetChanContext
//
//  This function configures the parameters passed to the SDMA via
//  the channel context.
//
//  Parameters:
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SdmaSetChanContext(UINT8 chan, PSDMA_CHAN_DESC pChanDesc, UINT32 waterMark, 
    PSDMA_CHANNEL_CONTEXT pChanCtxt)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(chan);

    // Zero out the context
    memset(pChanCtxt, 0, sizeof(SDMA_CHANNEL_CONTEXT));

    switch(pChanDesc->scriptAddr)
    {

#ifdef SDMA_ROM_V2
    case ap_2_ap_ADDR:
        // GR7 = ARM M3 (external memory) start address
        pChanCtxt->GR[7] = CSP_BASE_MEM_PA_CSD0;
        break;
#else
    case mcu_2_mcu_ADDR:
        // GR7 = scratch area base
        pChanCtxt->GR[7] = SDMA_DM_SCATCH_START +(SDMA_DM_SCRATCH_WORDS*chan);
        break;
#endif

    case ata_2_mcu_ADDR:
    case mcu_2_ata_ADDR:
        // GR0 = ATA alarm event mask
        // GR1 = ATA transfer end alarm event mask
        // GR6 = peripheral address
        // GR7 = watermark level
        pChanCtxt->GR[0] = pChanDesc->dmaMask & (~(1U << DMA_EVENT_ATA_TXEND));
        pChanCtxt->GR[1] = (1U << (DMA_EVENT_ATA_TXEND));
        pChanCtxt->GR[6] = pChanDesc->perAddr;
        pChanCtxt->GR[7] = waterMark;
        break;

    default:
        // GR1 = event mask
        // GR6 = peripheral address
        // GR7 = watermark level
        pChanCtxt->GR[1] = pChanDesc->dmaMask;
        pChanCtxt->GR[6] = pChanDesc->perAddr;
        pChanCtxt->GR[7] = waterMark;
        break;

    }

    // PC_RPC = script address
    pChanCtxt->PC_RPC = pChanDesc->scriptAddr;

    return TRUE;
}
