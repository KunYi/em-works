#include "bsp.h"
#include "em9k_ddk.h"
#include "em9k_ddk_class.h"
#include "tca6424a_i2c.h"


//-----------------------------------------------------------------------------
// Global Variable
// the handler to read us counter
PVOID pv_HWregDIGCTL = NULL;

//-----------------------------------------------------
// GPIOClass member function implementation
//-----------------------------------------------------
BOOL Em9kDDKClass::GpioI2CInit()
{
	DWORD			i;
    DDK_GPIO_CFG	intrCfg;

    intrCfg.DDK_PIN_IO           = DDK_GPIO_INPUT;
    intrCfg.DDK_PIN_IRQ_CAPABLE  = DDK_GPIO_IRQ_CAPABLE;
    intrCfg.DDK_PIN_IRQ_ENABLE   = DDK_GPIO_IRQ_DISABLED;
    intrCfg.DDK_PIN_IRQ_LEVEL    = DDK_GPIO_IRQ_EDGE;
    intrCfg.DDK_PIN_IRQ_POLARITY = DDK_GPIO_IRQ_POLARITY_LO;		// interrupt trigger on falling edge

	I2C_SCL_PIN = DDK_IOMUX_GPIO1_8;
	I2C_SDA_PIN = DDK_IOMUX_GPIO1_9;
	TCA6424A_INT_PIN = DDK_IOMUX_GPIO1_16;
	TCA6424A_RST_PIN = DDK_IOMUX_GPIO1_17;

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

	if(!DDKGpioConfig(TCA6424A_RST_PIN, intrCfg))
	{
		RETAILMSG(1,(TEXT("GPIOClass: config TCA6424A_RST_PIN failed!\r\n")));
	}
	//switch to GPIO mode
    DDKIomuxSetPinMux(TCA6424A_RST_PIN, DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPadConfig(TCA6424A_RST_PIN, DDK_IOMUX_PAD_DRIVE_4MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
	DDKGpioEnableDataPin(TCA6424A_RST_PIN, 1);		// output enable
	DDKGpioWriteDataPin(TCA6424A_RST_PIN, 0);		// output "0" to reset TCA6424A chip
	Sleep(2);
	DDKGpioWriteDataPin(TCA6424A_RST_PIN, 1);		// output "1" to release reset pin of TCA6424A

	// finally SCL output enable
	DDKGpioEnableDataPin(I2C_SCL_PIN, 1);

	// init variables for TCA6424A chip
	uTCA6424A_Addr = TCA6424A_ADDR;		// according to datasheet, and ADDR-pin lie to GND
	for(i = 0; i < 3; i++)
	{
		uGPIOX_DOUT[i] = 0xFF;			// on power-up, all bits are HIGH, valid with BIT0 - BIT23 
		uGPIOX_INV[i]  = 0;				// on power-up, all bits are LOW, valid with BIT0 - BIT23 
		uGPIOX_DIR[i]  = 0xFF;			// on power-up, all bits are HIGH, valid with BIT0 - BIT23
		uGPIOX_PORT[i] = (BYTE)i;
	}

	return TRUE;
}

__inline void Em9kDDKClass::SCL_SET(void)
{
	DDKGpioWriteDataPin(I2C_SCL_PIN, 1);
}

__inline void Em9kDDKClass::SCL_CLR(void)
{
	DDKGpioWriteDataPin(I2C_SCL_PIN, 0);
}

__inline void Em9kDDKClass::SDA_SET(void)
{
	DDKGpioWriteDataPin(I2C_SDA_PIN, 1);
}

__inline void Em9kDDKClass::SDA_CLR(void)
{
	DDKGpioWriteDataPin(I2C_SDA_PIN, 0);
}

__inline void Em9kDDKClass::SDA_OUTEN(void)
{
	DDKGpioEnableDataPin(I2C_SDA_PIN, 1);
}

__inline void Em9kDDKClass::SDA_OUTDIS(void)
{
	DDKGpioEnableDataPin(I2C_SDA_PIN, 0);
}

__inline UINT32 Em9kDDKClass::SDA_STATE(void)
{
	UINT32	Data;

	DDKGpioReadDataPin(I2C_SDA_PIN, &Data);
	return Data;
}

BOOL Em9kDDKClass::GpioSPIInit()
{
    DDK_GPIO_CFG	intrCfg;

    intrCfg.DDK_PIN_IO           = DDK_GPIO_INPUT;
    intrCfg.DDK_PIN_IRQ_CAPABLE  = DDK_GPIO_IRQ_CAPABLE;
    intrCfg.DDK_PIN_IRQ_ENABLE   = DDK_GPIO_IRQ_DISABLED;
    intrCfg.DDK_PIN_IRQ_LEVEL    = DDK_GPIO_IRQ_EDGE;
    intrCfg.DDK_PIN_IRQ_POLARITY = DDK_GPIO_IRQ_POLARITY_LO;		// interrupt trigger on falling edge

	DDK_IOMUX_PIN	SPI_MISO_PIN;			// data input pin for SPI interface
	DDK_IOMUX_PIN	SPI_MOSI_PIN;			// data output pin for SPI interface
	DDK_IOMUX_PIN	SPI_SCLK_PIN;			// clock output pin for SPI interface
	DDK_IOMUX_PIN	SPI_CS0N_PIN;			// chip select 0 pin for SPI interface 
	DDK_IOMUX_PIN	SPI_CS1N_PIN;			// chip select 0 pin for SPI interface 
	DDK_IOMUX_PIN	SPI_CS2N_PIN;			// chip select 0 pin for SPI interface 

	SPI_MISO_PIN = DDK_IOMUX_GPIO2_1;
	SPI_MOSI_PIN = DDK_IOMUX_GPIO2_0;
	SPI_SCLK_PIN = DDK_IOMUX_SSP0_D2;
	SPI_CS0N_PIN = DDK_IOMUX_GPIO0_17;
	SPI_CS1N_PIN = DDK_IOMUX_GPIO0_21;
	SPI_CS2N_PIN = DDK_IOMUX_GPIO0_28;

	//config GPIOs for I2C
	if(!DDKGpioConfig(SPI_MISO_PIN, intrCfg))
	{
		RETAILMSG(1,(TEXT("Em9kDDKClass: config SPI_MISO_PIN failed!\r\n")));
	}
	//switch to GPIO mode
    DDKIomuxSetPinMux(SPI_MISO_PIN, DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPadConfig(SPI_MISO_PIN, DDK_IOMUX_PAD_DRIVE_4MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

	if(!DDKGpioConfig(SPI_MOSI_PIN, intrCfg))
	{
		RETAILMSG(1,(TEXT("Em9kDDKClass: config SPI_MOSI_PIN failed!\r\n")));
	}
	//switch to GPIO mode
    DDKIomuxSetPinMux(SPI_MOSI_PIN, DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPadConfig(SPI_MOSI_PIN, DDK_IOMUX_PAD_DRIVE_4MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
	
	if(!DDKGpioConfig(SPI_SCLK_PIN, intrCfg))
	{
		RETAILMSG(1,(TEXT("Em9kDDKClass: config SPI_SCLK_PIN failed!\r\n")));
	}
	//switch to GPIO mode
    DDKIomuxSetPinMux(SPI_SCLK_PIN, DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPadConfig(SPI_SCLK_PIN, DDK_IOMUX_PAD_DRIVE_4MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

	if(!DDKGpioConfig(SPI_CS0N_PIN, intrCfg))
	{
		RETAILMSG(1,(TEXT("Em9kDDKClass: config SPI_CS0N_PIN failed!\r\n")));
	}
	//switch to GPIO mode
    DDKIomuxSetPinMux(SPI_CS0N_PIN, DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPadConfig(SPI_CS0N_PIN, DDK_IOMUX_PAD_DRIVE_4MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

	if(!DDKGpioConfig(SPI_CS1N_PIN, intrCfg))
	{
		RETAILMSG(1,(TEXT("Em9kDDKClass: config SPI_CS1N_PIN failed!\r\n")));
	}
	//switch to GPIO mode
    DDKIomuxSetPinMux(SPI_CS1N_PIN, DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPadConfig(SPI_CS1N_PIN, DDK_IOMUX_PAD_DRIVE_4MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

	if(!DDKGpioConfig(SPI_CS2N_PIN, intrCfg))
	{
		RETAILMSG(1,(TEXT("Em9kDDKClass: config SPI_CS2N_PIN failed!\r\n")));
	}
	//switch to GPIO mode
    DDKIomuxSetPinMux(SPI_CS2N_PIN, DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPadConfig(SPI_CS2N_PIN, DDK_IOMUX_PAD_DRIVE_4MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

	// then setup the output value for those output pins
	DDKGpioWriteDataPin(SPI_MOSI_PIN, 0);			// SPI dout output = 0
	DDKGpioWriteDataPin(SPI_SCLK_PIN, 1);			// SPI sclk output = 1
	DDKGpioWriteDataPin(SPI_CS0N_PIN, 1);			// SPI CS0N output = 1
	DDKGpioWriteDataPin(SPI_CS1N_PIN, 1);			// SPI CS1N output = 1
	DDKGpioWriteDataPin(SPI_CS2N_PIN, 1);			// SPI CS2N output = 1

	// finally SPI output pins enable
	DDKGpioEnableDataPin(SPI_MOSI_PIN, 1);
	DDKGpioEnableDataPin(SPI_SCLK_PIN, 1);
	DDKGpioEnableDataPin(SPI_CS0N_PIN, 1);
	DDKGpioEnableDataPin(SPI_CS1N_PIN, 1);
	DDKGpioEnableDataPin(SPI_CS2N_PIN, 1);

	return TRUE;
}

__inline void	Em9kDDKClass::SPI_ENABLE(DWORD dwCSNum)
{
	switch(dwCSNum)
	{
	case 0:
		DDKGpioWriteDataPin(SPI_CS0N_PIN, 0);		// active low
		break;

	case 1:
		DDKGpioWriteDataPin(SPI_CS1N_PIN, 0);		// active low
		break;

	case 2:
		DDKGpioWriteDataPin(SPI_CS2N_PIN, 0);		// active low
		break;
	}
}

__inline void	Em9kDDKClass::SPI_DISABLE(DWORD dwCSNum)
{
	switch(dwCSNum)
	{
	case 0:
		DDKGpioWriteDataPin(SPI_CS0N_PIN, 1);		// active low
		break;

	case 1:
		DDKGpioWriteDataPin(SPI_CS1N_PIN, 1);		// active low
		break;

	case 2:
		DDKGpioWriteDataPin(SPI_CS2N_PIN, 1);		// active low
		break;
	}
}

__inline void	Em9kDDKClass::SPI_DOUT_SET(void)
{
	DDKGpioWriteDataPin(SPI_MOSI_PIN, 1);
}

__inline void	Em9kDDKClass::SPI_DOUT_CLR(void)
{
	DDKGpioWriteDataPin(SPI_MOSI_PIN, 0);
}

__inline void	Em9kDDKClass::SPI_SCLK_SET(void)
{
	DDKGpioWriteDataPin(SPI_SCLK_PIN, 1);
}

__inline void	Em9kDDKClass::SPI_SCLK_CLR(void)
{
	DDKGpioWriteDataPin(SPI_SCLK_PIN, 0);
}

__inline UINT32	Em9kDDKClass::SPI_DIN_STATE(void)
{
	UINT32	Data;

	DDKGpioReadDataPin(SPI_MISO_PIN, &Data);
	return Data;
}

//-----------------------------------------------------
// GPIOClass public member functions
//-----------------------------------------------------
Em9kDDKClass::Em9kDDKClass(void)
{
	// init critical scetions
	InitializeCriticalSection(&csGpioSpiMutex);	
	InitializeCriticalSection(&csGpioI2cMutex);	

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

Em9kDDKClass::~Em9kDDKClass(void)
{
	DeleteCriticalSection(&csGpioSpiMutex);
	DeleteCriticalSection(&csGpioI2cMutex);

	//de-init the handler of microsecond delay
	if(pv_HWregDIGCTL != NULL)
	{
		MmUnmapIoSpace((PVOID)pv_HWregDIGCTL, 0x1000);
	}
}


BOOL Em9kDDKClass::DDKGpioI2cWrite(DWORD dwDeviceID, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength)
{
	DWORD  	i1, i2;
	DWORD	count = 0;	
	BYTE 	ub[36];

	//fill data
	ub[count] = (BYTE)dwDeviceID & ~I2C_READ_FLAG;	// 1st byte = ID + Write-Bit(=0)
	count++;
	ub[count] = (BYTE)dwCmdAddr;					// 2nd byte = command
	count++;
	if(dwCmdAddr & 0x80000000)						// check if 2-byte address
	{
		ub[count] = (BYTE)(dwCmdAddr >> 8);			// 3rd byte = command
		count++;
	}
	
	for(i1 = 0; i1 < dwLength; i1++, count++)
	{
		ub[count] = pBuf[i1];		// data write to the register
	}

	EnterCriticalSection(&csGpioI2cMutex);
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

			LeaveCriticalSection(&csGpioI2cMutex); 
			RETAILMSG(1,(TEXT("GpioI2CWrite: no ACK, write aborted\r\n")));
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
	LeaveCriticalSection(&csGpioI2cMutex); 

	return TRUE;
}

BOOL Em9kDDKClass::DDKGpioI2cRead(DWORD dwDeviceID, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength)
{
	DWORD  	i1, i2;
	DWORD	count = 0;	
	BYTE 	ub[36];

	// Session 1: setup register pointer
	ub[count] = (BYTE)dwDeviceID & ~I2C_READ_FLAG;	// 1st byte = ID + Write-Bit(=0)
	count++;
	ub[count] = (BYTE)dwCmdAddr;					// 2nd byte = command
	count++;
	if(dwCmdAddr & 0x80000000)						// check if 2-byte address
	{
		ub[count] = (BYTE)(dwCmdAddr >> 8);			// 3rd byte = command
		count++;
	}

	EnterCriticalSection(&csGpioI2cMutex);
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

			LeaveCriticalSection(&csGpioI2cMutex); 
			RETAILMSG(1,(TEXT("GpioI2CRead: no ACK, write aborted\r\n")));
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
	ub[0] = (BYTE)dwDeviceID | I2C_READ_FLAG;		// 1st byte = ID + Read-Bit(=1)
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

		LeaveCriticalSection(&csGpioI2cMutex); 
		RETAILMSG(1,(TEXT("GpioI2CRead: no ACK, read aborted\r\n")));
		return FALSE; 				//no acknowledgement -> failed
	}
	SCL_CLR();						//clear SCK=0;
	udelay(2);						//I2C timing: SCK low time > 1.3us
	//check ACK passed, config SDA as output for further outputing
	SDA_OUTEN();					//set SDA as output

	// Session 2-part 2: read data from device to pBuffer, and issue ACK properly
	for(i1 = 0; i1 < dwLength; i1++)
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
		pBuf[i1] = ub[0];			//save the data into buffer
		
		//issue ACK condition properly
		SDA_OUTEN();
		if(i1 == (dwLength - 1))
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
	LeaveCriticalSection(&csGpioI2cMutex); 

	return TRUE;
}



//---------------------------------------------------
// GPIO functions
//
BOOL Em9kDDKClass::DDKGpioxPinOutEn(DDK_GPIOX_PIN gpiox_pin)
{
	DWORD	dwPortIndex;
	BYTE	uBitMask;
	BYTE	uCurrentValue;
	BYTE	ucCmd;
	BOOL	bRet = FALSE;

	dwPortIndex = gpiox_pin / 8;
	uBitMask = (BYTE)(1 << (gpiox_pin % 8));

	uCurrentValue = uGPIOX_DIR[dwPortIndex] & ~uBitMask;	// BITx = 0: output, = 1: input
	ucCmd = TCA6424A_CMD_DIR								// access direction registers
		  | uGPIOX_PORT[dwPortIndex];						// with this port index

	if(DDKGpioI2cWrite(uTCA6424A_Addr, ucCmd, &uCurrentValue, 1))
	{
		uGPIOX_DIR[dwPortIndex] = uCurrentValue;
		bRet = TRUE;
	}

	return bRet;
}

BOOL Em9kDDKClass::DDKGpioxPinOutDis(DDK_GPIOX_PIN gpiox_pin)
{
	DWORD	dwPortIndex;
	BYTE	uBitMask;
	BYTE	uCurrentValue;
	BYTE	ucCmd;
	BOOL	bRet = FALSE;

	dwPortIndex = gpiox_pin / 8;
	uBitMask = (BYTE)(1 << (gpiox_pin % 8));

	uCurrentValue = uGPIOX_DIR[dwPortIndex] | uBitMask;		// BITx = 0: output, = 1: input
	ucCmd = TCA6424A_CMD_DIR								// access direction registers
		  | uGPIOX_PORT[dwPortIndex];						// with this port index

	if(DDKGpioI2cWrite(uTCA6424A_Addr, ucCmd, &uCurrentValue, 1))
	{
		uGPIOX_DIR[dwPortIndex] = uCurrentValue;
		bRet = TRUE;
	}

	return bRet;
}

BOOL Em9kDDKClass::DDKGpioxPinSet(DDK_GPIOX_PIN gpiox_pin)
{
	DWORD	dwPortIndex;
	BYTE	uBitMask;
	BYTE	uCurrentValue;
	BYTE	ucCmd;
	BOOL	bRet = FALSE;

	dwPortIndex = gpiox_pin / 8;
	uBitMask = (BYTE)(1 << (gpiox_pin % 8));

	uCurrentValue = uGPIOX_DOUT[dwPortIndex] | uBitMask;	// BITx = 0: output 0, = 1: output 1
	ucCmd = TCA6424A_CMD_OUTPUT								// access data output registers
		  | uGPIOX_PORT[dwPortIndex];						// with this port index

	if(DDKGpioI2cWrite(uTCA6424A_Addr, ucCmd, &uCurrentValue, 1))
	{
		uGPIOX_DOUT[dwPortIndex] = uCurrentValue;
		bRet = TRUE;
	}

	return bRet;
}

BOOL Em9kDDKClass::DDKGpioxPinClear(DDK_GPIOX_PIN gpiox_pin)
{
	DWORD	dwPortIndex;
	BYTE	uBitMask;
	BYTE	uCurrentValue;
	BYTE	ucCmd;
	BOOL	bRet = FALSE;

	dwPortIndex = gpiox_pin / 8;
	uBitMask = (BYTE)(1 << (gpiox_pin % 8));

	uCurrentValue = uGPIOX_DOUT[dwPortIndex] & ~uBitMask;	// BITx = 0: output 0, = 1: output 1
	ucCmd = TCA6424A_CMD_OUTPUT								// access data output registers
		  | uGPIOX_PORT[dwPortIndex];						// with this port index

	if(DDKGpioI2cWrite(uTCA6424A_Addr, ucCmd, &uCurrentValue, 1))
	{
		uGPIOX_DOUT[dwPortIndex] = uCurrentValue;
		bRet = TRUE;
	}

	return bRet;
}

BOOL Em9kDDKClass::DDKGpioxPinState(DDK_GPIOX_PIN gpiox_pin, PDWORD pRetState)
{
	DWORD	dwPortIndex;
	BYTE	uBitMask;
	BYTE	uCurrentValue = 0;
	BYTE	ucCmd;
	BOOL	bRet = FALSE;

	dwPortIndex = gpiox_pin / 8;
	uBitMask = (BYTE)(1 << (gpiox_pin % 8));

	ucCmd = TCA6424A_CMD_INPUT							// access data input registers
		  | uGPIOX_PORT[dwPortIndex];					// with this port index

	*pRetState = 0;
	if(DDKGpioI2cRead(uTCA6424A_Addr, ucCmd, &uCurrentValue, 1))
	{
		if(uCurrentValue & uBitMask)
		{
			*pRetState = 0;
		}
		bRet = TRUE;
	}

	return bRet;
}


//-----------------------------------------------------
// GPIOClass other public functions
//-----------------------------------------------------
void Em9kDDKClass::udelay(DWORD dwMicroSecond)
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

//------------------------------------------------------------------------------
// The basic routines for SPI read / write
//------------------------------------------------------------------------------
BOOL Em9kDDKClass::DDKGpioSpiRead(DWORD dwCS, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength)
{
	DWORD	i1, i2;
	DWORD	count = 0;	
	BYTE 	ub[4];

	ub[count] = (BYTE)dwCmdAddr;
	count++;
	if(dwCmdAddr & 0x80000000)
	{
		ub[count] = (BYTE)(dwCmdAddr >> 8);
		count++;
	}

	EnterCriticalSection(&csGpioSpiMutex);
	SPI_ENABLE(dwCS);

	//output register address, beginning from MSB(A7)
	for(i2 = 0; i2 < count; i2)
	{
		for(i1 = 0; i1 < 8; i1++)
		{
			SPI_SCLK_CLR();			//SPI_SCLK = 0: data update
			
			if(ub[i2] & 0x80)	
			{
				SPI_DOUT_SET();
			}
			else
			{
				SPI_DOUT_CLR();
			}
			ub[i2] <<= 1;			// shift-left 1-bit

			SPI_SCLK_SET();			//SPI_SCLK = 1: rising-edge latch data into device
		}
	}

	udelay(1);					//delay 1us

	// read data from SPI device, beginning from MSB(D7)
	for(i2 = 0; i2 < dwLength; i2)
	{
		for(i1 = 0; i1 < 8; i1++)
		{
			SPI_SCLK_CLR();			//SPI_SCLK = 0: data update
			ub[0] <<= 1;
			SPI_SCLK_SET();			//SPI_SCLK = 1: rising-edge latch data into device
			if(SPI_DIN_STATE())
			{
				ub[0] |= 0x01;
			}
		}
		pBuf[i2] = ub[0];
	}

	SPI_DISABLE(dwCS);
	LeaveCriticalSection(&csGpioSpiMutex); 

	return TRUE;
}

BOOL Em9kDDKClass::DDKGpioSpiWrite(DWORD dwCS, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength)
{
	DWORD	i1, i2;
	DWORD	count = 0;	
	BYTE 	ub[4];

	ub[count] = (BYTE)dwCmdAddr;
	count++;
	if(dwCmdAddr & 0x80000000)
	{
		ub[count] = (BYTE)(dwCmdAddr >> 8);
		count++;
	}

	EnterCriticalSection(&csGpioSpiMutex);
	SPI_ENABLE(dwCS);

	//output register address, beginning from MSB(A7)
	for(i2 = 0; i2 < count; i2)
	{
		for(i1 = 0; i1 < 8; i1++)
		{
			SPI_SCLK_CLR();			//SPI_SCLK = 0: data update
			
			if(ub[i2] & 0x80)	
			{
				SPI_DOUT_SET();
			}
			else
			{
				SPI_DOUT_CLR();
			}
			ub[i2] <<= 1;			// shift-left 1-bit

			SPI_SCLK_SET();			//SPI_SCLK = 1: rising-edge latch data into device
		}
	}

	//output data, beginning from MSB(A7)
	for(i2 = 0; i2 < dwLength; i2)
	{
		ub[0] = pBuf[i2];
		for(i1 = 0; i1 < 8; i1++)
		{
			SPI_SCLK_CLR();			//SPI_SCLK = 0: data update
			
			if(ub[0] & 0x80)	
			{
				SPI_DOUT_SET();
			}
			else
			{
				SPI_DOUT_CLR();
			}
			ub[0] <<= 1;			// shift-left 1-bit

			SPI_SCLK_SET();			//SPI_SCLK = 1: rising-edge latch data into device
		}
	}

	SPI_DISABLE(dwCS);
	LeaveCriticalSection(&csGpioSpiMutex); 

	return TRUE;
}

