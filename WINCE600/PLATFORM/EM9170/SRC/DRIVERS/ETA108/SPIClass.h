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
	BOOL CspiInitialize(DWORD Index);
	void CspiRelease(void);
	BOOL CspiIOMux( void );
	DWORD CspiADCRun( PADS_CONFIG pADSConfig, PCSPI_XCH_PKT_T pXchPkt );
	DWORD CspiNonDMADataExchange(PCSPI_XCH_PKT_T pXchPkt);

public:
	UINT32 m_Index;
	UINT16 *m_pSPITxBuf, *m_pSPIRxBuf;
	DWORD  m_dwXchBufLen;
	DWORD  m_dwSamplingLength;
	

private:
	// DMA specific
	BOOL InitCspiDMA(UINT32 Index);
	BOOL DeInitCspiDMA(void);
	BOOL InitChannelDMA(UINT32 Index);
	BOOL DeinitChannelDMA(void);
	BOOL UnmapDMABuffers(void);
	BOOL MapDMABuffers(void);
	VOID MoveDMABuffer(LPVOID pBuf, DWORD dwLen, BOOL bReceive);

private:
	PCSP_CSPI_REG m_pCSPI;
	HANDLE m_hHeap;
	HANDLE m_hIntrEvent;
	HANDLE m_hEnQEvent;
	HANDLE m_hThread;
	BOOL   m_bTerminate;
	CRITICAL_SECTION m_cspiCs;
	CRITICAL_SECTION m_cspiDataXchCs;

	PHYSICAL_ADDRESS m_pSpiPhysTxDMABufferAddr, m_pSpiPhysRxDMABufferAddr;
	PBYTE            m_pSpiVirtTxDMABufferAddr,m_pSpiVirtRxDmaBufferAddr ;
	UINT8             m_dmaChanCspiRx, m_dmaChanCspiTx; 
	DDK_DMA_REQ       m_dmaReqTx, m_dmaReqRx ; 
	DWORD m_dwSysIntr;
	UINT8			m_nSamplingMode;

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