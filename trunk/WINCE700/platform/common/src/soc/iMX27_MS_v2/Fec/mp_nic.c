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
    mp_nic.c

Abstract:
    This module contains miniport send/receive routines

Revision History:

Notes:

--*/

#include "precomp.h"
#include <xfilter.h>


#if DBG
#define _FILENUMBER     'CINM'
#endif
extern PCSP_FEC_REGS   gpFECReg;
const int NETBUFFER_SEND_THRESHOLD=8;
const int FREE_NETBUFFER_SEND_THRESHOLD=1;
const int RECV_MAX_ITERATIONS=16;

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

__inline PNET_BUFFER_LIST
FEC_FREE_SEND_NET_BUFFER(
    IN  PMP_ADAPTER Adapter,
    IN  PMP_TCB     pMpTcb
    )
/*++
Routine Description:

    Recycle a MP_TCB and complete the packet if necessary
    Assumption: Send spinlock has been acquired

Arguments:

    Adapter     Pointer to our adapter
    pMpTcb      Pointer to MP_TCB

Return Value:

    Return NULL if no NET_BUFFER_LIST is completed.
    Otherwise, return a pointer to a NET_BUFFER_LIST which has been completed.

--*/
{
    PNET_BUFFER         NetBuffer;
    PNET_BUFFER_LIST    NetBufferList;
    BOOLEAN             fWaitForMapping = FALSE;
    DBGPRINT(MP_TRACE, ("--> FEC_FREE_SEND_NET_BUFFER\n"));

    ASSERT(MP_TEST_FLAG(pMpTcb, fMP_TCB_IN_USE));

    fWaitForMapping = MP_TEST_FLAG(pMpTcb, fMP_TCB_WAIT_FOR_MAPPING);

    NetBuffer = pMpTcb->NetBuffer;
    NetBufferList = pMpTcb->NetBufferList;
    pMpTcb->NetBuffer = NULL;
    pMpTcb->Count = 0;

    Adapter->CurrSendHead = Adapter->CurrSendHead->Next;
    Adapter->CurrSendHead->Count = 0;
    //RETAILMSG(TRUE,(TEXT("SendIntr- Busy Send %d \n"), Adapter->nBusySend));

    if (NetBuffer && !fWaitForMapping)
    {
        //
        // Call ndis to free resouces allocated for a SG list
        //
        NdisDprReleaseSpinLock(&Adapter->SendLock);

        NdisMFreeNetBufferSGList(
                            Adapter->NdisMiniportDmaHandle,
                            pMpTcb->pSGList,
                            NetBuffer);
        NdisDprAcquireSpinLock(&Adapter->SendLock);
    }
    MP_CLEAR_FLAGS(pMpTcb);

    //
    // SendLock is hold
    //
    if (NetBufferList)
    {
        MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList)--;
        if (MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList) == 0)
        {
            DBGPRINT(MP_TRACE, ("Completing NetBufferList= "PTR_FORMAT"\n", NetBufferList));

            MP_GET_NET_BUFFER_LIST_NEXT_SEND(NetBufferList) = NULL;
            NET_BUFFER_LIST_NEXT_NBL(NetBufferList) = NULL;
            return NetBufferList;
        }
    }
    DBGPRINT(MP_TRACE, ("<-- FEC_FREE_SEND_NET_BUFFER\n"));
    return NULL;
}

NDIS_STATUS
MiniportSendNetBufferList(
    IN  PMP_ADAPTER         Adapter,
    IN  PNET_BUFFER_LIST    NetBufferList,
    IN  BOOLEAN             bFromQueue
    )
/*++
Routine Description:

    Do the work to send a packet
    Assumption: Send spinlock has been acquired

Arguments:

    Adapter             Pointer to our adapter
    NetBufferList       Pointer to the NetBufferList is going to send.
    bFromQueue          TRUE if it's taken from the send wait queue

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_PENDING         Put into the send wait queue
    NDIS_STATUS_HARD_ERRORS

    NOTE: SendLock is held, called at DISPATCH level
--*/
{
    NDIS_STATUS     Status = NDIS_STATUS_PENDING;
    NDIS_STATUS     SendStatus;
    PMP_TCB         pMpTcb = NULL;
    PMP_TXBUF       pMpTxBuf = NULL;
    PNET_BUFFER     NetBufferToSend;
    DBGPRINT(MP_TRACE, ("--> MiniportSendNetBufferList, NetBufferList="PTR_FORMAT"\n",
                            NetBufferList));

    SendStatus = NDIS_STATUS_SUCCESS;

    if (bFromQueue)
    {
        NetBufferToSend = MP_GET_NET_BUFFER_LIST_NEXT_SEND(NetBufferList);
        MP_GET_NET_BUFFER_LIST_NEXT_SEND(NetBufferList) = NULL;
    }
    else
    {
        NetBufferToSend = NET_BUFFER_LIST_FIRST_NB(NetBufferList);
    }
    for (;  NetBufferToSend != NULL;NetBufferToSend = NET_BUFFER_NEXT_NB(NetBufferToSend))
    {
        // If we run out of TCB
        //
        if (!MP_TCB_RESOURCES_AVAIABLE(Adapter))
        {
            DBGPRINT(MP_INFO, ("MiniportSendNetBufferList - Tcb not available \n"));

            ASSERT(NetBufferToSend != NULL);
            //
            // Put NetBufferList into wait queue
            //
            MP_GET_NET_BUFFER_LIST_NEXT_SEND(NetBufferList) = NetBufferToSend;
            if (!bFromQueue)
            {
                InsertHeadQueue(&Adapter->SendWaitQueue,
                                MP_GET_NET_BUFFER_LIST_LINK(NetBufferList));
                Adapter->nWaitSend++;

                RETAILMSG(TRUE,(TEXT("Res Not available Adding to Waiting Q{2} %d \n"),Adapter->nWaitSend));

                //Dump TBDs if we end up having all the TBs used up!
                RETAILMSG(TRUE,(TEXT("0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x \n"), 
                    Adapter->HwTbdChip[0]->ControlStatus,Adapter->HwTbdChip[1]->ControlStatus,Adapter->HwTbdChip[2]->ControlStatus,Adapter->HwTbdChip[3]->ControlStatus,
                    Adapter->HwTbdChip[4]->ControlStatus,Adapter->HwTbdChip[5]->ControlStatus,Adapter->HwTbdChip[6]->ControlStatus,Adapter->HwTbdChip[7]->ControlStatus));
                RETAILMSG(TRUE,(TEXT("0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x \n"),
                    Adapter->HwTbdChip[8]->ControlStatus,Adapter->HwTbdChip[9]->ControlStatus,Adapter->HwTbdChip[10]->ControlStatus,Adapter->HwTbdChip[11]->ControlStatus,
                    Adapter->HwTbdChip[12]->ControlStatus,Adapter->HwTbdChip[13]->ControlStatus,Adapter->HwTbdChip[14]->ControlStatus,Adapter->HwTbdChip[15]->ControlStatus));
                RETAILMSG(TRUE,(TEXT("0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x \n"), 
                    Adapter->HwTbdChip[16]->ControlStatus,Adapter->HwTbdChip[17]->ControlStatus,Adapter->HwTbdChip[18]->ControlStatus,Adapter->HwTbdChip[19]->ControlStatus,
                    Adapter->HwTbdChip[20]->ControlStatus,Adapter->HwTbdChip[21]->ControlStatus,Adapter->HwTbdChip[22]->ControlStatus,Adapter->HwTbdChip[23]->ControlStatus));
                RETAILMSG(TRUE,(TEXT("0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x 0x%4x \n"), 
                  Adapter->HwTbdChip[24]->ControlStatus,Adapter->HwTbdChip[25]->ControlStatus,Adapter->HwTbdChip[26]->ControlStatus,Adapter->HwTbdChip[27]->ControlStatus,
                    Adapter->HwTbdChip[28]->ControlStatus,Adapter->HwTbdChip[29]->ControlStatus,Adapter->HwTbdChip[30]->ControlStatus,Adapter->HwTbdChip[31]->ControlStatus));

                RETAILMSG(TRUE,(TEXT("\n Counters Where %d, With HW %d\n"), 
                    Adapter->HwSendTbdCurrentCount, Adapter->nBusySend));

            }
            //
            // The NetBufferList is already in the queue, we don't do anything
            //
            Adapter->SendingNetBufferList = NULL;
            NetBufferToSend = NULL;
            Status = NDIS_STATUS_RESOURCES;
            return Status;
        }
        //
        // Get TCB
        //
        pMpTcb = Adapter->CurrSendTail;
        DBGPRINT(MP_INFO, ("MiniportSendNetBufferList - Using Tcb "PTR_FORMAT" \n",pMpTcb));
        ASSERT(!MP_TEST_FLAG(pMpTcb, fMP_TCB_IN_USE)); //This Tcb should be free to use

        pMpTcb->Adapter = Adapter;
        pMpTcb->NetBuffer = NetBufferToSend;
        pMpTcb->NetBufferList = NetBufferList;

        ASSERT(MP_TEST_FLAG(Adapter, fMP_ADAPTER_SCATTER_GATHER));
        //
        // NOTE: net Buffer has to use new DMA APIs
        // The function is called at DISPATCH level
        //
        MP_SET_FLAG(pMpTcb, fMP_TCB_IN_USE);
        MP_SET_FLAG(pMpTcb, fMP_TCB_WAIT_FOR_MAPPING);


        ASSERT(Adapter->nBusySend <= Adapter->NumTcb);
        Adapter->CurrSendTail = Adapter->CurrSendTail->Next;
        DBGPRINT(MP_TRACE, ("MiniportSendNetBufferList- Next Available Tcb "PTR_FORMAT" \n",Adapter->CurrSendTail));

        MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, TRUE);

        DBGPRINT(MP_TRACE, ("MiniportSendNetBufferList, Sending for SG Processing with Tcb "PTR_FORMAT" Busy Send Count %d \n", pMpTcb,Adapter->nBusySend));
        SendStatus = NdisMAllocateNetBufferSGList(
                        Adapter->NdisMiniportDmaHandle,
                        NetBufferToSend,
                        pMpTcb,
                        NDIS_SG_LIST_WRITE_TO_DEVICE,
                        pMpTcb->ScatterGatherListBuffer,
                        Adapter->ScatterGatherListSize);

        MP_ACQUIRE_SPIN_LOCK(&Adapter->SendLock, TRUE);

        if (NDIS_STATUS_SUCCESS != SendStatus)
        {

            RETAILMSG(TRUE,(TEXT("SG Failed- Busy Send %d \n"), Adapter->nBusySend));

            DBGPRINT(MP_WARN, ("MiniportSendNetBufferList- NetBuffer "PTR_FORMAT" Couldnot be send to SG  \n", NetBufferToSend));

            Adapter->CurrSendTail = Adapter->CurrSendTail->Prev;
            DBGPRINT(MP_TRACE, ("MiniportSendNetBufferList- Next Available Tcb "PTR_FORMAT" \n",Adapter->CurrSendTail));

            MP_CLEAR_FLAGS(pMpTcb);
            pMpTcb->NetBuffer = NULL;
            pMpTcb->NetBufferList = NULL;
            //
            // We should fail the entire NET_BUFFER_LIST because the system
            // cannot map the NET_BUFFER
            //
            NET_BUFFER_LIST_STATUS(NetBufferList) = NDIS_STATUS_RESOURCES;

            for (; NetBufferToSend != NULL;
                   NetBufferToSend = NET_BUFFER_NEXT_NB(NetBufferToSend))
            {
                MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList)--;
            }

            break;
        }
    }
    //
    // All the NetBuffers in the NetBufferList has been processed,
    // If the NetBufferList is in queue now, dequeue it.
    //
    if (NetBufferToSend == NULL)
    {
        //if this packet came from the queue
        //but couldnt be sent, dont remove from queue
        //if sent remove from the queue
        if (bFromQueue)
        {
            RemoveHeadQueue(&Adapter->SendWaitQueue);
            Adapter->nWaitSend--;
        }
        Adapter->SendingNetBufferList = NULL;
    }else
    {
        DBGPRINT(MP_WARN, ("MiniportSendNetBufferList - Buffers pending \n"));
    }
    //
    // As far as the miniport knows, the NetBufferList has been sent out.
    // Complete the NetBufferList now
    //
    //Prevent multiplebuffer complete call
    if ((MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList) == 0) && (MP_GET_NET_BUFFER_LIST_NEXT_SEND(NetBufferList)!= NULL))
    {
        MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, TRUE);

        NET_BUFFER_LIST_NEXT_NBL(NetBufferList) = NULL;

        DBGPRINT(MP_TRACE, ("MiniportSendNetBufferList- SendingBufferComplete Buffer "PTR_FORMAT" Busy Send Count %d \n", NetBufferList,Adapter->nBusySend));
        NdisMSendNetBufferListsComplete(
            MP_GET_ADAPTER_HANDLE(Adapter),
            NetBufferList,
            NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL);

        MP_ACQUIRE_SPIN_LOCK(&Adapter->SendLock, TRUE);
    }
    DBGPRINT(MP_TRACE, ("<-- MiniportSendNetBufferList- Busy Count %d , Wait Send %d \n",Adapter->nBusySend,Adapter->nWaitSend));
    return Status;

}
/*++
Routine Description:

   Copy data in a packet to the specified location

Arguments:

    BytesToCopy          The number of bytes need to copy
    CurreentBuffer       The buffer to start to copy
    StartVa              The start address to copy the data to
    Offset               The start offset in the buffer to copy the data

Return Value:

    The number of bytes actually copied


--*/

ULONG
MpCopyNetBuffer(
    IN  PNET_BUFFER     NetBuffer,
    IN  PMP_TXBUF       pMpTxBuf
    )
{
    ULONG          CurrLength=0;
    PUCHAR         pSrc=NULL;
    PUCHAR         pDest;
    ULONG          BytesCopied = 0;
    ULONG          Offset;
    PMDL           CurrentMdl;
    ULONG          DataLength;

    DBGPRINT(MP_TRACE, ("--> MpCopyNetBuffer\n"));

    pDest = pMpTxBuf->pBuffer;
    CurrentMdl = NET_BUFFER_FIRST_MDL(NetBuffer);
    Offset = NET_BUFFER_DATA_OFFSET(NetBuffer);
    DataLength = NET_BUFFER_DATA_LENGTH(NetBuffer);

    while (CurrentMdl && DataLength > 0)
    {
        NdisQueryMdl(CurrentMdl, &pSrc, &CurrLength, NormalPagePriority);
        if (pSrc == NULL)
        {
            BytesCopied = 0;
            break;
        }
        //
        //  Current buffer length is greater than the offset to the buffer
        //
        if (CurrLength > Offset)
        {
            pSrc += Offset;
            CurrLength -= Offset;

            if (CurrLength > DataLength)
            {
                CurrLength = DataLength;
            }
            DataLength -= CurrLength;
            NdisMoveMemory(pDest, pSrc, CurrLength);
            BytesCopied += CurrLength;

            pDest += CurrLength;
            Offset = 0;
        }
        else
        {
            Offset -= CurrLength;
        }
        NdisGetNextMdl(CurrentMdl, &CurrentMdl);

    }
    if ((BytesCopied != 0) && (BytesCopied < NIC_MIN_PACKET_SIZE))
    {
        NdisZeroMemory(pDest, NIC_MIN_PACKET_SIZE - BytesCopied);
    }
    NdisAdjustMdlLength(pMpTxBuf->Mdl, BytesCopied);
    NdisFlushBuffer(pMpTxBuf->Mdl, TRUE);

    ASSERT(BytesCopied <= pMpTxBuf->BufferSize);
    DBGPRINT(MP_TRACE, ("<-- MpCopyNetBuffer\n"));
    return BytesCopied;
}


NDIS_STATUS
NICSendNetBuffer(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_TCB         pMpTcb,
    IN  PMP_FRAG_LIST   pFragList
    )
/*++
Routine Description:

    NIC specific send handler
    Assumption: Send spinlock has been acquired

Arguments:

    Adapter     Pointer to our adapter
    pMpTcb      Pointer to MP_TCB
    pFragList   The pointer to the frag list to be filled

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_HARD_ERRORS
NOTE: called with send lock held.

--*/
{
    NDIS_STATUS  Status;
    PTBD_STRUC      pHwTbd = pMpTcb->HwTbd;
    ULONG        DataOffset;
    PNET_BUFFER  NetBuffer;

    DBGPRINT(MP_TRACE, ("--> NICSendNetBuffer\n"));

    NetBuffer = pMpTcb->NetBuffer;
    ASSERT(1 == pFragList->NumberOfElements);

    if (MP_TEST_FLAG(pMpTcb, fMP_TCB_USE_LOCAL_BUF))
    {
        if (pFragList->Elements[0].Length)
        {
            pHwTbd->BufferAddress = NdisGetPhysicalAddressLow(pFragList->Elements[0].Address);
            pHwTbd->DataLen = (USHORT) pFragList->Elements[0].Length;
            Status = NICStartSend(Adapter, pMpTcb);
        }
    }else
    {
        //
        // NDIS starts creating SG list from CurrentMdl, the driver don't need to worry
        // about more data will be mapped because NDIS will only map the data to length of
        // (NetBuffer->DataLength + NetBuffer->CurrentMdlOffset).The driver only need to skip
        // the offset.
        //
        DataOffset = (ULONG)(NET_BUFFER_CURRENT_MDL_OFFSET(NetBuffer));
        pHwTbd->BufferAddress = NdisGetPhysicalAddressLow(pFragList->Elements[0].Address) + DataOffset;
        pHwTbd->DataLen = (USHORT) (pFragList->Elements[0].Length - DataOffset);
        Status = NICStartSend(Adapter, pMpTcb);
    }
    DBGPRINT(MP_TRACE, ("<-- NICSendNetBuffer\n"));
    return Status;
}

NDIS_STATUS
NICStartSend(
    IN  PMP_ADAPTER  Adapter,
    IN  PMP_TCB      pMpTcb
    )
/*++

Routine Description:

    Issue a send command to the NIC
    Assumption: Send spinlock has been acquired

Arguments:

    Adapter     Pointer to our adapter
    pMpTcb      Pointer to MP_TCB

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_HARD_ERRORS

--*/
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PMP_TCB         pPrevTcb = NULL;
    PMP_TCB         pNextTcb = NULL;
    PMP_TCB         pCurrTcb = NULL;
    UINT            TbdCount=0;
    UINT            TbdStart=0;
    UINT            nSendId = Adapter->nPendingSendPacket;
    UINT            nPendingPackets = ++Adapter->nPendingSendPacket;

    DBGPRINT(MP_TRACE, ("--> NICStartSend Tcb "PTR_FORMAT" - %d \n",pMpTcb,Adapter->HwSendTbdCurrentCount));
    DBGPRINT(MP_TRACE, ("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n",
                  Adapter->HwTbdChip[0]->ControlStatus,Adapter->HwTbdChip[1]->ControlStatus,Adapter->HwTbdChip[2]->ControlStatus,Adapter->HwTbdChip[3]->ControlStatus,
                  Adapter->HwTbdChip[4]->ControlStatus,Adapter->HwTbdChip[5]->ControlStatus,Adapter->HwTbdChip[6]->ControlStatus,Adapter->HwTbdChip[7]->ControlStatus,
                  Adapter->HwTbdChip[8]->ControlStatus,Adapter->HwTbdChip[9]->ControlStatus,Adapter->HwTbdChip[10]->ControlStatus,Adapter->HwTbdChip[11]->ControlStatus,
                  Adapter->HwTbdChip[12]->ControlStatus,Adapter->HwTbdChip[13]->ControlStatus,Adapter->HwTbdChip[14]->ControlStatus,Adapter->HwTbdChip[15]->ControlStatus));

    if( (pMpTcb->HwTbd->DataLen > NIC_MAX_PACKET_SIZE) ||(pMpTcb->HwTbd->DataLen < ETHER_HDR_SIZE))
    {
        Adapter->TxdStatus.FramesXmitBad++;
        DBGPRINT(MP_TRACE, ("<-- NICStartSend- Bad TX Buf Status %d \n", NDIS_STATUS_FAILURE));

        return NDIS_STATUS_FAILURE;
    }
    TbdCount =  Adapter->HwSendTbdCurrentCount;

    //
    // If any TCB in front of this one waiting for mapping, set the waiting
    // for mapping flag, and don't issue the send command.
    //
    pPrevTcb = pMpTcb->Prev;

    if ((pMpTcb != Adapter->CurrSendHead)
            && (MP_TEST_FLAG(pPrevTcb, fMP_TCB_FLAGS_MAPPING)))
    {
        ASSERT(!MP_TEST_FLAG(pMpTcb, fMP_TCB_WAIT_FOR_MAPPING));
        MP_SET_FLAG(pMpTcb, fMP_TCB_PREV_WAIT_FOR_MAPPING);
    }

    //
    // If current one is waiting for mapping to be completed, the previous
    // TCB should have finished the mapping
    //
    if (MP_TEST_FLAG(pMpTcb, fMP_TCB_WAIT_FOR_MAPPING))
    {
        if (pMpTcb != Adapter->CurrSendHead)
        {
            ASSERT(!MP_TEST_FLAG(pPrevTcb, fMP_TCB_FLAGS_MAPPING));
        }

        MP_CLEAR_FLAG(pMpTcb, fMP_TCB_WAIT_FOR_MAPPING);

        pNextTcb = pMpTcb->Next;
        while (MP_TEST_FLAG(pNextTcb, fMP_TCB_PREV_WAIT_FOR_MAPPING))
        {
            ASSERT(!MP_TEST_FLAG(pNextTcb, fMP_TCB_WAIT_FOR_MAPPING));
            MP_CLEAR_FLAG(pNextTcb, fMP_TCB_PREV_WAIT_FOR_MAPPING);
            pNextTcb = pNextTcb->Next;
        }
    }
    TbdStart = TbdCount;

    //if any packet being sent by HW, setup the BDs
    //sequentially at the next available BD slot
    if( (Adapter->HwTbdChip[TbdCount]->ControlStatus & BD_ENET_TX_READY) ||
        (Adapter->HwTbdChip[TbdCount]->ControlStatus & BD_ENET_TX_PAD) )
    {
        TbdStart = TbdCount;

        //Adjust the starting point if we 'were' here earlier...
        if(nPendingPackets)
        {
            TbdCount =  (TbdCount+nPendingPackets-1) % Adapter->NumTcb;
        }else
            TbdCount =  (TbdCount+1) % Adapter->NumTcb;

        while ((Adapter->HwTbdChip[TbdCount]->ControlStatus & BD_ENET_TX_READY))
        {
            TbdCount =  (TbdCount+1) % Adapter->NumTcb;
            if(TbdStart == TbdCount)
                break;
        }
        //exhausted all the BDs?
        if(TbdStart == TbdCount)
        {
            RETAILMSG(TRUE, (TEXT("NICStartSend[%d]- Tbd Count %d is not ready for TX \n"), nSendId,TbdCount));
            INSREG32BF(&gpFECReg->TDAR, FEC_TDAR_ACTIVE, FEC_TDAR_ACTIVE_ACTIVE);

            --Adapter->nPendingSendPacket;  //we havent sent this packet
            // Transmit buffers are full
            return NDIS_STATUS_HARD_ERRORS;
        }
    }
    // Set up the transmit buffer descriptor
    // Note- BD_ENET_TX_PAD will be used in the intr handler
    Adapter->HwTbdChip[TbdCount]->ControlStatus &= ~BD_ENET_TX_STATS;
    Adapter->HwTbdChip[TbdCount]->ControlStatus |= (BD_ENET_TX_LAST  | BD_ENET_TX_TC | BD_ENET_TX_PAD);
    Adapter->HwTbdChip[TbdCount]->BufferAddress = pMpTcb->HwTbd->BufferAddress;
    Adapter->HwTbdChip[TbdCount]->DataLen = pMpTcb->HwTbd->DataLen;

    //Restore the buffer Address to the cache aligned preallocated buffer
    pMpTcb->HwTbd->BufferAddress = (ULONG) pMpTcb->MpTxBuf->BufferPa.QuadPart;
    pMpTcb->HwTbd->DataLen = 0;


    pMpTcb->HwTbdIndex = TbdCount;

    DBGPRINT(MP_TRACE, ("NICStartSend- HW TX for Tcb "PTR_FORMAT" with index %d ,and TBD %d Buf "PTR_FORMAT" Len %d \n", pMpTcb,pMpTcb->HwTbdIndex,pMpTcb->HwTbd->ControlStatus,pMpTcb->HwTbd->BufferAddress,pMpTcb->HwTbd->DataLen));
    DBGPRINT(MP_TRACE, ("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n",
      Adapter->HwTbdChip[0]->ControlStatus,Adapter->HwTbdChip[1]->ControlStatus,Adapter->HwTbdChip[2]->ControlStatus,Adapter->HwTbdChip[3]->ControlStatus,
      Adapter->HwTbdChip[4]->ControlStatus,Adapter->HwTbdChip[5]->ControlStatus,Adapter->HwTbdChip[6]->ControlStatus,Adapter->HwTbdChip[7]->ControlStatus,
      Adapter->HwTbdChip[8]->ControlStatus,Adapter->HwTbdChip[9]->ControlStatus,Adapter->HwTbdChip[10]->ControlStatus,Adapter->HwTbdChip[11]->ControlStatus,
      Adapter->HwTbdChip[12]->ControlStatus,Adapter->HwTbdChip[13]->ControlStatus,Adapter->HwTbdChip[14]->ControlStatus,Adapter->HwTbdChip[15]->ControlStatus));

    Adapter->CurrentTx = pMpTcb;
    Adapter->HwTbdChip[TbdCount]->ControlStatus |= BD_ENET_TX_READY;
    pMpTcb->HwTbd->ControlStatus = Adapter->HwTbdChip[TbdCount]->ControlStatus;
    Adapter->nBusySend++;

    // Trigger transmission start
    INSREG32BF(&gpFECReg->TDAR, FEC_TDAR_ACTIVE, FEC_TDAR_ACTIVE_ACTIVE);

    DBGPRINT(MP_TRACE, ("<-- NICStartSend- Status %d \n", Status));
    return Status;
}

NDIS_STATUS
MpHandleSendInterrupt(
    IN  PMP_ADAPTER  Adapter
    )
/*++
Routine Description:

    Interrupt handler for sending processing
    Re-claim the send resources, complete sends and get more to send from the send wait queue
    Assumption: Send spinlock has been acquired

Arguments:

    Adapter     Pointer to our adapter

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_HARD_ERRORS
    NDIS_STATUS_PENDING

--*/
{
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    PMP_TCB             pMpTcb;
    PNET_BUFFER_LIST    NetBufferList;
    PNET_BUFFER_LIST    LastNetBufferList = NULL;
    PNET_BUFFER_LIST    CompleteNetBufferLists = NULL;
    BOOLEAN             TcbResult = FALSE;
    int                 nPacketProcessed = 0;

    DBGPRINT(MP_TRACE, ("---> MpHandleSendInterrupt Current Index %d\n", Adapter->HwSendTbdCurrentCount));
    DBGPRINT(MP_TRACE, ("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n",
      Adapter->HwTbdChip[0]->ControlStatus,Adapter->HwTbdChip[1]->ControlStatus,Adapter->HwTbdChip[2]->ControlStatus,Adapter->HwTbdChip[3]->ControlStatus,
      Adapter->HwTbdChip[4]->ControlStatus,Adapter->HwTbdChip[5]->ControlStatus,Adapter->HwTbdChip[6]->ControlStatus,Adapter->HwTbdChip[7]->ControlStatus,
      Adapter->HwTbdChip[8]->ControlStatus,Adapter->HwTbdChip[9]->ControlStatus,Adapter->HwTbdChip[10]->ControlStatus,Adapter->HwTbdChip[11]->ControlStatus,
      Adapter->HwTbdChip[12]->ControlStatus,Adapter->HwTbdChip[13]->ControlStatus,Adapter->HwTbdChip[14]->ControlStatus,Adapter->HwTbdChip[15]->ControlStatus));

    while  ( ((Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_READY) == 0 ) &&
        (Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_PAD) )
    {
        nPacketProcessed++;

        // Bad Packet? Update the outgoing statistics
        if(Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & (BD_ENET_TX_HB | BD_ENET_TX_LC |
               BD_ENET_TX_RL | BD_ENET_TX_UN |
               BD_ENET_TX_CSL))
        {
            Adapter->TxdStatus.FramesXmitBad++;

            if(Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_HB)  // No heartbeat
                Adapter->TxdStatus.FramesXmitHBErrors++;

            if(Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_LC)  // Late collision
                Adapter->TxdStatus.FramesXmitCollisionErrors++;

            if(Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_RL)  // Retrans limit
                Adapter->TxdStatus.FramesXmitAbortedErrors++;

            if(Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_UN)  // Underrun
                Adapter->TxdStatus.FramesXmitUnderrunErrors++;

            if(Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_CSL) // Carrier lost
                Adapter->TxdStatus.FramsXmitCarrierErrors++;

            DBGPRINT(MP_TRACE, ("HandleSendInterrupt- Bad Packet for index %d \n", Adapter->HwSendTbdCurrentCount));
            RETAILMSG(TRUE, (TEXT("Send- Packet Error %x, - index %d \n"), Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus, Adapter->HwSendTbdCurrentCount));

            pMpTcb = GetTcb(Adapter,(INT) Adapter->HwSendTbdCurrentCount);
            Adapter->CurrentTx = NULL;

            //clear out the pad bit
            Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus &= ~BD_ENET_TX_PAD;

            if (!pMpTcb)
            {
               ASSERT(FALSE);
               DBGPRINT(MP_ERROR, ("HandleSendInterrupt- Couldnt find Tcb for index %d \n", Adapter->HwSendTbdCurrentCount));
               RETAILMSG(TRUE, (TEXT("HandleSendInterrupt- Couldnt find Tcb for index %d \n"), Adapter->HwSendTbdCurrentCount));
            }else
            {
                pMpTcb->HwTbdIndex = -1;
                pMpTcb->HwTbd->ControlStatus = Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus;
            }
            if(Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_WRAP)
            {
                Adapter->HwSendTbdCurrentCount = 0;
            }
            else
            {
                Adapter->HwSendTbdCurrentCount++; //error case
            }
            Adapter->nPendingSendPacket--;
            Adapter->nBusySend--;
        }
        else
        {
            DBGPRINT(MP_TRACE, ("HandleSendInterrupt- current index %d \n", Adapter->HwSendTbdCurrentCount));
            pMpTcb = GetTcb(Adapter,(INT) Adapter->HwSendTbdCurrentCount);
            Adapter->CurrentTx = NULL;

            if (!pMpTcb)
            {
               DBGPRINT(MP_ERROR, ("HandleSendInterrupt- Couldnt find Tcb for index %d \n", Adapter->HwSendTbdCurrentCount));
               RETAILMSG(TRUE, (TEXT("HandleSendInterrupt- Couldnt find Tcb for index %d \n"), Adapter->HwSendTbdCurrentCount));

               ASSERT(FALSE);
               //clear out the pad bit
               Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus &= ~BD_ENET_TX_PAD;
               if(Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_WRAP)
               {
                    Adapter->HwSendTbdCurrentCount = 0;
               }
               else
               {
                    Adapter->HwSendTbdCurrentCount++; //normal case..no tcb
               }
               Adapter->nPendingSendPacket--;
               Adapter->nBusySend--;
               continue;
            }
            ASSERT(pMpTcb->HwTbdIndex == Adapter->HwSendTbdCurrentCount);
            DBGPRINT(MP_TRACE, ("HandleSendInterrupt- Tcb "PTR_FORMAT" for Index %d \n", pMpTcb, Adapter->HwSendTbdCurrentCount));
            pMpTcb->HwTbdIndex = -1;

            Adapter->TxdStatus.FramesXmitGood++;
            //clear out the pad bit
            Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus &= ~BD_ENET_TX_PAD;
            pMpTcb->HwTbd->ControlStatus = Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus;

            if(Adapter->HwTbdChip[Adapter->HwSendTbdCurrentCount]->ControlStatus & BD_ENET_TX_WRAP)
            {
                Adapter->HwSendTbdCurrentCount = 0;
            }
            else
            {
                Adapter->HwSendTbdCurrentCount++; //normal case
            }
            Adapter->nPendingSendPacket--;
            Adapter->nBusySend--;
            NetBufferList = FEC_FREE_SEND_NET_BUFFER(Adapter, pMpTcb);
            //
            // One NetBufferList got complete
            //
            if (NetBufferList != NULL)
            {
                NET_BUFFER_LIST_STATUS(NetBufferList) = NDIS_STATUS_SUCCESS;
                if (CompleteNetBufferLists == NULL)
                {
                    CompleteNetBufferLists = NetBufferList;
                }
                else
                {
                    NET_BUFFER_LIST_NEXT_NBL(LastNetBufferList) = NetBufferList;
                }
                NET_BUFFER_LIST_NEXT_NBL(NetBufferList) = NULL;
                LastNetBufferList = NetBufferList;
            }
        }
    }
    //
    // Complete the NET_BUFFER_LISTs
    //
    if (CompleteNetBufferLists != NULL)
    {
        NDIS_HANDLE MiniportHandle = MP_GET_ADAPTER_HANDLE(Adapter);

        NdisDprReleaseSpinLock(&Adapter->SendLock);
        DBGPRINT(MP_TRACE, ("HandleSendInterrupt- SendingBufferComplete for Tcb "PTR_FORMAT" Buffer "PTR_FORMAT", Busy Send Count %d \n", pMpTcb,CompleteNetBufferLists,Adapter->nBusySend));

        NdisMSendNetBufferListsComplete(
                MiniportHandle,
                CompleteNetBufferLists,
                NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL);

        NdisDprAcquireSpinLock(&Adapter->SendLock);
    }
    //
    // If we queued any transmits because we didn't have any TCBs earlier,
    // dequeue and send those packets now, as long as we have free TCBs.
    //
    if (MP_IS_READY(Adapter))
    {
        while (!IsQueueEmpty(&Adapter->SendWaitQueue) &&
            (MP_TCB_RESOURCES_AVAIABLE(Adapter) &&
            Adapter->SendingNetBufferList == NULL))
        {
            PQUEUE_ENTRY pEntry;
            //
            // We cannot remove it now, we just need to get the head
            //
            pEntry = GetHeadQueue(&Adapter->SendWaitQueue);
            ASSERT(pEntry);
            NetBufferList = MP_GET_NET_BUFFER_LIST_FROM_QUEUE_LINK (pEntry);

            DBGPRINT(MP_INFO, ("MpHandleSendInterrupt - send a queued NetBufferList "PTR_FORMAT" \n",NetBufferList));
            ASSERT(Adapter->SendingNetBufferList == NULL);
            Adapter->SendingNetBufferList = NetBufferList;
            Status = MiniportSendNetBufferList(Adapter, NetBufferList, TRUE);
            //
            // If we failed to send
            //
            if (Status != NDIS_STATUS_SUCCESS && Status != NDIS_STATUS_PENDING)
            {
                break;
            }
        }
    }
    DBGPRINT(MP_TRACE, ("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n",
        Adapter->HwTbdChip[0]->ControlStatus,Adapter->HwTbdChip[1]->ControlStatus,Adapter->HwTbdChip[2]->ControlStatus,Adapter->HwTbdChip[3]->ControlStatus,
        Adapter->HwTbdChip[4]->ControlStatus,Adapter->HwTbdChip[5]->ControlStatus,Adapter->HwTbdChip[6]->ControlStatus,Adapter->HwTbdChip[7]->ControlStatus,
        Adapter->HwTbdChip[8]->ControlStatus,Adapter->HwTbdChip[9]->ControlStatus,Adapter->HwTbdChip[10]->ControlStatus,Adapter->HwTbdChip[11]->ControlStatus,
        Adapter->HwTbdChip[12]->ControlStatus,Adapter->HwTbdChip[13]->ControlStatus,Adapter->HwTbdChip[14]->ControlStatus,Adapter->HwTbdChip[15]->ControlStatus));
    DBGPRINT(MP_TRACE, ("<--- MpHandleSendInterrupt; Current Index %d; Packet Processed %d\n",Adapter->HwSendTbdCurrentCount, nPacketProcessed));
    return Status;
}

VOID
MpHandleRecvInterrupt(
    IN  PMP_ADAPTER  Adapter
    )
/*++
Routine Description:

    Interrupt handler for receive processing
    Put the received packets into an array and call NdisMIndicateReceivePacket
    If we run low on RFDs, allocate another one
    Assumption: Rcv spinlock has been acquired

Arguments:

    Adapter     Pointer to our adapter

Return Value:

    None

--*/
{
    PMP_RFD             pMpRfd=NULL;
    PHW_RFD             pHwRfd=NULL;
    PHW_RFD             pHwRfdStart=NULL;
    PNET_BUFFER_LIST    FreeNetBufferList=NULL;
    PNET_BUFFER_LIST    FreeNetBufferListFull=NULL;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    PNET_BUFFER_LIST    NetBufferList = NULL;
    PNET_BUFFER         NetBuffer=NULL;
    LONG                Count;
    ULONG               ReceiveFlags = NDIS_RECEIVE_FLAGS_DISPATCH_LEVEL;
    UINT                nRbdStart = Adapter->HwRecvRbdCurrentCount;

    PNET_BUFFER_LIST    PrevNetBufferList = NULL;
    PNET_BUFFER_LIST    PrevFreeNetBufferList = NULL;
    PNET_BUFFER_LIST    PrevFreeNetBufferListFull = NULL;
    UINT                NetBufferListCount=0;
    UINT                NetBufferListFreeCount=0;
    int                 nInteration=0;

    // add an extra receive ref just in case we indicte up with status resources
    //
    MP_INC_RCV_REF(Adapter);

    pHwRfdStart = (PHW_RFD) Adapter->HwRecvMemAllocVa;

    while((!((pHwRfdStart[Adapter->HwRecvRbdCurrentCount]).ControlStatus & BD_ENET_RX_EMPTY))&& ((pHwRfdStart[Adapter->HwRecvRbdCurrentCount]).ControlStatus & BD_ENET_RX_PAD) && (nInteration < RECV_MAX_ITERATIONS) )
    {

        //DBGPRINT(MP_TRACE, ("MpHandleRecvInterrupt processing Index %d\n",Adapter->HwRecvRbdCurrentCount));

        pHwRfd = &pHwRfdStart[Adapter->HwRecvRbdCurrentCount];
        //clear out the RX Pad bit to indicate this RBD is processed
        pHwRfd->ControlStatus &= ~BD_ENET_RX_PAD;

        // If NDIS is pausing the miniport or miniport is paused
        // IGNORE the recvs
        //
        if ((Adapter->AdapterState == NicPausing) ||
            (Adapter->AdapterState == NicPaused))
        {
            DBGPRINT(MP_TRACE, ("Recv- Processing skipped - State %d \n", Adapter->AdapterState));
            pHwRfd->ControlStatus &= ~BD_ENET_RX_STATS;
            pHwRfd->ControlStatus |= (BD_ENET_RX_EMPTY|BD_ENET_RX_PAD);

            if(pHwRfd->ControlStatus & BD_ENET_RX_WRAP)
            {
                Adapter->HwRecvRbdCurrentCount = 0;
            }
            else
            {
                Adapter->HwRecvRbdCurrentCount++;  //NIC paused/pausing case
            }
            INSREG32BF(&gpFECReg->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);
            ++nInteration;
            continue;
        }


        // Is this packet completed?
        //
        if(pHwRfd->ControlStatus & (BD_ENET_RX_LG | BD_ENET_RX_NO | BD_ENET_RX_CR |
                                           BD_ENET_RX_OV | BD_ENET_RX_CL ))
        {
            Adapter->RcvStatus.FrameRcvErrors++;

            if(pHwRfd->ControlStatus & BD_ENET_RX_LG)  // too long frame
                Adapter->RcvStatus.FrameRcvExtraDataErrors++;

            if(pHwRfd->ControlStatus & BD_ENET_RX_NO)  // no-octet aligned frame
                Adapter->RcvStatus.FrameRcvAllignmentErrors++;

            if(pHwRfd->ControlStatus & BD_ENET_RX_CR)  // CRC error
                Adapter->RcvStatus.FrameRcvCRCErrors++;

            if(pHwRfd->ControlStatus & BD_ENET_RX_OV)  // receive FIFO overrun
                Adapter->RcvStatus.FrameRcvOverrunErrors;

            if(pHwRfd->ControlStatus & BD_ENET_RX_CL)  // late collisions error
                Adapter->RcvStatus.FrameRcvLCErrors++;

            //Wrap Around
            if(pHwRfd->ControlStatus & BD_ENET_RX_WRAP)
            {
                Adapter->HwRecvRbdCurrentCount = 0;
            }
            else
            {
                Adapter->HwRecvRbdCurrentCount++;  //bad packet case
            }
            // Clear the status flags for this BD
            pHwRfd->ControlStatus &= ~BD_ENET_RX_STATS;

            // Mark the buffer as empty
            pHwRfd->ControlStatus |= (BD_ENET_RX_EMPTY|BD_ENET_RX_PAD);
            pHwRfd->DataLen = 0;
            INSREG32BF(&gpFECReg->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);
            ++nInteration;
            continue;
        }

        if(0 == pHwRfd->DataLen )
        {
            //Wrap Around
            if(pHwRfd->ControlStatus & BD_ENET_RX_WRAP)
            {
                Adapter->HwRecvRbdCurrentCount = 0;
            }
            else
            {
                Adapter->HwRecvRbdCurrentCount++;   //zero packet size cases
            }
            // Clear the status flags for this BD
            pHwRfd->ControlStatus &= ~BD_ENET_RX_STATS;

            // Mark the buffer as empty
            pHwRfd->ControlStatus |= (BD_ENET_RX_EMPTY|BD_ENET_RX_PAD);
            INSREG32BF(&gpFECReg->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);

            ++nInteration;
            continue;
        }


        //We have a GOOD packet now..Proceed to send to NDIS

        //
        // Process up to the array size RFD's
        //
        if (IsListEmpty(&Adapter->RecvList))
        {
            ASSERT(Adapter->nReadyRecv == 0);
            break;
        }

        //Get the MP_RFD for the current Recv BD.
        pMpRfd = (PMP_RFD)Adapter->pMpRfdList[Adapter->HwRecvRbdCurrentCount];

        ++nInteration;

        //
        // Get the associated HW_RFD
        //
        pHwRfd = pMpRfd->HwRfd;

        // Get the packet size
        //
        pMpRfd->PacketSize = pMpRfd->HwRfd->DataLen;
        pMpRfd->PacketSize -= PKT_CRC_SIZE;

        //
        // Remove the RFD from the List
        //
        //This will be corresponding to the HwRecvRbdCurrentCount
        RemoveEntryList((PLIST_ENTRY)pMpRfd);
        Adapter->nReadyRecv--;
        ASSERT(Adapter->nReadyRecv >= 0);

        ASSERT(MP_TEST_FLAG(pMpRfd, fMP_RFD_RECV_READY));
        MP_CLEAR_FLAG(pMpRfd, fMP_RFD_RECV_READY);

        NetBuffer = NET_BUFFER_LIST_FIRST_NB(pMpRfd->NetBufferList);

        // During the call NdisAllocateNetBufferAndNetBufferList to allocate the NET_BUFFER_LIST, NDIS already
        // initializes DataOffset, CurrentMdl and CurrentMdlOffset, here the driver needs to update DataLength
        // in the NET_BUFFER to reflect the received frame size.
        //
        NdisAdjustNetBufferCurrentMdl(NetBuffer);
        NET_BUFFER_DATA_LENGTH(NetBuffer) = pMpRfd->PacketSize;
        NdisAdjustMdlLength(pMpRfd->Mdl, pMpRfd->PacketSize);
        NdisFlushBuffer(pMpRfd->Mdl, FALSE);

        //
        // set the status on the packet, either resources or success
        //
        if (Adapter->nReadyRecv >= MIN_NUM_RFD)
        {
            //
            // Success case: NDIS will pend the NetBufferLists
            //
            MP_SET_FLAG(pMpRfd, fMP_RFD_RECV_PEND);
            MP_INC_RCV_REF(Adapter);

            //NetBufferList (local variable) will be the start of 'NetBufferList'List
            //This will be send to NDIS.
            if (NetBufferList == NULL)
            {
                NetBufferList = pMpRfd->NetBufferList;
            }
            else
            {
                NET_BUFFER_LIST_NEXT_NBL(PrevNetBufferList) = pMpRfd->NetBufferList;
            }
            NET_BUFFER_LIST_NEXT_NBL(pMpRfd->NetBufferList) = NULL;
            MP_CLEAR_FLAG(pMpRfd->NetBufferList, (NET_BUFFER_LIST_FLAGS(pMpRfd->NetBufferList) & NBL_FLAGS_MINIPORT_RESERVED));
            PrevNetBufferList = pMpRfd->NetBufferList;
            NetBufferListCount++;
        }
        else
        {
            DBGPRINT(MP_TRACE, ("Recv- packet "PTR_FORMAT" will be held back \n", pMpRfd->NetBufferList ));

            //
            // Resources case: Miniport will retain ownership of the NetBufferLists
            //
            MP_SET_FLAG(pMpRfd, fMP_RFD_RESOURCES);

            //FreeNetBufferList (local variable) will be the start of 'NetBufferList'List
            //This will be retained at the miniport.
            if (FreeNetBufferList == NULL)
            {
                FreeNetBufferList = pMpRfd->NetBufferList;
                
                if(FreeNetBufferListFull == NULL)
                {
                    //Keep the start of the NetbufferList for use later
                    FreeNetBufferListFull = FreeNetBufferList;
                }
                else
                {
                    //Connect the new Freebuffer to the Full FreebufferList
                    NET_BUFFER_LIST_NEXT_NBL(PrevFreeNetBufferListFull) = FreeNetBufferList;
                }
            }
            else
            {
                NET_BUFFER_LIST_NEXT_NBL(PrevFreeNetBufferList) = pMpRfd->NetBufferList;
            }
#ifndef UNDER_CE
#pragma prefast(suppress: 8182, "pMpRfd->NetBufferList can never to NULL")
#endif  //  UNDER_CE
            NET_BUFFER_LIST_NEXT_NBL(pMpRfd->NetBufferList) = NULL;
            PrevFreeNetBufferList = pMpRfd->NetBufferList;
            MP_CLEAR_FLAG(pMpRfd->NetBufferList, (NET_BUFFER_LIST_FLAGS(pMpRfd->NetBufferList) & NBL_FLAGS_MINIPORT_RESERVED));
            NetBufferListFreeCount++;
        }

        //Wrap Around
        if(pHwRfd->ControlStatus & BD_ENET_RX_WRAP)
        {
            Adapter->HwRecvRbdCurrentCount = 0;
        }
        else
        {
            Adapter->HwRecvRbdCurrentCount++;  //normal case
        }

        //
        // Indicate the NetBufferLists to NDIS
        //
        if((NetBufferListCount == NETBUFFER_SEND_THRESHOLD) || (NetBufferListFreeCount == FREE_NETBUFFER_SEND_THRESHOLD))
        {
            NdisDprReleaseSpinLock(&Adapter->RcvLock);

            if(NetBufferList != NULL)
            {
                //Update the number of outstanding Recvs
                Adapter->PoMgmt.OutstandingRecv += NetBufferListCount;
                ReceiveFlags = NDIS_RECEIVE_FLAGS_DISPATCH_LEVEL;

                NdisMIndicateReceiveNetBufferLists(
                        MP_GET_ADAPTER_HANDLE(Adapter),
                        NetBufferList,
                        NDIS_DEFAULT_PORT_NUMBER,
                        NetBufferListCount,
                        ReceiveFlags);

                NetBufferList = NULL;
                NetBufferListCount = 0;
            }
            if(FreeNetBufferList != NULL)
            {
                NDIS_SET_RECEIVE_FLAG(ReceiveFlags, NDIS_RECEIVE_FLAGS_DISPATCH_LEVEL|NDIS_RECEIVE_FLAGS_RESOURCES);

                NdisMIndicateReceiveNetBufferLists(
                        MP_GET_ADAPTER_HANDLE(Adapter),
                        FreeNetBufferList,
                        NDIS_DEFAULT_PORT_NUMBER,
                        NetBufferListFreeCount,
                        ReceiveFlags);

                NetBufferListFreeCount = 0;

                //Start the freebufferlist since we have sent this set.
                FreeNetBufferList = NULL;
                PrevFreeNetBufferListFull = PrevFreeNetBufferList;

            }

            NdisDprAcquireSpinLock(&Adapter->RcvLock);
        }
    }
    //send the residues to NDIS..
    if ((NetBufferListCount != 0) || (NetBufferListFreeCount != 0))
    {
        NdisDprReleaseSpinLock(&Adapter->RcvLock);
        if(NetBufferList != NULL)
        {
            ReceiveFlags = NDIS_RECEIVE_FLAGS_DISPATCH_LEVEL;

            //Update the number of outstanding Recvs
            Adapter->PoMgmt.OutstandingRecv += NetBufferListCount;

            NdisMIndicateReceiveNetBufferLists(
                    MP_GET_ADAPTER_HANDLE(Adapter),
                    NetBufferList,
                    NDIS_DEFAULT_PORT_NUMBER,
                    NetBufferListCount,
                    ReceiveFlags);
        }
        if (FreeNetBufferList != NULL)
        {
            NDIS_SET_RECEIVE_FLAG(ReceiveFlags, NDIS_RECEIVE_FLAGS_DISPATCH_LEVEL|NDIS_RECEIVE_FLAGS_RESOURCES);

            DBGPRINT(MP_TRACE, ("**** Recv- Sending Packet "PTR_FORMAT" to NDIS with heldback %d packets \n", FreeNetBufferList));

            NdisMIndicateReceiveNetBufferLists(
                    MP_GET_ADAPTER_HANDLE(Adapter),
                    FreeNetBufferList,
                    NDIS_DEFAULT_PORT_NUMBER,
                    NetBufferListFreeCount,
                    ReceiveFlags);
        }
        NdisDprAcquireSpinLock(&Adapter->RcvLock);
    }
    //
    // NDIS won't take ownership for the packets with NDIS_STATUS_RESOURCES.
    // For other packets, NDIS always takes the ownership and gives them back
    // by calling MPReturnPackets
    //
    for (; FreeNetBufferListFull != NULL; FreeNetBufferListFull = NET_BUFFER_LIST_NEXT_NBL(FreeNetBufferListFull))
    {
        //
        // Get the MP_RFD saved in this packet, in NICAllocRfd
        //
        pMpRfd = MP_GET_NET_BUFFER_LIST_RFD(FreeNetBufferListFull);

        ASSERT(MP_TEST_FLAG(pMpRfd, fMP_RFD_RESOURCES));
        MP_CLEAR_FLAG(pMpRfd, fMP_RFD_RESOURCES);
        NICReturnRFD(Adapter, pMpRfd);

        INSREG32BF(&gpFECReg->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);

    }
    //
    // If we have set power pending, then complete it
    //
    if (((Adapter->PendingRequest)
            && ((Adapter->PendingRequest->RequestType == NdisRequestSetInformation)
            && (Adapter->PendingRequest->DATA.SET_INFORMATION.Oid == OID_PNP_SET_POWER)))
            && (Adapter->PoMgmt.OutstandingRecv == 0))
    {
        MpSetPowerLowComplete(Adapter);
    }
    //
    // get rid of the extra receive ref count we added at the beginning of this
    // function and check to see if we need to complete the pause.
    // Note that we don't have to worry about a blocked Halt here because
    // we are handling an interrupt DPC which means interrupt deregistertion is not
    // completed yet so even if our halt handler is running, NDIS will block the
    // interrupt deregisteration till we return back from this DPC.
    //
    MP_DEC_RCV_REF(Adapter);
    Count =  MP_GET_RCV_REF(Adapter);
    if ((Count == 0) && (Adapter->AdapterState == NicPausing))
    {
        //
        // If all the NetBufferLists are returned and miniport is pausing,
        // complete the pause
        //
        DBGPRINT(MP_TRACE, ("MpHandleRecvInterrupt- NIC was in pausing state. All Recv packet returned. Changing to Adapter Paused State \n"));
        Adapter->AdapterState = NicPaused;
        NdisDprReleaseSpinLock(&Adapter->RcvLock);
        NdisMPauseComplete(Adapter->AdapterHandle);
        NdisDprAcquireSpinLock(&Adapter->RcvLock);
    }
    ASSERT(Adapter->nReadyRecv >= NIC_MIN_RFDS);
}

VOID
NICReturnRFD(
    IN  PMP_ADAPTER  Adapter,
    IN  PMP_RFD      pMpRfd
    )
/*++
Routine Description:

    Recycle a RFD and put it back onto the receive list
    Assumption: Rcv spinlock has been acquired

Arguments:

    Adapter     Pointer to our adapter
    pMpRfd      Pointer to the RFD

Return Value:

    None

    NOTE: During return, we should check if we need to allocate new net buffer list
          for the RFD.
--*/
{
    PHW_RFD   pHwRfd = pMpRfd->HwRfd;

    MP_SET_FLAG(pMpRfd, fMP_RFD_RECV_READY);


    // The processing on this RFD is done, so put it back on the tail of
    // our list

    InsertTailList(&Adapter->RecvList, (PLIST_ENTRY)pMpRfd);
    Adapter->nReadyRecv++;
    ASSERT(Adapter->nReadyRecv <= Adapter->CurrNumRfd);

    //
    // HW_SPECIFIC_START
    //

    //Allow it to recv next packet
    pHwRfd->ControlStatus &= ~BD_ENET_RX_STATS;
    pHwRfd->DataLen = 0;
    //When this flag is set, HW can update the buffer and len
    pHwRfd->ControlStatus |= (BD_ENET_RX_EMPTY|BD_ENET_RX_PAD); 

    //
    // HW_SPECIFIC_END

}

VOID
MpFreeQueuedSendNetBufferLists(
    IN  PMP_ADAPTER  Adapter
    )
/*++
Routine Description:

    Free and complete the pended sends on SendWaitQueue
    Assumption: spinlock has been acquired

Arguments:

    Adapter     Pointer to our adapter

Return Value:

     None
NOTE: Run at DPC

--*/
{
    PQUEUE_ENTRY        pEntry;
    PNET_BUFFER_LIST    NetBufferList;
    PNET_BUFFER_LIST    NetBufferListToComplete = NULL;
    PNET_BUFFER_LIST    LastNetBufferList = NULL;
    NDIS_STATUS         Status = MP_GET_STATUS_FROM_FLAGS(Adapter);
    PNET_BUFFER         NetBuffer;

    DBGPRINT(MP_TRACE, ("--> MpFreeQueuedSendNetBufferLists\n"));

    while (!IsQueueEmpty(&Adapter->SendWaitQueue))
    {
        pEntry = RemoveHeadQueue(&Adapter->SendWaitQueue);
        ASSERT(pEntry);

        Adapter->nWaitSend--;

        NetBufferList = MP_GET_NET_BUFFER_LIST_FROM_QUEUE_LINK(pEntry);

        NET_BUFFER_LIST_STATUS(NetBufferList) = Status;
        //
        // The sendLock is held
        //
        NetBuffer = MP_GET_NET_BUFFER_LIST_NEXT_SEND(NetBufferList);

        for (; NetBuffer != NULL; NetBuffer = NET_BUFFER_NEXT_NB(NetBuffer))
        {
            MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList)--;
        }
        //
        // If Ref count goes to 0, then complete it.
        // Otherwise, Send interrupt DPC would complete it later
        //
        if (MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList) == 0)
        {
            if (NetBufferListToComplete == NULL)
            {
                NetBufferListToComplete = NetBufferList;
            }
            else
            {
                NET_BUFFER_LIST_NEXT_NBL(LastNetBufferList) = NetBufferList;
            }
            NET_BUFFER_LIST_NEXT_NBL(NetBufferList) = NULL;
            LastNetBufferList = NetBufferList;

        }
    }
    if (NetBufferListToComplete != NULL)
    {
        DBGPRINT(MP_TRACE, ("MpFreeQueuedSendNetBufferLists- SendingBufferComplete Buffer "PTR_FORMAT" \n", NetBufferListToComplete));
        MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, TRUE);
        NdisMSendNetBufferListsComplete(
               MP_GET_ADAPTER_HANDLE(Adapter),
               NetBufferListToComplete,
               NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL);

        MP_ACQUIRE_SPIN_LOCK(&Adapter->SendLock, TRUE);
    }
    DBGPRINT(MP_TRACE, ("<-- MpFreeQueuedSendNetBufferLists\n"));
}

void
FECFreeBusySendNetBufferLists(
    IN  PMP_ADAPTER  Adapter
    )
/*++
Routine Description:

    Free and complete the stopped active sends
    Assumption: Send spinlock has been acquired

Arguments:

    Adapter     Pointer to our adapter

Return Value:

     None

--*/
{
    PMP_TCB  pMpTcb;
    PNET_BUFFER_LIST   NetBufferList;
    PNET_BUFFER_LIST   CompleteNetBufferLists = NULL;
    PNET_BUFFER_LIST   LastNetBufferList = NULL;

    DBGPRINT(MP_TRACE, ("--> FECFreeBusySendNetBufferLists; Busy Count %d\n",Adapter->nBusySend));

    //
    // Any NET_BUFFER being sent? Check the first TCB on the send list
    //
    while (Adapter->nBusySend > 0)
    {
        pMpTcb = Adapter->CurrSendHead;

        //
        // To change this to complete a list of NET_BUFFER_LISTs
        //
        NetBufferList = FEC_FREE_SEND_NET_BUFFER(Adapter, pMpTcb); //this will advance the SendHead
        Adapter->nBusySend--;

        //
        // If one NET_BUFFER_LIST got complete
        //
        if (NetBufferList != NULL)
        {
            NET_BUFFER_LIST_STATUS(NetBufferList) = NDIS_STATUS_REQUEST_ABORTED;
            if (CompleteNetBufferLists == NULL)
            {
                CompleteNetBufferLists = NetBufferList;
            }
            else
            {
                NET_BUFFER_LIST_NEXT_NBL(LastNetBufferList) = NetBufferList;
            }
            NET_BUFFER_LIST_NEXT_NBL(NetBufferList) = NULL;
            LastNetBufferList = NetBufferList;
        }

        //
        // Complete the NET_BUFFER_LISTs
        //
        if (CompleteNetBufferLists != NULL)
        {
            DBGPRINT(MP_TRACE, ("FECFreeBusySendNetBufferLists- SendingBufferComplete Buffer "PTR_FORMAT" \n", CompleteNetBufferLists));
            NdisReleaseSpinLock(&Adapter->SendLock);
            NdisMSendNetBufferListsComplete(
               MP_GET_ADAPTER_HANDLE(Adapter),
               CompleteNetBufferLists,
               0);
            NdisAcquireSpinLock(&Adapter->SendLock);
        }
    }

    
    DBGPRINT(MP_TRACE, ("<-- FECFreeBusySendNetBufferLists\n"));
}

VOID
NICResetRecv(
    IN  PMP_ADAPTER   Adapter
    )
/*++
Routine Description:

    Reset the receive list
    Assumption: Rcv spinlock has been acquired

Arguments:

    Adapter     Pointer to our adapter

Return Value:

     None

--*/
{
    PMP_RFD   pMpRfd;
    PHW_RFD   pHwRfd;
    PLIST_ENTRY Head;
    int i=0;

    DBGPRINT(MP_TRACE, ("--> NICResetRecv\n"));

    ASSERT(!IsListEmpty(&Adapter->RecvList));

    //Loop recvlist and clear the status of BDs

    if(TRUE == IsListEmpty(&Adapter->RecvList))
        return;

    RETAILMSG(TRUE,(TEXT("NIC Reset Recv %d"),Adapter->nReadyRecv));

    Head = GetListFLink(&Adapter->RecvList);

    while (&Adapter->RecvList != Head)
    {
        ASSERT(i<Adapter->NumRfd);

        pMpRfd = (PMP_RFD) Head;
        pHwRfd = (PRFD_STRUC) pMpRfd->HwRfd;

        pHwRfd->ControlStatus &= ~BD_ENET_RX_STATS;
        pHwRfd->ControlStatus |= BD_ENET_RX_EMPTY|BD_ENET_RX_PAD;

        Head = GetListFLink(Head);
        ++i;
    }

    RETAILMSG(TRUE,(TEXT("NIC Reset Recv Iterated %d times through recvlist"),i));

    Adapter->HwRecvRbdCurrentCount = 0;
    DBGPRINT(MP_TRACE, ("<-- NICResetRecv\n"));
}


VOID
MpLinkDetectionDpc(
    IN  PVOID       SystemSpecific1,
    IN  PVOID       FunctionContext,
    IN  PVOID       SystemSpecific2,
    IN  PVOID       SystemSpecific3
    )
/*++

Routine Description:

    Timer function for postponed link negotiation

Arguments:

    SystemSpecific1     Not used
    FunctionContext     Pointer to our adapter
    SystemSpecific2     Not used
    SystemSpecific3     Not used

Return Value:

    None

--*/
{
    PMP_ADAPTER         Adapter = (PMP_ADAPTER)FunctionContext;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
#ifndef UNDER_CE
    NDIS_STATUS         IndicateStatus;
#endif  //  UNDER_CE
    PQUEUE_ENTRY        pEntry = NULL;
    PNDIS_OID_REQUEST   PendingRequest;
    NDIS_REQUEST_TYPE   RequestType;
    NDIS_OID            Oid;
    ULONG               PacketFilter;
    ULONG               LinkSpeed;
    ULONG               MediaState=0;
    BOOLEAN             isTimerAlreadyInQueue = FALSE;

    UNREFERENCED_PARAMETER(SystemSpecific1);
    UNREFERENCED_PARAMETER(SystemSpecific2);
    UNREFERENCED_PARAMETER(SystemSpecific3);

    //
    // NDIS 6.0 miniports are required to report their link speed, link state and
    // duplex state as soon as they figure it out. NDIS does not make any assumption
    // that they are connected, etc.
    //
    FECUpdateLinkStatus(Adapter);
    NdisDprAcquireSpinLock(&Adapter->Lock);

    //
    // Reset some variables for link detection
    //
    DBGPRINT(MP_WARN, ("MpLinkDetectionDpc - negotiation done\n"));

    MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION);
    //
    // Any pending request?
    //
    if (Adapter->PendingRequest)
    {
        PendingRequest = Adapter->PendingRequest;
        Adapter->PendingRequest = NULL;
        RequestType = PendingRequest->RequestType;
        NdisDprReleaseSpinLock(&Adapter->Lock);
        switch(RequestType)
        {
            case NdisRequestQueryInformation:
            case NdisRequestQueryStatistics:
                Oid = PendingRequest->DATA.QUERY_INFORMATION.Oid;
                Status = NDIS_STATUS_SUCCESS;
                switch (Oid)
                {
                    case OID_GEN_LINK_SPEED:
                        LinkSpeed = Adapter->usLinkSpeed * SPEED_FACTOR;
                        NdisMoveMemory(PendingRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                                        &LinkSpeed,
                                        sizeof(ULONG));

                        PendingRequest->DATA.QUERY_INFORMATION.BytesWritten = sizeof(ULONG);
                        break;

                    case OID_GEN_MEDIA_CONNECT_STATUS:
                    default:
                        ASSERT(PendingRequest->DATA.QUERY_INFORMATION.Oid == OID_GEN_MEDIA_CONNECT_STATUS);
                        NdisMoveMemory(PendingRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                                        &MediaState,
                                        sizeof(ULONG));
                        NdisDprAcquireSpinLock(&Adapter->Lock);
                        if (Adapter->MediaState != MediaState)
                        {
                            Adapter->MediaState = MediaState;
                            DBGPRINT(MP_WARN, ("OID- Media state changed to %s\n",
                                      ((MediaState == NdisMediaStateConnected)?
                                      "Connected": "Disconnected")));
                        }
                        NdisDprReleaseSpinLock(&Adapter->Lock);

                    PendingRequest->DATA.QUERY_INFORMATION.BytesWritten = sizeof(NDIS_MEDIA_STATE);
                    break;
                }
                PendingRequest->DATA.QUERY_INFORMATION.BytesNeeded = 0;
                break;

            case NdisRequestSetInformation:
                //
                // It has to be set packet filter
                //
                Oid = PendingRequest->DATA.QUERY_INFORMATION.Oid;
                if (Oid == OID_GEN_CURRENT_PACKET_FILTER)
                {
                    NdisMoveMemory(&PacketFilter,
                                     PendingRequest->DATA.SET_INFORMATION.InformationBuffer,
                                     sizeof(ULONG));

                    NdisDprAcquireSpinLock(&Adapter->Lock);

                    DBGPRINT(MP_INFO, ("NICSetPacketFilter set Pending Filter\n",PacketFilter));

                    Status = NICSetPacketFilter(
                                 Adapter,
                                 PacketFilter);

                    NdisDprReleaseSpinLock(&Adapter->Lock);

                    if (Status == NDIS_STATUS_SUCCESS)
                    {
                        PendingRequest->DATA.SET_INFORMATION.BytesRead = sizeof(ULONG);
                        Adapter->PacketFilter = PacketFilter;

                    }
                    PendingRequest->DATA.QUERY_INFORMATION.BytesNeeded = 0;
                }
                break;

            case NdisRequestMethod:
                Status = MpMethodRequest (Adapter, PendingRequest);
                break;

            default:
                ASSERT(FALSE);
                break;
        } //end of outside switch
        if (Status != NDIS_STATUS_PENDING)
        {
            NdisMOidRequestComplete(Adapter->AdapterHandle,
                                    PendingRequest,
                                    Status);
        }
        NdisDprAcquireSpinLock(&Adapter->Lock);
    }
    //
    // Adapter->Lock is held
    //
    // Any pending reset?
    //
    if (Adapter->bResetPending)
    {
        //
        // The link detection may have held some requests and caused reset.
        // Don't need to complete the reset, since the status is not SUCCESS.
        //
        Adapter->bResetPending = FALSE;
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS);

        NdisDprReleaseSpinLock(&Adapter->Lock);

        NdisMResetComplete(
            Adapter->AdapterHandle,
            NDIS_STATUS_ADAPTER_NOT_READY,
            FALSE);
    }
    else
    {
        NdisDprReleaseSpinLock(&Adapter->Lock);
    }

    NdisDprAcquireSpinLock(&Adapter->SendLock);

    //
    // Send NET_BUFFER_LISTs which have been queued while link detection was going on.
    //
    if (MP_IS_READY(Adapter))
    {
        while (!IsQueueEmpty(&Adapter->SendWaitQueue) &&
            (MP_TCB_RESOURCES_AVAIABLE(Adapter) &&
            Adapter->SendingNetBufferList == NULL))
        {
            PNET_BUFFER_LIST    NetBufferList;

            pEntry = GetHeadQueue(&Adapter->SendWaitQueue);
            ASSERT(pEntry);
            NetBufferList = MP_GET_NET_BUFFER_LIST_FROM_QUEUE_LINK(pEntry);
            DBGPRINT(MP_INFO, ("MpLinkDetectionDpc - send a queued NetBufferList "PTR_FORMAT" \n",NetBufferList));
            Adapter->SendingNetBufferList = NetBufferList;
            Status = MiniportSendNetBufferList(Adapter, NetBufferList, TRUE);
            if (Status != NDIS_STATUS_SUCCESS && Status != NDIS_STATUS_PENDING)
            {
                break;
            }
        }
    }
    MP_DEC_REF(Adapter);
    if (MP_GET_REF(Adapter) == 0)
    {
        NdisSetEvent(&Adapter->ExitEvent);
    }
    NdisDprReleaseSpinLock(&Adapter->SendLock);
}

VOID
MpProcessSGList(
    IN  PDEVICE_OBJECT          pDO,
    IN  PVOID                   pIrp,
    IN  PSCATTER_GATHER_LIST    pSGList,
    IN  PVOID                   Context
    )
/*++
Routine Description:

    Process  SG list for an NDIS packet or a NetBuffer by submitting the physical addresses
    in SG list to NIC's DMA engine and issuing command n hardware.

Arguments:

    pDO:  Ignore this parameter

    pIrp: Ignore this parameter

    pSGList: A pointer to Scatter Gather list built for the NDIS packet or NetBuffer passed
             to NdisMAllocNetBufferList. This is not necessarily
             the same ScatterGatherListBuffer passed to NdisMAllocNetBufferSGList

    Context: The context passed to NdisMAllocNetBufferList. Here is
             a pointer to MP_TCB

Return Value:

     None

    NOTE: called at DISPATCH level
--*/
{
    PMP_TCB             pMpTcb = (PMP_TCB)Context;
    PMP_ADAPTER         Adapter = pMpTcb->Adapter;
    PMP_FRAG_LIST       pFragList = (PMP_FRAG_LIST)pSGList;
    NDIS_STATUS         Status;
    MP_FRAG_LIST        FragList;
    BOOLEAN             fSendComplete = FALSE;
    ULONG               BytesCopied;
    PNET_BUFFER_LIST    NetBufferList;

    DBGPRINT(MP_TRACE, ("--> MpProcessSGList; Tcb "PTR_FORMAT" \n",pMpTcb));

    MP_ACQUIRE_SPIN_LOCK(&Adapter->SendLock, TRUE);

    // Save SG list that we got from NDIS. This is not necessarily the
    // same SG list buffer we passed to NdisMAllocNetBufferSGList

    pMpTcb->pSGList = pSGList;
    if (!MP_TEST_FLAG(pMpTcb, fMP_TCB_IN_USE))
    {
        DBGPRINT(MP_TRACE, ("MpProcessSGList, TCB not in use! \n"));

        // Before this callback function is called, reset happened and aborted
        // all the sends.
        // Call ndis to free resouces allocated for a SG list
        MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, TRUE);
    }
    else
    {

        if( 0 == pSGList->NumberOfElements )
        {
            DBGPRINT(MP_WARN, ("<-- MpProcessSGList; No element in the SG List!\n"));

            NetBufferList = pMpTcb->NetBufferList;

            NET_BUFFER_LIST_STATUS(NetBufferList) = NDIS_STATUS_INVALID_PACKET; 

            MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList)--;
            if (MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList) == 0)
            {
                fSendComplete = TRUE;
            }
            MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, TRUE);
            NdisMFreeNetBufferSGList(
                Adapter->NdisMiniportDmaHandle,
                pMpTcb->pSGList,
                pMpTcb->NetBuffer);
            
            if (fSendComplete)
            {
                NET_BUFFER_LIST_NEXT_NBL(NetBufferList) = NULL;

                RETAILMSG(TRUE,(TEXT("SG NoData- Busy Send %d \n"), Adapter->nBusySend));
                DBGPRINT(MP_TRACE, ("MpProcessSGList- SendingBufferComplete Buffer "PTR_FORMAT" Busy Send Count %d \n", NetBufferList,Adapter->nBusySend));
                NdisMSendNetBufferListsComplete(
                    MP_GET_ADAPTER_HANDLE(Adapter),
                    NetBufferList,
                    NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL);
            }

            return;
        }


        if ( pSGList->NumberOfElements > NIC_MAX_PHYS_BUF_COUNT )
        {
            //
            // the driver needs to do the local copy
            //
            BytesCopied = MpCopyNetBuffer(pMpTcb->NetBuffer, pMpTcb->MpTxBuf);

            //
            // MpCopyNetBuffer may return 0 if system resources are low or exhausted
            //
            if (BytesCopied == 0)
            {
                DBGPRINT(MP_ERROR, ("Copy NetBuffer NDIS_STATUS_RESOURCES, NetBuffer= "PTR_FORMAT"\n", pMpTcb->NetBuffer));
                NetBufferList = pMpTcb->NetBufferList;
                NET_BUFFER_LIST_STATUS(NetBufferList) = NDIS_STATUS_RESOURCES;

                MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList)--;
                if (MP_GET_NET_BUFFER_LIST_REF_COUNT(NetBufferList) == 0)
                {
                    fSendComplete = TRUE;
                }
                MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, TRUE);
                NdisMFreeNetBufferSGList(
                    Adapter->NdisMiniportDmaHandle,
                    pMpTcb->pSGList,
                    pMpTcb->NetBuffer);
                if (fSendComplete)
                {
                    NET_BUFFER_LIST_NEXT_NBL(NetBufferList) = NULL;

                    RETAILMSG(TRUE,(TEXT("SG NoData- Busy Send %d \n"), Adapter->nBusySend));
                    DBGPRINT(MP_TRACE, ("MpProcessSGList- SendingBufferComplete Buffer "PTR_FORMAT" Busy Send Count %d \n", NetBufferList,Adapter->nBusySend));
                    NdisMSendNetBufferListsComplete(
                        MP_GET_ADAPTER_HANDLE(Adapter),
                        NetBufferList,
                        NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL);
                }
             }
             else
             {
                MP_SET_FLAG(pMpTcb, fMP_TCB_USE_LOCAL_BUF);

                //
                // Set up the frag list, only one fragment after it's coalesced
                //
                pFragList = &FragList;
                pFragList->NumberOfElements = 1;
                pFragList->Elements[0].Address = pMpTcb->MpTxBuf->BufferPa;
                pFragList->Elements[0].Length = (BytesCopied >= NIC_MIN_PACKET_SIZE) ?
                                                BytesCopied : NIC_MIN_PACKET_SIZE;
                Status = NICSendNetBuffer(Adapter, pMpTcb, pFragList);
                MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, TRUE);
             }
        }
        else
        {
            Status = NICSendNetBuffer(Adapter, pMpTcb, pFragList);
            MP_RELEASE_SPIN_LOCK(&Adapter->SendLock, TRUE);
        }
    }
    DBGPRINT(MP_TRACE, ("<-- MpProcessSGList\n"));
}

VOID
FECIndicateLinkState(
    IN  PMP_ADAPTER     Adapter
    )
/*++
Routine Description:
    This routine sends a NDIS_STATUS_LINK_STATE status up to NDIS

Arguments:

    Adapter         Pointer to our adapter

Return Value:

     None

    NOTE: called at DISPATCH level
--*/

{
    NDIS_LINK_STATE                LinkState;
    NDIS_STATUS_INDICATION         StatusIndication;

    NdisZeroMemory(&LinkState, sizeof(NDIS_LINK_STATE));
    LinkState.Header.Revision = NDIS_LINK_STATE_REVISION_1;
    LinkState.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    LinkState.Header.Size = sizeof(NDIS_LINK_STATE);

    DBGPRINT(MP_WARN, ("Indicating Media state %s to NDIS\n",
              ((Adapter->MediaState == MediaConnectStateConnected)?
              "Connected": "Disconnected")));

    if (Adapter->MediaState == MediaConnectStateConnected)
    {
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_NO_CABLE);
        if (Adapter->usDuplexMode == 1)
        {
            Adapter->MediaDuplexState = MediaDuplexStateHalf;
        }
        else if (Adapter->usDuplexMode == 2)
        {
            Adapter->MediaDuplexState = MediaDuplexStateFull;
        }
        else
        {
            Adapter->MediaDuplexState = MediaDuplexStateUnknown;
        }
        //
        // NDIS 6.0 miniports report xmit and recv link speeds in bps
        //
        Adapter->LinkSpeed = Adapter->usLinkSpeed * SPEED_FACTOR;
    }
    else
    {
        MP_SET_FLAG(Adapter, fMP_ADAPTER_NO_CABLE);
        Adapter->MediaState = MediaConnectStateDisconnected;
        Adapter->MediaDuplexState = MediaDuplexStateUnknown;
        Adapter->LinkSpeed = NDIS_LINK_SPEED_UNKNOWN;
    }
    LinkState.MediaConnectState = Adapter->MediaState;
    LinkState.MediaDuplexState = Adapter->MediaDuplexState;
    LinkState.XmitLinkSpeed = LinkState.RcvLinkSpeed = Adapter->LinkSpeed;
    NdisDprReleaseSpinLock(&Adapter->Lock);
    MP_INIT_NDIS_STATUS_INDICATION(&StatusIndication,
                                   Adapter->AdapterHandle,
                                   NDIS_STATUS_LINK_STATE,
                                   (PVOID)&LinkState,
                                   sizeof(LinkState));

    DBGPRINT(MP_TRACE, ("Media State %d \n",Adapter->MediaState));
    NdisMIndicateStatusEx(Adapter->AdapterHandle, &StatusIndication);
    NdisDprAcquireSpinLock(&Adapter->Lock);

    return;
}


//------------------------------------------------------------------------------
//
// Function: MpHandleMIIInterrupt
//
// This function is the interrupt handler for the MII interrupt
//
// Parameters:
//      Adapter
//          [in]  Specifies the pointer to the driver allocated context area in
//                which the driver maintains FEC adapter state, set up by
//                FECInitialize
//
// Return Value:
//      None
//
//------------------------------------------------------------------------------
NDIS_STATUS
MpHandleMIIInterrupt(
    IN  PMP_ADAPTER  Adapter
    )
{
    PFEC_MII_LIST MIIPoint;
    UINT MIIReg;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +ProcessMIIInterrupts\r\n")));

    // Get the MII frame value from MMFR register
    MIIReg = INREG32( &gpFECReg->MMFR );
    if( (MIIPoint = gMIIHead) == NULL )
    {
        DEBUGMSG(ZONE_INFO,
        (TEXT("%s: MII head is null\r\n"), __WFUNCTION__));
        Status = NDIS_STATUS_SUCCESS;
    }
    if( MIIPoint->MIIFunction != NULL )
        (*(MIIPoint->MIIFunction))(MIIReg, (NDIS_HANDLE)Adapter);

    gMIIHead = MIIPoint->MIINext;
    MIIPoint->MIINext = gMIIFree;
    gMIIFree = MIIPoint;

    // Send the next MMI management frame to the external
    // PHY if any
    if( (MIIPoint = gMIIHead) != NULL )
        OUTREG32( &gpFECReg->MMFR, MIIPoint->MIIRegVal );

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -ProcessMIIInterrupts\r\n")));
    return Status;
}

//------------------------------------------------------------------------------
//
// Function: GetListEntry
//
// This function returns RFD from the recvList
//
// Parameters:
//      [in]  'index' of the RFD requested from the 'recvlist'
//
// Return Value:
//      RFD
//
//------------------------------------------------------------------------------
PLIST_ENTRY GetListEntry(UINT IndexCount, PLIST_ENTRY  RecvList)
{
    int i=0;
    PLIST_ENTRY Head = GetListFLink(RecvList);
    PMP_RFD pRfd = NULL;

    if(TRUE == IsListEmpty(RecvList))
        return (PLIST_ENTRY) pRfd;

    pRfd = (PMP_RFD)Head;
    while ( (NULL != pRfd) && ((UINT)pRfd != (UINT)RecvList) )
    {
        ASSERT(i<FEC_RX_RING_SIZE);
        if(pRfd->HwRfdIndex == IndexCount)
        {
            break;
        }else
        {
            Head = GetListFLink(Head);
            pRfd = (PMP_RFD)Head;
        }
        ++i;
    }
    //just compare the pointers..
    if ((UINT)pRfd == (UINT)RecvList)
        pRfd = NULL;

    return (PLIST_ENTRY) pRfd;
}


//------------------------------------------------------------------------------
//
// Function: GetListEntry
//
// This function returns Tcb for the Tbd Index
//
// Parameters:
//      [in]  'index' of the packet currently being send completed
//
// Return Value:
//      RFD
//
//------------------------------------------------------------------------------
PMP_TCB GetTcb(PMP_ADAPTER  Adapter, INT TbdIndex)
{
    //iterate through the tcb chain and return the tcb which matches
    //the current index
    int i=0;
    PMP_TCB pMpTcb = NULL;
    PMP_TCB pMpTcbStart = (PMP_TCB) Adapter->MpTcbMem;
    PMP_TCB pMpCurTcb = pMpTcbStart->Next;
    ASSERT(TbdIndex<Adapter->NumTcb);

    if(pMpTcbStart->HwTbdIndex == TbdIndex )
        return pMpTcbStart;

    while (pMpCurTcb != pMpTcbStart)
    {
        ASSERT(i<Adapter->NumTcb);
        if( pMpCurTcb->HwTbdIndex == TbdIndex )
        {
            pMpTcb = pMpCurTcb;
            break;
        }
        pMpCurTcb = pMpCurTcb->Next;
        ++i;
    }
    return pMpTcb;
}