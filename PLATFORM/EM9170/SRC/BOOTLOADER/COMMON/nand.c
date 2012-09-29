//-----------------------------------------------------------------------------
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  nand.c
//
//  Contains BOOT NAND flash support functions.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include "loader.h"
#include "nandboot.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables
extern BOOL g_bNandExist;


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: BSP_GetNANDBufferPointer
//
//    Get the Sector Buffer pointer
//
// Parameters:
//        pSectorBuffer[out] - buffer parameters
//
// Returns:
//        the buffer parameters
//
//------------------------------------------------------------------------------
void BSP_GetNANDBufferPointer(PSECTOR_BUFFER pSectorBuffer)
{   
    pSectorBuffer->pSectorBuf = (BYTE *)OALPAtoVA((IMAGE_BOOT_RAMDEV_RAM_PA_START + IMAGE_BOOT_RAMDEV_RAM_SIZE - NANDFC_BOOT_SIZE), TRUE);
    pSectorBuffer->dwSectorBufSize = NANDFC_BOOT_SIZE;
}

BOOL BSP_OEMIoControl(DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, 
    PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned)
{
    BOOL rc = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwIoControlCode);
    UNREFERENCED_PARAMETER(pInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);
    UNREFERENCED_PARAMETER(pOutBuf);
    UNREFERENCED_PARAMETER(nOutBufSize);
    UNREFERENCED_PARAMETER(pBytesReturned);
    return(rc);
}

