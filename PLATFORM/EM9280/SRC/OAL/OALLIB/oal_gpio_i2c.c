//
//------------------------------------------------------------------------------
//
// Copyright (C) 2012, Emtronix, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: oal_gpio_spi.c
//
// EM9280 GPIO based SPI API code. The API are used to access HT45B0F 
//
//------------------------------------------------------------------------------
#include <bsp.h>
#include "em9280_oal.h"

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregCLKCTRL;
extern PVOID pv_HWregPINCTRL;		// handler for pin mux operations

//------------------------------------------------------------------------------
// Defines
#define	GPIO_I2C_SCL_PIN			DDK_IOMUX_GPIO1_8
#define	GPIO_I2C_SDA_PIN			DDK_IOMUX_GPIO1_9
#define	udelay(v)					OALStall(v)

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
//
// Function: OALGpioI2cInit
//
// Function is called by PostInit to setup GPIO pins for I2C
//
//------------------------------------------------------------------------------
void OALGpioI2cInit(void)
{
    DDKIomuxSetPinMux(GPIO_I2C_SCL_PIN, DDK_IOMUX_MODE_GPIO);		// switch to GPIO1_8 -> muxmode = 2'b11
    DDKGpioEnableDataPin(GPIO_I2C_SCL_PIN, 1);						// set GPIO1_8 output enable
	DDKGpioWriteDataPin(GPIO_I2C_SCL_PIN, 1);						// set GPIO1_8 output value = 1 -> I2C_SCL = 1;

	DDKIomuxSetPadConfig(GPIO_I2C_SCL_PIN, 
        DDK_IOMUX_PAD_DRIVE_8MA, 
        DDK_IOMUX_PAD_PULL_ENABLE,
        DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(GPIO_I2C_SDA_PIN, DDK_IOMUX_MODE_GPIO);		// switch to GPIO1_9 -> muxmode = 2'b11
    DDKGpioEnableDataPin(GPIO_I2C_SDA_PIN, 0);						// set GPIO1_9 output disable
	DDKGpioWriteDataPin(GPIO_I2C_SDA_PIN, 1);						// set GPIO1_9 output value = 1 -> I2C_SDA = 1;

	DDKIomuxSetPadConfig(GPIO_I2C_SDA_PIN, 
        DDK_IOMUX_PAD_DRIVE_8MA, 
        DDK_IOMUX_PAD_PULL_ENABLE,
        DDK_IOMUX_PAD_VOLTAGE_3V3);
}

//------------------------------------------------------------------------------
// The basic routines for I2C timing
//------------------------------------------------------------------------------
__inline void SCL_SET(void)
{
	//DDKGpioWriteDataPin(GPIO_I2C_SCL_PIN, 1);		// DDK_IOMUX_GPIO1_8
	HW_PINCTRL_DOUT1_SET((1 << 8));	
}

__inline void SCL_CLR(void)
{
	//DDKGpioWriteDataPin(GPIO_I2C_SCL_PIN, 0);		// DDK_IOMUX_GPIO1_8
	HW_PINCTRL_DOUT1_CLR((1 << 8));	
}

__inline void SDA_SET(void)
{
	//DDKGpioWriteDataPin(GPIO_I2C_SDA_PIN, 1);		//DDK_IOMUX_GPIO1_9
	HW_PINCTRL_DOUT1_SET((1 << 9));	
}

__inline void SDA_CLR(void)
{
	DDKGpioWriteDataPin(GPIO_I2C_SDA_PIN, 0);		//DDK_IOMUX_GPIO1_9
	HW_PINCTRL_DOUT1_CLR((1 << 9));	
}

__inline void SDA_OUTEN(void)
{
	//DDKGpioEnableDataPin(GPIO_I2C_SDA_PIN, 1);	//DDK_IOMUX_GPIO1_9
	HW_PINCTRL_DOE1_SET((1 << 9));	
}

__inline void SDA_OUTDIS(void)
{
	//DDKGpioEnableDataPin(GPIO_I2C_SDA_PIN, 0);	//DDK_IOMUX_GPIO1_9
	HW_PINCTRL_DOE1_CLR((1 << 9));	
}

__inline UINT32 SDA_STATE(void)
{
	UINT32	Data;

	//DDKGpioReadDataPin(GPIO_I2C_SDA_PIN, &Data);
	//return Data;

	Data = HW_PINCTRL_DIN1_RD();
	if(Data & (1 << 9))
	{
		return 1;
	}

	return 0;
}


//------------------------------------------------------------------------------
// The basic routines for I2C read / write
//------------------------------------------------------------------------------
BOOL OALGpioI2cRead(DWORD dwAddr, DWORD dwCmd, PBYTE pDataBuf, DWORD dwDataLength)
{
	DWORD  	i1, i2;
	DWORD	count = 0;	
	BYTE 	ub[4];
	BYTE	uTmp;

	//UNREFERENCED_PARAMETER(dwAddr);
	//UNREFERENCED_PARAMETER(dwCmd);
	//UNREFERENCED_PARAMETER(pDataBuf);
	//UNREFERENCED_PARAMETER(dwDataLength);

	// Session 1: setup register pointer
	ub[count] = (BYTE)(dwAddr & 0xfe);			// 1st byte = ID + Write-Bit(=0)
	count++;
	if(dwCmd != (DWORD)-1)
	{
		ub[count] = (BYTE)(dwCmd & 0xff);		// 2nd byte = command
		count++;
	}

	// START condition
	SDA_OUTEN();					//set SDA as output
	SDA_SET();						//set SDA HIGH
	SCL_SET();						//set SCL HIGH
	udelay(3);						//can be more than 63us for a new START condition
	SDA_CLR();						//set SDA LOW
	udelay(0);						//I2C timing: start hold time > 0.6us
	SCL_CLR();						//set SCK LOW
	udelay(1);						//I2C timing: SCK low time  > 1.3us

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
			//udelay(0);					//I2C timing: data setup time > 200ns

			// issue a clock
			SCL_SET();					//set SCL HIGH
			udelay(0);					//I2C timing: SCK high time > 0.6us
			SCL_CLR();					//set SCK LOW
			udelay(1);					//I2C timing: SCK low time > 1.3us
			ub[i1] = ub[i1] << 1;
		}

		//check ACK(Low state) from I2C device 
		SDA_OUTDIS();					//set SDA as input
		//udelay(0);						//I2C timing: data (from I2C device) setup time > 200ns
		SCL_SET();						//issue a clock
		udelay(1);						//I2C timing: SCK high time > 0.6us
		if(SDA_STATE())					//read ACK_BIT issued from I2C device
		{
			// NO ACK, so setup STOP condition
			SCL_SET();					//SCL = "1"
			udelay(0);					//I2C timing: stop setup timing > 0.6us
			SDA_SET();					//SDA is in input status!

			OALMSG(1, (L"OALGpioI2cRead: no ACK, write aborted\r\n"));
			return FALSE;				// no acknowledgement -> failed
		}
		SCL_CLR();						//SCL = "0";
		udelay(1);						//I2C timing: SCK low time > 1.3us
		//check ACK passed, config SDA as output for further outputing
		SDA_OUTEN();					//set SDA as output
	}

	// issue Repeated START condition
	SDA_OUTEN();						//set SDA as output
	SDA_SET();							//set SDA HIGH
	SCL_SET();							//set SCK HIGH
	udelay(0);							//I2C timing: SCK high time > 0.6us
	SDA_CLR();							//set SDA LOW
	udelay(0);							//I2C timing: start hold time > 0.6us
	SCL_CLR();							//set SCK LOW
	udelay(1);							//I2C timing: SCK low time  > 1.3us

	// Session 2: read data  from the device
	// Session 2-part 1: write read cmd to the device
	uTmp = (BYTE)(dwAddr | 0x01);		// 1st byte = ID + Read-Bit(=1)
	for(i2 = 0; i2 < 8; i2++)
	{
		// setup data onto SDA
		if(uTmp & 0x80)
		{
			SDA_SET();					//set SDA = 1
		}
		else
		{
			SDA_CLR();					//clear SDA = 0
		}
		//udelay(0);						//I2C timing: data setup time > 200ns
		
		// issue a clock
		SCL_SET();						//Set SCL = 1;
		udelay(0);						//I2C timing: SCK high time > 0.6us
		SCL_CLR();						//clear SCL = 0
		udelay(1);						//I2C timing: SCK low time > 1.3us
		uTmp = uTmp << 1;
	}

	//check ACK(Low state) from I2C device 
	SDA_OUTDIS();					//set SDA as input
	//udelay(0);					//I2C timing: data(from I2C device) setup time > 200ns
	SCL_SET();						//issue a clock
	udelay(0);						//I2C timing: SCK high time > 0.6us
	if(SDA_STATE()) 				//read ACK_BIT issued from I2C device
	{
		// NO ACK, so setup STOP condition
		SCL_SET();					//Set SCK=1
		udelay(0);					//I2C timing: stop setup timing > 0.6us
		SDA_SET();					//SDA is in input status!
		OALMSG(1, (L"OALGpioI2cRead: no ACK, read aborted\r\n"));
		return FALSE; 				//no acknowledgement -> failed
	}
	SCL_CLR();						//clear SCK=0;
	udelay(1);						//I2C timing: SCK low time > 1.3us
	//check ACK passed, config SDA as output for further outputing
	SDA_OUTEN();					//set SDA as output

	// Session 2-part 2: read data from device to pBuffer, and issue ACK properly
	for(i1 = 0; i1 < dwDataLength; i1++)
	{
		uTmp = 0;
		SDA_OUTDIS();				//set SDA as input
		// then read a abyte
		for(i2 = 0; i2 < 8; i2++)
		{
			uTmp = uTmp << 1;
			// issue a clock
			SCL_SET();				//Set SCK=1
			udelay(0);				//I2C timing: SCK high time > 0.6us
			if(SDA_STATE())			// read SDA	
			{
				uTmp = uTmp | 0x01;
			}
			SCL_CLR();				//Set SCK=0
			udelay(1);				//I2C timing: SCK low time > 1.3us
		}
		pDataBuf[i1] = uTmp;		//save the data into buffer
		
		//issue ACK condition properly
		SDA_OUTEN();
		if(i1 == (dwDataLength - 1))
		{
			//This is the last byte, issue NO-ACK condition
			SDA_SET();				//SDA = "1": NO-ACK
		}
		else
		{
			//There are more byte need to read, so issue ACK condition
			SDA_CLR();				//SDA = "0": ACK (active low)
		}
		udelay(0);					//I2C timing: data setup time > 200ns
		SCL_SET();					//Set SCL = 1
		udelay(1);					//I2C timing: SCK high time > 1.3us
		SCL_CLR();					//Set SCL = 0
		udelay(0);					//I2C timing: SCK low time > 0.6us
		SDA_SET();					//Set SDA = 1, end ACK
	}

	// Here we issue a STOP condition
	SDA_CLR();						//Set SDA=0
	SCL_SET();						//Set SCK=1
	udelay(0);						//I2C timing: stop setup time > 0.6us
	SDA_SET();						//Set SDA=1
	SDA_OUTDIS();					//set SDA as input

	return TRUE;
}

BOOL OALGpioI2cWrite(DWORD dwAddr, DWORD dwCmd, PBYTE pDataBuf, DWORD dwDataLength)
{
	DWORD  	i1, i2;
	DWORD	count = 0;	
	BYTE 	ub[4];
	BYTE	uTmp;

	//UNREFERENCED_PARAMETER(dwAddr);
	//UNREFERENCED_PARAMETER(dwCmd);
	//UNREFERENCED_PARAMETER(pDataBuf);
	//UNREFERENCED_PARAMETER(dwDataLength);

	//fill data
	ub[count] = (BYTE)(dwAddr & 0xfe);			// 1st byte = ID + Write-Bit(=0)
	count++;
	if(dwCmd != (DWORD)-1)
	{
		ub[count] = (BYTE)(dwCmd & 0xff);		// 2nd byte = command
		count++;
	}

	// START condition
	SDA_OUTEN();					//set SDA as output
	SDA_SET();						//set SDA HIGH
	SCL_SET();						//set SCL HIGH
	udelay(3);						//can be more than 63us for a new START condition
	SDA_CLR();						//set SDA LOW
	udelay(0);						//I2C timing: start hold time > 0.6us
	SCL_CLR();						//set SCK LOW
	udelay(1);						//I2C timing: SCK low time  > 1.3us

	// send addr and cmd
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
			//udelay(0);					//I2C timing: data setup time > 200ns

			// issue a clock
			SCL_SET();					//set SCL HIGH
			udelay(0);					//I2C timing: SCK high time > 0.6us
			SCL_CLR();					//set SCK LOW
			udelay(1);					//I2C timing: SCK low time > 1.3us
			ub[i1] = ub[i1] << 1;
		}

		//check ACK(Low state) from I2C device 
		SDA_OUTDIS();					//set SDA as input
		//udelay(0);						//I2C timing: data (from I2C device) setup time > 200ns
		SCL_SET();						//issue a clock
		udelay(0);						//I2C timing: SCK high time > 0.6us
		if(SDA_STATE())					//read ACK_BIT issued from I2C device
		{
			// NO ACK, so setup STOP condition
			SCL_SET();					//SCL = "1"
			udelay(0);					//I2C timing: stop setup timing > 0.6us
			SDA_SET();					//SDA is in input status!

			OALMSG(1, (L"OALGpioI2cWrite: no ACK, write aborted\r\n"));
			return FALSE;				// no acknowledgement -> failed
		}
		SCL_CLR();						//SCL = "0";
		udelay(1);						//I2C timing: SCK low time > 1.3us
		//check ACK passed, config SDA as output for further outputing
		SDA_OUTEN();					//set SDA as output
	}

	for(i1 = 0; i1 < dwDataLength; i1++)
	{
		uTmp = pDataBuf[i1];
		for(i2 = 0; i2 < 8; i2++)
		{
			// setup data onto SDA
			if(uTmp & 0x80)
			{
				SDA_SET();				//set SDA HIGH
			}
			else
			{
				SDA_CLR();				//Clear SDA to LOW
			}
			//udelay(0);					//I2C timing: data setup time > 200ns

			// issue a clock
			SCL_SET();					//set SCL HIGH
			udelay(0);					//I2C timing: SCK high time > 0.6us
			SCL_CLR();					//set SCK LOW
			udelay(1);					//I2C timing: SCK low time > 1.3us
			uTmp = uTmp << 1;
		}

		//check ACK(Low state) from I2C device 
		SDA_OUTDIS();					//set SDA as input
		//udelay(0);						//I2C timing: data (from I2C device) setup time > 200ns
		SCL_SET();						//issue a clock
		udelay(0);						//I2C timing: SCK high time > 0.6us
		if(SDA_STATE())					//read ACK_BIT issued from I2C device
		{
			// NO ACK, so setup STOP condition
			SCL_SET();					//SCL = "1"
			udelay(0);					//I2C timing: stop setup timing > 0.6us
			SDA_SET();					//SDA is in input status!

			OALMSG(1, (L"OALGpioI2cWrite: no ACK, write aborted\r\n"));
			return FALSE;				// no acknowledgement -> failed
		}
		SCL_CLR();						//SCL = "0";
		udelay(1);						//I2C timing: SCK low time > 1.3us
		//check ACK passed, config SDA as output for further outputing
		SDA_OUTEN();					//set SDA as output
	}

	// Here we issue a STOP condition
	SDA_CLR();							//Set SDA = 0;
	SCL_SET();							//Set SCL = 1;
	udelay(0);							//I2C timing: stop setup time > 0.6us
	SDA_SET();							//Set SDA = 1
	SDA_OUTDIS();						//set SDA as input

	return TRUE;
}

BOOL OALI2C_Access(VOID* pInpBuffer, UINT32 inpSize)
{
	PI2cAccessInfo	pInfo;
	BOOL			bRet = FALSE;
	BYTE			uValue;
	BYTE			uMask;

	if(!pInpBuffer || (inpSize != sizeof(I2cAccessInfo)))
	{
		OALMSG(1, (L"->OALI2C_Access::parameter invalid!\r\n"));
		return FALSE;
	}

	pInfo = (PI2cAccessInfo)pInpBuffer;
	switch(pInfo->dwAccessCode)
	{
	case I2C_ACCESS_CODE_READ:
		bRet = OALGpioI2cRead(pInfo->dwAddr, pInfo->dwCmd, pInfo->pDataBuf, pInfo->dwDataLength);
		break;

	case I2C_ACCESS_CODE_WRITE:
		bRet = OALGpioI2cWrite(pInfo->dwAddr, pInfo->dwCmd, pInfo->pDataBuf, pInfo->dwDataLength);
		break;

	case I2C_ACCESS_CODE_SET:
		uMask = *pInfo->pDataBuf;
		bRet = OALGpioI2cRead(pInfo->dwAddr, pInfo->dwCmd, &uValue, 1);
		if(!bRet)
		{
			OALMSG(1, (L"OALI2C_Access::I2C_ACCESS_CODE_SET: OALGpioI2cRead failed\r\n"));
			break;
		}
		uValue |= uMask;
		bRet = OALGpioI2cWrite(pInfo->dwAddr, pInfo->dwCmd, &uValue, 1);
		break;

	case I2C_ACCESS_CODE_CLEAR:
		uMask = *pInfo->pDataBuf;
		bRet = OALGpioI2cRead(pInfo->dwAddr, pInfo->dwCmd, &uValue, 1);
		if(!bRet)
		{
			OALMSG(1, (L"OALI2C_Access::I2C_ACCESS_CODE_CLEAR: OALGpioI2cRead failed\r\n"));
			break;
		}
		uValue &= ~uMask;
		bRet = OALGpioI2cWrite(pInfo->dwAddr, pInfo->dwCmd, &uValue, 1);
		break;

	default:
		OALMSG(1, (L"->OALI2C_Access::unknown access code!\r\n"));
	}

	return bRet;
}
