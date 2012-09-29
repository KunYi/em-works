//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
#define FRAME_INTERRUPT             1
#define FIRSTMODULE_INTERRUPT   2
#define PP_EOF_EVENT_NAME L"Pp EOF Event"
#define PP_EOM_EVENT_NAME L"Pp EOM Event"

// IOCTL to configure PP
#define PP_IOCTL_CONFIGURE              1
// IOCTL to start PP tasks
#define PP_IOCTL_START                  2
// IOCTL to stop PP tasks
#define PP_IOCTL_STOP                   3
// IOCTL to add input buffers for the PP
#define PP_IOCTL_ADD_INPUT_BUFFER       4
// IOCTL to add output buffers for the PP
#define PP_IOCTL_ADD_OUTPUT_BUFFER      5
// IOCTL to clear all buffers for the PP
#define PP_IOCTL_CLEAR_BUFFERS          6
// IOCTL to enable pp interrupt
#define PP_IOCTL_ENABLE_INTERRUPT       7
// IOCTL to wait for pp complete its task
#define PP_IOCTL_WAIT_NOT_BUSY          8
// IOCTL to disable pp interrupt
#define PP_IOCTL_DISABLE_INTERRUPT      9
// IOCTL to add second input buffers for the PP
#define PP_IOCTL_ADD_INPUT_COMBBUFFER   10
// IOCTL to set active window position when mask channel is enabled
#define PP_IOCTL_SET_WINDOW_POS         11


//------------------------------------------------------------------------------
// Types


// Image Convertor Format
typedef enum icFormatEnum
{
    icFormat_YUV444 = 1,
    icFormat_YUV422,
    icFormat_YUV420,
    icFormat_YUV422P,  //YUV422 partial interleaved
    icFormat_YUV420P,  //YUV420 partial interleaved    
    icFormat_RGB,
    icFormat_RGBA,
    icFormat_YUV444IL,// YUV444 interleaved
    icFormat_YUYV422, // YUV422 interleaved patterns
    icFormat_YVYU422,
    icFormat_UYVY422,
    icFormat_VYUY422,
    icFormat_Generic,
    icFormat_Disabled,
    icFormat_Undefined
} icFormat;

//Image Convertor Data Width for RGB format
typedef enum icDataWidthEnum
{
    icDataWidth_32BPP = 0,
    icDataWidth_24BPP,
    icDataWidth_16BPP,
    icDataWidth_8BPP,
    icDataWidth_4BPP,
    icDataWidth_Undefined
} icDataWidth;

//Image Convertor Data Width for RGB format
typedef enum icAlphaTypeEnum
{
    icAlphaType_Global = 0,
    icAlphaType_Local, // Per-pixel alpha
    icAlphaType_SeparateChannel
} icAlphaType;

typedef enum icCSCMatrixEnum
{
    icCSC1,  //csc1 matrix
    icCSC2,  //csc2 matrix
} icCSCMatrix;

// Image Convertor RGB or YUV Format Structure
// For RGB, component0 = red, component1 = green, component2 = blue, 
// component3 = alpha
// For YUV, component0 = Y, component1 = U, component2 = V, component3 = NA
typedef struct icPixelFormatStruct {
    UINT8 component0_width;
    UINT8 component1_width;
    UINT8 component2_width;
    UINT8 component3_width;
    UINT8 component0_offset;
    UINT8 component1_offset;
    UINT8 component2_offset;
    UINT8 component3_offset;
} icPixelFormat, *pIcPixelFormat;

// Parameters for flipping and rotating frames
typedef struct icFlipRotStruct
{
    BOOL verticalFlip;
    BOOL horizontalFlip;
    BOOL rotate90;
} icFlipRot, *pIcFlipRot;


//Image Convertor Frame Size Structure
typedef struct icFrameSizeStruct {
    UINT16 width;
    UINT16 height;
} icFrameSize, *pIcFrameSize;

//Post processor buffer Structure
typedef struct ppBufferStruct {
    UINT32 size;
    UINT32 VAddr;
    UINT32 PAddr;
    HANDLE pBuffer;
} PPBuf, *pPPBuf;


typedef struct idmaChannelStruct
{
    icFormat             FrameFormat;  // YUV or RGB
    icFrameSize         FrameSize;      //  frame size
    UINT32               LineStride;       //  stride in bytes
    icPixelFormat       PixelFormat;    // Input frame RGB format, set NULL 
                                                   // to use standard settings.
    icDataWidth         DataWidth;     // Bits per pixel for RGB format
    UINT32               UBufOffset;     // offset of U buffer from Y buffer start address
                                                  // ignored if non-planar image format
    UINT32               VBufOffset;    // offset of U buffer from Y buffer start address
                                                 // ignored if non-planar image format    
} idmaChannel, *pIdmaChannel;

//Image Convertor  Configuration Data Structure
typedef struct ppConfigDataStruct
{
    //---------------------------------------------------------------
    // General controls
    //---------------------------------------------------------------
    UINT8         IntType;    
                                          //    FIRSTMODULE_INTERRUPT: the interrupt will be 
                                          //    rised once the first sub-module finished its job.
                                          //    FRAME_INTERRUPT:   the interrput will be rised 
                                          //    after all sub-modules finished their jobs.
    //---------------------------------------------------------------
    // Format controls
    //---------------------------------------------------------------

    // For input
    idmaChannel     inputIDMAChannel;

    BOOL            bCombineEnable;
    idmaChannel     inputcombIDMAChannel;
    UINT8           inputcombAlpha;
    UINT32          inputcombColorkey;

    icAlphaType     alphaType;
    
    // For output
    idmaChannel     outputIDMAChannel;
    CSCEQUATION  CSCEquation;    // Selects R2Y or Y2R CSC Equation
    icCSCCoeffs     CSCCoeffs;      // Selects R2Y or Y2R CSC Equation
    icFlipRot          FlipRot;        // Flip/Rotate controls for VF
    BOOL    allowNopPP;   // flag to indicate we need a NOP PP processing

}*pPpConfigData, ppConfigData;

const icFormat EDDGPEFormatToIcFormat[] = { icFormat_Undefined, //ddgpePixelFormat_1bpp
                                        icFormat_Undefined,     //ddgpePixelFormat_2bpp
                                        icFormat_RGB,           //ddgpePixelFormat_4bpp
                                        icFormat_RGB,           //ddgpePixelFormat_8bpp
                                        icFormat_RGB,           //ddgpePixelFormat_565
                                        icFormat_Undefined,     //ddgpePixelFormat_5551
                                        icFormat_Undefined,     //ddgpePixelFormat_4444
                                        icFormat_Undefined,     //ddgpePixelFormat_5550
                                        icFormat_RGB,           //ddgpePixelFormat_8880
                                        icFormat_RGB,          //ddgpePixelFormat_8888
                                        icFormat_YUYV422,       //ddgpePixelFormat_YUYV422
                                        icFormat_UYVY422,       //ddgpePixelFormat_UYVY422
                                        icFormat_YUYV422,     //ddgpePixelFormat_YUY2422
                                        icFormat_YUV420,         //ddgpePixelFormat_YV12 420
                                        icFormat_Generic,       //ddgpePixelFormat_15bppGeneric
                                        icFormat_Generic,       //ddgpePixelFormat_16bppGeneric
                                        icFormat_Generic,       //ddgpePixelFormat_24bppGeneric
                                        icFormat_Generic,       //ddgpePixelFormat_32bppGeneric
                                        icFormat_Undefined,     //ddgpePixelFormat_UnknownFormat
                                        icFormat_YUV420P};       //ddgpePixelFormat_CustomFormat
const icDataWidth EDDGPEFormatToIcDataWidth[] = { icDataWidth_Undefined, //ddgpePixelFormat_1bpp
                                        icDataWidth_Undefined,         //ddgpePixelFormat_2bpp
                                        icDataWidth_4BPP,              //ddgpePixelFormat_4bpp
                                        icDataWidth_8BPP,              //ddgpePixelFormat_8bpp
                                        icDataWidth_16BPP,             //ddgpePixelFormat_565
                                        icDataWidth_16BPP,             //ddgpePixelFormat_5551
                                        icDataWidth_16BPP,             //ddgpePixelFormat_4444
                                        icDataWidth_16BPP,             //ddgpePixelFormat_5550
                                        icDataWidth_24BPP,             //ddgpePixelFormat_8880
                                        icDataWidth_32BPP,             //ddgpePixelFormat_8888
                                        icDataWidth_16BPP,             //ddgpePixelFormat_YUYV422
                                        icDataWidth_16BPP,             //ddgpePixelFormat_UYVY422
                                        icDataWidth_16BPP,             //ddgpePixelFormat_YUY2 422
                                        icDataWidth_8BPP,              //ddgpePixelFormat_YV12 420
                                        icDataWidth_16BPP,             //ddgpePixelFormat_15bppGeneric
                                        icDataWidth_16BPP,             //ddgpePixelFormat_16bppGeneric
                                        icDataWidth_24BPP,             //ddgpePixelFormat_24bppGeneric
                                        icDataWidth_32BPP,             //ddgpePixelFormat_32bppGeneric
                                        icDataWidth_Undefined,         //ddgpePixelFormat_UnknownFormat
                                        icDataWidth_8BPP };       //ddgpePixelFormat_CustomFormat
const UINT16 IcDataWidthToBPP[] = { 32, 24, 16, 8, 4, 0 };
const UINT16 IcBitPerPixelToBPP[] = { 32, 24, 18, 16, 12, 8, 4};

//------------------------------------------------------------------------------
// Functions

HANDLE PPOpenHandle(void);
BOOL PPCloseHandle(HANDLE hPP);
BOOL PPConfigure(HANDLE hPP, pPpConfigData pConfigData);
BOOL PPStart(HANDLE hPP);
BOOL PPStop(HANDLE hPP);
BOOL PPAddInputBuffer(HANDLE hPP, UINT32 PhysBuf);
BOOL PPAddOutputBuffer(HANDLE hPP, UINT32 PhysBuf);
BOOL PPAddInputCombBuffer(HANDLE hPP, UINT32 PhysBuf);
BOOL PPClearBuffers(HANDLE hPP);
BOOL PPInterruptEnable(HANDLE hPP, UINT8 IntType);
BOOL PPWaitForNotBusy(HANDLE hPP, UINT8 IntType);
BOOL PPInterruptDisable(HANDLE hPP, UINT8 IntType);
BOOL PPSetWindowPos(HANDLE hPP, RECT * pPos);


#ifdef __cplusplus
}
#endif

#endif   // __PP_H__
