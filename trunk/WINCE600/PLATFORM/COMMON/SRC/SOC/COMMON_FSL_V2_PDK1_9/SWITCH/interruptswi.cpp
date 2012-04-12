//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2010£¬ Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  interruptswi.c
//
//  Implementation of ENET SWITCH Driver
//
//  This file implements the interrupt handler functions for ENET.
//
//-----------------------------------------------------------------------------
#include "common_macros.h"
#include "enetswi.h"
#include <winsock2.h>


extern HANDLE hMiiMutex;


//------------------------------------------------------------------------------
//
// Function: CopyFrame
//
//------------------------------------------------------------------------------
VOID CopyFrame(UINT8 *pDes, UINT8 *pSrc, UINT32 cnt)
{
    long *plDes = (long *)pDes;
    long *plSrc = (long *)pSrc;

   
    cnt = cnt / sizeof(long);
    while (cnt--)
    {
        *plDes = htonl(*plSrc);    
         plDes++;  plSrc++;
    }
}

VOID ExchangeFrame( UINT8 *pSrc, UINT32 cnt)
{
 
    long *plSrc = (long *)pSrc;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+ENET: pSrc:%x cnt:%d \r\n"), pSrc, cnt));

    cnt = cnt / sizeof(long);
    
    while (cnt--)
    {    
        *plSrc = htonl(*plSrc);    
         plSrc++;
    }
}


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
extern PVOID pv_HWregENET0;
extern PVOID pv_HWregENET1;
extern PVOID pv_HWregENETSWI;
extern PVOID pv_HWLMENETSWI;

extern HANDLE hMiiEvent;

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  Function:  EnetMiiHandleThread
//
//  This is the service thread for processing Enent Mii  handle.  
//
//  Parameters:
//      lpParam
//          [in] Thread data passed to the function using the 
//          lpParameter parameter of the CreateThread function. Not used.
//
//  Returns:
//      Returns thread exit code.
//
//-----------------------------------------------------------------------------
BOOL WINAPI EnetMiiHandleThread (LPVOID lpParam)
{
    BOOL retVal = TRUE;   
    pENET_t pEthernet = (pENET_t)lpParam;

    CeSetThreadPriority(GetCurrentThread(), 1);
    while(retVal)
    {
        if (WaitForSingleObject(hMiiEvent, INFINITE) == WAIT_OBJECT_0)
        {
            
            pEthernet->pENET->ProcessMIIInterrupts( pEthernet);
             
        }
    }

    return retVal;

}



//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETIsr
//
// The ENET adapter driver should do as little work as possible in the ENETIsr,
// deferring I/O operations for each interrupt the ENET adapter generates to 
// the ENETHandleInterrupt function.
//
// Parameters:
//        InterruptRecognized
//            [out] Points to a variable in which ENETISR returns whether
//                  the ENET adapter actually generated the interrupt.
//
//        QueueMiniportHandleInterrupt
//            [out] Points to a variable that ENETISR sets to TRUE if the 
//                  ENETHandleInterrupt function should be called to complete
//                  the interrupt-driven I/O operation.
//
//        MiniportAdapterContext
//            [in]  Specifies the handle to the driver allocated context area in
//                  which the driver maintains ENET adapter state, set up by
//                  ENETInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETClass::ENETIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
    IN  NDIS_HANDLE MiniportAdapterContext
    )
{
    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+ENET: ENETIsr\r\n")));
    
    // Set to TRUE, ENET IRQ is not shared with other network
    // adapter
    *InterruptRecognized = TRUE;
    
    // ENETHandleInterrupt will be called to complete the
    // operation
    *QueueMiniportHandleInterrupt = TRUE;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-ENET: ENETIsr\r\n")));
    
    return;
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETHandleInterrupt
//
// This function handles the interrupt events and calls the respective subroutines
// according to event.
//
// Parameters:
//        MiniportAdapterContext
//            [in]  Specifies the handle to the driver allocated context area in
//                  which the driver maintains ENET adapter state, set up by
//                  ENETInitialize.
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ENETHandleInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{
    UINT InterruptEvent[3];
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
      
    // Check the hardware status of the ENET adapter, 
    // if not ready, return
    if( pEthernet->CurrentState != NdisHardwareStatusReady )
    {
        return;
    }
    
    pEthernet->ReceiveCompleteNotifyFlag = FALSE;
    
    // Check which interrupt
    InterruptEvent[0] = HW_ENET_MAC_EIR_RD(0);
    InterruptEvent[1] = HW_ENET_MAC_EIR_RD(1);
    InterruptEvent[2] = HW_ENET_SWI_EIR_RD();
    while(((InterruptEvent[0]&BM_ENET_MAC_EIR_MII) != 0 )|| (InterruptEvent[2]!= 0 ))
    {
        // Clear the EIR
        HW_ENET_MAC_EIR_WR(0,InterruptEvent[0]);
        HW_ENET_MAC_EIR_WR(1,InterruptEvent[1]);
        HW_ENET_SWI_EIR_WR(InterruptEvent[2]);
        // RETAILMSG(1, (TEXT("EIR MAC0 0x%x MAC1 0x%x SWI 0x%x\r\n"),InterruptEvent[0],InterruptEvent[1],InterruptEvent[2]));
        
        // Handle receive packet event
        if(InterruptEvent[2] & BM_ENET_SWI_EIR_RXF)
            ProcessReceiveInterrupts(pEthernet);
            
        // Handle transmit packet interrupt
        if(InterruptEvent[2] &BM_ENET_SWI_EIR_TXF)
            ProcessTransmitInterrupts(pEthernet);
            
        // Handle MII interrupt
        if(InterruptEvent[0] & BM_ENET_MAC_EIR_MII)
        {           
            WaitForSingleObject(hMiiMutex, INFINITE);
            ProcessMIIInterrupts(pEthernet);
           ReleaseMutex(hMiiMutex);
        }    

        InterruptEvent[0] = HW_ENET_MAC_EIR_RD(0);
        InterruptEvent[2] = HW_ENET_SWI_EIR_RD();
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
        ENETStartXmit(pEthernet);
    }
    else
    {
        pEthernet->StartTx = FALSE;
    }
  
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETHandleInterrupt\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETDisableInterrupt
//
// This function accesses the interrupt mask register to disable the RxF, TxF
// and MII interrupts.
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETClass::ENETDisableInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{
    DWORD indextmp;
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: +ENETDisableInterrupt\r\n")));
    
    // Check current hardware status
    if(pEthernet->CurrentState != NdisHardwareStatusReady)
    {
        return;
    }
    
    // Disable TxF, RxF and MII interrupts
    for(indextmp=0;indextmp<2;indextmp++)
    {
        HW_ENET_MAC_EIMR_WR(indextmp,0x0);
    }
    HW_ENET_SWI_EIMR_WR(0x0);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETDisableInterrupt\r\n")));
    
    return;
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ENETEnableInterrupt
//
// This function accesses the interrupt mask register to enable the RxF, TxF
// and MII interrupts.
//
// Parameters:
//        MiniportAdapterContext
//            [in] Specifies the handle to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETClass::ENETEnableInterrupt(IN NDIS_HANDLE MiniportAdapterContext)
{
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterContext));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET%x: +ENETEnableInterrupt\r\n"),switchindex));
    
    // Check current hardware status
    if(pEthernet->CurrentState != NdisHardwareStatusReady)
    {
        return;
    }   
    
    // Enable TxF, RxF,TS_AVAIL, TS_TIMER and MII interrupts
    HW_ENET_MAC_EIMR_WR(0,BM_ENET_MAC_EIR_MII);   
    HW_ENET_MAC_EIMR_WR(1,0);   
    HW_ENET_SWI_EIMR_WR(BM_ENET_SWI_EIMR_EBERR|BM_ENET_SWI_EIMR_RXF|BM_ENET_SWI_EIMR_TXF|BM_ENET_SWI_EIMR_QM|BM_ENET_SWI_EIMR_OD0|BM_ENET_SWI_EIMR_OD1|BM_ENET_SWI_EIMR_OD2);   
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ENETEnableInterrupt\r\n")));
    
    return;
}


//------------------------------------------------------------------------------
//
// Function: ENETClass::ProcessReceiveInterrupts
//
// This function is the interrupt handler when a ethernet packet is received by
// the ENET hardware.
//
// Parameters:
//        pEthernet
//            [in]  Specifies the pointer to the driver allocated context area in
//                  which the driver maintains ENET adapter state, set up by
//                  ENETInitialize
//
// Return Value:
//        None
//------------------------------------------------------------------------------
void ENETClass::ProcessReceiveInterrupts(
         IN NDIS_HANDLE MiniportAdapterHandle)
{
    volatile PEBUFFER_DESC BufferDescPointer;
    USHORT PacketSize=0;
    PUCHAR pReceiveBuffer=NULL, pData;
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET%x: +ProcessReceiveInterrupts\r\n"),switchindex));
    
    BufferDescPointer = pENET->CurrentRx;
    
    DEBUGMSG(1, (TEXT("The value of ControlStatus is = %Xh\r\n"), BufferDescPointer->ControlStatus));
    
    while(!(BufferDescPointer->ControlStatus & BD_ENET_RX_EMPTY)) 
    {
        // Since we have allocated space to hold a complete 
        // frame, the last indicator should be set
        if((BufferDescPointer->ControlStatus & BD_ENET_RX_LAST) == 0)
            DEBUGMSG(ZONE_INFO, (TEXT("RCV is not the last\r\n")));

          PacketSize = BufferDescPointer->DataLen;
            
        // Check for errors
        if((BufferDescPointer->ControlStatus & (BD_ENET_RX_LG | BD_ENET_RX_SH |
                                               BD_ENET_RX_NO | BD_ENET_RX_CR |
                                               BD_ENET_RX_OV | BD_ENET_RX_CL ))||(PacketSize==0))
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
          
            // PacketSize -= 4;
            
            // Assign a pointer to the receive buffer
            pReceiveBuffer = pEthernet->ReceiveBuffer;
            pEthernet->ReceivePacketSize = PacketSize;
            
            pData = pENET->RxBuffAddr[BufferDescPointer - pENET->RxBufferDescBase];
            

            CopyFrame(pReceiveBuffer, pData, PacketSize + (4 - PacketSize%4));

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
   
        //when the PTP message type ,we will get timestamp

        DEBUGMSG(ZONE_FUNCTION, (TEXT("ProcessReceiveInterrupts: %x %x %x \r\n"),pReceiveBuffer[12],pReceiveBuffer[13],pReceiveBuffer[14]));
            
        // Clear the status flags for this BD
        BufferDescPointer->ControlStatus &= ~BD_ENET_RX_STATS;
        
        // Mark the buffer as empty
        BufferDescPointer->ControlStatus |= BD_ENET_RX_EMPTY;
        
        // Update BD pointer to next entry 
        if(BufferDescPointer->ControlStatus & BD_ENET_RX_WRAP)
            BufferDescPointer = pENET->RxBufferDescBase;
        else
            BufferDescPointer++;
            
        BW_ENET_SWI_RDAR_RDAR(1);
    }
    
    
    pENET->CurrentRx = (PEBUFFER_DESC)BufferDescPointer;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ProcessReceiveInterrupts\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: ENETClass::ProcessTransmitInterrupts
//
// This function is the interrupt handler when a ethernet packet had been
// transmitted by the ENET hardware.
//
// Parameters:
//        pEthernet
//            [in] Specifies the pointer to the driver allocated context area in
//                 which the driver maintains ENET adapter state, set up by
//                 ENETInitialize.
//
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void ENETClass::ProcessTransmitInterrupts(
        IN NDIS_HANDLE MiniportAdapterHandle)
{
    volatile PEBUFFER_DESC BufferDescPointer;
    PNDIS_PACKET pNdisPacket;
     pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    PENET_ENET_PRIVATE pENET = &(pEthernet->ENETPrivateInfo);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET%x: +ProcessTransmitInterrupts\r\n"),switchindex));
    
    BufferDescPointer = pENET->DirtyTx;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("The value of ControlStatus is = %Xh\r\n"), BufferDescPointer->ControlStatus));
    
    while((BufferDescPointer->ControlStatus & BD_ENET_TX_READY) == 0)
    {
        if((BufferDescPointer == pENET->CurrentTx) && (pENET->TxFull == FALSE) )
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
            EnterCriticalSection (&gENETBufCs);
            
            pNdisPacket = pEthernet->HeadPacket;
            pEthernet->HeadPacket = RESERVED(pNdisPacket)->Next;

            if (pNdisPacket == pEthernet->TailPacket) 
            {
                pEthernet->TailPacket = NULL;
            }

            LeaveCriticalSection (&gENETBufCs);

            NdisMSendComplete(pEthernet->ndisAdapterHandle, pNdisPacket, NDIS_STATUS_SUCCESS);
        }
        
        // Update pointer to next buffer descriptor to be transmitted
        if(BufferDescPointer->ControlStatus & BD_ENET_TX_WRAP)
            BufferDescPointer = pENET->TxBufferDescBase;
        else
            BufferDescPointer++;
            
        // Since we have freed up a buffer, the ring is no longer full
        if(pENET->TxFull == TRUE)
        {
            pENET->TxFull = FALSE;
      
        }
    }
    
    pENET->DirtyTx = BufferDescPointer;  
    if (pEthernet->HeadPacket != NULL )
    {
        pEthernet->StartTx = TRUE;
    }
    else 
    {
        pEthernet->TransmitInProgress = FALSE;
    }
  
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET: -ProcessTransmitInterrupts\r\n")));
} 

//------------------------------------------------------------------------------
//
// Function: ENETClass::ProcessMIIInterrupts
// 
// This function is the interrupt handler for the MII interrupt
//
// Parameters:
//        pEthernet
//            [in]  Specifies the pointer to the driver allocated context area in
//                  which the driver maintains ENET adapter state, set up by
//                  ENETInitialize
//
// Return Value:
//        None
//
//------------------------------------------------------------------------------
void ENETClass::ProcessMIIInterrupts( IN NDIS_HANDLE MiniportAdapterHandle)
{
    PENET_MII_LIST MIIPoint;
    UINT MIIReg;
    pENET_t pEthernet = ((pENET_t)(MiniportAdapterHandle));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET%x: +ProcessMIIInterrupts\r\n"),switchindex));
    WaitForSingleObject(hMiiMutex, INFINITE);
    // Get the MII frame value from MMFR register
    MIIReg = HW_ENET_MAC_MMFR_RD(MAC0);
    
    if( (MIIPoint = gMIIHead) == NULL )
    {
        DEBUGMSG(ZONE_INFO,
            (TEXT("ProcessMIIInterrupts: MII head is null\r\n")));
            
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
         HW_ENET_MAC_MMFR_WR(MAC0,MIIPoint->MIIRegVal);

    ReleaseMutex(hMiiMutex);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ENET%x: -ProcessMIIInterrupts\r\n"),switchindex));
    
    return;
}


