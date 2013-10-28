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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// FILE:    kitlserial.c
//
// PURPOSE: This module provides the OAL functions for supporting a
//          serial tranport for KITL.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// INCLUDE FILES
//-----------------------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include "csp.h"

//-----------------------------------------------------------------------------
// EXTERNAL FUNCTIONS
//-----------------------------------------------------------------------------
extern UINT OALGetSerialRefClock(void);

//-----------------------------------------------------------------------------
// GLOBAL DEFINITIONS
//-----------------------------------------------------------------------------
#define UART_UCR2_CTS_ACTIVE	0x1000
#define UART_RX_FRAMING_ERR	0x1000
#define UART_RX_PARITY_ERR		0x400
#define UART_RX_OVERRUN		0x2000
#define UART_RX_ERRORS		(UART_RX_FRAMING_ERR | UART_RX_PARITY_ERR | UART_RX_OVERRUN)


//-----------------------------------------------------------------------------
// GLOBAL VARIABLES
//-----------------------------------------------------------------------------
static PCSP_UART_REG g_pUART;


//-----------------------------------------------------------------------------
// STATIC FUNCTIONS
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialInit
//
// DESCRIPTION:
//      Initializes the interal UART with the specified communication settings.
//
// PARAMETERS:
//      pSerInfo
//          [in/out] Pointer to KITL_SERIAL_INFO structure that contains
//          information about how to initialize the serial KITL transport.
//
// RETURNS:
//      If this function succeeds, it returns TRUE, otherwise
//      it returns FALSE.
//
//-----------------------------------------------------------------------------

BOOL SerialInit(KITL_SERIAL_INFO *pSerInfo)
{    
    // Get the reference clock
    UINT32 refClkFreq = OALGetSerialRefClock();

	OALMSG(1, (TEXT("+SerialInit.\r\n")));
    // Get pointer to peripheral base
    g_pUART = (PCSP_UART_REG) pSerInfo->pAddress;

	
    if (g_pUART == NULL) return FALSE;

    // Tell KITL upper layers that read it the number of bytes per time.
    pSerInfo->bestSize = UART_TXFIFO_DEPTH;    
  
     // Disable UART
    OUTREG32(&g_pUART->UCR1, 0);

    // Reset UART
    OUTREG32(&g_pUART->UCR2, 0);

    // Software reset clears RX/TX state machines, FIFOs, and all status bits
    // which means all interrupts will be cleared

    // Wait until UART comes out of reset (reset asserted via UCR2 SRST)
    while (!(INREG32(&g_pUART->UCR2) & CSP_BITFMASK(UART_UCR2_SRST)));

    // Set default baud rate
     // Determine the UART_REF_CLK frequency
    
    // BAUD_RATE = 115200
    // RFFDIV set to /2 above
    // UART_REF_CLK = PERCLK / RFDIV
    // UBIR = (BAUD_RATE / 100) - 1
    // UBMR = (UART_REF_CLK / 1600) - 1    
    OUTREG32(&g_pUART->UBIR, (115200 / 100) - 1);
    OUTREG32(&g_pUART->UBMR, ((refClkFreq / 2) / 1600) - 1);	

    OUTREG32(&g_pUART->UCR1,
        CSP_BITFVAL(UART_UCR1_UARTEN, UART_UCR1_UARTEN_ENABLE) |
        CSP_BITFVAL(UART_UCR1_DOZE, UART_UCR1_DOZE_ENABLE) |
        CSP_BITFVAL(UART_UCR1_TDMAEN, UART_UCR1_TXDMAEN_DISABLE) |
        CSP_BITFVAL(UART_UCR1_SNDBRK, UART_UCR1_SNDBRK_NOBREAK) |
        CSP_BITFVAL(UART_UCR1_RTSDEN, UART_UCR1_RTSDEN_DISABLE) |
        CSP_BITFVAL(UART_UCR1_TXMPTYEN, UART_UCR1_TXMPTYEN_DISABLE) |
        CSP_BITFVAL(UART_UCR1_IREN, UART_UCR1_IREN_DISABLE) |
        CSP_BITFVAL(UART_UCR1_RDMAEN, UART_UCR1_RXDMAEN_DISABLE) |
        CSP_BITFVAL(UART_UCR1_RRDYEN, UART_UCR1_RRDYEN_DISABLE) |
        CSP_BITFVAL(UART_UCR1_ICD, UART_UCR1_ICD_8FRAMES) |
        CSP_BITFVAL(UART_UCR1_IDEN, UART_UCR1_IDEN_DISABLE) |
        CSP_BITFVAL(UART_UCR1_TRDYEN, UART_UCR1_TRDYEN_DISABLE) |
        CSP_BITFVAL(UART_UCR1_ADBR, UART_UCR1_ADBR_DISABLE) |
        CSP_BITFVAL(UART_UCR1_ADEN, UART_UCR1_ADEN_DISABLE));


#ifdef 	SERIAL_KITL_FLOWCTRL_BY_SOFTWARE
    OUTREG32(&g_pUART->UCR2,
        CSP_BITFVAL(UART_UCR2_SRST, UART_UCR2_SRST_NORESET) |
        CSP_BITFVAL(UART_UCR2_RXEN, UART_UCR2_RXEN_ENABLE) |
        CSP_BITFVAL(UART_UCR2_TXEN, UART_UCR2_TXEN_ENABLE) |
        CSP_BITFVAL(UART_UCR2_ATEN, UART_UCR2_ATEN_DISABLE) |
        CSP_BITFVAL(UART_UCR2_RTSEN, UART_UCR2_RTSEN_DISABLE) |
        CSP_BITFVAL(UART_UCR2_WS, UART_UCR2_WS_8BIT) |
        CSP_BITFVAL(UART_UCR2_STPB, UART_UCR2_STPB_1STOP) |
        CSP_BITFVAL(UART_UCR2_PROE, UART_UCR2_PROE_EVEN) |
        CSP_BITFVAL(UART_UCR2_PREN, UART_UCR2_PREN_DISBLE) |
        CSP_BITFVAL(UART_UCR2_RTEC, UART_UCR2_RTEC_RISEDGE) |
        CSP_BITFVAL(UART_UCR2_ESCEN, UART_UCR2_ESCEN_DISABLE) |
        CSP_BITFVAL(UART_UCR2_CTS, UART_UCR2_CTS_LOW) |
        CSP_BITFVAL(UART_UCR2_CTSC, UART_UCR2_CTSC_BITCTRL) |
        CSP_BITFVAL(UART_UCR2_IRTS, UART_UCR2_IRTS_IGNORERTS) |
        CSP_BITFVAL(UART_UCR2_ESCI, UART_UCR2_ESCI_DISABLE));
#else
    OUTREG32(&g_pUART->UCR2,
        CSP_BITFVAL(UART_UCR2_SRST, UART_UCR2_SRST_NORESET) |
        CSP_BITFVAL(UART_UCR2_RXEN, UART_UCR2_RXEN_ENABLE) |
        CSP_BITFVAL(UART_UCR2_TXEN, UART_UCR2_TXEN_ENABLE) |
        CSP_BITFVAL(UART_UCR2_ATEN, UART_UCR2_ATEN_DISABLE) |
        CSP_BITFVAL(UART_UCR2_RTSEN, UART_UCR2_RTSEN_DISABLE) |
        CSP_BITFVAL(UART_UCR2_WS, UART_UCR2_WS_8BIT) |
        CSP_BITFVAL(UART_UCR2_STPB, UART_UCR2_STPB_1STOP) |
        CSP_BITFVAL(UART_UCR2_PROE, UART_UCR2_PROE_EVEN) |
        CSP_BITFVAL(UART_UCR2_PREN, UART_UCR2_PREN_DISBLE) |
        CSP_BITFVAL(UART_UCR2_RTEC, UART_UCR2_RTEC_RISEDGE) |
        CSP_BITFVAL(UART_UCR2_ESCEN, UART_UCR2_ESCEN_DISABLE) |
        CSP_BITFVAL(UART_UCR2_CTS, UART_UCR2_CTS_HIGH) |
        CSP_BITFVAL(UART_UCR2_CTSC, UART_UCR2_CTSC_RXCTRL) |
        CSP_BITFVAL(UART_UCR2_IRTS, UART_UCR2_IRTS_IGNORERTS) |
        CSP_BITFVAL(UART_UCR2_ESCI, UART_UCR2_ESCI_DISABLE));
#endif



    OUTREG32(&g_pUART->UCR3,
        CSP_BITFVAL(UART_UCR3_ACIEN, UART_UCR3_ACIEN_DISABLE) |
        CSP_BITFVAL(UART_UCR3_INVT, UART_UCR3_INVT_ACTIVELOW) |
        CSP_BITFVAL(UART_UCR3_RXDMUXSEL, UART_UCR3_RXDMUXSEL_MUX) |
        CSP_BITFVAL(UART_UCR3_AWAKEN, UART_UCR3_AWAKEN_DISABLE) |
        CSP_BITFVAL(UART_UCR3_AIRINTEN, UART_UCR3_AIRINTEN_DISABLE) |
        CSP_BITFVAL(UART_UCR3_RXDSEN, UART_UCR3_RXDSEN_DISABLE) |
        CSP_BITFVAL(UART_UCR3_FRAERREN, UART_UCR3_FRAERREN_DISABLE) |
        CSP_BITFVAL(UART_UCR3_PARERREN, UART_UCR3_PARERREN_DISABLE));

    OUTREG32(&g_pUART->UCR4,
        CSP_BITFVAL(UART_UCR4_DREN, UART_UCR4_DREN_DISABLE) |
        CSP_BITFVAL(UART_UCR4_OREN, UART_UCR4_OREN_DISABLE) |
        CSP_BITFVAL(UART_UCR4_BKEN, UART_UCR4_BKEN_DISABLE) |
        CSP_BITFVAL(UART_UCR4_TCEN, UART_UCR4_TCEN_DISABLE) |
        CSP_BITFVAL(UART_UCR4_LPBYP, UART_UCR4_LPBYP_DISABLE) |
        CSP_BITFVAL(UART_UCR4_IRSC, UART_UCR4_IRSC_SAMPCLK) |
        CSP_BITFVAL(UART_UCR4_WKEN, UART_UCR4_WKEN_DISABLE) |
        CSP_BITFVAL(UART_UCR4_ENIRI, UART_UCR4_ENIRI_DISABLE) |
        CSP_BITFVAL(UART_UCR4_INVR, UART_UCR4_INVR_ACTIVELOW) |
        CSP_BITFVAL(UART_UCR4_CTSTL, 14));

    OUTREG32(&g_pUART->UTS,
        CSP_BITFVAL(UART_UTS_RXDBG, UART_UTS_RXDBG_NOINCREMENT) |
        CSP_BITFVAL(UART_UTS_LOOPIR, UART_UTS_LOOPIR_NOLOOP) |
        CSP_BITFVAL(UART_UTS_DBGEN, UART_UTS_DBGEN_DEBUG) |
        CSP_BITFVAL(UART_UTS_LOOP, UART_UTS_LOOP_NOLOOP) |
        CSP_BITFVAL(UART_UTS_FRCPERR, UART_UTS_FRCPERR_NOERROR));

    OUTREG32(&g_pUART->UFCR,
        CSP_BITFVAL(UART_UFCR_RXTL, 24) |
        CSP_BITFVAL(UART_UFCR_DCEDTE, UART_UFCR_DCEDTE_DCE) |
        CSP_BITFVAL(UART_UFCR_RFDIV, UART_UFCR_RFDIV_DIV2) |
        CSP_BITFVAL(UART_UFCR_TXTL, 4));

OALMSG(1, (TEXT("-SerialInit.\r\n")));	
    return TRUE;
	
}        

//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialDeinit
//
// DESCRIPTION:
//      Deinitializes the internal UART previously configured with SerialInit.
//
// PARAMETERS:
//      None.
//
// RETURNS:
//      If this function succeeds, it returns TRUE, otherwise
//      it returns FALSE.
//
//-----------------------------------------------------------------------------
VOID SerialDeinit(void)
{
}


//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialRecv
//
// DESCRIPTION:
//
//  Called from PQOAL KITL to read bytes from serial port
//
//  Return Value: Number of bytes written
//-----------------------------------------------------------------------------

UINT16 SerialRecv(UINT8 *pch, UINT16 cbRead)
{
	UINT32 urxd;
	UINT16 count;
	
	// read until buffer size is reached or an error occurs
	for(count = 0; count < cbRead; count++) {
		// Wait until the receive data ready
		if (!(INREG32(&g_pUART->USR2) & CSP_BITFMASK(UART_USR2_RDR))) break;
		
		// read char from FIFO
		urxd = INREG32(&g_pUART->URXD);		
		
		// If error detected in current character
		if (urxd & UART_RX_ERRORS) {
			KITLOutputDebugString("KitlSerialRecvRaw:  receive error\n");
			count = 0;
			break;
		}
		// Place read char into buffer
		*(pch + count) = urxd & 0xff;
	}

	// KITLOutputDebugString("<");
	return count;
}

//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialSend
//
// DESCRIPTION: 
//      Block until the bytes are sent
//      
// RETURNS:  
//      Returns the number of bytes sent
//
//-----------------------------------------------------------------------------
UINT16 SerialSend(UINT8 *pch, UINT16 cbSend)
{
	UINT16 count;
	
	for(count = 0; count < cbSend; count++) {
		// Wait until there is room in the FIFO
		while(INREG32(&g_pUART->UTS) & CSP_BITFMASK(UART_UTS_TXFULL));
		// Write character to port
		OUTREG8(&g_pUART->UTXD, *pch);	
		// Point to next character
        ++pch;
	}   
    // KITLOutputDebugString(">");

    return count;
}



VOID SerialSendComplete(UINT16 size)
{
}



//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialEnableInts
//
// DESCRIPTION: 
//      This function enables serial interrupts used for the KITL transport.
//
// PARAMETERS:
//      None.
//
// RETURNS:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SerialEnableInts(void)
{
}


//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialDisableInts
//
// DESCRIPTION: 
//      This function disables serial interrupts used for the KITL transport.
//
// PARAMETERS:
//      None.
//
// RETURNS:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SerialDisableInts(void)
{
}

#ifdef SERIAL_KITL_FLOWCTRL_BY_SOFTWARE
//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialFlowControl
//
// DESCRIPTION: 
//      This function control the serial flow for the KITL transport.
//
// PARAMETERS:
//      fOn
//
// RETURNS:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SerialFlowControl (BOOL fOn)
{
	UINT32 uCtrl = (INREG32(&g_pUART->UCR2) & ~UART_UCR2_CTS_ACTIVE);
	OUTREG32(&g_pUART->UCR2, (uCtrl | (fOn ? UART_UCR2_CTS_ACTIVE : 0)));
}

#endif
//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// END OF FILE
//-----------------------------------------------------------------------------
