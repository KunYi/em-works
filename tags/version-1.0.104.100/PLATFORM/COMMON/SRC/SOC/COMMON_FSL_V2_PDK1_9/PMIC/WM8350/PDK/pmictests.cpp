//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
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
/// @file   pmictests.cpp
/// @brief  Test code for the WM8350 PMIC functions.
///
/// This file contains tests for the PMIC platform-specific functions that
/// provide control over the Power Management IC.
///
/// @version $Id: pmictests.cpp 453 2007-05-02 11:33:48Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
///-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4115)
#pragma warning(disable: 4214)
#include <windows.h>
#pragma warning(pop)
#include <nkintr.h>
#include <cmnintrin.h>
#include <Devload.h>
#include <ceddk.h>

#include "pmi2cutil.h"
#include "WMPmic.h"
#include "WM8350.h"
#include "WM8350Util.h"


//-----------------------------------------------------------------------------
// External Functions
extern "C"  BOOL SetRegister( UINT32 regAddr, UINT32 regval, UINT32 mask );
extern "C"  BOOL GetRegister( UINT32 regAddr, UINT32* pRegval );

//-----------------------------------------------------------------------------
// Defines
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

#endif  // DEBUG

#define TEST_COMMS              TRUE
#define TEST_TOUCH              TRUE

#pragma warning( disable: 4127 )    // expression is always constant
#define RUN_TEST( _fn ) do                                                      \
{                                                                               \
    DEBUGMSG( ZONE_INFO, (_T("%s: running test %s\r\n"),                        \
              _T(__FUNCTION__),                                                 \
              _T(#_fn)                                                          \
            ));                                                                 \
    tests++;                                                                    \
    int testErrors = _fn( hDevice );                                            \
    if ( testErrors )                                                           \
    {                                                                           \
        errors += testErrors;                                                   \
        failures++;                                                             \
        DEBUGMSG( ZONE_ERROR, (                                                 \
                  _T("%s: %d errors from %s\r\n"),                              \
                  _T(__FUNCTION__),                                             \
                  errors,                                                       \
                  _T(#_fn)                                                      \
                ));                                                             \
    }                                                                           \
} while ( 0 )

#define PMIC_ALL_BITS       0xFFFF

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions
int TestTouch( WM_DEVICE_HANDLE hDevice );
int TestComms( WM_DEVICE_HANDLE hDevice );

//-----------------------------------------------------------------------------
//
// Function: PMICTests
//
// Runs tests for the PMIC module and associated functions.
//
// Parameters:
//      None.
//
// Returns:
//      Number of failing tests.
//
//-----------------------------------------------------------------------------
int PMICTests( WM_DEVICE_HANDLE hDevice )
{
    int         errors = 0;
    int         tests = 0;
    int         failures = 0;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), _T(__FUNCTION__)));

#if TEST_COMMS
    RUN_TEST( TestComms );
#endif // TEST_COMMS

#if TEST_TOUCH
    RUN_TEST( TestTouch );
#endif // TEST_TOUCH

    DEBUGMSG( ZONE_INFO, (_T("%s: %d tests run, %d passed, %d failed, %d errors\r\n"),
              _T(__FUNCTION__),
              tests,
              tests - failures,
              failures,
              errors
            ));

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), _T(__FUNCTION__)));

    return failures;
}

//-----------------------------------------------------------------------------
//
// Function: TouchTests
//
// Runs tests for the touch panel chip.
//
// Parameters:
//      None.
//
// Returns:
//      Number of failing tests.
//
//-----------------------------------------------------------------------------
int TestTouch( WM_DEVICE_HANDLE hDevice )
{
    int         errors = 0;
    int         i;

    UNREFERENCED_PARAMETER( hDevice );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), _T(__FUNCTION__)));

    //
    // Try talking to the touch controller.
    //
    for ( i = 0; i < 20; i++ )
    {
        BYTE writeData[4], readData[4];

        writeData[0] = 0xC0;

        if ( PmicI2CReadData( 0x48, writeData, 1, readData, 2 ) )
        {
            DEBUGMSG( ZONE_INFO, (
                      _T("Read 0x%02X%02X from touch panel\r\n"),
                      readData[0],
                      readData[1]
                    ));
        }
        else
        {
            DEBUGMSG( ZONE_ERROR, (
                      _T("Touch panel read error\r\n")
                    ));
            errors++;
        }
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), _T(__FUNCTION__)));

    return errors;
}

//-----------------------------------------------------------------------------
//
// Function: CommsTests
//
// Runs tests for the basic register communications.
//
// Parameters:
//      None.
//
// Returns:
//      Number of failing tests.
//
//-----------------------------------------------------------------------------
int TestComms( WM_DEVICE_HANDLE hDevice )
{
    int         errors = 0;
    WM_STATUS   status;
    int         i;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), _T(__FUNCTION__)));

    // Test reading
    status = WMDumpRegs( hDevice );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (_T("%s: WMDumpRegs failed: 0x%X\r\n"),
                  _T(__FUNCTION__),
                  status
                ));
        errors++;
    }

    // Test writing
    for ( i = 0; i <= 5; i++ )
    {
        WM_REGVAL regval1, regval2;

        regval1 = (WM_REGVAL) (i * 0x15);
        WMWrite( hDevice, 0x7, regval1 );
        WMRead( hDevice, 0x7, &regval2 );

        if ( regval2 != regval1 )
        {
            DEBUGMSG( ZONE_ERROR, (
                      _T("Error in write %d: 0x%04X != 0x%04X\r\n"),
                      regval2, regval1
                    ));
            errors++;
        }
    }

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), _T(__FUNCTION__)));

    return errors;
}

