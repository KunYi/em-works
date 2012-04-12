//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bsppxp.cpp
//
//  Provides BSP-specific configuration routines for the Post-processing peripheral.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

//#include "bsp.h"

//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: BSPSetPXPISRPriority
//
// This function sets the thread priority for
// the Post-processor ISR.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPSetPXPISRPriority()
{
    CeSetThreadPriority(GetCurrentThread(), 100);
    return;
}

