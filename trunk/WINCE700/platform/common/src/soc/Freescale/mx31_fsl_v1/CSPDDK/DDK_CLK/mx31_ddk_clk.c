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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddk_clk.c
//
//  This file contains the CCM DDK interface that is used by applications and
//  other drivers to access the capabilities of the CCM driver.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"

//-----------------------------------------------------------------------------
// External Functions
extern BOOL BSPClockAlloc(void);
extern BOOL BSPClockDealloc(void);
extern BOOL BSPClockGetFreq(DDK_CLOCK_SIGNAL sig, UINT32 *freq);
extern BOOL BSPClockUpdateFreq(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src,
                               UINT32 preDiv, UINT32 postDiv);
extern BOOL BSPClockSetGatingModePrepare(DDK_CLOCK_GATE_INDEX index);
extern BOOL BSPClockSetGatingModeComplete(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);
extern BOOL BSPClockSetpointRequest(DDK_DVFC_SETPOINT setpoint, BOOL bBlock);
extern BOOL BSPClockSetpointRelease(DDK_DVFC_SETPOINT setpoint);


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
HANDLE g_hDdkClkMutex;
HANDLE g_hDvfcWorkerEvent;
PCSP_CCM_REGS g_pCCM = NULL;
PDDK_CLK_CONFIG g_pDdkClkConfig = NULL;

//USB KITL (USING OTG) can control the USB clock independent of DDK_CLK
DWORD g_UsbKitl = 0;

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions
BOOL ClockInit(void);
BOOL ClockAlloc(void);
BOOL ClockDealloc(void);

//-----------------------------------------------------------------------------
//
//  Function:  ClockInit
//
//  This function initializes the DDK_CLOCK state variables.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL ClockInit(void)
{
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  ClockAlloc
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
//-----------------------------------------------------------------------------
BOOL ClockAlloc(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;
    HKEY hKey=NULL;
    DWORD dwRet;

    if (g_pCCM == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_CCM;

        // Map peripheral physical address to virtual address
        g_pCCM = (PCSP_CCM_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_CCM_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pCCM == NULL)
        {
            DBGCHK((_T("CSPDDK")),  FALSE);
            ERRORMSG(1, (_T("ClockAlloc:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

    }

    // Create event for signaling the DVFC driver
    if (g_hDvfcWorkerEvent == NULL)
    {
        g_hDvfcWorkerEvent = CreateEvent(NULL, FALSE, FALSE, L"EVENT_DVFC_WORKER");

        if (g_hDvfcWorkerEvent == NULL)
        {
            ERRORMSG(1, (_T("CreateEvent failed!\r\n")));
            goto cleanUp;
        }
    }

    // Create mutex for safe access to CCM registers
    if (g_hDdkClkMutex == NULL)
    {
        g_hDdkClkMutex = CreateMutex(NULL, FALSE, L"MUTEX_DDKCLK");

        if (g_hDdkClkMutex == NULL)
        {
            ERRORMSG(1, (_T("CreateMutex failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = BSPClockAlloc();

    //USB KITL ONLY: Check the KITL registry key to see if we are running on
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,TEXT("DRIVERS\\BUILTIN\\CSPDDK\\"),0,0,&hKey);

    if((ERROR_SUCCESS == dwRet) && (hKey != NULL))
    {
        DWORD dwType;
        DWORD dwLength = sizeof(g_UsbKitl);
        RegQueryValueEx(hKey, TEXT("USBKITL"), 0, &dwType, (PBYTE)&g_UsbKitl, &dwLength);
    }



cleanUp:

    if (!rc) ClockDealloc();

    return rc;

}


//-----------------------------------------------------------------------------
//
// Function:  ClockDealloc
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
//-----------------------------------------------------------------------------
BOOL ClockDealloc(void)
{
    // Unmap peripheral address space
    if (g_pCCM == NULL)
    {
        MmUnmapIoSpace(g_pCCM, sizeof(CSP_CCM_REGS));
        g_pCCM = NULL;
    }

    if (g_hDvfcWorkerEvent)
    {
        CloseHandle(g_hDvfcWorkerEvent);
        g_hDvfcWorkerEvent = NULL;
    }

    if (g_hDdkClkMutex)
    {
        CloseHandle(g_hDdkClkMutex);
        g_hDdkClkMutex = NULL;
    }

    return BSPClockDealloc();

}


//-----------------------------------------------------------------------------
//
// Function: DDKClockSetGatingMode
//
// Sets the clock gating mode of the peripheral.
//
// Parameters:
//      index
//           [in] Index for referencing the peripheral clock gating control 
//           bits.
//
//      mode
//           [in] Requested clock gating mode for the peripheral.
//
// Returns:
//      Returns TRUE if the clock gating mode was set successfully, otherwise 
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode)
{
    UINT32 mcgrIndex, mcgrShift, mcgrMask;
    
    // Check if this clock gate index is a "special case"
    if (index < DDK_CLOCK_GATE_INDEX_SPECIAL)
    {

        // Divide clock gating index by 16 to determine MCGR register index
        mcgrIndex = index >> 4;

        // Calculate shift for MCG bits by using the index within the
        // MCGR register (0-15) and multiply by 2
        mcgrShift = (index % 16) << 1;

        // Calculate bitmask for MCG bits
        mcgrMask = CCM_CGR_CG_MASK <<  mcgrShift;
    }

    // Handle special one-bit cases in CGR2
    else
    {
        mcgrIndex = 2;
        mcgrShift = index - DDK_CLOCK_GATE_INDEX_SPECIAL;
        mcgrMask = (1U << mcgrShift);

        // Limit the mode to single bit
        if (mode == DDK_CLOCK_GATE_MODE_DISABLED)
        {
            mode = 0;
        }
        else
        {
            mode = 1;
        }
    }

    // Return if we are using USBKITL and trying to turn off the USB clock.
    if((DDK_CLOCK_GATE_INDEX_USBOTG == index) && (DDK_CLOCK_GATE_MODE_DISABLED==mode) && (g_UsbKitl))
    {
        return TRUE;
    }


    // Check if we are already at the required state 
    if ((DDK_CLOCK_GATE_MODE) EXTREG32(&g_pCCM->CGR[mcgrIndex], mcgrMask, mcgrShift) == mode)
    {
        return TRUE;
    }


    // If clock is being enabled, call down to platform code to check if
    // we need to block until a system setpoint that can support this
    // clock mode is available
    if (mode != DDK_CLOCK_GATE_MODE_DISABLED)
    {
        BSPClockSetGatingModePrepare(index);
    }

    // Grab mutex before accessing/updating the shared CCM registers and
    // corresponding DVFC data structures
    WaitForSingleObject(g_hDdkClkMutex, INFINITE);

    INSREG32(&g_pCCM->CGR[mcgrIndex], mcgrMask, (mode << mcgrShift));

    // Allow platform code to complete the clock gating request
    BSPClockSetGatingModeComplete(index, mode);

    ReleaseMutex(g_hDdkClkMutex);

    return TRUE;

}


//-----------------------------------------------------------------------------
//
// Function: DDKClockGetGatingMode
//
// Retrieves the clock gating mode of the peripheral.
//
// Parameters:
//      index
//           [in] Index for referencing the peripheral clock gating control
//           bits.
//
//      pMode
//           [out] Current clock gating mode for the peripheral.
//
// Returns:
//      Returns TRUE if the clock gating mode was set successfully, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE *pMode)
{
    UINT32 mcgrIndex, mcgrShift, mcgrMask;
    
    // Check if this clock gate index is a "special case"
    if (index < DDK_CLOCK_GATE_INDEX_SPECIAL)
{

    // Divide clock gating index by 16 to determine MCGR register index
    mcgrIndex = index >> 4;

    // Calculate shift for MCG bits by using the index within the
    // MCGR register (0-15) and multiply by 2
    mcgrShift = (index % 16) << 1;

        // Calculate bitmask for MCG bits
        mcgrMask = CCM_CGR_CG_MASK;
    }

    // Handle special one-bit cases in CGR2
    else
    {
        mcgrIndex = 2;
        mcgrShift = index - DDK_CLOCK_GATE_INDEX_SPECIAL;
        mcgrMask = 1U;
    }


    *pMode = ((INREG32(&g_pCCM->CGR[mcgrIndex])) >> mcgrShift)
             & mcgrMask;

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: DDKClockGetFreq
//
// Retrieves the clock frequency in Hz for the specified clock signal.
//
// Parameters:
//      sig
//           [in] Clock signal.
//
//      freq
//           [out] Current frequency in Hz.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockGetFreq(DDK_CLOCK_SIGNAL sig, UINT32 *freq)
{
    return BSPClockGetFreq(sig, freq);
}


//-----------------------------------------------------------------------------
//
// Function: DDKClockConfigBaud
//
// Configures the input source clock and dividers for the specified
// CCM peripheral baud clock output.
//
// Parameters:
//      sig
//          [in] Clock signal to configure.
//
//      src
//          [in] Selects the input clock source.
//
//      preDiv
//          [in] Specifies the value programmed into the baud clock predivider.
//
//      postDiv
//          [in] Specifies the value programmed into the baud clock postdivider.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockConfigBaud(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src,
                        UINT32 preDiv, UINT32 postDiv)
{
    BOOL rc = TRUE;

    UINT32 oldReg, newReg, *pPDR = NULL;
    UINT32 ccmrShift = 0;
    UINT32 ccmrMask = 0;
    UINT32 preDivShift = 0;
    UINT32 preDivMask = 0;
    UINT32 postDivShift = 0;
    UINT32 postDivMask = 0;

    switch (sig)
    {

    case DDK_CLOCK_SIGNAL_SSI1:
        if (src == DDK_CLOCK_BAUD_SOURCE_NONE)
        {
            g_pDdkClkConfig->ssi1Cgr0Mask = 0;
            break;
        }

        g_pDdkClkConfig->ssi1Cgr0Mask = CCM_CGR0_SSI1_MASK;
        ccmrShift = CCM_CCMR_SSI1S_LSH;
        ccmrMask = CSP_BITFMASK(CCM_CCMR_SSI1S);
        preDivShift = CCM_PDR1_SSI1_PRE_PODF_LSH;
        preDivMask = CSP_BITFMASK(CCM_PDR1_SSI1_PRE_PODF);
        postDivShift = CCM_PDR1_SSI1_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_PDR1_SSI1_PODF);
        pPDR = &g_pCCM->PDR1;
        break;

    case DDK_CLOCK_SIGNAL_SSI2:
        if (src == DDK_CLOCK_BAUD_SOURCE_NONE)
        {
            g_pDdkClkConfig->ssi2Cgr2Mask = 0;
            break;
        }

        g_pDdkClkConfig->ssi2Cgr2Mask = CCM_CGR2_SSI2_MASK;
        ccmrShift = CCM_CCMR_SSI2S_LSH;
        ccmrMask = CSP_BITFMASK(CCM_CCMR_SSI2S);
        preDivShift = CCM_PDR1_SSI2_PRE_PODF_LSH;
        preDivMask = CSP_BITFMASK(CCM_PDR1_SSI2_PRE_PODF);
        postDivShift = CCM_PDR1_SSI2_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_PDR1_SSI2_PODF);
        pPDR = &g_pCCM->PDR1;
        break;

    case DDK_CLOCK_SIGNAL_FIRI:
        if (src == DDK_CLOCK_BAUD_SOURCE_NONE)
        {
            g_pDdkClkConfig->firCgr2Mask = 0;
            break;
        }

        g_pDdkClkConfig->firCgr2Mask = CCM_CGR2_FIRI_MASK;
        ccmrShift = CCM_CCMR_FIRS_LSH;
        ccmrMask = CSP_BITFMASK(CCM_CCMR_FIRS);
        preDivShift = CCM_PDR1_FIRI_PRE_PODF_LSH;
        preDivMask = CSP_BITFMASK(CCM_PDR1_FIRI_PRE_PODF);
        postDivShift = CCM_PDR1_FIRI_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_PDR1_FIRI_PODF);
        pPDR = &g_pCCM->PDR1;
        break;

    case DDK_CLOCK_SIGNAL_CSI:
        if (src == DDK_CLOCK_BAUD_SOURCE_NONE)
        {
            g_pDdkClkConfig->csiCgr1Mask = 0;
            break;
        }
        // CSI has different clock source selections
        else if (src == DDK_CLOCK_BAUD_SOURCE_SERPLL)
        {
            src = CCM_CCMR_CSCS_SERIAL_CLK;
        }
        else if (src == DDK_CLOCK_BAUD_SOURCE_USBPLL)
        {
            src = CCM_CCMR_CSCS_USB_CLK;
        }
        else
        {
            rc = FALSE;
            break;
        }

        g_pDdkClkConfig->csiCgr1Mask = CCM_CGR1_CSI_MASK;
        ccmrShift = CCM_CCMR_CSCS_LSH;
        ccmrMask = CSP_BITFMASK(CCM_CCMR_CSCS);
        preDiv = 0;
        postDivShift = CCM_PDR0_CSI_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_PDR0_CSI_PODF);
        pPDR = &g_pCCM->PDR0;
        break;

    case DDK_CLOCK_SIGNAL_USB:

        if (src == DDK_CLOCK_BAUD_SOURCE_NONE)
        {
            g_pDdkClkConfig->usbCgr1Mask = 0;
            break;
        }
    
        // USBPLL is only USB clock source
        else if (src != DDK_CLOCK_BAUD_SOURCE_USBPLL)
        {
            rc = FALSE;
            break;
        }

        g_pDdkClkConfig->usbCgr1Mask = CCM_CGR1_USBOTG_MASK;            
        preDivShift = CCM_PDR1_USB_PRDF_LSH;
        preDivMask = CSP_BITFMASK(CCM_PDR1_USB_PRDF);
        postDivShift = CCM_PDR1_USB_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_PDR1_USB_PODF);
        pPDR = &g_pCCM->PDR1;

        break;

    default:
        rc = FALSE;
        break;

    }

    // If baud configuration is available
    if (rc)
    {
        // Update the global clock signal table
        BSPClockUpdateFreq(sig, src, preDiv, postDiv);

        // Update the source selection
        if (ccmrMask)
        {
            // Grab mutex before updating CCMR
            WaitForSingleObject(g_hDdkClkMutex, INFINITE);

            INSREG32(&g_pCCM->CCMR, ccmrMask, (src << ccmrShift));

            ReleaseMutex(g_hDdkClkMutex);

        }

        // Update the dividers
        if (pPDR)
        {
        preDiv <<= preDivShift;
        preDiv &= preDivMask;
        postDiv <<= postDivShift;
        postDiv &= postDivMask;
        do
        {
            oldReg = INREG32(pPDR);
                newReg = (oldReg & (~(preDivMask | postDivMask))) 
                    | (preDiv) | (postDiv);
            } while ((UINT32) InterlockedTestExchange((LPLONG) pPDR, 
                               oldReg, newReg) != oldReg);        
        }

    }


    return rc;

}


//-----------------------------------------------------------------------------
//
// Function: DDKClockSetCKO
//
// Configures the clock output source (CKO) signal.
//
// Parameters:
//      bEnable
//          [in] Set to TRUE to enable CKO output.  Set to FALSE to disable
//          CKO output.
//
//      src
//          [in] Selects the CKO source signal.
//
//      div
//          [in] Specifies the CKO divide factor.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetCKO(BOOL bEnable, DDK_CLOCK_CKO_SRC src, DDK_CLOCK_CKO_DIV div)
{
    // Configure the CKO signal using the CCM COSR
    OUTREG32(&g_pCCM->COSR, CSP_BITFVAL(CCM_COSR_CLKOEN, bEnable) |
                            CSP_BITFVAL(CCM_COSR_CLKOSEL, src)    |
                            CSP_BITFVAL(CCM_COSR_CLKODIV, div));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKClockSetpointRequest
//
//  Requests the specified setpoint optionally blocks until the setpoint
//  is granted.
//
// Parameters:
//      setpoint
//          [in] - Specifies the setpoint to be requested.
//
//      bBlock
//          [in] - Set TRUE to block until the setpoint has been granted.  
//                 Set FALSE to return immediately after request has
//                 submitted.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetpointRequest(DDK_DVFC_SETPOINT setpoint, BOOL bBlock)
{
    return BSPClockSetpointRequest(setpoint, bBlock);
}

//-----------------------------------------------------------------------------
//
// Function: DDKClockEnablePanicMode
//
// Forces the DVFS logic to report a panic condition so that the system
// will immediately transition to the highest DVFS setpoint and prevent
// future requests to a lower setpoint.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockEnablePanicMode(void)
{
#if 0
    WaitForSingleObject(g_hDvfcMutex, INFINITE);

    // If this is first panic request
    if (BSPClockIncrPanicRequests() == 1)
    {
        // Issue request to enter panic mode
        SETREG32(&g_pCCM->PMCR1, DDK_CLOCK_DVFC_MODE_PANIC);
        INSREG32BF(&g_pCCM->PMCR0, CCM_PMCR0_DVFEV, CCM_PMCR0_DVFEV_REQ_ALWAYS);
    }

    ReleaseMutex(g_hDvfcMutex);
#endif
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: DDKClockDisablePanicMode
//
// Restores normal DVFS operation from a prior request for panic mode
// and allows the DVFS logic to request higher/lower setpoints as needed.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockDisablePanicMode(void)
{
#if 0
    WaitForSingleObject(g_hDvfcMutex, INFINITE);

    // If there are no active panic requests
    if (BSPClockDecrPanicRequests() == 0)
    {
        // Issue request to return to normal mode
        CLRREG32(&g_pCCM->PMCR1, DDK_CLOCK_DVFC_MODE_PANIC);
        INSREG32BF(&g_pCCM->PMCR0, CCM_PMCR0_DVFEV, CCM_PMCR0_DVFEV_REQ_ALWAYS);
    }

    ReleaseMutex(g_hDvfcMutex);
#endif
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKClockSetpointRelease
//
//  Releases a setpoint previously requested using DDKClockSetpointRequest.
//
// Parameters:
//      setpoint
//          [in] - Specifies the setpoint to be released.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetpointRelease(DDK_DVFC_SETPOINT setpoint)
{
    return BSPClockSetpointRelease(setpoint);
}

