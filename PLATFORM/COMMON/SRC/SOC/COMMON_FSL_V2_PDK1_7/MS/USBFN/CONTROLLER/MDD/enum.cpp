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
// Copyright (C) 2005-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//------------------------------------------------------------------------------
#include "ufnmdd.h"
#include <common_usbfnioctl.h>

#define DEBUG_LOG_USBCV 0

#ifdef DEBUG

static const LPCTSTR g_rgpszDeviceStates[] = {
    _T("detached"),
    _T("attached"),
    _T("powered"),
    _T("default"),
    _T("addressed"), 
    _T("configured"),
    _T("suspended"), 
};

// Get a textual name of a device state.
LPCTSTR
GetDeviceStateName(
    DEVICE_STATE ds
    )
{
    DEBUGCHK(ds < dim(g_rgpszDeviceStates));
    LPCTSTR psz = g_rgpszDeviceStates[ds];
    return psz;
}

#endif

DWORD
static
WINAPI 
EnumTransferComplete(
    PVOID pvNotifyParameter
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) pvNotifyParameter;    

    UFN_TRANSFER hTransfer = pContext->hEnumTransfer;
    pContext->hEnumTransfer = NULL;
    DWORD dwUsbError;
    DWORD cbTransferred;

    PREFAST_DEBUGCHK(hTransfer);

    DWORD dwErr = UfnMdd_GetTransferStatus(pContext, hTransfer, &cbTransferred, 
        &dwUsbError);
    DEBUGCHK(dwErr == ERROR_SUCCESS);

    dwErr = UfnMdd_CloseTransfer(pContext, hTransfer);
    DEBUGCHK(dwErr == ERROR_SUCCESS);

    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s %u bytes written\r\n"), pszFname, 
        cbTransferred));

    if (dwUsbError == UFN_NO_ERROR) {
        PCPipeBase pPipe = pContext->rgpPipes[0];
        pPipe->SendControlStatusHandshake();
    }

    FUNCTION_LEAVE_MSG();
    
    return ERROR_SUCCESS;
}


// Setup for a transfer.
static
VOID
SetupTx(
    PUFN_MDD_CONTEXT pContext,
    PVOID  pvBuffer,
    DWORD  cbBuffer,
    DWORD  cbRequested
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    ValidateContext(pContext);
    DEBUGCHK(pvBuffer);

    DWORD cbToSend = min(cbRequested, cbBuffer);
    
    DEBUGMSG(ZONE_SEND, (_T("%s Asked to send %u of %u bytes. Sending %u bytes.\r\n"),
        pszFname, cbRequested, cbBuffer, cbToSend));

    PCPipeBase pPipeEndpoint0 = pContext->rgpPipes[0];
    
    DEBUGCHK(pContext->hEnumTransfer == NULL);
    DWORD dwErr = UfnMdd_IssueTransfer(pContext, pPipeEndpoint0, 
        &EnumTransferComplete, pContext, USB_IN_TRANSFER, cbToSend,
        pvBuffer, 0, NULL, &pContext->hEnumTransfer);

    FUNCTION_LEAVE_MSG();
}


// Handle a Get Descriptor request.
static
CONTROL_RESPONSE
ProcessGetDescriptor(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    ValidateContext(pContext);

    PVOID pvBuffer;
    DWORD cbBuffer;
    
    CONTROL_RESPONSE response = pContext->pDescriptors->ProcessGetDescriptor(pContext->Speed, 
        udr, dwMsg, PddSupportsHighSpeed(pContext), &pvBuffer, &cbBuffer);

    if (response == CR_SUCCESS_TX) {
        SetupTx(pContext, (PBYTE) pvBuffer, cbBuffer, udr.wLength);
    }

    FUNCTION_LEAVE_MSG();

    return response;
}
#ifdef FSL_MERGE
// Handle a Set Descriptor request.
static
CONTROL_RESPONSE
ProcessSetDescriptor(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(udr);
    UNREFERENCED_PARAMETER(dwMsg);
    return CR_STALL_DEFAULT_PIPE;
}
#endif

// Handle a Get Configuration request.
static
CONTROL_RESPONSE
ProcessGetConfiguration(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    ValidateContext(pContext);

    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) {
        // Nothing to do.
        goto EXIT;
    }

    const DWORD dwLength = 1;
    DEBUGCHK(dim(pContext->rgbBuffer) >= dwLength);

    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Get configuration.\r\n"), pszFname)); 

    DWORD dwConfiguration = 
        pContext->pDescriptors->GetConfiguration(pContext->Speed);
    
    DEBUGCHK(
        ( (pContext->deviceState == DS_ADDRESSED)  && (dwConfiguration == 0) ) ||
        ( (pContext->deviceState == DS_CONFIGURED) && (dwConfiguration != 0) )
        );

    pContext->rgbBuffer[0] = (BYTE) dwConfiguration;
    SetupTx(pContext, pContext->rgbBuffer, dwLength, udr.wLength);

    FUNCTION_LEAVE_MSG();

EXIT:
    return CR_SUCCESS;
}


// Handle a Get Interface request.
static
CONTROL_RESPONSE
ProcessGetInterface(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    ValidateContext(pContext);

    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) {
        // Nothing to do.
        goto EXIT;
    }

    const DWORD dwLength = 1;
    DEBUGCHK(dim(pContext->rgbBuffer) >= dwLength);

    CONTROL_RESPONSE response = CR_SUCCESS;

    DWORD dwInterfaceIndex = udr.wIndex;

    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Get interface(%u).\r\n"), pszFname, 
        dwInterfaceIndex));
    
    if (pContext->deviceState == DS_CONFIGURED) {
        DWORD dwCurrentAltSetting;

        DWORD dwError = pContext->pDescriptors->GetInterface(pContext->Speed, 
            dwInterfaceIndex, &dwCurrentAltSetting);
        if (dwError != ERROR_SUCCESS) {
            response = CR_STALL_DEFAULT_PIPE;
            goto EXIT;
        }

        pContext->rgbBuffer[0] = (BYTE) dwCurrentAltSetting;
        SetupTx(pContext, pContext->rgbBuffer, dwLength, udr.wLength);
    }
    else {
        response = CR_STALL_DEFAULT_PIPE;
    }

EXIT:
    FUNCTION_LEAVE_MSG();

    return response;
}


// Handle a Get Status request.
static
CONTROL_RESPONSE
ProcessGetStatus(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    ValidateContext(pContext);

    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) {
        // Nothing to do.
        goto EXIT;
    }

    const DWORD dwLength = 2;
    DEBUGCHK(dim(pContext->rgbBuffer) >= dwLength);
    CONTROL_RESPONSE response = CR_SUCCESS;

    pContext->rgbBuffer[0] = 0;
    pContext->rgbBuffer[1] = 0;

    BYTE bRecipient = GET_REQUESET_RECIPIENT(udr.bmRequestType);

    if (bRecipient == USB_REQUEST_FOR_DEVICE) {
        DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Get device status.\r\n"), pszFname));

        BYTE bStatus = 0;
        DWORD dwConfiguration = pContext->pDescriptors->GetConfiguration(
            pContext->Speed);
        PCUSB_CONFIGURATION_DESCRIPTOR pConfigDesc = 
            pContext->pDescriptors->GetTotalConfigurationDescriptorByValue(
                pContext->Speed, dwConfiguration);
#ifdef QFE_MERGE /*080229*/ /*CE6QFE*/
        if (pConfigDesc == NULL) { // the device hasn't been configured yet. Get first one.
            pConfigDesc =pContext->pDescriptors->GetTotalConfigurationDescriptorByIndex(pContext->Speed, 0 );
            ASSERT(pConfigDesc);
        }

        if (pConfigDesc && (pConfigDesc->bmAttributes & USB_CONFIG_SELF_POWERED)) {
            bStatus |= USB_GETSTATUS_SELF_POWERED;
        }
#else
        if (pConfigDesc->bmAttributes & USB_CONFIG_SELF_POWERED) {
            bStatus |= USB_GETSTATUS_SELF_POWERED;
        }
#endif

        if (pContext->fRemoteWakeupEnabled) {
            bStatus |= USB_GETSTATUS_REMOTE_WAKEUP_ENABLED;
        }

        pContext->rgbBuffer[0] = bStatus;
    }
    else if (bRecipient == USB_REQUEST_FOR_INTERFACE) {
        if (pContext->deviceState != DS_CONFIGURED) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Bad interface request\r\n"),
                pszFname));
            response = CR_STALL_DEFAULT_PIPE;
        }
        // Else just return 0
    }
    else if (bRecipient == USB_REQUEST_FOR_ENDPOINT) {
        DWORD dwEndpointAddress = udr.wIndex;
        PCPipeBase pPipe = FindPipe(pContext, dwEndpointAddress);

        if ( pPipe && 
             ( (pContext->deviceState == DS_CONFIGURED) || (dwEndpointAddress == 0) ) ) {
            BOOL fHalted;
            pContext->PddInfo.pfnIsEndpointHalted(pContext->PddInfo.pvPddContext, 
                pPipe->GetPhysicalEndpoint(), &fHalted);
            if (fHalted) {
                pContext->rgbBuffer[0] = 1;
            }
        }
        else {
            DEBUGMSG(ZONE_ERROR, (_T("%s Bad endpoint request\r\n"),
                pszFname));
            response = CR_STALL_DEFAULT_PIPE;
        }
    }
    else {
        response = CR_UNHANDLED_REQUEST;
    }

    if (response == CR_SUCCESS) {
        // Need to send the data
        SetupTx(pContext, pContext->rgbBuffer, dwLength, udr.wLength);
    }

EXIT:
    FUNCTION_LEAVE_MSG();

    return response;
}


// Handle a Set Adress request.
static
CONTROL_RESPONSE
ProcessSetAddress(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
#ifdef FSL_MERGE
    SETFNAME();
    FUNCTION_ENTER_MSG();

    ValidateContext(pContext);

    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Set address (%u) request.\r\n"),
        pszFname, udr.wValue));

     CONTROL_RESPONSE response;
    if((udr.wValue > 0x7F) ||(udr.wLength !=0) ||(udr.wIndex != 0) ||
       (pContext->deviceState == DS_CONFIGURED))
    {
        DEBUGMSG(ZONE_USB_EVENTS,
                (TEXT("ProcessSetAddress: Incorrect parameters\n")));
        response = CR_STALL_DEFAULT_PIPE;
    }
    else
    {
    if (udr.wValue == 0) {
        ChangeDeviceState(pContext, DS_DEFAULT);
    }
    else {
        ChangeDeviceState(pContext, DS_ADDRESSED);
    }

        if (dwMsg == UFN_MSG_SETUP_PACKET) {
            response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
            pContext->PddInfo.pfnSetAddress(pContext->PddInfo.pvPddContext,
                (BYTE) udr.wValue);
        }
        else {
            response = CR_SUCCESS;
        }
    }

    FUNCTION_LEAVE_MSG();

    return response;
#else
    SETFNAME();
    FUNCTION_ENTER_MSG();

    ValidateContext(pContext);

    DWORD dwAddress = udr.wValue;

    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Set address (%u) request.\r\n"),
        pszFname, dwAddress));

    // Check for invalid conditions according to the USB spec.
    DEBUGCHK(dwAddress <= 0x7F);
    DEBUGCHK(pContext->deviceState != DS_CONFIGURED);

    if (dwAddress == 0) {
        ChangeDeviceState(pContext, DS_DEFAULT);
    }
    else {
        ChangeDeviceState(pContext, DS_ADDRESSED);
    }

    CONTROL_RESPONSE response;
    if (dwMsg == UFN_MSG_SETUP_PACKET) {
        response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
        pContext->PddInfo.pfnSetAddress(pContext->PddInfo.pvPddContext, 
            (BYTE) dwAddress);
    }
    else {
        response = CR_SUCCESS;
    }

    FUNCTION_LEAVE_MSG();

    return response;
#endif
}
    

// Handle a Set Configuration request.
static
CONTROL_RESPONSE
ProcessSetConfiguration(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    CONTROL_RESPONSE response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
    
    ValidateContext(pContext);

    DWORD dwConfigurationValue = udr.wValue;

    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Set configuration (%u) request.\r\n"),
        pszFname, dwConfigurationValue));

    if (dwConfigurationValue == 0) {
        if (pContext->deviceState == DS_CONFIGURED) {
            pContext->pDescriptors->SetConfiguration(pContext->Speed, 0);
            ChangeDeviceState(pContext, DS_ADDRESSED);
            pContext->pDescriptors->SetConfiguration(pContext->Speed, 0);
            SendDeviceNotification(pContext->lpDeviceNotify, 
                pContext->pvDeviceNotifyParameter, UFN_MSG_CONFIGURED, 0);
        }
        // else simply stay in addressed state        
    }
    else {
        DWORD dwCurrConfig = pContext->pDescriptors->GetConfiguration(pContext->Speed);
        if (dwCurrConfig != dwConfigurationValue) {
            PCUSB_CONFIGURATION_DESCRIPTOR pConfigDesc = 
                    pContext->pDescriptors->GetTotalConfigurationDescriptorByValue(
                        pContext->Speed, dwConfigurationValue);

            if (pConfigDesc) {
                if (pContext->deviceState != DS_CONFIGURED) {
                    ChangeDeviceState(pContext, DS_CONFIGURED);
                }
                // else simply stay in configured state

                // close all pipes
                for( DWORD dwPipeIndex = 1; dwPipeIndex < pContext->PddInfo.dwEndpointCount; dwPipeIndex++ )
                {
                    CPipeBase *pPipe = pContext->rgpPipes[dwPipeIndex];
                    if( pPipe )
                    {
                        pPipe->Close();
                    }
                }

                // set the new configuration value
                DWORD dwErr = pContext->pDescriptors->SetConfiguration(pContext->Speed, 
                    dwConfigurationValue);
                DEBUGCHK(dwErr == ERROR_SUCCESS);

                // We add this IOCtrl to inform device driver do bus current limitation work
                pContext->PddInfo.pfnIOControl(pContext->PddInfo.pvPddContext,
                    MDD_IOCTL, IOCTL_UFN_PDD_SET_CONFIGURATION, 
                    (PBYTE) &dwConfigurationValue, sizeof(dwConfigurationValue),
                    NULL, 0, NULL);

                SendDeviceNotification(pContext->lpDeviceNotify, 
                    pContext->pvDeviceNotifyParameter, UFN_MSG_CONFIGURED, dwConfigurationValue);

                // reset all interfaces to alternate setting 0
                DWORD dwInterfaces = pContext->pDescriptors->GetDefaultInterfaceCount(pContext->Speed);
                for (DWORD dwInterface = 0; dwInterface < dwInterfaces; ++dwInterface) {
                    DWORD dwInterfaceParam = UFN_MAKE_INTERFACE_PARAM(dwInterface, 0);
                    pContext->PddInfo.pfnIOControl(pContext->PddInfo.pvPddContext,
                        MDD_IOCTL, IOCTL_UFN_PDD_SET_INTERFACE, 
                        (PBYTE) &dwInterfaceParam, sizeof(dwInterfaceParam),
                        NULL, 0, NULL);
                }
            }
            else {
                response = CR_STALL_DEFAULT_PIPE;
            }
        }
        else {
            // No configuration change
            DEBUGCHK(pContext->deviceState == DS_CONFIGURED);
        }
    }

    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) {
        response = CR_SUCCESS;
    }

    FUNCTION_LEAVE_MSG();

    return response;
}


// Handle a Set Interface request.
static
CONTROL_RESPONSE
ProcessSetInterface(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    CONTROL_RESPONSE response;
    
    ValidateContext(pContext);

    DWORD dwInterface = udr.wIndex;
    DWORD dwAlternateSetting = udr.wValue;

    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Set interface (%u - %u) request.\r\n"),
        pszFname, dwInterface, dwAlternateSetting));

    if (pContext->deviceState != DS_CONFIGURED) {
        response = CR_STALL_DEFAULT_PIPE;
        goto EXIT;
    }

    DWORD dwCurrentAltSetting;

    DWORD dwError = pContext->pDescriptors->GetInterface(pContext->Speed, 
        dwInterface, &dwCurrentAltSetting);
    if (dwError != ERROR_SUCCESS) {
        goto EXIT;
    }
    else if (dwCurrentAltSetting == dwAlternateSetting) {
        // Nothing more to do
        response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
        goto EXIT;
    }
    
    dwError = pContext->pDescriptors->SetInterface(pContext->Speed, 
        dwInterface, dwAlternateSetting);
    if (dwError != ERROR_SUCCESS) {
        response = CR_STALL_DEFAULT_PIPE;
        goto EXIT;
    }

    // Close pipes in old interface
    for( DWORD dwPipeIndex = 1; dwPipeIndex < pContext->PddInfo.dwEndpointCount; dwPipeIndex++ )
    {
        CPipeBase *pPipe = pContext->rgpPipes[dwPipeIndex];
        if( pPipe )
        {
            if( pPipe->IsPartOfInterface( pContext->Speed, pContext->pDescriptors->GetConfiguration(pContext->Speed), dwInterface ) )
            {
                pPipe->Close();
            }
        }
    }

    DWORD dwInterfaceParam = UFN_MAKE_INTERFACE_PARAM(dwInterface, dwAlternateSetting);

    // Notify PDD
    pContext->PddInfo.pfnIOControl(pContext->PddInfo.pvPddContext,
        MDD_IOCTL, IOCTL_UFN_PDD_SET_INTERFACE, 
        (PBYTE) &dwInterfaceParam, sizeof(dwInterfaceParam),
        NULL, 0, NULL);

    // Notify the client driver
    SendDeviceNotification(pContext->lpDeviceNotify, pContext->pvDeviceNotifyParameter,
        UFN_MSG_INTERFACE, dwInterfaceParam);

    response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;

EXIT:
    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) {
        response = CR_SUCCESS;
    }

    FUNCTION_LEAVE_MSG();

    return response;
}


// Handle a Clear Feature request.
static
CONTROL_RESPONSE
ProcessClearFeature(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    CONTROL_RESPONSE response = CR_UNHANDLED_REQUEST;
    
    ValidateContext(pContext);

    if ( (GET_REQUESET_RECIPIENT(udr.bmRequestType) == USB_REQUEST_FOR_ENDPOINT) && 
         (udr.wValue == USB_FEATURE_ENDPOINT_STALL) ) {
        BYTE bEndpoint = LOBYTE(udr.wIndex);
        DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Clear endpoint (0x%02x) halt request.\r\n"),
            pszFname, bEndpoint));

        PCPipeBase pPipe = FindPipe(pContext, bEndpoint);
        if (pPipe == NULL) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Asked to clear halt for invalid endpoint 0x%02x\r\n"),
                pszFname, bEndpoint));
            response = CR_STALL_DEFAULT_PIPE;
        }
        else {
            if (dwMsg == UFN_MSG_SETUP_PACKET) {
                pContext->PddInfo.pfnClearEndpointStall(pContext->PddInfo.pvPddContext,
                    pPipe->GetPhysicalEndpoint());
                response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
            }
        }
    }
    else if ( (GET_REQUESET_RECIPIENT(udr.bmRequestType) == USB_REQUEST_FOR_DEVICE)) {
        switch (udr.wValue) {
          case USB_FEATURE_REMOTE_WAKEUP:
            pContext->fRemoteWakeupEnabled = FALSE;
            response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
            break;
          case USB_FEATURE_B_HNP_ENABLE:
            if (pContext->hParentBusHandle && BusChildIoControl(pContext->hParentBusHandle,IOCTL_BUS_USBOTG_HNP_DISABLE,NULL,0)) {
                response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
            }
            break;
        }
    }
    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) {
        response = CR_SUCCESS;
    }
    
    FUNCTION_LEAVE_MSG();

    return response;
}


// Handle a Set Feature request.
static
CONTROL_RESPONSE
ProcessSetFeature(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    CONTROL_RESPONSE response = CR_UNHANDLED_REQUEST;
    
    ValidateContext(pContext);
    DEBUGMSG( ZONE_USB_EVENTS,(_T("%s:ProcessSetFeature udr.bmRequestType=0x%x, udr.wIndex=0x%x,udr.wValue=0x%x\r\n"),pszFname,udr.bmRequestType,udr.wIndex,udr.wValue));

    if ( (GET_REQUESET_RECIPIENT(udr.bmRequestType) == USB_REQUEST_FOR_ENDPOINT) && 
         (udr.wValue == USB_FEATURE_ENDPOINT_STALL) ) {
        BYTE bEndpoint = LOBYTE(udr.wIndex);
        DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Halt endpoint (0x%02x) request.\r\n"),
            pszFname, bEndpoint));

        PCPipeBase pPipe = FindPipe(pContext, bEndpoint);
        if (pPipe == NULL) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Asked to halt invalid endpoint 0x%02x\r\n"),
                pszFname, bEndpoint));
            response = CR_STALL_DEFAULT_PIPE;
        }
        else {
            if (dwMsg == UFN_MSG_SETUP_PACKET) {
                pContext->PddInfo.pfnStallEndpoint(pContext->PddInfo.pvPddContext, 
                    pPipe->GetPhysicalEndpoint());
                response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
            }
        }
    }
    else if ( (GET_REQUESET_RECIPIENT(udr.bmRequestType) == USB_REQUEST_FOR_DEVICE)) {
        switch (udr.wValue) {
          case USB_FEATURE_REMOTE_WAKEUP:
            pContext->fRemoteWakeupEnabled = TRUE;
            response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
            break;
          case USB_FEATURE_B_HNP_ENABLE:
            if (pContext->hParentBusHandle && BusChildIoControl(pContext->hParentBusHandle,IOCTL_BUS_USBOTG_HNP_ENABLE,NULL,0)) {
                response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
            }
            break;
          case USB_FEATURE_A_HNP_SUPPORT:
            response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
            break;
          case USB_FEATURE_A_ALT_HNP_SUPPORT: {
            RETAILMSG(1, (_T("%s ProcessSetFeature: SetFeature (a_alt_hnp_support) indicate this device does not connected to OTG port!!!\r\n"), pszFname));
            response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
            break;
          }
#ifdef FSL_MERGE
          case USB_FEATURE_TEST_MODE: {
            int iTestMode;
            iTestMode = (udr.wIndex >> 8) & 0xFF;
            if ( ((udr.wIndex & 0xFF) == 0) && (iTestMode <= USB_TEST_MODE_MAX ) ) {
                response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
                pContext->bEnterTestMode = TRUE;
                pContext->iTestMode = iTestMode;
                RETAILMSG( 1, (TEXT("Test mode requested: %d\r\n"), iTestMode) );
            }
            else
            {
                RETAILMSG( 1, (TEXT("Invalid Test mode requested: wIndex=0x%02x\r\n"), udr.wIndex) );
                DEBUGMSG(ZONE_ERROR, (_T("%s Invalid test mode request 0x%02x\r\n"),
                    pszFname, udr.wIndex ));
                response = CR_STALL_DEFAULT_PIPE;
            }
          }
#endif
        }
    }
    else if ( (GET_REQUESET_RECIPIENT(udr.bmRequestType) == USB_REQUEST_FOR_DEVICE) && 
              (udr.wValue == USB_FEATURE_REMOTE_WAKEUP) ) {
        pContext->fRemoteWakeupEnabled = TRUE;
        response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
    }


    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) {
        response = CR_SUCCESS;
    }
    DEBUGMSG( ZONE_USB_EVENTS,(_T("%s:ProcessSetFeature return: response = %d \r\n"),pszFname,response));
    FUNCTION_LEAVE_MSG();

    return response;
}

#ifdef FSL_MERGE
// Handle Sync Frame request.
static
CONTROL_RESPONSE
ProcessSyncFrame(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(udr);
    UNREFERENCED_PARAMETER(dwMsg);

    //Sync Frame is not supported.
    return CR_STALL_DEFAULT_PIPE;
}
#endif

// Handle a USB request.
static
CONTROL_RESPONSE
ProcessRequest(
    PUFN_MDD_CONTEXT pContext,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    ValidateContext(pContext);

    CONTROL_RESPONSE (*pfnProcess)(PUFN_MDD_CONTEXT, USB_DEVICE_REQUEST, DWORD) = NULL;

    // Process device-to-host requests
    if ( udr.bmRequestType == 
         (USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE) ) {
        // Process device requests
        switch (udr.bRequest) {
            case USB_REQUEST_GET_DESCRIPTOR:
                pfnProcess = &ProcessGetDescriptor;
                break;
            case USB_REQUEST_GET_CONFIGURATION:
                pfnProcess = &ProcessGetConfiguration;
                break;
            case USB_REQUEST_GET_STATUS:
                pfnProcess = &ProcessGetStatus;
                break;
        }
    }
    else if ( udr.bmRequestType == 
        (USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_STANDARD | USB_REQUEST_FOR_INTERFACE) ) {
        // Process interface requests
        switch (udr.bRequest) {
            case USB_REQUEST_GET_INTERFACE:
                pfnProcess = &ProcessGetInterface;
                break;
            case USB_REQUEST_GET_STATUS:
                pfnProcess = &ProcessGetStatus;
                break;
        }
    }
    else if ( udr.bmRequestType == 
        (USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_STANDARD | USB_REQUEST_FOR_ENDPOINT) ) {
        // Process interface requests
        switch (udr.bRequest) {
            case USB_REQUEST_GET_STATUS:
                pfnProcess = &ProcessGetStatus;
                break;
#ifdef FSL_MERGE
            case USB_REQUEST_SYNC_FRAME:
                pfnProcess = &ProcessSyncFrame;
                break;
#endif
        }
    }    
    // Process host-to-device requests
    else if ( udr.bmRequestType == 
        (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE) ) {
        switch (udr.bRequest) {            
            case USB_REQUEST_SET_ADDRESS:
                pfnProcess = &ProcessSetAddress;
                break;

            case USB_REQUEST_SET_CONFIGURATION:
                pfnProcess = &ProcessSetConfiguration;
                break;
            case USB_REQUEST_CLEAR_FEATURE:
                pfnProcess = &ProcessClearFeature;
                break;
                
            case USB_REQUEST_SET_FEATURE:
                pfnProcess = &ProcessSetFeature;
                break;
#ifdef FSL_MERGE
            case USB_REQUEST_SET_DESCRIPTOR:
                pfnProcess = &ProcessSetDescriptor;
                break;
#endif
        }
    }
    else if ( udr.bmRequestType == 
        (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_INTERFACE) ) {
        switch (udr.bRequest) {
            case USB_REQUEST_SET_INTERFACE:
                pfnProcess = &ProcessSetInterface;
                break;
        }
    }
    else if ( udr.bmRequestType == 
        (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_ENDPOINT) ) {
        switch (udr.bRequest) {
            case USB_REQUEST_CLEAR_FEATURE:
                pfnProcess = &ProcessClearFeature;
                break;
                
            case USB_REQUEST_SET_FEATURE:
                pfnProcess = &ProcessSetFeature;
                break;
        }
    }

    CONTROL_RESPONSE response;

    if (pfnProcess) {
        response = pfnProcess(pContext, udr, dwMsg);
    }
    else {
        response = CR_UNHANDLED_REQUEST;
    }
    
    FUNCTION_LEAVE_MSG();

    return response;
}


static
VOID
CloseAllPipesExceptEndpointZero(
    PUFN_MDD_CONTEXT pContext 
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    ValidateContext(pContext);
    PREFAST_DEBUGCHK(pContext->rgpPipes);

    // Don't close endpoint 0.
    for (DWORD dwPipe = 1; dwPipe < pContext->PddInfo.dwEndpointCount; ++dwPipe) {
        PCPipeBase pPipe = pContext->rgpPipes[dwPipe];
        if (pPipe->IsOpen()) {
            pPipe->Close();
        }
    }

    FUNCTION_LEAVE_MSG();
}


// Performs the check to see if we are ready for a notification.
static
inline
BOOL
IgnoreNotificationCheck(
    PUFN_MDD_CONTEXT pContext
    )
{
    return (!pContext->pUfnBus->IsClientActive() || pContext->fClientIsBeingAddedOrRemoved);
}


// Ignore this notification because we are not ready for it?
static
BOOL
IgnoreNotification(
    PUFN_MDD_CONTEXT pContext,
    DWORD dwMsg,
    DWORD dwParam
    )
{
    BOOL fIgnore = IgnoreNotificationCheck(pContext);
    if (fIgnore) {
        // We may not be ready to receive notifications--perhaps we
        // are in the process of loading the client driver. Verify
        // that we are ready and then check the other flags again.
        WaitForSingleObject(pContext->hevReadyForNotifications, READY_FOR_NOTIFICATIONS_TIMEOUT);
        fIgnore = IgnoreNotificationCheck(pContext);
    }

    if (fIgnore == FALSE) {
        // Ignore notifications when detached
        if ( (pContext->deviceState == DS_DETACHED) &&
              (dwMsg != UFN_MSG_BUS_EVENTS || dwParam != UFN_ATTACH) ) {
            fIgnore = TRUE;
        }
    }
    
    return fIgnore;
}


// Notification routine for the device.
BOOL 
WINAPI 
UfnMdd_Notify(
    PVOID pvNotifyParameter,
    DWORD dwMsg,
    DWORD dwParam
    )
{   
    SETFNAME();

    PUFN_MDD_CONTEXT pContext = (PUFN_MDD_CONTEXT) pvNotifyParameter;
    ValidateContext(pContext);

    FUNCTION_ENTER_MSG();

    BOOL fRet = FALSE;
    CONTROL_RESPONSE response = CR_SUCCESS;
    BOOL fSendNotificationToClient = TRUE;
    BOOL fEnteredCs = FALSE;

    if (dwMsg == UFN_MSG_TRANSFER_COMPLETE) {
        PSTransfer pPddTransfer = (PSTransfer) dwParam;
        PCUfnMddTransfer pTransfer = CUfnMddTransfer::ConvertFromPddTransfer(pPddTransfer);
        DWORD dwErr = CUfnMddTransfer::ReferenceTransferHandle(pTransfer);
        if (dwErr == ERROR_SUCCESS) {
            DEBUGCHK(pTransfer->IsInPdd());
            pTransfer->LeavingPdd();
        
            PCPipeBase pPipe = pTransfer->GetPipe();
            dwErr = pPipe->TransferComplete(pTransfer, TRUE);
            pTransfer->Release();
            fRet = TRUE;
        }
        else {
            DEBUGMSG(ZONE_ERROR, (_T("%s PDD passed a bad transfer\r\n"),
                pszFname));
        }

        goto EXIT;
    }

    if (IgnoreNotification(pContext, dwMsg, dwParam)) {
        // Are we receiving a notification before the client is fully initialized?
        // If so try adding a sleep to the PDD's IST before enabling interrupts.
        RETAILMSG(1, (_T("%s Ignoring notification 0x%08x - 0x%08x from PDD\r\n"),
            pszFname, dwMsg, dwParam));
        goto EXIT;
    }

    EnterCriticalSection(&pContext->csMddAccess);
    fEnteredCs = TRUE;

    PCPipeBase pPipeZero = pContext->rgpPipes[0];

    switch(dwMsg) {
        case UFN_MSG_BUS_EVENTS: {
            DEVICE_STATE dsNew;
            BOOL fChangeState = TRUE;
            
            switch (dwParam) {
            case UFN_DETACH:
                DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Detached\r\n"), pszFname));                
                if (pContext->deviceState == DS_DETACHED) {
                    DEBUGMSG(ZONE_WARNING, (_T("%s Received detach when already detached!\r\n"),
                        pszFname));
                    fChangeState = FALSE;
                }
                else {
                    dsNew = DS_DETACHED;
                }
                // Free resources associated with free transfers
                pContext->pFreeTransferList->Compact();
                break;

            case UFN_ATTACH:
                DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Attached\r\n"), pszFname));
                if (pContext->deviceState == DS_ATTACHED) {
                    DEBUGMSG(ZONE_WARNING, (_T("%s Received attach when already attached!\r\n"),
                        pszFname));
                    fChangeState = FALSE;
                }
                else {                
                    dsNew = DS_ATTACHED;
                    pContext->Speed = BS_UNKNOWN_SPEED;
                    pContext->fRemoteWakeupEnabled = FALSE;
                }
                break;
            
            case UFN_RESET:
                DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Root port reset\r\n"), pszFname));
                if (pContext->deviceState == DS_DETACHED) {
                    DEBUGMSG(ZONE_WARNING, (_T("%s Received reset when detached!\r\n"),
                        pszFname));
                    fChangeState = FALSE;
                }
                else {
                    dsNew = DS_DEFAULT;
                    pContext->fRemoteWakeupEnabled = FALSE;
                }
                pContext->pDescriptors->SetConfiguration(BS_FULL_SPEED, 0);
                pContext->pDescriptors->SetConfiguration(BS_HIGH_SPEED, 0);

#if 1
                // Our current implementation of OTG don't contain HNP support, to
                // simplify the process, we tempororily disable HNP related processing
#else
                if (pContext->hParentBusHandle != NULL ) {
                    BusChildIoControl(pContext->hParentBusHandle,IOCTL_BUS_USBOTG_HNP_DISABLE,NULL,0);
                }
#endif
                break;
                
            case UFN_SUSPEND:
                DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Suspended\r\n"), pszFname));
                DEBUGCHK(pContext->deviceState != DS_DETACHED);
                pContext->deviceStatePriorToSuspend = pContext->deviceState;
                dsNew = DS_SUSPENDED;
                // Need to call bus driver to indicate bus suspend.
                if (pContext->hParentBusHandle != NULL && pContext->deviceState == DS_CONFIGURED ) { 
                    // we only send out suspend to Bus driver after it configured.
                    // Otherwise, the OTG driver will confused.
                    BusChildIoControl(pContext->hParentBusHandle,IOCTL_BUS_USBOTG_SUSPEND,NULL,0);
                }
                break;

            case UFN_RESUME:
                DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Resumed\r\n"), pszFname));
                DEBUGCHK(pContext->deviceState != DS_DETACHED);
                dsNew = pContext->deviceStatePriorToSuspend;
                // Need to call bus driver to indicate bus suspend.
                if (pContext->hParentBusHandle != NULL ) {
                    BusChildIoControl(pContext->hParentBusHandle,IOCTL_BUS_USBOTG_RESUME,NULL,0);
                }
                break;

            default:
                DEBUGCHK(FALSE); // What message is this?
                dsNew = pContext->deviceState;
            }

            if (fChangeState) {
                ChangeDeviceState(pContext, dsNew);
                CloseAllPipesExceptEndpointZero(pContext);
                pPipeZero->AbortAllTransfers();
            }
            else {
                fSendNotificationToClient = FALSE;
            }

            break;
        }

        case UFN_MSG_BUS_SPEED:
            DEBUGCHK( (dwParam == BS_HIGH_SPEED) || (dwParam == BS_FULL_SPEED) );
            DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Attached to %s speed bus.\r\n"),
                pszFname, (dwParam == BS_HIGH_SPEED ? _T("high") : _T("full") )));
            
            DEBUGCHK(pContext->deviceState != DS_DETACHED);            
            DEBUGCHK((pContext->PddInfo.dwCapabilities & dwParam) != 0);
            pContext->Speed = (UFN_BUS_SPEED) dwParam;
            break;

        case UFN_MSG_SETUP_PACKET:
        case UFN_MSG_PREPROCESSED_SETUP_PACKET: {
            PUSB_DEVICE_REQUEST pUdr = (PUSB_DEVICE_REQUEST) dwParam;
            
            PREFAST_DEBUGCHK(pUdr);
            DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Received %s setup packet: %02x %02x %04x %04x %04x\r\n"),
                pszFname, (dwMsg == UFN_MSG_SETUP_PACKET ? _T("") : _T("preprocessed")),
                pUdr->bmRequestType, pUdr->bRequest, pUdr->wValue,
                pUdr->wIndex, pUdr->wLength));
            DEBUGCHK(pContext->deviceState != DS_DETACHED);
            
            response = ProcessRequest(pContext, *pUdr, dwMsg);

            if (dwMsg == UFN_MSG_SETUP_PACKET) {
                if (response == CR_UNHANDLED_REQUEST) {
                    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Unhandled setup packet. Passing it to client\r\n"),
                        pszFname));
                }
                else {                
                    dwMsg = UFN_MSG_PREPROCESSED_SETUP_PACKET;
                }
            }
            
            break;
        }

        case UFN_MSG_SET_ADDRESS: {
            // Some controllers automatically handle set address so they must
            // tell us when the device has been addressed.
            DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Set address (%u).\r\n"), 
                pszFname, dwParam));
            DEBUGCHK(pContext->deviceState != DS_DETACHED);
            DEBUGCHK(pContext->deviceState != DS_CONFIGURED);
            DEVICE_STATE ds;
            
            if (dwParam == 0) {
                ds = DS_DEFAULT;
            }
            else {
                ds = DS_ADDRESSED;
            }

            ChangeDeviceState(pContext, ds);
            break;
        }

        case UFN_MSG_CONFIGURED: {
            // Some controllers automatically handle set address so they must
            // tell us when the device has been addressed.
            DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Set configuration (%u).\r\n"), 
                pszFname, dwParam));
            DEBUGCHK(pContext->deviceState != DS_DETACHED);
            
            if (dwParam == 0) {
                ChangeDeviceState(pContext, DS_ADDRESSED);
            }
            else {
                DEBUGCHK(pContext->deviceState == DS_ADDRESSED);
                ChangeDeviceState(pContext, DS_CONFIGURED);
            }            
            break;
        }

        default:
            DEBUGCHK(FALSE); // What is this message?
    }

    if (fSendNotificationToClient) {
        fRet = SendDeviceNotification(pContext->lpDeviceNotify,    // Client Level Notify
            pContext->pvDeviceNotifyParameter, dwMsg, dwParam);
    }

    if (response == CR_STALL_DEFAULT_PIPE) {
        pPipeZero->Stall();
#ifdef USBCV_FIX
        RETAILMSG(DEBUG_LOG_USBCV, (L"we choose not to handshake after stall\r\n"));
#else
        pPipeZero->SendControlStatusHandshake();
#endif
    }
    else if (response == CR_SUCCESS_SEND_CONTROL_HANDSHAKE) {
        pPipeZero->SendControlStatusHandshake();
    }

EXIT:
    FUNCTION_LEAVE_MSG();
    if (fEnteredCs) {
        LeaveCriticalSection(&pContext->csMddAccess);
    }

#ifdef FSL_MERGE
    if ( pContext->bEnterTestMode ) {
        /* go directly into test mode now by ioctl on the PDD */
        Sleep(0);

        if ( pContext->PddInfo.pfnIOControl ) {
            DWORD dwTestMode, dwRetBytes;

            dwTestMode = (DWORD)pContext->iTestMode;

            if ( pContext->PddInfo.pfnIOControl( pContext->PddInfo.pvPddContext, 
                    MDD_IOCTL, IOCTL_UFN_SET_TEST_MODE,
                    (PBYTE)&dwTestMode, sizeof(dwTestMode),
                    NULL, 0, &dwRetBytes ) != ERROR_SUCCESS )
            {
                DEBUGMSG(ZONE_ERROR, (_T("%s: failed to SET_TEST_MODE %d on pdd\r\n"),
                    pszFname, (int)dwTestMode));
            }
        }

        pContext->bEnterTestMode = FALSE;
    }
#endif

    return fRet;
}

