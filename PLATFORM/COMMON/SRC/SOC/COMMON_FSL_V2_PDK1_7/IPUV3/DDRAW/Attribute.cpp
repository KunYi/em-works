//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  Attribute.cpp
//
//  Add one function in DrvEscape
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#define WINCEMACRO
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#pragma warning(pop)

#include "ipu_common.h"

//------------------------------------------------------------------------------
//
// Function: VMSetAttributeEx
//
// Set certain memory space to write through mode.
//
// Parameters:
//      pVMSetAttributeExData 
//              [in] a pointer to VMSetAttributeExData
//
//      pOrgAttr 
//              [out] a pointer for returning original attribute
//
// Returns:
//      TRUE if setting successfully. Otherwise FALSE.
//
//------------------------------------------------------------------------------
BOOL VMSetAttributeEx(pVMSetAttributeExData pData, VM_ATTRIBUTE * pOrgAttr)
{
    DWORD oldFlag;
    DWORD newFlag;
    newFlag = (DWORD)pData->vmAttr << 2;
    // change to write-through caching, normal space(strongly ordered by default)
    if(VirtualSetAttributesEx((void*)GetCallerVMProcessId(), pData->lpvAddress, pData->cbSize, newFlag, 0xC, &oldFlag))
    {
        DEBUGMSG(1, (TEXT("SetAttributeEx: ADD=0x%x, SIZE=0x%x, OLDATTR=0x%x\r\n"), pData->lpvAddress,pData->cbSize, oldFlag )); 
        *pOrgAttr = (VM_ATTRIBUTE)((oldFlag&0xC)>>2);
        return TRUE;
    }
    else
    {
        ERRORMSG(1, (TEXT("SetAttributeEx() failed! \r\n")));
        DEBUGMSG(1, (TEXT("SetAttributeEx: ADD=0x%x, SIZE=0x%x, OLDATTR=0x%x\r\n"), pData->lpvAddress,pData->cbSize, oldFlag )); 
        return FALSE;
    }

}


