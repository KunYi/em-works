//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:  gptpdd.c
//
// Implementation of General Purpose Timer Driver
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"
#include "gpt_priv.h"


//-----------------------------------------------------------------------------
// External Functions
extern UINT32 BSPGptCalculateCompareVal(PCSP_GPT_STRUCT pController, UINT32 period);
extern UINT32 BSPGptGetClockSource(PCSP_GPT_STRUCT pController);
extern BOOL BSPGptSetClockGatingMode(PCSP_GPT_STRUCT pController, BOOL startClocks);
extern void BSPGptChangeClockSource(PCSP_GPT_STRUCT pController, PGPT_TIMER_SRC_PKT pSetSrcPkt);
extern void BSPGptShowClockSource(PCSP_GPT_STRUCT pController);
extern DWORD BSPGptGetBaseAddress(PCSP_GPT_STRUCT pController);
extern DWORD BSPGptGetIRQ(PCSP_GPT_STRUCT pController);


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
#define GPT_PRESCALER_VAL       0   // No prescaling
#define THREAD_PRIORITY         99


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions
static void GptStatus(PCSP_GPT_STRUCT pController);
static DWORD GptIntrThread(PCSP_GPT_STRUCT pController);
static DWORD GptISRLoop(PCSP_GPT_STRUCT pController, UINT32);
void GptRegInit(PCSP_GPT_STRUCT pController);
void GptRelease(PCSP_GPT_STRUCT pController);
void GptDisableTimer(PCSP_GPT_STRUCT pController);
void GptDisableTimerInterrupt(PCSP_GPT_STRUCT pController);
void GptRegChangeClkSrc(PCSP_GPT_STRUCT pController);

//-----------------------------------------------------------------------------
//
// Function: GptInitialize
//
// This function will call the PMU function to enable the timer source
// clock and create the timer irq event and create the IST thread.
//
// Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//          active key for the stream interface driver.
//
// Returns:
//      TRUE - If success
//
//      FALSE - If failure
//
//-----------------------------------------------------------------------------
DWORD GptInitialize(LPCTSTR pContext)
{   
    LONG    regError;
    HKEY    hKey;
    DWORD   dwDataSize;
    PHYSICAL_ADDRESS phyAddr;
    PCSP_GPT_STRUCT pController;
    
    GPT_FUNCTION_ENTRY();

    pController = (PCSP_GPT_STRUCT)LocalAlloc(LPTR, sizeof(CSP_GPT_STRUCT));
    if(!pController)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: LocalAlloc failed!\r\n"), __WFUNCTION__));
        goto Error;
    }

    // try to open active device registry key for this context
    hKey = OpenDeviceKey(pContext);
    if (hKey == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: OpenDeviceKey failed!\r\n"), __WFUNCTION__));
        goto Error;
    }

    // try to load eCSPI index from registry data
    dwDataSize = sizeof(DWORD);
    regError = RegQueryValueEx(
        hKey,                       // handle to currently open key
        L"Index",                   // string containing value to query
        NULL,                       // reserved, set to NULL
        NULL,                       // type not required, set to NULL
        (LPBYTE)(&(pController->dwGptIndex)),      // pointer to buffer receiving value
        &dwDataSize);               // pointer to buffer size

    // close handle to open key
    RegCloseKey(hKey);

    // check for errors during RegQueryValueEx
    if (regError != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: RegQueryValueEx failed!\r\n"), __WFUNCTION__));
        goto Error;
    }
    
    phyAddr.QuadPart = BSPGptGetBaseAddress(pController);
    if(!phyAddr.QuadPart)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: BSPGptGetBaseAddress() failed\r\n"), __WFUNCTION__));
        goto Error;
    }
    
    pController->pGPT = (PCSP_GPT_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_GPT_REGS), FALSE);

    // check if Map Virtual Address failed
    if (pController->pGPT == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s:  MmMapIoSpace failed!\r\n"), __WFUNCTION__));
        goto Error;
    }

    DEBUGMSG(ZONE_INFO, (TEXT("%s: CreateEvent\r\n"), __WFUNCTION__));
    pController->hGptIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pController->hGptIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed\r\n"), __WFUNCTION__));
        goto Error;
    }

    pController->dwGptIrq = BSPGptGetIRQ(pController);
    if(!pController->dwGptIrq)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: BSPGptGetIRQ() failed\r\n"), __WFUNCTION__));
        goto Error;
    }
  
    // Translate IRQ to SYSINTR
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &(pController->dwGptIrq), sizeof(pController->dwGptIrq),
        &(pController->dwGptIntr), sizeof(pController->dwGptIntr), NULL))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Request SYSINTR failed\r\n"), __WFUNCTION__));
        goto Error;
    }
    
    if(!InterruptInitialize(pController->dwGptIntr, pController->hGptIntrEvent, NULL, 0))
    {
        CloseHandle(pController->hGptIntrEvent);
        pController->hGptIntrEvent = NULL;
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Interrupt initialization failed! \r\n"), __WFUNCTION__));
        goto Error;
    }

    // Initialize critical section for GPT
    InitializeCriticalSection(&pController->hGptLock);

    if (NULL == (pController->hTimerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GptIntrThread, pController, 0, NULL)))
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        return 0;
    }
    else
    {
        DEBUGMSG(ZONE_INFO, (TEXT("%s: create timer thread success\r\n"), __WFUNCTION__));

        // Set our interrupt thread's priority
        CeSetThreadPriority(pController->hTimerThread, THREAD_PRIORITY);
    }

    // Set initial state for GPT registers
    GptRegInit(pController);

    GptStatus(pController);
    
    // Set initial state for GPT ClkSrc and currentlyOwned
    pController->ClkSrc = GPT_IPGCLK;
    pController->bCurrentlyOwned = FALSE;

    GPT_FUNCTION_EXIT();
    return (DWORD)pController;

Error:
    GptRelease(pController);

    return 0;
}


//------------------------------------------------------------------------------
//
// Function: GptRelease
//
// This function will release all resources and terminate the IST thread.
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptRelease(PCSP_GPT_STRUCT pController)
{
    GPT_FUNCTION_ENTRY();

    if (pController->pGPT)
    {
        GptDisableTimer(pController);
        GptDisableTimerInterrupt(pController);
        
        MmUnmapIoSpace(pController->pGPT,sizeof(CSP_GPT_REGS));
    }
    
    // Also set the clock gating mode to minimize power consumption when the
    // GPT is no longer being used.
    BSPGptSetClockGatingMode(pController,FALSE);

    if (pController->bCurrentlyOwned)
    {
        pController->bCurrentlyOwned = FALSE;
    }
    
    // Release SYSINTR
    if ((pController->dwGptIntr != SYSINTR_UNDEFINED)&& pController->dwGptIrq)
    {
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &(pController->dwGptIntr), sizeof(DWORD), NULL, 0, NULL);
        InterruptDisable(pController->dwGptIntr);
        pController->dwGptIntr = (DWORD) SYSINTR_UNDEFINED;
    }

    if (pController->hGptIntrEvent)
    {
        CloseHandle(pController->hGptIntrEvent);
        pController->hGptIntrEvent = NULL;
    }

    if (pController->hTimerEvent)
    {
        CloseHandle(pController->hTimerEvent);
        pController->hTimerEvent = NULL;
    }

    if (pController->hTimerThread)
    {
        CloseHandle(pController->hTimerThread);
        pController->hTimerThread = NULL;
    }

    // Delete GPT critical section
    DeleteCriticalSection(&pController->hGptLock);

    // Free the CSP_GPT_STRUCT structure.
    LocalFree(pController);


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
//      pController
//          [in] Pointer to GPT structure
//      eventString
//          [in] GPT event name
//
// Returns:
//      TRUE if success
//      FALSE if handle creation fails or if event
//      hasn't already been created by caller
//
//------------------------------------------------------------------------------
BOOL GptTimerCreateEvent(PCSP_GPT_STRUCT pController, LPTSTR eventString)
{
    GPT_FUNCTION_ENTRY();

    pController->hTimerEvent = CreateEvent(NULL, FALSE, FALSE, eventString);
    if ((pController->hTimerEvent == NULL) || (GetLastError() != ERROR_ALREADY_EXISTS))
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: CreateEvent failed or event does not already exist\r\n"), __WFUNCTION__));
        return FALSE;
    }

    pController->TimerEventString = eventString;

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
//      pController
//          [in] Pointer to GPT structure
//      eventString
//          [in] Event name
//
// Returns:
//      TRUE if success
//      FALSE if string name does not match
//
//------------------------------------------------------------------------------
BOOL GptTimerReleaseEvent(PCSP_GPT_STRUCT pController, LPTSTR eventString)
{
    GPT_FUNCTION_ENTRY();

    if (strcmp((const char *) pController->TimerEventString, (const char *) eventString) != 0)
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Cannot close GPT timer event handle: Wrong event string name.\r\n"), __WFUNCTION__));
        return FALSE;
    }
    
    CloseHandle(pController->hTimerEvent);
    
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
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      TRUE always.
//
//------------------------------------------------------------------------------
BOOL GptEnableTimer(PCSP_GPT_STRUCT pController)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&pController->hGptLock);

    INSREG32BF(&pController->pGPT->CR, GPT_CR_EN, GPT_CR_EN_ENABLE);
    INSREG32BF(&pController->pGPT->CR, GPT_CR_WAITEN, GPT_CR_WAITEN_ENABLE);

    LeaveCriticalSection(&pController->hGptLock);

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
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptDisableTimer(PCSP_GPT_STRUCT pController)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&pController->hGptLock);

    INSREG32BF(&pController->pGPT->CR, GPT_CR_EN, GPT_CR_EN_DISABLE);
    INSREG32BF(&pController->pGPT->CR, GPT_CR_WAITEN, GPT_CR_WAITEN_DISABLE);

    LeaveCriticalSection(&pController->hGptLock);
    
    GPT_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: GptEnableTimerInterrupt
//
// This function is used to enable the timer interrupt.
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptEnableTimerInterrupt(PCSP_GPT_STRUCT pController)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&pController->hGptLock);

    INSREG32BF(&pController->pGPT->IR, GPT_IR_OF1IE, GPT_IR_OF1IE_INT_ENABLE);

    LeaveCriticalSection(&pController->hGptLock);

    GPT_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: GptDisableTimerInterrupt
//
// This function is used to disable the timer interrupt.
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptDisableTimerInterrupt(PCSP_GPT_STRUCT pController)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&pController->hGptLock);

    OUTREG32(&pController->pGPT->IR,
        (CSP_BITFVAL(GPT_IR_OF1IE, GPT_IR_OF1IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_OF2IE, GPT_IR_OF2IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_OF3IE, GPT_IR_OF3IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_IF1IE, GPT_IR_IF1IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_IF2IE, GPT_IR_IF2IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_ROVIE, GPT_IR_ROVIE_INT_DISABLE)));

    LeaveCriticalSection(&pController->hGptLock);

    GPT_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: GptChangeClkSrc
//
// This function is used to change the timer clock source
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//      pSetSrcPkt 
//          [in] Packet containing clock source info.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptChangeClkSrc(PCSP_GPT_STRUCT pController, PGPT_TIMER_SRC_PKT pSetSrcPkt)
{
    GPT_FUNCTION_ENTRY();
    EnterCriticalSection(&pController->hGptLock);

    GptDisableTimer(pController);
    BSPGptChangeClockSource(pController, pSetSrcPkt);
    GptRegChangeClkSrc(pController);

    LeaveCriticalSection(&pController->hGptLock);
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
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptShowClkSrc(PCSP_GPT_STRUCT pController)
{
    GPT_FUNCTION_ENTRY();
    EnterCriticalSection(&pController->hGptLock);

    BSPGptShowClockSource(pController);

    LeaveCriticalSection(&pController->hGptLock);
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
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptClearInterruptStatus(PCSP_GPT_STRUCT pController)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&pController->hGptLock);

    // Clear Interrupt Status Bits
    INSREG32BF(&pController->pGPT->SR, GPT_SR_OF1, GPT_SR_OF1_STATUS_CLEAR);

    LeaveCriticalSection(&pController->hGptLock);

    // Kernel call to unmask the interrupt so that it can be signalled again
    InterruptDone(pController->dwGptIntr);

    GPT_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: GptRegInit
//
// Set GPT registers to initial state. 
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptRegInit(PCSP_GPT_STRUCT pController)
{
    UINT32 clkSrc;

    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&pController->hGptLock);

    // Must configure the clock gating mode before trying to access the GPT
    // control registers. Otherwise, we will hang here when the driver is
    // loaded during system boot.
    BSPGptSetClockGatingMode(pController,TRUE);
    
    // Disable GPT and clear all configuration bits
    OUTREG32(&pController->pGPT->CR, 0);

    // Assert software reset for the timer
    INSREG32BF(&pController->pGPT->CR, GPT_CR_SWR, GPT_CR_SWR_RESET);

    // Wait for the software reset to complete
    while (EXTREG32(&pController->pGPT->CR, CSP_BITFMASK(GPT_CR_SWR), GPT_CR_SWR_LSH));

    // Get BSP-specific clock source
    clkSrc = BSPGptGetClockSource(pController);

    // Initialize GPT in freerun mode, with capture and compare turned off
    OUTREG32 (&pController->pGPT->CR,
        (CSP_BITFVAL(GPT_CR_EN, GPT_CR_EN_DISABLE) |            //GPT is disabled
        CSP_BITFVAL(GPT_CR_ENMOD, GPT_CR_ENMOD_RETAIN) |        //GPT counter retain its value when it is disabled
        CSP_BITFVAL(GPT_CR_DBGEN, GPT_CR_DBGEN_DISABLE) |       //GPT is disabled in debug mode
        CSP_BITFVAL(GPT_CR_WAITEN, GPT_CR_WAITEN_DISABLE) |     //GPT is disabled in wait mode 
        CSP_BITFVAL(GPT_CR_STOPEN, GPT_CR_STOPEN_DISABLE) |     //GPT is disabled in stop mode
        CSP_BITFVAL(GPT_CR_CLKSRC, clkSrc) |                    //GPT CLKIN :  000(no_clk) 001(ipg_clk) 010(ipg_clk_highfreq) 011(external clock) 1XX(ipg_clk_32k)
        CSP_BITFVAL(GPT_CR_FRR, GPT_CR_FRR_FREERUN) |           //Freerun mode
        CSP_BITFVAL(GPT_CR_SWR, GPT_CR_SWR_NORESET) |           //GPT is not in reset mode
        CSP_BITFVAL(GPT_CR_IM1, GPT_CR_IM1_DISABLE) |           //Channel 1 input capture is disabled
        CSP_BITFVAL(GPT_CR_IM2, GPT_CR_IM2_DISABLE) |           //Channel 2 input capture is disabled
        CSP_BITFVAL(GPT_CR_OM1, GPT_CR_OM1_DISABLE) |           //Channel 1 output campare is no response
        CSP_BITFVAL(GPT_CR_OM2, GPT_CR_OM2_DISABLE) |           //Channel 2 output campare is no response
        CSP_BITFVAL(GPT_CR_OM3, GPT_CR_OM3_DISABLE) |           //Channel 3 output campare is no response
        CSP_BITFVAL(GPT_CR_FO1, GPT_CR_FO1_NOFORCE) |           //Channel 1 is not forced output compare
        CSP_BITFVAL(GPT_CR_FO2, GPT_CR_FO2_NOFORCE) |           //Channel 3 is not forced output compare
        CSP_BITFVAL(GPT_CR_FO3, GPT_CR_FO3_NOFORCE)));          //Channel 3 is not forced output compare


    // Initialize GPT prescaler value
    OUTREG32(&pController->pGPT->PR, GPT_PRESCALER_VAL);

    // Disable GPT interrupts
    OUTREG32(&pController->pGPT->IR,
        (CSP_BITFVAL(GPT_IR_OF1IE, GPT_IR_OF1IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_OF2IE, GPT_IR_OF2IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_OF3IE, GPT_IR_OF3IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_IF1IE, GPT_IR_IF1IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_IF2IE, GPT_IR_IF2IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_ROVIE, GPT_IR_ROVIE_INT_DISABLE)));

    // Clear timer compare interrupt flag (write-1-clear)
    OUTREG32(&pController->pGPT->SR, 
        (CSP_BITFVAL(GPT_SR_OF1, GPT_SR_OF1_STATUS_CLEAR) |
         CSP_BITFVAL(GPT_SR_OF2, GPT_SR_OF2_STATUS_CLEAR) |
         CSP_BITFVAL(GPT_SR_OF3, GPT_SR_OF3_STATUS_CLEAR) |
         CSP_BITFVAL(GPT_SR_IF1, GPT_SR_IF1_STATUS_CLEAR) |
         CSP_BITFVAL(GPT_SR_IF2, GPT_SR_IF1_STATUS_CLEAR) |
         CSP_BITFVAL(GPT_SR_ROV, GPT_SR_ROV_STATUS_CLEAR)));

    // Done with init, disable GPT clocks
    BSPGptSetClockGatingMode(pController,FALSE);
    
    LeaveCriticalSection(&pController->hGptLock);

    GPT_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: GptRegChangeClkSrc
//
// Set GPT registers to change clock src. 
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void GptRegChangeClkSrc(PCSP_GPT_STRUCT pController)
{
    UINT32 clkSrc;

    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&pController->hGptLock);
    
    // Disable GPT and clear all configuration bits
    OUTREG32(&pController->pGPT->CR, 0);

    // Get BSP-specific clock source
    clkSrc = BSPGptGetClockSource(pController);

    INSREG32BF(&pController->pGPT->CR, GPT_CR_CLKSRC, clkSrc);

    LeaveCriticalSection(&pController->hGptLock);

    GPT_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: GptIntrThread
//
// This function is the IST thread.
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
DWORD GptIntrThread(PCSP_GPT_STRUCT pController)
{
    GPT_FUNCTION_ENTRY();
    
    return (GptISRLoop(pController, INFINITE));
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
//      pController
//          [in] Pointer to GPT structure
//      timeout
//          [in] Timeout value while waiting for GPT timer interrupt.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
static DWORD GptISRLoop(PCSP_GPT_STRUCT pController, UINT32 timeout)
{
    DWORD rc = TRUE;

    GPT_FUNCTION_ENTRY();

    // loop here
    for (;;)
    {
        DEBUGMSG (ZONE_INFO, (TEXT("%s: In the loop\r\n"), __WFUNCTION__));
        if ((rc = WaitForSingleObject(pController->hGptIntrEvent, timeout)) == WAIT_OBJECT_0)
        {
            DEBUGMSG (ZONE_INFO, (TEXT("%s: Interrupt received\r\n"), __WFUNCTION__));

            if (pController->hTimerEvent)
            {
                // Trigger timer event
                SetEvent(pController->hTimerEvent);            
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
        GptClearInterruptStatus(pController);
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
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
static void GptStatus(PCSP_GPT_STRUCT pController)
{
#if DEBUG
    DEBUGMSG (ZONE_INFO, (TEXT("ctrl: %x  prescaler: %x  compare: %x status: %x cnt: %x\r\n"),
                    INREG32(&pController->pGPT->CR), 
                    INREG32(&pController->pGPT->PR), 
                    INREG32(&pController->pGPT->OCR1), 
                    INREG32(&pController->pGPT->SR), 
                    INREG32(&pController->pGPT->CNT)));
#else
    UNREFERENCED_PARAMETER(pController);
#endif
}


//------------------------------------------------------------------------------
//
// Function: GptResetTimer
//
// This function is used to reset GPT module
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
BOOL GptResetTimer(PCSP_GPT_STRUCT pController)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&pController->hGptLock);

    // Assert software reset for the timer
    INSREG32BF(&pController->pGPT->CR, GPT_CR_SWR, GPT_CR_SWR_RESET);

    // Wait for the software reset to complete
    while (EXTREG32(&pController->pGPT->CR, CSP_BITFMASK(GPT_CR_SWR), GPT_CR_SWR_LSH));

    LeaveCriticalSection(&pController->hGptLock);

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
//      pController
//          [in] Pointer to GPT structure
//      pGptCntValue
//          [out] 32 bit value of GPT main counter value
//
// Returns:
//      TRUE - Returns true always.
//
//
//------------------------------------------------------------------------------
BOOL GptGetTimerCount(PCSP_GPT_STRUCT pController, PDWORD pGptCntValue)
{
    GPT_FUNCTION_ENTRY();   
    // Read 32 -bit GPT Main Counter value
    *pGptCntValue = INREG32(&pController->pGPT->CNT);    
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
//      pController
//          [in] Pointer to GPT structure
//      Period
//          [in] This parameter is the period in microseconds.
//
// Returns:
//      TRUE - returns TRUE always.
//
//
//------------------------------------------------------------------------------
BOOL GptUpdateTimerPeriod(PCSP_GPT_STRUCT pController, DWORD Period)
{
    DWORD  TimerCounterVal = 0x00;
    BOOL   FreerunMode;
    
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&pController->hGptLock);    

    // In Freerun mode, current counter value should be added
    FreerunMode = ((INREG32(&pController->pGPT->CR) >> GPT_CR_FRR_LSH) & 0x01);
    if (FreerunMode)
    {
        // get the rest couter
        GptGetTimerCount(pController, &TimerCounterVal);
    }
    
    // Value in compare register set to the desired period
    // (in micro seconds), multiplied by the number of ticks per micro second
    OUTREG32(&pController->pGPT->OCR1, ((TimerCounterVal)+ BSPGptCalculateCompareVal(pController,Period)));
            
    DEBUGMSG (ZONE_INFO, (TEXT("%s: period: 0x%x, compare val: 0x%x\r\n"), 
                          __WFUNCTION__, Period, BSPGptCalculateCompareVal(pController,Period)));

    LeaveCriticalSection(&pController->hGptLock);

    GptStatus(pController);

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
//      pController
//          [in] Pointer to GPT structure
//      TimerMode
//          [in] This parametor is used to set the timer mode.
//          Set to timerModePeriodic for periodic mode, and
//          timerModeFreeRunning for free running mode.
// Returns:
//      TRUE - Returns TRUE always.
//
//------------------------------------------------------------------------------
BOOL GptSetTimerMode(PCSP_GPT_STRUCT pController, timerMode_c TimerMode)
{
    GPT_FUNCTION_ENTRY();

    EnterCriticalSection(&pController->hGptLock);   
    
    // If requested, set mode to Free Running
    if (TimerMode == timerModeFreeRunning)
    {
        INSREG32BF(&pController->pGPT->CR, GPT_CR_FRR, GPT_CR_FRR_FREERUN);
    }
    // If requested, set mode to Periodic
    else if(TimerMode == timerModePeriodic)
    {
        INSREG32BF(&pController->pGPT->CR, GPT_CR_FRR, GPT_CR_FRR_RESTART);
    }

    LeaveCriticalSection(&pController->hGptLock);

    GptStatus(pController);

    GPT_FUNCTION_EXIT();
    
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GPTCheckCLKSource
//
// This function is used to check if the GPT clock source has been set
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      TRUE - Returns TRUE if clock source has been set.
//      FALSE - Returns FALSE if clock source has not been set.
//------------------------------------------------------------------------------
BOOL GPTCheckCLKSource(PCSP_GPT_STRUCT pController)
{
     if(EXTREG32BF(&pController->pGPT->CR, GPT_CR_CLKSRC)){
        return TRUE;

     }else{
        return FALSE;
     }
}
