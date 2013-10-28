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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// Copyright (C) 2006,2007 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  gptmdd.c
//
//  This module provides a stream interface for the GPT
//  driver.  Client drivers can use the stream interface to
//  configure the GPT driver and run test programs.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#pragma warning(pop)

#include "mxarm11.h"
#include "gpt.h"
#include "gpt_priv.h"


//------------------------------------------------------------------------------
// External Functions
extern BOOL BSPGptSetClockGatingMode(BOOL);

//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
#ifdef DEBUG
extern DBGPARAM dpCurSettings =
{
    _T("GPT"),
    {
        _T("Init"), _T("DeInit"), _T("Ioctl"), _T(""),
        _T(""), _T(""), _T(""), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN
};
#endif


//------------------------------------------------------------------------------
// Local Variables
static BOOL currentlyOwned = FALSE;


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: GPT_Init
//
// The Device Manager calls this function as a result of a call to the
// ActivateDevice() function.
//
// Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//          active key for the stream interface driver.
//
// Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//------------------------------------------------------------------------------
DWORD GPT_Init(LPCTSTR pContext)
{
    GPT_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);

    if (!GptInitialize())
        return 0;

    GPT_FUNCTION_EXIT();

    return 1;

}


//------------------------------------------------------------------------------
//
// Function: GPT_Deinit
//
// This function uninitializes a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL GPT_Deinit(DWORD hDeviceContext)
{
    GPT_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);

    GptRelease();

    GPT_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GPT_Open
//
// This function opens a device for reading, writing, or both.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context. The XXX_Init function creates
//          and returns this handle.
//
//      AccessCode
//          [in] Access code for the device. The access is a combination of
//          read and write access from CreateFile.
//
//      ShareMode
//          [in] File share mode of the device. The share mode is a
//          combination of read and write access sharing from CreateFile.
//
// Returns:
//      This function returns a handle that identifies the open context of
//      the device to the calling application.
//
//------------------------------------------------------------------------------
DWORD GPT_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    GPT_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    if (currentlyOwned)
    {
        return 0;
    }
    else
    {
        currentlyOwned = TRUE;
    }

    if (!BSPGptSetClockGatingMode(TRUE))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error setting GPT clock mode.\r\n"), __WFUNCTION__));
        return 0;
    }

    GPT_FUNCTION_EXIT();
    return hDeviceContext;
}


//------------------------------------------------------------------------------
//
// Function: GPT_Close
//
// This function closes the GPT for reading and writing.
//
// Parameters:
//      hOpenContext
//          [in] Handle returned by the XXX_Open function, used to identify
//          the open context of the device.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL GPT_Close(DWORD hOpenContext)
{
    GPT_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    currentlyOwned = FALSE;

    if (!BSPGptSetClockGatingMode(FALSE))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error setting GPT clock mode.\r\n"), __WFUNCTION__));
        return 0;
    }

    GPT_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GPT_PowerDown
//
// This function suspends power to the device. It is useful only with
// devices that can power down under software control.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void GPT_PowerDown(DWORD hDeviceContext)
{
    GPT_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);

    // Just clock gate the GPT when powering down.
    BSPGptSetClockGatingMode(FALSE);

    GPT_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: GPT_PowerUp
//
// This function restores power to a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void GPT_PowerUp(void)
{
    GPT_FUNCTION_ENTRY();

    // Upon return from sleep or power off state, reinitialize GPT registers.
    //
    // Restore the clock only if GPT was under use during power down.

    if(currentlyOwned)
    {
        BSPGptSetClockGatingMode(TRUE);
    }
    else
    {
        BSPGptSetClockGatingMode(FALSE);
    }

    GPT_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: GPT_Read
//
// This function reads data from the device identified by the open
// context.
//
// Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//          function creates and returns this identifier.
//
//      pBuffer
//          [out] Pointer to the buffer that stores the data read from the
//          device. This buffer should be at least Count bytes long.
//
//      Count
//          [in] Number of bytes to read from the device into pBuffer.
//
// Returns:
//      Returns zero to indicate end-of-file. Returns -1 to indicate an
//      error. Returns the number of bytes read to indicate success.
//
//------------------------------------------------------------------------------
DWORD GPT_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    GPT_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(Count);

    GPT_FUNCTION_EXIT();
    return 0;
}


//------------------------------------------------------------------------------
//
// Function: GPT_Write
//
// This function writes data to the device.
//
// Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//          function creates and returns this identifier.
//
//      pBuffer
//          [out] Pointer to the buffer that contains the data to write.
//
//      Count
//          [in] Number of bytes to write from the pBuffer buffer into the
//          device.
//
// Returns:
//      The number of bytes written indicates success. A value of -1 indicates
//      failure.
//
//------------------------------------------------------------------------------
DWORD GPT_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    GPT_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

    GPT_FUNCTION_EXIT();
    return 0;
}


//------------------------------------------------------------------------------
//
// Function: GPT_Seek
//
// This function moves the data pointer in the device.
//
// Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//          function creates and returns this identifier.
//
//      Amount
//          [in] Number of bytes to move the data pointer in the device.
//          A positive value moves the data pointer toward the end of the
//          file, and a negative value moves it toward the beginning.
//
//      Type
//          [in] Starting point for the data pointer.
//
// Returns:
//      The new data pointer for the device indicates success. A value of -1
//      indicates failure.
//
//------------------------------------------------------------------------------
DWORD GPT_Seek(DWORD hOpenContext, long Amount, WORD Type)
{
    GPT_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(Amount);
    UNREFERENCED_PARAMETER(Type);

    GPT_FUNCTION_EXIT();
   return (DWORD) -1;
}


//------------------------------------------------------------------------------
//
// Function: GPT_IOControl
//
// This function sends a command to a device.
//
// Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//          function creates and returns this identifier.
//
//      dwCode
//          [in] I/O control operation to perform. These codes are
//          device-specific and are usually exposed to developers through
//          a header file.
//
//      pBufIn
//          [in] Pointer to the buffer containing data to transfer to the
//          device.
//
//      dwLenIn
//          [in] Number of bytes of data in the buffer specified for pBufIn.
//
//      pBufOut
//          [out] Pointer to the buffer used to transfer the output data
//          from the device.
//
//      dwLenOut
//          [in] Maximum number of bytes in the buffer specified by pBufOut.
//
//      pdwActualOut
//          [out] Pointer to the DWORD buffer that this function uses to
//          return the actual number of bytes received from the device.
//
// Returns:
//      The new data pointer for the device indicates success. A value of -1
//      indicates failure.
//
//------------------------------------------------------------------------------
BOOL GPT_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    BOOL bRet = FALSE;
    LPTSTR pStr = NULL;
    pGPT_Config pTimerConfig;
    PDWORD pGptPeriod;
    PDWORD pGptCount;
    DWORD GptPeriod;
    UINT32 period;
    timerMode_c timerMode;
    size_t cb = 0;
    DWORD dwErr = ERROR_SUCCESS;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pdwActualOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(hOpenContext);

    switch(dwCode)
    {
        case GPT_IOCTL_TIMER_CREATE_EVENT:
        {
            // WinCE 6.00 no longer supports MapCallerPtr(). However, we can
            // just do a simple assigment here because pointer arguments are
            // automatically checked for valid access in WinCE 6.00.
            if(pBufIn != NULL)
            {
                if (SUCCEEDED(StringCbLengthW((LPCTSTR)pBufIn, STRSAFE_MAX_CCH * sizeof(WCHAR), &cb)))
                {
                    if(dwLenIn < cb)
                    {
                        DEBUGMSG(ZONE_ERROR, (TEXT("GPT_IOCTL_TIMER_CREATE_EVENT: Invalid Length\r\n")));
                        dwErr = ERROR_INVALID_PARAMETER;
                        break;
                    }
                }
                else
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT("GPT_IOCTL_TIMER_CREATE_EVENT: StringCbLengthW failed\r\n")));
                    dwErr = GetLastError();
                    break;
                }
            }
            else
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("GPT_IOCTL_TIMER_CREATE_EVENT: Invalid Input Buffer\r\n")));
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            __try
            {
                pStr = (LPTSTR)pBufIn;
                bRet = GptTimerCreateEvent(pStr);
                if(!bRet)
                    dwErr = GetLastError();
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE,(_T("Exception in GPT")
                                _T("GPT_IOCTL_TIMER_CREATE_EVENT \r\n")));
                dwErr = GetExceptionCode();
                break;
            }
            DEBUGMSG(ZONE_IOCTL, (TEXT("GPT_IOControl: GPT_IOCTL_TIMER_CREATE_EVENT occurred\r\n")));
            break;
        }

        case GPT_IOCTL_TIMER_RELEASE_EVENT:
        {
            // WinCE 6.00 no longer supports MapCallerPtr(). However, we can
            // just do a simple assigment here because pointer arguments are
            // automatically checked for valid access in WinCE 6.00.
            if(pBufIn != NULL)
            {
                if (SUCCEEDED(StringCbLengthW((LPCTSTR)pBufIn, STRSAFE_MAX_CCH * sizeof(WCHAR), &cb)))
                {
                    if(dwLenIn < cb)
                    {
                        DEBUGMSG(ZONE_ERROR, (TEXT("GPT_IOCTL_TIMER_RELEASE_EVENT: Invalid Length\r\n")));
                        dwErr = ERROR_INVALID_PARAMETER;
                        break;
                    }
                }
                else
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT("GPT_IOCTL_TIMER_RELEASE_EVENT: StringCbLengthW failed\r\n")));
                    dwErr = GetLastError();
                    break;
                }
            }
            else
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("GPT_IOCTL_TIMER_RELEASE_EVENT: Invalid Input Buffer\r\n")));
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            __try
            {
                pStr = (LPTSTR)pBufIn;
                bRet = GptTimerReleaseEvent(pStr);
                if(!bRet)
                    dwErr = GetLastError();
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE,(_T("Exception in GPT")
                                _T("GPT_IOCTL_TIMER_RELEASE_EVENT \r\n")));
                dwErr = GetExceptionCode();
                break;
            }
            DEBUGMSG(ZONE_IOCTL, (TEXT("GPT_IOControl: GPT_IOCTL_TIMER_CLOSE_HANDLE occurred\r\n")));
            break;
        }

        case GPT_IOCTL_TIMER_START:
        {
            if ((pBufIn == NULL) || (dwLenIn != sizeof(GPT_Config)))
            {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            __try
            {
                // access check and marshell the input buffer
                pTimerConfig = (pGPT_Config) pBufIn;
                timerMode = pTimerConfig->timerMode;
                period = pTimerConfig->period;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE,(_T("Exception in GPT")
                                _T("GPT_IOCTL_TIMER_START \r\n")));
                dwErr = GetExceptionCode();
                break;
            }
            // In restart mode, period should not be 0
            if (timerMode == timerModePeriodic){
                if (0 == period){
                    dwErr = ERROR_INVALID_PARAMETER;
                    break;
                }
            }

            if (GptResetTimer()){
                // set operating mode and period for GPT
                if (GptSetTimerMode(timerMode)){
                    if (GptUpdateTimerPeriod((DWORD)period)){
                        GptEnableTimerInterrupt();
                        if (GptEnableTimer()){
                            bRet = TRUE;
                        }
                    }
                }
            }
            DEBUGMSG(ZONE_IOCTL, (TEXT("GPT_IOControl: GPT_IOCTL_TIMER_START occurred\r\n")));
            break;
        }

        case GPT_IOCTL_TIMER_STOP:
        {
            // disable GPT module
            GptDisableTimer();
            // disable interrpt
            GptDisableTimerInterrupt();
            bRet = TRUE;
            DEBUGMSG(ZONE_IOCTL, (TEXT("GPT_IOControl: GPT_IOCTL_TIMER_STOP occurred\r\n")));
            break;
        }

        case GPT_IOCTL_TIMER_UPDATE_PERIOD:
        {
            if ((pBufIn == NULL) || (dwLenIn != sizeof(DWORD)))
            {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            __try
            {
                // access check input buffer
                pGptPeriod = (PDWORD)pBufIn;
                GptPeriod = *pGptPeriod;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE,(_T("Exception in GPT")
                                _T("GPT_IOCTL_TIMER_UPDATE_PERIOD \r\n")));
                dwErr = GetExceptionCode();
                break;
            }
            // update compare reg1
            bRet = GptUpdateTimerPeriod(GptPeriod);
            DEBUGMSG(ZONE_IOCTL, (TEXT("GPT_IOControl: GPT_IOCTL_TIMER_UPDATE_PERIOD occurred\r\n")));
            break;
        }

        case GPT_IOCTL_TIMER_GET_COUNT:
        {
            if ((pBufOut == NULL) || (dwLenOut != sizeof(DWORD)))
            {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            __try
            {
                // access check input buffer
                pGptCount = (PDWORD)pBufOut;
                // get GPT main counter value
                bRet = GptGetTimerCount(pGptCount);
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE,(_T("Exception in GPT")
                                _T("GPT_IOCTL_TIMER_GET_COUNT \r\n")));
                dwErr = GetExceptionCode();
                break;
            }
            DEBUGMSG(ZONE_IOCTL, (TEXT("GPT_IOControl: GPT_IOCTL_TIMER_UPDATE_PERIOD occurred\r\n")));
            break;
        }

        case GPT_IOCTL_TIMER_RESUME:
        {
            // enable interrupt
            GptEnableTimerInterrupt();
            // enable GPT module
            bRet = GptEnableTimer();
            DEBUGMSG(ZONE_IOCTL, (TEXT("GPT_IOControl: GPT_IOCTL_TIMER_RESUME occurred\r\n")));
            break;
        }

        case GPT_IOCTL_TIMER_CHANGE_CLKSRC:
        {
            if ((pBufIn == NULL) || (dwLenIn != sizeof(GPT_TIMER_SRC_PKT)))
            {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            GptChangeClkSrc((PGPT_TIMER_SRC_PKT) pBufIn);
            dwErr = GetLastError();
            if (dwErr == ERROR_SUCCESS)
            {
                bRet = TRUE;
            }
            else
            {
                bRet = FALSE;
            }
            DEBUGMSG(ZONE_IOCTL, (TEXT("GPT_IOControl: GPT_IOCTL_TIMER_CHANGE_CLKSRC occurred\r\n")));
            break;
        }

        case GPT_IOCTL_TIMER_SHOW_CLKSRC:
        {
            GptShowClkSrc();
            bRet = TRUE;
            DEBUGMSG(ZONE_IOCTL, (TEXT("GPT_IOControl: GPT_IOCTL_TIMER_SHOW_CLKSRC occurred\r\n")));
            break;
        }

        default:
            dwErr = ERROR_NOT_SUPPORTED;
            DEBUGMSG(ZONE_WARN, (TEXT("GPT_IOControl: No matching IOCTL.\r\n")));
            break;
    }
    SetLastError(dwErr);
    return bRet;
}

BOOL WINAPI DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            //Register Debug Zones
            DEBUGREGISTER((HINSTANCE) hInstDll);
            DEBUGMSG(ZONE_INFO, (TEXT("GPT_DllEntry: DLL_PROCESS_ATTACH lpvReserved(0x%x)\r\n"),lpvReserved));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INFO, (TEXT("GPT_DllEntry: DLL_PROCESS_DETACH lpvReserved(0x%x)\r\n"),lpvReserved));
            break;
    }
    // Return TRUE for success
    return TRUE;
}
