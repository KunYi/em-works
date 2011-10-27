//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//------------------------------------------------------------------------------
/*---------------------------------------------------------------------------
* Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
//
//  File:  cspi_io.cpp
//
//   This module provides a stream interface for the CSPI bus
//   driver.  Client drivers can use the stream interface to
//   configure and exchange data with the CSPI peripheral.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_ddk.h"
#include "common_cspiv2.h"
#include "cspibus.h"
#include "cspiClass.h"

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines
#define REG_DEVINDEX_VAL_NAME           TEXT("Index")
#define REG_PHYSICAL_DEVICEID			TEXT("DeviceID")

#ifdef DEBUG
DBGPARAM dpCurSettings = {
    TEXT("cspi"), {
        TEXT("Init"),TEXT("Deinit"),TEXT("Open"),TEXT("Close"),
        TEXT("IOCtl"),TEXT("Thread"),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT("Function"),TEXT("Warning"),TEXT("Error") },
    0xC000
};
#endif // DEBUG

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function: SPI_Init
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
//      zero if not successfu.
//
//-----------------------------------------------------------------------------
DWORD SPI_Init(LPCTSTR pContext)
{
    LONG    regError;
    HKEY    hKey;
    DWORD   dwDataSize;
    DWORD   dwDevIndex;
    cspiClass *pCspi = NULL;

    //DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("SPI_Init +\r\n")));
    RETAILMSG (1, (TEXT("SPI_Init +\r\n")));

    // try to open active device registry key for this context
    hKey = OpenDeviceKey(pContext);
    if (hKey == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SPI_Init:  OpenDeviceKey failed!!!\r\n")));
        return 0;
    }

    // try to load CSPI index from registry data
    dwDataSize = sizeof(DWORD);
    regError = RegQueryValueEx(
        hKey,                       // handle to currently open key
        //REG_DEVINDEX_VAL_NAME,      // string containing value to query
		REG_PHYSICAL_DEVICEID,
        NULL,                       // reserved, set to NULL
        NULL,                       // type not required, set to NULL
        (LPBYTE)(&dwDevIndex),      // pointer to buffer receiving value
        &dwDataSize);               // pointer to buffer size

    // close handle to open key
    RegCloseKey(hKey);

    // check for errors during RegQueryValueEx
    if (regError != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SPI_Init:  RegQueryValueEx failed!!!\r\n")));
        return 0;
    }

    pCspi = new cspiClass();
    if (pCspi && !pCspi->CspiInitialize(dwDevIndex) ) 
    {
        delete pCspi;
        //DEBUGMSG(ZONE_ERROR, (TEXT("SPI_Init:  CspiInitialize failed!!!\r\n")));
        RETAILMSG(1, (TEXT("SPI_Init:  CspiInitialize failed!!!\r\n")));
        return 0;
    }
    //DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("SPI_Init: CSPI index = %d, pCspi=0x%x\r\n"),dwDevIndex,pCspi));
    RETAILMSG (1, (TEXT("SPI_Init: CSPI index = %d, pCspi=0x%x\r\n"),dwDevIndex,pCspi));

    return (DWORD)pCspi;
}


//-----------------------------------------------------------------------------
//
// Function: SPI_Deinit
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
BOOL SPI_Deinit(DWORD hDeviceContext)
{
    cspiClass * pCspi=(cspiClass *)hDeviceContext;

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("SPI_Deinit: hDeviceContext=0x%x\r\n"),hDeviceContext));

    if (pCspi) 
    {
        pCspi->CspiRelease();
        delete pCspi;
    }

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("SPI_Deinit -\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SPI_Open
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
DWORD SPI_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("SPI_Open: hDeviceContext=0x%x\r\n"),hDeviceContext));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

	cspiClass *pCspi = (cspiClass *)hDeviceContext;
	if( pCspi->CspiIOMux( ))
		return hDeviceContext;
	else return NULL;
    
}


//-----------------------------------------------------------------------------
//
// Function: SPI_Close
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
BOOL SPI_Close(DWORD hOpenContext)
{
    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("SPI_Close -\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SPI_PowerDown
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
void SPI_PowerDown(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
}


//-----------------------------------------------------------------------------
//
// Function: SPI_PowerUp
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
void SPI_PowerUp(void)
{
}


//-----------------------------------------------------------------------------
//
// Function: SPI_Read
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
DWORD SPI_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(Count);
    return 0;
}


//-----------------------------------------------------------------------------
//
// Function: SPI_Write
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
DWORD SPI_Write(DWORD hOpenContext, LPCVOID pBuffer, DWORD dwNumBytes)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);
    return 0;
}


//-----------------------------------------------------------------------------
//
// Function: SPI_Seek
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
DWORD SPI_Seek(DWORD hOpenContext, long Amount, WORD Type)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(Amount);
    UNREFERENCED_PARAMETER(Type);
    return (DWORD)-1;
}


//-----------------------------------------------------------------------------
//
// Function: SPI_IOControl
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
//      pBufOut
//          [out] Pointer to the buffer used to transfer the output data
//                  from the device.
//      dwLenOut
//          [in] Maximum number of bytes in the buffer specified by pBufOut.
//      pdwActualOut
//          [out] Pointer to the DWORD buffer that this function uses to
//                  return the actual number of bytes received from the device.
//
// Returns:
//      The new data pointer for the device indicates success. A value of -1
//      indicates failure.
//
//-----------------------------------------------------------------------------
BOOL SPI_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    BOOL bRet = FALSE;
    cspiClass *pCspi = (cspiClass *)hOpenContext;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwLenIn);

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("SPI_IOControl: hOpenContext=0x%x\r\n"),hOpenContext));

    if (pCspi) 
    {
        switch (dwCode) 
        {
            case CSPI_IOCTL_EXCHANGE:
                // enqueue the exchange packet
                if (pBufIn==NULL ||dwLenIn!=sizeof(CSPI_XCH_PKT_T))
                {
                    bRet=FALSE;
                }
                else if(pCspi->CheckPort())
                {
                    bRet = pCspi->CSPI2DataExchange((PCSPI_XCH_PKT_T) pBufIn);
                }
                else
                {
                    bRet = pCspi->CspiEnqueue((PCSPI_XCH_PKT_T) pBufIn);
                }
                DEBUGMSG(!bRet, (TEXT("SPI_IOControl: CSPI_IOCTL_EXCHANGE failed\r\n")));
                break;

            case CSPI_IOCTL_ENABLE_LOOPBACK:
                // Enable Loopback.
                pCspi->CspiEnableLoopback(TRUE);
                bRet = TRUE;
                DEBUGMSG(ZONE_IOCTL|ZONE_FUNCTION, (TEXT("SPI_IOControl: CSPI_IOCTL_ENABLE_LOOPBACK Called\r\n")));
                break;

            case CSPI_IOCTL_DISABLE_LOOPBACK:
                // Disable Loopback.
                pCspi->CspiEnableLoopback(FALSE);
                bRet = TRUE;
                DEBUGMSG(ZONE_IOCTL|ZONE_FUNCTION, (TEXT("SPI_IOControl: CSPI_IOCTL_DISABLE_LOOPBACK Called\r\n")));
                break;

            case IOCTL_POWER_CAPABILITIES:
                // Tell the power manager about ourselves.
                if (pBufOut != NULL 
                    && dwLenOut >= sizeof(POWER_CAPABILITIES) 
                    && pdwActualOut != NULL) 
                {
                    PREFAST_SUPPRESS(6320, "Generic exception handler");
                    __try 
                    {
                        PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;
                        memset(ppc, 0, sizeof(*ppc));              
                        ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D1) | DX_MASK(D4);
                        *pdwActualOut = sizeof(*ppc);
                        bRet = TRUE;
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                        ERRORMSG(TRUE, (_T("Exception in CSPI IOCTL_POWER_CAPABILITIES\r\n")));
                    }
                }

                break;
        
        
            case IOCTL_POWER_SET: 
                if(pBufOut != NULL 
                    && dwLenOut == sizeof(CEDEVICE_POWER_STATE) 
                    && pdwActualOut != NULL) 
                {
                    PREFAST_SUPPRESS(6320, "Generic exception handler");
                    __try 
                    {
                        CEDEVICE_POWER_STATE dx = *(PCEDEVICE_POWER_STATE) pBufOut;
                        if (VALID_DX(dx)) 
                        {
                            // Any request that is not D0 becomes a D4 request
                            if (dx != D0 && dx != D1) 
                            {
                                dx = D4;
                            }

                            *(PCEDEVICE_POWER_STATE) pBufOut = dx;
                            *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                            
                            // change the current power state                            
                            pCspi->m_dxCurrent = dx;

                            // If we are turning off
                            if (dx == D4) 
                            {
                                // polling mode
                                pCspi->m_bUsePolling = TRUE;
                            }
                            // Else we are powering on
                            else 
                            {
                                // Interrupt mode
                                pCspi->m_bUsePolling = FALSE;
                            }

                            // Leave 
                            bRet = TRUE;
                        }
                    } 
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                        ERRORMSG(TRUE, (_T("Exception in CSPI IOCTL_POWER_CAPABILITIES\r\n")));
                    }
                }
                break;
        
            case IOCTL_POWER_GET: 
                if(pBufOut != NULL 
                    && dwLenOut == sizeof(CEDEVICE_POWER_STATE) 
                    && pdwActualOut != NULL) 
                {
                    // Just return our current Dx value
                    PREFAST_SUPPRESS(6320, "Generic exception handler");
                    __try 
                    {
                        *(PCEDEVICE_POWER_STATE) pBufOut = pCspi->m_dxCurrent; //getCurrentPowerState();
                        *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                        bRet = TRUE;
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                        ERRORMSG(TRUE, (_T("Exception in CSPI IOCTL_POWER_CAPABILITIES\r\n")));
                    }
                }
                break;
            default:
                break;
        }
    }

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("SPI_IOControl -\r\n")));

    return bRet;
}

//-----------------------------------------------------------------------------
//  The main dll entry point
//
//  Parameters:
//    hInstDll 
//        The instance that is attaching
//    dwReason  
//        The reason for attaching
//    lpvReserved  
//        Specifies further aspects of DLL initialization and cleanup
//
// Returns:
//      The function returns TRUE if it succeeds 
//      or FALSE if initialization fails. 
//
//-----------------------------------------------------------------------------
BOOL WINAPI CSPI_DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);
    
    switch (dwReason) 
    {
        case DLL_PROCESS_ATTACH:
            DEBUGMSG(ZONE_FUNCTION, (TEXT("CSPI_DllEntry: DLL_PROCESS_ATTACH lpvReserved(0x%x)\r\n"),lpvReserved));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_FUNCTION, (TEXT("CSPI_DllEntry: DLL_PROCESS_DETACH lpvReserved(0x%x)\r\n"),lpvReserved));
            break;
    }
    // return TRUE for success
    return TRUE;
}
