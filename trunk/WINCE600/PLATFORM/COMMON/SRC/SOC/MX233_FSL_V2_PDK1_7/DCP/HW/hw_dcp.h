//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: hw_dcp.h
//  Brief data co-processor interface
//
//
/////////////////////////////////////////////////////////////////////////////////
#if !defined( HW_DCP_H )
#define HW_DCP_H 1

#include "hw_dcp_common.h"
#include "csp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DCP_MAX_CSC_CHANNELS    1
#define DCP_MAX_CHANNELS        4
#define DCP_MAX_LOCKED_CHANNELS (DCP_MAX_CHANNELS-1)


typedef struct _CSCCoefficients_t
{
    hw_dcp_csccoeff0_t Register0;
    hw_dcp_csccoeff1_t Register1;
    hw_dcp_csccoeff2_t Register2;
} CSCCoefficients_t;

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

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////
BOOL dcp_hw_IsEnabled(void);
HRESULT dcp_hw_Initialize(void);
HRESULT dcp_hw_RunChannel(UINT32 Channel,  DCPWorkPacket_t *Packets, UINT32 NumberOfPackets);
HRESULT dcp_hw_GatherResidualWrites(BOOL Enable);
HRESULT dcp_hw_ContextCaching(BOOL Enable);
HRESULT dcp_hw_ContextSwitching(BOOL Enable, void *Buffer);
HRESULT dcp_hw_Enable(BOOL Enable);
HRESULT dcp_hw_ChannelInterruptEnable(UINT32 Channel, BOOL Enable);
HRESULT dcp_hw_ChannelInterruptClear(UINT32 Channel);
HRESULT dcp_hw_ChannelEnable(UINT32 Channel, BOOL Enable);
HRESULT dcp_hw_SetChannelHighPriority(UINT32 Channel, BOOL Set);

HRESULT dcp_hw_Rotate(BOOL Enable);
HRESULT dcp_hw_SetScale(UINT32 InputWidth, UINT32 InputHeight, UINT32 OutputWidth, UINT32 OutputHeight);
HRESULT dcp_hw_SetOutputSize(UINT32 Width, UINT32 Height);
HRESULT dcp_hw_SetInputSize(UINT32 Width, UINT32 Height);
HRESULT dcp_hw_SetOutputBuffer(void *Buffer);
HRESULT dcp_hw_SetInputBuffer(void *Y, void *U, void *V);
HRESULT dcp_hw_SetOutputFormat(CSCFormat_t Format);
HRESULT dcp_hw_SetInputFormat(CSCFormat_t Format);
HRESULT dcp_hw_RunCSC(void);
HRESULT dcp_hw_CSCInterruptEnable(BOOL Enable);
HRESULT dcp_hw_CSCInterruptClear(void);
HRESULT dcp_hw_CSCSetPriority(UINT32 Priority);

HRESULT dcp_hw_VMIMergeIRQ(BOOL Merge);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
