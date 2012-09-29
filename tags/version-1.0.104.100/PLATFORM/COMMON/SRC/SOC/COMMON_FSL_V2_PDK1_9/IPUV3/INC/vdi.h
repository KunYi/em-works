//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  vdi.h
//
//  Video De-Interlacer definitions
//
//-----------------------------------------------------------------------------

#ifndef __VDI_H__
#define __VDI_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

// Post-processing EOF Event 
#define VDI_EOF_EVENT_NAME               L"VDI EOF Event"

// IOCTL to configure VDI
#define VDI_IOCTL_CONFIGURE              1
// IOCTL to start VDI tasks
#define VDI_IOCTL_START                  2
// IOCTL to start VDI tasks
#define VDI_IOCTL_STOP                   3
// IOCTL to wait for VDI complete its task
#define VDI_IOCTL_WAIT_NOT_BUSY          4

// VDI pixel format values for dwPixelFormat member fo vdiConfigData
#define IPU_VDI_C_VDI_CH_422_FORMAT_422               1
#define IPU_VDI_C_VDI_CH_422_FORMAT_420               0

// VDI motion select value for dwMotionSel member of vdiConfigData
#define IPU_VDI_C_VDI_MOT_SEL_ROM1                    0
#define IPU_VDI_C_VDI_MOT_SEL_ROM2                    1
#define IPU_VDI_C_VDI_MOT_SEL_FULL_MOTION             2

// VDI top field select values for dwTopField member of vdiConfigData
#define IPU_VDI_C_VDI_TOP_FIELD_FIELD0                0
#define IPU_VDI_C_VDI_TOP_FIELD_FIELD1                1

//------------------------------------------------------------------------------
// Types

typedef enum {
    VDI_INPUT_SOURCE_MEM,
    VDI_INPUT_SOURCE_CSI,
} VDI_INPUT_SOURCE;

typedef enum {
    VDI_OUTPUT_DEST_MEM,
    VDI_OUTPUT_DEST_IC,
} VDI_OUTPUT_DEST;

// PF buffer configuration structure
typedef struct vdiBufferStruct
{
    UINT32               *yBufPtr;
    UINT32                uOffset;     // Optional starting relative offset of U data
                                       // from beginning of buffer.  Set to 0 to use default
                                       // calculated based on height and stride.
    UINT32                vOffset;     // Optional starting relative offset of V data
                                       // from beginning of buffer.  Set to 0 to use default
                                       // calculated based on height and stride.
    DWORD                 dwStride;
    DWORD                 dwField;
} vdiBuffer, *pVdiBuffer;

// VDI IDMAC Channel configuration structure
typedef struct vdiConfigDataStruct
{
    UINT16                iHeight;
    UINT16                iWidth;
    UINT32                iStride;
    UINT32                Uoffset;
    UINT32                Voffset;
    DWORD                 dwPixelFormat;

    VDI_INPUT_SOURCE      inputSrc;
    VDI_OUTPUT_DEST       outputDst;
    DWORD                 dwTopField;
    DWORD                 dwMotionSel;
} vdiConfigData, *pVdiConfigData;

// Parameters to start VDI task
typedef struct vdiStartParamsStruct
{
    vdiBuffer            prevField;
    vdiBuffer            curField;
    vdiBuffer            nextField;
    vdiBuffer            outputFrame;
    BOOL                  bAutomaticStop;
} vdiStartParams, *pVdiStartParams;


//------------------------------------------------------------------------------
// Functions

HANDLE VDIOpenHandle(void);
BOOL VDICloseHandle(HANDLE hVDI);
BOOL VDIConfigure(HANDLE hVDI, pVdiConfigData pConfigData);
BOOL VDIStart(HANDLE hVDI, pVdiStartParams pStartParams);
BOOL VDIStop(HANDLE hVDI);
BOOL VDIWaitForNotBusy(HANDLE hVDI, UINT32 timeout);

// Debug helper function
void VDIDumpRegs();

#ifdef __cplusplus
}
#endif

#endif //__VDI_H__

