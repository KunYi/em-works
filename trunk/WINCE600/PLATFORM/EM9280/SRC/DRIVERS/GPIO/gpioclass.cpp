#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <cmnintrin.h>
#pragma warning(pop)

#include "bsp.h"
#include "em9280_oal.h"
#include "gpioclass.h"
#include "tca6424a_i2c.h"

//-----------------------------------------------------
// EM9280 V1.0 GPIO Pin Table
//-----------------------------------------------------
DDK_IOMUX_PIN g_EM9280_iMXGpioPin[32] =
{
	DDK_IOMUX_GPIO3_20,			// EM9280_GPIO0
	DDK_IOMUX_GPIO3_21,			// EM9280_GPIO1
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO2 -> TCA6424A-P0.0
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO3 -> TCA6424A-P0.1
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO4 -> TCA6424A-P0.2
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO5 -> TCA6424A-P0.3
	DDK_IOMUX_GPIO3_22,			// EM9280_GPIO6
	DDK_IOMUX_GPIO3_23,			// EM9280_GPIO7
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO8 -> TCA6424A-P0.4
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO9 -> TCA6424A-P0.5
	DDK_IOMUX_GPIO2_18,			// EM9280_GPIO10
	DDK_IOMUX_GPIO2_19,			// EM9280_GPIO11
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO12 -> TCA6424A-P1.0
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO13 -> TCA6424A-P1.1
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO14 -> TCA6424A-P1.2
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO15 -> TCA6424A-P1.3
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO16 -> TCA6424A-P1.4
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO17 -> TCA6424A-P1.5
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO18 -> TCA6424A-P1.6
	DDK_IOMUX_INVALID_PIN,		// EM9280_GPIO19 -> TCA6424A-P1.7
	DDK_IOMUX_GPIO3_28,			// EM9280_GPIO20
	DDK_IOMUX_GPIO3_29,			// EM9280_GPIO21
	DDK_IOMUX_GPIO3_25,			// EM9280_GPIO22
	DDK_IOMUX_GPIO3_24,			// EM9280_GPIO23
	DDK_IOMUX_GPIO2_3,          // EM9280_GPIO24
	DDK_IOMUX_GPIO2_20,			// EM9280_GPIO25
	DDK_IOMUX_GPIO3_30,			// EM9280_GPIO26
	DDK_IOMUX_GPIO4_20,			// EM9280_GPIO27
	DDK_IOMUX_GPIO2_4,          // EM9280_GPIO28
	DDK_IOMUX_GPIO2_6,			// EM9280_GPIO29
	DDK_IOMUX_GPIO2_7,			// EM9280_GPIO30
	DDK_IOMUX_GPIO2_5,			// EM9280_GPIO31
};

//-----------------------------------------------------
// CS&ZHL JULY-6-2012: add EM9283 GPIO Pin Table
//-----------------------------------------------------
DDK_IOMUX_PIN g_EM9283_iMXGpioPin[32] =
{
	DDK_IOMUX_GPIO3_20,			// EM9283_GPIO0
	DDK_IOMUX_GPIO3_21,			// EM9283_GPIO1
	DDK_IOMUX_GPIO1_8,			// EM9283_GPIO2
	DDK_IOMUX_GPIO1_9,			// EM9283_GPIO3
	DDK_IOMUX_GPIO1_16,			// EM9283_GPIO4
	DDK_IOMUX_GPIO1_17,			// EM9283_GPIO5
	DDK_IOMUX_GPIO2_16,			// EM9283_GPIO6
	DDK_IOMUX_GPIO2_17,			// EM9283_GPIO7
	DDK_IOMUX_GPIO2_18,			// EM9283_GPIO8
	DDK_IOMUX_GPIO2_19,			// EM9283_GPIO9
	DDK_IOMUX_GPIO3_24,			// EM9283_GPIO10
	DDK_IOMUX_GPIO3_25,			// EM9283_GPIO11
	DDK_IOMUX_GPIO2_4,          // EM9283_GPIO12
	DDK_IOMUX_GPIO2_6,			// EM9283_GPIO13
	DDK_IOMUX_GPIO2_7,			// EM9283_GPIO14
	DDK_IOMUX_GPIO2_5,			// EM9283_GPIO15
	DDK_IOMUX_GPIO4_7,			// EM9283_GPIO16
	DDK_IOMUX_GPIO4_3,			// EM9283_GPIO17
	DDK_IOMUX_GPIO4_8,			// EM9283_GPIO18
	DDK_IOMUX_GPIO4_4,			// EM9283_GPIO19
	DDK_IOMUX_GPIO2_9,			// EM9283_GPIO20
	DDK_IOMUX_GPIO4_16,			// EM9283_GPIO21
	DDK_IOMUX_GPIO2_8,			// EM9283_GPIO22
	DDK_IOMUX_GPIO2_10,			// EM9283_GPIO23
	DDK_IOMUX_GPIO2_0,          // EM9283_GPIO24
	DDK_IOMUX_GPIO2_1,			// EM9283_GPIO25
	DDK_IOMUX_GPIO2_2,			// EM9283_GPIO26
	DDK_IOMUX_GPIO2_3,			// EM9283_GPIO27
	DDK_IOMUX_GPIO3_23,         // EM9283_GPIO28 : SEP21-2012:DDK_IOMUX_GPIO3_22
	DDK_IOMUX_GPIO3_22,			// EM9283_GPIO29 : SEP21-2012:DDK_IOMUX_GPIO3_23
	DDK_IOMUX_GPIO3_28,			// EM9283_GPIO30
	DDK_IOMUX_GPIO3_26,			// EM9283_GPIO31 : SEP21-2012:DDK_IOMUX_GPIO3_29
};

// Format: 
DWORD	g_EM9280_GPIOXPin[32] = 
{
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO0
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO1
	(PORT0 << PORT_POSITION) | BIT0,			// EM9280_GPIO2 -> TCA6424A-P0.0
	(PORT0 << PORT_POSITION) | BIT1,			// EM9280_GPIO3 -> TCA6424A-P0.1
	(PORT0 << PORT_POSITION) | BIT2,			// EM9280_GPIO4 -> TCA6424A-P0.2
	(PORT0 << PORT_POSITION) | BIT3,			// EM9280_GPIO5 -> TCA6424A-P0.3
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO6
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO7
	(PORT0 << PORT_POSITION) | BIT4,			// EM9280_GPIO8 -> TCA6424A-P0.4
	(PORT0 << PORT_POSITION) | BIT5,			// EM9280_GPIO9 -> TCA6424A-P0.5
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO10
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO11
	(PORT1 << PORT_POSITION) | BIT0,			// EM9280_GPIO12 -> TCA6424A-P1.0
	(PORT1 << PORT_POSITION) | BIT1,			// EM9280_GPIO13 -> TCA6424A-P1.1
	(PORT1 << PORT_POSITION) | BIT2,			// EM9280_GPIO14 -> TCA6424A-P1.2
	(PORT1 << PORT_POSITION) | BIT3,			// EM9280_GPIO15 -> TCA6424A-P1.3
	(PORT1 << PORT_POSITION) | BIT4,			// EM9280_GPIO16 -> TCA6424A-P1.4
	(PORT1 << PORT_POSITION) | BIT5,			// EM9280_GPIO17 -> TCA6424A-P1.5
	(PORT1 << PORT_POSITION) | BIT6,			// EM9280_GPIO18 -> TCA6424A-P1.6
	(PORT1 << PORT_POSITION) | BIT7,			// EM9280_GPIO19 -> TCA6424A-P1.7
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO20
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO21
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO22
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO23
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO24
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO25
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO26
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO27
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO28
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO29
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO30
	(PORT_INVALID << PORT_POSITION),			// EM9280_GPIO31
};

#define	BIT0						(1 << 0)
#define	BIT1						(1 << 1)
#define	BIT2						(1 << 2)
#define	BIT3						(1 << 3)
#define	BIT4						(1 << 4)
#define	BIT5						(1 << 5)
#define	BIT6						(1 << 6)
#define	BIT7						(1 << 7)

#define	PORT0						0
#define	PORT1						1
#define	PORT2						2
#define	PORT_INVALID				0xFF
#define	PORT_POSITION				8

//-----------------------------------------------------
// GPIOClass member function implementation
//-----------------------------------------------------
BOOL GPIOClass::I2CInit()
{
    DDK_GPIO_CFG	intrCfg;

	TCA6424A_INT_PIN = DDK_IOMUX_GPIO1_16;

    DDKIomuxSetPinMux(TCA6424A_INT_PIN, DDK_IOMUX_MODE_GPIO);
	DDKGpioEnableDataPin(TCA6424A_INT_PIN, 0);		// output disable
    DDKIomuxSetPadConfig(TCA6424A_INT_PIN, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);  

	// config GPIO interrupt
	intrCfg.DDK_PIN_IO           = DDK_GPIO_INPUT;
	intrCfg.DDK_PIN_IRQ_CAPABLE  = DDK_GPIO_IRQ_CAPABLE;
	intrCfg.DDK_PIN_IRQ_ENABLE   = DDK_GPIO_IRQ_DISABLED;				// DDK_GPIO_IRQ_ENABLED;
	intrCfg.DDK_PIN_IRQ_LEVEL    = DDK_GPIO_IRQ_EDGE;					// DDK_GPIO_IRQ_LEVEL;
	intrCfg.DDK_PIN_IRQ_POLARITY = DDK_GPIO_IRQ_POLARITY_LO;			// interrupt trigger on falling edge
	if(!DDKGpioConfig(TCA6424A_INT_PIN, intrCfg))
	{
		ERRORMSG(1,(TEXT("GPIOClass: config TCA6424A_INT_PIN failed!\r\n")));
	}
	DDKGpioClearIntrPin(TCA6424A_INT_PIN);

	// init variables for TCA6424A chip
	uTCA6424A_Addr = 0x44;				// according to datasheet, and ADDR-pin lie to GND
	for(int i = 0; i < 3; i++)
	{
		uPortOut[i] = 0xFF;
		uPortDir[i] = 0xFF;				// 1 -> input; 0 -> output
		uPortPinState[i] = 0xFF;
	}

	return TRUE;
}


BOOL GPIOClass::I2CWrite(BYTE ucCmd, PBYTE pDatBuf, DWORD dwDatLen)
{
	I2cAccessInfo	I2cInfo;
	BOOL			bRet;

	I2cInfo.dwAccessCode = I2C_ACCESS_CODE_WRITE;
	I2cInfo.dwAddr = (DWORD)uTCA6424A_Addr;
	I2cInfo.dwCmd = (DWORD)ucCmd;
	I2cInfo.dwDataLength = dwDatLen;
	I2cInfo.pDataBuf = pDatBuf;

	bRet = KernelIoControl(IOCTL_HAL_I2C_ACCESS, 
						(PVOID)&I2cInfo, sizeof(I2cAccessInfo), 
						NULL, 0, 
						NULL);
	if(!bRet)
	{
		RETAILMSG(1, (TEXT("GPIOClass::I2CWrite failed\r\n")));
	}

	return bRet;
}

BOOL GPIOClass::I2CRead(BYTE ucCmd, PBYTE pDatBuf, DWORD dwDatLen)
{
	I2cAccessInfo	I2cInfo;
	BOOL			bRet;

	I2cInfo.dwAccessCode = I2C_ACCESS_CODE_READ;
	I2cInfo.dwAddr = (DWORD)uTCA6424A_Addr;
	I2cInfo.dwCmd = (DWORD)ucCmd;
	I2cInfo.dwDataLength = dwDatLen;
	I2cInfo.pDataBuf = pDatBuf;

	bRet = KernelIoControl(IOCTL_HAL_I2C_ACCESS, 
						(PVOID)&I2cInfo, sizeof(I2cAccessInfo), 
						NULL, 0, 
						NULL);
	if(!bRet)
	{
		RETAILMSG(1, (TEXT("GPIOClass::I2CRead failed\r\n")));
	}

	return bRet;
}

BOOL GPIOClass::I2CSet(BYTE ucCmd, BYTE uMask)
{
	I2cAccessInfo	I2cInfo;
	BOOL			bRet;

	I2cInfo.dwAccessCode = I2C_ACCESS_CODE_SET;
	I2cInfo.dwAddr = (DWORD)uTCA6424A_Addr;
	I2cInfo.dwCmd = (DWORD)ucCmd;
	I2cInfo.dwDataLength = 1;
	I2cInfo.pDataBuf = &uMask;

	bRet = KernelIoControl(IOCTL_HAL_I2C_ACCESS, 
						(PVOID)&I2cInfo, sizeof(I2cAccessInfo), 
						NULL, 0, 
						NULL);
	if(!bRet)
	{
		RETAILMSG(1, (TEXT("GPIOClass::I2CSet failed\r\n")));
	}

	return bRet;
}

BOOL GPIOClass::I2CClear(BYTE ucCmd, BYTE uMask)
{
	I2cAccessInfo	I2cInfo;
	BOOL			bRet;

	I2cInfo.dwAccessCode = I2C_ACCESS_CODE_CLEAR;
	I2cInfo.dwAddr = (DWORD)uTCA6424A_Addr;
	I2cInfo.dwCmd = (DWORD)ucCmd;
	I2cInfo.dwDataLength = 1;
	I2cInfo.pDataBuf = &uMask;

	bRet = KernelIoControl(IOCTL_HAL_I2C_ACCESS, 
						(PVOID)&I2cInfo, sizeof(I2cAccessInfo), 
						NULL, 0, 
						NULL);
	if(!bRet)
	{
		RETAILMSG(1, (TEXT("GPIOClass::I2CClear failed\r\n")));
	}

	return bRet;
}


//-----------------------------------------------------
// GPIOClass public member functions
//-----------------------------------------------------
GPIOClass::GPIOClass( )
{
	DWORD			i;
    DDK_GPIO_CFG	intrCfg;
	BYTE			uCmd;

#ifdef	EM9280
	pGpioPinTab = g_EM9280_iMXGpioPin;
	pTCA6424APinTab = g_EM9280_GPIOXPin;
#else	// -> EM9280
	pGpioPinTab = g_EM9283_iMXGpioPin;
	pTCA6424APinTab = NULL;
#endif	//EM9280
	dwPinTabLen = 32;						//sizeof(g_EM9280_iMXGpioPin) / sizeof(DWORD);

	//init i2c bus for GPIOX
	if(pTCA6424APinTab != NULL)
	{
		I2CInit();
	}

	// setup all iMX28 pins into GPIO with input mode
	for(i = 0; i < dwPinTabLen; i++)
	{
		if(pGpioPinTab[i] == DDK_IOMUX_INVALID_PIN)
		{
			continue;
		}
		else
		{
			//RETAILMSG(1, (TEXT("GPIOClass: config EM9280_GPIO%d -> iMX28 GPIO%d_%d as input\r\n"), 
			//			i, (pGpioPinTab[i] / 32), (pGpioPinTab[i] % 32)));

			//switch to GPIO mode -> config as input
			DDKIomuxSetPinMux(pGpioPinTab[i], DDK_IOMUX_MODE_GPIO);
			DDKGpioEnableDataPin(pGpioPinTab[i], 0);					//output disable
			DDKIomuxSetPadConfig(pGpioPinTab[i], 
							DDK_IOMUX_PAD_DRIVE_8MA, 
							DDK_IOMUX_PAD_PULL_ENABLE,
							DDK_IOMUX_PAD_VOLTAGE_3V3);
		}
	}

	// CS&ZHL JULY-6-2012: no expanded GPIO in EM9283
	if(pTCA6424APinTab == NULL)
	{
		hIRQEvent = NULL;
		dwSysIntr = (DWORD)SYSINTR_UNDEFINED;
		bIrqGpioPinEnabled = FALSE;
		RETAILMSG(1,(TEXT("GPIOClass: EM9283 GPIO init done\r\n")));
		return;
	}

	// set all GPIOX pins in input state
	uCmd = TCA6424A_CMD_AUTO_INC | TCA6424A_CMD_DIR | TCA6424A_CMD_ADDR_PORT0;
	if(!I2CWrite(uCmd, uPortDir, 3))
	{
		RETAILMSG(1,(TEXT("GPIOClass: config TCA6424A as input failed\r\n")));
		return;
	}

	uCmd = TCA6424A_CMD_AUTO_INC | TCA6424A_CMD_OUTPUT | TCA6424A_CMD_ADDR_PORT0;
	if(!I2CWrite(uCmd, uPortOut, 3))
	{
		RETAILMSG(1,(TEXT("GPIOClass: write TCA6424A output registers failed\r\n")));
		return;
	}

	// setup Gpio interrupt pin
	bIrqGpioPinEnabled = FALSE;
	dwSysIntr = (DWORD)SYSINTR_UNDEFINED;
	dwDeviceID = IRQ_GPIO1_PIN16;				// @field logical interrupt number
	dwIrqGpioPin = DDK_IOMUX_GPIO1_16;			// @filed enum { DDK_IOMUX_GPIO1_16 } 	

	// request system intr number
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, 
						(PVOID)&dwDeviceID, 
						sizeof(dwDeviceID), 
						(PVOID)&dwSysIntr, 
						sizeof(dwSysIntr), NULL))
	{
		ERRORMSG(1, (TEXT("ERROR: Failed to request sysintr.\r\n")));
		dwSysIntr = (DWORD)SYSINTR_UNDEFINED;
		return;
	}

	// create IRQ event
	hIRQEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(hIRQEvent == NULL)
	{
		ERRORMSG(1, (TEXT("ERROR: Failed to create IRQ event.\r\n")));
		return;
	}

	// bind SYSINTR to event
   	if(!InterruptInitialize(dwSysIntr, hIRQEvent, NULL, 0))
	{
		ERRORMSG(1, (TEXT("ERROR: Failed to bind Irq number and Irq event.\r\n")));
		return;
	}

	//CS&ZHL JUN-18-07: follow 'com_mmd2\mdd.c' to do this
    InterruptDone(dwSysIntr);

	//switch IrqGpioPin to GPIO mode -> config as input
	DDKIomuxSetPinMux((DDK_IOMUX_PIN)dwIrqGpioPin, DDK_IOMUX_MODE_GPIO);
	DDKGpioEnableDataPin((DDK_IOMUX_PIN)dwIrqGpioPin, 0);					//output disable
	DDKIomuxSetPadConfig((DDK_IOMUX_PIN)dwIrqGpioPin, DDK_IOMUX_PAD_DRIVE_8MA, 
						   DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

	intrCfg.DDK_PIN_IO           = DDK_GPIO_INPUT;
    intrCfg.DDK_PIN_IRQ_CAPABLE  = DDK_GPIO_IRQ_CAPABLE;
    intrCfg.DDK_PIN_IRQ_ENABLE   = DDK_GPIO_IRQ_DISABLED;
    intrCfg.DDK_PIN_IRQ_LEVEL    = DDK_GPIO_IRQ_EDGE;
    intrCfg.DDK_PIN_IRQ_POLARITY = DDK_GPIO_IRQ_POLARITY_LO;		// interrupt trigger on falling edge
	if(!DDKGpioConfig((DDK_IOMUX_PIN)dwIrqGpioPin, intrCfg))
	{
		RETAILMSG(1,(TEXT("GPIOClass: config %d# pin failed\r\n"), (DDK_IOMUX_PIN)dwIrqGpioPin));
		return;
	}
	DDKGpioClearIntrPin((DDK_IOMUX_PIN)dwIrqGpioPin);
}

GPIOClass::~GPIOClass(void)
{
	// CS&ZHL JULY-6-2012: only for EM9280
	if(pTCA6424APinTab != NULL)
	{
		if(hIRQEvent != NULL)
			CloseHandle(hIRQEvent);

		// release system intr number
		if(dwSysIntr != SYSINTR_UNDEFINED)
		{
			KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, 
							&dwSysIntr, sizeof(dwSysIntr), 
							NULL, 0, 0);
		}

		// clear intr flag
		DDKGpioClearIntrPin((DDK_IOMUX_PIN)dwIrqGpioPin);

		// disable interrupt
		DDKGpioIntrruptDisable((DDK_IOMUX_PIN)dwIrqGpioPin);
	}
}

//
// GPIO functions
//
BOOL GPIOClass::PIO_OutEnable( UINT32 dwGpioBits )
{
	DWORD	i;
	BYTE	uCmd;
	BYTE	uPort;
	BYTE	uMask;

	for(i = 0; i < dwPinTabLen; i++)
	{
		if( !(dwGpioBits & (1 << i)) )
		{
			continue;
		}

		if(pGpioPinTab[i] == DDK_IOMUX_INVALID_PIN)		// pin from TCA6224A
		{
			uPort = (BYTE)(pTCA6424APinTab[i] >> PORT_POSITION);
			if(uPort == PORT_INVALID)
			{
				continue;
			}
			// set this pin as input
			uMask = (BYTE)(pTCA6424APinTab[i] & 0xFF);
			uCmd = TCA6424A_CMD_DIR | uPort;			// select DIR register
			//if(!I2CClear(uCmd, uMask))					
			//{
			//	return FALSE;
			//}
			uPortDir[uPort] &= ~uMask;					// = 1: input, = 0: output;
			if(!I2CWrite(uCmd, &uPortDir[uPort], 1))
			{
				return FALSE;
			}
		}
		else											// pin from iMX28
		{
			DDKGpioEnableDataPin(pGpioPinTab[i], 1);
		}
	}

	return TRUE;
}

BOOL GPIOClass::PIO_OutDisable( UINT32 dwGpioBits )
{
	DWORD	i;
	BYTE	uCmd;
	BYTE	uPort;
	BYTE	uMask;

	for(i = 0; i < dwPinTabLen; i++)
	{
		if( !(dwGpioBits & (1 << i)) )
		{
			continue;
		}

		if(pGpioPinTab[i] == DDK_IOMUX_INVALID_PIN)		// pin from TCA6224A
		{
			uPort = (BYTE)(pTCA6424APinTab[i] >> PORT_POSITION);
			if(uPort == PORT_INVALID)
			{
				continue;
			}
			// set this pin as input
			uMask = (BYTE)(pTCA6424APinTab[i] & 0xFF);
			uCmd = TCA6424A_CMD_DIR | uPort;			// select DIR register
			//if(!I2CSet(uCmd, uMask))					// = 1: input, = 0: output;
			//{
			//	return FALSE;
			//}
			uPortDir[uPort] |= uMask;					// = 1: input, = 0: output;
			if(!I2CWrite(uCmd, &uPortDir[uPort], 1))
			{
				return FALSE;
			}
		}
		else											// pin from iMX28
		{
			DDKGpioEnableDataPin(pGpioPinTab[i], 0);
		}
	}

	return TRUE;
}

BOOL GPIOClass::PIO_OutSet( UINT32 dwGpioBits )
{
	DWORD	i;
	BYTE	uCmd;
	BYTE	uPort;
	BYTE	uMask;

	switch(dwGpioBits)
	{
	case GPIO0:		// case of single bit
	case GPIO1:
	case GPIO6:
	case GPIO7:
	case GPIO10:
	case GPIO11:
	case GPIO20:
	case GPIO21:
	case GPIO22:
	case GPIO23:
	case GPIO24:
	case GPIO25:
	case GPIO26:
	case GPIO27:
	case GPIO28:
	case GPIO29:
	case GPIO30:
	case GPIO31:
#ifdef	EM9283
	case GPIO2:
	case GPIO3:
	case GPIO4:
	case GPIO5:
	case GPIO8:
	case GPIO9:
	case GPIO12:
	case GPIO13:
	case GPIO14:
	case GPIO15:
	case GPIO16:
	case GPIO17:
	case GPIO18:
	case GPIO19:
#endif	//EM9283
		//returns the number of contiguous zero bits starting with the most significant bit in the argument.
		i = _CountLeadingZeros(dwGpioBits);				
		DDKGpioWriteDataPin(pGpioPinTab[31 - i], 1);
		break;

	default:	// case of multiple bits
		for(i = 0; i < dwPinTabLen; i++)
		{
			if( !(dwGpioBits & (1 << i)) )
			{
				continue;
			}

			if(pGpioPinTab[i] != DDK_IOMUX_INVALID_PIN)		// pin from iMX283
			{
				DDKGpioWriteDataPin(pGpioPinTab[i], 1);
				continue;
			}

			// pin from TCA6224A
			uPort = (BYTE)(pTCA6424APinTab[i] >> PORT_POSITION);
			if(uPort == PORT_INVALID)
			{
				continue;
			}
			// set this pin as input
			uMask = (BYTE)(pTCA6424APinTab[i] & 0xFF);
			uCmd = TCA6424A_CMD_OUTPUT | uPort;			// select data ouput register
			//if(!I2CSet(uCmd, uMask))					// = 1: high, = 0: low;
			//{
			//	return FALSE;
			//}
			uPortOut[uPort] |= uMask;					// = 1: high, = 0: low;
			if(!I2CWrite(uCmd, &uPortOut[uPort], 1))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL GPIOClass::PIO_OutClear( UINT32 dwGpioBits )
{
	DWORD	i;
	BYTE	uCmd;
	BYTE	uPort;
	BYTE	uMask;

	switch(dwGpioBits)
	{
	case GPIO0:		// case of single bit
	case GPIO1:
	case GPIO6:
	case GPIO7:
	case GPIO10:
	case GPIO11:
	case GPIO20:
	case GPIO21:
	case GPIO22:
	case GPIO23:
	case GPIO24:
	case GPIO25:
	case GPIO26:
	case GPIO27:
	case GPIO28:
	case GPIO29:
	case GPIO30:
	case GPIO31:
#ifdef	EM9283
	case GPIO2:
	case GPIO3:
	case GPIO4:
	case GPIO5:
	case GPIO8:
	case GPIO9:
	case GPIO12:
	case GPIO13:
	case GPIO14:
	case GPIO15:
	case GPIO16:
	case GPIO17:
	case GPIO18:
	case GPIO19:
#endif	//EM9283
		//returns the number of contiguous zero bits starting with the most significant bit in the argument.
		i = _CountLeadingZeros(dwGpioBits);				
		DDKGpioWriteDataPin(pGpioPinTab[31 - i], 0);
		break;

	default:	// case of multiple bits
		for(i = 0; i < dwPinTabLen; i++)
		{
			if( !(dwGpioBits & (1 << i)) )
			{
				continue;
			}

			if(pGpioPinTab[i] != DDK_IOMUX_INVALID_PIN)		// pin from iMX283
			{
				DDKGpioWriteDataPin(pGpioPinTab[i], 0);
				continue;
			}

			// pin from TCA6224A
			uPort = (BYTE)(pTCA6424APinTab[i] >> PORT_POSITION);
			if(uPort == PORT_INVALID)
			{
				continue;
			}
			// set this pin as input
			uMask = (BYTE)(pTCA6424APinTab[i] & 0xFF);
			uCmd = TCA6424A_CMD_OUTPUT | uPort;			// select data ouput register
			//if(!I2CClear(uCmd, uMask))					// = 1: high, = 0: low;
			//{
			//	return FALSE;
			//}
			uPortOut[uPort] &= ~uMask;					// = 1: high, = 0: low;
			if(!I2CWrite(uCmd, &uPortOut[uPort], 1))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL GPIOClass::PIO_State( UINT32* pStateBits )
{
	DWORD	i;
	BYTE	uCmd;
	UINT32	dwBitsToRead;
	UINT32	uData = 0;
	UINT32	dwMX28GpioBits = 0;
	BOOL	bGPIOXReadRequired;

	dwBitsToRead = *pStateBits;

	switch(dwBitsToRead)
	{
	case GPIO0:		// case of single bit
	case GPIO1:
	case GPIO6:
	case GPIO7:
	case GPIO10:
	case GPIO11:
	case GPIO20:
	case GPIO21:
	case GPIO22:
	case GPIO23:
	case GPIO24:
	case GPIO25:
	case GPIO26:
	case GPIO27:
	case GPIO28:
	case GPIO29:
	case GPIO30:
	case GPIO31:
#ifdef	EM9283
	case GPIO2:
	case GPIO3:
	case GPIO4:
	case GPIO5:
	case GPIO8:
	case GPIO9:
	case GPIO12:
	case GPIO13:
	case GPIO14:
	case GPIO15:
	case GPIO16:
	case GPIO17:
	case GPIO18:
	case GPIO19:
#endif	//EM9283
		//returns the number of contiguous zero bits starting with the most significant bit in the argument.
		i = _CountLeadingZeros(dwBitsToRead);
		// convert to position of "1"
		i = 31 - i;
		// read just one pin from iMX28
		DDKGpioReadDataPin(pGpioPinTab[i], &uData);
		if( uData )
		{
			dwMX28GpioBits |= (1 << i);
		}
		break;

	default:	// case of multiple bits
		bGPIOXReadRequired = FALSE;
		// all 0 => all 1
		if(dwBitsToRead == 0)
		{
			dwBitsToRead = 0xFFFFFFFF;
		}

		// read pins from iMX28
		for(i = 0; i < dwPinTabLen; i++)
		{
			// skip reading if not required
			if(!(dwBitsToRead & (1 << i)))
			{
				continue;
			}

			// set GPIOX read flag
			if(pGpioPinTab[i] == DDK_IOMUX_INVALID_PIN)		// pin from TCA6224A
			{
				bGPIOXReadRequired = TRUE;
				continue;
			}

			// read GPIO state from iMX28
			uData = 0;
			DDKGpioReadDataPin(pGpioPinTab[i], &uData);
			if( uData )
			{
				dwMX28GpioBits |= (1 << i);
			}
		}

		if(bGPIOXReadRequired)
		{
			// read pins from TCA6424A
			uCmd = TCA6424A_CMD_AUTO_INC | TCA6424A_CMD_INPUT | TCA6424A_CMD_ADDR_PORT0;
			I2CRead(uCmd, uPortPinState, 2);

			// emerge together
			dwMX28GpioBits |= (((UINT32)uPortPinState[0] & 0x0F) << 2);		// emerge GPIO2 - GPIO5
			dwMX28GpioBits |= (((UINT32)uPortPinState[0] & 0x30) << 4);		// emerge GPIO8 - GPIO9
			dwMX28GpioBits |= (((UINT32)uPortPinState[1] & 0xFF) << 12);	// emerge GPIO12 - GPIO19
		}
	}

	*pStateBits = dwMX28GpioBits;
	return TRUE;
}

DWORD GPIOClass::WaitGpioInterrupt(DWORD dwTimeout)
{
	DWORD	dwWaitReturn;

	if(!bIrqGpioPinEnabled)
	{
		// enable GPIO interrupt
		DDKGpioIntrruptEnable((DDK_IOMUX_PIN)dwIrqGpioPin);
		bIrqGpioPinEnabled = TRUE;
	}

	// return = WAIT_OBJECT_0, WAIT_TIMEOUT and WAIT_FAILED
	dwWaitReturn = WaitForSingleObject( hIRQEvent, dwTimeout );

	// if get a true interrupt, clear interrupt flag
	if(dwWaitReturn == WAIT_OBJECT_0)
	{
		// clear GPIO interrupt status 
		DDKGpioClearIntrPin((DDK_IOMUX_PIN)dwIrqGpioPin);
		InterruptDone(dwSysIntr);
	}

	return dwWaitReturn;
}

/*
// the handler to read us counter
PVOID pv_HWregDIGCTL = NULL;

	//init the handler of microsecond delay
	if(pv_HWregDIGCTL == NULL)
	{
		PHYSICAL_ADDRESS phyAddr;

        phyAddr.QuadPart = CSP_BASE_REG_PA_DIGCTL;
        pv_HWregDIGCTL = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregDIGCTL == NULL)
        {
            RETAILMSG(1, (TEXT("GPIOClass::MmMapIoSpace failed for pv_HWregDIGCTL\r\n")));
        }
        //enable the digital Control Microseconds counter
        //HW_DIGCTL_CTRL.B.XTAL24M_GATE = 0;
		HW_DIGCTL_CTRL_CLR(BM_DIGCTL_CTRL_XTAL24M_GATE);
	}



//-----------------------------------------------------
// GPIOClass other public functions
//-----------------------------------------------------
void GPIOClass::udelay(DWORD dwMicroSecond)
{
	DWORD	dwStartUS;
	DWORD	dwCurrentUS;
	DWORD	dwElapsedUS = 0;

	if(pv_HWregDIGCTL == NULL)
	{
		return;
	}

	dwStartUS = HW_DIGCTL_MICROSECONDS_RD();
	while(dwElapsedUS < dwMicroSecond)
	{
		dwCurrentUS = HW_DIGCTL_MICROSECONDS_RD();
		if(dwCurrentUS >= dwStartUS)
		{
			// normal counting
			dwElapsedUS = dwCurrentUS - dwStartUS;
		}
		else
		{
			// count wrap happened
			dwElapsedUS = ~dwStartUS + dwCurrentUS + 1;
		}
	}
}

	//de-init the handler of microsecond delay
	if(pv_HWregDIGCTL != NULL)
	{
		MmUnmapIoSpace((PVOID)pv_HWregDIGCTL, 0x1000);
	}
*/