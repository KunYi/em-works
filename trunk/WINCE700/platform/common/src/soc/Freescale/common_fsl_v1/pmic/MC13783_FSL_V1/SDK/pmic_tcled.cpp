//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2005, Motorola Inc. All Rights Reserved
//
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------
* Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
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

#include "socarm_macros.h"
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
// Function: PmicTCLEDEnable
//
// This function enables the Tri-Colour LED operation mode.
//
// Parameters:
//      mode [in] : operational mode for Tri-Color LED
//      bank[in]  : Selected tri-color bank
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDEnable(TCLED_MODE mode, FUNLIGHT_BANK bank)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;

    switch(mode)
    {
        case TCLED_FUN_MODE:
            switch(bank)
            {
                case TCLED_FUN_BANK1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK1,
                                             MC13783_LED_CTL0_FLBANK_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK1);
                    break;

                case TCLED_FUN_BANK2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK2,
                                             MC13783_LED_CTL0_FLBANK_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK2);
                    break;

                case TCLED_FUN_BANK3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK3,
                                             MC13783_LED_CTL0_FLBANK_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK3);
                    break;
            }
            break;

        case TCLED_IND_MODE:
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK1) |
                         CSP_BITFMASK(MC13783_LED_CTL0_FLBANK2) |
                         CSP_BITFMASK(MC13783_LED_CTL0_FLBANK3);
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK1,
                                     MC13783_LED_CTL0_FLBANK_DISABLE) |
                         CSP_BITFVAL(MC13783_LED_CTL0_FLBANK2,
                                     MC13783_LED_CTL0_FLBANK_DISABLE) |
                         CSP_BITFVAL(MC13783_LED_CTL0_FLBANK3,
                                     MC13783_LED_CTL0_FLBANK_DISABLE);
            break;

    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                         sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return PMIC_SUCCESS;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDDisable
//
// This function disables the Tri-Color LED operation mode.
//
// Parameters:
//      bank[in]  : Selected tri-color bank
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDDisable(FUNLIGHT_BANK bank)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;

    switch(bank)
    {
          case TCLED_FUN_BANK1:
               param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK1,
                                        MC13783_LED_CTL0_FLBANK_DISABLE);
               param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK1);
          break;

          case TCLED_FUN_BANK2:
               param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK2,
                                        MC13783_LED_CTL0_FLBANK_DISABLE);
               param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK2);
          break;

          case TCLED_FUN_BANK3:
               param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK3,
                                        MC13783_LED_CTL0_FLBANK_DISABLE);
               param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK3);
          break;
    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                         sizeof(param), NULL, 0, NULL, NULL))
    {
        return PMIC_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDGetMode
//
// This function retrieves the Tri-Color LED operation mode.
//
// Parameters:
//      mode [out] : pointer to operational mode for Tri-Color LED
//      bank[in]  : Selected tri-color bank
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDGetMode(TCLED_MODE *mode, FUNLIGHT_BANK bank)
{
    PMIC_PARAM_LLA_WRITE_REG param = {0};
    UINT32 temp;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
                        &temp, sizeof(temp), NULL, NULL))
    {
        return PMIC_ERROR;
    }

    switch(bank)
    {
          case TCLED_FUN_BANK1:
               temp = CSP_BITFEXT(temp, MC13783_LED_CTL0_FLBANK1);
          break;

          case TCLED_FUN_BANK2:
               temp = CSP_BITFEXT(temp, MC13783_LED_CTL0_FLBANK3);
          break;

          case TCLED_FUN_BANK3:
               temp = CSP_BITFEXT(temp, MC13783_LED_CTL0_FLBANK3);
          break;
    }

    if (temp == MC13783_LED_CTL0_FLBANK_ENABLE)
    {
         *mode = TCLED_FUN_MODE;
    }
    else
    {
         *mode = TCLED_IND_MODE;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightSetCurrentLevel
//
// This function sets the current level when Tri-Color LED is operating in
// fun light mode.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] channel.
//      level
//           [in] current level.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightSetCurrentLevel(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel, TCLED_CUR_LEVEL level)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (level > MC13783_MAX_FUNLIGHT_CURRENT_LEVEL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicTCLEDIndicatorSetCurrentLevel: ")
                              _T("Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL3_LEDR1, level);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL3_LEDR1);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL3_LEDG1, level);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL3_LEDG1);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL3_LEDB1, level);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL3_LEDB1);
                    break;
            }
            param.addr = MC13783_LED_CTL3_ADDR;
            break;

        case TCLED_FUN_BANK2:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL4_LEDR2, level);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL4_LEDR2);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL4_LEDG2, level);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL4_LEDG2);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL4_LEDB2, level);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL4_LEDB2);
                    break;
            }
            param.addr = MC13783_LED_CTL4_ADDR;
            break;

        case TCLED_FUN_BANK3:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL5_LEDR3, level);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL5_LEDR3);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL5_LEDG3, level);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL5_LEDG3);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL5_LEDB3, level);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL5_LEDB3);
                    break;
            }
            param.addr = MC13783_LED_CTL5_ADDR;
            break;

    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightGetCurrentLevel
//
// This function retrieves the current level when Tri-Color LED is operating
// in fun light mode.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] channel.
//      level
//           [out] variable to receive current level.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightGetCurrentLevel(FUNLIGHT_BANK bank,
                                             FUNLIGHT_CHANNEL channel,
                                             TCLED_CUR_LEVEL* level)
{
    UINT32 param;
    UINT32 temp;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            param = MC13783_LED_CTL3_ADDR;
            break;

        case TCLED_FUN_BANK2:
            param = MC13783_LED_CTL4_ADDR;
            break;

        case TCLED_FUN_BANK3:
            param = MC13783_LED_CTL5_ADDR;
            break;
    }

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
                        &temp, sizeof(temp), NULL, NULL))
    {
        return PMIC_ERROR;
    }

    switch(channel)
    {
        case TCLED_FUN_CHANNEL1:
            *level = (TCLED_CUR_LEVEL)CSP_BITFEXT(temp, MC13783_LED_CTL3_LEDR1);
            break;

       case TCLED_FUN_CHANNEL2:
            *level = (TCLED_CUR_LEVEL)CSP_BITFEXT(temp, MC13783_LED_CTL3_LEDG1);
            break;

        case TCLED_FUN_CHANNEL3:
            *level = (TCLED_CUR_LEVEL)CSP_BITFEXT(temp, MC13783_LED_CTL3_LEDB1);
            break;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightSetCycleTime
//
// This function sets the cycle time when Tri-Color LED is operating in fun
// light mode.
//
// Parameters:
//      bank
//           [in] bank.
//      ct
//           [in] cycle time.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightSetCycleTime(FUNLIGHT_BANK bank,
                                          TCLED_FUN_CYCLE_TIME ct)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    if (ct > MC13783_MAX_FUNLIGHT_CYCLE_TIME)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicTCLEDFunLightSetCycleTime:Invalid ")
                              _T("Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            param.data = CSP_BITFVAL(MC13783_LED_CTL3_TC1PERIOD, ct);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL3_TC1PERIOD);
            param.addr = MC13783_LED_CTL3_ADDR;
            break;

        case TCLED_FUN_BANK2:
            param.data = CSP_BITFVAL(MC13783_LED_CTL4_TC2PERIOD, ct);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL4_TC2PERIOD);
            param.addr = MC13783_LED_CTL4_ADDR;
            break;

        case TCLED_FUN_BANK3:
            param.data = CSP_BITFVAL(MC13783_LED_CTL5_TC3PERIOD, ct);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL5_TC3PERIOD);
            param.addr = MC13783_LED_CTL5_ADDR;
            break;

    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightGetCycleTime
//
// This function retrieves the cycle time when Tri-Color LED is operating in
// fun light mode.
//
// Parameters:
//      bank
//           [in] bank.
//      ct
//           [out] variable to receive cycle time.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightGetCycleTime(FUNLIGHT_BANK bank,
                                          TCLED_FUN_CYCLE_TIME* ct)
{
    UINT32 param;
    UINT32 temp;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            param = MC13783_LED_CTL3_ADDR;
            break;

        case TCLED_FUN_BANK2:
            param = MC13783_LED_CTL4_ADDR;
            break;

        case TCLED_FUN_BANK3:
            param = MC13783_LED_CTL5_ADDR;
            break;
    }

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
    {
        return PMIC_ERROR;
    }

    *ct = (TCLED_FUN_CYCLE_TIME)CSP_BITFEXT(temp, MC13783_LED_CTL3_TC1PERIOD);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightSetDutyCycle
//
// This function sets the duty cycle when Tri-Color LED is operating in fun
// light mode. The valid duty cycle settings are integers from 0 to 31.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] channel.
//      dc
//           [in] duty cycle.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightSetDutyCycle(FUNLIGHT_BANK bank,
                                          FUNLIGHT_CHANNEL channel,
                                          unsigned char dc)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (dc > MC13783_MAX_FUNLIGHT_DUTY_CYCLE)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicTCLEDFunLightSetDutyCycle:Invalid ")
                              _T("Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL3_LEDR1DC, dc);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL3_LEDR1DC);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL3_LEDG1DC, dc);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL3_LEDG1DC);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL3_LEDB1DC, dc);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL3_LEDB1DC);
                    break;
            }
            param.addr = MC13783_LED_CTL3_ADDR;
            break;

        case TCLED_FUN_BANK2:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL4_LEDR2DC, dc);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL4_LEDR2DC);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL4_LEDG2DC, dc);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL4_LEDG2DC);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL4_LEDB2DC, dc);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL4_LEDB2DC);
                    break;
            }
            param.addr = MC13783_LED_CTL4_ADDR;
            break;

        case TCLED_FUN_BANK3:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL5_LEDR3DC, dc);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL5_LEDR3DC);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL5_LEDG3DC, dc);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL5_LEDG3DC);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL5_LEDB3DC, dc);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL5_LEDB3DC);
                    break;
            }
            param.addr = MC13783_LED_CTL5_ADDR;
            break;

    }
    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightGetDutyCycle
//
// This function retrieves the duty cycle when Tri-Color LED is operating in
// fun light mode.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] channel.
//      dc
//           [out] variable to receive duty cycle.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightGetDutyCycle(FUNLIGHT_BANK bank,
                                          FUNLIGHT_CHANNEL channel,
                                          unsigned char* dc)
{
    UINT32 param;
    UINT32 temp;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            param = MC13783_LED_CTL3_ADDR;
            break;

        case TCLED_FUN_BANK2:
            param = MC13783_LED_CTL4_ADDR;
            break;

        case TCLED_FUN_BANK3:
            param = MC13783_LED_CTL5_ADDR;
            break;
    }

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
                        &temp, sizeof(temp), NULL, NULL))
    {
        return PMIC_ERROR;
    }

    switch(channel)
    {
        case TCLED_FUN_CHANNEL1:
            *dc = (UCHAR)CSP_BITFEXT(temp, MC13783_LED_CTL3_LEDR1DC);
            break;

       case TCLED_FUN_CHANNEL2:
            *dc = (UCHAR)CSP_BITFEXT(temp, MC13783_LED_CTL3_LEDG1DC);
            break;

        case TCLED_FUN_CHANNEL3:
            *dc = (UCHAR)CSP_BITFEXT(temp, MC13783_LED_CTL3_LEDB1DC);
            break;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightBlendedRamps
//
// This function sets the fun light in Blended Ramps pattern.
// The calling program has to disable the bank before changing to a new
// pattern, if the bank is enabled.
//
//
// Parameters:
//      bank
//           [in] bank.
//      speed
//           [in] speed.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightBlendedRamps(FUNLIGHT_BANK bank,
                                          TCLED_FUN_SPEED speed)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;

    switch(speed)
    {
        case TC_OFF:
            switch(bank)
            {
                case TCLED_FUN_BANK1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK1,
                                             MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK1);
                    break;

                case TCLED_FUN_BANK2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK2,
                                             MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK2);
                    break;

                case TCLED_FUN_BANK3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK3,
                                             MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK3);
                    break;
            }

            goto callioctl;
            break;

         case TC_SLOW:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLPATTERN,
                                     MC13783_BLENDED_RAMPS_SLOW);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLPATTERN);
            break;

         case TC_FAST:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLPATTERN,
                                     MC13783_BLENDED_RAMPS_FAST);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLPATTERN);
            break;
    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                         sizeof(param), NULL, 0, NULL, NULL))
    {
        return PMIC_ERROR;
    }

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK1,
                                     MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK1);
            break;

        case TCLED_FUN_BANK2:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK2,
                                     MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK2);
            break;

        case TCLED_FUN_BANK3:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK3,
                                     MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK3);
            break;
    }

    callioctl:
    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                         sizeof(param), NULL, 0, NULL, NULL))
    {
        return PMIC_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightSawRamps
//
// This function sets the fun light in Saw Ramps pattern.
// The calling program has to disable the bank before changing to a new
// pattern, if the bank is enabled.
//
// Parameters:
//      bank
//           [in] bank.
//      speed
//           [in] speed.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightSawRamps(FUNLIGHT_BANK bank, TCLED_FUN_SPEED speed)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;

    switch(speed)
    {
        case TC_OFF:
            switch(bank)
            {
                case TCLED_FUN_BANK1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK1,
                                             MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK1);
                    break;

                case TCLED_FUN_BANK2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK2,
                                             MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK2);
                    break;

                case TCLED_FUN_BANK3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK3,
                                             MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK3);
                    break;
            }

            goto callioctl;
            break;

         case TC_SLOW:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLPATTERN,
                                     MC13783_SAW_RAMPS_SLOW);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLPATTERN);
            break;

         case TC_FAST:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLPATTERN,
                                     MC13783_SAW_RAMPS_FAST);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLPATTERN);
            break;
    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                         sizeof(param), NULL, 0, NULL, NULL))
    {
        return PMIC_ERROR;
    }

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK1,
                                     MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK1);
            break;

        case TCLED_FUN_BANK2:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK2,
                                     MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK2);
            break;

        case TCLED_FUN_BANK3:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK3,
                                     MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK3);
            break;
    }

    callioctl:
    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                         sizeof(param), NULL, 0, NULL, NULL))
    {
        return PMIC_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightBlendedBowtie
//
// This function sets the fun light in Blended Bowtie pattern.
// The calling program has to disable the bank before changing to a new
// pattern, if the bank is enabled.
//
// Parameters:
//      bank
//           [in] bank.
//      speed
//           [in] speed.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightBlendedBowtie(FUNLIGHT_BANK bank,
                                           TCLED_FUN_SPEED speed)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;

    switch(speed)
    {
        case TC_OFF:
            switch(bank)
            {
                case TCLED_FUN_BANK1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK1,
                                             MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK1);
                    break;

                case TCLED_FUN_BANK2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK2,
                                             MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK2);
                    break;

                case TCLED_FUN_BANK3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK3,
                                             MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK3);
                    break;
            }

            goto callioctl;
            break;

         case TC_SLOW:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLPATTERN,
                                     MC13783_BLENDED_BOWTIE_RAMPS_SLOW);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLPATTERN);
            break;

         case TC_FAST:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLPATTERN,
                                     MC13783_BLENDED_BOWTIE_RAMPS_FAST);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLPATTERN);
            break;
    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                         sizeof(param), NULL, 0, NULL, NULL))
    {
        return PMIC_ERROR;
    }

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK1,
                                     MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK1);
            break;

        case TCLED_FUN_BANK2:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK2,
                                     MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK2);
            break;

        case TCLED_FUN_BANK3:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK3,
                                     MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK3);
            break;
    }

    callioctl:
    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                         sizeof(param), NULL, 0, NULL, NULL))
    {
        return PMIC_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightChasingLightsPattern
//
// This function sets the fun light in Chasing Lights RGB pattern.
//
// Parameters:
//      bank
//           [in] bank.
//      pattern
//           [in] pattern.
//      speed
//           [in] speed.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightChasingLightsPattern(FUNLIGHT_BANK bank, CHASELIGHT_PATTERN pattern, TCLED_FUN_SPEED speed)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL0_ADDR;

    switch(speed)
    {
        case TC_OFF:
            switch(bank)
            {
                case TCLED_FUN_BANK1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK1, MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK1);
                    break;

                case TCLED_FUN_BANK2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK2, MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK2);
                    break;

                case TCLED_FUN_BANK3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK3, MC13783_LED_CTL0_FLBANK_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK3);
                    break;

            }
            goto callioctl;
            break;

         case TC_SLOW:
            switch(pattern)
            {
                case RGB:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLPATTERN, MC13783_CHASING_LIGHTS_RGB_SLOW);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLPATTERN);
                    break;
                case BGR:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLPATTERN, MC13783_CHASING_LIGHTS_BGR_SLOW);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLPATTERN);
                    break;
            }
            break;

         case TC_FAST:
            switch(pattern)
            {
                case RGB:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLPATTERN, MC13783_CHASING_LIGHTS_RGB_FAST);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLPATTERN);
                    break;
                case BGR:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLPATTERN, MC13783_CHASING_LIGHTS_BGR_FAST);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLPATTERN);
                    break;
            }
            break;
    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK1, MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK1);
            break;

        case TCLED_FUN_BANK2:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK2, MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK2);
            break;

        case TCLED_FUN_BANK3:
            param.data = CSP_BITFVAL(MC13783_LED_CTL0_FLBANK3, MC13783_LED_CTL0_FLBANK_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL0_FLBANK3);
            break;
    }

    callioctl:
    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;


    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightRampUp
//
// This function starts LEDs Brightness Ramp Up function. Ramp time is fixed at 1 second.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] the channel to ramp up.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightRampUp(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL1_ADDR;

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDR1RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_ENABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDR1RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDR1RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDR1RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDG1RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_ENABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDG1RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDG1RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDG1RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDB1RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_ENABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDB1RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDB1RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDB1RAMPDOWN);
                    break;

            }
            break;

        case TCLED_FUN_BANK2:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDR2RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_ENABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDR2RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDR2RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDR2RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDG2RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_ENABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDG2RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDG2RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDG2RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDB2RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_ENABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDB2RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDB2RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDB2RAMPDOWN);
                    break;

            }
            break;

        case TCLED_FUN_BANK3:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDR3RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_ENABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDR3RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDR3RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDR3RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDG3RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_ENABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDG3RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDG3RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDG3RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDB3RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_ENABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDB3RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDB3RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDB3RAMPDOWN);
                    break;

            }
            break;

    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;

}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightRampDown
//
// This function starts LEDs Brightness Ramp Down function. Ramp time is fixed at 1 second.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] the channel to ramp down.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightRampDown(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL1_ADDR;

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDR1RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDR1RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDR1RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDR1RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDG1RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDG1RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDG1RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDG1RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDB1RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDB1RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDB1RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDB1RAMPDOWN);
                    break;

            }
            break;

        case TCLED_FUN_BANK2:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDR2RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDR2RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDR2RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDR2RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDG2RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDG2RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDG2RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDG2RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDB2RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDB2RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDB2RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDB2RAMPDOWN);
                    break;

            }
            break;

        case TCLED_FUN_BANK3:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDR3RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDR3RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDR3RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDR3RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDG3RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDG3RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDG3RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDG3RAMPDOWN);
                    break;

                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDB3RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDB3RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_ENABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDB3RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDB3RAMPDOWN);
                    break;

            }
            break;

    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;

}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightRampOff
//
// This function stop LEDs Brightness Ramp function.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] the channel to stop ramp
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightRampOff(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_LED_CTL1_ADDR;

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDR1RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDR1RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDR1RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDR1RAMPDOWN);
                    break;
        
                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDG1RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDG1RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDG1RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDG1RAMPDOWN);
                    break;
            
                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDB1RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDB1RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDB1RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDB1RAMPDOWN);
                    break;
            
            }
            break;

        case TCLED_FUN_BANK2:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDR2RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDR2RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDR2RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDR2RAMPDOWN);
                    break;
        
                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDG2RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDG2RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDG2RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDG2RAMPDOWN);
                    break;
            
                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDB2RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDB2RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDB2RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDB2RAMPDOWN);
                    break;
            
            }
            break;

        case TCLED_FUN_BANK3:
            switch(channel)
            {
                case TCLED_FUN_CHANNEL1:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDR3RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDR3RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDR3RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDR3RAMPDOWN);
                    break;
        
                case TCLED_FUN_CHANNEL2:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDG3RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDG3RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDG3RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDG3RAMPDOWN);
                    break;
            
                case TCLED_FUN_CHANNEL3:
                    param.data = CSP_BITFVAL(MC13783_LED_CTL1_LEDB3RAMPUP, MC13783_LED_CTL1_LEDRAMPUP_DISABLE)|
                                 CSP_BITFVAL(MC13783_LED_CTL1_LEDB3RAMPDOWN, MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE);
                    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_LEDB3RAMPUP)|
                                 CSP_BITFMASK(MC13783_LED_CTL1_LEDB3RAMPDOWN);
                    break;
            
            }
            break;

    }
    
    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
   
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightTriodeOn
//
// This function enable triode mode of the channel.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] the channel to enable triode mode.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightTriodeOn(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel)
{
    UNREFERENCED_PARAMETER(channel);

    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            param.data = CSP_BITFVAL(MC13783_LED_CTL3_TC1TRIODE, MC13783_LED_CTL3_TC1TRIODE_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL3_TC1TRIODE);
            param.addr = MC13783_LED_CTL3_ADDR;
            break;

        case TCLED_FUN_BANK2:
            param.data = CSP_BITFVAL(MC13783_LED_CTL4_TC2TRIODE, MC13783_LED_CTL4_TC2TRIODE_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL4_TC2TRIODE);
            param.addr = MC13783_LED_CTL4_ADDR;
            break;

        case TCLED_FUN_BANK3:
            param.data = CSP_BITFVAL(MC13783_LED_CTL5_TC3TRIODE, MC13783_LED_CTL5_TC3TRIODE_ENABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL5_TC3TRIODE);
            param.addr = MC13783_LED_CTL5_ADDR;
            break;

    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightTriodeOff
//
// This function disable triode mode of the channel.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] the channel to disable triode mode.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightTriodeOff(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel)
{
    UNREFERENCED_PARAMETER(channel);

    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch(bank)
    {
        case TCLED_FUN_BANK1:
            param.data = CSP_BITFVAL(MC13783_LED_CTL3_TC1TRIODE, MC13783_LED_CTL3_TC1TRIODE_DISABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL3_TC1TRIODE);
            param.addr = MC13783_LED_CTL3_ADDR;
            break;

        case TCLED_FUN_BANK2:
            param.data = CSP_BITFVAL(MC13783_LED_CTL4_TC2TRIODE, MC13783_LED_CTL4_TC2TRIODE_DISABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL4_TC2TRIODE);
            param.addr = MC13783_LED_CTL4_ADDR;
            break;

        case TCLED_FUN_BANK3:
            param.data = CSP_BITFVAL(MC13783_LED_CTL5_TC3TRIODE, MC13783_LED_CTL5_TC3TRIODE_DISABLE);
            param.mask = CSP_BITFMASK(MC13783_LED_CTL5_TC3TRIODE);
            param.addr = MC13783_LED_CTL5_ADDR;
            break;

    }

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDEnableEdgeSlow
//
// This function enable Analog Edge Slowing.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDEnableEdgeSlow(void)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.data = CSP_BITFVAL(MC13783_LED_CTL1_SLEWLIMTC, MC13783_LED_CTL1_SLEWLIMTC_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_SLEWLIMTC);
    param.addr = MC13783_LED_CTL1_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDDisableEdgeSlow
//
// This function disable Analog Edge Slowing.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDDisableEdgeSlow(void)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.data = CSP_BITFVAL(MC13783_LED_CTL1_SLEWLIMTC, MC13783_LED_CTL1_SLEWLIMTC_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_SLEWLIMTC);
    param.addr = MC13783_LED_CTL1_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDEnableHalfCurrent
//
// This function enable Half Current Mode for Tri-color 1 Driver channel.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDEnableHalfCurrent(void)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.data = CSP_BITFVAL(MC13783_LED_CTL1_TC1HALF, MC13783_LED_CTL1_T1HALF_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_TC1HALF);
    param.addr = MC13783_LED_CTL1_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDDisableHalfCurrent
//
// This function disable Half Current Mode for Tri-color 1 Driver channel.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDDisableHalfCurrent(void)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.data = CSP_BITFVAL(MC13783_LED_CTL1_TC1HALF, MC13783_LED_CTL1_T1HALF_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_LED_CTL1_TC1HALF);
    param.addr = MC13783_LED_CTL1_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
    return PMIC_SUCCESS;
}


PMIC_STATUS PmicTCLEDIndicatorSetCurrentLevel(IND_CHANNEL channel, TCLED_CUR_LEVEL level)
{
    UNREFERENCED_PARAMETER(level);
    UNREFERENCED_PARAMETER(channel);
    return PMIC_SUCCESS;
};

PMIC_STATUS PmicTCLEDIndicatorGetCurrentLevel(IND_CHANNEL channel, TCLED_CUR_LEVEL* level)
{
    UNREFERENCED_PARAMETER(level);
    UNREFERENCED_PARAMETER(channel);

    return PMIC_SUCCESS;
};

PMIC_STATUS PmicTCLEDIndicatorSetBlinkPattern(IND_CHANNEL channel, TCLED_IND_BLINK_MODE mode, BOOL skip)
{
    UNREFERENCED_PARAMETER(mode);
    UNREFERENCED_PARAMETER(channel);
    UNREFERENCED_PARAMETER(skip);

    return PMIC_SUCCESS;
};

PMIC_STATUS PmicTCLEDIndicatorGetBlinkPattern(IND_CHANNEL channel, TCLED_IND_BLINK_MODE* mode, BOOL* skip)
{
    UNREFERENCED_PARAMETER(mode);
    UNREFERENCED_PARAMETER(channel);
    UNREFERENCED_PARAMETER(skip);

    return PMIC_SUCCESS;
};

PMIC_STATUS PmicTCLEDFunLightStrobe(FUNLIGHT_CHANNEL channel, TCLED_FUN_STROBE_SPEED speed)
{
    UNREFERENCED_PARAMETER(speed);
    UNREFERENCED_PARAMETER(channel);

    return PMIC_NOT_SUPPORTED;
};

