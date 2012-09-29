//------------------------------------------------------------------------------
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
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ioctl_cfg.h
//
//  Configuration file for the IOCTL component.
//
//------------------------------------------------------------------------------

#ifndef __EM9280_OAL_SPI_H
#define __EM9280_OAL_SPI_H

#if __cplusplus
extern "C" {
#endif

//
// CS&ZHL MAY-13-2011: access NandFlash through KernelIoControl
//
#define SPI_ACCESS_CODE_READBYTE		(0)
#define SPI_ACCESS_CODE_WRITEBYTE		(1)
#define SPI_ACCESS_CODE_READBLOCK		(2)
#define SPI_ACCESS_CODE_WRITEBLOCK		(3)

typedef struct _SpiAccessInfo
{
	DWORD	dwAccessCode;
	DWORD	dwCSNum;			// = 0, 1, 2
	DWORD	dwStartAddr;
	DWORD	dwDataLength;
	PBYTE	pDataBuf;
    
} SpiAccessInfo, *PSpiAccessInfo;


#if __cplusplus
}
#endif

#endif	//__EM9280_OAL_SPI_H
