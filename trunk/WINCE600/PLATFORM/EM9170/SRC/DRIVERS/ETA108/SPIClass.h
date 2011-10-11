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
#ifdef DEBUG
	DBGPARAM dpCurSettings = {
		TEXT("eta108"), {
			TEXT("Init"),TEXT("Deinit"),TEXT("Open"),TEXT("Close"),
				TEXT("IOCtl"),TEXT("Thread"),TEXT(""),TEXT(""),
				TEXT(""),TEXT(""),TEXT(""),TEXT(""),
				TEXT(""),TEXT("Function"),TEXT("Warning"),TEXT("Error") },
				0xC000
	};
#endif // DEBUG

// #ifdef DEBUG
 #define ZONE_INIT        1
// #define ZONE_DEINIT      2
// #define ZONE_OPEN        3
// #define ZONE_CLOSE       4
// #define ZONE_IOCTL       5
// #define ZONE_THREAD      6
// #define ZONE_DMA         7
 #define ZONE_FUNCTION    8
// #define ZONE_WARN        9
// #define ZONE_ERROR       10
// #endif // DEBUG
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
	void	CspiADCDone( );		//DMA Transfer over
	DWORD	CspiADCConfig(PCSPI_XCH_PKT_T pXchPkt);	//Non DMA transfer
	void	CspiEnableLoopback(BOOL bEnable);

public:
	UINT16 *m_pSPIRxBuf;	// RX buffer
	UINT16 *m_pSPITxBuf;

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
	DWORD GetSdmaChannelIRQ(UINT32 chan);

private:
	UINT32 m_Index;				//CSPI index

	DWORD  m_dwSamplingLength;	//SPI sampling length
	DWORD  m_dwSpiXchCount;	// SPI transfer count. m_dwSpiXchCount=m_dwSamplingLength*2. count in uint16

	DWORD  m_dwSpiDmaCount; // SPI DMA count m_dxSpiDmaCount=m_dwSpiXchCount*2. count in bytes
	
	PCSP_CSPI_REG		m_pCSPI;
	HANDLE				m_hHeap;
	HANDLE				m_hIntrEvent;	// system interrupt event
	HANDLE				m_hThread;		// interrupt thread handle 
	HANDLE				m_hCSPIEvent;	// SPI transfer completed event
	BOOL				m_bTerminate;	// interrupt handle thread exit event
	CRITICAL_SECTION	m_cspiTxBufCs;
	CRITICAL_SECTION	m_cspiRxBufCs;
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
	VOID TransferTxBuf( UINT8 nCurrTxDmaBufIdx );

	static DWORD WINAPI CspiProcess(LPVOID lpParameter);
};
 




#ifdef __cplusplus
}
#endif

#endif __SPICALSS_H__