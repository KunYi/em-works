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
/*********************************************************************
*
*
*   Module Name: udp.h
*
*   Abstract:
*       This contains the udp.c specific data structure declarations.
*       Used by the ethernet support routines (kernel and ether bootloader).
*
*   Author:
*
*   History:
*
*   Note:
*
********************************************************************/
#ifndef UDP_H
#define UDP_H

#include <halether.h>
#include "eboot.h"

// These structures must be byte aligned so that they can be laid over real packet data
#include <pshpack1.h>

// This is the format for the ethernet frame that is transmitted on the wire
typedef struct EthernetFrameHeaderlab {
    UINT16 wDestMAC[3];
    UINT16 wSrcMAC[3];
    UINT16 wFrameType;
    // Then the data
    // After the data is a 4 byte CRC
} EthernetFrameHeader;

// This is the format for an ARP packet
typedef struct ARPPacketFormatTag {

    UINT16 wHardwareType;
    UINT16 wProtocolType;
    BYTE bHardwareAddrSize;
    BYTE bProtocolAddrSize;
    UINT16 wOperation;
    UINT16 wSrcMAC[3];
    DWORD dwSrcIP;
    UINT16 wDestMAC[3];
    DWORD dwDestIP;

} ARPPacketFormat;

// This is the format for the IP packet header
typedef struct IPHeaderFormatTag {
    BYTE bVersionLength;
    BYTE bTypeOfService;
    UINT16 cwTotalLength;
    UINT16 wIdentification;
    UINT16 wFragment;
    BYTE bTimeToLive;
    BYTE bProtocol;
    UINT16 wCRC;
    DWORD dwSrcIP;
    DWORD dwDestIP;
    // Options can go in here
    // Then comes the data
} IPHeaderFormat;

// This is the format for the UDP packet header
typedef struct UDPHeaderFormatTag {
    UINT16 wSrcPort;
    UINT16 wDestPort;
    UINT16 cwTotalUDPLength;
    UINT16 wCRC;
    // Then comes the data
} UDPHeaderFormat;

// This is the format for the UDP packet header
typedef struct UDPPseudoHeaderFormatTag {
    DWORD dwSrcIP;
    DWORD dwDestIP;
    BYTE bZero;
    BYTE bProtocol;
    UINT16 cwTotalUDPLength;
} UDPPseudoHeaderFormat;

#include <poppack.h>


// Since the ethernet, IP, and UDP headers are fixed length, we can use this
// offset to write user data directly into frame buffer, saving a copy.
#define UDP_DATA_FRAME_OFFSET  (sizeof(EthernetFrameHeader) +  sizeof(IPHeaderFormat) + sizeof(UDPHeaderFormat))

void SrcAddrFromFrame( EDBG_ADDR *pAddr, BYTE *pFrameBuffer );
UINT16 ProcessARP( EDBG_ADDR *pMyAddr, BYTE *pFrameBuffer );
void SetPromiscuousIP( void );
void ClearPromiscuousIP( void );
UINT16 CheckUDP( DWORD dwMyIP, BYTE *pFrameBuffer, UINT16 *wDestPort, UINT16 *wSrcPort, UINT16 **pwData, UINT16 *cwLength );
BOOL EbootCheckIP (EDBG_ADDR *pEdbgAddr);

#endif
