//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  lradcsdk.cpp
//
//  The implementation of LRADC driver SDK.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include "winioctl.h"
#include <Windev.h>
#include "ceddk.h"
#pragma warning(pop)

#include "hw_lradc.h"


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------

//#ifdef DEBUG
//// Debug zone bit positions
//#define ZONEID_INIT       0
//#define ZONEID_INFO       12
//#define ZONEID_FUNCTION   13
//#define ZONEID_WARN       14
//#define ZONEID_ERROR      15
//
//// Debug zone masks
//#define ZONEMASK_INIT     (1<<ZONEID_INIT)
//#define ZONEMASK_INFO     (1<<ZONEID_INFO)
//#define ZONEMASK_FUNCTION (1<<ZONEID_FUNCTION)
//#define ZONEMASK_WARN     (1<<ZONEID_WARN)
//#define ZONEMASK_ERROR    (1<<ZONEID_ERROR)
//
//// Debug zone args to DEBUGMSG
//#define ZONE_INIT         DEBUGZONE(ZONEID_INIT)
//#define ZONE_INFO         DEBUGZONE(ZONEID_INFO)
//#define ZONE_FUNCTION     DEBUGZONE(ZONEID_FUNCTION)
//#define ZONE_WARN         DEBUGZONE(ZONEID_WARN)
//#define ZONE_ERROR        DEBUGZONE(ZONEID_ERROR)
//#endif


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
#ifdef DEBUG
//extern DBGPARAM dpCurSettings =
//{
//    _T("LRADC"),
//    {
//        _T("Init"), _T(""), _T(""), _T(""),
//        _T(""), _T(""), _T(""), _T(""),
//        _T(""),_T(""),_T(""),_T(""),
//        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
//    },
//    ZONEMASK_ERROR | ZONEMASK_WARN | ZONEMASK_INFO
//};
#endif


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: LRADCOpenHandle
//
// This method creates a handle to the LRADC stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to LRADC driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE LRADCOpenHandle(LPCWSTR lpDevName)
{
    HANDLE hLRADC;


    hLRADC = CreateFile(lpDevName,            // name of device
                        GENERIC_READ|GENERIC_WRITE, // desired access
                        FILE_SHARE_READ|FILE_SHARE_WRITE, // sharing mode
                        NULL,               // security attributes (ignored)
                        OPEN_EXISTING,      // creation disposition
                        FILE_FLAG_RANDOM_ACCESS, // flags/attributes
                        NULL);              // template file (ignored)

    // if we failed to get handle to LRADC
    if (hLRADC == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    return hLRADC;
}

//------------------------------------------------------------------------------
//
// Function: LRADCCloseHandle
//
// This method closes a handle to the LRADC stream driver.
//
// Parameters:
//      hLRADC
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL LRADCCloseHandle(HANDLE hLRADC)
{
    // if we don't have handle to LRADC bus driver
    if (hLRADC != NULL)
    {
        if (!CloseHandle(hLRADC))
        {
            return FALSE;
        }
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: LRADCConfigureChannel
//
// This function Configures the LRADC Channel.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCConfigureChannel(HANDLE hLRADC,
                           LRADC_CHANNEL eChannel,
                           BOOL bEnableDivideByTwo,
                           BOOL bEnableAccum,
                           UINT8 u8NumSamples)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel =eChannel;
    stLRADCConfig.bEnableDivideByTwo=bEnableDivideByTwo;
    stLRADCConfig.bEnableAccum =bEnableAccum;
    stLRADCConfig.u8NumSamples =u8NumSamples;

    if (!DeviceIoControl(hLRADC,      // file handle to the driver
                         LRADC_IOCTL_CONFIGURE_CHANNEL, // I/O control code
                         &stLRADCConfig, // in buffer
                         0,         // in buffer size
                         NULL,      // out buffer
                         0,         // out buffer size
                         NULL,      // pointer to number of bytes returned
                         NULL))     // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: LRADCInit
//
// This function initializes the LRADC Channel.
//
// Parameters:
//
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCInit(HANDLE hLRADC,BOOL bEnableOnChipGroundRef, LRADC_CLOCKFREQ eFreq)
{
    STLRADCCONFIGURE stLRADCConfig;

    stLRADCConfig.eFreq = eFreq;
    stLRADCConfig.bEnableOnChipGroundRef = bEnableOnChipGroundRef;

    if (!DeviceIoControl(hLRADC,      // file handle to the driver
                         LRADC_IOCTL_INIT_CHANNEL,      // I/O control code
                         &stLRADCConfig, // in buffer
                         0,         // in buffer size
                         NULL,      // out buffer
                         0,         // out buffer size
                         NULL,      // pointer to number of bytes returned
                         NULL))     // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCGetClkGate
//
// This function initializes the LRADC Channel.
//
// Parameters:
//
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCGetClkGate(HANDLE hLRADC)
{
    if (!DeviceIoControl(hLRADC,      // file handle to the driver
                         LRADC_IOCTL_GET_CLKCGATE,      // I/O control code
                         NULL, // in buffer
                         0,         // in buffer size
                         NULL,      // out buffer
                         0,         // out buffer size
                         NULL,      // pointer to number of bytes returned
                         NULL))     // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCEnableInterrupt
//
// This function Enable the Interrupt of the LRADC Channel.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCEnableInterrupt(HANDLE hLRADC,LRADC_CHANNEL eChannel,BOOL bValue)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel =eChannel;
    stLRADCConfig.bValue =bValue;

    if (!DeviceIoControl(hLRADC,      // file handle to the driver
                         LRADC_IOCTL_ENABLE_INTERRUPT,  // I/O control code
                         &stLRADCConfig, // in buffer
                         0,         // in buffer size
                         NULL,      // out buffer
                         0,         // out buffer size
                         NULL,      // pointer to number of bytes returned
                         NULL))     // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: LRADCClearInterruptFlag
//
// This function Clears the interrupt flag of a specified LRADC channel.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCClearInterruptFlag(HANDLE hLRADC,LRADC_CHANNEL eChannel)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel =eChannel;

    if (!DeviceIoControl(hLRADC,      // file handle to the driver
                         LRADC_IOCTL_CLEAR_INTERRUPT,   // I/O control code
                         &stLRADCConfig, // in buffer
                         0,         // in buffer size
                         NULL,      // out buffer
                         0,         // out buffer size
                         NULL,      // pointer to number of bytes returned
                         NULL))     // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: LRADCSetDelayTrigger
//
// This function Sets the ADC conversion sample time of the LRADC Channel.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCSetDelayTrigger(HANDLE hLRADC,
                          LRADC_DELAYTRIGGER DelayTrigger,
                          UINT32 TriggerLradcs,
                          UINT32 DelayTriggers,
                          UINT32 LoopCount,
                          UINT32 DelayCount)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eDelayTrigger =DelayTrigger;
    stLRADCConfig.u32TriggerLradcs =TriggerLradcs;
    stLRADCConfig.u32DelayTriggers =DelayTriggers;
    stLRADCConfig.u32LoopCount =LoopCount;
    stLRADCConfig.u32DelayCount =DelayCount;

    if (!DeviceIoControl(hLRADC,      // file handle to the driver
                         LRADC_IOCTL_SET_DELAY_TRIGGER, // I/O control code
                         &stLRADCConfig, // in buffer
                         0,         // in buffer size
                         NULL,      // out buffer
                         0,         // out buffer size
                         NULL,      // pointer to number of bytes returned
                         NULL))     // ignored (=NULL)
    {

        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCCLearDelayChannel
//
// This function Sets the ADC conversion sample time of the LRADC Channel.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCCLearDelayChannel(HANDLE hLRADC,LRADC_DELAY_CHANNEL eDelayChannel)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eDelayChannel = eDelayChannel;

    if (!DeviceIoControl(hLRADC,      // file handle to the driver
                         LRADC_IOCTL_CLEAR_DELAY_CHANNEL,       // I/O control code
                         &stLRADCConfig, // in buffer
                         0,         // in buffer size
                         NULL,      // out buffer
                         0,         // out buffer size
                         NULL,      // pointer to number of bytes returned
                         NULL))     // ignored (=NULL)
    {

        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCSetDelayTriggerKick
//
// This function Sets the ADC conversion sample time of the LRADC Channel.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCSetDelayTriggerKick(HANDLE hLRADC,
                              LRADC_DELAYTRIGGER DelayTrigger,
                              BOOL bValue)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eDelayTrigger =DelayTrigger;
    stLRADCConfig.bValue =bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_SET_TRIGGER_KICK, // I/O control code
                         &stLRADCConfig,        // in buffer
                         0,                                             // in buffer size
                         NULL,                                  // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: LRADCGetAccumValue
//
// This function Gets the co-nversion value of a specified LRADC channel.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
UINT16 LRADCGetAccumValue(HANDLE hLRADC,LRADC_CHANNEL Channel)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel = Channel;
    UINT16 Accum_Value = 0;


    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_GET_ACCUM_VALUE, // I/O control code
                         &stLRADCConfig,        // in buffer
                         0,                                             // in buffer size
                         &Accum_Value,                  // out buffer
                         sizeof(UINT16),                                        // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return Accum_Value;
    }
    return Accum_Value;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCReadTouchX
//
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//
//-----------------------------------------------------------------------------
UINT16 LRADCReadTouchX(HANDLE hLRADC,LRADC_CHANNEL Channel)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel = Channel;
    UINT16 Xcordinate = 0;


    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_READ_TOUCH_X, // I/O control code
                         &stLRADCConfig,        // in buffer
                         0,                                             // in buffer size
                         &Xcordinate,                   // out buffer
                         sizeof(UINT16),                                        // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return Xcordinate;
    }
    return Xcordinate;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCReadTouchY
//
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//
//-----------------------------------------------------------------------------
UINT16 LRADCReadTouchY(HANDLE hLRADC,LRADC_CHANNEL Channel)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel = Channel;
    UINT16 Ycordinate = 0;


    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_READ_TOUCH_Y, // I/O control code
                         &stLRADCConfig,        // in buffer
                         0,                                             // in buffer size
                         &Ycordinate,                   // out buffer
                         sizeof(UINT16),                                        // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return Ycordinate;
    }
    return Ycordinate;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCEnableBatteryMeasurement
//
// This function enables the LRADC channel for battery measurement.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//      eTrigger
//          [in] Specifies the Delay Trigger to use
//      SamplingInterval
//          [in] Specifies the smapling interval for the Battery value updation
//      eBatteryMode
//          [in] Specifies the Battery mode setup
//
// Returns:
//      Return 0 If the operation is successful otherwise returns error value
//
//-----------------------------------------------------------------------------
UINT16 LRADCEnableBatteryMeasurement(HANDLE hLRADC,
                                     LRADC_DELAYTRIGGER eTrigger,
                                     UINT16 SamplingInterval,
                                     LRADC_BATTERYMODE eBatteryMode)
{
    STLRADCCONFIGURE stLRADCConfig;

    stLRADCConfig.eDelayTrigger = eTrigger;
    stLRADCConfig.u16BattSampRate = SamplingInterval;
    stLRADCConfig.eBatteryMode = eBatteryMode;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_ENABLE_BATT_MEASUREMENT,   // I/O control code
                         &stLRADCConfig,                                // in buffer
                         sizeof(STLRADCCONFIGURE),              // in buffer size
                         NULL,                                          // out buffer
                         0,                                                             // out buffer size
                         NULL,                                                  // pointer to number of bytes returned
                         NULL))                                                 // ignored (=NULL)
    {
        return 1;
    }
    return 0;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCEnableDieMeasurement
//
// This function enables the LRADC channel for die temperature measurement.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return die temperature.
//
//-----------------------------------------------------------------------------
UINT32 LRADCEnableDieMeasurement(HANDLE hLRADC)
{
    UINT32 temperature = 0;
    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_MEASUREDIETEMPERATURE,   // I/O control code
                         NULL,                                // in buffer
                         sizeof(STLRADCCONFIGURE),              // in buffer size
                         &temperature,                         // out buffer
                         sizeof(UINT32),                     // out buffer size
                         NULL,                                // pointer to number of bytes returned
                         NULL))                               // ignored (=NULL)
    {
        return temperature;
    }
    return temperature;
}

//-----------------------------------------------------------------------------
//
// Function: LRADCGetInterruptFlag
//
// This function Gets the Interrupt Status bit of the specified LRADC channel.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCGetInterruptFlag(HANDLE hLRADC,LRADC_CHANNEL Channel)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel = Channel;
    BOOL bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_GET_INTERRUPT_FLAG, // I/O control code
                         &stLRADCConfig,        // in buffer
                         0,                                             // in buffer size
                         &bValue,                                       // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: LRADCGetToggleFlag
//
// This function gets Toggle Value Status of the specified LRADC channel
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCGetToggleFlag(HANDLE hLRADC,LRADC_CHANNEL Channel)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel = Channel;
    BOOL bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_GET_TOGGLE_FLAG, // I/O control code
                         &stLRADCConfig,        // in buffer
                         0,                                             // in buffer size
                         &bValue,                                       // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCScheduleChannel
//
// This function Sets the Schedule bit of the specified LRADC channel
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCScheduleChannel(HANDLE hLRADC,LRADC_CHANNEL Channel)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel = Channel;
    BOOL bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_SCHEDULE_CHANNEL, // I/O control code
                         &stLRADCConfig,        // in buffer
                         0,                                             // in buffer size
                         &bValue,                                       // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCClearAccum
//
// This function Clears the Accum Value of the specified LRADC channel
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCClearAccum(HANDLE hLRADC,LRADC_CHANNEL Channel)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel = Channel;
    BOOL bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_CLEAR_ACCUM, // I/O control code
                         &stLRADCConfig,        // in buffer
                         0,                                             // in buffer size
                         &bValue,                                       // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCClearChannel
//
// This function Check if the lradc channel is present
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCClearChannel(HANDLE hLRADC,LRADC_CHANNEL Channel)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel = Channel;
    BOOL bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_CLEAR_CHANNEL, // I/O control code
                         &stLRADCConfig,        // in buffer
                         0,                                             // in buffer size
                         &bValue,                                       // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
// Function: LRADCGetChannelPresent
//
// This function Check if the lradc channel is present
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCGetChannelPresent(HANDLE hLRADC,LRADC_CHANNEL Channel)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.eChannel = Channel;
    BOOL bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_CHANNEL_PRESENT, // I/O control code
                         &stLRADCConfig,        // in buffer
                         0,                                             // in buffer size
                         &bValue,                                       // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCGetTouchDetectPresent
//
// This function Checks if the TOUCH_PANEL_PRESENT is set in LRADC STATUS register
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCGetTouchDetectPresent(HANDLE hLRADC)
{
    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_TOUCH_DETECT_PRESENT, // I/O control code
                         NULL,                                  // in buffer
                         0,                                             // in buffer size
                         NULL,                                  // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: LRADCEnableTouchDetect
//
// This function set or clear the  TOUCH_DETECT_ENABLE in HW_LRADC_CTRL0 Register.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//      bValue
//          [in] To set or clear the bit
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCEnableTouchDetect(HANDLE hLRADC,BOOL bValue)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.bValue = bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_ENABLE_TOUCH_DETECT, // I/O control code
                         &stLRADCConfig,                                // in buffer
                         0,                                             // in buffer size
                         NULL,                                  // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCGetTouchDetect
//
// This function Read the TOUCH_DETECT_RAW bit of HW_LRADC_STATUS register.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCGetTouchDetect(HANDLE hLRADC)
{
    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_GET_TOUCH_DETECT, // I/O control code
                         NULL,                                  // in buffer
                         0,                                             // in buffer size
                         NULL,                                  // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: LRADCEnableTouchDetectInterrupt
//
// This function set or clear the  TOUCH_DETECT_IRQ_EN in HW_LRADC_CTRL1 Register.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//      bValue
//          [in] To set or clear the TOUCH_DETECT_IRQ_EN bit
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCEnableTouchDetectInterrupt(HANDLE hLRADC,BOOL bValue)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.bValue = bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_ENABLE_TOUCH_DETECT_IRQ, // I/O control code
                         &stLRADCConfig,                                // in buffer
                         0,                                             // in buffer size
                         NULL,                                  // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCSetAnalogPowerUp
//
// This function set or clear the  TOUCH_DETECT_IRQ_EN in HW_LRADC_CTRL1 Register.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//      bValue
//          [in] To set or clear the TOUCH_DETECT_IRQ_EN bit
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCSetAnalogPowerUp(HANDLE hLRADC,BOOL bValue)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.bValue = bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_SET_ANALOG_POWER_UP, // I/O control code
                         &stLRADCConfig,                                // in buffer
                         0,                                             // in buffer size
                         NULL,                                  // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
} //-----------------------------------------------------------------------------
//
// Function: LRADCGetTouchDetectInterruptFlag
//
// This function Get the  TOUCH_DETECT_IRQ status in HW_LRADC_CTRL1 Register.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCGetTouchDetectInterruptFlag(HANDLE hLRADC)
{
    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_GET_TOUCH_DETECT_IRQ, // I/O control code
                         NULL,                                  // in buffer
                         0,                                             // in buffer size
                         NULL,                                  // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCClearTouchDetect
//
// This function Clears the  TOUCH_DETECT_IRQ in HW_LRADC_CTRL1 Register.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCClearTouchDetect(HANDLE hLRADC)
{
    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_CLR_TOUCH_DETECT, // I/O control code
                         NULL,                                  // in buffer
                         0,                                             // in buffer size
                         NULL,                                  // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCEnableTouchDetectXDrive
//
// This function enable or disable the X Channels in  HW_LRADC_CTRL0 Register.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//      bValue
//          [in] Enable or Disable the X drive
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCEnableTouchDetectXDrive(HANDLE hLRADC,BOOL bValue)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.bValue = bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_ENABLE_X_DRIVE, // I/O control code
                         &stLRADCConfig,                                // in buffer
                         0,                                             // in buffer size
                         NULL,                                  // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCEnableTouchDetectYDrive
//
// This function enable or disable the Y Channels in  HW_LRADC_CTRL0 Register.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//      bValue
//          [in] Enable or Disable the Y drive
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCEnableTouchDetectYDrive(HANDLE hLRADC,BOOL bValue)
{
    STLRADCCONFIGURE stLRADCConfig;
    stLRADCConfig.bValue = bValue;

    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_ENABLE_Y_DRIVE, // I/O control code
                         &stLRADCConfig,                                // in buffer
                         0,                                             // in buffer size
                         NULL,                                  // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LRADCDumpRegisters
//
// This function enable or disable the Y Channels in  HW_LRADC_CTRL0 Register.
//
// Parameters:
//      hLRADC
//          [in] The LRADC device handle retrieved from LRADCOpenHandle().
//      bValue
//          [in] Enable or Disable the Y drive
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL LRADCDumpRegisters(HANDLE hLRADC)
{
    if (!DeviceIoControl(hLRADC,                // file handle to the driver
                         LRADC_IOCTL_DUMPREGISTER, // I/O control code
                         NULL,                                  // in buffer
                         0,                                             // in buffer size
                         NULL,                                  // out buffer
                         0,                                             // out buffer size
                         NULL,                                  // pointer to number of bytes returned
                         NULL))                                 // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the LRADC DDK module.  This
// function is called when processed and threads attach and detach from this
// module.
//
// Parameters:
//      hInstDll
//           [in] The handle to this module.
//
//      dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//      lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the LRADC is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
extern "C"
BOOL WINAPI
DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        //DEBUGREGISTER((HMODULE)hInstDll);
        //DEBUGMSG(ZONE_INIT,
        //         (_T("***** DLL PROCESS ATTACH TO LRADC SDK *****\r\n")));
        DisableThreadLibraryCalls((HMODULE) hInstDll);
        break;

    case DLL_PROCESS_DETACH:
        //DEBUGMSG(ZONE_INIT,
        //         (_T("***** DLL PROCESS DETACH FROM LRADC SDK *****\r\n")));
        break;
    }

    // return TRUE for success
    return TRUE;
}

