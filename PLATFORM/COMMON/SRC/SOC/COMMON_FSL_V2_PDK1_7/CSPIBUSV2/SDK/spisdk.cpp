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
//  File:  cspisdk.cpp
//
//  The implementation of CSPI driver SDK.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include "ceddk.h"
#pragma warning(pop)

#include "common_macros.h"
#include "cspibus.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
#define CSPI_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define CSPI_FUNCTION_EXIT() \
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
    _T("CSPI"),
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
// Function: CSPIOpenHandle
//
// This method creates a handle to the CSPI stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to CSPI driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE CSPIOpenHandle(LPCWSTR lpDevName)
{
    HANDLE hCSPI;

    CSPI_FUNCTION_ENTRY();
    
    hCSPI = CreateFile(lpDevName,            // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to CSPI
    if (hCSPI == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s:  CreateFile CSPI failed!\r\n"), __WFUNCTION__));
        return hCSPI;
    }

    CSPI_FUNCTION_EXIT();

    return hCSPI;
}

//------------------------------------------------------------------------------
//
// Function: CSPICloseHandle
//
// This method closes a handle to the CSPI stream driver.
//
// Parameters:
//      hCSPI
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL CSPICloseHandle(HANDLE hCSPI)
{
    CSPI_FUNCTION_ENTRY();

    // if we don't have handle to CSPI bus driver
    if (hCSPI != NULL)
    {
        if (!CloseHandle(hCSPI))
        {
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: CloseHandle failed!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    CSPI_FUNCTION_EXIT();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CSPIExchange
//
// This function performs CSPI exchange operations.  
//
// Parameters:
//      hCSPI
//          [in] The CSPI device handle retrieved from CSPIOpenHandle().
//
//      pCspiXchPkt
//          [in] The pointer of CSPI_XCH_PKT_T structure type.
//          The fields of CSPI_XCH_PKT_T are described below:
//              pBusCnfg - Pointer to the CSPI configure.
//              pTxBuf - Pointer to the CSPI Tx data buffer in the Packet.
//              pRxBuf - Pointer to the CSPI Rx data buffer in the Packet, if 
//                       set to NULL, ignore received data.
//              xchCnt - Total of exchanges in the packet.
//              xchEvent - event to launch when XCH done, if set to NULL, no
//                         event launch but synchronously exchange the packet.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CSPIExchange(HANDLE hCSPI, PCSPI_XCH_PKT_T pCspiXchPkt)
{
    CSPI_FUNCTION_ENTRY();

    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_EXCHANGE,        // I/O control code
        pCspiXchPkt,                // in buffer
        sizeof(CSPI_XCH_PKT_T),     // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CSPI_IOCTL_EXCHANGE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CSPI_FUNCTION_EXIT();
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CSPIEnableLoopback
//
// This function enable loopback path for CSPI exchange operations.  
//
// Parameters:
//      hCSPI
//          [in] The CSPI device handle retrieved from CSPIOpenHandle().
//
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CSPIEnableLoopback(HANDLE hCSPI)
{
    CSPI_FUNCTION_ENTRY();

    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_ENABLE_LOOPBACK, // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CSPI_IOCTL_ENABLE_LOOPBACK failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CSPI_FUNCTION_EXIT();
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CSPIDisableLoopback
//
// This function Disable loopback path for CSPI exchange operations.  
//
// Parameters:
//      hCSPI
//          [in] The CSPI device handle retrieved from CSPIOpenHandle().
//
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CSPIDisableLoopback(HANDLE hCSPI)
{
    CSPI_FUNCTION_ENTRY();

    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_DISABLE_LOOPBACK, // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CSPI_IOCTL_DISABLE_LOOPBACK failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    CSPI_FUNCTION_EXIT();
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the CSPI DDK module.  This
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
//      TRUE if the CSPI is initialized; FALSE if an error occurred during
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
                     (_T("***** DLL PROCESS ATTACH TO CSPI SDK *****\r\n")));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS DETACH FROM CSPI SDK *****\r\n")));
            break;
    }

    // return TRUE for success
    return TRUE;
}

