//------------------------------------------------------------------------------
//
//  Copyright (C) 2011,Emtronix, Inc. All Rights Reserved.
//
//------------------------------------------------------------------------------

#include "bsp.h"
#include "mkeypadclass.h"

//Defines
#define  ALLHIGH 0xff

//-----------------------------------------------------
// EM9283 V1.0 keypad in Table
//-----------------------------------------------------
DDK_IOMUX_PIN g_dwKeyIn[ ] =
{
	DDK_IOMUX_ENET0_MDC,			// KEYIN0
	DDK_IOMUX_ENET0_MDIO,			// KEYIN1
	DDK_IOMUX_ENET0_RX_EN,			// KEYIN2
	DDK_IOMUX_ENET0_RXD0,			// KEYIN3
	DDK_IOMUX_ENET0_RXD1,			// KEYIN4
};

//-----------------------------------------------------
// EM9283 V1.0 keypad out Table
//-----------------------------------------------------
DDK_IOMUX_PIN g_dwKeyOut[ ] =
{
	DDK_IOMUX_LCD_RESET,			// KEYOUT0
	DDK_IOMUX_JTAG_RTCK,			// KEYOUT1
	DDK_IOMUX_ENET0_TX_EN,			// KEYOUT2
	DDK_IOMUX_ENET0_TXD0,			// KEYOUT3
	DDK_IOMUX_ENET0_TXD1,			// KEYOUT4
};

//------------------------------------------------------------------------------
// Global Variables
enum KEYSTATE  { KEYREADY=0, KEYPRESS, KEYRELEASE, KEYSTART };

//--------------------------------------------------------------------------------
// Global Functions
void MKeyPadHandleThread( MKPDClass *pMKpd )
{
	pMKpd->MKeyPadHandle( );
}
//--------------------------------------------------------------------------------


MKPDClass::MKPDClass( UINT32 dwPollingTimeOut )
{
	KeyIoInit( );
	m_dwPollingTimeOut = dwPollingTimeOut;
	m_nkeyState = KEYSTART;

	g_bMkeyPadIsOpen = TRUE;
	m_hThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL, 0, (LPTHREAD_START_ROUTINE)MKeyPadHandleThread, this, 0, NULL);
	// check if CreateThread failed
	if (m_hThread == NULL)
	{
		RETAILMSG (1, (TEXT("MKPDClass:: Create key handle thread failed\r\n")));
		g_bMkeyPadIsOpen = FALSE;
		goto error;
	}
	CeSetThreadPriority(m_hThread, 145 );		// PS2 KeyBoard Priority
	CloseHandle(m_hThread);

error:;
}

MKPDClass::~MKPDClass(void)
{
	g_bMkeyPadIsOpen = FALSE;
	KeyIoDeInit( );
	Sleep(100);
	RETAILMSG (1, (TEXT("MKPDClass::~MKPDClass:MKPD_DeInit\r\n")));
}


void MKPDClass::MKeyPadHandle()
{
	BYTE Din,Dout, i;
	UINT VKey, uScanCode;
	DWORD dwFlags;

	Dout = 0;
	VKey = 0;
	uScanCode = 0;
	dwFlags = 0;

	while( g_bMkeyPadIsOpen )
	{
		switch( m_nkeyState )
		{
		case KEYREADY:
			Din = GetDin(  );
			if( Din != ALLHIGH ) 
			{
				for( i=0; i<MAXOUT; i++ )
				{
					Dout = i;
					PutDout( Dout );
					Din = GetDin( );
					if( Din != ALLHIGH ) 
						break;
				}
				if( i < MAXOUT )
				{
					m_ucScanWord = Din;
					m_nkeyState = KEYPRESS;
				}
				else
				{
					m_nkeyState = KEYSTART;
				}
			}
			break;

		case KEYPRESS:
			Din = GetDin( );
			if( Din == m_ucScanWord )		
			{
				switch(  Din )
				{
				case 0xfe: i = 0; break;
				case 0xfd: i = 1; break;
				case 0xfb: i = 2; break;
				case 0xf7: i = 3; break;
				case 0xef: i = 4; break;
				//case 0xdf: i = 5; break;
				//case 0xbf: i = 6; break;
				//case 0x7f: i = 7; break;
				default: goto IDONNOTKNOW_KEY;
				}
				VKey = (UINT)g_uVkeyCode[(i*MAXOUT)+Dout];
				uScanCode = MapVirtualKey(VKey, 0);
				//RETAILMSG (1, (TEXT("Vkey:0x%x ScanCode:0x%x\r\n"), VKey, uScanCode));
				//send keydown and keychar message 
				dwFlags = 0;
				if (uScanCode & 0xFF00)
				{
					dwFlags |= KEYEVENTF_EXTENDEDKEY;
				}

				keybd_event((BYTE)VKey, (BYTE)(uScanCode & 0xFF), dwFlags, 0); 
				SetDelay( 50 );
				m_nkeyState = KEYRELEASE;
			}
			else
			{
IDONNOTKNOW_KEY:
				m_nkeyState = KEYSTART;
			}
			break;
		case KEYRELEASE:
			//ÅÐ¶Ï°´¼üÊÍ·Å
			Din = GetDin(  );
			if( Din == m_ucScanWord ) 
			{
				//send keydown message
				if( 0 == IsTimeOut())
				{
					keybd_event((BYTE)VKey, (BYTE)(uScanCode & 0xFF), dwFlags, 0); 
					SetDelay( 10 );			//send keydown message pre 100ms
				}
			}
			else
			{
				//send keyup message
				dwFlags = 0;
				dwFlags |= KEYEVENTF_KEYUP; 
				if (uScanCode & 0xFF00)
				{
					dwFlags |= KEYEVENTF_EXTENDEDKEY;
				}
				keybd_event((BYTE)VKey, (BYTE)(uScanCode & 0xFF), dwFlags, 0); 
				m_nkeyState = KEYSTART;
			}
			break;
		case KEYSTART:
			PutDoutAll( 0 );
			m_nkeyState = KEYREADY;
			break;
		}
		Sleep( m_dwPollingTimeOut );                    // Polling per 10 millisecond
	}
	RETAILMSG (1, (TEXT("MKPDClass::MKeyPadHandle Exit!\r\n")));

}

void MKPDClass::SetDelay( int nDelay )
{
	m_nKeyDelay = nDelay;
}

int MKPDClass::IsTimeOut( )
{
	if( 0 != m_nKeyDelay )
		m_nKeyDelay--;
	return m_nKeyDelay;
}


BYTE MKPDClass::GetDin()
{
	BYTE uVal;
	UINT32 uData;
	int i;

	uVal = 0xff;
	
	for( i=0; i<MAXIN; i++ )
	{
		DDKGpioReadDataPin(g_dwKeyIn[MAXIN-i-1], &uData);
		uVal <<= 1;
		uVal |= (uData&0x1);
	}
	return uVal;
}

void MKPDClass::PutDout( int nIdx, int nVal )
{
	PutDoutAll( 0xff );
	DDKGpioWriteDataPin(g_dwKeyOut[nIdx], nVal );
}

void MKPDClass::PutDoutAll( int nVal )
{
	int i;

	for( i=0; i<MAXOUT; i++ )
	{
		DDKGpioWriteDataPin(g_dwKeyOut[i],nVal );
	}
}

void MKPDClass::KeyIoInit()
{
	int i;
	
	// initializtion Key in pin
	for( i=0; i<MAXIN; i++ )
	{
		DDKIomuxSetPinMux(g_dwKeyIn[i],DDK_IOMUX_MODE_GPIO);
		DDKIomuxSetPadConfig(g_dwKeyIn[i], 
			DDK_IOMUX_PAD_DRIVE_4MA, 
			DDK_IOMUX_PAD_PULL_ENABLE,
			DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKGpioEnableDataPin( g_dwKeyIn[i], 0 );
	}

	// initializtion Key out pin
	for( i=0; i<MAXOUT; i++ )
	{
		DDKIomuxSetPinMux(g_dwKeyOut[i],DDK_IOMUX_MODE_GPIO);
		DDKIomuxSetPadConfig(g_dwKeyOut[i], 
			DDK_IOMUX_PAD_DRIVE_4MA, 
			DDK_IOMUX_PAD_PULL_ENABLE,
			DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKGpioEnableDataPin( g_dwKeyOut[i], 1 );
	}

}

void MKPDClass::KeyIoDeInit( )
{
	for( int i=0; i<MAXOUT; i++ )
		DDKGpioEnableDataPin( g_dwKeyOut[i], 0 );
}