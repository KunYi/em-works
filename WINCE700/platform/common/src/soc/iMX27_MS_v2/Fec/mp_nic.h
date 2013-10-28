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
/*++

Module Name:
    mp_nic.h

Abstract:
    Function prototypes for mp_nic.c, mp_init.c and mp_req.c

Revision History:

Notes:

--*/

#include "fec.h"

#ifndef _MP_NIC_H
#define _MP_NIC_H

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------
extern PCSP_FEC_REGS    gpFECReg;

extern PFEC_MII_LIST    gMIIFree;
extern PFEC_MII_LIST    gMIIHead;

extern CRITICAL_SECTION gFECRegCs;
extern CRITICAL_SECTION gFECBufCs;


__inline BOOL NIC_INTERRUPT_DISABLED(PMP_ADAPTER     Adapter){ return TRUE; }

__inline BOOL NIC_INTERRUPT_ACTIVE(PMP_ADAPTER     Adapter){ return TRUE; }

__inline BOOL NIC_ACK_INTERRUPT(Adapter, IntStatus){ return TRUE; }

__inline BOOL NIC_IS_RECV_READY(PMP_ADAPTER     Adapter)
{ return MP_TEST_FLAG(Adapter,fMP_RECV_READY); }

__inline BOOL NIC_SET_RECV_READY(PMP_ADAPTER     Adapter)
{ return MP_SET_FLAG(Adapter,fMP_RECV_READY); }

__inline BOOL NIC_CLEAR_RECV_READY(PMP_ADAPTER     Adapter)
{ return MP_CLEAR_FLAG(Adapter,fMP_RECV_READY); }

__inline VOID NICDisableInterrupt(
    IN PMP_ADAPTER Adapter)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECDisableInterrupt\r\n")));

    // Disable TxF, RxF and MII interrupts
    OUTREG32(&gpFECReg->EIMR,
                    CSP_BITFVAL(FEC_EIMR_TXF, FEC_EIMR_TXF_MASK) |
                    CSP_BITFVAL(FEC_EIMR_RXF, FEC_EIMR_RXF_MASK) |
                    CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_MASK) |
                    CSP_BITFVAL(FEC_EIMR_LC,  FEC_EIMR_LC_MASK)  |
                    CSP_BITFVAL(FEC_EIMR_UN,  FEC_EIMR_UN_MASK)  |
                    CSP_BITFVAL(FEC_EIMR_RL,  FEC_EIMR_RL_MASK) );

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECDisableInterrupt\r\n")));
    return;
}

ULONG NICGetMediaState(IN  PMP_ADAPTER   Adapter);

__inline VOID NICEnableInterrupt(
    IN PMP_ADAPTER Adapter)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECEnableInterrupt\r\n")));

    // Enable TxF, RxF and MII interrupts
    OUTREG32(&gpFECReg->EIMR,
                    CSP_BITFVAL(FEC_EIMR_TXF, FEC_EIMR_TXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_RXF, FEC_EIMR_RXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_LC,  FEC_EIMR_LC_UNMASK)  |
                    CSP_BITFVAL(FEC_EIMR_UN,  FEC_EIMR_UN_UNMASK)  |
                    CSP_BITFVAL(FEC_EIMR_RL,  FEC_EIMR_RL_UNMASK) );

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECEnableInterrupt\r\n")));
    return;
}

__inline VOID NICEnableMIIInterrupt()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +NICEnableMIIInterrupt\r\n")));

    // Enable MII interrupts
    OUTREG32(&gpFECReg->EIMR,
                    CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_UNMASK));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -NICEnableMIIInterrupt\r\n")));
    return;
}

__inline VOID NICDisableMIIInterrupt()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +NICDisableMIIInterrupt\r\n")));

    // Enable MII interrupts
    OUTREG32(&gpFECReg->EIMR,
                    CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_MASK));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -NICDisableMIIInterrupt\r\n")));
    return;
}


//
//  MP_NIC.C
//
NDIS_STATUS MiniportSendNetBufferList(
    IN  PMP_ADAPTER         Adapter,
    IN  PNET_BUFFER_LIST    NetBufferList,
    IN  BOOLEAN             bFromQueue);

NDIS_STATUS NICSendNetBuffer(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_TCB         pMpTcb,
    IN  PMP_FRAG_LIST   pFragList);

ULONG MpCopyPacket(
    IN  PMDL            CurrMdl,
    IN  PMP_TXBUF       pMpTxbuf);

ULONG MpCopyNetBuffer(
    IN  PNET_BUFFER     NetBuffer,
    IN  PMP_TXBUF       pMpTxbuf);


NDIS_STATUS NICStartSend(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_TCB         pMpTcb);

NDIS_STATUS MpHandleSendInterrupt(
    IN  PMP_ADAPTER     Adapter);

VOID MpHandleRecvInterrupt(
    IN  PMP_ADAPTER     Adapter);

NDIS_STATUS MpHandleMIIInterrupt(
    IN  PMP_ADAPTER     Adapter);

VOID NICReturnRFD(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_RFD         pMpRfd);

VOID MpFreeQueuedSendNetBufferLists(
    IN  PMP_ADAPTER     Adapter);

void FECFreeBusySendNetBufferLists(
    IN  PMP_ADAPTER     Adapter);

void NICResetRecv(
    IN  PMP_ADAPTER     Adapter);

VOID MpLinkDetectionDpc(
    IN  PVOID       SystemSpecific1,
    IN  PVOID       FunctionContext,
    IN  PVOID       SystemSpecific2,
    IN  PVOID       SystemSpecific3);

VOID
FECIndicateLinkState(
    IN  PMP_ADAPTER     Adapter
    );

PLIST_ENTRY GetListEntry(UINT IndexCount, PLIST_ENTRY  RecvList);
PMP_TCB GetTcb(PMP_ADAPTER  Adapter, INT TbdIndex);

//
// MP_INIT.C
//

NDIS_STATUS NICReadAdapterInfo(
    IN  PMP_ADAPTER     Adapter);

NDIS_STATUS MpAllocAdapterBlock(
    OUT PMP_ADAPTER    *pAdapter,
    IN  NDIS_HANDLE     MiniportAdapterHandle
    );

void MpFreeAdapter(
    IN  PMP_ADAPTER     Adapter);

NDIS_STATUS NICReadRegParameters(
    IN  PMP_ADAPTER     Adapter);

NDIS_STATUS NICAllocAdapterMemory(
    IN  PMP_ADAPTER     Adapter);

VOID NICInitSend(
    IN  PMP_ADAPTER     Adapter);

NDIS_STATUS NICInitRecv(
    IN  PMP_ADAPTER     Adapter);

ULONG NICAllocRfd(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_RFD         pMpRfd,
    IN  LONG            BufferIndex);

VOID NICFreeRfd(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_RFD         pMpRfd,
    IN  BOOLEAN         DispatchLevel);

//
// MP_REQ.C
//

NDIS_STATUS NICGetStatsCounters(
    IN  PMP_ADAPTER     Adapter,
    IN  NDIS_OID        Oid,
    OUT PULONG64        pCounter);

NDIS_STATUS NICSetPacketFilter(
    IN  PMP_ADAPTER     Adapter,
    IN  ULONG           PacketFilter);

NDIS_STATUS NICSetMulticastList(
    IN  PMP_ADAPTER     Adapter);

ULONG NICGetMediaConnectStatus(
    IN  PMP_ADAPTER     Adapter);

VOID
MPFillPoMgmtCaps (
    IN PMP_ADAPTER Adapter,
    IN OUT PNDIS_PNP_CAPABILITIES   pPower_Management_Capabilities,
    IN OUT  PNDIS_STATUS pStatus,
    IN OUT  PULONG pulInfoLen
    );

#endif  // MP_NIC_H
