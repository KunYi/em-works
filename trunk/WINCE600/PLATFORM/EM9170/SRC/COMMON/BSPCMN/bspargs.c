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
// File: bspargs.c
//
// The initialization of BSP ARGS.
//
//-----------------------------------------------------------------------------
#include <bsp.h>

//-----------------------------------------------------------------------------
// External Variables
extern PCSP_CRM_REGS g_pCRM;


//-----------------------------------------------------------------------------
// Global Variables
PDDK_CLK_CONFIG g_pDdkClkConfig;


//------------------------------------------------------------------------------
//
// Function: OALBspArgsInit
//
// This function initializes the parameters of the global BSP args structure.
//
// Parameters:
//      pBSPArgs
//          [out] Points to BSP arguments structure to be updated.
//
// Returns:
//      TRUE if boot successfully, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL OALBspArgsInit(BSP_ARGS *pBSPArgs)
{
    UINT32 pllRef, mcuPllFreq, usbPllFreq, armFreq, ahbFreq, ipgClkFreq, usbclkFreq;
    UINT32 pctl, mfi, mfn, mfd, pdf;
    DDK_DVFC_DOMAIN domain;
    DDK_DVFC_SETPOINT setpoint;

    g_pDdkClkConfig = (PDDK_CLK_CONFIG) IMAGE_WINCE_DDKCLK_RAM_UA_START;
    if (g_pDdkClkConfig == NULL)
    {
        return FALSE;
    }

    for (domain = 0; domain < DDK_DVFC_DOMAIN_ENUM_END; domain++)
    {
        for (setpoint = 0; setpoint < DDK_DVFC_SETPOINT_ENUM_END; setpoint++)
        {
            // Initialize setpoint requests
            g_pDdkClkConfig->setpointReqCount[domain][setpoint] = 0;
        }

        // Initialize load tracking requests
        g_pDdkClkConfig->setpointLoad[domain] = DDK_DVFC_SETPOINT_ENUM_END-1;
    }
    
    // Initialize DVFC state variables
    g_pDdkClkConfig->bDvfcActive = FALSE;
    g_pDdkClkConfig->bSetpointPending = FALSE;
    g_pDdkClkConfig->setpointCur[DDK_DVFC_DOMAIN_CPU] = DDK_DVFC_SETPOINT_HIGH;
    g_pDdkClkConfig->setpointMin[DDK_DVFC_DOMAIN_CPU] = DDK_DVFC_SETPOINT_LOW;
    g_pDdkClkConfig->setpointMax[DDK_DVFC_DOMAIN_CPU] = DDK_DVFC_SETPOINT_HIGH;

    // PLL reference is always CKIH on MX25
    pllRef = BSP_CLK_CKIH_FREQ;

    
    if (CSP_BITFEXT(g_pCRM->CCTL,CRM_CCTL_MPLL_BYPASS))
    {
        mcuPllFreq = pllRef;
    }
    else
    {
        // Calculate MCU PLL frequency
        pctl = INREG32(&g_pCRM->MPCTL);
        mfi = CSP_BITFEXT(pctl, CRM_MPCTL_MFI);
        mfn = CSP_BITFEXT(pctl, CRM_MPCTL_MFN);
        mfd = CSP_BITFEXT(pctl, CRM_MPCTL_MFD);
        pdf = CSP_BITFEXT(pctl, CRM_MPCTL_PDF);
        mcuPllFreq = (UINT32) (((2 * pllRef * mfi) + ((UINT64) 2 * pllRef * mfn)/(mfd+1)) / (pdf+1));
    }

    // Calculate USB PLL frequency
    pctl = INREG32(&g_pCRM->UPCTL);
    mfi = CSP_BITFEXT(pctl, CRM_MPCTL_MFI);
    mfn = CSP_BITFEXT(pctl, CRM_MPCTL_MFN);
    mfd = CSP_BITFEXT(pctl, CRM_MPCTL_MFD);
    pdf = CSP_BITFEXT(pctl, CRM_MPCTL_PDF);
    usbPllFreq = (UINT32) (((2 * pllRef * mfi) + ((UINT64) 2 * pllRef * mfn)/(mfd+1)) / (pdf+1));




    // Calculate ARM, AHB, HSP and MLB frequency as per the selected clock path
    {
        BOOL bUseExternal266M;      
        BOOL bARMDividerSourceIs075;
        DWORD dwARMClkDiv;
        DWORD dwAHBClkDiv;
        DWORD dwUSBDiv;
        
        bUseExternal266M = CSP_BITFEXT(g_pCRM->RCSR,CRM_SCSR_CLK_SEL);
        
        bARMDividerSourceIs075 = CSP_BITFEXT(g_pCRM->CCTL,CRM_CCTL_ARM_SRC);
        dwARMClkDiv = CSP_BITFEXT(g_pCRM->CCTL,CRM_CCTL_ARMCLK_DIV);
        dwAHBClkDiv = CSP_BITFEXT(g_pCRM->CCTL,CRM_CCTL_AHBCLK_DIV);
        dwUSBDiv = CSP_BITFEXT(g_pCRM->CCTL,CRM_CCTL_USB_DIV);



        if (bUseExternal266M)
        {
            armFreq = BSP_CLK_EXT266_FREQ;
        }
        else
        {
            if (bARMDividerSourceIs075)
            {
                armFreq = (mcuPllFreq*3)/4;
            }
            else
            {
                armFreq = mcuPllFreq;
            }
        }

        armFreq = armFreq / (dwARMClkDiv+1);
        ahbFreq = armFreq / (dwAHBClkDiv+1);
        ipgClkFreq = ahbFreq / 2;
        usbclkFreq = usbPllFreq / (dwUSBDiv+1);
    }

        // Initialize PLL clock configuration
    pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_MCUPLL] = mcuPllFreq;
    pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_USBPLL] = usbPllFreq;
    pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_ARM] = armFreq;
    pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB] = ahbFreq;
    pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_IPG] = ipgClkFreq;
    pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_USBCLK] = usbclkFreq;
    
//-----------------------------------------------------------------------------
// Here use some helper macro
#define GET_PER_DIV(pCRM, ClkIndex) \
    (CRM_PCDR_VAL_EXTRACT(pCRM->PCDR_REGS.PCDR[CRM_PCDR_INDEX(DDK_INDEX_TO_CPU_INDEX(ClkIndex))],\
    DDK_INDEX_TO_CPU_INDEX(ClkIndex)))                                      // get the divider for the specified clock (based on the DDK index)
#define FILL_PER_CLK_ENTRY(DDKIndex) pBSPArgs->clockFreq[DDKIndex] = ahbFreq / (GET_PER_DIV(g_pCRM,DDKIndex)+1) //fill the entry with the correct clock frequency

    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_CSI);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_EPIT);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_ESAI);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_ESDHC1);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_ESDHC2);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_GPT);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_I2C);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_LCDC);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_NFC);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_OWIRE);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_PWM);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_SIM1);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_SIM2);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_SSI1);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_SSI2);
    FILL_PER_CLK_ENTRY(DDK_CLOCK_SIGNAL_PER_UART);
    
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:   OALBspArgsPrint
//
//  This function prints the parameters of the global BSP args structure.
//
//  Parameters:
//      pBSPArgs
//          [in] Points to BSP arguments structure to be printed.
//
//  Returns:
//      None
//
//------------------------------------------------------------------------------
VOID OALBspArgsPrint(BSP_ARGS *pBSPArgs)
{
    if (pBSPArgs == NULL)
    {
        goto cleanUp;
    }

    // Print system configuration
    OALMSG(OAL_INFO, (TEXT("BSP Clock Configuration:\r\n")));
    
    OALMSG(OAL_INFO, (TEXT("    MCU PLL  = %d Hz\r\n"),
           pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_MCUPLL]));
    OALMSG(OAL_INFO, (TEXT("    USB PLL  = %d Hz\r\n"),
           pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_USBPLL]));
    OALMSG(OAL_INFO, (TEXT("    ARM CLOCK  = %d Hz\r\n"),
           pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_ARM]));
    OALMSG(OAL_INFO, (TEXT("    AHB CLOCK  = %d Hz\r\n"),
           pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB]));
    OALMSG(OAL_INFO, (TEXT("    IPG CLOCK  = %d Hz\r\n"),
           pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_IPG]));
    OALMSG(OAL_INFO, (TEXT("    USB CLOCK = %d Hz\r\n"),
           pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_USBCLK]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk CSI = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_CSI]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk EPIT = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_EPIT]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk ESAI = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_ESAI]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk ESDHC1 = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_ESDHC1]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk ESDHC2 = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_ESDHC2]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk GPT = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_GPT]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk I2C = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_I2C]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk LCDC = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_LCDC]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk NFC = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_NFC]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk OWIRE = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_OWIRE]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk PWM = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_PWM]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk SIM1 = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_SIM1]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk SIM2 = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_SIM2]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk SSI1 = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_SSI1]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk SSI2 = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_SSI2]));
    OALMSG(OAL_INFO, (TEXT("    Periph Clk UART = %d Hz\r\n"),
        pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_UART]));

cleanUp:
    return;
}
