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
//------------------------------------------------------------------------------
//
// Copyright (C) 2004,  MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: pp.h
//
// Definitions for Post-Processor of eMMA module
//
//------------------------------------------------------------------------------
#ifndef __PP_H__
#define __PP_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
//
// Post-processing EOF Event
#define PP_EOF_EVENT_NAME                   L"Pp EOF Event"

// IOCTL IDs
#define PP_IOCTL_CONFIGURE                  1
#define PP_IOCTL_START                      2
#define PP_IOCTL_STOP                       3
#define PP_IOCTL_ENQUEUE_BUFFERS            4
#define PP_IOCTL_CLEAR_BUFFERS              5
#define PP_IOCTL_GET_MAX_BUFFERS            6
#define PP_IOCTL_GET_FRAME_COUNT            7

//------------------------------------------------------------------------------
// Types
//
// Post-processing CSC equation
typedef enum {
    ppCSCEquationA_1 = 0,
    ppCSCEquationA_0 = 1,
    ppCSCEquationB_1 = 2,
    ppCSCEquationB_0 = 3
} ppCSCEquation;

// Post-processing CSC output format
typedef enum {
    ppCSCOutputFormat_RGB16     = 2,
    ppCSCOutputFormat_RGB32     = 3,
    ppCSCOutputFormat_YUV422    = 4
} ppCSCOutputFormat;

// Post-processing interrupt type
typedef enum {
    ppIntrType_NoInterrupt      = 0,
    ppIntrType_FrameComplete    = 1,
    ppIntrType_MBComplete       = 2,
    ppIntrType_Error            = 4
} ppIntrType;

// Post-processing frame size
typedef struct {
    UINT16 width;
    UINT16 height;
} ppFrameSize, *pPpFrameSize;

// Post-processing pixel format
// - RGB
//   component0 = red, component1 = green,
//   component2 = blue, component3 = alpha
// - YUV
//   component0 = Y, component1 = U,
//   component2 = V, component3 = NA
typedef struct {
    UINT8 component0_width;
    UINT8 component1_width;
    UINT8 component2_width;
    UINT8 component3_width;
    UINT8 component0_offset;
    UINT8 component1_offset;
    UINT8 component2_offset;
    UINT8 component3_offset;
} ppPixelFormat, *pPpPixelFormat;

// This structure is used to add input and output
// buffers to the post-processing queue.
typedef struct {
    PVOID InYBuf;
    PVOID InUBuf;
    PVOID InVBuf;
    PVOID InQPBuf;
    PVOID OutBuf;
} ppBuffers, *pPpBuffers;

// Post-processing configueration
typedef struct {
    BOOL                bDeblock;
    BOOL                bDering;
    // Input
    ppFrameSize         inputSize;
    UINT16              inputStride;    // In pixels
    // Output
    ppFrameSize         outputSize;
    UINT16              outputStride;   // In bytes
    ppCSCOutputFormat   outputFormat;
    ppPixelFormat       outputPixelFormat;
    ppCSCEquation       CSCEquation;
} ppConfigData, *pPpConfigData;

//------------------------------------------------------------------------------
// Functions
HANDLE PPOpenHandle(void);
BOOL PPCloseHandle(HANDLE);
BOOL PPConfigure(HANDLE hPP, pPpConfigData pConfigData);
BOOL PPStart(HANDLE hPP);
BOOL PPStop(HANDLE hPP);
BOOL PPAddBuffers(HANDLE hPP, pPpBuffers pBufs);
BOOL PPClearBuffers(HANDLE hPP);
UINT32 PPGetMaxBuffers(HANDLE hPP);
BOOL PPGetFrameCount(HANDLE hPP);

#ifdef __cplusplus
}
#endif

#endif // __PP_H__

