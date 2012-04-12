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

#ifndef __IOCTL_CFG_H
#define __IOCTL_CFG_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//  RESTRICTION
//
//  This file is a configuration file for the IOCTL component.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Define:  IOCTL_PLATFORM_TYPE/OEM
//
//  Defines the platform type and OEM string.
//
#define IOCTL_PLATFORM_TYPE                 (L"i.MX28")
#define IOCTL_PLATFORM_OEM                  (L"Freescale i.MX28 EVK")

#ifndef BSP_SI_VER_TO1_0
#define OCRAM_BOOT_MODE_OFFSET      0x19BF0
#else
#define OCRAM_BOOT_MODE_OFFSET      0x1A7F0
#endif

#define BOOT_MODE_MASK				0xF
#define BOOT_MODE_SDMMC			0x9
#define BOOT_MODE_SPI					0x2

//------------------------------------------------------------------------------
//  Define:  IOCTL_PROCESSOR_VENDOR/NAME/CORE
//
//  Defines the processor information
//

#define IOCTL_PROCESSOR_VENDOR              (L"Freescale")
#define IOCTL_PROCESSOR_NAME                (L"MX28")
#define IOCTL_PROCESSOR_CORE                (L"ARM926EJ-S")

//------------------------------------------------------------------------------
//
//  Define:  IOCTL_PROCESSOR_INSTRUCTION_SET
//
//  Defines the processor instruction set information
//
#define IOCTL_PROCESSOR_INSTRUCTION_SET     (0)
#define IOCTL_PROCESSOR_CLOCK_SPEED         (0)

BOOL OALIoCtlPowerOffEnable(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlQueryBoardId (UINT32 code, VOID *lpInBuf, 
    UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned); 


//------------------------------------------------------------------------------
#define IOCTL_HAL_POWER_OFF_ENABLE    CTL_CODE(FILE_DEVICE_HAL, 3072, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_QUERY_BOARD_ID      CTL_CODE(FILE_DEVICE_HAL, 3073, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_QUERY_BOOT_MODE     CTL_CODE(FILE_DEVICE_HAL, 3074, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_SET_BOOT_SOURCE     CTL_CODE(FILE_DEVICE_HAL, 3075, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_QUERY_UPDATE_SIG    CTL_CODE(FILE_DEVICE_HAL, 3076, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_SET_UPDATE_SIG      CTL_CODE(FILE_DEVICE_HAL, 3077, METHOD_BUFFERED, FILE_ANY_ACCESS)


typedef enum
{
    NandBoot = 1,
    SDMMCBoot,
    SPIBoot
}BootMedia;

typedef enum
{
    First = 1,
    Second
}BootSource;

typedef struct __BootMode__
{
    BootMedia   media;
    BootSource  source;
}BootMode, *PBootMode;

//------------------------------------------------------------------------------
// CS&ZHL MAR-8-2012: OEM IOCTL CODE for EM9280
//------------------------------------------------------------------------------
#ifdef	EM9280

#define IOCTL_HAL_NANDFMD_ACCESS			CTL_CODE(FILE_DEVICE_HAL, 4000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_TIMESTAMP_WRITE			CTL_CODE(FILE_DEVICE_HAL, 4010, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HAL_SHARE_INTERRUPT			CTL_CODE(FILE_DEVICE_HAL, 4012, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif	//EM9280

#define IOCTL_HAL_CPU_INFO_READ				CTL_CODE(FILE_DEVICE_HAL, 4018, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct __IMX_CPU_INFO__
{
    BYTE	FSLString[16];
    DWORD	dwStringLen;			// < 16
    DWORD	dwChipID;
}IMX_CPU_INFO, *PIMX_CPU_INFO;


#if __cplusplus
}
#endif

#endif
