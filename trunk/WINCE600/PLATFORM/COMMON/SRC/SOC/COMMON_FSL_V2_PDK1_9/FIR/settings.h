//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  settings.h
//
//   Header file for iMX51 FIR device.
//
//------------------------------------------------------------------------------

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define FIRI_FIFO_SIZE          128
#define OSF_VALUE(x)            (x-1)
#define TPL_VALUE(x)            (x-1)
#define RFDIV_VALUE(x)          ((x <= 6)?(6-x):6)
#define BURST_LENGTH_DEFAULT    16

#define XMIT_INTR_MASK          0x0F
#define RCV_INTR_MASK           0x1F

#define IR_ADDR_SIZE            1
#define IR_CONTROL_SIZE         1
#define MAX_NUM_EXTRA_BOFS      48

#define SLOW_IR_BOF_TYPE        UCHAR
#define SLOW_IR_BOF_SIZE        sizeof(SLOW_IR_BOF_TYPE)
#define SLOW_IR_EXTRA_BOF_TYPE  UCHAR
#define SLOW_IR_EXTRA_BOF_SIZE  sizeof(SLOW_IR_EXTRA_BOF_TYPE)
#define SLOW_IR_EOF_TYPE        UCHAR
#define SLOW_IR_EOF_SIZE        sizeof(SLOW_IR_EOF_TYPE)
#define SLOW_IR_FCS_TYPE        USHORT
#define SLOW_IR_FCS_SIZE        sizeof(SLOW_IR_FCS_TYPE)
#define SLOW_IR_BOF             0xC0
#define SLOW_IR_EXTRA_BOF       0xC0
#define SLOW_IR_EOF             0xC1
#define SLOW_IR_ESC             0x7D
#define SLOW_IR_ESC_COMP        0x20

#define FAST_IR_FCS_SIZE        4
#define FAST_IR_EOF_SIZE        1

#define NDIS_IRDA_SPEED_2400     (UINT)(1 << 0)   // SLOW IR ...
#define NDIS_IRDA_SPEED_9600     (UINT)(1 << 1)
#define NDIS_IRDA_SPEED_19200    (UINT)(1 << 2)
#define NDIS_IRDA_SPEED_38400    (UINT)(1 << 3)
#define NDIS_IRDA_SPEED_57600    (UINT)(1 << 4)
#define NDIS_IRDA_SPEED_115200   (UINT)(1 << 5)
#define NDIS_IRDA_SPEED_576K     (UINT)(1 << 6)   // MEDIUM IR ...
#define NDIS_IRDA_SPEED_1152K    (UINT)(1 << 7)
#define NDIS_IRDA_SPEED_4M       (UINT)(1 << 8)   // FAST IR

#define SUPPORTED_SIR_MASK ( \
    NDIS_IRDA_SPEED_9600 | \
    NDIS_IRDA_SPEED_19200 | NDIS_IRDA_SPEED_38400 | \
    NDIS_IRDA_SPEED_57600 | NDIS_IRDA_SPEED_115200)
#define SUPPORTED_MIR_MASK    (NDIS_IRDA_SPEED_1152K|NDIS_IRDA_SPEED_576K)
#define SUPPORTED_FIR_MASK    NDIS_IRDA_SPEED_4M

#define ALL_IRDA_SPEEDS    ( \
    NDIS_IRDA_SPEED_2400 | NDIS_IRDA_SPEED_9600 | \
    NDIS_IRDA_SPEED_19200 | NDIS_IRDA_SPEED_38400 | \
    NDIS_IRDA_SPEED_57600 | NDIS_IRDA_SPEED_115200 | \
    NDIS_IRDA_SPEED_1152K | NDIS_IRDA_SPEED_4M)

#define CURRENT_SUPPORTED_SPEEDS    (SUPPORTED_SIR_MASK | SUPPORTED_FIR_MASK)

#define MAX_SIR_SPEED    115200
#define MAX_MIR_SPEED    576000

#define DEFAULT_BAUD_RATE    9600

#define DEFAULT_EXTRA_BOF_REQUIRED    0
#define DEFAULT_TURNAROUND_TIME       1000

#define MAXIMUM_RCV_PACKETS    1
#define MAXIMUM_LOOKAHEAD      256
#define MAXIMUM_SEND_PACKETS   16
#define VENDOR_DRIVER_VERSION  1

#define GOOD_FCS    ((USHORT) ~0xf0b8)

// This is the largest IR packet size (counting _I_ field only,
// and not counting ESC characters) that we handle.
#ifdef FIR_SDMA_SUPPORT
#define MAX_IR_FRAME_SIZE    2048
#else  // polling mode we only have 128byte FIFO
#define MAX_IR_FRAME_SIZE    64
#endif

#define MAX_NDIS_DATA_SIZE    (IR_ADDR_SIZE+IR_CONTROL_SIZE+MAX_IR_FRAME_SIZE)
#define MAX_RCV_DATA_SIZE     (MAX_NDIS_DATA_SIZE+SLOW_IR_FCS_SIZE)

// We loop an extra time in the receive in order to see EOF after the
// last data byte; so we need some extra space in readBuf in case we then
// get garbage instead.

#define RCV_BUFFER_SIZE             (MAX_RCV_DATA_SIZE +FAST_IR_FCS_SIZE+sizeof(LIST_ENTRY))
#define NUM_RCV_BUFS    16


#define RCV_BUF_TO_LIST_ENTRY(b)    ((PLIST_ENTRY)((PUCHAR)(b)-sizeof(LIST_ENTRY)))
#define LIST_ENTRY_TO_RCV_BUF(e)    ((PVOID)((PUCHAR)(e)+sizeof(LIST_ENTRY)))

// We allocate buffers twice as large as the max rcv size to accomodate ESC
// characters and BOFs, etc.  Recall that in the worst possible case, the
// data contains all BOF/EOF/ESC characters, in which case we must expand
// it to twice its original size.
#define MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(dataLen)    ( \
    (dataLen) * 2 + (MAX_NUM_EXTRA_BOFS + 1) * \
    SLOW_IR_BOF_SIZE + IR_ADDR_SIZE + IR_CONTROL_SIZE + \
    SLOW_IR_FCS_SIZE + SLOW_IR_EOF_SIZE)

#define MAX_IRDA_DATA_SIZE \
    MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(MAX_IR_FRAME_SIZE)

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))

#define FIRI_DMA_RX_WATERMARK    16      
#define FIRI_DMA_TX_WATERMARK    16      

#define FIRI_MAX_TX_DESC_COUNT   1
#define FIRI_MAX_RX_DESC_COUNT   1

//------------------------------------------------------------------------------
// Types

enum baudRates {
    BAUDRATE_2400,
    BAUDRATE_9600,
    BAUDRATE_19200,
    BAUDRATE_38400,
    BAUDRATE_57600,
    BAUDRATE_115200,
    BAUDRATE_576000,
    BAUDRATE_1152000,
    BAUDRATE_4000000,

    NUM_BAUDRATES,
    BAUDRATE_INVALID
};

typedef struct {
    enum baudRates tableIndex;
    UINT bitsPerSec;
    UINT ndisCode;  // bitmask element
} baudRateInfo;


//------------------------------------------------------------------------------
// Functions


#ifdef __cplusplus
}
#endif

#endif // __SETTINGS_H__
