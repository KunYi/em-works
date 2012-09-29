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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

        USB Mass Storage Function Common Transport Layer Definitions.
        
--*/

#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

#include <windows.h>
#include "usbfntypes.h"
#include "usbmsfndbg.h"
#include "proxy.h"
#include "transporttypes.h"


#define DEFAULT_TRANSFER_THREAD_PRIORITY        100

#define CB_CONFIG_DESCRIPTOR                    0x20


// Typedefs

typedef struct _USB_PIPE_STATE {   
    BOOL    fSendingLess;
} USB_PIPE_STATE, * PUSB_PIPE_STATE;


// Mass storage class states
typedef enum _MSC_STATE {
    MSC_STATE_UNKNOWN = 0,
    MSC_STATE_IDLE,
    MSC_STATE_COMMAND_TRANSPORT,
    MSC_STATE_DATA_IN_TRANSPORT,
    MSC_STATE_DATA_OUT_TRANSPORT,
    MSC_STATE_STATUS_TRANSPORT,
    MSC_STATE_WAIT_FOR_RESET,
#ifdef USBCV_MSC
    MSC_STATE_COMMAND_FAILURE,
#endif
} MSC_STATE, *PMSC_STATE;


#define MAX_CBWCB_SIZE 16

// Command Block Wrapper Signature 'USBC'
#define CBW_SIGNATURE               0x43425355
#define CBW_FLAGS_DATA_IN           0x80
#define CBW_FLAGS_DATA_OUT          0x00

// Command Status Wrapper Signature 'USBS'
#define CSW_SIGNATURE               0x53425355
#define CSW_STATUS_GOOD             0x00
#define CSW_STATUS_FAILED           0x01
#define CSW_STATUS_PHASE_ERROR      0x02


#include <pshpack1.h>

// Command Block Wrapper
typedef struct _CBW {
    DWORD   dCBWSignature; // 0-3
    DWORD   dCBWTag;    // 4-7
    DWORD   dCBWDataTransferLength; // 8-11
    BYTE    bmCBWFlags; // 12
    BYTE    bCBWLUN:4; // 13
    BYTE    bReserved1:4;
    BYTE    bCBWCBLength:5; // 14
    BYTE    bReserved2:3;
    BYTE    CBWCB[MAX_CBWCB_SIZE]; // 15-30
} CBW, *PCBW;

// Command Status Wrapper
typedef struct _CSW {
    DWORD   dCSWSignature; // 0-3
    DWORD   dCSWTag; // 4-7
    DWORD   dCSWDataResidue; // 8-11
    BYTE    bCSWStatus; // 12
} CSW, *PCSW;

#include <poppack.h>


#define EP0_PACKET_SIZE                     0x40
#define HIGH_SPEED_BULK_PACKET_SIZES        0x200
#define FULL_SPEED_BULK_PACKET_SIZES        0x40
#define USB_VERSION                         0x200

// USB function object
static UFN_FUNCTIONS                        g_ufnFuncs;
static PCUFN_FUNCTIONS                      g_pUfnFuncs = &g_ufnFuncs;
static UFN_HANDLE                           g_hDevice = NULL;

// USB function state
static BOOL                                 g_fDeviceRegistered = FALSE;
static BOOL                                 g_fDeviceConfigured = FALSE;

// mass storage function state
static MSC_STATE                            g_MscState = MSC_STATE_IDLE;

// transport data
#define DEFAULT_DATA_BUFFER_SIZE            0x10000 // 64 KB
static TRANSPORT_COMMAND                    g_tcCommand;
static TRANSPORT_DATA                       g_tdData;
static BYTE                                *g_pbDataBuffer;
static DWORD                                g_cbDataBuffer;
static DWORD                                g_dwDataSize;


// BOT transport data
#ifdef USBCV_MSC
#define MAX_BOT_COMMAND_LENGTH 512
static BYTE                                 g_rgbCXWBuffer[MAX_BOT_COMMAND_LENGTH]; // Use for both CSW and CBW, we need exceed 31 byte for bad command
#else
static BYTE                                 g_rgbCXWBuffer[sizeof(CBW)]; // Use for both CSW and CBW
#endif
static CBW * const                          g_pCbw = (PCBW) g_rgbCXWBuffer;
static DWORD                                g_dwCBWRead = 0;
static DWORD                                g_dwCSWSent = 0;
static DWORD                                g_dwCBWDataTransferLength = 0;
static DWORD                                g_dwCSWDataResidue = 0;
static DWORD                                g_dwCSWStatus = 0;
static BOOL                                 g_fCBWArrived = FALSE;

// default (control) pipe state data
static UFN_PIPE                             g_hDefaultPipe;
static USB_PIPE_STATE                       g_psDefaultPipeState;

// bulk out pipe state data
static UFN_PIPE                             g_hBOPipe;
static USB_PIPE_STATE                       g_psBOPipeState;

// bulk in pipe state data
static UFN_PIPE                             g_hBIPipe;
static USB_PIPE_STATE                       g_psBIPipeState;

// scratch data
static BYTE                                 g_bScratch = 0;

static BOOL                                 g_fStoreOpened = FALSE;

// Bus Speed  - Used for selecting between Descriptors
static UFN_BUS_SPEED                        g_SpeedSupported = BS_HIGH_SPEED;

static UFN_CLIENT_REG_INFO                  g_RegInfo = { sizeof(UFN_CLIENT_REG_INFO) };


#define PID_MICROSOFT_MASS_STORAGE_PROTOTYPE    0xFFFF


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
    0x00,                                   // iSerialNumber
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
    0x00,                                   // iSerialNumber
    0x01                                    // bNumConfigurations
};

#define BULK_IN_ENDPOINT_ADDRESS  0x81
#define BULK_OUT_ENDPOINT_ADDRESS 0x02


static UFN_ENDPOINT                         g_HighSpeedEndpoints[] = {
    {
        sizeof(UFN_ENDPOINT),
        {
            sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
            USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
            BULK_IN_ENDPOINT_ADDRESS,       // bEndpointAddress (endpoint 1, in)
            USB_ENDPOINT_TYPE_BULK,         // bmAttributes
            HIGH_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
            0x00                            // bInterval (interrupt only)
        },
        NULL,
        0
    },
    {
        sizeof(UFN_ENDPOINT),
        {
            sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
            USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
            BULK_OUT_ENDPOINT_ADDRESS,      // bEndpointAddress (endpoint 2, out)
            USB_ENDPOINT_TYPE_BULK,         // bmAttributes
            HIGH_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
            0x00                            // bInterval (interrupt only)
        },
        NULL,
        0
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
        NULL,
        0
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
        NULL,
        0
    }
};

static UFN_INTERFACE                        g_HighSpeedInterface = {
    sizeof(UFN_INTERFACE),
    {
        sizeof(USB_INTERFACE_DESCRIPTOR),   // bLength
        USB_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
        0x00,                               // bInterfaceNumber    
        0x00,                               // bAlternateSetting

        dim(g_HighSpeedEndpoints),          // bNumEndpoints
        MASS_STORAGE_INTERFACE_CLASS,       // bInterfaceClass
        SCSI_TRANSPARENT_INTERFACE_SUBCLASS,// bInterfaceSubClass
        BOT_INTERFACE_PROTOCOL,             // bInterfaceProtocol
        0x00                                // iInterface    
    },
    NULL,                                   // extended
    0,
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
        MASS_STORAGE_INTERFACE_CLASS,       // bInterfaceClass
        SCSI_TRANSPARENT_INTERFACE_SUBCLASS,// bInterfaceSubClass
        BOT_INTERFACE_PROTOCOL,             // bInterfaceProtocol
        0x00                                // iInterface    
    },
    NULL,                                   // extended
    0,
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
        USB_CONFIG_RESERVED_ATTRIBUTE | USB_CONFIG_SELF_POWERED,            // bmAttributes
        0x00                                // MaxPower
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
        USB_CONFIG_RESERVED_ATTRIBUTE | USB_CONFIG_SELF_POWERED,            // bmAttributes
        0x00                                // MaxPower
    },
    NULL,
    0,
    &g_FullSpeedInterface                   // interface array
};

#endif // __TRANSPORT_H__

