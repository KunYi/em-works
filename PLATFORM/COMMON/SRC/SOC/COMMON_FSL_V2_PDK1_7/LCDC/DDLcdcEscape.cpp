//-----------------------------------------------------------------------------n
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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2005-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  DDLcdcEscape.cpp
//
//  Add functions in DrvEscape
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#define WINCEMACRO
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#include "DDLcdcEscape.h"

#pragma warning(pop)

BOOL LcdcVMSetAttributeEx(pLcdcVMSetAttributeExData pData)
{
    DWORD oldFlag;

    // change to write-through caching, normal space(strongly ordered by default)
    if(VirtualSetAttributesEx((void*)GetCallerVMProcessId(), pData->lpvAddress, pData->cbSize, 0x8, 0xC, &oldFlag))
    {
        DEBUGMSG(1, (TEXT("SetAttributeEx: ADD=0x%x, SIZE=0x%x, OLDATTR=0x%x\r\n"), pData->lpvAddress,pData->cbSize, oldFlag )); 
        return TRUE;
    }else
    {
        ERRORMSG(1, (TEXT("SetAttributeEx() failed! \r\n")));
        DEBUGMSG(1, (TEXT("SetAttributeEx: ADD=0x%x, SIZE=0x%x, OLDATTR=0x%x\r\n"), pData->lpvAddress,pData->cbSize, oldFlag )); 
        return FALSE;
    }
}
