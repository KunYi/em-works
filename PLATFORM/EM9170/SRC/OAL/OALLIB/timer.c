//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
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
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, 
    DDK_CLOCK_GATE_MODE mode);


//------------------------------------------------------------------------------
// External Variables
extern PCSP_AVIC_REGS g_pAVIC;
extern PDDK_CLK_CONFIG g_pDdkClkConfig;


//------------------------------------------------------------------------------
// Defines
#define WDOG_GET_WT(msec)       ((UINT16)((msec/WDOG_TIMEOUT_RES) & WDOG_WCR_WT_MASK))
#define WDOG_TIMEOUT_RES        500    // (4096*4*1000)/32768

#define WD_REFRESH_PERIOD       5000    // tell the OS to refresh watchdog every 5 second.
#define WD_RESET_PERIOD         10000    // tell the wdog to reset the system after 10 seconds.


//------------------------------------------------------------------------------
// Local Variables
static PCSP_WDOG_REGS g_pWDOG;
static 

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
VOID OALTimerNotifyReschedule(DWORD dwThrdId, DWORD dwPrio, DWORD dwQuantum, 
                              DWORD dwFlags)
{
    DWORD idleTime, idlePercent, timeWindow;
    static DWORD lastMSec = 0, lastIdleTime = 0;
    static BOOL bWindowRestart = TRUE;
    static UINT32 upMSec = 0, dnMSec = 0;
    static DDK_DVFC_SETPOINT lastSetpointCpu;
    static DDK_DVFC_SETPOINT lastSetpointPer;
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
                    // OALMSGS(TRUE, (_T("DOM0 = UP:  %d, %d\r\n"), idlePercent, upMSec));
                    OUTREG32(&g_pAVIC->INTFRCL, (1U << IRQ_CCM));
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
                    // OALMSGS(TRUE, (_T("DOM0 = DOWN:  %d, %d\r\n"), idlePercent, dnMSec));
                    OUTREG32(&g_pAVIC->INTFRCL, (1U << IRQ_CCM));
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
//  Function: OALTimerGetClkSrc
//
//  This function returns the clock source setting used to program the EPIT_CR
//  CLKSRC bits.
//
//  Parameters:
//      None.
//
//  Returns:
//      EPIT clock source selection.
//
//-----------------------------------------------------------------------------
UINT32 OALTimerGetClkSrc(void)
{
#if (BSP_EPIT_CLKSRC == EPIT_CR_CLKSRC_CKIL)

    return EPIT_CR_CLKSRC_CKIL;

#elif (BSP_EPIT_CLKSRC == EPIT_CR_CLKSRC_HIGHFREQ)
    
    return EPIT_CR_CLKSRC_HIGHFREQ;

#elif (BSP_EPIT_CLKSRC == EPIT_CR_CLKSRC_IPGCLK)

    // Map reschedule notification kernel function to perform CPU load tracking
    g_pOemGlobal->pfnNotifyReschedule = OALTimerNotifyReschedule;

    return EPIT_CR_CLKSRC_IPGCLK;

#endif
    
}


//-----------------------------------------------------------------------------
//
//  Function: OALTimerGetClkPrescalar
//
//  This function returns the clock prescalar used to program the EPIT_CR
//  PRESCALER bits.
//
//  Parameters:
//      None.
//
//  Returns:
//      EPIT prescalar.
//
//-----------------------------------------------------------------------------
UINT32 OALTimerGetClkPrescalar(void)
{
    return BSP_EPIT_PRESCALAR;
}


//-----------------------------------------------------------------------------
//
//  Function: OALTimerGetClkFreq
//
//  This function returns the frequency of the EPIT input clock.
//
//  Parameters:
//      None.
//
//  Returns:
//      EPIT input clock.
//
//-----------------------------------------------------------------------------
UINT32 OALTimerGetClkFreq(void)
{
    BSP_ARGS *pBspArgs = (BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START;
    switch(OALTimerGetClkSrc())
    {
    case EPIT_CR_CLKSRC_CKIL:
        return BSP_CLK_CKIL_FREQ;
    case EPIT_CR_CLKSRC_IPGCLK:
        return pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_EPIT];
    case EPIT_CR_CLKSRC_HIGHFREQ:
        return pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB];
    default:
        ERRORMSG(OAL_ERROR,(TEXT("Invalid Timer source\r\n")));
        DEBUGCHK(0);
        return 0;
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
#ifdef BSP_FAKE_IDLE

    // Wait until AVIC reports an interrupt is pending
    while((!INREG32(&g_pAVIC->NIPNDH)) && (!INREG32(&g_pAVIC->NIPNDL)));

#else

    // Enter wait-for-interrupt mode
    OALCPUEnterWFI();

#endif
}


//-----------------------------------------------------------------------------
//
// Function: WdogService
//
//  This function services the watchdog timer.
//
// Parameters:
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL WdogService(void)
{
    if (g_pWDOG == NULL)
    {
        OALMSG(OAL_ERROR, (L"WdogService:  Watchdog not initialized!\r\n"));
        return FALSE;
    }
    // 1. write 0x5555
    OUTREG16(&g_pWDOG->WSR, WDOG_WSR_WSR_RELOAD1);

    // 2. write 0xAAAA
    OUTREG16(&g_pWDOG->WSR, WDOG_WSR_WSR_RELOAD2);

    return TRUE;
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
UINT32 WdogInit(UINT32 TimeoutMSec)
{
    UINT16 wcr;
    
    // Get uncached virtual addresses for Watchdog
    g_pWDOG = (PCSP_WDOG_REGS) OALPAtoUA(CSP_BASE_REG_PA_WDOG);
    if (g_pWDOG == NULL)
    {
        OALMSG(OAL_ERROR, (L"WdogInit:  Watchdog null pointer!\r\n"));
        return 0;
    }

    OALMSG(1, (L"-->WdogInit:  Watchdog\r\n"));
    // Enable watchdog clocks
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_WDOG, DDK_CLOCK_GATE_MODE_ENABLED);

    // Watchdog is configured as follows:
    //
    //  WDW = continue timer operation in low-power wait mode
    //  WDT = generate reset signal upon watchdog timeout
    //  WDA = no software assertion of WDOG output pin
    //  SRS = no software reset of WDOG    
    //  WDE = disable watchdog (will be enabled after configuration)
    //  WDBG = suspend timer operation in debug mode
    //  WDZST = suspend timer operation in low-power stop mode
    //wcr =  CSP_BITFVAL(WDOG_WCR_WDW, WDOG_WCR_WDW_SUSPEND)|
    //        CSP_BITFVAL(WDOG_WCR_WDT, WDOG_WCR_WDT_TIMEOUT_ASSERTION) |
    //        CSP_BITFVAL(WDOG_WCR_WDA, WDOG_WCR_WDA_NOEFFECT) |
    //        CSP_BITFVAL(WDOG_WCR_SRS, WDOG_WCR_SRS_NOEFFECT) |            
    //        CSP_BITFVAL(WDOG_WCR_WDE, WDOG_WCR_WDE_DISABLE) |
    //        CSP_BITFVAL(WDOG_WCR_WDBG, WDOG_WCR_WDBG_SUSPEND) |
    //        CSP_BITFVAL(WDOG_WCR_WDZST, WDOG_WCR_WDZST_SUSPEND) |
    //        CSP_BITFVAL(WDOG_WCR_WT, WDOG_GET_WT(TimeoutMSec));    
	//
	// CS&ZHL JUN-28-2011: continue WDT always
	//
    wcr =  CSP_BITFVAL(WDOG_WCR_WDW, WDOG_WCR_WDW_CONTINUE)|
            CSP_BITFVAL(WDOG_WCR_WDT, WDOG_WCR_WDT_TIMEOUT_ASSERTION) |
            CSP_BITFVAL(WDOG_WCR_WDA, WDOG_WCR_WDA_NOEFFECT) |
            CSP_BITFVAL(WDOG_WCR_SRS, WDOG_WCR_SRS_NOEFFECT) |            
            CSP_BITFVAL(WDOG_WCR_WDE, WDOG_WCR_WDE_DISABLE) |
            CSP_BITFVAL(WDOG_WCR_WDBG, WDOG_WCR_WDBG_CONTINUE) |
            CSP_BITFVAL(WDOG_WCR_WDZST, WDOG_WCR_WDZST_CONTINUE) |
            CSP_BITFVAL(WDOG_WCR_WT, WDOG_GET_WT(TimeoutMSec));    


    // Configure and then enable the watchdog
    OUTREG16(&g_pWDOG->WCR, wcr);
    wcr |= CSP_BITFVAL(WDOG_WCR_WDE, WDOG_WCR_WDE_ENABLE);
    OUTREG16(&g_pWDOG->WCR,  wcr);
    
    // Service the watchdog
    WdogService();

    return (WDOG_GET_WT(TimeoutMSec)*WDOG_TIMEOUT_RES);
}


//
// function to refresh watchdog timer
//
void RefreshWatchdogTimer (void)
{
    static BOOL bFirstTime = TRUE;

    OALMSG(OAL_FUNC, (L"+RefreshWatchdogTimer\r\n"));

    if (bFirstTime)
    {
        OALMSG(OAL_FUNC, (L"+RefreshWatchdogTimer: First call, init the Wdog to timeout reset in %d secs\r\n",WD_RESET_PERIOD/1000));
        OALMSG(1, (L"+RefreshWatchdogTimer: First call, init the Wdog to timeout reset in %d secs\r\n",WD_RESET_PERIOD/1000));
        WdogInit(WD_RESET_PERIOD);
        bFirstTime = FALSE;
    }
    else
    {
        OALMSG(OAL_FUNC, (L"+RefreshWatchdogTimer: Subsequence calls, refresh the Wdog timeout to %d secs again\r\n",WD_RESET_PERIOD/1000));
        WdogService();
        //OALMSG(1, (L"+RefreshWatchdogTimer: Refresh WDT...\r\n"));
    }

    OALMSG(OAL_FUNC, (L"-RefreshWatchdogTimer\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  InitWatchDogTimer
//
//  This is the function to enable hardware watchdog timer support by kernel.
//
void InitWatchDogTimer (void)
{
    OALMSG(OAL_FUNC, (L"+InitWatchDogTimer\r\n"));

    pfnOEMRefreshWatchDog = RefreshWatchdogTimer;
    dwOEMWatchDogPeriod   = WD_REFRESH_PERIOD;

    OALMSG(OAL_FUNC, (L"-InitWatchDogTimer\r\n"));
}


//------------------------------------------------------------------------------
