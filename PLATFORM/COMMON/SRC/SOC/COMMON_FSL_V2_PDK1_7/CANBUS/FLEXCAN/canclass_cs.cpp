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

	//
	// CS&ZHL July-26 2011: move BSP Set functions to open 
	//
	can_index = index;
    //BSPCANConfigureGPIO( index);
    //BSPCANClockConfig(index,TRUE );   
    //BSPSetCanPowerEnable(index,TRUE);
 
	for( i=0; i<8; i++ )
		pTxRing[i] = new RingBuffer( TXBUF_SIZE );     
	pRxRing = new RingBuffer( RXBUF_SIZE );     

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

	if(!InitializeHardware())
	{
		RETAILMSG(1, (TEXT("CANClass::InitializeHardware() failed!\r\n")));
	}

	InitializeCriticalSection(&gcsCANBusLock);
	bHwTransmitting = FALSE;
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

	for( i=0; i<8; i++ )
	{
		delete pTxRing;
	}
	delete pRxRing;

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

    return;
}

BOOL	CANClass::InitializeHardware()
{
	BOOL		bRet = TRUE;
	DWORD		i;

    RETAILMSG(1, (TEXT("->CANClass::InitializeHardware\r\n")));

	// Setting the clock source = external crystal => 24MHz
	//             default baud rate = 250Kbps
	OUTREG32(&pCANreg->CTRL, CAN_CTRL_BIT_TIMING_250KHZ);

    // Enable all 64 MBs
    OUTREG8(&pCANreg->MCR, (CAN_NUMBER_OF_BUFFERS - 1));		// -> 0x3f
    
    // Enabling this flexCAN module 
    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_MDIS_BIT), CSP_BITFVAL(CAN_MDIS_BIT, DISABLE));

	// Freeze enable
    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_FRZ_BIT),CSP_BITFVAL(CAN_FRZ_BIT, ENABLE));

	// ABORT enable
    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_AEN_BIT),CSP_BITFVAL(CAN_AEN_BIT, ENABLE));

	// FIFO disable
    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_FEN_BIT),CSP_BITFVAL(CAN_FEN_BIT, DISABLE));  

	// setup Message Buffers
    for(i = 0; i < 8; i++)				// MB[0] - MB[7] are used as RxBuffers for specified purposes
    { 
         // inactivating the MBs
        OUTREG32( &pCANreg->MB[i].CS, CAN_MB_CS_CODE_RX_INACTIVE);
        // setting the ID
        OUTREG32( &pCANreg->MB[i].ID, 0x0 );
        // setting the DATA
        OUTREG32( &pCANreg->MB[i].BYTE_0_3, 0x0 );
        OUTREG32( &pCANreg->MB[i].BYTE_4_7, 0x0 );
    }

    for( ; i < 16; i++)					// MB[8] - MB[15] are used as TxBuffers
    { 
         // inactivating the MBs
        OUTREG32( &pCANreg->MB[i].CS, CAN_MB_CS_CODE_TX_INACTIVE);
        // setting the ID
        OUTREG32( &pCANreg->MB[i].ID, 0x0 );
        // setting the DATA
        OUTREG32( &pCANreg->MB[i].BYTE_0_3, 0x0 );
        OUTREG32( &pCANreg->MB[i].BYTE_4_7, 0x0 );
    }

	for( ; i < CAN_NUMBER_OF_BUFFERS; i++)		// MB[16] - MB[63] are used as RxBuffers for general purposes
    { 
         // inactivating the MBs
        OUTREG32( &pCANreg->MB[i].CS  , CAN_MB_CS_CODE_RX_INACTIVE);
        // setting the ID
        OUTREG32( &pCANreg->MB[i].ID   , 0x0 );
        // setting the DATA
        OUTREG32( &pCANreg->MB[i].BYTE_0_3  , 0x0 );
        OUTREG32( &pCANreg->MB[i].BYTE_4_7 , 0x0 );
    }
 
    OUTREG32(&pCANreg->ESR, 0);
    OUTREG32(&pCANreg->ECR, 0);

	// disable all RX/TX interrupts
    OUTREG32(&pCANreg->IMASK1, 0);		
    OUTREG32(&pCANreg->IMASK2, 0);
	
    RETAILMSG(1, (TEXT("<-CANClass::InitializeHardware\r\n")));
	return bRet;
}

//-----------------------------------------------------------------------------
//
// Function: CANClass::EnterFreezeMode
//
// Enter Freeze mode to setup parameters
// 
// Parameters:
//  None.
//
// Returns:  
//     TRUE: the flexCAN is in freeze mode.
//		FALSE: the flexCAN is NOT in freeze mode, still in normal mode	
//-----------------------------------------------------------------------------
BOOL CANClass::EnterFreezeMode()
{
	// set HALT bit to enter freeze mode
    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_HALT_BIT), CSP_BITFVAL(CAN_HALT_BIT, ENABLE));
	// wait the module's acknowledgement
	while(!(INREG32(&pCANreg->MCR) & CSP_BITFMASK(CAN_FRZ_ACK_BIT)));

	return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CANClass::EnterNormalMode
//
// Enter Normal mode to do data transfer
// 
// Parameters:
//  None.
//
// Returns:  
//     TRUE: the flexCAN is in normal mode.
//		FALSE: the flexCAN is NOT in normal mode, still in freeze mode?	
//-----------------------------------------------------------------------------
BOOL CANClass::EnterNormalMode()
{
	// clear HALT bit to exit freeze mode
    INSREG32(&pCANreg->MCR, CSP_BITFMASK(CAN_HALT_BIT), CSP_BITFVAL(CAN_HALT_BIT, DISABLE));
	// wait until the module leave freeze mode
	while(INREG32(&pCANreg->MCR) & CSP_BITFMASK(CAN_FRZ_ACK_BIT));

	return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CANClass::EnableCANControllerInterrupt
//
// Enable to RX/TX can data frame
// 
// Parameters:
//  None.
//
// Returns:  
//  None.
//-----------------------------------------------------------------------------
void CANClass::EnableCANControllerInterrupt()
{
	//enable all MB interrupts    
    OUTREG32(&pCANreg->IMASK1,	0xFFFFFFFF);
    OUTREG32(&pCANreg->IMASK2,	0xFFFFFFFF);

	//enable bus-off interrupt
    INSREG32(&pCANreg->CTRL, CSP_BITFMASK(CAN_BOFF_MSK_BIT), CSP_BITFVAL(CAN_BOFF_MSK_BIT, ENABLE));

	//enable error interrupt
    INSREG32(&pCANreg->CTRL, CSP_BITFMASK(CAN_ERR_MSK_BIT), CSP_BITFVAL(CAN_ERR_MSK_BIT, ENABLE));

	//enable Tx warning interrupt
    INSREG32(&pCANreg->CTRL, CSP_BITFMASK(CAN_TWRN_MSK_BIT), CSP_BITFVAL(CAN_TWRN_MSK_BIT, ENABLE));

	//enable Rx warning interrupt
    INSREG32(&pCANreg->CTRL, CSP_BITFMASK(CAN_RWRN_MSK_BIT), CSP_BITFVAL(CAN_RWRN_MSK_BIT, ENABLE));
}


//-----------------------------------------------------------------------------
//
// Function: CANClass::EnableCANControllerInterrupt
//
// Disable to RX/TX can data frame
// 
// Parameters:
//  None.
//
// Returns:  
//  None.
//-----------------------------------------------------------------------------
void CANClass::DisableCANControllerInterrupt()
{
	//disable all MB interrupts    
    OUTREG32(&pCANreg->IMASK1,	0);
    OUTREG32(&pCANreg->IMASK2,	0);

	//disable bus-off interrupt
    INSREG32(&pCANreg->CTRL, CSP_BITFMASK(CAN_BOFF_MSK_BIT), CSP_BITFVAL(CAN_BOFF_MSK_BIT, DISABLE));

	//disable error interrupt
    INSREG32(&pCANreg->CTRL, CSP_BITFMASK(CAN_ERR_MSK_BIT), CSP_BITFVAL(CAN_ERR_MSK_BIT, DISABLE));

	//disable Tx warning interrupt
    INSREG32(&pCANreg->CTRL, CSP_BITFMASK(CAN_TWRN_MSK_BIT), CSP_BITFVAL(CAN_TWRN_MSK_BIT, DISABLE));

	//disable Rx warning interrupt
    INSREG32(&pCANreg->CTRL, CSP_BITFMASK(CAN_RWRN_MSK_BIT), CSP_BITFVAL(CAN_RWRN_MSK_BIT, DISABLE));
}


BOOL CANClass::AbortPendingTX(DWORD dwTxMBIndex)
{
	DWORD		dwCS;

	dwCS = INREG32(&(pCANreg->MB[dwTxMBIndex].CS));
	if((dwCS & CAN_MB_CS_CODE_MASK) == CAN_MB_CS_CODE_TX_SEND)
	{
		// issue abort command
		dwCS = (dwCS & ~CAN_MB_CS_CODE_MASK) |CAN_MB_CS_CODE_TX_ABORT;
		OUTREG32( &(pCANreg->MB[dwTxMBIndex].CS), dwCS);

		// check both of CS and interrupt flag of the MB
		// ......
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CANClass::TransferMessageDirectly
//
// start to send a can frame out
// 
// Parameters:
//  pMB: a pointer to MessageBuffer need to be sent
//
// Returns:  
//     TRUE: the data pointed by pMB is sent out.
//		FALSE: something wrong	
//-----------------------------------------------------------------------------
BOOL CANClass::TransferMessageDirectly(PMB_REG pMB)
{
	BOOL		bRet = TRUE;
	DWORD		dwTxMBIndex;
	DWORD		dwID = pMB->ID;
	DWORD		dwCS;

	// get MB index from TX priority
	dwTxMBIndex = ((dwID & 0xE0000000) >> 29) + 8;

	// abort the pending transmission if existed
	if(!AbortPendingTX(dwTxMBIndex))
	{
		DEBUGMSG(ZONE_FUNCTION, (TEXT("CANClass::TransferMessageDirectly:AbortPendingTX failed\r\n")));
		bRet = FALSE;
		goto exit1;
	}

	dwCS = (pMB->CS & ~CAN_MB_CS_CODE_MASK) | CAN_MB_CS_CODE_TX_SEND;

    // setting the ID
    OUTREG32( &(pCANreg->MB[dwTxMBIndex].ID), dwID);
    
    // always fill 8 bytes
    OUTREG32( &(pCANreg->MB[dwTxMBIndex].BYTE_0_3), pMB->BYTE_0_3);
    OUTREG32( &(pCANreg->MB[dwTxMBIndex].BYTE_4_7), pMB->BYTE_4_7);
   
    // set controls command
    OUTREG32( &(pCANreg->MB[dwTxMBIndex].CS), dwCS);

exit1:
	return bRet;
}


BOOL CANClass::Start( );
BOOL CANClass::Stop( );
BOOL CANClass::Reset( ) ;// Only reset CAN, does not affect the class fields

BOOL CANClass::SetBaudRate( DWORD dwRate );
BOOL CANClass::SetFilter( PCAN_FILTER pFilter );
BOOL CANClass::EnableSync( PCAN_SYNC pSync )
{
	return TRUE;
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
BOOL CANClass::WritePacket(PCAN_PACKET pCANPkt )
{
    struct	MB_SEG	mb;
	BOOL					bRet;

	DEBUGMSG(ZONE_FUNCTION, (TEXT("WritePacket\r\n")));
	//  EnterCriticalSection(&gcsCANBusLock);
 
	// packet struct mb_reg
	memset( &mb, 0, sizeof(MB_SEG) );
	mb.cs = (0x08<<24)|(pCANPkt->dwType<<21)|(pCANPkt->dwRTR<<20)|(pCANPkt->dwDatLen<<16);
    // setting the ID
    DEBUGMSG(ZONE_FUNCTION, (TEXT(" WritePacket  pCANPkt->ID:%x\r\n"), pCANPkt->dwID));
    if(pCANPkt->dwType==1 )
		mb.id=(pCANPkt->dwPrio<<29)|(pCANPkt->dwID&EXTENDEDMSK);
    else
        mb.id=(pCANPkt->dwPrio<<29)|((pCANPkt->dwID<<18)&STANDARDMSK);  
    // setting the DATA
	memcpy( mb.BYTE_0_3, pCANPkt->ucDat, sizeof(UINT32) );
	memcpy( mb.BYTE_4_7, &pCANPkt->ucDat[4], sizeof(UINT32) );

	// put current data package into ring buffer
	if( !pTxRing[pCANPkt->dwPrio]->Put( &mb) )
	{
		// is full
		return FALSE;
	}

	//then start a new transmitting if FLEXCAN is NOT in transmitting state
	if(!HwTransmitting)
	{
		//get data from tx ring buffer
		if( !pTxRing[pCANPkt->dwPrio]->Get( &mb ) )
		{
			return FALSE;
		}
		//set hardware transmitting first!
		bHwTransmitting = TRUE;

		//then start to send
		result = TransferMessageDirectly( &mb );
		if( result )						//transmit failed
		{
			bHwTransmitting = FALSE;
			return FALSE;				//return error code
		}
	}
	else
	{
		// sja1000 is transmitting, so just fill data into tx ring buffer.
		//up(&devp->txbuf_sem);
	}
	
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
    CANClass	*pCAN = (CANClass *)lpParameter;
	DWORD		dwWaitReturn;

    while ( !pCAN->bTerminateISTThread ) 
	{
        dwWaitReturn = WaitForSingleObject( pCAN->hInterrupted, INFINITE);

		// IST Handler
		pCAN->CANInterruptHandler();

		//send EOI
		//InterruptDone(pCAN->dwSysIntr);
	}

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
void CANClass::CANInterruptHandler()
{
    BOOL		bRet = TRUE;
    DWORD		canecr,canesr;
	DWORD		dwIntFlags[2];
	DWORD		i = 0;
	DWORD		dwCurrentFlags;
	DWORD		dwThisMBFlag;
	BOOL		bNormalInterrupted = FALSE;
	BOOL		bMoreDataTransmitting = FALSE;
	BOOL		bDataReceived = FALSE;
	MB_REG	canframe;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("CAN_IntrThread Enter \r\n")));

	EnterCriticalSection(&gcsCANBusLock);

	//get CAN error state
	canesr = INREG32(&pCANreg->ESR);
	canecr = INREG32(&pCANreg->ECR);
	DEBUGMSG(ZONE_FUNCTION, (TEXT("CANInterruptHandler ESR:0x:%x \r\n"),canesr));
	DEBUGMSG(ZONE_FUNCTION, (TEXT("CANInterruptHandler ECR:0x:%x \r\n"),canecr));

	//get interrupt flags
	dwIntFlags[0] = GetIndex(INREG32(&pCANreg->IFLAG1));
	dwIntFlags[1] = GetIndex(INREG32(&pCANreg->IFLAG2));
	iResult = CAN_NO_ERROR;

	// interrupt processing
	bNormalInterrupted = FALSE;
	bMoreDataTransmitting = FALSE;
	bDataReceived = FALSE;
	for(i = 0; i < 64; i++)
	{
		dwCurrentFlags = dwIntFlags[i / 32];
		dwThisMBFlag = (1 << (i % 32));
		if(!(dwCurrentFlags & dwThisMBFlag))
		{
			continue;		// no interrupt for current MB, check next MB
		}

		bNormalInterrupted = TRUE;
		//clear IFLAGS
		if(i < 32)
			SETREG32(&pCANreg->IFLAG1, dwThisMBFlag);
		else
			SETREG32(&pCANreg->IFLAG2, dwThisMBFlag);

		switch(i)
		{
		case 0:
			// a SYNC frame received
			break;

		case 1:		// some broadcasting packet received
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			break;

		case 8:			// a CAN frame transmitting completed
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:		
			// try to get a waiting MB from ring buffer
			if(!pTxRing[i - 8]->Get(&canframe))
			{
				//there are still data need to send
				TransferMessageDirectly(&canframe);
				bMoreDataTransmitting = TRUE;
			}
			break;

		default:		// -> MB16 - MB63
			// a can frame received, save it to RX ring buffer
			memcpy(&canframe, &pCANreg->MB[i], sizeof(MB_REG));
			pRxRing->Put(&canframe);
			bDataReceived = TRUE;
		}
	}

	if(bDataReceived)
	{
		//send receive event.
        SetEvent(m_hRevEvent);
	}

	if(!bMoreDataTransmitting)
	{
		//all tx ring buffers are empty, so clear hardware transmitting flag
		bHwTransmitting = FALSE;
	}

	if(!bNormalInterrupted)		// no normal transfer interrupt, check specified interrupts or errors
	{
		// specified interrupt processing -> clear interrupt flag
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

		// error processing
        if ((CSP_BITFEXT(canesr, CAN_ERR_INT_BIT) == SPECINTOCCURED))
		{                         
			// clear error interrupt flag
			INSREG32(&pCANreg->ESR, CSP_BITFMASK(CAN_ERR_INT_BIT),CSP_BITFVAL(CAN_ERR_INT_BIT, ENABLE));        

			// send error event if required
			SetEvent(m_hErrEvent);

			// save error cause
			if ((CSP_BITFEXT(canesr, CAN_BIT_LSH1_ERR_BIT)==ERROCCURED))                            
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler: CAN_BIT_LSH1_ERR_BIT\r\n")));
				iResult = CAN_ERR_BIT0;
				goto FinishedHanlde;
			}

			if ((CSP_BITFEXT(canesr, CAN_BIT_LSH0_ERR_BIT)==ERROCCURED))                            
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler:CAN_BIT_LSH0_ERR_BIT \r\n")));
				iResult = CAN_ERR_BIT1;
				goto FinishedHanlde;
			}

			if ((CSP_BITFEXT(canesr, CAN_ACK_ERR_BIT)==ERROCCURED))                            
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler: CAN_ACK_ERR_BIT\r\n")));
				iResult = CAN_ERR_ACK;
				goto FinishedHanlde;
			}

			if ((CSP_BITFEXT(canesr, CAN_CRC_ERR_BIT)==ERROCCURED))                            
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler: CAN_CRC_ERR_BIT \r\n")));
				iResult = CAN_ERR_CRC;
				goto FinishedHanlde;
			}

			if ((CSP_BITFEXT(canesr, CAN_FRM_ERR_BIT)==ERROCCURED))                            
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler: CAN_FRM_ERR_BIT \r\n")));
				iResult = CAN_ERR_FRM;
				goto FinishedHanlde;
			} 

			if ((CSP_BITFEXT(canesr, CAN_STF_ERR_BIT)==ERROCCURED))                            
			{
				DEBUGMSG(ZONE_ERROR, (TEXT("CANClass::CANInterruptHandler: CAN_STF_ERR_BIT \r\n")));
				iResult = CAN_ERR_STF;
				goto FinishedHanlde;
			}
		} 
	}

FinishedHanlde:
	InterruptDone(dwSysIntr);
	LeaveCriticalSection(&gcsCANBusLock); 
} 

   

