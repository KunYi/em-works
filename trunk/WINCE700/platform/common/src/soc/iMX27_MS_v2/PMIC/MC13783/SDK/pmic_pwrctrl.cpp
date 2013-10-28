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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2005, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pmic_pwrctrl.cpp
//
//  This file contains the PMIC power control and power cut SDK interface that is used by applications
//  and other drivers to access registers of the PMIC.
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include "csparm_macros.h"
#include "pmic_ioctl.h"
#include "regs.h"
#include "regs_pwrctrl.h"
#include "pmic_pwrctrl.h"

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
#define ZONEID_ERROR            0
#define ZONEID_WARN             1
#define ZONEID_INIT             2
#define ZONEID_FUNC             3
#define ZONEID_INFO             4

// Debug zone masks
#define ZONEMASK_ERROR      (1 << ZONEID_ERROR)
#define ZONEMASK_WARN           (1 << ZONEID_WARN)
#define ZONEMASK_INIT           (1 << ZONEID_INIT)
#define ZONEMASK_FUNC       (1 << ZONEID_FUNC)
#define ZONEMASK_INFO           (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR              DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN               DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT               DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC               DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO               DEBUGZONE(ZONEID_INFO)

extern DBGPARAM dpCurSettings;

#endif  // DEBUG

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetPowerCutTimer
//
// This function is used to set the power cut timer duration. 
//
// Parameters:
//             duration [in]             The value to set to power cut timer register, it's from 0 to 255.
//                                            The timer will be set to a duration of 0 to 31.875 seconds,
//                                            in 125 ms increments.
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetPowerCutTimer (UINT8 duration)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL1_ADDR;

    // check input parameter
    if (duration > MC13783_PWRCTRL1_PCT_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetPowerCutTimer:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
    param.data = CSP_BITFVAL(MC13783_PWRCTRL1_PCT, duration);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL1_PCT);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetPowerCutTimer
//
// This function is used to get the power cut timer duration. 
//
// Parameters:
//             duration [out]             the duration to set to power cut timer
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetPowerCutTimer (UINT8* duration)
{
    UINT32 addr, temp;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (duration == NULL){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlGetPowerCutTimer:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    
    addr = MC13783_PWR_CTL1_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *duration = CSP_BITFEXT(temp, MC13783_PWRCTRL1_PCT);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnablePowerCut
//
// This function is used to enable the power cut. 
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnablePowerCut (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWRCTRL0_PCEN, MC13783_PWRCTRL0_PCEN_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_PCEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisablePowerCut
//
// This function is used to disable the power cut. 
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisablePowerCut (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWRCTRL0_PCEN, MC13783_PWRCTRL0_PCEN_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_PCEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetPowerCutCounter
//
// This function is used to set the power cut counter. 
//
// Parameters:
//             counter [in]            The counter number value to be set to the register. It's value 
//                                          from 0 to 15. 
//
//                                          The power cut counter is a 4 bit counter that keeps track of 
//                                          the number of rising edges of the UV_TIMER (power cut 
//                                          events) that have occurred since the counter was last initialized.
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetPowerCutCounter (UINT8 counter)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL1_ADDR;

    // check input parameter
    if (counter > MC13783_PWRCTRL1_PCCOUNT_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetPowerCutCounter:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
    param.data = CSP_BITFVAL(MC13783_PWRCTRL1_PCCOUNT, counter);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL1_PCCOUNT);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetPowerCutCounter
//
// This function is used to get the power cut counter. 
//
// Parameters:
//             counter [out]            to get the counter number
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetPowerCutCounter (UINT8* counter)
{
    UINT32 addr, temp;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (counter == NULL){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlGetPowerCutCounter:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    
    addr = MC13783_PWR_CTL1_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *counter = CSP_BITFEXT(temp, MC13783_PWRCTRL1_PCCOUNT);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetPowerCutMaxCounter
//
// This function is used to set the maxium number of power cut counter. 
//
// Parameters:
//             counter [in]            maxium counter number to set. It's value from 0 to 15. 
// 
//                                          The power cut register provides a method for disabling 
//                                          power cuts if this situation manifests itself. 
//                                          If PC_COUNT >= PC_MAX_COUNT, then the number of 
//                                          resets that have occurred since the power cut counter was 
//                                          last initialized exceeds the established limit, and power cuts 
//                                          will be disabled.
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetPowerCutMaxCounter (UINT8 counter)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL1_ADDR;            

    // check input parameter
    if (counter > MC13783_PWRCTRL_PCMAXCNT_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetPowerCutMaxCounter:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
    param.data = CSP_BITFVAL(MC13783_PWRCTRL1_PCMAXCNT, counter);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL1_PCMAXCNT);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlgetPowerCutMaxCounter
//
// This function is used to get the setting of maxium power cut counter. 
//
// Parameters:
//             counter [out]            to get the maxium counter number
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetPowerCutMaxCounter (UINT8* counter)
{
    UINT32 addr, temp;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (counter == NULL){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlGetPowerCutMaxCounter:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    
    addr = MC13783_PWR_CTL1_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *counter = CSP_BITFEXT(temp, MC13783_PWRCTRL1_PCMAXCNT);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableCounter
//
// The power cut register provides a method for disabling power cuts if this situation 
// manifests itself. If PC_COUNT >= PC_MAX_COUNT, then the number of resets that 
// have occurred since the power cut counter was last initialized exceeds the established 
// limit, and power cuts will be disabled.
// This function can be disabled by setting PC_COUNT_EN=0. In this case, each power cut 
// event will increment the power cut counter, but power cut coverage will not be disabled, 
// even if PC_COUNT exceeds PC_MAX_COUNT.
//
// This PmicPwrctrlEnableCounter function will set PC_COUNT_EN=1.
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableCounter(void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWRCTRL0_PCCOUNTEN, MC13783_PWRCTRL0_PCCOUNTEN_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_PCCOUNTEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableCounter
//
// The power cut register provides a method for disabling power cuts if this situation 
// manifests itself. If PC_COUNT >= PC_MAX_COUNT, then the number of resets that 
// have occurred since the power cut counter was last initialized exceeds the established 
// limit, and power cuts will be disabled.
// This function can be disabled by setting PC_COUNT_EN=0. In this case, each power cut 
// event will increment the power cut counter, but power cut coverage will not be disabled, 
// even if PC_COUNT exceeds PC_MAX_COUNT.
//
// This PmicPwrctrlEnableCounter function will set PC_COUNT_EN=0.
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableCounter (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWRCTRL0_PCCOUNTEN, MC13783_PWRCTRL0_PCCOUNTEN_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_PCCOUNTEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetMemHoldTimer
//
// This function is used to set the duration of memory hold timer. 
//
// Parameters:
//             duration [in]             The value to set to memory hold timer register. It's from 
//                                            0 to 15.
//                                            The resolution of the memory hold timer is 32 seconds 
//                                            for a maximum duration of 512 seconds.
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetMemHoldTimer (UINT8 duration)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL1_ADDR;           
    
    // check input parameter
    if (duration > MC13783_PWRCTRL1_MEMTMR_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetMemHoldTimer:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
    param.data = CSP_BITFVAL(MC13783_PWRCTRL1_MEMTMR, duration);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL1_MEMTMR);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetMemHoldTimer
//
// This function is used to get the setting of memory hold timer 
//
// Parameters:
//             duration [out]            to get the duration of the timer
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetMemHoldTimer (UINT8* duration)
{
    UINT32 addr, temp;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (duration == NULL){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlGetMemHoldTimer:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    
    addr = MC13783_PWR_CTL1_ADDR;          

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *duration = CSP_BITFEXT(temp, MC13783_PWRCTRL1_MEMTMR);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetMemHoldTimerAllOn
//
// This function is used to set the duration of the memory hold timer to infinity 
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetMemHoldTimerAllOn (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL1_ADDR;            
    param.data = CSP_BITFVAL(MC13783_PWRCTRL1_MEMALLON, MC13783_PWRCTRL1_MEMALLON_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL1_MEMALLON);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlClearMemHoldTimerAllOn
//
// This function is used to clear the infinity duration of the memory hold timer
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlClearMemHoldTimerAllOn (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL1_ADDR;            
    param.data = CSP_BITFVAL(MC13783_PWRCTRL1_MEMALLON, MC13783_PWRCTRL1_MEMALLON_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL1_MEMALLON);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableClk32kMCU
//
// This function is used to enable the CLK32KMCU
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableClk32kMCU (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWRCTRL0_CLK32KMCUEN, MC13783_PWRCTRL0_CLK32KMCUEN_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_CLK32KMCUEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableClk32kMCU
//
// This function is used to disable the CLK32KMCU
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableClk32kMCU (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWRCTRL0_CLK32KMCUEN, MC13783_PWRCTRL0_CLK32KMCUEN_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_CLK32KMCUEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableUserOffModeWhenDelay
//
// This function is used to place the phone in User Off Mode after a delay.
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableUserOffModeWhenDelay (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWRCTRL0_USEROFFSPI, MC13783_PWRCTRL0_USEROFFSPI_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_USEROFFSPI);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableUserOffModeWhenDelay
//
// This function is used to set not to place the phone in User Off Mode after a delay.
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableUserOffModeWhenDelay (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWRCTRL0_USEROFFSPI, MC13783_PWRCTRL0_USEROFFSPI_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_USEROFFSPI);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetVBKUPRegulator
//
// This function is used to set the VBKUP regulator
//
// Parameters:
//             reg [in]           the backup regulator to set
//             mode [in]        the mode to set to backup regulator
//
//                                   VBKUP_MODE1 - VBKUPxEN = 0, VBKUPxAUTO = 0
//                                   Backup Regulator Off in Non Power Cut Modes and Off in Power Cut Modes
//
//                                   VBKUP_MODE2 - VBKUPxEN = 0, VBKUPxAUTO = 1
//                                   Backup Regulator Off in Non Power Cut Modes and On in Power Cut Modes
//
//                                   VBKUP_MODE3 - VBKUPxEN = 1, VBKUPxAUTO = 0
//                                   Backup Regulator On in Non Power Cut Modes and Off in Power Cut Modes
//                                   
//                                   VBKUP_MODE4 - VBKUPxEN = 1, VBKUPxAUTO = 1
//                                   Backup Regulator On in Non Power Cut Modes and On in Power Cut Modes
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetVBKUPRegulator (MC13783_PWRCTRL_REG_VBKUP reg, MC13783_PWRCTRL_VBKUP_MODE mode)
{
     PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;

    if (reg == VBKUP1){
        switch (mode){
            case VBKUP_MODE1:
                param.data = CSP_BITFVAL(MC13783_PWRCTRL0_VBKUP1EN, VBKUP_MODE1);
                param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP1EN)
                                      | CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP1AUTO);
    
                if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                        sizeof(param), NULL, 0, NULL, NULL))
                     return PMIC_ERROR;

                break;

            case VBKUP_MODE2:
                param.data = CSP_BITFVAL(MC13783_PWRCTRL0_VBKUP1EN, VBKUP_MODE2);
                param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP1EN)
                                     | CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP1AUTO);
    
                if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                        sizeof(param), NULL, 0, NULL, NULL))
                     return PMIC_ERROR;

                break;

            case VBKUP_MODE3:
                param.data = CSP_BITFVAL(MC13783_PWRCTRL0_VBKUP1EN, VBKUP_MODE3);
                param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP1EN)
                                     | CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP1AUTO);
    
                if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                        sizeof(param), NULL, 0, NULL, NULL))
                     return PMIC_ERROR;

                break;

            case VBKUP_MODE4:
                param.data = CSP_BITFVAL(MC13783_PWRCTRL0_VBKUP1EN, VBKUP_MODE4);
                param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP1EN) 
                                     | CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP1AUTO);
    
                if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                        sizeof(param), NULL, 0, NULL, NULL))
                     return PMIC_ERROR;

                break;

            default:
                DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetVBKUPRegulator:Invalid Parameter\r\n")));
                return PMIC_PARAMETER_ERROR;
        }
    }
    else if (reg == VBKUP2){
        switch (mode){
            case VBKUP_MODE1:
                param.data = CSP_BITFVAL(MC13783_PWRCTRL0_VBKUP2EN, VBKUP_MODE1);
                param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP2EN)
                                     | CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP2AUTO);
    
                if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                        sizeof(param), NULL, 0, NULL, NULL))
                     return PMIC_ERROR;

                break;

            case VBKUP_MODE2:
                param.data = CSP_BITFVAL(MC13783_PWRCTRL0_VBKUP2EN, VBKUP_MODE2);
                param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP2EN)
                                     | CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP2AUTO);
    
                if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                        sizeof(param), NULL, 0, NULL, NULL))
                     return PMIC_ERROR;

                break;

            case VBKUP_MODE3:
                param.data = CSP_BITFVAL(MC13783_PWRCTRL0_VBKUP2EN, VBKUP_MODE3);
                param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP2EN)
                                     | CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP2AUTO);
    
                if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                        sizeof(param), NULL, 0, NULL, NULL))
                     return PMIC_ERROR;

                break;

            case VBKUP_MODE4:
                param.data = CSP_BITFVAL(MC13783_PWRCTRL0_VBKUP2EN, VBKUP_MODE4);
                param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP2EN)
                                     | CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP2AUTO);
    
                if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                        sizeof(param), NULL, 0, NULL, NULL))
                     return PMIC_ERROR;

                break;

            default:
                DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetVBKUPRegulator:Invalid Parameter\r\n")));
                return PMIC_PARAMETER_ERROR;
        }
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetVBKUPRegulatorVoltage
//
// This function is used to set the VBKUP regulator voltage
//
// Parameters:
//             reg [in]           the backup regulator to set
//             volt [in]        the voltage to set to backup regulator
//                                0 - 1.0v
//                                1 - 1.2v
//                                2 - 1.5v
//                                3 - 1.8v
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetVBKUPRegulatorVoltage (MC13783_PWRCTRL_REG_VBKUP reg, UINT8 volt)
{
     PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;

    // check input parameter
    if (volt > MC13783_PWRCTRL0_VBKUP_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetVBKUPRegulatorVoltage:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    

    if (reg == VBKUP1){
        param.data = CSP_BITFVAL(MC13783_PWRCTRL0_VBKUP1, volt);
        param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP1);
    }
    else if (reg == VBKUP2){
        param.data = CSP_BITFVAL(MC13783_PWRCTRL0_VBKUP2, volt);
        param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_VBKUP2);
    }   

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                sizeof(param), NULL, 0, NULL, NULL))
         return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableWarmStart
//
// This function is used to set the phone to transit from the ON state to the User Off state 
// when either the USER_OFF pin is pulled high or the USER_OFF_SPI bit is set (after an 8ms 
// delay in the Memwait state).
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableWarmStart (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWRCTRL0_WARMEN, MC13783_PWRCTRL0_WARMEN_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_WARMEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableWarmStart
//
// This function is used to disable the warm start and set the phone to transit from the 
// ON state to the MEMHOLD ONLY state when either the USER_OFF pin is pulled high or 
// the USER_OFF_SPI bit is set (after an 8ms delay in the Memwait state).
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableWarmStart (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWRCTRL0_WARMEN, MC13783_PWRCTRL0_WARMEN_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_PWRCTRL0_WARMEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableRegenAssig 
//
// This function enables the REGEN pin of slected voltage regulator. The
// REGEN function can be used in two ways. It can be used as a regulator enable pin like with SIMEN
// where the SPI programming is static and the REGEN pin is dynamic. It can also be used in a static
// fashion where REGEN is maintained high while the regulators get enabled and disabled dynamically
// via SPI. In that case REGEN functions as a master enable..
//
// Parameters:
//             t_regulator regu
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableRegenAssig (t_regulator regu)
{
    PMIC_PARAM_LLA_WRITE_REG param;
        unsigned int Enable=1;
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_REG_ASS_ADDR;
    param.data = ((Enable) << (REGULATOR_REGEN_BIT[regu]));
    param.mask = (((1U << (1)) - 1) << (REGULATOR_REGEN_BIT[regu]));

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}
//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableRegenAssig
//
// This function Disbale the REGEN pin of slected voltage regulator.
//
// Parameters:
//             t_regulator regu
// 
// Returns:
//          status 
// PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableRegenAssig (t_regulator regu)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    unsigned int Enable=0;
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_REG_ASS_ADDR;
    param.data = ((Enable) << (REGULATOR_REGEN_BIT[regu]));
    param.mask = (((1U << (1)) - 1) << (REGULATOR_REGEN_BIT[regu]));

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}
//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetRegenAssin
//
// This function reads the REGEN pin value for said voltage regulator.
//
// Parameters:
//             t_regulator regu , value
// 
// Returns:
//          status 
// PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetRegenAssig (t_regulator regu , UINT8* value)
{
    UINT32 param, temp;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    
    
    param = MC13783_REG_ASS_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *value  =((temp & (((1U << (1)) - 1) << (REGULATOR_REGEN_BIT[regu]))) >> (REGULATOR_REGEN_BIT[regu]));
        
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

// end of file
