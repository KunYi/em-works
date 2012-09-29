/*---------------------------------------------------------------------------
* Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//
//  File:  pmic_regulator.cpp
//
//  This file contains the PMIC regulator SDK interface that is used by applications
//  and other drivers to access registers of the PMIC.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "common_macros.h"
#include "pmic_ioctl.h"
#include "regs.h"
#include "regs_regulator.h"
#include "pmic_regulator.h"
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
extern LONG  RegRefCount[VMAX];
extern LONG  GateRefCount[2];
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
// Function: PmicSwitchModeRegulatorOn
//
// This function is used to turn on the switch mode regulator. 
//
// Parameters:
//             regulator [in]             which switch mode regulator to turn on
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorOn (PMIC_REGULATOR_SREG regulator) 
{
    UNREFERENCED_PARAMETER(regulator);
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorOff
//
// This function is used to turn off the switch regulator
//
// Parameters:
//             regulator [in]           
//                     which switch mode regulator to turn off
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorOff (PMIC_REGULATOR_SREG regulator) 
{
    UNREFERENCED_PARAMETER(regulator);

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;

}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorSetVoltageLevel
//
// This function is to set the voltage level for the regulator.
//
// Parameters:
//          regulator [in]       which switch mode regulator to be set
//
//          voltageType [in]     
//                  SW_VOLTAGE_NORMAL/SW_VOLTAGE_DVS/SW_VOLTAGE_STBY
//
//                  SWxy offers support for Dynamic Voltage-Frequency scaling. If this feature is
//                  activated, then assertion of the STANDBY input will automatically configure 
//                  SWxy to output the voltage defined by the 6-bit field SWxy_STBY. 
//                  If STANDBY=LOW, then assertion of the DVS input will automatically configure 
//                  SWxy to output the voltage defined by the 6-bit field SWxy_DVS. These 
//                  alternative bit fields would normally be programmed to a voltage lower than
//                  that encoded in the SWxy bit field. When STANDBY and DVS are both 
//                  de-asserted, the output voltage will revert the that encoded by the SWxy field.
//
//          voltage [in]        voltage to be set to the regulator. 
//                                  0-0x1F 
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorSetVoltageLevel (
                                                       PMIC_REGULATOR_SREG regulator, 
                                                       PMIC_REGULATOR_SREG_VOLTAGE_TYPE voltageType,
                                                       PMIC_REGULATOR_SREG_VOLTAGE voltage ) 
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (voltage > MC13892_SW_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorSetVoltageLevel:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    

    switch (regulator) 
    {
        case SW1:
            param.addr = MC13892_SW0_ADDR;
            switch (voltageType)
            {
                case SW_VOLTAGE_NORMAL:
                    param.data = CSP_BITFVAL(MC13892_SW0_SW1, voltage);
                    param.mask = CSP_BITFMASK(MC13892_SW0_SW1);
                    break;
                    
                case SW_VOLTAGE_DVS:
                    param.data = CSP_BITFVAL(MC13892_SW0_SW1DVS, voltage);
                    param.mask = CSP_BITFMASK(MC13892_SW0_SW1DVS);
                    break;
                    
                case SW_VOLTAGE_STBY:
                    param.data = CSP_BITFVAL(MC13892_SW0_SW1STBY, voltage);
                    param.mask = CSP_BITFMASK(MC13892_SW0_SW1STBY);
                    break;
            }
            break;
                    
        case SW2:
            param.addr = MC13892_SW1_ADDR;

            switch (voltageType)
            {
                case SW_VOLTAGE_NORMAL:
                    param.data = CSP_BITFVAL(MC13892_SW1_SW2, voltage);
                    param.mask = CSP_BITFMASK(MC13892_SW1_SW2);
                    break;
                    
                case SW_VOLTAGE_DVS:
                    param.data = CSP_BITFVAL(MC13892_SW1_SW2DVS, voltage);
                    param.mask = CSP_BITFMASK(MC13892_SW1_SW2DVS);
                    break;
                    
                case SW_VOLTAGE_STBY:
                    param.data = CSP_BITFVAL(MC13892_SW1_SW2STBY, voltage);
                    param.mask = CSP_BITFMASK(MC13892_SW1_SW2STBY);
                    break;
            }
            break;
                    
        case SW3:
            param.addr = MC13892_SW3_ADDR;
            switch (voltageType)
            {
                case SW_VOLTAGE_NORMAL:
                    param.data = CSP_BITFVAL(MC13892_SW2_SW3, voltage);
                    param.mask = CSP_BITFMASK(MC13892_SW2_SW3);
                    break;
                    
                    
                case SW_VOLTAGE_STBY:
                    param.data = CSP_BITFVAL(MC13892_SW2_SW3STBY, voltage);
                    param.mask = CSP_BITFMASK(MC13892_SW2_SW3STBY);
                    break;

                default:
                       return PMIC_PARAMETER_ERROR;
            }
            break;
                    
        case SW4:
            param.addr = MC13892_SW4_ADDR;

            switch (voltageType)
            {
                case SW_VOLTAGE_NORMAL:
                    param.data = CSP_BITFVAL(MC13892_SW3_SW4, voltage);
                    param.mask = CSP_BITFMASK(MC13892_SW3_SW4);
                    break;
                   
                    
                case SW_VOLTAGE_STBY:
                    param.data = CSP_BITFVAL(MC13892_SW3_SW4STBY, voltage);
                    param.mask = CSP_BITFMASK(MC13892_SW3_SW4STBY);
                    break;

                  default:
                       return PMIC_PARAMETER_ERROR;
                
            }
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
// Function: PmicSwitchModeRegulatorGetVoltageLevel
//
// This function is to get the voltage settings.
// Parameters:
//          regulator [in]             which regulator to get the voltage value
//
//          voltageType [in]         
//                                          SW_VOLTAGE_NORMAL
//                                          SW_VOLTAGE_DVS
//                                          SW_VOLTAGE_STBY
//
//          voltage [out]              the pointer to store the return value
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorGetVoltageLevel (
                                       PMIC_REGULATOR_SREG regulator, 
                                       PMIC_REGULATOR_SREG_VOLTAGE_TYPE voltageType,
                                       PMIC_REGULATOR_SREG_VOLTAGE* voltage)
{
    UINT32 addr, temp = 0;
       
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (voltage == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorGetVoltageLevel:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    switch (regulator) 
    {
        case SW1:
            addr = MC13892_SW0_ADDR;

            if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
                      &temp, sizeof(temp), NULL, NULL))
                     return PMIC_ERROR;

            if (voltageType == SW_VOLTAGE_NORMAL)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13892_SW0_SW1);
            else if (voltageType == SW_VOLTAGE_DVS)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13892_SW0_SW1DVS);
            else  // voltageType == SW_VOLTAGE_STBY
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13892_SW0_SW1STBY);

            break;
 
        case SW2:
            addr = MC13892_SW1_ADDR;

            if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
                      &temp, sizeof(temp), NULL, NULL))
                     return PMIC_ERROR;

            if (voltageType == SW_VOLTAGE_NORMAL)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13892_SW1_SW2);
            else if (voltageType == SW_VOLTAGE_DVS)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13892_SW1_SW2DVS);
            else //voltageType == SW_VOLTAGE_STBY
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13892_SW1_SW2STBY);

            break;

        case SW3:
            addr = MC13892_SW2_ADDR;

            if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
                      &temp, sizeof(temp), NULL, NULL))
                     return PMIC_ERROR;

            if (voltageType == SW_VOLTAGE_NORMAL)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13892_SW2_SW3);
            else  //voltageType == SW_VOLTAGE_STBY
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13892_SW2_SW3STBY);

            break;

        case SW4:
            addr = MC13892_SW3_ADDR;

            if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
                      &temp, sizeof(temp), NULL, NULL))
                     return PMIC_ERROR;

            if (voltageType == SW_VOLTAGE_NORMAL)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13892_SW3_SW4);
            else //voltageType == SW_VOLTAGE_STBY
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13892_SW3_SW4STBY);

            break;

     
        default:
            return PMIC_PARAMETER_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorSetMode
//
// This function is to set the switch mode regulator into different mode for both standby 
// and normal case.
//
// Parameters:
//          regulator [in]        the regulator to be set
//
//          standby [i]          standby = LOW,  the mode will be set in SWxMODE
//                                   standby = HIGH,  the mode will be set in SWxSTBYMODE
//
//          mode [in]          the mode to be use
//              1. OFF
//              2. PWM mode and no Pulse Skipping    
//              3. PWM mode and pulse Skipping Allowed 
//              4. Low Power PFM mode
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorSetMode (
                                                       PMIC_REGULATOR_SREG regulator, 
                                                       PMIC_REGULATOR_SREG_STBY standby,
                                                       PMIC_REGULATOR_SREG_MODE mode ) 
{
    PMIC_PARAM_LLA_WRITE_REG param;
    BYTE data;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    
    if (standby == LOW )
    {
        switch (mode) 
        {
            case SW_MODE_OFF:
                data=0x0;
                break;

            case SW_MODE_PWM:
                data=0x5;
                break;

            case SW_MODE_PWMPS:
                data=0x9;
                break;

            case SW_MODE_PFM:
                data=0xf;
                break;

            case SW_MODE_Auto:
                data=0x8;
                break;

            default:
                return PMIC_PARAMETER_ERROR;
        }

        switch (regulator) 
        {
            case SW1:
                param.addr = MC13892_SW4_ADDR;
                param.data =    CSP_BITFVAL(MC13892_SW4_SW1MODE, data);
                param.mask =    CSP_BITFMASK(MC13892_SW4_SW1MODE);
                break;

            case SW2:
                param.addr = MC13892_SW4_ADDR;
                param.data = CSP_BITFVAL(MC13892_SW4_SW2MODE, data);
                param.mask = CSP_BITFMASK(MC13892_SW4_SW2MODE);
                break;

            case SW3:
                param.addr = MC13892_SW5_ADDR;
                param.data =    CSP_BITFVAL(MC13892_SW5_SW3MODE, data);
                param.mask = CSP_BITFMASK(MC13892_SW5_SW3MODE);
                break;

            case SW4:
                param.addr = MC13892_SW5_ADDR;
                param.data = CSP_BITFVAL(MC13892_SW5_SW4MODE, data);
                param.mask = CSP_BITFMASK(MC13892_SW5_SW4MODE);
                break;
            default:
                return PMIC_PARAMETER_ERROR;
        }
    }
    else
    {
        switch (mode) 
        {
            case SW_MODE_OFF:
                data=0x0;
                break;

            case SW_MODE_PWM:
                data=0x1;
                break;

            case SW_MODE_PWMPS:
                data=0x2;
                break;

            case SW_MODE_PFM:
                data=0x3;
                break;

            case SW_MODE_Auto:
                data=0x4;
                break;

            default:
                return PMIC_PARAMETER_ERROR;
        }


        switch (regulator) 
        {
            case SW1:
                param.addr = MC13892_SW4_ADDR;
                param.data =    CSP_BITFVAL(MC13892_SW4_SW1MODE, data);
                param.mask = CSP_BITFMASK(MC13892_SW4_SW1MODE);
                break;

            case SW2:
                param.addr = MC13892_SW4_ADDR;
                param.data =    CSP_BITFVAL(MC13892_SW4_SW2MODE, data);
                param.mask = CSP_BITFMASK(MC13892_SW4_SW2MODE);
                break;
                
            case SW3:
                param.addr = MC13892_SW5_ADDR;
                param.data =    CSP_BITFVAL(MC13892_SW5_SW3MODE, data);
                param.mask = CSP_BITFMASK(MC13892_SW5_SW3MODE);
                break;
                
            case SW4:
                param.addr = MC13892_SW5_ADDR;
                param.data =    CSP_BITFVAL(MC13892_SW5_SW4MODE, data);
                param.mask = CSP_BITFMASK(MC13892_SW5_SW4MODE);
                break;
            default:
                return PMIC_PARAMETER_ERROR;
        }
    }


    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL)) 
            return PMIC_ERROR;          
    
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorGetMode
//
// This function is to get the switch regulator mode settings.
//
// Parameters:
//          regulator [in]        the regulator to get the settings from.
//
//          standby [i]          standby = LOW,  the mode will be set in SWxMODE
//                                   standby = HIGH,  the mode will be set in SWxSTBYMODE
//
//          mode [out]          the pointer to get the mode settings
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorGetMode (
                                                       PMIC_REGULATOR_SREG regulator, 
                                                       PMIC_REGULATOR_SREG_STBY standby,
                                                       PMIC_REGULATOR_SREG_MODE* mode ) 
{
    UINT32 addr, temp = 0;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (mode == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorGetMode:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    if (regulator == SW3)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorGetMode:SW3 does not support these modes\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    else if (regulator <SW3)
        addr = MC13892_SW4_ADDR;
    else
        addr = MC13892_SW5_ADDR;
    
    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    if (standby == LOW )
    {
        switch (regulator) 
        {
            case SW1:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13892_SW4_SW1MODE);
                break;

            case SW2:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13892_SW4_SW2MODE);
                break;
                
            case SW3:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13892_SW5_SW3MODE);
                break;

            case SW4:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13892_SW5_SW4MODE);
                break;
            default:
                return PMIC_PARAMETER_ERROR;
        }
    }
    else
    {
        switch (regulator) 
        {
            case SW1:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13892_SW4_SW1MODE);
                break;

            case SW2:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13892_SW4_SW2MODE);
                break;
                
            case SW3:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13892_SW5_SW3MODE);
                break;

            case SW4:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13892_SW5_SW4MODE);
                break;
            default:
                return PMIC_PARAMETER_ERROR;
        }
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorSetDVSSpeed
//
// This function is to set the DVS speed the regulator.
//
// Parameters:
//             regulator [in]         the regulator to be set
//             dvsspeed [in]     
//                         0 - 25mV step each 2us
//                         1 - 25mV step each 4us
//                         2 - 25mV step each 8us
//                         3 - 25mV step each 16us
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorSetDVSSpeed (
                                                       PMIC_REGULATOR_SREG regulator, 
                                                       UINT8 dvsspeed) 
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (dvsspeed > MC13892_SW_DVSSPEED_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorSetDVSSpeed: Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    

    switch (regulator) 
    {
        case SW1:
            param.addr = MC13892_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW4_SW1DVSSPEED, dvsspeed);
            param.mask = CSP_BITFMASK(MC13892_SW4_SW1DVSSPEED);
            break;
                    
        case SW2:
            param.addr = MC13892_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW4_SW2DVSSPEED, dvsspeed);
            param.mask = CSP_BITFMASK(MC13892_SW4_SW2DVSSPEED);
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
// Function: PmicSwitchModeRegulatorSetSidLevel
//
// This function is to set the DVS speed the regulator.
//
// Parameters:
//             regulator [in]         the regulator to be set
//             dvsspeed [in]     
//                                         hilevel
//             dvsspeed [in]     
//                                         lowlevel
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorSetSidLevel (
                                                       PMIC_REGULATOR_SREG regulator, 
                                                        UINT8 hilevel,
                                                        UINT8 lowlevel)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (hilevel > MC13892_SW_DIP_LEVEL){
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorSetSidLevel: Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    

    if (lowlevel > MC13892_SW_DIP_LEVEL){
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorSetSidLevel: Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
     
    switch (regulator) 
    {
        case SW1:
            param.addr = MC13892_SW0_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW0_SW1SIDMAX, hilevel)|CSP_BITFVAL(MC13892_SW0_SW1SIDMIN, lowlevel);
            param.mask = CSP_BITFMASK(MC13892_SW0_SW1SIDMAX)|CSP_BITFMASK(MC13892_SW0_SW1SIDMIN);
            break;
                    
        case SW2:
            param.addr = MC13892_SW1_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW1_SW2SIDMAX, hilevel)|CSP_BITFVAL(MC13892_SW1_SW2SIDMIN, lowlevel);
            param.mask = CSP_BITFMASK(MC13892_SW1_SW2SIDMAX)|CSP_BITFMASK(MC13892_SW1_SW2SIDMIN);
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
// Function: PmicSwitchModeRegulatorSetPLLMF
//
// This function is to set the DVS speed the regulator.
//
// Parameters:
//             mf        [in]     
//PLLX[2:0] Multiplication Factor Switching Frequency (Hz)
//000            84                   2 752 512
//001            87                   2 850 816
//010            90                   2 949 120
//011            93                   3 047 424
//100         (default) 96         3 145 728
//101            99                   3 244 032
//110            102                 3 342 336
//111            105                 3 440 640 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorSetPLLMF (
                                                       UINT8 mf) 
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // check input parameter
    if (mf > MC13892_SW_PLLMF_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorSetDVSSpeed: Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    
  
    param.addr = MC13892_SW4_ADDR;
    param.data = CSP_BITFVAL(MC13892_SW4_PLLX, mf);
    param.mask = CSP_BITFMASK(MC13892_SW4_PLLX);
          
    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL)) 
            return PMIC_ERROR;          
    
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}
//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorEnableHIRANGE
//
// This function is used to set the high range select  if enable ,the output (1.1 ~1.85)
//
// Parameters:
//           regulator [in]          the regulator to be set
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorEnableHIRANGE(PMIC_REGULATOR_SREG regulator)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {

        case SW2:
            param.addr = MC13892_SW1_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW1_SW2HI, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_SW1_SW2HI);
            break;
                    
        case SW3:
            param.addr = MC13892_SW2_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW2_SW3HI, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_SW2_SW3HI);
            break;
                    
        case SW4:
            param.addr = MC13892_SW3_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW3_SW4HI, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_SW3_SW4HI);
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
// Function: PmicSwitchModeRegulatorDisableHIRANGE
//
// This function is used to set the low range select  if enable ,the output (0.6 ~1.375)
// Parameters:
//           regulator [in]             the regulator to be set
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorDisableHIRANGE(PMIC_REGULATOR_SREG regulator)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    
    switch (regulator) 
    {
       
       case SW2:
            param.addr = MC13892_SW1_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW1_SW2HI, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_SW1_SW2HI);
            break;
                    
        case SW3:
            param.addr = MC13892_SW2_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW2_SW3HI, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_SW2_SW3HI);
            break;
                    
        case SW4:
            param.addr = MC13892_SW3_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW3_SW4HI, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_SW3_SW4HI);
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
// Function: PmicSwitchModeRegulatorEnablePanicMode
//
// This function is used to enable the panic mode.
//
// Parameters:
//           regulator [in]          the regulator to be set
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorEnableMemoryHoldMode(PMIC_REGULATOR_SREG regulator)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case SW1:
            param.addr = MC13892_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW4_SW1MHMODE, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_SW4_SW1MHMODE);
            break;
                    
        case SW2:
            param.addr = MC13892_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW4_SW2MHMODE, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_SW4_SW2MHMODE);
            break;
                    
        case SW3:
            param.addr = MC13892_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW5_SW3MHMODE, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_SW5_SW3MHMODE);
            break;
                    
        case SW4:
            param.addr = MC13892_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW5_SW4MHMODE, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_SW5_SW4MHMODE);
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
// Function: PmicSwitchModeRegulatorDisablePanicMode
//
// This function is used to disable the panic mode.
//
// Parameters:
//           regulator [in]             the regulator to be set
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorDisableMemoryHoldMode(PMIC_REGULATOR_SREG regulator)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case SW1:
            param.addr = MC13892_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW4_SW1MHMODE, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_SW4_SW1MHMODE);
            break;
                    
        case SW2:
            param.addr = MC13892_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW4_SW2MHMODE, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_SW4_SW2MHMODE);
            break;
                    
        case SW3:
            param.addr = MC13892_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW5_SW3MHMODE, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_SW5_SW3MHMODE);
            break;
                    
        case SW4:
            param.addr = MC13892_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW5_SW4MHMODE, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_SW5_SW4MHMODE);
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
// Function: PmicSwitchModeRegulatorEnableSoftStart
//
// This function is used to enable soft start.
//
// Parameters:
//           regulator [in]        the regulator to be set
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorEnableUserOffMode(PMIC_REGULATOR_SREG regulator)
{
     PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case SW1:
            param.addr = MC13892_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW4_SW1UOMODE, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_SW4_SW1UOMODE);
            break;
                    
        case SW2:
            param.addr = MC13892_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW4_SW2UOMODE, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_SW4_SW2UOMODE);
            break;
                    
        case SW3:
            param.addr = MC13892_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW5_SW3UOMODE, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_SW5_SW3UOMODE);
            break;
                    
        case SW4:
            param.addr = MC13892_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW5_SW4UOMODE, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_SW5_SW4UOMODE);
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
// Function: PmicSwitchModeRegulatorDisableSoftStart
//
// This function is used to disable soft start.
//
// Parameters:
//           regulator [in]       the regulator to be set
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorDisableUserOffMode(PMIC_REGULATOR_SREG regulator)
{
     PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case SW1:
            param.addr = MC13892_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW4_SW1UOMODE, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_SW4_SW1UOMODE);
            break;
                    
        case SW2:
            param.addr = MC13892_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW4_SW2UOMODE, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_SW4_SW2UOMODE);
            break;
                    
        case SW3:
            param.addr = MC13892_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW5_SW3UOMODE, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_SW5_SW3UOMODE);
            break;
                    
        case SW4:
            param.addr = MC13892_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13892_SW5_SW4UOMODE, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_SW5_SW4UOMODE);
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
// Function: PmicSwitchModeRegulatorEnableSIDMode
//
// This function is used to enable SID mode enable
//
// Parameters:
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorEnableSIDMode()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_SW4_ADDR;
    param.data = CSP_BITFVAL(MC13892_SW4_SIDEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_SW4_SIDEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
    sizeof(param), NULL, 0, NULL, NULL)) 
    return PMIC_ERROR;          

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorEnableSIDMode
//
// This function is used to disable SID mode 
//
// Parameters:
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorDisableSIDMode()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_SW4_ADDR;
    param.data = CSP_BITFVAL(MC13892_SW4_SIDEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_SW4_SIDEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
    sizeof(param), NULL, 0, NULL, NULL)) 
    return PMIC_ERROR;          

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorEnablePLL
//
// This function is used to enable Switcher PLL enable
//
// Parameters:
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorEnablePLL()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_SW4_ADDR;
    param.data = CSP_BITFVAL(MC13892_SW4_PLLEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_SW4_PLLEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
    sizeof(param), NULL, 0, NULL, NULL)) 
    return PMIC_ERROR;          

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorDisablePLL
//
// This function is used to disable Switcher PLL 
//
// Parameters:
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorDisablePLL()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_SW4_ADDR;
    param.data = CSP_BITFVAL(MC13892_SW4_PLLEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_SW4_PLLEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
    sizeof(param), NULL, 0, NULL, NULL)) 
    return PMIC_ERROR;          

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}



//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorEnableSWBST
//
// This function is used to enable SWBST 
//
// Parameters:
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorEnableSWBST()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_SW5_ADDR;
    param.data = CSP_BITFVAL(MC13892_SW5_SWBSTEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_SW5_SWBSTEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
    sizeof(param), NULL, 0, NULL, NULL)) 
    return PMIC_ERROR;          

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorDisableSWBST
//
// This function is used to disable SWBST
//
// Parameters:
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorDisableSWBST()
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    param.addr = MC13892_SW5_ADDR;
    param.data = CSP_BITFVAL(MC13892_SW5_SWBSTEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_SW5_SWBSTEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
    sizeof(param), NULL, 0, NULL, NULL)) 
    return PMIC_ERROR;          

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}
  
//------------------------------------------------------------------------------
//
// Function: RequestPmicVoltageRegulatorOn
//
// This function is used to turn on the voltage regulator
//
// Parameters:
//             regulator [in]         which voltage regulator to turn on
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS RequestPmicVoltageRegulatorOn (PMIC_REGULATOR_VREG regulator) 
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case VGEN1:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VGEN1EN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VGEN1EN);
            break;
            
        case VIOHI:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VIOHIEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VIOHIEN);
            break;
    
        case VDIG:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VDIGEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VDIGEN);
            break;
             
        case VGEN2:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VGEN2EN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VGEN2EN);
            break;
       
             
        case VPLL:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VPLLEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VPLLEN);
            break;

        case VUSB2:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VUSB2EN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VUSB2EN);
            break;
             
        case VGEN3:
            param.addr = MC13892_REG_MOD1_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE1_VGEN3EN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VGEN3EN);
            break;
             
        case VCAM:
            param.addr = MC13892_REG_MOD1_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE1_VCAMEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VCAMEN);
            break;
             
        case VVIDEO:
            param.addr = MC13892_REG_MOD1_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE1_VVIDEOEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VVIDEOEN);
            break;

                     
        case VAUDIO:
             param.addr = MC13892_REG_MOD1_ADDR;
             param.data = CSP_BITFVAL(MC13892_REG_MODE1_VAUDIOEN, ENABLE);
             param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VAUDIOEN);
             break;
             
        case VSD1:
             param.addr = MC13892_REG_MOD1_ADDR;
             param.data = CSP_BITFVAL(MC13892_REG_MODE1_VSDEN, ENABLE);
             param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VSDEN);
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
// Function: RequestPmicVoltageRegulatorOff
//
// This function is used to turn off the regulator
//
// Parameters:
//             regulator [in]              which voltage regulator to turn off
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS RequestPmicVoltageRegulatorOff (PMIC_REGULATOR_VREG regulator) 
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case VGEN1:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VGEN1EN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VGEN1EN);
            break;
            
        case VIOHI:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VIOHIEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VIOHIEN);
            break;
    
        case VDIG:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VDIGEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VDIGEN);
            break;
             
        case VGEN2:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VGEN2EN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VGEN2EN);
            break;
       
             
        case VPLL:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VPLLEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VPLLEN);
            break;

        case VUSB2:
            param.addr = MC13892_REG_MOD0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE0_VUSB2EN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VUSB2EN);
            break;
             
        case VGEN3:
            param.addr = MC13892_REG_MOD1_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE1_VGEN3EN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VGEN3EN);
            break;
             
        case VCAM:
            param.addr = MC13892_REG_MOD1_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE1_VCAMEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VCAMEN);
            break;
             
        case VVIDEO:
            param.addr = MC13892_REG_MOD1_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_MODE1_VVIDEOEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VVIDEOEN);
            break;

                     
        case VAUDIO:
             param.addr = MC13892_REG_MOD1_ADDR;
             param.data = CSP_BITFVAL(MC13892_REG_MODE1_VAUDIOEN, DISABLE);
             param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VAUDIOEN);
             break;
             
        case VSD1:
             param.addr = MC13892_REG_MOD1_ADDR;
             param.data = CSP_BITFVAL(MC13892_REG_MODE1_VSDEN, DISABLE);
             param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VSDEN);
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
// Function: PmicVoltageRegulatorOn
//
// This function is used to turn on  the voltage regulator ,the regular will added the power trace for power saving
//
// Parameters:
//             regulator [in]         which voltage regulator to turn on
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicVoltageRegulatorOn(PMIC_REGULATOR_VREG regulator)
{
   
    switch (regulator)
        {
        case VDIG:

            break;
            
        case VPLL:
          
            break;

        default:
            if (regulator >=VMAX) break;
            // Increment power reference count
            if (InterlockedIncrement(&RegRefCount[regulator]) == 1)
                {
                    RequestPmicVoltageRegulatorOn (regulator) ;
                }
            break;
        }
    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicVoltageRegulatorOff
//
// This function is used to turn off   the voltage regulator ,the power will not used for the driver 
//the regular will added the power trace for power saving
//
// Parameters:
//             regulator [in]         which voltage regulator to turn on
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicVoltageRegulatorOff(PMIC_REGULATOR_VREG regulator)
{

    switch (regulator)
        {
        case VDIG:

            break;
            
        case VPLL:
          
            break;

        default:
            if (regulator >=VMAX) break;
            // Increment  power reference count
            if (InterlockedDecrement(&RegRefCount[regulator]) == 0)
                {
                    RequestPmicVoltageRegulatorOff (regulator) ;
                }
            break;
        }
    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicVoltageRegulatorSetVoltageLevel
//
// This function is used to set voltage level for the voltage regulator.
//
// Parameters:
//          regulator [in]     which voltage regulator to be set
//          voltage [in]       the voltage level to set
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageRegulatorSetVoltageLevel (
                                                       PMIC_REGULATOR_VREG regulator, 
                                                       PMIC_REGULATOR_VREG_VOLTAGE voltage ) 
{
    PMIC_PARAM_LLA_WRITE_REG param;
       
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case VIOHI:
            // VIOHI is fixed to 2.775v output.
            return PMIC_SUCCESS;

        case VGEN1:
            param.addr = MC13892_REG_SET0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_SET0_VGEN1, voltage.vgen1);
            param.mask = CSP_BITFMASK(MC13892_REG_SET0_VGEN1);
            break;
             
        case VDIG:
            param.addr = MC13892_REG_SET0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_SET0_VDIG, voltage.vdig);
            param.mask = CSP_BITFMASK(MC13892_REG_SET0_VDIG);
            break;
             
        case VGEN2:
            param.addr = MC13892_REG_SET0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_SET0_VGEN2, voltage.vgen2);
            param.mask = CSP_BITFMASK(MC13892_REG_SET0_VGEN2);
            break;
             
        case VPLL:
            param.addr = MC13892_REG_SET0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_SET0_VPLL, voltage.vpll);
            param.mask = CSP_BITFMASK(MC13892_REG_SET0_VPLL);
            break;
             
        case VUSB2:
            param.addr = MC13892_REG_SET0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_SET0_VUSB2, voltage.vusb2);
            param.mask = CSP_BITFMASK(MC13892_REG_SET0_VUSB2);
            break;
             
        case VGEN3:
            param.addr = MC13892_REG_SET0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_SET0_VGEN3, voltage.vgen3);
            param.mask = CSP_BITFMASK(MC13892_REG_SET0_VGEN3);
            break;

                     
        case VCAM:
            param.addr = MC13892_REG_SET0_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_SET0_VCAM, voltage.vcam);
            param.mask = CSP_BITFMASK(MC13892_REG_SET0_VCAM);
             break;
             
        case VVIDEO:
            param.addr = MC13892_REG_SET1_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_SET1_VVIDEO, voltage.vvideo);
            param.mask = CSP_BITFMASK(MC13892_REG_SET1_VVIDEO);
             break;
             
        case VAUDIO:
            param.addr = MC13892_REG_SET1_ADDR;
            param.data = CSP_BITFVAL(MC13892_REG_SET1_VAUDIO, voltage.vaudio);
            param.mask = CSP_BITFMASK(MC13892_REG_SET1_VAUDIO);
             break;

        case VSD1:
             param.addr = MC13892_REG_SET1_ADDR;
             param.data = CSP_BITFVAL(MC13892_REG_SET1_VSD1, voltage.vsd1);
             param.mask = CSP_BITFMASK(MC13892_REG_SET1_VSD1);
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
// Function: PmicVoltageRegulatorGetVoltageLevel
//
// This function is to get the current voltage settings of the regulator.
//
// Parameters:
//          regulator [in]           which voltage regulator to get from
//          voltage [out]           get voltage value 
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageRegulatorGetVoltageLevel (
                                             PMIC_REGULATOR_VREG regulator, 
                                             PMIC_REGULATOR_VREG_VOLTAGE* voltage)
{
    UINT32 addr, temp = 0;
       
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (voltage == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicVoltageRegulatorGetVoltageLevel:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    if (regulator < VVIDEO)
        addr = MC13892_REG_SET0_ADDR;
    else
        addr = MC13892_REG_SET1_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;
    
    switch (regulator) 
    {
        case VIOHI:
            // VIOHI is fixed to 2.775v output.
             voltage->viohi = VIOHI_2_775;
            break;

        case VGEN1:
             voltage->vgen1= (MC13892_REGULATOR_VREG_VOLTAGE_VGEN1)CSP_BITFEXT(temp, MC13892_REG_SET0_VGEN1);
            break;
             
        case VDIG:
             voltage->vdig = (MC13892_REGULATOR_VREG_VOLTAGE_VDIG)CSP_BITFEXT(temp, MC13892_REG_SET0_VDIG);
            break;
             
        case VGEN2:
             voltage->vgen2 = (MC13892_REGULATOR_VREG_VOLTAGE_VGEN2)CSP_BITFEXT(temp, MC13892_REG_SET0_VGEN2);
            break;
             
        case VPLL:
             voltage->vpll = (MC13892_REGULATOR_VREG_VOLTAGE_VPLL)CSP_BITFEXT(temp, MC13892_REG_SET0_VPLL);
            break;
             
        case VUSB2:
             voltage->vusb2 = (MC13892_REGULATOR_VREG_VOLTAGE_VUSB2)CSP_BITFEXT(temp, MC13892_REG_SET0_VUSB2);
            break;
             
        case VGEN3:
            voltage->vgen3= (MC13892_REGULATOR_VREG_VOLTAGE_VGEN3)CSP_BITFEXT(temp, MC13892_REG_SET0_VGEN3);
            break;

        case VCAM:
            voltage->vcam= (MC13892_REGULATOR_VREG_VOLTAGE_VCAM)CSP_BITFEXT(temp, MC13892_REG_SET0_VCAM);
            break;
             
        case VVIDEO:
            voltage->vvideo= (MC13892_REGULATOR_VREG_VOLTAGE_VVIDEO)CSP_BITFEXT(temp, MC13892_REG_SET1_VVIDEO);
            break;
             
        case VAUDIO:
            voltage->vaudio= (MC13892_REGULATOR_VREG_VOLTAGE_VAUDIO)CSP_BITFEXT(temp, MC13892_REG_SET1_VAUDIO);
             break;

        case VSD1:
            voltage->vsd1 = (MC13892_REGULATOR_VREG_VOLTAGE_VSD1)CSP_BITFEXT(temp, MC13892_REG_SET1_VSD1);
             break;
      
        default:
            return PMIC_PARAMETER_ERROR;
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

  
//------------------------------------------------------------------------------
//
// Function: PmicVolatageRegulatorSetPowerMode
//
// This function is used to set low power mode for the regulator and whether to enter 
// low power mode during STANDBY assertion or not. 
//
// VxMODE=1, Set Low Power no matter of VxSTBY and STANDBY pin
// VxMODE=0, VxSTBY=1, Low Power Mode is contorled by STANDBY pin
// VxMODE=0, VxSTBY=0, Low Power Mode is disabled
//
// Parameters:
//          regulator [in]                which voltage regulator to be set
//          powerMode[in]              the power mode to be set
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageRegulatorSetPowerMode ( 
                                                            PMIC_REGULATOR_VREG regulator, 
                                                            PMIC_REGULATOR_VREG_POWER_MODE powerMode) 
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator)
    {
       case VGEN1:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE0_VGEN1MODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE0_VGEN1STBY, 0);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VGEN1MODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE0_VGEN1STBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE0_VGEN1MODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE0_VGEN1STBY, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VGEN1MODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE0_VGEN1STBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE0_VGEN1MODE, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VGEN1MODE);
                    break;
            }
            break;

        case VIOHI:
            switch(powerMode) {
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE0_VIOHISTBY, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VIOHISTBY);
                    break;

   
            }
            break;
            
        case VDIG:
            switch(powerMode) {
             
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE0_VDIGSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VDIGSTBY);
                    break;
            }
            break;

         case VGEN2:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE0_VGEN2MODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE0_VGEN2STBY, 0);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VGEN2MODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE0_VGEN2STBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE0_VGEN1MODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE0_VGEN2STBY, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VGEN2MODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE0_VGEN2STBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE0_VGEN2MODE, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VGEN2MODE);
                    break;
            }
            break;

         case VPLL:
            switch(powerMode) {

                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE0_VPLLSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VPLLSTBY);
                    break;

           }
            break;   
             
         case VUSB2:
            switch(powerMode) {
             
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE0_VUSB2STBY, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE0_VUSB2STBY);
                    break;
            }
            break;   

         case VGEN3:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13892_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VGEN3MODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE1_VGEN3STBY, 0);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VGEN3MODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE1_VGEN3STBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13892_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VGEN3MODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE1_VGEN3STBY, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VGEN3MODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE1_VGEN3STBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13892_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VGEN3MODE, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VGEN3MODE);
                    break;
            }
            break;

         
        case VCAM:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13892_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VCAMMODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE1_VCAMSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VCAMMODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE1_VCAMSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13892_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VCAMMODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE1_VCAMSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VCAMMODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE1_VCAMSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13892_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VCAMMODE, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VCAMMODE);
                    break;
            }
            break;   
                     
          case VVIDEO:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13892_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VVIDEOMODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE1_VVIDEOSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VVIDEOMODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE1_VVIDEOSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VVIDEOMODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE1_VVIDEOSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VVIDEOMODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE1_VVIDEOSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13892_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VVIDEOMODE, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VVIDEOMODE);
                    break;
            }
            break; 


          case VAUDIO:
            switch(powerMode) {
             
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13892_REG_MOD1_ADDR;
                    param.data =  CSP_BITFVAL(MC13892_REG_MODE1_VAUDIOSTBY, 1);
                    param.mask =  CSP_BITFMASK(MC13892_REG_MODE1_VAUDIOSTBY);
                    break;

            }
            break;   

             case VSD1:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13892_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VSDMODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE1_VSDSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VSDMODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE1_VSDSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13892_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VSDMODE, 0) |
                                         CSP_BITFVAL(MC13892_REG_MODE1_VSDSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VSDMODE) |
                                           CSP_BITFMASK(MC13892_REG_MODE1_VSDSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13892_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13892_REG_MODE1_VSDMODE, 1);
                    param.mask = CSP_BITFMASK(MC13892_REG_MODE1_VSDMODE);
                    break;
            }
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
// Function: PmicVolatageRegulatorGetPowerMode
//
// This function is to get the current power mode for the regulator
//
// Parameters:
//          regulator [in]                      to get which regulator setting
//          powerMode [out]                 the power mode to get
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageRegulatorGetPowerMode ( 
                                             PMIC_REGULATOR_VREG regulator, 
                                             PMIC_REGULATOR_VREG_POWER_MODE* powerMode) 
{
    UINT32 addr, temp = 0;  
    UINT8 lp=0, stby = 0;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (powerMode == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicVoltageRegulatorGetPowerMode:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    if (regulator < VGEN3)
        addr = MC13892_REG_MOD0_ADDR;
    else
        addr = MC13892_REG_MOD1_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    switch (regulator) 
    {
        case VGEN1:
             lp = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE0_VGEN1MODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE0_VGEN1STBY);
            break;
        
        case VIOHI:
             stby = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE0_VIOHISTBY);
            break;
             
        case VDIG:
           
             stby = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE0_VDIGSTBY);
            break;
             
        case VGEN2:
             lp = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE0_VGEN2MODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE0_VGEN2STBY);
            break;
             
        case VPLL:
            
             stby = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE0_VPLLSTBY);
            break;
             
        case VUSB2:
             
             stby = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE0_VUSB2STBY);
            break;
             
        case VGEN3:
             lp = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE1_VGEN3MODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE1_VGEN3STBY);
            break;
             
        case VCAM:
             lp = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE1_VCAMMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE1_VCAMSTBY);
             break;

        case VVIDEO:
             lp = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE1_VVIDEOMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE1_VVIDEOSTBY);
             break;
             
        case VAUDIO:
            
             stby = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE1_VAUDIOSTBY);
             break;
             
        case VSD1:
             lp = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE1_VSDMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13892_REG_MODE1_VSDSTBY);
             break;
             

        default:
        return PMIC_PARAMETER_ERROR;
    }

    if(lp==LOW)
         if(stby==LOW)
             *powerMode = LOW_POWER_DISABLED;
         else 
             *powerMode = LOW_POWER_CTRL_BY_PIN;
    else if (lp==HIGH)
         *powerMode = LOW_POWER;
                
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicVoltageGPOOn
//
// This function is to set GPO voltage on 
//
// Parameters:
//          gpo [in]      to get which gpo
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageGPOOn (MC13892_GPO_SREG gpo)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (gpo) 
    {
        case GPO1:
            param.addr = MC13892_PWR_MISC_ADDR;
            param.data =  CSP_BITFVAL(MC13892_REG_MISC_GPO1EN, ENABLE);
            param.mask =  CSP_BITFMASK(MC13892_REG_MISC_GPO1EN);
            break;
                    
         case GPO2:
            param.addr = MC13892_PWR_MISC_ADDR;
            param.data =  CSP_BITFVAL(MC13892_REG_MISC_GPO2EN, ENABLE);
            param.mask =  CSP_BITFMASK(MC13892_REG_MISC_GPO2EN);
            break;

        case GPO3:
            param.addr = MC13892_PWR_MISC_ADDR;
            param.data =  CSP_BITFVAL(MC13892_REG_MISC_GPO3EN, ENABLE);
            param.mask =  CSP_BITFMASK(MC13892_REG_MISC_GPO3EN);
            break;

        case GPO4:
            param.addr = MC13892_PWR_MISC_ADDR;
            param.data =  CSP_BITFVAL(MC13892_REG_MISC_GPO4EN, ENABLE);
            param.mask =  CSP_BITFMASK(MC13892_REG_MISC_GPO4EN);
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
// Function: PmicVoltageGPOOff
//
// This function is to set GPO voltage off 
//
// Parameters:
//          gpo [in]      to get which gpo
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageGPOOff (MC13892_GPO_SREG gpo)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

 
    switch (gpo) 
    {
        case GPO1:
            param.addr = MC13892_PWR_MISC_ADDR;
            param.data =  CSP_BITFVAL(MC13892_REG_MISC_GPO1EN, DISABLE);
            param.mask =  CSP_BITFMASK(MC13892_REG_MISC_GPO1EN);
            break;
                    
         case GPO2:
            param.addr = MC13892_PWR_MISC_ADDR;
            param.data =  CSP_BITFVAL(MC13892_REG_MISC_GPO2EN, DISABLE);
            param.mask =  CSP_BITFMASK(MC13892_REG_MISC_GPO2EN);
            break;

        case GPO3:
            param.addr = MC13892_PWR_MISC_ADDR;
            param.data =  CSP_BITFVAL(MC13892_REG_MISC_GPO3EN, DISABLE);
            param.mask =  CSP_BITFMASK(MC13892_REG_MISC_GPO3EN);
            break;

        case GPO4:
            param.addr = MC13892_PWR_MISC_ADDR;
            param.data =  CSP_BITFVAL(MC13892_REG_MISC_GPO4EN, DISABLE);
            param.mask =  CSP_BITFMASK(MC13892_REG_MISC_GPO4EN);
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
// Function: PmicVoltagePowerGateOn
//
// This function is to set Power Gate voltage off 
//
// Parameters:
//          gate [in]      to get which gate
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltagePowerGateOn (MC13892_POWER_GATE gate)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    
    if (InterlockedIncrement(&GateRefCount[gate]) == 1)
    {

        switch (gate) 
        {
            case GATE1:
                param.addr =  MC13892_PWR_MISC_ADDR;
                param.data =  CSP_BITFVAL(MC13892_REG_MISC_PWGT1SPIEN, DISABLE);
                param.mask =  CSP_BITFMASK(MC13892_REG_MISC_PWGT1SPIEN);
                break;
                        
            case GATE2:
                param.addr = MC13892_PWR_MISC_ADDR;
                param.data =  CSP_BITFVAL(MC13892_REG_MISC_PWGT2SPIEN, DISABLE);
                param.mask =  CSP_BITFMASK(MC13892_REG_MISC_PWGT2SPIEN);
                break;

                        
            default:
                return PMIC_PARAMETER_ERROR;
        }

        if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL)) 
        return PMIC_ERROR;          

        DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

     }

     return PMIC_SUCCESS;   

}
//------------------------------------------------------------------------------
//
// Function: PmicVoltagePowerGateOff
//
// This function is to set set Power Gate voltage off 
//
// Parameters:
//          gate [in]      to get which Gate
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltagePowerGateOff (MC13892_POWER_GATE gate)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (InterlockedDecrement(&GateRefCount[gate]) == 0)
        {
            switch (gate) 
            {
                case GATE1:
                    param.addr = MC13892_PWR_MISC_ADDR;
                    param.data =  CSP_BITFVAL(MC13892_REG_MISC_PWGT1SPIEN, ENABLE);
                    param.mask =  CSP_BITFMASK(MC13892_REG_MISC_PWGT1SPIEN);
                    break;
                            
                 case GATE2:
                    param.addr = MC13892_PWR_MISC_ADDR;
                    param.data =  CSP_BITFVAL(MC13892_REG_MISC_PWGT2SPIEN, ENABLE);
                    param.mask =  CSP_BITFMASK(MC13892_REG_MISC_PWGT2SPIEN);
                    break;

                            
                default:
                    return PMIC_PARAMETER_ERROR;
            }

            if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
                    sizeof(param), NULL, 0, NULL, NULL)) 
                    return PMIC_ERROR;          

            DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

          
        }
    return  PMIC_SUCCESS;

}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorEnableSTBYDVFS
//
// This function is used to enable the standby or Dynamic Voltage-Frequency scaling.
//
// Parameters:
//             regulator [in]        the regulator to be set
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
// Remarks:
// This function is only applicable to Roadrunner, it is a stub function here.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorEnableSTBYDVFS (PMIC_REGULATOR_SREG regulator)
{
    UNREFERENCED_PARAMETER(regulator);

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorDisableSTBYDVFS
//
// This function is used to disable the standby or Dynamic Voltage-Frequency scaling.
//
// Parameters:
//             regulator [in]        the regulator to be set
//
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
// Remarks:
// This function is only applicable to Roadrunner, it is a stub function here.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorDisableSTBYDVFS (PMIC_REGULATOR_SREG regulator)
{
    UNREFERENCED_PARAMETER(regulator);

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorEnableREGSCPEN
//
// This function is used to Enables Regulator short circuit protection 
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorEnableREGSCPEN (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if(PmicGetVersion()<PMIC_MC13892_VER_20a)
        return PMIC_NOT_SUPPORTED;

    param.addr = MC13892_PWR_MISC_ADDR;
    param.data = CSP_BITFVAL(MC13892_REG_MISC_REGSCPEN, ENABLE);
    param.mask = CSP_BITFMASK(MC13892_REG_MISC_REGSCPEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorDisableREGSCPEN
//
// This function is used to Disables Regulator short circuit protection 
//
// Parameters:
//             none
// 
// Returns:
//          status 
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorDisableREGSCPEN (void)
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if(PmicGetVersion()<PMIC_MC13892_VER_20a)
        return PMIC_NOT_SUPPORTED;

    param.addr = MC13892_PWR_MISC_ADDR;
    param.data = CSP_BITFVAL(MC13892_REG_MISC_REGSCPEN, DISABLE);
    param.mask = CSP_BITFMASK(MC13892_REG_MISC_REGSCPEN);

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_WRITE_REG, &param,
            sizeof(param), NULL, 0, NULL, NULL))
            return PMIC_ERROR;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}

