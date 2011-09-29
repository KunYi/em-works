//-----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

#include "sdma_script_code.h"
#define SDMA_EXT_BD                 TRUE


//-----------------------------------------------------------------------------
// External Functions
extern BOOL BSPSdmaBufIntMem(DDK_DMA_REQ dmaReq);
extern UINT32 BSPSdmaGetM3BaseAddr(void);


//-----------------------------------------------------------------------------
// External Variables 
extern UINT32 g_SiRev;

//-----------------------------------------------------------------------------
// Defines 


//-----------------------------------------------------------------------------
// Types
typedef struct
{
    UINT32 m_ap_2_ap_ADDR;
    UINT32 m_app_2_mcu_ADDR;
    UINT32 m_mcu_2_app_ADDR;
    UINT32 m_uart_2_mcu_ADDR;
    UINT32 m_shp_2_mcu_ADDR;
    UINT32 m_mcu_2_shp_ADDR;
    UINT32 m_per_2_shp_ADDR;
    UINT32 m_shp_2_per_ADDR;
    UINT32 m_uartsh_2_mcu_ADDR;
    UINT32 m_mcu_2_ata_ADDR;
    UINT32 m_ata_2_mcu_ADDR;
    UINT32 m_app_2_per_ADDR;
    UINT32 m_per_2_app_ADDR;
    UINT32 m_asrc__mcu_ADDR;
    UINT32 m_ext_mem__ipu_ram_ADDR;
    UINT32 m_mcu_2_spdif_ADDR;
    UINT32 m_p_2_p_ADDR;
    UINT32 m_spdif_2_mcu_ADDR;
    UINT32 m_uart_2_per_ADDR;
    UINT32 m_uartsh_2_per_ADDR;
    UINT16 m_ram_code_size;
    const short * m_sdma_code;
} SDMA_SCRIPT_INFO;


//-----------------------------------------------------------------------------
// Global Variables
const UINT32 g_pSdmaBaseRegPA = CSP_BASE_REG_PA_SDMA;


//-----------------------------------------------------------------------------
// Local Variables
static SDMA_SCRIPT_INFO ScriptInfo;


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
    memcpy(pBuf, ScriptInfo.m_sdma_code, ScriptInfo.m_ram_code_size << 1);
    return (ScriptInfo.m_ram_code_size << 1);
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
    case DDK_DMA_REQ_ATA_TX:
        pChanDesc->dmaMask[0] = (1U << DMA_EVENT_ATA_TX) | (1U << DMA_EVENT_ATA_TXEND);
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_ATA + ATA_FIFO_DATA_32_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_ata_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_ATA_RX:
        pChanDesc->dmaMask[0] = (1U << DMA_EVENT_ATA_RX) | (1U << DMA_EVENT_ATA_TXEND);
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_ATA;
        pChanDesc->scriptAddr = ScriptInfo.m_ata_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI2_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_CSPI2_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI2 + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI2_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_CSPI2_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI2 + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI1_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_CSPI1_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI1 + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_CSPI1_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_CSPI1_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI1 + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART3_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART3_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART3 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_uartsh_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART3_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART3_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART3 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART2_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART2_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART2 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_uart_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART2_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART2_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART2 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART1_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART1_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART1 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_uart_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART1_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_UART1_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART1 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_RX1:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI2_RX1;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_SRX1_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_TX1:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI2_TX1;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_STX1_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_RX0:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI2_RX0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_SRX0_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_TX0:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI2_TX0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_STX0_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_RX1:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI1_RX1;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_SRX1_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_TX1:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI1_TX1;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_STX1_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_RX0:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI1_RX0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_SRX0_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI1_TX0:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI1_TX0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI1 + SSI_STX0_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SPDIF_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SPDIF_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SPDIF + SPDIF_SRL_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_spdif_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SPDIF_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SPDIF_TX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SPDIF + SPDIF_STL_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_spdif_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_EXTMEM2IPURAM:
    case DDK_DMA_REQ_EXTMEM2IRAM:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = BSPSdmaGetM3BaseAddr();
        pChanDesc->scriptAddr = ScriptInfo.m_ext_mem__ipu_ram_ADDR;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_EXTMEM2EXTMEM:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = ScriptInfo.m_ap_2_ap_ADDR;
        pChanDesc->bExtended = TRUE;
        break;

    case DDK_DMA_REQ_ASRC_RXA:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ASRC_RXA-32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ASRC + ASRC_ASRDIA_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_ASRC_TXA:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ASRC_TXA - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ASRC + ASRC_ASRDOA_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;        

    case DDK_DMA_REQ_ASRC_RXB:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ASRC_RXB-32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ASRC + ASRC_ASRDIB_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;    

    case DDK_DMA_REQ_ASRC_TXB:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ASRC_TXB - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ASRC + ASRC_ASRDOB_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;        

    case DDK_DMA_REQ_ASRC_RXC:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ASRC_RXC-32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ASRC + ASRC_ASRDIC_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;    

    case DDK_DMA_REQ_ASRC_TXC:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ASRC_TXC - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ASRC + ASRC_ASRDOC_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;        

    case DDK_DMA_REQ_ESAI_RX:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ESAI_RX-32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESAI+ ESAI_ERDR_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_ESAI_TX:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ESAI_TX - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESAI + ESAI_ETDR_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;                
        
    case DDK_DMA_REQ_ASRC_TXA_2_ESAI_TX:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ESAI_TX-32);
        pChanDesc->dmaMask[1] |= 1U << (DMA_EVENT_ASRC_TXA - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ASRC + ASRC_ASRDOA_OFFSET;
        pChanDesc->perAddr2 = CSP_BASE_REG_PA_ESAI+ ESAI_ETDR_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_p_2_p_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;                
        
    case DDK_DMA_REQ_ASRC_TXB_2_ESAI_TX:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ESAI_TX-32);
        pChanDesc->dmaMask[1] |= 1U << (DMA_EVENT_ASRC_TXB - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ASRC + ASRC_ASRDOB_OFFSET;
        pChanDesc->perAddr2 = CSP_BASE_REG_PA_ESAI+ ESAI_ETDR_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_p_2_p_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;                
        
    case DDK_DMA_REQ_ASRC_TXC_2_ESAI_TX:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ESAI_TX-32);
        pChanDesc->dmaMask[1] |= 1U << (DMA_EVENT_ASRC_TXC - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ASRC + ASRC_ASRDOC_OFFSET;
        pChanDesc->perAddr2 = CSP_BASE_REG_PA_ESAI+ ESAI_ETDR_OFFSET;
        pChanDesc->scriptAddr = ScriptInfo.m_p_2_p_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;                
        
    default:
        rc = FALSE;
        break;
    }

    // If this is a shared peripheral, call down to the platform code
    // to determine if the DMA buffers are located in internal memory 
    // thus allowing use of a script that does not access the burstDMA.
    if (pChanDesc->scriptAddr == ScriptInfo.m_mcu_2_shp_ADDR)
    {
        if (BSPSdmaBufIntMem(dmaReq))
        {
            pChanDesc->scriptAddr = ScriptInfo.m_per_2_shp_ADDR;
            pChanDesc->bBufIntMem = TRUE;
        }
    }
    else if (pChanDesc->scriptAddr == ScriptInfo.m_shp_2_mcu_ADDR)
    {
        if (BSPSdmaBufIntMem(dmaReq))
        {
            pChanDesc->scriptAddr = ScriptInfo.m_shp_2_per_ADDR;
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
    // For now, there is no event sharing cases
    // like MX31 SDHC/SIM on MX35.
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

    if (pChanDesc->scriptAddr == ScriptInfo.m_ap_2_ap_ADDR)
    {
        // None
    }
    else if (pChanDesc->scriptAddr == ScriptInfo.m_ext_mem__ipu_ram_ADDR)
    {
        // GR7 = M3 start address
        pChanCtxt->GR[7] = pChanDesc->perAddr;
    }
    else if ((pChanDesc->scriptAddr == ScriptInfo.m_ata_2_mcu_ADDR) || 
             (pChanDesc->scriptAddr == ScriptInfo.m_mcu_2_ata_ADDR))
    {
        // GR0 = ATA alarm event mask
        // GR1 = ATA transfer end alarm event mask
        // GR6 = peripheral address
        // GR7 = watermark level
        pChanCtxt->GR[0] = pChanDesc->dmaMask[0] & (~(1U << DMA_EVENT_ATA_TXEND));
        pChanCtxt->GR[1] = (1U << (DMA_EVENT_ATA_TXEND));
        pChanCtxt->GR[6] = pChanDesc->perAddr;
        pChanCtxt->GR[7] = waterMark;
    }
    else if (pChanDesc->scriptAddr == ScriptInfo.m_p_2_p_ADDR)
    {
        switch (pChanDesc->dmaMask[1])
        {
        case (1U<<(DMA_EVENT_ASRC_TXA-32))|(1U<<(DMA_EVENT_ESAI_TX-32)):
            // ASRC TXA to ESAI
            pChanCtxt->GR[0] = 1U << (DMA_EVENT_ESAI_TX-32);
            pChanCtxt->GR[1] = 1U << (DMA_EVENT_ASRC_TXA-32);
            pChanCtxt->GR[2] = pChanDesc->perAddr;
            pChanCtxt->GR[6] = pChanDesc->perAddr2;
            pChanCtxt->GR[7] = waterMark;
            break;

        case (1U<<(DMA_EVENT_ASRC_TXB-32))|(1U<<(DMA_EVENT_ESAI_TX-32)):
            // ASRC TXB to ESAI
            pChanCtxt->GR[0] = 1U << (DMA_EVENT_ESAI_TX-32);
            pChanCtxt->GR[1] = 1U << (DMA_EVENT_ASRC_TXB-32);
            pChanCtxt->GR[2] = pChanDesc->perAddr;
            pChanCtxt->GR[6] = pChanDesc->perAddr2;
            pChanCtxt->GR[7] = waterMark;
            break;

        case (1U<<(DMA_EVENT_ASRC_TXC-32))|(1U<<(DMA_EVENT_ESAI_TX-32)):
            // ASRC TXC to ESAI
            pChanCtxt->GR[0] = 1U << (DMA_EVENT_ESAI_TX-32);
            pChanCtxt->GR[1] = 1U << (DMA_EVENT_ASRC_TXC-32);
            pChanCtxt->GR[2] = pChanDesc->perAddr;
            pChanCtxt->GR[6] = pChanDesc->perAddr2;
            pChanCtxt->GR[7] = waterMark;
            break;

        default:
            break;
        }
    }
    else
    {
        // GR0 = event (>=32) mask
        // GR1 = event (<32) mask
        // GR6 = peripheral address
        // GR7 = watermark level
        pChanCtxt->GR[0] = pChanDesc->dmaMask[1];
        pChanCtxt->GR[1] = pChanDesc->dmaMask[0];
        pChanCtxt->GR[6] = pChanDesc->perAddr;
        pChanCtxt->GR[7] = waterMark;
    }

    // PC_RPC = script address
    pChanCtxt->PC_RPC = pChanDesc->scriptAddr;

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  SocSdmaInit
//
//  This function initializes the SDMA in SoC level.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SocSdmaInit(void)
{
    // Pick up script info as per the silicon revision
    if (g_SiRev == DDK_SI_REV_TO1)
    {
        ScriptInfo.m_ap_2_ap_ADDR = ap_2_ap_ADDR_TO1;
        ScriptInfo.m_app_2_mcu_ADDR = app_2_mcu_ADDR_TO1;
        ScriptInfo.m_mcu_2_app_ADDR = mcu_2_app_ADDR_TO1;
        ScriptInfo.m_uart_2_mcu_ADDR = uart_2_mcu_ADDR_TO1;
        ScriptInfo.m_shp_2_mcu_ADDR = shp_2_mcu_ADDR_TO1;
        ScriptInfo.m_mcu_2_shp_ADDR = mcu_2_shp_ADDR_TO1;
        ScriptInfo.m_per_2_shp_ADDR = per_2_shp_ADDR_TO1;
        ScriptInfo.m_shp_2_per_ADDR = shp_2_per_ADDR_TO1;
        ScriptInfo.m_uartsh_2_mcu_ADDR = uartsh_2_mcu_ADDR_TO1;
        ScriptInfo.m_mcu_2_ata_ADDR = mcu_2_ata_ADDR_TO1;
        ScriptInfo.m_ata_2_mcu_ADDR = ata_2_mcu_ADDR_TO1;
        ScriptInfo.m_app_2_per_ADDR = app_2_per_ADDR_TO1;
        ScriptInfo.m_per_2_app_ADDR = per_2_app_ADDR_TO1;
        ScriptInfo.m_asrc__mcu_ADDR = asrc__mcu_ADDR_TO1;
        ScriptInfo.m_ext_mem__ipu_ram_ADDR = ext_mem__ipu_ram_ADDR_TO1;
        ScriptInfo.m_mcu_2_spdif_ADDR = mcu_2_spdif_ADDR_TO1;
        ScriptInfo.m_p_2_p_ADDR = p_2_p_ADDR_TO1;
        ScriptInfo.m_spdif_2_mcu_ADDR = spdif_2_mcu_ADDR_TO1;
        ScriptInfo.m_uart_2_per_ADDR = uart_2_per_ADDR_TO1;
        ScriptInfo.m_uartsh_2_per_ADDR = uartsh_2_per_ADDR_TO1;
        ScriptInfo.m_ram_code_size = RAM_CODE_SIZE_TO1;
        ScriptInfo.m_sdma_code = sdma_code_TO1;
    }
    else
    {
        ScriptInfo.m_ap_2_ap_ADDR = ap_2_ap_ADDR;
        ScriptInfo.m_app_2_mcu_ADDR = app_2_mcu_ADDR;
        ScriptInfo.m_mcu_2_app_ADDR = mcu_2_app_ADDR;
        ScriptInfo.m_uart_2_mcu_ADDR = uart_2_mcu_ADDR;
        ScriptInfo.m_shp_2_mcu_ADDR = shp_2_mcu_ADDR;
        ScriptInfo.m_mcu_2_shp_ADDR = mcu_2_shp_ADDR;
        ScriptInfo.m_per_2_shp_ADDR = per_2_shp_ADDR;
        ScriptInfo.m_shp_2_per_ADDR = shp_2_per_ADDR;
        ScriptInfo.m_uartsh_2_mcu_ADDR = uartsh_2_mcu_ADDR;
        ScriptInfo.m_mcu_2_ata_ADDR = mcu_2_ata_ADDR;
        ScriptInfo.m_ata_2_mcu_ADDR = ata_2_mcu_ADDR;
        ScriptInfo.m_app_2_per_ADDR = app_2_per_ADDR;
        ScriptInfo.m_per_2_app_ADDR = per_2_app_ADDR;
        ScriptInfo.m_asrc__mcu_ADDR = asrc__mcu_ADDR;
        ScriptInfo.m_ext_mem__ipu_ram_ADDR = ext_mem__ipu_ram_ADDR;
        ScriptInfo.m_mcu_2_spdif_ADDR = mcu_2_spdif_ADDR;
        ScriptInfo.m_p_2_p_ADDR = p_2_p_ADDR;
        ScriptInfo.m_spdif_2_mcu_ADDR = spdif_2_mcu_ADDR;
        ScriptInfo.m_uart_2_per_ADDR = uart_2_per_ADDR;
        ScriptInfo.m_uartsh_2_per_ADDR = uartsh_2_per_ADDR;
        ScriptInfo.m_ram_code_size = RAM_CODE_SIZE;
        ScriptInfo.m_sdma_code = sdma_code;
    }

    return TRUE;
}

