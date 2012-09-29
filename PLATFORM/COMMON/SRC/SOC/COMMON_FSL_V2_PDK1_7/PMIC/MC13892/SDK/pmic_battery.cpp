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
//  applications and other drivers to access registers of the PMIC (MC13892).
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
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
//      000 3.800
//      001 4.050
//      010 4.150
//      011 (default) 4.200
//      100 4.250
//      101 4.300
//      110 4.375
//      111 4.500
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
    if (chargevoltagelevel > MC13892_BATTERY_MAX_CHARGE_VOLTAGE_LEVEL)
    {
        ERRORMSG(1, (_T("PmicBatterySetChargeVoltage Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_VCHRG, chargevoltagelevel);
    param.mask = CSP_BITFMASK(MC13892_CHG0_VCHRG);

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
    param = MC13892_CHG0_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *chargevoltagelevel  = (UINT8)CSP_BITFEXT(temp, MC13892_CHG0_VCHRG);

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
//        0000 0 Off
//        0001 80 Standalone Charging Default for precharging, USB charging, and LPB
//        0010 240
//        0011 320
//        0100 400 Advised setting for USB charging with PHY active
//        0101 480
//        0110 560 Standalone Charging Default
//        0111 640
//        1000 720
//        1001 800
//        1010 880
//        1011 960
//        1100 1040
//        1101 1200 High Current Charger
//        1110 1600 High Current Charger
//        1111 Fully On ¨C M3 Open Externally Powered
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetChargeCurrent (UINT8 chargecurrentlevel)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check charge current level parameter
    if (chargecurrentlevel > MC13892_BATTERY_MAX_CHARGE_CURRENT_LEVEL)
    {
        ERRORMSG(1, (_T("PmicBatterySetChargeCurrent Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_ICHRG, chargecurrentlevel);
    param.mask = CSP_BITFMASK(MC13892_CHG0_ICHRG);

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
    param = MC13892_CHG0_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *chargecurrentlevel  = (UINT8)CSP_BITFEXT(temp, MC13892_CHG0_ICHRG);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryTrickleCurrentDisable
//
// This function disables the Trickle Current mode
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryTrickleCurrentDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_TREN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_TREN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryTrickleCurrentEnable
//
// This function enables the Trickle Current mode
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryTrickleCurrentEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_TREN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_TREN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicBatteryACKLPBDisable
//
// This function disablesAcknowledge Low Power Boot mode
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryACKLPBDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_ACKLPB, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_ACKLPB);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryACKLPBEnable
//
// This function enables Acknowledge Low Power Boot  mode
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryACKLPBEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_ACKLPB, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_ACKLPB);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryThermistorCheckDisable
//
// This function Battery thermistor check disable
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryThermistorCheckDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_THCHKB, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_THCHKB);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryThermistorCheckDisable
//
// This function enables Battery thermistor check 
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryThermistorCheckEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_THCHKB, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_THCHKB);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryAllowBATTFETCtlDisable
//
// This function disables Allows BATTFET Control
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryAllowBATTFETCtlDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_FETOVRD, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_FETOVRD);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryAllowBATTFETCtlEnable
//
// This function enables Allows BATTFET Control
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryAllowBATTFETCtlEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_FETOVRD, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_FETOVRD);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryBATTFETCtlDisable
//
// This function disables BATTFET Control
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryBATTFETCtlDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_FETCTRL, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_FETCTRL);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryBATTFETCtlEnable
//
// This function enables BATTFET Control
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryBATTFETCtlEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_FETCTRL, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_FETCTRL);

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

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_RVRSMODE, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_RVRSMODE);

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

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_RVRSMODE, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_RVRSMODE);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetPowerlimiter
//
// This function programs set the Charger Power Dissipation Limiter  
//
// Parameters:
//      chargevoltagelevel [IN] voltage level
//        PLIM[1:0] Power Limit (mW)
//        00 (default) 600
//        01 800
//        10 1000
//        11 1200
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetPowerlimiter(UINT8 powerlimter)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check charge voltage level parameter
    if (powerlimter > MC13892_BATTERY_MAX_CHARGE_POWER_LIMTER)
    {
        ERRORMSG(1, (_T("PmicBatterySetChargeVoltage Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_PLIM, powerlimter);
    param.mask = CSP_BITFMASK(MC13892_CHG0_PLIM);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetPowerlimiter
//
// This function returns  the Charger Power Dissipation Limiter  
//
// Parameters:
//      chargevoltagelevel [OUT] pointer to voltage level
//      
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetPowerlimiter(UINT8* powerlimter)
{
    UINT32 param, temp = 0;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    if (powerlimter == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicBatteryGetChargeVoltage:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    param = MC13892_CHG0_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *powerlimter  = (UINT8)CSP_BITFEXT(temp, MC13892_CHG0_PLIM);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBatterytPowerlimiterDisable
//
// This function Power limiter disable 
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterytPowerlimiterDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_PLIMDIS, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_PLIMDIS);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterytPowerlimiterEnable
//
// This function enables Power limiter 
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterytPowerlimiterEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_PLIMDIS, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_PLIMDIS);

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

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CHRGLEDEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CHRGLEDEN);

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

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CHRGLEDEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CHRGLEDEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicBatteryRestartsChargerStateDisable
//
// This function disables Restarts charger state machine
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryRestartsChargerStateDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CHGRESTART, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CHGRESTART);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryRestartsChargerStateEnable
//
// This function enables theRestarts charger state machine
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryRestartsChargerStateEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CHGRESTART, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CHGRESTART);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}
//------------------------------------------------------------------------------
//
// Function: PmicBatteryAvoidsAutoCharingDisable
//
// This function disables the charging Avoids automatic charging while on
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryAvoidsAutoCharingDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CHGAUTOB, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CHGAUTOB);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryAvoidsAutoCharingEnable
//
// This function enables the charging Avoids automatic charging while on
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryAvoidsAutoCharingEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CHGAUTOB, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CHGAUTOB);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBatteryCyclingDisable
//
// This function disables the charging Cycling
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryCyclingDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CYCLB, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CYCLB);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryCyclingEnable
//
// This function enables the charging Cycling
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryCyclingEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CYCLB, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CYCLB);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicBatteryAllowsVIDisable
//
// This function disables the Allows V and I programming
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryAllowsVIDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CHGAUTOVIB, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CHGAUTOVIB);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryAllowsVIEnable
//
// This function enables Allows V and I programming
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryAllowsVIEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CHGAUTOVIB, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CHGAUTOVIB);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryAllowsVIDisable
//
// This function disables Enables battery detect function
//
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryAllowsBatDetDisable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_BATTDETEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_BATTDETEN);

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
              sizeof(param), NULL, 0, NULL, NULL))
        return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryAllowsBatDetEnable
//
// This function enables  battery detect function 
// Parameters:
//      None.
//      
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryAllowsBatDetEnable()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWRCTRL0_BATTDETEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWRCTRL0_BATTDETEN);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWR_CTL0_COINCHEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_PWR_CTL0_COINCHEN);

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

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWR_CTL0_COINCHEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_PWR_CTL0_COINCHEN);

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
    if (coincellvoltagelevel > MC13892_BATTERY_MAX_COINCELL_VOLTAGE_LEVEL)
    {
        ERRORMSG(1, (_T("PmicBatterySetCoincellVoltage Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    param.addr = MC13892_PWR_CTL0_ADDR;
    param.data = CSP_BITFVAL(MC13892_PWR_CTL0_VCOIN, coincellvoltagelevel);
    param.mask = CSP_BITFMASK(MC13892_PWR_CTL0_VCOIN);

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
    param = MC13892_PWR_CTL0_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    *coincellvoltagelevel  = (UINT8)CSP_BITFEXT(temp, MC13892_PWR_CTL0_VCOIN);

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
    UINT32 param, temp, chrgmod0;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    
    if (mode == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicBatteryGetChargerMode:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    } 
    
    param = MC13892_PWR_MOD_ADDR;

    if(! DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &param, sizeof(param),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    chrgmod0  = CSP_BITFEXT(temp, MC13892_PWR_MOD_CHRGSSS);

    //To determine and return the Charger Mode
        
    if (chrgmod0 ==0x0)
        *mode = SINGLE_PATH;
               
    else if (chrgmod0 == 0x1)
        *mode = SERIAL_PATH;        
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
//      in MC13892
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
//      in MC13892
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
//      in MC13892
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
//      in MC13892
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
//      in MC13892
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
//      in MC13892
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
        PmicBatteryTrickleCurrentEnable();
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
        PmicBatteryTrickleCurrentDisable();
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
        PmicBatteryTrickleCurrentEnable();
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
        PmicBatteryTrickleCurrentEnable();
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
// Function: PmicBatterEnableCHGTMRRST
//
// This function is used to Enables Charge timer reset
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterEnableCHGTMRRST (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if(PmicGetVersion()<PMIC_MC13892_VER_20a)
        return PMIC_NOT_SUPPORTED;

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CHGTMRRST, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CHGTMRRST);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterDisableCHGTMRRST
//
// This function is used to Disables Charge timer reset
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterDisableCHGTMRRST (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if(PmicGetVersion()<PMIC_MC13892_VER_20a)
        return PMIC_NOT_SUPPORTED;

    param.addr = MC13892_CHG0_ADDR;
    param.data = CSP_BITFVAL(MC13892_CHG0_CHGTMRRST, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_CHG0_CHGTMRRST);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


