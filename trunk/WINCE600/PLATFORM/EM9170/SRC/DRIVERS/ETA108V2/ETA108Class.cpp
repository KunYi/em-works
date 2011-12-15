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
	m_pSpi->m_dwTxDMABufferSize = 0x1a40;		//5040
	m_pSpi->m_dwRxDMABufferSize = 0xC350;		//50K
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

	stCspiXchPkt.pBusCnfg = &stCspiConfig;
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
	//	CALLER_STUB_T marshalEventStub;
	//	HRESULT result;
	BOOL bRet = TRUE;
	int i;
	DWORD dwADChannleCount, dwTmp, j;

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
		if( m_stADSConfig.dwSamplingRate > 50000 )
		{
			//Total sampling length (125ms)
			m_stADSConfig.dwSamplingLength = m_stADSConfig.dwSamplingRate/8;
		}
		else
		{
			//Total sampling length (250ms)
			m_stADSConfig.dwSamplingLength = m_stADSConfig.dwSamplingRate/4;
		}
		if( m_stADSConfig.dwSamplingLength%4 != 0 )
		{
			//DMA transmit require XchCount is a multiple of 4( count in UINT32)
			m_stADSConfig.dwSamplingLength = ((m_stADSConfig.dwSamplingLength>>2)<<2)+4;
		}
		m_stADSConfig.dwSamplingLength = m_stADSConfig.dwSamplingLength/dwADChannleCount*dwADChannleCount;
	}
	else if( pADSConfig->dwSamplingLength>0 )
	{
		//Single sampling mode...
		m_pSpi->m_nSamplingMode = SAMPLING_MODE_SINGLE;
		//Total sampling length
		m_stADSConfig.dwSamplingLength = pADSConfig->dwSamplingLength * dwADChannleCount;
		if( m_stADSConfig.dwSamplingLength%4 != 0 )
		{
			//DMA transmit require XchCount is a multiple of 4( count in UINT32)
			m_stADSConfig.dwSamplingLength = ((m_stADSConfig.dwSamplingLength>>2)<<2)+4;
		}
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

	// ADS8201 register config data
	memset( m_ADS8201REG, 0, sizeof(m_ADS8201REG ));
	if( pADSConfig->lpContrlWord  && pADSConfig->dwContrlWordLength && pADSConfig->dwContrlWordLength<5 )
	{
		UINT16 *pUIN16 = (UINT16 *)pADSConfig->lpContrlWord;
		for( j=0; j<pADSConfig->dwContrlWordLength; j++ )
		{
			pUIN16[j] &= 0x00003cff; 
			if( pUIN16[j]>>10 == 4 )
			{
				pUIN16[j] |= 0x40;
			}
			m_ADS8201REG[j] = pUIN16[j];
		}
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
	CSPI_BUSCONFIG_T stCspiConfig;
	CSPI_XCH_PKT_T stCspiXchPkt;

	//1.Reset ADS8201

	//2.config ADS8201
 	stCspiXchPkt.pBusCnfg = &stCspiConfig;

	UINT16 SPITxBuf[10];
	UINT16 SPIRxBuf[10];
	stCspiXchPkt.pTxBuf = (LPVOID)SPITxBuf;
	stCspiXchPkt.pRxBuf = (LPVOID)SPIRxBuf;

	//ADS8201 Configuration parameter
	stCspiXchPkt.xchCnt=0;
	while( m_ADS8201REG[stCspiXchPkt.xchCnt] )
	{
		SPITxBuf[stCspiXchPkt.xchCnt] = ADS8201_REG_WRITE | m_ADS8201REG[stCspiXchPkt.xchCnt];
		stCspiXchPkt.xchCnt++;
	}
	if( stCspiXchPkt.xchCnt )
	{
		if( stCspiXchPkt.xchCnt != m_pSpi->CspiADCConfig( &stCspiXchPkt ))
			goto error_cleanup;
	}
		
	//3.Start ADC
	// Reset transfer done Semaphore
	while( WaitForSingleObject( m_pSpi->m_hTransferDoneSemaphore, 0 ) == WAIT_OBJECT_0 );
	
	//Burst will be triggered by the falling edge of the SPI_RDY signal (edge-triggered).
	//Redefine SPI bus configuration
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

// dwCount count in bytes
DWORD eta108Class::ETA108Read( LPVOID pBuffer, DWORD dwCount )
{
	return (m_pSpi->ReadADCData( pBuffer, dwCount ));
}

BOOL eta108Class::WateDataReady(DWORD dwTimeOut)
{
	BOOL bRet=FALSE;

	if( dwTimeOut<=0 )
	{
		dwTimeOut = (DWORD)(1000.0/m_stADSConfig.dwSamplingRate*m_stADSConfig.dwSamplingLength);
		if( dwTimeOut<4000 )
			dwTimeOut = 4000;
	}
	RETAILMSG( 1, (TEXT("WateDataReady: WateDataReady:TimeOut=%d\r\n"),dwTimeOut ));
	if( WaitForSingleObject(m_pSpi->m_hTransferDoneSemaphore, dwTimeOut ) == WAIT_OBJECT_0 )
	{
		bRet = TRUE;
	}
	
	return bRet;
}