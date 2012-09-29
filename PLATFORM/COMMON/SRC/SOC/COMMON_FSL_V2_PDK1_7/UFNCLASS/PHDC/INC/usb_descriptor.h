/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************//*!
 *
 * @file usb_descriptor.h
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief The file is a header file for USB Descriptors required for PHDC
 *        Application
 *
 *****************************************************************************/

#ifndef _USB_DESCRIPTOR_H
#define _USB_DESCRIPTOR_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "types_phdc.h"

/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/
#define REMOTE_WAKEUP_SHIFT              (5)
#define REMOTE_WAKEUP_SUPPORT            (TRUE)

/* This macro enables/disables Meta Data Processing */
#define USB_METADATA_SUPPORTED          (TRUE)
// #define PHDC_BULK_OUT_EP_SIZE            (32)
// #define PHDC_BULK_IN_EP_SIZE             (32)
// #if USB_METADATA_SUPPORTED
//     #if (PHDC_BULK_OUT_EP_SIZE < 32)
//       #error Minimum Endpoint Size required is 32 bytes
//     #endif
// 
//     #if (PHDC_BULK_IN_EP_SIZE < 32)
//       #error Minimum Endpoint Size required is 32 bytes
//     #endif
// #endif

#define META_DATA_MSG_PRE_IMPLEMENTED    (TRUE)

/* Various descriptor sizes */
#define DEVICE_DESCRIPTOR_SIZE            (18)
#define CONFIG_DESC_SIZE                  (68)

#define DEVICE_QUALIFIER_DESCRIPTOR_SIZE  (10)
#define CONFIG_ONLY_DESC_SIZE             (9)
#define IFACE_ONLY_DESC_SIZE              (9)
#define ENDP_ONLY_DESC_SIZE               (7)
#define USB_PHDC_CLASSFUNCTION_DESC_SIZE  (4)
#define USB_QOS_DESC_SIZE                 (4)
#define USB_PHDC_FUNCTION_EXT_DESC_SIZE   (6)

#define USB_METADATA_BULK_OUT_DESC_SIZE   (4)
#define USB_METADATA_BULK_IN_DESC_SIZE    (7)
#define USB_METADATA_INT_IN_DESC_SIZE     (2)

#define PHDC_DESC_ENDPOINT_COUNT  (3)

#define PHDC_INT_IN_EP_SIZE       (8)
#define PHDC_BULK_OUT_QOS         (0x88)
#define PHDC_BULK_IN_QOS          (0x88)
#define PHDC_INT_IN_QOS           (0x88)

/* string descriptors sizes */

/* descriptors codes */
#define USB_PHDC_CLASSFUNCTION_DESCRIPTOR (0x20)
#define USB_PHDC_QOS_DESCRIPTOR   (0x21)
#define PHDC_11073PHD_FUNCTION_DESCRIPTOR  (0x30)

#if USB_METADATA_SUPPORTED
#define PHDC_METADATA_DESCRIPTOR  (0x22)
#endif

#define USB_MAX_SUPPORTED_INTERFACES     (1)

/******************************************************************************
 * Types
 *****************************************************************************/
#if 0
typedef const struct _USB_LANGUAGE
{
    uint_16 const language_id;      /* Language ID */
    uint_8 const ** lang_desc;      /* Language Descriptor String */
    uint_8 const * lang_desc_size;  /* Language Descriptor Size */
} USB_LANGUAGE;

typedef const struct _USB_ALL_LANGUAGES
{
    /* Pointer to Supported Language String */
    uint_8 const *languages_supported_string;
    /* Size of Supported Language String */
    uint_8 const languages_supported_size;
    /* Array of Supported Languages */
    USB_LANGUAGE usb_language[USB_MAX_SUPPORTED_INTERFACES];
}USB_ALL_LANGUAGES;
#endif
typedef struct _usb_class_phdc_channel
{
    uint_8 channel_num;     /* endpoint num */
    uint_8 type;            /* type of the endpoint */
    uint_8 direction;       /* direction of the endpoint */
    USB_PACKET_SIZE size;   /* size of the endpoint buffer */
    uint_8 qos;             /* qos */
}USB_CLASS_PHDC_CHANNEL;


/* structure for the endpoints used */
typedef const struct _usb_class_phdc_channel_info
{
    /* Number of Non Control Endpoints */
    uint_8 count;
    /* Array of PHDC Channel Structures */
    USB_CLASS_PHDC_CHANNEL channel[PHDC_DESC_ENDPOINT_COUNT];
}USB_CLASS_PHDC_CHANNEL_INFO, *PTR_USB_CLASS_PHDC_CHANNEL_INFO;


/******************************************************************************
 * Global Functions
 *****************************************************************************/
#endif
