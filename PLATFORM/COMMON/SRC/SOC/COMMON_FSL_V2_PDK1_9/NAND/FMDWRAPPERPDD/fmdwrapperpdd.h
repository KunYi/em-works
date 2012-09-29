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

#ifndef _FMDWRAPPERPDD_H_
#define _FMDWRAPPERPDD_H_

#include "FmdWrapperMain.h"
#include "fmd.h"

class FmdWrapperPdd: public FlashPddInterface
{
public:

    FmdWrapperPdd();
    LRESULT Init(DWORD Context);
    LRESULT Deinit();

public:
    // Query the number of flash hardware regions.
    virtual LRESULT GetRegionCount (
        OUT DWORD* pRegionCount);

    // Query information about each flash hardware region.
    virtual LRESULT GetRegionInfoTable (
        IN  DWORD RegionCount,
        OUT FLASH_REGION_INFO RegionInfoList[]);

    // Query the status of a particular block.
    virtual LRESULT GetBlockStatus (
        IN BLOCK_RUN BlockRun, 
        IN BOOL IsInitialFlash,
        OUT ULONG BlockStatus[]);

    // Set the status of a particular block to a combination of bad, read-only, reserved, etc.
    virtual LRESULT SetBlockStatus (
        IN BLOCK_RUN BlockRun, 
        IN ULONG BlockStatus);


    // Read data and/or spare area from multiple, possibly discontiguous sectors.
    virtual LRESULT ReadPhysicalSectors (
        IN ULONG TransferCount, 
        IN OUT FLASH_PDD_TRANSFER ReadList[],
        OUT ULONG* pReadStatus);

    // Write data and/or spare area to multiple, possibly discontiguous sectors.
    virtual LRESULT WritePhysicalSectors (
        IN ULONG TransferCount, 
        IN FLASH_PDD_TRANSFER WriteList[],
        OUT ULONG* pWriteStatus);

    // Erase multiple, possibly discontiguous blocks.
    virtual LRESULT EraseBlocks (
        IN ULONG RunCount, 
        IN BLOCK_RUN BlockRunList[]);

    // Copy data from multiple source sectors to destination sectors, possibly changing the spare area for the 
    // destination sectors.
    virtual LRESULT CopyPhysicalSectors (
        IN ULONG TransferCount, 
        IN FLASH_PDD_COPY CopyList[]);

    // For every sector specified by the SectorRun, retrieve a corresponding physical 
    // sector address. Must be implemented if XIP is to be supported.
    virtual LRESULT GetPhysicalSectorAddress (
        IN DWORD RegionIndex,
        IN SECTOR_RUN SectorRun, 
        OUT VOID* pPhysicalAddressList[]);

    // Hardware lock a range of physical blocks. Must be implemented if block locking is to be 
    // supported.  BlockRun specifies the run of blocks to lock.  All other blocks are assumed
    // to be unlocked.
    virtual LRESULT LockBlocks (
        IN DWORD BlockRunCount,
        IN BLOCK_RUN BlockRunList[]);

    // Get life cycle information for each of the flash regions.
    virtual LRESULT GetLifeCycleInfo (
        IN DWORD RegionCount,
        OUT FLASH_LIFE_CYCLE_INFO* pInfo);

    // Get identity information for the flash part.
    virtual LRESULT GetIdentityInfo (
        OUT FLASH_IDENTITY_INFO* pInfo);

    // IoControl function for handling additional, part-specific IOCTLs.
    virtual LRESULT IoControl (
        DWORD Ioctl, 
        BYTE* pInBuffer, 
        DWORD InBufferSize, 
        BYTE* pOutBuffer, 
        DWORD OutBufferSize, 
        DWORD* pBytesReturned);

protected:
    VOID    GetFmdInterface();
    LRESULT BoolToWin32Result (BOOL Result);
    
    
private:
    PVOID             m_FmdHandle;
    PVOID             m_FmdHookHandle;
    FMDInterface      m_FmdInterface;
    FLASH_REGION_INFO m_RegionInfo;
    BOOL              m_LoadDone;
};

#endif // _FMDWRAPPERPDD_H_
