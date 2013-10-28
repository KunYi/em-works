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
//
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: ddk_clk.c
//
// This file contains the PLLCRC DDK interface that is used by applications and
// other drivers to access the capabilities of the PLLCRC driver.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <ceddk.h>
#include "csp.h"

//------------------------------------------------------------------------------
// External Functions
extern BOOL BSPClockAlloc(void);
extern BOOL BSPClockDealloc(void);
extern BOOL BSPClockGetFreq(DDK_CLOCK_SIGNAL sig, UINT32 *freq);
extern BOOL BSPClockUpdateFreq(DDK_CLOCK_SIGNAL sig, 
    DDK_CLOCK_BAUD_SOURCE src, UINT32 div);

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables
static PCSP_PLLCRC_REGS g_pPLLCRC = NULL;

//------------------------------------------------------------------------------
// Local Functions
BOOL ClockAlloc(void);
BOOL ClockDealloc(void);

//------------------------------------------------------------------------------
//
// Function: ClockAlloc
//
// This function allocates the data structures required for interaction
// with the clock configuration hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL ClockAlloc(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;
         
    if (g_pPLLCRC == NULL) {
        phyAddr.QuadPart = CSP_BASE_REG_PA_CRM;

        // Map peripheral physical address to virtual address
        g_pPLLCRC = (PCSP_PLLCRC_REGS) MmMapIoSpace(phyAddr, 
            sizeof(CSP_PLLCRC_REGS), FALSE);

        // Check if virtual mapping failed
        if (g_pPLLCRC == NULL) {
            DBGCHK((_T("CSPDDK")), FALSE);
            ERRORMSG(1, (_T("ClockAlloc: MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

    }
    rc = BSPClockAlloc();
     
cleanUp:

    if (!rc) ClockDealloc();
    return rc;
}

//------------------------------------------------------------------------------
//
// Function: ClockDealloc
//
// This function deallocates the data structures required for interaction
// with the clock configuration hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//------------------------------------------------------------------------------
BOOL ClockDealloc(void)
{
    // Unmap peripheral address space
    if (g_pPLLCRC == NULL) {
        MmUnmapIoSpace(g_pPLLCRC, sizeof(CSP_PLLCRC_REGS));
        g_pPLLCRC = NULL;
    }

    return BSPClockDealloc();
}

//------------------------------------------------------------------------------
//
// Function: DDKClockSetGatingMode
//
// Sets the clock gating mode of the peripheral.
//
// Parameters:
//      index
//          [in] Index for referencing peripheral clock gating control bits.
//      mode
//          [in] Requested clock gating mode for the peripheral.
//
// Returns:
//      Returns TRUE if the clock gating mode was set successfully, otherwise 
//      returns FALSE.
//
//------------------------------------------------------------------------------
BOOL DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode)
{
    UINT32 pccrValOld, pccrValNew, pccrIndex, pccrShift, pccrMask;
    
    // Divide clock gating index by 32 to determine PCCR register index
    pccrIndex = index >> 5;

    // Calculate shift for PCCR bits by using the index
    pccrShift = index % 32;
    
    // Calculate bitmask for PCCR bits
    pccrMask = PLLCRC_PCCR_CG_MASK  << pccrShift;

    // Keep trying until the shared register access to the PCCR succeeds
    do {
        pccrValOld = INREG32(&g_pPLLCRC->PCCR0 + pccrIndex);
        pccrValNew = (pccrValOld & (~pccrMask)) | (mode << pccrShift);
    } while (InterlockedTestExchange((LPLONG)(&g_pPLLCRC->PCCR0 + pccrIndex), 
        pccrValOld, pccrValNew) != pccrValOld);        

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: DDKClockGetGatingMode
//
// Retrieves the clock gating mode of the peripheral.
//
// Parameters:
//      index
//          [in] Index for referencing the peripheral clock gating control bits.
//      pMode
//          [out] Current clock gating mode for the peripheral.
//
// Returns:
//      Returns TRUE if the clock gating mode was set successfully, otherwise 
//      returns FALSE.
//
//------------------------------------------------------------------------------
BOOL DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE *pMode)
{
    UINT32 pccrIndex, pccrShift;
    
    // Divide clock gating index by 32 to determine PCCR register index
    pccrIndex = index >> 5;

    // Calculate shift for PCCR bits by using the index
    pccrShift = index % 32;

    *pMode = (INREG32(&g_pPLLCRC->PCCR0 + pccrIndex) >> pccrShift) 
        & PLLCRC_PCCR_CG_MASK;

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: DDKClockGetFreq
//
// Retrieves the clock frequency in Hz for the specified clock signal.
//
// Parameters:
//      sig
//          [in] Clock signal.
//      freq
//          [out] Current frequency in Hz.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL DDKClockGetFreq(DDK_CLOCK_SIGNAL sig, UINT32 *freq)
{
    return BSPClockGetFreq(sig, freq);
}

//------------------------------------------------------------------------------
//
// Function: DDKClockConfigBaud
//
// Configures the input source clock and dividers for the specified PLLCRC 
// peripheral baud clock output.
//
// Parameters:
//      sig
//          [in] Clock signal to configure.
//      src
//          [in] Selects the input clock source.
//      div
//          [in] Specifies the value programmed into the baud clock divider.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL DDKClockConfigBaud(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src, 
    UINT32 div)
{
    BOOL rc = TRUE;
    REG32 *pDivReg;
    UINT32 oldReg, newReg;
    UINT32 cscrShift, cscrMask;
    UINT32 divShift, divMask;
    
    switch (sig) {
        case DDK_CLOCK_SIGNAL_SSI1:
            cscrShift = PLLCRC_CSCR_SSI1_SEL_LSH;
            cscrMask = CSP_BITFMASK(PLLCRC_CSCR_SSI1_SEL);
            divShift = PLLCRC_PCDR0_SSI1DIV_LSH;
            divMask = CSP_BITFMASK(PLLCRC_PCDR0_SSI1DIV);
            pDivReg = &g_pPLLCRC->PCDR0;
            break;

        case DDK_CLOCK_SIGNAL_SSI2:
            cscrShift = PLLCRC_CSCR_SSI2_SEL_LSH;
            cscrMask = CSP_BITFMASK(PLLCRC_CSCR_SSI2_SEL);
            divShift = PLLCRC_PCDR0_SSI2DIV_LSH;
            divMask = CSP_BITFMASK(PLLCRC_PCDR0_SSI2DIV);
            pDivReg = &g_pPLLCRC->PCDR0;
            break;

        case DDK_CLOCK_SIGNAL_MSHC:
            cscrShift = PLLCRC_CSCR_MSHC_SEL_LSH;
            cscrMask = CSP_BITFMASK(PLLCRC_CSCR_MSHC_SEL);
            divShift = PLLCRC_PCDR0_MSHCDIV_LSH;
            divMask = CSP_BITFMASK(PLLCRC_PCDR0_MSHCDIV);
            pDivReg = &g_pPLLCRC->PCDR0;
            break;

        case DDK_CLOCK_SIGNAL_H264:
            cscrShift = PLLCRC_CSCR_H264_SEL_LSH;
            cscrMask = CSP_BITFMASK(PLLCRC_CSCR_H264_SEL);
            divShift = PLLCRC_PCDR0_H264DIV_LSH;
            divMask = CSP_BITFMASK(PLLCRC_PCDR0_H264DIV);
            pDivReg = &g_pPLLCRC->PCDR0;
            break;

        case DDK_CLOCK_SIGNAL_USB:
            // USBPLL is only USB clock source
            if (src != DDK_CLOCK_BAUD_SOURCE_SPLL) {
                rc = FALSE;
                break;
            }
            cscrMask = 0;
            divShift = PLLCRC_CSCR_USB_DIV_LSH;
            divMask = CSP_BITFMASK(PLLCRC_CSCR_USB_DIV);
            pDivReg = &g_pPLLCRC->CSCR;
            break;

        default:
            rc = FALSE;
            break;
    }

    // If baud configuration is available
    if (rc) {
        // Update the global clock signal table
        BSPClockUpdateFreq(sig, src, div);
        
        // Update the source selection
        if (cscrMask) {
            do {
                oldReg = INREG32(&g_pPLLCRC->CSCR);
                newReg = (oldReg & (~cscrMask)) | (src << cscrShift);
            } while (InterlockedTestExchange((LPLONG)&g_pPLLCRC->CSCR, 
                oldReg, newReg) != oldReg);        
        }

        // Update the dividers
        do {
            oldReg = INREG32(pDivReg);
            newReg = (oldReg & (~divMask)) | (div << divShift);
        } while (InterlockedTestExchange((LPLONG)pDivReg, 
            oldReg, newReg) != oldReg);        
    }

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: DDKClockSetCKO
//
// Configures the clock output source (CKO) signal.
//
// Parameters:
//      bEnable
//          [in] Set to TRUE to enable CKO output.  Set to FALSE to disable
//          CKO output.
//      src
//          [in] Selects the CKO source signal.
//      div
//          [in] Specifies the CKO divide factor.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL DDKClockSetCKO(BOOL bEnable, DDK_CLOCK_CKO_SRC src, DDK_CLOCK_CKO_DIV div)
{
    // Select CKO signal source
    INSREG32BF(&g_pPLLCRC->CCSR, PLLCRC_CCSR_CLKO_SEL, src);

    // Configure CKO divider
    INSREG32BF(&g_pPLLCRC->PCDR0, PLLCRC_PCDR0_CLKO_DIV, div);

    // Enable/Disable CKO
    INSREG32BF(&g_pPLLCRC->PCDR0, PLLCRC_PCDR0_CLKO_EN, bEnable);
    
    return TRUE;
}

