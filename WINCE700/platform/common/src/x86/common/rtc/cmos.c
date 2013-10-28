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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//  
// -----------------------------------------------------------------------------


// Abstract:  
//   This file implements the NK kernel interfaces for the real time clock.
//   NOTE: This file is also used by bootloader, so don't put any NK specific
//         code in here
 
#include <windows.h>
#include <pc.h>
#include <oal.h>

// NOTE: A problem has been found with some chipsets such that
// setting the time to 23:59:59 on the 29th or 30th day of a month which
// has less than 31 days causes the clock to roll over incorrectly.
// Uncomment the following line to fix this problem.  However, be aware 
// that the fix consists of responding to calls that set the time to 
// HH:MM:59 by instead setting the time to HH:MM:58.
//#define HARDWARE_TIME_SET_PROBLEM 1

void CMOS_Write(
                BYTE offset, 
                BYTE value 
                )
{   
    // Remember, we only change the low order 5 bits in address register
    BYTE cAddr = READ_PORT_UCHAR( (UCHAR*)CMOS_ADDR );
    WRITE_PORT_UCHAR( (UCHAR*)CMOS_ADDR, (cAddr & RTC_ADDR_MASK) | offset );
    WRITE_PORT_UCHAR( (UCHAR*)CMOS_DATA, value );
    
    //
    // NOTE : If we ever update bytes 16-45, we should also recaculate
    // the checksum & store it.  However, We don't currently use any of
    // those bytes so we are not going to worry about it here.
    //
    DEBUGCHK(offset < 16);
}

BYTE CMOS_Read(
               BYTE offset 
               )
{
    BYTE cResult;
    
    // Remember, we only change the low order 5 bits in address register
    BYTE cAddr = READ_PORT_UCHAR( (UCHAR*)CMOS_ADDR );
    WRITE_PORT_UCHAR( (UCHAR*)CMOS_ADDR, (cAddr & RTC_ADDR_MASK) | offset );
    cResult = READ_PORT_UCHAR( (UCHAR*)CMOS_DATA );
    
    return cResult;
}

static BOOL IsTimeEqual(
                        __in const SYSTEMTIME* const lpst1, 
                        __in const SYSTEMTIME* const lpst2
                        ) 
{
    if (lpst1->wYear != lpst2->wYear)           return FALSE;
    if (lpst1->wMonth != lpst2->wMonth)         return FALSE;
    if (lpst1->wDayOfWeek != lpst2->wDayOfWeek) return FALSE;
    if (lpst1->wDay != lpst2->wDay)             return FALSE;
    if (lpst1->wHour != lpst2->wHour)           return FALSE;
    if (lpst1->wMinute != lpst2->wMinute)       return FALSE;
    if (lpst1->wSecond != lpst2->wSecond)       return FALSE;

    return TRUE;
}

// NOTE : Not thread safe.
BOOL Bare_GetRealTime(
                      __out LPSYSTEMTIME lpst
                      )
{
    SYSTEMTIME st;
    LPSYSTEMTIME lpst1 = &st, lpst2 = lpst, lptmp;

    lpst1->wSecond = 61;    // initialize to an invalid value 
    lpst2->wSecond = 62;    // initialize to an invalid value 

    do 
    {
        // exchange lpst1 and lpst2
        lptmp = lpst1;
        lpst1 = lpst2;
        lpst2 = lptmp;

        // wait until not updating
        while (CMOS_Read(RTC_STATUS_A) & RTC_SRA_UIP);

        // Read all the values.
        lpst1->wYear = CMOS_Read(RTC_YEAR);
        lpst1->wMonth = CMOS_Read(RTC_MONTH); 
        // RTC clock stores DO_WEEK 1-7, SYSTEMTIME uses 0-6
        lpst1->wDayOfWeek = CMOS_Read(RTC_DO_WEEK)-1;
        lpst1->wDay = CMOS_Read(RTC_DO_MONTH);
        lpst1->wHour = CMOS_Read(RTC_HOUR); 
        lpst1->wMinute = CMOS_Read(RTC_MINUTE); 
        lpst1->wSecond = CMOS_Read(RTC_SECOND); 

    } 
    while (!IsTimeEqual (lpst1, lpst2));
   
    lpst->wMilliseconds = 0; // Not sure how we would get this
   
    if (!(CMOS_Read (RTC_STATUS_B) & RTC_SRB_DM)) 
    {
        // Values returned in BCD.
        lpst->wSecond = DECODE_BCD(lpst->wSecond);
        lpst->wMinute = DECODE_BCD(lpst->wMinute);
        lpst->wHour   = DECODE_BCD(lpst->wHour);
        lpst->wDay    = DECODE_BCD(lpst->wDay);
        lpst->wDayOfWeek = DECODE_BCD(lpst->wDayOfWeek);
        lpst->wMonth  = DECODE_BCD(lpst->wMonth);
        lpst->wYear   = DECODE_BCD(lpst->wYear);
    }
   
    // OK - PC RTC returns 2009 as 09.
    lpst->wYear += 2000;
   
#ifdef DEBUG_TIME_QUERYS
    NKDbgPrintfW(TEXT("\r\nReal Time %d, %d, %d, %d, %d, %d, %d, %d\r\n"),
                 lpst->wYear,
                 lpst->wMonth,
                 lpst->wDayOfWeek,
                 lpst->wDay,
                 lpst->wHour,
                 lpst->wMinute,
                 lpst->wSecond,
                 lpst->wMilliseconds );
#endif
    return TRUE;
}

// NOTE : Not thread safe.
BOOL Bare_SetRealTime(
                      __in const SYSTEMTIME* const lpst
                      )
{
    BYTE cStatusRegA, cStatusRegB, Year;

#ifdef DEBUG_TIME_QUERYS
    NKDbgPrintfW(TEXT("\r\nSet Real Time %d, %d, %d, %d, %d, %d, %d, %d\r\n"),
                 lpst->wYear,
                 lpst->wMonth,
                 lpst->wDayOfWeek,
                 lpst->wDay,
                 lpst->wHour,
                 lpst->wMinute,
                 lpst->wSecond,
                 lpst->wMilliseconds );
#endif

      // Maximum range supported by CMOS
      if(lpst->wYear < 2000 || lpst->wYear > 2099)
      {
          // Value is outside of range
          return FALSE;
      }
      else
      {
          Year = lpst->wYear % 100;
      }

#ifdef HARDWARE_TIME_SET_PROBLEM
    if (lpst->wSecond == 59) {
       lpst->wSecond = 58;
    }
#endif

    // Read the update in progress bit, wait for it to be clear.  This bit will
    // be set once per second for about 2us (Undoc. PC, page 897)
    do 
    {
        cStatusRegA = CMOS_Read( RTC_STATUS_A);
    } 
    while ( cStatusRegA & RTC_SRA_UIP );

    // Disable updates while we change the values
    cStatusRegB = CMOS_Read( RTC_STATUS_B );
    cStatusRegB |= RTC_SRB_UPDT;
    CMOS_Write( RTC_STATUS_B, cStatusRegB );


    if ( !(cStatusRegB & RTC_SRB_DM) ) 
    {
        // BCD Mode
        CMOS_Write( RTC_YEAR,     (BYTE)(CREATE_BCD(Year))); 
        CMOS_Write( RTC_MONTH,    (BYTE)(CREATE_BCD(lpst->wMonth))); 
        // RTC clock stores DO_WEEK 1-7, SYSTEMTIME uses 0-6
        CMOS_Write( RTC_DO_WEEK,  (BYTE)(CREATE_BCD(lpst->wDayOfWeek+1))); 
        CMOS_Write( RTC_DO_MONTH, (BYTE)(CREATE_BCD(lpst->wDay))); 
        CMOS_Write( RTC_HOUR,     (BYTE)(CREATE_BCD(lpst->wHour))); 
        CMOS_Write( RTC_MINUTE,   (BYTE)(CREATE_BCD(lpst->wMinute))); 
        CMOS_Write( RTC_SECOND,   (BYTE)(CREATE_BCD(lpst->wSecond))); 
        // Not sure how we can do lpst->wMilliseconds;
    } 
    else 
    {
        // Binary mode
        CMOS_Write( RTC_YEAR, (UCHAR)Year); 
        CMOS_Write( RTC_MONTH, (UCHAR)lpst->wMonth); 
        // RTC clock stores DO_WEEK 1-7, SYSTEMTIME uses 0-6
        CMOS_Write( RTC_DO_WEEK, (UCHAR)(lpst->wDayOfWeek+1)); 
        CMOS_Write( RTC_DO_MONTH, (UCHAR)lpst->wDay); 
        CMOS_Write( RTC_HOUR, (UCHAR)lpst->wHour); 
        CMOS_Write( RTC_MINUTE, (UCHAR)lpst->wMinute); 
        CMOS_Write( RTC_SECOND, (UCHAR)lpst->wSecond); 
        // Not sure how we can do lpst->wMilliseconds;
    }

    // Reenable updates
    cStatusRegB &= ~RTC_SRB_UPDT;
    CMOS_Write( RTC_STATUS_B, cStatusRegB );

    return TRUE;
}

// NOTE : Not thread safe.
BOOL Bare_SetAlarmTime(
                       __in const SYSTEMTIME* const lpst
                       )
{
    BYTE cStatusRegA, cStatusRegB, Year;

#ifdef DEBUG_TIME_QUERYS
    NKDbgPrintfW(TEXT("\r\nSet Alarm Time %d, %d, %d, %d, %d, %d, %d, %d\r\n"),
                 lpst->wYear,
                 lpst->wMonth,
                 lpst->wDayOfWeek,
                 lpst->wDay,
                 lpst->wHour,
                 lpst->wMinute,
                 lpst->wSecond,
                 lpst->wMilliseconds );
#endif

    Year = lpst->wYear % 100;

    // Read the update in progress bit, wait for it to be clear.  This bit will
    // be set once per second for about 2us (Undoc. PC, page 897)
    do 
    {
        cStatusRegA = CMOS_Read( RTC_STATUS_A);
    } 
    while ( cStatusRegA & RTC_SRA_UIP );

    // Disable updates while we change the values
    cStatusRegB = CMOS_Read( RTC_STATUS_B );
    cStatusRegB |= RTC_SRB_UPDT;
    CMOS_Write( RTC_STATUS_B, cStatusRegB );

    if ( !(cStatusRegB & RTC_SRB_DM) ) 
    {
        // BCD Mode
        CMOS_Write( RTC_ALRM_HOUR,     (BYTE)(CREATE_BCD(lpst->wHour))); 
        CMOS_Write( RTC_ALRM_MINUTE,   (BYTE)(CREATE_BCD(lpst->wMinute))); 
        CMOS_Write( RTC_ALRM_SECOND,   (BYTE)(CREATE_BCD(lpst->wSecond))); 
    } 
    else 
    {
        // Binary mode
        CMOS_Write( RTC_ALRM_HOUR, (UCHAR)lpst->wHour); 
        CMOS_Write( RTC_ALRM_MINUTE, (UCHAR)lpst->wMinute); 
        CMOS_Write( RTC_ALRM_SECOND, (UCHAR)lpst->wSecond); 
    }

    // Enable alarm interrupt and reenable updates
    cStatusRegB = (cStatusRegB | RTC_SRB_AI) & ~RTC_SRB_UPDT;
    CMOS_Write( RTC_STATUS_B, cStatusRegB );

    return TRUE;
}
