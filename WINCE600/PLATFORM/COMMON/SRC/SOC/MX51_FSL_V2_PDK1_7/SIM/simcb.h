//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Header:  simcb.h
//
//------------------------------------------------------------------------------

#ifndef __SIM_CB_H__
#define __SIM_CB_H__

#include <smclib.h>

//------------------------------------------------------------------------------
// Defines

#define SysCompareMemory( p1, p2, Len )             memcmp( p1,p2, Len )
#define SysCopyMemory( pDest, pSrc, Len )               RtlCopyMemory( pDest, pSrc, Len )

#define ATR_SIZE        0x40        // TS + 32 + SW + PROLOGUE + EPILOGUE...

#define CLA_IDX     0
#define INS_IDX     1
#define P1_IDX      2
#define P2_IDX      3
#define P3_IDX      4

#define ISO_OUT     TRUE
#define ISO_IN      !ISO_OUT


//------------------------------------------------------------------------------
// Types

//Reader power state
typedef enum _READER_POWER_STATE {
    PowerReaderUnspecified = 0,
    PowerReaderWorking,
    PowerReaderOff
} READER_POWER_STATE, *PREADER_POWER_STATE;

//Reader Extension
typedef struct _READER_EXTENSION {
    PCSP_SIM_REG    pSIMReg;       //Register structure

    UCHAR    ucReadBuffer[MIN_BUFFER_SIZE];
    ULONG    ulReadBufferLen;

    READER_POWER_STATE  ReaderPowerState;       //Current reader power state

    LONG    d_RefCount;     //Count the number of references to the device

    HANDLE  hBackgroundThread;      //Backgroud thread for track SIM card attach and detach
    DWORD   dwThreadID;     //Background thread ID

    UINT    d_uReaderState;     //State of device (STATE_OPENED, STATE_CLOSED, etc
    PWCHAR  d_ActivePath;       //Registry path to active device

    BOOLEAN IoctlPending;       //Flag for determining Card tracking started
} READER_EXTENSION, *PREADER_EXTENSION;

//------------------------------------------------------------------------------
// Functions

NTSTATUS CBCardPower(PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS CBSetProtocol(PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS CBTransmit(PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS CBCardTracking(PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS CBUpdateCardState(PSMARTCARD_EXTENSION SmartcardExtension);

NTSTATUS CBGenericIOCTL(PSMARTCARD_EXTENSION SmartcardExtension);


#endif // __SIM_CB_H__

//  ------------------------------- END OF FILE -------------------------------


