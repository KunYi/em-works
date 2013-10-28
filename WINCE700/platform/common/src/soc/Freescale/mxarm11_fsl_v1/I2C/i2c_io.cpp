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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  i2c_io.cpp
//
//  This module provides a stream interface for the I2C bus
//  driver.  Client drivers can use the stream interface to
//  configure and exchange data with the I2C peripheral.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <windev.h>
#pragma warning(pop)

#include "mxarm11.h"
#include "i2cbus.h"
#include "i2cclass.h"

#include "marshal.hpp" //helper classes to marshal/alloc embedded/async buffer

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
#define I2C_DEVINDEX_MAX_VAL            3
#define I2C_DEVINDEX_MIN_VAL            1
#define I2C_DEVINDEX_DEFAULT_VAL        1


#ifdef DEBUG
DBGPARAM dpCurSettings = {
    TEXT("i2c"), {
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
// Function: I2C_Init
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
DWORD I2C_Init(LPCTSTR pContext)
{    
    UINT32 error;
    HKEY  hKey;
    DWORD  dwIndex, dwSize;

    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("I2C_Init +\r\n")));

    // try to open active device registry key for this context
    hKey = OpenDeviceKey(pContext);
    if (hKey == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("I2C_Init:  OpenDeviceKey failed!!!\r\n")));
        return 0;
    }

    // try to load I2C index from registry data
    dwSize = sizeof(DWORD);
    error = RegQueryValueEx(
        hKey,                       // handle to currently open key
        REG_DEVINDEX_VAL_NAME,      // string containing value to query
        NULL,                       // reserved, set to NULL
        NULL,                       // type not required, set to NULL
        (LPBYTE)(&dwIndex),      // pointer to buffer receiving value
        &dwSize);               // pointer to buffer size

    // close handle to open key
    RegCloseKey(hKey);

    // check for errors during RegQueryValueEx
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("I2C_Init:  RegQueryValueEx failed!!!\r\n")));
        return 0;
    }

    // Construct the I2C Module Class
    I2CClass* pI2C = new I2CClass(dwIndex);

    // Managed to create the class?
    if (pI2C == NULL)
    {
        return NULL;
    }

    // If class construction not successful?
    if (pI2C->IsLastActionOK() != TRUE)
    {
        // Dispose the instance
        DEBUGMSG (ZONE_INIT|ZONE_ERROR, (TEXT("I2C_Init: I2C Class Failed! Err=%d\r\n"), pI2C->GetLastResult()));
        delete pI2C;
        return NULL;
    }

    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("I2C_Init - hDev=0x%x\r\n"), pI2C));

    // Otherwise return the created instance
    return (DWORD) pI2C;
}


//-----------------------------------------------------------------------------
//
// Function: I2C_Deinit
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
BOOL I2C_Deinit(DWORD hDeviceContext)
{
    I2CClass * pI2C = (I2CClass*) hDeviceContext;

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("I2C_Deinit +DeviceContext=0x%x\r\n"),hDeviceContext));

    if (pI2C != NULL)
    {
        delete pI2C;
    }

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("I2C_Deinit -\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: I2C_Open
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
DWORD I2C_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("I2C_Open +hDeviceContext=0x%x\r\n"),hDeviceContext));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("I2C_Open -\r\n")));

    // Open is meaningless!
    return hDeviceContext;
}


//-----------------------------------------------------------------------------
//
// Function: I2C_Close
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
BOOL I2C_Close(DWORD hOpenContext)
{
    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("I2C_Close +\r\n")));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("I2C_Close -\r\n")));

    // Close is meaningless!
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: I2C_PowerDown
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
void I2C_PowerDown(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    // Not implemented!
}


//-----------------------------------------------------------------------------
//
// Function: I2C_PowerUp
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
void I2C_PowerUp(void)
{
    // Not implemented!
}


//-----------------------------------------------------------------------------
//
// Function: I2C_Read
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
DWORD I2C_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
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
// Function: I2C_Write
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
DWORD I2C_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
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
// Function: I2C_Seek
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
DWORD I2C_Seek(DWORD hOpenContext, long Amount, WORD Type)
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
// Function: I2C_IOControl
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
BOOL I2C_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, 
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    BOOL bRet = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pdwActualOut);

    // hOpenContext is a pointer to I2CClass instance!
    I2CClass* pI2C = (I2CClass*) hOpenContext;

    MarshalledBuffer_t Marshalled_pInBuf(pBufIn,dwLenIn, ARG_O_PTR, FALSE, TRUE);
    pBufIn = reinterpret_cast<PBYTE>( Marshalled_pInBuf.ptr() );
    if( (dwLenIn > 0) && (NULL == pBufIn) )
    {
        return bRet;
    }

    MarshalledBuffer_t  Marshalled_pOutBuf(pBufOut, dwLenOut, ARG_O_PTR, FALSE, TRUE);
    pBufOut = reinterpret_cast<PBYTE>( Marshalled_pOutBuf.ptr() );
    if( (dwLenOut > 0) && (NULL == pBufOut) )
    {
        return bRet;
    }
    
    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl: +hOpenContext=0x%x\r\n"),hOpenContext));

    if (pI2C != NULL)
    {
        switch (dwCode)
        {
            case I2C_IOCTL_SET_SLAVE_MODE:
            {
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_SLAVE_MODE +\r\n")));
                pI2C->SetMode(I2C_SLAVE_MODE);
                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_SLAVE_MODE -\r\n")));
                break;
            }
            case I2C_IOCTL_SET_MASTER_MODE:
            {
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_MASTER_MODE +\r\n")));
                pI2C->SetMode(I2C_MASTER_MODE);
                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_MASTER_MODE +\r\n")));
                break;
            }
            case I2C_IOCTL_IS_MASTER:
            {
                if (dwLenOut != sizeof(BOOL))
                    return FALSE;

                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:IS_MASTER +\r\n")));
                PBOOL pbIsMaster = (PBOOL) pBufOut;
                if (pI2C->GetMode() == I2C_MASTER_MODE)
                    *pbIsMaster = TRUE;
                else
                    *pbIsMaster = FALSE;
                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:IS_MASTER - Val=0x%x\r\n"), *pbIsMaster));
                break;
            }
            case I2C_IOCTL_IS_SLAVE:
            {
                if (dwLenOut != sizeof(BOOL))
                    return FALSE;

                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:IS_SLAVE +\r\n")));
                PBOOL pbIsSlave = (PBOOL) pBufOut;
                if (pI2C->GetMode() == I2C_SLAVE_MODE)
                    *pbIsSlave = TRUE;
                else
                    *pbIsSlave = FALSE;
                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:IS_SLAVE - Val=0x%x\r\n"), *pbIsSlave));
                break;
            }
            case I2C_IOCTL_GET_CLOCK_RATE:
            {
                if (dwLenOut != sizeof(WORD))
                    return FALSE;

                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:GET_CLOCK_RATE +\r\n")));
                PWORD pwClkRate = (PWORD) pBufOut;
                *pwClkRate = pI2C->GetClockRateDivider();
                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:GET_CLOCK_RATE - Val=0x%x\r\n"), *pwClkRate));
                break;
            }
            case I2C_IOCTL_SET_CLOCK_RATE:
            {
                if (dwLenIn != sizeof(WORD))
                    return FALSE;

                PWORD pwClkRate = (PWORD) pBufIn;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_CLOCK_RATE + ValIn=0x%x\r\n"), *pwClkRate));
                pI2C->SetClockRateDivider(*pwClkRate);
                bRet = TRUE;
                break;
            }
            case I2C_IOCTL_SET_FREQUENCY:
            {
                if (dwLenIn != sizeof(DWORD))
                    return FALSE;

                PDWORD pdwFrequency = (PDWORD) pBufIn;
                DWORD dwFreq = *pdwFrequency;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_FREQUENCY + ValIn=0x%x\r\n"), dwFreq));
                if (*pdwFrequency > I2C_MAX_FREQUENCY)
                {
                    RETAILMSG (1, (TEXT("I2C_IOControl: I2C frequency may not exceed %d...setting to %d\r\n"), I2C_MAX_FREQUENCY, I2C_MAX_FREQUENCY));
                    dwFreq = I2C_MAX_FREQUENCY;
                    bRet = FALSE;
                }
                WORD wClkRate = BSPCalculateClkRateDiv(dwFreq);
                pI2C->SetClockRateDivider(wClkRate);
                bRet = TRUE;
                break;
            }
            case I2C_IOCTL_SET_SELF_ADDR:
            {
                if (dwLenIn != sizeof(BYTE))
                    return FALSE;

                PBYTE pbySelfAddr = (PBYTE) pBufIn;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_SELF_ADDR + ValIn=0x%x\r\n"), *pbySelfAddr));
                pI2C->SetSelfAddress(*pbySelfAddr);
                bRet = TRUE;
                break;
            }
            case I2C_IOCTL_GET_SELF_ADDR:
            {
                if (dwLenOut != sizeof(BYTE))
                    return FALSE;

                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:GET_SELF_ADDR +\r\n")));
                PBYTE pbySelfAddr = (PBYTE) pBufOut;
                *pbySelfAddr = pI2C->GetSelfAddress();
                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:GET_SELF_ADDR - Val=0x%x\r\n"), *pbySelfAddr));
                break;
            }
            case I2C_IOCTL_TRANSFER:
            {
                I2C_TRANSFER_BLOCK *pXferBlock = (I2C_TRANSFER_BLOCK *) pBufIn;
                I2C_PACKET *pPackets = (I2C_PACKET *) pXferBlock->pI2CPackets;

                // Map pointers for each packet in the array
                for (int i = 0; i < pXferBlock->iNumPackets; i++)
                {
                    pPackets[i].pbyBuf = (PBYTE) pPackets[i].pbyBuf;
                }

                bRet = pI2C->ProcessPackets(pPackets, pXferBlock->iNumPackets);
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:I2C_IOCTL_TRANSFER -\r\n")));
                break;
            }
            case I2C_IOCTL_RESET:
            {
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:RESET +\r\n")));

                pI2C->Reset();

                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:RESET -\r\n")));
                break;
            }
            default:
            {
                bRet = FALSE;
                break;
            }
        }
                
    }

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl -\r\n")));

    return bRet;
}

BOOL WINAPI I2C_DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HINSTANCE) hInstDll);
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2C_DllEntry: DLL_PROCESS_ATTACH lpvReserved(0x%x)\r\n"),lpvReserved));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;
         
        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2C_DllEntry: DLL_PROCESS_DETACH lpvReserved(0x%x)\r\n"),lpvReserved));
            break;
    }
    // return TRUE for success
    return TRUE;
}