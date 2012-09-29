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
#include "NANDTYPES.h"
//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// External Functions
extern BOOL BBT_Init();
extern void BBT_SetBlockBadStatus(DWORD dwCs, BLOCK_ID blockID);
extern BOOL BBT_GetBlockBadStatus(DWORD dwCs, BLOCK_ID blockID);
extern "C" BOOL BSP_OEMIoControl(DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, 
    PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned);
// CS&ZHL DEC-13-2011: add for Block status
extern "C" DWORD BSP_NANDImageFileSystemOffset();

//-----------------------------------------------------------------------------
// Global Variables
FlashInfoExt g_FlashInfoExt;
BOOL         g_bInterLeaveMode = FALSE;

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
//  Function: CalcBlock
//
//  This function computes the real block num & cs.
//
//  Parameters:
//      blockID
//          [in] passed in from upper level to FMD layer
//
//
//  Returns:
//      cs num
//
//-----------------------------------------------------------------------------
static DWORD CalcBlock(BLOCK_ID& blockID)
{
    DWORD dwCs = 0;
    
    if(g_FlashInfoExt.NumberOfChip > 1 && !g_bInterLeaveMode)
    {
        dwCs = blockID / g_FlashInfoExt.fi.dwNumBlocks;
        blockID = blockID % g_FlashInfoExt.fi.dwNumBlocks;
    }
    else 
    {
        if(g_bInterLeaveMode)
        {
            dwCs = INTERLEAVE_CS;
        }
    }
    
    return dwCs;
}

//-----------------------------------------------------------------------------
//
//  Function: CalcSector
//
//  This function computes the real sector num & cs.
//
//  Parameters:
//      sectorID
//          [in] passed in from upper level to FMD layer
//
//
//  Returns:
//      cs num
//
//-----------------------------------------------------------------------------
static DWORD CalcSector(SECTOR_ADDR& sectorID)
{
    DWORD dwCs = 0;
    
    if(g_FlashInfoExt.NumberOfChip > 1 && !g_bInterLeaveMode)
    {
        DWORD dwTemp = g_FlashInfoExt.fi.dwNumBlocks * g_FlashInfoExt.fi.wSectorsPerBlock / g_FlashInfoExt.ClusterCount;
        dwCs = sectorID / dwTemp; //need optimize
        sectorID = sectorID % dwTemp * g_FlashInfoExt.ClusterCount;
    }
    else
    {
        if(g_bInterLeaveMode)
        {
            dwCs = INTERLEAVE_CS;
        }
        sectorID = sectorID * g_FlashInfoExt.ClusterCount;    
    }
    
    return dwCs;
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_Init
//
//  This function initializes the flash memory of a device.
//
//  Parameters:
//      lpActiveReg
//          [in] Pointer to the active registry string used to find device 
//          information from the registry. Set to NULL if not needed. 
//
//      pRegIn
//          [in] Pointer to a PCI_REG_INFO structure. Used to find flash 
//          hardware on PCI hardware. Set to NULL if not needed.
//
//      pRegOut
//          [in/out] Pointer to a PCI_REG_INFO structure. Used to return 
//          flash information. Set to NULL if not needed.
//
//  Returns:
//      A handle that can be used in a call to FMD_Deinit. It is the 
//      responsibility of the specific flash media driver (FMD) implementation 
//      to determine what this value represents. A value of zero (0) 
//      represents failure.
//
//-----------------------------------------------------------------------------
PVOID FMD_Init(LPCTSTR lpActiveReg, PPCI_REG_INFO pRegIn, PPCI_REG_INFO pRegOut)
{
    DWORD i, j;
    PVOID pResult;
    DWORD FullCode;
    
    UNREFERENCED_PARAMETER(lpActiveReg);
    UNREFERENCED_PARAMETER(pRegIn);
    UNREFERENCED_PARAMETER(pRegOut);

	RETAILMSG(TRUE, (TEXT("->FMD_Init\r\n")));
    
    // 0. Setup global variables. Also may add more software logic here in future
    memset(&g_FlashInfoExt, 0, sizeof(g_FlashInfoExt));
    BSPNAND_GetFlashInfo(&g_FlashInfoExt);
    
    // 1. initial hardware
    if(g_FlashInfoExt.AutoDec.Enable)
    {
        g_FlashInfoExt.CmdReset = 0xff;
        g_FlashInfoExt.CmdReadId = 0x90;
    }
    
    pResult = CSPNAND_Init(&g_FlashInfoExt);
    if(pResult == NULL)
    {
        goto cleanUp;
    }
    
    // 2. Auto detect NAND chip if it's enabled
    if(g_FlashInfoExt.AutoDec.Enable)
    {
        g_FlashInfoExt.NumberOfChip = 0;
        //BSP enables auto detect mode
        for(i = 0; i < g_FlashInfoExt.AutoDec.CsSearchRange; i++)
        {
            CSPNAND_Reset(i);
            FullCode = CSPNAND_ReadID(i);
            //only support multi-nand with same type
            //only support multi-nand connected continously
            for(j = 0; j < sizeof(ChipInfo)/sizeof(ChipInfo[0]); j++)
            {
                if(!memcmp(&FullCode, ChipInfo[j].NANDCode, sizeof(FullCode)))     
                {
                    g_FlashInfoExt.NumberOfChip++;
                    if(g_FlashInfoExt.NumberOfChip == 1)
                    {
                        memcpy(&g_FlashInfoExt, &ChipInfo[j], sizeof(NandChipInfo));
                        g_FlashInfoExt.BBMarkPage = ChipInfo[j].BBMarkPage;
                    }
                    break;
                }
                else if(j == (sizeof(ChipInfo)/sizeof(ChipInfo[0]) - 1))
                {
                    if(!g_FlashInfoExt.NumberOfChip)
                    {
                        ERRORMSG(TRUE, (_T("FMD_Init::no NAND type matched! IDCode = 0x%08X\r\n"), FullCode));
                        return FALSE;   
                    }
                    goto PostInit;    
                }
            }
        }
    }
    else
    {
        for(i = 0; i < g_FlashInfoExt.NumberOfChip; i++)
        {
            CSPNAND_Reset(i);
            if((WORD)CSPNAND_ReadID(i) != g_FlashInfoExt.NANDIDCode)
            {
                pResult = NULL;
                goto cleanUp;
            }
        }
    }
PostInit:
    RETAILMSG(TRUE, (_T("FMD_Init::NumberOfChip=%d\r\n"),g_FlashInfoExt.NumberOfChip));
    CSPNAND_PostInit(&g_FlashInfoExt);			//CS&ZHL DEC-9-2011: will call NandBootReserved()
    if(g_FlashInfoExt.ILSupport && CSPNAND_ILSupport())
    {
        //silicon supports interleave mode & bsp needs interleave mode
        g_bInterLeaveMode = TRUE;
		RETAILMSG(TRUE, (TEXT("FMD_Init::g_bInterLeaveMode = TRUE\r\n")));
    }

    if(!BBT_Init())
    {
        RETAILMSG(TRUE, (TEXT("InitializeBBT Fail\r\n")));
    }

cleanUp:
	RETAILMSG(TRUE, (TEXT("<-FMD_Init 0x%x\r\n"), pResult));
    return pResult; 
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_Deinit
//
//  This function de-initializes the flash chip.
//
//  Parameters:
//      hFMD 
//          [in] The handle returned from FMD_Init. 
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL FMD_Deinit(PVOID hFMD)
{
    BOOL rc;
    
    rc = CSPNAND_Deinit(hFMD);
    
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_ReadSector
//
//  This function reads the requested sector data and metadata from the 
//  flash media.
//
//  Parameters:
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
//      dwNumSectors 
//          [in] Number of sectors to read.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL FMD_ReadSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, 
    PSectorInfo pSectorInfoBuff, DWORD dwNumSectors)
{
    BOOL rc = FALSE;
    DWORD dwCs = 0; 
    SECTOR_ADDR Sector;
    DWORD dwNumOfClusters;
    
    if (!pSectorBuff && !pSectorInfoBuff)
    {
        ERRORMSG(TRUE, (_T("Sector/Info buffers are NULL\r\n")));
        goto cleanUp;
    }
    
    while (dwNumSectors--)
    {
        //we have multi-cs chips, needs verify this feather
        Sector = startSectorAddr;
        dwCs = CalcSector(Sector);
        
        dwNumOfClusters = g_FlashInfoExt.ClusterCount;

        if(pSectorBuff)
        {
            while(dwNumOfClusters--)
            {
                if(!CSPNAND_ReadSector(dwCs, Sector, pSectorBuff, pSectorInfoBuff))
                {
                    goto cleanUp;
                }
                
                if(g_bInterLeaveMode)
                {
                    pSectorBuff += g_FlashInfoExt.fi.wDataBytesPerSector * g_FlashInfoExt.NumberOfChip;
                }
                else
                {
                    pSectorBuff += g_FlashInfoExt.fi.wDataBytesPerSector;
                }
                    
                Sector++;
            }
        }
        else
        {
            Sector += dwNumOfClusters - 1;
            if(!CSPNAND_ReadSector(dwCs, Sector, NULL, pSectorInfoBuff))
            {
                goto cleanUp;
            }
        }

        if(pSectorInfoBuff)
        {
            pSectorInfoBuff++;
        }
        
        startSectorAddr++;
    }
        
    rc = TRUE;

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: FMD_EraseBlock
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
BOOL FMD_EraseBlock(BLOCK_ID blockID)
{
    BOOL rc;
    DWORD dwCs = CalcBlock(blockID);
    
    rc = CSPNAND_EraseBlock(dwCs, blockID);
    
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: FMD_WriteSector
//
//  This function writes the requested sector data and metadata to the 
//  flash media.
//
//  Parameters:
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
BOOL FMD_WriteSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, 
    PSectorInfo pSectorInfoBuff, DWORD dwNumSectors)
{
    BOOL rc = FALSE;
    DWORD dwCs = 0; 
    SECTOR_ADDR Sector;
    DWORD dwNumOfClusters;
    
    if (!pSectorBuff && !pSectorInfoBuff)
    {
        ERRORMSG(TRUE, (_T("Sector/Info buffers are NULL\r\n")));
        goto cleanUp;
    }
    
    while (dwNumSectors--)
    {
        //we have multi-cs chips, needs verify this feather
        Sector = startSectorAddr;
        dwCs = CalcSector(Sector);
        
        dwNumOfClusters = g_FlashInfoExt.ClusterCount;

        if(pSectorBuff)
        {
            while(dwNumOfClusters--)
            {
                if(!CSPNAND_WriteSector(dwCs, Sector, pSectorBuff, pSectorInfoBuff))
                {
                    goto cleanUp;
                }
                
                if(g_bInterLeaveMode)
                {
                    pSectorBuff += g_FlashInfoExt.fi.wDataBytesPerSector * g_FlashInfoExt.NumberOfChip;
                }
                else
                {
                    pSectorBuff += g_FlashInfoExt.fi.wDataBytesPerSector;
                }
                    
                Sector++;
            }
        }
        else
        {
            Sector += dwNumOfClusters - 1;
            if(!CSPNAND_WriteSector(dwCs, Sector, NULL, pSectorInfoBuff))
            {
                goto cleanUp;
            }
        }

        if(pSectorInfoBuff)
        {
            pSectorInfoBuff++;
        }
        
        startSectorAddr++;
    }
        
    rc = TRUE;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_PowerUp
//
//  This function restores power to the flash memory device, if applicable.
//
//  Parameters:
//      None
//
//  Returns:  
//      None
//
//-----------------------------------------------------------------------------
VOID FMD_PowerUp(VOID)
{
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_PowerDown
//
//  This function suspends power to the flash memory device, if applicable.
//
//  Parameters:
//      None
//
//  Returns:  
//      None
//
//-----------------------------------------------------------------------------
VOID FMD_PowerDown(VOID)
{
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_OEMIoControl
//
//  This function implements user-defined commands for the flash memory device. 
//
//  Parameters:
//      dwIoControlCode 
//          [in] The control code specifying the command to execute.
//
//      pInBuf
//          [in] Long pointer to a buffer that contains the data required to 
//          perform the operation. Set to NULL if the dwIoControlCode parameter 
//          specifies an operation that does not require input data.
//                        
//      nInBufSize
//          [in] Size, in bytes, of the buffer pointed to by pInBuf. 
//
//      pOutBuf
//          [out] Long pointer to a buffer that receives the output data for 
//          the operation. Set to NULL if the dwIoControlCode parameter 
//          specifies an operation that does not produce output data.
//
//      nOutBufSize
//          [in] Size, in bytes, of the buffer pointed to by pOutBuf.
//
//      pBytesReturned
//          [out] Long pointer to a variable that receives the size, in bytes, 
//          of the data stored into the buffer pointed to by pOutBuf. Even 
//          when an operation produces no output data and pOutBuf is set to 
//          NULL, the DeviceIoControl function uses the variable pointed to 
//          by pBytesReturned. After such an operation, the value of the 
//          variable has no meaning. 
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL FMD_OEMIoControl(DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, 
    PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned)
{
	return BSP_OEMIoControl(dwIoControlCode,pInBuf, nInBufSize, pOutBuf, nOutBufSize, pBytesReturned);
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_GetInfo
//
//  This function determines the size characteristics for the flash memory 
//  device.
//
//  Parameters:
//      pFlashInfo 
//          [out] A pointer to a structure that contains the size 
//          characteristics for the flash memory device.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL FMD_GetInfo(PFlashInfo pFlashInfo)
{
	static BOOL	bFirstTime = TRUE;

	if(bFirstTime)
	{
		RETAILMSG(TRUE,(_T("->FMD_GetInfo\r\n")));
	}

	if (!pFlashInfo)
	{
		RETAILMSG(TRUE,(_T("FMD_GetInfo::pFlashInfo = NULL!\r\n")));
        return(FALSE);
	}
    
    if(g_bInterLeaveMode)
    {
        pFlashInfo->dwNumBlocks     = g_FlashInfoExt.fi.dwNumBlocks;
        pFlashInfo->wDataBytesPerSector = g_FlashInfoExt.fi.wDataBytesPerSector * g_FlashInfoExt.ClusterCount * g_FlashInfoExt.NumberOfChip;
    }
    else
    {
        pFlashInfo->dwNumBlocks     = g_FlashInfoExt.fi.dwNumBlocks * g_FlashInfoExt.NumberOfChip;
        pFlashInfo->wDataBytesPerSector = g_FlashInfoExt.fi.wDataBytesPerSector * g_FlashInfoExt.ClusterCount;
    }
    pFlashInfo->flashType           = NAND;
    pFlashInfo->wSectorsPerBlock    = g_FlashInfoExt.fi.wSectorsPerBlock / g_FlashInfoExt.ClusterCount;
    pFlashInfo->dwBytesPerBlock     = (pFlashInfo->wSectorsPerBlock * pFlashInfo->wDataBytesPerSector);

	if(bFirstTime)
	{
		RETAILMSG(TRUE,(_T("FMD_GetInfo::dwNumBlocks = %d\r\n"), pFlashInfo->dwNumBlocks));
		RETAILMSG(TRUE,(_T("FMD_GetInfo::wSectorsPerBlock = %d\r\n"), pFlashInfo->wSectorsPerBlock));
		RETAILMSG(TRUE,(_T("FMD_GetInfo::wDataBytesPerSector = %d\r\n"), pFlashInfo->wDataBytesPerSector));
		RETAILMSG(TRUE,(_T("FMD_GetInfo::dwBytesPerBlock = %d\r\n"), pFlashInfo->dwBytesPerBlock));
		bFirstTime = FALSE;
		RETAILMSG(TRUE,(_T("<-FMD_GetInfo\r\n")));
	}

    return(TRUE);
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_GetBlockStatus
//
//  This function returns the status of a block.
//
//  Parameters:
//      blockID 
//          [in] The block number used to check status.
//
//  Returns:  
//      Flags to describe the status of the block.
//
//-----------------------------------------------------------------------------
DWORD FMD_GetBlockStatus(BLOCK_ID blockID)
{
    DWORD dwCs = CalcBlock(blockID);
    SECTOR_ADDR Sector = (blockID * g_FlashInfoExt.fi.wSectorsPerBlock);
    SectorInfo SI;
    DWORD dwResult = 0;
	// CS&ZHL DEC-13-2011: FileSystemOffset is an offset in byte address
	//DWORD dwFileSysStartBlock = (BSP_NANDImageFileSystemOffset() + g_FlashInfoExt.fi.dwBytesPerBlock - 1) / g_FlashInfoExt.fi.dwBytesPerBlock;
    
    // Check Bad Block
    if(BBT_GetBlockBadStatus(dwCs, blockID))
    {
        dwResult = BLOCK_STATUS_BAD;
        goto cleanUp;
    }

    if (!CSPNAND_ReadSector(dwCs, Sector, NULL, &SI))
    {
        //DEBUGMSG(TRUE,(_T("FMD_GetBlockStatus : CSPNAND_ReadSector Error : Sector = 0x%X\r\n"), Sector));
        RETAILMSG(TRUE,(_T("FMD_GetBlockStatus : CSPNAND_ReadSector Error : Sector = 0x%X\r\n"), Sector));
		// CS&ZHL DEC-9-2011: make as bad block
        dwResult = BLOCK_STATUS_BAD;
        goto cleanUp;
    }
    
    if (!(SI.bOEMReserved & OEM_BLOCK_READONLY))  
    {
        DEBUGMSG(TRUE,(_T("BLOCK_STATUS_READONLY\r\n")));
        dwResult |= BLOCK_STATUS_READONLY;
    }
    
    if (!(SI.bOEMReserved & OEM_BLOCK_RESERVED)) 
    { 
        DEBUGMSG(TRUE,(_T("BLOCK_STATUS_RESERVED\r\n")));
        dwResult |= BLOCK_STATUS_RESERVED;
    }
    
cleanUp:
    if(dwResult == BLOCK_STATUS_BAD)
	{
        DEBUGMSG(TRUE, (_T("Bad Block found: Block No. = 0x%x\r\n"), blockID)); 
		//RETAILMSG(TRUE, (_T("FMD_GetBlockStatus::Bad Block 0x%x\r\n"), blockID)); 
	}
	//else if(blockID < dwFileSysStartBlock)
	//{
    //    dwResult |= BLOCK_STATUS_RESERVED;
    //    goto cleanUp;
	//}

    return(dwResult);   
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_SetBlockStatus
//
//  This function sets the status of a block.
//
//  Parameters:
//      blockID 
//          [in] The block number used to set status. 
//
//      dwStatus
//          [in] The status value to set.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL FMD_SetBlockStatus(BLOCK_ID blockID, DWORD dwStatus)
{
    DWORD dwCs = CalcBlock(blockID);
    SECTOR_ADDR Sector = (blockID * g_FlashInfoExt.fi.wSectorsPerBlock);
    SectorInfo SI;
    BOOL rc = FALSE;
    
    if (dwStatus & (BLOCK_STATUS_BAD | BLOCK_STATUS_READONLY | BLOCK_STATUS_RESERVED)) 
    {
        memset(&SI, 0xff, sizeof(SectorInfo));
        
        if (dwStatus & BLOCK_STATUS_BAD) 
        {
            BBT_SetBlockBadStatus(dwCs, blockID);
            rc = TRUE;
            CSPNAND_EraseBlock(dwCs, blockID);
            SI.bBadBlock = 0x00;
            Sector += g_FlashInfoExt.BBMarkPage[0];
            CSPNAND_WriteSector(dwCs, Sector, NULL, &SI);
            goto cleanUp;
        }
        
        if (dwStatus & BLOCK_STATUS_READONLY) 
        {
            SI.bOEMReserved &= ~OEM_BLOCK_READONLY;
        }
        
        if (dwStatus & BLOCK_STATUS_RESERVED) 
        {
            SI.bOEMReserved &= ~OEM_BLOCK_RESERVED;
        }

        if (!CSPNAND_WriteSector(dwCs, Sector, NULL, &SI)) 
        {
            goto cleanUp;
        }
    }   
    rc = TRUE; 
    
cleanUp:    

    return rc;
}


