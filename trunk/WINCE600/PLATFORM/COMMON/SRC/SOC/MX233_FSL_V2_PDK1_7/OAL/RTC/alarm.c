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
//  Module: alarm.c
//
//  Real-time clock (RTC) routines for the Freescale MX233_iMX233 processor
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#pragma warning(pop)

#include "csp.h"
#include "rtc_persistent.h"

//------------------------------------------------------------------------------
// External Variables
extern CSP_RTC_REGS *pRtcReg;

//------------------------------------------------------------------------------
// External Functions
extern BOOL CheckRealTime(LPSYSTEMTIME pTime);
extern WORD ConvertSystemTimeToDays(LPSYSTEMTIME pTime);
extern BOOL OALRTC_WritePersistentField(Predefined_PersistentBits field, UINT32 uData);
extern BOOL OALRTC_ReadPersistentField(Predefined_PersistentBits field, UINT32 *pData);
extern UINT32 CalculateSeconds(LPSYSTEMTIME lpTime);
extern UINT32 CalculateDays(LPSYSTEMTIME lpTime);

//------------------------------------------------------------------------------
// Defines
typedef enum RtcAlarmMode
{
    //! \brief Alarm will generate an interrupt
    ALARM_MODE_INTERRUPT = 0x1,
    //! \brief Alarm will wake the system
    ALARM_MODE_WAKE = 0x2,
    //! \brief Alarm will both interrupt and wake
    ALARM_MODE_BOTH = 0x3
} RtcAlarmMode_t;

//------------------------------------------------------------------------------
//
// Function: OALRTC_SetAlarmTime
//
// Sets the absolute value in seconds that will trigger the alarm
//
// Parameters:
//      u32Seconds - The alarm time in second
//
// Returns:
//      TRUE/FALSE
//
//------------------------------------------------------------------------------

BOOL OALRTC_SetAlarmTime(UINT32 u32Seconds)
{
    BOOL rc = TRUE;

    // Begin a write to a shadow register
    while((INREG32(&pRtcReg->STAT) & BM_RTC_STAT_NEW_REGS)) ;
    //

    OUTREG32(&pRtcReg->ALARM,u32Seconds);

    // Finish a write to a shadow register
    while((INREG32(&pRtcReg->STAT) & BM_RTC_STAT_NEW_REGS)) ;
    OUTREG32(&pRtcReg->CTRL[SETREG],BM_RTC_CTRL_FORCE_UPDATE);
    OUTREG32(&pRtcReg->CTRL[CLRREG],BM_RTC_CTRL_FORCE_UPDATE);
    while((INREG32(&pRtcReg->STAT) & BM_RTC_STAT_STALE_REGS)) ;
    //

    return (rc);
}
//------------------------------------------------------------------------------
//
// Function: OALRTC_SetAlarmMode
//
// Sets the behavior of the alarm mechanism (interrupt, wake, both)
//
// Parameters:
//      mode - The alarm mode (ALARM_MODE_INTERRUPT, ALARM_MODE_WAKE, or ALARM_MODE_BOTH)
//
// Returns:
//      TRUE/FALSE
//
//------------------------------------------------------------------------------
BOOL OALRTC_SetAlarmMode( RtcAlarmMode_t mode )
{
    BOOL rc = FALSE;

    // Check mode
    if( mode < ALARM_MODE_INTERRUPT || mode > ALARM_MODE_BOTH )
    {
        OALMSG(OAL_ERROR, (
                   L"ERROR: OALRTC_SetAlarmMode: Invalid Alarm Mode\r\n"
                   ));
        goto CleanUp;
    }
    // Enable/disable the alarm interrupt as the mode calls for
    if( mode & ALARM_MODE_INTERRUPT )
    {
        OUTREG32(&pRtcReg->CTRL[SETREG],BM_RTC_CTRL_ALARM_IRQ_EN);
    }
    else
    {
        OUTREG32(&pRtcReg->CTRL[CLRREG],BM_RTC_CTRL_ALARM_IRQ_EN);
    }
    // Enable/disable wake as the mode calls for it
    if( mode & ALARM_MODE_WAKE )
    {
        OALRTC_WritePersistentField( RTC_ALARM_WAKE_EN, 1 );
    }
    else
    {
        OALRTC_WritePersistentField( RTC_ALARM_WAKE_EN, 0 );
    }
    // Done
    rc = TRUE;

CleanUp:
    OALMSG(OAL_FUNC, (L"-OALRTC_SetAlarmMode(rc = %d)\r\n", rc));
    return rc;
}
//------------------------------------------------------------------------------
//
// Function: OALRTC_EnableAlarm
//
// Enables or disables the alarm
//
// Parameters:
//      bEnable  true - enable, false - disable
//
// Returns:
//      TRUE/FALSE
//
//------------------------------------------------------------------------------
BOOL OALRTC_EnableAlarm(BOOL bEnable )
{
    // Enable/disable as called for
    if( bEnable )
    {
        OALRTC_WritePersistentField( RTC_ALARM_EN, 1 );
    }
    else
    {
        OALRTC_WritePersistentField( RTC_ALARM_EN, 0 );
    }
    // Done
    return TRUE;
}
//------------------------------------------------------------------------------
//
//  Function:  OEMSetAlarmTime
//
//  Set the RTC alarm time.
//
BOOL OEMSetAlarmTime(SYSTEMTIME *pTime)
{
    BOOL rc = FALSE;
    UINT32 Seconds,Days,Alarmseconds;
    UINT32 irq;

    OALMSG(1, (
               L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n",
               pTime->wMonth, pTime->wDay, pTime->wYear, pTime->wHour, pTime->wMinute,
               pTime->wSecond, pTime->wMilliseconds
               ));

    if (pTime == NULL) goto cleanUp;

    // Check if the input time is valid or not
    if (CheckRealTime(pTime) == FALSE)
    {
        OALMSG(1, (TEXT("Alarm to be set INVALID!!!:  %u/%u/%u, %u:%u:%u  Day of week:%u \r\n"),
                   pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute, pTime->wSecond,pTime->wDayOfWeek));
        goto cleanUp;
    }

    OALMSG(1, (TEXT("Alarm to be set is OK:  %u/%u/%u, %u:%u:%u  Day of week:%u \r\n"),
               pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute, pTime->wSecond,pTime->wDayOfWeek));

    // calculate time of day in seconds.
    Seconds = CalculateSeconds(pTime);

    // Calculate days.
    Days = CalculateDays(pTime);

    // convert days to seconds
    Alarmseconds = (Days * 86400) + Seconds;

    // Disable the ALARM and clear the IRQ
    OUTREG32(&pRtcReg->CTRL[CLRREG],BM_RTC_CTRL_ALARM_IRQ);
    OUTREG32(&pRtcReg->CTRL[CLRREG],BM_RTC_CTRL_ALARM_IRQ_EN);

    if( !OALRTC_SetAlarmMode(ALARM_MODE_INTERRUPT ))
    {
        OALMSG(OAL_ERROR, (L"OEMSetAlarmTime ***Err*** ddi_rtc_SetAlarmMode failed\r\n"));
        goto cleanUp;
    }

    if( !OALRTC_SetAlarmTime(Alarmseconds ))
    {
        OALMSG(OAL_ERROR, (L"OEMSetAlarmTime ***Err*** ddi_rtc_SetAlarmTime failed\r\n"));
        goto cleanUp;
    }

    // Enable the alarm
    OALRTC_EnableAlarm(TRUE);

    // Enable/clear RTC interrupt
    irq = IRQ_RTC_ALARM;
    OALIntrDoneIrqs(1, &irq);

    // done
    rc = TRUE;

cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", rc));
    return rc;
}
//------------------------------------------------------------------------------
//
//  Function:  OEMSetAlarmTime
//
//  Set the RTC alarm time.
//
UINT32 OALRTCAlarmIntrHandler(ULONG ra)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ra);

    OALMSG(1, (L"OALRTCAlarmIntrHandler Alarm interrupt\r\n"));

    // Disable the ALARM and clear the IRQ
    OUTREG32(&pRtcReg->CTRL[CLRREG],BM_RTC_CTRL_ALARM_IRQ);
    OUTREG32(&pRtcReg->CTRL[CLRREG],BM_RTC_CTRL_ALARM_IRQ_EN);

    // reset the persistent bit
    OALRTC_EnableAlarm(FALSE);

    return SYSINTR_RTC_ALARM;
}
//------------------------------------------------------------------------------
