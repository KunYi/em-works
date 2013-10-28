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
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: prp.h
//
// Definitions for Pre-processor of eMMA module
//
//------------------------------------------------------------------------------
#ifndef __PRP_H__
#define __PRP_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
//
// Pre-processing EOF Event
#define PRP_VF_EOF_EVENT_NAME           L"Prp Viewfinding EOF Event"
#define PRP_ENC_EOF_EVENT_NAME          L"Prp Encoding EOF Event"

// IOCTL IDs
#define PRP_IOCTL_CONFIGURE             1
#define PRP_IOCTL_START                 2
#define PRP_IOCTL_STOP                  3
#define PRP_IOCTL_ADD_BUFFERS           4
#define PRP_IOCTL_GET_FRAME_COUNT       5

//------------------------------------------------------------------------------
// Types
//
// Pre-processing input format
// PRP_CNTL[DATA_IN_MODE] and PRP_SRC_PIXEL_FORMAT_CNTL
typedef enum {
    prpInputFormat_YUV420,          // 0x0, not supported when CSIEN = 1
    prpInputFormat_YUYV422,         // 0x1
    prpInputFormat_YVYU422,         // 0x1
    prpInputFormat_UYVY422,         // 0x1
    prpInputFormat_VYUY422,         // 0x1
    prpInputFormat_RGB16,           // 0x2
    prpInputFormat_RGB32,           // 0x3
    prpInputFormat_YUV444           // 0x3
} prpInputFormat;

// Viewfinding channel (CH1) output format
// PRP_CNTL[CH1_OUT_MODE] and PRP_CH1_PIXEL_FORMAT_CNTL
typedef enum {
    prpVfOutputFormat_RGB8,         // 0x0, RGB332
    prpVfOutputFormat_RGB16,        // 0x1, RGB565
    prpVfOutputFormat_RGB32,        // 0x2, unpacked RGB888
    prpVfOutputFormat_YUYV422,      // 0x3
    prpVfOutputFormat_YVYU422,      // 0x3
    prpVfOutputFormat_UYVY422,      // 0x3
    prpVfOutputFormat_VYUY422,      // 0x3
    prpVfOutputFormat_Disabled
} prpOutputVfFormat;

// Encoding channel (CH2) output format
// PRP_CNTL[CH2_OUT_MODE]
typedef enum {
    prpEncOutputFormat_YUV420,      // 0x0 or 0x1, IYUV YV12
    prpEncOutputFormat_YUV422,      // 0x1, YUYV
    prpEncOutputFormat_YUV444,      // 0x2, YUV0
    prpEncOutputFormat_Disabled
} prpOutputEncFormat;

// Pre-processing frame skip
// PRP_CNTL[IN_TSKIP] / PRP_CNTL[CH1_TSKIP] / PRP_CNTL[CH2_TSKIP]
typedef enum {
    prpSkip_NoSkip,                 // No skip
    prpSkip_1of2,                   // Skip 1 out of every 2 (1-0)
    prpSkip_1of3,                   // Skip 1 out of every 3 (1-0-1)
    prpSkip_2of3,                   // Skip 2 out of every 3 (1-0-0)
    prpSkip_1of4,                   // Skip 1 out of every 4 (1-1-1-0)
    prpSkip_3of4,                   // Skip 3 out of every 4 (1-0-0-0)
    prpSkip_2of5,                   // Skip 2 out of every 5 (1-0-1-0-1)
    prpSkip_4of5                    // Skip 4 out of every 5 (1-0-0-0-0)
} prpInputFrameSkip;

// Pre-processing CSC equation
typedef enum {
    prpCSCR2Y_A1,                   // RGB to YUV equation A.1
    prpCSCR2Y_A0,                   // RGB to YUV equation A.0
    prpCSCR2Y_B1,                   // RGB to YUV equation B.1
    prpCSCR2Y_B0,                   // RGB to YUV equation A.1
    prpCSCY2R_A1,                   // YUV to RGB equation A.1
    prpCSCY2R_A0,                   // YUV to RGB equation A.0
    prpCSCY2R_B1,                   // YUV to RGB equation B.1
    prpCSCY2R_B0                    // YUV to RGB equation A.1
} prpCSCEquation;

// Pre-processing channel selection
typedef enum {
    prpChannel_Viewfinding,
    prpChannel_Encoding
} prpChannel;

// Pre-processing frame size
typedef struct {
    UINT16 width;
    UINT16 height;
} prpFrameSize, *pPrpFrameSize;

// This structure is used to add input and output
// buffers to the pre-processor
typedef struct {
    PVOID InBuf;
    PVOID OutVfBuf;
    PVOID OutEncBuf;
} prpBuffers, *pPrpBuffers;

// Pre-processor configuration data
typedef struct {
    //---------------------------------------------------------------
    // Skip
    prpInputFrameSkip   CSIInputSkip;           // If line buffer overflow is encountered,
                                                // user can use this configure member to
                                                // slow down the input speed to PrP.
    prpInputFrameSkip   VfOutputSkip;
    prpInputFrameSkip   EncOutputSkip;

    //---------------------------------------------------------------
    // Input
    prpInputFormat      inputFormat;            // YUV / RGB format
    prpFrameSize        inputSize;              // Frame size

    BOOL                bWindowing;             // Enable frame input windowing
    UINT16              CSILineSkip;            // If windowing is enabled, this value specifies the number
                                                // of line to skip from start of the frame.
    UINT16              inputStride;            // If windowing is enabled, this value sets number of PIXELS
                                                // to skip from start of a line. Otherwise, it's line
                                                // stride for the input in BYTES.

    //---------------------------------------------------------------
    // Output
    prpOutputVfFormat   outputVfFormat;         // Viewfinding channel (CH1) format
    prpFrameSize        outputVfSize;           // Frame size
    UINT16              outputVfStride;         // Output line stride in bytes


    prpOutputEncFormat  outputEncFormat;        // Encoding channel (CH2) format
    prpFrameSize        outputEncSize;          // Output size

    //---------------------------------------------------------------
    // CSC
    prpCSCEquation      CSCEquation;            // Selects R2Y or Y2R CSC Equation

} prpConfigData, *pPrpConfigData;


//------------------------------------------------------------------------------
// Functions
HANDLE PRPOpenHandle(void);
BOOL PRPCloseHandle(HANDLE);
BOOL PRPConfigure(HANDLE hPrP, pPrpConfigData pConfigData);
BOOL PRPStart(HANDLE hPrP, prpChannel channel);
BOOL PRPStop(HANDLE hPrP, prpChannel channel);
BOOL PRPAddBuffers(HANDLE hPrP, pPrpBuffers pBufs);
BOOL PRPGetFrameCount(HANDLE hPrP, prpChannel channel);

#ifdef __cplusplus
}
#endif

#endif // __PRP_H__

