//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
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

#include "bsp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables
extern PDDK_CLK_CONFIG g_pDdkClkConfig;
extern PCSP_CRM_REGS g_pCRM;


//-----------------------------------------------------------------------------
// Defines
#define CGCR0_UPLL_CLOCK_MASK         0x1000ffff

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
static BSP_ARGS *g_pBspArgs = NULL;

static HANDLE g_hDvfcWorkerEvent = NULL;
static HANDLE g_hSetPointEvent = NULL;


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function:  BSPClockDealloc
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
BOOL BSPClockDealloc(void)
{
    // Unmap peripheral address space
    if (g_pBspArgs)
    {
        MmUnmapIoSpace(g_pBspArgs, sizeof(BSP_ARGS));
        g_pBspArgs = NULL;
    }

    if (g_pDdkClkConfig)
    {
        MmUnmapIoSpace(g_pDdkClkConfig, sizeof(DDK_CLK_CONFIG));
        g_pDdkClkConfig = NULL;
    }

    if (g_hDvfcWorkerEvent != NULL)
    {
        CloseHandle(g_hDvfcWorkerEvent);
        g_hDvfcWorkerEvent = NULL;
    }

    if (g_hSetPointEvent != NULL)
    {
        CloseHandle(g_hSetPointEvent);
        g_hSetPointEvent = NULL;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  BSPClockAlloc
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
BOOL BSPClockAlloc(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;
         
    if (g_pBspArgs == NULL)
    {
        phyAddr.QuadPart = IMAGE_SHARE_ARGS_RAM_PA_START;

        // Map peripheral physical address to virtual address
        g_pBspArgs = (BSP_ARGS *) MmMapIoSpace(phyAddr, sizeof(BSP_ARGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pBspArgs == NULL)
        {
            DBGCHK((_T("CSPDDK")),  FALSE);
            ERRORMSG(TRUE, (_T("BSPClockAlloc:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

    }

    if (g_pDdkClkConfig == NULL)
    {
        phyAddr.QuadPart = IMAGE_WINCE_DDKCLK_RAM_PA_START;

        // Map peripheral physical address to virtual address
        g_pDdkClkConfig = (PDDK_CLK_CONFIG) MmMapIoSpace(phyAddr, sizeof(DDK_CLK_CONFIG),
            FALSE);

        // Check if virtual mapping failed
        if (g_pDdkClkConfig == NULL)
        {
            DBGCHK((_T("CSPDDK")),  FALSE);
            ERRORMSG(TRUE, (_T("BSPClockAlloc:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

        if (g_pCRM == NULL)
        {
            ERRORMSG(TRUE, (_T("BSPClockAlloc: should map g_pCRM first!\r\n")));
            goto cleanUp;
        }
    }

    // Create event to sync with DVFC setpoint transitions
    if (g_hDvfcWorkerEvent == NULL)
    {
        g_hDvfcWorkerEvent = CreateEvent(NULL, FALSE, FALSE, L"EVENT_DVFC_WORKER");

        if (g_hDvfcWorkerEvent == NULL)
        {
            ERRORMSG(1, (_T("CreateEvent failed!\r\n")));
            goto cleanUp;
        }
    }

    // Create event to sync with DVFC setpoint transitions
    if (g_hSetPointEvent == NULL)
    {
        g_hSetPointEvent = CreateEvent(NULL, TRUE, FALSE, L"EVENT_SETPOINT");

        if (g_hSetPointEvent == NULL)
        {
            ERRORMSG(1, (_T("CreateEvent failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;
     
cleanUp:

    if (!rc) BSPClockDealloc();

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function: BSPClockGetFreq
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
BOOL BSPClockGetFreq(DDK_CLOCK_SIGNAL sig, UINT32 *freq)
{
    *freq = g_pBspArgs->clockFreq[sig];

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: BSPClockUpdateFreq
//
// Updates the clock frequency in Hz for the specified clock signal.
//
// Parameters:
//      sig
//           [in] Clock signal.
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
BOOL BSPClockUpdateFreq(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src, 
                        UINT32 divider)
{
    BOOL rc = TRUE;

    if (!(sig <= DDK_CLOCK_SIGNAL_START_PER_CLK || sig >= DDK_CLOCK_SIGNAL_ENUM_END))
    {
        UINT32 srcFreq;

        switch (src)
        {
        case DDK_CLOCK_BAUD_SOURCE_AHB:
            srcFreq = g_pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB];
            break;

        case DDK_CLOCK_BAUD_SOURCE_USBPLL:
            srcFreq = g_pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_USBPLL];
            break;

        default:
            srcFreq = 0;
            rc = FALSE;
        }

        if (rc)
        {
            g_pBspArgs->clockFreq[sig] = srcFreq / (divider+1);
        }
    } 
    else if ((sig == DDK_CLOCK_SIGNAL_USBCLK) && (src == DDK_CLOCK_BAUD_SOURCE_USBPLL))
    {
        g_pBspArgs->clockFreq[sig] = g_pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_USBPLL] / (divider+1);
    }
    else
    {
        rc = FALSE;
    }
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPClockSetGatingModePrepare
//
//  This function is called prior to updating the clock gating mode of 
//  peripherals to determine if the system is currently at a voltage/frequency
//  setpoint that is sufficient to support the peripheral.  If it is
//  determined that the current setpoint is not sufficient, then a request
//  to raise the setpoint is submitted to the DVFC driver.
//
//  Parameters:
//      index
//           [in] Index for referencing the peripheral clock gating control 
//           bits.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPClockSetGatingModePrepare(DDK_CLOCK_GATE_INDEX index)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(index);
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPClockSetGatingModeComplete
//
//  Scans the clock tree to determine the usage of EMI clock and PERCLK domains.
//  Configures the system for EMI clock gating when possible.  Updates global
//  data structure shared with DVFC driver to determine when power state 
//  transistions that effect system clocking are possible.
//
//  Parameters:
//      index
//           [in] Index for referencing the peripheral clock gating control 
//           bits.
//
//      mode
//           [in] Requested clock gating mode for the peripheral.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPClockSetGatingModeComplete(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode)
{
    UINT32 cgcr0;
    UINT32 cctl;
    BOOL bUsbPllEn;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(mode);
    
    cgcr0 = INREG32(&g_pCRM->CGR_REGS.CGR[0]);

    // Attempt to disable UPLL if all modules clocked from
    // it are inactive.
    if ((cgcr0 & CGCR0_UPLL_CLOCK_MASK)==0)
    {
        // Assume UPLL is not needed
        bUsbPllEn = FALSE;
    }
    else 
    {
        bUsbPllEn = TRUE;
    }
    
    cctl = INREG32(&g_pCRM->CCTL);

    // If clock tree scan found a module that needs UPLL
    if (bUsbPllEn)
    {
        // If UPLL is not already enabled
        if (CSP_BITFEXT(cctl, CRM_CCTL_UPLL_DIS) == 1)
        {
            // Turn on UPLL
            CSP_BITFINS(cctl, CRM_CCTL_UPLL_DIS, 0);
            OUTREG32(&g_pCRM->CCTL, cctl);

            // Lock time is ~100us
            StallExecution(100);
        }
    }

    // Else clock tree scan did not find any module that needs UPLL
    else
    {
        // If UPLL is not already disabled
        if (CSP_BITFEXT(cctl, CRM_CCTL_UPLL_DIS) != 1)
        {
            // Turn off UPLL
            CSP_BITFINS(cctl, CRM_CCTL_UPLL_DIS, 1);
            OUTREG32(&g_pCRM->CCTL, cctl);
        }
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPClockSetpointRequest
//
//  Requests the specified setpoint optionally blocks until the setpoint
//  is granted.
//
//  Parameters:
//      setpoint
//          [in] - Specifies the setpoint to be requested.
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
BOOL BSPClockSetpointRequest(DDK_DVFC_SETPOINT setpoint, BOOL bBlock)
{
    if (setpoint > g_pDdkClkConfig->setpointMin[DDK_DVFC_DOMAIN_CPU]) 
        return FALSE;

    if (setpoint != g_pDdkClkConfig->setpointMin[DDK_DVFC_DOMAIN_CPU])
    {
        InterlockedIncrement((LPLONG) &g_pDdkClkConfig->setpointReqCount[DDK_DVFC_DOMAIN_CPU][setpoint]);

        // If caller wants to block until setpoint is granted
        if (bBlock && g_pDdkClkConfig->bDvfcActive)
        {
            // Until the current setpoint is sufficient
            while ((g_pDdkClkConfig->setpointCur[DDK_DVFC_DOMAIN_CPU] > setpoint) || 
                   g_pDdkClkConfig->bSetpointPending)
            {
                ResetEvent(g_hSetPointEvent);

                // Signal the DVFC driver about our request
                SetEvent(g_hDvfcWorkerEvent);

                // Wait for setpoint to be granted by the DVFC driver
                WaitForSingleObject(g_hSetPointEvent, INFINITE);
            }
        }
        else
        {
            // Signal the DVFC driver about our request
            SetEvent(g_hDvfcWorkerEvent);
        }

    }
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: BSPClockSetpointRelease
//
//  Releases a setpoint previously requested using DDKClockSetpointRequest.
//
//  Parameters:
//      setpoint
//          [in] - Specifies the setpoint to be released.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPClockSetpointRelease(DDK_DVFC_SETPOINT setpoint)
{
    if (setpoint > g_pDdkClkConfig->setpointMin[DDK_DVFC_DOMAIN_CPU]) 
        return FALSE;

    if (setpoint != g_pDdkClkConfig->setpointMin[DDK_DVFC_DOMAIN_CPU])
    {
        InterlockedDecrement((LPLONG) &g_pDdkClkConfig->setpointReqCount[DDK_DVFC_DOMAIN_CPU][setpoint]);
        // Signal the DVFC driver about our request
        SetEvent(g_hDvfcWorkerEvent);
    }
    return TRUE;
}

