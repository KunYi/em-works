/*---------------------------------------------------------------------------
* Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
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
typedef enum _MC13892_REGULATOR_SREG{
    SW1 = 0,
    SW2,
    SW3,
    SW4,
} MC13892_REGULATOR_SREG;
typedef MC13892_REGULATOR_SREG PMIC_REGULATOR_SREG;

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
typedef enum _MC13892_REGULATOR_SREG_STBY{
    LOW = 0,
    HIGH,
}MC13892_REGULATOR_SREG_STBY;
typedef MC13892_REGULATOR_SREG_STBY PMIC_REGULATOR_SREG_STBY;


/*************************************************
// switch regulator modes:
//              1. OFF
//              2. PWM mode and no Pulse Skipping    
//              3. PWM mode and pulse Skipping Allowed 
//              4. Low Power PFM mode
**************************************************/
typedef enum _MC13892_REGULATOR_SREG_MODE{
    SW_MODE_OFF,
    SW_MODE_PWM,
    SW_MODE_PWMPS,
    SW_MODE_PFM,    
    SW_MODE_Auto,
}MC13892_REGULATOR_SREG_MODE;
typedef MC13892_REGULATOR_SREG_MODE PMIC_REGULATOR_SREG_MODE;

// linear voltage regulator
typedef enum _MC13892_REGULATOR_VREG{
    VGEN1 = 0,
    VIOHI,
    VDIG,
    VGEN2,
    VPLL,
    VUSB2,
    VGEN3,
    VCAM,
    VVIDEO,
    VAUDIO,
    VSD1,
    VMAX
} MC13892_REGULATOR_VREG;
typedef MC13892_REGULATOR_VREG PMIC_REGULATOR_VREG;


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
typedef enum _MC13892_REGULATOR_VREG_POWER_MODE{
    LOW_POWER_DISABLED = 0,
    LOW_POWER,
    LOW_POWER_CTRL_BY_PIN,
} MC13892_REGULATOR_VREG_POWER_MODE;
typedef MC13892_REGULATOR_VREG_POWER_MODE PMIC_REGULATOR_VREG_POWER_MODE;

typedef enum _MC13892_REGULATOR_VREG_VOLTAGE_VGEN1{
   VGEN1_1_20V = 0,   //output  1.20V,             
   VGEN1_1_50V,       //output   1.50V,          
   VGEN1_2_775V,      //output   2.775V,    
   VGEN1_3_15V,       //output   3.15V,               
} MC13892_REGULATOR_VREG_VOLTAGE_VGEN1;

typedef enum _MC13783_REGULATOR_VREG_VOLTAGE_VIOHI{
    VIOHI_2_775 = 0,   //output  2.775V,          
 } MC13783_REGULATOR_VREG_VOLTAGE_VIOHI;


typedef enum _MC13892_REGULATOR_VREG_VOLTAGE_VDIG{
    VDIG_1_05V = 0,   //output  1.05V,             
    VDIG_1_25V,        //output   1.25V, 
    VDIG_1_65V,        //output   1.65V, 
    VDIG_1_80V,        //output   1.80V,     
} MC13892_REGULATOR_VREG_VOLTAGE_VDIG;

typedef enum _MC13892_REGULATOR_VREG_VOLTAGE_VGEN2{
    VGEN2_1_20V = 0,   //output  1.20V,             
    VGEN2_1_50V,        //output   1.50V,     
    VGEN2_1_60V,        //output   1.60V, 
    VGEN2_1_80V,        //output   1.80V,      
    VGEN2_2_70V,        //output      2.7V,             
    VGEN2_2_80V,        //output   2.8V,          
    VGEN2_3_00V,        //output   3.0V,  
    VGEN2_3_15V,        //output   3.15V,       
 } MC13892_REGULATOR_VREG_VOLTAGE_VGEN2;
 
 typedef enum _MC13892_REGULATOR_VREG_VOLTAGE_VPLL{
     VPLL_1_20V = 0,    //output  1.20V,             
     VPLL_1_25V,        //output   1.25V, 
     VPLL_1_50V,        //output   1.5V, 
     VPLL_1_80V,        //output   1.80V,     
 } MC13892_REGULATOR_VREG_VOLTAGE_VPLL;

 typedef enum _MC13892_REGULATOR_VREG_VOLTAGE_VUSB2{
     VUSB2_2_40V = 0,    //output  2.40V,          
     VUSB2_2_60V,        //output   2.60V,          
     VUSB2_2_70V,        //output   2.70V,          
     VUSB2_2_775V,       //output   2.775V,          
 } MC13892_REGULATOR_VREG_VOLTAGE_VUSB2;

 typedef enum _MC13892_REGULATOR_VREG_VOLTAGE_VGEN3{
    VGEN3_1_80V=0,       //output   1.80V,         
    VGEN3_2_90V,         //output   2.9V,               
 } MC13892_REGULATOR_VREG_VOLTAGE_VGEN3;

 typedef enum _MC13892_REGULATOR_VREG_VOLTAGE_VCAM{          
     VCAM_2_50V=0,      //output   2.50V,          
     VCAM_2_60V,        //output   2.60V,          
     VCAM_2_75V,        //output   2.775V,    
     VCAM_3_00V,        //output   3.0V,
 } MC13892_REGULATOR_VREG_VOLTAGE_VCAM;

typedef enum _MC13892_REGULATOR_VREG_VOLTAGE_VVIDEO{
    VVIDEO_2_70V = 0,   //output  2.70V,          
    VVIDEO_2_775V,      //output   2.775V,          
    VVIDEO_2_50V,       //output   2.50V,          
    VVIDEO_2_60V,       //output   2.60V,          
} MC13892_REGULATOR_VREG_VOLTAGE_VVIDEO;

typedef enum _MC13892_REGULATOR_VREG_VOLTAGE_VAUDIO{
    VAUDIO_2_30V = 0,   //output  2.30V,          
    VAUDIO_2_50V,       //output   2.50V,          
    VAUDIO_2_775V,      //output  2.775V,          
    VAUDIO_3_000V,      //output   3.0V,          
} MC13892_REGULATOR_VREG_VOLTAGE_VAUDIO;

typedef enum _MC13892_REGULATOR_VREG_VOLTAGE_VSD1{
    VSD1_1_80V = 0,    //output  1.80V,             
    VSD1_2_00V,        //output   2.00V,     
    VSD1_2_60V,        //output   2.60V, 
    VSD1_2_70V,        //output   2.70V,              
    VSD1_2_80V,        //output   2.8V,          
    VSD1_2_90V,        //output   2.9V,    
    VSD1_3_00V,        //output   3.0V,     
    VSD1_3_15V,        //output   3.15V,  
} MC13892_REGULATOR_VREG_VOLTAGE_VSD1;

typedef union {
    MC13892_REGULATOR_VREG_VOLTAGE_VGEN1  vgen1;
    MC13783_REGULATOR_VREG_VOLTAGE_VIOHI  viohi;
    MC13892_REGULATOR_VREG_VOLTAGE_VDIG   vdig;
    MC13892_REGULATOR_VREG_VOLTAGE_VGEN2  vgen2;
    MC13892_REGULATOR_VREG_VOLTAGE_VPLL   vpll;
    MC13892_REGULATOR_VREG_VOLTAGE_VUSB2  vusb2;
    MC13892_REGULATOR_VREG_VOLTAGE_VGEN3  vgen3;
    MC13892_REGULATOR_VREG_VOLTAGE_VCAM   vcam;
    MC13892_REGULATOR_VREG_VOLTAGE_VVIDEO vvideo;
    MC13892_REGULATOR_VREG_VOLTAGE_VAUDIO vaudio;
    MC13892_REGULATOR_VREG_VOLTAGE_VSD1   vsd1;
} MC13892_REGULATOR_VREG_VOLTAGE;
typedef MC13892_REGULATOR_VREG_VOLTAGE PMIC_REGULATOR_VREG_VOLTAGE;

typedef enum _MC13892_REGULATOR_ENABLE{
    DISABLE = 0,
    ENABLE = 1,
} MC13892_REGULATOR_ENABLE;


typedef enum _MC13892_GPO_SREG{
    GPO1 = 0,
    GPO2,
    GPO3,
    GPO4,
} MC13892_GPO_SREG;

typedef enum _MC13892_POWER_GATE{
    GATE1 = 0,
    GATE2,
} MC13892_POWER_GATE;


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

PMIC_STATUS PmicSwitchModeRegulatorSetSidLevel (PMIC_REGULATOR_SREG regulator, UINT8 hilevel,UINT8 lowlevel);
PMIC_STATUS PmicSwitchModeRegulatorGetSidLevel(PMIC_REGULATOR_SREG regulator, UINT8* hilevel,UINT8* lowlevel);
PMIC_STATUS PmicSwitchModeRegulatorSetPLLMF ( UINT8 mf);
PMIC_STATUS PmicSwitchModeRegulatorEnableHIRANGE(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorDisableHIRANGE(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorEnableMemoryHoldMode(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorDisableMemoryHoldMode(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorEnableUserOffMode(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorDisableUserOffMode(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorEnableSIDMode();
PMIC_STATUS PmicSwitchModeRegulatorDisableSIDMode();
PMIC_STATUS PmicSwitchModeRegulatorEnablePLL();
PMIC_STATUS PmicSwitchModeRegulatorDisablePLL();
PMIC_STATUS PmicSwitchModeRegulatorEnableSWBST();
PMIC_STATUS PmicSwitchModeRegulatorDisableSWBST();
    
// linear voltage regulator
PMIC_STATUS PmicVoltageRegulatorOn (PMIC_REGULATOR_VREG regulator);
PMIC_STATUS PmicVoltageRegulatorOff (PMIC_REGULATOR_VREG regulator);
PMIC_STATUS PmicVoltageRegulatorSetVoltageLevel (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_VOLTAGE voltage);
PMIC_STATUS PmicVoltageRegulatorGetVoltageLevel (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_VOLTAGE* voltage);
PMIC_STATUS PmicVoltageRegulatorSetPowerMode (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_POWER_MODE powerMode);
PMIC_STATUS PmicVoltageRegulatorGetPowerMode (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_POWER_MODE* powerMode);

PMIC_STATUS PmicVoltageGPOOn (MC13892_GPO_SREG gpo);
PMIC_STATUS PmicVoltageGPOOff (MC13892_GPO_SREG gpo);

PMIC_STATUS PmicVoltagePowerGateOn (MC13892_POWER_GATE gate);
PMIC_STATUS PmicVoltagePowerGateOff (MC13892_POWER_GATE gate);

PMIC_STATUS PmicSwitchModeRegulatorEnableREGSCPEN (void);
PMIC_STATUS PmicSwitchModeRegulatorDisableREGSCPEN (void);




#ifdef __cplusplus
}
#endif

#endif // __PMIC_REGULATOR_H__
