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
//
// Copyright (C) 2005-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  rndis.c
//
#pragma warning(push)
#pragma warning(disable: 4115 4127 4201 4214)
#include <windows.h>
#include <usbfntypes.h>
#include <usbfn.h>
#undef ZONE_ERROR
#undef ZONE_INIT
#undef ZONE_WARNING
#include <rndismini.h>
#include <linklist.h>
#include <halether.h>
#include <kitl.h>
#include <oal.h>
//#include "bsp.h"
#include "rndis_pdd.h"

#include "common_usbcommon.h"

volatile PULONG  pUsbDevReg;

#define CS_INTERFACE    0x24 // Class Interface
#define iCONF           18
#define CFGLEN          62
#define MANUFACTURER    L"Microsoft"
#define PRODUCT         L"Microsoft RNDIS KITL for FSL HS ARC USB"

#define iEP3            73
#define iEP2            66
#define iEP1            50

#define iINF0           27
#define iINF1           57

INTERRUPT_DATA g_InterruptData = {
    0x01,               // notification
    0x00                // reserved
};
static USB_DEVICE_QUALIFIER_DESCRIPTOR qualifier_desc;
extern void GetRNDISMACAddress(UINT16 mac[3]);
static void RegisterUSBDevice();

static UCHAR g_ucScratch = 0;
static UCHAR g_RndisMacAddress[6] = {0x00, 0x02, 0xb3, 0x92, 0xa8, 0xc4};
static UCHAR g_RndisMacPC[6];
static UCHAR pucVendorID[] = "Microsoft.\0";
static UCHAR pucVendorDescription[] = "Microsoft RNDIS virtual adapter miniport.\0";

int PDDZONE0;
int PDDZONE;
int MDDZONE0;
int MDDZONE;
int MDDZONE2;
int MDDZONE3;
int MDDZONE4;
int MDDZONE5;

USB_DEVICE_REQUEST g_udr;
RNDIS_KITLDEV g_RndisKitlDev;

DATA_WRAPPER g_EP0DataWrapper;
DATA_WRAPPER *g_pEP3DataWrapper = NULL;

static USB_SERIAL_NUMBER gs_SerialNumber;
static UFN_BUS_SPEED usb_dev_speed = BS_HIGH_SPEED;

static const UCHAR gs_pucSupportedLanguage[] = 
{
    0x04,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09, 0x04          //  US English only..
};

static const USB_STRING gs_Manufacturer =
{
    sizeof(MANUFACTURER) + 2,
    USB_STRING_DESCRIPTOR_TYPE,
    MANUFACTURER
};

static const USB_STRING gs_Product =
{
    sizeof(PRODUCT) + 2,
    USB_STRING_DESCRIPTOR_TYPE,
    PRODUCT
};

//static const UCHAR gs_pucUSBDescriptors[] =
static UCHAR gs_pucUSBDescriptors[] = /*mkt may be changed between 0x200 and 0x40*/
{
    ////////////////////////////////////////////////////////////////////////////
    // Standard Device Descriptor
    //

/* 0  */    0x12,                         //  bLength = 18 bytes.
/* 1  */    USB_DEVICE_DESCRIPTOR_TYPE, //  bDescriptorType = DEVICE
/* 2  */    0x00, 0x02,                 //  bcdUSB          = 2.0
/* 4  */    0x02,                       //  bDeviceClass    = Communication Device Class
/* 5  */    0x00,                       //  bDeviceSubClass = Unused at this time.
/* 6  */    0x00,                       //  bDeviceProtocol = Unused at this time.
/* 7  */    EP0MaxSize,                 //  bMaxPacketSize0 = EP0 buffer size..
/* 8  */    0x5E, 0x04,                 //  idVendor        = Microsoft Vendor ID.
/* 10 */    0x01, 0x03,                 //  idProduct       = Microsoft generic RNDISMINI Product ID.
/* 12 */    0x00, 0x00,                 //  bcdDevice       = 0.1
/* 14 */    0x01,                       //  iManufacturer   = OEM should fill this..
/* 15 */    0x02,                       //  iProduct        = OEM should fill this..
/* 16 */    0x00,                       //  iSerialNumber   = OEM should fill this..
/* 17 */    0x01,                       //  bNumConfigs     = 1 


    ////////////////////////////////////////////////////////////////////////////
    //  RNDIS requires only one configuration as follows..
    //  And we have 2 interfaces (Communication Class if & Dataclass if).
    //

/* 18 */    9,                                  //  bLength         = 9 bytes.
/* 19 */    USB_CONFIGURATION_DESCRIPTOR_TYPE,  //  bDescriptorType = CONFIGURATION
/* 20 */    0x3e, 0x00,                 //  wTotalLength    = From offset 18 to end <---
/* 22 */    0x02,                       //  bNumInterfaces  = 2 (RNDIS spec).
/* 23 */    0x01,                       //  bConfValue      = 1
/* 24 */    0x00,                       //  iConfiguration  = unused.
/* 25 */    0xc0,                       //  bmAttributes    = Self-Powered.
/* 26 */    0x01,                       //  MaxPower        = 2mA



        
    ////////////////////////////////////////////////////////////////////////////
    //  Communication Class INTERFACE descriptor.
    //  RNDIS specifies 2 endpoints, EP0 & notification element (interrupt)
    //

/* 27 */    9,                          //  bLength         = 9 bytes.
/* 28 */    USB_INTERFACE_DESCRIPTOR_TYPE, //  bDescriptorType = INTERFACE
/* 29 */    0x00,                       //  bInterfaceNo    = 0
/* 30 */    0x00,                       //  bAlternateSet   = 0
/* 31 */    0x01,                       //  bNumEndPoints   = 1 (RNDIS spec)
/* 32 */    0x02,                       //  bInterfaceClass = Comm if class (RNDIS spec)
/* 33 */    0x02,                       //  bIfSubClass     = Comm if sub        (ditto)
/* 34 */    0xff,                       //  bIfProtocol     = Vendor specific    (ditto)
/* 35 */    0x00,                       //  iInterface      = unused.

    ////////////////////////////////////////////////////////////////////////////
    //  Functional Descriptors for Communication Class Interface
    //  per RNDIS spec.
    //

/* 36 */    5,                  //  bFunctionLength = 5 bytes.
/* 37 */    CS_INTERFACE,       //  bDescriptorType = Class Interface
/* 38 */    0x01,               //  bDescSubType    = CALL MGMT func descriptor.
/* 39 */    0x00,               //  bmCapabilities  = See sect 5.2.3.2 USB CDC
/* 40 */    0x01,               //  bDataInterface  = 1 data class i/f.
        

/* 41 */    4,                  //  bFunctionLength = 5 bytes.
/* 42 */    CS_INTERFACE,       //  bDescriptorType = Class Interface
/* 43 */    0x02,               //  bDescSubType    = ABSTRACT ctrl mgmt desc.
/* 44 */    0x00,               //  bmCapabilities  = See sect 5.2.3.3 USB CDC

/* 45 */    5,                  //  bFunctionLength = 5 bytes.
/* 46 */    CS_INTERFACE,       //  bDescriptorType = Class Interface.
/* 47 */    0x02,               //  bDescSubType    = UNION func descriptor.
/* 48 */    0x00,               //  bMasterIf       = i/f 0 is the master if.
/* 49 */    0x01,               //  bSlaveIf        = i/f 1 is the slave if.

    ////////////////////////////////////////////////////////////////////////////
    //  Endpoint descriptors for Communication Class Interface
    //

/* 50 */    7,                  //  bLength         = 7 bytes.
/* 51 */    USB_ENDPOINT_DESCRIPTOR_TYPE, //  bDescriptorType = ENDPOINT
/* 52 */    (0x01 | USB_ENDPOINT_DIRECTION_MASK),               //  bEndpointAddr   
/* 53 */    USB_ENDPOINT_TYPE_INTERRUPT,               //  bmAttributes  
/* 54 */    EPIntMaxSize, 0x00,         //  wMaxPacketSize
/* 56 */    1,                   //  bInterval      



    ////////////////////////////////////////////////////////////////////////////
    //  Data Class INTERFACE descriptor.
    //

/* 57 */    9,                  //  bLength         = 9 bytes.
/* 58 */    USB_INTERFACE_DESCRIPTOR_TYPE, //  bDescriptorType = INTERFACE
/* 59 */    0x01,               //  bInterfaceNo    = 1
/* 60 */    0x00,               //  bAlternateSet   = 0
/* 61 */    0x02,               //  bNumEndPoints   = 2 (RNDIS spec)
/* 62 */    0x0A,               //  bInterfaceClass = Data if class (RNDIS spec)
/* 63 */    0x00,               //  bIfSubClass     = unused.
/* 64 */    0x00,               //  bIfProtocol     = unused.
/* 65 */    0x00,               //  iInterface      = unused.


    ////////////////////////////////////////////////////////////////////////////
    //  Endpoint descriptors for Data Class Interface
    //

/* 66 */    7,                  //  bLength         = 7 bytes.
/* 67 */    USB_ENDPOINT_DESCRIPTOR_TYPE, //  bDescriptorType = ENDPOINT [IN]
/* 68 */    (0x02 | USB_ENDPOINT_DIRECTION_MASK),               //  bEndpointAddr   = IN -- EP2
/* 69 */    USB_ENDPOINT_TYPE_BULK,               //  bmAttributes    = BULK
/* 70 */    0x00,0x02,     //  wMaxPacketSize
/* 72 */    0,                  //  bInterval       = ignored for BULK.

/* 73 */    7,                  //  bLength         = 7 bytes.
/* 74 */    USB_ENDPOINT_DESCRIPTOR_TYPE, //  bDescriptorType = ENDPOINT 
/* 75 */    (0x03 ),               //  bEndpointAddr   
/* 76 */    USB_ENDPOINT_TYPE_BULK,        
/* 77 */    0x00,0x02,     //  wMaxPacketSize
/* 79 */    0                   //  bInterval   

};  //  gs_pucUSBDescriptors[]

const TCHAR DigitTable[] = TEXT("0123456789ABCDEF");

// when registering with the USBFN PDD layer, the PDD will fill in this
// structure with appropriate data and function pointers
UFN_MDD_INTERFACE_INFO g_mddInterface; 

// contains the MDD callback functions to be called by the PDD layer
UFN_PDD_INTERFACE_INFO g_pddInterface; 

// current transfer information for each endpoint
STransfer g_EP0Transfer; // EP0
STransfer g_EP1Transfer; // INT in
STransfer g_EP2Transfer; // Bulk in
STransfer g_EP3Transfer; // Bulk Out

enum DEVICE_STATE g_devState = DS_DETACHED;
enum TRANSFER_STATE g_EP0TransferState = TS_IDLE;
enum TRANSFER_STATE g_EP2TransferState = TS_IDLE;

void SetRNDISSerialNumber(void);
void SetRNDISMACAddress(void);


BOOL InitializeHardware(); // performs the hardware initialization (clock, I2C, OTG transceiver, OTG controller)

extern DWORD InterruptThread(VOID *pPddContext);

// after a message is successfully sent, this function will be called to indicate this fact to the KITL MDD
static VOID PDD_SendNotification(EP0_REQUEST *pRequest, PVOID pvUser)
{
    DATA_WRAPPER *pWrapper = (DATA_WRAPPER *)pvUser;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pRequest);

    if (pWrapper != g_RndisKitlDev.pEP0DataWrapper) {
        ASSERT(FALSE);
        return;
    }
    MddSendRndisMessageComplete(g_RndisKitlDev.pEP0DataWrapper);
    g_RndisKitlDev.pEP0DataWrapper = NULL;
}

//static char ep0_buf[0x1000];
extern char g_rndis_ep0_buf[];
extern char g_rndis_ep2_buf[];
extern char g_rndis_ep3_buf[];

// after a message is successfully received, this function will be called to indicate this fact to the KITL MDD
static VOID PDD_RecvNotification(EP0_REQUEST *pRequest, PVOID pvUser)
{
    DATA_WRAPPER *pWrapper = (DATA_WRAPPER *)pvUser;

    if (pWrapper != &g_RndisKitlDev.EP0DataWrapper) {
        ASSERT(FALSE);
        return;
    }
    //g_RndisKitlDev.EP0DataWrapper.pucData = OALCAtoUA(pRequest->pucData);
    g_RndisKitlDev.EP0DataWrapper.pucData = OALCAtoUA(g_rndis_ep0_buf);
    g_RndisKitlDev.EP0DataWrapper.dwDataSize = pRequest->dwActualSize;
    MddIndicateRndisMessage(&(g_RndisKitlDev.EP0DataWrapper));

}

// computes the descriptor data for a GET_DESCRIPTOR setup request
BOOL PDD_GetDescriptor(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest)
{
    UCHAR *pucData = NULL;
    WORD wLength = 0;
    WORD wType = pUdr->wValue;
    BOOL fRet = TRUE;   
    

    switch (HIBYTE(wType)) {
        case USB_DEVICE_DESCRIPTOR_TYPE:
            
            pucData = (UCHAR *)gs_pucUSBDescriptors;
            wLength = gs_pucUSBDescriptors[0];
            break;

        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
            pucData = (UCHAR *)&gs_pucUSBDescriptors[iCONF];
            wLength = CFGLEN;;
            break;

        case USB_STRING_DESCRIPTOR_TYPE:
            switch (LOBYTE(wType)) {
                case 0x00:
                    pucData = (UCHAR *)gs_pucSupportedLanguage;
                    wLength = gs_pucSupportedLanguage[0];
                    break;

                case 0x01:
                    pucData = (UCHAR *)&gs_Manufacturer;
                    wLength = gs_Manufacturer.ucbLength;
                    break;

                case 0x02:
                    pucData = (UCHAR *)&gs_Product;
                    wLength = gs_Product.ucbLength;
                    break;

                case 0x03:
                    pucData = (UCHAR *)&gs_SerialNumber;
                    wLength = gs_SerialNumber.ucbLength;
                    break;

                default:
                    OALMSG(OAL_ERROR, (L"*** Unknown STRING index %d\r\n", LOBYTE(wType)));
                    fRet = FALSE;
                    break;
            }
            break;
        case USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE:
            if( LOBYTE(wType) != 0)
            {
                break;
            }
            wLength = sizeof(USB_DEVICE_QUALIFIER_DESCRIPTOR);
            memcpy( &qualifier_desc,gs_pucUSBDescriptors, wLength);
             // Copy over the values that do match
            qualifier_desc.bLength = (BYTE)wLength;
            qualifier_desc.bDescriptorType = (BYTE)USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE;
            qualifier_desc.bNumConfigurations = 1;
            qualifier_desc.bReserved = 0;
            pucData = (UCHAR *)&qualifier_desc;
            break;

        default:
            OALMSG(OAL_ERROR, (L"*** Unknown GET_DESCRIPTOR request:0x%x\r\n", HIBYTE(wType)));
            fRet = FALSE;
            break;
    }

    if (fRet) {
        pRequest->eDir = EP0In;
        pRequest->pucData = pucData;
        pRequest->dwExpectedSize = pUdr->wLength;
        pRequest->dwActualSize = min(wLength, pUdr->wLength);
        pRequest->pfnNotification = NULL;       // notification not required
        pRequest->pvUser = NULL;
    }
    return fRet;
}

// parses the setup packet to determine if it is one of the two standard RNDIS requests
// and sets up a send or receive transfer request on EP0 if necessary
BOOL PDD_DoVendorCommand(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest)
{
    BOOL fRet = TRUE;

    //
    // RNDIS spec:: we should only be getting:
    // - SEND_ENCAPSULATED_COMMAND  or
    // - GET_ENCAPSULATED_RESPONSE.
    // So let's just look for these two, and bail out if it does
    //not match..
    //
    switch (pUdr->bRequest) {
        case SEND_ENCAPSULATED_COMMAND:
            pRequest->eDir = EP0Out;
            pRequest->pucData = (UCHAR *)(g_RndisKitlDev.EP0RxBuffer);
            pRequest->dwExpectedSize = pUdr->wLength;
            pRequest->dwActualSize = pUdr->wLength;
            pRequest->pfnNotification = PDD_RecvNotification;
            pRequest->pvUser = &(g_RndisKitlDev.EP0DataWrapper);
            break;

        case GET_ENCAPSULATED_RESPONSE:
            pRequest->eDir = EP0In;
            if (!IsListEmpty(&(g_RndisKitlDev.listTxRndisMessageQueue))) {
                PLIST_ENTRY pLink;
                PDATA_WRAPPER pDataWrapper;

                pLink = RemoveHeadList(&(g_RndisKitlDev.listTxRndisMessageQueue));
                pDataWrapper = CONTAINING_RECORD(pLink, DATA_WRAPPER, Link);
                //KITLDEBUGMSG(MDDZONE, ("<0x%X\r\n", pDataWrapper));
                pRequest->pucData = pDataWrapper->pucData;
                pRequest->dwExpectedSize = pUdr->wLength;
                pRequest->dwActualSize = pDataWrapper->dwDataSize;
                pRequest->pfnNotification = PDD_SendNotification;
                pRequest->pvUser = pDataWrapper;

                g_RndisKitlDev.pEP0DataWrapper = pDataWrapper;
            } else {
                //
                // RNDIS spec says if we have no data to send then 
                // send one byte packet set to 00 instead of stalling the 
                // pipe.
                //
                pRequest->pucData = &g_ucScratch;
                pRequest->dwActualSize = sizeof(UCHAR);
                pRequest->dwExpectedSize = pUdr->wLength;
                pRequest->pfnNotification = NULL;
                pRequest->pvUser = NULL;
            }
            break;

        default:
            fRet = FALSE;
            break;
    }
    return fRet;
}

// sets up a packet receive transfer on EP3 (BULK OUT)
VOID setupDataOutTransfer()
{


    // allocate data transfer for EP3
    if (!g_pEP3DataWrapper) {
        g_pEP3DataWrapper = MDDAllocDataWrapper();
        if (!g_pEP3DataWrapper) {
            OALMSG(OAL_ERROR, (L"No mem for RX DataWrapper\r\n"));
            return;
        }

        g_pEP3DataWrapper->pucData = OALCAtoUA(g_rndis_ep3_buf);

        if (!g_pEP3DataWrapper->pucData) {
            OALMSG(OAL_ERROR, (L"No mem for RX data\r\n"));
            MDDFreeDataWrapper(g_pEP3DataWrapper);
            return;
        }
        g_pEP3DataWrapper->dwDataSize = MAX_INCOMING_BUFFER;
    }

    g_EP3Transfer.pvBuffer = g_pEP3DataWrapper->pucData;    
    g_EP3Transfer.cbBuffer = g_pEP3DataWrapper->dwDataSize;
    g_EP3Transfer.cbTransferred = 0;
    g_EP3Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; // Possible values are in usbfntypes.h
    g_EP3Transfer.pvPddData = NULL;
    g_EP3Transfer.pvPddTransferInfo = NULL;
    g_EP3Transfer.dwFlags = USB_REQUEST_HOST_TO_DEVICE;
    g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext, 3, &g_EP3Transfer );

}

// parses the setup packet received on EP0
BOOL HandleSetupPacket(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest)
{
    if (pUdr->bmRequestType & (USB_REQUEST_CLASS | USB_REQUEST_VENDOR)) {
        if (PDD_DoVendorCommand(pUdr, pRequest)) {
            // do nothing
        } else {
            OALMSG(OAL_ERROR, (
                L"**** Unhandled verndor command 0x%x\r\n", pUdr->bRequest
            ));
            pRequest->eDir = EP0Setup;
            pRequest->pucData = NULL;
            pRequest->dwExpectedSize = 0;
            pRequest->dwActualSize = 0;
            pRequest->pfnNotification = NULL;
            pRequest->pvUser = NULL;
        }
        return TRUE;
    }

    // standard chapter 9 commands
    pRequest->eDir = EP0Setup;
    pRequest->pucData = NULL;
    pRequest->dwExpectedSize = 0;
    pRequest->dwActualSize = 0;
    pRequest->pfnNotification = NULL;
    pRequest->pvUser = NULL;

    //KITLOutputDebugString("HandleSetupPacket,pUdr->bRequest is %d \n",pUdr->bRequest);
    switch(pUdr->bRequest) {
        case USB_REQUEST_GET_STATUS:
            if (pUdr->bmRequestType == 0x82) {
                // TODO: handle this
                OALMSG(OAL_WARN, (L"***RequestType==0x82\r\n"));
                // check for the stall bit
            }
            break;

        case USB_REQUEST_CLEAR_FEATURE:
            if (pUdr->bmRequestType == 0x02) {
                // TODO: handle this
                OALMSG(OAL_WARN, (L"***RequestType==0x02\r\n"));
            }
            break;

        case USB_REQUEST_SET_FEATURE:
            if (pUdr->bmRequestType == 0x02) {
                // TODO: handle this
                OALMSG(OAL_WARN, (L"***RequestType==0x02\r\n"));
            }
            break;

        case USB_REQUEST_SET_ADDRESS:
            // TODO: handle this
            g_pddInterface.pfnSetAddress(g_pddInterface.pvPddContext,(BYTE)(pUdr->wValue));
            break;

        case USB_REQUEST_GET_DESCRIPTOR:
            if (PDD_GetDescriptor(pUdr, pRequest)) {
                // do nothing
            } else {
                OALMSG(OAL_ERROR, (
                    L"*** UnHandled GET_DESCRIPTOR request:0x%x\r\n",
                    pUdr->wValue
                ));
            }
            break;

        case USB_REQUEST_SET_DESCRIPTOR:
            // TODO: handle this
            break;

        case USB_REQUEST_GET_CONFIGURATION:
            // TODO: handle this
            break;

        case USB_REQUEST_SET_CONFIGURATION:            
        {
           USB_ENDPOINT_DESCRIPTOR        *pUsbEP1         = (PVOID)&gs_pucUSBDescriptors[iEP1];
                 USB_ENDPOINT_DESCRIPTOR        *pUsbEP2         = (PVOID)&gs_pucUSBDescriptors[iEP2];
           USB_ENDPOINT_DESCRIPTOR        *pUsbEP3         = (PVOID)&gs_pucUSBDescriptors[iEP3]; 

           g_pddInterface.pfnInitEndpoint( g_pddInterface.pvPddContext, 
                   1, usb_dev_speed, pUsbEP1, NULL, 0, 0, 0);        
           g_pddInterface.pfnInitEndpoint( g_pddInterface.pvPddContext, 
                   2, usb_dev_speed, pUsbEP2, NULL, 0, 0, 0);
           g_pddInterface.pfnInitEndpoint( g_pddInterface.pvPddContext, 
                        3, usb_dev_speed, pUsbEP3, NULL, 0, 0, 0);

    
            }
       // setup RX transfer on data out
           setupDataOutTransfer();
            break;

        case USB_REQUEST_GET_INTERFACE:
            // TODO: handle this
            break;

        case USB_REQUEST_SET_INTERFACE:
            // TODO: handle this
            break;

        case USB_REQUEST_SYNC_FRAME:
            // TODO: handle this
            break;

        default:
            OALMSG(OAL_WARN, (
                L"*** Unknown request 0x%x\r\n", pUdr->bRequest
            ));
    }
    return TRUE;
}

// indicates to the KITL MDD that a packet has been sent
void TxComplete(void)
{
    RNDIS_KITLDEV *pRndisKitlDev = &g_RndisKitlDev;

    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"SendPacket request completed! [%d bytes]\r\n", 
        g_EP2Transfer.cbTransferred 
    ));
    MddSendRndisPacketComplete(pRndisKitlDev->pTxDataWrapper);

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"Done\r\n"));
    pRndisKitlDev->pTxDataWrapper = NULL;
}


// The USB FN PDD will call this function to notify it of various events (data sent/received, 
// setup packet received, device addressed, reset, etc.
BOOL WINAPI NotifyHandler (
    PVOID pvMddContext,
    DWORD dwMsg,
    DWORD dwParam
    )
{
    USB_DEVICE_REQUEST request;
    EP0_REQUEST EP0Request;
    STransfer *pTransfer;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pvMddContext);

    switch( dwMsg )
    {
    case UFN_MSG_SETUP_PACKET:
        request = *(USB_DEVICE_REQUEST*)(void*)dwParam;
        g_udr = request;

        if( !HandleSetupPacket( &g_udr, &EP0Request ) )
        {
            OALMSG(OAL_ERROR, (L"ERROR PARSING SETUP PACKET\r\n"));
        }

        memcpy(&g_RndisKitlDev.EP0Request, &EP0Request, sizeof(EP0Request));
        g_RndisKitlDev.EP0Request.dwProcessed = 0;
        g_RndisKitlDev.EP0Request.fCompleted = FALSE;

        if (g_RndisKitlDev.EP0Request.eDir != EP0Setup) {
            g_EP0DataWrapper.pucData = g_RndisKitlDev.EP0Request.pucData;
            g_EP0DataWrapper.dwDataSize = g_RndisKitlDev.EP0Request.dwActualSize;
            
            // g_EP2Transfer.dwCallerPermissions;
            g_EP0Transfer.pvBuffer = g_EP0DataWrapper.pucData;
            // g_EP2Transfer.dwBufferPhysicalAddress; // not used
            g_EP0Transfer.cbBuffer = g_EP0DataWrapper.dwDataSize;
            g_EP0Transfer.cbTransferred = 0;
            g_EP0Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; // Possible values are in usbfntypes.h
            g_EP0Transfer.pvPddData = NULL;
            g_EP0Transfer.pvPddTransferInfo = NULL;

            if (g_RndisKitlDev.EP0Request.eDir == EP0In) {
                OALMSG(OAL_ETHER&&OAL_FUNC, (L"Got SETUP IN packet\r\n"));
                g_EP0Transfer.dwFlags = USB_REQUEST_DEVICE_TO_HOST;
                g_EP0TransferState = TS_SENDING_MESSAGE;
                g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext, 0, &g_EP0Transfer );
            }
            else
            {
                OALMSG(OAL_ETHER&&OAL_FUNC, (L"Got SETUP OUT packet.\r\n"));
                g_EP0Transfer.dwFlags = 0;
                g_EP0TransferState = TS_RECEIVING_MESSAGE;
                g_EP0Transfer.pvBuffer = OALCAtoUA(g_rndis_ep0_buf);
                g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext, 0, &g_EP0Transfer );
            }
        }
        else
        {
            OALMSG(OAL_ETHER&&OAL_FUNC, (
                L"Got SETUP <NO DATA: %x %x %x %x %x> packet.\r\n",
                g_udr.bmRequestType, g_udr.bRequest, g_udr.wValue, g_udr.wIndex,
                 g_udr.wLength 
            ));

            // Send the SETUP ACK packet
            g_EP0TransferState = TS_IDLE;
            g_pddInterface.pfnSendControlStatusHandshake( g_pddInterface.pvPddContext, 0 );
        }
        break;

    case UFN_MSG_TRANSFER_COMPLETE: 
        pTransfer = (STransfer *)dwParam;
        if( pTransfer == &g_EP0Transfer )
        {
            switch( g_EP0TransferState )
            {
            case TS_RECEIVING_MESSAGE:
                g_EP0TransferState = TS_IDLE;
                OALMSG(OAL_ETHER&&OAL_FUNC, (L"Done receiving data.\r\n"));

                g_pddInterface.pfnSendControlStatusHandshake( g_pddInterface.pvPddContext, 0 );

                if (g_RndisKitlDev.EP0Request.pfnNotification) {
                    OALMSG(OAL_ETHER&&OAL_FUNC, (L"Received a message!\r\n"));
                    g_RndisKitlDev.EP0Request.pfnNotification(&g_RndisKitlDev.EP0Request, g_RndisKitlDev.EP0Request.pvUser);
                }
          memset(&g_RndisKitlDev.EP0Request, 0, sizeof (g_RndisKitlDev.EP0Request));
                break;
            case TS_SENDING_MESSAGE:
                OALMSG(OAL_ETHER&&OAL_FUNC, (L"Done sending data.\r\n"));

                g_EP0TransferState = TS_IDLE;
                g_pddInterface.pfnSendControlStatusHandshake( g_pddInterface.pvPddContext, 0 );

                if (g_RndisKitlDev.EP0Request.pfnNotification) {
                    OALMSG(OAL_ETHER&&OAL_FUNC, (L"SendMessage request completed!\r\n"));
                    g_RndisKitlDev.EP0Request.pfnNotification(&g_RndisKitlDev.EP0Request, g_RndisKitlDev.EP0Request.pvUser);

                    //memset(&g_RndisKitlDev.EP0Request, 0, sizeof (g_RndisKitlDev.EP0Request));
                    g_RndisKitlDev.EP0Request.pfnNotification = NULL;
                    g_RndisKitlDev.EP0Request.pvUser = NULL;
                }
                break;
            }
        }
        else if( pTransfer == &g_EP3Transfer )
        {
            // BULK OUT data received on EP1
            OALMSG(OAL_ETHER&&OAL_FUNC, (
                L"Received a packet [%d bytes]!\r\n", pTransfer->cbTransferred
            ));
            // notify the MDD
            g_pEP3DataWrapper->dwDataSize = pTransfer->cbTransferred;
            MddIndicateRndisPacket(g_pEP3DataWrapper);
            g_pEP3DataWrapper = NULL;
            // setup another transfer
            setupDataOutTransfer( );
        }
        else if( pTransfer == &g_EP2Transfer )
        {
            if( g_EP2TransferState == TS_SENDING_PACKET )
            {
                int mkt = EPHSMaxSize;
                  if( usb_dev_speed == BS_FULL_SPEED)
                  {
                    mkt = EPFSMaxSize;
                  }
                  
                if( ( g_EP2Transfer.cbTransferred % mkt ) == 0 )
                {

                    // g_EP2Transfer.dwCallerPermissions;
                    g_EP2Transfer.dwFlags = USB_REQUEST_DEVICE_TO_HOST;
#ifdef USE_1BYTE_TERMINATING_PACKETS
                    g_EP2Transfer.pvBuffer = &g_ucScratch;
                    g_EP2Transfer.cbBuffer = sizeof(UCHAR);
#else
                    g_EP2Transfer.pvBuffer = NULL;
                    g_EP2Transfer.cbBuffer = 0;
#endif
                    // g_EP2Transfer.dwBufferPhysicalAddress; // not used
                    g_EP2Transfer.cbTransferred = 0;
                    g_EP2Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; // Possible values are in usbfntypes.h
                    g_EP2Transfer.pvPddData = NULL;
                    g_EP2Transfer.pvPddTransferInfo = NULL;


                    OALMSG(OAL_ETHER&&OAL_FUNC, (
                        L"Sending NULL terminator for the packet...\r\n"
                    ));
                    g_EP2TransferState = TS_SENDING_PACKET_TERMINATOR;

                    g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext, 2, &g_EP2Transfer );
                }
                else
                {
                    g_EP2TransferState = TS_IDLE;
                    TxComplete();
                }
            }
            else if( g_EP2TransferState == TS_SENDING_PACKET_TERMINATOR )
            {
                ASSERT( g_EP2Transfer.dwUsbError == UFN_NO_ERROR );
                OALMSG(OAL_ETHER&&OAL_FUNC, (L"SendPacket NULL terminator sent!\r\n"));
                g_EP2TransferState = TS_IDLE;

                TxComplete();
            }
            else
            {
                OALMSG(OAL_ERROR, (L"Unexpected EP2 transfer state!"));
            }
        }
        else if( pTransfer == &g_EP1Transfer )
        {
          // OALMSG(OAL_ERROR, (L"EP1 Interrupt successfully sent!\r\n"));
        }
        else
        {
            //OALMSG(OAL_ERROR, (L"INVALID TRANSFER!\r\n"));
        }
        break;

    case UFN_MSG_BUS_EVENTS:
        switch( dwParam )
        {
        case UFN_DETACH:
            g_devState = DS_DETACHED;
            break;
        case UFN_ATTACH:
            g_devState = DS_ATTACHED;
            break;
        case UFN_RESET:
            g_devState = DS_DEFAULT;
            g_EP0TransferState = TS_IDLE;
            g_EP2TransferState = TS_IDLE;

#if 0
            OALMSG (OAL_WARN, (TEXT("RNDISFN:: UFN_RESET received.\r\n")));                
            //ClosePipes(pContext);
            
            //AbortAllTransfers(pContext);
            //MddDisconnect();
            // try to AbortTransfer on EP0
            g_pddInterface.pfnAbortTransfer(g_pddInterface.pvPddContext, 0, &g_EP0Transfer);
            g_pddInterface.pfnAbortTransfer(g_pddInterface.pvPddContext, 2, &g_EP2Transfer);
#endif  //  0
            break;
        case UFN_SUSPEND:
            g_devState = DS_ATTACHED;
            break;
        case UFN_RESUME:
            g_devState = DS_ATTACHED;
            break;
        default:
            ASSERT(!"Unexpected bus event!");
        }
        break;

    case UFN_MSG_BUS_SPEED:
     usb_dev_speed = dwParam;
     {
        USB_ENDPOINT_DESCRIPTOR        *pUsbEP1         = (PVOID)&gs_pucUSBDescriptors[iEP1];
        USB_ENDPOINT_DESCRIPTOR        *pUsbEP2         = (PVOID)&gs_pucUSBDescriptors[iEP2];
            USB_ENDPOINT_DESCRIPTOR        *pUsbEP3         = (PVOID)&gs_pucUSBDescriptors[iEP3];
        if( usb_dev_speed == BS_FULL_SPEED)
        {
            pUsbEP1->bInterval = 1; //equal to 1 ms
            pUsbEP2->wMaxPacketSize = EPFSMaxSize;
            pUsbEP3->wMaxPacketSize = EPFSMaxSize;
        }
        else
        {
            pUsbEP1->bInterval = 4; // 125us * ( 2 ^ (4-1)) = 1ms
            pUsbEP2->wMaxPacketSize = EPHSMaxSize;
            pUsbEP3->wMaxPacketSize = EPHSMaxSize;
        }
     }
     RegisterUSBDevice();
        break;

    case UFN_MSG_SET_ADDRESS:
        if( dwParam )
        {
            g_devState = DS_ADDRESSED;          
        }
        else
        {
            g_devState = DS_DEFAULT;
        }
        // we should call pfnSetAddress here
        break;

    default:
        ASSERT(!"Unexpected Msg in NotifyHandler!");
    }

    return FALSE;
}

// register the USB device, setup the non zero endpoints
void RegisterUSBDevice()
{
    static USB_DEVICE_DESCRIPTOR deviceDescriptor;
    static UFN_ENDPOINT rgEndPoints[3];
    static UFN_INTERFACE Interface;
    static UFN_CONFIGURATION deviceConfiguration;


    static USB_ENDPOINT_DESCRIPTOR EndpointZeroDesc = {
        7,                              //  bLength
        USB_ENDPOINT_DESCRIPTOR_TYPE,   //  DescriptorType;
        0,                              //  bEndpointAddress;
        USB_ENDPOINT_TYPE_CONTROL,      //  bmAttributes;
        EP0MaxSize,                     //  wMaxPacketSize;
        0,                              //  bInterval;
    };
    

    // we also need to apply IsEndpointSupportable to initial endpoint in some USBFN driver
    USB_CONFIGURATION_DESCRIPTOR   *pUsbConfDesc    = (PVOID)&gs_pucUSBDescriptors[iCONF];
    USB_INTERFACE_DESCRIPTOR       *pUsbInfDesc0    = (PVOID)&gs_pucUSBDescriptors[iINF0];
    USB_INTERFACE_DESCRIPTOR       *pUsbInfDesc1    = (PVOID)&gs_pucUSBDescriptors[iINF1];

    USB_ENDPOINT_DESCRIPTOR        *pUsbEP0         = (PVOID)&EndpointZeroDesc;
    USB_ENDPOINT_DESCRIPTOR        *pUsbEP1         = (PVOID)&gs_pucUSBDescriptors[iEP1];
    USB_ENDPOINT_DESCRIPTOR        *pUsbEP2         = (PVOID)&gs_pucUSBDescriptors[iEP2];
    USB_ENDPOINT_DESCRIPTOR        *pUsbEP3         = (PVOID)&gs_pucUSBDescriptors[iEP3];

    int mkt = EPHSMaxSize;
    if( usb_dev_speed == BS_FULL_SPEED)
    {
     mkt = EPFSMaxSize;
    }


    // Enpoint 0, interface 0
    g_pddInterface.pfnIsEndpointSupportable( g_pddInterface.pvPddContext, 
                                        0, usb_dev_speed, pUsbEP0,
                                        0, 0, 0);

    // Enpoint 1, interface 0
    g_pddInterface.pfnIsEndpointSupportable( g_pddInterface.pvPddContext, 
                                        1, usb_dev_speed, pUsbEP1, 
                                        pUsbConfDesc->bConfigurationValue,
                                        pUsbInfDesc0->bInterfaceNumber, pUsbInfDesc0-> bAlternateSetting);

    // Enpoint 2, interface 1
    g_pddInterface.pfnIsEndpointSupportable( g_pddInterface.pvPddContext, 
                                        2, usb_dev_speed, pUsbEP2, 
                                        pUsbConfDesc->bConfigurationValue,
                                        pUsbInfDesc1->bInterfaceNumber, pUsbInfDesc1-> bAlternateSetting);

    // Enpoint 3, interface 1
    g_pddInterface.pfnIsEndpointSupportable( g_pddInterface.pvPddContext, 
                                        3, usb_dev_speed, pUsbEP3, 
                                        pUsbConfDesc->bConfigurationValue,
                                        pUsbInfDesc1->bInterfaceNumber, pUsbInfDesc1-> bAlternateSetting);


    deviceConfiguration.pInterfaces = &Interface;
    deviceConfiguration.Descriptor.bNumInterfaces = 1;
    Interface.pEndpoints = rgEndPoints;
    Interface.Descriptor.bNumEndpoints = 3;
    rgEndPoints[0].Descriptor.bEndpointAddress = 0x81; // EP1, IN (INT)
    rgEndPoints[0].Descriptor.wMaxPacketSize = EPIntMaxSize;
    rgEndPoints[0].Descriptor.bmAttributes = 0;
    rgEndPoints[1].Descriptor.bEndpointAddress = 0x82; // EP2, IN (BULK)
    rgEndPoints[1].Descriptor.wMaxPacketSize = (USHORT)mkt;
    rgEndPoints[1].Descriptor.bmAttributes = 0;
    rgEndPoints[2].Descriptor.bEndpointAddress = 0x03; // EP3, OUT (BULK)
    rgEndPoints[2].Descriptor.wMaxPacketSize = (USHORT)mkt;
    rgEndPoints[2].Descriptor.bmAttributes = 0;
    deviceDescriptor.bMaxPacketSize0 = EP0MaxSize;

    
    // We assume knowledge of the RegisterDevice function implementation and pass very simplified arguments
    g_pddInterface.pfnRegisterDevice( g_pddInterface.pvPddContext,                                        
                                        &deviceDescriptor, &deviceConfiguration, NULL, //HS
                         &deviceDescriptor, &deviceConfiguration, NULL, //FS
                                        NULL, 0 );


}

// initialize the USB KITL driver
BOOL PDDInit(
  RNDIS_PDD_CHARACTERISTICS* pRndisPddCharacteristics,
  PBYTE pBaseAddress
)
{
    PDDZONE0 = 0;
    PDDZONE = 0;
    MDDZONE0 = 0;
    MDDZONE = 1;
    MDDZONE2 = 1;
    MDDZONE3 = 1;
    MDDZONE4 = 1;
    MDDZONE5 = 0;

    g_mddInterface.dwVersion = 1;
    g_mddInterface.pfnNotify = NotifyHandler;

    memset( &g_pddInterface, 0, sizeof(g_pddInterface) );
    g_pddInterface.dwVersion = 1;

    // configure clock, I2C cotroller, USB OTG transceiver and USB OTG controller
    //InitializeHardware();

    // initialize usb function unit
    if( UfnPdd_Init( NULL, 
        (PVOID)0x11223344, //  assigned some dummy value or we'll get hanged.
        &g_mddInterface, &g_pddInterface ) != ERROR_SUCCESS )
    {
        OALMSG(OAL_ERROR, (L"UfnPdd_Init failed!\r\n"));
        return FALSE;
    }

    // register device
    //RegisterUSBDevice();

    // attach device to USB bus
    g_pddInterface.pfnStart( g_pddInterface.pvPddContext );

    SetRNDISMACAddress();
    SetRNDISSerialNumber();

    //
    // Everything ok, fill up our characteristics.
    //
    memset(pRndisPddCharacteristics, 0x00, sizeof(RNDIS_PDD_CHARACTERISTICS));

    pRndisPddCharacteristics->SendRndisMessageHandler = PDD_SendRndisMessage;
    pRndisPddCharacteristics->SendRndisPacketHandler = PDD_SendRndisPacket;
    pRndisPddCharacteristics->SetHandler = PDD_Set;
    pRndisPddCharacteristics->GetHandler = PDD_Get;
    pRndisPddCharacteristics->ISRHandler = PDD_ISR;
    pRndisPddCharacteristics->dwIRQ = 0xFFFFFFFF;       // @todo : check this
    pRndisPddCharacteristics->dwMaxRx = MAX_INCOMING_BUFFER;
    pRndisPddCharacteristics->dwBaseAddr = (DWORD)pBaseAddress/*USB_CLIENT_BASE*/;       // TODO: check this 
    pRndisPddCharacteristics->IndicateRndisPacketCompleteHandler = PDD_IndicateRndisPacketComplete;

    // we are not a PCI device
    pRndisPddCharacteristics->bPCIDevice = FALSE;

    //
    // Everything is fine... Proceed!
    //
    InitializeListHead(&g_RndisKitlDev.listTxRndisMessageQueue);

    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L" RNDIS_USBFN_PDDInit: initialization completed\r\n"
    ));

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"-RNDIS_USBFN_PDDInit\r\n"));

    PDDZONE0 = 0;
    PDDZONE = 0;
    MDDZONE0 = 0;
    MDDZONE = 0;
    MDDZONE2 = 0;
    MDDZONE3 = 0; // 1
    MDDZONE4 = 0;
    MDDZONE5 = 0;

    return TRUE;
}

void PDDDeinit(void)
{
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"+RNDIS_USBFN_PDDDenit\r\n"));
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"-RNDIS_USBFN_PDDDenit\r\n"));
}

////////////////////////////////////////////////////////////////////////////////
//  PDD_SendRndisMessage()
//  
//  Routine Description:
//
//      This routine is called by MDD to send one buffer through control 
//      channel. Interrupt IN.
//  
//  Arguments:
//      
//      pDataWrapper :: Structure containing the data..
//
//  Return Value:
//
//      None.
//

void PDD_SendRndisMessage(PDATA_WRAPPER  pDataWrapper)
{
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"+RNDIS_USBFN_PDD_SendRndisMessage\r\n"));
    

    //
    // We don't do the actual sending here but queue it up...
    // We then trigger Interrupt endpoint to send interrupt to host which will in turn
    // use EP0 to GET_ENCAPSULATED_RESPONSE
    InsertTailList((&g_RndisKitlDev.listTxRndisMessageQueue), &(pDataWrapper->Link));

    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"+RNDIS_USBFN_PDD_SendRndisMessage: message queued.\r\n"
    ));
    // Interrupt (via interrupt endpoint) the host to GET_ENCAPSULATED_RESPONSE,0x01

    g_EP1Transfer.dwFlags = USB_REQUEST_DEVICE_TO_HOST;
    g_EP1Transfer.pvBuffer = &g_InterruptData;
    g_EP1Transfer.cbBuffer = sizeof(g_InterruptData);
    g_EP1Transfer.cbTransferred = 0;
    g_EP1Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; // Possible values are in usbfntypes.h
    g_EP1Transfer.pvPddData = NULL;
    g_EP1Transfer.pvPddTransferInfo = NULL;

    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"+RNDIS_USBFN_PDD_SendRndisMessage: issuing interrupt...\r\n"
    ));

    
    
     g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext, 1, &g_EP1Transfer );

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"-RNDIS_USBFN_PDD_SendRndisMessage\r\n"));
    

}

////////////////////////////////////////////////////////////////////////////////
//  PDD_SendRndisPacket()
//
//  Routine Description:
//
//      This routine is called by MDD to send data to host via IN pipe.
//      PDD is guaranteed to have only one outstanding packet to send until
//      the packet is retured to MDD via MddSendRndisPacketComplete()
//  
//  Arguments:
//      
//      pDataWrapper :: structure holding data we need to send.
//
//  Return Value:
//
//      None.
//
void PDD_SendRndisPacket(PDATA_WRAPPER pDataWrapper)
{    

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"+RNDIS_USBFN_PDD_SendRndisPacket\r\n"));
    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"Got SendPacket Request [%d bytes]\r\n", pDataWrapper->dwDataSize
    ));

    if (g_RndisKitlDev.pTxDataWrapper != NULL) {
        //
        // BAD!
        // This should never happen!!
        // Return the packet immediately..
        //
        OALMSG(OAL_ERROR, (L"****Multiple pending send rndis packet!!\r\n"));
        MddSendRndisPacketComplete(pDataWrapper);
    }

    // save the data wrapper pointer so we can return later it in TxComplete call
    g_RndisKitlDev.pTxDataWrapper = pDataWrapper;

    // g_EP2Transfer.dwCallerPermissions;
    g_EP2Transfer.dwFlags = USB_REQUEST_DEVICE_TO_HOST;
//    g_EP2Transfer.pvBuffer = pDataWrapper->pucData;
    g_EP2Transfer.pvBuffer = OALCAtoUA(g_rndis_ep2_buf);

    

    memcpy(g_EP2Transfer.pvBuffer, pDataWrapper->pucData, pDataWrapper->dwDataSize);
    // g_EP2Transfer.dwBufferPhysicalAddress; // not used
    g_EP2Transfer.cbBuffer = pDataWrapper->dwDataSize;
    g_EP2Transfer.cbTransferred = 0;
    g_EP2Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; // Possible values are in usbfntypes.h
    g_EP2Transfer.pvPddData = NULL;
    g_EP2Transfer.pvPddTransferInfo = NULL;

    g_EP2TransferState = TS_SENDING_PACKET;

    g_pddInterface.pfnIssueTransfer( g_pddInterface.pvPddContext, 2, &g_EP2Transfer );

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"-RNDIS_USBFN_PDD_SendRndisPacket\r\n"));
}

// discard outstanding messages when reset
static VOID PDD_DiscardPendingMessages()
{
    PLIST_ENTRY pLink;
    PDATA_WRAPPER pDataWrapper;
    DWORD dwDiscards = 0;

    while (!IsListEmpty(&(g_RndisKitlDev.listTxRndisMessageQueue))) {
        // remove pending message
        pLink = RemoveHeadList(&(g_RndisKitlDev.listTxRndisMessageQueue));
        pDataWrapper = CONTAINING_RECORD(pLink, DATA_WRAPPER, Link);
        // let MDD free buffer
        MddSendRndisMessageComplete(pDataWrapper);
        // record number
        ++dwDiscards;
    }

    KITLOutputDebugString("Discard %d Pending Messages\r\n", dwDiscards);
}

////////////////////////////////////////////////////////////////////////////////
//  PDD_Set()
//
//  Routine Description:
//
//      This routine is used by the MDD to set misc aspects of USB 
//      communication.
//  
//  Arguments:
//      
//      uiRequestId  :: As defined in rndismini.h
//      pvData       :: What to set to.
//      ulDataLength :: The length of data pointed to by pvData.
//
//  Return Value:
//
//      TRUE  :: If successful.
//      FALSE :: otherwise..
//
BOOL PDD_Set(
    IN  UINT    uiRequestId,
    IN  PVOID   pvData,
    IN  ULONG   ulDataLength)
{
    BOOL fRet = TRUE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pvData);
    UNREFERENCED_PARAMETER(ulDataLength);

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"+RNDIS_USBFN_PDD_Set\r\n"));
    // TODO: finish all requests here
    switch(uiRequestId) {
    case REQ_ID_HARD_RESET:
        OALMSG(OAL_ETHER&&OAL_FUNC, (L"PDD_Set:REQ_ID_HARD_RESET\r\n"));
        PDD_DiscardPendingMessages();
        break;
    case REQ_ID_SOFT_RESET:
        OALMSG(OAL_ETHER&&OAL_FUNC, (L"PDD_Set:REQ_ID_SOFT_RESET\r\n"));
        PDD_DiscardPendingMessages();
        break;
    case REQ_ID_ENABLE_INT:
        OALMSG(OAL_ETHER&&OAL_FUNC, (L"PDD_Set:REQ_ID_ENABLE_INT\r\n"));
        break;
    case REQ_ID_DISABLE_INT:
        OALMSG(OAL_ETHER&&OAL_FUNC, (L"PDD_Set:REQ_ID_DISABLE_INT\r\n"));
        break;
    default:
        OALMSG(OAL_ETHER&&OAL_FUNC, (
            L"PDD_Set:Unknown request:0x%x\r\n", uiRequestId
        ));
        fRet = FALSE;
        break;
    }
    
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"-RNDIS_USBFN_PDD_Set\r\n"));
    return fRet;
}

////////////////////////////////////////////////////////////////////////////////
//  PDD_Get()
//
//  Routine Description:
//
//      This routine is used by MDD to query information pertaining to 
//      PDD (like vendor ID, Vendor description, etc).      
//  
//  Arguments:
//      
//      uiRequestId      :: as defined in RndisMini.h
//      pvData           :: The return buffer.
//      ulDataLength     :: Length of the buffer.
//      ulRequiredLength :: Return by us if the passed in buffer is not enough.
//
//  Return Value:
//
//      TRUE  :: If successful.
//      FALSE :: otherwise..
//

BOOL PDD_Get(
    IN  UINT    uiRequestId,
    IN  PVOID   pvData,
    IN  ULONG   ulDataLength,
    OUT ULONG   *pulRequiredLength)
{
    BOOL fRet = FALSE;
    ULONG GenericUlong;
    ULONG ulTotalBytes;
    PUCHAR pucBuffer;

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"+RNDIS_USBFN_PDD_Get\r\n"));

    switch(uiRequestId) {
    case REQ_ID_VENDOR_ID:
        pucBuffer = pucVendorID;
        ulTotalBytes = strlen((const char *)pucVendorID);
        break;
    case REQ_ID_VENDOR_DESCRIPTION:
        pucBuffer = pucVendorDescription;
        ulTotalBytes = strlen((const char *)pucVendorDescription);
        break;
    case REQ_ID_DEVICE_MAX_RX:
        GenericUlong = MAX_INCOMING_BUFFER;
        pucBuffer = (UCHAR *)&GenericUlong;
        ulTotalBytes = sizeof(GenericUlong);
        break;
    case REQ_ID_DEVICE_MACADDR:
        memcpy( g_RndisMacPC, g_RndisMacAddress,6);
        //pucBuffer = g_RndisMacAddress;
        g_RndisMacPC[0] += 2; //PC RNDIS mac must be different from board
        pucBuffer = g_RndisMacPC;
        ulTotalBytes = sizeof(g_RndisMacPC);
        break;
    default:
        OALMSG(OAL_ETHER&&OAL_FUNC, (
            L"-PDD_Get: unknown request 0x%x\r\n", uiRequestId
        ));
        pucBuffer = NULL;
        ulTotalBytes = 0;
        break;
    }

    if (pucBuffer) {
        if (pulRequiredLength)
            *pulRequiredLength = ulTotalBytes;

        if (ulTotalBytes <= ulDataLength) {
            memcpy(pvData, pucBuffer, ulTotalBytes);
            fRet = TRUE;
        }
    }

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"-RNDIS_USBFN_PDD_Get\r\n"));
    return fRet;
}

////////////////////////////////////////////////////////////////////////////////
//  PDD_ISR()
//  
//  Routine Description:
//
//      This function handles the USB interrupt.
//  
//  Arguments:
//      
//      pdwWaitTime ::  The next time out value while waiting for interrupt..
//
//  Return Value:
//
//      TRUE  :: We want more interrupt coming..
//      FALSE :: We have enough, getting outta here!!! that's the end of 
//               eveything.. (in theory, should never be used..).
//
extern DWORD UsbFnInterruptThread(PVOID *);

BOOL PDD_ISR(PDWORD  pdwWaitTime)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pdwWaitTime);

    UsbFnInterruptThread( g_pddInterface.pvPddContext );

    return TRUE;
}   //  PDD_ISR()

////////////////////////////////////////////////////////////////////////////////
//  PDD_IndicateRndisPacketComplete()
//
//  Routine Description:
//
//      Called by MDD when the data passed to it via MddIndicateRndisPacket is
//      completed.
//  
//  Arguments:
//      
//      pucBuffer    :: Buffer passed in MddIndicateRndisPacket.
//      uiBufferSize :: Size of the buffer.
//
//  Return Value:
//
//      None.
//
void PDD_IndicateRndisPacketComplete(PDATA_WRAPPER   pDataWrapper)   
{
    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"+RNDIS_USBFN_PDD_IndicateRndisPacketComplete\r\n"
    ));
    MDDFreeMem(pDataWrapper->pucData);
    MDDFreeDataWrapper(pDataWrapper);
    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"-RNDIS_USBFN_PDD_IndicateRndisPacketComplete\r\n"
    ));
}

// compute a unique MAC address
void SetRNDISMACAddress(void)
{
    GetRNDISMACAddress((UINT16*) g_RndisMacAddress);
}

// compute a unique device serial number
void SetRNDISSerialNumber(void)
{
    BYTE b;
    int i;
    BYTE *pIdBytes; 
    UINT16 mac[3];

    GetRNDISMACAddress(mac);
    pIdBytes = (BYTE*)&mac[0];

    for (i=0; i< 6; i++) {
        b = pIdBytes[i] & 0xff;
        gs_SerialNumber.ptcbString[i * 2] = DigitTable[b % 16];
        gs_SerialNumber.ptcbString[(i * 2) + 1] = DigitTable[b / 16];
    }

    gs_SerialNumber.ptcbString[sizeof(mac) * 2] = '\0';
    gs_SerialNumber.ucbLength = sizeof(gs_SerialNumber);
    gs_SerialNumber.udbDescriptorType = USB_STRING_DESCRIPTOR_TYPE;

    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"RNDIS Serial Number=[%s]\r\n", gs_SerialNumber.ptcbString
    ));
}



//------------------------------------------------------------------------------
