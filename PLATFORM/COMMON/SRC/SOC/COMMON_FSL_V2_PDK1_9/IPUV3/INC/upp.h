//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  upp.h
//
//  Definitions for User-Pre-Processor Driver
//
//------------------------------------------------------------------------------
#ifndef __UPP_H__
#define __UPP_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

// IOCTL to configure PP
#define UPP_IOCTL_CONFIGURE              1
// IOCTL to start PP tasks
#define UPP_IOCTL_START                  2
// IOCTL to stop PP tasks
#define UPP_IOCTL_STOP                   3
// IOCTL to add input buffers for the PP
#define UPP_IOCTL_ADD_INPUT_BUFFER       4
// IOCTL to add output buffers for the PP
#define UPP_IOCTL_ADD_OUTPUT_BUFFER      5
// IOCTL to clear all buffers for the PP
#define UPP_IOCTL_CLEAR_BUFFERS          6
// IOCTL to enable pp interrupt
#define UPP_IOCTL_ENABLE_INTERRUPT       7
// IOCTL to wait for pp complete its task
#define UPP_IOCTL_WAIT_NOT_BUSY          8
// IOCTL to disable pp interrupt
#define UPP_IOCTL_DISABLE_INTERRUPT      9
// IOCTL to add second input buffers for the PP
#define UPP_IOCTL_ADD_INPUT_COMBBUFFER   10
// IOCTL to set active window position when mask channel is enabled
#define UPP_IOCTL_SET_WINDOW_POS         11
// IOCTL to allocate buffer
#define UPP_IOCTL_ALLOC_BUFFER           12
// IOCTL to Deallocate buffer
#define UPP_IOCTL_DEALLOC_BUFFER         13
// IOCTL to add applcation own input buffers for the PP
// This buffer maybe not safty, application need to garantee it safety
#define UPP_IOCTL_ADD_APP_INPUT_BUFFER   14
// IOCTL to add applcation own output buffers for the PP
// This buffer maybe not safty, application need to garantee it safety
#define UPP_IOCTL_ADD_APP_OUTPUT_BUFFER  15
// IOCTL to add second applcation own input buffers for the PP
// This buffer maybe not safty, application need to garantee it safety
#define UPP_IOCTL_ADD_APP_INPUT_COMBBUFFER   16


//------------------------------------------------------------------------------
// Types
typedef struct {
        ULONG PhysAdd;
        ULONG VirtAdd;
        ULONG Reserved; // Used by driver internally
        ULONG Size;
} AllocMemInf;


#ifdef __cplusplus
}
#endif

#endif   // __UPP_H__
