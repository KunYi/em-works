#ifndef _OEMFMDWRAPPERPDD_H_
#define _OEMFMDWRAPPERPDD_H_

#include "fmd.h"

//
// CS&ZHL MAY-14-2011: copied from AN4139, 
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

#endif	//_OEMFMDWRAPPERPDD_H_
