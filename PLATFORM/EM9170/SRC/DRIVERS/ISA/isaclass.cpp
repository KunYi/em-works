#include "windows.h"
#include "isaclass.h"


#define ENABLE  1
#define DISABLE 0

GPIO_INFO g_EM9170_GPIOTab[ ] =
{
	{DDK_IOMUX_PIN_CSPI1_MISO,	DDK_IOMUX_PAD_CSPI1_MISO,	DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  15 },		//GPIO 0
	{DDK_IOMUX_PIN_CSPI1_MOSI,	DDK_IOMUX_PAD_CSPI1_MOSI,	DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  14 },		//GPIO 1
	{DDK_IOMUX_PIN_CSI_D4,			DDK_IOMUX_PAD_CSI_D4,			DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  29 },		//GPIO 2
	{DDK_IOMUX_PIN_CSI_D2,			DDK_IOMUX_PAD_CSI_D2,			DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  27 },		//GPIO 3
	{DDK_IOMUX_PIN_CSI_D6,			DDK_IOMUX_PAD_CSI_D6,			DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  31 },		//GPIO 4
	{DDK_IOMUX_PIN_CSI_D3,			DDK_IOMUX_PAD_CSI_D3,			DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  28 },		//GPIO 5
	{DDK_IOMUX_PIN_I2C1_CLK,		DDK_IOMUX_PAD_I2C1_CLK,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  12 },		//GPIO 6
	{DDK_IOMUX_PIN_I2C1_DAT,		DDK_IOMUX_PAD_I2C1_DAT,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  13 },		//GPIO 7
	{DDK_IOMUX_PIN_CSI_D5,			DDK_IOMUX_PAD_CSI_D5,			DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  30 },		//GPIO 8
	{DDK_IOMUX_PIN_CSI_D7,			DDK_IOMUX_PAD_CSI_D7,			DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  6 },		//GPIO 9
	{DDK_IOMUX_PIN_GPIO_F,			DDK_IOMUX_PAD_GPIO_F,			DDK_IOMUX_PIN_MUXMODE_ALT0,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  5 },		//GPIO 10
	{DDK_IOMUX_PIN_GPIO_E,			DDK_IOMUX_PAD_GPIO_E,			DDK_IOMUX_PIN_MUXMODE_ALT0,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  4 },		//GPIO 11
	{DDK_IOMUX_PIN_CSPI1_SS1,		DDK_IOMUX_PAD_CSPI1_SS1,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  17 },		//GPIO 12
	{DDK_IOMUX_PIN_CSPI1_SS0,		DDK_IOMUX_PAD_CSPI1_SS0,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  16 },		//GPIO 13
	{DDK_IOMUX_PIN_UART1_RTS,	DDK_IOMUX_PAD_UART1_RTS,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT4,  24 },		//GPIO 14
	{DDK_IOMUX_PIN_CSI_VSYNC,	DDK_IOMUX_PAD_CSI_VSYNC,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  9 },		//GPIO 15
	{DDK_IOMUX_PIN_D13,				DDK_IOMUX_PAD_D13,					DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT4,  7 },		//GPIO 16
	{DDK_IOMUX_PIN_D12,				DDK_IOMUX_PAD_D12,					DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT4,  8 },		//GPIO 17
	{DDK_IOMUX_PIN_D9,					DDK_IOMUX_PAD_D9,					DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT4,  11 },		//GPIO 18
	{DDK_IOMUX_PIN_D8,					DDK_IOMUX_PAD_D8,					DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT4,  12 },		//GPIO 19
	{DDK_IOMUX_PIN_UPLL_BYPCLK,	DDK_IOMUX_PAD_NULL,				DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT3,  16 },		//GPIO 20
	{DDK_IOMUX_PIN_VSTBY_REQ,	DDK_IOMUX_PAD_VSTBY_REQ,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT3,  17 },		//GPIO 21
	{DDK_IOMUX_PIN_VSTBY_ACK,	DDK_IOMUX_PAD_VSTBY_ACK,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT3,  18 },		//GPIO 22
	{DDK_IOMUX_PIN_POWER_FAIL,	DDK_IOMUX_PAD_POWER_FAIL,	DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT3,  19 },		//GPIO 23
	{DDK_IOMUX_PIN_CSI_MCLK,		DDK_IOMUX_PAD_CSI_MCLK,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  8 },		//GPIO 24
	{DDK_IOMUX_PIN_CSI_PIXCLK,	DDK_IOMUX_PAD_CSI_PIXCLK,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  11 },		//GPIO 25
	{DDK_IOMUX_PIN_CSI_HSYNC,	DDK_IOMUX_PAD_CSI_HSYNC,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  10 },		//GPIO 26
	{DDK_IOMUX_PIN_CLKO,				DDK_IOMUX_PAD_CLKO,				DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT2,  21 },		//GPIO 27
	{DDK_IOMUX_PIN_CSPI1_RDY,		DDK_IOMUX_PAD_CSPI1_RDY,		DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT2,  22 },		//GPIO 28
	{DDK_IOMUX_PIN_EXT_ARMCLK,	DDK_IOMUX_PAD_NULL,				DDK_IOMUX_PIN_MUXMODE_ALT5,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT3,  15 },		//GPIO 29
	{DDK_IOMUX_PIN_GPIO_A,			DDK_IOMUX_PAD_GPIO_A,			DDK_IOMUX_PIN_MUXMODE_ALT0,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  0 },		//GPIO 30
	{DDK_IOMUX_PIN_GPIO_C,			DDK_IOMUX_PAD_GPIO_C,			DDK_IOMUX_PIN_MUXMODE_ALT0,  DDK_IOMUX_PIN_SION_REGULAR, DDK_GPIO_PORT1,  2 },		//GPIO 31
};

ISAClass::ISAClass( UINT32 index )
{
    PHYSICAL_ADDRESS phyAddr;

	if( index==1 )    // for EM9170
		pGPIOTab = g_EM9170_GPIOTab;
	else
		pGPIOTab = NULL;

	// setup pointer of CS5 in EM9170
	dwISACtrlReg = 0;
    phyAddr.QuadPart = CSP_BASE_MEM_PA_CS5;
	dwCPLDSize = 128;												// we only use A0 - A6
	pCPLD = (PEM9K_CPLD_REGS)MmMapIoSpace(phyAddr, dwCPLDSize, FALSE);	
	if(pCPLD == NULL)
	{
		RETAILMSG (1, (TEXT("ISA_Init::pCPLD == NULL!!!\r\n")));
	}
	else
	{
		OUTREG8 (&pCPLD->ISACtrlReg, (UCHAR)dwISACtrlReg);				// disable ISA operations
	}

	// enable all GPIO -> 32-bit GPIO!
	dwGPIOEnable = 0xFFFFFFFF;

	InitializeCriticalSection(&gcsISABusLock);	
}

ISAClass::~ISAClass(void)
{
	if(pCPLD)
	{
		MmUnmapIoSpace(pCPLD, dwCPLDSize);
		pCPLD = NULL;
		dwCPLDSize = 0;
	}

	DeleteCriticalSection(&gcsISABusLock);
	pGPIOTab = NULL;
}

//
// GPIO functions
//
int	ISAClass::PIO_OutEnable( UINT32 EnBits )
{
	int		i1;
	if( pGPIOTab==NULL )
		return -1;

	// clear all bits which are NOT enable
	EnBits &= dwGPIOEnable;

    EnterCriticalSection(&gcsISABusLock);
	for( i1=0; i1<sizeof(g_EM9170_GPIOTab)/sizeof(GPIO_INFO); i1++ )
	{
		if( !( EnBits&(1<<i1)) )
		{
			continue;
		}
		// Configure PIO for output
		DDKIomuxSetPinMux( pGPIOTab[i1].iomux_pin,  pGPIOTab[i1].muxmode, pGPIOTab[i1].sion );
		if( pGPIOTab[i1].iomux_pad != DDK_IOMUX_PAD_NULL )
		{
			DDKIomuxSetPadConfig(pGPIOTab[i1].iomux_pad, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
				                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_UP_100K,  DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
											DDK_IOMUX_PAD_VOLTAGE_3V3);
		}
		DDKGpioSetConfig(pGPIOTab[i1].gpioport, pGPIOTab[i1].gpio_pin, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
	}
    LeaveCriticalSection(&gcsISABusLock); 
	return 0;
}

int ISAClass::PIO_OutDisable( UINT32 DisBits )
{
	int   i1;
	if( pGPIOTab==NULL )
		return -1;

	// clear all bits which are NOT enable
	DisBits &= dwGPIOEnable;

    EnterCriticalSection(&gcsISABusLock);
	for( i1=0; i1<sizeof(g_EM9170_GPIOTab)/sizeof(GPIO_INFO); i1++ )
	{
		if( !( DisBits&(1<<i1)) )
		{
			continue;
		}
		// Configure PIO for input
		DDKIomuxSetPinMux( pGPIOTab[i1].iomux_pin,  pGPIOTab[i1].muxmode, pGPIOTab[i1].sion );
		if( pGPIOTab[i1].iomux_pad != DDK_IOMUX_PAD_NULL )
		{
			DDKIomuxSetPadConfig(pGPIOTab[i1].iomux_pad, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
				                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_UP_100K,  DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
											DDK_IOMUX_PAD_VOLTAGE_3V3);
		}
		DDKGpioSetConfig(pGPIOTab[i1].gpioport, pGPIOTab[i1].gpio_pin, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);
	}
    LeaveCriticalSection(&gcsISABusLock); 
	return 0;
}

int ISAClass::PIO_OutSet( UINT32 SetBits )
{
	int  i1;
	if( pGPIOTab==NULL )
		return -1;

	// clear all bits which are NOT enable
	SetBits &= dwGPIOEnable;

    EnterCriticalSection(&gcsISABusLock);
	for( i1=0; i1<sizeof(g_EM9170_GPIOTab)/sizeof(GPIO_INFO); i1++ )
	{
		if( !( SetBits&(1<<i1)) )
		{
			continue;
		}
		// set 1
		DDKGpioWriteDataPin( pGPIOTab[i1].gpioport,  pGPIOTab[i1].gpio_pin, ENABLE );
	}
    LeaveCriticalSection(&gcsISABusLock); 
	return 0;
}

int ISAClass::PIO_OutClear( UINT32 ClearBits )
{
	int   i1;
	if( pGPIOTab==NULL )
		return -1;

	// clear all bits which are NOT enable
	ClearBits &= dwGPIOEnable;

    EnterCriticalSection(&gcsISABusLock);
	for( i1=0; i1<sizeof(g_EM9170_GPIOTab)/sizeof(GPIO_INFO); i1++ )
	{
		if( !( ClearBits&(1<<i1)) )
		{
			continue;
		}
		// set 0
		DDKGpioWriteDataPin( pGPIOTab[i1].gpioport,  pGPIOTab[i1].gpio_pin, DISABLE );
	}

    LeaveCriticalSection(&gcsISABusLock); 
	return 0;
}

int ISAClass::PIO_State( UINT32* pInValue )
{
	int			i1;
	UINT32  uData;
	UINT32  uGPIOState = 0;

	if( pGPIOTab==NULL )
		return -1;

    EnterCriticalSection(&gcsISABusLock);
	for( i1=0; i1<sizeof(g_EM9170_GPIOTab)/sizeof(GPIO_INFO); i1++ )
	{
		uData = 0;
		DDKGpioReadDataPin( pGPIOTab[i1].gpioport,  pGPIOTab[i1].gpio_pin,  &uData );
		if( uData )
			uGPIOState |= (1<<i1 );
	}

	*pInValue = uGPIOState;

    LeaveCriticalSection(&gcsISABusLock); 
	return 0;
}

//
// ISA read/write functions
//
int ISAClass::ISA_ReadUchar( int nSeg, UINT nOffset, UCHAR* pRdValue )
{
	PBYTE	pThisAddr;
	int			i1;

    EnterCriticalSection(&gcsISABusLock);
	if( !(dwISACtrlReg & EM9K_CPLD_STATE_ISA_EN))
	{
		// Configure GPIO16 - GPIO23 as input
		for(i1 = 16; i1 <= 23; i1++)
		{
			DDKIomuxSetPinMux( pGPIOTab[i1].iomux_pin,  pGPIOTab[i1].muxmode, pGPIOTab[i1].sion );
			if( pGPIOTab[i1].iomux_pad != DDK_IOMUX_PAD_NULL )
			{
				DDKIomuxSetPadConfig(pGPIOTab[i1].iomux_pad, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
												DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_UP_100K,  DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
												DDK_IOMUX_PAD_VOLTAGE_3V3);
			}
			DDKGpioSetConfig(pGPIOTab[i1].gpioport, pGPIOTab[i1].gpio_pin, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);
			
			dwGPIOEnable &= ~(1 << i1);												// clear corespondent bit
		}

		dwISACtrlReg |= EM9K_CPLD_STATE_ISA_EN;
		OUTREG8 (&pCPLD->ISACtrlReg, (UCHAR)dwISACtrlReg);				// enable ISA operations
	}

	if(nSeg == 0)
	{
		if( !(dwISACtrlReg & EM9K_CPLD_STATE_CS0_EN))
		{
			dwISACtrlReg |= EM9K_CPLD_STATE_CS0_EN;
			OUTREG8 (&pCPLD->ISACtrlReg, (UCHAR)dwISACtrlReg);			// enable ISA_CS0 operations
		}
		pThisAddr = &pCPLD->ISA_CS0[nOffset & 0x0F];							// A0 - A3
	}
	else			// -> ISA_CS1
	{
		if( !(dwISACtrlReg & EM9K_CPLD_STATE_CS0_EN))
		{
			pThisAddr = &pCPLD->ISA_CS0[0] + (nOffset & 0x1F);				// A0 - A4
		}
		else
		{
			pThisAddr = &pCPLD->ISA_CS1[nOffset & 0x0F];						// A0 - A3
		}
	}
    
	// make ISA read finally
	*pRdValue = INREG8(pThisAddr);
	LeaveCriticalSection(&gcsISABusLock); 
	
	return 0;
}

int	ISAClass::ISA_WriteUchar( int nSeg, UINT nOffset, UCHAR WrValue )
{
	PBYTE	pThisAddr;
	int			i1;

    EnterCriticalSection(&gcsISABusLock);
	if( !(dwISACtrlReg & EM9K_CPLD_STATE_ISA_EN))
	{
		// Configure GPIO16 - GPIO23 as input
		for(i1 = 16; i1 <= 23; i1++)
		{
			DDKIomuxSetPinMux( pGPIOTab[i1].iomux_pin,  pGPIOTab[i1].muxmode, pGPIOTab[i1].sion );
			if( pGPIOTab[i1].iomux_pad != DDK_IOMUX_PAD_NULL )
			{
				DDKIomuxSetPadConfig(pGPIOTab[i1].iomux_pad, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
												DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_UP_100K,  DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
												DDK_IOMUX_PAD_VOLTAGE_3V3);
			}
			DDKGpioSetConfig(pGPIOTab[i1].gpioport, pGPIOTab[i1].gpio_pin, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);
			
			dwGPIOEnable &= ~(1 << i1);												// clear corespondent bit
		}

		dwISACtrlReg |= EM9K_CPLD_STATE_ISA_EN;
		OUTREG8 (&pCPLD->ISACtrlReg, (UCHAR)dwISACtrlReg);				// enable ISA operations
	}

	if(nSeg == 0)
	{
		if( !(dwISACtrlReg & EM9K_CPLD_STATE_CS0_EN))
		{
			dwISACtrlReg |= EM9K_CPLD_STATE_CS0_EN;
			OUTREG8 (&pCPLD->ISACtrlReg, (UCHAR)dwISACtrlReg);			// enable ISA_CS0 operations
		}
		pThisAddr = &pCPLD->ISA_CS0[nOffset & 0x0F];							// A0 - A3
	}
	else			// -> ISA_CS1
	{
		if( !(dwISACtrlReg & EM9K_CPLD_STATE_CS0_EN))
		{
			pThisAddr = &pCPLD->ISA_CS0[0] + (nOffset & 0x1F);				// A0 - A4
		}
		else
		{
			pThisAddr = &pCPLD->ISA_CS1[nOffset & 0x0F];						// A0 - A3
		}
	}
    
	// make ISA write finally
	OUTREG8 (pThisAddr, WrValue);
	LeaveCriticalSection(&gcsISABusLock); 

	return 0;
}


//
// GPIO15 can be used as ISA_RSTOUTn, active low
//
int ISAClass::ISA_Reset( UINT nMilliseconds )
{
	PIO_OutEnable(GPIO15);
	PIO_OutClear(GPIO15);
	Sleep(nMilliseconds);
	PIO_OutSet(GPIO15);

	return 0;
}
