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
//  File: args.h
//
//  This file defines the shared argument structure used by the bootloader and
//  the OS image.
//
//------------------------------------------------------------------------------
#ifndef _ARGS_H_
#define _ARGS_H_

#define BSP_ARGS_QUERY_KITL_FLAGS           BSP_ARGS_QUERY      // Query for KITL flags

#define BSP_ARGS_DEFAULT_MAC_BYTE0          0x12
#define BSP_ARGS_DEFAULT_MAC_BYTE1          0x13
#define BSP_ARGS_DEFAULT_MAC_BYTE2          0x17
#define BSP_ARGS_DEFAULT_MAC_BYTE3          0x28
#define BSP_ARGS_DEFAULT_MAC_BYTE4          0x31
#define BSP_ARGS_DEFAULT_MAC_BYTE5          0x00


//------------------------------------------------------------------------------

#define BSP_ARGS_VERSION    1

typedef struct {
    OAL_ARGS_HEADER header;
    UINT8           deviceId[16];   // Device identification
    OAL_KITL_ARGS   kitl;
    UINT32          clockFreq[DDK_CLOCK_SIGNAL_ENUM_END];
    BOOL            updateMode;
} BSP_ARGS;


BOOL OALBspArgsInit(BSP_ARGS *pBSPArgs);

#endif
