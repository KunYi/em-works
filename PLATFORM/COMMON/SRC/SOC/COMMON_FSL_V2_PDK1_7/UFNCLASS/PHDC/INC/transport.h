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
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
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

#define CB_CONFIG_DESCRIPTOR                    0x44

// Typedefs

typedef struct _USB_PIPE_STATE {   
    BOOL    fSendingLess;
} USB_PIPE_STATE, * PUSB_PIPE_STATE;


#include <pshpack1.h>
#include <poppack.h>

#define EP0_PACKET_SIZE                     0x40
#define HIGH_SPEED_BULK_PACKET_SIZES        0x200
#define HIGH_SPEED_ISO_PACKET_SIZES         0x200
#define FULL_SPEED_BULK_PACKET_SIZES        0x40
#define FULL_SPEED_ISO_PACKET_SIZES         0x8
#define USB_VERSION                         0x200


// USB function object
static UFN_FUNCTIONS                        g_ufnFuncs;
static PCUFN_FUNCTIONS                      g_pUfnFuncs = &g_ufnFuncs;
static UFN_HANDLE                           g_hDevice = NULL;

// USB function state
static BOOL                                 g_fDeviceRegistered = FALSE;
static BOOL                                 g_fDeviceConfigured = FALSE;

// mass storage function state
// static MSC_STATE                            g_MscState = MSC_STATE_IDLE;

// BOT transport data
#define MAX_PHD_RX_LENGTH 512
static BYTE                                 g_rgbPhdRxBuffer[MAX_PHD_RX_LENGTH];
#define PHD_BUFF_SIZE     114
static BYTE                                 g_phd_buffer[PHD_BUFF_SIZE];
static BOOL                                 g_phd_buffer_being_used = FALSE;
static BOOL                                 g_sent_resp_get_attr = FALSE;

// default (control) pipe state data
static UFN_PIPE                             g_hDefaultPipe;
static USB_PIPE_STATE                       g_psDefaultPipeState;

// bulk out pipe state data
static UFN_PIPE                             g_hBOPipe;
static USB_PIPE_STATE                       g_psBOPipeState;

// bulk in pipe state data
static UFN_PIPE                             g_hBIPipe;
static USB_PIPE_STATE                       g_psBIPipeState;

// interrupt in pipe state data
static UFN_PIPE                             g_hIIPipe;
static USB_PIPE_STATE                       g_psIIPipeState;

// scratch data
// static BYTE                                 g_bScratch = 0;

// static BOOL                                 g_fStoreOpened = FALSE;

// Bus Speed  - Used for selecting between Descriptors
static UFN_BUS_SPEED                        g_SpeedSupported = BS_HIGH_SPEED;

static UFN_CLIENT_REG_INFO                  g_RegInfo = { sizeof(UFN_CLIENT_REG_INFO) };


// #define PID_MICROSOFT_MASS_STORAGE_PROTOTYPE    0xFFFF

static USB_DEVICE_DESCRIPTOR                g_HighSpeedDeviceDesc = {
    sizeof(USB_DEVICE_DESCRIPTOR),          // bLength
    USB_DEVICE_DESCRIPTOR_TYPE,             // bDescriptorType
    USB_VERSION,                            // bcdUSB
    0x00,                                   // bDeviceClass
    0x00,                                   // bDeviceSubClass
    0x00,                                   // bDeviceProtocol
    EP0_PACKET_SIZE,                        // bMaxPacketSize0
    0,                                      // idVendor
    0,                                      // idProduct
    0x0000,                                 // bcdDevice
    0x01,                                   // iManufacturer
    0x02,                                   // iProduct
    0x03,                                   // iSerialNumber
    0x01                                    // bNumConfigurations
};

static USB_DEVICE_DESCRIPTOR                g_FullSpeedDeviceDesc = {
    sizeof(USB_DEVICE_DESCRIPTOR),          // bLength
    USB_DEVICE_DESCRIPTOR_TYPE,             // bDescriptorType
    USB_VERSION,                            // bcdUSB
    0x00,                                   // bDeviceClass
    0x00,                                   // bDeviceSubClass
    0x00,                                   // bDeviceProtocol
    EP0_PACKET_SIZE,                        // bMaxPacketSize0
    0,                                      // idVendor
    0,                                      // idProduct
    0x0000,                                 // bcdDevice
    0x01,                                   // iManufacturer
    0x02,                                   // iProduct
    0x03,                                   // iSerialNumber
    0x01                                    // bNumConfigurations
};

#define BULK_OUT_ENDPOINT_ADDRESS  0x01
#define BULK_IN_ENDPOINT_ADDRESS 0x82
#define INTERRUPT_IN_ENDPOINT_ADDRESS 0x83

UCHAR g_EndpointsExtend_BulkIn[] = {
// Qos
    USB_QOS_DESC_SIZE, // 0x4,
    USB_PHDC_QOS_DESCRIPTOR, // 0x21,
    0x01,
    PHDC_BULK_IN_QOS, // 0x88,

// meta data
    USB_METADATA_BULK_IN_DESC_SIZE, // 0x7,
    PHDC_METADATA_DESCRIPTOR, // 0x22,

    0xAB,   // opaque meta data
    0xCD,
    0xEF,
    0x01,
    0x02
};

UCHAR g_EndpointsExtend_BulkOut[] = {
    USB_METADATA_BULK_OUT_DESC_SIZE, // 0x4,
    PHDC_METADATA_DESCRIPTOR, // 0x22,
    0xCC,
    0xDD
};

UCHAR g_EndpointsExtend_InterruptIn[] = {
    USB_QOS_DESC_SIZE, // 0x4,
    USB_PHDC_QOS_DESCRIPTOR, // 0x21,
    0x01,
    PHDC_INT_IN_QOS  //0x88
};

static UFN_ENDPOINT                         g_HighSpeedEndpoints[] = {
    {
        sizeof(UFN_ENDPOINT),
        {
            sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
            USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
            BULK_IN_ENDPOINT_ADDRESS,      // bEndpointAddress (endpoint 2, out)
            USB_ENDPOINT_TYPE_BULK,         // bmAttributes
            HIGH_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
            0x00                            // bInterval (interrupt only)
        },
        &g_EndpointsExtend_BulkIn,
        sizeof(g_EndpointsExtend_BulkIn)
    },
    {
        sizeof(UFN_ENDPOINT),
        {
            sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
            USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
            BULK_OUT_ENDPOINT_ADDRESS,       // bEndpointAddress (endpoint 1, bulk in)
            USB_ENDPOINT_TYPE_BULK,         // bmAttributes
            HIGH_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
            0x00                            // bInterval (interrupt only)
        },
        &g_EndpointsExtend_BulkOut,
        sizeof(g_EndpointsExtend_BulkOut)
    },
    {
        sizeof(UFN_ENDPOINT),
        {
            sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
            USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
            INTERRUPT_IN_ENDPOINT_ADDRESS,  // bEndpointAddress (endpoint 3, interrupt in)
            USB_ENDPOINT_TYPE_INTERRUPT,    // bmAttributes
            HIGH_SPEED_ISO_PACKET_SIZES,    // wMaxPacketSize
            0xF0                            // bInterval (interrupt only)
        },
        &g_EndpointsExtend_InterruptIn,
        sizeof(g_EndpointsExtend_InterruptIn)
    }
};

static UFN_ENDPOINT                         g_FullSpeedEndpoints[] = {
    {
        sizeof(UFN_ENDPOINT),
        {
            sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
            USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
            BULK_IN_ENDPOINT_ADDRESS,       // bEndpointAddress (endpoint 1, in)
            USB_ENDPOINT_TYPE_BULK,         // bmAttributes
            FULL_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
            0x00                            // bInterval (interrupt only)
        },
        &g_EndpointsExtend_BulkIn,
        sizeof(g_EndpointsExtend_BulkIn)
    },
    {
        sizeof(UFN_ENDPOINT),
        {
            sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
            USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
            BULK_OUT_ENDPOINT_ADDRESS,      // bEndpointAddress (endpoint 2, out)
            USB_ENDPOINT_TYPE_BULK,         // bmAttributes
            FULL_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
            0x00                            // bInterval (interrupt only)
        },
        &g_EndpointsExtend_BulkOut,
        sizeof(g_EndpointsExtend_BulkOut)
    },
    {
        sizeof(UFN_ENDPOINT),
        {
            sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
            USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
            INTERRUPT_IN_ENDPOINT_ADDRESS,  // bEndpointAddress (endpoint 3, interrupt in)
            USB_ENDPOINT_TYPE_INTERRUPT,    // bmAttributes
            FULL_SPEED_ISO_PACKET_SIZES,    // wMaxPacketSize
            0xF0                            // bInterval (interrupt only)
        },
        &g_EndpointsExtend_InterruptIn,
        sizeof(g_EndpointsExtend_InterruptIn)
    }
};

static UCHAR g_InterfaceExtended[] = {
    /* PHDC class function descriptor */
    USB_PHDC_CLASSFUNCTION_DESC_SIZE,  // 0x04,
    USB_PHDC_CLASSFUNCTION_DESCRIPTOR, // 0x20,
    0x02,                              // data and messaging formats not defined by vendor DATA FORMAT -- PHDC_11073_20601
    META_DATA_MSG_PRE_IMPLEMENTED,     // 0x01,

    /*PHDC function extension descriptor */
    USB_PHDC_FUNCTION_EXT_DESC_SIZE,   // 0x06,
    PHDC_11073PHD_FUNCTION_DESCRIPTOR, // 0x30,
    0x00,
    0x02,
    0x34,
    0x12
};

static UFN_INTERFACE                        g_HighSpeedInterface = {
    sizeof(UFN_INTERFACE),
    {
        sizeof(USB_INTERFACE_DESCRIPTOR),   // bLength
        USB_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
        0x00,                               // bInterfaceNumber    
        0x00,                               // bAlternateSetting

        dim(g_HighSpeedEndpoints),          // bNumEndpoints
        PHDC_INTERFACE_CLASS,               // bInterfaceClass
        PHDC_INTERFACE_SUBCLASS,            // bInterfaceSubClass
        PHDC_INTERFACE_PROTOCOL,            // bInterfaceProtocol
        0x00                                // iInterface    
    },
    &g_InterfaceExtended,                   // extended
    sizeof(g_InterfaceExtended),            // extended size
    g_HighSpeedEndpoints                    // endpoint array
};

static UFN_INTERFACE                        g_FullSpeedInterface = {
    sizeof(UFN_INTERFACE),
    {
        sizeof(USB_INTERFACE_DESCRIPTOR),   // bLength
        USB_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
        0x00,                               // bInterfaceNumber    
        0x00,                               // bAlternateSetting

        dim(g_FullSpeedEndpoints),          // bNumEndpoints
        PHDC_INTERFACE_CLASS,               // bInterfaceClass
        PHDC_INTERFACE_SUBCLASS,            // bInterfaceSubClass
        PHDC_INTERFACE_PROTOCOL,            // bInterfaceProtocol
        0x00                                // iInterface    
    },
    &g_InterfaceExtended,                   // extended
    sizeof(g_InterfaceExtended),            // extended size
    g_FullSpeedEndpoints                    // endpoint array
};

static UFN_CONFIGURATION                    g_HighSpeedConfig = {
    sizeof(UFN_CONFIGURATION),
    {
        sizeof(USB_CONFIGURATION_DESCRIPTOR),// bLength
        USB_CONFIGURATION_DESCRIPTOR_TYPE,  // bDescriptorType
        
        CB_CONFIG_DESCRIPTOR,               // wTotalLength
        0x01,                               // bNumInterfaces
        0x01,                               // bConfigurationValue
        0x00,                               // iConfiguration
        0xe0,                               // VBus current draw config
        0x32
    },
    NULL,
    0,
    &g_HighSpeedInterface                   // interface array
};

static UFN_CONFIGURATION                    g_FullSpeedConfig = {
    sizeof(UFN_CONFIGURATION),
    {
        sizeof(USB_CONFIGURATION_DESCRIPTOR),// bLength
        USB_CONFIGURATION_DESCRIPTOR_TYPE,  // bDescriptorType
        
        CB_CONFIG_DESCRIPTOR,               // wTotalLength
        0x01,                               // bNumInterfaces
        0x01,                               // bConfigurationValue
        0x00,                               // iConfiguration
        0xe0,                               // VBus current draw config
        0x32
    },
    NULL,
    0,
    &g_FullSpeedInterface                   // interface array
};

#endif // __TRANSPORT_H__
