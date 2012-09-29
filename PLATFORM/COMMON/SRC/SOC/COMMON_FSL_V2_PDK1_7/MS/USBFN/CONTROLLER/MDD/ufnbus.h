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
#ifndef _UFNBUS_H_
#define _UFNBUS_H_

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 4512 4100 6287)
#include <windows.h>
#include <usbfnioctl.h>
#include <defbus.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4512)  // disable warning "assignment operator could not be generated"

#define UFN_BUS_PREFIX  _T("USBFN")
#define UFN_CLIENT_BUS_NAME_MAX_CHARS 30

struct UFN_MDD_CONTEXT;
struct UFN_MDD_BUS_OPEN_CONTEXT;

class CUfnBusDevice : public DeviceFolder {
public:
    CUfnBusDevice(LPCTSTR pszBusName, LPCTSTR pszRegPath,
        LPCTSTR pszChildName, LPCTSTR pszDeviceBusName, DWORD dwController, HANDLE hParent);
    virtual ~CUfnBusDevice() {}

    virtual BOOL Init() {
        return (DeviceFolder::Init() && m_fValid);
    }

    LPCTSTR GetUfnName() { return m_szUfnName; }
    BOOL GetUfnDescription(LPTSTR pszDescription, DWORD  cchDescription);

protected:
    TCHAR  m_szUfnName[UFN_CLIENT_NAME_MAX_CHARS];
    BOOL   m_fValid;
};

class CUfnBus : public DefaultBusDriver {
public:
    CUfnBus(LPCTSTR pszActivePath, UFN_MDD_CONTEXT *pContext);
    virtual BOOL Init();
    virtual ~CUfnBus();
    
    virtual BOOL PostInit();
    virtual BOOL IOControl(UFN_MDD_BUS_OPEN_CONTEXT *pBusContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
        PDWORD pdwActualOut);
    virtual DWORD GetBusNamePrefix(LPTSTR pszBusName, DWORD cchBusName);

    virtual BOOL ActivateChild(LPCTSTR lpChildBusName);
    virtual BOOL DeactivateChild(LPCTSTR lpChildBusName);

    BOOL IsClientActive() { 
        CUfnBusDevice *pDevice = (CUfnBusDevice*) GetDeviceList();
        if ( pDevice && (pDevice->GetDeviceHandle() != NULL) ) {
            return TRUE;
        }
        else {
            return FALSE;
        }
    }
    
    virtual BOOL TranslateChildBusAddr(
        PCE_BUS_TRANSLATE_BUS_ADDR pcbtba);
    virtual BOOL TranslateChildSystemAddr(
        PCE_BUS_TRANSLATE_SYSTEM_ADDR pcbtsa);
    
    virtual BOOL SetChildDevicePowerState(PCE_BUS_POWER_STATE pcbps,DeviceFolder **ppDeviceFoler = NULL );
    virtual BOOL GetChildDevicePowerState(PCE_BUS_POWER_STATE pcbps,DeviceFolder **ppDeviceFoler = NULL );
    
    virtual BOOL SetChildDeviceConfigurationData(
        PCE_BUS_DEVICE_CONFIGURATION_DATA pcbdcd,DeviceFolder **ppDeviceFoler = NULL );
    virtual BOOL GetChildDeviceConfigurationData(
        PCE_BUS_DEVICE_CONFIGURATION_DATA pcbdcd,DeviceFolder **ppDeviceFoler = NULL );

    DWORD GetDefaultClientName(LPTSTR pszClientName, DWORD cchClientName);
    VOID GetDefaultClientKey(LPTSTR pszDefaultClientKey,
        DWORD cchDefaultClientKey);

    BOOL SetChildDevicePowerState(CEDEVICE_POWER_STATE cps) {
        CE_BUS_POWER_STATE cbps = { m_szClientBusName,
            &cps, NULL };
        return SetChildDevicePowerState(&cbps);
    }

    BOOL IsChildPowerManaged() { return m_fIsChildPowerManaged; }

    static DWORD OpenFunctionKey(HKEY *phkFunctions);
    
protected:
    DWORD CreateChild(LPCTSTR pszChildName, CUfnBusDevice **ppDevice);
    
    DWORD CleanUpAfterClient();

    BOOL IsClientPresent() { return (GetDeviceList() != NULL); }

    UFN_MDD_CONTEXT *m_pContext;

    DWORD  m_dwControllerNumber;
    TCHAR  m_szClientBusName[UFN_CLIENT_BUS_NAME_MAX_CHARS];
    HANDLE m_hInitThread;
    BOOL   m_fIsChildPowerManaged;
};


#endif _UFNBUS_H_
