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
//  File:  cansdk.cpp
//
//  The implementation of CAN driver SDK.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include "ceddk.h"
#pragma warning(pop)
#include "common_macros.h"
#include "canbus.h"
//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
#define CAN_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define CAN_FUNCTION_EXIT() \
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
    _T("CAN"),
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
// Function: CANOpenHandle
//
// This method creates a handle to the CAN stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to CAN driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE CANOpenHandle(LPCWSTR lpDevName)
{
    HANDLE hCAN;

    CAN_FUNCTION_ENTRY();
    
    hCAN = CreateFile(lpDevName,            // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to CAN
    if (hCAN == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s:  CreateFile CAN failed!\r\n"), __WFUNCTION__));
        return hCAN;
    }

    CAN_FUNCTION_EXIT();

    return hCAN;
}

//------------------------------------------------------------------------------
//
// Function: CANCloseHandle
//
// This method closes a handle to the CAN stream driver.
//
// Parameters:
//      hCAN
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL CANCloseHandle(HANDLE hCAN)
{
    CAN_FUNCTION_ENTRY();

    // if we don't have handle to CAN bus driver
    if (hCAN != NULL)
    {
        if (!CloseHandle(hCAN))
        {
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: CloseHandle failed!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    CAN_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CANGetClockRate
//
// This function will retrieve the clock rate divisor. Note that the value 
// is not the absolute peripheral clock frequency. The value retrieved should 
// be compared against the CAN specifications to obtain the true frequency.
//
// Parameters:
//      hCAN
//          [in] The CAN device handle retrieved from CANOpenHandle().
//
//      pwClkRate
//          [out] The pointer of WORD variable that retrieves divisor index.
//          Refer to CAN specification to obtain the true clock frequency.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CANGetClockRate(HANDLE hCAN, PWORD pwClkRate)
{
    CAN_FUNCTION_ENTRY();

    if (!DeviceIoControl(hCAN,      // file handle to the driver
        CAN_IOCTL_GET_CLOCK_RATE,   // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        pwClkRate,                  // out buffer
        sizeof(WORD),               // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CAN_IOCTL_GET_CLOCK_RATE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CAN_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CANSetClockRate
//
// This function will initialize the CAN device with the given clock rate. Note
// that this function does not expect to receive the absolute peripheral clock
// frequency. Rather, it will be expecting the clock rate divisor index stated
// in the CAN specification. If absolute clock frequency must be used, please
// use the function CANSetFrequency().
//
// Parameters:
//      hCAN
//          [in] The CAN device handle retrieved from CANOpenHandle().
//
//      wClkRate
//          [in] Contains the divisor index. Refer to CAN specification to
//          obtain the true clock frequency.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CANSetClockRate(HANDLE hCAN, WORD wClkRate)
{
    CAN_FUNCTION_ENTRY();

    if (!DeviceIoControl(hCAN,      // file handle to the driver
        CAN_IOCTL_SET_CLOCK_RATE,   // I/O control code
        &wClkRate,                  // in buffer
        sizeof(WORD),               // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CAN_IOCTL_SET_CLOCK_RATE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CAN_FUNCTION_EXIT();
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CANTransfer
//
// This function performs one or more CAN read or write operations.  
// pCANTransferBlock contains a pointer to the first of an array of CAN 
// packets to be processed by the CAN.  All the required information for the 
// CAN operations should be contained in the array elements of pCANPackets.
//
// Parameters:
//      hCAN
//          [in] The CAN device handle retrieved from CANOpenHandle().
//
//      pCANTransferBlock
//          [in] The pointer of CAN_TRANSFER_BLOCK structure type.
//          The fields of CAN_TRANSFER_BLOCK are described below:
//              pCANPackets - Pointer to the CAN Packets to transfer.
//              iNumPackets - Number of packets in pCANPackets array.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CANTransfer(HANDLE hCAN, PCAN_TRANSFER_BLOCK pCANTransferBlock)
{
    CAN_FUNCTION_ENTRY();

    if (!DeviceIoControl(hCAN,      // file handle to the driver
        CAN_IOCTL_TRANSFER,         // I/O control code
        pCANTransferBlock,          // in buffer
        sizeof(CAN_TRANSFER_BLOCK), // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CAN_IOCTL_TRANSFER failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CAN_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CANReset
//
// This function performs a hardware reset. Note that the CAN driver will still
// maintain all the current information of the device, which includes all the
// initialized addresses.
//
// Parameters:
//      hCAN
//          [in] The CAN device handle retrieved from CANOpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CANReset(HANDLE hCAN)
{
    CAN_FUNCTION_ENTRY();

    if (!DeviceIoControl(hCAN,      // file handle to the driver
        CAN_IOCTL_RESET,            // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CAN_IOCTL_RESET failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CAN_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CANEnableTLPRIO
//
// This function enables a CAN slave access from the bus. Note that after 
// the CAN slave interface enabled, CAN slave driver wait for master access
// all the time.
//
// Parameters:
//      hCAN
//          [in] The CAN device handle retrieved from CANOpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CANEnableTLPRIO(HANDLE hCAN)
{
    CAN_FUNCTION_ENTRY();

    if (!DeviceIoControl(hCAN,      // file handle to the driver
        CAN_IOCTL_ENABLE_TLPRIO,     // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CAN_IOCTL_ENABLE_SLAVE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CAN_FUNCTION_EXIT();
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CANDisableTLPRIO
//
// This function disables CAN slave access from the bus. Note that after 
// the CAN slave interface disabled, CAN slave module can be turned off.
//
// Parameters:
//      hCAN
//          [in] The CAN device handle retrieved from CANOpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CANDisableTLPRIO(HANDLE hCAN)
{
    CAN_FUNCTION_ENTRY();

    if (!DeviceIoControl(hCAN,      // file handle to the driver
        CAN_IOCTL_DISABLE_TLPRIO,    // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CAN_IOCTL_DISABLE_SLAVE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CAN_FUNCTION_EXIT();
    
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: CANSetMode
//
// This function set can index as transmitte or receiver mode 
//
// Parameters:
//      hCAN
//          [in] The CAN device handle retrieved from CANOpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL  CANSetMode(HANDLE hCAN,DWORD index,CAN_MODE mode)
{
    DWORD ChangedMode;
    CAN_FUNCTION_ENTRY();
    ChangedMode=(index<<8)|mode;
    if (!DeviceIoControl(hCAN,      // file handle to the driver
        CAN_IOCTL_SET_CAN_MODE,    // I/O control code
        &ChangedMode,                       // in buffer
        sizeof(DWORD),                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CAN_IOCTL_DISABLE_SLAVE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CAN_FUNCTION_EXIT();

return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the CAN DDK module.  This
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
//      TRUE if the CAN is initialized; FALSE if an error occurred during
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
                     (_T("***** DLL PROCESS ATTACH TO CAN SDK *****\r\n")));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS DETACH FROM CAN SDK *****\r\n")));
            break;
    }

    // return TRUE for success
    return TRUE;
}

