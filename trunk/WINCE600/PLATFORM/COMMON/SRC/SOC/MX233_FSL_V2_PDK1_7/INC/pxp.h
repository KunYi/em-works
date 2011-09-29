//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pxp.h
//
//  Public definitions for Pixel Pineline Driver
//
//------------------------------------------------------------------------------
#ifndef __PXP_H__
#define __PXP_H__

//#ifdef __cplusplus
//extern "C" {
//#endif

//------------------------------------------------------------------------------
// Defines
#define PXP_INTERRUPT             1
#define PXP_INT_EVENT_NAME L"PXP INT Event"

// IOCTL to enable PXP
#define PXP_IOCTL_ENABLE                        1
// IOCTL to disable PXP
#define PXP_IOCTL_DISABLE                       2
// IOCTL to configure PXP
#define PXP_IOCTL_CONFIG_GENERAL                3
// IOCTL to call PXP to start process
#define PXP_IOCTL_START_PROCESS                 4
// IOCTL to set output buffer property for the PXP
#define PXP_IOCTL_SET_OUTPUT_PROPERTY           5
// IOCTL to set output buffer1 address for the PXP
#define PXP_IOCTL_SET_OUTPUTBUFFER1_ADDRESS     6
// IOCTL to set S0 buffer property for the PXP
#define PXP_IOCTL_SET_S0BUFFER_PROPERTY         7
// IOCTL to set S0 buffer address for the PXP
#define PXP_IOCTL_SET_S0BUFFER_ADDRESS          8
// IOCTL to set S0 buffer offset location within the output frame buffer for the PXP
#define PXP_IOCTL_SET_S0BUFFER_OFFSETINOUTPUT   9
// IOCTL to set overlay buffers property for the PXP
#define PXP_IOCTL_SET_OVERLAYBUFFERS_PROPERTY   10
// IOCTL to set overlay buffers address for the PXP
#define PXP_IOCTL_SET_OVERLAYBUFFERS_ADDRESS    11
// IOCTL to set overlay buffers position for the PXP
#define PXP_IOCTL_SET_OVERLAYBUFFERS_POSITION   12
// IOCTL to enable PXP interrupt
#define PXP_IOCTL_ENABLE_INTERRUPT              13  
// IOCTL to wait for PXP complete its task
#define PXP_IOCTL_WAIT_COMPLETION               14
// IOCTL to disable PXP interrupt
#define PXP_IOCTL_DISABLE_INTERRUPT             15
// IOCTL to set output buffer2 address for the PXP
#define PXP_IOCTL_SET_OUTPUTBUFFER2_ADDRESS     16
// IOCTL to set S0 buffer color key for the PXP
#define PXP_IOCTL_SET_S0BUFFER_COLORKEY         17
// IOCTL to set S0 buffer size
#define PXP_IOCTL_SET_S0BUFFER_SIZE             18
// IOCTL to set overlay color key for the PXP
#define PXP_IOCTL_SET_OVERLAY_COLORKEY          19
// IOCTL to reset PXP driver status
#define PXP_IOCTL_RESET_DRIVER_STATUS           20

//------------------------------------------------------------------------------
// Types
//PXP interlace output format
typedef enum pxpInterlaceOutputEnum
{  
    pxpInterlaceOutput_PROGRESSIVE = 0,
    pxpInterlaceOutput_FIELD0,
    pxpInterlaceOutput_FIELD1,
    pxpInterlaceOutput_INTERLACE
} pxpInterlaceOutput;

//PXP interlace input format
typedef enum pxpInterlaceInputEnum
{  
    pxpInterlaceInput_PROGRESSIVE = 0,
    pxpInterlaceInput_FIELD0,
    pxpInterlaceInput_FIELD1
} pxpInterlaceInput;

//PXP S0 buffer format
typedef enum pxpS0BufferFormatEnum
{  
    pxpS0BufferFormat_RGB888 = 1,
    pxpS0BufferFormat_RGB565 = 4,
    pxpS0BufferFormat_RGB555 = 5,
    pxpS0BufferFormat_YUV422 = 8,
    pxpS0BufferFormat_YUV420 = 9
} pxpS0BufferFormat;

//PXP output clockwise rotation
typedef enum pxpOutputRotationEnum
{  
    pxpOutputROT_0 = 0,
    pxpOutputROT_90,
    pxpOutputROT_180,
    pxpOutputROT_270
} pxpOutputRotation;

//PXP output RGB format
typedef enum pxpOutputRGBFormatEnum
{  
    pxpOutputFormat_ARGB8888 = 0,
    pxpOutputFormat_RGB888,
    pxpOutputFormat_RGB888P,
    pxpOutputFormat_ARGB1555,
    pxpOutputFormat_RGB565,
    pxpOutputFormat_RGB555
} pxpOutputRGBFormat;

//PXP raster operation
typedef enum pxpROPEnum
{  
    pxpROP_MASKOL = 0,
    pxpROP_MASKNOTOL,
    pxpROP_MASKOLNOT,
    pxpROP_MERGEOL,
    pxpROP_MERGENOTOL,
    pxpROP_MERGEOLNOT,
    pxpROP_NOTCOPYOL,
    pxpROP_NOT,
    pxpROP_NOTMASKOL,
    pxpROP_NOTMERGEOL,
    pxpROP_XOROL,
    pxpROP_NOTXOROL
} pxpROP;

//PXP output RGB format
typedef enum pxpOverlayFormatEnum
{  
    pxpOverlayFormat_ARGB8888 = 0,
    pxpOverlayFormat_RGB888,
    pxpOverlayFormat_ARGB1555 = 3,
    pxpOverlayFormat_RGB565,
    pxpOverlayFormat_RGB555
} pxpOverlayFormat;

//PXP overlay alpha control format
typedef enum pxpAlphaControlEnum
{  
    pxpAlphaControl_EMBEDDED = 0,
    pxpAlphaControl_OVERRIDE,
    pxpAlphaControl_MULTIPLE,
    pxpAlphaControl_ROPS
} pxpAlphaControl;

typedef struct pxpParaConfigStruct
{
    pxpInterlaceOutput  eInterlaceOutput;
    pxpInterlaceInput   eInterlaceInput;
    BOOL                bIn_PLACE;  
} pxpParaConfig, *pPxpParaConfig;

typedef struct pxpOutPropertyStruct
{
    BOOL                bAlphaOutput;       //BP_PXP_CTRL_ALPHA_OUTPUT
    BOOL                bVFlip;
    BOOL                bHFlip;
    pxpOutputRotation   epxpOutputRot;
    pxpOutputRGBFormat  epxpOutputRGB;
    UINT16              iOutputWidth;
    UINT16              iOutputHeight;
    UINT8               iOutputAlpha;             

} pxpOutProperty, *pPxpOutProperty;

typedef struct pxpS0PropertyStruct  
{
    pxpS0BufferFormat   eS0BufferFormat;
    BOOL                bCrop;
    BOOL                bScale;
    UINT32              iS0BKColor;
    RECT                rSOCropRect;
    float               fYScale;
    float               fXScale;
    float               fYScaleOffset;
    float               fXScaleOffset;
    BOOL                bYCbCrCsc;

} pxpS0Property, *pPxpS0Property;

typedef struct pxpColorKeyStruct
{
    UINT32              iColorKeyHigh;
    UINT32              iColorKeyLow;
} pxpColorKey, *pPxpColorKey;

typedef struct pxpS0BufferAddrGroupStruct  
{
    UINT32             iRGBorYBufAddr;
    UINT32             iUorCbBufAddr;
    UINT32             iVorCrBufAddr;

} pxpS0BufferAddrGroup, *pPxpS0BufferAddrGroup;

typedef struct pxpOverlayBuffersPosStruct
{
    RECT                rOverlayBufRect;
    UINT8               iOverlayBufNum;
} pxpOverlayBuffersPos, *pPxpOverlayBuffersPos;

typedef struct pxpOverlayBuffersAddrStruct
{
    UINT32              iOverlayBufAddress;
    UINT8               iOverlayBufNum;
} pxpOverlayBuffersAddr, *pPxpOverlayBuffersAddr;

typedef struct pxpOverlayPropertyStruct
{
    pxpROP               epxpROPType;
    UINT8                iAlphaOverlay;
    pxpOverlayFormat     eOverlayFormat;
    BOOL                 bColorKey;
    pxpAlphaControl      eAlphaCntl;
    BOOL                 bEnableOverlay;
    UINT8                iOverlayBufNum;
} pxpOverlayProperty, *pPxpOverlayProperty;

typedef struct pxpCoordinateStruct
{
    UINT16               iXBase;
    UINT16               iYBase;
} pxpCoordinate, *pPxpCoordinate;

typedef struct pxpRectSizeStruct
{
    UINT16               iWidth;
    UINT16               iHeight;
} pxpRectSize, *pPxpRectSize;

typedef struct {
    UINT32 ol;
    UINT32 olsize;
    UINT32 olparam;
    UINT32 olparam2;
}pxp_overlay_registers;

typedef struct  {
    UINT32 ctrl;
    UINT32 rgbbuf;
    UINT32 rgbbuf2;
    UINT32 rgbsize;
    UINT32 s0buf;
    UINT32 s0ubuf;
    UINT32 s0vbuf;
    UINT32 s0param;
    UINT32 s0background;
    UINT32 s0crop;
    UINT32 s0scale;
    UINT32 s0offset;
    UINT32 s0colorkeylow;
    UINT32 s0colorkeyhigh;
    UINT32 olcolorkeylow;
    UINT32 olcolorkeyhigh;

    pxp_overlay_registers ol[8];

    UINT32 bWait;   //To make sure of 32-bit alignment for NEXT start address

}pxp_registers, *pPxp_registers;


// Functions

HANDLE PXPOpenHandle(void);
BOOL PXPCloseHandle(HANDLE hPXP);
BOOL PXPStartProcess(HANDLE hPXP, BOOL bWaitForCompletion);
BOOL PXPInterruptEnable(HANDLE hPXP);
BOOL PXPInterruptDisable(HANDLE hPXP);
BOOL PXPSetOutputBuffer1Addr(HANDLE hPXP, UINT32 PhysBuf);
BOOL PXPSetOutputBuffer2Addr(HANDLE hPXP, UINT32 PhysBuf);
BOOL PXPSetS0BufferAddrGroup(HANDLE hPXP, pPxpS0BufferAddrGroup pAddrGroup);
BOOL PXPSetS0BufferOffsetInOutput(HANDLE hPXP, pPxpCoordinate pCoordinate);
BOOL PXPSetS0BufferColorKey(HANDLE hPXP, pPxpColorKey pColorKey);
BOOL PXPSetOverlayBuffersAddr(HANDLE hPXP, pPxpOverlayBuffersAddr pBufsAddr);
BOOL PXPWaitForCompletion(HANDLE hPXP);
BOOL PXPSetOverlayBuffersPos(HANDLE hPXP, pPxpOverlayBuffersPos pBufsPos);
BOOL PXPConfigureGeneral(HANDLE hPXP, pPxpParaConfig pParaConfig);
BOOL PXPSetS0BufProperty(HANDLE hPXP, pPxpS0Property pS0Property);
BOOL PXPSetS0BufferSize(HANDLE hPXP, pPxpRectSize pRectSize);
BOOL PXPSetOverlayBufsProperty(HANDLE hPXP, pPxpOverlayProperty pOverlayProperty);
BOOL PXPSetOutputProperty(HANDLE hPXP, pPxpOutProperty pOutProperty);
BOOL PXPSetOverlayColorKey(HANDLE hPXP, pPxpColorKey pColorKey);
BOOL PXPResetDriverStatus(HANDLE hPXP);
//#ifdef __cplusplus
//}
//#endif

#endif   // __PXP_H__
