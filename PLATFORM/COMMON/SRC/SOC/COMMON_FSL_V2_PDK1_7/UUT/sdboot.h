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

BOOL SDWriteImage(HANDLE hDsk, LPBYTE pImage, DWORD dwLength);
BOOL SDPrePartialWriteImage(HANDLE hDsk, DWORD dwLength);
BOOL SDPartialWriteImage(LPBYTE pImage, DWORD dwLength);
BOOL SDEndPartialWriteImage();

#endif //_SD_BOOT_H__