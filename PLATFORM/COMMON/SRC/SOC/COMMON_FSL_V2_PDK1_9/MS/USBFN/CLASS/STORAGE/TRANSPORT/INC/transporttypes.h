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

        TRANSPORTTYPES.H

Abstract:

        USB Mass Storage Function Common Transport Types.
        
--*/

#ifndef __MSC_TRANSPORTTYPES_H__
#define __MSC_TRANSPORTTYPES_H__

// Direction flags
#define DATA_OUT            0x00000000
#define DATA_IN             0x00000001

// Command block
typedef struct _TRANSPORT_COMMAND {
    DWORD Flags;            // [in]  - DATA_IN or DATA_OUT
    DWORD Timeout;          // [in]  - timeout for this command block; not used
    DWORD Length;           // [in]  - length of the command block buffer
    PVOID CommandBlock;     // [in]  - address of the command block buffer
} TRANSPORT_COMMAND, *PTRANSPORT_COMMAND;

// Data block
typedef struct _TRANSPORT_DATA_BUFFER {
    DWORD RequestLength;    // [in]  - requested length
    DWORD TransferLength;   // [out] - number of bytes actually transferred 
    PVOID DataBlock;        // [in]  - address of the data buffer
} TRANSPORT_DATA, *PTRANSPORT_DATA;

#endif // __MSC_TRANSPORTTYPES_H__

