//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  spi_io.cpp
//
//   This module provides a stream interface for the SPI bus
//   driver.  Client drivers can use the stream interface to
//   configure and exchange data with the SPI peripheral.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

#include "spiclass.h"
//-----------------------------------------------------------------------------
// External Functions
// zxw 2012-06-11
extern "C" BOOL BSPSPISetIOMux(UINT32 Index);
extern "C" BOOL BSPSPIReleaseIOMux(UINT32 dwIndex);

// zxw 2012-6-27
extern "C" PBYTE BSPSPIGetDataBuffer(LPVOID pBuf, DWORD dwLength);
extern "C" DWORD BSPSPIGetDataLength(LPVOID pBuf, DWORD dwLength);
extern "C" BOOL BSPSPIGetLCS(LPVOID pBuf, DWORD dwLength);
extern "C" BYTE BSPSPIGetBitDataLength(LPVOID pBuf, DWORD dwLength);

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
#define REG_DEVINDEX_VAL_NAME           TEXT("Index")
#define REG_CSINDEX_VAL_NAME            TEXT("CSIndex")

#ifdef DEBUG
DBGPARAM dpCurSettings = 
{
    TEXT("spi"), {
        TEXT("Init"),TEXT("Deinit"),TEXT("Open"),TEXT("Close"),
        TEXT("IOCtl"),TEXT("Thread"),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT("Function"),TEXT("Warning"),TEXT("Error") },
    0xC000
};
#endif // DEBUG
//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Functions
BOOL SPIDeInit(void);
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
//      zero if not successful.
//
//-----------------------------------------------------------------------------
DWORD SPI_Init(LPCTSTR pContext)
{
    BOOL rc = FALSE;
    LONG    regError;
    HKEY    hKey;
    DWORD   dwDataSize;
    DWORD   dwDevIndex;
	DWORD	dwCSIndex = (DWORD)-1;
    spiClass *pSpi = NULL;

    // try to open active device registry key for this context
    hKey = OpenDeviceKey(pContext);
    if (hKey == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SPI_Init:  OpenDeviceKey failed!!!\r\n")));
        return 0;
    }

    // try to load SPI index from registry data
    dwDataSize = sizeof(DWORD);
    regError = RegQueryValueEx(
        hKey,                       // handle to currently open key
        REG_DEVINDEX_VAL_NAME,      // string containing value to query
        NULL,                       // reserved, set to NULL
        NULL,                       // type not required, set to NULL
        (LPBYTE)(&dwDevIndex),      // pointer to buffer receiving value
        &dwDataSize);               // pointer to buffer size

    // check for errors during RegQueryValueEx
    if (regError != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SPI_Init:  RegQueryValueEx failed!!!\r\n")));
		// close handle to open key
		RegCloseKey(hKey);
        return NULL;
    }

    // try to load SPI CS index from registry data
    dwDataSize = sizeof(DWORD);
    regError = RegQueryValueEx(
        hKey,                       // handle to currently open key
        REG_CSINDEX_VAL_NAME,       // string containing value to query
        NULL,                       // reserved, set to NULL
        NULL,                       // type not required, set to NULL
        (LPBYTE)(&dwCSIndex),       // pointer to buffer receiving value
        &dwDataSize);               // pointer to buffer size

    // check for errors during RegQueryValueEx
    if (regError != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SPI_Init:  CSIndex NOT available!\r\n")));
		dwCSIndex = (DWORD)-1;
    }

	// close handle to open key
    RegCloseKey(hKey);

    pSpi = new spiClass();

    // Check if Class creation has Failed
    if (!pSpi)
    {
        DEBUGMSG(ZONE_ERROR,(L"ERROR: m_pSPI null pointer!\r\n"));
        return NULL;
    }
    rc = pSpi->SpiInitialize(dwDevIndex);

    if(rc == FALSE)
    {  
        pSpi->SpiRelease();
        delete pSpi;
        ERRORMSG(1, (_T("SPI_Init:pSPI->SpiInitialize failed!\r\n")));
        return NULL;
    }
    else
    {
        RETAILMSG(1, (TEXT("SPI_Init: SpiInitialize Sucess: pSpi  0x%x !!\r\n"),pSpi));
		pSpi->SpiSetCSIndex(dwCSIndex);
    }

    // Managed to create the class?
    if (pSpi == NULL)
    {
        return NULL;
    }

    return (DWORD) pSpi;
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

    spiClass* pSpi = (spiClass*) hDeviceContext;

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("SPI_Deinit: hDeviceContext=0x%x\r\n"),hDeviceContext));

    if (pSpi != NULL)

    {   pSpi->SpiRelease();
        delete pSpi;
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
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

	// zxw 2012-06-11
	spiClass* pSPI = (spiClass*) hDeviceContext;

	if( BSPSPISetIOMux( pSPI->m_Index ) )	
		RETAILMSG(1, (TEXT("SPI_Opened SPI:%d\r\n"), hDeviceContext));

    return hDeviceContext;
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
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

	// zxw 2012-06-11
	spiClass* pSPI = (spiClass*) hOpenContext;

	if( BSPSPIReleaseIOMux( pSPI->m_Index ) )	
		RETAILMSG(1, (TEXT("SPI_Closed SPI:%d\r\n"), hOpenContext));

    // Close is meaningless!
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
    // Not implemented!
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
    // Not implemented!
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

// zxw 2012-6-27
	spiClass *pSPI = (spiClass*)hOpenContext;

	PBYTE    pBuf;
	DWORD    dLen;
	BOOL     bCS;
	BYTE     sBitcount;

	bCS = BSPSPIGetLCS( pBuffer , Count );
	pBuf = BSPSPIGetDataBuffer( pBuffer , Count );
	dLen = BSPSPIGetDataLength( pBuffer , Count );
	sBitcount = BSPSPIGetBitDataLength( pBuffer , Count );

	return pSPI->MasterRead( bCS , pBuf , dLen , sBitcount );

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
//DWORD SPI_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
DWORD SPI_Write(DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

	// zxw 2012-6-27
	spiClass *pSPI = (spiClass*)Handle;
	PBYTE    pBuf;
	DWORD    dLen;
	BOOL     bCS;
	BYTE     sBitcount;


	bCS = BSPSPIGetLCS( pBuffer , dwNumBytes );
	pBuf = BSPSPIGetDataBuffer( pBuffer , dwNumBytes );
	dLen = BSPSPIGetDataLength( pBuffer , dwNumBytes );
	sBitcount = BSPSPIGetBitDataLength( pBuffer , dwNumBytes );

	return pSPI->MasterWrite( bCS , pBuf , dLen , sBitcount );

    // Nothing to write
    //return 0;
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

    // Seeking is meaningless!
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
BOOL SPI_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    BOOL bRet = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pdwActualOut);

    UNREFERENCED_PARAMETER(dwLenIn);

    UNREFERENCED_PARAMETER(pBufIn);

    // hOpenContext is a pointer to spiClass instance!
    spiClass* pSpi = (spiClass*) hOpenContext;


    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("SPI_IOControl: hOpenContext=0x%x\r\n"),hOpenContext));

    if (pSpi != NULL)
    {
        switch (dwCode)
        {
//#ifdef POLLING_MODE
        //DEBUGMSG(1, (TEXT("SPI_IOControl: case SPI_IOCTL_EXCHANGE:\r\n")));
        case SPI_IOCTL_EXCHANGE:
            // enqueue the exchange packet
            if (pBufIn==NULL ||dwLenIn!=sizeof(SPI_XCH_PKT_T))           
            {
                bRet=FALSE;
            }
            else
            {
                bRet = pSpi->SpiEnqueue((PSPI_XCH_PKT_T) pBufIn);
            }
            DEBUGMSG(!bRet, (TEXT("SPI_IOControl: SPI_IOCTL_EXCHANGE failed\r\n")));
            break;
//#endif

        case SPI_IOCTL_SSPCONFIGURE:
            bRet = pSpi->ConfigureSSP((SSP_INIT *) pBufIn);
            break;

        case SPI_IOCTL_SSPRESETAFTERERROR:
            bRet = pSpi->SSPResetAfterError((SSP_RESETCONFIG *) pBufIn);
            break;

        case SPI_IOCTL_SSPCHECKERRORS:
            bRet = pSpi->SSPCheckErrors();
            break;

        case SPI_IOCTL_SSPGETIRQSTATUS:
            bRet = pSpi->SSPGetIrqStatus(*(SSP_IRQ *)pBufIn) ;
            break;

        case SPI_IOCTL_SSPCLEARIRQ:
            bRet = pSpi->SSPClearIrq(*(SSP_IRQ *)pBufIn) ;
            break;

        case SPI_IOCTL_SSPCONFIGTIMING:
            bRet = pSpi->SSPConfigTiming(*(SSP_SPEED *)pBufIn) ;
            break;
        case SPI_IOCTL_SSP_EN_ERROR:
            bRet = pSpi->SSPEnableErrIrq(*(BOOL *)pBufIn) ;
            break;

        case SPI_IOCTL_SSP_DIS_ERROR:
            bRet = pSpi->SSPDisableErrIrq() ;
            break;

        case IOCTL_POWER_CAPABILITIES:
        {
            // Tell the power manager about ourselves.
            if (pBufOut != NULL
                && dwLenOut >= sizeof(POWER_CAPABILITIES)
                && pdwActualOut != NULL)
            {
                __try
                {
                    PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;
                    memset(ppc, 0, sizeof(*ppc));
                    ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D4);
                    *pdwActualOut = sizeof(*ppc);
                    bRet = TRUE;
                }
                __except(GetExceptionCode()==EXCEPTION_EXECUTE_HANDLER)
                {
                    ERRORMSG(TRUE, (_T("Exception in SPI IOCTL_POWER_CAPABILITIES\r\n")));
                }
            }

            break;
        }
        case IOCTL_POWER_SET:
        {
            if(pBufOut != NULL
               && dwLenOut == sizeof(CEDEVICE_POWER_STATE)
               && pdwActualOut != NULL)
            {
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
                        pSpi->m_dxCurrent = dx;

                        // If we are turning off
                        if (dx == D4)
                        {
                            // Interrupt mode
                            pSpi->m_bUsePolling = TRUE;
                        }
                        // Else we are powering on
                        else
                        {
                            // Interrupt mode
                            pSpi->m_bUsePolling = FALSE;
                        }

                        // Leave
                        bRet = TRUE;
                    }
                }
                __except(GetExceptionCode()==EXCEPTION_EXECUTE_HANDLER)
                {
                    ERRORMSG(TRUE, (_T("Exception in SPI IOCTL_POWER_CAPABILITIES\r\n")));
                }
            }
            break;
        }
        case IOCTL_POWER_GET:
        {
            if(pBufOut != NULL
               && dwLenOut == sizeof(CEDEVICE_POWER_STATE)
               && pdwActualOut != NULL)
            {
                // Just return our current Dx value
                __try
                {
                    *(PCEDEVICE_POWER_STATE) pBufOut = pSpi->m_dxCurrent; //getCurrentPowerState();
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                    bRet = TRUE;
                }
                __except(GetExceptionCode()==EXCEPTION_EXECUTE_HANDLER)
                {
                    ERRORMSG(TRUE, (_T("Exception in SPI IOCTL_POWER_CAPABILITIES\r\n")));
                }
            }
            break;
        }
        default:
           break;
        }
    }

     DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("SPI_IOControl -\r\n")));

    return bRet;
}

