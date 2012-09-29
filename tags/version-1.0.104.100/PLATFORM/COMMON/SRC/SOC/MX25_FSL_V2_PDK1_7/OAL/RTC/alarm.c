//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Module: alarm.c
//
//  Real-time clock (RTC) alarm routines for the Freescale MX25 processor
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

extern DWORD g_oalRtcMaxYear;
extern PCSP_DRYICE_REGS g_pDryIce;
extern ULARGE_INTEGER g_oalRtcOriginTime;
extern CRITICAL_SECTION g_oalRtcCS;

extern void DRYICE_WRITE(PCSP_DRYICE_REGS pDry,REG32* pAddr,DWORD dwValue);
extern DWORD DRYICE_READ(PCSP_DRYICE_REGS pDry,REG32* pAddr);

extern PCSP_CRM_REGS g_pCRM;
//------------------------------------------------------------------------------
//
//  Function:  OEMSetAlarmTime
//
//  This function is called by the kernel to set the real-time clock alarm.
//
BOOL OEMSetAlarmTime(LPSYSTEMTIME lpst) 
{
    BOOL rc = FALSE;
    ULARGE_INTEGER time;
    FILETIME ft;
    
    if(!lpst) goto cleanUp;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        lpst->wYear, lpst->wMonth, lpst->wDay, lpst->wHour, lpst->wMinute,
        lpst->wSecond, lpst->wMilliseconds
    ));
    
    // Check if requested time exceeds SRTC range
    if (lpst->wYear > g_oalRtcMaxYear)
    {
        OALMSG(OAL_VERBOSE&&OAL_ERROR, (
            L"ERROR: OEMSetAlarmTime: Unsupported time value.\r\n"
        ));
        goto cleanUp;
    }

    // Convert to FILETIME
    if (!NKSystemTimeToFileTime(lpst, &ft))
    {
        OALMSG(OAL_ERROR, (
            L"ERROR: OEMSetAlarmTime: NKSystemTimeToFileTime failed.\r\n"
        ));
        goto cleanUp;
    }
    
    // Reformat FILETIME
    time.LowPart = ft.dwLowDateTime;
    time.HighPart = ft.dwHighDateTime;
    
    // Check if requested time is prior to RTC origin
    if (time.QuadPart < g_oalRtcOriginTime.QuadPart)
    {
        OALMSG(OAL_VERBOSE&&OAL_ERROR, (
            L"ERROR: OEMSetAlarmTime: Unsupported time value.\r\n"
        ));
        goto cleanUp;
    }

    // Subtract the time origin to determine RTC offset
    time.QuadPart -= g_oalRtcOriginTime.QuadPart;
    
    // Convert FILETIME ticks into seconds
    //  FILETIME tick = 100 nsec = 1e-7 sec
    time.QuadPart = time.QuadPart / 10000000;

    EnterCriticalSection(&g_oalRtcCS);

    // ENGcm07872 : CG_CTL does not work as expected. we need to keep it to 0.
    //CLRREG32(&g_pCRM->CCTL,CSP_BITFMASK(CRM_CCTL_CG_CTL));

    // Update the alarm time
    DRYICE_WRITE(g_pDryIce,&g_pDryIce->DCALR, 0);
    DRYICE_WRITE(g_pDryIce,&g_pDryIce->DCAMR, time.LowPart);

    // Clear the alarm status bit (w1c) 
    DRYICE_WRITE(g_pDryIce,&g_pDryIce->DSR, CSP_BITFMASK(DRYICE_DSR_CAF));
    
    // Enable the alarm interrupt
    DRYICE_WRITE(g_pDryIce,&g_pDryIce->DIER,
        (DRYICE_READ(g_pDryIce,&g_pDryIce->DIER) & ~(CSP_BITFMASK(DRYICE_DIER_CAIE)))  | CSP_BITFVAL(DRYICE_DIER_CAIE,DRYICE_DIER_CAIE_ENABLE));
    

    // ENGcm07872 : CG_CTL does not work as expected. we need to keep it to 0.
    //SETREG32(&g_pCRM->CCTL,CSP_BITFMASK(CRM_CCTL_CG_CTL));

    LeaveCriticalSection(&g_oalRtcCS);
    
    // Re-enable interrupt (it is disabled since last alarm occurs)
    OEMInterruptDone(SYSINTR_RTC_ALARM);
   
    rc = TRUE;
    
cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", rc));


    
    return rc;
}

//------------------------------------------------------------------------------
