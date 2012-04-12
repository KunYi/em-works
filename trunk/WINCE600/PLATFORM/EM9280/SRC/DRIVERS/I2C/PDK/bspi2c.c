//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bspi2c.c
//
//  Provides BSP-specific configuration routines for the I2C peripheral.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "bsp.h"

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

// default timeout value for interrupt waiting
#define DEFAULT_TIMEOUT_VALUE           5000            // 5000 ms

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function:  BSPI2CIOMUXConfig
//
// This function makes the DDK call to configure the IOMUX
// pins required for the I2C.
//
// Parameters:
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL BSPI2CIOMUXConfig(DWORD index)
{   
    switch(index)
    {
        case 0:
            // I2C_CLK
            DDKIomuxSetPinMux(DDK_IOMUX_I2C0_SCL_1, DDK_IOMUX_MODE_00);
            DDKIomuxSetPadConfig(DDK_IOMUX_I2C0_SCL_1, 
                                 DDK_IOMUX_PAD_DRIVE_8MA, 
                                 DDK_IOMUX_PAD_PULL_ENABLE,
                                 DDK_IOMUX_PAD_VOLTAGE_RESERVED);
            //DDKIomuxEnablePullup(DDK_IOMUX_I2C0_SCL_0, TRUE);
        
            // I2C_SDA
            DDKIomuxSetPinMux(DDK_IOMUX_I2C0_SDA_1, DDK_IOMUX_MODE_00);
            DDKIomuxSetPadConfig(DDK_IOMUX_I2C0_SDA_1, 
                                 DDK_IOMUX_PAD_DRIVE_8MA, 
                                 DDK_IOMUX_PAD_PULL_ENABLE,
                                 DDK_IOMUX_PAD_VOLTAGE_RESERVED);
            //DDKIomuxEnablePullup(DDK_IOMUX_I2C0_SDA_0, TRUE);
            break;
        case 1:
            // I2C_CLK
            DDKIomuxSetPinMux(DDK_IOMUX_I2C1_SCL_0, DDK_IOMUX_MODE_00);
            DDKIomuxSetPadConfig(DDK_IOMUX_I2C1_SCL_0, 
                                 DDK_IOMUX_PAD_DRIVE_8MA, 
                                 DDK_IOMUX_PAD_PULL_ENABLE,
                                 DDK_IOMUX_PAD_VOLTAGE_RESERVED);
            //DDKIomuxEnablePullup(DDK_IOMUX_I2C1_SCL_0, TRUE);
        
            // I2C_SDA
            DDKIomuxSetPinMux(DDK_IOMUX_I2C1_SDA_0, DDK_IOMUX_MODE_00);
            DDKIomuxSetPadConfig(DDK_IOMUX_I2C1_SDA_0, 
                                 DDK_IOMUX_PAD_DRIVE_8MA, 
                                 DDK_IOMUX_PAD_PULL_ENABLE,
                                 DDK_IOMUX_PAD_VOLTAGE_RESERVED);
            //DDKIomuxEnablePullup(DDK_IOMUX_I2C1_SDA_0, TRUE);
            break;
        default:
            break;   
    }
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  BSPI2CGetTimeout
//
// This function returns the timeout value when waiting for interrupts
//
// Parameters:
//
// Returns:
//      Timeout value
//
//-----------------------------------------------------------------------------
DWORD BSPI2CGetTimeout()
{
    return DEFAULT_TIMEOUT_VALUE;
}

