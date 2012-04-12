//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  lradc.cpp
//
//  This file  provides a stream interface for the LRADC module
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"
#include "lradc_class.h"
//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Local Variables
PVOID pv_HWregLRADC = NULL;

//-----------------------------------------------------------------------------
// Local Functions
BOOL LRADCDeInit(void);
BOOL LRADCAlloc(void);
BOOL LRADCDealloc(void);
static BOOL LRADCValidChan(UINT8 Channel);

//-----------------------------------------------------------------------------
//
//  Function:  LRADC
//
//  This function
//
//  Parameters:
//
//      Channel
//          [in] - channel number.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function:  LRADCAlloc
//
// This function allocates the data structures required for interaction
// with LRADC.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL LRADCAlloc(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    if (pv_HWregLRADC == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_LRADC;

        // Map peripheral physical address to virtual address
        pv_HWregLRADC = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregLRADC == NULL)
        {
            ERRORMSG(1, (_T("LRADCAlloc::MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:
    if (!rc) LRADCDealloc();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  LRADCDealloc
//
// This function deallocates the data structures required for interaction
// with LRADC.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL LRADCDealloc(void)
{
    // Unmap peripheral registers
    if (pv_HWregLRADC)
    {
        MmUnmapIoSpace(pv_HWregLRADC, 0x1000);
        pv_HWregLRADC = NULL;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: LDC_Init
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
DWORD LDC_Init()
{
    BOOL rc = FALSE;

    STLRADCCONFIGURE stLradconfig;

    //Map the LRADC register space for access
    if(!LRADCAlloc())
    {
        ERRORMSG(1, (_T("LRADCAlloc::MmMapIoSpace failed!\r\n")));
        return NULL;
    }

    LRADCClass* pLRADC = new LRADCClass();

    if( pLRADC == NULL )
    {
        return NULL;
    }

    // Initial parameters
    stLradconfig.bEnableOnChipGroundRef = TRUE;
    stLradconfig.eFreq = LRADC_CLOCK_6MHZ;

    rc = pLRADC->Init(&stLradconfig);
    if(rc == FALSE)
    {
        ERRORMSG(1, (_T("LDC_Init:pLRADC->Init failed!\r\n")));
        return NULL;
    }
    // Managed to create the class?
    if (pLRADC == NULL)
    {
        return NULL;

    }

    return (DWORD) pLRADC;
}

//-----------------------------------------------------------------------------
//
// Function: LDC_Deinit
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
BOOL LDC_Deinit(DWORD hDeviceContext)
{

    LRADCClass* pLRADC = (LRADCClass*) hDeviceContext;
    pLRADC->DeInit();

    if (pLRADC != NULL)
    {
        delete pLRADC;
    }

    //UnMap the LRADC register space
    LRADCDealloc();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: LDC_Open
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
DWORD LDC_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);
    UNREFERENCED_PARAMETER(hDeviceContext);

    // Open is meaningless!
    return 1;
}

//-----------------------------------------------------------------------------
//
// Function: LDC_Close
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
BOOL LDC_Close(DWORD hOpenContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    // Close is meaningless!
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: LDC_PowerDown
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
void LDC_PowerDown(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    // Not implemented!
}
//-----------------------------------------------------------------------------
//
// Function: LDC_PowerUp
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
void LDC_PowerUp(void)
{
    // Not implemented!
}

//-----------------------------------------------------------------------------
//
// Function: LDC_Read
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
DWORD LDC_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
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
// Function: LDC_Write
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
DWORD LDC_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
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
// Function: LDC_Seek
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
DWORD LDC_Seek(DWORD hOpenContext, long Amount, WORD Type)
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
// Function: LDC_IOControl
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
BOOL LDC_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    BOOL bRet = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pdwActualOut);

    // hOpenContext is a pointer to LRADCClass instance!
    LRADCClass* pLRADC = (LRADCClass*) hOpenContext;


    //DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl: +hOpenContext=0x%x\r\n"),hOpenContext));

    if (pLRADC != NULL)
    {
        switch (dwCode)
        {
        case IOCTL_POWER_CAPABILITIES:
        {
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
                    ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D4);
                    *pdwActualOut = sizeof(*ppc);
                    bRet = TRUE;
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    ERRORMSG(TRUE, (_T("Exception in CSPI IOCTL_POWER_CAPABILITIES\r\n")));
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
                        //pI2C->m_dxCurrent = dx;

                        // If we are turning off
                        if (dx == D4)
                        {
                            // polling mode
                            //  pI2C->m_bUsePolling = TRUE;
                        }
                        // Else we are powering on
                        else
                        {
                            // Interrupt mode
                            //pI2C->m_bUsePolling = FALSE;
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
        }
        case IOCTL_POWER_GET:
        {
            if(pBufOut != NULL
               && dwLenOut == sizeof(CEDEVICE_POWER_STATE)
               && pdwActualOut != NULL)
            {
                // Just return our current Dx value
                PREFAST_SUPPRESS(6320, "Generic exception handler");
                __try
                {
                    //*(PCEDEVICE_POWER_STATE) pBufOut = pI2C->m_dxCurrent; //getCurrentPowerState();
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                    bRet = TRUE;
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    ERRORMSG(TRUE, (_T("Exception in CSPI IOCTL_POWER_CAPABILITIES\r\n")));
                }
            }
            break;
        }
        default:
        {
            dwLenIn = NULL;
            bRet = pLRADC->IOControl(hOpenContext,dwCode,pBufIn,pBufOut);
            break;
        }
        }

    }

    //   DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl -\r\n")));

    return bRet;
}
