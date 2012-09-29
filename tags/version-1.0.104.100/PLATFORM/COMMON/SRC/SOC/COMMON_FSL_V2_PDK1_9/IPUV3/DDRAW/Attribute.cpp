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
    DWORD mask;

    switch(pData->vmAttr)
    {
    case VMATTR_NONCACHED:
        // Small Page Descriptor:
        //
        //  TEX[2:0], C, B = 0, 0, 0 => strongly ordered
        //
        //                       FLAGS                    MASK
        //  TEX = 0 = (0 << 6) = 0x000      (0x7 << 6) = 0x1C0
        //    C = 0 = (0 << 3) = 0x000      (0x1 << 3) = 0x008
        //    B = 0 = (0 << 2) = 0x000      (0x1 << 2) = 0x004
        //  --------------------------------------------------
        //                       0x000                   0x1CC
        newFlag = 0x0;
        mask = 0x1CC;
        break;
        
    case VMATTR_ONLYBUFFERED:
        // Small Page Descriptor:
        //
        //  TEX[2:0], C, B = 1, 0, 0 => normal, outer/inner non-cacheable
        //
        //                       FLAGS                    MASK
        //  TEX = 1 = (1 << 6) = 0x040      (0x7 << 6) = 0x1C0
        //    C = 0 = (0 << 3) = 0x000      (0x1 << 3) = 0x008
        //    B = 0 = (0 << 2) = 0x000      (0x1 << 2) = 0x004
        //  --------------------------------------------------
        //                       0x040                   0x1CC
        newFlag = 0x40;
        mask = 0x1CC;
        break;

    case VMATTR_CACHED_WRITETHROUGH:
        // Small Page Descriptor:
        //
        //  TEX[2:0], C, B = 0, 1, 0 => normal, outer/inner write-through
        //
        //                       FLAGS                    MASK
        //  TEX = 0 = (0 << 6) = 0x000      (0x7 << 6) = 0x1C0
        //    C = 0 = (1 << 3) = 0x008      (0x1 << 3) = 0x008
        //    B = 0 = (0 << 2) = 0x000      (0x1 << 2) = 0x004
        //  --------------------------------------------------
        //                       0x008                   0x1CC
        //
        newFlag = 0x8;
        mask = 0x1CC;
        break;

    case VMATTR_CACHED_WRITEBACK:
        // Small Page Descriptor:
        //
        //  TEX[2:0], C, B = 0, 1, 1 => normal, outer/inner write-back
        //
        //                       FLAGS                    MASK
        //  TEX = 0 = (0 << 6) = 0x000      (0x7 << 6) = 0x1C0
        //    C = 0 = (1 << 3) = 0x008      (0x1 << 3) = 0x008
        //    B = 0 = (1 << 2) = 0x004      (0x1 << 2) = 0x004
        //  --------------------------------------------------
        //                       0x00C                   0x1CC
        //
        newFlag = 0xC;
        mask = 0x1CC;
        break;

    default:
        newFlag = 0x0;
        mask = 0x0;
        break;
    }
            
    // change to write-through caching, normal space(strongly ordered by default)
    if(VirtualSetAttributesEx((void*)GetCallerVMProcessId(), pData->lpvAddress, pData->cbSize, newFlag, mask, &oldFlag))
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


