/*---------------------------------------------------------------------------
* Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

typedef enum _MC13892_PWRCTRL_PWRON{
    PWRON1=0,   
    PWRON2,   
    PWRON3,   
} MC13892_PWRCTRL_PWRON;


typedef enum _MC13892_PWRCTRL_MODES{
    MODES_GROUNDED=0,   
    MODES_RESEVED,   
    MODES_VCOREDIG,
    MODES_VCORE,   
} MC13892_PWRCTRL_MODES;

typedef enum _MC13892_PWRCTRL_I2CS{
    SPI=0,   
    I2C,   
} MC13892_PWRCTRL_I2CS;

typedef enum _MC13892_PWRCTRL_PUMSS_INDEX{
    PUMSS1=0,   
    PUMSS2,   
} MC13892_PWRCTRL_PUMSS_INDEX;

typedef enum _MC13892_PWRCTRL_PUMSS{
    PUMSS_GROUNDED=0,   
    PUMSS_OPEN,   
    PUMSS_VCOREDIG,
    PUMSS_VCORE,   
} MC13892_PWRCTRL_PUMSS;

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
PMIC_STATUS PmicPwrctrlEnableClk32kMCU (void);
PMIC_STATUS PmicPwrctrlDisableClk32kMCU (void);
PMIC_STATUS PmicPwrctrlEnableDRM(void);
PMIC_STATUS PmicPwrctrlDisableDRM (void);
PMIC_STATUS PmicPwrctrlEnableUSEROFFCLK(void);
PMIC_STATUS PmicPwrctrlDisableUSEROFFCLK (void);
PMIC_STATUS PmicPwrctrlEnablePCUTEXPB(void);
PMIC_STATUS PmicPwrctrlDisablePCUTEXPB (void);
PMIC_STATUS PmicPwrctrlEnableUserOffModeWhenDelay (void);
PMIC_STATUS PmicPwrctrlDisableUserOffModeWhenDelay (void);
PMIC_STATUS PmicPwrctrlEnableWarmStart (void);
PMIC_STATUS PmicPwrctrlDisableWarmStart (void);
PMIC_STATUS PmicPwrctrlEnablePWRONRESET(MC13892_PWRCTRL_PWRON PweIndex);
PMIC_STATUS PmicPwrctrlDisablePWRONRESET(MC13892_PWRCTRL_PWRON PweIndex);
PMIC_STATUS PmicPwrctrlSetDebtime(MC13892_PWRCTRL_PWRON PweIndex,UINT8 Debounce);
PMIC_STATUS PmicPwrctrlEnableSTANDBYINV (void);
PMIC_STATUS PmicPwrctrlDisableSTANDBYINV (void);
PMIC_STATUS PmicPwrctrlEnableSTANDBYSECINV (void);
PMIC_STATUS PmicPwrctrlDisableSTANDBYSECINV (void);
PMIC_STATUS PmicPwrctrlEnableWDIRESET (void);
PMIC_STATUS PmicPwrctrlDisableWDIRESET (void);
PMIC_STATUS PmicPwrctrlSetSPIDRV (UINT8 spidrv);
PMIC_STATUS PmicPwrctrlGetSPIDRV (UINT8 spidrv);
PMIC_STATUS PmicPwrctrlSetCLK32KDRV (UINT8 clk32drv);
PMIC_STATUS PmicPwrctrlGetCLK32KDRV (UINT8 clk32drv);
PMIC_STATUS PmicPwrctrlSetSTBYDLY (UINT8 delay);
PMIC_STATUS PmicPwrctrlGetSTBYDLY (UINT8 delay);

PMIC_STATUS PmicPwrctrlGetMODES (MC13892_PWRCTRL_PWRON *modes);
PMIC_STATUS PmicPwrctrlGetI2CS (MC13892_PWRCTRL_I2CS *i2cs); //AAPL To2 not support
PMIC_STATUS PmicPwrctrlGetPUMSS (MC13892_PWRCTRL_PUMSS_INDEX index,MC13892_PWRCTRL_PUMSS *pumss);

PMIC_STATUS PmicPwrctrlEnableTHSEL (void);
PMIC_STATUS PmicPwrctrlDisableTHSEL (void);



#ifdef __cplusplus
}
#endif

#endif // __PMIC_PWRCTRL_H__
