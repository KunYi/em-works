//**********************************************************************
//                                                                      
// Filename: realtimeclock.c
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
// Copyright(c) Cirrus Logic Corporation 2005, All Rights Reserved
//                                                                      
//**********************************************************************

#include <windows.h>
#include <nkintr.h>
#include <hwdefs.h>


DWORD dwReschedIncrement;

#ifdef REAL_RTC

extern BOOL InitializeISL1208(LPSYSTEMTIME ptime);
//extern BOOL ISL1208GetRealTime( LPSYSTEMTIME  ptime );
extern BOOL ISL1208SetRealTime(LPSYSTEMTIME ptime);

BOOL g_bRTCWithBattery=TRUE;
BOOL g_bRealRTCInitialized=FALSE;

unsigned __int64 RealTimeBias = 0;  // Number of 100-nanosecond intervals since
                                    // January 1, 1601. 
typedef volatile struct _rtcif_tag 
{
	DWORD	dr;			// data register
	DWORD	mr;			// match register
	union {
		DWORD	stat;		// interrupt status (read only)
		DWORD	eoi;		// interrupt clear (write only)
	};
	DWORD	lr;			// load register
	DWORD	cr;			// control register
} RTCIF, *PRTCIF;


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
	__int64 time;

	day=GetDaysFrom1601( pTime);
	day*=1000;
	
	time= (__int64)day * nsDay;
	nsDay= ((  60*pTime->wHour  + pTime->wMinute ) *60 +pTime->wSecond ) ;

	time+= (__int64)nsDay*10000000;

	return time;
}

BOOL OEMGetRealTime(LPSYSTEMTIME lpst)
{
    unsigned __int64 realTime;
    FILETIME ft;
	BOOL bRet;
    //
    // Implement the realtime clock.
    //
    PRTCIF pRTC = (PRTCIF) RTC_BASE;
    DWORD dwRTCDR = pRTC->dr;

	if( g_bRTCWithBattery ){

		if( !g_bRealRTCInitialized ){
		
			SYSTEMTIME time;
			time.wYear=2007;
			time.wMonth=1;
			time.wDay=25; 
			time.wHour=8; 
			time.wMinute=30; 
			time.wSecond=30; 

			time.wDayOfWeek=4;		// Sunday=0, Monday=1,... 

			time.wMilliseconds=0;

			if( InitializeISL1208( &time ) )
				g_bRealRTCInitialized=TRUE;
			else
				g_bRTCWithBattery=FALSE;

			*(__int64*)(&ft)= MySystemTimeToFileTime(&time);

			realTime = (unsigned __int64) dwRTCDR;		// data in seconds
			realTime *= 1000;							// convert to ms
			realTime *= 10000;							// convert to 100ns
			
			// get the desired "real" time
			RealTimeBias = (unsigned __int64) ft.dwHighDateTime << 32;
			RealTimeBias += ft.dwLowDateTime;
		
			// compensate for the clock time
			RealTimeBias -= realTime;
		}
	}
    //
    // read the clock registers
    //
    realTime = (unsigned __int64) dwRTCDR;		// data in seconds
    realTime *= 1000;							// convert to ms
    realTime *= 10000;							// convert to 100ns
    realTime += RealTimeBias;					// convert to "real" time

    //
    // load time/data structure
    //
    ft.dwLowDateTime = (DWORD)realTime;
    ft.dwHighDateTime = (DWORD)(realTime >> 32);

	bRet=KFileTimeToSystemTime( &ft, lpst );

	RETAILMSG(0,(TEXT("Time %d/%d/%d  %d:%d:%d --%d(%d)\r\n")
		,lpst->wYear
		,lpst->wMonth
		,lpst->wDay
		,lpst->wHour
		,lpst->wMinute
		,lpst->wSecond
		,lpst->wDayOfWeek
		,bRet
		));

    return bRet;
}

BOOL OEMSetRealTime(LPSYSTEMTIME lpst)
{
    unsigned __int64 realTime;
    FILETIME ft;
    //
    // Impliment battery backed real time clock.
    //
    //
    PRTCIF pRTC = (PRTCIF) RTC_BASE;
    DWORD dwRTCDR = pRTC->dr;

	if( !ISL1208SetRealTime( lpst ) )
		g_bRTCWithBattery=FALSE;
	
	*(__int64*)(&ft)= MySystemTimeToFileTime(lpst);
//    if (bRet = KSystemTimeToFileTime(lpst, &ft))
    {
        // read the clock registers
        realTime = (unsigned __int64) dwRTCDR;		// data in seconds
        realTime *= 1000;							// convert to ms
        realTime *= 10000;							// convert to 100ns
		
        // get the desired "real" time
        RealTimeBias = (unsigned __int64) ft.dwHighDateTime << 32;
        RealTimeBias += ft.dwLowDateTime;
	
		// compensate for the clock time
        RealTimeBias -= realTime;
    }
    return TRUE;
}

BOOL OEMSetAlarmTime(LPSYSTEMTIME lpst)
{
    FILETIME ft;
    BOOL bRet = FALSE;
    ULARGE_INTEGER alarmTime, currentTime, deltaTime;
    PRTCIF pRTC = (PRTCIF) RTC_BASE;
    DWORD dwRTCDR = pRTC->dr;
	
    // get the desired alarm time
    if (bRet = KSystemTimeToFileTime(lpst, &ft)) {
		alarmTime.LowPart = ft.dwLowDateTime;
		alarmTime.HighPart = ft.dwHighDateTime;
		alarmTime.QuadPart -= RealTimeBias;
		
        // get the current time
        currentTime.QuadPart = (unsigned __int64) dwRTCDR;		// data in seconds
        currentTime.QuadPart *= 1000;							// convert to ms
        currentTime.QuadPart *= 10000;						// convert to 100ns
		
        // make sure the alarm occurs in the future
        if(alarmTime.QuadPart < currentTime.QuadPart) {
			DEBUGMSG(FALSE, (_T("OEMSetAlarmTime: alarm 0x%08x:%08x occurs before 0x%08x:%08x\r\n"),
				alarmTime.HighPart, alarmTime.LowPart, currentTime.HighPart, currentTime.LowPart));
        } else {
			// round up to the nearest number of seconds
			deltaTime.QuadPart = alarmTime.QuadPart - currentTime.QuadPart;
			deltaTime.QuadPart += 9999999;					// msecs, usecs, 100ns
			deltaTime.QuadPart /= 10000000;				// convert to seconds
			
			// do we have enough resolution in our timer to handle the request?
			if(deltaTime.HighPart != 0) {
				DEBUGMSG(FALSE, (_T("OEMSetAlarmTime: alarm 0x%08x:%08x delta with 0x%08x:%08x (0x%08x:%08x) is too large\r\n"),
					alarmTime.HighPart, alarmTime.LowPart, currentTime.HighPart, currentTime.LowPart, deltaTime.HighPart, deltaTime.LowPart));
			} else {
				// clear interrupts, write the comparator, and enable interrupts
				pRTC->eoi = 0;			// any value clears pending interrupts
				pRTC->mr = dwRTCDR + deltaTime.LowPart;
				pRTC->cr = 1;			// enable match interrupt
				OEMInterruptEnable(SYSINTR_RTC_ALARM, NULL, 0);
				bRet = TRUE;
        	}
        }
    }

    // return TRUE if alarm set, FALSE otherwise
    return bRet;
}

#else //REAL_RTC

extern volatile LARGE_INTEGER CurTicks;

// The following control the debug output of the alarm & set functions
#define DEBUG_ALARM     0
#define DEBUG_SET       0

static volatile BOOL    gfInitRTC = TRUE;
static FILETIME         gftLastFileTime;
static LARGE_INTEGER    gliLastCurTicks;             // in ms

//
// The following are located in hal\interrupt.c
// They are used to control the alarm setting.
//
extern volatile BOOL           gfRTCAlarm;      // Is the RTC alarm enabled?
extern volatile LARGE_INTEGER  gliRTCAlarmTicks; // Date & time of the alarm.


//
// Multipling by 161111 and then right shifting by 13 is the same as 10000000 / TIMER_508KHZ_FREQ  
//
// This gives you the number of 100 nano second chunks...(nano chunks)
//
//#define TICKS_TO_NANO_CHUNKS( time )  (time) *= 161111; (time) = ((time) >> 13)
//#define TICKS_TO_NANO_CHUNKS( time )  (time)*=10*17/100;

#define TICKS_TO_NANO_CHUNKS( time )  (time)*=1302; (time)=(time)>>7


//#define TICKS_TO_NANO_CHUNKS( time )  (time)*=10*11; (time)=(time)>>6
//
// Multiply by 33 and then right shift by 24 is the same as TIMER_508KHZ_FREQ / 10000000
// This will give us TICKS
//
//#define NANO_CHUNKS_TO_TICKS( time )  (time) = ((time) << 13); (time) = (time) / 161111


static void add64_64_64(
    const LPFILETIME lpNum1, 
    const LPFILETIME lpNum2, 
          LPFILETIME lpResult
)
{
    __int64 num1, num2;

    num1  = (((__int64)lpNum1->dwHighDateTime)<<32);
    num1 += (__int64)lpNum1->dwLowDateTime;
    num2  = (((__int64)lpNum2->dwHighDateTime)<<32);
    num2 += (__int64)lpNum2->dwLowDateTime;
    num1 += num2;
    lpResult->dwHighDateTime = (DWORD)(num1 >> 32);
    lpResult->dwLowDateTime  = (DWORD)(num1 & 0xffffffff);
}

// The following function initializes the RT area in an attempt to 
// eliminate the run time dependency of the OS on the CMOS. The X86
// CMOS is very slow and can take up to 1ms to get the full CMOS clock.
// This function get the start time from the CMOS and initializes the
// local variables that the RT functions need.
//
static void initRTC( void )
{
    SYSTEMTIME curSysTime;
    FILETIME   curFTime;

    RETAILMSG(FALSE,(TEXT("\r\n\n******Called initRTC\r\n\n")));

    // This is changed here so that we don't try to
    // initialize the RTC at the same time that it
    // is already being done.
    //
    gfInitRTC = FALSE;

    //
    // Provide a default time...
    memset( &curSysTime, 0, sizeof( curSysTime ) );
    curSysTime.wMonth  = 6;
    curSysTime.wDay    = 1;
    curSysTime.wYear   = 1999;
    curSysTime.wHour   = 12;
    curSysTime.wMinute = 0;
    curSysTime.wSecond = 0;

    // Convert the file time structure
    //
    if( !KSystemTimeToFileTime( &curSysTime, &curFTime ) )
    {
        memset( &curSysTime, 0, sizeof( curSysTime ) );
        memset( &curFTime, 0, sizeof( curFTime ) );
    }

    // Update current versions of the RTC support
    //
    gliLastCurTicks.QuadPart = CurTicks.QuadPart;
    gftLastFileTime.dwLowDateTime  = curFTime.dwLowDateTime;
    gftLastFileTime.dwHighDateTime = curFTime.dwHighDateTime;
}


BOOL OEMGetRealTime(LPSYSTEMTIME lpst)
{
    //
    // NOTE:  this function is assumed to return a LOCAL time rather than a UTC time!
    //
    unsigned __int64 ui64Delta;
    BOOL             bResult = FALSE;
    LARGE_INTEGER    liTimeDelta;
    FILETIME         ftDelta;
    FILETIME         ftCurFTime;
    LARGE_INTEGER    tmpCurTicks;

    if (gfInitRTC) 
    {
        // Setup the software RTC...
        // gliLastCurTicks & gftLastFileTime is set in 
        // the following call.
        initRTC();
    }

    // get a snapshot of the current tick count...
    tmpCurTicks.QuadPart = CurTicks.QuadPart;

    // Calculate the current diference
    //
    if( tmpCurTicks.QuadPart >= gliLastCurTicks.QuadPart )
    {
        liTimeDelta.QuadPart = 
                tmpCurTicks.QuadPart - gliLastCurTicks.QuadPart;
    }
    else
    {
        // The counter wrapped...
        LARGE_INTEGER liMaxLargeInt;
        
        liMaxLargeInt.HighPart = 0xFFFFFFFF;
        liMaxLargeInt.LowPart  = 0xFFFFFFFF;

        liTimeDelta.QuadPart = 
            tmpCurTicks.QuadPart + 
            (liMaxLargeInt.QuadPart - gliLastCurTicks.QuadPart);
    }

    ui64Delta = (unsigned __int64) liTimeDelta.QuadPart;
    TICKS_TO_NANO_CHUNKS( ui64Delta ); // convert to nano chunks

    // setup to add in the nano chunk time difference
    ftDelta.dwLowDateTime  = (DWORD)  ui64Delta;
    ftDelta.dwHighDateTime = (DWORD) (ui64Delta >> 32);

    // Add the delta to the last known time...
    //
    add64_64_64(  &gftLastFileTime , &ftDelta, &ftCurFTime );

    // convert the answer to a system time format
    //
    if(KFileTimeToSystemTime( &ftCurFTime, lpst ) )
    {
        //
        // update the RTC variables...
        //
        gftLastFileTime.dwLowDateTime  = ftCurFTime.dwLowDateTime;
        gftLastFileTime.dwHighDateTime = ftCurFTime.dwHighDateTime;
        gliLastCurTicks.QuadPart = tmpCurTicks.QuadPart;
        bResult = TRUE;
    }
    else
    {
        // Failed, don't change anything!!!!
        //
        bResult = FALSE;
    }
    return( bResult );
}

BOOL OEMSetRealTime(LPSYSTEMTIME lpst)
{
    //
    // NOTE:  this function is passed a LOCAL time rather than a UTC time!
    //
    BOOL        bResult;
    SYSTEMTIME  curTime;

    if (gfInitRTC) 
    {
        // Setup the software RTC...
        initRTC();
    }

    KFileTimeToSystemTime( &gftLastFileTime, &curTime );

    // if the last file time is updated, then update
    // the last current tick value...
    if( KSystemTimeToFileTime( lpst, &gftLastFileTime ) )
    {
        SYSTEMTIME  newTime;

        // remember when we last updated the time...
        gliLastCurTicks.QuadPart = CurTicks.QuadPart;

        KFileTimeToSystemTime( &gftLastFileTime, &newTime );

        DEBUGMSG(DEBUG_SET,  (_T("OEMSetRealTime: gliLastCurTicks: %d\r\n"),
                 gliLastCurTicks.QuadPart ));
        DEBUGMSG(DEBUG_SET,  (_T("OEMSetRealTime: cur: ST(%02d/%02d/%04d  %02d:%02d:%02d)\r\n"),
                 curTime.wMonth, curTime.wDay,    curTime.wYear,
                 curTime.wHour,  curTime.wMinute, curTime.wSecond ));
        DEBUGMSG(DEBUG_SET,  (_T("OEMSetRealTime: In: ST(%02d/%02d/%04d  %02d:%02d:%02d)\r\n"),
                 lpst->wMonth, lpst->wDay,    lpst->wYear,
                 lpst->wHour,  lpst->wMinute, lpst->wSecond ));
        DEBUGMSG(DEBUG_SET,  (_T("OEMSetRealTime: ft: ST(%02d/%02d/%04d  %02d:%02d:%02d)\r\n"),
                 newTime.wMonth, newTime.wDay,    newTime.wYear,
                 newTime.wHour,  newTime.wMinute, newTime.wSecond ));

        bResult = TRUE;
    }
    return( bResult );
}

BOOL OEMSetAlarmTime(LPSYSTEMTIME lpst)
{
    BOOL             bResult = TRUE;
    SYSTEMTIME       stCurTime;
    FILETIME         ftAlarmTime;
    ULARGE_INTEGER   uliCurFTime;
    ULARGE_INTEGER   uliAlarmFTime;

    // The following call will make sure that
    // the RTC is properly initialized...
    // This will return the LOCAL time.
    // A side-effect that is used here is that
    // gftLastFileTime is set to the current time...
    // gliLastCurTicks is also modified...
    OEMGetRealTime( &stCurTime );

    DEBUGMSG(DEBUG_ALARM, (_T("OEMSetAlarmTime: CurTime: ST(%02d/%02d/%04d  %02d:%02d:%02d)\r\n"),
             stCurTime.wMonth, stCurTime.wDay,    stCurTime.wYear,
             stCurTime.wHour,  stCurTime.wMinute, stCurTime.wSecond ));
    DEBUGMSG(DEBUG_ALARM, (_T("OEMSetAlarmTime:   Alarm: ST(%02d/%02d/%04d  %02d:%02d:%02d)\r\n"),
             lpst->wMonth, lpst->wDay,    lpst->wYear,
             lpst->wHour,  lpst->wMinute, lpst->wSecond ));
    DEBUGMSG(DEBUG_ALARM, (_T("OEMSetAlarmTime:   gliLastCurTicks(0x%08X:0x%08X)\r\n"),
             gliLastCurTicks.HighPart, gliLastCurTicks.LowPart ));

    uliCurFTime.LowPart  = gftLastFileTime.dwLowDateTime;
    uliCurFTime.HighPart = gftLastFileTime.dwHighDateTime;

    // Convert the SYSTEMTIME alarmTime to FILETIME
    KSystemTimeToFileTime( lpst, &ftAlarmTime );
    // now to ULARGE_INTEGER
    uliAlarmFTime.LowPart  = ftAlarmTime.dwLowDateTime;
    uliAlarmFTime.HighPart = ftAlarmTime.dwHighDateTime;

    DEBUGMSG(DEBUG_ALARM, (_T("OEMSetAlarmTime:   uliCurFTime(0x%08X:0x%08X) uliAlarmFTime(0x%08X:0x%08X)\r\n"),
             uliCurFTime.HighPart,   uliCurFTime.LowPart,
             uliAlarmFTime.HighPart, uliAlarmFTime.LowPart ));

    // Compare the current time with the alarm time
    // to make sure that the alarm is in the future.
    if (uliAlarmFTime.QuadPart > uliCurFTime.QuadPart)
    {
#if (1 == DEBUG_ALARM)
        unsigned __int64 ui64RealTime;

        // The alarm time is in the future, so
        // we now need to figure out the TICK count
        // to look for.
        // Find the difference in NANO chunks...
        uliAlarmFTime.QuadPart -= uliCurFTime.QuadPart;
        // now we need to convert NANO chunks to TICKS
        ui64RealTime  = (unsigned __int64) uliAlarmFTime.HighPart << 32;
        ui64RealTime += uliAlarmFTime.LowPart;
        NANO_CHUNKS_TO_TICKS( ui64RealTime );

        // Add the current ticks back to get desired the alarm ticks...
        ui64RealTime += (unsigned __int64) gliLastCurTicks.QuadPart;

        // Save it back into uliAlarmFTime (now in TICKS)
        uliAlarmFTime.HighPart  = (DWORD) (ui64RealTime >> 32);
        uliAlarmFTime.LowPart   = (DWORD) ui64RealTime;

        DEBUGMSG(TRUE, (_T("OEMSetAlarmTime:   uliAlarmFTime(0x%08X:0x%08X)\r\n"),
                 uliAlarmFTime.HighPart, uliAlarmFTime.LowPart ));
#endif  // ( DEBUG_ALARM )

        gliRTCAlarmTicks.QuadPart = uliAlarmFTime.QuadPart;

#if (1 == DEBUG_ALARM )
    // Test the alarm date & time to see if it is close to what
    // was wanted...
{        
    LARGE_INTEGER    tmpCurTicks = gliRTCAlarmTicks;
    LARGE_INTEGER    liTimeDelta;
    FILETIME         ftDelta;
    FILETIME         ftCurFTime;
    SYSTEMTIME       stTime;
    unsigned __int64 ui64Delta;

    // Calculate the current diference
    //
    liTimeDelta.QuadPart = 
        tmpCurTicks.QuadPart - gliLastCurTicks.QuadPart;

    ui64Delta = (unsigned __int64) liTimeDelta.QuadPart;
    TICKS_TO_NANO_CHUNKS( ui64Delta ); // convert to nano chunks

    // setup to add in the nano chunk time difference
    ftDelta.dwLowDateTime  = (DWORD)  ui64Delta;
    ftDelta.dwHighDateTime = (DWORD) (ui64Delta >> 32);

    // Add the delta to the last known time...
    //
    add64_64_64(  &gftLastFileTime , &ftDelta, &ftCurFTime );

    // convert the answer to a system time format
    //
    KFileTimeToSystemTime( &ftCurFTime, &stTime );

    DEBUGMSG( DEBUG_ALARM, (_T("OEMSetAlarmTime: Converted alarmTime: ST(%02d/%02d/%04d  %02d:%02d:%02d)\r\n"),
              stTime.wMonth, stTime.wDay,    stTime.wYear,
              stTime.wHour,  stTime.wMinute, stTime.wSecond ) );
    // Write out to the debug serial port that we
    // are enabling the alarm...
    //OEMWriteDebugByte('S');
}
#endif  //  ( DEBUG_ALARM )

        gfRTCAlarm = TRUE;

        // clear interrupts, write the comparator, and enable interrupts
        OEMInterruptEnable( SYSINTR_RTC_ALARM, NULL, 0 );
        bResult = TRUE;
    }
    else
    {
        DEBUGMSG( TRUE, 
                  (_T("OEMSetAlarmTime: alarm occurs before current time!\r\n") ) );
        bResult = FALSE;
    }
    return( bResult );
}

#endif  //EDB9315A

/* EOF timex20t.c */
