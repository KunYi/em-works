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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

//------------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------


/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

        fsl_usbotg.h

Abstract:

        iMX233 USB OTG Driver Header.

--*/

#pragma once

#include <usbotg.hpp>
#include <ist.hpp>


class CSTMPOTG :public USBOTG, public CIST 
{
public:
    CSTMPOTG (LPCTSTR lpActivePath);
    ~CSTMPOTG ();
    BOOL Init();
    BOOL PostInit();
    BOOL MapHardware();
    BOOL ConfigurePinout();

    // Overwrite
    virtual BOOL IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);
    virtual BOOL PowerUp();
    virtual BOOL PowerDown();

    // OTG PDD Function.
    BOOL SessionRequest(BOOL fPulseLocConn, BOOL fPulseChrgVBus);
    BOOL DischargVBus() {return TRUE;};
    BOOL NewStateAction(USBOTG_STATES usbOtgState, USBOTG_OUTPUT usbOtgOutput);
    BOOL IsSE0();
    BOOL UpdateInput();
// BOOL StateChangeNotification (USBOTG_TRANSCEIVER_STATUS_CHANGE, USBOTG_TRANSCEIVER_STATUS);
    USBOTG_MODE UsbOtgConfigure(USBOTG_MODE usbOtgMode) {return usbOtgMode;};

protected:
    HANDLE m_hParent;
    HANDLE m_hOTGFeatureEvent;
    LPTSTR m_ActiveKeyPath;
    USBOTG_STATES m_OldStates;

    CSP_USB_REGS *m_pUsbReg;
    PVOID m_pUsbStaticAddr;// Used by Installable ISR
    PVOID m_pvHWregPINCTRL;

private:
    virtual BOOL ISTProcess();
    virtual BOOL ISTTimeout();

    // The following 2 variable is used to avoid unnecessary load/unload of 
    // UFN/HCD driver
    BOOL m_bHostDriverRunning;
    BOOL m_bDevDriverRunning;
};


