//------------------------------------------------------------------------------
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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  profiler.c
//
//  This file contains implementation of profiler module suitable for the
//  Freescale AR11 family CPU/SoC with count/compare timer.  The routines
//  use match register 2 (M2) for the profiling timer.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <intr.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_gpt.h"

//------------------------------------------------------------------------------
// External Functions
extern UINT32 OALProfileGetIrq(void);
extern UINT32 OALProfileGetBaseRegAddr(void);
extern UINT32 OALProfileGetClkSrc(void);
extern UINT32 OALProfileGetClkFreq(void);
extern VOID OALProfileTimerEnable(void);
extern VOID OALProfileTimerDisable(void);

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Local Variables
static PCSP_GPT_REGS g_pGPT;
static struct
{
    BOOL enabled;                               // is profiler active?
    UINT32 countsPerHit;                        // counts per profiler interrupt
} g_profiler;

//------------------------------------------------------------------------------
// External Variables
extern PFN_PROFILER_ISR g_pProfilerISR;


//------------------------------------------------------------------------------
// Local Functions
UINT32 OALProfileIntrHandler(UINT32 ra);


//------------------------------------------------------------------------------
//
//  Function:  OEMProfileTimerEnable
//
//  This function is called by kernel to start kernel profiling timer.
//
VOID OEMProfileTimerEnable(DWORD interval)
{
    BOOL enabled;
    UINT32 irq;

    OALMSG(TRUE, (L"+OEMProfileTimerEnable(%d)\r\n", interval));

    // We can't enable timer second time
    if (g_profiler.enabled) return;

    // Obtain a pointer to the PWM registers.
    if (!g_pGPT)
    {
        g_pGPT = (PCSP_GPT_REGS) OALPAtoVA(OALProfileGetBaseRegAddr(), FALSE);
    }

    // How many hi-res ticks per profiler hit
    g_profiler.countsPerHit = ((OALProfileGetClkFreq()/1000) * interval) / 1000;

    OALMSG(TRUE, (L"OEMProfileTimerEnable: countsPerHit = %d\r\n", g_profiler.countsPerHit));

    enabled = INTERRUPTS_ENABLE(FALSE);

    // Configure profiling ISR callback function.
    g_pProfilerISR = OALProfileIntrHandler;

    // Allow platform-specific configuration
    OALProfileTimerEnable();

    // Initialize GPT in restart mode, with capture and compare turned off
    OUTREG32 (&g_pGPT->CR,
        (CSP_BITFVAL(GPT_CR_EN, GPT_CR_EN_DISABLE) |
        CSP_BITFVAL(GPT_CR_ENMOD, GPT_CR_ENMOD_RETAIN) |
        CSP_BITFVAL(GPT_CR_DBGEN, GPT_CR_DBGEN_ENABLE) |
        CSP_BITFVAL(GPT_CR_WAITEN, GPT_CR_WAITEN_ENABLE) |
        CSP_BITFVAL(GPT_CR_STOPEN, GPT_CR_STOPEN_ENABLE) |
        CSP_BITFVAL(GPT_CR_CLKSRC, OALProfileGetClkSrc()) |
        CSP_BITFVAL(GPT_CR_FRR, GPT_CR_FRR_RESTART) |
        CSP_BITFVAL(GPT_CR_SWR, GPT_CR_SWR_NORESET) |
        CSP_BITFVAL(GPT_CR_IM1, GPT_CR_IM1_DISABLE) |
        CSP_BITFVAL(GPT_CR_IM2, GPT_CR_IM2_DISABLE) |
        CSP_BITFVAL(GPT_CR_OM1, GPT_CR_OM1_DISABLE) |
        CSP_BITFVAL(GPT_CR_OM2, GPT_CR_OM2_DISABLE) |
        CSP_BITFVAL(GPT_CR_OM3, GPT_CR_OM3_DISABLE) |
        CSP_BITFVAL(GPT_CR_FO1, GPT_CR_FO1_NOFORCE) |
        CSP_BITFVAL(GPT_CR_FO2, GPT_CR_FO2_NOFORCE)));

    // Initialize GPT prescaler value
    OUTREG32(&g_pGPT->PR, 0);

    // Enable output compare 1 interrupt
    OUTREG32(&g_pGPT->IR,
        (CSP_BITFVAL(GPT_IR_OF1IE, GPT_IR_OF1IE_INT_ENABLE) |
        CSP_BITFVAL(GPT_IR_OF2IE, GPT_IR_OF2IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_OF3IE, GPT_IR_OF3IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_IF1IE, GPT_IR_IF1IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_IF2IE, GPT_IR_IF2IE_INT_DISABLE) |
        CSP_BITFVAL(GPT_IR_ROVIE, GPT_IR_ROVIE_INT_DISABLE)));

    // Clear timer compare interrupt flag (write-1-clear)
    INSREG32BF(&g_pGPT->SR, GPT_SR_OF1, GPT_SR_OF1_STATUS_CLEAR);
    
    // Update the compare register for the next profile hit.
    OUTREG32(&g_pGPT->OCR1, g_profiler.countsPerHit);

    // Start the timer
    INSREG32BF(&g_pGPT->CR, GPT_CR_EN, GPT_CR_EN_ENABLE);

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);

    irq = OALProfileGetIrq();
    OALIntrDoneIrqs(1, &irq);

    // Set flag
    g_profiler.enabled = TRUE;

    OALMSG(TRUE, (L"-OEMProfileTimerEnable\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OEMProfileTimerDisable
//
//  This function is called by kernel to stop kernel profiling timer.
//

VOID OEMProfileTimerDisable()
{
    BOOL enabled;
    UINT32 irq;

    OALMSG(TRUE, (L"+OEMProfileTimerDisable()\r\n"));

    // No disable without enable
    if (!g_profiler.enabled) goto cleanUp;

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Disable the profile timer interrupt
    irq = OALProfileGetIrq();
    OALIntrDisableIrqs(1, &irq);

    // Stop the timer
    INSREG32BF(&g_pGPT->CR, GPT_CR_EN, GPT_CR_EN_DISABLE);

    // Allow platform-specific configuration
    OALProfileTimerDisable();
    
    // Deconfigure profiling ISR callback function.
    g_pProfilerISR = NULL;

    // Reset flag
    g_profiler.enabled = FALSE;

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);

cleanUp:
    OALMSG(TRUE, (L"-OEMProfileTimerDisable\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OALProfileIntrHandler
//
//  This is timer interrupt handler which replace default handler in time when
//  kernel profiling is active. It calls original interrupt handler in
//  appropriate times.
//
UINT32 OALProfileIntrHandler(UINT32 ra)
{
    // Clear timer compare interrupt flag (write-1-clear) for the
    // next profile hit.
    INSREG32BF(&g_pGPT->SR, GPT_SR_OF1, GPT_SR_OF1_STATUS_CLEAR);
    
    // First call profiler
    ProfilerHit(ra);

    return(SYSINTR_NOP);
}

//------------------------------------------------------------------------------
