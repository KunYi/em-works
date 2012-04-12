//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
///
/// This software as well as any related documentation may only be used or
/// copied in accordance with the terms of the Wolfson Microelectronic plc
/// agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
///
/// The information in this file is furnished for informational use only,
/// is subject to change without notice, and should not be construed as a
/// commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
/// assumes no responsibility or liability for any errors or inaccuracies that
/// may appear in the software or any related documention.
///
/// Except as permitted by the agreement(s), no part of the software or any
/// related documention may be reproduced, stored in a retrieval system, or
/// transmitted in any form or by any means without the express written
/// consent of Wolfson Microelectronics plc.
///
/// @file   cspiutil.h
/// @brief  Defines the interface to the CSPI driver for the WM8350.
///
/// This file contains the functions for communicating with the WM8350 over
/// the 3-wire and 4-wire (SPI) interfaces.  This code is specifically for
/// the PMIC interface and should be run over a dedicated SPI bus.
///
/// This code is designed to handle both normal (interrupt-driven) operations
/// and power-up/power-down operation (where interrupts are disabled), and to
/// switch between the two.  It specifically signals and aborts transactions
/// which are still waiting when entering power state.
///
/// @version $Id: cspiutil.c 380 2007-04-23 09:52:42Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
////////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4115)
#pragma warning(disable: 4214)
#include <windows.h>
#pragma warning(pop)
#include <Devload.h>
#pragma warning(push)
#pragma warning(disable: 4214)
#include <ceddk.h>
#pragma warning(pop)
#include "common_macros.h"
#include "cspiutil.h"

//-----------------------------------------------------------------------------
// External Functions
extern void   PmicConfigPolling( BOOL bPoll );
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
extern BOOL g_bPmicUsePolling;      // Main configuration variable, set in pmicpdk.c
extern volatile BOOL g_bUsePolling; // Current state variable

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

#define DEBUG_PMIC_LOCKING      TRUE    // We shouldn't be seeing these!
#if DEBUG_PMIC_LOCKING
#   define LOCK_ENTRY()         DEBUGMSG( ZONE_INFO, ( _T("+++ PMIC CSPI Lock taken +++\r\n"), _T(__FUNCTION__) ) )
#   define LOCK_EXIT()          DEBUGMSG( ZONE_INFO, ( _T("--- PMIC CSPi Lock released ---\r\n"), _T(__FUNCTION__) ) )
#else
#   define LOCK_ENTRY()
#   define LOCK_EXIT()
#endif

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables
static HANDLE g_hCspiIntrEvent;
static HANDLE g_hCspiWaitEvent;
static HANDLE g_hCspiIntrServThread;
static DWORD g_cspiSysIntr;
static volatile BOOL g_bSync;
static HANDLE g_hCspiMutex;


//-----------------------------------------------------------------------------
// Local Functions
static DWORD WaitForInterruptEvent(HANDLE hIntrEvent, DWORD dwMilliseconds);
static DWORD WINAPI CspiIntrServThread (LPVOID lpParam);

//-----------------------------------------------------------------------------
//
// Function: PmicCspiInitialize
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
BOOL PmicCspiInitialize(int index, UINT32 dwFrequency)
{
    UNREFERENCED_PARAMETER( dwFrequency );

    if (!BSPPmicCSPIGetSysIntr(index, &g_cspiSysIntr))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("cspiInitialize: Failed to obtain SYSINTR value.\r\n")));
        goto Error;
    }

    if (!BSPPmicCSPIMapIoSpace(index))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("cspiInitialize: MmMapIoSpace failed for CSP_CSPI_REG.\r\n")));
        goto Error;
    }

    // Create event for CSPI interrupt signaling
    g_hCspiIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (g_hCspiIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("cspiInitialize: CreateEvent failed for g_hCspiIntrEvent.\r\n")));
        goto Error;
    }

    // Register CSPI interrupt
    if (!InterruptInitialize(g_cspiSysIntr, g_hCspiIntrEvent, NULL, 0))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("cspiInitialize: InterruptInitialize failed.\r\n")));
        goto Error;
    }

    // Create event for CSPI thread wait signaling
    g_hCspiWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (g_hCspiWaitEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("cspiInitialize: CreateEvent failed for g_hCspiWaitEvent.\r\n")));
        goto Error;
    }

    g_hCspiMutex = CreateMutex(NULL, FALSE, L"MUTEX_PMIC");

    // Create IST for CSPI interrupts
    g_hCspiIntrServThread = CreateThread(NULL, 0, CspiIntrServThread, NULL, 0, NULL);
    if (!g_hCspiIntrServThread)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("cspiInitialize: CreateThread failed for CSPI IST\r\n")));
        goto Error;
    }

    // Initialize global for sync with threads blocked on CSPI transfer
    g_bSync = FALSE;

    // Configure for interrupt-driven communication
    PmicConfigPolling(g_bPmicUsePolling);

    return TRUE;

Error:
    return FALSE;
}


//-----------------------------------------------------------------------------
//
// Function: PmicCspiRelease
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
VOID PmicCspiRelease(int index)
{
    UNREFERENCED_PARAMETER( index );

    return;
}

//-----------------------------------------------------------------------------
//
// Function: PmicCspiAddWritePacket
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
BOOL PmicCspiAddWritePacket(UINT32 addr, UINT32 data)
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
// Function: PmicCspiAddReadPacket
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
BOOL PmicCspiAddReadPacket(UINT32 addr)
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
// Function: PmicCspiDataExchange
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
BOOL PmicCspiDataExchange()
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
// Function: PmicCspiReceiveData
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
BOOL PmicCspiReceiveData(UINT32* data)
{
#ifdef VPMX31
    *data = BSPPmicCSPIReadRXFIFO();
    *data &= CSPI_DATA_MASK;
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
            DEBUGMSG(ZONE_ERROR, (_T("cspiReceiveData failed\r\n")));
            *data = 0;
            goto cleanUp;
        }
    }


    *data = BSPPmicCSPIReadRXFIFO();
    *data &= CSPI_DATA_MASK;
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
// Function: PmicCspiDiscardData
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
BOOL PmicCspiDiscardData()
{
    UINT data;

    return PmicCspiReceiveData(&data);
}


//-----------------------------------------------------------------------------
//
// Function: PmicCspiPowerDown
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
VOID PmicCspiPowerDown(void)
{
}


//-----------------------------------------------------------------------------
//
// Function: PmicCspiSync
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
VOID PmicCspiSync(void)
{
    // Wait for pending interrupt-driven transfers to complete.  The g_bSync
    // global is set prior to blocking on the CSPI transfer and will be cleared
    // by CspiIntrServThread or completion of PmicCspiReceiveData
    while (g_bSync);

}


//-----------------------------------------------------------------------------
//
//  Function: PmicCspiLock
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
void PmicCspiLock(void)
{
    WaitForSingleObject(g_hCspiMutex, INFINITE);

    LOCK_ENTRY();

    BSPPmicSetSpiClockGating(TRUE);
    BSPPmicCSPIEnable();

    // Configure the CSPI for polled/interrupt-driven mode
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
//  Function: PmicCspiUnlock
//
//  Release the lock previously obtained by PmicCspiLock.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PmicCspiUnlock(void)
{
    BSPPmicCSPIDisable();
    BSPPmicSetSpiClockGating(FALSE);
    ReleaseMutex(g_hCspiMutex);
    LOCK_EXIT();
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

    UNREFERENCED_PARAMETER( lpParam );

    // Bump up our priority to the highest level available to CE drivers.
    // This will make sure we have a chance to signal threads blocked on a
    // CSPI transfer before starting the suspend procedure.
    CeSetThreadPriority(GetCurrentThread(), 97);

    for(;;)
    {
        if(WaitForSingleObject(g_hCspiIntrEvent, INFINITE) == WAIT_OBJECT_0)
        {
            // Disable CSPI interrupts, so we can call InterruptDone.  The
            // thread signaled with g_hCspiWaitEvent will perform the acutal
            // CSPI servicing
            BSPPmicCSPIIRQDisable();
            g_bSync = FALSE;
            InterruptDone(g_cspiSysIntr);
            SetEvent(g_hCspiWaitEvent);
        }
        else
        {
            // Abnormal signal
            rc = FALSE;
            break;
        }
    }

    return rc;
}

//////////////////////////////// END OF FILE ///////////////////////////////////
