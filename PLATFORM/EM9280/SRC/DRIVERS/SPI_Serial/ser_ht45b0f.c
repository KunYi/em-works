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

ser_ht45b0f.c

Abstract:  

    This file implements the standard device specific functions for ht45b0f

Functions:

    SL4_Init()
    SL_PostInit()
    SL_Deinit()
    SL_Open()
    SL_Close()
    SL_ClearDTR()
    SL_SetDTR()
    SL_ClearRTS()
    SL_SetRTS()
    SL_ClearBreak()
    SL_SetBreak()
    SL_SetBaudRate()
    SL_SetByteSize()
    SL_SetParity()
    SL_SetStopBits()
    SL_GetRxBufferSize()
    SL_GetRxStart()
    SL_GetInterruptType()
    SL_RxIntr()
    SL_PutBytes()
    SL_TxIntr()
    SL_LineIntr()
    SL_OtherIntr()
    SL_GetStatus()
    SL_Reset()
    SL_GetModemStatus()
    SL_PurgeComm()
    SL_XmitComChar()
    SL_PowerOff()
    SL_PowerOn()
    SL_SetDCB()
    SL_SetCommTimeouts()
    SL_Ioctl()
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
//#include <hw16550.h>


// Controller includes
#include "bsp.h"

// it is important to include head files "*.h" as above oder! 
#include "spi_com.h"
#include "ser_ht45b0f.h"
#include "em9280_oal.h"

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

//! PWM_7 output frequency times 100
#define COMCLK_OUTPUT_FREQ_TIMES_100		(1846154UL * 100UL)				// 24MHz / 13
#define	COMCLK_OUTPUT_HIGH_PERCENT			46								// 46% -> (6 / 13)

// Macros to read/write serial registers.
//#define INB(pInfo, reg) (READ_PORT_UCHAR((UCHAR *)((pInfo)->reg)))
//#define OUTB(pInfo, reg, value) (WRITE_PORT_UCHAR((UCHAR *)((pInfo)->reg), (unsigned char)(value)))

//#define INB(pInfo, reg)             (*((volatile UCHAR *)((pInfo)->reg)))
//#define OUTB(pInfo, reg, value)     (*(volatile UCHAR *)((pInfo)->reg) = (UCHAR)(value))
//
// CS&ZHL MAR-13-2012: construct INB/OUTB
//
BYTE INB(PVOID pHead, DWORD dwRegIdx)
{
	BYTE			ub1;
	SpiAccessInfo	SpiInfo;
    PSER_INFO		pSerHead = (PSER_INFO)pHead;   

	SpiInfo.dwAccessCode = SPI_ACCESS_CODE_READBYTE;
	SpiInfo.dwCSNum = pSerHead->dwLocalIndex;
	SpiInfo.dwDataLength = 1;
	SpiInfo.pDataBuf = &ub1;
	dwRegIdx &= 0x07;
	if(dwRegIdx == HT45B0F_REG_DAT)
	{
		SpiInfo.dwStartAddr = HT45B0F_DAT_READ | dwRegIdx;
	}
	else
	{
		SpiInfo.dwStartAddr = HT45B0F_CMD_READ | dwRegIdx;
	}

	if (!KernelIoControl(IOCTL_HAL_SPI_ACCESS, 
						(PVOID)&SpiInfo, sizeof(SpiAccessInfo), 
						NULL, 0, 
						NULL))
	{
		RETAILMSG(1, (TEXT("INB::read SPI register failed 0x%X\r\n"), ub1));
	}

	return ub1;
}

void OUTB(PVOID pHead, DWORD dwRegIdx, BYTE uValue)
{
	SpiAccessInfo	SpiInfo;
    PSER_INFO		pSerHead = (PSER_INFO)pHead;   

	SpiInfo.dwAccessCode = SPI_ACCESS_CODE_WRITEBYTE;
	SpiInfo.dwCSNum = pSerHead->dwLocalIndex;
	SpiInfo.dwDataLength = 1;
	SpiInfo.pDataBuf = &uValue;
	dwRegIdx &= 0x07;
	if(dwRegIdx == HT45B0F_REG_DAT)
	{
		SpiInfo.dwStartAddr = HT45B0F_DAT_WRITE | dwRegIdx;
	}
	else
	{
		SpiInfo.dwStartAddr = HT45B0F_CMD_WRITE | dwRegIdx;
	}

	if (!KernelIoControl(IOCTL_HAL_SPI_ACCESS, 
						(PVOID)&SpiInfo, sizeof(SpiAccessInfo), 
						NULL, 0, 
						NULL))
	{
		RETAILMSG(1, (TEXT("OUTB::write SPI register failed\r\n")));
	}
}

BOOL SL_SetByteSize(PVOID pHead, ULONG ByteSize);
BOOL SL_SetStopBits(PVOID pHead, ULONG StopBits);
BOOL SL_SetParity(PVOID pHead, ULONG Parity);

#define EXCEPTION_ACCESS_VIOLATION STATUS_ACCESS_VIOLATION 

//
// Reading the LSR clears most of its bits.  So, we provide this wrapper,
// which reads the register, records any interesting values, and
// stores the current LSR contents in the shadow register.
//
__inline VOID ProcessLSR (PSER_INFO pSerHead)
{
    ULONG LineEvents = 0;

    if (pSerHead->UartHt45.USR & (HT45B0F_USR_OERR | HT45B0F_USR_PERR | HT45B0F_USR_FERR)) 
	{
        // Note: Its not wise to do debug msgs in here since they will
        // pretty much guarantee that the FIFO gets overrun.
        if ( pSerHead->UartHt45.USR & HT45B0F_USR_OERR ) 
		{
            // DEBUGMSG (ZONE_WARN, (TEXT("Overrun\r\n")));
            pSerHead->DroppedBytes++;
            pSerHead->CommErrors |= CE_OVERRUN;
        }

        if ( pSerHead->UartHt45.USR & HT45B0F_USR_PERR ) 
		{
            // DEBUGMSG (ZONE_WARN, (TEXT("parity\r\n")));
            pSerHead->CommErrors |= CE_RXPARITY;
        }

        if ( pSerHead->UartHt45.USR & HT45B0F_USR_FERR ) 
		{
            // DEBUGMSG (ZONE_WARN, (TEXT("frame\r\n")));
            pSerHead->CommErrors |= CE_FRAME;
        }

        LineEvents |= EV_ERR;
    }

	// ignore break 
    //if ( pHWHead->LSR & SERIAL_LSR_BI )
    //    LineEvents |= EV_BREAK;

    // Let WaitCommEvent know about this error
    if ( LineEvents )
        pSerHead->EventCallback( pSerHead->pMddHead, LineEvents );
}

__inline VOID ReadLSR( PSER_INFO  pSerHead )
{
    try {
        pSerHead->UartHt45.USR = INB(pSerHead, HT45B0F_REG_USR);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
		pSerHead->UartHt45.USR = HT45B0F_USR_RIDLE | HT45B0F_USR_TIDLE | HT45B0F_USR_TXIF;
    }
	ProcessLSR (pSerHead);
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
__inline VOID ProcessMSR (PSER_INFO  pSerHead)
{
    UNREFERENCED_PARAMETER(pSerHead);
	//nothing to do as HT45 hasn't any modem lines.
}

__inline VOID ReadMSR( PSER_INFO  pSerHead )
{
    UNREFERENCED_PARAMETER(pSerHead);
	//nothing to do as HT45 hasn't any modem lines.
}


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

#define BAUD_TABLE_SIZE 9
static const PAIRS LS_BaudPairs[BAUD_TABLE_SIZE] =    
{
    {1200,   0x009C},
    {2400,   0x004E},
    {4800,   0x019C},
    {9600,   0x014E},
    {14400,  0x0134},
    {19200,  0x0127},
    {28800,  0x011A},
    {38400,  0x0114},			// error = +2.4%
    {57600,  0x010D}
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
    PSER_INFO  pSerHead = (PSER_INFO)pHead;

    divisor = (USHORT)LookUpBaudTableValue(BaudRate, pSerHead->pBaudTable, &errorcode);

    if ( errorcode )
        divisor = 0;

    return(divisor);
}

#define HT45B0F_NORMAL_INTS		(HT45B0F_UCR2_RX_INTEN)												// Rx interrupt enable
#define HT45B0F_STATUS_IDLE		(HT45B0F_USR_RIDLE | HT45B0F_USR_TIDLE | HT45B0F_USR_TXIF)			// 8'b00001011

// Routine to clear any pending interrupts.  Called from Init and PostInit
// to make sure we atart out in a known state.
VOID ExClearPendingInts( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    PSER_INFO pSerHead = (PSER_INFO)pHead;

    EnterCriticalSection(&(pSerHead->RegCritSec));
    try {
        pSerHead->UartHt45.USR = INB(pSerHead, HT45B0F_REG_USR); 
        while((pSerHead->UartHt45.USR & HT45B0F_STATUS_IDLE) != HT45B0F_STATUS_IDLE) 
		{
            DEBUGMSG (ZONE_INIT, (TEXT("USR = 0x%02X\r\n"), pSerHead->UartHt45.USR));
            // Reading LSR clears RLS interrupts.
            ReadLSR( pSerHead );

            // Reset RX FIFO to clear any old data remaining in it.
			while(pSerHead->UartHt45.USR & HT45B0F_USR_RXIF)
			{
				pSerHead->UartHt45.RDR = INB(pSerHead, HT45B0F_REG_DAT);		// read data out
				pSerHead->UartHt45.USR = INB(pSerHead, HT45B0F_REG_USR);		// read status again
			}

            // Reading MSR clears Modem Status interrupt
            ReadMSR( pSerHead );

			// read status again
			pSerHead->UartHt45.USR = INB(pSerHead, HT45B0F_REG_USR); 
        }    
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
        DEBUGMSG (ZONE_ERROR,(TEXT("-SL_PostInit, 0x%X - ERROR\r\n"), pSerHead));
        // Just fall through & release CritSec
    }
    LeaveCriticalSection(&(pSerHead->RegCritSec));
}

//
/////////////////// Start of exported entrypoints ////////////////
//
#define WATERMAKER_ENTRY 2
//
// @doc OEM 
// @func PVOID | SL_Open | Configures 16550 for default behaviour.
//
VOID SL_Open( PVOID pHead )	 // @parm PVOID returned by HWinit.
{
    PSER_INFO	pSerHead = (PSER_INFO)pHead;   

    //DEBUGMSG (ZONE_OPEN, (TEXT("+SL_Open 0x%X\r\n"), pHead));
    //RETAILMSG(1, (TEXT("+SL_Open 0x%X\r\n"), pHead));

    pSerHead->DroppedBytes = 0;
    pSerHead->CTSFlowOff   = FALSE;  // Not flowed off yet
    pSerHead->DSRFlowOff   = FALSE;  // Not flowed off yet
    pSerHead->CommErrors   = 0;
    pSerHead->ModemStatus  = 0;

    EnterCriticalSection(&(pSerHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
		// enable GPIO interrupt
		DDKGpioIntrruptEnable((DDK_IOMUX_PIN)pSerHead->dwIrqGpioPin);

        // Set default framing bits: 8-N-1.
		pSerHead->UartHt45.UCR1 = HT45B0F_UCR1_UARTEN | HT45B0F_UCR1_DATABIT_8 | HT45B0F_UCR1_PARITY_DIS | HT45B0F_UCR1_STOPBIT_1;
        OUTB(pSerHead, HT45B0F_REG_UCR1, pSerHead->UartHt45.UCR1);

		// enable TX and RX
		pSerHead->UartHt45.UCR2 |= (HT45B0F_UCR2_TXEN | HT45B0F_UCR2_RXEN);
        OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);

        // Get defaults from the DCB structure
        DEBUGMSG (ZONE_OPEN, (TEXT("SL_Open Setting DCB parameters\r\n")));
        SL_SetBaudRate( pSerHead, pSerHead->dcb.BaudRate );
        SL_SetByteSize( pSerHead, pSerHead->dcb.ByteSize );
        SL_SetStopBits( pSerHead, pSerHead->dcb.StopBits );
        SL_SetParity( pSerHead, pSerHead->dcb.Parity );

        // For CE 3.0, we are still supporting
        // the old style MDDs, and they don't call our PostInit, which
        // needs to happen sometime prior to this.  So for now, we go ahead
        // ahead and clear out interrupts one last time.  In 4.0, we can
        // kill the old serial MDD and assume that everyone uses the new
        // MDD and calls post init.  
        SL_PostInit( pHead );			// clear interrupt flag in UART hardware register

		// enable interrupt
        DEBUGMSG(ZONE_OPEN, (TEXT("SL_Open enable RX interrupt\r\n")));
		pSerHead->UartHt45.UCR2 &= ~(HT45B0F_UCR2_TIDLE_INTEN | HT45B0F_UCR2_TXIF_INTEN);	//disable TX interrupt
		pSerHead->UartHt45.UCR2 |= HT45B0F_UCR2_RX_INTEN;									//enable RX interrupt
        OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
 
        ReadLSR(pSerHead);

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
    LeaveCriticalSection(&(pSerHead->RegCritSec));

	//DEBUGMSG (ZONE_OPEN, (TEXT("-SL_Open::COM%d\r\n"), pSerHead->dwDeviceArrayIndex));
	//RETAILMSG(1, (TEXT("-SL_Open::COM%d\r\n"), pSerHead->dwDeviceArrayIndex));
}

//
// @doc OEM 
// @func PVOID | SL_Close | Does nothing except keep track of the
// open count so that other routines know what to do.
//
VOID SL_Close( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    PSER_INFO	pSerHead = (PSER_INFO)pHead;
    ULONG		uTries = 0;
	UCHAR		ub1;

    //DEBUGMSG(ZONE_CLOSE, (TEXT("+SL_Close 0x%X\r\n"), pHead));
    //RETAILMSG(1, (TEXT("+SL_Close 0x%X\r\n"), pHead));

    if ( pSerHead->OpenCount )
        pSerHead->OpenCount--;

    EnterCriticalSection(&(pSerHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
		// disable interrupt
		pSerHead->UartHt45.UCR2 &= ~(HT45B0F_UCR2_RX_INTEN | HT45B0F_UCR2_TIDLE_INTEN | HT45B0F_UCR2_TXIF_INTEN);
        OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
	
		//
		// CS&ZHL JAN-23-2009: while we are still transmitting, sleep.
		//
		ub1 = INB(pSerHead, HT45B0F_REG_USR) & HT45B0F_STATUS_IDLE;
		while ( (ub1 != HT45B0F_STATUS_IDLE)				// indicates FIFO not yet empty
              && (uTries++ < 100))											// safety net
		{
			DEBUGMSG ( ZONE_CLOSE, (TEXT("SerClose, TX in progress, USR 0x%X\r\n"), ub1));

			// clear data in Rx FIFO as well
			while(ub1 & HT45B0F_USR_RXIF)
			{
				pSerHead->UartHt45.RDR = INB(pSerHead, HT45B0F_REG_DAT);		// read data out
				ub1 = INB(pSerHead, HT45B0F_REG_USR) & HT45B0F_STATUS_IDLE;		// read status again
			}

			Sleep(10);
			ub1 = INB(pSerHead, HT45B0F_REG_USR) & HT45B0F_STATUS_IDLE;
		}

		// disable GPIO interrupt
		DDKGpioIntrruptDisable((DDK_IOMUX_PIN)pSerHead->dwIrqGpioPin);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
        // Just get out of here.
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pSerHead->RegCritSec));

    //DEBUGMSG (ZONE_CLOSE, (TEXT("-SL_Close 0x%X\r\n"), pHead));
    //RETAILMSG(1, (TEXT("-SL_Close 0x%X\r\n"), pHead));
}

//
// @doc OEM 
// @func PVOID | SL_Init | Initializes 16550 device head.  
//
BOOL
SL4_Init(
       PVOID		pHead,				// @parm points to device head
       PUCHAR		pRegBase,			// Pointer to 16550 register base, not used in EM9280
       UINT8		RegStride,			// Stride amongst the 16550 registers, not used in EM9280
       EVENT_FUNC	EventCallback,		// This callback exists in MDD
       PVOID		pMddHead,			// This is the first parm to callback
       PLOOKUP_TBL  pBaudTable			// BaudRate Table
       )
{
    PSER_INFO	pSerHead = (PSER_INFO)pHead;
	UCHAR		ub1 = 0;

    UNREFERENCED_PARAMETER(pRegBase);
    UNREFERENCED_PARAMETER(RegStride);

    DEBUGMSG (ZONE_CLOSE,(TEXT("+SL4_INIT, 0x%X\r\n"), pHead));

    // Store info for callback function
    pSerHead->EventCallback = EventCallback;
    pSerHead->pMddHead = pMddHead;

    // Now set up remaining fields
    if ( pBaudTable != NULL )
        pSerHead->pBaudTable = (LOOKUP_TBL *)pBaudTable;
    else
        pSerHead->pBaudTable = (LOOKUP_TBL *)&LS_BaudTable;

	// issue a soft reset to the device
	OUTB(pSerHead, HT45B0F_REG_UCR3, HT45B0F_UCR3_USRT);
	Sleep(1);
	ub1 = 0;
	while((INB(pSerHead, HT45B0F_REG_UCR3) & HT45B0F_UCR3_USRT) && (ub1 < 10))
	{
		RETAILMSG(1, (TEXT("SL_INIT::wait soft-reset completed\r\n")));
		Sleep(1);
		ub1++;
	}
	if(ub1 >= 10)
	{
		RETAILMSG(1, (TEXT("SL_INIT:: COM%d soft-reset failed\r\n"), pSerHead->dwDeviceArrayIndex));
		return(FALSE);
	}

    // Init HT45 info
	pSerHead->UartHt45.USR  = INB(pSerHead, HT45B0F_REG_USR);		// = HT45B0F_USR_RIDLE | HT45B0F_USR_TIDLE | HT45B0F_USR_TXIF;
	pSerHead->UartHt45.UCR1 = 0;
	pSerHead->UartHt45.UCR2 = HT45B0F_UCR2_BRGH;					// high speed mode: BR = 12MHz / (16 * (BRG + 1))
	pSerHead->UartHt45.BRG  = 78 - 1;								// default settings => 9600bps
	pSerHead->UartHt45.UCR3 = 0;


	// write default value to registers
    OUTB(pSerHead, HT45B0F_REG_UCR1, pSerHead->UartHt45.UCR1);
	////debug only
	//ub1 = INB(pSerHead, HT45B0F_REG_UCR1);
	//RETAILMSG(1, (TEXT("SL_INIT::WR_UCR1[0x%02x], RD_UCR1[0x%02x]\r\n"), pSerHead->UartHt45.UCR1, ub1));

    OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
	////debug only
	//ub1 = INB(pSerHead, HT45B0F_REG_UCR2);
	//RETAILMSG(1, (TEXT("SL_INIT::WR_UCR2[0x%02x], RD_UCR2[0x%02x]\r\n"), pSerHead->UartHt45.UCR2, ub1));

    OUTB(pSerHead, HT45B0F_REG_BRG, pSerHead->UartHt45.BRG);
	// read back value of BRG
	ub1 = INB(pSerHead, HT45B0F_REG_BRG);
	if(ub1 != pSerHead->UartHt45.BRG)
	{
		RETAILMSG(1, (TEXT("SL_INIT::Read BRG[0x%02x] != Write BRG[0x%02x]\r\n"), ub1, pSerHead->UartHt45.BRG));
		return(FALSE);
	}
	//debug only
	//RETAILMSG(1, (TEXT("SL_INIT::WR_BRG[0x%02x], RD_BRG[0x%02x]\r\n"), pSerHead->UartHt45.BRG, ub1));

    pSerHead->FlushDone = CreateEvent(0, FALSE, FALSE, NULL);
    pSerHead->OpenCount = 0;

    InitializeCriticalSection(&(pSerHead->TransmitCritSec));
    InitializeCriticalSection(&(pSerHead->RegCritSec));
    pSerHead->PowerDown = FALSE;
    pSerHead->bSuspendResume = FALSE;

	//Software Interrupt is not supported here
    //InstallSoftwareISR(pHWHead,pHWHead->pVirtualStaticAddr,RegStride);
    
	// Clear any interrupts which may be pending.  Normally only
    // happens if we were warm reset.
    ExClearPendingInts( pHead );

	// setup GPIO interrupt function
	{
		DDK_GPIO_CFG	intrCfg;

        DDKIomuxSetPinMux((DDK_IOMUX_PIN)pSerHead->dwIrqGpioPin, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin((DDK_IOMUX_PIN)pSerHead->dwIrqGpioPin, 0);		// output disable
        DDKIomuxSetPadConfig((DDK_IOMUX_PIN)pSerHead->dwIrqGpioPin, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);  

		// config GPIO interrupt c
		intrCfg.DDK_PIN_IO           = DDK_GPIO_INPUT;
		intrCfg.DDK_PIN_IRQ_CAPABLE  = DDK_GPIO_IRQ_CAPABLE;
		intrCfg.DDK_PIN_IRQ_ENABLE   = DDK_GPIO_IRQ_DISABLED;				// DDK_GPIO_IRQ_ENABLED;
		intrCfg.DDK_PIN_IRQ_LEVEL    = DDK_GPIO_IRQ_EDGE;					// DDK_GPIO_IRQ_LEVEL;
		intrCfg.DDK_PIN_IRQ_POLARITY = DDK_GPIO_IRQ_POLARITY_LO;			// interrupt trigger on falling edge
		if(!DDKGpioConfig((DDK_IOMUX_PIN)pSerHead->dwIrqGpioPin, intrCfg))
		{
			RETAILMSG(1,(TEXT("SL_Init: config %d# pin failed\r\n"), (DDK_IOMUX_PIN)pSerHead->dwIrqGpioPin));
			return(FALSE);
		}
		DDKGpioClearIntrPin((DDK_IOMUX_PIN)pSerHead->dwIrqGpioPin);
	}

	DEBUGMSG (ZONE_CLOSE,(TEXT("-SL_INIT, 0x%X\r\n"), pHead));
	return(TRUE);
}

//
// @doc OEM
// @func void | SL_PostInit | This routine takes care of final initialization.
//
// @rdesc None.
//
BOOL SL_PostInit( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    DEBUGMSG (ZONE_INIT,(TEXT("+SL_PostInit, 0x%X\r\n"), pHead));
    //RETAILMSG (1, (TEXT("+SL_PostInit, 0x%X\r\n"), pHead));
    
    // Since we are just a library which might get used for 
    // builtin ports which init at boot, or by PCMCIA ports
    // which init at (, we can't do anything too fancy.
    // Lets just make sure we cancel any pending interrupts so
    // that if we are being used with an edge triggered PIC, he
    // will see an edge after the MDD hooks the interrupt.
    ExClearPendingInts( pHead );
    
    DEBUGMSG (ZONE_INIT,(TEXT("-SL_PostInit, 0x%X\r\n"), pHead));
    //RETAILMSG (1, (TEXT("-SL_PostInit, 0x%X\r\n"), pHead));
    return(TRUE);
}

//
// @doc OEM 
// @func PVOID | SL_Deinit | De-initializes 16550 device head.  
//
VOID SL_Deinit( PVOID pHead )				// @parm points to device head
{
    PSER_INFO   pSerHead   = (PSER_INFO)pHead;

    DEBUGMSG (ZONE_CLOSE,(TEXT("+SL_DEINIT, 0x%X\r\n"), pSerHead));
   
	DeleteCriticalSection(&(pSerHead->TransmitCritSec));
    DeleteCriticalSection(&(pSerHead->RegCritSec));

    // Free the flushdone event
    if ( pSerHead->FlushDone )
        CloseHandle( pSerHead->FlushDone );

    DEBUGMSG (ZONE_CLOSE,(TEXT("-SL_DEINIT, 0x%X\r\n"), pSerHead));
}

//
// @doc OEM
// @func void | SL_ClearDtr | This routine clears DTR.
//
// @rdesc None.
//
VOID SL_ClearDTR( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    UNREFERENCED_PARAMETER(pHead);
	// nothing to do
}

//
// @doc OEM
// @func VOID | SL_SetDTR | This routine sets DTR.
// 
// @rdesc None.
//
VOID SL_SetDTR( PVOID pHead )			// @parm PVOID returned by HWinit.
{    
    UNREFERENCED_PARAMETER(pHead);
	// nothing to do
}

//
// @doc OEM
// @func VOID | SL_ClearRTS | This routine clears RTS.
// 
// @rdesc None.
// 
VOID SL_ClearRTS( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    PSER_INFO   pSerHead = (PSER_INFO)pHead;

	// CS&ZHL JUN-14-2012: setup RTS if(fRtsControl == RTS_CONTROL_TOGGLE)
	if((pSerHead->dcb.fRtsControl == RTS_CONTROL_TOGGLE) && (pSerHead->dwRtsGpioPin != DDK_IOMUX_INVALID_PIN))
	{
		DDKGpioWriteDataPin((DDK_IOMUX_PIN)pSerHead->dwRtsGpioPin, 1);		// DOUT -> High, active low
	}
}

//
// @doc OEM
// @func VOID | SL_SetRTS | This routine sets RTS.
// 
// @rdesc None.
//
VOID SL_SetRTS( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    PSER_INFO   pSerHead = (PSER_INFO)pHead;

	// CS&ZHL JUN-14-2012: setup RTS if(fRtsControl == RTS_CONTROL_TOGGLE)
	if((pSerHead->dcb.fRtsControl == RTS_CONTROL_TOGGLE) && (pSerHead->dwRtsGpioPin != DDK_IOMUX_INVALID_PIN))
	{
		DDKGpioWriteDataPin((DDK_IOMUX_PIN)pSerHead->dwRtsGpioPin, 0);		// DOUT -> Low, active low
	}
}

//
// @doc OEM
// @func VOID | SL_ClearBreak | This routine clears break.
// 
// @rdesc None.
// 
VOID SL_ClearBreak( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    PSER_INFO   pSerHead = (PSER_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_ClearBreak, 0x%X\r\n"), pHead));

    EnterCriticalSection(&(pSerHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
		pSerHead->UartHt45.UCR1 &= ~HT45B0F_UCR1_TX_BREAK;
        OUTB(pSerHead, HT45B0F_REG_UCR1, pSerHead->UartHt45.UCR1);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pSerHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_ClearBreak, 0x%X\r\n"), pHead));
}

//
// @doc OEM
// @func VOID | SL_SetBreak | This routine sets break.
// 
// @rdesc None.
//
VOID SL_SetBreak( PVOID pHead )			// @parm PVOID returned by HWinit.
{
    PSER_INFO   pSerHead = (PSER_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetBreak, 0x%X\r\n"), pHead));

    EnterCriticalSection(&(pSerHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
		pSerHead->UartHt45.UCR1 |= HT45B0F_UCR1_TX_BREAK;
        OUTB(pSerHead, HT45B0F_REG_UCR1, pSerHead->UartHt45.UCR1);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pSerHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetBreak, 0x%X\r\n"), pHead));
}

//
// SetUARTBaudRate
//
// Internal function.  The only real reason for splitting this out
// is so that we can call it from PowerOn and still allow SL_SetBaud
// to do debug messages, acquire critical sections, etc.
//
// @parm     PVOID returned by HWInit
// @parm     ULONG representing decimal baud rate.
BOOL SetUARTBaudRate( PVOID pHead, ULONG BaudRate )
{
    PSER_INFO      pSerHead = (PSER_INFO)pHead;
    USHORT         divisor;

    // **** Warning ***** Make no system calls, called in power context
    divisor = DivisorOfBaudRate(pHead, BaudRate);

    if ( divisor ) 
	{
		pSerHead->UartHt45.BRG = (BYTE)((divisor & 0xFF) - 1);
		if((divisor >> 8) & 0xFF)
		{
			pSerHead->UartHt45.UCR2 |= HT45B0F_UCR2_BRGH;
		}
		else
		{
			pSerHead->UartHt45.UCR2 &= ~HT45B0F_UCR2_BRGH;
		}
		InterruptMask(pSerHead->dwSysIntr, TRUE);
		OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
		OUTB(pSerHead, HT45B0F_REG_BRG, pSerHead->UartHt45.BRG);
		InterruptMask(pSerHead->dwSysIntr, FALSE);
		return( TRUE );
    } 
	else 
        return( FALSE );
}

//
// @doc OEM
// @func BOOL | SL_SetBaudRate |
//  This routine sets the baud rate of the device.
//
// @rdesc None.
//
// @parm     PVOID returned by HWInit
// @parm     ULONG representing decimal baud rate.
BOOL SL_SetBaudRate( PVOID pHead, ULONG BaudRate )   
{
    BOOL		fRet;
    PSER_INFO	pSerHead = (PSER_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetbaudRate 0x%X, x%X\r\n"), pHead, BaudRate));

    try {
        // Enter critical section before calling function, since
        // we can't make sys calls inside SetUARTBaudRate
        EnterCriticalSection(&(pSerHead->RegCritSec));
        fRet = SetUARTBaudRate(pHead, BaudRate);
        LeaveCriticalSection(&(pSerHead->RegCritSec));
    }except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        return( FALSE );
    }

    if ( fRet ) 
	{
        pSerHead->dcb.BaudRate = BaudRate;

        DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetbaudRate 0x%X (%d Baud)\r\n"), pHead, BaudRate));
        return( TRUE );
    } 
	else 
	{
        DEBUGMSG (ZONE_FUNCTION | ZONE_ERROR, 
			     (TEXT("-SL_SetbaudRate - Error setting %d, failing to %d\r\n"),
                 BaudRate, pSerHead->dcb.BaudRate) );
        return( FALSE );
    }
}

//
// @doc OEM
// @func BOOL | SL_SetByteSize |
//  This routine sets the WordSize of the device.
//
// @rdesc None.
//
// @parm     PVOID returned by HWInit
// @parm     ULONG ByteSize field from DCB.
BOOL SL_SetByteSize( PVOID pHead, ULONG ByteSize )
{
    PSER_INFO	pSerHead = (PSER_INFO)pHead;
    BOOL		bRet;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetByteSize 0x%X, x%X\r\n"), pHead, ByteSize));

    EnterCriticalSection(&(pSerHead->RegCritSec));
    try {
        switch ( ByteSize ) 
		{
        case 8:		// only 8-bit is available
			pSerHead->UartHt45.UCR1 &= ~HT45B0F_UCR1_DATABIT_MASK;
			pSerHead->UartHt45.UCR1 |= HT45B0F_UCR1_DATABIT_8;
			bRet = TRUE;
            break;

		default:
            bRet = FALSE;
            break;
        }
        if (bRet) {
			OUTB(pSerHead, HT45B0F_REG_UCR1, pSerHead->UartHt45.UCR1);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
    }
    LeaveCriticalSection(&(pSerHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetByteSize 0x%X\r\n"), pHead));

    return(bRet);
}

//
// @doc OEM
// @func BOOL | SL_SetParity |
//  This routine sets the parity of the device.
//
// @rdesc None.
//
// @parm     PVOID returned by HWInit
// @parm     ULONG parity field from DCB.
BOOL SL_SetParity( PVOID pHead, ULONG Parity )
{
    PSER_INFO   pSerHead = (PSER_INFO)pHead;
    BOOL		bRet;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetParity 0x%X, x%X\r\n"), pHead, Parity));

    bRet = TRUE;

    EnterCriticalSection(&(pSerHead->RegCritSec));
    try {
        switch ( Parity )
		{
        case ODDPARITY:
			pSerHead->UartHt45.UCR1 &= ~HT45B0F_UCR1_PARITY_MASK;
			pSerHead->UartHt45.UCR1 |= (HT45B0F_UCR1_PARITY_EN | HT45B0F_UCR1_PARITY_ODD);
            break;

        case EVENPARITY:
			pSerHead->UartHt45.UCR1 &= ~HT45B0F_UCR1_PARITY_MASK;
			pSerHead->UartHt45.UCR1 |= (HT45B0F_UCR1_PARITY_EN | HT45B0F_UCR1_PARITY_EVEN);
            break;

        case MARKPARITY:
			// markparity => 9-bit with TX8 = 1
			pSerHead->UartHt45.UCR1 &= ~(HT45B0F_UCR1_PARITY_EN | HT45B0F_UCR1_DATABIT_MASK | HT45B0F_UCR1_TX8);
			pSerHead->UartHt45.UCR1 |= (HT45B0F_UCR1_DATABIT_9 | HT45B0F_UCR1_TX8);
            break;

        case SPACEPARITY:
			// spaceparity => 9-bit with TX8 = 0
			pSerHead->UartHt45.UCR1 &= ~(HT45B0F_UCR1_PARITY_EN | HT45B0F_UCR1_DATABIT_MASK | HT45B0F_UCR1_TX8);
			pSerHead->UartHt45.UCR1 |= (HT45B0F_UCR1_DATABIT_9);
            break;

        case NOPARITY:
			// resume 8-bit without parity
			pSerHead->UartHt45.UCR1 &= ~(HT45B0F_UCR1_PARITY_EN | HT45B0F_UCR1_DATABIT_MASK);
			pSerHead->UartHt45.UCR1 |= HT45B0F_UCR1_DATABIT_8;
            break;

		default:
            bRet = FALSE;
            break;
        }
        if (bRet) {
			OUTB(pSerHead, HT45B0F_REG_UCR1, pSerHead->UartHt45.UCR1);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
    }
    LeaveCriticalSection(&(pSerHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetParity 0x%X\r\n"), pHead));

    return(bRet);
}
//
// @doc OEM
// @func VOID | SL_SetStopBits |
//  This routine sets the Stop Bits for the device.
//
// @rdesc None.
//
// @parm     PVOID returned by HWInit
// @parm     ULONG StopBits field from DCB.
BOOL SL_SetStopBits( PVOID pHead, ULONG StopBits )
{
    PSER_INFO   pSerHead = (PSER_INFO)pHead;
    BOOL		bRet;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetStopBits 0x%X, x%X\r\n"), pHead, StopBits));

    bRet = TRUE;

    EnterCriticalSection(&(pSerHead->RegCritSec));
    try {
        switch ( StopBits ) 
		{
        case ONESTOPBIT:
			pSerHead->UartHt45.UCR1 &= ~HT45B0F_UCR1_STOPBIT_MASK;
			pSerHead->UartHt45.UCR1 |= HT45B0F_UCR1_STOPBIT_1;
            break;

        case TWOSTOPBITS:
			pSerHead->UartHt45.UCR1 &= ~HT45B0F_UCR1_STOPBIT_MASK;
			pSerHead->UartHt45.UCR1 |= HT45B0F_UCR1_STOPBIT_2;
            break;

		case ONE5STOPBITS:		// 1.5 stop bits not supported
        default:
            bRet = FALSE;
            break;
        }

        if (bRet) {
			OUTB(pSerHead, HT45B0F_REG_UCR1, pSerHead->UartHt45.UCR1);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        bRet = FALSE;
    }

    LeaveCriticalSection(&(pSerHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetStopBits 0x%X\r\n"), pHead));

    return(bRet);
}

//
// @doc OEM
// @func ULONG | SL_GetRxBufferSize | This function returns
// the size of the hardware buffer passed to the interrupt
// initialize function.  It would be used only for devices
// which share a buffer between the MDD/PDD and an ISR.
//
// 
// @rdesc This routine always returns 0 for 16550 UARTS.
// 
ULONG SL_GetRxBufferSize( PVOID pHead )
{
    UNREFERENCED_PARAMETER(pHead);

	return(0);
}

//
// @doc OEM
// @func PVOID | SC_GetRxStart | This routine returns the start of the hardware
// receive buffer.  See SL_GetRxBufferSize.
// 
// @rdesc The return value is a pointer to the start of the device receive buffer.
// 
PVOID SL_GetRxStart( PVOID pHead )			// @parm PVOID returned by SC_init.
{
    UNREFERENCED_PARAMETER(pHead);

	return(NULL);
}

//
// @doc OEM
// @func ULONG | SL_GetGetInterruptType | This function is called
//   by the MDD whenever an interrupt occurs.  The return code
//   is then checked by the MDD to determine which of the four
//   interrupt handling routines are to be called.
// 
// @rdesc This routine returns a bitmask indicating which interrupts
//   are currently pending.
// 
INTERRUPT_TYPE SL_GetInterruptType( PVOID pHead )			// Pointer to hardware head
{
    PSER_INFO		pSerHead = (PSER_INFO)pHead;
    INTERRUPT_TYPE	interrupts = INTR_NONE;
	BYTE			uIrqFlags;

    DEBUGMSG (0, (TEXT("+SL_GetInterruptType 0x%X\r\n"), pHead));
	uIrqFlags = pSerHead->UartHt45.UCR2 & (HT45B0F_UCR2_RX_INTEN | HT45B0F_UCR2_TIDLE_INTEN | HT45B0F_UCR2_TXIF_INTEN);
    //RETAILMSG (1, (TEXT("SL_GetInterruptType IrqFlag = 0x%X\r\n"), uIrqFlags));

	try {
        pSerHead->UartHt45.USR = INB(pSerHead, HT45B0F_REG_USR);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
        pSerHead->UartHt45.USR &= ~uIrqFlags;	// simulate no interrupt
    }
    //RETAILMSG(1, (TEXT("SL_GetInterruptType USR=0x%x\r\n"), pSerHead->UartHt45.USR));

    if ( !(pSerHead->UartHt45.USR & uIrqFlags) ) 
	{
        // No interrupts pending, vector is useless
        interrupts = INTR_NONE;
		// CS&ZHL SEP-3-2011: clear GPIO interrupt flags
		DDKGpioClearIntrPin((DDK_IOMUX_PIN)pSerHead->dwIrqGpioPin);
    } 
	else 
	{
        // The interrupt value is valid
		if(pSerHead->UartHt45.USR & HT45B0F_USR_RXIF)
		{	//Rx Data Register has available data 
            interrupts |= INTR_RX;
		}

		if(pSerHead->UartHt45.USR & (HT45B0F_USR_PERR | HT45B0F_USR_FERR | HT45B0F_USR_OERR))
		{	// some error encounted
            interrupts |= INTR_LINE;
		}

		if(pSerHead->UartHt45.USR & (HT45B0F_USR_TIDLE | HT45B0F_USR_TXIF))
		{	// THR empty
			//RETAILMSG(1, (TEXT("SL_GetInterruptType INTR_TX found\r\n")));
            interrupts |= INTR_TX;
		}
    }

    if (pSerHead->AddTXIntr) 
	{
        interrupts |= INTR_TX;
        pSerHead->AddTXIntr = FALSE;
    }
    DEBUGMSG (ZONE_THREAD, (TEXT("-SL_GetInterruptType 0x%X, 0x%X\r\n"), pHead, interrupts));

    return(interrupts);
}


// @doc OEM
// @func ULONG | SL_RxIntr | This routine gets several characters from the hardware
//   receive buffer and puts them in a buffer provided via the second argument.
//   It returns the number of bytes lost to overrun.
// 
// @rdesc The return value indicates the number of overruns detected.
//   The actual number of dropped characters may be higher.
//
// @parm Pointer to hardware head
// @parm Pointer to receive buffer
// @parm In = max bytes to read, out = bytes read
ULONG SL_RxIntr( PVOID pHead, PUCHAR pRxBuffer, ULONG *pBufflen )
{
    PSER_INFO	pSerHead = (PSER_INFO)pHead;
    ULONG       RetVal  = 0;
    ULONG       TargetRoom = *pBufflen;
    BOOL        fRXFlag = FALSE;
    BOOL        fReplaceparityErrors = FALSE;
    BOOL        fNull;
    UCHAR       cEvtChar, cRXChar;

    *pBufflen = 0;

    // LAM - I have local copies of some DCB elements since I don't
    // want to keep dereferencing inside my read loop and there are too
    // many of them to trust the compiler.
    cEvtChar = pSerHead->dcb.EvtChar;
    fNull = pSerHead->dcb.fNull;
    if ( pSerHead->dcb.fErrorChar && pSerHead->dcb.fParity )
        fReplaceparityErrors = TRUE;

#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        while ( TargetRoom ) 
		{
            // See if there is another byte to be read
			pSerHead->UartHt45.USR = INB(pSerHead, HT45B0F_REG_USR);
            if ( pSerHead->UartHt45.USR & HT45B0F_USR_RXIF ) 
			{
                // Read the byte
                cRXChar = INB(pSerHead, HT45B0F_REG_DAT);
            }
            else
                break;

            // But we may want to discard it
			if (!cRXChar && fNull) 
			{
                // Do nothing - byte gets discarded
                DEBUGMSG (ZONE_FLOW| ZONE_WARN, (TEXT("Dropping NULL byte due to fNull\r\n")));
            } 
			else 
			{
                // Do character replacement if parity error detected.
                if ( fReplaceparityErrors && (pSerHead->UartHt45.USR & HT45B0F_USR_PERR) ) 
				{
                    cRXChar = pSerHead->dcb.ErrorChar;
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
        pSerHead->EventCallback( pSerHead->pMddHead, EV_RXFLAG );

    if ( pSerHead->DroppedBytes )
        DEBUGMSG (ZONE_WARN, (TEXT("Rx drop %d.\r\n"), pSerHead->DroppedBytes));

    DEBUGMSG (ZONE_READ, (TEXT("-GetBytes - rx'ed %d, dropped %d.\r\n"),
              *pBufflen, pHWHead->DroppedBytes));

    RetVal = pSerHead->DroppedBytes;
    pSerHead->DroppedBytes = 0;
    return(RetVal);
}


//
// @doc OEM
// @func ULONG | SL_TXIntrEx | This routine is called from the new MDD
//   whenever INTR_TX is returned by SL_GetInterruptType
// 
// @rdesc None
//
// @parm	Hardware Head
// @parm	Pointer to transmit buffer
// @parm	In = max bytes to transmit, out = bytes transmitted
VOID SL_TxIntrEx( PVOID pHead, PUCHAR pTxBuffer, ULONG *pBufflen )            
{
    PSER_INFO   pSerHead = (PSER_INFO)pHead;
    ULONG		NumberOfBytes = *pBufflen;

    DEBUGMSG (ZONE_THREAD, (TEXT("Transmit Event\r\n")));
    //DEBUGMSG (ZONE_WRITE, (TEXT("+SL_TxIntrEx 0x%X, Len %d\r\n"), pHead, *pBufflen));
    //RETAILMSG (1, (TEXT("+SL_TxIntrEx 0x%X, Len %d\r\n"), pHead, *pBufflen));

    // We may be done sending.  If so, just disable the TX interrupts and return to the MDD.  
    if( ! *pBufflen ) 
	{
        DEBUGMSG (ZONE_WRITE, (TEXT("SL_TxIntrEx: Disable INTR_TX.\r\n")));
		//
		// CS&ZHL JUN-14-2012: clear RTS if(fRtsControl == RTS_CONTROL_TOGGLE)
		//
		if((pSerHead->dcb.fRtsControl == RTS_CONTROL_TOGGLE) && (pSerHead->dwRtsGpioPin != DDK_IOMUX_INVALID_PIN))
		{
			DWORD	dwDelayMilliseconds;
			DWORD	dwCount;
			DWORD	dwBitSize;
			
			// CS&ZHL APR-24-2012: compute the number of bits for each data
			dwBitSize = 1 + pSerHead->dcb.ByteSize;		// "1" -> start bit
			if(pSerHead->dcb.Parity != NOPARITY)
			{
				dwBitSize++;							// add parity bit
			}

			if(pSerHead->dcb.StopBits == ONESTOPBIT)
			{
				dwBitSize++;							// add 1 stop bit
			}
			else
			{
				dwBitSize += 2;							// add 2 stop bits
			}

			// CS&ZHL APR-24-2012: compute milliseconds need to delay
			dwDelayMilliseconds = (dwBitSize * 1000) / pSerHead->dcb.BaudRate;
			if(dwDelayMilliseconds > 2)
			{
				Sleep(dwDelayMilliseconds - 2);
			}

			// CS&ZHL APR-24-2012: use polling to switch clear RTS immediately after data transmit completed
			dwCount = GetTickCount(); 
			while((GetTickCount() - dwCount) <= 10)		// timeout check
			{
				pSerHead->UartHt45.USR = INB(pSerHead, HT45B0F_REG_USR);
				// on both Tx FIFO is empty & the last byte is transmitted
				if((pSerHead->UartHt45.USR & (HT45B0F_USR_TXIF | HT45B0F_USR_TIDLE)) == (HT45B0F_USR_TXIF | HT45B0F_USR_TIDLE))
				{
					break;
				}
			} 

			DDKGpioWriteDataPin((DDK_IOMUX_PIN)pSerHead->dwRtsGpioPin, 1);		// DOUT -> High, active low
		}

		pSerHead->UartHt45.UCR2 &= ~(HT45B0F_UCR2_TIDLE_INTEN | HT45B0F_UCR2_TXIF_INTEN);	//disable TX interrupt
        OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
        return;
    }
        
    *pBufflen = 0;  // In case we don't send anything below.
    
    // Disable xmit intr.  Most 16550s will keep hammering
    // us with xmit interrupts if we don't turn them off
    // Whoever gets the FlushDone will then need to turn
    // TX Ints back on if needed.
    EnterCriticalSection(&(pSerHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        // Need to signal FlushDone for XmitComChar
        PulseEvent(pSerHead->FlushDone);

        pSerHead->CommErrors &= ~CE_TXFULL;
		// hardware flow control is NOT supported in HT45B0F 
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Do nothing.  The worst case is that this was a fluke,
        // and a TX Intr will come right back at us and we will
        // resume transmission.
    }
#pragma prefast(pop)

    LeaveCriticalSection(&(pSerHead->RegCritSec));

    //  OK, now lets actually transmit some data.
	//RETAILMSG (1, (TEXT("SL_TxIntrEx::OK, now lets actually transmit some data.\r\n")));

    DEBUGMSG (ZONE_WRITE, (TEXT("SL_TxIntrEx wait for CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));
    EnterCriticalSection(&(pSerHead->TransmitCritSec));
    DEBUGMSG (ZONE_WRITE, (TEXT("SL_TxIntrEx got CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));

    EnterCriticalSection(&(pSerHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
		while(NumberOfBytes)
		{
			// send a byte data if THR is empty
			pSerHead->UartHt45.USR = INB(pSerHead, HT45B0F_REG_USR);
			if (!(pSerHead->UartHt45.USR & HT45B0F_USR_TXIF)) 
			{
				break;
			}

			OUTB(pSerHead, HT45B0F_REG_DAT, *pTxBuffer);
			//
			// CS&ZHL JUN-25-2008: it seems unnecessary to issue EOI here??
			//
			//InterruptDone(pHWHead->dwSysIntr);			//Is this ok to inform kernel in advance?
			++pTxBuffer;
			(*pBufflen)++;
			NumberOfBytes--;
		}

        // Enable xmit intr. We need to do this no matter what, 
        // since the MDD relies on one final interrupt before
        // returning to the application. 
        DEBUGMSG (ZONE_WRITE, (TEXT("SL_TxIntrEx: Enable INTR_TX.\r\n")));
		pSerHead->UartHt45.UCR2 |= (HT45B0F_UCR2_TIDLE_INTEN | HT45B0F_UCR2_TXIF_INTEN);	//enable TX interrupt
        OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Hmm, not sure what would cause this.  Lets just tell
        // the MDD to go away until we get another TX interrupt.
    }
#pragma prefast(pop)

    LeaveCriticalSection(&(pSerHead->RegCritSec));
    LeaveCriticalSection(&(pSerHead->TransmitCritSec));

    DEBUGMSG (ZONE_WRITE, (TEXT("SL_TxIntrEx released CritSec %x.\r\n"), &(pSerHead->TransmitCritSec)));
    //DEBUGMSG (ZONE_WRITE, (TEXT("-SL_TxIntrEx - sent %d.\r\n"), *pBufflen));
    //RETAILMSG (1, (TEXT("-SL_TxIntrEx - sent %d.\r\n"), *pBufflen));
    return;
}

//
// @doc OEM
// @func ULONG | SL_LineIntr | This routine is called from the MDD
//   whenever INTR_LINE is returned by SL_GetInterruptType.
// 
// @rdesc None
//
VOID SL_LineIntr( PVOID pHead )               // Hardware Head 
{
    PSER_INFO   pSerHead = (PSER_INFO)pHead;
	DWORD		i;

    DEBUGMSG (ZONE_READ, (TEXT("+SL_LineIntr 0x%X\r\n"), pHead));
    ReadLSR( pSerHead );
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
		// clear Rx FIFO because it is error 
		for(i = 0; i < 4; i++)			// HT45B0F Rx FIFO deepth = 4 
		{
			pSerHead->UartHt45.USR = INB(pSerHead, HT45B0F_REG_USR);
            if ( !(pSerHead->UartHt45.USR & HT45B0F_USR_RXIF) ) 
			{
				break;
			}
			// dummy Read the byte
			INB(pSerHead, HT45B0F_REG_DAT);
		}
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
	{
        // nothing to do.
    }
#pragma prefast(pop)
    DEBUGMSG (ZONE_READ, (TEXT("-SL_LineIntr 0x%X\r\n"), pHead));
}

//
// @doc OEM
// @func ULONG | SL_OtherIntr | This routine is called from the MDD
//   whenever INTR_MODEM is returned by SL_GetInterruptType.
// 
// @rdesc None
//
VOID SL_OtherIntr( PVOID pHead )                // Hardware Head
{
    UNREFERENCED_PARAMETER(pHead);
	// modem lines is not supported in HT45B0F
}


//
// @doc OEM
// @func ULONG | SL_OtherIntr | This routine is called from the MDD
//   whenever INTR_MODEM is returned by SL_GetInterruptType.
// 
// @rdesc None
//
VOID SL_ModemIntr( PVOID pHead )                // Hardware Head
{
    SL_OtherIntr(pHead);
}

//  
// @doc OEM
// @func    ULONG | SL_GetStatus | This structure is called by the MDD
//   to retrieve the contents of a COMSTAT structure.
//
// @rdesc    The return is a ULONG, representing success (0) or failure (-1).
//
// @parm PVOID returned by HWInit.
// @parm Pointer to LPCOMMSTAT to hold status.
ULONG SL_GetStatus( PVOID pHead, LPCOMSTAT lpStat )    
{
    PSER_INFO	pSerHead = (PSER_INFO)pHead;
    ULONG		RetVal  = pSerHead->CommErrors;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_GetStatus 0x%X\r\n"), pHead));

    pSerHead->CommErrors = 0; // Clear old errors each time

    if ( lpStat ) {
        try {
            if (pSerHead->CTSFlowOff)
                pSerHead->Status.fCtsHold = 1;
            else
                pSerHead->Status.fCtsHold = 0;

            if (pSerHead->DSRFlowOff)
                pSerHead->Status.fDsrHold = 1;
            else
                pSerHead->Status.fDsrHold = 0;

            // NOTE - I think what they really want to know here is
            // the amount of data in the MDD buffer, not the amount
            // in the UART itself.  Just set to 0 for now since the
            // MDD doesn't take care of this.
            pSerHead->Status.cbInQue  = 0;
            pSerHead->Status.cbOutQue = 0;

            memcpy(lpStat, &(pSerHead->Status), sizeof(COMSTAT));
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            RetVal = (ULONG)-1;
        }        
    } else
        RetVal = (ULONG)-1;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_GetStatus 0x%X\r\n"), pHead));
    return(RetVal);
}

//
// @doc OEM
// @func    ULONG | SL_Reset | Perform any operations associated
//   with a device reset
//
// @rdesc    None.
//
VOID SL_Reset( PVOID pHead )		    // @parm PVOID returned by HWInit.
{
    PSER_INFO pSerHead = (PSER_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_Reset 0x%X\r\n"), pHead));

    memset(&pSerHead->Status, 0, sizeof(COMSTAT));

    EnterCriticalSection(&(pSerHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
        //OUTB(pHWHead, pIER, IER_NORMAL_INTS);
		// enable RX interrupt
		pSerHead->UartHt45.UCR2 &= ~(HT45B0F_UCR2_TIDLE_INTEN | HT45B0F_UCR2_TXIF_INTEN);	//disable TX interrupt
		pSerHead->UartHt45.UCR2 |= HT45B0F_UCR2_RX_INTEN;									//enable RX interrupt
        OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Do nothing
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pSerHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_Reset 0x%X\r\n"), pHead));
}

//
// @doc OEM
// @func    VOID | SL_GetModemStatus | Retrieves modem status.
//
// @rdesc    None.
//
// @parm PVOID returned by HWInit.
// @parm PULONG passed in by user.
VOID SL_GetModemStatus( PVOID pHead, PULONG pModemStatus )    
{
    UNREFERENCED_PARAMETER(pHead);

	/*
    if ( ubModemStatus & SERIAL_MSR_CTS )
        *pModemStatus |= MS_CTS_ON;

    if ( ubModemStatus & SERIAL_MSR_DSR )
        *pModemStatus |= MS_DSR_ON;

    if ( ubModemStatus & SERIAL_MSR_RI )
        *pModemStatus |= MS_RING_ON;

    if ( ubModemStatus & SERIAL_MSR_DCD )
        *pModemStatus |= MS_RLSD_ON;
	*/

	// modem lines is not supported in HT45B0F, so give a simulate result:
    *pModemStatus |= (MS_CTS_ON | MS_DSR_ON | MS_RLSD_ON);
}

//
// @doc OEM
// @func    VOID | SL_PurgeComm | Purge RX and/or TX
// 
// @rdesc    None.
//
// @parm PVOID returned by HWInit.
// @parm Action to take. 
VOID SL_PurgeComm( PVOID pHead, DWORD fdwAction )        
{
    PSER_INFO	pSerHead = (PSER_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_PurgeComm 0x%X\r\n"), pHead));

    EnterCriticalSection(&(pSerHead->RegCritSec));
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    try {
#ifdef TODO
        // REVIEW THIS - I don't see how this could have terminated a pending read,
        // nor how RX interrupts would ever get turned back on.  I suspect that
        // RXABORT and TXABORT would both be better implemented in the MDD.
        if ( fdwAction & PURGE_RXABORT )
		{
            //OUTB(pHWHead, pIER, IER_NORMAL_INTS & ~SERIAL_IER_RDA);
			pSerHead->UartHt45.UCR2 |= HT45B0F_UCR2_RX_INTEN;									//enable RX interrupt
			OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
		}
#endif    
        if ( fdwAction & PURGE_TXCLEAR ) 
		{
            // Write the TX reset bit.  It is self clearing
            //OUTB(pHWHead, pIIR_FCR, pHWHead->FCR | SERIAL_FCR_TXMT_RESET);
			pSerHead->UartHt45.UCR2 &= ~HT45B0F_UCR2_TXEN;									//TX machine disable which clear TX buffer, abort transmission
			OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
			Sleep(0);
			pSerHead->UartHt45.UCR2 |= HT45B0F_UCR2_TXEN;									//TX machine enable again
			OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
        }

        if ( fdwAction & PURGE_RXCLEAR ) {
            // Write the RX reset bit.  It is self clearing
            //OUTB(pHWHead, pIIR_FCR, pHWHead->FCR | SERIAL_FCR_RCVR_RESET);
			pSerHead->UartHt45.UCR2 &= ~HT45B0F_UCR2_RXEN;									//RX machine disable which clear RX buffer, abort transmission
			OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
			Sleep(0);
			pSerHead->UartHt45.UCR2 |= HT45B0F_UCR2_RXEN;									//RX machine enable again
			OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just exit
    }
#pragma prefast(pop)
    LeaveCriticalSection(&(pSerHead->RegCritSec));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_PurgeComm 0x%X\r\n"), pHead));
    return;
}

//
// @doc OEM
// @func    BOOL | SL_XmitComChar | Transmit a char immediately
// 
// @rdesc    TRUE if succesful
//
// @parm PVOID returned by HWInit.
// @parm Character to transmit. 
BOOL SL_XmitComChar( PVOID pHead, UCHAR ComChar )   
{
    PSER_INFO	pSerHead = (PSER_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_XmitComChar 0x%X\r\n"), pHead));

    // Get critical section, then transmit when buffer empties
    DEBUGMSG (ZONE_WRITE, (TEXT("XmitComChar wait for CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));
    EnterCriticalSection(&(pSerHead->TransmitCritSec));
    DEBUGMSG (ZONE_WRITE, (TEXT("XmitComChar got CritSec %x.\r\n"), &(pHWHead->TransmitCritSec)));
    try {
        //while( TRUE ) {  // We know THR will eventually empty
        for( ; ; ) {  // We know THR will eventually empty
            EnterCriticalSection(&(pSerHead->RegCritSec));
            // Write the character if we can
            ReadLSR( pSerHead );
            if ( pSerHead->UartHt45.USR & HT45B0F_USR_TXIF ) 
			{
                // FIFO is empty, send this character
                OUTB(pSerHead, HT45B0F_REG_DAT, ComChar);
                // Make sure we release the register critical section
				pSerHead->UartHt45.UCR2 |= (HT45B0F_UCR2_RX_INTEN | HT45B0F_UCR2_TIDLE_INTEN | HT45B0F_UCR2_TXIF_INTEN);	//enable RX/TX interrupt
				OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
                LeaveCriticalSection(&(pSerHead->RegCritSec));
               
                DEBUGMSG (ZONE_WRITE, (TEXT("XmitComChar wrote x%X\r\n"), ComChar));
                break;
            }

			// If we couldn't write the data yet, then wait for a
            // TXINTR to come in and try it again.

            // Enable xmit intr.
            //OUTB(pHWHead, pIER, IER_NORMAL_INTS | SERIAL_IER_THR);
			pSerHead->UartHt45.UCR2 |= (HT45B0F_UCR2_RX_INTEN | HT45B0F_UCR2_TIDLE_INTEN | HT45B0F_UCR2_TXIF_INTEN);	//enable RX/TX interrupt
			OUTB(pSerHead, HT45B0F_REG_UCR2, pSerHead->UartHt45.UCR2);
            LeaveCriticalSection(&(pSerHead->RegCritSec));

            // Wait until the txintr has signalled.
            DEBUGMSG (ZONE_WRITE, (TEXT("XmitComChar WaitIntr x%X\r\n"), pSerHead->FlushDone));
            WaitForSingleObject(pSerHead->FlushDone, (ULONG)1000);
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Make sure we release the register critical section
        LeaveCriticalSection(&(pSerHead->RegCritSec));
    }

    LeaveCriticalSection(&(pSerHead->TransmitCritSec));
    DEBUGMSG (ZONE_WRITE, (TEXT("XmitComChar released CritSec %x.\r\n"), &(pSerHead->TransmitCritSec)));
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_XmitComChar 0x%X\r\n"), pHead));
    return(TRUE);
}

//
// @doc OEM
// @func    BOOL | SL_PowerOff | Perform powerdown sequence.
// 
// @rdesc    TRUE if succesful
//
VOID
SL_PowerOff( PVOID pHead )       // @parm    PVOID returned by HWInit.
{
    UNREFERENCED_PARAMETER(pHead);
	// nothing to do as other UARTs
}

//
// @doc OEM
// @func    BOOL | SL_PowerOn | Perform poweron sequence.
// 
// @rdesc    TRUE if succesful
//
VOID SL_PowerOn( PVOID pHead )        // @parm    PVOID returned by HWInit.
{
    UNREFERENCED_PARAMETER(pHead);
	// nothing to do as other UARTs
}

//
// @doc OEM
// @func    BOOL | SL_SetDCB | Sets new values for DCB.  This
// routine gets a DCB from the MDD.  It must then compare
// this to the current DCB, and if any fields have changed take
// appropriate action.
// 
// @rdesc    BOOL
//
// @parm    PVOID returned by HWInit.
// @parm    Pointer to DCB structure
BOOL SL_SetDCB( PVOID pHead, LPDCB lpDCB )       
{
    PSER_INFO	pSerHead = (PSER_INFO)pHead;
    BOOL		bRet;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetDCB 0x%X\r\n"), pHead));

    bRet = TRUE;

    // If the device is open, scan for changes and do whatever
    // is needed for the changed fields.  if the device isn't
    // open yet, just save the DCB for later use by the open.
    if ( pSerHead->OpenCount ) 
	{
        // Note, fparity just says whether we should check
        // receive parity.  And the 16550 won't let us NOT
        // check parity if we generate it.  So this field
        // has no effect on the hardware.

        if ( lpDCB->BaudRate != pSerHead->dcb.BaudRate ) 
            bRet = SL_SetBaudRate( pHead, lpDCB->BaudRate );
        
        if ( bRet && (lpDCB->ByteSize != pSerHead->dcb.ByteSize )) 
            bRet = SL_SetByteSize( pHead, lpDCB->ByteSize );

        if ( bRet && (lpDCB->Parity != pSerHead->dcb.Parity )) 
            bRet = SL_SetParity( pHead, lpDCB->Parity );

        if ( bRet && (lpDCB->StopBits != pSerHead->dcb.StopBits )) 
            bRet = SL_SetStopBits( pHead, lpDCB->StopBits );

        // Don't worry about fOutxCtsFlow.  It is a flag which
        // will be examined every time we load the TX buffer.
        // No special action required here.
    }

    if (bRet)
	{
		//pSerHead->dcb = *lpDCB;		// Now that we have done the right thing, store this DCB
		memcpy(&pSerHead->dcb, lpDCB, sizeof(DCB));

		// CS&ZHL JUN-14-2012: setup RTS if(fRtsControl == RTS_CONTROL_TOGGLE)
		if((pSerHead->dcb.fRtsControl == RTS_CONTROL_TOGGLE) && (pSerHead->dwRtsGpioPin != DDK_IOMUX_INVALID_PIN))
		{
			DDKIomuxSetPinMux((DDK_IOMUX_PIN)pSerHead->dwRtsGpioPin, DDK_IOMUX_MODE_GPIO);	// config as GPIO
			DDKGpioWriteDataPin((DDK_IOMUX_PIN)pSerHead->dwRtsGpioPin, 1);					// DOUT -> High
			DDKGpioEnableDataPin((DDK_IOMUX_PIN)pSerHead->dwRtsGpioPin, 1);					// output enable
		}
	}

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetDCB 0x%X\r\n"), pHead));

    return(bRet);
}

//
// @doc OEM
// @func    BOOL | SL_SetCommTimeouts | Sets new values for the
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
    PSER_INFO	pSerHead = (PSER_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetCommTimeout 0x%X\r\n"), pHead));

    // OK, first check for any changes and act upon them
    if ( lpCommTimeouts->WriteTotalTimeoutMultiplier !=
         pSerHead->CommTimeouts.WriteTotalTimeoutMultiplier ) 
	{		// for what action?
    }

    // Now that we have done the right thing, store this DCB
    //pHWHead->CommTimeouts = *lpCommTimeouts;
	memcpy(&pSerHead->CommTimeouts, lpCommTimeouts, sizeof(COMMTIMEOUTS));

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetCommTimeout 0x%X\r\n"), pHead));

    return(TRUE);
}



//
//  @doc OEM
//  @func    BOOL | SL_Ioctl | Device IO control routine.  
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
BOOL SL_Ioctl(PVOID pHead, DWORD dwCode,PBYTE pBufIn,DWORD dwLenIn,
               PBYTE pBufOut,DWORD dwLenOut,PDWORD pdwActualOut)
{
    BOOL RetVal = TRUE;

    UNREFERENCED_PARAMETER(pHead);
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pdwActualOut);

	DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_Ioctl 0x%X\r\n"), pHead));

	switch (dwCode) 
	{
	// CS&ZHL JUN-14-2012: support GPIO as RTS direction-control if(dcb.fRtsControl == RTS_CONTROL_TOGGLE)
	case IOCTL_SET_UART_RTS_PIN:
		{
			PSER_INFO	pSerHead = (PSER_INFO)pHead;
			DWORD		dwEM9280_GPIO;

			if(!pBufIn || (dwLenIn != sizeof(DWORD)))
			{
				RETAILMSG(1, (TEXT("SL_Ioctl::IOCTL_SET_UART_RTS_PIN: inavlid parameters\r\n")));
				RetVal = FALSE;
				break;
			}

			dwEM9280_GPIO = *((DWORD*)pBufIn);
			switch(dwEM9280_GPIO)
			{
			case GPIO6:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO3_22;
				break;

			case GPIO7:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO3_23;
				break;

			case GPIO20:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO3_28;
				break;

			case GPIO21:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO3_29;
				break;

			case GPIO22:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO3_25;
				break;

			case GPIO23:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO3_24;
				break;

			case GPIO24:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO2_3;
				break;

			case GPIO25:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO2_20;
				break;

			case GPIO26:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO3_30;
				break;

			case GPIO27:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO4_20;
				break;

			case GPIO28:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO2_4;
				break;

			case GPIO29:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO2_6;
				break;

			case GPIO30:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO2_7;
				break;

			case GPIO31:
				pSerHead->dwRtsGpioPin = DDK_IOMUX_GPIO2_5;
				break;

			default:
				RETAILMSG(1, (TEXT("SL_Ioctl::IOCTL_SET_UART_RTS_PIN: GPIO% is NOT supported as RTS!\r\n"), 
						(31 - _CountLeadingZeros(dwEM9280_GPIO))));
				pSerHead->dwRtsGpioPin = DDK_IOMUX_INVALID_PIN;
				RetVal = FALSE;
			}

			if(RetVal)
			{
				RETAILMSG(1, (TEXT("SL_Ioctl: GPIO% is used as RTS of COM%\r\n"), 
						(31 - _CountLeadingZeros(dwEM9280_GPIO)), pSerHead->dwDeviceArrayIndex));
			}
		}
		break;

    // Currently, no defined IOCTLs
    default:
        RetVal = FALSE;
        DEBUGMSG (ZONE_FUNCTION, (TEXT(" Unsupported ioctl 0x%X\r\n"), dwCode));
        break;            
    }
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_Ioctl 0x%X\r\n"), pHead));
    return(RetVal);
}

// ----- end of ser_ht45b0f.c ----- //