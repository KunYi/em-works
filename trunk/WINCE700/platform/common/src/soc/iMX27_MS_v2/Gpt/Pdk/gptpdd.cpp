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
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  gptpdd.c
//
//  Implementation of General Purpose Timer Driver
//
//-----------------------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>

#include "csp.h"
#include "gpt.h"
#include "gpt_priv.h"


//-----------------------------------------------------------------------------
// External Functions
extern UINT32 BSPGptGetClockSource();
extern UINT32 BSPGptGetPERCLK1();
extern UINT32 BSPGptGet32kHzRefFreq();
extern BOOL BSPGptSetClockGatingMode(DWORD, BOOL);


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
#define GPT_PRESCALER_VAL   0  // No prescaling
#define THREAD_PRIORITY                  250


//-----------------------------------------------------------------------------
//
// Function: GptInitialize
//
// This function will call the PMU function to enable the timer source
// clock and create the timer irq event and create the IST thread.
//
// Parameters:
//      dwIOBase
//          [in] Physical IOBase address of the GPT device
//      
//      dwIOLen
//           [in] Physical IOLen of the GPT device
//      
//      dwIRQ
//           [in] Physical IRQ number of the GPT device
//
// Returns:
//      TRUE - If success
//
//      FALSE - If failure
//
//-----------------------------------------------------------------------------
BOOL gptClass::GptInitialize(DWORD dwIOBase, DWORD dwIOLen, DWORD dwIRQ)
{
    PHYSICAL_ADDRESS phyAddr;
    DWORD gptIrq = dwIRQ;

    GPT_FUNCTION_ENTRY();

    phyAddr.QuadPart = dwIOBase;

    m_pGPT = (PCSP_GPT_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_GPT_REGS), FALSE);

    // check if Map Virtual Address failed
    if (m_pGPT == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s:  MmMapIoSpace failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_dwIOBase = dwIOBase;

    DEBUGMSG(ZONE_INFO, (TEXT("%s: CreateEvent\r\n"), __WFUNCTION__));
    m_hGptIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hGptIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Translate IRQ to SYSINTR
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &gptIrq, sizeof(gptIrq),
        &m_gptIntr, sizeof(m_gptIntr), NULL))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Request SYSINTR failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    if(!InterruptInitialize(m_gptIntr, m_hGptIntrEvent, NULL, 0))
    {
        CloseHandle(m_hGptIntrEvent);
        m_hGptIntrEvent = NULL;
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Interrupt initialization failed! \r\n"), __WFUNCTION__));
        goto Error;
    }

    // Initialize critical section for GPT
    InitializeCriticalSection(&m_GptCS);

    if (!(m_hTimerThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GptIntrThread, this, 0, NULL)))
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        return 0;
    }
    else
    {
        DEBUGMSG(ZONE_INFO, (TEXT("%s: create timer thread success\r\n"), __WFUNCTION__));

        // Set our interrupt thread's priority
        CeSetThreadPriority(m_hTimerThread, THREAD_PRIORITY);
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
void gptClass::GptRelease()
{
    GPT_FUNCTION_ENTRY();

    GptStopTimer();
    GptDisableTimerInterrupt();

    // Also set the clock gating mode to minimize power consumption when the
    // GPT is no longer being used.
    BSPGptSetClockGatingMode(m_dwIOBase, FALSE);

    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_gptIntr, sizeof(DWORD), NULL, 0, NULL);
    m_gptIntr = SYSINTR_UNDEFINED;

    // deregister the system interrupt
    if (m_gptIntr != SYSINTR_UNDEFINED)
    {
        InterruptDisable(m_gptIntr);
        m_gptIntr = SYSINTR_UNDEFINED;
    }

    if (m_hGptIntrEvent)
    {
        CloseHandle(m_hGptIntrEvent);
        m_hGptIntrEvent = NULL;
    }

    if (m_hTimerEvent)
    {
        CloseHandle(m_hTimerEvent);
        m_hTimerEvent = NULL;
    }

    if (m_hTimerThread)
    {
        CloseHandle(m_hTimerThread);
        m_hTimerThread = NULL;
    }

    // Delete GPT critical section
    DeleteCriticalSection(&m_GptCS);

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
BOOL gptClass::GptTimerCreateEvent(LPTSTR eventString)
{
    GPT_FUNCTION_ENTRY();

    m_hTimerEvent = CreateEvent(NULL, FALSE, FALSE, eventString);
    if ((m_hTimerEvent == NULL) || (GetLastError() != ERROR_ALREADY_EXISTS))
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: CreateEvent failed or event does not already exist\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_TimerEventString = eventString;

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
BOOL gptClass::GptTimerReleaseEvent(LPTSTR eventString)
{
    GPT_FUNCTION_ENTRY();

    if (strcmp((const char *) m_TimerEventString, (const char *) eventString) != 0)
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Cannot close GPT timer event handle: Wrong event string name.\r\n"), __WFUNCTION__));
        return FALSE;
    }
    
    CloseHandle(m_hTimerEvent);
    
    GPT_FUNCTION_EXIT();    
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GptStartTimer
//
// This function is used to start the timer. It will set the enable 
// bit in the GPT control register. This function should be called to
// enable the timer after configuring the timer.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success.  FALSE if timer has not been set.
//
//------------------------------------------------------------------------------
BOOL gptClass::GptStartTimer(void)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&m_GptCS);

    if (INREG32(&m_pGPT->TCMP) == 0xFFFFFFFF)
    {
        DEBUGMSG (ZONE_ERROR, (TEXT("%s: Timer period has not been set.  Cannot start timer.\r\n"), __WFUNCTION__));
        LeaveCriticalSection(&m_GptCS);
        return FALSE;
    }

    INSREG32BF(&m_pGPT->TCTL, GPT_TCTL_TEN, GPT_TCTL_TEN_ENABLE);
    INSREG32BF(&m_pGPT->TCTL, GPT_TCTL_COMPEN, GPT_TCTL_COMPEN_ENABLE);

    LeaveCriticalSection(&m_GptCS);

    GPT_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GptStopTimer
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
void gptClass::GptStopTimer(void)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&m_GptCS);

    INSREG32BF(&m_pGPT->TCTL, GPT_TCTL_TEN, GPT_TCTL_TEN_DISABLE);
    INSREG32BF(&m_pGPT->TCTL, GPT_TCTL_COMPEN, GPT_TCTL_COMPEN_DISABLE);

    LeaveCriticalSection(&m_GptCS);
    
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
void gptClass::GptEnableTimerInterrupt(void)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&m_GptCS);

    INSREG32BF(&m_pGPT->TCTL, GPT_TCTL_COMPEN, GPT_TCTL_COMPEN_ENABLE);

    LeaveCriticalSection(&m_GptCS);

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
void gptClass::GptDisableTimerInterrupt(void)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&m_GptCS);

    INSREG32BF(&m_pGPT->TCTL, GPT_TCTL_COMPEN, GPT_TCTL_COMPEN_DISABLE);

    LeaveCriticalSection(&m_GptCS);

    GPT_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: GptSetTimerDelay
//
// This function is used to configure the timer and
// set the timer for some period delay.
//
// Parameters:
//      timerMode
//          [in] This parametor is used to set the timer mode.
//          Set to timerModePeriodic for periodic mode, and
//          timerModeFreeRunning for free running mode.
//
//      period
//          [in] This parameter is the period in milliseconds.
//
// Returns:
//      TRUE - If success.
//
//      FALSE - If the timer is not allocated.
//
//------------------------------------------------------------------------------
BOOL gptClass::GptSetTimerDelay(PGPT_TIMER_SET_PKT pSetTimerPkt)
{
    UINT16 prescaler = 0;
    UINT32 freq;
    UINT32 div;
    BOOL done;

    timerMode_c timerMode = pSetTimerPkt->timerMode;
    timerPeriodType_c periodType = pSetTimerPkt->periodType;
    UINT32 period = pSetTimerPkt->period;
    

    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&m_GptCS);

    //Stop timer to set delay
    GptStopTimer();

    // Write 1 to clear
    m_pGPT->TSTAT = 1;

    done = FALSE;

    switch (periodType)
    {
	case MICROSEC:
            // Use perclk1 clock for periods in usec
            INSREG32BF(&m_pGPT->TCTL, GPT_TCTL_CLKSOURCE, GPT_TCTL_CLKSOURCE_PERCLK1);
            freq = BSPGptGetPERCLK1();
            for(prescaler = 0; prescaler <= CSP_BITFMASK(GPT_TPRER_PRESCALER); prescaler++)
            {
                if( (UINT64)((freq / (prescaler + 1) / 1000000) * period) < GPT_TCN_COUNT_MAX )
                {
                    m_pGPT->TCMP = freq / (prescaler + 1) / 1000000 * period;
                    done = TRUE;
                    break;
                }
            }
            if(done)
                break;
            // else fall thru 
        case MILLISEC:
            // Use perclk1/4 clock for periods in msec
            INSREG32BF(&m_pGPT->TCTL, GPT_TCTL_CLKSOURCE, GPT_TCTL_CLKSOURCE_PERCLK1DIV4);
            freq = BSPGptGetPERCLK1() / 4;
            if(periodType == MICROSEC) div = 1000000;
            if(periodType == MILLISEC) div = 1000;
            for(prescaler = 0; prescaler <= CSP_BITFMASK(GPT_TPRER_PRESCALER); prescaler++)
            {
                if( (UINT64)((freq / (prescaler + 1) / div) * period) < GPT_TCN_COUNT_MAX )
                {
                    m_pGPT->TCMP = freq / (prescaler + 1) / div * period;
                    done = TRUE;
                    break;
                }
            }
            if(done)
                break;
            // else fall thru 
        case SECOND:
            // Use 32k ref clock for periods in seconds
            INSREG32BF(&m_pGPT->TCTL, GPT_TCTL_CLKSOURCE, GPT_TCTL_CLKSOURCE_32KCLK);
            freq = BSPGptGet32kHzRefFreq();
            if(periodType == MICROSEC) div = 1000000;
            if(periodType == MILLISEC) div = 1000;
            if(periodType == SECOND) div = 1;
            for(prescaler = 0; prescaler <= CSP_BITFMASK(GPT_TPRER_PRESCALER); prescaler++)
            {
                if( (UINT64)((freq / (prescaler + 1) / div) * period) < GPT_TCN_COUNT_MAX )
                {
                    m_pGPT->TCMP = freq / (prescaler + 1) / div * period;
                    done = TRUE;
                    break;
                }
            }
            if(done)
                break;
            else
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("TimerClass: Unable to support requested delay!\r\n")));
                return FALSE;               
            }
            
        default:            
            DEBUGMSG(ZONE_ERROR, (TEXT("TimerClass: Invalid period type!\r\n")));
            return FALSE;               
    }

     m_pGPT->TPRER = prescaler;

   if (timerMode == timerModeFreeRunning)
        m_pGPT->TCTL |= CSP_BITFMASK(GPT_TCTL_FRR);
   else if(timerMode == timerModePeriodic)
        m_pGPT->TCTL &= ~CSP_BITFMASK(GPT_TCTL_FRR);  

    LeaveCriticalSection(&m_GptCS);

    GptStatus();

    GPT_FUNCTION_EXIT();
    
    return TRUE;
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
void gptClass::GptClearInterruptStatus(void)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&m_GptCS);

    // Clear Interrupt Status Bits
    INSREG32BF(&m_pGPT->TSTAT, GPT_TSTAT_COMP, GPT_TSTAT_COMP_RESET);

    LeaveCriticalSection(&m_GptCS);

    // Kernel call to unmask the interrupt so that it can be signalled again
    InterruptDone(m_gptIntr);
    
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
void gptClass::GptRegInit()
{
    UINT32 clkSrc;

    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&m_GptCS);

    // Must configure the clock gating mode before trying to access the GPT
    // control registers. Otherwise, we will hang here when the driver is
    // loaded during system boot.
    BSPGptSetClockGatingMode(m_dwIOBase, TRUE);
    
    // Disable GPT and clear all configuration bits
    OUTREG32(&m_pGPT->TCTL, 0);

    // Reset timer
    OUTREG32(&m_pGPT->TCTL, CSP_BITFMASK(GPT_TCTL_SWR));

    // Wait for the software reset to complete
    while (INREG32(&m_pGPT->TCTL) & CSP_BITFMASK(GPT_TCTL_SWR));

    // Get BSP-specific clock source
    clkSrc = BSPGptGetClockSource();

    m_pGPT->TPRER = 0;
    m_pGPT->TCMP = 0xFFFFFFFF;

    OUTREG32(&m_pGPT->TCTL,
        CSP_BITFVAL(GPT_TCTL_TEN, GPT_TCTL_TEN_DISABLE) |
        CSP_BITFVAL(GPT_TCTL_CLKSOURCE, clkSrc) |
        CSP_BITFVAL(GPT_TCTL_COMPEN, GPT_TCTL_COMPEN_DISABLE) |
        CSP_BITFVAL(GPT_TCTL_CAPTEN, GPT_TCTL_CAPTEN_DISABLE) |
        CSP_BITFVAL(GPT_TCTL_CAP, GPT_TCTL_CAP_DISABLE) |
        CSP_BITFVAL(GPT_TCTL_FRR, GPT_TCTL_FRR_FREERUN) |
        CSP_BITFVAL(GPT_TCTL_OM, GPT_TCTL_OM_ACTIVELOW) |
        CSP_BITFVAL(GPT_TCTL_CC, GPT_TCTL_CC_ENABLE));


    
    LeaveCriticalSection(&m_GptCS);

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
void gptClass::GptIntrThread(LPVOID lpParameter)
{
    gptClass *pGpt = (gptClass *)lpParameter;

    GPT_FUNCTION_ENTRY();
    
    pGpt->GptISRLoop(INFINITE);

    GPT_FUNCTION_ENTRY();
    
    return;
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
void gptClass::GptISRLoop(UINT32 timeout)
{
    GPT_FUNCTION_ENTRY();

    // loop here
    while(TRUE)
    {
        DEBUGMSG (ZONE_INFO, (TEXT("%s: In the loop\r\n"), __WFUNCTION__));
        if (WaitForSingleObject(m_hGptIntrEvent, timeout) == WAIT_OBJECT_0)
        {
            DEBUGMSG (ZONE_INFO, (TEXT("%s: Interrupt received\r\n"), __WFUNCTION__));

            if (m_hTimerEvent)
            {
                // Trigger timer event
                SetEvent(m_hTimerEvent);            
            }
        }
        else
        {
            // Timeout as requested
            DEBUGMSG (ZONE_INFO, (TEXT("%s: Time out\r\n"), __WFUNCTION__));
        }
        // Clear interrupt bits
        GptClearInterruptStatus();
    }

    GPT_FUNCTION_ENTRY();
    return;
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
void gptClass::GptStatus(void)
{
    DEBUGMSG (ZONE_INFO, (TEXT("ctrl: %x  prescaler: %x  compare: %x status: %x cnt: %x\r\n"),
                    INREG32(&m_pGPT->TCTL), 
                    INREG32(&m_pGPT->TPRER), 
                    INREG32(&m_pGPT->TCMP), 
                    INREG32(&m_pGPT->TSTAT), 
                    INREG32(&m_pGPT->TCN)));
}
