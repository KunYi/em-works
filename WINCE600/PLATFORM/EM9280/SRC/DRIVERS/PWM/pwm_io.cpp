//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2011,Emtronix, Inc. All Rights Reserved.
//
//------------------------------------------------------------------------------
// PWM_io.cpp : Defines the entry point for the DLL application.
//
#pragma warning(disable: 4115 4201 4204 4214 4245)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <devload.h>
#include "bsp.h"
#include "pwmclass.h"

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

#define REG_DEVINDEX_VAL_NAME           L"Index"


#ifdef DEBUG
DBGPARAM dpCurSettings = {
    TEXT("WDT"), {
        TEXT("Init"),TEXT("Deinit"),TEXT("Open"),TEXT("Close"),
        TEXT("IOCtl"),TEXT("Thread"),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT("Function"),TEXT("Warning"),TEXT("Error") },
    (ZONEMASK_WARN | ZONEMASK_ERROR)
};
#endif // DEBUG


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function: PWM_Init
//
// Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//                active key for the stream interface driver.
//
// Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//-----------------------------------------------------------------------------
DWORD PWM_Init(LPCTSTR pContext)
{    
    UINT32			error;
    HKEY			hKey;
    DWORD			dwIndex, dwSize;

    RETAILMSG (1, (TEXT("->PWM_Init\r\n")));

    // try to open active device registry key for this context
    hKey = OpenDeviceKey(pContext);
    if (hKey == NULL)
    {
        RETAILMSG(1, (TEXT("PWM_Init:  OpenDeviceKey failed!!!\r\n")));
        return 0;
    }

    // try to load WDT index from registry data
    dwSize = sizeof(DWORD);
    error = RegQueryValueEx(
        hKey,							// handle to currently open key
        REG_DEVINDEX_VAL_NAME,			// string containing value to query
        NULL,							// reserved, set to NULL
        NULL,							// type not required, set to NULL
        (LPBYTE)(&dwIndex),				// pointer to buffer receiving value
        &dwSize);						// pointer to buffer size

    // close handle to open key
    RegCloseKey(hKey);

    // check for errors during RegQueryValueEx
    if (error != ERROR_SUCCESS)
    {
        RETAILMSG(1, (TEXT("PWM_Init: RegQueryValueEx failed!!!\r\n")));
        return NULL;
    }

    // Construct the PWM Module Class
    PWMClass* pPwm = new PWMClass(dwIndex);
    if (pPwm == NULL)	// Managed to create the class?
    {
		RETAILMSG(1, (TEXT("PWM_Init: Allocate PWMClass failed\r\n")));
        return NULL;
    }

	RETAILMSG (1, (TEXT("<-PWM_Init: PWM%d Initialization OK\r\n"), pPwm->GetIndex()));
	return (DWORD)pPwm;
}


//-----------------------------------------------------------------------------
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
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL PWM_Deinit(DWORD hDeviceContext)
{
	PWMClass* pPwm = (PWMClass*)hDeviceContext;

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("PWM_Deinit +DeviceContext=0x%x\r\n"),hDeviceContext));
    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("PWM_Deinit -\r\n")));

    if (pPwm != NULL)
    {
        delete pPwm;
    }

	return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PWM_Open
//
// This function opens a device for reading, writing, or both.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context. The XXX_Init function creates
//                and returns this handle.
//      AccessCode
//          [in] Access code for the device. The access is a combination of
//                read and write access from CreateFile.
//      ShareMode
//          [in] File share mode of the device. The share mode is a
//                combination of read and write access sharing from CreateFile.
//
// Returns:
//      This function returns a handle that identifies the open context of
//      the device to the calling application.
//
//-----------------------------------------------------------------------------
DWORD PWM_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
	PWMClass* pPwm = (PWMClass*)hDeviceContext;

	DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("PWM_Open +hDeviceContext=0x%x\r\n"),hDeviceContext));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

	// validation check  
	if(pPwm == NULL)
	{
        RETAILMSG(1, (TEXT("PWM_Open: null handle!\r\n")));
		return NULL;
	}

	if(!pPwm->PinConfig())
	{
        RETAILMSG(1, (TEXT("PWM_Open: PinConfig failed!\r\n")));
		return NULL;
	}

	// just pass handle
	return (DWORD)pPwm;
}


//-----------------------------------------------------------------------------
//
// Function: PWM_Close
//
// This function opens a device for reading, writing, or both.
//
// Parameters:
//      hOpenContext
//          [in] Handle returned by the XXX_Open function, used to identify
//                the open context of the device.
//
// Returns:  
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL PWM_Close(DWORD hOpenContext)
{
	PWMClass* pPwm = (PWMClass*)hOpenContext;

	DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("PWM_Close +\r\n")));

	if(!pPwm->OutputConfig(0, 0, 0))
	{
        RETAILMSG(1, (TEXT("PWM_Close: stop PWM%d output failed!\r\n"), pPwm->GetIndex()));
		return FALSE;
	}

	return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PWM_PowerDown
//
// This function suspends power to the device. It is useful only with 
//      devices that can power down under software control.
//
// Parameters:
//      hDeviceContext 
//          [in] Handle to the device context.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
void PWM_PowerDown(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    // Not implemented!
}


//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void PWM_PowerUp(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    // Not implemented!
}


//-----------------------------------------------------------------------------
//
// Function: PWM_Read
//
// This function reads data from the device identified by the open 
//      context.
//
// Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
//      pBuffer 
//         [out] Pointer to the buffer that stores the data read from the 
//                 device. This buffer should be at least Count bytes long. 
//      Count 
//          [in] Number of bytes to read from the device into pBuffer. 
//
// Returns:  
//      Returns zero to indicate end-of-file. Returns -1 to indicate an 
//      error. Returns the number of bytes read to indicate success.
//
//-----------------------------------------------------------------------------
DWORD PWM_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
	PWMClass* pPwm = (PWMClass*)hOpenContext;
	PPWM_INFO pPwmInfo;

	if( !pBuffer || (Count < sizeof(PWM_INFO)) )
	{
		RETAILMSG(1, (TEXT("PWM_Read: parameter error!\r\n")));
		return (DWORD)-1;
	}

	pPwmInfo = (PPWM_INFO)pBuffer;
	pPwmInfo->dwFreq = pPwm->GetFreq( );
	pPwmInfo->dwDuty = pPwm->GetDuty( );
	pPwmInfo->dwResolution = pPwm->GetResolution( );

	return sizeof(PWM_INFO);
}


//-----------------------------------------------------------------------------
//
// Function: PWM_Write
//
// This function writes data to the device.
//
// Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
//      pBuffer 
//         [out] Pointer to the buffer that contains the data to write. 
//      Count 
//          [in] Number of bytes to read from the device into pBuffer. 
//
// Returns:  
//      The number of bytes written indicates success. A value of -1 indicates 
//      failure.
//
//-----------------------------------------------------------------------------
DWORD PWM_Write(DWORD hOpenContext, LPCVOID pBuffer, DWORD dwNumBytes)
{
	PWMClass* pPwm = (PWMClass*)hOpenContext;
	PPWM_INFO pPwmInfo;

	if( !pBuffer || (dwNumBytes < sizeof(PWM_INFO)) )
	{
		RETAILMSG(1, (TEXT("PWM_Write: parameter error!\r\n")));
		return (DWORD)-1;
	}

	pPwmInfo = (PPWM_INFO)pBuffer;
	if(!pPwm->OutputConfig(pPwmInfo->dwFreq, pPwmInfo->dwDuty, pPwmInfo->dwResolution))
	{
        RETAILMSG(1, (TEXT("PWM_Write: start PWM%d output failed!\r\n"), pPwm->GetIndex()));
		return 0;
	}

	return sizeof(PWM_INFO);
}

//-----------------------------------------------------------------------------
//
// Function: PWM_Seek
//
// This function moves the data pointer in the device.
//
// Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
//      Amount 
//         [in] Number of bytes to move the data pointer in the device. 
//               A positive value moves the data pointer toward the end of the 
//               file, and a negative value moves it toward the beginning.
//      Type 
//         [in] Starting point for the data pointer. 
//
// Returns:  
//      The new data pointer for the device indicates success. A value of -1 
//      indicates failure.
//
//-----------------------------------------------------------------------------
DWORD PWM_Seek(DWORD hOpenContext, long Amount, WORD Type)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(Amount);
    UNREFERENCED_PARAMETER(Type);

    // Seeking is meaningless!
    return (DWORD)-1;
}


//-----------------------------------------------------------------------------
//
// Function: PWM_IOControl
//
// This function sends a command to a device.
//
// Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
//      dwCode 
//          [in] I/O control operation to perform. These codes are 
//                device-specific and are usually exposed to developers through 
//                a header file.
//      pBufIn 
//          [in] Pointer to the buffer containing data to transfer to the 
//                device. 
//      dwLenIn 
//         [in] Number of bytes of data in the buffer specified for pBufIn.
//
//      pBufOut 
//          [out] Pointer to the buffer used to transfer the output data 
//                  from the device.
//      dwLenOut 
//          [in] Maximum number of bytes in the buffer specified by pBufOut.
//
//      pdwActualOut 
//          [out] Pointer to the DWORD buffer that this function uses to 
//                  return the actual number of bytes received from the device.
//
// Returns:  
//      The new data pointer for the device indicates success. A value of -1 
//      indicates failure.
//
//-----------------------------------------------------------------------------
BOOL PWM_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, 
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
	BOOL						bRet = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(dwCode);
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pdwActualOut);


    return bRet;
}

BOOL WINAPI DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HINSTANCE) hInstDll);
            DEBUGMSG(ZONE_FUNCTION, (TEXT("DllEntry: DLL_PROCESS_ATTACH lpvReserved(0x%x)\r\n"),lpvReserved));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;
         
        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_FUNCTION, (TEXT("DllEntry: DLL_PROCESS_DETACH lpvReserved(0x%x)\r\n"),lpvReserved));
            break;
    }
    // return TRUE for success
    return TRUE;
}
