//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  Module: profiler.c
//
//  This module provides the BSP-specific interfaces required to support
//  the PQOAL profiler code.
//
//-----------------------------------------------------------------------------
#include <bsp.h>

//-----------------------------------------------------------------------------
//
//  Function: OALProfileGetClkSrc
//
//  This function returns the clock source setting used to program the GPT
//  CLKSRC bits.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
UINT32 OALProfileGetClkSrc(void)
{
    return GPT_CR_CLKSRC_HIGHFREQ;
}


//-----------------------------------------------------------------------------
//
//  Function: OALProfileGetClkFreq
//
//  This function returns the frequency of the profiler input clock.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
UINT32 OALProfileGetClkFreq(void)
{
    BSP_ARGS *pBspArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;

    switch (OALProfileGetClkSrc())
    {
    case GPT_CR_CLKSRC_HIGHFREQ:
        return pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB];
    case GPT_CR_CLKSRC_IPGCLK:
        return pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_GPT];
    default:
        OALMSG(OAL_ERROR,(TEXT("Invalid profiler clock source : %d\r\n"),OALProfileGetClkSrc()));
        break;
    }
    return 0;
}
