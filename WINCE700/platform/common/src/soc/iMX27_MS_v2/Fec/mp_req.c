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
    mp_req.c

Abstract:
    This module contains miniport OID related handlers

Revision History:

Notes:

--*/

#include "precomp.h"

#if DBG
#define _FILENUMBER     'QERM'
#endif

ULONG VendorDriverVersion = NIC_VENDOR_DRIVER_VERSION;
UCHAR CalculateHashValue(UCHAR *pAddr);

/**
Local Prototypes
**/

//
// Macros used to walk a doubly linked list. Only macros that are not defined in ndis.h
// The List Next macro will work on Single and Doubly linked list as Flink is a common
// field name in both
//
#define ListNext(_pL)                       (_pL)->Flink
#define ListPrev(_pL)                       (_pL)->Blink

__inline
BOOLEAN
MPIsPoMgmtSupported(
   IN PMP_ADAPTER pAdapter
   )
{
    return TRUE;//FALSE;
}

NDIS_STATUS
FECOidRequest(
    IN  NDIS_HANDLE        MiniportAdapterContext,
    IN  PNDIS_OID_REQUEST  NdisRequest
    )

/*++
Routine Description:

    MiniportRequest dispatch handler

Arguments:

    MiniportAdapterContext  Pointer to the adapter structure
    NdisRequest             Pointer to NDIS_OID_REQUEST

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_NOT_SUPPORTED
    NDIS_STATUS_BUFFER_TOO_SHORT

--*/
{
    PMP_ADAPTER             Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    NDIS_REQUEST_TYPE       RequestType;

    DBGPRINT(MP_TRACE, ("====> FECOidRequest\n"));

    //
    // Should abort the request if reset is in process
    //
    NdisAcquireSpinLock(&Adapter->Lock);

    //
    // If there is a request pending then assert.
    //

    ASSERT(Adapter->PendingRequest == NULL);

    if (MP_TEST_FLAG(Adapter,
        (fMP_ADAPTER_RESET_IN_PROGRESS | fMP_ADAPTER_REMOVE_IN_PROGRESS)))
    {
        NdisReleaseSpinLock(&Adapter->Lock);
        return NDIS_STATUS_REQUEST_ABORTED;
    }
    NdisReleaseSpinLock(&Adapter->Lock);
    RequestType = NdisRequest->RequestType;
    switch (RequestType)
    {
        case NdisRequestMethod:
            Status = MpMethodRequest(Adapter, NdisRequest);
            break;

        case NdisRequestSetInformation:
            Status = FECSetInformation(Adapter, NdisRequest);
            break;

        case NdisRequestQueryInformation:
        case NdisRequestQueryStatistics:
            Status = FECQueryInformation(Adapter, NdisRequest);
            break;

        default:
            //
            // Later the entry point may by used by all the requests
            //
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
    }
    DBGPRINT(MP_TRACE, ("<==== FECOidRequest\n"));
    return Status;
}

NDIS_STATUS NICGetStatsCounters(
    IN  PMP_ADAPTER  Adapter,
    IN  NDIS_OID     Oid,
    OUT PULONG64     pCounter
    )
/*++
Routine Description:

    Get the value for a statistics OID

Arguments:

    Adapter     Pointer to our adapter
    Oid         Self-explanatory
    pCounter    Pointer to receive the value

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_NOT_SUPPORTED

--*/
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PNDIS_STATISTICS_INFO StatisticsInfo;

    DBGPRINT(MP_TRACE, ("--> NICGetStatsCounters\n"));

    *pCounter = 0;

    switch(Oid)
    {
        case OID_GEN_XMIT_OK:
            *pCounter = Adapter->TxdStatus.FramesXmitGood;
            break;

        case OID_GEN_RCV_OK:
            *pCounter = Adapter->RcvStatus.FrameRcvGood;
            break;

        case OID_GEN_XMIT_ERROR:
            *pCounter = Adapter->TxdStatus.FramesXmitBad;
            break;

        case OID_GEN_RCV_ERROR:
            *pCounter = Adapter->RcvStatus.FrameRcvErrors;
            break;

        case OID_GEN_RCV_NO_BUFFER:
            *pCounter = Adapter->RcvResourceErrors;
            break;

        case OID_GEN_RCV_CRC_ERROR:
            *pCounter = Adapter->RcvCrcErrors;
            break;

        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
            *pCounter = Adapter->nWaitSend;
            break;

        case OID_802_3_RCV_ERROR_ALIGNMENT:
            *pCounter = Adapter->RcvStatus.FrameRcvAllignmentErrors;
            break;

        case OID_802_3_XMIT_ONE_COLLISION:
            *pCounter = Adapter->TxdStatus.FramesXmitCollisionErrors;
            break;

        case OID_802_3_XMIT_MORE_COLLISIONS:
            break;

        case OID_802_3_XMIT_DEFERRED:
            *pCounter = Adapter->TxOKButDeferred;
            break;

        case OID_802_3_XMIT_MAX_COLLISIONS:
            *pCounter = Adapter->TxdStatus.FramesXmitCollisionErrors;   //was Adapter->TxAbortExcessCollisions;
            break;

        case OID_802_3_RCV_OVERRUN:
            *pCounter = Adapter->RcvStatus.FrameRcvOverrunErrors;  //was Adapter->RcvDmaOverrunErrors;
            break;

        case OID_802_3_XMIT_UNDERRUN:
            *pCounter = Adapter->TxdStatus.FramesXmitUnderrunErrors;
            break;

        case OID_802_3_XMIT_HEARTBEAT_FAILURE:
            *pCounter = Adapter->TxLostCRS;
            break;

        case OID_802_3_XMIT_TIMES_CRS_LOST:
            *pCounter = Adapter->TxLostCRS;
            break;

        case OID_802_3_XMIT_LATE_COLLISIONS:
            *pCounter = Adapter->TxLateCollisions;
            break;

        case OID_GEN_RCV_DISCARDS:
            *pCounter = Adapter->RcvCrcErrors +
                        Adapter->RcvAlignmentErrors +
                        Adapter->RcvResourceErrors +
                        Adapter->RcvDmaOverrunErrors +
                        Adapter->RcvRuntErrors;
            break;

        case OID_GEN_DIRECTED_BYTES_XMIT:
            *pCounter = Adapter->OutUcastOctets;
                break;
        case OID_GEN_DIRECTED_FRAMES_XMIT:
            *pCounter = Adapter->OutUcastPkts;
                break;
        case OID_GEN_MULTICAST_BYTES_XMIT:
            *pCounter = Adapter->OutMulticastOctets;
                break;
        case OID_GEN_MULTICAST_FRAMES_XMIT:
            *pCounter = Adapter->OutMulticastPkts;
                break;
        case OID_GEN_BROADCAST_BYTES_XMIT:
            *pCounter = Adapter->OutBroadcastOctets;
                break;
        case OID_GEN_BROADCAST_FRAMES_XMIT:
            *pCounter = Adapter->OutBroadcastPkts;
                break;
        case OID_GEN_DIRECTED_BYTES_RCV:
            *pCounter = Adapter->InUcastOctets;
                break;
        case OID_GEN_DIRECTED_FRAMES_RCV:
            *pCounter = Adapter->InUcastPkts;
            break;
        case OID_GEN_MULTICAST_BYTES_RCV:
            *pCounter = Adapter->InMulticastOctets;
                break;
        case OID_GEN_MULTICAST_FRAMES_RCV:
            *pCounter = Adapter->InMulticastOctets;
            break;
        case OID_GEN_BROADCAST_BYTES_RCV:
            *pCounter = Adapter->InBroadcastOctets;
            break;
        case OID_GEN_BROADCAST_FRAMES_RCV:
            *pCounter = Adapter->InBroadcastPkts;
            break;
        case OID_GEN_STATISTICS:

            StatisticsInfo = (PNDIS_STATISTICS_INFO)pCounter;
            StatisticsInfo->Header.Revision = NDIS_OBJECT_REVISION_1;
            StatisticsInfo->Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
            StatisticsInfo->Header.Size = sizeof(NDIS_STATISTICS_INFO);
            StatisticsInfo->SupportedStatistics = NDIS_STATISTICS_FLAGS_VALID_RCV_DISCARDS          |
                                                  NDIS_STATISTICS_FLAGS_VALID_RCV_ERROR             |
                                                  NDIS_STATISTICS_FLAGS_VALID_XMIT_ERROR            |
                                                  NDIS_STATISTICS_FLAGS_VALID_DIRECTED_FRAMES_RCV   |
                                                  NDIS_STATISTICS_FLAGS_VALID_MULTICAST_FRAMES_RCV  |
                                                  NDIS_STATISTICS_FLAGS_VALID_BROADCAST_FRAMES_RCV  |
                                                  NDIS_STATISTICS_FLAGS_VALID_BYTES_RCV             |
                                                  NDIS_STATISTICS_FLAGS_VALID_DIRECTED_FRAMES_XMIT  |
                                                  NDIS_STATISTICS_FLAGS_VALID_MULTICAST_FRAMES_XMIT |
                                                  NDIS_STATISTICS_FLAGS_VALID_BROADCAST_FRAMES_XMIT;

            StatisticsInfo->ifInDiscards = Adapter->RcvCrcErrors +
                                           Adapter->RcvAlignmentErrors +
                                           Adapter->RcvResourceErrors +
                                           Adapter->RcvDmaOverrunErrors +
                                           Adapter->RcvRuntErrors;

            StatisticsInfo->ifInErrors = StatisticsInfo->ifInDiscards -
                                         Adapter->RcvResourceErrors;

            StatisticsInfo->ifHCInOctets = Adapter->InBroadcastOctets + Adapter->InMulticastOctets + Adapter->InUcastOctets;
            StatisticsInfo->ifHCInUcastPkts = Adapter->InUcastPkts;
            StatisticsInfo->ifHCInMulticastPkts = Adapter->InMulticastPkts;
            StatisticsInfo->ifHCInBroadcastPkts = Adapter->InBroadcastPkts;
            StatisticsInfo->ifHCOutOctets = Adapter->OutMulticastOctets + Adapter->OutBroadcastOctets + Adapter->OutUcastOctets;
            StatisticsInfo->ifHCOutUcastPkts = Adapter->OutUcastPkts;
            StatisticsInfo->ifHCOutMulticastPkts = Adapter->OutMulticastPkts;
            StatisticsInfo->ifHCOutBroadcastPkts = Adapter->OutBroadcastPkts;
            StatisticsInfo->ifOutErrors = Adapter->TxAbortExcessCollisions +
                                          Adapter->TxDmaUnderrun +
                                          Adapter->TxLostCRS +
                                          Adapter->TxLateCollisions;
            StatisticsInfo->ifOutDiscards = 0;


            break;

        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
    }
    DBGPRINT(MP_TRACE, ("<-- NICGetStatsCounters\n"));
    return(Status);
}

NDIS_STATUS NICSetPacketFilter(
    IN PMP_ADAPTER Adapter,
    IN ULONG PacketFilter
    )
/*++
Routine Description:

    This routine will set up the adapter so that it accepts packets
    that match the specified packet filter.  The only filter bits
    that can truly be toggled is multicast

Arguments:

    Adapter         Pointer to our adapter
    PacketFilter    The new packet filter

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_NOT_SUPPORTED

--*/
{
    NDIS_STATUS     StatusToReturn = NDIS_STATUS_SUCCESS;
    UINT            i;
    ULONG Filter;

    DBGPRINT(MP_TRACE, ("--> NICSetPacketFilter, PacketFilter=%08x\n", PacketFilter));
    NdisMoveMemory((PUCHAR)&Filter, (PUCHAR)&PacketFilter, sizeof(ULONG));

    Adapter->PacketFilter = Filter;

    if(Filter & NDIS_PACKET_TYPE_PROMISCUOUS )
    {
        RETAILMSG(TRUE,(TEXT(" Promiscous Filter being set..")));
        SetPromiscous();
    }
    else
        ClearPromiscous();

    if(Filter & NDIS_PACKET_TYPE_MULTICAST )
    {
        // clear all the content in hardware Hash Table
        ClearAllMultiCast();
        for (i = 0; i < Adapter->MCAddressCount; i++)
        {
            DBGPRINT(MP_TRACE, ("MC(%d) = %02x-%02x-%02x-%02x-%02x-%02x\n",
                i,
                Adapter->MCList[i][0],
                Adapter->MCList[i][1],
                Adapter->MCList[i][2],
                Adapter->MCList[i][3],
                Adapter->MCList[i][4],
                Adapter->MCList[i][5]));
             AddMultiCast(&(Adapter->MCList[i][0]));
        }
    }
    else
        ClearAllMultiCast();

    if(Filter & NDIS_PACKET_TYPE_DIRECTED )
    {
        SetUnicast(Adapter);
    }
    DBGPRINT(MP_TRACE, ("<-- NICSetPacketFilter, Status=%x\n", StatusToReturn));
    return(StatusToReturn);
}


NDIS_STATUS NICSetMulticastList(
    IN  PMP_ADAPTER  Adapter
    )
/*++
Routine Description:

    This routine will set up the adapter for a specified multicast address list

Arguments:

    Adapter     Pointer to our adapter

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_NOT_ACCEPTED

--*/
{
    UINT            i;

    DBGPRINT(MP_TRACE, ("--> NICSetMulticastList\n"));

    for (i = 0; i < Adapter->MCAddressCount; i++)
    {
       DBGPRINT(MP_TRACE, ("MC(%d) = %02x-%02x-%02x-%02x-%02x-%02x\n",
            i,
            Adapter->MCList[i][0],
            Adapter->MCList[i][1],
            Adapter->MCList[i][2],
            Adapter->MCList[i][3],
            Adapter->MCList[i][4],
            Adapter->MCList[i][5]));

        AddMultiCast(&(Adapter->MCList[i][0]));
    }
    DBGPRINT_S(MP_INFO, ("<-- NICSetMulticastList \n"));
    return(NDIS_STATUS_SUCCESS);
}

VOID
MPSetPowerD0(
    PMP_ADAPTER  Adapter
    )
/*++
Routine Description:

    This routine is called when the adapter receives a SetPower
    to D0.

Arguments:

    Adapter                 Pointer to the adapter structure

Return Value:


--*/
{
    NDIS_STATUS    Status;
    PMP_TCB        pMpTcb = NULL;

    //
    // MPSetPowerD0Private initializes the adapter, issues a selective reset
    //
#if defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)
    MPSetPowerD0Private (Adapter);
#endif  //  defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)
    Adapter->CurrentPowerState = NdisDeviceStateD0;
    Adapter->NextPowerState    = NdisDeviceStateD0;
    //
    // Set up the packet filter
    //
    NdisAcquireSpinLock(&Adapter->Lock);
    Status = NICSetPacketFilter(
                Adapter,
                Adapter->OldPacketFilter);
    //
    // If set packet filter succeeds, restore the old packet filter
    //
    if (Status == NDIS_STATUS_SUCCESS)
    {
        Adapter->PacketFilter = Adapter->OldPacketFilter;
    }
    NdisReleaseSpinLock(&Adapter->Lock);
    //
    // Set up the multicast list address
    //
    NdisAcquireSpinLock(&Adapter->RcvLock);
    Status = NICSetMulticastList(Adapter);
    SetUnicast(Adapter);
    NdisReleaseSpinLock(&Adapter->RcvLock);

    //
    // Enable the interrupt, so the driver can send/receive packets
    //
    RETAILMSG(TRUE, (TEXT("MPSetPowerD0- Power State D0- Enabling Interrupts\n")));
    NdisAcquireSpinLock(&Adapter->SendLock);

    FECSetMII(Adapter);
    NICEnableInterrupt(Adapter);

    NdisReleaseSpinLock(&Adapter->SendLock);
    RETAILMSG(TRUE, (TEXT("MPSetPowerD0- Interrupts Enabled \n")));
}

NDIS_STATUS
MPSetPowerLow(
    PMP_ADAPTER              Adapter ,
    NDIS_DEVICE_POWER_STATE  PowerState
    )
/*++
Routine Description:

    This routine is called when the adapter receives a SetPower
    to a PowerState > D0

Arguments:

    Adapter                 Pointer to the adapter structure
    PowerState              NewPowerState

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_PENDING
    NDIS_STATUS_HARDWARE_ERRORS


--*/
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PMP_TCB             pMpTcb = NULL;

    do
    {
        Adapter->NextPowerState = PowerState;
        Adapter->OldPacketFilter = Adapter->PacketFilter;

        // Stop sending packets. Create a new flag and make it part
        // of the Send Fail Mask
 
        // Stop hardware from receiving packets - Set the RU to idle
        //
        RETAILMSG(TRUE, (TEXT("MPSetPowerLow- Power State Changed; Disabling HW \n")));

        NdisAcquireSpinLock(&Adapter->RcvLock);

        NICDisableInterrupt(Adapter);
        FECResetMII(Adapter);

        NdisReleaseSpinLock(&Adapter->RcvLock);

        NdisAcquireSpinLock(&Adapter->SendLock);
        //Reset the check for hang counter in Low Power State
        Adapter->CurrSendHead->Count = 0;
        NdisReleaseSpinLock(&Adapter->SendLock);

        RETAILMSG(TRUE, (TEXT("MPSetPowerLow- HW Disabled \n")));
        //
        // If there are any outstanding receive packets, return NDIS_STATUS_PENDING.
        // When all the packets are returned later, the driver will complete the request.
        //
        if (Adapter->PoMgmt.OutstandingRecv != 0)
        {
            Status = NDIS_STATUS_PENDING;
            DBGPRINT_S(MP_INFO, ("MPSetPowerLow %d Pending packets from NDIS; Setting Status as PENDING\n",Adapter->PoMgmt.OutstandingRecv));
            break;
        }
        //
        // Wait for all incoming sends to complete
        //
#if defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)
        //
        // MPSetPowerLowPrivate first disables the interrupt, acknowledges all the pending
        // interrupts and sets pAdapter->CurrentPowerState to the given low power state
        // then starts Hardware specific part of the transition to low power state
        // Setting up wake-up patterns, filters, wake-up events etc
        //
        NdisMSynchronizeWithInterruptEx(
                Adapter->NdisInterruptHandle,
                0,
                (PVOID)MPSetPowerLowPrivate,
                Adapter);
#endif  //  defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)

        Status = NDIS_STATUS_SUCCESS;
    } while (FALSE);
    return Status;
}

VOID
MpSetPowerLowComplete(
    IN PMP_ADAPTER Adapter
    )
/*++
Routine Description:
    This routine is called when all the packets are returned to the driver and the driver has
    a pending OID_PNP_SET_POWER request to set it to lower power state

Arguments:

    Adapter                 Pointer to the adapter structure

Return Value:
    NOTE: this function is called with RcvLock held

--*/
{
    NDIS_STATUS        Status = NDIS_STATUS_SUCCESS;
    PNDIS_OID_REQUEST  NdisRequest;

    NdisReleaseSpinLock(&Adapter->RcvLock);
#if defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)
    //
    // MPSetPowerLowPrivate first disables the interrupt, acknowledges all the pending
    // interrupts and sets pAdapter->CurrentPowerState to the given low power state
    // then starts Hardware specific part of the transition to low power state
    // Setting up wake-up patterns, filters, wake-up events etc
    //
    NdisMSynchronizeWithInterruptEx(
                        Adapter->NdisInterruptHandle,
                        0,
                        (PVOID)MPSetPowerLowPrivate,
                        Adapter);
#endif  //  defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)

    NdisAcquireSpinLock(&Adapter->Lock);
    NdisRequest = Adapter->PendingRequest;
    Adapter->PendingRequest = NULL;
    NdisReleaseSpinLock(&Adapter->Lock);
    if (NdisRequest != NULL)
    {
        NdisRequest->DATA.SET_INFORMATION.BytesRead = sizeof(NDIS_DEVICE_POWER_STATE);
        NdisMOidRequestComplete(Adapter->AdapterHandle, NdisRequest, Status);
    }
    NdisAcquireSpinLock(&Adapter->RcvLock);
}

NDIS_STATUS
MPSetPower(
    PMP_ADAPTER     Adapter ,
    NDIS_DEVICE_POWER_STATE   PowerState
    )
/*++
Routine Description:

    This routine is called when the adapter receives a SetPower
    request. It redirects the call to an appropriate routine to
    Set the New PowerState

Arguments:

    Adapter                 Pointer to the adapter structure
    PowerState              NewPowerState

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_PENDING
    NDIS_STATUS_HARDWARE_ERRORS


--*/
{
    NDIS_STATUS       Status = NDIS_STATUS_SUCCESS;

    ASSERT(PowerState >= NdisDeviceStateD0 && PowerState <= NdisDeviceStateD3);

    if ((PowerState < NdisDeviceStateD0
        || PowerState > NdisDeviceStateD3))
    {
        Status = NDIS_STATUS_INVALID_DATA;
        return Status;
    }
    if (PowerState == NdisDeviceStateD0)
    {
        MPSetPowerD0 (Adapter);
    }
    else
    {
        Status = MPSetPowerLow (Adapter, PowerState);
    }
    return Status;
}

VOID
MPFillPoMgmtCaps (
    IN PMP_ADAPTER                 pAdapter,
    IN OUT PNDIS_PNP_CAPABILITIES  pPower_Management_Capabilities,
    IN OUT PNDIS_STATUS            pStatus,
    IN OUT PULONG                  pulInfoLen
    )
/*++
Routine Description:

    Fills in the Power  Managment structure depending the capabilities of
    the software driver and the card.

    Currently this is only supported on 82559 Version of the driver

Arguments:

    Adapter                 Pointer to the adapter structure
    pPower_Management_Capabilities - Power management struct as defined in the DDK,
    pStatus                 Status to be returned by the request,
    pulInfoLen              Length of the pPowerManagmentCapabilites

Return Value:

    Success or failure depending on the type of card
--*/

{
    BOOLEAN bIsPoMgmtSupported;

    bIsPoMgmtSupported = MPIsPoMgmtSupported(pAdapter);
    if (bIsPoMgmtSupported == TRUE)
    {
        pPower_Management_Capabilities->Flags = NDIS_DEVICE_WAKE_UP_ENABLE ;
        pPower_Management_Capabilities->WakeUpCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
        pPower_Management_Capabilities->WakeUpCapabilities.MinPatternWakeUp = NdisDeviceStateD3;
        pPower_Management_Capabilities->WakeUpCapabilities.MinLinkChangeWakeUp  = NdisDeviceStateUnspecified;
        *pulInfoLen = sizeof (*pPower_Management_Capabilities);
        *pStatus = NDIS_STATUS_SUCCESS;
    }
    else
    {
        NdisZeroMemory (pPower_Management_Capabilities, sizeof(*pPower_Management_Capabilities));
        *pStatus = NDIS_STATUS_NOT_SUPPORTED;
        *pulInfoLen = 0;
    }
}

#if defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)
NDIS_STATUS
MPAddWakeUpPattern(
    IN PMP_ADAPTER  pAdapter,
    IN PVOID        InformationBuffer,
    IN UINT         InformationBufferLength,
    OUT PULONG      BytesRead,
    OUT PULONG      BytesNeeded
    )
/*++
Routine Description:

    This routine will allocate a local memory structure, copy the pattern,
    insert the pattern into a linked list and return success

    We are gauranteed that we wll get only one request at a time, so this is implemented
    without locks.

Arguments:

    Adapter                 Adapter structure
    InformationBuffer       Wake up Pattern
    InformationBufferLength Wake Up Pattern Length
    BytesRead               Bytes Read from the Information Buffer
    BytesNeeded             Bytes Needed to process the request

Return Value:

    Success - if successful.
    NDIS_STATUS_FAILURE - if memory allocation fails.

--*/
{
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    PMP_WAKE_PATTERN        pWakeUpPattern = NULL;
    UINT                    AllocationLength = 0;
    PNDIS_PM_PACKET_PATTERN pPmPattern = NULL;
    ULONG                   Signature = 0;
    ULONG                   CopyLength = 0;

    do
    {
        pPmPattern = (PNDIS_PM_PACKET_PATTERN) InformationBuffer;

        if (InformationBufferLength < sizeof(NDIS_PM_PACKET_PATTERN))
        {
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;

            *BytesNeeded = sizeof(NDIS_PM_PACKET_PATTERN);
            break;
        }
        if (InformationBufferLength < pPmPattern->PatternOffset + pPmPattern->PatternSize)
        {
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;

            *BytesNeeded = pPmPattern->PatternOffset + pPmPattern->PatternSize;
            break;
        }

        if((pPmPattern->PatternOffset + pPmPattern->PatternSize) <
            pPmPattern->PatternOffset)
        {
            Status = NDIS_STATUS_RESOURCES;

            *BytesNeeded = sizeof(NDIS_PM_PACKET_PATTERN);
            break;
        }

        *BytesRead = pPmPattern->PatternOffset + pPmPattern->PatternSize;

        //
        // Calculate the e100 signature
        //
        Status = MPCalculateE100PatternForFilter (
            (PUCHAR)pPmPattern+ pPmPattern->PatternOffset,
            pPmPattern->PatternSize,
            (PUCHAR)pPmPattern +sizeof(NDIS_PM_PACKET_PATTERN),
            pPmPattern->MaskSize,
            &Signature );

        if ( Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        CopyLength = pPmPattern->PatternOffset + pPmPattern->PatternSize;

        //
        // Allocate the memory to hold the WakeUp Pattern
        //
        AllocationLength = sizeof (MP_WAKE_PATTERN) + CopyLength;

        pWakeUpPattern = MP_ALLOCMEMTAG(pAdapter->AdapterHandle, AllocationLength);

        if (pWakeUpPattern == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        //
        // Initialize pWakeUpPattern
        //
        NdisZeroMemory (pWakeUpPattern, AllocationLength);

        pWakeUpPattern->AllocationSize = AllocationLength;

        pWakeUpPattern->Signature = Signature;

        //
        // Copy the pattern into local memory
        //
        NdisMoveMemory (&pWakeUpPattern->Pattern[0], InformationBuffer, CopyLength);

        //
        // Insert the pattern into the list
        //
        NdisInterlockedInsertHeadList (&pAdapter->PoMgmt.PatternList,
                                        &pWakeUpPattern->linkListEntry,
                                        &pAdapter->Lock);

        Status = NDIS_STATUS_SUCCESS;
    } while (FALSE);
    return Status;
}
#endif  //  defined(WINCE_PM_ENABLE) || !defined(UNDER_CE)

NDIS_STATUS
MPRemoveWakeUpPattern(
    IN PMP_ADAPTER  pAdapter,
    IN PVOID        InformationBuffer,
    IN UINT         InformationBufferLength,
    OUT PULONG      BytesRead,
    OUT PULONG      BytesNeeded
    )
/*++
Routine Description:

    This routine will walk the list of wake up pattern and attempt to match the wake up pattern.
    If it finds a copy , it will remove that WakeUpPattern

Arguments:

    Adapter                 Adapter structure
    InformationBuffer       Wake up Pattern
    InformationBufferLength Wake Up Pattern Length
    BytesRead               Bytes Read from the Information Buffer
    BytesNeeded             Bytes Needed to process the request

Return Value:

    Success - if successful.
    NDIS_STATUS_FAILURE - if memory allocation fails.

--*/
{

    NDIS_STATUS              Status = NDIS_STATUS_FAILURE;
    PNDIS_PM_PACKET_PATTERN  pReqPattern = (PNDIS_PM_PACKET_PATTERN)InformationBuffer;
    PLIST_ENTRY              pPatternEntry = ListNext(&pAdapter->PoMgmt.PatternList) ;

    do
    {
        if (InformationBufferLength < sizeof(NDIS_PM_PACKET_PATTERN))
        {
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;

            *BytesNeeded = sizeof(NDIS_PM_PACKET_PATTERN);
            break;
        }
        if (InformationBufferLength < pReqPattern->PatternOffset + pReqPattern->PatternSize)
        {
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;

            *BytesNeeded = pReqPattern->PatternOffset + pReqPattern->PatternSize;
            break;
        }
        *BytesRead = pReqPattern->PatternOffset + pReqPattern->PatternSize;

        while (pPatternEntry != (&pAdapter->PoMgmt.PatternList))
        {
            BOOLEAN                  bIsThisThePattern = FALSE;
            PMP_WAKE_PATTERN         pWakeUpPattern = NULL;
            PNDIS_PM_PACKET_PATTERN  pCurrPattern = NULL;

            //
            // initialize local variables
            //
            pWakeUpPattern = CONTAINING_RECORD(pPatternEntry, MP_WAKE_PATTERN, linkListEntry);
            pCurrPattern = (PNDIS_PM_PACKET_PATTERN)&pWakeUpPattern->Pattern[0];

            //
            // increment the iterator
            //
            pPatternEntry = ListNext (pPatternEntry);

            //
            // Begin Check : Is (pCurrPattern  == pReqPattern)
            //
            bIsThisThePattern = MPAreTwoPatternsEqual(pReqPattern, pCurrPattern);
            if (bIsThisThePattern == TRUE)
            {
                //
                // we have a match - remove the entry
                //
                RemoveEntryList (&pWakeUpPattern->linkListEntry);

                //
                // Free the entry
                //
                MP_FREEMEM (pWakeUpPattern);
                Status = NDIS_STATUS_SUCCESS;
                break;
            }
        }
    }
    while (FALSE);

    return Status;
}

BOOLEAN
MPAreTwoPatternsEqual(
    IN PNDIS_PM_PACKET_PATTERN pNdisPattern1,
    IN PNDIS_PM_PACKET_PATTERN pNdisPattern2
    )
/*++
Routine Description:

    This routine will compare two wake up patterns to see if they are equal

Arguments:

    pNdisPattern1 - Pattern1
    pNdisPattern2 - Pattern 2


Return Value:

    True - if patterns are equal
    False - Otherwise
--*/
{
    BOOLEAN bEqual = FALSE;

    // Local variables used later in the compare section of this function
    PUCHAR  pMask1, pMask2;
    PUCHAR  pPattern1, pPattern2;
    UINT    MaskSize, PatternSize;

    do
    {
        bEqual = (BOOLEAN)(pNdisPattern1->Priority == pNdisPattern2->Priority);
        if (bEqual == FALSE)
        {
            break;
        }
        bEqual = (BOOLEAN)(pNdisPattern1->MaskSize == pNdisPattern2->MaskSize);
        if (bEqual == FALSE)
        {
            break;
        }
        //
        // Verify the Mask
        //
        MaskSize = pNdisPattern1->MaskSize ;
        pMask1 = (PUCHAR) pNdisPattern1 + sizeof (NDIS_PM_PACKET_PATTERN);
        pMask2 = (PUCHAR) pNdisPattern2 + sizeof (NDIS_PM_PACKET_PATTERN);

        bEqual = (BOOLEAN)NdisEqualMemory (pMask1, pMask2, MaskSize);
        if (bEqual == FALSE)
        {
            break;
        }
        //
        // Verify the Pattern
        //
        bEqual = (BOOLEAN)(pNdisPattern1->PatternSize == pNdisPattern2->PatternSize);
        if (bEqual == FALSE)
        {
            break;
        }
        PatternSize = pNdisPattern2->PatternSize;
        pPattern1 = (PUCHAR) pNdisPattern1 + pNdisPattern1->PatternOffset;
        pPattern2 = (PUCHAR) pNdisPattern2 + pNdisPattern2->PatternOffset;

        bEqual  = (BOOLEAN)NdisEqualMemory (pPattern1, pPattern2, PatternSize );
        if (bEqual == FALSE)
        {
            break;
        }
    } while (FALSE);

    return bEqual;
}

NDIS_STATUS
MpMethodRequest(
    IN  PMP_ADAPTER            Adapter,
    IN  PNDIS_OID_REQUEST      Request
    )
/*++
Routine Description:

    WMI method request handler

Arguments:

    MiniportAdapterContext  Pointer to the adapter structure
    Request                 Piointer to the request sent down by NDIS

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_NOT_SUPPORTED
    NDIS_STATUS_BUFFER_TOO_SHORT
    NDIS_STATUS_XXX

--*/
{
    NDIS_OID                Oid;
    ULONG                   MethodId;
    PVOID                   InformationBuffer;
    ULONG                   InputBufferLength;
    ULONG                   OutputBufferLength;
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    ULONG                   FirstInt;
    ULONG                   SecondInt;
    ULONG                   BytesNeeded = 0;
    ULONG                   BytesWritten = 0;
    ULONG                   BytesRead = 0;

    DBGPRINT(MP_TRACE, ("====> MpMethodRequest\n"));

    Oid = Request->DATA.METHOD_INFORMATION.Oid;
    InformationBuffer = (PVOID)(Request->DATA.METHOD_INFORMATION.InformationBuffer);
    InputBufferLength = Request->DATA.METHOD_INFORMATION.InputBufferLength;
    OutputBufferLength = Request->DATA.METHOD_INFORMATION.OutputBufferLength;
    MethodId = Request->DATA.METHOD_INFORMATION.MethodId;   //Only for method oids

    BytesNeeded = 0;
    switch (Oid)
    {
        case OID_CUSTOM_METHOD:
            switch(MethodId)
            {
                case ADD_TWO_INTEGERS:

                    if (InputBufferLength < 2 * sizeof(ULONG))
                    {
                        Status = NDIS_STATUS_INVALID_DATA;
                        break;
                    }
                    FirstInt = *((PULONG UNALIGNED)InformationBuffer);
                    SecondInt = *((PULONG)((PULONG UNALIGNED)InformationBuffer+1));

                    BytesRead = 2 * sizeof(ULONG);

                    BytesNeeded = sizeof(ULONG);
                    if (OutputBufferLength < BytesNeeded)
                    {
                        Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                        break;
                    }

                    *((PULONG UNALIGNED)InformationBuffer) = FirstInt + SecondInt;
                    BytesWritten = sizeof(ULONG);

                    break;

                case MINUS_TWO_INTEGERS:

                    if (InputBufferLength < 2 * sizeof(ULONG))
                    {
                        Status = NDIS_STATUS_INVALID_DATA;
                        break;
                    }
                    FirstInt = *((PULONG UNALIGNED)InformationBuffer);
                    SecondInt = *((PULONG)((PULONG UNALIGNED)InformationBuffer+1));

                    BytesRead = 2 * sizeof(ULONG);

                    BytesNeeded = sizeof(ULONG);
                    if (OutputBufferLength < BytesNeeded)
                    {
                        Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                        break;
                    }
                    if (FirstInt < SecondInt)
                    {
                        Status = NDIS_STATUS_INVALID_DATA;
                        break;
                    }

                    *((PULONG UNALIGNED)InformationBuffer) = FirstInt - SecondInt;
                    BytesWritten = sizeof(ULONG);
                    break;

                default:
                    Status = NDIS_STATUS_NOT_SUPPORTED;
                    break;
            }
            break;

        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
    }
    if (Status != NDIS_STATUS_SUCCESS)
    {
        Request->DATA.METHOD_INFORMATION.BytesNeeded = BytesNeeded;
    }
    else
    {
        Request->DATA.METHOD_INFORMATION.BytesWritten = BytesWritten;
        Request->DATA.METHOD_INFORMATION.BytesRead = BytesRead;
    }
    DBGPRINT(MP_TRACE, ("<====> MpMethodRequest Status: %x\n", Status));
    return Status;
}

VOID
FECCancelOidRequest(
    IN  NDIS_HANDLE            MiniportAdapterContext,
    IN  PVOID                  RequestId
    )
/*++
Routine Description:

    This function aborts the request pending in the miniport.

Arguments:

    MiniportAdapterContext  Pointer to the adapter structure
    RequestId               Specify the request to be cancelled.

Return Value:

--*/
{
    PNDIS_OID_REQUEST    PendingRequest;
    PMP_ADAPTER          Adapter = (PMP_ADAPTER) MiniportAdapterContext;

    DBGPRINT(MP_TRACE, ("====> FECCancelOidRequest\n"));

    NdisAcquireSpinLock(&Adapter->Lock);

    if (Adapter->PendingRequest != NULL
            && Adapter->PendingRequest->RequestId == RequestId)
    {
        PendingRequest = Adapter->PendingRequest;
        Adapter->PendingRequest = NULL;

        NdisReleaseSpinLock(&Adapter->Lock);

        NdisMOidRequestComplete(Adapter->AdapterHandle,
                            PendingRequest,
                            NDIS_STATUS_REQUEST_ABORTED);
    }
    else
    {
        NdisReleaseSpinLock(&Adapter->Lock);
    }
    DBGPRINT(MP_TRACE, ("<==== FECCancelOidRequest\n"));
}

