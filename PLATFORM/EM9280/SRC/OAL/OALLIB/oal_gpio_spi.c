//#ifdef	EM9280_GPIO_SPI
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
#include "em9280_oal_spi.h"

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregCLKCTRL;
extern PVOID pv_HWregPINCTRL;		// handler for pin mux operations
extern PVOID pv_HWRegPWM;			// handler for config PWM_7 output 1.8432MHz as COMCLK

//------------------------------------------------------------------------------
// Defines
#define	GPIO_SPI_MOSI_PIN			DDK_IOMUX_GPIO2_0
#define	GPIO_SPI_MISO_PIN			DDK_IOMUX_GPIO2_1
#define	GPIO_SPI_SCLK_PIN			DDK_IOMUX_GPIO2_2
#define	GPIO_SPI_CS0N_PIN			DDK_IOMUX_GPIO0_17
#define	GPIO_SPI_CS1N_PIN			DDK_IOMUX_GPIO0_21
#define	GPIO_SPI_CS2N_PIN			DDK_IOMUX_GPIO0_28

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
//
// Function: GPIO_SPI_PinInit
//
// Function is called by PostInit to setup GPIO pins for SPI
// SSP0_D0 / GPIO2_0 -> GPIO_SPI_MOSI
// SSP0_D1 / GPIO2_1 -> GPIO_SPI_MISO
// SSP0_D2 / GPIO2_2 -> GPIO_SPI_SCLK
//
// GPMI_CE1N / SSP3_D3  / GPIO0_17 -> GPIO_SPI_CS0N -> UART4
// GPMI_RDY1 / SSP1_CMD / GPIO0_21 -> GPIO_SPI_CS1N -> UART8
// GPMI_RSTN / SSP3_CMD / GPIO0_28 -> GPIO_SPI_CS2N -> UART9
//
// PWM_7 / GPIO3_26	-> COMCLK = 1.8432MHz
//
//------------------------------------------------------------------------------
void OALGpioSpiInit(void)
{
	DWORD	dwPeriodValue;

	//
	// step1: turn on clock to modules required
	//
	// enable clock source of PWM module -> 24MHz
    HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_PWM_CLK24M_GATE);
    // Turn on Clock for iomux module
    HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_CLKGATE | BM_PINCTRL_CTRL_SFTRST);
    // Turn on Clock for pwm module
	HW_PWM_CTRL_CLR(BM_PWM_CTRL_SFTRST);
    HW_PWM_CTRL_RD(); // waist some cyles.
	HW_PWM_CTRL_CLR(BM_PWM_CTRL_CLKGATE);
    HW_PWM_CTRL_RD(); // waist some cyles.
	if(!(HW_PWM_CTRL_RD() & BM_PWM_CTRL_PWM7_PRESENT))
	{
        OALMSG(1, (L"OALGpioSpiInit:PWM_7 is NOT present in this chip!\r\n"));
	}

	//
	// step2: make PWM_7 output 1.8432MHz => (24MHz main clock / 13) => 1.8462MHz with error 0.16% 
	//
	//HW_PWM_ACTIVEn_WR(7, 0x00060000);					// ACTIVE = 0, INACTIVE = 6
	HW_PWM_ACTIVEn_WR(7, 0x00010000);					// ACTIVE = 0, INACTIVE = 1
	dwPeriodValue = BF_PWM_PERIODn_MATT_SEL(1)											// select 24MHz main clock as source clock
				  | BF_PWM_PERIODn_CDIV(BV_PWM_PERIODn_CDIV__DIV_1)						// clock = mainclock / 1
				  | BF_PWM_PERIODn_INACTIVE_STATE(BV_PWM_PERIODn_INACTIVE_STATE__0)		// INACTIVE_STATE = 0 
		          | BF_PWM_PERIODn_ACTIVE_STATE(BV_PWM_PERIODn_ACTIVE_STATE__1)			// ACTIVE_STATE = 1
				  | BF_PWM_PERIODn_PERIOD(0x0001);										// number of divided clock = 0x0001 + 1 => 1
				  //| BF_PWM_PERIODn_PERIOD(0x000C);										// number of divided clock = 0x000C + 1 => 13

	HW_PWM_PERIODn_WR(7, dwPeriodValue);				// select 24MHz main clock onto PWM_7 pin
    // Enable PWM channel
	HW_PWM_CTRL_SET(BM_PWM_CTRL_PWM7_ENABLE);			//enable PWM_7
	// select PWM_7 onto chip pins 
    HW_PINCTRL_MUXSEL7_SET(3 << 20);	// switch to GPIO3_26 first -> muxmode = 2'b11
    HW_PINCTRL_MUXSEL7_CLR(1 << 21);    // then change to muxmode = 2'b01 -> PWM_7

	//
	// step3: config all the pins of the GPIO_SPI port
	//
	// select GPIO2_0(SPI_MOSI) as output with LOW
 //   DDKIomuxSetPinMux(GPIO_SPI_MOSI_PIN, DDK_IOMUX_MODE_GPIO);		// switch to GPIO2_0 -> muxmode = 2'b11
 //   DDKGpioEnableDataPin(GPIO_SPI_MOSI_PIN, 1);						// set GPIO2_0 output enable
	//DDKGpioWriteDataPin(GPIO_SPI_MOSI_PIN, 0);						// clear GPIO2_0 output value = 0 -> SPI_MOSI = 0;

	//DDKIomuxSetPadConfig(GPIO_SPI_MOSI_PIN, 
 //       DDK_IOMUX_PAD_DRIVE_8MA, 
 //       DDK_IOMUX_PAD_PULL_ENABLE,
 //       DDK_IOMUX_PAD_VOLTAGE_3V3);

	//// select GPIO2_1(SPI_MISO) as input with pullup
 //   DDKIomuxSetPinMux(GPIO_SPI_MISO_PIN, DDK_IOMUX_MODE_GPIO);		// switch to GPIO2_1 -> muxmode = 2'b11
	//DDKGpioEnableDataPin(GPIO_SPI_MISO_PIN, 0);						// set GPIO2_1 output disable
	//DDKGpioWriteDataPin(GPIO_SPI_MISO_PIN, 1);						// set GPIO2_1 with pull-up resistor

	//DDKIomuxSetPadConfig(GPIO_SPI_MISO_PIN, 
 //       DDK_IOMUX_PAD_DRIVE_8MA, 
 //       DDK_IOMUX_PAD_PULL_ENABLE,
 //       DDK_IOMUX_PAD_VOLTAGE_3V3);

	//// select GPIO2_2(SPI_SCLK) as output with HIGH
 //   DDKIomuxSetPinMux(GPIO_SPI_SCLK_PIN, DDK_IOMUX_MODE_GPIO);		// switch to GPIO2_2 -> muxmode = 2'b11
	//DDKGpioEnableDataPin(GPIO_SPI_SCLK_PIN, 1);						// set GPIO2_2 output enable
	//DDKGpioWriteDataPin(GPIO_SPI_SCLK_PIN, 1);						// set GPIO2_2 output value = 1 -> SPI_SCLK = 1;

	//DDKIomuxSetPadConfig(GPIO_SPI_SCLK_PIN, 
 //       DDK_IOMUX_PAD_DRIVE_8MA, 
 //       DDK_IOMUX_PAD_PULL_ENABLE,
 //       DDK_IOMUX_PAD_VOLTAGE_3V3);

	//// select GPIO0_17(SPI_CS0N) as output with HIGH
 //   DDKIomuxSetPinMux(GPIO_SPI_CS0N_PIN, DDK_IOMUX_MODE_GPIO);		// switch to GPIO0_17 -> muxmode = 2'b11
	//DDKGpioEnableDataPin(GPIO_SPI_CS0N_PIN, 1);						// set GPIO0_17 output enable
	//DDKGpioWriteDataPin(GPIO_SPI_CS0N_PIN, 1);						// set GPIO0_17 output value = 1 -> SPI_CS0N = 1;

	//DDKIomuxSetPadConfig(GPIO_SPI_CS0N_PIN, 
 //       DDK_IOMUX_PAD_DRIVE_8MA, 
 //       DDK_IOMUX_PAD_PULL_ENABLE,
 //       DDK_IOMUX_PAD_VOLTAGE_3V3);

	//// select GPIO0_21(SPI_CS1N) as output with HIGH
 //   DDKIomuxSetPinMux(GPIO_SPI_CS1N_PIN, DDK_IOMUX_MODE_GPIO);		// switch to GPIO0_21 -> muxmode = 2'b11
	//DDKGpioEnableDataPin(GPIO_SPI_CS1N_PIN, 1);						// set GPIO0_17 output enable
	//DDKGpioWriteDataPin(GPIO_SPI_CS1N_PIN, 1);						// set GPIO0_17 output value = 1 -> SPI_CS1N = 1;

	//DDKIomuxSetPadConfig(GPIO_SPI_CS1N_PIN, 
 //       DDK_IOMUX_PAD_DRIVE_8MA, 
 //       DDK_IOMUX_PAD_PULL_ENABLE,
 //       DDK_IOMUX_PAD_VOLTAGE_3V3);

	//// select GPIO0_28(SPI_CS2N) as output with HIGH
 //   DDKIomuxSetPinMux(GPIO_SPI_CS2N_PIN, DDK_IOMUX_MODE_GPIO);		// switch to GPIO0_21 -> muxmode = 2'b11
	//DDKGpioEnableDataPin(GPIO_SPI_CS2N_PIN, 1);						// set GPIO0_17 output enable
	//DDKGpioWriteDataPin(GPIO_SPI_CS2N_PIN, 1);						// set GPIO0_17 output value = 1 -> SPI_CS2N = 1;

	//DDKIomuxSetPadConfig(GPIO_SPI_CS2N_PIN, 
 //       DDK_IOMUX_PAD_DRIVE_8MA, 
 //       DDK_IOMUX_PAD_PULL_ENABLE,
 //       DDK_IOMUX_PAD_VOLTAGE_3V3);

	// select GPIO2_0(SPI_MOSI) as output with LOW
	HW_PINCTRL_MUXSEL4_SET(3 << 0);		// switch to GPIO2_0 -> muxmode = 2'b11	
	HW_PINCTRL_DOE2_SET(1 << 0);		// set GPIO2_0 output enable
	HW_PINCTRL_DOUT2_CLR(1 << 0);		// clear GPIO2_0 output value = 0 -> SPI_MOSI = 0;

	// select GPIO2_1(SPI_MISO) as input with pullup
	HW_PINCTRL_MUXSEL4_SET(3 << 2);		// switch to GPIO2_1 -> muxmode = 2'b11
	HW_PINCTRL_DOE2_CLR(1 << 1);		// set GPIO2_1 output disable
	HW_PINCTRL_PULL2_SET(1 << 1);		// set GPIO2_1 with pull-up resistor

	// select GPIO2_2(SPI_SCLK) as output with HIGH
	HW_PINCTRL_MUXSEL4_SET(3 << 4);		// switch to GPIO2_2 -> muxmode = 2'b11
	HW_PINCTRL_DOE2_SET(1 << 2);		// set GPIO2_2 output enable
	HW_PINCTRL_DOUT2_SET(1 << 2);		// set GPIO2_2 output value = 1 -> SPI_SCLK = 1;

	// select GPIO0_17(SPI_CS0N) as output with HIGH
	// select GPIO0_21(SPI_CS1N) as output with HIGH
	// select GPIO0_28(SPI_CS2N) as output with HIGH
	HW_PINCTRL_MUXSEL1_SET(0x0300C0C0);	// switch to GPIO0_17/21/28 -> muxmode = 2'b11
	HW_PINCTRL_DOE0_SET(0x10220000);	// set GPIO0_17/21/28 outputs enable
	HW_PINCTRL_DOUT0_SET(0x10220000);	// set GPIO0_17/21/28 outputs value = 1
}

//------------------------------------------------------------------------------
// The basic routines for SPI timing
//------------------------------------------------------------------------------
void OALGpioSpiEnable(DWORD dwCSNum)
{
	switch(dwCSNum)
	{
	case 0:
		HW_PINCTRL_DOUT0_SET((1 << 28) | (1 << 21));	// GPIO0_21 = GPIO0_28 = 1
		HW_PINCTRL_DOUT0_CLR(1 << 17);		// GPIO0_17 = 0;
		//DDKGpioWriteDataPin(GPIO_SPI_CS2N_PIN, 1);		//SPI_CS2N = 1;
		//DDKGpioWriteDataPin(GPIO_SPI_CS1N_PIN, 1);		//SPI_CS1N = 1;
		//DDKGpioWriteDataPin(GPIO_SPI_CS0N_PIN, 0);		//SPI_CS0N = 0;
		break;

	case 1:
		HW_PINCTRL_DOUT0_SET((1 << 28) | (1 << 17));	// GPIO0_17 = GPIO0_28 = 1
		HW_PINCTRL_DOUT0_CLR(1 << 21);		// GPIO0_21 = 0;
		//DDKGpioWriteDataPin(GPIO_SPI_CS0N_PIN, 1);		//SPI_CS0N = 1;
		//DDKGpioWriteDataPin(GPIO_SPI_CS2N_PIN, 1);		//SPI_CS2N = 1;
		//DDKGpioWriteDataPin(GPIO_SPI_CS1N_PIN, 0);		//SPI_CS1N = 0;
		break;

	case 2:
		HW_PINCTRL_DOUT0_SET((1 << 21) | (1 << 17));	// GPIO0_17 = GPIO0_21 = 1
		HW_PINCTRL_DOUT0_CLR(1 << 28);		// GPIO0_28 = 0;
		//DDKGpioWriteDataPin(GPIO_SPI_CS1N_PIN, 1);		//SPI_CS1N = 1;
		//DDKGpioWriteDataPin(GPIO_SPI_CS0N_PIN, 1);		//SPI_CS0N = 1;
		//DDKGpioWriteDataPin(GPIO_SPI_CS2N_PIN, 0);		//SPI_CS2N = 0;
		break;
	}
}

void OALGpioSpiDisable(DWORD dwCSNum)
{
    UNREFERENCED_PARAMETER(dwCSNum);
	HW_PINCTRL_DOUT0_SET((1 << 28) | (1 << 21) | (1 << 17));		// set GPIO0_17/21/28 outputs value = 1

	//switch(dwCSNum)
	//{
	//case 0:
	//	DDKGpioWriteDataPin(GPIO_SPI_CS0N_PIN, 1);		//SPI_CS0N = 1;
	//	break;

	//case 1:
	//	DDKGpioWriteDataPin(GPIO_SPI_CS1N_PIN, 1);		//SPI_CS1N = 1;
	//	break;

	//case 2:
	//	DDKGpioWriteDataPin(GPIO_SPI_CS2N_PIN, 1);		//SPI_CS2N = 1;
	//	break;
	//}
}

void OALGpioSpiDout(DWORD dwLevel)
{
	if(!dwLevel)
	{
		//HW_PINCTRL_DOUT2_CLR(1 << 0);		// clear GPIO2_0 output value = 0 -> SPI_MOSI = 0;
		DDKGpioWriteDataPin(GPIO_SPI_MOSI_PIN, 0);
	}
	else
	{
		//HW_PINCTRL_DOUT2_SET(1 << 0);		// clear GPIO2_0 output value = 1 -> SPI_MOSI = 1;
		DDKGpioWriteDataPin(GPIO_SPI_MOSI_PIN, 1);
	}
}

void OALGpioSpiClock(DWORD dwLevel)
{
	if(!dwLevel)
	{
		HW_PINCTRL_DOUT2_CLR(1 << 2);		// clear GPIO2_2 output value = 0 -> SPI_SCLK = 0;
		//DDKGpioWriteDataPin(GPIO_SPI_SCLK_PIN, 0);
	}
	else
	{
		HW_PINCTRL_DOUT2_SET(1 << 2);		// clear GPIO2_2 output value = 1 -> SPI_SCLK = 1;
		//DDKGpioWriteDataPin(GPIO_SPI_SCLK_PIN, 1);
	}
}

DWORD OALGpioSpiDin(void)
{
	UINT32	state = 0;

	// get GPIO2_1's state
	DDKGpioReadDataPin(GPIO_SPI_MISO_PIN, &state);
	//if( (HW_PINCTRL_DIN2_RD()) & (1 << 1))
	//{
	//	state = 1;
	//}

	return state;
}


//------------------------------------------------------------------------------
// The basic routines for SPI read / write
//------------------------------------------------------------------------------
BYTE OALGpioSpiRead(DWORD dwCSNum, DWORD dwRegAddr)
{
	DWORD	i1;
	BYTE	ub1 = 0;

	//OALMSG(1, (L"OALGpioSpiRead::CS%dN, Addr=0x%x\r\n", dwCSNum, dwRegAddr));
	OALGpioSpiEnable(dwCSNum);

	//output register address, beginning from MSB(A7)
	for(i1 = 0; i1 < 8; i1++)
	{
		OALGpioSpiClock(0);		//SPI_SCLK = 0: data update
		
		if(dwRegAddr & 0x80)	
		{
			OALGpioSpiDout(1);
		}
		else
		{
			OALGpioSpiDout(0);
		}

		OALGpioSpiClock(1);		//SPI_SCLK = 1: rising-edge latch data into device
		dwRegAddr <<= 1;		//shift-left 1-bit
	}

	OALStall(1);				//delay 1us

	// read data from SPI device, beginning from MSB(D7)
	for(i1 = 0; i1 < 8; i1++)
	{
		OALGpioSpiClock(0);		//SPI_SCLK = 0: data update
		ub1 <<= 1;
		OALGpioSpiClock(1);		//SPI_SCLK = 1: rising-edge latch data from device
		if(OALGpioSpiDin())
		{
			ub1 |= 0x01;
		}
	}

	OALGpioSpiDisable(dwCSNum);

	return ub1;
}

void OALGpioSpiWrite(DWORD dwCSNum, DWORD dwRegAddr, BYTE ucByteData)
{
	DWORD	i1;
	DWORD	dwAddrData;

	//OALMSG(1, (L"OALGpioSpiWrite::CS%dN, Addr=0x%x, Data=0x%x\r\n", dwCSNum, dwRegAddr, ucByteData));

	// combine addr and data
	dwAddrData = (dwRegAddr << 8) | ucByteData;
	//OALMSG(1, (L"OALGpioSpiWrite::AddrData=0x%04x\r\n", dwAddrData));

	OALGpioSpiEnable(dwCSNum);

	//output register address, beginning from MSB(BIT15)
	for(i1 = 0; i1 < 16; i1++)
	{
		OALGpioSpiClock(0);		//SPI_SCLK = 0: data update
		
		if(dwAddrData & 0x8000)	
		{
			OALGpioSpiDout(1);
		}
		else
		{
			OALGpioSpiDout(0);
		}

		OALGpioSpiClock(1);		//SPI_SCLK = 1: rising-edge latch data into device
		dwAddrData <<= 1;		//shift-left 1-bit
	}

	OALGpioSpiDisable(dwCSNum);
}

BOOL OALSPI_Access(VOID* pInpBuffer, UINT32 inpSize)
{
	PSpiAccessInfo	pInfo;
	BOOL			bRet = FALSE;
	DWORD			i;

	if(!pInpBuffer || (inpSize != sizeof(SpiAccessInfo)))
	{
		OALMSG(1, (L"->OALSPI_Access::parameter invalid!\r\n"));
		return FALSE;
	}

	pInfo = (PSpiAccessInfo)pInpBuffer;
	switch(pInfo->dwAccessCode)
	{
	case SPI_ACCESS_CODE_READBYTE:
		pInfo->pDataBuf[0] = OALGpioSpiRead(pInfo->dwCSNum, pInfo->dwStartAddr);
		bRet = TRUE;
		break;

	case SPI_ACCESS_CODE_WRITEBYTE:
		OALGpioSpiWrite(pInfo->dwCSNum, pInfo->dwStartAddr, *pInfo->pDataBuf);
		bRet = TRUE;
		break;

	case SPI_ACCESS_CODE_READBLOCK:
		for(i = 0; i < pInfo->dwDataLength; i++)
		{
			pInfo->pDataBuf[i] = OALGpioSpiRead(pInfo->dwCSNum, (pInfo->dwStartAddr + i));
		}
		bRet = TRUE;
		break;

	case SPI_ACCESS_CODE_WRITEBLOCK:
		for(i = 0; i < pInfo->dwDataLength; i++)
		{
			OALGpioSpiWrite(pInfo->dwCSNum, (pInfo->dwStartAddr + i), pInfo->pDataBuf[i]);
		}
		bRet = TRUE;
		break;

	default:
		OALMSG(1, (L"->OALSPI_Access::unknown access code!\r\n"));
	}

	return bRet;
}
//#endif	//EM9280_GPIO_SPI