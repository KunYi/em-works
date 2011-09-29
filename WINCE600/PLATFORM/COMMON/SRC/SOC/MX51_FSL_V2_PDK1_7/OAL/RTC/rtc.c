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
//  Module: rtc.c
//
//  Real-time clock (RTC) routines.
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

static PCSP_SRTC_REGS g_pSRTC;
static ULARGE_INTEGER g_oalRtcOriginTime;
static DWORD g_oalRtcMaxYear;
static BOOL g_oalRtcMapped = FALSE;
static CRITICAL_SECTION g_oalRtcCS;

#define SRTC_MAX_ELAPSE_YEARS   135
#define SRTC_PWR_GLITCH_VAL     0x41736166

//------------------------------------------------------------------------------
//
//  Function:  OALRtcMap
//
BOOL OALRtcMap(void) 
{
    g_oalRtcMapped = FALSE;
    
    if (!g_pSRTC)
    {
        g_pSRTC = (PCSP_SRTC_REGS) OALPAtoUA(CSP_BASE_REG_PA_SRTC);
        if (!g_pSRTC)
        {
            OALMSG(OAL_ERROR, (
                L"ERROR: OALRtcMap: SRTC null pointer!\r\n"
            ));
            goto cleanUp;
        }
    }

    if (!g_oalRtcCS.hCrit)
    {
        InitializeCriticalSection(&g_oalRtcCS);
        if (!g_oalRtcCS.hCrit)
        {
            OALMSG(OAL_ERROR, (
                L"ERROR: OALRtcMap: RTC critical section failure!\r\n"
            ));
            goto cleanUp;
        }
    }

    g_oalRtcMapped = TRUE;

cleanUp:
    return g_oalRtcMapped;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by WinCE OS to initialize the time after boot. 
//  Input buffer contains SYSTEMTIME structure with default time value. If
//  hardware has persistent real time clock it will ignore this value
//  (or all call).
//
BOOL OALIoCtlHalInitRTC(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, 
                        VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
    BOOL rc = FALSE;
    SYSTEMTIME *lpst = (SYSTEMTIME*)pInpBuffer;
    FILETIME ft;
    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);


    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalInitRTC(...)\r\n"));

    if (pOutSize) {
        *pOutSize = 0;
    }

    // Validate inputs
    if (pInpBuffer == NULL || inpSize < sizeof(SYSTEMTIME)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIoCtlHalInitRTC: Invalid parameter\r\n"
        ));
        goto cleanUp;
    }

    OALMSG(TRUE, (
        L"OALIoCtlHalInitRTC(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        lpst->wYear, lpst->wMonth, lpst->wDay, lpst->wHour, lpst->wMinute,
        lpst->wSecond, lpst->wMilliseconds
    ));

    // Add static mapping for RTC alarm
    OALIntrStaticTranslate(SYSINTR_RTC_ALARM, IRQ_SRTC_NTZ);

    rc = NKSystemTimeToFileTime(lpst, &ft);

    g_oalRtcOriginTime.LowPart = ft.dwLowDateTime;
    g_oalRtcOriginTime.HighPart = ft.dwHighDateTime;
    g_oalRtcMaxYear = lpst->wYear + SRTC_MAX_ELAPSE_YEARS;

    // If we have not done so yet, perform RTC mappings 
    if (!g_oalRtcMapped)
    {
        if (!OALRtcMap())
        {
            OALMSG(OAL_ERROR, (L"ERROR: OALIoCtlHalInitRTC: OALRtcMap failed.\r\n"));
            goto cleanUp;
        }
    }

    // Protect access to RTC hardware
    EnterCriticalSection(&g_oalRtcCS);

    // Check if glitch detector was reset
    if (INREG32(&g_pSRTC->LPPDR) != SRTC_PWR_GLITCH_VAL)
    {    
        OALMSG(OAL_WARN, (L"WARNING: SRTC was reset.\r\n"));

        // Initialize glitch detector
        OUTREG32(&g_pSRTC->LPPDR, SRTC_PWR_GLITCH_VAL);

        // Configure IE bit to exit from INIT state
        INSREG32BF(&g_pSRTC->LPCR, SRTC_LPCR_IE, SRTC_LPCR_IE_NORMAL);

        // Wait for IES to indicate exit from INIT state.  Terminate loop if SRTC enters FAILURE state.
        while ((EXTREG32BF(&g_pSRTC->LPSR, SRTC_LPSR_IES) != SRTC_LPSR_IES_NORMAL) &&
               (EXTREG32BF(&g_pSRTC->LPSR, SRTC_LPSR_STATE_LP) != SRTC_LPSR_STATE_LP_FAILURE));

        // Configure NVE bit to exit from INVALID state
        INSREG32BF(&g_pSRTC->LPCR, SRTC_LPCR_NVE, SRTC_LPCR_NVE_VALID);

        // Wait for NVES to indicate exit from INVALID state.  Terminate loop if SRTC enters FAILURE state.
        while ((EXTREG32BF(&g_pSRTC->LPSR, SRTC_LPSR_NVES) != SRTC_LPSR_NVES_VALID) &&
               (EXTREG32BF(&g_pSRTC->LPSR, SRTC_LPSR_STATE_LP) != SRTC_LPSR_STATE_LP_FAILURE));

        // Check if we successfully entered VALID state
        if (EXTREG32BF(&g_pSRTC->LPSR, SRTC_LPSR_STATE_LP) != SRTC_LPSR_STATE_LP_VALID)
        {
            OALMSG(OAL_WARN, (L"WARNING: SRTC is not in VALID state.\r\n"));
        }
    }

    LeaveCriticalSection(&g_oalRtcCS);
    
cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalInitRTC(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMGetRealTime
//
//  This function is called by the kernel to retrieve the time from
//  the real-time clock.
//
BOOL OEMGetRealTime(LPSYSTEMTIME lpst) 
{
    BOOL rc = FALSE;
    ULARGE_INTEGER time;
    UINT32 lpscmr;
    FILETIME ft;
    
    OALMSG(OAL_RTC&&OAL_FUNC, (L"+OEMGetRealTime\r\n"));

    if(!lpst) goto cleanUp;

    if (!g_oalRtcMapped)
    {
        if (!OALRtcMap())
        {
            OALMSG(OAL_VERBOSE&&OAL_ERROR, (
                L"ERROR: OEMGetRealTime: OALRtcMap failed.\r\n"
            ));
            goto cleanUp;
        }
    }
    
    // Protect access to RTC hardware
    EnterCriticalSection(&g_oalRtcCS);
    
    // LPSCMR is only valid when two consecutive reads
    // return the same result
    do
    {
        lpscmr = INREG32(&g_pSRTC->LPSCMR);
    } while (lpscmr != INREG32(&g_pSRTC->LPSCMR));

    time.QuadPart = lpscmr;
    
    LeaveCriticalSection(&g_oalRtcCS);

    // Convert raw RTC ticks into FILETIME ticks
    //  FILETIME tick = 100 nsec = 1e-7 sec
    time.QuadPart = time.QuadPart * 10000000;

    // Add RTC offset to the time origin
    time.QuadPart += g_oalRtcOriginTime.QuadPart;
    
    // Reformat to FILETIME 
    ft.dwLowDateTime = time.LowPart;
    ft.dwHighDateTime = time.HighPart;

    // Convert to SYSTEMTIME
    rc = NKFileTimeToSystemTime(&ft, lpst);

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"-OEMGetRealTime( %d/%d/%d %d:%d:%d.%03d)\r\n", 
        lpst->wYear, lpst->wMonth, lpst->wDay, lpst->wHour, lpst->wMinute,
        lpst->wSecond, lpst->wMilliseconds
    ));

cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMGetRealTime(rc = %d)\r\n", rc));
    return rc;

}


//------------------------------------------------------------------------------
//
//  Function:  OEMSetRealTime
//
//  This function is called by the kernel to set the real-time clock.
//
BOOL OEMSetRealTime(LPSYSTEMTIME lpst) 
{
    BOOL rc = FALSE;
    ULARGE_INTEGER time;
    FILETIME ft;
    UINT32 i, count;

    if(!lpst) goto cleanUp;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        lpst->wYear, lpst->wMonth, lpst->wDay, lpst->wHour, lpst->wMinute,
        lpst->wSecond, lpst->wMilliseconds
    ));

    if (!g_oalRtcMapped)
    {
        if (!OALRtcMap())
        {
            OALMSG(OAL_VERBOSE&&OAL_ERROR, (
                L"ERROR: OEMSetRealTime: OALRtcMap failed.\r\n"
            ));
            goto cleanUp;
        }
    }

    // Check if requested time exceeds SRTC range
    if (lpst->wYear > g_oalRtcMaxYear)
    {
        OALMSG(OAL_VERBOSE&&OAL_ERROR, (
            L"ERROR: OEMSetRealTime: Unsupported time value.\r\n"
        ));
        goto cleanUp;
    }

    // Convert to FILETIME
    if (!NKSystemTimeToFileTime(lpst, &ft))
    {
        OALMSG(OAL_ERROR, (
            L"ERROR: OEMSetRealTime: NKSystemTimeToFileTime failed.\r\n"
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
            L"ERROR: OEMSetRealTime: Unsupported time value.\r\n"
        ));
        goto cleanUp;
    }

    // Subtract the time origin to determine RTC offset
    time.QuadPart -= g_oalRtcOriginTime.QuadPart;
    
    // Convert FILETIME ticks into seconds
    //  FILETIME tick = 100 nsec = 1e-7 sec
    time.QuadPart = time.QuadPart / 10000000;

    EnterCriticalSection(&g_oalRtcCS);

    // Update the RTC offset
    OUTREG32(&g_pSRTC->LPSCLR, 0);
    OUTREG32(&g_pSRTC->LPSCMR, time.LowPart);

    // Counter write operation has two CKIL clock delay.  Wait
    // for three CKIL edges before releasing critical section
    // so that calls to OEMGetRealTime will read new setting.
    for(i = 0; i < 3; i++)
    {
        count = INREG32(&g_pSRTC->LPSCLR);
        while (INREG32(&g_pSRTC->LPSCLR) == count);
    }

    LeaveCriticalSection(&g_oalRtcCS);

    rc = TRUE;    

cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetRealTime(rc = %d)\r\n", rc));
    return rc;
}


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

    if (!g_oalRtcMapped)
    {
        if (!OALRtcMap())
        {
            OALMSG(OAL_VERBOSE&&OAL_ERROR, (
                L"ERROR: OEMSetAlarmTime: OALRtcMap failed.\r\n"
            ));
            goto cleanUp;
        }
    }

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

    // Clear the alarm status bit (w1c) 
    OUTREG32(&g_pSRTC->LPSR, CSP_BITFMASK(SRTC_LPSR_ALP));

    // Enable the alarm interrupt
    INSREG32BF(&g_pSRTC->LPCR, SRTC_LPCR_ALP, SRTC_LPCR_ALP_ENABLE);

    // Update the alarm time
    OUTREG32(&g_pSRTC->LPSAR, time.LowPart);

    LeaveCriticalSection(&g_oalRtcCS);

    // Re-enable interrupt (it is disabled since last alarm occurs)
    OEMInterruptDone(SYSINTR_RTC_ALARM);

    rc = TRUE;
    
cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------

