//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_ddk_clk.c
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
extern UINT32 g_SiRev;

//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
HANDLE g_hDdkClkMutex;
PCSP_CCM_REGS g_pCCM = NULL;
PDDK_CLK_CONFIG g_pDdkClkConfig = NULL;

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
    if (g_pCCM)
    {
        MmUnmapIoSpace(g_pCCM, sizeof(CSP_CCM_REGS));
        g_pCCM = NULL;
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
    
    if (index == DDK_CLOCK_GATE_INDEX_NFC)
    {
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

        // Allow platform code to complete the clock gating request
        BSPClockSetGatingModeComplete(index, mode);

        ReleaseMutex(g_hDdkClkMutex);
        return TRUE;
    }

    // Divide clock gating index by 16 to determine MCGR register index
    mcgrIndex = index >> 4;

    // Calculate shift for MCG bits by using the index within the
    // MCGR register (0-15) and multiply by 2
    mcgrShift = (index % 16) << 1;

    // Calculate bitmask for MCG bits
    mcgrMask = CCM_CGR_CG_MASK <<  mcgrShift;

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

    // Divide clock gating index by 16 to determine MCGR register index
    mcgrIndex = index >> 4;

    // Calculate shift for MCG bits by using the index within the
    // MCGR register (0-15) and multiply by 2
    mcgrShift = (index % 16) << 1;

    // Calculate bitmask for MCG bits
    mcgrMask = CCM_CGR_CG_MASK;

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
    UINT32 oldReg, newReg, *pPDR = NULL, *psrcPDR = NULL;
    UINT32 srcShift = 0;
    UINT32 srcMask = 0;
    UINT32 srcPll = 0;
    UINT32 preDivShift = 0;
    UINT32 preDivMask = 0;
    UINT32 postDivShift = 0;
    UINT32 postDivMask = 0;

    switch (sig)
    {
    case DDK_CLOCK_SIGNAL_ESDHC1:
        if (g_SiRev == DDK_SI_REV_TO1)
        {
            srcShift = CCM_PDR3_ESDHC_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR3_ESDHC_M_U);
            preDivShift = CCM_PDR3_ESDHC1_DIV_PRE_LSH;
            preDivMask = CSP_BITFMASK(CCM_PDR3_ESDHC1_DIV_PRE);
            postDivShift = CCM_PDR3_ESDHC1_DIV_PST_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR3_ESDHC1_DIV_PST);
        }
        else
        {
            srcShift = CCM_PDR3_ESDHC_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR3_ESDHC_M_U);
            postDivShift = CCM_PDR3_ESDHC1_DIV_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR3_ESDHC1_DIV);
        }
        pPDR = &g_pCCM->PDR3;
        break;

    case DDK_CLOCK_SIGNAL_ESDHC2:
        if (g_SiRev == DDK_SI_REV_TO1)
        {
            srcShift = CCM_PDR3_ESDHC_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR3_ESDHC_M_U);
            preDivShift = CCM_PDR3_ESDHC2_DIV_PRE_LSH;
            preDivMask = CSP_BITFMASK(CCM_PDR3_ESDHC2_DIV_PRE);
            postDivShift = CCM_PDR3_ESDHC2_DIV_PST_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR3_ESDHC2_DIV_PST);
            pPDR = &g_pCCM->PDR3;
        }
        else
        {
            srcShift = CCM_PDR3_ESDHC_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR3_ESDHC_M_U);
            postDivShift = CCM_PDR3_ESDHC2_DIV_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR3_ESDHC2_DIV);
            pPDR = &g_pCCM->PDR3;
        }
        break;

    case DDK_CLOCK_SIGNAL_ESDHC3:
        if (g_SiRev == DDK_SI_REV_TO1)
        {
            srcShift = CCM_PDR3_ESDHC_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR3_ESDHC_M_U);
            preDivShift = CCM_PDR3_ESDHC3_DIV_PRE_LSH;
            preDivMask = CSP_BITFMASK(CCM_PDR3_ESDHC3_DIV_PRE);
            postDivShift = CCM_PDR3_ESDHC3_DIV_PST_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR3_ESDHC3_DIV_PST);
            pPDR = &g_pCCM->PDR3;
        }
        else
        {
            srcShift = CCM_PDR3_ESDHC_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR3_ESDHC_M_U);
            postDivShift = CCM_PDR3_ESDHC3_DIV_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR3_ESDHC3_DIV);
            pPDR = &g_pCCM->PDR3;
        }
        break;

    case DDK_CLOCK_SIGNAL_SSI1:
        if (preDiv == 0)
        {
            ERRORMSG(1, (_T("The preDiv of SSI1 baud clock must not be 0!\r\n")));
            rc = FALSE;
            break;
        }
        srcShift = CCM_PDR2_SSI_M_U_LSH;
        srcMask = CSP_BITFMASK(CCM_PDR2_SSI_M_U);
        preDivShift = CCM_PDR2_SSI1_DIV_PRE_LSH;
        preDivMask = CSP_BITFMASK(CCM_PDR2_SSI1_DIV_PRE);
        postDivShift = CCM_PDR2_SSI1_DIV_LSH;
        postDivMask = CSP_BITFMASK(CCM_PDR2_SSI1_DIV);
        pPDR = &g_pCCM->PDR2;
        break;

    case DDK_CLOCK_SIGNAL_SSI2:
        if (preDiv == 0)
        {
            ERRORMSG(1, (_T("The preDiv of SSI2 baud clock must not be 0!\r\n")));
            rc = FALSE;
            break;
        }
        srcShift = CCM_PDR2_SSI_M_U_LSH;
        srcMask = CSP_BITFMASK(CCM_PDR2_SSI_M_U);
        preDivShift = CCM_PDR2_SSI2_DIV_PRE_LSH;
        preDivMask = CSP_BITFMASK(CCM_PDR2_SSI2_DIV_PRE);
        postDivShift = CCM_PDR2_SSI2_DIV_LSH;
        postDivMask = CSP_BITFMASK(CCM_PDR2_SSI2_DIV);
        pPDR = &g_pCCM->PDR2;
        break;

    case DDK_CLOCK_SIGNAL_SPDIF:
        if (preDiv == 0)
        {
            ERRORMSG(1, (_T("The preDiv of SPDIF baud clock must not be 0!\r\n")));
            rc = FALSE;
            break;
        }
        srcShift = CCM_PDR3_SPDIF_M_U_LSH;
        srcMask = CSP_BITFMASK(CCM_PDR3_SPDIF_M_U);
        preDivShift = CCM_PDR3_SPDIF_DIV_PRE_LSH;
        preDivMask = CSP_BITFMASK(CCM_PDR3_SPDIF_DIV_PRE);
        postDivShift = CCM_PDR3_SPDIF_DIV_LSH;
        postDivMask = CSP_BITFMASK(CCM_PDR3_SPDIF_DIV);
        pPDR = &g_pCCM->PDR3;
        break;

    case DDK_CLOCK_SIGNAL_CSI:
        if (g_SiRev == DDK_SI_REV_TO1)
        {
            srcShift = CCM_PDR2_CSI_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR2_CSI_M_U);
            preDivShift = CCM_PDR2_CSI_DIV_PRE_LSH;
            preDivMask = CSP_BITFMASK(CCM_PDR2_CSI_DIV_PRE);
            postDivShift = CCM_PDR2_CSI_DIV_PST_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR2_CSI_DIV_PST);
            pPDR = &g_pCCM->PDR2;
        }
        else
        {
            srcShift = CCM_PDR2_CSI_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR2_CSI_M_U);
            postDivShift = CCM_PDR2_CSI_DIV_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR2_CSI_DIV);
            pPDR = &g_pCCM->PDR2;
        }
        break;

    case DDK_CLOCK_SIGNAL_USB:
        if (g_SiRev == DDK_SI_REV_TO1)
        {
            srcShift = CCM_PDR4_USB_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR4_USB_M_U);
            preDivShift = CCM_PDR4_USB_DIV_PRE_LSH;
            preDivMask = CSP_BITFMASK(CCM_PDR4_USB_DIV_PRE);
            postDivShift = CCM_PDR4_USB_DIV_PST_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR4_USB_DIV_PST);
            pPDR = &g_pCCM->PDR4;
        }
        else
        {
            srcShift = CCM_PDR4_USB_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR4_USB_M_U);
            postDivShift = CCM_PDR4_USB_DIV_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR4_USB_DIV);
            pPDR = &g_pCCM->PDR4;
        }
        break;

    case DDK_CLOCK_SIGNAL_UART:
        if (g_SiRev == DDK_SI_REV_TO1)
        {
            srcShift = CCM_PDR3_UART_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR3_UART_M_U);
            preDivShift = CCM_PDR4_UART_DIV_PRE_LSH;
            preDivMask = CSP_BITFMASK(CCM_PDR4_UART_DIV_PRE);
            postDivShift = CCM_PDR4_UART_DIV_PST_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR4_UART_DIV_PST);
            pPDR = &g_pCCM->PDR4;
            psrcPDR = &g_pCCM->PDR3;
        }
        else
        {
            srcShift = CCM_PDR3_UART_M_U_LSH;
            srcMask = CSP_BITFMASK(CCM_PDR3_UART_M_U);
            postDivShift = CCM_PDR4_UART_DIV_LSH;
            postDivMask = CSP_BITFMASK(CCM_PDR4_UART_DIV);
            pPDR = &g_pCCM->PDR4;
            psrcPDR = &g_pCCM->PDR3;
        }
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

        // The src pll and divider setting bits are in the same one PDR
        if (pPDR && !psrcPDR)
        {
            srcPll = src << srcShift;
            srcPll &= srcMask;
            preDiv <<= preDivShift;
            preDiv &= preDivMask;
            postDiv <<= postDivShift;
            postDiv &= postDivMask;
            do
            {
                oldReg = INREG32(pPDR);
                newReg = (oldReg & (~(srcMask | preDivMask | postDivMask))) 
                    | (srcPll) | (preDiv) | (postDiv);
            } while ((UINT32) InterlockedTestExchange((LPLONG) pPDR, 
                               oldReg, newReg) != oldReg);        
        }
        // The src pll setting bit locates in another PDR
        else if (pPDR && psrcPDR)
        {
            // Update dividers firstly
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

            // Update src pll bit then
            srcPll = src << srcShift;
            srcPll &= srcMask;
            do
            {
                oldReg = INREG32(psrcPDR);
                newReg = (oldReg & (~srcMask)) | (srcPll);
            } while ((UINT32) InterlockedTestExchange((LPLONG) psrcPDR, 
                               oldReg, newReg) != oldReg);        
        }
    }

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function: DDKClockConfigACM
//
// Configures the Audio Clock Mux (ACM) source selections for the specified
// Audio module.
//
// Parameters:
//      module
//          [in] Audio module to configure.
//
//      src
//          [in] Selects the ACM source.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockConfigACM(DDK_CLOCK_ACM module, DDK_CLOCK_ACM_SRC src)
{
    UINT32 oldReg, newReg, *pReg;

    pReg = &g_pCCM->ACMR;
    do
    {
        oldReg = INREG32(pReg);
        newReg = (oldReg & (~(0xf << module))) | (src << module);
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldReg, newReg) != oldReg);

    return TRUE;
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
//      div1
//          [in] Specifies the CKO_DIV1, either 0 or 1.
//
//      preDiv
//          [in] Specifies the CKO_DIV[5:3].
//
//      postDiv
//          [in] Specifies the CKO_DIV[2:0].
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetCKO(BOOL bEnable, DDK_CLOCK_CKO_SRC src, UINT32 div1, 
    UINT32 preDiv, UINT32 postDiv)
{
    UINT32 oldReg, newReg, *pReg;
    UINT32 mask, val;

    mask = CSP_BITFMASK(CCM_COSR_CLKOSEL) | 
        CSP_BITFMASK(CCM_COSR_CLKOEN) | 
        CSP_BITFMASK(CCM_COSR_CLKO_DIV1) | 
        CSP_BITFMASK(CCM_COSR_CLKO_DIV) | 
        CSP_BITFMASK(CCM_COSR_CLKO_DIV_PRE);

    val = (src << CCM_COSR_CLKOSEL_LSH) | 
        (bEnable ? (1 << CCM_COSR_CLKOEN_LSH) : 0) | 
        (div1 << CCM_COSR_CLKO_DIV1_LSH) | 
        (postDiv << CCM_COSR_CLKO_DIV_LSH) | 
        (preDiv << CCM_COSR_CLKO_DIV_PRE_LSH);

    pReg = &g_pCCM->COSR;
    do
    {
        oldReg = INREG32(pReg);
        newReg = (oldReg & (~mask)) | val;
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldReg, newReg) != oldReg);        

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

