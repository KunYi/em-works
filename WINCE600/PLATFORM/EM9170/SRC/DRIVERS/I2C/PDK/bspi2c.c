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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
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
#define I2C_INTERRUPT_WAIT_DELAY     500

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
// Function: BSPI2CGetModuleClock
//
// Retrieves the clock frequency in Hz for the specified i2c module.
//
// Parameters:
//      index
//          [in] Index of the I2C device requested.
//      pdwFreq
//          [out] Contains frequency of the specified i2c module clock in Hz.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPI2CGetModuleClock(UINT32 index, PDWORD pdwFreq)
{
    BOOL bRet;
    switch(index)
    {
    default:
        bRet=DDKClockGetFreq(DDK_CLOCK_SIGNAL_PER_I2C, (UINT32*)pdwFreq);
    }

    return bRet;
}
    
//-----------------------------------------------------------------------------
//
// Function:  BSPI2CIOMUXConfig
//
// This function makes the DDK call to configure the IOMUX
// pins required for the I2C.
//
// Parameters:
//      index
//          [in] Index of the I2C device requested.
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL BSPI2CIOMUXConfig(UINT32 index)
{
    // Configure IOMUX to set I2C pins
    switch (index)
    {
        case 1:
            //slow slew, pull-up enabled,  3.3V, Hysteresis enabled
            DDKIomuxSetPinMux(DDK_IOMUX_PIN_I2C1_DAT, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_FORCE);
            DDKIomuxSetPadConfig(DDK_IOMUX_PAD_I2C1_DAT, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_MAX,DDK_IOMUX_PAD_OPENDRAIN_ENABLE,DDK_IOMUX_PAD_PULL_UP_100K,DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
            DDKIomuxSetPinMux(DDK_IOMUX_PIN_I2C1_CLK, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_FORCE);
            DDKIomuxSetPadConfig(DDK_IOMUX_PAD_I2C1_CLK, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_MAX,DDK_IOMUX_PAD_OPENDRAIN_ENABLE,DDK_IOMUX_PAD_PULL_UP_100K,DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
            break;
        // current HW does not use the corresponding pads for i2c2, i2c3
        case 2:
          DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_RDATA1, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
          DDKIomuxSetPadConfig(DDK_IOMUX_PAD_FEC_RDATA1, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_MAX,DDK_IOMUX_PAD_OPENDRAIN_ENABLE,DDK_IOMUX_PAD_PULL_UP_100K,DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
          DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_RX_DV, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
          DDKIomuxSetPadConfig(DDK_IOMUX_PAD_FEC_RX_DV, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_MAX,DDK_IOMUX_PAD_OPENDRAIN_ENABLE,DDK_IOMUX_PAD_PULL_UP_100K,DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
          break;
        case 3:
          DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_A, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
          DDKIomuxSetPadConfig(DDK_IOMUX_PAD_GPIO_A, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_MAX,DDK_IOMUX_PAD_OPENDRAIN_ENABLE,DDK_IOMUX_PAD_PULL_UP_100K,DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
          DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_B, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
          DDKIomuxSetPadConfig(DDK_IOMUX_PAD_GPIO_B, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_MAX,DDK_IOMUX_PAD_OPENDRAIN_ENABLE,DDK_IOMUX_PAD_PULL_UP_100K,DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
          break;

        default:
            return FALSE;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: BSPI2CEnableClock
//
// This function is a wrapper for I2C to enable/disable its clock using a valid
// CRM handle.
//
// Parameters:
//      index
//          [in]    Index specifying the I2C module.
//      bEnable
//          [in]    TRUE if I2C Clock is to be enabled. FALSE if I2C Clock is
//                  to be disabled.
//
// Returns:
//      TRUE if successfully performed the required action.
//
//-----------------------------------------------------------------------------
BOOL BSPI2CEnableClock(UINT32 index, BOOL bEnable)
{
    BOOL bResult;
    DDK_CLOCK_GATE_INDEX clockGateIndex;

    switch (index)
    {
        case 1:
            clockGateIndex = DDK_CLOCK_GATE_INDEX_I2C1;
            break;
        case 2:
            clockGateIndex = DDK_CLOCK_GATE_INDEX_I2C2;
            break;
        case 3:
            clockGateIndex = DDK_CLOCK_GATE_INDEX_I2C3;
            break;
        default:
            DEBUGMSG(1, (TEXT("Invalid I2C module index\r\n")));
            return FALSE;
    }

    if (bEnable)
    {
        bResult = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, 
            DDK_CLOCK_GATE_MODE_ENABLED);
        return bResult && DDKClockSetGatingMode(clockGateIndex, 
            DDK_CLOCK_GATE_MODE_ENABLED);
    }
    else
    {
        bResult = DDKClockSetGatingMode(clockGateIndex, 
            DDK_CLOCK_GATE_MODE_DISABLED);
        return bResult;
    }
}

//-----------------------------------------------------------------------------
//
// Function: BSPGetTimeoutValue
//
// This function returns an integer representing the wait time that
// occurs before timing out while waiting for an I2C interrupt.
// CRM handle.
//
// Parameters:
//      None.
//
// Returns:
//      Interrupt wait timeout value.
//
//-----------------------------------------------------------------------------
INT BSPGetTimeoutValue(void)
{
    return I2C_INTERRUPT_WAIT_DELAY;
}
