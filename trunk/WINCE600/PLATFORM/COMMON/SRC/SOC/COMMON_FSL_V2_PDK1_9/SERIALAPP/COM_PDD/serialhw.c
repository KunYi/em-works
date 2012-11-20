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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
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
#include "common_ddkvs.h"
#include "common_uartapp.h"
#include "serial.h"

//------------------------------------------------------------------------------
// External Functions
extern DWORD BSPUartGetIndex(ULONG HWAddr);
extern BOOL  BSPUartConfigureHandshake(ULONG HWAddr);	// CS&ZHL JUN-14-2012: RTS/CTS config

//
// LQK NOV-2-2012: supporting GPIO based RTSn
//
extern DWORD BSPUartGPIO2RTS( DWORD dwGpioRTS );		// select GPIO as RTS pin
extern BOOL	BSPUartConfigureRTS( DWORD dwGpioRTS );		// enable RTS
extern BOOL BSPUartSetGPIORTS( DWORD dwGpioRTS );		// set RTS
extern BOOL BSPUartClearGPIORTS( DWORD dwGpioRTS );		// clear RTS

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines
#define MIN_VAL(a, b)      (((a) > (b)) ? (b) : (a))

#define DISABLEUART        HW_UARTAPPCTRL2_CLR(pHWHead->dwIndex, BM_UARTAPPCTRL2_UARTEN | BM_UARTAPPCTRL2_RXE | BM_UARTAPPCTRL2_TXE)
#define ENABLEUART         HW_UARTAPPCTRL2_SET(pHWHead->dwIndex, BM_UARTAPPCTRL2_UARTEN | BM_UARTAPPCTRL2_RXE | BM_UARTAPPCTRL2_TXE)


#define virt2phys(addr)           (VOID*)(((UINT32) (addr) - (UINT32) &pSerHead->pSerialVirtTxDMADescAddr[0]) + \
                                           (UINT32) pSerHead->SerialPhysTxDMADescAddr.LowPart)

#define GetRXDescvirt2phys(addr)  (VOID*)(((UINT32) (addr) - (UINT32) &pSerHead->pSerialVirtRxDMADescAddr[0]) + \
                                           (UINT32) pSerHead->SerialPhysRxDMADescAddr.LowPart)

//------------------------------------------------------------------------------
// Types
//
// LQK NOV-2-2012: supporting GPIO based RTSn
//
#define DDK_IOMUX_INVALID_PIN			255
#define IOCTL_SET_UART_RTS_PIN			CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3924, METHOD_BUFFERED, FILE_ANY_ACCESS)
//------------------------------------------------------------------------------
// Global Variables

//WINCE600
// Current Serial device power state.
static CEDEVICE_POWER_STATE CurDx;


//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------

static VOID SetupRXDescDESC(PVOID pContext,
                            UARTAPP_RXDMA_DESCRIPTOR* dma,
                            UARTAPP_RXDMA_DESCRIPTOR* prev_dma,
                            const UINT32 pBuf,
                            UINT32 size)
{

    PUART_INFO pHWHead = (PUART_INFO)pContext;
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    dma->NextCmdDesc = 0;

    // First clear all the bits
    dma->CMD.CommandBits = 0;

    // Next, set desired bits.
    dma->CMD.B.Command    = DDK_DMA_COMMAND_DMA_WRITE;
    dma->CMD.B.IRQ        = 1;
    dma->CMD.B.Semaphore  = 1;
    dma->CMD.B.WaitForEnd = 1;

    dma->CMD.B.HALTONTERMINATE = 0;
    dma->CMD.B.TERMINATEFLUSH  = 1;

    // Setup PIO words.
    dma->CMD.B.PIOWords   = 1;
    dma->RegCTRL0.U = 0;

    dma->CMD.B.DMABytes = (UINT16) size;

    if (prev_dma != NULL)
    {
        prev_dma->NextCmdDesc = (UINT32) GetRXDescvirt2phys(dma);
        prev_dma->CMD.B.Chain = 1;
    }
    dma->BufAddr = pBuf;
    dma->RegCTRL0.U = HW_UARTAPPCTRL0_RD(pHWHead->dwIndex);
    dma->RegCTRL0.B.XFER_COUNT = (UINT16)size;
}

//
// CS&ZHL AUG-23-2012: a new setup for Rx DMA descriptor
//
static VOID SetupRXDescEx(PVOID pContext,
                          UARTAPP_RXDMA_DESCRIPTOR* dma,
                          UARTAPP_RXDMA_DESCRIPTOR* next_dma,
                          const UINT32 pBuf,
                          UINT32 size)
{

    PUART_INFO pHWHead = (PUART_INFO)pContext;
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    // First clear all the bits
    dma->CMD.CommandBits = 0;

    // Next, set desired bits.
    dma->CMD.B.Command    = DDK_DMA_COMMAND_DMA_WRITE;
    dma->CMD.B.IRQ        = 1;
    dma->CMD.B.Semaphore  = 1;
    dma->CMD.B.WaitForEnd = 1;

    dma->CMD.B.HALTONTERMINATE = 0;
    dma->CMD.B.TERMINATEFLUSH  = 1;

    // Setup PIO words.
    dma->CMD.B.PIOWords   = 1;
    dma->RegCTRL0.U = 0;

    dma->CMD.B.DMABytes = (UINT16) size;

	// processing chain
    dma->NextCmdDesc = 0;
    if (next_dma != NULL)
    {
        dma->NextCmdDesc = (UINT32) GetRXDescvirt2phys(next_dma);
        dma->CMD.B.Chain = 1;
    }
    dma->BufAddr = pBuf;
    dma->RegCTRL0.U = HW_UARTAPPCTRL0_RD(pHWHead->dwIndex);
    dma->RegCTRL0.B.XFER_COUNT = (UINT16)size;
}

static VOID SetupTXDescDESC(PVOID pContext,
                            UARTAPP_TXDMA_DESCRIPTOR* dma,
                            UARTAPP_TXDMA_DESCRIPTOR* prev_dma,
                            const UINT32 pBuf,
                            UINT32 size)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    dma->NextCmdDesc = 0;

    // First clear all the bits
    dma->CMD.CommandBits = 0;

    // Next, set desired bits.
    dma->CMD.B.Command    = DDK_DMA_COMMAND_DMA_READ;
    dma->CMD.B.IRQ        = 1;
    dma->CMD.B.Semaphore  = 1;
    dma->CMD.B.WaitForEnd = 1;

    // Setup PIO words.
    dma->CMD.B.PIOWords   = 1;
    dma->RegCTRL1.U = 0;

    dma->CMD.B.DMABytes = (UINT16) size;

    if (prev_dma != NULL)
    {
        prev_dma->NextCmdDesc = (UINT32) virt2phys(dma);

        prev_dma->CMD.B.Chain = 1;
        prev_dma->CMD.B.IRQ = 0;
    }
    dma->BufAddr = pBuf;

    dma->RegCTRL1.U = HW_UARTAPPCTRL1_RD(pHWHead->dwIndex);
    dma->RegCTRL1.B.XFER_COUNT = (UINT16)size;
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
VOID SL_ReadModemStatus(PVOID pContext)
{
    PUART_INFO pHWHead = (PUART_INFO)pContext;
    ULONG Events = 0;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ReadModemStatus+\r\n")));

    // CTS(RTS) status change
    if ( HW_UARTAPPINTR_RD(pHWHead->dwIndex) & BM_UARTAPPINTR_CTSMIS )
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("EV_CTS\r\n")));
        HW_UARTAPPINTR_CLR(pHWHead->dwIndex, BM_UARTAPPINTR_CTSMIS);
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
        u32Status = HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & ALL_RX_ERROR_MASK;
        // Clear all Rx error bits.
        HW_UARTAPPSTAT_WR(pHWHead->dwIndex, HW_UARTAPPSTAT_RD(pHWHead->dwIndex)&~ALL_RX_ERROR_MASK);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        u32Status = 0;
        DEBUGMSG(ZONE_WARN, (TEXT("SL_ReadLineStatus :Exception caught \n")));
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    // Check for an overrun error.
    if((u32Status & BM_UARTAPPSTAT_OERR))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error Overrun\r\n")));
        pHWHead->DroppedBytes++;
        pHWHead->CommErrors |= CE_OVERRUN;
        LineEvents |= EV_ERR;
    }
    // Check for a break error.
    if(u32Status & BM_UARTAPPSTAT_BERR)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error Break detect!!!\r\n")));
        LineEvents |= EV_BREAK;
    }
    // Check for a parity error.
    if(u32Status & BM_UARTAPPSTAT_PERR)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Error Parity\r\n")));
        pHWHead->CommErrors |= CE_RXPARITY;
        LineEvents |= EV_ERR;
    }
    // Check for a framing error.
    if(u32Status & BM_UARTAPPSTAT_FERR)
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
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_ClearPendingInts+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // Clear all Pending Interrupts.
        HW_UARTAPPINTR_CLR(pHWHead->dwIndex, ALL_INT_STATUS_MASK);

        if(pSerHead->useDMA)
        {
            DDKApbxDmaClearCommandCmpltIrq(pSerHead->SerialDmaChanRx);
            DDKApbxDmaClearCommandCmpltIrq(pSerHead->SerialDmaChanTx);
        }
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
            HW_UARTAPPLINECTRL_CLR(pHWHead->dwIndex, BM_UARTAPPLINECTRL_STP2); 
            break;

        case ONE5STOPBITS:
            bRet = FALSE;
            break;
        case TWOSTOPBITS:
            HW_UARTAPPLINECTRL_SET(pHWHead->dwIndex, BM_UARTAPPLINECTRL_STP2); 
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

    divisor = BUILD_UARTAPP_BAUDRATE_DIVIDER(u32BaudRate);
    if((divisor < MIN_UARTAPP_DIVISOR)|| (divisor > MAX_UARTAPP_DIVISOR))
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
        HW_UARTAPPLINECTRL_CLR(pHWHead->dwIndex, BM_UARTAPPLINECTRL_BAUD_DIVINT); 

        // Set new value
        HW_UARTAPPLINECTRL_SET(pHWHead->dwIndex, BF_UARTAPPLINECTRL_BAUD_DIVINT(GET_UARTAPP_BAUD_DIVINT(u32BaudRate))); 

        // First clear the bits
        HW_UARTAPPLINECTRL_CLR(pHWHead->dwIndex, BM_UARTAPPLINECTRL_BAUD_DIVFRAC); 

        // Set new value
        HW_UARTAPPLINECTRL_SET(pHWHead->dwIndex, BF_UARTAPPLINECTRL_BAUD_DIVFRAC(GET_UARTAPP_BAUD_DIVFRAC(u32BaudRate))); 

        //Enable RX & TX
        ENABLEUART;

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

        HW_UARTAPPLINECTRL_CLR(pHWHead->dwIndex, BM_UARTAPPLINECTRL_WLEN); 
        switch (ByteSize) {
        case 5:

            break;
        case 6:
            HW_UARTAPPLINECTRL_SET(pHWHead->dwIndex, BF_UARTAPPLINECTRL_WLEN(UARTAPP_LINECTRL_WLEN_6)); 
            break;
        case 7:
            HW_UARTAPPLINECTRL_SET(pHWHead->dwIndex, BF_UARTAPPLINECTRL_WLEN(UARTAPP_LINECTRL_WLEN_7)); 
            break;
        case 8:
            HW_UARTAPPLINECTRL_SET(pHWHead->dwIndex, BF_UARTAPPLINECTRL_WLEN(UARTAPP_LINECTRL_WLEN_8)); 
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
            HW_UARTAPPLINECTRL_CLR(pHWHead->dwIndex, BM_UARTAPPLINECTRL_EPS);
            HW_UARTAPPLINECTRL_SET(pHWHead->dwIndex, BM_UARTAPPLINECTRL_PEN);
            break;
        case EVENPARITY:
            HW_UARTAPPLINECTRL_SET(pHWHead->dwIndex, (BM_UARTAPPLINECTRL_EPS | BM_UARTAPPLINECTRL_PEN)); 
            break;
        case MARKPARITY:
        case SPACEPARITY:
            bRet = FALSE;
            break;
        case NOPARITY:
            HW_UARTAPPLINECTRL_CLR(pHWHead->dwIndex, (BM_UARTAPPLINECTRL_EPS | BM_UARTAPPLINECTRL_PEN)); 
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
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_Reset+\r\n")));

    memset(&pHWHead->Status, 0, sizeof(COMSTAT));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // Reset the Block
        HW_UARTAPPCTRL0_SET(pHWHead->dwIndex, BM_UARTAPPCTRL0_SFTRST); 
        Sleep(10);
        // Release the Block from Reset and starts the clock
        HW_UARTAPPCTRL0_CLR(pHWHead->dwIndex, BM_UARTAPPCTRL0_SFTRST | BM_UARTAPPCTRL0_CLKGATE); 
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        DEBUGMSG(ZONE_WARN, (TEXT("SL_Reset :Exception caught \n")));
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

    pHWHead->UseIrDA = bIR;
    pHWHead->HwAddr  = HWAddress;
    pHWHead->dwIndex = BSPUartGetIndex(HWAddress);
	//RETAILMSG(1, (TEXT("SL_INIT:: Index=%d, HwAddr = 0x%x, pRegBase = 0x%x\r\n"), pHWHead->dwIndex, pHWHead->HwAddr, pRegBase));

    switch(pHWHead->dwIndex)
    {
        case 0:
            pHWHead->pv_HWregUARTApp0 = (DWORD)pRegBase;
            break;
        case 1:
            pHWHead->pv_HWregUARTApp1 = (DWORD)pRegBase;
            break;
        case 2:
            pHWHead->pv_HWregUARTApp2 = (DWORD)pRegBase;
            break;
        case 3:
            pHWHead->pv_HWregUARTApp3 = (DWORD)pRegBase;
            break;
        case 4:
            pHWHead->pv_HWregUARTApp4 = (DWORD)pRegBase;
            break;
        default:
            ERRORMSG(TRUE, (TEXT("AUART Index invalid\r\n")));
            return FALSE;
    }

    // Store info for callback function
    pHWHead->EventCallback = EventCallback;
    pHWHead->pMDDContext = pMDDContext;

    // Now set up remaining fields

    pHWHead->FlushDone = CreateEvent(0, FALSE, FALSE, NULL);
    pHWHead->ulDiscard = 0;

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
    // Turn off APP UART Clock
    HW_UARTAPPCTRL0_SET(pHWHead->dwIndex, BM_UARTAPPCTRL0_CLKGATE); 


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
    PSER_INFO pSerHead = (PSER_INFO)pContext;
    UINT8 i;

    DEBUGMSG(ZONE_OPEN, (TEXT("SL_Open+ \r\n")));
	RETAILMSG(1, (TEXT("SL_Open:: Index=%d\r\n"), pHWHead->dwIndex));

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

        // Reset the Block
        HW_UARTAPPCTRL0_SET(pHWHead->dwIndex, BM_UARTAPPCTRL0_SFTRST); 

        // Release the Block from Reset and starts the clock
        HW_UARTAPPCTRL0_CLR(pHWHead->dwIndex, BM_UARTAPPCTRL0_SFTRST | BM_UARTAPPCTRL0_CLKGATE); 

        // Set the enable FIFO bit.
        HW_UARTAPPLINECTRL_SET(pHWHead->dwIndex, BM_UARTAPPLINECTRL_FEN); 

        // Set the FIFO trigger levels
        HW_UARTAPPCTRL2_CLR(pHWHead->dwIndex, BM_UARTAPPCTRL2_TXIFLSEL | BM_UARTAPPCTRL2_RXIFLSEL);
        HW_UARTAPPCTRL2_SET(pHWHead->dwIndex, BF_UARTAPPCTRL2_TXIFLSEL(UART_APP_FIFO_ONE_EIGHT) | BF_UARTAPPCTRL2_RXIFLSEL(UART_APP_FIFO_ONE_HALF)); 

        // Get defaults from the DCB structure
        SL_SetByteSize(pContext, pHWHead->dcb.ByteSize);
        SL_SetStopBits(pContext, pHWHead->dcb.StopBits);
        SL_SetParity(pContext, pHWHead->dcb.Parity);
        SL_SetBaudRate(pContext, pHWHead->dcb.BaudRate);

		// LQK NOV-2-2012: 
		pSerHead->dwRtsGpioPin = DDK_IOMUX_INVALID_PIN;

		if(pSerHead->useDMA)
        {
            // Reset the Tx DMA Channel and Enable DMA Interrupt
            if(!DDKApbxDmaInitChan(pSerHead->SerialDmaChanTx,TRUE))
            {
                DEBUGMSG(ZONE_INIT, (TEXT("SL_OPEN :Failed to  Init TX DMA \n")));
            }

            // Reset the Rx DMA Channel and Enable DMA Interrupt
            if(!DDKApbxDmaInitChan(pSerHead->SerialDmaChanRx,TRUE))
            {
                DEBUGMSG(ZONE_INIT, (TEXT("SL_OPEN :Failed to  Init RX DMA \n")));
            }

            // Enable the Application UART DMA bits.
            HW_UARTAPPCTRL2_SET(pHWHead->dwIndex, BM_UARTAPPCTRL2_RXDMAE |BM_UARTAPPCTRL2_TXDMAE);

            // Set the Rx DMA abort on error bit, if requested.
            // Note: Hardware defaults to OFF out of reset.
            // HW_UARTAPPCTRL2_SET(pHWHead->dwIndex, BM_UARTAPPCTRL2_DMAONERR); 

            // Set the RXDMA Timeout counter
            HW_UARTAPPCTRL0_SET(pHWHead->dwIndex, BM_UARTAPPCTRL0_RXTO_ENABLE); 

            // First clear the bits
            // HW_UARTAPPCTRL0_CLR(pHWHead->dwIndex, BM_UARTAPPCTRL0_RXTIMEOUT); 

            // Set new value
            // HW_UARTAPPCTRL0_SET(pHWHead->dwIndex, BF_UARTAPPCTRL0_RXTIMEOUT(SERIAL_DMA_RX_TIMEOUT)); 

            // Init the RX DMA chain
            for (i = 0; i < SERIAL_MAX_DESC_COUNT_RX; i++)
            {
                //SetupRXDescDESC(pHWHead,
                //                &pSerHead->pSerialVirtRxDMADescAddr[i],
                //                (i == 0) ? NULL : &pSerHead->pSerialVirtRxDMADescAddr[i - 1],
                //                pSerHead->SerialPhysRxDMABufferAddr.LowPart + (i * pSerHead->rxDMABufSize),
                //                pSerHead->rxDMABufSize);
				// CS&ZHL AUG-23-2012: use new setup
                SetupRXDescEx(pHWHead,
                              &pSerHead->pSerialVirtRxDMADescAddr[i],
                              &pSerHead->pSerialVirtRxDMADescAddr[(i + 1) % SERIAL_MAX_DESC_COUNT_RX],
                              pSerHead->SerialPhysRxDMABufferAddr.LowPart + (i * pSerHead->rxDMABufSize),
                              pSerHead->rxDMABufSize);
            }

            pSerHead->currRxDmaBufId = 0;
            pSerHead->dmaRxStartIdx = 0;
            pSerHead->availRxByteCount = 0;

            // Enable the RXDMA interrupt
            DDKApbxDmaEnableCommandCmpltIrq(pSerHead->SerialDmaChanRx,TRUE);

            // Start DMA RX Chain with semaphore count = i => SERIAL_MAX_DESC_COUNT_RX
            DDKApbxStartDma(pSerHead->SerialDmaChanRx,(PVOID) GetRXDescvirt2phys(&pSerHead->pSerialVirtRxDMADescAddr[0]), i);
        }
        // Enable the UART
        ENABLEUART;

        SL_ClearPendingInts(pHWHead);

        if(!pSerHead->useDMA)
        {
            HW_UARTAPPINTR_SET(pHWHead->dwIndex, ( BM_UARTAPPINTR_RXIEN | BM_UARTAPPINTR_RTIEN |
                                 BM_UARTAPPINTR_OEIEN | BM_UARTAPPINTR_CTSMIEN )); 
        }
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
    PSER_INFO pSerHead = (PSER_INFO)pContext;

    DEBUGMSG(ZONE_CLOSE,(TEXT("SL_Close+ \r\n")));
	RETAILMSG(1, (TEXT("SL_Close:: Index=%d\r\n"), pHWHead->dwIndex));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try
    {
        SL_ClearPendingInts(pHWHead);

        // Disable UART
        DISABLEUART;

        if(pSerHead->useDMA)
        {
            // Disable TX DMA interrupt
            DDKApbxDmaEnableCommandCmpltIrq(pSerHead->SerialDmaChanTx,FALSE);
            DDKApbxDmaEnableCommandCmpltIrq(pSerHead->SerialDmaChanRx,FALSE);

            // Stop the DMA if it is running
            DDKApbxStopDma(pSerHead->SerialDmaChanTx);
            DDKApbxStopDma(pSerHead->SerialDmaChanRx);
        }
        // Turn off APP UART Clock
        HW_UARTAPPCTRL0_SET(pHWHead->dwIndex, BM_UARTAPPCTRL0_CLKGATE); 
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

    PUART_INFO pHWHead   = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_PowerOff+ \r\n")));

    // Disable UART
    DISABLEUART;

    // Turn off APP UART Clock
    HW_UARTAPPCTRL0_SET(pHWHead->dwIndex, BM_UARTAPPCTRL0_CLKGATE); 

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
    PUART_INFO pHWHead = (PUART_INFO)pContext;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SL_PowerOn+ \r\n")));

    /*pHWHead->ulDiscard = 0;*/
    // Restore any registers that we need
    // In power handler context, so don't try to do a critical section
    // Turn on APP UART Clock
    HW_UARTAPPCTRL0_CLR(pHWHead->dwIndex, BM_UARTAPPCTRL0_CLKGATE); 

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
    PSER_INFO pSerHead = (PSER_INFO)pHead;
    INTERRUPT_TYPE interrupts  = INTR_NONE;
    ULONG u32Interrupt;

    DEBUGMSG( ZONE_FUNCTION, (TEXT("+Ser_GetInterruptType \r\n")));
    EnterCriticalSection(&(pHWHead->RegCritSec));

    u32Interrupt = HW_UARTAPPINTR_RD(pHWHead->dwIndex) & ALL_INT_STATUS_MASK; 

    if ( (u32Interrupt & BM_UARTAPPINTR_CTSMIS) && (u32Interrupt & BM_UARTAPPINTR_CTSMIEN))
    {
        interrupts |= INTR_MODEM;
    }

    if ( (u32Interrupt & BM_UARTAPPINTR_OEIS) || (u32Interrupt & BM_UARTAPPINTR_BEIS) ||
         (u32Interrupt & BM_UARTAPPINTR_FEIS) || (u32Interrupt & BM_UARTAPPINTR_PEIS))
    {
        HW_UARTAPPINTR_CLR(pHWHead->dwIndex, (BM_UARTAPPINTR_OEIS | BM_UARTAPPINTR_BEIS | BM_UARTAPPINTR_FEIS |
                            BM_UARTAPPINTR_PEIS) & ALL_INT_STATUS_MASK);
  
        interrupts |= INTR_LINE;
    }

    // Capture the old mask to return to the caller.
    if(pSerHead->useDMA)
    {

        if( DDKApbxDmaGetActiveIrq(pSerHead->SerialDmaChanTx))
        {
            DDKApbxDmaClearCommandCmpltIrq(pSerHead->SerialDmaChanTx);
            interrupts |= INTR_TX;
        }
        if( DDKApbxDmaGetActiveIrq(pSerHead->SerialDmaChanRx))
        {
            DDKApbxDmaClearCommandCmpltIrq(pSerHead->SerialDmaChanRx);
            interrupts |= INTR_RX;
        }
        if( DDKApbxDmaGetErrorStatus(pSerHead->SerialDmaChanRx) == DDK_DMA_ERROR_EARLYTERM )
        {
            DDKApbxDmaClearErrorIrq(pSerHead->SerialDmaChanRx);
            interrupts |= INTR_RX;
        }

		//
		// CS&ZHL AUG23-2012: set INTR_RX if data aiavlable in Rx DMA buffer
		//
		if( pSerHead->availRxByteCount )
		{
            interrupts |= INTR_RX;
		}

        // Clear all signalled events.
        HW_UARTAPPINTR_CLR(pHWHead->dwIndex, ALL_INT_STATUS_MASK);
    }
    else
    {

        try {
            if ( u32Interrupt & BM_UARTAPPINTR_TXIS )
            {
                HW_UARTAPPINTR_CLR(pHWHead->dwIndex, BM_UARTAPPINTR_TXIS & ALL_INT_STATUS_MASK);
                interrupts |= INTR_TX;
            }
            if ((u32Interrupt & BM_UARTAPPINTR_RTIS) || (u32Interrupt & BM_UARTAPPINTR_RXIS))
            {
                HW_UARTAPPINTR_CLR(pHWHead->dwIndex, ((BM_UARTAPPINTR_RTIS | BM_UARTAPPINTR_RXIS))& ALL_INT_STATUS_MASK);

                interrupts |= INTR_RX;
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            u32Interrupt = INTR_NONE;             // simulate no interrupt
            DEBUGMSG(ZONE_FUNCTION, (TEXT("Ser_GetInterruptType :Exception caught \n")));
        }

    }
    HW_UARTAPPINTR_CLR(pHWHead->dwIndex, 0xd);
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
    PSER_INFO pSerHead = (PSER_INFO)pContext;
    ULONG RetVal                = 0;
    ULONG TargetRoom    = *pByteNumber;
    BOOL fRXFlag               = FALSE;
    BOOL fReplaceparityErrors = FALSE;
    BOOL fNull;
    UCHAR cEvtChar, cRXChar;
    UINT32 u32Data  = 0;
    UINT32 u32RxErrStatus;
    UINT32 copyCount;
    LPBYTE pBuffer;
    UINT8 i;
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
        if(pSerHead->useDMA == FALSE )
        {
            while ( TargetRoom )
            {
                // See if there is another byte to be read
                u32Status = HW_UARTAPPSTAT_RD(pHWHead->dwIndex);
                if (!(u32Status & BM_UARTAPPSTAT_RXFE))
                {
                    // Read the RX register
                    u32Data = HW_UARTAPPDATA_RD(pHWHead->dwIndex);
                    cRXChar = (UCHAR)( u32Data & 0xFF );

                    if (HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & INVALID_DATA)
                    {
                        // Do nothing - byte gets discarded
                        DEBUGMSG(ZONE_READ, (TEXT("Invalid Data\r\n")));
                        continue;
                    }
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
        else    // Rx using DMA
        {
            *pByteNumber = 0;
            RetVal = 0;     // tracks bytes dropped due to MDD buffer inadequacy

            // attempt to fill the TargetRoom space in MDD buffer as much as
            // possible, by copying from one or more of SDMA completed Rx
            // buffers. When a SDMA buffer has been emptied (by copying to MDD),
            // re-issue SDMA request. If the SDMA buffer has not yet been
            // emptied, update availRxByteCount and dmaRxStartIdx and return.

            while (TargetRoom > 0)
            {
                // Check for errors in the data and set return value.
                u32RxErrStatus = SL_ReadLineStatus(pHWHead);
                if(u32RxErrStatus != 0)
                {
                    // To ensure correct behavior, the byte count is set to zero when
                    // *ANY* error is detected.
                    *pByteNumber = 0;
                    break;
                }

                // First RX Desc has finished
				//LQK:Aug 23,2012
				if( pSerHead->availRxByteCount == 0 )
				{
					pSerHead->availRxByteCount = HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & 0xFFFF;
					pSerHead->dmaRxStartIdx = pSerHead->currRxDmaBufId * pSerHead->rxDMABufSize;		//0;
				}
                copyCount = MIN_VAL(pSerHead->availRxByteCount, TargetRoom);

                if (copyCount)
                {
                    pBuffer = (LPBYTE)(pTargetBuffer + *pByteNumber);

                    memcpy(pBuffer,
                           pSerHead->pSerialVirtRxDMABufferAddr + pSerHead->dmaRxStartIdx,
                           copyCount);

                    *pByteNumber += copyCount;
                    TargetRoom -= copyCount;
					pSerHead->availRxByteCount -= copyCount;
					pSerHead->dmaRxStartIdx += copyCount;
                }

				if( pSerHead->availRxByteCount == 0 )
				{
					// re-init current RxDMA descriptor
					i = pSerHead->currRxDmaBufId;
					SetupRXDescEx(pHWHead,
								  &pSerHead->pSerialVirtRxDMADescAddr[i],
								  &pSerHead->pSerialVirtRxDMADescAddr[(i + 1) % SERIAL_MAX_DESC_COUNT_RX],
								  pSerHead->SerialPhysRxDMABufferAddr.LowPart + (i * pSerHead->rxDMABufSize),
								  pSerHead->rxDMABufSize);

					// Enable the RXDMA interrupt
					DDKApbxDmaEnableCommandCmpltIrq(pSerHead->SerialDmaChanRx,TRUE);

					// increase semaphore count by 1
					DDKApbxStartDma(pSerHead->SerialDmaChanRx, (PVOID)GetRXDescvirt2phys(&pSerHead->pSerialVirtRxDMADescAddr[i]), 0);

					// clear Interrupt flag
					DDKApbxDmaClearCommandCmpltIrq(pSerHead->SerialDmaChanRx);

					// points to next RxDMA buffer
                    pSerHead->currRxDmaBufId = (pSerHead->currRxDmaBufId + 1 ) % SERIAL_MAX_DESC_COUNT_RX;

					// current Rx DMA buffer is no more data, so break
					break;
				}
            }
            pHWHead->DroppedBytes = RetVal;
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

        DEBUGMSG(ZONE_WARN, (TEXT("SL_RxIntrHandler :Exception caught \n")));

        // just exit
    }

    if (pSerHead->useDMA == FALSE)
    {
        // if we saw one (or more) EVT chars, then generate an event
        if(fRXFlag) {
            EvaluateEventFlag(pHWHead->pMDDContext, EV_RXFLAG);
        }

        if ( pHWHead->DroppedBytes )
            RETAILMSG (0, (TEXT("Rx drop %d.\r\n"), pHWHead->DroppedBytes));
    }
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
    PSER_INFO pSerHead       = (PSER_INFO)pContext;
    ULONG NumberOfBytes = *pByteNumber;

    UINT32 curByteCount;
    LPBYTE pBuffer;
    UINT8 bufId;

    DEBUGMSG(ZONE_WRITE, (TEXT("SL_TxIntrHandler+\r\n")));

    DEBUGMSG(ZONE_WRITE, (TEXT("Transmit Event 0x%X, Len %d\r\n"), pContext, *pByteNumber));

    // We may be done sending.  If so, just disable the TX interrupts
    // and return to the MDD.
    if (!*pByteNumber)
    {
        DEBUGMSG(ZONE_WRITE, (TEXT("SL_TxIntrHandler - Disable INTR_TX.\r\n")));

        if(pSerHead->useDMA )
        {
            // Disable TX DMA interrupt
            DDKApbxDmaEnableCommandCmpltIrq(pSerHead->SerialDmaChanTx,FALSE);

			//
			// LQK NOV-5-2012: if RTS_CONTROL_TOGGLE, wait all data transfer completed before clearing RTSn
			// 增加RTS信号的稳定信，所以将清RTS信号放在中断中。
			if (pHWHead->dcb.fRtsControl == RTS_CONTROL_TOGGLE)
			{
				DWORD	dwDelayMilliSeconds;
				DWORD	dwStartCount, dwCurrCount;
				DWORD	dwElipsedMilliSeconds = 0;

				dwDelayMilliSeconds = 9600 / pHWHead->dcb.BaudRate;
				if(dwDelayMilliSeconds == 0)
				{
					dwDelayMilliSeconds = 1;
				}

				dwStartCount = GetTickCount();
				while(dwElipsedMilliSeconds < dwDelayMilliSeconds)
				{
					// check transmit complete bit
					//if ( HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & BM_UARTAPPSTAT_TXFE && 
					//	((HW_UARTAPPLINECTRL_RD( pHWHead->dwIndex) && BM_UARTAPPLINECTRL_FEN) == 0) )
					if ( HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & BM_UARTAPPSTAT_TXFE )
					{
						break;
					}

					// check timeout
					dwCurrCount = GetTickCount();
					if(dwCurrCount >= dwStartCount)
					{
						dwElipsedMilliSeconds = dwCurrCount - dwStartCount;
					}
					else
					{
						dwElipsedMilliSeconds = ~dwStartCount + dwCurrCount + 1;
					}
				}

				// clear GPIO based RTSn if required
				BSPUartClearGPIORTS(pSerHead->dwRtsGpioPin);
			}

        }
        else
        {
            //Disable the TX interrupts
            HW_UARTAPPINTR_CLR(pHWHead->dwIndex, BM_UARTAPPINTR_TXIEN);
        }
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
        if (pHWHead->dcb.fOutxCtsFlow && ((HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & BM_UARTAPPSTAT_CTS) == 0))
        {
            DEBUGMSG(ZONE_WRITE, (TEXT("SL_TxIntrHandler - fOutxCtsFlow true\r\n")));

            pHWHead->CTSFlowOff = TRUE;  // Record flowed off state

            // disable TX interrupts while flowed off
            HW_UARTAPPINTR_CLR(pHWHead->dwIndex, BM_UARTAPPINTR_TXIEN);

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
        if (pSerHead->useDMA == FALSE)
        {
            //always enable FIFO
            //byteCount = UARTAPP_TXFIFO_DEPTH;
            //Application UART module has not TX FIFO empty interrupt, and only number of DATA in TX FIFO
            //decreases to less than trigger level, TX interrupt is triggered, so we may fill the TX FIFO fully and leave.

            // Wait until there is room in the FIFO
            while(HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & BM_UARTAPPSTAT_TXFF);

            for (*pByteNumber = 0; NumberOfBytes/* && byteCount*/; NumberOfBytes--/*, byteCount--*/)
            {
                HW_UARTAPPDATA_WR(pHWHead->dwIndex, *pSourceBuffer);

                // Check again if FIFO is FULL
                if(HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & BM_UARTAPPSTAT_TXFF)
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
            HW_UARTAPPINTR_SET(pHWHead->dwIndex, BM_UARTAPPINTR_TXIEN);

            DEBUGMSG(ZONE_WRITE, (TEXT("SL_TxIntrHandler: Enable INTR_TX.\r\n")));
        }
        else            // Tx using DMA
        {
            // issue DMA requests for the requested bytes on available Tx buffer descriptors.
            for (bufId = 0; (bufId < SERIAL_MAX_DESC_COUNT_TX) && (NumberOfBytes > 0); bufId++)
            {
                curByteCount = MIN_VAL(NumberOfBytes, pSerHead->txDMABufSize);

                pBuffer = (LPBYTE)(pSourceBuffer + *pByteNumber);


                memcpy(pSerHead->pSerialVirtTxDMABufferAddr +
                       bufId * pSerHead->txDMABufSize,
                       pBuffer,
                       curByteCount);

                SetupTXDescDESC(pHWHead,
                                &pSerHead->pSerialVirtTxDMADescAddr[bufId],
                                (bufId == 0) ? NULL : &pSerHead->pSerialVirtTxDMADescAddr[bufId - 1],
                                pSerHead->SerialPhysTxDMABufferAddr.LowPart + (bufId * pSerHead->txDMABufSize),
                                curByteCount);

                NumberOfBytes -= curByteCount;
                *pByteNumber += curByteCount;
            }

            // Enable the DMA interrupt
            DDKApbxDmaEnableCommandCmpltIrq(pSerHead->SerialDmaChanTx,TRUE);

            if (*pByteNumber > 0)
            {
                DDKApbxStartDma(pSerHead->SerialDmaChanTx,(PVOID) virt2phys(&pSerHead->pSerialVirtTxDMADescAddr[0]), bufId);
            }
        }
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

    if (pHWHead->CTSFlowOff && 
        (HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & BM_UARTAPPSTAT_CTS))
    {
        pHWHead->CTSFlowOff = FALSE;
        // CTS is set, so go ahead and resume sending
        // Enable TX intr.
        HW_UARTAPPINTR_SET(pHWHead->dwIndex, BM_UARTAPPINTR_TXIEN);

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
    // Application UART does not support DTR LINE
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
    // Application UART does not support DTR LINE
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
	//PSER_INFO pSerHead = (PSER_INFO) pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_ClearRTS+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
		/*//
		// LQK NOV-2-2012: if RTS_CONTROL_TOGGLE, wait all data transfer completed before clearing RTSn
		//
		if (pHWHead->dcb.fRtsControl == RTS_CONTROL_TOGGLE)
		{
			DWORD	dwDelayMilliSeconds;
			DWORD	dwStartCount, dwCurrCount;
			DWORD	dwElipsedMilliSeconds = 0;

			dwDelayMilliSeconds = 9600 / pHWHead->dcb.BaudRate;
			if(dwDelayMilliSeconds == 0)
			{
				dwDelayMilliSeconds = 1;
			}

			dwStartCount = GetTickCount();
			while(dwElipsedMilliSeconds < dwDelayMilliSeconds)
			{
				// check transmit complete bit
				//if ( HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & BM_UARTAPPSTAT_TXFE && 
				//	((HW_UARTAPPLINECTRL_RD( pHWHead->dwIndex) && BM_UARTAPPLINECTRL_FEN) == 0) )
				if ( HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & BM_UARTAPPSTAT_TXFE )
				{
					break;
				}

				// check timeout
				dwCurrCount = GetTickCount();
				if(dwCurrCount >= dwStartCount)
				{
					dwElipsedMilliSeconds = dwCurrCount - dwStartCount;
				}
				else
				{
					dwElipsedMilliSeconds = ~dwStartCount + dwCurrCount + 1;
				}
			}

			// clear GPIO based RTSn if required
			BSPUartClearGPIORTS(pSerHead->dwRtsGpioPin);
		}*/

		//Clear RTS to logic "0":high;
        HW_UARTAPPCTRL2_CLR(pHWHead->dwIndex, BM_UARTAPPCTRL2_RTS);

        // If RTS hardware handshaking is enabled and the MDD is 
        // requesting that we flow off, we override the RTS setting
        // controlled by the receiver to avoid overflowing the RX buffer
        if (pHWHead->dcb.fRtsControl == RTS_CONTROL_HANDSHAKE)
        {                
            // set RTS to be controlled by MDD software
            HW_UARTAPPCTRL2_CLR(pHWHead->dwIndex, BM_UARTAPPCTRL2_RTSEN);
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
	PSER_INFO pSerHead = (PSER_INFO) pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SL_SetRTS+\r\n")));

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        // If RTS hardware handshaking is enabled and the MDD is 
        // requesting that we flow on, we give control of the RTS setting
        // back to the the receiver to avoid overflowing the RX FIFO
        if (pHWHead->dcb.fRtsControl == RTS_CONTROL_HANDSHAKE)
        {                
            // set RTS to be controlled by receiver
            HW_UARTAPPCTRL2_SET(pHWHead->dwIndex, BM_UARTAPPCTRL2_RTSEN);
        }

        //Set RTS to logic "1":low;
        HW_UARTAPPCTRL2_SET(pHWHead->dwIndex, BM_UARTAPPCTRL2_RTS);

		// LQK NOV-2-2012: set GPIO based RTSn if required
		if (pHWHead->dcb.fRtsControl == RTS_CONTROL_TOGGLE)
		{
			BSPUartSetGPIORTS(pSerHead->dwRtsGpioPin );
		}

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
        HW_UARTAPPLINECTRL_CLR(pHWHead->dwIndex, BM_UARTAPPLINECTRL_BRK);

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
        HW_UARTAPPLINECTRL_SET(pHWHead->dwIndex, BM_UARTAPPLINECTRL_BRK);

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
            if(HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & BM_UARTAPPSTAT_TXFE)
            {
                EnterCriticalSection(&(pHWHead->RegCritSec));
                // FIFO is empty, send this character
                HW_UARTAPPDATA_WR(pHWHead->dwIndex, ComChar);
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
            HW_UARTAPPINTR_SET(pHWHead->dwIndex, BM_UARTAPPINTR_TXIEN);

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

    if ( HW_UARTAPPSTAT_RD(pHWHead->dwIndex) & BM_UARTAPPSTAT_CTS) 
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("MS_CTS_ON\r\n")));
        *pModemStatus |= MS_CTS_ON;
        pHWHead->ModemStatus |= MS_CTS_ON;
    }

    // Application UART does not support DTR\DSR, and the following is done to cater for Serial Communications CETK.
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
	PSER_INFO pSerHead = (PSER_INFO)pHead;	// LQK NOV-2-2012:  supporting RTS_CONTROL_TOGGLE
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
                HW_UARTAPPCTRL2_SET(pHWHead->dwIndex, BM_UARTAPPCTRL2_CTSEN);
            }
            else
            {
                HW_UARTAPPCTRL2_CLR(pHWHead->dwIndex, BM_UARTAPPCTRL2_CTSEN);
            }
        }
        if (bRet && (lpDCB->fRtsControl != pHWHead->dcb.fRtsControl))
        {
            if (lpDCB->fRtsControl == RTS_CONTROL_HANDSHAKE)
            {
				// CS&ZHL JUN-14-2012: config RTS/CTS pins if required
				BSPUartConfigureHandshake(pHWHead->HwAddr);

                HW_UARTAPPCTRL2_SET(pHWHead->dwIndex, BM_UARTAPPCTRL2_RTSEN);
			}
            else
            {
                HW_UARTAPPCTRL2_CLR(pHWHead->dwIndex, BM_UARTAPPCTRL2_RTSEN);
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

		//
		// LQK NOV-2-2012: set GPIO to RTSn if required
		//
		if (pHWHead->dcb.fRtsControl == RTS_CONTROL_TOGGLE  )
		{
			if( !BSPUartConfigureRTS( pSerHead->dwRtsGpioPin ) )
				pHWHead->dcb.fRtsControl = RTS_CONTROL_DISABLE;
		}

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
	PSER_INFO	pSerHead = (PSER_INFO)pContext;

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

		//LQK NOV-2-2012: support GPIO as RTS direction-control if(dcb.fRtsControl == RTS_CONTROL_TOGGLE)
	case IOCTL_SET_UART_RTS_PIN:
		if( pInBuf != NULL && InBufLen == sizeof(DWORD))
		{
			pSerHead->dwRtsGpioPin = BSPUartGPIO2RTS( *((DWORD*)pInBuf ));
			if( pSerHead->dwRtsGpioPin != DDK_IOMUX_INVALID_PIN )
			{
				dwErr = ERROR_SUCCESS;
				RETAILMSG(1, (TEXT("SL_Ioctl: GPIO%d is used as RTS of UART%d\r\n"),
					(31 - _CountLeadingZeros(pSerHead->dwRtsGpioPin)), pHWHead->dwIndex));
			}
			else
				RETAILMSG(1, (TEXT("SL_Ioctl::IOCTL_SET_UART_RTS_PIN: GPIO%d is NOT supported as RTS!\r\n"), 
				(31 - _CountLeadingZeros(pSerHead->dwRtsGpioPin))));
		}				
		else 	RETAILMSG(1, (TEXT("SL_Ioctl::IOCTL_SET_UART_RTS_PIN: inavlid parameters\r\n")));
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

