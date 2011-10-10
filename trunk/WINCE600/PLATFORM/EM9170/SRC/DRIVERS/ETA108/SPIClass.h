/*---------------------------------------------------------------------------
* Copyright (C) 2004-2008, Emroonix, inc. Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
//
//  File:  eta108class.h
//
//   Header file for cspi bus driver.
//
//------------------------------------------------------------------------------
#ifndef __SPICALSS_H__
#define __SPICALSS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"
#include "cspibus.h"
#include "ETA108.h"

//------------------------------------------------------------------------------
//Defines

//------------------------------------------------------------------------------
//Types
// 
class spiClass
{

public:
 	spiClass();
 	~spiClass();
	BOOL	CspiInitialize(DWORD Index);
	void	CspiRelease(void);
	BOOL	CspiIOMux( void );
	DWORD	CspiADCRun( PADS_CONFIG pADSConfig, PCSPI_XCH_PKT_T pXchPkt );
	void	CspiADCDone( );		//DMA Transfer
	DWORD	CspiADCConfig(PCSPI_XCH_PKT_T pXchPkt);	//Non DMA transfer
	void	CspiEnableLoopback(BOOL bEnable);

public:
	UINT16 *m_pSPIRxBuf;	// RX buffer
	UINT16 *m_pSPITxBuf;

	DWORD  m_dwSamplingLength;	//SPI sampling length
	DWORD  m_dwSpiXchCount;	// SPI transfer count. m_dwSpiXchCount=m_dwSamplingLength*2
							//the count in uint16

	DWORD  m_dwSpiDmaCount; // SPI DMA count m_dxSpiDmaCount=m_dwSpiXchCount*2
							// the count in bytes

	DWORD  m_dwCurrSpiCount;// Already exchanged SPI DMA bytes. count in bytes

	DWORD  m_dwDMABufferSize;
	DWORD  m_dwMultDmaBufSize;	//Buffer Size of Continuous Sampling 
	
private:
	// DMA specific
	BOOL InitCspiDMA(UINT32 Index);
	BOOL DeInitCspiDMA(void);
	BOOL InitChannelDMA(UINT32 Index);
	BOOL DeinitChannelDMA(void);
	BOOL UnmapDMABuffers(void);
	BOOL MapDMABuffers(void);
	VOID TransferTxBuf( UINT8 nCurrTxDmaBufIdx );

private:
	UINT32 m_Index;
	
	PCSP_CSPI_REG		m_pCSPI;
	HANDLE				m_hHeap;
	HANDLE				m_hIntrEvent;	// system interrupt event
	//HANDLE				m_hEnQEvent;	// interrupt handle thread exit event
	HANDLE				m_hThread;		// interrupt thread handle 
	BOOL				m_bTerminate;	// interrupt handle thread exit event
	CRITICAL_SECTION	m_cspiCs;
	CRITICAL_SECTION	m_cspiDataXchCs;
	BOOL				m_bUseLoopBack;

	PHYSICAL_ADDRESS	m_pSpiPhysTxDMABufferAddr, m_pSpiPhysRxDMABufferAddr;
	PBYTE				m_pSpiVirtTxDMABufferAddr,m_pSpiVirtRxDmaBufferAddr ;
	UINT8				m_dmaChanCspiRx, m_dmaChanCspiTx; 
	UINT8				m_currRxDmaBufIdx, m_currTxDmaBufIdx;
	DDK_DMA_REQ			m_dmaReqTx, m_dmaReqRx ; 
	DWORD				m_dwSysIntr;
	UINT8				m_nSamplingMode;
	UINT8				m_ADChannle[8];
	DWORD				m_dwADChannleCount, m_dwADChannleIdx;
	DWORD				m_dwAvailRxByteCount, m_dwSendByteCount;

private:
//	UINT32 CspiExchangeSize(PCSPI_XCH_PKT0_T pXchPkt);
	static UINT32 CspiBufRd8(LPVOID pBuf);
	static UINT32 CspiBufRd16(LPVOID pBuf);
	static UINT32 CspiBufRd32(LPVOID pBuf);
	static void CspiBufWrt8(LPVOID pBuf, UINT32 data);
	static void CspiBufWrt16(LPVOID pBuf, UINT32 data);
	static void CspiBufWrt32(LPVOID pBuf, UINT32 data);

	static DWORD WINAPI CspiProcess(LPVOID lpParameter);
};
 




#ifdef __cplusplus
}
#endif

#endif __SPICALSS_H__