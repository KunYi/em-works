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
// mkeypad_io.cpp : Defines the entry point for the DLL application.
//
#include <windows.h>
#include <Devload.h>
#include <windev.h>
#include <ceddk.h>
#include "mkeypadclass.h"

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

/*UCHAR VKeyCode[in][out] = { 
	{VK_ENTRY,   '.',			'0',	  '*', 	VK_SHIFT},
	{VK_UP,		 '9',			'8',	  '7'   '+'},
	{VK_ESCAPE,  '6',           '5',      '4'   '-'},
	{TAB,   '3',           '2',	  '1'	'\'},
	{VK_BACK,	VK_RIGHT,      VK_DOWN,	VK_LEFT, VK_CAPITAL	} };*/
//------------------------------------------------------------------------------

// Global Variables
const TCHAR VirtualKey[][10]={
	L"KEY00", L"KEY01", L"KEY02", L"KEY03", L"KEY04",
	L"KEY10", L"KEY11", L"KEY12", L"KEY13", L"KEY14",
	L"KEY20", L"KEY21", L"KEY22", L"KEY23", L"KEY24",
	L"KEY30", L"KEY31", L"KEY32", L"KEY33", L"KEY34",
	L"KEY40", L"KEY41", L"KEY42", L"KEY43", L"KEY44"
};

DWORD g_uVkeyCode[MAXIN*MAXOUT];

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
BOOL MKPDClass::g_bMkeyPadIsOpen = FALSE;
//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function: KPD_Init
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
DWORD KPD_Init(LPCTSTR pContext)
{    
    UINT32	error,dwPollingTimeOut, dwMkeyPadFromat;
    HKEY	hKey;
    DWORD	dwSize;


    RETAILMSG (1, (TEXT("->MKPD_Init\r\n")));
	if( MKPDClass::g_bMkeyPadIsOpen )
	{
		RETAILMSG (1, (TEXT("MKPD_Init :MKeyPad has opened!\r\n")));
		return 0;
	}


    // try to open active device registry key for this context
    hKey = OpenDeviceKey(pContext);
    if (hKey == NULL)
    {
        RETAILMSG(1, (TEXT("MKPD_Init:  OpenDeviceKey failed!!!\r\n")));
        return 0;
    }

    // try to load virtual-key codes from registry data
	for( int i=0; i<MAXIN*MAXOUT; i++ )
	{
		dwSize = sizeof(DWORD);
		error = RegQueryValueEx(
			hKey,								// handle to currently open key
			VirtualKey[i],						// string containing value to query
			NULL,								// reserved, set to NULL
			NULL,								// type not required, set to NULL
			(LPBYTE)(&g_uVkeyCode[i]),			// pointer to buffer receiving value
			&dwSize);							// pointer to buffer size
		if (error != ERROR_SUCCESS)
		{ 
			RegCloseKey(hKey);
			RETAILMSG(1, (TEXT("MKPD_Init:  RegQueryValueEx failed!!%d\r\n"), i));
			return 0;
		}
	}
    
	// try to load polling timeout from registry data
	dwSize = sizeof(DWORD);
	error = RegQueryValueEx(
		hKey,										 // handle to currently open key
		TEXT("PollingTimeout"),						// string containing value to query
		NULL,										 // reserved, set to NULL
		NULL,										 // type not required, set to NULL
		(LPBYTE)(&dwPollingTimeOut),				// pointer to buffer receiving value
		&dwSize);									 // pointer to buffer size

	if (error != ERROR_SUCCESS)
	{ 
		RegCloseKey(hKey);
		RETAILMSG(1, (TEXT("MKPD_Init:  RegQueryValueEx failed!!!\r\n")));
		return 0;
	}

	// try to load key pad format from registry data
	dwSize = sizeof(DWORD);
	error = RegQueryValueEx(
		hKey,										 // handle to currently open key
		TEXT("MKeyPadFormat"),						 // string containing value to query
		NULL,										 // reserved, set to NULL
		NULL,										 // type not required, set to NULL
		(LPBYTE)(&dwMkeyPadFromat),					// pointer to buffer receiving value
		&dwSize);									 // pointer to buffer size

	if (error != ERROR_SUCCESS)
	{ 
		RegCloseKey(hKey);
		RETAILMSG(1, (TEXT("MKPD_Init:  RegQueryValueEx failed!!!\r\n")));
		return 0;
	}

    // close handle to open key
    RegCloseKey(hKey);

    // Construct the GPIO Module Class
    MKPDClass* pMKpd = new MKPDClass( dwPollingTimeOut, dwMkeyPadFromat );
    if (pMKpd == NULL)	// Managed to create the class?
    {
		RETAILMSG (1, (TEXT("MKPD_Init: Allocate GPIOClass failed\r\n")));
        return NULL;
    }

	if( !MKPDClass::g_bMkeyPadIsOpen )
 	{
 		if (pMKpd != NULL)
 		{
 			delete pMKpd;
 		}
 		RETAILMSG (1, (TEXT("MKPDClass:Open mtraix keypad failed\r\n")));
 		return NULL;
 	}

	RETAILMSG (1, (TEXT("<-MKPD_Init hDev=0x%x\r\n"), pMKpd));
	return (DWORD)pMKpd;
}


//-----------------------------------------------------------------------------
//
// Function: KPD_Deinit
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
BOOL KPD_Deinit(DWORD hDeviceContext)
{
	MKPDClass* pMKpd = (MKPDClass*)hDeviceContext;

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("PIO_Deinit +DeviceContext=0x%x\r\n"), hDeviceContext));

    if (pMKpd != NULL)
    {
        delete pMKpd;
    }

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("KPD_Deinit -\r\n")));
	return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: KPD_Open
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
DWORD KPD_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("KPD_Open +hDeviceContext=0x%x\r\n"),hDeviceContext));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("KPD_Open -\r\n")));

    return hDeviceContext;		//just return the handler
}


//-----------------------------------------------------------------------------
//
// Function: KPD_Close
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
BOOL KPD_Close(DWORD hOpenContext)
{
    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("KPD_Close +\r\n")));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    // nothing to do!
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: KPD_PowerDown
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
void KPD_PowerDown(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    // Not implemented!
}


//-----------------------------------------------------------------------------
//
// Function: KPD_PowerUp
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
void KPD_PowerUp(void)
{
    // Not implemented!
}


//-----------------------------------------------------------------------------
//
// Function: KPD_Read
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
DWORD KPD_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD dwNumBytes)
{
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

	// not implemented
    return (DWORD)-1;
}


//-----------------------------------------------------------------------------
//
// Function: KPD_Write
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
DWORD KPD_Write(DWORD hOpenContext, LPCVOID pBuffer, DWORD dwNumBytes)
{
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

	// not implemented
    return (DWORD)-1;
}


//-----------------------------------------------------------------------------
//
// Function: KPD_Seek
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
DWORD KPD_Seek(DWORD hOpenContext, long Amount, WORD Type)
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
// Function: KPD_IOControl
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
BOOL KPD_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, 
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
	// Remove-W4: Warning C4100 workaround
	UNREFERENCED_PARAMETER(hOpenContext);
	UNREFERENCED_PARAMETER(dwCode);
	UNREFERENCED_PARAMETER(pBufIn);
	UNREFERENCED_PARAMETER(dwLenIn);
	UNREFERENCED_PARAMETER(pBufOut);
	UNREFERENCED_PARAMETER(dwLenOut);
	UNREFERENCED_PARAMETER(pdwActualOut);
	
	// IOControl is meaningless!
	return (DWORD)-1;
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
