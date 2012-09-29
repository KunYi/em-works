//**********************************************************************
//                                                                      
// Filename: em9260_rtc.c
//                                                                      
// Description: 
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Use of this source code is subject to the terms of the Cirrus end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to 
// use this source code. For a copy of the EULA, please see the 
// EULA.RTF on your install media.
//
// Copyright(c) Emtronix 2007, All Rights Reserved
//                                                                      
//**********************************************************************
//
// CS&ZHL FEB-15-2009: ISL1208 based RTC are NOT for EM9160
//
#ifndef	EM9160
	
#include <windows.h>
#include <oal.h>
#include <nkintr.h>

#include "AT91SAM926x.h"
#include "em9260_isl1208.h"


#define  RTC_IS_BASED_ON_RTT     1

#ifdef RTC_IS_BASED_ON_RTT

BOOL g_bRTCWithBattery=TRUE;
BOOL g_bRealRTCInitialized=FALSE;

UINT64 RealTimeBias = 0;  // Number of 100-nanosecond intervals since January 1, 1601. 

//
// The RTT counter is incremented every RTT_PERIOD number of slow clock(=32768Hz). 
// If RTT_PERIOD = 328, RTT counter will incremented around every 10ms ->(328/32768)
//
#define RTT_PERIOD				328			

//
//If the RTT timer gose over this value, then we restart it
// 0x337F9800 => RTT counter value for 100 day
//
#define RTT_RESTART_THRESOLD	0x337F9800

static AT91PS_RTTC pRTT = NULL;


BOOL isleap(int year)
{
    BOOL leap=FALSE;

	if ((year % 4) == 0)
	{
        leap = TRUE;
        if ((year % 100) == 0) {
            leap = (year%400) ? 0 : 1;
        }
    }
    return leap;
}


short DayOfThisMonth( short year, short month )
{
	if(month<=7)
	{
		if( month%2) return 31;
		else if( month !=2) return 30;
		else if( isleap( year ) ) return 29;
		else  return 28;
	}
	else
	{
		if( month%2) return 30;
        else         return 31;
	}
}

DWORD  GetDaysFrom1601( LPSYSTEMTIME pTime )
{
	DWORD year;
	DWORD leapYear=0;
	DWORD day;
	WORD  i;
	UCHAR cMonth=pTime->wMonth-1;

	year= pTime->wYear;
	year-=1601;

	leapYear = year/4;
	leapYear-= year/100;
	leapYear+= year/400;

	day= year*365 + leapYear;

	for( i=1; i< pTime->wMonth; i++ )
	{
		day+=DayOfThisMonth( (short)(pTime->wYear) , (short)i );
	}
	day+=pTime->wDay -1;
	return day;
}


__int64 MySystemTimeToFileTime( LPSYSTEMTIME pTime )
{ 
	DWORD    day;
	DWORD    nsDay= 864000000;// 000; //8640,0000,0000
	__int64  time;

	day=GetDaysFrom1601( pTime);
	day*=1000;
	
	time= (__int64)day * nsDay;
	nsDay= ((  60*pTime->wHour  + pTime->wMinute ) *60 +pTime->wSecond ) ;

	time+= (__int64)nsDay*10000000;

	return time;
}

BOOL OEMGetRealTime(SYSTEMTIME *pTime)
{
    UINT64           realTime;
	UINT64           thisTime;
    FILETIME         ft;
	BOOL             bRet;
	SYSTEMTIME       time;
	DWORD            dwCurrentValue;
	BOOL             fWasEnabled;

	if (pRTT == NULL)
	{
		pRTT = (AT91PS_RTTC)OALPAtoVA((DWORD)AT91C_BASE_RTTC, FALSE);	

		// restart RTT with the same pre-divider
		//dwCurrentValue = pRTT->RTTC_RTMR;
		//pRTT->RTTC_RTMR = AT91C_RTTC_RTTRST | dwCurrentValue;		

		// force ISL1208 initialize
		//g_bRTCWithBattery = TRUE;
		//g_bRealRTCInitialized = FALSE;
	}

	//
	// read the current value of RTT
	//
	dwCurrentValue = (DWORD)pRTT->RTTC_RTVR;
	if( dwCurrentValue >= RTT_RESTART_THRESOLD )
	{
		// restart RTT
		dwCurrentValue = pRTT->RTTC_RTMR;
		pRTT->RTTC_RTMR = AT91C_RTTC_RTTRST | dwCurrentValue;		

		//force ISL1208 initialize
		g_bRTCWithBattery = TRUE;
		g_bRealRTCInitialized = FALSE;
	}

    //
    // Implement the realtime clock.
    //
	if( g_bRTCWithBattery )
	{
		if( !g_bRealRTCInitialized)
		{
			// set default date & time 2007-5-8 10:30:0 Tuesday(=2)
			time.wYear=2007;
			time.wMonth=5;
			time.wDay=8; 
			time.wHour=10; 
			time.wMinute=30; 
			time.wSecond=0; 
			time.wDayOfWeek=2;		// Sunday=0, Monday=1,... 
			time.wMilliseconds=0;

			if( InitializeISL1208( &time ) )
			{
				g_bRealRTCInitialized=TRUE;
			}
			else
			{
				g_bRTCWithBattery=FALSE;
			}

			*(__int64*)(&ft)= MySystemTimeToFileTime(&time);

			//
			// convert SYSTEMTIME to UINT64
			//
			thisTime = (UINT64)ft.dwHighDateTime << 32;
			thisTime += ft.dwLowDateTime;

			// read realtime tick
			realTime = (UINT64)pRTT->RTTC_RTVR;		
			// read realtime pre-divider
			dwCurrentValue = pRTT->RTTC_RTMR & AT91C_RTTC_RTPRES;
			// convert realTime to 100ns-format
			realTime *= ( (UINT64)dwCurrentValue * ((UINT64)10000000) );
			realTime /= SLOW_CLOCK_FREQUENCY;

			//
			// make realtime bias for compensation
			//
			realTime = thisTime - realTime;

			//
			// save realtime bias to compensate for the clock time in future
			//
			fWasEnabled = INTERRUPTS_ENABLE(FALSE);	// disable interrupt
			RealTimeBias = realTime;
			INTERRUPTS_ENABLE(fWasEnabled);			// restore interrupt state
		}
	}

    //
    // read the clock registers
    //
	realTime = (UINT64)pRTT->RTTC_RTVR;				
	// read realtime pre-divider
	dwCurrentValue = pRTT->RTTC_RTMR & AT91C_RTTC_RTPRES;
	// convert realTime to 100ns-format
	realTime *= ( (UINT64)dwCurrentValue * ((UINT64)10000000) );
	realTime /= SLOW_CLOCK_FREQUENCY;

	//
	// get realtime bias
	//
	fWasEnabled = INTERRUPTS_ENABLE(FALSE);			// disable interrupt
    thisTime = RealTimeBias;						// convert to "real" time
	INTERRUPTS_ENABLE(fWasEnabled);					// restore interrupt state

	//
	// get current time
	//
	realTime += thisTime;
			
    //
    // load time/data structure
    //
    ft.dwLowDateTime = (DWORD)realTime;
    ft.dwHighDateTime = (DWORD)(realTime >> 32);

	bRet=KFileTimeToSystemTime( &ft, pTime );

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"-OEMGetRealTime(rc = %d %d/%d/%d %d:%d:%d.%03d)\r\n", bRet, 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

    return bRet;
}

BOOL OEMSetRealTime(SYSTEMTIME *pTime)
{
    UINT64           realTime;
	UINT64           thisTime;
    FILETIME         ft;
	BOOL             fWasEnabled;
	DWORD            dwCurrentValue;

	DEBUGMSG(1, (_T("+OEMSetRealTime\r\n")));

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

	if (pRTT == NULL)
	{
		pRTT = (AT91PS_RTTC)OALPAtoVA((DWORD)AT91C_BASE_RTTC, FALSE);	

		//force restart the RTT
		//pRTT->RTTC_RTMR = AT91C_RTTC_RTTRST | RTT_PERIOD;
	}

	if( !ISL1208SetRealTime( pTime ) )
	{
		g_bRTCWithBattery=FALSE;
	}
	
	//
	// update realtime bias
	//
	*(__int64*)(&ft)= MySystemTimeToFileTime(pTime);

	//
	// convert SYSTEMTIME to UINT64
	//
	thisTime = (UINT64)ft.dwHighDateTime << 32;
	thisTime += ft.dwLowDateTime;

	// get read realtime clock
	realTime = (UINT64)pRTT->RTTC_RTVR;		
	// read realtime pre-divider
	dwCurrentValue = pRTT->RTTC_RTMR & AT91C_RTTC_RTPRES;
	// convert realTime to 100ns-format
	realTime *= ( (UINT64)dwCurrentValue * ((UINT64)10000000) );
	realTime /= SLOW_CLOCK_FREQUENCY;

	//
	// make realtime bias for compensation
	//
	realTime = thisTime - realTime;

	//
	// save realtime bias to compensate for the clock time in future
	//
	fWasEnabled = INTERRUPTS_ENABLE(FALSE);	// disable interrupt
	RealTimeBias = realTime;
	INTERRUPTS_ENABLE(fWasEnabled);			// restore interrupt state

    return TRUE;
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
    /*
	ULARGE_INTEGER time;
    FILETIME fileTime;
    BOOL enabled;*/

	DEBUGMSG(1, (_T("OEMSetAlarmTime----------- NOT SUPPORTED !!--------\r\n")));

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

	/*
    // Convert time to miliseconds since Jan 1, 1601
    if (!KSystemTimeToFileTime(pTime, &fileTime)) goto cleanUp;
    time.LowPart = fileTime.dwLowDateTime;
    time.HighPart = fileTime.dwHighDateTime;
    time.QuadPart /= 10000;

    enabled = INTERRUPTS_ENABLE(FALSE);
	g_oalRTCAlarm = time.QuadPart - g_oalRTCTicks;
    INTERRUPTS_ENABLE(enabled);

    // We are done
    rc = TRUE;
      
cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", rc));
    return rc;*/
	
	return rc;
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
BOOL OALIoCtlHalInitRTC(UINT32 code, void* pInpBuffer, UINT32 inpSize, void* pOutBuffer, UINT32 outSize, UINT32* pOutSize)
{
    UINT64      realTime;
	UINT64      thisTime;
    FILETIME    ft;
    BOOL        rc = FALSE;
    SYSTEMTIME *pTime = (SYSTEMTIME*)pInpBuffer;
	BOOL        fWasEnabled;
	DWORD       dwCurrentValue;
	

	RETAILMSG(1, (L"+OALIoCtlHalInitRTC(...)\r\n"));
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalInitRTC(...)\r\n"));

	// Validate inputs
	if (pInpBuffer == NULL || inpSize < sizeof(SYSTEMTIME)) {
		NKSetLastError(ERROR_INVALID_PARAMETER);
		OALMSG(OAL_ERROR, (
			L"ERROR: OALIoCtlHalInitRTC: Invalid parameter\r\n"
		));
		goto cleanUp;
	}
	
	//rc = OEMSetRealTime(pTime);	
	if( g_bRTCWithBattery )
	{
		if( !g_bRealRTCInitialized)
		{
			rc = InitializeISL1208( pTime );
			if( rc )
			{
				g_bRealRTCInitialized=TRUE;
			}
			else
			{
				g_bRTCWithBattery=FALSE;
			}
			*(__int64*)(&ft)= MySystemTimeToFileTime(pTime);

			//
			// convert SYSTEMTIME to UINT64
			//
			thisTime = (UINT64)ft.dwHighDateTime << 32;
			thisTime += ft.dwLowDateTime;

			// read realtime clock
			realTime = (UINT64)pRTT->RTTC_RTVR;		// data in 10 ms
			// read realtime pre-divider
			dwCurrentValue = pRTT->RTTC_RTMR & AT91C_RTTC_RTPRES;
			// convert realTime to 100ns-format
			realTime *= ( (UINT64)dwCurrentValue * ((UINT64)10000000) );
			realTime /= SLOW_CLOCK_FREQUENCY;

			//
			// make realtime bias for compensation
			//
			realTime = thisTime - realTime;

			//
			// save realtime bias to compensate for the clock time in future
			//
			fWasEnabled = INTERRUPTS_ENABLE(FALSE);	// disable interrupt
			RealTimeBias = realTime;
			INTERRUPTS_ENABLE(fWasEnabled);			// restore interrupt state
		}
		else
		{
			rc = TRUE;
		}
	}


cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalInitRTC(rc = %d)\r\n", rc));
    return rc;
}

#else // RTC_IS_BASED_ON_RTT

//------------------------------------------------------------------------------
//
//  Global: g_oalRTCTicks/g_pOALRTCTicks
//
//  This variable in increment by one in timer interrupt handler. It contains
//  relative time in miliseconds since January 1, 1601.
//
UINT64 g_oalRTCTicks = 12685896000000;

//------------------------------------------------------------------------------
//
//  Global: g_oalRTCAlarm
//
//  This variable contains number of miliseconds till next SYSINTR_ALARM. It
//  is zero when alarm isn't active.
//
UINT64 g_oalRTCAlarm = 0;

//------------------------------------------------------------------------------
// Update timer using SC_GetTickCount
// Each time you can try to call this function
void UpdateTimer()
{
	BOOL	enabled = FALSE;
	static	DWORD dwOldTime = 0;
	static	DWORD dwActualTime = 0;
	int		i=0;

	if (dwOldTime==0 && dwActualTime==0)
	{
		dwOldTime = CurMSec;
		for (;i<100000;i++);
	}

//	DEBUGMSG( 1,(TEXT("UpdateTimer, seconds from start:%d\n\r"), CurMSec/1000));

	dwActualTime = CurMSec;

    enabled = INTERRUPTS_ENABLE(FALSE);
	g_oalRTCTicks += dwActualTime - dwOldTime;
    INTERRUPTS_ENABLE(enabled);

	dwOldTime = dwActualTime;
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
BOOL OALIoCtlHalInitRTC(UINT32 code, void* pInpBuffer, UINT32 inpSize, void* pOutBuffer, UINT32 outSize, UINT32* pOutSize)
{
    BOOL rc = FALSE;
    SYSTEMTIME *pTime = (SYSTEMTIME*)pInpBuffer;

	RETAILMSG(1, (L"+OALIoCtlHalInitRTC(...)\r\n"));
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalInitRTC(...)\r\n"));

	// Update timer value
	UpdateTimer();

    // Validate inputs
    if (pInpBuffer == NULL || inpSize < sizeof(SYSTEMTIME)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIoCtlHalInitRTC: Invalid parameter\r\n"
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
//  Function:  OEMGetRealTime
//
//  This function is called by the kernel to retrieve the time from
//  the real-time clock.
//
BOOL OEMGetRealTime(SYSTEMTIME *pTime)
{
    BOOL rc;
    ULARGE_INTEGER time;
    FILETIME fileTime;
    UINT64 ticks;
    BOOL enabled;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"+OEMGetRealTime\r\n"));

	UpdateTimer();

    enabled = INTERRUPTS_ENABLE(FALSE);
	ticks = g_oalRTCTicks;
    INTERRUPTS_ENABLE(enabled);

    // Convert time to appropriate format (in 100ns unit)
    time.QuadPart = ticks * 10000;

    fileTime.dwLowDateTime = time.LowPart;
    fileTime.dwHighDateTime = time.HighPart;

    rc = KFileTimeToSystemTime(&fileTime, pTime);

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"-OEMGetRealTime(rc = %d %d/%d/%d %d:%d:%d.%03d)\r\n", rc, 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSetRealTime
//
//  This function is called by the kernel to set the real-time clock.
//
BOOL OEMSetRealTime(SYSTEMTIME *pTime)
{
    BOOL rc = FALSE;
    ULARGE_INTEGER time;
    FILETIME fileTime;
    BOOL enabled;

	DEBUGMSG(1, (_T("OEMSetRealTime-------------------\r\n")));

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

	UpdateTimer();

    if (!KSystemTimeToFileTime(pTime, &fileTime)) goto cleanUp;

    // Convert time to miliseconds since Jan 1, 1601
    time.LowPart = fileTime.dwLowDateTime;
    time.HighPart = fileTime.dwHighDateTime;
    time.QuadPart /= 10000;

    enabled = INTERRUPTS_ENABLE(FALSE);

	g_oalRTCTicks = time.QuadPart;

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
    ULARGE_INTEGER time;
    FILETIME fileTime;
    BOOL enabled;

	DEBUGMSG(1, (_T("OEMSetAlarmTime-------------------\r\n")));

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

	UpdateTimer();

    // Convert time to miliseconds since Jan 1, 1601
    if (!KSystemTimeToFileTime(pTime, &fileTime)) goto cleanUp;
    time.LowPart = fileTime.dwLowDateTime;
    time.HighPart = fileTime.dwHighDateTime;
    time.QuadPart /= 10000;

    enabled = INTERRUPTS_ENABLE(FALSE);
	g_oalRTCAlarm = time.QuadPart - g_oalRTCTicks;
    INTERRUPTS_ENABLE(enabled);

    // We are done
    rc = TRUE;
      
cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", rc));
    return rc;
}

#endif //RTC_IS_BASED_ON_RTT

#endif	//EM9160
//
// CS&ZHL FEB-15-2009: ISL1208 based RTC are NOT for EM9160
//

// EOF em9260_rtc.c
