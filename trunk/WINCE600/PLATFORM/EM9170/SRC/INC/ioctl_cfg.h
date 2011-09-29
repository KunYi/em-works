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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
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
#define IOCTL_PLATFORM_TYPE                 (L"i.MX25")
#define IOCTL_PLATFORM_OEM                  (L"Freescale i.MX25 3DS")

//------------------------------------------------------------------------------
//  Define:  IOCTL_PROCESSOR_VENDOR/NAME/CORE
//
//  Defines the processor information
//

#define IOCTL_PROCESSOR_VENDOR              (L"Freescale")
#define IOCTL_PROCESSOR_NAME                (L"MX25")
#define IOCTL_PROCESSOR_CORE                (L"ARM926")

//------------------------------------------------------------------------------
//
//  Define:  IOCTL_PROCESSOR_INSTRUCTION_SET
//
//  Defines the processor instruction set information
//
#define IOCTL_PROCESSOR_INSTRUCTION_SET     (0)
#define IOCTL_PROCESSOR_CLOCK_SPEED         (0)

//------------------------------------------------------------------------------
// Board-specific IOCTLS
//------------------------------------------------------------------------------
#define IOCTL_HAL_CPLD_READ16  \
    CTL_CODE(FILE_DEVICE_HAL, 3072, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_CPLD_WRITE16  \
    CTL_CODE(FILE_DEVICE_HAL, 3073, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_SHARED_CSPI_TRANSFER  \
    CTL_CODE(FILE_DEVICE_HAL, 3074, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------
//
// CS&ZHL MAY-13-2011: supporting multiple partitions of NandFlash
//
#ifdef NAND_PDD
//OEM IOCTL CODE
#define IOCTL_HAL_NANDFMD_ACCESS			CTL_CODE(FILE_DEVICE_HAL, 4000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif	//NAND_PDD

//------------------------------------------------------------------------------
//
// CS&ZHL JUN-23-2011: board info
//
#ifdef	EM9170
#define IOCTL_HAL_TIMESTAMP_WRITE			CTL_CODE(FILE_DEVICE_HAL, 4010, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif	//EM9170

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
