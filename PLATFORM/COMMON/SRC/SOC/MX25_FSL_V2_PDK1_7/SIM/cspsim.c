//-----------------------------------------------------------------------------
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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  cspsim.c
//
//  Provides SoC-specific configuration routines for
//  the SIM controller.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


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

//-----------------------------------------------------------------------------
//
// Function:  CspSIMGetIRQ
//
// This function returns the IRQ number for the
// SIM controller based on the device index requested.
//
// Parameters:
//      index
//          [in] Index of the SIM device requested.
//
// Returns:
//      IRQ number for SIM, or 0 if an
//      invalid index was passed.
//
//-----------------------------------------------------------------------------
DWORD CspSIMGetIRQs(UINT32 index,DWORD* irqArray,DWORD* pdwSize)
{
    DWORD dwResult = 0;

    if (pdwSize == NULL)
    {
        return 0;
    }

    switch (index)
    {
        case 1:
            {
                if (*pdwSize >= sizeof(DWORD))
                {
                    irqArray[0] = IRQ_SIM1;
                    *pdwSize = sizeof(DWORD);
                    dwResult = 1;
                }
                else
                { //not enough space in buffer. return the minimum required space
                    *pdwSize = sizeof(DWORD);
                }
                break;
            }
        case 2:            
            {
                if (*pdwSize >= sizeof(DWORD))
                {
                    irqArray[0] = IRQ_SIM2;
                    *pdwSize = sizeof(DWORD);
                    dwResult = 1;
                }
                else
                { //not enough space in buffer. return the minimum required space
                    *pdwSize = sizeof(DWORD);
                }
                break;
            }
        default:
            break;
    }
    return dwResult;
}


//-----------------------------------------------------------------------------
//
// Function:  CspSIMGetBaseAddress
//
// This function returns the base address for the
// SIM controller based on the device index.
//
// Parameters:
//      index
//          [in] Index of the SIM device requested.
//
// Returns:
//      DWORD the base adress of the controller or 0x0 if the index is not valid
//
//-----------------------------------------------------------------------------

DWORD CspSIMGetBaseAddress(DWORD dwIndex)
{
    switch (dwIndex)
    {
    case 1:
        return CSP_BASE_REG_PA_SIM1;
    case 2:
        return CSP_BASE_REG_PA_SIM2;
    default:
        return 0x0;
    }
}
