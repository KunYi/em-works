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
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  i2csdk.cpp
//
//  The implementation of I2C driver SDK.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include "ceddk.h"
#pragma warning(pop)

#include "common_macros.h"
#include "i2cbus.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
#define I2C_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define I2C_FUNCTION_EXIT() \
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
    _T("I2C"),
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
// Function: I2COpenHandle
//
// This method creates a handle to the I2C stream driver.
//
// Parameters:
//      lpDevName
//          [in] name of device driver to open
//
// Returns:
//      Handle to I2C driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE I2COpenHandle(LPCWSTR lpDevName)
{
    HANDLE hI2C;

    I2C_FUNCTION_ENTRY();
    
    hI2C = CreateFile(lpDevName,            // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to I2C
    if (hI2C == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s:  CreateFile I2C failed!\r\n"), __WFUNCTION__));
        return hI2C;
    }

    I2C_FUNCTION_EXIT();

    return hI2C;
}

//------------------------------------------------------------------------------
//
// Function: I2CCloseHandle
//
// This method closes a handle to the I2C stream driver.
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
BOOL I2CCloseHandle(HANDLE hI2C)
{
    I2C_FUNCTION_ENTRY();

    // if we don't have handle to I2C bus driver
    if (hI2C != NULL)
    {
        if (!CloseHandle(hI2C))
        {
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: CloseHandle failed!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    I2C_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CSetSlaveMode
//
// This function sets the I2C device in slave mode.
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CSetSlaveMode(HANDLE hI2C)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_SET_SLAVE_MODE,   // I/O control code
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

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CSetMasterMode
//
// This function sets the I2C device in master mode.
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CSetMasterMode(HANDLE hI2C)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_SET_MASTER_MODE,  // I/O control code
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

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CIsMaster
//
// This function determines whether the I2C is currently in Master mode.
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
//      pbIsMaster
//          [out] TRUE if the I2C device is in Master mode.
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CIsMaster(HANDLE hI2C, PBOOL pbIsMaster)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_IS_MASTER,        // I/O control code
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

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CIsSlave
//
// This function determines whether the I2C is currently in Slave mode.
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
//      pbIsSlave
//          [out] The pointer to BOOL variable that indicates 
//          if the I2C device is in Slave mode.
//
// Returns:
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CIsSlave(HANDLE hI2C, PBOOL pbIsSlave)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_IS_SLAVE,         // I/O control code
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

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CGetClockRate
//
// This function will retrieve the clock rate. 
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
//      pdwClkRate
//          [out] The pointer of WORD variable that retrieves clock rate.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CGetClockRate(HANDLE hI2C, PDWORD pdwClkRate)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_GET_CLOCK_RATE,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        pdwClkRate,                  // out buffer
        sizeof(WORD),               // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_GET_CLOCK_RATE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CSetClockRate
//
// This function will initialize the I2C device with the given clock rate. Note
// that this function does not expect to receive the absolute peripheral clock
// frequency. It will set either 100 khz for std mode, or 400 khz for fast-mode
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
//      dwClkRate
//          [in] Contains the clock rate. 
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CSetClockRate(HANDLE hI2C, DWORD dwClkRate)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_SET_CLOCK_RATE,   // I/O control code
        &dwClkRate,                  // in buffer
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

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: I2CSetSelfAddr
//
// This function will initialize the I2C device with the given address. 
// The device will be expected to respond when any master within the 
// I2C bus wish to proceed with any transfer.
// 
// Note that this function will have no effect if the I2C device is 
// in Master mode.
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
//      bySelfAddr
//          [in] The expected I2C device address. The valid range of address
//          is [0x00, 0x7F].
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CSetSelfAddr(HANDLE hI2C, BYTE bySelfAddr)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_SET_SELF_ADDR,    // I/O control code
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

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CGetSelfAddr
//
// This function will retrieve the address of the I2C device.
//
// Note that this function is only meaningful if it is currently in Slave mode.
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
//      pbySelfAddr
//          [out] The pointer to BYTE variable that retrieves I2C device address. 
//          The valid range of address is [0x00, 0x7F].
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CGetSelfAddr(HANDLE hI2C, PBYTE pbySelfAddr)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_GET_SELF_ADDR,    // I/O control code
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

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CTransfer
//
// This function performs one or more I2C read or write operations.  
// pI2CTransferBlock contains a pointer to the first of an array of I2C 
// packets to be processed by the I2C.  All the required information for the 
// I2C operations should be contained in the array elements of pI2CPackets.
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
//      pI2CTransferBlock
//          [in] The pointer of I2C_TRANSFER_BLOCK structure type.
//          The fields of I2C_TRANSFER_BLOCK are described below:
//              pI2CPackets - Pointer to the I2C Packets to transfer.
//              iNumPackets - Number of packets in pI2CPackets array.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CTransfer(HANDLE hI2C, PI2C_TRANSFER_BLOCK pI2CTransferBlock)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_TRANSFER,         // I/O control code
        pI2CTransferBlock,          // in buffer
        sizeof(I2C_TRANSFER_BLOCK), // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: I2C_IOCTL_TRANSFER failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CReset
//
// This function performs a hardware reset. Note that the I2C driver will still
// maintain all the current information of the device, which includes all the
// initialized addresses.
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CReset(HANDLE hI2C)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_RESET,            // I/O control code
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

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CEnableSlave
//
// This function enables a I2C slave access from the bus. Note that after 
// the I2C slave interface enabled, I2C slave driver wait for master access
// all the time.
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CEnableSlave(HANDLE hI2C)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_ENABLE_SLAVE,     // I/O control code
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

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CDisableSlave
//
// This function disables I2C slave access from the bus. Note that after 
// the I2C slave interface disabled, I2C slave module can be turned off.
//
// Parameters:
//      hI2C
//          [in] The I2C device handle retrieved from I2COpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL I2CDisableSlave(HANDLE hI2C)
{
    I2C_FUNCTION_ENTRY();

    if (!DeviceIoControl(hI2C,      // file handle to the driver
        I2C_IOCTL_DISABLE_SLAVE,    // I/O control code
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

    I2C_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the I2C DDK module.  This
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
//      TRUE if the I2C is initialized; FALSE if an error occurred during
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
                     (_T("***** DLL PROCESS ATTACH TO I2C SDK *****\r\n")));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS DETACH FROM I2C SDK *****\r\n")));
            break;
    }

    // return TRUE for success
    return TRUE;
}

