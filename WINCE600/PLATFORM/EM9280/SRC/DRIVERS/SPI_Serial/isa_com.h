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


#ifndef __ISA_COM_H__   
#define __ISA_COM_H__

#ifdef __cplusplus
extern "C" {
#endif

// TODO - Define PAGE_CONTAINING_UART
#define PAGE_CONTAINING_UART   XXXXX

/*
* @doc HWINTERNAL
* @struct SER_INFO | Private structure.
*/

typedef struct __COM_INFO {
    // CS&ZHL SEP-3-2011: registry specific fields
	DWORD			dwDeviceArrayIndex;			//@field COM logical index = 2, 3, 4, 5, 6, ...
	DWORD			dwIOBasePhyAddress;		//@field UART IO base physical address
	DWORD			dwLocalIndex;					//@field local index within ETA503(16C554)

	PUCHAR		pBaseAddress;					//@field Start of serial registers - mapped
	DWORD			dwDeviceID;						//@field device interrupt number for the UART
    DWORD			dwSysIntr;						//@field System Interrupt number for this peripheral

	//
	// CS&ZHL JUN-27-2008: variances for UART on ISA bus
	//
    SER16550_INFO	ser16550;
    BOOL			fIRMode;				// @field Boolean, are we running in IR mode?

    // parameter fields
    COMMPROP			CommProp;			// @field Pointer to CommProp structure.
	EVENT_FUNC			EventCallback;		// This callback exists in MDD
    PVOID				pMddHead;			// @field First arg to mdd callbacks.

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

	CRITICAL_SECTION	RegCritSec;			//@field Protects UART
	CRITICAL_SECTION	TransmitCritSec;	//@field Protects UART registers for non-atomic accesses

	ULONG				ChipID;				// @field Chip ID (CHIP_ID_16550 or CHIP_ID_16450)
	BOOL				PowerDown;			// did we power down the chip
	BOOL                bSuspendResume;		// indicate Suspend/Resume happens

#ifdef EXAMINE_BOOTARGS
    PBOOT_ARGS			pBootArgs;			// @field Pointer to global boot args struct
#endif    
    PHWOBJ				pHWObj;				// @field Pointer to PDDs HWObj structure
} COM_INFO, *PCOM_INFO;


// Here are the names of the values stored in the registry
#define PC_REG_SYSINTR_VAL_NAME TEXT("SysIntr") 
#define PC_REG_SYSINTR_VAL_LEN  sizeof( DWORD )

#define PC_REG_DEVINDEX_VAL_NAME TEXT("DeviceArrayIndex") 
#define PC_REG_DEVINDEX_VAL_LEN  sizeof( DWORD )

#ifdef __cplusplus
}
#endif


#endif __ISA_COM_H__
