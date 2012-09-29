//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  serialutils.c
//
//  Support routines for configuring the serial port for SBOOT, the OAL,
//  and KITL.
//
//-----------------------------------------------------------------------------

#include <bsp.h>
#include <serialutils.h>

extern BSP_ARGS *g_pBSPArgs;

//-----------------------------------------------------------------------------
//
// FUNCTION:    OALConfigSerialUART
//
// DESCRIPTION:
//      Initializes the interal UART with the specified communication settings.
//
// PARAMETERS:
//      pSerInfo
//          [in]   Serial port configuration settings.
//
// RETURNS:
//      If this function succeeds, it returns TRUE, otherwise
//      it returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL OALConfigSerialUART(PSERIAL_INFO pSerInfo)
{
    UINT32 refClkFreq;
    PCSP_UART_REG pUART;



    pUART = (PCSP_UART_REG) OALPAtoUA(pSerInfo->uartBaseAddr);
    if (pUART == NULL)
    {

        return FALSE;
    }


    INSREG32BF(&pUART->UCR2,UART_UCR2_TXEN,UART_UCR2_TXEN_DISABLE);
    INSREG32BF(&pUART->UCR2,UART_UCR2_RXEN,UART_UCR2_RXEN_DISABLE);
    INSREG32BF(&pUART->UCR1,UART_UCR1_UARTEN,UART_UCR1_UARTEN_DISABLE);

    OUTREG32(&pUART->UCR2,
             CSP_BITFVAL(UART_UCR2_SRST, UART_UCR2_SRST_RESET)     |
             CSP_BITFVAL(UART_UCR2_RXEN, UART_UCR2_RXEN_ENABLE)    |
             CSP_BITFVAL(UART_UCR2_TXEN, UART_UCR2_TXEN_ENABLE)    |
             CSP_BITFVAL(UART_UCR2_ATEN, UART_UCR2_ATEN_DISABLE)   |
             CSP_BITFVAL(UART_UCR2_RTSEN, UART_UCR2_RTSEN_DISABLE) |
             CSP_BITFVAL(UART_UCR2_WS, pSerInfo->dataBits)         |
             CSP_BITFVAL(UART_UCR2_STPB, pSerInfo->stopBits)       |
             CSP_BITFVAL(UART_UCR2_PROE, pSerInfo->parity)         |
             CSP_BITFVAL(UART_UCR2_PREN, pSerInfo->bParityEnable)  |
             CSP_BITFVAL(UART_UCR2_RTEC, UART_UCR2_RTEC_RISEDGE)   |
             CSP_BITFVAL(UART_UCR2_ESCEN, UART_UCR2_ESCEN_DISABLE) |
             CSP_BITFVAL(UART_UCR2_CTS, UART_UCR2_CTS_HIGH)        |
             CSP_BITFVAL(UART_UCR2_CTSC, UART_UCR2_CTSC_RXCTRL)    |
             CSP_BITFVAL(UART_UCR2_IRTS, UART_UCR2_IRTS_IGNORERTS) |
             CSP_BITFVAL(UART_UCR2_ESCI, UART_UCR2_ESCI_DISABLE));

    // Software reset clears RX/TX state machines, FIFOs, and all status bits
    // which means all interrupts will be cleared

    // Wait until UART comes out of reset (reset asserted via UCR2 SRST)
    while (!(INREG32(&pUART->UCR2) & CSP_BITFMASK(UART_UCR2_SRST)))
        ; // Intentional polling loop.

    // RTS/CTS not required. Output an error message telling that this won't work because we didn't configure the pins for it
    if (pSerInfo->flowControl)
    {
         
        ERRORMSG(1,(TEXT("Hardware flow control is not supported.\r\n")));
        /*    OUTREG32(&pUART->UCR2,
                 INREG32(&pUART->UCR2) & ~CSP_BITFMASK(UART_UCR2_IRTS));
        OUTREG32(&pUART->UCR2,
                 INREG32(&pUART->UCR2) & ~CSP_BITFMASK(UART_UCR2_CTSC));
        INSREG32BF(&pUART->UCR2, UART_UCR2_CTS, UART_UCR2_CTS_LOW);*/
    }
    INSREG32BF(&pUART->UCR2,UART_UCR2_RXEN, UART_UCR2_RXEN_ENABLE);
    INSREG32BF(&pUART->UCR2,UART_UCR2_TXEN, UART_UCR2_TXEN_ENABLE);
    INSREG32BF(&pUART->UCR2,UART_UCR2_ATEN, UART_UCR2_ATEN_DISABLE);
    INSREG32BF(&pUART->UCR2,UART_UCR2_RTSEN, UART_UCR2_RTSEN_DISABLE);
    INSREG32BF(&pUART->UCR2,UART_UCR2_WS, pSerInfo->dataBits);
    INSREG32BF(&pUART->UCR2,UART_UCR2_STPB, pSerInfo->stopBits);
    INSREG32BF(&pUART->UCR2,UART_UCR2_PROE, pSerInfo->parity);
    INSREG32BF(&pUART->UCR2,UART_UCR2_PREN, pSerInfo->bParityEnable);
    INSREG32BF(&pUART->UCR2,UART_UCR2_ESCEN, UART_UCR2_ESCEN_DISABLE);
    INSREG32BF(&pUART->UCR2,UART_UCR2_CTS, UART_UCR2_CTS_HIGH);
    INSREG32BF(&pUART->UCR2,UART_UCR2_CTSC, UART_UCR2_CTSC_RXCTRL);
    INSREG32BF(&pUART->UCR2,UART_UCR2_IRTS, UART_UCR2_IRTS_IGNORERTS);
    INSREG32BF(&pUART->UCR2,UART_UCR2_ESCI, UART_UCR2_ESCI_DISABLE);


    OUTREG32(&pUART->UCR1,
             CSP_BITFVAL(UART_UCR1_UARTEN, UART_UCR1_UARTEN_ENABLE)      |
             CSP_BITFVAL(UART_UCR1_DOZE, UART_UCR1_DOZE_ENABLE)          |
             CSP_BITFVAL(UART_UCR1_TDMAEN, UART_UCR1_TXDMAEN_DISABLE)    |
             CSP_BITFVAL(UART_UCR1_SNDBRK, UART_UCR1_SNDBRK_NOBREAK)     |
             CSP_BITFVAL(UART_UCR1_RTSDEN, UART_UCR1_RTSDEN_DISABLE)     |
             CSP_BITFVAL(UART_UCR1_TXMPTYEN, UART_UCR1_TXMPTYEN_DISABLE) |
             CSP_BITFVAL(UART_UCR1_IREN, UART_UCR1_IREN_DISABLE)         |
             CSP_BITFVAL(UART_UCR1_RDMAEN, UART_UCR1_RXDMAEN_DISABLE)    |
             CSP_BITFVAL(UART_UCR1_RRDYEN, UART_UCR1_RRDYEN_DISABLE)     |
             CSP_BITFVAL(UART_UCR1_ICD, UART_UCR1_ICD_8FRAMES)           |
             CSP_BITFVAL(UART_UCR1_IDEN, UART_UCR1_IDEN_DISABLE)         |
             CSP_BITFVAL(UART_UCR1_TRDYEN, UART_UCR1_TRDYEN_DISABLE)     |
             CSP_BITFVAL(UART_UCR1_ADBR, UART_UCR1_ADBR_DISABLE)         |
             CSP_BITFVAL(UART_UCR1_ADEN, UART_UCR1_ADEN_DISABLE));

    OUTREG32(&pUART->UCR3,
             CSP_BITFVAL(UART_UCR3_ACIEN, UART_UCR3_ACIEN_DISABLE)       |
             CSP_BITFVAL(UART_UCR3_INVT, UART_UCR3_INVT_ACTIVELOW)       |
             CSP_BITFVAL(UART_UCR3_RXDMUXSEL, UART_UCR3_RXDMUXSEL_MUX)   |
             CSP_BITFVAL(UART_UCR3_DTRDEN, UART_UCR3_DTRDEN_DISABLE)     |
             CSP_BITFVAL(UART_UCR3_AWAKEN, UART_UCR3_AWAKEN_DISABLE)     |
             CSP_BITFVAL(UART_UCR3_AIRINTEN, UART_UCR3_AIRINTEN_DISABLE) |
             CSP_BITFVAL(UART_UCR3_RXDSEN, UART_UCR3_RXDSEN_DISABLE)     |
             CSP_BITFVAL(UART_UCR3_RI, UART_UCR3_RI_LOW)                 |
             CSP_BITFVAL(UART_UCR3_DCD, UART_UCR3_DCD_LOW)               |
             CSP_BITFVAL(UART_UCR3_DSR, UART_UCR3_DSR_LOW)               |
             CSP_BITFVAL(UART_UCR3_FRAERREN, UART_UCR3_FRAERREN_DISABLE) |
             CSP_BITFVAL(UART_UCR3_PARERREN, UART_UCR3_PARERREN_DISABLE) |
             CSP_BITFVAL(UART_UCR3_DTREN, UART_UCR3_DTREN_DISABLE)       |
             CSP_BITFVAL(UART_UCR3_DPEC, UART_UCR3_DPEC_RISEDGE));

    OUTREG32(&pUART->UCR4,
             CSP_BITFVAL(UART_UCR4_DREN, UART_UCR4_DREN_DISABLE)   |
             CSP_BITFVAL(UART_UCR4_OREN, UART_UCR4_OREN_DISABLE)   |
             CSP_BITFVAL(UART_UCR4_BKEN, UART_UCR4_BKEN_DISABLE)   |
             CSP_BITFVAL(UART_UCR4_TCEN, UART_UCR4_TCEN_DISABLE)   |
             CSP_BITFVAL(UART_UCR4_LPBYP, UART_UCR4_LPBYP_DISABLE) |
             CSP_BITFVAL(UART_UCR4_IRSC, UART_UCR4_IRSC_SAMPCLK)   |
             CSP_BITFVAL(UART_UCR4_WKEN, UART_UCR4_WKEN_DISABLE)   |
             CSP_BITFVAL(UART_UCR4_ENIRI, UART_UCR4_ENIRI_DISABLE) |
             CSP_BITFVAL(UART_UCR4_INVR, UART_UCR4_INVR_ACTIVELOW) |
             CSP_BITFVAL(UART_UCR4_CTSTL, 14));

    OUTREG32(&pUART->UFCR,
             CSP_BITFVAL(UART_UFCR_RXTL, 24)                     |
             CSP_BITFVAL(UART_UFCR_DCEDTE, UART_UFCR_DCEDTE_DCE) |
             CSP_BITFVAL(UART_UFCR_RFDIV, UART_UFCR_RFDIV_DIV2)  |
             CSP_BITFVAL(UART_UFCR_TXTL, 4));

    OUTREG32(&pUART->UTS,
             CSP_BITFVAL(UART_UTS_RXDBG, UART_UTS_RXDBG_NOINCREMENT) |
             CSP_BITFVAL(UART_UTS_LOOPIR, UART_UTS_LOOPIR_NOLOOP)    |
             CSP_BITFVAL(UART_UTS_DBGEN, UART_UTS_DBGEN_DEBUG)       |
             CSP_BITFVAL(UART_UTS_LOOP, UART_UTS_LOOP_NOLOOP)        |
             CSP_BITFVAL(UART_UTS_FRCPERR, UART_UTS_FRCPERR_NOERROR));

    // Determine the UART_REF_CLK frequency
    
    refClkFreq = g_pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_UART];

    // BAUD_RATE = 115200
    // RFFDIV set to /2 above
    // UART_REF_CLK = PERCLK / RFDIV
    // UBIR = (BAUD_RATE / 100) - 1
    // UBMR = (UART_REF_CLK / 1600) - 1
    OUTREG32(&pUART->UBIR, (pSerInfo->baudRate / 100) - 1);
    OUTREG32(&pUART->UBMR, ((refClkFreq / 2) / 1600) - 1);

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// FUNCTION:    OALConfigSerialIOMUX
//
// DESCRIPTION:
//      Configures the respective UART's IOMUX settings.
//
// PARAMETERS:
//      uartBaseAddr
//          [in] Physical Base address of corresponding UART
//
//      pIOMUX
//          [in] Pointer to the IOMUX control registers
//
//      pPBC
//          [in] Pointer to the PBC control registers
//
// RETURNS:
//      If this function succeeds, it returns TRUE, otherwise
//      it returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL OALConfigSerialIOMUX (DWORD uartBaseAddr, PCSP_IOMUX_REGS pIOMUX)
{
    BOOL bFlag = TRUE;

    switch (uartBaseAddr)
    {
        case CSP_BASE_REG_PA_UART1:
            OAL_IOMUX_SET_MUX(pIOMUX,
                              DDK_IOMUX_PIN_UART1_RXD, DDK_IOMUX_PIN_MUXMODE_ALT0,
                              DDK_IOMUX_PIN_SION_REGULAR);
            OAL_IOMUX_SET_MUX(pIOMUX,
                              DDK_IOMUX_PIN_UART1_TXD, DDK_IOMUX_PIN_MUXMODE_ALT0,
                              DDK_IOMUX_PIN_SION_REGULAR);
            /* RTS/CTS not required
            OAL_IOMUX_SET_MUX(pIOMUX,
                              DDK_IOMUX_PIN_UART1_RTS, DDK_IOMUX_PIN_MUXMODE_ALT0,
                              DDK_IOMUX_PIN_SION_REGULAR);
            OAL_IOMUX_SET_MUX(pIOMUX,
                              DDK_IOMUX_PIN_UART1_CTS, DDK_IOMUX_PIN_MUXMODE_ALT0,
                              DDK_IOMUX_PIN_SION_REGULAR);
                              */
            break;
        //additional serial ports here
        default:
            // Unknown or invalid UART selected.
            bFlag = FALSE;
    }

    return bFlag;
}
