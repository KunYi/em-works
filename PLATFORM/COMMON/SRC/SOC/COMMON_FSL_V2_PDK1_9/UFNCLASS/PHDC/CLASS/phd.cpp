//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        PHD.CPP

Abstract:

        USB Personal Health Care Protocol Class Level Code
        
--*/

//------------------------------------------------------------------------------
//
// Copyright (C) 2009-2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------ 

#pragma warning(push)
#pragma warning(disable: 4201)
#include <windows.h>
#pragma warning(pop)

#include <devload.h>
#include "usbfntypes.h"
#include "transporttypes.h"
#include "transport.h"
#include "phd_com_model.h"
#include "usb_phdc.h"               /* USB PHDC Class Header File */

#ifdef DEBUG

DBGPARAM dpCurSettings = {
    _T("usbphdfn"),
    {
        _T("Error"), _T("Warning"), _T("Init"), _T("Function"),
        _T("Comments"), _T(""), _T(""), _T(""),
        _T(""), _T(""), _T(""), _T(""), 
        _T(""), _T(""), _T(""), _T("")
    },
    0x5
};

#endif // DEBUG

/******************************************************************************
 * Macros
 ******************************************************************************/
#define dim(x) (sizeof(x)/sizeof((x)[0]))

#define PHD_INI_LOG 0
#define PHD_PROTOCOL_LOG 0
#define PHD_APP_LOG 0

/******************************************************************************
 * Typedefs
 ******************************************************************************/
enum CONTROL_RESPONSE {
    CR_SUCCESS = 0,
    CR_SUCCESS_SEND_CONTROL_HANDSHAKE, // Use if no data stage
    CR_STALL_DEFAULT_PIPE,
    CR_UNHANDLED_REQUEST,
};

/******************************************************************************
 * Local Global Variables
 ******************************************************************************/
static BOOL g_phd_metadata;
static TCHAR g_szActiveKey[MAX_PATH];
static CRITICAL_SECTION g_cs;
static HANDLE g_htTransfers;
static LPUSB_CLASS_CALLBACK g_phdc_class_callback = NULL;
static LPAPP_NOTIFY g_app_callback = NULL;

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
static bool g_pipesstalled = false;
static bool g_outpipestalled = false;
#endif


// USB function state
static BOOL g_fDeviceRegistered = FALSE;
static BOOL g_fDeviceConfigured = FALSE;

// Bus Speed  - Used for selecting between Descriptors
static UFN_BUS_SPEED g_SpeedSupported = BS_HIGH_SPEED;

static UFN_CLIENT_REG_INFO g_RegInfo = { sizeof(UFN_CLIENT_REG_INFO) };

static LPCWSTR g_rgpszStrings0409[] = {
    g_RegInfo.szVendor, g_RegInfo.szProduct , g_RegInfo.szSerialNumber
};

static UFN_STRING_SET g_rgStringSets[] = {
    0x0409, g_rgpszStrings0409, dim(g_rgpszStrings0409)
};

// USB function object
static UFN_FUNCTIONS g_ufnFuncs;
static UFN_HANDLE g_hDevice = NULL;

static PCUFN_FUNCTIONS g_pUfnFuncs = &g_ufnFuncs;

static uint_16 g_phd_ep_has_data = 0;

// default (control) pipe state data
static UFN_PIPE g_hDefaultPipe;
static USB_PIPE_STATE g_psDefaultPipeState;

// bulk out pipe state data
static UFN_PIPE g_hBOPipe;
static USB_PIPE_STATE g_psBOPipeState;

// bulk in pipe state data
static UFN_PIPE g_hBIPipe;
static USB_PIPE_STATE g_psBIPipeState;

#if USB_INTERRUPT_EP_SUPPORTED
// interrupt in pipe state data
static UFN_PIPE g_hIIPipe;
static USB_PIPE_STATE g_psIIPipeState;
#endif

static PIPE_TRANSFER g_rgPipeTransfers[] = 
#if USB_INTERRUPT_EP_SUPPORTED
    { { &g_hDefaultPipe }, { &g_hBIPipe }, { &g_hBOPipe }, {&g_hIIPipe} };
#else
    { { &g_hDefaultPipe }, { &g_hBIPipe }, { &g_hBOPipe } };
#endif

static BYTE g_rgbPhdRxBuffer[MAX_PHD_RX_LENGTH];

static DWORD g_b_phd_rx_setup = 0;     // prevent RX endpoint is primed twice

/******************************************************************************
 * Functions
 ******************************************************************************/
UFN_HANDLE* getDeviceHandle(void)
{
    return &g_hDevice;
}

PUFN_FUNCTIONS getFuncsHandle(void)
{
    return &g_ufnFuncs;
}

DWORD
static
WINAPI 
DefaultTransferComplete(
    PVOID pvNotifyParameter
    )
{   
    HANDLE hev = (HANDLE) pvNotifyParameter;

    DEBUGMSG(ZONE_COMMENT, (_T("DefaultTransferComplete setting event\r\n")));

    SetEvent(hev);

    return ERROR_SUCCESS;
}


// Prepare to receive data from the host.
VOID
PHD_SetupRx(
    PPIPE_TRANSFER pPipeTransfer,
    PBYTE pbData,
    DWORD cbData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_SetupRx"));
#endif
    if (g_b_phd_rx_setup)
    {
        RETAILMSG(PHD_PROTOCOL_LOG, (L"Already has a RX prime, ignore\r\n"));   
    }
    else
    {
        InterlockedExchange((LONG*)&g_b_phd_rx_setup, TRUE);
        DEBUGCHK(pbData != NULL);

        RETAILMSG(PHD_PROTOCOL_LOG, (L"Prepare to Recieve Data\r\n"));
        g_pUfnFuncs->lpIssueTransfer(g_hDevice, *pPipeTransfer->phPipe, 
            &DefaultTransferComplete, pPipeTransfer->hev, USB_OUT_TRANSFER, cbData,
            pbData, 0, NULL, &pPipeTransfer->hTransfer);
    }
}


// Prepare to send data to the host.
VOID
PHD_SetupTx(
    PPIPE_TRANSFER pPipeTransfer,
    PBYTE pbData,
    DWORD cbData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_SetupTx"));
#endif

    DEBUGCHK(pbData != NULL);

    // RETAILMSG(1, (L"IssueTransfer Tx\r\n"));
    RETAILMSG(PHD_PROTOCOL_LOG, (L"Prepare to Transmit Data\r\n"));
    g_pUfnFuncs->lpIssueTransfer(g_hDevice, *pPipeTransfer->phPipe, 
        &DefaultTransferComplete, pPipeTransfer->hev, USB_IN_TRANSFER, 
        cbData, pbData, 0, NULL, &pPipeTransfer->hTransfer);                // UfnMdd_IssueTransfer
}

/**************************************************************************//*!
 *
 * @name  USB_Class_PHDC_Send_Data
 *
 * @brief This fucntion is used by Application to send data through PHDC class
 *
 * @param meta_data             : Packet is meta data or not
 * @param num_tfr               : Number of transfers following meta data packet
 * @param qos                   : Qos of the transfer
 * @param app_buff              : Buffer to send
 * @param size                  : Length of the transfer
 *
 * @return status
 *         USB_OK           : When Successfull
 *         Others           : Errors
 ******************************************************************************
 * This fucntion is used by Application to send data through PHDC class
 *****************************************************************************/
uint_8 USB_Class_PHDC_Send_Data (
    boolean meta_data,      /* [IN] Packet is meta data or not */
    uint_8 num_tfr,         /* [IN] Number of transfers
                                    following meta data packet */
    uint_8 qos,             /* [IN] Qos of the transfer */
    uint_8_ptr app_buff,    /* [IN] Buffer to send */
    USB_PACKET_SIZE size    /* [IN] Length of the transfer */
)
{
    UNREFERENCED_PARAMETER(num_tfr);
    UNREFERENCED_PARAMETER(qos);

    if (meta_data)
    {
        // ERIC : TO-DO
        RETAILMSG(1, (L"To do metadata processing\r\n"));
    }

    // RETAILMSG(1, (L"Will Issue IN transfer of size %d\r\n", size));

    g_phd_ep_has_data |= 1 << (BULK_IN_ENDPOINT_ADDRESS & 0xf);
    PHD_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], app_buff, size);  
    return 0;
}

/**************************************************************************//*!
 *
 * @name  USB_Class_PHDC_Recieve_Data
 *
 * @brief This fucntion is used by Application to send data through PHDC class
 *
 * @return status
 *         USB_OK           : When Successfull
 *         Others           : Errors
 ******************************************************************************
 * This fucntion is used by Application to send data through PHDC class
 *****************************************************************************/
uint_8 USB_Class_PHDC_Recieve_Data (
)
{
    PHD_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], &g_rgbPhdRxBuffer[0], MAX_PHD_RX_LENGTH);
    return 0;
}

// Class
// Read a configuration value from the registry.
// Query "pszValue" in registry and store the value in pdwResult
static
BOOL
PHD_ReadConfigurationValue(
    HKEY    hClientDriverKey,
    LPCTSTR pszValue,
    PDWORD  pdwResult,
    BOOL    fMustSucceed
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_ReadConfigurationValue"));
#endif
    
    
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbData = sizeof(DWORD);
    DWORD dwType;
    BOOL  fResult = TRUE;

    dwError = RegQueryValueEx(hClientDriverKey, pszValue, NULL, 
        &dwType, (PBYTE) pdwResult, &cbData);
    if ( (dwError != ERROR_SUCCESS) || (dwType != REG_DWORD) ) {
        if (fMustSucceed) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Failed to read %s. Error %d\r\n"), 
                pszFname, pszValue, dwError));
        }
        fResult = FALSE;
    }
    else {
        DEBUGMSG(ZONE_INIT, (_T("%s %s = 0x%x\r\n"), 
            pszFname, pszValue, *pdwResult));
    }

    return fResult;
}


// Configure the function controller based on registry settings.  This
// routine is not responsible for validating the data supplied by the registry.
static
BOOL
PHD_Configure(
    LPCTSTR pszActiveKey,
    HKEY hClientDriverKey
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_Configure"));
#endif
    
    UNREFERENCED_PARAMETER(hClientDriverKey);
    
    DWORD dwRet = UfnGetRegistryInfo(pszActiveKey, &g_RegInfo);
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }
    
    // Adjust device descriptors
    g_HighSpeedDeviceDesc.idVendor = (USHORT) g_RegInfo.idVendor;
    g_HighSpeedDeviceDesc.idProduct = (USHORT) g_RegInfo.idProduct;
    g_HighSpeedDeviceDesc.bcdDevice = (USHORT) g_RegInfo.bcdDevice;
    
    g_FullSpeedDeviceDesc.idVendor = g_HighSpeedDeviceDesc.idVendor;
    g_FullSpeedDeviceDesc.idProduct = g_HighSpeedDeviceDesc.idProduct;
    g_FullSpeedDeviceDesc.bcdDevice = g_HighSpeedDeviceDesc.bcdDevice;

    dwRet = ERROR_SUCCESS;

EXIT:
    return dwRet;
}


// Reset the transfer state of a pipe.
static
VOID
PHD_ResetPipeState(
    PUSB_PIPE_STATE pPipeState
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_ResetPipeState"));
#endif
    pPipeState->fSendingLess = FALSE;
}

// Return the configuration tree structure for the given speed.
static
PUFN_ENDPOINT
GetEndpointDescriptor(
    UFN_BUS_SPEED    Speed
    )
{
    DEBUGCHK( (Speed == BS_FULL_SPEED) || (Speed == BS_HIGH_SPEED) );
    PUFN_ENDPOINT pEndpoint;

    if (Speed == BS_HIGH_SPEED) {
        pEndpoint = &g_HighSpeedEndpoints[0];
    }
    else {
        pEndpoint = &g_FullSpeedEndpoints[0];
    }

    return pEndpoint;
}

// Process a USB Class Request.  Call Request-specific handler.
static
CONTROL_RESPONSE
PHD_HandleClassRequest(
    USB_DEVICE_REQUEST udr
    )
{
    SETFNAME(_T("PHD_HandleClassRequest"));

    CONTROL_RESPONSE response = CR_STALL_DEFAULT_PIPE;
           
    if (udr.bmRequestType == (USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE | USB_REQUEST_HOST_TO_DEVICE) ) 
    {
        if (udr.bRequest == PHD_SET_FEATURE_REQUEST)
        {
#if USB_METADATA_SUPPORTED
            RETAILMSG(PHD_PROTOCOL_LOG, (L"PHD Class Set Feature\r\n"));
            g_phd_metadata = TRUE;
            response = CR_SUCCESS;
#else
            RETAILMSG(PHD_PROTOCOL_LOG, (L"Set Meta data Feature not supported\r\n"));
#endif
        }
        else if (udr.bRequest == PHD_CLEAR_FEATURE_REQUEST)
        {
#if USB_METADATA_SUPPORTED
            RETAILMSG(PHD_PROTOCOL_LOG, (L"PHD Class Clear Feature\r\n"));
            g_phd_metadata = FALSE;
            response = CR_SUCCESS;
#else
            RETAILMSG(PHD_PROTOCOL_LOG, (L"Clear Meta data Feature not supported\r\n"));
#endif
        }
        else 
        {
            ERRORMSG(1, (_T("%s Unrecognized BOT class bRequest -> 0x%x\r\n"), pszFname, udr.bmRequestType));
        }
    }
    else if (udr.bmRequestType == (USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE | USB_REQUEST_DEVICE_TO_HOST) ) 
    {
        if (udr.bRequest == PHD_GET_STATUS_REQUEST)
        {
            if ((udr.wValue == 0) && (udr.wIndex == 0))
            {
                PHD_SetupTx(&g_rgPipeTransfers[CONTROL_TRANSFER], (PBYTE)&g_phd_ep_has_data, sizeof(g_phd_ep_has_data));
                response = CR_SUCCESS;
            }
            else
            {
                RETAILMSG(PHD_PROTOCOL_LOG, (L"[PHDC] Invalid parameter in GetStatus\r\n"));
            }
        }
        else 
        {
            ERRORMSG(1, (_T("%s Unrecognized class bRequest -> 0x%x\r\n"), pszFname, udr.bmRequestType));
        }     
    }
    else 
    {
        ERRORMSG(1, (_T("%s Unrecognized class bRequest -> 0x%x\r\n"), pszFname, udr.bmRequestType));
    }
    
    return response;
}


// Process a USB Standard Request.  Call Request-specific handler.
static
VOID
PHD_HandleRequest(
    DWORD dwMsg,
    USB_DEVICE_REQUEST udr
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_HandleRequest"));
#endif

    CONTROL_RESPONSE response;

    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) 
    {
        response = CR_SUCCESS; // Don't respond since it was already handled.
        
        if ( udr.bmRequestType == (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_ENDPOINT) ) 
        {
            switch (udr.bRequest) {
                case USB_REQUEST_CLEAR_FEATURE:
                    break;
                default:
                    break;
            }
        }

        else if (udr.bmRequestType == (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE) ) 
        {
            if (udr.bRequest == USB_REQUEST_SET_CONFIGURATION) 
            {
            }
        }
    }
    else 
    {
        DEBUGCHK(dwMsg == UFN_MSG_SETUP_PACKET);
        response = CR_STALL_DEFAULT_PIPE;

        if (udr.bmRequestType & USB_REQUEST_CLASS) 
        {
            DEBUGMSG(ZONE_COMMENT, (_T("%s Class request\r\n"), pszFname));
            response = PHD_HandleClassRequest(udr);
        }
    }

    if (response == CR_STALL_DEFAULT_PIPE) {
        g_pUfnFuncs->lpStallPipe(g_hDevice, g_hDefaultPipe);
        g_pUfnFuncs->lpSendControlStatusHandshake(g_hDevice);
    }
    else if (response == CR_SUCCESS_SEND_CONTROL_HANDSHAKE) 
    {
        g_pUfnFuncs->lpSendControlStatusHandshake(g_hDevice);
    }
}

static
VOID
ProcessBOPipeTransfer(
    DWORD cbTransferred
    )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(cbTransferred);

    // RETAILMSG(1, (L"To Process BO Transfer\r\n"));
    InterlockedExchange((LONG*)&g_b_phd_rx_setup, FALSE);
    g_phdc_class_callback(USB_APP_DATA_RECEIVED, 0, (void*)&g_rgbPhdRxBuffer[0]);
}


static
VOID
ProcessBIPipeTransfer(
    )
{
    // RETAILMSG(1, (L"To Process BI Transfer\r\n"));
    DWORD epNum = BULK_IN_ENDPOINT_ADDRESS & 0xf;
    g_phdc_class_callback(USB_APP_SEND_COMPLETE, epNum, NULL);
    // Update ep data status
    g_phd_ep_has_data &= ~(1 << epNum);

}

static
VOID
ProcessIIPipeTransfer(
    )
{
    // N.A.
}

// Open the pipes associated with the default interface.
static
BOOL
PHD_OpenInterface(
    )
{
    SETFNAME(_T("PHD_OpenInterface"));
    
    
    BOOL fResult = FALSE;
    PUFN_ENDPOINT pEndpoint= GetEndpointDescriptor(g_SpeedSupported);

    // Open the pipes of the associated interface.

    DEBUGCHK(g_hBOPipe == NULL);
    DEBUGCHK(g_hBIPipe == NULL);
#if USB_INTERRUPT_EP_SUPPORTED
    DEBUGCHK(g_hIIPipe == NULL);
#endif
    
    RETAILMSG(PHD_INI_LOG, (L"Open BI Pipe\r\n"));
    DWORD dwRet = g_pUfnFuncs->lpOpenPipe(g_hDevice, 
        pEndpoint[0].Descriptor.bEndpointAddress,
        &g_hBIPipe);                                    // UfnMdd_OpenPipe
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, (_T("%s Failed to open bulk in pipe\r\n"),
            pszFname));
        goto EXIT;
    }

    RETAILMSG(PHD_INI_LOG, (L"Open BO Pipe\r\n"));
    dwRet = g_pUfnFuncs->lpOpenPipe(g_hDevice, 
        pEndpoint[1].Descriptor.bEndpointAddress,
        &g_hBOPipe);
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, (_T("%s Failed to open bulk in pipe\r\n"),
            pszFname));
        goto EXIT;
    }

#if USB_INTERRUPT_EP_SUPPORTED
    RETAILMSG(PHD_INI_LOG, (L"Open II Pipe\r\n"));
    dwRet = g_pUfnFuncs->lpOpenPipe(g_hDevice, 
        pEndpoint[2].Descriptor.bEndpointAddress,
        &g_hIIPipe);
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, (_T("%s Failed to interrupt bulk in pipe\r\n"),
            pszFname));
        RETAILMSG(1, (L"\t[PHDC] Interrupt EP Open Failed\r\n"));
        goto EXIT;
    }
#endif

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
    g_pUfnFuncs->lpClearPipeStall(g_hDevice, g_hBIPipe);
    g_pUfnFuncs->lpClearPipeStall(g_hDevice, g_hBOPipe);
#if USB_INTERRUPT_EP_SUPPORTED
    g_pUfnFuncs->lpClearPipeStall(g_hDevice, g_hIIPipe);
#endif
    g_pipesstalled = FALSE;
    g_outpipestalled = FALSE;
#endif

    fResult = TRUE;
    
EXIT:
    return fResult;
}


// Close the store
DWORD
PHD_Close(
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_Close"));
#endif
    
    // if (g_pbDataBuffer) LocalFree(g_pbDataBuffer);
    
    return ERROR_SUCCESS;
}


// Class
// Process a device event.
static
BOOL
WINAPI
PHD_DeviceNotify(
    PVOID   pvNotifyParameter,
    DWORD   dwMsg,
    DWORD   dwParam
    ) 
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pvNotifyParameter);

    EnterCriticalSection(&g_cs);
    
    SETFNAME(_T("PHD_DeviceNotify"));
    

    switch(dwMsg) {
        case UFN_MSG_BUS_EVENTS: {
            // Ensure device is in running state
            DEBUGCHK(g_hDefaultPipe);

            switch(dwParam) {
                case UFN_DETACH:
                    RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify DETACH\r\n"));
                    // Reset device

                    PHD_ResetPipeState(&g_psDefaultPipeState);
                    PHD_ResetPipeState(&g_psBOPipeState);
                    PHD_ResetPipeState(&g_psBIPipeState);
#if USB_INTERRUPT_EP_SUPPORTED
                    PHD_ResetPipeState(&g_psIIPipeState);
#endif
                    InterlockedExchange((LONG*)&g_b_phd_rx_setup, FALSE);
                    g_phd_ep_has_data = 0;

                    if (g_hBOPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                        g_hBOPipe = NULL;
                    }

                    if (g_hBIPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                        g_hBIPipe = NULL;
                    }

#if 0
                    ChangeAppState(APP_PHD_UNINITIALISED);
                    ChangePhdcState(PHD_AG_STATE_DISCONNECTED);
#else
                    g_app_callback(pvNotifyParameter, dwMsg, dwParam);
#endif
                    break;

                case UFN_ATTACH: {
                    // Open store if not already open
                    RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify ATTACH\r\n"));

                    // Reset device
                    PHD_ResetPipeState(&g_psDefaultPipeState);
                    PHD_ResetPipeState(&g_psBOPipeState);
                    PHD_ResetPipeState(&g_psBIPipeState);
#if USB_INTERRUPT_EP_SUPPORTED
                    PHD_ResetPipeState(&g_psIIPipeState);
#endif

                    if (g_hBOPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                        g_hBOPipe = NULL;
                    }

                    if (g_hBIPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                        g_hBIPipe = NULL;
                    }

                    break;
                }
            
                case UFN_RESET:
                    // Reset device
                    RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify RESET\r\n"));
#if 0
                    ChangeAppState(APP_PHD_UNINITIALISED);
                    ChangePhdcState(PHD_AG_STATE_DISCONNECTED);
#else
                    g_app_callback(pvNotifyParameter, dwMsg, dwParam);
#endif
                    InterlockedExchange((LONG*)&g_b_phd_rx_setup, FALSE);
                    g_phd_ep_has_data = 0;
                    PHD_ResetPipeState(&g_psDefaultPipeState);
                    PHD_ResetPipeState(&g_psBOPipeState);
                    PHD_ResetPipeState(&g_psBIPipeState);
#if USB_INTERRUPT_EP_SUPPORTED
                    PHD_ResetPipeState(&g_psIIPipeState);
#endif

                    if (g_hBOPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                        g_hBOPipe = NULL;
                    }

                    if (g_hBIPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                        g_hBIPipe = NULL;
                    }
                    
#if USB_INTERRUPT_EP_SUPPORTED
                    if (g_hIIPipe)
                    {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hIIPipe);
                        g_hIIPipe = NULL;
                    }
#endif
                    break;

                default:
                    break;
            }

            break;
        }

        case UFN_MSG_BUS_SPEED:
            RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify SPEED %s\r\n", (dwParam == BS_HIGH_SPEED) ? L"HS" : L"FS"));
            g_SpeedSupported = (UFN_BUS_SPEED) dwParam;
            break;

        case UFN_MSG_SETUP_PACKET:
        case UFN_MSG_PREPROCESSED_SETUP_PACKET: {
            RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify [PREPROCESSED] SETUP PACKET\r\n"));
            PUSB_DEVICE_REQUEST pudr = (PUSB_DEVICE_REQUEST) dwParam;
            PHD_HandleRequest(dwMsg, *pudr);
            break;
        }

        case UFN_MSG_CONFIGURED:
            RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] PHD Notify CONFIGURED\r\n"));
            if (dwParam == 0) {
                // Reset device
                PHD_ResetPipeState(&g_psDefaultPipeState);
                PHD_ResetPipeState(&g_psBOPipeState);
                PHD_ResetPipeState(&g_psBIPipeState);
#if USB_INTERRUPT_EP_SUPPORTED
                PHD_ResetPipeState(&g_psIIPipeState);
#endif

                if (g_hBOPipe) {
                    g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                    g_hBOPipe = NULL;
                }

                if (g_hBIPipe) {
                    g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                    g_hBIPipe = NULL;
                }
                
#if USB_INTERRUPT_EP_SUPPORTED
                if (g_hIIPipe) 
                {
                    g_pUfnFuncs->lpClosePipe(g_hDevice, g_hIIPipe);
                    g_hIIPipe = NULL;
                }
#endif
            }
            else {
                RETAILMSG(PHD_INI_LOG, (L"\t[PHDC] SET CONFIGURATION DONE\r\n"));
                DEBUGCHK(g_hBIPipe == NULL);
                PHD_OpenInterface();
            }
            break;
        default:
            break;
    }

    g_phdc_class_callback(dwMsg, dwParam, NULL);

    LeaveCriticalSection(&g_cs);

    return TRUE;
}

// Class
DWORD
WINAPI
PHD_TransferThread(
    LPVOID lpParameter
    )
{    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParameter);

#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("PHD_TransferThread"));
#endif

    HANDLE rghevWaits[dim(g_rgPipeTransfers)];

    for (DWORD dwIdx = 0; dwIdx < dim(g_rgPipeTransfers); ++dwIdx) {
        DEBUGCHK(g_rgPipeTransfers[dwIdx].hev);
        rghevWaits[dwIdx] = g_rgPipeTransfers[dwIdx].hev;
    }

    for (;;) 
    {
        DWORD dwWait = WaitForMultipleObjects(dim(rghevWaits), rghevWaits, FALSE, INFINITE);

        DEBUGMSG(ZONE_COMMENT, (_T("%s Transfer %u\r\n"), pszFname, dwWait));

        DWORD dwIdx = dwWait - WAIT_OBJECT_0;
            
        if (dwIdx >= dim(rghevWaits)) {
            break;
        }
        
        EnterCriticalSection(&g_cs);
        
        PPIPE_TRANSFER pPipeTransfer;
        pPipeTransfer = &g_rgPipeTransfers[dwIdx];
        DEBUGCHK(pPipeTransfer->hev);
        DEBUGCHK(pPipeTransfer->hTransfer);

        DWORD dwUsbError;
        DWORD cbTransferred;
        DWORD dwErr;
        
        dwErr = g_pUfnFuncs->lpGetTransferStatus(g_hDevice, pPipeTransfer->hTransfer, &cbTransferred, &dwUsbError);
        DEBUGCHK(dwErr == ERROR_SUCCESS);

        DEBUGMSG(ZONE_COMMENT, (_T("%s %u bytes transferred\r\n"), pszFname, cbTransferred));

        RETAILMSG(PHD_PROTOCOL_LOG, (L"[ep %d] %d bytes transferred\r\n", dwIdx, cbTransferred));

        dwErr = g_pUfnFuncs->lpCloseTransfer(g_hDevice, pPipeTransfer->hTransfer);
        DEBUGCHK(dwErr == ERROR_SUCCESS);
        pPipeTransfer->hTransfer = NULL;

        if (dwUsbError != UFN_NO_ERROR) {
            DEBUGCHK(dwUsbError == UFN_CANCELED_ERROR);
            goto CONTINUE;
        }

        // TODO: Don't do these if transfer error
        if (dwIdx == CONTROL_TRANSFER) {
            g_pUfnFuncs->lpSendControlStatusHandshake(g_hDevice);
        }
        else if (dwIdx == OUT_TRANSFER) {
            ProcessBOPipeTransfer(cbTransferred);
        }
        else if (dwIdx == IN_TRANSFER) {
            ProcessBIPipeTransfer();
        }
        else {
            DEBUGCHK(dwIdx == IN_TRANSFER);
            ProcessIIPipeTransfer();
        }

CONTINUE:
        LeaveCriticalSection(&g_cs);
    }

    return 0;
}


// Initialize the PHDC Transport layer.
DWORD
PHD_InternalInit(
    LPCTSTR pszActiveKey,
    LPUSB_CLASS_CALLBACK phdc_class_callback,
    LPAPP_NOTIFY app_callback
    )
{
    SETFNAME(_T("PHD_InternalInit"));
    
    HKEY    hClientDriverKey = NULL;
    DWORD   dwRet;
    HRESULT hr = 0;

    PREFAST_DEBUGCHK(pszActiveKey);

    g_phdc_class_callback = phdc_class_callback;
    g_app_callback = app_callback;

    RETAILMSG(PHD_PROTOCOL_LOG, (L"\t[PHDCPHDC] PHD_InternalInit\r\n"));
    InitializeCriticalSection(&g_cs);

    hr = StringCchCopy(g_szActiveKey, dim(g_szActiveKey), pszActiveKey);
    if (FAILED(hr)) {
        dwRet = GetLastError();
        ERRORMSG(1, (_T("%s Failed to copy device key. Error = %d\r\n"),
            pszFname, dwRet));
        goto EXIT;
    }

    hClientDriverKey = OpenDeviceKey(g_szActiveKey);
    if (hClientDriverKey == NULL) {
        dwRet = GetLastError();
        ERRORMSG(1, (_T("%s Failed to open device key. Error = %d\r\n"),
            pszFname, dwRet));
        goto EXIT;
    }

    // Configure function controller
    dwRet = PHD_Configure(pszActiveKey, hClientDriverKey);
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, 
            (_T("%s Failed to configure function controller from registry\r\n"),
            pszFname));
        goto EXIT;
    }

    // initially has no data at any channel
    g_phd_ep_has_data = 0;

    // initially disable meta data feature
    g_phd_metadata = FALSE;
    
    //    g_pUfnFuncs = pUfnFuncs;

    // Register descriptor tree with device controller
    RETAILMSG(PHD_PROTOCOL_LOG, (L"\t[PHDC] PHD Register Descriptors\r\n"));
    // RETAILMSG(1, (L"\t[PHDC] g_pUfnFuncs %x\r\n", g_pUfnFuncs));
    // g_pUfnFuncs = &g_ufnFuncs;
    // RETAILMSG(1, (L"\t[PHDC] g_pUfnFuncs %x\r\n", g_pUfnFuncs));
    dwRet = g_pUfnFuncs->lpRegisterDevice(g_hDevice,                // UfnMdd_RegisterDevice
        &g_HighSpeedDeviceDesc, &g_HighSpeedConfig, 
        &g_FullSpeedDeviceDesc, &g_FullSpeedConfig, 
        g_rgStringSets, dim(g_rgStringSets));
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, (_T("%s Descriptor registration failed\r\n"), 
            pszFname));        
        goto EXIT;
    }

    // Mark registration
    g_fDeviceRegistered = TRUE;

    // Create pipe events
    DWORD dwTransfer;
    for (dwTransfer = 0; dwTransfer < dim(g_rgPipeTransfers); ++dwTransfer) {
        g_rgPipeTransfers[dwTransfer].hev = CreateEvent(0, FALSE, FALSE, NULL);
        if (g_rgPipeTransfers[dwTransfer].hev == NULL) {
            dwRet = GetLastError();
            ERRORMSG(1, (_T("%s Error creating event. Error = %d\r\n"), 
                pszFname, dwRet));
            goto EXIT;
        }
    }

    // Read transfer thread priority from registry
    DWORD dwTransferThreadPriority;
    if (PHD_ReadConfigurationValue(hClientDriverKey, 
        UMS_REG_TRANSFER_THREAD_PRIORITY_VAL, &dwTransferThreadPriority, FALSE))
    {
        DEBUGMSG(1,
            (_T("%s PHD transfer thread priority = %u (from registry)\r\n"), 
            pszFname, dwTransferThreadPriority));
    }
    else {
        dwTransferThreadPriority = DEFAULT_TRANSFER_THREAD_PRIORITY;
        DEBUGMSG(1, (_T("%s PHD transfer thread priority = %u\r\n"), 
            pszFname, dwTransferThreadPriority));
    }

    // Create transfer thread
    RETAILMSG(PHD_PROTOCOL_LOG, (L"\t[PHDC] Create PHD Thread\r\n"));
    g_htTransfers = CreateThread(NULL, 0, PHD_TransferThread, NULL, 0, NULL);
    if (g_htTransfers == NULL) {
        ERRORMSG(1, (_T("%s Transfer thread creation failed\r\n"), pszFname));
        dwRet = GetLastError();
        goto EXIT;
    }

    // Set transfer thread priority
    if (!CeSetThreadPriority(g_htTransfers, dwTransferThreadPriority)) {
        dwRet = GetLastError();
        DEBUGMSG(1,
            (_T("%s Failed to set thread priority, last error = %u; exiting\r\n"), 
            pszFname, dwRet));
        goto EXIT;
    }

    // Start the device controller
    dwRet = g_pUfnFuncs->lpStart(g_hDevice, PHD_DeviceNotify, NULL,   // UfnMdd_Start
        &g_hDefaultPipe);
    if (dwRet != ERROR_SUCCESS) {
         ERRORMSG(1, (_T("%s Device controller failed to start\r\n"),
            pszFname));
        goto EXIT;
    }
    
    dwRet = ERROR_SUCCESS;    

EXIT:

    if (hClientDriverKey) {
        RegCloseKey(hClientDriverKey);
    }

    if (dwRet != ERROR_SUCCESS) {
        if (g_htTransfers) {
            CloseHandle(g_htTransfers);
        }

        if (g_fDeviceRegistered) {
            g_pUfnFuncs->lpDeregisterDevice(g_hDevice);
        }
    }
    
    return dwRet;
}
