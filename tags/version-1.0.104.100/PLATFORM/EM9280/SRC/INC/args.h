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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File: args.h
//
//  This file defines the shared argument structure used by the bootloader and
//  the OS image.
//
//------------------------------------------------------------------------------
#ifndef _ARGS_H_
#define _ARGS_H_

#define BSP_ARGS_QUERY_KITL_FLAGS           BSP_ARGS_QUERY        // Query for KITL flags
#define BSP_ARGS_QUERY_DBGSERIAL            BSP_ARGS_QUERY + 1      // Query debug serial port.

#define BSP_ARGS_DEFAULT_MAC_BYTE0          0x00
#define BSP_ARGS_DEFAULT_MAC_BYTE1          0x02
#define BSP_ARGS_DEFAULT_MAC_BYTE2          0xB3
#define BSP_ARGS_DEFAULT_MAC_BYTE3          0x92
#define BSP_ARGS_DEFAULT_MAC_BYTE4          0xA8
#define BSP_ARGS_DEFAULT_MAC_BYTE5          0xC4


//------------------------------------------------------------------------------

#define BSP_ARGS_VERSION    1

typedef struct {
    OAL_ARGS_HEADER header;
    UINT8 deviceId[16];             // Device identification
    OAL_KITL_ARGS kitl;
    UINT32 dbgSerPhysAddr;          // Debug serial physical address
    UINT8 uuid[16];
    BOOL updateMode;
    BOOL bHiveCleanFlag;            // TRUE = Clean hive at boot
    BOOL bCleanBootFlag;            // TRUE = Clear RAM, hive, user store at boot
    BOOL bFormatPartFlag;           // TRUE = Format partion when mounted at boot
	BOOL bDebugFlag;				// CS&ZHL MAY-29-2012: save DBGSLn state
} BSP_ARGS;

VOID OALArgsInit(BSP_ARGS *pBSPArgs);

#endif
