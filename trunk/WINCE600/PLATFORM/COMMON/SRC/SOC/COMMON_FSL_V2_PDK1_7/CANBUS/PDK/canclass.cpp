//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  canclass.c
//
//  This file contains CAN module can only support  main MessageBox mode not support FIFO mode
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4127 4201)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <linklist.h>
#include <windev.h>
#include <ceddk.h>
#include <stdlib.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_can.h"
#include "devload.h"
#include "canbus.h"
#include "canclass.h"

#define iIntrWaitTimeout 3000


//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines 
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function:  GetIndex
//
// This function returns the index of the can interrupt source
// 
//
// Parameters:
//      data
//          [in] Index of the CAN device requested.
//
// Returns:
//
//-----------------------------------------------------------------------------

int  GetIndex(unsigned int data)
{
    int index=0;
    
    if(data==0)
         return 0;
         
    while(data!=0)
    {
        index++;
        data=data>>1;
    }
    return index;
}


//-----------------------------------------------------------------------------
//
// Function: CANClass::CANClass 
//
// This CANClass constructor creates all the mutexes, events and heaps required
// for the subsequent use of CAN bus interface. It will also allocate the
// interrupt id from the HAL and binds the interrupt event to the interrupt id.
// This constructor has a built-in mechanism to prevent concurrent execution and a
// multiple interrupt binding procedure.
//
// Parameters:
//  None.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
CANClass::CANClass (UINT32 index)
{

    int i;
    BSPCANConfigureGPIO( index);
    BSPCANClockConfig(index,TRUE );   
    BSPSetCanPowerEnable(index,TRUE);
 
    PHYSICAL_ADDRESS phyAddr;

    // Copy CAN physical address
    phyAddr.QuadPart = BSPGetCANBaseRegAddr(index);
    // Map to virtual address
    pCANreg = (PCSP_CAN_REG) MmMapIoSpace(phyAddr, sizeof(CSP_CAN_REG), FALSE);

    // If mapping fails, fatal error, quit
    if (pCANreg == NULL)
    {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("CAN_Init:MmMapIoSpace(): Failed! ErrCode=%d \r\n"), GetLastError()));
       
        return ;
    }
    
    DEBUGMSG(ZONE_INIT, (TEXT("CAN_Init:MmMapIoSpace(): pCANreg=0x%x \r\n"), pCANreg));

    // Create Hardware Interrupt Occurrence Event
    {

        hInterrupted = CreateEvent(NULL, FALSE, FALSE, NULL);
        // Able to create or obtain the event?
        if (hInterrupted == NULL)
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("CAN_Init:CreateEvent(): Interrupt Occurrence Event (hardware) Failed! ErrCode=%d \r\n"), GetLastError()));

              return ;
        }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("CAN_Init:CreateEvent(): hInterrupted=0x%x \r\n"), hInterrupted));

   for(i=0;i<64;i++)
    {
          hRevEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
          // Able to create or obtain the event?
          if (hRevEvent[i] == NULL)
          {
              DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("CAN_Init:CreateEvent(): Interrupt Occurrence Event (hardware) Failed! ErrCode=%d \r\n"), GetLastError()));
   
                return ;
          }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("CAN_Init:CreateEvent(): hRevEvent=0x%x \r\n"), hRevEvent));

    {
        // Copy our CAN IRQ Number
        DWORD dwIrq = BSPGetCANIRQ(index);

        // Get kernel to translate IRQ -> System Interrupt ID
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq, sizeof(DWORD), &dwSysIntr, sizeof(DWORD), NULL))
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("CAN_Init:KernelIoControl(): IRQ -> SysIntr Failed! ErrCode=%d \r\n"), GetLastError()));

             return ;
        }

        if(!KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &dwSysIntr,sizeof(dwSysIntr), NULL, 0, NULL))

        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("CAN_Init:KernelIoControl():IOCTL_HAL_ENABLE_WAKE! ErrCode=%d \r\n"), GetLastError()));

            return ;
        }

    }
    DEBUGMSG(ZONE_INIT, (TEXT("CAN_Init:KernelIoControl(): dwSysIntr=0x%x \r\n"), dwSysIntr));

    // Link hInterrupted -> CAN Interrupt Pin
    {
        if (!InterruptInitialize(dwSysIntr, hInterrupted, NULL, 0))
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("CANClass::CANClass:Interruptinitialize(): Linking failed! ErrCode=%d \r\n"), GetLastError()));

             return ;
        }
    }
  // Create IST thread to receive hardware interrupts
    {
        // Initialize this to FALSE to allow the IST handler thread to start
        // up and run.
        bTerminateISTThread = FALSE;

        // Start CAN IST thread
        // Initialize thread for CAN interrupt handling
        //      pThreadAttributes = NULL (must be NULL)
        //      dwStackSize = 0 => default stack size determined by linker
        //      lpStartAddress = CANIST => thread entry point
        //      lpParameter = this => point to thread parameter
        //      dwCreationFlags = 0 => no flags
        //      lpThreadId = NULL => thread ID is not returned
        PREFAST_SUPPRESS(5451, "This warning does not apply to WinCE because it is not a 64-bit OS");
        DWORD tid;
        m_hCANIST = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CANIST, this, 0, &tid);

        if (m_hCANIST == NULL)
        {
            DEBUGMSG(ZONE_INIT,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
            iResult = CAN_ERR_INT_INIT;
            return;
        }
        else
        {
            DEBUGMSG(ZONE_INIT, (TEXT("%s: create CAN IST thread success\r\n"), __WFUNCTION__));
            DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: create CAN IST thread success, TID: %08x\r\n"), __WFUNCTION__, tid));
            CeSetThreadPriority(m_hCANIST, 100);//THREAD_PRIORITY_TIME_CRITICAL);
        }

    }

     // Setting the clock source 
    INSREG32(&pCANreg->CTRL, CSP_BITFMASK(CAN_CLK_SRC_BIT),
             CSP_BITFVAL(CAN_CLK_SRC_BIT, ENABLE));       // for 66.5M
    //INSREG32(&pCANreg->CTRL, CSP_BITFMASK(CAN_CLK_SRC_BIT),
    //         CSP_BITFVAL(CAN_CLK_SRC_BIT, DISABLE));       // for 24M


    // Enable 64 MBs
    OUTREG8(&pCANreg->MCR, 0x3f );
    
    // Enabling the module 
    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_MDIS_BIT),
             CSP_BITFVAL(CAN_MDIS_BIT, DISABLE));

    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_AEN_BIT),CSP_BITFVAL(CAN_AEN_BIT, ENABLE));

    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_FEN_BIT),
             CSP_BITFVAL(CAN_FEN_BIT, DISABLE));  

    // Programming the bit timing segments"
    //OUTREG16(&pCANreg->CTRL_H, CAN_TIMING_PARAMETERS );

#define CAN_CTRL_BIT_TIMING_250KHZ			0x05790004
    OUTREG32(&pCANreg->CTRL, CAN_CTRL_BIT_TIMING_250KHZ );


    for(i=0;i<CAN_NUMBER_OF_BUFFERS;i++)
    { 
         // inactivating the MBs
        OUTREG32( &pCANreg->MB[i].CS  , 0x0);
        // setting the ID
        OUTREG32( &pCANreg->MB[i].ID   , 0x0 );
        // setting the DATA
        OUTREG32( &pCANreg->MB[i].BYTE_0_3  , 0x0 );
        OUTREG32( &pCANreg->MB[i].BYTE_4_7 , 0x0 );
    }
 
    OUTREG32(&pCANreg->ESR,0);
    OUTREG32(&pCANreg->ECR,0);
    OUTREG32(&pCANreg->IMASK1,0);
    OUTREG32(&pCANreg->IMASK2,0);

   InitializeCriticalSection(&gcsCANBusLock);

}

//-----------------------------------------------------------------------------
//
// Function: CANClass::~CANClass
//
// This CANClass destructor releases all the mutexes, events and heaps created.
// It will also attempt to terminate the worker thread. It has built-in 
// mechanism to determine whether it is safe to unbind the interrupt event from
// the interrupt id. This is to facilitate situations where multiple
// processes have obtained the same file handle to the CAN Bus Interface.
//
// Parameters:
//  None.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
 CANClass::~CANClass()
{
    DEBUGMSG(ZONE_DEINIT, (TEXT("CANClass::~CANClass +\r\n")));

    // Release the interrupt resources
    InterruptDisable(dwSysIntr);
    DEBUGMSG(ZONE_DEINIT, (TEXT("CANClass::~CANClass: Release System Interrupt \r\n")));

    // Kill interrupt service thread
    if (m_hCANIST)
    {
        // Set the bTerminateISTThred flag to TRUE and then wake up the IST
        // thread to ensure that it will gracefully terminate (by simply
        // executing all of the "return" calls).
        bTerminateISTThread = TRUE;
        PulseEvent(hInterrupted);

        CloseHandle(m_hCANIST);
        m_hCANIST = NULL;
    }
    
    int i;
    for(i=0;i<64;i++)
    {
        CloseHandle(hRevEvent[i]);
        // Prevent the later sections from thinking it is still valid
        hRevEvent[i] = NULL;
    }

    CloseHandle(hInterrupted);
    // Prevent the later sections from thinking it is still valid
    hInterrupted = NULL;

    // Release the interrupt
    DEBUGMSG(ZONE_DEINIT, (TEXT("CANClass::~CANClass(): Releasing dwSysIntr = 0x%x \r\n"), dwSysIntr));
    if (!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &dwSysIntr, sizeof(DWORD), NULL, 0, NULL))
        DEBUGMSG(ZONE_DEINIT | ZONE_ERROR, (TEXT("ERROR: Failed to release dwSysIntr.\r\n")));                

    DEBUGMSG(ZONE_DEINIT, (TEXT("CANClass::~CANClass:DeleteCriticalSection(): Deleting CAN_IOControl Critical Section! \r\n")));


    // Release Interrupt Occurrence Event -
    {   
        if (hInterrupted != NULL)
            CloseHandle(hInterrupted);

        DEBUGMSG(ZONE_DEINIT, (TEXT("CANClass::~CANClass: Release Interrupt Occurence Event \r\n")));
    }

    // Release CAN Register Base Mapped Virtual Memory -
    {
        if (pCANreg != NULL)
            MmUnmapIoSpace((LPVOID) pCANreg, sizeof(CSP_CAN_REG));

        DEBUGMSG(ZONE_DEINIT, (TEXT("CANClass::~CANClass: Release CAN Register Base Mapped Virtual Memory \r\n")));
    }

    DEBUGMSG(ZONE_DEINIT, (TEXT("CANClass::~CANClass -\r\n")));

}

//-----------------------------------------------------------------------------
//
// Function: CANClass::CANEnableTLPRIOMode
//
// This function indicates to the driver that the can bus enter local proiy mode 
//
// Parameters:
//      ISEnable
//          [in] TRUE indicates that the canbus  is enter Local priority mode 
//         
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL CANClass::CANEnableTLPRIOMode (BOOL ISEnable)
{
   
    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_PEN_BIT), CSP_BITFVAL(CAN_PEN_BIT, ISEnable));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CANClass::CANReset
//
// This method performs a software reset on CAN internal register (MCR). It is
// important to take note that the fields of the CANClass are not modified in
// any way.
//
// Parameters:
//  None.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
BOOL CANClass::CANReset()
{
    DEBUGMSG(ZONE_FUNCTION, (_T("CANReset++\r\n")));
    
    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_SOFT_RST_BIT),CSP_BITFVAL(CAN_SOFT_RST_BIT, ENABLE));

    DEBUGMSG(ZONE_FUNCTION, (_T("CANReset--\r\n")));
    return TRUE;

}


//-----------------------------------------------------------------------------
//
// Function: CANPowerHandler
//
// This function indicates to the driver that the system is entering
// or leaving the suspend state.
//
// Parameters:
//      bOff
//          [in] TRUE indicates that the system is turning off. FALSE
//          indicates that the system is turning on.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL CANClass::CANPowerHandler(BOOL boff)
{

    DEBUGMSG(ZONE_FUNCTION, (_T("CANPowerHandler+:%d\r\n"),boff));
    
    if(boff)
        INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_DOZE_BIT), CSP_BITFVAL(CAN_DOZE_BIT, ENABLE));
    else
        INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_DOZE_BIT), CSP_BITFVAL(CAN_DOZE_BIT, DISABLE));

    BSPSetCanPowerEnable(index,!boff);
    
    DEBUGMSG(ZONE_FUNCTION, (_T("CANPowerHandler-\r\n")));
    
    return TRUE;

}

//-----------------------------------------------------------------------------
//
// Function: CANClass::CANSetMode
//
// This method performs a software reset on CAN  MessageBox Mode setting 
// 
// 
//
// Parameters:
//  None.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------

void CANClass::CANSetMode(DWORD index,CAN_MODE mode)
{
    UNREFERENCED_PARAMETER(mode);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CANSetMode index:%d CAN_MODE:%d \r\n"),index,mode));
    
    if(index<32)
        SETREG32(&pCANreg->IMASK1,(1<<index));

    else
        SETREG32(&pCANreg->IMASK2,1<<(index-32));


    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_HALT_BIT), CSP_BITFVAL(CAN_HALT_BIT, DISABLE));

    return ;

}

//-----------------------------------------------------------------------------
//
// Function: CANClass::WritePacket
//
// This method performs a write operation with the data in one CAN_PACKET.
//
// Parameters:
//      pCANPkt
//          [in] Contains all the necessary information to transmit/
//          receive from the slave device.
//
//      bLast
//          [in] Signifies if this is the last packet to be transmitted.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
BOOL CANClass::WritePacket(PCAN_PACKET pCANPkt, BOOL bLast)
{

    DWORD cs=0;
    DWORD id=0;

    UNREFERENCED_PARAMETER(bLast);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("WritePacket\r\n")));

  //  EnterCriticalSection(&gcsCANBusLock);

    DEBUGMSG(ZONE_FUNCTION, (TEXT(" WritePacket  pCANPkt->ID:%x\r\n"), pCANPkt->ID));
  
    cs=pCANPkt->timestamp|(pCANPkt->wLen<<16)|(pCANPkt->frame<<20)|(pCANPkt->fromat<<21)|(0x1<<22)|(0x9<<24);
          
    // Buffer interrupt
    OUTREG32( &pCANreg->MB[pCANPkt->byIndex].CS , cs);
    
    // setting the ID
    if(pCANPkt->fromat==CAN_EXTENDED)
        id=(pCANPkt->PRIO<<29)|pCANPkt->ID&EXTENDEDMSK;
    else
        id=(pCANPkt->PRIO<<29)|pCANPkt->ID&STANDARDMSK;  
    
    OUTREG32( &pCANreg->MB[pCANPkt->byIndex].ID ,id);
    
    // setting the DATA

    OUTREG32( &pCANreg->MB[pCANPkt->byIndex].BYTE_0_3 , *(DWORD *)pCANPkt->pbyBuf);
    
    OUTREG32( &pCANreg->MB[pCANPkt->byIndex].BYTE_4_7 , *(DWORD *)(pCANPkt->pbyBuf+4) );
  
    cs=pCANPkt->timestamp|(pCANPkt->wLen<<16)|(pCANPkt->frame<<20)|(pCANPkt->fromat<<21)|(0x1<<22)|(0xc<<24);
    
    // Buffer interrupt
    OUTREG32( &pCANreg->MB[pCANPkt->byIndex].CS , cs);

                           
   if (WaitForSingleObject(hRevEvent[pCANPkt->byIndex], iIntrWaitTimeout) == WAIT_TIMEOUT)
    {
        
      (*(pCANPkt->lpiResult)) = CAN_ERR_TRANSFER_TIMEOUT;
      InterruptDone(dwSysIntr);
    //  LeaveCriticalSection(&gcsCANBusLock); 
      return FALSE;
    }
   
    (*(pCANPkt->lpiResult))=iResult;
    InterruptDone(dwSysIntr);
//    LeaveCriticalSection(&gcsCANBusLock); 
    return TRUE;

}


//-----------------------------------------------------------------------------
//
// Function: CANClass::ReadPacket
//
// This method performs a read operation with the data in one CAN_PACKET.
//
// Parameters:
//      pCANPkt
//          [in] Contains all the necessary information to transmit/
//          receive from the slave device.
//
//      bLast
//          [in] Signifies if this is the last packet to be transmitted.
//
//      bAddrCycleComplete
//          [in] Signifies that the slave address cycle was just completed.
//
// Returns:  
//      TRUE   Success.
//      FALSE  Arbitration lost or timeout error.
//
//-----------------------------------------------------------------------------
BOOL CANClass::ReadPacket(PCAN_PACKET pCANPkt, BOOL bLast)
{

    DWORD cs=0,id;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ReadPacket\r\n")));

    UNREFERENCED_PARAMETER(bLast);
    CeSetThreadPriority(GetCurrentThread(), 1);
       
  //  cs=pCANPkt->timestamp|(pCANPkt->wLen<<16)|(pCANPkt->frame<<20)|(pCANPkt->fromat<<21)|(0x1<<22)|(0x9<<24);

        // Buffer interrupt
  //  OUTREG32( &pCANreg->MB[pCANPkt->byIndex].CS , cs);
    
    // setting the ID
    if(pCANPkt->fromat==CAN_EXTENDED)
        id=(pCANPkt->PRIO<<29)|pCANPkt->ID&EXTENDEDMSK;
    else
        id=(pCANPkt->PRIO<<29)|pCANPkt->ID&STANDARDMSK;  
    
    OUTREG32( &pCANreg->MB[pCANPkt->byIndex].ID ,id);
    // setting the DATA
   
    cs=pCANPkt->timestamp|(pCANPkt->wLen<<16)|(pCANPkt->frame<<20)|(pCANPkt->fromat<<21)|(0x4<<24);
    
    // Buffer interrupt
    OUTREG32( &pCANreg->MB[pCANPkt->byIndex].CS , cs);
                        
    if (WaitForSingleObject(hRevEvent[pCANPkt->byIndex], iIntrWaitTimeout) == WAIT_TIMEOUT)
    {
      
      (*(pCANPkt->lpiResult)) = CAN_ERR_TRANSFER_TIMEOUT;
      InterruptDone(dwSysIntr);
      return FALSE;
    }
    
    (*(pCANPkt->lpiResult))=iResult;   

    *(DWORD *)pCANPkt->pbyBuf=  INREG32(&pCANreg->MB[index].BYTE_0_3);
    *(DWORD *)(pCANPkt->pbyBuf+4)=INREG32(&pCANreg->MB[index].BYTE_4_7);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CAN_Read ID:0x:%x \r\n"),INREG32(&pCANreg->MB[index].ID)));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CAN_Read IMASK1:0x:%x \r\n"),INREG32(&pCANreg->IMASK1)));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CAN_Read IMASK2:0x:%x \r\n"),INREG32(&pCANreg->IMASK2)));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("ReadPacket Index :%d gindex:%d  @ CS:0x:%x  ESR:0x:%x \r\n"),pCANPkt->byIndex, index,  INREG32(&pCANreg->MB[index].CS),INREG32(&pCANreg->ESR)));

    InterruptDone(dwSysIntr);

    return TRUE;

}

//-----------------------------------------------------------------------------
//
// Function: CANClass::ProcessPackets
//
// This is the main engine that transmits or receives data from CAN Bus
// Interface. This engine implements the complete CAN Bus Protocol which allows
// the calling process to interact with all CAN-compliant slave device. This
// method has built-in mechanism to prevent concurrent execution.
//
// Parameters:
//      pPacket 
//          [in] Contains all the necessary information to transmit/
//          receive from the slave device.
//
//      dwNumPackets
//          [in] Number of packets to be processed.
//
// Returns:  
//      TRUE   Success.
//      FALSE  Failed due to any one of the following conditions:
//               - failed to enable the CAN clock
//               - GenerateStart() call returned an error
//               - timed out waiting for interrupt notification
//               - arbitration lost
//               - transfer operation timed out
//
//-----------------------------------------------------------------------------
BOOL CANClass::ProcessPackets(CAN_PACKET packets[], INT32 numPackets)
{
    int i;
      
    for ( i = 0; i < numPackets; i++)
        {
         
             (*(packets->lpiResult)) = CAN_NO_ERROR;
                // Initialize Tx/Rx buffer navigator
                DEBUGMSG(ZONE_FUNCTION, (TEXT("CANClass::ProcessPacket(): Setting up Tx/Rx Buffer Navigator \r\n")));

                // CAN Transmitting?
                if (packets[i].byRW == CAN_RW_WRITE)
                {
                    if (!WritePacket(&packets[i], (i + 1 == numPackets)))
                    {
                    
                        goto __exit;
                    }
              
                }
                // CAN Receiving?
                else
                {
                    if (!ReadPacket(&packets[i], (i + 1 == numPackets)))
                    {
                      
                        goto __exit;
                    }
                }
             
            }

        return TRUE;
      
__exit:

    DEBUGMSG(1, (TEXT("CAN::ProcessPackets Fail\r\n")));
    return FALSE;


}



//------------------------------------------------------------------------------
//
// Function: CANIST
//
// This function is the CAN IST thread.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void WINAPI CANClass::CANIST(LPVOID lpParameter)
{
    CANClass *pCAN = (CANClass *)lpParameter;

    pCAN->CANInterruptHandler(INFINITE);

    // Should only ever get here with bTerminateISTThread == TRUE. Simply
    // returning here will gracefully terminate the IST handler thread.
    return;
}


//-----------------------------------------------------------------------------
//
// Function: CANInterruptHandler
//
// This function is the interrupt handler for the CAN.
// It waits for an CAN interrupt, and signals
// the event to the driver.  This additional layer of 
// event signalling is required to prevent priority inversion bugs.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void CANClass::CANInterruptHandler(UINT32 timeout)
{
   
    BOOL retVal = TRUE;
    DWORD  index1=0,index2=0;
    DWORD canecr,canesr;
    BOOL bCSReleased = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("CAN_IntrThread Enter \r\n")));

    // Remove-W4: Warning C4100 workaround
    while(retVal)
    {
       
        if (WaitForSingleObject(hInterrupted, timeout) == WAIT_OBJECT_0)
        {
            EnterCriticalSection(&gcsCANBusLock);
           
            canesr=INREG32(&pCANreg->ESR);
            canecr=INREG32(&pCANreg->ECR);

            DEBUGMSG(ZONE_FUNCTION, (TEXT("CANInterruptHandler ESR:0x:%x \r\n"),canesr));
            DEBUGMSG(ZONE_FUNCTION, (TEXT("CANInterruptHandler ECR:0x:%x \r\n"),canecr));
            
            index2=GetIndex(INREG32(&pCANreg->IFLAG2));
            index1=GetIndex(INREG32(&pCANreg->IFLAG1));
   
            iResult=CAN_NO_ERROR;
            if(index1!=0)
                index=index1;
            else if(index2!=0)
                index=index2+32;
            else
                index=0;

            index--;
            
            if(index!=-1)    
            {       

                if(index<32)
                {
                    SETREG32(&pCANreg->IFLAG1,index);
                  
                }
                else
                {
                    SETREG32(&pCANreg->IFLAG2,((index-32)));             
                }
                LeaveCriticalSection(&gcsCANBusLock); 
                bCSReleased = TRUE;
                SetEvent(hRevEvent[index]);
            }
            else
                {
            
                  if ((CSP_BITFEXT(canesr, CAN_WAK_INT_BIT)==SPECINTOCCURED))
                     {                       
                          DEBUGMSG(ZONE_FUNCTION, (TEXT("CANClass::CANInterruptHandler: CAN_WAK_INT_BIT intr Occor\r\n")));
                          INSREG32(&pCANreg->ESR, CSP_BITFMASK(CAN_WAK_INT_BIT),CSP_BITFVAL(CAN_WAK_INT_BIT, ENABLE));  
                          goto FinishedHanlde;
                    }
                   if ((CSP_BITFEXT(canesr, CAN_BOFF_INT_BIT)==SPECINTOCCURED))
                     {
                          DEBUGMSG(ZONE_FUNCTION, (TEXT("CANClass::CANInterruptHandler: CAN_BOFF_INT_BIT intr Occor\r\n")));
                          INSREG32(&pCANreg->ESR, CSP_BITFMASK(CAN_BOFF_INT_BIT),CSP_BITFVAL(CAN_BOFF_INT_BIT, ENABLE)); 
                          goto FinishedHanlde;
                    }
                    if ((CSP_BITFEXT(canesr, CAN_TWRN_INT_BIT)==SPECINTOCCURED))
                     {
                          DEBUGMSG(ZONE_FUNCTION, (TEXT("CANClass::CANInterruptHandler: CAN_TWRN_INT_BIT intr Occor\r\n")));
                          INSREG32(&pCANreg->ESR, CSP_BITFMASK(CAN_TWRN_INT_BIT),CSP_BITFVAL(CAN_TWRN_INT_BIT, ENABLE)); 
                          goto FinishedHanlde;
                    }
                     if ((CSP_BITFEXT(canesr, CAN_RWRN_INT_BIT)==SPECINTOCCURED))
                     {
                          DEBUGMSG(ZONE_FUNCTION, (TEXT("CANClass::CANInterruptHandler:  CAN_RWRN_INT_BIT intr Occor\r\n")));
                          INSREG32(&pCANreg->ESR, CSP_BITFMASK(CAN_RWRN_INT_BIT),CSP_BITFVAL(CAN_RWRN_INT_BIT, ENABLE)); 
                          goto FinishedHanlde;
                    }
             

                    if ((CSP_BITFEXT(canesr, CAN_ERR_INT_BIT)==SPECINTOCCURED) )
                        {                         
                               if ((CSP_BITFEXT(canesr, CAN_BIT_LSH1_ERR_BIT)==ERROCCURED))                            
                                {

                                    DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler: CAN_BIT_LSH1_ERR_BIT\r\n")));
                                    iResult=CAN_ERR_BIT0;
                                    goto FinishedHanlde;
                                }
                               if ((CSP_BITFEXT(canesr, CAN_BIT_LSH0_ERR_BIT)==ERROCCURED))                            
                                {

                                      DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler:CAN_BIT_LSH0_ERR_BIT \r\n")));
                                      iResult=CAN_ERR_BIT0;
                                      goto FinishedHanlde;
                                }
                               if ((CSP_BITFEXT(canesr, CAN_ACK_ERR_BIT)==ERROCCURED))                            
                                {

                                      DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler: CAN_ACK_ERR_BIT\r\n")));
                                      iResult=CAN_ERR_ACK;
                                      goto FinishedHanlde;
                                }
                               if ((CSP_BITFEXT(canesr, CAN_CRC_ERR_BIT)==ERROCCURED))                            
                                {

                                      DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler: CAN_CRC_ERR_BIT \r\n")));
                                      iResult=CAN_ERR_CRC;
                                      goto FinishedHanlde;
                                }
                                if ((CSP_BITFEXT(canesr, CAN_FRM_ERR_BIT)==ERROCCURED))                            
                                {

                                      DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler: CAN_FRM_ERR_BIT \r\n")));
                                      iResult=CAN_ERR_FRM;
                                      goto FinishedHanlde;
                                } 
                                if ((CSP_BITFEXT(canesr, CAN_STF_ERR_BIT)==ERROCCURED))                            
                                {

                                      DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler: CAN_STF_ERR_BIT \r\n")));
                                      iResult=CAN_ERR_STF;
                                      goto FinishedHanlde;
                                }
                                INSREG32(&pCANreg->ESR, CSP_BITFMASK(CAN_ERR_INT_BIT),CSP_BITFVAL(CAN_ERR_INT_BIT, ENABLE));        

                            } 
                        
                       
                    }
        
          
FinishedHanlde:
         InterruptDone(dwSysIntr);
         if (bCSReleased == FALSE)
         {
            LeaveCriticalSection(&gcsCANBusLock); 
         }

         }


    }

    return  ; 
} 

   

