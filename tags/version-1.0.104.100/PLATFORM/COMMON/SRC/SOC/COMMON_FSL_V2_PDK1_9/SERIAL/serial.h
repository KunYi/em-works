//------------------------------------------------------------------------------
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

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
// Defines

// Here are the names of the values stored in the registry
#define PC_REG_IOBASE_VAL_NAME TEXT("IoBase")
#define PC_REG_IOBASE_VAL_LEN  sizeof(DWORD)

#define PC_REG_IOLEN_VAL_NAME TEXT("IoLen")
#define PC_REG_IOLEN_VAL_LEN  sizeof(DWORD)

#define PC_REG_DMA_VAL_NAME TEXT("DMA")
#define PC_REG_DMA_VAL_LEN  sizeof(DWORD);

#define PC_REG_DEVINDEX_VAL_NAME TEXT("DeviceArrayIndex")
#define PC_REG_DEVINDEX_VAL_LEN  sizeof(DWORD);

// SDMA interfacing of UART
#define SERIAL_MAX_DESC_COUNT_RX   0x02
#define SERIAL_MAX_DESC_COUNT_TX   0x01 

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

typedef struct  __PAIRS {
    ULONG    Key;
    ULONG    AssociatedValue;
} PAIRS, *PPAIRS;

typedef struct __LOOKUP_TBL {
    ULONG    Size;
    PPAIRS    Table;
} LOOKUP_TBL, *PLOOKUP_TBL;

// We use a callback for serial events
typedef VOID    (*EVENT_FUNC)(PVOID Arg1, ULONG Arg2);

typedef struct {
    volatile PCSP_UART_REG    pUartReg;

    ULONG    sUSR1;
    ULONG    sUSR2;

    //To keep the DSR state, because we use either trigger, we only get int 
    //state when change of state.
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
    DCB    dcb;        // Device Control Block (copy of DCB in MDD)
    // And the same thing applies for CommTimeouts
    COMMTIMEOUTS    CommTimeouts;  // Copy of CommTimeouts structure

    // Misc fields used by ser16550 library
    PLOOKUP_TBL    pBaudTable;     // Pointer to Baud Table
    ULONG    DroppedBytes;  // Number of dropped bytes
    HANDLE    FlushDone;        // Handle to flush done event.

    BOOL    CTSFlowOff;     // Flag - CTS flow control state.
    BOOL    MDDFlowOff;     // Flag - Set to TRUE when MDD takes control of RTS bit to flow off when MDD buffer is full
    BOOL    DSRFlowOff;     // Flag - DSR flow control state.
    BOOL    AddTXIntr;      // Flag - Fake a TX intr.
    COMSTAT    Status;      // Bitfield representing Win32 comm status.
    ULONG    CommErrors;    // Bitfield representing Win32 comm error status.
    ULONG    ModemStatus;   // Bitfield representing Win32 modem status.
    // Protects UART Registers for non-atomic accesses
    CRITICAL_SECTION    TransmitCritSec; 
    CRITICAL_SECTION    RegCritSec; // Protects UART
    ULONG    ChipID;        // Chip identifier (CHIP_ID_16550 or CHIP_ID_16450)
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
    ULONG    irq;

    PUCHAR    pBaseAddress;   // @field Start of serial registers - mapped
    UINT8    cOpenCount;     // @field Count of concurrent opens
    COMMPROP    CommProp;       // @field Pointer to CommProp structure.
    PHWOBJ    pHWObj;           // @field Pointer to PDDs HWObj structure

    //SDMA related fields
    // flag indicates if SDMA is to be used for transfers through this UART.  
    BOOL        useDMA;
    
    // transmit and receive SDMA request lines
    DDK_DMA_REQ     SerialDmaReqTx;
    DDK_DMA_REQ     SerialDmaReqRx;
    
    // Physical addresses of Rx and Tx side SDMA buffers (used by SDMA)
    PHYSICAL_ADDRESS SerialPhysTxDMABufferAddr;
    PHYSICAL_ADDRESS SerialPhysRxDMABufferAddr;
    
    // Virtual addresses of Rx and Tx side SDMA buffers (used by ARM)
    PBYTE       pSerialVirtTxDMABufferAddr;
    PBYTE       pSerialVirtRxDMABufferAddr;
    
    // SDMA virtual channel indices for Rx and Tx channels
    UINT8       SerialDmaChanRx;
    UINT8       SerialDmaChanTx;
    
    // Index of the buffer descriptor next expected to complete its SDMA in the
    // Rx and Tx SDMA buffer descriptor chains.
    UINT8       currRxDmaBufId;
    UINT8       currTxDmaBufId;
    
    // These fields are used for managing delivery of partial data from SDMA Rx
    // buffer to MDD. If MDD's TargetRoom is smaller than the data available in
    // recently completed Rx SDMA buffer, then these two variables keep the 
    // start index of byte to be delivered to MDD in its next call, and the 
    // remaining bytes in the Rx SDMA buffer. 
    UINT        dmaRxStartIdx;
    UINT        availRxByteCount;
    
    // indicates if an SDMA request is in progress on Tx SDMA buffer descriptor.
    // A bitmap of flags, i-th bit for i-th buffer descriptor. 
    UINT32      awaitingTxDMACompBmp;
    
    // indicator for first time use of a Tx SDMA buffer descriptor. (First use
    // of SDMA Tx buffer descriptor needs special handling, as even though it 
    // is free, DDK_DMA_FLAGS_BUSY is set). A bitmap of flags, i-th bit for 
    // i-th buffer descriptor. 
    UINT32      dmaTxBufFirstUseBmp;
    
    // size of buffer per SDMA buffer descriptor for Rx and Tx.
    UINT16      rxDMABufSize;
    UINT16      txDMABufSize;
} SER_INFO, *PSER_INFO;

//------------------------------------------------------------------------------
// Functions

VOID SL_ClearPendingInts(PVOID pContext);
VOID SL_SetOutputMode(PVOID pContext, BOOL UseIR, BOOL Use9Pin);
VOID SL_Reset(PVOID pContext);
BOOL SL_Init(BOOL bIR, uartType_c bType, ULONG HWAddress, PUCHAR pRegBase, 
                PVOID pContext, EVENT_FUNC EventCallback, PVOID pMDDContext, 
                PLOOKUP_TBL pBaudTable);
VOID SL_Deinit(PVOID pContext);
VOID SL_Open(PVOID pContext);
VOID SL_Close(PVOID pContext);
VOID SL_PowerOff(PVOID pContext);
VOID SL_PowerOn(PVOID pContext);
INTERRUPT_TYPE SL_GetIntrType(PVOID pContext);
ULONG SL_RxIntrHandler(PVOID pContext, PUCHAR pTargetBuffer, ULONG *pByteNumber);
VOID SL_TxIntrHandler(PVOID pContext, PUCHAR pSourceBuffer, ULONG *pByteNumber);
VOID SL_ModemIntrHandler(PVOID pContext);
VOID SL_LineIntrHandler(PVOID pContext);
ULONG SL_GetRxBufferSize(PVOID pContext);
VOID SL_ClearDTR(PVOID pContext);
VOID SL_SetDTR(PVOID pContext);
VOID SL_ClearRTS(PVOID pContext);
VOID SL_SetRTS(PVOID pContext);
VOID SL_ClearBreak(PVOID pContext);
VOID SL_SetBreak(PVOID pContext);
BOOL SL_XmitComChar(PVOID pContext, UCHAR ComChar);
ULONG SL_GetStatus(PVOID pContext, LPCOMSTAT lpStat);
VOID SL_GetModemStatus(PVOID pContext, PULONG pModemStatus);
VOID SL_PurgeComm(PVOID pContext, DWORD fdwAction);
BOOL SL_SetDCB(PVOID pContext, LPDCB lpDCB);
BOOL SL_SetCommTimeouts(PVOID pContext, LPCOMMTIMEOUTS lpCommTimeouts);

//WINCE600
BOOL SL_Ioctl(PVOID  pContext,DWORD  Ioctl,PUCHAR pInBuf,DWORD  InBufLen,PUCHAR pOutBuf,
                  DWORD  OutBufLen,PDWORD pdwBytesTransferred);

#ifdef __cplusplus
}
#endif

#endif // __SERIAL_H__
