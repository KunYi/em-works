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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pdd.h
//
#ifndef __PDD_H
#define __PDD_H

//------------------------------------------------------------------------------

// For BULK IN transfers of data where the transfer length is a multiple of
// maximum endpoint size we need to issue an extra zero length packet to
// indicate the end of the transfer.  It seems that the controller has some
// issues and does not send the 0-length packets correctly.  by uncommenting
// the line below we will use 1-byte packets instead (each packet has
// internally encoded length and the RNDIS driver is supposed to ignore
// the extra 1 byte at the end).

#define USE_1BYTE_TERMINATING_PACKETS

//------------------------------------------------------------------------------

#define EP0MaxSize        0x40
#define EPIntMaxSize     0x10
#define EPHSMaxSize      0x200
#define EPFSMaxSize     0x40

enum TRANSFER_STATE {
    TS_IDLE=0,
    TS_RECEIVING_MESSAGE,
    TS_SENDING_MESSAGE,
    TS_RECEIVING_PACKET,
    TS_SENDING_PACKET,
    TS_SENDING_PACKET_TERMINATOR
};

enum DEVICE_STATE {
    DS_DETACHED = 0,
    DS_ATTACHED,
    DS_POWERED,
    DS_DEFAULT,
    DS_ADDRESSED,
    DS_CONFIGURED,
    DS_SUSPENDED,
};

typedef struct {
    UCHAR ucbLength;
    UCHAR udbDescriptorType;
    TCHAR ptcbString[12 + 1];
} USB_SERIAL_NUMBER;

typedef struct {
    UCHAR   ucbLength;
    UCHAR   ucbDescriptorType;
    PTCHAR   ptcbString;
} USB_STRING;


// Check dwFlags
#define TRANSFER_IS_IN(pTransfer)   (pTransfer->dwFlags & USB_REQUEST_DEVICE_TO_HOST)
#define TRANSFER_IS_OUT(pTransfer)  (!TRANSFER_IS_IN(pTransfer))

typedef struct {
    DWORD Notification;
    DWORD dwReserved;
} INTERRUPT_DATA, *PINTERRUPT_DATA;

#define MAX_INCOMING_BUFFER         8192
#define EP0_MAX_RECEIVE_BUFFER      1024

typedef enum {
    EP0Setup = 0,
    EP0Out,
    EP0In
} EP0_DIR;

typedef struct _EP0_REQUEST EP0_REQUEST, *PEP0_REQUEST;
struct _EP0_REQUEST
{
    EP0_DIR eDir;
    UCHAR *pucData;
    DWORD dwExpectedSize;
    DWORD dwActualSize;
    VOID (*pfnNotification)(EP0_REQUEST *pRequest, PVOID pvUser);
    PVOID pvUser;

    DWORD dwProcessed;
    BOOL fCompleted;
};

typedef BOOL (*PFN_GETDESCRIPTOR)(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest);
typedef BOOL (*PFN_DOVENDORCOMMAND)(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest);

//------------------------------------------------------------------------------

typedef struct {
    EP0_REQUEST EP0Request;

    DWORD EP0RxBuffer[EP0_MAX_RECEIVE_BUFFER/sizeof(DWORD)];
    DATA_WRAPPER EP0DataWrapper;
    DATA_WRAPPER *pEP0DataWrapper;

    LIST_ENTRY listTxRndisMessageQueue;
    DATA_WRAPPER *pTxDataWrapper;

    DATA_WRAPPER *pRxDataWrapper;
} RNDIS_KITLDEV, *PRNDIS_KITLDEV;

//------------------------------------------------------------------------------

void TxComplete(void);
BOOL PDD_GetDescriptor(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest);
BOOL PDD_DoVendorCommand(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest);

void PDD_SendRndisMessage(PDATA_WRAPPER  pDataWrapper);
void PDD_SendRndisPacket(PDATA_WRAPPER pDataWrapper);
BOOL PDD_Set(UINT uiRequestId, PVOID pvData, ULONG ulDataLength);
BOOL PDD_Get(UINT uiRequestId, PVOID pvData, ULONG ulDataLength, ULONG *pulRequiredLength);
BOOL PDD_ISR(PDWORD pdwWaitTime);
void PDD_IndicateRndisPacketComplete(PDATA_WRAPPER pDataWrapper);

//------------------------------------------------------------------------------

#endif // __PDD_H
