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
//  Copyright (C) 2004-2007,2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#ifndef __PINDRIVER_H
#define __PINDRIVER_H

#include "marshal.hpp" //helper classes to marshal/alloc embedded/async buffer

#ifdef __cplusplus
extern "C" {
#endif

#define PIN_TIMEOUT    250
#define WAIT_BUFFER_TIMEOUT             2000     // Timeout for receive a buffer from SMFC module,

#define PIN_REG_PATH                    TEXT("Software\\Microsoft\\DirectX\\DirectShow\\Capture")


typedef struct _tagStreamDescriptorShadow
{
    PCS_STREAM_DESCRIPTOR pCsStreamDescriptorExternal;
    CS_STREAM_DESCRIPTOR  csStreamDescriptorShadow;
    BOOL                  m_fBusy;
    MarshalledBuffer_t   *m_pMarshalledDataBuffer; //Marshalled/Async allocated data buffer when the memory model is not CSPROPERTY_BUFFER_DRIVER
    MarshalledBuffer_t   *m_pMarshalledStreamDesc; //Marshalled/Async allocated stream Desc when the memory model is not CSPROPERTY_BUFFER_DRIVER
    PCS_STREAM_DESCRIPTOR m_pUnMarshalledStreamDesc; //driver maintains the unmarshalled buffer value to instruct the client where to read data from    
} CS_STREAM_DESCRIPTOR_SHADOW, * PCS_STREAM_DESCRIPTOR_SHADOW;

typedef struct {
    ULONG PhysAdd;
    ULONG VirtAdd;
    UINT Reserved; // Used by driver internally
} CAMMemAlloc;

typedef class CPinDevice 
{
public:
    CPinDevice( );

    ~CPinDevice( );

    BOOL
    InitializeSubDevice(
        PCAMERADEVICE pCamDevice
        );

    DWORD
    StreamInstantiate(
        PCSPROPERTY_STREAMEX_S pCsPropStreamEx,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD                  OutBufLen,
        PDWORD                 pdwBytesTransferred
        );

    DWORD
    PinHandleConnectionRequests(
        PCSPROPERTY pCsProp,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD       OutBufLen,
        PDWORD      pdwBytesTransferred
        );

    DWORD
    PinHandleBufferRequest(
        CSBUFFER_INFO  csBufferInfo,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        DWORD *pdwBytesTransferred
        );

    DWORD
    PinHandleCustomRequests(
        __in_bcount(InBufLen) PUCHAR pInBuf,
        DWORD  InBufLen,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred
        );

    DWORD 
    PinHandlePowerRequests(
        DWORD  dwCode,
        PUCHAR pInBuf,
        DWORD  InBufLen,
        PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred
        );

    DWORD
    HandlePinIO( );

    void 
    FlushBufferQueue();

    CSSTATE
    GetState( ) { return m_CsState; };
    
    void
    SetState(
        CSSTATE   CsState,
        CSSTATE * CsPrevState
        );


    DWORD
    PauseStream( );

    DWORD
    CloseSubDevice( );

    ULONG
    PictureNumber( ) const;

    ULONG
    FramesDropped( ) const;

    ULONG
    FrameSize( ) const;

    LONG
    IncrementStillCount()
    {   return InterlockedIncrement(&m_lStillCount);   };

    CS_DATARANGE_VIDEO m_CsDataRangeVideo;    

private:

    DWORD
    AllocateBuffer( 
        PCS_STREAM_DESCRIPTOR pCsDescriptor,
        LPVOID pOutBuf,
        DWORD  OutBufLen,
        DWORD *pdwBytesTransferred
        );

    DWORD
    DeallocateBuffer(
        PCS_STREAM_DESCRIPTOR pCsDescriptor
        );

    DWORD
    EnqueueDescriptor(
        PCS_STREAM_DESCRIPTOR pCsDescriptor
        );

    BOOL
    RemoveBufferFromList(
        PCS_STREAM_DESCRIPTOR * ppCsStreamDesc,
        PVOID                 * ppMappedData,
        PVOID                 * ppUnmappedData
        );

    BOOL
    RemoveBufferFromList(
        PUCHAR                  pBuffer,
        PCS_STREAM_DESCRIPTOR * ppCsStreamDesc,
        PVOID                 * ppMappedData,
        PVOID                 * ppUnmappedData
        );

    BOOL CheckBufferAvailable();

    BOOL
    ResetBufferList( );

    BOOL
    ResetMarshal();
    
    DWORD
    PinHandleConnStateSubReqs(
        ULONG  ulReqFlags,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred
        );

    DWORD
    PinHandleConnDataFormatSubReqs(
        ULONG                          ulReqFlags,
        PCS_DATAFORMAT_VIDEOINFOHEADER pCsDataFormatVidInfoHdr,
        PDWORD                         pdwBytesTransferred
        );

    BOOL InitMsgQueueDescriptor(
        PCS_MSGQUEUE_BUFFER pCsMsgQBuff, 
        PCS_STREAM_DESCRIPTOR pCsStreamDesc, 
        PVOID pMappedData, 
        PVOID pUnmappedData, 
        BOOL bFillBuffer);

    LONG
    GetIndexFromHandle( 
        DWORD  dwHandle,
        LPVOID pBuffer
        );

    DWORD 
    CreateHandle( 
        DWORD  dwIndex, 
        LPVOID pBuffer 
        );

    DWORD
    SwSetupStreamDescriptor(
        DWORD                 dwIndex,
        PCS_STREAM_DESCRIPTOR pCsStreamDesc,
        LPVOID                pBuffer
        );

    DWORD
    HwSetupStreamDescriptor(
        DWORD dwIndex
        );

    BOOL ReadMemoryModelFromRegistry();

    LONG DecrementStillCount()
    {   return InterlockedDecrement(&m_lStillCount);   }; 

    ULONG              m_ulPinId;
    // Get MAX buffer information from pdd ULONG              m_ulMaxNumOfBuffers;
    ULONG              m_ulFrameSize;
    ULONG              m_ulFramesDropped;
    ULONG              m_ulPictureNumber;
    LONG               m_lStillCount; //Count of still capture requests.

    DWORD              m_dwMemoryModel;
    DWORD              m_dwBufferCount;
    DWORD              m_ulMaxNumOfBuffers;
    DWORD              m_msStart;
    DWORD              m_msLastPT;
    
    REFERENCE_TIME     m_RtAveTimePerFrame;

    HANDLE             m_hMsgQ;

    CRITICAL_SECTION   m_csStreamBuffer;
    CRITICAL_SECTION   m_csStreamIO;

    PCAMERADEVICE      m_pCamAdapter;
    PCS_STREAM_DESCRIPTOR_SHADOW m_pStreamDescriptorList;

    CSSTATE            m_CsState;
    BOOL               m_fClientInitialized;
    BOOL               m_fDiscontinuity;
} PINDEVICE, * PPINDEVICE;

typedef struct CPinInitHandle
{
    PCAMERADEVICE pCamDevice;
} PININITHANDLE, * PPININITHANDLE;

#ifdef __cplusplus
}
#endif

#endif //__PINDRIVER_H
