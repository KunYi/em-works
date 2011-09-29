//-----------------------------------------------------------------------------
// Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:    vpu_io.h
//  Purpose: Defines IOCTL code and structures for VPU API driver.
//
//------------------------------------------------------------------------------

#ifndef __VPU_SDK_H__
#define __VPU_SDK_H__

#if __cplusplus
extern "C" {
#endif

#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>
#pragma warning(pop)

#define VPU_POWER 0
//------------------------------------------------------------------------------
// IOCTL code definition
//------------------------------------------------------------------------------

//Common Codec commands..
#define IOCTL_VPU_IS_BUSY               \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4000, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_VERSION_INFO              \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4001, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_ALLOC_PHY_MEM             \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FREE_PHY_MEM              \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4003, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_PHY_ADDR                 \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4004, METHOD_BUFFERED, FILE_ANY_ACCESS)
 #define IOCTL_RESET_VPU                 \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4026, METHOD_BUFFERED, FILE_ANY_ACCESS)

//VPU Decoder Commands...
#define IOCTL_DEC_OPEN                  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4005, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_DEC_CLOSE                 \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4006, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_DEC_SET_ESC_SEQ_INIT      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4007, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_DEC_GET_INITIAL_INFO      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4008, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_DEC_REGISTER_FRAME_BUF      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4009, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_DEC_GET_BITSTREAM_BUF     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4010, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_DEC_UPDATE_BITSTREAM_BUF  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4011, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_DEC_START_ONE_FRAME         \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4012, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_DEC_GET_OUTPUT_INFO       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4013, METHOD_BUFFERED, FILE_ANY_ACCESS)   
#define IOCTL_DEC_BIT_BUFFER_FLUSH      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4014, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_DEC_CLR_DISPLAY_FLAG      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4015, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_DEC_GIVE_COMMAND          \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4016, METHOD_BUFFERED, FILE_ANY_ACCESS)

//VPU Encoder Commands...
#define IOCTL_ENC_OPEN                  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4017, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_ENC_CLOSE                 \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4018, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_ENC_GET_INITIAL_INFO      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4019, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_ENC_REGISTER_FRAME_BUF      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4020, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_ENC_GET_BITSTREAM_BUF     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4021, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_ENC_UPDATE_BITSTREAM_BUF  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4022, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_ENC_START_ONE_FRAME         \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4023, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_ENC_GET_OUTPUT_INFO       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4024, METHOD_BUFFERED, FILE_ANY_ACCESS)   
#define IOCTL_ENC_GIVE_COMMAND          \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4025, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VPU_GET_CLEAR_FLAG          \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4027, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------
// Wrapped struct definition for IOCTL operation
//------------------------------------------------------------------------------


// for encoder
// Input parameter definition

typedef struct {
    DecHandle    handle;
    int escape;
} DecSetEscSeqInitInput, *PDecSetEscSeqInitInput;

typedef struct {
    DecHandle    handle;
    FrameBuffer *pbufarray;
    int          num;
    int          stride;
    DecBufInfo  *pbufinfo;
} DecRegisterFrameBufferInput, *PDecRegisterFrameBufferInput;

typedef struct {
    DecHandle    handle;
    Uint32    size;
} DecUpdateBitstreamBufferInput, *PDecUpdateBitstreamBufferInput;

typedef struct {
    DecHandle    handle;
    DecParam    *pdecparam;
} DecStartOneFrameInput, *PDecStartOneFrameInput;

typedef struct {
    DecHandle    handle;
    int index;
} DecClrDispFlagInput, *PDecClrDispFlagInput;

typedef struct {
    DecHandle    handle;
    int *piClearFlag;
} DecGetDispFlagInput, *PDecGetDispFlagInput;

typedef struct {
    DecHandle     handle;
    CodecCommand  cmd;
    void      *pparam;
} DecGiveCommandInput, *PDecGiveCommandInput;

typedef struct {
    void    *pvaddress;
    Uint32  size;
} GetPhysAddressInput, *PGetPhysAddressInput;

// Output parameter definition

typedef struct {
    PhysicalAddress phyaddr;
    void        *pvirAddr;
} DecAllocPhysMemOutput, *PDecAllocPhysMemOutput;

typedef struct {
    DecHandle    handle;
    RetCode     retcode;
} DecOpenOutput, *PDecOpenOutput;

typedef struct {
    DecInitialInfo   intialinfo;
    RetCode         retcode;
} DecGetInitialInfoOutput, *PDecGetInitialInfoOutput;

typedef struct {
    PhysicalAddress readptr;
    PhysicalAddress writeptr;
    Uint32        size;
    RetCode        retcode;
} DecGetBitstreamBufferOutput, *PDecGetBitstreamBufferOutput;

typedef struct {
    DecOutputInfo  outputinfo;
    RetCode       retcode;
} DecGetOutputInfoOutput, *PDecGetOutputInfoOutput;

// for encoder
// Input parameter definition

typedef struct {
    EncHandle   handle;
    FrameBuffer *pbufarray;
    int         num;
    int         stride;
} EncRegisterFrameBufferInput, *PEncRegisterFrameBufferInput;

typedef struct {
    EncHandle   handle;
    Uint32      size;
} EncUpdateBitstreamBufferInput, *PEncUpdateBitstreamBufferInput;

typedef struct {
    EncHandle   handle;
    EncParam    *pencparam;
} EncStartOneFrameInput, *PEncStartOneFrameInput;

typedef struct {
    EncHandle       handle;
    CodecCommand    cmd;
    void            *pparam;
} EncGiveCommandInput, *PEncGiveCommandInput;

// Output parameter definition
typedef struct {
    EncHandle   handle;
    RetCode retcode;
} EncOpenOutput, *PEncOpenOutput;

typedef struct {
    EncInitialInfo  intialinfo;
    RetCode         retcode;
} EncGetInitialInfoOutput, *PEncGetInitialInfoOutput;

typedef struct {
    PhysicalAddress readptr;
    PhysicalAddress writeptr;
    Uint32          size;
    RetCode         retcode;
} EncGetBitstreamBufferOutput, *PEncGetBitstreamBufferOutput;

typedef struct {
    EncOutputInfo   outputinfo;
    RetCode         retcode;
} EncGetOutputInfoOutput, *PEncGetOutputInfoOutput;

// Other Miscellaneous API

typedef struct {
    CodecHandle codecHandle;
    int instIndex;
} VpuResetInput, *PVpuResetInput;

#ifdef __cplusplus
}
#endif

#endif //_VPU_SDK_H__
