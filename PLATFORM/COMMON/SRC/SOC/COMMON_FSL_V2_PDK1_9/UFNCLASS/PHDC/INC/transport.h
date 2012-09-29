//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        TRANSPORT.H

Abstract:

        USB Personal Health Care Function Common Transport Layer Definitions.
        
--*/

#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

#include <windows.h>
#include "usbfntypes.h"
#include "usbphdfndbg.h"
#include "transporttypes.h"
#include "user_config_phdc.h"
#include "usb_descriptor.h"


#define DEFAULT_TRANSFER_THREAD_PRIORITY        100

#if USB_INTERRUPT_EP_SUPPORTED
    #if USB_METADATA_SUPPORTED
        #define CB_CONFIG_DESCRIPTOR                    0x48
    #else
        #define CB_CONFIG_DESCRIPTOR                    0x3d
    #endif
#else
    #if USB_METADATA_SUPPORTED
        #define CB_CONFIG_DESCRIPTOR                    0x3d
    #else
        #define CB_CONFIG_DESCRIPTOR                    0x32
    #endif
#endif

// Typedefs

typedef struct _USB_PIPE_STATE {   
    BOOL    fSendingLess;
} USB_PIPE_STATE, * PUSB_PIPE_STATE;


#include <pshpack1.h>
#include <poppack.h>

#define EP0_PACKET_SIZE                     0x40
#define HIGH_SPEED_BULK_PACKET_SIZES        0x200
#if 1  /*FOR USBCV CHAP9 TEST*/
#define HIGH_SPEED_ISO_PACKET_SIZES         0x40     // maximum allowable packet size is 0x40
#else
#define HIGH_SPEED_ISO_PACKET_SIZES         0x200
#endif
#define FULL_SPEED_BULK_PACKET_SIZES        0x40
#define FULL_SPEED_ISO_PACKET_SIZES         0x8
#define USB_VERSION                         0x200

#define BULK_OUT_ENDPOINT_ADDRESS  0x01
#define BULK_IN_ENDPOINT_ADDRESS 0x82
#define INTERRUPT_IN_ENDPOINT_ADDRESS 0x83

#define MAX_PHD_RX_LENGTH 512
#define PHD_BUFF_SIZE     114

extern USB_DEVICE_DESCRIPTOR g_HighSpeedDeviceDesc;
extern USB_DEVICE_DESCRIPTOR g_FullSpeedDeviceDesc;
extern UCHAR g_EndpointsExtend_BulkIn[];
extern UCHAR g_EndpointsExtend_BulkOut[];
extern UCHAR g_EndpointsExtend_InterruptIn[];
extern UFN_ENDPOINT g_HighSpeedEndpoints[];
extern UFN_ENDPOINT g_FullSpeedEndpoints[];
extern UCHAR g_InterfaceExtended[];
extern UFN_INTERFACE g_HighSpeedInterface;
extern UFN_CONFIGURATION g_HighSpeedConfig;
extern UFN_CONFIGURATION g_FullSpeedConfig;

#endif // __TRANSPORT_H__
