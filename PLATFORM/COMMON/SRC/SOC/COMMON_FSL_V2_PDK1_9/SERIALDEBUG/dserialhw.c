//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//
//------------------------------------------------------------------------------
//
//  File:  serialhw.c
//
//   This file implements the device specific functions for serial device, IrDa
//   function is not implemented.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
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
#include <serpriv.h>
#include <pm.h>
#pragma warning(pop)

#include "common_types.h"
#include "common_uartdbg.h"
#include "dserial.h"

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines
#define MIN_VAL(a, b)   (((a) > (b)) ? (b) : (a))

#define DISABLEUART        HW_UARTDBGCR_CLR(BM_UARTDBGCR_UARTEN | BM_UARTDBGCR_RXE | BM_UARTDBGCR_TXE)

#define ENABLEUART         HW_UARTDBGCR_SET(BM_UARTDBGCR_UARTEN | BM_UARTDBGCR_RXE | BM_UARTDBGCR_TXE)


//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

PVOID pv_HWregUARTDbg  = NULL;

//WINCE600
// Current Serial device power state.
static CEDEVICE_POWER_STATE CurDx;

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
VOID SL_ReadModemStatus(PVOID pContext)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    ULONG Events = 0;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ReadModemStatus+\r\n")));

    // CTS(RTS) status change
    if ( HW_UARTDBGMIS_RD() & BM_UARTDBGMIS_CTSMMIS)
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("EV_CTS\r\n")));
        HW_UARTDBGIMSC_CLR(BM_UARTDBGMIS_CTSMMIS);
        Events |= EV_CTS;
    }

    if (Events) {
        EvaluateEventFlag(pHWHead->pMDDContext, Events);
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
UINT32 SL_ReadLineStatus(PVOID pContext)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    ULONG LineEvents = 0;
    UINT32 u32Status;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ReadLineStatus+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // Get all the current Rx error bits.
        u32Status = HW_UARTDBGRSR_ECR_RD() & UARTDBG_ALL_RX_ERROR_MASK;
        // Clear all Rx error bits.
        HW_UARTDBGRSR_ECR_SET(UARTDBG_ALL_RX_ERROR_MASK);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        u32Status = 0;
        DEBUGMSG(ZONE_WARN, (TEXT("SL_ReadLineStatus :Exception caught \n")));
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    // Check for an overrun error.
    if((u32Status & BM_UARTDBGRSR_ECR_OE))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error Overrun\r\n")));
        pHWHead->DroppedBytes++;
        pHWHead->CommErrors |= CE_OVERRUN;
        LineEvents |= EV_ERR;
    }
    // Check for a break error.
    if(u32Status & BM_UARTDBGRSR_ECR_BE)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error Break detect!!!\r\n")));
        LineEvents |= EV_BREAK;
    }
    // Check for a parity error.
    if(u32Status & BM_UARTDBGRSR_ECR_PE)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error Parity\r\n")));
        pHWHead->CommErrors |= CE_RXPARITY;
        LineEvents |= EV_ERR;
    }
    // Check for a framing error.
    if(u32Status & BM_UARTDBGRSR_ECR_FE)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error frame\r\n")));
        pHWHead->CommErrors |= CE_FRAME;
        LineEvents |= EV_ERR;
    }

    // Let WaitCommEvent know about this error
    if ( LineEvents ){
       EvaluateEventFlag(pHWHead->pMDDContext, LineEvents);
    }

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ReadLineStatus-\r\n")));

    return (u32Status);
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
VOID SL_SetOutputMode(PVOID pContext, BOOL UseIR, BOOL Use9Pin)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Use9Pin);

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // If you support IR, here you need to set the interface to either IR mode
        // or normal serial. Note that it is possible for both BOOLs to
        // be false (i.e. power down), but never for both to be TRUE.
        if (UseIR) {
            return;
        }
        else {
            //Disable IR interface;
            return;
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_SetOutpuMode :Exception caught \n")));
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));
    return;
}
//-----------------------------------------------------------------------------
//
// Function: SL_ClearPendingInts
//
// This function is used to clear any pending interrupts. It is called from Init and PostInit
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
VOID SL_ClearPendingInts(PVOID pContext)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ClearPendingInts+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // Clear all Pending Interrupts.
        HW_UARTDBGICR_SET(UARTDBG_ALL_INT_CLEAR);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_ClearPendingInts :Exception caught \n")));
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
BOOL SL_SetStopBits(PVOID pContext, ULONG StopBits)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL bRet = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetStopBits+ 0x%X\r\n"), StopBits));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        //Disable RX & TX
        DISABLEUART;

        switch (StopBits) {
        case ONESTOPBIT:
            HW_UARTDBGLCR_H_CLR(BM_UARTDBGLCR_H_STP2);
            break;

        case ONE5STOPBITS:
            bRet = FALSE;
            break;
        case TWOSTOPBITS:
            HW_UARTDBGLCR_H_SET(BF_UARTDBGLCR_H_STP2(0x1));
            break;
        default:
            bRet = FALSE;
            break;
        }
        //Enable RX & TX
        ENABLEUART;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
        DEBUGMSG(ZONE_WARN, (TEXT("SL_SetStopBits :Exception caught \n")));
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
BOOL SL_SetBaudRate(PVOID pContext, UINT32 u32BaudRate)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL rc = FALSE;
    UINT32 divisor;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetbaudRate+ %d\r\n"),u32BaudRate));

    divisor = BUILD_DBG_SERIAL_BAUDRATE_DIVIDER(u32BaudRate);
    if((divisor < MIN_UARTDBG_DIVISOR)|| (divisor > MAX_UARTDBG_DIVISOR))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("SL_SetBaudRate Failed to set the baudrate %d \n"),u32BaudRate));
        goto CleanUp;
    }

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try
    {
        //Disable RX & TX
        DISABLEUART;

        // First clear the bits
        HW_UARTDBGIBRD_CLR(BM_UARTDBGIBRD_BAUD_DIVINT);

        // Set new value
        HW_UARTDBGIBRD_SET(BF_UARTDBGIBRD_BAUD_DIVINT(GET_UARTDBG_BAUD_DIVINT(u32BaudRate)));

        // First clear the bits
        HW_UARTDBGFBRD_CLR(BM_UARTDBGFBRD_BAUD_DIVFRAC);

        // Set new value
        HW_UARTDBGFBRD_SET(BF_UARTDBGFBRD_BAUD_DIVFRAC(GET_UARTDBG_BAUD_DIVFRAC(u32BaudRate)));

        //Enable RX & TX
        ENABLEUART;

        // workarond(after change baudrate, must operate LCR_H, or else the new baudrate does not work)
        HW_UARTDBGLCR_H_SET(BM_UARTDBGLCR_H_FEN);

    } except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
              EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

        DEBUGMSG(ZONE_WARN, (TEXT("SL_SetBaudRate :Exception caught \n")));
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    rc = TRUE;
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetbaudRate- \r\n")));

CleanUp:
    return(rc);
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
BOOL SL_SetByteSize(PVOID pContext, ULONG ByteSize)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL bRet = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetByteSize+ 0x%X\r\n"), ByteSize));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {

        //Disable RX & TX
        DISABLEUART;

        switch (ByteSize) {
        case 5:
            HW_UARTDBGLCR_H_CLR(BM_UARTDBGLCR_H_WLEN);
            HW_UARTDBGLCR_H_SET(BF_UARTDBGLCR_H_WLEN(UARTDBG_LINECTRL_WLEN_5));
            break;
        case 6:
            HW_UARTDBGLCR_H_CLR(BM_UARTDBGLCR_H_WLEN);
            HW_UARTDBGLCR_H_SET(BF_UARTDBGLCR_H_WLEN(UARTDBG_LINECTRL_WLEN_6));
            break;
        case 7:
            HW_UARTDBGLCR_H_CLR(BM_UARTDBGLCR_H_WLEN);
            HW_UARTDBGLCR_H_SET(BF_UARTDBGLCR_H_WLEN(UARTDBG_LINECTRL_WLEN_7));
            break;
        case 8:
            HW_UARTDBGLCR_H_SET(BF_UARTDBGLCR_H_WLEN(UARTDBG_LINECTRL_WLEN_8));
            break;
        default:
            bRet = FALSE;
            break;
        }
        //Enable RX & TX
        ENABLEUART;

    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
        DEBUGMSG(ZONE_WARN, (TEXT("SL_SetByteSize :Exception caught \n")));
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
BOOL SL_SetParity(PVOID pContext, ULONG Parity)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL bRet = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetParity+ 0x%X\r\n"),Parity));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {

        //Disable RX & TX
        DISABLEUART;

        switch (Parity) {

        case ODDPARITY:
            HW_UARTDBGLCR_H_CLR(BM_UARTDBGLCR_H_EPS|BM_UARTDBGLCR_H_PEN);
            HW_UARTDBGLCR_H_SET(BM_UARTDBGLCR_H_PEN);
            break;
        case EVENPARITY:
            HW_UARTDBGLCR_H_SET(BM_UARTDBGLCR_H_EPS|BM_UARTDBGLCR_H_PEN);
            break;
        case MARKPARITY:
        case SPACEPARITY:
            bRet = FALSE;
            break;
        case NOPARITY:
            HW_UARTDBGLCR_H_CLR(BM_UARTDBGLCR_H_EPS|BM_UARTDBGLCR_H_PEN);
            break;
        default:
            bRet = FALSE;
            break;
        }
        //Enable RX & TX
        ENABLEUART;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
        DEBUGMSG(ZONE_WARN, (TEXT("SL_SetParity :Exception caught \n")));
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_SetParity- Ret = %d\r\n"), bRet));
    return(bRet);
}

//-----------------------------------------------------------------------------
//
// Function: SL_Reset
//
// This routine performs any operations associated with a device reset.
//
// Parameters:
//      pContext
//          [in] Pointer to device head.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_Reset(PVOID pContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_Reset+\r\n")));

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
BOOL SL_Init(BOOL bIR,ULONG HWAddress, PUCHAR pRegBase,
             PVOID pContext, EVENT_FUNC EventCallback,PVOID pMDDContext)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_INIT,(TEXT("SL_INIT+ \r\n")));

    pHWHead->UseIrDA =bIR;
    pHWHead->pUartdbgReg = (PCSP_UARTDBG_REG)pRegBase;
    pv_HWregUARTDbg = (PVOID)pRegBase;
    pHWHead->HwAddr = HWAddress;

    // Store info for callback function
    pHWHead->EventCallback = EventCallback;
    pHWHead->pMDDContext = pMDDContext;

    // Now set up remaining fields

    pHWHead->FlushDone = CreateEvent(0, FALSE, FALSE, NULL);
    pHWHead->ulDiscard = 0;

    // Disable the UART
    ENABLEUART;
    SL_ClearPendingInts(pHWHead);
    // if ILPR bit0=1, DEBUG UART port will stop to output debug messages.
    HW_UARTDBGILPR_WR(1);
    DISABLEUART;

    InitializeCriticalSection(&(pHWHead->TransmitCritSec));
    InitializeCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_INIT,(TEXT("SL_INIT-\r\n")));

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
VOID SL_Deinit(PVOID pContext)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_Deinit+\r\n")));

    DISABLEUART;

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
// This function is called by the upper layer to open the serial device.
//
// Parameters:
//      pContext
//          [in] Pointer to device head.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_Open(PVOID pContext)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    DEBUGMSG(ZONE_OPEN, (TEXT("SL_Open+ \r\n")));

    // because Debug UART has not reset bit, manually clear the registers values
    HW_UARTDBGRSR_ECR_WR(0xFF);
    HW_UARTDBGCR_WR(0x0);
    HW_UARTDBGIFLS_WR(0x0);
    HW_UARTDBGIMSC_WR(0x0);
    
    SL_ClearPendingInts(pHWHead);

    pHWHead->DroppedBytes = 0;
    pHWHead->CTSFlowOff = FALSE;  // Not flowed off yet
    pHWHead->MDDFlowOff = FALSE;  // Not flowed off yet by MDD
    pHWHead->DSRFlowOff = FALSE;  // Not flowed off yet
    pHWHead->CommErrors   = 0;
    pHWHead->ModemStatus  = 0;

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        DEBUGMSG(ZONE_OPEN, (TEXT("SL_Open Setting DCB parameters\r\n")));


        // Set the enable FIFO bit.
        HW_UARTDBGLCR_H_SET(BM_UARTDBGLCR_H_FEN);


        // Set the FIFO trigger levels : half full trigger
        HW_UARTDBGIFLS_CLR(BM_UARTDBGIFLS_RXIFLSEL|BM_UARTDBGIFLS_TXIFLSEL);
        HW_UARTDBGIFLS_SET(BF_UARTDBGIFLS_RXIFLSEL(0x2)|BF_UARTDBGIFLS_TXIFLSEL(0x2));
        
        // Get defaults from the DCB structure
        SL_SetByteSize(pContext, pHWHead->dcb.ByteSize);
        SL_SetStopBits(pContext, pHWHead->dcb.StopBits);
        SL_SetParity(pContext, pHWHead->dcb.Parity);
        SL_SetBaudRate(pContext, pHWHead->dcb.BaudRate);


        SL_ClearPendingInts(pHWHead);

        // only enable RXI/RTI/OEI/CTSMI
        HW_UARTDBGIMSC_SET(UARTDBG_ALL_INT_STATUS_MASK & (BM_UARTDBGIMSC_RXIM|BM_UARTDBGIMSC_RTIM|BM_UARTDBGIMSC_OEIM|BM_UARTDBGIMSC_CTSMIM));

        // Enable the UART
        ENABLEUART;

    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_Open :Exception caught \n")));
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_OPEN, (TEXT("SL_Open- \r\n")));
    return;
}
//-----------------------------------------------------------------------------
//
// Function: SL_Close
//
// This function closes the device initialized by the SL_Init function.
//
// Parameters:
//      pContext
//          [in] Pointer to device head.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_Close(PVOID pContext)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_CLOSE,(TEXT("SL_Close+ \r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try
    {
        HW_UARTDBGIMSC_WR(0x0);

        HW_UARTDBGRSR_ECR_WR(0xFF);
        HW_UARTDBGIFLS_WR(0x0);
        HW_UARTDBGIBRD_WR(0x0);
        HW_UARTDBGFBRD_WR(0x0);

        SL_ClearPendingInts(pHWHead);

        // Disable UART
        DISABLEUART;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_Close :Exception caught \n")));
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    //WINCE600
    //set device power state to D4
    CurDx = D4;

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
VOID SL_PowerOff(PVOID pContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_PowerOff+ \r\n")));

    // Disable UART
    DISABLEUART;

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
VOID SL_PowerOn(PVOID pContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_PowerOn+ \r\n")));

    /*pHWHead->ulDiscard = 0;*/
    // Restore any registers that we need
    // In power handler context, so don't try to do a critical section

    // Enable UART
    ENABLEUART;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_PowerOn- \r\n")));
    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_GetIntrType
//
// This function is called by the MDD whenever an interrupt occurs.  The return code is then
// checked by the MDD to determine which of the four interrupt handling routines are to be called.
//
// Parameters:
//      pContext
//          [in] Pointer to device head.
//
// Returns:
//      This routine returns a bitmask indicating which interrupts are currently pending. It returns
//      one of the following interrupt types:
//          INTR_NONE
//          INTR_LINE
//          INTR_RX
//          INTR_TX
//          INTR_MODEM
//      These interrupt types are declared in Serhw.h.
//
//-----------------------------------------------------------------------------
INTERRUPT_TYPE SL_GetIntrType(
    PVOID pHead                     // Pointer to hardware head
    )
{
    PUART_INFO pHWHead = (PUART_INFO)pHead;
    INTERRUPT_TYPE interrupts  = INTR_NONE;
    ULONG u32Interrupt;

    DEBUGMSG( ZONE_FUNCTION, (TEXT("+Ser_GetInterruptType \r\n")));
    EnterCriticalSection(&(pHWHead->RegCritSec));

    u32Interrupt = HW_UARTDBGMIS_RD();

    if ( u32Interrupt & BM_UARTDBGMIS_CTSMMIS )
    {
        interrupts |= INTR_MODEM;
    }

    if ( (u32Interrupt & BM_UARTDBGMIS_OEMIS) || (u32Interrupt & BM_UARTDBGMIS_BEMIS) ||
         (u32Interrupt & BM_UARTDBGMIS_FEMIS) || (u32Interrupt & BM_UARTDBGMIS_PEMIS))
    {
        HW_UARTDBGICR_SET((BM_UARTDBGMIS_OEMIS|BM_UARTDBGMIS_BEMIS|BM_UARTDBGMIS_FEMIS|BM_UARTDBGMIS_PEMIS)& UARTDBG_ALL_INT_STATUS_MASK);
        interrupts |= INTR_LINE;
    }

    // Capture the old mask to return to the caller.

    try {
        if ( u32Interrupt & BM_UARTDBGMIS_TXMIS)
        {
            HW_UARTDBGICR_SET(BM_UARTDBGICR_TXIC);
            interrupts |= INTR_TX;
        }
        if ((u32Interrupt & BM_UARTDBGMIS_RTMIS) || (u32Interrupt & BM_UARTDBGMIS_RXMIS))
        {
            HW_UARTDBGICR_SET(BM_UARTDBGICR_RTIC|BM_UARTDBGICR_RXIC);
            interrupts |= INTR_RX;
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        u32Interrupt = INTR_NONE;             // simulate no interrupt
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Ser_GetInterruptType :Exception caught \n")));
    }

    LeaveCriticalSection(&(pHWHead->RegCritSec));

    if (pHWHead->AddTXIntr)
    {
        interrupts |= INTR_TX;
        pHWHead->AddTXIntr = FALSE;
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("Ser_GetInterruptType- 0x%X\r\n"),interrupts));
    return interrupts;
}

//-----------------------------------------------------------------------------
//
// Function: SL_RxIntrHandler
//
// This function gets zero or more characters from the hardware receive buffer and puts them
// into the location pointed to by the pTargetBuffer parameter. If there are no characters available
// for reading, this function returns immediately.
// This function is called in response to a receive interrupt indication from the SL_GetInterruptType
// function.
//
// Parameters:
//      pContext
//          [in] Pointer to device head.
//      pTargetBuffer
//          [in] Pointer to the target buffer in which to put the data.
//      pByteNumber
//          [in] Pointer to, on entry, the maximum number of bytes to read. On exit, the number of
//                bytes read.
//
// Returns:
//      The return value indicates the number of overruns detected. The actual number of dropped
//      characters may be higher.
//
//-----------------------------------------------------------------------------
ULONG SL_RxIntrHandler(PVOID pContext,PUCHAR pTargetBuffer,ULONG *pByteNumber)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    ULONG RetVal                = 0;
    ULONG TargetRoom    = *pByteNumber;
    BOOL fRXFlag               = FALSE;
    BOOL fReplaceparityErrors = FALSE;
    BOOL fNull;
    UCHAR cEvtChar, cRXChar;
    UINT32 u32Data  = 0;
    UINT32 u32Status;
    ULONG LineEvents = 0;
    DEBUGMSG(ZONE_READ, (TEXT("SL_RxIntrHandler+ : len %d. EvtChar 0x%x\r\n"), *pByteNumber,pHWHead->dcb.EvtChar));

    *pByteNumber = 0;

    cEvtChar = pHWHead->dcb.EvtChar;
    fNull = pHWHead->dcb.fNull;
    if (pHWHead->dcb.fErrorChar && pHWHead->dcb.fParity)
        fReplaceparityErrors = TRUE;

    try
    {
        while ( TargetRoom )
        {
            // See if there is another byte to be read
            u32Status = HW_UARTDBGFR_RD();
            if (!(u32Status & BM_UARTDBGFR_RXFE))
            {
                // Read the RX register
                u32Data = HW_UARTDBGDR_RD();
                cRXChar = (UCHAR)( u32Data & 0xFF );

                // Check some error conditions
                if(u32Data & FIFO_ERROR)
                {
                    if(u32Data & FIFO_FRAME_ERROR)
                    {
                        DEBUGMSG(ZONE_READ, (TEXT("Error frame\r\n")));
                        pHWHead->CommErrors |= CE_FRAME;
                        LineEvents |= EV_ERR;
                    }

                    if(u32Data & FIFO_PARITY_ERROR)
                    {
                        DEBUGMSG(ZONE_READ, (TEXT("Error parity\r\n")));
                        pHWHead->CommErrors |= CE_RXPARITY;
                        LineEvents |= EV_ERR;
                    }

                    if(u32Data & FIFO_BREAK_ERROR)
                    {
                        DEBUGMSG(ZONE_READ, (TEXT("Error Break detect!!!\r\n")));
                        LineEvents |= EV_BREAK;
                    }

                    if(u32Data & FIFO_OVERRUN_ERROR)
                    {
                        DEBUGMSG(ZONE_READ, (TEXT("Error frame\r\n")));
                        pHWHead->CommErrors |= CE_OVERRUN;
                        LineEvents |= EV_ERR;
                    }

                    // Let WaitCommEvent know about this error
                    if (LineEvents)
                    {
                        EvaluateEventFlag(pHWHead->pMDDContext, LineEvents);
                    }  
                }
                if (!cRXChar && fNull)
                {
                    // Do nothing - byte gets discarded
                    DEBUGMSG(ZONE_READ, (TEXT("Dropping NULL byte due to fNull\r\n")));
                }
                else
                {
                    // Do character replacement if parity error detected.
                    // Do character replacement if parity error detected.
                    if (fReplaceparityErrors && (u32Data & FIFO_FRAME_ERROR))
                    {
                        cRXChar = pHWHead->dcb.ErrorChar;
                    } else {
                        // See if we need to generate an EV_RXFLAG for the received char.
                        if ( cRXChar == cEvtChar )
                            fRXFlag = TRUE;
                    }
                    // Finally, we can get byte, update status and save.
                    *pTargetBuffer++ = cRXChar;
                    (*pByteNumber)++;
                    --TargetRoom;
                }

            } else {
                // We read all chars, so we're done
                break;
            }
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

        DEBUGMSG(ZONE_WARN, (TEXT("SL_RxIntrHandler :Exception caught \n")));

        // just exit
    }

    // if we saw one (or more) EVT chars, then generate an event
    if(fRXFlag) {
        EvaluateEventFlag(pHWHead->pMDDContext, EV_RXFLAG);
    }

    if ( pHWHead->DroppedBytes )
        RETAILMSG (0, (TEXT("Rx drop %d.\r\n"), pHWHead->DroppedBytes));

    RetVal = pHWHead->DroppedBytes;
    pHWHead->DroppedBytes = 0;
    return(RetVal);
}

//-----------------------------------------------------------------------------
//
// Function: SL_TxIntrHandler
//
// This function is called when the driver detects a transmit interrupt (INTR_TX), as set by the
// SL_GetInterruptType function. It sends several characters to the hardware transmission buffer.
//
// Parameters:
//      pContext
//          [in] Pointer to device head.
//      pSourceBuffer
//          [in] Pointer to the source buffer that contains data to be sent.
//      pByteNumber
//          [in] When SL_TxIntrEx is called,
//                 pByteNumber points to the maximum number of bytes to send. When
//                 SL_TxIntrEx returns, pByteNumber points to the actual number of bytes sent..
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SL_TxIntrHandler(PVOID pContext,PUCHAR pSourceBuffer,ULONG *pByteNumber)
{
    PUART_INFO pHWHead    = (PUART_INFO)pContext;
    ULONG NumberOfBytes = *pByteNumber;

    DEBUGMSG(ZONE_WRITE, (TEXT("SL_TxIntrHandler+\r\n")));

    DEBUGMSG(ZONE_WRITE, (TEXT("Transmit Event 0x%X, Len %d\r\n"), pContext, *pByteNumber));

    // We may be done sending.  If so, just disable the TX interrupts
    // and return to the MDD.
    if (!*pByteNumber)
    {
        DEBUGMSG(ZONE_WRITE, (TEXT("SL_TxIntrHandler - Disable INTR_TX.\r\n")));

        //Disable the TX interrupts
        HW_UARTDBGIMSC_CLR(BM_UARTDBGIMSC_TXIM);

        return;
    }


    *pByteNumber = 0;  // In case we don't send anything below.

    EnterCriticalSection(&(pHWHead->RegCritSec));

    if (pHWHead->dcb.fRtsControl != RTS_CONTROL_HANDSHAKE)
    {
        // Need to signal FlushDone for XmitComChar
        PulseEvent(pHWHead->FlushDone);

        pHWHead->CommErrors &= ~CE_TXFULL;

        // If CTS flow control is desired, check cts. If clear, don't send,
        if (pHWHead->dcb.fOutxCtsFlow && ((HW_UARTDBGFR_RD() & BM_UARTDBGFR_CTS) == 0))
        {
            DEBUGMSG(ZONE_WRITE, (TEXT("SL_TxIntrHandler - fOutxCtsFlow true\r\n")));

            pHWHead->CTSFlowOff = TRUE;  // Record flowed off state

            // disable TX interrupts while flowed off
            HW_UARTDBGIMSC_CLR(BM_UARTDBGIMSC_TXIM);

            LeaveCriticalSection(&(pHWHead->RegCritSec));
            return;

        }
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));


    // Disable xmit intr.  Most 16550s will keep hammering
    // us with xmit interrupts if we don't turn them off
    // Whoever gets the FlushDone will then need to turn
    // TX Ints back on if needed.
    EnterCriticalSection(&(pHWHead->RegCritSec));
    try
    {
        //always enable FIFO
        //byteCount = UARTAPP_TXFIFO_DEPTH;
        //MX233 UART module has not TX FIFO empty interrupt, and only number of DATA in TX FIFO
        //decreases to less than trigger level, TX interrupt is triggered, so we may fill the TX FIFO fully and leave.

        // Wait until there is room in the FIFO
        while(HW_UARTDBGFR_RD() & BM_UARTDBGFR_TXFF);

        for (*pByteNumber = 0; NumberOfBytes/* && byteCount*/; NumberOfBytes--/*, byteCount--*/)
        {
            HW_UARTDBGDR_WR(*pSourceBuffer);

            // Check again if FIFO is FULL
            if(HW_UARTDBGFR_RD() & BM_UARTDBGFR_TXFF)
            {
                ++pSourceBuffer;
                (*pByteNumber)++;
                break;
            }

            //We need to discard the echo char for irda
            if (pHWHead->UseIrDA)
            {
                pHWHead->ulDiscard++;
            }

            ++pSourceBuffer;
            (*pByteNumber)++;
        }

        // Enable xmit intr. We need to do this no matter what,
        // since the MDD relies on one final interrupt before
        // returning to the application.
        HW_UARTDBGIMSC_SET(BM_UARTDBGIMSC_TXIM);

        DEBUGMSG(ZONE_WRITE, (TEXT("SL_TxIntrHandler: Enable INTR_TX.\r\n")));

    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_TxIntrHandler :Exception caught \n")));
    }

    LeaveCriticalSection(&(pHWHead->RegCritSec));
    DEBUGMSG(ZONE_WRITE, (TEXT("SL_TxIntrHandler- sent %d.\r\n"), *pByteNumber));
    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_ModemIntrHandler
//
// This function handles the modem interrupt. It collects the modem status of the serial port, and
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
VOID SL_ModemIntrHandler(PVOID pContext)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ModemIntrHandler+\r\n")));

    SL_ReadModemStatus(pHWHead);

    if (pHWHead->CTSFlowOff && (HW_UARTDBGFR_RD() & BM_UARTDBGFR_CTS))
    {
        pHWHead->CTSFlowOff = FALSE;
        // CTS is set, so go ahead and resume sending
        // Enable TX intr.
        HW_UARTDBGIMSC_SET(BM_UARTDBGIMSC_TXIM);

        // Then simulate a TX intr to get things moving
        pHWHead->AddTXIntr = TRUE;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ModemIntrHandler-\r\n")));
    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_LineIntrHandler
//
// This function handles the line interrupt. It collects the line status of the serial port, and
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
VOID SL_LineIntrHandler(PVOID pContext)
{
    DEBUGMSG(1,(TEXT("SL_LineIntrHandler+ \r\n")));

    SL_ReadLineStatus(pContext);

    DEBUGMSG(1,(TEXT("SL_LineIntrHandler- \r\n")));
    return;
}


//-----------------------------------------------------------------------------
//
// Function: SL_GetRxBufferSize
//
// This function returns the maximum number of bytes that the hardware buffer can hold, not
// counting the padding, stop, and start bits.
// It would be used only for devices which share a buffer between the MDD/PDD and an ISR.
//
// Parameters:
//      pContext
//          [in] Pointer to device head.
//
// Returns:
//      Returns the number of bytes in the hardware receive queue.
//
//-----------------------------------------------------------------------------
ULONG SL_GetRxBufferSize(PVOID pContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);
    return(0); //This routine always returns 0 for 16550 UARTS.
}


////////////////////////////////////////////////////////////////////////////////////
//
// @func void | Ser_ClearDTR | This routine clears DTR.
//
// @rdesc None.
////////////////////////////////////////////////////////////////////////////////////
VOID SL_ClearDTR(PVOID pContext)
{
    // MX233 UART does not support DTR LINE
    UNREFERENCED_PARAMETER(pContext);
    return;
}
////////////////////////////////////////////////////////////////////////////////////
// @doc OEM
// @func VOID | Ser_SetDTR | This routine sets DTR.
//
// @rdesc None.
////////////////////////////////////////////////////////////////////////////////////

VOID SL_SetDTR(PVOID pContext)
{
    // MX233 UART does not support DTR LINE
    UNREFERENCED_PARAMETER(pContext);
    return;
}
////////////////////////////////////////////////////////////////////////////////////

// @doc OEM
// @func VOID | Ser_ClearRTS | This routine clears RTS.
//
// @rdesc None.
////////////////////////////////////////////////////////////////////////////////////

VOID SL_ClearRTS(PVOID pContext)
{

    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ClearRTS+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        //Clear RTS to logic "0":high;
        HW_UARTDBGCR_CLR(BM_UARTDBGCR_RTS);

        // If RTS hardware handshaking is enabled and the MDD is 
        // requesting that we flow off, we override the RTS setting
        // controlled by the receiver to avoid overflowing the RX buffer
        if (pHWHead->dcb.fRtsControl == RTS_CONTROL_HANDSHAKE)
        {                
            // set RTS to be controlled by MDD software
            HW_UARTDBGCR_CLR(BM_UARTDBGCR_RTSEN);
            pHWHead->MDDFlowOff  = TRUE;
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_ClearRTS :Exception caught \n")));
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-SL_ClearRTS-\r\n")));
    return;
}
////////////////////////////////////////////////////////////////////////////////////

// @doc OEM
// @func VOID | Ser_SetRTS | This routine sets RTS.
//
// @rdesc None.
////////////////////////////////////////////////////////////////////////////////////

VOID SL_SetRTS(PVOID pContext)
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
            // set RTS to be controlled by receiver
            HW_UARTDBGCR_SET(BM_UARTDBGCR_RTSEN);
        }

        //Set RTS to logic "1":low;
        HW_UARTDBGCR_SET(BM_UARTDBGCR_RTS);

        pHWHead->MDDFlowOff  = FALSE;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_SetRTS :Exception caught \n")));
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetRTS-\r\n")));

    return;
}

////////////////////////////////////////////////////////////////////////////////////

// @doc OEM
// @func VOID | Ser_ClearBreak | This routine clears break.
//
// @rdesc None.
////////////////////////////////////////////////////////////////////////////////////

VOID SL_ClearBreak(PVOID pContext)
{
    PUART_INFO pHWHead    = (PUART_INFO)pContext;

    DEBUGMSG( 1,(TEXT("+Ser_ClearBreak:\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));

    try {
        // DISABLE RX/TX
        DISABLEUART;

        // Clearing the BRK Bit of the Control register.
        HW_UARTDBGLCR_H_CLR(BM_UARTDBGLCR_H_BRK);

        // ENABLE RX/TX
        ENABLEUART;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_ClearBreak :Exception caught \n")));
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG( 1,(TEXT("-Ser_ClearBreak:\r\n")));
    return;
}

////////////////////////////////////////////////////////////////////////////////////
// @doc OEM
// @func VOID | Ser_SetBreak | This routine sets break.
//
// @rdesc None.
//////////////////////////////////////////////////////////////////////////////////////
VOID SL_SetBreak(PVOID pContext)
{
    PUART_INFO pHWHead    = (PUART_INFO)pContext;
    DEBUGMSG( 1,(TEXT("+Ser_SetBreak:\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // DISABLE RX/TX
        DISABLEUART;

        // Set the BRK Bit of the Control register.
        HW_UARTDBGLCR_H_SET(BM_UARTDBGLCR_H_BRK);

        // ENABLE RX/TX
        ENABLEUART;
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_SetBreak :Exception caught \n")));
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
BOOL SL_XmitComChar(PVOID pContext, UCHAR ComChar)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_WRITE,(TEXT("SL_XmitComChar-\n")));

    // Get critical section, then transmit when buffer empties
    EnterCriticalSection(&(pHWHead->TransmitCritSec));
    try {
        for (;;)
        {   // We know THR will eventually empty
            // Write the character if we can
            SL_ReadLineStatus(pHWHead);
            if(HW_UARTDBGFR_RD() & BM_UARTDBGFR_TXFE)
            {
                EnterCriticalSection(&(pHWHead->RegCritSec));
                // FIFO is empty, send this character
                HW_UARTDBGDR_WR(ComChar);
                LeaveCriticalSection(&(pHWHead->RegCritSec));

                //We need to discard the echo char for irda
                if (pHWHead->UseIrDA)
                {
                    pHWHead->ulDiscard++;
                }
                break;
            }

            EnterCriticalSection(&(pHWHead->RegCritSec));
            // Enable xmit intr.
            HW_UARTDBGIMSC_SET(BM_UARTDBGIMSC_TXIM);

            LeaveCriticalSection(&(pHWHead->RegCritSec));
            WaitForSingleObject(pHWHead->FlushDone, (ULONG)1000);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_XmitComChar :Exception caught \n")));
    }
    LeaveCriticalSection(&(pHWHead->TransmitCritSec));

    DEBUGMSG(ZONE_WRITE,(TEXT("SL_XmitComChar-\r\n")));
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
ULONG SL_GetStatus(PVOID pContext, LPCOMSTAT lpStat)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    ULONG RetVal  = pHWHead->CommErrors;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_GetStatus+\r\n")));

    pHWHead->CommErrors = 0; // Clear old errors each time

    if (lpStat)
    {
        try {
            if (pHWHead->CTSFlowOff)
            {
                pHWHead->Status.fCtsHold = 1;
            }
            else
            {
                pHWHead->Status.fCtsHold = 0;
            }

            if (pHWHead->DSRFlowOff)
            {
                pHWHead->Status.fDsrHold = 1;
            }
            else
            {
                pHWHead->Status.fDsrHold = 0;
            }

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
            DEBUGMSG(ZONE_WARN, (TEXT("SL_GetStatus :Exception caught \n")));
        }
    }
    else
    {
        RetVal = (ULONG)-1;
    }

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
VOID SL_GetModemStatus(PVOID pContext, PULONG pModemStatus)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_GetModemStatus+\r\n")));

    SL_ReadModemStatus(pHWHead);

    if ( HW_UARTDBGFR_RD() & BM_UARTDBGFR_CTS) 
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("MS_CTS_ON\r\n")));
        *pModemStatus |= MS_CTS_ON;
        pHWHead->ModemStatus |= MS_CTS_ON;
    }

    // MX233 does not support DTR\DSR, and the following is done to cater for Serial Communications CETK.
    *pModemStatus |= MS_DSR_ON;
    
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_GetModemStatus- \r\n")));
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
VOID SL_PurgeComm(PVOID pContext,DWORD fdwAction)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_PurgeComm+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        if (fdwAction & PURGE_TXCLEAR)
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("PURGE_TXCLEAR\r\n")));
            // not supported;
            // Hardware limitation;
        }
        if (fdwAction & PURGE_RXCLEAR)
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("PURGE_RXCLEAR\r\n")));
            // not supported;
            // Hardware limitation;
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_PurgeComm :Exception caught \n")));
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
BOOL SL_SetDCB(PVOID pHead,LPDCB lpDCB)
{
    PUART_INFO pHWHead = (PUART_INFO)pHead;
    BOOL bRet= TRUE;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetDCB 0x%X\r\n"), pHead));

    // If the device is open, scan for changes and do whatever
    // is needed for the changed fields.  if the device isn't
    // open yet, just save the DCB for later use by the open.

    DEBUGMSG (1,(TEXT("+Ser_SetDCB BaudRate %d\r\n"), lpDCB->BaudRate));
    DEBUGMSG (1,(TEXT("+Ser_SetDCB ByteSize %d\r\n"), lpDCB->ByteSize));
    DEBUGMSG (1,(TEXT("+Ser_SetDCB Parity %d\r\n"),   lpDCB->Parity));
    DEBUGMSG (1,(TEXT("+Ser_SetDCB StopBits %d\r\n"), lpDCB->StopBits));

    if(((PSER_INFO)pHead)->cOpenCount)
    {
        // Check if it's valid DCB settings
        if ((lpDCB->ByteSize <= 0) || (lpDCB->BaudRate <= 0))
        {
            return FALSE;
        }
        if (lpDCB->BaudRate != pHWHead->dcb.BaudRate)
        {
            bRet = SL_SetBaudRate(pHWHead, lpDCB->BaudRate);
        }

        if (bRet && (lpDCB->ByteSize != pHWHead->dcb.ByteSize))
        {
            bRet = SL_SetByteSize(pHWHead, lpDCB->ByteSize);
        }

        if (bRet && (lpDCB->Parity != pHWHead->dcb.Parity))
        {
            bRet = SL_SetParity(pHWHead, lpDCB->Parity);
        }

        if (bRet && (lpDCB->StopBits != pHWHead->dcb.StopBits))
        {
            bRet = SL_SetStopBits(pHWHead, lpDCB->StopBits);
        }
        if (bRet && (lpDCB->fOutxCtsFlow != pHWHead->dcb.fOutxCtsFlow))
        {
            if (lpDCB->fOutxCtsFlow)
            {
                HW_UARTDBGCR_SET(BM_UARTDBGCR_CTSEN);
            }
            else
            {
                HW_UARTDBGCR_CLR(BM_UARTDBGCR_CTSEN);
            }
        }
        if (bRet && (lpDCB->fRtsControl != pHWHead->dcb.fRtsControl))
        {
            if (lpDCB->fRtsControl == RTS_CONTROL_HANDSHAKE)
            {
                HW_UARTDBGCR_SET(BM_UARTDBGCR_RTSEN);
            }
            else
            {
                HW_UARTDBGCR_CLR(BM_UARTDBGCR_RTSEN);
            }
        }
    }
    //
    // Make sure everything was set okay.  If it was, then
    // update the whole DCB.  Otherwise, leave it as is
    // (the set routines make changes to the setting they
    // change) and return failure.
    //

    if (bRet)
    {
        // Now that we have done the right thing, store this DCB
        pHWHead->dcb = *lpDCB;
    }

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-Ser_SetDCB 0x%X bRet %d\r\n"), pHead,bRet));
    return bRet;
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
//      TRUE indicates success.
//
//-----------------------------------------------------------------------------
BOOL SL_SetCommTimeouts(PVOID pContext,LPCOMMTIMEOUTS lpCommTimeouts)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    BOOL retval = TRUE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetCommTimeout+ \r\n")));

    // check for any changes and act upon them
    if (lpCommTimeouts->WriteTotalTimeoutMultiplier !=
        pHWHead->CommTimeouts.WriteTotalTimeoutMultiplier)
    {

    }

    // Now that we have done the right thing, store this DCB
    pHWHead->CommTimeouts = *lpCommTimeouts;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetCommTimeout-\r\n")));
    return(retval);
}
//------------------------------------------------------------------------------
// Function: SL_IOControl
//
// This function sends a command to a device.wince600
//
// Parameters:
//      dwContext
//          [IN] Handle to the open context of the device.
//      Ioctl
//          [IN] I/O control operation to perform.
//      pInBuf
//          [IN] Pointer to the buffer containing data to transfer to the device.
//      InBufLen
//          [IN] Number of bytes of data in the buffer specified for pBufIn.
//      pOutBuf
//          [out] Pointer to the buffer used to transfer the output data
//                from the device.
//      OutBufLen
//          [IN] Maximum number of bytes in the buffer specified by pBufOut.
//      pdwBytesTransferred
//          [IN] Pointer to the DWORD buffer that this function uses to return
//               the actual number of bytes received from the device.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL SL_Ioctl(PVOID pContext,
              DWORD Ioctl,
              PUCHAR pInBuf,
              DWORD InBufLen,
              PUCHAR pOutBuf,
              DWORD OutBufLen,
              PDWORD pdwBytesTransferred)
{

    BOOL bRc = FALSE;
    DWORD dwErr = ERROR_INVALID_PARAMETER;
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    UNREFERENCED_PARAMETER(pInBuf);
    UNREFERENCED_PARAMETER(InBufLen);
    switch (Ioctl)
    {
    case IOCTL_POWER_CAPABILITIES:
        // tell the power manager about ourselves.
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_IOCTL_POWER_CAPABILITIES\r\n")));
        if ( pOutBuf != NULL &&
             OutBufLen >= sizeof(POWER_CAPABILITIES) &&
             pdwBytesTransferred != NULL)
        {
            PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pOutBuf;
            memset(ppc, 0, sizeof(POWER_CAPABILITIES));
            ppc->DeviceDx = 0x11;   // support D0, D4
            ppc->WakeFromDx = 0x00; // No wake capability
            ppc->InrushDx = 0x00;       // No in rush requirement
            ppc->Power[D0] = (DWORD) PwrDeviceUnspecified;
            ppc->Power[D1] = (DWORD) PwrDeviceUnspecified;
            ppc->Power[D2] = 0;
            ppc->Power[D3] = (DWORD) PwrDeviceUnspecified;
            ppc->Power[D4] = (DWORD) PwrDeviceUnspecified;
            ppc->Latency[D0] = 0;
            ppc->Latency[D1] = (DWORD) PwrDeviceUnspecified;
            ppc->Latency[D2] = 0;
            ppc->Latency[D3] = (DWORD) PwrDeviceUnspecified;
            ppc->Latency[D4] = (DWORD) PwrDeviceUnspecified;
            ppc->Flags = 0;
            *pdwBytesTransferred = sizeof(POWER_CAPABILITIES);
            dwErr = ERROR_SUCCESS;
        }
        break;

    case IOCTL_POWER_QUERY:
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_IOCTL_POWER_QUERY\r\n")));
        if ( pOutBuf != NULL &&
             OutBufLen == sizeof(CEDEVICE_POWER_STATE) &&
             pdwBytesTransferred != NULL)
        {
            CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pOutBuf;
            DEBUGMSG(ZONE_FUNCTION, (TEXT("NewDx = %d\r\n"), NewDx));

            if (VALID_DX(NewDx))
            {
                // this is a valid Dx state so return a good status
                *pdwBytesTransferred = sizeof(CEDEVICE_POWER_STATE);
                dwErr = ERROR_SUCCESS;
            }
            DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_IOCTL_POWER_QUERY %u %s\r\n"),
                                     NewDx, dwErr == ERROR_SUCCESS ?
                                     TEXT("succeeded") : TEXT("failed")));
        }
        break;

    case IOCTL_POWER_SET:
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_IOCTL_POWER_SET\r\n")));
        if ( pOutBuf != NULL &&
             OutBufLen == sizeof(CEDEVICE_POWER_STATE) &&
             pdwBytesTransferred != NULL)
        {
            CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pOutBuf;
            DEBUGMSG(ZONE_FUNCTION, (TEXT("NewDx = %d\r\n"), NewDx));
            if (NewDx != CurDx)
            {
                if (NewDx == D4)
                {
                    //Call Sl_PowerOff Function
                    SL_PowerOff(pContext);
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("SERIAL OFF\r\n")));
                }
                else
                {
                    // if asked for a state we don't support, Set the state
                    //to default power up state which is D0
                    NewDx = D0;
                    SL_PowerOn(pContext);
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("SERIAL Off\r\n")));
                }
                EnterCriticalSection(&(pHWHead->RegCritSec));
                CurDx = NewDx;
                LeaveCriticalSection(&(pHWHead->RegCritSec));
                *(PCEDEVICE_POWER_STATE)pOutBuf = CurDx;
            }
            *pdwBytesTransferred = sizeof(CEDEVICE_POWER_STATE);
            DEBUGMSG(ZONE_FUNCTION, (TEXT("CurDx = %d\r\n"), CurDx));
            dwErr = ERROR_SUCCESS;
        }
        break;

    case IOCTL_POWER_GET:
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_IOCTL_POWER_GET\r\n")));
        if ( pOutBuf != NULL &&
             OutBufLen == sizeof(CEDEVICE_POWER_STATE) &&
             pdwBytesTransferred != NULL)
        {
            // just return our CurrentDx value
            *(PCEDEVICE_POWER_STATE)pOutBuf = CurDx;
            *pdwBytesTransferred = sizeof(CEDEVICE_POWER_STATE);
            dwErr = ERROR_SUCCESS;
        }
        break;

    default:
        DEBUGMSG(ZONE_ERROR, (TEXT("Sl_IOControl:Unsupported IOCTL code %u\r\n"), Ioctl));
        dwErr = ERROR_NOT_SUPPORTED;
        break;
    }

    // pass back appropriate response codes
    SetLastError(dwErr);
    if (dwErr != ERROR_SUCCESS)
    {
        bRc = FALSE;
    }
    else
    {
        bRc = TRUE;
    }

    return bRc;
}

