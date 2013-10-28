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
//  File:  serial.h
//
//   Header file for zeus serial device.
//
//------------------------------------------------------------------------------

#ifndef __SERIAL_H__
#define __SERIAL_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// MACRO DEFINITIONS 
//------------------------------------------------------------------------------
// Here are the names of the values stored in the registry
#define PC_REG_IOBASE_VAL_NAME TEXT("IoBase")
#define PC_REG_IOBASE_VAL_LEN  sizeof( DWORD )

#define PC_REG_IOLEN_VAL_NAME TEXT("IoLen")
#define PC_REG_IOLEN_VAL_LEN  sizeof( DWORD )

#define PC_REG_DMA_VAL_NAME TEXT("DMA")
#define PC_REG_DMA_VAL_LEN  sizeof( DWORD );

#define PC_REG_DEVINDEX_VAL_NAME TEXT("DeviceArrayIndex")
#define PC_REG_DEVINDEX_VAL_LEN  sizeof( DWORD );

#define SERIAL_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define SERIAL_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))
    
//------------------------------------------------------------------------------
// CLASS DEFINITIONS
//------------------------------------------------------------------------------

typedef struct	__PAIRS {
    ULONG    Key;
    ULONG    AssociatedValue;
} PAIRS, *PPAIRS;

typedef struct __LOOKUP_TBL {
    ULONG    Size;
    PPAIRS    Table;
} LOOKUP_TBL, *PLOOKUP_TBL;

// We use a callback for serial events
typedef VOID    (*EVENT_FUNC)( PVOID Arg1, ULONG Arg2 );

typedef struct {
    volatile PCSP_UART_REG    pUartReg;

    ULONG    sUSR1;
    ULONG    sUSR2;

    //To keep the DSR state, because we use either trigger, we only get int state when change of state.
    BOOL    bDSR;
    uartType_c    UartType;

    //To discard echo chars in Irda mode
    ULONG    ulDiscard;
    BOOL    UseIrDA;
    ULONG    HwAddr;

    // We have an event callback into the MDD
    EVENT_FUNC    EventCallback; // This callback exists in MDD
    PVOID    pMDDContext;  // This is the first parm to callback

    // Keep a copy of DCB since we rely on may of its parms
    DCB    dcb;        // @field Device Control Block (copy of DCB in MDD)
    // And the same thing applies for CommTimeouts
    COMMTIMEOUTS    CommTimeouts;  // @field Copy of CommTimeouts structure

    // Misc fields used by ser16550 library
    PLOOKUP_TBL    pBaudTable;     // @field Pointer to Baud Table
    ULONG    DroppedBytes;		// @field Number of dropped bytes
    HANDLE    FlushDone;		// @field Handle to flush done event.

    BOOL    CTSFlowOff;		// @field Flag - CTS flow control state.
    BOOL    DSRFlowOff;		// @field Flag - DSR flow control state.
    BOOL    AddTXIntr;		// @field Flag - Fake a TX intr.
    COMSTAT    Status; 		// @field Bitfield representing Win32 comm status.
    ULONG    CommErrors;	// @field Bitfield representing Win32 comm error status.
    ULONG    ModemStatus;	// @field Bitfield representing Win32 modem status.
    CRITICAL_SECTION    TransmitCritSec; // @field Protects UART Registers for non-atomic accesses
    CRITICAL_SECTION    RegCritSec; // @field Protects UART
    ULONG    ChipID;			// @field Chip identifier (CHIP_ID_16550 or CHIP_ID_16450)
} UART_INFO, * PUART_INFO;

//struct SER_INFO | Private structure.
typedef struct __SER_INFO {
    // Put lib struct first so we can easily cast pointers
    UART_INFO    uart_info;         // UART H/W register

    BOOL    fIRMode;
    // now hardware specific fields
    DWORD    dwDevIndex;    // @field Index of device
    DWORD    dwIOBase;
    DWORD    dwIOLen;

    PUCHAR    pBaseAddress;   // @field Start of serial registers - mapped
    UINT8    cOpenCount;     // @field Count of concurrent opens
    COMMPROP    CommProp;       // @field Pointer to CommProp structure.
    PHWOBJ    pHWObj;           // @field Pointer to PDDs HWObj structure
} SER_INFO, *PSER_INFO;

//------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//------------------------------------------------------------------------------
VOID SL_ClearPendingInts( PVOID pContext );
VOID SL_SetOutputMode( PVOID pContext, BOOL UseIR, BOOL Use9Pin );
VOID SL_Reset( PVOID pContext );
BOOL SL_Init( BOOL bIR, uartType_c bType, ULONG HWAddress, PUCHAR pRegBase, PVOID pContext, EVENT_FUNC EventCallback, PVOID pMDDContext, PLOOKUP_TBL pBaudTable );
VOID SL_Deinit( PVOID pContext );
VOID SL_Open( PVOID pContext );
VOID SL_Close( PVOID pContext );
VOID SL_PowerOff( PVOID pContext );
VOID SL_PowerOn( PVOID pContext );
INTERRUPT_TYPE SL_GetIntrType( PVOID pContext );
ULONG SL_RxIntrHandler( PVOID pContext, PUCHAR pTargetBuffer, ULONG *pByteNumber );
VOID SL_TxIntrHandler( PVOID pContext, PUCHAR pSourceBuffer, ULONG *pByteNumber );
VOID SL_ModemIntrHandler( PVOID pContext );
VOID SL_LineIntrHandler( PVOID pContext );
ULONG SL_GetRxBufferSize( PVOID pContext );
VOID SL_ClearDTR( PVOID pContext );
VOID SL_SetDTR( PVOID pContext );
VOID SL_ClearRTS( PVOID pContext );
VOID SL_SetRTS( PVOID pContext );
VOID SL_ClearBreak( PVOID pContext );
VOID SL_SetBreak( PVOID pContext );
BOOL SL_XmitComChar( PVOID pContext, UCHAR ComChar );
ULONG SL_GetStatus( PVOID pContext, LPCOMSTAT lpStat );
VOID SL_GetModemStatus( PVOID pContext, PULONG pModemStatus );
VOID SL_PurgeComm( PVOID pContext, DWORD fdwAction );
BOOL SL_SetDCB( PVOID pContext, LPDCB lpDCB );
ULONG SL_SetCommTimeouts( PVOID pContext, LPCOMMTIMEOUTS lpCommTimeouts );

#ifdef __cplusplus
}
#endif

#endif // __SERIAL_H__