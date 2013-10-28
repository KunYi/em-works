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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddk_gpio.c
//
//  This file contains the SoC-specific DDK interface for the GPIO module.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
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
PCSP_GPIO_REGS g_pGPIO[DDK_GPIO_PORT3+1];


//-----------------------------------------------------------------------------
// Local Functions
BOOL GpioAlloc(void);
BOOL GpioDealloc(void);


//-----------------------------------------------------------------------------
//
// Function:  GpioAlloc
//
// This function allocates the data structures required for interaction
// with the GPIO hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL GpioAlloc(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    if (g_pGPIO[DDK_GPIO_PORT1] == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_GPIO1;

        // Map peripheral physical address to virtual address
        g_pGPIO[DDK_GPIO_PORT1] =
            (PCSP_GPIO_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_GPIO_REGS),
                                          FALSE);

        // Check if virtual mapping failed
        if (g_pGPIO[DDK_GPIO_PORT1] == NULL)
        {
            DBGCHK((_T("CSPDDK")),  FALSE);
            ERRORMSG(1, (_T("GpioAlloc:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    if (g_pGPIO[DDK_GPIO_PORT2] == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_GPIO2;

        // Map peripheral physical address to virtual address
        g_pGPIO[DDK_GPIO_PORT2] =
            (PCSP_GPIO_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_GPIO_REGS),
                                          FALSE);

        // Check if virtual mapping failed
        if (g_pGPIO[DDK_GPIO_PORT2] == NULL)
        {
            DBGCHK((_T("CSPDDK")),  FALSE);
            ERRORMSG(1, (_T("GpioAlloc:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    if (g_pGPIO[DDK_GPIO_PORT3] == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_GPIO3;

        // Map peripheral physical address to virtual address
        g_pGPIO[DDK_GPIO_PORT3] =
            (PCSP_GPIO_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_GPIO_REGS),
                                          FALSE);

        // Check if virtual mapping failed
        if (g_pGPIO[DDK_GPIO_PORT3] == NULL)
        {
            DBGCHK((_T("CSPDDK")),  FALSE);
            ERRORMSG(1, (_T("GpioAlloc:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:

    if (!rc) GpioDealloc();

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  GpioDealloc
//
// This function deallocates the data structures required for interaction
// with the GPIO hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL GpioDealloc(void)
{
    // Unmap peripheral address space
    if (g_pGPIO[DDK_GPIO_PORT1] != NULL)
    {
        MmUnmapIoSpace(g_pGPIO[DDK_GPIO_PORT1], sizeof(CSP_GPIO_REGS));
        g_pGPIO[DDK_GPIO_PORT1] = NULL;
    }

    if (g_pGPIO[DDK_GPIO_PORT2] != NULL)
    {
        MmUnmapIoSpace(g_pGPIO[DDK_GPIO_PORT2], sizeof(CSP_GPIO_REGS));
        g_pGPIO[DDK_GPIO_PORT2] = NULL;
    }

    if (g_pGPIO[DDK_GPIO_PORT3] != NULL)
    {
        MmUnmapIoSpace(g_pGPIO[DDK_GPIO_PORT3], sizeof(CSP_GPIO_REGS));
        g_pGPIO[DDK_GPIO_PORT3] = NULL;
    }

    return TRUE;
}
