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
//  This file contains the Clock generation and conrol DDK interface that is used
//  by applications and other drivers to access the capabilities of the clock driver.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#ifdef BOOTLOADER
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

#ifdef BOOTLOADER
#define LOCK()
#define UNLOCK()
#else
#define LOCK()   EnterCriticalSection(&g_csDdkClk)
#define UNLOCK() LeaveCriticalSection(&g_csDdkClk)
#endif

#define MAX_CLKCTRL_PIX_DIV     255
#define MAX_CLKCTRL_IR_DIV      768
#define MIN_CLKCTRL_IR_DIV      5
#define MAX_CLKCTRL_IROV_DIV    260
#define MIN_CLKCTRL_IROV_DIV    4
#define MIN_CLKCTRL_PFD_VALUE   18
#define MAX_CLKCTRL_PFD_VALUE   35

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
extern BOOL BSPClockUpdateFreq(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src,
                               UINT32 u32Div);
#ifndef BOOTLOADER
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
#ifndef BOOTLOADER
    PHYSICAL_ADDRESS phyAddr;
#endif
    if (pv_HWregCLKCTRL == NULL)
    {
#ifdef BOOTLOADER
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
#ifndef BOOTLOADER
        MmUnmapIoSpace(pv_HWregCLKCTRL, 0x1000);
#endif
        pv_HWregCLKCTRL = NULL;
    }
#ifndef BOOTLOADER
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
    case DDK_CLOCK_GATE_ETM_CLK:  
        HW_CLKCTRL_ETM.B.CLKGATE = bClkGate ? 1 : 0;
        break;
              
    case DDK_CLOCK_GATE_SSP_CLK  :  
        HW_CLKCTRL_SSP.B.CLKGATE = bClkGate ? 1 : 0;
        break;
                
    case DDK_CLOCK_GATE_GPMI_CLK :  
        HW_CLKCTRL_GPMI.B.CLKGATE = bClkGate ? 1 : 0;
        break;
                
    case DDK_CLOCK_GATE_IROV_CLK :                
    case DDK_CLOCK_GATE_IR_CLK   : 
        HW_CLKCTRL_IR.B.CLKGATE = bClkGate ? 1 : 0;
        break;
                
    case DDK_CLOCK_GATE_PIX_CLK  :        
        HW_CLKCTRL_PIX.B.CLKGATE = bClkGate ? 1 : 0;
        break;
        
    case DDK_CLOCK_GATE_SAIF_CLK:       
        HW_CLKCTRL_SAIF.B.CLKGATE = bClkGate ? 1 : 0;
        break;
        
    case DDK_CLOCK_GATE_PCMSPDIF_CLK :
        HW_CLKCTRL_SPDIF.B.CLKGATE = bClkGate ? 1 : 0;
        break;
        
    case DDK_CLOCK_GATE_TVENC_FIFO:
    case DDK_CLOCK_GATE_TV27M_CLK :
    case DDK_CLOCK_GATE_TV54M_CLK :
        HW_CLKCTRL_TV.B.CLK_TV_GATE = bClkGate ? 1 : 0;
        break;

    case DDK_CLOCK_GATE_TV108M_CLK :
        HW_CLKCTRL_TV.B.CLK_TV108M_GATE = bClkGate ? 1 : 0;
        break;

    case DDK_CLOCK_GATE_UART_CLK: 
        if(bClkGate) 
            HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_UART_CLK_GATE);
        else
            HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_UART_CLK_GATE);
        break;
        
    case DDK_CLOCK_GATE_FILT24M_CLK: 
        if(bClkGate) 
            HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_FILT_CLK24M_GATE);
        else
            HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_FILT_CLK24M_GATE);
        break;
                
    case DDK_CLOCK_GATE_PWM24M_CLK:   
        if(bClkGate) 
            HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_PWM_CLK24M_GATE);
        else
            HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_PWM_CLK24M_GATE);
        break;
                
    case DDK_CLOCK_GATE_DRI24M_CLK: 
        if(bClkGate) 
            HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_DRI_CLK24M_GATE);
        else
            HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_DRI_CLK24M_GATE);
        break;
                
    case DDK_CLOCK_GATE_DIGCTL_CLK1M_CLK: 
        if(bClkGate) 
            HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_DIGCTRL_CLK1M_GATE);
        else
            HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_DIGCTRL_CLK1M_GATE);
        break;
                
    case DDK_CLOCK_GATE_TIMROT_32K_CLK:
        if(bClkGate) 
            HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_TIMROT_CLK32K_GATE);
        else
            HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_TIMROT_CLK32K_GATE);
        break;
        
    case DDK_CLOCK_GATE_UTMI_CLK480M:
        if( bClkGate )
            HW_CLKCTRL_PLLCTRL0_CLR(BM_CLKCTRL_PLLCTRL0_EN_USB_CLKS);
        else
            HW_CLKCTRL_PLLCTRL0_SET(BM_CLKCTRL_PLLCTRL0_EN_USB_CLKS);
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
#ifndef BOOTLOADER    
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
#ifndef BOOTLOADER
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
       
    case DDK_CLOCK_BAUD_SOURCE_REF_IO: 
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC_SET(BM_CLKCTRL_FRAC_CLKGATEIO);
            //RETAILMSG(1, (_T("REF_IO Disabled!\r\n")));
        }
        else
        {
            HW_CLKCTRL_FRAC_CLR(BM_CLKCTRL_FRAC_CLKGATEIO);
            //RETAILMSG(1, (_T("REF_IO Enabled!\r\n")));
        }        
        break;
        
    case DDK_CLOCK_BAUD_SOURCE_REF_PIX:         
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC_SET(BM_CLKCTRL_FRAC_CLKGATEPIX);
            //RETAILMSG(1, (_T("REF_PIX Disabled!\r\n")));            
        }    
        else
        {
            HW_CLKCTRL_FRAC_CLR(BM_CLKCTRL_FRAC_CLKGATEPIX);
            //RETAILMSG(1, (_T("REF_PIX Enabled!\r\n")));             
        }
        break; 
             
    case DDK_CLOCK_BAUD_SOURCE_REF_VID:
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEVID);
            //RETAILMSG(1, (_T("REF_VID Disabled!\r\n")));            
        }
        else
        {
            HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEVID);
            //RETAILMSG(1, (_T("REF_VID Enabled!\r\n")));            
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
#ifndef BOOTLOADER    
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
#ifndef BOOTLOADER
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
#ifndef BOOTLOADER
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
    case DDK_CLOCK_GATE_ETM_CLK:  
        rc = HW_CLKCTRL_ETM.B.CLKGATE ? TRUE : FALSE;
        break;
               
    case DDK_CLOCK_GATE_SSP_CLK  :  
        rc = HW_CLKCTRL_SSP.B.CLKGATE ? TRUE : FALSE;
        break;
                
    case DDK_CLOCK_GATE_GPMI_CLK :  
        rc = HW_CLKCTRL_GPMI.B.CLKGATE ? TRUE : FALSE;
        break;
                
    case DDK_CLOCK_GATE_IROV_CLK :                
    case DDK_CLOCK_GATE_IR_CLK   : 
        rc = HW_CLKCTRL_IR.B.CLKGATE ? TRUE : FALSE;
        break;
                
    case DDK_CLOCK_GATE_PIX_CLK  :        
        rc = HW_CLKCTRL_PIX.B.CLKGATE ? TRUE : FALSE;
        break;
        
    case DDK_CLOCK_GATE_SAIF_CLK:       
        rc = HW_CLKCTRL_SAIF.B.CLKGATE ? TRUE : FALSE;
        break;
        
    case DDK_CLOCK_GATE_PCMSPDIF_CLK :
        rc = HW_CLKCTRL_SPDIF.B.CLKGATE ? TRUE : FALSE;
        break;
        
    case DDK_CLOCK_GATE_TVENC_FIFO:
    case DDK_CLOCK_GATE_TV27M_CLK :
    case DDK_CLOCK_GATE_TV54M_CLK :
        rc = HW_CLKCTRL_TV.B.CLK_TV_GATE ? TRUE : FALSE;
        break;
    
    case DDK_CLOCK_GATE_TV108M_CLK :
        rc = HW_CLKCTRL_TV.B.CLK_TV108M_GATE ? TRUE : FALSE;
        break;
    
    case DDK_CLOCK_GATE_UART_CLK: 
        rc = HW_CLKCTRL_XTAL.B.UART_CLK_GATE ? TRUE : FALSE;
        break;
          
    case DDK_CLOCK_GATE_FILT24M_CLK: 
        rc = HW_CLKCTRL_XTAL.B.FILT_CLK24M_GATE ? TRUE : FALSE;
        break;
                
    case DDK_CLOCK_GATE_PWM24M_CLK:   
        rc = HW_CLKCTRL_XTAL.B.PWM_CLK24M_GATE ? TRUE : FALSE;
        break;
                
    case DDK_CLOCK_GATE_DRI24M_CLK: 
        rc = HW_CLKCTRL_XTAL.B.DRI_CLK24M_GATE ? TRUE : FALSE;
        break;    
                
    case DDK_CLOCK_GATE_DIGCTL_CLK1M_CLK: 
        rc = HW_CLKCTRL_XTAL.B.DIGCTRL_CLK1M_GATE ? TRUE : FALSE;
        break;    
                
    case DDK_CLOCK_GATE_TIMROT_32K_CLK:
        rc = HW_CLKCTRL_XTAL.B.TIMROT_CLK32K_GATE ? TRUE : FALSE;
        break;
        
    case DDK_CLOCK_GATE_UTMI_CLK480M:
        rc = HW_CLKCTRL_PLLCTRL0.B.EN_USB_CLKS ? FALSE : TRUE;
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
    case DDK_CLOCK_SIGNAL_ETM_CLK: 

        // Return if busy with another divider change.
        // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
        if(HW_CLKCTRL_ETM.B.BUSY)
        {
            rc = FALSE;
            break;
        }

        // Change Clock source
        if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_ETM);
        else if(src == DDK_CLOCK_BAUD_SOURCE_REF_CPU)
            HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_ETM);
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
            HW_CLKCTRL_ETM_SET(BM_CLKCTRL_ETM_CLKGATE);
        
        break;
                
    case DDK_CLOCK_SIGNAL_SSP_CLK  :  

        // Return if busy with another divider change.
        // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
        if(HW_CLKCTRL_SSP.B.BUSY )
        {
            RETAILMSG(1, (_T(" DDK_CLOCK_SIGNAL_SSP_CLK ERROR1 \r\n")));
            rc = FALSE;
            break;
        }

        // Change Clock source
        if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_SSP);
        else if(src == DDK_CLOCK_BAUD_SOURCE_REF_IO)
            HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SSP);
        else 
        {
            RETAILMSG(1, (_T(" DDK_CLOCK_SIGNAL_SSP_CLK ERROR2 \r\n")));        
            rc = FALSE;
            break;
        }
        ClockUpdateRoot(DDK_CLOCK_GATE_SSP_CLK, src);  
      
        // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
        if((HW_CLKCTRL_SSP_RD() & BM_CLKCTRL_SSP_CLKGATE) != 0)
        {
            HW_CLKCTRL_SSP_CLR(BM_CLKCTRL_SSP_CLKGATE); 
            bclkgate = TRUE;
        }
        
        // Change divider
        // Always use integer divide for SSP clock
        HW_CLKCTRL_SSP_CLR(BM_CLKCTRL_SSP_DIV_FRAC_EN);
        
        // Set divider
        HW_CLKCTRL_SSP.B.DIV = u32Div;   
        
        //restore clock gating states
        if(bclkgate)
            HW_CLKCTRL_SSP_SET(BM_CLKCTRL_SSP_CLKGATE);            
        break;
                
    case DDK_CLOCK_SIGNAL_GPMI_CLK :  

        // Return if busy with another divider change.
        // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
        if(HW_CLKCTRL_GPMI.B.BUSY )
        {
            rc = FALSE;
            break;
        }
        
        // Change Clock source
        if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);
        else if(src == DDK_CLOCK_BAUD_SOURCE_REF_IO)
            HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);
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
            HW_CLKCTRL_GPMI_SET(BM_CLKCTRL_GPMI_CLKGATE);
        
        break;
                
    case DDK_CLOCK_SIGNAL_IROV_CLK : 

        // Make sure our values are valid
        if( (u32Div < MIN_CLKCTRL_IROV_DIV) || (u32Div > MAX_CLKCTRL_IROV_DIV) )
        {
            rc = FALSE;
            break;
        }

        // Return if busy with another divider change
        // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
        if(HW_CLKCTRL_IR.B.IROV_BUSY)
        {
            rc = FALSE;
            break;
        }

        // Change Ref Clock source
        if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_IR);
        else if(src == DDK_CLOCK_BAUD_SOURCE_REF_IO)
            HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_IR);
        else 
        {
            rc = FALSE;
            break;
        }
        ClockUpdateRoot(DDK_CLOCK_GATE_IROV_CLK, src);    

        // Only change  DIV when CLKGATE = 0
        if((HW_CLKCTRL_IR_RD() & BM_CLKCTRL_IR_CLKGATE) != 0)
        {
            HW_CLKCTRL_IR_CLR(BM_CLKCTRL_IR_CLKGATE); 
            bclkgate = TRUE;
        }        
        
        // Change gate and/or divider
        // Set divider for IR_DIV
        HW_CLKCTRL_IR.B.IROV_DIV = u32Div;

        //restore clock gating states
        if(bclkgate)
            HW_CLKCTRL_IR_SET(BM_CLKCTRL_IR_CLKGATE);

        break;
        
    case DDK_CLOCK_SIGNAL_IR_CLK   : 

        // Make sure our values are valid
        if( (u32Div < MIN_CLKCTRL_IR_DIV) || (u32Div > MAX_CLKCTRL_IR_DIV) )
        {
            rc = FALSE;
            break;
        }

        // Return if busy with another divider change
        // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
        if(HW_CLKCTRL_IR.B.IROV_BUSY || HW_CLKCTRL_IR.B.IR_BUSY)
        {
            rc = FALSE;
            break;
        }

        // Change Ref Clock source
        if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_IR);
        else if(src == DDK_CLOCK_BAUD_SOURCE_REF_IO)
            HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_IR);
        else 
        {
            rc = FALSE;
            break;
        }

        ClockUpdateRoot(DDK_CLOCK_GATE_IR_CLK, src); 

        // Only change  DIV when CLKGATE = 0
        if((HW_CLKCTRL_IR_RD() & BM_CLKCTRL_IR_CLKGATE) != 0)
        {
            HW_CLKCTRL_IR_CLR(BM_CLKCTRL_IR_CLKGATE); 
            bclkgate = TRUE;
        } 
        
        // Change gate and/or divider
        // Set divider for IR_DIV
        HW_CLKCTRL_IR.B.IR_DIV = u32Div;

        //restore clock gating states
        if(bclkgate)
            HW_CLKCTRL_IR_SET(BM_CLKCTRL_IR_CLKGATE);

        break;
                
    case DDK_CLOCK_SIGNAL_PIX_CLK  : 

        if(u32Div > MAX_CLKCTRL_PIX_DIV)
        {
            rc = FALSE;
            break;
        }

        // Return if busy with another divider change
        // Only change DIV_FRAC_EN and DIV when CLKGATE = 0      
        if(HW_CLKCTRL_PIX.B.BUSY)
        {
            rc = FALSE;
            break;
        } 
        
        // Chang Clock source
        if(src == DDK_CLOCK_BAUD_SOURCE_REF_XTAL)
            HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_PIX);
        else if(src == DDK_CLOCK_BAUD_SOURCE_REF_PIX)
            HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_PIX);
        else 
        {
            rc = FALSE;
            break;
        }
        
        ClockUpdateRoot(DDK_CLOCK_GATE_PIX_CLK, src);

        // Only change  DIV when CLKGATE = 0
        if((HW_CLKCTRL_PIX_RD() & BM_CLKCTRL_PIX_CLKGATE) != 0)
        {
            HW_CLKCTRL_PIX_CLR(BM_CLKCTRL_PIX_CLKGATE); 
            bclkgate = TRUE;
        }  

        // Change divider
        // Always use integer divide for PIX clock
        HW_CLKCTRL_PIX_CLR(BM_CLKCTRL_PIX_DIV_FRAC_EN);
        
        // Set divider
        HW_CLKCTRL_PIX.B.DIV = u32Div;

        //restore clock gating states
        if(bclkgate)
            HW_CLKCTRL_PIX_SET(BM_CLKCTRL_PIX_CLKGATE);

        break;
          
    case DDK_CLOCK_SIGNAL_SAIF_CLK:       

        // Return if busy with another divider change
        // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
        if(HW_CLKCTRL_SAIF.B.BUSY)
        {
            rc = FALSE;
            break;
        }

        // SAIF Clock source always from REF_PLL
        HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_SAIF);

        ClockUpdateRoot(DDK_CLOCK_GATE_SAIF_CLK, DDK_CLOCK_BAUD_SOURCE_REF_PLL);

        // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
        if((HW_CLKCTRL_SAIF_RD() & BM_CLKCTRL_SAIF_CLKGATE) != 0)
        {
            HW_CLKCTRL_SAIF_CLR(BM_CLKCTRL_SAIF_CLKGATE); 
            bclkgate = TRUE;
        }  
        
        // Change gate and/or divider
        // Always enable fractional divide for SAIF
        HW_CLKCTRL_SAIF_SET(BM_CLKCTRL_SAIF_DIV_FRAC_EN);
        
        // Set divider for
        HW_CLKCTRL_SAIF.B.DIV = u32Div;

        //restore clock gating states
        if(bclkgate)
            HW_CLKCTRL_SAIF_SET(BM_CLKCTRL_SAIF_CLKGATE);

        break;   

    case DDK_CLOCK_SIGNAL_REF_PLL :   
    case DDK_CLOCK_SIGNAL_REF_XTAL:  
    case DDK_CLOCK_SIGNAL_TVENC_FIFO  :
    case DDK_CLOCK_SIGNAL_TV27M_CLK :
    case DDK_CLOCK_SIGNAL_TV54M_CLK :
    case DDK_CLOCK_SIGNAL_TV108M_CLK :
    case DDK_CLOCK_SIGNAL_UART_CLK:  
    case DDK_CLOCK_SIGNAL_FILT24M_CLK:            
    case DDK_CLOCK_SIGNAL_PWM24M_CLK:              
    case DDK_CLOCK_SIGNAL_DRI24M_CLK:               
    case DDK_CLOCK_SIGNAL_DIGCTL_CLK1M_CLK:                 
    case DDK_CLOCK_SIGNAL_TIMROT_32K_CLK:

    case DDK_CLOCK_SIGNAL_OCROM_CLK:         
    case DDK_CLOCK_SIGNAL_ADC_CLK:          
    case DDK_CLOCK_SIGNAL_ANA24M_CLK:  
    case DDK_CLOCK_SIGNAL_UTMI_CLK480M :   
    case DDK_CLOCK_SIGNAL_VDAC_CLK:         
    case DDK_CLOCK_SIGNAL_PCMSPDIF_CLK :
    case DDK_CLOCK_SIGNAL_REF_VID   :
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

#ifndef BOOTLOADER
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


