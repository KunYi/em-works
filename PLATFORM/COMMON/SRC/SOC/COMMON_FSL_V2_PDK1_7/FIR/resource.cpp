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
//  File:  resource.cpp
//
//   This file implements the device specific functions for iMX51 fir device.
//
//------------------------------------------------------------------------------
#include "IrFir.h"

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

// FIRI registers base address
PCSP_FIRI_REG g_pVFiriReg = NULL;

// UART registers base address
PCSP_UART_REG g_pVSIRReg = NULL;

static const USHORT fcsTable[256] =
{
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};


//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: ComputeFCS
//
//  This function computes the FCS based on FCS table.
//
// Parameters:
//      data
//          [in] .
//      dataLen
//          [in] .
//
// Returns:
//    This function returns calculated fcs.
//
//-----------------------------------------------------------------------------
USHORT ComputeFCS( UCHAR *data, UINT dataLen )
{
    USHORT fcs = 0xffff;
    UINT i;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Resource: +ComputeFCS() on %d-byte buffer.\r\n"), dataLen));

    for (i = 0; i < dataLen; i++)
    {
        fcs = (fcs >> 8) ^ fcsTable[(fcs ^ *data++) & 0xff];
    }

    fcs = ~fcs;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Resource: -ComputeFCS returning 0x%x.\r\n"), fcs));
    return fcs;
}


//-----------------------------------------------------------------------------
//
// Function: GetPacketInfo
//
//  This function retrieves the packet information.
//
// Parameters:
//      packet
//          [in] .
//
// Returns:
//    This function returns the pointer to the retrieved packet information.
//
//-----------------------------------------------------------------------------
PNDIS_IRDA_PACKET_INFO GetPacketInfo( PNDIS_PACKET packet )
{
    MEDIA_SPECIFIC_INFORMATION *mediaInfo;
    UINT size;

    NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(packet, (PPVOID)&mediaInfo, &size);
    
    if(mediaInfo != NULL)
        return (PNDIS_IRDA_PACKET_INFO)mediaInfo->ClassInformation;
    else
        return NULL;
}


//-----------------------------------------------------------------------------
//
// Function: InsertBufferSorted
//
//  This function inserted the received buffer to the tail of the list.
//
// Parameters:
//      Head
//          [in] .
//      rcvBuf
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
void InsertBufferSorted( PLIST_ENTRY Head, rcvBuffer *rcvBuf )
{
    PLIST_ENTRY ListEntry;

    if (IsListEmpty(Head))
    {
        InsertHeadList(Head, &rcvBuf->listEntry);
    }
    else
    {
        BOOLEAN EntryInserted = FALSE;

        for (ListEntry = Head->Flink; ListEntry != Head; ListEntry = ListEntry->Flink)
        {
            rcvBuffer *temp = CONTAINING_RECORD(ListEntry, rcvBuffer, listEntry);

            if (temp->dataBuf > rcvBuf->dataBuf)
            {
                // We found one that comes after ours.
                // We need to insert before it

                InsertTailList(ListEntry, &rcvBuf->listEntry);
                EntryInserted = TRUE;
                break;
            }
        }

        if (!EntryInserted)
        {
            // We didn't find an entry on the last who's address was later
            // than our buffer.  We go at the end.
            InsertTailList(Head, &rcvBuf->listEntry);
        }
    }
}


//-----------------------------------------------------------------------------
//
// Function: MyRemoveHeadList
//
//  This function removes the entry from the head of the list.
//
// Parameters:
//      ListHead
//          [in] .
//
// Returns:
//    This function returns pointer to the removed list entry.
//
//-----------------------------------------------------------------------------
PLIST_ENTRY MyRemoveHeadList( IN PLIST_ENTRY ListHead )
{
    PLIST_ENTRY pListEntry;

    if (IsListEmpty(ListHead))
        pListEntry = NULL;
    else
        pListEntry = RemoveHeadList(ListHead);

    return pListEntry;
}


//-----------------------------------------------------------------------------
//
// Function: MyMemAlloc
//
//  This function allocates a memory area of specified size.
//
// Parameters:
//      size
//          [in] .
//
// Returns:
//    This function returns pointer to the allocated memory area.
//
//-----------------------------------------------------------------------------
PVOID MyMemAlloc( UINT size )
{
    NDIS_STATUS stat;
    PVOID memptr;

    NDIS_PHYSICAL_ADDRESS noMaxAddr = NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);
    stat = NdisAllocateMemory(&memptr, size, 0, noMaxAddr);

    if (stat == NDIS_STATUS_SUCCESS)
    {
        PREFAST_SUPPRESS(419, "Suppressed since we are allocating proper memory before filling it with iMX51");
        NdisZeroMemory((PVOID)memptr, size);
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Resource: Memory allocation failed\r\n")));
        memptr = NULL;
    }

    return memptr;
}


//-----------------------------------------------------------------------------
//
// Function: MyMemFree
//
//  This function frees the specified memory area.
//
// Parameters:
//      memptr
//          [in] .
//      size
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID MyMemFree( PVOID memptr, UINT size )
{
    NdisFreeMemory(memptr, size, 0);
}


//-----------------------------------------------------------------------------
//
// Function: InitDevice
//
//  This function initializes the device resources.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID InitDevice( pFirDevice_t thisDev )
{

    //NdisZeroMemory((PVOID)thisDev, sizeof(FirDevice_t));
    NdisInitializeListHead(&thisDev->SendQueue);

    NdisAllocateSpinLock(&thisDev->Lock);

    NdisInitializeListHead(&thisDev->rcvBufBuf);
    NdisInitializeListHead(&thisDev->rcvBufFree);
    NdisInitializeListHead(&thisDev->rcvBufFull);
    NdisInitializeListHead(&thisDev->rcvBufPend);

    DEBUGMSG(ZONE_INIT, (TEXT("Resources: Free:%x Full:%x Pend:%x\r\n"),
        &thisDev->rcvBufFree, &thisDev->rcvBufFull, &thisDev->rcvBufPend));
}


//-----------------------------------------------------------------------------
//
// Function: NewDevice
//
//  This function allocates and initializes the device resources.
//
// Parameters:
//      None.
//
// Returns:
//    This function returns the pointer to the created Fir Device.
//
//-----------------------------------------------------------------------------
pFirDevice_t NewDevice( void )
{
    pFirDevice_t newdev;

    newdev = (pFirDevice_t)MyMemAlloc(sizeof(FirDevice_t));

    if (newdev)
        InitDevice(newdev);

    return newdev;
}


//-----------------------------------------------------------------------------
//
// Function: FreeDevice
//
//  This function frees the device resources.
//
// Parameters:
//      dev
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID FreeDevice( pFirDevice_t dev )
{
    CloseDevice(dev);
    NdisFreeSpinLock(&dev->Lock);
    MyMemFree((PVOID)dev, sizeof(FirDevice_t));
}


//-----------------------------------------------------------------------------
//
// Function: OpenDevice
//
//  This function opens the Fir device.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    This function returns status of open device.
//
//-----------------------------------------------------------------------------
BOOLEAN OpenDevice( pFirDevice_t thisDev )
{
    NDIS_STATUS stat;
    BOOLEAN result = FALSE;
    UINT bufIndex;

    DEBUGMSG(ZONE_OPEN, (TEXT("Resource: +OpenDevice()\r\n")));

    if (!thisDev)
    {
        return FALSE;
    }

    thisDev->SirPhyAddr = CspFirGetSIRBaseRegAddr();
    if(!SirInitialize(thisDev))
    {
        goto done;
    }

    thisDev->FirPhyAddr = CspFirGetFIRBaseRegAddr();
    if(!FirInitialize(thisDev))
    {
        goto done;
    }

    // ---------------------------------------------------------------
    // Initialize memory resouces for NDIS of various buffers
    // ---------------------------------------------------------------
    //  Allocate the NDIS packet and NDIS buffer pools
    //  for this device's RECEIVE buffer queue.
    //  Our receive packets must only contain one buffer apiece,
    //  so #buffers == #packets.

    NdisAllocatePacketPool(&stat, &thisDev->packetPoolHandle, NUM_RCV_BUFS, 16);
    if (stat != NDIS_STATUS_SUCCESS)
    {
        goto done;
    }

    NdisAllocateBufferPool(&stat, &thisDev->bufferPoolHandle, NUM_RCV_BUFS);
    if (stat != NDIS_STATUS_SUCCESS)
    {
        goto done;
    }

    //  Initialize each of the RECEIVE packet objects for this device.
    for (bufIndex = 0; bufIndex < NUM_RCV_BUFS; bufIndex++)
    {
        PVOID buf;
        rcvBuffer *rcvBuf = (pRcvBuffer_t)MyMemAlloc(sizeof(rcvBuffer));

        if (!rcvBuf)
        {
            goto done;
        }

        rcvBuf->state = STATE_FREE;
        rcvBuf->dataBuf = NULL;
        rcvBuf->isDmaBuf = FALSE;

        //  Allocate the NDIS_PACKET.
        NdisAllocatePacket(&stat, &rcvBuf->packet, thisDev->packetPoolHandle);
        if (stat != NDIS_STATUS_SUCCESS)
        {
            goto done;
        }

        //  For future convenience, set the MiniportReserved portion of the packet
        //  to the index of the rcv buffer that contains it.
        //  This will be used in ReturnPacketHandler.
        *(ULONG *)rcvBuf->packet->MiniportReserved = (ULONG)rcvBuf;

        rcvBuf->dataLen = 0;

        InsertHeadList(&thisDev->rcvBufFree, &rcvBuf->listEntry);

        buf = MyMemAlloc(RCV_BUFFER_SIZE);
        if(!buf)
        {
            goto done;
        }

        InsertHeadList(&thisDev->rcvBufBuf, (PLIST_ENTRY)buf);
    }

#if 0
    PLIST_ENTRY pListEntry;

    thisDev->writeBuf = (PUCHAR)MyMemAlloc(MAX_IRDA_DATA_SIZE);
    if (!thisDev->writeBuf)
    {
        goto done;
    }

    pListEntry = MyRemoveHeadList(&thisDev->rcvBufBuf);
    if(pListEntry)
        thisDev->readBuf = (PUCHAR) LIST_ENTRY_TO_RCV_BUF(pListEntry);
    else
        goto done;
#endif
    //  Set mediaBusy to TRUE initially.  That way, we won't
    //  IndicateStatus to the protocol in the ISR unless the
    //  protocol has expressed interest by clearing this flag
    //  via MiniportSetInformation(OID_IRDA_MEDIA_BUSY).
    thisDev->mediaBusy = FALSE;
    // Set default speed as 9600
    thisDev->newSpeed = DEFAULT_BAUD_RATE;

    thisDev->lastPacketAtOldSpeed = NULL;
    thisDev->setSpeedAfterCurrentSendPacket = FALSE;

    result = TRUE;

done:
    if (!result)
    {
        //  If we're failing, close the device to free up any resources
        //  that were allocated for it.
        CloseDevice(thisDev);
        DEBUGMSG(ZONE_ERROR, (TEXT("Resource: OpenDevice() failed\r\n")));
    }
    else
    {
        DEBUGMSG(ZONE_OPEN, (TEXT("Resource: OpenDevice() succeeded\r\n")));
    }

    DEBUGMSG(ZONE_OPEN, (TEXT("Resource: -OpenDevice\r\n")));
    return result;
}


//-----------------------------------------------------------------------------
//
// Function: CloseDevice
//
//  This function closes the Fir device.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID CloseDevice( pFirDevice_t thisDev )
{
    PLIST_ENTRY ListEntry;

    DEBUGMSG(ZONE_CLOSE, (TEXT("Resource: +CloseDevice()\r\n")));

    if (!thisDev)
        return;

    // ---------------------------------------------------------------
    // Free memory resouces for NDIS of various buffers
    // ---------------------------------------------------------------

    // Free all resources for the RECEIVE buffer queue.
    while (!IsListEmpty(&thisDev->rcvBufFree))
    {
        pRcvBuffer_t rcvBuf;

        ListEntry = RemoveHeadList(&thisDev->rcvBufFree);
        rcvBuf = CONTAINING_RECORD(ListEntry,rcvBuffer,listEntry);
        if (rcvBuf->packet)
        {
            NdisFreePacket(rcvBuf->packet);
            rcvBuf->packet = NULL;
        }
        MyMemFree(rcvBuf, sizeof(rcvBuffer));
    }

    while (!IsListEmpty(&thisDev->rcvBufBuf))
    {
        ListEntry = RemoveHeadList(&thisDev->rcvBufBuf);
        MyMemFree(ListEntry, RCV_BUFFER_SIZE);
    }

    // Free the packet and buffer pool handles for this device.
    if (thisDev->packetPoolHandle)
    {
        NdisFreePacketPool(thisDev->packetPoolHandle);
        thisDev->packetPoolHandle = NULL;
    }

    if (thisDev->bufferPoolHandle)
    {
        NdisFreeBufferPool(thisDev->bufferPoolHandle);
        thisDev->bufferPoolHandle = NULL;
    }

    // Free all resources for the SEND buffer queue.
    while ((ListEntry = MyRemoveHeadList(&thisDev->SendQueue)) != 0)
    {
        PNDIS_PACKET Packet = CONTAINING_RECORD(ListEntry, NDIS_PACKET, MiniportReserved);
        NdisAcquireSpinLock(&thisDev->Lock);
        NdisMSendComplete(thisDev->ndisAdapterHandle, Packet, NDIS_STATUS_FAILURE);
        NdisReleaseSpinLock(&thisDev->Lock);
    }

    thisDev->mediaBusy = FALSE;
    thisDev->newSpeed = 0;
    thisDev->linkSpeedInfo = NULL;
    thisDev->lastPacketAtOldSpeed = NULL;
    thisDev->setSpeedAfterCurrentSendPacket = FALSE;

    //-----------------------------------------------------------------
    // Free hardware memory
    //-----------------------------------------------------------------
    thisDev->readBuf  = NULL;
    thisDev->writeBuf = NULL;


    FirDeinitialize(thisDev);
    SirDeInitialize(thisDev);
}


//-----------------------------------------------------------------------------
//
// Function: QueueReceivePacket
//
//  This function queue the received packet.
//
// Parameters:
//      thisDev
//          [in] .
//      data
//          [in] .
//      dataLen
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID QueueReceivePacket(pFirDevice_t thisDev, PUCHAR data, UINT dataLen,BOOLEAN IsFIR)
{
    rcvBuffer *rcvBuf = NULL;
    PLIST_ENTRY ListEntry;

    // Note: We cannot use a spinlock to protect the rcv buffer structures
    // in an ISR.  This is ok, since we used a sync-with-isr function
    // the the deferred callback routine to access the rcv buffers.
    DEBUGMSG(ZONE_FUNCTION, (TEXT("Resource: +QueueReceivePacket(0x%x, 0x%lx, 0x%x)\r\n"),
        (UINT) thisDev, data, dataLen));

    if (IsListEmpty(&thisDev->rcvBufFree))
    {
        ListEntry = NULL;
    }
    else
    {
        ListEntry = MyRemoveHeadList(&thisDev->rcvBufFree);
    }

    if (ListEntry)
    {
        if ((rcvBuf = CONTAINING_RECORD(ListEntry, rcvBuffer, listEntry)) != 0)
        {
            rcvBuf->dataBuf = data;
            rcvBuf->state = STATE_FULL;
            rcvBuf->dataLen = dataLen;
            rcvBuf->isDmaBuf = IsFIR;
            InsertTailList(&thisDev->rcvBufFull, ListEntry);
        }
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("Resource: -QueueReceivePacket\r\n")));
}
