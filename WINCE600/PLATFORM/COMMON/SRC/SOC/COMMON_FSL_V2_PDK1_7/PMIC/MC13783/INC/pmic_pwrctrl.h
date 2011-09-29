/*---------------------------------------------------------------------------
* Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//
// File: pmic_pwrctrl.h
//
// Defines the public prototypes used for the PMIC Power Control and Power Cut API
//
//------------------------------------------------------------------------------

#ifndef __PMIC_PWRCTRL_H__
#define __PMIC_PWRCTRL_H__

#include "pmic_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

// The backup regulators VBKUP1 and VBKUP2 provide two independent low power 
// supplies during memory hold, user off and power cut operation.
typedef enum _MC13783_PWRCTRL_REG_VBKUP{
   VBKUP1,   
   VBKUP2,   
} MC13783_PWRCTRL_REG_VBKUP;

typedef enum _MC13783_PWRCTRL_VBKUP_MODE{
   VBKUP_MODE1,   // Backup Regulator Off in Non Power Cut Modes and Off in Power Cut Modes
   VBKUP_MODE2,   // Backup Regulator Off in Non Power Cut Modes and On in Power Cut Modes
   VBKUP_MODE3,   // Backup Regulator On in Non Power Cut Modes and Off in Power Cut Modes
   VBKUP_MODE4,   // Backup Regulator On in Non Power Cut Modes and On in Power Cut Modes
} MC13783_PWRCTRL_VBKUP_MODE;

/*!
 * This enumeration define all regulator enabled by regen
 */
typedef enum {
        /*! 
         * VAudio 
         */
        REGU_VAUDIO=0,
        /*! 
         * VIOHI 
         */
        REGU_VIOHI,
        /*! 
         * VIOLO 
         */
        REGU_VIOLO,
        /*! 
         * VDIG 
         */
        REGU_VDIG,
        /*! 
         * VGEN 
         */
        REGU_VGEN,
        /*! 
         * VRFDIG 
         */
        REGU_VRFDIG, /*5*/
        /*! 
         * VRFREF 
         */
        REGU_VRFREF,
        /*! 
         * VRFCP
         */
        REGU_VRFCP,
        /*! 
         * VSIM
         */
        REGU_VSIM,
        /*! 
         * VESIM
         */
        REGU_VESIM,
        /*! 
         * VCAM
         */
        REGU_VCAM, /*10*/
        /*! 
         * VRFBG
         */
        REGU_VRFBG,
        /*! 
         * VVIB
         */
        REGU_VVIB,
        /*! 
         * VRF1
         */
        REGU_VRF1,
        /*! 
         * VRF2
         */
        REGU_VRF2,
        /*! 
         * VMMC1
         */
        REGU_VMMC1,
        /*! 
         * VMMC2
         */
        REGU_VMMC2,
        /*! 
         * GPO1
         */
        REGU_GPO1,
        /*! 
         * GPP2
         */
        REGU_GPO2,
        /*! 
         * GPO3
         */
        REGU_GPO3,
        /*! 
         * GPO4
         */
        REGU_GPO4,
        /*! 
         * REGU_NUMBER
         */
        REGU_NUMBER,
} t_regulator;
/*!
 * This tab define bit for regen of all regulator
 */
int   REGULATOR_REGEN_BIT[REGU_NUMBER]={
        0, /* VAUDIO */ 
        1, /* VIOHI  */
        2, /* VIOLO  */
        3, /* VDIG   */
        4, /* VGEN   */
        5, /* VRFDIG */
        6, /* VRFREF */
        7, /* VRFCP  */
        -1,  /* VSIM   */
        -1,  /* VESIM  */
        8, /* VCAM   */
        9, /* VRFBG  */
        -1,  /* VVIB   */
        10, /* VRF1   */
        11, /* VRF2   */
        12, /* VMMC1  */
        13, /* VMMC2  */
        16, /* VGPO1  */
        17, /* VGPO2  */
        18, /* VGPO3  */
        19, /* VGPO4  */
};

//------------------------------------------------------------------------------
// Functions

PMIC_STATUS PmicPwrctrlSetPowerCutTimer (UINT8 duration);
PMIC_STATUS PmicPwrctrlGetPowerCutTimer (UINT8* duration);
PMIC_STATUS PmicPwrctrlEnablePowerCut (void);
PMIC_STATUS PmicPwrctrlDisablePowerCut (void);
PMIC_STATUS PmicPwrctrlSetPowerCutCounter (UINT8 counter);
PMIC_STATUS PmicPwrctrlGetPowerCutCounter (UINT8* counter);
PMIC_STATUS PmicPwrctrlSetPowerCutMaxCounter (UINT8 counter);
PMIC_STATUS PmicPwrctrlGetPowerCutMaxCounter (UINT8* counter);
PMIC_STATUS PmicPwrctrlEnableCounter(void);
PMIC_STATUS PmicPwrctrlDisableCounter (void);
PMIC_STATUS PmicPwrctrlSetMemHoldTimer (UINT8 duration);
PMIC_STATUS PmicPwrctrlGetMemHoldTimer (UINT8* duration);
PMIC_STATUS PmicPwrctrlSetMemHoldTimerAllOn (void);
PMIC_STATUS PmicPwrctrlClearMemHoldTimerAllOn (void);
PMIC_STATUS PmicPwrctrlEnableClk32kMCU (void);
PMIC_STATUS PmicPwrctrlDisableClk32kMCU (void);
PMIC_STATUS PmicPwrctrlEnableUserOffModeWhenDelay (void);
PMIC_STATUS PmicPwrctrlDisableUserOffModeWhenDelay (void);
PMIC_STATUS PmicPwrctrlSetVBKUPRegulator (MC13783_PWRCTRL_REG_VBKUP, MC13783_PWRCTRL_VBKUP_MODE);
PMIC_STATUS PmicPwrctrlSetVBKUPRegulatorVoltage (MC13783_PWRCTRL_REG_VBKUP, UINT8);
PMIC_STATUS PmicPwrctrlEnableWarmStart (void);
PMIC_STATUS PmicPwrctrlDisableWarmStart (void);
PMIC_STATUS PmicPwrctrlEnableRegenAssig (t_regulator regu);
PMIC_STATUS PmicPwrctrlDisableRegenAssig (t_regulator regu);
PMIC_STATUS PmicPwrctrlGetRegenAssig (t_regulator regu , UINT8* value);

#ifdef __cplusplus
}
#endif

#endif // __PMIC_PWRCTRL_H__
