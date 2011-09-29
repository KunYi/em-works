//------------------------------------------------------------------------------
// Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  power.h
//
//   Header file for power module
//
//------------------------------------------------------------------------------

#ifndef __POWER_H__
#define __POWER_H__

#include "hw_lradc.h"

#ifdef __cplusplus
extern "C" {
#endif




//-----------------------------------------------------------------------------
// Defines
#define DCFUNCV_CONST_160 160
#define DCFUNCV_CONST_1000 1000
#define DCFUNCV_MAX_VALUE 1023
#define BATT_BRWNOUT_LIION_BASE_MV 2800
#define BATT_BRWNOUT_LIION_CEILING_OFFSET_MV 39
#define BATT_BRWNOUT_LIION_LEVEL_STEP_MV 40
#define BATT_BRWNOUT_LIION_EQN_CONST (BATT_BRWNOUT_LIION_BASE_MV - BATT_BRWNOUT_LIION_CEILING_OFFSET_MV)
#define BATT_BRWNOUT_ALKAL_BASE_MV 800
#define BATT_BRWNOUT_ALKAL_CEILING_OFFSET_MV 19
#define BATT_BRWNOUT_ALKAL_LEVEL_STEP_MV 20
#define BATT_BRWNOUT_ALKAL_EQN_CONST (BATT_BRWNOUT_ALKAL_BASE_MV - BATT_BRWNOUT_ALKAL_CEILING_OFFSET_MV)
#define BATT_VOLTAGE_8_MV 8
#define POWERDOWN_KEY 0x3e77
#define DIE_TEMP_CHAN_0 LRADC_CH2
#define DIE_TEMP_CHAN_1 LRADC_CH3
#define POWER_MAX_VOLTAGE_STEP_MV     100    // in millivolts

typedef enum _POWER_VOLTAGEVALUES
{
    VOLTAGE_STEP_MV     = 25,
    BO_MIN_OFFSET_MV    = 25,
    BO_MAX_OFFSET_MV    = VOLTAGE_STEP_MV * 7,
    VDDD_MARGIN_STEPS   = 4,
    VDDD_TO_BO_MARGIN  = (VDDD_MARGIN_STEPS * VOLTAGE_STEP_MV),
    VDDD_DEFAULT_MV    = 1200,
    VDDD_DEFAULT_BO    = (VDDD_DEFAULT_MV - VDDD_TO_BO_MARGIN),
    VDDD_ABS_MIN_MV    = 800,
    VDDD_ABS_MAX_MV    = 1575,
    VDDD_SAFE_MAX_MV   = 1575,
    VDDD_SAFE_MIN_MV   = (VDDD_ABS_MIN_MV + BO_MIN_OFFSET_MV),
    VDDD_ALKALINE_MAX_MV = 1300,
    VDDD_BO_MIN_MV     = VDDD_ABS_MIN_MV,
    VDDD_BO_MAX_MV     = (VDDD_SAFE_MAX_MV - VDDD_TO_BO_MARGIN),
    VDDD_BASE_MV       = VDDD_ABS_MIN_MV,
    VDDA_MARGIN_STEPS   = 4,
    VDDA_TO_BO_MARGIN  = (VDDA_MARGIN_STEPS * VOLTAGE_STEP_MV),
    VDDA_DEFAULT_MV    = 1750,
    VDDA_DEFAULT_BO    = (VDDA_DEFAULT_MV - VDDA_TO_BO_MARGIN),
    VDDA_ABS_MIN_MV    = 1500,
    VDDA_ABS_MAX_MV    = 2275,
    VDDA_SAFE_MAX_MV   = 1950,
    VDDA_SAFE_MIN_MV   = (VDDA_ABS_MIN_MV + BO_MIN_OFFSET_MV),
    VDDA_BO_MIN_MV     = VDDA_ABS_MIN_MV,
    VDDA_BO_MAX_MV     = (VDDA_SAFE_MAX_MV - VDDA_TO_BO_MARGIN),
    VDDA_BASE_MV       = VDDA_ABS_MIN_MV,

    VDDIO_MARGIN_STEPS  = 4,
    VDDIO_TO_BO_MARGIN  = (VDDIO_MARGIN_STEPS * VOLTAGE_STEP_MV),
    VDDIO_DEFAULT_MV    = 3100,
    VDDIO_DEFAULT_BO    = (VDDIO_DEFAULT_MV - VDDIO_TO_BO_MARGIN),
    VDDIO_ABS_MIN_MV    = 2800,
    VDDIO_ABS_MAX_MV    = 3575,
    VDDIO_SAFE_MAX_MV   = 3575,
    VDDIO_SAFE_MIN_MV   = (VDDIO_ABS_MIN_MV + BO_MIN_OFFSET_MV),
    VDDIO_BO_MIN_MV     = VDDIO_ABS_MIN_MV,
    VDDIO_BO_MAX_MV     = (VDDIO_SAFE_MAX_MV - VDDIO_TO_BO_MARGIN),
    VDDIO_BASE_MV       = VDDIO_ABS_MIN_MV,
} POWER_VOLTAGEVALUES;

//Threshold values for Vbus valid comparator
typedef enum _POWER_VBUSVALIDTHRESH
{
    VBUS_VALID_THRESH_4400_4210 = 0,
    VBUS_VALID_THRESH_4170_4000 = 1,
    VBUS_VALID_THRESH_2500_2450 = 2,
    VBUS_VALID_THRESH_4730_4480 = 3,
    VBUS_VALID_THRESH_MAX       = 3,

    VBUS_VALID_THRESH_NORMAL = VBUS_VALID_THRESH_4400_4210,
    VBUS_VALID_THRESH_LOW    = VBUS_VALID_THRESH_4170_4000,
    VBUS_VALID_THRESH_HIGH   = VBUS_VALID_THRESH_4730_4480,
    VBUS_VALID_THRESH_TEST   = VBUS_VALID_THRESH_2500_2450
} POWER_VBUSVALIDTHRESH;

//Battery mode enumeration
typedef enum _POWER_BATTERYMODE
{
    HW_POWER_BATT_MODE_LIION  = 0,
    HW_POWER_BATT_MODE_ALKALINE_NIMH = 1,

    // We will remove these later.  They are redundant or obsolete.
    HW_POWER_BATT_MODE_0                      = 0,
    HW_POWER_BATT_MODE_LIION_DUAL_CONVERTOR   = 0,
    HW_POWER_BATT_MODE_1                      = 1,
    HW_POWER_BATT_MODE_LIION_SINGLE_INDUCTOR  = 1,
    HW_POWER_BATT_MODE_2                      = 2,
    HW_POWER_BATT_MODE_SERIES_AA_AAA          = 2,
    HW_POWER_BATT_MODE_DUAL_ALKALINE_NIMH     = 2,
    HW_POWER_BATT_MODE_3                      = 3,
    HW_POWER_BATT_MODE_SINGLE_AA              = 3,
    HW_POWER_BATT_MODE_SINGLE_ALKALINE_NIMH   = 3,
    HW_PWR_BATT_MODE_0                      = 0,
    HW_PWR_BATT_MODE_LIION_DUAL_CONVERTOR   = 0,
    HW_PWR_BATT_MODE_1                      = 1,
    HW_PWR_BATT_MODE_LIION_SINGLE_INDUCTOR  = 1,
    HW_PWR_BATT_MODE_2                      = 2,
    HW_PWR_BATT_MODE_SERIES_AA_AAA          = 2,
    HW_PWR_BATT_MODE_DUAL_ALKALINE_NIMH     = 2,
    HW_PWR_BATT_MODE_3                      = 3,
    HW_PWR_BATT_MODE_SINGLE_AA              = 3,
    HW_PWR_BATT_MODE_SINGLE_ALKALINE_NIMH   = 3
} POWER_BATTERYMODE;

//Possible power sources for each power supply
typedef enum _POWER_POWERSOURCE
{
    //Powered by linear regulator.  DCDC output is gated off and
    //the linreg output is equal to the target.
    HW_POWER_LINREG_DCDC_OFF,
    HW_POWER_LINREG_DCDC_READY,
    HW_POWER_DCDC_LINREG_ON,
    HW_POWER_DCDC_LINREG_OFF,
    HW_POWER_DCDC_LINREG_READY,
    HW_POWER_EXTERNAL_SOURCE_5V,
    HW_POWER_EXTERNAL_SOURCE_BATTERY,
    HW_POWER_UNKNOWN_SOURCE,
    HW_POWER_DCDC1 = 1,
    HW_POWER_DCDC2 = 2,
    HW_POWER_LINEAR_REGULATOR = HW_POWER_LINREG_DCDC_OFF
} POWER_POWERSOURCE;

//Selects the source of the bias current
typedef enum _POWER_BIASCURRENTSOURCE
{
    HW_POWER_EXTERNAL_BIAS_CURRENT = 0x0,
    HW_POWER_INTERNAL_BIAS_CURRENT = 0x1
} POWER_BIASCURRENTSOURCE;

//Battery charging current magnitudes converted to register settings.
typedef enum _POWER_BATTCHARGECURRENTMAG
{
    HW_POWER_CURRENT_MAG_400 = 0x20,
    HW_POWER_CURRENT_MAG_200 = 0x10,
    HW_POWER_CURRENT_MAG_100 = 0x08,
    HW_POWER_CURRENT_MAG_50  = 0x04,
    HW_POWER_CURRENT_MAG_20  = 0x02,
    HW_POWER_CURRENT_MAG_10  = 0x01
} POWER_BATTCHARGECURRENTMAG;

//Linear regulator offset values
typedef enum _POWER_LINREGOFFSETSTEP
{
    HW_POWER_LINREG_OFFSET_NO_STEP    = 0,
    HW_POWER_LINREG_OFFSET_STEP_ABOVE = 1,
    HW_POWER_LINREG_OFFSET_STEP_BELOW = 2,
    HW_POWER_LINREG_OFFSET_MAX        = 3,
    HW_POWER_LINREG_OFFSET_LINREG_MODE = HW_POWER_LINREG_OFFSET_NO_STEP,
    HW_POWER_LINREG_OFFSET_DCDC_MODE = HW_POWER_LINREG_OFFSET_STEP_BELOW
} POWER_LINREGOFFSETSTEP;

//Possible RC Scale values to increase transient load response
typedef enum _POWER_RCSCALELEVELS
{
    HW_POWER_RCSCALE_DISABLED   = 0,
    HW_POWER_RCSCALE_2X_INCR    = 1,
    HW_POWER_RCSCALE_4X_INCR    = 2,
    HW_POWER_RCSCALE_8X_INCR    = 3
} POWER_RCSCALELEVELS;

//Interrupt polarities for power hardware
typedef enum _POWER_INTERRUPTPOLARITY
{
    HW_POWER_CHECK_5V_DISCONNECTED      = 0,
    HW_POWER_CHECK_5V_CONNECTED         = 1,
    HW_POWER_CHECK_LINEAR_REGULATORS_OK = 1,
    HW_POWER_CHECK_PSWITCH_HIGH         = 1,
    HW_POWER_CHECK_PSWITCH_LOW          = 0
} POWER_INTERRUPTPOLARITY;

//Possible sources for Pswitch IRQ source
typedef enum _POWER_PSWITCHIRQSOURCE
{
    HW_POWER_STS_PSWITCH_BIT_0,
    HW_POWER_STS_PSWITCH_BIT_1
} POWER_PSWITCHIRQSOURCE;

//Possible 5V detection methods
typedef enum _POWER_5VDETECTION
{
    //! \brief VBUSVALID will be used for 5V/USB detection.
    DDI_POWER_VBUSVALID,
    //! \brief VDD5V_GT_VDDIO will be used for 5V/USB detection.
    DDI_POWER_VDD5V_GT_VDDIO
} POWER_5VDETECTION;

typedef struct _POWER_INITVALUES
{
    UINT32 u32BatterySamplingInterval;
    POWER_5VDETECTION e5vDetection;

    // Not used by 37xx,377x
    BOOL bEnable4p2;
} POWER_INITVALUES;

//-----------------------------------------------------------------------------
// Functions

BOOL DDKPowerInit(POWER_INITVALUES* pInitValues);
VOID DDKPowerExecuteBatteryTo5VoltsHandoff(VOID);
VOID DDKPowerEnable5VoltsToBatteryHandoff(VOID);
VOID DDKPowerExecute5VoltsToBatteryHandoff(VOID);
VOID DDKPowerEnableBatteryTo5VoltsHandoff(VOID);
BOOL DDKPowerGet5vPresentFlag(VOID);
BOOL DDKPowerGetBatteryChargingStatus(VOID);
UINT16 DDKPowerGetBatteryVoltage(VOID);
BOOL    DDKPowerDeInit(VOID);
BOOL    DDKPowerAlloc(VOID);
BOOL    DDKPowerDealloc(VOID);
VOID    DDKDumpPowerRegisters(VOID);
UINT32  DDKPowerInitBatteryMonitor(LRADC_DELAYTRIGGER eTrigger, UINT32 SamplingInterval);
POWER_BATTERYMODE DDKPowerGetBatteryMode(VOID);
VOID    DDKPowerEnableRcScale(POWER_RCSCALELEVELS eLevel);
VOID    DDKPowerEnableHalfFets(BOOL bEnable);
UINT32  DDKPowerEnable5vDetection(POWER_5VDETECTION eDetection);
UINT32  DDKPowerInitPowerSupplies(VOID);
BOOL    DDKPowerStart4p2HW(VOID);
VOID    DDKPowerEnableDcdc(BOOL bEnable);
UINT32  DDKPowerSetVdddPowerSource(POWER_POWERSOURCE eSource);
UINT32  DDKPowerSetVddioPowerSource(POWER_POWERSOURCE eSource);
UINT32  DDKPowerSetVddaPowerSource(POWER_POWERSOURCE eSource);
VOID    DDKPowerEnableExternalPowerSource(BOOL bEnable);
VOID    DDKPowerDisableVddioLinearRegulator(BOOL bDisable);
UINT32  DDKPowerSetVddaPowerSource(POWER_POWERSOURCE eSource);
VOID    DDKPowerInit4p2EnabledPowerSupply(VOID);
UINT32  DDKPowerStart4p2(VOID);
VOID    DDKPowerInit4p2EnabledPowerSupply(VOID);
VOID    DDKPowerDisableAutoHardwarePowerdown(BOOL bDisable);
VOID    DDKPowerStop4p2(VOID);
VOID    DDKPowerEnableBatteryBrownoutInterrupt(BOOL bEnable);
POWER_POWERSOURCE DDKPowerGetVdddPowerSource(VOID);
VOID DDKPowerSetVdddBrownoutValue(UINT16 u16VdddBoOffset_mV);
VOID DDKPowerSetVdddValue(UINT16 u16Vddd_mV);
UINT32 DDKPowerGetVdddValue(VOID);
VOID DDKPowerSetVdddBrownoutValue(UINT16 u16VdddBoOffset_mV);
VOID DDKPowerEnable5vUnplugDetect(BOOL bEnable);
VOID DDKPowerEnableAutoDcdcTransfer(BOOL bEnable);
VOID DDKPowerEnable5vPlugInDetect(BOOL bEnable);
POWER_LINREGOFFSETSTEP DDKPowerGetVdddLinRegOffset(VOID);
UINT16 DDKPowerConvertVdddToSetting(UINT16 u16Vddd);
VOID DDKPowerSetVbusValidInterruptPolarity(BOOL bPolarity);
VOID DDKPowerSetVdd5vGtVddioInterruptPolarity(BOOL bPolarity);
VOID DDKPowerEnableUSBPHY(VOID);
VOID DDKPowerEnableVbusValid5vDetect(BOOL bEnable);
VOID DDKPowerSetCharger(DWORD current);
VOID DDKPowerStopCharger();
BOOL DDKPowerGet5VIrq();
BOOL DDKPowerGetPSwitchIrq();
VOID DDKPowerClear5VIrq();
VOID DDKPowerClearPSwitchIrq();
BOOL DDKPowerGetLimit();
BOOL DDKPowerInitSupply(void);
BOOL DDKPowerGetUSBPhy();
VOID DDKPowerSetLimit(void);
void DDKPowerInitCpuHclkClock(void);
BOOL DDKPowerGetDirectBoot(void);
VOID DDKPowerClearLimit(void);
BOOL DDKPowerGetPSwitchStatus();

#ifdef __cplusplus
}
#endif

#endif  //__POWER_H__
