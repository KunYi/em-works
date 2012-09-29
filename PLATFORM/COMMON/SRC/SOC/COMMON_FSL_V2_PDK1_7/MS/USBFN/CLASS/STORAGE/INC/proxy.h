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
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        PROXY.H

Abstract:

        USB Mass Storage Function Device Emulation Proxy.
        
--*/

#ifndef __USBMSFDEP_H__
#define __USBMSFDEP_H__

#include "transporttypes.h"

// configuration values
#define PRX_DEVICE_NAME_VAL                 (_T("DeviceName"))
#define PRX_DEVICE_NAME_LEN                 256
#define PRX_PART_VAL                        (_T("Partitions"))
#define PRX_RMB_VAL                         (_T("Removable"))
#define PRX_STREAM_NAME_LEN                 6

// return codes
#define EXECUTE_PASS                        0x00000000
#define EXECUTE_FAIL                        0x00000001
#define EXECUTE_ERROR                       0x00000002

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#define DATA_UNKNOWN                        3
#endif

// Initialize a mass storage device.
DWORD
STORE_Init(
    LPCTSTR             lpcszContext
);


// Close a mass storage device.
DWORD
STORE_Close(
);


// Determine whether a mass storage command is supported.
//
//    ptcCommand - the target command
//
//    pfDataStageRequired - the command has a data stage 
//    (if the command is supported)
//
//    pdwDirection - the direction of the data stage 
//    (if the command is supported)
//
//    pdwDataSize - the size of the data stage (if the command
//    is supported)
BOOL
STORE_IsCommandSupported(
    PTRANSPORT_COMMAND  ptcCommand,
    PBOOL               pfDataStageRequired,
    PDWORD              pdwDirection,
    PDWORD              pdwDataSize
);


// Execute a mass storage command.
// 
//    ptcCommand - the target command
//
//    ptdData - data associated with the command (may be NULL)
DWORD
STORE_ExecuteCommand(
    PTRANSPORT_COMMAND  ptcCommand,
    PTRANSPORT_DATA     ptdData
);

#endif // __USBMSFDEP_H__

