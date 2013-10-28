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
#include <bootMemory.h>

//------------------------------------------------------------------------------
//  External variables

extern
const
BOOT_MAP_TABLE
BootMapTable[];

//------------------------------------------------------------------------------

void*
BootPAtoVA(
    uint32_t pa,
    bool_t cached
    )
{
    const BOOT_MAP_TABLE* pTable = BootMapTable;
    VOID *va = NULL;

    // Search the table for address range
    while (pTable->size != 0)
        {
        if ((pa >= pTable->PA) && 
            (pa <= (pTable->PA + (pTable->size << 20) - 1)))
            break;                      // match found 
        pTable++;
        }

    // If address table entry is valid compute the VA
    if (pTable->size != 0)
        {
        va = (VOID *)(pTable->CA + (pa - pTable->PA));
        // If VA is uncached, set the uncached bit
        if (!cached) va = (VOID *)((uint32_t)va | BOOT_MEMORY_CACHE_BIT);
    }

    return va;
}

//------------------------------------------------------------------------------

uint32_t
BootVAtoPA(
    void *pVA
    )
{
    const BOOT_MAP_TABLE* pTable = BootMapTable;
    uint32_t va = (uint32_t)pVA;
    uint32_t pa = 0;


    // Virtual address must be in CACHED or UNCACHED regions.
    if (va < 0x80000000 || va >= 0xC0000000) goto cleanUp;

    // Address must be cached, as entries in OEMAddressTable are cached address.
    va &= ~BOOT_MEMORY_CACHE_BIT;

    // Search the table for address range
    while (pTable->size != 0)
        {
        if ((va >= pTable->CA) && 
            (va <= pTable->CA + (pTable->size << 20) - 1))
            break;
        pTable++;
    }

    // If address table entry is valid compute the PA
    if (pTable->size != 0) pa = pTable->PA + va - pTable->CA;

cleanUp:
    return pa;
}

//------------------------------------------------------------------------------

uint32_t
BootImageVAtoPA(
    void *pVA
    )
{
    return BootVAtoPA(pVA);
}

//------------------------------------------------------------------------------

