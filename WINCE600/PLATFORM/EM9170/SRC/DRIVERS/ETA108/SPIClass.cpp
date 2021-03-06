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

extern "C" void BAPCSPIRDY2IO( UINT32 dwIndex );
extern "C" void BSPCSPICS2IO( UINT32 dwIndex  );
extern "C" void BSPCSPIRDYSet( UINT32 dwIndex, BOOL bVal );
extern "C" void BSPCSPICSSet( UINT32 dwIndex, BOOL bVal);


//
//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

#define  CSPI_DMA_WATERMARK_RX		8
#define  CSPI_DMA_WATERMARK_TX		8
#define  CSPI_DMA_COUNT				2

#define  SAMPLING_MODE_MANUAL		1
#define  SAMPLING_MODE_CONTINUOUS	2

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
	m_hTransferDoneEvent = NULL;
	
	m_pSPITxBuf = NULL;
	m_pSPIRxBuf = NULL;

	m_hThread = NULL;

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
	DEBUGMSG(ZONE_INIT, (TEXT("CspiInitialize: m_hHeap=0x%x\r\n"),m_hHeap));
	m_hHeap = HeapCreate(0, 0, 0);

	// check if HeapCreate failed
	if (m_hHeap == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  HeapCreate failed!\r\n")));
		goto Init_Error;
	}

	// create event for CSPI interrupt signaling, SPI RX DMA interrupt and TX DMA interrupt.
	//      pEventAttributes = NULL (must be NULL)
	//      bManualReset = FALSE => resets automatically to nonsignaled
	//                              state after waiting thread released
	//      bInitialState = FALSE => initial state is non-signaled
	//      lpName = NULL => object created without a name
	m_hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// check if CreateEvent failed
	if (m_hIntrEvent == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  CreateEvent failed!\r\n")));
		goto Init_Error;
	}

	// Get the Base Address according to Index
	phyAddr.QuadPart = CspCSPIGetBaseRegAddr(Index);
	if( !phyAddr.QuadPart )
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  invalid CSPI instance!\r\n")));
		goto Init_Error;
	}
	RETAILMSG(1, (TEXT("CspiInitialize:CSPI Base: 0x%x)\r\n"), phyAddr.QuadPart) );

	// Map peripheral physical address to virtual address
	m_pCSPI = (PCSP_CSPI_REG) MmMapIoSpace(phyAddr, sizeof(CSP_CSPI_REG),FALSE);

	// Check if virtual mapping failed
	if (m_pCSPI == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  MmMapIoSpace failed!\r\n")));
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
		DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  InterruptInitialize failed!\r\n")));
		goto Init_Error;
	}

	// create CSPI Tx buffer critical section
	InitializeCriticalSection(&m_cspiTxBufCs);

	// create CSPI Rx buffer critical section
	InitializeCriticalSection(&m_cspiRxBufCs);

	DEBUGMSG(ZONE_INIT, (TEXT("CspiInitialize: &m_pCSPI=0x%x\r\n"),&m_pCSPI));
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
		DEBUGMSG(ZONE_INIT, (TEXT("CspiInitialize:  this=0x%x\r\n"),this));

		// check if CreateThread failed
		if (m_hThread == NULL)
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  CreateThread failed!\r\n")));
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
	DEBUGMSG(ZONE_CLOSE, (TEXT("CspiRelease +\r\n")));

	// kill the exchange packet processing thread
	if (m_hThread)
	{
		m_bTerminate=TRUE;
// 		// try to signal the thread so that it can wake up and terminate
// 		if (m_hEnQEvent)
// 		{
// 			SetEvent(m_hEnQEvent);
// 		}
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	// Release IOMUX pins
	//BSPCSPIReleaseIOMux(m_Index);
	// Release SYSINTR
	KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_dwSysIntr, sizeof(DWORD), NULL, 0, NULL);
	m_dwSysIntr = (DWORD)SYSINTR_UNDEFINED;

	// destroy the heap
	if (m_hHeap != NULL)
	{
		HeapDestroy(m_hHeap);
		m_hHeap = NULL;
	}

	// close interrupt event handle
	if (m_hIntrEvent)
	{
		CloseHandle(m_hIntrEvent);
		m_hIntrEvent = NULL;
	}

	// free the virtual space allocated for CSPI memory map
	if (m_pCSPI != NULL)
	{
		MmUnmapIoSpace(m_pCSPI, sizeof(CSP_CSPI_REG));
		m_pCSPI = NULL;
	}

	// deregister the system interrupt
	if (m_dwSysIntr != SYSINTR_UNDEFINED)
	{
		InterruptDisable(m_dwSysIntr);
		m_dwSysIntr = (DWORD)SYSINTR_UNDEFINED;
	}

	// delete the Tx buffer critical section
	DeleteCriticalSection(&m_cspiTxBufCs);

	// delete the Rx buffer critical section
	DeleteCriticalSection(&m_cspiRxBufCs);

	DEBUGMSG(ZONE_CLOSE, (TEXT("CspiRelease -\r\n")));
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
 		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi:DeinitChannelDMA() - Failed to deinitialize DMA.\r\n")));
 		return FALSE;
 	}
 	if(!UnmapDMABuffers())
 	{
 		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi:UnmapDMABuffers() - Failed to Unmap DMA buffers\r\n")));
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
	DEBUGMSG(ZONE_FUNCTION,(_T("Cspi::MapDMABuffers+\r\n")));

	m_pSpiVirtTxDMABufferAddr = NULL;
	m_pSpiVirtRxDmaBufferAddr = NULL;

	memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
	Adapter.InterfaceType = Internal;
	Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
	// Allocate a block of virtual memory (physically contiguous) for the RX DMA buffers.
	m_pSpiVirtRxDmaBufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, m_dwDMABufferSize*CSPI_DMA_COUNT
		, &(m_pSpiPhysRxDMABufferAddr), FALSE);

	if (m_pSpiVirtRxDmaBufferAddr == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi: MapDMABuffers() - Failed to allocate RX DMA buffer.\r\n")));
		return(FALSE);
	}

	memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
	Adapter.InterfaceType = Internal;
	Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

	// Allocate a block of virtual memory (physically contiguous) for the TX DMA buffers.
	m_pSpiVirtTxDMABufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, m_dwDMABufferSize*CSPI_DMA_COUNT
		, &(m_pSpiPhysTxDMABufferAddr), FALSE);

    // Check if DMA buffer has been allocated
	if (m_pSpiVirtTxDMABufferAddr == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi: MapDMABuffers() - Failed to allocate TX DMA buffer.\r\n")));
		return(FALSE);
	}

	DEBUGMSG(ZONE_FUNCTION,(_T("Cspi::MapDMABuffers-\r\n")));
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
  	DEBUGMSG(ZONE_FUNCTION,(_T("Cspi::UnmapDMABuffers+\r\n")));
  
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

	DEBUGMSG(ZONE_FUNCTION,(_T("Cspi::InitChannelDMA+\r\n")));

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
	DEBUGMSG(ZONE_DMA,(_T("Channel Allocated(Rx) : %d\r\n"),m_dmaChanCspiRx));
	if (!m_dmaChanCspiRx)
	{
		DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): SdmaOpenChan for input failed.\r\n")));
		goto cleanUp;
	}

	// Allocate DMA chain buffer
	if (!DDKSdmaAllocChain(m_dmaChanCspiRx, CSPI_DMA_COUNT))
	{
		DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaAllocChain for input failed.\r\n")));
		goto cleanUp;
	}  

	// Open virtual DMA channels 
	
	m_dmaChanCspiTx = DDKSdmaOpenChan(m_dmaReqTx, dmaChannelPriority[0]); /// NULL, CspCSPIGetIRQ(Index));
	DEBUGMSG(ZONE_DMA,(_T("Channel Allocated(Tx) : %d\r\n"),m_dmaChanCspiTx));
	if (!m_dmaChanCspiTx)
	{
		DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): SdmaOpenChan(Rx) for input failed.\r\n")));
		goto cleanUp;
	}

	// Allocate DMA chain buffer
	if (!DDKSdmaAllocChain(m_dmaChanCspiTx, CSPI_DMA_COUNT))
	{
		DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): DDKSdmaAllocChain for input failed.\r\n")));
		goto cleanUp;
	}  

	RETAILMSG( 1, (TEXT("InitChannelDMA: \r\nm_dmaChanCspiTx:%d m_dmaChanCspiRx:%d\r\n"), m_dmaChanCspiTx, m_dmaChanCspiRx ));
	rc = TRUE;

cleanUp:
	if (!rc)
	{
		DeinitChannelDMA();
	}
	DEBUGMSG(ZONE_DMA,(_T("Cspi::InitChannelDMA-\r\n")));
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

	DEBUGMSG(ZONE_FUNCTION, (TEXT("Cspi: InitDMA+\r\n")));

	m_dmaReqTx = CspCSPIGetDmaReqTx(Index) ; 
	m_dmaReqRx = CspCSPIGetDmaReqRx(Index) ; 
	RETAILMSG( 1, (TEXT("InitCspiDMA: \r\nm_dmaReqTx:%d m_dmaReqRx:%d\r\n"), m_dmaReqTx, m_dmaReqRx ));

	// Map the DMA buffers into driver's virtual address space
	if (!MapDMABuffers())
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi:InitDMA() - Failed to map DMA buffers.\r\n")));
		return FALSE;
	}

	// Initialize the DMA channel
	if (!InitChannelDMA(Index))
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi:InitDMA() - Failed to initialize output DMA.\r\n")));
		return FALSE;
	}

	DEBUGMSG(ZONE_FUNCTION, (TEXT("Cspi::InitDMA-\r\n")));
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

	DEBUGMSG(ZONE_THREAD, (TEXT("CspiADCConfig: &m_pCSPI=0x%x\r\n"),&m_pCSPI));
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
		DEBUGMSG(ZONE_WARN, (TEXT("CspiMasterDataExchange:  unsupported bitcount!\r\n")));
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

	RETAILMSG( 1, (TEXT("m_pCSPI->CONREG=0x%x\r\n"),INREG32(&m_pCSPI->CONREG)));

	if(m_bUseLoopBack)
	{
		INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_CONN);
	}
	else
	{
		INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_NOCONN);
	}

	// 	//CSPI pin Config in CSPI master mode
	BAPCSPIRDY2IO( m_Index );
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
	DEBUGMSG(ZONE_THREAD, (TEXT("CspiDataExchange -\r\n")));
	
	BSPCSPIReleaseIOMux( m_Index );

	return xchRxCnt;
}

void spiClass::PreTxBuf(  )
{
	DWORD dwIdx,dwCount;

	memset( m_pSPITxBuf, 0, m_dwDMABufferSize );
	dwCount = m_dwDMABufferSize/sizeof( UINT16 );
	for( dwIdx=0; dwIdx<dwCount ; )
	{
		m_pSPITxBuf[dwIdx++] = (UINT16)(ADS8201_REG_WRITE|ADS8021_CHA_SEL_CCR|m_ADChannle[m_dwADChannleIdx]);	
		//m_pSPITxBuf[dwIdx++] = (UINT16)(ADS8201_REG_READ|ADS8021_CHA_SEL_CCR);
		dwIdx++;
		m_dwADChannleIdx = (m_dwADChannleIdx+1)%m_dwADChannleCount;
	}

	memcpy( m_pSpiVirtTxDMABufferAddr, (PBYTE)m_pSPITxBuf, m_dwDMABufferSize );
	memcpy( m_pSpiVirtTxDMABufferAddr+m_dwDMABufferSize, (PBYTE)m_pSPITxBuf, m_dwDMABufferSize );
}

/*UINT16 spiClass::TransferTxBuf(  UINT8 bufIdx )
{
	UINT16 dwIdx = 0;
	UINT16 DMABytesCount = 0;

	memset( m_pSPITxBuf, 0, m_dwDMABufferSize );
	while( m_dwSendTxByteCount < m_dwSpiTxDmaCount )
	{
		//Config channel
		//m_pSPITxBuf[dwIdx++] = ((UINT32)(ADS8201_REG_WRITE|ADS8021_CHA_SEL_CCR|m_ADChannle[m_dwADChannleIdx]))<<4;	
		//dwIdx++;
		//m_pSPITxBuf[dwIdx++] = (UINT32)(ADS8201_REG_WRITE|ADS8021_CHA_SEL_CCR|m_ADChannle[m_dwADChannleIdx]);	
		//m_pSPITxBuf[dwIdx++] = (UINT32)(ADS8201_REG_READ|ADS8021_CHA_SEL_CCR);
		//m_dwSendTxByteCount += 8;
		//DMABytesCount = dwIdx<<2;		// DMABytesCount = dwIdx*sizeof(UINT32);

		m_pSPITxBuf[dwIdx++] = (UINT16)(ADS8201_REG_WRITE|ADS8021_CHA_SEL_CCR|m_ADChannle[m_dwADChannleIdx]);	
		m_pSPITxBuf[dwIdx++] = (UINT16)(ADS8201_REG_READ|ADS8021_CHA_SEL_CCR);
		m_dwSendTxByteCount += 4;
		DMABytesCount = dwIdx<<1;		// DMABytesCount = dwIdx*sizeof(UINT16);

		m_dwADChannleIdx = (m_dwADChannleIdx+1)%m_dwADChannleCount;
		
		if( DMABytesCount >= m_dwDMABufferSize )
			break;
	}
	
	if( DMABytesCount )
	{
		//RETAILMSG( 1, (TEXT("TransferTxBuf: nCurrTxDmaBufIdx=0x%x,DMABytesCount=0x%x, m_dwSendTxByteCount=0x%x, m_dwDMABufferSize=0x%x\r\n"),
		//	bufIdx,DMABytesCount, m_dwSendTxByteCount, m_dwDMABufferSize ));

		memcpy( m_pSpiVirtTxDMABufferAddr+bufIdx*m_dwDMABufferSize, (PBYTE)m_pSPITxBuf, DMABytesCount );
		
  		//for( int i=0; i<dwIdx; i++ )
  		//	RETAILMSG( 1, (TEXT("0x%x "), m_pSPITxBuf[i]));
  		//RETAILMSG( 1, (TEXT("\r\n")));
	}

	return DMABytesCount;
}*/

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

	DEBUGMSG(ZONE_THREAD, (TEXT("CspiADCRun: &m_pCSPI=0x%x\r\n"),&m_pCSPI));
	
	//check for parameter
	if( pADSConfig->dwSamplingRate>100000 || pADSConfig->dwSamplingRate<1 ) 
	{
		RETAILMSG( 1, (TEXT("CspiADCRun:The shampling rate out of range!\r\n")));
		goto error_adcRun;
	}

	//EM9170's CSPI in slave mode, spi_cs and spi_sclk supported by the CPLD of the ETA108.
	BSPCSPIRDYSet( m_Index, FALSE );

	pBusCnfg = pXchPkt->pBusCnfg;
	// Enable the clock gating
	BSPCSPIEnableClock(m_Index,TRUE);

	// disable all CSPI interrupts
	OUTREG32(&m_pCSPI->INTREG, 0);

	// set client CSPI bus configuration based
	//  default EN = disabled
	//  default MODE = master
	//  default XCH = idle
	//  default SMC = XCH bit controls master start transfer
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


	//INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_SWAP, CSPI_TESTREG_SWAP_ENABLE);

	RETAILMSG( 1, (TEXT("m_pCSPI->CONREG=0x%x\r\n"),INREG32(&m_pCSPI->CONREG)));

	// enable the CSPI
	INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
		CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_ENABLE));

	//CSPI pin Config in CSPI slave mode
	BAPCSPIRDY2IO( m_Index );
	BSPCSPIRDYSet( m_Index, FALSE );
	BSPCSPISetIOMux( m_Index );


	switch(pADSConfig->dwSamplingLength )
	{
	case 0: return 0;

	case -1:   // Continuous sampling
		m_nSamplingMode = SAMPLING_MODE_CONTINUOUS;
		//Fixed AD(SPI) buffer length = CSPI_SDMA_BUFFER_SIZE*4
		m_dwSamplingLength = m_dwDMABufferSize;
		break;

	default:	// Manual sampling
		m_nSamplingMode = SAMPLING_MODE_MANUAL;
		m_dwSamplingLength = pADSConfig->dwSamplingLength;
	}

	m_pSPITxBuf = (UINT16 *)HeapAlloc( m_hHeap, HEAP_ZERO_MEMORY,  m_dwDMABufferSize );
	if( m_pSPITxBuf == NULL )
		goto error_adcRun;

	// Set SPI transfer count in uint16.
	m_dwSpiXchCount = m_dwSamplingLength<<1;	
	// Set SPI TX DMA count in bytes.
	m_dwSpiTxDmaCount = m_dwSpiXchCount*sizeof( UINT16 );

	m_dwSpiRxDmaCount = m_dwSpiXchCount*sizeof( UINT16 );

	m_pSPIRxBuf = (UINT16 *)HeapAlloc( m_hHeap, HEAP_ZERO_MEMORY,  m_dwSpiRxDmaCount );
	if( m_pSPIRxBuf == NULL )
		goto error_adcRun;

	m_dwSpiRxDmaCount = (m_dwSpiRxDmaCount/CSPI_DMA_WATERMARK_RX)*CSPI_DMA_WATERMARK_RX;
	
	RETAILMSG( 1, (TEXT("m_dwSpiTxDmaCount = 0x%x \r\n") ,m_dwSpiTxDmaCount) );

// 	RETAILMSG( 1, (TEXT("Sampling Channel Count = %d\r\n"),m_dwADChannleCount ));
// 	for( i=0; i<m_dwADChannleCount; i++ )
// 		RETAILMSG( 1, (TEXT("%d "),m_ADChannle[i]));
// 	RETAILMSG( 1, (TEXT("\r\n")));

	//Prepare Sampling Channel
	for( m_dwADChannleIdx=0, m_dwADChannleCount=0, i=0; i<8; i++ )
	{
		if( pADSConfig->dwSamplingChannel & (0x01<<i))
			m_ADChannle[m_dwADChannleCount++] = (UINT8)i;
	}
	m_dwADChannleIdx = 0;		//ADC channle idx reset

	PreTxBuf( );

	m_hTransferDoneEvent = CreateEvent( NULL, FALSE, FALSE, pXchPkt->xchEvent );
	if( m_hTransferDoneEvent == NULL )
		goto error_adcRun;

	// ensure RXFIFO is empty before start DMA
	while ( INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR))
	{
		INREG32(&m_pCSPI->RXDATA);
	}

	m_dwSendTxByteCount = m_dwSpiTxDmaCount;
	m_dwSendTxDMAByteCount = 0;
	m_dwAvailRxByteCount = 0;
	m_dwReceiveByteCount = m_dwSpiRxDmaCount;

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
		transferBytes = (UINT16)MIN_VAL( m_dwDMABufferSize, m_dwReceiveByteCount );
		if( transferBytes> 0 )
		{
			DDKSdmaSetBufDesc(m_dmaChanCspiRx,
				nBufIdx,
				(Mode|DDK_DMA_FLAGS_INTR),
				(m_pSpiPhysRxDMABufferAddr.LowPart) +nBufIdx * m_dwDMABufferSize,
				0,
				DDK_DMA_ACCESS_16BIT,
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
		//transferBytes = TransferTxBuf( nBufIdx );
		transferBytes = (UINT16)MIN_VAL( m_dwDMABufferSize, m_dwSendTxByteCount );
		if( transferBytes>0 )
		{
			DDKSdmaSetBufDesc(m_dmaChanCspiTx,
				nBufIdx,
				(Mode|DDK_DMA_FLAGS_INTR),
				(m_pSpiPhysTxDMABufferAddr.LowPart) + nBufIdx * m_dwDMABufferSize,
				0,
				DDK_DMA_ACCESS_16BIT,
				transferBytes );	// Set the count in bytes
		}
		m_dwSendTxByteCount -= transferBytes;
	}

	if (!DDKSdmaInitChain(m_dmaChanCspiRx, CSPI_DMA_WATERMARK_RX))
	{
		DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaInitChain failed.\r\n")));
		goto error_adcRun;
	}
	// start Rx DMA channel
	DDKSdmaStartChan(m_dmaChanCspiRx);

	if (!DDKSdmaInitChain(m_dmaChanCspiTx, CSPI_DMA_WATERMARK_TX))
	{
		DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaInitChain failed.\r\n")));
		goto error_adcRun;
	}
	// start Tx DMA channel
	DDKSdmaStartChan(m_dmaChanCspiTx);
	
	// load Tx FIFO until enough for XCH, or until we run out of data
	while (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TE))
	{
		RETAILMSG( 1, (TEXT("m_dmaChanCspiTx:Wait data...\r\n")));
		Sleep(1000) ;
	}

	//RETAILMSG( 1, (TEXT("m_pCSPI->TESTREG=0x%x\r\n"),INREG32(&m_pCSPI->TESTREG)));

	// enable TC interrupt
	//INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_TCEN), 
	//	CSP_BITFVAL(CSPI_INTREG_TCEN, CSPI_INTREG_TCEN_ENABLE));

	// start exchange
	//INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_XCH), 
	//	CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_EN));

	//RETAILMSG( 1, (TEXT("m_dmaReqTx=%d, m_dmaReqRx=%d\r\n"), m_dmaReqTx,m_dmaReqRx) );
	RETAILMSG( 1, (TEXT("m_dmaChanCspiTx:%d m_dmaChanCspiRx:%d\r\n"), m_dmaChanCspiTx, m_dmaChanCspiRx ));

	return TRUE;

error_adcRun:
	CspiADCDone( );
	return FALSE;
}

void spiClass::CspiProcess(  )
{

	UINT32 dwTmp, Mode;
	UINT8  nBufIdx;
	UINT32 bufDescStatus[CSPI_DMA_COUNT], status;
	UINT16	transferBytes;	
	BOOL    bStartTxDMA, bStartRxDMA;
//	try
//	{
		while( !m_bTerminate )
		{
			// wait for requested transfer interrupt
			WaitForSingleObject(m_hIntrEvent, INFINITE);
			if( m_bTerminate )
				break;

			// disable all CSPI interrupts
			//OUTREG32(&m_pCSPI->INTREG, 0);

			//RETAILMSG( 1, (TEXT("\r\nCspiProcess: m_dwSpiTxDmaCount=0x%x\r\n"),m_dwSpiTxDmaCount ));
			//RETAILMSG( 1, (TEXT("CspiProcess:m_pCSPI->STATREG=0x%x\r\n"),INREG32(&m_pCSPI->STATREG)));

			// Check RX DMA interrupt
			// attempt to fill the RX Buffer(m_pSPIRxBuf) as much ad possible, by copying from currently  
			// IDX SDMA buffer. update m_dwAvailRxByteCount and m_currRxDmaBufIdx.
			//DDKSdmaGetChainStatus( m_dmaChanCspiRx, bufDescStatus );
			bStartRxDMA = FALSE;
			DDKSdmaGetChainStatus( m_dmaChanCspiRx, bufDescStatus );
			for (nBufIdx = 0; nBufIdx < CSPI_DMA_COUNT; nBufIdx++)
			{
				//DDKSdmaGetBufDescStatus ( m_dmaChanCspiRx, nBufIdx, &status);
				status = bufDescStatus[nBufIdx];
				if ((status & DDK_DMA_FLAGS_BUSY) == 0 )
				{
					DDKSdmaClearBufDescStatus(m_dmaChanCspiRx, nBufIdx);
					//RETAILMSG( 1, (TEXT("CspiProcess: RX BUFFER_%d. status = 0x%x\r\n"), nBufIdx, status ));
					status &= 0xffff;
					if( status>0 )
					{
						// SPI Receive buffer size equal to m_dwSpiTxDmaCount.
						memcpy( (PBYTE)m_pSPIRxBuf+m_dwAvailRxByteCount, m_pSpiVirtRxDmaBufferAddr+nBufIdx*m_dwDMABufferSize, status );

						//for( int j=0; j<20; j++ )
						//	RETAILMSG( 1, (TEXT(" 0x%x"), *((PBYTE)m_pSPIRxBuf+m_dwAvailRxByteCount+j )));
						//RETAILMSG( 1, (TEXT("\r\n")));

						m_dwAvailRxByteCount += status;
						//RETAILMSG( 1, (TEXT("m_dwAvailRxByteCount=0x%x,m_dwReceiveByteCount=0x%x\r\n"), m_dwAvailRxByteCount, m_dwReceiveByteCount));
					}

					if( m_dwAvailRxByteCount >= m_dwSpiRxDmaCount )
					{
						BSPCSPIReleaseIOMux( m_Index );
						DDKSdmaStopChan(m_dmaChanCspiRx, TRUE);  //force stop Rx DMA

						RETAILMSG( 1, (TEXT("CspiProcess: Stop rx dma.RX DMA size:0x%x\r\n"), m_dwAvailRxByteCount));
						// while there is data in Rx FIFO 
						m_dwAvailRxByteCount <<= 1; //m_dwAvailRxByteCount /= sizeof(UINT16);
						while (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR))
						{
							// increment Rx Buffer to next data point
							dwTmp = INREG32(&m_pCSPI->RXDATA);
							RETAILMSG( 1, (TEXT(" 0x%x"), dwTmp));
							++m_dwAvailRxByteCount;

							if( m_dwAvailRxByteCount < m_dwSpiXchCount )
								CspiBufWrt16(m_pSPIRxBuf+(m_dwAvailRxByteCount%m_dwSpiXchCount), dwTmp);			
						}

						// disable the CSPI
						INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
							CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE));

						BSPCSPIEnableClock(m_Index,FALSE);

						SetEvent( m_hTransferDoneEvent );

						RETAILMSG( 1, (TEXT("CspiProcess: Transfer completed. Receive:0x%x\r\n\r\n"), m_dwAvailRxByteCount));
						break;
					}
					
					transferBytes = (UINT16)MIN_VAL( m_dwDMABufferSize, m_dwReceiveByteCount );
					if( transferBytes>0 )
					{
							
						Mode = nBufIdx ? DDK_DMA_FLAGS_WRAP : DDK_DMA_FLAGS_CONT;
						DDKSdmaSetBufDesc(m_dmaChanCspiRx,
							nBufIdx,
							(Mode|DDK_DMA_FLAGS_INTR),
							(m_pSpiPhysRxDMABufferAddr.LowPart) +
							nBufIdx * m_dwDMABufferSize,
							0,
							DDK_DMA_ACCESS_16BIT,
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
						RETAILMSG( 1, (TEXT("CspiProcess: Stop tx dma.\r\n")));
						// enable TC interrupt
						INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_TCEN), 
							CSP_BITFVAL(CSPI_INTREG_TCEN, CSPI_INTREG_TCEN_ENABLE));
						break;
					}
					Mode = nBufIdx ? DDK_DMA_FLAGS_WRAP : DDK_DMA_FLAGS_CONT;
					//transferBytes = TransferTxBuf( nBufIdx );
					//memcpy( m_pSpiVirtTxDMABufferAddr+bufIdx*m_dwDMABufferSize, (PBYTE)m_pSPITxBuf, m_dwDMABufferSize );
					transferBytes = (UINT16)MIN_VAL( m_dwDMABufferSize, m_dwSendTxByteCount );
 					//for( int i=0; i<10; i++ )
 					//	RETAILMSG( 1, (TEXT(" 0x%x"), *(m_pSpiVirtTxDMABufferAddr+nBufIdx*m_dwDMABufferSize+i )));
 					//RETAILMSG( 1, (TEXT("\r\n")));
					if( transferBytes>0 )
					{
						DDKSdmaSetBufDesc(m_dmaChanCspiTx,
							nBufIdx,
							(Mode|DDK_DMA_FLAGS_INTR),
							(m_pSpiPhysTxDMABufferAddr.LowPart) +
							nBufIdx * m_dwDMABufferSize,
							0,
							DDK_DMA_ACCESS_16BIT,
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
//	}
// 	except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
// 		EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
// 		DEBUGMSG(ZONE_WARN, (TEXT("SL_RxIntrHandler :Exception caught \n")));

}

void spiClass::CspiADCDone()
{
	RETAILMSG( 1, (TEXT("m_pCSPI->STATREG=0x%x\r\n"),INREG32(&m_pCSPI->STATREG)));
	RETAILMSG( 1, (TEXT("m_pCSPI->CONREG=0x%x\r\n"),INREG32(&m_pCSPI->CONREG)));

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

	if( m_hTransferDoneEvent!= NULL )
	{
		CloseHandle( m_hTransferDoneEvent );
		m_hTransferDoneEvent = NULL;
	}
}