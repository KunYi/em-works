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

//
// This module contains a sample Notification LED (NLed) driver for windows CE.  It
// exposes its interface to the system via a set of IOCTLs which device.exe
// invokes on its behalf when one of the NLed APIs are invoked.  The
// following APIs are modeled in this way:
//
//      NLedGetDeviceInfo
//      NLedSetDevice
//
// When it initializes, this driver is responsible for setting the NLed
// API ready event to indicate it is ready to receive IOCTLs in response
// to these APIs.
// 
// This example driver comes with a stubbed-out implementation of the
// low-level NLed interfaces.  OEMs that wish to produce their own
// driver can write a module containing the low-level entry points
// in their platform and simply link with the library containing this module.
// The linker will bring in the rest of the driver as an MDD.
//
// The low-level interfaces necessary that must be overridden to produce
// a platform-specific PDD are:
//
//      NLedDriverPowerDown
//      NLedDriverGetDeviceInfo
//      NLedDriverSetDevice
//      NLedDriverInitialize
//      NLedDriverDeInitialize
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#define NLED_DATA_DEF    // global NLED driver variables defined here

#include <windows.h>
#include <nled.h>
#include <led_drvr.h>

// global variables
CRITICAL_SECTION gcsNLed;
PFN_NLED_PDD_IOCONTROL gpfnNLedPddIOControl;
BOOL gfDriverLoaded;

// This routine initializes the Notification LED driver.
DWORD Init(PVOID Context)
{
    DWORD dwHandle = 0;     // assume failure
    HANDLE hevReady;
    UNREFERENCED_PARAMETER(Context);
    SETFNAME(_T("NLedDrvr: Init"));
    DEBUGMSG(ZONE_INIT, (_T("%s: invoked w/ context 0x%08x\r\n"), pszFname, Context));

    InitializeCriticalSection(&gcsNLed);

    // have we already been loaded?
    LOCKNLED();
    if(gfDriverLoaded) 
    {
        UNLOCKNLED();
        goto done;
    } 
    else 
    {
        gfDriverLoaded = TRUE;
        UNLOCKNLED();
    }

    // get a handle to our API event
    hevReady = OpenEvent(EVENT_ALL_ACCESS, FALSE, NLED_API_EVENT_NAME);
    if(hevReady == NULL) 
    {
        DEBUGMSG(ZONE_ERROR || ZONE_INIT, (_T("%s: fatal error: can't open API event\r\n"), pszFname));
        goto done;
    }

    // no, initialize global variables
    gpfnNLedPddIOControl = NULL;     // must be initialized by the PDD
    if(!NLedDriverInitialize()) 
    {
        DEBUGMSG(ZONE_ERROR || ZONE_INIT, (_T("%s: NLedDriverInitialize() failed\r\n"), pszFname));
    } 
    else 
    {
        // notify the world that we're up and running
        SetEvent(hevReady);
        // return success
        dwHandle = 1;
    }
    
    // don't need to keep this handle open
    CloseHandle(hevReady);
        
done:
    
    DEBUGMSG(ZONE_INIT, (_T("%s: returning %d\r\n"), pszFname, dwHandle));
    return dwHandle;
}


BOOL Deinit(DWORD dwContext)
{
    UNREFERENCED_PARAMETER(dwContext);
    DEBUGMSG(ZONE_INIT, (_T("NLedDrvr: Deinit: invoked w/ context 0x%08x\r\n"),dwContext));
    DEBUGCHK(dwContext == 1);
    
    // notify the PDD
    NLedDriverDeInitialize();

    // clean up globals
    gpfnNLedPddIOControl = NULL;
    LOCKNLED();
    gfDriverLoaded = FALSE;
    UNLOCKNLED();
    DeleteCriticalSection(&gcsNLed);
    DEBUGMSG(ZONE_INIT, (_T("NLedDrvr: Deinit: all done\r\n")));
    return TRUE;
}


BOOL IOControl(
              DWORD  dwContext,
              DWORD  Ioctl,
              PUCHAR pInBuf,
              DWORD  InBufLen, 
              PUCHAR pOutBuf,
              DWORD  OutBufLen,
              PDWORD pdwBytesTransferred
              )
{
    BOOL   fOk = TRUE;
    SETFNAME(_T("NLED::IOControl"));

    DEBUGMSG(ZONE_FUNCTION, (_T("%s: IOCTL:0x%x, InBuf:0x%x, InBufLen:%d, OutBuf:0x%x, OutBufLen:0x%x)\r\n"),
        pszFname, Ioctl, pInBuf, InBufLen, pOutBuf, OutBufLen));

    switch (Ioctl) 
    {
        case IOCTL_NLED_GETDEVICEINFO:
        // sanity check parameters
            if(pInBuf != NULL && InBufLen == sizeof(UINT)) 
            {
                UINT nInfoId = 0;

                nInfoId = *((UINT *) pInBuf);
                DWORD dwExpectedSize;

                // figure out what size buffer to expect
                switch(nInfoId) 
                {
                    case NLED_COUNT_INFO_ID:
                        dwExpectedSize = sizeof(NLED_COUNT_INFO);
                        break;
                    case NLED_SUPPORTS_INFO_ID:
                        dwExpectedSize = sizeof(NLED_SUPPORTS_INFO);
                        break;
                    case NLED_SETTINGS_INFO_ID:
                        dwExpectedSize = sizeof(NLED_SETTINGS_INFO);
                        break;
                    default:
                        DEBUGMSG(ZONE_WARN, (_T("%s: Unexpected nInfoId %d\r\n"), pszFname, nInfoId));
                        dwExpectedSize = 0;
                        fOk = FALSE;
                        break;
                }

            // did we get a valid output buffer?
            if(pOutBuf == NULL || OutBufLen != dwExpectedSize) 
            {
                fOk = FALSE;
            }


            // call the PDD?
            if(fOk) 
            {
                LOCKNLED();
                fOk = NLedDriverGetDeviceInfo(nInfoId, pOutBuf);
                UNLOCKNLED();
            }
        }
            break;
        
        case IOCTL_NLED_SETDEVICE:
            // sanity check parameters
            if(pOutBuf == NULL && OutBufLen == 0
            && pInBuf != NULL && InBufLen == sizeof(NLED_SETTINGS_INFO) && pdwBytesTransferred == NULL) 
            {
                // update the LEDs
                LOCKNLED();
                fOk = NLedDriverSetDevice(NLED_SETTINGS_INFO_ID, pInBuf);
                UNLOCKNLED();
            }
            break;
        
        default:
        {
            DWORD dwErr;

            // Pass through to the PDD if enabled.  Note that the PDD function returns an error code,
            // not a boolean.
            if(gpfnNLedPddIOControl != NULL) 
            {
                dwErr = gpfnNLedPddIOControl(dwContext, Ioctl, pInBuf, InBufLen, pOutBuf, OutBufLen, pdwBytesTransferred);
                DEBUGMSG(ZONE_IOCTL || ZONE_WARN, (_T("%s: PDD IOControl(%u) returned %d\r\n"), pszFname, Ioctl, dwErr));
            } 
            else 
            {
                DEBUGMSG(ZONE_IOCTL || ZONE_WARN, (_T("%s: Unsupported IOCTL code %u\r\n"), pszFname, Ioctl));
                dwErr = ERROR_NOT_SUPPORTED;
            }
            if(dwErr != ERROR_SUCCESS) 
            {
                SetLastError(dwErr);
                fOk = FALSE;
            }
        }
            break;
    }
    
    DEBUGMSG(ZONE_FUNCTION || ZONE_IOCTL || (!fOk && ZONE_WARN), (_T("%s: returning %d, error %d\r\n"),pszFname, fOk, GetLastError()));
    return fOk;
}

VOID PowerDown(DWORD dwContext)
{
    UNREFERENCED_PARAMETER(dwContext);
    // notify the PDD
    NLedDriverPowerDown(TRUE);
}


VOID PowerUp(DWORD dwContext)
{
    UNREFERENCED_PARAMETER(dwContext);
    // notify the PDD
    NLedDriverPowerDown(FALSE);
}


DWORD Open(DWORD Context, DWORD Access,DWORD ShareMode)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("Open(%x, 0x%x, 0x%x)\r\n"),Context, Access, ShareMode));
    DEBUGCHK(Context == 1);
    
    UNREFERENCED_PARAMETER(Access);
    UNREFERENCED_PARAMETER(ShareMode);
    
    // pass back the device handle
    return Context;     // 0 indicates failure
}


BOOL  Close(DWORD Context) 
{
    UNREFERENCED_PARAMETER(Context);
    DEBUGMSG(ZONE_FUNCTION,(_T("Close(%x)\r\n"), Context));
    DEBUGCHK(Context == 1);
    
    return TRUE;
}


BOOL WINAPI DllEntry(HANDLE hDllHandle, DWORD  dwReason, LPVOID lpreserved) 
{
    BOOL bRc = TRUE;
    
    UNREFERENCED_PARAMETER(hDllHandle);
    UNREFERENCED_PARAMETER(lpreserved);
    
    switch (dwReason) 
    {
        case DLL_PROCESS_ATTACH: 
        {
            DEBUGREGISTER((HINSTANCE) hDllHandle);
            DEBUGMSG(ZONE_INIT,(_T("*** DLL_PROCESS_ATTACH - Current Process: 0x%x, ID: 0x%x ***\r\n"),GetCurrentProcess(), GetCurrentProcessId()));
            DisableThreadLibraryCalls((HMODULE) hDllHandle);
            InitializeCriticalSection(&gcsNLed);
        } 
            break;
        
        case DLL_PROCESS_DETACH: 
        {
            DEBUGMSG(ZONE_INIT,(_T("*** DLL_PROCESS_DETACH - Current Process: 0x%x, ID: 0x%x ***\r\n"),GetCurrentProcess(), GetCurrentProcessId()));
            DeleteCriticalSection(&gcsNLed);
        } 
            break;
        
        default:
            break;
    }
    
    return bRc;
}




