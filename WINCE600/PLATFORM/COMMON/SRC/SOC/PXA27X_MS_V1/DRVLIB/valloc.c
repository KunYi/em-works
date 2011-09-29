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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: valloc.c

Date Created: 9/25/2003 

Abstract: Virtual allocation/mapping helper routines.

Functions: 

Notes: 

--*/
#include <windows.h>

#define PAGE_SIZE 0x1000
#define ALIGNMENT_MASK (PAGE_SIZE-1)

PVOID VirtualAllocCopy(unsigned size,char *str,PVOID pVirtualAddress)
{
    PVOID ptr;
    unsigned offset;

    offset = (unsigned)pVirtualAddress & ALIGNMENT_MASK;

    size +=offset ? PAGE_SIZE : 0;
    ptr = VirtualAlloc(0,size,MEM_RESERVE,PAGE_NOACCESS);
    if (ptr == NULL)
    {
        ERRORMSG(1,(TEXT("VirtualAlloc failed! %s : size=0x%x, (0x%x)\r\n"),str,size,GetLastError()));
        return(0);
    }

    if (!VirtualCopy((PVOID)ptr,(PVOID)((unsigned)pVirtualAddress - offset),size,PAGE_READWRITE|PAGE_NOCACHE))
    {
        ERRORMSG(1,(TEXT("VirtualCopy failed! %s : addr=0x%x, offset=0x%x(0x%x)\r\n"),str,(unsigned)pVirtualAddress,offset,GetLastError()));
        return(0);
    }
    return((PVOID)((PBYTE)ptr+offset));
}


PVOID VirtualAllocCopyPhysical(unsigned size, char *str, PVOID pPhysicalAddress)
{
    PVOID ptr;
    unsigned offset;

    offset = (unsigned)pPhysicalAddress & ALIGNMENT_MASK;

    size +=offset ? PAGE_SIZE : 0;
    ptr = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);

    if (ptr == NULL)
    {
        ERRORMSG(1,(TEXT("VirtualAllocCopyPhysical failed! %s : size=0x%x, (0x%x)\r\n"),str,size,GetLastError()));
        return(0);
    }

    if (!VirtualCopy((PVOID)ptr, (PVOID)(((unsigned)pPhysicalAddress - offset) >> 8), size, PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
    {
        ERRORMSG(1,(TEXT("VirtualCopyCopyPhysical failed! %s : addr=0x%x, offset=0x%x(0x%x)\r\n"),str,(unsigned)pPhysicalAddress,offset,GetLastError()));
        return(0);
    }
    return((PVOID)((PBYTE)ptr+offset));
}
