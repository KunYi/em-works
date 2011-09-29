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
//	m_pCspi = NULL;

	m_hCSPI = INVALID_HANDLE_VALUE;
	m_hPWM = INVALID_HANDLE_VALUE;
	
	m_hHeap = NULL;
	m_pSPIRxBuf = NULL;
	m_pSPITxBuf = NULL;
	m_hThread = NULL;
	m_hADCEvent = NULL;
	m_hCSPIEvent = NULL;

	dwOpenCount = 0;
}

eta108Class::~eta108Class()
{

}

BOOL eta108Class::ETA108Initialize()
{
	//crate global heap for internal AD buffers
	m_hHeap = HeapCreate(0, 0, 0);
	if( m_hHeap == NULL )
	{
		goto error_init;
	}

	// create event for CSPI interrupt signaling
	m_hCSPIEvent = CreateEvent( NULL, FALSE, FALSE, g_szCSPIEvent );
	if( m_hCSPIEvent == NULL )
		goto error_init;

	if( !m_hThread )
	{
		m_bTerminate = FALSE;
		m_hThread = ::CreateThread(NULL, 0, ADCEventHandle, this, 0, NULL);
		if( m_hThread == NULL )
			goto error_init;
	}

// 	m_pSpi = new spiClass;
// 	if (m_pSpi && !m_pSpi->CspiInitialize(1)) 
// 	{
// 		goto error_init;
// 	}

	return TRUE;

error_init:
	ETA108Release( );
	return FALSE;
}

BOOL eta108Class::ETA108Release()
{
// 	if( m_pCspi )
// 	{
// 		m_pCspi->CspiRelease( );
// 		delete m_pCspi;
// 		m_pCspi = NULL;
// 	}

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

	if( m_hHeap != NULL )
	{
		HeapDestroy(m_hHeap);
		m_hHeap = NULL;
	}

	return TRUE;
}

BOOL eta108Class::ETA108Open()
{
	m_hCSPI = CreateFile(_T("SPI1:"),         // name of device
		GENERIC_READ|GENERIC_WRITE,         // desired access
		FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
		NULL,                               // security attributes (ignored)
		OPEN_EXISTING,                      // creation disposition
		FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
		NULL);                              // template file (ignored)

	// if we failed to get handle to CSPI
	if (m_hCSPI == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	m_hPWM = CreateFile(_T("PWM1:"),          // name of device
		GENERIC_READ|GENERIC_WRITE,         // desired access
		FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
		NULL,                               // security attributes (ignored)
		OPEN_EXISTING,                      // creation disposition
		FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
		NULL);   

	// if we failed to get handle to CSPI
	if (m_hPWM == INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hCSPI);
		m_hCSPI = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	return TRUE;
}

BOOL eta108Class::ETA108Close()
{
	if( m_hADCEvent != NULL )
	{
		CloseHandle(m_hADCEvent);
		m_hADCEvent = NULL;
	}

	if( m_pSPITxBuf != NULL )
	{
		HeapFree( m_hHeap, 0, m_pSPITxBuf );
		m_pSPITxBuf = NULL;
	}
	if( m_pSPIRxBuf != NULL )
	{
		HeapFree( m_hHeap, 0, m_pSPIRxBuf );
		m_pSPIRxBuf = NULL ;
	}

	if( m_hCSPI != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hCSPI );
		m_hCSPI = INVALID_HANDLE_VALUE;
	}
	if( m_hPWM != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hPWM);
		m_hPWM = INVALID_HANDLE_VALUE;
	}
	return TRUE;
}

BOOL eta108Class::ETA108Run( PADS_CONFIG pADSConfig )
{
	CALLER_STUB_T marshalEventStub;
	HRESULT result;
	DWORD  dwIdx,i,k;
	UINT8  Channle[8];

	if( m_hCSPI == INVALID_HANDLE_VALUE || m_hPWM == INVALID_HANDLE_VALUE )
		return FALSE;
	//1.Parameter marshall
	if( pADSConfig->dwADCompleteEventLength > 0 && pADSConfig->lpADCompleteEvent )
	{
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

		m_hADCEvent = CreateEvent(NULL,TRUE, FALSE, pADSConfig->lpADCompleteEvent );
		CeCloseCallerBuffer(marshalEventStub);

		if( m_hADCEvent == NULL )
		{
			goto error_cleanup;
		}
	}
	else 	return FALSE;;

	//2.Config CSPI & config ADS8201
	CSPI_BUSCONFIG_T stCspiConfig;
	CSPI_XCH_PKT_T stCspiXchPkt;

	stCspiConfig.bitcount = 16;		//data rate = 16bit
	stCspiConfig.chipselect = 0;	//use channel 0
	stCspiConfig.freq = 16000000;	//XCH speed = 16M
	stCspiConfig.pha = FALSE;
	stCspiConfig.pol = FALSE;
	stCspiConfig.ssctl = TRUE;		//one entry entry with each SPI burst
	stCspiConfig.sspol = TRUE;		//SSPOL Active high
	stCspiConfig.usepolling = FALSE;//Don't polling
	
	stCspiConfig.drctl = 0;			//Don't care SPI_RDY
	stCspiConfig.usedma = FALSE;	//Don't DMA

	stCspiXchPkt.pBusCnfg = &stCspiConfig;
	stCspiXchPkt.xchEvent = NULL;
	stCspiXchPkt.xchEventLength = 0;

	m_dwSamplingLength = pADSConfig->dwSamplingLength;
	m_dwXchBufLen = m_dwSamplingLength<<1;	//m_dwSamplingLength*2
	m_pSPITxBuf = (UINT16 *)HeapAlloc( m_hHeap, HEAP_ZERO_MEMORY,  m_dwXchBufLen );
	if( m_pSPITxBuf == NULL )
		goto error_cleanup;
	m_pSPIRxBuf = (UINT16 *)HeapAlloc( m_hHeap, HEAP_ZERO_MEMORY,  m_dwXchBufLen );
	if( m_pSPIRxBuf == NULL )
		goto error_cleanup;

	stCspiXchPkt.pTxBuf = (LPVOID)m_pSPITxBuf;
	stCspiXchPkt.pRxBuf = (LPVOID)m_pSPIRxBuf;

	//ADS8201 Configuration parameter
	if( pADSConfig->dwContrlWordLength == 0 || pADSConfig->lpContrlWord == NULL )
	{
		//ADS8201 default configuration
		memset( &m_stADS8201CFG, 0, sizeof(m_stADS8201CFG));
	}
	stCspiXchPkt.xchCnt = 0;
	m_pSPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE| ADS8021_CHA0_1_CCR | m_stADS8201CFG.cha0_1_ccr;
	m_pSPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE| ADS8021_CHA2_3_CCR | m_stADS8201CFG.cha2_3_ccr;
	m_pSPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE| ADS8021_CHA4_5_CCR | m_stADS8201CFG.cha4_5_ccr;
	m_pSPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE| ADS8021_CHA6_7_CCR | m_stADS8201CFG.cha6_7_ccr;
	//Skip ADS8201's Channel Select Register[04h]
	m_pSPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE| ADS8021_ADC_SCR | m_stADS8201CFG.adc_scr;
	m_pSPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE| ADS8021_INT_SCR | m_stADS8201CFG.int_scr;
	//Skip ADS8201's Status SCR[07h]
	m_pSPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE| ADS8021_ADC_TRIGGER_SCR | m_stADS8201CFG.adc_trigger_scr;
	//Skip ADS8201's Reset SCR[09h]
	m_pSPITxBuf[stCspiXchPkt.xchCnt++] = ADS8201_REG_WRITE| ADS8021_CONV_DELAY_SCR | m_stADS8201CFG.conv_delay_scr;

	//ADS8201 configuration
	if (!DeviceIoControl(m_hCSPI,     // file handle to the driver
		CSPI_IOCTL_EXCHANGE,         // I/O control code
		&stCspiXchPkt,               // in buffer
		sizeof(CSPI_XCH_PKT_T),      // in buffer size
		NULL,                        // out buffer
		0,                           // out buffer size
		NULL,                        // pointer to number of bytes returned
		NULL))                       // ignored (=NULL)
	{
		goto error_cleanup;
	}	

	//Prepare Sampling Channel
	for( k=0, i=0; k<8; i++ )
	{
		if( pADSConfig->dwSamplingChannel & (0x01<<i))
			Channle[k++] = (UINT8)i;
	}
	//Prepare for TX buffer
	for( dwIdx=0,i=0; dwIdx<pADSConfig->dwSamplingLength; dwIdx++ )
	{
		m_pSPITxBuf[dwIdx] = ADS8201_ADC_READ;			//Read ADC Data
		m_pSPITxBuf[dwIdx+1] =	ADS8201_REG_WRITE|ADS8021_CHA_SEL_CCR|Channle[i];		//Config channel
		i++;
		i %= k;
	}

	//Create event for AD conversion completed.
	m_hADCEvent = CreateEvent( NULL, FALSE, FALSE, pADSConfig->lpADCompleteEvent );
	if( m_hADCEvent == NULL )
		goto error_cleanup;
	
	//Redefine SPI bus configuration
	stCspiConfig.drctl = 1;			//Use SPI_RDY
	stCspiConfig.usedma = TRUE;		//Use DMA
	stCspiXchPkt.xchEvent = g_szCSPIEvent;
	stCspiXchPkt.xchEventLength = wcslen( g_szCSPIEvent );

	//Prepare to AD conversion
	if (!DeviceIoControl(m_hCSPI,     // file handle to the driver
		CSPI_IOCTL_EXCHANGE,         // I/O control code
		&stCspiXchPkt,               // in buffer
		sizeof(CSPI_XCH_PKT_T),      // in buffer size
		NULL,                        // out buffer
		0,                           // out buffer size
		NULL,                        // pointer to number of bytes returned
		NULL))                       // ignored (=NULL)
	{
		goto error_cleanup;
	}	
	//3.Config PWM
	PWMINFO PwmInfo; 
	DWORD dwNumberOfBytesToWrite; 
	DWORD dwNumberOfBytesWritten; 
	BOOL bRet; 

	PwmInfo.dwFreq = pADSConfig->dwSamplingRate;
	PwmInfo.dwDuty = 1;				// PWM Duty cycle = 50% 
	PwmInfo.dwDuration = 0;			// Remain output till issue a new write operation
	dwNumberOfBytesToWrite = sizeof(PWMINFO); 
	dwNumberOfBytesWritten = 0; 
	//4.Now start AD conversion
	bRet = WriteFile(m_hPWM, (LPCVOID)&PwmInfo, dwNumberOfBytesToWrite, &dwNumberOfBytesWritten, NULL); 
	if( !bRet )
		goto error_cleanup;
	dwOpenCount	= 1;
	return TRUE;

error_cleanup:
	ETA108Close();
	return FALSE;

}

BOOL eta108Class::ETA108Read( UINT16* pBuffer, DWORD dwCount )
{
	DWORD dwIdx, i;
	BOOL bRet;
	if( dwOpenCount == 0)
		return FALSE;
	i = m_dwRxBufSeek+dwCount;
	if(  i>=0 && i<= m_dwSamplingLength )
	{
		for( dwIdx=0, i=0; dwIdx<m_dwXchBufLen; )
		{
			m_pSPIRxBuf[i++] = m_pSPIRxBuf[++dwIdx];
			dwIdx++;
		}
		memcpy( pBuffer, m_pSPIRxBuf+m_dwRxBufSeek, dwCount<<1 );
		bRet = TRUE;
	}
	else bRet = FALSE;

	if( m_hADCEvent != NULL )
	{
		CloseHandle(m_hADCEvent);
		m_hADCEvent = NULL;
	}

	if( m_pSPITxBuf != NULL )
	{
		HeapFree( m_hHeap, 0, m_pSPITxBuf );
		m_pSPITxBuf = NULL;
	}
	if( m_pSPIRxBuf != NULL )
	{
		HeapFree( m_hHeap, 0, m_pSPIRxBuf );
		m_pSPIRxBuf = NULL ;
	}
	dwOpenCount = 0;
	return bRet;
}

DWORD eta108Class::ReadSeek( long lAmount, WORD dwType )
{
	DWORD dwSeek;

	dwSeek = m_dwRxBufSeek;
	switch( dwType )
	{
	case FILE_BEGIN:
		dwSeek = lAmount;
		break;
	case FILE_CURRENT:
		dwSeek += lAmount;
		break;
	case FILE_END:
		dwSeek = m_dwSamplingLength + lAmount;
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
		if( !pETA108->m_bTerminate )
			break;
		SetEvent( pETA108->m_hADCEvent );

		PwmInfo.dwDuration = 1;			// 
		dwNumberOfBytesToWrite = sizeof(PWMINFO); 
		dwNumberOfBytesWritten = 0; 
		WriteFile(pETA108->m_hPWM, (LPCVOID)&PwmInfo, dwNumberOfBytesToWrite, &dwNumberOfBytesWritten, NULL); 
	}
	pETA108->m_hThread = NULL;
	return 0;
}