#include "bsp.h"
#include "gpioclass.h"
#include "tca6424a_i2c.h"

//-----------------------------------------------------
// EM9280 V1.0 GPIO Pin Table
//-----------------------------------------------------
DDK_IOMUX_PIN g_dwEM9280_GpioPin[ ] =
{
	DDK_IOMUX_GPIO3_5,			// EM9280_GPIO0
	DDK_IOMUX_GPIO3_4,			// EM9280_GPIO1
	DDK_IOMUX_GPIO2_17,			// EM9280_GPIO2
	DDK_IOMUX_GPIO2_16,			// EM9280_GPIO3
	DDK_IOMUX_GPIO2_19,			// EM9280_GPIO4
	DDK_IOMUX_GPIO2_18,			// EM9280_GPIO5
	DDK_IOMUX_GPIO3_24,			// EM9280_GPIO6
	DDK_IOMUX_GPIO3_25,			// EM9280_GPIO7
	DDK_IOMUX_GPIO3_30,			// EM9280_GPIO8
	DDK_IOMUX_GPIO4_20,			// EM9280_GPIO9
	DDK_IOMUX_GPIO2_20,			// EM9280_GPIO10
	DDK_IOMUX_GPIO2_3,          // EM9280_GPIO11
	DDK_IOMUX_GPIO3_16,			// EM9280_GPIO12
	DDK_IOMUX_GPIO3_17,			// EM9280_GPIO13
	DDK_IOMUX_GPIO3_28,			// EM9280_GPIO14
	DDK_IOMUX_GPIO3_29,			// EM9280_GPIO15
	DDK_IOMUX_GPIO2_4,          // EM9280_GPIO16
	DDK_IOMUX_GPIO2_6,			// EM9280_GPIO17
	DDK_IOMUX_GPIO2_7,			// EM9280_GPIO18
	DDK_IOMUX_GPIO2_5,			// EM9280_GPIO19
};

// the handler to read us counter
PVOID pv_HWregDIGCTL = NULL;

//-----------------------------------------------------
// GPIOClass member function implementation
//-----------------------------------------------------
BOOL GPIOClass::GpioI2CInit()
{
    DDK_GPIO_CFG	intrCfg;

    intrCfg.DDK_PIN_IO           = DDK_GPIO_INPUT;
    intrCfg.DDK_PIN_IRQ_CAPABLE  = DDK_GPIO_IRQ_CAPABLE;
    intrCfg.DDK_PIN_IRQ_ENABLE   = DDK_GPIO_IRQ_DISABLED;
    intrCfg.DDK_PIN_IRQ_LEVEL    = DDK_GPIO_IRQ_EDGE;
    intrCfg.DDK_PIN_IRQ_POLARITY = DDK_GPIO_IRQ_POLARITY_LO;		// interrupt trigger on falling edge

	I2C_SCL_PIN = DDK_IOMUX_GPIO1_8;
	I2C_SDA_PIN = DDK_IOMUX_GPIO1_9;
	TCA6424A_INT_PIN = DDK_IOMUX_GPIO1_16;

	//config GPIOs for I2C
	if(!DDKGpioConfig(I2C_SCL_PIN, intrCfg))
	{
		RETAILMSG(1,(TEXT("GPIOClass: config I2C_SCL_PIN failed!\r\n")));
	}
	//switch to GPIO mode
    DDKIomuxSetPinMux(I2C_SCL_PIN, DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPadConfig(I2C_SCL_PIN, DDK_IOMUX_PAD_DRIVE_4MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

	if(!DDKGpioConfig(I2C_SDA_PIN, intrCfg))
	{
		RETAILMSG(1,(TEXT("GPIOClass: config I2C_SDA_PIN failed!\r\n")));
	}
	//switch to GPIO mode
    DDKIomuxSetPinMux(I2C_SDA_PIN, DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPadConfig(I2C_SDA_PIN, DDK_IOMUX_PAD_DRIVE_4MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
	
	if(!DDKGpioConfig(TCA6424A_INT_PIN, intrCfg))
	{
		RETAILMSG(1,(TEXT("GPIOClass: config TCA6424A_INT_PIN failed!\r\n")));
	}
	//switch to GPIO mode
    DDKIomuxSetPinMux(TCA6424A_INT_PIN, DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPadConfig(TCA6424A_INT_PIN, DDK_IOMUX_PAD_DRIVE_4MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

	// finally SCL output enable
	DDKGpioEnableDataPin(I2C_SCL_PIN, 1);

	// init variables for TCA6424A chip
	uTCA6424A_Addr = 0x44;				// according to datasheet, and ADDR-pin lie to GND
	dwGPIOX_DOUT = 0xFFFFFFFF;			// on power-up, all bits are HIGH, valid with BIT0 - BIT23 
	dwGPIOX_INV  = 0;					// on power-up, all bits are LOW, valid with BIT0 - BIT23 
	dwGPIOX_DIR  = 0xFFFFFFFF;			// on power-up, all bits are HIGH, valid with BIT0 - BIT23 

	return TRUE;
}

__inline void GPIOClass::SCL_SET(void)
{
	DDKGpioWriteDataPin(I2C_SCL_PIN, 1);
}

__inline void GPIOClass::SCL_CLR(void)
{
	DDKGpioWriteDataPin(I2C_SCL_PIN, 0);
}

__inline void GPIOClass::SDA_SET(void)
{
	DDKGpioWriteDataPin(I2C_SDA_PIN, 1);
}

__inline void GPIOClass::SDA_CLR(void)
{
	DDKGpioWriteDataPin(I2C_SDA_PIN, 0);
}

__inline void GPIOClass::SDA_OUTEN(void)
{
	DDKGpioEnableDataPin(I2C_SDA_PIN, 1);
}

__inline void GPIOClass::SDA_OUTDIS(void)
{
	DDKGpioEnableDataPin(I2C_SDA_PIN, 0);
}

__inline UINT32 GPIOClass::SDA_STATE(void)
{
	UINT32	Data;

	DDKGpioReadDataPin(I2C_SDA_PIN, &Data);
	return Data;
}

BOOL GPIOClass::TCA6424A_GpioI2CWrite(BYTE ucCmd, PBYTE pDatBuf, DWORD dwDatLen)
{
	DWORD  	i1, i2;
	DWORD	count = 0;	
	BYTE 	ub[32];

	//fill data
	ub[count] = uTCA6424A_Addr & 0xfe;		// 1st byte = ID + Write-Bit(=0)
	count++;
	ub[count] = ucCmd;						// 2nd byte = command
	count++;
	
	for(i1 = 0; i1 < dwDatLen; i1++, count++)
	{
		ub[count] = pDatBuf[i1];			// data write to the register
	}

	// START condition
	SDA_OUTEN();					//set SDA as output
	SDA_SET();						//set SDA HIGH
	SCL_SET();						//set SCL HIGH
	udelay(4);						//can be more than 63us for a new START condition
	SDA_CLR();						//set SDA LOW
	udelay(1);						//I2C timing: start hold time > 0.6us
	SCL_CLR();						//set SCK LOW
	udelay(2);						//I2C timing: SCK low time  > 1.3us

	for(i1 = 0; i1 < count; i1++)
	{
		for(i2 = 0; i2 < 8; i2++)
		{
			// setup data onto SDA
			if(ub[i1] & 0x80)
			{
				SDA_SET();				//set SDA HIGH
			}
			else
			{
				SDA_CLR();				//Clear SDA to LOW
			}
			udelay(1);					//I2C timing: data setup time > 200ns

			// issue a clock
			SCL_SET();					//set SCL HIGH
			udelay(1);					//I2C timing: SCK high time > 0.6us
			SCL_CLR();					//set SCK LOW
			udelay(2);					//I2C timing: SCK low time > 1.3us
			ub[i1] = ub[i1] << 1;
		}

		//check ACK(Low state) from I2C device 
		SDA_OUTDIS();					//set SDA as input
		udelay(1);						//I2C timing: data (from I2C device) setup time > 200ns
		SCL_SET();						//issue a clock
		udelay(1);						//I2C timing: SCK high time > 0.6us
		if(SDA_STATE())					//read ACK_BIT issued from I2C device
		{
			// NO ACK, so setup STOP condition
			SCL_SET();					//SCL = "1"
			udelay(1);					//I2C timing: stop setup timing > 0.6us
			SDA_SET();					//SDA is in input status!

			RETAILMSG(1,(TEXT("TCA6424A_GpioI2CWrite: no ACK, write aborted\r\n")));
			return FALSE;				// no acknowledgement -> failed
		}
		SCL_CLR();						//SCL = "0";
		udelay(2);						//I2C timing: SCK low time > 1.3us
		//check ACK passed, config SDA as output for further outputing
		SDA_OUTEN();					//set SDA as output
	}

	// Here we issue a STOP condition
	SDA_CLR();							//Set SDA = 0;
	SCL_SET();							//Set SCL = 1;
	udelay(1);							//I2C timing: stop setup time > 0.6us
	SDA_SET();							//Set SDA = 1
	SDA_OUTDIS();						//set SDA as input

	return TRUE;
}

BOOL GPIOClass::TCA6424A_GpioI2CRead(BYTE ucCmd, PBYTE pDatBuf, DWORD dwDatLen)
{
	DWORD  	i1, i2;
	DWORD	count = 0;	
	BYTE 	ub[32];

	// Session 1: setup register pointer
	ub[count] = uTCA6424A_Addr & 0xfe;		// 1st byte = ID + Write-Bit(=0)
	count++;
	ub[count] = ucCmd;						// 2nd byte = command
	count++;

	// START condition
	SDA_OUTEN();					//set SDA as output
	SDA_SET();						//set SDA HIGH
	SCL_SET();						//set SCL HIGH
	udelay(4);						//can be more than 63us for a new START condition
	SDA_CLR();						//set SDA LOW
	udelay(1);						//I2C timing: start hold time > 0.6us
	SCL_CLR();						//set SCK LOW
	udelay(2);						//I2C timing: SCK low time  > 1.3us

	for(i1 = 0; i1 < count; i1++)
	{
		for(i2 = 0; i2 < 8; i2++)
		{
			// setup data onto SDA
			if(ub[i1] & 0x80)
			{
				SDA_SET();				//set SDA HIGH
			}
			else
			{
				SDA_CLR();				//Clear SDA to LOW
			}
			udelay(1);					//I2C timing: data setup time > 200ns

			// issue a clock
			SCL_SET();					//set SCL HIGH
			udelay(1);					//I2C timing: SCK high time > 0.6us
			SCL_CLR();					//set SCK LOW
			udelay(2);					//I2C timing: SCK low time > 1.3us
			ub[i1] = ub[i1] << 1;
		}

		//check ACK(Low state) from I2C device 
		SDA_OUTDIS();					//set SDA as input
		udelay(1);						//I2C timing: data (from I2C device) setup time > 200ns
		SCL_SET();						//issue a clock
		udelay(1);						//I2C timing: SCK high time > 0.6us
		if(SDA_STATE())					//read ACK_BIT issued from I2C device
		{
			// NO ACK, so setup STOP condition
			SCL_SET();					//SCL = "1"
			udelay(1);					//I2C timing: stop setup timing > 0.6us
			SDA_SET();					//SDA is in input status!

			RETAILMSG(1,(TEXT("TCA6424A_GpioI2CRead: no ACK, write aborted\r\n")));
			return FALSE;				// no acknowledgement -> failed
		}
		SCL_CLR();						//SCL = "0";
		udelay(2);						//I2C timing: SCK low time > 1.3us
		//check ACK passed, config SDA as output for further outputing
		SDA_OUTEN();					//set SDA as output
	}

	// issue Repeated START condition
	SDA_OUTEN();						//set SDA as output
	SDA_SET();							//set SDA HIGH
	SCL_SET();							//set SCK HIGH
	udelay(1);							//I2C timing: SCK high time > 0.6us
	SDA_CLR();							//set SDA LOW
	udelay(1);							//I2C timing: start hold time > 0.6us
	SCL_CLR();							//set SCK LOW
	udelay(2);							//I2C timing: SCK low time  > 1.3us

	// Session 2: read data  from the device
	// Session 2-part 1: write read cmd to the device
	ub[0] = uTCA6424A_Addr | 0x01;		// 1st byte = ID + Read-Bit(=1)
	for(i2 = 0; i2 < 8; i2++)
	{
		// setup data onto SDA
		if(ub[0] & 0x80)
		{
			SDA_SET();					//set SDA = 1
		}
		else
		{
			SDA_CLR();					//clear SDA = 0
		}
		udelay(1);						//I2C timing: data setup time > 200ns
		
		// issue a clock
		SCL_SET();						//Set SCL = 1;
		udelay(1);						//I2C timing: SCK high time > 0.6us
		SCL_CLR();						//clear SCL = 0
		udelay(2);						//I2C timing: SCK low time > 1.3us
		ub[0] = ub[0] << 1;
	}

	//check ACK(Low state) from I2C device 
	SDA_OUTDIS();					//set SDA as input
	udelay(1);						//I2C timing: data(from I2C device) setup time > 200ns
	SCL_SET();						//issue a clock
	udelay(1);						//I2C timing: SCK high time > 0.6us
	if(SDA_STATE()) 				//read ACK_BIT issued from I2C device
	{
		// NO ACK, so setup STOP condition
		SCL_SET();					//Set SCK=1
		udelay(1);					//I2C timing: stop setup timing > 0.6us
		SDA_SET();					//SDA is in input status!
		RETAILMSG(1,(TEXT("TCA6424A_GpioI2CRead: no ACK, read aborted\r\n")));
		return FALSE; 				//no acknowledgement -> failed
	}
	SCL_CLR();						//clear SCK=0;
	udelay(2);						//I2C timing: SCK low time > 1.3us
	//check ACK passed, config SDA as output for further outputing
	SDA_OUTEN();					//set SDA as output

	// Session 2-part 2: read data from device to pBuffer, and issue ACK properly
	for(i1 = 0; i1 < dwDatLen; i1++)
	{
		ub[0] = 0;
		SDA_OUTDIS();				//set SDA as input
		// then read a abyte
		for(i2 = 0; i2 < 8; i2++)
		{
			ub[0] = ub[0] << 1;
			// issue a clock
			SCL_SET();				//Set SCK=1
			udelay(1);				//I2C timing: SCK high time > 0.6us
			if(SDA_STATE())			// read SDA	
			{
				ub[0] = ub[0] | 0x01;
			}
			SCL_CLR();				//Set SCK=0
			udelay(2);				//I2C timing: SCK low time > 1.3us
		}
		pDatBuf[i1] = ub[0];		//save the data into buffer
		
		//issue ACK condition properly
		SDA_OUTEN();
		if(i1 == (dwDatLen - 1))
		{
			//This is the last byte, issue NO-ACK condition
			SDA_SET();				//SDA = "1": NO-ACK
		}
		else
		{
			//There are more byte need to read, so issue ACK condition
			SDA_CLR();				//SDA = "0": ACK (active low)
		}
		udelay(1);					//I2C timing: data setup time > 200ns
		SCL_SET();					//Set SCL = 1
		udelay(1);					//I2C timing: SCK high time > 0.6us
		SCL_CLR();
		udelay(2);					//I2C timing: SCK low time > 1.3us
	}

	// Here we issue a STOP condition
	SDA_CLR();						//Set SDA=0
	SCL_SET();						//Set SCK=1
	udelay(1);						//I2C timing: stop setup time > 0.6us
	SDA_SET();						//Set SDA=1
	SDA_OUTDIS();					// set SDA as input

	return TRUE;
}



//-----------------------------------------------------
// GPIOClass public member functions
//-----------------------------------------------------
GPIOClass::GPIOClass( UINT32 index )
{
	DWORD			i;
    DDK_GPIO_CFG	intrCfg;

	if( index==1 )    // for EM9170
	{
		pGpioPinTab = g_dwEM9280_GpioPin;
		dwGpioPinTabLen = sizeof(g_dwEM9280_GpioPin) / sizeof(DWORD);
	}
	else
	{
		pGpioPinTab = NULL;
		dwGpioPinTabLen = 0;
	}

	InitializeCriticalSection(&csGpioLock);	

    intrCfg.DDK_PIN_IO           = DDK_GPIO_INPUT;
    intrCfg.DDK_PIN_IRQ_CAPABLE  = DDK_GPIO_IRQ_CAPABLE;
    intrCfg.DDK_PIN_IRQ_ENABLE   = DDK_GPIO_IRQ_DISABLED;
    intrCfg.DDK_PIN_IRQ_LEVEL    = DDK_GPIO_IRQ_EDGE;
    intrCfg.DDK_PIN_IRQ_POLARITY = DDK_GPIO_IRQ_POLARITY_HI;		// interrupt trigger on rising edge

	// setup all pins into GPIO with input mode
	for(i = 0; i < dwGpioPinTabLen; i++)
	{
		//config GPIO
		if(!DDKGpioConfig(pGpioPinTab[i], intrCfg))
		{
			RETAILMSG(1,(TEXT("GPIOClass: Failed to config %d# pin!!!\r\n"), pGpioPinTab[i]));
			continue;
		}
		//switch to GPIO mode
        DDKIomuxSetPinMux(pGpioPinTab[i], DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPadConfig(pGpioPinTab[i], 
                        DDK_IOMUX_PAD_DRIVE_8MA, 
                        DDK_IOMUX_PAD_PULL_ENABLE,
                        DDK_IOMUX_PAD_VOLTAGE_3V3);
	}

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

	//init i2c bus for GPIOX
	GpioI2CInit();
}

GPIOClass::~GPIOClass(void)
{
	DeleteCriticalSection(&csGpioLock);
	pGpioPinTab = NULL;

	//de-init the handler of microsecond delay
	if(pv_HWregDIGCTL != NULL)
	{
		MmUnmapIoSpace((PVOID)pv_HWregDIGCTL, 0x1000);
	}
}

//
// GPIO functions
//
BOOL GPIOClass::PIO_OutEnable( UINT32 dwGpioBits )
{
	DWORD	i;
	DWORD	dwCurrentGPIOX_DIR;
	BYTE	uBuf[4];
	BYTE	ucCmd;

	EnterCriticalSection(&csGpioLock);
	if(!(dwGpioBits & 0x80000000))		// -> GPIO
	{
		if( pGpioPinTab == NULL )
		{
			RETAILMSG(1,(TEXT("PIO_OutEnable: No Gpio Pin Table!\r\n")));
			return FALSE;
		}

		for(i = 0; i < dwGpioPinTabLen; i++)
		{
			if( !(dwGpioBits & (1 << i)) )
			{
				continue;
			}
		
			DDKGpioEnableDataPin(pGpioPinTab[i], 1);
		}
	}
	else							// -> GPIOX
	{
		dwCurrentGPIOX_DIR = dwGPIOX_DIR & ~dwGpioBits;		// BITx = 0: output, = 1: input
		uBuf[0] = (BYTE)((dwCurrentGPIOX_DIR >>  0) & 0xFF);
		uBuf[1] = (BYTE)((dwCurrentGPIOX_DIR >>  8) & 0xFF);
		uBuf[2] = (BYTE)((dwCurrentGPIOX_DIR >> 16) & 0xFF);

		ucCmd = TCA6424A_CMD_AUTO_INC						// write all three ports with auto-inc function
			  | TCA6424A_CMD_DIR							// access direction registers
			  | TCA6424A_CMD_ADDR_PORT0;					// start from port0

		if(TCA6424A_GpioI2CWrite(ucCmd, uBuf, 3))
		{
			dwGPIOX_DIR = dwCurrentGPIOX_DIR;
		}
	}
	LeaveCriticalSection(&csGpioLock); 

	return TRUE;
}

BOOL GPIOClass::PIO_OutDisable( UINT32 dwGpioBits )
{
	DWORD	i;
	DWORD	dwCurrentGPIOX_DIR;
	BYTE	uBuf[4];
	BYTE	ucCmd;

	EnterCriticalSection(&csGpioLock);
	if(!(dwGpioBits & 0x80000000))		// -> GPIO
	{
		if( pGpioPinTab == NULL )
		{
			RETAILMSG(1,(TEXT("PIO_OutDisable: No Gpio Pin Table!\r\n")));
			return FALSE;
		}

		for(i = 0; i < dwGpioPinTabLen; i++)
		{
			if( !(dwGpioBits & (1 << i)) )
			{
				continue;
			}
		
			DDKGpioEnableDataPin(pGpioPinTab[i], 0);
		}
	}
	else							// -> GPIOX
	{
		dwCurrentGPIOX_DIR = dwGPIOX_DIR | dwGpioBits;		// BITx = 0: output, = 1: input
		uBuf[0] = (BYTE)((dwCurrentGPIOX_DIR >>  0) & 0xFF);
		uBuf[1] = (BYTE)((dwCurrentGPIOX_DIR >>  8) & 0xFF);
		uBuf[2] = (BYTE)((dwCurrentGPIOX_DIR >> 16) & 0xFF);

		ucCmd = TCA6424A_CMD_AUTO_INC						// write all three ports with auto-inc function
			  | TCA6424A_CMD_DIR							// access direction registers
			  | TCA6424A_CMD_ADDR_PORT0;					// start from port0

		if(TCA6424A_GpioI2CWrite(ucCmd, uBuf, 3))
		{
			dwGPIOX_DIR = dwCurrentGPIOX_DIR;
		}
	}
	LeaveCriticalSection(&csGpioLock); 

	return TRUE;
}

BOOL GPIOClass::PIO_OutSet( UINT32 dwGpioBits )
{
	DWORD	i;
	DWORD	dwCurrentGPIOX_DOUT;
	BYTE	uBuf[4];
	BYTE	ucCmd;

	EnterCriticalSection(&csGpioLock);
	if(!(dwGpioBits & 0x80000000))		// -> GPIO
	{
		if( pGpioPinTab == NULL )
		{
			RETAILMSG(1,(TEXT("PIO_OutDisable: No Gpio Pin Table!\r\n")));
			return FALSE;
		}

		for(i = 0; i < dwGpioPinTabLen; i++)
		{
			if( !(dwGpioBits & (1 << i)) )
			{
				continue;
			}
		
			DDKGpioWriteDataPin(pGpioPinTab[i], 1);
		}
	}
	else							// -> GPIOX
	{
		dwCurrentGPIOX_DOUT = dwGPIOX_DOUT | dwGpioBits;		// BITx = 0: output 0, = 1: output 1
		uBuf[0] = (BYTE)((dwCurrentGPIOX_DOUT >>  0) & 0xFF);
		uBuf[1] = (BYTE)((dwCurrentGPIOX_DOUT >>  8) & 0xFF);
		uBuf[2] = (BYTE)((dwCurrentGPIOX_DOUT >> 16) & 0xFF);

		ucCmd = TCA6424A_CMD_AUTO_INC						// write all three ports with auto-inc function
			  | TCA6424A_CMD_OUTPUT							// access data output registers
			  | TCA6424A_CMD_ADDR_PORT0;					// start from port0

		if(TCA6424A_GpioI2CWrite(ucCmd, uBuf, 3))
		{
			dwGPIOX_DOUT = dwCurrentGPIOX_DOUT;
		}
	}
	LeaveCriticalSection(&csGpioLock); 

	return TRUE;
}

BOOL GPIOClass::PIO_OutClear( UINT32 dwGpioBits )
{
	DWORD	i;
	DWORD	dwCurrentGPIOX_DOUT;
	BYTE	uBuf[4];
	BYTE	ucCmd;

	EnterCriticalSection(&csGpioLock);
	if(!(dwGpioBits & 0x80000000))		// -> GPIO
	{
		if( pGpioPinTab == NULL )
		{
			RETAILMSG(1,(TEXT("PIO_OutDisable: No Gpio Pin Table!\r\n")));
			return FALSE;
		}

		for(i = 0; i < dwGpioPinTabLen; i++)
		{
			if( !(dwGpioBits & (1 << i)) )
			{
				continue;
			}
		
			DDKGpioWriteDataPin(pGpioPinTab[i], 0);
		}
	}
	else							// -> GPIOX
	{
		dwCurrentGPIOX_DOUT = dwGPIOX_DOUT & ~dwGpioBits;		// BITx = 0: output 0, = 1: output 1
		uBuf[0] = (BYTE)((dwCurrentGPIOX_DOUT >>  0) & 0xFF);
		uBuf[1] = (BYTE)((dwCurrentGPIOX_DOUT >>  8) & 0xFF);
		uBuf[2] = (BYTE)((dwCurrentGPIOX_DOUT >> 16) & 0xFF);

		ucCmd = TCA6424A_CMD_AUTO_INC						// write all three ports with auto-inc function
			  | TCA6424A_CMD_OUTPUT							// access data output registers
			  | TCA6424A_CMD_ADDR_PORT0;					// start from port0

		if(TCA6424A_GpioI2CWrite(ucCmd, uBuf, 3))
		{
			dwGPIOX_DOUT = dwCurrentGPIOX_DOUT;
		}
	}
	LeaveCriticalSection(&csGpioLock); 

	return TRUE;
}

BOOL GPIOClass::PIO_State( UINT32* pStateBits )
{
	DWORD	i;
	DWORD	dwGpioBits = *pStateBits;
	UINT32	uData = 0;
	BYTE	uBuf[4];
	BYTE	ucCmd;

	EnterCriticalSection(&csGpioLock);
	if(!(dwGpioBits & 0x80000000))		// -> GPIO
	{
		if( pGpioPinTab == NULL )
		{
			RETAILMSG(1,(TEXT("PIO_OutDisable: No Gpio Pin Table!\r\n")));
			return FALSE;
		}

		dwGpioBits = 0;
		for(i = 0; i < dwGpioPinTabLen; i++)
		{
			DDKGpioReadDataPin(pGpioPinTab[i], &uData);
			if( uData )
			{
				dwGpioBits |= (1 << i);
			}
		}
	}
	else								// -> GPIOX
	{
		memset(uBuf, 0xFF, 4);
		ucCmd = TCA6424A_CMD_AUTO_INC						// read all three ports with auto-inc function
			  | TCA6424A_CMD_INPUT							// access data input registers
			  | TCA6424A_CMD_ADDR_PORT0;					// start from port0

		if(TCA6424A_GpioI2CRead(ucCmd, uBuf, 3))
		{
			dwGpioBits = (dwGpioBits << 8) | uBuf[2];
			dwGpioBits = (dwGpioBits << 8) | uBuf[1];
			dwGpioBits = (dwGpioBits << 8) | uBuf[0];
		}
	}
	// save the data read finally
	*pStateBits = dwGpioBits;
	LeaveCriticalSection(&csGpioLock); 

	return TRUE;
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

