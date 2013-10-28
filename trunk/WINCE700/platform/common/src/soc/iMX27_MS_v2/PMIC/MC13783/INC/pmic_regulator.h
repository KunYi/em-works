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
// File: pmic_regulator.h
//
// Defines the public prototypes used for the PMIC regulator API
//
//------------------------------------------------------------------------------

#ifndef __PMIC_REGULATOR_H__
#define __PMIC_REGULATOR_H__

#include "pmic_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types
// switch mode regulator
typedef enum _MC13783_REGULATOR_SREG{
    SW1A = 0,
    SW1B,
    SW2A,
    SW2B,
    SW3,
} MC13783_REGULATOR_SREG;
typedef MC13783_REGULATOR_SREG PMIC_REGULATOR_SREG;

typedef UINT8 PMIC_REGULATOR_SREG_VOLTAGE;


/*************************************
 * Switch regulator voltage settings type:
 *
 * SW_VOLTAGE_NORMAL
 * SW_VOLTAGE_DVS
 * SW_VOLTAGE_STBY
 *
 *************************************/
typedef enum _RR_REGULATOR_SREG_VOLTAGE_TYPE{
    SW_VOLTAGE_NORMAL=0,
    SW_VOLTAGE_DVS,  
    SW_VOLTAGE_STBY,  
 } RR_REGULATOR_SREG_VOLTAGE_TYPE;
typedef RR_REGULATOR_SREG_VOLTAGE_TYPE PMIC_REGULATOR_SREG_VOLTAGE_TYPE;


// standby input state H/L
typedef enum _MC13783_REGULATOR_SREG_STBY{
    LOW = 0,
    HIGH,
}MC13783_REGULATOR_SREG_STBY;
typedef MC13783_REGULATOR_SREG_STBY PMIC_REGULATOR_SREG_STBY;


/*************************************************
// switch regulator modes:
//              1. OFF
//              2. PWM mode and no Pulse Skipping    
//              3. PWM mode and pulse Skipping Allowed 
//              4. Low Power PFM mode
**************************************************/
typedef enum _MC13783_REGULATOR_SREG_MODE{
    SW_MODE_OFF,
    SW_MODE_PWM,
    SW_MODE_PULSESKIP,
    SW_MODE_PFM,
}MC13783_REGULATOR_SREG_MODE;
typedef MC13783_REGULATOR_SREG_MODE PMIC_REGULATOR_SREG_MODE;

// linear voltage regulator
typedef enum _MC13783_REGULATOR_VREG{
    VIOHI = 0,
    VIOLO,
    VDIG,
    VGEN,
    VRFDIG,
    VRFREF,
    VRFCP,
    VSIM,
    VESIM,
    VCAM,
    V_VIB,
    VRF1,
    VRF2,
    VMMC1,
    VMMC2,
} MC13783_REGULATOR_VREG;
typedef MC13783_REGULATOR_VREG PMIC_REGULATOR_VREG;


/****************************************************************************
// LOW_POWER   
// VxMODE=1, Set Low Power no matter of VxSTBY and STANDBY pin
//
// LOW_POWER_CTL_BY_PIN       
// VxMODE=0, VxSTBY=1, Low Power Mode is contorled by STANDBY pin
//
// LOW_POWER_DISABLED         
// VxMODE=0, VxSTBY=0, Low Power Mode is disabled
*****************************************************************************/
typedef enum _MC13783_REGULATOR_VREG_POWER_MODE{
    LOW_POWER_DISABLED = 0,
    LOW_POWER,
    LOW_POWER_CTRL_BY_PIN,
} MC13783_REGULATOR_VREG_POWER_MODE;
typedef MC13783_REGULATOR_VREG_POWER_MODE PMIC_REGULATOR_VREG_POWER_MODE;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_VIOHI{
    VIOHI_2_775 = 0,   //output  2.775V,          
 } MC13783_REGULATOR_VREG_VOLTAGE_VIOHI;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_VIOLO{
    VIOLO_1_20V = 0,   //output  1.20V,          
    VIOLO_1_30V,        //output   1.30V,          
    VIOLO_1_50V,        //output   1.50V,          
    VIOLO_1_80V,        //output   1.80V,    
} MC13783_REGULATOR_VREG_VOLTAGE_VIOLO;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_VRFDIG{
    VRFDIG_1_20V = 0,   //output  1.20V,          
    VRFDIG_1_50V,        //output   1.50V,          
    VRFDIG_1_80V,        //output   1.80V,          
    VRFDIG_1_875V,        //output   1.875V,    
} MC13783_REGULATOR_VREG_VOLTAGE_VRFDIG;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_VDIG{
    VDIG_1_20V = 0,   //output  1.20V,          
    VDIG_1_30V,        //output   1.30V,          
    VDIG_1_50V,        //output   1.50V,          
    VDIG_1_80V,        //output   1.80V,          
} MC13783_REGULATOR_VREG_VOLTAGE_VDIG;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_VGEN{
    VGEN_1_20V = 0,   //output  1.20V,          
    VGEN_1_30V,        //output   1.30V,          
    VGEN_1_50V,        //output   1.50V,          
    VGEN_1_80V,        //output   1.80V,          
} MC13783_REGULATOR_VREG_VOLTAGE_VGEN;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_VRF{
    VRF2_1_875V = 0,   //output  1.875V,          
    VRF2_2_475V,        //output   2.475V,          
    VRF2_2_700V,        //output   2.700V,          
    VRF2_2_775V,        //output   2.775V,          
} MC13783_REGULATOR_VREG_VOLTAGE_VRF;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_VRFCP{
    VRFCP_2_700V = 0,   //output  2.700V,          
    VRFCP_2_775V,        //output   2.775V,          
} MC13783_REGULATOR_VREG_VOLTAGE_VRFCP;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_VRFREF{
    VRFREF_2_475V = 0,   //output  2.475V,          
    VRFREF_2_600V,        //output   2.600V,          
    VRFREF_2_700V,        //output   2.700V,          
    VRFREF_2_775V,        //output   2.775V,          
} MC13783_REGULATOR_VREG_VOLTAGE_VRFREF;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_CAM{
                          //        1st silicon,  2nd silicon
    VCAM_1 = 0,   //output  1.50V,          1.5V.
    VCAM_2,        //output   1.80V,          1.80V
    VCAM_3,        //output   2.50V,          2.50V
    VCAM_4,        //output   2.80V,          2.55V
    VCAM_5,        //output    -                2.60V
    VCAM_6,        //output    -                2.80V
    VCAM_7,        //output    -                3.00V
    VCAM_8,        //output    -                TBD
} MC13783_REGULATOR_VREG_VOLTAGE_CAM;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_SIM{
    VSIM_1_8V = 0,   //output = 1.80V
    VSIM_2_9V,   //output = 2.90V
} MC13783_REGULATOR_VREG_VOLTAGE_SIM;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_ESIM{
    VESIM_1_8V = 0,   //output = 1.80V
    VESIM_2_9V,   //output = 2.90V
} MC13783_REGULATOR_VREG_VOLTAGE_ESIM;


typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_MMC{
                    //        1st silicon,  2nd silicon
    VMMC_1,  //output    1.60V,           1.60V
    VMMC_2,  //output    1.80V,           1.80V
    VMMC_3,  //output    2.00V,           2.00V
    VMMC_4, //output     2.20V,           2.60V
    VMMC_5,  //output    2.40V,           2.70V
    VMMC_6,  //output    2.60V,           2.80V
    VMMC_7,  //output    2.80V,           2.90V
    VMMC_8,  //output    2.90V,           3.00V
} MC13783_REGULATOR_VREG_VOLTAGE_MMC;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_VIB{
    V_VIB_1_3V = 0,   //output = 1.30V
    V_VIB_1_8V,   //output = 1.80V
    V_VIB_2_0V,   //output = 2.0V
    V_VIB_3_0V,   //output = 3.0V
} MC13783_REGULATOR_VREG_VOLTAGE_VIB;

typedef union {
    MC13783_REGULATOR_VREG_VOLTAGE_VIOHI viohi;
    MC13783_REGULATOR_VREG_VOLTAGE_VIOLO violo;
    MC13783_REGULATOR_VREG_VOLTAGE_VRFDIG vrfdig;
    MC13783_REGULATOR_VREG_VOLTAGE_VDIG vdig;
    MC13783_REGULATOR_VREG_VOLTAGE_VGEN vgen;
    MC13783_REGULATOR_VREG_VOLTAGE_VRF vrf;
    MC13783_REGULATOR_VREG_VOLTAGE_VRFCP vrfcp;
    MC13783_REGULATOR_VREG_VOLTAGE_VRFREF vrfref;
    MC13783_REGULATOR_VREG_VOLTAGE_CAM vcam;
    MC13783_REGULATOR_VREG_VOLTAGE_SIM vsim;
    MC13783_REGULATOR_VREG_VOLTAGE_ESIM vesim;
    MC13783_REGULATOR_VREG_VOLTAGE_MMC vmmc;
    MC13783_REGULATOR_VREG_VOLTAGE_VIB v_vib;
} MC13783_REGULATOR_VREG_VOLTAGE;
typedef MC13783_REGULATOR_VREG_VOLTAGE PMIC_REGULATOR_VREG_VOLTAGE;

typedef enum _MC13783_REGULATOR_ENABLE{
    DISABLE = 0,
    ENABLE = 1,
} MC13783_REGULATOR_ENABLE;


//------------------------------------------------------------------------------
// Functions

//switch mode regulator
PMIC_STATUS PmicSwitchModeRegulatorOn (PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorOff (PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorSetVoltageLevel (PMIC_REGULATOR_SREG regulator, PMIC_REGULATOR_SREG_VOLTAGE_TYPE voltageType, PMIC_REGULATOR_SREG_VOLTAGE voltage);
PMIC_STATUS PmicSwitchModeRegulatorGetVoltageLevel (PMIC_REGULATOR_SREG regulator, PMIC_REGULATOR_SREG_VOLTAGE_TYPE voltageType, PMIC_REGULATOR_SREG_VOLTAGE* voltage);
PMIC_STATUS PmicSwitchModeRegulatorSetMode (PMIC_REGULATOR_SREG regulator,PMIC_REGULATOR_SREG_STBY standby,PMIC_REGULATOR_SREG_MODE mode );
PMIC_STATUS PmicSwitchModeRegulatorGetMode (PMIC_REGULATOR_SREG regulator, PMIC_REGULATOR_SREG_STBY standby,PMIC_REGULATOR_SREG_MODE* mode );
PMIC_STATUS PmicSwitchModeRegulatorEnableSTBYDVFS (PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorDisableSTBYDVFS (PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorSetDVSSpeed (PMIC_REGULATOR_SREG regulator, UINT8 dvsspeed);
PMIC_STATUS PmicSwitchModeRegulatorEnablePanicMode(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorDisablePanicMode(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorEnableSoftStart(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorDisableSoftStart(PMIC_REGULATOR_SREG regulator);
    
// linear voltage regulator
PMIC_STATUS PmicVoltageRegulatorOn (PMIC_REGULATOR_VREG regulator);
PMIC_STATUS PmicVoltageRegulatorOff (PMIC_REGULATOR_VREG regulator);
PMIC_STATUS PmicVoltageRegulatorSetVoltageLevel (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_VOLTAGE voltage);
PMIC_STATUS PmicVoltageRegulatorGetVoltageLevel (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_VOLTAGE* voltage);
PMIC_STATUS PmicVoltageRegulatorSetPowerMode (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_POWER_MODE powerMode);
PMIC_STATUS PmicVoltageRegulatorGetPowerMode (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_POWER_MODE* powerMode);


#ifdef __cplusplus
}
#endif

#endif // __PMIC_REGULATOR_H__
