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
#include "..\FmdWrapperPdd.h"
#include "common_nandfmd.h"

#pragma warning(disable: 4100)
#define MAX_TRANSFER_COUNT  32
SectorInfo pFmdSectorInfo[MAX_TRANSFER_COUNT];

FmdWrapperPdd::FmdWrapperPdd():
    m_FmdHandle(NULL),
    m_FmdHookHandle(NULL)    
{
    ZeroMemory (&m_FmdInterface, sizeof(FMDInterface));
    ZeroMemory (&m_RegionInfo, sizeof(FLASH_REGION_INFO));
}

LRESULT FmdWrapperPdd::Init(DWORD Context)
{
    LRESULT Result = ERROR_SUCCESS;
    
    m_LoadDone = FALSE;
    BSPNAND_SetClock(TRUE);
    
    m_FmdHandle = FMD_Init ((LPCTSTR)Context, NULL, NULL);
    if (m_FmdHandle == NULL)
    {
        Result = ERROR_GEN_FAILURE;
        goto exit;        
    }

    GetFmdInterface(); 

    Result = GetRegionInfoTable (1, &m_RegionInfo);

exit:
    if(Result != ERROR_SUCCESS) 
        BSPNAND_SetClock(FALSE);
    return Result;
}

LRESULT FmdWrapperPdd::Deinit()
{
    BOOL Result = ERROR_SUCCESS;

    BSPNAND_SetClock(TRUE);

    if (m_FmdInterface.pDeInit)
    {
        m_FmdInterface.pDeInit(m_FmdHandle);
    }

    if (m_FmdHookHandle)
    {
        FMDHOOK_UnhookInterface(m_FmdHookHandle, &m_FmdInterface);
    }
    
    BSPNAND_SetClock(FALSE);
    return BoolToWin32Result(Result);
}

LRESULT FmdWrapperPdd::GetRegionCount (
    OUT DWORD* pRegionCount)
{
    *pRegionCount = 1;
    return ERROR_SUCCESS;
}


LRESULT FmdWrapperPdd::GetRegionInfoTable (
    IN DWORD RegionCount,
    OUT FLASH_REGION_INFO RegionInfoList[])
{
    LRESULT Result = ERROR_SUCCESS;

    if (RegionCount != 1)
    {
        Result = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    FlashInfo FmdFlashInfo;
    Result = BoolToWin32Result (m_FmdInterface.pGetInfo(&FmdFlashInfo));
    if (Result != ERROR_SUCCESS)
    {
        goto exit;
    }

    // Assume that the FMD implements locking, since there is no good way to query this
    // from the FMD.
    //
    if (FmdFlashInfo.flashType == NAND)
    {        
        RegionInfoList[0].FlashFlags = (FLASH_FLAG_NAND | FLASH_FLAG_SUPPORTS_LOCKING);
    }
    else
    {
        RegionInfoList[0].FlashFlags = (FLASH_FLAG_NOR | FLASH_FLAG_SUPPORTS_XIP | FLASH_FLAG_SUPPORTS_LOCKING);
    }

    RegionInfoList[0].StartBlock = 0;
    RegionInfoList[0].BlockCount = FmdFlashInfo.dwNumBlocks;
    RegionInfoList[0].SectorsPerBlock = FmdFlashInfo.wSectorsPerBlock;
    RegionInfoList[0].DataBytesPerSector = FmdFlashInfo.wDataBytesPerSector;
    RegionInfoList[0].PageProgramLimit = 1;
    RegionInfoList[0].BadBlockHundredthPercent = 100;

    // Don't allow the flash MDD to write to the bad block byte or
    // the OEM reserved field in the SectorInfo
    //
    RegionInfoList[0].SpareBytesPerSector = sizeof(SectorInfo) - 2;

exit:
    return Result;
}
    

LRESULT FmdWrapperPdd::GetBlockStatus (
    IN BLOCK_RUN BlockRun, 
    IN BOOL IsInitialFlash,
    OUT ULONG BlockStatusList[])
{
    LRESULT Result = ERROR_SUCCESS;
    
    if (m_LoadDone) 
        BSPNAND_SetClock(TRUE);
    for (DWORD BlockIndex = 0; BlockIndex < BlockRun.BlockCount; BlockIndex++)
    {
        if ((BlockRun.StartBlock + BlockIndex) >= m_RegionInfo.BlockCount)
        {
            Result = ERROR_INVALID_PARAMETER;
            goto exit;
        }

        DWORD FmdBlockStatus = m_FmdInterface.pGetBlockStatus ((BLOCK_ID)BlockRun.StartBlock + BlockIndex);

        if (FmdBlockStatus == BLOCK_STATUS_UNKNOWN)
        {
            Result = ERROR_GEN_FAILURE;
            goto exit;
        }

        // Translate from FMD block status to flash PDD block status
        //
        BlockStatusList[BlockIndex] = 0;
        if (FmdBlockStatus & BLOCK_STATUS_BAD)
        {
            BlockStatusList[BlockIndex] |= FLASH_BLOCK_STATUS_BAD;
        }
        if (FmdBlockStatus & BLOCK_STATUS_RESERVED)
        {
            BlockStatusList[BlockIndex] |= FLASH_BLOCK_STATUS_RESERVED;
        }
    }

exit:
    if (m_LoadDone) 
        BSPNAND_SetClock(FALSE);
    return Result;
}


LRESULT FmdWrapperPdd::SetBlockStatus (
    IN BLOCK_RUN BlockRun, 
    IN ULONG BlockStatus)
{
    LRESULT Result = ERROR_SUCCESS;

    // Translate from FMD block status to flash PDD block status
    //
    DWORD FmdBlockStatus = 0;
    
    if (BlockStatus & FLASH_BLOCK_STATUS_BAD)
    {
        FmdBlockStatus |= BLOCK_STATUS_BAD;
    }
    if (BlockStatus & FLASH_BLOCK_STATUS_RESERVED)
    {
        FmdBlockStatus |= BLOCK_STATUS_RESERVED;
    }
    
    if (m_LoadDone) 
        BSPNAND_SetClock(TRUE);
    for (DWORD BlockIndex = 0; BlockIndex < BlockRun.BlockCount; BlockIndex++)
    {
        if ((BlockRun.StartBlock + BlockIndex) >= m_RegionInfo.BlockCount)
        {
            Result = ERROR_INVALID_PARAMETER;
            goto exit;
        }

        Result = BoolToWin32Result (m_FmdInterface.pSetBlockStatus ((BLOCK_ID)BlockRun.StartBlock + BlockIndex, FmdBlockStatus));
        if (Result != ERROR_SUCCESS)
        {
            goto exit;
        }
    }

exit:
    if (m_LoadDone) 
        BSPNAND_SetClock(FALSE);
    return Result;
}


LRESULT FmdWrapperPdd::ReadPhysicalSectors (
    IN ULONG TransferCount, 
    IN OUT FLASH_PDD_TRANSFER ReadList[],
    OUT ULONG* pReadStatus)
{
    LRESULT Result = ERROR_SUCCESS;
    
    if (m_LoadDone) 
        BSPNAND_SetClock(TRUE);
    for (DWORD TransferIndex = 0; TransferIndex < TransferCount; TransferIndex++)
    {
        DWORD StartSector = (DWORD)ReadList[TransferIndex].SectorRun.StartSector;
        DWORD SectorCount = ReadList[TransferIndex].SectorRun.SectorCount;
        LPBYTE pData = ReadList[TransferIndex].pData;
        LPBYTE pSpare = ReadList[TransferIndex].pSpare;

        PREFAST_SUPPRESS(28197,"This Warning can be skipped!")
        
        memset (pFmdSectorInfo, 0xff, sizeof(SectorInfo)*MAX_TRANSFER_COUNT);

        DWORD SectorIndex = 0;
        DWORD SectorCountForOnce = 1;
        do
        {   
            if ((StartSector + SectorIndex) >= (m_RegionInfo.BlockCount * m_RegionInfo.SectorsPerBlock))
            {
                ReportError((TEXT("FmdWrapperPdd::ReadPhysicalSectors(): The number of sectors to be written are too many!\r\n")));
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }        
            
            SectorCountForOnce = ((SectorCount - SectorIndex) >= MAX_TRANSFER_COUNT)? \
                MAX_TRANSFER_COUNT:(SectorCount - SectorIndex);

            Result = BoolToWin32Result (m_FmdInterface.pReadSector (StartSector+SectorIndex,
                                                                    pData,
                                                                    PREFAST_SUPPRESS(28197,"This Warning can be skipped!")
                                                                    pSpare ? pFmdSectorInfo : NULL,
                                                                    SectorCountForOnce));

            if (Result != ERROR_SUCCESS)
            {
                goto exit;
            }
            
            if (pData)
            {
                pData += m_RegionInfo.DataBytesPerSector*SectorCountForOnce;
            }

            if (pSpare)
            {
                for(DWORD i=0;i<SectorCountForOnce;i++){
                    PREFAST_SUPPRESS(6011,"This Warning can be skipped!")
                    memcpy (pSpare, &pFmdSectorInfo[i].dwReserved1, sizeof(DWORD));
                    memcpy (pSpare + sizeof(DWORD), &pFmdSectorInfo[i].wReserved2, sizeof(WORD));
                    pSpare += m_RegionInfo.SpareBytesPerSector;
                }
            }
            
            SectorIndex += SectorCountForOnce;
            
        }while(SectorIndex<SectorCount);
            
    }

exit:
    if (m_LoadDone) 
        BSPNAND_SetClock(FALSE);
    return Result;    
}


LRESULT FmdWrapperPdd::WritePhysicalSectors (
    IN ULONG TransferCount, 
    IN FLASH_PDD_TRANSFER WriteList[],
    OUT ULONG* pWriteStatus)
{
    LRESULT Result = ERROR_SUCCESS;  
    
    if (m_LoadDone) 
        BSPNAND_SetClock(TRUE);
    
    for (DWORD TransferIndex = 0; TransferIndex < TransferCount; TransferIndex++)
    {
        DWORD StartSector = (DWORD)WriteList[TransferIndex].SectorRun.StartSector;
        DWORD SectorCount = WriteList[TransferIndex].SectorRun.SectorCount;
        LPBYTE pData = WriteList[TransferIndex].pData;
        LPBYTE pSpare = WriteList[TransferIndex].pSpare;
        
        PREFAST_SUPPRESS(28197,"This Warning can be skipped!")
        
        memset (pFmdSectorInfo, 0xff, sizeof(SectorInfo)*MAX_TRANSFER_COUNT);

            
        DWORD SectorIndex = 0;
        DWORD SectorCountForOnce = 1;
        
        do
        {           
            if ((StartSector + SectorIndex) >= (m_RegionInfo.BlockCount * m_RegionInfo.SectorsPerBlock))
            {
                ReportError((TEXT("FmdWrapperPdd::WritePhysicalSectors(): The number of sectors to be written are too many!\r\n")));
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }        
            
            SectorCountForOnce = ((SectorCount - SectorIndex) >= MAX_TRANSFER_COUNT)? \
                MAX_TRANSFER_COUNT:(SectorCount - SectorIndex);
            
            if (pSpare)
            {
                for(DWORD i=0;i<SectorCountForOnce;i++){
                    PREFAST_SUPPRESS(6011,"This Warning can be skipped!")
                    memcpy (&pFmdSectorInfo[i].dwReserved1, pSpare, sizeof(DWORD));
                    memcpy (&pFmdSectorInfo[i].wReserved2, pSpare + sizeof(DWORD), sizeof(WORD));
                    pSpare += m_RegionInfo.SpareBytesPerSector;
                }
            }
            
            Result = BoolToWin32Result (m_FmdInterface.pWriteSector (StartSector+SectorIndex,
                                                                     pData,
                                                                     PREFAST_SUPPRESS(28197,"This Warning can be skipped!")
                                                                     pSpare ? pFmdSectorInfo : NULL,
                                                                     SectorCountForOnce));
            
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::WritePhysicalSectors(): WriteSector failed!\r\n")));
            
                goto exit;
            }
            
            if (pData)
            {
                pData += m_RegionInfo.DataBytesPerSector*SectorCountForOnce;
            }
            
            SectorIndex += SectorCountForOnce;          
                        
        }while(SectorIndex<SectorCount);
    }

exit:
    if (m_LoadDone) 
        BSPNAND_SetClock(FALSE);
    return Result;    
}

LRESULT FmdWrapperPdd::EraseBlocks (
    IN ULONG RunCount, 
    IN BLOCK_RUN BlockRunList[])
{
    LRESULT Result = ERROR_SUCCESS;
    
    
    if (m_LoadDone) 
        BSPNAND_SetClock(TRUE);
    for (DWORD RunIndex = 0; RunIndex < RunCount; RunIndex++)
    {
        BLOCK_ID StartBlock = (BLOCK_ID)BlockRunList[RunIndex].StartBlock;
        DWORD BlockCount = BlockRunList[RunIndex].BlockCount;

        for (DWORD BlockIndex = 0; BlockIndex < BlockCount; BlockIndex++)
        {
            if ((StartBlock + BlockIndex) >= m_RegionInfo.BlockCount)
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }

            DWORD BlockStatus = m_FmdInterface.pGetBlockStatus (StartBlock + BlockIndex);
            if (BlockStatus == BLOCK_STATUS_BAD)
            {
                continue;
            }
            
            Result = BoolToWin32Result (m_FmdInterface.pEraseBlock (StartBlock + BlockIndex));
            if (Result != ERROR_SUCCESS)
            {
                goto exit;
            }
        }
    }

exit:
    if (m_LoadDone) 
        BSPNAND_SetClock(FALSE);
    return Result;    
}

LRESULT FmdWrapperPdd::CopyPhysicalSectors (
    IN ULONG TransferCount, 
    IN FLASH_PDD_COPY CopyList[])
{
    return ERROR_NOT_SUPPORTED;
}

LRESULT FmdWrapperPdd::GetPhysicalSectorAddress (
    IN DWORD RegionIndex,
    IN SECTOR_RUN SectorRun, 
    OUT VOID* pPhysicalAddressList[])
{
    LRESULT Result = ERROR_SUCCESS;

    if (!m_FmdInterface.pGetPhysSectorAddr)
    {
        Result = ERROR_NOT_SUPPORTED;
        goto exit;
    }

    DWORD StartSector = (DWORD)SectorRun.StartSector;
        
    for (DWORD SectorIndex = 0; SectorIndex < SectorRun.SectorCount; SectorIndex++)
    {
        m_FmdInterface.pGetPhysSectorAddr (StartSector + SectorIndex, 
                                           (PSECTOR_ADDR)&pPhysicalAddressList[SectorIndex]);
    }

exit:
    return Result;
}

LRESULT FmdWrapperPdd::LockBlocks (
    IN DWORD BlockRunCount,
    IN BLOCK_RUN BlockRunList[])
{
    LRESULT Result = ERROR_SUCCESS;

    for (DWORD RunIndex = 0; RunIndex < BlockRunCount; RunIndex++)
    {
        BlockLockInfo LockInfo = {0};
        LockInfo.StartBlock = (BLOCK_ID)BlockRunList[RunIndex].StartBlock;
        LockInfo.NumBlocks = BlockRunList[RunIndex].BlockCount;
        
        DWORD EndBlock = LockInfo.StartBlock + LockInfo.NumBlocks - 1;
        if ((EndBlock >= m_RegionInfo.BlockCount) ||
            (EndBlock < LockInfo.StartBlock))
        {
            Result = ERROR_INVALID_PARAMETER;
            goto exit;
        }

        Result = IoControl (IOCTL_FMD_LOCK_BLOCKS,
                            (LPBYTE)&LockInfo, sizeof(BlockLockInfo),
                            NULL, 0,
                            NULL);

        if (Result != ERROR_SUCCESS)
        {
            goto exit;
        }
    }

exit:
    return Result;
}

LRESULT FmdWrapperPdd::GetLifeCycleInfo (
    IN DWORD RegionCount,
    OUT FLASH_LIFE_CYCLE_INFO* pInfo)
{
    return ERROR_NOT_SUPPORTED;
}

LRESULT FmdWrapperPdd::GetIdentityInfo (
    OUT FLASH_IDENTITY_INFO* pInfo)
{
    return ERROR_NOT_SUPPORTED;
}


LRESULT FmdWrapperPdd::IoControl(
    DWORD IoControlCode,
    PBYTE pInBuf,
    DWORD InBufSize,
    PBYTE pOutBuf,
    DWORD OutBufSize,
    PDWORD pBytesReturned)
{
    if (IoControlCode == IOCTL_DISK_GET_STORAGEID) 
    {
        DEBUGMSG(TRUE, (_T("FLASH Init Step Finish\r\n")));
        BSPNAND_SetClock(FALSE);
        m_LoadDone = TRUE;
    }
    // Pass any other IOCTL through to the FMD.
    //
    return BoolToWin32Result (m_FmdInterface.pOEMIoControl(
                 IoControlCode, 
                 pInBuf, InBufSize, 
                 pOutBuf, OutBufSize, 
                 pBytesReturned));    
}                                                                             

VOID FmdWrapperPdd::GetFmdInterface()
{
    m_FmdInterface.cbSize = sizeof(FMDInterface);

    // Query FMD intrface from the FMD    
    //
    m_FmdInterface.pOEMIoControl = FMD_OEMIoControl;
    
    if (!FMD_OEMIoControl (IOCTL_FMD_GET_INTERFACE, 
                           NULL, 0, 
                           (PBYTE)&m_FmdInterface, sizeof(FMDInterface), 
                           NULL))
    {
        // FMD does not support IOCTL_FMD_GET_INTERFACE, so build the FMDInterface 
        // structure using the legacy FMD functions
        //
        m_FmdInterface.pInit = FMD_Init;
        m_FmdInterface.pDeInit = FMD_Deinit;
        m_FmdInterface.pGetInfo = FMD_GetInfo;        
        m_FmdInterface.pGetBlockStatus = FMD_GetBlockStatus;     
        m_FmdInterface.pSetBlockStatus = FMD_SetBlockStatus;
        m_FmdInterface.pReadSector = FMD_ReadSector;
        m_FmdInterface.pWriteSector = FMD_WriteSector;
        m_FmdInterface.pEraseBlock = FMD_EraseBlock;
        m_FmdInterface.pPowerUp = FMD_PowerUp;
        m_FmdInterface.pPowerDown = FMD_PowerDown;
    }

    // Query hook library in case any FMD functions need to be shimmed
    //
    m_FmdHookHandle = FMDHOOK_HookInterface(&m_FmdInterface);
}


LRESULT FmdWrapperPdd::BoolToWin32Result (BOOL Result)
{
    if (Result)
    {
        return ERROR_SUCCESS;
    }
    else
    {
        DWORD LastError = GetLastError();
        if (LastError != ERROR_SUCCESS)
        {
            return LastError;
        }
        else
        {
            return ERROR_GEN_FAILURE;
        }
    }
    
}


