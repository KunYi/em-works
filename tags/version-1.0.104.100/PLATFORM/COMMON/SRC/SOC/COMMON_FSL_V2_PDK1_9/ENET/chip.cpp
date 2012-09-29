//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  chip.c
//
//  Implementation of ENET Driver
//
//  This file implements hardware related functions for ENET.
//
//-----------------------------------------------------------------------------
#include "common_macros.h"
#include "enet.h"
#include "phys.h"


//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// File-local(static) Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

extern "C"  BOOL BSPENETIomuxConfig( DWORD index,IN BOOL Enable );
extern "C"  BOOL BSPENETClockConfig( DWORD index,IN BOOL Enable );
extern "C"  BOOL BSPENETusesRMII(void);
extern "C"  void BSPENETPowerEnable(DWORD index,BOOL bEnable);
extern VOID ExchangeFrame( UINT8 *pSrc, UINT32 cnt);
extern PVOID pv_HWregENET0;
extern PVOID pv_HWregENET1;
extern HANDLE hMiiMutex;


//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Functions implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETStartXmit
//
// This function gets the transmit data and sets the transmit buffer descriptors
// for transmitting process.
//
// Parameters:
//        pEthernet
//            [in]  Specifies the driver allocated context area in which the driver
//                  maintains ENET adapter state, set up by ENETInitialize.
// 
// Return value:
//        TRUE for success, FALSE for failure.
//
//------------------------------------------------------------------------------
BOOL ENETClass::ENETStartXmit(IN NDIS_HANDLE MiniportAdapterHandle )
{
    volatile PEBUFFER_DESC BufferDescPointer;
    UINT Txindex;
    PUCHAR MemAddr;
    PUCHAR BkMemAddr=NULL;
    UINT16 EthernetType=0x0;;

    // Packet size of the packet
    UINT PacketSize;
    UINT CurrenSize = 0;
    PNDIS_BUFFER pNDISBuffer;

    // Holds virtual address of the current buffer
    PUCHAR CurrentBufAddress;

    // Holds the length of the current buffer of the packet
    DWORD BufferLength;

    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));

    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETStartXmit\r\n")));
    
    //if(pENET->LinkStatus == FALSE)
    //{
        // Link is down or autonegotiation is in progress
    //    return FALSE;
    //}
    
    BufferDescPointer = pENET->CurrentTx;
    
    if(BufferDescPointer->ControlStatus & BD_ENET_TX_READY)
    {
        // Transmit buffers are full
        return FALSE;
    }
    
    // Clear all of the status flags
    BufferDescPointer->ControlStatus &= ~BD_ENET_TX_STATS; 
    BufferDescPointer->BDU = 0x0;
    BufferDescPointer->Control2Status = TX_BD_INT ;
    Txindex = BufferDescPointer - pENET->TxBufferDescBase;
    MemAddr = pENET->TxBuffAddr[Txindex];
    BkMemAddr=MemAddr;
    
    // Check whether the first packet is NULL
    if(pEthernet->HeadPacket == NULL)
    {
        return FALSE;
    }
    
    // Get the length of the packet and the pointer to NDIS Buffer
    NdisQueryPacket(pEthernet->HeadPacket, NULL, NULL, &pNDISBuffer, &PacketSize);
    
    NdisQueryBuffer(pNDISBuffer, (PVOID *)&CurrentBufAddress, (PUINT)&BufferLength);
    
    while(pNDISBuffer)
    {
        if((BufferLength != 0) && (CurrentBufAddress != NULL))
        {
            memcpy(MemAddr, CurrentBufAddress, BufferLength);
            MemAddr += BufferLength;
            CurrenSize += BufferLength;
        }
        
        NdisGetNextBuffer(pNDISBuffer, &pNDISBuffer);
                
        if (pNDISBuffer)
        {
            NdisQueryBuffer(pNDISBuffer, (PVOID *)&CurrentBufAddress, (PUINT)&BufferLength);
        }
    
    }
    
    //check the EthernetType is PTPV2 frame
    PREFAST_SUPPRESS(6385, "Reading invalid data"); 
    EthernetType=(BkMemAddr[0xc]<<8)+BkMemAddr[0xd];
    
    if(EthernetType==PTP_TYPE)
    {
        if(BkMemAddr[0xe]<0x4)
        {
            BufferDescPointer->Control2Status |= TX_BD_TS ;
        }
    }    
    ExchangeFrame(BkMemAddr, CurrenSize+ (4 - CurrenSize%4));
    
    //We enable ff_tx_ts_frm when we send the event PTPv2 message   
    
    // set up the transmit buffer descriptor
    BufferDescPointer->ControlStatus |= (BD_ENET_TX_READY  |
                                         BD_ENET_TX_LAST  | BD_ENET_TX_TC);
                                         
    BufferDescPointer->DataLen = (USHORT)PacketSize;
    
    BW_ENET_MAC_TDAR_TDAR(index,1);
    
    // If this was the last BD in the ring, start at the begining again
    if(BufferDescPointer->ControlStatus & BD_ENET_TX_WRAP)
    {
        BufferDescPointer = pENET->TxBufferDescBase;
    }
    else
    {
        BufferDescPointer++;
    }
    
    if(BufferDescPointer == pENET->DirtyTx)
    {
        pENET->TxFull = TRUE;
    }
    
    pENET->CurrentTx = (PEBUFFER_DESC)BufferDescPointer;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETStartXmit\r\n")));
    
    return TRUE;
}



//------------------------------------------------------------------------------
// MII management related functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETSetMII
//
// This function sets the clock for the MII interface
//
// Parameters:
//        pENET
//            [in] Pointer to a structure which contains the status and control
//                 information for the ENET controller and MII
// Return Value:
//        None.
//
//------------------------------------------------------------------------------ 
void  ENETClass::ENETSetMII(  IN PENET_ENET_PRIVATE pENET  )
{

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETSetMII\r\n")));
   
    BW_ENET_MAC_RCR_MAX_FL(index,PKT_MAXBUF_SIZE);
    BW_ENET_MAC_RCR_MII_MODE(index,1);                    
    HW_ENET_MAC_TCR_WR(index,0x0);
    // if(index==MAC0)
    {
        if (pENET->fUseRMII == FALSE)
        {
            // Set MII speed to 2.5MHz
            pENET->MIIPhySpeed = 40;
            BW_ENET_MAC_RCR_RMII_MODE(index, 0);
    
        }
        else
        {
            // Set MII speed
            pENET->MIIPhySpeed = 40;//4
            BW_ENET_MAC_RCR_RMII_MODE(index, 1);
        }

        BW_ENET_MAC_MSCR_MII_SPEED(index,pENET->MIIPhySpeed);
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETSetMII\r\n")));
}


//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETQueueMII
//
// This function adds a MII management command to the MII command list
//
// Parameters:
//        pENET
//            [in] Pointer to a structure which contains the status and control
//                 information for the ENET controller and MII
//        RegValue
//            [in] The MII command which will be added to the MII command list
//                 
//        OpFunc
//          [in] An optional function which will be performed when the MII interrupt
//                 arrived.
// Return Value:
//        TRUE if the MII command has been added to the command list successfully,
//      FALSE if the command list is full.
//
//------------------------------------------------------------------------------
BOOL ENETClass::ENETQueueMII(
    IN PENET_ENET_PRIVATE pENET,
    IN UINT RegValue,
    IN void (*OpFunc)(UINT, NDIS_HANDLE)
    )
{
    BOOL RetVal = TRUE;
    PENET_MII_LIST MIIPoint;

    WaitForSingleObject(hMiiMutex, INFINITE);
 
    // add PHY address to the MII command
    RegValue |= (pENET->MIIPhyAddr << BP_ENET_MAC_MMFR_PA);
    
    if((MIIPoint = gMIIFree) != NULL)
    {
        gMIIFree = MIIPoint->MIINext;
        MIIPoint->MIIRegVal = RegValue;
        MIIPoint->MIIFunction = OpFunc;
        MIIPoint->MIINext = NULL;
        
        if(gMIIHead) 
        {
            gMIITail->MIINext = MIIPoint;
            gMIITail = MIIPoint;    
        }
        else
        {
            gMIIHead = gMIITail = MIIPoint;
    
            HW_ENET_MAC_MMFR_WR(MAC0,RegValue);
        }
    }
    else
    {
        RetVal = FALSE;
    }
   ReleaseMutex(hMiiMutex);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETQueueMII\r\n")));
    
    return RetVal;
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETDoMIICmd
//
// This function will call ENETQueueMII to queue all the requested MII management
// commands to the sending list.
//
// Parameters:
//        pENET
//            [in] Points to a structure which contains the status and control
//                 information for the ENET controller and MII
//
//        pCmd
//            [in] Points to a ENET_PHY_CMD array which specifies the MII management
//                 commands and the parsing functions
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETDoMIICmd(
    IN PENET_ENET_PRIVATE pENET,
    IN PENET_PHY_CMD pCmd
    )
{
    UINT i;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETDoMIICmd\r\n")));
    
    if(!pCmd)
        return;
        
    for(i = 0; (pCmd + i)->MIIData != ENET_MII_END; i++ )
    {
        ENETQueueMII(pENET, (pCmd + i)->MIIData, (pCmd + i)->MIIFunct);
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETDoMIICmd\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETParseMIISr
//
// This function parses the status register data of the external MII compatible
// PHY(s).
//
// Parameters:
//        RegVal
//            [in] the status register value get from external MII compatible
//                 PHY(s)
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETParseMIISr(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETParseMIISr\r\n")));
    
    s = &(pENET->MIIPhyStatus);
    
    Status = *s & ~(PHY_STAT_LINK | PHY_STAT_FAULT | PHY_STAT_ANC);
    
    if(RegVal & 0x0004)
        Status |= PHY_STAT_LINK;
    if(RegVal & 0x0010)
        Status |= PHY_STAT_FAULT;
    if(RegVal & 0x0020)
        Status |= PHY_STAT_ANC;
        
    *s = Status;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETParseMIISr\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETParseMIICr
//
// This function parses the control register data of the external MII compatible
// PHY(s).
//
// Parameters:
//        RegVal
//            [in] the control register value get from external MII compatible
//                 PHY(s)
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETParseMIICr(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETParseMIICr\r\n")));
    
    s = &(pENET->MIIPhyStatus);
    
    Status = *s & ~(PHY_CONF_ANE | PHY_CONF_LOOP);
    
    if(RegVal & 0x1000)
        Status |= PHY_CONF_ANE;
    if(RegVal & 0x4000)
        Status |= PHY_CONF_LOOP;
        
    *s = Status;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETParseMIICr\r\n")));    
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETParseMIIAnar
//
// This function parses the Auto-Negotiation Advertisement Register data of the  
// external MII compatible PHY(s).
//
// Parameters:
//        RegVal
//            [in] the Auto-Negotiation Advertisement Register value get from 
//                 external MII compatible PHY(s)
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETParseMIIAnar(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETParseMIIAnar\r\n")));
    
    s = &(pENET->MIIPhyStatus);
    
    Status = *s & ~(PHY_CONF_SPMASK);
    
    if(RegVal & 0x0020)
        Status |= PHY_CONF_10HDX;
    if(RegVal & 0x0040)
        Status |= PHY_CONF_10FDX;
    if(RegVal & 0x0080)
        Status |= PHY_CONF_100HDX;
    if(RegVal & 0x0100)
        Status |= PHY_CONF_100FDX;
        
    *s = Status;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETParseMIIAnar\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETParseAm79c874Dr
//
// This function parses the diagnostic register data of the external Am79c874
// PHY.
//
// Parameters:
//        RegVal
//            [in] the Adiagnostic Register value get from external Am79c874 PHY
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETParseAm79c874Dr(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETParseAm79c874Dr\r\n")));
    
    s = &(pENET->MIIPhyStatus);
    
    Status = *s & ~(PHY_STAT_SPMASK | PHY_STAT_ANC);
    
    if(RegVal & 0x0080)
    Status |= PHY_STAT_ANC;
    if(RegVal & 0x0400)
    {
        Status |= ((RegVal & 0x0800) ? PHY_STAT_100FDX : PHY_STAT_100HDX);
        pEthernet->SpeedMode = TRUE;
    }
    else
    {
        Status |= ((RegVal & 0x0800) ? PHY_STAT_10FDX : PHY_STAT_10HDX);
        pEthernet->SpeedMode = FALSE;
    }    
    *s = Status;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETParseAm79c874Dr\r\n")));
}


//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETParseLAN87xxSCSR
//
// This function parses the diagnostic register data of the external Am79c874
// PHY.
//
// Parameters:
//        RegVal
//            [in] the Adiagnostic Register value get from external Am79c874 PHY
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETParseLAN87xxSCSR(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    UINT16 HcdSpeed;

    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETParseLAN87xxSCSR\r\n")));

    s = &(pENET->MIIPhyStatus);

    Status = *s & ~(PHY_STAT_SPMASK | PHY_STAT_ANC);
    
    HcdSpeed=(((UINT16)RegVal & 0x001C)>>2);
    if(1==HcdSpeed)
    {
        Status |=PHY_STAT_10HDX;
        pEthernet->SpeedMode = FALSE;    
    }
    else if(5==HcdSpeed)

    {
        Status |=PHY_STAT_10FDX;
        pEthernet->SpeedMode = FALSE;    
    }

    else if(2==HcdSpeed)
    {

        Status |=PHY_STAT_100HDX;
        pEthernet->SpeedMode = TRUE;    

    }

    else if(6==HcdSpeed)
    {

        Status |=PHY_STAT_100FDX;
        pEthernet->SpeedMode = TRUE;    

    }
    *s = Status;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETParseLAN87xxSCSR\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETParseDP83640PHYSTS
//
// This function parses the diagnostic register data of the external DP83640
// PHY.
//
// Parameters:
//        RegVal
//            [in] the Adiagnostic Register value get from external DP83640 PHY
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETParseDP83640PHYSTS(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    UINT16 HcdSpeed;

    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETParseDP83640PHYSTS\r\n")));

    s = &(pENET->MIIPhyStatus);

    Status = *s & ~(PHY_STAT_SPMASK | PHY_STAT_ANC);
    
    HcdSpeed=(((UINT16)RegVal & 0x0006)>>1);
    if(1 & HcdSpeed)
    {
        if(2 & HcdSpeed)
        {
            Status |=PHY_STAT_10FDX;
            pEthernet->SpeedMode = FALSE;
        }
        else
        {
            Status |=PHY_STAT_10HDX;
            pEthernet->SpeedMode = FALSE;
        }
    }
    else
    {
        if(2 & HcdSpeed)
        {
            Status |=PHY_STAT_100FDX;
            pEthernet->SpeedMode = TRUE;
        }
        else
        {
            Status |=PHY_STAT_100HDX;
            pEthernet->SpeedMode = TRUE;
        }    
    }

    *s = Status;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETParseDP83640PHYSTS\r\n")));
}



//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETParsePHYLink
//
// This function will update the link status according to the Status register
// of the external PHY.
// 
// Parameters:
//        RegVal
//            [in] the MII frame value which is read from external PHY registers
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETClass::ENETParsePHYLink(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    NDIS_MEDIA_STATE oldState;
    PNDIS_PACKET pNdisPacket;
    NDIS_STATUS Status;
    
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);

    UNREFERENCED_PARAMETER(RegVal);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETParsePHYLink\r\n")));
    
    oldState = pEthernet->MediaState;
    
    pENET->LinkStatus = (pENET->MIIPhyStatus & PHY_STAT_LINK)? TRUE : FALSE;
    
    if(pENET->LinkStatus)
    {
        pEthernet->MediaState = NdisMediaStateConnected;
        Status = NDIS_STATUS_MEDIA_CONNECT;
          DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +NDIS_STATUS_MEDIA_CONNECT $$$$$$$$$\r\n")));
    }
    else
    {
        pEthernet->MediaState = NdisMediaStateDisconnected;
        Status = NDIS_STATUS_MEDIA_DISCONNECT;
          DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +NDIS_STATUS_MEDIA_DISCONNECT $$$$$$$$$\r\n")));
    }
        
    if(oldState != pEthernet->MediaState)
    {
        if(oldState    == NdisMediaStateConnected && pEthernet->MediaState == NdisMediaStateDisconnected)
        {
            // Remove the packet from the queue
            EnterCriticalSection (&gENETBufCs);
            pNdisPacket = pEthernet->HeadPacket;
            
            while(pNdisPacket != NULL)
            {
                pEthernet->HeadPacket = RESERVED(pNdisPacket)->Next;
                if (pNdisPacket == pEthernet->TailPacket) 
                {
                    pEthernet->TailPacket = NULL;
                }

                NdisMSendComplete(pEthernet->ndisAdapterHandle, pNdisPacket, NDIS_STATUS_SUCCESS);
                pNdisPacket = pEthernet->HeadPacket;    
            }
            LeaveCriticalSection (&gENETBufCs);

        }
        
        if ( pEthernet->CurrentState != NdisHardwareStatusInitializing )
        {
            NdisMIndicateStatus( pEthernet->ndisAdapterHandle, Status, NULL, 0 );
            NdisMIndicateStatusComplete( pEthernet->ndisAdapterHandle );
        }    
    }
    
        
    // Finished the link status checking process
    pEthernet->MediaStateChecking = FALSE;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETParsePHYLink\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETGetPHYId
// 
// Scan all of the MII PHY addresses looking for someone to respond with a valid
// ID. This usually happens quickly.
//
// Parameters:
//        MIIReg
//            [in] the MII frame value which is read from external PHY registers
//
//      MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETGetPHYId(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT PhyType;
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    int i;
 
   pENET->MIIPhyAddr=index;
   DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETGetPHYId  pENET->MIIPhyAddr :%d \r\n"), pENET->MIIPhyAddr));
    
    for(i=0;i<10;i++)
        {
        if(pENET->MIIPhyAddr < PHY_MAX_COUNT)
        {
            Sleep(100);
            if((PhyType = (MIIReg & 0xffff)) != 0xffff && PhyType != 0)
            {
                // The first part of the ID have been got, then get the 
                // the remainder
                pENET->MIIPhyId = PhyType << 16;
                ENETQueueMII(pENET, MII_READ_COMMAND(MII_REG_PHYIR2), GetENETPHYId2);
                goto End;
            }
            else
            {
                // Try the next PHY address
            //    pENET->MIIPhyAddr++;
                ENETQueueMII(pENET, MII_READ_COMMAND(MII_REG_PHYIR1), GetENETPHYId);
            }
        }
       
        
    }
    {
            // Close the clock for MII
    pENET->MIIPhySpeed = 0;
    BW_ENET_MAC_MSCR_MII_SPEED(index,pENET->MIIPhySpeed);       
    DEBUGMSG(ZONE_INFO,
        (TEXT("ENETGetPHYId: No external PHY found\r\n")));
     }
End:  
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETGetPHYId\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETGetPHYId2
//
// Read the second part of the external PHY id.
//
// Parameters:
//        MIIReg
//            [in] the MII frame value which is read from external PHY registers
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in 
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETGetPHYId2(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT i;
    
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETGetPHYId2\r\n")));
    
    pENET->MIIPhyId |= (MIIReg & 0xffff);
    
    for(i = 0; PhyInfo[i] != NULL; i++)
    {
        if(PhyInfo[i]->PhyId == pENET->MIIPhyId)
            break;

        if(((PhyInfo[i]->PhyId)&0xffff0) == (pENET->MIIPhyId&0xffff0)) 
            break;
          
    }
    
    if(PhyInfo[i])
        DEBUGMSG(ZONE_INFO,
            (TEXT("%s: The name for the external PHY is %s\r\n"), __WFUNCTION__, PhyInfo[i]->PhyName));
    else
        DEBUGMSG(ZONE_INFO,
            (TEXT("%s: No supported PHY found\r\n"), __WFUNCTION__));
            
    pENET->MIIPhy = PhyInfo[i];
    pENET->MIIPhyIdDone = TRUE;
   
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETGetPHYId2\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETGetLinkStatus
//
// This function will send the command to the external PHY to get the link 
// status of the cable. The updated link status is stored in the context
// area designated by the parameter MiniportAdapterContext.
//
// Parameters:
//        MiniportAdapterHandle
//            [in] Specifies the handle to a ENET driver allocated context area
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETGetLinkStatus(
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETGetLinkStatus\r\n")));
    
    ENETDoMIICmd(pENET, pENET->MIIPhy->PhyActint);
    ENETDoMIICmd(pENET, PHYCmdLink);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETGetLinkStatus\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETDispPHYCfg
//
// This function displays the current status of the external PHY
//
// Parameters:
//        MIIReg
//            [in] the MII frame value which is read from external PHY registers
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETDispPHYCfg(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);

    UNREFERENCED_PARAMETER(MIIReg);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETDispPHYCfg\r\n")));
    
    Status = pENET->MIIPhyStatus;
    
    if(Status & PHY_CONF_ANE)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: Auto-negotiation is on\r\n"), __WFUNCTION__));
    else
        DEBUGMSG(ZONE_INFO, (TEXT("%s: Auto-negotiation is off\r\n"), __WFUNCTION__));
        
    if(Status & PHY_CONF_100FDX)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: 100M Full Duplex Mode\r\n"), __WFUNCTION__));
    if(Status & PHY_CONF_100HDX)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: 100M Half Duplex Mode\r\n"), __WFUNCTION__));
    if(Status & PHY_CONF_10FDX)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: 10M Full Duplex Mode\r\n"), __WFUNCTION__));
    if(Status & PHY_CONF_10HDX)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: 10M Half Duplex Mode\r\n"), __WFUNCTION__));
        
    if(!(Status & PHY_CONF_SPMASK))
        DEBUGMSG(ZONE_INFO, (TEXT("%s: No speed/duplex selected\r\n"), __WFUNCTION__));
        
    if(Status & PHY_CONF_LOOP)
        DEBUGMSG(ZONE_INFO, (TEXT("%s: Loop back mode is enabled\r\n"), __WFUNCTION__));
        
    pENET->MIISeqDone =  TRUE;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETDispPHYCfg\r\n")));
    
    return;
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETEnetInit
//
// This function initializes the ENET hardware and the external PHY(s). The ENET
// hardware initialization process includes:
//         1. allocate DMA memory for buffer descriptors(BDs)
//         2. allocate DMA memory for receiving buffers
//         3. allocate DMA memory for transmitting buffers
//         4. set the ENET hardware registers so that the ENET hardware is ready for
//           receiving and transmitting frames, and responsing to interrupts
//         5. to detect and initialize the external PHY(s)
//
// Parameters:
//        pEthernet
//            [in] the ENET driver context area allocated in function ENETInitialize
//
// Return Value:
//        returns TRUE if the initialization process is successful, otherwise 
//        returns FALSE.
//
//------------------------------------------------------------------------------
BOOL ENETClass::ENETEnetInit(IN NDIS_HANDLE MiniportAdapterHandle )
{
    PUCHAR MemAddr;
    PHYSICAL_ADDRESS    MemPhysicalBase;
    
    volatile PEBUFFER_DESC BufferDescPointer;
    PEBUFFER_DESC DescriptorBase;
    DMA_ADAPTER_OBJECT Adapter;
    UINT i;
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETEnetInit\r\n")));
    
    // enable gpio and clock for ENET hardware
    BSPENETIomuxConfig( index,TRUE );
    BSPENETClockConfig( index,TRUE );
    
    pENET->fUseRMII = BSPENETusesRMII();
     
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    // allocate DMA compatible memory for both receiving and transmitting
    // buffer descriptors(BDs) for enet driver ,we only support enhanced mode 
    
    pENET->RingBase = HalAllocateCommonBuffer(
                            &Adapter,
                            (ENET_RX_RING_SIZE + ENET_TX_RING_SIZE) * sizeof(EBUFFER_DESC),
                            &(pENET->RingPhysicalBase),
                            FALSE);
                            
    if(pENET->RingBase == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ENETEnetInit: Allocate DMA memory for BDs failed\r\n")));
        return FALSE;
    }
                            
    DescriptorBase = (PEBUFFER_DESC)pENET->RingBase;
    
    // Set receive and transmit descriptor base
    pENET->RxBufferDescBase = DescriptorBase;
    pENET->TxBufferDescBase = DescriptorBase + ENET_RX_RING_SIZE;
    
    pENET->DirtyTx = pENET->CurrentTx = pENET->TxBufferDescBase;
    pENET->CurrentRx = pENET->RxBufferDescBase;
    
    // allocate receive buffers and initialize the receive buffer descriptors
    BufferDescPointer = pENET->RxBufferDescBase;
    
    MemAddr = (PUCHAR)HalAllocateCommonBuffer(
                            &Adapter,
                            ENET_RX_RING_SIZE * ENET_ENET_RX_FRSIZE,
                            &MemPhysicalBase,
                            FALSE);                           
    if(MemAddr == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ENETEnetInit: Allocate DMA memory for receive buffers failed\r\n")));
        return FALSE;
    }
    else
    {
        pENET->RxBufferBase = (PVOID)MemAddr;
    }
                            
    for(i = 0; i < ENET_RX_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = BD_ENET_RX_EMPTY;
        BufferDescPointer->BDU = 0x0;
        BufferDescPointer->Control2Status = RX_BD_INT ; 
        BufferDescPointer->BufferAddr = (ULONG)MemPhysicalBase.QuadPart + i*ENET_ENET_RX_FRSIZE;
        
        pENET->RxBuffAddr[BufferDescPointer - pENET->RxBufferDescBase] = MemAddr + i*ENET_ENET_RX_FRSIZE;
        
        BufferDescPointer++;
    }
    
    // set the last buffer to wrap 
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;
    
    // allocate transmit buffers and initialize the transmit buffer descriptors
    BufferDescPointer = pENET->TxBufferDescBase;
    
    MemAddr = (PUCHAR)HalAllocateCommonBuffer(
                            &Adapter,
                            ENET_TX_RING_SIZE * ENET_ENET_TX_FRSIZE,
                            &MemPhysicalBase,
                            FALSE);
    if(MemAddr == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ENETEnetInit: Allocate DMA memory for transmit buffers failed\r\n")));
        return FALSE;
    }
    else
    {
        pENET->TxBufferBase = (PVOID)MemAddr;
    }                        
    
    for(i = 0; i < ENET_TX_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = 0;
        BufferDescPointer->BDU = 0x0;
        BufferDescPointer->Control2Status =  TX_BD_IINS | TX_BD_PINS;
        BufferDescPointer->BufferAddr = (ULONG)MemPhysicalBase.QuadPart + i*ENET_ENET_TX_FRSIZE;
        
        pENET->TxBuffAddr[BufferDescPointer - pENET->TxBufferDescBase] = MemAddr + i*ENET_ENET_TX_FRSIZE;
        
        BufferDescPointer++;
    }
    
    // set the last buffer to wrap
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;

    BW_ENET_MAC_ECR_RESET(index,1);

    while(HW_ENET_MAC_ECR_RD(index)&BM_ENET_MAC_ECR_RESET); 

    BW_ENET_MAC_ECR_ENA_1588(index,1);
    //Enable magic wake up
    BW_ENET_MAC_ECR_MAGIC_ENA(index,1);

    // set the receive and transmit BDs ring base to 
    // hardware registers(ERDSR & ETDSR)
  
    HW_ENET_MAC_ERDSR_WR(index,(long)pENET->RingPhysicalBase.QuadPart);
    HW_ENET_MAC_ETDSR_WR(index,(long)pENET->RingPhysicalBase.QuadPart + ENET_RX_RING_SIZE*sizeof(EBUFFER_DESC));
    
               
    // set other hardware registers
    HW_ENET_MAC_EIR_WR(index,0x0);
    

    HW_ENET_MAC_IAUR_WR(index,0x0);
    HW_ENET_MAC_IALR_WR(index,0x0);
    HW_ENET_MAC_GAUR_WR(index,0x0);
    HW_ENET_MAC_GALR_WR(index,0x0);

   
    HW_ENET_MAC_EMRBR_WR(index,PKT_MAXBLR_SIZE);
    
    Sleep(10);

    HW_ENET_MAC_EIMR_WR(index,BM_ENET_MAC_EIR_RXF|BM_ENET_MAC_EIR_TXF|BM_ENET_MAC_EIR_MII|BM_ENET_MAC_EIR_TS_AVAIL|BM_ENET_MAC_EIR_TS_TIMER);   

 
    BW_ENET_MAC_TCR_FEDN(index,1); 
    BW_ENET_MAC_RCR_LOOP(index,0);
    
    // Set the station address for the ENET Adapter
    HW_ENET_MAC_PALR_WR(index,pEthernet->EnetMacAddress[3] |
                               pEthernet->EnetMacAddress[2]<<8 |
                               pEthernet->EnetMacAddress[1]<<16 |
                               pEthernet->EnetMacAddress[0]<<24);
                               

    HW_ENET_MAC_PAUR_WR(index,pEthernet->EnetMacAddress[5]<<16 |
                               pEthernet->EnetMacAddress[4]<<24);
    for(i = 0; i < ENET_MII_COUNT-1; i++)
    {
        gMIICmds[i].MIINext = &gMIICmds[i+1];
    }
    
    gMIIFree = gMIICmds;

    gMIIHead=NULL;
    gMIITail=NULL;
    
    // setup MII interface
    ENETSetMII(pENET);
     
    BW_ENET_MAC_RCR_FCE(index,1);

    BW_ENET_MAC_RCR_PROM(index,1);
    BW_ENET_MAC_RCR_BC_REJ(index,1);

    HW_ENET_MAC_TCR_WR(index,0x1c);
   
    BW_ENET_MAC_ECR_ETHER_EN(index,1);

    BW_ENET_MAC_RDAR_RDAR(index, 1);

    BW_ENET_MAC_RCR_NO_LGTH_CHECK(index, 1);
    
    // Queue up command to detect the PHY and initialize the remainder of 
    // the interface
    pENET->TxFull = FALSE;
    pENET->MIIPhyIdDone = FALSE;
    pENET->MIIPhyAddr = index;
    pENET->MIISeqDone = FALSE;
    pENET->LinkStatus = FALSE;
    
    pENET->MIIPhy = NULL;
 
    
    ENETQueueMII( pENET, MII_READ_COMMAND(MII_REG_PHYIR1), GetENETPHYId );
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETEnetInit\r\n")));
    
    InitTimer();
    
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETEnetDeinit
//
// This function Deinitialize the ENET hardware. It will free the DMA buffers and
// disable the GPIO and CLK for ENET.
//
// Parameters:
//        pEthernet
//            [in] the ENET driver context area allocated in function ENETInitialize
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETClass::ENETEnetDeinit( IN NDIS_HANDLE MiniportAdapterHandle)
{
    PHYSICAL_ADDRESS    MemPhysicalBase;
    DMA_ADAPTER_OBJECT Adapter;
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETEnetDeinit\r\n")));
    
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
    
    MemPhysicalBase.QuadPart = 0;
    
    // Free DMA buffers allocated(Note: only virtual address is needed)
    HalFreeCommonBuffer(&Adapter, (ENET_RX_RING_SIZE + ENET_TX_RING_SIZE) * sizeof(BUFFER_DESC), MemPhysicalBase, pENET->RingBase, FALSE);
    HalFreeCommonBuffer(&Adapter, ENET_RX_RING_SIZE * ENET_ENET_RX_FRSIZE, MemPhysicalBase, pENET->RxBufferBase, FALSE);
    HalFreeCommonBuffer(&Adapter, ENET_TX_RING_SIZE * ENET_ENET_TX_FRSIZE, MemPhysicalBase, pENET->TxBufferBase, FALSE);
    
    // disable GPIO and CLK for ENET
    BSPENETIomuxConfig( index,FALSE );
    BSPENETClockConfig( index,FALSE );
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETEnetDeinit\r\n")));    
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETEnetAutoNego
//
// This function issues the auto-negotiation sequences to the external PHY device
//
// Parameters:
//        pEthernet
//            [in] the ENET driver context area allocated in function ENETInitialize
//
// Return value
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETEnetAutoNego( IN NDIS_HANDLE MiniportAdapterHandle)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETEnetAutoNego\r\n")));
    
    // Wait for MII interrupt to set to TURE
    while(pENET->MIIPhyIdDone != TRUE)  
         Sleep(10);
        
    if(pENET->MIIPhy)
    {
        // Set to auto-negotiation mode and restart the
        // auto-negotiation process
        ENETDoMIICmd(pENET, pENET->MIIPhy->PhyActint);
        ENETDoMIICmd(pENET, pENET->MIIPhy->PhyConfig);
        ENETDoMIICmd(pENET, PHYCmdCfg);      
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETEnetAutoNego\r\n")));
}


//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETEnetPowerHandle
//
// This function is called to handle ENET phy suspend and resume
//
// Parameters:
//        pEthernet
//            [in] the ENET driver context area allocated in function ENETInitialize

//
//         En
//            [in] TRUE for ENET phy power resume  , FALSE for ENET power suspend
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETEnetPowerHandle( IN NDIS_HANDLE MiniportAdapterHandle,BOOL En)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETEnetPowerHandle\r\n")));    
    if(pENET->MIIPhy)
    {        
        if(En)
        {
        
            BSPENETPowerEnable( index,TRUE);           
            ENETDoMIICmd(pENET, PHYCmdResume);
           
        }
        else  
        {
            ENETDoMIICmd(pENET, PHYCmdSuspend);       
            BSPENETPowerEnable( index,FALSE);

        }
    }  
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETEnetPowerHandle\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETEnetReset
//
// This function is called to restart the ENET hardware. Linking status change
// or switching between half and full duplex mode will cause this function to
// be called.
//
// Parameters:
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in 
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize
//
//        DuplexMode
//            [in] TRUE for full duplex mode, FALSE for half duplex mode
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETEnetReset(
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN BOOL DuplexMode
    )
{
    volatile PEBUFFER_DESC BufferDescPointer;
    UINT i;
    
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET%x: +ENETEnetReset\r\n"),index));

    BW_ENET_MAC_ECR_RESET(index,1);

    while(HW_ENET_MAC_ECR_RD(index)&BM_ENET_MAC_ECR_RESET); 

    BW_ENET_MAC_ECR_ENA_1588(index,1);
    //Enable magic wake up
    BW_ENET_MAC_ECR_MAGIC_ENA(index,1);

    // set the receive and transmit BDs ring base to 
    // hardware registers(ERDSR & ETDSR)
  
    HW_ENET_MAC_ERDSR_WR(index,(long)pENET->RingPhysicalBase.QuadPart);
    HW_ENET_MAC_ETDSR_WR(index,(long)pENET->RingPhysicalBase.QuadPart + ENET_RX_RING_SIZE*sizeof(EBUFFER_DESC));

            
    pENET->DirtyTx = pENET->CurrentTx = pENET->TxBufferDescBase;
    pENET->CurrentRx = pENET->RxBufferDescBase;
    
    // Initialize the receive buffer descriptors
    BufferDescPointer = pENET->RxBufferDescBase;
    
    for(i = 0; i < ENET_RX_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = BD_ENET_RX_EMPTY;
        BufferDescPointer->BDU = 0x0;
        BufferDescPointer->Control2Status = RX_BD_INT ; 
        BufferDescPointer++;
    }
    
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;
    
    
    // Initialize the transmit buffer descriptor
    BufferDescPointer = pENET->TxBufferDescBase;
    
    for(i = 0; i < ENET_TX_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = 0;
        BufferDescPointer->Control2Status =  TX_BD_IINS | TX_BD_PINS;
     
        BufferDescPointer++;
    }
    
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;
               
    // set other hardware registers

    // setup MII interface
    ENETSetMII(pENET);
  
    HW_ENET_MAC_EIR_WR(index,0x0);
    

    HW_ENET_MAC_IAUR_WR(index,0x0);
    HW_ENET_MAC_IALR_WR(index,0x0);
    HW_ENET_MAC_GAUR_WR(index,0x0);
    HW_ENET_MAC_GALR_WR(index,0x0);

   
    HW_ENET_MAC_EMRBR_WR(index,PKT_MAXBLR_SIZE);
    
    Sleep(10);

    HW_ENET_MAC_EIMR_WR(index,BM_ENET_MAC_EIR_RXF|BM_ENET_MAC_EIR_TXF|BM_ENET_MAC_EIR_MII|BM_ENET_MAC_EIR_TS_AVAIL|BM_ENET_MAC_EIR_TS_TIMER);   

    BW_ENET_MAC_RCR_LOOP(index,0);

    if(DuplexMode)
    {
        // MII mode enable and FD(Full Duplex) enable   
        BW_ENET_MAC_RCR_MII_MODE(index,1);
        BW_ENET_MAC_RCR_MAX_FL(index,PKT_MAXBUF_SIZE);
        HW_ENET_MAC_EMRBR_WR(index,PKT_MAXBLR_SIZE);
        BW_ENET_MAC_TCR_FEDN(index,1);
                     
    }
    else
    {
        // MII mode enable and FD disable                 
        BW_ENET_MAC_RCR_MII_MODE(index,1);
        BW_ENET_MAC_RCR_MAX_FL(index,PKT_MAXBUF_SIZE);
        HW_ENET_MAC_EMRBR_WR(index,PKT_MAXBLR_SIZE);
        BW_ENET_MAC_RCR_DRT(index,1);
        HW_ENET_MAC_TCR_WR(index,0);
    }

    pENET->FullDuplex = DuplexMode;
    
    
    // Set the station address for the ENET Adapter
    HW_ENET_MAC_PALR_WR(index,pEthernet->EnetMacAddress[3] |
                               pEthernet->EnetMacAddress[2]<<8 |
                               pEthernet->EnetMacAddress[1]<<16 |
                               pEthernet->EnetMacAddress[0]<<24);
                               

    HW_ENET_MAC_PAUR_WR(index,pEthernet->EnetMacAddress[5]<<16 |
                               pEthernet->EnetMacAddress[4]<<24);

       
    BW_ENET_MAC_RCR_FCE(index,1);

    BW_ENET_MAC_TCR_RFC_PAUSE(index,1); 

    BW_ENET_MAC_TCR_TFC_PAUSE(index,1); 

    BW_ENET_MAC_TCR_FEDN(index,1); 

    BW_ENET_MAC_ECR_ETHER_EN(index,1);

    BW_ENET_MAC_RDAR_RDAR(index, 1);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETEnetReset\r\n")));
    
    return;     
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::CalculateHashValue
//
// This function calculates the 6-bit Hash value for multicasting.
//
// Parameters:
//      pAddr
//        [in] pointer to a ethernet address
//
// Returns:
//      Returns the calculated 6-bit Hash value.
//
//------------------------------------------------------------------------------
UCHAR ENETClass::CalculateHashValue(UCHAR *pAddr)
{
    ULONG CRC;
    UCHAR HashValue = 0;
    UCHAR AddrByte;
    int byte, bit;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +CalculateHashValue\r\n")));
    
    // calculate CRC32 value of MAC address
    CRC = CRC_PRIME;
    
    for(byte=0; byte < ETHER_ADDR_SIZE; byte++)
    {
        AddrByte = *pAddr++;
        
        for(bit = 0; bit < 8; bit++, AddrByte >>= 1)
        {
            CRC = (CRC >> 1) ^
                    (((CRC ^ AddrByte) & 1) ? CRC_POLYNOMIAL : 0);
        }
    }
    
    // only upper 6 bits (HASH_BITS) are used which point to specific 
    // bit in the hash registers
    
    HashValue = (UCHAR)((CRC >> (32 - HASH_BITS)) & 0x3f);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -CalculateHashValue [0x%.8X]\r\n"), HashValue));
    
    return HashValue;

}


//------------------------------------------------------------------------------
//
// Function: ENETClass::AddMultiCast
//
// This function adds the Hash value to the Hash table(GAUR and GALR).
//
// Parameters:
//      pAddr
//          [in] pointer to a ethernet address.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void ENETClass::AddMultiCast( UCHAR *pAddr )
{
    UCHAR HashValue = 0;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +AddMultiCast\r\n")));
    
    HashValue = CalculateHashValue( pAddr );
    
    if( HashValue > 31 )
        HW_ENET_MAC_GAUR_WR(index,HW_ENET_MAC_GAUR_RD(index)|(1 << (HashValue-32)));
    else
        HW_ENET_MAC_GALR_WR(index,HW_ENET_MAC_GALR_RD(index)|(1 << (HashValue)));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -AddMultiCast\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ClearAllMultiCast
//
// This function clears the Hash table(GAUR and GALR).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void ENETClass::ClearAllMultiCast()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ClearAllMultiCast\r\n")));
   
    HW_ENET_MAC_GAUR_WR(index,0x0);
    HW_ENET_MAC_GALR_WR(index,0x0);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ClearAllMultiCast\r\n")));
}
