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
#pragma warning(push)
#pragma warning(disable: 6262)
#include "ufnbus.h"
#pragma warning(pop)
#include "ufnmdd.h"
#include "ufnclient.h"
#include <devload.h>


CUfnBusDevice::CUfnBusDevice(
    LPCTSTR pszBusName,
    LPCTSTR pszRegPath,
    LPCTSTR pszDeviceBusName,
    LPCTSTR pszChildName,
    DWORD dwController,
    HANDLE hParent
    ) : 
        DeviceFolder(pszBusName, pszRegPath, Internal, 0, dwController, 
            0, hParent, 0, pszDeviceBusName)
{
    m_fValid = TRUE;
    
    HRESULT hr = StringCchCopy(m_szUfnName, dim(m_szUfnName), pszChildName);
    if (FAILED(hr)) {
        m_fValid = FALSE;
    }

    DEBUGCHK(m_fValid);
}

static
BOOL
GetUfnDescription(
    HKEY hkClient,
    LPTSTR pszDescription,
    DWORD  cchDescription
    )
{
    SETFNAME();

    DEBUGCHK(hkClient);
    PREFAST_DEBUGCHK(pszDescription);

    DWORD dwType;
    DWORD cbValue = sizeof(TCHAR) * cchDescription;
    DWORD dwError = RegQueryValueEx(hkClient, PSZ_REG_FRIENDLY_NAME, 
        NULL, &dwType, (PBYTE) pszDescription, &cbValue);
    pszDescription[cchDescription - 1] = 0; // Null-terminate

    if ( (dwError != ERROR_SUCCESS) || (dwType != REG_SZ) ) {
        // No description. Still return success, though.
        pszDescription[0] = 0;
        DEBUGMSG(ZONE_WARNING, (_T("%s No friendly name given for client\r\n"),
            pszFname));
    }

    return TRUE;
}
    
#define MAX_ENDPOINTS 0x1000 // arbitrary max endpoint count

static
DWORD
AllocEndpoints(
    PUFN_MDD_CONTEXT pContext
    )
{
    SETFNAME();
    DWORD dwRet = ERROR_SUCCESS;
    
    // Set up our pipe table
    DEBUGCHK(pContext->rgpPipes == NULL);
    DEBUGCHK(pContext->pFreeTransferList);

    // First guard against integer overflow
    if (pContext->PddInfo.dwEndpointCount > MAX_ENDPOINTS) {
        dwRet = ERROR_INVALID_PARAMETER;
        DEBUGMSG(ZONE_ERROR, (_T("%s Too many endpoints: %u\r\n"), pszFname, 
            pContext->PddInfo.dwEndpointCount));
    }
    else {
        pContext->rgpPipes = new PCPipeBase[pContext->PddInfo.dwEndpointCount];
        if (pContext->rgpPipes == NULL) {
            dwRet = GetLastError();
            DEBUGMSG(ZONE_ERROR, (_T("%s LocalAlloc failed!\r\n"), pszFname));
        }
        else {
            BOOL fUseDynamicPipes = PddSupportsReusableEndpoints(pContext);
            for (DWORD dwPipe = 0; dwPipe < pContext->PddInfo.dwEndpointCount; dwPipe++) {
                CPipeBase* pPipe;
                if( fUseDynamicPipes ) {
                    CDynamicPipe *pDynamicPipe = new CDynamicPipe(dwPipe, &pContext->PddInfo, pContext->pFreeTransferList, pContext);
                    pPipe = pDynamicPipe;
                } else {
                    CStaticPipe *pStaticPipe  = new CStaticPipe(dwPipe, &pContext->PddInfo, pContext->pFreeTransferList, pContext);
                    pPipe = pStaticPipe;
                }

                if( pPipe == NULL )
                {
                    for( DWORD dwIndex = 0; dwIndex < dwPipe; dwIndex++ )
                    {
                        delete pContext->rgpPipes[dwPipe];
                    }
                    delete [] pContext->rgpPipes;
                    pContext->rgpPipes = NULL;
                    return ERROR_OUTOFMEMORY;
                }
                pContext->rgpPipes[dwPipe] = pPipe;
            }
        }
    }

    return dwRet;
}


static
VOID
FreePipes(
    PUFN_MDD_CONTEXT pContext
    )
{
    PREFAST_DEBUGCHK(pContext->rgpPipes);
    
    for (DWORD dwPipe = 0; dwPipe < pContext->PddInfo.dwEndpointCount; ++dwPipe) {
        PCPipeBase pPipe = pContext->rgpPipes[dwPipe];
        delete pPipe;
    }

    delete [] pContext->rgpPipes;
    pContext->rgpPipes = NULL;
}


CUfnBus::CUfnBus(
    LPCTSTR pszActivePath,
    UFN_MDD_CONTEXT *pContext
    ) :
        DefaultBusDriver(pszActivePath) 
{
    DEBUGCHK(pszActivePath);
    DEBUGCHK(pContext);
    
    m_pContext = pContext;
    m_dwControllerNumber = 0xFFFFFFFF;
    m_fIsChildPowerManaged = FALSE;
}


BOOL 
CUfnBus::Init(
    )
{
    SETFNAME();
    
    BOOL fRet = DefaultBusDriver::Init();
    
    if (fRet) {
        DEVMGR_DEVICE_INFORMATION ddi;
        ddi.dwSize = sizeof(ddi);
        if (!GetDeviceInformationByDeviceHandle(GetDeviceHandle(), &ddi)) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Could not get device information. Error = %d\r\n"),
                pszFname, GetLastError()));
            fRet = FALSE;        
        }
        else {
            m_dwControllerNumber = (ddi.szLegacyName[3] - L'0');    
            HRESULT hr = StringCchPrintf(m_szClientBusName, dim(m_szClientBusName),
                TEXT("%s_%d_0"), UFN_BUS_PREFIX, m_dwControllerNumber);
            DEBUGCHK(SUCCEEDED(hr));
            fRet = TRUE;
        }
    }

    return fRet;
}


CUfnBus::~CUfnBus(
    )
{
    if (IsClientActive()) {
        DeactivateChild(m_szClientBusName);
    }
}
    

DWORD
CUfnBus::CreateChild(
    LPCTSTR pszChildName,
    CUfnBusDevice **ppDevice
    )
{
    SETFNAME();

    TCHAR szFullRegPath[MAX_PATH];
    DWORD dwRet;

    HRESULT hr = StringCchPrintf(szFullRegPath, dim(szFullRegPath), _T("%s\\%s"),
        PSZ_REG_CLIENT_DRIVER_PATH, pszChildName);
    if (FAILED(hr)) {
        dwRet = HRESULT_CODE(hr);
        RETAILMSG(1, (_T("%s Reg path too long for buffer. \"%s\\%s\"\r\n"),
            pszFname, PSZ_REG_CLIENT_DRIVER_PATH, pszChildName));
        goto EXIT;
    }
        
    DEBUGMSG(ZONE_INIT, (_T("%s Using client driver key \"%s\"\r\n"), pszFname,
        szFullRegPath));

    CUfnBusDevice *pDevice = new CUfnBusDevice(UFN_BUS_PREFIX, szFullRegPath, 
        m_szClientBusName, pszChildName, m_dwControllerNumber, GetDeviceHandle());
    if (!pDevice || !pDevice->Init()) {
        if (pDevice) {
            delete pDevice;
        }
        dwRet = ERROR_GEN_FAILURE;
        goto EXIT;
    }

    dwRet = ERROR_SUCCESS;
    *ppDevice = pDevice;

EXIT:
    return dwRet;
}


DWORD
CUfnBus::OpenFunctionKey(HKEY *phkFunctions)
{
    SETFNAME();
    
    DEBUGCHK(phkFunctions);
    
    // Determine which client driver to load
    DWORD dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, PSZ_REG_CLIENT_DRIVER_PATH, phkFunctions);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Failed to open function client key \"%s\". Error = %d\r\n"),
            pszFname, PSZ_REG_CLIENT_DRIVER_PATH, dwRet));
    }

    return dwRet;
}


BOOL
CUfnBus::PostInit(
    )
{
    SETFNAME();
    
    BOOL fErr;
    CUfnBusDevice *pDevice = NULL;

    TCHAR szClientName[UFN_CLIENT_NAME_MAX_CHARS];
    DWORD dwRet;
    BOOL fInserted = FALSE;

    DEBUGCHK(GetDeviceList() == NULL);

    dwRet = GetDefaultClientName(szClientName, dim(szClientName));
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }

    dwRet = CreateChild(szClientName, &pDevice);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s CreateChild failed.\r\n"), pszFname));
        dwRet = ERROR_GEN_FAILURE;
        goto EXIT;
    }

    fErr = InsertChild(pDevice);
    if (!fErr) {
        DEBUGMSG(ZONE_ERROR, (_T("%s InsertChild failed.\r\n"), pszFname));
        dwRet = ERROR_GEN_FAILURE;
        goto EXIT;
    }

    fInserted = TRUE;

    fErr = ActivateChild(m_szClientBusName);
    if (!fErr) {
        DEBUGMSG(ZONE_ERROR, (_T("%s ActivateChild failed.\r\n"), pszFname));
        dwRet = ERROR_GEN_FAILURE;
        goto EXIT;
    }

EXIT:
    BOOL fRet = TRUE;
    
    if (dwRet != ERROR_SUCCESS) {
        if (pDevice) {
            if (fInserted) {
                RemoveChildByFolder(pDevice);
                // This will take reference count to 0 and delete it.
            }
            else {
                delete pDevice;
            }
        }
        
        SetLastError(dwRet);
        fRet = FALSE;
    }

    return fRet;
}

BOOL CUfnBus::ActivateChild(LPCTSTR pszChildBusName) 
{
    SETFNAME();
    DWORD dwRet;

    DEBUGCHK(m_fIsChildPowerManaged == FALSE);
    
    // Allocate our free transfer list
    DEBUGCHK(m_pContext->pFreeTransferList == NULL);
    m_pContext->pFreeTransferList = new CFreeTransferList();
    if (m_pContext->pFreeTransferList == NULL) {
        dwRet = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s Failed to create free transfer list. Error %u\r\n"),
            pszFname, dwRet));
        goto EXIT;
    }

#ifdef DEBUG
    // Verify that a compact can occur on a free list where
    // nothing has been allocated yet.
    //
    // Perform an initial compact to simulate the compact that will
    // occur after a quick attach and detach when no transfers have
    // been allocated yet.
    m_pContext->pFreeTransferList->Compact();
#endif

    dwRet = AllocEndpoints(m_pContext);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Failed to create pipes. Error %u\r\n"),
            pszFname, dwRet));
        goto EXIT;
    }

    // Load the client driver
    DEBUGCHK(m_pContext->fClientIsBeingAddedOrRemoved == FALSE);
    m_pContext->fClientIsBeingAddedOrRemoved = TRUE;
    m_hInitThread = GetCurrentThread();

    // ActivateChild will eventually call the PDD's Start routine. The Start
    // routine will create the IST which could immediately send notifications
    // to the MDD. Reset the event so that notifications will be stalled.
    ResetEvent(m_pContext->hevReadyForNotifications);
    BOOL fErr = DefaultBusDriver::ActivateChild(pszChildBusName);

    m_hInitThread = NULL;
    m_pContext->fClientIsBeingAddedOrRemoved = FALSE;

    // Allow the stalled notification to continue now that the client is 
    // fully loaded.
    SetEvent(m_pContext->hevReadyForNotifications);

    if (fErr == FALSE) {
        DEBUGMSG(ZONE_ERROR, (_T("%s ActivateChild failed.\r\n"), pszFname));
        dwRet = ERROR_GEN_FAILURE;
        goto EXIT;
    }

    DEBUGMSG(ZONE_INIT, (_T("%s Activated client driver \"%s\"\r\n"), 
        pszFname, ((CUfnBusDevice*) GetDeviceList())->GetUfnName()));    

EXIT:
    if (dwRet != ERROR_SUCCESS) {
        CleanUpAfterClient();
    }
    
    return (dwRet == ERROR_SUCCESS);
}


BOOL 
CUfnBus::DeactivateChild(LPCTSTR pszChildBusName)
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    PREFAST_DEBUGCHK(m_pContext);
    DEBUGCHK(IsClientActive());

    DWORD dwRet = CleanUpAfterClient();

    m_fIsChildPowerManaged = FALSE;

    FUNCTION_LEAVE_MSG();

    return (dwRet == ERROR_SUCCESS);
}


BOOL 
CUfnBus::TranslateChildBusAddr(
    PCE_BUS_TRANSLATE_BUS_ADDR pcbtba
    )
{
    PREFAST_DEBUGCHK(pcbtba);
    PREFAST_DEBUGCHK(pcbtba->lpDeviceBusName);
    PREFAST_DEBUGCHK(pcbtba->AddressSpace);
    PREFAST_DEBUGCHK(pcbtba->TranslatedAddress);

    BOOL fRet = FALSE;

    CUfnBusDevice *pDevice = (CUfnBusDevice *) GetChildByName(pcbtba->lpDeviceBusName);
    if (pDevice) {
        DWORD dwRet = m_pContext->PddInfo.pfnIOControl(
            m_pContext->PddInfo.pvPddContext, MDD_IOCTL, IOCTL_BUS_TRANSLATE_BUS_ADDRESS,
            (PBYTE) pcbtba, sizeof(*pcbtba), NULL, 0, NULL);

        if (dwRet == ERROR_SUCCESS) {
            fRet = TRUE;
        }
        else {
            SetLastError(dwRet);
        }

        pDevice->DeRef();
    }
    else {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return fRet;
}


BOOL 
CUfnBus::TranslateChildSystemAddr(
    PCE_BUS_TRANSLATE_SYSTEM_ADDR pcbtsa
    )
{
    PREFAST_DEBUGCHK(pcbtsa);
    PREFAST_DEBUGCHK(pcbtsa->lpDeviceBusName);
    PREFAST_DEBUGCHK(pcbtsa->TranslatedAddress);

    BOOL fRet = FALSE;

    CUfnBusDevice *pDevice = (CUfnBusDevice *) GetChildByName(pcbtsa->lpDeviceBusName);
    if (pDevice) {
        DWORD dwRet = m_pContext->PddInfo.pfnIOControl(
            m_pContext->PddInfo.pvPddContext, MDD_IOCTL, IOCTL_BUS_TRANSLATE_SYSTEM_ADDRESS,
            (PBYTE) pcbtsa, sizeof(*pcbtsa), NULL, 0, NULL);
    
        if (dwRet == ERROR_SUCCESS) {
            fRet = TRUE;
        }
        else {
            SetLastError(dwRet);
        }

        pDevice->DeRef();
    }
    else {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return fRet;
}


BOOL 
CUfnBus::SetChildDevicePowerState(
    PCE_BUS_POWER_STATE pcbps,
    DeviceFolder **ppDeviceFoler 
    )
{
    PREFAST_DEBUGCHK(pcbps);
    PREFAST_DEBUGCHK(pcbps->lpDeviceBusName);
    PREFAST_DEBUGCHK(pcbps->lpceDevicePowerState);

    BOOL fRet = FALSE;

    if (!VALID_DX(*pcbps->lpceDevicePowerState)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto EXIT;
    }

    CUfnBusDevice *pDevice = 
        (CUfnBusDevice *) GetChildByName(pcbps->lpDeviceBusName,ppDeviceFoler);
    if (!pDevice) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto EXIT;
    }
    
    DWORD dwRet = m_pContext->PddInfo.pfnIOControl(
        m_pContext->PddInfo.pvPddContext, MDD_IOCTL, IOCTL_BUS_SET_POWER_STATE,
        (PBYTE) pcbps, sizeof(*pcbps), NULL, 0, NULL);
    if (dwRet == ERROR_SUCCESS) {
        fRet = TRUE;
    }
    else {
        SetLastError(dwRet);
    }

    pDevice->DeRef();

EXIT:
    return fRet;
}


BOOL 
CUfnBus::GetChildDevicePowerState(
    PCE_BUS_POWER_STATE pcbps ,
    DeviceFolder **ppDeviceFoler 
    )
{
    PREFAST_DEBUGCHK(pcbps);
    PREFAST_DEBUGCHK(pcbps->lpDeviceBusName);
    PREFAST_DEBUGCHK(pcbps->lpceDevicePowerState);

    BOOL fRet = FALSE;

    CUfnBusDevice *pDevice = (CUfnBusDevice *) GetChildByName(pcbps->lpDeviceBusName,ppDeviceFoler);
    if (pDevice) {
        DWORD dwRet = m_pContext->PddInfo.pfnIOControl(
            m_pContext->PddInfo.pvPddContext, MDD_IOCTL, IOCTL_BUS_GET_POWER_STATE,
            (PBYTE) pcbps, sizeof(*pcbps), NULL, 0, NULL);

        if (dwRet == ERROR_SUCCESS) {
            fRet = TRUE;
        }
        else {
            SetLastError(dwRet);
        }

        pDevice->DeRef();
    }
    else {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return fRet;
}


BOOL 
CUfnBus::SetChildDeviceConfigurationData(
    PCE_BUS_DEVICE_CONFIGURATION_DATA pcbdcd,
    DeviceFolder **ppDeviceFoler
    )
{
    PREFAST_DEBUGCHK(pcbdcd);
    PREFAST_DEBUGCHK(pcbdcd->lpDeviceBusName);
    PREFAST_DEBUGCHK(pcbdcd->pBuffer);

    BOOL fRet = FALSE;

    CUfnBusDevice *pDevice = (CUfnBusDevice *) GetChildByName(pcbdcd->lpDeviceBusName,ppDeviceFoler);
    if (pDevice) {
        DWORD dwRet = m_pContext->PddInfo.pfnIOControl(
            m_pContext->PddInfo.pvPddContext, MDD_IOCTL, IOCTL_BUS_SET_CONFIGURE_DATA,
            (PBYTE) pcbdcd, sizeof(*pcbdcd), NULL, 0, NULL);
        
        if (dwRet == ERROR_SUCCESS) {
            fRet = TRUE;
        }
        else {
            SetLastError(dwRet);
        }

        pDevice->DeRef();
    }
    else {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return fRet;
}


BOOL 
CUfnBus::GetChildDeviceConfigurationData(
    PCE_BUS_DEVICE_CONFIGURATION_DATA pcbdcd,
    DeviceFolder **ppDeviceFoler
    )
{
    PREFAST_DEBUGCHK(pcbdcd);
    PREFAST_DEBUGCHK(pcbdcd->lpDeviceBusName);
    PREFAST_DEBUGCHK(pcbdcd->pBuffer);

    BOOL fRet = FALSE;

    CUfnBusDevice *pDevice = (CUfnBusDevice *) GetChildByName(pcbdcd->lpDeviceBusName,ppDeviceFoler);
    if (pDevice) {
        DWORD dwRet = m_pContext->PddInfo.pfnIOControl(
            m_pContext->PddInfo.pvPddContext, MDD_IOCTL, IOCTL_BUS_GET_CONFIGURE_DATA,
            (PBYTE) pcbdcd, sizeof(*pcbdcd), NULL, 0, NULL);
    
        if (dwRet == ERROR_SUCCESS) {
            fRet = TRUE;
        }
        else {
            SetLastError(dwRet);
        }

        pDevice->DeRef();
    }
    else {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return fRet;
}


BOOL 
CUfnBus::IOControl(
    PUFN_MDD_BUS_OPEN_CONTEXT pBusContext,
    DWORD  dwCode,
    PBYTE  pbInBuf,
    DWORD  cbInBuf,
    PBYTE  pbOutBuf,
    DWORD  cbOutBuf,
    PDWORD pcbActualOutBuf
    )
{
    SETFNAME();
    
    DWORD dwRet = ERROR_SUCCESS;
    BOOL fRet;
    BOOL fCallDefaultBusDriver = FALSE;
    
    switch (dwCode) {
        case IOCTL_UFN_ENUMERATE_AVAILABLE_CLIENTS_SETUP: {
            pBusContext->dwIndex = 0;
            pBusContext->fSetupCalled = TRUE;
            break;
        }
            
        case IOCTL_UFN_ENUMERATE_AVAILABLE_CLIENTS: {
            if ( !pbOutBuf || (cbOutBuf != sizeof(UFN_CLIENT_INFO)) ) {
                dwRet = ERROR_INVALID_PARAMETER;
                break;
            }
            else if (pBusContext->fSetupCalled == FALSE) {
                dwRet = ERROR_INVALID_STATE;
                break;
            }

            PUFN_CLIENT_INFO pClientInfo = (PUFN_CLIENT_INFO) pbOutBuf; 
            pClientInfo->szDescription[0] = 0;
            pClientInfo->szName[0] = 0;
            
            DWORD cchSubKey = dim(pClientInfo->szName);

            dwRet = RegEnumKeyEx(pBusContext->hkClients, pBusContext->dwIndex++, 
                pClientInfo->szName, &cchSubKey, NULL, NULL, NULL, NULL);
            NULL_TERMINATE(pClientInfo->szName);

            if (dwRet == ERROR_SUCCESS) {
                HKEY hkCurrentEnumClient;
                dwRet = RegOpenKeyEx(pBusContext->hkClients, 
                    pClientInfo->szName, 0, 0, &hkCurrentEnumClient);
                if (dwRet != ERROR_SUCCESS) {
                    break;
                }

                CUfnBusDevice *pDevice = (CUfnBusDevice *) GetDeviceList();
                GetUfnDescription(hkCurrentEnumClient, pClientInfo->szDescription, 
                    dim(pClientInfo->szDescription));
                if (pcbActualOutBuf) {
                    *pcbActualOutBuf = sizeof(UFN_CLIENT_INFO);
                }

                RegCloseKey(hkCurrentEnumClient);
            }
            
            break;
        }

        case IOCTL_UFN_GET_CURRENT_CLIENT: {
            if ( !pbOutBuf || (cbOutBuf != sizeof(UFN_CLIENT_INFO)) ) {
                dwRet = ERROR_INVALID_PARAMETER;
                break;
            }

            PUFN_CLIENT_INFO pClientInfo = (PUFN_CLIENT_INFO) pbOutBuf;
            pClientInfo->szDescription[0] = 0;
            pClientInfo->szName[0] = 0;

            if (IsClientPresent()) {
                CUfnBusDevice *pDevice = (CUfnBusDevice *) GetDeviceList();
                HRESULT hr = StringCchCopy(pClientInfo->szName, dim(pClientInfo->szName),
                    pDevice->GetUfnName());
                GetUfnDescription(pDevice->GetHKey(), pClientInfo->szDescription, 
                    dim(pClientInfo->szDescription));
            }
            else {
                // No current client. Return success but no data.
                dwRet = ERROR_SUCCESS;
            }
            
            if (dwRet == ERROR_SUCCESS) {
                if (pcbActualOutBuf) {
                    *pcbActualOutBuf = sizeof(UFN_CLIENT_INFO);
                }
            }
            
            break;
        }

        case IOCTL_UFN_CHANGE_CURRENT_CLIENT: {
            if ( !pbInBuf || (cbInBuf != sizeof(UFN_CLIENT_NAME)) ) {
                dwRet = ERROR_INVALID_PARAMETER;
                break;
            }

            PUFN_CLIENT_NAME pClientName = (PUFN_CLIENT_NAME) pbInBuf;
            UFN_CLIENT_NAME ClientName;
            HRESULT hr = StringCchCopy(ClientName.szName, dim(ClientName.szName),
                pClientName->szName);

            if (ClientName.szName[0] == 0) {
                // Just remove the current client if there is one
                if (IsClientPresent()) {
                    if (IsClientActive()) {
                        DeactivateChild(m_szClientBusName);
                    }

                    RemoveChildByFolder(GetDeviceList());
                }
            }
            else {
                // Load new child
                CUfnBusDevice *pDevice;
                dwRet = CreateChild(ClientName.szName, &pDevice);
                if (dwRet != ERROR_SUCCESS) {
                    DEBUGMSG(ZONE_ERROR, (_T("%s CreateChild failed.\r\n"), 
                        pszFname));
                    break;
                }

                if (pDevice && pDevice->Init()) {
                    // Remove the current client if there is one
                    if (IsClientPresent()) {
                        if (IsClientActive()) {
                            DeactivateChild(m_szClientBusName);
                        }

                        RemoveChildByFolder(GetDeviceList());
                    }

                    // Activate new client
                    fRet = InsertChild(pDevice);
                    if (fRet == FALSE) {
                        dwRet = ERROR_GEN_FAILURE;
                        DEBUGMSG(ZONE_ERROR, (_T("%s InsertChild failed.\r\n"), 
                            pszFname));
                        delete pDevice;
                        break;
                    }
                    
                    fRet = ActivateChild(m_szClientBusName);
                    if (fRet == FALSE) {
                        dwRet = ERROR_GEN_FAILURE;
                        DEBUGMSG(ZONE_ERROR, (_T("%s ActivateChild failed.\r\n"), 
                            pszFname));
                        RemoveChildByFolder(pDevice);
                        break;
                    }
                }
            }
     
            break;
        }

        case IOCTL_UFN_GET_DEFAULT_CLIENT: {
            if ( !pbOutBuf || (cbOutBuf != sizeof(UFN_CLIENT_INFO)) ) {
                dwRet = ERROR_INVALID_PARAMETER;
                break;
            }

            PUFN_CLIENT_INFO pClientInfo = (PUFN_CLIENT_INFO) pbOutBuf;
            pClientInfo->szDescription[0] = 0;
            pClientInfo->szName[0] = 0;

            dwRet = GetDefaultClientName(pClientInfo->szName,
                dim(pClientInfo->szName));
            if (dwRet == ERROR_SUCCESS) {
                HKEY hkDefaultClient;
                if ( ERROR_SUCCESS == RegOpenKeyEx(pBusContext->hkClients,
                        pClientInfo->szName, 0, 0, &hkDefaultClient) ) {
                    GetUfnDescription(hkDefaultClient, pClientInfo->szDescription, 
                        dim(pClientInfo->szDescription));
                    RegCloseKey(hkDefaultClient);
                }
            }
            else {
                // No default is not an error.
                dwRet = ERROR_SUCCESS;
                pClientInfo->szName[0] = 0;
            }

            if (pcbActualOutBuf) {
                *pcbActualOutBuf = sizeof(UFN_CLIENT_INFO);
            }

            break;
        }

        case IOCTL_UFN_CHANGE_DEFAULT_CLIENT: {
            if ( !pbInBuf || (cbInBuf != sizeof(UFN_CLIENT_NAME)) ) {
                dwRet = ERROR_INVALID_PARAMETER;
                break;
            }

            PUFN_CLIENT_NAME pClientName = (PUFN_CLIENT_NAME) pbInBuf;
            UFN_CLIENT_NAME ClientName;
            HRESULT hr = StringCchCopy(ClientName.szName, dim(ClientName.szName),
                pClientName->szName);
            
            TCHAR szDefaultClientKey[MAX_PATH];
            
            GetDefaultClientKey(szDefaultClientKey, dim(szDefaultClientKey));

            // Make sure that the requested default client exists.
            HKEY hkNewDefaultClient;
            dwRet = RegOpenKeyEx(pBusContext->hkClients, 
                ClientName.szName, 0, 0, &hkNewDefaultClient);
            if (dwRet == ERROR_SUCCESS) {
                RegCloseKey(hkNewDefaultClient);
            
                dwRet = RegSetValueEx(pBusContext->hkClients,
                    szDefaultClientKey, NULL, REG_SZ, (PBYTE) ClientName.szName, 
                    (_tcslen(ClientName.szName) + 1) * sizeof(TCHAR));
                if (dwRet == ERROR_SUCCESS) {
                    DEBUGMSG(ZONE_FUNCTION, (_T("%s Changed default client to \"%s\"\r\n"),
                        pszFname, ClientName.szName));
                }
                else {
                    DEBUGMSG(ZONE_FUNCTION, (_T("%s Failure changing default client to \"%s\". Error = %u\r\n"),
                        pszFname, ClientName.szName, dwRet));
                }
            }
            else {
                DEBUGMSG(ZONE_ERROR, (_T("%s \"%s\" is not an available client\r\n"),
                    pszFname, ClientName.szName));
            }

            break;
        }        

        case IOCTL_UFN_GET_CLIENT_DATA:
        case IOCTL_UFN_GET_CLIENT_DATA_EX: {
            PUFN_CLIENT_DATA pucd = (PUFN_CLIENT_DATA) pbOutBuf;
            
            if ( !pucd || (cbOutBuf != sizeof(UFN_CLIENT_DATA) ) || 
                 (pucd->dwVersion != UFN_CLIENT_INTERFACE_VERSION) ) {
                dwRet = ERROR_INVALID_PARAMETER;
                break;
            }
            
            if (GetCurrentThread() != m_hInitThread) {
                dwRet = ERROR_ACCESS_DENIED;
                break;
            }
            
            DEBUGCHK(m_pContext->fClientIsBeingAddedOrRemoved);
            PREFAST_DEBUGCHK(m_pContext->hKey);

            TCHAR szDll[MAX_PATH];
            DWORD cbDll = sizeof(szDll);
            dwRet = RegQueryValueEx(m_pContext->hKey, DEVLOAD_DLLNAME_VALNAME, 
                NULL, NULL, (PBYTE) szDll, &cbDll);
            if (dwRet != ERROR_SUCCESS) {
                break;
            }
            NULL_TERMINATE(szDll);

            dwRet = GetClientFunctions(&pucd->ufnFunctions);
            if (dwRet != ERROR_SUCCESS) {
                break;
            }

            pucd->hiParent = LoadLibrary(szDll);
            if (!pucd->hiParent) {
                dwRet = GetLastError();
                break;
            }

            pucd->hDevice = (UFN_HANDLE) m_pContext;
            if (pcbActualOutBuf) {
                *pcbActualOutBuf = sizeof(*pucd);
            }

            break;
        }


        case IOCTL_BUS_SET_POWER_STATE:
            m_fIsChildPowerManaged = TRUE;        
            // Fall through to let the default bus driver process it.
        
        default:
            dwRet = ERROR_NOT_SUPPORTED;
            break;
    }

    if (dwRet == ERROR_NOT_SUPPORTED) {
        dwRet = m_pContext->PddInfo.pfnIOControl(
            m_pContext->PddInfo.pvPddContext, BUS_IOCTL, dwCode,
            pbInBuf, cbInBuf, pbOutBuf, cbOutBuf, pcbActualOutBuf);
        if (dwRet != ERROR_SUCCESS) {
            if (dwRet == ERROR_INVALID_PARAMETER) {
                fCallDefaultBusDriver = TRUE;
            }
        }
    }

    if (fCallDefaultBusDriver) {
        DEBUGCHK(dwRet != ERROR_SUCCESS);
        SetLastError(dwRet);
        fRet = DefaultBusDriver::IOControl(dwCode,
            pbInBuf, cbInBuf, pbOutBuf, cbOutBuf, pcbActualOutBuf);
    }
    else if (dwRet != ERROR_SUCCESS) {
        SetLastError(dwRet);
        fRet = FALSE;
    }
    else {
        fRet = TRUE;
    }

    return fRet;
}


DWORD CUfnBus::GetBusNamePrefix(
    LPTSTR pszBusName, 
    DWORD cchBusName
    )
{
    HRESULT hr = StringCchCopy(pszBusName, cchBusName, UFN_BUS_PREFIX);
    return _tcslen(pszBusName) + 1;
}


DWORD
CUfnBus::CleanUpAfterClient(
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PREFAST_DEBUGCHK(m_pContext);
    
    BOOL fClientActive = IsClientActive();

    DWORD dwRet = ERROR_SUCCESS;

    DEBUGCHK(m_pContext->fClientIsBeingAddedOrRemoved == FALSE);
    m_pContext->fClientIsBeingAddedOrRemoved = TRUE;

    if (m_pContext->fRunning) {
        if (fClientActive) {
            if (m_pContext->deviceState != DS_DETACHED) {
                ChangeDeviceState(m_pContext, DS_DETACHED);
                BOOL fRet = SendDeviceNotification(m_pContext->lpDeviceNotify, 
                    m_pContext->pvDeviceNotifyParameter, UFN_MSG_BUS_EVENTS, UFN_DETACH);
            }
        }

        DoStop(m_pContext);
    }
    
    if (m_pContext->fRegistered) {
        DoDeregisterDevice(m_pContext);
    }

    if (fClientActive) {
        CUfnBusDevice *pDevice = (CUfnBusDevice *) GetDeviceList();
        DEBUGCHK(pDevice);
        
        if (!DefaultBusDriver::DeactivateChild(m_szClientBusName)) {
            // We got an error for some reason. We'll return it but keep going.
            dwRet = GetLastError();
            DEBUGMSG(ZONE_WARNING, (_T("%s DeactivateDevice on \"%s\" failed\r\n"), 
                pszFname, pDevice->GetUfnName()));
        }
        else {
            DEBUGMSG(ZONE_INIT, (_T("%s Deactivated client driver \"%s\"\r\n"), 
                pszFname, pDevice->GetUfnName()));
        }
    }

    m_pContext->fClientIsBeingAddedOrRemoved = FALSE;

    FreePipes(m_pContext);

    DEBUGCHK(m_pContext->pFreeTransferList);
    delete m_pContext->pFreeTransferList;
    m_pContext->pFreeTransferList = NULL;

    m_pContext->deviceStatePriorToSuspend = DS_DETACHED;
    m_pContext->Speed = BS_FULL_SPEED;

    FUNCTION_LEAVE_MSG();

    return dwRet;
}


VOID
CUfnBus::GetDefaultClientKey(
    LPTSTR pszDefaultClientKey,
    DWORD cchDefaultClientKey
    )
{
    DEBUGCHK(pszDefaultClientKey);
    
    DWORD cbData = cchDefaultClientKey * sizeof(TCHAR);
    DWORD dwType;
    DWORD dwRet = RegQueryValueEx(m_pContext->hKey,
        PSZ_REG_DEFAULT_CLIENT_KEY, NULL, 
        &dwType, (PBYTE) pszDefaultClientKey, &cbData);
    pszDefaultClientKey[cchDefaultClientKey - 1] = 0;
    if ( (dwRet != ERROR_SUCCESS) || (dwType != REG_SZ) ) {
        // Use the default
        StringCchCopy(pszDefaultClientKey,cchDefaultClientKey,
            PSZ_REG_DEFAULT_DEFAULT_CLIENT_DRIVER);
    }
}


DWORD
CUfnBus::GetDefaultClientName(
    LPTSTR pszClientName,
    DWORD cchClientName
    )
{
    SETFNAME();
    DEBUGCHK(pszClientName);
    
    TCHAR szDefaultClientKey[MAX_PATH];
    HKEY hkFunctions = NULL;
    
    DWORD dwRet = OpenFunctionKey(&hkFunctions);
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }
    
    GetDefaultClientKey(szDefaultClientKey, dim(szDefaultClientKey));

    DEBUGMSG(ZONE_FUNCTION, (_T("%s Using default client key named \"%s\"\r\n"),
        pszFname, szDefaultClientKey));    

    DWORD cbData = cchClientName * sizeof(TCHAR);
    DWORD dwType;
    dwRet = RegQueryValueEx(hkFunctions, szDefaultClientKey, NULL, 
        &dwType, (PBYTE) pszClientName, &cbData);
    pszClientName[cchClientName - 1] = 0;
    if ( (dwRet != ERROR_SUCCESS) || (dwType != REG_SZ) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Failed to read the client driver key name. Error: %d\r\n"),
            pszFname, dwRet));
        goto EXIT;
    }
    else if (pszClientName[0] == 0) {
        // No client selected. This is not an error state--we do not require
        // a client since the user can tell us to activate one later.
        DEBUGMSG(ZONE_WARNING, (_T("%s No default client driver listed in \"%s\".\r\n"), 
            pszFname, szDefaultClientKey));
        dwRet = ERROR_SUCCESS;
        goto EXIT;
    }

EXIT:
    if (hkFunctions) RegCloseKey(hkFunctions);

    return dwRet;       
}

