//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_ddk_sdma.c
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

#include "sdma_script_code.h"
#define SDMA_EXT_BD                 TRUE
#define SDMA_MEM2MEM_SCRIPT         ap_2_ap_ADDR


//-----------------------------------------------------------------------------
// External Functions
extern BOOL BSPSdmaBufIntMem(DDK_DMA_REQ dmaReq);
extern UINT32 BSPSdmaGetM3BaseAddr(void);


//-----------------------------------------------------------------------------
// External Variables 


//-----------------------------------------------------------------------------
// Defines 


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
const UINT32 g_pSdmaBaseRegPA = CSP_BASE_REG_PA_SDMA;


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
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_EXTREQ0;
        pChanDesc->dmaMask[1] = 0;      
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_ATA_TX:
        pChanDesc->dmaMask[0] = (1U << DMA_EVENT_ATA_TX) | (1U << DMA_EVENT_ATA_TXEND);
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_ATA + ATA_FIFO_DATA_32_OFFSET;
        pChanDesc->scriptAddr = mcu_2_ata_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_ATA_RX:
        pChanDesc->dmaMask[0] = (1U << DMA_EVENT_ATA_RX) | (1U << DMA_EVENT_ATA_TXEND);
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_ATA;
        pChanDesc->scriptAddr = ata_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI2_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_CSPI2_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI2 + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI2_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_CSPI2_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI2 + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI1_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_CSPI1_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI1 + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI1_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_CSPI1_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI1 + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART3_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART3_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART3 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uartsh_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART3_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART3_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART3 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
    case DDK_DMA_REQ_UART4_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART4_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART4 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uartsh_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART4_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART4_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART4 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
	//
	// CS&ZHL Jun 09-2011: add DMA request for UART5
	//
    case DDK_DMA_REQ_UART5_RX:
        pChanDesc->dmaMask[0] = 0; 
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_UART5_RX - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART5 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uartsh_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART5_TX:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << ( DMA_EVENT_UART5_TX - 32 );
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART5 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_EXT1:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_EXTREQ1;
        pChanDesc->dmaMask[1] = 0;      
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_EXT2:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_EXTREQ2;
        pChanDesc->dmaMask[1] = 0;      
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_UART2_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART2_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART2 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uart_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART2_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART2_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART2 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART1_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART1_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART1 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uart_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART1_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART1_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART1 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_RX1:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI2_RX1;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_SRX1_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_TX1:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI2_TX1;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_STX1_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_RX0:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI2_RX0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_SRX0_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_TX0:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI2_TX0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_STX0_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_RX1:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI1_RX1;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_SRX1_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_TX1:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI1_TX1;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_STX1_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_RX0:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI1_RX0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_SRX0_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_TX0:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI1_TX0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_STX0_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    /*case DDK_DMA_REQ_CSPI3_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_CSPI2_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI2 + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI3_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_CSPI2_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI2 + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;*/
    // lqk OCT-13 2011 Fix DMA request for CSPI3
	case DDK_DMA_REQ_CSPI3_RX:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_CSPI3_RX-32);

		pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI3 + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
		//pChanDesc->scriptAddr = shp_2_mcu_ADDR;
		//pChanDesc->scriptAddr = uartsh_2_mcu_ADDR;
		//pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI3_TX:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_CSPI3_TX-32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI3 + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
		//pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
	// end  lqk OCT-13 2011       
    case DDK_DMA_REQ_EXTMEM2IPURAM:
    case DDK_DMA_REQ_EXTMEM2IRAM:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = BSPSdmaGetM3BaseAddr();
        pChanDesc->scriptAddr = ext_mem__ipu_ram_ADDR;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_EXTMEM2EXTMEM:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = ap_2_ap_ADDR;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_ESAI_RX:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ESAI_RX-32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESAI+ ESAI_ERDR_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_ESAI_TX:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ESAI_TX - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESAI + ESAI_ETDR_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;    

    default:
        rc = FALSE;
        break;
    }

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
//          [in] Specifies one of the possible DMA requests valid for
//               a channel with a shared DMA event.  The DMA request
//               must be one of the set of requests valid for the existing
//               channel descriptor.
//
//      pChanDesc
//          [in] The SDMA chanel descriptor.
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL SdmaUpdateChanDesc(DDK_DMA_REQ dmaReq, PSDMA_CHAN_DESC pChanDesc)
{
    // For now, there is no event sharing cases
    UNREFERENCED_PARAMETER(dmaReq);
    UNREFERENCED_PARAMETER(pChanDesc);

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  SdmaSetChanContext
//
//  This function configures the parameters passed to the SDMA via
//  the channel context.
//
//  Parameters:
//      chan
//          [in] Chanel number
//
//      pChanDesc
//          [in] SDMA chanel descriptor
//
//      waterMark
//          [in] water mark
//
//      pChanCtxt
//          [in/out] SDMA channel context
//
//
//  Returns:
//      Returns TRUE.
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
    case ap_2_ap_ADDR:
        // None
        break;

    case ext_mem__ipu_ram_ADDR:
        // GR7 = M3 start address
        pChanCtxt->GR[7] = pChanDesc->perAddr;
        break;

    case ata_2_mcu_ADDR:
    case mcu_2_ata_ADDR:
        // GR0 = ATA alarm event mask
        // GR1 = ATA transfer end alarm event mask
        // GR6 = peripheral address
        // GR7 = watermark level
        pChanCtxt->GR[0] = pChanDesc->dmaMask[0] & (~(1U << DMA_EVENT_ATA_TXEND));
        pChanCtxt->GR[1] = (1U << (DMA_EVENT_ATA_TXEND));
        pChanCtxt->GR[6] = pChanDesc->perAddr;
        pChanCtxt->GR[7] = waterMark;
        break;

    default:
        // GR0 = event (>=32) mask
        // GR1 = event (<32) mask
        // GR6 = peripheral address
        // GR7 = watermark level
        pChanCtxt->GR[0] = pChanDesc->dmaMask[1];
        pChanCtxt->GR[1] = pChanDesc->dmaMask[0];
        pChanCtxt->GR[6] = pChanDesc->perAddr;
        pChanCtxt->GR[7] = waterMark;
        break;
    }

    // PC_RPC = script address
    pChanCtxt->PC_RPC = pChanDesc->scriptAddr;

    return TRUE;
}
