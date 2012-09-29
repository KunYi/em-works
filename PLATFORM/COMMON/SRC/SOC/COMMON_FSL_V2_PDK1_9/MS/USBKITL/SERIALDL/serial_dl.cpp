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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  serial_dl.c
//
//  This is a hardware-independent USB Serial Client driver. It will interpret 
//  and respond to standard USB Host requests, as well as serial-specific
//  Host requests during initialization. 
//
//  This code is meant to provide the minimal set of routines necessary for USB
//  transfers, and is targeted for the boot-loader and small code size.

#pragma warning(push)
#pragma warning(disable: 6287 6262 4201 4512 4100)
#include <windows.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <oal.h>
#include <oal_blserial.h>
#include <usbkitl.h>
#pragma warning(pop)

//------------------------------------------------------------------------------
//
//  Entry points for calling application
//
extern "C" BOOL USBSerialInit();
extern "C" DWORD FslUfnGetKitlDMABuffer();

static BOOL g_USBSerialInitialized = FALSE;

extern void OEMKitlSerialInit (void);

//------------------------------------------------------------------------------
//
//  Exit points to USB Function driver. Using the UFN MDD/PDD interfaces, reused
//  from OS driver. PDD Interface receives transfer information in 'STransfer's
//
UFN_MDD_INTERFACE_INFO g_mddInterface;
UFN_PDD_INTERFACE_INFO g_pddInterface;

STransfer g_EP0Transfer; // EP0
STransfer g_EP1Transfer; // Bulk out
STransfer g_EP2Transfer; // Bulk in

extern "C" char g_serial_send_buf[];
extern "C" char g_serial_rev_buf[];

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_STATE, g_DeviceState
//
//  Keep track of the USB state - Cable detatched, attached, initialized, etc.
//
enum DEVICE_STATE {
    DS_DETACHED = 0,
    DS_ATTACHED,
    DS_POWERED,
    DS_DEFAULT,
    DS_ADDRESSED,
    DS_CONFIGURED,
    DS_SUSPENDED,
};
enum DEVICE_STATE g_DeviceState = DS_DETACHED;


//------------------------------------------------------------------------------
//
//  Type:  TRANSFER_STATE
//
//  USB Serial defines 3 endpoints, hold the current state of each endpoint
//    0: Control
//    1: Receive Packets
//    2: Send Packets
//  
//
enum TRANSFER_STATE {
    TS_IDLE=0,
    TS_RECEIVING_MESSAGE,
    TS_SENDING_MESSAGE,
    TS_RECEIVING_PACKET,
    TS_SENDING_PACKET,
    TS_SENDING_PACKET_TERMINATOR
};
enum TRANSFER_STATE g_EP0State = TS_IDLE;
enum TRANSFER_STATE g_EP1State = TS_IDLE;
enum TRANSFER_STATE g_EP2State = TS_IDLE;

//------------------------------------------------------------------------------
//
//  Endpoint 0 Transfer types
//
//  Necessary for constructing Host responses and issuing them to hardware
typedef enum {
    EP0Setup = 0,
    EP0Out,
    EP0In
} EP0_DIR;

typedef struct _EP0_REQUEST EP0_REQUEST, *PEP0_REQUEST;
struct _EP0_REQUEST
{
    EP0_DIR eDir;
    UCHAR *pucData;
    DWORD dwExpectedSize;
    DWORD dwActualSize;
    VOID (*pfnNotification)(EP0_REQUEST *pRequest, PVOID pvUser);
    PVOID pvUser;

    DWORD dwProcessed;
    BOOL fCompleted;
};


//------------------------------------------------------------------------------
//
//  Receive buffer managment routines
//
//  This wrapper will buffer data coming from the hardware line, and provide
//  it to the application on request.
//
#define RECV_BUFFER_SIZE 0x1000 //1750  // Must be at least KITL_MTU + 1
#define SPACE_IN_BUFFER(head,tail) \
    (tail >= head ? tail-head : RECV_BUFFER_SIZE - head + tail)
#define SPACE_UNTIL_WRAP(tail) (RECV_BUFFER_SIZE - tail)

//BYTE g_RecvBuffer[RECV_BUFFER_SIZE];
BYTE * g_RecvBuffer;

USHORT g_RecvBufferHead = 0;
USHORT g_RecvBufferTail = 0;


//------------------------------------------------------------------------------
//
//  USB Serial session descriptors
//
//  Data needed by the USB Host to initialize a USB Serial session
//
#define EP0MaxSize 64
#define EPFullMaxSize  64
#define EPHighMaxSize  0x200

#define REV_DMA_OFFSET 0x9000

#define CFGLEN     32    // Length of configuration descriptor
#define iCONF      18    // Index in gs_pucUSBDescriptors for Configuration Descriptors
#define iINF       27    // Index in gs_pucUsBDescriptors for Interface Descriptors

static USB_DEVICE_DESCRIPTOR deviceDescriptor;
static UFN_ENDPOINT rgEndPoints[3];
static UFN_INTERFACE Interface;
static UFN_CONFIGURATION deviceConfiguration;
static UFN_BUS_SPEED busSpeed;

///////////////////////////////
// Serial USB Descriptor Set //
///////////////////////////////

static UCHAR gs_pucUSBDescriptors[]=  {

    /////////////////////////////////////////////
    // Standard Device Descriptor (One Device) //
    /////////////////////////////////////////////

    /* 0  */    18,                 // bLength
    /* 1  */    USB_DEVICE_DESCRIPTOR_TYPE, 
    /* 2  */    0, 2,               // bcdUSB
    /* 4  */    0xff,               // bDeviceClass (0xFF = Vendor Specific)
    /* 5  */    0xff,               // bDeviceSubClass
    /* 6  */    0xff,               // bDeviceProtocol
    /* 7  */    EP0MaxSize,         // bMaxPacketSize0
    /* 8  */    0x5E, 0x04,         // idVendor (Microsoft Vendor ID)
#if USING_USBSER
    /* 10 */    0x79, 0x00,         // idProduct
    /* 12 */    0x90, 0,            // bcdDevice
#else
    /* 10 */    0xCE, 0x00,         // idProduct
    /* 12 */    0, 0,               // bcdDevice
#endif
    /* 14 */    0,                  // iManufacturer - index of Manf String Descriptor
    /* 15 */    0,                  // iProduct - index of Product String Descriptor
    /* 16 */    0,                  // iSerialNumber - Index of Serial Number String
    /* 17 */    1,                  // bNumConfigurations

    ///////////////////////////////////////////////////////////
    // Standard Configuration Descriptor (One Configuration) //
    ///////////////////////////////////////////////////////////

    /* 18 */    9,                  // bLength
    /* 19 */    USB_CONFIGURATION_DESCRIPTOR_TYPE, 
    /* 20 */    CFGLEN%256,CFGLEN/256,  // wTotalLength
    /* 22 */    1,                  // bNumInterfaces
    /* 23 */    1,                  // bConfigurationValue
    /* 24 */    0,                  // iConfiguration
    /* 25 */    0x40,               // bmAttributes (self powered)
    /* 26 */    1,                  // MaxPower (x2 mA)

    ///////////////////////////////////////////////////
    // Standard Interface Descriptor (One Interface) //
    ///////////////////////////////////////////////////

    /* 27 */    9,                  // bLength
    /* 28 */    USB_INTERFACE_DESCRIPTOR_TYPE, 
    /* 29 */    0,                  // bInterfaceNumber
    /* 30 */    0,                  // bAlternateSetting
    /* 31 */    2,                  // bNumEndpoints (number endpoints used, excluding EP0)
    /* 32 */    0xff,               // bInterfaceClass
    /* 33 */    0xff,               // bInterfaceSubClass
    /* 34 */    0xff,               // bInterfaceProtocol
    /* 35 */    0,                  // ilInterface  (Index of this interface string desc.)

    ///////////////////////////////////////////////////
    // Standard Endpoint Descriptor (EP1 - BULK OUT) //
    ///////////////////////////////////////////////////

    /* 36 */    7,                  // bLength
    /* 37 */    USB_ENDPOINT_DESCRIPTOR_TYPE,
    /* 38 */    0x01,               // bEndpointAddress (EP 1, OUT)
    /* 39 */    2,                  // bmAttributes  (0010 = Bulk)
    /* 40 */    (EPHighMaxSize&0xFF), (EPHighMaxSize>>8),       // wMaxPacketSize
    /* 42 */    0,                  // bInterval (ignored for Bulk)

    //////////////////////////////////////////////////
    // Standard Endpoint Descriptor (EP2 - BULK IN) //
    //////////////////////////////////////////////////

    /* 43 */    7,                  // bLength
    /* 44 */    USB_ENDPOINT_DESCRIPTOR_TYPE,   
    /* 45 */    0x82,               // bEndpointAddress (EP 2, IN)
    /* 46 */    2,                  // bmAttributes  (0010 = Bulk)
    /* 47 */    (EPHighMaxSize&0xFF), (EPHighMaxSize>>8),       // wMaxPacketSize
    /* 49 */    0                   // bInterval (ignored for Bulk)
};

// Device specific request
#define SET_CONTROL_LINE_STATE  0x22
static UFN_BUS_SPEED usb_dev_speed = BS_HIGH_SPEED;

//------------------------------------------------------------------------------
//
//  Function:  Interrupt Thread
//
//  Entry to PDD interrupt thread. This will be called repeatedly (bootloader uses polling)
//
extern "C" DWORD UsbFnInterruptThread(VOID *pPddContext);
extern "C" BOOL InitializeHardware(); // Hook for board-specific init (GPIO's, clocks, etc)
extern "C" void msWait(UINT32 msVal);

//------------------------------------------------------------------------------
//
//  Function:  HandleUSBDeviceRegistration
//
//  Negotiate for physical bulk endpoints for send/receive. Negotiate hardware's
//  maxPacketSizes and update our descriptors to reflect hardware capabilities.
//
static void HandleUSBDeviceRegistration()
{
    DWORD dwRet;
    DWORD logicalEndPoint;
    DWORD physicalEndPoint;
    BOOL logEndPointTaken[4] = {FALSE, FALSE, FALSE, FALSE};
    BOOL physEndPointTaken[16] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
        FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };

    USB_CONFIGURATION_DESCRIPTOR *pUsbConfDesc = (USB_CONFIGURATION_DESCRIPTOR*)&gs_pucUSBDescriptors[iCONF];
    USB_INTERFACE_DESCRIPTOR *pUsbInfDesc = (USB_INTERFACE_DESCRIPTOR*)&gs_pucUSBDescriptors[iINF];

    // Fill descriptors that the PDD will use to register configuration and interface
    deviceConfiguration.pInterfaces = &Interface;
    deviceConfiguration.Descriptor.bNumInterfaces = 1;
    Interface.pEndpoints = rgEndPoints+1; // EP0 don't included in Endpoint descritpions
    Interface.Descriptor.bNumEndpoints = 2;
    deviceDescriptor.bMaxPacketSize0 = EP0MaxSize;
    busSpeed = usb_dev_speed;

    // Fill endpoint descriptors with some initial values, and negotiate them with PDD
    // Logical EP0
    rgEndPoints[0].Descriptor.bEndpointAddress = 0x00;
    rgEndPoints[0].Descriptor.wMaxPacketSize = EP0MaxSize;
    rgEndPoints[0].Descriptor.bmAttributes = USB_ENDPOINT_TYPE_CONTROL;
    // Logical EP1
    rgEndPoints[1].Descriptor.bEndpointAddress = 0x01; // EP1, OUT (BULK) 
    rgEndPoints[1].Descriptor.wMaxPacketSize = (busSpeed==BS_HIGH_SPEED)? EPHighMaxSize:EPFullMaxSize;
    rgEndPoints[1].Descriptor.bmAttributes = USB_ENDPOINT_TYPE_BULK;

    // Logical EP2    
    rgEndPoints[2].Descriptor.bEndpointAddress = 0x82; // EP2, IN (BULK) 
    rgEndPoints[2].Descriptor.wMaxPacketSize = (busSpeed==BS_HIGH_SPEED)? EPHighMaxSize:EPFullMaxSize;
    rgEndPoints[2].Descriptor.bmAttributes = USB_ENDPOINT_TYPE_BULK;


    // Ensure that the specified endpoint 0 can be supported
    dwRet = g_pddInterface.pfnIsEndpointSupportable(
        g_pddInterface.pvPddContext, 0, busSpeed, &rgEndPoints[0].Descriptor, 0,0,0);
    if( dwRet != ERROR_SUCCESS )
    {
        OALMSG(OAL_ETHER&&OAL_FUNC, (L"ERROR: PDD cannot support endpoint 0\r\n"));
    }

    // Setup remaining endpoints (Logical endpoints 1 and 2 for Serial) 
    for( logicalEndPoint = 1; logicalEndPoint <=2; logicalEndPoint++)
    {
        for( physicalEndPoint=1; physicalEndPoint<16; physicalEndPoint++ )
        {

            if( !physEndPointTaken[physicalEndPoint] )
            {

                rgEndPoints[logicalEndPoint].Descriptor.bEndpointAddress &= ~0x7F;
                rgEndPoints[logicalEndPoint].Descriptor.bEndpointAddress |= physicalEndPoint;

                dwRet = g_pddInterface.pfnIsEndpointSupportable(
                    g_pddInterface.pvPddContext, 
                    physicalEndPoint, busSpeed, &rgEndPoints[logicalEndPoint].Descriptor, 
                    pUsbConfDesc->bConfigurationValue,
                    pUsbInfDesc->bInterfaceNumber, pUsbInfDesc->bAlternateSetting);

                if( dwRet == ERROR_SUCCESS )
                {
                    OALMSG(OAL_ETHER&&OAL_FUNC, (L"Mapping %d to %d\r\n", logicalEndPoint, physicalEndPoint));
                    physEndPointTaken[physicalEndPoint] = TRUE;
                    logEndPointTaken[logicalEndPoint] = TRUE;
                    break;
                }
            }
        }

        if( !logEndPointTaken[logicalEndPoint])
        {
            OALMSG(OAL_ETHER&&OAL_FUNC, (L"ERROR: PDD cannot support logical endpoint %d\r\n", logicalEndPoint));
            break;
        }
    }


    // Update master descriptor with negotiated values. 
    // Logical EP1
    gs_pucUSBDescriptors[38] = rgEndPoints[1].Descriptor.bEndpointAddress;
    gs_pucUSBDescriptors[40] = (UCHAR)((rgEndPoints[1].Descriptor.wMaxPacketSize >> 0) & 0xFF);
    gs_pucUSBDescriptors[41] = (UCHAR)((rgEndPoints[1].Descriptor.wMaxPacketSize >> 8) & 0xFF);
    // Logical EP2
    gs_pucUSBDescriptors[45] = rgEndPoints[2].Descriptor.bEndpointAddress;
    gs_pucUSBDescriptors[47] = (UCHAR)((rgEndPoints[2].Descriptor.wMaxPacketSize >> 0) & 0xFF);
    gs_pucUSBDescriptors[48] = (UCHAR)((rgEndPoints[2].Descriptor.wMaxPacketSize >> 8) & 0xFF);

    // We assume knowledge of the RegisterDevice function implementation and pass very simplified arguments
    g_pddInterface.pfnRegisterDevice( g_pddInterface.pvPddContext, 
        &deviceDescriptor, &deviceConfiguration, NULL,
        &deviceDescriptor, &deviceConfiguration, NULL,
        NULL, 0 );

}

//------------------------------------------------------------------------------
//
//  Function:  HandleClassRequest
//
//  Called by HandleSetupPkt when type is USB_REQUEST_CLASS.
//  This is serial-function specific stuff that updates modem status
//
void HandleClassRequest(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest)
{
    if (pUdr->bmRequestType == (USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE | USB_REQUEST_HOST_TO_DEVICE) ) 
    {
        if( pUdr->bRequest == SET_CONTROL_LINE_STATE ) 
        {
            // Fill out pRequest structure such that a handshake is sent
            // and ignore any line status data
            pRequest->eDir = EP0Setup;
            pRequest->pucData = NULL;
            pRequest->dwExpectedSize = 0;
            pRequest->dwActualSize = 0;
            pRequest->pfnNotification = NULL;
            pRequest->pvUser = NULL;
        }
        else ASSERT(!"Host indicated an unsupported SET Class Request");
    }
    else
    {
        ASSERT(!"Host sent unhandled request type!");
    }
}


//------------------------------------------------------------------------------
//
//  Function:  HandleGetDescriptor
//
//  Called by HandleSetupPkt when type is USB_REQUEST_GET_DESCRIPTOR.
//  Will manage transmission of descriptors, strings, etc.
//
void HandleGetDescriptor(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest)
{
    UCHAR *pucData = NULL;
    WORD wLength =0;
    
    switch(HIBYTE(pUdr->wValue))
    {
    case USB_DEVICE_DESCRIPTOR_TYPE:
        pucData = (UCHAR *)gs_pucUSBDescriptors;
        wLength = gs_pucUSBDescriptors[0];
        break;
    case USB_CONFIGURATION_DESCRIPTOR_TYPE:
        pucData = (UCHAR *)&gs_pucUSBDescriptors[iCONF];
        wLength = CFGLEN;
        break;
    case USB_STRING_DESCRIPTOR_TYPE:
        pucData = NULL;
        wLength = 0;
        break;
    default:
        ASSERT(!"Unhandled Get DESCRIPTOR_TYPE!");
    }

    // Update EP0 Request to send data, as this is a GET request.
    pRequest->eDir = EP0In;
    pRequest->pucData = pucData;
    pRequest->dwExpectedSize = pUdr->wLength;
    pRequest->dwActualSize = min(wLength, pUdr->wLength);
}

//------------------------------------------------------------------------------
//
//  Function:  HandleTransferComplete
//
//  Respond to UFN_MSG_TRANSFER_COMPLETE. Do some cleanup and change EP state.
//
void HandleTransferComplete(STransfer *pTransfer)
{
    int pksize;
    pksize = (usb_dev_speed == BS_HIGH_SPEED)? EPHighMaxSize:EPFullMaxSize;
    if( pTransfer == &g_EP0Transfer )
    {
        g_pddInterface.pfnSendControlStatusHandshake( g_pddInterface.pvPddContext, 0 );
        g_EP0State = TS_IDLE;
    }
    else if( pTransfer == &g_EP1Transfer )
    {
        // Data received on BULK OUT endpoint
        if( g_EP1Transfer.dwUsbError != UFN_NO_ERROR )
        {
            OALMSG(OAL_ERROR, (L"ERROR: PDD Indicated there was a transfer error on EP1!\r\n"));
            DEBUGCHK(0);
        }

        if(g_EP1State != TS_RECEIVING_PACKET)
        {
            OALMSG(OAL_ERROR, (L"ERROR: PDD Indicated RECV complete before transfer issued! ep1st is 0x%X\r\n",g_EP1State));
            DEBUGCHK(0);
        }

        g_EP1State = TS_IDLE;
    }
    else if( pTransfer == &g_EP2Transfer )
    {
        if( g_EP2State == TS_SENDING_PACKET )
        {
            // Data sent on BULK IN endpoint
            if( g_EP2Transfer.cbTransferred != g_EP2Transfer.cbBuffer )
            {
                OALMSG(OAL_ERROR, (L"ERROR: Sent data does not match expected size!\r\n"));
                DEBUGCHK(0);
            }
            if( g_EP2Transfer.dwUsbError != UFN_NO_ERROR )
            {
                OALMSG(OAL_ERROR, (L"ERROR: PDD Indicated there was a transfer error on EP2!\r\n"));
                DEBUGCHK(0);
            }

            if( (g_EP2Transfer.cbTransferred % pksize ) == 0 )
            {
                g_EP2Transfer.dwFlags = USB_REQUEST_DEVICE_TO_HOST;
                g_EP2Transfer.pvBuffer = NULL;
                g_EP2Transfer.cbBuffer = 0;
                g_EP2Transfer.cbTransferred = 0;
                g_EP2Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR;
                g_EP2Transfer.pvPddData = NULL;
                g_EP2Transfer.pvPddTransferInfo = NULL;

                g_EP2State = TS_SENDING_PACKET_TERMINATOR;
                g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext, 2, &g_EP2Transfer );
            }
            else
            {
                g_EP2State = TS_IDLE;
            }
        }
        else if ( g_EP2State == TS_SENDING_PACKET_TERMINATOR)
        {
            if( g_EP2Transfer.dwUsbError != UFN_NO_ERROR )
            {
                OALMSG(OAL_ERROR, (L"ERROR: PDD Indicated there was a transfer error on EP2 terminator!\r\n"));
                DEBUGCHK(0);
            }
            g_EP2State = TS_IDLE;
        }
        else
        {
            ASSERT(!"Endpoint 2 has entered an unsupported state!");
        }
    }
    else
    {
        ASSERT(!"PDD Signaled bogus transfer complete!");
    }
}

//------------------------------------------------------------------------------
//
//  Function:  HandleSetupPkt
//
//  Respond to SETUP packets as they come in from the PDD on EP0
//
static void HandleSetupPkt(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest)
{
    //At a minimum prepare to ACK the SETUP packet
    pRequest->eDir = EP0Setup;
    pRequest->pucData = NULL;
    pRequest->dwExpectedSize = 0;
    pRequest->dwActualSize = 0;
    pRequest->pfnNotification = NULL;
    pRequest->pvUser = NULL;

    switch( pUdr->bmRequestType )
    {
    case USB_REQUEST_DEVICE_TO_HOST:
    case USB_REQUEST_HOST_TO_DEVICE:
        switch( pUdr->bRequest )
        {
        case USB_REQUEST_GET_DESCRIPTOR:
            HandleGetDescriptor(pUdr, pRequest);
            break;

        case USB_REQUEST_SET_CONFIGURATION:
            g_DeviceState = DS_CONFIGURED;
            {
                USB_ENDPOINT_DESCRIPTOR        *pUsbEP1  = (USB_ENDPOINT_DESCRIPTOR*)&gs_pucUSBDescriptors[36];
                USB_ENDPOINT_DESCRIPTOR        *pUsbEP2  = (USB_ENDPOINT_DESCRIPTOR*)&gs_pucUSBDescriptors[43];

                g_pddInterface.pfnInitEndpoint( g_pddInterface.pvPddContext, 
                    1, usb_dev_speed, pUsbEP1, NULL, 0, 0, 0);         
                g_pddInterface.pfnInitEndpoint( g_pddInterface.pvPddContext, 
                    2, usb_dev_speed, pUsbEP2, NULL, 0, 0, 0);


            }
            // setup RX transfer on data out
            // setupDataOutTransfer();
            break;

        case USB_REQUEST_SET_ADDRESS:
            // TODO: handle this
            g_pddInterface.pfnSetAddress(g_pddInterface.pvPddContext,(BYTE)(pUdr->wValue));
            break;

        default:
            ASSERT(!"Only GET_DESC and SET_CONFIG supported!");
        }
        break;
    default:
        HandleClassRequest(pUdr, pRequest);
    }
}


//------------------------------------------------------------------------------
//
//  Function:  HandlePDDNotification
//
//  The USB FN PDD will call this function to notify it of various events (data sent/received, 
//  setup packet received, device addressed, reset, etc.
//
static BOOL WINAPI HandlePDDNotification (
    PVOID pvMddContext,
    DWORD dwMsg,
    DWORD dwParam
    )
{
    EP0_REQUEST Request;
    EP0_REQUEST *pRequest = &Request;
    DWORD ret;
    UNREFERENCED_PARAMETER(pvMddContext);
    switch( dwMsg )
    {
    case UFN_MSG_SETUP_PACKET:

        HandleSetupPkt( (USB_DEVICE_REQUEST*)(void*)dwParam, pRequest);

        pRequest->dwProcessed = 0;
        pRequest->fCompleted = FALSE;

        if( pRequest->eDir != EP0Setup )
        {
            g_EP0Transfer.pvBuffer = pRequest->pucData;
            g_EP0Transfer.cbBuffer = pRequest->dwActualSize;
            g_EP0Transfer.cbTransferred = 0;
            g_EP0Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR;
            g_EP0Transfer.pvPddData = NULL;
            g_EP0Transfer.pvPddTransferInfo = NULL;

            if( pRequest->eDir == EP0In )
            {
                OALMSG(OAL_FUNC, (L"Got SETUP IN packet\r\n"));
                g_EP0Transfer.dwFlags = USB_REQUEST_DEVICE_TO_HOST;
                g_EP0State = TS_SENDING_MESSAGE;
                ret = g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext, 0, &g_EP0Transfer);

            }
            else
            {
                OALMSG(OAL_FUNC, (L"Got SETUP OUT packet\r\n"));
                g_EP0Transfer.dwFlags = USB_REQUEST_HOST_TO_DEVICE;
                g_EP0State = TS_RECEIVING_MESSAGE;
                g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext, 0, &g_EP0Transfer);
            }        
        }
        else
        {
            g_pddInterface.pfnSendControlStatusHandshake( g_pddInterface.pvPddContext, 0 );
            g_EP0State = TS_IDLE;
        }
        break;

    case UFN_MSG_TRANSFER_COMPLETE:
        HandleTransferComplete( (STransfer*)(void*)dwParam);
        break;

    case UFN_MSG_BUS_EVENTS:
        switch( dwParam )
        {
        case UFN_DETACH:
            g_DeviceState = DS_DETACHED;
            break;
        case UFN_ATTACH:
            g_DeviceState = DS_ATTACHED;
            break;
        case UFN_RESET:
            g_DeviceState = DS_DEFAULT;
            // TODO: return to clean state?
            break;
        case UFN_SUSPEND:
            g_DeviceState = DS_SUSPENDED;
            break;
        case UFN_RESUME:
            g_DeviceState = DS_DEFAULT;
            break;
        default:
            ASSERT(!"Unexpected bus event!");
        }
        break;

    case UFN_MSG_BUS_SPEED:
        usb_dev_speed = (UFN_BUS_SPEED)dwParam;
        HandleUSBDeviceRegistration();
        break;

    case UFN_MSG_SET_ADDRESS:
        if( dwParam ) g_DeviceState = DS_ADDRESSED;
        else g_DeviceState = DS_DEFAULT;
        break;

    default:
        ASSERT(!"Unexpected Msg in HandlePDDNotification!");
    }
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  USBSerialInit
//
//  Lets send some data to the USB Bus.
//
BOOL USBSerialInit()
{
    if(g_USBSerialInitialized)
    {
        OALMSG(OAL_ERROR, (L"ERROR: USBSerialInit called again before USBSerialDeinit\r\n"));
        return FALSE;   
    }

    g_RecvBuffer = (BYTE*)OALCAtoUA( g_serial_rev_buf );

    // Setup the MDD/PDD interface structures
    g_mddInterface.dwVersion = 1;
    g_mddInterface.pfnNotify = HandlePDDNotification;
    memset( &g_pddInterface, 0, sizeof(g_pddInterface) );
    g_pddInterface.dwVersion = 1;

    // Initialize our internal receive buffer markers
    g_RecvBufferHead = g_RecvBufferTail = 0;

    // Give the OEM an opportunity to setup clocks, GPIOs, etc:
    //if( !InitializeHardware() )
    //{
    //    OALMSG(OAL_ERROR, (L"InitializeHardware failed!"));
    //    return FALSE;
    //}

    //OALMSG(1, (L"Call UfnPdd_Init!\r\n"));    
    // Initialize USB Function block
    if( UfnPdd_Init( NULL, (PVOID)0xdeadbeef, &g_mddInterface, &g_pddInterface ) != ERROR_SUCCESS )
    {
        OALMSG(OAL_ERROR, (L"UfnPdd_Init failed!"));
        return FALSE;
    }

    //OALMSG(1, (L"HandleUSBDeviceRegistration!\r\n"));    
    // Negotiate with hardware by calling IsEndpointSupportable, IsConfigurationSupportable, etc.
    // Update descriptors to reflect negotiated endpoints.
    HandleUSBDeviceRegistration();

     // Setup complete, Attach device to USB bus
    g_pddInterface.pfnStart( g_pddInterface.pvPddContext );

    // Give time for host to recognize device
    // Sometimes we can hang without this delay
    // msWait(1);

    // Respond to GET_DESCRIPTOR, SET_ADDRESS... until SET_CONFIGURATON
    //OALMSGS(1, (L"pfnStart!\r\n"));    
    //OALMSGS(1, (L"UsbFnInterruptThread\r\n"));

    while( g_DeviceState != DS_CONFIGURED )
    {
        UsbFnInterruptThread( g_pddInterface.pvPddContext );
    }

    g_USBSerialInitialized = TRUE;
    return g_USBSerialInitialized;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSerialSendRaw
//
//  Lets send some data to the USB Bus.
//
#define MAX_SENDSIZE 0x1000
BOOL OEMSerialSendRaw(LPBYTE pbFrame, USHORT cbFrame)
{
    BYTE * pDmaBuffer;
    int size=0;
    if( !g_USBSerialInitialized)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMSerialSendRaw called before USBSerialInit\r\n"));
        return g_USBSerialInitialized;
    }
    // OALMSG(1, (L"OEMSerialSendRaw 0x%x 0x%x\r\n", pbFrame, cbFrame));
    // Package the transfer information into a struct to pass to PDD
    pDmaBuffer = (BYTE*)OALCAtoUA( g_serial_send_buf);
#pragma warning(push)
#pragma warning(disable: 4127)
    while(1)
#pragma warning(pop)
    {

        size = (cbFrame>MAX_SENDSIZE)? MAX_SENDSIZE:cbFrame;
        memcpy(pDmaBuffer,pbFrame, size);

        g_EP2Transfer.dwFlags = USB_REQUEST_DEVICE_TO_HOST;
        g_EP2Transfer.pvBuffer = pDmaBuffer;
        g_EP2Transfer.cbBuffer = size;
        g_EP2Transfer.cbTransferred = 0;
        g_EP2Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; //possible values in usbfntypes.h
        g_EP2Transfer.pvPddData = NULL;
        g_EP2Transfer.pvPddTransferInfo = NULL;

        g_EP2State = TS_SENDING_PACKET;
        g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext,
            rgEndPoints[2].Descriptor.bEndpointAddress & 0x7F,
            &g_EP2Transfer);

        // Poll Interrupt thread until they call notification routine
        while( g_EP2State != TS_IDLE )
        {
            UsbFnInterruptThread( g_pddInterface.pvPddContext );
        }

        pbFrame += size;
#pragma warning(push)
#pragma warning(disable: 4244)
        cbFrame -= size;
#pragma warning(pop)
        if(cbFrame == 0)
            break;
    }
    //OALMSG(1, (L"OEMSerialSendRaw Success\r\n", pbFrame, cbFrame));

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSerialRecvRaw
//
//  Lets get some data from our internal buffer to return to caller. If we don't have enough data,
//  we'll wait until we get it from the USB bus.
//
BOOL OEMSerialRecvRaw(LPBYTE pbFrame, PUSHORT pcbFrame, BOOLEAN bWaitInfinite)
{
    USHORT space;
    int aligntail=0;
    UNREFERENCED_PARAMETER(bWaitInfinite);

    // Make sure OEMSerialRecvRaw is called before USBSerialInit
    if( !g_USBSerialInitialized)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMSerialRecvRaw called before USBSerialInit\r\n"));
        return FALSE;
    }

    // Sanity check parameters
    if( *pcbFrame > RECV_BUFFER_SIZE - rgEndPoints[1].Descriptor.wMaxPacketSize + 1 )
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMSerialRecvRaw caller asking for 0x%X bytes\r\n",*pcbFrame));
        OALMSG(OAL_ERROR, (L"Must increase RECV_BUFFER_SIZE to accomodate request!\r\n"));
        OALMSG(OAL_ERROR, (L"Exiting OEMSerialRecvRaw....\r\n"));
        return FALSE;
    }

    // If our internal buffer cannot satisfy the receive request
    while( *pcbFrame > SPACE_IN_BUFFER(g_RecvBufferHead, g_RecvBufferTail) )
    {
        space = SPACE_IN_BUFFER(g_RecvBufferHead, g_RecvBufferTail);

        // Serial interface won't be able to wrap its pointer...account for this
        if( SPACE_UNTIL_WRAP(g_RecvBufferTail) < 
            rgEndPoints[1].Descriptor.wMaxPacketSize+4)
        {
            memmove( &g_RecvBuffer[0], &g_RecvBuffer[g_RecvBufferHead], space);
            g_RecvBufferHead = 0;
            g_RecvBufferTail = space;
        }

        aligntail=g_RecvBufferTail;
        if(aligntail &0x3)
        {
            aligntail&=~0x3;
            aligntail+=0x4;
        }

        // Package the transfer information into a struct to pass to PDD
        g_EP1Transfer.dwFlags = USB_REQUEST_HOST_TO_DEVICE;
        g_EP1Transfer.pvBuffer = &g_RecvBuffer[aligntail];
        g_EP1Transfer.cbBuffer = rgEndPoints[1].Descriptor.wMaxPacketSize;
        g_EP1Transfer.cbTransferred = 0;
        g_EP1Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; //possible values in usbfntypes.h
        g_EP1Transfer.pvPddData = NULL;
        g_EP1Transfer.pvPddTransferInfo = NULL;

        // Update state machine and kick off transfer
        g_EP1State = TS_RECEIVING_PACKET;
        g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext,
            rgEndPoints[1].Descriptor.bEndpointAddress & 0x7F,
            &g_EP1Transfer);
        while( g_EP1State != TS_IDLE )
        {
            UsbFnInterruptThread( g_pddInterface.pvPddContext );
        }

        if(aligntail!=g_RecvBufferTail)
        {
             memmove( &g_RecvBuffer[g_RecvBufferTail], &g_RecvBuffer[aligntail], g_EP1Transfer.cbTransferred);   
        }

        // Done, make sure amount transferred doesn't overflow caller's UCHAR
        if( (DWORD)(USHORT)g_EP1Transfer.cbTransferred != g_EP1Transfer.cbTransferred )
        {
            ASSERT(!"Amount of transferred data in DWORD too large for EBOOTs UCHAR!");
        }

        // Advance buffer's tail
        g_RecvBufferTail = g_RecvBufferTail + (USHORT)g_EP1Transfer.cbTransferred;
        if( g_RecvBufferTail >= RECV_BUFFER_SIZE )
        {
            ASSERT(!"Serial Wrapper Circular Buffer Overflow error!!!");
        }
    }

    // Now, we definately have enough space to work with
    ASSERT(*pcbFrame <= SPACE_IN_BUFFER(g_RecvBufferHead, g_RecvBufferTail) );

    // Lets give the caller what he wants, advance our buffer's head
    memcpy(pbFrame, &g_RecvBuffer[g_RecvBufferHead], *pcbFrame );
#pragma warning(push)
#pragma warning(disable: 4244)
    g_RecvBufferHead += *pcbFrame;
#pragma warning(pop)
    return TRUE;
}

BOOL USBKitlSerialInit(KITL_SERIAL_INFO *pSerInfo)
{
    pSerInfo->bestSize = EPHighMaxSize;
    return USBSerialInit();
}


UINT16 USBKitlSerialRecv(UINT8 *pch, UINT16 cbRead)
{
    if( !g_USBSerialInitialized)
    {
        KITL_RETAILMSG(OAL_ERROR, ("ERROR: OEMSerialRecvRaw called before USBSerialInit\r\n"));
        return 0;
    }
    if(cbRead >RECV_BUFFER_SIZE)
        cbRead = RECV_BUFFER_SIZE;
    
    //KITL_RETAILMSG(1,("USB Start Recv %d\r\n",cbRead ));
    if(g_EP1State != TS_IDLE )
    {
        UsbFnInterruptThread( g_pddInterface.pvPddContext );
        return 0;
    }

    if( g_EP1Transfer.cbTransferred )
    {
         memcpy(pch, g_RecvBuffer,g_EP1Transfer.cbTransferred );
         cbRead = (UINT16)g_EP1Transfer.cbTransferred;
         g_EP1Transfer.cbTransferred =0;
         //KITL_RETAILMSG(1,("USB Recv %d 0x%x\r\n",cbRead, pch[4]));
         return cbRead;
    }

    //KITL_RETAILMSG(1, ("USB Recv issue transfer\r\n"));
    g_EP1Transfer.dwFlags = USB_REQUEST_HOST_TO_DEVICE;
    g_EP1Transfer.pvBuffer = g_RecvBuffer;
    g_EP1Transfer.cbBuffer = cbRead;
    g_EP1Transfer.cbTransferred = 0;
    g_EP1Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; //possible values in usbfntypes.h
    g_EP1Transfer.pvPddData = NULL;
    g_EP1Transfer.pvPddTransferInfo = NULL;

    // Update state machine and kick off transfer
    g_EP1State = TS_RECEIVING_PACKET;
    g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext,
            rgEndPoints[1].Descriptor.bEndpointAddress & 0x7F,
            &g_EP1Transfer);
  
    return 0;
}

UINT16 USBKitlSerialSend(UINT8 *pch, UINT16 cbSend)
{
    //OALMSGS(1,(L"USB Send %d\r\n",cbSend));
    if(OEMSerialSendRaw(pch, cbSend))
    {
        //OALMSGS(1,(L"USB Send Complete %d\r\n",cbSend));
        return cbSend;
    }
    else
        return (UINT16)-1;
}

//-----------------------------------------------------------------------------
//
// FUNCTION:    USBKitlSerialEnableInts
//
// DESCRIPTION: 
//      This function enables serial interrupts used for the KITL transport.
//
// PARAMETERS:
//      None.
//
// RETURNS:  
//      None.
//
//-----------------------------------------------------------------------------
VOID USBKitlSerialEnableInts(void)
{
}


//-----------------------------------------------------------------------------
//
// FUNCTION:    USBKitlSerialDisableInts
//
// DESCRIPTION: 
//      This function disables serial interrupts used for the KITL transport.
//
// PARAMETERS:
//      None.
//
// RETURNS:  
//      None.
//
//-----------------------------------------------------------------------------
VOID USBKitlSerialDisableInts(void)
{
}


