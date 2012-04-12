//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bspserial.c
//
//  Provides BSP-specific configuration routines for the UART peripheral.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "bsp.h"

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
// Function: BSPUartConfigureGPIO
//
// This function is used to configure the GPIO.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
//
// Returns:
//      TRUE if successfully performed the required action.
//
//-----------------------------------------------------------------------------
BOOL BSPUartConfigureGPIO(ULONG HWAddr)
{
    BOOL ret = TRUE;

    switch (HWAddr)
    {
    case CSP_BASE_REG_PA_UARTDBG:
        DDKIomuxSetPinMux(DDK_IOMUX_PWM1_1, DDK_IOMUX_MODE_02);      // TX
        DDKIomuxSetPadConfig(DDK_IOMUX_PWM1_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
        //DDKIomuxEnablePullup(DDK_IOMUX_PWM1_1, 1);

        DDKIomuxSetPinMux(DDK_IOMUX_PWM0_1, DDK_IOMUX_MODE_02);      // RX
        DDKIomuxSetPadConfig(DDK_IOMUX_PWM0_1,
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
        //DDKIomuxEnablePullup(DDK_IOMUX_PWM0_1, 1);

        break;

    default:
        ret=FALSE;
        break;
    }

    return ret;
}

//-----------------------------------------------------------------------------
//
// Function:  BSPGetIRQ
//
// This function returns the physical base address for debug uart.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
//
// Returns:
//      Physical IRQ  for debug uart
//
//-----------------------------------------------------------------------------
ULONG  BSPGetIRQ(ULONG HWAddr)
{
  
    switch (HWAddr)
    {
    case CSP_BASE_REG_PA_UARTDBG:
        return IRQ_DBG_UART;
    default:
        return 0;
    }
}

