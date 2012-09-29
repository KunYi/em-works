//**********************************************************************
//                                                                      
// Filename: em9260_com.h
//                                                                      
// Description: Holds definitions for AT91SAM9260 USART serial interface.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Use of this source code is subject to the terms of the Cirrus end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to 
// use this source code. For a copy of the EULA, please see the 
// EULA.RTF on your install media.
//
// Copyright(c) Emtronix 2007, All Rights Reserved                       
//                                                                      
//**********************************************************************


#ifndef __EM9X61_COM_H__   
#define __EM9X61_COM_H__

#ifdef __cplusplus
extern "C" {
#endif

// TODO - Define PAGE_CONTAINING_UART
#define PAGE_CONTAINING_UART   XXXXX

//default baud table
typedef struct __TRIPLES
{
	ULONG	Key;
	ULONG	AssociatedCD;
	ULONG	AssociatedFP;
} TRIPLES, *PTRIPLES;

typedef struct __EM9260_LOOKUP_TBL 
{
	ULONG		Size;
	PTRIPLES	Table;
} EM9260_LOOKUP_TBL, *PEM9260_LOOKUP_TBL;


/*
* @doc HWINTERNAL
* @struct SER_INFO | Private structure.
*/

typedef struct __SER_INFO {
    // hardware specific fields
	volatile AT91PS_PIO				pPIOAReg;			//!< Access to PIOA registers
	volatile PEM9161_ISA_INTERFACE	pISAReg;

	/*
	//volatile AT91PS_USART	pUSARTReg;		//!< Access to USART registers
	//volatile AT91PS_PDC		pPDCReg;		//!< Access to PDC registers
	
	UCHAR					LSR;			//@field simulated 16550 Line Status Register
	UCHAR					MSR;			//@field simulated 16550 Modem Status Register
	UCHAR					MSRatInt;		//@MSR state at interrupt happened
	ULONG                   CSR;			//@field shadow of US_CSR

	// DMA Buffer
	volatile PUCHAR			pBufferPool;	//@field Allocated DMA Buffer Pool - mapped+uncached
	DWORD				    dwPhyBufPool;	//
	DWORD					dwBufPoolSize;

	// Rx DMA Buffer
	volatile PUCHAR			pRxBufAddr[2];	//@field Satrts of Rx Pingpong Buffer - mapped+uncached
	DWORD					ulRxPhyBuf[2];	//@field Physical Satrts of Rx Pingpong Buffer
	DWORD					nRxBufSize;		//@field 
	DWORD					nRxCountLen;	//@field <=> 16550 RX_FIFO_TRIGGER_LEVEL
	int  					nRxBufIdx;		//@field = 0, 1 alternatively

	// Tx DMA Buffer
	volatile PUCHAR			pTxBuffer;		//@field Start of PDC Tx Buffer - mapped+uncached
	DWORD					ulTxPhyBuf;		//@field Physical Start of PDC Tx Buffer
	DWORD					nTxBufSize;		//@field Size of Tx Buffer 
	DWORD                   nTxCountLen;	//@field <=> 16550 SERIAL_FIFO_DEPTH = 16

	UCHAR                   nRxFIFO[64];
	DWORD                   dwDMA_PutFIFOLen;
	DWORD                   dwMDD_GetFIFOLen;
	*/

	//USART work clock = Master Clock (MCK) 
	DWORD					dwMasterClock;	//@field Master Clock in Hz

	DWORD			dwDeviceArrayIndex;		//@field COM logical index = 2, 3, 4, 5, 6
	DWORD			dwDeviceID;				//@field AT91SAM9260 CPU Peripherals ID		
	DWORD			dwUSARTIndex;			//@field USART index = 0 - 5 for SAM9260
											//@field USART index = 0 - 7 for 16C550 on ISA

	//
	// CS&ZHL JUN-27-2008: variances for UART on ISA bus
	//
    SER16550_INFO	ser16550;
	DWORD			dwExDeviceID;			// @field ISA UART ID
    PUCHAR			pBaseAddress;			// @field Start of serial registers - mapped
	DWORD			dwIrqInfoAddress;

    BOOL			fIRMode;				// @field Boolean, are we running in IR mode?

    // parameter fields
    COMMPROP			CommProp;			// @field Pointer to CommProp structure.
	EVENT_FUNC			EventCallback;		// This callback exists in MDD
    PVOID				pMddHead;			// @field First arg to mdd callbacks.

	DCB					dcb;				// @field Device Control Block (copy of DCB in MDD)
	COMMTIMEOUTS		CommTimeouts;		// @field copy of CommTimeouts structure
    ULONG				OpenCount;			// @field Count of concurrent opens
	PEM9260_LOOKUP_TBL	pBaudTable;			// @field pointer to baud table
	
	ULONG				DroppedBytes;		// @field Number of dropped bytes
	HANDLE              FlushDone;			// @field Handle to flush done event
	BOOL				CTSFlowOff;			// @field Flag - CTS flow control state.
	BOOL				DSRFlowOff;			// @field Flag - DSR flow control state.
	BOOL				AddTXIntr;			// @field Flag - Fake a TX interrupt
	COMSTAT				Status;				// @field Bitfield representing Win32 modem status
	ULONG				CommErrors;			// @field Bitfield representing Win32 comm error status
	ULONG				ModemStatus;		// @field Bitfield representing Win32 modem status

	CRITICAL_SECTION	RegCritSec;			//@field Protects UART
	CRITICAL_SECTION	TransmitCritSec;	//@field Protects UART registers for non-atomic accesses

	ULONG				ChipID;				// @field Chip ID (CHIP_ID_16550 or CHIP_ID_16450)
	BOOL				PowerDown;			// did we power down the chip
	BOOL                bSuspendResume;		// indicate Suspend/Resume happens

    DWORD				dwSysIntr;			// @field System Interrupt number for this peripheral

#ifdef EXAMINE_BOOTARGS
    PBOOT_ARGS			pBootArgs;			// @field Pointer to global boot args struct
#endif    
    PHWOBJ				pHWObj;				// @field Pointer to PDDs HWObj structure
} SER_INFO, *PSER_INFO;


// Here are the names of the values stored in the registry
#define PC_REG_SYSINTR_VAL_NAME TEXT("SysIntr") 
#define PC_REG_SYSINTR_VAL_LEN  sizeof( DWORD )

#define PC_REG_DEVINDEX_VAL_NAME TEXT("DeviceArrayIndex") 
#define PC_REG_DEVINDEX_VAL_LEN  sizeof( DWORD )

#ifdef __cplusplus
}
#endif


#endif __EM9X61_COM_H__
