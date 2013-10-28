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
//  Common RS232/USB Serial routines for bootloaders.
//
#ifndef __OAL_BLSERIAL_H
#define __OAL_BLSERIAL_H

#include <windows.h>
#include <kitl.h>
#include <kitlprot.h>

//------------------------------------------------------------------------------

#if __cplusplus
extern "C" {
#endif

// Use the serial bootloader in conjunction with the serial kitl transport
// Both the transport and the download service expect packets with the
// same header format. Packets not of type DLREQ, DLPKT, or DLACK are
// ignored by the serial bootloader.
// serial packet header and trailer definitions

static const UCHAR packetHeaderSig[] = { 'k', 'I', 'T', 'L' };
#define HEADER_SIG_BYTES    sizeof(packetHeaderSig)

#define KS_PKT_KITL         0xAA
#define KS_PKT_DLREQ        0xBB
#define KS_PKT_DLPKT        0xCC
#define KS_PKT_DLACK        0xDD
#define KS_PKT_JUMP         0xEE

BOOL 
OEMSerialSendRaw(
    __in_ecount(cbFrame) LPBYTE pbFrame, 
    USHORT cbFrame
    );

BOOL
OEMSerialRecvRaw(
    __inout_ecount(*pcbFrame) LPBYTE pbFrame, 
    __inout PUSHORT pcbFrame, 
    BOOLEAN bWaitInfinite
    );

// packet header
#include <pshpack1.h>

typedef struct tagSERIAL_PACKET_HEADER {
    UCHAR headerSig[HEADER_SIG_BYTES];
    UCHAR pktType;
    UCHAR Reserved;
    USHORT payloadSize; // not including this header    
    UCHAR crcData;
    UCHAR crcHdr;
} SERIAL_PACKET_HEADER, *PSERIAL_PACKET_HEADER;

#include <poppack.h>

// packet payload
typedef struct SERIAL_BOOT_REQUEST {
    UCHAR PlatformId[KITL_MAX_DEV_NAMELEN+1];
    UCHAR DeviceName[KITL_MAX_DEV_NAMELEN+1];
    USHORT reserved;
} SERIAL_BOOT_REQUEST, *PSERIAL_BOOT_REQUEST;

typedef struct SERIAL_BOOT_ACK {
    DWORD fJumping;
} SERIAL_BOOT_ACK, *PSERIAL_BOOT_ACK;

typedef struct SERIAL_JUMP_REQUEST {
    DWORD dwKitlTransport;
} SERIAL_JUMP_REQUEST, *PSERIAL_JUMP_REQUEST;

typedef struct SERIAL_BLOCK_HEADER {
    DWORD uBlockNum;
} SERIAL_BLOCK_HEADER, *PSERIAL_BLOCK_HEADER;

BOOL 
RecvHeader(
    __in PSERIAL_PACKET_HEADER pHeader, 
    BOOLEAN bWaitInfinite
    );

BOOL
RecvPacket(
    __in PSERIAL_PACKET_HEADER pHeader, 
    __in PBYTE pbFrame, 
    __in PUSHORT pcbFrame, 
    BOOLEAN bWaitInfinite
    );

BOOL  
SerialSendBlockAck(
    DWORD uBlockNumber
    );

BOOL  
SerialSendBootRequest(
    __in const char * platformString
    );

DWORD
SerialWaitForJump(
    );

BOOL
SerialWaitForBootAck(
    __out BOOL *pfJump
    );

BOOL  
SerialReadData(
    DWORD cbData, 
    __in_bcount(cbData) LPBYTE pbData
    );

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
