/*---------------------------------------------------------------------------
* Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
//
//  File:  pmic_tcled.cpp
//
//  This file contains the PMIC TCLED SDK interface that is used by applications
//  and other drivers to access registers of the PMIC.
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
#include "pmic_tcled.h"

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
// Function: PmicLEDIndicatorEnableRamp
//
// This function sets the operation mode for PMIC Led channel in Ramp Mode 
//
// Parameters:
//      channel[IN]    backlight channel
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicLEDIndicatorEnableRamp(LED_CHANNEL channel)
{    
  
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (channel)
    {
        case TCLED_RED:
            param.addr = MC13892_LED_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL2_LEDRRAMP, MC13892_LED_CTL2_LEDRRAMP_ENABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL2_LEDRRAMP) ;
            break;

        case TCLED_GREEN:
            param.addr = MC13892_LED_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL2_LEDGRAMP, MC13892_LED_CTL2_LEDGRAMP_ENABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL2_LEDGRAMP) ;
              
            break;

        case TCLED_BLUE:
            param.addr = MC13892_LED_CTL3_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL3_LEDBRAMP, MC13892_LED_CTL3_LEDBRAMP_ENABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL3_LEDBRAMP) ;
            
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
//  Function: PmicLEDIndicatorDisableRamp
//
//This function disable  the operation mode forPMIC Led channel in Ramp Mode 
//
//  Parameters:
//      channel[IN]    backlight channel
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicLEDIndicatorDisableRamp(LED_CHANNEL channel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (channel)
    {
        case TCLED_RED:
            param.addr = MC13892_LED_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL2_LEDRRAMP, MC13892_LED_CTL2_LEDRRAMP_DISABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL2_LEDRRAMP) ;
            break;

        case TCLED_GREEN:
            param.addr = MC13892_LED_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL2_LEDGRAMP, MC13892_LED_CTL2_LEDGRAMP_DISABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL2_LEDGRAMP) ;
              
            break;

        case TCLED_BLUE:
            param.addr = MC13892_LED_CTL3_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL3_LEDBRAMP, MC13892_LED_CTL3_LEDBRAMP_DISABLE);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL3_LEDBRAMP) ;
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
//  Function: PmicLEDIndicatorSetCurrentLevel
//
//  This function program the current level for Led channel.
//
//  Parameters:
//      channel[IN]    led channel
//      level[IN]    current level
//    LEDx[2:0]  LEDx Current Level (mA)
//      000            0
//      001            3
//      010            6
//      011            9
//     100            12
//     101            15
//     110            18
//     111            21
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicLEDIndicatorSetCurrentLevel(LED_CHANNEL channel, UINT8 level)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check current level parameter
    if (level > MC13892_LED_MAX_LED_CURRENT_LEVEL)
    {
        ERRORMSG(1, (_T("PmicLEDIndicatorSetCurrentLevel Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    switch (channel)
    {
        case TCLED_RED:
            param.addr = MC13892_LED_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL2_LEDR, level);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL2_LEDR);
            break;

        case TCLED_GREEN:
            param.addr = MC13892_LED_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL2_LEDG, level);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL2_LEDG);
            break;

        case TCLED_BLUE:
            param.addr = MC13892_LED_CTL3_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL3_LEDB, level);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL3_LEDB);
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
//  Function: PmicLEDIndicatorGetCurrentLevel
//
//  This function returns the current level for led channel.
//
//  Parameters:
//      channel[IN]    led channel
//      level[OUT]    pointer to current level
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicLEDIndicatorGetCurrentLevel(LED_CHANNEL channel, UINT8* level)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    UINT32 temp = 0;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = (channel==TCLED_BLUE)?MC13892_LED_CTL3_ADDR:MC13892_LED_CTL2_ADDR;
    param.data = 0;
    param.mask = 0;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    switch (channel)
    {
        case TCLED_RED:
            *level  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL2_LEDR);
            break;

        case TCLED_GREEN:
            *level  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL2_LEDG);
            break;

        case TCLED_BLUE:
            *level  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL3_LEDB);
            break;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function: PmicLEDIndicatorSetDutyCycle
//
//  This function program the Duty Cycle  for Led channel.
//
//  Parameters:
//      channel[IN]    led channel
//      level[IN]    current level
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
PMIC_STATUS PmicLEDIndicatorSetDutyCycle(LED_CHANNEL channel, unsigned char dc)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check current level parameter
    if (dc > MC13892_LED_MAX_LED_DUTY_CYCLE)
    {
        ERRORMSG(1, (_T("PmicBacklightSetDutyCycle Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    
    switch (channel)
    {
        case TCLED_RED:
            param.addr = MC13892_LED_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL2_LEDRDC, dc);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL2_LEDRDC);
            break;

        case TCLED_GREEN:
            param.addr = MC13892_LED_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL2_LEDGDC, dc);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL2_LEDGDC);
            break;

        case TCLED_BLUE:
            param.addr = MC13892_LED_CTL3_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL3_LEDBDC, dc);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL3_LEDBDC);
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
//  Function: PmicLEDIndicatorGetDutyCycle
//
//  This function returns the Duty Cycle  for led channel.
//
//  Parameters:
//      channel[IN]    led channel
//      level[OUT]    pointer to Duty Cycle 
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicLEDIndicatorGetDutyCycle(LED_CHANNEL channel,  unsigned char* dc)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    UINT32 temp = 0;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = (channel==TCLED_BLUE)?MC13892_LED_CTL3_ADDR:MC13892_LED_CTL2_ADDR;
    param.data = 0;
    param.mask = 0;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    switch (channel)
    {
        case TCLED_RED:
            *dc  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL2_LEDRDC);
            break;

        case TCLED_GREEN:
            *dc  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL2_LEDGDC);
            break;

        case TCLED_BLUE:
            *dc  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL3_LEDBDC);
            break;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function: PmicLEDIndicatorSetBlinkPeriod
//
//  This function program the Blink Period  for Led channel.
//
//  Parameters:
//      channel[IN]    led channel
//      level[IN]    current level
//LEDxPER[1:0] Repetition Rate Units
//   00                      1/256 s
//   01                         1/8 s
//   10                         1 s
//   11                         2 s
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicLEDIndicatorSetBlinkPeriod(LED_CHANNEL channel, unsigned char bp)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check current level parameter
    if (bp > MC13892_LED_MAX_LED_BLINK_PERIOD)
    {
        ERRORMSG(1, (_T("PmicLEDIndicatorSetBlinkPeriod Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    
    switch (channel)
    {
        case TCLED_RED:
            param.addr = MC13892_LED_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL2_LEDRPER, bp);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL2_LEDRPER);
            break;

        case TCLED_GREEN:
            param.addr = MC13892_LED_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL2_LEDGPER, bp);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL2_LEDGPER);
            break;

        case TCLED_BLUE:
            param.addr = MC13892_LED_CTL3_ADDR;
            param.data = CSP_BITFVAL(MC13892_LED_CTL3_LEDBPER, bp);
            param.mask = CSP_BITFMASK(MC13892_LED_CTL3_LEDBPER);
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
//  Function: PmicLEDIndicatorGetBlinkPeriod
//
//  This function returns the Blink Period for led channel.
//
//  Parameters:
//      channel[IN]    led channel
//      level[OUT]      pointer to Blink Period
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicLEDIndicatorGetBlinkPeriod(LED_CHANNEL channel,  unsigned char* bp)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    UINT32 temp = 0;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = (channel==TCLED_BLUE)?MC13892_LED_CTL3_ADDR:MC13892_LED_CTL2_ADDR;
    param.data = 0;
    param.mask = 0;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    switch (channel)
    {
        case TCLED_RED:
            *bp  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL2_LEDRPER);
            break;

        case TCLED_GREEN:
            *bp  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL2_LEDGPER);
            break;

        case TCLED_BLUE:
            *bp  = (UINT8)CSP_BITFEXT(temp, MC13892_LED_CTL3_LEDBPER);
            break;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicLEDIndicatorEnableSWBST
//
// This function sets the operation enable  SWBST mode
//
// Parameters:
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicLEDIndicatorEnableSWBST()
{    
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_LED_CTL3_ADDR;
    param.data = CSP_BITFVAL(MC13892_LED_CTL3_LEDSWBSTEN, MC13892_LED_CTL3_LEDSWBSTEN_ENABLE);
    param.mask = CSP_BITFMASK(MC13892_LED_CTL3_LEDSWBSTEN) ;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
      sizeof(param), NULL, 0, NULL, NULL))
    return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function: PmicLEDIndicatorDisableSWBST
//
//This function sets the operation Disable  SWBST mode
//
//  Parameters:
//      channel[IN]    backlight channel
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicLEDIndicatorDisableSWBST()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_LED_CTL3_ADDR;
    param.data = CSP_BITFVAL(MC13892_LED_CTL3_LEDSWBSTEN, MC13892_LED_CTL3_LEDSWBSTEN_ENABLE);
    param.mask = CSP_BITFMASK(MC13892_LED_CTL3_LEDSWBSTEN) ;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
      sizeof(param), NULL, 0, NULL, NULL))
    return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


