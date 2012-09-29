//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bspuut.cpp
//
//  Implements bsp related uce commands.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:  6287 6262 4201 4512 4100 4115 4214)
#include <windows.h>
#include "stdio.h"
#include "stdlib.h"
#pragma warning(pop)


BOOL BSPUceCmdDeal(char * pbCmd)
{
    UNREFERENCED_PARAMETER(pbCmd);
    return TRUE;
}

BOOL BSPUceTransDeal(PBYTE pbData,  DWORD DataLength)
{
    UNREFERENCED_PARAMETER(pbData);
    UNREFERENCED_PARAMETER(DataLength);
    
    return TRUE;    
}