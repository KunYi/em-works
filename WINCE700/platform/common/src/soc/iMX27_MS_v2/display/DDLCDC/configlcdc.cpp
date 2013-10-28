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
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  configlcdc.cpp
//
//  Provides BSP-specific configuration routines for the Display driver.
//  Includes initialization routine for the TV out encoder chip.
//
//------------------------------------------------------------------------------

#include "precomp.h"


//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------
extern void   BSPTurnOnLCD(void);
extern void   BSPTurnOffLCD(void);

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Function: LCDCEnable
//
// This function enable or disable Lcdc gpio and clock for LCD panel mode.
//
// Parameters:
//      bEnable
//          [in] This argument is enable or disable Lcdc gpio and clock for LCD panel mode.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
static void LCDCEnable(BOOL bEnable)
{
    BOOL initState;
    DDK_GPIO_CFG cfg;

    DDK_GPIO_SET_CONFIG(cfg, LCDC);

    if(bEnable)
    {
        // Configure LCDC GPIO pins
        initState = DDKGpioEnable(&cfg);

        initState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_ENABLE);
        initState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_HCLK_LCDC, DDK_CLOCK_GATE_MODE_ENABLE);
    }
    else
    {
        // Disable LCDC
        DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_DISABLE);
        DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_HCLK_LCDC, DDK_CLOCK_GATE_MODE_DISABLE);

        // Reset LCDC GPIO pins
        DDKGpioDisable(&cfg);
    }
}

//------------------------------------------------------------------------------
//
// Function: InitializeLCDC
//
// This function init and config Lcdc.
//
// Parameters:
//      bTVModeActive
//          [in] This argument choose config for TV mode or LCD mode.
//      bTVNTSCOut
//          [in] This argument choose config for NTSC or PAL mode.
//      dwPanelType
//          [in] This argument choose config for SHARP QVGA or NEC VGA mode.
//
// Returns:
//      Returns TRUE if success.
//      Returns FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL InitializeLCDC(BOOL bTVModeActive, BOOL bTVNTSCOut, DWORD dwPanelType)
{
    BOOL                  rc = FALSE;
    PHYSICAL_ADDRESS      phyAddr;
    CSP_LCDC_REGS         *pLCDC;
    LCDC_CTX              CLcdcCtx;
    GPEMode               *pCLcdcMode;
    struct DisplayPanel   *pDisplayPanel;
    DWORD                 dPCR;
#ifndef USE_EPSON_TFT
    PCSP_PLLCRC_REGS      g_pPLLCRC;
    DWORD                 dHtotal, dVtotal, dPCDR1DIV3;
    UINT32                uLCDRefClk, uLCDPixClk;

    phyAddr.QuadPart = CSP_BASE_REG_PA_CRM;
    // Map peripheral physical address to virtual address
    g_pPLLCRC = (PCSP_PLLCRC_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_PLLCRC_REGS),
            FALSE);
    if (g_pPLLCRC  ==  NULL)
    {
        DEBUGMSG (1, (TEXT("MmMapIoSpace CSP_PLLCRC_REGS Failed\r\n")) );
        goto cleanup;
    }
#endif

    phyAddr.QuadPart = CSP_BASE_REG_PA_LCDC;
    pLCDC = (CSP_LCDC_REGS *)MmMapIoSpace(phyAddr, sizeof(CSP_LCDC_REGS), FALSE);
    if (pLCDC  ==  NULL)
    {
        DEBUGMSG (1, (TEXT("MmMapIoSpace CSP_LCDC_REGS Failed\r\n")) );
        goto cleanup;
    }

    pDisplayPanel = BSPConfigPanel(bTVModeActive, bTVNTSCOut, dwPanelType);
    BSPInitLCDC(&CLcdcCtx);
    pCLcdcMode = (GPEMode*)(CLcdcCtx.pGPEModeInfo);

    // Disable LCD panel
    BSPTurnOffLCD();
    // Disable LCDC clock
    LCDCEnable(FALSE);

    // LCDC Refresh Mode Control Register
    CLRREG32(&pLCDC->RMCR, CSP_BITFMASK(LCDC_RMCR_SELF_REF));

    // LCDC Screen Start Address Register
    OUTREG32(&pLCDC->SSAR,
              (CLcdcCtx.VideoMemoryPhyAdd));

    // LCDC Size Register
    OUTREG32(&pLCDC->SR,
              (CSP_BITFVAL(LCDC_SR_BUSSIZE, LCDC_SR_BUSSIZE_32BIT)|  // ???
              CSP_BITFVAL(LCDC_SR_XMAX, (pDisplayPanel->width / 16))|
              CSP_BITFVAL(LCDC_SR_YMAX, (pDisplayPanel->height))));

    // LCDC Virtual Page Width Register
    OUTREG32(&pLCDC->VPWR,
              (pDisplayPanel->width / (32/pCLcdcMode->Bpp))); // the number of 32-bit words

    // LCDC Cursor Position Register
    OUTREG32(&pLCDC->CPR,
                (CSP_BITFVAL(LCDC_CPR_OP, LCDC_CPR_OP_DISABLE) |
                CSP_BITFVAL(LCDC_CPR_CC, LCDC_CPR_CC_DISABLED)));

    // LCDC Cursor Width Height and Blink Register
    OUTREG32(&pLCDC->CWHBR,
                  (CSP_BITFVAL(LCDC_CWHBR_BK_EN, LCDC_CWHBR_BK_EN_DISABLE) |
                  CSP_BITFVAL(LCDC_CWHBR_CW, LCDC_CWHBR_CW_CURSOR_DISABLED) |
                  CSP_BITFVAL(LCDC_CWHBR_CH, LCDC_CWHBR_CH_CURSOR_DISABLED) |
                  CSP_BITFVAL(LCDC_CWHBR_BD, LCDC_CWHBR_BD_MAX_DIV)));

    // LCDC Color Cursor Mapping Register
    OUTREG32(&pLCDC->CCMR, 0);

    // LCDC Panel Configuration Register
#ifdef USE_EPSON_TFT
    // Epson TFT Panel Configuration Register:
    //    31: TFT display
    //    30: Color display
    // 28-29: 8-bit panel bus width (required for color)
    // 25-27: BPP: 16bpp
    //    24: Active high pixel polarity
    //    23: Active low first line marker polarity
    //    22: Active low line pulse polarity
    //    21: Active on negative edge of LSCLK
    //    20: Active high output enable signal polarity
    //    19: Enable LSCLK when VSYNC is idle
    //    18: Image is little endian in memory
    //    17: Little endian swap mode on
    //    16: Vertical scan in normal direction
    //    15: Use LP/HSYN as clock source for alternative crystal direction counter (ACD)
    //  8-14: Unused for TFT
    //     7: Always enable LSCLK in TFT even if there is no data output
    //     6: Disable Sharp HR-TFT 240x320 signals
    //  0- 5: Pixel clock divide rate of 2 (value + 1), gives refresh rate of ~34 fps
    dPCR =
            CSP_BITFVAL(LCDC_PCR_SHARP, LCDC_PCR_SHARP_DISABLE) |
            CSP_BITFVAL(LCDC_PCR_PIXPOL, LCDC_PCR_PIXPOL_ACTIVE_HIGH) |
            CSP_BITFVAL(LCDC_PCR_SCLKSEL, LCDC_PCR_SCLKSEL_ENABLE) |
            CSP_BITFVAL(LCDC_PCR_ACDSEL, LCDC_PCR_ACDSEL_USE_LPHSYNC) |
            CSP_BITFVAL(LCDC_PCR_REV_VS, LCDC_PCR_REV_VS_NORMAL) |
            CSP_BITFVAL(LCDC_PCR_SWAP_SEL, LCDC_PCR_SWAP_SEL_16BPP) |
            CSP_BITFVAL(LCDC_PCR_END_SEL, LCDC_PCR_END_SEL_LITTLE_ENDIAN) |
            CSP_BITFVAL(LCDC_PCR_SCLKIDLE, LCDC_PCR_SCLKIDLE_ENABLE) |
            CSP_BITFVAL(LCDC_PCR_OEPOL, LCDC_PCR_OEPOL_ACTIVE_HIGH) |
            CSP_BITFVAL(LCDC_PCR_CLKPOL, LCDC_PCR_CLKPOL_NEG_EDGE) |
            CSP_BITFVAL(LCDC_PCR_LPPOL, LCDC_PCR_LPPOL_ACTIVE_LOW) |
            CSP_BITFVAL(LCDC_PCR_FLMPOL, LCDC_PCR_FLMPOL_ACTIVE_LOW) |
            CSP_BITFVAL(LCDC_PCR_PBSIZ, LCDC_PCR_PBSIZ_8BIT) |
            CSP_BITFVAL(LCDC_PCR_COLOR, LCDC_PCR_COLOR_COLOR) |
            CSP_BITFVAL(LCDC_PCR_TFT, LCDC_PCR_TFT_ACTIVE) |
            CSP_BITFVAL(LCDC_PCR_PCD, 1);
#else
    if(bTVModeActive)
    {
        dPCDR1DIV3 = 1;
        INSREG32BF(&g_pPLLCRC->PCDR1, PLLCRC_PCDR1_PERDIV3, (dPCDR1DIV3-1));
        uLCDRefClk = 266400000 / dPCDR1DIV3;
    }
    else
    {
        dPCDR1DIV3 = EXTREG32BF(&g_pPLLCRC->PCDR1, PLLCRC_PCDR1_PERDIV3)+1;
        uLCDRefClk = 266000000 / dPCDR1DIV3;
    }
    dHtotal = pDisplayPanel->width + pDisplayPanel->hsync_width + pDisplayPanel->hwait1
        + pDisplayPanel->hwait2;
    dVtotal = pDisplayPanel->height + pDisplayPanel->vsync_width + pDisplayPanel->vwait1
        + pDisplayPanel->vwait2;
    uLCDPixClk = dHtotal*dVtotal*pCLcdcMode->frequency;
    if(bTVModeActive)
    {
        dPCR =
              CSP_BITFVAL(LCDC_PCR_SHARP, LCDC_PCR_SHARP_DISABLE) |
              CSP_BITFVAL(LCDC_PCR_SCLKIDLE, LCDC_PCR_SCLKIDLE_ENABLE) |
              CSP_BITFVAL(LCDC_PCR_ACDSEL, LCDC_PCR_ACDSEL_USE_FRM)|
              CSP_BITFVAL(LCDC_PCR_OEPOL, LCDC_PCR_OEPOL_ACTIVE_LOW) |
              CSP_BITFVAL(LCDC_PCR_CLKPOL, LCDC_PCR_CLKPOL_POS_EDGE) |
              CSP_BITFVAL(LCDC_PCR_PIXPOL, LCDC_PCR_PIXPOL_ACTIVE_HIGH);
    }
    else
    {
        if(dwPanelType == 0) //QVGA
        {
            dPCR =
                  CSP_BITFVAL(LCDC_PCR_SHARP, LCDC_PCR_SHARP_ENABLE) |
                  CSP_BITFVAL(LCDC_PCR_PIXPOL, LCDC_PCR_PIXPOL_ACTIVE_LOW);
        }
        else // VGA
        {
            dPCR =
                  CSP_BITFVAL(LCDC_PCR_SHARP, LCDC_PCR_SHARP_DISABLE) |
                  CSP_BITFVAL(LCDC_PCR_PIXPOL, LCDC_PCR_PIXPOL_ACTIVE_HIGH);
        }
        dPCR |=
              CSP_BITFVAL(LCDC_PCR_SCLKIDLE, LCDC_PCR_SCLKIDLE_DISABLE) |
              CSP_BITFVAL(LCDC_PCR_OEPOL, LCDC_PCR_OEPOL_ACTIVE_HIGH) |
              CSP_BITFVAL(LCDC_PCR_CLKPOL, LCDC_PCR_CLKPOL_NEG_EDGE) |
              CSP_BITFVAL(LCDC_PCR_ACDSEL, LCDC_PCR_ACDSEL_USE_LPHSYNC);
    }
    dPCR |=
              CSP_BITFVAL(LCDC_PCR_SCLKSEL, LCDC_PCR_SCLKSEL_ENABLE) |
              CSP_BITFVAL(LCDC_PCR_REV_VS, LCDC_PCR_REV_VS_NORMAL) |
              CSP_BITFVAL(LCDC_PCR_SWAP_SEL, LCDC_PCR_SWAP_SEL_16BPP) |
              CSP_BITFVAL(LCDC_PCR_END_SEL, LCDC_PCR_END_SEL_LITTLE_ENDIAN) |
              CSP_BITFVAL(LCDC_PCR_LPPOL, LCDC_PCR_LPPOL_ACTIVE_HIGH) |
              CSP_BITFVAL(LCDC_PCR_FLMPOL, LCDC_PCR_FLMPOL_ACTIVE_HIGH) |
              CSP_BITFVAL(LCDC_PCR_PBSIZ, LCDC_PCR_PBSIZ_8BIT) |
              CSP_BITFVAL(LCDC_PCR_COLOR, LCDC_PCR_COLOR_COLOR) |
              CSP_BITFVAL(LCDC_PCR_TFT, LCDC_PCR_TFT_ACTIVE) |
              CSP_BITFVAL(LCDC_PCR_PCD, LCDC_PCD_VALUE(uLCDRefClk, uLCDPixClk));
#endif
    switch(pCLcdcMode->Bpp)
    {
        case 1:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_1BPP);
            break;
        case 2:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_2BPP);
            break;
        case 4:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_4BPP);
            break;
        case 8:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_8BPP);
            break;
        case 12:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_12BPP);
            break;
        case 16:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_16BPP);
            break;
        case 18:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_18BPP);
            break;
        default:
            goto cleanup;
    }

    OUTREG32(&pLCDC->PCR, dPCR);

    // LCDC Horizontal Configuration Register
    OUTREG32(&pLCDC->HCR,
              (CSP_BITFVAL(LCDC_HCR_H_WIDTH, (pDisplayPanel->hsync_width - 1))|
              CSP_BITFVAL(LCDC_HCR_H_WAIT_1, (pDisplayPanel->hwait1 - 1))|
              CSP_BITFVAL(LCDC_HCR_H_WAIT_2, (pDisplayPanel->hwait2 - 3))));

    // LCDC Vertical Configuration Register
    OUTREG32(&pLCDC->VCR,
              (CSP_BITFVAL(LCDC_VCR_V_WIDTH, (pDisplayPanel->vsync_width))|
              CSP_BITFVAL(LCDC_VCR_V_WAIT_1, (pDisplayPanel->vwait1))|
              CSP_BITFVAL(LCDC_VCR_V_WAIT_2, (pDisplayPanel->vwait2))));

    // LCDC Panning Offset Register
    OUTREG32(&pLCDC->POR, 0);

    // LCDC Sharp Configuration Register
    OUTREG32(&pLCDC->SCR,
          (CSP_BITFVAL(LCDC_SCR_GRAY1, 0) |
          CSP_BITFVAL(LCDC_SCR_GRAY2, 0) |
          CSP_BITFVAL(LCDC_SCR_REV_TOGGLE_DELAY, 3) |
          CSP_BITFVAL(LCDC_SCR_CLS_RISE_DELAY, 18) |
          CSP_BITFVAL(LCDC_SCR_PS_RISE_DELAY, 0))); // 0001 = 2 LSCLK period

    // LCDC PWM Contrast Control Register
    OUTREG32(&pLCDC->PCCR,
          (CSP_BITFVAL(LCDC_PCCR_PW, LCDC_PCCR_PW_MAX / 2) |
          CSP_BITFVAL(LCDC_PCCR_CC_EN, LCDC_PCCR_CC_EN_ENABLE) |
          CSP_BITFVAL(LCDC_PCCR_SCR, LCDC_PCCR_SCR_PIXELCLK) |
          CSP_BITFVAL(LCDC_PCCR_LDMSK, LCDC_PCCR_LDMSK_DISABLE) |
          CSP_BITFVAL(LCDC_PCCR_CLS_HI_WIDTH, 169)));

    // LCDC DMA Control Register
    OUTREG32(&pLCDC->DCR,
              (CSP_BITFVAL(LCDC_DCR_BURST, LCDC_DCR_BURST_DYNAMIC)|
              CSP_BITFVAL(LCDC_DCR_HM, (0x03))|   // DMA High Mark
              CSP_BITFVAL(LCDC_DCR_TM, (0x1D)))); // DMA Trigger Mark

    // LCDC Interrupt Configuration Register
    // Interrupt flag is set on loading last data of frame from memory.
    OUTREG32(&pLCDC->ICR,
          (CSP_BITFVAL(LCDC_ICR_GW_INT_CON, LCDC_ICR_GW_INT_CON_END) |
          CSP_BITFVAL(LCDC_ICR_INTSYN, LCDC_ICR_INTSYN_MEMORY) |
          CSP_BITFVAL(LCDC_ICR_INTCON, LCDC_ICR_INTCON_EOF)));

    // LCDC Interrupt Enable Register
    OUTREG32(&pLCDC->IER, 0);

    // LCDC Graphic Window

    // LCDC Graphic Window Start Address Register
    OUTREG32(&pLCDC->GWSAR,
              (CLcdcCtx.VideoMemoryPhyAdd));

    // LCDC Graphic Window DMA Control Register
    OUTREG32(&pLCDC->GWDCR,
              (CSP_BITFVAL(LCDC_GWDCR_GWBT, LCDC_GWDCR_GWBT_DYNAMIC)|
              CSP_BITFVAL(LCDC_GWDCR_GWHM, 0x02)|
              CSP_BITFVAL(LCDC_GWDCR_GWTM, 0x10)));

    // Graphic window at first time can only be enabled while the HCLK to the LCDC is disabled.
    // Once enabled it can subsequently be disabled and enabled without turning off the HCLK.
    // So we need to enable and then disable the graphic window at hardware init part(configlcdc),
    // then at next time to enable graphic window, the HCLK to LCDC does not need to be disabled, and the flicker (due to disabling of HCLK to LCDC) is avoided.
    {
        // Enable graphic window
        // LCDC Graphic Window Size Register
        OUTREG32(&pLCDC->GWSR,
                  (CSP_BITFVAL(LCDC_GWSR_GWW, (pDisplayPanel->width >> 4))|
                  CSP_BITFVAL(LCDC_GWSR_GWH, pDisplayPanel->height)));

        // LCDC Graphic Window Virtual Page Width Register
        OUTREG32(&pLCDC->GWVPWR,
                  (pDisplayPanel->width / (32/pCLcdcMode->Bpp))); // the number of 32-bit words

        // LCDC Graphic Window Position Register
        OUTREG32(&pLCDC->GWPR,
                  (CSP_BITFVAL(LCDC_GWPR_GWXP, 0)|
                  CSP_BITFVAL(LCDC_GWPR_GWYP, 0)));

        // LCDC Graphic Window Panning Offset Register
        OUTREG32(&pLCDC->GWPOR,
                  16);

        // LCDC Graphic Window Control Registers
        OUTREG32(&pLCDC->GWCR,
                  (CSP_BITFVAL(LCDC_GWCR_GWAV, LCDC_GWCR_GWAV_OPAQUE)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKE, LCDC_GWCR_GWCKE_DISABLE)|
                  CSP_BITFVAL(LCDC_GWCR_GWE, LCDC_GWCR_GWE_ENABLE)|
                  CSP_BITFVAL(LCDC_GWCR_GW_RVS, LCDC_GWCR_GW_RVS_NORMAL)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKR, 0)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKG, 0)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKB, 0)));

        // Disable graphic window
        CLRREG32(&pLCDC->GWCR, CSP_BITFMASK(LCDC_GWCR_GWAV));

        // LCDC Graphic Window Size Register
        OUTREG32(&pLCDC->GWSR,
                  (CSP_BITFVAL(LCDC_GWSR_GWW, (16 >> 4))|
                  CSP_BITFVAL(LCDC_GWSR_GWH, 16)));
        Sleep(100);

        CLRREG32(&pLCDC->GWCR, CSP_BITFMASK(LCDC_GWCR_GWE));
    }

    LCDCEnable(TRUE);

    if (bTVModeActive)
    {
#ifdef ENABLE_TV_ON
        // Initialize and turn on TV Out
        if (!BSPInitializeTVOut(bTVNTSCOut))
        {
            DEBUGMSG(1, (TEXT("InitializeTVOut Failed")));
            goto cleanup;
        }
#endif
    }
    else
    {
        // Turn on lcd panel
        BSPTurnOnLCD();

#ifdef ENABLE_TV_ON
        // Disable TV Out if LCD active
        if (!BSPDeinitializeTVOut())
        {
            DEBUGMSG(1, (TEXT("DeinitializeTVOut Failed")));
            // goto cleanup; // No impact to lcdc
        }
#endif
    }

    rc = TRUE;

cleanup:
    return rc;
}
