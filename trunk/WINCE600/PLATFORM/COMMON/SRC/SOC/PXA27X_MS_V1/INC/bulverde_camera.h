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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------

/**************************************************************************
** Copyright 2000-2003 Intel Corporation. All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.

Module Name:  

    bulverde_camera.h

Abstract:  

**************************************************************************/

#ifndef __BULVERDE_CAMERA_H
#define __BULVERDE_CAMERA_H

#if __cplusplus
    extern "C" 
    {
#endif

#include "xllp_gpio.h"
#include "xllp_clkmgr.h"
#include "xllp_camera.h"


#define VIDEO_CAPTURE_BUFFER 0x1
#define STILL_CAPTURE_BUFFER 0x2

#define CAMERA_EVENT_NAME               _T("CameraFrameComplete")
#define CAMERA_EVENT_STILL_IMAGE_READY  _T("CameraStillImageReady")
#define CAMERA_SHUTDOWN_COMPLETE        _T("CameraShutdownComplete")

typedef struct
{
        DWORD VirtAddr;         // Address of the 4K aligned DMA buffer, derived from pBuf.
        int   size;             // size of the buffer
        int   BufferID;         // a buffer ID used to identify this buffer to the driver
        DWORD *pY;
        DWORD *pCb;
        DWORD *pCr;
        DWORD *pBuf;            // Address of the DMA buffer returned from a call to malloc().
} CAMERA_DMA_BUFFER_INFO, *P_CAMERA_DMA_BUFFER_INFO;

typedef void (*PFNCAMHANDLEFRAME)( DWORD dwContext );

extern DWORD dwCameraDriverContext;

extern volatile XLLP_GPIO_T   * v_pGPIORegs;
extern volatile XLLP_CLKMGR_T * v_pClkRegs;
extern volatile unsigned int  * v_pOSTRegs;
extern volatile unsigned int  * v_pCIRegs;
extern volatile unsigned int  * v_pI2C;


// Function prototypes
extern int  CameraInit(void *DriverIndex);
extern int  CameraDeinit(void);

extern int  CameraInitSensor(void);
extern int  SensorInitPlatform(P_XLLP_Camera_Context_T pCameraContext,
                               P_XLLP_Camera_DMA_Context_T pDmaContext);
extern int  SensorDeinitPlatform(P_XLLP_Camera_Context_T pCameraContext,
                                 P_XLLP_Camera_DMA_Context_T pDmaContext);

extern int  CameraSleep(void);
extern int  CameraResume(void);

extern int  CameraChangeCaptureFormat(void);
extern void CameraStartVideoCapture(void);
extern void CameraStopVideoCapture(void);
extern void CameraCaptureStillImage(void);
extern void CameraReadRegister(unsigned char subAddress, unsigned char *bufP);
extern void CameraWriteRegister(unsigned char subAddress, unsigned char *bufP);
extern int  CameraShutdown(void);

extern int  CameraPrepareBuffer(P_CAMERA_DMA_BUFFER_INFO pBufInfo, int BufferType);
extern void CameraSubmitBuffer(P_CAMERA_DMA_BUFFER_INFO pBufInfo, int BufferType);
extern void CameraUnprepareBuffer(P_CAMERA_DMA_BUFFER_INFO pCamDmaBuf, int ucBufferType);


extern PFNCAMHANDLEFRAME pfnCameraHandleVideoFrame;
extern PFNCAMHANDLEFRAME pfnCameraHandleStillFrame;

#if __cplusplus
    }
#endif

#endif 
