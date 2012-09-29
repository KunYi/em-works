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
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  can_io.cpp
//
//  This module provides a stream interface for the CAN bus
//  driver.  Client drivers can use the stream interface to
//  configure and exchange data with the CAN peripheral.
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <windev.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_ddk.h"
#include "common_can.h"
#include "flex_can.h"
#include "canclass.h"

#pragma warning(push)
#pragma warning(disable: 4512)
#include "marshal.hpp" //helper classes to marshal/alloc embedded/async buffer
#pragma warning(pop)

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
#define CAN_DEVINDEX_MAX_VAL            2
#define CAN_DEVINDEX_MIN_VAL            1
#define CAN_DEVINDEX_DEFAULT_VAL        1

#define CAN_METHOD_MASK 1

#ifdef DEBUG
DBGPARAM dpCurSettings = {
    TEXT("can"), {
        TEXT("Init"),TEXT("Deinit"),TEXT("Open"),TEXT("Close"),
        TEXT("IOCtl"),TEXT("Thread"),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT("Read"),
        TEXT("Write"),TEXT("Function"),TEXT("Warning"),TEXT("Error") },
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
// Function: CAN_Init
//
// The Device Manager calls this function as a result of a call to the
//      ActivateDevice() function.
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
DWORD CAN_Init(LPCTSTR pContext)
{    
    UINT32 error;
    HKEY  hKey;
    DWORD  dwIndex, dwSize;

    //DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("CAN_Init +\r\n")));
	RETAILMSG (1, (TEXT("->CAN_Init::flexcan driver\r\n")));

    // try to open active device registry key for this context
    hKey = OpenDeviceKey(pContext);
    if (hKey == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CAN_Init:  OpenDeviceKey failed!!!\r\n")));
        return 0;
    }

    // try to load CAN index from registry data
    dwSize = sizeof(DWORD);
    error = RegQueryValueEx(
        hKey,                       // handle to currently open key
        REG_DEVINDEX_VAL_NAME,      // string containing value to query
        NULL,                       // reserved, set to NULL
        NULL,                       // type not required, set to NULL
        (LPBYTE)(&dwIndex),         // pointer to buffer receiving value
        &dwSize);                   // pointer to buffer size

    // close handle to open key
    RegCloseKey(hKey);

    // check for errors during RegQueryValueEx
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CAN_Init:  RegQueryValueEx failed!!!\r\n")));
        return 0;
    }

    // Construct the CAN Module Class
    CANClass* pCAN = new CANClass(dwIndex);

    // Managed to create the class?
    if (pCAN == NULL)
    {
        return NULL;
    }

    //DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("CAN_Init - hDev=0x%x\r\n"), pCAN));
    RETAILMSG (1, (TEXT("<-CAN_Init - hDev=0x%x\r\n"), pCAN));

    // Otherwise return the created instance
    return (DWORD) pCAN;
}


//-----------------------------------------------------------------------------
//
// Function: CAN_Deinit
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
BOOL CAN_Deinit(DWORD hDeviceContext)
{
    CANClass * pCAN = (CANClass*) hDeviceContext;

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("CAN_Deinit +DeviceContext=0x%x\r\n"),hDeviceContext));

    if (pCAN != NULL)
    {
        delete pCAN;
    }

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("CAN_Deinit -\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CAN_Open
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
DWORD CAN_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    CANClass * pCAN = (CANClass*)hDeviceContext;

	DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("CAN_Open +hDeviceContext=0x%x\r\n"),hDeviceContext));
	//RETAILMSG(1, (TEXT("CAN_Open +hDeviceContext=0x%x\r\n"), hDeviceContext));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

	//
	// CS&ZHL July 28-2011: config EM9170's can_tx & can_rx
	//
	BSPCANConfigureGPIO( pCAN->can_index );
    BSPCANClockConfig(pCAN->can_index,TRUE );   
    BSPSetCanPowerEnable(pCAN->can_index,TRUE);

	DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("CAN_Open -\r\n")));
	//RETAILMSG(1, (TEXT("CAN_Open -\r\n")));
    return hDeviceContext;
}


//-----------------------------------------------------------------------------
//
// Function: CAN_Close
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
BOOL CAN_Close(DWORD hOpenContext)
{
    CANClass * pCAN = (CANClass*)hOpenContext;

	DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("CAN_Close +\r\n")));

    // Remove-W4: Warning C4100 workaround
    //UNREFERENCED_PARAMETER(hOpenContext);

	pCAN->Stop( );

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("CAN_Close -\r\n")));

    // Close is meaningless!
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CAN_PowerDown
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
void CAN_PowerDown(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    // Not implemented!
}

//-----------------------------------------------------------------------------
//
// Function: CAN_PowerUp
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
void CAN_PowerUp(void)
{
    // Not implemented!
}

//-----------------------------------------------------------------------------
//
// Function: CAN_Read
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
DWORD CAN_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
	CANClass			*pCAN = (CANClass*) hOpenContext;
	PCAN_PACKET	pPkt;
	DWORD				dwDatLen, dwSize;
	BOOL				bRet;

	DEBUGMSG (ZONE_READ|ZONE_FUNCTION, (TEXT("CAN_Read +\r\n")));
	if (pCAN == NULL)
	{
		DEBUGMSG (ZONE_ERROR, (TEXT("CAN_Read::null file handle\r\n")));
		return 0;
	}

	dwSize = sizeof(CAN_PACKET);
	if( !pBuffer || (Count < dwSize) )
	{
		DEBUGMSG (ZONE_ERROR, (TEXT("CAN_Read::invalid input parameters\r\n")));
		return 0;
	}
	
	pPkt = (PCAN_PACKET)pBuffer;
	for( dwDatLen = 0; ; )
	{
		bRet = pCAN->ReadPacket( pPkt );
		if( !bRet )     // is empty
		{
			break;
		}

		dwDatLen += dwSize;
		pPkt++;

		if((dwDatLen + dwSize) > Count)
		{
			break;
		}
	}

	return dwDatLen;
}

//-----------------------------------------------------------------------------
//
// Function: CAN_Write
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
DWORD CAN_Write(DWORD hOpenContext, LPCVOID pBuffer, DWORD dwNumBytes)
{
    CANClass			*pCAN = (CANClass*) hOpenContext;
	PCAN_PACKET	pPkt;
	DWORD				dwDatLen, dwSize;
	BOOL				bRet;

	DEBUGMSG (ZONE_WRITE|ZONE_FUNCTION, (TEXT("CAN_Write +\r\n")));
	if (pCAN == NULL)
	{
		RETAILMSG (1, (TEXT("CAN_Write::null file handle\r\n")));
		return 0;
	}

	dwSize = sizeof(CAN_PACKET);
	if( !pBuffer || (dwNumBytes < dwSize) )
	{
		RETAILMSG (1, (TEXT("CAN_Wite::invalid input parameters\r\n")));
		return 0;
	}

	pPkt = (PCAN_PACKET)pBuffer;
	for( dwDatLen = 0; ; )
	{
		bRet = pCAN->WritePacket( pPkt );
		if( !bRet )     // is full
		{
			break;
		}

		dwDatLen += dwSize;
		pPkt++;

		if((dwDatLen + dwSize) > dwNumBytes)
		{
			break;
		}
	}

	return dwDatLen;
}

//-----------------------------------------------------------------------------
//
// Function: CAN_Seek
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
DWORD CAN_Seek(DWORD hOpenContext, long Amount, WORD Type)
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
// Function: CAN_IOControl
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
BOOL CAN_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, 
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    // hOpenContext is a pointer to CANClass instance!
	CANClass*	pCAN = (CANClass*) hOpenContext;
    BOOL			bRet = FALSE;
	HANDLE		WaitEvent[2];
	DWORD			dwWaitReturn;
	DWORD			dwTemp;

    // Remove-W4: Warning C4100 workaround
    //UNREFERENCED_PARAMETER(pdwActualOut);

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("CAN_IOControl: +hOpenContext=0x%x\r\n"),hOpenContext));

	if (pCAN == NULL)
		return bRet;
     
	switch (dwCode)
    {
		case CAN_IOCTL_START:
			//RETAILMSG(1, (TEXT("CAN_IOControl::CAN_IOCTL_START\r\n")));
			bRet = pCAN->Start( );
			break;

		case CAN_IOCTL_STOP:
			//RETAILMSG(1, (TEXT("CAN_IOControl::CAN_IOCTL_STOP\r\n")));
			bRet = pCAN->Stop( );
			break;

		case CAN_IOCTL_RESET:
			//RETAILMSG(1, (TEXT("CAN_IOControl::CAN_IOCTL_RESET\r\n")));
			bRet = pCAN->SoftReset( );
			break;

		case CAN_IOCTL_SET_BAUD_RATE:
			//RETAILMSG(1, (TEXT("CAN_IOControl::CAN_IOCTL_SET_BAUD_RATE\r\n")));
			if(!pBufIn || (dwLenIn < sizeof(DWORD)))
			{
				bRet = FALSE;
				break;
			}
			memcpy( &dwTemp, pBufIn, sizeof(DWORD));
			//RETAILMSG(1, (TEXT("CAN_IOControl::Set BR = %d\r\n"), dwTemp));
			bRet = pCAN->SetBaudRate( dwTemp );
			break;

		case CAN_IOCTL_SET_FILTER:
			//RETAILMSG(1, (TEXT("CAN_IOControl::CAN_IOCTL_SET_FILTER\r\n")));
			if(!pBufIn || (dwLenIn < sizeof(CAN_FILTER)))
			{
				RETAILMSG(1, (TEXT("CAN_IOCTL_SET_FILTER failed! ->pFilter = 0x%x, len=%d\r\n"), pBufIn, dwLenIn));
				bRet = FALSE;
				break;
			}
			bRet = pCAN->SetFilter( (PCAN_FILTER)pBufIn );
			break;

		case CAN_IOCTL_ENABLE_SYNC:
			break;
		case CAN_IOCTL_SELF_TEST:
			break;
		case CAN_IOCTL_GET_STATISTICS:
			break;

		case CAN_IOCTL_WAIT_EVENT:
			//RETAILMSG(1, (TEXT("CAN_IOControl::CAN_IOCTL_WAIT_EVENT\r\n")));
			if(!pBufIn || (dwLenIn < sizeof(DWORD)))
			{
				bRet = FALSE;
				break;
			}

			if(!pBufOut || (dwLenOut < sizeof(DWORD)))
			{
				bRet = FALSE;
				break;
			}

			WaitEvent[0] = pCAN->m_hRecvEvent;
			WaitEvent[1] = pCAN->m_hErrEvent;
			dwWaitReturn = WaitForMultipleObjects( 2,  WaitEvent,  FALSE,  *((PDWORD)pBufIn) );
			if(  dwWaitReturn == WAIT_OBJECT_0 )					 //The state of the specified object for received data
			{
				*((PDWORD)pBufOut) = 1;
			}
			else if(  dwWaitReturn == WAIT_OBJECT_0+1 )			//The state of the specified object for error message
			{
				*((PDWORD)pBufOut) = 2;
			}
			else																			// The state of WAIT_TIMEOUT and WAIT_FAILED
			{
				*((PDWORD)pBufOut) = 0;
			}

			if(pdwActualOut != NULL)
			{
				*pdwActualOut = (DWORD)(sizeof(DWORD));
			}
			bRet = TRUE;
			break;

		default:
			break;
	}

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("CAN_IOControl -\r\n")));
    return bRet;
}

BOOL WINAPI CAN_DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) 
	{
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HINSTANCE) hInstDll);
            DEBUGMSG(ZONE_FUNCTION, (TEXT("CAN_DllEntry: DLL_PROCESS_ATTACH lpvReserved(0x%x)\r\n"),lpvReserved));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;
         
        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_FUNCTION, (TEXT("CAN_DllEntry: DLL_PROCESS_DETACH lpvReserved(0x%x)\r\n"),lpvReserved));
            break;
    }
    return TRUE;
}
