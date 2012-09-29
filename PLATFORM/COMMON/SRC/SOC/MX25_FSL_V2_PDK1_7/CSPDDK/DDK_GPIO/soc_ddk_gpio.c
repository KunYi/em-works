//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_ddk_gpio.c
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
// Following variables define the GPIO SoC-specific features.
BOOL g_SupportBothEdgeIntr = TRUE;
DDK_GPIO_PORT g_MaxPort = DDK_GPIO_PORT4;
PCSP_GPIO_REGS g_pGPIO[DDK_GPIO_PORT4+1];
PHYSICAL_ADDRESS g_PhyBaseAddress[DDK_GPIO_PORT4+1] = 
{
    {CSP_BASE_REG_PA_GPIO1, 0},
    {CSP_BASE_REG_PA_GPIO2, 0},
    {CSP_BASE_REG_PA_GPIO3, 0},
    {CSP_BASE_REG_PA_GPIO4, 0}
};
//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions
