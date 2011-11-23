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

#include "ETA108Class.h"

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
#define  ETA108WRITE_BLOCK		TRUE
#define  ETA108WRITE_UNBLOCK	FALSE

#define  MIN_VAL(a, b)   (((a) > (b)) ? (b) : (a))
//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions

inline HRESULT CeOpenCallerBuffer(
								  CALLER_STUB_T& CallerStub,
								  PVOID pSrcUnmarshalled,
								  DWORD cbSrc,
								  DWORD ArgumentDescriptor,
								  BOOL ForceDuplicate)
{
	CallerStub.m_pCallerUnmarshalled =pSrcUnmarshalled;
	CallerStub.m_cbSize =cbSrc;
	CallerStub.m_ArgumentDescriptor =ArgumentDescriptor; 
	CallerStub.m_pLocalAsync = 0;

	return ::CeOpenCallerBuffer(&CallerStub.m_pLocalSyncMarshalled,
		pSrcUnmarshalled, cbSrc,
		ArgumentDescriptor, ForceDuplicate);
}

inline HRESULT CeCloseCallerBuffer(CALLER_STUB_T& CallerStub)
{
	return ::CeCloseCallerBuffer(
		CallerStub.m_pLocalSyncMarshalled,
		CallerStub.m_pCallerUnmarshalled,
		CallerStub.m_cbSize,
		CallerStub.m_ArgumentDescriptor); 
}

eta108Class::eta108Class()
{
	m_pSpi = NULL;

	//m_hPWM = INVALID_HANDLE_VALUE;
	
}

eta108Class::~eta108Class()
{

}

BOOL eta108Class::ETA108Initialize()
{
	m_pSpi = new spiClass;
	if( m_pSpi == NULL )
		goto error_init;
	
	m_dwCSPIChannle = 3;
	m_pSpi->m_dwTxDMABufferSize = 0x1b30;		//6960
	m_pSpi->m_dwRxDMABufferSize = 0x61a8;		//25K
 	if ( !(m_pSpi && m_pSpi->CspiInitialize(m_dwCSPIChannle))) 
 	{
		goto error_init;
 	}
	
	return TRUE;

error_init:
	ETA108Release( );
	return FALSE;
}

BOOL eta108Class::ETA108Release()
{
 	if( m_pSpi )
 	{
 		m_pSpi->CspiRelease( );
 		delete m_pSpi;
 		m_pSpi = NULL;
 	}

	return TRUE;
}

BOOL eta108Class::ETA108Open()
{
	CSPI_BUSCONFIG_T stCspiConfig;
	CSPI_XCH_PKT_T stCspiXchPkt;

// 	stCspiConfig.bitcount = 16;		//data rate = 16bit
// 	stCspiConfig.chipselect = 0;	//use channel 0
// 	stCspiConfig.freq = 12000000;	//XCH speed = 16M
// 	stCspiConfig.pha = TRUE;
// 	stCspiConfig.pol = FALSE;
// 	stCspiConfig.ssctl = TRUE;		//one entry entry with each SPI burst
// 	stCspiConfig.sspol = FALSE;		//SPI_CS Active low
// 	stCspiConfig.usepolling = FALSE;//polling don't use interrupt
// 
// 	stCspiConfig.drctl = 0;			//Don't care SPI_RDY
// 	stCspiConfig.usedma = FALSE;	//Don't DMA
// 
 	stCspiXchPkt.pBusCnfg = &stCspiConfig;
// 	stCspiXchPkt.xchEvent = NULL;
// 	stCspiXchPkt.xchEventLength = 0;


	UINT16 SPITxBuf[5];
	UINT16 SPIRxBuf[5];
	stCspiXchPkt.pTxBuf = (LPVOID)SPITxBuf;
	stCspiXchPkt.pRxBuf = (LPVOID)SPIRxBuf;

	SPITxBuf[0] = ADS8201_REG_READ|ADS8021_ADC_TRIGGER_SCR;
	SPITxBuf[1] = ADS8201_REG_READ|ADS8021_CONV_DELAY_SCR;
	SPITxBuf[2] = ADS8201_REG_WRITE| ADS8021_ADC_SCR | 0x04;	//BUSY
	SPITxBuf[3] = ADS8201_REG_READ| ADS8021_ADC_SCR;	//BUSY
	stCspiXchPkt.xchCnt = 4;
	SPIRxBuf[0] = 0;
	SPIRxBuf[1] = 0;
	SPIRxBuf[2] = 0;
	SPIRxBuf[3] = 0;
	if( stCspiXchPkt.xchCnt != m_pSpi->CspiADCConfig( &stCspiXchPkt ))
	{
		RETAILMSG( 1, (TEXT("ETA108Open: CSPI exchange failed!!!\r\n")));
		goto open_error;
	}
	
	if( (SPIRxBuf[0]&0x1fff )  != 2  || (SPIRxBuf[1]&0x1fff) != 2 || (SPIRxBuf[3]&0x1fff) != 4 )
	{
		RETAILMSG( 1, (TEXT("ETA108Open: Read ADS8201's register failed!!! 0x%x, 0x%x, 0x%x\r\n"), SPIRxBuf[0], SPIRxBuf[1], SPIRxBuf[3]));
		goto open_error;
	}

	BOOL bRet = m_pSpi->OpenPWM( );
	if( !bRet )
		goto open_error;

	RETAILMSG( 1, (TEXT("ETA108Open: ETA108 is Opened.\r\n")));
	return TRUE;

open_error:
	ETA108Close( );
	return FALSE;
}

BOOL eta108Class::ETA108Close()
{
	ETA108Stop( );
	m_pSpi->ClosePWM();
	
	return TRUE;
}

BOOL eta108Class::ETA108Setup( PADS_CONFIG pADSConfig, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut )
{
	BOOL bRet = TRUE;
	int i;
	DWORD dwADChannleCount, dwTmp;

	dwTmp = 0;
	
	memcpy( &m_stADSConfig, pADSConfig, sizeof(m_stADSConfig ));

	for( dwADChannleCount=0,i=0; i<8; i++ )
	{
		if( pADSConfig->dwSamplingChannel & (0x01<<i))
			dwADChannleCount++;
	}

	m_stADSConfig.dwSamplingChannel = dwADChannleCount;

	if( pADSConfig->dwSamplingLength == 0 )
	{
		//Continue sampling mode...
		m_pSpi->m_nSamplingMode = SAMPLING_MODE_CONTINUOUS;
		// Total sampling rate
		m_stADSConfig.dwSamplingRate = pADSConfig->dwSamplingRate*dwADChannleCount;
		//Total sampling length 250ms
		m_stADSConfig.dwSamplingLength = m_stADSConfig.dwSamplingRate/4/dwADChannleCount*dwADChannleCount;
		if( m_stADSConfig.dwSamplingLength< 4)
		{
			m_stADSConfig.dwSamplingLength = 4;
		}

	}
	else if( pADSConfig->dwSamplingLength>0 )
	{
		//Single sampling mode...
		m_pSpi->m_nSamplingMode = SAMPLING_MODE_SINGLE;
		//Total sampling length
		m_stADSConfig.dwSamplingLength = pADSConfig->dwSamplingLength * dwADChannleCount;
		// Total sampling rate
		m_stADSConfig.dwSamplingRate = pADSConfig->dwSamplingRate*dwADChannleCount;
	}
	else
	{
		bRet = FALSE;
		RETAILMSG( 1, (TEXT("ETA108Setup:The shampling rate out of range!\r\n")));
	}


	if( m_stADSConfig.dwSamplingRate > 100000 )
	{
		bRet = FALSE;
		m_stADSConfig.dwSamplingRate = 100000;
		RETAILMSG( 1, (TEXT("ETA108Setup:The shampling rate out of range!\r\n")));
	}
	if( m_stADSConfig.dwSamplingRate<1 )
	{
		bRet = FALSE;
		m_stADSConfig.dwSamplingRate = 1;
		RETAILMSG( 1, (TEXT("ETA108Setup:The shampling rate out of range!\r\n")));
	}

	if( pBufOut != NULL && dwLenOut == sizeof( ADS_CONFIG ))
	{
		dwTmp= sizeof( ADS_CONFIG );
		memcpy( pBufOut, (PBYTE)&m_stADSConfig, dwTmp);
	}

	if( pdwActualOut )
	{
		*pdwActualOut = dwTmp;
	}

	m_stADSConfig.dwSamplingChannel = pADSConfig->dwSamplingChannel;

	return bRet;

}


BOOL eta108Class::ETA108Stop( )
{
	m_pSpi->CspiADCDone( );
	return TRUE;
}

BOOL eta108Class::ETA108Start( )
{
//	CALLER_STUB_T marshalEventStub;
//	HRESULT result;
	DWORD i;

	//2.Config CSPI & config ADS8201
	CSPI_BUSCONFIG_T stCspiConfig;
	CSPI_XCH_PKT_T stCspiXchPkt;

// 	stCspiConfig.bitcount = 16;		//data rate = 16bit
// 	stCspiConfig.chipselect = 0;	//use channel 0
// 	stCspiConfig.freq = 12000000;	//XCH speed = 16M
// 
// 	stCspiConfig.pha = TRUE;
// 	stCspiConfig.pol = FALSE;
// 	stCspiConfig.ssctl = TRUE;		//one entry entry with each SPI burst
// 	stCspiConfig.sspol = FALSE;		//SPI_CS Active low
// 	stCspiConfig.usepolling = FALSE;//polling don't use interrupt
// 	
// 	stCspiConfig.drctl = 0;			//Don't care SPI_RDY
// 	stCspiConfig.usedma = FALSE;	//Don't DMA
// 
 	stCspiXchPkt.pBusCnfg = &stCspiConfig;
// 	stCspiXchPkt.xchEvent = NULL;
// 	stCspiXchPkt.xchEventLength = 0;

	UINT16 SPITxBuf[10];
	UINT16 SPIRxBuf[10];
	stCspiXchPkt.pTxBuf = (LPVOID)SPITxBuf;
	stCspiXchPkt.pRxBuf = (LPVOID)SPIRxBuf;

	//ADS8201 Configuration parameter
	//memset( &m_stADS8201CFG, 0, sizeof(m_stADS8201CFG));
	//ADS8201 default configuration
	stCspiXchPkt.xchCnt = 0;
	SPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE| ADS8021_ADC_SCR | 0x04;	//BUSY
	if( m_stADSConfig.dwContrlWordLength > 0 && m_stADSConfig.dwContrlWordLength < 6 && m_stADSConfig.lpContrlWord != NULL )
	{
		for( i=0; i<m_stADSConfig.dwContrlWordLength ; i++)
		{
			SPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE|*((UINT16 *)(UINT16)(m_stADSConfig.lpContrlWord)+i);
		}
	}
	
	/*stCspiXchPkt.xchCnt = 4;
	for( DWORD i=0; i<stCspiXchPkt.xchCnt; i++ )
		SPITxBuf[i]  = (UINT16)i+1;*/
	//ADS8201 configuration
	//if( stCspiXchPkt.xchCnt != m_pSpi->CspiADCConfig( &stCspiXchPkt ))
	//	goto error_cleanup;
	
 	//RETAILMSG( 1, (TEXT("ADS8201 Config words = %d\r\n"),stCspiXchPkt.xchCnt));
 	/*for( i=0; i<stCspiXchPkt.xchCnt; i++ )
		RETAILMSG( 1, (TEXT("0x%x "),SPIRxBuf[i]));
	RETAILMSG( 1, (TEXT("\r\n")));*/
	
	//Redefine SPI bus configuration
	//Burst will be triggered by the falling edge of the SPI_RDY signal (edge-triggered).

	stCspiConfig.chipselect = 0;	//use channel 0
 	stCspiConfig.pol = FALSE;
 	stCspiConfig.sspol = FALSE;		//SPI_CS Active low
 	stCspiConfig.usepolling = FALSE;//polling don't use interrupt
	stCspiConfig.drctl = 0;			//Don't care SPI_RDY

	stCspiConfig.bitcount = 32;		//data rate = 16bit
	stCspiConfig.usedma = TRUE;		//Use DMA
	stCspiConfig.pha = TRUE;
	stCspiConfig.ssctl = FALSE;		
	stCspiXchPkt.xchEvent = NULL;
	stCspiXchPkt.xchEventLength = 0;

	if( !m_pSpi->CspiADCRun( &m_stADSConfig, &stCspiXchPkt ))
		goto error_cleanup;

	return DWORD(sizeof(ADS_CONFIG));

error_cleanup:
	ETA108Stop();
	return DWORD(-1);

}

// dwCount count in UINT16
DWORD eta108Class::ETA108Read( LPVOID pBuffer, DWORD dwCount )
{
	return (m_pSpi->ReadADCData( pBuffer, dwCount ));
}

BOOL eta108Class::WateDataReady(DWORD dwTimeOut)
{
	BOOL bRet=FALSE;

	if( dwTimeOut<=0 )
	{
		if( m_pSpi->m_nSamplingMode == SAMPLING_MODE_SINGLE )
		{
			dwTimeOut = (DWORD)(1000.0/m_stADSConfig.dwSamplingRate*m_stADSConfig.dwSamplingLength);
		}
		else
		{
			dwTimeOut = 250;
		}
		dwTimeOut += 50;
	}
	RETAILMSG( 1, (TEXT("WateDataReady:TimeOut=%d\r\n"),dwTimeOut ));
	if( WaitForSingleObject(m_pSpi->m_hTransferDoneEvent, dwTimeOut ) == WAIT_OBJECT_0 )
	{
		bRet = TRUE;
	}
	
	return bRet;
}