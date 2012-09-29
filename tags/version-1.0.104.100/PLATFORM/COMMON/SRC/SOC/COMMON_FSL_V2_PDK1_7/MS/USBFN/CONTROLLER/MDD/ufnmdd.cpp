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
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        UFNMDD.CPP

Abstract:

        USB Function Controller Model Device Driver Implementation.
        
--*/

#include "ufnmdd.h"
#pragma warning(push)
#pragma warning(disable: 4189 6001 6262 6385)
#include <devload.h>
#include <creg.hxx>
#pragma warning(pop)
 

#define VERIFY_RUNNING() \
    if (!pContext->fRunning) { \
        DEBUGMSG(ZONE_ERROR, (_T("%s Device is not running\r\n"), pszFname)); \
        dwRet = ERROR_INVALID_HANDLE; \
        goto EXIT; \
    }


extern "C" 
BOOL
UFN_PreClose (
    DWORD dwContext
    );

extern "C" 
BOOL
UFN_Close (
    DWORD dwContext
    );

#ifdef DEBUG

// Validate a context object.
VOID
ValidateContext(
    PUFN_MDD_CONTEXT pContext
    )
{
    PREFAST_DEBUGCHK(pContext);
    DEBUGCHK(pContext->dwSig == UFN_MDD_SIG);

    
    if (pContext->Speed != BS_UNKNOWN_SPEED) {
        DEBUGCHK(pContext->Speed & pContext->PddInfo.dwCapabilities);
    }

    if (!pContext->fClientIsBeingAddedOrRemoved) {
        if (pContext->pUfnBus->IsClientActive()) {
            DEBUGCHK(pContext->rgpPipes != NULL);
            DEBUGCHK(pContext->pFreeTransferList);
        }
        else {
            DEBUGCHK(pContext->rgpPipes == NULL);
            DEBUGCHK(pContext->pFreeTransferList == NULL);
        }
    }
    else {
        DEBUGCHK(pContext->rgpPipes);
        DEBUGCHK(pContext->pFreeTransferList);
    }
}

#endif

#ifndef SHIP_BUILD

LPCTSTR
GetSpeedString(
    UFN_BUS_SPEED Speed
    )
{
    LPCTSTR pszSpeed = _T("unknown");

    if (Speed == BS_FULL_SPEED) {
        pszSpeed = _T("full");
    }
    else if (Speed == BS_HIGH_SPEED) {
        pszSpeed = _T("high");
    }

    return pszSpeed;
}

#endif


// Reset the pipe status table to its default state.
static
VOID
ResetPipeStatusTable(
    PUFN_MDD_CONTEXT pContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PREFAST_DEBUGCHK(pContext);
    DEBUGCHK(pContext->rgpPipes);

    // the default endpoint is always endpoint 0
    for (DWORD dwEp = 0; dwEp < pContext->PddInfo.dwEndpointCount; ++dwEp) {
        PCPipeBase pPipe = pContext->rgpPipes[dwEp];
        pPipe->Reset();
    }

    FUNCTION_LEAVE_MSG();
}


static
VOID
InitializeEndpointZeroDescriptor(
    PUFN_MDD_CONTEXT pContext,
    BYTE bMaxPacketSize,
    PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc
    )
{
    DEBUGCHK(pContext);
    PREFAST_DEBUGCHK(pEndpointDesc);
    
    pEndpointDesc->bEndpointAddress = 0;
    pEndpointDesc->bmAttributes = USB_ENDPOINT_TYPE_CONTROL;
    pEndpointDesc->wMaxPacketSize = bMaxPacketSize;
}


// Search for the target endpoint address. The endpoint address is
// stored in the endpoint descriptor.
PCPipeBase
FindPipe(
    PUFN_MDD_CONTEXT pContext,
    DWORD dwEndpointAddress
    )
{
    PCPipeBase pPipe = NULL;
    PREFAST_DEBUGCHK(pContext);
    PREFAST_DEBUGCHK(pContext->rgpPipes);
    DEBUGCHK(dwEndpointAddress != -1);
    
    for (DWORD dwPipe = 0; dwPipe < pContext->PddInfo.dwEndpointCount; ++dwPipe) {
        if (pContext->rgpPipes[dwPipe]->GetEndpointAddress() == dwEndpointAddress) {
            pPipe = pContext->rgpPipes[dwPipe];
            break;
        }
    }

    return pPipe;
}


static
DWORD
ReadRegDword(
    HKEY hKey,
    LPCTSTR pszKey,
    BOOL fMustSucceed,
    PDWORD pdwValue
    )
{
    SETFNAME();
    
    PREFAST_DEBUGCHK(hKey);
    DEBUGCHK(pszKey);
    DEBUGCHK(pdwValue);

    DWORD dwType;
    DWORD cbData = sizeof(DWORD);
    DWORD dwErr = RegQueryValueEx(hKey, pszKey, NULL, 
        &dwType, (PBYTE) pdwValue, &cbData);
    
    if ( (dwErr == ERROR_SUCCESS) && (dwType != REG_DWORD) ) {
        dwErr = ERROR_INVALID_DATA;
    }

    if ( fMustSucceed && (dwErr != ERROR_SUCCESS) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Failed to read %s. Error: %d\r\n"),
            pszFname, pszKey, dwErr));
    }

    return dwErr;
}


// Register the descriptors with the MDD and PDD.
DWORD
WINAPI
UfnMdd_RegisterDevice(
    UFN_HANDLE              hDevice,
    PUSB_DEVICE_DESCRIPTOR  pHighSpeedDeviceDesc,
    PUFN_CONFIGURATION      pHighSpeedConfig,
    PUSB_DEVICE_DESCRIPTOR  pFullSpeedDeviceDesc,
    PUFN_CONFIGURATION      pFullSpeedConfig,
    PCUFN_STRING_SET        pStringSets,
    DWORD                   cStringSets
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet = ERROR_INVALID_PARAMETER;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    BOOL fTookCS = FALSE;
    CDescriptors *pdescClient = NULL;
    
    VERIFY_PCONTEXT();

    EnterCriticalSection(&pContext->csMddAccess);
    fTookCS = TRUE;

    if ( (pHighSpeedDeviceDesc == NULL) || (pFullSpeedDeviceDesc == NULL) || 
         (pHighSpeedConfig == NULL) || (pFullSpeedConfig == NULL) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid parameter\r\n"), pszFname));
        goto EXIT;
    }

    if (pContext->fRunning) {
        // Must stop device before registering
        DEBUGMSG(ZONE_ERROR, (_T("%s Device is still running\r\n"), pszFname));
        dwRet = ERROR_BAD_ENVIRONMENT;
        goto EXIT;
    }

    if (pContext->fRegistered) {
        // Must deregister before re-registering
        DEBUGMSG(ZONE_ERROR, (_T("%s Device is still registered\r\n"), pszFname));
        dwRet = ERROR_BAD_ENVIRONMENT;
        goto EXIT;
    }
    
    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try {
        pdescClient = new CDescriptors;
        if( pdescClient == NULL )
        {
            // Failed to allocate memory for device descriptors
            DEBUGMSG(ZONE_ERROR, (_T("RegisterDevice failed to allocate memory for device descriptors\r\n")));
            dwRet = ERROR_OUTOFMEMORY;
            goto EXIT;
        }
        
        dwRet = CDescriptors::ValidateDescriptorAndChildren(pHighSpeedDeviceDesc,
            pHighSpeedConfig, pFullSpeedDeviceDesc, pFullSpeedConfig, pStringSets,
            cStringSets);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
        
        dwRet = pdescClient->Init(pHighSpeedDeviceDesc,
            pHighSpeedConfig, pFullSpeedDeviceDesc, pFullSpeedConfig, pStringSets,
            cStringSets);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
    
        // Verify that the device and config descriptors are supportable.
        dwRet = pdescClient->IsSupportable(pContext);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
        
        // Copy the structures so we are no longer referencing the client's
        // structures.
        dwRet = pdescClient->Clone(&pContext->pDescriptors);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }

        dwRet = pContext->pDescriptors->RegisterDevice(pContext);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }

        pContext->fRegistered = TRUE;
        DEBUGMSG(ZONE_INIT, (_T("%s Device registered\r\n"), pszFname));
        dwRet = ERROR_SUCCESS;
    } // __try
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Exception!\r\n"), pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
    }

EXIT:
#pragma warning(push)
#pragma warning(disable: 6011)
    if (dwRet != ERROR_SUCCESS) {
        DEBUGCHK(pContext->fRegistered == FALSE);
        if (pContext->pDescriptors) {
            delete pContext->pDescriptors;
            pContext->pDescriptors = NULL;
        }
#pragma warning(pop)
        ResetPipeStatusTable(pContext);
    }

    if (pdescClient) delete pdescClient;

    FUNCTION_LEAVE_MSG();
    if (fTookCS) {
        LeaveCriticalSection(&pContext->csMddAccess);
    }

    return dwRet;
}


// Attach the device to the USB bus and register notification routines.
DWORD
WINAPI
UfnMdd_Start(
    UFN_HANDLE      hDevice,
    LPUFN_NOTIFY    lpDeviceNotify,
    PVOID           pvDeviceNotifyParameter,
    PUFN_PIPE       phDefaultPipe
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    PCPipeBase pPipeZero = NULL;
    BOOL fTookCS = FALSE;
    
    VERIFY_PCONTEXT();

    EnterCriticalSection(&pContext->csMddAccess);
    fTookCS = TRUE;

    if (phDefaultPipe == NULL) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid parameter\r\n"), pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
        goto EXIT;
    }

    if (!pContext->fRegistered) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Device has not been registered\r\n"),
            pszFname)); 
        dwRet = ERROR_BAD_ENVIRONMENT;
        goto EXIT;
    }

    pContext->lpDeviceNotify = lpDeviceNotify;
    pContext->pvDeviceNotifyParameter = pvDeviceNotifyParameter;

    // For endpoint 0, transfer type and direction are ignored. Since
    // we do not know what speed bus this will be hooked up to, we use
    // the full speed endpoint 0 packet size.
    USB_ENDPOINT_DESCRIPTOR EndpointDesc;
    PCUSB_DEVICE_DESCRIPTOR pDeviceDesc = 
        pContext->pDescriptors->GetDeviceDescriptor(BS_FULL_SPEED);
    CMddEndpoint::InitializeEndpointZeroDescriptor(
        pDeviceDesc->bMaxPacketSize0, &EndpointDesc);
    pPipeZero = pContext->rgpPipes[0];
    dwRet = pPipeZero->Open(BS_FULL_SPEED, &EndpointDesc);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Could not init endpoint 0\r\n"), pszFname));
        goto EXIT;
    }

#pragma warning(push)
#pragma warning(disable: 4245)
    DEBUGCHK(pPipeZero->IsReserved(BS_FULL_SPEED, -1, -1, -1));
#pragma warning (pop)

    // Return handle to default pipe
    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try {
        *phDefaultPipe = pPipeZero;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Exception!\r\n"), pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
    }

    pContext->pUfnBus->SetChildDevicePowerState(D0);

    dwRet = pContext->PddInfo.pfnStart(pContext->PddInfo.pvPddContext); // UfnPdd_Start
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }

    // Let Bus Driver know, we are ready.
    if (pContext->hParentBusHandle) {
        DWORD dwUsbFnReady = 1 ;
        BusChildIoControl(pContext->hParentBusHandle, IOCTL_BUS_USBOTG_USBFN_ACTIVE,&dwUsbFnReady, sizeof(dwUsbFnReady));
    }
    
    DEBUGMSG(ZONE_INIT, (_T("%s Function controller running\r\n"), pszFname));
    
    pContext->fRunning = TRUE;
    
EXIT:
    if (dwRet != ERROR_SUCCESS) {
        if (IS_VALID_MDD_CONTEXT(pContext)) {
            if (pPipeZero && pPipeZero->IsOpen()) {
                pPipeZero->Close();
            }

            pContext->lpDeviceNotify = NULL;
            pContext->pvDeviceNotifyParameter = NULL;
        }
    }
    
    FUNCTION_LEAVE_MSG();
    if (fTookCS) {
        LeaveCriticalSection(&pContext->csMddAccess);
    }

    return dwRet;
}


DWORD
DoStop(
    PUFN_MDD_CONTEXT pContext
    )
{
    SETFNAME();

    EnterCriticalSection(&pContext->csMddAccess);
    FUNCTION_ENTER_MSG();

    // At this point, the client should not be able to receive notification callbacks 
    pContext->fRunning = FALSE;
    pContext->lpDeviceNotify = NULL;
    pContext->pvDeviceNotifyParameter = NULL;

    // Close all the pipes except for EP 0.
    for (DWORD dwPipe = 1; dwPipe < pContext->PddInfo.dwEndpointCount; ++dwPipe) {
        PCPipeBase pPipe = pContext->rgpPipes[dwPipe];

        if (pPipe->IsOpen()) {
            VERIFY(pPipe->Close()); // Why would this fail?
        }
    }

    // Leave the critical section while calling PDD's Stop, since it will wait
    // for the IST to close, but the IST might be in the process of calling the
    // MDD's notification routine which also uses this critical section.
    LeaveCriticalSection(&pContext->csMddAccess);
    DWORD dwRet = pContext->PddInfo.pfnStop(pContext->PddInfo.pvPddContext);  // UfnPdd_Stop
    DEBUGCHK(dwRet == ERROR_SUCCESS); // Why would this fail?
    if (pContext->hParentBusHandle) {
        DWORD dwUsbFnReady = 0 ;
        BusChildIoControl(pContext->hParentBusHandle, IOCTL_BUS_USBOTG_USBFN_ACTIVE,&dwUsbFnReady, sizeof(dwUsbFnReady));
    }
    EnterCriticalSection(&pContext->csMddAccess);

    // Close endpoint 0.
    DEBUGCHK(pContext->rgpPipes[0]->IsOpen());
    VERIFY(pContext->rgpPipes[0]->Close()); // Why would this fail?

    pContext->pUfnBus->SetChildDevicePowerState(D4);

    FUNCTION_LEAVE_MSG();
    LeaveCriticalSection(&pContext->csMddAccess);

    return dwRet;
}


// Disconnect the device from the USB bus.
DWORD
WINAPI
UfnMdd_Stop(
    UFN_HANDLE hDevice
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    
    VERIFY_PCONTEXT();
    VERIFY_RUNNING();
    dwRet = DoStop(pContext);

EXIT:
    FUNCTION_LEAVE_MSG();

    return dwRet;
}


DWORD
DoDeregisterDevice(
    PUFN_MDD_CONTEXT pContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    DWORD dwRet;
    
    if (pContext->fRunning) {
        // Must stop device before registering
        DEBUGMSG(ZONE_ERROR, (_T("%s Device is still running\r\n"), pszFname));
        dwRet = ERROR_BAD_ENVIRONMENT;
        goto EXIT;
    }

    if (pContext->fRegistered == FALSE) {
        // Must register before deregistering
        DEBUGMSG(ZONE_ERROR, (_T("%s Device is not registered\r\n"), pszFname));
        dwRet = ERROR_BAD_ENVIRONMENT;
        goto EXIT;
    }

    pContext->PddInfo.pfnDeregisterDevice(pContext->PddInfo.pvPddContext);

    delete pContext->pDescriptors;
    pContext->pDescriptors = NULL;    
    ResetPipeStatusTable(pContext);

    pContext->fRegistered = FALSE;
    dwRet = ERROR_SUCCESS;

EXIT:
    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}


// Clears the registered descriptors.
DWORD
WINAPI
UfnMdd_DeregisterDevice(
    UFN_HANDLE hDevice
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    BOOL fTookCS = FALSE;
    
    VERIFY_PCONTEXT();
    EnterCriticalSection(&pContext->csMddAccess);
    fTookCS = TRUE;
    dwRet = DoDeregisterDevice(pContext);
    
EXIT:
    FUNCTION_LEAVE_MSG();
    if (fTookCS) {
        LeaveCriticalSection(&pContext->csMddAccess);
    }

    return dwRet;
}


// Open a pipe.
DWORD
WINAPI
UfnMdd_OpenPipe(
    UFN_HANDLE   hDevice,
    DWORD        dwEndpointAddress,
    PUFN_PIPE    phPipe
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    PCPipeBase pPipe = NULL;
    BOOL fTookCS = FALSE;
    
    VERIFY_PCONTEXT();
    EnterCriticalSection(&pContext->csMddAccess);
    fTookCS = TRUE;
    
    VERIFY_RUNNING();

    if (phPipe == NULL) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid parameter\r\n"), pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
        goto EXIT;
    }

    if (!pContext->fRegistered) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Device not registered\r\n"), pszFname));
        dwRet = ERROR_INVALID_STATE;
        goto EXIT;
    }

    if (dwEndpointAddress > 0xFF) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid endpoint address = 0x%x\r\n"), 
            pszFname, dwEndpointAddress));
        dwRet = ERROR_INVALID_PARAMETER;
        goto EXIT;
    }

    // Search for the target endpoint address. The endpoint address is
    // stored in the endpoint descriptor.
    pPipe = FindPipe(pContext, dwEndpointAddress);
    if (pPipe == NULL) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid endpoint address = 0x%x\r\n"),
            pszFname, dwEndpointAddress));
        dwRet = ERROR_INVALID_PARAMETER;
        goto EXIT;
    }

    // Open the endpoint
    dwRet = pPipe->Open(pContext->Speed, NULL);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Error opening endpoint 0x%x\r\n"),
            pszFname, pPipe->GetEndpointAddress()));
        goto EXIT;
    }
    
    DEBUGMSG(ZONE_INIT || ZONE_PIPE, (_T("%s Opened endpoint 0x%x\r\n"),
        pszFname, pPipe->GetEndpointAddress()));

    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try {
        *phPipe = pPipe;
        dwRet = ERROR_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Exception!\r\n"), pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
    }
    
EXIT:
    FUNCTION_LEAVE_MSG();
    if (fTookCS) {
        LeaveCriticalSection(&pContext->csMddAccess);
    }

    return dwRet;
}


// Close a pipe.
DWORD
WINAPI
UfnMdd_ClosePipe(
    UFN_HANDLE hDevice,
    UFN_PIPE   hPipe
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet = ERROR_INVALID_PARAMETER;;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    PCPipeBase pPipe = (PCPipeBase) hPipe;
    BOOL fTookCS = FALSE;
    
    VERIFY_PCONTEXT();
    VERIFY_RUNNING();

    dwRet = CPipeBase::ValidatePipeHandle(pPipe);
    if (dwRet != ERROR_SUCCESS) { 
        goto EXIT; 
    }

    if (pPipe->GetPhysicalEndpoint() == 0) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Cannot close endpoint 0 pipe (0x%08x)\r\n"), 
            pszFname, pPipe));
        dwRet = ERROR_INVALID_HANDLE;
        goto EXIT;
    }

    EnterCriticalSection(&pContext->csMddAccess);
    fTookCS = TRUE;
    
    dwRet = pPipe->Close();

EXIT:
    FUNCTION_LEAVE_MSG();
    if (fTookCS) {
        LeaveCriticalSection(&pContext->csMddAccess);
    }
    
    return dwRet;
}


DWORD
WINAPI
UfnMdd_GetTransferStatus(
    UFN_HANDLE   hDevice,
    UFN_TRANSFER hTransfer,
    PDWORD       pcbTransferred,
    PDWORD       pdwUsbError
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    PCUfnMddTransfer pTransfer = (PCUfnMddTransfer) hTransfer;

    VERIFY_PCONTEXT();

    if ( (pcbTransferred == NULL) || (pdwUsbError == NULL) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Bad parameter\r\n"), pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
        goto EXIT;
    }

    dwRet = CUfnMddTransfer::ReferenceTransferHandle(pTransfer);
    if (dwRet == ERROR_SUCCESS) {
        dwRet = CPipeBase::GetTransferStatus(pTransfer, pcbTransferred, pdwUsbError);
        pTransfer->Release();
    }

EXIT:
    FUNCTION_LEAVE_MSG();

    return dwRet;
}


DWORD
WINAPI
UfnMdd_AbortTransfer(
    UFN_HANDLE   hDevice,
    UFN_TRANSFER hTransfer
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    PCUfnMddTransfer pTransfer = (PCUfnMddTransfer) hTransfer;

    VERIFY_PCONTEXT();
    VERIFY_RUNNING();

    dwRet = CUfnMddTransfer::ReferenceTransferHandle(pTransfer);
    if (dwRet == ERROR_SUCCESS) {     
        PCPipeBase pPipe = pTransfer->GetPipe();
        dwRet = pPipe->AbortTransfer(pTransfer);
        pTransfer->Release();
    }

EXIT:
    FUNCTION_LEAVE_MSG();

    return dwRet;
}


DWORD
WINAPI
UfnMdd_CloseTransfer(
    UFN_HANDLE   hDevice,
    UFN_TRANSFER hTransfer
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    PCUfnMddTransfer pTransfer = (PCUfnMddTransfer) hTransfer;

    VERIFY_PCONTEXT();

    dwRet = CUfnMddTransfer::ReferenceTransferHandle(pTransfer);
    if (dwRet == ERROR_SUCCESS) {
        PCPipeBase pPipe = pTransfer->GetPipe();
        dwRet = pPipe->CloseTransfer(pTransfer);
        pTransfer->Release();
    }

EXIT:
    FUNCTION_LEAVE_MSG();

    return dwRet;
}


DWORD
WINAPI
UfnMdd_IssueTransfer(
    UFN_HANDLE                  hDevice,
    UFN_PIPE                    hPipe,
    LPTRANSFER_NOTIFY_ROUTINE   lpNotifyRoutine,
    PVOID                       pvNotifyContext,
    DWORD                       dwFlags,
    DWORD                       cbBuffer, 
    PVOID                       pvBuffer,
    DWORD                       dwBufferPhysicalAddress,
    PVOID                       pvPddTransferInfo,
    PUFN_TRANSFER               phTransfer
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    PCPipeBase pPipe = (PCPipeBase) hPipe;
    
    VERIFY_PCONTEXT();
    VERIFY_RUNNING();

    dwRet = CPipeBase::ValidatePipeHandle(pPipe);
    if (dwRet != ERROR_SUCCESS) { 
        goto EXIT; 
    }

    if ( (cbBuffer && (pvBuffer == NULL)) || (phTransfer == NULL) || 
          ( (dwFlags != USB_IN_TRANSFER) && (dwFlags != USB_OUT_TRANSFER) ) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Bad parameter\r\n"), pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
        goto EXIT;
    }

    PCUfnMddTransfer *ppTransfer;
    ppTransfer = (PCUfnMddTransfer*) phTransfer;
    dwRet = pPipe->IssueTransfer(lpNotifyRoutine, 
        pvNotifyContext, dwFlags, cbBuffer, pvBuffer, 
        dwBufferPhysicalAddress, ppTransfer, pvPddTransferInfo);

#if 0
    // Verify that the transfer was assigned properly
    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try {
        if (dwRet == ERROR_SUCCESS) {
            DEBUGCHK(*phTransfer != NULL);
        }
        else {
            DEBUGCHK(*phTransfer == NULL);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGCHK(dwRet != ERROR_SUCCESS);
    }
#endif

EXIT:
    FUNCTION_LEAVE_MSG();

    return dwRet;
}


// Stall a pipe.
DWORD
WINAPI
UfnMdd_StallPipe(
    UFN_HANDLE hDevice,
    UFN_PIPE   hPipe
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    PCPipeBase pPipe = (PCPipeBase) hPipe;
    
    VERIFY_PCONTEXT();
    VERIFY_RUNNING();
    
    dwRet = CPipeBase::ValidatePipeHandle(pPipe);
    if (dwRet != ERROR_SUCCESS) { 
        goto EXIT; 
    }
    dwRet = pPipe->Stall();

EXIT:
    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}


// Clear a pipe stall.
DWORD
WINAPI
UfnMdd_ClearPipeStall(
    UFN_HANDLE hDevice,
    UFN_PIPE   hPipe
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    PCPipeBase pPipe = (PCPipeBase) hPipe;
    
    VERIFY_PCONTEXT();
    VERIFY_RUNNING();

    dwRet = CPipeBase::ValidatePipeHandle(pPipe);
    if (dwRet != ERROR_SUCCESS) { 
        goto EXIT; 
    }
    
    dwRet = pPipe->ClearStall();

EXIT:
    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}


DWORD
WINAPI
UfnMdd_InitiateRemoteWakeup(
    UFN_HANDLE hDevice
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;

    VERIFY_PCONTEXT();
    VERIFY_RUNNING();

    if (!pContext->fRemoteWakeupEnabled) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Remote wakeup has not been enabled by the host\r\n"),
            pszFname));
        dwRet = ERROR_GEN_FAILURE;
        goto EXIT;
    }

    dwRet = pContext->PddInfo.pfnInitiateRemoteWakeup(pContext->PddInfo.pvPddContext);
    
EXIT:
    FUNCTION_LEAVE_MSG();

    return dwRet;
}


// Send the control status handshake. Called after sending or receiving the 
// entire data transfer for a control transfer.
DWORD
WINAPI
UfnMdd_SendControlStatusHandshake(
    UFN_HANDLE hDevice
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) hDevice;
    PCPipeBase pPipe;
    
    VERIFY_PCONTEXT();
    VERIFY_RUNNING();

    ValidateContext(pContext);
    DEBUGCHK(pContext->rgpPipes);
    
    pPipe = pContext->rgpPipes[0];
    dwRet = pPipe->SendControlStatusHandshake();

EXIT:
    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}


// Free all the context resources.
static
VOID
FreeContext(
    PUFN_MDD_CONTEXT pContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    PREFAST_DEBUGCHK(pContext);

    // Deinitialize the client.
    if (pContext->pUfnBus) {
        delete pContext->pUfnBus;
    }

    if (pContext->fPddInitialized) {
        // Deinitialize the PDD
        pContext->PddInfo.pfnDeinit(pContext->PddInfo.pvPddContext);
    }
    
    if (pContext->hKey) RegCloseKey(pContext->hKey);

    DEBUGCHK(pContext->hEnumTransfer == NULL);

    if (pContext->hevReadyForNotifications) CloseHandle(pContext->hevReadyForNotifications);

    if (pContext->pOpenList) delete pContext->pOpenList;

    DeleteCriticalSection(&pContext->csMddAccess);
    DeleteCriticalSection(&pContext->csBusIoctlAccess);

    if (pContext->hParentBusHandle) {
        CloseBusAccessHandle(pContext->hParentBusHandle);
    }

    pContext->dwSig = GARBAGE_DWORD;
    LocalFree(pContext);

    FUNCTION_LEAVE_MSG();
}


static
BOOL
IsPddInterfaceInfoValid(
    PUFN_PDD_INTERFACE_INFO pPddInterfaceInfo
    )
{
    BOOL fRet = TRUE;
    
    if ( !pPddInterfaceInfo ||
         pPddInterfaceInfo->dwVersion != UFN_PDD_INTERFACE_VERSION ||
         pPddInterfaceInfo->dwEndpointCount == 0 ||
         (pPddInterfaceInfo->dwCapabilities & UFN_PDD_CAPS_SUPPORTS_FULL_SPEED) == 0 ||
         !pPddInterfaceInfo->pfnDeinit || 
         !pPddInterfaceInfo->pfnIsConfigurationSupportable || 
         !pPddInterfaceInfo->pfnIsEndpointSupportable || 
         !pPddInterfaceInfo->pfnInitEndpoint ||
         !pPddInterfaceInfo->pfnRegisterDevice || 
         !pPddInterfaceInfo->pfnDeregisterDevice || 
         !pPddInterfaceInfo->pfnStart || 
         !pPddInterfaceInfo->pfnStop || 
         !pPddInterfaceInfo->pfnIssueTransfer ||
         !pPddInterfaceInfo->pfnAbortTransfer || 
         !pPddInterfaceInfo->pfnDeinitEndpoint || 
         !pPddInterfaceInfo->pfnStallEndpoint || 
         !pPddInterfaceInfo->pfnClearEndpointStall || 
         !pPddInterfaceInfo->pfnSendControlStatusHandshake ||
         !pPddInterfaceInfo->pfnSetAddress || 
         !pPddInterfaceInfo->pfnIsEndpointHalted || 
         !pPddInterfaceInfo->pfnInitiateRemoteWakeup || 
         !pPddInterfaceInfo->pfnPowerDown || 
         !pPddInterfaceInfo->pfnPowerUp || 
         !pPddInterfaceInfo->pfnIOControl ) {
        fRet = FALSE;
    }

    return fRet;
}


// Initialize the MDD, PDD, and client.
extern "C"
DWORD
UFN_Init(
    LPCTSTR pszContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    DWORD dwErr;
    PUFN_MDD_CONTEXT pContext = NULL;
    CReg regActive;

    UFN_MDD_INTERFACE_INFO MddInterfaceInfo = {
        UFN_PDD_INTERFACE_VERSION, &UfnMdd_Notify
    };

    DEBUGCHK(pszContext);

    // If this DEBUGCHK fires, that means that the MDD's DllEntry
    // is not being called. Please verify that you are using that 
    // entry point and step through it in the debugger.
    DEBUGCHK(CTransferQueue<CUfnMddTransfer>::InitializeClassCalled());

    if (!regActive.Open(HKEY_LOCAL_MACHINE, pszContext)) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Failed to open active key\r\n"), pszFname));
        goto EXIT;
    }
    // Allocate our context

    pContext = (PUFN_MDD_CONTEXT) LocalAlloc(LPTR, sizeof(*pContext));
    if (pContext == NULL) {
        DEBUGMSG(ZONE_ERROR, (_T("%s LocalAlloc failed. Error: %d\r\n"), pszFname, 
            GetLastError()));
        goto EXIT;
    }
    pContext->dwSig = UFN_MDD_SIG;
    pContext->Speed = BS_UNKNOWN_SPEED;
    pContext->uOTGCapabilityBits = 0 ;
    pContext->hParentBusHandle = CreateBusAccessHandle(pszContext ) ;  
    if (pContext->hParentBusHandle) {
        if (!BusChildIoControl(pContext->hParentBusHandle, IOCTL_BUS_USBOTG_GETOTG_ENABLE_BIT, &(pContext->uOTGCapabilityBits),sizeof(pContext->uOTGCapabilityBits)))
            pContext->uOTGCapabilityBits = 0 ;
        //BusChildIoControl(pContext->hParentBusHandle, IOCTL_BUS_USBOTG_REQUEST_BUS,NULL, 0);
    }
    
    InitializeCriticalSection(&pContext->csMddAccess);
    InitializeCriticalSection(&pContext->csBusIoctlAccess);

#pragma warning(push)
#pragma warning(disable: 28197)
    pContext->pOpenList = new COpenList();
#pragma warning(pop)
    if (pContext->pOpenList == NULL) {
        DEBUGMSG(ZONE_ERROR, (_T("%s new failed. Error: %d\r\n"), pszFname, 
            GetLastError()));
        goto EXIT;
    }

#ifdef FSL_MERGE
    pContext->bEnterTestMode = FALSE;
    pContext->iTestMode = 0;
#endif

    pContext->hevReadyForNotifications = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (pContext->hevReadyForNotifications == NULL) {
        dwErr = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s Failed to create event. Error = %d\r\n"),
            pszFname, dwErr));
        goto EXIT;
    }

    // Open the device key
    pContext->hKey = OpenDeviceKey(pszContext);
    if (pContext->hKey == NULL) {
        dwErr = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s Failed to open device key \"%s\". Error = %d\r\n"),
            pszFname, pszContext, dwErr));
        goto EXIT;
    }

#pragma warning (push)
#pragma warning( disable:28197 )     
    pContext->pUfnBus = new CUfnBus(pszContext, pContext);
    if ( (pContext->pUfnBus == NULL) || (!pContext->pUfnBus->Init()) ) {
        goto EXIT;
    }
#pragma warning (pop)

    // Initialize the PDD
    dwErr = UfnPdd_Init(pszContext, pContext, &MddInterfaceInfo, &pContext->PddInfo);
    if (dwErr != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s PDD initialization failed.\r\n"), pszFname));
        goto EXIT;
    }

    if (IsPddInterfaceInfoValid(&pContext->PddInfo)) { 
        DEBUGMSG(ZONE_INIT, (_T("%s PDD has %u endpoints\r\n"), pszFname, 
            pContext->PddInfo.dwEndpointCount));

        DEBUGMSG(ZONE_INIT, (_T("%s PDD supports speeds 0x%x\r\n"), pszFname, 
            pContext->PddInfo.dwCapabilities));
        
        pContext->fPddInitialized = TRUE;
    }
    else {
        RETAILMSG(1, (_T("%s PDD interface info is invalid.\r\n"), 
            pszFname));
        // Try to call PDD's deinit.
        if (pContext->PddInfo.pfnDeinit) {
            pContext->PddInfo.pfnDeinit(pContext->PddInfo.pvPddContext);
        }
    }

EXIT:
    if (pContext && !pContext->fPddInitialized) {
        // Failed to initialize. Free resources.
        FreeContext(pContext);
        pContext = NULL;
    }
    
    FUNCTION_LEAVE_MSG();
    
    return (DWORD) pContext;
}


extern "C" 
BOOL
UFN_PreDeinit (
    DWORD dwContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    DWORD dwRet = ERROR_SUCCESS;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) dwContext;
    
    VERIFY_PCONTEXT();
    
    ValidateContext(pContext);
    pContext->pOpenList->PreDeinit(UFN_PreClose);
    
EXIT:
    FUNCTION_LEAVE_MSG();
    
    return (dwRet == ERROR_SUCCESS);
}


// Deinitialize the client, PDD, and MDD.
extern "C" 
BOOL
UFN_Deinit (
    DWORD dwContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet = ERROR_SUCCESS;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) dwContext;
    
    VERIFY_PCONTEXT();
    
    ValidateContext(pContext);
    pContext->pOpenList->Deinit(UFN_Close);    
    FreeContext(pContext);
    
EXIT:
    FUNCTION_LEAVE_MSG();
    
    return (dwRet == ERROR_SUCCESS);
}


static
VOID
ValidateBusContext(
    PUFN_MDD_BUS_OPEN_CONTEXT pBusContext
    )
{
    PREFAST_DEBUGCHK(pBusContext);
    DEBUGCHK(pBusContext->dwSig == UFN_MDD_BUS_SIG);
    DEBUGCHK(pBusContext->pMddContext);
    ValidateContext(pBusContext->pMddContext);
    DEBUGCHK(pBusContext->hkClients);
}


extern "C" 
DWORD
UFN_Open (
    DWORD dwContext,
    DWORD dwAccessMode, 
    DWORD dwShareMode
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet;
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) dwContext;
    DEBUGCHK(IS_VALID_MDD_CONTEXT(pContext));

    if (dwAccessMode & DEVACCESS_BUSNAMESPACE) {        
        // This is a bus access
        PUFN_MDD_BUS_OPEN_CONTEXT pBusContext = (PUFN_MDD_BUS_OPEN_CONTEXT) 
            LocalAlloc(LPTR, sizeof(UFN_MDD_BUS_OPEN_CONTEXT));
        if (pBusContext) {
            pBusContext->dwSig = UFN_MDD_BUS_SIG;
            pBusContext->pMddContext = pContext;

            DWORD dwErr = CUfnBus::OpenFunctionKey(&pBusContext->hkClients);
            if (dwErr != ERROR_SUCCESS) {
                LocalFree(pBusContext);
                pBusContext = NULL;
            }
        }

        if (pBusContext) {
            ValidateBusContext(pBusContext);
            if (!pContext->pOpenList->AddContext((DWORD) pBusContext)) {
                LocalFree(pBusContext);
                pBusContext = NULL;                
            }
        }

        dwRet = (DWORD) pBusContext;
    }
    else {
        // Standard access
        dwRet = (DWORD) pContext;
    }
    
    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}


extern "C" 
BOOL
UFN_PreClose (
    DWORD dwContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    DWORD dwRet = ERROR_SUCCESS;
    PUFN_MDD_CONTEXT_SIG pMddContextSig = (PUFN_MDD_CONTEXT_SIG) dwContext;
    
    if (pMddContextSig == NULL) {
        dwRet = ERROR_INVALID_HANDLE;
    }
    else if (pMddContextSig->dwSig == UFN_MDD_BUS_SIG) {
        PUFN_MDD_BUS_OPEN_CONTEXT pBusContext = 
            (PUFN_MDD_BUS_OPEN_CONTEXT) pMddContextSig;
        ValidateBusContext(pBusContext);
    }
    else if (pMddContextSig->dwSig == UFN_MDD_SIG) {
        PUFN_MDD_CONTEXT pMddContext = 
            (PUFN_MDD_CONTEXT) pMddContextSig;
        ValidateContext(pMddContext);
    }
    
    // No threads to release.

    if (dwRet != ERROR_SUCCESS) {
        SetLastError(dwRet);
    }

    FUNCTION_LEAVE_MSG();
    
    return (dwRet == ERROR_SUCCESS);
}


extern "C" 
BOOL
UFN_Close (
    DWORD dwContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet = ERROR_SUCCESS;
    PUFN_MDD_CONTEXT_SIG pMddContextSig = (PUFN_MDD_CONTEXT_SIG) dwContext;

    if (pMddContextSig == NULL) {
        dwRet = ERROR_INVALID_HANDLE;
    }
    else if (pMddContextSig->dwSig == UFN_MDD_BUS_SIG) {
        PUFN_MDD_BUS_OPEN_CONTEXT pBusContext = 
            (PUFN_MDD_BUS_OPEN_CONTEXT) pMddContextSig;
        ValidateBusContext(pBusContext);
        RegCloseKey(pBusContext->hkClients);
        
        PUFN_MDD_CONTEXT pContext = pBusContext->pMddContext;
        pContext->pOpenList->RemoveContext(dwContext);        
        LocalFree(pBusContext);
    }
    else if (pMddContextSig->dwSig == UFN_MDD_SIG) {
        PUFN_MDD_CONTEXT pMddContext = 
            (PUFN_MDD_CONTEXT) pMddContextSig;
        ValidateContext(pMddContext);
    }
    else {
        DEBUGCHK(FALSE);
        dwRet = ERROR_INVALID_HANDLE;
    }
    
    if (dwRet != ERROR_SUCCESS) {
        SetLastError(dwRet);
    }
    
    FUNCTION_LEAVE_MSG();

    return (dwRet == ERROR_SUCCESS);
}


// Pass power down request to the PDD.
extern "C"
void
UFN_PowerDown(
    DWORD dwContext
    )
{
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) dwContext;
    
    DEBUGCHK(IS_VALID_MDD_CONTEXT(pContext));

    if (!pContext->pUfnBus->IsChildPowerManaged()) {
        pContext->pUfnBus->SetChildDevicePowerState(D4);
    }
    
    pContext->PddInfo.pfnPowerDown(pContext->PddInfo.pvPddContext);
}


// Pass power up request to the PDD.
extern "C"
void
UFN_PowerUp(
    DWORD dwContext
    )
{
    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) dwContext;
    
    DEBUGCHK(IS_VALID_MDD_CONTEXT(pContext));
    pContext->PddInfo.pfnPowerUp(pContext->PddInfo.pvPddContext);

    if (!pContext->pUfnBus->IsChildPowerManaged()) {
        pContext->pUfnBus->SetChildDevicePowerState(D0);
    }
}


DWORD
GetClientFunctions(
    PUFN_FUNCTIONS  pUfnFunctions
    )
{
    SETFNAME();
    PREFAST_DEBUGCHK(pUfnFunctions);
    DWORD dwRet = ERROR_SUCCESS;
    
    pUfnFunctions->lpRegisterDevice = &UfnMdd_RegisterDevice;
    pUfnFunctions->lpStart = &UfnMdd_Start;
    pUfnFunctions->lpStop = &UfnMdd_Stop;
    pUfnFunctions->lpDeregisterDevice = &UfnMdd_DeregisterDevice;
    pUfnFunctions->lpOpenPipe = &UfnMdd_OpenPipe;
    pUfnFunctions->lpClosePipe = &UfnMdd_ClosePipe;
    pUfnFunctions->lpIssueTransfer = &UfnMdd_IssueTransfer;
    pUfnFunctions->lpGetTransferStatus = &UfnMdd_GetTransferStatus;
    pUfnFunctions->lpAbortTransfer = &UfnMdd_AbortTransfer;
    pUfnFunctions->lpCloseTransfer = &UfnMdd_CloseTransfer;
    pUfnFunctions->lpStallPipe = &UfnMdd_StallPipe;
    pUfnFunctions->lpClearPipeStall = &UfnMdd_ClearPipeStall;
    pUfnFunctions->lpInitiateRemoteWakeup = &UfnMdd_InitiateRemoteWakeup;
    pUfnFunctions->lpSendControlStatusHandshake = &UfnMdd_SendControlStatusHandshake;

    return dwRet;
}


// IOCTLs with a Function value between 0x200 and 0x2FF are reserved 
// for the OEM and the PDD.
extern "C" 
BOOL
UFN_IOControl (
    DWORD  dwContext,
    DWORD  dwCode,
    PBYTE  pbInBuf,
    DWORD  cbInBuf,
    PBYTE  pbOutBuf,
    DWORD  cbOutBuf,
    PDWORD pcbActualOutBuf
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    BOOL fRet = FALSE;
    PUFN_MDD_CONTEXT_SIG pMddContextSig = (PUFN_MDD_CONTEXT_SIG) dwContext;

    if (pMddContextSig == NULL) {
        SetLastError(ERROR_INVALID_HANDLE);
    }
    else if (pMddContextSig->dwSig == UFN_MDD_BUS_SIG) {
        PUFN_MDD_BUS_OPEN_CONTEXT pBusContext = 
            (PUFN_MDD_BUS_OPEN_CONTEXT) pMddContextSig;
        ValidateBusContext(pBusContext);

        EnterCriticalSection(&pBusContext->pMddContext->csBusIoctlAccess);
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try {
            fRet = pBusContext->pMddContext->pUfnBus->IOControl(pBusContext, dwCode, pbInBuf, cbInBuf,
                pbOutBuf, cbOutBuf, pcbActualOutBuf);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            fRet = FALSE;
        }
        LeaveCriticalSection(&pBusContext->pMddContext->csBusIoctlAccess);
    }
    else if (pMddContextSig->dwSig == UFN_MDD_SIG) {
        PUFN_MDD_CONTEXT pMddContext = 
            (PUFN_MDD_CONTEXT) pMddContextSig;
        ValidateContext(pMddContext);
        
        // Pass all normal IOCTLs to the PDD
        DWORD dwRet = pMddContext->PddInfo.pfnIOControl(
            pMddContext->PddInfo.pvPddContext, EXTERN_IOCTL, dwCode,
            pbInBuf, cbInBuf, pbOutBuf, cbOutBuf, pcbActualOutBuf);
        if (dwRet != ERROR_SUCCESS) {
            SetLastError(dwRet);
        }
        else {
            fRet = TRUE;
        }
    }
    else {
        SetLastError(ERROR_INVALID_HANDLE);
    }

    FUNCTION_LEAVE_MSG();

    return fRet;
}


// Send a notification.
BOOL
SendDeviceNotification(
    LPUFN_NOTIFY lpNotify,
    PVOID        pvNotifyParameter,
    DWORD        dwMsg,
    DWORD        dwParam
    )
{
    SETFNAME();
    
    BOOL fRet = TRUE;
    
    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try {
        if (lpNotify) {
            (*lpNotify)(pvNotifyParameter, dwMsg, dwParam);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Exception in notification routine!\r\n"), pszFname));
        fRet = FALSE;
    }

    return fRet;
}


// Declare the statics required by CTransferQueue<CUfnMddTransfer>
CTransferQueue_DECLARE_STATICS(CUfnMddTransfer);

extern "C"
BOOL
WINAPI
DllEntry(
    HANDLE hDllHandle,
    DWORD  dwReason, 
    LPVOID lpReserved
    )
{
    SETFNAME();
    
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HINSTANCE)hDllHandle);
            DEBUGMSG(ZONE_INIT, (_T("%s Attach\r\n"), pszFname));
            DisableThreadLibraryCalls((HMODULE) hDllHandle);
            svsutil_Initialize();
            CTransferQueue<CUfnMddTransfer>::InitializeClass(DEFAULT_TRANSFER_LIST_SIZE);
            break;
            
        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT, (_T("%s Detach\r\n"), pszFname));
            CTransferQueue<CUfnMddTransfer>::DeinitializeClass();
            svsutil_DeInitialize();
            break;
    }

    return UfnPdd_DllEntry(hDllHandle, dwReason, lpReserved);
}

