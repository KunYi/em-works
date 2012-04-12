//**********************************************************************
//                                                                      
// Filename: em9161_com.h
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


#ifndef __SPI_COM_H__   
#define __SPI_COM_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _UART_HT45B0F_INFO
{
	BYTE	RDR;		//Rx Data Register
	BYTE	USR;		//UART Status Register
	BYTE	UCR1;		//UART Control Register 1
	BYTE	UCR2;		//UART Control Register 2
	BYTE	BRG;		//UART Baud Rate Generator
	BYTE	UCR3;		//UART Control Register 2
	BYTE	THR;		//Tx Hold Register
} UART_HT45B0F_INFO, *PUART_HT45B0F_INFO;


// TODO - Define PAGE_CONTAINING_UART
#define PAGE_CONTAINING_UART   XXXXX

/*
* @doc HWINTERNAL
* @struct SER_INFO | Private structure.
*/
typedef struct __SER_INFO {
	DWORD				dwDeviceArrayIndex;	// @field COM logical index = 4, 8, 9
	DWORD				dwLocalIndex;		// @field local index of HT45B0F
	DWORD				dwIrqNum;			// @field interrupt request number for the UART
	DWORD				dwIrqGpioPin;		// @filed enum { DDK_IOMUX_GPIO2_8, DDK_IOMUX_GPIO2_9, DDK_IOMUX_GPIO2_10 } 	

	UART_HT45B0F_INFO	UartHt45;
    DWORD				dwSysIntr;			// @field System Interrupt number for the UART
    BOOL				fIRMode;			// @field Boolean, are we running in IR mode?

    // parameter fields
    COMMPROP			CommProp;			// @field Pointer to CommProp structure.
	EVENT_FUNC			EventCallback;		// This callback exists in MDD
    PVOID				pMddHead;			// @field First arg to mdd callbacks.
    PLOOKUP_TBL			pBaudTable;			// Pointer to Baud Table

	DCB					dcb;				// @field Device Control Block (copy of DCB in MDD)
	COMMTIMEOUTS		CommTimeouts;		// @field copy of CommTimeouts structure
    ULONG				OpenCount;			// @field Count of concurrent opens
	
	ULONG				DroppedBytes;		// @field Number of dropped bytes
	HANDLE              FlushDone;			// @field Handle to flush done event
	BOOL				CTSFlowOff;			// @field Flag - CTS flow control state.
	BOOL				DSRFlowOff;			// @field Flag - DSR flow control state.
	BOOL				AddTXIntr;			// @field Flag - Fake a TX interrupt
	COMSTAT				Status;				// @field Bitfield representing Win32 modem status
	ULONG				CommErrors;			// @field Bitfield representing Win32 comm error status
	ULONG				ModemStatus;		// @field Bitfield representing Win32 modem status

	CRITICAL_SECTION	RegCritSec;			// @field Protects UART
	CRITICAL_SECTION	TransmitCritSec;	// @field Protects UART registers for non-atomic accesses

	ULONG				ChipID;				// @field Chip ID (CHIP_ID_16550 or CHIP_ID_16450)
	BOOL				PowerDown;			// did we power down the chip
	BOOL                bSuspendResume;		// indicate Suspend/Resume happens

#ifdef EXAMINE_BOOTARGS
    PBOOT_ARGS			pBootArgs;			// @field Pointer to global boot args struct
#endif    
    PHWOBJ				pHWObj;				// @field Pointer to PDDs HWObj structure
} SER_INFO, *PSER_INFO;


// Here are the names of the values stored in the registry
#define PC_REG_DEVINDEX_VAL_NAME	TEXT("DeviceArrayIndex") 
#define PC_REG_DEVINDEX_VAL_LEN		sizeof( DWORD )

#define PC_REG_LOCALIDX_VAL_NAME	TEXT("LocalIndex") 
#define PC_REG_LOCALIDX_VAL_LEN		sizeof( DWORD )

#define PC_REG_IRQNUM_VAL_NAME		TEXT("IrqNum") 
#define PC_REG_IRQNUM_VAL_LEN		sizeof( DWORD )


#ifdef __cplusplus
}
#endif


#endif __SPI_COM_H__
