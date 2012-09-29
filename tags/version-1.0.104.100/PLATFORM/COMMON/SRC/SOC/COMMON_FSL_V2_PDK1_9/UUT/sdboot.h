//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  sdboot.h
//
//  Implements the SD/MMC boot functions.
//
//-----------------------------------------------------------------------------
#ifndef _SD_BOOT_H__
#define _SD_BOOT_H__

void SDGetHandle(HANDLE hDsk);
BOOL SDWriteImage(LPBYTE pImage, DWORD dwLength);
BOOL SDPrePartialWriteImage(DWORD dwLength);
BOOL SDPartialWriteImage(LPBYTE pImage, DWORD dwLength);
BOOL SDWriteMBRModeImage(LPBYTE pImage, DWORD startSectorAddr, DWORD dwLength);
BOOL SDWriteMBRModeEboot(LPBYTE pImage, DWORD dwLength);
BOOL SDWriteMBRModeNK(LPBYTE pImage, DWORD dwLength);
BOOL SDPrePartialWriteMBRModeImage(fwType type, DWORD dwLength);
BOOL SDPartialWriteRedundantImage(LPBYTE pImage, DWORD dwLength);
BOOL SDEndPartialWriteImage();
BOOL SDPreSet(DWORD startAddr, DWORD dwValidDataLength);
BOOL SDWriteRawData(PBYTE pbData, DWORD dwValidDataLength);
BOOL SDWriteMBR(HANDLE hDsk,INT iPartition,PARTITION_CONT partCont);
BOOL SDCreatePartitions(HANDLE hDsk,INT iPartition,PARTITION_CONT partCont);
#endif //_SD_BOOT_H__