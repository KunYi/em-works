//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//  
//------------------------------------------------------------------------------
#ifndef _PCI_HEADER_H_
#define _PCI_HEADER_H_

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
ULONG PCIReadBusData(
    IN ULONG BusNumber,
    IN ULONG DeviceNumber,
    IN ULONG FunctionNumber,
    OUT PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
ULONG PCIWriteBusData(
    IN ULONG BusNumber,
    IN ULONG DeviceNumber,
    IN ULONG FunctionNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
ULONG
PCIGetBusDataByOffset(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
ULONG
PCISetBusDataByOffset(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL 
PCIInitConfigMechanism (
    UCHAR ucConfigMechanism
    );

#endif
