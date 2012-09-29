//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  pmic_bklight.cpp
//
//  This file contains the PMIC Backlight SDK interface that is used by
//  applications and other drivers to access registers of the PMIC.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_macros.h"
#include "pmic_ioctl.h"
#include "pmic_basic_types.h"
#include "pmic_bklight.h"

#include "regs.h"
#include "regs_led.h"

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

extern HANDLE hPMI;

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNC          (1 << ZONEID_FUNC)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC              DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)

extern DBGPARAM dpCurSettings;

#endif

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: PmicBacklightMasterEnable
//
// This function sets the master enable bit of the PMIC backlight and tri-color led controller.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightMasterEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_LED_CTL0_LEDEN, MC13783_LED_CTL0_LEDEN_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_LEDEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightMasterDisable
//
// This function clears the master enable bit of the PMIC backlight and tri-color led controller.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightMasterDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    //TODO: This bit controls both backlight and tri-color led channels. 
    //extra steps need to be done to avoid conflit.
    param.addr = MC13783_LED_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_LED_CTL0_LEDEN, MC13783_LED_CTL0_LEDEN_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_LEDEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightRampUp
//
// This function ramps up the PMIC backlight channel.
//
// Parameters:
//      channel [IN]    backlight channel
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightRampUp(BACKLIGHT_CHANNEL channel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_LEDMDRAMPUP, MC13783_LED_CTL0_LEDRAMPUP_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_LEDMDRAMPUP) |
                CSP_BITFMASK(MC13783_LED_CTL0_LEDMDRAMPDOWN);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_LEDADRAMPUP, MC13783_LED_CTL0_LEDRAMPUP_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_LEDADRAMPUP) |
                CSP_BITFMASK(MC13783_LED_CTL0_LEDADRAMPDOWN);
            break;

        case BACKLIGHT_KEYPAD:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_LEDKPRAMPUP, MC13783_LED_CTL0_LEDRAMPUP_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_LEDKPRAMPUP) |
                CSP_BITFMASK(MC13783_LED_CTL0_LEDKPRAMPDOWN);
            break;
    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightRampDown
//
// This function ramps down the PMIC backlight channel.
//
// Parameters:
//      channel[IN]    backlight channel
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightRampDown(BACKLIGHT_CHANNEL channel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_LEDMDRAMPDOWN, MC13783_LED_CTL0_LEDRAMPDOWN_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_LEDMDRAMPUP) |
                CSP_BITFMASK(MC13783_LED_CTL0_LEDMDRAMPDOWN);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_LEDADRAMPDOWN, MC13783_LED_CTL0_LEDRAMPDOWN_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_LEDADRAMPUP) |
                CSP_BITFMASK(MC13783_LED_CTL0_LEDADRAMPDOWN);
            break;

        case BACKLIGHT_KEYPAD:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_LEDKPRAMPDOWN, MC13783_LED_CTL0_LEDRAMPDOWN_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_LEDKPRAMPUP) |
                CSP_BITFMASK(MC13783_LED_CTL0_LEDKPRAMPDOWN);
            break;
    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBacklightRampUpOff
//
// This function stops ramping of the PMIC backlight channel.
//
// Parameters:
//      channel [IN]    backlight channel
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightRampOff(BACKLIGHT_CHANNEL channel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_LEDMDRAMPUP, MC13783_LED_CTL0_LEDRAMPUP_DISABLE) |
                     CSP_BITFVAL(MC13783_LED_CTL0_LEDMDRAMPDOWN, MC13783_LED_CTL0_LEDRAMPDOWN_DISABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_LEDMDRAMPUP) |
                CSP_BITFMASK(MC13783_LED_CTL0_LEDMDRAMPDOWN);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_LEDADRAMPUP, MC13783_LED_CTL0_LEDRAMPUP_DISABLE) |
                     CSP_BITFVAL(MC13783_LED_CTL0_LEDADRAMPDOWN, MC13783_LED_CTL0_LEDRAMPDOWN_DISABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_LEDADRAMPUP) |
                CSP_BITFMASK(MC13783_LED_CTL0_LEDADRAMPDOWN);
            break;

        case BACKLIGHT_KEYPAD:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_LEDKPRAMPUP, MC13783_LED_CTL0_LEDRAMPUP_DISABLE) |
                     CSP_BITFVAL(MC13783_LED_CTL0_LEDKPRAMPDOWN, MC13783_LED_CTL0_LEDRAMPDOWN_DISABLE);             
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_LEDKPRAMPUP) |
                CSP_BITFMASK(MC13783_LED_CTL0_LEDKPRAMPDOWN);
            break;
    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightSetMode
//
// This function sets the operation mode for PMIC backlight channel.
//
// Parameters:
//      channel[IN]    backlight channel
//      mode[IN]    backlight operation mode
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightSetMode(BACKLIGHT_CHANNEL channel, BACKLIGHT_MODE mode)
{    
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_TRIODEMD, mode);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_TRIODEMD);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_TRIODEAD, mode);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_TRIODEAD);
            break;

        case BACKLIGHT_KEYPAD:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_TRIODEKP, mode);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_TRIODEKP);
            break;
    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBacklightGetMode
//
// This function sets the operation mode for PMIC backlight channel.
//
// Parameters:
//      channel[IN]    backlight channel
//      mode[OUT]    pointer to backlight operation mode
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightGetMode(BACKLIGHT_CHANNEL channel, BACKLIGHT_MODE *mode)
{    
    PMIC_PARAM_LLA_WRITE_REG param;
    UINT32                                    temp;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;
    param.data = 0;
    param.mask = 0;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param,
              sizeof(param), &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            *mode = (BACKLIGHT_MODE)CSP_BITFEXT(temp, MC13783_LED_CTL0_TRIODEMD);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            *mode = (BACKLIGHT_MODE)CSP_BITFEXT(temp, MC13783_LED_CTL0_TRIODEAD);
            break;

        case BACKLIGHT_KEYPAD:
            *mode = (BACKLIGHT_MODE)CSP_BITFEXT(temp, MC13783_LED_CTL0_TRIODEKP);
            break;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightSetCurrentLevel
//
//  This function program the current level for backlight channel.
//
//  Parameters:
//      channel[IN]    backlight channel
//      level[IN]    current level
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightSetCurrentLevel(BACKLIGHT_CHANNEL channel, UINT8 level)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check current level parameter
    if (level > MC13783_LED_MAX_BACKLIGHT_CURRENT_LEVEL)
    {
        ERRORMSG(1, (_T("PmicBacklightSetCurrentLevel Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13783_LED_CTL2_ADDR;

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL2_LEDMD, level);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL2_LEDMD);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL2_LEDAD, level);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL2_LEDAD);
            break;

        case BACKLIGHT_KEYPAD:
            param.data = CSP_BITFVAL(MC13783_LED_CTL2_LEDKP, level);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL2_LEDKP);
            break;
    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightGetCurrentLevel
//
//  This function returns the current level for backlight channel.
//
//  Parameters:
//      channel[IN]    backlight channel
//      level[OUT]    pointer to current level
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightGetCurrentLevel(BACKLIGHT_CHANNEL channel, UINT8* level)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    UINT32 temp = 0;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL2_ADDR;
    param.data = 0;
    param.mask = 0;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            *level  = (UINT8)CSP_BITFEXT(temp, MC13783_LED_CTL2_LEDMD);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            *level  = (UINT8)CSP_BITFEXT(temp, MC13783_LED_CTL2_LEDAD);
            break;

        case BACKLIGHT_KEYPAD:
            *level  = (UINT8)CSP_BITFEXT(temp, MC13783_LED_CTL2_LEDKP);
            break;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightSetDutyCycle
//
//  This function program the duty cycle for backlight channel.
//
//  Parameters:
//      channel[IN]    backlight channel
//      cycle[IN]    duty cycle
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightSetDutyCycle(BACKLIGHT_CHANNEL channel, UINT8 cycle)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check duty cycle parameter
    if (cycle > MC13783_LED_MAX_BACKLIGHT_DUTY_CYCLE)
    {
        ERRORMSG(1, (_T("PmicBacklightSetDutyCycle Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13783_LED_CTL2_ADDR;

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL2_LEDMDDC, cycle);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL2_LEDMDDC);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.data = CSP_BITFVAL(MC13783_LED_CTL2_LEDADDC, cycle);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL2_LEDADDC);
            break;

        case BACKLIGHT_KEYPAD:
            param.data = CSP_BITFVAL(MC13783_LED_CTL2_LEDKPDC, cycle);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL2_LEDKPDC);
            break;
    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightGetDutyCycle
//
//  This function returns the duty cycle for backlight channel.
//
//  Parameters:
//      channel[IN]    backlight channel
//      cycle[OUT]    duty cycle
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightGetDutyCycle(BACKLIGHT_CHANNEL channel, UINT8* cycle)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    UINT32 temp;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL2_ADDR;
    param.data = 0;
    param.mask = 0;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            *cycle  = (UINT8)CSP_BITFEXT(temp, MC13783_LED_CTL2_LEDMDDC);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            *cycle  = (UINT8)CSP_BITFEXT(temp, MC13783_LED_CTL2_LEDADDC);
            break;

        case BACKLIGHT_KEYPAD:
            *cycle  = (UINT8)CSP_BITFEXT(temp, MC13783_LED_CTL2_LEDKPDC);
            break;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightSetCycleTime
//
//  This function program the cycle period for backlight controller.
//
//  Parameters:
//      period[IN]    cycle time.
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightSetCycleTime(UINT8 period)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check cycle time parameter
    if (period > MC13783_LED_MAX_BACKLIGHT_PERIOD)
    {
        ERRORMSG(1, (_T("PmicBacklightSetCycleTime Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13783_LED_CTL2_ADDR;
    param.data = CSP_BITFVAL(MC13783_LED_CTL2_BLPERIOD, period);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL2_BLPERIOD);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightGetCycleTime
//
//  This function returns the cycle period for backlight controller.
//
//  Parameters:
//      period[OUT]    pointer to the cycle time.
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightGetCycleTime(UINT8* period)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    UINT32 temp;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL2_ADDR;
    param.data = 0;
    param.mask = 0;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *period  = (UINT8)CSP_BITFEXT(temp, MC13783_LED_CTL2_BLPERIOD);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightEnableEdgeSlow
//
// This function enables the edge slowing for backlight.
//
// Parameters:
//      None. 
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightEnableEdgeSlow()
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL2_ADDR;
    param.data = CSP_BITFVAL(MC13783_LED_CTL2_SLEWLIMBL, MC13783_LED_CTL2_SLEWLIMBL_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL2_SLEWLIMBL);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBacklightDisableEdgeSlow
//
// This function disables the edge slowing for backlight.
//
// Parameters:
//      None. 
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightDisableEdgeSlow()
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL2_ADDR;
    param.data = CSP_BITFVAL(MC13783_LED_CTL2_SLEWLIMBL, MC13783_LED_CTL2_SLEWLIMBL_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL2_SLEWLIMBL);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBacklightGetEdgeSlow
//
// This function gets the edge slowing status for backlight.
//
// Parameters:
//      edge[OUT]    pointer to status of edge slowing for backlight. 
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightGetEdgeSlow(BOOL *edge)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    UINT32                                    temp;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL2_ADDR;
    param.data = 0;
    param.mask = 0;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *edge  = CSP_BITFEXT(temp, MC13783_LED_CTL2_SLEWLIMBL);
            
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightEnableBoostMode
//
// This function enables boost mode for backlight. (Only availabe on MC13783.
//
// Parameters:
//      none
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightEnableBoostMode()
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_LED_CTL0_BOOSTEN, MC13783_LED_CTL0_BOOST_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_BOOSTEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightDisableBoostMode
//
// This function disables boost mode for backlight. (Only availabe on MC13783.
//
// Parameters:
//      none
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightDisableBoostMode()
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_LED_CTL0_BOOSTEN, MC13783_LED_CTL0_BOOST_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_BOOSTEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightSetBoostMode
//
// This function gets the edge slowing status for backlight.
//
// Parameters:
//      edge[OUT]    pointer to status of edge slowing for backlight. 
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightSetBoostMode(UINT32 abms, UINT32 abr)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check adaptive boost mode selection  parameter
    if (abms > MC13783_LED_MAX_BACKLIGHT_BOOST_ABMS)
    {
        ERRORMSG(1, (_T("PmicBacklightSetBoostMode Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    // check adaptive boost reference parameter
    if (abr > MC13783_LED_MAX_BACKLIGHT_BOOST_ABR)
    {
        ERRORMSG(1, (_T("PmicBacklightSetBoostMode Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13783_LED_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_LED_CTL0_BOOSTABMS, abms) |
                   CSP_BITFVAL(MC13783_LED_CTL0_BOOSTABR, abr);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_BOOSTABMS) |
                    CSP_BITFMASK(MC13783_LED_CTL0_BOOSTABR);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


