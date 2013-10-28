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
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pp.h
//
//  Public definitions for Pre-Processor Driver
//
//------------------------------------------------------------------------------
#ifndef __PP_H__
#define __PP_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

// Post-processing EOF Event
#define PP_EOF_EVENT_NAME               L"Pp EOF Event"
//Named PP event for PP driver load indication.
#define PP_LOAD_EVENT_NAME                         TEXT("PP_DRIVER_LOAD_EVENT")
//PP driver load timeout value, arbitrary value.
#define PP_DRIVER_LOAD_TIMEOUT                     120000


// IOCTL to configure PP
#define PP_IOCTL_CONFIGURE              1
// IOCTL to start PP tasks
#define PP_IOCTL_START                  2
// IOCTL to stop PP tasks
#define PP_IOCTL_STOP                   3
// IOCTL to add input and output buffers for the PP
#define PP_IOCTL_ENQUEUE_BUFFERS        4
// IOCTL to clear all buffers for the PP
#define PP_IOCTL_CLEAR_BUFFERS          5
// IOCTL to get the maximum number of buffers
// that can be queued for the PP
#define PP_IOCTL_GET_MAX_BUFFERS        7
// IOCTL to pause the PP viewfinding display
#define PP_IOCTL_PAUSE_VIEWFINDING      8
// IOCTL to retrieve the PP frame count
#define PP_IOCTL_GET_FRAME_COUNT        9

// Based on the number of EDDGPE formats
// defined in ddgpe.h
#define MAX_EDDGPE_FORMATS              19
//------------------------------------------------------------------------------
// Types


// Post-Processing Format
typedef enum ppFormatEnum
{
    ppFormat_YUV444 = 1,
    ppFormat_YUV422,
    ppFormat_YUV420,
    ppFormat_RGB,
    ppFormat_RGBA,
    ppFormat_YUV444IL,// YUV444 interleaved
    ppFormat_YUYV422, // YUV422 interleaved patterns
    ppFormat_YVYU422,
    ppFormat_UYVY422,
    ppFormat_VYUY422,
    ppFormat_Generic,
    ppFormat_Disabled,
    ppFormat_Undefined
} ppFormat;

//Post-Processing Data Width for RGB format
typedef enum ppDataWidthEnum
{
    ppDataWidth_32BPP = 0,
    ppDataWidth_24BPP,
    ppDataWidth_16BPP,
    ppDataWidth_8BPP,
    ppDataWidth_4BPP,
    ppDataWidth_Undefined
} ppDataWidth;


//Post-Processing CSC equation
typedef enum ppCSCEquationEnum
{
    ppCSCR2Y_A1,  //RGB to YUV equation A.1
    ppCSCR2Y_A0,  //RGB to YUV equation A.0
    ppCSCR2Y_B1,  //RGB to YUV equation B.1
    ppCSCR2Y_B0,  //RGB to YUV equation A.1
    ppCSCY2R_A1,  //YUV to RGB equation A.1
    ppCSCY2R_A0,  //YUV to RGB equation A.0
    ppCSCY2R_B1,  //YUV to RGB equation B.1
    ppCSCY2R_B0,  //YUV to RGB equation A.1
    ppCSCNoOp,
    ppCSCCustom
} ppCSCEquation;


// Post-Processing RGB or YUV Format Structure
// For RGB, component0 = red, component1 = green, component2 = blue,
// component3 = alpha
// For YUV, component0 = Y, component1 = U, component2 = V, component3 = NA
typedef struct ppPixelFormatStruct {
    UINT8 component0_width;
    UINT8 component1_width;
    UINT8 component2_width;
    UINT8 component3_width;
    UINT8 component0_offset;
    UINT8 component1_offset;
    UINT8 component2_offset;
    UINT8 component3_offset;
} ppPixelFormat, *pPpPixelFormat;


// Post-Processing CSC equation coeffprpients
// These should be set when the prpCSCCustom CSC equation is selected
typedef struct ppCSCCoeffsStruct
{
    UINT16 C00;
    UINT16 C01;
    UINT16 C02;
    UINT16 C10;
    UINT16 C11;
    UINT16 C12;
    UINT16 C20;
    UINT16 C21;
    UINT16 C22;
    UINT16 A0;
    UINT16 A1;
    UINT16 A2;
    UINT16 Scale;
} ppCSCCoeffs, *pPpCSCCoeffs;

// Parameters for flipping and rotating frames
typedef struct ppFlipRotStruct
{
    BOOL verticalFlip;
    BOOL horizontalFlip;
    BOOL rotate90;
} ppFlipRot, *pPpFlipRot;


//Post-Processing Frame Size Structure
typedef struct ppFrameSizeStruct {
    UINT16 width;
    UINT16 height;
} ppFrameSize, *pPpFrameSize;


//Post-Processing Configuration Data Structure
typedef struct ppConfigDataStruct
{
    //---------------------------------------------------------------
    // General controls
    //---------------------------------------------------------------
    BOOL                  directDisplay;  // If enabled, viewfinding data will be sent to the display.
                                          // Otherwise, viewfinding data is written into memory.
                                          // In the former case, if the platform uses the ADC
                                          // driver, no buffers are required for viewfinding,
                                          // as the data is sent directly to the display without
                                          // being written to memory.

    //---------------------------------------------------------------
    // Format controls
    //---------------------------------------------------------------

    // For input
    ppFormat             inputFormat;    // YUV or RGB
    ppFrameSize          inputSize;      // input frame size
    UINT32               inputStride;    // input stride in bytes
    ppPixelFormat        inputRGBPixelFormat;  // Input frame RGB format, set NULL
                                          // to use standard settings.
    ppDataWidth          inputDataWidth;    // Bits per pixel for RGB format

    // For combining input
    BOOL                 bCombining;
    UINT32               colorKey;
    UINT32               alpha;
    ppFormat             inputCombFormat;      // YUV or RGB
    ppFrameSize          inputCombSize;        // input frame size
    UINT32               inputCombStride;      // Stride for Combining surface
    ppPixelFormat        inputCombRGBPixelFormat;  // Input frame RGB format, set NULL
                                               // to use standard settings.
    ppDataWidth          inputCombDataWidth;       // Bits per pixel for RGB format

    // For output
    ppFormat             outputFormat;   // Output format for Viewfinding channel
    ppFrameSize          outputSize;     // Channel-2 output size
    UINT32               outputStride;   // output stride in bytes
    ppPixelFormat        outputRGBPixelFormat;  // Output frame RGB format, set NULL
                                         // to use standard settings.
    ppDataWidth          outputDataWidth;    // Bits per pixel for RGB format

    POINT                offset;         // If windowing and direct display enabled,
                                         // this point specifies an offset for
                                         // displaying the VF image
    ppCSCEquation        CSCEquation;    // Selects R2Y or Y2R CSC Equation
    ppCSCCoeffs          CSCCoeffs;      // Selects R2Y or Y2R CSC Equation
    ppFlipRot            flipRot;        // Flip/Rotate controls for VF
    BOOL                 bDelayFlip;    // Flag for delay flip
}ppConfigData, *pPpConfigData;

// This structure is used to add input and output
// buffers to the post-processing queue.
typedef struct ppBuffersStruct
{
    // Info for main input buffer
    UINT32 *inputBuf;
    UINT32 inBufLen;
    UINT32 inputUBufOffset; // offset of U buffer from Y buffer start address
                            // ignored if non-planar image format
    UINT32 inputVBufOffset; // offset of U buffer from Y buffer start address
                            // ignored if non-planar image format

    // Info for combining input buffer (only needed when combining is done
    UINT32 *inputCombBuf; // Pointer for
    UINT32 inCombBufLen;
    UINT32 inputCombUBufOffset; // offset of U buffer from Y buffer start address for combining surface
                            // ignored if non-planar image format
    UINT32 inputCombVBufOffset; // offset of U buffer from Y buffer start address for combining
                            // ignored if non-planar image format

    // Info for output buffer
    UINT32 *outputBuf;
    UINT32 outBufLen;
    UINT32 outputUBufOffset;
    UINT32 outputVBufOffset;
} ppBuffers, *pPpBuffers;

/*
const ppFormat EDDGPEFormatToPpFormat[] = { ppFormat_Undefined, //ddgpePixelFormat_1bpp
                                        ppFormat_Undefined,     //ddgpePixelFormat_2bpp
                                        ppFormat_RGB,           //ddgpePixelFormat_4bpp
                                        ppFormat_RGB,           //ddgpePixelFormat_8bpp
                                        ppFormat_RGB,           //ddgpePixelFormat_565
                                        ppFormat_Undefined,     //ddgpePixelFormat_5551
                                        ppFormat_Undefined,     //ddgpePixelFormat_4444
                                        ppFormat_Undefined,     //ddgpePixelFormat_5550
                                        ppFormat_RGB,           //ddgpePixelFormat_8880
                                        ppFormat_RGB,          //ddgpePixelFormat_8888
                                        ppFormat_YUYV422,       //ddgpePixelFormat_YUYV422
                                        ppFormat_UYVY422,       //ddgpePixelFormat_UYVY422
                                        ppFormat_Undefined,     //ddgpePixelFormat_YUY2422
                                        ppFormat_Generic,       //ddgpePixelFormat_15bppGeneric
                                        ppFormat_Generic,       //ddgpePixelFormat_16bppGeneric
                                        ppFormat_Generic,       //ddgpePixelFormat_24bppGeneric
                                        ppFormat_Generic,       //ddgpePixelFormat_32bppGeneric
                                        ppFormat_Undefined,     //ddgpePixelFormat_UnknownFormat
                                        ppFormat_YUV420};       //ddgpePixelFormat_CustomFormat
const ppDataWidth EDDGPEFormatToDataWidth[] = { ppDataWidth_Undefined, //ddgpePixelFormat_1bpp
                                        ppDataWidth_Undefined,         //ddgpePixelFormat_2bpp
                                        ppDataWidth_4BPP,              //ddgpePixelFormat_4bpp
                                        ppDataWidth_8BPP,              //ddgpePixelFormat_8bpp
                                        ppDataWidth_16BPP,             //ddgpePixelFormat_565
                                        ppDataWidth_16BPP,             //ddgpePixelFormat_5551
                                        ppDataWidth_16BPP,             //ddgpePixelFormat_4444
                                        ppDataWidth_16BPP,             //ddgpePixelFormat_5550
                                        ppDataWidth_24BPP,             //ddgpePixelFormat_8880
                                        ppDataWidth_32BPP,             //ddgpePixelFormat_8888
                                        ppDataWidth_16BPP,             //ddgpePixelFormat_YUYV422
                                        ppDataWidth_16BPP,             //ddgpePixelFormat_UYVY422
                                        ppDataWidth_16BPP,             //ddgpePixelFormat_YUY2422
                                        ppDataWidth_16BPP,             //ddgpePixelFormat_15bppGeneric
                                        ppDataWidth_16BPP,             //ddgpePixelFormat_16bppGeneric
                                        ppDataWidth_24BPP,             //ddgpePixelFormat_24bppGeneric
                                        ppDataWidth_32BPP,             //ddgpePixelFormat_32bppGeneric
                                        ppDataWidth_Undefined,         //ddgpePixelFormat_UnknownFormat
                                        ppDataWidth_Undefined };       //ddgpePixelFormat_CustomFormat
const UINT32 PpDataWidthToBPP[] = { 32, 24, 16, 8, 4, 0 };
*/
//------------------------------------------------------------------------------
// Functions

HANDLE PPOpenHandle(void);
BOOL PPCloseHandle(HANDLE);
void PPConfigure(HANDLE hPP, pPpConfigData pConfigData);
void PPStart(HANDLE hPP);
void PPStop(HANDLE hPP);
BOOL PPAddBuffers(HANDLE hPP, pPpBuffers pBufs);
BOOL PPClearBuffers(HANDLE hPP);
UINT32 PPGetMaxBuffers(HANDLE hPP);
BOOL PPPauseDisplay(HANDLE hPP);
BOOL PPGetFrameCount(HANDLE hPP);

#ifdef __cplusplus
}
#endif

#endif   // __PP_H__
