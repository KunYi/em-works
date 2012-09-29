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


extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, 
    DDK_CLOCK_GATE_MODE mode);
extern PCSP_CRM_REGS g_pCRM;


DWORD g_oalRtcMaxYear;
PCSP_DRYICE_REGS g_pDryIce;
ULARGE_INTEGER g_oalRtcOriginTime;
CRITICAL_SECTION g_oalRtcCS;

static BOOL g_oalRtcMapped = FALSE;


#define SRTC_MAX_ELAPSE_YEARS   135

void DRYICE_WRITE(PCSP_DRYICE_REGS pDry,REG32* pAddr,DWORD dwValue)
{
    while ((pDry->DSR & (1<<10)) != 0);
    *pAddr = dwValue;
}

DWORD DRYICE_READ(PCSP_DRYICE_REGS pDry,REG32* pAddr)
{
    while ((pDry->DSR & (1<<10)) != 0);
    return *pAddr;
}


//------------------------------------------------------------------------------
//
//  Function:  OALRtcMap
//
BOOL OALRtcMap(void) 
{
    g_oalRtcMapped = FALSE;
    
    if (!g_pDryIce)
    {
        g_pDryIce = (PCSP_DRYICE_REGS) OALPAtoUA(CSP_BASE_REG_PA_DRYICE);
        if (!g_pDryIce)
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
//  Function:  OALInitRTC
//
//  This function is called by OEMInit to initialize the RTC controller
//  It doesn't initialize the system time because this is the job of OALIoCtlHalInitRTC.
//  If the RTC wasn't initialized (not running) when this function was entered, it returns FALSE (TRUE otherwise)

BOOL OALInitRTC()
{
    DWORD dwStatus;
    BOOL bResult = TRUE;    

    // ENGcm07872 : CG_CTL does not work as expected. we need to keep it to 0.
    //CLRREG32(&g_pCRM->CCTL,CSP_BITFMASK(CRM_CCTL_CG_CTL));
    g_pDryIce = (PCSP_DRYICE_REGS) OALPAtoUA(CSP_BASE_REG_PA_DRYICE);    

    RETAILMSG(1,(TEXT("dry ice initial status 0x%x\r\n"),INREG32(&g_pDryIce->DSR)));

    // Clear the non-valid & failure state flags
    DRYICE_WRITE(g_pDryIce,&g_pDryIce->DSR,0x3);

    dwStatus  = DRYICE_READ(g_pDryIce,&g_pDryIce->DSR);
    RETAILMSG(1,(TEXT("dry ice status 0x%x\r\n"),dwStatus));

    // Clear the tamper flags
    DRYICE_WRITE(g_pDryIce,&g_pDryIce->DSR,0x00FF0000);

    //Turn on the RTC if it's not already running
    if ((DRYICE_READ(g_pDryIce,&g_pDryIce->DCR) & CSP_BITFVAL(DRYICE_DCR_TCE,1)) == 0)
    {
        bResult = FALSE;
        RETAILMSG(1,(TEXT("RTC not running. turning it on now\r\n")));
        DRYICE_WRITE(g_pDryIce,&g_pDryIce->DTCLR,0);
        DRYICE_WRITE(g_pDryIce,&g_pDryIce->DTCMR,0);         
        DRYICE_WRITE(g_pDryIce,&g_pDryIce->DCR,DRYICE_READ(g_pDryIce,&g_pDryIce->DCR)|CSP_BITFVAL(DRYICE_DCR_TCE,1));
    }        

    
    // turn off the RTC alarm pin
   //DRYICE_WRITE(g_pDryIce,&g_pDryIce->DCR,
   //     (DRYICE_READ(g_pDryIce,&g_pDryIce->DCR) & ~(CSP_BITFMASK(DRYICE_DCR_APE)))  | CSP_BITFVAL(DRYICE_DCR_APE,DRYICE_DCR_APE_DISABLE));
    // disable the alarm interrupt
    DRYICE_WRITE(g_pDryIce,&g_pDryIce->DIER,
        (DRYICE_READ(g_pDryIce,&g_pDryIce->DIER) & ~(CSP_BITFMASK(DRYICE_DIER_CAIE)))  | CSP_BITFVAL(DRYICE_DIER_CAIE,DRYICE_DIER_CAIE_DISABLE));
    

    // Add static mapping for RTC alarm
    OALIntrStaticTranslate(SYSINTR_RTC_ALARM, IRQ_RTC);
    OEMInterruptEnable(SYSINTR_RTC_ALARM,NULL,0);

    while ((g_pDryIce->DSR & (1<<10)) != 0);
    // ENGcm07872 : CG_CTL does not work as expected. we need to keep it to 0.
    //SETREG32(&g_pCRM->CCTL,CSP_BITFMASK(CRM_CCTL_CG_CTL));


    return bResult;
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

    rc = NKSystemTimeToFileTime(lpst, &ft);

    g_oalRtcOriginTime.LowPart = ft.dwLowDateTime;
    g_oalRtcOriginTime.HighPart = ft.dwHighDateTime;
    g_oalRtcMaxYear = lpst->wYear + SRTC_MAX_ELAPSE_YEARS;

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
    
    EnterCriticalSection(&g_oalRtcCS);
    
    // Convert raw RTC ticks into FILETIME ticks
    //  FILETIME tick = 100 nsec = 1e-7 sec
    time.QuadPart = DRYICE_READ(g_pDryIce,&g_pDryIce->DTCMR);

    LeaveCriticalSection(&g_oalRtcCS);

    time.QuadPart = time.QuadPart * 10000000;

    // Add RTC offset to the time origin
    time.QuadPart += g_oalRtcOriginTime.QuadPart;
    
    // Reformat to FILETIME 
    ft.dwLowDateTime = time.LowPart;
    ft.dwHighDateTime = time.HighPart;

    // Convert to SYSTEMTIME
    rc = NKFileTimeToSystemTime(&ft, lpst);

cleanUp:
    if(lpst)
    {
        OALMSG(OAL_RTC&&OAL_FUNC, (
            L"-OEMGetRealTime(rc = %d, %d/%d/%d %d:%d:%d.%03d)\r\n", 
            rc, lpst->wYear, lpst->wMonth, lpst->wDay, lpst->wHour, lpst->wMinute,
            lpst->wSecond, lpst->wMilliseconds
        ));
    }
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

    // ENGcm07872 : CG_CTL does not work as expected. we need to keep it to 0.
    //CLRREG32(&g_pCRM->CCTL,CSP_BITFMASK(CRM_CCTL_CG_CTL));
    // Update the RTC offset
    DRYICE_WRITE(g_pDryIce,&g_pDryIce->DTCLR, 0);
    DRYICE_WRITE(g_pDryIce,&g_pDryIce->DTCMR, time.LowPart);
    while ((g_pDryIce->DSR & (1<<10)) != 0);
    // ENGcm07872 : CG_CTL does not work as expected. we need to keep it to 0.
    //SETREG32(&g_pCRM->CCTL,CSP_BITFMASK(CRM_CCTL_CG_CTL));

    LeaveCriticalSection(&g_oalRtcCS);

    rc = TRUE;    

cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetRealTime(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------

