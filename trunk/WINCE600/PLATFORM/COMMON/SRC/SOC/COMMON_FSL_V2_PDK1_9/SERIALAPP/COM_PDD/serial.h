//------------------------------------------------------------------------------
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
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
// Defines
// Here are the names of the values stored in the registry
#define PC_REG_IOBASE_VAL_NAME TEXT("IoBase")
#define PC_REG_IOBASE_VAL_LEN  sizeof(DWORD)

#define PC_REG_IOLEN_VAL_NAME TEXT("IoLen")
#define PC_REG_IOLEN_VAL_LEN  sizeof(DWORD)

#define PC_REG_DMA_VAL_NAME TEXT("useDMA")
#define PC_REG_DMA_VAL_LEN  sizeof(DWORD);

#define PC_REG_DEVINDEX_VAL_NAME TEXT("DeviceArrayIndex")
#define PC_REG_DEVINDEX_VAL_LEN  sizeof(DWORD);

// SDMA interfacing of UART
#define SERIAL_MAX_DESC_COUNT_RX   0x02
#define SERIAL_MAX_DESC_COUNT_TX   0x04

#define SERIAL_DMA_RX_TIMEOUT      0x03

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

typedef union
{
    UINT32 U;
    struct
    {
        unsigned XFER_COUNT   : 16;
        unsigned RXTIMEOUT    : 11;
        unsigned RXTO_ENABLE  :  1;
        unsigned RX_SOURCE    :  1;
        unsigned RUN          :  1;
        unsigned CLKGATE      :  1;
        unsigned SFTRST       :  1;
    } B;
} UARTAPP_CTRL0;

typedef union
{
    UINT32 U;
    struct
    {
        unsigned XFER_COUNT      : 16;
        unsigned RSVD1       : 12;
        unsigned RUN         :  1;
        unsigned RSVD2       :  3;
    } B;
} UARTAPP_CTRL1;

typedef struct _UARTAPP_RXDMA_DESCRIPTOR
{
    DWORD NextCmdDesc;

    union
    {
        UINT32 CommandBits;                 //!< Union member to manipulate all bit fields at once.
        struct
        {
            unsigned Command    : 2; //!< DMA command for the DMA enginer to execute.
            unsigned Chain      : 1; //!< Indicates if another command structure follows this one. (pNext)
            unsigned IRQ        : 1; //!< Indicates if an interrupt should be generated when this command completes.
            unsigned Reserved1  : 2; //!< Reserved
            unsigned Semaphore  : 1; //!< Indicates if the semaphore counter should be decremented when the command completes.
            unsigned WaitForEnd : 1; //!< Indicates if this command should wait for an end of command signal from the device.
            unsigned HALTONTERMINATE : 1;
            unsigned TERMINATEFLUSH  : 1;
            unsigned Reserved2  : 2; //!< Reserved
            unsigned PIOWords   : 4; //!< Number of PIO words in this command.
            unsigned DMABytes  : 16; //!< Number of bytes to transfer with this command.
        } B;
    } CMD;

    DWORD BufAddr;
    UARTAPP_CTRL0 RegCTRL0;

} UARTAPP_RXDMA_DESCRIPTOR, *PUARTAPP_RXDMA_DESCRIPTOR;

typedef struct _UARTAPP_TXDMA_DESCRIPTOR
{
    DWORD NextCmdDesc;
    union
    {
        UINT32 CommandBits;         //!< Union member to manipulate all bit fields at once.
        struct
        {
            //! Note: The values for the command field can be found in : regsapbx.h
            //!              BV_APBX_CHn_CMD_COMMAND__NO_DMA_XFER
            //!              BV_APBX_CHn_CMD_COMMAND__HW_UARTAPP_DMA_WRITE
            //!              BV_APBX_CHn_CMD_COMMAND__HW_UARTAPP_DMA_READ
            //!
            unsigned Command    : 2; //!< DMA command for the DMA enginer to execute.
            unsigned Chain      : 1; //!< Indicates if another command structure follows this one. (pNext)
            unsigned IRQ        : 1; //!< Indicates if an interrupt should be generated when this command completes.
            unsigned Reserved1  : 2; //!< Reserved
            unsigned Semaphore  : 1; //!< Indicates if the semaphore counter should be decremented when the command completes.
            unsigned WaitForEnd : 1; //!< Indicates if this command should wait for an end of command signal from the device.
            unsigned Reserved2  : 4; //!< Reserved
            unsigned PIOWords   : 4; //!< Number of PIO words in this command.
            unsigned DMABytes   : 16; //!< Number of bytes to transfer with this command.
        } B;
    } CMD;

    DWORD BufAddr;

    UARTAPP_CTRL1 RegCTRL1;

} UARTAPP_TXDMA_DESCRIPTOR, *PUARTAPP_TXDMA_DESCRIPTOR;

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
    DWORD dwIndex;
    DWORD pv_HWregUARTApp0;
    DWORD pv_HWregUARTApp1;
    DWORD pv_HWregUARTApp2;
    DWORD pv_HWregUARTApp3;

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

    //SDMA related fields
    // flag indicates if SDMA is to be used for transfers through this UART.
    BOOL useDMA;

    // Physical addresses of Rx and Tx side SDMA buffers (used by SDMA)
    PHYSICAL_ADDRESS SerialPhysTxDMABufferAddr;
    PHYSICAL_ADDRESS SerialPhysRxDMABufferAddr;

    // Virtual addresses of Rx and Tx side SDMA buffers (used by ARM)
    PBYTE pSerialVirtTxDMABufferAddr;
    PBYTE pSerialVirtRxDMABufferAddr;

    // Physical addresses of Rx and Tx side SDMA buffers (used by SDMA)
    PHYSICAL_ADDRESS SerialPhysTxDMADescAddr;
    PHYSICAL_ADDRESS SerialPhysRxDMADescAddr;

    //Virtual addresses of Rx and Tx side DMA descriptor
    UARTAPP_TXDMA_DESCRIPTOR * pSerialVirtTxDMADescAddr;
    UARTAPP_RXDMA_DESCRIPTOR * pSerialVirtRxDMADescAddr;

    // SDMA virtual channel indices for Rx and Tx channels
    UINT8 SerialDmaChanRx;
    UINT8 SerialDmaChanTx;

    // Index of the buffer descriptor next expected to complete its SDMA in the
    // Rx and Tx SDMA buffer descriptor chains.
    UINT8 currRxDmaBufId;
    UINT8 currTxDmaBufId;

    // These fields are used for managing delivery of partial data from SDMA Rx
    // buffer to MDD. If MDD's TargetRoom is smaller than the data available in
    // recently completed Rx SDMA buffer, then these two variables keep the
    // start index of byte to be delivered to MDD in its next call, and the
    // remaining bytes in the Rx SDMA buffer.
    UINT dmaRxStartIdx;
    UINT availRxByteCount;

    // indicates if an SDMA request is in progress on Tx SDMA buffer descriptor.
    // A bitmap of flags, i-th bit for i-th buffer descriptor.
    UINT32 awaitingTxDMACompBmp;

    // indicator for first time use of a Tx SDMA buffer descriptor. (First use
    // of SDMA Tx buffer descriptor needs special handling, as even though it
    // is free, DDK_DMA_FLAGS_BUSY is set). A bitmap of flags, i-th bit for
    // i-th buffer descriptor.
    UINT32 dmaTxBufFirstUseBmp;

    // size of buffer per SDMA buffer descriptor for Rx and Tx.
    UINT16 rxDMABufSize;
    UINT16 txDMABufSize;

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
