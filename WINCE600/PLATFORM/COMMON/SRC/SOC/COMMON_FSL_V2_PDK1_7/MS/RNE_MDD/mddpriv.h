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
/*

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  
    mddpriv.h

Abstract:  
    This module contains private declarations for rndismdd module.

Functions:

    
--*/

#ifndef _MDDPRIV_H

//
//  Global defines affecting MDD behaviour..
//

#define DATA_WRAPPER_LOOKASIDE_DEPTH        32
#define RCV_RNDIS_PACKET_LOOKASIDE_DEPTH    32
#define SEND_DATA_WRAPPER_DEPTH             32
#define RNDIS_PACKET_HEADER_DEPTH           32

#define RNDIS_MAX_PACKETS_PER_MESSAGE       16  //  in RndisInitializeComplete
                                                //      Arbitrarily chosen.
#define SUPPORTED_RNDIS_MAJOR_VER           1
#define SUPPORTED_RNDIS_MINOR_VER           0

#define MDD_DRIVER_MAJOR_VERSION            1
#define MDD_DRIVER_MINOR_VERSION            0

#define DEFAULT_MULTICASTLIST_MAX           8   //  Most adapters should support
                                                //      this minimum no..

#define MAXIMUM_NDIS_PACKET_POOL            64  //  Used to indicate packet to
#define MAXIMUM_NDIS_BUFFER_POOL            64  //      CE stack.


//
//  Device State according to RNDIS spec.
//

#define RNDIS_UNINITIALIZED     0x00000001  //  In the beginning..
#define RNDIS_INITIALIZED       0x00000002  //  After RNDIS_INITIALIZE msg.
#define RNDIS_DATA_INITIALIZED  0x00000004  //  When filter is set..



//
//  dwMdd Flag values..
//

#define MDD_FLAG_WAITING        0x00000001      // MDD is waiting for event..


//
//  Global MDD structure.
//

typedef struct _RNDIS_MDD
{
    //
    //  MDD operation..
    //

//  CRITICAL_SECTION    lockMdd;                //  Global MDD lock.
    DWORD               dwDeviceState;          //  From host point of view.
    DWORD               dwMddFlag;              //  MDD operational.    
    DWORD               dwSysIntr;              //  Sysintr from PDD
    BOOL                bQuitNow;
    
    HANDLE              hRndisMddEvent;         //  Time to work!!      
    
//  CRITICAL_SECTION    lockInRndisPackets;     //  For listInRndisPackets
//  LIST_ENTRY          listInRndisPackets;     //  In rndis packets
    
    
//  CRITICAL_SECTION    lockVMiniNdisPackets;   //  For the list & buffer..
    DWORD               dwTotalVMiniPendings;   //  # entries in queue..
    LIST_ENTRY          listVMiniNdisPackets;   //  NdisPacket fr VMini 
//  PUCHAR              pucTxBuffer;            //  The flat TX buffer..
    BOOL                bPddSending;            //  After 
                                                //  SendRndisPacketHandler()


    //
    //  Rndis miniport operation.
    //

    DWORD               dwCurrentPacketFilter;  //  Current Filter.     
    DWORD               dwTransmitOkay;         //  Statistics..
    DWORD               dwReceiveOkay;
    DWORD               dwTransmitError;
    DWORD               dwReceiveError;
    DWORD               dwReceiveNoBuffer;

    UCHAR               PermanentNwAddr[6];
    UCHAR               MulticastAddresses[DEFAULT_MULTICASTLIST_MAX][6];
    DWORD               dwMulticastListInUse;
    
    
    //
    //  CE miniport operation..
    //

    BOOL                bMiniportBound;


    //
    //  PDD functions...
    //

    RNDIS_PDD_CHARACTERISTICS   PddCharacteristics; 


    //
    //  From RNDIS host..
    //

    DWORD           dwHostMaxRx;        //  From RNDIS_INIT, host max RX
    DWORD           dwDeviceMaxRx;      //  From PDD, our max RX    

}   RNDIS_MDD, *PRNDIS_MDD;

//
//  Structure holding information about indicated RNDIS_PACKET
//

typedef struct _RCV_RNDIS_PACKET
{
    PDATA_WRAPPER   pDataWrapper;       //  DataWrapper to return to PDD
    DWORD           dwReturnsPending;   //  When == 0, return it..

}   RCV_RNDIS_PACKET, *PRCV_RNDIS_PACKET;

//
//  Ref count for shared resources..
//
/*
#define NDIS_EVENT                  HANDLE

#define KeSetEvent(pEvent, i, b)    SetEvent(*(pEvent))
#define KeResetEvent(pEvent)        ResetEvent(*(pEvent))
#define KeWaitForSingleObject       WaitForSingleObject

#define KeInitializeEvent(      \
            pHandle,            \
            bManualReset,       \
            bInitialState)      \
                                \
        *(pHandle) = CreateEvent(NULL, (bManualReset), (bInitialState), NULL)

#define NdisResetEvent(pEvent)      KeResetEvent(pEvent)
#define NdisSetEvent(pEvent)        KeSetEvent(pEvent, EVENT_INCREMENT, FALSE)
#define NdisInitializeEvent(pEvent) KeInitializeEvent(pEvent, TRUE, FALSE)  
#define NdisWaitEvent(pEvent, Time_ms)  \
                                    KeWaitForSingleObject(*(pEvent), Time_ms)


typedef struct _WAIT_REFCOUNT
{
    LONG            Refcount;           // The refcount
    NDIS_EVENT      Event;              // Signaled when RefCount hits 0

} WAIT_REFCOUNT, *PWAIT_REFCOUNT;



_inline VOID
InitializeWaitRef(
    IN PWAIT_REFCOUNT   pRefcount)
{
    NdisInitializeEvent(&pRefcount->Event);
    pRefcount->Refcount = 0L;
    
    //
    //  The event starts life signaled since the refcount starts at zero
    //
    NdisSetEvent(&pRefcount->Event);
}


_inline VOID
IncrementWaitRef(
    IN PWAIT_REFCOUNT   pRefcount)
{
    LONG        Scratch;
    
    ASSERT( pRefcount != NULL );
    Scratch = InterlockedIncrement(&pRefcount->Refcount);
    ASSERT( Scratch > 0L );

    if( Scratch == 1L )
    {
        //
        //  We incremented from zero. Reset the event.
        //

        NdisResetEvent( &pRefcount->Event );
    }
}


_inline VOID
DecrementWaitRef(
    IN PWAIT_REFCOUNT   pRefcount)
{
    LONG        Scratch;

    ASSERT( pRefcount != NULL );
    Scratch = InterlockedDecrement(&pRefcount->Refcount);
    ASSERT( Scratch >= 0L );

    if( Scratch == 0L )
    {
        //
        //  Signal anyone waiting for the refount to go to zero
        //

        NdisSetEvent( &pRefcount->Event );
    }
}


_inline VOID
WaitOnRef(
    IN PWAIT_REFCOUNT   pRefcount)
{
    ASSERT( pRefcount != NULL );

    //
    // Blocks until a wait-refcount reaches zero
    //
    NdisWaitEvent( &pRefcount->Event, INFINITE);
}





//
//  Pool-tag value definitions, sorted by tag value
//

#define MEM_TAG_DATA_WRAPPER                'DaWT'
#define MEM_TAG_RCV_RNDIS_PACKET            'RrpT'
#define MEM_TAG_SEND_DATA_WRAPPER           'SdwT'
#define MEM_TAG_RNDIS_PACKET_HEADER         'RphT'



//
//  Useful macros.. 
//

#define ALLOCATE_DATA_WRAPPER()  \
    ExAllocateFromNPagedLookasideList(&DataWrapperLookAsideList)

#define FREE_DATA_WRAPPER(Block) \
    ExFreeToNPagedLookasideList(&DataWrapperLookAsideList, (Block))


#define ALLOCATE_RCV_RNDIS_PACKET() \
    ExAllocateFromNPagedLookasideList(&RcvRndisPacketLookAsideList)

#define FREE_RCV_RNDIS_PACKET(Block) \
    ExFreeToNPagedLookasideList(&RcvRndisPacketLookAsideList, (Block))

*/

//
//  Functions exported by rndis.c
//

BOOL
RndisInit( BYTE *pbBaseAddress, DWORD dwMultiplier, USHORT MacAddr[3]);

//RndisInit(void);

void
RndisDeInit(void);

void
RndisProcessMessage(PDATA_WRAPPER pDataWrapper);

BOOL
RndisProcessPacket (PDATA_WRAPPER pDataWrapper);

void
RndisReturnIndicatedPacket(PNDIS_PACKET pNdisPacket);

BOOL
RndisSendPacket(PNDIS_PACKET pNdisPacket);



//
//  Functions exported by miniport.c
//

BOOL
HostMiniInit(BYTE *pbBaseAddress, DWORD dwMultiplier, USHORT MacAddr[3]);

void
HostMiniDeInit(void);

NDIS_STATUS
HostMiniQueryInformation(    
    IN NDIS_OID Oid,
    IN PVOID    pvInformationBuffer,
    IN ULONG    ulInformationBufferLength,
    OUT PULONG  pulBytesWritten,
    OUT PULONG  pulBytesNeeded
    );


NDIS_STATUS
HostMiniSetInformation(
   IN   NDIS_OID    Oid,
   IN   PVOID       pvInformationBuffer,
   IN   ULONG       ulInformationBufferLength,
   OUT  PULONG      pulBytesRead,
   OUT  PULONG      pulBytesNeeded
   );

void
RndisMini1Bind(BOOL bBind);

void
RndisMini1HardBind();

void
RndisRestart(BOOL bResetPDD);



//
//  Functions exported by CEVmini.c
//

BOOL
VMiniDriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath);


void
VMiniIndicatePackets(
    PPNDIS_PACKET   NdisPacketsArray,
    UINT            uiNumberOfPackets);

void
VMiniIndicatePacketDone(
    IN PNDIS_PACKET     pNdisPacket);

VOID
VMiniInstantiateMiniport(bInstantiate);





#endif  //  _MDDPRIV_H
