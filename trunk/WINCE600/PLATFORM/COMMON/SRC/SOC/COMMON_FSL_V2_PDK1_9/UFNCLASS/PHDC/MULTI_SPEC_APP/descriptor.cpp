//------------------------------------------------------------------------------
//
// Copyright (C) 2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------ 

#include "transport.h"

USB_DEVICE_DESCRIPTOR                g_HighSpeedDeviceDesc = {
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

USB_DEVICE_DESCRIPTOR                g_FullSpeedDeviceDesc = {
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

UCHAR g_EndpointsExtend_BulkIn[] = {
// Qos
    USB_QOS_DESC_SIZE, // 0x4,
    USB_PHDC_QOS_DESCRIPTOR, // 0x21,
    0x01,
    PHDC_BULK_IN_QOS // 0x88,
#if USB_METADATA_SUPPORTED
    ,
// meta data
    USB_METADATA_BULK_IN_DESC_SIZE, // 0x7,
    PHDC_METADATA_DESCRIPTOR, // 0x22,

    0xAB,   // opaque meta data
    0xCD,
    0xEF,
    0x01,
    0x02
#endif
};

UCHAR g_EndpointsExtend_BulkOut[] = {
    USB_QOS_DESC_SIZE,
    USB_PHDC_QOS_DESCRIPTOR,
    0x01,                                  /* qos encoding version */
    PHDC_BULK_OUT_QOS                     /* latency/reliability bin */
#if USB_METADATA_SUPPORTED
    ,
// meta data
    USB_METADATA_BULK_OUT_DESC_SIZE, // 0x4,
    PHDC_METADATA_DESCRIPTOR, // 0x22,
    0xCC,
    0xDD
#endif
};

UCHAR g_EndpointsExtend_InterruptIn[] = {
    USB_QOS_DESC_SIZE, // 0x4,
    USB_PHDC_QOS_DESCRIPTOR, // 0x21,
    0x01,
    PHDC_INT_IN_QOS  //0x88
};

UFN_ENDPOINT                         g_HighSpeedEndpoints[] = {
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
#if USB_INTERRUPT_EP_SUPPORTED
    {
        sizeof(UFN_ENDPOINT),
        {
            sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
            USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
            INTERRUPT_IN_ENDPOINT_ADDRESS,  // bEndpointAddress (endpoint 3, interrupt in)
            USB_ENDPOINT_TYPE_INTERRUPT,    // bmAttributes
            HIGH_SPEED_ISO_PACKET_SIZES,    // wMaxPacketSize
#if 1  /*FOR USBCV CHAP9 TEST*/
            0x10                            // bInterval (interrupt only), for passing usbcv, this value must in [0x01..0x10]
#else
            0xF0                            // bInterval (interrupt only)
#endif
        },
        &g_EndpointsExtend_InterruptIn,
        sizeof(g_EndpointsExtend_InterruptIn)
    }
#endif
};

UFN_ENDPOINT                         g_FullSpeedEndpoints[] = {
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
#if USB_INTERRUPT_EP_SUPPORTED
    {
        sizeof(UFN_ENDPOINT),
        {
            sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
            USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
            INTERRUPT_IN_ENDPOINT_ADDRESS,  // bEndpointAddress (endpoint 3, interrupt in)
            USB_ENDPOINT_TYPE_INTERRUPT,    // bmAttributes
            FULL_SPEED_ISO_PACKET_SIZES,    // wMaxPacketSize
#if 1  /*FOR USBCV CHAP9 TEST*/
            0x10                            // bInterval (interrupt only), for passing usbcv, this value must in [0x01..0x10]
#else
            0xF0                            // bInterval (interrupt only)
#endif
        },
        &g_EndpointsExtend_InterruptIn,
        sizeof(g_EndpointsExtend_InterruptIn)
    }
#endif
};

UCHAR g_InterfaceExtended[] = {
    /* PHDC class function descriptor */
    USB_PHDC_CLASSFUNCTION_DESC_SIZE,  // 0x04,
    USB_PHDC_CLASSFUNCTION_DESCRIPTOR, // 0x20,
    0x02,                              // data and messaging formats not defined by vendor DATA FORMAT -- PHDC_11073_20601
    META_DATA_MSG_PRE_IMPLEMENTED,     // 0x01,

    /*PHDC function extension descriptor */
    USB_PHDC_FUNCTION_EXT_DESC_SIZE,   // 0x06,
    PHDC_11073PHD_FUNCTION_DESCRIPTOR, // 0x30,
    0x00,
    0x01,
    0x34,
    0x12
};

UFN_INTERFACE                        g_HighSpeedInterface = {
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

UFN_INTERFACE                        g_FullSpeedInterface = {
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

UFN_CONFIGURATION                    g_HighSpeedConfig = {
    sizeof(UFN_CONFIGURATION),
    {
        sizeof(USB_CONFIGURATION_DESCRIPTOR),// bLength
        USB_CONFIGURATION_DESCRIPTOR_TYPE,  // bDescriptorType
        
        CB_CONFIG_DESCRIPTOR,               // wTotalLength
        0x01,                               // bNumInterfaces
        0x01,                               // bConfigurationValue
        0x00,                               // iConfiguration
#if 1  /*FOR USBCV CHAP9 TEST*/
        0xc0,                               // Bit 6 : Self Power, Bit 5 : Remote Wakeup
#else
        0xe0,                               // VBus current draw config
#endif
        0x32
    },
    NULL,
    0,
    &g_HighSpeedInterface                   // interface array
};

UFN_CONFIGURATION                    g_FullSpeedConfig = {
    sizeof(UFN_CONFIGURATION),
    {
        sizeof(USB_CONFIGURATION_DESCRIPTOR),// bLength
        USB_CONFIGURATION_DESCRIPTOR_TYPE,  // bDescriptorType
        
        CB_CONFIG_DESCRIPTOR,               // wTotalLength
        0x01,                               // bNumInterfaces
        0x01,                               // bConfigurationValue
        0x00,                               // iConfiguration
#if 1  /*FOR USBCV CHAP9 TEST*/
        0xc0,                               // Bit 6 : Self Power, Bit 5 : Remote Wakeup
#else
        0xe0,                               // VBus current draw config
#endif
        0x32
    },
    NULL,
    0,
    &g_FullSpeedInterface                   // interface array
};
