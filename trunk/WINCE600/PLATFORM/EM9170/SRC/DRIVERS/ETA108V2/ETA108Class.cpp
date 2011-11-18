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
WCHAR  g_szCSPIEvent[] = TEXT("CspiEvent");
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

	m_hPWM = INVALID_HANDLE_VALUE;
	
	m_hThread = NULL;
	m_hADCEvent = NULL;
	m_hCSPIEvent = NULL;
}

eta108Class::~eta108Class()
{

}

BOOL eta108Class::ETA108Initialize()
{
	// create event for CSPI transfer completed signaling
	m_hCSPIEvent = CreateEvent( NULL, FALSE, FALSE, g_szCSPIEvent );
	if( m_hCSPIEvent == NULL )
		goto error_init;

	if( !m_hThread )
	{
		m_bTerminate = FALSE;
		// create ADC moniter thread
		m_hThread = ::CreateThread(NULL, 0, ADCEventHandle, this, 0, NULL);
		if( m_hThread == NULL )
			goto error_init;
	}

	m_pSpi = new spiClass;
	if( m_pSpi == NULL )
		goto error_init;
	
	m_dwCSPIChannle = 3;
	//m_pSpi->m_dwDMABufferSize = m_dwDMABufSize;
	m_pSpi->m_dwDMABufferSize = 0x2bc0;
	m_pSpi->m_dwMultDmaBufSize = m_dwMultDmaBufSize;
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

	if( m_hThread != NULL )
	{
		m_bTerminate = TRUE;
		if( m_hCSPIEvent )
		{
			SetEvent(m_hCSPIEvent);
		}
		CloseHandle(m_hThread);
	}

	if( m_hCSPIEvent != NULL )
		CloseHandle(m_hCSPIEvent);
	return TRUE;
}

BOOL eta108Class::ETA108Open()
{
	CSPI_BUSCONFIG_T stCspiConfig;
	CSPI_XCH_PKT_T stCspiXchPkt;

	stCspiConfig.bitcount = 16;		//data rate = 16bit
	stCspiConfig.chipselect = 0;	//use channel 0
	stCspiConfig.freq = 12000000;	//XCH speed = 16M
	stCspiConfig.pha = TRUE;
	stCspiConfig.pol = FALSE;
	stCspiConfig.ssctl = TRUE;		//one entry entry with each SPI burst
	stCspiConfig.sspol = FALSE;		//SPI_CS Active low
	stCspiConfig.usepolling = FALSE;//polling don't use interrupt

	stCspiConfig.drctl = 0;			//Don't care SPI_RDY
	stCspiConfig.usedma = FALSE;	//Don't DMA

	stCspiXchPkt.pBusCnfg = &stCspiConfig;
	stCspiXchPkt.xchEvent = NULL;
	stCspiXchPkt.xchEventLength = 0;


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
		return FALSE;
	}
	
	if( (SPIRxBuf[0]&0x1fff )  != 2  || (SPIRxBuf[1]&0x1fff) != 2 || (SPIRxBuf[3]&0x1fff) != 4 )
	{
		RETAILMSG( 1, (TEXT("ETA108Open: Read ADS8201's register failed!!! 0x%x, 0x%x, 0x%x\r\n"), SPIRxBuf[0], SPIRxBuf[1], SPIRxBuf[3]));
		return FALSE;
	}

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

	RETAILMSG( 1, (TEXT("ETA108Open: ETA108 is Opened.\r\n")));
	return TRUE;
}

BOOL eta108Class::ETA108Close()
{
	m_pSpi->CspiADCDone( );
	if( m_hADCEvent != NULL )
	{
		CloseHandle(m_hADCEvent);
		m_hADCEvent = NULL;
	}

	if( m_hPWM != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hPWM);
		m_hPWM = INVALID_HANDLE_VALUE;
	}

	return TRUE;
}

DWORD eta108Class::ETA108Run( PADS_CONFIG pADSConfig )
{
	CALLER_STUB_T marshalEventStub;
	HRESULT result;
	DWORD i;

 	if(m_hPWM == INVALID_HANDLE_VALUE )
 		goto error_cleanup;

	m_dwRxBufSeek = 0;
	m_dwSamplingLength = pADSConfig->dwSamplingLength;
	if( m_hADCEvent != NULL )
	{
		CloseHandle(m_hADCEvent);
		m_hADCEvent = NULL;
	}
	m_pSpi->CspiADCDone( );
	//1.Parameter marshal
	if( pADSConfig->lpADCompleteEvent && (pADSConfig->dwADCompleteEventLength == wcslen(pADSConfig->lpADCompleteEvent)) )
	{
		m_bWriteBlock = ETA108WRITE_UNBLOCK;
		result = CeOpenCallerBuffer(
			marshalEventStub,
			pADSConfig->lpADCompleteEvent,
			pADSConfig->dwADCompleteEventLength,
			ARG_I_PTR,
			FALSE);

		if (!SUCCEEDED(result)) 
		{
			goto error_cleanup;
		}

		//Create event for AD conversion completed.
		m_hADCEvent = CreateEvent(NULL,FALSE, FALSE, pADSConfig->lpADCompleteEvent );
		CeCloseCallerBuffer(marshalEventStub);

		if( m_hADCEvent == NULL )
		{
			goto error_cleanup;
		}
	}
	else 	
	{
		m_bWriteBlock = ETA108WRITE_BLOCK;	
		//Create event for AD conversion completed.
		m_hADCEvent = CreateEvent(NULL,FALSE, FALSE, NULL );
		if( m_hADCEvent == NULL )
			goto error_cleanup;
	}

	//2.Config CSPI & config ADS8201
	CSPI_BUSCONFIG_T stCspiConfig;
	CSPI_XCH_PKT_T stCspiXchPkt;

	stCspiConfig.bitcount = 16;		//data rate = 16bit
	stCspiConfig.chipselect = 0;	//use channel 0
	stCspiConfig.freq = 12000000;	//XCH speed = 16M

	stCspiConfig.pha = TRUE;
	stCspiConfig.pol = FALSE;
	stCspiConfig.ssctl = TRUE;		//one entry entry with each SPI burst
	stCspiConfig.sspol = FALSE;		//SPI_CS Active low
	stCspiConfig.usepolling = FALSE;//polling don't use interrupt
	
	stCspiConfig.drctl = 0;			//Don't care SPI_RDY
	stCspiConfig.usedma = FALSE;	//Don't DMA

	stCspiXchPkt.pBusCnfg = &stCspiConfig;
	stCspiXchPkt.xchEvent = NULL;
	stCspiXchPkt.xchEventLength = 0;


	UINT16 SPITxBuf[10];
	UINT16 SPIRxBuf[10];
	stCspiXchPkt.pTxBuf = (LPVOID)SPITxBuf;
	stCspiXchPkt.pRxBuf = (LPVOID)SPIRxBuf;

	//ADS8201 Configuration parameter
	//memset( &m_stADS8201CFG, 0, sizeof(m_stADS8201CFG));
	//ADS8201 default configuration
	stCspiXchPkt.xchCnt = 0;
	SPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE| ADS8021_ADC_SCR | 0x04;	//BUSY
	if( pADSConfig->dwContrlWordLength > 0 && pADSConfig->dwContrlWordLength < 6 && pADSConfig->lpContrlWord != NULL )
	{
		for( i=0; i<pADSConfig->dwContrlWordLength ; i++)
		{
			SPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE|*((UINT16 *)(UINT16)(pADSConfig->lpContrlWord)+i);
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
	stCspiConfig.bitcount = 32;		//data rate = 16bit
	stCspiConfig.usedma = TRUE;		//Use DMA
	stCspiConfig.pha = TRUE;
	stCspiConfig.ssctl = FALSE;		
	//stCspiConfig.ssctl = TRUE;		
	stCspiXchPkt.xchEvent = g_szCSPIEvent;
	stCspiXchPkt.xchEventLength = wcslen( g_szCSPIEvent );

	if( !m_pSpi->CspiADCRun( pADSConfig, &stCspiXchPkt ))
		goto error_cleanup;

	//Config PWM
	PWMINFO PwmInfo; 
	DWORD dwNumberOfBytesToWrite; 
	DWORD dwNumberOfBytesWritten; 
	BOOL bRet; 

	PwmInfo.dwFreq = pADSConfig->dwSamplingRate;

	// PWM Duty cycle = CONVST(convert start)pulse width>40nS.(CONVST active low level)
	// ETA108'S CPLD utilize 6M sample clock to generate AD start conversion pulse ,
	// so the high level of the PWM should be greater than 166ns(1/6M).
	PwmInfo.dwDuty = 2;	
	// Remain output until issue a new write operation
	PwmInfo.dwDuration = 0;			
	dwNumberOfBytesToWrite = sizeof(PWMINFO); 
	dwNumberOfBytesWritten = 0; 
	//Now start AD conversion
	bRet = WriteFile(m_hPWM, (LPCVOID)&PwmInfo, dwNumberOfBytesToWrite, &dwNumberOfBytesWritten, NULL); 
	if( !bRet )
		goto error_cleanup;
	
	if( m_bWriteBlock == ETA108WRITE_BLOCK )
	{
		WaitForSingleObject( m_hADCEvent, INFINITE );
	}

	return DWORD(sizeof(ADS_CONFIG));

error_cleanup:
	ETA108Close();
	return DWORD(-1);

}

// dwCount count in UINT16
DWORD eta108Class::ETA108Read( LPVOID pBuffer, DWORD dwCount )
{
	DWORD dwReadBytes;
		
	if( !(m_hADCEvent && pBuffer && m_pSpi->m_pSPIRxBuf) )
		 return DWORD(-1);

	if( m_dwRxBufSeek+dwCount > m_dwSamplingLength*4 )
		dwReadBytes = m_dwSamplingLength*4 - m_dwRxBufSeek;
	else
		dwReadBytes = dwCount;

	memcpy( pBuffer, m_pSpi->m_pSPIRxBuf+m_dwRxBufSeek, dwReadBytes );
	return (dwReadBytes);
}

// lAmount count in bytes
DWORD eta108Class::ReadSeek( long lAmount, WORD dwType )
{
	DWORD dwSeek;

	switch( dwType )
	{
	case FILE_BEGIN:
		dwSeek = lAmount;
		break;
	case FILE_CURRENT:
		dwSeek = m_dwRxBufSeek + lAmount;
		break;
	case FILE_END:
		dwSeek = m_dwSamplingLength - lAmount;
		break;
	default:return (DWORD)-1;
	}

	if( dwSeek >=0 && dwSeek <= m_dwSamplingLength )
	{
		m_dwRxBufSeek = dwSeek;
		return m_dwRxBufSeek;
	}
	return (DWORD)-1;
}	


DWORD WINAPI eta108Class::ADCEventHandle(LPVOID lpParameter)
{
	eta108Class *pETA108 = (eta108Class *)lpParameter;
	PWMINFO PwmInfo;
	DWORD dwNumberOfBytesToWrite; 
	DWORD dwNumberOfBytesWritten; 

	while( !pETA108->m_bTerminate )
	{
		WaitForSingleObject(pETA108->m_hCSPIEvent, INFINITE );
		if( pETA108->m_bTerminate )
			break;

		SetEvent( pETA108->m_hADCEvent );

		// stop PWM output
		PwmInfo.dwDuration = 1;			
		dwNumberOfBytesToWrite = sizeof(PWMINFO); 
		dwNumberOfBytesWritten = 0; 
		PwmInfo.dwFreq = 100;
		WriteFile(pETA108->m_hPWM, (LPCVOID)&PwmInfo, dwNumberOfBytesToWrite, &dwNumberOfBytesWritten, NULL); 
	}
	pETA108->m_hThread = NULL;
	return 0;
}