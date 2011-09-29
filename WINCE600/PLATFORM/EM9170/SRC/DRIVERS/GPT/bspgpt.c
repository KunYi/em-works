//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// File:  bspgpt.c
//
// Provides BSP-specific configuration routines for the GPT peripheral.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)


#include "csp.h"
#include "bsp_clocks.h"

#include "gpt_priv.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#define GPT_CLKSRC_FREQ_32K     BSP_CLK_CKIL_FREQ
#define TICKS_PER_S_32K         GPT_CLKSRC_FREQ_32K         // ticks per second

#define GPT_CLKSRC_VAL_NO       GPT_CR_CLKSRC_NOCLK
#define GPT_CLKSRC_VAL_IPG      GPT_CR_CLKSRC_IPGCLK        
#define GPT_CLKSRC_VAL_HIGH     GPT_CR_CLKSRC_HIGHFREQ      
#define GPT_CLKSRC_VAL_EXT      GPT_CR_CLKSRC_EXTCLK
#define GPT_CLKSRC_VAL_32K      GPT_CR_CLKSRC_CLK32K

#define ADJUST_VAL_H            1000000
#define ADJUST_VAL_L            1000000


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables

UINT32 TICKS_PER_S_IPGCLK = 0;              //be read from CM 

//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: BSPGptGetClockSource
//
// This function returns the BSP-specific clock
// source selection value for the GPT.
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      The clock source for the GPT.
//
//------------------------------------------------------------------------------
UINT32 BSPGptGetClockSource(PCSP_GPT_STRUCT pController)
{
    switch (pController->ClkSrc)
    {
    case GPT_NOCLK:
    case GPT_IPGCLK:
    case GPT_HIGHCLK:
    case GPT_EXTCLK:
        return GPT_CLKSRC_VAL_IPG;
    case GPT_32KCLK:
        return GPT_CLKSRC_VAL_32K;
    default:
        return GPT_CLKSRC_VAL_IPG;
    }
}

//------------------------------------------------------------------------------
//
// Function: BSPCalculateCompareVal
//
// This function computes the tick value that should go
// into the GPT output compare register to achieve
// a requested timer delay period.
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//      period
//          [in] The desired timer delay period, in microseconds.
//
// Returns:
//      The value to go into the GPT output compare register.
//
//------------------------------------------------------------------------------
UINT32 BSPGptCalculateCompareVal(PCSP_GPT_STRUCT pController, UINT32 period)
{
    DDKClockGetFreq (DDK_CLOCK_SIGNAL_IPG,&TICKS_PER_S_IPGCLK);
    switch (pController->ClkSrc)
    {
    case GPT_NOCLK:
    case GPT_IPGCLK:
    case GPT_HIGHCLK:
    case GPT_EXTCLK:
        return (UINT32)(((double)period * TICKS_PER_S_IPGCLK ) / ADJUST_VAL_H);
    case GPT_32KCLK:
        return (UINT32)(((double)period * TICKS_PER_S_32K) / ADJUST_VAL_L);
    default:
        return (UINT32)(((double)period * TICKS_PER_S_IPGCLK ) / ADJUST_VAL_H);
    }
}

//------------------------------------------------------------------------------
//
// Function: BSPGptChangeClockSource
//
// This function changes the GPT clock source
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//      pSetSrcPkt 
//          [in] The desired timer source
//
// Returns:
//     NA
//
//------------------------------------------------------------------------------
void BSPGptChangeClockSource(PCSP_GPT_STRUCT pController, PGPT_TIMER_SRC_PKT pSetSrcPkt)
{
    pController->ClkSrc = pSetSrcPkt->timerSrc;
    return;
}

//------------------------------------------------------------------------------
//
// Function: BSPGptShowClockSource
//
// This function shows the current clock source
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//     NA
//
//------------------------------------------------------------------------------
extern void BSPGptShowClockSource(PCSP_GPT_STRUCT pController)
{
    switch (pController->ClkSrc)
    {
    case GPT_NOCLK:
        RETAILMSG(1, (L"Clk Source is NO CLK\r\n"));
        DEBUGMSG(1, (L"Clk Source is NO CLK\r\n"));
        break;
    case GPT_IPGCLK:
        RETAILMSG(1, (L"Clk Source is IPG_CLK\r\n"));
        DEBUGMSG(1, (L"Clk Source is IPG_CLK\r\n"));
        break;
    case GPT_HIGHCLK:
        RETAILMSG(1, (L"Clk Source is HI_FRQCLK\r\n"));
        DEBUGMSG(1, (L"Clk Source is HI_FRQCLK\r\n"));
        break;
    case GPT_EXTCLK:
        RETAILMSG(1, (L"Clk Source is EXT_CLK\r\n"));
        DEBUGMSG(1, (L"Clk Source is EXT_CLK\r\n"));
        break;
    case GPT_32KCLK:
        RETAILMSG(1, (L"Clk Source is 32K_CLK\r\n"));
        DEBUGMSG(1, (L"Clk Source is 32K_CLK\r\n"));
        break;
    }
    return;
}

//------------------------------------------------------------------------------
//
// Function: BSPGptCrmSetClockGatingMode
//
// This function calls to the CRM module to
// set the clock gating mode, turning on or off
// clocks to the GPT.
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//      startClocks
//          [in] If TRUE, turn clocks to GPT on.
//                If FALSE, turn clocks to GPT off
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL BSPGptSetClockGatingMode(PCSP_GPT_STRUCT pController, BOOL startClocks)
{
    DDK_CLOCK_GATE_INDEX GateIndex;
    switch (pController->dwGptIndex)
    {
    case 1:
        GateIndex = DDK_CLOCK_GATE_INDEX_GPT1;
        break;
    case 2:
        GateIndex = DDK_CLOCK_GATE_INDEX_GPT2;
        break;
    case 3:
        GateIndex = DDK_CLOCK_GATE_INDEX_GPT3;
        break;
    case 4:
        GateIndex = DDK_CLOCK_GATE_INDEX_GPT4;
        break;
    default:
        return FALSE;
        break;
    }

    if (startClocks)
    {
        // Turn GPT clocks on
        if (!DDKClockSetGatingMode(GateIndex, DDK_CLOCK_GATE_MODE_ENABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed to set CRM clock gating mode!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }
    else
    {
        // Turn GPT clocks off
        if (!DDKClockSetGatingMode(GateIndex, DDK_CLOCK_GATE_MODE_DISABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed to set CRM clock gating mode!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: BSPGptGetBaseAddress
//
// This function gets GPT base address
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      Base address of the GPT controller - if success.
//      NULL if failure.
//
//------------------------------------------------------------------------------
DWORD BSPGptGetBaseAddress(PCSP_GPT_STRUCT pController)
{
    DWORD bRet = 0;
    switch (pController->dwGptIndex)
    {
    case 1:
        bRet = CSP_BASE_REG_PA_GPT1;
        break;
    case 2:
        bRet = CSP_BASE_REG_PA_GPT2;
        break;
    case 3:
        bRet = CSP_BASE_REG_PA_GPT3;
        break;
    case 4:
        bRet = CSP_BASE_REG_PA_GPT4;
        break;
    default:
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed to get BaseAddress!\r\n"), __WFUNCTION__));
        break;
    }

    return bRet;
}


//------------------------------------------------------------------------------
//
// Function: BSPGptGetIRQ
//
// This function gets GPT IRQ number
//
// Parameters:
//      pController
//          [in] Pointer to GPT structure
//
// Returns:
//      IRQ of the GPT controller - if success.
//      0 if failure.
//
//------------------------------------------------------------------------------
DWORD BSPGptGetIRQ(PCSP_GPT_STRUCT pController)
{
    DWORD bRet = 0;
    switch (pController->dwGptIndex)
    {
    case 1:
        bRet = IRQ_GPT1;
        break;
    case 2:
        bRet = IRQ_GPT2;
        break;
    case 3:
        bRet = IRQ_GPT3;
        break;
    case 4:
        bRet = IRQ_GPT4;
        break;
    default:
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed to get IRQ!\r\n"), __WFUNCTION__));
        break;
    }

    return bRet;
}
