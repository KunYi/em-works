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
//  File:  irfir.h
//
//   Header file for iMX51 FIR device.
//
//------------------------------------------------------------------------------

#ifndef __IRFIR_H__
#define __IRFIR_H__

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)
#include <Winbase.h>
#include <nkintr.h>

#pragma warning(push)
#pragma warning(disable: 4127)
#include <linklist.h>
#undef RemoveEntryList
#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Entry;\
    _EX_Entry = (Entry);\
    _EX_Entry->Blink->Flink = _EX_Entry->Flink;\
    _EX_Entry->Flink->Blink = _EX_Entry->Blink;\
    }
#undef InsertTailList
#define InsertTailList(_ListHead,_Entry) {\
    PLIST_ENTRY _EX_ListHead = _ListHead; \
    PLIST_ENTRY _EX_Blink = _EX_ListHead->Blink; \
    (_Entry)->Flink = _EX_ListHead; \
    (_Entry)->Blink = _EX_Blink; \
    _EX_Blink->Flink = _Entry; \
    _EX_ListHead->Blink = _Entry; \
    }
#undef InsertHeadList
#define InsertHeadList(_ListHead,_Entry) {\
    PLIST_ENTRY _EX_ListHead = _ListHead; \
    PLIST_ENTRY _EX_Flink = _EX_ListHead->Flink; \
    (_Entry)->Flink = _EX_Flink; \
    (_Entry)->Blink = _EX_ListHead; \
    _EX_Flink->Blink = _Entry; \
    _EX_ListHead->Flink = _Entry; \
    }
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4214)
#include <ndis.h>
#pragma warning(pop)
#include <ntddndis.h>  // defines OID's
#include "common_firi.h"
#include "common_ddk.h"
#include "common_uart.h"
#include "common_macros.h"
#include "settings.h"
#include "sir.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#ifdef DEBUG
#define ZONE_INIT       DEBUGZONE(0)
#define ZONE_DEINIT     DEBUGZONE(1)
#define ZONE_RECV       DEBUGZONE(2)
#define ZONE_SEND       DEBUGZONE(3)
#define ZONE_SETINFO    DEBUGZONE(4)
#define ZONE_QUERYINFO  DEBUGZONE(5)
#define ZONE_THREAD     DEBUGZONE(6)
#define ZONE_OPEN       DEBUGZONE(7)
#define ZONE_CLOSE      DEBUGZONE(8)
#define ZONE_COMMMASK   DEBUGZONE(9)
#define ZONE_MISC       DEBUGZONE(11)
#define ZONE_ALLOC      DEBUGZONE(12)
#define ZONE_FUNCTION   DEBUGZONE(13)
#define ZONE_WARN       DEBUGZONE(14)
#define ZONE_ERROR      DEBUGZONE(15)
#endif

#define NDIS_MAJOR_VERSION 5
#define NDIS_MINOR_VERSION 0
#define HEAD_SEND_PACKET(dev)    (PNDIS_PACKET) (IsListEmpty(&(dev)->SendQueue) ? NULL :    \
                                                        CONTAINING_RECORD((dev)->SendQueue.Flink, NDIS_PACKET, MiniportReserved))

__inline TCHAR * DBG_NDIS_RESULT_STR(NDIS_STATUS status)
{
    switch (status)
    {
        case NDIS_STATUS_SUCCESS:
            return (TEXT("NDIS_STATUS_SUCCESS"));
        case NDIS_STATUS_FAILURE:
            return (TEXT("NDIS_STATUS_FAILURE"));
        case NDIS_STATUS_PENDING:
            return (TEXT("NDIS_STATUS_PENDING"));
        case NDIS_STATUS_RESOURCES:
            return (TEXT("NDIS_STATUS_RESOURCES"));
    }
    return (TEXT("NDIS_STATUS_???"));
}


//------------------------------------------------------------------------------
// Types

typedef enum rcvbufferStates {
    STATE_FREE,
    STATE_FULL,
    STATE_PENDING
} rcvBufferState_c;

typedef enum {
    ADAPTER_NONE=0,
    ADAPTER_TX,
    ADAPTER_RX
} adapterState_c;

typedef struct {
    LIST_ENTRY listEntry;
    rcvBufferState_c state;
    PNDIS_PACKET packet;
    UINT dataLen;
    PUCHAR dataBuf;
    BOOLEAN isDmaBuf;
} rcvBuffer, *pRcvBuffer_t;

struct FirDevice;

typedef struct _HW_IR_VTBL{
    BOOLEAN        (*m_pInitialize)(FirDevice *thisDev);
    VOID           (*m_pDeInitialize)(FirDevice *thisDev);
    BOOLEAN        (*m_pAcquireAdapterResources)(FirDevice *thisDev);
    VOID           (*m_pReleaseAdapterResources)(FirDevice *thisDev);
    VOID           (*m_pEnableInterrupt)(FirDevice *thisDev);
    VOID           (*m_pDisableInterrupt)(FirDevice *thisDev);
    NDIS_STATUS    (*m_pDelayedSendPacket)(FirDevice *thisDev, BOOLEAN firstBufIsPending);
    NDIS_STATUS    (*m_pSendPacket)(FirDevice *thisDev, PNDIS_PACKET Packet);
    VOID           (*m_pSetupRecv)(FirDevice *thisDev);
    VOID           (*m_pInterruptHandler)(FirDevice *thisDev);
    baudRates      (*m_pSetupSpeed)(FirDevice *thisDev);
}_HW_IR_VTBL, *PHW_IR_VTBL;

typedef struct FirDevice
{
    NDIS_HANDLE ndisAdapterHandle;
    NDIS_SPIN_LOCK Lock;
    BOOLEAN resourcesReleased;

    NDIS_MINIPORT_INTERRUPT FirinterruptObj;
    NDIS_MINIPORT_INTERRUPT SirinterruptObj;
    DWORD sysIntrFir;
    DWORD sysIntrSir;

    NDIS_MINIPORT_TIMER TurnaroundTimer;
    BOOLEAN HangChk;
    baudRateInfo *linkSpeedInfo;
    UINT newSpeed;
    BOOLEAN mediaBusy;

    PUCHAR readBuf;    // Specially for FIR receiving buf
    PUCHAR writeBuf;

    BOOLEAN writePending;

    BOOLEAN nowReceiving;
    adapterState_c AdapterState;

    NDIS_HANDLE packetPoolHandle;
    NDIS_HANDLE bufferPoolHandle;

    LIST_ENTRY SendQueue;

    UINT writeBufLen;

    ULONG FirPhyAddr;
    ULONG SirPhyAddr;

    LIST_ENTRY rcvBufBuf;     // Protected by SyncWithInterrupt, specially used for SIR
    LIST_ENTRY rcvBufFree;    // Protected by SyncWithInterrupt
    LIST_ENTRY rcvBufFull;    // Protected by SyncWithInterrupt
    LIST_ENTRY rcvBufPend;    // Protected by QueueLock

    PNDIS_PACKET lastPacketAtOldSpeed;
    BOOLEAN setSpeedAfterCurrentSendPacket;

    ULONG rcvDataLength;

    SirPort_t portInfo;
    PHW_IR_VTBL IR_VTbl;

    struct FirDevice *next;
} FirDevice_t, *pFirDevice_t;


//------------------------------------------------------------------------------
// Functions

#include "externs.h"

#ifdef __cplusplus
}
#endif

#endif // __IRFIR_H__
