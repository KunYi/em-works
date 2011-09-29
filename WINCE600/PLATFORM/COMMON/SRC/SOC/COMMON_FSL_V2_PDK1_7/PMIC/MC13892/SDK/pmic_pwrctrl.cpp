/*---------------------------------------------------------------------------
* Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//
//  File:  pmic_pwrctrl.cpp
//
//  This file contains the PMIC power control and power cut SDK interface that is used by applications
//  and other drivers to access registers of the PMIC.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
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
#define ZONEID_ERROR            0
#define ZONEID_WARN             1
#define ZONEID_INIT             2
#define ZONEID_FUNC             3
#define ZONEID_INFO             4

// Debug zone masks
#define ZONEMASK_ERROR          (1 << ZONEID_ERROR)
#define ZONEMASK_WARN           (1 << ZONEID_WARN)
#define ZONEMASK_INIT           (1 << ZONEID_INIT)
#define ZONEMASK_FUNC           (1 << ZONEID_FUNC)
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

    param.addr = MC13892_PWR_CTL1_ADDR;

    // check input parameter
    if (duration > MC13892_PWRCTRL1_PCT_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetPowerCutTimer:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
    param.data = CSP_BITFVAL(MC13892_PWRCTRL1_PCT, duration);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL1_PCT);

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
    UINT32 addr, temp = 0;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (duration == NULL){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlGetPowerCutTimer:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    
    addr = MC13892_PWR_CTL1_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *duration = (UINT8)CSP_BITFEXT(temp, MC13892_PWRCTRL1_PCT);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_PCEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_PCEN);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_PCEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_PCEN);

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

    param.addr = MC13892_PWR_CTL1_ADDR;

    // check input parameter
    if (counter > MC13892_PWRCTRL1_PCCOUNT_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetPowerCutCounter:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
    param.data = CSP_BITFVAL(MC13892_PWRCTRL1_PCCOUNT, counter);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL1_PCCOUNT);

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
    
    addr = MC13892_PWR_CTL1_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *counter = (UINT8)CSP_BITFEXT(temp, MC13892_PWRCTRL1_PCCOUNT);

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

    param.addr = MC13892_PWR_CTL1_ADDR;            

    // check input parameter
    if (counter > MC13892_PWRCTRL_PCMAXCNT_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetPowerCutMaxCounter:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
    param.data = CSP_BITFVAL(MC13892_PWRCTRL1_PCMAXCNT, counter);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL1_PCMAXCNT);

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
    
    addr = MC13892_PWR_CTL1_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *counter = (UINT8)CSP_BITFEXT(temp, MC13892_PWRCTRL1_PCMAXCNT);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_PCCOUNTEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_PCCOUNTEN);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_PCCOUNTEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_PCCOUNTEN);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_CLK32KMCUEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_CLK32KMCUEN);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_CLK32KMCUEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_CLK32KMCUEN);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_USEROFFSPI, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_USEROFFSPI);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_USEROFFSPI, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_USEROFFSPI);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_WARMEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_WARMEN);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_WARMEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_WARMEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableDRM
//
// This function is used to set  Keeps VSRTC and CLK32KMCU on for all states
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableDRM (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_DRM, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_DRM);

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
// This function is used to disable  Keeps VSRTC and CLK32KMCU on for all states
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableDRM (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_DRM, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_DRM);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableUSEROFFCLK
//
// This function is used to set  Keeps the CLK32KMCU active during user off
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableUSEROFFCLK (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_USEROFFCLK, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_USEROFFCLK);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableUSEROFFCLK
//
// This function is used to disable  Keeps the CLK32KMCU active during user off
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableUSEROFFCLK (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_USEROFFCLK, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_USEROFFCLK);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnablePCUTEXPB
//
// This function is used to PCUTEXPB=1 at a startup event indicates that PCUT timer did not
//expire (assuming it was set to 1 after booting)
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnablePCUTEXPB (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_PCUTEXPB, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_PCUTEXPB);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisablePCUTEXPB
//
// This function is used to Disable PCUTEXPB=1 at a startup event indicates that PCUT timer did not
//expire (assuming it was set to 1 after booting)
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisablePCUTEXPB (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_PCUTEXPB, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_PCUTEXPB);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableRESTARTEN
//
// This function is used to Enables automatic restart after a system reset
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableRESTARTEN (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL2_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL2_RESTARTEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_RESTARTEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableRESTARTEN
//
// This function is used to Disable  automatic restart after a system reset
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableRESTARTEN (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL2_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL2_RESTARTEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_RESTARTEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnablePWRONRESET
//
// This function is to system reset on PWRON pin
//
// Parameters:
//             PweIndex [in]         the PWRON key index  to be set

// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnablePWRONRESET (MC13892_PWRCTRL_PWRON PweIndex)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
 

    switch (PweIndex) 
    {
        case PWRON1:
            param.addr = MC13892_PWR_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_PWRCTRL2_ON1BRSTEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_ON1BRSTEN);
            break;
                    
        case PWRON2:
            param.addr = MC13892_PWR_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_PWRCTRL2_ON2BRSTEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_ON2BRSTEN);
            break;
            
         case PWRON3:
            param.addr = MC13892_PWR_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_PWRCTRL2_ON3BRSTEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_ON3BRSTEN);
            break;
                             
        default:
            return PMIC_PARAMETER_ERROR;
    }

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL)) 
            return PMIC_ERROR;          
    
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisablePWRONRESET
//
// This function is to Disable system reset on PWRON pin
//
// Parameters:
//             PweIndex [in]         the PWRON key index  to be set

// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisablePWRONRESET (MC13892_PWRCTRL_PWRON PinIndex)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
 

    switch (PinIndex) 
    {
        case PWRON1:
            param.addr = MC13892_PWR_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_PWRCTRL2_ON1BRSTEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_ON1BRSTEN);
            break;
                    
        case PWRON2:
            param.addr = MC13892_PWR_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_PWRCTRL2_ON2BRSTEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_ON2BRSTEN);
            break;
            
         case PWRON3:
            param.addr = MC13892_PWR_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_PWRCTRL2_ON3BRSTEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_ON3BRSTEN);
            break;
                             
        default:
            return PMIC_PARAMETER_ERROR;
    }

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL)) 
            return PMIC_ERROR;          
    
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetDebtime
//
// This function is to Sets debounce time on PWRON pin.
//
// Parameters:
//             regulator [in]         the PWRON key index  to be set
//             dvsspeed [in]        Turn On Debounce (ms)
//                         0 - 0        ms
//                         1 - 31.25  ms
//                         2 - 125     ms
//                         3 - 750     ms
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetDebtime (
                                                       MC13892_PWRCTRL_PWRON PinIndex, 
                                                       UINT8 Debounce) 
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (Debounce > 0x3){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetDebtime: Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    

    switch (PinIndex) 
    {
        case PWRON1:
            param.addr = MC13892_PWR_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_PWRCTRL2_ON1BDBNC, Debounce);
            param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_ON1BDBNC);
            break;
                    
        case PWRON2:
            param.addr = MC13892_PWR_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_PWRCTRL2_ON2BDBNC, Debounce);
            param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_ON2BDBNC);
            break;

        case PWRON3:
            param.addr = MC13892_PWR_CTL2_ADDR;
            param.data = CSP_BITFVAL(MC13892_PWRCTRL2_ON3BDBNC, Debounce);
            param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_ON3BDBNC);
            break;
                    
        default:
            return PMIC_PARAMETER_ERROR;
    }

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL)) 
            return PMIC_ERROR;          
    
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableSTANDBYINV
//
// This function is STANDBY is interpreted as active low
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableSTANDBYINV (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL2_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL2_STANDBYPRIINV, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_STANDBYPRIINV);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableSTANDBYINV
//
// This function is used to Disable  STANDBY is interpreted as  active not low
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableSTANDBYINV(void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL2_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL2_STANDBYPRIINV, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_STANDBYPRIINV);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableSTANDBYSECINV
//
// This function is STANDBYSEC is interpreted as active low
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableSTANDBYSECINV (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL2_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL2_STANDBYSECINV, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_STANDBYSECINV);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableSTANDBYSECINV
//
// This function is used to Disable  STANDBYSEC is interpreted as  active not low
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableSTANDBYSECINV(void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL2_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL2_STANDBYSECINV, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_STANDBYSECINV);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableWDIRESET
//
// This function is Enable system reset through WDI
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableWDIRESET (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL2_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL2_WDIRESET, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_WDIRESET);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableWDIRESET
//
// This function is used to Disable  system reset through WDI
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableWDIRESET(void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL2_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL2_WDIRESET, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_WDIRESET);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetSPIDRV
//
// This function is used to set SPI drive strength
//
// Parameters:
//             duration [in]            
// SPIDRV[1:0]=00 (default) 11 ns
// SPIDRV[1:0]=01 6 ns
// SPIDRV[1:0]=10 High Z ns
// SPIDRV[1:0]=11 22 ns
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetSPIDRV (UINT8 spidrv)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL2_ADDR;

    // check input parameter
    if (spidrv > 3){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetSPIDRV:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
    param.data = CSP_BITFVAL(MC13892_PWRCTRL2_SPIDRV, spidrv);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_SPIDRV);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetSPIDRV
//
// This function is used to get the SPI drive strength
//
// Parameters:
//             duration [out]    the duration to set to SPI drive strength
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetSPIDRV (UINT8* spidrv)
{
    UINT32 addr, temp = 0;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (spidrv == NULL){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlGetSPIDRV:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    
    addr = MC13892_PWR_CTL2_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *spidrv = (UINT8)CSP_BITFEXT(temp, MC13892_PWRCTRL2_SPIDRV);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}




//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetCLK32KDRV
//
// This function is used to set CLK32K and CLK32KMCU drive strength (master control bits)
//
// Parameters:
//             duration [in]            
// CLK32KDRV[1:0]=00 (default) 22 ns
// CLK32KDRV[1:0]=01 11 ns
// CLK32KDRV[1:0]=10 High Z ns
// CLK32KDRV[1:0]=11 44 ns
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetCLK32KDRV (UINT8 clk32drv)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL2_ADDR;

    // check input parameter
    if (clk32drv > 3){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetSPIDRV:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
    param.data = CSP_BITFVAL(MC13892_PWRCTRL2_CLK32KDRV, clk32drv);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_CLK32KDRV);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetSPIDRV
//
// This function is used to get the CLK32K and CLK32KMCU drive strength (master control bits)
//
// Parameters:
//             duration [out]    the duration to get toCLK32K and CLK32KMCU drive strength (master control bits)
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetCLK32KDRV(UINT8* clk32drv)
{
    UINT32 addr, temp = 0;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (clk32drv == NULL){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlGetSPIDRV:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    
    addr = MC13892_PWR_CTL2_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *clk32drv = (UINT8)CSP_BITFEXT(temp, MC13892_PWRCTRL2_CLK32KDRV);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetSTBYDLY
//
// This function is used to set Standby delay 
//
// Parameters:
//             duration [in]            
// 00 No Delay
// 01 One 32K period (default)
// 10 Two 32K periods
// 11 Three 32K periods
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetSTBYDLY (UINT8 delay)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL2_ADDR;

    // check input parameter
    if (delay > 3){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlSetSPIDRV:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
    param.data = CSP_BITFVAL(MC13892_PWRCTRL2_STBYDLY, delay);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL2_STBYDLY);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetSTBYDLY
//
// This function is used to get the Standby delay 
//
// Parameters:
//             duration [out]    the duration to get to Standby delay 
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetSTBYDLY(UINT8* delay)
{
    UINT32 addr, temp = 0;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (delay == NULL){
        DEBUGMSG(ZONE_ERROR, (_T("PmicPwrctrlGetSPIDRV:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    
    addr = MC13892_PWR_CTL2_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *delay = (UINT8)CSP_BITFEXT(temp, MC13892_PWRCTRL2_STBYDLY);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetMODES
//
// This function is used to get the MODE sense
//
// Parameters:
//             duration [out]    the duration to get the MODE sense
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetMODES (MC13892_PWRCTRL_PWRON *modes)
{
    UINT32 addr, temp = 0;
    

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    addr = MC13892_PWR_MOD_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    temp = (UINT8)CSP_BITFEXT(temp, MC13892_PWR_MOD_MODES);

    *modes=(MC13892_PWRCTRL_PWRON)temp;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetSTBYDLY
//
// This function is used to get the I2CS mode 
//
// Parameters:
//             duration [out]    the duration to get  I2CS mode  
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetI2CS (MC13892_PWRCTRL_I2CS *i2cs)
{
    UINT32 addr, temp = 0;


    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    addr = MC13892_PWR_MOD_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
          &temp, sizeof(temp), NULL, NULL))
    return PMIC_ERROR;

    temp = (UINT8)CSP_BITFEXT(temp, MC13892_PWR_MOD_I2CS);

    *i2cs=(MC13892_PWRCTRL_I2CS)temp;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetPUMSS
//
// This function is used to get the PUMSS mode  
//
// Parameters:
//             duration [out]    the duration to get the PUMSS mode  
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetPUMSS (MC13892_PWRCTRL_PUMSS_INDEX index,MC13892_PWRCTRL_PUMSS *pumss)
{
    UINT32 addr, temp = 0;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    addr = MC13892_PWR_MOD_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    (index==PUMSS1)?temp = (UINT8)CSP_BITFEXT(temp, MC13892_PWR_MOD_PUMSS1):temp = (UINT8)CSP_BITFEXT(temp, MC13892_PWR_MOD_PUMSS2);

    *pumss=(MC13892_PWRCTRL_PUMSS)temp;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableTHSEL
//
// This function is used to Enables Thermal protection threshold select
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableTHSEL (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if(PmicGetVersion()<PMIC_MC13892_VER_20a)
        return PMIC_NOT_SUPPORTED;

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_THSEL, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_THSEL);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableTHSEL
//
// This function is used to Disables Thermal protection threshold select
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableTHSEL (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if(PmicGetVersion()<PMIC_MC13892_VER_20a)
        return PMIC_NOT_SUPPORTED;

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_THSEL, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_THSEL);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



// end of file
