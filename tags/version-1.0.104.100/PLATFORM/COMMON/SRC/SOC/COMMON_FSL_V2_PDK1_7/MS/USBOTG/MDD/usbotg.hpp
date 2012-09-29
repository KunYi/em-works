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
//     USBOTG.h
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
#ifndef __USBOTG_H_
#define __USBOTG_H_
#include <ceotgbus.h>
#include <DefBus.h>
#include <CRegEdit.h>
#include <cmthread.h>

#ifdef DEBUG
#define ZONE_OTG_INIT       DEBUGZONE(0)
#define ZONE_OTG_OPEN       DEBUGZONE(1)
#define ZONE_OTG_READ       DEBUGZONE(2)
#define ZONE_OTG_WRITE      DEBUGZONE(3)
#define ZONE_OTG_CLOSE      DEBUGZONE(4)
#define ZONE_OTG_IOCTL      DEBUGZONE(5)
#define ZONE_OTG_THREAD     DEBUGZONE(6)
#define ZONE_OTG_TRANS      DEBUGZONE(7)
#define ZONE_OTG_STATE      DEBUGZONE(12)
#define ZONE_OTG_FUNCTION   DEBUGZONE(13)
#define ZONE_OTG_WARN       DEBUGZONE(14)
#define ZONE_OTG_ERROR      DEBUGZONE(15)

#endif // DEBUG


typedef union _USBOTG_INPUT {
// A Port Event. please refer OTG 6.6.1
    struct {
        DWORD a_bus_drop:1;
        DWORD a_bus_req:1;
        DWORD a_bus_resume:1;
        DWORD a_bus_suspend:1;

        DWORD a_conn:1;
        DWORD a_sess_vld:1;
        DWORD a_srp_det:1;
        DWORD a_vbus_vld:1;

        DWORD b_bus_req:1;
        DWORD b_bus_resume:1;
        DWORD b_bus_suspend:1;
        DWORD b_conn:1;

        DWORD b_se0_srp:1;
        DWORD b_sess_end:1;
        DWORD b_sess_vld:1;
        DWORD id:1;

        DWORD a_suspend_req:1;
        DWORD a_clr_err:1;
        DWORD a_wait_vrise_tmout:1;
        DWORD a_wait_bcon_tmout:1;

        DWORD a_aidl_bdis_tmout:1;
        DWORD b_ase0_brst_tmout:1;
        DWORD b_srp_fail_tmout:1;
        DWORD b_usbfn_active:1;

        DWORD b_hcd_accept:1;
    } bit;
    DWORD ul;
} USBOTG_INPUT, *PUSBOTG_INPUT;

typedef union _USBOTG_INTERNAL {
// Internal Variables OTG 6.6.2
    struct {
        DWORD a_set_b_hnp_en:1;
        DWORD b_srp_done:1;
        DWORD b_hnp_enable:1;
    } bit;
    DWORD ul;
} USBOTG_INTERNAL, *PUSBOTG_INTERNAL;

typedef union _USBOTG_OUTPUT {
    // OTG 6.6.3
    struct {
        DWORD chrg_vbus:1;
        DWORD drv_vbus:1;
        DWORD loc_con:1;
        DWORD loc_sof:1;
    }bit;
    DWORD ul;
} USBOTG_OUTPUT, *PUSBOTG_OUTPUT;

typedef struct _USBOTG_TIMERS {
    // OTG 6.6.5
    DWORD a_wait_vrise_tmr;
    DWORD a_wait_bcon_tmr;
    DWORD a_aidl_bdis_tmr;
    DWORD b_ase0_brst_tmr;
    DWORD b_srp_fail_tmr;
    DWORD b_srp_fail_count;
} USBOTG_TIMERS, *PUSBOTG_TIMERS;
typedef enum {
    unkown_usbotg_watch_item,
    a_wait_vrise,
    a_wait_bcon,
    a_aidl_bdis,
    b_ase0_brst,
    b_srp_fail,
    b_srp_init
} USBOTG_WATCHITEM;


typedef enum {
    // OTG 6.8
    USBOTG_states_unknown,
// A Port States
    USBOTG_a_idle,
    USBOTG_a_wait_vrise,
    USBOTG_a_wait_bcon,
    USBOTG_a_host,
    USBOTG_a_suspend,
    USBOTG_a_peripheral,
    USBOTG_a_wait_vfall,
    USBOTG_a_vbus_err,
// B Port States
    USBOTG_b_idle,
    USBOTG_b_srp_init,
    USBOTG_b_peripheral,
    USBOTG_b_wait_acon,
    USBOTG_b_host
} USBOTG_STATES, *PUSBOTG_STATES;

typedef enum {
    USBOTG_mode_unknow,
    USBOTG_Bidirection,
    USBOTG_Host,
    USBOTG_Function
} USBOTG_MODE, *PUSBOTG_MODE;

#define USBOTG_HOST_DRIVER TEXT("Hcd")
#define USBOTG_FUNC_DRIVER TEXT("UsbFn")
#define USBOTG_DEFAULTOTG_BUSNAME TEXT("OTG")

#define USBOTG_HCD_PORT_NUMBER_ON_OTG TEXT("HCDPortNumberOnOTG")
#define USBOTG_DYNAMIC_CLIENT_LOAD TEXT("DynamicClientLoad")
#define USBOTG_DISABLE_ROLE_SWITCH TEXT("DisableRoleSwitch")
#define USBOTG_INITIAL_ROLE_MASTER TEXT("InitialRoleMaster")
#define USBOTG_THREAD_PRIORITY TEXT("Priority256")

#define USBOTG_HCD_DRIVER_DEVICE_INDEX 1
#define USBOTG_USBFN_DRIVER_DEVICE_INDEX 2
// For m_dwDynamicClietLoad
#define USBOTG_HCD_DYNAMIC_LOAD 1
#define USBOTG_USBFN_DYNAMIC_LOAD 2

#define USBOTG_DEFAULT_THREAD_PRIORITY 90

class USBOTGWatchDog;

class USBOTG: public DefaultBusDriver, public CRegistryEdit 
{
public:
    USBOTG(LPCTSTR lpActivePath);
    virtual ~USBOTG();
    virtual BOOL Init();
    virtual BOOL PostInit();
    virtual DeviceFolder * CreateFunctionDeviceFolder(LPCTSTR lpBusName, LPCTSTR lpTemplateRegPath, DWORD dwBusType, DWORD BusNumber, DWORD DeviceNumber, DWORD FunctionNumber, HANDLE hParent, DWORD dwMaxInitReg = MAX_INIT_REG, LPCTSTR lpDeviceBusName = NULL) {
        return new DeviceFolder(lpBusName, lpTemplateRegPath, dwBusType, BusNumber, DeviceNumber, FunctionNumber, hParent, dwMaxInitReg, lpDeviceBusName);
    }
    virtual DeviceFolder * CreateHostDeviceFolder(LPCTSTR lpBusName, LPCTSTR lpTemplateRegPath, DWORD dwBusType, DWORD BusNumber, DWORD DeviceNumber, DWORD FunctionNumber, HANDLE hParent, DWORD dwMaxInitReg = MAX_INIT_REG, LPCTSTR lpDeviceBusName = NULL) {
        return new DeviceFolder(lpBusName, lpTemplateRegPath, dwBusType, BusNumber, DeviceNumber, FunctionNumber, hParent, dwMaxInitReg, lpDeviceBusName);
    }

    virtual BOOL ActivateAllChildDrivers();
    virtual BOOL LoadUnloadHCD(BOOL fLoad);
    virtual BOOL LoadUnloadUSBFN(BOOL fLoad);
    DeviceFolder * GetChildByDeviceNumber(DWORD dwIndex);
    virtual DWORD GetBusNamePrefix(LPTSTR lpReturnBusName, DWORD dwSizeInUnit);
    virtual BOOL Open(DWORD AccessCode, DWORD Share) {return TRUE;};
    virtual BOOL IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);

    virtual USBOTG_MODE UsbOtgConfigure(USBOTG_MODE usbOtgMode) = 0;
    virtual BOOL EventNotification();
    virtual BOOL TimeOut(USBOTG_WATCHITEM usbOtgWatchItem);
    // Action.
    virtual BOOL NewStateAction(USBOTG_STATES usbOtgState, USBOTG_OUTPUT usbOtgOutput);
    virtual BOOL UpdateInput() = 0;
    virtual BOOL SessionRequest(BOOL fPulseLocConn, BOOL fPulseChrgVBus) = 0;
    virtual BOOL DischargVBus() = 0;
    virtual BOOL IsSE0() = 0;

    // Bus Driver Interface.
    virtual BYTE GetOTGEnableBit(LPCTSTR lpDeviceName);
    virtual BOOL OtgRequestBus(BOOL bRequest);
    virtual BOOL OtgDropBus(BOOL bRequest);
    virtual BOOL OtgSuspendBus(LPCTSTR lpDeviceName, BOOL bSuspend);
    virtual BOOL OtgEnableHNP(LPCTSTR lpDeviceName, BOOL fEnable);
    virtual BOOL OTGNotifyDetach(LPCTSTR lpDeviceName);
    virtual BOOL OTGUSBFNNotfyActive(LPCTSTR lpDeviceName, BOOL fActive);
    virtual BOOL OTGHCDNotfyAccept(LPCTSTR lpDeviceName, BOOL fActive);
    virtual HANDLE GetOtgAConnectEvent() {return m_hAConnectEvent;};
    // Power Manager.
    virtual BOOL SetChildDevicePowerState(PCE_BUS_POWER_STATE) {return FALSE;};
    virtual BOOL GetChildDevicePowerState(PCE_BUS_POWER_STATE) {return FALSE;};
    virtual BOOL PowerUp() {return TRUE;};
    virtual BOOL PowerDown(){return TRUE;};
    virtual BOOL UpdatePowerState() {return TRUE;};
    virtual POWER_CAPABILITIES GetPowerCapabilities() {return m_PowerCapabilities;};

protected:
    USBOTGWatchDog * m_pUSBOTGWatchDog;
    INT m_iThreadPriority;
    CLockObject m_SyncAccess;
#ifdef DEBUG
    static LPCTSTR m_cppOtgStateString[];
#endif
    USBOTG_INPUT m_UsbOtgInput;
    USBOTG_INTERNAL m_UsbOtgInternal;
    USBOTG_OUTPUT m_UsbOtgOutputValues;
    USBOTG_TIMERS m_UsbOtgTimers;
    USBOTG_STATES m_UsbOtgState;
    USBOTG_MODE m_UsbOtgMode;
// For Bus implementation.
    DWORD m_dwDynamicClietLoad;
    BOOL m_fDisalbeRoleSwitch;
    CRegistryEdit* m_HCD_Registry;
    CRegistryEdit* m_USBFN_Registry;
    LPTSTR m_lpActiveRegPath;
    DWORD m_dwBusType;
    DWORD m_dwBusNumber;
    LPTSTR m_lpBusName;
    UCHAR m_bHCDPortOnOTG;
    HANDLE m_hAConnectEvent;
// PM
    POWER_CAPABILITIES m_PowerCapabilities;
    CEDEVICE_POWER_STATE m_RequestedPowerState;
    BOOL m_IsThisPowerManaged;

protected:
    virtual BOOL EnterState(USBOTG_STATES usbOtgState);
    virtual USBOTG_STATES StateChange(USBOTG_STATES usbOtgState);
private:
    BOOL LoadUnloadClientDriver(BOOL fLoad, DeviceFolder * pClientDriver);
    DWORD m_WaitSRPCount;
};


class USBOTGWatchDog : public CLockObject, public CMiniThread 
{
public:
    USBOTGWatchDog(USBOTG * pUsbOtg);
    virtual ~USBOTGWatchDog();
    virtual BOOL Init();
    virtual BOOL WatchForItem(USBOTG_WATCHITEM watchItem, DWORD dwTicks);
    USBOTG_WATCHITEM GetCurrentWatchItem(){return m_WatchingItem;};
private:
    virtual DWORD ThreadRun(); // User have to implement this function.
    HANDLE m_hEvent;
    USBOTG_WATCHITEM m_WatchItem, m_WatchingItem;
    DWORD m_TimeoutTicks;
    USBOTG * const m_pUsbOtg;
};

// OTG Timing
#define USBOTG_TA_SPR_RSPNS_DEFAULT (30*1000)
#define USBOTG_TA_WAIT_VRISE_DEFAULT 100
#define USBOTG_TA_BCON_LDB_DEFAULT 100
#define USBOTG_TA_BCON_ARST_DEFAULT (30*1000)
#define USBOTG_TA_WAIT_BCON_DEFAULT (1*1000)
#define USBOTG_TA_AIDL_BDIS_DEFAULT 2000 //200 is not enough for testing we set 2000
#define USBOTG_TA_BDIS_ACON_DEFAULT 3
#define USBOTG_TA_BIDL_ADIS_DEFAULT 200
#define USBOTG_TA_BCON_SDB_DEFAULT 1
#define USBOTG_TA_BCON_SDB_WIN_DEFAULT 100

#define USBOTG_TB_SE0_SRP_DEFAULT 2
#define USBOTG_TB_DATA_PLS_DEFAULT 10
#define USBOTG_TB_SRP_INIT_DEFAULT 100
#define USBOTG_TB_SRP_FAIL_DEFAULT (30*1000)
#define USBOTG_TB_SVLD_BCON_DEFAULT (1*1000)
#define USBOTG_TB_AIDL_BDIS_DEFAULT 150
#define USBOTG_TB_FS_BDIS_DEFAULT 147
#define USBOTG_TB_ASE0_BRST_DEFAULT 4
#define USBOTB_TB_ACON_DBNC_DEFAULT 1
#define USBOTG_TB_ACON_BSE0_DEFAULT 1

#define USBOTG_TLDIS_DSCHG_DEFAULT 1

// OTG Timing Registry Setting.
#define REG_TA_WAIT_VRISE_VAL TEXT("TA_WAIT_VRISE")
#define REG_TA_WAIT_BCON_VAL TEXT("TA_WAIT_BCON")
#define REG_TA_AIDL_BDIS_VAL TEXT("TA_AIDL_BDIS")
#define REG_TB_ASE0_BRST_VAL TEXT("TB_ASE0_BRST")
#define REG_TB_SRP_FAIL_VAL TEXT("TB_SRP_FAIL")

USBOTG * CreateUSBOTGObject(LPTSTR lpActivePath);
void DeleteUSBOTGObject(USBOTG * pUsbOtg);

#endif

