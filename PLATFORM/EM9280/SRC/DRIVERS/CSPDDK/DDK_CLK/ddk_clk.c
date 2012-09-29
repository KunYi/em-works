//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
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

#include "bsp.h"
#include "dvfs.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
static HANDLE g_hSetPointEvent = NULL;
static HANDLE g_hDvfcWorkerEvent = NULL;
#endif
PDDK_CLK_CONFIG g_pDdkClkConfig;

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
    if (g_pDdkClkConfig)
    {
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
        MmUnmapIoSpace(g_pDdkClkConfig, sizeof(DDK_CLK_CONFIG));
#endif
        g_pDdkClkConfig = NULL;
    }
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
    if (g_hSetPointEvent)
    {
        CloseHandle(g_hSetPointEvent);
        g_hSetPointEvent = NULL;
    }

    if (g_hDvfcWorkerEvent)
    {
        CloseHandle(g_hDvfcWorkerEvent);
        g_hDvfcWorkerEvent = NULL;
    }    
#endif

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
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
    PHYSICAL_ADDRESS phyAddr;
#endif
         
    if (g_pDdkClkConfig == NULL)
    {
#ifdef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
        // Map peripheral physical address to virtual address
        g_pDdkClkConfig = (PVOID) OALPAtoUA(IMAGE_WINCE_DDKCLK_RAM_PA_START);
#else    
        phyAddr.QuadPart = IMAGE_WINCE_DDKCLK_RAM_PA_START;

        // Map peripheral physical address to virtual address
        g_pDdkClkConfig = (PDDK_CLK_CONFIG) MmMapIoSpace(phyAddr, sizeof(DDK_CLK_CONFIG),
            FALSE);
#endif
        // Check if virtual mapping failed
        if (g_pDdkClkConfig == NULL)
        {
            DBGCHK((_T("CSPDDK")),  FALSE);
            ERRORMSG(TRUE, (_T("BSPClockAlloc:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

    }
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
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
#endif

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
    *freq = g_pDdkClkConfig->clockFreq[sig];

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
BOOL BSPClockUpdateFreq(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src, 
    UINT32 u32Div)
{
    BOOL rc = TRUE;   
    
    UINT32 srcFreq;

    if(sig == DDK_CLOCK_SIGNAL_H_CLK)
    {
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_H_CLK] = 
               g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_P_CLK] / u32Div ;
        return rc;
    } 

    switch (src)
    {
        case DDK_CLOCK_BAUD_SOURCE_PLL0:
            srcFreq = g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_PLL0];
            break;        
        case DDK_CLOCK_BAUD_SOURCE_REF_CPU:
            srcFreq = g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_CPU];
            break;           
        case DDK_CLOCK_BAUD_SOURCE_REF_EMI:
             srcFreq = g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_EMI];
            break;     
        case DDK_CLOCK_BAUD_SOURCE_REF_IO0:
             srcFreq = g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO0];
            break; 
        case DDK_CLOCK_BAUD_SOURCE_REF_IO1:
             srcFreq = g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO1];
            break;        
        case DDK_CLOCK_BAUD_SOURCE_REF_PIX:
             srcFreq = g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_PIX];
            break;
        case DDK_CLOCK_BAUD_SOURCE_REF_HSADC:
             srcFreq = g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_HSADC];
            break;
        case DDK_CLOCK_BAUD_SOURCE_REF_GPMI:
             srcFreq = g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_GPMI];
            break;        
        case DDK_CLOCK_BAUD_SOURCE_REF_PLL:
             srcFreq = g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_PLL];
            break;
        case DDK_CLOCK_BAUD_SOURCE_REF_XTAL:
             srcFreq = g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL];
            break;
        default:
            srcFreq = 0;
            rc = FALSE;
    }

    if (rc)
    {
        g_pDdkClkConfig->clockFreq[sig] = srcFreq / (u32Div);
    }

    return rc;
}

#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
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
    
    //RETAILMSG(TRUE, (_T("BSPClockSetpointRequest \r\n")));               

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

    //RETAILMSG(TRUE, (_T("BSPClockSetpointRelease \r\n")));   

    if (setpoint != g_pDdkClkConfig->setpointMin[DDK_DVFC_DOMAIN_CPU])
    {
        InterlockedDecrement((LPLONG) &g_pDdkClkConfig->setpointReqCount[DDK_DVFC_DOMAIN_CPU][setpoint]);
        // Signal the DVFC driver about our request
        SetEvent(g_hDvfcWorkerEvent);
    }
    return TRUE;
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
    //ENET, LCD, USB0 USB1
    if(index == DDK_CLOCK_GATE_DIS_LCDIF_CLK     ||
       index == DDK_CLOCK_GATE_UTMI0_CLK480M_CLK ||
       index == DDK_CLOCK_GATE_UTMI1_CLK480M_CLK ||
       index == DDK_CLOCK_GATE_ENET_CLK)
       //return TRUE;
       BSPClockSetpointRequest(DDK_DVFC_SETPOINT_HIGH, TRUE);

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
BOOL BSPClockSetGatingModeComplete(DDK_CLOCK_GATE_INDEX index, BOOL bClkGate)
{
    // If a clock is being disabled, release ENET, USB0,l USB1 and LCD med setpoint requried.
    if (bClkGate == TRUE)
    {
        if(index == DDK_CLOCK_GATE_DIS_LCDIF_CLK     ||
           index == DDK_CLOCK_GATE_UTMI0_CLK480M_CLK ||
           index == DDK_CLOCK_GATE_UTMI1_CLK480M_CLK ||
           index == DDK_CLOCK_GATE_ENET_CLK)
           //return TRUE;
           BSPClockSetpointRelease(DDK_DVFC_SETPOINT_HIGH);
    }
    
    return TRUE;
}


#endif

