//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  uce_media.h
//
//  Implements the update command engine.
//
//-----------------------------------------------------------------------------
#ifndef _UCE_MEDIA_H__
#define _UCE_MEDIA_H__

#define MEDIA_NAND     0
#define MEDIA_SDMMC    1

#define STORAGE_PROFILE_FMT     _T("System\\StorageManager\\Profiles\\%s")
#define STORAGE_PROFILE_NAME    _T("Name")

typedef enum {
    fwType_XL_NB = 0,
    fwType_EB_NB,
    fwType_NK_NB,
    fwType_EB_SB,
    fwType_NK_SB
}fwType;

//TODO: should use single filed number to replace array.
typedef struct 
{
    INT iPartSize[4];//partition size
    TCHAR  PartName[4][15];//partition name:Firmware,File for MX;Eboot,NK,File for ST   
}PARTITION_CONT;

BOOL UceOpenFile(TCHAR* ptchFileName, HANDLE &hFile);
BOOL UceWriteFile(HANDLE hFile, PBYTE pBuff, DWORD dwBuffSize);
BOOL UceCloseFile(HANDLE hFile);
BOOL WriteImage(LPBYTE pImage, DWORD dwLength);
DWORD GetMinImgBufSize();
BOOL PrePartialWriteImage(TCHAR *pDiskName, DWORD dwLength);
BOOL PartialWriteImage(LPBYTE pImage, DWORD dwLength);
BOOL EndPartialWriteImage();
BOOL UcePreWriteRawData(TCHAR * pDiskName, DWORD startAddr, DWORD dwValidDataLength);
BOOL UceWriteRawData(PBYTE pbData, DWORD dwValidDataLength);
// CS&ZHL MAY11-2012: write security info to NandFlash & OTP
BOOL UceWriteSecurityInfo( PBYTE pbData, DWORD dwValidDataLength );
BOOL UceCreatePartitions(INT iPartition, PARTITION_CONT partCont);
BOOL GetDeviceHandle(TCHAR *pDiskName);

// CS&ZHL JUN1-2012: Get OTP Info
BOOL UceGetOTPInfo(DWORD* pdwCUST, DWORD dwLen);
//
// CS&ZHL AUG-13-2012: code for EM9280 uce to format NandFlash
//
BOOL UceNandLowLevelFormat( );

#endif //_UCE_MEDIA_H__
