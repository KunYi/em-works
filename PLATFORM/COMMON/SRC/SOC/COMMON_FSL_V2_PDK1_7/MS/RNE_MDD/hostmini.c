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
/*

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  
    hostmini.c

Abstract:  
    This module implements the miniport functionality for the RNDIS host.

Functions:

    
--*/
//------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4057 4115 4127 4131 4201 4204 4214)
#include <windows.h>
#include <halether.h>
#ifdef ZONE_INIT
#undef ZONE_INIT
#endif
#ifdef ZONE_WARNING
#undef ZONE_WARNING
#endif
#ifdef ZONE_ERROR
#undef ZONE_ERROR
#endif
#include <ndis.h>
#include <linklist.h>
#include <rndismini.h>
#include "rndis.h"
#include "mddpriv.h"
#include <celog.h>

////////////////////////////////////////////////////////////////////////////////
//  Global variables used by mdd module.
//

static UCHAR   DefaultVendorDescription[]   = "Microsoft Virtual Rndis Miniport\0";
static UCHAR   DefaultVendorID[]            = "MSFT";

//
//  Where we alloc and queue RNdis message list..
//

//NPAGED_LOOKASIDE_LIST DataWrapperLookAsideList;


//
//  Global Structure used by MDD module.
//  Initialized in wince.c
//

RNDIS_MDD   RndisMdd;       

UINT RNdisMddSupportedOids[] = 
{
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_VENDOR_ID,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_CRC_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_GEN_VENDOR_DRIVER_VERSION,
    OID_GEN_MEDIA_CONNECT_STATUS,

#ifdef NDIS50_MINIPORT

    //
    //  Power Management..
    //
    
    OID_PNP_CAPABILITIES,
    OID_PNP_SET_POWER,
    OID_PNP_QUERY_POWER,
    OID_PNP_ADD_WAKE_UP_PATTERN,
    OID_PNP_REMOVE_WAKE_UP_PATTERN,
    OID_PNP_ENABLE_WAKE_UP    
#endif
};      



////////////////////////////////////////////////////////////////////////////////
//  HostMiniExtractOneDataWrapper()
//
//  Routine Description:
//
//      This function will extract one DATA_WRAPPER from listInRndisPacket.
//
//  Arguments:
//
//      None.
//
//  Return Value:
//
//      PDATA_WRAPPER or NULL if both queue is empty.
//
/*
PDATA_WRAPPER
HostMiniExtractOneDataWrapper(void)
{
    PLIST_ENTRY     pLink = NULL;
    PDATA_WRAPPER   pDataWrapper = NULL;


    EnterCriticalSection(&(RndisMdd.lockInRndisPackets));   
        
        if (!IsListEmpty(&(RndisMdd.listInRndisPackets)))
        {
            pLink = RemoveHeadList(
                        &(RndisMdd.listInRndisPackets));
            
        }
        
    LeaveCriticalSection(&(RndisMdd.lockInRndisPackets));

    if (pLink)
    {
        //
        //  Getting here means we have a message to process.
        //

        pDataWrapper = CONTAINING_RECORD(
                            pLink, 
                            DATA_WRAPPER, 
                            Link);
    }

    return pDataWrapper;


}   //  HostMiniExtractOneDataWrapper()

*/

////////////////////////////////////////////////////////////////////////////////
//  HostMiniThread()
//  
//  Routine Description:
//
//      This is the main thread listening and servicing all RNDIS and 
//      Virtual Ethernet transactions..
//  
//  Arguments:
//
//      None.
//
//  Return Value:
//
//      TRUE if successful, FALSE otherwise.
//
/*
void
HostMiniThread(void)
{   
    PDATA_WRAPPER   pDataWrapper;

    CELOGMSG (ZONE_HOSTMINI | ZONE_RNDIS, 
        (TEXT("HostMiniThread() starts..\r\n")));
    
    RndisMdd.dwMddFlag |= MDD_FLAG_WAITING;


    ////////////////////////////////////////////////////////////////////////////
    //  This loop runs forever..
    //

    while (1)
    {
        WaitForSingleObject(
            RndisMdd.hRndisMddEvent,
            INFINITE);      

        if (RndisMdd.bQuitNow)
        {
            CloseHandle(RndisMdd.hRndisMddEvent);
            return;
        }
        
        //
        //  Get here when there is NDIS message to look at..
        //  Make sure everyone knows that we are no longer waiting..
        //

        EnterCriticalSection(&(RndisMdd.lockMdd));
            RndisMdd.dwMddFlag &= ~MDD_FLAG_WAITING;
        LeaveCriticalSection(&(RndisMdd.lockMdd));


        //
        //  Immediately RESET the event, to make sure that event between the
        //  flag set to WAITING and WaitForSingleObject(), will trigger.
        //
        
        ResetEvent (RndisMdd.hRndisMddEvent);   

        //
        //  Start popping messages and service them..
        //

        while(1)
        {
            //
            //  Get one RNDIS packet.
            //

            pDataWrapper = HostMiniExtractOneDataWrapper();

            if (pDataWrapper == NULL)
            {
                //
                //  The only way outta the loop..
                //

                break;
            }

            CELOGMSG (0, (TEXT(">> Buf[0x%x]-l[%d]\r\n"),
                pDataWrapper->pucData,
                pDataWrapper->dwDataSize));
            
            //
            //  Call the RNDIS packet handler routine to process
            //  the packet then free the resources associated with the msg.
            //

            RndisProcessPacket(pDataWrapper);           
        
        }   //   while(1) consuming RNDIS packets.


        //
        //  All done.
        //  Back to hybernating mode..
        //

        EnterCriticalSection(&(RndisMdd.lockMdd));
            RndisMdd.dwMddFlag |= MDD_FLAG_WAITING;
        LeaveCriticalSection(&(RndisMdd.lockMdd));

    }   //  while(1) waiting for event..

}   //  HostMiniThread()

*/

////////////////////////////////////////////////////////////////////////////////
//  HostMiniQueryInformation()
//  
//  Routine Description:
//
//      Process NDIS query request.
//  
//  Arguments:
//
//      NDIS_OID    NDIS OID
//      PVOID       pvInformationBuffer
//      ULONG       ulInformationBufferLength
//      PULONG      pulBytesWritten
//      PULONG      pulBytesNeeded
//
//  Return Value:
//
//      NDIS_STATUS
//

NDIS_STATUS
HostMiniQueryInformation(    
    IN NDIS_OID Oid,
    IN PVOID    pvInformationBuffer,
    IN ULONG    ulInformationBufferLength,
    OUT PULONG  pulBytesWritten,
    OUT PULONG  pulBytesNeeded
    )
{       
    ULONG       GenericUlong;
    USHORT      GenericUshort;
    PUCHAR      pucBuffer    = (PUCHAR)&GenericUlong;       
    ULONG       ulTotalBytes = sizeof(ULONG);       
    NDIS_STATUS NdisStatus   = NDIS_STATUS_SUCCESS;
    // We can not use this variable in stack because it is to big for the kernel.
    static UCHAR        pucScrapBuffer[512];


//    CELOGMSG (0, (TEXT("HostMini:: QueryInfo() InBuff=[%d] :: "),
//      ulInformationBufferLength));
    
    *pulBytesNeeded = *pulBytesWritten = 0;

    switch (Oid) 
    {   
        ////////////////////////////////////////////////////////////////

        case OID_GEN_MAC_OPTIONS:
//            CELOGMSG (0, (TEXT("OID_GEN_MAC_OPTIONS.\r\n")));
            GenericUlong = (ULONG)(NDIS_MAC_OPTION_TRANSFERS_NOT_PEND   |
                                NDIS_MAC_OPTION_RECEIVE_SERIALIZED      |
                                NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA     |
                                NDIS_MAC_OPTION_NO_LOOPBACK);
            break;

        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_SUPPORTED_LIST:
 //         CELOGMSG (0, (TEXT("OID_GEN_SUPPORTED_LIST.\r\n")));
            pucBuffer = (PUCHAR)RNdisMddSupportedOids;
            ulTotalBytes   = sizeof(RNdisMddSupportedOids);         
            break;

        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_HARDWARE_STATUS:           
//          CELOGMSG (0, (TEXT("OID_GEN_HARDWARE_STATUS.\r\n")));
            GenericUlong = NdisHardwareStatusReady;         
            break;

        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:
//          CELOGMSG (0, 
//              (TEXT("OID_GEN_MEDIA_SUPPORTED/IN_USE.\r\n")));
            GenericUlong = NdisMedium802_3;         
            break;

        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_MAXIMUM_FRAME_SIZE:
//          CELOGMSG (0, (TEXT("OID_GEN_MAXIMUM_FRAME_SIZE.\r\n")));            
            GenericUlong = 1500;
            break;

        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_LINK_SPEED:
//          CELOGMSG (0, (TEXT("OID_GEN_LINK_SPEED.\r\n")));
            GenericUlong = (ULONG)(100000);
            break;

        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_TRANSMIT_BLOCK_SIZE:
  //        CELOGMSG (0, 
    //          (TEXT("OID_GEN_TRANSMIT_BLOCK_SIZE.\r\n")));
            GenericUlong = 1518;
            break;      

        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_RECEIVE_BLOCK_SIZE:
//          CELOGMSG (0, (TEXT("OID_GEN_RECEIVE_BLOCK_SIZE.\r\n")));
            GenericUlong = 1518;
            break;

        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_MAXIMUM_LOOKAHEAD:
//          CELOGMSG (0, 
//              (TEXT("OID_GEN_MAXIMUM_LOOKAHEAD.\r\n")));
            GenericUlong = 1500;
            break;

        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_VENDOR_ID:         
        {
            ULONG   ulRequiredLength;

//          CELOGMSG (0, (TEXT("OID_GEN_VENDOR_ID.\r\n")));                 
            
            //
            //  Call into PDD to get the Vendor ID.
            //

//          if (!PDD_Get(
            if (!RndisMdd.PddCharacteristics.GetHandler(
                    REQ_ID_VENDOR_ID,
                    pucScrapBuffer,
                    512,
                    &ulRequiredLength))
            {
                pucBuffer    = DefaultVendorID;             
                ulTotalBytes = strlen(DefaultVendorID);

            }   
            else
            {           
                pucBuffer    = pucScrapBuffer;
                ulTotalBytes = ulRequiredLength;            
            }
            break;
        }

        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_VENDOR_DESCRIPTION:        
        {
            ULONG   ulRequiredLength;           

//          CELOGMSG (0, (TEXT("OID_GEN_VENDOR_DESCRIPTION.\r\n")));
            
            //
            //  Call into PDD to get the Vendor ID.
            //

//          if (!PDD_Get(
            if (!RndisMdd.PddCharacteristics.GetHandler(
                    REQ_ID_VENDOR_DESCRIPTION,
                    pucScrapBuffer,
                    512,
                    &ulRequiredLength))
            {
                pucBuffer    = DefaultVendorDescription;                
                ulTotalBytes = strlen(DefaultVendorDescription);

            }   
            else
            {           
                pucBuffer    = pucScrapBuffer;
                ulTotalBytes = ulRequiredLength;            
            }
            break;
        }

        ////////////////////////////////////////////////////////////////////////            

        case OID_GEN_VENDOR_DRIVER_VERSION:
//          CELOGMSG (0, 
//              (TEXT("OID_GEN_VENDOR_DRIVER_VERSION.\r\n")));
            GenericUshort = (MDD_DRIVER_MAJOR_VERSION << 8) + 
                            (MDD_DRIVER_MINOR_VERSION);
            pucBuffer    = (PUCHAR)&GenericUshort;
            ulTotalBytes = sizeof(USHORT);
            break;


        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_CURRENT_PACKET_FILTER:         
//          CELOGMSG (0, (
//              TEXT("OID_GEN_CURRENT_PACKET_FILTER.\r\n")));
            
//            CELOGMSG (ZONE_HOSTMINI, 
//              (TEXT("HostMini:: GET Current Filter = [0x%x]\r\n"),
//              RndisMdd.dwCurrentPacketFilter));

            GenericUlong = RndisMdd.dwCurrentPacketFilter;
            break;

        
        ////////////////////////////////////////////////////////////////////////
        
        case OID_GEN_MAXIMUM_TOTAL_SIZE:
//          CELOGMSG (0, (TEXT("OID_GEN_MAXIMUM_TOTAL_SIZE.\r\n")));
            GenericUlong = (ULONG)(1514);
            break;

        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_MEDIA_CONNECT_STATUS:
//          CELOGMSG (0, 
//              (TEXT("OID_GEN_MEDIA_CONNECT_STATUS.\r\n")));
            GenericUlong = NdisMediaStateConnected;
            break;

        ////////////////////////////////////////////////////////////////////////

        case OID_802_3_PERMANENT_ADDRESS:
//          CELOGMSG (0, 
//              (TEXT("OID_802_3_PERMANENT_ADDRESS.\r\n")));            
            pucBuffer    = RndisMdd.PermanentNwAddr;
            ulTotalBytes = 6;

//            CELOGMSG (0, 
//              (TEXT("RndisMdd:: PermanentNetworkAddress = %x%x%x%x%x%x\r\n"),
//                pucBuffer[0],
//                pucBuffer[1],
//                pucBuffer[2],
//                pucBuffer[3],
//                pucBuffer[4],
//                pucBuffer[5]));                   
            break;

        ////////////////////////////////////////////////////////////////////////
        
        case OID_802_3_CURRENT_ADDRESS:            
//          CELOGMSG (0, 
//              (TEXT("OID_802_3_PERMANENT_ADDRESS.\r\n")));
            pucBuffer    = RndisMdd.PermanentNwAddr;
            ulTotalBytes = 6;

//            CELOGMSG (0, 
//              (TEXT("RndisMdd:: CurrentNetworkAddress = %x%x%x%x%x%x\r\n"),
//                pucBuffer[0],
//                pucBuffer[1],
//                pucBuffer[2],
//                pucBuffer[3],
//                pucBuffer[4],
//                pucBuffer[5]));                   
            break;

        
        ////////////////////////////////////////////////////////////////
        
        case OID_GEN_MAXIMUM_SEND_PACKETS:
//          CELOGMSG (0, 
//              (TEXT("OID_GEN_MAXIMUM_SEND_PACKETS.\r\n")));           
            //
            //  Arbitrarily chosen number..
            //
            GenericUlong = 16;
            break;

        ////////////////////////////////////////////////////////////////

        case OID_802_3_MAXIMUM_LIST_SIZE:
//          CELOGMSG (0,
//              (TEXT("OID_802_3_MAXIMUM_LIST_SIZE returning [%d]\r\n"),
//              RNDIS_MAX_PACKETS_PER_MESSAGE));
            GenericUlong = RNDIS_MAX_PACKETS_PER_MESSAGE;             
            break;
        
        ////////////////////////////////////////////////////////////////

        case OID_GEN_XMIT_OK:
//          CELOGMSG (0,
//              (TEXT("OID_GEN_XMIT_OK returning [%d]\r\n"),
//              RndisMdd.dwTransmitOkay));

            GenericUlong = RndisMdd.dwTransmitOkay;
            break;

        ////////////////////////////////////////////////////////////////

        case OID_GEN_RCV_OK:
//          CELOGMSG (0,
//              (TEXT("OID_GEN_RCV_OK returning [%d]\r\n"),
//              RndisMdd.dwReceiveOkay));
            GenericUlong = RndisMdd.dwReceiveOkay;
            break;

        ////////////////////////////////////////////////////////////////

        case OID_GEN_XMIT_ERROR:                    
//          CELOGMSG (0,
//              (TEXT("OID_GEN_XMIT_ERROR returning [%d]\r\n"),
//              RndisMdd.dwTransmitError));
            GenericUlong = RndisMdd.dwTransmitError;
            break;

        ////////////////////////////////////////////////////////////////

        case OID_GEN_RCV_ERROR:                 
//          CELOGMSG (0,
//              (TEXT("OID_GEN_RCV_ERROR returning [%d]\r\n"),
//              RndisMdd.dwReceiveError));
            GenericUlong = RndisMdd.dwReceiveError;
            break;

        ////////////////////////////////////////////////////////////////

        case OID_GEN_RCV_NO_BUFFER:                 
//          CELOGMSG (0,
//              (TEXT("OID_GEN_RCV_NO_BUFFER returning [%d]\r\n"),
//              RndisMdd.dwReceiveNoBuffer));
            GenericUlong = RndisMdd.dwReceiveNoBuffer;
            break;


        ////////////////////////////////////////////////////////////////

        case OID_GEN_RCV_CRC_ERROR:                 
//          CELOGMSG (0,
//              (TEXT("OID_GEN_RCV_CRC_ERROR returning [%d]\r\n"),
//              0x00));
            GenericUlong = 0x00;
            break;
        
        ////////////////////////////////////////////////////////////////////////
        
        default:
            KITLOutputDebugString("OID[%x] not yet implemented!\r\n", Oid);         

//          CELOGMSG (ZONE_HOSTMINI,
//              (TEXT("OID[%x] not yet implemented!\r\n"), 
//              Oid));          

            NdisStatus = NDIS_STATUS_INVALID_OID;
            break;
    
    }   //  switch()


    //
    //  Everyone gets here...
    //  
    
    if (NdisStatus == NDIS_STATUS_SUCCESS) 
    {
//      CELOGMSG (0, (TEXT("ulTotalBytes = [%d]\r\n"),
//          ulTotalBytes));

        if (ulTotalBytes > ulInformationBufferLength) 
        {
            //
            //  Not enough room in pvInformationBuffer. Punt
            //          

//          CELOGMSG (0, 
//              (TEXT("HostMini:: Small InBuff [%d]bytes, need [%d]\r\n"),
//              ulInformationBufferLength,
//              ulTotalBytes));

            *pulBytesNeeded = ulTotalBytes;
            NdisStatus = NDIS_STATUS_INVALID_LENGTH;
        } 
        else 
        {
            //
            //  Store result.
            //          

            memcpy(
                pvInformationBuffer, 
                pucBuffer, 
                ulTotalBytes);

            *pulBytesWritten = ulTotalBytes;         
        }   
    }
    
//    CELOGMSG (0, 
//      (TEXT("HostMini:: QueryInformation returning: %s\r\n"),
//        NdisStatus ? TEXT("Error") : TEXT("Success")));
    
    return NdisStatus;  

}   // HostMiniQueryInformation()



////////////////////////////////////////////////////////////////////////////////
//  HostMiniSetInformation()
//
//  Routine Description:
//
//  Handles a set operation for a single Oid
//
//  Arguments:
//      Oid                         -  The OID of the set.
//      pvInformationBuffer         -  Holds the data to be set.
//      ulInformationBufferLength   -  The length of pvInformationBuffer.
//      pulBytesRead                -  If the call is successful, returns the number
//                                      of bytes read from pvInformationBuffer.
//      pulBytesNeeded              -  If there is not enough data in 
//                                      pvInformationBuffer to satisfy the OID, 
//                                      returns the amount of storage needed.
//
//  Return Value:
//
//  -   NDIS_STATUS_SUCCESS
//  -   NDIS_STATUS_PENDING
//  -   NDIS_STATUS_INVALID_LENGTH
//  -   NDIS_STATUS_INVALID_OID
//
//
NDIS_STATUS
HostMiniSetInformation(
   IN   NDIS_OID    Oid,
   IN   PVOID       pvInformationBuffer,
   IN   ULONG       ulInformationBufferLength,
   OUT  PULONG      pulBytesRead,
   OUT  PULONG      pulBytesNeeded
   )
{    
//  PUCHAR          InfoBuffer      = (PUCHAR)pvInformationBuffer;    
    DWORD           dwFilter;

//  CELOGMSG (0, (TEXT("HostMini:: SetInformation():: ")));

    //
    //  Check for the most common OIDs
    //
    
    switch (Oid) 
    { 
        ////////////////////////////////////////////////////////////////////////

        case OID_802_3_MULTICAST_LIST:                                      
//          CELOGMSG (0, 
//              (TEXT("OID_802_3_MULTICAST_LIST.\r\n")));

            *pulBytesRead = ulInformationBufferLength;
            *pulBytesNeeded = 0;
            
            if ((ulInformationBufferLength % 6) != 0)
                return NDIS_STATUS_INVALID_LENGTH;          

            if ((ulInformationBufferLength / 6) > DEFAULT_MULTICASTLIST_MAX)
                return NDIS_STATUS_MULTICAST_FULL;
            
            memcpy(
                &(RndisMdd.MulticastAddresses),
                pvInformationBuffer, 
                ulInformationBufferLength);

            RndisMdd.dwMulticastListInUse = ulInformationBufferLength / 6;

            //
            //  [stjong] VEHub interface
            //  Do something here to inform VEHub that we are listening to the
            //  new set of multicast addresses..
            //
            return NDIS_STATUS_SUCCESS;
            break;
        
        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_CURRENT_PACKET_FILTER:
 //           CELOGMSG (0, 
//              (TEXT("OID_GEN_CURRENT_PACKET_FILTER.\r\n")));

            *pulBytesRead   = ulInformationBufferLength;
            *pulBytesNeeded = 0;

            if (ulInformationBufferLength != 4 ) 
                return NDIS_STATUS_INVALID_LENGTH;                        
        
            memcpy (&dwFilter, pvInformationBuffer, 4);
        
            //
            //  Reject types we don't support..
            //

            if (dwFilter & 
                 (NDIS_PACKET_TYPE_SOURCE_ROUTING   |
                  NDIS_PACKET_TYPE_SMT              |
                  NDIS_PACKET_TYPE_MAC_FRAME        |
                  NDIS_PACKET_TYPE_FUNCTIONAL       |
                  NDIS_PACKET_TYPE_ALL_FUNCTIONAL   |
                  NDIS_PACKET_TYPE_GROUP)) 
            {
                return NDIS_STATUS_NOT_SUPPORTED;               
            }           

            KITLOutputDebugString("HostMini:: New filter set: [0x%x] --> [0x%x]\r\n",
                RndisMdd.dwCurrentPacketFilter,
                dwFilter);

            //
            //  Hence we support:
            //  DIRECTED, MULTICAST, ALL_MULTICAST, BROADCAST, 
            //  Set the new value on the adapter..
            //
            
            RndisMdd.dwCurrentPacketFilter = dwFilter;

            if (dwFilter)                           
                RndisMdd.dwDeviceState = RNDIS_INITIALIZED;                         
            else            
                RndisMdd.dwDeviceState = RNDIS_DATA_INITIALIZED;

            return NDIS_STATUS_SUCCESS;
            break;

        
        ////////////////////////////////////////////////////////////////////////

        case OID_GEN_CURRENT_LOOKAHEAD:
            
            //
            //  No need to record requested lookahead length since we
            //  always indicate the whole packet.
            //
//            CELOGMSG (0, 
//              (TEXT("HostMini:: OID_GEN_CURRENT_LOOKAHEAD...\r\n")));            
            *pulBytesRead = 4;
            break;


        ////////////////////////////////////////////////////////////////////////

        default:
//            CELOGMSG(
//              0, (TEXT("HostMini:: Default...returning INVALID\r\n")));
            *pulBytesRead = 0;
            *pulBytesNeeded = 0;
            return NDIS_STATUS_INVALID_OID;
    }
    
    return NDIS_STATUS_SUCCESS;

}   //  HostMiniSetInformation()



//
//  Functions below are exported to PDD..
//


////////////////////////////////////////////////////////////////////////////////
//  MddIndicateRndisMessage()
//  
//  Routine Description:
//
//      This routine is called by PDD when it has received one complete RNDIS
//      message.   We process it immediately and generate a reply if necessary.
//  
//  Arguments:
//
//      None.
//
//  Return Value:
//
//      None. [PDD does not need to care whether or not data is valid.]
//      
//  Note:
//      
//      This function will not retain PDD buffer.
//
void
MddIndicateRndisMessage(PDATA_WRAPPER pDataWrapper) 
{
//  CELOGMSG (0,
//      (TEXT("HostMini:: Rndis message received; [%d] bytes.\r\n"),
//      pDataWrapper->dwDataSize));

    RndisProcessMessage(pDataWrapper);
    
    return;

}   // MddIndicateRndisMessage()


////////////////////////////////////////////////////////////////////////////////
//  MddSendRndisMessageComplete()
//  
//  Routine Description:
//
//      This routine is called by PDD when it has completed 
//      sending one RNDIS message.
//  
//  Arguments:
//
//      pDataWrapper :: structure containing the data..
//
//  Return Value:
//
//      None. [PDD does not need to care whether or not data is valid.]
//      
//  Note:
//      
//      This function will not retain PDD buffer.
//
static BOOL RdisPacketBufferEmpty=TRUE;
static DATA_WRAPPER RndisPacketRapper;
static BYTE RdisPacketBuffer[MAX_PACKET_SIZE];

////////////////////////////////////////////////////////////////////////////////
//  MddIndicateRndisPacket()
//  
//  Routine Description:
//
//      This routine is called by PDD when it has completed 
//      sending one RNDIS message.
//  
//  Arguments:
//
//      pDataWrapper  :: The structure containing RNDIS_PACKET
//
//  Return Value:
//
//      None. [PDD does not need to care whether or not data is valid.]
//      
//  Note:
//      
//      This function ** MUST ** retain PDD buffer to be returned in
//      PDD's IndicateRndisPacketCompleteHandler()
//
void
MddIndicateRndisPacket(PDATA_WRAPPER    pDataWrapper)
{
/*  CELOGMSG (0,
        (TEXT("HostMini:: Rndis message received; [%d] bytes.\r\n"),
        pDataWrapper->dwDataSize));
    
    //
    //  Queue it..
    //  

    CELOGMSG (0, 
        (TEXT("HostMini:: Q-Pkt: Wrap[0x%x] Buf[0x%x]\r\n"),        
        pDataWrapper,
        pDataWrapper->pucData));

    EnterCriticalSection(&(RndisMdd.lockInRndisPackets));
        InsertTailList(
            &(RndisMdd.listInRndisPackets),
            &(pDataWrapper->Link));
    LeaveCriticalSection(&(RndisMdd.lockInRndisPackets));


    //
    //  Trigger the worker thread if it is not already working..
    //
    
    EnterCriticalSection(&(RndisMdd.lockMdd));
        if (RndisMdd.dwMddFlag & MDD_FLAG_WAITING)
            SetEvent(RndisMdd.hRndisMddEvent);
    LeaveCriticalSection(&(RndisMdd.lockMdd));
*/
    RndisProcessPacket(pDataWrapper);
    RdisPacketBufferEmpty=TRUE;
    return;

}   //  MddIndicateRndisPacket()

// Define for new PDD
PDATA_WRAPPER MDDAllocDataWrapper()
{
    if (RdisPacketBufferEmpty) {
        RdisPacketBufferEmpty=FALSE;
        return &RndisPacketRapper;
    }
    else {
        ASSERT(FALSE);
        return NULL;
    };

}
PBYTE MDDAllocMem(void)
{
    return (PBYTE)RdisPacketBuffer;
}
void MDDFreeMem(PBYTE pDataBuffer)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pDataBuffer);
};
void MDDFreeDataWrapper(PDATA_WRAPPER pDataWrapper)
{
  // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pDataWrapper);
    
    RdisPacketBufferEmpty=TRUE;
};


////////////////////////////////////////////////////////////////////////////////
//  MddDisconnect()
//  
//  Routine Description:
//
//      PDD calls this function when it detects hot disconnect.
//      We reset PDD hardware and get back to ready to connect state..
//  
//  Arguments:
//
//      None.
//
//  Return Value:
//
//      None.
//      
void
MddDisconnect(void)
{
    KITLOutputDebugString("HostMini:: MddDisconnect!!\r\n");
    
    RndisRestart(TRUE); 

}   //  MddDisconnect()



////////////////////////////////////////////////////////////////////////////////
//  HostMiniInit()
//  
//  Routine Description:
//
//      Called during initialization, our one and only chance to initialize..
//  
//  Arguments:
//
//      None.
//
//  Return Value:
//
//      TRUE if successful, FALSE otherwise.
//
BOOL
HostMiniInit( BYTE *pbBaseAddress, DWORD dwLogicalAddress, USHORT MacAddr[3])
{
    //
    //  Global initialization..
    //
    
/*  InitializeListHead(&RndisMdd.listInRndisPackets);
    InitializeListHead(&RndisMdd.listVMiniNdisPackets);
    
    RndisMdd.hRndisMddEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    InitializeCriticalSection(&RndisMdd.lockMdd);
    InitializeCriticalSection(&RndisMdd.lockVMiniNdisPackets);

    CELOGMSG (ZONE_MEM, (TEXT("LookAside for DATA_WRAPPER:\r\n")));

    ExInitializeNPagedLookasideList(
        &DataWrapperLookAsideList,
        NULL,
        NULL,
        0,
        sizeof(DATA_WRAPPER),
        MEM_TAG_DATA_WRAPPER,
        DATA_WRAPPER_LOOKASIDE_DEPTH
        );

*/
    KITLOutputDebugString("HostMiniInit:: !!\r\n");
    //
    //  Let RNDIS message handling engine init.
    //
    if (!RndisInit( pbBaseAddress,dwLogicalAddress,MacAddr))
    {
//      CloseHandle(RndisMdd.hRndisMddEvent);
        return FALSE;
    }   
    
    
    //
    //  Everything is fine, fire our worker thread that services h/w.   
    //
    
/*  CreateThread(0,
         0,
         (LPTHREAD_START_ROUTINE)HostMiniThread,
         NULL,
         0,                                         
         NULL );
*/
    return TRUE;
    
}   //  HostMiniInit()



////////////////////////////////////////////////////////////////////////////////
//  HostMiniDeInit()
//  
//  Routine Description:
//
//      Reverse what's done in HostMiniInit()
//  
//  Arguments:
//
//      None.
//
//  Return Value:
//
//      None.
//
void
HostMiniDeInit(void)
{
    RndisMdd.bQuitNow = TRUE;

//  SetEvent(RndisMdd.hRndisMddEvent);

    RndisDeInit();  

//  ExDeleteNPagedLookasideList(&DataWrapperLookAsideList);

}   //  HostMiniDeInit()
#pragma warning(pop)


