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
// ISA_io.cpp : Defines the entry point for the DLL application.
//
#include <windows.h>
#include <Devload.h>
#include <windev.h>
#include <ceddk.h>
#include "bsp.h"
#include "bsp_drivers.h"
#include "gpioclass.h"

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
    TEXT("PIO"), {
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
// Function: PIO_Init
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
DWORD PIO_Init(LPCTSTR pContext)
{    
    UINT32	error;
    HKEY	hKey;
    DWORD	dwIndex, dwSize;

    RETAILMSG (1, (TEXT("->PIO_Init\r\n")));

    // try to open active device registry key for this context
    hKey = OpenDeviceKey(pContext);
    if (hKey == NULL)
    {
        RETAILMSG(1, (TEXT("ISA_Init:  OpenDeviceKey failed!!!\r\n")));
        return 0;
    }

    // try to load index from registry data
    dwSize = sizeof(DWORD);
    error = RegQueryValueEx(
        hKey,										 // handle to currently open key
        REG_DEVINDEX_VAL_NAME,      // string containing value to query
        NULL,										 // reserved, set to NULL
        NULL,										 // type not required, set to NULL
        (LPBYTE)(&dwIndex),				// pointer to buffer receiving value
        &dwSize);								 // pointer to buffer size

    // close handle to open key
    RegCloseKey(hKey);

    // check for errors during RegQueryValueEx
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ISA_Init:  RegQueryValueEx failed!!!\r\n")));
        return 0;
    }

    // Construct the GPIO Module Class
    GPIOClass* pGpio = new GPIOClass( );
    if (pGpio == NULL)	// Managed to create the class?
    {
		RETAILMSG (1, (TEXT("PIO_Init: Allocate GPIOClass failed\r\n")));
        return NULL;
    }

	RETAILMSG (1, (TEXT("<-PIO_Init - hDev=0x%x\r\n"), pGpio));
	return (DWORD)pGpio;
}


//-----------------------------------------------------------------------------
//
// Function: PIO_Deinit
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
BOOL PIO_Deinit(DWORD hDeviceContext)
{
	GPIOClass* pGpio = (GPIOClass*)hDeviceContext;

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("PIO_Deinit +DeviceContext=0x%x\r\n"), hDeviceContext));

    if (pGpio != NULL)
    {
        delete pGpio;
    }

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("PIO_Deinit -\r\n")));
	return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PIO_Open
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
DWORD PIO_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("PIO_Open +hDeviceContext=0x%x\r\n"),hDeviceContext));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("PIO_Open -\r\n")));

    return hDeviceContext;		//just return the handler
}


//-----------------------------------------------------------------------------
//
// Function: PIO_Close
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
BOOL PIO_Close(DWORD hOpenContext)
{
    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("PIO_Close +\r\n")));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    // nothing to do!
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PIO_PowerDown
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
void PIO_PowerDown(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    // Not implemented!
}


//-----------------------------------------------------------------------------
//
// Function: PIO_PowerUp
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
void PIO_PowerUp(void)
{
    // Not implemented!
}


//-----------------------------------------------------------------------------
//
// Function: PIO_Read
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
DWORD PIO_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD dwNumBytes)
{
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

	// not implemented
    return (DWORD)-1;
}


//-----------------------------------------------------------------------------
//
// Function: PIO_Write
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
DWORD PIO_Write(DWORD hOpenContext, LPCVOID pBuffer, DWORD dwNumBytes)
{
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

	// not implemented
    return (DWORD)-1;
}


//-----------------------------------------------------------------------------
//
// Function: PIO_Seek
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
DWORD PIO_Seek(DWORD hOpenContext, long Amount, WORD Type)
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
// Function: PIO_IOControl
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
BOOL PIO_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, 
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
	BOOL		bRet = TRUE;
	DWORD		dwGpioBits;
	GPIOClass*	pGpio = (GPIOClass*)hOpenContext;

    // Remove-W4: Warning C4100 workaround
    //UNREFERENCED_PARAMETER(pdwActualOut);
    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("PIO_IOControl: +hOpenContext=0x%x\r\n"),hOpenContext));
    
	if (pGpio == NULL)
    {
		RETAILMSG(1, (TEXT("PIO_IOControl: Device Handler = NULL\r\n")));
		return FALSE;
	}

	// check input parameters
	switch(dwCode)
	{
	case GPIO_IOCTL_OUT_ENABLE:
	case GPIO_IOCTL_OUT_DISABLE:
	case GPIO_IOCTL_OUT_SET:
	case GPIO_IOCTL_OUT_CLEAR:
        if (!pBufIn || (dwLenIn != sizeof(UINT32)))
		{
			RETAILMSG(1, (TEXT("PIO_IOControl: input parameter error\r\n")));
			return FALSE;
		}
		break;

	case GPIO_IOCTL_PIN_STATE:
        if (!pBufIn || (dwLenIn != sizeof(UINT32)))
		{
			RETAILMSG(1, (TEXT("PIO_IOControl::GPIO_IOCTL_PIN_STATE: input parameter error\r\n")));
			return FALSE;
		}

		if (!pBufOut || (dwLenOut != sizeof(UINT32)))
		{
			RETAILMSG(1, (TEXT("PIO_IOControl::GPIO_IOCTL_PIN_STATE: input parameter error\r\n")));
			return FALSE;
		}
		break;

	default:
		RETAILMSG(1, (TEXT("PIO_IOControl: unknown IOCTL code\r\n")));
		return bRet;
	}

	// access GPIO pins on EM9280, including GPIO and GPIOX
    switch(dwCode)
    {
	case GPIO_IOCTL_OUT_ENABLE:
        dwGpioBits = *((DWORD*)pBufIn);
		if( !pGpio->PIO_OutEnable(dwGpioBits) )
		{
			RETAILMSG(1, (TEXT("PIO_IOControl::GPIO_IOCTL_OUT_ENABLE failed\r\n")));
			bRet = FALSE;
		}
		break;

	case GPIO_IOCTL_OUT_DISABLE:
        dwGpioBits = *((DWORD*)pBufIn);
		if( !pGpio->PIO_OutDisable(dwGpioBits) )
		{
			RETAILMSG(1, (TEXT("PIO_IOControl::GPIO_IOCTL_OUT_DISABLE failed\r\n")));
			bRet = FALSE;
		}
		break;

	case GPIO_IOCTL_OUT_SET:
        dwGpioBits = *((DWORD*)pBufIn);
		if( !pGpio->PIO_OutSet(dwGpioBits) )
		{
			RETAILMSG(1, (TEXT("PIO_IOControl::GPIO_IOCTL_OUT_SET failed\r\n")));
			bRet = FALSE;
		}
		break;

	case GPIO_IOCTL_OUT_CLEAR:
        dwGpioBits = *((DWORD*)pBufIn);
		if( !pGpio->PIO_OutClear(dwGpioBits) )
		{
			RETAILMSG(1, (TEXT("PIO_IOControl::GPIO_IOCTL_OUT_CLEAR failed\r\n")));
			bRet = FALSE;
		}
		break;

	case GPIO_IOCTL_PIN_STATE:
        dwGpioBits = *((DWORD*)pBufIn);
		if( !pGpio->PIO_State((UINT32*)&dwGpioBits) )
		{
			RETAILMSG(1, (TEXT("PIO_IOControl::GPIO_IOCTL_PIN_STATE failed\r\n")));
			bRet = FALSE;
		}
		memcpy( pBufOut, &dwGpioBits, dwLenOut );		// copy state data into output buffer
		if(pdwActualOut != NULL)
		{
			*pdwActualOut = dwLenOut;
		}
		break;

	case IOCTL_WAIT_FOR_IRQ:
		if(!pBufIn || (dwLenIn < sizeof(DWORD)))
		{
			RETAILMSG(1, (TEXT("PIO_IOControl::IOCTL_WAIT_FOR_IRQ: input parameter error\r\n")));
			bRet = FALSE;
			break;
		}

		if(!pBufOut || (dwLenOut < sizeof(DWORD)))
		{
			RETAILMSG(1, (TEXT("PIO_IOControl::IOCTL_WAIT_FOR_IRQ: output parameter error\r\n")));
			bRet = FALSE;
			break;
		}

		// return = WAIT_OBJECT_0, WAIT_TIMEOUT and WAIT_FAILED
		*((PDWORD)pBufOut) = pGpio->WaitGpioInterrupt( *((PDWORD)pBufIn) );

		if(pdwActualOut != NULL)
		{
			*pdwActualOut = (DWORD)(sizeof(DWORD));
		}
		break;
	}

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("GPIO_IOControl -\r\n")));
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
