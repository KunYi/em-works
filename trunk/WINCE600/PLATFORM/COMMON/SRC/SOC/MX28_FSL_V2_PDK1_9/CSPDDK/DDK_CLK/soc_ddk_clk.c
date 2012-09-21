//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_ddk_clk.c
//
//  This file contains the Clock generation and conrol DDK interface that is used
//  by applications and other drivers to access the capabilities of the clock driver.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#ifdef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
#include <oal.h>
#endif 
#pragma warning(pop)

#include "csp.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables
extern PDDK_CLK_CONFIG g_pDdkClkConfig;

//-----------------------------------------------------------------------------
// Defines
CRITICAL_SECTION g_csDdkClk;

#ifdef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
#define LOCK()
#define UNLOCK()
#else
#define LOCK()   EnterCriticalSection(&g_csDdkClk)
#define UNLOCK() LeaveCriticalSection(&g_csDdkClk)
#endif

#define MAX_CLKCTRL_DIS_LCDIF_DIV     255
#define MIN_CLKCTRL_PFD_VALUE         18
#define MAX_CLKCTRL_PFD_VALUE         35

//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
PVOID pv_HWregCLKCTRL = NULL;


//-----------------------------------------------------------------------------
// External Functions
extern BOOL BSPClockAlloc(void);
extern BOOL BSPClockDealloc(void);
extern BOOL BSPClockGetFreq(DDK_CLOCK_SIGNAL sig, UINT32 *freq);
extern BOOL BSPClockUpdateFreq(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src,UINT32 u32Div);
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
extern BOOL BSPClockSetGatingModePrepare(DDK_CLOCK_GATE_INDEX index);
extern BOOL BSPClockSetGatingModeComplete(DDK_CLOCK_GATE_INDEX index, 
                                          BOOL bClkGate);
extern BOOL BSPClockSetpointRequest(DDK_DVFC_SETPOINT setpoint, BOOL bBlock);
extern BOOL BSPClockSetpointRelease(DDK_DVFC_SETPOINT setpoint);
#endif


//-----------------------------------------------------------------------------
// Local Functions
BOOL ClockAlloc(void);
BOOL ClockDealloc(void);

//-----------------------------------------------------------------------------
//
// Function:  ClockAlloc
//
// This function allocates the data structures required for interaction
// with the Clock control module.
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
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
    PHYSICAL_ADDRESS phyAddr;
#endif
    if (pv_HWregCLKCTRL == NULL)
    {
#ifdef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
        // Map peripheral physical address to virtual address
        pv_HWregCLKCTRL = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_CLKCTRL);
#else
        // Create critical section for safe access to shared CCM registers and CSPDDK
        // data structures
        InitializeCriticalSection(&g_csDdkClk);

        phyAddr.QuadPart = CSP_BASE_REG_PA_CLKCTRL;

        // Map peripheral physical address to virtual address
        pv_HWregCLKCTRL = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);
#endif 
        // Check if virtual mapping failed
        if (pv_HWregCLKCTRL == NULL)
        {
            ERRORMSG(1, (_T("ClockAlloc::MmMapIoSpace failed!\r\n")));
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
// with the Clock control module.
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
    // Unmap peripheral registers
    if (pv_HWregCLKCTRL)
    {
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
        MmUnmapIoSpace(pv_HWregCLKCTRL, 0x1000);
#endif
        pv_HWregCLKCTRL = NULL;
    }
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
    DeleteCriticalSection(&g_csDdkClk);
#endif
    return BSPClockDealloc();
}

//-----------------------------------------------------------------------------
//
// Function: ClockSetGatingMode
//
// Sets the clock gating mode of the peripheral.
//
// Parameters:
//      index
//           [in] Index for referencing the peripheral clock gating control 
//           bits.
//
//      bClkGate
//           [in] Requested clock gating mode for the peripheral.
//
// Returns:
//      Returns TRUE if the clock gating mode was set successfully, otherwise 
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL ClockSetGating(DDK_CLOCK_GATE_INDEX index, BOOL bClkGate)
{
    BOOL rc = TRUE;
    switch(index)
    {
        case DDK_CLOCK_GATE_UART_CLK: 
            if(bClkGate) 
            {
                HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_UART_CLK_GATE);
            }
            else
            {
                HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_UART_CLK_GATE);
            }
            break;
            
        case DDK_CLOCK_GATE_PWM24M_CLK:   
            if(bClkGate) 
            {
                HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_PWM_CLK24M_GATE);
            }
            else
            {
                HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_PWM_CLK24M_GATE);
            }
            break;
            
        case DDK_CLOCK_GATE_TIMROT32K_CLK:
            if(bClkGate) 
            {
                HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_TIMROT_CLK32K_GATE);
            }
            else
            {
                HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_TIMROT_CLK32K_GATE);
            }
            break;

        case DDK_CLOCK_GATE_UTMI0_CLK480M_CLK:
            if( bClkGate )
            {
                HW_CLKCTRL_PLL0CTRL0_CLR(BM_CLKCTRL_PLL0CTRL0_EN_USB_CLKS);
            }
            else
            {
                HW_CLKCTRL_PLL0CTRL0_SET(BM_CLKCTRL_PLL0CTRL0_EN_USB_CLKS);
            }
            break;

        case DDK_CLOCK_GATE_UTMI1_CLK480M_CLK:
            if( bClkGate )
            {
                HW_CLKCTRL_PLL1CTRL0_CLR(BM_CLKCTRL_PLL1CTRL0_EN_USB_CLKS);
            }
            else
            {
                HW_CLKCTRL_PLL1CTRL0_SET(BM_CLKCTRL_PLL1CTRL0_EN_USB_CLKS);
            }
            break;

        case DDK_CLOCK_GATE_HSADC_CLK:
           //Do nothing here
            break;
    
        case DDK_CLOCK_GATE_SSP0_CLK:  
            HW_CLKCTRL_SSP0.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SSP1_CLK:  
            HW_CLKCTRL_SSP1.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SSP2_CLK:  
            HW_CLKCTRL_SSP2.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SSP3_CLK:  
            HW_CLKCTRL_SSP3.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_GPMI_CLK :  
            HW_CLKCTRL_GPMI.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SPDIF_CLK:  
            HW_CLKCTRL_SPDIF.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SAIF0_CLK:       
            HW_CLKCTRL_SAIF0.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SAIF1_CLK:       
            HW_CLKCTRL_SAIF1.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_DIS_LCDIF_CLK  :        
            HW_CLKCTRL_DIS_LCDIF.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_ETM_CLK:  
            HW_CLKCTRL_ETM.B.CLKGATE = bClkGate ? 1 : 0;
            break;   
            
        case DDK_CLOCK_GATE_ENET_CLK  :  
            HW_CLKCTRL_ENET.B.SLEEP = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_FLEXCAN0_CLK  :  
            HW_CLKCTRL_FLEXCAN.B.STOP_CAN0 = bClkGate ? 1 : 0;
            break;   
            
        case DDK_CLOCK_GATE_FLEXCAN1_CLK  :  
           HW_CLKCTRL_FLEXCAN.B.STOP_CAN1 = bClkGate ? 1 : 0;
           break; 
           
        default:
            rc = FALSE;
            break; 
    }
    return rc;
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
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
    EnterCriticalSection(&g_csDdkClk);
#endif
    oldRoot = g_pDdkClkConfig->root[index];
    nodeRefCnt = g_pDdkClkConfig->clockRefCount[index];
    
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

    g_pDdkClkConfig->root[index] = root;
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
    LeaveCriticalSection(&g_csDdkClk);   
#endif
}



//-----------------------------------------------------------------------------
//
//  Function: ClockRootEnable
//
//  This function enables the specified root clock.
//
//  Parameters:
//      root
//           [in] Index for referencing the root clock
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID ClockRootGating(DDK_CLOCK_BAUD_SOURCE root, BOOL bClkGate)
{
    switch(root)
    {

    case DDK_CLOCK_BAUD_SOURCE_PLL1: 
        if(bClkGate) 
        {
            HW_CLKCTRL_PLL1CTRL0_CLR(BM_CLKCTRL_PLL1CTRL0_POWER);
            //RETAILMSG(1, (_T("PLL1 Disabled!\r\n")));
        }
        else
        {
            HW_CLKCTRL_PLL1CTRL0_SET(BM_CLKCTRL_PLL1CTRL0_POWER);
            // Sleep(1);     // Powerup/PowerDown will call this function and cause low priority error. omment this out 
            //RETAILMSG(1, (_T("PLL1 Enabled!\r\n")));
        }        
        break;

    case DDK_CLOCK_BAUD_SOURCE_PLL2: 
        if(bClkGate) 
        {
            HW_CLKCTRL_PLL2CTRL0_SET(BM_CLKCTRL_PLL2CTRL0_CLKGATE);
            HW_CLKCTRL_PLL2CTRL0_CLR(BM_CLKCTRL_PLL2CTRL0_POWER);
            //RETAILMSG(1, (_T("PLL2 Disabled!\r\n")));
        }
        else
        {       
            HW_CLKCTRL_PLL2CTRL0_SET(BM_CLKCTRL_PLL2CTRL0_POWER);
            // Sleep(1);     // Powerup/PowerDown will call this function and cause low priority error. omment this out 
            HW_CLKCTRL_PLL2CTRL0_CLR(BM_CLKCTRL_PLL2CTRL0_CLKGATE);
            //RETAILMSG(1, (_T("PLL2 Enabled!\r\n")));
        }        
        break;
       
    case DDK_CLOCK_BAUD_SOURCE_REF_IO0: 
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC0_SET(BM_CLKCTRL_FRAC0_CLKGATEIO0);
            //RETAILMSG(1, (_T("REF_IO0 Disabled!\r\n")));
        }
        else
        {
            HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATEIO0);
            //RETAILMSG(1, (_T("REF_IO0 Enabled!\r\n")));
        }        
        break;
        
    case DDK_CLOCK_BAUD_SOURCE_REF_IO1: 
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC0_SET(BM_CLKCTRL_FRAC0_CLKGATEIO1);
            //RETAILMSG(1, (_T("REF_IO1 Disabled!\r\n")));
        }
        else
        {
            HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATEIO1);
            //RETAILMSG(1, (_T("REF_IO1 Enabled!\r\n")));
        }        
        break;
             
    case DDK_CLOCK_BAUD_SOURCE_REF_GPMI:
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEGPMI);
            //RETAILMSG(1, (_T("REF_GPMI Disabled!\r\n")));            
        }
        else
        {
            HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEGPMI);
            //RETAILMSG(1, (_T("REF_GPMI Enabled!\r\n")));            
        }        
        break;
        
    case DDK_CLOCK_BAUD_SOURCE_REF_HSADC:
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEHSADC);
            //RETAILMSG(1, (_T("REF_HSADC Disabled!\r\n")));            
        }
        else
        {
            HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEHSADC);
            //RETAILMSG(1, (_T("REF_HSADC Enabled!\r\n")));            
        }        
        break;

    case DDK_CLOCK_BAUD_SOURCE_REF_PIX:         
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEPIX);
            //RETAILMSG(1, (_T("REF_PIX Disabled!\r\n")));            
        }    
        else
        {
            HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEPIX);
            //RETAILMSG(1, (_T("REF_PIX Enabled!\r\n")));             
        }
        break; 

    }
    
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
//      bClkGate
//           [in] Requested clock gating mode for the peripheral.
//           TRUE:   Clock Gating
//           FALSE:  Clock Enable
//
// Returns:
//      Returns TRUE if the clock gating mode was set successfully, otherwise 
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, BOOL bClkGate)
{
    DDK_CLOCK_BAUD_SOURCE root;

    if (bClkGate == TRUE)
    {
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
        EnterCriticalSection(&g_csDdkClk);
#endif        
        switch (g_pDdkClkConfig->clockRefCount[index])
        {
            case 0:
                break;
    
            case 1:
                // Remove reference to root clock
                root = g_pDdkClkConfig->root[index];
                switch(g_pDdkClkConfig->rootRefCount[root])
                {
                case 0:
                    break;
    
                case 1:
                    ClockRootGating(root, TRUE);
                    // Fall through
    
                default:
                    --g_pDdkClkConfig->rootRefCount[root];
                    break;
                }
                            
                // Disable the clock
                ClockSetGating(index, TRUE);
    
                // Fall through
                
            default:
                --g_pDdkClkConfig->clockRefCount[index];
                break;
        }

    
    }
    else
    {        
    
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL

        // If clock is being enabled, call down to platform code to check if
        // we need to block until a system setpoint that can support this
        // clock mode is available.  Avoid acquiring lock to prevent
        // blocking other drivers from managing their own clocks.
        BSPClockSetGatingModePrepare(index);

        EnterCriticalSection(&g_csDdkClk);

#endif
        switch (g_pDdkClkConfig->clockRefCount[index])
        {
        case 0:
            // Add reference to root clock
            root = g_pDdkClkConfig->root[index];
            switch(g_pDdkClkConfig->rootRefCount[root])
            {
            case 0:
                ClockRootGating(root, FALSE);
                // Fall through

            default:
                ++g_pDdkClkConfig->rootRefCount[root];
                break;
            }

            // Enable the clock
            ClockSetGating(index, FALSE);   
            
            // Fall through
            
        default:
            // Reference count cannot be incremented since some drivers
            // will enable clock multiple times but only disable once.  Force
            // reference count to 1 since clock nodes managed directly by drivers
            // should not be shared and accurate reference count is not
            // required.
            g_pDdkClkConfig->clockRefCount[index] = 1;
            break;
        }
    }
    
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
    // Allow platform code to complete the clock gating request
    BSPClockSetGatingModeComplete(index, bClkGate);

    LeaveCriticalSection(&g_csDdkClk);
#endif    
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
// Returns:
//      Returns TRUE if the clock gating mode was set, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX index)
{
    BOOL rc = FALSE;
    switch(index)
    {
        case DDK_CLOCK_GATE_UART_CLK: 
            rc = HW_CLKCTRL_XTAL.B.UART_CLK_GATE ? TRUE : FALSE;
            break;
            
        case DDK_CLOCK_GATE_PWM24M_CLK:   
            rc = HW_CLKCTRL_XTAL.B.PWM_CLK24M_GATE ? TRUE : FALSE;
            break;
            
        case DDK_CLOCK_GATE_TIMROT32K_CLK:
            rc = HW_CLKCTRL_XTAL.B.TIMROT_CLK32K_GATE ? TRUE : FALSE;
            break;
            
        case DDK_CLOCK_GATE_SSP0_CLK  :  
            rc = HW_CLKCTRL_SSP0.B.CLKGATE ? TRUE : FALSE;
            break;
            
        case DDK_CLOCK_GATE_SSP1_CLK  :  
            rc = HW_CLKCTRL_SSP1.B.CLKGATE ? TRUE : FALSE;
            break;
            
        case DDK_CLOCK_GATE_SSP2_CLK  :  
            rc = HW_CLKCTRL_SSP2.B.CLKGATE ? TRUE : FALSE;
            break;
            
        case DDK_CLOCK_GATE_SSP3_CLK  :  
            rc = HW_CLKCTRL_SSP3.B.CLKGATE ? TRUE : FALSE;
            break;
            
        case DDK_CLOCK_GATE_GPMI_CLK :  
            rc = HW_CLKCTRL_GPMI.B.CLKGATE ? TRUE : FALSE;
            break;
            
        case DDK_CLOCK_GATE_SPDIF_CLK:  
            rc = HW_CLKCTRL_SPDIF.B.CLKGATE ? TRUE : FALSE;
            break;
            
        case DDK_CLOCK_GATE_SAIF0_CLK:       
            rc = HW_CLKCTRL_SAIF0.B.CLKGATE ? TRUE : FALSE;
            break;
            
        case DDK_CLOCK_GATE_SAIF1_CLK:       
            rc = HW_CLKCTRL_SAIF1.B.CLKGATE ? TRUE : FALSE;
            break;

        case DDK_CLOCK_GATE_DIS_LCDIF_CLK:  
            rc = HW_CLKCTRL_DIS_LCDIF.B.CLKGATE ? TRUE : FALSE;
            break;

        case DDK_CLOCK_GATE_ETM_CLK:  
            rc = HW_CLKCTRL_ETM.B.CLKGATE ? TRUE : FALSE;
            break;
            
        case DDK_CLOCK_GATE_ENET_CLK:  
            rc = HW_CLKCTRL_ENET.B.SLEEP ? TRUE : FALSE;
            break;              

        case DDK_CLOCK_GATE_FLEXCAN0_CLK  :  
            rc = HW_CLKCTRL_FLEXCAN.B.STOP_CAN0 ? TRUE : FALSE;
            break;   
            
        case DDK_CLOCK_GATE_FLEXCAN1_CLK  :  
           rc = HW_CLKCTRL_FLEXCAN.B.STOP_CAN1  ? TRUE : FALSE;
           break;     

        case DDK_CLOCK_GATE_UTMI0_CLK480M_CLK  :  
           rc = HW_CLKCTRL_PLL0CTRL0.B.EN_USB_CLKS  ? FALSE : TRUE;
           break;

        case DDK_CLOCK_GATE_UTMI1_CLK480M_CLK  :  
           rc = HW_CLKCTRL_PLL1CTRL0.B.EN_USB_CLKS  ? FALSE : TRUE;
           break;

        case DDK_CLOCK_GATE_HSADC_CLK  :  
           rc = HW_CLKCTRL_FRAC1.B.CLKGATEHSADC  ? TRUE : FALSE;
           break;

        default:
            rc = FALSE;
            break; 
    }
    return rc;
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
// CCM serial peripheral baud clock output.
//
// Parameters:
//      sig
//          [in] Clock signal to configure.
//
//      src
//          [in] Selects the input clock source.
//
//      u32Div
//          [in] Specifies the value programmed into the baud clock divider.
//
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKClockConfigBaud(DDK_CLOCK_SIGNAL index, DDK_CLOCK_BAUD_SOURCE src,
                                UINT32 u32Div)
{
    BOOL rc = TRUE;
    BOOL bclkgate = FALSE;
    // Check divider range
    if(u32Div == 0)
    {
        rc = FALSE;
        ERRORMSG(1, (_T("DDKClockConfigBaud: Divide by zero\r\n")));            
        return rc;
    }
    switch(index)
    {
        case DDK_CLOCK_SIGNAL_SSP0  :  
            // Return if busy with another divider change.
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
            if(HW_CLKCTRL_SSP0.B.BUSY )
            {
                RETAILMSG(1, (_T(" DDK_CLOCK_SIGNAL_SSP0_CLK ERROR1 \r\n")));
                rc = FALSE;
                break;
            }
            // Change Clock source
            if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            {
                HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_SSP0);
            }
            else if(src == DDK_CLOCK_BAUD_SOURCE_REF_IO0)
            {
                HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SSP0);
            }
            else 
            {
                RETAILMSG(1, (_T(" DDK_CLOCK_SIGNAL_SSP0_CLK ERROR2 \r\n")));        
                rc = FALSE;
                break;
            }
            ClockUpdateRoot(DDK_CLOCK_GATE_SSP0_CLK, src);  
          
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
            if((HW_CLKCTRL_SSP0_RD() & BM_CLKCTRL_SSP0_CLKGATE) != 0)
            {
                HW_CLKCTRL_SSP0_CLR(BM_CLKCTRL_SSP0_CLKGATE); 
                bclkgate = TRUE;
            }
            
            // Change divider
            // Always use integer divide for SSP clock
            HW_CLKCTRL_SSP0_CLR(BM_CLKCTRL_SSP0_DIV_FRAC_EN);
            
            // Set divider
            HW_CLKCTRL_SSP0.B.DIV = u32Div;   
            
            //restore clock gating states
            if(bclkgate)
                HW_CLKCTRL_SSP0_SET(BM_CLKCTRL_SSP0_CLKGATE);            
            break;
            
        case DDK_CLOCK_SIGNAL_SSP1  :  
            // Return if busy with another divider change.
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
            if(HW_CLKCTRL_SSP1.B.BUSY )
            {
                RETAILMSG(1, (_T(" DDK_CLOCK_SIGNAL_SSP1_CLK ERROR1 \r\n")));
                rc = FALSE;
                break;
            }
            // Change Clock source
            if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            {
                HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_SSP1);
            }
            else if(src == DDK_CLOCK_BAUD_SOURCE_REF_IO0)
            {
                HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SSP1);
            }
            else 
            {
                RETAILMSG(1, (_T(" DDK_CLOCK_SIGNAL_SSP0_CLK ERROR2 \r\n")));        
                rc = FALSE;
                break;
            }
            ClockUpdateRoot(DDK_CLOCK_GATE_SSP1_CLK, src);  
          
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
            if((HW_CLKCTRL_SSP1_RD() & BM_CLKCTRL_SSP1_CLKGATE) != 0)
            {
                HW_CLKCTRL_SSP1_CLR(BM_CLKCTRL_SSP1_CLKGATE); 
                bclkgate = TRUE;
            }
            
            // Change divider
            // Always use integer divide for SSP clock
            HW_CLKCTRL_SSP1_CLR(BM_CLKCTRL_SSP1_DIV_FRAC_EN);
            
            // Set divider
            HW_CLKCTRL_SSP1.B.DIV = u32Div;   
            
            //restore clock gating states
            if(bclkgate)
                HW_CLKCTRL_SSP1_SET(BM_CLKCTRL_SSP1_CLKGATE);            
            break;

        case DDK_CLOCK_SIGNAL_SSP2  :  
            // Return if busy with another divider change.
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
            if(HW_CLKCTRL_SSP2.B.BUSY )
            {
                RETAILMSG(1, (_T(" DDK_CLOCK_SIGNAL_SSP2_CLK ERROR1 \r\n")));
                rc = FALSE;
                break;
            }
            // Change Clock source
            if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            {
                HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_SSP2);
            }
            else if(src == DDK_CLOCK_BAUD_SOURCE_REF_IO1)
            {
                HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SSP2);
            }
            else 
            {
                RETAILMSG(1, (_T(" DDK_CLOCK_SIGNAL_SSP2_CLK ERROR2 \r\n")));        
                rc = FALSE;
                break;
            }
            ClockUpdateRoot(DDK_CLOCK_GATE_SSP2_CLK, src);  
          
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
            if((HW_CLKCTRL_SSP2_RD() & BM_CLKCTRL_SSP2_CLKGATE) != 0)
            {
                HW_CLKCTRL_SSP2_CLR(BM_CLKCTRL_SSP2_CLKGATE); 
                bclkgate = TRUE;
            }
            
            // Change divider
            // Always use integer divide for SSP clock
            HW_CLKCTRL_SSP2_CLR(BM_CLKCTRL_SSP2_DIV_FRAC_EN);
            
            // Set divider
            HW_CLKCTRL_SSP2.B.DIV = u32Div;   
            
            //restore clock gating states
            if(bclkgate)
                HW_CLKCTRL_SSP2_SET(BM_CLKCTRL_SSP2_CLKGATE);            
            break;

        case DDK_CLOCK_SIGNAL_SSP3  :  
            // Return if busy with another divider change.
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
            if(HW_CLKCTRL_SSP3.B.BUSY )
            {
                RETAILMSG(1, (_T(" DDK_CLOCK_SIGNAL_SSP3_CLK ERROR1 \r\n")));
                rc = FALSE;
                break;
            }
            // Change Clock source
            if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            {
                HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_SSP3);
            }
            else if(src == DDK_CLOCK_BAUD_SOURCE_REF_IO1)
            {
                HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SSP3);
            }
            else 
            {
                RETAILMSG(1, (_T(" DDK_CLOCK_SIGNAL_SSP3_CLK ERROR2 \r\n")));        
                rc = FALSE;
                break;
            }
            ClockUpdateRoot(DDK_CLOCK_GATE_SSP3_CLK, src);  
          
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
            if((HW_CLKCTRL_SSP3_RD() & BM_CLKCTRL_SSP3_CLKGATE) != 0)
            {
                HW_CLKCTRL_SSP3_CLR(BM_CLKCTRL_SSP3_CLKGATE); 
                bclkgate = TRUE;
            }
            
            // Change divider
            // Always use integer divide for SSP clock
            HW_CLKCTRL_SSP3_CLR(BM_CLKCTRL_SSP3_DIV_FRAC_EN);
            
            // Set divider
            HW_CLKCTRL_SSP3.B.DIV = u32Div;   
            
            //restore clock gating states
            if(bclkgate)
                HW_CLKCTRL_SSP3_SET(BM_CLKCTRL_SSP3_CLKGATE);            
            break;

        case DDK_CLOCK_SIGNAL_GPMI :  
            // Return if busy with another divider change.
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
            if(HW_CLKCTRL_GPMI.B.BUSY )
            {
                rc = FALSE;
                break;
            }
            // Change Clock source
            if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            {
                HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);
            }
            else if(src == DDK_CLOCK_BAUD_SOURCE_REF_GPMI)
            {
                HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);
            }
            else 
            {
                rc = FALSE;
                break;
            }
            ClockUpdateRoot(DDK_CLOCK_GATE_GPMI_CLK, src);   
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
            if((HW_CLKCTRL_GPMI_RD() & BM_CLKCTRL_GPMI_CLKGATE) != 0)
            {
                HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_ETM_CLKGATE); 
                bclkgate = TRUE;
            }
            // Change divider
            // Always use integer divide for SSP clock
            HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_GPMI_DIV_FRAC_EN);
            // Set divider
            HW_CLKCTRL_GPMI.B.DIV = u32Div;
            //waiting Clock transfering to new divider
            //while(HW_CLKCTRL_GPMI.B.BUSY);
            //restore clock gating states
            if(bclkgate)
            {
                HW_CLKCTRL_GPMI_SET(BM_CLKCTRL_GPMI_CLKGATE);
            }
            break;

        case DDK_CLOCK_SIGNAL_SAIF0  :       
            // Return if busy with another divider change
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
            if(HW_CLKCTRL_SAIF0.B.BUSY)
            {
                rc = FALSE;
                break;
            }

            // SAIF0 Clock source always from REF_PLL
            HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SAIF0);
            
            ClockUpdateRoot(DDK_CLOCK_GATE_SAIF0_CLK, DDK_CLOCK_BAUD_SOURCE_REF_PLL);
            
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
            if((HW_CLKCTRL_SAIF0_RD() & BM_CLKCTRL_SAIF0_CLKGATE) != 0)
            {
                HW_CLKCTRL_SAIF0_CLR(BM_CLKCTRL_SAIF0_CLKGATE); 
                bclkgate = TRUE;
            }  
            // Change gate and/or divider
            // Always enable fractional divide for SAIF
            HW_CLKCTRL_SAIF0_SET(BM_CLKCTRL_SAIF0_DIV_FRAC_EN);
            // Set divider for
            HW_CLKCTRL_SAIF0.B.DIV = u32Div;
            //restore clock gating states
            if(bclkgate)
            {
                HW_CLKCTRL_SAIF0_SET(BM_CLKCTRL_SAIF0_CLKGATE);
            }
            break;   

        case DDK_CLOCK_SIGNAL_SAIF1  :       
            // Return if busy with another divider change
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
            if(HW_CLKCTRL_SAIF1.B.BUSY)
            {
                rc = FALSE;
                break;
            }

            // SAIF1 Clock source always from REF_PLL
            HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SAIF1);
            
            ClockUpdateRoot(DDK_CLOCK_GATE_SAIF1_CLK, DDK_CLOCK_BAUD_SOURCE_REF_PLL);
            
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
            if((HW_CLKCTRL_SAIF1_RD() & BM_CLKCTRL_SAIF1_CLKGATE) != 0)
            {
                HW_CLKCTRL_SAIF1_CLR(BM_CLKCTRL_SAIF1_CLKGATE); 
                bclkgate = TRUE;
            }  
            // Change gate and/or divider
            // Always enable fractional divide for SAIF
            HW_CLKCTRL_SAIF1_SET(BM_CLKCTRL_SAIF1_DIV_FRAC_EN);
            // Set divider for
            HW_CLKCTRL_SAIF1.B.DIV = u32Div;
            //restore clock gating states
            if(bclkgate)
            {
                HW_CLKCTRL_SAIF1_SET(BM_CLKCTRL_SAIF1_CLKGATE);
            }
            break;   

        case DDK_CLOCK_SIGNAL_DIS_LCDIF  : 
            if(u32Div > MAX_CLKCTRL_DIS_LCDIF_DIV)
            {
                rc = FALSE;
                break;
            }
            // Return if busy with another divider change
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0      
            if(HW_CLKCTRL_DIS_LCDIF.B.BUSY)
            {
                rc = FALSE;
                break;
            } 
            // Chang Clock source
            if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            {
                HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF);
            }
            else if(src == DDK_CLOCK_BAUD_SOURCE_REF_PIX)
            {
                HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF);
            }
            else 
            {
                rc = FALSE;
                break;
            }
            ClockUpdateRoot(DDK_CLOCK_GATE_DIS_LCDIF_CLK, src);
            // Only change  DIV when CLKGATE = 0
            if((HW_CLKCTRL_DIS_LCDIF_RD() & BM_CLKCTRL_DIS_LCDIF_CLKGATE) != 0)
            {
                HW_CLKCTRL_DIS_LCDIF_CLR(BM_CLKCTRL_DIS_LCDIF_CLKGATE); 
                bclkgate = TRUE;
            }  
            // Change divider
            // Always use integer divide for PIX clock
            HW_CLKCTRL_DIS_LCDIF_CLR(BM_CLKCTRL_DIS_LCDIF_DIV_FRAC_EN);
            // Set divider
            HW_CLKCTRL_DIS_LCDIF.B.DIV = u32Div;
            //restore clock gating states
            if(bclkgate)
            {
                HW_CLKCTRL_DIS_LCDIF_SET(BM_CLKCTRL_DIS_LCDIF_CLKGATE);
            }
            break;

        case DDK_CLOCK_SIGNAL_ETM  : 
            // Return if busy with another divider change.
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
            if(HW_CLKCTRL_ETM.B.BUSY)
            {
                rc = FALSE;
                break;
            }
            // Change Clock source
            if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            {
                HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_ETM);
            }
            else if(src == DDK_CLOCK_BAUD_SOURCE_REF_CPU)
            {
                HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_ETM);
            }
            else 
            {
                rc = FALSE;
                break;
            }
            ClockUpdateRoot(DDK_CLOCK_GATE_ETM_CLK, src); 
            // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
            if((HW_CLKCTRL_ETM_RD() & BM_CLKCTRL_ETM_CLKGATE) != 0)
            {
                HW_CLKCTRL_ETM_CLR(BM_CLKCTRL_ETM_CLKGATE); 
                bclkgate = TRUE;
            }
            // Change divider
            // Always use integer divide for ETM clock
            HW_CLKCTRL_ETM_CLR(BM_CLKCTRL_ETM_DIV_FRAC_EN);
            // Set divider
            HW_CLKCTRL_ETM.B.DIV = u32Div;  
            //restore clock gating states
            if(bclkgate)
            {
                HW_CLKCTRL_ETM_SET(BM_CLKCTRL_ETM_CLKGATE);
            }
            break;

        case DDK_CLOCK_SIGNAL_HSADC :
            //HW_CLKCTRL_HSADC.B.RESETB = 1;
            //HW_CLKCTRL_HSADC.B.FREQDIV = u32Div;              
			// CS&ZHL JUN-12-2012: setup HSADC clock
			switch(u32Div)
			{
			case 9:
				HW_CLKCTRL_HSADC.B.RESETB = 1;
				HW_CLKCTRL_HSADC.B.FREQDIV = 0; 
				break;

			case 18:
				HW_CLKCTRL_HSADC.B.RESETB = 1;
				HW_CLKCTRL_HSADC.B.FREQDIV = 1; 
				break;

			case 36:
				HW_CLKCTRL_HSADC.B.RESETB = 1;
				HW_CLKCTRL_HSADC.B.FREQDIV = 2; 
				break;

			case 72:
				HW_CLKCTRL_HSADC.B.RESETB = 1;
				HW_CLKCTRL_HSADC.B.FREQDIV = 3; 
				break;

			default:
				rc = FALSE;
				ERRORMSG(1, (_T("DDKClockConfigBaud: invalid HSADC divider = %d\r\n"), u32Div));            
			}
            break;

        case DDK_CLOCK_SIGNAL_PLL0 :
        case DDK_CLOCK_SIGNAL_PLL1 :
        case DDK_CLOCK_SIGNAL_PLL2 :
        case DDK_CLOCK_SIGNAL_REF_CPU :
        case DDK_CLOCK_SIGNAL_REF_EMI :
        case DDK_CLOCK_SIGNAL_REF_IO0 :
        case DDK_CLOCK_SIGNAL_REF_IO1 :
        case DDK_CLOCK_SIGNAL_REF_PIX :
        case DDK_CLOCK_SIGNAL_REF_HSADC :
        case DDK_CLOCK_SIGNAL_REF_PLL :   
        case DDK_CLOCK_SIGNAL_REF_XTAL:  
        case DDK_CLOCK_SIGNAL_REF_ENET_PLL :
        case DDK_CLOCK_SIGNAL_P_CLK :
        case DDK_CLOCK_SIGNAL_H_CLK :
        case DDK_CLOCK_SIGNAL_H_CLK_FLEXCAN0_IPG :
        case DDK_CLOCK_SIGNAL_H_CLK_FLEXCAN1_IPG :
        case DDK_CLOCK_SIGNAL_H_CLK_FLEXCAN0 :
        case DDK_CLOCK_SIGNAL_H_CLK_FLEXCAN1 :
        case DDK_CLOCK_SIGNAL_H_CLK_ENET_SWI :
        case DDK_CLOCK_SIGNAL_H_CLK_MAC0 :
        case DDK_CLOCK_SIGNAL_H_CLK_MAC1 :
        case DDK_CLOCK_SIGNAL_OCROM :
        case DDK_CLOCK_SIGNAL_EMI :
        case DDK_CLOCK_SIGNAL_ENET_TIME :
        case DDK_CLOCK_SIGNAL_X_CLK :
        case DDK_CLOCK_SIGNAL_UART :
        case DDK_CLOCK_SIGNAL_XTAL24M :
        case DDK_CLOCK_SIGNAL_32K :
        case DDK_CLOCK_SIGNAL_FLEXCAN0 :
        case DDK_CLOCK_SIGNAL_FLEXCAN1 :
        case DDK_CLOCK_SIGNAL_LRADC2K :
        case DDK_CLOCK_SIGNAL_UTMI0 :
        case DDK_CLOCK_SIGNAL_UTMI1 :
        case DDK_CLOCK_SIGNAL_PWM24M :
        case DDK_CLOCK_SIGNAL_TIMROT32K :
                        
        default:
            rc = FALSE;
            break; 
    }
    if(rc)
    {
        BSPClockUpdateFreq(index, src, u32Div);
    }
    else
    {
        RETAILMSG(1, (_T(" DDKClockConfigBaud ERROR, clock index= %x ERROR \r\n"),index ));        
    } 
    return rc;
}

#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
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
BOOL DDKClockSetpointRequest(DDK_DVFC_SETPOINT setpoint,BOOL bBlock)
{
    return BSPClockSetpointRequest(setpoint, bBlock);
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
#endif

//-----------------------------------------------------------------------------
//
//  Function: DDKClockReadENetConfig
//
//  Read ENet Clock config register
//
//  Parameters:
//      None.
//
//  Returns:
//      Enet register state.
//
//-----------------------------------------------------------------------------
UINT32 DDKClockReadENetConfig(VOID)
{
    return HW_CLKCTRL_ENET_RD();
}

//-----------------------------------------------------------------------------
//
//  Function: DDKClockWriteENetConfig
//
// Write config data to Enet clock config register
//
//  Parameters:
//      Enet register state.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID DDKClockWriteENetConfig(UINT32 RegData)
{
    HW_CLKCTRL_ENET_WR(RegData);
}




