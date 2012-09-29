//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  i_hal_nand.c
//
//-----------------------------------------------------------------------------
// common
#include "dma_memory.c"
#include "cspnand.c"

VOID* BSPNAND_RemapRegister(DWORD PhyAddr, DWORD size)
{
    UNREFERENCED_PARAMETER(size);
    return OALPAtoUA(PhyAddr);
}


VOID BSPNAND_UnmapRegister(PVOID VirtAddr, DWORD size)
{
    UNREFERENCED_PARAMETER(size);
    VirtAddr = NULL;
}
