//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  File:  GPSCtlPdd.c
//
//  Provides BSP-specific configuration routines for the GPS peripheral.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "bsp.h"


//-----------------------------------------------------------------------------


/// Initialize the HW.
/// Initialize the HW registers to proper stopped state
bool initHw(LPCTSTR pContext)
{
    UNREFERENCED_PARAMETER(pContext);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+initHw\r\n")));

    // Clock enable
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_VSTBY_ACK, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_VSTBY_ACK, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKGpioSetConfig(DDK_GPIO_PORT3, 18, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT3, 18, 1);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-initHw\r\n")));
    return TRUE;
}


bool deinitHw(DWORD deviceContext)
{
    UNREFERENCED_PARAMETER(deviceContext);
    // Clock enable
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_VSTBY_ACK, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_VSTBY_ACK, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKGpioSetConfig(DDK_GPIO_PORT3, 18, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT3, 18, 0);
    return true;
}



void enableAsicReset()
{
    DEBUGMSG(ZONE_INIT, (TEXT("enableAsicReset()\r\n")));
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D9, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D9, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKGpioSetConfig(DDK_GPIO_PORT4, 11, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT4, 11, 1);
}

void disableAsicReset()
{
    DEBUGMSG(ZONE_INIT, (TEXT("disableAsicReset()\r\n")));

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D9, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D9, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKGpioSetConfig(DDK_GPIO_PORT4, 11, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT4, 11, 0);
}

void enableAsicPowerOn()
{
    DEBUGMSG(ZONE_INIT, (TEXT("enableAsicPowerOn()\r\n")));

    // Power enable
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D8, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D8, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKGpioSetConfig(DDK_GPIO_PORT4, 12, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT4, 12, 1);
}

void disableAsicPowerOn()
{
    DEBUGMSG(ZONE_INIT, (TEXT("disableAsicPowerOn()\r\n")));

    // Power disable
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D8, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D8, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKGpioSetConfig(DDK_GPIO_PORT4, 12, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT4, 12, 0);
}
