//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  pmic_battery.cpp
//
//  This file contains the PMIC Battery SDK interface that is used by
//  applications and other drivers to access registers of the PMIC (MC13783).
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#pragma warning(pop)

#include "common_macros.h"
#include "pmic_ioctl.h"
#include "pmic_battery.h"
#include "pmic_adc.h"
 
#include "regs.h"
#include "regs_battery.h"

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
// Function: PmicBatterySetChargeVoltage
//
// This function programs the output voltage of charge regulator 
//
// Parameters:
//      chargevoltagelevel [IN] voltage level
//      level 0 = 4.05V
//            1 = 4.10V
//            2 = 4.15V
//            3 = 4.20V
//            4 = 4.25V
//            5 = 4.30V
//            6 = 3.80V ... lowest setting
//            7 = 4.50V      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetChargeVoltage(UINT8 chargevoltagelevel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check charge voltage level parameter
    if (chargevoltagelevel > MC13783_BATTERY_MAX_CHARGE_VOLTAGE_LEVEL)
    {
        ERRORMSG(1, (_T("PmicBatterySetChargeVoltage Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_VCHRG, chargevoltagelevel);
    param.mask = CSP_BITFMASK(MC13783_CHG0_VCHRG);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetChargeVoltage
//
// This function returns the output voltage of charge regulator 
//
// Parameters:
//      chargevoltagelevel [OUT] pointer to voltage level
//      
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetChargeVoltage(UINT8* chargevoltagelevel)
{
    UINT32 param, temp = 0;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    if (chargevoltagelevel == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicBatteryGetChargeVoltage:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    param = MC13783_CHG0_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *chargevoltagelevel  = (UINT8)CSP_BITFEXT(temp, MC13783_CHG0_VCHRG);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetChargeCurrent
//
// This function programs the charge current limit level to the main battery 
//
// Parameters:
//      chargecurrentlevel [IN] current level      
//      level 0 = 0 mA    (max value)
//            1 = 100 mA  (max value)
//              ...       (in increment of 100 mA)
//           13 = 1300 mA (max value)
//           14 = 1800 mA (max value)
//           15 = disables the current limit
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetChargeCurrent (UINT8 chargecurrentlevel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check charge current level parameter
    if (chargecurrentlevel > MC13783_BATTERY_MAX_CHARGE_CURRENT_LEVEL)
    {
        ERRORMSG(1, (_T("PmicBatterySetChargeCurrent Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_ICHRG, chargecurrentlevel);
    param.mask = CSP_BITFMASK(MC13783_CHG0_ICHRG);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetChargeCurrent
//
// This function returns the charge current setting of the main battery 
//
// Parameters:
//      chargecurrentlevel [OUT] pointer to current level      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetChargeCurrent (UINT8* chargecurrentlevel)
{
    UINT32 param, temp;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    
    if (chargecurrentlevel == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicBatteryGetChargeCurrent:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    param = MC13783_CHG0_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *chargecurrentlevel  = (UINT8)CSP_BITFEXT(temp, MC13783_CHG0_ICHRG);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetTrickleCurrent
//
// This function programs the current of the trickle charger
//
// Parameters:
//      tricklecurrentlevel [IN] trickle current level      
//      level 0 = 0 mA 
//            1 = 12 mA
//            ...  (in addition of 12 mA per level)
//            6 = 72 mA
//            7 = 84 mA
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetTrickleCurrent(UINT8 tricklecurrentlevel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check trickle current level parameter
    if (tricklecurrentlevel > MC13783_BATTERY_MAX_TRICKLE_CURRENT_LEVEL)
    {
        ERRORMSG(1, (_T("PmicBatterySetTrickleCurrent Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_ICHRGTR, tricklecurrentlevel);
    param.mask = CSP_BITFMASK(MC13783_CHG0_ICHRGTR);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetTrickleCurrent
//
// This function returns the current of the trickle charger
//
// Parameters:
//      tricklecurrentlevel [OUT] pointer to trickle current level
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetTrickleCurrent (UINT8* tricklecurrentlevel)
{
    UINT32 param, temp;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    if (tricklecurrentlevel == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicBatteryGetTrickleCurrent:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    param = MC13783_CHG0_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *tricklecurrentlevel  = (UINT8)CSP_BITFEXT(temp, MC13783_CHG0_ICHRGTR);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryFETControl
//
// This function programs the control mode and setting of BPFET and FETOVRD
// BATTFET and BPFET to be controlled by FETCTRL bit or hardware
//
// Parameters:
//      fetcontrol [IN] BPFET and FETOVRD control mode and setting 
//      
//      input = 0 (BATTFET and BPFET outputs are controlled by hardware)
//            = 1 (BATTFET and BPFET outputs are controlled by hardware)
//            = 2 (BATTFET low and BATTFET high, controlled by FETCTRL)
//            = 3 (BATTFET high and BATTFET low, controlled by FETCTRL)
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryFETControl(UINT8 fetcontrol)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check FETcontrol input parameter
    if (fetcontrol > MC13783_BATTERY_MAX_FETCONTROL_INPUT_LEVEL)
    {
        ERRORMSG(1, (_T("PmicBatteryFETControl Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_FETOVRD, fetcontrol);
    param.mask = (CSP_BITFMASK(MC13783_CHG0_FETOVRD) | CSP_BITFMASK(MC13783_CHG0_FETCTRL));

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryReverseDisable
//
// This function disables the reverse mode
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryReverseDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_RVRSMODE, MC13783_CHG0_RVRSMODE_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_CHG0_RVRSMODE);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryReverseEnable
//
// This function enables the reverse mode
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryReverseEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_RVRSMODE, MC13783_CHG0_RVRSMODE_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_CHG0_RVRSMODE);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetOvervoltageThreshold
//
// This function programs the overvoltage threshold value
//
// Parameters:
//      ovthresholdlevel [IN] overvoltage threshold level
//      High to low, Low to High  (5.35V)
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetOvervoltageThreshold(UINT8 ovthresholdlevel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check overvoltage threshold level parameter
    if (ovthresholdlevel > MC13783_BATTERY_MAX_OVERVOLTAGE_THRESHOLD_LEVEL)
    {
        ERRORMSG(1, (_T("PmicBatterySetOvervoltageThreshold Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_OVCTRL, ovthresholdlevel);
    param.mask = CSP_BITFMASK(MC13783_CHG0_OVCTRL);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetOvervoltageThreshold
//
// This function returns the overvoltage threshold value
//
// Parameters:
//      ovthresholdlevel [OUT] pointer to overvoltage threshold level
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetOvervoltageThreshold (UINT8* ovthresholdlevel)
{
    UINT32 param, temp;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    if (ovthresholdlevel == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicBatteryGetOvervoltageThreshold:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    param = MC13783_CHG0_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *ovthresholdlevel  = (UINT8)CSP_BITFEXT(temp, MC13783_CHG0_OVCTRL);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryUnregulatedChargeDisable
//
// This function disables the unregulated charge path. The voltage and current 
// limits will be controlled by the charge path regulator.
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryUnregulatedChargeDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_UCHEN, MC13783_CHG0_UCHEN_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_CHG0_UCHEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryUnregulatedChargeEnable
//
// This function enables the unregulated charge path. The settings of the charge
// path regulator (voltage and current limits) will be overruled.
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryUnregulatedChargeEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_UCHEN, MC13783_CHG0_UCHEN_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_CHG0_UCHEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryChargeLedDisable
//
// This function disables the charging LED
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryChargeLedDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_CHRGLEDEN, MC13783_CHG0_CHRGLEDEN_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_CHG0_CHRGLEDEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryChargeLedEnable
//
// This function enables the charging LED
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryChargeLedEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_CHRGLEDEN, MC13783_CHG0_CHRGLEDEN_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_CHG0_CHRGLEDEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryEnablePulldown
//
// This function enables the 5k pulldown resistor used in the dual path charging
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryEnablePulldown()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_CHRGRAWPDEN, MC13783_CHG0_CHRGRAWPDEN_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_CHG0_CHRGRAWPDEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryDisablePulldown
//
// This function disables the 5k pulldown resistor used in dual path charging
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryDisablePulldown()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13783_CHG0_CHRGRAWPDEN, MC13783_CHG0_CHRGRAWPDEN_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_CHG0_CHRGRAWPDEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryEnableCoincellCharger
//
// This function enables the coincell charger
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryEnableCoincellCharger()
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWR_CTL0_COINCHEN, MC13783_PWR_CTL0_COINCHEN_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_PWR_CTL0_COINCHEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicBatteryDisableCoincellCharger
//
// This function disables the coincell charger
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryDisableCoincellCharger()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWR_CTL0_COINCHEN, MC13783_PWR_CTL0_COINCHEN_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_PWR_CTL0_COINCHEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
        
        
    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetCoincellVoltage
//
// This function programs the output voltage level of coincell charger
//
// Parameters:
//      votlagelevel [IN] voltage level
//      level 0 = 2.7V
//            1 = 2.8V
//            2 = 2.9V
//            ... (in 100mV increment)
//            6 = 3.3V   
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetCoincellVoltage (UINT8 coincellvoltagelevel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check coincell voltage level parameter
    if (coincellvoltagelevel > MC13783_BATTERY_MAX_COINCELL_VOLTAGE_LEVEL)
    {
        ERRORMSG(1, (_T("PmicBatterySetCoincellVoltage Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWR_CTL0_VCOIN, coincellvoltagelevel);
    param.mask = CSP_BITFMASK(MC13783_PWR_CTL0_VCOIN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetCoincellVoltage
//
// This function returns the output voltage level of coincell charger
//
// Parameters:
//      voltagelevel [OUT] pointer to voltage level
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetCoincellVoltage (UINT8* coincellvoltagelevel)
{
    UINT32 param, temp;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    if (coincellvoltagelevel == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicBatteryGetCoincellVoltage:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    param = MC13783_PWR_CTL0_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *coincellvoltagelevel  = (UINT8)CSP_BITFEXT(temp, MC13783_PWR_CTL0_VCOIN);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryEnableEolComparator
//
// This function enables the end-of-life function instead of the LOBAT
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryEnableEolComparator()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWR_CTL0_EOLSEL, MC13783_PWR_CTL0_EOLSEL_ENABLE);
    param.mask = CSP_BITFMASK(MC13783_PWR_CTL0_EOLSEL);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryDisableEolComparator
//
// This function disables the end-of-life comparator function
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryDisableEolComparator()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13783_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13783_PWR_CTL0_EOLSEL, MC13783_PWR_CTL0_EOLSEL_DISABLE);
    param.mask = CSP_BITFMASK(MC13783_PWR_CTL0_EOLSEL);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
// Function: PmicBatteryGetChargerMode
//
//      This function returns the charger mode (ie. Dual Path, Single Path, 
//  Serial Path, Dual Input Single Path and Dual Input Serial Path).
// 
//      The way the bits are mapped to the actual pin definitions is:
//              LLOW --> GND 
//              OPEN --> HI Z
//              HHIGH--> VPMIC
//
// Parameters:
//       mode [OUT] pointer to charger mode.
//      
// Returns:
//      PMIC_STATUS.
//
// Remarks: 
//      
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetChargerMode(CHARGER_MODE *mode)
{    
    UINT32 param, temp, chrgmod0, chrgmod1;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    
    if (mode == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicBatteryGetChargerMode:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    } 
    
    param = MC13783_PWR_MOD_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    chrgmod0  = CSP_BITFEXT(temp, MC13783_PWR_MOD_CHRGMOD0);

    chrgmod1  = CSP_BITFEXT(temp, MC13783_PWR_MOD_CHRGMOD1);
    
    //To determine and return the Charger Mode
    if (chrgmod0 == LLOW && chrgmod1 == OPEN)
        *mode = DUAL_PATH;
        
    else if (chrgmod0 == OPEN && chrgmod1 == OPEN)
        *mode = SINGLE_PATH;
               
    else if (chrgmod0 == HHIGH && chrgmod1 == OPEN)
        *mode = SERIAL_PATH;

    else if (chrgmod0 == OPEN && chrgmod1 == HHIGH)
        *mode = DUAL_INPUT_SINGLE_PATH;

    else if (chrgmod0 == HHIGH && chrgmod1 == HHIGH)
        *mode = DUAL_INPUT_SERIAL_PATH;        
    
    else
        *mode = INVALID_CHARGER_MODE;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;

}

//------------------------------------------------------------------------------
// Function: PmicBatteryEnableAdChannel5
//
// This function enables use of AD channel 5 to read charge current
//
// Parameters:
//      None.
//      
// Returns:
//      PMIC_STATUS.
//
// Remarks: 
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryEnableAdChannel5()
{    
    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicBatteryDisableAdChannel5
//
// This function disables use of AD channel 5 to read charge current
//
// Parameters:
//      None.      
//
// Returns:
//      PMIC_STATUS.
//
// Remarks: 
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryDisableAdChannel5()
{    
    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetCoincellCurrentlimit
//
// This function limits the output current level of coincell charger
//
// Parameters:
//      coincellcurrentlevel [IN] coincell current level      
//
// Returns:
//      PMIC_STATUS.
//
// Remarks: 
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetCoincellCurrentlimit (UINT8 coincellcurrentlevel)
{
    UNREFERENCED_PARAMETER(coincellcurrentlevel);
    
    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetCoincellCurrentlimit
//
// This function returns the output current limit of coincell charger
//
// Parameters:
//      coincellcurrentlevel [OUT] pointer to coincell current level
//      
//
// Returns:
//      PMIC_STATUS.
//
// Remarks: 
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetCoincellCurrentlimit (UINT8* coincellcurrentlevel)
{
    UNREFERENCED_PARAMETER(coincellcurrentlevel);

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetEolTrip
//
// This function sets the end-of-life threshold
//
// Parameters:
//      eoltriplevel [IN] eol trip level      
//
// Returns:
//      PMIC_STATUS.
//
// Remarks: 
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetEolTrip (UINT8 eoltriplevel)
{
    UNREFERENCED_PARAMETER(eoltriplevel);

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetEolTrip
//
// This function returns the end-of-life threshold
//
// Parameters:
//      eoltriplevel [OUT] pointer to eol trip level      
//
// Returns:
//      PMIC_STATUS.
//
// Remarks: 
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetEolTrip (UINT8* eoltriplevel)
{
    UNREFERENCED_PARAMETER(eoltriplevel);

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterEnableCharger
//
// This function is used to start charging a battery. For different charger,
// different voltage and current range are supported.
//
// Parameters:
//      chgr
//          [in]  Charger as defined in BATT_CHARGER.
//      c_voltage
//          [in]  Charging voltage.
//      c_current
//          [in]  Charging current.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterEnableCharger(BATT_CHARGER chgr, UINT8 c_voltage,
                                        UINT8 c_current)
{
     switch (chgr) 
    {
    case BATT_MAIN_CHGR:
        PmicBatterySetChargeVoltage(c_voltage);
        PmicBatterySetChargeCurrent (c_current);
        break;
                
    case BATT_CELL_CHGR:
        PmicBatteryEnableCoincellCharger ();
        PmicBatterySetCoincellVoltage (c_voltage);
        break;
                
    case BATT_TRCKLE_CHGR:
        PmicBatterySetTrickleCurrent(c_current);
        break;

    default:
        return PMIC_PARAMETER_ERROR;
        break;
    }

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterDisableCharger
//
// This function turns off the selected charger.
//
// Parameters:
//      chgr
//          [in]  Charger as defined in BATT_CHARGER.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterDisableCharger(BATT_CHARGER chgr)
{
    switch (chgr) 
    {
    case BATT_MAIN_CHGR:
        PmicBatterySetChargeCurrent(0);
        break;
                
    case BATT_CELL_CHGR:
        PmicBatteryDisableCoincellCharger();
        break;
                
    case BATT_TRCKLE_CHGR:
        PmicBatterySetTrickleCurrent(0);
        break;

    default:
        return PMIC_PARAMETER_ERROR;
        break;
    }

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterSetCharger
//
// This function is used to change the charger setting.
//
// Parameters:
//      chgr
//          [in]  Charger as defined in BATT_CHARGER.
//      c_voltage
//          [in]  Charging voltage.
//      c_current
//          [in]  Charging current.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterSetCharger(BATT_CHARGER chgr, UINT8 c_voltage,
                                        UINT8 c_current)
{
    switch (chgr) 
    {
    case BATT_MAIN_CHGR:
        PmicBatterySetChargeVoltage(c_voltage);
        PmicBatterySetChargeCurrent (c_current);
        break;
                
    case BATT_CELL_CHGR:
        PmicBatterySetCoincellVoltage (c_voltage);
        PmicBatteryEnableCoincellCharger();
        break;
                
    case BATT_TRCKLE_CHGR:
        PmicBatterySetTrickleCurrent(c_current);
        break;

    default:
        return PMIC_PARAMETER_ERROR;
        break;
    }

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterGetChargerSetting
//
// This function is used to retrive the charger setting.
//
// Parameters:
//      chgr
//          [in]  Charger as defined in BATT_CHARGER.
//      c_voltage
//          [out] a pointer to what the charging voltage is set to.
//      c_current
//          [out] a pointer to what the charging current is set to.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterGetChargerSetting(BATT_CHARGER chgr, UINT8* c_voltage, 
                                        UINT8* c_current)
{
    switch (chgr) 
    {
    case BATT_MAIN_CHGR:
        PmicBatteryGetChargeVoltage(c_voltage);
        PmicBatteryGetChargeCurrent (c_current);
        break;
                
    case BATT_CELL_CHGR:
        PmicBatteryGetCoincellVoltage (c_voltage);
        break;
                
    case BATT_TRCKLE_CHGR:
        PmicBatteryGetTrickleCurrent(c_current);
        break;

    default:
        return PMIC_PARAMETER_ERROR;
        break;
    }

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterGetChargeCurrent
//
// This function retrives the main charger current.
//
// Parameters:
//      c_current
//          [out] a pointer to what the charging current measures
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterGetChargeCurrent(UINT16* c_current)
{
    // 
    return PmicADCGetSingleChannelEightSamples(CHRGISNS_CHNL,
                                                c_current);
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterEnableEol
//
// This function enables End-of-Life comparator.
//
// Parameters:  -- none
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterEnableEol(void)
{
    return PmicBatteryEnableEolComparator();
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterDisableEol
//
// This function disables End-of-Life comparator.
//
// Parameters:  -- none
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterDisableEol(void)
{
    return PmicBatteryDisableEolComparator();
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterLedControl
//
// This function controls charge LED.
//
// Parameters:
//      on
//          [in]  If on is ture, LED will be turned on, or otherwise the
//                LED will be turned off.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterLedControl(BOOL on)
{
    if (on)
        return PmicBatteryChargeLedEnable();
    else
        return PmicBatteryChargeLedDisable();
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterSetReverseSupply
//
// This function sets reverse supply mode. 
//
// Parameters:
//      enable 
//          [i]]  If enable is ture, reverse supply mode is enable or otherwise 
//                the reverse supply mode is disabled.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterSetReverseSupply(BOOL enable)
{
    if(enable)
        return PmicBatteryReverseEnable();
    else
        return PmicBatteryReverseDisable();
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterSetUnregulated
//
// This function sets unregulatored charging mode on main battery.
//
// Parameters:
//      enable
//          [in]  If enable is ture, unregulated charging mode is enabled 
//                otherwise it is disabled.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterSetUnregulated(BOOL enable)
{
    if(enable)
        return PmicBatteryUnregulatedChargeEnable();
    else
        return PmicBatteryUnregulatedChargeDisable();
}
