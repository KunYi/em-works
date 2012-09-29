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
    rndis.c

Abstract:  
    This module understands RNDIS messages..    

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
#pragma warning(disable: 4115 4127 4131 4201 4204 4214)
#include <windows.h>
#include <ndis.h>
#include <halether.h>
#undef ZONE_INIT
#undef ZONE_WARNING
#undef ZONE_ERROR
#include <string.h>
#include <rndismini.h>
#include "mddpriv.h"
#include "rneutil.h"
#include "rndis.h"
#define __THIS_FILE__  "rndis.cpp"
#ifdef  CELOGMSG 
#undef  CELOGMSG
#endif
#define CELOGMSG(Cond, Printf_exp) ((void)0)//DEBUGCELOGMSG(Cond, Printf_exp) 
#define CELOGDUMPDATA(Cond, ID, Data, Len) ((void)0)//DEBUGCELOG(Cond, ID, Data, Len)
#define ZONE_RNDIS      DEBUGZONE(1)
#define ZONE_DATA       DEBUGZONE(5)

#define DEBUG_SINGLE_CHAR 1
#include <celog.h>
#pragma warning(pop)

////////////////////////////////////////////////////////////////////////////////
//  For convenience..
//

#define GET_PTR_TO_RNDIS_DATA_BUFF(_Message)                    \
    ((PVOID) ((PUCHAR)(_Message) +  _Message->DataOffset))

#define RESERVED_FROM_RECV_PACKET(_Packet)                      \
    ((PRCV_RNDIS_PACKET *)((_Packet)->MiniportReserved))

#define NDIS_PACKET_MINIPORTRESERVED(_Packet)                   \
    ((_Packet)->MiniportReserved)

#define MIN(x,y) ((x > y) ? y : x)

DWORD OEMKitlGetSecs(void);


////////////////////////////////////////////////////////////////////////////////
//  Externs..
//

//extern NPAGED_LOOKASIDE_LIST  DataWrapperLookAsideList;



////////////////////////////////////////////////////////////////////////////////
//  Global variable for RNDIS MDD..
//

RNDIS_MDD               RndisMdd;               
//NPAGED_LOOKASIDE_LIST RcvRndisPacketLookAsideList;
//NDIS_HANDLE               NdisPacketPool;
//NDIS_HANDLE               NdisBufferPool;
//HANDLE                    g_TxEvent;

// RNDIS Message Buffer
static BufferDescript MsgBufferDesc;
static DWORD MsgBuffer[(MAX_PACKET_SIZE+4*(sizeof(DATA_WRAPPER)+sizeof(DWORD)))/sizeof(DWORD) + 1];

// Ethernet Frame
#define MAX_ETH_BLOCK 1520
static BufferDescript EthBufferDesc;
static DWORD EthBuffer[((MAX_ETH_BLOCK+sizeof(DWORD))*10)/sizeof(DWORD) + 1];



////////////////////////////////////////////////////////////////////////////////
//  RndisStartRNDISMINI1()
//
//  Routine Description:
//
//      Thread that instantiate the virtual adapter instance.
//  
//  Arguments:
//
//      None.
//
//  Return Value:
//
//      None.
//
//void
//RndisStartRNDISMINI1(void)
//{
//  VMiniInstantiateMiniport(TRUE);
//
//} //  RndisStartRNDISMINI1()



////////////////////////////////////////////////////////////////////////////////
//  RndisForceReturnCEVminiPackets()
//
//  Routine Description:
//
//      Force return all packets we queued in listVMiniNdisPackets.
//  
//  Arguments:
//
//      None.
//
//  Return Value:
//  
//      None.
//  
//  Note:
//
//      ** WARNING ** This function should only be called in the INTERRUPT
//      context!!!
//  
/*
void
RndisForceReturnCEVminiPackets(void)
{
    PLIST_ENTRY     pLink;
    PNDIS_PACKET    pNdisPacket;    


    EnterCriticalSection(&RndisMdd.lockVMiniNdisPackets);

    while (!IsListEmpty(&(RndisMdd.listVMiniNdisPackets)))
    {
        ASSERT(RndisMdd.dwTotalVMiniPendings);

        pLink = RemoveHeadList(&(RndisMdd.listVMiniNdisPackets));

        RndisMdd.dwTotalVMiniPendings--;

        pNdisPacket = CONTAINING_RECORD(
                            pLink,
                            NDIS_PACKET,
                            MiniportReserved);

        //
        //  Return the NdisPacket to VMini now..
        //  Can't be in CS since VMiniIndicatePacketDone() will use NDIS CS
        //  and NDIS may be calling RndisSendPacket() hence a deadlock!
        //

        LeaveCriticalSection(&RndisMdd.lockVMiniNdisPackets);
        VMiniIndicatePacketDone(
            pNdisPacket);
        EnterCriticalSection(&RndisMdd.lockVMiniNdisPackets);
    }

    LeaveCriticalSection(&RndisMdd.lockVMiniNdisPackets);

}   //  RndisForceReturnCEVminiPackets()
*/


////////////////////////////////////////////////////////////////////////////////
//  RndisRestart()
//
//  Routine Description:
//
//      DeInitialize RNDISMINI1 and get us back to RNDIS_UNINITIALIZED state
//      ready for new connection..
//  
//  Arguments:
//
//      bResetPDD :: TRUE if we need to reset the PDD, FALSE otherwise.
//
//  Return Value:
//  
//      None.
//  
//  Note:
//
//      ** WARNING ** This function should only be called in the INTERRUPT
//      context!!!
//  
void
RndisRestart(BOOL bResetPDD)
{
    CELOGMSG  (ZONE_INIT, (TEXT("RNDISMINI --- RESTART ---\r\n")));
    
    KITLOutputDebugString("RNDISMINI --- RESET ---\r\n");

    RndisMdd.dwDeviceState = RNDIS_UNINITIALIZED;

    //  
    //  1. Deinitialize RNDISMINI1 which in its ShutDownHandler() will get
    //        us to return all its pending TX packets.
    //
    //  2. Hard Reset PDD.
    //        This should cause PDD to return all pending RNDIS messages that
    //        it may be half sending, and reset its hw to ready a future
    //        new connection.
    //

    //
    //  Tear down CE Miniport driver RNDISMINI1
    //      

//  VMiniInstantiateMiniport(FALSE);    

    if (bResetPDD)
    {
        //
        //  HardReset the hardware here..
        //  Somehow we need to wait a while till host is not looking to do this.
        //  Otherwise host will fail to unload cleanly the rndis host 
        //  driver (???)
        //

//      Sleep(500);
        DWORD dwStartSec=OEMKitlGetSecs();
        while (dwStartSec==OEMKitlGetSecs()); // Wait 1 second.


        RndisMdd.PddCharacteristics.SetHandler(
            REQ_ID_HARD_RESET,
            NULL,
            0x00);
//      PDD_Set(
//          REQ_ID_HARD_RESET,
//          NULL,
//          0x00);
    }
//  else
//      RndisMdd.PddCharacteristics.SetHandler(
//          REQ_ID_SOFT_RESET,
//          NULL,
//          0x00);
//      PDD_Set(
//          REQ_ID_SOFT_RESET,
//          NULL,
//          0x00);

    // Reset to internal flag
    RndisMdd.dwCurrentPacketFilter=0;
    //InBuffer.dwDataSize==0;
    //BufferReset();
}   //  RndisRestart()



////////////////////////////////////////////////////////////////////////////////
//  RndisPrepareRndisPacket()
//  
//  Routine Description:
//
//      This function forms a linke DATA_WRAPPER and queue that to pListEntry.
//  
//  Arguments:
//
//      pNdisPacket :: The NdisPacket that contains user data.
//      pListEntry  :: Where we link the DATA_WRAPPER containing RNDIS_PACKET
//                          data header and the NdisBuffer from NdisPacket.
//                          The newly formed DATA_WRAPPER will be linked at the
//                          end of this list.
//      pdwTotalLen ::  Total Length of the collection of RNDIS_PACKETs so far..
//
//  Return Value:
//
//      SUCCESS, ERROR_NO_MEM, ERROR_MAX_REACHED
//

#define SUCCESS             0x00000000
#define ERROR_MAX_REACHED   0x00000002
DWORD
RndisPrepareRndisPacket(
    PBYTE       pDataBuffer,    
    DWORD       dwDataLen,
    PRNDIS_MESSAGE pRndisMessage,
    DWORD dwTotalMessageBufferSize)

{
    DWORD           dwCurrentPacketLength;
    DWORD           dwTotalDataLength;
//  PRNDIS_MESSAGE  pRndisMessage;
    PRNDIS_PACKET   pRndisPacket;
//  PNDIS_BUFFER    pNdisBuffer;    

    CELOGMSG (ZONE_RNDIS,
                (TEXT("RNdis:: PrePareRndisPacket (%d Bytes)\r\n"),dwDataLen));

    if (pRndisMessage) {
        dwCurrentPacketLength =
            RNDIS_MESSAGE_SIZE (RNDIS_PACKET) + 
            dwDataLen;

        dwTotalDataLength = dwCurrentPacketLength;
        
        //
        //  Pad it to multiple of 8 bytes (required by RNDIS host).
        //
        
        dwCurrentPacketLength += (8 - (dwCurrentPacketLength % 8));

        if ((dwCurrentPacketLength > dwTotalMessageBufferSize) ||
            (dwTotalDataLength < dwDataLen) ||                  //check for overflow : RNDIS_MESSAGE_SIZE (RNDIS_PACKET) + dwDataLen 
            (dwCurrentPacketLength < dwTotalDataLength))        //check for overflow : dwCurrentPacketLength += (8 - (dwCurrentPacketLength % 8))
        {
            KITLOutputDebugString("RNdis:: Too much outgoing, bailing out!\r\n");           

            CELOGMSG (ZONE_RNDIS,(TEXT("RNdis:: Max[%d] SoFar[%d] CurentPacket[%d]\r\n"),
                dwTotalMessageBufferSize,
                0,
                dwCurrentPacketLength));            

            return  ERROR_MAX_REACHED;
        }   



        //
        //  Okay, it fits.. make it..
        //
        //  Fill up the RNDIS_PACKET header
        //
        
        
        pRndisMessage->NdisMessageType = REMOTE_NDIS_PACKET_MSG;
        pRndisMessage->MessageLength   = dwCurrentPacketLength; 

        pRndisPacket = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pRndisMessage);

        pRndisPacket->DataOffset            = sizeof(RNDIS_PACKET);
        pRndisPacket->DataLength            = dwDataLen;
        pRndisPacket->OOBDataOffset         = 0x00;
        pRndisPacket->OOBDataLength         = 0x00;
        pRndisPacket->NumOOBDataElements    = 0x00;
        pRndisPacket->PerPacketInfoOffset   = 0x00;
        pRndisPacket->PerPacketInfoLength   = 0x00;
        pRndisPacket->VcHandle              = 0x00;
        pRndisPacket->Reserved              = 0x00;
        

        //
        //  Now, handle the NDIS_BUFFERs
        //

        memcpy(pRndisPacket+1,pDataBuffer,dwDataLen);

        //
        //  Outta here with flying color..
        //  Here is the new offset..
        //

        
    };
    return SUCCESS;

}   //  RndisPrepareRndisPacket()

// Rndis SendPacket flag
BOOL bRndisSendPacketEmpty=TRUE;
BOOL PDD_IsPacketSend(void)
{
    return bRndisSendPacketEmpty;
}
void
MddSendRndisPacketComplete(PDATA_WRAPPER pDataWrapper)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pDataWrapper);
    
    bRndisSendPacketEmpty=TRUE;
}   //  MddSendRndisPacketComplete()

////////////////////////////////////////////////////////////////////////////////
//  RndisSendPacket()
//  
//  Routine Description:
//
//      When VMini has packet to indicate up to the Rndis host..
//      If we are not already sending, then start the PDD to send otherwise
//      queue this NdisPacket.
//  
//  Arguments:
//
//      pNdisPacket :: The Ndis packet.
//
//  Return Value:
//
//      TRUE if we want to keep the packet, FALSE otherwise.
//
#define RNDIS_MESSAGE_HEAD_SIZE (sizeof(RNDIS_MESSAGE) - sizeof(RNDIS_MESSAGE_CONTAINER))

UINT16 RndisEDbgSendFrame(BYTE * pData, DWORD dwLength)
{
//  DWORD   dwTotalLen  = 0x00;
    DWORD   dwWaitMSec=0;
    BOOL    bSuccess = FALSE;
    static BYTE bTempBuffer[MAX_PACKET_SIZE];
    PRNDIS_MESSAGE pRndisMessage=(PRNDIS_MESSAGE)bTempBuffer;
    DWORD dwTotalMessageBufferSize=MAX_PACKET_SIZE- RNDIS_MESSAGE_HEAD_SIZE;
    DWORD dwStartSec=OEMKitlGetSecs();
    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwLength);

    CELOGMSG (ZONE_DATA,(TEXT("Rndis::RndisEDbgSendFrame(dwLength=%d)\r\n"),dwLength));
    
    while  (RndisMdd.dwCurrentPacketFilter == 0 &&  OEMKitlGetSecs()- dwStartSec <2) 
        //PDD_ISR();
        RndisMdd.PddCharacteristics.ISRHandler(&dwWaitMSec);

    if (RndisMdd.dwCurrentPacketFilter == 0)
    {
        CELOGMSG (ZONE_DATA,(TEXT("Rndis::RndisSendPacket() to host:: Reject! filter == 0\r\n")));
        return 1;
    }
    


    if (RndisPrepareRndisPacket(pData,dwLength,pRndisMessage,dwTotalMessageBufferSize)
        ==SUCCESS) {

        static DATA_WRAPPER DataWrapper;
        
        dwStartSec=OEMKitlGetSecs();
        DataWrapper.Link.Flink= DataWrapper.Link.Blink=NULL;
        DataWrapper.pucData=bTempBuffer;
        DataWrapper.dwDataSize=pRndisMessage->MessageLength + RNDIS_MESSAGE_HEAD_SIZE;
        bRndisSendPacketEmpty=FALSE;

        do
        {   
            //
            //  PDD is idling, kick start the sending process..
            //
//          if (PDD_SendRndisPacket(&DataWrapper)) {
            RndisMdd.PddCharacteristics.SendRndisPacketHandler(&DataWrapper);
            bSuccess = TRUE;

//          }
//          PDD_ISR();
            RndisMdd.PddCharacteristics.ISRHandler(&dwWaitMSec);

        }while (!bSuccess && (OEMKitlGetSecs()-dwStartSec<2));

        if (bSuccess) { // Loop until success.
            while (!PDD_IsPacketSend())
//              PDD_ISR();
                RndisMdd.PddCharacteristics.ISRHandler(&dwWaitMSec);
        }

    }

//  PDD_ISR();
    RndisMdd.PddCharacteristics.ISRHandler(&dwWaitMSec);
//  KITLOutputDebugString("Rndis::RndisEDbgSendFrame return (%d) Byte (%d)\r\n",bSuccess,dwLength);
    return (bSuccess?0:1);

}   //  RndisSendPacket()


void
MddSendRndisMessageComplete(PDATA_WRAPPER pDataWrapper) 
{

    if (FreeBuffer(&MsgBufferDesc, pDataWrapper)==FALSE) {
        KITLOutputDebugString("ASSERT! Rndis::MddSendRndisMessageComplete return FALSE\r\n");
    }
}   //  MddSendRndisMessageComplete();

////////////////////////////////////////////////////////////////////////////////
//  RndisSendRndisMessage()
//  
//  Routine Description:
//
//      This function creates RNDIS message and send it out.  
//  
//  Arguments:
//
//      None.
//
//  Return Value:
//
//      TRUE if successful, FALSE otherwise.
//
void
RndisSendRndisMessage(
    DWORD       dwRndisMessageType,
    DWORD       dwRequestId,
    NDIS_STATUS NdisStatus,
    PUCHAR      pucInBuffer,
    DWORD       dwBufferLength,
    DWORD       dwUnspecified1)
{
    UINT            uiMessageSize;
    PRNDIS_MESSAGE  pRndisMessage;
    DATA_WRAPPER    * pDataWrapper;
//  BOOL bReturn;

    CELOGMSG (ZONE_RNDIS,
        (TEXT("RNdis:: Create [%s] InBuff[0x%x] Size[%d] Unspec[%d]\r\n"),
        dwRndisMessageType == REMOTE_NDIS_INITIALIZE_CMPLT  ? 
            TEXT("INIT-CMPLT") :
        dwRndisMessageType == REMOTE_NDIS_QUERY_CMPLT       ? 
            TEXT("QUERY-CMPLT"):
        dwRndisMessageType == REMOTE_NDIS_SET_CMPLT         ? 
            TEXT("SET-CMPLT")  :
        dwRndisMessageType == REMOTE_NDIS_RESET_CMPLT       ? 
            TEXT("RESET-CMPLT"):
        dwRndisMessageType == REMOTE_NDIS_KEEPALIVE_CMPLT   ? 
            TEXT("KEEPALIVE-CMPLT") : TEXT("UNKNOWN!"),
        pucInBuffer,
        dwBufferLength,
        dwUnspecified1));
/*
    pDataWrapper = ALLOCATE_DATA_WRAPPER();
    if (pDataWrapper == NULL)
    {
        //
        //  We run out of memory, so can't reply.  Just return.
        //  

        CELOGMSG (ZONE_ERROR, 
            (TEXT("RNdis:: No mem in RndisSendRndisMessage()\r\n")));
        return;
    }
*/  
    switch (dwRndisMessageType)
    {
        ////////////////////////////////////////////////////////////////////////

        case REMOTE_NDIS_INITIALIZE_CMPLT:
        {
            PRNDIS_INITIALIZE_COMPLETE  pInitComplete;

            uiMessageSize = RNDIS_MESSAGE_SIZE(RNDIS_INITIALIZE_COMPLETE);  
            if (!(uiMessageSize<=MAX_PACKET_SIZE))
                KITLOutputDebugString("ASSERT Fail at uiMessageSize(%d)<=MAX_PACKET_SIZE(%d)\r\n",
                    uiMessageSize,MAX_PACKET_SIZE);
            //
            //  No extra buffer needed.
            //  AFListOffset and AFListSize don't seem to be needed.
            //  so let's just create the buffer and have it sent..
            //

            pDataWrapper = (DATA_WRAPPER *)AllocBuffer (&MsgBufferDesc, (WORD)(uiMessageSize+sizeof(DATA_WRAPPER))) ;//LocalAlloc(0, uiMessageSize);
            if (pDataWrapper == NULL)
            {
                KITLOutputDebugString("RNdis:: No mem for INITIALIZE_COMPLETE.\r\n");           
                return;
            }
            pRndisMessage = (PRNDIS_MESSAGE)(pDataWrapper+1);
            //
            //  Have the buffer, fill it in now..
            //

            pRndisMessage->NdisMessageType = dwRndisMessageType;
            pRndisMessage->MessageLength   = uiMessageSize;     

            pInitComplete = &(pRndisMessage->Message.InitializeComplete);
            
            pInitComplete->RequestId             = dwRequestId;
            pInitComplete->Status                = NdisStatus;
            pInitComplete->MajorVersion          = SUPPORTED_RNDIS_MAJOR_VER;
            pInitComplete->MinorVersion          = SUPPORTED_RNDIS_MINOR_VER;
            pInitComplete->DeviceFlags           = RNDIS_DF_CONNECTIONLESS;
            pInitComplete->Medium                = 0x00;    
            pInitComplete->MaxPacketsPerMessage  = RNDIS_MAX_PACKETS_PER_MESSAGE;           
            pInitComplete->AFListOffset          = 0x00;    
            pInitComplete->AFListSize            = 0x00;            
            pInitComplete->MaxTransferSize       = RndisMdd.dwDeviceMaxRx;

            //
            //   8 byte alignment, to cater for non x86 devices..
            //
            
            pInitComplete->PacketAlignmentFactor = 0x03;    

            CELOGMSG(1,
                (TEXT("RndisSendRndisMessage():: RNDIS_INIT_COMPLETE..\r\n")));

            break;
        }
        
        
        ////////////////////////////////////////////////////////////////////////
        
        case REMOTE_NDIS_QUERY_CMPLT:
        {
            PRNDIS_QUERY_COMPLETE   pQueryComplete;
            PUCHAR                  pucBuffer;          
            UINT                    uiRndisQuerySize;

            uiRndisQuerySize = RNDIS_MESSAGE_SIZE(RNDIS_QUERY_COMPLETE);            
            uiMessageSize    = uiRndisQuerySize + dwBufferLength;
            if (!(uiMessageSize<=MAX_PACKET_SIZE) || uiMessageSize < uiRndisQuerySize || uiMessageSize < dwBufferLength)
                KITLOutputDebugString("ASSERT Fail at uiMessageSize(%d)<=MAX_PACKET_SIZE(%d)\r\n",
                    uiMessageSize,MAX_PACKET_SIZE);
            
            //
            //  Extra space for queried information to be returned.
            //

            pDataWrapper = (DATA_WRAPPER *)AllocBuffer (&MsgBufferDesc, (WORD)(sizeof(DATA_WRAPPER)+uiMessageSize)) ;//LocalAlloc(0, uiMessageSize);
            if (pDataWrapper == NULL)
            {
                KITLOutputDebugString("RNdis:: No mem for INITIALIZE_COMPLETE.\r\n");           
                return;
            }
            pRndisMessage = (PRNDIS_MESSAGE)(pDataWrapper+1);

            pucBuffer = (PUCHAR)pRndisMessage + uiRndisQuerySize;

//          CELOGMSG (0, 
//              (TEXT("uiMessageSize = [%d] - dwBufferLength = [%d]\r\n"),
//              uiMessageSize,
//              dwBufferLength));


            //
            //  Have the buffer will fill now..
            //

            pRndisMessage->NdisMessageType = dwRndisMessageType;
            pRndisMessage->MessageLength   = uiMessageSize;     

            pQueryComplete = &(pRndisMessage->Message.QueryComplete);
            
            pQueryComplete->RequestId               = dwRequestId;          
            pQueryComplete->Status                  = NdisStatus;
            pQueryComplete->InformationBufferLength = dwUnspecified1;           

//          CELOGMSG (0, (TEXT("dwBufferLength = [%d]\r\n"),
//              dwBufferLength));

            pQueryComplete->InformationBufferOffset = 
                (dwBufferLength == 0) ? 
                    (0) : 
                    (pucBuffer - (PUCHAR)pQueryComplete);           

            if (dwBufferLength)             
                memcpy(pucBuffer, pucInBuffer, dwUnspecified1);

            break;
        }

        ////////////////////////////////////////////////////////////////////////        
        //  Both messages have same entries..
        //  They are only interested in NdisStatus..
        //

        case REMOTE_NDIS_KEEPALIVE_CMPLT:
        case REMOTE_NDIS_SET_CMPLT:
        {
            PRNDIS_KEEPALIVE_COMPLETE   pKeepAliveOrSetComplete;

            uiMessageSize = RNDIS_MESSAGE_SIZE(RNDIS_KEEPALIVE_COMPLETE);   
            if (!(uiMessageSize<=MAX_PACKET_SIZE))
                KITLOutputDebugString("ASSERT Fail at uiMessageSize(%d)<=MAX_PACKET_SIZE(%d)\r\n",
                    uiMessageSize,MAX_PACKET_SIZE);

            pDataWrapper = (DATA_WRAPPER *)AllocBuffer (&MsgBufferDesc, (WORD)(sizeof(DATA_WRAPPER)+uiMessageSize)) ;//LocalAlloc(0, uiMessageSize);
            if (pDataWrapper == NULL)
            {
                KITLOutputDebugString("RNdis:: No mem for INITIALIZE_COMPLETE.\r\n");           
                return;
            }
            pRndisMessage = (PRNDIS_MESSAGE)(pDataWrapper+1);

            pRndisMessage->NdisMessageType = dwRndisMessageType;
            pRndisMessage->MessageLength   = uiMessageSize;

            pKeepAliveOrSetComplete = &(pRndisMessage->Message.KeepaliveComplete);

            pKeepAliveOrSetComplete->RequestId = dwRequestId;
            pKeepAliveOrSetComplete->Status    = NdisStatus;
            break;
        }


        ////////////////////////////////////////////////////////////////////////

        case REMOTE_NDIS_RESET_CMPLT:
        {
            PRNDIS_RESET_COMPLETE   pResetComplete;

            uiMessageSize = RNDIS_MESSAGE_SIZE(RNDIS_KEEPALIVE_COMPLETE);   
            if (!(uiMessageSize<=MAX_PACKET_SIZE))
                KITLOutputDebugString("ASSERT Fail at uiMessageSize(%d)<=MAX_PACKET_SIZE(%d)\r\n",
                    uiMessageSize,MAX_PACKET_SIZE);
            pDataWrapper = (DATA_WRAPPER *)AllocBuffer (&MsgBufferDesc, (WORD)(sizeof(DATA_WRAPPER)+uiMessageSize)) ;//LocalAlloc(0, uiMessageSize);
            if (pDataWrapper == NULL)
            {
                KITLOutputDebugString("RNdis:: No mem for INITIALIZE_COMPLETE.\r\n");           
                return;
            }
            pRndisMessage = (PRNDIS_MESSAGE)(pDataWrapper+1);

            pRndisMessage->NdisMessageType = dwRndisMessageType;
            pRndisMessage->MessageLength   = uiMessageSize;

            pResetComplete = &(pRndisMessage->Message.ResetComplete);

            //
            //  Reset is always successful..
            //  We always require host to return the multicast list etc..
            //

            pResetComplete->Status          = NDIS_STATUS_SUCCESS;          
            pResetComplete->AddressingReset = 0x01;                     
            break;
        }
        
        ////////////////////////////////////////////////////////////////////////

        default:
            KITLOutputDebugString("RNdis:: Should not happen!\r\n");            
            //ASSERT(0);
            //FREE_DATA_WRAPPER(pDataWrapper);
            return;
    }


    //
    //  Send it out..
    //  Buffer will return back to us and freed in 
    //  MddSendRndisMessageComplete()
    //
    pDataWrapper->Link.Flink= pDataWrapper->Link.Blink=NULL;
    pDataWrapper->pucData    = (PUCHAR)pRndisMessage;
    pDataWrapper->dwDataSize = uiMessageSize;
    //PDD_SendRndisMessage(pDataWrapper);
    RndisMdd.PddCharacteristics.SendRndisMessageHandler(pDataWrapper);
//  CELOGMSG (ZONE_MEM,
//      (TEXT("**> RndisMsg send        :: buff[0x%x] l[%d] Wrap[0x%x]\r\n"),
//      pDataWrapper->pucData,
//      pDataWrapper->dwDataSize,
//      pDataWrapper));
/*
    {
        DWORD dwStartSec=OEMKitlGetSecs();
        bReturn=FALSE;
        do
        {   
            //
            //  PDD is idling, kick start the sending process..
            //
            if (PDD_SendRndisMessage(&DataWrapper)) {
                bReturn = TRUE;

            }
            PDD_ISR();
        }while (!bReturn && (OEMKitlGetSecs()- dwStartSec <1));

        if (bReturn) { //
            while (!PDD_IsMessageSend() && (OEMKitlGetSecs()- dwStartSec <1));
                PDD_ISR();
        }
    };
    if (!bReturn)
        KITLOutputDebugString("ASSERT Return(%d)\r\n",bReturn);
*/
//  ASSERT(bReturn);

}   //  RndisSendRndisMessage()



////////////////////////////////////////////////////////////////////////////////
//  RndisProcessMessage()
//  
//  Routine Description:
//
//      This function process RNDIS message received.  
//  
//  Arguments:
//
//      None.
//
//  Return Value:
//
//      TRUE if successful, FALSE otherwise.
//
void
RndisProcessMessage(PDATA_WRAPPER pDataWrapper)
{
    PRNDIS_MESSAGE  pRndisMessage;

//  CELOGMSG(0, 
//      (TEXT("RNdis:: RndisProcessOneMessage().\r\n")));

    #if 0
        DumpMemory( 
            pDataWrapper->pucData,
            pDataWrapper->dwDataSize);
    #endif

    pRndisMessage = (PRNDIS_MESSAGE) pDataWrapper->pucData;
    
    CELOGMSG (ZONE_RNDIS,
        (TEXT("RNdis:: Processing RndisMessage [%s] - Length [%d]\r\n"),
        pRndisMessage->NdisMessageType == REMOTE_NDIS_INITIALIZE_MSG      ? 
                TEXT("RNDIS_INITIALIZE") : 
        pRndisMessage->NdisMessageType == REMOTE_NDIS_HALT_MSG            ? 
                TEXT("RNDIS_HALT")       :
        pRndisMessage->NdisMessageType == REMOTE_NDIS_QUERY_MSG           ? 
                TEXT("RNDIS_QUERY")      :
        pRndisMessage->NdisMessageType == REMOTE_NDIS_SET_MSG             ?
                TEXT("RNDIS_SET")        :
        pRndisMessage->NdisMessageType == REMOTE_NDIS_RESET_MSG           ? 
                TEXT("RNDIS_RESET")      :
        pRndisMessage->NdisMessageType == REMOTE_NDIS_PACKET_MSG          ? 
                TEXT("RNDIS_PACKET")   :
        pRndisMessage->NdisMessageType == REMOTE_NDIS_INDICATE_STATUS_MSG ? 
                TEXT("RNDIS_INDICATE")   :
        pRndisMessage->NdisMessageType == REMOTE_NDIS_KEEPALIVE_MSG       ? 
            TEXT("RNDIS_KEEPALIVE")      :      
        TEXT("UNKNOWN!"),
        pRndisMessage->MessageLength));

/*  
    CELOGMSG (0, 
        (TEXT("RNdis:: Wrapper[0x%x] - Buffer[0x%x] l=[%d]!\r\n"),      
        pDataWrapper,
        pDataWrapper->pucData,
        pDataWrapper->dwDataSize)); 
*/  

    switch (pRndisMessage->NdisMessageType)
    {
        case REMOTE_NDIS_INITIALIZE_MSG:
        {
            PRNDIS_INITIALIZE_REQUEST   pInitializeRequest;
            NDIS_STATUS                 NdisStatus;
//          HANDLE                      hThread;

            pInitializeRequest = &pRndisMessage->Message.InitializeRequest;

            CELOGMSG (ZONE_RNDIS,
                (TEXT("RNdis:: ReqID[0x%x] - Ver[%d-%d] - MaxXfer[%d]\r\n"),
                pInitializeRequest->RequestId,
                pInitializeRequest->MajorVersion,
                pInitializeRequest->MinorVersion,
                pInitializeRequest->MaxTransferSize));

            //
            //  We support SUPPORTED_RNDIS_MAJOR_VERSION, so bail out if 
            //  it's not.           
            //

            if (pInitializeRequest->MajorVersion > SUPPORTED_RNDIS_MAJOR_VER  ||
                (pInitializeRequest->MajorVersion == SUPPORTED_RNDIS_MAJOR_VER &&
                 pInitializeRequest->MinorVersion > SUPPORTED_RNDIS_MINOR_VER))
            {
                CELOGMSG (ZONE_RNDIS,
                    (TEXT("RNdisMini Err!! unsupported RNDIS host ver.\r\n")));
                
                KITLOutputDebugString("RNDISMINI Err!! unsupported RNDIS host ver.\r\n");

                NdisStatus = NDIS_STATUS_FAILURE;               
            }
            else
            {
                ULONG   ulRequiredLength;               

                //
                //  The very first business is to make sure we are all in the 
                //  INIT state (i.e MDD & PDD).
                //  The reason is we may get this message at anytime even during
                //  initialized stated.
                //  For example this may happen when PDD stalls one of the end 
                //  point when it encounters error..  
                //  or when RNDIS host does not receive KEEP_ALIVE return within
                //  a specified period of time..
                //              

                if (RndisMdd.dwDeviceState != RNDIS_UNINITIALIZED)
                {
                    //
                    //  This will put us back to RNDIS_UNINITIALIZED state.
                    //
                    CELOGMSG (ZONE_RNDIS,(TEXT("ERROR! RndisInitialize msg received!!! \r\n")));
                    
                    KITLOutputDebugString("ERROR! RndisInitialize msg received!!! \r\n");

                    //
                    //  Well, DeInitialize RNDISMINI1 if necessary, but don't 
                    //  hard reset PDD..
                    //

                    RndisRestart(FALSE);
                }

                //ASSERT(RndisMdd.dwDeviceState == RNDIS_UNINITIALIZED);
                if (!(RndisMdd.dwDeviceState == RNDIS_UNINITIALIZED))
                    KITLOutputDebugString("ASSERT at File(%s) line (%d)\r\n",__THIS_FILE__, __LINE__);

                //
                //  Okay, we are in business.
                //  Initialize all we need, and return a reply..
                //

                NdisStatus = NDIS_STATUS_SUCCESS;               


                //
                //  Remember HOST's MAX RX.             
                //          

                RndisMdd.dwHostMaxRx = pInitializeRequest->MaxTransferSize;

                //
                //  Alloc memory for it, free the previous allocated chunk..
                //
/*
                if (RndisMdd.pucTxBuffer)
                    LocalFree(RndisMdd.pucTxBuffer);

                RndisMdd.pucTxBuffer = LocalAlloc(0, RndisMdd.dwHostMaxRx);

                if (RndisMdd.pucTxBuffer == NULL)
                {
                    CELOGMSG (ZONE_ERROR, 
                        (TEXT("RNdis:: Error, No mem for TX buff..\r\n")));             

                    NdisStatus = NDIS_STATUS_FAILURE;
                }
*/              

                //
                //  Get PDD's maximum RX buffer.
                //  

                if (NdisStatus == NDIS_STATUS_SUCCESS &&
                    //!PDD_Get(
                    !RndisMdd.PddCharacteristics.GetHandler(
                        REQ_ID_DEVICE_MAX_RX,
                        &(RndisMdd.dwDeviceMaxRx),
                        sizeof(RndisMdd.dwDeviceMaxRx),
                        &ulRequiredLength))
                {
                    //
                    //  This should never happen!!
                    //  a DWORD should be enough..
                    //

                    KITLOutputDebugString("ASSERT at File(%s) line (%d)\r\n",__THIS_FILE__, __LINE__);
                    NdisStatus = NDIS_STATUS_FAILURE;
                }


                KITLOutputDebugString("RndisMdd:: PDD's max RX buffer = [%d] bytes.\r\n",RndisMdd.dwDeviceMaxRx);               
            }


            //
            //  Send the reply out..
            //

            CELOGMSG (ZONE_RNDIS, 
                (TEXT("Send InitializeComplete Status[0x%x] ID[0x%x]\r\n"),
                NdisStatus,
                pInitializeRequest->RequestId));
            
            RndisSendRndisMessage(
                REMOTE_NDIS_INITIALIZE_CMPLT,
                pInitializeRequest->RequestId,                  
                NdisStatus,
                NULL,
                0x00,
                0x00);

            RndisMdd.dwDeviceState = RNDIS_INITIALIZED;

            //
            //  Instantiate CE Miniport driver RNDISMINI1
            //
/*
            hThread = CreateThread(0,
                         0,
                         (LPTHREAD_START_ROUTINE)RndisStartRNDISMINI1,
                         NULL,
                         0,                                         
                         NULL );

            CloseHandle(hThread);
*/
            break;
        }

        ////////////////////////////////////////////////////////////////////////

        case REMOTE_NDIS_QUERY_MSG:
        {
            PRNDIS_QUERY_REQUEST    pRndisRequest;
            NDIS_STATUS             NdisStatus;         
            ULONG                   ulBytesWritten;
            ULONG                   ulBytesNeeded;
            UCHAR                   *pucBuffer;


            if(RndisMdd.dwDeviceState == RNDIS_UNINITIALIZED)
                break;

            pRndisRequest = &pRndisMessage->Message.QueryRequest;

            CELOGMSG (ZONE_RNDIS, 
                (TEXT("RNdis:: Query: ID[0x%x]:OID[0x%x]:Buff[%d-%d]\r\n"),
                pRndisRequest->RequestId,
                pRndisRequest->Oid,
                pRndisRequest->InformationBufferLength,
                pRndisRequest->InformationBufferOffset));           

            
            //
            //  Special treatment for this OID.
            //  RNDIS host will only pass down 4 bytes, so let's just stuff it
            //  with our own buffer and blast it back to the RNDIS.
            //
    
            if (pRndisRequest->Oid == OID_GEN_SUPPORTED_LIST)
            {
                extern UINT RNdisMddSupportedOids[];

                pucBuffer = (PUCHAR)RNdisMddSupportedOids;
                
                HostMiniQueryInformation(
                        pRndisRequest->Oid,
                        NULL,
                        0x00,
                        &ulBytesWritten,
                        &ulBytesNeeded);

                ulBytesWritten  = ulBytesNeeded;
                ulBytesNeeded   = 0x00;
                NdisStatus      = NDIS_STATUS_SUCCESS;
            }
            else
            {
                //
                //  Pass this to our miniport handler..
                //

                pucBuffer  = 
                    (PUCHAR)((PUCHAR)pRndisRequest + 
                            pRndisRequest->InformationBufferOffset);

                if (!(
                    pRndisRequest->InformationBufferLength ==
                    (ULONG) (pDataWrapper->pucData + pDataWrapper->dwDataSize 
                             - pucBuffer)))
                    KITLOutputDebugString("ASSERT at File(%s) line (%d)\r\n",__THIS_FILE__, __LINE__);
//              CELOGMSG (0,
//                  (TEXT("RNdis:: Claimed length = [%d] actual [%d]\r\n"),
//                  pRndisRequest->InformationBufferLength,
//                  (pDataWrapper->pucData    + 
//                   pDataWrapper->dwDataSize -
//                   pucBuffer)));


                NdisStatus = 
                    HostMiniQueryInformation(
                        pRndisRequest->Oid,
                        pucBuffer,
                        pRndisRequest->InformationBufferLength,
                        &ulBytesWritten,
                        &ulBytesNeeded);

                
//              CELOGMSG (0, (TEXT("QueryInfo returns [%d]\r\n"),
//                  ulBytesWritten));

                #if 0
                    DumpMemory(
                        pucBuffer,
                        ulBytesWritten);
                #endif
            }


            //
            //  Reply back to host..
            //

            RndisSendRndisMessage(
                REMOTE_NDIS_QUERY_CMPLT,
                pRndisRequest->RequestId,
                NdisStatus,
                pucBuffer,
                ulBytesWritten,
                ulBytesWritten ? ulBytesWritten : ulBytesNeeded);


            break;
        }   

        ////////////////////////////////////////////////////////////////////////

        case REMOTE_NDIS_SET_MSG:
        {
            PRNDIS_SET_REQUEST      pRndisSet;
            NDIS_STATUS             NdisStatus;         
            ULONG                   ulBytesRead;
            ULONG                   ulBytesNeeded;
            UCHAR                   *pucBuffer;

            if(RndisMdd.dwDeviceState == RNDIS_UNINITIALIZED)
                break;

            pRndisSet = &pRndisMessage->Message.SetRequest;

//          CELOGMSG (0, 
//              (TEXT("RNdis:: Set: ID[0x%x]:OID[0x%x]:Buff[%d-%d]\r\n"),
//              pRndisSet->RequestId,
//              pRndisSet->Oid,
//              pRndisSet->InformationBufferLength,
//              pRndisSet->InformationBufferOffset));       
            
            pucBuffer  = 
                (PUCHAR)((PUCHAR)pRndisSet + 
                            pRndisSet->InformationBufferOffset);


            NdisStatus = HostMiniSetInformation(
                            pRndisSet->Oid,
                            pucBuffer,
                            pRndisSet->InformationBufferLength,
                            &ulBytesRead,
                            &ulBytesNeeded);

            //
            //  Reply back to host on the status..
            //
            
            RndisSendRndisMessage(
                REMOTE_NDIS_SET_CMPLT,
                pRndisSet->RequestId,
                NdisStatus,
                NULL,
                0,
                0);

            break;
        }       

        ////////////////////////////////////////////////////////////////////////

        case REMOTE_NDIS_KEEPALIVE_MSG:
        {
            PRNDIS_KEEPALIVE_REQUEST    pKeepAliveRequest;

            if(RndisMdd.dwDeviceState == RNDIS_UNINITIALIZED)
                break;

            CELOGMSG (ZONE_RNDIS,
                (TEXT("RNdis:: REMOTE_NDIS_KEEPALIVE_MSG.\r\n")));

            
            pKeepAliveRequest = &pRndisMessage->Message.KeepaliveRequest;

            //
            //  [stjong] DISCONNECTION HANDLING
            //  We probably need to keep track of this to detect 
            //  device being disconnected..
            //  
            

            //
            //  We are here host!!!
            //
            
            RndisSendRndisMessage(
                REMOTE_NDIS_KEEPALIVE_CMPLT,
                pKeepAliveRequest->RequestId,
                RNDIS_STATUS_SUCCESS,
                NULL,
                0,
                0);
        
            break;
        }       

        ////////////////////////////////////////////////////////////////////////

        case REMOTE_NDIS_HALT_MSG:
        {
            if(RndisMdd.dwDeviceState == RNDIS_UNINITIALIZED)
                break;

            KITLOutputDebugString("RndisMini recv REMOTE_NDIS_HALT_MSG from host!\r\n");

            //
            //  PDD needs to reset.
            //

            RndisRestart(TRUE);         

            break;
        }
        
        ////////////////////////////////////////////////////////////////////////

        case REMOTE_NDIS_RESET_MSG:
        {
            if(RndisMdd.dwDeviceState == RNDIS_UNINITIALIZED)
                break;          

            KITLOutputDebugString("RndisMini rcv REMOTE_NDIS_RESET_MSG from host!\r\n");

            //
            //  Per RNDIS spec, notify PDD to discard all outstanding messages
            //
            RndisMdd.PddCharacteristics.SetHandler(
                REQ_ID_SOFT_RESET,
                NULL,
                0x00);

            //
            //  Reply this message.
            //

            RndisSendRndisMessage(
                REMOTE_NDIS_RESET_CMPLT,
                0x00,
                0x00,
                NULL,
                0,
                0);

            break;
        }
        
        ////////////////////////////////////////////////////////////////////////

        case REMOTE_NDIS_INDICATE_STATUS_MSG:
        {
            if(RndisMdd.dwDeviceState == RNDIS_UNINITIALIZED)
                break;      
            break;      
        }
        
        ////////////////////////////////////////////////////////////////////////        

        default:
            KITLOutputDebugString("RNdis:: Unknown RNDIS msg, buff[0x%x] l=[%d]!\r\n",
                pDataWrapper->pucData,
                pDataWrapper->dwDataSize);      

            #if 0
                DumpMemory(
                    pDataWrapper->pucData,
                    pDataWrapper->dwDataSize);
            #endif
            break;
    }   


}   //  RndisProcessMessage()
#define RECV_RETRY_COUNT 10
UINT16 RndisEDbgGetFrame(BYTE * pData, UINT16 *pdwLength)
{
    BOOL bRet=FALSE;
    DWORD dwStartSec;
    DWORD dwWaitMSec;
    DWORD dwRetryCount;

//  KITLOutputDebugString("Rndis::RndisEDbgGetFrame RndisMdd.dwCurrentPacketFilter=%d \r\n",RndisMdd.dwCurrentPacketFilter);

    dwStartSec=OEMKitlGetSecs();

    while  (RndisMdd.dwCurrentPacketFilter == 0 &&  OEMKitlGetSecs()-dwStartSec<2) 
        //PDD_ISR();
        RndisMdd.PddCharacteristics.ISRHandler(&dwWaitMSec);

    if (pData && pdwLength && *pdwLength) {
        PVOID pDataBuffer=NULL;
        WORD  wLength;
        dwStartSec=OEMKitlGetSecs();
        bRet=FALSE;
/*
        while (InBuffer.dwDataSize==0 &&(OEMKitlGetSecs()- dwStartSec <2))
            PDD_ISR();
        if (InBuffer.dwDataSize!=0) {
            *pdwLength=(UINT16)min (*pdwLength , InBuffer.dwDataSize);
            memcpy(pData,(PVOID)InBuffer.rcvBuffer,*pdwLength);
            bRet=TRUE;
        }
        else 
            *pdwLength=0;
*/
        dwRetryCount=0;
        while ((GetFirstUsedBuffer(&EthBufferDesc,&pDataBuffer, &wLength)==FALSE || pDataBuffer==NULL)
                && dwRetryCount++<RECV_RETRY_COUNT) 
//          PDD_ISR();
            RndisMdd.PddCharacteristics.ISRHandler(&dwWaitMSec);

        if (GetFirstUsedBuffer(&EthBufferDesc,&pDataBuffer, &wLength)==TRUE && pDataBuffer!=NULL) {
            wLength=min(*pdwLength,wLength);
            memcpy(pData,pDataBuffer,wLength);
            *pdwLength=wLength;
            FreeBuffer(&EthBufferDesc,pDataBuffer);
            CELOGDUMPDATA(ZONE_DATA,CELID_RAW_UCHAR,pData,(WORD)*pdwLength);
        }
        else 
            *pdwLength=0;
//      InBuffer.dwDataSize=0;
//      KITLOutputDebugString("Rndis::RndisEDbgGetFrame  return %d bytes\r\n",*pdwLength);
        return *pdwLength;
    }
    else
        return 0;
}



////////////////////////////////////////////////////////////////////////////////
//  RndisProcessPacket()
//  
//  Routine Description:
//
//      This function process one RNDIS packet received.  
//  
//  Arguments:
//
//      None.
//
//  Return Value:
//
//      None.
//
BOOL 
RndisProcessPacket (PDATA_WRAPPER pDataWrapper)
{
//  PRCV_RNDIS_PACKET   pRcvRndisPacket = NULL;
//  BOOL                bReturnImmediately = TRUE;
    PRNDIS_MESSAGE  pRndisMessage = (PRNDIS_MESSAGE) pDataWrapper->pucData; 
    DWORD           dwLengthRemaining = pDataWrapper->dwDataSize;
    DWORD           dwPacketCount=0;

    static DWORD dwOverRunCount=0;
    CELOGMSG (ZONE_DATA,(TEXT("Rndis::ProcessPacket (%d Bytes) \r\n"),dwLengthRemaining));
    if (!(RndisMdd.dwDeviceState != RNDIS_UNINITIALIZED))
        KITLOutputDebugString("ASSERT at File(%s) line (%d)\r\n",__THIS_FILE__, __LINE__);


//  CELOGMSG(0, 
//      (TEXT("RNdis:: RndisProcessPacket().\r\n")));   
    

    while (pRndisMessage  && dwLengthRemaining >=  RNDIS_MESSAGE_SIZE(RNDIS_PACKET) ) {
        PVOID pBuffer;
        PRNDIS_PACKET pRndisPacket = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pRndisMessage);

        if (pRndisMessage->NdisMessageType != REMOTE_NDIS_PACKET_MSG)
        {
            //
            //  This packet is to be returned immediately..
            //
            CELOGMSG (ZONE_DATA,(TEXT("Rndis::ProcessPacket Wrong Data Type 0x%x\n\r\r\n")));
            KITLOutputDebugString("Wrong Data Type 0x%x\r\n",pRndisMessage->NdisMessageType);
            return FALSE;
        };

        CELOGMSG (ZONE_DATA,(TEXT("Rndis::ProcessPacket Message Length is %d\r\n"),pRndisMessage->MessageLength));
        if (pRndisMessage->MessageLength > dwLengthRemaining) {
            CELOGMSG (ZONE_ERROR,
                    (TEXT("RNdis:: Invalid RndisPacket l[%d] rem [%d]\r\n"),
                    pRndisMessage->MessageLength,
                    dwLengthRemaining));         
            KITLOutputDebugString("RNdis:: Invalid RndisPacket l[%d] rem [%d]\r\n",
                    pRndisMessage->MessageLength,dwLengthRemaining);
            return FALSE;
        }

        if (pRndisPacket->DataLength > pRndisMessage->MessageLength) {
            KITLOutputDebugString("Wrong Data DataLength Msg=0x%x,Pkt=0x%x\n\r",
                pRndisMessage->NdisMessageType,pRndisPacket->DataLength);
            return FALSE;
        }
/*      CELOGMSG (ZONE_DATA,(TEXT("Rndis::ProcessPacket Existing Data %d byte \r\n"),InBuffer.dwDataSize));
        if ( InBuffer.dwDataSize!=0) {
            KITLOutputDebugString("Overrun=0x%x\n\r",++dwOverRunCount);
        }
        InBuffer.dwDataSize=min(pRndisPacket->DataLength,MAX_PACKET_SIZE);
        memcpy((PVOID)InBuffer.rcvBuffer,GET_PTR_TO_RNDIS_DATA_BUFF(pRndisPacket),InBuffer.dwDataSize);
*/
        if (pRndisPacket->DataLength>1520) { 
            KITLOutputDebugString("Buffer is too big 0x%x\n\r",pRndisPacket->DataLength);
            CELOGMSG (ZONE_ERROR,(TEXT("Buffer is too big 0x%x\n\r"),pRndisPacket->DataLength));
            return FALSE;
        };
        pBuffer= AllocBuffer (&EthBufferDesc, (WORD)pRndisPacket->DataLength);

        if (pBuffer) {
            CELOGMSG (ZONE_DATA,(TEXT("Rndis::ProcessPacket Existing Data %d \r\n"),pRndisPacket->DataLength));
            memcpy(pBuffer,GET_PTR_TO_RNDIS_DATA_BUFF(pRndisPacket),pRndisPacket->DataLength);
            CELOGDUMPDATA(ZONE_DATA,CELID_RAW_UCHAR,pBuffer,(WORD)pRndisPacket->DataLength);
            dwPacketCount++;
        }
        else {
            KITLOutputDebugString("Overrun=0x%x\n\r",++dwOverRunCount);
            CELOGMSG (ZONE_ERROR,(TEXT("Overrun=0x%x\n\r"),++dwOverRunCount));
            return FALSE;
        };

        dwLengthRemaining -= pRndisMessage->MessageLength;
        pRndisMessage     = 
                (PRNDIS_MESSAGE)(((BYTE *)pRndisMessage) +
                                  pRndisMessage->MessageLength);

    };
    CELOGMSG (ZONE_DATA,(TEXT("Rndis::ProcessPacket %d packet copied\r\n"),dwPacketCount));
    return TRUE;



//  CELOGMSG (0, 
//      (TEXT("RNdis:: Wrapper[0x%x] - Buffer[0x%x] l=[%d]!\r\n"),
//      pDataWrapper,
//      pDataWrapper->pucData,
//      pDataWrapper->dwDataSize));     

//  CELOGMSG (0,
//      (TEXT(">>>>>>>>   RNDIS_PACKET [0x%x] - [%d] bytes..\r\n"),
//      pDataWrapper,
//      pDataWrapper->dwDataSize));


    //
    //  Start Processing RNDIS packet.
    //
/*
    do
    {
        DWORD               dwNumberOfPackets = 0x00;
        DWORD               dwLengthRemaining = pDataWrapper->dwDataSize;
        PRNDIS_PACKET       pRndisPacket;
        PNDIS_PACKET        PacketArray[RNDIS_MAX_PACKETS_PER_MESSAGE];
        PNDIS_PACKET        pNdisPacket;
        PNDIS_BUFFER        pNdisBuffer;
        NDIS_STATUS         NdisStatus;


        //
        //  toss it immediately if it is not RNDIS_PACKET message.
        //

        if (pRndisMessage->NdisMessageType != REMOTE_NDIS_PACKET_MSG)
        {
            //
            //  This packet is to be returned immediately..
            //

            break;
        }   


        //
        //  Allocate the RCV_RNDIS_PACKET, if we can't return the PDD
        //  buffer immediately..
        //
        
        pRcvRndisPacket = ALLOCATE_RCV_RNDIS_PACKET();
        
        if (pRcvRndisPacket == NULL)
            break;      

        CELOGMSG (ZONE_MEM,
            (TEXT("%%> RcvRndisPacket[0x%x] alloced.\r\n"),
            pRcvRndisPacket));

        memset(
            pRcvRndisPacket,
            0x00,
            sizeof(RCV_RNDIS_PACKET));

        CELOGMSG (0, 
            (TEXT("Initially: pRcvRndisPacket->dwReturnsPending == [%d]\r\n"),
            pRcvRndisPacket->dwReturnsPending));
        

        pRcvRndisPacket->pDataWrapper = pDataWrapper;

        //
        //  Prepare NDIS packets for indicating up. 
        //
        
        do
        {
            pRndisPacket = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pRndisMessage);

            //
            // Some sanity checks.
            //
            
            if (pRndisMessage->MessageLength > dwLengthRemaining)
            {
                CELOGMSG (ZONE_ERROR,
                    (TEXT("RNdis:: Invalid RndisPacket l[%d] rem [%d]\r\n"),
                    pRndisMessage->MessageLength,
                    dwLengthRemaining));         
                
                break;
            }

            if (pRndisPacket->DataLength > pRndisMessage->MessageLength)
            {
                CELOGMSG (ZONE_ERROR,
                    (TEXT("RNdis:: Err! RndisPacket l[%d]>MsgLen[%d]\r\n"),
                    pRndisPacket->DataLength,
                    pRndisMessage->MessageLength));
                
                break;
            }

            //
            //  Allocate an NDIS packet to do the indication with.            
            //  

            NdisAllocatePacket(
                &NdisStatus, 
                &pNdisPacket, 
                NdisPacketPool);


            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                pNdisPacket = NULL;

                CELOGMSG (ZONE_ERROR,
                    (TEXT("RNdis:: Err! no mem for NDIS_PACKET...\r\n")));      
                
                break;
            }


            //
            //  Protocol driver can keep the packet and return later..
            //

            NDIS_SET_PACKET_HEADER_SIZE(pNdisPacket, 14);
            NDIS_SET_PACKET_STATUS(pNdisPacket, NDIS_STATUS_SUCCESS);
            NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(
                pNdisPacket, NULL, 0);


            //
            //  Here is the NDIS_BUFFER wrapper.
            //

            NdisAllocateBuffer(
                &NdisStatus,
                &pNdisBuffer,
                NdisBufferPool,
                GET_PTR_TO_RNDIS_DATA_BUFF(pRndisPacket),
                pRndisPacket->DataLength);

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                CELOGMSG (ZONE_ERROR, 
                    (TEXT("RNdis:: Err! non mem for NDIS_BUFFER..\r\n")));          
                
                NdisFreePacket(pNdisPacket);                
                break;
            }


            //
            //  Cool, by here we have NDIS_PACKET and NDIS_BUFFER..
            //  Link them and queue them in PacketArray.
            //

            NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);

            #if 0
                CELOGMSG (ZONE_RNDIS, (TEXT("RX packet [%d] bytes: \r\n"),
                    pNdisBuffer->BufferLength));
                DumpMemory(
                    pNdisBuffer->VirtualAddress,
                    MIN(48, pNdisBuffer->BufferLength));
            #endif

            CELOGMSG (0, (TEXT(">> [%d] bytes.\r\n"),
                pNdisBuffer->BufferLength));

            PacketArray[dwNumberOfPackets] = pNdisPacket;
            dwNumberOfPackets++;        

            *(RESERVED_FROM_RECV_PACKET(pNdisPacket)) = pRcvRndisPacket;

            //
            //  Next packet please..
            //

            dwLengthRemaining -= pRndisMessage->MessageLength;            
            pRndisMessage     = 
                (PRNDIS_MESSAGE)((ULONG_PTR)pRndisMessage +
                                  pRndisMessage->MessageLength);

            NdisInterlockedIncrement(
                &pRcvRndisPacket->dwReturnsPending);

            CELOGMSG (0, 
                (TEXT("pRcvRndisPacket->dwReturnsPending == [%d]\r\n"),
                pRcvRndisPacket->dwReturnsPending));



            //
            //  See if we have reached the maximum, or there is no packet
            //  left in the RNDIS_PACKET message.
            //  We should have told RNDIS host the max ndis packet it may
            //  include in one RNDIS_MESSAGE.   So if it goes beyond that
            //  we toss 'em..
            //

            if ((dwNumberOfPackets == RNDIS_MAX_PACKETS_PER_MESSAGE) ||
                (dwLengthRemaining <  RNDIS_MESSAGE_SIZE(RNDIS_PACKET)))
            {
                ASSERT(dwNumberOfPackets < RNDIS_MAX_PACKETS_PER_MESSAGE);
                break;
            }
        }   
        while (dwLengthRemaining >= RNDIS_MESSAGE_SIZE(RNDIS_PACKET));


        //
        //  Indicate to VMINI which will pass it to CE stack.
        //  Each packet will then come back via RndisReturnIndicatedPacket()
        //  when the dwReturnsPending == 0 we then release the
        //  pRcvRndisPacket and return the PDATA_WRAPPER to PDD.
        //

        if (dwNumberOfPackets)
        {
            ASSERT(pRcvRndisPacket->dwReturnsPending == dwNumberOfPackets);

            CELOGMSG (ZONE_RNDIS, 
                (TEXT("IND to RNDISMINI1 [0x%x]: DataWrap[0x%x]-[%d]pkts.\r\n"),
                pRcvRndisPacket,
                pRcvRndisPacket->pDataWrapper,
                dwNumberOfPackets));

            VMiniIndicatePackets(
                PacketArray,
                dwNumberOfPackets);     

            RndisMdd.dwTransmitOkay += dwNumberOfPackets;       

            //
            //  CE VMINI will call RndisReturnIndicatedPacket() to return 
            //   this packet.
            //

            bReturnImmediately = FALSE;
        }

    } 
    while (FALSE);

    //
    //  If we don't need to retain this packet, return to PDD immediately..
    //
        
    if (bReturnImmediately)
    {       
        //
        //  indicate this packet is returned immediately..
        //
        
        RndisMdd.PddCharacteristics.IndicateRndisPacketCompleteHandler(
            pDataWrapper);  

        if (pRcvRndisPacket)
        {
            FREE_RCV_RNDIS_PACKET(pRcvRndisPacket);
            CELOGMSG (ZONE_MEM,
                (TEXT("%%> RcvRndisPacket[0x%x] freed.\r\n"),
                pRcvRndisPacket));
        }
    }           
*/
}   //  RndisProcessPacket()



////////////////////////////////////////////////////////////////////////////////
//  RndisReturnIndicatedPacket()
//  
//  Routine Description:
//
//      Handle packet return sent to VMini earlier..  
//  
//  Arguments:
//
//      pNdisPacket :: The packet we indicate to VMini in 
//                     VMiniIndicatePackets().
//
//  Return Value:
//
//      TRUE if successful, FALSE otherwise.
//
/*
void
RndisReturnIndicatedPacket(
    PNDIS_PACKET    pNdisPacket)
{
    PRCV_RNDIS_PACKET   pRcvRndisPacket;
    DWORD               dwReturnsPending;
    PNDIS_BUFFER        pNdisBuffer;

    pRcvRndisPacket  = *(RESERVED_FROM_RECV_PACKET(pNdisPacket));
    dwReturnsPending = NdisInterlockedDecrement(
                            &pRcvRndisPacket->dwReturnsPending);

    CELOGMSG (ZONE_RNDIS, 
        (TEXT("RNDISMINI1 Returns[0x%x] : Wrap[0x%x] Pend[%d]\r\n"),
        pRcvRndisPacket,
        pRcvRndisPacket->pDataWrapper,
        dwReturnsPending));

    NdisQueryPacket(
        pNdisPacket,
        NULL,
        NULL,
        &pNdisBuffer,
        NULL);

    NdisFreePacket(pNdisPacket);
    NdisFreeBuffer(pNdisBuffer);

    if (dwReturnsPending == 0)
    {
        //
        //  All NDIS_PACKETs consumed, we can now return the PDATA_WRAPPER
        //  passed to us in RndisProcessPacket() and then release the 
        //  RCV_RNDIS_PACKET wrapper.
        //

        CELOGMSG (0, 
            (TEXT("RNdis:: VMini consumed RNDIS_PACKET[0x%x]..\r\n"),
            pRcvRndisPacket));

        RndisMdd.PddCharacteristics.IndicateRndisPacketCompleteHandler(
            pRcvRndisPacket->pDataWrapper);
        
        FREE_RCV_RNDIS_PACKET (pRcvRndisPacket);

        CELOGMSG (ZONE_MEM,
            (TEXT("%%> RcvRndisPacket[0x%x] freed.\r\n"),
            pRcvRndisPacket));
    }

}   //  RndisReturnIndicatedPacket()

*/



LPVOID
NKCreateStaticMapping(
    DWORD dwPhysBase,
    DWORD dwSize
    ) ;




////////////////////////////////////////////////////////////////////////////////
//  RndisInit()
//  
//  Routine Description:
//
//      Our one and only chance to init..
//  
//  Arguments:
//
//      None.
//
//  Return Value:
//
//      TRUE if successful, FALSE otherwise.
//

BOOL RndisInit( BYTE *pbBaseAddress, DWORD dwLogicalAddress, USHORT MacAddr[3])
{
    DWORD dwWaitMSec;
#define INITIALTIME_SEC 100
//  NDIS_STATUS     Status;
//  DWORD dwSysInt=0;

    KITLOutputDebugString("Rndis:: initialization: with addr=%x\r\n",pbBaseAddress);
    if (pbBaseAddress == NULL) { // Open the following 
        pbBaseAddress=
            (PBYTE)NKCreateStaticMapping((DWORD)dwLogicalAddress>>8,0x100000L);
        KITLOutputDebugString("Rndis:: Address static map to addr=%x\r\n",pbBaseAddress);
    };
    memset(&RndisMdd,0,sizeof(RndisMdd));
    RndisMdd.bPddSending = FALSE;
    KITLOutputDebugString("Rndis:: initialization!\r\n");

// Initialize The Buffer
    InitBuffer(&MsgBufferDesc, MsgBuffer , MAX_PACKET_SIZE+4*(sizeof(DATA_WRAPPER)+sizeof(DWORD)));
// Initialize The Ethnet Frame Buffer
    InitBuffer(&EthBufferDesc, EthBuffer , (MAX_ETH_BLOCK+sizeof(DWORD))*10);

    if (PDDInit(&(RndisMdd.PddCharacteristics), pbBaseAddress)) {
        DWORD dwStartSec;
        ULONG ulRequiredLength=0;
        KITLOutputDebugString("Rndis:: PDDInit Success!\r\n");
        RndisMdd.dwDeviceState = RNDIS_UNINITIALIZED;
        if (!RndisMdd.PddCharacteristics.GetHandler(
            REQ_ID_DEVICE_MACADDR,
            MacAddr,
            6,
            &ulRequiredLength))
        {
            KITLOutputDebugString("Rndis:: Err no MAC address read from PDD.\r\n");
            return FALSE;
        }
        memcpy(RndisMdd.PermanentNwAddr,MacAddr,6);
        RndisMdd.PermanentNwAddr[0] |= 0x02;
        KITLOutputDebugString("Rndis:: Get MAC address %x,%x,%x\r\n",MacAddr[0],MacAddr[1],MacAddr[2]);
        dwStartSec=OEMKitlGetSecs();
        while (OEMKitlGetSecs()-dwStartSec<=INITIALTIME_SEC) {
//          PDD_ISR();
            RndisMdd.PddCharacteristics.ISRHandler(&dwWaitMSec);
            if (RndisMdd.dwDeviceState == RNDIS_INITIALIZED) {
                break;
            }

        }
    }
    if (RndisMdd.dwDeviceState == RNDIS_INITIALIZED) {
        DWORD dwStartSec=OEMKitlGetSecs();
        KITLOutputDebugString("Rndis:: initialization: Success\r\n");
        while (OEMKitlGetSecs()-dwStartSec<=2) {
            PVOID pDataBuffer=NULL;
            WORD  wLength;
            RndisMdd.PddCharacteristics.ISRHandler(&dwWaitMSec);
            // Dump Any data we recevied.
            if (GetFirstUsedBuffer(&EthBufferDesc,&pDataBuffer, &wLength)==TRUE && pDataBuffer!=NULL) {
                FreeBuffer(&EthBufferDesc,pDataBuffer);
            };
        };
        return TRUE;
    }
    else {
        KITLOutputDebugString("Rndis:: initialization: Fail!\r\n");
        return FALSE;
    };
}   //  RndisInit()

DWORD RndisGetIrq() {
        // We do not have any other way to return IRQ.
    return RndisMdd.PddCharacteristics.dwIRQ;
};
void RndisGetPCICard(PDWORD pdwBaseAddr, PBYTE pucIrq)
{
    *pdwBaseAddr=RndisMdd.PddCharacteristics.dwBaseAddr;
    *pucIrq=(BYTE)RndisMdd.PddCharacteristics.dwIRQ;
}
////////////////////////////////////////////////////////////////////////////////
//  RndisDeInit()
//  
//  Routine Description:
//
//      Reverse what's done in RndisInit()
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
RndisDeInit(void)
{
/*
    NdisFreePacketPool(NdisPacketPool);
    NdisFreeBufferPool(NdisBufferPool);
    SetEvent(g_TxEvent);    

    ExDeleteNPagedLookasideList(&RcvRndisPacketLookAsideList);
*/
}   //  RndisDeInit()

void
RndisCurrentPacketFilter(DWORD dwFilter)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwFilter);
    
    // Do know how.
}


BOOL
RndisMulticastList(PUCHAR pucMulticastAddresses, DWORD dwNoOfAddresses)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pucMulticastAddresses);
    UNREFERENCED_PARAMETER(dwNoOfAddresses);
    
    return TRUE;
}
DWORD 
RndisGetPendingInts(void)
{
    DWORD dwWaitMSec;
    //KITLOutputDebugString("+NE2000GetPendingInts\r\n");
//  PDD_ISR();
    RndisMdd.PddCharacteristics.ISRHandler(&dwWaitMSec);
    //KITLOutputDebugString("-NE2000GetPendingInts\r\n");
    if (GetFirstUsedBuffer(&EthBufferDesc,NULL,NULL)) {
#ifdef DEBUG_SINGLE_CHAR
        KITLOutputDebugString("R"); // indicating a received packet
#endif
        return INTR_TYPE_RX;    
    }
#ifdef DEBUG_SINGLE_CHAR
    KITLOutputDebugString("B"); // discarding non-ARP broadcast traffic.
#endif
    return 0;    
}   // NE2000GetPendingInts
void RndisEnableInts (void )
{
    RndisMdd.PddCharacteristics.SetHandler(REQ_ID_ENABLE_INT,NULL,0);

};
void RndisDisableInts (void)
{
    RndisMdd.PddCharacteristics.SetHandler(REQ_ID_DISABLE_INT,NULL,0);

};


DWORD  RndisSetOptions(DWORD dwOptions)
{
    return dwOptions;

}


void MddInitializeCriticalSection(CRITICAL_SECTION *pCS) 
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pCS);
}

void MddDeleteCriticalSection(CRITICAL_SECTION *pCS)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pCS);
}

void MddEnterCriticalSection(CRITICAL_SECTION *pCS)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pCS);
}

void MddLeaveCriticalSection(CRITICAL_SECTION *pCS)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pCS);
}

void RndisMiniCeLogMsg (LPCTSTR szFormat,... )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(szFormat);
    
/*  TCHAR szBuffer[1024];
    va_list pArgs; 
    va_start(pArgs, szFormat);
    _vsntprintf(szBuffer, sizeof(szBuffer)/sizeof(TCHAR), szFormat, pArgs);
    va_end(pArgs);
    CeLogData(TRUE,CELID_RAW_WCHAR,szBuffer,(WORD)((_tcslen(szBuffer)+1)*sizeof(TCHAR)),
        CELZONE_ALWAYSON, 0, 0, FALSE);
*/
};
