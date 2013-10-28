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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  gptpdd.c
//
//  Implementation of General Purpose Timer Driver
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "mxarm11.h"
#include "gpt.h"
#include "gpt_priv.h"


//-----------------------------------------------------------------------------
// External Functions
extern UINT32 BSPGptCalculateCompareVal(UINT32 period);
extern UINT32 BSPGptGetClockSource();
extern BOOL BSPGptSetClockGatingMode(BOOL);
extern void BSPGptChangeClockSource(PGPT_TIMER_SRC_PKT pSetSrcPkt);
extern void BSPGptShowClockSource(void);


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
#define GPT_PRESCALER_VAL   0  // No prescaling
#define THREAD_PRIORITY                  99


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
static PCSP_GPT_REGS g_pGPT;
static CRITICAL_SECTION g_hGptLock;
static DWORD g_gptIntr;
static HANDLE g_hGptIntrEvent = NULL;
static HANDLE g_hTimerThread = NULL;
static HANDLE g_hTimerEvent = NULL;
static LPCTSTR g_TimerEventString = NULL;

//-----------------------------------------------------------------------------
// Local Functions
static void GptStatus(void);
static DWORD GptIntrThread(PVOID);
static DWORD GptISRLoop(UINT32);


//-----------------------------------------------------------------------------
//
// Function: GptInitialize
//
// This function will call the PMU function to enable the timer source
// clock and create the timer irq event and create the IST thread.
//
// Parameters:
//      None
//
// Returns:
//      TRUE - If success
//
//      FALSE - If failure
//
//-----------------------------------------------------------------------------
BOOL GptInitialize(void)
{
    PHYSICAL_ADDRESS phyAddr;
    DWORD gptIrq = IRQ_GPT;

    GPT_FUNCTION_ENTRY();

    phyAddr.QuadPart = CSP_BASE_REG_PA_GPT;

    g_pGPT = (PCSP_GPT_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_GPT_REGS), FALSE);

    // check if Map Virtual Address failed
    if (g_pGPT == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s:  MmMapIoSpace failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    DEBUGMSG(ZONE_INFO, (TEXT("%s: CreateEvent\r\n"), __WFUNCTION__));
    g_hGptIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_hGptIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Translate IRQ to SYSINTR
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &gptIrq, sizeof(gptIrq),
        &g_gptIntr, sizeof(g_gptIntr), NULL))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Request SYSINTR failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    if(!InterruptInitialize(g_gptIntr, g_hGptIntrEvent, NULL, 0))
    {
        CloseHandle(g_hGptIntrEvent);
        g_hGptIntrEvent = NULL;
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Interrupt initialization failed! \r\n"), __WFUNCTION__));
        goto Error;
    }

    // Initialize critical section for GPT
    InitializeCriticalSection(&g_hGptLock);

#if 0 // Remove-W4: Warning C4706 workaround
    if (!(g_hTimerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GptIntrThread, NULL, 0, NULL)))
#else
    if ((g_hTimerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GptIntrThread, NULL, 0, NULL)) == 0)
#endif
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        return 0;
    }
    else
    {
        DEBUGMSG(ZONE_INFO, (TEXT("%s: create timer thread success\r\n"), __WFUNCTION__));

        // Set our interrupt thread's priority
        CeSetThreadPriority(g_hTimerThread, THREAD_PRIORITY);
    }

    // Set initial state for GPT registers
    GptRegInit();

    GptStatus();

    GPT_FUNCTION_EXIT();
    return TRUE;

Error:
    GptRelease();

    return FALSE;
}


//------------------------------------------------------------------------------
//
// Function: GptRelease
//
// This function will release all resources and terminate the IST thread.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptRelease(void)
{
    GPT_FUNCTION_ENTRY();

    GptDisableTimer();
    GptDisableTimerInterrupt();

    // Also set the clock gating mode to minimize power consumption when the
    // GPT is no longer being used.
    BSPGptSetClockGatingMode(FALSE);

    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &g_gptIntr, sizeof(DWORD), NULL, 0, NULL);
    g_gptIntr = (DWORD) SYSINTR_UNDEFINED;

    // deregister the system interrupt
    if (g_gptIntr != SYSINTR_UNDEFINED)
    {
        InterruptDisable(g_gptIntr);
        g_gptIntr = (DWORD) SYSINTR_UNDEFINED;
    }

    if (g_hGptIntrEvent)
    {
        CloseHandle(g_hGptIntrEvent);
        g_hGptIntrEvent = NULL;
    }

    if (g_hTimerEvent)
    {
        CloseHandle(g_hTimerEvent);
        g_hTimerEvent = NULL;
    }

    if (g_hTimerThread)
    {
        CloseHandle(g_hTimerThread);
        g_hTimerThread = NULL;
    }

    // Delete GPT critical section
    DeleteCriticalSection(&g_hGptLock);

    GPT_FUNCTION_EXIT();
    return;
}


//------------------------------------------------------------------------------
//
// Function: GptTimerCreateEvent
//
// This function is used to create a GPT timer event handle
// triggered in the GPT ISR.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success
//      FALSE if handle creation fails or if event
//      hasn't already been created by caller
//
//------------------------------------------------------------------------------
BOOL GptTimerCreateEvent(LPTSTR eventString)
{
    DWORD dwErr;
    GPT_FUNCTION_ENTRY();
    g_hTimerEvent = CreateEvent(NULL, FALSE, FALSE, eventString);
    dwErr = GetLastError();
    if ((g_hTimerEvent == NULL) || (dwErr != ERROR_ALREADY_EXISTS))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed or event does not already exist\r\n"), __WFUNCTION__));
        SetLastError(dwErr);
        return FALSE;
    }

    g_TimerEventString = eventString;

    GPT_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GptTimerReleaseEvent
//
// This function is used to close the handle to the
// GPT timer event.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success
//      FALSE if string name does not match
//
//------------------------------------------------------------------------------
BOOL GptTimerReleaseEvent(LPTSTR eventString)
{
    GPT_FUNCTION_ENTRY();

    if((g_TimerEventString == NULL) || (g_hTimerEvent == NULL))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Invalid timer event handle or timer event string.\r\n"), __WFUNCTION__));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (strcmp((const char *) g_TimerEventString, (const char *) eventString) != 0)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Cannot close GPT timer event handle: Wrong event string name.\r\n"), __WFUNCTION__));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    CloseHandle(g_hTimerEvent);
    GPT_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GptEnableTimer
//
// This function is used to start the timer. It will set the enable
// bit in the GPT control register. This function should be called to
// enable the timer after configuring the timer.
//
// Parameters:
//      None
//
// Returns:
//      TRUE always.
//
//------------------------------------------------------------------------------
BOOL GptEnableTimer(void)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&g_hGptLock);

    INSREG32BF(&g_pGPT->CR, GPT_CR_EN, GPT_CR_EN_ENABLE);
    INSREG32BF(&g_pGPT->CR, GPT_CR_WAITEN, GPT_CR_WAITEN_ENABLE);

    LeaveCriticalSection(&g_hGptLock);

    GPT_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GptDisableTimer
//
// This function is used to stop the timer. It will
// clear the enable bit in the GPT control register.
// After this, the counter will reset to 0x00000000.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptDisableTimer(void)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&g_hGptLock);

    INSREG32BF(&g_pGPT->CR, GPT_CR_EN, GPT_CR_EN_DISABLE);
    INSREG32BF(&g_pGPT->CR, GPT_CR_WAITEN, GPT_CR_WAITEN_DISABLE);

    LeaveCriticalSection(&g_hGptLock);

    GPT_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: GptEnableTimerInterrupt
//
// This function is used to enable the timer interrupt.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptEnableTimerInterrupt(void)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&g_hGptLock);

    INSREG32BF(&g_pGPT->IR, GPT_IR_OF1IE, GPT_IR_OF1IE_INT_ENABLE);

    LeaveCriticalSection(&g_hGptLock);

    GPT_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: GptDisableTimerInterrupt
//
// This function is used to disable the timer interrupt.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptDisableTimerInterrupt(void)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&g_hGptLock);

    INSREG32BF(&g_pGPT->IR, GPT_IR_OF1IE, GPT_IR_OF1IE_INT_DISABLE);

    LeaveCriticalSection(&g_hGptLock);

    GPT_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: GptChangeClkSrc
//
// This function is used to change the timer clock source
//
// Parameters:
//      pSetSrcPkt : packet containing clock source info.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptChangeClkSrc(PGPT_TIMER_SRC_PKT pSetSrcPkt)
{
    GPT_FUNCTION_ENTRY();
    EnterCriticalSection(&g_hGptLock);

    GptDisableTimer();
    BSPGptChangeClockSource(pSetSrcPkt);
    GptRegChangeClkSrc();

    LeaveCriticalSection(&g_hGptLock);
    GPT_FUNCTION_EXIT();
    return;
}

//------------------------------------------------------------------------------
//
// Function: GptShowClkSrc
//
// This function is used to disable the timer interrupt.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptShowClkSrc(void)
{
    GPT_FUNCTION_ENTRY();
    EnterCriticalSection(&g_hGptLock);

    BSPGptShowClockSource();

    LeaveCriticalSection(&g_hGptLock);
    GPT_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: GptClearInterruptStatus
//
// This function is used to clear the GPT interrupt status and signal to the
// kernel that interrupt processing is completed.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptClearInterruptStatus(void)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&g_hGptLock);

    // Clear Interrupt Status Bits
    INSREG32BF(&g_pGPT->SR, GPT_SR_OF1, GPT_SR_OF1_STATUS_CLEAR);

    LeaveCriticalSection(&g_hGptLock);

    // Kernel call to unmask the interrupt so that it can be signalled again
    InterruptDone(g_gptIntr);

    GPT_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: GptRegInit
//
// Set GPT registers to initial state.
//
// Parameters:
//      None.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptRegInit(void)
{
    UINT32 clkSrc;

    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&g_hGptLock);

    // Must configure the clock gating mode before trying to access the GPT
    // control registers. Otherwise, we will hang here when the driver is
    // loaded during system boot.
    BSPGptSetClockGatingMode(TRUE);

    // Disable GPT and clear all configuration bits
    OUTREG32(&g_pGPT->CR, 0);

    // Assert software reset for the timer
    INSREG32BF(&g_pGPT->CR, GPT_CR_SWR, GPT_CR_SWR_RESET);

    // Wait for the software reset to complete
    while (EXTREG32(&g_pGPT->CR, CSP_BITFMASK(GPT_CR_SWR), GPT_CR_SWR_LSH));

    // Get BSP-specific clock source
    clkSrc = BSPGptGetClockSource();

    // Initialize GPT in freerun mode, with capture and compare turned off
    OUTREG32 (&g_pGPT->CR,
        (CSP_BITFVAL(GPT_CR_EN, GPT_CR_EN_DISABLE) |
        CSP_BITFVAL(GPT_CR_ENMOD, GPT_CR_ENMOD_RETAIN) |
        CSP_BITFVAL(GPT_CR_DBGEN, GPT_CR_DBGEN_DISABLE) |
        CSP_BITFVAL(GPT_CR_WAITEN, GPT_CR_WAITEN_DISABLE) |
        CSP_BITFVAL(GPT_CR_DOZEN, GPT_CR_DOZEN_DISABLE) |
        CSP_BITFVAL(GPT_CR_STOPEN, GPT_CR_STOPEN_DISABLE) |
        CSP_BITFVAL(GPT_CR_CLKSRC, clkSrc) |
        CSP_BITFVAL(GPT_CR_FRR, GPT_CR_FRR_FREERUN) |
        CSP_BITFVAL(GPT_CR_SWR, GPT_CR_SWR_NORESET) |
        CSP_BITFVAL(GPT_CR_IM1, GPT_CR_IM1_DISABLE) |
        CSP_BITFVAL(GPT_CR_IM2, GPT_CR_IM2_DISABLE) |
        CSP_BITFVAL(GPT_CR_OM1, GPT_CR_OM1_DISABLE) |
        CSP_BITFVAL(GPT_CR_OM2, GPT_CR_OM2_DISABLE) |
        CSP_BITFVAL(GPT_CR_OM3, GPT_CR_OM3_DISABLE) |
        CSP_BITFVAL(GPT_CR_FO1, GPT_CR_FO1_NOFORCE) |
        CSP_BITFVAL(GPT_CR_FO2, GPT_CR_FO2_NOFORCE)));


    // Initialize GPT prescaler value
    OUTREG32(&g_pGPT->PR, GPT_PRESCALER_VAL);

    // Disable GPT interrupts
    OUTREG32(&g_pGPT->IR,
        (CSP_BITFVAL(GPT_IR_OF1IE, GPT_IR_OF1IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_OF2IE, GPT_IR_OF2IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_OF3IE, GPT_IR_OF3IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_IF1IE, GPT_IR_IF1IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_IF2IE, GPT_IR_IF2IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_ROVIE, GPT_IR_ROVIE_INT_DISABLE)));

    // Clear timer compare interrupt flag (write-1-clear)
    INSREG32BF(&g_pGPT->SR, GPT_SR_OF1, GPT_SR_OF1_STATUS_CLEAR);

    // Done with init, disable GPT clocks
    BSPGptSetClockGatingMode(FALSE);

    LeaveCriticalSection(&g_hGptLock);

    GPT_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: GptRegChangeClkSrc
//
// Set GPT registers to change clock src.
//
// Parameters:
//      None.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptRegChangeClkSrc(void)
{
    UINT32 clkSrc;

    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&g_hGptLock);

    // Disable GPT and clear all configuration bits
    OUTREG32(&g_pGPT->CR, 0);

    // Get BSP-specific clock source
    clkSrc = BSPGptGetClockSource();

    // Initialize GPT in freerun mode, with capture and compare turned off
    OUTREG32 (&g_pGPT->CR,
        (CSP_BITFVAL(GPT_CR_EN, GPT_CR_EN_DISABLE) |
        CSP_BITFVAL(GPT_CR_ENMOD, GPT_CR_ENMOD_RETAIN) |
        CSP_BITFVAL(GPT_CR_DBGEN, GPT_CR_DBGEN_DISABLE) |
        CSP_BITFVAL(GPT_CR_WAITEN, GPT_CR_WAITEN_DISABLE) |
        CSP_BITFVAL(GPT_CR_DOZEN, GPT_CR_DOZEN_DISABLE) |
        CSP_BITFVAL(GPT_CR_STOPEN, GPT_CR_STOPEN_DISABLE) |
        CSP_BITFVAL(GPT_CR_CLKSRC, clkSrc) |
        CSP_BITFVAL(GPT_CR_FRR, GPT_CR_FRR_FREERUN) |
        CSP_BITFVAL(GPT_CR_SWR, GPT_CR_SWR_NORESET) |
        CSP_BITFVAL(GPT_CR_IM1, GPT_CR_IM1_DISABLE) |
        CSP_BITFVAL(GPT_CR_IM2, GPT_CR_IM2_DISABLE) |
        CSP_BITFVAL(GPT_CR_OM1, GPT_CR_OM1_DISABLE) |
        CSP_BITFVAL(GPT_CR_OM2, GPT_CR_OM2_DISABLE) |
        CSP_BITFVAL(GPT_CR_OM3, GPT_CR_OM3_DISABLE) |
        CSP_BITFVAL(GPT_CR_FO1, GPT_CR_FO1_NOFORCE) |
        CSP_BITFVAL(GPT_CR_FO2, GPT_CR_FO2_NOFORCE)));

    LeaveCriticalSection(&g_hGptLock);

    GPT_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: GptIntrThread
//
// This function is the IST thread.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
DWORD GptIntrThread(
    PVOID Reserved  //@parm Reserved, not used.
    )
{
    UNREFERENCED_PARAMETER(Reserved);

    GPT_FUNCTION_ENTRY();

    return (GptISRLoop(INFINITE));
}


//------------------------------------------------------------------------------
//
// Function: GptISRLoop
//
// This function is the interrupt handler for the GPT.
// It waits for the GPT interrupt event, and signals
// the timer registered by the user of this timer.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for GPT timer interrupt.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
static DWORD GptISRLoop(UINT32 timeout)
{
    DWORD rc = TRUE;

    GPT_FUNCTION_ENTRY();

    // loop here
    for (;;)
    {
        DEBUGMSG (ZONE_INFO, (TEXT("%s: In the loop\r\n"), __WFUNCTION__));
        if ((rc = WaitForSingleObject(g_hGptIntrEvent, timeout)) == WAIT_OBJECT_0)
        {
            DEBUGMSG (ZONE_INFO, (TEXT("%s: Interrupt received\r\n"), __WFUNCTION__));

            if (g_hTimerEvent)
            {
                // Trigger timer event
                SetEvent(g_hTimerEvent);
            }
        }
        else if (rc == WAIT_TIMEOUT)
        {
            // Timeout as requested/
            DEBUGMSG (ZONE_INFO, (TEXT("%s: Time out\r\n"), __WFUNCTION__));
        }
        else
        {
            //Abnormal signal
            rc = FALSE;
            break;
        }
        // Clear interrupt bits
        GptClearInterruptStatus();
    }

#if 0 // Remove-W4: Warning C4702 workaround
    GPT_FUNCTION_ENTRY();
#endif

    return rc;
}


//------------------------------------------------------------------------------
//
// Function: GptStatus
//
// This function is used to print out the GPT register status.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
static void GptStatus(void)
{
    DEBUGMSG (ZONE_INFO, (TEXT("ctrl: %x  prescaler: %x  compare: %x status: %x cnt: %x\r\n"),
                    INREG32(&g_pGPT->CR),
                    INREG32(&g_pGPT->PR),
                    INREG32(&g_pGPT->OCR1),
                    INREG32(&g_pGPT->SR),
                    INREG32(&g_pGPT->CNT)));
}


//------------------------------------------------------------------------------
//
// Function: GptResetTimer
//
// This function is used to reset GPT module
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
BOOL GptResetTimer(void)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&g_hGptLock);

    INSREG32BF(&g_pGPT->CR, GPT_CR_SWR, CSP_BITFVAL(GPT_CR_SWR, GPT_CR_SWR_RESET));

    LeaveCriticalSection(&g_hGptLock);

    GPT_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GptGetTimerCount
//
// This function is used to obtain the GPT mian counter value.
//
// Parameters:
//      pGptCntValue
//          [out] 32 bit value of GPT main counter value
//
// Returns:
//      TRUE - Returns true always.
//
//
//------------------------------------------------------------------------------
BOOL GptGetTimerCount(PDWORD pGptCntValue)
{
    GPT_FUNCTION_ENTRY();
    // Read 32 -bit GPT Main Counter value
    *pGptCntValue = INREG32(&g_pGPT->CNT);
    GPT_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GptUpdateTimerPeriod
//
// This function is used to set the timer for specified period delay.
//
// Parameters:
//      Period
//          [in] This parameter is the period in microseconds.
//
// Returns:
//      TRUE - returns TRUE always.
//
//
//------------------------------------------------------------------------------
BOOL GptUpdateTimerPeriod(DWORD Period)
{
    DWORD  TimerCounterVal = 0x00;
    BOOL   FreerunMode;

    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&g_hGptLock);

    // In Freerun mode, current counter value should be added
    FreerunMode = ((INREG32(&g_pGPT->CR) >> GPT_CR_FRR_LSH) &  0x01);
    if (FreerunMode)
    {
        if (!GptGetTimerCount(&TimerCounterVal))
        {
            return FALSE;
        }
    }

    // Value in compare register set to the desired period
    // (in micro seconds), multiplied by the number of ticks per micro second
    OUTREG32(&g_pGPT->OCR1, ((TimerCounterVal)+ BSPGptCalculateCompareVal(Period)));

    DEBUGMSG (ZONE_INFO, (TEXT("%s: period: 0x%x, compare val: 0x%x\r\n"),
                          __WFUNCTION__, Period, BSPGptCalculateCompareVal(Period)));

    LeaveCriticalSection(&g_hGptLock);

    GptStatus();

    GPT_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GptSetTimerMode
//
// This function is used to configure operation mode of GPT.
//
// Parameters:
//      TimerMode
//          [in] This parametor is used to set the timer mode.
//          Set to timerModePeriodic for periodic mode, and
//          timerModeFreeRunning for free running mode.
// Returns:
//      TRUE - Returns TRUE always.
//
//------------------------------------------------------------------------------
BOOL GptSetTimerMode(timerMode_c TimerMode)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&g_hGptLock);

    // If requested, set mode to Free Running
    if (TimerMode == timerModeFreeRunning)
    {
        INSREG32BF(&g_pGPT->CR, GPT_CR_FRR, GPT_CR_FRR_FREERUN);
    }
    // If requested, set mode to Periodic
    else if(TimerMode == timerModePeriodic)
    {
        INSREG32BF(&g_pGPT->CR, GPT_CR_FRR, GPT_CR_FRR_RESTART);
    }

    LeaveCriticalSection(&g_hGptLock);

    GptStatus();

    GPT_FUNCTION_EXIT();

    return TRUE;
}
