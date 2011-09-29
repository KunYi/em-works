//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  chip.c
//
//  Implementation of FEC Driver
//
//  This file implements hardware related functions for FEC.
//
//-----------------------------------------------------------------------------

#include "fec.h"
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
extern PCSP_FEC_REGS    gpFECReg;
extern CRITICAL_SECTION gFECBufCs;

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
FEC_MII_LIST   gMIICmds[FEC_MII_COUNT];
PFEC_MII_LIST  gMIIFree = NULL;
PFEC_MII_LIST  gMIIHead = NULL;
PFEC_MII_LIST  gMIITail = NULL;

//------------------------------------------------------------------------------
// File-local(static) Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------
extern BOOL BSPFECIomuxConfig( IN BOOL Enable );
extern BOOL BSPFECClockConfig( IN BOOL Enable );
extern BOOL BSPFECusesRMII(void);



//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

void  FECSetMII(IN PFEC_ENET_PRIVATE pFEC);

BOOL FECQueueMII(
    IN PFEC_ENET_PRIVATE pFEC,
    IN UINT RegValue,
    IN void (*OpFunc)(UINT, NDIS_HANDLE)
    );
    
void FECDoMIICmd(
    IN PFEC_ENET_PRIVATE pFEC,
    IN PFEC_PHY_CMD pCmd
    );
    
void FECGetPHYId2(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    );

//------------------------------------------------------------------------------
// Functions implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Function: FECStartXmit
//
// This function gets the transmit data and sets the transmit buffer descriptors
// for transmitting process.
//
// Parameters:
//        pEthernet
//            [in]  Specifies the driver allocated context area in which the driver
//                  maintains FEC adapter state, set up by FECInitialize.
// 
// Return value:
//        TRUE for success, FALSE for failure.
//
//------------------------------------------------------------------------------
BOOL FECStartXmit(pFEC_t pEthernet)
{
    volatile PBUFFER_DESC BufferDescPointer;
    UINT index;
    PUCHAR MemAddr;

    // Packet size of the packet
    UINT PacketSize;
    UINT CurrenSize = 0;
    PNDIS_BUFFER pNDISBuffer;
    
    // Holds virtual address of the current buffer
    PUCHAR CurrentBufAddress;
    
    // Holds the length of the current buffer of the packet
    DWORD BufferLength;
    
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECStartXmit\r\n")));
    
    //if(pFEC->LinkStatus == FALSE)
    //{
        // Link is down or autonegotiation is in progress
    //    return FALSE;
    //}
    
    BufferDescPointer = pFEC->CurrentTx;
    
    if(BufferDescPointer->ControlStatus & BD_ENET_TX_READY)
    {
        // Transmit buffers are full
        return FALSE;
    }
    
    // Clear all of the status flags
    BufferDescPointer->ControlStatus &= ~BD_ENET_TX_STATS; 
    
    index = BufferDescPointer - pFEC->TxBufferDescBase;
    MemAddr = pFEC->TxBuffAddr[index];
    
    // Check whether the first packet is NULL
    if(pEthernet->HeadPacket == NULL)
    {
        return FALSE;
    }
    
    // Get the length of the packet and the pointer to NDIS Buffer
    NdisQueryPacket(pEthernet->HeadPacket, NULL, NULL, &pNDISBuffer, &PacketSize);
    
    NdisQueryBuffer(pNDISBuffer, (PVOID *)&CurrentBufAddress, &BufferLength);
    
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
            NdisQueryBuffer(pNDISBuffer, (PVOID *)&CurrentBufAddress, &BufferLength);
        }
    
    }
    
    // set up the transmit buffer descriptor
    BufferDescPointer->ControlStatus |= (BD_ENET_TX_READY | BD_ENET_TX_INTR |
                                         BD_ENET_TX_LAST  | BD_ENET_TX_TC);
                                         
    BufferDescPointer->DataLen = (USHORT)PacketSize;
    
    // Trigger transmission start
    INSREG32BF(&gpFECReg->TDAR, FEC_TDAR_ACTIVE, FEC_TDAR_ACTIVE_ACTIVE);
    
    // If this was the last BD in the ring, start at the begining again
    if(BufferDescPointer->ControlStatus & BD_ENET_TX_WRAP)
    {
        BufferDescPointer = pFEC->TxBufferDescBase;
    }
    else
    {
        BufferDescPointer++;
    }
    
    if(BufferDescPointer == pFEC->DirtyTx)
    {
        pFEC->TxFull = TRUE;
    }
    
    pFEC->CurrentTx = (PBUFFER_DESC)BufferDescPointer;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECStartXmit\r\n")));
    
    return TRUE;
}



//------------------------------------------------------------------------------
// MII management related functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Function: FECSetMII
//
// This function sets the clock for the MII interface
//
// Parameters:
//        pFEC
//            [in] Pointer to a structure which contains the status and control
//                 information for the FEC controller and MII
// Return Value:
//        None.
//
//------------------------------------------------------------------------------ 
void  FECSetMII(  IN PFEC_ENET_PRIVATE pFEC  )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECSetMII\r\n")));
    
    OUTREG32(&gpFECReg->RCR,
                    CSP_BITFVAL(FEC_RCR_MAXFL, PKT_MAXBUF_SIZE)|
                    CSP_BITFVAL(FEC_RCR_MIIMODE, FEC_RCR_MIIMODE_ENABLE));
                    
    OUTREG32(&gpFECReg->TCR, 0);
    
    if (pFEC->fUseRMII == FALSE)
    {
        // Set MII speed to 2.5MHz
        pFEC->MIIPhySpeed = 14;
    }
    else
    {
        // Set MII speed
        pFEC->MIIPhySpeed = 4;
    }
    
    OUTREG32(&gpFECReg->MSCR,
                     CSP_BITFVAL(FEC_MSCR_MIISPEED, pFEC->MIIPhySpeed));
                     
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECSetMII\r\n")));
}


//------------------------------------------------------------------------------
//
// Function: FECQueueMII
//
// This function adds a MII management command to the MII command list
//
// Parameters:
//        pFEC
//            [in] Pointer to a structure which contains the status and control
//                 information for the FEC controller and MII
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
BOOL FECQueueMII(
    IN PFEC_ENET_PRIVATE pFEC,
    IN UINT RegValue,
    IN void (*OpFunc)(UINT, NDIS_HANDLE)
    )
{
    BOOL RetVal = TRUE;
    PFEC_MII_LIST MIIPoint;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECQueueMII\r\n")));
    
    // add PHY address to the MII command
    RegValue |= CSP_BITFVAL(FEC_MMFR_PA, pFEC->MIIPhyAddr);
    
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
            OUTREG32(&gpFECReg->MMFR, RegValue);
        }
    }
    else
    {
        RetVal = FALSE;
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECQueueMII\r\n")));
    
    return RetVal;
}

//------------------------------------------------------------------------------
//
// Function: FECDoMIICmd
//
// This function will call FECQueueMII to queue all the requested MII management
// commands to the sending list.
//
// Parameters:
//        pFEC
//            [in] Points to a structure which contains the status and control
//                 information for the FEC controller and MII
//
//        pCmd
//            [in] Points to a FEC_PHY_CMD array which specifies the MII management
//                 commands and the parsing functions
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECDoMIICmd(
    IN PFEC_ENET_PRIVATE pFEC,
    IN PFEC_PHY_CMD pCmd
    )
{
    UINT i;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECDoMIICmd\r\n")));
    
    if(!pCmd)
        return;
        
    for(i = 0; (pCmd + i)->MIIData != FEC_MII_END; i++ )
    {
        FECQueueMII(pFEC, (pCmd + i)->MIIData, (pCmd + i)->MIIFunct);
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECDoMIICmd\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECParseMIISr
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
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECParseMIISr(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParseMIISr\r\n")));
    
    s = &(pFEC->MIIPhyStatus);
    
    Status = *s & ~(PHY_STAT_LINK | PHY_STAT_FAULT | PHY_STAT_ANC);
    
    if(RegVal & 0x0004)
        Status |= PHY_STAT_LINK;
    if(RegVal & 0x0010)
        Status |= PHY_STAT_FAULT;
    if(RegVal & 0x0020)
        Status |= PHY_STAT_ANC;
        
    *s = Status;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParseMIISr\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECParseMIICr
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
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECParseMIICr(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParseMIICr\r\n")));
    
    s = &(pFEC->MIIPhyStatus);
    
    Status = *s & ~(PHY_CONF_ANE | PHY_CONF_LOOP);
    
    if(RegVal & 0x1000)
        Status |= PHY_CONF_ANE;
    if(RegVal & 0x4000)
        Status |= PHY_CONF_LOOP;
        
    *s = Status;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParseMIICr\r\n")));    
}

//------------------------------------------------------------------------------
//
// Function: FECParseMIIAnar
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
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECParseMIIAnar(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParseMIIAnar\r\n")));
    
    s = &(pFEC->MIIPhyStatus);
    
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
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParseMIIAnar\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECParseAm79c874Dr
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
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECParseAm79c874Dr(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParseAm79c874Dr\r\n")));
    
    s = &(pFEC->MIIPhyStatus);
    
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
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParseAm79c874Dr\r\n")));
}


//------------------------------------------------------------------------------
//
// Function: FECParseLAN8700SCSR
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
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECParseLAN8700SCSR(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    UINT16 HcdSpeed;

    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParseLAN8700SCSR\r\n")));

    s = &(pFEC->MIIPhyStatus);

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

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParseLAN8700SCSR\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECParseDP83640PHYSTS
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
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECParseDP83640PHYSTS(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    UINT16 HcdSpeed;

    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParseDP83640PHYSTS\r\n")));

    s = &(pFEC->MIIPhyStatus);

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

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParseDP83640PHYSTS\r\n")));
}


//------------------------------------------------------------------------------
//
// CS&ZHL JUN-2-2011
//
// Function: FECParseDM9161State
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
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECParseDM9161State(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    volatile UINT *s;
    UINT16 HcdSpeed;

    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);

    //DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParseDM9161State\r\n")));
    RETAILMSG(1, (TEXT("FEC: +FECParseDM9161State\r\n")));

    s = &(pFEC->MIIPhyStatus);

    Status = *s & ~(PHY_STAT_SPMASK | PHY_STAT_ANC);
    
    HcdSpeed = (UINT16)RegVal;
	if(HcdSpeed & MII_DM9161_SR_10FDX)
	{
        Status |=PHY_STAT_10FDX;
        pEthernet->SpeedMode = FALSE;
		RETAILMSG(1, (TEXT("FEC: 10Mbps Full Duplex\r\n")));
	}
	
	if(HcdSpeed & MII_DM9161_SR_10HDX)
	{
        Status |=PHY_STAT_10HDX;
        pEthernet->SpeedMode = FALSE;
		RETAILMSG(1, (TEXT("FEC: 10Mbps Half Duplex\r\n")));
	}
	
	if(HcdSpeed & MII_DM9161_SR_100FDX)
	{
        Status |=PHY_STAT_100FDX;
        pEthernet->SpeedMode = TRUE;
		RETAILMSG(1, (TEXT("FEC: 100Mbps Full Duplex\r\n")));
	}
	
	if(HcdSpeed & MII_DM9161_SR_100HDX)
	{
        Status |=PHY_STAT_100HDX;
        pEthernet->SpeedMode = TRUE;
		RETAILMSG(1, (TEXT("FEC: 100Mbps Half Duplex\r\n")));
	}

    *s = Status;

    //DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParseDM9161State\r\n")));
}


//------------------------------------------------------------------------------
//
// Function: FECParsePHYLink
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
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void FECParsePHYLink(
    IN UINT RegVal,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    NDIS_MEDIA_STATE oldState;
    PNDIS_PACKET pNdisPacket;
    NDIS_STATUS Status;
    
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);

    UNREFERENCED_PARAMETER(RegVal);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECParsePHYLink\r\n")));
    
    oldState = pEthernet->MediaState;
    
    pFEC->LinkStatus = (pFEC->MIIPhyStatus & PHY_STAT_LINK)? TRUE : FALSE;
    
    if(pFEC->LinkStatus)
    {
        pEthernet->MediaState = NdisMediaStateConnected;
        Status = NDIS_STATUS_MEDIA_CONNECT;
    }
    else
    {
        pEthernet->MediaState = NdisMediaStateDisconnected;
        Status = NDIS_STATUS_MEDIA_DISCONNECT;
    }
        
    if(oldState != pEthernet->MediaState)
    {
        if(oldState    == NdisMediaStateConnected && pEthernet->MediaState == NdisMediaStateDisconnected)
        {
            // Remove the packet from the queue
            EnterCriticalSection (&gFECBufCs);
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
            LeaveCriticalSection (&gFECBufCs);

        }
        
        if ( pEthernet->CurrentState != NdisHardwareStatusInitializing )
        {
            NdisMIndicateStatus( pEthernet->ndisAdapterHandle, Status, NULL, 0 );
            NdisMIndicateStatusComplete( pEthernet->ndisAdapterHandle );
        }    
    }
    
        
    // Finished the link status checking process
    pEthernet->MediaStateChecking = FALSE;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECParsePHYLink\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECGetPHYId
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
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECGetPHYId(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT PhyType;
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECGetPHYId\r\n")));
    
    
    if(pFEC->MIIPhyAddr < PHY_MAX_COUNT)
    {
        Sleep(50);
        if((PhyType = (MIIReg & 0xffff)) != 0xffff && PhyType != 0)
        {
            // The first part of the ID have been got, then get the 
            // the remainder
            pFEC->MIIPhyId = PhyType << 16;
            FECQueueMII(pFEC, MII_READ_COMMAND(MII_REG_PHYIR2), FECGetPHYId2);
        }
        else
        {
            // Try the next PHY address
            pFEC->MIIPhyAddr++;
            FECQueueMII(pFEC, MII_READ_COMMAND(MII_REG_PHYIR1), FECGetPHYId);
        }
    }
    else
    {
        // Close the clock for MII
        pFEC->MIIPhySpeed = 0;
        INSREG32BF( &gpFECReg->MSCR, FEC_MSCR_MIISPEED, pFEC->MIIPhySpeed );
               
        DEBUGMSG(ZONE_INFO,
            (TEXT("%s: No external PHY found\r\n"), __WFUNCTION__));
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECGetPHYId\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECGetPHYId2
//
// Read the second part of the external PHY id.
//
// Parameters:
//        MIIReg
//            [in] the MII frame value which is read from external PHY registers
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in 
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECGetPHYId2(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT i;
    
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECGetPHYId2\r\n")));
    
    pFEC->MIIPhyId |= (MIIReg & 0xffff);
    
    for(i = 0; PhyInfo[i] != NULL; i++)
    {
        if(PhyInfo[i]->PhyId == pFEC->MIIPhyId)
            break;

        if(((PhyInfo[i]->PhyId)&0xffff0) == (pFEC->MIIPhyId&0xffff0)) 
            break;
          
    }
    
    if(PhyInfo[i])
        DEBUGMSG(ZONE_INFO,
            (TEXT("%s: The name for the external PHY is %s\r\n"), __WFUNCTION__, PhyInfo[i]->PhyName));
    else
        DEBUGMSG(ZONE_INFO,
            (TEXT("%s: No supported PHY found\r\n"), __WFUNCTION__));
            
    pFEC->MIIPhy = PhyInfo[i];
    pFEC->MIIPhyIdDone = TRUE;


    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECGetPHYId2\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECGetLinkStatus
//
// This function will send the command to the external PHY to get the link 
// status of the cable. The updated link status is stored in the context
// area designated by the parameter MiniportAdapterContext.
//
// Parameters:
//        MiniportAdapterHandle
//            [in] Specifies the handle to a FEC driver allocated context area
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECGetLinkStatus(
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECGetLinkStatus\r\n")));
    
    FECDoMIICmd(pFEC, pFEC->MIIPhy->PhyActint);
    FECDoMIICmd(pFEC, PHYCmdLink);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECGetLinkStatus\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECDispPHYCfg
//
// This function displays the current status of the external PHY
//
// Parameters:
//        MIIReg
//            [in] the MII frame value which is read from external PHY registers
//
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECDispPHYCfg(
    IN UINT MIIReg,
    IN NDIS_HANDLE MiniportAdapterHandle
    )
{
    UINT Status;
    
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);

    UNREFERENCED_PARAMETER(MIIReg);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECDispPHYCfg\r\n")));
    
    Status = pFEC->MIIPhyStatus;
    
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
        
    pFEC->MIISeqDone =  TRUE;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECDispPHYCfg\r\n")));
    
    return;
}

//------------------------------------------------------------------------------
//
// Function: FECEnetInit
//
// This function initializes the FEC hardware and the external PHY(s). The FEC
// hardware initialization process includes:
//         1. allocate DMA memory for buffer descriptors(BDs)
//        2. allocate DMA memory for receiving buffers
//        3. allocate DMA memory for transmitting buffers
//        4. set the FEC hardware registers so that the FEC hardware is ready for
//           receiving and transmitting frames, and responsing to interrupts
//        5. to detect and initialize the external PHY(s)
//
// Parameters:
//        pEthernet
//            [in] the FEC driver context area allocated in function FECInitialize
//
// Return Value:
//        returns TRUE if the initialization process is successful, otherwise 
//        returns FALSE.
//
//------------------------------------------------------------------------------
BOOL FECEnetInit(pFEC_t pEthernet)
{
    PUCHAR MemAddr;
    PHYSICAL_ADDRESS    MemPhysicalBase;
    
    volatile PBUFFER_DESC BufferDescPointer;
    PBUFFER_DESC DescriptorBase;
    DMA_ADAPTER_OBJECT Adapter;
    UINT i;
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);

    //DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECEnetInit\r\n")));
    RETAILMSG(1, (TEXT("FEC: +FECEnetInit\r\n")));
    
    // enable gpio and clock for FEC hardware
    BSPFECIomuxConfig( TRUE );
    BSPFECClockConfig( TRUE );
    
    pFEC->fUseRMII = BSPFECusesRMII();
    
    
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    // allocate DMA compatible memory for both receiving and transmitting
    // buffer descriptors(BDs)
    
    pFEC->RingBase = HalAllocateCommonBuffer(
                            &Adapter,
                            (FEC_RX_RING_SIZE + FEC_TX_RING_SIZE) * sizeof(BUFFER_DESC),
                            &(pFEC->RingPhysicalBase),
                            FALSE);
                            
    if(pFEC->RingBase == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("FECEnetInit: Allocate DMA memory for BDs failed\r\n")));
        return FALSE;
    }
                            
    DescriptorBase = (PBUFFER_DESC)pFEC->RingBase;
    
    // Set receive and transmit descriptor base
    pFEC->RxBufferDescBase = DescriptorBase;
    pFEC->TxBufferDescBase = DescriptorBase + FEC_RX_RING_SIZE;
    
    pFEC->DirtyTx = pFEC->CurrentTx = pFEC->TxBufferDescBase;
    pFEC->CurrentRx = pFEC->RxBufferDescBase;
    
    // allocate receive buffers and initialize the receive buffer descriptors
    BufferDescPointer = pFEC->RxBufferDescBase;
    
    MemAddr = (PUCHAR)HalAllocateCommonBuffer(
                            &Adapter,
                            FEC_RX_RING_SIZE * FEC_ENET_RX_FRSIZE,
                            &MemPhysicalBase,
                            FALSE);
                            
    if(MemAddr == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("FECEnetInit: Allocate DMA memory for receive buffers failed\r\n")));
        return FALSE;
    }
    else
    {
        pFEC->RxBufferBase = (PVOID)MemAddr;
    }
                            
    for(i = 0; i < FEC_RX_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = BD_ENET_RX_EMPTY;
        BufferDescPointer->BufferAddr = (ULONG)MemPhysicalBase.QuadPart + i*FEC_ENET_RX_FRSIZE;
        
        pFEC->RxBuffAddr[BufferDescPointer - pFEC->RxBufferDescBase] = MemAddr + i*FEC_ENET_RX_FRSIZE;
        
        BufferDescPointer++;
    }
    
    // set the last buffer to wrap 
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;
    
    // allocate transmit buffers and initialize the transmit buffer descriptors
    BufferDescPointer = pFEC->TxBufferDescBase;
    
    MemAddr = (PUCHAR)HalAllocateCommonBuffer(
                            &Adapter,
                            FEC_TX_RING_SIZE * FEC_ENET_TX_FRSIZE,
                            &MemPhysicalBase,
                            FALSE);
    if(MemAddr == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("FECEnetInit: Allocate DMA memory for transmit buffers failed\r\n")));
        return FALSE;
    }
    else
    {
        pFEC->TxBufferBase = (PVOID)MemAddr;
    }                        
    
    for(i = 0; i < FEC_TX_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = 0;
        BufferDescPointer->BufferAddr = (ULONG)MemPhysicalBase.QuadPart + i*FEC_ENET_TX_FRSIZE;
        
        pFEC->TxBuffAddr[BufferDescPointer - pFEC->TxBufferDescBase] = MemAddr + i*FEC_ENET_TX_FRSIZE;
        
        BufferDescPointer++;
    }
    
    // set the last buffer to wrap
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;
    
    // issue a reset to the FEC hardware
    INSREG32BF(&gpFECReg->ECR, FEC_ECR_RESET, FEC_ECR_RESET_RESET);
    
    // wait for the hareware reset to complete
    while(EXTREG32BF(&gpFECReg->ECR, FEC_ECR_RESET));
    
    // set the receive and transmit BDs ring base to 
    // hardware registers(ERDSR & ETDSR)
    OUTREG32(&gpFECReg->ERDSR, (ULONG)pFEC->RingPhysicalBase.QuadPart);
    OUTREG32(&gpFECReg->ETDSR, 
            (ULONG)pFEC->RingPhysicalBase.QuadPart + FEC_RX_RING_SIZE*sizeof(BUFFER_DESC));
            
    // set other hardware registers
    OUTREG32(&gpFECReg->EIR, FEC_EIR_CLEARALL_MASK);

#if 0
    OUTREG32(&gpFECReg->EIMR,
                    CSP_BITFVAL(FEC_EIMR_TXF, FEC_EIMR_TXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_RXF, FEC_EIMR_RXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_UNMASK));
#endif
                
    OUTREG32(&gpFECReg->IAUR, 0);
    OUTREG32(&gpFECReg->IALR, 0);
    OUTREG32(&gpFECReg->GAUR, 0);
    OUTREG32(&gpFECReg->GALR, 0);
    
    OUTREG32(&gpFECReg->EMRBR, PKT_MAXBLR_SIZE);
    
    // enable the FEC
    INSREG32BF(&gpFECReg->ECR, FEC_ECR_ETHEREN, FEC_ECR_ETHEREN_ENABLE);
    
    INSREG32BF(&gpFECReg->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);
    
    // Set the station address for the FEC Adapter
    OUTREG32(&gpFECReg->PALR, pEthernet->FecMacAddress[3] |
                              pEthernet->FecMacAddress[2]<<8 |
                              pEthernet->FecMacAddress[1]<<16 |
                              pEthernet->FecMacAddress[0]<<24);
                              
    OUTREG32(&gpFECReg->PAUR, pEthernet->FecMacAddress[5]<<16 |
                              pEthernet->FecMacAddress[4]<<24);
    
    for(i = 0; i < FEC_MII_COUNT-1; i++)
    {
        gMIICmds[i].MIINext = &gMIICmds[i+1];
    }
    
    gMIIFree = gMIICmds;
    
    // setup MII interface
    FECSetMII(pFEC);
    
    if (pFEC->fUseRMII)
    {    
        // switch the FEC in RMII mode    
        OUTREG32(&gpFECReg->MIIGSK_CFGR,FEC_MIIGSK_CFGR_IF_MODE_RMII);
        //Enable frame control pause 
        INSREG32BF(&gpFECReg->RCR, FEC_RCR_FCE, 1);
        OUTREG32(&gpFECReg->TCR,0x1c);
    }

    
    // Queue up command to detect the PHY and initialize the remainder of 
    // the interface
    pFEC->TxFull = FALSE;
    pFEC->MIIPhyIdDone = FALSE;
    pFEC->MIIPhyAddr = 0;
    pFEC->MIISeqDone = FALSE;
    pFEC->LinkStatus = FALSE;
    
    pFEC->MIIPhy = NULL;
    
    FECQueueMII( pFEC, MII_READ_COMMAND(MII_REG_PHYIR1), FECGetPHYId );
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECEnetInit\r\n")));
    
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: FECEnetDeinit
//
// This function Deinitialize the FEC hardware. It will free the DMA buffers and
// disable the GPIO and CLK for FEC.
//
// Parameters:
//        pEthernet
//            [in] the FEC driver context area allocated in function FECInitialize
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void FECEnetDeinit(pFEC_t pEthernet)
{
    PHYSICAL_ADDRESS    MemPhysicalBase;
    DMA_ADAPTER_OBJECT Adapter;
    
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECEnetDeinit\r\n")));
    
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
    
    MemPhysicalBase.QuadPart = 0;
    
    // Free DMA buffers allocated(Note: only virtual address is needed)
    HalFreeCommonBuffer(&Adapter, (FEC_RX_RING_SIZE + FEC_TX_RING_SIZE) * sizeof(BUFFER_DESC), MemPhysicalBase, pFEC->RingBase, FALSE);
    HalFreeCommonBuffer(&Adapter, FEC_RX_RING_SIZE * FEC_ENET_RX_FRSIZE, MemPhysicalBase, pFEC->RxBufferBase, FALSE);
    HalFreeCommonBuffer(&Adapter, FEC_TX_RING_SIZE * FEC_ENET_TX_FRSIZE, MemPhysicalBase, pFEC->TxBufferBase, FALSE);
    
    // disable GPIO and CLK for FEC
    BSPFECIomuxConfig( FALSE );
    BSPFECClockConfig( FALSE );
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECEnetDeinit\r\n")));    
}

//------------------------------------------------------------------------------
//
// Function: FECEnetAutoNego
//
// This function issues the auto-negotiation sequences to the external PHY device
//
// Parameters:
//        pEthernet
//            [in] the FEC driver context area allocated in function FECInitialize
//
// Return value
//        None
//
//------------------------------------------------------------------------------
void FECEnetAutoNego(pFEC_t pEthernet)
{
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECEnetAutoNego\r\n")));
    
    // Wait for MII interrupt to set to TURE
    while(pFEC->MIIPhyIdDone != TRUE)  
         Sleep(10);
        
    if(pFEC->MIIPhy)
    {
        // Set to auto-negotiation mode and restart the
        // auto-negotiation process
        FECDoMIICmd(pFEC, pFEC->MIIPhy->PhyActint);
        FECDoMIICmd(pFEC, pFEC->MIIPhy->PhyConfig);
        FECDoMIICmd(pFEC, PHYCmdCfg);
        
    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECEnetAutoNego\r\n")));
}


//------------------------------------------------------------------------------
//
// Function: FECEnetPowerHandle
//
// This function is called to handle fec phy suspend and resume
//
// Parameters:
//        pEthernet
//            [in] the FEC driver context area allocated in function FECInitialize

//
//         En
//            [in] TRUE for fec phy power resume  , FALSE for fec power suspend
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------

void FECEnetPowerHandle(pFEC_t pEthernet,BOOL En)
{
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECEnetPowerHandle\r\n")));
 
      
    if(pFEC->MIIPhy)
    {
        
        if(En)
            FECDoMIICmd(pFEC, PHYCmdResume);
        else           
            FECDoMIICmd(pFEC, PHYCmdSuspend);

    }
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECEnetPowerHandle\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECEnetReset
//
// This function is called to restart the FEC hardware. Linking status change
// or switching between half and full duplex mode will cause this function to
// be called.
//
// Parameters:
//        MiniportAdapterHandle
//            [in] Specifies the handle to the driver allocated context area in 
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize
//
//        DuplexMode
//            [in] TRUE for full duplex mode, FALSE for half duplex mode
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECEnetReset(
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN BOOL DuplexMode
    )
{
    volatile PBUFFER_DESC BufferDescPointer;
    UINT i;
    
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterHandle));
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECEnetReset\r\n")));
    
    // Issues a hardware reset, we should wait for this
    INSREG32BF(&gpFECReg->ECR, FEC_ECR_RESET, FEC_ECR_RESET_RESET);
    
    while(EXTREG32BF(&gpFECReg->ECR, FEC_ECR_RESET));
    
    // Enable RxF, TxF and MII interrupts
    OUTREG32(&gpFECReg->EIMR,
                    CSP_BITFVAL(FEC_EIMR_TXF, FEC_EIMR_TXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_RXF, FEC_EIMR_RXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_UNMASK));
                    
    // Clear all pending interrupts
    OUTREG32(&gpFECReg->EIR, FEC_EIR_CLEARALL_MASK);
    
    // Set the station MAC address
    //FECSetAddr(MiniportAdapterHandle);
    
    // Reset all multicast hashtable and individual hashtable
    // NIDS library will restore this by calling 
    // FECSetInformation() function
    OUTREG32(&gpFECReg->IAUR, 0);
    OUTREG32(&gpFECReg->IALR, 0);
    OUTREG32(&gpFECReg->GAUR, 0);
    OUTREG32(&gpFECReg->GALR, 0);
    
    // Set maximum receive buffer size, the size must be aligned with
    // 16 bytes
    OUTREG32(&gpFECReg->EMRBR, PKT_MAXBLR_SIZE);
    
    // Set receive and transmit descriptor base
    OUTREG32(&gpFECReg->ERDSR, (ULONG)pFEC->RingPhysicalBase.QuadPart);
    OUTREG32(&gpFECReg->ETDSR,
            (ULONG)pFEC->RingPhysicalBase.QuadPart + FEC_RX_RING_SIZE*sizeof(BUFFER_DESC));
            
    pFEC->DirtyTx = pFEC->CurrentTx = pFEC->TxBufferDescBase;
    pFEC->CurrentRx = pFEC->RxBufferDescBase;
    
    // Initialize the receive buffer descriptors
    BufferDescPointer = pFEC->RxBufferDescBase;
    
    for(i = 0; i < FEC_RX_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = BD_ENET_RX_EMPTY;
        BufferDescPointer++;
    }
    
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;
    
    
    // Initialize the transmit buffer descriptor
    BufferDescPointer = pFEC->TxBufferDescBase;
    
    for(i = 0; i < FEC_TX_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = 0;
        BufferDescPointer++;
    }
    
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;
    
    // Enable MII mode and set the Duplex mode
    if(DuplexMode)
    {
        // MII mode enable and FD(Full Duplex) enable 
        OUTREG32(&gpFECReg->RCR,
                    CSP_BITFVAL(FEC_RCR_MAXFL, PKT_MAXBUF_SIZE)|
                    CSP_BITFVAL(FEC_RCR_MIIMODE, FEC_RCR_MIIMODE_ENABLE));
                    
        OUTREG32(&gpFECReg->TCR,
                    CSP_BITFVAL(FEC_TCR_FDEN, FEC_TCR_FDEN_ENABLE)); 
    }
    else
    {
        // MII mode enable and FD disable
        OUTREG32(&gpFECReg->RCR,
                    CSP_BITFVAL(FEC_RCR_MAXFL, PKT_MAXBUF_SIZE)|
                    CSP_BITFVAL(FEC_RCR_MIIMODE, FEC_RCR_MIIMODE_ENABLE)|
                    CSP_BITFVAL(FEC_RCR_DRT, FEC_RCR_DRT_ENABLE));
                    
        OUTREG32(&gpFECReg->TCR, 0);
    }


    pFEC->FullDuplex = DuplexMode;
    
    
    if (pFEC->fUseRMII)
    {
        // switch the FEC in RMII mode    
        OUTREG32(&gpFECReg->MIIGSK_CFGR,FEC_MIIGSK_CFGR_IF_MODE_RMII);
    }

    // Set MII speed
    OUTREG32(&gpFECReg->MSCR,
                     CSP_BITFVAL(FEC_MSCR_MIISPEED, pFEC->MIIPhySpeed));
                     
    // Finally, enable the fec and enable receive/transmit processing
    INSREG32BF(&gpFECReg->ECR, FEC_ECR_ETHEREN, FEC_ECR_ETHEREN_ENABLE);
    
    INSREG32BF(&gpFECReg->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECEnetReset\r\n")));
    
    return;     
}

//------------------------------------------------------------------------------
//
// Function: CalculateHashValue
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
UCHAR CalculateHashValue(UCHAR *pAddr)
{
    ULONG CRC;
    UCHAR HashValue = 0;
    UCHAR AddrByte;
    int byte, bit;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +CalculateHashValue\r\n")));
    
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
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -CalculateHashValue [0x%.8X]\r\n"), HashValue));
    
    return HashValue;

}


//------------------------------------------------------------------------------
//
// Function: AddMultiCast
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
void AddMultiCast( UCHAR *pAddr )
{
    UCHAR HashValue = 0;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +AddMultiCast\r\n")));
    
    HashValue = CalculateHashValue( pAddr );
    
    if( HashValue > 31 )
        SETREG32(&gpFECReg->GAUR, 1 << (HashValue-32));
    else
        SETREG32(&gpFECReg->GALR, 1 << HashValue);
    
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -AddMultiCast\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ClearAllMultiCast
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
void ClearAllMultiCast()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +ClearAllMultiCast\r\n")));
    
    // clear all GAUR and GALR bits
    
    CLRREG32(&gpFECReg->GAUR, 0xFFFFFFFF);
    CLRREG32(&gpFECReg->GALR, 0xFFFFFFFF);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -ClearAllMultiCast\r\n")));
}
