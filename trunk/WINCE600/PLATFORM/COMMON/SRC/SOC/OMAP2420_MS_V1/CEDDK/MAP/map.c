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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  map.c
//
//  This file contains DDK library implementation for IO space mapping
//  functions. For OMAP2420 we choose to use one common mapping for all
//  on chip devices to reduce number of mapping (usually each device
//  will create its own mapping which cause unnecessary TLB missises).
//  
#include <windows.h>
#include <types.h>
#include <ceddk.h>
#include <omap2420.h>

//------------------------------------------------------------------------------
//
//  Global:  g_pTIPB
//
//  This global variable contains virtual address for TIPB peripherals
//  address region (physicall address 0xFFF00000, size 1 MB).
//
static VOID *g_pTIPB = NULL;

//------------------------------------------------------------------------------

PVOID MmMapIoSpace(PHYSICAL_ADDRESS phAddress, ULONG size, BOOLEAN cacheEnable)
{
    VOID *pAddress = NULL;
    UINT64 phSource;
    UINT32 sourceSize, offset;
    BOOL rc;

    // Check if we can use common mapping for device registers (test is
    // simplified as long as we know that we support only 32 bit addressing).
    if (
        !cacheEnable && phAddress.HighPart == 0 && 
        phAddress.LowPart >= OMAP2420_REGS_PA
    ) {

        // Create mapping when it doesn't exist yet
        if (g_pTIPB == NULL) {
            
            g_pTIPB = VirtualAlloc(
                0, OMAP2420_REGS_SIZE, MEM_RESERVE, PAGE_NOACCESS
            );
            if (g_pTIPB == NULL) {
                DEBUGMSG(TRUE, (
                    L"ERROR: MmMapIoSpace failed reserve registers memory\r\n"
                ));
                goto cleanUp;
            }
            rc = VirtualCopy(
                g_pTIPB, (PVOID)(OMAP2420_REGS_PA >> 8), 
                OMAP2420_REGS_SIZE, PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE
            );
            if (!rc) {
                DEBUGMSG(TRUE, (
                    L"ERROR: MmMapIoSpace failed allocate registers memory\r\n"
                ));
                VirtualFree(g_pTIPB, 0, MEM_RELEASE);
                g_pTIPB = NULL;
                goto cleanUp;
            }
        }

        // Calculate offset
        offset = phAddress.LowPart - OMAP2420_REGS_PA;
        (UINT32)pAddress = (UINT32)g_pTIPB + offset;
        
    } else {

        phSource = phAddress.QuadPart & ~(PAGE_SIZE - 1);
        sourceSize = size + (phAddress.LowPart & (PAGE_SIZE - 1));

        pAddress = VirtualAlloc(0, sourceSize, MEM_RESERVE, PAGE_NOACCESS);
        if (pAddress == NULL) {
            DEBUGMSG(TRUE, (
                L"ERROR: MmMapIoSpace failed reserve memory\r\n"
            ));
            goto cleanUp;
        }            
        rc = VirtualCopy(
            pAddress, (PVOID)(phSource >> 8), sourceSize,
            PAGE_PHYSICAL | PAGE_READWRITE | (cacheEnable ? 0 : PAGE_NOCACHE)
        );
        if (!rc) {
            DEBUGMSG(TRUE, (
                L"ERROR: MmMapIoSpace failed allocate memory\r\n"
            ));
            VirtualFree(pAddress, 0, MEM_RELEASE);
            pAddress = NULL;
            goto cleanUp;
        }
        (UINT32)pAddress += phAddress.LowPart & (PAGE_SIZE - 1);
    }

cleanUp:
    return pAddress;
}

//------------------------------------------------------------------------------
//
//  Function:  MmUnmapIoSpace
//
//  Unmap a specified range of physical addresses previously mapped by
//  MmMapIoSpace
//
VOID MmUnmapIoSpace(VOID *pAddress, ULONG size)
{
    // We want unmap only in case that memory wasn't mapped from common
    // mapping.
    if (
        g_pTIPB == NULL || 
        (UINT32)pAddress < (UINT32)g_pTIPB ||
        (UINT32)pAddress >= (UINT32)g_pTIPB + OMAP2420_REGS_SIZE
    ) {
        VirtualFree(
            (VOID*)((UINT32)pAddress & ~(PAGE_SIZE - 1)), 0, MEM_RELEASE
        );
    }
}

//------------------------------------------------------------------------------
//
//  Function:  TransBusAddrToVirtual
//
BOOL TransBusAddrToVirtual(
    INTERFACE_TYPE ifcType, ULONG busNumber, PHYSICAL_ADDRESS busAddress,
    ULONG length, ULONG *pAddressSpace, VOID **ppMappedAddress
) {
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phAddress; 

    if (!HalTranslateBusAddress(
        ifcType, busNumber, busAddress, pAddressSpace, &phAddress
    )) {
        goto cleanUp;
    }

    switch (*pAddressSpace) {
    case 0:
        // Memory-mapped I/O, get virtual address for translated address
        *ppMappedAddress = MmMapIoSpace(phAddress, length, FALSE);
        if (*ppMappedAddress != NULL) rc = TRUE;
        break;
    case 1:        
        // I/O port
        *ppMappedAddress = (VOID*)phAddress.LowPart;
        rc = TRUE;
        break;
    }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  TransBusAddrToVirtual
//
BOOL TransBusAddrToStatic(
    INTERFACE_TYPE ifcType, ULONG busNumber, PHYSICAL_ADDRESS busAddress,
    ULONG length, ULONG *pAddressSpace, VOID **ppAddress
) {
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phAddress;
    UINT64 address;
    UINT32 size;

    if (!HalTranslateBusAddress(
        ifcType, busNumber, busAddress, pAddressSpace, &phAddress
    )) goto cleanUp;

    switch (*pAddressSpace) {
    case 0:        
        // Memory-mapped I/O, get statically-mapped virtual address
        // for translated physical address
        address = phAddress.QuadPart & ~(PAGE_SIZE - 1);
        size = length + (phAddress.LowPart & (PAGE_SIZE - 1));
        *ppAddress = CreateStaticMapping((UINT32)(address >> 8), size);
        if (*ppAddress != NULL) {
            rc = TRUE;
            // Adjust with offset from page
            (UINT32)*ppAddress += phAddress.LowPart & (PAGE_SIZE - 1);
        }
        break;
    case 1:        
        // I/O port
        *ppAddress = (VOID*)phAddress.LowPart;
        rc = TRUE;
        break;
    }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
