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
#ifndef __ETA108CALSS_H__
#define __ETA108CALSS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ETA108.h"
#include "SPIClass.h"

//------------------------------------------------------------------------------
//Defines
#define ADS8021_CHA0_1_CCR			(0<<10)
#define ADS8021_CHA2_3_CCR			(1<<10)
#define ADS8021_CHA4_5_CCR			(2<<10)
#define ADS8021_CHA6_7_CCR			(3<<10)
#define ADS8021_CHA_SEL_CCR			(4<<10)
#define ADS8021_ADC_SCR				(5<<10)
#define ADS8021_INT_SCR				(6<<10)
#define ADS8021_STATUS_SCR			(7<<10)
#define ADS8021_ADC_TRIGGER_SCR		(8<<10)
#define ADS8021_REST_SCR			(9<<10)
#define	ADS8021_CONV_DELAY_SCR		(10<<10)

#define	ADS8201_REG_READ			(0x4000)		
#define ADS8201_REG_WRITE			(0x8000)		
#define ADS8201_ADC_READ			(0x0000)	
#define ADS8201_CPLD_WRITE			(0xc000)
//------------------------------------------------------------------------------
//Types
typedef struct {
	PVOID  m_pLocalAsync;
	PVOID  m_pLocalSyncMarshalled;
	PVOID  m_pCallerUnmarshalled;
	DWORD  m_cbSize;
	DWORD  m_ArgumentDescriptor;
}CALLER_STUB_T, *PCALLER_STUB_T;

typedef struct
{
	DWORD dwFreq;
	DWORD dwDuty ;
	DWORD dwDuration;
}PWMINFO ;

typedef struct{
	BYTE cha0_1_ccr;
	BYTE cha2_3_ccr;
	BYTE cha4_5_ccr;
	BYTE cha6_7_ccr;
	BYTE cha_sel_ccr;
	BYTE adc_scr;
	BYTE int_scr;
	BYTE status_scr;
	BYTE adc_trigger_scr;
	BYTE rest_scr;
	BYTE conv_delay_scr;
}ADS8201_CFG;

class eta108Class
{
public:
	eta108Class();
	~eta108Class();
	BOOL ETA108Initialize( void );
	BOOL ETA108Release( void );
	BOOL ETA108Open( );
	BOOL ETA108Close( );
	DWORD ETA108Run( PADS_CONFIG pBuffer);
	DWORD ETA108Read( LPVOID pBuffer, DWORD dwCount );
	DWORD ReadSeek( long lAmount, WORD dwType );

public:
	DWORD  m_dwCSPIChannle;	
	DWORD  m_dwPWMChannle;
	DWORD  m_dwDMABufSize;
	DWORD  m_dwMultDmaBufSize;	//Buffer Size of Continuous Sampling 
	spiClass *m_pSpi;

private:
	HANDLE m_hPWM;			// PWM:
	HANDLE m_hThread;		// ADC moniter thread
	HANDLE m_hADCEvent;		// ADC completed event
	HANDLE m_hCSPIEvent;	// SPI transfer completed event
	BOOL   m_bWriteBlock;
	DWORD  m_dwRxBufSeek;
	DWORD  m_dwSamplingLength;
	ADS8201_CFG m_stADS8201CFG;
	BOOL   m_bTerminate;	// ADC moniter thread exit event
	static DWORD WINAPI ADCEventHandle(LPVOID lpParameter);
};





#ifdef __cplusplus
}
#endif

#endif __ETA108CALSS_H__