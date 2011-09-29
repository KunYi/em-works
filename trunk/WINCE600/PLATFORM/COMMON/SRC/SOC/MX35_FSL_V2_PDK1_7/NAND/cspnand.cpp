//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#include <fmd.h>
#include "common_nandfmd.h"
#include "cspnand.h"
#include "csp.h"

#pragma warning(disable: 4100)

//-----------------------------------------------------------------------------
// External Functions
extern "C" void RdPageAlign1(LPBYTE pSectorBuff, USHORT SectorSize);
extern "C" void RdPageAlign2(LPBYTE pSectorBuff, USHORT SectorSize);
extern "C" void RdPageAlign3(LPBYTE pSectorBuff, USHORT SectorSize);
extern "C" void RdPageAlign4(LPBYTE pSectorBuff, USHORT SectorSize);
extern "C" void WrPageAlign1(LPBYTE pSectorBuff, USHORT SectorSize);
extern "C" void WrPageAlign2(LPBYTE pSectorBuff, USHORT SectorSize);
extern "C" void WrPageAlign3(LPBYTE pSectorBuff, USHORT SectorSize);
extern "C" void WrPageAlign4(LPBYTE pSectorBuff, USHORT SectorSize);
extern BOOL NFCSetPagesize(UINT32 PageSize);


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

// The location used for BBI fix
#define NFC_BUFFER_NUM_USED     (g_CSPfi.fi.wDataBytesPerSector / NANDFC_MAIN_BUFFER_SIZE)
#define NFC_BBI_MAIN_SEGMENT    (NFC_BUFFER_NUM_USED - 1)
#define NFC_SPARE_SEGMENT_LOW   (NFC_BUFFER_NUM_USED - 2)
#define NFC_SPARE_SEGMENT_HIGH  (NFC_BUFFER_NUM_USED - 1)

#define NAND_PAGE_COLUMN_ADDR   (NFC_SPARE_SEGMENT_LOW * ((g_CSPfi.fi.wDataBytesPerSector + ((g_CSPfi.SpareDataLength == 218)? 208:g_CSPfi.SpareDataLength)) / NFC_BUFFER_NUM_USED))

#define NANDFC_NAND_FLASH_CONFIG1_PPB_PAGES   ((g_CSPfi.fi.wSectorsPerBlock == 256) ? 3 : (g_CSPfi.fi.wSectorsPerBlock >> 6))
//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
extern "C" PCSP_NANDFC_REGS g_pNFC = NULL;
FlashInfoExt g_CSPfi;
bool bECCMode;
//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions



//-----------------------------------------------------------------------------
//
//  Function: SwapBBI
//
//  This function implements the BBI(Bad Block Information) swap workaround for
//  the NFC design flaw.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SwapBBI()
{
    UINT16 MainData, TempMainData;
    UINT16 SpareData, TempSpareData;
    
    MainData = INREG16(&g_pNFC->MAIN[NFC_BBI_MAIN_SEGMENT][g_CSPfi.BBIMainAddr / (sizeof(UINT16))]);
    SpareData = INREG16(&g_pNFC->SPARE[NFC_SPARE_SEGMENT_HIGH][0]);

    TempMainData = (MainData & 0xFF00) | (SpareData >> 8);
    TempSpareData = (SpareData & 0x00FF) | (MainData << 8);

    OUTREG16(&g_pNFC->MAIN[NFC_BBI_MAIN_SEGMENT][g_CSPfi.BBIMainAddr / (sizeof(UINT16))], TempMainData);
    OUTREG16(&g_pNFC->SPARE[NFC_SPARE_SEGMENT_HIGH][0], TempSpareData);
}


//-----------------------------------------------------------------------------
//
//  Function: NFCReadMain
//
//  This function reads the NAND flash controller main area and provides this
//  sector data to the upper FMD layers.
//
//  Parameters:
//      pSectorBuff 
//          [out] Buffer containing sector data read.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID NFCReadMain(LPBYTE pSectorBuff)
{        
    if((UINT32)pSectorBuff & 0x1)        
    {
        // Byte-aligned on BYTE[3]
        if((UINT32)pSectorBuff & 0x2)        
        {
            RdPageAlign3(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
        }

        // Byte-aligned on BYTE[1]
        else
        {
            RdPageAlign1(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
        }
    }
    else if ((UINT32)pSectorBuff & 0x2)
    {
        // Half-word aligned on BYTE[2]
        RdPageAlign2(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
    } 
    else
    {
        // Word-aligned
        RdPageAlign4(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
    }
}


//-----------------------------------------------------------------------------
//
//  Function: NFCWriteMain
//
//  This function writes the sector data provided by the upper FMD layers
//  to the NAND flash controller main area.
//
//  Parameters:
//      pSectorBuff 
//          [in] Buffer containing sector data to be written.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID NFCWriteMain(LPBYTE pSectorBuff)
{
    if((UINT32)pSectorBuff & 0x1)        
    {
        // Byte-aligned on BYTE[3]
        if((UINT32)pSectorBuff & 0x2)        
        {
            WrPageAlign3(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
        }

        // Byte-aligned on BYTE[1]
        else
        {
            WrPageAlign1(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
        }
    }
    else if ((UINT32)pSectorBuff & 0x2)
    {
        // Half-word aligned on BYTE[2]
        WrPageAlign2(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
    } 
    else
    {
        // Word-aligned
        WrPageAlign4(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
    }
}


//-----------------------------------------------------------------------------
//
//  Function: NFCReadSpare
//
//  This function reads the NAND flash controller spare area and provides this
//  information to the upper FMD layers.
//
//  Parameters:
//      pSectorInfoBuff 
//          [out] Buffer containing sector information read.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID NFCReadSpare(PSectorInfo pSectorInfoBuff)
{
    pSectorInfoBuff->dwReserved1 = INREG32((UINT32 *)&g_pNFC->SPARE[NFC_SPARE_SEGMENT_LOW][0]);
    *((UINT32 *)(&pSectorInfoBuff->bOEMReserved)) = INREG32(&g_pNFC->SPARE[NFC_SPARE_SEGMENT_HIGH][0]);
}


//-----------------------------------------------------------------------------
//
//  Function: NFCClearSpare
//
//  This function clears the NAND flash controller spare area (all
//  ones written to the spare area will prevent the spare area of
//  the NAND memory device from being updated since only zeros are
//  written during a NAND program cycle).
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID NFCClearSpare(VOID)
{
    UINT32 *dest;
    int i;

    for(i = 0; i < NFC_BUFFER_NUM_USED; i++)
    {
        dest = (UINT32 *)&g_pNFC->SPARE[i];

        OUTREG32(&dest[0], 0xFFFFFFFF);
        OUTREG32(&dest[1], 0xFFFFFFFF);
        OUTREG32(&dest[2], 0xFFFFFFFF);
        OUTREG32(&dest[3], 0xFFFFFFFF);
    }
}


//-----------------------------------------------------------------------------
//
//  Function: NFCWriteSpare
//
//  This function writes the spare data provided by the upper MDD layers
//  to the NAND flash controller spare area.
//
//  Parameters:
//      pSectorInfoBuff 
//          [in] Buffer containing sector information to be written.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID NFCWriteSpare(PSectorInfo pSectorInfoBuff)
{
    NFCClearSpare();

    if(pSectorInfoBuff)
    {
        OUTREG32(&g_pNFC->SPARE[NFC_SPARE_SEGMENT_LOW][0], pSectorInfoBuff->dwReserved1);
        OUTREG32(&g_pNFC->SPARE[NFC_SPARE_SEGMENT_HIGH][0], *((UINT32 *)(&pSectorInfoBuff->bOEMReserved)));
    }
}


//-----------------------------------------------------------------------------
//
//  Function: SetCs
//
//  This function sets NFC to certain cs
//
//  Parameters:
//      dwCs:   which cs is needed to set.
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
static VOID SetCs(DWORD dwCs)
{
    static DWORD m_cs = 0;
    
    if(m_cs != dwCs && dwCs < 4)
    {
        INSREG16BF(&g_pNFC->RAM_BUFFER_ADDRESS, NANDFC_RAM_BUFFER_ADDRESS_CS, dwCs);
        m_cs = dwCs;
    }
}

//-----------------------------------------------------------------------------
//
//  Function: NFCAlloc
//
//  This function maps the NFC peripheral, configures interupts, and
//  configures NFC DMA support.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL NFCAlloc(VOID)
{
    // Map peripheral physical address to virtual address
    g_pNFC = (PCSP_NANDFC_REGS) BSPNAND_RemapRegister(CSP_BASE_REG_PA_NANDFC, sizeof(CSP_NANDFC_REGS));

    // Check if virtual mapping failed
    if (g_pNFC == NULL)
    {
        RETAILMSG(1, (_T("NFCAlloc:  MmMapIoSpace failed!\r\n")));
        return FALSE;
    }
 
    return TRUE;   
}
//-----------------------------------------------------------------------------
//
// Function: NFCSetPagesize
//
// This function will set the NFMS bit of register RCSR in CCM module. For large
// page size nand flash, bit NFMS should be set to 1.
// 
// Parameters:
//      PageSize
//          [in] - 4096, 2048, 512 bytes
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL NFCSetPagesize(UINT32 PageSize)
{
    PCSP_CCM_REGS pCCM;
    
    // Map peripheral physical address to virtual address
    pCCM = (PCSP_CCM_REGS)BSPNAND_RemapRegister(CSP_BASE_REG_PA_CCM, sizeof(CSP_CCM_REGS));

    // Check if virtual mapping failed
    if (pCCM == NULL)
    {
        RETAILMSG(1, (_T("NFCSetPagesize: MmMapIoSpace failed!\r\n")));
        return FALSE;
    }

    switch(PageSize)
    {
    case 4096:
        // 4KB NAND Flash Page Size
        INSREG32BF(&pCCM->RCSR, CCM_RCSR_NFC_FMS, CCM_RCSR_NFC_FMS_NOT2K);
        INSREG32BF(&pCCM->RCSR, CCM_RCSR_NFC_4K, CCM_RCSR_NFC_4K_4K);
        break;
       
    case 2048:
        // 2KB NAND Flash Page Size
        INSREG32BF(&pCCM->RCSR, CCM_RCSR_NFC_FMS, CCM_RCSR_NFC_FMS_2K);
        INSREG32BF(&pCCM->RCSR, CCM_RCSR_NFC_4K, CCM_RCSR_NFC_4K_NOT4K);
        break;

    case 512:
        // 512B NAND Flash Page Size
        INSREG32BF(&pCCM->RCSR, CCM_RCSR_NFC_FMS, CCM_RCSR_NFC_FMS_NOT2K);
        INSREG32BF(&pCCM->RCSR, CCM_RCSR_NFC_4K, CCM_RCSR_NFC_4K_NOT4K);
        break;

    default:
        break;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  NFCWait
//
//  This functions waits for the pending NANDFC opeation to complete.  
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID NFCWait()
{
    while (!(INREG16(&g_pNFC->NAND_FLASH_CONFIG2) & CSP_BITFMASK(NANDFC_NAND_FLASH_CONFIG2_INT)));

    // Clear the NANDFC interrupt
    CLRREG16(&g_pNFC->NAND_FLASH_CONFIG2, 
             CSP_BITFMASK(NANDFC_NAND_FLASH_CONFIG2_INT));

}

//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_Reset
//
//  This function reset NAND flash.
//
//  Parameters:
//      None
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
VOID CSPNAND_Reset(DWORD dwCs)
{
    SetCs(dwCs);
    NF_CMD(g_CSPfi.CmdReset);     // Send flash reset command
}


//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_ReadID
//
//  This function read NAND flash ID.
//
//  Parameters:
//      None
//
//  Returns:
//      UINT32: NAND Device ID.
//
//-----------------------------------------------------------------------------
UINT32 CSPNAND_ReadID(DWORD dwCs)
{
    UINT32 NandDeviceID;
    
    SetCs(dwCs);
    // Get manufacturer and device codes.
    NF_CMD(g_CSPfi.CmdReadId);      // Read ID command
    NF_ADDR(0);                     // Required address cycle
    NF_RD_ID();                     // Read ID into NFC buffer

    NandDeviceID = INREG32(&g_pNFC->MAIN[0][0]);
    
    return NandDeviceID;
}

//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_EnableECC
//
//  This function enable/disable ECC
//
//  Parameters:
//      Enable:   enable/disable
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
VOID CSPNAND_EnableECC(BOOL Enable)
{
    if(Enable)
    {
        INSREG16BF(&g_pNFC->NAND_FLASH_CONFIG1, NANDFC_NAND_FLASH_CONFIG1_ECC_EN, NANDFC_NAND_FLASH_CONFIG1_ECC_EN_ENABLE);
    }
    else
    {
        INSREG16BF(&g_pNFC->NAND_FLASH_CONFIG1, NANDFC_NAND_FLASH_CONFIG1_ECC_EN, NANDFC_NAND_FLASH_CONFIG1_ECC_EN_BYPASS);
    }
}

//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_ILSupport
//
//  This function returns whether NFC supports interleave mode
//
//  Parameters:
//      None
//
//  Returns:
//      TRUE indicates support, FALSE indicates not support
//
//-----------------------------------------------------------------------------
BOOL CSPNAND_ILSupport()
{
    // Ringo does not support interleave mode
    return FALSE;
}

VOID CSPNAND_PostInit(PFlashInfoExt pFlashInfo)
{
    UNREFERENCED_PARAMETER(pFlashInfo);
}

//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_Init
//
//  This function initializes the flash memory of a device.
//
//  Parameters:
//      pFlashInfo
//          [in] Pointer to the FlashInfoExt used to pass down information. 
//
//  Returns:
//      A handle that can be used in a call to CSPNAND_Deinit. A value of zero (0) 
//      represents failure.
//
//-----------------------------------------------------------------------------
PVOID CSPNAND_Init(PFlashInfoExt pFlashInfo)
{
    PVOID rc = NULL;
        
    if(!pFlashInfo)
    {
        return FALSE;   
    }
    
    memcpy(&g_CSPfi, pFlashInfo, sizeof(FlashInfoExt));
    
    // Perform NFC allocations, mappings, and initializations
    if(!NFCAlloc())
    {
        ERRORMSG(TRUE, (_T("FMD_Init: failed memory allocation/mapping\r\n")));
        goto cleanUp;
    }
    
    // For NAND Flash page size in register RCSR(CCM module) 
    if(!NFCSetPagesize(g_CSPfi.fi.wDataBytesPerSector))
    {
        ERRORMSG(TRUE, (_T("FMD_Init: failed setting page size\r\n")));
        goto cleanUp;
    }
    
    // Unlock buffer pages
    INSREG16BF(&g_pNFC->NFC_CONFIGURATION, NANDFC_NFC_CONFIGURATION_BLS,
                NANDFC_NFC_CONFIGURATION_BLS_UNLOCKED);

    // Select Chip 0, NFC buffer 0 for the moment
    INSREG16BF(&g_pNFC->RAM_BUFFER_ADDRESS, NANDFC_RAM_BUFFER_ADDRESS_CS, 0);
    INSREG16BF(&g_pNFC->RAM_BUFFER_ADDRESS, NANDFC_RAM_BUFFER_ADDRESS_RBA, 0);

    // Unlock all blocks
    OUTREG16(&g_pNFC->UNLOCK_START_BLK_ADD0, 0);
    OUTREG16(&g_pNFC->UNLOCK_END_BLK_ADD0, g_CSPfi.fi.dwNumBlocks - 1);
    OUTREG16(&g_pNFC->NF_WR_PROT, CSP_BITFVAL(NANDFC_NF_WR_PROT_WPC, NANDFC_NF_WR_PROT_WPC_UNLOCK));
    
    if(g_CSPfi.SpareDataLength > 128)
        bECCMode = NANDFC_NAND_FLASH_CONFIG1_ECC_MODE_8BIT;
    else
        bECCMode = NANDFC_NAND_FLASH_CONFIG1_ECC_MODE_4BIT;    
    // Configure the NFC
    //  - Enable hardware ECC
    //  - Mask NFC interrupt
    //  - Little endian mode
    //  - Reset NFC state machine
    //  - CE signal operates normally
    //  - PPB set according to NAND Flash chip, 128 pages
    //  - ECC_MODE 4bit error correction
    //  - FP_INT geneated whole page
    //  - SYM, NFC_RST, SP_EN, DMA_MODE,
    OUTREG16(&g_pNFC->NAND_FLASH_CONFIG1, 
            CSP_BITFVAL(NANDFC_NAND_FLASH_CONFIG1_FP_INT, NANDFC_NAND_FLASH_CONFIG1_FP_INT_FULL_PAGE_ENABLE) |
            CSP_BITFVAL(NANDFC_NAND_FLASH_CONFIG1_PPB, NANDFC_NAND_FLASH_CONFIG1_PPB_PAGES) |
            CSP_BITFVAL(NANDFC_NAND_FLASH_CONFIG1_NF_CE, NANDFC_NAND_FLASH_CONFIG1_NF_CE_UNFORCE)  |
            CSP_BITFVAL(NANDFC_NAND_FLASH_CONFIG1_NF_BIG, NANDFC_NAND_FLASH_CONFIG1_NF_BIG_LITTLE) |
            CSP_BITFVAL(NANDFC_NAND_FLASH_CONFIG1_INT_MSK, NANDFC_NAND_FLASH_CONFIG1_INT_MSK_MASK) |
            CSP_BITFVAL(NANDFC_NAND_FLASH_CONFIG1_ECC_EN, NANDFC_NAND_FLASH_CONFIG1_ECC_EN_ENABLE) |
            CSP_BITFVAL(NANDFC_NAND_FLASH_CONFIG1_DMA_MODE, NANDFC_NAND_FLASH_CONFIG1_DMA_MODE_PAGE) |
            CSP_BITFVAL(NANDFC_NAND_FLASH_CONFIG1_ECC_MODE, bECCMode));    

    // Spare Area Size, Dependent on NAND Flash chip
    OUTREG16(&g_pNFC->SPAS, CSP_BITFVAL(NANDFC_SPAS_SPAS, (g_CSPfi.SpareDataLength / 2)));

    rc = g_pNFC;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_Deinit
//
//  This function de-initializes the flash chip.
//
//  Parameters:
//      hFMD 
//          [in] The handle returned from CSPNAND_Init. 
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL CSPNAND_Deinit(PVOID hFMD)
{
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_ReadSector
//
//  This function reads the requested sector data and metadata from the 
//  flash media.
//
//  Parameters:
//      
//      startSectorAddr 
//          [in] The starting physical sector address to read.
//
//      pSectorBuff 
//          [out] Pointer to the buffer that contains the sector data read 
//          from flash memory. Set to NULL if this data is not needed.
//
//      pSectorInfoBuff 
//          [out] Buffer for an array of sector information structures. There 
//          is one sector information entry for every sector that is to be read. 
//          Set to NULL if this data is not needed. 
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL CSPNAND_ReadSector(DWORD dwCs, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, 
    PSectorInfo pSectorInfoBuff)
{
    BOOL rc = FALSE;
    UINT32 RegECCValue = 0;
    UINT32 RowAddr, ColumnAddr;
    UINT16 NumBufferUsed = NFC_BUFFER_NUM_USED;

    // Set Address
    RowAddr = (UINT32)startSectorAddr;
    if(pSectorBuff)
    {
        ColumnAddr = (UINT32)0;
    }
    else
    {
        ColumnAddr = (UINT32)(NAND_PAGE_COLUMN_ADDR);
    }

    SetCs(dwCs);
    // Send Command
    NF_CMD(g_CSPfi.CmdRead1);
    NF_ADDR_COL(ColumnAddr);
    NF_ADDR_PAGE(RowAddr);
    NF_CMD(g_CSPfi.CmdRead2);

    // Read page data into NFC Buffer
    if(pSectorBuff)
    {
        NF_BUF_ADDR(0);
    }
    else
    {
        NF_BUF_ADDR(NFC_SPARE_SEGMENT_LOW);
    }
    NF_RD_PAGE();

    // Check ECC Status
    if(pSectorBuff)
    {
        RegECCValue = INREG16(&g_pNFC->ECC_STATUS_RESULT1);

        if(bECCMode == NANDFC_NAND_FLASH_CONFIG1_ECC_MODE_4BIT)
        {        
            if(((RegECCValue & 0x000F) > NANDFC_ECC_STATUS_RESULT_NOSER1_4SB_ERR) ||
               ((RegECCValue & 0x00F0) > NANDFC_ECC_STATUS_RESULT_NOSER2_4SB_ERR) ||
               ((RegECCValue & 0x0F00) > NANDFC_ECC_STATUS_RESULT_NOSER3_4SB_ERR) ||
               ((RegECCValue & 0xF000) > NANDFC_ECC_STATUS_RESULT_NOSER4_4SB_ERR))
            {
                ERRORMSG(TRUE, (_T("Sector 0x%X --- Uncorrectable ECC error : 0x%X\r\n"), startSectorAddr, RegECCValue));
                goto cleanUp;
            }
        }
        else
        {        
            if(((RegECCValue & 0x000F) > NANDFC_ECC_STATUS_RESULT_NOSER1_8SB_ERR) ||
               ((RegECCValue & 0x00F0) > NANDFC_ECC_STATUS_RESULT_NOSER2_8SB_ERR) ||
               ((RegECCValue & 0x0F00) > NANDFC_ECC_STATUS_RESULT_NOSER3_8SB_ERR) ||
               ((RegECCValue & 0xF000) > NANDFC_ECC_STATUS_RESULT_NOSER4_8SB_ERR))
            {
                ERRORMSG(TRUE, (_T("Sector 0x%X --- Uncorrectable ECC error : 0x%X, 8bit ECC mode.\r\n"), startSectorAddr, RegECCValue));
                goto cleanUp;
            }
        }
        if(NumBufferUsed == NANDFC_NUM_BUFFERS)
        {
            RegECCValue = INREG16(&g_pNFC->ECC_STATUS_RESULT2);

            if(bECCMode == NANDFC_NAND_FLASH_CONFIG1_ECC_MODE_4BIT)
            {            
                if(((RegECCValue & 0x000F) > NANDFC_ECC_STATUS_RESULT_NOSER5_4SB_ERR) ||
                   ((RegECCValue & 0x00F0) > NANDFC_ECC_STATUS_RESULT_NOSER6_4SB_ERR) ||
                   ((RegECCValue & 0x0F00) > NANDFC_ECC_STATUS_RESULT_NOSER7_4SB_ERR) ||
                   ((RegECCValue & 0xF000) > NANDFC_ECC_STATUS_RESULT_NOSER8_4SB_ERR))
                {
                    ERRORMSG(TRUE, (_T("Sector 0x%X --- Uncorrectable ECC error : 0x%X\r\n"), startSectorAddr, RegECCValue));
                    goto cleanUp;
                }
            }
            else
            {
                if(((RegECCValue & 0x000F) > NANDFC_ECC_STATUS_RESULT_NOSER5_8SB_ERR) ||
                   ((RegECCValue & 0x00F0) > NANDFC_ECC_STATUS_RESULT_NOSER6_8SB_ERR) ||
                   ((RegECCValue & 0x0F00) > NANDFC_ECC_STATUS_RESULT_NOSER7_8SB_ERR) ||
                   ((RegECCValue & 0xF000) > NANDFC_ECC_STATUS_RESULT_NOSER8_8SB_ERR))
                {
                    ERRORMSG(TRUE, (_T("Sector 0x%X --- Uncorrectable ECC error : 0x%X, 8bit ECC mode.\r\n"), startSectorAddr, RegECCValue<<16));
                    goto cleanUp;
                }                
            }
        }
    }
    else
    {
        if(NumBufferUsed == NANDFC_NUM_BUFFERS)
        {
            RegECCValue = INREG16(&g_pNFC->ECC_STATUS_RESULT2);

            if(bECCMode == NANDFC_NAND_FLASH_CONFIG1_ECC_MODE_4BIT)
            {             
                if(((RegECCValue & 0xF000) > NANDFC_ECC_STATUS_RESULT_NOSER8_4SB_ERR) ||
                   ((RegECCValue & 0x0F00) > NANDFC_ECC_STATUS_RESULT_NOSER7_4SB_ERR))
                {
                    ERRORMSG(TRUE, (_T("Sector 0x%X --- Uncorrectable ECC error : 0x%X\r\n"), startSectorAddr, RegECCValue));
                    goto cleanUp;
                }
            }
            else
            {
                if(((RegECCValue & 0xF000) > NANDFC_ECC_STATUS_RESULT_NOSER8_8SB_ERR) ||
                   ((RegECCValue & 0x0F00) > NANDFC_ECC_STATUS_RESULT_NOSER7_8SB_ERR))
                {
                    ERRORMSG(TRUE, (_T("Sector 0x%X --- Uncorrectable ECC error : 0x%X, 8bit ECC mode.\r\n"), startSectorAddr, RegECCValue<<16));       
                    goto cleanUp;
                }                
            }
        }
        else
        {
            RegECCValue = INREG16(&g_pNFC->ECC_STATUS_RESULT1);
            if(bECCMode == NANDFC_NAND_FLASH_CONFIG1_ECC_MODE_4BIT)
            {             
                if(((RegECCValue & 0xF000) > NANDFC_ECC_STATUS_RESULT_NOSER4_4SB_ERR) ||
                   ((RegECCValue & 0x0F00) > NANDFC_ECC_STATUS_RESULT_NOSER3_4SB_ERR))
                {
                    ERRORMSG(TRUE, (_T("Sector 0x%X --- Uncorrectable ECC error : 0x%X\r\n"), startSectorAddr, RegECCValue));
                    goto cleanUp;
                }
            }
            else
            {
                if(((RegECCValue & 0xF000) > NANDFC_ECC_STATUS_RESULT_NOSER4_8SB_ERR) ||
                   ((RegECCValue & 0x0F00) > NANDFC_ECC_STATUS_RESULT_NOSER3_8SB_ERR))
                {
                    ERRORMSG(TRUE, (_T("Sector 0x%X --- Uncorrectable ECC error : 0x%X, 8bit ECC mode.\r\n"), startSectorAddr, RegECCValue));
                    goto cleanUp;
                }                
            }                
        }
    }
    
    // BBI Swap
    SwapBBI();

    // Read Data
    if(pSectorBuff)
    {
        NFCReadMain(pSectorBuff);
    }

    if(pSectorInfoBuff)
    {
        NFCReadSpare(pSectorInfoBuff);
    }

    rc = TRUE;

cleanUp:

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_EraseBlock
//
//  This function erases the specified flash block.
//
//  Parameters:
//      blockID 
//          [in] The block number to erase.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL CSPNAND_EraseBlock(DWORD dwCs, BLOCK_ID blockID)
{
    UINT16 Status;
    UINT32 RowAddr = blockID * g_CSPfi.fi.wSectorsPerBlock;

    SetCs(dwCs);
    // Erase the block
    NF_CMD(g_CSPfi.CmdErase1);
    NF_ADDR_PAGE(RowAddr);
    NF_CMD(g_CSPfi.CmdErase2);

    // Erase Status
    NF_BUF_ADDR(0);
    NF_CMD(g_CSPfi.CmdReadStatus)          // Send status command
    NF_RD_STATUS();
    Status = INREG16(&g_pNFC->MAIN[0][0]);
        
    if(Status & (1 << g_CSPfi.StatusErrorBit))
    {
        ERRORMSG(TRUE, (_T("NAND erase failed (blockID = 0x%x)\r\n"), blockID)); 
        return FALSE;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_WriteSector
//
//  This function writes the requested sector data and metadata to the 
//  flash media.
//
//  Parameters:
//
//      startSectorAddr 
//          [in] The starting physical sector address to write to.
//
//      pSectorBuff 
//          [in] Pointer to the buffer that contains the sector data to write. 
//          Set to NULL if no data is to be written.
//
//      pSectorInfoBuff 
//          [in] Buffer for an array of sector information structures. There 
//          must be one sector information entry for each sector that is to be 
//          written. Set to NULL if this data is not written.
//
//      dwNumSectors 
//          [in] Number of sectors to write.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL CSPNAND_WriteSector(DWORD dwCs, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, 
    PSectorInfo pSectorInfoBuff)
{
    UINT16 Status;
    BOOL rc = FALSE;
    UINT32 RowAddr, ColumnAddr;

    // Set Address
    RowAddr = (UINT32)startSectorAddr;
    if(pSectorBuff)
    {
        ColumnAddr = (UINT32)0;
    }
    else
    {
        ColumnAddr = (UINT32)(NAND_PAGE_COLUMN_ADDR);
    }
    
    // Write data into the NFC Buffer
    if(pSectorBuff)
    {
        NFCWriteMain(pSectorBuff);
    }

    NFCWriteSpare(pSectorInfoBuff);

    // BBI Swap
    SwapBBI();
    
    SetCs(dwCs);
    NF_CMD(g_CSPfi.CmdWrite1);                  // Send sequential data input command
    NF_ADDR_COL(ColumnAddr);                    // Send column address
    NF_ADDR_PAGE(RowAddr);                      // Send page address

    // Write out the page data
    if(pSectorBuff)
    {
        NF_BUF_ADDR(0);
    }
    else
    {
        NF_BUF_ADDR(NFC_SPARE_SEGMENT_LOW);
    }  
    NF_WR_PAGE();    
    
    NF_CMD(g_CSPfi.CmdWrite2)                   // Send program command
    
    NF_BUF_ADDR(0);        
    NF_CMD(g_CSPfi.CmdReadStatus)               // Send status command
    NF_RD_STATUS();                             // Read status into NFC buffer
    Status = INREG16(&g_pNFC->MAIN[0][0]);      // Read status from NFC buffer
    
    if(Status & (1 << g_CSPfi.StatusErrorBit))
    {
        ERRORMSG(TRUE, (_T("NAND program failed (Sectoraddr = 0x%x)\r\n"), startSectorAddr)); 
        goto cleanUp;
    }
            
    rc = TRUE;
        
cleanUp:
        
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_GetECCStatus
//
//  This function retrieve the ECC status
//
//  Parameters:
//      None
//
//  Returns:  
//      ECC status
//
//-----------------------------------------------------------------------------
UINT32 CSPNAND_GetECCStatus()
{
    UINT32 ECC_Status1, ECC_Status2;
    UINT16 NumBufferUsed = NFC_BUFFER_NUM_USED;
    
    ECC_Status1 = INREG16(&g_pNFC->ECC_STATUS_RESULT1);
    ECC_Status2 = 0;   

    if(NumBufferUsed == NANDFC_NUM_BUFFERS)
    {
        ECC_Status2 = INREG16(&g_pNFC->ECC_STATUS_RESULT2);    
    }

    return (ECC_Status1 | (ECC_Status2 << 16));
}


