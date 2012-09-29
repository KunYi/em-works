//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  bspbacklight.c
//
//  Provides BSP-specific configuration routines for the backlight driver.
//  The routines in this file assume that we use IPU built-in pulse-width
//  modulator,
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4201)
#include <windows.h>
#pragma warning(pop)

#include "bsp.h"

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
static PCSP_LCDC_REGS gpBackLightLCD;

//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: BSPBacklightInitialize
//
// This function maps the LCDC registers and configure default pwm value.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPBacklightInitialize()
{
    PHYSICAL_ADDRESS phyAddr;

    phyAddr.QuadPart = CSP_BASE_REG_PA_LCDC;

    // Map peripheral physical address to virtual address
    gpBackLightLCD = (PCSP_LCDC_REGS)MmMapIoSpace(phyAddr, sizeof(CSP_LCDC_REGS), FALSE);

    OUTREG32(&gpBackLightLCD->PCCR,
      (CSP_BITFVAL(LCDC_PCCR_PW, CSP_BITFVAL(LCDC_PCCR_PW,LCDC_PCCR_PW_MAX / 2)) |           
       CSP_BITFVAL(LCDC_PCCR_CC_EN, LCDC_PCCR_CC_EN_ENABLE) |
       CSP_BITFVAL(LCDC_PCCR_SCR, LCDC_PCCR_SCR_LCDCLK) |  //LCDC_PCCR_SCR_PIXELCLK
       CSP_BITFVAL(LCDC_PCCR_LDMSK, LCDC_PCCR_LDMSK_DISABLE) |
       CSP_BITFVAL(LCDC_PCCR_CLS_HI_WIDTH, 169)
      ));
}


//------------------------------------------------------------------------------
//
// Function: BSPBacklightRelease
//
// This function unmaps the LCDC registers.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPBacklightRelease()
{
    if (gpBackLightLCD != NULL)
    {
        MmUnmapIoSpace((PVOID)gpBackLightLCD, sizeof(CSP_LCDC_REGS));
    }

    return;
}


//------------------------------------------------------------------------------
//
// Function: BSPBacklightSetIntensity
//
// This function set backlight level.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPBacklightSetIntensity(DWORD level)
{    
    level = (level /25) *(0xc8/10); 
    INSREG32BF(&(gpBackLightLCD->PCCR), LCDC_PCCR_PW, level);
}

//------------------------------------------------------------------------------
//
// Function: BSPBacklightEnable
//
// This function enables the backligth.
// This function is called in power up and power down backlight driver
// functions. This function is actually empty because there is no parameter to 
// differenciate pouwer-up and power-down.
// The actual working code is in functions BSPBacklightInitialize and BSPBacklightSetIntensity
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPBacklightEnable()
{

}
