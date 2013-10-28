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
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//------------------------------------------------------------------------------
//
//  File: otgtransceiver_musb.hpp
//

#include "omap35xx_musbotg.h"

class HSUSBOTGTransceiver
{
public:
    HSUSBOTGTransceiver() {};
    ~HSUSBOTGTransceiver() {};
    virtual void EnableVBusDischarge(BOOL fDischarge) = 0;
    virtual void SetVBusSource(BOOL fBat) = 0;
    virtual void EnableWakeupInterrupt(BOOL fEnable) = 0;
    virtual void AconnNotifHandle(HANDLE hAconnEvent) = 0;
    virtual BOOL UpdateUSBVBusSleepOffState(BOOL fActive) = 0;
    virtual BOOL SupportsTransceiverWakeWithoutClock() = 0;
    virtual BOOL SetLowPowerMode() = 0;
    virtual BOOL IsSE0() = 0;
    virtual void DumpULPIRegs() = 0;
    virtual void Reset() = 0;
    virtual BOOL ResetPHY() = 0;
    virtual BOOL IsADeviceConnected() = 0;
    virtual BOOL IsBDeviceConnected() = 0;
};

EXTERN_C HSUSBOTGTransceiver * CreateHSUSBOTGTransceiver(PHSMUSB_T pOTG);

