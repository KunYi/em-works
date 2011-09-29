//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
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
extern BOOL BSPClockSetFreq(DDK_CLOCK_SIGNAL sig, UINT32 freq);
extern BOOL BSPClockUpdateFreq(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src,
                               UINT32 preDiv, UINT32 postDiv);
extern BOOL BSPClockSetGatingModePrepare(DDK_CLOCK_GATE_INDEX index);
extern BOOL BSPClockSetGatingModeComplete(DDK_CLOCK_GATE_INDEX index, 
                                          DDK_CLOCK_GATE_MODE mode);
extern BOOL BSPClockSetpointRequest(DDK_DVFC_SETPOINT setpoint, 
                                    DDK_DVFC_DOMAIN domain, BOOL bBlock);
extern BOOL BSPClockSetpointRelease(DDK_DVFC_SETPOINT setpoint, 
                                    DDK_DVFC_DOMAIN domain);
extern VOID BSPClockRootEnable(DDK_CLOCK_BAUD_SOURCE root);
extern VOID BSPClockRootDisable(DDK_CLOCK_BAUD_SOURCE root);
extern VOID BSPClockPowerUp(DDK_CLOCK_GATE_INDEX index);
extern VOID BSPClockPowerDown(DDK_CLOCK_GATE_INDEX index);


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
CRITICAL_SECTION g_csDdkClk;
HANDLE g_hDvfcWorkerEvent;
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

    // Create critical section for safe access to shared CCM registers and CSPDDK
    // data structures
    InitializeCriticalSection(&g_csDdkClk);

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

    if (g_hDvfcWorkerEvent)
    {
        CloseHandle(g_hDvfcWorkerEvent);
        g_hDvfcWorkerEvent = NULL;
    }

    DeleteCriticalSection(&g_csDdkClk);

    return BSPClockDealloc();

}


//-----------------------------------------------------------------------------
//
// Function: ClockUpdateRoot
//
// Updates references to the root clock of a specified clock node.
//
// Parameters:
//      index
//           [in] Index of clock node for which the root reference will be
//                updated.
//
//      root
//           [in] Root for the specified clock node.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID ClockUpdateRoot(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_BAUD_SOURCE root)
{
    DDK_CLOCK_BAUD_SOURCE oldRoot;
    UINT32 nodeRefCnt;
    
    EnterCriticalSection(&g_csDdkClk);

    oldRoot = g_pDdkClkConfig->clkTreeNode[index].root;
    nodeRefCnt = g_pDdkClkConfig->clkRefInfo[index].refCount;
    
    // Check if clock node has open references.
    if (nodeRefCnt != 0)
    {
        // Reduce old root clock references using the amount of 
        // open references for the specified clock node.  Make
        // sure ref count does not go negative.
        if (g_pDdkClkConfig->rootRefCount[oldRoot] >= nodeRefCnt)
        {
            g_pDdkClkConfig->rootRefCount[oldRoot] -= nodeRefCnt;
        }
        else
        {
            ERRORMSG(TRUE, (_T("ClockUpdateRoot:  Node references exceed root references.\r\n")));
            g_pDdkClkConfig->rootRefCount[oldRoot] = 0;
        }
        
        // Transfer clock references to new clock root.
        g_pDdkClkConfig->rootRefCount[root] += nodeRefCnt;
    }

    g_pDdkClkConfig->clkTreeNode[index].root = root;

    LeaveCriticalSection(&g_csDdkClk);    
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
    DDK_CLOCK_GATE_INDEX leaf;
    DDK_CLOCK_BAUD_SOURCE root;
    UINT32 i;
    BOOL bManageLeaf = FALSE;
    
    EnterCriticalSection(&g_csDdkClk);
    
    if ((mode == DDK_CLOCK_GATE_MODE_DISABLED) ||
        (mode == DDK_CLOCK_GATE_MODE_POWER_OFF))
    {        
        switch (g_pDdkClkConfig->clkRefInfo[index].refCount)
        {
        case 0:
            break;

        case 1:
            // Remove reference to root clock
            root = g_pDdkClkConfig->clkTreeNode[index].root;
            switch(g_pDdkClkConfig->rootRefCount[root])
            {
            case 0:
                break;

            case 1:
                BSPClockRootDisable(root);
                // Fall through

            default:
                --g_pDdkClkConfig->rootRefCount[root];
                break;
            }
    
            // Power down before disabling clocks so that state retention
            // logic has clocks
            if (mode == DDK_CLOCK_GATE_MODE_POWER_OFF)
            {
                BSPClockPowerDown(index);
                mode = DDK_CLOCK_GATE_MODE_DISABLED;
            }
            
            // Disable the clock
            INSREG32(&g_pCCM->CCGR[CCM_CGR_INDEX(index)],
                     CCM_CGR_MASK(index),
                     CCM_CGR_VAL(index, g_pDdkClkConfig->clkRefInfo[index].disabledMode));

            // Set flag to manage leaf nodes
            bManageLeaf = TRUE;

            // Fall through
            
        default:
            --g_pDdkClkConfig->clkRefInfo[index].refCount;
            break;
        }
    }
    else
    {        
        switch (g_pDdkClkConfig->clkRefInfo[index].refCount)
        {
        case 0:
            // If clock is being enabled for the first time, call down to platform 
            // code to check if we need to block until a system setpoint that can 
            // support this clock mode is available.  Platform code must
            // release DDK_CLK critical section while blocking on setpoint
            // request.
            BSPClockSetGatingModePrepare(index);
            
            // Add reference to root clock
            root = g_pDdkClkConfig->clkTreeNode[index].root;
            switch(g_pDdkClkConfig->rootRefCount[root])
            {
            case 0:
                BSPClockRootEnable(root);
                // Fall through

            default:
                ++g_pDdkClkConfig->rootRefCount[root];
                break;
            }

            // Enable the clock
            INSREG32(&g_pCCM->CCGR[CCM_CGR_INDEX(index)],
                     CCM_CGR_MASK(index),
                     CCM_CGR_VAL(index, mode));

            // Power up after enabling clocks so that state retention
            // logic has clocks
            BSPClockPowerUp(index);
            
            bManageLeaf = TRUE;
            // Fall through
            
        default:
            // Increment reference count
            ++g_pDdkClkConfig->clkRefInfo[index].refCount;
            break;
        }
    }

    // Leaf node clock management
    if (bManageLeaf)
    {
        for (i = 0; i < g_pDdkClkConfig->clkTreeNode[index].numLeaf; i++)
        {
            leaf = g_pDdkClkConfig->clkTreeNode[index].leaf[i];
            
            if (mode == DDK_CLOCK_GATE_MODE_DISABLED)
            {
                switch (g_pDdkClkConfig->clkRefInfo[leaf].refCount)
                {
                case 0:
                    break;

                case 1:
                    INSREG32(&g_pCCM->CCGR[CCM_CGR_INDEX(leaf)],
                             CCM_CGR_MASK(leaf),
                             CCM_CGR_VAL(leaf, g_pDdkClkConfig->clkRefInfo[leaf].disabledMode));

                    // Fall through
                    
                default:
                    --g_pDdkClkConfig->clkRefInfo[leaf].refCount;
                    break;
                }
            }
            else
            {        
                ++g_pDdkClkConfig->clkRefInfo[leaf].refCount;
                INSREG32(&g_pCCM->CCGR[CCM_CGR_INDEX(leaf)],
                         CCM_CGR_MASK(leaf),
                         CCM_CGR_VAL(leaf, g_pDdkClkConfig->clkRefInfo[leaf].enabledMode));
            }
        }
        
        // Allow platform code to complete the clock gating request
        BSPClockSetGatingModeComplete(index, mode);
    }

    LeaveCriticalSection(&g_csDdkClk);
    
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

    *pMode = ((INREG32(&g_pCCM->CCGR[mcgrIndex])) >> mcgrShift)
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
// Function: DDKClockSetFreq
//
// Sets the clock frequency in Hz for the specified clock signal.
//
// Parameters:
//      sig
//           [in] Clock signal.
//
//      freq
//           [in] Requested frequency in Hz.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetFreq(DDK_CLOCK_SIGNAL sig, UINT32 freq)
{
    return BSPClockSetFreq(sig, freq);
}


//-----------------------------------------------------------------------------
//
// Function: DDKClockConfigBaud
//
// Configures the input source clock and dividers for the specified
// CCM serial peripheral baud clock output.
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

    UINT32 oldReg, newReg, *pSelReg = NULL, *pDivReg = NULL;
    UINT32 selShift = 0;
    UINT32 selMask = 0;
    UINT32 preDivShift = 0;
    UINT32 preDivMask = 0;
    UINT32 postDivShift = 0;
    UINT32 postDivMask = 0;
    UINT32 selVal = src;
    UINT32 selVal2 = 0;
    DDK_CLOCK_BAUD_SOURCE root;

    switch(sig)
    {      
    case DDK_CLOCK_SIGNAL_USBOH3:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_LP_APM)
        {
            rc = FALSE;
            break;
        }

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_USBOH3_60M, src);
        
        selShift = CCM_CSCMR1_USBOH3_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_USBOH3_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;
        
        preDivShift = CCM_CSCDR1_USBOH3_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR1_USBOH3_CLK_PRED);
        postDivShift = CCM_CSCDR1_USBOH3_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CSCDR1_USBOH3_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR1;
        break;   
        
    case DDK_CLOCK_SIGNAL_ESDHC1:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_LP_APM)
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_ESDHC1_PERCLK, src);

        selShift = CCM_CSCMR1_ESDHC1_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_ESDHC1_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;
        
        preDivShift = CCM_CSCDR1_ESDHC1_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR1_ESDHC1_CLK_PRED);
        postDivShift = CCM_CSCDR1_ESDHC1_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CSCDR1_ESDHC1_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR1;
        break;
        
    case DDK_CLOCK_SIGNAL_ESDHC2:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_LP_APM)
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_ESDHC2_PERCLK, src);

        selShift = CCM_CSCMR1_ESDHC2_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_ESDHC2_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;
        
        preDivShift = CCM_CSCDR1_ESDHC2_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR1_ESDHC2_CLK_PRED);
        postDivShift = CCM_CSCDR1_ESDHC2_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CSCDR1_ESDHC2_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR1;
        break;
        
    case DDK_CLOCK_SIGNAL_ESDHC3:
        if(src == DDK_CLOCK_BAUD_SOURCE_ESDHC1)
        {
            selVal = 0;
            root = g_pDdkClkConfig->clkTreeNode[DDK_CLOCK_GATE_INDEX_ESDHC1_PERCLK].root;
        }    
        else if(src == DDK_CLOCK_BAUD_SOURCE_ESDHC2)
        {
            selVal = 1;
            root = g_pDdkClkConfig->clkTreeNode[DDK_CLOCK_GATE_INDEX_ESDHC2_PERCLK].root;
        }
        else 
        {
            rc = FALSE;
            break;
        }

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_ESDHC3_PERCLK, root);

        // There're no preDiv and postDiv for ESDHC3_CLK_ROOT
        selShift = CCM_CSCMR1_ESDHC3_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_ESDHC3_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;
        break;
        
    case DDK_CLOCK_SIGNAL_ESDHC4:
        if(src == DDK_CLOCK_BAUD_SOURCE_ESDHC1)
        {
            selVal = 0;
            root = g_pDdkClkConfig->clkTreeNode[DDK_CLOCK_GATE_INDEX_ESDHC1_PERCLK].root;
        }    
        else if(src == DDK_CLOCK_BAUD_SOURCE_ESDHC2)
        {
            selVal = 1;
            root = g_pDdkClkConfig->clkTreeNode[DDK_CLOCK_GATE_INDEX_ESDHC2_PERCLK].root;
        }
        else 
        {
            rc = FALSE;
            break;
        }

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_ESDHC4_PERCLK, root);
        
        // There're no preDiv and postDiv for ESDHC4_CLK_ROOT
        selShift = CCM_CSCMR1_ESDHC4_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_ESDHC4_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;
        break;        

    case DDK_CLOCK_SIGNAL_UART:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_LP_APM)
        {
            rc = FALSE;
            break;
        }

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_UART1_PERCLK, src);
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_UART2_PERCLK, src);
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_UART3_PERCLK, src);

        selShift = CCM_CSCMR1_UART_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_UART_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;

        preDivShift = CCM_CSCDR1_UART_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR1_UART_CLK_PRED);
        postDivShift = CCM_CSCDR1_UART_CLK_PODF_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR1_UART_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR1;
        break;

    case DDK_CLOCK_SIGNAL_SSI1:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_SSI_LP_APM)
        {
            rc = FALSE;
            break;
        }

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_SSI1_SSI, src);

        if(src == DDK_CLOCK_BAUD_SOURCE_SSI_LP_APM)
            selVal = 3;

        selShift = CCM_CSCMR1_SSI1_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_SSI1_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;

        preDivShift = CCM_CS1CDR_SSI1_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CS1CDR_SSI1_CLK_PRED);
        postDivShift = CCM_CS1CDR_SSI1_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CS1CDR_SSI1_CLK_PODF);
        pDivReg = &g_pCCM->CS1CDR;
        break;

    case DDK_CLOCK_SIGNAL_SSI2:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_SSI_LP_APM)
        {
            rc = FALSE;
            break;
        }

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_SSI2_SSI, src);

        if(src == DDK_CLOCK_BAUD_SOURCE_SSI_LP_APM)
            selVal = 3;

        selShift = CCM_CSCMR1_SSI2_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_SSI2_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;

        preDivShift = CCM_CS2CDR_SSI2_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CS2CDR_SSI2_CLK_PRED);
        postDivShift = CCM_CS2CDR_SSI2_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CS2CDR_SSI2_CLK_PODF);
        pDivReg = &g_pCCM->CS2CDR;
        break;

    case DDK_CLOCK_SIGNAL_SSI3:
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI1)
        {
            selVal = 0;
            root = g_pDdkClkConfig->clkTreeNode[DDK_CLOCK_GATE_INDEX_SSI1_SSI].root;
        }
        else if(src == DDK_CLOCK_BAUD_SOURCE_SSI2)
        {
            selVal = 1;
            root = g_pDdkClkConfig->clkTreeNode[DDK_CLOCK_GATE_INDEX_SSI2_SSI].root;
        }
        else
        {
            rc = FALSE;
            break;
        }

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_SSI3_SSI, root);

        selShift = CCM_CSCMR1_SSI3_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_SSI3_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;
        break;


    case DDK_CLOCK_SIGNAL_SSI_EXT1:
        root = src;
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI1)
        {
            selVal2 = 1; // Generate SSI_EXT1_CLK_ROOT from SSI1_CLK_ROOT
            root = g_pDdkClkConfig->clkTreeNode[DDK_CLOCK_GATE_INDEX_SSI1_SSI].root;
        }
        else if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
                src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_SSI_LP_APM)
        {
            rc = FALSE;
            break;
        }

        // Set the ssi_ext1_com
        do
        {
            oldReg = INREG32(&g_pCCM->CSCMR1);
            newReg = (oldReg & (~CSP_BITFMASK(CCM_CSCMR1_SSI_EXT1_COM))) |
                      CSP_BITFVAL(CCM_CSCMR1_SSI_EXT1_COM, selVal2);
        } while ((UINT32)InterlockedTestExchange((LPLONG)&g_pCCM->CSCMR1,
                           oldReg, newReg) != oldReg);

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_SSI_EXT1, root);
        
        // Check if selected src is SSI1_CLK_ROOT for SSI_EXT1_CLK_ROOT
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI1)
            break;

        // If selected src isn't SSI1_CLK_ROOT, need more settings
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI_LP_APM)
            selVal = 3;       

        selShift = CCM_CSCMR1_SSI_EXT1_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_SSI_EXT1_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;

        preDivShift = CCM_CS1CDR_SSI_EXT1_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CS1CDR_SSI_EXT1_CLK_PRED);
        postDivShift = CCM_CS1CDR_SSI_EXT1_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CS1CDR_SSI_EXT1_CLK_PODF);
        pDivReg = &g_pCCM->CS1CDR;
        break;

    case DDK_CLOCK_SIGNAL_SSI_EXT2:
        root = src;
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI2)
        {
            selVal2 = 1; // Generate SSI_EXT1_CLK_ROOT from SSI1_CLK_ROOT
            root = g_pDdkClkConfig->clkTreeNode[DDK_CLOCK_GATE_INDEX_SSI2_SSI].root;
        }
        else if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
                src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_SSI_LP_APM)
        {
            rc = FALSE;
            break;
        }

        // Set the ssi_ext2_com
        do
        {
            oldReg = INREG32(&g_pCCM->CSCMR1);
            newReg = (oldReg & (~CSP_BITFMASK(CCM_CSCMR1_SSI_EXT2_COM))) | 
                      CSP_BITFVAL(CCM_CSCMR1_SSI_EXT2_COM, selVal2);
        } while ((UINT32)InterlockedTestExchange((LPLONG)&g_pCCM->CSCMR1,
                          oldReg, newReg) != oldReg);

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_SSI_EXT2, root);
        
        // Check if selected src is SSI2_CLK_ROOT for SSI_EXT2_CLK_ROOT
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI2)
            break;

        // If selected src isn't SSI2_CLK_ROOT, need more settings
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI_LP_APM)
            selVal = 3;       

        selShift = CCM_CSCMR1_SSI_EXT2_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_SSI_EXT2_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;

        preDivShift = CCM_CS2CDR_SSI_EXT2_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CS2CDR_SSI_EXT2_CLK_PRED);
        postDivShift = CCM_CS2CDR_SSI_EXT2_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CS2CDR_SSI_EXT2_CLK_PODF);
        pDivReg = &g_pCCM->CS2CDR;
        break;

    case DDK_CLOCK_SIGNAL_USB_PHY:
        if(src == DDK_CLOCK_BAUD_SOURCE_PLL3)
            selVal2 = 1;
        else if(src != DDK_CLOCK_BAUD_SOURCE_OSC)
        {
            rc = FALSE;
            break; 
        }

        // Set the usb_phy_clk_sel
        do
        {
            oldReg = INREG32(&g_pCCM->CSCMR1);
            newReg = (oldReg & (~CSP_BITFMASK(CCM_CSCMR1_USB_PHY_CLK_SEL))) | 
                      CSP_BITFVAL(CCM_CSCMR1_USB_PHY_CLK_SEL, selVal2);
        } while ((UINT32)InterlockedTestExchange((LPLONG)&g_pCCM->CSCMR1,
                           oldReg, newReg) != oldReg);

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_USB_PHY, src);

        // Don't need more work if selected source is oscillator
        if(src == DDK_CLOCK_BAUD_SOURCE_OSC)
            break;

        preDivShift = CCM_CDCDR_USB_PHY_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CDCDR_USB_PHY_PRED);
        postDivShift = CCM_CDCDR_USB_PHY_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CDCDR_USB_PHY_PODF);
        pDivReg = &g_pCCM->CDCDR;
        break;

    case DDK_CLOCK_SIGNAL_TVE_216_54:
        if(src == DDK_CLOCK_BAUD_SOURCE_PLL3)
        {
            selVal = 0;
            selShift = CCM_CSCMR1_TVE_CLK_SEL_LSH;
            selMask = CSP_BITFMASK(CCM_CSCMR1_TVE_CLK_SEL);
            pSelReg = &g_pCCM->CSCMR1;

            preDivShift = CCM_CDCDR_TVE_CLK_PRED_LSH;
            preDivMask = CSP_BITFMASK(CCM_CDCDR_TVE_CLK_PRED);
            postDiv = 0;
            pDivReg = &g_pCCM->CDCDR;
            break;
        }
        else if(src == DDK_CLOCK_BAUD_SOURCE_CKIH_CAMP1)
            selVal2 = 1;
        else if(src != DDK_CLOCK_BAUD_SOURCE_OSC)
        {
            rc = FALSE;
            break; 
        } 

        // Set the tve_ext_clk_sel
        do
        {
            oldReg = INREG32(&g_pCCM->CSCMR1);
            newReg = (oldReg & (~CSP_BITFMASK(CCM_CSCMR1_TVE_EXT_CLK_SEL))) | 
                      CSP_BITFVAL(CCM_CSCMR1_TVE_EXT_CLK_SEL, selVal2);
        } while ((UINT32)InterlockedTestExchange((LPLONG)&g_pCCM->CSCMR1,
                           oldReg, newReg) != oldReg);

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_TVE, src);
        
        // If external clock is selected for TVE, 
        // Only mux but no divider for TVE clock
        selVal = 1;
        selShift = CCM_CSCMR1_TVE_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_TVE_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;
        break;        
        
    case DDK_CLOCK_SIGNAL_DI:
        if(src == DDK_CLOCK_BAUD_SOURCE_OSC)
            selVal2 = 1;
        else if(src == DDK_CLOCK_BAUD_SOURCE_CKIH_CAMP1)
            selVal2 = 2;
        else if(src == DDK_CLOCK_BAUD_SOURCE_TVE_DI)
            selVal2 = 3;
        else if(src == DDK_CLOCK_BAUD_SOURCE_IPP_DI)
            selVal2 = 4;
        else if(src != DDK_CLOCK_BAUD_SOURCE_PLL3)
        {
            rc =  FALSE;
            break;
        }

        // Set the di_clk_sel
        do
        {
            oldReg = INREG32(&g_pCCM->CSCMR2);
            newReg = (oldReg & (~CSP_BITFMASK(CCM_CSCMR2_DI_CLK_SEL))) | 
                      CSP_BITFVAL(CCM_CSCMR2_DI_CLK_SEL, selVal2);
        } while ((UINT32)InterlockedTestExchange((LPLONG)&g_pCCM->CSCMR2,
                           oldReg, newReg) != oldReg);

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_IPU_DI, src);
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_IPU_DI0, src);
        
        if (src == DDK_CLOCK_BAUD_SOURCE_OSC || 
        src == DDK_CLOCK_BAUD_SOURCE_CKIH_CAMP1 ||
        src == DDK_CLOCK_BAUD_SOURCE_TVE_DI ||
        src == DDK_CLOCK_BAUD_SOURCE_IPP_DI)
            break;
        
        preDivShift = CCM_CDCDR_DI_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CDCDR_DI_CLK_PRED);
        postDiv = 0;
        pDivReg = &g_pCCM->CDCDR;

        // No post divider for DI_CLK_ROOT
        break;         

    case DDK_CLOCK_SIGNAL_DI1:
        if(src == DDK_CLOCK_BAUD_SOURCE_OSC)
            selVal2 = 1;
        else if(src == DDK_CLOCK_BAUD_SOURCE_CKIH_CAMP1)
            selVal2 = 2;
        else if(src == DDK_CLOCK_BAUD_SOURCE_TVE_DI)
            selVal2 = 3;
        else if(src == DDK_CLOCK_BAUD_SOURCE_IPP_DI)
            selVal2 = 4;
        else if(src != DDK_CLOCK_BAUD_SOURCE_PLL3)
        {
            rc =  FALSE;
            break;
        }

        // Set the di_clk_sel
        do
        {
            oldReg = INREG32(&g_pCCM->CSCMR2);
            newReg = (oldReg & (~CSP_BITFMASK(CCM_CSCMR2_DI1_CLK_SEL))) | 
                      CSP_BITFVAL(CCM_CSCMR2_DI1_CLK_SEL, selVal2);
        } while ((UINT32)InterlockedTestExchange((LPLONG)&g_pCCM->CSCMR2,
                           oldReg, newReg) != oldReg);

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_IPU_DI1, src);
        
        if (src == DDK_CLOCK_BAUD_SOURCE_OSC || 
        src == DDK_CLOCK_BAUD_SOURCE_CKIH_CAMP1 ||
        src == DDK_CLOCK_BAUD_SOURCE_TVE_DI ||
        src == DDK_CLOCK_BAUD_SOURCE_IPP_DI)
            break;
        
        preDivShift = CCM_CDCDR_DI_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CDCDR_DI_CLK_PRED);
        postDiv = 0;
        pDivReg = &g_pCCM->CDCDR;

        // No post divider for DI_CLK_ROOT
        break;         

    case DDK_CLOCK_SIGNAL_VPU_RCLK:
        if(src == DDK_CLOCK_BAUD_SOURCE_CKIH_CAMP1)
            selVal2 = 1;
        else if(src != DDK_CLOCK_BAUD_SOURCE_OSC)
        {
            rc =  FALSE;
            break;
        }

        // Set the di_clk_sel
        do
        {
            oldReg = INREG32(&g_pCCM->CSCMR1);
            newReg = (oldReg & (~CSP_BITFMASK(CCM_CSCMR1_VPU_RCLK_ROOT))) | 
                      CSP_BITFVAL(CCM_CSCMR1_VPU_RCLK_ROOT, selVal2);
        } while ((UINT32)InterlockedTestExchange((LPLONG)&g_pCCM->CSCMR1,
                           oldReg, newReg) != oldReg);
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_VPU_REFERENCE, src);
        break;

    case DDK_CLOCK_SIGNAL_SPDIF0:
        root = src;
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI1)
        {
            selVal2 = 1; // Generate SPDIF0_CLK_ROOT from SSI1_CLK_ROOT
            root = g_pDdkClkConfig->clkTreeNode[DDK_CLOCK_GATE_INDEX_SSI1_SSI].root;
        }
        else if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
                src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_SPDIF_XTAL)
        {
            rc = FALSE;
            break;
        }

        // Set the spdif0_com
        do
        {
            oldReg = INREG32(&g_pCCM->CSCMR2);
            newReg = (oldReg & (~CSP_BITFMASK(CCM_CSCMR2_SPDIF0_COM))) | 
                CSP_BITFVAL(CCM_CSCMR2_SPDIF0_COM, selVal2);
        } while ((UINT32)InterlockedTestExchange((LPLONG)&g_pCCM->CSCMR2, 
                           oldReg, newReg) != oldReg);

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_SPDIF0, root);
        
        // Check if selected src is SSI1_CLK_ROOT for SPDIF0_CLK_ROOT
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI1)
            break;

        // If selected src isn't SSI1_CLK_ROOT, need more settings
        if(src == DDK_CLOCK_BAUD_SOURCE_SPDIF_XTAL)
            selVal = 3;       

        selShift = CCM_CSCMR2_SPDIF0_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR2_SPDIF0_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR2;

        preDivShift = CCM_CDCDR_SPDIF0_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CDCDR_SPDIF0_CLK_PRED);
        postDivShift = CCM_CDCDR_SPDIF0_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CDCDR_SPDIF0_CLK_PODF);
        pDivReg = &g_pCCM->CDCDR;
        break; 

    case DDK_CLOCK_SIGNAL_SPDIF1:
        root = src;
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI2)
        {
            selVal2 = 1; // Generate SPDIF1_CLK_ROOT from SSI2_CLK_ROOT
            root = g_pDdkClkConfig->clkTreeNode[DDK_CLOCK_GATE_INDEX_SSI2_SSI].root;
        }
        else if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
                src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_SPDIF_XTAL)
        {
            rc = FALSE;
            break;
        }

        // Set the spdif1_com
        do
        {
            oldReg = INREG32(&g_pCCM->CSCMR2);
            newReg = (oldReg & (~CSP_BITFMASK(CCM_CSCMR2_SPDIF1_COM))) | 
                      CSP_BITFVAL(CCM_CSCMR2_SPDIF1_COM, selVal2);
        } while ((UINT32)InterlockedTestExchange((LPLONG)&g_pCCM->CSCMR2,
                           oldReg, newReg) != oldReg);

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_SPDIF1, root);

        // Check if selected src is SSI2_CLK_ROOT for SPDIF1_CLK_ROOT
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI2)
            break;

        // If selected src isn't SSI2_CLK_ROOT, need more settings
        if(src == DDK_CLOCK_BAUD_SOURCE_SPDIF_XTAL)
            selVal = 3;       

        selShift = CCM_CSCMR2_SPDIF1_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR2_SPDIF1_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR2;

        preDivShift = CCM_CDCDR_SPDIF1_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CDCDR_SPDIF1_CLK_PRED);
        postDivShift = CCM_CDCDR_SPDIF1_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CDCDR_SPDIF1_CLK_PODF);
        pDivReg = &g_pCCM->CDCDR;
        break;

    case DDK_CLOCK_SIGNAL_SLIMBUS:
        root = src;
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI1)
        {
            selVal2 = 1; // Generate SLIMBUS_CLK_ROOT from SSI1_CLK_ROOT
            root = g_pDdkClkConfig->clkTreeNode[DDK_CLOCK_GATE_INDEX_SSI1_SSI].root;
        }
        else if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
                src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_CKIH &&
                src != DDK_CLOCK_BAUD_SOURCE_CKIH2)
        {
            rc = FALSE;
            break;
        }

        // Set the slimbus_com
        do
        {
            oldReg = INREG32(&g_pCCM->CSCMR2);
            newReg = (oldReg & (~CSP_BITFMASK(CCM_CSCMR2_SLIMBUS_COM))) | 
                      CSP_BITFVAL(CCM_CSCMR2_SLIMBUS_COM, selVal2);
        } while ((UINT32)InterlockedTestExchange((LPLONG)&g_pCCM->CSCMR2,
                           oldReg, newReg) != oldReg);

        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_SLIMBUS_SERIAL, root);

        // Check if selected src is SSI1_CLK_ROOT for SLIMBUS_CLK_ROOT
        if(src == DDK_CLOCK_BAUD_SOURCE_SSI1)
            break;

        // If selected src isn't SSI1_CLK_ROOT, need more settings
        if(src == DDK_CLOCK_BAUD_SOURCE_CKIH)
            selVal = 3;  
        else if(src == DDK_CLOCK_BAUD_SOURCE_CKIH2)
            selVal = 4; 

        selShift = CCM_CSCMR2_SLIMBUS_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR2_SLIMBUS_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR2;

        preDivShift = CCM_CSCDR2_SLIMBUS_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR2_SLIMBUS_CLK_PRED);
        postDivShift = CCM_CSCDR2_SLIMBUS_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CSCDR2_SLIMBUS_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR2;
        break;

    case DDK_CLOCK_SIGNAL_SIM:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_LP_APM)
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_SIM_SERIAL, src);
        
        selShift = CCM_CSCMR2_SIM_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR2_SIM_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR2;
        
        preDivShift = CCM_CSCDR2_SIM_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR2_SIM_CLK_PRED);
        postDivShift = CCM_CSCDR2_SIM_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CSCDR2_SIM_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR2;
        break;

    case DDK_CLOCK_SIGNAL_FIRI:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_LP_APM)
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_FIRI_SERIAL, src);
        
        selShift = CCM_CSCMR2_FIRI_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR2_FIRI_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR2;
        
        preDivShift = CCM_CSCDR3_FIRI_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR3_FIRI_CLK_PRED);
        postDivShift = CCM_CSCDR3_FIRI_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CSCDR3_FIRI_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR3;
        break;

    case DDK_CLOCK_SIGNAL_HSI2C:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_LP_APM)
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_HSI2C_SERIAL, src);
        
        selShift = CCM_CSCMR2_HSI2C_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR2_HSI2C_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR2;
        
        preDivShift = CCM_CSCDR3_HSI2C_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR3_HSI2C_CLK_PRED);
        postDivShift = CCM_CSCDR3_HSI2C_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CSCDR3_HSI2C_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR3;
        break;

    case DDK_CLOCK_SIGNAL_SSI_LP_APM:
        if(src == DDK_CLOCK_BAUD_SOURCE_LP_APM)
            selVal = 1;
        else if(src == DDK_CLOCK_BAUD_SOURCE_CKIH_CAMP1)
            selVal = 0;
        else if(src == DDK_CLOCK_BAUD_SOURCE_CKIH2_CAMP2)
            selVal = 2;
        else
        {
            rc =  FALSE;
            break;
        }
        selShift = CCM_CSCMR1_SSI_APM_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_SSI_APM_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;
        // No divider for SSI_LP_APM_CLK
        break; 

    case DDK_CLOCK_SIGNAL_SPDIF_XTAL:
        if(src == DDK_CLOCK_BAUD_SOURCE_OSC)
            selVal = 0;
        else if(src == DDK_CLOCK_BAUD_SOURCE_CKIH_CAMP1)
            selVal = 1;
        else if(src == DDK_CLOCK_BAUD_SOURCE_CKIH2_CAMP2)
            selVal = 2;
        else
        {
            rc =  FALSE;
            break;
        }
        selShift = CCM_CSCMR1_SPDIF_XTAL_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_SPDIF_XTAL_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;
        // No divider for SPDIF_XTAL_CLK
        break; 

    case DDK_CLOCK_SIGNAL_HSC1:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3)
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_HSC_HS1, src);
        
        selShift = CCM_CSCMR2_HSC1_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR2_HSC1_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR2;

        // preDiv  for HSC1_CLK_ROOT Control by periph dvfs
        preDiv = 0;
        postDivShift = CCM_CHSCCDR_HSC1_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CHSCCDR_HSC1_CLK_PODF);
        pDivReg = &g_pCCM->CHSCCDR;
        break;

    case DDK_CLOCK_SIGNAL_HSC2:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3)
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_HSC_HS2, src);
        
        selShift = CCM_CSCMR2_HSC2_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR2_HSC2_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR2;

        // preDiv  for HSC2_CLK_ROOT Control by periph dvfs
        preDiv = 0;
        postDivShift = CCM_CHSCCDR_HSC2_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CHSCCDR_HSC2_CLK_PODF);
        pDivReg = &g_pCCM->CHSCCDR;
        break;


    case DDK_CLOCK_SIGNAL_ESC:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_LP_APM)
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_HSC_ESC, src);
        
        selShift = CCM_CSCMR2_ESC_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR2_ESC_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR2;
        
        preDivShift = CCM_CHSCCDR_ESC_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CHSCCDR_ESC_CLK_PRED);
        postDivShift = CCM_CHSCCDR_ESC_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CHSCCDR_ESC_CLK_PODF);
        pDivReg = &g_pCCM->CHSCCDR;
        break;

    case DDK_CLOCK_SIGNAL_CSI_MCLK1:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3)
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_CSI_MCLK1, src);
        
        selShift = CCM_CSCMR2_CSI_MCLK1_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR2_CSI_MCLK1_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR2;
        
        preDivShift = CCM_CSCDR4_CSI_MCLK1_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR4_CSI_MCLK1_CLK_PRED);
        postDivShift = CCM_CSCDR4_CSI_MCLK1_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CSCDR4_CSI_MCLK1_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR4;
        break;

    case DDK_CLOCK_SIGNAL_CSI_MCLK2:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3)
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_CSI_MCLK2, src);

        selShift = CCM_CSCMR2_CSI_MCLK2_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR2_CSI_MCLK2_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR2;
        
        preDivShift = CCM_CSCDR4_CSI_MCLK2_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR4_CSI_MCLK2_CLK_PRED);
        postDivShift = CCM_CSCDR4_CSI_MCLK2_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CSCDR4_CSI_MCLK2_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR4;
        break;


    case DDK_CLOCK_SIGNAL_ECSPI:
        if(src != DDK_CLOCK_BAUD_SOURCE_PLL1 && src != DDK_CLOCK_BAUD_SOURCE_PLL2 &&
           src != DDK_CLOCK_BAUD_SOURCE_PLL3 && src != DDK_CLOCK_BAUD_SOURCE_LP_APM)
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_ECSPI1_PERCLK, src);
        ClockUpdateRoot(DDK_CLOCK_GATE_INDEX_ECSPI2_PERCLK, src);

        selShift = CCM_CSCMR1_ECSPI_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CSCMR1_ECSPI_CLK_SEL);
        pSelReg = &g_pCCM->CSCMR1;
        
        preDivShift = CCM_CSCDR2_ECSPI_CLK_PRED_LSH;
        preDivMask = CSP_BITFMASK(CCM_CSCDR2_ECSPI_CLK_PRED);
        postDivShift = CCM_CSCDR2_ECSPI_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CSCDR2_ECSPI_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR2;
        break;

        
    case DDK_CLOCK_SIGNAL_LPSR:
        if(src == DDK_CLOCK_BAUD_SOURCE_FPM)
            selVal = 1;
         else if(src == DDK_CLOCK_BAUD_SOURCE_FPM_DIV2)
            selVal = 2;       
        else if(src == DDK_CLOCK_BAUD_SOURCE_GND)
            selVal = 3;
        else
        {
            rc = FALSE;
            break;
        }
        // There're no PreDiv and PostDiv for LPSR_CLK_ROOT, 
        selShift = CCM_CLPCR_LPSR_CLK_SEL_LSH;
        selMask = CSP_BITFMASK(CCM_CLPCR_LPSR_CLK_SEL);
        pSelReg = &g_pCCM->CLPCR;
        break;
        
    case DDK_CLOCK_SIGNAL_PGC:
        if(src != DDK_CLOCK_BAUD_SOURCE_IPG)
        {
            rc = FALSE;
            break;
        }
        // There're no preDiv and mux for PGC_CLK_ROOG
        preDiv = 0;
        postDivShift = CCM_CSCDR1_PGC_CLK_PODF_LSH;
        postDivMask = CSP_BITFMASK(CCM_CSCDR1_PGC_CLK_PODF);
        pDivReg = &g_pCCM->CSCDR1;
        // For PGC_CLK_ROOT, the post divider mapping is
        //  postDiv     Divider value
        //    00 ---------> 1
        //    01 ---------> 2
        //    10 ---------> 4
        //    11 ---------> 8
        break;
        
    default:
        rc = FALSE;
        break;        
    }

    // If baud configuration is available
    if (rc)    
    {
        if(pSelReg)
        {
            selVal <<= selShift;
            
            do
            {
                oldReg = INREG32(pSelReg);
                newReg = (oldReg & (~selMask)) | selVal;
            } while ((UINT32) InterlockedTestExchange((LPLONG) pSelReg, 
                               oldReg, newReg) != oldReg);
        }

       // Update the dividers
        if (pDivReg)
        {
            // Update the global clock signal table
            BSPClockUpdateFreq(sig, src, preDiv, postDiv);
            
            preDiv <<= preDivShift;
            preDiv &= preDivMask;
            postDiv <<= postDivShift;
            postDiv &= postDivMask;
            do
            {
                oldReg = INREG32(pDivReg);
                newReg = (oldReg & (~(preDivMask | postDivMask))) 
                    | (preDiv) | (postDiv);
            } while ((UINT32)InterlockedTestExchange((LPLONG)pDivReg, 
                               oldReg, newReg) != oldReg);        
        }
        else
        {
            // Update the global clock signal table
            BSPClockUpdateFreq(sig, src, 0, 0);
        }
    }
    
    return rc;
    
}


//-----------------------------------------------------------------------------
//
// Function: DDKClockSetCKO1
//
// Configures the clock output source CKO1 signal.
//
// Parameters:
//      bEnable
//          [in] Set to TRUE to enable CKO1 output.  Set to FALSE to disable
//          CKO1 output.
//
//      index
//          [in] Selects the CKO1 source signal.
//
//      div
//          [in] Specifies the CKO1 divide factor.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetCKO1(BOOL bEnable, DDK_CLOCK_CKO1_SRC index, DDK_CLOCK_CKO_DIV div)
{
    UINT32 mask, data, oldVal, newVal, *pReg;

    mask = CSP_BITFMASK(CCM_CCOSR_CKO1_EN) |
           CSP_BITFMASK(CCM_CCOSR_CKO1_SEL) |
           CSP_BITFMASK(CCM_CCOSR_CKO1_DIV);
    data = CSP_BITFVAL(CCM_CCOSR_CKO1_EN, bEnable) |
           CSP_BITFVAL(CCM_CCOSR_CKO1_SEL, index)  |
           CSP_BITFVAL(CCM_CCOSR_CKO1_DIV, div);

    pReg = &g_pCCM->CCOSR;
    do
    {
        oldVal = INREG32(pReg);
        newVal = (oldVal & (~mask)) | data;
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldVal, newVal) != oldVal);
    
    return TRUE;
}



//-----------------------------------------------------------------------------
//
// Function: DDKClockSetCKO2
//
// Configures the clock output source CKO2 signal.
//
// Parameters:
//      bEnable
//          [in] Set to TRUE to enable CKO2 output.  Set to FALSE to disable
//          CKO1 output.
//
//      index
//          [in] Selects the CKO2 source signal.
//
//      div
//          [in] Specifies the CKO2 divide factor.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetCKO2(BOOL bEnable, DDK_CLOCK_CKO2_SRC index, DDK_CLOCK_CKO_DIV div)
{
    UINT32 mask, data, oldVal, newVal, *pReg;

    mask = CSP_BITFMASK(CCM_CCOSR_CKO2_EN) |
           CSP_BITFMASK(CCM_CCOSR_CKO2_SEL) |
           CSP_BITFMASK(CCM_CCOSR_CKO2_DIV);
    data = CSP_BITFVAL(CCM_CCOSR_CKO2_EN, bEnable) |
           CSP_BITFVAL(CCM_CCOSR_CKO2_SEL, index)  |
           CSP_BITFVAL(CCM_CCOSR_CKO2_DIV, div);

    pReg = &g_pCCM->CCOSR;
    do
    {
        oldVal = INREG32(pReg);
        newVal = (oldVal & (~mask)) | data;
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldVal, newVal) != oldVal);
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKClockSetpointRequest
//
//  Requests the specified setpoint optionally blocks until the setpoint
//  is granted.
//
//  Parameters:
//      setpoint
//          [in] - Specifies the setpoint to be requested.
//
//      domain
//          [in] - Specifies DVFC domain for which the setpoint will be 
//                 requested.
//
//      bBlock
//          [in] - Set TRUE to block until the setpoint has been granted.  
//                 Set FALSE to return immediately after request has
//                 submitted.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetpointRequest(DDK_DVFC_SETPOINT setpoint, 
                             DDK_DVFC_DOMAIN domain, BOOL bBlock)
{
    return BSPClockSetpointRequest(setpoint, domain, bBlock);
}


//-----------------------------------------------------------------------------
//
//  Function: DDKClockSetpointRelease
//
//  Releases a setpoint previously requested using DDKClockSetpointRequest.
//
//  Parameters:
//      setpoint
//          [in] - Specifies the setpoint to be released.
//
//      domain
//          [in] - Specifies DVFC domain for which the setpoint will be 
//                 released.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetpointRelease(DDK_DVFC_SETPOINT setpoint, 
                             DDK_DVFC_DOMAIN domain)
{
    return BSPClockSetpointRelease(setpoint, domain);
}

//-----------------------------------------------------------------------------
//
//  Function: DDKClockGetSharedConfig
//
//  Obtains a reference to the global shared clock configuration data 
//  structure.  This is intended to be used by the DVFC driver.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns a pointer to the clock configuration data structure.
//
//-----------------------------------------------------------------------------
PDDK_CLK_CONFIG DDKClockGetSharedConfig(VOID)
{
    return g_pDdkClkConfig;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKClockLock
//
//  Requests a lock of the global shared clock configuration data structure.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID DDKClockLock(VOID)
{
    EnterCriticalSection(&g_csDdkClk);
}


//-----------------------------------------------------------------------------
//
//  Function: DDKClockUnlock
//
//  Releases a lock of the global shared clock configuration data structure.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID DDKClockUnlock(VOID)
{
    LeaveCriticalSection(&g_csDdkClk);
}


//-----------------------------------------------------------------------------
//
//  Function: DDKClockSetOverride
//
//  Enable/disable the CMEOR bits to override the clock enable signal from 
//  the module
//
//  Parameters:
//      index
//          [in] - Specifies the clock enable signal to be override.
//
//      mode
//          [in] - Enable or disable the selected enable signal
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetOverride(DDK_CLOCK_OVERRIDE_ENABLE_INDEX index, DDK_CLOCK_OVERRIDE_MODE mode)
{
    UINT32 newVal, oldVal, *pReg;

    pReg = &g_pCCM->CMEOR;

    // Update the corresponding bit
    do
    {
        oldVal = INREG32(pReg);
        newVal = (oldVal & (~ CCM_CMEOR_MASK(index))) | CCM_CMEOR_VAL(index, mode);
    } while((UINT32) InterlockedTestExchange((LPLONG) pReg, oldVal, newVal) != oldVal);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKClockGetOverride
//
//  Retrieve the enable sigal override mode
//
//  Parameters:
//      index
//          [in] - Specifies the clock enable signal that will be retrieved.
//
//      pMode
//          [out] - Pointer to DDK_CLOCK_OVERRIDE_MODE that stores current mode
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockGetOverride(DDK_CLOCK_OVERRIDE_ENABLE_INDEX index, DDK_CLOCK_OVERRIDE_MODE *pMode)
{
    *pMode = ((INREG32(&g_pCCM->CMEOR))& CCM_CMEOR_MASK(index)) >> CCM_CMEOR_SHIFT(index);
    return TRUE;
}
