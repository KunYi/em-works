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

BOOL UceOpenFile(TCHAR* ptchFileName, HANDLE &hFile);
BOOL UceWriteFile(HANDLE hFile, PBYTE pBuff, DWORD dwBuffSize);
BOOL UceCloseFile(HANDLE hFile);
BOOL WriteImage(TCHAR *pDiskName, LPBYTE pImage, DWORD dwLength);
DWORD GetMinImgBufSize(TCHAR *pDiskName);
BOOL PrePartialWriteImage(TCHAR *pDiskName, DWORD dwLength);
BOOL PartialWriteImage(LPBYTE pImage, DWORD dwLength);
BOOL EndPartialWriteImage();
BOOL OTPProgram(char *pBuff);
#endif //_UCE_MEDIA_H__
