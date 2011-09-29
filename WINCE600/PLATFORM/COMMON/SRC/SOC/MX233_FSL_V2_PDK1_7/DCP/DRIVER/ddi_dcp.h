//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: ddi_dcp.h
//  Brief data co-processor interface
//
//
/////////////////////////////////////////////////////////////////////////////////

#if !defined( DDI_DCP_H )
#define DDI_DCP_H 1

#include "../hw/hw_dcp.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

typedef void (*pHandlerFunction_t)(void*);

typedef struct _CSCBuffer_t
{
    void        *pRGBBuffer;
    void        *pYBuffer;
    void        *pUBuffer;
    void        *pVBuffer;
    UINT32    Width;
    UINT32    Height;
    UINT32      Stride;
    CSCFormat_t Format;
} CSCBuffer_t;

typedef void (*DCPCallback_t)(void *);

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

////////////////////////////////////////////////////////////////////////////////
// prototypes
////////////////////////////////////////////////////////////////////////////////

// DCP
HRESULT dcp_Initialize(void);
HRESULT dcp_ExecutePackets(DCPHandle_t Handle, DCPWorkPacket_t *Packets, UINT32 NumberOfPackets);
HRESULT dcp_AcquireChannel(DCPHandle_t *Handle, UINT32 DesiredChannel, BOOL LockIt);
HRESULT dcp_ReleaseLockedChannel(DCPHandle_t Handle);
HRESULT dcp_SetCallbackInfo(DCPHandle_t Handle, DCPCallback_t Function, void *PrivateData);
HRESULT dcp_GetChannelNumber(DCPHandle_t Handle, UINT32 *Channel);
HRESULT dcp_memcopyAsync(void *Source, void *Destination, UINT32 Length, DCPCallback_t pCallback, void *PrivateData, DCPHandle_t *Handle);
HRESULT dcp_memcopy(void *Source, void *Destination, UINT32 Length);
HRESULT dcp_WaitForComplete(DCPHandle_t Handle, UINT32 TimeOut);
HRESULT dcp_LockChannel(DCPHandle_t *Handle, UINT32 DesiredChannel);
HRESULT dcp_GetChannelWorkPacket(DCPHandle_t Handle, DCPWorkPacket_t **Packet);
HRESULT dcp_blt(void *Source, void *Destination, UINT32 Width, UINT32 Height, UINT32 Stride, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle);
HRESULT dcp_bltfill(UINT32 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Width, UINT32 Height, UINT32 Stride, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle);
HRESULT dcp_SetChannelPriority(UINT32 Channel, DCPPriority_t Priority);
HRESULT dcp_memfill(UINT8 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Length);
HRESULT dcp_memfillAsync(UINT8 Source, void *DestinationPhy, void *DestinationVrt, UINT32 Length, DCPCallback_t Callback, void *PrivateData, DCPHandle_t *RtHandle);


// Color Space Converter
HRESULT dcp_csc_DoConversion(CSCBuffer_t *pInput, CSCBuffer_t *pOutput, BOOL bRotate, DCPCallback_t pCallbackFunc, void *PrivateData, CSCCoefficients_t *pCoefficients);
HRESULT dcp_csc_SetCallbackInfo(DCPCallback_t CallbackFunction,  void *PrivateData);
HRESULT dcp_csc_WaitForComplete(UINT32 TimeOut);
HRESULT dcp_csc_SetPriority(DCPPriority_t Priority);
HRESULT dcp_csc_SetCoefficients(CSCCoefficients_t *Coefficients);

BOOL       dcp_csc_Available(void);

// VMI
HRESULT ddi_dcp_CopyD( UINT32 uChannel, UINT32 uSrc1PhysAddr, UINT32 uDst1PhysAddr, UINT32 uSrc2PhysAddr, UINT32 uDst2PhysAddr);
HRESULT ddi_dcp_CopyS( UINT32 uChannel, UINT32 uSrc1PhysAddr, UINT32 uDst1PhysAddr );
HRESULT ddi_dcp_ReleaseVMIChannel(DCPHandle_t Handle);
HRESULT ddi_dcp_GetVMIChannel(DCPHandle_t *Handle);
HRESULT ddi_dcp_Init(UINT32 uInitChannel, pHandlerFunction_t pHandlerFunction, UINT32 uDefaultXferByteSize);
HRESULT ddi_dcp_SetupSDRAMLESSVMIChannel(void);

#ifdef __cplusplus
} //extern "C"
#endif

#endif

