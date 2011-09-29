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
#include <omap2420.h>
#include <bsp.h>

//------------------------------------------------------------------------------
// Defines 

#define BCD2BIN(b)          (((b) >> 4)*10 + ((b)&0xF))
#define BIN2BCD(b)          ((((UINT8)(b)/10) << 4)|((UINT8)(b)%10))

// Time period wherein consecutive calls to OEMGetRealTime() will return 
// a local calculated value instead of fetching the actual RTC from the RTC
#define RTC_MAX_PERIOD		60000   // 1 minutes in 1-millisecond units (1000*60)
#define RTC_MAX_YEAR        2099    // Maximum year limited by hardware
#define RTC_MIN_YEAR        2000    // Minimum year limited by hardware

//
// RTC_CTRL Offset - 0x21 bit description
//
#define RTC_EN                          0x1 //RTC Enable bit
#define RTC_AL_EN                       0x2 //Alarm Enable bit
#define MODE12_N24                      0x4 //12HR = 1, 24HR = 0
#define RTC_UPDATE_ALL                  0x8 //Update everything

#define RTCALM_MSK	                    0x2 // RTC alarm mask bit

#define RTC_TIMEREG_OFFSET              MENELAUS_RTCSEC_OFFSET    //offset 0x23
#define RTC_ALARMREG_OFFSET             MENELAUS_RTCALSEC_OFFSET  //offset 0x2A
#define RTC_TIMEREG_SIZE                7  //0x23-29
#define RTC_ALARMREG_SIZE               6  //0x2A-2F

// RTC Data Structure
struct MENELAUS_RTC_REGS {

  UINT8 ADDR;
  UINT8 RTC_SEC;      //offset 0x23
  UINT8 RTC_MIN;      //offset 0x24
  UINT8 RTC_HR;       //offset 0x25
  UINT8 RTC_DAY;      //offset 0x26
  UINT8 RTC_MON;      //offset 0x27
  UINT8 RTC_YR;       //offset 0x28
  UINT8 RTC_WKDAY;    //offset 0x29

};

struct MENELAUS_RTC_REGS RTCDATA;

//----------------------------------------------------------------------------
// External Functions
//
BOOL WriteRTCCtrlData ( UCHAR reg, UCHAR data );
BOOL ReadRTCCtrlData  ( UCHAR reg, UCHAR *pData );
BOOL WriteRTCTimeData ( PVOID pBuffer, DWORD size  );
BOOL ReadRTCTimeData  ( PVOID pBuffer, DWORD size );
BOOL WriteRTCAlarmData( PVOID pBuffer, DWORD size);
UINT32 OEMGetTickCount();

//------------------------------------------------------------------------------
// Local Variables 
//
static CRITICAL_SECTION g_CritSecRTC;       // Critical section for device access.
static BOOL g_RTCBUSInitialized = FALSE;
static BOOL g_fetchRTCViaI2c = TRUE;        // flags when OEMGetRealTime needs to read hardware RTC value through i2c bus
static UINT32 g_lastRTCTickCount = 0;		// System Tick count when FetchRTCViaI2c() was last called
static SYSTEMTIME g_lastRTCTime;            // Last hardware RTC value fetched via i2c bus
static const FILETIME g_rtcUpperBound = { 0x6302b800, 0x05f825a0 }; // This is maximal real time supported by RTC code (currently 2964/01/17 
                                                                    // 14:20:00), but it can be almost any time supported by FILETIME structure.

//------------------------------------------------------------------------------
// Local Functions
//
BOOL InitializeRTC();

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
    SYSTEMTIME *pRTCTime = &g_lastRTCTime;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"+OALIoCtlHalInitRTC(...)\r\n"));

    // Validate inputs
    if (pInpBuffer == NULL || inpSize < sizeof(SYSTEMTIME)) {
        OALMSG(OAL_ERROR, (L"ERROR:OALIoCtlHalInitRTC: INVALID PARAMETER\r\n"));
        goto cleanUp;
    }
    
    if (!g_RTCBUSInitialized)
    {
        // Init the critical section
        InitializeCriticalSection(&g_CritSecRTC);

        EnterCriticalSection(&g_CritSecRTC);
        InitializeRTC();
        LeaveCriticalSection(&g_CritSecRTC);

        g_RTCBUSInitialized = TRUE;
        OALMSG(OAL_RTC&&OAL_FUNC, (L"RTC Enabled\r\n")); 
    }

    // The H4 Platform has a backup battery for the RTC. OEM requirements for the implementation
    // of OALIoCtlHalInitRTC, one has to determine whether the current clock time is valid, and if so, ignore the time provided by the kernel.
    // Otherwise, set the real-time clock to match the provided time.
    // The RTC, when reset, would have the date 1/1/2004.
    g_fetchRTCViaI2c = TRUE;
    rc = OEMGetRealTime(pRTCTime);
    if( ((pRTCTime->wYear==2004) && (pRTCTime->wMonth==1) && (pRTCTime->wDay==1))
        || (pRTCTime->wYear > RTC_MAX_YEAR) 
        || (pRTCTime->wYear < RTC_MIN_YEAR) )
    {
        // Set time
        memcpy(pRTCTime, pTime, sizeof(SYSTEMTIME));
        rc = OEMSetRealTime(pTime);
    }

cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OALIoCtlHalInitRTC(rc = %d), g_lastRTCTickCount = %d\r\n", rc, g_lastRTCTickCount));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalRtcAlarm
//
//  This function is called by Triton driver when alarm interrupt occurs.
//
BOOL
OALIoCtlHalRtcAlarm(
    UINT32 code, 
    VOID *pInBuffer, 
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    UINT8 MaskData = RTCALM_MSK;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"+OALIoCtlHalRtcAlarm\r\n"));

    EnterCriticalSection(&g_CritSecRTC);
    

    // clear status bit
    ReadRTCCtrlData(MENELAUS_INTACK2_OFFSET, &MaskData);
    WriteRTCCtrlData(MENELAUS_INTACK2_OFFSET, MaskData | RTCALM_MSK);
    
    // mask the RTC ALARM bit in the interrupt mask
    ReadRTCCtrlData(MENELAUS_INTMASK2_OFFSET, &MaskData);
    WriteRTCCtrlData(MENELAUS_INTMASK2_OFFSET, MaskData | RTCALM_MSK);

    // Disable alarm interrupt
    ReadRTCCtrlData(MENELAUS_RTCCTRL_OFFSET, &MaskData);
    WriteRTCCtrlData (MENELAUS_RTCCTRL_OFFSET, MaskData & (~RTC_AL_EN));
    
    LeaveCriticalSection(&g_CritSecRTC);

    NKSetInterruptEvent(SYSINTR_RTC_ALARM);

    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OALIoCtlHalRtcAlarm(rc = 1)\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMGetRealTime
//
//  This function is called by the kernel to retrieve the time from
//  the real-time clock (RTC).
//
//  Since we don't want to burden the I2C bus. We calculate the current
//  time/date based on tick count arithmetic compared to the last fetch.
//
BOOL OEMGetRealTime(SYSTEMTIME *pTime) 
{
    ULONGLONG delta, time;
    static ULONGLONG lastRetTime = 0;	//last returned file time
    UINT32 i=0;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"+OEMGetRealTime\r\n"));

    EnterCriticalSection(&g_CritSecRTC);
    
    delta = OEMGetTickCount() - g_lastRTCTickCount; 
    if (delta >= RTC_MAX_PERIOD) g_fetchRTCViaI2c = TRUE;

    do {
        // Fetch the RTC from the RTC in the Menalaus
	    if (g_fetchRTCViaI2c) { 
	        RTCDATA.ADDR = RTC_TIMEREG_OFFSET;      // Set the RTC Start Address

	        // Read RTC time registers
	        if (ReadRTCTimeData( &RTCDATA, RTC_TIMEREG_SIZE )) {
	            //Convert the time to Binary format
                g_lastRTCTime.wMilliseconds = 0;
                g_lastRTCTime.wDayOfWeek    = BCD2BIN(RTCDATA.RTC_WKDAY);
                g_lastRTCTime.wYear         = BCD2BIN(RTCDATA.RTC_YR) + RTC_MIN_YEAR;
                g_lastRTCTime.wMonth        = BCD2BIN(RTCDATA.RTC_MON);
                g_lastRTCTime.wDay          = BCD2BIN(RTCDATA.RTC_DAY);
                g_lastRTCTime.wHour         = BCD2BIN(RTCDATA.RTC_HR);
                g_lastRTCTime.wMinute       = BCD2BIN(RTCDATA.RTC_MIN);
                g_lastRTCTime.wSecond       = BCD2BIN(RTCDATA.RTC_SEC);

                // Update the tick count value for the latest RTC fetch
                g_lastRTCTickCount = OEMGetTickCount();
                delta = 0;
            }
	    }

        NKSystemTimeToFileTime(&g_lastRTCTime, (FILETIME*)&time);
        time += (delta/1000) * 10000000;
        i++;

    // The while loop makes sure the rtc value is not going backward.
    // If the RTC really wants to go backward, let it go, don't loop for ever.
    } while ((time < lastRetTime) && (i<3));

    g_fetchRTCViaI2c = FALSE;

    lastRetTime = time;

    NKFileTimeToSystemTime((FILETIME*)&time, pTime);

    LeaveCriticalSection(&g_CritSecRTC);

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"-OEMGetRealTime(rc = 1, %d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

    return TRUE;
}
     
//------------------------------------------------------------------------------
//
//  Function:     OEMSetRealTime
//
//  This function is called by the kernel to set the real-time clock.
//
BOOL OEMSetRealTime(SYSTEMTIME *pTime) 
{
    UINT8 CtrlData = 0;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"OEMSetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

    // Validate time range
    if ((pTime->wYear > RTC_MAX_YEAR) || (pTime->wYear < RTC_MIN_YEAR)) return FALSE;

    // Convert from SYSTEMTIME structure to BCD
    RTCDATA.ADDR      = RTC_TIMEREG_OFFSET; 
    RTCDATA.RTC_SEC   = BIN2BCD(pTime->wSecond);  
    RTCDATA.RTC_MIN   = BIN2BCD(pTime->wMinute);  
    RTCDATA.RTC_HR    = BIN2BCD(pTime->wHour);  
    RTCDATA.RTC_DAY   = BIN2BCD(pTime->wDay);
    RTCDATA.RTC_MON   = BIN2BCD(pTime->wMonth);
    RTCDATA.RTC_YR    = BIN2BCD(pTime->wYear - RTC_MIN_YEAR);
    RTCDATA.RTC_WKDAY = BIN2BCD(pTime->wDayOfWeek);

    EnterCriticalSection(&g_CritSecRTC);
    
    // Enable RTC chip
    ReadRTCCtrlData (MENELAUS_RTCCTRL_OFFSET, &CtrlData);
    
    CtrlData |= RTC_EN;

    WriteRTCCtrlData(MENELAUS_RTCCTRL_OFFSET, CtrlData);

    // Write RTC time into the RTC device using I2C bus
    WriteRTCTimeData(&RTCDATA, RTC_TIMEREG_SIZE);
    
    // Set the RTC_UPDATE bit to force the new RTC values into RTC
    WriteRTCCtrlData( MENELAUS_RTCUPDATE_OFFSET, RTC_UPDATE_ALL);

    // wait for the RTC update completed
    CtrlData = RTC_UPDATE_ALL;
    while (CtrlData & RTC_UPDATE_ALL)
    {
        ReadRTCCtrlData(MENELAUS_RTCUPDATE_OFFSET, &CtrlData);
    }

    // Fetch real RTC vaule next time OEMGetRealtime() is called
    g_fetchRTCViaI2c = TRUE;

    LeaveCriticalSection(&g_CritSecRTC);

    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetRealTime(rc = 1)\r\n"));
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
    UINT8 CtrlData =0;
    UINT8 MaskData =0;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n", 
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

    // Validate time range
    if ((pTime->wYear > RTC_MAX_YEAR) || (pTime->wYear < RTC_MIN_YEAR)) return FALSE;

    if (!g_RTCBUSInitialized)
    {
        // Init the critical section
        InitializeCriticalSection(&g_CritSecRTC);
        EnterCriticalSection(&g_CritSecRTC);
        InitializeRTC();
        LeaveCriticalSection(&g_CritSecRTC);
        g_RTCBUSInitialized = TRUE;
    }

    // Convert from SYSTEMTIME structure to BCD
    RTCDATA.ADDR      = RTC_ALARMREG_OFFSET; 
    RTCDATA.RTC_SEC   = BIN2BCD(pTime->wSecond);  
    RTCDATA.RTC_MIN   = BIN2BCD(pTime->wMinute);  
    RTCDATA.RTC_HR    = BIN2BCD(pTime->wHour);  
    RTCDATA.RTC_DAY   = BIN2BCD(pTime->wDay);
    RTCDATA.RTC_MON   = BIN2BCD(pTime->wMonth);
    RTCDATA.RTC_YR    = BIN2BCD(pTime->wYear - RTC_MIN_YEAR);

    EnterCriticalSection(&g_CritSecRTC);
    
    ReadRTCCtrlData  (MENELAUS_RTCCTRL_OFFSET, &CtrlData);

    // Write RTC time into the RTC device using I2C bus
    WriteRTCTimeData(&RTCDATA, RTC_ALARMREG_SIZE);

	// MT - play safe, clear status bit
    ReadRTCCtrlData(MENELAUS_INTACK2_OFFSET, &MaskData);
    WriteRTCCtrlData(MENELAUS_INTACK2_OFFSET, MaskData & RTCALM_MSK);
    
	// MT - Unmask the RTC ALARM bit in the interrupt mask
    ReadRTCCtrlData(MENELAUS_INTMASK2_OFFSET, &MaskData);
    WriteRTCCtrlData(MENELAUS_INTMASK2_OFFSET, MaskData & (~RTCALM_MSK));

    // Re-enable alarm interrupt
    CtrlData |= RTC_AL_EN; 
    WriteRTCCtrlData ( MENELAUS_RTCCTRL_OFFSET, CtrlData);
    
    LeaveCriticalSection(&g_CritSecRTC);

    // Re-enable interrupt (it is disabled since last alarm occurs)
    OEMInterruptDone(SYSINTR_RTC_ALARM);

    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetAlarmTime(rc = 1)\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------
// Function : InitializeRTC
//
// This function initializes the RTC hardware

BOOL InitializeRTC()
{
   UINT8 CtrlData =0;

   //Initialize RTC
   //Settings: RTC Enabled
   ReadRTCCtrlData  (MENELAUS_RTCCTRL_OFFSET, &CtrlData);
   CtrlData |= RTC_EN;
   CtrlData &= ~MODE12_N24;
   WriteRTCCtrlData (MENELAUS_RTCCTRL_OFFSET, CtrlData);

   // Turn on charging to backup battery
   ReadRTCCtrlData  (MENELAUS_BBSMS_OFFSET, &CtrlData);
   CtrlData |= 0x5;
   WriteRTCCtrlData (MENELAUS_BBSMS_OFFSET, CtrlData);

   return (TRUE);
}   

