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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

isa_16550.c

Abstract:  

    This file implements the standard device specific functions for a 16550
    based serial device on EM9X60's ISA bus.

Functions:

    SL4_Init()
    SL4_PostInit()
    SL4_Deinit()
    SL4_Open()
    SL4_Close()
    SL4_ClearDTR()
    SL4_SetDTR()
    SL4_ClearRTS()
    SL4_SetRTS()
    SL4_ClearBreak()
    SL4_SetBreak()
    SL4_SetBaudRate()
    SL4_SetByteSize()
    SL4_SetParity()
    SL4_SetStopBits()
    SL4_GetRxBufferSize()
    SL4_GetRxStart()
    SL4_GetInterruptType()
    SL4_RxIntr()
    SL4_PutBytes()
    SL4_TxIntr()
    SL4_LineIntr()
    SL4_OtherIntr()
    SL4_GetStatus()
    SL4_Reset()
    SL4_GetModemStatus()
    SL4_PurgeComm()
    SL4_XmitComChar()
    SL4_PowerOff()
    SL4_PowerOn()
    SL4_SetDCB()
    SL4_SetCommTimeouts()
    SL4_Ioctl()
    ReadLSR()
    ReadMSR()
    DumpSerialRegisters()
    LookUpBaudTableValue()
    DivisorOfBaudRate()

Notes:
    The RegCritSec is there to protect against non-atomic access of
    register pairs.  On a 16550, the main such collision comes from 
    the fact that THR and IER are overloaded as the DivLatch registers
    and their mode is controlled via the LCR.  So we need the 
    critical section protection around all access of these 3 registers.
    But we also need to watch out for read/modify/write where we are
    setting/ckearing a bit.  In general, I just go ahead and acquire
    the CS around all register accesses.
--*/

#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <windev.h>
#include <types.h>
#include <memory.h>
#include <nkintr.h>
#include <serhw.h>
#include <notify.h>
#include <devload.h>
#include <ser16550.h>
#include <hw16550.h>

// Controller includes
#include "bsp.h"

// it is important to include head files "*.h" as above oder! 
#include "isa_com.h"
#include "isa_16550.h"		

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

// Macros to read/write serial registers.
//#define INB(pInfo, reg) (READ_PORT_UCHAR((UCHAR *)((pInfo)->reg)))
//#define OUTB(pInfo, reg, value) (WRITE_PORT_UCHAR((UCHAR *)((pInfo)->reg), (unsigned char)(value)))

//#define INB(pInfo, reg)             (*((volatile UCHAR *)((pInfo)->reg)))
//#define OUTB(pInfo, reg, value)     (*(volatile UCHAR *)((pInfo)->reg) = (UCHAR)(value))
// CS&ZHL SEP-3-2011: use CSPDDK's macro
#define INB(pInfo, reg)						(INREG8(pInfo->reg))
#define OUTB(pInfo, reg, value)			(OUTREG8(pInfo->reg, value))

BOOL SL4_SetByteSize(PVOID pHead, ULONG ByteSize);
BOOL SL4_SetStopBits(PVOID pHead, ULONG StopBits);
BOOL SL4_SetParity(PVOID pHead, ULONG Parity);

#define EXCEPTION_ACCESS_VIOLATION STATUS_ACCESS_VIOLATION 

//
// Reading the LSR clears most of its bits.  So, we provide this wrapper,
// which reads the register, records any interesting values, and
// stores the current LSR contents in the shadow register.
//
__inline VOID ProcessLSR (PSER16550_INFO pHWHead)
{
    ULONG LineEvents = 0;
    if ( pHWHead->LSR & (SERIAL_LSR_OE | SERIAL_LSR_PE | SERIAL_LSR_FE)) 
	{
        // Note: Its not wise to do debug msgs in here since they will
        // pretty much guarantee that the FIFO gets overrun.
        if ( pHWHead->LSR & SERIAL_LSR_OE ) 
		{
            // DEBUGMSG (ZONE_WARN, (TEXT("Overrun\r\n")));
            pHWHead->DroppedBytes++;
            pHWHead->CommErrors |= CE_OVERRUN;
        }

        if ( pHWHead->LSR & SERIAL_LSR_PE ) 
		{
            // DEBUGMSG (ZONE_WARN, (TEXT("parity\r\n")));
            pHWHead->CommErrors |= CE_RXPARITY;
        }

        if ( pHWHead->LSR & SERIAL_LSR_FE ) 
		{
            // DEBUGMSG (ZONE_WARN, (TEXT("frame\r\n")));
            pHWHead->CommErrors |= CE_FRAME;
        }

        LineEvents |= EV_ERR;
    }

    if ( pHWHead->LSR & SERIAL_LSR_BI )
        LineEvents |= EV_BREAK;

    // Let WaitCommEvent know about this error
    if ( LineEvents )
        pHWHead->EventCallback( pHWHead->pMddHead, LineEvents );
}

__inline VOID ReadLSR( PSER16550_INFO  pHWHead )
{
    try {
        pHWHead->LSR = INB(pHWHead, pLSR);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
        pHWHead->LSR = SERIAL_LSR_THRE;
    }
	ProcessLSR (pHWHead);
}

//
// Reading the MSR clears many of its bits.  So, we provide this wrapper,
// which reads the register, records any interesting values, and
// stores the current MSR contents in the shadow register.
// Note that we always have DDCD and DCTS enabled, so if someone
// wants to keep an eye on these lines, its OK to simply read the
// shadow register, since if the value changes, the interrupt
// will cause the shadow to be updated.
//
__inline VOID ProcessMSR (PSER16550_INFO  pHWHead)
{
    ULONG        Events = 0;
    // For changes, we use callback to evaluate the event
    if (pHWHead->MSR  & SERIAL_MSR_DCTS)
        Events |= EV_CTS;

    if ( pHWHead->MSR  & SERIAL_MSR_DDSR )
        Events |= EV_DSR;

    if ( pHWHead->MSR  & SERIAL_MSR_TERI )
        Events |= EV_RING;

    if ( pHWHead->MSR  & SERIAL_MSR_DDCD )
        Events |= EV_RLSD;

    if ( Events )
        pHWHead->EventCallback( pHWHead->pMddHead, Events );
}

__inline VOID ReadMSR( PSER16550_INFO  pHWHead )
{
    UCHAR       msr;

    try {
        msr = INB(pHWHead, pMSR);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        msr = 0;
    }

    // Save the MSR value in a shadow
    pHWHead->MSR = msr;
    ProcessMSR (pHWHead);
}

#ifdef DEBUG
//
// This routine is used only for debugging, and performs a formatted
// ascii dump of the various UART registers.
//
VOID DumpSerialRegisters( PVOID  pHead )
{
    UINT8 byte;
    //PSER16550_INFO   pHWHead   = (PSER16550_INFO)pHead;
    PSER16550_INFO  pHWHead = &(((PCOM_INFO)pHead)->ser16550);

#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        ReadLSR( pHWHead );
        byte = pHWHead->LSR;

        NKDbgPrintfW(TEXT("16550 lsr: \t%2.2X\t"), byte);
        if ( byte & SERIAL_LSR_DR )
            NKDbgPrintfW(TEXT("DataReady "));
        if ( byte & SERIAL_LSR_OE )
            NKDbgPrintfW(TEXT("OverRun "));
        if ( byte & SERIAL_LSR_PE )
            NKDbgPrintfW(TEXT("ParityErr "));
        if ( byte & SERIAL_LSR_FE )
            NKDbgPrintfW(TEXT("FramingErr "));
        if ( byte & SERIAL_LSR_BI )
            NKDbgPrintfW(TEXT("BreakIntpt "));
        if ( byte & SERIAL_LSR_THRE )
            NKDbgPrintfW(TEXT("THREmpty "));
        if ( byte & SERIAL_LSR_TEMT )
            NKDbgPrintfW(TEXT("TXEmpty "));
        if ( byte & SERIAL_LSR_FIFOERR )
            NKDbgPrintfW(TEXT("FIFOErr "));
        NKDbgPrintfW(TEXT("\r\n"));

        byte = INB(pHWHead, pData);
        NKDbgPrintfW(TEXT("16550 rbr/thr:\t%2.2X\r\n"), byte);

        byte = INB(pHWHead, pIER);
        NKDbgPrintfW(TEXT("16550 IER: \t%2.2X\t"), byte);
        if ( byte & SERIAL_IER_RDA )
            NKDbgPrintfW(TEXT("RXData "));
        if ( byte & SERIAL_IER_THR )
            NKDbgPrintfW(TEXT("TXData "));
        if ( byte & SERIAL_IER_RLS )
            NKDbgPrintfW(TEXT("RXErr "));
        if ( byte & SERIAL_IER_MS )
            NKDbgPrintfW(TEXT("ModemStatus "));
        NKDbgPrintfW(TEXT("\r\n"));

        byte = INB(pHWHead, pIIR_FCR);
        NKDbgPrintfW(TEXT("16550 iir: \t%2.2X\t"), byte);
        if ( byte & SERIAL_IIR_RDA )
            NKDbgPrintfW(TEXT("RXData "));
        if ( byte & SERIAL_IIR_THRE )
            NKDbgPrintfW(TEXT("TXData "));
        if ( byte & SERIAL_IIR_RLS )
            NKDbgPrintfW(TEXT("RXErr "));
        if ( (byte & SERIAL_IIR_CTI) == SERIAL_IIR_CTI )
            NKDbgPrintfW(TEXT("CTI "));
        if ( byte == SERIAL_IIR_MS )
            NKDbgPrintfW(TEXT("ModemStatus "));
        if ( byte & 0x01 )
            NKDbgPrintfW(TEXT("IntPending "));
        NKDbgPrintfW(TEXT("\r\n"));

        byte = INB(pHWHead, pLCR);
        NKDbgPrintfW(TEXT("16550 lcr: \t%2.2X\t"), byte);

        NKDbgPrintfW(TEXT("%dBPC "), ((byte & 0x03)+5) );

        if ( byte & SERIAL_LCR_DLAB )
            NKDbgPrintfW(TEXT("DLAB "));
        if ( byte & SERIAL_LCR_DLAB )
            NKDbgPrintfW(TEXT("Break "));
        NKDbgPrintfW(TEXT("\r\n"));

        byte = INB(pHWHead, pMCR);
        NKDbgPrintfW(TEXT("16550 mcr: \t%2.2X\t"), byte);
        if ( byte & SERIAL_MCR_DTR )
            NKDbgPrintfW(TEXT("DTR "));
        if ( byte & SERIAL_MCR_RTS )
            NKDbgPrintfW(TEXT("RTS "));
        if ( byte & SERIAL_MCR_OUT1 )
            NKDbgPrintfW(TEXT("OUT1 "));
        if ( byte & SERIAL_MCR_OUT2 )
            NKDbgPrintfW(TEXT("OUT2 "));
        if ( byte & SERIAL_MCR_LOOP )
            NKDbgPrintfW(TEXT("LOOP "));
        NKDbgPrintfW(TEXT("\r\n"));

        ReadMSR( pHWHead );
        byte = pHWHead->MSR;
        NKDbgPrintfW(TEXT("16550 msr: \t%2.2X\t"), byte);
        if ( byte & SERIAL_MSR_DCTS )
            NKDbgPrintfW(TEXT("DCTS "));
        if ( byte & SERIAL_MSR_DDSR )
            NKDbgPrintfW(TEXT("DDSR "));
        if ( byte & SERIAL_MSR_TERI )
            NKDbgPrintfW(TEXT("TERI "));
        if ( byte & SERIAL_MSR_DDCD )
            NKDbgPrintfW(TEXT("DDCD"));
        if ( byte & SERIAL_MSR_CTS )
            NKDbgPrintfW(TEXT(" CTS"));
        if ( byte & SERIAL_MSR_DSR )
            NKDbgPrintfW(TEXT("DSR "));
        if ( byte & SERIAL_MSR_RI )
            NKDbgPrintfW(TEXT("RI "));
        if ( byte & SERIAL_MSR_DCD )
            NKDbgPrintfW(TEXT("DCD "));
        NKDbgPrintfW(TEXT("\r\n"));

    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
        // Nothing much to clean up here.
    }
#pragma prefast(pop)
}
#endif // DEBUG


//
// Helper routine to search through a lookup table for a designated
// key.
//
ULONG LookUpBaudTableValue( ULONG Key, PLOOKUP_TBL pTbl, PULONG pErrorCode )
{
    ULONG   index = 0;

    *pErrorCode = 0;

    while ( index < pTbl->Size ) {
        if ( Key == pTbl->Table[index].Key )
            return(pTbl->Table[index].AssociatedValue);

        ++index;
    }

    *pErrorCode = (ULONG)-1;
    return(0);
}

#define BAUD_TABLE_SIZE 23
static const PAIRS LS_BaudPairs[BAUD_TABLE_SIZE] =    
{
    {50,        2307},
    {75,        1538},
    {110,       1049},
    {135,        858},
    {150,        769},
    {300,        384},
    {600,        192},
    {1200,        96},
    {1800,        64},
    {2000,        58},
    {2400,        48},
    {3600,        32},
    {4800,        24},
    {7200,        16},
    {9600,        12},
    {12800,        9},
    {14400,        8},
    {19200,        6},
    {23040,        5},
    {28800,        4},
    {38400,        3},
    {57600,        2},
    {115200,       1}
};

static const LOOKUP_TBL  LS_BaudTable = {BAUD_TABLE_SIZE, (PAIRS *) LS_BaudPairs};

//
// Helper function.  Pass in a baudrate, and the corresponding divisor
// (from the baudtable) is returned.  If no matching baudrate is found
// in baudtable, then return 0.
//
// @parm PVOID returned by HWinit.
// @parm     ULONG representing decimal baud rate. 
USHORT DivisorOfBaudRate( PVOID pHead, ULONG BaudRate )
{
    ULONG   errorcode = 0;
    USHORT  divisor;    
    PSER16550_INFO  pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    divisor = (USHORT)LookUpBaudTableValue(BaudRate, pHWHead->pBaudTable, &errorcode);

    if ( errorcode )
        divisor = 0;

    return(divisor);
}

//
// This is a reverse lookup table which can be used to determine
// the FIFO trigger level from the 2 bit value stored in the FCR
//
#define HIGH_WATER_SIZE     4
static const PAIRS HighWaterPairs[HIGH_WATER_SIZE] = 
{
    {SERIAL_1_BYTE_HIGH_WATER, 0},
    {SERIAL_4_BYTE_HIGH_WATER, 4},
    {SERIAL_8_BYTE_HIGH_WATER, 8},
    {SERIAL_14_BYTE_HIGH_WATER, 14}
};

static const LOOKUP_TBL  HighWaterTable = {HIGH_WATER_SIZE, (PAIRS *) HighWaterPairs};

#define IER_NORMAL_INTS (SERIAL_IER_RDA | SERIAL_IER_RLS | SERIAL_IER_MS)

// Routine to clear any pending interrupts.  Called from Init and PostInit
// to make sure we atart out in a known state.
VOID ExClearPendingInts( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    //PSER16550_INFO pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO  pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    EnterCriticalSection(&(pHWHead->RegCritSec));

    try {
        pHWHead->IIR = INB(pHWHead, pIIR_FCR); 
        while ( ! (pHWHead->IIR & 0x01) ) 
		{
            DEBUGMSG (ZONE_INIT, (TEXT("!!IIR %X\r\n"), pHWHead->IIR));
            // Reading LSR clears RLS interrupts.
            ReadLSR( pHWHead );

            // Reset RX FIFO to clear any old data remaining in it.
            OUTB(pHWHead, pIIR_FCR, pHWHead->FCR | SERIAL_FCR_RCVR_RESET);

            // Reading MSR clears Modem Status interrupt
            ReadMSR( pHWHead );

            // Simply reading IIR is sufficient to clear THRE
            pHWHead->IIR = INB(pHWHead, pIIR_FCR);
        }    
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
        DEBUGMSG (ZONE_ERROR,(TEXT("-SL4_PostInit, 0x%X - ERROR\r\n"), pHWHead));
        // Just fall through & release CritSec
    }

    LeaveCriticalSection(&(pHWHead->RegCritSec));
}

//
/////////////////// Start of exported entrypoints ////////////////
//
#define WATERMAKER_ENTRY 2
//
// @doc OEM 
// @func PVOID | SL4_Open | Configures 16550 for default behaviour.
//
VOID SL4_Open( PVOID pHead )	 // @parm PVOID returned by HWinit.
{
    //PCOM_INFO		pSerHead = (PCOM_INFO)pHead;   
    PSER16550_INFO  pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_OPEN, (TEXT("+SL4_Open 0x%X\r\n"), pHead));

    // If the device is already open, all we do is increment count
    if ( pHWHead->OpenCount++ ) 
	{
        DEBUGMSG (ZONE_OPEN, (TEXT("-SL4_Open 0x%X (%d opens)\r\n"), pHead, pHWHead->OpenCount));
        return ;
    }

    pHWHead->FCR = 0;
    pHWHead->IER = 0;
    pHWHead->IIR = 0;
    pHWHead->LSR = 0;
    pHWHead->MSR = 0;
    pHWHead->DroppedBytes = 0;
    pHWHead->CTSFlowOff = FALSE;  // Not flowed off yet
    pHWHead->DSRFlowOff = FALSE;  // Not flowed off yet
    pHWHead->CommErrors   = 0;
    pHWHead->ModemStatus  = 0;

    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
		//
		// CS&ZHL OCT-23-2008: enable ETA503 interrupt if necessary
		//
		//OUTB(pHWHead, pScratch, EM9XXX_SHARE_IRQ_ENABLE);
        //OUTB(pHWHead, pIER, (UCHAR)IER_NORMAL_INTS);

        OUTB(pHWHead, pMCR, SERIAL_MCR_IRQ_ENABLE);

        // Set default framing bits.
        OUTB(pHWHead, pLCR, SERIAL_8_DATA | SERIAL_1_STOP | SERIAL_NONE_PARITY);

        DEBUGMSG (ZONE_OPEN,
                  (TEXT("SL4_Open Setting DCB parameters\r\n")));

        // Get defaults from the DCB structure
        SL4_SetBaudRate( pHead, pHWHead->dcb.BaudRate );
        SL4_SetByteSize( pHead, pHWHead->dcb.ByteSize );
        SL4_SetStopBits( pHead, pHWHead->dcb.StopBits );
        SL4_SetParity( pHead, pHWHead->dcb.Parity );

        //
        // A 16450 (which is pretty much a FIFO-less 16550) can be supported by
        // not initializing the FIFO.
        //
        if (pHWHead->ChipID == CHIP_ID_16550) 
		{
            // Set up to use 16550 fifo for 14 byte interrupt granularity.
            // Shadow the FCR bitmask since reading this location is the IIR.
            pHWHead->FCR = SERIAL_FCR_ENABLE | (BYTE)HighWaterPairs[WATERMAKER_ENTRY].Key;
            OUTB(pHWHead, pIIR_FCR, (pHWHead->FCR | SERIAL_FCR_RCVR_RESET | SERIAL_FCR_TXMT_RESET) );
        }

        // For CE 3.0, we are still supporting
        // the old style MDDs, and they don't call our PostInit, which
        // needs to happen sometime prior to this.  So for now, we go ahead
        // ahead and clear out interrupts one last time.  In 4.0, we can
        // kill the old serial MDD and assume that everyone uses the new
        // MDD and calls post init.  
        SL4_PostInit( pHead );			// clear interrupt flag in UART hardware register

		//
		// CS&ZHL FEB-22-2010: enable ETA503 interrupt if necessary
		//
		OUTB(pHWHead, pScratch, EM9XXX_SHARE_IRQ_ENABLE);
        OUTB(pHWHead, pIER, (UCHAR)IER_NORMAL_INTS);
 
        ReadMSR(pHWHead);
        ReadLSR(pHWHead);

#ifdef DEBUG
        if ( ZONE_INIT )
            DumpSerialRegisters(pHead);
#endif
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just get out of here.
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

	DEBUGMSG (ZONE_OPEN, (TEXT("-SL4_Open::COM%d, IIR 0x%X\r\n"), pSerHead->dwDeviceArrayIndex, pHWHead->IIR));
    //RETAILMSG(1, (TEXT("-SL4_Open::COM%d, IIR 0x%X\r\n"), pSerHead->dwDeviceArrayIndex, pHWHead->IIR));
}

//
// @doc OEM 
// @func PVOID | SL4_Close | Does nothing except keep track of the
// open count so that other routines know what to do.
//
VOID SL4_Close( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    //PSER16550_INFO pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO	pHWHead = &(((PCOM_INFO)pHead)->ser16550);
    ULONG			uTries = 0;
	UCHAR			ub1;

    DEBUGMSG (ZONE_CLOSE, (TEXT("+SL4_Close 0x%X\r\n"), pHead));

    if ( pHWHead->OpenCount )
        pHWHead->OpenCount--;

    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        // Disable all interrupts and clear MCR.
        OUTB(pHWHead, pIER, (UCHAR)0); 
        OUTB(pHWHead, pMCR, (UCHAR)0);

        pHWHead->IIR = INB(pHWHead, pIIR_FCR);        

		//
		// CS&ZHL OCT-23-2008: disable ETA503 interrupt if necessary
		//
		OUTB(pHWHead, pScratch, EM9XXX_SHARE_IRQ_DISABLE);
		
		//
		// CS&ZHL JAN-23-2009: while we are still transmitting, sleep.
		//
		ub1 = INB(pHWHead, pLSR) & (SERIAL_LSR_TEMT | SERIAL_LSR_THRE);
		while ( (ub1 != (SERIAL_LSR_TEMT | SERIAL_LSR_THRE))				// indicates FIFO not yet empty
              && (uTries++ < 100))											// safety net
		{
			DEBUGMSG ( ZONE_CLOSE, (TEXT("SerClose, TX in progress, LSR 0x%X\r\n"),
					   ub1));
			Sleep(10);
			ub1 = INB(pHWHead, pLSR) & (SERIAL_LSR_TEMT | SERIAL_LSR_THRE);
		}
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
        // Just get out of here.
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_CLOSE, (TEXT("-SL4_Close 0x%X\r\n"), pHead));
}

//
// @doc OEM 
// @func PVOID | SL4_Init | Initializes 16550 device head.  
//
BOOL
SL4_Init(
       PVOID   pHead,				// @parm points to device head
       PUCHAR  pRegBase,			// Pointer to 16550 register base
       UINT8   RegStride,			// Stride amongst the 16550 registers = 1 for EM9161 
       EVENT_FUNC EventCallback,	// This callback exists in MDD
       PVOID   pMddHead,			// This is the first parm to callback
       PLOOKUP_TBL   pBaudTable		// BaudRate Table
       )
{
    PCOM_INFO				pSerHead = (PCOM_INFO)pHead;
    PSER16550_INFO		pHWHead  = &(pSerHead->ser16550);
	UCHAR						ub1;

    DEBUGMSG (ZONE_CLOSE,(TEXT("+SL4_INIT, 0x%X\r\n"), pHead));

    // Set up pointers to 16550 registers
    pHWHead->pData    = pRegBase + (RegStride * RECEIVE_BUFFER_REGISTER);
    pHWHead->pIER     = pRegBase + (RegStride * INTERRUPT_ENABLE_REGISTER);
    pHWHead->pIIR_FCR = pRegBase + (RegStride * INTERRUPT_IDENT_REGISTER);
    pHWHead->pLCR     = pRegBase + (RegStride * LINE_CONTROL_REGISTER);
    pHWHead->pMCR     = pRegBase + (RegStride * MODEM_CONTROL_REGISTER);
    pHWHead->pLSR     = pRegBase + (RegStride * LINE_STATUS_REGISTER);
    pHWHead->pMSR     = pRegBase + (RegStride * MODEM_STATUS_REGISTER);
    pHWHead->pScratch = pRegBase + (RegStride * SCRATCH_REGISTER);
    //RETAILMSG (1, (TEXT("+SL4_INIT, LSR = 0x%x\r\n"), (DWORD)(pHWHead->pLSR)));

    // Store info for callback function
    pHWHead->EventCallback = EventCallback;
    pHWHead->pMddHead = pMddHead;

    // Now set up remaining fields
    if ( pBaudTable != NULL )
        pHWHead->pBaudTable = (LOOKUP_TBL *) pBaudTable;
    else
        pHWHead->pBaudTable = (LOOKUP_TBL *) &LS_BaudTable;

	//
	// CS&ZHL SEP-2-2011: enable ISA bus
	//
	{
	    PHYSICAL_ADDRESS phyAddr;
		PEM9K_CPLD_REGS	pCPLD;

		phyAddr.LowPart = CSP_BASE_MEM_PA_CS5;
		phyAddr.HighPart = 0;
		pCPLD = (PEM9K_CPLD_REGS)MmMapIoSpace(phyAddr, sizeof(EM9K_CPLD_REGS), FALSE);	
		if(pCPLD == NULL)
		{
			RETAILMSG(1, (TEXT("SL_Init::CPLD map failed\r\n")));
			return FALSE;
		}

		OUTREG8 (&pCPLD->ISACtrlReg, EM9K_CPLD_ISACTRL_ISAEN);				// enable ISA with CS1 + (A0 - A4)

		MmUnmapIoSpace(pCPLD, sizeof(EM9K_CPLD_REGS)); 
	}

	// Don't allow any interrupts till PostInit.
    OUTB(pHWHead, pIER, (UCHAR)0);

	//
	// CS&ZHL JAN-23-2009: disable shared interrupt with MSR writing
	//
    OUTB(pHWHead, pScratch, (UCHAR)EM9XXX_SHARE_IRQ_DISABLE);			

	//
	// CS&ZHL JLY-18-2008: check if 16C550 existed on ISA bus
	//
    OUTB(pHWHead, pLCR, 0x55);
    OUTB(pHWHead, pScratch, (UCHAR)EM9XXX_SHARE_IRQ_DISABLE);			// discharge data bus
    ub1 = INB(pHWHead, pLCR);
    OUTB(pHWHead, pScratch, (UCHAR)EM9XXX_SHARE_IRQ_DISABLE);			// discharge data bus
    ub1 = INB(pHWHead, pLCR);
	if( ub1 != 0x55 )
	{
		//
		// CS&ZHL MAY-18-2009: dump for debug
		//
		RETAILMSG(1, (TEXT("+SL4_INIT::pLCR(0x%08x) = 0x%02x\r\n"), pHWHead->pLCR, ub1));

		return(FALSE);
	}
    OUTB(pHWHead, pLCR, 0xaa);
    OUTB(pHWHead, pScratch, (UCHAR)EM9XXX_SHARE_IRQ_DISABLE);			// discharge data bus
    ub1 = INB(pHWHead, pLCR);
    OUTB(pHWHead, pScratch, (UCHAR)EM9XXX_SHARE_IRQ_DISABLE);			// discharge data bus
    ub1 = INB(pHWHead, pLCR);
	if( ub1 != 0xaa )
	{
		//
		// CS&ZHL MAY-18-2009: dump for debug
		//
		RETAILMSG(1, (TEXT("+SL4_INIT::pLCR(0x%08x) = 0x%x\r\n"), pHWHead->pScratch, ub1));

		return(FALSE);
	}

    pHWHead->FlushDone = CreateEvent(0, FALSE, FALSE, NULL);
    pHWHead->OpenCount = 0;

    InitializeCriticalSection(&(pHWHead->TransmitCritSec));
    InitializeCriticalSection(&(pHWHead->RegCritSec));
    pHWHead->PowerDown = FALSE;
    pHWHead->bSuspendResume = FALSE;

	//Software Interrupt is not supported here
    //InstallSoftwareISR(pHWHead,pHWHead->pVirtualStaticAddr,RegStride);
    
	// Clear any interrupts which may be pending.  Normally only
    // happens if we were warm reset.
    ExClearPendingInts( pHead );

	//
	// CS&ZHL SEP-2-2011:setup ISA_IRQ1
	//
	{
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_F,						// pin name
										DDK_IOMUX_PIN_MUXMODE_ALT0,			// select GPIO1[5] 
										DDK_IOMUX_PIN_SION_REGULAR);			// no SION option on this pin

		DDKIomuxSetPadConfig(DDK_IOMUX_PAD_GPIO_F,						
										 DDK_IOMUX_PAD_SLEW_SLOW,				// -> the same electrical config as PDK1_7
										 DDK_IOMUX_PAD_DRIVE_HIGH,
										 DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
										 DDK_IOMUX_PAD_PULL_DOWN_100K,
										 DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
										 DDK_IOMUX_PAD_VOLTAGE_3V3);
		
		// try to use GPIO interrupt: rise edge active
		DDKGpioSetConfig(DDK_GPIO_PORT1,						//gpio group number = DDK_GPIO_PORT1..DDK_GPIO_PORT4
									5,													//gpio number within a gpio group = 0..31
									DDK_GPIO_DIR_IN,						//gpio direction = DDK_GPIO_DIR_IN, DDK_GPIO_DIR_OUT
									DDK_GPIO_INTR_RISE_EDGE);		//gpio interrupt = level, edge,none, etc 
    
		DDKGpioClearIntrPin(DDK_GPIO_PORT1, 5);
	}

	DEBUGMSG (ZONE_CLOSE,(TEXT("-SL4_INIT, 0x%X\r\n"), pHead));
	return(TRUE);
}

//
// @doc OEM
// @func void | SL4_PostInit | This routine takes care of final initialization.
//
// @rdesc None.
//
BOOL SL4_PostInit( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    //PSER16550_INFO   pHWHead   = (PSER16550_INFO)pHead;
    //PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_INIT,(TEXT("+SL4_PostInit, 0x%X\r\n"), pHead));
    
    // Since we are just a library which might get used for 
    // builtin ports which init at boot, or by PCMCIA ports
    // which init at Open, we can't do anything too fancy.
    // Lets just make sure we cancel any pending interrupts so
    // that if we are being used with an edge triggered PIC, he
    // will see an edge after the MDD hooks the interrupt.
    ExClearPendingInts( pHead );
    
    DEBUGMSG (ZONE_INIT,(TEXT("-SL4_PostInit, 0x%X\r\n"), pHead));
    return(TRUE);
}

//
// @doc OEM 
// @func PVOID | SL4_Deinit | De-initializes 16550 device head.  
//
VOID SL4_Deinit( PVOID pHead )				// @parm points to device head
{
    //PSER16550_INFO   pHWHead   = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_CLOSE,(TEXT("+SL4_DEINIT, 0x%X\r\n"), pHWHead));

    //UninstallSoftwareISR( pHWHead);
    
	DeleteCriticalSection(&(pHWHead->TransmitCritSec));
    DeleteCriticalSection(&(pHWHead->RegCritSec));

    // Free the flushdone event
    if ( pHWHead->FlushDone )
        CloseHandle( pHWHead->FlushDone );

    DEBUGMSG (ZONE_CLOSE,(TEXT("-SL4_DEINIT, 0x%X\r\n"), pHWHead));
}

//
// @doc OEM
// @func void | SL4_ClearDtr | This routine clears DTR.
//
// @rdesc None.
//
VOID SL4_ClearDTR( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    //PSER16550_INFO   pHWHead   = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_ClearDTR, 0x%X\r\n"), pHead));
    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        unsigned char byte;

        byte = INB(pHWHead, pMCR);
        OUTB(pHWHead, pMCR, byte & ~SERIAL_MCR_DTR);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_ClearDTR, 0x%X\r\n"), pHead));
}

//
// @doc OEM
// @func VOID | SL4_SetDTR | This routine sets DTR.
// 
// @rdesc None.
//
VOID SL4_SetDTR( PVOID pHead )			// @parm PVOID returned by HWinit.
{    
    //PSER16550_INFO   pHWHead   = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_SetDTR, 0x%X\r\n"), pHead));
    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        unsigned char byte;

        byte = INB(pHWHead, pMCR);
        OUTB(pHWHead, pMCR, byte | SERIAL_MCR_DTR);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_SetDTR, 0x%X\r\n"), pHead));
}

//
// @doc OEM
// @func VOID | SL4_ClearRTS | This routine clears RTS.
// 
// @rdesc None.
// 
VOID SL4_ClearRTS( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    //PSER16550_INFO   pHWHead   = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_ClearRTS, 0x%X\r\n"), pHead));
    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        unsigned char	byte;
		int				i1;	
		DWORD			dwDelayMilliseconds;

		if(pHWHead->dcb.fRtsControl == RTS_CONTROL_TOGGLE)
		{
			//
			// CS&ZHL APR-06-2008: wait until transmitting completed -> (LSR.THRE== 1) && (LSR.TEMT == 1)
			//
			dwDelayMilliseconds = 9600 / pHWHead->dcb.BaudRate;
			if( dwDelayMilliseconds == 0 )
				dwDelayMilliseconds = 1;

			for(i1 = 0; i1 < 16; i1++)		// this loop may take a few milliseconds!!!
			{
				byte = INB(pHWHead, pLSR);
				if( !(byte & SERIAL_LSR_THRE) )
				{
					Sleep(dwDelayMilliseconds);
					continue;
				}
				// transmitting the last byte!
				while( !(byte & SERIAL_LSR_TEMT) )
				{
					byte = INB(pHWHead, pLSR);
					if( !(byte & SERIAL_LSR_THRE) )		// something wrong
						break;
				}
				break;
			}
		}

		byte = INB(pHWHead, pMCR);
        OUTB(pHWHead, pMCR, byte & ~SERIAL_MCR_RTS);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_ClearRTS, 0x%X\r\n"), pHead));
}

//
// @doc OEM
// @func VOID | SL4_SetRTS | This routine sets RTS.
// 
// @rdesc None.
//
VOID SL4_SetRTS( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    //PSER16550_INFO   pHWHead   = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_SetRTS, 0x%X\r\n"), pHead));

    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        unsigned char byte;

        byte = INB(pHWHead, pMCR);
        OUTB(pHWHead, pMCR, byte | SERIAL_MCR_RTS);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_SetRTS, 0x%X\r\n"), pHead));
}

//
// @doc OEM
// @func VOID | SL4_ClearBreak | This routine clears break.
// 
// @rdesc None.
// 
VOID SL4_ClearBreak( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    //PSER16550_INFO   pHWHead   = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_ClearBreak, 0x%X\r\n"), pHead));

    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        unsigned char byte;

        byte = INB(pHWHead, pLCR);
        OUTB(pHWHead, pLCR, byte & ~SERIAL_LCR_BREAK);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_ClearBreak, 0x%X\r\n"), pHead));
}

//
// @doc OEM
// @func VOID | SL4_SetBreak | This routine sets break.
// 
// @rdesc None.
//
VOID SL4_SetBreak( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    //PSER16550_INFO   pHWHead   = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_SetBreak, 0x%X\r\n"), pHead));

    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        unsigned char byte;

        byte = INB(pHWHead, pLCR);
        OUTB(pHWHead, pLCR, byte | SERIAL_LCR_BREAK);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_SetBreak, 0x%X\r\n"), pHead));
}

//
// SetUARTBaudRate
//
// Internal function.  The only real reason for splitting this out
// is so that we can call it from PowerOn and still allow SL4_SetBaud
// to do debug messages, acquire critical sections, etc.
//
// @parm     PVOID returned by HWInit
// @parm     ULONG representing decimal baud rate.
BOOL SetUARTBaudRate( PVOID pHead, ULONG BaudRate )
{
    PCOM_INFO      pSerHead = (PCOM_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);
    USHORT         divisor;
    UCHAR          lcr;

    // **** Warning ***** Make no system calls, called in power context
    divisor = DivisorOfBaudRate(pHead, BaudRate);

    if ( divisor ) 
	{
       InterruptMask(pSerHead->dwSysIntr,TRUE);
       lcr = INB(pHWHead, pLCR);
       OUTB(pHWHead, pLCR, lcr | SERIAL_LCR_DLAB);
       OUTB(pHWHead, pData, divisor & 0xff); //pData is DivLatch Lo
       OUTB(pHWHead, pIER, (divisor >> 8) & 0xff); //pIER is DivLatch Hi
       OUTB(pHWHead, pLCR, lcr);
       InterruptMask(pSerHead->dwSysIntr,FALSE);
       return( TRUE );
    } 
	else 
        return( FALSE );
}

//
// @doc OEM
// @func BOOL | SL4_SetBaudRate |
//  This routine sets the baud rate of the device.
//
// @rdesc None.
//
// @parm     PVOID returned by HWInit
// @parm     ULONG representing decimal baud rate.
BOOL SL4_SetBaudRate( PVOID pHead, ULONG BaudRate )   
{
    BOOL fRet;
    //PSER16550_INFO    pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+SL4_SetbaudRate 0x%X, x%X\r\n"), pHead, BaudRate));

    try {
        // Enter critical section before calling function, since
        // we can't make sys calls inside SetUARTBaudRate
        EnterCriticalSection(&(pHWHead->RegCritSec));
        fRet = SetUARTBaudRate(pHead, BaudRate);
        LeaveCriticalSection(&(pHWHead->RegCritSec));
    }except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        return( FALSE );
    }

    if ( fRet ) 
	{
        pHWHead->dcb.BaudRate = BaudRate;

        DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_SetbaudRate 0x%X (%d Baud)\r\n"), pHead, BaudRate));
        return( TRUE );
    } 
	else 
	{
        DEBUGMSG (ZONE_FUNCTION | ZONE_ERROR, 
			     (TEXT("-SL4_SetbaudRate - Error setting %d, failing to %d\r\n"),
                 BaudRate, pHWHead->dcb.BaudRate) );
        return( FALSE );
    }
}

//
// @doc OEM
// @func BOOL | SL4_SetByteSize |
//  This routine sets the WordSize of the device.
//
// @rdesc None.
//
// @parm     PVOID returned by HWInit
// @parm     ULONG ByteSize field from DCB.
BOOL SL4_SetByteSize( PVOID pHead, ULONG ByteSize )
{
    //PSER16550_INFO    pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);
    UINT8 lcr;
    BOOL bRet;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_SetByteSize 0x%X, x%X\r\n"), pHead, ByteSize));

    bRet = TRUE;

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        lcr = INB(pHWHead, pLCR);
        lcr &= ~SERIAL_DATA_MASK;
        switch ( ByteSize ) {
        case 5:
            lcr |= SERIAL_5_DATA;
            break;
        case 6:
            lcr |= SERIAL_6_DATA;
            break;
        case 7:
            lcr |= SERIAL_7_DATA;
            break;
        case 8:
            lcr |= SERIAL_8_DATA;
            break;
        default:
            bRet = FALSE;
            break;
        }
        if (bRet) {
            OUTB(pHWHead, pLCR, lcr);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_SetByteSize 0x%X\r\n"), pHead));

    return(bRet);
}

//
// @doc OEM
// @func BOOL | SL4_SetParity |
//  This routine sets the parity of the device.
//
// @rdesc None.
//
// @parm     PVOID returned by HWInit
// @parm     ULONG parity field from DCB.
BOOL SL4_SetParity( PVOID pHead, ULONG Parity )
{
    //PSER16550_INFO    pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);
    UINT8 lcr;
    BOOL bRet;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_SetParity 0x%X, x%X\r\n"), pHead, Parity));

    bRet = TRUE;

    EnterCriticalSection(&(pHWHead->RegCritSec));
    try {
        lcr = INB(pHWHead, pLCR);
        lcr &= ~SERIAL_PARITY_MASK;
        switch ( Parity ) {
        case ODDPARITY:
            lcr |= SERIAL_ODD_PARITY;
            break;

        case EVENPARITY:
            lcr |= SERIAL_EVEN_PARITY;
            break;

        case MARKPARITY:
            lcr |= SERIAL_MARK_PARITY;
            break;

        case SPACEPARITY:
            lcr |= SERIAL_SPACE_PARITY;
            break;

        case NOPARITY:
            lcr |= SERIAL_NONE_PARITY;
            break;
        default:
            bRet = FALSE;
            break;
        }
        if (bRet) {
            OUTB(pHWHead, pLCR, lcr);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
    }
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_SetParity 0x%X\r\n"), pHead));

    return(bRet);
}
//
// @doc OEM
// @func VOID | SL4_SetStopBits |
//  This routine sets the Stop Bits for the device.
//
// @rdesc None.
//
// @parm     PVOID returned by HWInit
// @parm     ULONG StopBits field from DCB.
BOOL SL4_SetStopBits( PVOID pHead, ULONG StopBits )
{
    //PSER16550_INFO    pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);
    UINT8 lcr;
    BOOL bRet;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_SetStopBits 0x%X, x%X\r\n"), pHead, StopBits));

    bRet = TRUE;

    EnterCriticalSection(&(pHWHead->RegCritSec));
    lcr = INB(pHWHead, pLCR);
    lcr &= ~SERIAL_STOP_MASK;

    try {
        // Note that 1.5 stop bits only works if the word size
        // is 5 bits.  Any other xmit word size will cause the
        // 1.5 stop bit setting to generate 2 stop bits.
        switch ( StopBits ) {
        case ONESTOPBIT :
            lcr |= SERIAL_1_STOP ;
            break;
        case ONE5STOPBITS :
            lcr |= SERIAL_1_5_STOP ;
            break;
        case TWOSTOPBITS :
            lcr |= SERIAL_2_STOP ;
            break;
        default:
            bRet = FALSE;
            break;
        }

        if (bRet) {
            OUTB(pHWHead, pLCR, lcr);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
    }

    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_SetStopBits 0x%X\r\n"), pHead));

    return(bRet);
}

//
// @doc OEM
// @func ULONG | SL4_GetRxBufferSize | This function returns
// the size of the hardware buffer passed to the interrupt
// initialize function.  It would be used only for devices
// which share a buffer between the MDD/PDD and an ISR.
//
// 
// @rdesc This routine always returns 0 for 16550 UARTS.
// 
ULONG SL4_GetRxBufferSize( PVOID pHead )
{
    UNREFERENCED_PARAMETER(pHead);

	return(0);
}

//
// @doc OEM
// @func PVOID | SC_GetRxStart | This routine returns the start of the hardware
// receive buffer.  See SL4_GetRxBufferSize.
// 
// @rdesc The return value is a pointer to the start of the device receive buffer.
// 
PVOID SL4_GetRxStart( PVOID pHead )			// @parm PVOID returned by SC_init.
{
    UNREFERENCED_PARAMETER(pHead);

	return(NULL);
}

//
// @doc OEM
// @func ULONG | SL4_GetGetInterruptType | This function is called
//   by the MDD whenever an interrupt occurs.  The return code
//   is then checked by the MDD to determine which of the four
//   interrupt handling routines are to be called.
// 
// @rdesc This routine returns a bitmask indicating which interrupts
//   are currently pending.
// 
INTERRUPT_TYPE SL4_GetInterruptType( PVOID pHead )			// Pointer to hardware head
{
    PCOM_INFO			pSerHead = (PCOM_INFO)pHead;
    PSER16550_INFO	pHWHead = &(pSerHead->ser16550);
    INTERRUPT_TYPE	interrupts=INTR_NONE;

    DEBUGMSG (0, (TEXT("+SL4_GetInterruptType 0x%X\r\n"), pHead));

    try {
        pHWHead->IIR = INB(pHWHead, pIIR_FCR);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
        pHWHead->IIR = SERIAL_IIR_INT_INVALID; // simulate no interrupt
    }
    DEBUGMSG (ZONE_THREAD, (TEXT("+SL4_GetInterruptType IIR=%x\r\n"),pHWHead->IIR ));

    if ( pHWHead->IIR & SERIAL_IIR_INT_INVALID ) 
	{
        // No interrupts pending, vector is useless
        interrupts = INTR_NONE;
		// CS&ZHL SEP-3-2011: clear GPIO interrupt flags
		DDKGpioClearIntrPin(DDK_GPIO_PORT1, 5);
    } 
	else 
	{
        // The interrupt value is valid
        switch ( pHWHead->IIR & SERIAL_IIR_INT_MASK ) 
		{
        case SERIAL_IIR_RLS:
            interrupts |= INTR_LINE;
            break;

        case SERIAL_IIR_CTI:
        case SERIAL_IIR_CTI_2:
        case SERIAL_IIR_RDA:
            interrupts = INTR_RX;
            break;

        case SERIAL_IIR_THRE :
            interrupts = INTR_TX;
            break;

        case SERIAL_IIR_MS :
            interrupts = INTR_MODEM;
            break;

        default:
            interrupts = INTR_NONE;
			// CS&ZHL SEP-3-2011: clear GPIO interrupt flags
			DDKGpioClearIntrPin(DDK_GPIO_PORT1, 5);
            break;
        }
    }

    if (pHWHead->AddTXIntr) {
        interrupts |= INTR_TX;
        pHWHead->AddTXIntr = FALSE;
    }
    DEBUGMSG (ZONE_THREAD, (TEXT("-SL4_GetInterruptType 0x%X, 0x%X\r\n"), pHead, interrupts));

    return(interrupts);
}


// @doc OEM
// @func ULONG | SL4_RxIntr | This routine gets several characters from the hardware
//   receive buffer and puts them in a buffer provided via the second argument.
//   It returns the number of bytes lost to overrun.
// 
// @rdesc The return value indicates the number of overruns detected.
//   The actual number of dropped characters may be higher.
//
// @parm Pointer to hardware head
// @parm Pointer to receive buffer
// @parm In = max bytes to read, out = bytes read
ULONG SL4_RxIntr( PVOID pHead, PUCHAR pRxBuffer, ULONG *pBufflen )
{
    //PSER16550_INFO   pHWHead    = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);
    ULONG          RetVal  = 0;
    ULONG          TargetRoom = *pBufflen;
    BOOL           fRXFlag = FALSE;
    BOOL           fReplaceparityErrors = FALSE;
    BOOL           fNull;
    UCHAR          cEvtChar, cRXChar;

    *pBufflen = 0;

    // LAM - I have local copies of some DCB elements since I don't
    // want to keep dereferencing inside my read loop and there are too
    // many of them to trust the compiler.
    cEvtChar = pHWHead->dcb.EvtChar;
    fNull = pHWHead->dcb.fNull;
    if ( pHWHead->dcb.fErrorChar && pHWHead->dcb.fParity )
        fReplaceparityErrors = TRUE;

#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        while ( TargetRoom ) 
		{
            // See if there is another byte to be read
            //ReadLSR( pHWHead );
			//
			// CS&ZHL JUN-25-2008: just read LSR to speed up
			//
			pHWHead->LSR = INB(pHWHead, pLSR);

            if ( pHWHead->LSR & SERIAL_LSR_DR ) 
			{
                // Read the byte
                cRXChar = INB(pHWHead, pData);
            }
            else
                break;

            // But we may want to discard it
            if ( pHWHead->dcb.fDsrSensitivity &&
                (! (pHWHead->MSR & SERIAL_MSR_DSR)) ) 
			{
                // Do nothing - byte gets discarded
                DEBUGMSG (ZONE_FLOW, (TEXT("Dropping byte because DSR is low\r\n")));
            } 
			else if (!cRXChar && fNull) 
			{
                // Do nothing - byte gets discarded
                DEBUGMSG (ZONE_FLOW| ZONE_WARN, (TEXT("Dropping NULL byte due to fNull\r\n")));
            } 
			else 
			{
                // Do character replacement if parity error detected.
                if ( fReplaceparityErrors && (pHWHead->LSR & SERIAL_LSR_PE) ) 
				{
                    cRXChar = pHWHead->dcb.ErrorChar;
                } 
				else 
				{
                    // See if we need to generate an EV_RXFLAG for the received char.
                    if ( cRXChar == cEvtChar )
                        fRXFlag = TRUE;
                }
                
                // Finally, we can get byte, update status and save.
                *pRxBuffer++ = cRXChar;
                (*pBufflen)++;
                --TargetRoom;
            }
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // just exit
    }
#pragma prefast(pop)

    // if we saw one (or more) EVT chars, then generate an event
    if ( fRXFlag )
        pHWHead->EventCallback( pHWHead->pMddHead, EV_RXFLAG );

    if ( pHWHead->DroppedBytes )
        DEBUGMSG (ZONE_WARN, (TEXT("Rx drop %d.\r\n"), pHWHead->DroppedBytes));

    DEBUGMSG (ZONE_READ, (TEXT("-GetBytes - rx'ed %d, dropped %d.\r\n"),
              *pBufflen, pHWHead->DroppedBytes));

    RetVal = pHWHead->DroppedBytes;
    pHWHead->DroppedBytes = 0;
    return(RetVal);
}

// @doc OEM
// @func ULONG | SL4_PutBytes | This routine is called from the MDD
//   in order to write a stream of data to the device. (Obselete)
// 
// @rdesc Always returns 0
//
// @parm    PVOID returned by HWInit.
// @parm    Pointer to bytes to be sent.
// @parm    Number of bytes to be sent.
// @parm    Pointer to actual number of bytes put.
ULONG SL4_PutBytes( PVOID pHead, PUCHAR pSrc, ULONG NumberOfBytes, PULONG pBytesSent )        
{
    //PSER16550_INFO   pHWHead    = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_WRITE, (TEXT("+PutBytes - Len %d.\r\n"), NumberOfBytes));

    pHWHead->CommErrors &= ~CE_TXFULL;
    *pBytesSent = 0;

    // If CTS flow control is desired, check cts. If clear, don't send,
    // but loop.  When CTS comes back on, the OtherInt routine will
    // detect this and re-enable TX interrupts (causing Flushdone).
    // For finest granularity, we would check this in the loop below,
    // but for speed, I check it here (up to 8 xmit characters before
    // we actually flow off.
    if ( pHWHead->dcb.fOutxCtsFlow ) 
	{
        // ReadMSR( pHWHead );
        // We don't need to explicitly read the MSR, since we always enable
        // IER_MS, which ensures that we will get an interrupt and read
        // the MSR whenever CTS, DSR, TERI, or DCD change.

        if (! (pHWHead->MSR & SERIAL_MSR_CTS) ) 
		{
            unsigned char byte;
            DEBUGMSG (ZONE_WRITE|ZONE_FLOW, (TEXT("PutBytes, flowed off via CTS\n")) );
            pHWHead->CTSFlowOff = TRUE;  // Record flowed off state

            EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
            try {
                byte = INB(pHWHead, pIER);
                OUTB(pHWHead, pIER, byte & ~SERIAL_IER_THR); // disable TX interrupts while flowed off
            }
            except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
                // Just ignore it, we'll eventually fall out of here
            }
#pragma prefast(pop)
            LeaveCriticalSection(&(pHWHead->RegCritSec));

            // We could return a positive value here, which would
            // cause the MDD to periodically check the flow control
            // status.  However, we don't need to since we know that
            // the DCTS interrupt will cause the MDD to call us, and we
            // will subsequently fake a TX interrupt to the MDD, causing
            // him to call back into PutBytes.
            return(0);
        }
    }

    // Same thing applies for DSR
    if ( pHWHead->dcb.fOutxDsrFlow ) 
	{
        // ReadMSR( pHWHead );
        // We don't need to explicitly read the MSR, since we always enable
        // IER_MS, which ensures that we will get an interrupt and read
        // the MSR whenever CTS, DSR, TERI, or DCD change.

        if (! (pHWHead->MSR & SERIAL_MSR_DSR) ) 
		{
            DEBUGMSG (ZONE_WRITE|ZONE_FLOW, (TEXT("PutBytes, flowed off via DSR\n")) );
            pHWHead->DSRFlowOff = TRUE;  // Record flowed off state

            EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
            try {
                OUTB(pHWHead, pIER, IER_NORMAL_INTS); // disable TX interrupts while flowed off
            }
            except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
                // Just ignore it, we'll eventually fall out of here
            }
#pragma prefast(pop)
            LeaveCriticalSection(&(pHWHead->RegCritSec));

            // See the comment above above positive return codes.
            return(0);
        }
    }

    DEBUGMSG (ZONE_WRITE, (TEXT("PutBytes wait for CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));
    EnterCriticalSection(&(pHWHead->TransmitCritSec));
    DEBUGMSG (ZONE_WRITE, (TEXT("PutBytes got CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));
    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        ReadLSR( pHWHead );
        if ( pHWHead->LSR & SERIAL_LSR_THRE ) 
		{
            UCHAR       byteCount;
            if ( pHWHead->IIR & SERIAL_IIR_FIFOS_ENABLED )
                byteCount = SERIAL_FIFO_DEPTH;
            else
                byteCount = 1;
            OUTB(pHWHead, pIER, IER_NORMAL_INTS ); // Mask Xmit Interrupt.
            DEBUGMSG (ZONE_WRITE | ZONE_THREAD, (TEXT("Put Bytes - Write max of %d bytes\r\n"), byteCount));
            for ( ; NumberOfBytes && byteCount; NumberOfBytes--, byteCount-- ) 
			{
                DEBUGLED( ZONE_WRITE, (1, 0x10200000 | *pSrc) );
                OUTB(pHWHead, pData, *pSrc);
                ++pSrc;
                (*pBytesSent)++;
            }
        }
        // Enable xmit intr. We need to do this no matter what, 
        // since the MDD relies on one final interrupt before
        // returning to the application. 
        OUTB(pHWHead, pIER, IER_NORMAL_INTS | SERIAL_IER_THR);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Hmm, not sure what would cause this.  Lets just tell
        // the MDD to go away until we get another TX
        // interrupt.
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    LeaveCriticalSection(&(pHWHead->TransmitCritSec));
    DEBUGMSG (ZONE_WRITE, (TEXT("PutBytes released CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));
    DEBUGMSG (ZONE_WRITE, (TEXT("-PutBytes - sent %d.\r\n"), *pBytesSent));

    return(0);
}

//
// @doc OEM
// @func ULONG | SL4_TXIntr | This routine is called from the old MDD
//   whenever INTR_TX is returned by SL4_GetInterruptType (Obselete)
// 
// @rdesc None
//
VOID SL4_TxIntr( PVOID pHead )               // Hardware Head
{
    //PSER16550_INFO   pHWHead    = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (0, (TEXT("+SL4_TxIntr 0x%X\r\n"), pHead));

    // Disable xmit intr.  Most 16550s will keep hammering
    // us with xmit interrupts if we don't turn them off
    // Whoever gets the FlushDone will then need to turn
    // TX Ints back on if needed.
    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        OUTB(pHWHead, pIER, IER_NORMAL_INTS);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Do nothing.  The worst case is that this was a fluke,
        // and a TX Intr will come right back at us and we will
        // resume transmission.
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    // Let the putbytes routine know he can continue
    PulseEvent(pHWHead->FlushDone);

    DEBUGMSG (0, (TEXT("+SL4_TxIntr 0x%X\r\n"), pHead));
}

//
// @doc OEM
// @func ULONG | SL4_TXIntrEx | This routine is called from the new MDD
//   whenever INTR_TX is returned by SL4_GetInterruptType
// 
// @rdesc None
//
// @parm	Hardware Head
// @parm	Pointer to transmit buffer
// @parm	In = max bytes to transmit, out = bytes transmitted
VOID SL4_TxIntrEx( PVOID pHead, PUCHAR pTxBuffer, ULONG *pBufflen )            
{
    //PSER16550_INFO   pHWHead    = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);
    ULONG NumberOfBytes = *pBufflen;

    DEBUGMSG (ZONE_THREAD, (TEXT("Transmit Event\r\n")));
    DEBUGMSG (ZONE_WRITE, (TEXT("+SL4_TxIntrEx 0x%X, Len %d\r\n"), pHead, *pBufflen));
    //RETAILMSG (1, (TEXT("+SL4_TxIntrEx LSR = 0x%x, Len %d\r\n"), (DWORD)(pHWHead->pLSR), *pBufflen));

    // We may be done sending.  If so, just disable the TX interrupts and return to the MDD.  
    if( ! *pBufflen ) 
	{
        DEBUGMSG (ZONE_WRITE, (TEXT("SL4_TxIntrEx: Disable INTR_TX.\r\n")));
        OUTB(pHWHead, pIER, IER_NORMAL_INTS);
        return;
    }
        
    *pBufflen = 0;  // In case we don't send anything below.
    
    // Disable xmit intr.  Most 16550s will keep hammering
    // us with xmit interrupts if we don't turn them off
    // Whoever gets the FlushDone will then need to turn
    // TX Ints back on if needed.
    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
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
        if ( pHWHead->dcb.fOutxCtsFlow ) 
		{
            // ReadMSR( pHWHead );
            // We don't need to explicitly read the MSR, since we always enable
            // IER_MS, which ensures that we will get an interrupt and read
            // the MSR whenever CTS, DSR, TERI, or DCD change.

            if (! (pHWHead->MSR & SERIAL_MSR_CTS) ) 
			{
                unsigned char byte;
                DEBUGMSG (ZONE_WRITE|ZONE_FLOW, (TEXT("SL4_TxIntrEx, flowed off via CTS\n")));
                pHWHead->CTSFlowOff = TRUE;  // Record flowed off state
                byte = INB(pHWHead, pIER);
                OUTB(pHWHead, pIER, byte & ~SERIAL_IER_THR); // disable TX interrupts while flowed off

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
        if ( pHWHead->dcb.fOutxDsrFlow ) 
		{
            // ReadMSR( pHWHead );
            // We don't need to explicitly read the MSR, since we always enable
            // IER_MS, which ensures that we will get an interrupt and read
            // the MSR whenever CTS, DSR, TERI, or DCD change.

            if (! (pHWHead->MSR & SERIAL_MSR_DSR) ) 
			{
                DEBUGMSG (ZONE_WRITE|ZONE_FLOW, (TEXT("SL4_TxIntrEx, flowed off via DSR\n")));
                pHWHead->DSRFlowOff = TRUE;				// Record flowed off state
                OUTB(pHWHead, pIER, IER_NORMAL_INTS);	// disable TX interrupts while flowed off
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
#pragma prefast(pop)

    LeaveCriticalSection(&(pHWHead->RegCritSec));

    //  OK, now lets actually transmit some data.
	//RETAILMSG (1, (TEXT("SL4_TxIntrEx::OK, now lets actually transmit some data.\r\n")));

    DEBUGMSG (ZONE_WRITE, (TEXT("SL4_TxIntrEx wait for CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));
    EnterCriticalSection(&(pHWHead->TransmitCritSec));
    DEBUGMSG (ZONE_WRITE, (TEXT("SL4_TxIntrEx got CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));

    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        //ReadLSR( pHWHead );
		//
		// CS&ZHL JUN-25-2008: just read LSR to speed up
		//
		pHWHead->LSR = INB(pHWHead, pLSR);
        if ( pHWHead->LSR & SERIAL_LSR_THRE ) 
		{
            UCHAR       byteCount;
            if ( pHWHead->IIR & SERIAL_IIR_FIFOS_ENABLED )
                byteCount = SERIAL_FIFO_DEPTH;
            else
                byteCount = 1;
            
            DEBUGMSG (ZONE_WRITE | ZONE_THREAD, (TEXT("SL4_TxIntrEx - Write max of %d bytes\r\n"), byteCount));
            //RETAILMSG (1, (TEXT("SL4_TxIntrEx - Write max of %d bytes\r\n"), byteCount));
            for ( *pBufflen=0; NumberOfBytes && byteCount; NumberOfBytes--, byteCount-- ) 
			{
                DEBUGLED( ZONE_WRITE, (1, 0x10200000 | *pTxBuffer) );
                OUTB(pHWHead, pData, *pTxBuffer);
				//
				// CS&ZHL JUN-25-2008: it seems unnecessary to issue EOI here??
				//
                //InterruptDone(pHWHead->dwSysIntr);			//Is this ok to inform kernel in advance?
                ++pTxBuffer;
                (*pBufflen)++;
            }
        }

        // Enable xmit intr. We need to do this no matter what, 
        // since the MDD relies on one final interrupt before
        // returning to the application. 
        DEBUGMSG (ZONE_WRITE, (TEXT("SL4_TxIntrEx: Enable INTR_TX.\r\n")));
        OUTB(pHWHead, pIER, IER_NORMAL_INTS | SERIAL_IER_THR);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Hmm, not sure what would cause this.  Lets just tell
        // the MDD to go away until we get another TX interrupt.
    }
#pragma prefast(pop)

    LeaveCriticalSection(&(pHWHead->RegCritSec));
    LeaveCriticalSection(&(pHWHead->TransmitCritSec));

    DEBUGMSG (ZONE_WRITE, (TEXT("SL4_TxIntrEx released CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));
    DEBUGMSG (ZONE_WRITE, (TEXT("-SL4_TxIntrEx - sent %d.\r\n"), *pBufflen));
    //RETAILMSG (1, (TEXT("-SL4_TxIntrEx - sent %d.\r\n"), *pBufflen));
    return;
}

//
// @doc OEM
// @func ULONG | SL4_LineIntr | This routine is called from the MDD
//   whenever INTR_LINE is returned by SL4_GetInterruptType.
// 
// @rdesc None
//
VOID SL4_LineIntr( PVOID pHead )               // Hardware Head 
{
    //PSER16550_INFO   pHWHead    = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_READ, (TEXT("+SL4_LineIntr 0x%X\r\n"), pHead));
    ReadLSR( pHWHead );
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        OUTB(pHWHead, pIIR_FCR, pHWHead->FCR| SERIAL_FCR_RCVR_RESET ); // We have to reset Receive FIFO because is is error.
        while (INB(pHWHead, pLSR) & SERIAL_LSR_DR ) {
            INB(pHWHead, pData);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
        // nothing to do.
    };
#pragma prefast(pop)
    DEBUGMSG (ZONE_READ, (TEXT("-SL4_LineIntr 0x%X\r\n"), pHead));
}

//
// @doc OEM
// @func ULONG | SL4_OtherIntr | This routine is called from the MDD
//   whenever INTR_MODEM is returned by SL4_GetInterruptType.
// 
// @rdesc None
//
VOID SL4_OtherIntr( PVOID pHead )                // Hardware Head
{
    //PSER16550_INFO   pHWHead    = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (0, (TEXT("+SL4_OtherIntr 0x%X\r\n"), pHead));

    ReadMSR( pHWHead );

    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        // If we are currently flowed off via CTS or DSR, then
        // we better signal the TX thread when one of them changes
        // so that TX can resume sending.
        if ( pHWHead->DSRFlowOff && (pHWHead->MSR & SERIAL_MSR_DSR) ) 
		{
            DEBUGMSG (ZONE_WRITE|ZONE_FLOW, (TEXT("PutBytes, flowed on via DSR\n")));
            pHWHead->DSRFlowOff = FALSE;
            // DSR is set, so go ahead and resume sending
            OUTB(pHWHead, pIER, IER_NORMAL_INTS | SERIAL_IER_THR); // Enable xmit intr.
            // Then simulate a TX intr to get things moving
            pHWHead->AddTXIntr = TRUE;
        }
        if ( pHWHead->CTSFlowOff && (pHWHead->MSR & SERIAL_MSR_CTS) ) 
		{
            DEBUGMSG (ZONE_WRITE|ZONE_FLOW, (TEXT("PutBytes, flowed on via CTS\n")));
            pHWHead->CTSFlowOff = FALSE;
            // CTS is set, so go ahead and resume sending
            OUTB(pHWHead, pIER, IER_NORMAL_INTS | SERIAL_IER_THR); // Enable xmit intr.
            // Then simulate a TX intr to get things moving
            pHWHead->AddTXIntr = TRUE;
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
#pragma prefast(pop)

    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (0, (TEXT("-SL4_OtherIntr 0x%X\r\n"), pHead));
}


//
// @doc OEM
// @func ULONG | SL4_OtherIntr | This routine is called from the MDD
//   whenever INTR_MODEM is returned by SL4_GetInterruptType.
// 
// @rdesc None
//
VOID SL4_ModemIntr( PVOID pHead )                // Hardware Head
{
    SL4_OtherIntr(pHead);
}

//  
// @doc OEM
// @func    ULONG | SL4_GetStatus | This structure is called by the MDD
//   to retrieve the contents of a COMSTAT structure.
//
// @rdesc    The return is a ULONG, representing success (0) or failure (-1).
//
// @parm PVOID returned by HWInit.
// @parm Pointer to LPCOMMSTAT to hold status.
ULONG SL4_GetStatus( PVOID pHead, LPCOMSTAT lpStat )    
{
    //PSER16550_INFO pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);
    ULONG      RetVal  = pHWHead->CommErrors;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_GetStatus 0x%X\r\n"), pHead));

    pHWHead->CommErrors = 0; // Clear old errors each time

    if ( lpStat ) {
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
    } else
        RetVal = (ULONG)-1;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_GetStatus 0x%X\r\n"), pHead));
    return(RetVal);
}

//
// @doc OEM
// @func    ULONG | SL4_Reset | Perform any operations associated
//   with a device reset
//
// @rdesc    None.
//
VOID SL4_Reset( PVOID pHead )		    // @parm PVOID returned by HWInit.
{
    //PSER16550_INFO pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_Reset 0x%X\r\n"), pHead));

    memset(&pHWHead->Status, 0, sizeof(COMSTAT));

    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        OUTB(pHWHead, pIER, IER_NORMAL_INTS);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Do nothing
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_Reset 0x%X\r\n"), pHead));
}

//
// @doc OEM
// @func    VOID | SL4_GetModemStatus | Retrieves modem status.
//
// @rdesc    None.
//
// @parm PVOID returned by HWInit.
// @parm PULONG passed in by user.
VOID SL4_GetModemStatus( PVOID pHead, PULONG pModemStatus )    
{
    //PSER16550_INFO pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);
    UINT8 ubModemStatus;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_GetModemStatus 0x%X\r\n"), pHead));

    ReadMSR( pHWHead );
    ubModemStatus = pHWHead->MSR;

    if ( ubModemStatus & SERIAL_MSR_CTS )
        *pModemStatus |= MS_CTS_ON;

    if ( ubModemStatus & SERIAL_MSR_DSR )
        *pModemStatus |= MS_DSR_ON;

    if ( ubModemStatus & SERIAL_MSR_RI )
        *pModemStatus |= MS_RING_ON;

    if ( ubModemStatus & SERIAL_MSR_DCD )
        *pModemStatus |= MS_RLSD_ON;

    DEBUGMSG (ZONE_FUNCTION | ZONE_EVENTS, 
		      (TEXT("-SL4_GetModemStatus 0x%X (stat x%X) \r\n"), pHead, *pModemStatus));
}

//
// @doc OEM
// @func    VOID | SL4_PurgeComm | Purge RX and/or TX
// 
// @rdesc    None.
//
// @parm PVOID returned by HWInit.
// @parm Action to take. 
VOID SL4_PurgeComm( PVOID pHead, DWORD fdwAction )        
{
    //PSER16550_INFO pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_PurgeComm 0x%X\r\n"), pHead));

    EnterCriticalSection(&(pHWHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
#ifdef TODO
        // REVIEW THIS - I don't see how this could have terminated a pending read,
        // nor how RX interrupts would ever get turned back on.  I suspect that
        // RXABORT and TXABORT would both be better implemented in the MDD.
        if ( fdwAction & PURGE_RXABORT )
            OUTB(pHWHead, pIER, IER_NORMAL_INTS & ~SERIAL_IER_RDA);
#endif    
        if ( fdwAction & PURGE_TXCLEAR ) {
            // Write the TX reset bit.  It is self clearing
            OUTB(pHWHead, pIIR_FCR, pHWHead->FCR | SERIAL_FCR_TXMT_RESET);
        }

        if ( fdwAction & PURGE_RXCLEAR ) {
            // Write the RX reset bit.  It is self clearing
            OUTB(pHWHead, pIIR_FCR, pHWHead->FCR | SERIAL_FCR_RCVR_RESET);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pHWHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_PurgeComm 0x%X\r\n"), pHead));
    return;
}

//
// @doc OEM
// @func    BOOL | SL4_XmitComChar | Transmit a char immediately
// 
// @rdesc    TRUE if succesful
//
// @parm PVOID returned by HWInit.
// @parm Character to transmit. 
BOOL SL4_XmitComChar( PVOID pHead, UCHAR ComChar )   
{
    //PSER16550_INFO pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_XmitComChar 0x%X\r\n"), pHead));

    // Get critical section, then transmit when buffer empties
    DEBUGMSG (ZONE_WRITE, (TEXT("XmitComChar wait for CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));
    EnterCriticalSection(&(pHWHead->TransmitCritSec));
    DEBUGMSG (ZONE_WRITE, (TEXT("XmitComChar got CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));
    try {
        //while ( TRUE ) {  // We know THR will eventually empty
        for( ; ; ) {  // We know THR will eventually empty
            EnterCriticalSection(&(pHWHead->RegCritSec));
            // Write the character if we can
            ReadLSR( pHWHead );
            if ( pHWHead->LSR & SERIAL_LSR_THRE ) 
			{
                // FIFO is empty, send this character
                OUTB(pHWHead, pData, ComChar);
                // Make sure we release the register critical section
                OUTB(pHWHead, pIER, IER_NORMAL_INTS | SERIAL_IER_THR);
                LeaveCriticalSection(&(pHWHead->RegCritSec));
               
                DEBUGMSG (ZONE_WRITE, (TEXT("XmitComChar wrote x%X\r\n"), ComChar));
                break;
            }

			// If we couldn't write the data yet, then wait for a
            // TXINTR to come in and try it again.

            // Enable xmit intr.
            OUTB(pHWHead, pIER, IER_NORMAL_INTS | SERIAL_IER_THR);
            LeaveCriticalSection(&(pHWHead->RegCritSec));

            // Wait until the txintr has signalled.
            DEBUGMSG (ZONE_WRITE, (TEXT("XmitComChar WaitIntr x%X\r\n"), pHWHead->FlushDone));
            WaitForSingleObject(pHWHead->FlushDone, (ULONG)1000);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Make sure we release the register critical section
        LeaveCriticalSection(&(pHWHead->RegCritSec));
    }

    LeaveCriticalSection(&(pHWHead->TransmitCritSec));
    DEBUGMSG (ZONE_WRITE, (TEXT("XmitComChar released CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_XmitComChar 0x%X\r\n"), pHead));
    return(TRUE);
}

//
// @doc OEM
// @func    BOOL | SL4_PowerOff | Perform powerdown sequence.
// 
// @rdesc    TRUE if succesful
//
VOID
SL4_PowerOff( PVOID pHead )       // @parm    PVOID returned by HWInit.
{
    UNREFERENCED_PARAMETER(pHead);
	/*
    //PSER16550_INFO pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    // Current FCR is already saved in a shadow

    // Current IER is not normally shadowed, save it
    pHWHead->IER = INB(pHWHead, pIER);

    // Current LCR is not normally shadowed, save it
    pHWHead->LCR = INB(pHWHead, pLCR);

    // Current MCR is not normally shadowed, save it
    pHWHead->MCR = INB(pHWHead, pMCR);

    // Current Scratch is not normally shadowed, save it
    pHWHead->Scratch = INB(pHWHead, pScratch);

    pHWHead->PowerDown = TRUE;
	*/
	// nothing to do as other UARTs
}

//
// @doc OEM
// @func    BOOL | SL4_PowerOn | Perform poweron sequence.
// 
// @rdesc    TRUE if succesful
//
VOID SL4_PowerOn( PVOID pHead )        // @parm    PVOID returned by HWInit.
{
    UNREFERENCED_PARAMETER(pHead);
	/*
    //PSER16550_INFO pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    if (pHWHead->PowerDown) {
        // Restore any registers that we need

        // In power handler context, so don't try to do a critical section
        OUTB(pHWHead, pIIR_FCR, pHWHead->FCR);
        OUTB(pHWHead, pIER, pHWHead->IER);
        OUTB(pHWHead, pLCR, pHWHead->LCR);
        OUTB(pHWHead, pMCR, pHWHead->MCR);
        OUTB(pHWHead, pScratch, pHWHead->Scratch);

        pHWHead->PowerDown = FALSE;

        // And we didn't save the Divisor Reg, so set baud rate
        // But don't call SL4_SetBaud, since it does DebugMsg.
        // Call our internal function instead.  Can't acquire
        // the RegCritSec, but shouldn't really need to since
        // we are in power context.
        SetUARTBaudRate( pHWHead, pHWHead->dcb.BaudRate );
        pHWHead->bSuspendResume = TRUE;
        SetInterruptEvent(pHWHead->dwSysIntr);
    }
	*/
	// nothing to do as other UARTs
}

//
// @doc OEM
// @func    BOOL | SL4_SetDCB | Sets new values for DCB.  This
// routine gets a DCB from the MDD.  It must then compare
// this to the current DCB, and if any fields have changed take
// appropriate action.
// 
// @rdesc    BOOL
//
// @parm    PVOID returned by HWInit.
// @parm    Pointer to DCB structure
BOOL SL4_SetDCB( PVOID pHead, LPDCB lpDCB )       
{
    //PSER16550_INFO pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);
    BOOL bRet;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_SetDCB 0x%X\r\n"), pHead));

    bRet = TRUE;

    // If the device is open, scan for changes and do whatever
    // is needed for the changed fields.  if the device isn't
    // open yet, just save the DCB for later use by the open.
    if ( pHWHead->OpenCount ) 
	{
        // Note, fparity just says whether we should check
        // receive parity.  And the 16550 won't let us NOT
        // check parity if we generate it.  So this field
        // has no effect on the hardware.

        if ( lpDCB->BaudRate != pHWHead->dcb.BaudRate ) 
            bRet = SL4_SetBaudRate( pHead, lpDCB->BaudRate );
        
        if ( bRet && (lpDCB->ByteSize != pHWHead->dcb.ByteSize )) 
            bRet = SL4_SetByteSize( pHead, lpDCB->ByteSize );

        if ( bRet && (lpDCB->Parity != pHWHead->dcb.Parity )) 
            bRet = SL4_SetParity( pHead, lpDCB->Parity );

        if ( bRet && (lpDCB->StopBits != pHWHead->dcb.StopBits )) 
            bRet = SL4_SetStopBits( pHead, lpDCB->StopBits );

        // Don't worry about fOutxCtsFlow.  It is a flag which
        // will be examined every time we load the TX buffer.
        // No special action required here.
    }

    if (bRet)
	{
		//pHWHead->dcb = *lpDCB;		// Now that we have done the right thing, store this DCB
		memcpy(&pHWHead->dcb, lpDCB, sizeof(DCB));
	}

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_SetDCB 0x%X\r\n"), pHead));

    return(bRet);
}

//
// @doc OEM
// @func    BOOL | SL4_SetCommTimeouts | Sets new values for the
// CommTimeouts structure. routine gets a DCB from the MDD.  It
// must then compare this to the current DCB, and if any fields
// have changed take appropriate action.
// 
// @rdesc    ULONG
//
// @parm    PVOID returned by HWInit.
// @parm	Pointer to CommTimeout structure
BOOL SL4_SetCommTimeouts( PVOID pHead, LPCOMMTIMEOUTS lpCommTimeouts )
{
    //PSER16550_INFO pHWHead = (PSER16550_INFO)pHead;
    PSER16550_INFO pHWHead = &(((PCOM_INFO)pHead)->ser16550);

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_SetCommTimeout 0x%X\r\n"), pHead));

    // OK, first check for any changes and act upon them
    if ( lpCommTimeouts->WriteTotalTimeoutMultiplier !=
         pHWHead->CommTimeouts.WriteTotalTimeoutMultiplier ) 
	{		// for what action?
    }

    // Now that we have done the right thing, store this DCB
    //pHWHead->CommTimeouts = *lpCommTimeouts;
	memcpy(&pHWHead->CommTimeouts, lpCommTimeouts, sizeof(COMMTIMEOUTS));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_SetCommTimeout 0x%X\r\n"), pHead));

    return(TRUE);
}



//
//  @doc OEM
//  @func    BOOL | SL4_Ioctl | Device IO control routine.  
//  @parm DWORD | dwOpenData | value returned from COM_Open call
//    @parm DWORD | dwCode | io control code to be performed
//    @parm PBYTE | pBufIn | input data to the device
//    @parm DWORD | dwLenIn | number of bytes being passed in
//    @parm PBYTE | pBufOut | output data from the device
//    @parm DWORD | dwLenOut |maximum number of bytes to receive from device
//    @parm PDWORD | pdwActualOut | actual number of bytes received from device
//
//    @rdesc        Returns TRUE for success, FALSE for failure
//
//  @remark  The MDD will pass any unrecognized IOCTLs through to this function.
//
BOOL SL4_Ioctl(PVOID pHead, DWORD dwCode,PBYTE pBufIn,DWORD dwLenIn,
               PBYTE pBufOut,DWORD dwLenOut,PDWORD pdwActualOut)
{
    BOOL RetVal = TRUE;

    UNREFERENCED_PARAMETER(pHead);
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pdwActualOut);

	DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL4_Ioctl 0x%X\r\n"), pHead));

	switch (dwCode) 
	{
    // Currently, no defined IOCTLs
    default:
        RetVal = FALSE;
        DEBUGMSG (ZONE_FUNCTION, (TEXT(" Unsupported ioctl 0x%X\r\n"), dwCode));
        break;            
    }
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL4_Ioctl 0x%X\r\n"), pHead));
    return(RetVal);
}

// ----- end of isa_16550.c ----- //