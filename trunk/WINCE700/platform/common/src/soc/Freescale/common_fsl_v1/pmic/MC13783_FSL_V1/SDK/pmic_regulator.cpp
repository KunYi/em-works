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
/*---------------------------------------------------------------------------
* Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
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

#include "socarm_macros.h"
#include "pmic_ioctl.h"
#include "regs.h"
#include "regs_regulator.h"
#include "pmic_regulator.h"

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
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        // SW1X, SW2X will be turn on with different mode.
        // use PmicSwitchModeRegulatorSetMode() function to turn on/off.
        case SW1A:
        case SW2A:      
        case SW1B:
        case SW2B:
            DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorOn:SW1x and SW2x don't turn on/off here\r\n")));
            DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorOn:use PmicSwitchModeRegulatorSetMode() \r\n")));
            return PMIC_PARAMETER_ERROR;

        case SW3:
             param.addr = MC13783_SW5_ADDR;
             param.data =   CSP_BITFVAL(MC13783_SW5_SW3EN, MC13783_SW5_SW3EN_ENABLE);
             param.mask = CSP_BITFMASK(MC13783_SW5_SW3EN);
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
    PMIC_PARAM_LLA_WRITE_REG param;
   
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator)
    {
        // SW1X, SW2X will be turn on with different mode.
        // use PmicSwitchModeRegulatorSetMode() function to turn on/off.
        case SW1A:
        case SW2A:      
        case SW1B:
        case SW2B:
            DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorOn:SW1x and SW2x don't turn on/off here\r\n")));
            DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorOn:use PmicSwitchModeRegulatorSetMode() \r\n")));
            return PMIC_PARAMETER_ERROR;

        case SW3:
             param.addr = MC13783_SW5_ADDR;
             param.data =   CSP_BITFVAL(MC13783_SW5_SW3EN, MC13783_SW5_SW3EN_DISABLE);
             param.mask = CSP_BITFMASK(MC13783_SW5_SW3EN);
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
//                                  0-0x3F (0.900v-2.200v)   
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
    if (voltage > MC13783_SW_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorSetVoltageLevel:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    

    switch (regulator) 
    {
        case SW1A:
            param.addr = MC13783_SW0_ADDR;

            switch (voltageType)
            {
                case SW_VOLTAGE_NORMAL:
                    param.data = CSP_BITFVAL(MC13783_SW0_SW1A, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW0_SW1A);
                    break;
                    
                case SW_VOLTAGE_DVS:
                    param.data = CSP_BITFVAL(MC13783_SW0_SW1ADVS, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW0_SW1ADVS);
                    break;
                    
                case SW_VOLTAGE_STBY:
                    param.data = CSP_BITFVAL(MC13783_SW0_SW1ASTBY, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW0_SW1ASTBY);
                    break;
            }
            break;
                    
        case SW1B:
            param.addr = MC13783_SW1_ADDR;

            switch (voltageType)
            {
                case SW_VOLTAGE_NORMAL:
                    param.data = CSP_BITFVAL(MC13783_SW1_SW1B, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW1_SW1B);
                    break;
                    
                case SW_VOLTAGE_DVS:
                    param.data = CSP_BITFVAL(MC13783_SW1_SW1BDVS, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW1_SW1BDVS);
                    break;
                    
                case SW_VOLTAGE_STBY:
                    param.data = CSP_BITFVAL(MC13783_SW1_SW1BSTBY, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW1_SW1BSTBY);
                    break;
            }
            break;
                    
        case SW2A:
            param.addr = MC13783_SW3_ADDR;

            switch (voltageType)
            {
                case SW_VOLTAGE_NORMAL:
                    param.data = CSP_BITFVAL(MC13783_SW2_SW2A, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW2_SW2A);
                    break;
                    
                case SW_VOLTAGE_DVS:
                    param.data = CSP_BITFVAL(MC13783_SW2_SW2ADVS, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW2_SW2ADVS);
                    break;
                    
                case SW_VOLTAGE_STBY:
                    param.data = CSP_BITFVAL(MC13783_SW2_SW2ASTBY, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW2_SW2ASTBY);
                    break;
            }
            break;
                    
        case SW2B:
            param.addr = MC13783_SW4_ADDR;

            switch (voltageType)
            {
                case SW_VOLTAGE_NORMAL:
                    param.data = CSP_BITFVAL(MC13783_SW3_SW2B, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW3_SW2B);
                    break;
                    
                case SW_VOLTAGE_DVS:
                    param.data = CSP_BITFVAL(MC13783_SW3_SW2BDVS, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW3_SW2BDVS);
                    break;
                    
                case SW_VOLTAGE_STBY:
                    param.data = CSP_BITFVAL(MC13783_SW3_SW2BSTBY, voltage);
                    param.mask = CSP_BITFMASK(MC13783_SW3_SW2BSTBY);
                    break;
            }
            break;
                    
        case SW3:
            // check input parameter
            if (voltage > MC13783_SW3_MAX){
                DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorSetVoltageLevel:Invalid Parameter\r\n")));
                return PMIC_PARAMETER_ERROR;
            }    
            
            param.addr = MC13783_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW5_SW3, voltage);
            param.mask = CSP_BITFMASK(MC13783_SW5_SW3);
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
    UINT32 addr, temp;
       
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (voltage == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorGetVoltageLevel:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    switch (regulator) 
    {
        case SW1A:
            addr = MC13783_SW0_ADDR;

            if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
                      &temp, sizeof(temp), NULL, NULL))
                     return PMIC_ERROR;

            if (voltageType == SW_VOLTAGE_NORMAL)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW0_SW1A);
            else if (voltageType == SW_VOLTAGE_DVS)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW0_SW1ADVS);
            else  // voltageType == SW_VOLTAGE_STBY
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW0_SW1ASTBY);

            break;
 
        case SW1B:
            addr = MC13783_SW1_ADDR;

            if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
                      &temp, sizeof(temp), NULL, NULL))
                     return PMIC_ERROR;

            if (voltageType == SW_VOLTAGE_NORMAL)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW1_SW1B);
            else if (voltageType == SW_VOLTAGE_DVS)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW1_SW1BDVS);
            else //voltageType == SW_VOLTAGE_STBY
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW1_SW1BSTBY);

            break;

        case SW2A:
            addr = MC13783_SW2_ADDR;

            if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
                      &temp, sizeof(temp), NULL, NULL))
                     return PMIC_ERROR;

            if (voltageType == SW_VOLTAGE_NORMAL)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW2_SW2A);
            else if (voltageType == SW_VOLTAGE_DVS)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW2_SW2ADVS);
            else  //voltageType == SW_VOLTAGE_STBY
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW2_SW2ASTBY);

            break;

        case SW2B:
            addr = MC13783_SW3_ADDR;

            if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
                      &temp, sizeof(temp), NULL, NULL))
                     return PMIC_ERROR;

            if (voltageType == SW_VOLTAGE_NORMAL)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW3_SW2B);
            else if (voltageType == SW_VOLTAGE_DVS)
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW3_SW2BDVS);
            else //voltageType == SW_VOLTAGE_STBY
                *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW3_SW2BSTBY);

            break;

        case SW3:
            addr = MC13783_SW5_ADDR;

            if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
                      &temp, sizeof(temp), NULL, NULL))
                     return PMIC_ERROR;

            *voltage = (PMIC_REGULATOR_SREG_VOLTAGE)CSP_BITFEXT(temp, MC13783_SW5_SW3);

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
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (regulator == SW3)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorSetMode:SW3 does not support these modes\r\n")));
        return PMIC_PARAMETER_ERROR;
    }
    
    if (standby == LOW )
    {
        switch (regulator) 
        {
            case SW1A:
                param.addr = MC13783_SW4_ADDR;
                param.data =    CSP_BITFVAL(MC13783_SW4_SW1AMODE, mode);
                param.mask = CSP_BITFMASK(MC13783_SW4_SW1AMODE);
                break;

            case SW1B:
                param.addr = MC13783_SW4_ADDR;
                param.data =    CSP_BITFVAL(MC13783_SW4_SW1BMODE, mode);
                param.mask = CSP_BITFMASK(MC13783_SW4_SW1BMODE);
                break;

            case SW2A:
                param.addr = MC13783_SW5_ADDR;
                param.data =    CSP_BITFVAL(MC13783_SW5_SW2AMODE, mode);
                param.mask = CSP_BITFMASK(MC13783_SW5_SW2AMODE);
                break;

            case SW2B:
                param.addr = MC13783_SW5_ADDR;
                param.data =    CSP_BITFVAL(MC13783_SW5_SW2BMODE, mode);
                param.mask = CSP_BITFMASK(MC13783_SW5_SW2BMODE);
                break;
            default:
                return PMIC_PARAMETER_ERROR;
        }
    }
    else
    {
        switch (regulator) 
        {
            case SW1A:
                param.addr = MC13783_SW4_ADDR;
                param.data =    CSP_BITFVAL(MC13783_SW4_SW1ASTBYMODE, mode);
                param.mask = CSP_BITFMASK(MC13783_SW4_SW1ASTBYMODE);
                break;

            case SW1B:
                param.addr = MC13783_SW4_ADDR;
                param.data =    CSP_BITFVAL(MC13783_SW4_SW1BSTBYMODE, mode);
                param.mask = CSP_BITFMASK(MC13783_SW4_SW1BSTBYMODE);
                break;
                
            case SW2A:
                param.addr = MC13783_SW5_ADDR;
                param.data =    CSP_BITFVAL(MC13783_SW5_SW2ASTBYMODE, mode);
                param.mask = CSP_BITFMASK(MC13783_SW5_SW2ASTBYMODE);
                break;
                
            case SW2B:
                param.addr = MC13783_SW5_ADDR;
                param.data =    CSP_BITFVAL(MC13783_SW5_SW2BSTBYMODE, mode);
                param.mask = CSP_BITFMASK(MC13783_SW5_SW2BSTBYMODE);
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
    UINT32 addr, temp;

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
    else if (regulator <SW2A)
        addr = MC13783_SW4_ADDR;
    else
        addr = MC13783_SW5_ADDR;
    
    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    if (standby == LOW )
    {
        switch (regulator) 
        {
            case SW1A:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13783_SW4_SW1AMODE);
                break;

            case SW1B:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13783_SW4_SW1BMODE);
                break;
                
            case SW2A:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13783_SW5_SW2AMODE);
                break;

            case SW2B:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13783_SW5_SW2BMODE);
                break;
            default:
                return PMIC_PARAMETER_ERROR;
        }
    }
    else
    {
        switch (regulator) 
        {
            case SW1A:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13783_SW4_SW1ASTBYMODE);
                break;

            case SW1B:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13783_SW4_SW1BSTBYMODE);
                break;
                
            case SW2A:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13783_SW5_SW2ASTBYMODE);
                break;

            case SW2B:
                *mode = (PMIC_REGULATOR_SREG_MODE)CSP_BITFEXT(temp, MC13783_SW5_SW2BSTBYMODE);
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
//                         0 - Transition speed is dictated by the current
//                               limit and input -output conditions
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
    if (dvsspeed > MC13783_SW_DVSSPEED_MAX){
        DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorSetDVSSpeed: Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }    

    switch (regulator) 
    {
        case SW1A:
            param.addr = MC13783_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW4_SW1ADVSSPEED, dvsspeed);
            param.mask = CSP_BITFMASK(MC13783_SW4_SW1ADVSSPEED);
            break;
                    
        case SW1B:
            param.addr = MC13783_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW4_SW1BDVSSPEED, dvsspeed);
            param.mask = CSP_BITFMASK(MC13783_SW4_SW1BDVSSPEED);
            break;
                    
        case SW2A:
            param.addr = MC13783_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW5_SW2ADVSSPEED, dvsspeed);
            param.mask = CSP_BITFMASK(MC13783_SW5_SW2ADVSSPEED);
            break;
                    
        case SW2B:
            param.addr = MC13783_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW5_SW2BDVSSPEED, dvsspeed);
            param.mask = CSP_BITFMASK(MC13783_SW5_SW2BDVSSPEED);
            break;
                    
        case SW3:           // Switch 3 don't have dvs speed setting.
            DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorSetDVSSpeed:SW3 don't have dvs speed setting\r\n")));
            return PMIC_PARAMETER_ERROR;
        
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

PMIC_STATUS PmicSwitchModeRegulatorEnablePanicMode(PMIC_REGULATOR_SREG regulator)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case SW1A:
            param.addr = MC13783_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW4_SW1APANIC, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_SW4_SW1APANIC);
            break;
                    
        case SW1B:
            param.addr = MC13783_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW4_SW1BPANIC, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_SW4_SW1BPANIC);
            break;
                    
        case SW2A:
            param.addr = MC13783_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW5_SW2APANIC, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_SW5_SW2APANIC);
            break;
                    
        case SW2B:
            param.addr = MC13783_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW5_SW2BPANIC, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_SW5_SW2BPANIC);
            break;
                    
        case SW3:           // Switch 3 don't have this mode.
            DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorEnablePanicMode:SW3 don't have this mode\r\n")));
            return PMIC_PARAMETER_ERROR;

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

PMIC_STATUS PmicSwitchModeRegulatorDisablePanicMode(PMIC_REGULATOR_SREG regulator)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case SW1A:
            param.addr = MC13783_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW4_SW1APANIC, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_SW4_SW1APANIC);
            break;
                    
        case SW1B:
            param.addr = MC13783_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW4_SW1BPANIC, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_SW4_SW1BPANIC);
            break;
                    
        case SW2A:
            param.addr = MC13783_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW5_SW2APANIC, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_SW5_SW2APANIC);
            break;
                    
        case SW2B:
            param.addr = MC13783_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW5_SW2BPANIC, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_SW5_SW2BPANIC);
            break;
                    
        case SW3:           // Switch 3 don't have this mode.
            DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorDisablePanicMode:SW3 don't have this mode\r\n")));
            return PMIC_PARAMETER_ERROR;
                    
        default:
            break;
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

PMIC_STATUS PmicSwitchModeRegulatorEnableSoftStart(PMIC_REGULATOR_SREG regulator)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case SW1A:
            param.addr = MC13783_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW4_SW1ASFST, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_SW4_SW1ASFST);
            break;
                    
        case SW1B:
            param.addr = MC13783_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW4_SW1BSFST, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_SW4_SW1BSFST);
            break;
                    
        case SW2A:
            param.addr = MC13783_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW5_SW2ASFST, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_SW5_SW2ASFST);
            break;
                    
        case SW2B:
            param.addr = MC13783_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW5_SW2BSFST, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_SW5_SW2BSFST);
            break;
                    
        case SW3:           // Switch 3 don't have this mode.
            DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorEnableSoftStart:SW3 don't have this mode\r\n")));
            return PMIC_PARAMETER_ERROR;
                    
        default:
            break;
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

PMIC_STATUS PmicSwitchModeRegulatorDisableSoftStart(PMIC_REGULATOR_SREG regulator)
{
    PMIC_PARAM_LLA_WRITE_REG param;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case SW1A:
            param.addr = MC13783_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW4_SW1ASFST, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_SW4_SW1ASFST);
            break;
                    
        case SW1B:
            param.addr = MC13783_SW4_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW4_SW1BSFST, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_SW4_SW1BSFST);
            break;
                    
        case SW2A:
            param.addr = MC13783_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW5_SW2ASFST, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_SW5_SW2ASFST);
            break;
                    
        case SW2B:
            param.addr = MC13783_SW5_ADDR;
            param.data = CSP_BITFVAL(MC13783_SW5_SW2BSFST, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_SW5_SW2BSFST);
            break;
                    
        case SW3:           // Switch 3 don't have this mode.
            DEBUGMSG(ZONE_ERROR, (_T("PmicSwitchModeRegulatorDisableSoftStart:SW3 don't have this mode\r\n")));
            return PMIC_PARAMETER_ERROR;
                    
        default:
            break;
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
PMIC_STATUS PmicVoltageRegulatorOn (PMIC_REGULATOR_VREG regulator) 
{
    PMIC_PARAM_LLA_WRITE_REG param;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case VIOHI:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VIOHIEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VIOHIEN);
            break;

        case VIOLO:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VIOLOEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VIOLOEN);
            break;
             
        case VDIG:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VDIGEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VDIGEN);
            break;
             
        case VGEN:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VGENEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VGENEN);
            break;
             
        case VRFDIG:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VRFDIGEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFDIGEN);
            break;
             
        case VRFREF:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VRFREFEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFREFEN);
            break;
             
        case VRFCP:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VRFCPEN, ENABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFCPEN);
            break;

                     
        case VSIM:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VSIMEN, ENABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VSIMEN);
             break;
             
        case VESIM:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VESIMEN, ENABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VESIMEN);
             break;
             
        case VCAM:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VCAMEN, ENABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VCAMEN);
             break;

        case V_VIB:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VVIBEN, ENABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VVIBEN);
             break;

        case VRF1:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VRF1EN, ENABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VRF1EN);
             break;
        case VRF2:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VRF2EN, ENABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VRF2EN);
             break;
        case VMMC1:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VMMC1EN, ENABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VMMC1EN);
             break;
        case VMMC2:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VMMC2EN, ENABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VMMC2EN);
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
// Function: PmicVoltageRegulatorOff
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
PMIC_STATUS PmicVoltageRegulatorOff (PMIC_REGULATOR_VREG regulator) 
{
    PMIC_PARAM_LLA_WRITE_REG param;
   
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (regulator) 
    {
        case VIOHI:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VIOHIEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VIOHIEN);
            break;
             
        case VIOLO:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VIOLOEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VIOLOEN);
            break;

        case VDIG:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VDIGEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VDIGEN);
            break;
             
        case VGEN:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VGENEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VGENEN);
            break;
             
        case VRFDIG:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VRFDIGEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFDIGEN);
            break;
             
        case VRFREF:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VRFREFEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFREFEN);
            break;
             
        case VRFCP:
            param.addr = MC13783_REG_MOD0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_MODE0_VRFCPEN, DISABLE);
            param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFCPEN);
            break;

                     
        case VSIM:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VSIMEN, DISABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VSIMEN);
             break;
             
        case VESIM:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VESIMEN, DISABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VESIMEN);
             break;
             
        case VCAM:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VCAMEN, DISABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VCAMEN);
             break;

        case V_VIB:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VVIBEN, DISABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VVIBEN);
             break;

        case VRF1:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VRF1EN, DISABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VRF1EN);
             break;
        case VRF2:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VRF2EN, DISABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VRF2EN);
             break;
        case VMMC1:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VMMC1EN, DISABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VMMC1EN);
             break;
        case VMMC2:
             param.addr = MC13783_REG_MOD1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_MODE1_VMMC2EN, DISABLE);
             param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VMMC2EN);
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

        case VIOLO:
            param.addr = MC13783_REG_SET0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_SET0_VIOLO, voltage.violo);
            param.mask = CSP_BITFMASK(MC13783_REG_SET0_VIOLO);
            break;
             
        case VDIG:
            param.addr = MC13783_REG_SET0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_SET0_VDIG, voltage.vdig);
            param.mask = CSP_BITFMASK(MC13783_REG_SET0_VDIG);
            break;
             
        case VGEN:
            param.addr = MC13783_REG_SET0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_SET0_VGEN, voltage.vgen);
            param.mask = CSP_BITFMASK(MC13783_REG_SET0_VGEN);
            break;
             
        case VRFDIG:
            param.addr = MC13783_REG_SET0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_SET0_VRFDIG, voltage.vrfdig);
            param.mask = CSP_BITFMASK(MC13783_REG_SET0_VRFDIG);
            break;
             
        case VRFREF:
            param.addr = MC13783_REG_SET0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_SET0_VRFREF, voltage.vrfref);
            param.mask = CSP_BITFMASK(MC13783_REG_SET0_VRFREF);
            break;
             
        case VRFCP:
            param.addr = MC13783_REG_SET0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_SET0_VRFCP, voltage.vrfcp);
            param.mask = CSP_BITFMASK(MC13783_REG_SET0_VRFCP);
            break;

                     
        case VSIM:
            param.addr = MC13783_REG_SET0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_SET0_VSIM, voltage.vsim);
            param.mask = CSP_BITFMASK(MC13783_REG_SET0_VSIM);
             break;
             
        case VESIM:
            param.addr = MC13783_REG_SET0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_SET0_VESIM, voltage.vesim);
            param.mask = CSP_BITFMASK(MC13783_REG_SET0_VESIM);
             break;
             
        case VCAM:
            param.addr = MC13783_REG_SET0_ADDR;
            param.data =    CSP_BITFVAL(MC13783_REG_SET0_VCAM, voltage.vcam);
            param.mask = CSP_BITFMASK(MC13783_REG_SET0_VCAM);
             break;

        case V_VIB:
             param.addr = MC13783_REG_SET1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_SET1_VVIB, voltage.v_vib);
             param.mask = CSP_BITFMASK(MC13783_REG_SET1_VVIB);
             break;

        case VRF1:
             param.addr = MC13783_REG_SET1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_SET1_VRF1, voltage.vrf);
             param.mask = CSP_BITFMASK(MC13783_REG_SET1_VRF1);
             break;
             
        case VRF2:
             param.addr = MC13783_REG_SET1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_SET1_VRF2, voltage.vrf);
             param.mask = CSP_BITFMASK(MC13783_REG_SET1_VRF2);
             break;
             
        case VMMC1:
             param.addr = MC13783_REG_SET1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_SET1_VMMC1, voltage.vmmc);
             param.mask = CSP_BITFMASK(MC13783_REG_SET1_VMMC1);
             break;
             
        case VMMC2:
             param.addr = MC13783_REG_SET1_ADDR;
             param.data =   CSP_BITFVAL(MC13783_REG_SET1_VMMC2, voltage.vmmc);
             param.mask = CSP_BITFMASK(MC13783_REG_SET1_VMMC2);
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

    if (regulator < V_VIB)
        addr = MC13783_REG_SET0_ADDR;
    else
        addr = MC13783_REG_SET1_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;
    
    switch (regulator) 
    {
        case VIOHI:
            // VIOHI is fixed to 2.775v output.
             voltage->viohi = VIOHI_2_775;
            break;

        case VIOLO:
             voltage->violo = (MC13783_REGULATOR_VREG_VOLTAGE_VIOLO)CSP_BITFEXT(temp, MC13783_REG_SET0_VIOLO);
            break;
             
        case VDIG:
             voltage->vdig = (MC13783_REGULATOR_VREG_VOLTAGE_VDIG)CSP_BITFEXT(temp, MC13783_REG_SET0_VDIG);
            break;
             
        case VGEN:
             voltage->vgen = (MC13783_REGULATOR_VREG_VOLTAGE_VGEN)CSP_BITFEXT(temp, MC13783_REG_SET0_VGEN);
            break;
             
        case VRFDIG:
             voltage->vrfdig = (MC13783_REGULATOR_VREG_VOLTAGE_VRFDIG)CSP_BITFEXT(temp, MC13783_REG_SET0_VRFDIG);
            break;
             
        case VRFREF:
             voltage->vrfref = (MC13783_REGULATOR_VREG_VOLTAGE_VRFREF)CSP_BITFEXT(temp, MC13783_REG_SET0_VRFREF);
            break;
             
        case VRFCP:
            voltage->vrfcp = (MC13783_REGULATOR_VREG_VOLTAGE_VRFCP)CSP_BITFEXT(temp, MC13783_REG_SET0_VRFCP);
            break;

        case VSIM:
            voltage->vsim = (MC13783_REGULATOR_VREG_VOLTAGE_SIM)CSP_BITFEXT(temp, MC13783_REG_SET0_VSIM);
            break;
             
        case VESIM:
            voltage->vesim = (MC13783_REGULATOR_VREG_VOLTAGE_ESIM)CSP_BITFEXT(temp, MC13783_REG_SET0_VESIM);
            break;
             
        case VCAM:
            voltage->vcam = (MC13783_REGULATOR_VREG_VOLTAGE_CAM)CSP_BITFEXT(temp, MC13783_REG_SET0_VCAM);
             break;

        case V_VIB:
            voltage->v_vib = (MC13783_REGULATOR_VREG_VOLTAGE_VIB)CSP_BITFEXT(temp, MC13783_REG_SET1_VVIB);
             break;

        case VRF1:
            voltage->vrf = (MC13783_REGULATOR_VREG_VOLTAGE_VRF)CSP_BITFEXT(temp, MC13783_REG_SET1_VRF1);
             break;
             
        case VRF2:
            voltage->vrf =(MC13783_REGULATOR_VREG_VOLTAGE_VRF)CSP_BITFEXT(temp, MC13783_REG_SET1_VRF2);
             break;
             
        case VMMC1:
            voltage->vmmc = (MC13783_REGULATOR_VREG_VOLTAGE_MMC)CSP_BITFEXT(temp, MC13783_REG_SET1_VMMC1);
             break;
             
        case VMMC2:
            voltage->vmmc = (MC13783_REGULATOR_VREG_VOLTAGE_MMC)CSP_BITFEXT(temp, MC13783_REG_SET1_VMMC2);
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
        case VIOHI:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VIOHIMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VIOHISTBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VIOHIMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VIOHISTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VIOHIMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VIOHISTBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VIOHIMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VIOHISTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VIOHIMODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VIOHIMODE);
                    break;
            }
            break;
            
        case VIOLO:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VIOLOMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VIOLOSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VIOLOMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VIOLOSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VIOLOMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VIOLOSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VIOLOMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VIOLOSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VIOLOMODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VIOLOMODE);
                    break;
            }
            break;

        case VDIG:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VDIGMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VDIGSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VDIGMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VDIGSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VDIGMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VDIGSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VDIGMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VDIGSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VDIGMODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VDIGMODE);
                    break;
            }
            break;   

        case VGEN:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VGENMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VGENSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VGENMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VGENSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VGENMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VGENSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VGENMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VGENSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VGENMODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VGENMODE);
                    break;
            }
            break;   
             
        case VRFDIG:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VRFDIGMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VRFDIGSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFDIGMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VRFDIGSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VRFDIGMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VRFDIGSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFDIGMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VRFDIGSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VRFDIGMODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFDIGMODE);
                    break;
            }
            break;   
         
        case VRFREF:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VRFREFMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VRFREFSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFREFMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VRFREFSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VRFREFMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VRFREFSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFREFMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VRFREFSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VRFREFMODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFREFMODE);
                    break;
            }
            break;   
             
        case VRFCP:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VRFCPMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VRFCPSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFCPMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VRFCPSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VRFCPMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE0_VRFCPSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFCPMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE0_VRFCPSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD0_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE0_VRFCPMODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE0_VRFCPMODE);
                    break;
            }
            break;   

        case VSIM:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VSIMMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VSIMSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VSIMMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VSIMSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VSIMMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VSIMSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VSIMMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VSIMSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VSIMMODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VSIMMODE);
                    break;
            }
            break;   

        case VESIM:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VESIMMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VESIMSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VESIMMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VESIMSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VESIMMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VESIMSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VESIMMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VESIMSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VESIMMODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VESIMMODE);
                    break;
            }
            break;   

        case VCAM:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VCAMMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VCAMSTBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VCAMMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VCAMSTBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VCAMMODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VCAMSTBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VCAMMODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VCAMSTBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VCAMMODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VCAMMODE);
                    break;
            }
            break;   
                     
        case V_VIB:
            //V_VIB has no low power mode and no standby control
            DEBUGMSG(ZONE_ERROR, (_T("PmicVoltageRegulatorSetPowerMode: VIB don't have these modes\r\n")));
            return PMIC_PARAMETER_ERROR;

        case VRF1:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VRF1MODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VRF1STBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VRF1MODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VRF1STBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VRF1MODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VRF1STBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VRF1MODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VRF1STBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VRF1MODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VRF1MODE);
                    break;
            }
            break;   

        case VRF2:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VRF2MODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VRF2STBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VRF2MODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VRF2STBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VRF2MODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VRF2STBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VRF2MODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VRF2STBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VRF2MODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VRF2MODE);
                    break;
            }
            break;   

        case VMMC1:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VMMC1MODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VMMC1STBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VMMC1MODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VMMC1STBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VMMC1MODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VMMC1STBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VMMC1MODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VMMC1STBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VMMC1MODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VMMC1MODE);
                    break;
            }
            break;   
             
        case VMMC2:
            switch(powerMode) {
                case LOW_POWER_DISABLED:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VMMC2MODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VMMC2STBY, 0);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VMMC2MODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VMMC2STBY);
                    break;
                    
                case LOW_POWER_CTRL_BY_PIN:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VMMC2MODE, 0) |
                                         CSP_BITFVAL(MC13783_REG_MODE1_VMMC2STBY, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VMMC2MODE) |
                                           CSP_BITFMASK(MC13783_REG_MODE1_VMMC2STBY);
                    break;

                case LOW_POWER:
                    param.addr = MC13783_REG_MOD1_ADDR;
                    param.data = CSP_BITFVAL(MC13783_REG_MODE1_VMMC2MODE, 1);
                    param.mask = CSP_BITFMASK(MC13783_REG_MODE1_VMMC2MODE);
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
    UINT8 lp, stby = 0;
    
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if (powerMode == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (_T("PmicVoltageRegulatorGetPowerMode:Invalid Parameter\r\n")));
        return PMIC_PARAMETER_ERROR;
    }

    if (regulator < VSIM)
        addr = MC13783_REG_MOD0_ADDR;
    else
        addr = MC13783_REG_MOD1_ADDR;

    if(!DeviceIoControl(hPMI, PMIC_IOCTL_LLA_READ_REG, &addr, sizeof(addr),
              &temp, sizeof(temp), NULL, NULL))
        return PMIC_ERROR;

    switch (regulator) 
    {
        case VIOHI:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VIOHIMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VIOHISTBY);
            break;

        case VIOLO:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VIOLOMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VIOLOSTBY);
            break;
             
        case VDIG:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VDIGMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VDIGSTBY);
            break;
             
        case VGEN:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VGENMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VGENSTBY);
            break;
             
        case VRFDIG:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VRFDIGMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VRFDIGSTBY);
            break;
             
        case VRFREF:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VRFREFMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VRFREFSTBY);
            break;
             
        case VRFCP:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VRFCPMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE0_VRFCPSTBY);
            break;

        case VSIM:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VSIMMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VSIMSTBY);
            break;
             
        case VESIM:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VESIMMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VESIMSTBY);
            break;
             
        case VCAM:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VCAMMODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VCAMSTBY);
             break;

        case V_VIB: // V_VIB don't have low power mode
            DEBUGMSG(ZONE_ERROR, (_T("PmicVoltageRegulatorGetPowerMode: VIB don't have these modes\r\n")));
            return PMIC_PARAMETER_ERROR;

        case VRF1:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VRF1MODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VRF1STBY);
             break;
             
        case VRF2:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VRF2MODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VRF2STBY);
             break;
             
        case VMMC1:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VMMC1MODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VMMC1STBY);
             break;
             
        case VMMC2:
             lp = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VMMC2MODE);
             stby = (UINT8)CSP_BITFEXT(temp, MC13783_REG_MODE1_VMMC2STBY);
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
