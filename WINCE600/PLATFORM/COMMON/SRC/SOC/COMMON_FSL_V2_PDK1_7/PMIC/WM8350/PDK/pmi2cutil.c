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
/// @file   PmicI2Cutil.c
/// @brief  Defines the interface to the I2C driver for the WM8350.
///
/// This file contains the interface for communicating with the WM8350 over
/// the 2-wire (I2C) interface.  This code is specifically for the PMIC
/// interface and should be run over a dedicated I2C bus.
///
/// This code is designed to handle both normal (interrupt-driven) operations
/// and power-up/power-down operation (where interrupts are disabled), and to
/// switch between the two.  It specifically signals and aborts transactions
/// which are still waiting when entering power state.
///
/// @version $Id: pmi2cutil.c 671 2007-06-25 21:59:38Z ib $
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
#include "PmI2Cutil.h"
#include "i2cbus.h"

//-----------------------------------------------------------------------------
// External Functions
extern void     PmicConfigPolling( BOOL bPoll );
extern UINT32   I2CGetBaseRegAddr( UINT32 index );
extern DWORD    I2CGetIRQ( UINT32 index );
extern UINT32   BSPPmicGetI2CFreqOut();
extern UINT32   BSPPmicGetI2CFreqIn();
extern BOOL     BSPPmicSetI2CClockGating( BOOL bEnable );
extern int      BSPPmicI2CGetTimeoutValue();
extern BOOL     BSPPmicInitI2C( int port );
extern BOOL     BSPPmicI2CMapIoSpace( int index );
extern BOOL     BSPPmicI2CGetSysIntr( int index, DWORD *pI2CSysIntr );
extern WORD     BSPPmicI2CCalculateClkRateDiv( DWORD dwFrequency );
extern VOID     BSPPmicI2CReset();
extern void     BSPPmicI2CSetTransmitMode( BOOL transmit );
extern void     BSPPmicI2CSetDeviceAddress( BYTE devAddr, BOOL transmit );
extern void     BSPPmicI2CSetClockDivider( WORD wClockDiv );
extern void     BSPPmicI2CRepeatedStart();
extern void     BSPPmicI2CTransmitByte( BYTE data );
extern void     BSPPmicI2CReceiveByte( BYTE *pData );
extern void     BSPPmicI2CWaitWhileBusy();
extern void     BSPPmicI2CWaitTransferDone();
extern void     BSPPmicI2CClaimMaster();
extern void     BSPPmicI2CGenerateAcks( BOOL ack );
extern BOOL     BSPPmicI2CLostArbitration();
extern BOOL     BSPPmicI2CAckFailed();
extern void     BSPPmicI2CAbort();
extern void     BSPPmicI2CStop();
extern void     BSPPmicI2CFinish();
extern void     BSPPmicI2CEnable(void);
extern void     BSPPmicI2CDisable(void);
extern void     BSPPmicI2CIRQEnable();
extern void     BSPPmicI2CIRQDisable();
extern void     BSPPmicI2CClearInterrupt();
extern void     BSPPmicI2CWaitInterrupt();
#ifdef DEBUG
extern void     BSPPmicI2CDumpRegs();
#endif /* DEBUG */

//-----------------------------------------------------------------------------
// External Variables
extern BOOL g_bPmicUsePolling;      // Main configuration variable, set in pmicpdk.c
extern volatile BOOL g_bUsePolling; // Current state variable

//-----------------------------------------------------------------------------
// Defines
#define I2C_READ               0
#define I2C_WRITE              1

#define I2C_TIMEOUT            1000

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

#define DEBUG_I2C              FALSE
#define TRACE_I2C_READS        FALSE
#define TRACE_I2C_WRITES       FALSE

extern void BSPPmicI2CDumpRegs();

#else
#   define DEBUG_I2C           FALSE
#   define BSPPmicI2CDumpRegs()
#endif


//
// Enable or disable debugging for the I2C.
//
#if DEBUG_I2C
#   define I2C_TRACE( _params ) DEBUGMSG( ZONE_INFO, _params )
#   define I2C_DUMP_REGS()      BSPPmicI2CDumpRegs()
#else
#   define I2C_TRACE( _params )
#   define I2C_DUMP_REGS()
#endif
#define I2C_TRACE_ENTRY()       I2C_TRACE(( _T("%s: + \r\n"), __WFUNCTION__ ))
#define I2C_TRACE_EXIT()        I2C_TRACE(( _T("%s: - \r\n"), __WFUNCTION__ ))
#if TRACE_I2C_WRITES
#   define TRACE_WRITE( _params ) DEBUGMSG( ZONE_INFO, _params )
#else
#   define TRACE_WRITE          I2C_TRACE
#endif
#if TRACE_I2C_READS
#   define TRACE_READ( _params ) DEBUGMSG( ZONE_INFO, _params )
#else
#   define TRACE_READ           I2C_TRACE
#endif

#define DEBUG_PMIC_LOCKING      FALSE
#if DEBUG_PMIC_LOCKING
#   define LOCK_ENTRY()         DEBUGMSG( ZONE_INFO, ( _T("+++ PMIC I2C Lock taken +++\r\n"), _T(__FUNCTION__) ) )
#   define LOCK_EXIT()          DEBUGMSG( ZONE_INFO, ( _T("--- PMIC I2C Lock released ---\r\n"), _T(__FUNCTION__) ) )
#else
#   define LOCK_ENTRY()
#   define LOCK_EXIT()
#endif

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables
static int              g_nPmicI2CIndex;
static HANDLE           g_hPmicI2CIntrEvent;        // Hardware interrupt event
static HANDLE           g_hPmicI2CWaitEvent;        // Software interrupt event
static HANDLE           g_hPmicI2CIntrServThread;
static DWORD            g_PmicI2CSysIntr;
static volatile BOOL    g_bSync;
static HANDLE           g_hPmicI2CMutex;
static INT              g_IntrWaitTimeout;          // Time to wait for interrupt before timing out
static WORD             g_wClockRateDiv;


//-----------------------------------------------------------------------------
// Local Functions
static DWORD WINAPI PmicI2CIntrServThread( LPVOID lpParam );
static BOOL private_PrepareI2C( BYTE devAddr );
static void private_ShutdownI2C();
static int private_GenerateStart( BYTE devAddr, BOOL bWrite );
static int private_RepeatedStart( BYTE devAddr, BOOL bWrite );
static int private_WriteData( BYTE *pData, UINT nBufLen, BOOL bLast );
static int private_ReadData( BYTE *pData, UINT nBufLen, BOOL bLast );
static int private_WaitForCompletion( BOOL checkArbitration, BOOL checkAck );
static int private_WaitForInterrupt();


//-----------------------------------------------------------------------------
//
// Function: PmicI2CInitialize
//
// Initializes the I2C interface and data structures.
//
// Parameters:
//      index
//          [in] I2C instance (1 = I2C1, 2 = I2C2) to initialize.
//
//      dwFrequency
//          [in] Frequency requested.
//
// Returns:
//      TRUE is I2C is initialized successfully.
//
//-----------------------------------------------------------------------------
BOOL PmicI2CInitialize( int index, UINT32 dwFrequency )
{
    DEBUGMSG( ZONE_INIT, (_T("%s +\r\n"), __WFUNCTION__ ));

    g_nPmicI2CIndex = index;

    //
    // Map in our registers
    //
    if (!BSPPmicI2CMapIoSpace( g_nPmicI2CIndex ))
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: couldn't map I2C registers\r\n"),
                  __WFUNCTION__
                ));
        goto error;
    }

    //
    // Create I2C interrupt event
    //
    {
        g_hPmicI2CIntrEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        // Able to create or obtain the event?
        if ( g_hPmicI2CIntrEvent == NULL)
        {
            DEBUGMSG( ZONE_INIT | ZONE_ERROR, (
                      _T("%s: couldn't create I2C interrupt: %d\r\n" ),
                      __WFUNCTION__,
                       GetLastError()
                    ));
            goto error;
        }
    }
    DEBUGMSG( ZONE_INIT, (
                _T( "%s: g_hPmicI2CIntrEvent=0x%X\r\n"),
                __WFUNCTION__,
                g_hPmicI2CIntrEvent
            ));

    //
    // Create for I2C thread wait signalling.
    //
    {
        g_hPmicI2CWaitEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        if ( !g_hPmicI2CWaitEvent )
        {
            DEBUGMSG( ZONE_INIT | ZONE_ERROR, (
                      _T("%s: couldn't create signal event: %d\r\n"),
                      __WFUNCTION__,
                      GetLastError()
                    ));
            goto error;
        }
    }
    DEBUGMSG( ZONE_INIT, (
              _T("%s: m_hI2CWaitEvent=0x%X\r\n"),
              __WFUNCTION__,
               g_hPmicI2CWaitEvent
            ));

    //
    // I2C bus mutex - this allows us to synchronise with the OAL.
    //
    {
        g_hPmicI2CMutex = CreateMutex( NULL, FALSE, L"MUTEX_PMIC" );
        if ( !g_hPmicI2CMutex )
        {
            DEBUGMSG( ZONE_INIT | ZONE_ERROR, (
                      _T("%s: couldn't create mutex: %d\r\n"),
                      __WFUNCTION__,
                      GetLastError()
                    ));
            goto error;
        }
    }

    //
    // Configure IOMUX for I2C pins
    //
    if ( !BSPPmicInitI2C( g_nPmicI2CIndex ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: Error configuring IOMUX for I2C.\r\n"),
                  __WFUNCTION__
                ));
        goto error;
    }

    //
    // Get our System Interrupt ID
    //
    if ( !BSPPmicI2CGetSysIntr( g_nPmicI2CIndex, &g_PmicI2CSysIntr ) )
    {
        DEBUGMSG( ZONE_INIT | ZONE_ERROR, (
                  _T("%s: IRQ -> SysIntr Failed! ErrCode=%d\r\n"),
                  __WFUNCTION__,
                  GetLastError()
                ));
        goto error;
    }
    DEBUGMSG( ZONE_INIT, (
              _T("%s: g_PmicI2CSysIntr=0x%X\r\n"),
              __WFUNCTION__,
              g_PmicI2CSysIntr
            ));

    // Link interrupt -> I2C Interrupt Pin
    if ( !InterruptInitialize( g_PmicI2CSysIntr, g_hPmicI2CIntrEvent, NULL, 0 ) )
    {
        DEBUGMSG( ZONE_INIT | ZONE_ERROR, (
                  _T("%s: couldn't associate event with I2C interrupt: %d \r\n"),
                  __WFUNCTION__,
                  GetLastError()
                ));
        goto error;
    }
    DEBUGMSG( ZONE_INIT, ( _T("%s: Linking passed! \r\n"), __WFUNCTION__ ) );

    // Get BSP interrupt wait timeout value
    {
        g_IntrWaitTimeout = BSPPmicI2CGetTimeoutValue();
    }
    DEBUGMSG( ZONE_INIT, (_T("%s: Timeout %d\r\n"), __WFUNCTION__, g_IntrWaitTimeout ) );

    // Get BSP clock frequency
    {
        g_wClockRateDiv = BSPPmicI2CCalculateClkRateDiv( dwFrequency );
        DEBUGMSG( ZONE_INIT, (
                  _T("%s: Frequency %d => divider 0x%X\r\n"),
                  __WFUNCTION__,
                  dwFrequency,
                  g_wClockRateDiv
                ));
    }

    // Create IST for I2C interrupts
    //      pThreadAttributes = NULL (must be NULL)
    //      dwStackSize = 0 => default stack size determined by linker
    //      lpStartAddress = I2CIST => thread entry point
    //      lpParameter = this => point to thread parameter
    //      dwCreationFlags = 0 => no flags
    //      lpThreadId = NULL => thread ID is not returned
    g_hPmicI2CIntrServThread = CreateThread( NULL, 0, PmicI2CIntrServThread, NULL, 0, NULL );
    if ( !g_hPmicI2CIntrServThread )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: CreateThread failed for I2C IST\r\n"),
                  __WFUNCTION__
                ));
        goto error;
    }

    // Bump up our priority to the highest level available to CE drivers.
    // This will make sure we have a chance to signal threads blocked on a
    // I2C transfer before starting the suspend procedure.
    CeSetThreadPriority( g_hPmicI2CIntrServThread, 97 );

    // Disable I2C Module Initially
    BSPPmicI2CReset();
    I2C_DUMP_REGS();

    // Initialize global for sync with threads blocked on I2C transfer
    g_bSync = FALSE;

    // Configure for interrupt-driven communication
    PmicConfigPolling( g_bPmicUsePolling );

    // Wait for the thread to start up.
    if ( WAIT_OBJECT_0 != WaitForSingleObject( g_hPmicI2CWaitEvent, g_IntrWaitTimeout ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: Interrupt thread hasn't started in %dms\r\n"),
                  __WFUNCTION__,
                  g_IntrWaitTimeout
                ));
        goto error;
    }

    DEBUGMSG( ZONE_INIT, (_T("%s -\r\n"), __WFUNCTION__ ));

    return TRUE;

error:
    DEBUGMSG( ZONE_INIT, (_T("%s [error] -\r\n"), __WFUNCTION__ ));

    return FALSE;
}


//-----------------------------------------------------------------------------
//
// Function: PmicI2CRelease
//
// Frees allocated memory, closes reference to handles, and resets the state
// of global member variables.
//
// Parameters:
//      index
//          [in] I2C instance (1 = I2C1, 2 = I2C2) to uninitialize
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID PmicI2CRelease( int index )
{
    UNREFERENCED_PARAMETER( index );

    return;
}


//-----------------------------------------------------------------------------
//
//  Function: PmicI2CPowerDown
//
//  Power down the I2C interface.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID PmicI2CPowerDown()
{
}


//-----------------------------------------------------------------------------
//
//  Function: PmicI2CSync
//
//  Waits for interrupt-driven transactions to complete.
//
//  This is required so the system can suspend properly.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID PmicI2CSync()
{
    // Wait for pending interrupt-driven transfers to complete.  The g_bSync
    // global is set prior to blocking on the I2C transfer and will be cleared
    // by PmicI2CIntrServThread or completion of PmicI2CReceiveData
    while ( g_bSync )
        /* do nothing */;
}


//-----------------------------------------------------------------------------
//
//  Function: PmicI2CLock
//
//  Prepares the I2C for a transaction.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE if successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
BOOL PmicI2CLock()
{
    I2C_TRACE_ENTRY();

    while ( WAIT_OBJECT_0 != WaitForSingleObject( g_hPmicI2CMutex, g_IntrWaitTimeout ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: Couldn't get I2C mutex in %dms\r\n"),
                  __WFUNCTION__,
                  g_IntrWaitTimeout
                ));
        //goto error;
    }

    LOCK_ENTRY();

    // Enable I2C Clock
    BSPPmicSetI2CClockGating( TRUE );

    // Reset I2CR
    BSPPmicI2CReset();
    I2C_TRACE(( _T("%s: Resetting I2CR \r\n"),
               __WFUNCTION__
             ));

    // Try resetting I2DR = 0x0
    BSPPmicI2CTransmitByte( 0x0 );
    I2C_TRACE(( _T("%s: Resetting I2DR \r\n"),
               __WFUNCTION__
             ));

    // Configure data sampling rate
    BSPPmicI2CSetClockDivider( g_wClockRateDiv );
    I2C_TRACE(( _T("%s: Setting clock rate divider 0x%X\r\n"),
               __WFUNCTION__,
               g_wClockRateDiv
             ));

    // Enable I2C
    BSPPmicI2CEnable();
    I2C_TRACE(( _T("%s: Enable I2C \r\n"),
               __WFUNCTION__
             ));

    // Configure the I2C for polled/interrupt-driven mode
    if ( g_bUsePolling )
    {
        // Disable all I2C interrupts
        I2C_TRACE(( _T("%s: Polling - no I2C Interrupt \r\n"),
                   __WFUNCTION__
                 ));
        BSPPmicI2CIRQDisable();
    }
    else
    {
        // Enable interrupt
        I2C_TRACE(( _T("%s: Enable I2C Interrupt \r\n"),
                   __WFUNCTION__
                 ));
        BSPPmicI2CIRQEnable();
    }

    I2C_DUMP_REGS();

    I2C_TRACE_EXIT();

    return TRUE;

//error:
//    I2C_TRACE_EXIT();
//    return FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function: PmicI2CUnlock
//
//  Shuts down the I2C after a transaction.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID PmicI2CUnlock()
{
    I2C_TRACE_ENTRY();
    // Turn off interrupt again
    BSPPmicI2CIRQDisable();

    // Disable I2C module
    BSPPmicI2CDisable();

    // Disable I2C Clock
    BSPPmicSetI2CClockGating( FALSE );

    ReleaseMutex( g_hPmicI2CMutex );

    I2C_TRACE_EXIT();
    LOCK_EXIT();
}

//-----------------------------------------------------------------------------
//
//  Function: PmicI2CIntrServThread
//
//  Interrupt service thread for I2C interrupts.
//
//  Parameters:
//      lpParam     Thread data passed to the function using the
//                  lpParameter parameter of the CreateThread function. Not used.
//
//  Returns:
//      Thread exit code.
//
//-----------------------------------------------------------------------------
static DWORD WINAPI PmicI2CIntrServThread( LPVOID lpParam )
{
    DWORD rc = TRUE;

    UNREFERENCED_PARAMETER( lpParam );

    // Bump up our priority to the highest level available to CE drivers.
    // This will make sure we have a chance to signal threads blocked on a
    // I2C transfer before starting the suspend procedure.
    CeSetThreadPriority( GetCurrentThread(), 97 );

    // Configure for interrupt-driven communication now we can deal with it
    PmicConfigPolling( g_bPmicUsePolling );

    // Set our event to tell the startup code we're ready
    SetEvent( g_hPmicI2CWaitEvent );

    for(;;)
    {
        if ( WAIT_OBJECT_0 == WaitForSingleObject( g_hPmicI2CIntrEvent, INFINITE ) )
        {
            g_bSync = FALSE;
            SetEvent( g_hPmicI2CWaitEvent );
            I2C_TRACE(( _T("%s: I2C interrupt received\r\n"), __WFUNCTION__ ));
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


//-----------------------------------------------------------------------------
//
//  Function: PmicI2CWriteRegister
//
//  Writes data to the specified PMIC register address.
//
//  Parameters:
//      devAddr     PMIC device address
//      regAddr     PMIC register address
//      regval      value to write
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL PmicI2CWriteRegister( BYTE devAddr, UINT32 regAddr, UINT32 regval )
{
    BOOL success;
    BYTE data[3];

    I2C_TRACE_ENTRY();

    data[0] = (BYTE) regAddr;
    data[1] = (BYTE) ((regval & 0xFF00) >> 8);
    data[2] = (BYTE) (regval & 0xFF);

    // Do the write
    success = PmicI2CWriteData( devAddr, data, 3 );

    TRACE_WRITE(( _T("%s: R%02Xh = 0x%04X [%s]\r\n"),
                __WFUNCTION__,
                regAddr,
                regval,
                success ? _T("success") : _T("failed")
              ));

    return success;
}


//-----------------------------------------------------------------------------
//
//  Function: PmicI2CWriteData
//
//  Writes data across the I2C.
//
//  Parameters:
//      devAddr     I2C device address
//      pData       Data to write
//      nBytes      Number of bytes of data
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL PmicI2CWriteData( BYTE devAddr, BYTE *pData, UINT nBytes )
{
    BOOL    success;
    int     retval = I2C_NO_ERROR;

    I2C_TRACE_ENTRY();

    success = PmicI2CLock();
    if ( !success )
        goto cleanUp;

    // Start the transaction
    retval = private_GenerateStart( devAddr, TRUE );
    if ( I2C_NO_ERROR != retval )
    {
        success = FALSE;
        goto cleanUp;
    }

    // Do the write
    retval = private_WriteData( pData, nBytes, FALSE );
    if ( I2C_NO_ERROR != retval )
    {
        success = FALSE;
        goto cleanUp;
    }

cleanUp:
    PmicI2CUnlock();

    I2C_TRACE_EXIT();

    return success;
}


//-----------------------------------------------------------------------------
//
//  Function: PmicI2CReadRegister
//
//  Reads data from the specified PMIC register address.
//
//  Parameters:
//      devAddr     PMIC device address
//      addr        PMIC register address
//      pRegval     value read
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL PmicI2CReadRegister( BYTE devAddr, UINT32 addr, UINT32 *pRegval )
{
    BOOL success;
    BYTE regAddr = (BYTE) addr;
    BYTE data[2];

    I2C_TRACE_ENTRY();

    // Now read the data in
    success = PmicI2CReadData( devAddr, &regAddr, 1, data, 2 );
    if ( !success )
        goto cleanUp;

    *pRegval = (data[0] << 8) | data[1];

cleanUp:
    TRACE_READ(( _T("%s: R%02Xh = 0x%04X [%s]\r\n"),
                __WFUNCTION__,
                regAddr,
                *pRegval,
                success ? _T("success") : _T("failed")
              ));

    return success;
}


//-----------------------------------------------------------------------------
//
//  Function: PmicI2CReadData
//
//  Reads data from the specified I2C register address.
//
//  Parameters:
//      devAddr     PMIC device address
//      pWriteData  Data to write (register address)
//      writeBytes  Number of bytes of write data
//      pReadData   Buffer to receive read data
//      readBytes   Number of bytes of write data
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL PmicI2CReadData( BYTE devAddr,
                      BYTE *pWriteData,
                      UINT writeBytes,
                      BYTE *pReadData,
                      UINT readBytes
                    )
{
    BOOL success;
    int  retval = I2C_NO_ERROR;

    I2C_TRACE_ENTRY();

    success = PmicI2CLock();
    if ( !success )
        goto cleanUp;

    // Start the transaction
    retval = private_GenerateStart( devAddr, TRUE );
    if ( I2C_NO_ERROR != retval )
    {
        success = FALSE;
        goto cleanUp;
    }

    // Write the register address
    retval = private_WriteData( pWriteData, writeBytes, FALSE );
    if ( I2C_NO_ERROR != retval )
    {
        success = FALSE;
        goto cleanUp;
    }

    // Generate a repeated start to switch to readback
    I2C_TRACE(( _T("%s:Issuing REPEATED START command.\r\n"),
               __WFUNCTION__
             ));
    retval = private_RepeatedStart( devAddr, FALSE );
    if ( I2C_NO_ERROR != retval )
    {
        success = FALSE;
        goto cleanUp;
    }

    // Now read the data in
    retval = private_ReadData( pReadData, readBytes, TRUE );
    if ( I2C_NO_ERROR != retval )
    {
        success = FALSE;
        goto cleanUp;
    }

cleanUp:
    PmicI2CUnlock();

    I2C_TRACE_EXIT();

    return success;
}


//-----------------------------------------------------------------------------
//
//  Function: PmicI2CProcessPackets
//
//  Reads data from the specified I2C register address.
//
//  Parameters:
//      devAddr     PMIC device address
//      pWriteData  Data to write (register address)
//      writeBytes  Number of bytes of write data
//      pReadData   Buffer to receive read data
//      readBytes   Number of bytes of write data
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL PmicI2CProcessPackets( I2C_PACKET packets[], INT32 numPackets )
{
    BOOL    success;
    int     i;
    BOOL    bAddrCycleComplete; // Flag to signal if address cycle just completed
    int     retval;

    I2C_TRACE_ENTRY();

    success = PmicI2CLock();
    if ( !success )
        goto cleanUp;

    for ( i = 0; i < numPackets; i++ )
    {
        bAddrCycleComplete = FALSE;
        (*(packets[i].lpiResult)) = I2C_NO_ERROR;

        // Send a START signal if this is our first packet
        if (i == 0)
        {
            retval = private_GenerateStart( packets[i].byAddr,
                                            (I2C_RW_WRITE == packets[i].byRW) ? TRUE : FALSE
                                          );
            if ( I2C_NO_ERROR != retval )
            {
                (*(packets[i].lpiResult)) = retval;
                success = FALSE;
                goto cleanUp;
            }

            bAddrCycleComplete = TRUE;
        }
        // Send a REPEATED START signal if the address
        // changed or the transfer direction changed.
        else if ((packets[i].byAddr != packets[i - 1].byAddr) ||
                   (packets[i].byRW != packets[i - 1].byRW))
        {
            retval = private_RepeatedStart( packets[i].byAddr,
                                            (I2C_RW_WRITE == packets[i].byRW) ? TRUE : FALSE
                                          );
            if ( I2C_NO_ERROR != retval )
            {
                (*(packets[i].lpiResult)) = retval;
                success = FALSE;
                goto cleanUp;
            }

            bAddrCycleComplete = TRUE;
        }

        if (packets[i].byRW == I2C_RW_WRITE)
        {
            // It's a write
            retval = private_WriteData( packets[i].pbyBuf,
                                         packets[i].wLen,
                                         (i + 1 == numPackets)
                                       );
            if ( I2C_NO_ERROR != retval )
            {
                (*(packets[i].lpiResult)) = retval;
                success = FALSE;
                goto cleanUp;
            }
        }
        else
        {
            // It's a read
            retval = private_ReadData( packets[i].pbyBuf,
                                        packets[i].wLen,
                                        (i + 1 == numPackets)
                                      );
            if ( I2C_NO_ERROR != retval )
            {
                (*(packets[i].lpiResult)) = retval;
                success = FALSE;
                goto cleanUp;
            }
        }
    }

cleanUp:
    PmicI2CUnlock();

    I2C_TRACE_EXIT();

    return success;
}


//-----------------------------------------------------------------------------
//
//  Function: private_GenerateStart
//
//  Generates an I2C start
//
//  Parameters:
//      devAddr     Device address to read from
//      bWrite      Whether to read or write
//
//  Returns:
//      I2C_NO_ERROR                - successful
//      I2C_ERR_TRANSFER_TIMEOUT    - timeout waiting for I2C interrupt
//      I2C_ERR_NO_ACK_ISSUED       - no ACK received
//
//-----------------------------------------------------------------------------
static int private_GenerateStart( BYTE devAddr, BOOL bWrite )
{
    int retval;

    I2C_TRACE(( _T("%s: 0x%02X %s\r\n"),
                __WFUNCTION__,
                devAddr,
                bWrite ? _T("write") : _T("read")
             ));
    I2C_DUMP_REGS();

    //
    // Wait until the bus goes quiet then claim ownership.
    //
    BSPPmicI2CWaitWhileBusy();
    BSPPmicI2CClaimMaster();

    // Transmit the device address, then change to receive mode after
    // we complete the address cycle.
    BSPPmicI2CSetTransmitMode( TRUE );
    BSPPmicI2CSetDeviceAddress( devAddr, bWrite );

    // Wait for completion
    retval = private_WaitForCompletion( FALSE, TRUE );

    return retval;
}


//-----------------------------------------------------------------------------
//
//  Function: private_RepeatedStart
//
//  Generates an I2C repeated start
//
//  Parameters:
//      devAddr     Device address to read from
//      bWrite      Whether to read or write
//
//  Returns:
//      I2C_NO_ERROR                - successful
//      I2C_ERR_TRANSFER_TIMEOUT    - timeout waiting for I2C interrupt
//      I2C_ERR_NO_ACK_ISSUED       - no ACK received
//
//-----------------------------------------------------------------------------
static int private_RepeatedStart( BYTE devAddr, BOOL bWrite )
{
    int retval;

    I2C_TRACE(( _T("%s: 0x%02X %s\r\n"),
                __WFUNCTION__,
                devAddr,
                bWrite ? _T("write") : _T("read")
             ));

    // Set the repeated start bit in the I2C CR.
    BSPPmicI2CRepeatedStart();

    // Temporary fix related to Repeated Start.  Delay after repeated start for 1 PAT_REF_CLK period.
    StallExecution(3);

    // Set the device address again
    BSPPmicI2CSetDeviceAddress( devAddr, bWrite );

    // Wait for completion
    retval = private_WaitForCompletion( FALSE, TRUE );

    return retval;
}


//-----------------------------------------------------------------------------
//
//  Function: private_WriteData
//
//  Writes data to the I2C, after the transaction has been initialised.
//
//  Parameters:
//      pData       Buffer to write
//      nBufLen     Buffer length in bytes
//      bLast       Whether this is the end of the transaction
//
//  Returns:
//      I2C_NO_ERROR                - successful
//      I2C_ERR_TRANSFER_TIMEOUT    - timeout waiting for I2C interrupt
//      I2C_ERR_NO_ACK_ISSUED       - no ACK received
//      I2C_ERR_ARBITRATION_LOST    - checkArbitration was TRUE and arbitration was lost
//
//-----------------------------------------------------------------------------
static int private_WriteData( BYTE *pData, UINT nBufLen, BOOL bLast )
{
    PBYTE           pBufPtr = pData;
    unsigned int    i;
    int             retval = I2C_NO_ERROR;

    I2C_DUMP_REGS();

    // Set MTX to switch to transmit mode
    BSPPmicI2CSetTransmitMode( TRUE );

    //
    // Transmit each byte.
    //
    for ( i = 0; i < nBufLen; i++ )
    {
        // Send the byte
        I2C_TRACE(( _T("%s: [%d]: %02x\r\n"),
                   __WFUNCTION__,
                   i,
                   *pBufPtr
                 ));
        BSPPmicI2CTransmitByte( *pBufPtr );
        pBufPtr++;

        retval = private_WaitForCompletion( TRUE, TRUE );
        if ( I2C_NO_ERROR != retval )
        {
            goto done;
        }
    }
    if ( bLast )
    {
        I2C_TRACE(( _T("%s: Send STOP...this is the last packet written.\r\n"),
                   __WFUNCTION__
                 ));
        BSPPmicI2CFinish();
    }

done:
    return retval;
}


//-----------------------------------------------------------------------------
//
//  Function: private_ReadData
//
//  Reads data from the I2C, after the transaction has been initialised.
//
//  Parameters:
//      pBuf                Buffer to read into
//      nBufLen             Buffer length in bytes
//      bLast               Whether this is the end of the transaction
//      bAddrCycleComplete  Whether we've finished the address cycle
//
//  Returns:
//      I2C_NO_ERROR                - successful
//      I2C_ERR_TRANSFER_TIMEOUT    - timeout waiting for I2C interrupt
//      I2C_ERR_NO_ACK_ISSUED       - no ACK received
//
//-----------------------------------------------------------------------------
static int private_ReadData( BYTE *pData, UINT nBufLen, BOOL bLast )
{
    PBYTE           pReadBufPtr = pData;
    unsigned int    i;
    int             retval = I2C_NO_ERROR;

    // Switch to Receive Mode
    BSPPmicI2CSetTransmitMode( FALSE );

    // Clear the TXAK bit to gen an ack when receiving only one byte.
    if ( 1 == nBufLen )
    {
        BSPPmicI2CGenerateAcks( FALSE );
    }
    else
    {
        BSPPmicI2CGenerateAcks( TRUE );
    }

    {
        BYTE dummy;
        I2C_TRACE(( _T("%s: Dummy read to trigger I2C Read operation.\r\n"),
                   __WFUNCTION__
                 ));
        // Dummy read to trigger I2C Read operation
        BSPPmicI2CReceiveByte( &dummy );
    }

    for ( i = 0; i < nBufLen; i++)
    {

        // Wait for data transmission to complete.
        retval = private_WaitForCompletion( FALSE, FALSE );
        if ( I2C_NO_ERROR != retval )
        {
            goto done;
        }

        // Do not generate an ACK for the last byte
        if ( (nBufLen - 2) == i )
        {
            I2C_TRACE(( _T("%s: Change to No ACK for last byte. \r\n"),
                       __WFUNCTION__
                     ));
            BSPPmicI2CGenerateAcks( FALSE );
        }
        else if ( (nBufLen - 1) == i )
        {
            if ( bLast )
            {
                I2C_TRACE(( _T("%s: Send STOP...this is the last packet read.\r\n"),
                           __WFUNCTION__
                         ));
                BSPPmicI2CFinish();
            }
        }

        I2C_TRACE(( _T("%s: Read next byte! \r\n"),
                   __WFUNCTION__
                 ));
        BSPPmicI2CReceiveByte( pReadBufPtr );
        I2C_TRACE(( _T("%s: Byte read: %x \r\n"),
                   __WFUNCTION__,
                   *pReadBufPtr
                 ));
        ++pReadBufPtr;
    }

done:
    return retval;
}


//-----------------------------------------------------------------------------
//
//  Function: private_WaitForCompletion
//
//  Waits for the operation to complete.
//
//  Parameters:
//      checkArbitration        whether to check for arbitration lost
//      checkAck                whether to check for an ACK
//
//  Returns:
//      I2C_NO_ERROR                - successful
//      I2C_ERR_TRANSFER_TIMEOUT    - timeout waiting for I2C interrupt
//      I2C_ERR_NO_ACK_ISSUED       - no ACK received
//      I2C_ERR_ARBITRATION_LOST    - checkArbitration was TRUE and arbitration was lost
//
//-----------------------------------------------------------------------------
static int private_WaitForCompletion( BOOL checkArbitration, BOOL checkAck )
{
    int retval = I2C_NO_ERROR;

    I2C_TRACE_ENTRY();

    I2C_DUMP_REGS();

    // Wait for interrupt event.
    I2C_TRACE(( _T("%s: Waiting for incoming interrupt! \r\n"),
               __WFUNCTION__
             ));
    retval = private_WaitForInterrupt();
    if ( I2C_NO_ERROR != retval )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: Timed out waiting for interrupt!  Aborting I2C transfer.\r\n"),
                  __WFUNCTION__
                ));

        BSPPmicI2CDumpRegs();

        // Send stop signal to abort
        BSPPmicI2CAbort();

        goto done;
    }

    if ( checkArbitration )
    {
        //
        // Check whether we've lost arbitration.
        //
        if ( BSPPmicI2CLostArbitration() )
        {
            //
            // Arbitration lost.  An error has occurred, likely due to a bad
            // slave I2C address.  BSPPmicI2CLostArbitration will clean up for us.
            //
            DEBUGMSG( ZONE_ERROR, (
                      _T("%s: Lost arbitration - aborting I2C transfer.\r\n"),
                      __WFUNCTION__
                    ));

            retval = I2C_ERR_ARBITRATION_LOST;

            goto done;
        }
    }

    if ( checkAck )
    {
        //
        // Check for our ACK.
        //
        if ( BSPPmicI2CAckFailed() )
        {
            //
            // No ACK received.
            //
            DEBUGMSG( ZONE_ERROR, (
                      _T("%s: No ACK, STOP issued! -\r\n"),
                      __WFUNCTION__
                    ));

            //
            // Send a STOP to abort.
            //
            BSPPmicI2CAbort();

            retval = I2C_ERR_NO_ACK_ISSUED;

            goto done;
        }
    }

    // Clear Interrupt Signal
    BSPPmicI2CClearInterrupt();

done:
    return retval;
}


//-----------------------------------------------------------------------------
//
//  Function: private_WaitForInterrupt
//
//  Waits for the I2C interrupt as appropriate to the mode.
//
//  This will poll for the interrupt bit if we're in polling mode, or
//  wait for the interrupt signal otherwise.
//
//  Parameters:
//      None.
//
//  Returns:
//      I2C_NO_ERROR                - successful
//      I2C_ERR_TRANSFER_TIMEOUT    - timeout waiting for I2C interrupt
//
//-----------------------------------------------------------------------------
static int private_WaitForInterrupt()
{
    // Unmask the interrupt so it can be signalled again
    InterruptDone( g_PmicI2CSysIntr );

    if ( g_bUsePolling )
    {
        // Wait until transaction is complete
        BSPPmicI2CWaitInterrupt();
    }

    // Else use interrupt-driven communication
    else
    {

        // Set a global to indicate we are about to block on the I2C
        // transfer.  This will be used during a suspend procedure
        // to see if we need to wait until the transfer is complete
        g_bSync = TRUE;

        // NOTE: Can't use WaitForSingleObject directly on an event
        // registered with InterruptInitialize as the kernel faults
        // on priority inversion!
        if ( WAIT_OBJECT_0 != WaitForSingleObject( g_hPmicI2CWaitEvent, I2C_TIMEOUT ) )
        {
            DEBUGMSG( ZONE_ERROR, (
                      _T("%s: Timed out waiting for interrupt!  Aborting I2C transfer.\r\n"),
                      __WFUNCTION__
                    ));
            goto error;
        }
    }

    return I2C_NO_ERROR;

error:
    return I2C_ERR_TRANSFER_TIMEOUT;
}


//////////////////////////////// END OF FILE ///////////////////////////////////
