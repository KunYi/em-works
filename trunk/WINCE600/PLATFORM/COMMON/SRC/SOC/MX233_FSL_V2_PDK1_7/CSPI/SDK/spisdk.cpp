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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

#include "hw_spi.h"



//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------

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
//
// Debug zone args to DEBUGMSG
#define ZONE_INIT         DEBUGZONE(ZONEID_INIT)
#define ZONE_INFO         DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION     DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN         DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR        DEBUGZONE(ZONEID_ERROR)

DBGPARAM dpCurSettings = {
    TEXT("cspi"), {
        TEXT("Init"),TEXT("Deinit"),TEXT("Open"),TEXT("Close"),
        TEXT("IOCtl"),TEXT("Thread"),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT("Function"),TEXT("Warning"),TEXT("Error") },
    0xC000
};

#endif


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
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
    RETAILMSG(TRUE, (TEXT("CSPIOpenHandle\r\n")));
    RETAILMSG(TRUE, (TEXT("ETH: CSPIOpenHandle %s\r\n"),lpDevName));

    hCSPI = CreateFile(lpDevName,            // name of device
                        GENERIC_READ|GENERIC_WRITE, // desired access
                        FILE_SHARE_READ|FILE_SHARE_WRITE, // sharing mode
                        NULL,               // security attributes (ignored)
                        OPEN_EXISTING,      // creation disposition
                        FILE_FLAG_RANDOM_ACCESS, // flags/attributes
                        NULL);              // template file (ignored)

    RETAILMSG(TRUE, (TEXT("ETH: hCSPI %d\n"),GetLastError()));

    // if we failed to get handle to CSPI
    if (hCSPI == INVALID_HANDLE_VALUE)
    {RETAILMSG(TRUE, (TEXT("hCSPI == INVALID_HANDLE_VALUE\r\n")));
        return FALSE;
    }
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
    // if we don't have handle to CSPI bus driver
    if (hCSPI != NULL)
    {
        if (!CloseHandle(hCSPI))
        {
            return FALSE;
        }
    }
    return TRUE;
}

//#ifdef POLLING_MODE
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
    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_EXCHANGE,        // I/O control code
        pCspiXchPkt,                // in buffer
        sizeof(CSPI_XCH_PKT_T),     // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//#endif

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
    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_ENABLE_LOOPBACK, // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        return FALSE;
    }
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
    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_DISABLE_LOOPBACK, // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CSPIConfigure
//
// This function Configures the SPI bus required for interaction.  
//
// Parameters:
//      hCSPI
//          [in] The CSPI device handle retrieved from CSPIOpenHandle().
//      sInit
//          [in] pointer to structure SSP_INIT which specifies input parameters to confogure the SPI bus
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CSPIConfigure(HANDLE hCSPI,PSSP_INIT  pSspInit)
{
    RETAILMSG(1, (TEXT("Ether_Init:sInit Address 0x%x \r\n"),pSspInit));

    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_SSPCONFIGURE,    // I/O control code
        pSspInit,                     // in buffer
        sizeof(SSP_INIT),           // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: CSPIResetAfterError
//
// This function Reset and Reconfigure the SSP interface after an Error IRQ.  
//
// Parameters:
//      hCSPI
//          [in] The CSPI device handle retrieved from CSPIOpenHandle().
//      sConfigparams
//          [in] pointer to Data structure with field representing the SSP_CTRL0, SSP_CTRL1, SSP_TIMING registers
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CSPIResetAfterError(HANDLE hCSPI,SSP_RESETCONFIG * sConfigparams)
{
    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_SSPRESETAFTERERROR, // I/O control code
        &sConfigparams,             // in buffer
        sizeof(SSP_RESETCONFIG),    // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: CSPICheckErros
//
// This function check the SSP Control1 and Status registers for errors.  
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
BOOL CSPICheckErros(HANDLE hCSPI)
{
    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_SSPCHECKERRORS,  // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: CSPIGetIRQStatus
//
// This function returns the status of the SSP Interrupt bit.
//
// Parameters:
//      hCSPI
//          [in] The CSPI device handle retrieved from CSPIOpenHandle().
//      eIrq 
//          [in] The Enum value of the Interrupt Bit.
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CSPIGetIRQStatus(HANDLE hCSPI,SSP_IRQ eIrq)
{
    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_SSPGETIRQSTATUS, // I/O control code
        &eIrq,                      // in buffer
        sizeof(SSP_IRQ),            // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: CSPIClearIRQ
//
// This function Clears the interrupt flag of a specified SSP interrupt.  
//
// Parameters:
//      hCSPI
//          [in] The CSPI device handle retrieved from CSPIOpenHandle().
//      eIrq 
//          [in] The Enum value of the Interrupt Bit.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CSPIClearIRQ(HANDLE hCSPI,SSP_IRQ eIrq)
{

    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_SSPCLEARIRQ,     // I/O control code
        &eIrq,                      // in buffer
        sizeof(SSP_IRQ),            // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: CSPIConfigTiming
//
// This function configures the Timing register of SSP1 block as accoring to the set frequency.  
//
// Parameters:
//      hCSPI
//          [in] The CSPI device handle retrieved from CSPIOpenHandle().
//      eIrq 
//          [in] The Enum value of the Speed to be set.
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CSPIConfigTiming(HANDLE hCSPI,SSP_SPEED eSpeed)
{

    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_SSPCONFIGTIMING,     // I/O control code
        &eSpeed,                      // in buffer
        sizeof(SSP_SPEED),            // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: CSPIEnableErrorIRQ
//
// This function Enables the Error IRQ of the SSP Block.
//
// Parameters:
//      hCSPI
//          [in] The CSPI device handle retrieved from CSPIOpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CSPIEnableErrorIRQ(HANDLE hCSPI,BOOL bEnable)
{
        if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_SSP_EN_ERROR,     // I/O control code
        &bEnable,                       // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        return FALSE;
    }
    return TRUE;

}
//-----------------------------------------------------------------------------
//
// Function: CSPIConfigTiming
//
// This function Disables the Error interrupts of the SSP1 Block.  
//
// Parameters:
//      hCSPI
//          [in] The CSPI device handle retrieved from CSPIOpenHandle().
//
// Returns:  
//      Return TRUE or FALSE. If the result is TRUE, the operation is
//      successful.
//
//-----------------------------------------------------------------------------
BOOL CSPIDisableErrorIRQ(HANDLE hCSPI)
{
    if (!DeviceIoControl(hCSPI,     // file handle to the driver
        CSPI_IOCTL_SSP_DIS_ERROR,     // I/O control code
        NULL,                          // in buffer
        0,                          // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        return FALSE;
    }
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
