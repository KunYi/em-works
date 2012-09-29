//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2009 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  simce.c
//
//   This file implements the stream interface functions for sim
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>
#pragma warning(pop)
#include <devload.h>
#pragma warning(push)
#pragma warning(disable: 4201)
#pragma warning(disable: 4214)
#include <smclib.h>
#include <winsmcrd.h>
#pragma warning(pop)
#include "simce.h"
#include "simcb.h"
#include "simhw.h"

//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines

#define MAXIMUM_SIM_DEVICES     2

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

CRITICAL_SECTION g_DriverCritSect;      // Used to synchronize access to global driver data structures
PSMARTCARD_EXTENSION g_DeviceSlot[MAXIMUM_SIM_DEVICES];       // allow upto MAX_SIM_DEVICES

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
static DWORD GetDeviceName( LPTSTR ActivePath, LPTSTR szDeviceName );
static void MakeFriendlyName(PSMARTCARD_EXTENSION SmartcardExtension, LPWSTR szFriendlyName);
static DWORD GetIOIndex(PREADER_EXTENSION pReaderExtension,LPTSTR ActiveKey);
static PSMARTCARD_EXTENSION SIMLoadDevice( LPTSTR ActiveKey );
static void SIMUnloadDevice(PSMARTCARD_EXTENSION SmartcardExtension);

extern DWORD CspSIMGetBaseAddress(DWORD dwIndex);

#ifdef DEBUG

DBGPARAM dpCurSettings = {
    TEXT("SIM"), {
    TEXT("Ioctl"), TEXT("ATR"), TEXT("Protocol"), TEXT("Driver"),
    TEXT("Trace"),TEXT("Error"), TEXT("break"),TEXT("all"),
    TEXT(" unused"),TEXT("unused"),TEXT("unused"),TEXT("unused"),
    TEXT("unused"),TEXT("unused"),TEXT("unused"),TEXT("unused") },
    0x00000400    // DEBUG_ERROR
};
#endif  // DEBUG


//-----------------------------------------------------------------------------
//
// Function: AddDevice
//
// This function registers the device context
//
// Parameters:
//      pDevice
//          [in] Pointer of the context of device
//
// Returns:
//      BOOL true for success, false for fail
//
//-----------------------------------------------------------------------------
static BOOL AddDevice(PSMARTCARD_EXTENSION pDevice)
{
    int i;
    EnterCriticalSection(&g_DriverCritSect);
    for (i=0; i< MAXIMUM_SIM_DEVICES; i++)
    {
        if (g_DeviceSlot[i] == NULL)
        {
            g_DeviceSlot[i] = pDevice;
            break;
        }
    }
    LeaveCriticalSection(&g_DriverCritSect);
    return (i < MAXIMUM_SIM_DEVICES);
}


//-----------------------------------------------------------------------------
//
// Function: RemoveDevice
//
// This function removes the device context
//
// Parameters:
//      pDevice
//          [in]Pointer of the context of device
//
// Returns:
//      BOOL true for success, false for fail
//
//-----------------------------------------------------------------------------
static BOOL RemoveDevice(PSMARTCARD_EXTENSION pDevice)
{
    int i;
    EnterCriticalSection(&g_DriverCritSect);
    for (i=0; i< MAXIMUM_SIM_DEVICES; i++)
    {
        if (g_DeviceSlot[i] == pDevice)
        {
            g_DeviceSlot[i] = NULL;
            break;
        }
    }
    LeaveCriticalSection(&g_DriverCritSect);

    return (i < MAXIMUM_SIM_DEVICES);
}


//-----------------------------------------------------------------------------
//
// Function: ValidateAndEnterDevice
//
// This function verifies and then registers the device context
//
// Parameters:
//      pDevice
//          [in]Pointer of the context of device
//
// Returns:
//      BOOL true for success, false for fail
//
//-----------------------------------------------------------------------------
static BOOL ValidateAndEnterDevice(PSMARTCARD_EXTENSION pDevice)
{
    int i;
    EnterCriticalSection(&g_DriverCritSect);
    
    for (i=0; i< MAXIMUM_SIM_DEVICES; i++)
    {
        if (g_DeviceSlot[i] == pDevice)
        {
            EnterDevice(pDevice);
            break;
        }
    }
    LeaveCriticalSection(&g_DriverCritSect);
#ifdef DEBUG
    if (i >= MAXIMUM_SIM_DEVICES)
    {
        SmartcardDebug(DEBUG_ERROR,(TEXT("ValidateDevice - Invalid Object %x\n"),pDevice));
    }
#endif
    return (i < MAXIMUM_SIM_DEVICES);
}


//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This function provides the dll entry for the driver
//
// Parameters:
//      DllInstance
//          [in] the instance to the dll
//      Reason
//          [in] command for attach and detach
//      Reserved
//          [in] reserved for future use
//
// Returns:
//      true for success, false for fail
//
//-----------------------------------------------------------------------------
BOOL WINAPI DllEntry(HINSTANCE DllInstance, INT Reason, LPVOID Reserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Reserved);

    switch(Reason) {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER(DllInstance);
        SmartcardDebug(DEBUG_TRACE, (TEXT("DLL_PROCESS_ATTACH\r\n")));
#ifdef DEBUG
        RegisterDbgZones( (HINSTANCE)DllInstance, &dpCurSettings );
#endif
        InitializeCriticalSection(&g_DriverCritSect);
        memset(g_DeviceSlot,0,sizeof(g_DeviceSlot));
        DisableThreadLibraryCalls((HMODULE) DllInstance);
        break;
   
    case DLL_PROCESS_DETACH:
        SmartcardDebug(DEBUG_TRACE, (TEXT("DLL_PROCESS_DETACH\r\n")));
        DeleteCriticalSection(&g_DriverCritSect);
        break;
    }
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SIMCardDetectionThread
//
// This function creates the card detection thread
//
// Parameters:
//      pData
//          [in] context of call
//
// Returns:
//      0
//
//-----------------------------------------------------------------------------
static DWORD CALLBACK SIMCardDetectionThread(PVOID pData )
{
    NTSTATUS    NTStatus = STATUS_SUCCESS;
    DWORD OldState;
    PSMARTCARD_EXTENSION SmartcardExtension=(PSMARTCARD_EXTENSION)pData;       

    SmartcardDebug(DEBUG_TRACE,(TEXT("+SIMCardDetectionThread Entering polling thread\n")));

    while (SmartcardExtension->ReaderExtension->d_uReaderState == STATE_OPENED)  // infinit loop
    {
        
        if (SmartcardExtension->ReaderExtension->TrackingThreadTerminate == TRUE)
            break;
        
        OldState = SmartcardExtension->ReaderCapabilities.CurrentState;

        NTStatus = CBUpdateCardState(SmartcardExtension);

         if(((OldState >  SCARD_ABSENT) && (SmartcardExtension->ReaderCapabilities.CurrentState <= SCARD_ABSENT))
         || ((OldState <= SCARD_ABSENT) && (SmartcardExtension->ReaderCapabilities.CurrentState >  SCARD_ABSENT)))        
         {
            SmartcardDebug(DEBUG_TRACE, (TEXT("SIMCardDetectionThread - Card %s\n"),
                (SmartcardExtension->ReaderCapabilities.CurrentState <= SCARD_ABSENT)? TEXT("removed"): TEXT("inserted")));

            SmartcardCompleteCardTracking(SmartcardExtension);
            SmartcardExtension->ReaderExtension->IoctlPending=FALSE;

         }
        Sleep(POLLING_PERIOD);
    };

    SmartcardDebug(DEBUG_TRACE,(TEXT("-SIMCardDetectionThread Terminate polling thread\n")));
    return 0; 
}


//-----------------------------------------------------------------------------
//
// Function: SIMVendorIoctl
//
// This function performs the vendor io control
//
// Parameters:
//      SmartcardExtension
//          [in] context of call
//
// Returns:
//      NTSTATUS
//
//-----------------------------------------------------------------------------
static NTSTATUS SIMVendorIoctl(PSMARTCARD_EXTENSION SmartcardExtension)
{
    NTSTATUS status;
    static TCHAR answer[] = _T("Vendor IOCTL");

    SmartcardDebug(DEBUG_PROTOCOL,(TEXT("+SIMVendorIoctl\n")));
    
    if (SmartcardExtension->IoRequest.ReplyBuffer != NULL && 
        SmartcardExtension->IoRequest.ReplyBufferLength >= (_tcslen(answer) + 1)*sizeof(TCHAR)) 
    { 
        StringCchCopy ((TCHAR *)SmartcardExtension->IoRequest.ReplyBuffer, SmartcardExtension->IoRequest.ReplyBufferLength, answer);
        *SmartcardExtension->IoRequest.Information = _tcslen(answer);
        status = STATUS_SUCCESS;
    } 
    else 
    {
        status = STATUS_BUFFER_TOO_SMALL;
    }

    SmartcardDebug(DEBUG_PROTOCOL,(TEXT("-SIMVendorIoctl (%lx)\n"),status));

    return status;
}


//-----------------------------------------------------------------------------
//
// Function: SCR_Init
//
// This function does the initialization to the device
//
// Parameters:
//      dwContext
//          [in] registry path for this device's active key
//
// Returns:
//      DWORD context data (PDISK) for this Init instance or 0 for failure.
//
//-----------------------------------------------------------------------------
DWORD SCR_Init(DWORD dwContext)
{
    PSMARTCARD_EXTENSION   pSmartcardExtension;
    LPTSTR ActiveKey = (LPTSTR)dwContext;

    SmartcardDebug(DEBUG_PROTOCOL,(TEXT("+SCR_Init\n")));

#if 0 // Remove-W4: Warning C4706 workaround
    if (pSmartcardExtension=SIMLoadDevice(ActiveKey)) 
#else
    if ((pSmartcardExtension=SIMLoadDevice(ActiveKey)) != 0)
#endif
    {
        if (AddDevice(pSmartcardExtension)) // check for device overflow
        {
            return (DWORD)pSmartcardExtension;
        }
        else 
        {
            SmartcardDebug(DEBUG_TRACE|DEBUG_ERROR, (TEXT("SCR_Init Device Overflow error\r\n")));
            SIMUnloadDevice(pSmartcardExtension);
            ASSERT(FALSE);
        };

        SmartcardDebug(DEBUG_PROTOCOL,(TEXT("-SCR_Init\n")));
    }

    return (DWORD)pSmartcardExtension;
    // insert call to detection function if one is defined
    //
    // do device initialization
}


//-----------------------------------------------------------------------------
//
// Function: SCR_Deinit
//
// This function does the de-initialization to the device
//
// Parameters:
//      dwContext
//          [in] registry path for this device's active key
//
// Returns:
//      BOOL The device manager does not check the return code
//
//-----------------------------------------------------------------------------
BOOL SCR_Deinit(DWORD dwContext)
{
    SmartcardDebug(DEBUG_TRACE,(TEXT("+SCR_Deinit\r\n")));
    
    SIMUnloadDevice((PSMARTCARD_EXTENSION)dwContext);

    SmartcardDebug(DEBUG_TRACE,(TEXT("-SCR_Deinit\r\n")));
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SCR_Open
//
// This function open the device instance
//
// Parameters:
//      dwData
//          [in] context of call
//      dwAccess
//          [in] not used
//      dwShareMode
//          [in] not used
//
// Returns:
//      DWORD handle value for the open instance
//
//-----------------------------------------------------------------------------
DWORD SCR_Open(DWORD dwData,DWORD dwAccess,DWORD dwShareMode)
{
    PSMARTCARD_EXTENSION   pSmartcardExtension = (PSMARTCARD_EXTENSION) dwData;
    PREADER_EXTENSION readerExtension = pSmartcardExtension->ReaderExtension;
    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwAccess);
    UNREFERENCED_PARAMETER(dwShareMode);

    SmartcardDebug(DEBUG_TRACE,(TEXT("+SCR_Open(%x)\n"),dwData));
    if (!ValidateAndEnterDevice(pSmartcardExtension))
    {
        SetLastError(ERROR_BAD_DEVICE);
        ASSERT(FALSE);
        return 0;
    }

    SmartcardLockDevice(pSmartcardExtension);
    
    if (readerExtension->d_uReaderState != STATE_CLOSED)
    {
        SmartcardDebug(DEBUG_ERROR,(TEXT("Open - invalid state %d\n"),readerExtension->d_uReaderState));
        dwData = 0;
        SetLastError(ERROR_SHARING_VIOLATION);
        ASSERT(FALSE);
    }
    else
    {
        // clear card state
        memset(&pSmartcardExtension->CardCapabilities, 0, sizeof(SCARD_CARD_CAPABILITIES));
        // clear reader state
        pSmartcardExtension->ReaderCapabilities.CurrentState = SCARD_UNKNOWN;

        // save the current power state of the reader
        readerExtension->ReaderPowerState = PowerReaderWorking;

        //initialize the SIM module
        SIM_Init(readerExtension);
    
        pSmartcardExtension->ReaderExtension->TrackingThreadTerminate = FALSE;
        pSmartcardExtension->ReaderExtension->hBackgroundThread = CreateThread(NULL,0,SIMCardDetectionThread,pSmartcardExtension,0,NULL);

        Sleep(10);
        CBUpdateCardState(pSmartcardExtension);
        SmartcardDebug(DEBUG_TRACE,(TEXT("CardState %d\n"),pSmartcardExtension->ReaderCapabilities.CurrentState));

        readerExtension->d_uReaderState = STATE_OPENED;

    

    }
    
    SmartcardUnlockDevice(pSmartcardExtension);
    LeaveDevice(pSmartcardExtension);

    //may not be necessary
    //SIM_Open(readerExtension->pSIMReg);

    SmartcardDebug(DEBUG_TRACE,(TEXT("-SCR_Open\n")));
    
    return dwData;
}


//-----------------------------------------------------------------------------
//
// Function: SCR_Close
//
// This function close the device instance
//
// Parameters:
//      Handle
//          [in] context of call
//
// Returns:
//      BOOL true means success false means fail
//
//-----------------------------------------------------------------------------
BOOL SCR_Close(DWORD Handle)
{
    BOOL fRet;
    DWORD retry = 0;
    PSMARTCARD_EXTENSION   pSmartcardExtension = (PSMARTCARD_EXTENSION) Handle;

    SmartcardDebug(DEBUG_TRACE,(TEXT("+SCR_Close(%x) - entered\n"),Handle));
    
    if (!ValidateAndEnterDevice(pSmartcardExtension))
    {
        SetLastError(ERROR_BAD_DEVICE);
        return FALSE;
    }
    
    pSmartcardExtension->ReaderExtension->d_uReaderState = STATE_CLOSED;

    while (pSmartcardExtension->ReaderExtension->d_RefCount > 1 && ++retry < 3)
    {
        // cancel any outstanding blocking calls
        SmartcardDebug(DEBUG_TRACE,(TEXT("Close - waiting for %d threads to exit\n"), 
        pSmartcardExtension->ReaderExtension->d_RefCount -1));
        
        SmartcardDeviceControl(pSmartcardExtension, IOCTL_SMARTCARD_CANCEL_BLOCKING, NULL,0,NULL,0,NULL);

        //check whether need this sleep during testing
        //Sleep(10);
    }
   

   if (pSmartcardExtension->ReaderExtension->hBackgroundThread) 
    {
        DWORD dwWait;
        
        pSmartcardExtension->ReaderExtension->TrackingThreadTerminate = TRUE;
        
        dwWait = WaitForSingleObject(pSmartcardExtension->ReaderExtension->hBackgroundThread,10*POLLING_PERIOD);
        ASSERT(dwWait != WAIT_TIMEOUT);
        CloseHandle(pSmartcardExtension->ReaderExtension->hBackgroundThread);
        pSmartcardExtension->ReaderExtension->hBackgroundThread=NULL;
    }
    fRet = TRUE;

    LeaveDevice(pSmartcardExtension);

    SIM_Close(pSmartcardExtension->ReaderExtension);

    SmartcardDebug(DEBUG_TRACE,(TEXT("+SCR_Close\n")));

    return fRet;
}


//-----------------------------------------------------------------------------
//
// Function: SCR_IOControl
//
// This function responds to info, read and write control codes.
//
// Parameters:
//      Handle
//          [in] context of call
//      dwIoControlCode
//          [in] the io control code passed from upper layer
//      pInBuf
//          [in] IN buffer
//      nInBufSize
//          [in]/[out] size of IN buffer
//      pOutBuf
//          [in] OUT buffer
//      nOutBufSize
//          [in]/[out] size of IN buffer
//      pBytesReturned
//          [in]/[out] returned bytes
//
// Returns:
//      BOOL
//
//-----------------------------------------------------------------------------
BOOL SCR_IOControl(
    DWORD Handle,
    DWORD dwIoControlCode,
    PBYTE pInBuf,
    DWORD nInBufSize,
    PBYTE pOutBuf,
    DWORD nOutBufSize,
    PDWORD pBytesReturned
    )
{
    NTSTATUS status;
    PSMARTCARD_EXTENSION pSmartcardExtension = (PSMARTCARD_EXTENSION) Handle;

    SmartcardDebug(DEBUG_TRACE,(TEXT("+SCR_IOControl\n")));
    
    if (!ValidateAndEnterDevice(pSmartcardExtension))
    {
        SetLastError(ERROR_BAD_DEVICE);
        ASSERT(FALSE);
        return FALSE;
    }

    if (pSmartcardExtension->ReaderExtension->d_uReaderState != STATE_OPENED)
    {
        SmartcardDebug(DEBUG_ERROR,(TEXT("DeviceIOCTL - invalid state %d\n"),
            pSmartcardExtension->ReaderExtension->d_uReaderState));
        
        status = ERROR_GEN_FAILURE;
    }
    else 
    {
        ASSERT(pSmartcardExtension->ReaderExtension->ReaderPowerState == 
            PowerReaderWorking);
        status = SmartcardDeviceControl(
            pSmartcardExtension, dwIoControlCode,
            pInBuf, nInBufSize,
            pOutBuf, nOutBufSize,
            pBytesReturned);
    };
    LeaveDevice(pSmartcardExtension);

    SmartcardDebug(DEBUG_TRACE,(TEXT("-SCR_IOControl\n")));

    return (status == STATUS_SUCCESS ? TRUE: FALSE);
}



// The function Not supported.
DWORD SCR_Read(DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);
    return 0;
}
DWORD SCR_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);
    return 0;
}
DWORD SCR_Seek(DWORD Handle, long lDistance, DWORD dwMoveMethod)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(lDistance);
    UNREFERENCED_PARAMETER(dwMoveMethod);
    return 0;
}

// Power Up and Down Support
void SCR_PowerUp(void)
{
    SmartcardDebug(DEBUG_TRACE,(TEXT("PowerUp() - entered\n")));
}
void SCR_PowerDown(void)
{
    SmartcardDebug(DEBUG_TRACE,(TEXT("PowerDown() - entered\n")));
}


//-----------------------------------------------------------------------------
//
// Function: GetIOIndex
//
// This function obtains which port to use and the mode of operation one port mode
//
// Parameters:
//      PREADER_EXTENSION pReaderExtension : The structure containing the device information (base address, index, etc.)
//      LPTSTR ActiveKey : the driver's active key string
//
// Returns:
//      DWORD iobase address of the SIM module. 0x0 indicates failure
//
//-----------------------------------------------------------------------------
static DWORD GetIOIndex(PREADER_EXTENSION pReaderExtension,LPTSTR ActiveKey)
{
    DWORD ValType;
    DWORD ValLen;
    HKEY hCardKey;
    DWORD status;
    DWORD index;
    DWORD port_select;

    // Open active device registry key
    hCardKey = OpenDeviceKey(ActiveKey);
    if (hCardKey == INVALID_HANDLE_VALUE) 
    {
        SmartcardDebug(DEBUG_TRACE|DEBUG_ERROR,
                 (TEXT("GetIOIndex OpenDeviceKey returned %x!!!\r\n"), hCardKey));
        return 0x0;
    }

    // get our device index
    ValLen = sizeof(DWORD);
    status = RegQueryValueEx(
                hCardKey,
                SIM_REG_INDEX_VAL_NAME,
                NULL,
                &ValType,
                (LPBYTE)(&index),
                &ValLen);
    if (status != ERROR_SUCCESS) 
    {
        SmartcardDebug(DEBUG_TRACE|DEBUG_ERROR, (TEXT("GetIOIndex - RegQueryValueEx(%s) returned %d\r\n"),
                  SIM_REG_INDEX_VAL_NAME, status));
        goto error;
    }
    pReaderExtension->dwIndex = index;
    
    ValLen = sizeof(DWORD);
    status = RegQueryValueEx(
                hCardKey,
                SIM_REG_PORT_VAL_NAME,
                NULL,
                &ValType,
                (LPBYTE)(&port_select),
                &ValLen);
    if (status != ERROR_SUCCESS) 
    {
        SmartcardDebug(DEBUG_TRACE|DEBUG_ERROR, (TEXT("GetIOIndex - RegQueryValueEx(%s) returned %d\r\n"),
                  SIM_REG_PORT_VAL_NAME, status));
        goto error;
    }

    pReaderExtension->dwPort = port_select;

    RegCloseKey(hCardKey);
   
    

    return CspSIMGetBaseAddress(index);

error:
    RegCloseKey(hCardKey);
    return 0;
}
//-----------------------------------------------------------------------------
//
// Function: GetDeviceName
//
// This function obtains the device name from registry
//
// Parameters:
//      ActivePath
//          [in] pointer to the name of the subkey to open
//      szDeviceName
//          [in] pointer to the buffer that receives value
//
// Returns:
//      DWORD status
//
//-----------------------------------------------------------------------------
static DWORD GetDeviceName(LPTSTR ActivePath, LPTSTR szDeviceName)
{
    DWORD ValType;
    DWORD ValLen;
    HKEY hCardKey;
    DWORD status;

    //
    // Open active device registry key
    //
    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                ActivePath,
                0,
                0,
                &hCardKey);
    if (status) 
    {
        SmartcardDebug(DEBUG_TRACE|DEBUG_ERROR,
                 (TEXT("GetDeviceName RegOpenKeyEx(HLM\\%s) returned %d!!!\r\n"), ActivePath, status));
        return status;
    }
    // get our device name (eg. SIM1:)
    ValLen = DEVNAME_LEN*sizeof(TCHAR);
    status = RegQueryValueEx(
                hCardKey,
                DEVLOAD_DEVNAME_VALNAME,
                NULL,
                &ValType,
                (PUCHAR)szDeviceName,
                &ValLen);
    if (status != ERROR_SUCCESS) 
    {
        SmartcardDebug(DEBUG_TRACE|DEBUG_ERROR,
                 (TEXT("GetDeviceName - RegQueryValueEx(%s) returned %d\r\n"),
                  DEVLOAD_DEVNAME_VALNAME, status));
    }
    RegCloseKey(hCardKey);
    return status;
}


//-----------------------------------------------------------------------------
//
// Function: MakeFriendlyName
//
// This function makes the device friendly name
//
// Parameters:
//      SmartcardExtension
//          [in] context of device
//      szFriendlyName
//          [in] pointer to the friendly name
//
// Returns:
//      void
//
//-----------------------------------------------------------------------------
static void MakeFriendlyName(PSMARTCARD_EXTENSION SmartcardExtension, LPTSTR szFriendlyName)
{
    TCHAR szUnitNoPart[] = L" [ ]";
    szFriendlyName[0] = 0;  // NULL in case IfdType is empty
    PREFAST_SUPPRESS( 5425, "No changes required in the code" )
    mbstowcs(szFriendlyName, (const char *) SmartcardExtension->VendorAttr.IfdType.Buffer, SmartcardExtension->VendorAttr.IfdType.Length);
    szUnitNoPart[2] = (WCHAR)(SmartcardExtension->VendorAttr.UnitNo % 10 + '0');
    StringCchCat( szFriendlyName, MAXIMUM_ATTR_STRING_LENGTH+DEVNAME_LEN+5, szUnitNoPart);

}


//-----------------------------------------------------------------------------
//
// Function: SIMLoadDevice
//
// This function creates ReaderExtension stucure and calls the hardware to do initialization
//
// Parameters:
//      ActiveKey
//          [in] active key of device
//
// Returns:
//      PSMARTCARD_EXTENSION pointer to new device structure or NULL.
//
//-----------------------------------------------------------------------------
static PSMARTCARD_EXTENSION SIMLoadDevice(LPTSTR ActiveKey)
{
    NTSTATUS status;
    PREADER_EXTENSION ReaderExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;
    DWORD iobase;

    SmartcardDebug(DEBUG_TRACE,(TEXT("+SIMLoadDevice\n")));

    // allocate the device context (including smartcard and reader extension)
    SmartcardExtension = LocalAlloc(LPTR, sizeof( SMARTCARD_EXTENSION )+ sizeof(READER_EXTENSION));

    if( SmartcardExtension == NULL ) 
    {
        status = STATUS_INSUFFICIENT_RESOURCES;         
        return (NULL);
    }

    //    allocate the reader extension
    ReaderExtension = (PREADER_EXTENSION) (SmartcardExtension +1);

    if ((ReaderExtension->d_ActivePath = LocalAlloc(LPTR, wcslen(ActiveKey)*sizeof(WCHAR)+sizeof(WCHAR))) != 0)
    {
        StringCchCopy(ReaderExtension->d_ActivePath, wcslen(ActiveKey)*sizeof(WCHAR)+sizeof(WCHAR), ActiveKey);
    }
        
    ReaderExtension->d_uReaderState = STATE_CLOSED;

    SmartcardExtension->ReaderExtension = ReaderExtension;

    //    setup smartcard extension - callback's
    SmartcardExtension->ReaderFunction[RDF_CARD_POWER] = CBCardPower;
    SmartcardExtension->ReaderFunction[RDF_TRANSMIT] = CBTransmit;
    SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING] = CBCardTracking;
    SmartcardExtension->ReaderFunction[RDF_SET_PROTOCOL] = CBSetProtocol;
    SmartcardExtension->ReaderFunction[RDF_IOCTL_VENDOR] = SIMVendorIoctl;

    //    setup smartcard extension - vendor attribute
    RtlCopyMemory(SmartcardExtension->VendorAttr.VendorName.Buffer, SIM_VENDOR_NAME, sizeof(SIM_VENDOR_NAME));
    SmartcardExtension->VendorAttr.VendorName.Length = sizeof(SIM_VENDOR_NAME);

    RtlCopyMemory(SmartcardExtension->VendorAttr.IfdType.Buffer, SIM_PRODUCT_NAME, sizeof(SIM_PRODUCT_NAME));
    SmartcardExtension->VendorAttr.IfdType.Length = sizeof(SIM_PRODUCT_NAME);

    SmartcardExtension->VendorAttr.UnitNo = 0;

    SmartcardExtension->VendorAttr.IfdVersion.BuildNumber = 0;

    SmartcardExtension->VendorAttr.IfdSerialNo.Length = 0;
        
    //    setup smartcard extension - reader capabilities
    SmartcardExtension->ReaderCapabilities.SupportedProtocols = SCARD_PROTOCOL_T0;

    SmartcardExtension->ReaderCapabilities.MechProperties = 0;
    SmartcardExtension->ReaderCapabilities.Channel = 0;

    SmartcardExtension->ReaderCapabilities.CLKFrequency.Default = 4000;
    SmartcardExtension->ReaderCapabilities.CLKFrequency.Max    = 4000;

    SmartcardExtension->ReaderCapabilities.DataRate.Default = 10750;
    SmartcardExtension->ReaderCapabilities.DataRate.Max = 10750;

    //    enter correct version of the lib
    SmartcardExtension->Version = SMCLIB_VERSION;
    SmartcardExtension->SmartcardRequest.BufferSize    = MIN_BUFFER_SIZE;
    SmartcardExtension->SmartcardReply.BufferSize    = MIN_BUFFER_SIZE;

    SmartcardExtension->ReaderExtension->ReaderPowerState = PowerReaderWorking;
    
    SmartcardExtension->ReaderCapabilities.MaxIFSD = 254;
    
    status = SmartcardInitialize(SmartcardExtension);
    if (status != STATUS_SUCCESS) 
    {
            SmartcardDebug(DEBUG_ERROR,(TEXT("SmartcardInitialize failed - %x\n"), status));
            SIMUnloadDevice(SmartcardExtension);
            SmartcardExtension = NULL;
            return (NULL);
    }

    {
    // The device name should be available from the Active Key
    // [On versions prior to CE 3.0, this wont work until the post-init IOCTL]
    TCHAR szDeviceName[DEVNAME_LEN];
    TCHAR szFriendlyName[MAXIMUM_ATTR_STRING_LENGTH+DEVNAME_LEN+5];

    status = GetDeviceName(ReaderExtension->d_ActivePath,szDeviceName);
    if (status == STATUS_SUCCESS)
    {
            // figure out the unit number from the device name
            PTCHAR pch = szDeviceName;
            while (*pch && (*pch < '0' || *pch > '9'))
            ++pch;
            if (*pch)
                SmartcardExtension->VendorAttr.UnitNo = *pch - '0';
            // Attempt to register a friendly name for this device 
            // for the benefit of the resource manager. 
            MakeFriendlyName(SmartcardExtension, szFriendlyName);
            SmartcardCreateLink(szFriendlyName,szDeviceName); 
    }
    }

    //determine which port to use and the mode of operation
    iobase = GetIOIndex(ReaderExtension,ActiveKey);
    if (iobase == 0)
    {
            SmartcardDebug(DEBUG_ERROR,(TEXT("SmartcardInitialize failed - %x\n"), status));
            SIMUnloadDevice(SmartcardExtension);
            SmartcardExtension = NULL;
    }
    
    //map the virtual address for SIM register structure
    if (SmartcardExtension)
    {
        SmartcardExtension->ReaderExtension->pSIMReg = (PCSP_SIM_REG)SIM_InternalMapRegisterAddresses(iobase);
    }

    SmartcardDebug(DEBUG_TRACE, (TEXT("SIMCreateDevice: Exit %x\n"), status ));

    SmartcardDebug(DEBUG_TRACE,(TEXT("-SIMLoadDevice\n")));

    return SmartcardExtension;
    
} 


//-----------------------------------------------------------------------------
//
// Function: SIMUnloadDevice
//
// This function unloads the device from lib
//
// Parameters:
//      SmartcardExtension
//          [in]call of context
//
// Returns:
//      void
//
//-----------------------------------------------------------------------------
static void SIMUnloadDevice(PSMARTCARD_EXTENSION SmartcardExtension)
{
    WCHAR szFriendlyName[MAXIMUM_ATTR_STRING_LENGTH+DEVNAME_LEN+5];
    PREADER_EXTENSION ReaderExtension = (PREADER_EXTENSION) (SmartcardExtension +1);
    DWORD retry = 0;

    SmartcardDebug(DEBUG_TRACE,(TEXT("+SIMUnloadDevice\n")));
    if(SmartcardExtension)
    {
        // remove device from list of active devices managed by this driver
        RemoveDevice(SmartcardExtension);
        
        // At this point no other thread should have a reference to this object
        // make sure that's the case
        while (SmartcardExtension->ReaderExtension->d_RefCount && retry++ < 3)
        {
            Sleep(retry * 32);
            SmartcardDebug(DEBUG_TRACE, (TEXT("UnloadDevice - waiting for refCount %d to drop to zero\r\n"),
            SmartcardExtension->ReaderExtension->d_RefCount));
        }
        ASSERT(!SmartcardExtension->ReaderExtension->d_RefCount);

        // Remove friendly name
        MakeFriendlyName(SmartcardExtension, szFriendlyName);
        SmartcardDeleteLink(szFriendlyName);
        
        // sign off from SMCLIB
        SmartcardExit(SmartcardExtension);

        //Unmap the register mapping
        SIM_InternalUnMapRegisterAddresses(SmartcardExtension->ReaderExtension->pSIMReg);
        
        SmartcardDebug(DEBUG_TRACE, (TEXT("%s:UnloadDevice - freeing resources\r\n")));
        if(ReaderExtension->d_ActivePath)
        {
            LocalFree(ReaderExtension->d_ActivePath);
            ReaderExtension->d_ActivePath=NULL;
        }
        LocalFree(SmartcardExtension);
        SmartcardExtension = NULL;
    }
    SmartcardDebug(DEBUG_TRACE,(TEXT("-SIMUnloadDevice\n")));
}

