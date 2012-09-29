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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//
//  File:  ethdrv.c
//
//  Common Ethernet debug driver support that is shared between EBOOT and
//  KITL.
//
//-----------------------------------------------------------------------------
#include "bsp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
// Prototypes for Ethernet controller I/O functions
typedef void (*PFN_EDBG_SETREGDW)(const UINT32 dwBase, const UINT32 dwOffset, 
                                    const UINT32 dwVal);
typedef UINT32 (*PFN_EDBG_GETREGDW)(const UINT32 dwBase, const UINT32 dwOffset);
typedef void (*PFN_EDBG_READFIFO)(const UINT32 dwBase, const UINT32 dwOffset, 
                                    UINT32 *pdwBuf, UINT32 dwDwordCount);
typedef void (*PFN_EDBG_WRITEFIFO)(const UINT32 dwBase, const UINT32 dwOffset, 
                                      const UINT32 *pdwBuf, UINT32 dwDwordCount);


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
//

/*
//------------------------------------------------------------------------------
//
//  Function:  OALStall
//
VOID OALStall(UINT32 uSecs)
{
    LARGE_INTEGER liStart, liEnd, liStallCount;
    static LARGE_INTEGER liFreq = {0, 0};
    static PCSP_EPIT_REG pEPIT = NULL;
    BSP_ARGS *pBspArgs;

    if (pEPIT == NULL)
    {
        // Map EPIT registers
        pEPIT = (PCSP_EPIT_REG) OALPAtoUA(CSP_BASE_REG_PA_EPIT1);
        if (pEPIT == NULL)
        {
            EdbgOutputDebugString("OALStall: EPIT mapping failed!\r\n");
            return;
        }

    }

    if (liFreq.QuadPart == 0)
    {
        pBspArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;
        
        switch(EXTREG32BF(&pEPIT->CR, EPIT_CR_CLKSRC))
        {
        case EPIT_CR_CLKSRC_CKIL:
            liFreq.QuadPart = BSP_CLK_CKIL_FREQ;
            break;

        case EPIT_CR_CLKSRC_IPGCLK:
            liFreq.QuadPart = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_EPIT];
            break;
        case EPIT_CR_CLKSRC_HIGHFREQ:
            liFreq.QuadPart = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB];
            break;
        }

        liFreq.QuadPart = liFreq.QuadPart / (EXTREG32BF(&pEPIT->CR, EPIT_CR_PRESCALAR) + 1);
    }


    liStallCount.QuadPart = ((liFreq.QuadPart * uSecs - 1) / 1000000) + 1;   // always round up
    
    liStart.QuadPart = INREG32(&pEPIT->CNT);
    do {
        liEnd.QuadPart = INREG32(&pEPIT->CNT);
    } while ( (liStart.QuadPart - liEnd.QuadPart) <= liStallCount.QuadPart);

}


//-----------------------------------------------------------------------------
//
//  Function: OEMEthGetSecs
//
//  This function returns the number of seconds that have passed since a 
//  certain fixed time.  This function handles time-outs while in polling 
//  mode. The origin of the count is unimportant as long as the count is 
//  incremental.
//
//  Parameters:
//      None.
//
// Returns:
//      Count of seconds that have passed since a certain fixed time.
//
//-----------------------------------------------------------------------------

DWORD OEMEthGetSecs(void)
{
#if 1
    static LARGE_INTEGER liFreq = {0, 0};
    static PCSP_EPIT_REG pEPIT = NULL;
    BSP_ARGS *pBspArgs;
    DWORD dwCount;
    DWORD freqHz;
    
    if (pEPIT == NULL)
    {
        pEPIT = (PCSP_EPIT_REG) OALPAtoUA(CSP_BASE_REG_PA_EPIT1);
    }
    dwCount = 0-INREG32(&pEPIT->CNT);
    // Calculate the timer frequency
    if (liFreq.QuadPart == 0)
    {
        pBspArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;
        
        switch(EXTREG32BF(&pEPIT->CR, EPIT_CR_CLKSRC))
        {
        case EPIT_CR_CLKSRC_CKIL:
            liFreq.QuadPart = BSP_CLK_CKIL_FREQ;
            break;

        case EPIT_CR_CLKSRC_IPGCLK:
            liFreq.QuadPart = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_EPIT];
            break;

        case EPIT_CR_CLKSRC_HIGHFREQ:
            liFreq.QuadPart = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB];
            break;
        }
        
        liFreq.QuadPart = liFreq.QuadPart / (EXTREG32BF(&pEPIT->CR, EPIT_CR_PRESCALAR) + 1);        
    }

    freqHz = liFreq.LowPart;
    if (freqHz)
    {
        return dwCount/freqHz;
    }
    else
    {
        return 0;
    }

#else
    DWORD dwCounter;
    //using Dry Ice Timer instead of RTC since there isn't an RTC in i.MX25
    volatile PCSP_DRYICE_REGS pDryIce = (PCSP_DRYICE_REGS)OALPAtoUA(CSP_BASE_REG_PA_DRYICE);

    //Should we lock the Time Counter Registers so that the counters are read-only?

    //If Time Counter Reg is secure-only register (if we read back all 0's) then system may need to set the NSA bit in the Control
    //register to 1. (Datasheet did not say Time ctr register can only be accessed by secure SW only so for now assume Time Ctr 
    //reg is non-secure.
    
    //Time Counter MSB is a seconds timer
    dwCounter = INREG32(&pDryIce->DTCMR);
    //RETAILMSG(1,(TEXT("OEMEthGetSecs : %d 0x%x\r\n"),dwCounter,INREG32(&pDryIce->DSR)));
    return dwCounter;
#endif
}
*/


void EthSetRegDW_16bit(const UINT32 dwBase, const UINT32 dwOffset, const UINT32 dwVal)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwBase);
    CPLDWrite16(CPLD_LAN9217_OFFSET + dwOffset, (UINT16)dwVal);
    CPLDWrite16(CPLD_LAN9217_OFFSET + dwOffset + 2, (UINT16)(dwVal >> 16));
}


UINT32 EthGetRegDW_16bit(const UINT32 dwBase, const UINT32 dwOffset)
{
    UINT16 hi, lo;
  
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwBase);

    lo = CPLDRead16(CPLD_LAN9217_OFFSET + dwOffset);
    hi = CPLDRead16(CPLD_LAN9217_OFFSET + dwOffset + 2);    

    return (UINT32)((hi << 16) + lo);
}


void EthWriteFifo_16bit(const UINT32 dwBase, const UINT32 dwOffset, const UINT32 *pdwBuf, UINT32 dwDwordCount)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwBase);

    CPLDWriteFifo16(CPLD_LAN9217_OFFSET + dwOffset, (UINT16*)pdwBuf, dwDwordCount);    
}


void EthReadFifo_16bit(const UINT32 dwBase, const UINT32 dwOffset, UINT32 *pdwBuf, UINT32 dwDwordCount)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwBase);

    CPLDReadFifo16(CPLD_LAN9217_OFFSET + dwOffset, (UINT16*)pdwBuf, dwDwordCount);
}



extern PFN_EDBG_SETREGDW g_pfnEdbgSetRegDW;
extern PFN_EDBG_GETREGDW g_pfnEdbgGetRegDW;
extern PFN_EDBG_READFIFO g_pfnEdbgReadFIFO;
extern PFN_EDBG_WRITEFIFO g_pfnEdbgWriteFIFO;

void EthMapIO(void)
{
    g_pfnEdbgSetRegDW = (PFN_EDBG_SETREGDW) EthSetRegDW_16bit;
    g_pfnEdbgGetRegDW = (PFN_EDBG_GETREGDW) EthGetRegDW_16bit;
    g_pfnEdbgWriteFIFO = (PFN_EDBG_WRITEFIFO) EthWriteFifo_16bit;
    g_pfnEdbgReadFIFO = (PFN_EDBG_READFIFO) EthReadFifo_16bit;
}
