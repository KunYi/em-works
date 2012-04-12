//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  lradc_class.h
//
//  Header file, for LRADC driver.
//
//------------------------------------------------------------------------------

#ifndef __LRADC_CLASS_H
#define __LRADC_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pm.h>
#include "hw_lradc.h"
//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_INIT         0
#define ZONEID_DEINIT       1
#define ZONEID_OPEN         2
#define ZONEID_CLOSE        3
#define ZONEID_IOCTL        4
#define ZONEID_THREAD       5
#define ZONEID_FUNCTION     13
#define ZONEID_WARN         14
#define ZONEID_ERROR        15

// Debug zone masks
#define ZONEMASK_INIT       (1 << ZONEID_INIT)
#define ZONEMASK_DEINIT     (1 << ZONEID_DEINIT)
#define ZONEMASK_OPEN       (1 << ZONEID_OPEN)
#define ZONEMASK_CLOSE      (1 << ZONEID_CLOSE)
#define ZONEMASK_IOCTL      (1 << ZONEID_IOCTL)
#define ZONEMASK_THREAD     (1 << ZONEID_THREAD)
#define ZONEMASK_FUNCTION   (1 << ZONEID_FUNCTION)
#define ZONEMASK_WARN       (1 << ZONEID_WARN)
#define ZONEMASK_ERROR      (1 << ZONEID_ERROR)

#define ZONE_INIT       DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT     DEBUGZONE(ZONEID_DEINIT)
#define ZONE_OPEN       DEBUGZONE(ZONEID_OPEN)
#define ZONE_CLOSE      DEBUGZONE(ZONEID_CLOSE)
#define ZONE_IOCTL      DEBUGZONE(ZONEID_IOCTL)
#define ZONE_THREAD     DEBUGZONE(ZONEID_THREAD)
#define ZONE_FUNCTION   DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN       DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR      DEBUGZONE(ZONEID_ERROR)
#endif // DEBUG

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------


class LRADCClass
{
public:
    CEDEVICE_POWER_STATE m_dxCurrent;

    // LRADC CONSTRUCTOR/DESTRUCTOR METHODS
    LRADCClass(/*UINT32 index*/);
    ~LRADCClass(void)
    {
    };
    BOOL Init(VOID);
    VOID DeInit(VOID);

private:

    CRITICAL_SECTION gcsLradcChanLock;
    BOOL Alloc(VOID);
    VOID DeaAlloc(VOID);

    static DWORD RelayThreadApp(LPVOID pData)
    {
        return ((LRADCClass *) pData)->LARDCIST(/*LPVOID lpParameter*/);
    };
    static DWORD RelayThreadIST(LPVOID pData)
    {
        return ((LRADCClass *) pData)->LRADCInterruptHandler(/*UINT32 timeout*/);
    };

    DWORD LARDCIST(/*LPVOID lpParameter*/);

    DWORD LRADCInterruptHandler(/*UINT32 timeout*/)
    {
        return 0;
    }

    //
    HANDLE m_hLRADCIST;
    DWORD dwSysIntr;                                    // System Interrupt ID

public:
    //general lradc functions
    BOOL IOControl(DWORD Handle,
                   DWORD dwIoControlCode,
                   PBYTE pInBuf,PBYTE pOutBuf);

    BOOL Init (STLRADCCONFIGURE *Lradc_Configure);
    BOOL GetClkGate(VOID);
    BOOL EnableInterrupt(LRADC_CHANNEL Channel, BOOL bValue);
    BOOL ConfigureChannel(STLRADCCONFIGURE *Lradc_Configure);
    BOOL SetDelayTrigger( STLRADCCONFIGURE *Lradc_Configure);
    BOOL SetDelayTriggerKick(BOOL bValue,LRADC_DELAYTRIGGER DelayTrigger);
    BOOL ClearInterruptFlag(LRADC_CHANNEL Channel);
    UINT16 GetAccumValue(LRADC_CHANNEL Channel);
    BOOL ClearAccum(LRADC_CHANNEL Channel);
    BOOL GetInterruptFlag(LRADC_CHANNEL Channel);
    BOOL GetToggleFlag(LRADC_CHANNEL Channel);
    BOOL ScheduleChannel(LRADC_CHANNEL Channel);
    BOOL GetChannelPresent(LRADC_CHANNEL Channel);
    BOOL ClearChannel(LRADC_CHANNEL Channel);
    BOOL SetAnalogPowerUp(BOOL Use5WireTouch,BOOL bValue);
    BOOL CLearDelayChannel(LRADC_DELAY_CHANNEL eDelayChannel);
    UINT32 InitLadder(LRADC_CHANNEL Channel,
                      LRADC_DELAYTRIGGER Trigger,
                      UINT16 u16SamplingInterval);

    UINT32 EnableBatteryMeasurement( LRADC_DELAYTRIGGER eTrigger,
                                     UINT16 u16SamplingInterval);
    UINT16 MeasureBatteryTemperature(LRADC_CHANNEL BattTempChannel);
    
    UINT32 MeasureDieTemperature();
    UINT32 MeasureVDD5V();
    VOID EnableTempSensorCurrent(LRADC_TEMPSENSOR eSensor,BOOL bValue);
    VOID SetTempSensorCurrent(LRADC_TEMPSENSOR TempSensor,LRADC_CURRENTMAGNITUDE Current);
    VOID EnableTempSensor(BOOL bEnable);

    //Touch Related functions
    BOOL GetTouchDetectPresent(VOID);
    BOOL EnableTouchDetect(BOOL bValue);
    BOOL EnableTouchDetectInterrupt(BOOL bValue);
    BOOL GetTouchDetectInterruptFlag(VOID);
    BOOL ClearTouchDetectInterruptFlag(VOID);
    
    BOOL EnableButtonDetect(LRADC_BUTTON button,BOOL bValue);
    BOOL EnableButtonDetectInterrupt(LRADC_BUTTON button,BOOL bValue);
    BOOL ClearButtonDetectInterruptFlag(LRADC_BUTTON button);
    
    BOOL EnableTouchDetectXDrive(BOOL Use5WireTouch,BOOL bValue);
    BOOL EnableTouchDetectYDrive(BOOL Use5WireTouch,BOOL bValue);
    BOOL GetTouchDetect(VOID);
    UINT16 TouchReadX(LRADC_CHANNEL Channel);
    UINT16 TouchReadY(LRADC_CHANNEL Channel);

    BOOL DumpRegister(VOID);

private:

public:

};



//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

extern BOOL BSPLardcEnableClock(UINT32 index, BOOL bEnable);
extern BOOL BSPLardcGetModuleClock(UINT32 index, PDWORD pdwFreq);

#ifdef __cplusplus
}
#endif

#endif   // __LRADC_CLASS_H

