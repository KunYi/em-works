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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  timer.c
//
//  This file contains SoC-specific routines to support the OAL timer.
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
CSP_RTC_REGS *pRtcReg;

//-----------------------------------------------------------------------------
//  Global variables

//------------------------------------------------------------------------------
// External Functions
extern BOOL OALRTC_ReadPersistentField(Predefined_PersistentBits field,  UINT32 *pData) ;
extern BOOL OALRTC_WritePersistentField(Predefined_PersistentBits field, UINT32 data ) ;

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions
BOOL OALRTC_ConfigurePowerUpClockSource();

BOOL InitRTC()
{
    BOOL rc = FALSE;
    UINT32 uSpare;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"+InitRTC(...)\r\n"));

    // MAP the Hardware registers
    pRtcReg = (PCSP_RTC_REGS) OALPAtoUA(CSP_BASE_REG_PA_RTC);
    if (!pRtcReg)
    {
        OALMSG(OAL_ERROR, (
                   L"ERROR: InitRTC: pv_HWregRTC null pointer!\r\n"
                   ));
        goto cleanUp;
    }
    // Immediately bail if there is no RTC hardware
    if( !(INREG32(&pRtcReg->STAT) & BM_RTC_STAT_RTC_PRESENT))
    {
        OALMSG(OAL_ERROR, (
                   L"ERROR: InitRTC: NO RTC Present!\r\n"
                   ));
        goto cleanUp;
    }

    // Immediately bail if there is no alarm hardware
    if( !(INREG32(&pRtcReg->STAT) & BM_RTC_STAT_ALARM_PRESENT))
    {
        OALMSG(OAL_ERROR, (
                   L"ERROR: InitRTC: NO ALARM Present!\r\n"
                   ));
        goto cleanUp;
    }

    // Remove soft reset
    OUTREG32(&pRtcReg->CTRL[CLRREG],BM_RTC_CTRL_SFTRST);

    // Remove clock gate
    OUTREG32(&pRtcReg->CTRL[CLRREG],BM_RTC_CTRL_CLKGATE);

    while((INREG32(&pRtcReg->STAT) & BM_RTC_STAT_STALE_REGS)) ;

    // Configure clock source & power up/down appropriate crystals

    OALRTC_ConfigurePowerUpClockSource();

    //
    while((INREG32(&pRtcReg->STAT) & BM_RTC_STAT_STALE_REGS)) ;

    // Clearing AUTO_RESTART
    OALRTC_WritePersistentField(RTC_AUTO_RESTART, 0);

    // might need to put 378x chip revision checking here.
    uSpare = INREG32(&pRtcReg->PERSISTENT0) & BM_RTC_PERSISTENT0_SPARE_ANALOG;
    uSpare = uSpare >> BP_RTC_PERSISTENT0_SPARE_ANALOG;
    if ( uSpare != 2 )
    {
        // set hld_gnd_z : clears up audio issue on 378x.
        OUTREG32(&pRtcReg->PERSISTENT0[CLRREG],BM_RTC_PERSISTENT0_SPARE_ANALOG);
        OUTREG32(&pRtcReg->PERSISTENT0[SETREG],BF_RTC_PERSISTENT0_SPARE_ANALOG(2));
    }

    rc = TRUE;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"-InitRTC(...)\r\n"));

cleanUp:
    return rc;
}
//
//! \brief RTC device driver power up clock source selection algorithm
//!
//! \fntype Function
//!
//! Selects the crystal source at power up for the RTC analog block
//! according to package type and compile-time flags.
//!
//! \retval SUCCESS                    If no error has occurred
//!
//! \pre None
//!
//! \note None
//
BOOL OALRTC_ConfigurePowerUpClockSource()
{
    BOOL use24MHzCrystal = TRUE;

    // See if 32.768KHz crystal is present
    if( !(INREG32(&pRtcReg->STAT) & BM_RTC_STAT_XTAL32768_PRESENT))
    {
        // 32.768KHz crystal is present and no compile time override
        use24MHzCrystal = FALSE;
    }
    else
    {
        // No 32.768KHz crystal is present
        use24MHzCrystal = TRUE;
    }

    // Make final clock source adjustment & power up selected crystal
    if( use24MHzCrystal )
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
        OALRTC_WritePersistentField( RTC_CLOCKSOURCE, RTC_SOURCE_CHOICE_32KHZ );
    }
    return TRUE;
}
