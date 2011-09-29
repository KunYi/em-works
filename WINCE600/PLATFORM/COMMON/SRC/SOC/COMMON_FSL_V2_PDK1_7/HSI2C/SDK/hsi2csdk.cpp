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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  hsi2csdk.cpp
//
//  The implementation of HSI2C driver SDK.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include "ceddk.h"
#pragma warning(pop)

#include "common_macros.h"
#include "hsi2cbus.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
#define HSI2C_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define HSI2C_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT       0
#define ZONEID_INFO       12
#define ZONEID_FUNCTION   13
#define ZONEID_WARN       14
#define ZONEID_ERROR      15

// Debug zone masks
#define ZONEMASK_INIT     (1<<ZONEID_INIT)
#define ZONEMASK_INFO     (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN     (1<<ZONEID_WARN)
#define ZONEMASK_ERROR    (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT         DEBUGZONE(ZONEID_INIT)
#define ZONE_INFO         DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION     DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN         DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR        DEBUGZONE(ZONEID_ERROR)
#endif


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
#ifdef DEBUG
extern DBGPARAM dpCurSettings =
{
    _T("HSI2C"),
    {
        _T("Init"), _T(""), _T(""), _T(""),
        _T(""), _T(""), _T(""), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN | ZONEMASK_INFO
};
#endif


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: HSI2COpenHandle
//
// This method creates a handle to the HSI2C stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to HSI2C driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE HSI2COpenHandle(LPCWSTR lpDevName)
{
    HANDLE hI2C;

    HSI2C_FUNCTION_ENTRY();
    
    hI2C = CreateFile(lpDevName,            // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to HSI2C
    if (hI2C == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s:  CreateFile HSI2C failed!\r\n"), __WFUNCTION__));
        return hI2C;
    }

    HSI2C_FUNCTION_EXIT();

    return hI2C;
}

//------------------------------------------------------------------------------
//
// Function: HSI2CCloseHandle
//
// This method closes a handle to the HSI2C stream driver.
//
// Parameters:
//      hI2C
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL HSI2CCloseHandle(HANDLE hI2C)
{
    HSI2C_FUNCTION_ENTRY();

    // if we don't have handle to HSI2C bus driver
    if (hI2C != NULL)
    {
        if (!CloseHandle(hI2C))
        {
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: CloseHandle failed!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    HSI2C_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CSetSlaveMode
//
// This function sets the HSI2C device in slave mode.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CSetSlaveMode(HANDLE hI2C)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_SET_SLAVE_MODE,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_SET_SLAVE_MODE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CSetMasterMode
//
// This function sets the HSI2C device in master mode.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CSetMasterMode(HANDLE hI2C)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_SET_MASTER_MODE,  // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_SET_MASTER_MODE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CIsMaster
//
// This function determines whether the HSI2C is currently in Master mode.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
//      pbIsMaster
//          [out] TRUE if the HSI2C device is in Master mode.
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CIsMaster(HANDLE hI2C, PBOOL pbIsMaster)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_IS_MASTER,        // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        pbIsMaster,                 // out buffer
        sizeof(BOOL),               // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_IS_MASTER failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CIsSlave
//
// This function determines whether the HSI2C is currently in Slave mode.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
//      pbIsSlave
//          [out] The pointer to BOOL variable that indicates 
//          if the HSI2C device is in Slave mode.
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CIsSlave(HANDLE hI2C, PBOOL pbIsSlave)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_IS_SLAVE,         // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        pbIsSlave,                  // out buffer
        sizeof(BOOL),               // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_IS_SLAVE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CGetClockRate
//
// This function will retrieve the clock rate divisor. Note that the value 
// is not the absolute peripheral clock frequency. The value retrieved should 
// be compared against the HSI2C specifications to obtain the true frequency.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
//      pwClkRate
//          [out] The pointer of WORD variable that retrieves divisor index.
//          Refer to HSI2C specification to obtain the true clock frequency.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CGetClockRate(HANDLE hI2C, PWORD pwClkRate)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_GET_CLOCK_RATE,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        pwClkRate,                  // out buffer
        sizeof(WORD),               // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_GET_CLOCK_RATE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CSetClockRate
//
// This function will initialize the HSI2C device with the given clock rate. Note
// that this function does not expect to receive the absolute peripheral clock
// frequency. Rather, it will be expecting the clock rate divisor index stated
// in the HSI2C specification. If absolute clock frequency must be used, please
// use the function I2CSetFrequency().
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
//      wClkRate
//          [in] Contains the divisor index. Refer to HSI2C specification to
//          obtain the true clock frequency.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CSetClockRate(HANDLE hI2C, WORD wClkRate)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_SET_CLOCK_RATE,   // I/O control code
        &wClkRate,                  // in buffer
        sizeof(WORD),               // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_SET_CLOCK_RATE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CSetFrequency
//
// This function will estimate the nearest clock rate acceptable for 
// HSI2C device and initialize the HSI2C device to use the estimated  clock 
// rate divisor. If the estimated clock rate divisor index is required, 
// please refer to the macro HSI2CGetClockRate to determine the estimated index.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
//      dwFreq
//          [in] The desired frequency.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CSetFrequency(HANDLE hI2C, DWORD dwFreq)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_SET_FREQUENCY,    // I/O control code
        &dwFreq,                    // in buffer
        sizeof(DWORD),              // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_SET_FREQUENCY failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CSetSelfAddr
//
// This function will initialize the HSI2C device with the given address. 
// The device will be expected to respond when any master within the 
// HSI2C bus wish to proceed with any transfer.
// 
// Note that this function will have no effect if the HSI2C device is 
// in Master mode.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
//      bySelfAddr
//          [in] The expected HSI2C device address. The valid range of address
//          is [0x00, 0x7F].
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CSetSelfAddr(HANDLE hI2C, BYTE bySelfAddr)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_SET_SELF_ADDR,    // I/O control code
        &bySelfAddr,                // in buffer
        sizeof(BYTE),               // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_SET_SELF_ADDR failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CGetSelfAddr
//
// This function will retrieve the address of the HSI2C device.
//
// Note that this function is only meaningful if it is currently in Slave mode.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
//      pbySelfAddr
//          [out] The pointer to BYTE variable that retrieves HSI2C device address. 
//          The valid range of address is [0x00, 0x7F].
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CGetSelfAddr(HANDLE hI2C, PBYTE pbySelfAddr)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_GET_SELF_ADDR,    // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        pbySelfAddr,                // out buffer
        sizeof(BYTE),               // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_GET_SELF_ADRR failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CTransfer
//
// This function performs one or more HSI2C read or write operations.  
// pHSI2CTransferBlock contains a pointer to the first of an array of HSI2C 
// packets to be processed by the HSI2C.  All the required information for the 
// HSI2C operations should be contained in the array elements of pI2CPackets.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
//      pHSI2CTransferBlock
//          [in] The pointer of HSI2C_TRANSFER_BLOCK structure type.
//          The fields of HSI2C_TRANSFER_BLOCK are described below:
//              pI2CPackets - Pointer to the HSI2C Packets to transfer.
//              iNumPackets - Number of packets in pI2CPackets array.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CTransfer(HANDLE hI2C, PHSI2C_TRANSFER_BLOCK pHSI2CTransferBlock)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_TRANSFER,         // I/O control code
        pHSI2CTransferBlock,          // in buffer
        sizeof(HSI2C_TRANSFER_BLOCK), // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_TRANSFER failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CReset
//
// This function performs a hardware reset. Note that the HSI2C driver will still
// maintain all the current information of the device, which includes all the
// initialized addresses.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CReset(HANDLE hI2C)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_RESET,            // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_RESET failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CEnableSlave
//
// This function enables a HSI2C slave access from the bus. Note that after 
// the HSI2C slave interface enabled, HSI2C slave driver wait for master access
// all the time.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CEnableSlave(HANDLE hI2C)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_ENABLE_SLAVE,     // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_ENABLE_SLAVE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CDisableSlave
//
// This function disables HSI2C slave access from the bus. Note that after 
// the HSI2C slave interface disabled, HSI2C slave module can be turned off.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CDisableSlave(HANDLE hI2C)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_DISABLE_SLAVE,    // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_DISABLE_SLAVE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CGetSlaveSize
//
// This function returns HSI2C slave interface buffer length. Note that HSI2C 
// slave driver directly return data to master from interface buffer. 
// The interface buffer can be set at any time, even when HSI2C slave module 
// has been turned off.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
//      pdwSize
//          [Out] Pointer to BYTE variable that retrieves interface buffer length.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CGetSlaveSize(HANDLE hI2C, PDWORD pdwSize)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_GET_SLAVESIZE,    // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        pdwSize,                    // out buffer
        sizeof(DWORD),              // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_GET_SLAVESIZE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CSetSlaveSize
//
// This function sets HSI2C slave interface buffer length. The max acceptable 
// length is I2CSLAVEBUFSIZE. If input length is longer than I2CSLAVEBUFSIZE, 
// the operation will fail, and origin buffer length not changed. Note that 
// HSI2C slave driver directly return data to master from interface buffer. 
// The interface buffer can be set at any time, even when HSI2C slave module 
// has been turned off.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
//      bySize
//          [in] Slave interface buffer length.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CSetSlaveSize(HANDLE hI2C, DWORD dwSize)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_SET_SLAVESIZE,    // I/O control code
        &dwSize,                    // in buffer
        sizeof(DWORD),              // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_SET_SLAVESIZE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CGetSlaveText
//
// This function returns HSI2C slave interface buffer text. Note that HSI2C 
// slave driver directly returns data to master from interface buffer. 
// The interface buffer can be accessed at any time, even when HSI2C slave 
// module has been turned off.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from I2COpenHandle().
//
//      pbyTextBuf
//          [out] User buffer to store text returned from interface buffer.
//
//      dwBufSize
//          [in] User buffer size.
//
//      pdwTextLen
//          [out] Actual data bytes returned.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CGetSlaveText(HANDLE hI2C, PBYTE pbyTextBuf, DWORD dwBufSize, PDWORD pdwTextLen)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_GET_SLAVE_TXT,    // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        pbyTextBuf,                 // out buffer
        dwBufSize,                  // out buffer size
        pdwTextLen,                 // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_GET_SLAVE_TXT failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CSetSlaveText
//
// This function stores HSI2C slave text to interface buffer. Note that 
// HSI2C slave driver directly return data to master from interface buffer. 
// The interface buffer can be accessed at any time, even when HSI2C slave 
// module has been turned off.
//
// Parameters:
//      hI2C
//          [in] The HSI2C device handle retrieved from HSI2COpenHandle().
//
//      pbyTextBuf
//          [out] User buffer to store text to interface buffer.
//
//      dwTextLen
//          [in] Text length in user buffer.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL HSI2CSetSlaveText(HANDLE hI2C, PBYTE pbyTextBuf, DWORD dwTextLen)
{
    HSI2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        HSI2C_IOCTL_SET_SLAVE_TXT,    // I/O control code
        pbyTextBuf,                 // in buffer
        dwTextLen,                  // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_SET_SLAVE_TXT failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    HSI2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the HSI2C DDK module.  This
// function is called when processed and threads attach and detach from this
// module.
//
// Parameters:
//      hInstDll
//           [in] The handle to this module.
//
//      dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//      lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the HSI2C is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
extern "C"
BOOL WINAPI
DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HMODULE)hInstDll);
            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS ATTACH TO HSI2C SDK *****\r\n")));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS DETACH FROM HSI2C SDK *****\r\n")));
            break;
    }

    // return TRUE for success
    return TRUE;
}

