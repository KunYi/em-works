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

        BOT.CPP

Abstract:

        USB Mass Storage Function Bulk-Only Transport.
        
--*/

//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2007, Freescale Semiconductor, Inc. All Rights Reserved.
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
#include "proxy.h"
#include "transporttypes.h"
#include "transport.h"


#if 0
#ifdef ZONE_INIT
#undef ZONE_INIT
#endif
#define ZONE_INIT 1

#ifdef ZONE_COMMENT
#undef ZONE_COMMENT
#endif
#define ZONE_COMMENT 1

#undef DEBUGMSG
#define DEBUGMSG(x,y) NKDbgPrintfW y
#undef RETAILMSG
#define RETAILMSG(x,y) NKDbgPrintfW y
#endif


#define USBCV_MSC_LOG 0

#ifdef DEBUG

DBGPARAM dpCurSettings = {
    _T("usbmsfn"),
    {
        _T("Error"), _T("Warning"), _T("Init"), _T("Function"),
        _T("Comments"), _T(""), _T(""), _T(""),
        _T(""), _T(""), _T(""), _T(""), 
        _T(""), _T(""), _T(""), _T("")
    },
    0x5
};

#endif // DEBUG

extern void STORE_ReturnReadBuffer(PVOID pbData, DWORD dwTransferLength);
extern PVOID STORE_GetWriteBuffer(DWORD dwLength);

// DLL entry point
extern "C"
BOOL
WINAPI
DllEntry(
    HINSTANCE hinstDll,
    DWORD     dwReason,
    LPVOID    lpReserved
    )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpReserved);

#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("DllEntry"));
#endif

    if (dwReason == DLL_PROCESS_ATTACH) {
        DEBUGREGISTER(hinstDll);
        DEBUGMSG(ZONE_INIT, (_T("%s Attached\r\n"), pszFname));
        DisableThreadLibraryCalls((HMODULE) hinstDll);
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        DEBUGMSG(ZONE_INIT, (_T("%s Detached\r\n"), pszFname));       
    }

    return TRUE;
}


#define dim(x) (sizeof(x)/sizeof((x)[0]))


#define BOT_RESET_REQUEST 0xFF
#define BOT_GET_MAX_LUN_REQUEST 0xFE


static TCHAR g_szActiveKey[MAX_PATH];


CRITICAL_SECTION g_cs;

HANDLE g_htTransfers;


enum CONTROL_RESPONSE {
    CR_SUCCESS = 0,
    CR_SUCCESS_SEND_CONTROL_HANDSHAKE, // Use if no data stage
    CR_STALL_DEFAULT_PIPE,
    CR_UNHANDLED_REQUEST,
};


typedef struct _PIPE_TRANSFER {
    PUFN_PIPE phPipe;
    HANDLE hev;
    UFN_TRANSFER hTransfer;
} PIPE_TRANSFER, *PPIPE_TRANSFER;

PIPE_TRANSFER g_rgPipeTransfers[] = 
    { { &g_hDefaultPipe }, { &g_hBIPipe }, { &g_hBOPipe } };


#define CONTROL_TRANSFER 0
#define IN_TRANSFER 1
#define OUT_TRANSFER 2


static LPCWSTR g_rgpszStrings0409[] = {
    g_RegInfo.szVendor, g_RegInfo.szProduct
#ifdef USBCV_MSC /*We should enable this for USBCV test*/
        , g_RegInfo.szSerialNumber
#endif
};

static UFN_STRING_SET g_rgStringSets[] = {
    0x0409, g_rgpszStrings0409, dim(g_rgpszStrings0409)
};

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
// This variable tracks whether the IN and OUT pipes need to be kept stalled
// inspite of clear stall requests from the host side
// as per the Section 6.6.1 of 
// Universal Serial Bus Mass Storage Class Bulk-Only Transport
static bool g_pipesstalled = false;

// Tracks the stall status of the OUT pipe
static bool g_outpipestalled = false;
#endif

//#ifdef DEBUG
#if 0    // Removed for Level4 warning
static const LPCTSTR g_rgpszMscStates[] = {
    _T("unknown"),
    _T("idle"),
    _T("command transport"),
    _T("data in transport"),
    _T("data out transport"), 
    _T("status transport"),
    _T("wait for reset"), 
};

// Get a textual name of a device state.
static
LPCTSTR
GetMscStateName(
    MSC_STATE msc
    )
{
    DEBUGCHK(msc < dim(g_rgpszMscStates));
    LPCTSTR psz = g_rgpszMscStates[msc];
    return psz;
}
#endif

// Change the device state and print out a message.
static
inline
VOID
ChangeMscState(
    MSC_STATE msc
    )
{
    // SETFNAME(_T("ChangeMscState"));
//    DEBUGMSG(ZONE_COMMENT, (_T("%s State change: %s -> %s\r\n"), pszFname,
//        GetMscStateName(g_MscState), GetMscStateName(msc)));
    g_MscState = msc;
}



// Read a configuration value from the registry.
static
BOOL
BOT_ReadConfigurationValue(
    HKEY    hClientDriverKey,
    LPCTSTR pszValue,
    PDWORD  pdwResult,
    BOOL    fMustSucceed
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("BOT_ReadConfigurationValue"));
#endif
    FUNCTION_ENTER_MSG();
    
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

    FUNCTION_LEAVE_MSG();

    return fResult;
}


// Configure the function controller based on registry settings.  This
// routine is not responsible for validating the data supplied by the registry.
static
BOOL
BOT_Configure(
    LPCTSTR pszActiveKey,
    HKEY hClientDriverKey
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("BOT_Configure"));
#endif
    FUNCTION_ENTER_MSG();
    
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

#if 0
    DWORD cStrings = dim(g_rgpszStrings0409);
    DWORD iSerialNumber = 3;
    if (g_RegInfo.szSerialNumber[0] == 0) {
        DWORD dwSuccessSerialNumber = UfnGetSystemSerialNumber(
            g_RegInfo.szSerialNumber, dim(g_RegInfo.szSerialNumber));
        
        if (dwSuccessSerialNumber != ERROR_SUCCESS) {
            // No serial number
            cStrings = dim(g_rgpszStrings0409) - 1;
            iSerialNumber = 0;
        }
    }

    g_rgStringSets[0].cStrings = cStrings;
    g_HighSpeedDeviceDesc.iSerialNumber = (UCHAR) iSerialNumber;
    g_FullSpeedDeviceDesc.iSerialNumber = (UCHAR) iSerialNumber;
    
    UfnCheckPID(&g_HighSpeedDeviceDesc, &g_FullSpeedDeviceDesc, 
        PID_MICROSOFT_MASS_STORAGE_PROTOTYPE);
#endif

    if (!BOT_ReadConfigurationValue(hClientDriverKey, UMS_REG_BUFFER_VAL, 
            &g_cbDataBuffer, FALSE)) {
        g_cbDataBuffer = DEFAULT_DATA_BUFFER_SIZE;
    }

    dwRet = ERROR_SUCCESS;

EXIT:
    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}


// Reset the transfer state of a pipe.
static
VOID
BOT_ResetPipeState(
    PUSB_PIPE_STATE pPipeState
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("BOT_ResetPipeState"));
#endif
    FUNCTION_ENTER_MSG();

    pPipeState->fSendingLess = FALSE;

    FUNCTION_LEAVE_MSG();
}


// Build a Command-Status packet.  The resulting packet is stored in
// g_rgbCXWBuffer.
static
VOID
BOT_BuildCSW(
    DWORD dwCSWTag,
    DWORD dwCSWDataResidue,
    BYTE  bCSWStatus
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("BOT_BuildCSW"));
#endif
    FUNCTION_ENTER_MSG();

    PCSW pCsw = (PCSW) g_rgbCXWBuffer;

    pCsw->dCSWSignature = CSW_SIGNATURE;
    pCsw->dCSWTag = dwCSWTag;
    pCsw->dCSWDataResidue = dwCSWDataResidue;
    pCsw->bCSWStatus = bCSWStatus;

    FUNCTION_LEAVE_MSG();
}

DWORD
static
WINAPI 
DefaultTransferComplete(
    PVOID pvNotifyParameter
    )
{   
    HANDLE hev = (HANDLE) pvNotifyParameter;

    DEBUGMSG(ZONE_COMMENT, (_T("BOT DefaultTransferComplete setting event\r\n")));

    SetEvent(hev);

    return ERROR_SUCCESS;
}


// Prepare to receive data from the host.
static
VOID
BOT_SetupRx(
    PPIPE_TRANSFER pPipeTransfer,
    PBYTE pbData,
    DWORD cbData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("BOT_SetupRx"));
#endif
    FUNCTION_ENTER_MSG();

    DEBUGCHK(pbData != NULL);

    g_pUfnFuncs->lpIssueTransfer(g_hDevice, *pPipeTransfer->phPipe, 
        &DefaultTransferComplete, pPipeTransfer->hev, USB_OUT_TRANSFER, cbData,
        pbData, 0, NULL, &pPipeTransfer->hTransfer);

    FUNCTION_LEAVE_MSG();
}


// Prepare to send data to the host.
static
VOID
BOT_SetupTx(
    PPIPE_TRANSFER pPipeTransfer,
    PBYTE pbData,
    DWORD cbData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("BOT_SetupTx"));
#endif
    FUNCTION_ENTER_MSG();

    DEBUGCHK(pbData != NULL);

    g_pUfnFuncs->lpIssueTransfer(g_hDevice, *pPipeTransfer->phPipe, 
        &DefaultTransferComplete, pPipeTransfer->hev, USB_IN_TRANSFER, 
        cbData, pbData, 0, NULL, &pPipeTransfer->hTransfer);

    FUNCTION_LEAVE_MSG();
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



// Get the pipe handle given an endpoint address.
static
UFN_PIPE
BOT_GetPipe(
    DWORD dwAddress
    )
{
    UFN_PIPE hPipe = NULL;
    
    PUFN_ENDPOINT pEndpoint= GetEndpointDescriptor(g_SpeedSupported);
        
    if (dwAddress == pEndpoint[0].Descriptor.bEndpointAddress) {
        hPipe = g_hBIPipe;
    }
    else if (dwAddress == pEndpoint[1].Descriptor.bEndpointAddress) {
        hPipe = g_hBOPipe;
    }
    
    return hPipe;
}




static
VOID
ReceiveCBW(
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ReceiveCBW"));
#endif
    FUNCTION_ENTER_MSG();

    if (g_MscState == MSC_STATE_IDLE) {
        g_dwCBWRead = 0;
        memset(g_rgbCXWBuffer, 0, sizeof(CBW));
#ifdef USBCV_MSC
        BOT_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], g_rgbCXWBuffer, MAX_BOT_COMMAND_LENGTH);
#else
        BOT_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], g_rgbCXWBuffer, sizeof(CBW));
#endif
        ChangeMscState(MSC_STATE_COMMAND_TRANSPORT);
    }

    FUNCTION_LEAVE_MSG();
}


// Process a USB Clear Feature Standard Request. The MDD has already 
// responded to the request so this is strictly informational.
static
VOID
BOT_HandleClearFeature(
    USB_DEVICE_REQUEST udr
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("BOT_HandleClearFeature"));
#endif
    FUNCTION_ENTER_MSG();
    
    if ( (udr.bmRequestType & USB_REQUEST_FOR_ENDPOINT) && 
         (udr.wValue == USB_FEATURE_ENDPOINT_STALL) ) {
        UFN_PIPE hPipe = BOT_GetPipe(udr.wIndex);

        if (hPipe != NULL) {
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
            if ((g_MscState == MSC_STATE_WAIT_FOR_RESET) ) {
                if (g_pipesstalled) {
                    ChangeMscState(MSC_STATE_WAIT_FOR_RESET);
                    g_pUfnFuncs->lpStallPipe(g_hDevice, g_hBIPipe);
                    g_pUfnFuncs->lpStallPipe(g_hDevice, g_hBOPipe);
                }
#ifdef USBCV_MSC
                else
                {
                    if (hPipe == g_hBIPipe) 
                    {
                        RETAILMSG(USBCV_MSC_LOG, (L"goto Idle phase\r\n"));
                        ChangeMscState(MSC_STATE_IDLE);
                        ReceiveCBW();
                    }
                }
            }

            if (g_MscState == MSC_STATE_COMMAND_FAILURE)
            {
                if (hPipe == g_hBIPipe)
                {
                    ChangeMscState(MSC_STATE_STATUS_TRANSPORT);
                    if (g_outpipestalled)
                    {
                        g_pUfnFuncs->lpClearPipeStall(g_hDevice, g_hBOPipe);
                        g_outpipestalled = false;
                    }
                    BOT_BuildCSW(g_pCbw->dCBWTag, g_dwCSWDataResidue, (BYTE) g_dwCSWStatus);
                    BOT_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], g_rgbCXWBuffer, sizeof(CSW));  
                }
            }
#else /*of "USBCV_MSC*/
                else {
                    if (hPipe == g_hBIPipe) {
                        ChangeMscState(MSC_STATE_STATUS_TRANSPORT);        
                        if (g_outpipestalled) {
                            g_pUfnFuncs->lpClearPipeStall(g_hDevice, g_hBOPipe);
                            g_outpipestalled = false;
                        }
                        BOT_BuildCSW(g_pCbw->dCBWTag, g_dwCSWDataResidue, (BYTE) g_dwCSWStatus);
                        BOT_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], g_rgbCXWBuffer, sizeof(CSW));  
                    }
                }
            }
#endif

#else
            if ( (hPipe == g_hBIPipe) && (g_MscState == MSC_STATE_WAIT_FOR_RESET) ) {
                ChangeMscState(MSC_STATE_STATUS_TRANSPORT);
                BOT_BuildCSW(g_pCbw->dCBWTag, g_dwCSWDataResidue, (BYTE) g_dwCSWStatus);
                BOT_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], g_rgbCXWBuffer, sizeof(CSW));  
            }
#endif
        }
    }
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
    FUNCTION_LEAVE_MSG();
#endif
}


// Process a USB Class Request.  Call Request-specific handler.
static
CONTROL_RESPONSE
BOT_HandleClassRequest(
    USB_DEVICE_REQUEST udr
    )
{
    SETFNAME(_T("BOT_HandleClassRequest"));
    FUNCTION_ENTER_MSG();

    CONTROL_RESPONSE response = CR_STALL_DEFAULT_PIPE;
           
    if (udr.bmRequestType == 
         (USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE | USB_REQUEST_HOST_TO_DEVICE) ) {
        if (udr.bRequest == BOT_RESET_REQUEST) {
            if ( (udr.wValue == 0) && (udr.wIndex == 0) && (udr.wLength == 0) ) {
                // Bulk-only mass storage reset
                DEBUGMSG(ZONE_COMMENT,
                    (_T("%s Bulk-only mass storage reset class request\r\n"),
                    pszFname));
                BOT_ResetPipeState(&g_psDefaultPipeState);
                BOT_ResetPipeState(&g_psBIPipeState);
                BOT_ResetPipeState(&g_psBOPipeState);
                response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
                g_pipesstalled = false;
#endif
            }
            else {
                ERRORMSG(1, (_T("%s Malformed bulk-only mass storage reset ")
                     _T("{value=0x%x, index=0x%x, length=0x%x}\r\n"), 
                    pszFname, udr.wValue, udr.wIndex, udr.wLength));
            }
        }
        else {
            ERRORMSG(1, (_T("%s Unrecognized BOT class bRequest -> 0x%x\r\n"), 
                pszFname, udr.bmRequestType));
        }
    }
    else if (udr.bmRequestType == 
       (USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE | USB_REQUEST_DEVICE_TO_HOST) ) {
        if (udr.bRequest == BOT_GET_MAX_LUN_REQUEST) {
            if ( (udr.wValue == 0) && (udr.wIndex == 0) && (udr.wLength == 1) ) {
                // Limited to a single LUN
                DEBUGMSG(ZONE_COMMENT, (_T("%s Get max LUN class request\r\n"),
                    pszFname));
                g_bScratch = 0;
                BOT_SetupTx(&g_rgPipeTransfers[CONTROL_TRANSFER], &g_bScratch, sizeof(g_bScratch));
                response = CR_SUCCESS;
            }
            else {
                ERRORMSG(1, (_T("%s Malformed get max LUN -> ")
                    _T("{value=0x%, index=0x%x, length=0x%x}\r\n"), 
                    pszFname, udr.wValue, udr.wIndex, udr.wLength));
            }
        }
        else {
            ERRORMSG(1, (_T("%s Unrecognized BOT class bRequest -> 0x%x\r\n"), 
                pszFname, udr.bmRequestType));
        }     
    }
    else {
        ERRORMSG(1, (_T("%s Unrecognized BOT class bRequest -> 0x%x\r\n"), 
            pszFname, udr.bmRequestType));
    }

    FUNCTION_LEAVE_MSG();

    return response;
}


// Process a USB Standard Request.  Call Request-specific handler.
static
VOID
BOT_HandleRequest(
    DWORD dwMsg,
    USB_DEVICE_REQUEST udr
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("BOT_HandleRequest"));
#endif
    FUNCTION_ENTER_MSG();

    CONTROL_RESPONSE response;

    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) {
        response = CR_SUCCESS; // Don't respond since it was already handled.
        
        if ( udr.bmRequestType ==
            (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_ENDPOINT) ) {
            switch (udr.bRequest) {
                case USB_REQUEST_CLEAR_FEATURE:
                    BOT_HandleClearFeature(udr);
                    break;
                default:
                    break;
            }
        }

        else if (udr.bmRequestType ==
            (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE) ) {
            if (udr.bRequest == USB_REQUEST_SET_CONFIGURATION) {                
                // Prepare for command transfer
//                ReceiveCBW();
            }
        }
    }
    else {
        DEBUGCHK(dwMsg == UFN_MSG_SETUP_PACKET);
        response = CR_STALL_DEFAULT_PIPE;

        if (udr.bmRequestType & USB_REQUEST_CLASS) {
            DEBUGMSG(ZONE_COMMENT, (_T("%s Class request\r\n"), pszFname));
//            ChangeMscState(MSC_STATE_IDLE);
            response = BOT_HandleClassRequest(udr);
        }
    }

    if (response == CR_STALL_DEFAULT_PIPE) {
        g_pUfnFuncs->lpStallPipe(g_hDevice, g_hDefaultPipe);
        g_pUfnFuncs->lpSendControlStatusHandshake(g_hDevice);
    }
    else if (response == CR_SUCCESS_SEND_CONTROL_HANDSHAKE) {
        g_pUfnFuncs->lpSendControlStatusHandshake(g_hDevice);
    }

    FUNCTION_LEAVE_MSG();
}


// Execute a Command-Block.
static
DWORD
BOT_ExecuteCB(
    PTRANSPORT_COMMAND  pCommand,
    PTRANSPORT_DATA     pData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("BOT_ExecuteCB"));
#endif
    FUNCTION_ENTER_MSG();
    
    // Execute the command   
    DWORD dwResult = STORE_ExecuteCommand(pCommand, pData);
    if (dwResult == EXECUTE_PASS) {
        DEBUGMSG(ZONE_COMMENT, (_T("%s Command : passed\r\n"), pszFname));
    }
    else {
        DEBUGMSG(ZONE_ERROR, (_T("%s Command : failed\r\n"), pszFname));
    }
    
    FUNCTION_LEAVE_MSG();

    return dwResult;
}


// Process an Command-Block-Wrapper (CBW).
static
BOOL
BOT_HandleCBW(
    )
{
    SETFNAME(_T("BOT_HandleCBW"));
    FUNCTION_ENTER_MSG();
    
    BOOL    fData;
    DWORD   dwDirection;
    DWORD   dwDataSize;
    BOOL    fResult = FALSE;
#ifdef ASYNC_TRANSFER
    static BOOL blRead10 = FALSE;
#endif

    switch (g_MscState) {
        case MSC_STATE_COMMAND_TRANSPORT: {
            // Validate CBW
            if (g_pCbw->dCBWSignature != CBW_SIGNATURE) {
                ERRORMSG(1, (_T("%s Invalid CBW signature\r\n"), pszFname));
                // Reset state and stall the request
                ChangeMscState(MSC_STATE_WAIT_FOR_RESET);
#ifdef USBCV_MSC
#else
                g_pUfnFuncs->lpStallPipe(g_hDevice, g_hBIPipe);
                g_pUfnFuncs->lpStallPipe(g_hDevice, g_hBOPipe);
#endif
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
                g_pipesstalled = true;
#endif
                goto EXIT;
            }            

            // Read bCBWCBLength and CBWCB
            g_tcCommand.Length = g_pCbw->bCBWCBLength;
            g_tcCommand.CommandBlock = g_pCbw->CBWCB;

            DEBUGMSG(ZONE_COMMENT, (_T("%s Length: 0x%04x Flags: 0x%x CBWLen: 0x%x\r\n"),
                pszFname, g_pCbw->dCBWDataTransferLength, g_pCbw->bmCBWFlags, g_pCbw->bCBWCBLength));
            
            if (STORE_IsCommandSupported(&g_tcCommand, &fData, &dwDirection, 
                    &dwDataSize)) {
                g_dwCBWDataTransferLength = g_pCbw->dCBWDataTransferLength;

                g_tcCommand.Flags = dwDirection;
                g_tdData.RequestLength = g_dwCBWDataTransferLength;
                g_tdData.DataBlock = g_pbDataBuffer;

                // Do we need to create a larger data buffer?
                if (g_tdData.RequestLength > g_cbDataBuffer) {
                    g_pbDataBuffer = (PBYTE) LocalReAlloc(g_pbDataBuffer, 
                        g_tdData.RequestLength, LMEM_MOVEABLE);
                    if (g_pbDataBuffer == NULL) {
                        // We are in trouble. We cannot handle this request.
                        ERRORMSG(1, (_T("%s OUT OF MEMORY! STALLING REQUEST.\r\n"),
                            pszFname));

                        // Reset the pointer
                        g_pbDataBuffer = (PBYTE) g_tdData.DataBlock;
                        
                        // Reset state and stall the request
                        ChangeMscState(MSC_STATE_WAIT_FOR_RESET);
                        g_pUfnFuncs->lpStallPipe(g_hDevice, g_hBIPipe);
                        g_pUfnFuncs->lpStallPipe(g_hDevice, g_hBOPipe);
                        goto EXIT;
                    }
                    else {
                        g_tdData.DataBlock = g_pbDataBuffer;
                        g_cbDataBuffer = g_tdData.RequestLength;
                    }
                }

                if (fData) {
                    // Data command
                    if (dwDirection == DATA_IN) {                        
                        ChangeMscState(MSC_STATE_DATA_IN_TRANSPORT);
                        // Execute the command
                        g_dwCSWStatus = BOT_ExecuteCB(&g_tcCommand, &g_tdData);
                        if (g_dwCSWStatus == EXECUTE_PASS) {                            
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#ifdef ASYNC_TRANSFER
                            if(((PBYTE)g_tcCommand.CommandBlock)[0] == 0x28)//SCSI_READ10
                            {
                                blRead10 = TRUE;
                            }
#endif
                            if (g_tdData.TransferLength > 0) {
                                DEBUGMSG(ZONE_COMMENT, (_T("%s request length = %u\r\n"), 
                                    pszFname, g_tdData.RequestLength));
                                BOT_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], (PBYTE) g_tdData.DataBlock,
                                    g_tdData.TransferLength);                            
                                g_dwCSWDataResidue = g_dwCBWDataTransferLength - g_tdData.TransferLength;
                                DEBUGCHK(g_dwCSWDataResidue >= 0);
                                if (g_dwCSWDataResidue > 0) g_psBIPipeState.fSendingLess = TRUE;                            
                                fResult = TRUE;
                            }
                            else {
                                ChangeMscState(MSC_STATE_STATUS_TRANSPORT);
                                BOT_BuildCSW(g_pCbw->dCBWTag, g_dwCSWDataResidue, (BYTE) g_dwCSWStatus);
                                BOT_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], g_rgbCXWBuffer, sizeof(CSW));
                                fResult = TRUE;
                            }
#else
                            DEBUGMSG(ZONE_COMMENT, (_T("%s request length = %u\r\n"), 
                                pszFname, g_tdData.RequestLength));
                            BOT_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], (PBYTE) g_tdData.DataBlock,
                                g_tdData.TransferLength);                            
                            g_dwCSWDataResidue = g_dwCBWDataTransferLength - g_tdData.TransferLength;
                            DEBUGCHK(g_dwCSWDataResidue >= 0);
                            if (g_dwCSWDataResidue > 0) g_psBIPipeState.fSendingLess = TRUE;                            
                            fResult = TRUE;
#endif
                        }
                        else {                            
                            //
                            // the command failed--stall the request; after clearing the stall,
                            // the host will request a CSW
                            //                            
                            g_pUfnFuncs->lpStallPipe(g_hDevice, g_hBIPipe);
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#ifdef USBCV_MSC
                            /*We add a state to handle special case in USBCV test*/
                            ChangeMscState(MSC_STATE_COMMAND_FAILURE);
#else
                            ChangeMscState(MSC_STATE_WAIT_FOR_RESET);
#endif
#else
                            ChangeMscState(MSC_STATE_STATUS_TRANSPORT);
#endif
                            // prepare for CSW transfer
                            BOT_BuildCSW(g_pCbw->dCBWTag, 0, (BYTE) g_dwCSWStatus);
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#else
                            BOT_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], g_rgbCXWBuffer, sizeof(CSW));
#endif
                        }
                    }
                    else if (dwDirection == DATA_OUT) {
                        ChangeMscState(MSC_STATE_DATA_OUT_TRANSPORT);
                        DEBUGMSG(ZONE_COMMENT, (_T("%s request length = %u\r\n"),
                            pszFname, g_tdData.RequestLength));   
#ifdef ASYNC_TRANSFER
                        if(((PBYTE)g_tcCommand.CommandBlock)[0] == 0x2A)//SCSI_WRITE10
                        {
                            g_tdData.DataBlock = (PBYTE) STORE_GetWriteBuffer(g_tdData.RequestLength);
                        }
#endif
                        BOT_SetupRx(&g_rgPipeTransfers[OUT_TRANSFER], (PBYTE) g_tdData.DataBlock, 
                            g_tdData.RequestLength);

                        fResult = TRUE;
                    }
                    else {
                        // This is impossible
                        DEBUGCHK(FALSE);
                    }
                }
                else {
                    // Non-data command. Execute the command and prepare for CSW transfer.
                    ChangeMscState(MSC_STATE_STATUS_TRANSPORT);
                    BOT_BuildCSW(g_pCbw->dCBWTag, 0, (BYTE) BOT_ExecuteCB(&g_tcCommand, NULL));
                    BOT_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], g_rgbCXWBuffer, sizeof(CSW));                    
                    fResult = TRUE;
                }
            }
            else {
                // If the command is not supported, then return a phase error
                DEBUGMSG(ZONE_WARNING, (_T("%s command not supported\r\n"),
                    pszFname));

                // Prepare for CSW transfer
#ifdef USBCV_MSC
                ChangeMscState(MSC_STATE_COMMAND_FAILURE);
                g_dwCSWDataResidue = 0;
                g_dwCSWStatus = EXECUTE_FAIL;

                g_pUfnFuncs->lpStallPipe(g_hDevice, g_hBIPipe);
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
                if ((dwDirection == DATA_OUT) || (dwDirection == DATA_UNKNOWN)) {
                    g_pUfnFuncs->lpStallPipe(g_hDevice, g_hBOPipe);
                    g_outpipestalled = true;
                }
#endif
#else
                ChangeMscState(MSC_STATE_WAIT_FOR_RESET);
                g_dwCSWDataResidue = 0;
                g_dwCSWStatus = EXECUTE_FAIL;

                g_pUfnFuncs->lpStallPipe(g_hDevice, g_hBIPipe);
#endif
            }
            
            break;
        }
        
        case MSC_STATE_DATA_IN_TRANSPORT:
#ifdef ASYNC_TRANSFER
            if(blRead10)
            {
                STORE_ReturnReadBuffer(g_tdData.DataBlock, g_tdData.RequestLength);
                blRead10 = FALSE;
            }
#endif
            ChangeMscState(MSC_STATE_STATUS_TRANSPORT);
            BOT_BuildCSW(g_pCbw->dCBWTag, g_dwCSWDataResidue, (BYTE) g_dwCSWStatus);
            BOT_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], g_rgbCXWBuffer, sizeof(CSW));
            fResult = TRUE;
            break;
            
        case MSC_STATE_DATA_OUT_TRANSPORT:
            // The data block has been transferred and is in g_tdData                       
            g_dwCSWStatus = BOT_ExecuteCB(&g_tcCommand, &g_tdData);
            g_dwCSWDataResidue = g_dwCBWDataTransferLength - g_tdData.TransferLength;            
            DEBUGCHK(g_dwCSWDataResidue >= 0);
            if (g_dwCSWDataResidue > 0) g_psBOPipeState.fSendingLess = TRUE;            
            ChangeMscState(MSC_STATE_STATUS_TRANSPORT);
            BOT_BuildCSW(g_pCbw->dCBWTag, g_dwCSWDataResidue, (UCHAR) g_dwCSWStatus);
            BOT_SetupTx(&g_rgPipeTransfers[IN_TRANSFER], g_rgbCXWBuffer, sizeof(CSW));
            fResult = TRUE;
            break;
        
        case MSC_STATE_STATUS_TRANSPORT:
            ChangeMscState(MSC_STATE_IDLE);
            // Prepare for command transfer
            ReceiveCBW();
            fResult = TRUE;
            break;

        case MSC_STATE_WAIT_FOR_RESET:
            fResult = TRUE;
            break;
        
        default:            
            // This is impossible
            DEBUGCHK(FALSE);
    }

EXIT:

    FUNCTION_LEAVE_MSG();

    return fResult;
}


static
VOID
ProcessBOPipeTransfer(
    DWORD cbTransferred
    )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(cbTransferred);

#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessBOPipeTransfer"));
#endif
    FUNCTION_ENTER_MSG();

    if (g_MscState == MSC_STATE_COMMAND_TRANSPORT) {
        // Command state
        DEBUGCHK(cbTransferred == sizeof(CBW));
        DEBUGMSG(ZONE_COMMENT, (_T("%s Received CBW\r\n"),
            pszFname));
#ifdef USBCV_MSC
        if (cbTransferred == sizeof(CBW))
        {
            BOT_HandleCBW();
        }
        else
        {
            RETAILMSG(USBCV_MSC_LOG, (L"invalid CBW length\r\n"));
            ChangeMscState(MSC_STATE_WAIT_FOR_RESET);
#ifdef QFE_MERGE
            g_pipesstalled = true;
#endif
        }
#else
        BOT_HandleCBW();
        g_fCBWArrived = TRUE;
#endif
    }
    else if (g_MscState == MSC_STATE_DATA_OUT_TRANSPORT) {
        // Data out state
        DEBUGMSG(ZONE_COMMENT, (_T("%s Received data block\r\n"),
            pszFname));
        BOT_HandleCBW();
    }

    FUNCTION_LEAVE_MSG();
}


static
VOID
ProcessBIPipeTransfer(
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessBIPipeTransfer"));
#endif
    FUNCTION_ENTER_MSG();

    if (g_MscState == MSC_STATE_STATUS_TRANSPORT) {
        BOT_HandleCBW();
    }
    else if (g_MscState == MSC_STATE_DATA_IN_TRANSPORT) {
        if (g_psBIPipeState.fSendingLess) {
            g_psBIPipeState.fSendingLess = FALSE;
            g_pUfnFuncs->lpStallPipe(g_hDevice, g_hBIPipe);
            ChangeMscState(MSC_STATE_WAIT_FOR_RESET);
        }
        
        BOT_HandleCBW();
    }
    
    FUNCTION_LEAVE_MSG();
}


// Open the pipes associated with the default interface.
static
BOOL
BOT_OpenInterface(
    )
{
    SETFNAME(_T("BOT_OpenInterface"));
    FUNCTION_ENTER_MSG();
    
    BOOL fResult = FALSE;
    PUFN_ENDPOINT pEndpoint= GetEndpointDescriptor(g_SpeedSupported);

    // Open the pipes of the associated interface.

    DEBUGCHK(g_hBOPipe == NULL);
    DEBUGCHK(g_hBIPipe == NULL);
    
    DWORD dwRet = g_pUfnFuncs->lpOpenPipe(g_hDevice, 
        pEndpoint[0].Descriptor.bEndpointAddress,
        &g_hBIPipe);
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, (_T("%s Failed to open bulk in pipe\r\n"),
            pszFname));
        goto EXIT;
    }

    dwRet = g_pUfnFuncs->lpOpenPipe(g_hDevice, 
        pEndpoint[1].Descriptor.bEndpointAddress,
        &g_hBOPipe);
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, (_T("%s Failed to open bulk in pipe\r\n"),
            pszFname));
        goto EXIT;
    }

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
    g_pUfnFuncs->lpClearPipeStall(g_hDevice, g_hBIPipe);
    g_pUfnFuncs->lpClearPipeStall(g_hDevice, g_hBOPipe);
    g_pipesstalled = FALSE;
    g_outpipestalled = FALSE;
#endif

    // Prepare for command transfer
    ReceiveCBW();
    
    fResult = TRUE;
    
EXIT:
    FUNCTION_LEAVE_MSG();

    return fResult;
}


// Close the store
DWORD
BOT_Close(
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("BOT_Close"));
#endif
    FUNCTION_ENTER_MSG();
    
    if (g_fStoreOpened) {
        STORE_Close();
        g_fStoreOpened = FALSE;
    }
    
    if (g_pbDataBuffer) LocalFree(g_pbDataBuffer);

    FUNCTION_LEAVE_MSG();
    
    return ERROR_SUCCESS;
}


// Process a device event.
static
BOOL
WINAPI
BOT_DeviceNotify(
    PVOID   pvNotifyParameter,
    DWORD   dwMsg,
    DWORD   dwParam
    ) 
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pvNotifyParameter);

    EnterCriticalSection(&g_cs);
    
    SETFNAME(_T("BOT_DeviceNotify"));
    FUNCTION_ENTER_MSG();

    switch(dwMsg) {
        case UFN_MSG_BUS_EVENTS: {
            // Ensure device is in running state
            DEBUGCHK(g_hDefaultPipe);

            switch(dwParam) {
                case UFN_DETACH:
                    if (g_fStoreOpened) {
                        STORE_Close();
                        g_fStoreOpened = FALSE;
                    }

                    // Reset device
                    BOT_ResetPipeState(&g_psDefaultPipeState);
                    BOT_ResetPipeState(&g_psBOPipeState);
                    BOT_ResetPipeState(&g_psBIPipeState);

                    if (g_hBOPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                        g_hBOPipe = NULL;
                    }

                    if (g_hBIPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                        g_hBIPipe = NULL;
                    }

                    g_fCBWArrived = FALSE;
                    ChangeMscState(MSC_STATE_IDLE);
                    break;

                case UFN_ATTACH: {
                    // Open store if not already open
                    if (!g_fStoreOpened) {
                        RETAILMSG(1, (TEXT("UFN_ATTACH with key = %s\r\n"), g_szActiveKey));
                        DWORD dwRet = STORE_Init(g_szActiveKey);
                        if (dwRet != ERROR_SUCCESS) {
                            ERRORMSG(1, (_T("%s Failed to open store\r\n"),
                                pszFname));
                        }
                        else {
                            g_fStoreOpened = TRUE;
                        }
                    }

                    // Reset device
                    BOT_ResetPipeState(&g_psDefaultPipeState);
                    BOT_ResetPipeState(&g_psBOPipeState);
                    BOT_ResetPipeState(&g_psBIPipeState);

                    if (g_hBOPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                        g_hBOPipe = NULL;
                    }

                    if (g_hBIPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                        g_hBIPipe = NULL;
                    }

                    g_fCBWArrived = FALSE;
                    ChangeMscState(MSC_STATE_IDLE);
                    break;
                }
            
                case UFN_RESET:
                    // Reset device
                    BOT_ResetPipeState(&g_psDefaultPipeState);
                    BOT_ResetPipeState(&g_psBOPipeState);
                    BOT_ResetPipeState(&g_psBIPipeState);

                    if (g_hBOPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                        g_hBOPipe = NULL;
                    }

                    if (g_hBIPipe) {
                        g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                        g_hBIPipe = NULL;
                    }
                    
                    g_fCBWArrived = FALSE;
                    ChangeMscState(MSC_STATE_IDLE);
                    break;
                default:
                    break;
            }

            break;
        }

        case UFN_MSG_BUS_SPEED:
            g_SpeedSupported = (UFN_BUS_SPEED) dwParam;
            break;

        case UFN_MSG_SETUP_PACKET:
        case UFN_MSG_PREPROCESSED_SETUP_PACKET: {
            PUSB_DEVICE_REQUEST pudr = (PUSB_DEVICE_REQUEST) dwParam;
            BOT_HandleRequest(dwMsg, *pudr);
            break;
        }

        case UFN_MSG_CONFIGURED:
            if (dwParam == 0) {
                // Reset device
                BOT_ResetPipeState(&g_psDefaultPipeState);
                BOT_ResetPipeState(&g_psBOPipeState);
                BOT_ResetPipeState(&g_psBIPipeState);

                if (g_hBOPipe) {
                    g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBOPipe);
                    g_hBOPipe = NULL;
                }

                if (g_hBIPipe) {
                    g_pUfnFuncs->lpClosePipe(g_hDevice, g_hBIPipe);
                    g_hBIPipe = NULL;
                }
                
                g_fCBWArrived = FALSE;
                ChangeMscState(MSC_STATE_IDLE);
            }
            else {
                RETAILMSG(1,(TEXT("UFN_CONFIGURED: BOT_OpenInterface again\r\n")));
                DEBUGCHK(g_hBIPipe == NULL);
                BOT_OpenInterface();
            }
            break;
        default:
            break;
    }
    
    FUNCTION_LEAVE_MSG();

    LeaveCriticalSection(&g_cs);

    return TRUE;
}



DWORD
WINAPI
BOT_TransferThread(
    LPVOID lpParameter
    )
{    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParameter);

#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("BOT_TransferThread"));
#endif
    FUNCTION_ENTER_MSG();
#ifdef MSCDSK
    HANDLE rghevWaits[dim(g_rgPipeTransfers) + 1];
#else
    HANDLE rghevWaits[dim(g_rgPipeTransfers)];
#endif

    for (DWORD dwIdx = 0; dwIdx < dim(g_rgPipeTransfers); ++dwIdx) {
        DEBUGCHK(g_rgPipeTransfers[dwIdx].hev);
        rghevWaits[dwIdx] = g_rgPipeTransfers[dwIdx].hev;
    }
#ifdef MSCDSK
    rghevWaits[dim(g_rgPipeTransfers)] = CreateEvent(NULL, TRUE, FALSE, L"MSC_DETACH_EVENT");
#endif

    for (;;) {
        // How to exit?
        DWORD dwWait = WaitForMultipleObjects(dim(rghevWaits), rghevWaits, 
            FALSE, INFINITE);

        DEBUGMSG(ZONE_COMMENT, (_T("%s Transfer %u\r\n"), pszFname, dwWait));

        DWORD dwIdx = dwWait - WAIT_OBJECT_0;
            
        if (dwIdx >= dim(rghevWaits)) {
            break;
        }
#ifdef MSCDSK
        if (dwIdx == dim(rghevWaits) - 1)
        {
            //this is the MSC_DETACH_EVENT

            BOT_DeviceNotify(NULL, UFN_MSG_BUS_EVENTS, UFN_DETACH);

            BOT_ResetPipeState(&g_psDefaultPipeState);
            BOT_ResetPipeState(&g_psBIPipeState);
            BOT_ResetPipeState(&g_psBOPipeState);
            g_pUfnFuncs->lpStop(g_hDevice);

            g_pUfnFuncs->lpStart(g_hDevice, BOT_DeviceNotify, NULL, 
                                 &g_hDefaultPipe);
            Sleep(200);
            continue;
        }
#endif
        
        EnterCriticalSection(&g_cs);
        
        PPIPE_TRANSFER pPipeTransfer;
        pPipeTransfer = &g_rgPipeTransfers[dwIdx];
        DEBUGCHK(pPipeTransfer->hev);
        DEBUGCHK(pPipeTransfer->hTransfer);

        DWORD dwUsbError;
        DWORD cbTransferred;
        DWORD dwErr;
        
        dwErr = g_pUfnFuncs->lpGetTransferStatus(g_hDevice, pPipeTransfer->hTransfer,
            &cbTransferred, &dwUsbError);
        DEBUGCHK(dwErr == ERROR_SUCCESS);

        DEBUGMSG(ZONE_COMMENT, (_T("%s %u bytes transferred\r\n"), pszFname, 
            cbTransferred));

        dwErr = g_pUfnFuncs->lpCloseTransfer(g_hDevice, pPipeTransfer->hTransfer);
        DEBUGCHK(dwErr == ERROR_SUCCESS);
        pPipeTransfer->hTransfer = NULL;

        if (dwUsbError != UFN_NO_ERROR) {
            DEBUGCHK(dwUsbError == UFN_CANCELED_ERROR);
            goto CONTINUE;
        }

        // Don't process if detached
        if (g_MscState == MSC_STATE_IDLE) {
            goto CONTINUE;
        }

        // TODO: Don't do these if transfer error
        if (dwIdx == CONTROL_TRANSFER) {
            g_pUfnFuncs->lpSendControlStatusHandshake(g_hDevice);
        }
        else if (dwIdx == OUT_TRANSFER) {
            ProcessBOPipeTransfer(cbTransferred);
        }
        else {
            DEBUGCHK(dwIdx == IN_TRANSFER);
            ProcessBIPipeTransfer();
        }

CONTINUE:
        LeaveCriticalSection(&g_cs);
    }

    FUNCTION_LEAVE_MSG();

    return 0;
}


// Initialize the Bulk-Only Transport layer.
DWORD
BOT_InternalInit(
    LPCTSTR         pszActiveKey
    )
{
    SETFNAME(_T("BOT_InternalInit"));
    FUNCTION_ENTER_MSG();
    
    HKEY    hClientDriverKey = NULL;
    DWORD   dwRet;
    HRESULT hr = 0;

    PREFAST_DEBUGCHK(pszActiveKey);

    RETAILMSG(1, (TEXT("BOT_InternalInit with %s\r\n"), pszActiveKey));
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
    dwRet = BOT_Configure(pszActiveKey, hClientDriverKey);
    if (dwRet != ERROR_SUCCESS) {
        ERRORMSG(1, 
            (_T("%s Failed to configure function controller from registry\r\n"),
            pszFname));
        goto EXIT;
    }
    
//    g_pUfnFuncs = pUfnFuncs;

    // Allocate data buffer
    g_pbDataBuffer = (PBYTE) LocalAlloc(0, g_cbDataBuffer);
    if (g_pbDataBuffer == NULL) {
        dwRet = GetLastError();
        ERRORMSG(1, (_T("%s LocalAlloc failed. Error: %d\r\n"), 
            pszFname, dwRet));
        goto EXIT;
    }

    // Register descriptor tree with device controller
    dwRet = g_pUfnFuncs->lpRegisterDevice(g_hDevice,
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

    // Create transfer thread
    g_htTransfers = CreateThread(NULL, 0, BOT_TransferThread, NULL, 0, NULL);
    if (g_htTransfers == NULL) {
        ERRORMSG(1, (_T("%s Transfer thread creation failed\r\n"), pszFname));
        dwRet = GetLastError();
        goto EXIT;
    }

    // Read transfer thread priority from registry
    DWORD dwTransferThreadPriority;
    if (BOT_ReadConfigurationValue(hClientDriverKey, 
        UMS_REG_TRANSFER_THREAD_PRIORITY_VAL, &dwTransferThreadPriority, FALSE))
    {
        DEBUGMSG(1,
            (_T("%s BOT transfer thread priority = %u (from registry)\r\n"), 
            pszFname, dwTransferThreadPriority));
    }
    else {
        dwTransferThreadPriority = DEFAULT_TRANSFER_THREAD_PRIORITY;
        DEBUGMSG(1, (_T("%s BOT transfer thread priority = %u\r\n"), 
            pszFname, dwTransferThreadPriority));
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
    dwRet = g_pUfnFuncs->lpStart(g_hDevice, BOT_DeviceNotify, NULL,   // UfnMdd_Start
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
        if (g_pbDataBuffer) {
            LocalFree(g_pbDataBuffer);
        }
        if (g_fDeviceRegistered) {
            g_pUfnFuncs->lpDeregisterDevice(g_hDevice);
        }
    }

    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}


static PVOID g_pvInterface = NULL;


extern "C"
DWORD
Init(
    LPCTSTR pszActiveKey
    )
{
    DWORD dwErr = UfnInitializeInterface(pszActiveKey, &g_hDevice, 
        &g_ufnFuncs, &g_pvInterface);
    if (dwErr == ERROR_SUCCESS) {
        dwErr = BOT_InternalInit(pszActiveKey);

        if (dwErr != ERROR_SUCCESS) {
            UfnDeinitializeInterface(g_pvInterface);
        }
    }

    return (dwErr == ERROR_SUCCESS);
}


extern "C" 
BOOL
Deinit(
    DWORD dwContext
    )
{
    BOT_Close();
    DEBUGCHK(g_pvInterface);
    UfnDeinitializeInterface(g_pvInterface);
    
    // Remove-W4: Warning C4100 workaround    
    UNREFERENCED_PARAMETER(dwContext);

    return TRUE;
}

