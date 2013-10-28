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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
// Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  cspiutil.c
//
//  This file contains the utility function for accessing perpherials via CSPI.
//  The code is currently used for accessing PMIC.  We assume 32 bit access.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

#include "cspiutil.h"
#include "socarm_macros.h"
#include "ioctl_pmic.h"

//-----------------------------------------------------------------------------
// External Functions
extern BOOL   BSPPmicCSPIGetSysIntr(int index, DWORD *cspiSysIntr);
extern BOOL   BSPPmicCSPIMapIoSpace(int index);
extern void   BSPPmicCSPIWriteTXFIFO(unsigned int data);
extern UINT32 BSPPmicCSPIReadRXFIFO(void);
extern void   BSPPmicCSPIExchange(void);
extern void   BSPPmicCPSIWaitTransactionComplete(void);
extern void   BSPPmicCSPIWaitReadReady(void);
extern void   BSPPmicCSPIEnable(void);
extern void   BSPPmicCSPIDisable(void);
extern void   BSPPmicCSPIRXIRQEnable(void);
extern void   BSPPmicCSPIIRQDisable(void);
extern UINT32 BSPPmicGetSpiFreqIn(void);
extern BOOL   BSPPmicSetSpiClockGating(BOOL bEnable);

//-----------------------------------------------------------------------------
// External Variables
extern BOOL g_bPmicUseCspiPolling;

//-----------------------------------------------------------------------------
// Defines
#define CSPI_READ               0
#define CSPI_WRITE              1

#define CSPI_TIMEOUT            1000

//-----------------------------------------------------------------------------
// Types

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNC          (1 << ZONEID_FUNC)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC              DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)

#endif

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables
static HANDLE g_hCspiIntrEvent;
static HANDLE g_hCspiWaitEvent;
static HANDLE g_hCspiIntrServThread;
static DWORD g_cspiSysIntr;
static BOOL g_bUsePolling;
static volatile BOOL g_bSync;


//-----------------------------------------------------------------------------
// Local Functions
static DWORD WaitForInterruptEvent(HANDLE hIntrEvent, DWORD dwMilliseconds);
static DWORD WINAPI CspiIntrServThread (LPVOID lpParam);

//-----------------------------------------------------------------------------
//
// Function: cspiInitialize
//
// Initializes the CSPI interface and data structures.
//
// Parameters:
//      index
//          [in] CSPI instance (1 = CSPI1, 2 = CSPI2) to initialize.
//
//      dwFrequency
//          [in] Frequency requested.
//
// Returns:
//      TRUE is CSPI is initialized successfully.
//
//-----------------------------------------------------------------------------
BOOL cspiInitialize(int index, UINT32 dwFrequency)
{
    UNREFERENCED_PARAMETER(dwFrequency);

    if (!BSPPmicCSPIGetSysIntr(index, &g_cspiSysIntr))
    {
        ERRORMSG(TRUE, (TEXT("%s(): Failed to obtain SYSINTR value.\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

    if (!BSPPmicCSPIMapIoSpace(index))
    {
        ERRORMSG(TRUE, (TEXT("%s(): MmMapIoSpace() failed for ")
                        TEXT("CSP_CSPI_REG.\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Create event for CSPI interrupt signaling
    g_hCspiIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (g_hCspiIntrEvent == NULL)
    {
        ERRORMSG(TRUE, (TEXT("%s(): CreateEvent failed for ")
                        TEXT("g_hCspiIntrEvent.\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Register CSPI interrupt
    if (!InterruptInitialize(g_cspiSysIntr, g_hCspiIntrEvent, NULL, 0))
    {
        ERRORMSG(TRUE, (TEXT("%s(): InterruptInitialize() failed.\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

    // Create event for CSPI thread wait signaling
    g_hCspiWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (g_hCspiWaitEvent == NULL)
    {
        ERRORMSG(TRUE, (TEXT("%s(): CreateEvent() failed for ")
                        TEXT("g_hCspiWaitEvent.\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Create IST for CSPI interrupts
    g_hCspiIntrServThread = CreateThread(NULL, 0, CspiIntrServThread, NULL,
                                         0, NULL);
    if (!g_hCspiIntrServThread) 
    {
        ERRORMSG(TRUE, (TEXT("%s(): CreateThread() failed for CSPI IST\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

    // Initialize global for sync with threads blocked on CSPI transfer
    g_bSync = FALSE;

    // Configure for interrupt-driven communication
    cspiConfigPolling(g_bPmicUseCspiPolling);

    return TRUE;

Error:
    return FALSE;
}


//-----------------------------------------------------------------------------
//
// Function: cspiRelease
//
// Frees allocated memory, closes reference to handles, and resets the state
// of global member variables.
//
// Parameters:
//      index
//          [in] CSPI instance (1 = CSPI1, 2 = CSPI2) to initialize
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID cspiRelease(int index)
{
    UNREFERENCED_PARAMETER(index);

    return;
}

//-----------------------------------------------------------------------------
//
// Function: cspiConfigPolling
//
// Configures polled or interrupt-driven communication for the CSPI interface.
//
// Parameters:
//      bPoll
//          [in] Set to TRUE to forced polled communication.  Set to FALSE for
//          interrupt-driven communication.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID cspiConfigPolling(BOOL bPoll)
{
    g_bUsePolling = bPoll;
}


//-----------------------------------------------------------------------------
//
// Function: cspiAddWritePacket
//
// Add a CSPI write packet.
//
// Parameters:
//      addr
//          [in] address
//      data
//          [in] data to write
//
// Returns:
//      TRUE if CSPI writing is successful.
//
//-----------------------------------------------------------------------------
BOOL cspiAddWritePacket(UINT32 addr, UINT32 data)
{
    CSPI_PACKET32 packet;

    // Format the packet
    packet.reg.data = data;
    packet.reg.null = 0;
    packet.reg.address = addr;
    packet.reg.rw = CSPI_WRITE;

    // Write the packet to the TXFIFO
    BSPPmicCSPIWriteTXFIFO(packet.data);
    DEBUGMSG(ZONE_INFO, (_T("write 0x%x = 0x%x\r\n"), addr, data));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: cspiAddReadPacket
//
// Add a CSPI read packet.
//
// Parameters:
//      addr
//          [in] address
//
// Returns:
//      TRUE if CSPI reading is successful.
//
//-----------------------------------------------------------------------------
BOOL cspiAddReadPacket(UINT32 addr)
{
    CSPI_PACKET32 packet;

    // Format the packet
    packet.reg.data = 0;
    packet.reg.null = 0;
    packet.reg.address = addr;
    packet.reg.rw = CSPI_READ;

    // Write the packet to the TXFIFO
    BSPPmicCSPIWriteTXFIFO(packet.data);

    DEBUGMSG(ZONE_INFO, (_T("read addr = 0x%x\r\n"), addr));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: cspiDataExchange
//
// Initiate CSPI data exchange.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if data exchange is successful.
//
//-----------------------------------------------------------------------------
BOOL cspiDataExchange()
{
    BSPPmicCSPIExchange();

#ifdef VPMX31
    // wait until transaction is complete
    BSPPmicCSPIWaitTransactionComplete();
#endif

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: cspiReceiveData
//
// Read a packet from CSPI RXDATA.
//
// Parameters:
//      data
//          [out] data read from CSPI RXDATA
//
// Returns:
//      TRUE if CSPI reading is successful.
//
//-----------------------------------------------------------------------------
BOOL cspiReceiveData(UINT32* data)
{
#ifdef VPMX31
    *data = BSPPmicCSPIReadRXFIFO();
    *data &= 0xFFFFFF;
    DEBUGMSG(ZONE_INFO, (_T("Receive data = 0x%x\r\n"), *data));

    return TRUE;
#else
    BOOL rc = FALSE;

    // If polled communication is requested
    if (g_bUsePolling)
    {
        // Wait until transaction is complete
        BSPPmicCSPIWaitReadReady();
    }

    // Else use interrupt-driven communication
    else
    {
        // Set a global to indicate we are about to block on the CSPI
        // transfer.  This will be used during a suspend procedure
        // to see if we need to wait until the transfer is complete
        g_bSync = TRUE;

        // NOTE: Can't use WaitForSingleObject directly on an event
        // registered with InterruptInitialize as the kernel faults
        // on priority inversion!
        if (WaitForSingleObject(g_hCspiWaitEvent, CSPI_TIMEOUT) != WAIT_OBJECT_0)
        {
            ERRORMSG(TRUE, (_T("cspiReceiveData failed\r\n")));
            *data = 0;
            goto cleanUp;
        }
    }

    *data = BSPPmicCSPIReadRXFIFO();
    *data &= 0xFFFFFF;
    DEBUGMSG(ZONE_INFO, (_T("Receive data = 0x%x\r\n"), *data));

    rc = TRUE;

cleanUp:
    g_bSync = FALSE;
    InterruptDone(g_cspiSysIntr);
    return rc;
#endif
}


//-----------------------------------------------------------------------------
//
// Function: cspiDiscardData
//
// Discard all data in CSPI RXDATA.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE.
//
//-----------------------------------------------------------------------------
BOOL cspiDiscardData()
{
    UINT data;

    return cspiReceiveData(&data);
}


//-----------------------------------------------------------------------------
//
// Function: cspiPowerDown
//
// Power down the CSPI interface.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE.
//
//-----------------------------------------------------------------------------
VOID cspiPowerDown(void)
{
}


//-----------------------------------------------------------------------------
//
// Function: cspiSync
//
// Waits for pending interrupt-driven SPI transfers to complete so that the
// system can properly suspend.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE.
//
//-----------------------------------------------------------------------------
VOID cspiSync(void)
{
    // Wait for pending interrupt-driven transfers to complete.  The g_bSync
    // global is set prior to blocking on the CSPI transfer and will be cleared
    // by CspiIntrServThread or completion of cspiReceiveData
    while (g_bSync)
        ; // Polling loop.
}


//-----------------------------------------------------------------------------
//
//  Function: cspiLock
//
//  Obtains a lock for the CSPI hardware so that the OAL and PMIC core driver
//  can safely share access.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void cspiLock(void)
{
    // For WinCE 6.00, use an OEM IOCTL call to signal the OAL to grant
    // exclusive access to the CSPI bus. We must use this approach because
    // WinCE 6.00 does not support the use of named mutexes.
    if (!KernelIoControl(IOCTL_PMIC_CSPI_LOCK, NULL, 0, NULL, 0, NULL))
    {
        ERRORMSG(TRUE, (TEXT("cspiLock(): KernelIoControl() returned FALSE ")
                        TEXT("(expected TRUE always)\r\n")));
    }

    BSPPmicSetSpiClockGating(TRUE);
    BSPPmicCSPIEnable();

    // Configure the CSPI for polled/interrupt-driven mode.
    if (g_bUsePolling)
    {
        // Disable all CSPI interrupts
        BSPPmicCSPIIRQDisable();
    }
    else
    {
        // Enable RX FIFO ready interrupt
        BSPPmicCSPIRXIRQEnable();
    }
}


//-----------------------------------------------------------------------------
//
//  Function: cspiUnlock
//
//  Release the lock previously obtained by cspiLock.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void cspiUnlock(void)
{
    BSPPmicCSPIDisable();
    BSPPmicSetSpiClockGating(FALSE);

    // Use an OEM IOCTL call to signal the OAL to release exclusive
    // access to the CSPI bus. This should only be called by the thread
    // that had previously called cspiLock(). Then we can guarantee that
    // the KernelIoControl() call will never block and always be successful.
    if (!KernelIoControl(IOCTL_PMIC_CSPI_UNLOCK, NULL, 0, NULL, 0, NULL))
    {
        ERRORMSG(TRUE, (TEXT("cspiUnlock(): KernelIoControl() returned FALSE ")
                        TEXT("(expected TRUE always)\r\n")));
    }
}


//-----------------------------------------------------------------------------
//
//  Function:  CspiIntrServThread
//
//  This is the interrupt service thread for CSPI interrupts.  
//
//  Parameters:
//      lpParam
//          [in] Thread data passed to the function using the 
//          lpParameter parameter of the CreateThread function. Not used.
//
//  Returns:
//      Returns thread exit code.
//
//-----------------------------------------------------------------------------
static DWORD WINAPI CspiIntrServThread (LPVOID lpParam)
{
    DWORD rc = TRUE;

    UNREFERENCED_PARAMETER(lpParam);

    // Bump up our priority to the highest level available to CE drivers.
    // This will make sure we have a chance to signal threads blocked on a
    // CSPI transfer before starting the suspend procedure.
    CeSetThreadPriority(GetCurrentThread(), 97);

    for(;;)
    {
        if(WaitForSingleObject(g_hCspiIntrEvent, INFINITE) == WAIT_OBJECT_0)
        {
            // Disable CSPI interrupts, so we can call InterruptDone. The
            // thread signaled with g_hCspiWaitEvent will perform the actual
            // CSPI servicing.
            BSPPmicCSPIIRQDisable();
            g_bSync = FALSE;
            InterruptDone(g_cspiSysIntr);
            SetEvent(g_hCspiWaitEvent);
        }
        else
        {
            // Abnormal interrupt signal.
            ERRORMSG(TRUE, (TEXT("CSPI interrupt handler exiting ")
                            TEXT("due to error!\r\n")));
            rc = FALSE;
            break;
        }
    }

    return rc;
}
