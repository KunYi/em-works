/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 ******************************************************************************
 *
 * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
 * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 **************************************************************************//*!
 *
 * @file usb_phdc.h
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief The file contains USB stack PHDC class layer API header function.
 *
 *****************************************************************************/

#ifndef _USB_PHDC_H
#define _USB_PHDC_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "usb_descriptor.h"


#pragma pack(push)
/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/

#define MAX_QOS_BIN_ELEMS                 (4)
#define PHDC_RX_ENDPOINTS                 (1)/* the num of receive endpoints */
#ifndef _MC9S08JS16_H
#define PHDC_TX_ENDPOINTS                 (2)/* the num of transmit endpoints*/
#else
#define PHDC_TX_ENDPOINTS                 (1)/* the num of transmit endpoints*/
#endif
#define SET_FEATURE_REQUEST               (3)
#define CLEAR_FEATURE_REQUEST             (1)
#define GET_STATUS_REQUEST                (0)
#define INVALID_VAL                       (0xFF)

#define  USB_SET_REQUEST_MASK             (0x02)

#define USB_APP_META_DATA_PARAMS_CHANGED  (0xF2)
#define USB_APP_FEATURE_CHANGED           (0xF3)
#define BYTE_SWAP16(a) (uint_16)((((uint_16)(a)&0xFF00)>>8) | \
                                    (((uint_16)(a)&0x00FF)<<8))

/******************************************************************************
 * Types
 *****************************************************************************/
#pragma pack(1)
/* structure to hold a request in the endpoint QOS bin */
typedef struct _usb_class_phdc_qos_bin
{
    uint_8 controller_ID;   /* Controller ID*/
    uint_8 channel;         /* Endpoint number */
    boolean meta_data;      /* Packet is a meta data or not */
    uint_8 num_tfr;         /* Num of transfers that follow the meta
                               data packet.
                               used only when meta_data is TRUE */
    uint_8 qos;             /* Qos of the transfers that follow the meta
                               data packet */
    uint_8_ptr app_buff;    /* Buffer to send */
    USB_PACKET_SIZE size;   /* Size of the transfer */
}USB_CLASS_PHDC_QOS_BIN, *PTR_USB_CLASS_PHDC_QOS_BIN;

/* USB class phdc endpoint data */
typedef struct _usb_class_phdc_tx_endpoint
{
    uint_8 endpoint;                     /* from the application */
    uint_8 type;                         /* from the application */
    USB_PACKET_SIZE size;                /* from the application */
    uint_8 qos;                          /* from the application */
    uint_8 current_qos;                  /* from received meta data */
    uint_8 transfers_queued;             /* from application meta data */
    uint_8 bin_consumer;                 /* num of dequeued transfers */
    uint_8 bin_producer;                 /* num of queued transfers */
    USB_CLASS_PHDC_QOS_BIN qos_bin[MAX_QOS_BIN_ELEMS];
}USB_CLASS_PHDC_TX_ENDPOINT;

typedef struct _usb_class_phdc_rx_endpoint
{
    uint_8 endpoint;                     /* from the application */
    uint_8 type;                         /* from the application */
    USB_PACKET_SIZE size;                /* from the application */
    uint_8 qos;                          /* from the application */
    uint_8 current_qos;                  /*from received meta data */
    uint_8 transfers_left;               /*from received meta data */
    uint_16 buffer_size;
    uint_8_ptr buff_ptr;
}USB_CLASS_PHDC_RX_ENDPOINT;

typedef struct _usb_class_phdc_endpoint_data
{
    /* Number of recv non control endpoints */
    uint_8 count_rx;
    /* Number of send non control endpoints */
    uint_8 count_tx;
    /* Receive endpoint info structure */
    USB_CLASS_PHDC_RX_ENDPOINT ep_rx[PHDC_RX_ENDPOINTS];
    /* Send endpoint info structure */
    USB_CLASS_PHDC_TX_ENDPOINT ep_tx[PHDC_TX_ENDPOINTS];
}USB_CLASS_PHDC_ENDPOINT_DATA, *PTR_USB_CLASS_PHDC_ENDPOINT_DATA;

typedef struct _usb_class_phdc_rx_buff
{
    uint_8_ptr in_buff;  /* Pointer to input Buffer */
    USB_PACKET_SIZE out_size; /* Size of Output Buffer */
    uint_8_ptr out_buff; /* Pointer to Output Buffer */
#if USB_METADATA_SUPPORTED
    boolean meta_data_packet;/* meta data packet flag */
#endif
}USB_CLASS_PHDC_RX_BUFF, *PTR_USB_CLASS_PHDC_RX_BUFF;

/* event structures */
typedef struct _usb_app_event_send_complete
{
    uint_8 qos;             /* Qos of the data sent */
    uint_8_ptr buffer_ptr;  /* Pointer to the buffer sent */
    USB_PACKET_SIZE size;   /* Size of the data sent */
}USB_APP_EVENT_SEND_COMPLETE, *PTR_USB_APP_EVENT_SEND_COMPLETE;

typedef struct _usb_app_event_data_received
{
    uint_8 qos;             /* Qos of the data received */
    uint_8_ptr buffer_ptr;  /* Pointer to the data received */
    USB_PACKET_SIZE size;   /*Size of the data received */
}USB_APP_EVENT_DATA_RECIEVED, *PTR_USB_APP_EVENT_DATA_RECIEVED;

/* PHDC error codes */
typedef enum _usb_phdc_error
{
  USB_PHDC_SUCCESS = 0,
#if USB_METADATA_SUPPORTED
  USB_PHDC_METADATA_EXPECTED_NOT_RECEIVED = 256,
  USB_PHDC_METADATA_RECEIVED_NOT_EXPECTED = 257,
  USB_PHDC_CORRUPT_METADATA_PACKET_RECEIVED = 258,
#endif
}USB_PHDC_ERROR;

/* PHDC error structure */
typedef struct _usb_phdc_error_struct
{
    USB_PHDC_ERROR error_code;
}USB_PHDC_ERROR_STRUCT, *PTR_USB_PHDC_ERROR_STRUCT;

#define PHD_GET_STATUS_REQUEST    0
#define PHD_CLEAR_FEATURE_REQUEST 1
#define PHD_SET_FEATURE_REQUEST   3

#define CONTROL_TRANSFER 0
#define OUT_TRANSFER 1
#define IN_TRANSFER 2
#define I_IN_TRANSFER 3     // Must Be Consistent with the order in Endponint Descriptors

typedef struct _PIPE_TRANSFER {
    PUFN_PIPE phPipe;
    HANDLE hev;
    UFN_TRANSFER hTransfer;
} PIPE_TRANSFER, *PPIPE_TRANSFER;


extern VOID PHD_SetupRx(
    PPIPE_TRANSFER pPipeTransfer,
    PBYTE pbData,
    DWORD cbData
    );

// extern DWORD g_b_phd_rx_setup;

#if USB_METADATA_SUPPORTED

#define METADATA_PREAMBLE_SIGNATURE     (16)
#define METADATA_QOSENCODING_VERSION    (1)
#define METADATA_HEADER_SIZE            (20)

/* structure for meta_data msg preamble */
typedef struct _usb_meta_data_msg_preamble
{
    /*Meta data string for verifiability*/
    char signature[METADATA_PREAMBLE_SIGNATURE];
    /* Number of transfers to follow the meta data packet */
    uint_8 num_tfr;
    /* QOS encoding version */
    uint_8 version;
    /* QOS of the transfers to follow */
    uint_8 qos;
    /* Size of the opaque meta data */
    uint_8 opaque_data_size;
    /* Opaque meta data */
    uint_8 opaque_data[1];
}USB_META_DATA_MSG_PREAMBLE, *PTR_USB_META_DATA_MSG_PREAMBLE;

typedef struct _usb_app_event_metadata_params
{
    uint_8 channel;             /* Endpoint number */
    uint_8 num_tfr;             /* Number of transfers */
    uint_8 qos;                 /* QOS of the data */
    uint_8_ptr metadata_ptr;    /* pointer to the meta data */
    USB_PACKET_SIZE size;       /* Size of the transfer */
}USB_APP_EVENT_METADATA_PARAMS, *PTR_USB_APP_EVENT_METADATA_PARAMS;
#endif
/******************************************************************************
 * Global Functions
 *****************************************************************************/
typedef void (WINAPI *LPUSB_CLASS_CALLBACK) (
    DWORD   dwMsg,
    DWORD   dwParam,
    void*   val
    );

typedef BOOL (WINAPI *LPAPP_NOTIFY) (
    PVOID   pvNotifyParameter,
    DWORD   dwMsg,
    DWORD   dwParam
    );

typedef BOOL (WINAPI *LPAPP_CALLBACK) (
    PVOID   pvNotifyParameter,
    DWORD   dwMsg,
    DWORD   dwParam
    );

extern DWORD PHD_InternalInit(
    LPCTSTR pszActiveKey,
    LPUSB_CLASS_CALLBACK phdc_class_callback,
    LPAPP_NOTIFY app_callback
    );

extern DWORD PHD_Close();

extern uint_8 USB_Class_PHDC_Send_Data (
    boolean meta_data,      /* [IN] Packet is meta data or not */
    uint_8 num_tfr,         /* [IN] Number of transfers
                                    following meta data packet */
    uint_8 qos,             /* [IN] Qos of the transfer */
    uint_8_ptr app_buff,    /* [IN] Buffer to send */
    USB_PACKET_SIZE size    /* [IN] Length of the transfer */
);

extern uint_8 USB_Class_PHDC_Recieve_Data(void );

extern UFN_HANDLE* getDeviceHandle(void);
extern PUFN_FUNCTIONS getFuncsHandle(void);

// extern PIPE_TRANSFER g_rgPipeTransfers[];

#define USB_Class_PHDC_Periodic_Task    USB_Class_Periodic_Task
#define USB_Class_PHDC_Recv_Data        USB_Device_Recv_Data

#pragma pack(pop)
#endif
