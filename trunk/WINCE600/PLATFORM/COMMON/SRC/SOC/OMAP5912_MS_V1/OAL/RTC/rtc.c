//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  rtc.c
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <omap5912.h>

//------------------------------------------------------------------------------
// Defines 

#define BCD2BIN(b)          (((b) >> 4)*10 + ((b)&0xF))
#define BIN2BCD(b)          ((((UINT8)(b)/10) << 4)|((UINT8)(b)%10))

#define RTC_YEAR_BEGIN      2000
#define RTC_YEAR_END        2100

//------------------------------------------------------------------------------
// Local Variables 

static OMAP5912_RTC_REGS *g_pRTCRegs = NULL;

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by WinCE OS to initialize the time after boot.
//  Input buffer contains SYSTEMTIME structure with default time value.
//  If hardware has persistent real time clock it will ignore this value
//  (or all call).
//
BOOL OALIoCtlHalInitRTC(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) 
{
    SYSTEMTIME *lpst = NULL;
    BOOL rc = FALSE;
    UINT8 stat;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalInitRTC(...)\r\n"));

    if(pOutSize) {
        *pOutSize = 0;
    }

    // Validity checks
    if(code!=IOCTL_HAL_INIT_RTC || pInpBuffer==NULL || inpSize!=sizeof(SYSTEMTIME))
    {
        OALMSG(OAL_ERROR, (L"ERROR: Invalid calling parameters...returning\r\n"));
        rc = FALSE;
        goto cleanUp;
    }

    // Initialize virtual address pointer
    if (g_pRTCRegs == NULL) g_pRTCRegs = OALPAtoUA(OMAP5912_RTC_REGS_PA);
    
    // Initialize the control register.
    OUTREG8(&g_pRTCRegs->CTRL, RTC_CTRL_INIT);

    // Wait until RUN is active
    while ((INREG8(&g_pRTCRegs->STAT) & RTC_STAT_RUN) != 0);

    // Initialize interrupt register
    OUTREG8(&g_pRTCRegs->INTR, 0);

    // Save reset status
    stat = INREG8(&g_pRTCRegs->STAT);
    
    // Clear power up status and alarm interrupt
    OUTREG8(&g_pRTCRegs->STAT, RTC_STAT_ALARM|RTC_STAT_RESET);

    // Start the RTC
    SETREG8(&g_pRTCRegs->CTRL, RTC_CTRL_RUN);

    rc = TRUE;
    
    // Set time defined in platform only once after powerup
    lpst = (SYSTEMTIME *)pInpBuffer;
    if ((stat & RTC_STAT_RESET) == RTC_STAT_RESET) {
        rc = OEMSetRealTime(lpst);
    }

    // OMAP5912 has 1s alarm resolution
    g_pOemGlobal->dwAlarmResolution = 1000;
    
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
BOOL OEMGetRealTime(SYSTEMTIME *pTime) 
{
    BOOL rc = FALSE;
    BOOL enabled;
    UINT8 year, month, dweek, day, hour, min, sec;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"+OEMGetRealTime\r\n"));

    if (!pTime) goto cleanUp;

    // Initialize virtual address pointer
    if (g_pRTCRegs == NULL) g_pRTCRegs = OALPAtoUA(OMAP5912_RTC_REGS_PA);

    
    // Disable interrupts when registers are read
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Wait for BUSY low. Anytime BUSY is low, there is
    // at least 15us to carry out the R/W.
    while ((INREG8(&g_pRTCRegs->STAT) & RTC_STAT_BUSY) != 0);

    // Read RTC time registers
    year  = INREG8(&g_pRTCRegs->YEAR);
    month = INREG8(&g_pRTCRegs->MONTH);
    dweek = INREG8(&g_pRTCRegs->WEEKDAY);
    day   = INREG8(&g_pRTCRegs->DAY);
    hour  = INREG8(&g_pRTCRegs->HOURS);
    min   = INREG8(&g_pRTCRegs->MINS);
    sec   = INREG8(&g_pRTCRegs->SECS);

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);

    // Convert from RTC Binary Coded Decimal to SYSTEMTIME and store.
    pTime->wYear         = BCD2BIN(year)+RTC_YEAR_BEGIN;
    pTime->wMonth        = BCD2BIN(month);
    pTime->wDayOfWeek    = BCD2BIN(dweek);
    pTime->wDay          = BCD2BIN(day);
    pTime->wHour         = BCD2BIN(hour);
    pTime->wMinute       = BCD2BIN(min);
    pTime->wSecond       = BCD2BIN(sec);
    pTime->wMilliseconds = 0;

    rc = TRUE;
	

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"-OEMGetRealTime(rc = %d, %d/%d/%d %d:%d:%d.%03d)\r\n", rc,
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

cleanUp:
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:     OEMSetRealTime
//
//  This function is called by the kernel to set the real-time clock.
//
BOOL OEMSetRealTime(SYSTEMTIME *pTime) 
{
    BOOL rc = FALSE;
    BOOL enabled;
    UINT8 year, month, dweek, day, hour, min, sec;

    if(!pTime) goto cleanUp;

    // year must be in [RTC_YEAR_BEGIN, RTC_YEAR_END)
	if(pTime->wYear<RTC_YEAR_BEGIN || pTime->wYear>=RTC_YEAR_END) goto cleanUp;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

    // Initialize virtual address pointer
    if (g_pRTCRegs == NULL) g_pRTCRegs = OALPAtoUA(OMAP5912_RTC_REGS_PA);

    year = BIN2BCD(pTime->wYear-RTC_YEAR_BEGIN);
    month = BIN2BCD(pTime->wMonth);
    dweek = BIN2BCD(pTime->wDayOfWeek);
    day   = BIN2BCD(pTime->wDay);
    hour  = BIN2BCD(pTime->wHour);
    min   = BIN2BCD(pTime->wMinute);
    sec   = BIN2BCD(pTime->wSecond);  

    // Disable interrupts when registers are written
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Wait for BUSY low. Anytime BUSY is low, there is
    // at least 15us to carry out the R/W.
    while ((INREG8(&g_pRTCRegs->STAT) & RTC_STAT_BUSY) != 0);

    // Write RTC time registers.
    OUTREG8(&g_pRTCRegs->YEAR, year);
    OUTREG8(&g_pRTCRegs->MONTH, month);
    OUTREG8(&g_pRTCRegs->WEEKDAY, dweek);
    OUTREG8(&g_pRTCRegs->DAY, day);
    OUTREG8(&g_pRTCRegs->HOURS, hour);
    OUTREG8(&g_pRTCRegs->MINS, min);
    OUTREG8(&g_pRTCRegs->SECS, sec);

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);

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
BOOL OEMSetAlarmTime(SYSTEMTIME *pTime) 
{
    BOOL rc = FALSE;
    BOOL enabled;
    UINT8 year, month, day, hour, min, sec;

    if(!pTime) goto cleanUp;

    // year must be in [RTC_YEAR_BEGIN, RTC_YEAR_END)
    if(pTime->wYear<RTC_YEAR_BEGIN || pTime->wYear>=RTC_YEAR_END) goto cleanUp;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

    // Get virtual address
    if (g_pRTCRegs == NULL) g_pRTCRegs = OALPAtoUA(OMAP5912_RTC_REGS_PA);

    year = BIN2BCD(pTime->wYear-RTC_YEAR_BEGIN);
    month = BIN2BCD(pTime->wMonth);
    day   = BIN2BCD(pTime->wDay);
    hour  = BIN2BCD(pTime->wHour);
    min   = BIN2BCD(pTime->wMinute);
    sec   = BIN2BCD(pTime->wSecond);  

    // Reset interrupt
    OUTREG8(&g_pRTCRegs->STAT, RTC_STAT_ALARM);

    // Disable alarm interrupt for while
    CLRREG8(&g_pRTCRegs->INTR, RTC_INTR_ALARM);

    // Disable interrupts when registers are written
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Wait for BUSY low. Anytime BUSY is low, there is
    // at least 15us to carry out the R/W.
    while ((INREG8(&g_pRTCRegs->STAT) & RTC_STAT_BUSY) != 0);

    // Write RTC time registers.
    OUTREG8(&g_pRTCRegs->ALARM_YEAR, year);
    OUTREG8(&g_pRTCRegs->ALARM_MONTH, month);
    OUTREG8(&g_pRTCRegs->ALARM_DAY, day);
    OUTREG8(&g_pRTCRegs->ALARM_HOURS, hour);
    OUTREG8(&g_pRTCRegs->ALARM_MINS, min);
    OUTREG8(&g_pRTCRegs->ALARM_SECS, sec);

    // Re-enable alarm interrupt
    SETREG8(&g_pRTCRegs->INTR, RTC_INTR_ALARM);

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);
    
    // Re-enable interrupt (it is disabled since last alarm occurs)
    OEMInterruptDone(SYSINTR_RTC_ALARM);

    rc = TRUE;

cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
