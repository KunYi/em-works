//------------------------------------------------------------------------------
// Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  pmu.h
//
//   Header file for PMU module
//
//------------------------------------------------------------------------------

#ifndef __PMU_H__
#define __PMU_H__

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
#define POWER_MAX_VOLTAGE_STEP_MV     100    // in millivolts

typedef enum 
{
    VOLTAGE_STEP_MV     = 25,
    BO_MIN_OFFSET_MV    = 25,
    BO_MAX_OFFSET_MV    = VOLTAGE_STEP_MV * 7,
    
    VDDD_MARGIN_STEPS   = 4,
    VDDD_TO_BO_MARGIN  = (VDDD_MARGIN_STEPS * VOLTAGE_STEP_MV),
    
    VDDD_DEFAULT_MV    = 1200,
    VDDD_DEFAULT_BO    = (VDDD_DEFAULT_MV - BO_MAX_OFFSET_MV),
    VDDD_ABS_MIN_MV    = 800,
    VDDD_ABS_MAX_MV    = 1575,
    VDDD_SAFE_MAX_MV   = 1575,
    VDDD_SAFE_MIN_MV   = (VDDD_ABS_MIN_MV + BO_MIN_OFFSET_MV),
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
typedef enum 
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

//Possible power sources for each power supply
typedef enum 
{
    //Powered by linear regulator.  DCDC output is gated off and
    //the linreg output is equal to the target.
    POWER_LINREG_DCDC_OFF,
    POWER_LINREG_DCDC_READY,
    POWER_DCDC_LINREG_ON,
    POWER_DCDC_LINREG_OFF,
    POWER_DCDC_LINREG_READY,
    POWER_EXTERNAL_SOURCE_BATTERY,
    POWER_UNKNOWN_SOURCE,
    POWER_DCDC1 = 1,
    POWER_DCDC2 = 2,
    POWER_LINEAR_REGULATOR = POWER_LINREG_DCDC_OFF
} POWER_POWERSOURCE;


//Selects the source of the bias current
typedef enum 
{
    HW_POWER_EXTERNAL_BIAS_CURRENT = 0x0,
    HW_POWER_INTERNAL_BIAS_CURRENT = 0x1
} POWER_BIASCURRENTSOURCE;

//Battery charging current magnitudes converted to register settings.
typedef enum 
{
    HW_POWER_CURRENT_MAG_400 = 0x20,
    HW_POWER_CURRENT_MAG_200 = 0x10,
    HW_POWER_CURRENT_MAG_100 = 0x08,
    HW_POWER_CURRENT_MAG_50  = 0x04,
    HW_POWER_CURRENT_MAG_20  = 0x02,
    HW_POWER_CURRENT_MAG_10  = 0x01
} POWER_BATTCHARGECURRENTMAG;

//Linear regulator offset values
typedef enum 
{
    HW_POWER_LINREG_OFFSET_NO_STEP    = 0,
    HW_POWER_LINREG_OFFSET_STEP_ABOVE = 1,
    HW_POWER_LINREG_OFFSET_STEP_BELOW = 2,
    HW_POWER_LINREG_OFFSET_MAX        = 3,
    HW_POWER_LINREG_OFFSET_LINREG_MODE = HW_POWER_LINREG_OFFSET_NO_STEP,
    HW_POWER_LINREG_OFFSET_DCDC_MODE = HW_POWER_LINREG_OFFSET_STEP_BELOW
} POWER_LINREGOFFSETSTEP;

//Possible RC Scale values to increase transient load response
typedef enum 
{
    HW_POWER_RCSCALE_DISABLED   = 0,
    HW_POWER_RCSCALE_2X_INCR    = 1,
    HW_POWER_RCSCALE_4X_INCR    = 2,
    HW_POWER_RCSCALE_8X_INCR    = 3
} POWER_RCSCALELEVELS;

//Interrupt polarities for power hardware
typedef enum 
{
    HW_POWER_CHECK_5V_DISCONNECTED      = 0,
    HW_POWER_CHECK_5V_CONNECTED         = 1,
    HW_POWER_CHECK_LINEAR_REGULATORS_OK = 1,
    HW_POWER_CHECK_PSWITCH_HIGH         = 1,
    HW_POWER_CHECK_PSWITCH_LOW          = 0
} POWER_INTERRUPTPOLARITY;

//Possible sources for Pswitch IRQ source
typedef enum 
{
    HW_POWER_STS_PSWITCH_BIT_0,
    HW_POWER_STS_PSWITCH_BIT_1
} POWER_PSWITCHIRQSOURCE;

//Possible 5V detection methods
typedef enum 
{
    //! \brief VBUSVALID will be used for 5V/USB detection.
    POWER_VBUSVALID,
    //! \brief VDD5V_GT_VDDIO will be used for 5V/USB detection.
    POWER_VDD5V_GT_VDDIO
} POWER_5VDETECTION;



//Possible 5V detection methods
typedef enum 
{
    PMU_POWER_HALF_FETS,
    PMU_POWER_NORMAL_FETS,
    PMU_POWER_DOUBLE_FETS
} PMU_POWER_FETSSET;

//
typedef enum 
{
    PMU_POWER_SUPPLY_5V,
    PMU_POWER_SUPPLY_BATTERY
} PMU_POWER_SUPPLY_MODE;

//Power Rail 
typedef struct
{
    UINT32 TargetmV;
    UINT32 BrownOutmV;
} POWER_RAIL_VALUE,*PPOWER_RAIL_VALUE; 


//
typedef enum 
{
    PMU_POWER_THERMAL_115_DEGC = 0,
    PMU_POWER_THERMAL_120_DEGC = 1,
    PMU_POWER_THERMAL_125_DEGC = 2,
    PMU_POWER_THERMAL_130_DEGC = 3,
    PMU_POWER_THERMAL_135_DEGC = 4,
    PMU_POWER_THERMAL_140_DEGC = 5,
    PMU_POWER_THERMAL_145_DEGC = 6,
    PMU_POWER_THERMAL_150_DEGC = 7,    
    PMU_POWER_THERMAL_TEST40_DEGC = 8,
    PMU_POWER_THERMAL_110_DEGC  = 9,
    PMU_POWER_THERMAL_105_DEGC  = 10,
    PMU_POWER_THERMAL_155_DEGC  = 11,
    PMU_POWER_THERMAL_160_DEGC  = 12   
} PMU_POWER_THERMAL_TEMP;

//------------------------------------------------------------------------------
// Functions for PMU module
BOOL PmuInitBatteryMonitor(void);
BOOL PmuGetBatteryVoltage(UINT32 *pBattVol);
BOOL PmuSetCharger(DWORD current);
BOOL PmuStopCharger();
BOOL PmuGetBatteryChargingStatus(BOOL *bChargStas);
BOOL PmuSetVddd(UINT32 NewTargetmV,  UINT32 NewBrownoutmV );
BOOL PmuGetVddd(UINT32 *VdddmV );
BOOL PmuGetVdddBrownont(UINT32 *VdddBo );
BOOL PmuSetFets(PMU_POWER_FETSSET bFetsMode);
BOOL PmuPowerGetSupplyMode(PMU_POWER_SUPPLY_MODE *PowerMode);
BOOL PmuGetPowerSource(UINT32 *powerSource);

//LQK:Jul-25-2012
BOOL PmuIsBatteryAttach(BOOL *bIsBatteryAttach );

#ifdef __cplusplus
}
#endif

#endif  //__PMU_H__