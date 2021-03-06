//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//------------------------------------------------------------------------------
/*---------------------------------------------------------------------------
* Copyright (C) 2004-2008, Emtronix Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//
//  File:  cspiClass.cpp
//
//  Provides the implementation of the CSPI bus driver to support CSPI
//  transactions from multiple client drivers.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4127 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "SPIClass.h"
#include "ETA108Class.H"

//-----------------------------------------------------------------------------
// External Functions
extern "C" UINT32 BSPCSPICalculateDivRate(UINT32 dwFrequency, UINT32 dwTolerance);
extern "C" BOOL BSPCSPISetIOMux(UINT32 Index);
extern "C" BOOL BSPCSPIEnableClock(UINT32 Index, BOOL bEnable);
extern "C" BOOL BSPCSPIReleaseIOMux(UINT32 dwIndex);
extern "C" DWORD CspCSPIGetBaseRegAddr(UINT32 index);
extern "C" DWORD CspCSPIGetIRQ(UINT32 index);
extern "C" BOOL BSPCheckPort(UINT32 Index);


extern "C" BOOL BSPCspiGetChannelPriority(UINT8 (*priority)[2]);
extern "C" DDK_DMA_REQ CspCSPIGetDmaReqTx(UINT32 index);
extern "C" DDK_DMA_REQ CspCSPIGetDmaReqRx(UINT32 index);
extern "C" BOOL BSPCspiIsDMAEnabled(UINT8 Index);
extern "C" BOOL BSPCspiAcquireGprBit(UINT8 Index);
extern "C" BOOL BSPCspiIsAllowPolling(UINT8 Index);
extern "C" BOOL BSPCspiExchange(VOID *lpInBuf, LPVOID pRxBuf, UINT32 nOutBufSize, LPDWORD BytesReturned);

extern "C" void BSPCSPIRDY2IO( UINT32 dwIndex );
extern "C" void BSPCSPICS2IO( UINT32 dwIndex  );
extern "C" void BSPCSPIRDYSet( UINT32 dwIndex, BOOL bVal );
extern "C" void BSPCSPICSSet( UINT32 dwIndex, BOOL bVal);

extern "C" void BSPETA108Reset( UINT32 dwTimes );


//
//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

#define  CSPI_DMA_WATERMARK_RX		16
#define  CSPI_DMA_WATERMARK_TX		16
#define  CSPI_DMA_COUNT				2
#define	 MULT_DMABUFFER_SIZE		4

#define  STOP_SAMPLING				3
#define  TXDMA_IS_STOP				1
#define  RXDMA_IS_STOP				2


#define  MIN_VAL(a, b)   (((a) > (b)) ? (b) : (a))

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Functions
void CallCspiProcessThread( spiClass *pSpiClass )
{
	//UNREFERENCED_PARAMETER( pSpiClass);
	pSpiClass->CspiProcess( );
}
//-----------------------------------------------------------------------------

// Local Variables

//-----------------------------------------------------------------------------
// Local Functions
spiClass::spiClass()
{
	m_pCSPI = NULL;
	m_hHeap = NULL;
	m_hIntrEvent = NULL;
	m_hTransferDoneSemaphore = NULL;
	
	m_pSPITxBuf = NULL;
	m_pSPIRxBuf = NULL;

	m_hThread = NULL;

	m_nStopSampling = 0;

}

spiClass::~spiClass()
{

}

BOOL spiClass::CspiInitialize(DWORD Index)
{
	PHYSICAL_ADDRESS phyAddr;
	UINT32 irq[5];

	m_Index = Index;
	// create global heap for internal queues/buffers
	//      flOptions = 0 => no options
	//      dwInitialSize = 0 => zero bytes initial size
	//      dwMaximumSize = 0 => heap size limited only by available memory
	DEBUGMSG(1, (TEXT("CspiInitialize: m_hHeap=0x%x\r\n"),m_hHeap));
	m_hHeap = HeapCreate(0, 0, 0);

	// check if HeapCreate failed
	if (m_hHeap == NULL)
	{
		DEBUGMSG(1, (TEXT("CspiInitialize:  HeapCreate failed!\r\n")));
		goto Init_Error;
	}

	// create event for CSPI interrupt signaling, SPI RX DMA interrupt and TX DMA interrupt.
	//      pEventAttributes = NULL (must be NULL)
	//      bManualReset = FALSE => resets automatically to nonsignaled
	//                              state after waiting thread released
	//      bInitialState = FALSE => initial state is non-signaled
	//      lpName = NULL => object created without a name
	m_hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hTransferDoneSemaphore = CreateSemaphore( NULL, 0, MULT_DMABUFFER_SIZE, NULL );

	// check if CreateEvent failed
	if (m_hIntrEvent == NULL || m_hTransferDoneSemaphore == NULL)
	{
		DEBUGMSG(1, (TEXT("CspiInitialize:  CreateEvent failed!\r\n")));
		goto Init_Error;
	}

	// Get the Base Address according to Index
	phyAddr.QuadPart = CspCSPIGetBaseRegAddr(Index);
	if( !phyAddr.QuadPart )
	{
		DEBUGMSG(1, (TEXT("CspiInitialize:  invalid CSPI instance!\r\n")));
		goto Init_Error;
	}
	//RETAILMSG(1, (TEXT("CspiInitialize:CSPI Base: 0x%x\r\n"), phyAddr.QuadPart) );

	// Map peripheral physical address to virtual address
	m_pCSPI = (PCSP_CSPI_REG) MmMapIoSpace(phyAddr, sizeof(CSP_CSPI_REG),FALSE);

	// Check if virtual mapping failed
	if (m_pCSPI == NULL)
	{
		DEBUGMSG(1, (TEXT("CspiInitialize:  MmMapIoSpace failed!\r\n")));
		goto Init_Error;
	}

	//initialize DMA
	if( !InitCspiDMA(Index))
	{
		goto Init_Error;
	}

	irq[0] = (UINT32)-1;
	irq[1] = 0;
	irq[2] = GetSdmaChannelIRQ( m_dmaChanCspiTx );
	irq[3] = GetSdmaChannelIRQ( m_dmaChanCspiRx );
	//Get the IRQ according to Index
	irq[4] = CspCSPIGetIRQ(Index);

	// Call the OAL to translate the IRQ into a SysIntr value.
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, irq, sizeof(irq),
		&m_dwSysIntr, sizeof(m_dwSysIntr), NULL))
	{
		RETAILMSG(1, (TEXT("ERROR: Failed to obtain sysintr value for CSPI interrupt.\r\n")));
		goto Init_Error;
	}

	// register CSPI interrupt
	if (!InterruptInitialize(m_dwSysIntr, m_hIntrEvent, NULL, 0))
	{
		DEBUGMSG(1, (TEXT("CspiInitialize:  InterruptInitialize failed!\r\n")));
		goto Init_Error;
	}

	// create CSPI Tx buffer critical section
	InitializeCriticalSection(&m_cspiTxBufCs);

	// create CSPI Rx buffer critical section
	InitializeCriticalSection(&m_cspiRxBufCs);

	DEBUGMSG(1, (TEXT("CspiInitialize: &m_pCSPI=0x%x\r\n"),&m_pCSPI));
	// disable CSPI to reset internal logic
	INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN),
		CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE));

	// create CSPI queue processing thread if it does not exist
	if (!m_hThread)
	{
		// set global termination flag
		m_bTerminate=FALSE;

		// create processing thread
		//      pThreadAttributes = NULL (must be NULL)
		//      dwStackSize = 0 => default stack size determined by linker
		//      lpStartAddress = CspiProcessQueue => thread entry point
		//      lpParameter = NULL => point to thread parameter
		//      dwCreationFlags = 0 => no flags
		//      lpThreadId = NULL => thread ID is not returned
		m_hThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL, 0, (LPTHREAD_START_ROUTINE)CallCspiProcessThread, this, 0, NULL);
		DEBUGMSG(1, (TEXT("CspiInitialize:  this=0x%x\r\n"),this));

		// check if CreateThread failed
		if (m_hThread == NULL)
		{
			DEBUGMSG(1, (TEXT("CspiInitialize:  CreateThread failed!\r\n")));
			goto Init_Error;
		}
		SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST );
	}

	//m_cspiOpenCount++;    

	// By default, Loopback is disabled.
	CspiEnableLoopback(FALSE);

	return TRUE;

Init_Error:
	CspiRelease();

	return FALSE;
}

void spiClass::CspiRelease()
{
	DEBUGMSG(1, (TEXT("CspiRelease +\r\n")));

	// kill the exchange packet processing thread
	if (m_hThread)
	{
		m_bTerminate=TRUE;
// 		// try to signal the thread so that it can wake up and terminate
 		if (m_hIntrEvent)
 		{
 			SetEvent(m_hIntrEvent);
			// close interrupt event handle
			CloseHandle(m_hIntrEvent);
 		}
	}
	while( m_hThread )
		Sleep( 100 );

	// Release IOMUX pins
	//BSPCSPIReleaseIOMux(m_Index);
	// Release SYSINTR
	KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_dwSysIntr, sizeof(DWORD), NULL, 0, NULL);
	// deregister the system interrupt
	if (m_dwSysIntr != SYSINTR_UNDEFINED)
	{
		InterruptDisable(m_dwSysIntr);
		m_dwSysIntr = (DWORD)SYSINTR_UNDEFINED;
	}
	
	if( m_hTransferDoneSemaphore!= NULL )
	{
		CloseHandle( m_hTransferDoneSemaphore );
		m_hTransferDoneSemaphore = NULL;
	}

	DeInitCspiDMA( );
	// free the virtual space allocated for CSPI memory map
	if (m_pCSPI != NULL)
	{
		MmUnmapIoSpace(m_pCSPI, sizeof(CSP_CSPI_REG));
		m_pCSPI = NULL;
	}

	// destroy the heap
	if (m_hHeap != NULL)
	{
		HeapDestroy(m_hHeap);
		m_hHeap = NULL;
	}

	// delete the Tx buffer critical section
	DeleteCriticalSection(&m_cspiTxBufCs);

	// delete the Rx buffer critical section
	DeleteCriticalSection(&m_cspiRxBufCs);

	DEBUGMSG(1, (TEXT("CspiRelease -\r\n")));
}

//-----------------------------------------------------------------------------
//
//  Function: GetSdmaChannelIRQ
//
//  This function returns the IRQ number of the specified SDMA channel.
//
//  Parameters:
//      chan
//          [in] The SDMA channel.
//
//  Returns:
//      IRQ number.
//
//-----------------------------------------------------------------------------
DWORD spiClass::GetSdmaChannelIRQ(UINT32 chan)
{
	return (IRQ_SDMA_CH0 + chan);
}

//------------------------------------------------------------------------------
//
// Function: DeInitDMA
//
//  Performs deintialization of DMA
//
// Parameters:
//         None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL spiClass::DeInitCspiDMA(void) 
{
 	if(!DeinitChannelDMA())
 	{
 		DEBUGMSG(1, (TEXT("Cspi:DeinitChannelDMA() - Failed to deinitialize DMA.\r\n")));
 		return FALSE;
 	}
 	if(!UnmapDMABuffers())
 	{
 		DEBUGMSG(1, (TEXT("Cspi:UnmapDMABuffers() - Failed to Unmap DMA buffers\r\n")));
 		return FALSE;
 	}
	return TRUE ; 
}
//----------------------------------------------------------------------------
//
// Function: CspIOMux
//
// This function implement IOMUX pins selected for the CSPI bus.
//
// Parameters: void
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//----------------------------------------------------------------------------
// BOOL spiClass::CspiIOMux( void )
// {
// 	return BSPCSPISetIOMux( m_Index );
// }

//-----------------------------------------------------------------------------
//
// Function: CspiBufRd8
//
// This function is used to access a buffer as an array of 8-bit (UINT8) 
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer, promoted to UINT32.
//
//-----------------------------------------------------------------------------
UINT32 spiClass::CspiBufRd8(LPVOID pBuf)
{
	UINT8 *p;

	p = (UINT8 *) pBuf;

	return *p;
}


//-----------------------------------------------------------------------------
//
// Function: CspiBufRd16
//
// This function is used to access a buffer as an array of 16-bit (UINT16) 
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer, promoted to UINT32.
//
//-----------------------------------------------------------------------------
UINT32 spiClass::CspiBufRd16(LPVOID pBuf)
{
	UINT16 *p;

	p = (UINT16 *) pBuf;

	return *p;
}


//-----------------------------------------------------------------------------
//
// Function: CspiBufRd32
//
// This function is used to access a buffer as an array of 32-bit (UINT32) 
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer.
//
//-----------------------------------------------------------------------------
UINT32 spiClass::CspiBufRd32(LPVOID pBuf)
{
	UINT32 *p;

	p = (UINT32 *) pBuf;

	return *p;
}


//-----------------------------------------------------------------------------
//
// Function: CspiBufWrt8
//
// This function is used to access a buffer as an array of 8-bit (UINT8) 
// values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written demoted to UINT8.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void spiClass::CspiBufWrt8(LPVOID pBuf, UINT32 data)
{
	UINT8 *p;

	p = (UINT8 *) pBuf;

	*p = (UINT8) data;
}


//-----------------------------------------------------------------------------
//
// Function: CspiBufWrt16
//
// This function is used to access a buffer as an array of 16-bit (UINT16) 
// values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written demoted to UINT16.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void spiClass::CspiBufWrt16(LPVOID pBuf, UINT32 data)
{
	UINT16 *p;

	p = (UINT16 *) pBuf;

	*p = (UINT16) data;
}


//-----------------------------------------------------------------------------
//
// Function: CspiBufWrt32
//
//      This function is used to access a buffer as an array of 32-bit (UINT32) 
//      values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void spiClass::CspiBufWrt32(LPVOID pBuf, UINT32 data)
{
	UINT32 *p;

	p = (UINT32 *) pBuf;

	*p = data;
}

//------------------------------------------------------------------------------
//
// Function: MapDMABuffers
//
// Allocate and map DMA buffer 
//
// Parameters:
//        None
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL spiClass::MapDMABuffers(void)
{
	DMA_ADAPTER_OBJECT Adapter;
	DEBUGMSG(1,(_T("spiClass::MapDMABuffers+\r\n")));

	m_pSpiVirtTxDMABufferAddr = NULL;
	m_pSpiVirtRxDmaBufferAddr = NULL;

	memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
	Adapter.InterfaceType = Internal;
	Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
	// Allocate a block of virtual memory (physically contiguous) for the RX DMA buffers.
	m_pSpiVirtRxDmaBufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, m_dwRxDMABufferSize*CSPI_DMA_COUNT
		, &(m_pSpiPhysRxDMABufferAddr), FALSE);

	if (m_pSpiVirtRxDmaBufferAddr == NULL)
	{
		DEBUGMSG(1, (TEXT("spiClass::MapDMABuffers() - Failed to allocate RX DMA buffer.\r\n")));
		return(FALSE);
	}

	memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
	Adapter.InterfaceType = Internal;
	Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

	// Allocate a block of virtual memory (physically contiguous) for the TX DMA buffers.
	m_pSpiVirtTxDMABufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, m_dwTxDMABufferSize*CSPI_DMA_COUNT
		, &(m_pSpiPhysTxDMABufferAddr), FALSE);

    // Check if DMA buffer has been allocated
	if (m_pSpiVirtTxDMABufferAddr == NULL)
	{
		DEBUGMSG(1, (TEXT("spiClass::MapDMABuffers() - Failed to allocate TX DMA buffer.\r\n")));
		return(FALSE);
	}

	DEBUGMSG(1,(_T("spiClass::MapDMABuffers-\r\n")));
	return(TRUE);
}

//------------------------------------------------------------------------------
//
// Function: UnmapDMABuffers
//
//  This function unmaps the DMA buffers previously mapped with the
//  MapDMABuffers function.
//
// Parameters:
//        None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL spiClass::UnmapDMABuffers(void)
{
  	DMA_ADAPTER_OBJECT Adapter;
  	DEBUGMSG(1,(_T("spiClass::UnmapDMABuffers+\r\n")));
  
 	memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
 	Adapter.InterfaceType = Internal;
 	Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
 
 	if(m_pSpiVirtTxDMABufferAddr)
 	{
 		// Logical address parameter is ignored
 		m_pSpiPhysTxDMABufferAddr.QuadPart = 0;
 		HalFreeCommonBuffer(&Adapter, 0, m_pSpiPhysTxDMABufferAddr, 
 			(PVOID)(m_pSpiVirtTxDMABufferAddr), FALSE);
 	}
 
 	memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
 	Adapter.InterfaceType = Internal;
 	Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
 
 	if(m_pSpiVirtRxDmaBufferAddr)
 	{
 		// Logical address parameter is ignored
 		m_pSpiPhysRxDMABufferAddr.QuadPart = 0;
 		HalFreeCommonBuffer(&Adapter, 0, m_pSpiPhysRxDMABufferAddr, 
 			(PVOID)(m_pSpiVirtRxDmaBufferAddr), FALSE);
 	}
 
 	m_pSpiVirtTxDMABufferAddr = NULL;
 	m_pSpiVirtRxDmaBufferAddr = NULL;
 	return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: DeinitChannelDMA
//
//  This function deinitializes the DMA channel for output.
//
// Parameters:
//        None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL spiClass::DeinitChannelDMA(void)
{    
	if (m_dmaChanCspiRx != 0)
 	{
 		DDKSdmaCloseChan(m_dmaChanCspiRx);
 		m_dmaChanCspiRx = 0;
 	}
 	if (m_dmaChanCspiTx != 0)
 	{
 		DDKSdmaCloseChan(m_dmaChanCspiTx);
 		m_dmaChanCspiTx = 0;
 	}
 	return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: InitChannelDMA
//
//  This function initializes the DMA channel for output.
//
// Parameters:
//  Index
//      CSPI device Index
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL spiClass::InitChannelDMA(UINT32 Index)
{
	UINT8 tmp, dmaChannelPriority[2];
	BOOL rc = FALSE;
	Index = Index;

	DEBUGMSG(1,(_T("spiClass::InitChannelDMA+\r\n")));

	BSPCspiGetChannelPriority(&dmaChannelPriority);
	if (dmaChannelPriority[0]==dmaChannelPriority[1])
	{
		DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA: Invalid DMA Channel Priority.\r\n")));
		goto cleanUp;
	}

	if (dmaChannelPriority[0]>dmaChannelPriority[1])
	{
		tmp = dmaChannelPriority[0];
		dmaChannelPriority[0] = dmaChannelPriority[1];
		dmaChannelPriority[1] = tmp;

		DEBUGMSG(ZONE_WARN, (_T("WARNING:InitChannelDMA: Reversed DMA Channel Priority.\r\n")));
	}

	// Open virtual DMA channels 
	m_dmaChanCspiRx = DDKSdmaOpenChan(m_dmaReqRx, dmaChannelPriority[1]); /// NULL, CspCSPIGetIRQ(Index));
	DEBUGMSG(1,(_T("Channel Allocated(Rx) : %d\r\n"),m_dmaChanCspiRx));
	if (!m_dmaChanCspiRx)
	{
		DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Rx): SdmaOpenChan for input failed.\r\n")));
		goto cleanUp;
	}

	// Allocate DMA chain buffer
	if (!DDKSdmaAllocChain(m_dmaChanCspiRx, CSPI_DMA_COUNT))
	{
		DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaAllocChain for input failed.\r\n")));
		goto cleanUp;
	}  

	// Open virtual DMA channels 
	
	m_dmaChanCspiTx = DDKSdmaOpenChan(m_dmaReqTx, dmaChannelPriority[0]); /// NULL, CspCSPIGetIRQ(Index));
	DEBUGMSG(ZONE_DMA,(_T("Channel Allocated(Tx) : %d\r\n"),m_dmaChanCspiTx));
	if (!m_dmaChanCspiTx)
	{
		DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Tx): SdmaOpenChan(Rx) for input failed.\r\n")));
		goto cleanUp;
	}

	// Allocate DMA chain buffer
	if (!DDKSdmaAllocChain(m_dmaChanCspiTx, CSPI_DMA_COUNT))
	{
		DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Tx): DDKSdmaAllocChain for input failed.\r\n")));
		goto cleanUp;
	}  

	RETAILMSG( 1, (TEXT("InitChannelDMA: m_dmaChanCspiTx:%d m_dmaChanCspiRx:%d\r\n"), m_dmaChanCspiTx, m_dmaChanCspiRx ));
	rc = TRUE;

cleanUp:
	if (!rc)
	{
		DeinitChannelDMA();
	}
	DEBUGMSG(1,(_T("spiClass::InitChannelDMA-\r\n")));
	return rc;
}




//------------------------------------------------------------------------------
//
// Function: InitCspiDMA
//
//  Performs DMA channel initialization
//
// Parameters:
//  Index
//      CSPI device Index
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------

BOOL spiClass::InitCspiDMA(UINT32 Index) 
{ 

	DEBUGMSG(1, (TEXT("spiClass::InitDMA+\r\n")));

	m_dmaReqTx = CspCSPIGetDmaReqTx(Index) ; 
	m_dmaReqRx = CspCSPIGetDmaReqRx(Index) ; 
	RETAILMSG( 1, (TEXT("InitCspiDMA: m_dmaReqTx:%d m_dmaReqRx:%d\r\n"), m_dmaReqTx, m_dmaReqRx ));

	// Map the DMA buffers into driver's virtual address space
	if (!MapDMABuffers())
	{
		DEBUGMSG(1, (TEXT("spiClass::InitDMA() - Failed to map DMA buffers.\r\n")));
		return FALSE;
	}

	// Initialize the DMA channel
	if (!InitChannelDMA(Index))
	{
		DEBUGMSG(1, (TEXT("spiClass::InitDMA() - Failed to initialize output DMA.\r\n")));
		return FALSE;
	}

	DEBUGMSG(1, (TEXT("spiClass::InitDMA-\r\n")));
	return TRUE ; 
}


//------------------------------------------------------------------------------
//
// Function: CspiEnableLoopback
//
//  This function controls the LoopBackcontol of the FIFO.
//    Enable loopback also needs to enable Polling also.
//
// Parameters:
//    bEnable
//      Enable or Disable loopback
//
//  Returns:
//      None.
//
//------------------------------------------------------------------------------
void spiClass::CspiEnableLoopback(BOOL bEnable)
{
	if(bEnable)
	{
		m_bUseLoopBack = TRUE;
		//m_bUsePolling = TRUE;
	}
	else
	{
		m_bUseLoopBack = FALSE;
		//m_bUsePolling = FALSE;
	}
}

//-----------------------------------------------------------------------------
//
// Function: CspiADCConfig
//
// Exchanges CSPI data in Application Processor(CPU) Master mode.
//
// Parameters:
//      pXchPkt
//          [in] Points to exchange packet information.
//
// Returns:
//      Returns the number of data exchanges that completed successfully.
//   
//-----------------------------------------------------------------------------
DWORD spiClass::CspiADCConfig(PCSPI_XCH_PKT_T pXchPkt)
{
	enum {
		LOAD_TXFIFO, 
		MAX_XCHG, 
		FETCH_RXFIFO
	} xchState;
	PCSPI_BUSCONFIG_T pBusCnfg = pXchPkt->pBusCnfg;
	LPVOID pTxBuf = pXchPkt->pTxBuf;
	LPVOID pRxBuf = pXchPkt->pRxBuf;
	UINT32 xchCnt = pXchPkt->xchCnt;
	UINT32 xchTxCnt = 0;
	UINT32 xchRxCnt = 0;
	volatile UINT32 tmp;
	BOOL bXchDone;
	BOOL bReqPolling;
	UINT32 (*pfnTxBufRd)(LPVOID);
	void (*pfnRxBufWrt)(LPVOID, UINT32);
	UINT8 bufIncr;
	UINT8 byParamChipSelect;
	UINT8 byParamSSPOL;
	UINT8 byParamSSCTL;
	UINT8 byParamPOL;
	UINT8 byParamPHA;
	UINT8 byParamDRCTL;

	pBusCnfg->bitcount = 16;		//data rate = 16bit
	pBusCnfg->chipselect = 0;	//use channel 0
	pBusCnfg->freq = 12000000;	//XCH speed = 16M
	pBusCnfg->pha = TRUE;
	pBusCnfg->pol = FALSE;
	pBusCnfg->ssctl = TRUE;		//one entry entry with each SPI burst
	pBusCnfg->sspol = FALSE;		//SPI_CS Active low
	pBusCnfg->usepolling = FALSE;//polling don't use interrupt

	pBusCnfg->drctl = 0;			//Don't care SPI_RDY
	pBusCnfg->usedma = FALSE;	//Don't DMA


	pXchPkt->xchEvent = NULL;
	pXchPkt->xchEventLength = 0;

	DEBUGMSG(1, (TEXT("CspiADCConfig: &m_pCSPI=0x%x\r\n"),&m_pCSPI));
	// check all translated pointers
	if ((pBusCnfg == NULL) || (pTxBuf == NULL))
	{
		return 0;
	}

	// select access funtions based on exchange bit width
	//
	// bitcount        Tx/Rx Buffer Access Width
	// --------        -------------------------
	//   1 - 8           UINT8 (unsigned 8-bit)
	//   9 - 16          UINT16 (unsigned 16-bit)
	//  17 - 32          UINT32 (unsigned 32-bit)
	//
	if ((pBusCnfg->bitcount >= 1) && (pBusCnfg->bitcount <= 8))
	{
		// 8-bit access width
		pfnTxBufRd = CspiBufRd8;
		pfnRxBufWrt = CspiBufWrt8;
		bufIncr = sizeof(UINT8);
	}
	else if ((pBusCnfg->bitcount >= 9) && (pBusCnfg->bitcount <= 16))
	{
		// 16-bit access width
		pfnTxBufRd = CspiBufRd16;
		pfnRxBufWrt = CspiBufWrt16;
		bufIncr = sizeof(UINT16);
	}
	else if ((pBusCnfg->bitcount >= 17) && (pBusCnfg->bitcount <= 32))
	{
		// 32-bit access width
		pfnTxBufRd = CspiBufRd32;
		pfnRxBufWrt = CspiBufWrt32;
		bufIncr = sizeof(UINT32);
	}
	else
	{
		// unsupported access width
		DEBUGMSG(1, (TEXT("CspiMasterDataExchange:  unsupported bitcount!\r\n")));
		return 0;
	}

	// Enable the clock gating
	BSPCSPIEnableClock(m_Index,TRUE);

	// disable all interrupts
	OUTREG32(&m_pCSPI->INTREG, 0);

	// set client CSPI bus configuration based
	//  default EN = disabled
	//  default MODE = master
	//  default XCH = idle
	//  default SMC = XCH bit controls master start transfer
	byParamChipSelect= pBusCnfg->chipselect &((1U<<CSPI_CONREG_CHIPSELECT_WID) -1);
	byParamSSPOL = pBusCnfg->sspol? CSPI_CONREG_SSPOL_ACTIVE_HIGH: CSPI_CONREG_SSPOL_ACTIVE_LOW;
	//byParamSSCTL = pBusCnfg->ssctl? CSPI_CONREG_SSCTL_PULSE: CSPI_CONREG_SSCTL_ASSERT;
	byParamSSCTL = CSPI_CONREG_SSCTL_PULSE;
	byParamPOL = pBusCnfg->pol? CSPI_CONREG_POL_ACTIVE_LOW: CSPI_CONREG_POL_ACTIVE_HIGH;
	byParamPHA = pBusCnfg->pha? CSPI_CONREG_PHA1: CSPI_CONREG_PHA0;
	byParamDRCTL = pBusCnfg->drctl &((1U<<CSPI_CONREG_DRCTL_WID) -1);
	bReqPolling = FALSE;

	OUTREG32(&m_pCSPI->CONREG, 
		CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE) |
		CSP_BITFVAL(CSPI_CONREG_MODE, CSPI_CONREG_MODE_MASTER) |
		//CSP_BITFVAL(CSPI_CONREG_MODE, CSPI_CONREG_MODE_SLAVE) |
		CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_IDLE) |
		CSP_BITFVAL(CSPI_CONREG_SMC, CSPI_CONREG_SMC_XCH) |
		CSP_BITFVAL(CSPI_CONREG_CHIPSELECT, byParamChipSelect) |
		CSP_BITFVAL(CSPI_CONREG_DATARATE, BSPCSPICalculateDivRate(pBusCnfg->freq,pBusCnfg->freq/10)) |
		CSP_BITFVAL(CSPI_CONREG_SSPOL, byParamSSPOL) |
		CSP_BITFVAL(CSPI_CONREG_SSCTL, byParamSSCTL) |  //pBusCnfg->ssctl) |
		CSP_BITFVAL(CSPI_CONREG_POL, byParamPOL) |
		CSP_BITFVAL(CSPI_CONREG_PHA, byParamPHA) |
		CSP_BITFVAL(CSPI_CONREG_BITCOUNT, pBusCnfg->bitcount-1) |
		CSP_BITFVAL(CSPI_CONREG_DRCTL, byParamDRCTL));

	// enable the CSPI
	INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
		CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_ENABLE));

// 	if( !pBusCnfg->ssctl )
// 	{
// 		BSPCSPICS2IO( m_Index );
// 	}

//	RETAILMSG( 1, (TEXT("m_pCSPI->CONREG=0x%x\r\n"),INREG32(&m_pCSPI->CONREG)));

	if(m_bUseLoopBack)
	{
		INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_CONN);
	}
	else
	{
		INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_NOCONN);
	}

	// 	//CSPI pin Config in CSPI master mode
	BSPCSPIRDY2IO( m_Index );
	BSPCSPIRDYSet( m_Index, TRUE );
	BSPCSPISetIOMux( m_Index );
	//CSPI pin Config in CSPI slave mode
	//BAPCSPIRDY2IO( m_Index );
	//BSPCSPIRDYSet( m_Index, FALSE );
	//BSPCSPISetIOMux( m_Index );

	bXchDone = FALSE;
	xchState = LOAD_TXFIFO;

	// until we are done with requested transfers
	while(!bXchDone)
	{
xch_loop:
		switch (xchState)
		{
		case LOAD_TXFIFO: 

			// load Tx FIFO until full, or until we run out of data
			while ((!(INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TF)))
				&& (xchTxCnt < xchCnt))
			{
				// put next Tx data into CSPI FIFO
				OUTREG32(&m_pCSPI->TXDATA, pfnTxBufRd(pTxBuf));
				//RETAILMSG (1, (TEXT("pfnTxBufRd=0x%x, xchTxCnt=%d\r\n"),pfnTxBufRd(pTxBuf), xchTxCnt ));

				// increment Tx Buffer to next data point
				pTxBuf = (LPVOID) ((UINT) pTxBuf + bufIncr);

				// increment Tx exchange counter
				xchTxCnt++;
			}

			//lqk Sep 13, 2011
// 			if( !pBusCnfg->ssctl )
// 			{
// 				BSPCSPICSSet( m_Index, FALSE );
// 				//RETAILMSG (1, (TEXT("to Low\r\n")));
// 			}
			// start exchange
			INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_XCH), 
				CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_EN));

			xchState = (xchTxCnt<xchCnt)? MAX_XCHG: FETCH_RXFIFO;

			break;

		case MAX_XCHG:

			if (xchTxCnt >= xchCnt)
			{
				xchState = FETCH_RXFIFO;
				break;
			}
			// we need to wait until FIFO has more room, so we enable 
			// interrupt for Rx FIFO half-full (RHEN) or Rx FIFO ready 
			// to ensure we can
			// read out data that arrived during exchange
			// use polling
			// wait until RR
			while (!(INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
			{
				if (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC))
				{
					xchState = FETCH_RXFIFO;
					goto xch_loop;
				}
			}
			tmp = INREG32(&m_pCSPI->RXDATA);

			// if receive data is not to be discarded
			if (pRxBuf != NULL)
			{
				// get next Rx data from CSPI FIFO
				pfnRxBufWrt(pRxBuf, tmp);

				// increment Rx Buffer to next data point
				pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
			}

			// increment Rx exchange counter
			xchRxCnt++;

			OUTREG32(&m_pCSPI->TXDATA, pfnTxBufRd(pTxBuf));

			// increment Tx Buffer to next data point
			pTxBuf = (LPVOID) ((UINT) pTxBuf + bufIncr);

			// increment Tx exchange counter
			xchTxCnt++;

			if (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC))
			{
				xchState = FETCH_RXFIFO;
			}
			break;

		case FETCH_RXFIFO:

			// Fetch all rxdata already in RXFIFO
			while ((INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
			{
				tmp = INREG32(&m_pCSPI->RXDATA);

				// if receive data is not to be discarded
				if (pRxBuf != NULL)
				{
					// get next Rx data from CSPI FIFO
					pfnRxBufWrt(pRxBuf, tmp);

					// increment Rx Buffer to next data point
					pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
				}

				// increment Rx exchange counter
				xchRxCnt++;
			}

			// wait until transaction is complete
			while (!(INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC)))
				;
			while ((INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
			{
				tmp = INREG32(&m_pCSPI->RXDATA);

				// if receive data is not to be discarded
				if (pRxBuf != NULL)
				{
					// get next Rx data from CSPI FIFO
					pfnRxBufWrt(pRxBuf, tmp);

					// increment Rx Buffer to next data point
					pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
				}

				// increment Rx exchange counter
				xchRxCnt++;
			}

			// acknowledge transfer complete (w1c)
			OUTREG32(&m_pCSPI->STATREG, CSP_BITFMASK(CSPI_STATREG_TC));

			if (xchRxCnt >= xchCnt)
			{
				//lqk Sep 13, 2011
// 				if( !pBusCnfg->ssctl )
// 				{
// 					BSPCSPICSSet( m_Index, TRUE );
// 					//RETAILMSG (1, (TEXT("to High\r\n")));
// 				}
				// set flag to indicate requested exchange done
				bXchDone = TRUE;
			}
			else 
			{
				// exchange stopped, and we have received all data in RXFIFO
				// then there MUST be some data in the TxBuf, or in the TXFIFO,
				// or both.
				// we return the state to restart cspi XCH. 
				xchState = LOAD_TXFIFO;
			}

			break;

		default:
			bXchDone = TRUE;
			break;
		}
	} // while(!bXchDone)

	// disable the CSPI
	INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
		CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE));

	// Disable the clock gating
	BSPCSPIEnableClock(m_Index,FALSE);
	DEBUGMSG(1, (TEXT("CspiDataExchange -\r\n")));
	
	BSPCSPIReleaseIOMux( m_Index );

	return xchRxCnt;
}

void spiClass::PreTxBuf( BOOL bTest )
{
	DWORD dwIdx,dwCount;
	UINT32* pBuf;

	pBuf = (UINT32 *)m_pSPITxBuf;

	memset( m_pSPITxBuf, 0, m_dwTxDMABufferSize );
	dwCount = m_dwTxDMABufferSize/sizeof( UINT32 );

	bTest = FALSE;
	if( bTest )
	{
		for( dwIdx=0; dwIdx<dwCount ; )
		{
			pBuf[dwIdx++] = (UINT32)(((ADS8201_REG_WRITE|ADS8021_CHA_SEL_CCR)<<16)|m_ADChannle[m_dwADChannleIdx]<<18);	
			pBuf[dwIdx++] = (UINT32)((ADS8201_REG_READ|ADS8021_CHA_SEL_CCR)<<16);	
			m_dwADChannleIdx = (m_dwADChannleIdx+1)%m_dwADChannleCount;
		}
	}else
	{
		for( dwIdx=0; dwIdx<dwCount ; )
		{
			pBuf[dwIdx++] = (UINT32)(((ADS8201_REG_WRITE|ADS8021_CHA_SEL_CCR)<<16)|m_ADChannle[m_dwADChannleIdx]<<18);	
			m_dwADChannleIdx = (m_dwADChannleIdx+1)%m_dwADChannleCount;
		}
	}

	memcpy( m_pSpiVirtTxDMABufferAddr, m_pSPITxBuf, m_dwTxDMABufferSize );
	memcpy( m_pSpiVirtTxDMABufferAddr+m_dwTxDMABufferSize, m_pSPITxBuf, m_dwTxDMABufferSize );
}

DWORD spiClass::CspiADCRun( PADS_CONFIG pADSConfig, PCSPI_XCH_PKT_T pXchPkt )
{
	PCSPI_BUSCONFIG_T pBusCnfg;
	UINT8 byParamChipSelect;
	UINT8 byParamSSPOL;
	UINT8 byParamSSCTL;
	UINT8 byParamPOL;
	UINT8 byParamPHA;
	UINT8 byParamDRCTL;
	BOOL bReqPolling;

	DWORD i;
	UINT32 Mode;
	UINT8 nBufIdx;

	switch( m_nSamplingMode )
	{
	case -1:   // Continuous sampling
		m_bTestMode = TRUE;
		//m_dwSamplingLength = m_dwTxDMABufferSize;
		break;

	case SAMPLING_MODE_CONTINUOUS:	//Continuous sampling
		m_bTestMode = FALSE;
		
		m_dwSpiXchCount = pADSConfig->dwSamplingLength;	
		// Set SPI TX DMA count in bytes.
		m_dwSpiTxDmaCount = m_dwSpiXchCount*sizeof( UINT32 );
		m_dwSpiRxDmaCount = m_dwSpiXchCount*sizeof( UINT32);
		m_dwReceiveDataBufSize = m_dwSpiRxDmaCount*MULT_DMABUFFER_SIZE;
		break;

	default:	// Single sampling
		m_bTestMode = FALSE;

		m_dwSpiXchCount = pADSConfig->dwSamplingLength;	
		// Set SPI TX DMA count in bytes.
		m_dwSpiTxDmaCount = m_dwSpiXchCount*sizeof( UINT32 );
		m_dwSpiRxDmaCount = m_dwSpiXchCount*sizeof( UINT32 );
		m_dwReceiveDataBufSize = m_dwSpiRxDmaCount;
	}

	m_pSPITxBuf = (PBYTE)HeapAlloc( m_hHeap, HEAP_ZERO_MEMORY,  m_dwTxDMABufferSize );
	if( m_pSPITxBuf == NULL )
		goto error_adcRun;

	m_pSPIRxBuf = (PBYTE)HeapAlloc( m_hHeap, HEAP_ZERO_MEMORY,  m_dwReceiveDataBufSize );
	if( m_pSPIRxBuf == NULL )
		goto error_adcRun;

	//DMA transmit require XchCount is a multiple of 16( count in byte)
	m_dwSpiRxDmaCount = (m_dwSpiRxDmaCount/CSPI_DMA_WATERMARK_RX)*CSPI_DMA_WATERMARK_RX;
	
	RETAILMSG( 1, (TEXT("CspiADCRun:m_dwSpiTxDmaCount = 0x%x \r\n") ,m_dwSpiTxDmaCount) );

	//Prepare Sampling Channel
	for( m_dwADChannleIdx=0, m_dwADChannleCount=0, i=0; i<8; i++ )
	{
		if( pADSConfig->dwSamplingChannel & (0x01<<i))
			m_ADChannle[m_dwADChannleCount++] = (UINT8)i;
	}
	m_dwADChannleIdx = 0;		//ADC channle idx reset
	// 	RETAILMSG( 1, (TEXT("Sampling Channel Count = %d\r\n"),m_dwADChannleCount ));
	// 	for( i=0; i<m_dwADChannleCount; i++ )
	// 		RETAILMSG( 1, (TEXT("%d "),m_ADChannle[i]));
	// 	RETAILMSG( 1, (TEXT("\r\n")));

	//fill TX DMA buffer
	PreTxBuf( m_bTestMode );

	// set first sampling channel
	CSPI_BUSCONFIG_T stCspiConfig;
	CSPI_XCH_PKT_T stCspiXchPkt;
	stCspiXchPkt.pBusCnfg = &stCspiConfig;
	UINT16 SPITxBuf[10];
	UINT16 SPIRxBuf[10];
	stCspiXchPkt.pTxBuf = (LPVOID)SPITxBuf;
	stCspiXchPkt.pRxBuf = (LPVOID)SPIRxBuf;
	stCspiXchPkt.xchCnt = 1;
	SPITxBuf[0] = ADS8201_REG_WRITE|ADS8021_CHA_SEL_CCR|m_ADChannle[0];
	CspiADCConfig( &stCspiXchPkt );

	// reset register
	m_dwTempChAddr = m_ADChannle[0];
	m_dwSendTxByteCount = m_dwSpiTxDmaCount;
	m_dwReceiveByteCount = m_dwSpiRxDmaCount;
	// CspiADCConfig have sent the frist channel address, so m_dwSendTxDMAByteCount + 4
	m_dwSendTxDMAByteCount = 4;
	m_dwAvailRxByteCount = 0;
	m_nStopSampling = 0;
	m_dwPutIdx = 0;
	m_dwGetIdx = 0;

	//EM9170's CSPI in slave mode, spi_cs and spi_sclk supported by the CPLD of the ETA108.
	BSPCSPIRDYSet( m_Index, FALSE );

	// Enable the clock gating
	BSPCSPIEnableClock(m_Index,TRUE);

	// disable all CSPI interrupts
	OUTREG32(&m_pCSPI->INTREG, 0);

	pBusCnfg = pXchPkt->pBusCnfg;

	// set client CSPI bus configuration based
	byParamChipSelect= pBusCnfg->chipselect &((1U<<CSPI_CONREG_CHIPSELECT_WID) -1);
	byParamSSPOL = pBusCnfg->sspol? CSPI_CONREG_SSPOL_ACTIVE_HIGH: CSPI_CONREG_SSPOL_ACTIVE_LOW;
	byParamSSCTL = pBusCnfg->ssctl? CSPI_CONREG_SSCTL_PULSE: CSPI_CONREG_SSCTL_ASSERT;
	byParamPOL = pBusCnfg->pol? CSPI_CONREG_POL_ACTIVE_LOW: CSPI_CONREG_POL_ACTIVE_HIGH;
	byParamPHA = pBusCnfg->pha? CSPI_CONREG_PHA1: CSPI_CONREG_PHA0;
	byParamDRCTL = pBusCnfg->drctl &((1U<<CSPI_CONREG_DRCTL_WID) -1);
	bReqPolling = FALSE;

	OUTREG32(&m_pCSPI->CONREG, 
		CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE) |
		//CSP_BITFVAL(CSPI_CONREG_MODE, CSPI_CONREG_MODE_MASTER) |
		CSP_BITFVAL(CSPI_CONREG_MODE, CSPI_CONREG_MODE_SLAVE) |
		CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_IDLE) |
		CSP_BITFVAL(CSPI_CONREG_SMC, CSPI_CONREG_SMC_XCH) |
		CSP_BITFVAL(CSPI_CONREG_CHIPSELECT, byParamChipSelect) |
		CSP_BITFVAL(CSPI_CONREG_DATARATE, BSPCSPICalculateDivRate(pBusCnfg->freq,pBusCnfg->freq/10)) |
		CSP_BITFVAL(CSPI_CONREG_SSPOL, byParamSSPOL) |
		CSP_BITFVAL(CSPI_CONREG_SSCTL, byParamSSCTL) |
		CSP_BITFVAL(CSPI_CONREG_POL, byParamPOL) |
		CSP_BITFVAL(CSPI_CONREG_PHA, byParamPHA) |
		CSP_BITFVAL(CSPI_CONREG_BITCOUNT, pBusCnfg->bitcount-1) |
		CSP_BITFVAL(CSPI_CONREG_DRCTL, byParamDRCTL));

	if(m_bUseLoopBack)
	{
		INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_CONN);
	}
	else
	{
		INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_NOCONN);
	}

	//RETAILMSG( 1, (TEXT("CspiADCRun:m_pCSPI->CONREG=0x%x\r\n"),INREG32(&m_pCSPI->CONREG)));

	// enable the CSPI
	INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
		CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_ENABLE));

	//CSPI pin Config in CSPI slave mode
	BSPCSPIRDY2IO( m_Index );
	BSPCSPIRDYSet( m_Index, FALSE );
	BSPCSPISetIOMux( m_Index );

	// ensure RXFIFO is empty before start DMA
	while ( INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR))
	{
		INREG32(&m_pCSPI->RXDATA);
	}

	// assert DMA request for half or empty
	// irrespective of the condition, 
	// we need to enable the transmitter
	OUTREG32(&m_pCSPI->DMAREG, 
		CSP_BITFVAL(CSPI_DMAREG_THDEN, CSPI_DMAREG_THDEN_ENABLE)|
		CSP_BITFVAL(CSPI_DMAREG_RHDEN, CSPI_DMAREG_RHDEN_ENABLE));
	//CSP_BITFVAL(CSPI_DMAREG_TEDEN, CSPI_DMAREG_TEDEN_ENABLE)|
	//CSP_BITFVAL(CSPI_DMAREG_RFDEN, CSPI_DMAREG_RFDEN_ENABLE));

	UINT16 transferBytes;

	// configure RX DMA
	DDKSdmaClearChainStatus(m_dmaChanCspiRx);
	for ( transferBytes = 0, nBufIdx = 0; nBufIdx < CSPI_DMA_COUNT; nBufIdx++)
	{
		DDKSdmaClearBufDescStatus(m_dmaChanCspiRx,nBufIdx);
		Mode = nBufIdx?DDK_DMA_FLAGS_WRAP:DDK_DMA_FLAGS_CONT;
		if( m_nSamplingMode == SAMPLING_MODE_CONTINUOUS )
			transferBytes = (UINT16)m_dwSpiRxDmaCount;
		else transferBytes = (UINT16)MIN_VAL( m_dwRxDMABufferSize, m_dwReceiveByteCount );
		if( transferBytes> 0 )
		{
			DDKSdmaSetBufDesc(m_dmaChanCspiRx,
				nBufIdx,
				(Mode|DDK_DMA_FLAGS_INTR),
				(m_pSpiPhysRxDMABufferAddr.LowPart) +nBufIdx * m_dwRxDMABufferSize,
				0,
				DDK_DMA_ACCESS_32BIT,
				transferBytes );	// Set the count in bytes
			m_dwReceiveByteCount -= transferBytes;
		}
	}

	// configure TX DMA
	DDKSdmaClearChainStatus(m_dmaChanCspiTx);
	for (transferBytes = 0, nBufIdx = 0; nBufIdx < CSPI_DMA_COUNT; nBufIdx++)
	{
		DDKSdmaClearBufDescStatus(m_dmaChanCspiTx,nBufIdx);
		Mode = nBufIdx?DDK_DMA_FLAGS_WRAP:DDK_DMA_FLAGS_CONT;
		if( m_nSamplingMode == SAMPLING_MODE_CONTINUOUS )
			transferBytes = (UINT16)m_dwTxDMABufferSize;
		else transferBytes = (UINT16)MIN_VAL( m_dwTxDMABufferSize, m_dwSendTxByteCount );
		if( transferBytes>0 )
		{
			if( nBufIdx == 0 )
			{
				DDKSdmaSetBufDesc(m_dmaChanCspiTx,
					nBufIdx,
					(Mode|DDK_DMA_FLAGS_INTR),
					(m_pSpiPhysTxDMABufferAddr.LowPart) + 4,
					0,
					DDK_DMA_ACCESS_32BIT,
					transferBytes-4 );	// Set the count in bytes
			}
			else
			{
				DDKSdmaSetBufDesc(m_dmaChanCspiTx,
					nBufIdx,
					(Mode|DDK_DMA_FLAGS_INTR),
					(m_pSpiPhysTxDMABufferAddr.LowPart) + nBufIdx * m_dwTxDMABufferSize,
					0,
					DDK_DMA_ACCESS_32BIT,
					transferBytes );	// Set the count in bytes
			}
		}
		m_dwSendTxByteCount -= transferBytes;
	}

	if (!DDKSdmaInitChain(m_dmaChanCspiRx, CSPI_DMA_WATERMARK_RX))
	{
		DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaInitChain failed.\r\n")));
		goto error_adcRun;
	}
	// start Rx DMA channel
	DDKSdmaStartChan(m_dmaChanCspiRx);

	if (!DDKSdmaInitChain(m_dmaChanCspiTx, CSPI_DMA_WATERMARK_TX))
	{
		DEBUGMSG(1, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaInitChain failed.\r\n")));
		goto error_adcRun;
	}
	// start Tx DMA channel
	DDKSdmaStartChan(m_dmaChanCspiTx);
	
	// load Tx FIFO until enough for XCH
	while (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TE))
	{
		RETAILMSG( 1, (TEXT("m_dmaChanCspiTx:Wait data...\r\n")));
		Sleep(1000);
	}

	RETAILMSG( 1, (TEXT("CspiADCRun:Start PWM.\r\n")));
	StartPWM( pADSConfig->dwSamplingRate, 0 );
	return TRUE;

error_adcRun:
	CspiADCDone( );
	return FALSE;
}

void spiClass::CspiProcess(  )
{

	while( !m_bTerminate )
	{
		// wait for requested transfer interrupt
		WaitForSingleObject(m_hIntrEvent, INFINITE);
		if( m_bTerminate )
			break;

		if( m_nSamplingMode == SAMPLING_MODE_SINGLE )
		{
			CspiSingleSampling( );
		}
		else
		{
			CspiContinueSampling( );
		}


		//Check TC interrupt
		//transfer completed if TC interrupt.
		if( INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TE))
		{
			RETAILMSG( 1, (TEXT("CspiProcess: TC interrupt, m_pCSPI->STATREG=0x%x\r\n"),INREG32(&m_pCSPI->STATREG)));
			// acknowledge transfer complete (w1c)
			OUTREG32(&m_pCSPI->STATREG, CSP_BITFMASK(CSPI_STATREG_TC));	
			if( m_dwSpiTxDmaCount ==  m_dwSendTxDMAByteCount )
			{
				//BSPCSPIReleaseIOMux( m_Index );
			}
//  				else
//  				{
//  					// load Tx FIFO until enough for XCH, or until we run out of data
//  					while (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TE));
//  					// start exchange
//  					INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_XCH), 
//  						CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_EN));
//  				}

		}
		// enable TC interrupt
		//INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_TCEN), 
		//	CSP_BITFVAL(CSPI_INTREG_TCEN, CSPI_INTREG_TCEN_ENABLE));

		InterruptDone(m_dwSysIntr);
	}
	m_hThread = NULL;
}

void spiClass::CspiSingleSampling( )
{
	UINT32 Mode;
	UINT8  nBufIdx;
	UINT32 bufDescStatus[CSPI_DMA_COUNT], status;
	UINT16	transferBytes;	
	BOOL    bStartTxDMA, bStartRxDMA;
	

	//RETAILMSG( 1, (TEXT("\r\nCspiProcess: m_dwSpiTxDmaCount=0x%x\r\n"),m_dwSpiTxDmaCount ));
	//RETAILMSG( 1, (TEXT("CspiProcess:m_pCSPI->STATREG=0x%x\r\n"),INREG32(&m_pCSPI->STATREG)));

	// Check RX DMA interrupt
	// attempt to fill the RX Buffer(m_pSPIRxBuf) as much ad possible, by copying from currently  
	// IDX SDMA buffer. update m_dwAvailRxByteCount and m_currRxDmaBufIdx.

	bStartRxDMA = FALSE;
	DDKSdmaGetChainStatus( m_dmaChanCspiRx, bufDescStatus );
	for (nBufIdx = 0; nBufIdx < CSPI_DMA_COUNT; nBufIdx++)
	{
		status = bufDescStatus[nBufIdx];
		if ((status & DDK_DMA_FLAGS_BUSY) == 0 )
		{
			DDKSdmaClearBufDescStatus(m_dmaChanCspiRx, nBufIdx);
			//RETAILMSG( 1, (TEXT("CspiProcess: RX BUFFER_%d. status = 0x%x\r\n"), nBufIdx, status ));
			status &= 0xffff;
			if( status>0 )
			{
				// SPI Receive buffer size equal to m_dwSpiTxDmaCount.
				memcpy( (PBYTE)m_pSPIRxBuf+m_dwAvailRxByteCount, m_pSpiVirtRxDmaBufferAddr+nBufIdx*m_dwRxDMABufferSize, status );

// 				for( int j=0; j<20; j++ )
// 					RETAILMSG( 1, (TEXT(" 0x%x"), *((PBYTE)m_pSPIRxBuf+m_dwAvailRxByteCount+j )));
// 				RETAILMSG( 1, (TEXT("\r\n")));

				m_dwAvailRxByteCount += status;
				//RETAILMSG( 1, (TEXT("m_dwAvailRxByteCount=0x%x,m_dwReceiveByteCount=0x%x\r\n"), m_dwAvailRxByteCount, m_dwReceiveByteCount));
			}

			if( m_dwAvailRxByteCount >= m_dwSpiRxDmaCount )
			{
				DDKSdmaStopChan(m_dmaChanCspiRx, TRUE);  //force stop Rx DMA
				BSPCSPIReleaseIOMux( m_Index );
				StartPWM( 1, 1 );
				RETAILMSG( 1, (TEXT("CspiSingleSampling: Stop RX dma.Receive DMA bytes:0x%x\r\n"), m_dwAvailRxByteCount));
// 				while( m_dwAvailRxByteCount < m_dwReceiveDataBufSize )
// 				{
// 					// while there is data in Rx FIFO 
// 					while (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR))
// 					{
// 						// increment Rx Buffer to next data point
// 						dwTmp = INREG32(&m_pCSPI->RXDATA);
// 		
// 						RETAILMSG( 1, (TEXT(" 0x%x"), dwTmp));
// 						if( m_dwAvailRxByteCount < m_dwReceiveDataBufSize )
// 							CspiBufWrt32(m_pSPIRxBuf+m_dwAvailRxByteCount, dwTmp);			
// 						else break;
// 						m_dwAvailRxByteCount+=sizeof(UINT32);
// 						Sleep(10);
// 						//RETAILMSG( 1, (TEXT(" 0x%x, m_dwAvailRxByteCount=0x%x\r\n"), dwTmp, m_dwAvailRxByteCount));
// 					}
// 				}


				// disable the CSPI
				INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
					CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE));

				BSPCSPIEnableClock(m_Index,FALSE);

				ReleaseSemaphore( m_hTransferDoneSemaphore, 1, NULL );

				RETAILMSG( 1, (TEXT("CspiSingleSampling: Transfer completed. Receive:0x%x\r\n\r\n"), m_dwAvailRxByteCount));
				break;
			}

			transferBytes = (UINT16)MIN_VAL( m_dwRxDMABufferSize, m_dwReceiveByteCount );
			if( transferBytes>0 )
			{

				Mode = nBufIdx ? DDK_DMA_FLAGS_WRAP : DDK_DMA_FLAGS_CONT;
				DDKSdmaSetBufDesc(m_dmaChanCspiRx,
					nBufIdx,
					(Mode|DDK_DMA_FLAGS_INTR),
					(m_pSpiPhysRxDMABufferAddr.LowPart) +
					nBufIdx * m_dwRxDMABufferSize,
					0,
					DDK_DMA_ACCESS_32BIT,
					transferBytes );	// Set the count in bytes
				m_dwReceiveByteCount -= transferBytes;
				bStartRxDMA = TRUE;
			}
		}
	}
	if( bStartRxDMA )
	{
		DDKSdmaStartChan(m_dmaChanCspiRx);
		//RETAILMSG( 1, (TEXT("CspiProcess: Start rx dma\r\n")));
	}

	// Check TX DMA interrupt
	DDKSdmaGetChainStatus( m_dmaChanCspiTx, bufDescStatus );
	for (bStartTxDMA = FALSE, nBufIdx = 0; nBufIdx < CSPI_DMA_COUNT; nBufIdx++)
	{
		if(!( bufDescStatus[nBufIdx] &DDK_DMA_FLAGS_BUSY ))
		{
			DDKSdmaClearBufDescStatus(m_dmaChanCspiTx,nBufIdx);
			m_dwSendTxDMAByteCount += bufDescStatus[nBufIdx]&0xffff;
			//RETAILMSG( 1, (TEXT("CspiProcess: TX BUFFER_%d status=0x%x, m_dwSendTxDMAByteCount=0x%x\r\n"),nBufIdx,bufDescStatus[nBufIdx],m_dwSendTxDMAByteCount  ));
			if( m_dwSendTxDMAByteCount ==  m_dwSpiTxDmaCount )
			{
				DDKSdmaStopChan(m_dmaChanCspiTx, TRUE);	//force stop Tx DMA
				RETAILMSG( 1, (TEXT("CspiSingleSampling: Stop TX dma.Send DMA bytes:0x%x\r\n"), m_dwSendTxDMAByteCount));
				// enable TC interrupt
				INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_TCEN), 
					CSP_BITFVAL(CSPI_INTREG_TCEN, CSPI_INTREG_TCEN_ENABLE));
				break;
			}

			transferBytes = (UINT16)MIN_VAL( m_dwTxDMABufferSize, m_dwSendTxByteCount );

			//for( int i=0; i<10; i++ )
			//	RETAILMSG( 1, (TEXT(" 0x%x"), *(m_pSpiVirtTxDMABufferAddr+nBufIdx*m_dwTxDMABufferSize+i )));
			//RETAILMSG( 1, (TEXT("\r\n")));
			Mode = nBufIdx ? DDK_DMA_FLAGS_WRAP : DDK_DMA_FLAGS_CONT;
			if( transferBytes>0 )
			{
				DDKSdmaSetBufDesc(m_dmaChanCspiTx,
					nBufIdx,
					(Mode|DDK_DMA_FLAGS_INTR),
					(m_pSpiPhysTxDMABufferAddr.LowPart) +
					nBufIdx * m_dwTxDMABufferSize,
					0,
					DDK_DMA_ACCESS_32BIT,
					transferBytes );	// Set the count in bytes
				bStartTxDMA = TRUE;
			}
			m_dwSendTxByteCount -= transferBytes;
		}
	}
	if( bStartTxDMA )
	{
		DDKSdmaStartChan(m_dmaChanCspiTx);
		//RETAILMSG( 1, (TEXT("CspiProcess: Start tx dma\r\n")));
	}
}

void spiClass::CspiContinueSampling( )
{
	UINT32 Mode;
	UINT8  nBufIdx;
	UINT32 bufDescStatus[CSPI_DMA_COUNT], status;
	BOOL    bStartTxDMA, bStartRxDMA;

	bStartRxDMA = FALSE;
	DDKSdmaGetChainStatus( m_dmaChanCspiRx, bufDescStatus );
	for (nBufIdx = 0; nBufIdx < CSPI_DMA_COUNT; nBufIdx++)
	{
		status = bufDescStatus[nBufIdx];
		if ((status & DDK_DMA_FLAGS_BUSY) == 0 )
		{
			DDKSdmaClearBufDescStatus(m_dmaChanCspiRx, nBufIdx);
			//RETAILMSG( 1, (TEXT("CspiContinueSampling: RX BUFFER_%d. status = 0x%x\r\n"), nBufIdx, status ));
			status &= 0xffff;
			if( status>0 )
			{
				// SPI Receive buffer size equal to m_dwSpiTxDmaCount.
				EnterCriticalSection( &m_cspiRxBufCs );
				if( (m_dwPutIdx+1)%MULT_DMABUFFER_SIZE != m_dwGetIdx )
				{
					memcpy( (PBYTE)m_pSPIRxBuf+m_dwPutIdx*m_dwSpiRxDmaCount, m_pSpiVirtRxDmaBufferAddr+nBufIdx*m_dwRxDMABufferSize, status );
					m_dwPutIdx = (m_dwPutIdx+1)%MULT_DMABUFFER_SIZE;
				}
				else
				{
					RETAILMSG( 1, (TEXT("CspiContinueSampling: Data loss!!!\r\n")));
				}
				LeaveCriticalSection( &m_cspiRxBufCs );

				m_dwAvailRxByteCount += status;
				//RETAILMSG( 1, (TEXT("m_dwAvailRxByteCount=0x%x,m_dwReceiveByteCount=0x%x\r\n"), m_dwAvailRxByteCount, m_dwReceiveByteCount));
				ReleaseSemaphore( m_hTransferDoneSemaphore, 1, NULL );
			}

			if( m_nStopSampling )
			{
				//m_nStopSampling &= ~RXDMA_IS_STOP; 
				m_nStopSampling = 0;
				BSPCSPIReleaseIOMux( m_Index );
				DDKSdmaStopChan(m_dmaChanCspiRx, TRUE);  //force stop Rx DMA
				DDKSdmaStopChan(m_dmaChanCspiTx, TRUE);  //force stop Rx DMA
				//stop PWM output
				StartPWM(1,1);

				// disable the CSPI
				INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
					CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE));

				BSPCSPIEnableClock(m_Index,FALSE);

				RETAILMSG( 1, (TEXT("CspiContinueSampling: Transfer abort. Receive:0x%x\r\n\r\n"), m_dwAvailRxByteCount));
				break;
			}

			Mode = nBufIdx ? DDK_DMA_FLAGS_WRAP : DDK_DMA_FLAGS_CONT;
			DDKSdmaSetBufDesc(m_dmaChanCspiRx,
				nBufIdx,
				(Mode|DDK_DMA_FLAGS_INTR),
				(m_pSpiPhysRxDMABufferAddr.LowPart) +
				nBufIdx * m_dwRxDMABufferSize,
				0,
				DDK_DMA_ACCESS_32BIT,
				(UINT16)m_dwSpiRxDmaCount );	// Set the count in bytes
			DDKSdmaStartChan(m_dmaChanCspiRx);
		}
	}


	// Check TX DMA interrupt
	DDKSdmaGetChainStatus( m_dmaChanCspiTx, bufDescStatus );
	for (bStartTxDMA = FALSE, nBufIdx = 0; nBufIdx < CSPI_DMA_COUNT; nBufIdx++)
	{
		if(!( bufDescStatus[nBufIdx] &DDK_DMA_FLAGS_BUSY ))
		{
			DDKSdmaClearBufDescStatus(m_dmaChanCspiTx,nBufIdx);
			//m_dwSendTxDMAByteCount += bufDescStatus[nBufIdx]&0xffff;
			//RETAILMSG( 1, (TEXT("CspiProcess: TX BUFFER_%d status=0x%x, m_dwSendTxDMAByteCount=0x%x\r\n"),nBufIdx,bufDescStatus[nBufIdx],m_dwSendTxDMAByteCount  ));
// 			if( m_nStopSampling )
// 			{
// 				m_nStopSampling &= ~TXDMA_IS_STOP;
// 				DDKSdmaStopChan(m_dmaChanCspiTx, TRUE);	//force stop Tx DMA
// 				RETAILMSG( 1, (TEXT("CspiProcess: Stop tx dma.\r\n")));
// 				// enable TC interrupt
// 				//INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_TCEN), 
// 				//	CSP_BITFVAL(CSPI_INTREG_TCEN, CSPI_INTREG_TCEN_ENABLE));
// 				break;
// 			}

			//for( int i=0; i<10; i++ )
			//	RETAILMSG( 1, (TEXT(" 0x%x"), *(m_pSpiVirtTxDMABufferAddr+nBufIdx*m_dwTxDMABufferSize+i )));
			//RETAILMSG( 1, (TEXT("\r\n")));
			Mode = nBufIdx ? DDK_DMA_FLAGS_WRAP : DDK_DMA_FLAGS_CONT;
			DDKSdmaSetBufDesc(m_dmaChanCspiTx,
				nBufIdx,
				(Mode|DDK_DMA_FLAGS_INTR),
				(m_pSpiPhysTxDMABufferAddr.LowPart) +
				nBufIdx * m_dwTxDMABufferSize,
				0,
				DDK_DMA_ACCESS_32BIT,
				(UINT16)m_dwTxDMABufferSize );	// Set the count in bytes
			DDKSdmaStartChan(m_dmaChanCspiTx);
		}
	}
}

void spiClass::CspiADCDone()
{
	//RETAILMSG( 1, (TEXT("m_pCSPI->STATREG=0x%x\r\n"),INREG32(&m_pCSPI->STATREG)));
	//RETAILMSG( 1, (TEXT("m_pCSPI->CONREG=0x%x\r\n"),INREG32(&m_pCSPI->CONREG)));
	RETAILMSG( 1, (TEXT("CspiADCDone: ETA108 is Closed.\r\n")));
	if( m_nSamplingMode == SAMPLING_MODE_CONTINUOUS )
	{
		m_nStopSampling = STOP_SAMPLING;
		while( m_nStopSampling )
		{
			Sleep(100);
		}
	}

	BSPCSPIReleaseIOMux( m_Index );
	// disable the CSPI
	INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
		CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE));

	BSPCSPIEnableClock(m_Index,FALSE);

	EnterCriticalSection( &m_cspiRxBufCs );
	if( m_pSPIRxBuf != NULL )
	{
		HeapFree( m_hHeap, 0, m_pSPIRxBuf );
		m_pSPIRxBuf = NULL;
	}
	LeaveCriticalSection( &m_cspiRxBufCs );
	
	EnterCriticalSection( &m_cspiTxBufCs );
	if( m_pSPITxBuf != NULL )
	{
	
		HeapFree( m_hHeap, 0, m_pSPITxBuf );
		m_pSPITxBuf = NULL;
	}
	LeaveCriticalSection( &m_cspiTxBufCs );
}

DWORD  spiClass::ReadADCData(LPVOID pBuffer, DWORD dwCount)
{
	DWORD i,j,k;
	DWORD dwReadBytes;
	DWORD dwTemp;
	DWORD dwReadCount, dwPerChanSampCount;		//Count in UINT32
	UINT32 *pBuf = (UINT32*)pBuffer;
	UINT32 *pRxBuf = (UINT32*)m_pSPIRxBuf;	
	
//  	RETAILMSG( 1, (TEXT("ReadADCData:") ) ); 
// 	for( i=0; i<10;i++ )
//  		RETAILMSG( 1, (TEXT(" 0x%x\r\n") ,pRxBuf[i]) );
//	pRxBuf++;	// ignore the frist data.
	//RETAILMSG( 1, (TEXT("ReadADCData:dwCount=0x%x,m_dwSpiTxDmaCount=0x%x\r\n"),dwCount, m_dwSpiTxDmaCount ));
	if( pBuf == NULL || pRxBuf == NULL )
		return 0;
	dwReadBytes = MIN_VAL( dwCount, m_dwSpiTxDmaCount );
 	//memcpy( pBuf, pRxBuf, dwReadBytes );
 	//return dwReadBytes;
	dwPerChanSampCount = dwReadBytes/m_dwADChannleCount/sizeof(UINT32);
	dwReadCount = dwPerChanSampCount*m_dwADChannleCount;

	EnterCriticalSection( &m_cspiRxBufCs );
	// Channel address right
 	for( i=0; i<dwReadCount; i++ )
 	{
 		dwTemp = pRxBuf[i]&0x0f;
 		pRxBuf[i] = pRxBuf[i] & 0xfffffff0 | m_dwTempChAddr;
 		m_dwTempChAddr = dwTemp;
 	}

	if( m_nSamplingMode == SAMPLING_MODE_SINGLE )
	{
		for( i=0; i<m_dwADChannleCount; i++ )
		{
			for( j=0, k=i; j<dwPerChanSampCount; j++ )
			{
				pBuf[ i*dwPerChanSampCount+j] = pRxBuf[k]&ADS8201_IGNORE_BIT;
				k += m_dwADChannleCount;
			}
		}
	}else
	{
		if( m_dwGetIdx != m_dwPutIdx )
		{
			for( i=0; i<m_dwADChannleCount; i++ )
			{
				for( j=0, k=i+m_dwGetIdx*m_dwSpiXchCount; j<dwPerChanSampCount; j++ )
				{
					pBuf[ i*dwPerChanSampCount+j] = pRxBuf[k]&ADS8201_IGNORE_BIT;
					k += m_dwADChannleCount;
				}
			}
			m_dwGetIdx = (m_dwGetIdx+1)%MULT_DMABUFFER_SIZE;
		}
		else
		{
			dwReadCount = 0;
		}
	}
	LeaveCriticalSection( &m_cspiRxBufCs );

	return (dwReadCount*sizeof(UINT32));
}


BOOL spiClass::OpenPWM()
{
	m_hPWM = CreateFile(_T("PWM2:"),          // name of device
		GENERIC_READ|GENERIC_WRITE,         // desired access
		FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
		NULL,                               // security attributes (ignored)
		OPEN_EXISTING,                      // creation disposition
		FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
		NULL);   

	// if we failed to get handle to CSPI
	if (m_hPWM == INVALID_HANDLE_VALUE)
	{
		RETAILMSG( 1, (TEXT("ETA108Open: Con't open pwm!!!\r\n")));
		return FALSE;
	}
	else return TRUE;
}

BOOL spiClass::StartPWM( DWORD dwFreq, DWORD dwDuration )
{
	if( m_hPWM == INVALID_HANDLE_VALUE )
		return FALSE;
	//Config PWM
	PWMINFO PwmInfo; 
	DWORD dwNumberOfBytesToWrite; 
	DWORD dwNumberOfBytesWritten; 

	PwmInfo.dwFreq = dwFreq;

	// PWM Duty cycle = CONVST(convert start)pulse width>40nS.(CONVST active low level)
	// ETA108'S CPLD utilize 6M sample clock to generate AD start conversion pulse ,
	// so the high level of the PWM should be greater than 166ns(1/6M).
	PwmInfo.dwDuty = 2;	
	// Remain output until issue a new write operation
	PwmInfo.dwDuration = dwDuration;			
	dwNumberOfBytesToWrite = sizeof(PWMINFO); 
	dwNumberOfBytesWritten = 0; 
	//Now start AD conversion
	return WriteFile(m_hPWM, (LPCVOID)&PwmInfo, dwNumberOfBytesToWrite, &dwNumberOfBytesWritten, NULL); 	
}

void spiClass::ClosePWM()
{
	if( m_hPWM != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hPWM);
		m_hPWM = INVALID_HANDLE_VALUE;
	}

}

void spiClass::ETA108Reset( UINT32 dwTimes )
{
	BSPETA108Reset( dwTimes );
}