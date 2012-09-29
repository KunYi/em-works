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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  power.c
//
//  Power BSP callback functions implementation. This function are called as
//  last function before OALCPUPowerOff. The KITL was already disabled by
//  OALKitlPowerOff.
//
//-----------------------------------------------------------------------------

#include <bsp.h>

//-----------------------------------------------------------------------------
// External Functions
extern void OALCPUEnterWFI(void);


//-----------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregPOWER;
//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
//
//  Function:  OALPowerInit
//
//  This function initializes the power management hardware on the CPU.
//-----------------------------------------------------------------------------
VOID OALPowerInit()
{
}

//-----------------------------------------------------------------------------
//
//  Function:  OALCPUPowerOff
//
//  This function powers off CPU.
//-----------------------------------------------------------------------------
VOID OALCPUPowerOff()
{
}

//-----------------------------------------------------------------------------
//
//  Function:  BSPPowerOff
//
//  This function performs board-level power off operations.
//-----------------------------------------------------------------------------
VOID BSPPowerOff()
{
}

//-----------------------------------------------------------------------------
//
//  Function:  BSPPowerOn
//
//  This function performs board-level power on operations.
//-----------------------------------------------------------------------------
VOID BSPPowerOn()
{
}

//-----------------------------------------------------------------------------
//
//  Function:  GetPowerOffIRAMSize
//
//  This function return PowerOff IRAM Alloc Size.
//

UINT32 GetPowerOffIRAMSize()
{
    return IMAGE_WINCE_POWEROFF_IRAM_SIZE;
}

