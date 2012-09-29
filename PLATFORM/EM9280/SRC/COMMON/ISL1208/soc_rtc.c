//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_rtc.c
//
//  This file contains routine to initial RTC.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#pragma warning(pop)

#include "csp.h"
#include "rtc_persistent.h"

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregRTC;
//-----------------------------------------------------------------------------
//  Global variables

//------------------------------------------------------------------------------
// External Functions
extern BOOL OALRTC_ReadPersistentField(Predefined_PersistentBits field, UINT32 *pData);
extern BOOL OALRTC_WritePersistentField(Predefined_PersistentBits field, UINT32 data );

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions
BOOL OALRTC_ConfigurePowerUpClockSource();

//-----------------------------------------------------------------------------
//
//  Function:  InitRTC
//
//  Init RTC.
//
//  Parameters:
//
//  Returns:
//      TRUE if successful.
//
//-----------------------------------------------------------------------------
BOOL InitRTC()
{
    BOOL rc = FALSE;
    UINT32 uSpare;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"+InitRTC(...)\r\n"));

    // MAP the Hardware registers
    if(pv_HWregRTC == NULL)
    {
        pv_HWregRTC = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_RTC, FALSE);
    }
    
    if (!pv_HWregRTC)
    {
        OALMSG(OAL_ERROR, (L"ERROR: InitRTC: pv_HWregRTC null pointer!\r\n"));
        goto cleanUp;
    }
    
    // Immediately bail if there is no RTC hardware
    if(!(HW_RTC_STAT_RD() & BM_RTC_STAT_RTC_PRESENT))
    {
        OALMSG(OAL_ERROR, (L"ERROR: InitRTC: NO RTC Present!\r\n"));
        goto cleanUp;
    }

    // Immediately bail if there is no alarm hardware
    if(!(HW_RTC_STAT_RD() & BM_RTC_STAT_ALARM_PRESENT))
    {
        OALMSG(OAL_ERROR, (L"ERROR: InitRTC: NO ALARM Present!\r\n"));
        goto cleanUp;
    }

    // Remove soft reset
    HW_RTC_CTRL_CLR(BM_RTC_CTRL_SFTRST);

    // Remove clock gate
    HW_RTC_CTRL_CLR(BM_RTC_CTRL_CLKGATE);

    while(HW_RTC_STAT_RD() & BM_RTC_STAT_STALE_REGS);

    // Configure clock source & power up/down appropriate crystals

    OALRTC_ConfigurePowerUpClockSource();

    //
    while(HW_RTC_STAT_RD() & BM_RTC_STAT_STALE_REGS);

    // Clearing AUTO_RESTART
    OALRTC_WritePersistentField(RTC_AUTO_RESTART, 0);

    // might need to put 378x chip revision checking here.
    uSpare = HW_RTC_PERSISTENT0_RD() & BM_RTC_PERSISTENT0_SPARE_ANALOG;
    uSpare = uSpare >> BP_RTC_PERSISTENT0_SPARE_ANALOG;
    if ( uSpare != 2 )
    {
        // set hld_gnd_z : clears up audio issue on 378x.
        HW_RTC_PERSISTENT0_CLR(BM_RTC_PERSISTENT0_SPARE_ANALOG);
        HW_RTC_PERSISTENT0_SET(BF_RTC_PERSISTENT0_SPARE_ANALOG(2));
    }

    rc = TRUE;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"-InitRTC(...)\r\n"));

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  OALRTC_ConfigurePowerUpClockSource
//
//  Configure the RTC clock source.
//
//  Parameters:
//
//  Returns:
//      TRUE if successful.
//
//-----------------------------------------------------------------------------
BOOL OALRTC_ConfigurePowerUpClockSource()
{
    BOOL use24MHzCrystal = TRUE;

    // See if 32.768KHz crystal is present
    if(!(HW_RTC_STAT_RD() & BM_RTC_STAT_XTAL32768_PRESENT))
    {    
        // 32.768KHz crystal is present and no compile time override
        use24MHzCrystal = FALSE;
    }
    else
    {
        // No 32.768KHz crystal is present
        use24MHzCrystal = TRUE;
    }

    // Force the RTC to use internal 24MHz,because the 32.768KHz is not accurate,
    // this is align with the IC owner.
    use24MHzCrystal = TRUE;

    // Make final clock source adjustment & power up selected crystal
    if(use24MHzCrystal)
    {
        // Clear bits to use 24MHz crystal & power it up
        OALRTC_WritePersistentField(RTC_XTAL24MHZ_PWRUP, 1);
        OALRTC_WritePersistentField(RTC_CLOCKSOURCE, RTC_SOURCE_CHOICE_24MHZ );
        // Power down 32.768KHz crystal
        OALRTC_WritePersistentField( RTC_XTAL32KHZ_PWRUP, 0);
    }
    else
    {
        // Power up 32.768KHz
        OALRTC_WritePersistentField( RTC_XTAL32KHZ_PWRUP, 1);
        // Power down 24MHz crystal & select 32.768KHz as clock source
        OALRTC_WritePersistentField( RTC_XTAL24MHZ_PWRUP, 0);
        OALRTC_WritePersistentField( RTC_XTAL32_FREQ, 0);
        OALRTC_WritePersistentField( RTC_CLOCKSOURCE, RTC_SOURCE_CHOICE_32KHZ );
    }
    return TRUE;
}
