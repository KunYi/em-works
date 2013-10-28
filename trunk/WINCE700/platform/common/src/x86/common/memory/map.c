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
#include <windows.h>
#include <oal.h>

//------------------------------------------------------------------------------
//
//  Function:  OALPAtoVA
//
//  Converts a physical address (PA) to a virtual address (VA).
//
VOID* OALPAtoVA(UINT32 pa, BOOL cached)
{
    UINT32 offset;
    UINT8 *va = NULL;

    OALMSG(OAL_MEMORY&&OAL_FUNC, (L"+OALPAtoVA(0x%x, %d)\r\n", pa, cached));

    offset = pa & (VM_PAGE_SIZE - 1);
    pa &= ~(VM_PAGE_SIZE - 1);
    va = NKPhysToVirt( pa >> 8, cached );
    if (va) {
        va += offset;
    }

    // Indicate the virtual address
    OALMSG(OAL_MEMORY&&OAL_FUNC, (L"-OALPAtoVA(va = 0x%08x)\r\n", va));
    return va;
}

//------------------------------------------------------------------------------
//
//  Function:  OALVAtoPA
//
//  Converts a virtual address (VA) to a physical address (PA).
//
UINT32 OALVAtoPA(VOID *pVA)
{
    UINT32 va = (UINT32)pVA;
    UINT32 offset;
    UINT32 pa;

    OALMSG(OAL_MEMORY&&OAL_FUNC, (L"+OALVAtoPA(0x%08x)\r\n", pVA));

    offset = va & (VM_PAGE_SIZE - 1);
    va &= ~(VM_PAGE_SIZE - 1);
    
    pa = (UINT32)NKVirtToPhys( (LPCVOID)va );

    if (INVALID_PHYSICAL_ADDRESS != pa) {
        pa = (pa << 8) + offset;
    }

    // Indicate physical address 
    OALMSG(OAL_MEMORY&&OAL_FUNC, (L"-OALVAtoPA(pa = 0x%x)\r\n", pa));
    return pa;
}

//------------------------------------------------------------------------------

