//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  cspnand.c
//
//
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nand_hal.h>
#include <nand_ecc.h>
#include <nand_dma.h>
#include <nand_gpmi.h>
#include <fmd.h>
#include <ceddk.h>
#pragma warning(pop)
#include "csp.h"
#include "regsdigctl.h"
#include "regsgpmi.h"
#include "regsbch.h"
#include "regsocotp.h"
#include "common_nandfmd.h"
#include "cspnand.h"
#include "mx28_nandlayout.h"

//-----------------------------------------------------------------------------
//  Types
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Prototypes
//-----------------------------------------------------------------------------
PVOID pv_HWregGPMI=NULL;
PVOID pv_HWregDIGCTL=NULL;
PVOID pv_HWregBCH=NULL;
//-----------------------------------------------------------------------------
//  Globals
//-----------------------------------------------------------------------------

// NAND flash sector buffer physical allocation done to get physical and virtual pointers
static UINT8* pFlashPageBuf;
// NAND flash sector info (metadata) buffer physical allocation done to get physical and virtual pointers
static UINT8* pFlashAuxBuf;

FlashInfoExt g_CSPfi;

extern  void NANDBootReserved();

//-----------------------------------------------------------------------------
//  FMD Interface functions
//-----------------------------------------------------------------------------
#define FLASH_SECTOR_BUFFER_SIZE        (4096)
#define FLASH_SECTOR_INFO_BUFFER_SIZE   (256)

// Warning: not persistent. Driver overwrites this with 0 as it attempts to detect multi nands.
BYTE *pReadIDDecode;

VOID HALDelay();

UINT32 ComputeBufMask(UINT32 byteCount, UINT32 * dataCount, UINT32 * auxCount)
{
    UINT32 mask;
    UINT32 temp;

    // Calculate the ECC Mask for this transaction.
    // Auxilliary = 0x100 set to request transfer to/from the Auxiliary buffer.
    // Buffer7 = 0x080 set to request transfer to/from buffer7.
    // Buffer6 = 0x040 set to request transfer to/from buffer6.
    // Buffer5 = 0x020 set to request transfer to/from buffer5.
    // Buffer4 = 0x010 set to request transfer to/from buffer4.
    // Buffer3 = 0x008 set to request transfer to/from buffer3.
    // Buffer2 = 0x004 set to request transfer to/from buffer2.
    // Buffer1 = 0x002 set to request transfer to/from buffer1.
    // Buffer0 = 0x001 set to request transfer to/from buffer0.
    // First calculate how many 512 byte buffers fit in here.
    temp = (byteCount / 512);
    mask = ((1 << temp) - 1);
    temp *= 512;

    // If there are any leftovers, assume they are redundant area.
    if (byteCount - temp)
    {
        mask |= 0x100;
    }

    if (dataCount)
    {
        *dataCount = temp;
    }

    if (auxCount)
    {
        *auxCount = byteCount - temp;
    }

    return mask;
}

VOID HALDelay()
{
    volatile UINT32 i;
    for(i=0;i<300*1000;i++);
}

BOOL GetStatus(DWORD dwCs, UINT16 *pStatus)
{
    NAND_dma_generic_struct_t * pNandDmaDescriptor = &DmaGenericDescriptor[0];
    BOOL retCode;
    
    // Command for Read Status - may be 0x70 - 0x74...
    pNandDmaDescriptor->NandReadStatus.CommandBuffer = (UCHAR)eNandProgCmdReadStatus;

    DMA_SetupReadStatusDesc(&(pNandDmaDescriptor->NandReadStatus), dwCs,
                               &(pNandDmaDescriptor->NandReadStatus.ReadStatusResult));

    DMA_Start((dma_cmd_t *)&(pNandDmaDescriptor->NandReadStatus), dwCs);

    retCode = DMA_Wait(NAND_READ_PAGE_TIMEOUT, (UINT8)dwCs+NAND0_APBH_CH);

    if (!retCode)
    {
        ERRORMSG(TRUE, (TEXT("GetStatus: FALSE\r\n")));
        return FALSE;
    }

    *pStatus = (WORD)(pNandDmaDescriptor->NandReadStatus.ReadStatusResult);
    
    return TRUE;
}


BOOL CheckStatus(WORD Status, PBYTE pBusy)
{
    if(!(Status & (1 << g_CSPfi.StatusBusyBit)))
    {
        *pBusy = TRUE;
        ERRORMSG(TRUE, (_T("StatusBusy\r\n")));
        return TRUE;   
    }
    if(Status & (1 << g_CSPfi.StatusErrorBit))
    {
        ERRORMSG(TRUE, (_T("StatusError\r\n")));
        return FALSE;   
    }
    *pBusy = FALSE;
    return TRUE;
}

BOOL WaitForProgramDone(DWORD dwCs,
    DWORD wTimeout, UINT16 *pStatus)
{
    int i = 0;
    BYTE busy;
    
    // Wait for DMA to complete.
    if (!(DMA_Wait(wTimeout, dwCs))) {
        RETAILMSG(0, (TEXT("-WaitForProgramDone(FALSE)\r\n")));
        return(FALSE);
    }

    //Now we need to test the return Status.
    while(CheckStatus(*pStatus, &busy) && busy && i++ < 1000)
    {
        if(!GetStatus(dwCs, pStatus))
        {
            ERRORMSG(TRUE, (_T("GetStatus returns false\r\n")));
            return FALSE;   
        }
    }
    if(!CheckStatus(*pStatus, &busy) || i >=1000)
    {
        return FALSE;   
    }
    
    return TRUE;
}

BOOL WriteNand(
    VOID                                    *pWriteDmaDescriptor,
    UINT32 u32NandDeviceNumber,
    UINT32 u32ColumnOffset,
    UINT32 u32PageNum,
    UINT32 u32WriteSize,
    UINT32 u32EnableEcc,
    UINT8  *p8PageBuf,
    UINT8  *p8AuxillaryBuf)
{
    NAND_dma_program_t              *pNandDmaDescriptor;
    
    pNandDmaDescriptor = (NAND_dma_program_t *) pWriteDmaDescriptor;

    // Always 2 bytes and we're starting at column after data ends.
    pNandDmaDescriptor->NandProgSeed.bType2Columns[0] =
        (UCHAR)(u32ColumnOffset & 0xFF);
    pNandDmaDescriptor->NandProgSeed.bType2Columns[1] =
        (UCHAR)((u32ColumnOffset >> 8) & 0xFF);

    // Fill in the Row Address. (Can be 2 or 3 bytes)
    pNandDmaDescriptor->NandProgSeed.bType2Rows[0] =
        (UCHAR)(u32PageNum & 0xFF);
    pNandDmaDescriptor->NandProgSeed.bType2Rows[1] =
        (UCHAR)((u32PageNum >> 8) & 0xFF);
    pNandDmaDescriptor->NandProgSeed.bType2Rows[2] =
        (UCHAR)((u32PageNum >> 16) & 0xFF);

    // Load Command Code for Serial Data Input (0x80)
    pNandDmaDescriptor->NandProgSeed.tx_cle1 = eNandProgCmdSerialDataInput;
    
    // Load Command Code for Page Program (0x10)
    pNandDmaDescriptor->NandProgSeed.tx_cle2 = eNandProgCmdPageProgram;
    
    pNandDmaDescriptor->NandProgSeed.u32EnableHwEcc = u32EnableEcc;
    
    pNandDmaDescriptor->NandProgSeed.pDataBuffer = p8PageBuf;
    pNandDmaDescriptor->NandProgSeed.pAuxBuffer = p8AuxillaryBuf;
    pNandDmaDescriptor->NandProgSeed.uiWriteSize = u32WriteSize;
    
    DMA_ModifyWriteDesc(pNandDmaDescriptor, u32NandDeviceNumber, &(pNandDmaDescriptor->NandProgSeed));
    
    DMA_Start((dma_cmd_t *)pNandDmaDescriptor, u32NandDeviceNumber);

    return TRUE;
}
BOOL ReadNand(
    VOID   *pReadDmaDescriptor,
    UINT32 u32NandDeviceNumber,
    UINT32 u32ColumnOffset,
    UINT32 u32PageNum,
    UINT32 u32ReadSize,
    UINT32 u32EnableEcc,
    UINT8           *p8PageBuf,
    UINT8           *p8AuxillaryBuf)
{
    NAND_dma_read_t  *pDmaReadDescriptor = (NAND_dma_read_t *) pReadDmaDescriptor;
    NAND_read_seed_t *pDmaReadSeed = (NAND_read_seed_t *) &(pDmaReadDescriptor->NAND_DMA_Read_Seed);

    // Fill in the Column Address (Always 2 bytes)
    pDmaReadSeed->bType2Columns[0] = (UCHAR)(u32ColumnOffset & 0xFF);
    pDmaReadSeed->bType2Columns[1] = (UCHAR)((u32ColumnOffset>>8) & 0xFF);

    // Fill in the Row Address. (Can be 2 or 3 bytes)
    pDmaReadSeed->bType2Rows[0] = (UCHAR)(u32PageNum & 0xFF);
    pDmaReadSeed->bType2Rows[1] = (UCHAR)((u32PageNum>>8) & 0xFF);
    // This is always created, but the Address size determines whether
    // this data is actually sent.
    pDmaReadSeed->bType2Rows[2] = (UCHAR)((u32PageNum>>16) & 0xFF);

    // Set how many bytes need to be read.
    pDmaReadSeed->uiReadSize = u32ReadSize;
    // Set the location where data will be read into.
    pDmaReadSeed->pDataBuffer = p8PageBuf;
    // Set the location where auxillary buffers will reside..
    pDmaReadSeed->pAuxBuffer = p8AuxillaryBuf;

    pDmaReadSeed->enableECC = u32EnableEcc;

    DMA_ModifyReadDesc(pDmaReadDescriptor, u32NandDeviceNumber,
                                      pDmaReadSeed);
    // Clear the ECC Complete flag.
    DDI_NAND_HAL_CLEAR_ECC_COMPLETE_FLAG();
    
    // Start the DMA.
    {
        // Kick off the DMA.
        DMA_Start((dma_cmd_t *)pDmaReadDescriptor, u32NandDeviceNumber);

    }
    return(TRUE);
}

void UnMapRegister(VOID)
{
    if(pv_HWregBCH)
    {
        BSPNAND_UnmapRegister(pv_HWregBCH, 0x1000);
    }
    if(pv_HWregGPMI)
    {
        BSPNAND_UnmapRegister(pv_HWregGPMI, 0x1000);
    }
    if(pv_HWregDIGCTL)
    {
        BSPNAND_UnmapRegister(pv_HWregDIGCTL, 0x1000);
    }   
}
//-----------------------------------------------------------------------------
//
//  Function: MapRegister
//
//  This function maps the GPMI/BCH/DIGITAL registers and internal RAM from physical 
//  address to virtual address.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL MapRegister(VOID)
{
    pv_HWregBCH = (PVOID) BSPNAND_RemapRegister(CSP_BASE_REG_PA_BCH, 0x1000);
    // Check if virtual mapping failed
    if (pv_HWregBCH == NULL)
    {
        RETAILMSG(1, (_T("MapRegister:  BSPNAND_RemapRegister for BCH registers failed!\r\n")));
        goto cleanup;
    }
    
    pv_HWregGPMI = (PVOID) BSPNAND_RemapRegister(CSP_BASE_REG_PA_GPMI, 0x1000);
    // Check if virtual mapping failed
    if (pv_HWregGPMI == NULL)
    {
        RETAILMSG(1, (_T("MapRegister:  BSPNAND_RemapRegister for GPMI registers failed!\r\n")));
        goto cleanup;
    }
    
    pv_HWregDIGCTL = (PVOID) BSPNAND_RemapRegister(CSP_BASE_REG_PA_DIGCTL, 0x1000);
    // Check if virtual mapping failed
    if (pv_HWregDIGCTL == NULL)
    {
        RETAILMSG(1, (_T("MapRegister:  BSPNAND_RemapRegister for DIGCTL registers failed!\r\n")));
        goto cleanup;
    }
    
    return TRUE;
cleanup:
    UnMapRegister();
    return FALSE;   
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
    // Load the reset command
    // Most devices have the same reset command (0xFF)
    DmaGenericDescriptor[0].NandReset.tx_reset_command_buf[0] =
        g_CSPfi.CmdReset;;

    // Build the Reset Device DMA chain.
    DMA_SetupResetDesc(&DmaGenericDescriptor[0].NandReset,
                                dwCs);

    // Kick it off.
    DMA_Start((dma_cmd_t *)&DmaGenericDescriptor[0].NandReset, dwCs);

    DMA_Wait(NAND_RESET_TIMEOUT, (UINT8)dwCs+NAND0_APBH_CH);
    HALDelay();
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
    //int i ;
    *(UINT32*)(pReadIDDecode) = 0;
    // Change the default ReadID code to what is being passed in.
    DmaGenericDescriptor[0].NandReadID.NandReadIDSeed.txCLEByte = g_CSPfi.CmdReadId;
    DmaGenericDescriptor[0].NandReadID.NandReadIDSeed.txALEByte = 0x00;

    DMA_SetupReadIdDesc(&DmaGenericDescriptor[0].NandReadID, dwCs,
                       &DmaGenericDescriptor[0].NandReadID.NandReadIDSeed, pReadIDDecode);

    DMA_Start((dma_cmd_t *)&DmaGenericDescriptor[0].NandReadID, dwCs);

    DMA_Wait(NAND_RESET_TIMEOUT,(NAND0_APBH_CH+dwCs));

    HALDelay();
    
    return *(UINT32*)(pReadIDDecode);
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
    UNREFERENCED_PARAMETER(Enable);
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
    //Marley does not support interleave mode
    return FALSE;
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
    memcpy(&g_CSPfi, pFlashInfo, sizeof(FlashInfoExt));
    
    MapRegister();

    BSPNAND_ConfigIOMUX(MAX_NAND_DEVICES);
    
    pReadIDDecode = (BYTE*)DMA_MemAlloc(sizeof(DWORD)*2);       //allocate more than 6 bytes
    
    GPMI_Init();
    ECC_Init();
    DMA_Init(5);

    return &g_CSPfi;
}

VOID CSPNAND_PostInit(PFlashInfoExt pFlashInfo)
{
    memcpy(&g_CSPfi, pFlashInfo, sizeof(FlashInfoExt));
    
    ECC_ConfigLayout(pFlashInfo->fi.wDataBytesPerSector, pFlashInfo->SpareDataLength);

    GPMI_SetTiming(&pFlashInfo->timings, 0);
    
    pFlashPageBuf   = (UINT8*)DMA_MemAlloc(pFlashInfo->fi.wDataBytesPerSector + FLASH_SECTOR_INFO_BUFFER_SIZE);
    pFlashAuxBuf    = (UINT8*)((PBYTE)pFlashPageBuf + pFlashInfo->fi.wDataBytesPerSector);
    
    NANDBootReserved();
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
#ifdef SHIP_BUILD
    UNREFERENCED_PARAMETER(hFMD);
#else
    ERRORMSG(TRUE, (_T("FMD_Deinit: FMD Driver is unloading (%x)\r\n"), hFMD));

    ASSERT(hFMD != NULL);
#endif
    
    // Free global sector and meta-data buffers
    DMA_MemFree((ULONG)pFlashPageBuf);
    DMA_MemFree((ULONG)pFlashAuxBuf);
    UnMapRegister();
    
    return(TRUE);
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
    BOOL Status;
    UINT32 readSize;
    BOOL EnableECC = TRUE;
    BOOL PreservBBMark = FALSE;
    
    //  Sanity check
    if (!pSectorBuff && !pSectorInfoBuff) {
        ERRORMSG(TRUE, (_T("FMD_ReadSector: Invalid parameters!\r\n")));
        return FALSE;
    }
    
    //reading/writing to NCB area, we need to disable ECC.
    if(dwCs == 0 && startSectorAddr < (SECTOR_ADDR)CHIP_NCB_SEARCH_RANGE * g_CSPfi.fi.wSectorsPerBlock)
    {
        EnableECC = FALSE;
    }
    else
    {
        EnableECC = TRUE;
        PreservBBMark = TRUE;
    }
    
    if(!pSectorBuff)
    {
        //we will use same layout for 2K and 4K
        //thus we could simulate 4K page as 2K in bcb
        readSize = METADATA_SIZE + ECC0_2K4K_PAGE * 13 / 4;
    }
    else
    {
        readSize = g_CSPfi.SpareDataLength + g_CSPfi.fi.wDataBytesPerSector;
    }
    
    if(!EnableECC)
    {
        readSize = g_CSPfi.SpareDataLength + g_CSPfi.fi.wDataBytesPerSector;
    }
    
    Status = ReadNand(&DmaReadDescriptor[0],
                                   dwCs,
                                   0,
                                   startSectorAddr,
                                   readSize,
                                   EnableECC,
                                   (UINT8 *)pFlashPageBuf,
                                   (UINT8 *)pFlashAuxBuf);
        
    // Wait for the DMA to finish.
    // This is a complete DMA which includes the wait for data.
    Status = DMA_Wait(NAND_READ_PAGE_TIMEOUT, dwCs);

    // Check the ECC results.
    if ( TRUE == Status && EnableECC)
    {
        Status = ECC_IsCorrectable();
    }
    
    if (Status != TRUE)
    {
        ERRORMSG(TRUE, (_T("Failed Read cs=%d, sector=0x%x\r\n"), dwCs, startSectorAddr));
        return FALSE;
    }
    
    if(PreservBBMark)
    {
        DWORD dwOffsetByte;
        BYTE  OffsetBit;
        if(g_CSPfi.fi.wDataBytesPerSector == 2048)
        {
            dwOffsetByte = BBMarkByteOffsetInPageData_2K64;
            OffsetBit = BBMarkBitOffset_2K64;
        }
        else if(g_CSPfi.SpareDataLength == 128)
        {
            dwOffsetByte = BBMarkByteOffsetInPageData_4K128;
            OffsetBit = BBMarkBitOffset_4K128;
        }
        else
        {
            dwOffsetByte = BBMarkByteOffsetInPageData_4K218;
            OffsetBit = BBMarkBitOffset_4K218;
        }
        //do swap
        pFlashPageBuf[dwOffsetByte] = \
            (pFlashAuxBuf[0] << OffsetBit)|(pFlashPageBuf[dwOffsetByte] & (0xFF >> (8 - OffsetBit))) ;
            
        pFlashPageBuf[dwOffsetByte + 1] = \
            (pFlashAuxBuf[0] >> (8 - OffsetBit))|(pFlashPageBuf[dwOffsetByte + 1] & (0xFF << OffsetBit));
    }
    
    if(pSectorBuff)
    {
        memcpy(pSectorBuff,pFlashPageBuf,g_CSPfi.fi.wDataBytesPerSector);
    }
    
    if(pSectorInfoBuff)
    {
        memcpy(pSectorInfoBuff,pFlashAuxBuf + METADATA_EXTRA,sizeof(*pSectorInfoBuff));
        if(startSectorAddr < (SECTOR_ADDR)(CHIP_NCB_SEARCH_RANGE) * g_CSPfi.fi.wSectorsPerBlock)
        {
            //before LDLB area, we won't return bad block information
            //and always return block reserved.
            //TBD
            pSectorInfoBuff->bOEMReserved = (BYTE)~OEM_BLOCK_RESERVED;
            pSectorInfoBuff->bBadBlock = 0xff;
        }
    }

    return TRUE;
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
    DWORD wRowAddr;
    NAND_dma_block_erase_t * pNandDmaDescriptor;
    
    // The seed can be local because the DMA portion will complete before
    // leaving this function.
    pNandDmaDescriptor = &DmaEraseBlockDescriptor[0];

    // Use the 1st page of the block to calculate the Row address.
    wRowAddr = blockID * g_CSPfi.fi.wSectorsPerBlock;

    // Fill in the Row Address. (Can be 2 or 3 bytes)
    // The Address size will determine how many bytes are sent.
    pNandDmaDescriptor->NandEraseSeed.tx_block[0] =
        (UCHAR)(wRowAddr & 0xFF);
    pNandDmaDescriptor->NandEraseSeed.tx_block[1] =
        (UCHAR)((wRowAddr >> 8) & 0xFF);
    pNandDmaDescriptor->NandEraseSeed.tx_block[2] =
        (UCHAR)((wRowAddr >> 16) & 0xFF);

    // Load Command Code for Serial Data Input (0x80)
    pNandDmaDescriptor->NandEraseSeed.tx_cle1 = eNandProgCmdBlockErase;

    // Load Command Code for Page Program (0x10)
    pNandDmaDescriptor->NandEraseSeed.tx_cle2 = eNandProgCmdBlockErase_2ndCycle;

    // Load command and mask for GetStatus portion of DMA.
    pNandDmaDescriptor->NandEraseSeed.u8StatusCmd = eNandProgCmdReadStatus;

    // Modify the DMA descriptor that will program this sector.
    //SetupDMAEraseDesc(pNandDmaDescriptor, g_CSPfi.NumBlockCycles, dwCs);
    DMA_ModifyEraseDesc(pNandDmaDescriptor, g_CSPfi.NumBlockCycles, dwCs);
    
    DMA_Start((dma_cmd_t *)pNandDmaDescriptor, dwCs);

    return WaitForProgramDone(
        dwCs, NAND_ERASE_BLOCK_TIMEOUT,
        &(DmaEraseBlockDescriptor[0].NandEraseSeed.u16Status));

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
    UINT PageSize = g_CSPfi.fi.wDataBytesPerSector;
    BOOL EnableECC = TRUE;
    BOOL PreservBBMark = FALSE;
    
    if (!pSectorBuff && !pSectorInfoBuff) {
        ERRORMSG(TRUE, (_T("FMD_WriteSector: Invalid parameters!\r\n")));
        return FALSE;
    }
    
    //reading/writing to NCB area, we need to disable ECC.
    if(dwCs == 0 && startSectorAddr < (SECTOR_ADDR)CHIP_NCB_SEARCH_RANGE * g_CSPfi.fi.wSectorsPerBlock)
    {
        EnableECC = FALSE;
    }
    else
    {
        EnableECC = TRUE;
        PreservBBMark = TRUE;
    }
    
    memset(pFlashPageBuf,0xff,PageSize);
    memset(pFlashAuxBuf,0xFF,METADATA_SIZE);
    
    if(pSectorBuff)
    {
        memcpy(pFlashPageBuf,pSectorBuff,PageSize);
    }
    
    if(pSectorInfoBuff)
    {
        memcpy(pFlashAuxBuf + METADATA_EXTRA,pSectorInfoBuff,sizeof(*pSectorInfoBuff));
    }

    if(PreservBBMark)
    {
        DWORD dwOffsetByte;
        BYTE  OffsetBit;
        if(g_CSPfi.fi.wDataBytesPerSector == 2048)
        {
            dwOffsetByte = BBMarkByteOffsetInPageData_2K64;
            OffsetBit = BBMarkBitOffset_2K64;
        }
        else if(g_CSPfi.SpareDataLength == 128)
        {
            dwOffsetByte = BBMarkByteOffsetInPageData_4K128;
            OffsetBit = BBMarkBitOffset_4K128;
        }
        else
        {
            dwOffsetByte = BBMarkByteOffsetInPageData_4K218;
            OffsetBit = BBMarkBitOffset_4K218;
        }
        //do swap
        pFlashAuxBuf[0] = \
                (pFlashPageBuf[dwOffsetByte] >> OffsetBit) | (pFlashPageBuf[dwOffsetByte + 1] << (8 - OffsetBit));

        if(pSectorInfoBuff && pSectorInfoBuff->bBadBlock)
        {
            pFlashPageBuf[dwOffsetByte] &= ~(0xff << OffsetBit);
            pFlashPageBuf[dwOffsetByte + 1] &= ~(0xff >> (8 - OffsetBit));
        }
        else
        {
            pFlashPageBuf[dwOffsetByte] |= (0xff << OffsetBit);
            pFlashPageBuf[dwOffsetByte + 1] |= (0xff >> (8 - OffsetBit));
        }
    }
    
    WriteNand(
        &DmaProgramDescriptor[0],
        dwCs,
        0,
        startSectorAddr,
        g_CSPfi.SpareDataLength + g_CSPfi.fi.wDataBytesPerSector,
        EnableECC,
        (UINT8 *)pFlashPageBuf,
        (UINT8 *)pFlashAuxBuf);

    return WaitForProgramDone(
        dwCs,
        NAND_WRITE_PAGE_TIMEOUT,
        &(DmaProgramDescriptor[0].NandProgSeed.u16Status));

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
    return 0;
}
