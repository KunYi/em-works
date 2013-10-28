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
#ifndef __SERIAL_H
#define __SERIAL_H

//------------------------------------------------------------------------------

// Use the serial bootloader in conjunction with the serial kitl transport
// Both the transport and the download service expect packets with the
// same header format. Packets not of type DLREQ, DLPKT, or DLACK are 
// ignored by the serial bootloader.

#define KITL_MAX_DEV_NAMELEN      16
#define KITL_MTU                  1520

// serial packet header and trailer definitions
static const uint8_t packetHeaderSig[] = { 'k', 'I', 'T', 'L' };
#define HEADER_SIG_BYTES    sizeof(packetHeaderSig)

#define KS_PKT_KITL         0xAA
#define KS_PKT_DLREQ        0xBB
#define KS_PKT_DLPKT        0xCC
#define KS_PKT_DLACK        0xDD
#define KS_PKT_JUMP         0xEE

//------------------------------------------------------------------------------

// packet header
#include <pshpack1.h>
typedef struct tagSERIAL_PACKET_HEADER {
    uint8_t headerSig[HEADER_SIG_BYTES];
    uint8_t pktType;
    uint8_t Reserved;
    uint16_t payloadSize; // not including this header    
    uint8_t crcData;
    uint8_t crcHdr;
} SERIAL_PACKET_HEADER, *PSERIAL_PACKET_HEADER;
#include <poppack.h>

//------------------------------------------------------------------------------

// packet payload
typedef struct tagSERIAL_BOOT_REQUEST {
    uint8_t PlatformId[KITL_MAX_DEV_NAMELEN+1];
    uint8_t DeviceName[KITL_MAX_DEV_NAMELEN+1];
    uint16_t reserved;
} SERIAL_BOOT_REQUEST, *PSERIAL_BOOT_REQUEST;

//------------------------------------------------------------------------------

typedef struct tagSERIAL_BOOT_ACK {
    uint32_t fJumping;
} SERIAL_BOOT_ACK, *PSERIAL_BOOT_ACK;

//------------------------------------------------------------------------------

typedef struct tagSERIAL_JUMP_REQUST {
    uint32_t dwKitlTransport;    
} SERIAL_JUMP_REQUEST, *PSERIAL_JUMP_REQUEST;

//------------------------------------------------------------------------------

typedef struct tagSERIAL_BLOCK_HEADER {
    uint32_t uBlockNum;    
} SERIAL_BLOCK_HEADER, *PSERIAL_BLOCK_HEADER;

//------------------------------------------------------------------------------

__declspec(align(4)) BYTE g_buffer[KITL_MTU];

// OS launch function type
typedef void (*PFN_LAUNCH)();

//------------------------------------------------------------------------------

#endif
