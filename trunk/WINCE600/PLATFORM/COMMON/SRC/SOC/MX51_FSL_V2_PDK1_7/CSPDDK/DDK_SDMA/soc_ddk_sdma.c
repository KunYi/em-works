//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
//#define USE_AP2AP_4_IRAM
#define SDMA_EXT_BD                 TRUE
#define SDMA_MEM2MEM_SCRIPT         ap_2_ap_ADDR

//#define GPC_SDMA_BASE_ADDRESS       0x53f80000
// No difference between external and internal memory on Marlry, BurstDMA is always 
// used and so the start address of M3 RAM can be set to 0x00000000.// 
#define M3_START_ADDRESS            0x00000000 


//-----------------------------------------------------------------------------
// External Functions
//extern BOOL BSPSdmaBufIntMem(DDK_DMA_REQ dmaReq);

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
    //pChanDesc->bBufIntMem = FALSE;
    
    switch(dmaReq)
    {
/*    Script for Elvis DVFS haven't ready 
    case DDK_DMA_REQ_CCM:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_GPC;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = GPC_SDMA_BASE_ADDRESS;
        pChanDesc->scriptAddr = dptc_dvfs_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
*/
    case DDK_DMA_REQ_ATA_RX:
        pChanDesc->dmaMask[0] = (1U << DMA_EVENT_PATA_RX) | (1U << DMA_EVENT_PATA_TFER_END);
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_PATA_UDMA;
        pChanDesc->scriptAddr = ata_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_ATA_TX:
        pChanDesc->dmaMask[0] = (1U << DMA_EVENT_PATA_TX) | (1U << DMA_EVENT_PATA_TFER_END);
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_PATA_UDMA + ATA_FIFO_DATA_32_OFFSET; 
        pChanDesc->scriptAddr = mcu_2_ata_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
/*Script for Elvis SLM  haven't ready
    case DDK_DMA_REQ_SLIMBUS:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_SLIMBUS;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_SLIMBUS + SLIMBUS_RXDATA_OFFSET;
        pChanDesc->scriptAddr = ;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

*/
    //ECSPI1 RX
    case DDK_DMA_REQ_CSPI1_RX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_ECSPI1_RX;
        pChanDesc->dmaMask[1] = 0;
        pChanDesc->perAddr = CSP_BASE_REG_PA_ECSPI1 + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    //ECSPI1 TX
    case DDK_DMA_REQ_CSPI1_TX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_ECSPI1_TX;
        pChanDesc->dmaMask[1] = 0;      
        pChanDesc->perAddr = CSP_BASE_REG_PA_ECSPI1 + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    //ECSPI2 RX
    case DDK_DMA_REQ_CSPI2_RX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_ECSPI2_RX;
        pChanDesc->dmaMask[1] = 0;      
        pChanDesc->perAddr = CSP_BASE_REG_PA_ECSPI2 + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    //ECSPI2 TX
    case DDK_DMA_REQ_CSPI2_TX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_ECSPI2_TX;
        pChanDesc->dmaMask[1] = 0;      
        pChanDesc->perAddr = CSP_BASE_REG_PA_ECSPI2 + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_HSI2C_TX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_HSI2C_TX;
        pChanDesc->dmaMask[1] = 0;      
        pChanDesc->perAddr = CSP_BASE_REG_PA_HSI2C + I2C_HITDR_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_HSI2C_RX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_HSI2C_RX;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_HSI2C + I2C_HIRDR_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_FIRI_TX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_FIRI_TX;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_FIRI + FIRI_TXFIFO_OFFSET;
        pChanDesc->scriptAddr = mcu_2_firi_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_FIRI_RX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_FIRI_RX;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_FIRI + FIRI_RXFIFO_OFFSET;
        pChanDesc->scriptAddr = firi_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

/*
    case DDK_DMA_REQ_EXT0:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_IOMUX_GPIO1_4;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;
        
    case DDK_DMA_REQ_EXT1:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_IOMUX_GPIO1_5;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;
*/
    case DDK_DMA_REQ_UART2_RX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_UART2_RX;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART2 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uart_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART2_TX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_UART2_TX;
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART2 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART1_RX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_UART1_RX;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART1 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uart_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART1_TX:
        pChanDesc->dmaMask[0] = 1U <<  DMA_EVENT_UART1_TX;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART1 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_I2C1_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_ESDHC1_I2C1;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_I2C1 + I2C_I2DR_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_I2C1_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_ESDHC1_I2C1;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_I2C1 + I2C_I2DR_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
/*
    case DDK_DMA_REQ_MSHC1_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_ESDHC1_I2C1;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_MSHC1 + MSHC_DATR_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_MSHC1_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_ESDHC1_I2C1;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_MSHC1 + MSHC_DATR_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
*/        
    case DDK_DMA_REQ_SDHC1_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_ESDHC1_I2C1;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESDHC1 + ESDHC_DATPORT_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SDHC1_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_ESDHC1_I2C1;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESDHC1 + ESDHC_DATPORT_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_I2C2_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_ESDHC2_I2C2;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_I2C2 + I2C_I2DR_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_I2C2_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_ESDHC2_I2C2;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_I2C2 + I2C_I2DR_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SDHC2_RX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_ESDHC2_I2C2;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESDHC2 + ESDHC_DATPORT_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SDHC2_TX:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_ESDHC2_I2C2;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESDHC2 + ESDHC_DATPORT_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
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
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI2_TX1_SLM_TX3;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_STX1_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_RX0:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI2_RX0_SLM_RX2;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI2 + SSI_SRX0_OFFSET;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI2_TX0:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_SSI2_TX0_SLM_TX2;
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

    case DDK_DMA_REQ_NFC_READ:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_EMIV2_READ;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
/*            
    case DDK_DMA_REQ_EXT2:
        pChanDesc->dmaMask[0] = 1U << DMA_EVENT_CTI2_0;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = TRUE;
        break;
*/    
    //DMA event after 31

    case DDK_DMA_REQ_NFC_WRITE:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_EMIV2_WRITE - 32);      
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
 
    case DDK_DMA_REQ_SSI3_RX1:
        pChanDesc->dmaMask[0] = 0;          
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_SSI3_RX1_SLM_RX1 - 32);   
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI3 + SSI_SRX1_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI3_TX1:
        pChanDesc->dmaMask[0] = 0;         
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_SSI3_TX1_SLM_TX1 - 32);    
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI3 + SSI_STX1_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI3_RX0:
        pChanDesc->dmaMask[0] = 0;           
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_SSI3_RX0_SLM_RX0 - 32);    
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI3 + SSI_SRX0_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SSI3_TX0:
        pChanDesc->dmaMask[0] = 0;         
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_SSI3_TX0_SLM_TX0 - 32);      
        pChanDesc->perAddr = CSP_BASE_REG_PA_SSI3 + SSI_STX0_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    //Map MX51 CSPI to CSPI3        
    case DDK_DMA_REQ_CSPI3_RX:
        pChanDesc->dmaMask[0] = 0;        
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_CSPI_RX - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI + CSPI_RXDATA_OFFSET;
        pChanDesc->scriptAddr = app_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
    
    case DDK_DMA_REQ_CSPI3_TX:
        pChanDesc->dmaMask[0] = 0;    
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_CSPI_TX - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_CSPI + CSPI_TXDATA_OFFSET;
        pChanDesc->scriptAddr = mcu_2_app_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;        
 
    case DDK_DMA_REQ_SDHC3_RX:
        pChanDesc->dmaMask[0] = 0;        
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ESDHC3 - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESDHC3 + ESDHC_DATPORT_OFFSET ;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
        
    case DDK_DMA_REQ_SDHC3_TX:
        pChanDesc->dmaMask[0] = 0;        
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ESDHC3 - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESDHC3 + ESDHC_DATPORT_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_SDHC4_RX:
        pChanDesc->dmaMask[0] = 0;        
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ESDHC4 - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESDHC4 + ESDHC_DATPORT_OFFSET ;
        pChanDesc->scriptAddr = shp_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
        
    case DDK_DMA_REQ_SDHC4_TX:
        pChanDesc->dmaMask[0] = 0;        
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_ESDHC4 - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_ESDHC4 + ESDHC_DATPORT_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_UART3_RX:
        pChanDesc->dmaMask[0] = 0;        
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_UART3_RX - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART3 + UART_URXD_OFFSET;
        pChanDesc->scriptAddr = uartsh_2_mcu_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
    
    case DDK_DMA_REQ_UART3_TX:
        pChanDesc->dmaMask[0] = 0;        
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_UART3_TX - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_UART3 + UART_UTXD_OFFSET;
        pChanDesc->scriptAddr = mcu_2_shp_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

   case DDK_DMA_REQ_SPDIF_TX:
        pChanDesc->dmaMask[0] = 0;        
        pChanDesc->dmaMask[1] = 1U << (DMA_EVENT_SPDIF - 32);
        pChanDesc->perAddr = CSP_BASE_REG_PA_SPDIF + SPDIF_STL_OFFSET;
        pChanDesc->scriptAddr = mcu_2_spdif_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;
        
    case DDK_DMA_REQ_EXTMEM2IPURAM:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = M3_START_ADDRESS;
        pChanDesc->scriptAddr = ext_mem__ipu_ram_ADDR;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    // Use ap_2_ap script since no difference between internal memroy
    // and external memory on Elvis.
    case DDK_DMA_REQ_EXTMEM2IRAM:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    case DDK_DMA_REQ_EXTMEM2EXTMEM:
        pChanDesc->dmaMask[0] = 0;
        pChanDesc->dmaMask[1] = 0;        
        pChanDesc->perAddr = 0;
        pChanDesc->scriptAddr = SDMA_MEM2MEM_SCRIPT;
        pChanDesc->bExtended = SDMA_EXT_BD;
        break;

    default:
        rc = FALSE;
        break;

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
    BOOL rc = FALSE;
    
    switch(dmaReq)
    {

    case DDK_DMA_REQ_I2C1_RX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[0] & (1U << DMA_EVENT_ESDHC1_I2C1))
        {
            pChanDesc->scriptAddr = shp_2_mcu_ADDR;
            rc = TRUE;
        }
        break;
        
    case DDK_DMA_REQ_I2C1_TX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[0] & (1U <<  DMA_EVENT_ESDHC1_I2C1))
        {
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_I2C2_RX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[0] & (1U << DMA_EVENT_ESDHC2_I2C2))
        {
            pChanDesc->scriptAddr = shp_2_mcu_ADDR;
            rc = TRUE;
        }
        break;
        
    case DDK_DMA_REQ_I2C2_TX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[0] & (1U << DMA_EVENT_ESDHC2_I2C2))
        {
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_SDHC1_RX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[0] & (1U << DMA_EVENT_ESDHC1_I2C1))
        {
            pChanDesc->scriptAddr = shp_2_mcu_ADDR;
            rc = TRUE;
        }
        break;
        
    case DDK_DMA_REQ_SDHC1_TX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[0] & (1U <<  DMA_EVENT_ESDHC1_I2C1))
        {
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
            rc = TRUE;
        }
        break;
        
    case DDK_DMA_REQ_SDHC2_RX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[0] & (1U << DMA_EVENT_ESDHC2_I2C2))
        {
            pChanDesc->scriptAddr = shp_2_mcu_ADDR;
            rc = TRUE;
        }
        break;
        
    case DDK_DMA_REQ_SDHC2_TX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[0] & (1U << DMA_EVENT_ESDHC2_I2C2))
        {
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_SDHC3_RX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[1] & (1U << (DMA_EVENT_ESDHC3 - 32)))
        {
            pChanDesc->scriptAddr = shp_2_mcu_ADDR;
            rc = TRUE;
        }
        break;
        
    case DDK_DMA_REQ_SDHC3_TX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[1] & (1U << (DMA_EVENT_ESDHC3 - 32)))
        {
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_SDHC4_RX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[1] & (1U << (DMA_EVENT_ESDHC4 - 32)))
        {
            pChanDesc->scriptAddr = shp_2_mcu_ADDR;
            rc = TRUE;
        }
        break;
        
    case DDK_DMA_REQ_SDHC4_TX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[1] & (1U << (DMA_EVENT_ESDHC4 - 32)))
        {
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
            rc = TRUE;
        }
        break;
/*
    case DDK_DMA_REQ_MSHC1_RX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[1] & (1U << DMA_EVENT_ESDHC1_MSHC1))
        {
            pChanDesc->scriptAddr = shp_2_mcu_ADDR;
            rc = TRUE;
        }
        break;

    case DDK_DMA_REQ_MSHC1_TX:
        // Check to make sure DMA request belongs to shared DMA event
        if (pChanDesc->dmaMask[1] & (1U << DMA_EVENT_ESDHC1_MSHC1))
        {
            pChanDesc->scriptAddr = mcu_2_shp_ADDR;
            rc = TRUE;
        }
        break;
*/        
    default:
        rc = FALSE;
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
        pChanCtxt->GR[0] = pChanDesc->dmaMask[0] & (~(1U << DMA_EVENT_PATA_TFER_END));
        pChanCtxt->GR[1] = (1U << (DMA_EVENT_PATA_TFER_END));
        pChanCtxt->GR[6] = pChanDesc->perAddr;
        pChanCtxt->GR[7] = waterMark;
        break;

    default:
        // GR1 = event mask
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
