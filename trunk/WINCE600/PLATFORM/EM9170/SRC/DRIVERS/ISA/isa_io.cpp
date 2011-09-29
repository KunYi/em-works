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
#include "common_ddk.h"
#include "isaclass.h"

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
    TEXT("ISA"), {
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
// Function: ISA_Init
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
DWORD ISA_Init(LPCTSTR pContext)
{    
    UINT32 error;
    HKEY  hKey;
    DWORD  dwIndex, dwSize;

    RETAILMSG (1, (TEXT("ISA_Init +\r\n")));

    // try to open active device registry key for this context
    hKey = OpenDeviceKey(pContext);
    if (hKey == NULL)
    {
        RETAILMSG(1, (TEXT("ISA_Init:  OpenDeviceKey failed!!!\r\n")));
        return 0;
    }

    // try to load ISA index from registry data
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

    // Construct the ISA Module Class
    ISAClass* pISA = new ISAClass(dwIndex);

    // Managed to create the class?
    if (pISA == NULL)
    {
        return NULL;
    }

	RETAILMSG (1, (TEXT("ISA_Init - hDev=0x%x\r\n"), pISA));

	return (DWORD)pISA;
}


//-----------------------------------------------------------------------------
//
// Function: ISA_Deinit
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
BOOL ISA_Deinit(DWORD hDeviceContext)
{
    ISAClass * pISA = (ISAClass*) hDeviceContext;

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("ISA_Deinit +DeviceContext=0x%x\r\n"),hDeviceContext));

    if (pISA != NULL)
    {
        delete pISA;
    }

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("ISA_Deinit -\r\n")));
	
	return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: ISA_Open
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
DWORD ISA_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    //DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("ISA_Open +hDeviceContext=0x%x\r\n"),hDeviceContext));
    RETAILMSG (1, (TEXT("ISA_Open +hDeviceContext=0x%x\r\n"),hDeviceContext));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    //DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("ISA_Open -\r\n")));
    RETAILMSG (1, (TEXT("ISA_Open - hDev=0x%x\r\n"), hDeviceContext));

    // Open is meaningless!
    return hDeviceContext;
}


//-----------------------------------------------------------------------------
//
// Function: ISA_Close
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
BOOL ISA_Close(DWORD hOpenContext)
{
    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("ISA_Close +\r\n")));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    // Close is meaningless!
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: ISA_PowerDown
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
void ISA_PowerDown(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    // Not implemented!
}


//-----------------------------------------------------------------------------
//
// Function: ISA_PowerUp
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
void ISA_PowerUp(void)
{
    // Not implemented!
}


//-----------------------------------------------------------------------------
//
// Function: ISA_Read
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
DWORD ISA_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(Count);

    // Nothing to read
    return 0;
}


//-----------------------------------------------------------------------------
//
// Function: ISA_Write
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
DWORD ISA_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

    // Nothing to write
    return 0;
}


//-----------------------------------------------------------------------------
//
// Function: ISA_Seek
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
DWORD ISA_Seek(DWORD hOpenContext, long Amount, WORD Type)
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
// Function: ISA_IOControl
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
BOOL ISA_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, 
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    int							i1;
	BOOL						bRet = FALSE;
	UINT32					Bits;
	DWORD					dwTemp;
	PISA_BUS_ACCESS	pISABus;

    // Remove-W4: Warning C4100 workaround
    //UNREFERENCED_PARAMETER(pdwActualOut);

    // hOpenContext is a pointer to ISAClass instance!
    ISAClass* pISA = (ISAClass*) hOpenContext;

    
    //DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("ISA_IOControl: +hOpenContext=0x%x\r\n"),hOpenContext));
    //RETAILMSG (1, (TEXT("ISA_IOControl: +hOpenContext=0x%x\r\n"),hOpenContext));

    if (pISA != NULL)
    {
        switch (dwCode)
        {
			case GPIO_IOCTL_OUT_ENABLE:
                if (!pBufIn || (dwLenIn != sizeof(UINT32)))
				{
					RETAILMSG(1, (TEXT("GPIO_IOCTL_OUT_ENABLE: input parameter error\r\n")));
                    break;
				}

                Bits = *((UINT32*)(pBufIn));
				i1 = pISA->PIO_OutEnable(  Bits );
				if( i1==0 )
					bRet = TRUE;
				break;

			case GPIO_IOCTL_OUT_DISABLE:
                if (!pBufIn || (dwLenIn != sizeof(UINT32)))
				{
					RETAILMSG(1, (TEXT("GPIO_IOCTL_OUT_DISABLE: input parameter error\r\n")));
                    break;
				}

                Bits = *((UINT32*)(pBufIn));
				i1 = pISA->PIO_OutDisable(  Bits );
				if( i1==0 )
					bRet = TRUE;
				break;

			case GPIO_IOCTL_OUT_SET:
                if (!pBufIn || (dwLenIn != sizeof(UINT32)))
				{
					RETAILMSG(1, (TEXT("GPIO_IOCTL_OUT_SET: input parameter error\r\n")));
                    break;
				}

                Bits = *((UINT32*)(pBufIn));
				i1 = pISA->PIO_OutSet(  Bits );
				if( i1==0 )
					bRet = TRUE;
				break;

			case GPIO_IOCTL_OUT_CLEAR:
                if (!pBufIn || (dwLenIn != sizeof(UINT32)))
				{
					RETAILMSG(1, (TEXT("GPIO_IOCTL_OUT_CLEAR: input parameter error\r\n")));
                    break;
				}

                Bits = *((UINT32*)(pBufIn));
				i1 = pISA->PIO_OutClear(  Bits );
				if( i1==0 )
					bRet = TRUE;
				break;

			case GPIO_IOCTL_PIN_STATE:
                if (!pBufOut || (dwLenOut != sizeof(UINT32)))
				{
					RETAILMSG(1, (TEXT("GPIO_IOCTL_PIN_STATE: output parameter error\r\n")));
                    break;
				}

				i1 = pISA->PIO_State(  &Bits );
				if( i1==0 )
				{
					memcpy( pBufOut, &Bits, dwLenOut );				
					bRet = TRUE;
				}
				break;
			
			case ISA_IOCTL_READ_WRITE:
				if(!pBufIn || (dwLenIn < sizeof(ISA_BUS_ACCESS)))
				{
					RETAILMSG(1, (TEXT("ISA_IOCTL_READ_WRITE: input parameter error\r\n")));
					break;
				}
				pISABus = (PISA_BUS_ACCESS)pBufIn;
				if(pISABus->dwCmd)		// -> Write operation
				{
					i1 = pISA->ISA_WriteUchar( pISABus->dwSeg, pISABus->dwOffset, (UCHAR)pISABus->dwValue);
					if(!i1)
					{
						bRet = TRUE;
					}
				}
				else							// -> Read operation
				{
					if(!pBufOut || (dwLenOut < sizeof(UCHAR)))
					{
						RETAILMSG(1, (TEXT("ISA_IOCTL_READ_WRITE: output parameter error\r\n")));
						break;
					}

					i1 = pISA->ISA_ReadUchar( pISABus->dwSeg, pISABus->dwOffset, (UCHAR*)pBufOut);
					if(!i1)
					{
						bRet = TRUE;
					}

					if(pdwActualOut != NULL)
					{
						*pdwActualOut = sizeof(UCHAR);
					}
				}
				break;

			case ISA_IOCTL_BUS_RESET:
				if(!pBufIn || (dwLenIn < sizeof(DWORD)))
				{
					RETAILMSG(1, (TEXT("ISA_IOCTL_READ_WRITE: input parameter error\r\n")));
					break;
				}
				dwTemp = *((DWORD*)pBufIn);
				i1 = pISA->ISA_Reset(dwTemp);
				if(!i1)
				{
					bRet = TRUE;
				}
				break;
		}
	}

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("ISA_IOControl -\r\n")));

    return bRet;
}

BOOL WINAPI ISA_DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HINSTANCE) hInstDll);
            DEBUGMSG(ZONE_FUNCTION, (TEXT("ISA_DllEntry: DLL_PROCESS_ATTACH lpvReserved(0x%x)\r\n"),lpvReserved));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;
         
        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_FUNCTION, (TEXT("ISA_DllEntry: DLL_PROCESS_DETACH lpvReserved(0x%x)\r\n"),lpvReserved));
            break;
    }
    // return TRUE for success
    return TRUE;
}
