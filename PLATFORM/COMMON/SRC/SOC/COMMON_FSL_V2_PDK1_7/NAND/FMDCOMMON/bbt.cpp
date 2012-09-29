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

//-----------------------------------------------------------------------------
// Defines
#define BBT_SIGNATURE   0x5A3B5A3B
#define BBT_MAX_NESTED  3

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// External Variables
extern FlashInfoExt g_FlashInfoExt;
extern BOOL         g_bInterLeaveMode;

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables
static BBTBuffer g_BBTBuffer;           // BBT Buffer


//-----------------------------------------------------------------------------
// Local Functions
static BOOL ReadBBTSignature(DWORD dwCs, SECTOR_ADDR startSectorAddr);
static BOOL WriteBBTSignature(DWORD dwCs, SECTOR_ADDR startSectorAddr);
static BOOL WriteBBT(DWORD dwBBTBlock,DWORD dwBBTNum);
static BOOL BBT_EraseOperation(DWORD dwCs, DWORD dwStartBlock);


//-----------------------------------------------------------------------------
//
//  Function: ReadBlockBadStatus
//
//  This function shows whether the block is bad.
//
//  Parameters:
//      dwCs
//          [in] chip selection.
//
//      blockID
//          [in] block number.  
//
//  Returns:
//      TRUE if block is bad.
//      FALSE if block is not bad.
//
//-----------------------------------------------------------------------------
static BOOL ReadBlockBadStatus(DWORD dwCs, BLOCK_ID blockID)
{
    SECTOR_ADDR SectorAddr;
    SectorInfo  SI;
    BOOL        rc = TRUE;
    
    for(BYTE i = 0; i < g_FlashInfoExt.BBMarkNum; i++)
    {        
        SectorAddr = blockID * g_FlashInfoExt.fi.wSectorsPerBlock + g_FlashInfoExt.BBMarkPage[i];
                
        if(!CSPNAND_ReadSector(dwCs, SectorAddr, g_BBTBuffer.pSectorBuf, &SI))
        {
            goto cleanUp;
        }

        if(g_FlashInfoExt.ECCType == ECC_BCH)
        {
            if(SI.bBadBlock == 0xFF)
            {
                rc = FALSE;
            }
            goto cleanUp;
        }

        UINT32 Status = CSPNAND_GetECCStatus();
        if(Status == 0)//No ECC error
        {
            if(SI.bBadBlock != 0xFF)
            {
                DEBUGMSG(TRUE, (TEXT("ReadBlockBadStatus 0x%X, ECC Status = 0x%X, Goto Bad\r\n"), blockID, Status));
                goto cleanUp;
            }
        }
        else
        {
            BOOL result = TRUE;
            SectorInfo SITemp;
            //There must be some bit error in this 512+16 section
            //We should copy the data in backup buffer, turn off ECC, and re-read out this sector 
            //&& compare with the data in backup buffer to see if the bit error is in bad block flag byte.
            //First of all, we should check if all data is 0xFF, if it is not, then it should be programmed before which indicates it is a good block.
            //DEBUGMSG(TRUE, (TEXT("ReadBlockBadStatus 0x%X, ECC Status = 0x%X\r\n"), blockID, Status));
            for(WORD j = 0; j < g_FlashInfoExt.fi.wDataBytesPerSector; j += sizeof(DWORD))
            {
                if(0xFFFFFFFF != *(DWORD *)(g_BBTBuffer.pSectorBuf + j))
                {
                    result = FALSE;
                    break;
                }
            }

            if(result)
            {
                memset(&SITemp, 0xFF, sizeof(SectorInfo));
                if(memcmp(&SI, &SITemp, sizeof(SectorInfo)))
                {
                    result = FALSE;
                }
            }

            if(result)
            {
                CSPNAND_EnableECC(FALSE);
                CSPNAND_ReadSector(dwCs, SectorAddr, NULL, &SITemp);
                CSPNAND_EnableECC(TRUE);

                DEBUGMSG(TRUE, (TEXT("bBadBlock : ECC Enable 0x%X, ECC Disable 0x%X\r\n"), SI.bBadBlock, SITemp.bBadBlock));
                if(SI.bBadBlock != SITemp.bBadBlock)
                {
                    goto cleanUp;
                }
            }
        }
    }
    
    rc = FALSE;
    
cleanUp:
    return rc;    
}


//-----------------------------------------------------------------------------
//
//  Function: SetBlockBadStatus
//
//  This function marks the bad block in the BBT.
//
//  Parameters:
//      blockID
//          [in] block number.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void SetBlockBadStatus(BLOCK_ID blockID)
{
    BYTE *pBBT = g_BBTBuffer.pBBTFirstBuf;
    
    if(pBBT != NULL)
    {
        *(pBBT + blockID / 8) |= (1 << (blockID % 8));
    }

    DEBUGMSG(TRUE, (TEXT("SetBlockBadStatus : bad block 0x%X\r\n"), blockID));
}


//-----------------------------------------------------------------------------
//
//  Function: GetBlockBadStatus
//
//  This function shows whether the block is bad.
//
//  Parameters:
//      blockID
//          [in] block number.
//
//  Returns:
//      TRUE if block is bad.
//      FALSE if block is not bad.
//
//-----------------------------------------------------------------------------
static BOOL GetBlockBadStatus(BLOCK_ID blockID)
{
    BOOL rc = TRUE;
    BYTE *pBBT = g_BBTBuffer.pBBTFirstBuf;
    
    if(pBBT != NULL)
    {
        if(*(pBBT + blockID / 8) & (1 << (blockID % 8)))
        {
            DEBUGMSG(TRUE, (TEXT("GetBlockBadStatus : bad block 0x%X\r\n"), *(pBBT + blockID / 8)));
            goto cleanUp;
        }
    }
    
    rc = FALSE;

cleanUp:
    return rc;
} 

//-----------------------------------------------------------------------------
//
//  Function: BBT_GetNumSector
//
//  This function gets the number of sectors used to store the BBT.
//
//  Parameters:
//      None.  
//
//  Returns:
//      The number of sectors that need to store the BBT.
//
//-----------------------------------------------------------------------------
static DWORD BBT_GetNumSector()
{
    WORD wDataBytes = g_FlashInfoExt.fi.wDataBytesPerSector;
    return (g_BBTBuffer.dwBBTFirstSize + wDataBytes - 1) / wDataBytes;
}


//-----------------------------------------------------------------------------
//
//  Function: BBT_GetSectorAddr
//
//  This function gets the changed sector address.
//
//  Parameters:
//      SectorAddr
//          [in] sector address.
//
//  Returns:
//      The new sector address.
//
//-----------------------------------------------------------------------------
static DWORD BBT_GetSectorAddr(SECTOR_ADDR SectorAddr)
{
    SECTOR_ADDR SectorAddrOffset = SectorAddr % g_FlashInfoExt.fi.wSectorsPerBlock;
    for(BYTE bNumPage = 0; bNumPage < g_FlashInfoExt.BBMarkNum; bNumPage++)
    {
        if(SectorAddrOffset == g_FlashInfoExt.BBMarkPage[bNumPage])
        {
            SectorAddr++;
            SectorAddrOffset++;
        }
    }

    return SectorAddr;
}


//-----------------------------------------------------------------------------
//
//  Function: BBT_GetBlockBadStatus
//
//  This function gets the block status.
//
//  Parameters:
//      dwCs
//          [in] chip selection.
//
//      blockID
//          [in] block number.
// 
//  Returns:
//      TRUE if block is bad
//      FALSE if block is not bad
//
//-----------------------------------------------------------------------------
BOOL BBT_GetBlockBadStatus(DWORD dwCs, BLOCK_ID blockID)
{
    // Check the BBT Buffer
    if(g_BBTBuffer.pBBTFirstBuf == NULL)
    {
        return FALSE;
    }
    
    if(dwCs == INTERLEAVE_CS)
    {
        dwCs = 0;
    }
    return GetBlockBadStatus(blockID + dwCs * g_FlashInfoExt.fi.dwNumBlocks);
}


//-----------------------------------------------------------------------------
//
//  Function: BBT_SetBlockBadStatus
//
//  This function sets the block status.
//
//  Parameters:
//      dwCs
//          [in] chip selection.
//
//      blockID
//          [in] block number.
// 
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void BBT_SetBlockBadStatus(DWORD dwCs, BLOCK_ID blockID)
{
    // Check the BBT Buffer
    if(g_BBTBuffer.pBBTFirstBuf == NULL)
    {
        return;
    }
    
    if(dwCs == INTERLEAVE_CS)
    {
        dwCs = 0;
    }
    SetBlockBadStatus(blockID + dwCs * g_FlashInfoExt.fi.dwNumBlocks);
    WriteBBT(0, 2);
}


//-----------------------------------------------------------------------------
//
//  Function: BBT_GetBlockReservedStatus
//
//  This function gets the block reserved status.
//
//  Parameters:
//      dwCs
//          [in] chip selection.
//
//      blockID
//          [in] block number.
// 
//  Returns:
//      
//      TRUE if block is reserved.
//      FALSE if block is not reserved.
//
//-----------------------------------------------------------------------------
static BOOL BBT_GetBlockReservedStatus(DWORD dwCs, BLOCK_ID blockID)
{
    SECTOR_ADDR Sector = (blockID * g_FlashInfoExt.fi.wSectorsPerBlock);
    SectorInfo SI;
    BOOL rc = FALSE;

    if(!CSPNAND_ReadSector(dwCs, Sector, NULL, &SI))
    {
        DEBUGMSG(TRUE, (_T("BBT_GetBlockReservedStatus : CSPNAND_ReadSector Error : Sector = 0x%X\r\n"), Sector));
        if(!BBT_EraseOperation(dwCs, blockID))
        {
            DEBUGMSG(TRUE, (_T("BBT_GetBlockReservedStatus : BBT_EraseOperation Error : blockID = 0x%X\r\n"), blockID));
        }

        goto cleanUp;
    }
    
    if(SI.bOEMReserved & OEM_BLOCK_RESERVED) 
    { 
        goto cleanUp;
    }
 
    rc = TRUE;
    
cleanUp:
    return rc;   
}


//-----------------------------------------------------------------------------
//
//  Function: BBT_SetBlockReservedStatus
//
//  This function sets the block reserved status.
//
//  Parameters:
//      dwCs
//          [in] chip selection.
//
//      blockID
//          [in] block number.
// 
//  Returns:
//      TRUE if successful.
//      FALSE if not successful.
//
//-----------------------------------------------------------------------------
static BOOL BBT_SetBlockReservedStatus(DWORD dwCs, BLOCK_ID blockID)
{
    SECTOR_ADDR Sector = (blockID * g_FlashInfoExt.fi.wSectorsPerBlock);
    SectorInfo SI;
    BOOL rc = FALSE;

    DEBUGMSG(TRUE, (TEXT("BBT_SetBlockReservedStatus : block = 0x%X\r\n"), blockID));
    // Erase the block
    if(!BBT_EraseOperation(dwCs, blockID))
    {
        DEBUGMSG(TRUE, (TEXT("BBT_SetBlockReservedStatus : BBT_EraseOperation Fail\r\n")));
        goto cleanUp;
    }

    // Set SectorInfo
    memset(&SI, 0xff, sizeof(SectorInfo));     
    SI.bOEMReserved &= ~OEM_BLOCK_RESERVED;

    // Write the reserved status
    if(!CSPNAND_WriteSector(dwCs, Sector, NULL, &SI)) 
    {
        if(!BBT_EraseOperation(dwCs, blockID))
        {
            DEBUGMSG(TRUE, (TEXT("BBT_SetBlockReservedStatus : BBT_EraseOperation Fail\r\n")));
        }

        goto cleanUp;
    }
    
    rc = TRUE; 
    
cleanUp:    
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: BBT_ReadOperation
//
//  This function implements the read operation.
//
//  Parameters:
//      dwCs
//          [in] chip selection.
//
//      dwStartBlock
//          [in] block number.
//
//      pBBTBuff
//          [in] buffer pointer.
// 
//  Returns:
//      TRUE if successful.
//      FALSE if not successful.
//
//-----------------------------------------------------------------------------
static BOOL BBT_ReadOperation(DWORD dwCs, DWORD dwStartBlock, LPBYTE pBBTBuff)
{
    SECTOR_ADDR SectorAddr;
    SectorInfo SI;
    DWORD dwBBTNumSectors;
    BOOL rc = FALSE;

    if(pBBTBuff == NULL)
    {
        goto cleanUp;
    }

    // Calculate the sector address
    SectorAddr = dwStartBlock * g_FlashInfoExt.fi.wSectorsPerBlock + 1;
    SectorAddr = BBT_GetSectorAddr(SectorAddr);
    
    DEBUGMSG(TRUE, (TEXT("BBT_ReadOperation : dwStartBlock = 0x%X, SectorAddr = 0x%X\r\n"), dwStartBlock, SectorAddr));
    if(!ReadBBTSignature(dwCs, SectorAddr))
    {
        goto cleanUp;
    }

    // Calculate sector number of BBT
    dwBBTNumSectors = BBT_GetNumSector();
                
    // Calculate the sector address
    SectorAddr++;
    SectorAddr = BBT_GetSectorAddr(SectorAddr);
    
    while(dwBBTNumSectors--)
    {
        if(!CSPNAND_ReadSector(dwCs, SectorAddr, pBBTBuff, &SI))
        {
            if(!ReadBlockBadStatus(dwCs, dwStartBlock))
            {
                if(!BBT_EraseOperation(dwCs, dwStartBlock))
                {
                    DEBUGMSG(TRUE, (TEXT("BBT_ReadOperation : BBT_EraseOperation 0x%X Fail\r\n"), dwStartBlock));
                }
            }

            goto cleanUp;
        }
        else
        {
            pBBTBuff += g_FlashInfoExt.fi.wDataBytesPerSector;  
        }

        // Calculate the sector address
        SectorAddr++;
        SectorAddr = BBT_GetSectorAddr(SectorAddr);
    }
    
    // Check BBT Signature
    if(!ReadBBTSignature(dwCs, SectorAddr))
    { 
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: BBT_WriteOperation
//
//  This function implements the write operation.
//
//  Parameters:
//      dwCs
//          [in] chip selection.
//
//      dwStartBlock
//          [in] block number.
// 
//  Returns:    
//      TRUE if successful.
//      FALSE if not successful.
//
//-----------------------------------------------------------------------------
static BOOL BBT_WriteOperation(DWORD dwCs, DWORD dwStartBlock, LPBYTE pBBTBuff)
{
    SECTOR_ADDR SectorAddr;
    SectorInfo SI;
    DWORD dwBBTNumSectors;
    BOOL rc = FALSE;

    if(pBBTBuff == NULL)
    {
        goto cleanUp;
    }

    if(!BBT_EraseOperation(dwCs, dwStartBlock))
    {
        DEBUGMSG(TRUE, (TEXT("BBT_WriteOperation : BBT_EraseOperation Fail\r\n")));
        goto cleanUp;
    }

    // Initialize the SectorInfo
    memset(&SI, 0xFF, sizeof(SI));
    SI.bOEMReserved &= ~OEM_BLOCK_RESERVED;

    // Set reserved status
    SectorAddr = dwStartBlock * g_FlashInfoExt.fi.wSectorsPerBlock;
    if(!CSPNAND_WriteSector(dwCs, SectorAddr, NULL, &SI))
    {
        DEBUGMSG(TRUE, (TEXT("CSPNAND_WriteSector Fail\r\n")));

        if(!BBT_EraseOperation(dwCs, dwStartBlock))
        {
            DEBUGMSG(TRUE, (TEXT("BBT_EraseOperation Fail\r\n")));
        }
            
        goto cleanUp;
    }

    // Calculate the sector address
    SectorAddr++;
    SectorAddr = BBT_GetSectorAddr(SectorAddr);
    
    DEBUGMSG(TRUE, (TEXT("BBT_WriteOperation : dwStartBlock = 0x%X, SectorAddr = 0x%X\r\n"), dwStartBlock, SectorAddr));
    if(!WriteBBTSignature(dwCs, SectorAddr))
    {
        DEBUGMSG(TRUE, (TEXT("WriteBBTSignature Fail\r\n")));
        goto cleanUp;
    }

    // Calculate the sector number of BBT
    dwBBTNumSectors = BBT_GetNumSector();
    
    // Calculate the sector address
    SectorAddr++;
    SectorAddr = BBT_GetSectorAddr(SectorAddr);

    // Write BBT Data
    while(dwBBTNumSectors--)
    {
        if(!CSPNAND_WriteSector(dwCs, SectorAddr, pBBTBuff, &SI))
        {
            DEBUGMSG(TRUE, (TEXT("CSPNAND_WriteSector Fail\r\n")));

            if(!BBT_EraseOperation(dwCs, dwStartBlock))
            {
                DEBUGMSG(TRUE, (TEXT("BBT_EraseOperation Fail\r\n")));
            }
            
            goto cleanUp;
        }
        
        pBBTBuff += g_FlashInfoExt.fi.wDataBytesPerSector;

        // Calculate the sector address
        SectorAddr++;
        SectorAddr = BBT_GetSectorAddr(SectorAddr);
    }

    // Write the Signature
    if(!WriteBBTSignature(dwCs, SectorAddr))
    {
        DEBUGMSG(TRUE, (TEXT("WriteBBTSignature Fail\r\n")));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: BBT_EraseOperation
//
//  This function implements the erase operation.
//
//  Parameters:
//      dwCs
//          [in] chip selection.
//
//      dwStartBlock
//          [in] block number.
// 
//  Returns:    
//      TRUE if successful.
//      FALSE if not successful.
//
//-----------------------------------------------------------------------------
static BOOL BBT_EraseOperation(DWORD dwCs, DWORD dwStartBlock)
{
    static BYTE bNumber = 0;
    BOOL rc = FALSE;

    DEBUGMSG(TRUE, (TEXT("BBT_EraseOperation Begin : bNumber = %d\r\n"), bNumber));
 
    if(!CSPNAND_EraseBlock(dwCs, dwStartBlock))
    {
        DEBUGMSG(TRUE, (TEXT("CSPNAND_EraseBlock 0x%X Fail\r\n"), dwStartBlock));
        if(bNumber < BBT_MAX_NESTED)
        {
            bNumber++;
            BBT_SetBlockBadStatus(dwCs, dwStartBlock);
            bNumber--;
        }
    }
    else
    {
        rc = TRUE;
    }

    DEBUGMSG(TRUE, (TEXT("BBT_EraseOperation End : bNumber = %d\r\n"), bNumber));

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: ReadBBTSignature
//
//  This function reads the BBT signature.
//
//  Parameters:
//      dwCs
//          [in] chip selection.
//
//      startSectorAddr
//          [in] sector address.
// 
//  Returns:
//      TRUE if successful
//      FALSE if not successful
//
//-----------------------------------------------------------------------------
static BOOL ReadBBTSignature(DWORD dwCs, SECTOR_ADDR startSectorAddr)
{
    BYTE *pSectorBuff = NULL;
    SectorInfo SI;
    DWORD dwBBTSignature;
    BOOL rc = FALSE;

    pSectorBuff = g_BBTBuffer.pSectorBuf;
    
    if(!CSPNAND_ReadSector(dwCs, startSectorAddr, pSectorBuff, &SI))
    {
        DEBUGMSG(TRUE, (TEXT("CSPNAND_ReadSector Fail : startSectorAddr = 0x%X block = 0x%X\r\n"), startSectorAddr, startSectorAddr / g_FlashInfoExt.fi.wSectorsPerBlock));

        if(!ReadBlockBadStatus(dwCs, startSectorAddr / g_FlashInfoExt.fi.wSectorsPerBlock))
        {
            if(!BBT_EraseOperation(dwCs, startSectorAddr / g_FlashInfoExt.fi.wSectorsPerBlock))
            {
                DEBUGMSG(TRUE, (TEXT("ReadBBTSignature : BBT_EraseOperation 0x%X Fail\r\n"), startSectorAddr / g_FlashInfoExt.fi.wSectorsPerBlock));
            }
        }

        goto cleanUp;
    }
            
    // Check BBT Signature
    dwBBTSignature = *(DWORD *)pSectorBuff;
    if(dwBBTSignature != BBT_SIGNATURE)
    {
        DEBUGMSG(TRUE, (TEXT("Not BBT_SIGN 1 : 0x%X\r\n"), dwBBTSignature));
        goto cleanUp;
    }
    
    dwBBTSignature = *(DWORD *)(pSectorBuff + g_FlashInfoExt.fi.wDataBytesPerSector - sizeof(DWORD));               
    if(dwBBTSignature != BBT_SIGNATURE)
    {
        DEBUGMSG(TRUE, (TEXT("Not BBT_SIGN 2 : 0x%X\r\n"), dwBBTSignature));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: WriteBBTSignature
//
//  This function writes the BBT signature.
//
//  Parameters:
//      dwCs
//          [in] chip selection.
//
//      startSectorAddr
//          [in] sector address.
// 
//  Returns:
//      TRUE if successful.
//      FALSE if not successful.
//
//-----------------------------------------------------------------------------
static BOOL WriteBBTSignature(DWORD dwCs, SECTOR_ADDR startSectorAddr)
{
    SectorInfo SI;
    BYTE *pSectorBuff = NULL;
    BOOL rc = FALSE;

    DEBUGMSG(TRUE, (TEXT("WriteBBTSignature\r\n")));
    
    pSectorBuff = g_BBTBuffer.pSectorBuf;
    *(DWORD *)pSectorBuff = BBT_SIGNATURE;
    *(DWORD*)(pSectorBuff + g_FlashInfoExt.fi.wDataBytesPerSector - sizeof(DWORD)) = BBT_SIGNATURE;
    memset(&SI, 0xFF, sizeof(SI));
    if(!CSPNAND_WriteSector(dwCs, startSectorAddr, pSectorBuff, &SI))
    {
        DEBUGMSG(TRUE, (TEXT("CSPNAND_WriteSector Fail\r\n")));

        if(!BBT_EraseOperation(dwCs, startSectorAddr / g_FlashInfoExt.fi.wSectorsPerBlock))
        {
            DEBUGMSG(TRUE, (TEXT("BBT_EraseOperation Fail\r\n")));
        }

        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: OpenBBT
//
//  This function opens the BBT.
//
//  Parameters:
//      None.
// 
//  Returns:      
//      TRUE if successful
//      FALSE if not successful
//
//-----------------------------------------------------------------------------
static BOOL OpenBBT()
{
    BOOL rc = FALSE;
    
    memset(&g_BBTBuffer, 0, sizeof(g_BBTBuffer));
    // Get BBT Buffer Address
    BSPNAND_GetBufferPointer(&g_BBTBuffer);

    // Check BBT Buffer
    if((g_BBTBuffer.pSectorBuf == NULL)    ||
       (g_BBTBuffer.pBBTFirstBuf == NULL)  || 
       (g_BBTBuffer.pBBTSecondBuf == NULL))
    {
        goto cleanUp;
    }

    // Check BBT Size
    if((g_BBTBuffer.dwBBTFirstSize == 0)   ||
       (g_BBTBuffer.dwBBTSecondSize == 0))
    {
        goto cleanUp;
    }
    
    // Two BBT should be the same size
    if(g_BBTBuffer.dwBBTFirstSize != g_BBTBuffer.dwBBTSecondSize)
    {
        goto cleanUp;
    }

    // Check BBT Range
    if((g_BBTBuffer.dwBBTStartBlock + 1 > g_FlashInfoExt.fi.dwNumBlocks) ||
       (g_BBTBuffer.dwBBTEndBlock > g_FlashInfoExt.fi.dwNumBlocks) ||
       (g_BBTBuffer.dwBBTStartBlock >= g_BBTBuffer.dwBBTEndBlock) ||
       (g_BBTBuffer.dwCs >= g_FlashInfoExt.NumberOfChip))
    {
        goto cleanUp;
    }

    // Initialize the BBT
    memset(g_BBTBuffer.pBBTFirstBuf, 0x00, g_BBTBuffer.dwBBTFirstSize);
    memset(g_BBTBuffer.pBBTSecondBuf, 0x00, g_BBTBuffer.dwBBTSecondSize);
    memset(g_BBTBuffer.pSectorBuf, 0xFF, g_FlashInfoExt.fi.wDataBytesPerSector);

    rc = TRUE;
    
cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: WriteBBT
//
//  This function writes the BBT.
//
//  Parameters:
//      dwBBTBlock
//          [in] block that contains the BBT.
//
//      dwBBTNum
//          [in] number of BBT to be written.
// 
//  Returns:    
//      TRUE if successful.
//      FALSE if not successful.
//
//-----------------------------------------------------------------------------
static BOOL WriteBBT(DWORD dwBBTBlock, DWORD dwBBTNum)
{
    DWORD dwStartBlock;
    DWORD dwEndBlock;
    DWORD dwCs;
    LPBYTE pBBTBuff;
    BOOL rc = FALSE;

    DEBUGMSG(TRUE, (TEXT("WriteBBT : dwBBTBlock = 0x%X, dwBBTNum = %d\r\n"), dwBBTBlock, dwBBTNum));

    if(dwBBTNum != 1 && dwBBTNum != 2)
    {
        goto cleanUp;
    }

    dwCs = g_BBTBuffer.dwCs;

    // Initialize the start block
    dwStartBlock = g_BBTBuffer.dwBBTStartBlock;
    dwEndBlock = g_BBTBuffer.dwBBTEndBlock;
    pBBTBuff = g_BBTBuffer.pBBTFirstBuf;
    while(dwBBTNum--)
    {
        // Check the reserved blocks for BBT
        for( ; dwStartBlock < dwEndBlock; dwStartBlock++)
        {
            DEBUGMSG(TRUE, (TEXT("WriteBBT For-loop : dwStartBlock = 0x%X, dwBBTNum = %d\r\n"), dwStartBlock, dwBBTNum));
            
            // Skip the existing BBT Block
            if(dwStartBlock == dwBBTBlock)
            {
                continue;
            }

            // Check the block status
            if(BBT_GetBlockBadStatus(dwCs, dwStartBlock))
            {
                continue;
            }

            // Update the BBT
            if(BBT_WriteOperation(dwCs, dwStartBlock, pBBTBuff))
            {
                dwStartBlock++;
                break;
            }
        }

        // Update fail
        if(dwStartBlock == dwEndBlock)
        {
            DEBUGMSG(TRUE, (TEXT("WriteBBT : Update fail\r\n")));
            goto cleanUp;
        }
    }

    DEBUGMSG(TRUE, (TEXT("Call BBT_SetBlockReservedStatus\r\n")));
    // Set the block reserved status
    dwStartBlock = g_BBTBuffer.dwBBTStartBlock;
    for( ; dwStartBlock < dwEndBlock; dwStartBlock++)
    {
        DEBUGMSG(TRUE, (TEXT("block = 0x%X\r\n"), dwStartBlock));
        if(BBT_GetBlockBadStatus(dwCs, dwStartBlock))
        {
            continue;
        }

        if(BBT_GetBlockReservedStatus(dwCs, dwStartBlock))
        {
            continue;
        }

        BBT_SetBlockReservedStatus(dwCs, dwStartBlock);
    }
    
    rc = TRUE;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: ReadBBT
//
//  This function reads the BBT.
//
//  Parameters:
//      None. 
//
//  Returns:
//      TRUE if successful.
//      FALSE if not successful.
//
//-----------------------------------------------------------------------------
static BOOL ReadBBT()
{
    DWORD dwCs;
    DWORD dwStartBlock;
    DWORD dwEndBlock;
    DWORD dwBBTFirstBlock = 0;
    DWORD dwBBTSecondBlock = 0;
    DWORD dwBBTBlock = 0;
    DWORD dwBBTReadNum = 0;
    LPBYTE pBBTBuff;

    // Return Value
    BOOL rc = FALSE;

    // NAND Chip Selection
    dwCs = g_BBTBuffer.dwCs;

    dwStartBlock = g_BBTBuffer.dwBBTStartBlock;
    dwEndBlock = g_BBTBuffer.dwBBTEndBlock;

    pBBTBuff = g_BBTBuffer.pBBTFirstBuf;
    for( ; dwStartBlock < dwEndBlock; dwStartBlock++)
    {
        if(BBT_ReadOperation(dwCs, dwStartBlock, pBBTBuff))
        {
            dwBBTReadNum++;

            if(dwBBTReadNum == 1)
            {
                // Store the first BBT block
                dwBBTFirstBlock = dwStartBlock;

                // Point to the second BBT buffer
                pBBTBuff = g_BBTBuffer.pBBTSecondBuf;
            }
            else if(dwBBTReadNum == 2)
            {
                // Store the second BBT block
                dwBBTSecondBlock = dwStartBlock;

                // Jump out of the for-loop
                break;
            }
        }
    }


    if(dwBBTReadNum == 2)
    {
        // Read two BBT
        BOOL bBBTFlush = FALSE;

        // Compare two BBT
        for(DWORD i = 0; i < g_BBTBuffer.dwBBTFirstSize; i += sizeof(DWORD))
        {
            if((*(DWORD *)(g_BBTBuffer.pBBTFirstBuf + i)) > (*(DWORD *)(g_BBTBuffer.pBBTSecondBuf + i)))
            {
                DEBUGMSG(TRUE, (TEXT("BBT Difference : %d : 0x%X - 0x%X\r\n"), i, (*(DWORD *)(g_BBTBuffer.pBBTFirstBuf + i)), (*(DWORD *)(g_BBTBuffer.pBBTSecondBuf + i))));
                dwBBTBlock = dwBBTFirstBlock;
                bBBTFlush = TRUE;

                break;
            }
            else if((*(DWORD *)(g_BBTBuffer.pBBTFirstBuf + i)) < (*(DWORD *)(g_BBTBuffer.pBBTSecondBuf + i)))
            {
                DEBUGMSG(TRUE, (TEXT("BBT Difference : %d : 0x%X - 0x%X\r\n"), i, (*(DWORD *)(g_BBTBuffer.pBBTFirstBuf + i)), (*(DWORD *)(g_BBTBuffer.pBBTSecondBuf + i))));
                memcpy(g_BBTBuffer.pBBTFirstBuf, g_BBTBuffer.pBBTSecondBuf, g_BBTBuffer.dwBBTFirstSize);
                dwBBTBlock = dwBBTSecondBlock;
                bBBTFlush = TRUE;

                break;
            }
        }

        if(bBBTFlush)
        {
            DEBUGMSG(TRUE, (TEXT("Both BBT need to be flushed\r\n")));
            if(!WriteBBT(dwBBTBlock, 1))
            {
                goto cleanUp;
            }
        }
    }
    else if(dwBBTReadNum == 1)
    {
        dwBBTBlock = dwBBTFirstBlock;
        if(!WriteBBT(dwBBTBlock, 1))
        {
            goto cleanUp;
        }
    }
    else
    {    
        DEBUGMSG(TRUE, (TEXT("No BBT Found\r\n")));
        goto cleanUp;
    }
    
    rc = TRUE;
    
cleanUp:

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: CreateBBT
//
//  This function creates the BBT.
//
//  Parameters:
//      None. 
//
//  Returns:
//      TRUE if successful.
//      FALSE if not successful.
//
//-----------------------------------------------------------------------------
static BOOL CreateBBT()
{
    DWORD i, j;
    BOOL rc = FALSE;
    
    if(g_bInterLeaveMode)
    {
        DEBUGMSG(TRUE, (TEXT("CreateBBT : Interleave Mode\r\n")));
        for(i = 0; i < g_FlashInfoExt.fi.dwNumBlocks; i++)
        {
            for(j = 0; j < g_FlashInfoExt.NumberOfChip; j++)
            {
                if(ReadBlockBadStatus(j, i))
                {
                    SetBlockBadStatus(i);
                    break;
                }
            }
        }
    }
    else
    {
        DEBUGMSG(TRUE, (TEXT("CreateBBT : Non-Interleave Mode\r\n")));
        for(i = 0; i < g_FlashInfoExt.NumberOfChip; i++)
        {
            for(j = 0; j < g_FlashInfoExt.fi.dwNumBlocks; j++)
            {
                if(ReadBlockBadStatus(i, j))
                {
                    SetBlockBadStatus(j + i * g_FlashInfoExt.fi.dwNumBlocks);
                }
            }
        }
    }

    rc = WriteBBT(0, 2);
    
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: BBT_Init
//
//  This function intializes the BBT.
//
//  Parameters:
//      None. 
//
//  Returns:
//      TRUE if successful.
//      FALSE if not successful.
//
//-----------------------------------------------------------------------------
BOOL BBT_Init()
{
    BOOL rc = FALSE;
    
    // Open BBT Buffer
    if(!OpenBBT())
    {
        goto cleanUp;
    }
    
    // Read existing BBT
    if(!ReadBBT())
    {
        // Create BBT if BBT does not exist
        if(!CreateBBT())
        {
            goto cleanUp;
        }
    }
    
    rc = TRUE;

cleanUp:
    if(!rc)
    {
        g_BBTBuffer.pBBTFirstBuf = NULL;
    }
    
    return rc;
}


