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
/****************************************************************************** 
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
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
********************************************************************************/
#ifndef _XLLP_CAMERA_H_
#define _XLLP_CAMERA_H_

#include "xllp_defs.h"
#include "xllp_dmac.h"

//////////////////////////////////////////////////////////////////////////////////////
//
//          Macros
//
//////////////////////////////////////////////////////////////////////////////////////
//
//  Sensor type
//
#define XLLP_CAMERA_TYPE_ADCM_2650               01
#define XLLP_CAMERA_TYPE_ADCM_2670               02
#define XLLP_CAMERA_TYPE_ADCM_2700               03
#define XLLP_CAMERA_TYPE_OMNIVISION_9640         04
#define XLLP_CAMERA_TYPE_UNKNOWN                 05
#define XLLP_CAMERA_TYPE_MAX                     XLLP_CAMERA_TYPE_UNKNOWN

//
//  Image format definition
//
#define XLLP_CAMERA_IMAGE_FORMAT_RAW8                0
#define XLLP_CAMERA_IMAGE_FORMAT_RAW9                1
#define XLLP_CAMERA_IMAGE_FORMAT_RAW10               2
                                                     
#define XLLP_CAMERA_IMAGE_FORMAT_RGB444              3
#define XLLP_CAMERA_IMAGE_FORMAT_RGB555              4
#define XLLP_CAMERA_IMAGE_FORMAT_RGB565              5
#define XLLP_CAMERA_IMAGE_FORMAT_RGB666_PACKED       6
#define XLLP_CAMERA_IMAGE_FORMAT_RGB666_PLANAR       7
#define XLLP_CAMERA_IMAGE_FORMAT_RGB888_PACKED       8
#define XLLP_CAMERA_IMAGE_FORMAT_RGB888_PLANAR       9
#define XLLP_CAMERA_IMAGE_FORMAT_RGBT555_0          10  //RGB+Transparent bit 0
#define XLLP_CAMERA_IMAGE_FORMAT_RGBT888_0          11
#define XLLP_CAMERA_IMAGE_FORMAT_RGBT555_1          12  //RGB+Transparent bit 1  
#define XLLP_CAMERA_IMAGE_FORMAT_RGBT888_1          13

#define XLLP_CAMERA_IMAGE_FORMAT_YCBCR400           14
#define XLLP_CAMERA_IMAGE_FORMAT_YCBCR422_PACKED    15
#define XLLP_CAMERA_IMAGE_FORMAT_YCBCR422_PLANAR    16
#define XLLP_CAMERA_IMAGE_FORMAT_YCBCR444_PACKED    17
#define XLLP_CAMERA_IMAGE_FORMAT_YCBCR444_PLANAR    18
#define XLLP_CAMERA_IMAGE_FORMAT_MAX                XLLP_CAMERA_IMAGE_FORMAT_YCBCR444_PLANAR

//
//  Image Size definition
//
#define XLLP_CAMERA_IMAGE_SIZE_QQVGA                0x01
#define XLLP_CAMERA_IMAGE_SIZE_QVGA                 0x02
#define XLLP_CAMERA_IMAGE_SIZE_VGA                  0x04
#define XLLP_CAMERA_IMAGE_SIZE_SXGA                 0x08
#define XLLP_CAMERA_IMAGE_SIZE_QQCIF                0x10
#define XLLP_CAMERA_IMAGE_SIZE_QCIF                 0x20
#define XLLP_CAMERA_IMAGE_SIZE_CIF                  0x40
#define XLLP_CAMERA_IMAGE_SIZE_INVALID              0x80


// Interrupt mask
#define XLLP_CAMERA_INTMASK_FIFO_OVERRUN            0x0001
#define XLLP_CAMERA_INTMASK_END_OF_FRAME            0x0002  
#define XLLP_CAMERA_INTMASK_START_OF_FRAME          0x0004
#define XLLP_CAMERA_INTMASK_CI_DISABLE_DONE         0x0008
#define XLLP_CAMERA_INTMASK_CI_QUICK_DISABLE        0x0010
#define XLLP_CAMERA_INTMASK_PARITY_ERROR            0x0020
#define XLLP_CAMERA_INTMASK_END_OF_LINE             0x0040
#define XLLP_CAMERA_INTMASK_FIFO_EMPTY              0x0080
#define XLLP_CAMERA_INTMASK_RCV_DATA_AVALIBLE       0x0100
#define XLLP_CAMERA_INTMASK_TIME_OUT                0x0200
#define XLLP_CAMERA_INTMASK_END_OF_DMA              0x0400

// Interrupt status
#define XLLP_CAMERA_INTSTATUS_FIFO_OVERRUN_0        0x00000001
#define XLLP_CAMERA_INTSTATUS_FIFO_OVERRUN_1        0x00000002
#define XLLP_CAMERA_INTSTATUS_FIFO_OVERRUN_2        0x00000004
#define XLLP_CAMERA_INTSTATUS_END_OF_FRAME          0x00000008  
#define XLLP_CAMERA_INTSTATUS_START_OF_FRAME        0x00000010
#define XLLP_CAMERA_INTSTATUS_CI_DISABLE_DONE       0x00000020
#define XLLP_CAMERA_INTSTATUS_CI_QUICK_DISABLE      0x00000040
#define XLLP_CAMERA_INTSTATUS_PARITY_ERROR          0x00000080
#define XLLP_CAMERA_INTSTATUS_END_OF_LINE           0x00000100
#define XLLP_CAMERA_INTSTATUS_FIFO_EMPTY_0          0x00000200
#define XLLP_CAMERA_INTSTATUS_FIFO_EMPTY_1          0x00000400
#define XLLP_CAMERA_INTSTATUS_FIFO_EMPTY_2          0x00000800
#define XLLP_CAMERA_INTSTATUS_RCV_DATA_AVALIBLE_0   0x00001000
#define XLLP_CAMERA_INTSTATUS_RCV_DATA_AVALIBLE_1   0x00002000
#define XLLP_CAMERA_INTSTATUS_RCV_DATA_AVALIBLE_2   0x00004000
#define XLLP_CAMERA_INTSTATUS_TIME_OUT              0x00008000
#define XLLP_CAMERA_INTSTATUS_END_OF_DMA            0x00010000

// Capture status
#define XLLP_CAMERA_STATUS_VIDEO_CAPTURE_IN_PROCESS 0x0001
#define XLLP_CAMERA_STATUS_RING_BUFFER_FULL         0x0002

// Capture mode
#define XLLP_CAMERA_MODE_PREVIEW        0x0001
#define XLLP_CAMERA_MODE_STILL          0x0002
#define XLLP_CAMERA_MODE_RECORD         0x0004

// Light
#define XLLP_CAMERA_LIGHT_NORMAL        0x0001
#define XLLP_CAMERA_LIGHT_LOW           0x0002

// Camera sensor flip image mode
#define XLLP_CAMERA_VIDEO_FLIP_VERTICAL     0x0001 // v-mirror
#define XLLP_CAMERA_VIDEO_FLIP_HORIZONTAL   0x0002 // h-mirror
#define XLLP_CAMERA_STILL_FLIP_VERTICAL     0x0004 // v-mirror
#define XLLP_CAMERA_STILL_FLIP_HORIZONTAL   0x0008 // h-mirror

//////////////////////////////////////////////////////////////////////////////////////
//
//          Structures
//
//////////////////////////////////////////////////////////////////////////////////////

typedef struct XLLP_Camera_Context_S XLLP_Camera_Context_T, *P_XLLP_Camera_Context_T;

// Camera functions
typedef struct {

    XLLP_STATUS_T (*init)(P_XLLP_Camera_Context_T);
    XLLP_STATUS_T (*deinit)(P_XLLP_Camera_Context_T);   
    XLLP_STATUS_T (*set_capture_format)(P_XLLP_Camera_Context_T);
    XLLP_STATUS_T (*start_capture)(P_XLLP_Camera_Context_T, unsigned int);
    XLLP_STATUS_T (*stop_capture)(P_XLLP_Camera_Context_T);
    XLLP_STATUS_T (*WriteRegister)(P_XLLP_Camera_Context_T, const XLLP_UINT8_T, XLLP_UINT8_T *);
    XLLP_STATUS_T (*ReadRegister)(P_XLLP_Camera_Context_T, const XLLP_UINT8_T, XLLP_UINT8_T *);
    XLLP_STATUS_T (*SetWhiteBalance)(void);

} XLLP_Camera_Function_T, *P_XLLP_Camera_Function_T;

// Camera context 
typedef struct XLLP_Camera_Context_S {

    //
    // DRIVER FILLED PARAMTER
    //
    // sensor info  
    unsigned int sensor_type;
    
    // capture image info
    unsigned int capture_width; 
    unsigned int capture_height;
    unsigned int Video_capture_width;
    unsigned int Video_capture_height;
    unsigned int Still_capture_width;
    unsigned int Still_capture_height;

    unsigned int Video_capture_input_format;
    unsigned int Video_capture_output_format;
    unsigned int Still_capture_input_format;
    unsigned int Still_capture_output_format;
    
    unsigned int capture_mode;

    unsigned int sensor_flip_mode;
    
    // frame rate control
    unsigned int frame_rate;

    // wait frames
    unsigned int still_wait_frames;
    unsigned int video_wait_frames;
    
    // os mapped register address       
    unsigned int clk_reg_base;
    unsigned int ost_reg_base;
    unsigned int gpio_reg_base;
    unsigned int ci_reg_base;
    unsigned int board_reg_base;

    // function dispatch table
    P_XLLP_Camera_Function_T camera_functions;

    //
    // XLLP FILLED PARAMTER
    //
    unsigned int capture_status;
    
    unsigned int mode;
    unsigned int light;
    unsigned int zoom;
    unsigned int clkrc;
    unsigned int gain;
    unsigned int exposure;
    unsigned int ci_disable_complete;

} XLLP_Camera_Context_T, *P_XLLP_Camera_Context_T;

// Camera DMA context 
typedef struct XLLP_Camera_DMA_Context_S {

    // ring buffers
    // note: must pass in 8 bytes aligned address
    void *buffer_virtual;
    void *buffer_physical;
    unsigned int buf_size;

    // memory for dma descriptors, layout:
    //      dma descriptor chain 0,
    //      dma descriptor chain 1,
    //      ...  
    void *dma_descriptors_virtual;
    void *dma_descriptors_physical;
    unsigned int dma_descriptors_size;
    
    //
    // XLLP INTERNALLY USED: DON'T TOUCH!
    //
    unsigned int block_number, block_size;
    unsigned int block_header, block_tail;
    unsigned int fifo0_descriptors_virtual, fifo0_descriptors_physical;
    unsigned int fifo1_descriptors_virtual, fifo1_descriptors_physical;
    unsigned int fifo2_descriptors_virtual, fifo2_descriptors_physical;
    unsigned int fifo0_transfer_size;
    unsigned int fifo1_transfer_size;
    unsigned int fifo2_transfer_size;

    XLLP_DMAC_CHANNEL_T dma_channels[3];

} XLLP_Camera_DMA_Context_T, *P_XLLP_Camera_DMA_Context_T;


//////////////////////////////////////////////////////////////////////////////////////
//
//          Prototypes
//
//////////////////////////////////////////////////////////////////////////////////////

/***********************************************************************
 *
 * Init/Deinit APIs
 *
 ***********************************************************************/
// Setup the sensor type, configure image capture format
// (RGB, yuv 444, yuv 422, yuv 420, packed | planar, MJPEG) regardless
// of current operating mode (i.e. sets mode for both still capture and video capture)
XLLP_STATUS_T XllpCameraInit( P_XLLP_Camera_Context_T camera_context );

// Power off sensor
XLLP_STATUS_T XllpCameraDeInit( P_XLLP_Camera_Context_T camera_context );


/***********************************************************************
 *
 * Capture APIs
 *
 ***********************************************************************/
// Set the image format
XLLP_STATUS_T XllpCameraSetCaptureFormat( P_XLLP_Camera_Context_T camera_context );


/***********************************************************************
 *
 * Buffer Info APIs
 *
 ***********************************************************************/
// Return: the number of frame buffers allocated for use.
unsigned int XllpCameraGetNumFrameBuffers( P_XLLP_Camera_Context_T camera_context );

// FrameBufferID is a number between 0 and N-1, where N is the total number of frame buffers in use.  Returns the address of
// the given frame buffer.  The application will call this once for each frame buffer at application initialization only.
void* XllpCameraGetFrameBufferAddress( P_XLLP_Camera_Context_T camera_context, unsigned int frame_buffer_id );

// Return the block id
int XllpCameraGetFrameBufferID( P_XLLP_Camera_Context_T camera_context, void* address );

/***********************************************************************
 *
 * Frame rate APIs
 *
 ***********************************************************************/
// Set desired frame rate
void XllpCameraSetCaptureFrameRate( P_XLLP_Camera_Context_T camera_context ); 

// return current setting
void XllpCameraGetCaptureFrameRate( P_XLLP_Camera_Context_T camera_context ); 


/***********************************************************************
 *
 * Interrupt APIs
 *
 ***********************************************************************/
// set interrupt mask 
void XllpCameraSetInterruptMask( P_XLLP_Camera_Context_T camera_context, unsigned int mask ); 

// get interrupt mask 
unsigned int XllpCameraGetInterruptMask( P_XLLP_Camera_Context_T camera_context ); 

// clear interrupt status
void XllpCameraClearInterruptStatus( P_XLLP_Camera_Context_T camera_context, unsigned int status );

#endif
