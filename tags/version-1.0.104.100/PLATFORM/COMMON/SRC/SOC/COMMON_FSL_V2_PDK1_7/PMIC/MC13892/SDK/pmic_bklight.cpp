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
#include "pmic.h"

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

//------------------------------------------------------------------------------
//
// Function: PmicBacklightEnableHIMode
//
// This function set PMIC backlight channel high current mode
//
// Parameters:
//      channel [IN]    backlight channel
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightEnableHIMode(BACKLIGHT_CHANNEL channel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDMDHI, MC13892_LED_CTL0_LEDMDHI_ENABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDMDHI) ;
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDADHI, MC13892_LED_CTL0_LEDADHI_ENABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDADHI) ;           
            break;

        case BACKLIGHT_KEYPAD:
            param.addr = MC13892_LED_CTL1_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL1_LEDKPHI, MC13892_LED_CTL1_LEDKPHI_ENABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL1_LEDKPHI) ;
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
// Function: PmicBacklightEnableHIMode
//
// This function disable  PMIC backlight channel in  high current mode
//
//
// Parameters:
//      channel[IN]    backlight channel
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightDisableHIMode(BACKLIGHT_CHANNEL channel)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDMDHI, MC13892_LED_CTL0_LEDMDHI_DISABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDMDHI) ;
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDADHI, MC13892_LED_CTL0_LEDADHI_DISABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDADHI) ;
              
            break;

        case BACKLIGHT_KEYPAD:
            param.addr = MC13892_LED_CTL1_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL1_LEDKPHI, MC13892_LED_CTL1_LEDKPHI_DISABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL1_LEDKPHI) ;
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
// Function: PmicBacklightEnableRamp
//
// This function sets the operation mode for PMIC backlight channel in Ramp Mode 
//
// Parameters:
//      channel[IN]    backlight channel
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightEnableRamp(BACKLIGHT_CHANNEL channel)
{    
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDMDRAMP, MC13892_LED_CTL0_LEDMDRAMP_ENABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDMDRAMP) ;
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDADRAMP, MC13892_LED_CTL0_LEDADRAMP_ENABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDADRAMP) ;
              
            break;

        case BACKLIGHT_KEYPAD:
            param.addr = MC13892_LED_CTL1_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL1_LEDKPRAMP, MC13892_LED_CTL1_LEDKPRAMP_ENABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL1_LEDKPRAMP) ;
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
//  Function: PmicBacklightSetCurrentLevel
//
//This function disable  the operation mode for PMIC backlight channel in Ramp Mode 
//
//  Parameters:
//      channel[IN]    backlight channel
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightDisableRamp(BACKLIGHT_CHANNEL channel)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));  

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDMDRAMP, MC13892_LED_CTL0_LEDMDRAMP_DISABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDMDRAMP) ;
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDADRAMP, MC13892_LED_CTL0_LEDADRAMP_DISABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDADRAMP) ;           
            break;

        case BACKLIGHT_KEYPAD:
            param.addr = MC13892_LED_CTL1_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL1_LEDKPRAMP, MC13892_LED_CTL1_LEDKPRAMP_DISABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL1_LEDKPRAMP) ;
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
//  Function: PmicBacklightSetCurrentLevel
//
//  This function program the current level for backlight channel.
//
//  Parameters:
//      channel[IN]    backlight channel
//      level[IN]    current level
//    000 0  0
//    001 3  6
//    010 6 12
//    011 9 18
//    100 12 24
//    101 15 30   
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
    if (level > MC13892_LED_MAX_BACKLIGHT_CURRENT_LEVEL)
    {
        ERRORMSG(1, (_T("PmicBacklightSetCurrentLevel Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDMD, level);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDMD);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDAD, level);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDAD);
            break;

        case BACKLIGHT_KEYPAD:
            param.addr = MC13892_LED_CTL1_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL1_LEDKP, level);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL1_LEDKP);
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

    param.addr = (channel==BACKLIGHT_KEYPAD)?MC13892_LED_CTL1_ADDR:MC13892_LED_CTL0_ADDR;
    param.data = 0;
    param.mask = 0;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            *level  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL0_LEDMD);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            *level  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL0_LEDAD);
            break;

        case BACKLIGHT_KEYPAD:
            *level  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL1_LEDKP);
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
//        LEDxDC[5:0] Duty Cycle
//        000000 0/32
//        000001 1/32
//
//        бн бн
//        010000 16/32
//        бн бн
//        011111 31/32
//        100000 to 111111 32/32, Continuously On
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightSetDutyCycle(BACKLIGHT_CHANNEL channel, UINT8 cycle)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check current level parameter
   if (cycle > MC13892_LED_MAX_BACKLIGHT_DUTY_CYCLE)
    {
        ERRORMSG(1, (_T("PmicBacklightSetDutyCycle Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDMDDC, cycle);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDMDDC);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            param.addr = MC13892_LED_CTL0_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL0_LEDADDC, cycle);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL0_LEDADDC);
            break;

        case BACKLIGHT_KEYPAD:
            param.addr = MC13892_LED_CTL1_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL1_LEDKPDC, cycle);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL1_LEDKPDC);
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
    UINT32 temp = 0;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = (channel==BACKLIGHT_KEYPAD)?MC13892_LED_CTL1_ADDR:MC13892_LED_CTL0_ADDR;
    param.data = 0;
    param.mask = 0;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    switch (channel)
    {
        case BACKLIGHT_MAIN_DISPLAY:
            *cycle  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL0_LEDMDDC);
            break;

        case BACKLIGHT_AUX_DISPLAY:
            *cycle  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL0_LEDADDC);
            break;

        case BACKLIGHT_KEYPAD:
            *cycle  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL1_LEDKPDC);
            break;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



