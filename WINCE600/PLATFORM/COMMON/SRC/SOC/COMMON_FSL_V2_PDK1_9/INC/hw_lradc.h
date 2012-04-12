//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: hw_lradc.h
//------------------------------------------------------------------------------
#ifndef __HW_LRADC__
#define __HW_LRADC__

#ifdef __cplusplus
extern "C" {
#endif

#define LRADC_IOCTL_INIT_CHANNEL                        CTL_CODE(FILE_DEVICE_UNKNOWN, 3017, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_CONFIGURE_CHANNEL                   CTL_CODE(FILE_DEVICE_UNKNOWN, 3018, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_ENABLE_INTERRUPT                    CTL_CODE(FILE_DEVICE_UNKNOWN, 3019, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_CLEAR_INTERRUPT                     CTL_CODE(FILE_DEVICE_UNKNOWN, 3020, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_SET_DELAY_TRIGGER                   CTL_CODE(FILE_DEVICE_UNKNOWN, 3021, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_SET_TRIGGER_KICK                    CTL_CODE(FILE_DEVICE_UNKNOWN, 3022, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_GET_ACCUM_VALUE                     CTL_CODE(FILE_DEVICE_UNKNOWN, 3023, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_GET_INTERRUPT_FLAG                  CTL_CODE(FILE_DEVICE_UNKNOWN, 3024, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_GET_TOGGLE_FLAG                     CTL_CODE(FILE_DEVICE_UNKNOWN, 3025, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_SCHEDULE_CHANNEL                    CTL_CODE(FILE_DEVICE_UNKNOWN, 3026, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_CLEAR_ACCUM                         CTL_CODE(FILE_DEVICE_UNKNOWN, 3027, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_CHANNEL_PRESENT                     CTL_CODE(FILE_DEVICE_UNKNOWN, 3028, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_GET_CLKCGATE                        CTL_CODE(FILE_DEVICE_UNKNOWN, 3029, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_TOUCH_DETECT_PRESENT                CTL_CODE(FILE_DEVICE_UNKNOWN, 3030, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_ENABLE_TOUCH_DETECT                 CTL_CODE(FILE_DEVICE_UNKNOWN, 3031, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_GET_TOUCH_DETECT                    CTL_CODE(FILE_DEVICE_UNKNOWN, 3032, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_ENABLE_TOUCH_DETECT_IRQ             CTL_CODE(FILE_DEVICE_UNKNOWN, 3033, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_GET_TOUCH_DETECT_IRQ                CTL_CODE(FILE_DEVICE_UNKNOWN, 3034, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_CLR_TOUCH_DETECT                    CTL_CODE(FILE_DEVICE_UNKNOWN, 3035, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_ENABLE_X_DRIVE                      CTL_CODE(FILE_DEVICE_UNKNOWN, 3036, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_ENABLE_Y_DRIVE                      CTL_CODE(FILE_DEVICE_UNKNOWN, 3037, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_CLEAR_CHANNEL                       CTL_CODE(FILE_DEVICE_UNKNOWN, 3038, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_SET_ANALOG_POWER_UP                 CTL_CODE(FILE_DEVICE_UNKNOWN, 3039, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_CLEAR_DELAY_CHANNEL                 CTL_CODE(FILE_DEVICE_UNKNOWN, 3040, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_DUMPREGISTER                        CTL_CODE(FILE_DEVICE_UNKNOWN, 3041, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_READ_TOUCH_X                        CTL_CODE(FILE_DEVICE_UNKNOWN, 3042, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_READ_TOUCH_Y                        CTL_CODE(FILE_DEVICE_UNKNOWN, 3043, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_ENABLE_BATT_MEASUREMENT             CTL_CODE(FILE_DEVICE_UNKNOWN, 3044, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_MEASUREDIETEMPERATURE               CTL_CODE(FILE_DEVICE_UNKNOWN, 3045, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_ENABLE_BUTTON_DETECT                CTL_CODE(FILE_DEVICE_UNKNOWN, 3046, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_ENABLE_BUTTON_DETECT_IRQ            CTL_CODE(FILE_DEVICE_UNKNOWN, 3047, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_CLR_BUTTON_DETECT                   CTL_CODE(FILE_DEVICE_UNKNOWN, 3048, METHOD_BUFFERED, FILE_ANY_ACCESS)   //
#define LRADC_IOCTL_MEASUREVDD5V                        CTL_CODE(FILE_DEVICE_UNKNOWN, 3049, METHOD_BUFFERED, FILE_ANY_ACCESS)   //

#define NUM_TEMP_READINGS_TO_AVG    5
#define NUM_VDD5V_READINGS_TO_AVG   5
#define LRADC_FULL_SCALE_VALUE      4096
#define LRADC_VDD5V_SCALE_VALUE     1850 * 4

#define TEMP_SENSOR_CONVERSION_CONSTANT 1104
#define DIE_TEMP_SENSOR_CONVERSION_CONSTANT 1012

//! Enumeration of LRADC temperature sensor current magnitude
typedef enum _LRADC_CURRENTMAGNITUDE
{
    TEMP_SENSOR_CURRENT_0UA,
    TEMP_SENSOR_CURRENT_20UA,
    TEMP_SENSOR_CURRENT_40UA,
    TEMP_SENSOR_CURRENT_60UA,
    TEMP_SENSOR_CURRENT_80UA,
    TEMP_SENSOR_CURRENT_100UA,
    TEMP_SENSOR_CURRENT_120UA,
    TEMP_SENSOR_CURRENT_140UA,
    TEMP_SENSOR_CURRENT_160UA,
    TEMP_SENSOR_CURRENT_180UA,
    TEMP_SENSOR_CURRENT_200UA,
    TEMP_SENSOR_CURRENT_220UA,
    TEMP_SENSOR_CURRENT_240UA,
    TEMP_SENSOR_CURRENT_260UA,
    TEMP_SENSOR_CURRENT_280UA,
    TEMP_SENSOR_CURRENT_300UA,
    TEMP_SENSOR_CURRENT_LOW_READING = TEMP_SENSOR_CURRENT_20UA,
    TEMP_SENSOR_CURRENT_HIGH_READING = TEMP_SENSOR_CURRENT_300UA
} LRADC_CURRENTMAGNITUDE;

//! Enumeration of LRADC Channels
typedef enum _LRADC_CHANNEL
{
    LRADC_CH0           = 0,
    LRADC_CH1           = 1,
    LRADC_CH2           = 2,
    LRADC_CH3           = 3,
    LRADC_CH4           = 4,
    LRADC_CH5           = 5,
    LRADC_CH6           = 6,
    VDDIO_VOLTAGE_CH    = 6,
    LRADC_CH7           = 7,
    BATTERY_VOLTAGE_CH  = 7
} LRADC_CHANNEL;

//! Enumeration of DELAY Channels
typedef enum _LRADC_DELAY_CHANNEL
{
    LRADC_DELAY_CH0     = 0,
    LRADC_DELAY_CH1     = 1,
    LRADC_DELAY_CH2     = 2,
    LRADC_DELAY_CH3     = 3,
} LRADC_DELAY_CHANNEL;

//! Enumeration of LRADC Delay Triggers
typedef enum _LRADC_DELAYTRIGGER
{
    LRADC_DELAY_TRIGGER0    = 0,
    LRADC_DELAY_TRIGGER1    = 1,
    LRADC_DELAY_TRIGGER2    = 2,
    LRADC_DELAY_TRIGGER3    = 3,
    LRADC_DELAY_TRIGGER_MAX = 3
} LRADC_DELAYTRIGGER;

//! Enumeration of LRADC clock frequency
typedef enum _LRADC_CLOCKFREQ
{
    LRADC_CLOCK_6MHZ,
    LRADC_CLOCK_4MHZ,
    LRADC_CLOCK_3MHZ,
    LRADC_CLOCK_2MHZ
} LRADC_CLOCKFREQ;

//! Battery mode enum
typedef enum _LRADC_BATTERYMODE
{
    LRADC_BATT_MODE_0                      = 0,
    LRADC_BATT_MODE_LILON_DUAL_CONVERTOR   = 0,
    LRADC_BATT_MODE_1                      = 1,
    LRADC_BATT_MODE_LILON_SINGLE_INDUCTOR  = 1,
    LRADC_BATT_MODE_2                      = 2,
    LRADC_BATT_MODE_SERIES_AA_AAA          = 2,
    LRADC_BATT_MODE_DUAL_ALKALINE_NIMH     = 2,
    LRADC_BATT_MODE_3                      = 3,
    LRADC_BATT_MODE_SINGLE_AA              = 3,
    LRADC_BATT_MODE_SINGLE_ALKALINE_NIMH   = 3
} LRADC_BATTERYMODE;

//! LRADC temperature sensor interface
typedef enum _hw_lradc_TempSensor
{
    TEMP_SENSOR0,
    TEMP_SENSOR1
} LRADC_TEMPSENSOR;

typedef struct _LRADC_CONFIGURE
{
    LRADC_CLOCKFREQ eFreq;                  //!<  Frequency to be set.
    LRADC_CHANNEL eChannel;                 //!<  Channel to be Configured
    LRADC_DELAYTRIGGER eDelayTrigger;
    LRADC_DELAY_CHANNEL eDelayChannel;
    UINT8 u8NumSamples;
    UINT32 u32TriggerLradcs;
    UINT32 u32DelayTriggers;
    UINT32 u32LoopCount;
    UINT32 u32DelayCount;
    UINT16 u16AccumValue;
    BOOL bEnableOnChipGroundRef;
    BOOL bEnableDivideByTwo;
    BOOL bEnableAccum;
    BOOL bValue;
    UINT16 u16BattSampRate;
    BOOL Use5WireTouch;
} STLRADCCONFIGURE,*pSTLRADCCONFIGURE;

typedef enum _LRADC_BUTTON
{
    LRADC_BUTTON0           = 0,
    LRADC_BUTTON1           = 1,      
} LRADC_BUTTON;

typedef struct _LRADC_BUTTON_CONFIGURE
{
    LRADC_BUTTON button;              //button Channel to be Configured
    BOOL         bValue;
} LRADCBUTTONCONFIGURE,*pLRADCBUTTONCONFIGURE;

  #define _LRADC_CH_USER0                       0
  #define _LRADC_CH_USER1                       1
  #define _LRADC_CH_TOUCH_XPLUSE                2
  #define _LRADC_CH_TOUCH_XMINUS                4
  #define _LRADC_CH_TOUCH_YPLUSE                3
  #define _LRADC_CH_TOUCH_YMINUS                5
  #define _LRADC_CH_REFERENCE                   6
  #define _LRADC_CH_VDDIO                       6
  #define _LRADC_CH_BATTERY                     7
  #define _LRADC_CH_TEMPERTURE0                 8
  #define _LRADC_CH_TEMPERTURE1                 9
  #define _LRADC_CH_USB_DP                      12
  #define _LRADC_CH_USB_DM                      13
  #define _LRADC_CH_BANDGAP                     14
  #define _LRADC_CH_VDD5V                       15
  #define _LRADC_AVALIABLE_CHANNELS             8


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//
//
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  #define _LRADC_SAMPLE_BASE_FREQ               (2000)
  #define _LRADC_SAMPLES_PER_SECOND             (100)
  #define _LRADC_SAMPLES_PERIOD                 (_LRADC_SAMPLE_BASE_FREQ/_LRADC_SAMPLES_PER_SECOND)

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//
//
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  #define _LRADC_IRQ_PEN_DETECT                 8
  #define _LRADC_IRQ_CONVERT                    7

  #define _HW_CH_CONVERT                        _LRADC_CH_BATTERY
  #define _BP_HW_CH_CONVERT                     (1 << _HW_CH_CONVERT)

  #define _HW_LRADC_SCHEDULER                   3
  #define _BP_HW_LRADC_SCHEDULER                (1 << _HW_LRADC_SCHEDULER)
  #define _TRIGGER_LRADCS                       (0xFF)

  #define BUILD_SCHEDULER_VALUE()       \
    (BF_LRADC_DELAYn_TRIGGER_LRADCS(_TRIGGER_LRADCS) | \
     BF_LRADC_DELAYn_TRIGGER_DELAYS(0)                               | \
     BF_LRADC_DELAYn_KICK(1)                                                 | \
     BF_LRADC_DELAYn_LOOP_COUNT(0)                                   | \
     BF_LRADC_DELAYn_DELAY(_LRADC_SAMPLES_PERIOD))

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//
//
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  #define _HW_TOUCH_CHANNEL_X                   _LRADC_CH_TOUCH_XPLUSE
  #define _HW_TOUCH_CHANNEL_Y                   _LRADC_CH_TOUCH_YPLUSE
  #define _HW_TOUCH_CHANNEL_NORMALIZE           _LRADC_CH_VDDIO
  #define _BP_HW_TOUCH_CHANNEL_X                (1 << _HW_TOUCH_CHANNEL_X)
  #define _BP_HW_TOUCH_CHANNEL_Y                (1 << _HW_TOUCH_CHANNEL_Y)
  #define _BP_HW_TOUCH_CHANNEL_NORMALIZE        (1 << _HW_TOUCH_CHANNEL_NORMALIZE)

typedef enum {
    _STATE_TOUCH_WAIT_PENDOWN=0,
    _STATE_SAMPLE_X,
    _STATE_SAMPLE_Y,
};

#define _HW_TOUCH_SAMPLES                                   4

#define _NAME_TOUCH_EVENT                     _T("Lradc_TouchEvent")
#define _NAME_KEYPAD_EVENT                    _T("Lradc_KeyPadEvent")
#define _NAME_BATTERY_EVENT                   _T("Lradc_BatteryEvent")
#define _NAME_USBD_FORCE_PWRNDN_EVENT         _T("USBD_FORCE_PWRDN")
#define _NAME_USBD_FORCE_PWRNUP_EVENT         _T("USBD_FORCE_PWRUP")


//Functions

HANDLE LRADCOpenHandle(LPCWSTR lpDevName);
BOOL LRADCCloseHandle(HANDLE hLRADC);
BOOL LRADCInit(HANDLE hLRADC,BOOL bEnableOnChipGroundRef, LRADC_CLOCKFREQ eFreq);
BOOL LRADCConfigureChannel(HANDLE hLRADC,
                           LRADC_CHANNEL eChannel,
                           BOOL bEnableDivideByTwo,
                           BOOL bEnableAccum,
                           UINT8 u8NumSamples);
BOOL LRADCEnableInterrupt(HANDLE hLRADC,LRADC_CHANNEL eChannel,BOOL bValue);
BOOL LRADCClearInterruptFlag(HANDLE hLRADC,LRADC_CHANNEL eChannel);
BOOL LRADCSetDelayTrigger(HANDLE hLRADC,
                          LRADC_DELAYTRIGGER DelayTrigger,
                          UINT32 TriggerLradcs,
                          UINT32 DelayTriggers,
                          UINT32 LoopCount,
                          UINT32 DelayCount);
BOOL LRADCSetDelayTriggerKick(HANDLE hLRADC,
                              LRADC_DELAYTRIGGER DelayTrigger,
                              BOOL bValue);
UINT16 LRADCGetAccumValue(HANDLE hLRADC,LRADC_CHANNEL Channel);
BOOL LRADCGetInterruptFlag(HANDLE hLRADC,LRADC_CHANNEL Channel);
BOOL LRADCGetToggleFlag(HANDLE hLRADC,LRADC_CHANNEL Channel);
BOOL LRADCScheduleChannel(HANDLE hLRADC,LRADC_CHANNEL Channel);
BOOL LRADCClearAccum(HANDLE hLRADC,LRADC_CHANNEL Channel);
BOOL LRADCGetChannelPresent(HANDLE hLRADC,LRADC_CHANNEL Channel);
BOOL LRADCClearChannel(HANDLE hLRADC,LRADC_CHANNEL Channel);
BOOL LRADCSetAnalogPowerUp(HANDLE hLRADC,BOOL Use5WireTouch,BOOL bValue);
BOOL LRADCCLearDelayChannel(HANDLE hLRADC,LRADC_DELAY_CHANNEL eDelayChannel);
BOOL LRADCGetClkGate(HANDLE hLRADC); 
BOOL LRADCGetTouchDetectPresent(HANDLE hLRADC);
BOOL LRADCEnableTouchDetect(HANDLE hLRADC,BOOL bValue);
BOOL LRADCGetTouchDetect(HANDLE hLRADC);
BOOL LRADCEnableTouchDetectInterrupt(HANDLE hLRADC,BOOL bValue);
BOOL LRADCGetTouchDetectInterruptFlag(HANDLE hLRADC);
BOOL LRADCClearTouchDetect(HANDLE hLRADC);
BOOL LRADCEnableButtonDetect(HANDLE hLRADC,LRADC_BUTTON button,BOOL bValue);
BOOL LRADCEnableButtonDetectInterrupt(HANDLE hLRADC,LRADC_BUTTON button,BOOL bValue);
BOOL LRADCClearButtonDetect(HANDLE hLRADC,LRADC_BUTTON button);

BOOL LRADCEnableTouchDetectXDrive(HANDLE hLRADC,BOOL Use5WireTouch,BOOL bValue);
BOOL LRADCEnableTouchDetectYDrive(HANDLE hLRADC,BOOL Use5WireTouch,BOOL bValue);
UINT16 LRADCReadTouchX(HANDLE hLRADC,LRADC_CHANNEL Channel);
UINT16 LRADCReadTouchY(HANDLE hLRADC,LRADC_CHANNEL Channel);

UINT16 LRADCEnableBatteryMeasurement(HANDLE hLRADC,
                                     LRADC_DELAYTRIGGER eTrigger,
                                     UINT16 SamplingInterval);
UINT32 LRADCEnableDieMeasurement(HANDLE hLRADC);
UINT32 LRADCEnableVDD5VMeasurement(HANDLE hLRADC);
BOOL   LRADCDumpRegisters(HANDLE hLRADC);

#ifdef __cplusplus
}
#endif
#endif //__HW_LRADC__
