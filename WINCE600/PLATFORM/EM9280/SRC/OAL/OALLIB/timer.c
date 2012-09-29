//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  Module: timer.c
//
//  This module provides the BSP-specific interfaces required to support
//  the PQOAL timer code.
//
//-----------------------------------------------------------------------------

#include <bsp.h>
#include "dvfs.h"

//------------------------------------------------------------------------------
// External Functions
extern void OALCPUEnterWFI(void);

//------------------------------------------------------------------------------
// External Variables
extern PDDK_CLK_CONFIG g_pDdkClkConfig;
extern PVOID pv_HWregICOLL;
extern PVOID pv_HWregDIGCTL;
extern PVOID pv_HWregCLKCTRL;
extern PVOID pv_HWregRTC;
//------------------------------------------------------------------------------
// Defines
#if	(defined EM9280 || defined EM9283)
#define WD_RESET_PERIOD          10000						// tell the wdog to reset the system after 10 seconds.
#define WD_REFRESH_PERIOD        (WD_RESET_PERIOD / 2)		// tell the OS to refresh watchdog every 5 second.
#else	// -> iMX28EVK
#define WD_REFRESH_PERIOD        3000						// tell the OS to refresh watchdog every 3 second.
#define WD_RESET_PERIOD          4500						// tell the wdog to reset the system after 4.5 seconds.
#endif	//EM9280 | EM9283
// CS&ZHL SEP-19-2012: actual boot time < 20s
#define	MAX_BOOTING_TIME		60000						// max booting time means from nk booting to wstartup end

//------------------------------------------------------------------------------
// Local Variables
// CS&ZHL SEP-18-2012: flag for app is startup
BOOL bWstartupIsEnd = FALSE;	


//-----------------------------------------------------------------------------
//
//  Function: OALTimerNotifyReschedule
//
//  This function is called when the new thread is ready to run.  The BSP
//  uses this kernel function to perform CPU load tracking.
//
//  Parameters:
//      dwThrdId 
//          [in] Identifier of the next running thread.
//
//      dwPrio 
//          [in] Priority of the next running thread.
//
//      dwQuantum 
//          [in] Quantum of the next running thread.
//
//      dwFlags 
//          [in] Reserved for future use.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID OALTimerNotifyReschedule(DWORD dwThrdId, DWORD dwPrio, DWORD dwQuantum, DWORD dwFlags)
{
    DWORD idleTime, idlePercent, timeWindow;
    static DWORD lastMSec = 0, lastIdleTime = 0;
    static BOOL bWindowRestart = TRUE;
    static UINT32 upMSec = 0, dnMSec = 0;
    static DDK_DVFC_SETPOINT lastSetpointCpu;
    DDK_DVFC_SETPOINT setpointCpu;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwThrdId);
    UNREFERENCED_PARAMETER(dwPrio);
    UNREFERENCED_PARAMETER(dwQuantum);
    UNREFERENCED_PARAMETER(dwFlags);

    // Set restart flag if DVFC is inactive
    if (!g_pDdkClkConfig->bDvfcActive)
    {
        bWindowRestart = TRUE;
        return;
    }

    setpointCpu = g_pDdkClkConfig->setpointCur[DDK_DVFC_DOMAIN_CPU];
    if (setpointCpu != lastSetpointCpu)
    {   
        // If setpoint has changed since the last reschedule, base 
        // load setpoint on current setpoint
        lastSetpointCpu = setpointCpu;
        g_pDdkClkConfig->setpointLoad[DDK_DVFC_DOMAIN_CPU] = setpointCpu;              
    }        

    if (bWindowRestart)
    {
        // Restart load tracking window parameters
        bWindowRestart = FALSE;
        lastMSec = CurMSec;
        upMSec = 0;
        dnMSec = 0;
        lastIdleTime = (DWORD)(g_pNKGlobal->liIdle.QuadPart/g_pNKGlobal->dwIdleConv);
        return;
    }

    // Calculate load tracking duration
    timeWindow = CurMSec - lastMSec;
        
    // Check if tracking duration exceeds minimum tracking window
    if (timeWindow > BSP_DVFS_LOADTRACK_MSEC)
    {
        // Calculate idle percentage of the tracking window
        idleTime = (DWORD)(g_pNKGlobal->liIdle.QuadPart/g_pNKGlobal->dwIdleConv);
        idlePercent = (idleTime - lastIdleTime) * 100 / timeWindow;

        // Restart tracking window
        lastMSec = CurMSec;
        lastIdleTime = idleTime;
        
        // Check for setpoint increase
        if (idlePercent < BSP_DVFS_LOADTRACK_UP_PCT)
        {
            dnMSec = 0;
            upMSec += timeWindow;

            if (upMSec >= BSP_DVFS_LOADTRACK_UP_MSEC)
            {
                // Avoid setpoint increase request if current setpoint is already 
                // highest
                if (setpointCpu != g_pDdkClkConfig->setpointMax[DDK_DVFC_DOMAIN_CPU])
                {
                    g_pDdkClkConfig->setpointLoad[DDK_DVFC_DOMAIN_CPU] = setpointCpu - 1;
                    //OALMSG(TRUE, (_T("DOM0 = UP:  %d, %d\r\n"), idlePercent, upMSec));
                    HW_ICOLL_INTERRUPTn_SET(IRQ_DVFC, BM_ICOLL_INTERRUPTn_SOFTIRQ);
                }
                upMSec = 0;                
            }
        }
        // Check for setpoint decrease
        else if (idlePercent > BSP_DVFS_LOADTRACK_DN_PCT)
        {
            upMSec = 0;
            dnMSec += timeWindow;
            
            if (dnMSec >= BSP_DVFS_LOADTRACK_DN_MSEC)
            {
                  // Avoid setpoint decrease request if current setpoint is already 
                // lowest or if setpoint is being held by CSPDDK/Power Manager.
                if ((setpointCpu != g_pDdkClkConfig->setpointMin[DDK_DVFC_DOMAIN_CPU]) &&
                    (setpointCpu == g_pDdkClkConfig->setpointLoad[DDK_DVFC_DOMAIN_CPU]))                  
                {
                    g_pDdkClkConfig->setpointLoad[DDK_DVFC_DOMAIN_CPU] = setpointCpu + 1;
                    //OALMSGS(TRUE, (_T("DOM0 = DOWN:  %d, %d\r\n"), idlePercent, dnMSec));

                    HW_ICOLL_INTERRUPTn_SET(IRQ_DVFC, BM_ICOLL_INTERRUPTn_SOFTIRQ);                    
                }
                dnMSec = 0;            
            }
        }
        else
        {
            upMSec = 0;
            dnMSec = 0;
        }
    }
} 


//-----------------------------------------------------------------------------
//
//  Function: OALCPUIdle
//
//  This function puts the CPU or SOC in idle state. The CPU or SOC
//  should exit the idle state when an interrupt occurs. This function is
//  called with interrupts are disabled. When this function returns, interrupts
//  must be disabled also.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID OALCPUIdle()
{
    // Enter wait-for-interrupt mode
    OALCPUEnterWFI();
}
//-----------------------------------------------------------------------------
//
// Function: WdogInit
//
//  This function asserts a system reset after the desired timeout.
//
// Parameters:
//      timeoutMSec
//          [in] watchdog timeout in msec.
//
// Returns:
//      This function returns the actual timeout period(msec).
//
//-----------------------------------------------------------------------------
VOID WdogInit(UINT32 TimeoutMSec)
{
    // Get uncached virtual addresses for Watchdog
    if (pv_HWregRTC == NULL) 
    {
        OALMSG(OAL_ERROR, (L"WdogInit:  Watchdog null pointer!\r\n"));
        return;
    }

    // Watchdog is configured as follows:
    HW_RTC_WATCHDOG_WR(TimeoutMSec);
    
#if (defined EM9280 || defined EM9283)
	{
		// CS&ZHL JUN-12-2012: always enable WDT in EM9280
		HW_RTC_CTRL_SET(BM_RTC_CTRL_WATCHDOGEN);
	}
#else	// -> iMX28EVK
	{
		// Enable the watchdog, BUT ONLY IF KITL IS NOT ENABLED.
#ifdef IMGNOKITL
		HW_RTC_CTRL_SET(BM_RTC_CTRL_WATCHDOGEN);
#endif
	}
#endif	//EM9280 | EM9283

    return;
}

//-----------------------------------------------------------------------------
//
// Function: ResetChip
//
//  This function asserts a system reset.
//
// Parameters:
//
// Returns:
//
//-----------------------------------------------------------------------------
VOID ResetChip()
{
    // MAP the Hardware registers
    if(pv_HWregCLKCTRL == NULL)
    {
        pv_HWregCLKCTRL = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_CLKCTRL, FALSE);
    }
    
    if (!pv_HWregCLKCTRL)
    {
        OALMSG(OAL_ERROR, (L"ERROR: InitRTC: pv_HWregRTC null pointer!\r\n"));
        goto cleanUp;
    }
    
    // ChipReset
    HW_CLKCTRL_RESET_SET(BM_CLKCTRL_RESET_CHIP);
cleanUp:
    return;
}

//------------------------------------------------------------------------------
//
//  Function:  RefreshWatchdogTimer
//
//  This is the function to refresh the watchdog timer.
//------------------------------------------------------------------------------
void RefreshWatchdogTimer (void)
{
    static BOOL bFirstTime = TRUE;

    //OALMSG(1, (L"+RefreshWatchdogTimer\r\n"));

    if (bFirstTime)
    {
        OALMSG(OAL_FUNC, (L"+RefreshWatchdogTimer: First call, init the Wdog to timeout reset in 4.5 secs\r\n"));
        //OALMSG(1, (L"+RefreshWatchdogTimer: First call, init the Wdog to timeout reset in 4.5 secs\r\n"));
        WdogInit(WD_RESET_PERIOD);
        bFirstTime = FALSE;
		bWstartupIsEnd = FALSE;
    }
    else
    {
        OALMSG(OAL_FUNC, (L"+RefreshWatchdogTimer: Subsequence calls, refresh the Wdog timeout to 4.5 secs again\r\n"));
        //OALMSG(1, (L"+RefreshWatchdogTimer: Subsequence calls, refresh the Wdog timeout to 4.5 secs again\r\n"));
		//
		// CS&ZHL SEP-18-2012: smart refresh WDT 
		//					   CurMSec: global variable indicates the number of milliseconds since boot.
		//
		if((CurMSec < MAX_BOOTING_TIME) || bWstartupIsEnd)			
		{
			HW_RTC_WATCHDOG_WR(WD_RESET_PERIOD);
		}
    }

    OALMSG(OAL_FUNC, (L"-RefreshWatchdogTimer\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  InitWatchDogTimer
//
//  This is the function to enable hardware watchdog timer support by kernel.
//------------------------------------------------------------------------------
void InitWatchDogTimer (void)
{
    //OALMSG(OAL_FUNC, (L"+InitWatchDogTimer\r\n"));
    OALMSG(1, (L"+InitWatchDogTimer\r\n"));

    pfnOEMRefreshWatchDog = RefreshWatchdogTimer;
    dwOEMWatchDogPeriod   = WD_REFRESH_PERIOD;

    OALMSG(OAL_FUNC, (L"-InitWatchDogTimer\r\n"));
}

//------------------------------------------------------------------------------
