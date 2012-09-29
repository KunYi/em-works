//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008£¬ Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  interrupt.c
//
//  Implementation of FEC Driver
//
//  This file implements the interrupt handler functions  for FEC.
//
//-----------------------------------------------------------------------------

#include "fec.h"

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------
extern PCSP_FEC_REGS    gpFECReg;

extern PFEC_MII_LIST    gMIIFree;
extern PFEC_MII_LIST    gMIIHead;

extern CRITICAL_SECTION gFECRegCs;
extern CRITICAL_SECTION gFECBufCs;

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Function: FECIsr
//
// The FEC adapter driver should do as little work as possible in the FECIsr,
// deferring I/O operations for each interrupt the FEC adapter generates to 
// the FECHandleInterrupt function.
//
// Parameters:
//        InterruptRecognized
//            [out] Points to a variable in which FECISR returns whether
//                  the FEC adapter actually generated the interrupt.
//
//        QueueMiniportHandleInterrupt
//            [out] Points to a variable that FECISR sets to TRUE if the 
//                  FECHandleInterrupt function should be called to complete
//                  the interrupt-driven I/O operation.
//
//        MiniportAdapterContext
//            [in]  Specifies the handle to the driver allocated context area in
//                  which the driver maintains FEC adapter state, set up by
//                  FECInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void FECIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
    IN  NDIS_HANDLE MiniportAdapterContext
    )
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+FEC: FECIsr\r\n")));
    
    // Set to TRUE, FEC IRQ is not shared with other network
    // adapter
    *InterruptRecognized = TRUE;
    
    // FECHandleInterrupt will be called to complete the
    // operation
    *QueueMiniportHandleInterrupt = TRUE;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-FEC: FECIsr\r\n")));
    
    return;
}

//------------------------------------------------------------------------------
//
// Function: FECHandleInterrupt
//
// This function handles the interrupt events and calls the respective subroutines
// according to event.
//
// Parameters:
//        MiniportAdapterContext
//            [in]  Specifies the handle to the driver allocated context area in
//                  which the driver maintains FEC adapter state, set up by
//                  FECInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void FECHandleInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{
    UINT InterruptEvent;
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECHandleInterrupt\r\n")));
    
    // Check the hardware status of the FEC adapter, 
    // if not ready, return
    if( pEthernet->CurrentState != NdisHardwareStatusReady )
    {
        return;
    }
    
    pEthernet->ReceiveCompleteNotifyFlag = FALSE;
    
    // Check which interrupt
    while( (InterruptEvent = INREG32(&gpFECReg->EIR)) != 0 )
    {
        // Clear the EIR
        OUTREG32(&gpFECReg->EIR, InterruptEvent);
        
        // Handle receive packet event
        if(InterruptEvent & CSP_BITFVAL(FEC_EIR_RXF, 1))
            ProcessReceiveInterrupts(pEthernet);
            
        // Handle transmit packet interrupt
        if(InterruptEvent & CSP_BITFVAL(FEC_EIR_TXF, 1))
            ProcessTransmitInterrupts(pEthernet);
            
        // Handle MII interrupt
        if(InterruptEvent & CSP_BITFVAL(FEC_EIR_MII, 1))
            ProcessMIIInterrupts(pEthernet);
    }
    
     // check for ReceiveCompleteNotifyFlag
    if (pEthernet->ReceiveCompleteNotifyFlag == TRUE)
    {
        NdisMEthIndicateReceiveComplete( pEthernet->ndisAdapterHandle );
    }

    // Handle packets that have not been transmit yet
    if ((pEthernet->StartTx == TRUE) && (pEthernet->HeadPacket != NULL))
    {
        pEthernet->StartTx = FALSE;
        FECStartXmit(pEthernet);
    }
    else
    {
        pEthernet->StartTx = FALSE;
    }

    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECHandleInterrupt\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: FECDisableInterrupt
//
// This function accesses the interrupt mask register to disable the RxF, TxF
// and MII interrupts.
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void FECDisableInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECDisableInterrupt\r\n")));
    
    // Check current hardware status
    if(pEthernet->CurrentState != NdisHardwareStatusReady)
    {
        return;
    }
    
    // Disable TxF, RxF and MII interrupts
    OUTREG32(&gpFECReg->EIMR,
                    CSP_BITFVAL(FEC_EIMR_TXF, FEC_EIMR_TXF_MASK) |
                    CSP_BITFVAL(FEC_EIMR_RXF, FEC_EIMR_RXF_MASK) |
                    CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_MASK));
                    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECDisableInterrupt\r\n")));
    
    return;
}

//------------------------------------------------------------------------------
//
// Function: FECEnableInterrupt
//
// This function accesses the interrupt mask register to enable the RxF, TxF
// and MII interrupts.
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void FECEnableInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{
    pFEC_t pEthernet = ((pFEC_t)(MiniportAdapterContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +FECEnableInterrupt\r\n")));
    
    // Check current hardware status
    if(pEthernet->CurrentState != NdisHardwareStatusReady)
    {
        return;
    }
    
    // Enable TxF, RxF and MII interrupts
    OUTREG32(&gpFECReg->EIMR,
                    CSP_BITFVAL(FEC_EIMR_TXF, FEC_EIMR_TXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_RXF, FEC_EIMR_RXF_UNMASK) |
                    CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_UNMASK));
                    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -FECEnableInterrupt\r\n")));
    
    return;
}


//------------------------------------------------------------------------------
//
// Function: ProcessReceiveInterrupts
//
// This function is the interrupt handler when a ethernet packet is received by
// the FEC hardware.
//
// Parameters:
//        pEthernet
//            [in]  Specifies the pointer to the driver allocated context area in
//                  which the driver maintains FEC adapter state, set up by
//                  FECInitialize
//
// Return Value:
//        None
//------------------------------------------------------------------------------
void ProcessReceiveInterrupts(
        IN pFEC_t pEthernet)
{
    volatile PBUFFER_DESC BufferDescPointer;
    USHORT PacketSize;
    PUCHAR pReceiveBuffer, pData;
    
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +ProcessReceiveInterrupts\r\n")));
    
    BufferDescPointer = pFEC->CurrentRx;
    
    DEBUGMSG(1, (TEXT("The value of ControlStatus is = %Xh\r\n"), BufferDescPointer->ControlStatus));
    
    while(!(BufferDescPointer->ControlStatus & BD_ENET_RX_EMPTY)) 
    {
        // Since we have allocated space to hold a complete 
        // frame, the last indicator should be set
        if((BufferDescPointer->ControlStatus & BD_ENET_RX_LAST) == 0)
            DEBUGMSG(ZONE_INFO, (TEXT("RCV is not the last\r\n")));
            
        // Check for errors
        if(BufferDescPointer->ControlStatus & (BD_ENET_RX_LG | BD_ENET_RX_SH |
                                               BD_ENET_RX_NO | BD_ENET_RX_CR |
                                               BD_ENET_RX_OV | BD_ENET_RX_CL ))
        {
            pEthernet->RcvStatus.FrameRcvErrors++;
            
            if(BufferDescPointer->ControlStatus & BD_ENET_RX_LG)  // too long frame
                pEthernet->RcvStatus.FrameRcvExtraDataErrors++;
                
            if(BufferDescPointer->ControlStatus & BD_ENET_RX_SH)  // too short frame
                pEthernet->RcvStatus.FrameRcvShortDataErrors++;
                
            if(BufferDescPointer->ControlStatus & BD_ENET_RX_NO)  // no-octet aligned frame
                pEthernet->RcvStatus.FrameRcvAllignmentErrors++;    
                
            if(BufferDescPointer->ControlStatus & BD_ENET_RX_CR)  // CRC error
                pEthernet->RcvStatus.FrameRcvCRCErrors++;
                
            if(BufferDescPointer->ControlStatus & BD_ENET_RX_OV)  // receive FIFO overrun
                pEthernet->RcvStatus.FrameRcvOverrunErrors;
                
            if(BufferDescPointer->ControlStatus & BD_ENET_RX_CL)  // late collisions error
                pEthernet->RcvStatus.FrameRcvLCErrors++;
        }
        else
        {
            // Process the successfully received frame
            pEthernet->RcvStatus.FrameRcvGood++;
            
            // Get frame size of the received frame
            PacketSize = BufferDescPointer->DataLen;
            PacketSize -= 4;
            
            // Assign a pointer to the receive buffer
            pReceiveBuffer = pEthernet->ReceiveBuffer;
            pEthernet->ReceivePacketSize = PacketSize;
            
            pData = pFEC->RxBuffAddr[BufferDescPointer - pFEC->RxBufferDescBase];
            
            memcpy(pReceiveBuffer, pData, PacketSize);
            
            NdisMEthIndicateReceive( pEthernet->ndisAdapterHandle,
                                     (NDIS_HANDLE)pEthernet,
                                     (PCHAR)pEthernet->ReceiveBuffer,
                                     ETHER_HDR_SIZE,
                                     pEthernet->ReceiveBuffer +ETHER_HDR_SIZE,
                                     PacketSize-ETHER_HDR_SIZE,
                                     PacketSize-ETHER_HDR_SIZE
                                     );
                                     
            
            pEthernet->ReceiveCompleteNotifyFlag = TRUE;

        }
        
        // Clear the status flags for this BD
        BufferDescPointer->ControlStatus &= ~BD_ENET_RX_STATS;
        
        // Mark the buffer as empty
        BufferDescPointer->ControlStatus |= BD_ENET_RX_EMPTY;
        
        // Update BD pointer to next entry 
        if(BufferDescPointer->ControlStatus & BD_ENET_RX_WRAP)
            BufferDescPointer = pFEC->RxBufferDescBase;
        else
            BufferDescPointer++;
            
        INSREG32BF(&gpFECReg->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);
    }
    
    
    pFEC->CurrentRx = (PBUFFER_DESC)BufferDescPointer;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -ProcessReceiveInterrupts\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ProcessTransmitInterrupts
//
// This function is the interrupt handler when a ethernet packet had been
// transmitted by the FEC hardware.
//
// Parameters:
//        pEthernet
//            [in] Specifies the pointer to the driver allocated context area in
//                 which the driver maintains FEC adapter state, set up by
//                 FECInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------

void ProcessTransmitInterrupts(
        IN pFEC_t pEthernet)
{
    volatile PBUFFER_DESC BufferDescPointer;
    PNDIS_PACKET pNdisPacket;
    
    PFEC_ENET_PRIVATE pFEC = &(pEthernet->FECPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +ProcessTransmitInterrupts\r\n")));
    
    BufferDescPointer = pFEC->DirtyTx;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("The value of ControlStatus is = %Xh\r\n"), BufferDescPointer->ControlStatus));
    
    while((BufferDescPointer->ControlStatus & BD_ENET_TX_READY) == 0)
    {
        if((BufferDescPointer == pFEC->CurrentTx) && (pFEC->TxFull == FALSE) )
            break;
            
        if(BufferDescPointer->ControlStatus & (BD_ENET_TX_HB | BD_ENET_TX_LC |
                                               BD_ENET_TX_RL | BD_ENET_TX_UN |
                                               BD_ENET_TX_CSL))
        {
            pEthernet->TxdStatus.FramesXmitBad++;
            
            if(BufferDescPointer->ControlStatus & BD_ENET_TX_HB)  // No heartbeat
                pEthernet->TxdStatus.FramesXmitHBErrors++;
                
            if(BufferDescPointer->ControlStatus & BD_ENET_TX_LC)  // Late collision 
                pEthernet->TxdStatus.FramesXmitCollisionErrors++;
                
            if(BufferDescPointer->ControlStatus & BD_ENET_TX_RL)  // Retrans limit
                pEthernet->TxdStatus.FramesXmitAbortedErrors++;
                
            if(BufferDescPointer->ControlStatus & BD_ENET_TX_UN)  // Underrun
                pEthernet->TxdStatus.FramesXmitUnderrunErrors++;
                
            if(BufferDescPointer->ControlStatus & BD_ENET_TX_CSL) // Carrier lost
                pEthernet->TxdStatus.FramsXmitCarrierErrors++;
        }
        else
        {
            pEthernet->TxdStatus.FramesXmitGood++;
            
            // Remove the successfully transmitted packet from the
            // packet queue
            EnterCriticalSection (&gFECBufCs);
            
            pNdisPacket = pEthernet->HeadPacket;
            pEthernet->HeadPacket = RESERVED(pNdisPacket)->Next;

            if (pNdisPacket == pEthernet->TailPacket) 
            {
                pEthernet->TailPacket = NULL;
            }

            LeaveCriticalSection (&gFECBufCs);

            NdisMSendComplete(pEthernet->ndisAdapterHandle, pNdisPacket, NDIS_STATUS_SUCCESS);
        }
        
        // Update pointer to next buffer descriptor to be transmitted
        if(BufferDescPointer->ControlStatus & BD_ENET_TX_WRAP)
            BufferDescPointer = pFEC->TxBufferDescBase;
        else
            BufferDescPointer++;
            
        // Since we have freed up a buffer, the ring is no longer full
        if(pFEC->TxFull == TRUE)
        {
            pFEC->TxFull = FALSE;
            // NdisMSendComplete(pEthernet->ndisAdapterHandle, pNdisPacket, NDIS_STATUS_SUCCESS);
        }
    }
    
    pFEC->DirtyTx = BufferDescPointer;
    
    if (pEthernet->HeadPacket != NULL )
    {
        pEthernet->StartTx = TRUE;
    }
    else 
    {
        pEthernet->TransmitInProgress = FALSE;
    }

    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -ProcessTransmitInterrupts\r\n")));
} 

//------------------------------------------------------------------------------
//
// Function: ProcessMIIInterrupts
// 
// This function is the interrupt handler for the MII interrupt
//
// Parameters:
//        pEthernet
//            [in]  Specifies the pointer to the driver allocated context area in
//                  which the driver maintains FEC adapter state, set up by
//                  FECInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ProcessMIIInterrupts(IN pFEC_t pEthernet)
{
    PFEC_MII_LIST MIIPoint;
    UINT MIIReg;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: +ProcessMIIInterrupts\r\n")));
    
    // Get the MII frame value from MMFR register
    MIIReg = INREG32( &gpFECReg->MMFR );
    
    if( (MIIPoint = gMIIHead) == NULL )
    {
        DEBUGMSG(ZONE_INFO,
            (TEXT("%s: MII head is null\r\n"), __WFUNCTION__));
            
        return;
    }
    
    if( MIIPoint->MIIFunction != NULL )
        (*(MIIPoint->MIIFunction))(MIIReg, (NDIS_HANDLE)pEthernet);
        
    gMIIHead = MIIPoint->MIINext;
    MIIPoint->MIINext = gMIIFree;
    gMIIFree = MIIPoint;
        
    // Send the next MMI management frame to the external 
    // PHY if any
    if( (MIIPoint = gMIIHead) != NULL )
        OUTREG32( &gpFECReg->MMFR, MIIPoint->MIIRegVal );        
    
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FEC: -ProcessMIIInterrupts\r\n")));
    
    return;
}
