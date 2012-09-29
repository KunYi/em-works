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
// Module Name:  
//     USBOTG.cpp
// 
// Abstract: Provides Liberary for Bus Access.
// 
// Notes: 
//
//------------------------------------------------------------------------------
// Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4512 4245 4189 4100 4083)

#include <windows.h>
#include <types.h>
#include <usbotg.hpp>
/* Debug Zones.
 */
#ifdef DEBUG

    #define DBG_INIT    0x0001
    #define DBG_OPEN    0x0002
    #define DBG_READ    0x0004
    #define DBG_WRITE   0x0008
    #define DBG_CLOSE   0x0010
    #define DBG_IOCTL   0x0020
    #define DBG_THREAD  0x0040
    #define DBG_TRANS   0x0080
    #define DBG_STATE   0x1000
    #define DBG_FUNCTION 0x2000
    #define DBG_WARNING 0x4000
    #define DBG_ERROR   0x8000

DBGPARAM dpCurSettings = {
    TEXT("USBOTG"), {
        TEXT("Init"), TEXT("Open"), TEXT("Read"), TEXT("Write"),
        TEXT("Close"), TEXT("Ioctl"), TEXT("Thread"), TEXT("TransCeiver"),
        TEXT("Unspecify"), TEXT("Unspecify"), TEXT("Unspecify"), TEXT("Unspecify"),
        TEXT("State"), TEXT("Function"), TEXT("Warning"), TEXT("Error")},
    0xc001
};

LPCTSTR USBOTG::m_cppOtgStateString[] = {
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

#endif

USBOTGWatchDog::USBOTGWatchDog(USBOTG * pUsbOtg)
: m_pUsbOtg (pUsbOtg)
, CMiniThread(0, TRUE)
{
    m_TimeoutTicks = INFINITE;
    m_WatchingItem = m_WatchItem = unkown_usbotg_watch_item;
    m_hEvent = CreateEvent(0, FALSE, FALSE, NULL);
}

USBOTGWatchDog::~USBOTGWatchDog()
{
    // Terminationg thread.
    m_bTerminated = TRUE;
    ThreadStart();
    WatchForItem(unkown_usbotg_watch_item, 0);
    ThreadTerminated(1000);
    if (m_hEvent)
        CloseHandle (m_hEvent);
}

BOOL USBOTGWatchDog::Init()
{
    ThreadStart();
    return (m_hEvent != NULL && m_pUsbOtg != NULL);
}

BOOL USBOTGWatchDog::WatchForItem(USBOTG_WATCHITEM watchItem, DWORD dwTicks)
{
    BOOL bReturn = FALSE;
    Lock();
    if (m_hEvent)
    {
        m_WatchingItem = m_WatchItem = watchItem;
        m_TimeoutTicks = dwTicks;
        SetEvent(m_hEvent);
    }
    Unlock();
    return bReturn;
}

DWORD USBOTGWatchDog::ThreadRun()
{
    while (!m_bTerminated)
    {
        PREFAST_ASSERT(m_hEvent != NULL);
        PREFAST_ASSERT(m_pUsbOtg != NULL);

        Lock();
        DWORD dwTimeoutTicks = m_TimeoutTicks;
        USBOTG_WATCHITEM usbOtgWatchItem = m_WatchingItem = m_WatchItem;
        m_WatchItem = unkown_usbotg_watch_item;
        m_TimeoutTicks = INFINITE;
        Unlock();

        if (WaitForSingleObject(m_hEvent, dwTimeoutTicks) != WAIT_OBJECT_0)
        {
            // Timeout.
            m_pUsbOtg->TimeOut(usbOtgWatchItem);
        }
    }
    return 1;
}

// We actually have a 64K Stack Size, so we can ignore prefast warning 6262
PREFAST_SUPPRESS(6262,"This Warning can be skipped!")
USBOTG::USBOTG(LPCTSTR lpActivePath)
: DefaultBusDriver(lpActivePath)
, CRegistryEdit(lpActivePath)
{
    m_HCD_Registry = NULL;
    m_USBFN_Registry = NULL;
    // Get DEVLOAD_DEVKEY_VALNAME name
    m_lpActiveRegPath = NULL;
    m_lpBusName = NULL;
    m_dwBusNumber = 0;
    m_iThreadPriority = USBOTG_DEFAULT_THREAD_PRIORITY;
    m_hAConnectEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    HANDLE hThisDevice = GetDeviceHandleFromContext(lpActivePath);
    if(hThisDevice != NULL)
    {
        DEVMGR_DEVICE_INFORMATION di;
        memset(&di, 0, sizeof(di));
        di.dwSize = sizeof(di);
        if(GetDeviceInformationByDeviceHandle(hThisDevice, &di))
        {
            DWORD dwKeyLen = wcslen(di.szDeviceKey) + 2;
            m_lpActiveRegPath = new TCHAR [dwKeyLen];
            if (m_lpActiveRegPath)
            {
                StringCchCopy(m_lpActiveRegPath, dwKeyLen, di.szDeviceKey);
                StringCchCat(m_lpActiveRegPath, dwKeyLen, TEXT("\\"));
            }
        }
    }
    TCHAR busNameString[MAX_PATH];
    DWORD dwType;
    DWORD dwLength = sizeof(busNameString);
    if (RegQueryValueEx(DEVLOAD_BUSNAME_VALNAME, &dwType, (PBYTE)busNameString, &dwLength) && dwType == DEVLOAD_BUSNAME_VALTYPE)
    {
        dwLength /= sizeof(TCHAR);
        m_lpBusName = new TCHAR [dwLength + 1];
        if (m_lpBusName)
        {
            StringCchCopy(m_lpBusName, dwLength + 1, busNameString);
        }
    };
    m_UsbOtgTimers.a_aidl_bdis_tmr = USBOTG_TA_AIDL_BDIS_DEFAULT;
    m_UsbOtgTimers.a_wait_bcon_tmr = USBOTG_TA_WAIT_BCON_DEFAULT;
    m_UsbOtgTimers.a_wait_vrise_tmr = USBOTG_TA_WAIT_VRISE_DEFAULT;
    m_UsbOtgTimers.b_ase0_brst_tmr = USBOTG_TB_ASE0_BRST_DEFAULT;
    m_UsbOtgTimers.b_srp_fail_tmr = USBOTG_TB_SRP_FAIL_DEFAULT;

    m_UsbOtgState = USBOTG_states_unknown;
    m_UsbOtgMode = USBOTG_mode_unknow;
    m_dwDynamicClietLoad = 0;
    m_fDisalbeRoleSwitch = FALSE;
    m_bHCDPortOnOTG = 1;
    m_pUSBOTGWatchDog = NULL;

    memset(&m_PowerCapabilities, 0, sizeof(m_PowerCapabilities));
    m_PowerCapabilities.DeviceDx = DX_MASK(D0)|DX_MASK(D3)|DX_MASK(D4);
    m_RequestedPowerState = D0;
    m_IsThisPowerManaged = FALSE;
    m_WaitSRPCount = 0;
}

USBOTG::~USBOTG()
{
    if (m_pUSBOTGWatchDog)
        delete m_pUSBOTGWatchDog;
    if (m_HCD_Registry != NULL)
        delete m_HCD_Registry;
    if (m_USBFN_Registry != NULL)
        delete m_USBFN_Registry;
    if (m_lpActiveRegPath)
        delete m_lpActiveRegPath;
    if (m_lpBusName)
        delete m_lpBusName;
    if (m_hAConnectEvent)
        CloseHandle(m_hAConnectEvent);
}

BOOL USBOTG::Init()
{
    TakeWriteLock();
    BOOL bReturn = FALSE;

    m_pUSBOTGWatchDog = new USBOTGWatchDog(this);
    if (m_pUSBOTGWatchDog && m_pUSBOTGWatchDog->Init() && DefaultBusDriver::Init() && IsKeyOpened())
    {
        if (!GetRegValue(USBOTG_DYNAMIC_CLIENT_LOAD, (LPBYTE) &m_dwDynamicClietLoad, sizeof(m_dwDynamicClietLoad)))
        {
            m_dwDynamicClietLoad = 0;
        }

        DWORD dwValue = 0;
        if (GetRegValue(USBOTG_DISABLE_ROLE_SWITCH, (LPBYTE) &dwValue, sizeof(dwValue)) && dwValue != 0)
        {
            m_fDisalbeRoleSwitch = TRUE;
        }

        dwValue = 1;
        if (GetRegValue(USBOTG_HCD_PORT_NUMBER_ON_OTG, (LPBYTE) &dwValue, sizeof(dwValue)))
        {
            m_bHCDPortOnOTG = (BYTE)dwValue;
        };

        dwValue = 0;
        if (GetRegValue(USBOTG_THREAD_PRIORITY, (LPBYTE)&dwValue, sizeof(dwValue)))
        {
            m_iThreadPriority = (INT)dwValue;
        }

        m_HCD_Registry = new CRegistryEdit(GetHKey(), USBOTG_HOST_DRIVER);
        if (m_HCD_Registry != NULL && !m_HCD_Registry->IsKeyOpened())
        {
            // Can't open HCD Sub Key.
            delete m_HCD_Registry;
            m_HCD_Registry = NULL;
        }

        m_USBFN_Registry = new CRegistryEdit(GetHKey(), USBOTG_FUNC_DRIVER);
        if (m_USBFN_Registry != NULL && !m_USBFN_Registry->IsKeyOpened())
        {
            // Can't open USBFN Sub Key
            delete m_USBFN_Registry;
            m_USBFN_Registry = NULL;
        }

        if (m_HCD_Registry != NULL || m_USBFN_Registry != NULL)
        {
            // At least one registry been specified.
            DWORD dwKeyLen = 0;
            if (!GetRegValue(DEVLOAD_INTERFACETYPE_VALNAME, (PUCHAR) &m_dwBusType, sizeof(m_dwBusType)))
            {
                m_dwBusType = InterfaceTypeUndefined; // No interface type defined
            }
            if (!GetRegValue(DEVLOAD_BUSNUMBER_VALNAME, (LPBYTE)&m_dwBusNumber, sizeof(m_dwBusNumber)))
            {
                m_dwBusNumber = 0;
            }

            if (m_HCD_Registry)
            {
                // We Create Foler for this driver.
                TCHAR lpChildPath[DEVKEY_LEN];
                StringCchCopy(lpChildPath, DEVKEY_LEN, m_lpActiveRegPath);
                StringCchCat(lpChildPath, DEVKEY_LEN, USBOTG_HOST_DRIVER);
                DeviceFolder * nDevice = CreateHostDeviceFolder(m_lpBusName != NULL?m_lpBusName:USBOTG_DEFAULTOTG_BUSNAME,
                    lpChildPath, m_dwBusType, m_dwBusNumber, USBOTG_HCD_DRIVER_DEVICE_INDEX, 0, GetDeviceHandle());
                if (nDevice) InsertChild(nDevice);

                // Get Registry Setting That related to HCD
                GetRegValue(REG_TA_AIDL_BDIS_VAL, (PBYTE)&m_UsbOtgTimers.a_aidl_bdis_tmr, sizeof(m_UsbOtgTimers.a_aidl_bdis_tmr));
                GetRegValue(REG_TA_WAIT_BCON_VAL, (PBYTE)&m_UsbOtgTimers.a_wait_bcon_tmr, sizeof(m_UsbOtgTimers.a_wait_bcon_tmr));
                GetRegValue(REG_TA_WAIT_VRISE_VAL, (PBYTE)&m_UsbOtgTimers.a_wait_vrise_tmr, sizeof(m_UsbOtgTimers.a_wait_vrise_tmr));
                GetRegValue(REG_TB_SRP_FAIL_VAL, (PBYTE)&m_UsbOtgTimers.b_srp_fail_tmr, sizeof(m_UsbOtgTimers.b_srp_fail_tmr));

                if (m_UsbOtgMode == USBOTG_mode_unknow)
                    m_UsbOtgMode = USBOTG_Host;
                if (m_UsbOtgMode == USBOTG_Function)
                    m_UsbOtgMode = USBOTG_Bidirection;
            }

            if (m_USBFN_Registry)
            {
                // We Create Foler for this driver.
                TCHAR lpChildPath[DEVKEY_LEN];
                StringCchCopy(lpChildPath, DEVKEY_LEN, m_lpActiveRegPath);
                StringCchCat(lpChildPath, DEVKEY_LEN, USBOTG_FUNC_DRIVER);
                DeviceFolder * nDevice = CreateFunctionDeviceFolder (m_lpBusName != NULL?m_lpBusName:USBOTG_DEFAULTOTG_BUSNAME,
                    lpChildPath, m_dwBusType, m_dwBusNumber, USBOTG_USBFN_DRIVER_DEVICE_INDEX, 0, GetDeviceHandle());
                if (nDevice) InsertChild(nDevice);

                // Get Registry Setting That related to UFN
                GetRegValue(REG_TB_ASE0_BRST_VAL, (PBYTE)&m_UsbOtgTimers.b_ase0_brst_tmr, sizeof(m_UsbOtgTimers.b_ase0_brst_tmr));

                if (m_UsbOtgMode == USBOTG_mode_unknow)
                    m_UsbOtgMode = USBOTG_Function;
                if (m_UsbOtgMode == USBOTG_Host)
                    m_UsbOtgMode = USBOTG_Bidirection;
            }

            m_UsbOtgInput.ul = 0;
            m_UsbOtgInternal.ul = 0;
            m_UsbOtgOutputValues.ul = 0;

            m_UsbOtgState = (m_HCD_Registry != NULL? USBOTG_a_idle: USBOTG_b_idle);

            m_UsbOtgTimers.b_srp_fail_count = m_UsbOtgTimers.b_srp_fail_tmr / USBOTG_TB_SRP_INIT_DEFAULT;
            m_UsbOtgTimers.b_srp_fail_count++;
            m_WaitSRPCount = 0;

            m_UsbOtgInternal.bit.b_hnp_enable = 0;

            if (m_HCD_Registry)
            {
                DWORD dwInitMaster = 0;
                if (GetRegValue(USBOTG_INITIAL_ROLE_MASTER, (PBYTE)&dwInitMaster, sizeof(dwInitMaster)) && dwInitMaster != 0)
                {
                    if (!m_fDisalbeRoleSwitch)
                        m_UsbOtgInput.bit.b_bus_req = 1;
                    m_UsbOtgInput.bit.a_bus_req = 1;
                }
            }
            UpdatePowerState();
            bReturn = TRUE;
        }
    }
    LeaveWriteLock();
    return bReturn;
}

DWORD USBOTG::GetBusNamePrefix(LPTSTR lpReturnBusName, DWORD dwSizeInUnit)
{
    DWORD dwCopyUnit = 0;
    if (lpReturnBusName && dwSizeInUnit)
    {
        lpReturnBusName[0] = 0;
        StringCchCopy(lpReturnBusName, dwSizeInUnit, m_lpBusName != NULL?m_lpBusName:USBOTG_DEFAULTOTG_BUSNAME);
        dwCopyUnit = _tcslen(lpReturnBusName);
    }
    return dwCopyUnit;
}

BOOL USBOTG::PostInit()
{
    BOOL bReturn = TRUE;
    m_SyncAccess.Lock();
    m_UsbOtgMode = UsbOtgConfigure(m_UsbOtgMode);
    ASSERT(m_UsbOtgMode != USBOTG_mode_unknow);
    bReturn = (m_UsbOtgMode != USBOTG_mode_unknow);
    if (bReturn)
    {
        if (GetDeviceHandle() != NULL && IsKeyOpened())
        {
            bReturn = ActivateAllChildDrivers();
        }
        EnterState(m_UsbOtgState);
        UpdateInput();
        EventNotification(); // Start State Diaglam
    }
    m_SyncAccess.Unlock();
    return bReturn;
}

BOOL USBOTG::ActivateAllChildDrivers()
{
    TakeReadLock();
    DeviceFolder* pCurDevice = GetDeviceList();
    // Activate Device
    DWORD dwCurOrder = 0;
    while (dwCurOrder != MAXDWORD)
    {
        DEBUGMSG(ZONE_OTG_INIT, (TEXT(" USBOTG::ActivateChild load device driver at order %d \r\n"), dwCurOrder));
        DWORD dwNextOrder = MAXDWORD;
        pCurDevice = GetDeviceList();
        while (pCurDevice)
        {
            DWORD dwDeviceLoadOrder = pCurDevice->GetLoadOrder();
            if (dwDeviceLoadOrder == dwCurOrder)
            {
                if ((pCurDevice->GetDeviceNumber() == USBOTG_HCD_DRIVER_DEVICE_INDEX &&
                        (m_dwDynamicClietLoad & USBOTG_HCD_DYNAMIC_LOAD) == 0))
                {
                    // No dynamic Load HCD
                    LoadUnloadHCD(TRUE);
                }
                if ((pCurDevice->GetDeviceNumber() == USBOTG_USBFN_DRIVER_DEVICE_INDEX &&
                        (m_dwDynamicClietLoad & USBOTG_USBFN_DYNAMIC_LOAD) == 0))
                {
                    // No dynamic load USBFN
                    LoadUnloadUSBFN(TRUE);
                }
            }
            else if (dwDeviceLoadOrder > dwCurOrder && dwDeviceLoadOrder < dwNextOrder)
            {
                dwNextOrder = dwDeviceLoadOrder;
            }
            pCurDevice = pCurDevice->GetNextDeviceFolder();
        }
        dwCurOrder = dwNextOrder;
    }
    LeaveReadLock();
    return TRUE;
}

DeviceFolder * USBOTG::GetChildByDeviceNumber(DWORD dwDeviceNumber)
{
    DeviceFolder* pReturn = NULL;
    TakeReadLock();
    if (GetDeviceList())
    {
        pReturn = GetDeviceList();
        while (pReturn != NULL)
        {
            if (pReturn->GetDeviceNumber() == dwDeviceNumber)
            {
                // IT is same, We found it.
                break;
            }
            else
            {
                pReturn = pReturn->GetNextDeviceFolder();
            }
        }
        if (pReturn)
            pReturn->AddRef();
    }
    LeaveReadLock();
    return pReturn;
}

BOOL USBOTG::LoadUnloadClientDriver(BOOL fLoad, DeviceFolder * pClientDriver)
{
    if (pClientDriver)
    {
        if (fLoad && pClientDriver->IsDriverLoaded() == FALSE)
        {
            pClientDriver->LoadDevice();
        }
        if (!fLoad && pClientDriver->IsDriverLoaded() == TRUE)
        {
            pClientDriver->UnloadDevice();
        }
        return TRUE;
    }
    return FALSE;
}

BOOL USBOTG::LoadUnloadHCD(BOOL fLoad)
{
    DeviceFolder * pDevice = GetChildByDeviceNumber(USBOTG_HCD_DRIVER_DEVICE_INDEX);
    BOOL bReturn = FALSE;
    if (fLoad || (m_dwDynamicClietLoad & USBOTG_HCD_DYNAMIC_LOAD) != 0)
    {
        if (pDevice)
        {
            bReturn = LoadUnloadClientDriver(fLoad, pDevice);
            pDevice->DeRef();
        }
    }
    return bReturn;
}

BOOL USBOTG::LoadUnloadUSBFN(BOOL fLoad)
{
    DeviceFolder * pDevice = GetChildByDeviceNumber(USBOTG_USBFN_DRIVER_DEVICE_INDEX);
    BOOL bReturn = FALSE;
    if (fLoad || (m_dwDynamicClietLoad & USBOTG_USBFN_DYNAMIC_LOAD) != 0)
    {
        if (pDevice)
        {
            bReturn = LoadUnloadClientDriver(fLoad, pDevice);
            pDevice->DeRef();
        }
    }
    return bReturn;
}

BOOL USBOTG::EnterState(USBOTG_STATES usbOtgState)
{
    DEBUGMSG(ZONE_OTG_STATE, (TEXT("+USBOTG::EnterState OtgState = %s, \r\n"), m_cppOtgStateString[m_UsbOtgState]));
    // RETAILMSG(1, (L"\t-+-+-+- USBOTG : State Change to \""));
    switch (usbOtgState)
    {
    case USBOTG_a_idle:
        // RETAILMSG(1, (L"USBOTG_a_idle"));
        m_UsbOtgInput.bit.a_srp_det = 0;
        m_UsbOtgInput.bit.a_bus_drop = 0;
        m_UsbOtgInternal.bit.a_set_b_hnp_en = 0;
        m_UsbOtgOutputValues.ul = 0;
        break;
    case USBOTG_a_wait_vrise:
        // RETAILMSG(1, (L"USBOTG_a_wait_vrise"));
        m_UsbOtgInput.bit.a_srp_det = 0;
        m_UsbOtgInput.bit.a_wait_vrise_tmout = 0;

        m_UsbOtgOutputValues.bit.drv_vbus = 1;
        m_UsbOtgOutputValues.bit.loc_con = 0;
        m_UsbOtgOutputValues.bit.loc_sof = 0;
        break;
    case USBOTG_a_wait_bcon:
        // RETAILMSG(1, (L"USBOTG_a_wait_bcon"));
        m_UsbOtgInput.bit.a_wait_bcon_tmout = 0;

        m_UsbOtgOutputValues.bit.drv_vbus = 1;
        m_UsbOtgOutputValues.bit.loc_con = 0;
        m_UsbOtgOutputValues.bit.loc_sof = 0;
        break;
    case USBOTG_a_host:
        // RETAILMSG(1, (L"USBOTG_a_host"));
        m_UsbOtgInput.bit.a_bus_drop = 0;
        m_UsbOtgInput.bit.b_bus_resume = 1;
        m_UsbOtgInput.bit.b_bus_suspend = 0;
        m_UsbOtgInput.bit.a_suspend_req = 0;
        m_UsbOtgInput.bit.b_conn = 1;

        m_UsbOtgOutputValues.bit.drv_vbus = 1;
        m_UsbOtgOutputValues.bit.loc_con = 0;
        m_UsbOtgOutputValues.bit.loc_sof = 1;
        break;
    case USBOTG_a_suspend:
        // RETAILMSG(1, (L"USBOTG_a_suspend"));
        m_UsbOtgInput.bit.b_bus_resume = 0;
        m_UsbOtgInput.bit.b_bus_suspend = 0;
        m_UsbOtgInput.bit.a_suspend_req = 1;
        m_UsbOtgInput.bit.a_aidl_bdis_tmout = 0;

        m_UsbOtgOutputValues.bit.drv_vbus = 1;
        m_UsbOtgOutputValues.bit.loc_con = 0;
        m_UsbOtgOutputValues.bit.loc_sof = 0;
        break;
    case USBOTG_a_peripheral:
        // RETAILMSG(1, (L"USBOTG_a_peripheral"));
        m_UsbOtgInput.bit.b_bus_suspend = 0;
        m_UsbOtgOutputValues.bit.drv_vbus = 1;
        m_UsbOtgOutputValues.bit.loc_con = 1;
        m_UsbOtgOutputValues.bit.loc_sof = 0;
        break;
    case USBOTG_a_wait_vfall:
        // RETAILMSG(1, (L"USBOTG_a_wait_vfall"));
        m_UsbOtgInput.bit.a_wait_bcon_tmout = 0;

        m_UsbOtgOutputValues.ul = 0;
        break;
    case USBOTG_a_vbus_err:
        // RETAILMSG(1, (L"USBOTG_a_vbus_err"));
        m_UsbOtgOutputValues.ul = 0;
        break;
// Default B Device.
    case USBOTG_b_idle:
        // RETAILMSG(1, (L"USBOTG_b_idle"));
        m_UsbOtgInternal.bit.b_hnp_enable = 0;
        m_UsbOtgInput.bit.a_conn = 0;
        m_UsbOtgInput.bit.b_hcd_accept = 0;
        m_UsbOtgOutputValues.ul = 0;
        break;
    case USBOTG_b_srp_init:
        // RETAILMSG(1, (L"USBOTG_b_srp_init"));
        m_UsbOtgInternal.bit.b_srp_done = 0;
        m_UsbOtgInput.bit.b_srp_fail_tmout = 0;
        m_UsbOtgInput.bit.b_se0_srp = 0;
        m_UsbOtgTimers.b_srp_fail_tmr = 0;

        m_UsbOtgOutputValues.bit.drv_vbus = 0;
        m_UsbOtgOutputValues.bit.loc_con = 0;
        m_UsbOtgOutputValues.bit.loc_sof = 0;
    case USBOTG_b_peripheral:
        // RETAILMSG(1, (L"USBOTG_b_peripheral"));
        m_UsbOtgOutputValues.bit.drv_vbus = 0;
        m_UsbOtgOutputValues.bit.loc_con = 1;
        m_UsbOtgOutputValues.bit.loc_sof = 0;
        break;
    case USBOTG_b_wait_acon:
        // RETAILMSG(1, (L"USBOTG_b_wait_acon"));
        m_UsbOtgInput.bit.b_ase0_brst_tmout = 0;

        m_UsbOtgOutputValues.bit.drv_vbus = 0;
        m_UsbOtgOutputValues.bit.loc_con = 0;
        m_UsbOtgOutputValues.bit.loc_sof = 0;
        break;
    case USBOTG_b_host:
        // RETAILMSG(1, (L"USBOTG_b_host"));

        m_UsbOtgOutputValues.bit.drv_vbus = 0;
        m_UsbOtgOutputValues.bit.loc_con = 0;
        m_UsbOtgOutputValues.bit.loc_sof = 1;
        break;
    }
    // RETAILMSG(1, (L"\"\r\n"));
    BOOL fContinue = NewStateAction(m_UsbOtgState, m_UsbOtgOutputValues);
    return fContinue;
}

USBOTG_STATES USBOTG::StateChange(USBOTG_STATES usbOtgState)
{
    // RETAILMSG(1, (L"\t-+-+-+- USBOTG : State Change from \""));
    switch (usbOtgState)
    {
    case USBOTG_states_unknown:
    default:
        // RETAILMSG(1, (L"USBOTG_states_unknown"));
        ASSERT(FALSE);
        break;
// A Port States
    case USBOTG_a_idle:
        // Possible goes to 1) b_idle
        //                  2) a_wait_vrise
        // RETAILMSG(1, (L"USBOTG_a_idle"));
        m_UsbOtgOutputValues.ul = 0;
        m_UsbOtgInternal.ul = 0;
        if (m_UsbOtgInput.bit.id)
        {
            // Swith to B.
            if (m_USBFN_Registry)
            {
                usbOtgState = USBOTG_b_idle;
            }
            else
            {
                ASSERT(FALSE);
            }
        }
        else
        {
            if (m_HCD_Registry)
            {
                // This is for testing.
                //m_UsbOtgInput.bit.a_bus_req = 1;
                //m_UsbOtgInput.bit.a_bus_drop = 0;
                //m_UsbOtgInput.bit.a_suspend_req = 0;
                //m_UsbOtgInternal.bit.a_set_b_hnp_en = 0;
            }

            if (m_UsbOtgInput.bit.a_bus_drop == 0 && (m_UsbOtgInput.bit.a_bus_req || m_UsbOtgInput.bit.a_srp_det))
            {
                usbOtgState = USBOTG_a_wait_vrise;
            }
        }
        break;
    case USBOTG_a_wait_vrise:
        // Possible goes to 1) a_wait_vfall
        //                  2) a_wait_bcon
        // RETAILMSG(1, (L"USBOTG_a_wait_vrise"));
        if (m_UsbOtgInput.bit.id || m_UsbOtgInput.bit.a_bus_drop || m_UsbOtgInput.bit.a_vbus_vld || m_UsbOtgInput.bit.a_wait_vrise_tmout)
        {
            usbOtgState = USBOTG_a_wait_bcon;
        }
        else
            m_pUSBOTGWatchDog->WatchForItem(a_wait_vrise, m_UsbOtgTimers.a_wait_vrise_tmr);
        break;
    case USBOTG_a_wait_bcon:
        // Possible goes to 1) a_wait_vfall
        //                  2) a_vbus_err
        //                  3) a_host
        // RETAILMSG(1, (L"USBOTG_a_wait_bcon"));
        if (m_UsbOtgInput.bit.a_vbus_vld == 0)
        {
            usbOtgState = USBOTG_a_vbus_err;
        }
        else if (m_UsbOtgInput.bit.id || m_UsbOtgInput.bit.a_bus_drop || m_UsbOtgInput.bit.a_wait_bcon_tmout)
        {
            usbOtgState = USBOTG_a_wait_vfall;
        }
        else if (m_UsbOtgInput.bit.b_conn)
        {
            usbOtgState = USBOTG_a_host;

        }
        else
            m_pUSBOTGWatchDog->WatchForItem(a_wait_bcon, m_UsbOtgTimers.a_wait_bcon_tmr);
        break;
    case USBOTG_a_host:
        // Possible goes to 1) a_wait_vfall
        //                  2) a_wait_bcon
        //                  3) a_vbus_err
        //                  4) a_suspend
        // RETAILMSG(1, (L"USBOTG_a_host"));
        if (m_UsbOtgInput.bit.a_vbus_vld == 0)
        {
            usbOtgState = USBOTG_a_vbus_err;
        }
        else if (m_UsbOtgInput.bit.id || m_UsbOtgInput.bit.b_conn == 0 || m_UsbOtgInput.bit.a_bus_drop)
        {
            usbOtgState = USBOTG_a_wait_bcon;
        }
        else if ((m_UsbOtgInput.bit.a_bus_req == 0 || m_UsbOtgInput.bit.a_suspend_req)
            && m_UsbOtgInternal.bit.a_set_b_hnp_en) // I think this has to be.
        {
            // The Spec said "||"
            usbOtgState = USBOTG_a_suspend;
        }
        break;
    case USBOTG_a_suspend:
        // RETAILMSG(1, (L"USBOTG_a_suspend"));
        if (m_UsbOtgInput.bit.a_vbus_vld == 0)
        {
            usbOtgState = USBOTG_a_vbus_err;

        }
        else if (m_UsbOtgInput.bit.b_conn == 0)
        {
            if (m_UsbOtgInternal.bit.a_set_b_hnp_en)
            {
                usbOtgState = USBOTG_a_peripheral;
            }
            else
            {
                usbOtgState = USBOTG_a_wait_bcon;
                m_UsbOtgInput.bit.a_wait_bcon_tmout = 0;
            }

        }
        else if (m_UsbOtgInput.bit.a_bus_req /*|| m_UsbOtgInput.bit.b_bus_resume*/)
        {
            // I don't believe b_bus_resume should be lock at it here.
            usbOtgState = USBOTG_a_host;
        }
        else if (m_UsbOtgInput.bit.id || m_UsbOtgInput.bit.a_bus_drop || m_UsbOtgInput.bit.a_aidl_bdis_tmout)
        {
            usbOtgState = USBOTG_a_wait_vfall;
            m_UsbOtgInput.bit.a_aidl_bdis_tmout = 0;
        }
        else
            m_pUSBOTGWatchDog->WatchForItem(a_aidl_bdis, m_UsbOtgTimers.a_aidl_bdis_tmr);
        break;
    case USBOTG_a_peripheral:
        // RETAILMSG(1, (L"USBOTG_a_peripheral"));
        if (m_UsbOtgInput.bit.a_vbus_vld == 0)
        {
            usbOtgState = USBOTG_a_vbus_err;
        }
        else if (m_UsbOtgInput.bit.id || m_UsbOtgInput.bit.a_bus_drop)
        {
            usbOtgState = USBOTG_a_wait_vfall;
        }
        else if (m_UsbOtgInput.bit.b_bus_suspend)
        {
            usbOtgState = USBOTG_a_wait_bcon;
        }
        break;
    case USBOTG_a_wait_vfall:
        // RETAILMSG(1, (L"USBOTG_a_wait_vfall"));
        if (m_UsbOtgInput.bit.id || m_UsbOtgInput.bit.a_bus_req || //I believe flow chart wrong here. We have to wait V bus to drop and device disconnected.
                (m_UsbOtgInput.bit.a_sess_vld == 0 && m_UsbOtgInput.bit.b_conn == 0)) {
            usbOtgState = USBOTG_a_idle;

        }
        break;
    case USBOTG_a_vbus_err:
        // RETAILMSG(1, (L"USBOTG_a_vbus_err"));
        RETAILMSG(1, (TEXT("USBOTG::EventNotification: VBUS ERROR entered.\n\r")));
        if (m_UsbOtgInput.bit.id || m_UsbOtgInput.bit.a_bus_drop || m_UsbOtgInput.bit.a_clr_err)
        {
            usbOtgState = USBOTG_a_wait_vfall;
        }
        break;
// B Port States
    case USBOTG_b_idle:
        // RETAILMSG(1, (L"USBOTG_b_idle"));
        m_UsbOtgOutputValues.ul = 0;
        if (m_UsbOtgInput.bit.id == 0)
        {
            if (m_HCD_Registry != NULL)
            {
                usbOtgState = USBOTG_a_idle;
            }
            else
            {
                ASSERT(FALSE);
            }
        }
        else
        {
            if (IsSE0())
                m_UsbOtgInput.bit.b_se0_srp = 1;

            if (m_UsbOtgInput.bit.b_sess_vld)
            {
                usbOtgState = USBOTG_b_peripheral;
            }
            else if (m_UsbOtgInput.bit.b_bus_req && m_UsbOtgInput.bit.b_sess_end && m_UsbOtgInput.bit.b_se0_srp)
            {
                if (++ m_WaitSRPCount >= m_UsbOtgTimers.b_srp_fail_count)
                {
                    RETAILMSG(1, (TEXT("SRP TB_SRT_FAIL has expired. Make sure the cable is connected!!!\r\n")));
                    m_WaitSRPCount = 0;
                    m_UsbOtgInternal.bit.b_srp_done = 1;
                    m_UsbOtgInput.bit.b_srp_fail_tmout = 1;
                    m_UsbOtgInput.bit.b_se0_srp = 0;
                    m_UsbOtgInput.bit.b_bus_req = 0;
                }
                else
                {
                    DEBUGMSG(ZONE_OTG_STATE, (TEXT("SRP TB_SRP has expired. Resend m_WaitSRPCount = %d!\r\n"), m_WaitSRPCount));
                    Sleep(USBOTG_TB_SE0_SRP_DEFAULT);
                    SessionRequest(TRUE, TRUE);
                    m_UsbOtgInput.bit.b_se0_srp = 1;
                    m_UsbOtgInternal.bit.b_srp_done = 0;
                    m_UsbOtgInput.bit.b_srp_fail_tmout = 0;
                    m_pUSBOTGWatchDog->WatchForItem(b_srp_init, USBOTG_TB_SRP_INIT_DEFAULT);
                }
            }
        }
        break;
    case USBOTG_b_peripheral:
        // RETAILMSG(1, (L"USBOTG_b_peripheral"));
        if (m_UsbOtgInput.bit.id == 0 || m_UsbOtgInput.bit.b_sess_vld == 0)
        {
            usbOtgState = USBOTG_b_idle;
        }
        else if (m_UsbOtgInput.bit.b_bus_req && m_UsbOtgInternal.bit.b_hnp_enable && m_UsbOtgInput.bit.a_bus_suspend && m_HCD_Registry != NULL)
        {
            usbOtgState = USBOTG_b_wait_acon;
        }
        break;
    case USBOTG_b_wait_acon:
        // RETAILMSG(1, (L"USBOTG_b_wait_acon"));
        if (m_UsbOtgInput.bit.id == 0 || m_UsbOtgInput.bit.b_sess_vld == 0)
        {
            usbOtgState = USBOTG_b_idle;
        }
        else if (m_UsbOtgInput.bit.a_conn)
        {
            usbOtgState = USBOTG_b_host;
        }
        else if (m_UsbOtgInput.bit.b_bus_req == 0 || m_UsbOtgInput.bit.b_ase0_brst_tmout)
        {
            // a_bus_resume coulbe be mistakenly generated by noise during the A-side switch.
            // It is very hard to rely on this signal.
            // I decided use b_bus_reques/ to terminate this because it is setup by user.
            usbOtgState = USBOTG_b_peripheral;
        }
        else
            m_pUSBOTGWatchDog->WatchForItem(b_ase0_brst, m_UsbOtgTimers.b_ase0_brst_tmr);
        break;
    case USBOTG_b_host:
        // RETAILMSG(1, (L"USBOTG_b_host"));
        if (m_UsbOtgInput.bit.id == 0 || m_UsbOtgInput.bit.b_sess_vld == 0)
        {
            usbOtgState = USBOTG_b_idle;
        }
        else if (m_UsbOtgInput.bit.b_bus_req == 0 || m_UsbOtgInput.bit.a_conn == 0)
        {
            usbOtgState = USBOTG_b_peripheral;
        }
    }
    // RETAILMSG(1, (L"\"\r\n"));
    return usbOtgState;
}

// Notification Processing. It implements 7.
BOOL USBOTG::EventNotification()
{
    BOOL bReturn = TRUE;
    m_SyncAccess.Lock();
    DEBUGMSG(ZONE_OTG_STATE, (TEXT("+USBOTG::EventNotification OtgState = %s, OtgInnput = 0x%x, OtgInternal = 0x%x"), m_cppOtgStateString[m_UsbOtgState], m_UsbOtgInput.ul, m_UsbOtgInternal.ul));
    BOOL fContinue = TRUE;

    // The loop following move along the state diagram once a time until no further state change
    do {
        UpdateInput();
        USBOTG_STATES newOTGState = StateChange(m_UsbOtgState);   // Move one step
        EnterState(newOTGState);
        if (newOTGState != m_UsbOtgState)
        {
            m_UsbOtgState = newOTGState;
            fContinue = NewStateAction(m_UsbOtgState, m_UsbOtgOutputValues);   // Looks Dummy, it is called once in EnterState
            DEBUGMSG(ZONE_OTG_STATE, (TEXT("USBOTG::EventNotification Current OtgState = %s, OtgInnput = 0x%x, OtgInternal = 0x%x"), m_cppOtgStateString[m_UsbOtgState], m_UsbOtgInput.ul, m_UsbOtgInternal.ul));
        }
        else
            break;
    } while (m_UsbOtgState != USBOTG_states_unknown && fContinue);

    DEBUGMSG(ZONE_OTG_STATE, (TEXT("-USBOTG::EventNotification OtgState = %s, OtgInnput = 0x%x, OtgInternal = 0x%x"), m_cppOtgStateString[m_UsbOtgState], m_UsbOtgInput.ul, m_UsbOtgInternal.ul));
    m_SyncAccess.Unlock();
    return bReturn;
}

BOOL USBOTG::NewStateAction(USBOTG_STATES usbOtgState, USBOTG_OUTPUT usbOtgOutput)
{
    switch (usbOtgState)
    {
      case USBOTG_b_idle:
      case USBOTG_a_idle:
        m_UsbOtgInternal.ul = 0;
        break;
    }
    if (usbOtgState != USBOTG_a_host && usbOtgState != USBOTG_a_suspend)
    {
        // a_suspend_req suspend request only valid in those state.
        m_UsbOtgInput.bit.a_suspend_req = 0;
    }
    return TRUE;
}

BOOL USBOTG::TimeOut(USBOTG_WATCHITEM usbOtgWatchItem)
{
    DEBUGMSG(ZONE_OTG_STATE, (TEXT("+USBOTG::TimeOut OtgState = %s usbOtgWatchItem = %d \r\n"), m_cppOtgStateString[m_UsbOtgState], usbOtgWatchItem));
    m_SyncAccess.Lock();
    UpdateInput();
    switch (usbOtgWatchItem)
    {
    case a_wait_vrise:
        if (m_UsbOtgState == USBOTG_a_wait_vrise)
        {
            m_UsbOtgInput.bit.a_wait_vrise_tmout = 1;
            EventNotification();
        }
        // Eric : Originally this break is in the if {} statement, seems wrong
        break;
    case a_wait_bcon:
        if (m_UsbOtgState == USBOTG_a_wait_bcon)
        {
            m_UsbOtgInput.bit.a_wait_bcon_tmout = 1;
            EventNotification();
        }
        break;
    case a_aidl_bdis:
        if (m_UsbOtgState == USBOTG_a_suspend)
        {
            m_UsbOtgInput.bit.a_aidl_bdis_tmout = 1;
            EventNotification();
        }
        break;
    case b_ase0_brst:
        if (m_UsbOtgState == USBOTG_b_wait_acon)
        {
            m_UsbOtgInput.bit.b_ase0_brst_tmout = 1;
            EventNotification();
        }
        break;
    case b_srp_init:
        if (m_UsbOtgState == USBOTG_b_idle)
        {
            m_UsbOtgInternal.bit.b_srp_done = 1;
        }
        break;

    case unkown_usbotg_watch_item:
    default:
        break;
    }
    m_SyncAccess.Unlock();
    return EventNotification();
}

BOOL USBOTG::IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    BOOL bReturn = FALSE;
    switch (dwCode)
    {
    case IOCTL_BUS_USBOTG_GETOTG_ENABLE_BIT :
        if (pBufOut == NULL || dwLenOut < sizeof(BYTE) || !pBufIn || dwLenIn < (_tcslen((TCHAR *)pBufIn)+1)*sizeof(TCHAR))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        else
        {
            *(PBYTE)pBufOut = GetOTGEnableBit((TCHAR *)pBufIn);
            bReturn = TRUE;
            if (pdwActualOut)
            {
                *pdwActualOut = sizeof(BYTE);
            }
        }
        break;
    case IOCTL_BUS_USBOTG_REQUEST_BUS:
        if (!pBufIn || dwLenIn < sizeof(DWORD))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        else
        {
            SetLastError(ERROR_GEN_FAILURE);
            bReturn = OtgRequestBus((*(DWORD *)pBufIn) != 0);
        }
        break;
    case IOCTL_BUS_USBOTG_DROP_BUS :
        if (!pBufIn || dwLenIn < sizeof(DWORD))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        else
        {
            SetLastError(ERROR_GEN_FAILURE);
            bReturn = OtgRequestBus((*(DWORD *)pBufIn) != 0);
        }
        break;
    case IOCTL_BUS_USBOTG_RESUME :
        if (!pBufIn || dwLenIn < (_tcslen((TCHAR *)pBufIn)+1)*sizeof(TCHAR))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        else
        {
            SetLastError(ERROR_GEN_FAILURE);
            bReturn = OtgSuspendBus((TCHAR *)pBufIn, FALSE);
        }
        break;
    case IOCTL_BUS_USBOTG_SUSPEND :
        if (!pBufIn || dwLenIn < (_tcslen((TCHAR *)pBufIn)+1)*sizeof(TCHAR))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        else
        {
            SetLastError(ERROR_GEN_FAILURE);
            bReturn = OtgSuspendBus((TCHAR *)pBufIn, TRUE);
        }
        break;
    case IOCTL_BUS_USBOTG_HNP_ENABLE:
        if (!pBufIn || dwLenIn < (_tcslen((TCHAR *)pBufIn)+1)*sizeof(TCHAR))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        else
        {
            SetLastError(ERROR_GEN_FAILURE);
            bReturn = OtgEnableHNP((TCHAR *)pBufIn, TRUE);
        }
        break;
    case IOCTL_BUS_USBOTG_HNP_DISABLE:
        if (!pBufIn || dwLenIn < (_tcslen((TCHAR *)pBufIn)+1)*sizeof(TCHAR))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        else
        {
            SetLastError(ERROR_GEN_FAILURE);
            bReturn = OtgEnableHNP((TCHAR *)pBufIn, FALSE);
        }
        break;
    case IOCTL_BUS_USBOTG_GET_HOST_PORT:
        if (pBufOut == NULL || dwLenOut < sizeof(BYTE) || !pBufIn || dwLenIn < (_tcslen((TCHAR *)pBufIn)+1)*sizeof(TCHAR))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        else
        {
            *(PBYTE)pBufOut = m_bHCDPortOnOTG;
            bReturn = TRUE;
            if (pdwActualOut)
            {
                *pdwActualOut = sizeof(BYTE);
            }
        }
        break;
    case IOCTL_BUS_USBOTG_NOTIFY_DETACH:
        if (!pBufIn || dwLenIn < (_tcslen((TCHAR *)pBufIn)+1)*sizeof(TCHAR))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        else
        {
            SetLastError(ERROR_GEN_FAILURE);
            bReturn = OTGNotifyDetach((TCHAR *)pBufIn);
        }
        break;
    case IOCTL_BUS_USBOTG_GET_A_ATTACH_EVENT:
        if (pBufOut == NULL || dwLenOut < sizeof(HANDLE) || !pBufIn || dwLenIn < (_tcslen((TCHAR *)pBufIn)+1)*sizeof(TCHAR))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        else
        {
            *(HANDLE *)pBufOut = GetOtgAConnectEvent();
            bReturn = TRUE;
            if (pdwActualOut)
            {
                *pdwActualOut = sizeof(HANDLE);
            }
        }
        break;
    case IOCTL_POWER_CAPABILITIES:
        if (!pBufOut || dwLenOut < sizeof(POWER_CAPABILITIES) || !pdwActualOut)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            bReturn = FALSE;
        }
        else
        {
            PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES)pBufOut;
            m_IsThisPowerManaged = TRUE;
            *ppc = GetPowerCapabilities();
            if (pdwActualOut)
                *pdwActualOut = sizeof(POWER_CAPABILITIES);
            bReturn = TRUE;
        }
        break;
    case IOCTL_POWER_SET:
        if (!pBufOut || dwLenOut < sizeof(CEDEVICE_POWER_STATE) || !pdwActualOut)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            bReturn = FALSE;
        }
        else
        {
            m_RequestedPowerState = *(PCEDEVICE_POWER_STATE) pBufOut;
            m_IsThisPowerManaged = TRUE;
            DEBUGMSG(ZONE_OTG_INIT, (TEXT("SBOTG::IOControl:IOCTL_POWER_SET: D%d\r\n"), m_RequestedPowerState));
            UpdatePowerState();
            // did we set the device power?
            if (pdwActualOut)
            {
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
            }
        }
        break;
    case IOCTL_BUS_USBOTG_USBFN_ACTIVE:
        if (pBufOut == NULL || dwLenOut < sizeof(DWORD) || !pBufIn || dwLenIn < (_tcslen((TCHAR *)pBufIn)+1)*sizeof(TCHAR))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            bReturn = FALSE;
        }
        else
        {
            bReturn = OTGUSBFNNotfyActive((TCHAR *)pBufIn, (*(PDWORD)pBufOut) != 0);
        }
        break;
    case IOCTL_BUS_USBOTG_HCD_ACCEPT:
        if (pBufOut == NULL || dwLenOut < sizeof(DWORD) || !pBufIn || dwLenIn < (_tcslen((TCHAR *)pBufIn)+1)*sizeof(TCHAR))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            bReturn = FALSE;
        }
        else
        {
            bReturn = OTGHCDNotfyAccept((TCHAR *)pBufIn, (*(PDWORD)pBufOut) != 0);
        }
        break;
    default:
        bReturn = DefaultBusDriver::IOControl(dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
        break;
    }
    return bReturn;
}

BYTE USBOTG::GetOTGEnableBit(LPCTSTR lpDeviceName)
{
    BYTE bReturn = 0;
    DeviceFolder* pChildFolder = GetChildByName(lpDeviceName);
    if (pChildFolder != NULL)
    {
        bReturn |= (m_HCD_Registry != NULL ? USBOTG_HCD_ENABLE : 0);
        bReturn |= (m_UsbOtgInput.bit.id == 0 ? USBOTG_CONNECT_DEFAULT_A : 0);
        if (m_UsbOtgInput.bit.id != 0 || !m_fDisalbeRoleSwitch)
        {
            bReturn |= (m_USBFN_Registry != NULL ? USBOTG_DEVICE_ENABLE : 0);
        }
        pChildFolder->DeRef();
    }
    //DEBUGMSG(ZONE_OTG_IOCTL, (TEXT("USBOTG::GetOTGEnableBit lpDeviceName = %s return Bit = %x \r\n"), lpDeviceName, bReturn));
    return bReturn;
}

BOOL USBOTG::OTGUSBFNNotfyActive(LPCTSTR lpDeviceName, BOOL fActive)
{
    BOOL bReturn = FALSE;
    DeviceFolder* pChildFolder = GetChildByName(lpDeviceName);
    if (pChildFolder != NULL && pChildFolder->GetDeviceNumber() == USBOTG_USBFN_DRIVER_DEVICE_INDEX)
    {
        m_SyncAccess.Lock();
        m_UsbOtgInput.bit.b_usbfn_active = (fActive? 1: 0);
        EventNotification();
        m_SyncAccess.Unlock();
        bReturn = TRUE;
    }
    return bReturn;
}

BOOL USBOTG::OTGHCDNotfyAccept(LPCTSTR lpDeviceName, BOOL fAccept)
{
    BOOL bReturn = FALSE;
    DeviceFolder* pChildFolder = GetChildByName(lpDeviceName);
    if (pChildFolder != NULL && pChildFolder->GetDeviceNumber() == USBOTG_HCD_DRIVER_DEVICE_INDEX)
    {
        m_SyncAccess.Lock();
        m_UsbOtgInput.bit.b_hcd_accept = (fAccept? 1: 0);
        EventNotification();
        m_SyncAccess.Unlock();
        bReturn = TRUE;
    }
    return bReturn;
}


BOOL USBOTG::OtgRequestBus(BOOL bRequest)
{
    DEBUGMSG(ZONE_OTG_IOCTL, (TEXT("USBOTG::OtgRequestBus bRequest = %x \r\n"), bRequest));
    if (!m_fDisalbeRoleSwitch)
    {
        m_SyncAccess.Lock();
        if (bRequest)
        {
            m_UsbOtgInput.bit.b_bus_req = 1;
            m_UsbOtgInput.bit.a_bus_req = 1;
            m_UsbOtgInput.bit.a_bus_drop = 0;
            m_WaitSRPCount = 0;
        }
        else
        {
            m_UsbOtgInput.bit.a_bus_req = 0;
            m_UsbOtgInput.bit.b_bus_req = 0;
        }
        EventNotification();
        m_SyncAccess.Unlock();
    }
    return TRUE;
}

BOOL USBOTG::OtgDropBus(BOOL bRequest)
{
    DEBUGMSG(ZONE_OTG_IOCTL, (TEXT("USBOTG::OtgDropBus bRequest = %x \r\n"), bRequest));
/*
    m_SyncAccess.Lock();
    if (bRequest)
    {
        m_UsbOtgInput.bit.a_bus_drop = 1;
    }
    else
    {
        m_UsbOtgInput.bit.a_bus_drop = 0;
    }
    EventNotification();
    m_SyncAccess.Unlock();
*/
    return TRUE;
}

BOOL USBOTG::OtgSuspendBus(LPCTSTR lpDeviceName, BOOL bSuspend)
{
    DEBUGMSG(ZONE_OTG_IOCTL, (TEXT("USBOTG::OtgSuspendBus lpDeviceName = %s bSuspend = %x \r\n"), lpDeviceName, bSuspend));
    DeviceFolder * pChildFolder = GetChildByName(lpDeviceName);
    BOOL bReturn = FALSE;
    if (pChildFolder)
    {
        switch (pChildFolder->GetDeviceNumber())
        {
        case USBOTG_HCD_DRIVER_DEVICE_INDEX:
            m_SyncAccess.Lock();
            if (bSuspend)
            {
                m_UsbOtgInput.bit.a_suspend_req = 1;
            }
            else
            {
                m_UsbOtgInput.bit.a_suspend_req = 0;
            }
            EventNotification();
            m_SyncAccess.Unlock();
            bReturn = TRUE;
            break;
        case USBOTG_USBFN_DRIVER_DEVICE_INDEX:
            m_SyncAccess.Lock();
            if (bSuspend)
            {
                m_UsbOtgInput.bit.b_bus_suspend = 1;
                m_UsbOtgInput.bit.b_bus_resume = 0;
                m_UsbOtgInput.bit.a_bus_suspend = 1;
                m_UsbOtgInput.bit.a_bus_resume = 0;
            }
            else
            {
                m_UsbOtgInput.bit.b_bus_suspend = 0;
                m_UsbOtgInput.bit.b_bus_resume = 1;
                m_UsbOtgInput.bit.a_bus_resume = 1;
                m_UsbOtgInput.bit.a_bus_suspend = 0;
            }
            EventNotification();
            m_SyncAccess.Unlock();
            bReturn = TRUE;
            break;
        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            bReturn = FALSE;
            break;
        }
        pChildFolder->DeRef();
    }
    return bReturn;
}

BOOL USBOTG::OtgEnableHNP(LPCTSTR lpDeviceName, BOOL fEnable)
{
    DEBUGMSG(ZONE_OTG_IOCTL, (TEXT("USBOTG::OtgEnableHNP lpDeviceName = %s fEnable = %x \r\n"), lpDeviceName, fEnable));
    DeviceFolder* pChildFolder = GetChildByName(lpDeviceName);
    BOOL bReturn = FALSE;
    m_SyncAccess.Lock();
    if (lpDeviceName && pChildFolder && !m_fDisalbeRoleSwitch)
    {
        switch (pChildFolder->GetDeviceNumber())
        {
            case USBOTG_HCD_DRIVER_DEVICE_INDEX:
                Sleep(10); // For device Ready.
                m_UsbOtgInternal.bit.a_set_b_hnp_en = (fEnable ? 1 : 0);
                bReturn = TRUE;
                break;
            case USBOTG_USBFN_DRIVER_DEVICE_INDEX:
                m_UsbOtgInternal.bit.b_hnp_enable = (fEnable ? 1 : 0);
                bReturn = TRUE;
                break;
            default:
                break;
        }
        EventNotification();
    }
    m_SyncAccess.Unlock();
    if (pChildFolder)
        pChildFolder->DeRef();
    return bReturn;
}

BOOL USBOTG::OTGNotifyDetach(LPCTSTR lpDeviceName)
{
    DEBUGMSG(ZONE_OTG_IOCTL, (TEXT("USBOTG::OTGHostNotifyDetach lpDeviceName = %s\r\n"), lpDeviceName));
    DeviceFolder * pChildFolder = GetChildByName(lpDeviceName);
    BOOL bReturn = FALSE;
    m_SyncAccess.Lock();
    if (lpDeviceName && pChildFolder)
    {
        bReturn = TRUE;
        switch (pChildFolder->GetDeviceNumber())
        {
        case USBOTG_HCD_DRIVER_DEVICE_INDEX:
            m_UsbOtgInput.bit.a_conn = m_UsbOtgInput.bit.b_conn = 0;
            m_UsbOtgInternal.bit.a_set_b_hnp_en = 0;
            break;
        case USBOTG_USBFN_DRIVER_DEVICE_INDEX:
            m_UsbOtgInternal.bit.b_hnp_enable = 0;
            break;
        }
        EventNotification();
    }
    m_SyncAccess.Unlock();
    if (pChildFolder)
        pChildFolder->DeRef();
    return bReturn;
}

extern "C" BOOL WINAPI
DllEntry(HINSTANCE DllInstance, INT Reason, LPVOID Reserved)
{
    switch(Reason)
    {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER(DllInstance);
        DEBUGMSG(ZONE_OTG_INIT, (TEXT("USBOTG.DLL DLL_PROCESS_ATTACH\r\n")));
        DisableThreadLibraryCalls((HMODULE) DllInstance);
        break;
    case DLL_PROCESS_DETACH:
        DEBUGMSG(ZONE_OTG_INIT, (TEXT("USBOTG.DLL DLL_PROCESS_DETACH\r\n")));
        break;
    };
    return TRUE;
}

//extern "C" DWORD PDCardInitServices(DWORD dwInfo);
// Init() - called by RegisterDevice when we get loaded
extern "C" DWORD
OTG_Init (DWORD dwInfo)
{
    USBOTG * pBusDriver = CreateUSBOTGObject((LPTSTR) dwInfo);;
    if (pBusDriver)
    {
        if (pBusDriver->Init())
            return (DWORD)pBusDriver;
        else
            delete pBusDriver;
    }
    return NULL;
}


extern "C" void
OTG_Deinit (DWORD dwData)
{
    USBOTG* pBusDriver = (USBOTG*) dwData;
    if (pBusDriver)
        DeleteUSBOTGObject(pBusDriver);
}

extern "C" BOOL
OTG_PowerUp(DWORD dwData)
{
    DEBUGMSG(ZONE_OTG_INIT, (TEXT("BusEnum.DLL : +PowerUp dwData = %x \r\n"), dwData));
    USBOTG * pBusDriver = (USBOTG *)dwData;
    if (pBusDriver)
        pBusDriver->PowerUp();
    DEBUGMSG(ZONE_OTG_INIT, (TEXT("BusEnum.DLL : -PowerUp dwData = %x \r\n")));
    return TRUE;
}

extern "C" BOOL
OTG_PowerDown(DWORD dwData)
{
    DEBUGMSG(ZONE_OTG_INIT, (TEXT("BusEnum.DLL : +PowerDown dwData = %x \r\n"), dwData));
    USBOTG * pBusDriver = (USBOTG *)dwData;
    if (pBusDriver)
        pBusDriver->PowerDown();
    DEBUGMSG(ZONE_OTG_INIT, (TEXT("BusEnum.DLL : -PowerDown dwData = %x \r\n")));
    return TRUE;
}

extern "C" HANDLE
OTG_Open(
        HANDLE pHead, // @parm Handle returned by COM_Init.
        DWORD AccessCode, // @parm access code.
        DWORD ShareMode // @parm share mode - Not used in this driver.
        )
{
    USBOTG * pBusDriver = (USBOTG *)pHead;
    if (pBusDriver && pBusDriver->Open(AccessCode, ShareMode))
        return (HANDLE)pBusDriver;
    return NULL;
}

extern "C" BOOL
OTG_Close(HANDLE pOpenHead)
{
    USBOTG * pBusDriver = (USBOTG *)pOpenHead;
    if (pBusDriver)
        return pBusDriver->Close();
    return FALSE;
}

extern "C" BOOL
OTG_IOControl(HANDLE pOpenHead,
              DWORD dwCode, PBYTE pBufIn,
              DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
              PDWORD pdwActualOut)
{
    USBOTG * pBusDriver = (USBOTG *)pOpenHead;
    if (pBusDriver)
        return pBusDriver->IOControl(dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
    return FALSE;
}

#pragma warning(pop)
