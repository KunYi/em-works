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

//
// CS&ZHL MAR-22-2012: copied from EM9170 fmdwrapperpdd.h which was copied from AN4139
//
typedef PVOID (*PFN_OemINIT)(LPCTSTR lpActiveReg, PPCI_REG_INFO pRegIn, PPCI_REG_INFO pRegOut);
typedef BOOL (*PFN_OemDEINIT)(PVOID pContext);
typedef BOOL (*PFN_OemGETINFO)(PVOID pContext, PFlashInfo pFlashInfo);
typedef BOOL (*PFN_OemGETINFOEX)(PVOID pContext, PFlashInfoEx pFlashInfo, PDWORD pdwNumRegions);
typedef DWORD (*PFN_OemGETBLOCKSTATUS)(PVOID pContext, BLOCK_ID blockID, BOOL bWithStartBlock);
typedef BOOL (*PFN_OemSETBLOCKSTATUS)(PVOID pContext, BLOCK_ID blockID, DWORD dwStatus, BOOL bWithStartBlock);
typedef BOOL (*PFN_OemREADSECTOR)(PVOID pContext, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors, BOOL bWithStartBlock);
typedef BOOL (*PFN_OemWRITESECTOR)(PVOID pContext, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors, BOOL bWithStartBlock);
typedef BOOL (*PFN_OemERASEBLOCK)(PVOID pContext, BLOCK_ID blockID, BOOL bWithStartBlock);
typedef VOID (*PFN_OemPOWERUP)(PVOID pContext);
typedef VOID (*PFN_OemPOWERDOWN)(PVOID pContext);
typedef VOID (*PFN_OemGETPHYSSECTORADDR)(PVOID pContext, DWORD dwSector, PSECTOR_ADDR pStartSectorAddr, BOOL bWithStartBlock);
typedef BOOL (*PFN_OemIOCONTROL)(PVOID pContext, DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned);

typedef struct _OemFMDInterface
{
    DWORD cbSize;
    PFN_OemINIT pInit;
    PFN_OemDEINIT pDeInit;
    PFN_OemGETINFO pGetInfo;
    PFN_OemGETBLOCKSTATUS pGetBlockStatus;
    PFN_OemSETBLOCKSTATUS pSetBlockStatus;
    PFN_OemREADSECTOR pReadSector;
    PFN_OemWRITESECTOR pWriteSector;
    PFN_OemERASEBLOCK pEraseBlock;
    PFN_OemPOWERUP pPowerUp;
    PFN_OemPOWERDOWN pPowerDown;
    PFN_OemGETPHYSSECTORADDR pGetPhysSectorAddr;
    PFN_OemGETINFOEX pGetInfoEx;
    PFN_OemIOCONTROL pOEMIoControl;
    
} OemFMDInterface, *POemFMDInterface;

PVOID		OEMFMD_Init(LPCTSTR lpActiveReg, PPCI_REG_INFO pRegIn, PPCI_REG_INFO pRegOut);
BOOL		OEMFMD_Deinit(PVOID pContext);
BOOL		OEMFMD_GetInfo(PVOID pContext, PFlashInfo pFlashInfo);
BOOL		OEMFMD_GetInfoEx(PVOID pContext, PFlashInfoEx pFlashInfo, PDWORD pdwNumRegions);
DWORD		OEMFMD_GetBlockStatus(PVOID pContext, BLOCK_ID blockID, BOOL bWithStartBlock);
BOOL		OEMFMD_SetBlockStatus(PVOID pContext, BLOCK_ID blockID, DWORD dwStatus, BOOL bWithStartBlock);
BOOL		OEMFMD_ReadSector (PVOID pContext, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors, BOOL bWithStartBlock);
BOOL		OEMFMD_WriteSector(PVOID pContext, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors, BOOL bWithStartBlock);
BOOL		OEMFMD_EraseBlock(PVOID pContext, BLOCK_ID blockID, BOOL bWithStartBlock);
VOID		OEMFMD_PowerUp(PVOID pContext);
VOID		OEMFMD_PowerDown(PVOID pContext);
BOOL		OEMFMD_OemIoControl(PVOID pContext, DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned);
// end of CS&ZHL MAR-22-2012: copied from EM9170 fmdwrapperpdd.h which was copied from AN4139

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
    //FMDInterface      m_FmdInterface;
	// CS&ZHL MAR-22-2012: replace the FMD_XXX with OEMFMD_XXX which contains input pointer -> FmdWrapperPdd
    OemFMDInterface	  m_FmdInterface;
	// end of CS&ZHL MAR-22-2012: replace the FMD_XXX with OEMFMD_XXX which contains input pointer -> FmdWrapperPdd
    FLASH_REGION_INFO m_RegionInfo;
    BOOL              m_LoadDone;

//
// CS&ZHL MAY-14-2011: supporting multi-partition within a nandflash chip
//
public:
	DWORD		m_dwRegionNumber;
	DWORD		m_dwStartBlock;
	DWORD		m_dwBlockCounts;
	BOOL		m_bInitialized;

	FlashInfo	m_FlashInfo;					// Info about NandFlash chip Block/Sector
	DWORD		m_dwFlashInfoSize;
};

#endif // _FMDWRAPPERPDD_H_
