//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  File:  bspsim.c
//
//   This file implements the BSP specific functions for SIM.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>
#pragma warning(pop)
#include "bsp.h"


// We don't use the VEN bit of the SIM controller to enable the voltage,
// because the VEN signal of SIM1 is also used to enable the SIM2 voltage
// Instead we just use a PIO to enable the voltage for both. Once enable the voltage 
// is never disabled.
// To use the VEN bit to driver the VEN signal, comment the following line.
#define DRIVE_VEN_AS_PIO    1

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

// This will define the choice of clock source in SIM register

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
// Function: BSPGetSIMCLK
//
// This function returns the BSP-specific clock
// source selection value for the SIM.
//
// Parameters:
//      dwIndex
//          [in] The controller ID (1 for SIM1 or 2 for SIM2)
//
// Returns:
//      The clock source for the SIM.
//
//------------------------------------------------------------------------------
UINT32 BSPGetSIMCLK(DWORD dwIndex)
{
    UINT32 freq = 0;

    switch (dwIndex)
    {
        case 1 : 
            DDKClockGetFreq(DDK_CLOCK_SIGNAL_PER_SIM1, &freq); 
            break;
        case 2 : 
            DDKClockGetFreq(DDK_CLOCK_SIGNAL_PER_SIM2, &freq); 
            break;
    }

    return freq;
}

//------------------------------------------------------------------------------
//
// Function: BSPSetSIMClockGatingMode
//
// This function enable or disable CRM clock for SIM.
//
// Parameters:
//      dwIndex
//          [in] The controller ID (1 for SIM1 or 2 for SIM2)
//      startClocks
//          [in] boolean variable to enable or disable CRM clock
//
// Returns:
//      Return TRUE.
//
//------------------------------------------------------------------------------
BOOL BSPSetSIMClockGatingMode(DWORD dwIndex, BOOL startClocks)
{
    BOOL rc = FALSE;
    DWORD dwClkPerGate,dwClkGate;
    DDK_CLOCK_GATE_MODE mode;       

    switch (dwIndex)
    {
        case 1 : 
            dwClkPerGate = DDK_CLOCK_GATE_INDEX_PER_SIM1;
            dwClkGate = DDK_CLOCK_GATE_INDEX_SIM1;
            break;
        case 2 :
            dwClkPerGate = DDK_CLOCK_GATE_INDEX_PER_SIM2;
            dwClkGate = DDK_CLOCK_GATE_INDEX_SIM2;
            break;
        default:
                return rc;
    }

    if (startClocks)
    {
        mode = DDK_CLOCK_GATE_MODE_ENABLED;
    }
    else
    {
        mode = DDK_CLOCK_GATE_MODE_DISABLED;        
    }

    rc = DDKClockSetGatingMode(dwClkPerGate, mode);
    rc = rc && DDKClockSetGatingMode(dwClkGate, mode);

    return rc;
}
//------------------------------------------------------------------------------
//
// Function: BSPSimIomuxSetPin
//
// This function Setup pins' IOMUX for SIM.
//
// Parameters:
//      dwIndex
//          [in] The controller ID (1 for SIM1 or 2 for SIM2)
//      dwPort
//          [in] The port used (port 0 or port 1)
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPSimIomuxSetPin(DWORD dwIndex,DWORD dwPort)
{
    if ((dwIndex == 1) && (dwPort == 0))
    {
    //SIM1_CLK0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D2, DDK_IOMUX_PIN_MUXMODE_ALT4, 
            DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D2
            , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K,
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);

    //SIM1_RST0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D3, DDK_IOMUX_PIN_MUXMODE_ALT4, 
            DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D3
            , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);

#ifndef DRIVE_VEN_AS_PIO
    //SIM1_VEN0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D4, DDK_IOMUX_PIN_MUXMODE_ALT4, 
            DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D4
            , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
#else
    //SIM1_VEN0 configure as GPIO1_29
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D4, DDK_IOMUX_PIN_MUXMODE_ALT5, 
            DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D4
            , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKGpioSetConfig(DDK_GPIO_PORT1, 29, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT1, 29, 1);
#endif
    //SIM1_TX0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D5, DDK_IOMUX_PIN_MUXMODE_ALT4, 
                DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D5
                , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
                DDK_IOMUX_PAD_OPENDRAIN_ENABLE, DDK_IOMUX_PAD_PULL_NONE,
                DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);

    //SIM1_PD0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D6, DDK_IOMUX_PIN_MUXMODE_ALT4, 
                DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D6
                , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
                DDK_IOMUX_PAD_OPENDRAIN_ENABLE, DDK_IOMUX_PAD_PULL_NONE,
                DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
    }
    else if ((dwIndex == 2) && (dwPort == 0))
    {
    //SIM2_CLK0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D8, DDK_IOMUX_PIN_MUXMODE_ALT4, 
            DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D8
            , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K,
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);

    //SIM2_RST0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D9, DDK_IOMUX_PIN_MUXMODE_ALT4, 
            DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D9
            , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);

#ifndef DRIVE_VEN_AS_PIO
    //SIM2_VEN0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_MCLK, DDK_IOMUX_PIN_MUXMODE_ALT4, 
            DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_MCLK
            , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
#else
    //SIM1_VEN0 configure as GPIO1_29
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D4, DDK_IOMUX_PIN_MUXMODE_ALT5, 
            DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_D4
            , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKGpioSetConfig(DDK_GPIO_PORT1, 29, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT1, 29, 1);
#endif

    //SIM2_TX0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_VSYNC, DDK_IOMUX_PIN_MUXMODE_ALT4, 
                DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_VSYNC
                , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
                DDK_IOMUX_PAD_OPENDRAIN_ENABLE, DDK_IOMUX_PAD_PULL_NONE,
                DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);

    //SIM2_PD0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_HSYNC, DDK_IOMUX_PIN_MUXMODE_ALT4, 
                DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSI_HSYNC
                , DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
                DDK_IOMUX_PAD_OPENDRAIN_ENABLE, DDK_IOMUX_PAD_PULL_NONE,
                DDK_IOMUX_PAD_HYSTERESIS_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
    }
        
}
//------------------------------------------------------------------------------
//
// Function: BSPSimSelect3Volts
//
// Stub function.
//
// Parameters:
//      dwIndex
//          [in] The controller ID (1 for SIM1 or 2 for SIM2)
//      dwPort
//          [in] The port used (port 0 or port 1)
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPSimSelect3Volts(DWORD dwIndex,DWORD dwPort)
{
    //Stub function:
    //Nothing to implement here as the power to the SIM interface comes from the PMIC and is shared with a lot of other peripherals.
    UNREFERENCED_PARAMETER(dwIndex);
    UNREFERENCED_PARAMETER(dwPort);
}
