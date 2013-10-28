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
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  serialhw.c
//
//   This file implements the device specific functions for zeus serial device.
//
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// INCLUDE FILES  
//------------------------------------------------------------------------------

#include <windows.h>
#include <types.h>
#include <memory.h>
#include <nkintr.h>
#include <serhw.h>
#include <memory.h>
#include <notify.h>
#include <devload.h>
#include <ceddk.h>
#include <windev.h>
#include <serdbg.h>
#include <uart.h>
#include <csp.h>
#include <serial.h>

//------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------
extern UCHAR BSPUartCalRFDIV( ULONG* pRefFreq );
extern BOOL BSPUartEnableClock(ULONG HWAddr, BOOL bEnable);
extern BOOL BSPUartConfigGPIO(ULONG HWAddr, BOOL bIR, BOOL bEnable);
extern VOID BSPIrdaEnable();
extern VOID BSPIrdaSetMode(irMode_c mode);

#ifdef DSR_ENABLE
extern BOOL BSPUartGetPort(ULONG HWAddr, UartPort * pPort);
#endif


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

//-----------------------------------------------------------------------------
//
// Function: SL_CalculateRFDIV
//
// This function is used to calculate RFDIV setting.
//
// Parameters:
//      pRefFreq
//          [in/out] Pointer to reference frequency
//
// Returns:
//      RFDIV setting.
//
//-----------------------------------------------------------------------------
UCHAR SL_CalculateRFDIV( ULONG* pRefFreq )
{
    UCHAR RFDIV=0;

    RFDIV = BSPUartCalRFDIV(pRefFreq);
    if (RFDIV <= 6)
        RFDIV = 6-RFDIV;
    else
        RFDIV = 6;

    return RFDIV;
}

//-----------------------------------------------------------------------------
//
// Function: SL_ReadModemStatus
//
// This function is used to read the modem status.
//
// Parameters:
//      pContext
//          [in] Pointer to device head.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_ReadModemStatus( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    ULONG Events = 0;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ReadModemStatus+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // Save the value in a shadow
        pHWHead->sUSR1 = INREG32(&pHWHead->pUartReg->USR1);
        pHWHead->sUSR2 = INREG32(&pHWHead->pUartReg->USR2);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        pHWHead->sUSR1 = 0;
        pHWHead->sUSR2 = 0;
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));
    // For changes, we use callback to evaluate the event

    // CTS(RTS) status change
    if (pHWHead->sUSR1 & CSP_BITFVAL(UART_USR1_RTSD,UART_USR1_RTSD_SET)) {
        OUTREG32(&(pHWHead->pUartReg->USR1), CSP_BITFVAL(UART_USR1_RTSD, UART_USR1_RTSD_SET));
        DEBUGMSG(ZONE_FUNCTION,(TEXT("EV_CTS\r\n")));
        Events |= EV_CTS;
    }

    if (Events) {
        pHWHead->EventCallback(pHWHead->pMDDContext, Events);
    }
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ReadModemStatus-\r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_ReadLineStatus
//
// This function is used to read the line status.
//
// Parameters:
//      pContext
//          [in] Pointer to device head.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_ReadLineStatus( PVOID pContext )
{

    PUART_INFO pHWHead = (PUART_INFO)pContext;
    ULONG LineEvents = 0;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ReadLineStatus+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
		//Keep the value in a shadow
        pHWHead->sUSR1 = INREG32(&pHWHead->pUartReg->USR1);
        pHWHead->sUSR2 = INREG32(&pHWHead->pUartReg->USR2);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
		pHWHead->sUSR1 = 0;	
		pHWHead->sUSR2 = 0;
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    if (pHWHead->sUSR1 & CSP_BITFVAL(UART_USR1_FRAMERR, UART_USR1_FRAMERR_SET)) {  
        DEBUGMSG(ZONE_ERROR, (TEXT("Error frame\r\n")));
        //Clear interrupt;
        OUTREG32(&(pHWHead->pUartReg->USR1),CSP_BITFVAL(UART_USR1_FRAMERR, UART_USR1_FRAMERR_SET)); 
        pHWHead->CommErrors |= CE_FRAME;
        LineEvents |= EV_ERR;
    }

    if (pHWHead->sUSR1 & CSP_BITFVAL(UART_USR1_PARITYERR, UART_USR1_PARITYERR_SET)) {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error parity\r\n")));
        //Clear interrupt;
        OUTREG32(&(pHWHead->pUartReg->USR1),CSP_BITFVAL(UART_USR1_PARITYERR, UART_USR1_PARITYERR_SET)); 
        pHWHead->CommErrors |= CE_RXPARITY;
        LineEvents |= EV_ERR;
    }

    if (pHWHead->sUSR2 & CSP_BITFVAL(UART_USR2_ORE, UART_USR2_ORE_SET)) {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error Overrun\r\n")));
        //Clear interrupt;
        OUTREG32(&(pHWHead->pUartReg->USR2),CSP_BITFVAL(UART_USR2_ORE, UART_USR2_ORE_SET)); 
        pHWHead->DroppedBytes++;
        pHWHead->CommErrors |= CE_OVERRUN;
        LineEvents |= EV_ERR;
    }

    if (pHWHead->sUSR2 & CSP_BITFVAL(UART_USR2_BRCD, UART_USR2_BRCD_SET)) {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error Break detect!!!\r\n")));
        //Clear interrupt;
        OUTREG32(&(pHWHead->pUartReg->USR2),CSP_BITFVAL(UART_USR2_BRCD, UART_USR2_BRCD_SET)); 
        LineEvents |= EV_BREAK;
	}

    // Let WaitCommEvent know about this error
    if (LineEvents) {
       pHWHead->EventCallback(pHWHead->pMDDContext, LineEvents);
    }
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ReadLineStatus-\r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_SetOutputMode
//
// This function sets the serial device mode (IR/non-IR).
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      UseIR 
//          [in] Should we use IR interface.
//      Use9Pin 
//          [in] Should we use Wire interface.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_SetOutputMode( PVOID pContext, BOOL UseIR, BOOL Use9Pin )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // If you support IR, here you need to set the interface to either IR mode
        // or normal serial. Note that it is possible for both BOOLs to
        // be false (i.e. power down), but never for both to be TRUE.
        if (UseIR) {
            //Enable IR interface;
            INSREG32BF(&pHWHead->pUartReg->UCR1, UART_UCR1_IREN, UART_UCR1_IREN_ENABLE);

            //Set IrDA reception to active low;
            OUTREG32( &(pHWHead->pUartReg->UCR4), INREG32(&(pHWHead->pUartReg->UCR4))&~CSP_BITFMASK(UART_UCR4_INVR) );
            //Set IrDA transmission to active low;
            OUTREG32( &pHWHead->pUartReg->UCR3, INREG32(&pHWHead->pUartReg->UCR3)&~CSP_BITFMASK(UART_UCR3_INVT) );
        }
        else {
            //Disable IR interface;
            OUTREG32( &pHWHead->pUartReg->UCR1, INREG32(&pHWHead->pUartReg->UCR1)&~CSP_BITFMASK(UART_UCR1_IREN) );
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_ClearPendingInts
//
// This function is used to clear any pending 
// interrupts. It is called from Init and PostInit
// to make sure we start out in a known state.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_ClearPendingInts( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ClearPendingInts+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        //USR1
        OUTREG32(&pHWHead->pUartReg->USR1,CSP_BITFVAL(UART_USR1_PARITYERR, UART_USR1_PARITYERR_SET) |
                                CSP_BITFVAL(UART_USR1_RTSD, UART_USR1_RTSD_SET) |
                                CSP_BITFVAL(UART_USR1_ESCF, UART_USR1_ESCF_SET) |
                                CSP_BITFVAL(UART_USR1_FRAMERR, UART_USR1_FRAMERR_SET) |
                                CSP_BITFVAL(UART_USR1_AGTIM, UART_USR1_AGTIM_SET) |                                
                                CSP_BITFVAL(UART_USR1_AIRINT, UART_USR1_AIRINT_SET) |
                                CSP_BITFVAL(UART_USR1_AWAKE, UART_USR1_AWAKE_SET));
        //USR2
        OUTREG32(&pHWHead->pUartReg->USR2,CSP_BITFVAL(UART_USR2_ADET, UART_USR2_ADET_SET) |                                
                                CSP_BITFVAL(UART_USR2_IDLE,UART_USR2_IDLE_SET) |
                                CSP_BITFVAL(UART_USR2_ACST, UART_USR2_ACST_SET) |
                                CSP_BITFVAL(UART_USR2_IRINT, UART_USR2_IRINT_SET) |
                                CSP_BITFVAL(UART_USR2_WAKE, UART_USR2_WAKE_SET) |
                                CSP_BITFVAL(UART_USR2_RTSF, UART_USR2_RTSF_SET) |
                                CSP_BITFVAL(UART_USR2_BRCD, UART_USR2_BRCD_SET) |
                                CSP_BITFVAL(UART_USR2_ORE, UART_USR2_ORE_SET));
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ClearPendingInts-\r\n")));
}


//-----------------------------------------------------------------------------
//
// Function: SL_SetStopBits
//
// This routine sets the Stop Bits for the device.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      StopBits 
//          [in] ULONG StopBits field from DCB.
//
// Returns:  
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL SL_SetStopBits( PVOID pContext, ULONG StopBits )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL bRet = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetStopBits+ 0x%X\r\n"), StopBits));

    EnterCriticalSection(&(pHWHead->RegCritSec));    
    try {
        //Disable RX & TX
       OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_RXEN) );
	OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_TXEN) );

	switch ( StopBits ) {
            case ONESTOPBIT :
                OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_STPB) );
                break;
            case ONE5STOPBITS :
                bRet = FALSE;
                break;
            case TWOSTOPBITS :
                INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_STPB, UART_UCR2_STPB_2STOP);
                break;
            default:
                bRet = FALSE;
                break;
        }
        //Enable RX & TX
       INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_RXEN, UART_UCR2_RXEN_ENABLE);
	INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_TXEN, UART_UCR2_TXEN_ENABLE);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetStopBits-\r\n")));

    return(bRet);
}


//-----------------------------------------------------------------------------
//
// Function: SL_SetBaudRate
//
// This routine sets the baud rate of the device.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      BaudRate 
//          [in] ULONG representing decimal baud rate.
//
// Returns:  
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL SL_SetBaudRate( PVOID pContext, ULONG BaudRate )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL bRet = TRUE;
    ULONG bRefFreq = UART_REF_FREQ;
    UCHAR bDIV = 0;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetbaudRate+ %d\r\n"),BaudRate));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        if ((pHWHead->UseIrDA) && (BaudRate < IRSC_BAUDRATE)) {
            //IR special case;
            DEBUGMSG(ZONE_IR,(TEXT("IR special case!\r\n")));	
            INSREG32BF(&pHWHead->pUartReg->UCR4, UART_UCR4_IRSC, UART_UCR4_IRSC_REFCLK);
        }
        else {
            OUTREG32( &pHWHead->pUartReg->UCR4, INREG32(&pHWHead->pUartReg->UCR4)&~CSP_BITFMASK(UART_UCR4_IRSC) );
        }
       OUTREG32(&pHWHead->pUartReg->UBIR,UART_UBIR_INC(BaudRate, bRefFreq));
	   OUTREG32(&pHWHead->pUartReg->UBMR,UART_UBMR_MOD);
       
	   pHWHead->dcb.BaudRate = BaudRate;
    }except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetbaudRate- Ret = %d\r\n"), bRet));

    return(bRet);
}


//-----------------------------------------------------------------------------
//
// Function: SL_SetByteSize
//
// This routine sets the WordSize of the device.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      ByteSize 
//          [in] ULONG ByteSize field from DCB.
//
// Returns:  
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL SL_SetByteSize( PVOID pContext, ULONG ByteSize )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL bRet = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetByteSize+ 0x%X\r\n"), ByteSize));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        //Disable RX & TX
       OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_RXEN) );
	OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_TXEN) );

        switch (ByteSize) {
            case 7:
                OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_WS) );
                break;
            case 8:
                INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_WS, UART_UCR2_WS_8BIT);
                break;
            default:
                bRet = FALSE;
                break;
        }
        //Enable RX & TX
       INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_RXEN, UART_UCR2_RXEN_ENABLE);
 	INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_TXEN, UART_UCR2_TXEN_ENABLE);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetByteSize- Ret = %d\r\n"), bRet));

    return(bRet);
}


//-----------------------------------------------------------------------------
//
// Function: SL_SetParity
//
// This routine sets the parity of the device.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      Parity 
//          [in] ULONG parity field from DCB.
//
// Returns:  
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL SL_SetParity( PVOID pContext, ULONG Parity )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL bRet = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetParity+ 0x%X\r\n"),Parity));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        //Disable RX & TX
       OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_RXEN) );
	OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_TXEN) );
        switch (Parity) {
            case ODDPARITY:
                INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_PREN, UART_UCR2_PREN_ENABLE);
                INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_PROE, UART_UCR2_PROE_ODD);
                break;
				
            case EVENPARITY:
               INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_PREN, UART_UCR2_PREN_ENABLE);      
  		 OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_PROE) );
                break;
				
            case MARKPARITY:
            case SPACEPARITY:
                bRet = FALSE;
                break;
				
            case NOPARITY:
                OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_PREN) );
                break;
				
            default:
                bRet = FALSE;
                break;
				
        }
        //Enable RX & TX
       INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_RXEN, UART_UCR2_RXEN_ENABLE);
	INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_TXEN, UART_UCR2_TXEN_ENABLE);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetParity- Ret = %d\r\n"), bRet));

    return(bRet);
}


//-----------------------------------------------------------------------------
//
// Function: SL_SetFlowControl
//
// This routine sets the flow control of the device.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      FlowCtrl 
//          [in]  fOutxCtsFlow field from DCB.
//
// Returns:  
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL SL_SetFlowControl( PVOID pContext, BOOL FlowCtrl )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL bRet = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetFlowControl+ 0x%X\r\n"), FlowCtrl));

    EnterCriticalSection(&(pHWHead->RegCritSec));

    try {
        //Disable RX & TX
        OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_RXEN) );
	 OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_TXEN) );


        if (FlowCtrl) {
            OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_IRTS) );
            DEBUGMSG(ZONE_FUNCTION,(TEXT("Clear IRTS 0x%X\r\n"), pHWHead->pUartReg->UCR2));
        }
        else {
            INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_IRTS, UART_UCR2_IRTS_IGNORERTS);
            DEBUGMSG(ZONE_FUNCTION,(TEXT("Set IRTS 0x%X\r\n"), pHWHead->pUartReg->UCR2));
        }
        //Enable RX & TX
        INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_RXEN, UART_UCR2_RXEN_ENABLE);
	INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_TXEN, UART_UCR2_TXEN_ENABLE);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
    }

    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetFlowControl- Ret = %d\r\n"), bRet));

    return(bRet);
}


//-----------------------------------------------------------------------------
//
// Function: SL_Reset
//
// This routine performs any operations associated
// with a device reset.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_Reset( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
  
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_Reset+\r\n")));

    memset(&pHWHead->Status, 0, sizeof(COMSTAT));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_SRST) );
        //Clear RTS
        OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_CTS) );
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_Reset-\r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_Init
//
// This function initializes serial device head.
//
// Parameters:
//      bIR 
//          [in] Is IR mode enabled.
//      bType 
//          [in] Serial device type (DCE/DTE).
//      HWAddress 
//          [in] UART physical base adress.
//      pRegBase 
//          [in] Pointer UART base adress.
//      pContext 
//          [in] Pointer to device head.
//      EventCallback 
//          [in] This callback exists in MDD.
//      pMDDContext 
//          [in] This is the first parm to callback.
//      pBaudTable 
//          [in] BaudRate Table.
//
// Returns:  
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL SL_Init( BOOL bIR, uartType_c bType, ULONG HWAddress, PUCHAR pRegBase, 
    PVOID pContext, EVENT_FUNC EventCallback, 
    PVOID pMDDContext, PLOOKUP_TBL pBaudTable )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL Ret = TRUE;
    ULONG bRefFreq = UART_REF_FREQ;
    UCHAR bDIV = 0;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_INIT+ \r\n"))); 

    pHWHead->UseIrDA = bIR;
    pHWHead->UartType = bType;
    pHWHead->pUartReg = (PCSP_UART_REG)pRegBase;
    pHWHead->HwAddr = HWAddress;

    // Store info for callback function
    pHWHead->EventCallback = EventCallback;
    pHWHead->pMDDContext = pMDDContext;

    // Now set up remaining fields
    if (pBaudTable != NULL)
        pHWHead->pBaudTable = (LOOKUP_TBL *) pBaudTable;
    else
        return FALSE;

    pHWHead->FlushDone = CreateEvent(0, FALSE, FALSE, NULL);
    pHWHead->ulDiscard = 0;

    InitializeCriticalSection(&(pHWHead->TransmitCritSec));
    InitializeCriticalSection(&(pHWHead->RegCritSec));

    if (!BSPUartEnableClock(pHWHead->HwAddr, TRUE)) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_Open:  BSPUartEnableClock failed!\r\n")));
        return FALSE;
    }

    if (pHWHead->UseIrDA) {
        BSPIrdaEnable(TRUE);
        BSPIrdaSetMode(SIR_MODE);
    }

    if (! BSPUartConfigGPIO(pHWHead->HwAddr, pHWHead->UseIrDA, TRUE)) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_Open:  BSPUartConfigGPIO failed!\r\n")));
        return FALSE;
    }

    OUTREG32(&pHWHead->pUartReg->UCR1, CSP_BITFVAL(UART_UCR1_UARTEN, UART_UCR1_UARTEN_ENABLE) |//Enable UART
                 CSP_BITFVAL(UART_UCR1_ICD, UART_UCR1_ICD_8FRAMES));        //idle condition detect
    OUTREG32(&pHWHead->pUartReg->UCR2, CSP_BITFVAL(UART_UCR2_SRST, UART_UCR2_SRST_NORESET));

    OUTREG32(&pHWHead->pUartReg->UCR3, CSP_BITFVAL(UART_UCR3_RXDMUXSEL, UART_UCR3_RXDMUXSEL_MUX));
        
    OUTREG32(&pHWHead->pUartReg->UCR4, CSP_BITFVAL(UART_UCR4_CTSTL, UART_RXFIFO_DEPTH/2));

    bDIV = SL_CalculateRFDIV(&bRefFreq);

     OUTREG32(&pHWHead->pUartReg->UFCR, CSP_BITFVAL(UART_UFCR_RXTL, SER_FIFO_RXTL) |
                 CSP_BITFVAL(UART_UFCR_TXTL, SER_FIFO_TXTL) |
                 CSP_BITFVAL(UART_UFCR_RFDIV, bDIV));

    if (pHWHead->UartType == DTE)
        OUTREG32(&pHWHead->pUartReg->UFCR, INREG32(&pHWHead->pUartReg->UFCR)|CSP_BITFVAL(UART_UFCR_DCEDTE, UART_UFCR_DCEDTE_DTE)); // Configure as DTE
        
    OUTREG32(&pHWHead->pUartReg->ONEMS, (UINT16)(bRefFreq/1000));
	
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_INIT3 RTS 0x%x\r\n"), pHWHead->pUartReg->UCR2)); 
    //Clear RTS
    OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_CTS) );

    if (!pHWHead->UseIrDA) {
        INSREG32BF(&pHWHead->pUartReg->UCR1, UART_UCR1_RTSDEN, UART_UCR1_RTSDEN_ENABLE);
    } else {
        INSREG32BF(&pHWHead->pUartReg->UCR4, UART_UCR4_ENIRI, UART_UCR4_ENIRI_ENABLE);
    }

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_INIT-\r\n")));
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SL_Deinit
//
// This function frees any memory allocated.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_Deinit( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_Deinit+\r\n")));

    OUTREG32( &pHWHead->pUartReg->UCR1, INREG32(&pHWHead->pUartReg->UCR1)&~CSP_BITFMASK(UART_UCR1_UARTEN) ); // Disable UART

    // Disable extern IrDa pin
    if (pHWHead->UseIrDA) {
        BSPIrdaEnable(FALSE);
    }

    BSPUartEnableClock(pHWHead->HwAddr, FALSE);
    BSPUartConfigGPIO(pHWHead->HwAddr, pHWHead->UseIrDA, FALSE);

    if (pHWHead->FlushDone) {
        CloseHandle(pHWHead->FlushDone);
        pHWHead->FlushDone = NULL;
    }

    // delete the critical section
    DeleteCriticalSection(&(pHWHead->TransmitCritSec));
    DeleteCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_Deinit-\r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_Open
//
// This function is called by the upper layer to
// open the serial device.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_Open( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    ULONG bRefFreq = UART_REF_FREQ;
    UCHAR bDIV = 0;

    DEBUGMSG(ZONE_OPEN, (TEXT("SL_Open+ \r\n")));

    SL_ClearPendingInts(pHWHead);

    pHWHead->DroppedBytes = 0;
    pHWHead->CTSFlowOff = FALSE;  // Not flowed off yet
    pHWHead->DSRFlowOff = FALSE;  // Not flowed off yet
    pHWHead->CommErrors   = 0;
    pHWHead->ModemStatus  = 0;

    //initialize as logic 0:high
    pHWHead->bDSR = TRUE;

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        DEBUGMSG(ZONE_OPEN, (TEXT("SL_Open Setting DCB parameters\r\n")));

       OUTREG32(&pHWHead->pUartReg->UCR1, CSP_BITFVAL(UART_UCR1_UARTEN, UART_UCR1_UARTEN_ENABLE) |//Enable UART
                 CSP_BITFVAL(UART_UCR1_ICD, UART_UCR1_ICD_8FRAMES));        //idle condition detect
        
        OUTREG32(&pHWHead->pUartReg->UCR2, CSP_BITFVAL(UART_UCR2_SRST, UART_UCR2_SRST_NORESET) | //no reset
                 CSP_BITFVAL(UART_UCR2_WS, UART_UCR2_WS_8BIT));       //8 Bits
        
        OUTREG32(&pHWHead->pUartReg->UCR3, CSP_BITFVAL(UART_UCR3_RXDMUXSEL, UART_UCR3_RXDMUXSEL_MUX));
        
        OUTREG32(&pHWHead->pUartReg->UCR4, CSP_BITFVAL(UART_UCR4_CTSTL, UART_RXFIFO_DEPTH/2));

        bDIV = SL_CalculateRFDIV(&bRefFreq);

        OUTREG32(&pHWHead->pUartReg->UFCR, CSP_BITFVAL(UART_UFCR_RXTL, SER_FIFO_RXTL) |
                 CSP_BITFVAL(UART_UFCR_TXTL, SER_FIFO_TXTL) |
                 CSP_BITFVAL(UART_UFCR_RFDIV, bDIV));

        if (pHWHead->UartType == DTE)
            OUTREG32(&pHWHead->pUartReg->UFCR, INREG32(&pHWHead->pUartReg->UFCR)|CSP_BITFVAL(UART_UFCR_DCEDTE, UART_UFCR_DCEDTE_DTE)); // Configure as DTE	

		OUTREG32(&pHWHead->pUartReg->ONEMS, (UINT16)(bRefFreq/1000));
        
        // Get defaults from the DCB structure
        SL_SetByteSize(pContext, pHWHead->dcb.ByteSize);
        SL_SetStopBits(pContext, pHWHead->dcb.StopBits);
        SL_SetParity(pContext, pHWHead->dcb.Parity);
        SL_SetBaudRate(pContext, pHWHead->dcb.BaudRate);
		
        //Enable Transmiter & Receiver
        INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_TXEN, UART_UCR2_TXEN_ENABLE);
        INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_RXEN, UART_UCR2_RXEN_ENABLE);

        //Ignore CTS
         OUTREG32(&pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)| CSP_BITFVAL(UART_UCR2_IRTS, UART_UCR2_IRTS_IGNORERTS));
        //Set RTS
        OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_CTSC) );
        INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_CTS, UART_UCR2_CTS_LOW);

        SL_ClearPendingInts( pHWHead );

        //Enable Aging timer interrupt
        INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_ATEN, UART_UCR2_ATEN_ENABLE);

        //Enable Parity Error Interrupt;
        //Enable Frame Error interrupt;
        INSREG32BF(&pHWHead->pUartReg->UCR3, UART_UCR3_PARERREN, UART_UCR3_PARERREN_ENABLE);
        INSREG32BF(&pHWHead->pUartReg->UCR3, UART_UCR3_FRAERREN, UART_UCR3_FRAERREN_ENABLE);

        //Enable Break Condition Detected Interrupt;
        //Enable Overrun interrupt;
        INSREG32BF(&pHWHead->pUartReg->UCR4, UART_UCR4_BKEN, UART_UCR4_BKEN_ENABLE);
        INSREG32BF(&pHWHead->pUartReg->UCR4, UART_UCR4_OREN, UART_UCR4_OREN_ENABLE);

        //Enable Receiver Data Ready Interrupt;
        INSREG32BF(&pHWHead->pUartReg->UCR4, UART_UCR4_DREN, UART_UCR4_DREN_ENABLE);

        if (pHWHead->UseIrDA) {
            //Enable IR interrupt
            INSREG32BF(&pHWHead->pUartReg->UCR4, UART_UCR4_ENIRI, UART_UCR4_ENIRI_ENABLE);
             OUTREG32( &pHWHead->pUartReg->UCR1, INREG32(&pHWHead->pUartReg->UCR1)&~CSP_BITFMASK(UART_UCR1_RTSDEN) );
        }
        else {
            //Enable CTS state change Interrupt;
            INSREG32BF(&pHWHead->pUartReg->UCR1, UART_UCR1_RTSDEN, UART_UCR1_RTSDEN_ENABLE);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_OPEN, (TEXT("SL_Open- \r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_Close
//
// This function closes the device initialized 
// by the SL_Init function. 
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_Close( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_CLOSE,(TEXT("SL_Close+ \r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        if (pHWHead->UseIrDA)
            OUTREG32( &pHWHead->pUartReg->UCR4, INREG32(&pHWHead->pUartReg->UCR4)&~CSP_BITFMASK(UART_UCR4_ENIRI) );

        //Disable Receiver Data Ready Interrupt;
        OUTREG32( &pHWHead->pUartReg->UCR4, INREG32(&pHWHead->pUartReg->UCR4)&~CSP_BITFMASK(UART_UCR4_DREN) );
	
	 OUTREG32( &pHWHead->pUartReg->UCR4, INREG32(&pHWHead->pUartReg->UCR4)&~CSP_BITFMASK(UART_UCR4_TCEN) );

        //Disable Break Condition Detected Interrupt;
        //Disable Overrun interrupt;
        OUTREG32( &pHWHead->pUartReg->UCR4, INREG32(&pHWHead->pUartReg->UCR4)&~CSP_BITFMASK(UART_UCR4_BKEN) );
	 OUTREG32( &pHWHead->pUartReg->UCR4, INREG32(&pHWHead->pUartReg->UCR4)&~CSP_BITFMASK(UART_UCR4_OREN) );


        //Disable Parity Error Interrupt;
        //Disable Frame Error interrupt;
        OUTREG32( &pHWHead->pUartReg->UCR3, INREG32(&pHWHead->pUartReg->UCR3)&~CSP_BITFMASK(UART_UCR3_PARERREN) );
	 OUTREG32( &pHWHead->pUartReg->UCR3, INREG32(&pHWHead->pUartReg->UCR3)&~CSP_BITFMASK(UART_UCR3_FRAERREN) );

        //Disable Aging timer interrupt
        OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_ATEN) );

        //Clear RTS
        OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_CTS) );

        //Disable Transmiter & Receiver
        OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_TXEN) );
  	 OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_RXEN) );
		
        SL_ClearPendingInts( pHWHead );
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_CLOSE,(TEXT("SL_Close- \r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_PowerOff
//
// This routine performs powerdown sequence. 
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_PowerOff( PVOID pContext )
{

    PUART_INFO   pHWHead   = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_PowerOff+ \r\n")));

    // Disable UART
    OUTREG32( &pHWHead->pUartReg->UCR1, INREG32(&pHWHead->pUartReg->UCR1)&~CSP_BITFMASK(UART_UCR1_UARTEN) );

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_PowerOff- \r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_PowerOn
//
// This routine performs poweron sequence. 
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_PowerOn( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_PowerOn+ \r\n")));

    pHWHead->ulDiscard = 0;
    // Restore any registers that we need
    // In power handler context, so don't try to do a critical section
    // Enable UART
    INSREG32BF(&pHWHead->pUartReg->UCR1, UART_UCR1_UARTEN, UART_UCR1_UARTEN_ENABLE);

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_PowerOn- \r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_GetIntrType
//
// This function is called by the MDD whenever 
// an interrupt occurs.  The return code is then 
// checked by the MDD to determine which of the four
// interrupt handling routines are to be called.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      This routine returns a bitmask indicating 
//      which interrupts are currently pending. It returns
//      one of the following interrupt types: 
//          INTR_NONE 
//          INTR_LINE 
//          INTR_RX 
//          INTR_TX 
//          INTR_MODEM 
//      These interrupt types are declared in Serhw.h.
//
//-----------------------------------------------------------------------------
INTERRUPT_TYPE SL_GetIntrType( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    INTERRUPT_TYPE interrupts;
    UINT64 intPndVal = 0;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_GetIntrType+ \r\n")));
    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        if ((pHWHead->pUartReg->USR1 & CSP_BITFVAL(UART_USR1_FRAMERR, UART_USR1_FRAMERR_SET)) 
            || (pHWHead->pUartReg->USR1 & CSP_BITFVAL(UART_USR1_PARITYERR, UART_USR1_PARITYERR_SET))
            || (pHWHead->pUartReg->USR2 & CSP_BITFVAL(UART_USR2_BRCD, UART_USR2_BRCD_SET))
            || (pHWHead->pUartReg->USR2 & CSP_BITFVAL(UART_USR2_ORE, UART_USR2_ORE_SET))){
            interrupts = INTR_LINE;    // 0x1
        } else if ((pHWHead->pUartReg->USR2 & CSP_BITFVAL(UART_USR2_TXDC,UART_USR2_TXDC_SET)) 
            && (pHWHead->pUartReg->UCR4 & CSP_BITFVAL(UART_UCR4_TCEN, UART_UCR4_TCEN_ENABLE))){
            CSP_BITFCLR(pHWHead->pUartReg->UCR4, UART_UCR4_TCEN);
            interrupts = INTR_TX;      // 0x4
        } else if (((pHWHead->pUartReg->USR2 & CSP_BITFVAL(UART_USR2_RDR, UART_USR2_RDR_SET))
            && (pHWHead->pUartReg->UCR4 & CSP_BITFVAL(UART_UCR4_DREN, UART_UCR4_DREN_ENABLE)))
            || (pHWHead->pUartReg->USR1 & CSP_BITFVAL(UART_USR1_AGTIM, UART_USR1_AGTIM_SET))){
            pHWHead->pUartReg->USR1=CSP_BITFVAL(UART_USR1_AGTIM, UART_USR1_AGTIM_SET); 
            interrupts = INTR_RX;
        } else if ((pHWHead->pUartReg->USR1 & CSP_BITFVAL(UART_USR1_RTSD, UART_USR1_RTSD_SET)) 
            && (pHWHead->pUartReg->UCR1& CSP_BITFVAL(UART_UCR1_RTSDEN, UART_UCR1_RTSDEN_ENABLE))){

            DEBUGMSG(ZONE_FUNCTION, (TEXT("RTS change state detected 0x%x\r\n"),pHWHead->pUartReg->USR1));
            interrupts = INTR_MODEM;   // 0x8
        } else if (pHWHead->pUartReg->USR2 & CSP_BITFVAL(UART_USR2_IRINT, UART_USR2_IRINT_SET)){    //Infrared
            pHWHead->pUartReg->USR2=CSP_BITFVAL(UART_USR2_IRINT, UART_USR2_IRINT_SET); 
            interrupts = INTR_MODEM;   // 0x8
        } else 
            interrupts = INTR_NONE;      // 0x0 No interrupts pending, vector is useless
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        intPndVal = INTR_NONE; // simulate no interrupt
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    if (pHWHead->AddTXIntr) {
        interrupts |= INTR_TX;
        pHWHead->AddTXIntr = FALSE;
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_GetIntrType - AddTXIntr 0x%x\r\n"),pHWHead->pUartReg->USR1));                
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("UART_USR1: 0x%x | UART_USR2: 0x%x\r\n"), pHWHead->pUartReg->USR1,  pHWHead->pUartReg->USR2));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_GetIntrType- 0x%X\r\n"),interrupts));

    return(interrupts);
}


//-----------------------------------------------------------------------------
//
// Function: SL_RxIntrHandler
//
// This function gets zero or more characters 
// from the hardware receive buffer and puts them 
// into the location pointed to by the pTargetBuffer 
// parameter. If there are no characters available 
// for reading, this function returns immediately. 
// This function is called in response to a receive 
// interrupt indication from the SL_GetInterruptType
// function.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      pTargetBuffer 
//          [in] Pointer to the target buffer in which to put the data.
//      pByteNumber 
//          [in] Pointer to, on entry, the maximum 
//                number of bytes to read. On exit, the number of 
//                bytes read.
//
// Returns:  
//      The return value indicates the number of overruns 
//      detected. The actual number of dropped characters 
//      may be higher.
//
//-----------------------------------------------------------------------------
ULONG SL_RxIntrHandler( PVOID pContext, PUCHAR pTargetBuffer, ULONG *pByteNumber )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    ULONG RetVal = 0;
    ULONG TargetRoom = *pByteNumber;
    BOOL fRXFlag = FALSE;
    BOOL fReplaceparityErrors = FALSE;
    BOOL fNull;
    UCHAR cEvtChar, cRXChar;
    ULONG ulTemp=0;
    ULONG LineEvents = 0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_RxIntrHandler+ : len %d. EvtChar 0x%x\r\n"), *pByteNumber,pHWHead->dcb.EvtChar));

    *pByteNumber = 0;

    cEvtChar = pHWHead->dcb.EvtChar;
    fNull = pHWHead->dcb.fNull;
    if (pHWHead->dcb.fErrorChar && pHWHead->dcb.fParity)
        fReplaceparityErrors = TRUE;

    try {
        while (TargetRoom) {
            // See if there is another byte to be read
            SL_ReadLineStatus(pHWHead);

            if (pHWHead->sUSR2 & CSP_BITFVAL(UART_USR2_RDR, UART_USR2_RDR_SET)) {
                // Read the RX register
                ulTemp = INREG32(&pHWHead->pUartReg->URXD);

                //Discard the echo char for irda
                if (pHWHead->UseIrDA)
                    if (pHWHead->ulDiscard) {
                        pHWHead->ulDiscard--;
                        continue;
                    }
                //Check if a valid char
                if(ulTemp & CSP_BITFVAL(UART_URXD_ERR, UART_URXD_ERR_ERROR)) {
                    DEBUGMSG(ZONE_ERROR,(TEXT("READ ERROR!!!  x%x\r\n"),ulTemp));
                    continue;
                }

                cRXChar = (UCHAR)ulTemp & UART_URXD_RX_DATA_MSK;
                DEBUGMSG(ZONE_FUNCTION, (TEXT("Read x%x\r\n"), cRXChar));

                // But we may want to discard it
                //Check DSR(DTR)
                if (pHWHead->dcb.fDsrSensitivity && (!pHWHead->bDSR)) {
                    // Do nothing - byte gets discarded
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("Dropping byte because DSR is high\r\n")));
                }
                else if (!cRXChar && fNull) {
                    // Do nothing - byte gets discarded
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("Dropping NULL byte due to fNull\r\n")));
                }
                else {
                    // Do character replacement if parity error detected.
                    //if ( fReplaceparityErrors && (pHWHead->sUSR1 & CSP_BITFVAL(UART_USR1_PARITYERR, UART_USR1_PARITYERR_SET)) ) {
                    if (fReplaceparityErrors && (ulTemp & CSP_BITFVAL(UART_URXD_PRERR, UART_URXD_PRERR_ERROR))) {
                        cRXChar = pHWHead->dcb.ErrorChar;
                    }
                    else {
                        // See if we need to generate an EV_RXFLAG for the received char.
                        if (cRXChar == cEvtChar) {
                            DEBUGMSG(ZONE_FUNCTION, (TEXT("Evt Char x%x\r\n"), cEvtChar));
                            fRXFlag = TRUE;
                        }
                    }
                    // Finally, we can get byte, update status and save.
                    *pTargetBuffer++ = cRXChar;

                    DEBUGMSG(ZONE_FUNCTION,(TEXT("R%02x\r\n"),cRXChar));
                    (*pByteNumber)++;
                    --TargetRoom;
                }
            }
            else {
                // We read all chars, so we're done
                break;
            }
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }

    // if we saw one (or more) EVT chars, then generate an event
    if (fRXFlag) {
        pHWHead->EventCallback(pHWHead->pMDDContext, EV_RXFLAG);
        DEBUGMSG(ZONE_FUNCTION, (TEXT("EV_RXFLAG set\r\n")));
    }
    else {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("EV_RXFLAG not set\r\n")));
    }
    if (pHWHead->DroppedBytes)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Rx drop %d.\r\n"), pHWHead->DroppedBytes));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_RxIntrHandler - rx'ed %d, dropped %d.\r\n"), *pByteNumber, pHWHead->DroppedBytes));

    RetVal = pHWHead->DroppedBytes;
    pHWHead->DroppedBytes = 0;

    return(RetVal);
}


//-----------------------------------------------------------------------------
//
// Function: SL_TxIntrHandler
//
// This function is called when the driver 
// detects a transmit interrupt (INTR_TX), as set by 
// the SL_GetInterruptType function. It sends several 
// characters to the hardware transmission buffer.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      pSourceBuffer 
//          [in] Pointer to the source buffer that contains data to be sent.
//      pByteNumber 
//          [in] When SL_TxIntrEx is called, 
//                 pByteNumber points to the maximum number of bytes 
//                 to send. When SL_TxIntrEx returns, pByteNumber 
//                 points to the actual number of bytes sent..
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_TxIntrHandler( PVOID pContext, PUCHAR pSourceBuffer, ULONG *pByteNumber )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
 
    UCHAR byteCount;
    ULONG NumberOfBytes = *pByteNumber;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_TxIntrHandler+\r\n")));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("Transmit Event 0x%X, Len %d\r\n"), pContext, *pByteNumber));

    SL_ReadModemStatus(pHWHead);
    // We may be done sending.  If so, just disable the TX interrupts
    // and return to the MDD.  
    if (! *pByteNumber) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_TxIntrHandler - Disable INTR_TX.\r\n")));
        //Disable the TX interrupts
        OUTREG32( &pHWHead->pUartReg->UCR4, INREG32(&pHWHead->pUartReg->UCR4)&~CSP_BITFMASK(UART_UCR4_TCEN) );
        return;
    }

    *pByteNumber = 0;  // In case we don't send anything below.
    
    // Disable xmit intr.  Most 16550s will keep hammering
    // us with xmit interrupts if we don't turn them off
    // Whoever gets the FlushDone will then need to turn
    // TX Ints back on if needed.
    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // Need to signal FlushDone for XmitComChar
        PulseEvent(pHWHead->FlushDone);

        pHWHead->CommErrors &= ~CE_TXFULL;

        // If CTS flow control is desired, check cts. If clear, don't send,
        // but loop.  When CTS comes back on, the OtherInt routine will
        // detect this and re-enable TX interrupts (causing Flushdone).
        // For finest granularity, we would check this in the loop below,
        // but for speed, I check it here (up to 8 xmit characters before
        // we actually flow off.
        if (pHWHead->dcb.fOutxCtsFlow) {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_TxIntrHandler - fOutxCtsFlow true\r\n")));
            // We don't need to explicitly read the MSR, since we always enable
            // IER_MS, which ensures that we will get an interrupt and read
            // the MSR whenever CTS, DSR, TERI, or DCD change.
            //Check CTS (RTS), 1:low, 0:high;
            if (!(pHWHead->sUSR1 & CSP_BITFVAL(UART_USR1_RTSS, UART_USR1_RTSS_SET))) {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_TxIntrHandler - fOutxCtsFlow true 0x%x, 0x%x\r\n"),pHWHead->sUSR1,pHWHead->pUartReg->USR1));                
                DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_TxIntrHandler, flowed off via CTS\n") ) );
                pHWHead->CTSFlowOff = TRUE;  // Record flowed off state

                // disable TX interrupts while flowed off
                OUTREG32( &pHWHead->pUartReg->UCR4, INREG32(&pHWHead->pUartReg->UCR4)&~CSP_BITFMASK(UART_UCR4_TCEN) );

                // We could return a positive value here, which would
                // cause the MDD to periodically check the flow control
                // status.  However, we don't need to since we know that
                // the DCTS interrupt will cause the MDD to call us, and we
                // will subsequently fake a TX interrupt to the MDD, causing
                // him to call back into PutBytes.
                LeaveCriticalSection(&(pHWHead->RegCritSec));
                return;
            }
        }

        // Same thing applies for DSR
        if (pHWHead->dcb.fOutxDsrFlow) {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_TxIntrHandler - fOutxDsrFlow true\r\n")));
            // We don't need to explicitly read the register, since we always enable
            // interrupt, which ensures that we will get an interrupt and read
            // the register whenever CTS, DSR, TERI, or DCD change.
            //Check shadow bDSR, 1:low, 0:high
            if (!pHWHead->bDSR) {
                pHWHead->DSRFlowOff = TRUE;  // Record flowed off state
                // disable TX interrupts while flowed off
                OUTREG32( &pHWHead->pUartReg->UCR4, INREG32(&pHWHead->pUartReg->UCR4)&~CSP_BITFMASK(UART_UCR4_TCEN) );

                // See the comment above above positive return codes.
                LeaveCriticalSection(&(pHWHead->RegCritSec));
                return;
            }
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Do nothing.  The worst case is that this was a fluke,
        // and a TX Intr will come right back at us and we will
        // resume transmission.
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    //  OK, now lets actually transmit some data.
    EnterCriticalSection(&(pHWHead->TransmitCritSec));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // Enable xmit intr. We need to do this no matter what, 
        // since the MDD relies on one final interrupt before
        // returning to the application. 
        OUTREG32(&pHWHead->pUartReg->USR2,CSP_BITFVAL(UART_USR2_TXDC, UART_USR2_TXDC_SET)); 

        SL_ReadLineStatus(pHWHead);
        if (pHWHead->sUSR2 & CSP_BITFVAL(UART_USR2_TXFE, UART_USR2_TXFE_SET)) {
            //always enable FIFO
            byteCount = UART_TXFIFO_DEPTH-SER_FIFO_TXTL;
            DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_TxIntrHandler - Write max of %d bytes to 0x%x\r\n"), byteCount, &pHWHead->pUartReg->UTXD));

            for (*pByteNumber=0; NumberOfBytes && byteCount; NumberOfBytes--, byteCount--) {
                OUTREG32(&pHWHead->pUartReg->UTXD,*pSourceBuffer);
                DEBUGMSG(ZONE_FUNCTION, (TEXT("Write x%x\r\n"), *pSourceBuffer));
				
                //We need to discard the echo char for irda
                if (pHWHead->UseIrDA)
                    pHWHead->ulDiscard++;

                ++pSourceBuffer;
                (*pByteNumber)++;
            }
        }

        DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_TxIntrHandler: Enable INTR_TX.\r\n")));
        INSREG32BF(&pHWHead->pUartReg->UCR4, UART_UCR4_TCEN, UART_UCR4_TCEN_ENABLE); 
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));
    LeaveCriticalSection(&(pHWHead->TransmitCritSec));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_TxIntrHandler- sent %d.\r\n"), *pByteNumber));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_ModemIntrHandler
//
// This function handles the modem interrupt. It
// collects the modem status of the serial port, and 
// updates internal driver status information. 
// In the new serial port upper layer implementation 
// available in Microsoft Windows CE 3.0 and later, 
// this function replaces the HWOtherIntrHandler 
// function.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_ModemIntrHandler( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ModemIntrHandler+\r\n")));
    
    SL_ReadModemStatus( pHWHead );
    if (!((PSER_INFO)pContext)->cOpenCount) {
        // We want to indicate a cable event.
        if (!pHWHead->UseIrDA) {
            if ((pHWHead->sUSR1 & CSP_BITFVAL(UART_USR1_RTSS, UART_USR1_RTSS_SET)))
                if (IsAPIReady(SH_WMGR)) {
                        DEBUGMSG(ZONE_FUNCTION, (TEXT("RS232 notification!!!\r\n")));
                        CeEventHasOccurred (NOTIFICATION_EVENT_RS232_DETECTED,NULL);
                    }
        }
        else {
            if (IsAPIReady(SH_WMGR)) 
                CeEventHasOccurred (NOTIFICATION_EVENT_IR_DISCOVERED,NULL);
        }           
    }
    else {
        EnterCriticalSection(&(pHWHead->RegCritSec));
        try {
            if (pHWHead->sUSR1 & CSP_BITFVAL(UART_USR1_RTSS, UART_USR1_RTSS_SET)) {
                // If we are currently flowed off via CTS or DSR, then
                // we better signal the TX thread when one of them changes
                // so that TX can resume sending.
                if (pHWHead->DSRFlowOff && pHWHead->bDSR) {
                    DEBUGMSG(ZONE_FUNCTION,(TEXT("PutBytes, flowed on via DSR\n")));
                    pHWHead->DSRFlowOff = FALSE;
                    // DSR is set, so go ahead and resume sending
                    // Enable xmit intr.
                    INSREG32BF(&pHWHead->pUartReg->UCR4, UART_UCR4_TCEN, UART_UCR4_TCEN_ENABLE);
                    // Then simulate a TX intr to get things moving
                    pHWHead->AddTXIntr = TRUE;
                }
                if (pHWHead->CTSFlowOff && (pHWHead->sUSR1 & CSP_BITFVAL(UART_USR1_RTSS, UART_USR1_RTSS_SET))) {
                    pHWHead->CTSFlowOff = FALSE;
                    // CTS is set, so go ahead and resume sending
                    // Enable xmit intr.
                    INSREG32BF(&pHWHead->pUartReg->UCR4, UART_UCR4_TCEN, UART_UCR4_TCEN_ENABLE);
                    // Then simulate a TX intr to get things moving
                    pHWHead->AddTXIntr = TRUE;
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ModemIntrHandler - CTSFlowOff FALSE 0x%x\r\n"),pHWHead->pUartReg->USR1));                
                }
            }
            else{
                DEBUGMSG(ZONE_FUNCTION, (TEXT("RS232 disconnect notification!!!\r\n")));
                EvaluateEventFlag(pHWHead->pMDDContext, EV_RLSD);
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        }
        LeaveCriticalSection(&(pHWHead->RegCritSec));
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ModemIntrHandler-\r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_LineIntrHandler
//
// This function handles the line interrupt. It
// collects the line status of the serial port, and 
// updates internal driver status information.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_LineIntrHandler( PVOID pContext )
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_LineIntrHandler+ \r\n")));

    SL_ReadLineStatus( pContext );

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_LineIntrHandler- \r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_GetRxBufferSize
//
// This function returns the maximum number of
// bytes that the hardware buffer can hold, not 
// counting the padding, stop, and start bits.
// It would be used only for devices which share a 
// buffer between the MDD/PDD and an ISR.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      Returns the number of bytes in the hardware receive queue.
//
//-----------------------------------------------------------------------------
ULONG SL_GetRxBufferSize( PVOID pContext )
{
    return(0); //This routine always returns 0 for 16550 UARTS.
}


//-----------------------------------------------------------------------------
//
// Function: SL_ClearDTR
//
// This routine clears DTR.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_ClearDTR( PVOID   pContext )
{

#ifdef DSR_ENABLE

	UartPort Port;
  	UINT16 *pPBC_UartCtrlClear = NULL;	
	PHYSICAL_ADDRESS phyAddr;

	phyAddr.QuadPart = BSP_BASE_REG_PA_PBC_BASE + PBC_UART_CTRL_CLEAR_OFFSET;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ClearDTR+\r\n")));

	pPBC_UartCtrlClear = (UINT16*) MmMapIoSpace(phyAddr, sizeof(UINT16), FALSE); 

	if(pPBC_UartCtrlSet)
	{	

		EnterCriticalSection(&(pHWHead->RegCritSec));
		try {
			//Clear DTR(DSR) 	
			if(BSPUartGetPort(pHWHead->HwAddr, &Port))
			{
				switch(Port)
				{
					case UARTA:
						// Clear DSR
						OUTREG16(pPBC_UartCtrlClear, (1 << PBC_UART_CTRL_CLEAR_DCE_DSR_LSH));
						break;
					
					case UARTB:
						//Clear DTR
						OUTREG16(pPBC_UartCtrlClear,  (1 << PBC_UART_CTRL_CLEAR_DTE_DTR_LSH));
						break;

					default:
						break;
				}
			}
		}
		except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
			EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
		}

		MmUnmapIoSpace(pPBC_UartCtrlClear, sizeof(UINT16));

		LeaveCriticalSection(&(pHWHead->RegCritSec));
	}
    
	DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ClearDTR-\r\n")));

#endif

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_SetDTR
//
// This routine sets DTR.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_SetDTR( PVOID pContext )
{    

#ifdef DSR_ENABLE

    PUART_INFO pHWHead = (PUART_INFO)pContext;
	UartPort Port;
  	UINT16 *pPBC_UartCtrlSet = NULL;

	PHYSICAL_ADDRESS phyAddrSet,phyAddrClear;
	phyAddrSet.QuadPart = BSP_BASE_REG_PA_PBC_BASE + PBC_UART_CTRL_SET_OFFSET;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetDTR+\r\n")));

	pPBC_UartCtrlSet = (UINT16*) MmMapIoSpace(phyAddrSet, sizeof(UINT16), FALSE); // mx27 : Map PBC

	if(pPBC_UartCtrlSet)
	{	

		EnterCriticalSection(&(pHWHead->RegCritSec));
		try {

			if(BSPUartGetPort(pHWHead->HwAddr, &Port))	// mx27 : changed
			{
				switch(Port)
				{
					case UARTA:
						// Set DSR
						OUTREG16(pPBC_UartCtrlSet, (1 << PBC_UART_CTRL_SET_DCE_DSR_LSH));
						break;
						
					case UARTB:
						//Set DTR
						OUTREG16(pPBC_UartCtrlSet,  (1 << PBC_UART_CTRL_SET_DTE_DTR_LSH));
						break;
						
					default:
						break;
				}
			}
		}
		except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
			EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
		}

		MmUnmapIoSpace(pPBC_UartCtrlSet, sizeof(UINT16)); // mx27 : Map PBC

		LeaveCriticalSection(&(pHWHead->RegCritSec));
	}

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetDTR-\r\n")));

#endif


    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_ClearRTS
//
// This routine clears RTS.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_ClearRTS( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ClearRTS+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        //Clear RTS(CTS) to logic "0":high;
        OUTREG32( &pHWHead->pUartReg->UCR2, INREG32(&pHWHead->pUartReg->UCR2)&~CSP_BITFMASK(UART_UCR2_CTS) );

        // If RTS hardware handshaking is enabled and the MDD is 
        // requesting that we flow off, we override the RTS setting
        // controlled by the receiver to avoid overflowing the RX buffer
        if (pHWHead->dcb.fRtsControl == RTS_CONTROL_HANDSHAKE)
        {                
            // set RTS (CTS in MX1) to be controlled by MDD software
            INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_CTSC, UART_UCR2_CTSC_BITCTRL);
        }
        
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("ClearRTS Not OK\r\n")));        
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-SL_ClearRTS-\r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_SetRTS
//
// This routine sets RTS.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_SetRTS( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetRTS+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // If RTS hardware handshaking is enabled and the MDD is 
        // requesting that we flow on, we give control of the RTS setting
        // back to the the receiver to avoid overflowing the RX FIFO
        if (pHWHead->dcb.fRtsControl == RTS_CONTROL_HANDSHAKE)
        {                
            // set RTS (CTS in MX1) to be controlled by receiver
            INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_CTSC, UART_UCR2_CTSC_RXCTRL);
        }
        
        //Set RTS(CTS) to logic "1":low;
        INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_CTS, UART_UCR2_CTS_LOW);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetRTS-\r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_ClearBreak
//
// This routine clears break.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_ClearBreak( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ClearBreak+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        //Do not Send Break;
        OUTREG32( &pHWHead->pUartReg->UCR1, INREG32(&pHWHead->pUartReg->UCR1)&~CSP_BITFMASK(UART_UCR1_SNDBRK) );
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ClearBreak-\r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_SetBreak
//
// This routine sets break.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_SetBreak( PVOID pContext )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetBreak+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        //Send Break;
        INSREG32BF(&pHWHead->pUartReg->UCR1, UART_UCR1_SNDBRK, UART_UCR1_SNDBRK_BREAK);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetBreak-\r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_XmitComChar
//
// This routine transmit a char immediately.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      ComChar 
//          [in] Character to transmit.
//
// Returns:  
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL SL_XmitComChar( PVOID pContext, UCHAR ComChar )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_XmitComChar-\n")));

    // Get critical section, then transmit when buffer empties
    EnterCriticalSection(&(pHWHead->TransmitCritSec));
    try {
        while (TRUE) {  // We know THR will eventually empty
            // Write the character if we can
            SL_ReadLineStatus(pHWHead);
            if (pHWHead->sUSR2 & CSP_BITFVAL(UART_USR2_TXFE, UART_USR2_TXFE_SET)) {
                EnterCriticalSection(&(pHWHead->RegCritSec));
                // FIFO is empty, send this character
                pHWHead->pUartReg->UTXD = ComChar;
                LeaveCriticalSection(&(pHWHead->RegCritSec));

                //We need to discard the echo char for irda
                if (pHWHead->UseIrDA)
                    pHWHead->ulDiscard++;
                break;
            }

            EnterCriticalSection(&(pHWHead->RegCritSec));
            // Enable xmit complete intr.
            INSREG32BF(&pHWHead->pUartReg->UCR4, UART_UCR4_TCEN, UART_UCR4_TCEN_ENABLE); 
            LeaveCriticalSection(&(pHWHead->RegCritSec));
            WaitForSingleObject(pHWHead->FlushDone, (ULONG)1000);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }
    LeaveCriticalSection(&(pHWHead->TransmitCritSec));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_XmitComChar-\r\n")));

    return(TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SL_GetStatus
//
// This routine is called by the MDD to retrieve 
// the contents of a COMSTAT structure.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      lpStat 
//          [in] Pointer to LPCOMMSTAT to hold status.
//
// Returns:  
//      The return is a ULONG, representing success (0) or failure (-1).
//
//-----------------------------------------------------------------------------
ULONG SL_GetStatus( PVOID pContext, LPCOMSTAT lpStat )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    ULONG RetVal  = pHWHead->CommErrors;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_GetStatus+\r\n")));

    pHWHead->CommErrors = 0; // Clear old errors each time

    if (lpStat) {
        try {
            if (pHWHead->CTSFlowOff)
                pHWHead->Status.fCtsHold = 1;
            else
                pHWHead->Status.fCtsHold = 0;

            if (pHWHead->DSRFlowOff)
                pHWHead->Status.fDsrHold = 1;
            else
                pHWHead->Status.fDsrHold = 0;

            // NOTE - I think what they really want to know here is
            // the amount of data in the MDD buffer, not the amount
            // in the UART itself.  Just set to 0 for now since the
            // MDD doesn't take care of this.
            pHWHead->Status.cbInQue  = 0;
            pHWHead->Status.cbOutQue = 0;

            memcpy(lpStat, &(pHWHead->Status), sizeof(COMSTAT));
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            RetVal = (ULONG)-1;
        }        
    }
    else
        RetVal = (ULONG)-1;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_GetStatus-\r\n")));

    return(RetVal);
}


//-----------------------------------------------------------------------------
//
// Function: SL_GetModemStatus
//
// This routine retrieves modem status.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      pModemStatus 
//          [in] PULONG passed in by user.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_GetModemStatus( PVOID pContext, PULONG pModemStatus )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_GetModemStatus+\r\n")));

    SL_ReadModemStatus(pHWHead);
    
    //Check CTS(RTS) flow control not supported
    if (pHWHead->sUSR1 & CSP_BITFVAL(UART_USR1_RTSS, UART_USR1_RTSS_SET)){
        DEBUGMSG(ZONE_FUNCTION,(TEXT("MS_CTS_ON\r\n")));
        *pModemStatus |= MS_CTS_ON;
    }

    //Check DSR(DTR) status (always TRUE)
    if (pHWHead->bDSR) {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("MS_DSR_ON\r\n")));
        *pModemStatus |= MS_DSR_ON;
    }

    if (pHWHead->sUSR1 & CSP_BITFVAL(UART_USR1_RTSS, UART_USR1_RTSS_SET)){
        DEBUGMSG (ZONE_FUNCTION,(TEXT("MS_RLSD_ON\r\n")));
        *pModemStatus |= MS_RLSD_ON;
        DEBUGMSG(ZONE_FUNCTION,(TEXT("MS_RING_ON\r\n")));
        *pModemStatus |= MS_RING_ON;
    } else  {	//set state to disconnect
        DEBUGMSG (ZONE_FUNCTION,(TEXT("Turn off MS_RLSD_ON\r\n")));
        *pModemStatus &= ~MS_RLSD_ON;
    }
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_GetModemStatus-(stat 0x%X) \r\n"), *pModemStatus));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_PurgeComm
//
// This routine purge RX and/or TX.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      fdwAction 
//          [in] Action to take.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_PurgeComm( PVOID pContext, DWORD fdwAction )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_PurgeComm+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        if (fdwAction & PURGE_TXCLEAR) {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("PURGE_TXCLEAR\r\n")));
            // not supported;
            // Hardware limitation;
        }
        if (fdwAction & PURGE_RXCLEAR) {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("PURGE_RXCLEAR\r\n")));
            // not supported;
            // Hardware limitation;
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_PurgeComm- \r\n")));

    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_SetDCB
//
// This routine sets new values for DCB.  It gets a 
// DCB from the MDD.  It must then compare this to 
// the current DCB, and if any fields have changed 
// take appropriate action.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      lpDCB 
//          [in] Pointer to DCB structure.
//
// Returns:  
//      TRUE if success. FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL SL_SetDCB( PVOID pContext, LPDCB lpDCB )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL bRet = TRUE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetDC+ \r\n")));

    // If the device is open, scan for changes and do whatever
    // is needed for the changed fields.  if the device isn't
    // open yet, just save the DCB for later use by the open.
    if (((PSER_INFO)pContext)->cOpenCount) {
        // Check if it's valid DCB settings
        if ((lpDCB->ByteSize <= 0) || (lpDCB->BaudRate <= 0))
            return FALSE;

        if (lpDCB->BaudRate != pHWHead->dcb.BaudRate) {
            bRet = SL_SetBaudRate(pHWHead, lpDCB->BaudRate);
        }

        if (bRet && (lpDCB->ByteSize != pHWHead->dcb.ByteSize)) {
            bRet = SL_SetByteSize(pHWHead, lpDCB->ByteSize);
        }

        if (bRet && (lpDCB->Parity != pHWHead->dcb.Parity)) {
            bRet = SL_SetParity(pHWHead, lpDCB->Parity);
        }

        if (bRet && (lpDCB->StopBits != pHWHead->dcb.StopBits)) {
            bRet = SL_SetStopBits(pHWHead, lpDCB->StopBits);
        }

        if (bRet && (lpDCB->fOutxCtsFlow != pHWHead->dcb.fOutxCtsFlow)) {
            if (lpDCB->fOutxCtsFlow)
                bRet = SL_SetFlowControl(pHWHead, TRUE);
            else
                bRet = SL_SetFlowControl(pHWHead, FALSE);
        }

        if (bRet && (lpDCB->fRtsControl != pHWHead->dcb.fRtsControl)) {
            if (lpDCB->fRtsControl == RTS_CONTROL_HANDSHAKE) {
                INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_CTSC, UART_UCR2_CTSC_RXCTRL);
            } else {
                INSREG32BF(&pHWHead->pUartReg->UCR2, UART_UCR2_CTSC, UART_UCR2_CTSC_BITCTRL);
            }
        }  
    }

    if (bRet) {
        // store this DCB
        pHWHead->dcb = *lpDCB;
    }

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetDCB- Ret = %d\r\n"), bRet));

    return(bRet);
}


//-----------------------------------------------------------------------------
//
// Function: SL_SetCommTimeouts
//
// This routine sets new values for the CommTimeouts 
// structure. The routine gets a DCB from the MDD. It
// must then compare this to the current DCB, and if 
// any fields have changed take appropriate action.
//
// Parameters:
//      pContext 
//          [in] Pointer to device head.
//      lpCommTimeouts 
//          [in] Pointer to CommTimeout structure.
//
// Returns:  
//      Zero indicates success.
//
//-----------------------------------------------------------------------------
ULONG SL_SetCommTimeouts( PVOID pContext, LPCOMMTIMEOUTS lpCommTimeouts )
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    ULONG retval = 0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetCommTimeout+ \r\n")));

    // check for any changes and act upon them
    if (lpCommTimeouts->WriteTotalTimeoutMultiplier !=
        pHWHead->CommTimeouts.WriteTotalTimeoutMultiplier) {
    }

    // Now that we have done the right thing, store this DCB
    pHWHead->CommTimeouts = *lpCommTimeouts;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetCommTimeout-\r\n")));
    return(retval);
}
