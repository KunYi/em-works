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
#ifndef __VMINI_H
#define __VMINI_H

#include <windows.h>

//------------------------------------------------------------------------------

BOOL
VBridgeInit(
    );

// See if there is packet to be sent.
BOOL
VBridgeKGetOneTxBuffer(
    PUCHAR *ppucBuffer, 
    UINT *puiLength
    );

// Return one free buffer (either one acquired in VBridgeKGetOneTxBuffer()
// or swapping out one from TX descriptor.
void 
VBridgeKGetOneTxBufferComplete(
    PUCHAR pucBuffer
    );

// Indicate that there is a packet for the user mode. When bSwappable
// is set it means that kernel can take other buffer for its rx descriptor.
PUCHAR
VBridgeKIndicateOneRxBuffer(
    PUCHAR pBuffer, 
    UINT uiLength, 
    BOOL bSwappable, 
    BOOL *pbTaken
    );

// Used to indicate the MAC address currently used by the EDBG.
void
VBridgeKSetLocalMacAddress(
    PUCHAR pucMacAddress
    );

BOOL
VBridgeUGetOneTxPacket(
    PUCHAR *ppucBuffer, 
    UINT uiLength
    );

void
VBridgeUGetOneTxPacketComplete(
    PUCHAR pucBufer, 
    UINT uiLength
    );

BOOL
VBridgeUGetOneRxPacket(
    PUCHAR *ppucBuffer,
    UINT *puiLength
    );

BOOL
VBridgeUGetOneRxPacketComplete(
    PUCHAR pucBuffer
    );

void
VBridgeUGetEDBGMac(
    PUCHAR pucMacAddress
    );

void
VBridgeUCurrentPacketFilter(
    PDWORD pdwFilter
    );

BOOL
VBridgeUWildCard(
    LPVOID lpInBuf,
    DWORD nInBufSize,
    LPVOID lpOutBuf,
    DWORD nOutBufSize,
    LPDWORD lpBytesReturned
    );

//------------------------------------------------------------------------------

#endif // __VMINI_H
