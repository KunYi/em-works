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
//  File:  pcistub.c
//
//  This file implements DDK PCI bus functions.
//
#include <windows.h>
#include <ceddk.h>
#include <oal.h>
#include <nkintr.h>
#include <hal.h>
#include <oal_pci.h>

//------------------------------------------------------------------------------
//
//  Function:  OALPCIGetId
//
UINT32 OALPCIGetId(
                   UINT32 busId, 
                   OAL_PCI_LOCATION pciLoc
                   )
{
    UNREFERENCED_PARAMETER(busId);
    UNREFERENCED_PARAMETER(pciLoc);
    // stub function - x86 doesn't use this
    return 0;
}

//------------------------------------------------------------------------------
//
//  Function:  OALPCIPowerOff
//
void OALPCIPowerOff(
                    UINT32 busId
                    )
{
    UNREFERENCED_PARAMETER(busId);
    // stub function - x86 doesn't use this
}

//------------------------------------------------------------------------------
//
//  Function:  OALPCIPowerOn
//
void OALPCIPowerOn(
                   UINT32 busId
                   )
{
    UNREFERENCED_PARAMETER(busId);
    // stub function - x86 doesn't use this
}

//------------------------------------------------------------------------------
//
//  Function:  OALPCITransBusAddress
//
BOOL OALPCITransBusAddress(
                           UINT32 busNumber, 
                           UINT64 busAddress, 
                           __out UINT32 * pAddressSpace, 
                           __out UINT64 * pSystemAddress
                           ) 
{
    BOOL rc = FALSE;

    OALMSG(OAL_PCI&&OAL_FUNC, (
        L"+OALPCITranslateBusAddress(%d, 0x%08x%08x, %d)\r\n",
        busNumber, (UINT32)(busAddress >> 32), (UINT32)busAddress,
        *pAddressSpace
    ));

    // No special translation necessary for x86, just return success.
    rc = TRUE;

    OALMSG(OAL_PCI&&OAL_FUNC, (
        L"-OALPCITranslateBusAddress(addressSpace = %d, "
        L"systemAddress = 0x%08x%08x, rc = %d)\r\n", *pAddressSpace,
        (UINT32)(*pSystemAddress >> 32), (UINT32)*pSystemAddress, rc
    ));

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALPCICfgRead
//
//  This function reads PCI configuration space at location specified by pciLoc.
//  This routine is hardware dependend and in most cases it will be implemented
//  in SoC library or it will platform specific.
//
UINT32 OALPCICfgRead(
                     UINT32 busId, 
                     OAL_PCI_LOCATION pciLoc, 
                     UINT32 offset, 
                     UINT32 size,
                     __out_bcount(size) void *pData
                    )
{
    ULONG device, function, bus;
    UNREFERENCED_PARAMETER(busId);

    // Note: Bus Number is always 0 on x86
    bus = (pciLoc.logicalLoc >> 16) & 0xFF;   // Subordinate Bus Number
    device = (pciLoc.logicalLoc >> 8) & 0xFF; // Device Number
    function = (pciLoc.logicalLoc) & 0xFF;    // Function Number.

    return PCIReadBusData (bus, device, function, pData, offset, size);
}

//------------------------------------------------------------------------------
//
//  Function:  OALPCICfgWrite
//
//  This function write PCI configuration space at location specified by pciLoc.
//  This routine is hardware dependend and in most cases it will be implemented
//  in SoC library or it will be platform specific.
//
UINT32 OALPCICfgWrite(
                      UINT32 busId, 
                      OAL_PCI_LOCATION pciLoc, 
                      UINT32 offset, 
                      UINT32 size,
                      __in_bcount(size) void *pData
                     )
{
    ULONG device, function, bus;
    UNREFERENCED_PARAMETER(busId);

    // Note: Bus Number is always 0 on x86
    bus = (pciLoc.logicalLoc >> 16) & 0xFF;   // Subordinate Bus Number
    device = (pciLoc.logicalLoc >> 8) & 0xFF; // Device Number
    function = (pciLoc.logicalLoc) & 0xFF;    // Function Number.

    return PCIWriteBusData (bus, device, function, pData, offset, size);
}

