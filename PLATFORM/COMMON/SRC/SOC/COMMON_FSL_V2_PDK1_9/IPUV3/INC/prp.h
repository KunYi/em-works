//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  prp.h
//
//  Public definitions for Pre-Processor Driver
//
//------------------------------------------------------------------------------
#ifndef __PRP_H__
#define __PRP_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define FRAME_INTERRUPT             1
#define FIRSTMODULE_INTERRUPT       2
#define MAX_EDDGPE_FORMATS          14

#define PRP_MIN_INPUT_WIDTH     8
#define PRP_MIN_INPUT_HEIGHT    2
#define PRP_MAX_INPUT_WIDTH     4096
#define PRP_MAX_INPUT_HEIGHT    4096
#define PRP_MIN_OUTPUT_WIDTH    8
#define PRP_MIN_OUTPUT_HEIGHT   1
#define PRP_MAX_OUTPUT_WIDTH    1024
#define PRP_MAX_OUTPUT_HEIGHT   1024

// Maximum downscaling resize ratio
#define PRP_MAX_DOWNSIZE_RATIO    8  


//------------------------------------------------------------------------------
// Types


// Image Converter Format
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

//Image Converter Data Width for RGB format
typedef enum icDataWidthEnum
{
    icDataWidth_32BPP = 0,
    icDataWidth_24BPP,
    icDataWidth_16BPP,
    icDataWidth_8BPP,
    icDataWidth_4BPP,
    icDataWidth_Undefined
} icDataWidth;

//ic Buffer status
typedef enum icBufferStatusEnum
{
    BufferIdle = 0,
    OneBufferBusy,
    TwoBufferBusy
} icBufferStatus;


typedef enum icCSCMatrixEnum
{
    icCSC1,  //csc1 matrix
    icCSC2,  //csc2 matrix
} icCSCMatrix;

// Image Converter RGB or YUV Format Structure
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


//Image Converter Frame Size Structure
typedef struct icFrameSizeStruct {
    UINT16 width;
    UINT16 height;
} icFrameSize, *pIcFrameSize;

typedef struct idmaChannelStruct
{
    icFormat            FrameFormat;   // YUV or RGB
    icFrameSize         FrameSize;     //  frame size
    UINT32              LineStride;    //  stride in bytes
    icPixelFormat       PixelFormat;   // Input frame RGB format, set NULL 
                                       // to use standard settings.
    icDataWidth         DataWidth;     // Bits per pixel for RGB format
    UINT32              UBufOffset;    // offset of U buffer from Y buffer start address
                                       // ignored if non-planar image format
    UINT32              VBufOffset;    // offset of U buffer from Y buffer start address
                                       // ignored if non-planar image format    
} idmaChannel, *pIdmaChannel;

//Image Converter  Configuration Data Structure
typedef struct prpConfigDataStruct
{
    // For input
    idmaChannel         inputIDMAChannel;
    BOOL                bInputIsInterlaced;    //TRUE:   interlaced mode (for interlaced video)
                                               //FALSE:progressive mode(normal video)
                                          
    BOOL                bCombineEnable;
    idmaChannel         inputcombVFIDMAChannel;
    UINT8               inputcombAlpha;
    UINT32              inputcombColorkey;

    // For output
    BOOL                bVFEnable;
    idmaChannel         outputVFIDMAChannel;
    CSCEQUATION         VFCSCEquation;      // Selects R2Y or Y2R CSC Equation
    dpCSCCoeffs         VFCSCCoeffs;      
    icFlipRot           VFFlipRot;          // Flip/Rotate controls for VF
    BOOL                bVFIsSyncFlow;      //TRUE:   synchronous flow  dp sync channel
                                            //FALSE:asynchronous flow dp async0 channel
    BOOL                bVFIsInterlaced;    //TRUE:   interlaced mode (already for tv)
                                            //FALSE:progressive mode
                                            
    BOOL                bENCEnable;
    idmaChannel         outputENCIDMAChannel;
    CSCEQUATION         ENCCSCEquation;     // Selects R2Y or Y2R CSC Equation
    dpCSCCoeffs         ENCCSCCoeffs;      
    icFlipRot           ENCFlipRot;         // Flip/Rotate controls for ENC
    BOOL                bENCIsSyncFlow;     //TRUE:   synchronous flow  dp sync channel
                                            //FALSE:asynchronous flow dp async0 channel   
    BOOL                bENCIsInterlaced;   //TRUE:   interlaced mode (already for tv)
                                            //FALSE:progressive mode    
    BOOL                bNoDirectDP;        //TRUE: break the pipline between prp and DP.
}*pPrpConfigData, prpConfigData;

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
                                        icDataWidth_8BPP };            //ddgpePixelFormat_CustomFormat
const UINT16 IcDataWidthToBPP[] = { 32, 24, 16, 8, 4, 0 };
const UINT16 IcBitPerPixelToBPP[] = { 32, 24, 18, 16, 12, 8, 4};

//------------------------------------------------------------------------------
// Functions

HANDLE PrPOpenHandle(void);
BOOL PrPCloseHandle(HANDLE);
BOOL PrPConfigure(HANDLE pPrP, pPrpConfigData pConfigData);
void PrPStart(HANDLE);
void PrPStop(HANDLE);
BOOL PrPAddInputBuffer(HANDLE hDeviceContext, UINT32 PhysicalBuf, BOOL bWait, UINT8 IntType);
BOOL PrPAddInputCombBuffer(HANDLE hDeviceContext, UINT32 PhysicalBuf);
BOOL PrPAddVFOutputBuffer(HANDLE hDeviceContext,UINT32 PhysicalBuf);
BOOL PrPClearBuffers(HANDLE);
BOOL PrPIsBusy(HANDLE);
BOOL PrPBufferStatus(HANDLE, icBufferStatus *);

#ifdef __cplusplus
}
#endif

#endif   // __PRP_H__
