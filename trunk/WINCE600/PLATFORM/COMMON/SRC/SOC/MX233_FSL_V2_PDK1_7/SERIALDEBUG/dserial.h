//------------------------------------------------------------------------------
// Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//
//------------------------------------------------------------------------------
//
//  File:  serial_mx233.h
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
// Defines
// Here are the names of the values stored in the registry
#define PC_REG_IOBASE_VAL_NAME TEXT("IoBase")
#define PC_REG_IOBASE_VAL_LEN  sizeof(DWORD)

#define PC_REG_IOLEN_VAL_NAME TEXT("IoLen")
#define PC_REG_IOLEN_VAL_LEN  sizeof(DWORD)

#define PC_REG_DEVINDEX_VAL_NAME TEXT("DeviceArrayIndex")
#define PC_REG_DEVINDEX_VAL_LEN  sizeof(DWORD);


#define UART_CLOCK_FREQUENCY                    (24000000)

#define BUILD_DBG_SERIAL_BAUDRATE_DIVIDER(b)    (((UART_CLOCK_FREQUENCY * 4) + ((b) / 2)) / (b))

#define GET_UARTDBG_BAUD_DIVINT(b)              ((BUILD_DBG_SERIAL_BAUDRATE_DIVIDER(b)) >> 6)
#define GET_UARTDBG_BAUD_DIVFRAC(b)             ((BUILD_DBG_SERIAL_BAUDRATE_DIVIDER(b)) & 0x3F)

//divisor must be between 0x40 and 0x3FFFC0, inclusive.
#define MIN_UARTDBG_DIVISOR      0x40       // Minimum UART XCLK divisor constant.
#define MAX_UARTDBG_DIVISOR      0x3FFFC0   // Maximum UART XCLK divisor constant.


#ifdef DEBUG
#define ZONE_INIT       DEBUGZONE(0)
#define ZONE_OPEN       DEBUGZONE(1)
#define ZONE_READ       DEBUGZONE(2)
#define ZONE_WRITE      DEBUGZONE(3)
#define ZONE_CLOSE      DEBUGZONE(4)
#define ZONE_IOCTL      DEBUGZONE(5)
#define ZONE_THREAD     DEBUGZONE(6)
#define ZONE_EVENTS     DEBUGZONE(7)
#define ZONE_CRITSEC    DEBUGZONE(8)
#define ZONE_FLOW       DEBUGZONE(9)
#define ZONE_IR         DEBUGZONE(10)
#define ZONE_USR_READ   DEBUGZONE(11)
#define ZONE_ALLOC      DEBUGZONE(12)
#define ZONE_FUNCTION   DEBUGZONE(13)
#define ZONE_WARN       DEBUGZONE(14)
#define ZONE_ERROR      DEBUGZONE(15)
#endif // DEBUG

//------------------------------------------------------------------------------
// Types

typedef struct
{
    UINT32 DR;
    UINT32 RSR_ESR;
    UINT32 FR;
    UINT32 ILPR;
    UINT32 IBIR;
    UINT32 FBIR;
    UINT32 LCR;
    UINT32 CRC_POLYNOMIAL;
    UINT32 IFLS;
    UINT32 IMSC;
    UINT32 RIS;
    UINT32 MIS;
    UINT32 ICR;
    UINT32 DMACR;

} CSP_UARTDBG_REG, *PCSP_UARTDBG_REG;



typedef struct  __PAIRS {
    ULONG Key;
    ULONG AssociatedValue;
} PAIRS, *PPAIRS;

typedef struct __LOOKUP_TBL {
    ULONG Size;
    PPAIRS Table;
} LOOKUP_TBL, *PLOOKUP_TBL;

// We use a callback for serial events
typedef VOID (*EVENT_FUNC)(PVOID Arg1, ULONG Arg2);

typedef struct {

    volatile PCSP_UARTDBG_REG pUartdbgReg;

    ULONG ulDiscard;
    BOOL UseIrDA;
    ULONG HwAddr;

    // We have an event callback into the MDD
    EVENT_FUNC EventCallback;    // This callback exists in MDD
    PVOID pMDDContext;     // This is the first parm to callback

    // Keep a copy of DCB since we rely on may of its parms
    DCB dcb;           // Device Control Block (copy of DCB in MDD)
    // And the same thing applies for CommTimeouts
    COMMTIMEOUTS CommTimeouts;  // @field Copy of CommTimeouts structure

    ULONG DroppedBytes;     // Number of dropped bytes
    HANDLE FlushDone;           // Handle to flush done event.

    BOOL CTSFlowOff;        // Flag - CTS flow control state.
    BOOL MDDFlowOff;            // Flag - Set to TRUE when MDD takes control of RTS bit to flow off when MDD buffer is full
    BOOL DSRFlowOff;        // Flag - DSR flow control state.
    BOOL AddTXIntr;         // Flag - Fake a TX intr.
    COMSTAT Status;         // Bitfield representing Win32 comm status.
    ULONG CommErrors;       // Bitfield representing Win32 comm error status.
    ULONG ModemStatus;      // Bitfield representing Win32 mo

    // Protects UART Registers for non-atomic accesses
    CRITICAL_SECTION TransmitCritSec;
    CRITICAL_SECTION RegCritSec;    // Protects UART
} UART_INFO, *PUART_INFO;

typedef struct __SER_INFO {
    // Put lib struct first so we can easily cast pointers
    UART_INFO uart_info;            // UART H/W register

    BOOL fIRMode;
    // now hardware specific fields
    DWORD dwDevIndex;       // @field Index of device
    DWORD dwIOBase;
    DWORD dwIOLen;
    ULONG irq;

    PUCHAR pBaseAddress;      // @field Start of serial registers - mapped
    UINT8 cOpenCount;         // @field Count of concurrent opens
    COMMPROP CommProp;        // @field Pointer to CommProp structure.
    PHWOBJ pHWObj;            // @field Pointer to PDDs HWObj structure

    UINT availRxByteCount;

} SER_INFO, *PSER_INFO;

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

VOID SL_ClearPendingInts(PVOID pContext);
VOID SL_SetOutputMode(PVOID pContext, BOOL UseIR, BOOL Use9Pin);
VOID SL_Reset(PVOID pContext);
BOOL SL_Init(BOOL bIR,ULONG HWAddress, PUCHAR pRegBase,
             PVOID pContext, EVENT_FUNC EventCallback,PVOID pMDDContext);
VOID SL_Deinit(PVOID pContext);
VOID SL_Open(PVOID pContext);
VOID SL_Close(PVOID pContext);
VOID SL_PowerOff(PVOID pContext);
VOID SL_PowerOn(PVOID pContext);
INTERRUPT_TYPE SL_GetIntrType(PVOID pHead);
ULONG SL_RxIntrHandler(PVOID pHead,PUCHAR pRxBuffer,ULONG *pBufflen);
VOID  SL_TxIntrHandler(PVOID pHead,PUCHAR pTxBuffer,ULONG *pBufflen);
VOID SL_ModemIntrHandler(PVOID pContext);
VOID SL_LineIntrHandler(PVOID pContext);
ULONG SL_GetRxBufferSize(PVOID pContext);
VOID SL_ClearDTR(PVOID pHead);
VOID SL_SetDTR(PVOID pHead);
VOID SL_ClearRTS(PVOID pHead);
VOID SL_SetRTS(PVOID pHead);
VOID SL_ClearBreak(PVOID pHead);
VOID SL_SetBreak(PVOID pHead);
BOOL SL_XmitComChar(PVOID pContext, UCHAR ComChar);
ULONG SL_GetStatus(PVOID pHead,LPCOMSTAT lpStat);
VOID SL_GetModemStatus(PVOID pContext, PULONG pModemStatus);
VOID SL_PurgeComm(PVOID pHead,DWORD fdwAction);
BOOL SL_SetDCB(PVOID pHead,LPDCB lpDCB);
BOOL SL_SetCommTimeouts(PVOID pHead,LPCOMMTIMEOUTS lpCommTimeouts);

//WINCE600
BOOL SL_Ioctl(PVOID pContext,DWORD Ioctl,PUCHAR pInBuf,DWORD InBufLen,PUCHAR pOutBuf,
              DWORD OutBufLen,PDWORD pdwBytesTransferred);

#ifdef __cplusplus
}
#endif

#endif // __SERIAL_H__
