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
// Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
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

        fsl_usbotg.cpp

Abstract:

        MX233 USB OTG Driver.

--*/
#include <windows.h>
#include <types.h>
#include <nkintr.h>
#include <ceddk.h>
#include <ddkreg.h>

#pragma warning(push)
#pragma warning(disable: 4100 4512 4245 6258 6262 6287)
#include <mx28_usb.h>
#include <fsl_usbotg.h>
#include "csp.h"
#include <..\USBD\OS\oscheckkitl.c>
#pragma warning(pop)

#pragma warning(disable: 4100 4065 6258 6262 6287)

#ifdef DEBUG
const DWORD cISTTimeOut = (30*1000);
#else
const DWORD cISTTimeOut = INFINITE;
#endif

LPCTSTR g_mycppOtgStateString[] = {
    TEXT("USBOTG_states_unknown"),
// A Port States
    TEXT("USBOTG_a_idle"),
    TEXT("USBOTG_a_wait_vrise"),
    TEXT("USBOTG_a_wait_bcon"),
    TEXT("USBOTG_a_host"),
    TEXT("USBOTG_a_suspend"),
    TEXT("USBOTG_a_peripheral"),
    TEXT("USBOTG_a_wait_vfall"),
    TEXT("USBOTG_a_vbus_err"),
// B Port States
    TEXT("USBOTG_b_idle"),
    TEXT("USBOTG_b_srp_init"),
    TEXT("USBOTG_b_peripheral"),
    TEXT("USBOTG_b_wait_acon"),
    TEXT("USBOTG_b_host")
};

extern "C" void BSPUSBSwitchModulePower(BOOL bOn);
extern "C" void BSPUsbPhyStartUp(void);
extern "C" void DumpUSBRegs(PUCHAR baseMem);
extern "C" BOOL USBClockInit();
extern "C" void BSPUsbPhyRegDump(void);
extern "C" void BSPUsbhPutPhySuspend(PUCHAR baseMem, BOOL bOn);
extern "C" void BSPUsbPhyGoDown(void);
extern "C" void BSPUsbPhyExit(void);

                     // DPIS     BSEIS     BSVIS     ASVIS     AVVIS       IDIS
//#define OTG_IRQ_MASK ((1<<22) | (1<<20) | (1<<19) | (1<<18) | (1<<17) | (1<<16))
#define OTG_IRQ_MASK (1<<16)

CSTMPOTG::CSTMPOTG(LPCTSTR lpActivePath)
: USBOTG(lpActivePath)
, CIST(lpActivePath, cISTTimeOut)   // cISTTimeOut is a global variable "INFINITE"
{
    m_hParent = CreateBusAccessHandle(lpActivePath);
    m_hOTGFeatureEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    m_ActiveKeyPath = NULL;

    if (lpActivePath)
    {
        DWORD dwLength = _tcslen(lpActivePath) + 1;
        m_ActiveKeyPath = new TCHAR [dwLength];
        if (m_ActiveKeyPath)
            StringCchCopy(m_ActiveKeyPath, dwLength, lpActivePath);
    }

    m_pvHWregPINCTRL = NULL;

    m_bHostDriverRunning = FALSE;
    m_bDevDriverRunning = FALSE;
}

BOOL CSTMPOTG::Init()
{
    m_SyncAccess.Lock();
    BOOL fRet = FALSE;
    if (!FslUfnIsUSBKitlEnable()
            && m_ActiveKeyPath != NULL && m_hOTGFeatureEvent != NULL
            && USBOTG::Init()
            && CIST::Init()
            && MapHardware()
            && IsIISRLoaded())
    {
        // OTG need USB module working before judging weather
        // UFN or HCD should be loaded, so hardware initialization
        // is necessary
        BSPUsbPhyStartUp();
        BSPUSBSwitchModulePower(TRUE);
        USBClockInit();
        fRet = TRUE;

        //clear pending otgsc irq;
        USB_OTGSC_T otgsc;
        DWORD *pTemp = (DWORD*)&otgsc;
        *pTemp = INREG32(&m_pUsbReg->OTG.OTGSC);
        OUTREG32(&m_pUsbReg->OTG.OTGSC, *pTemp);

        // Setup IISR
        GIISR_INFO Info;
        Info.SysIntr = CIST::GetSysIntr();
        Info.CheckPort = TRUE;
        Info.PortIsIO = FALSE;
        Info.UseMaskReg = FALSE;
        Info.PortAddr = (DWORD) m_pUsbStaticAddr + offsetof(CSP_USB_REG, OTGSC); // Offset to udc_otgisr
        Info.Mask = OTG_IRQ_MASK;
        Info.PortSize = sizeof(DWORD);
        if (fRet && !IntChainHandlerIoControl(IOCTL_GIISR_INFO, &Info, sizeof(Info), NULL, 0, NULL))
        {
            fRet = FALSE;
        }
        if (fRet && !ConfigurePinout())
        {
            fRet = FALSE;
        }
    }
    m_SyncAccess.Unlock();
    return fRet;
}

BOOL CSTMPOTG::PostInit()
{
    m_SyncAccess.Lock();

    // Step 1. Clear USBSTS and Disable all STS related
    OUTREG32(&(m_pUsbReg->OTG.USBINTR), 0); //disable all interrupt

    DWORD sts = INREG32(&(m_pUsbReg->OTG.USBSTS));
    OUTREG32(&(m_pUsbReg->OTG.USBSTS), sts); //clear all interrupt

    // Step 2. Enable OTG Interrupt.

    // Configure GPIO ID Pin
    DDKIomuxSetPinMux(DDK_IOMUX_USB0_ID_1, DDK_IOMUX_MODE_01);
    DDKIomuxSetPadConfig(DDK_IOMUX_USB0_ID_1,
                         DDK_IOMUX_PAD_DRIVE_8MA,
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    //DDKIomuxEnablePullup(DDK_IOMUX_USB_OTG_ID, FALSE);

    // Enable IDIE
    USB_OTGSC_T otgsc;
    DWORD *pTemp = (DWORD*)&otgsc;
    *pTemp = INREG32(&m_pUsbReg->OTG.OTGSC);
    otgsc.IDIE = 1;
    OUTREG32(&m_pUsbReg->OTG.OTGSC, *pTemp);

    // Connect SYSINTR and EVENT
    CIST::IntializeInterrupt();

    // In USBOTG::PostInit, state diagram will be turned on
    USBOTG::PostInit();
    
    m_SyncAccess.Unlock();
    return TRUE;
}

CSTMPOTG::~CSTMPOTG()
{
    m_bTerminated = FALSE;

    if (m_hParent)
    {
        ::SetDevicePowerState(m_hParent, D4, NULL);
        CloseBusAccessHandle(m_hParent);
    }

    if (m_pUsbReg != NULL)
        MmUnmapIoSpace((PVOID)m_pUsbReg, sizeof(CSP_USB_REGS));

    if (m_ActiveKeyPath)
        delete[] m_ActiveKeyPath;

    // release resources got in USBPHY driver
    BSPUsbPhyExit();
}

//
// device is turn off by Power Down, we need reintialize everything
BOOL CSTMPOTG::PowerUp()
{
    return TRUE;
}

BOOL CSTMPOTG::PowerDown()
{
    return TRUE;
}

BOOL CSTMPOTG::MapHardware()
{
    DDKWINDOWINFO dwi;
    if (GetWindowInfo(&dwi) == ERROR_SUCCESS && dwi.dwNumMemWindows != 0)
    {
        if (dwi.memWindows[0].dwBase && dwi.memWindows[0].dwLen >= sizeof(CSP_USB_REGS))
        {
            PHYSICAL_ADDRESS ioPhysicalBase = {dwi.memWindows[0].dwBase, 0};

            ULONG AddressSpace = 0;
            if (!BusTransBusAddrToVirtual(m_hParent, Internal, 0, ioPhysicalBase, sizeof(CSP_USB_REGS), &AddressSpace, (PPVOID)&m_pUsbReg) || AddressSpace != 0)
            {
                m_pUsbReg = NULL;
            }

            AddressSpace = 0;

            if (!BusTransBusAddrToStatic(m_hParent, Internal, 0, ioPhysicalBase, sizeof(CSP_USB_REGS), &AddressSpace, &m_pUsbStaticAddr) || AddressSpace != 0)
            {
                m_pUsbStaticAddr = NULL;
            }

            PHYSICAL_ADDRESS PhysAddr;
            PhysAddr.QuadPart = CSP_BASE_REG_PA_PINCTRL;
            m_pvHWregPINCTRL = (PVOID)MmMapIoSpace(PhysAddr, 0x1000, FALSE);
            if (m_pvHWregPINCTRL == NULL)
            {
                RETAILMSG(1, (TEXT("OTGPinIntialize:: m_pvHWregPINCTRL NULL\r\n")));
            }
        }
    }

    ASSERT(m_pUsbReg != NULL && m_pUsbStaticAddr != NULL && m_pvHWregPINCTRL != NULL);
    return (m_pUsbReg != NULL && m_pUsbStaticAddr != NULL && m_pvHWregPINCTRL != NULL);
}

BOOL CSTMPOTG::ConfigurePinout()
{
    return TRUE;
}

BOOL CSTMPOTG::IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    BOOL bReturn = FALSE;
    switch (dwCode)
    {
    default:
        bReturn = USBOTG::IOControl(dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
        break;
    }
    ASSERT(bReturn);
    return bReturn;
}


// OTG PDD Function.
BOOL CSTMPOTG::SessionRequest(BOOL fPulseLocConn, BOOL fPulseChrgVBus)
{
    USB_OTGSC_T otgsc;
    DWORD *pTemp = (DWORD*)&otgsc;

    m_SyncAccess.Lock();
    if (fPulseLocConn)
    {
        *pTemp = INREG32(&m_pUsbReg->OTG.OTGSC);
        otgsc.DP = 1;
        OUTREG32(&m_pUsbReg->OTG.OTGSC, *pTemp);
        Sleep(10);
        otgsc.DP = 0;
        OUTREG32(&m_pUsbReg->OTG.OTGSC, *pTemp);
    }
    if (fPulseChrgVBus)
    {
        *pTemp = INREG32(&m_pUsbReg->OTG.OTGSC);
        otgsc.VC = 1;
        OUTREG32(&m_pUsbReg->OTG.OTGSC, *pTemp);
        Sleep(10);
        otgsc.VC = 0;
        OUTREG32(&m_pUsbReg->OTG.OTGSC, *pTemp);
    }
    m_SyncAccess.Unlock();
    return TRUE;
}

BOOL CSTMPOTG::NewStateAction(USBOTG_STATES usbOtgState, USBOTG_OUTPUT usbOtgOutput)
{
    USB_OTGSC_T otgsc;
    DWORD *pTemp = (DWORD*)&otgsc;

    *pTemp = INREG32(&m_pUsbReg->OTG.OTGSC);

    if (!USBOTG::NewStateAction(usbOtgState, usbOtgOutput))
    {
        return FALSE;
    }

    if (m_OldStates == usbOtgState)
    {
        return TRUE;
    }

    m_SyncAccess.Lock();

    // RETAILMSG(1, (L"\t---- update m_OldStates from \"%s\" to \"%s\"\r\n", 
    //             g_mycppOtgStateString[m_OldStates], g_mycppOtgStateString[usbOtgState]));

    m_OldStates = usbOtgState;

    // When otg state goes to "USBOTG_b_peripheral", we unload HCD driver & load UFN driver
    // when otg state goes to "USBOTG_a_host" we unload UFN driver & load HCD driver
    if (usbOtgState == USBOTG_b_idle)
    {
        if (m_bHostDriverRunning)
        {
            RETAILMSG(1, (L"\tOTG : Unload HCD\r\n"));
            m_bHostDriverRunning = FALSE;
            LoadUnloadHCD(FALSE);
            OUTREG32(&(m_pUsbReg->OTG.USBINTR), 0); //disable all interrupt
        }

        Sleep(2000);

        if (!m_bDevDriverRunning)
        {
            RETAILMSG(1, (L"\tOTG : Load UFN\r\n"));
            m_bDevDriverRunning = TRUE;
            LoadUnloadUSBFN(TRUE);
        }
    }

    if (usbOtgState == USBOTG_a_idle)
    {

        if (m_bDevDriverRunning)
        {
            RETAILMSG(1, (L"\tOTG : Unload UFN\r\n"));
            m_bDevDriverRunning = FALSE;
            LoadUnloadUSBFN(FALSE);
            OUTREG32(&(m_pUsbReg->OTG.USBINTR), 0); //disable all interrupt
        }

        Sleep(2000);

        if (!m_bHostDriverRunning)
        {
            RETAILMSG(1, (L"\tOTG : Load HCD\r\n"));
            m_bHostDriverRunning = TRUE;
            LoadUnloadHCD(TRUE);
        }
    }

    m_SyncAccess.Unlock();
    return TRUE;
}

BOOL CSTMPOTG::IsSE0()
{
    return FALSE;
}

BOOL CSTMPOTG::UpdateInput()
{
    USB_OTGSC_T otgsc;
    DWORD* temp = (DWORD*)&otgsc;

    m_SyncAccess.Lock();

    if (m_UsbOtgOutputValues.bit.loc_con || m_UsbOtgOutputValues.bit.loc_sof)
        m_UsbOtgInput.bit.b_conn = 1;
    else
        m_UsbOtgInput.bit.b_conn = !IsSE0(); // Always True?

    if (m_UsbOtgState == USBOTG_a_idle)
    {
        // we should let state auto transfer to a_host
        m_UsbOtgInput.bit.a_bus_req = 1;
        m_UsbOtgInput.bit.a_vbus_vld = 1;
    }

    *temp = INREG32(&m_pUsbReg->OTG.OTGSC);

    // Retrive OTGSC value
    m_UsbOtgInput.bit.a_sess_vld = otgsc.ASV;
    m_UsbOtgInput.bit.b_sess_end = otgsc.BSE;
    m_UsbOtgInput.bit.b_sess_vld = otgsc.BSV;
    m_UsbOtgInput.bit.id = otgsc.ID;
    m_UsbOtgInput.bit.a_vbus_vld = otgsc.AVV;
    m_SyncAccess.Unlock();

    return TRUE;
}

BOOL CSTMPOTG::ISTProcess()
{
    USB_OTGSC_T otgsc;
    DWORD *pTemp = (DWORD*)&otgsc;

    *pTemp = INREG32(&m_pUsbReg->OTG.OTGSC);
    OUTREG32(&m_pUsbReg->OTG.OTGSC, *pTemp);

    UpdateInput();
    EventNotification();

    return TRUE;
}

BOOL CSTMPOTG::ISTTimeout()
{
    /*Is it necessary?*/
    UpdateInput();
    EventNotification();
    return TRUE;
}

// Class Factory.
USBOTG* CreateUSBOTGObject(LPTSTR lpActivePath)
{
    return new CSTMPOTG(lpActivePath);
}

void DeleteUSBOTGObject(USBOTG* pUsbOtg)
{
    if (pUsbOtg)
        delete pUsbOtg;
}
