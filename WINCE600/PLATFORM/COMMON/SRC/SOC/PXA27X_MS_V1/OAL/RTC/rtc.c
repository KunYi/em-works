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
//  Module: rtc.c
//
//  Real-time clock (RTC) routines for the Intel Bulverde processor.
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <bulverde.h>

//------------------------------------------------------------------------------
// Unions to make it easier to extract bit fields from the 32 bit registers
//
typedef union DAY_REGISTER 
{
    UINT32 value;
    struct
    {
        unsigned seconds    : 6;
        unsigned minutes    : 6;
        unsigned hours      : 5;
        unsigned dayOfWeek  : 3;
        unsigned weekOfMonth: 3;
        unsigned reserved   : 9;
    };
} DayRegister;

typedef union YEAR_REGISTER
{
    UINT32 value;
    struct 
    {
        unsigned dayOfMonth : 5;
        unsigned month      : 4;
        unsigned year       : 12;
        unsigned reserved   : 11;
    };
} YearRegister;

//------------------------------------------------------------------------------
// Internal function to check the validity of the SYSTEMTIME structure before
// setting the RTC or the Alarm time.
static BOOL IsSystemTimeStructValid(LPSYSTEMTIME systemTime);
static BOOL IsLeapYear(WORD wYear);

//------------------------------------------------------------------------------
//
//  Function:  OEMGetRealTime
//
//  Reads the current RTC value and returns a system time.
//
BOOL OEMGetRealTime(LPSYSTEMTIME pTime)
{
    volatile BULVERDE_RTC_REG *pRTCRegs = (volatile BULVERDE_RTC_REG *) OALPAtoVA(BULVERDE_BASE_REG_PA_RTC, FALSE);
    DayRegister  dayReg;
    YearRegister yearReg;
    BOOL     retVal  = FALSE;
    WORD     seconds = 0;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"+OEMGetRealTime(pTime = 0x%x)\r\n", pTime));
    if (!pTime) goto Done;

    do
    {
        // Get register values
        dayReg.value  = (UINT32)pRTCRegs->rdcr;
        seconds = dayReg.seconds;
        yearReg.value = (UINT32)pRTCRegs->rycr; 
    } while (seconds != (pRTCRegs->rdcr & 0x3F));

    // Fill in the SYSTEMTIME structure with the appropriate values
    pTime->wYear      = yearReg.year;
    pTime->wMonth     = yearReg.month;
    pTime->wDay       = yearReg.dayOfMonth;
    pTime->wDayOfWeek = dayReg.dayOfWeek - 1;   // Hardware is [1,7] Structure is [0,6]
    pTime->wHour      = dayReg.hours;
    pTime->wMinute    = dayReg.minutes;
    pTime->wSecond    = dayReg.seconds;
    pTime->wMilliseconds = 0;
    retVal = TRUE;

Done:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMGetRealTime(retVal = %d)\r\n", retVal));
    return(retVal);
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSetRealTime
//
//  Updates the RTC with the specified system time.
//
BOOL OEMSetRealTime(LPSYSTEMTIME pTime) 
{
    volatile BULVERDE_RTC_REG *pRTCRegs = (volatile BULVERDE_RTC_REG *) OALPAtoVA(BULVERDE_BASE_REG_PA_RTC, FALSE);
    BOOL         retVal = FALSE;
    DayRegister  dayReg;
    YearRegister yearReg;
    BOOL enabled;


    if (!pTime) goto Done;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

    if (!IsSystemTimeStructValid(pTime)) goto Done;

    dayReg.value = 0;
    yearReg.value = 0;
    #define RDCRBITSSET 0x000fffff  //RDCR register bits set by the code
    #define RYCRBITSSET 0x001fffff  //RYCR register bits set
    
    // Set up the format of the register variables
    yearReg.year        = pTime->wYear;
    yearReg.month       = pTime->wMonth;
    yearReg.dayOfMonth  = pTime->wDay;
    dayReg.dayOfWeek    = pTime->wDayOfWeek + 1;    // Hardware is [1,7] Structure is [0,6]
    dayReg.hours        = pTime->wHour;
    dayReg.minutes      = pTime->wMinute;
    dayReg.seconds      = pTime->wSecond;

    enabled = INTERRUPTS_ENABLE(FALSE);
    
    // Set the value in the RTC and be sure to write to the day register 
    // after the year...this will set the year register up properly.
    pRTCRegs->rycr = yearReg.value;
    pRTCRegs->rdcr = dayReg.value;
    // Wait for RDCR & RYCR update
    while(((pRTCRegs->rdcr & RDCRBITSSET) != dayReg.value) || ((pRTCRegs->rycr & RYCRBITSSET) != yearReg.value)) {
        pRTCRegs->rycr = yearReg.value;
        pRTCRegs->rdcr = dayReg.value;
    }

    INTERRUPTS_ENABLE(enabled);
    
    retVal         = TRUE;

Done:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetRealTime(retVal = %d)\r\n", retVal));
    return(retVal);
}


//------------------------------------------------------------------------------
//
//  Function:  OEMSetAlarmTime
//
//  Set the RTC alarm time.
//
BOOL OEMSetAlarmTime(LPSYSTEMTIME pTime)
{
    volatile BULVERDE_RTC_REG *pRTCRegs = (volatile BULVERDE_RTC_REG *) OALPAtoVA(BULVERDE_BASE_REG_PA_RTC, FALSE);
    BOOL         retVal = FALSE;
    UINT32       irq;
    DayRegister  dayReg;
    YearRegister yearReg;
    BOOL enabled;


    if (!pTime) goto Done;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

    if (!IsSystemTimeStructValid(pTime)) goto Done;

    // Set up the format of the register variables
    yearReg.year        = pTime->wYear;
    yearReg.month       = pTime->wMonth;
    yearReg.dayOfMonth  = pTime->wDay;
    dayReg.dayOfWeek    = pTime->wDayOfWeek + 1;    // Hardware is [1,7] Structure is [0,6]
    dayReg.hours        = pTime->wHour;
    dayReg.minutes      = pTime->wMinute;
    dayReg.seconds      = pTime->wSecond;

    enabled = INTERRUPTS_ENABLE(FALSE);

    // Set the value in the RTC and be sure to write to the day register 
    // after the year...this will set the year register up properly.
    pRTCRegs->ryar1 = yearReg.value;
    pRTCRegs->rdar1 = dayReg.value;

    INTERRUPTS_ENABLE(enabled);

    retVal          = TRUE;


    // Enable the RTC wristwatch1 alarm
    // (this will also clear the RDAL1 bit if it is set)
    pRTCRegs->rtsr  |= XLLP_RTSR_RDALE1;

    // Enable the RTC alarm interrupt
    irq = IRQ_RTCALARM;
    OALIntrDoneIrqs(1, &irq);
    retVal = TRUE;

Done:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", retVal));
    return(retVal);
}


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
) {
    BOOL rc = FALSE;
    SYSTEMTIME *pTime = (SYSTEMTIME*)pInpBuffer;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalInitRTC(...)\r\n"));

    if(pOutSize) {
        *pOutSize = 0;
    }

    // Validate inputs
    if (pInpBuffer == NULL || inpSize < sizeof(SYSTEMTIME)) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIoCtlHalInitRTC: INVALID PARAMETER\r\n"
        ));
        goto cleanUp;
    }

    rc = OEMSetRealTime(pTime);

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalInitRTC(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
BOOL IsSystemTimeStructValid(LPSYSTEMTIME pTime)
{
    WORD dayCompare = 0;

    if (!pTime)
    { 
        OALMSG(OAL_RTC&&OAL_ERROR, (L"Invalid Argument\r\n"));
        return FALSE;
    }

    // Check the basic validity of the values that are passed in
    if ((pTime->wYear > 4095))
    {
        OALMSG(OAL_RTC&&OAL_ERROR, (L"Invalid Year\r\n"));
        return FALSE;
    }

    if ((pTime->wMonth > 12) || (pTime->wMonth == 0))
    {
        OALMSG(OAL_RTC&&OAL_ERROR, (L"Invalid Month\r\n"));
        return FALSE;
    } 

    if (!(pTime->wHour < 24))
    {
        OALMSG(OAL_RTC&&OAL_ERROR, (L"Invalid Hour\r\n"));
        return FALSE;
    }
    if (!(pTime->wMinute < 60))
    {
        OALMSG(OAL_RTC&&OAL_ERROR, (L"Invalid Minutes\r\n"));
        return FALSE;
    }

    if (!(pTime->wSecond < 60))
    {
        OALMSG(OAL_RTC&&OAL_ERROR, (L"Invalid Minutes\r\n"));
        return FALSE;
    }

    // SYSTEMTIME struct store the day of week starting
    // at 0 while the hardware is expecting a 1.
    if ((pTime->wDayOfWeek >= 7))
    {
        OALMSG(OAL_RTC&&OAL_ERROR, (L"Invalid Day of Week\r\n"));
        return FALSE;
    }

    // Calculate the day comparison value
    if ((pTime->wMonth == 4) ||
        (pTime->wMonth == 6) ||
        (pTime->wMonth == 9) ||
        (pTime->wMonth == 11))
    {
        dayCompare = 30;
    }
    else if (pTime->wMonth == 2)
    {
        dayCompare = (IsLeapYear(pTime->wYear)) ? 29:28;
    }
    else
    {
        dayCompare = 31;
    }

    // This should be checking for a wDay == 0 as an error
    // but the documentation does not specify.
    if ((pTime->wDay > dayCompare))
    {
        OALMSG(OAL_RTC&&OAL_ERROR, (L"Invalid Day of Month\r\n"));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
// Check the year value to determine if this is a leap year or not
//
BOOL IsLeapYear(WORD wYear)
{
    BOOL leap = FALSE;

    // Leap years must be a multiple of 4
    if ((wYear % 4) == 0)
    {
        leap = TRUE;

        // It is not a leap year if it is a multiple of 100
        if ((wYear % 100) == 0)
        {
            leap = FALSE;

            // Unless it is a multiple of 400
            if ((wYear % 400) == 0)
            {
                leap = TRUE;
            }
        }
    }

    return leap;
}

//------------------------------------------------------------------------------
