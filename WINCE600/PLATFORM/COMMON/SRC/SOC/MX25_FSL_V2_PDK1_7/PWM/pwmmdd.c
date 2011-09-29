//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//-----------------------------------------------------------------------------
//
//  File:  pwmmdd.c
//
//  This module provides a stream interface for the PWM
//  driver.  Client drivers can use the stream interface to
//  configure the PWM driver and run test programs.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)
//#include <Devload.h>
#include "csp.h"
#include "pwmdefs.h"
#include "pwm.h"


//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------
extern BOOL BSPPwmSetClockGatingMode(PVOID pContext, BOOL startClocks);
extern BOOL PwmGetRegistryValues(LPCTSTR pContext);
extern BOOL PwmOpen(DWORD dwIndex);

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
#ifdef DEBUG
extern DBGPARAM dpCurSettings =
{
    _T("PWM"),
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
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//
// Function: PWM_Init
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
DWORD PWM_Init(LPCTSTR pContext)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PWM_Init\r\n")));

    return PwmInitialize(pContext);
}


//------------------------------------------------------------------------------
//
// Function: PWM_Deinit
//
// This function uninitializes a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      TRUE indicates success.
//
//------------------------------------------------------------------------------
BOOL PWM_Deinit(DWORD hDeviceContext)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PWM_Deinit\r\n")));

    PwmDeinit((PVOID) hDeviceContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PWM_Deinit\r\n")));

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: PWM_Open
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
DWORD PWM_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    PPWM_OPEN_CONTEXT pPwmOpen = 0;
    LPWSTR DeviceName;

    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PWM_Open\r\n")));

    //Allocate for the Open structure
    pPwmOpen = (PPWM_OPEN_CONTEXT) LocalAlloc(LPTR, sizeof(PWM_OPEN_CONTEXT));

    pPwmOpen->pPwm = (PPWM_DEVICE_CONTEXT) hDeviceContext;

    switch(pPwmOpen->pPwm->dwIndex)
    {
        case 1:
            DeviceName = _T("PWM1:");
            pPwmOpen->pPwm->CurrentDx = D0;
            break;
        case 2:
            DeviceName = _T("PWM2:");
            pPwmOpen->pPwm->CurrentDx = D0;
            break;
        case 3:
            DeviceName = _T("PWM3:");
            pPwmOpen->pPwm->CurrentDx = D0;
            break;
        case 4:
            DeviceName = _T("PWM4:");
            pPwmOpen->pPwm->CurrentDx = D0;
            break;
        default:
            DeviceName = 0;
            pPwmOpen->pPwm->CurrentDx = PwrDeviceUnspecified;
            return 0;
            break;
    }


    if(!PwmOpen(pPwmOpen->pPwm->dwIndex))
    {
        return 0;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PWM_Open\r\n")));
    return (DWORD) pPwmOpen;
}


//------------------------------------------------------------------------------
//
// Function: PWM_Close
//
// This function closes the PWM for reading and writing.
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
BOOL PWM_Close(DWORD hOpenContext)
{
    PPWM_OPEN_CONTEXT pPwmOpen = (PPWM_OPEN_CONTEXT) hOpenContext;
    WORD DeviceName[6];
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PWM_Close\r\n")));

    switch(pPwmOpen->pPwm->dwIndex)
    {
        case 1:
            memcpy(DeviceName, _T("PWM1:"), 6);
            pPwmOpen->pPwm->CurrentDx = D4;
            break;
        case 2:
            memcpy(DeviceName, _T("PWM2:"), 6);
            pPwmOpen->pPwm->CurrentDx = D4;
            break;
        case 3:
            memcpy(DeviceName, _T("PWM3:"), 6);
            pPwmOpen->pPwm->CurrentDx = D4;
            break;
        case 4:
            memcpy(DeviceName, _T("PWM4:"), 6);
            pPwmOpen->pPwm->CurrentDx = D4;
            break;
        default:
            memset(DeviceName, 0, 6);
            pPwmOpen->pPwm->CurrentDx = PwrDeviceUnspecified;
            return 0;
            break;
    }

	//
	// CS&ZHL AUG-17-2011: always stop PWM output before close
	//
	PwmStop(pPwmOpen->pPwm);

    LocalFree(pPwmOpen);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PWM_Close\r\n")));
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: PWM_PowerDown
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
void PWM_PowerDown(DWORD hDeviceContext)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PWM_PowerDown\r\n")));
    UNREFERENCED_PARAMETER(hDeviceContext);
   
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PWM_PowerDown\r\n")));
}


//------------------------------------------------------------------------------
//
// Function: PWM_PowerUp
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
void PWM_PowerUp(DWORD hDeviceContext)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PWM_PowerUp\r\n")));
    PwmReset((PVOID) hDeviceContext);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PWM_PowerUp\r\n")));
}


//------------------------------------------------------------------------------
//
// Function: PWM_Read
//
// This function reads contents of 6 PWM registers.
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
DWORD PWM_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    PPWM_OPEN_CONTEXT pPwmOpen = (PPWM_OPEN_CONTEXT) hOpenContext;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PWM_Read\r\n")));

    if(!PwmReadRegister((PVOID)pPwmOpen, (UINT32 *)pBuffer, Count))
        return (DWORD) (-1);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PWM_Read\r\n")));
    return Count;
}


//------------------------------------------------------------------------------
//
// Function: PWM_Write
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
//      dwNumBytes
//          [in] Number of bytes to write from the pBuffer buffer into the
//          device.
//
// Returns:
//      The number of bytes written indicates success. A value of -1 indicates
//      failure.
//
//------------------------------------------------------------------------------
DWORD PWM_Write(DWORD hOpenContext, LPCVOID pBuffer, DWORD dwNumBytes)
{
    PPWM_OPEN_CONTEXT		pPwmOpen = (PPWM_OPEN_CONTEXT) hOpenContext;
    DWORD								ret = dwNumBytes;
	PwmSample_t					PwmSample;
	PPWMINFO						pPWMInfo = (PPWMINFO)pBuffer;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PWM_Write\r\n")));

	if(!pBuffer || (dwNumBytes != sizeof(PWMINFO)))
	{
		//invalide input parameters
		RETAILMSG(1, (TEXT("PWM_Write::invalide input parameters\r\n")));
        return (DWORD)(-1);
	}

	if(!pPWMInfo->dwFreq)
	{
		//nothing to do
		RETAILMSG(1, (TEXT("PWM_Write::nothing to do as PWM freq = 0\r\n")));
        return (DWORD)(-1);
	}

	//
	// CS&ZHL AUG-17-2011: convert ...
	//
	pPwmOpen->pPwm->dwPreScaler = PwmInfo2PwmSample((PPWMINFO)pBuffer, &PwmSample);	

   // dwNumBytes is the number of pwm samples in the buffer
    if(!PwmPlaySample((PVOID)pPwmOpen->pPwm, (LPCVOID)&PwmSample, sizeof(PwmSample_t)))
    {
        dwNumBytes = 0;
        return (DWORD)(-1);
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PWM_Write\r\n")));
    return ret;
}

//------------------------------------------------------------------------------
//
// Function: PWM_IOControl
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
//      TRUE indicate success, FALSE indicate failure.
//
//------------------------------------------------------------------------------
BOOL PWM_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                                      DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                                      PDWORD pdwActualOut)
{
    PPWM_OPEN_CONTEXT pPwmOpen = (PPWM_OPEN_CONTEXT) hOpenContext;
    BOOL bRet = FALSE;

    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(pBufIn);
    

    switch(dwCode)
    {
        case PWM_IOCTL_RESET:
            PwmReset(pPwmOpen->pPwm);
            bRet = TRUE;
            DEBUGMSG(ZONE_IOCTL, (TEXT("PWM_IOControl: PWM_IOCTL_RESET occurred\r\n")));
            break;
        case IOCTL_POWER_CAPABILITIES:
            if (!pBufOut || dwLenOut < sizeof(POWER_CAPABILITIES) || !pdwActualOut)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                bRet = FALSE;
            }
            else 
            {
                PPOWER_CAPABILITIES power = (PPOWER_CAPABILITIES)pBufOut;
                memset(power, 0, sizeof(POWER_CAPABILITIES));
                SetPwmPower(power);          // get configurations from PDD layer
                *pdwActualOut = sizeof(POWER_CAPABILITIES);
                bRet = TRUE;
            }
            break;

        case IOCTL_POWER_GET:
            if(pBufOut != NULL && dwLenOut == sizeof(CEDEVICE_POWER_STATE))
            {
                *(PCEDEVICE_POWER_STATE)pBufOut = pPwmOpen->pPwm->CurrentDx;
                bRet = TRUE;
            }
            break;
        case IOCTL_POWER_SET:
            if(pBufOut != NULL && dwLenOut == sizeof(CEDEVICE_POWER_STATE))
            {
                CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pBufOut;
                if(VALID_DX(NewDx))
                {
                    if(NewDx == D1) 
                    {
                        NewDx = D0;
                    }
                    if(NewDx == D3) 
                    {
                        NewDx = D4;    // turn off
                    }
                    if(NewDx == D0) 
                    {
                        BSPPwmSetClockGatingMode(pPwmOpen->pPwm, TRUE);
                    }
                    if(NewDx == D4) 
                    {
                        BSPPwmSetClockGatingMode(pPwmOpen->pPwm, FALSE);                                                                        
                    }
                    pPwmOpen->pPwm->CurrentDx = NewDx;
                    bRet = TRUE;
                }
                else
                {
                    DEBUGMSG(ZONE_WARN, (TEXT("DUM_IOControl: IOCTL_POWER_SET: invalid state request %u\r\n"), NewDx));
                }
            }
            break; 
        default:
            DEBUGMSG(ZONE_WARN, (TEXT("PWM_IOControl: No matching IOCTL.\r\n")));
            break;
    }
    return bRet;
}

//-----------------------------------------------------------------------------
//
//  Function:  PWM_DllEntry
//
//  This function is an optional method of entry into a DLL. If the function
//  is used, it is called by the system when processes and threads are
//  initialized and terminated, or on calls to the LoadLibrary and
//  FreeLibrary functions.
//
//  Parameters:
//      hinstDLL
//          [in] Handle to the DLL. The value is the base address of the DLL.
//
//      dwReason
//          [in] Specifies a flag indicating why the DLL entry-point function
//          is being called.
//
//      lpvReserved
//          [in] Specifies further aspects of DLL initialization and cleanup.
//          If dwReason is DLL_PROCESS_ATTACH, lpvReserved is NULL for
//          dynamic loads and nonnull for static loads. If dwReason is
//          DLL_PROCESS_DETACH, lpvReserved is NULL if DllMain is called
//          by using FreeLibrary and nonnull if DllMain is called during
//          process termination.
//
//  Returns:
//      When the system calls the DllMain function with the
//      DLL_PROCESS_ATTACH value, the function returns TRUE if it
//      succeeds or FALSE if initialization fails.
//
//      If the return value is FALSE when DllMain is called because the
//      process uses the LoadLibrary function, LoadLibrary returns NULL.
//
//      If the return value is FALSE when DllMain is called during
//      process initialization, the process terminates with an error.
//
//      When the system calls the DllMain function with a value other
//      than DLL_PROCESS_ATTACH, the return value is ignored.
//
//-----------------------------------------------------------------------------
BOOL WINAPI PWM_DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            //Register Debug Zones
            DEBUGREGISTER((HINSTANCE) hInstDll);
            DEBUGMSG(ZONE_INFO, (TEXT("PWM_DllEntry: DLL_PROCESS_ATTACH lpvReserved(0x%x)\r\n"),lpvReserved));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INFO, (TEXT("PWM_DllEntry: DLL_PROCESS_DETACH lpvReserved(0x%x)\r\n"),lpvReserved));
            break;
    }
   // Return TRUE for success
    return TRUE;
}



