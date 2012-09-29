//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: common_dcp.h
//  Brief data co-processor interface
//
//
//-----------------------------------------------------------------------------

#if !defined( COMMON_DCP_H )
#define COMMON_DCP_H 1

#include "common_regsdcp.h"

#ifdef __cplusplus
extern "C" {
#endif


#define DCP_GET_ANY_CHANNEL 0x0000ffff

#define DCP_HANDLE_SHIFT    28
#define CHANNEL_FROM_HANDLE(dcpHandle) ((dcpHandle >> (DCP_HANDLE_SHIFT)) & 0x00000003)
#define DCP_HANDLE_MASK (0x0fffffff)

#define DCP_VMI_CHANNEL 0

#define DCP_MAX_CHANNELS        4
#define DCP_MAX_LOCKED_CHANNELS (DCP_MAX_CHANNELS-1)

typedef UINT32 DCPHandle_t;

typedef long RtResult;

typedef void (*pHandlerFunction_t)(void*);

typedef void (*DCPCallback_t)(void *);

typedef union _DCPPacketStatus_t
{
    UINT32 U;
    struct
    {
        unsigned COMPLETE :1;
        unsigned HASH_MISMATCH : 1;
        unsigned ERROR_SETUP : 1;
        unsigned ERROR_PACKET : 1;
        unsigned ERROR_SRC : 1;
        unsigned ERROR_DST : 1;
        unsigned rsvd : 10;
        unsigned ERROR_CODE : 8;
        unsigned TAG : 8;
    } B;
} DCPPacketStatus_t;

typedef struct _DCPWorkPacket_t
{
    UINT32              *NextPacket;
    hw_dcp_packet1_t    Ctrl0;
    hw_dcp_packet2_t    Ctrl1;
    void                *Source;
    void                *Destination;
    UINT32              Size;
    UINT32              *Payload;
    DCPPacketStatus_t   Status;
} DCPWorkPacket_t;

typedef struct _DCPChannel_t
{
    UINT32        Available;
    UINT32        Locked;
    DCPCallback_t   CallbackFunction;
    void *          CallbackData;
    UINT32        Handle;
    HANDLE    swSemaphore;
    DCPWorkPacket_t WorkPacket;
} DCPChannel_t;

typedef enum _DCPPriority_t {
    DCP_BACKGROUND,
    DCP_LOW_PRIORITY,
    DCP_MEDIUM_PRIORITY,
    DCP_HIGH_PRIORITY
} DCPPriority_t;

//-----------------------------------------------------------------------------
// prototypes
//-----------------------------------------------------------------------------
BOOL dcp_hw_IsEnabled(void);
RtResult dcp_hw_Initialize(void);
RtResult dcp_hw_RunChannel(UINT32 Channel,  DCPWorkPacket_t *Packets, UINT32 NumberOfPackets);
RtResult dcp_hw_GatherResidualWrites(BOOL Enable);
RtResult dcp_hw_ContextCaching(BOOL Enable);
RtResult dcp_hw_ContextSwitching(BOOL Enable, void *Buffer);
RtResult dcp_hw_Enable(BOOL Enable);
RtResult dcp_hw_ChannelInterruptEnable(UINT32 Channel, BOOL Enable);
RtResult dcp_hw_ChannelInterruptClear(UINT32 Channel);
RtResult dcp_hw_ChannelEnable(UINT32 Channel, BOOL Enable);
RtResult dcp_hw_SetChannelHighPriority(UINT32 Channel, BOOL Set);

RtResult dcp_hw_VMIMergeIRQ(BOOL Merge);

// DCP
RtResult dcp_Initialize(void);
RtResult dcp_ExecutePackets(DCPHandle_t Handle, DCPWorkPacket_t *Packets, UINT32 NumberOfPackets);
RtResult dcp_AcquireChannel(DCPHandle_t *Handle, UINT32 DesiredChannel, BOOL LockIt);
RtResult dcp_ReleaseLockedChannel(DCPHandle_t Handle);
RtResult dcp_SetCallbackInfo(DCPHandle_t Handle, DCPCallback_t Function, void *PrivateData);
RtResult dcp_GetChannelNumber(DCPHandle_t Handle, UINT32 *Channel);
RtResult dcp_memcopyAsync(void *Source, void *Destination, UINT32 Length, DCPCallback_t pCallback, void *PrivateData, DCPHandle_t *Handle);
RtResult dcp_memcopy(void *Source, void *Destination, UINT32 Length);
RtResult dcp_WaitForComplete(DCPHandle_t Handle, UINT32 TimeOut);
RtResult dcp_LockChannel(DCPHandle_t *Handle, UINT32 DesiredChannel);
RtResult dcp_GetChannelWorkPacket(DCPHandle_t Handle, DCPWorkPacket_t **Packet);
RtResult dcp_blt(void *Source, void *Destination, UINT32 Width, UINT32 Height, UINT32 Stride, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle);
RtResult dcp_bltfill(UINT32 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Width, UINT32 Height, UINT32 Stride, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle);
RtResult dcp_SetChannelPriority(UINT32 Channel, DCPPriority_t Priority);
RtResult dcp_memfill(UINT8 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Length);
RtResult dcp_memfillAsync(UINT8 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Length, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle);

#ifdef __cplusplus
} //extern "C"
#endif

#endif

