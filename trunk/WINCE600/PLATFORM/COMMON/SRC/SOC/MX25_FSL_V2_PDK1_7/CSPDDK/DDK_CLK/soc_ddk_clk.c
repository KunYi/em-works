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

extern BOOL BSPClockUpdateFreq(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src, UINT32 divider);
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
CRITICAL_SECTION g_csDdkClk;
PCSP_CRM_REGS g_pCRM = NULL;
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
//      Returns TRUE everytime.
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

    if (g_pCRM == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_CRM;

        // Map peripheral physical address to virtual address
        g_pCRM = (PCSP_CRM_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_CRM_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pCRM == NULL)
        {
            DBGCHK((_T("CSPDDK")),  FALSE);
            ERRORMSG(1, (_T("ClockAlloc:  MmMapIoSpace failed!\r\n")));
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
    if (g_pCRM)
    {
        MmUnmapIoSpace(g_pCRM, sizeof(CSP_CRM_REGS));
        g_pCRM = NULL;
    }

    DeleteCriticalSection(&g_csDdkClk);

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
    // Check if we are already at the required state 
    if ((DDK_CLOCK_GATE_MODE) EXTREG32(&g_pCRM->CGR_REGS.CGR[CRM_CGR_INDEX(index)], CRM_CGR_MASK(index), CRM_CGR_SHIFT(index)) == mode)
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

    // Grab lock before accessing/updating the shared CRM registers and
    // corresponding DVFC data structures
    EnterCriticalSection(&g_csDdkClk);

    INSREG32(&g_pCRM->CGR_REGS.CGR[CRM_CGR_INDEX(index)], CRM_CGR_MASK(index), CRM_CGR_VAL(index, mode));

    // Allow platform code to complete the clock gating request
    BSPClockSetGatingModeComplete(index, mode);

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
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE *pMode)
{
   *pMode = ((INREG32(&g_pCRM->CGR_REGS.CGR[CRM_CGR_INDEX(index)])) & CRM_CGR_MASK(index)) >> CRM_CGR_SHIFT(index);
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
// CRM peripheral baud clock output.
//
// Parameters:
//      sig
//          [in] Clock signal to configure.
//
//      src
//          [in] Selects the input clock source.
//
//      divider
//          [in] Specifies the value programmed into the baud clock divider.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockConfigBaud(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src,
                        UINT32 divider)
{
    BOOL rc = TRUE;

    if (!(sig <= DDK_CLOCK_SIGNAL_START_PER_CLK || sig >= DDK_CLOCK_SIGNAL_ENUM_END))
    {

        UINT32 mcrVal, mcrValOld;

        // Get current MCR value (current src clk value)
        mcrValOld = INREG32(&g_pCRM->MCR);

        // Activate the correct src clk
        switch (src)
        {
        case DDK_CLOCK_BAUD_SOURCE_AHB:
            mcrVal = mcrValOld & (~(CRM_MCR_PER_CLK_MUX_HCLK << DDK_INDEX_TO_CPU_INDEX(sig)));
            break;

        case DDK_CLOCK_BAUD_SOURCE_USBPLL:
            mcrVal = mcrValOld | ((CRM_MCR_PER_CLK_MUX_UPLL << DDK_INDEX_TO_CPU_INDEX(sig)));
            break;

        default:
            rc = FALSE;
            mcrVal = 0;
        }
        if (divider > 63)
        {
            rc = FALSE;
        }

        if (rc)
        {
            // Update the global clock signal table
            BSPClockUpdateFreq(sig, src, divider);

            // Write Misc register value to choose the clk source (USBPLL or AHB)
            OUTREG32(&g_pCRM->MCR, mcrVal);

            // Write correct PCDR register to set the divider
            INSREG32(&g_pCRM->PCDR_REGS.PCDR[CRM_PCDR_INDEX(DDK_INDEX_TO_CPU_INDEX(sig))]
            ,CRM_PCDR_MASK(DDK_INDEX_TO_CPU_INDEX(sig))
                ,((divider) << CRM_PCDR_SHIFT(DDK_INDEX_TO_CPU_INDEX(sig))));
        }

    } else if ((sig == DDK_CLOCK_SIGNAL_USBCLK) && (src == DDK_CLOCK_BAUD_SOURCE_USBPLL))
    {

        if (divider > 63)
        {
            rc = FALSE;
        }
        if (rc)
        {
            // Update the global clock signal table
            BSPClockUpdateFreq(sig, src, divider);

            // Write the new divider for the USB clock (in CCTL register)
            INSREG32(&g_pCRM->CCTL,CSP_BITFMASK(CRM_CCTL_USB_DIV),CSP_BITFVAL(CRM_CCTL_USB_DIV,divider));
        }
    }
    else
    {
        rc = FALSE;
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
//      divider
//          [in] Specifies the CKO_DIV, Value of divider - 1.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetCKO(BOOL bEnable, DDK_CLOCK_CKO_SRC src, UINT32 divider)
{
    UINT32 mask, val;
 
    mask = CSP_BITFMASK(CRM_MCR_CLKO_EN) | 
        CSP_BITFMASK(CRM_MCR_CLKO_DIV) | 
        CSP_BITFMASK(CRM_MCR_CLKO_SEL);

    val = (src << CRM_MCR_CLKO_SEL_LSH) | 
        (bEnable ? (CRM_MCR_CLKO_ENABLE << CRM_MCR_CLKO_EN_LSH) : CRM_MCR_CLKO_DISABLE) | 
        (divider << CRM_MCR_CLKO_DIV_LSH);

    INSREG32(&g_pCRM->MCR,mask,val);

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


