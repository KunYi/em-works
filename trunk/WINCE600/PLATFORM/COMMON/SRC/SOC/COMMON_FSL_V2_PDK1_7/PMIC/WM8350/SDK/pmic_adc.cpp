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
/// @file   pmic_adc.c
/// @brief  WM8350 PMIC A/D converter functions.
///
//  This file contains the PMIC ADC (A/D Converter, including Touch Screen) SDK
//  interface that is used by applications and other drivers to access registers
//  of the PMIC.
///
/// @version $Id: pmic_adc.cpp 419 2007-04-24 09:25:37Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
///-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)   // Non-standard extension
#include <windows.h>
#pragma warning(pop)
#include <Devload.h>
#include <ceddk.h>
#include "pmic_lla.h"
#include "pmic_ioctl.h"
#include "pmic_adc.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
extern "C" HANDLE               hPMI;           // Our global handle to the PMIC driver
extern "C" WM_DEVICE_HANDLE     g_hWMDevice;    // Our global Wolfson device handle

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

extern DBGPARAM dpCurSettings;

#endif  // DEBUG

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: PmicADCInit
//
// This function initializes PMIC ADC's resources.
//
// Parameters:
//
// Returns:
//              Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCInit(void)
{

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicADCDeinit
//
// This function uninitializes the PMIC ADC's resources.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PmicADCDeinit(void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));
}

//------------------------------------------------------------------------------
//
// Function: PmicADCSetComparatorThresholds
//
// This function sets WHIGH and WLOW for automatic ADC result comparators.
//
// Parameters:
//
//      whigh
//              [in]  a high comparator threshold (<0x3FF).
//      wlow
//              [in]  a low comparator threshold (<0x3FF).
//
// Returns:
//              Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCSetComparatorThresholds(UINT16 whigh, UINT16 wlow)
{
    UNREFERENCED_PARAMETER( whigh );
    UNREFERENCED_PARAMETER( wlow );

    // ### TBD
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicADCGetSingleChannelOneSample
//
// This function gets one channel one sample.
//
// Parameters
//
//      channel
//              [in]    a selected channel.
//      pResult
//              [out]   pointer to  the sampled value.
//
// Returns:
//              Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCGetSingleChannelOneSample(UINT16 channels, UINT16* pResult)
{
    UNREFERENCED_PARAMETER( channels );
    UNREFERENCED_PARAMETER( pResult );

    // ### TBD
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicADCGetSingleChannelEightSamples
//
// This function gets one channel eight samples.
//
// Parameters:
//
//  channels
//      [in]  a selected channel.
//  pResult
//      [out] pointer to  the sampled values (up to 8 sampled values).
//
// Returns:
//              Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCGetSingleChannelEightSamples(UINT16 channels, UINT16* pResult)
{
    UNREFERENCED_PARAMETER( channels );
    UNREFERENCED_PARAMETER( pResult );

    // ### TBD
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicADCGetMultipleChannelsSamples
//
// This function gets one sample for multiple channels.
//
// Parameters:
//
//      channels
//      [in] selected  channels (up to 16 channels).
//      pResult
//      [out] pointer to  the sampled values (up to 16 sampled values).
//           The returned results will be ordered according to the channel selected.
//
// Returns:
//              Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCGetMultipleChannelsSamples(UINT16 channels, UINT16* pResult)
{
    UNREFERENCED_PARAMETER( channels );
    UNREFERENCED_PARAMETER( pResult );

    // ### TBD
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicADCGetHandsetCurrent
//
// This function gets handset battery current measurement values.
//
// Parameters:
//
//      mode
//              [in]     a ADC converter mode: ADC_8CHAN_1X or ADC_1CHAN_8X.
//      pResult
//              [out]    pointer to the handset battery current measurement value(s).
//
// Returns:
//              Status.
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCGetHandsetCurrent(PMIC_ADC_CONVERTOR_MODE mode,
                                     UINT16* pResult)
{
    UNREFERENCED_PARAMETER( mode );
    UNREFERENCED_PARAMETER( pResult );

    // ### TBD
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicADCTouchRead
//
// This function reads 3 pairs of  touch screen sample.
//
// Parameters:
//      x
//          [out] X-coordinate of the point.
//      y
//          [out] Y-coordinate of the point.
//
// Returns:
//      Status.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCTouchRead(UINT16* x, UINT16* y)
{
    UNREFERENCED_PARAMETER( x );
    UNREFERENCED_PARAMETER( y );

    // The WM8350 doesn't support touch screen
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
//  Function: PmicTouchStandby
//
//  This function causes the PMIC touch screen controller to enter standby
//  mode and wait for the next pen down condition.
//
//  Parameters:
//      intEna
//          [in] pen down interrupt enable.
//
//  Returns:
//      Status.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicADCTouchStandby(bool intEna)
{
    UNREFERENCED_PARAMETER( intEna );

    // The WM8350 doesn't support touch screen
    return PMIC_NOT_SUPPORTED;
}
