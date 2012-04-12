//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  bspbacklight.c
//
//  Provides BSP-specific configuration routines for the backlight driver.
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4201)
#include <windows.h>
#pragma warning(pop)

#include "csp.h"
#include "hw_pwm.h"
#include "hw_lradc.h"

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines
// Defines
//! Min allowed brightness percentage
#define MIN_BRIGHTNESS_PERCENTAGE       0
//! Max allowed brightness percentage
#define MAX_BRIGHTNESS_PERCENTAGE       90
//! PWM Freq
#define BACKLIGHT_PWM_INPUT_FREQUENCY   15000
//! Init input active percent
#define BACKLIGHT_PWM_INIT_PERCENT      40
//! DIV the level passed from UI
#define LEVELDIV                        25
//! Resize the DIVed level passed from UI
#define LEVELMUL                        8


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
// Function: BSPBacklightInitialize
//
// This function maps the IPU registers.00000
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
    RETAILMSG(1, (TEXT("-->BSPBacklightInitialize\r\n")));
    if(!PwmInitialize())
    {
        ERRORMSG(1, (TEXT("BSPBacklightInitialize: failed!\r\n")));
        return;
    }     
    //Setup IOMUX
    PWMChSetIOMux(PWM_CHANNEL_2,DDK_IOMUX_MODE_00);

    //Setup initial frequency and duty cycle with 1 kHz
    PWMChSetConfig(PWM_CHANNEL_2,PWM_STATE_HIGH, PWM_STATE_LOW,
                   BACKLIGHT_PWM_INPUT_FREQUENCY, BACKLIGHT_PWM_INIT_PERCENT);
    //Enable PWM2
    PWMChOutputEnable(PWM_CHANNEL_2,TRUE);
    RETAILMSG(1, (TEXT("<--BSPBacklightInitialize\r\n")));
}
//------------------------------------------------------------------------------
//
// Function: BSPBacklightRelease
//
// This function unmaps the BackLight registers.
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
    level = (level / LEVELDIV) * LEVELMUL;

    // Set the raw values in the PWM registers
    if (level > MAX_BRIGHTNESS_PERCENTAGE)
    {
        level = MAX_BRIGHTNESS_PERCENTAGE;
    }
    if(level < MIN_BRIGHTNESS_PERCENTAGE)
    {
        level = MIN_BRIGHTNESS_PERCENTAGE;
    }

    PWMChSetDutyCycle(PWM_CHANNEL_2,PWM_STATE_HIGH, PWM_STATE_LOW,level);
    //Level = 0,disable PWM
    if (level == 0) 
    {
        PWMChOutputEnable(PWM_CHANNEL_2, FALSE);
    }
    //Enable PWM
    else 
    {
        PWMChOutputEnable(PWM_CHANNEL_2, TRUE);
    }
}

//------------------------------------------------------------------------------
//
// Function: BSPBacklightEnable
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
void BSPBacklightEnable()
{

}

