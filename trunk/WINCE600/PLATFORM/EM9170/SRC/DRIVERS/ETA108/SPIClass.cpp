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

//lqk Sep 13,2011
extern "C" void BSPCSPICS2IO( void );
extern "C" void BSPCSPICSSet(BOOL bVal);

//extern DWORD GetSdmaChannelIRQ(UINT32 chan);
//
//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
#define  CSPI_DMA_WATERMARK_RX		4
#define  CSPI_DMA_WATERMARK_TX		4
#define  CSPI_DMA_COUNT				2

#define  SAMPLING_MODE_MANUAL		1
#define  SAMPLING_MODE_CONTINUOUS	2

#define MIN_VAL(a, b)   (((a) > (b)) ? (b) : (a))
//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions
spiClass::spiClass()
{
	m_pCSPI = NULL;
	m_hHeap = NULL;
	m_hIntrEvent = NULL;
	//m_hEnQEvent = NULL;

}

spiClass::~spiClass()
{

}

BOOL spiClass::CspiInitialize(DWORD Index)
{
	PHYSICAL_ADDRESS phyAddr;
	DWORD irq[5];

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
	// Map peripheral physical address to virtual address
	m_pCSPI = (PCSP_CSPI_REG) MmMapIoSpace(phyAddr, sizeof(CSP_CSPI_REG),FALSE);

	// Check if virtual mapping failed
	if (m_pCSPI == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  MmMapIoSpace failed!\r\n")));
		goto Init_Error;
	}

	//initialize DMA
	InitCspiDMA(Index);
	BSPCspiAcquireGprBit((UINT8)Index);


	irq[0] = (DWORD)-1;
	irq[1] = 0;
	//irq[2] = GetSdmaChannelIRQ( m_dmaChanCspiRx );
	//irq[3] = GetSdmaChannelIRQ( m_dmaChanCspiTx );
	//Get the IRQ according to Index
	irq[4] = CspCSPIGetIRQ(Index);

	// Call the OAL to translate the IRQ into a SysIntr value.
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(irq),
		&m_dwSysIntr, sizeof(DWORD), NULL))
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

	// create CSPI critical section
	InitializeCriticalSection(&m_cspiCs);

	// create CSPI Data Exchange critical section
	InitializeCriticalSection(&m_cspiDataXchCs);

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
		m_hThread = ::CreateThread(NULL, 0, CspiProcess, this, 0, NULL);
		DEBUGMSG(ZONE_INIT, (TEXT("CspiInitialize:  this=0x%x\r\n"),this));

		// check if CreateThread failed
		if (m_hThread == NULL)
		{
			DEBUGMSG(ZONE_ERROR, (TEXT("CspiInitialize:  CreateThread failed!\r\n")));
			goto Error;
		}
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
		// try to signal the thread so that it can wake up and terminate
		if (m_hEnQEvent)
		{
			SetEvent(m_hEnQEvent);
		}
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

	// close enqueue event handle
	if (m_hEnQEvent)
	{
		CloseHandle(m_hEnQEvent);
		m_hEnQEvent = NULL;
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

	// delete the critical section
	DeleteCriticalSection(&m_cspiCs);

	// delete the Data Exchange critical section
	DeleteCriticalSection(&m_cspiDataXchCs);

	DEBUGMSG(ZONE_CLOSE, (TEXT("CspiRelease -\r\n")));
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
BOOL spiClass::CspiIOMux( void )
{
	return BSPCSPISetIOMux( m_Index );
}

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

	// Allocate a block of virtual memory (physically contiguous) for the TX DMA buffers.
	m_pSpiVirtTxDMABufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, (CSPI_SDMA_BUFFER_SIZE)*2
		, &(m_pSpiPhysTxDMABufferAddr), FALSE);

    // Check if DMA buffer has been allocated
	if (m_pSpiVirtTxDMABufferAddr == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi: MapDMABuffers() - Failed to allocate TX DMA buffer.\r\n")));
		return(FALSE);
	}

	// Allocate a block of virtual memory (physically contiguous) for the RX DMA buffers.
	m_pSpiVirtRxDmaBufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, (CSPI_SDMA_BUFFER_SIZE)*2
		, &(m_pSpiPhysRxDMABufferAddr), FALSE);

	if (m_pSpiVirtRxDmaBufferAddr == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi: MapDMABuffers() - Failed to allocate RX DMA buffer.\r\n")));
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

	// Initialize the chain and set the watermark level     
	if (!DDKSdmaInitChain(m_dmaChanCspiRx, CSPI_DMA_WATERMARK_RX))
	{
		DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaInitChain failed.\r\n")));
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

	// Initialize the chain and set the watermark level 
	if (!DDKSdmaInitChain(m_dmaChanCspiTx, CSPI_DMA_WATERMARK_TX))
	{
		DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): DDKSdmaInitChain failed.\r\n")));
		goto cleanUp;
	}

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

//-----------------------------------------------------------------------------
//
// Function: CspiExchangeSize
//
// This function returns data bytes count in every exchange Packet. 
//
// Parameters:
//      pXchPkt
//          [in] Points to exchange packet information.
//
// Returns:
//      Returns data size of exchange packet.
//
//-----------------------------------------------------------------------------
// UINT32 spiClass::CspiExchangeSize(PCSPI_XCH_PKT0_T pXchPkt)
// {
// 	PCSPI_BUSCONFIG_T pBusCnfg = pXchPkt->pBusCnfg;
// 
// 	if ((pBusCnfg->bitcount >= 1) && (pBusCnfg->bitcount <= 8))
// 	{
// 		// 8-bit access width
// 		return pXchPkt->xchCnt *sizeof(UINT8);
// 	}
// 	else if ((pBusCnfg->bitcount >= 9) && (pBusCnfg->bitcount <= 16))
// 	{
// 		// 16-bit access width
// 		return pXchPkt->xchCnt *sizeof(UINT16);
// 	}
// 	else if ((pBusCnfg->bitcount >= 17) && (pBusCnfg->bitcount <= 32))
// 	{
// 		// 32-bit access width
// 		return pXchPkt->xchCnt *sizeof(UINT32);
// 	}
// 	else
// 	{
// 		// unsupported access width
// 		return 0;
// 	}
// }

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
// Function: CspiNonDMADataExchange
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
DWORD spiClass::CspiNonDMADataExchange(PCSPI_XCH_PKT_T pXchPkt)
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
	UINT32 byParamBurstLen;
	BOOL  bSsctl = FALSE;

	DEBUGMSG(ZONE_THREAD, (TEXT("CspiDataExchange: &m_pCSPI=0x%x\r\n"),&m_pCSPI));
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
	if( !pBusCnfg->ssctl )
	{
		pBusCnfg->ssctl = TRUE;
		bSsctl = TRUE;
		BSPCSPICS2IO( );
	}

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
	byParamSSCTL = pBusCnfg->ssctl? CSPI_CONREG_SSCTL_PULSE: CSPI_CONREG_SSCTL_ASSERT;
	byParamPOL = pBusCnfg->pol? CSPI_CONREG_POL_ACTIVE_LOW: CSPI_CONREG_POL_ACTIVE_HIGH;
	byParamPHA = pBusCnfg->pha? CSPI_CONREG_PHA1: CSPI_CONREG_PHA0;
	byParamDRCTL = pBusCnfg->drctl &((1U<<CSPI_CONREG_DRCTL_WID) -1);
	bReqPolling = (m_bAllowPolling && pBusCnfg->usepolling)?TRUE: FALSE;

	//lqk Sep 6, 2011
	byParamBurstLen = pBusCnfg->ssctl ? pBusCnfg->bitcount:( pBusCnfg->bitcount*pXchPkt->xchCnt );
	//RETAILMSG (1, (TEXT("\r\nbyParamBurstLen=%d\r\n"),byParamBurstLen ));

	//end //lqk sep 6,2011
	OUTREG32(&m_pCSPI->CONREG, 
		CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE) |
		CSP_BITFVAL(CSPI_CONREG_MODE, CSPI_CONREG_MODE_MASTER) |
		CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_IDLE) |
		CSP_BITFVAL(CSPI_CONREG_SMC, CSPI_CONREG_SMC_XCH) |
		CSP_BITFVAL(CSPI_CONREG_CHIPSELECT, byParamChipSelect) |
		CSP_BITFVAL(CSPI_CONREG_DATARATE, BSPCSPICalculateDivRate(pBusCnfg->freq,pBusCnfg->freq/10)) |
		CSP_BITFVAL(CSPI_CONREG_SSPOL, byParamSSPOL) |
		CSP_BITFVAL(CSPI_CONREG_SSCTL, byParamSSCTL) |  //pBusCnfg->ssctl) |
		CSP_BITFVAL(CSPI_CONREG_POL, byParamPOL) |
		CSP_BITFVAL(CSPI_CONREG_PHA, byParamPHA) |
		//lqk sep 6,2011
		//CSP_BITFVAL(CSPI_CONREG_BITCOUNT, pBusCnfg->bitcount-1) |
		CSP_BITFVAL(CSPI_CONREG_BITCOUNT, byParamBurstLen-1) |
		//end //lqk sep 6,2011
		CSP_BITFVAL(CSPI_CONREG_DRCTL, byParamDRCTL));

	// enable the CSPI
	INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
		CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_ENABLE));

	if(m_bUseLoopBack)
	{
		INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_CONN);
	}
	else
	{
		INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_NOCONN);
	}

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
			if( bSsctl )
			{
				BSPCSPICSSet( FALSE );
				//RETAILMSG (1, (TEXT("to Low\r\n")));
			}
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
			if(m_bUsePolling || bReqPolling)
			{
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
			}
			else
			{
				if (bSsctl==1)
				{
					// wait until RH, but wait RR is also OK. 
					INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_RHEN), 
						CSP_BITFVAL(CSPI_INTREG_RHEN, CSPI_INTREG_RHEN_ENABLE));
				}
				else
				{
					// wait until RH, but wait RR is also OK. 
					INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_RREN), 
						CSP_BITFVAL(CSPI_INTREG_RREN, CSPI_INTREG_RREN_ENABLE));
				}

				WaitForSingleObject(m_hIntrEvent, INFINITE);

				// disable all interrupts
				OUTREG32(&m_pCSPI->INTREG, 0);

				while (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR))
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

					OUTREG32(&m_pCSPI->TXDATA, pfnTxBufRd(pTxBuf));

					// increment Tx Buffer to next data point
					pTxBuf = (LPVOID) ((UINT) pTxBuf + bufIncr);

					// increment Tx exchange counter
					xchTxCnt++;

					if (xchTxCnt >= xchCnt)
					{
						break;
					}
				}
				// signal that interrupt has been handled
				InterruptDone(m_dwSysIntr);
			}

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

			// Wait all data in the chain exchaged
			if(!(m_bUsePolling || bReqPolling))
			{
				INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_TCEN), 
					CSP_BITFVAL(CSPI_INTREG_TCEN, CSPI_INTREG_TCEN_ENABLE));

				WaitForSingleObject(m_hIntrEvent, INFINITE);

				// disable all interrupts
				OUTREG32(&m_pCSPI->INTREG, 0);
			}
			else
			{
				// wait until transaction is complete
				while (!(INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC)))
					;
			}

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

			if(!(m_bUsePolling|| bReqPolling))
			{
				// signal that interrupt has been handled
				InterruptDone(m_dwSysIntr);
			}

			if (xchRxCnt >= xchCnt)
			{
				//lqk Sep 13, 2011
				if( bSsctl )
				{
					BSPCSPICSSet( TRUE );
					pBusCnfg->ssctl = FALSE;
					//RETAILMSG (1, (TEXT("to High\r\n")));
				}
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

	return xchRxCnt;
}

VOID spiClass::TransferTxBuf( UINT8 nNumBuf )
{
	DWORD dwIdx;

	//To Prepare TX buffer
	for( dwIdx=0,m_dwADChannleIdx=0; dwIdx<CSPI_SDMA_BUFFER_SIZE*2; )
	{
		//Read ADC Data
		m_pSpiVirtTxDMABufferAddr[dwIdx + nNumBuf*CSPI_SDMA_BUFFER_SIZE] = ADS8201_ADC_READ;
		dwIdx++;
		//Config channel
		m_pSpiVirtTxDMABufferAddr[dwIdx + nNumBuf*CSPI_SDMA_BUFFER_SIZE] = 
			ADS8201_REG_WRITE|ADS8021_CHA_SEL_CCR|m_ADChannle[m_dwADChannleIdx++];	
		dwIdx++;
		m_dwADChannleIdx %= m_dwADChannleCount;
	}
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

	DWORD i, k, dwIdx;
	UINT32 Mode, DMACount;

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
		CSP_BITFVAL(CSPI_CONREG_MODE, CSPI_CONREG_MODE_MASTER) |
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

	// enable the CSPI
	INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
		CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_ENABLE));

	if(m_bUseLoopBack)
	{
		INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_CONN);
	}
	else
	{
		INSREG32BF(&m_pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_NOCONN);
	}


	switch(pADSConfig->dwSamplingLength )
	{
	case 0: return 0;

	case -1:   // Continuous sampling
		m_nSamplingMode = SAMPLING_MODE_CONTINUOUS;
		//Fixed AD(SPI) buffer length = CSPI_SDMA_BUFFER_SIZE*4
		m_dwSamplingLength = CSPI_SDMA_BUFFER_SIZE;
		break;

	default:	// Manual sampling
		m_nSamplingMode = SAMPLING_MODE_MANUAL;
		m_dwSamplingLength = pADSConfig->dwSamplingLength;
	}

	// Set SPI transfer count in uint16.
	m_dwSpiXchCount = m_dwSamplingLength<<1;	
	// Set SPI DMA count in bytes.
	m_dxSpiDmaCount = m_dwSpiXchCount<<1;
	m_pSPIRxBuf = (UINT16 *)HeapAlloc( m_hHeap, HEAP_ZERO_MEMORY,  m_dxSpiDmaCount );
	if( m_pSPIRxBuf == NULL )
		goto error_adcRun;

	//Prepare Sampling Channel
	for( m_dwADChannleCount=0, i=0; m_dwADChannleCount<8; i++ )
	{
		if( pADSConfig->dwSamplingChannel & (0x01<<i))
			m_ADChannle[m_dwADChannleCount++] = (UINT8)i;
	}

	// ensure RXFIFO is empty before start DMA
	while ( INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR))
	{
		INREG32(&m_pCSPI->RXDATA);
	}

	// assert DMA request for half or empty
	// irrespective of the condition, 
	// we need to enable the transmitter
	OUTREG32(&m_pCSPI->DMAREG, 
		//CSP_BITFMASK(CSPI_DMAREG_THDEN)|CSP_BITFMASK(CSPI_DMAREG_RHDEN), 
		CSP_BITFVAL(CSPI_DMAREG_THDEN, CSPI_DMAREG_THDEN_ENABLE)|
		CSP_BITFVAL(CSPI_DMAREG_RHDEN, CSPI_DMAREG_RHDEN_ENABLE));

	// configure RX DMA
	
	for (m_currRxDmaBufIdx = 0; i < CSPI_DMA_COUNT; m_currRxDmaBufIdx++)
	{
		DDKSdmaClearBufDescStatus(m_dmaChanCspiRx,m_currRxDmaBufIdx);
		Mode = m_currRxDmaBufIdx ? DDK_DMA_FLAGS_WRAP : DDK_DMA_FLAGS_CONT;
		DDKSdmaSetBufDesc(m_dmaChanCspiRx,
			m_currRxDmaBufIdx,
			(DDK_DMA_FLAGS_INTR | Mode),
			(m_pSpiPhysRxDMABufferAddr.LowPart) +
			m_currRxDmaBufIdx * m_dwDMABufferSize,
			0,
			DDK_DMA_ACCESS_16BIT,
			m_dwDMABufferSize*szieof(UINT16));	// Set the count in bytes
	}
	m_currRxDmaBufIdx = 0;

	// configure TX DMA
	for (m_currTxDmaBufIdx = 0; m_currTxDmaBufIdx < CSPI_DMA_COUNT; m_currTxDmaBufIdx++)
	{
		DDKSdmaClearBufDescStatus(m_dmaChanCspiTx,0);
		DMACount = MIN_VAL( m_dwXchBufLen, CSPI_SDMA_BUFFER_SIZE*2);
		Mode = m_currTxDmaBufIdx ? DDK_DMA_FLAGS_WRAP : DDK_DMA_FLAGS_CONT;
		DDKSdmaSetBufDesc(m_dmaChanCspiTx,
			m_currTxDmaBufIdx,
			(DDK_DMA_FLAGS_INTR | Mode),
			(m_pSpiPhysTxDMABufferAddr.LowPart) +
			m_currTxDmaBufIdx * m_dwDMABufferSize,
			0,
			DDK_DMA_ACCESS_16BIT,
			DMACount );	// Set the count in bytes
		m_dwXchBufLen -= DMACount;
		if( m_dwXchBufLen==0 )
			break;
	}

	// start Rx DMA channel
	DDKSdmaStartChan(m_dmaChanCspiRx);

	TransferTxBuf(0);
	if( i<2 )
		TransferTxBuf(1);
	// start Tx DMA channel
	DDKSdmaStartChan(m_dmaChanCspiTx);

	// load Tx FIFO until enough for XCH, or until we run out of data
	while (INREG32(&m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TE))
		;
	// start exchange
	INSREG32(&m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_XCH), 
		CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_EN));

	// enable TC interrupts
	INSREG32(&m_pCSPI->INTREG, CSP_BITFMASK(CSPI_INTREG_TCEN), 
		CSP_BITFVAL(CSPI_INTREG_TCEN, CSPI_INTREG_TCEN_ENABLE));

	return TRUE;

error_adcRun:
	return FALSE;
}

DWORD WINAPI spiClass::CspiProcess( LPVOID lpParameter )
{
	spiClass *pSpi = (spiClass *)lpParameter;
	UINT32 dwStatus, dwTmp;

	while( !pSpi->m_bTerminate )
	{
		// wait for requested transfer interrupt
		WaitForSingleObject(pSpi->m_hIntrEvent, INFINITE);

		// disable all CSPI interrupts
		OUTREG32(&pSpi->m_pCSPI->INTREG, 0);

		//Check TC interrupt
		//transfer completed if TC interrupt.
		if( INREG32(&pSpi->m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TE))
		{
			// acknowledge transfer complete (w1c)
			OUTREG32(&pSpi->m_pCSPI->STATREG, CSP_BITFMASK(CSPI_STATREG_TC));	


			// while there is data in Rx FIFO 
			while (INREG32(&pSpi->m_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR))
			{
				// increment Rx Buffer to next data point
//				pSpi->m_pSPIRxBuf = (LPVOID) ((UINT) pSpi->m_pSPIRxBuf + 2);
				++pSpi->m_dwAvailRxByteCount;			
				dwTmp = INREG32(&pSpi->m_pCSPI->RXDATA);
				CspiBufWrt16(pSpi->m_pSPIRxBuf, dwTmp);			
			}

			DDKSdmaGetBufDescStatus(pSpi->m_dmaChanCspiRx, 0, &dwStatus);
			DDKSdmaGetBufDescStatus(pSpi->m_dmaChanCspiRx, 1, &dwStatus);
			DDKSdmaGetBufDescStatus(pSpi->m_dmaChanCspiTx, 0, &dwStatus);
			DDKSdmaGetBufDescStatus(pSpi->m_dmaChanCspiTx, 1, &dwStatus);

			// disable the CSPI
			INSREG32(&pSpi->m_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
				CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE));

			BSPCSPIEnableClock(pSpi->m_Index,FALSE);
			DEBUGMSG(ZONE_THREAD, (TEXT("CspiDataExchange -\r\n")));
		}

		// Check RX DMA interrupt
		// attempt to fill the RX Buffer(m_pSPIRxBuf) as much ad possible, by copying from currently  
		// IDX SDMA buffer. update m_dwAvailRxByteCount and m_currRxDmaBufIdx.
		DDKSdmaGetBufDescStatus(pSpi->m_dmaChanCspiRx, pSpi->m_currRxDmaBufIdx, &dwStatus);
		if( (dwStatus & DDK_DMA_FLAGS_BUSY) == 0 )
		{
			DDKSdmaClearBufDescStatus(pSpi->m_dmaChanCspiRx, pSpi->m_currRxDmaBufIdx );
			pSpi->m_dwAvailRxByteCount = dwStatus & 0xFFFF;
			memcpy( pSpi->m_pSPIRxBuf, pSpi->m_currRxDmaBufIdx*CSPI_SDMA_BUFFER_SIZE, pSpi->m_dwAvailRxByteCount*2);
			// Circularly advance m_currRxDmaBufIdx.
			pSpi->m_currRxDmaBufIdx = (pSpi->m_currRxDmaBufIdx + 1) % 2;
		}

		// Check TX DMA interrupt
		// attempt to fill the currently IDX SDMA buffer.update m_dwSendByteCount and m_currTxDmaBufIdx.
		DDKSdmaGetBufDescStatus(pSpi->m_dmaChanCspiTx, pSpi->m_currTxDmaBufIdx, &dwStatus);
		if ((dwStatus & DDK_DMA_FLAGS_BUSY) == 0)
		{
			
			pSpi->TransferTxBuf( pSpi->m_currTxDmaBufIdx );
			// Circularly advance m_currTxDmaBufIdx.
			pSpi->m_currTxDmaBufIdx = (pSpi->m_currTxDmaBufIdx + 1) % 2;
		}

		// signal that interrupt has been handled
		InterruptDone(pSpi->m_dwSysIntr);
	}
	return TRUE;
}