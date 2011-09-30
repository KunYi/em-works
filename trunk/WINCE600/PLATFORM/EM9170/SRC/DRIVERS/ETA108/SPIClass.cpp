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
//#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "SPIClass.h"

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
//
//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

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

}

spiClass::~spiClass()
{

}

BOOL spiClass::CspiInitialize(DWORD Index)
{
	//We may need specal operation since on some board the CSPI is also used by the OAL
	if(cspiClass::CheckPort()){
		return TRUE;
	}

	m_Index = Index;

	return TRUE;
}

void spiClass::CspiRelease()
{

}


//------------------------------------------------------------------------------
//
// Function: InitDMA
//
//  Performs DMA channel intialization
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
	Index = Index;

	m_dmaReqTx = CspCSPIGetDmaReqTx(Index) ; 
	m_dmaReqRx = CspCSPIGetDmaReqRx(Index) ; 

	// Map the DMA buffers into driver's virtual address space
	if (!MapDMABuffers())
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi:InitDMA() - Failed to map DMA buffers.\r\n")));
		return FALSE;
	}

	// Initialize the output DMA
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
UINT32 cspiClass::CspiBufRd8(LPVOID pBuf)
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
UINT32 cspiClass::CspiBufRd16(LPVOID pBuf)
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
UINT32 cspiClass::CspiBufRd32(LPVOID pBuf)
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
void cspiClass::CspiBufWrt8(LPVOID pBuf, UINT32 data)
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
void cspiClass::CspiBufWrt16(LPVOID pBuf, UINT32 data)
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
void cspiClass::CspiBufWrt32(LPVOID pBuf, UINT32 data)
{
	UINT32 *p;

	p = (UINT32 *) pBuf;

	*p = data;
}

// ----------------------------------------------------------------------------
// Function: MoveDMABuffer
//     Move data from s/g buffer to DMA buffer or vice versa
//
// Parameters:
//     pBuf 
//       Destination or Source buffer according to bReceive.
//
//     dwLen
//       Length of the buffer to be copied.
//
//     bReceive
//       Used to indicate Receive or Transmit.
//
//     Returns:
//       None.  
//     
// ----------------------------------------------------------------------------
VOID cspiClass::MoveDMABuffer(LPVOID pBuf, DWORD dwLen, BOOL bReceive)
{    
	DEBUGMSG(ZONE_FUNCTION, (_T("Cspi: MoveDMABuffer(%d)+\n"), dwLen));

	if (pBuf == NULL) 
	{
		// security violation
		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi: MoveDMABuffer Failed to map pointer to caller\r\n")));
		return;
	}
	if (bReceive)
		memcpy(pBuf, pVirtDMABufferAddr + CSPI_RECV_OFFSET, dwLen);
	else
		memcpy(pVirtDMABufferAddr + CSPI_TXMT_OFFSET, pBuf, dwLen);
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
BOOL cspiClass::MapDMABuffers(void)
{
	DMA_ADAPTER_OBJECT Adapter;
	DEBUGMSG(ZONE_FUNCTION,(_T("Cspi::MapDMABuffers+\r\n")));

	pVirtDMABufferAddr = NULL;

	memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
	Adapter.InterfaceType = Internal;
	Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

	// Allocate a block of virtual memory (physically contiguous) for the DMA buffers.
	pVirtDMABufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, (CSPI_SDMA_BUFFER_SIZE)
		, &(PhysDMABufferAddr), FALSE);

	if (pVirtDMABufferAddr == NULL)
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi: MapDMABuffers() - Failed to allocate DMA buffer.\r\n")));
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
BOOL cspiClass::UnmapDMABuffers(void)
{
	DMA_ADAPTER_OBJECT Adapter;
	DEBUGMSG(ZONE_FUNCTION,(_T("Cspi::UnmapDMABuffers+\r\n")));

	if(pVirtDMABufferAddr)
	{
		memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
		// Logical address parameter is ignored
		PhysDMABufferAddr.QuadPart = 0;
		HalFreeCommonBuffer(&Adapter, 0, PhysDMABufferAddr, (PVOID)pVirtDMABufferAddr, FALSE);
	}

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
BOOL cspiClass::DeinitChannelDMA(void)
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
BOOL cspiClass::InitChannelDMA(UINT32 Index)
{
	UINT8 tmp, dmaChannelPriority[2];
	BOOL rc = FALSE;
	Index = Index;

	DEBUGMSG(ZONE_FUNCTION,(_T("Cspi::InitChannelDMA+\r\n")));

	// Check if DMA buffer has been allocated
	if (!PhysDMABufferAddr.LowPart || !pVirtDMABufferAddr)
	{
		DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA: Invalid DMA buffer physical address.\r\n")));
		goto cleanUp;
	}

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
	if (!DDKSdmaAllocChain(m_dmaChanCspiRx, 1))
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
	if (!DDKSdmaAllocChain(m_dmaChanCspiTx, 1))
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
// Function: InitDMA
//
//  Performs DMA channel intialization
//
// Parameters:
//  Index
//      CSPI device Index
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------

BOOL cspiClass::InitCspiDMA(UINT32 Index) 
{ 

	DEBUGMSG(ZONE_FUNCTION, (TEXT("Cspi: InitDMA+\r\n")));
	Index = Index;

	m_dmaReqTx = CspCSPIGetDmaReqTx(Index) ; 
	m_dmaReqRx = CspCSPIGetDmaReqRx(Index) ; 

	// Map the DMA buffers into driver's virtual address space
	if (!MapDMABuffers())
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Cspi:InitDMA() - Failed to map DMA buffers.\r\n")));
		return FALSE;
	}

	// Initialize the output DMA
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
BOOL cspiClass::DeInitCspiDMA(void) 
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
UINT32 cspiClass::CspiExchangeSize(PCSPI_XCH_PKT0_T pXchPkt)
{
	PCSPI_BUSCONFIG_T pBusCnfg = pXchPkt->pBusCnfg;

	if ((pBusCnfg->bitcount >= 1) && (pBusCnfg->bitcount <= 8))
	{
		// 8-bit access width
		return pXchPkt->xchCnt *sizeof(UINT8);
	}
	else if ((pBusCnfg->bitcount >= 9) && (pBusCnfg->bitcount <= 16))
	{
		// 16-bit access width
		return pXchPkt->xchCnt *sizeof(UINT16);
	}
	else if ((pBusCnfg->bitcount >= 17) && (pBusCnfg->bitcount <= 32))
	{
		// 32-bit access width
		return pXchPkt->xchCnt *sizeof(UINT32);
	}
	else
	{
		// unsupported access width
		return 0;
	}
}
