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
#include "hw_spi.h"
#include "em9280_oal.h"

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregCLKCTRL;
extern PVOID pv_HWregPINCTRL;		// handler for pin mux operations
extern PVOID pv_HWRegPWM;			// handler for config PWM_7 output 12MHz as COMCLK
extern PVOID pv_HWregSSP0;			// handler for SSP0_SPI transfer
extern PVOID pv_HWregSSP1;
extern PVOID pv_HWregSSP2;
extern PVOID pv_HWregSSP3;

//------------------------------------------------------------------------------
// Defines
//#define	GPIO_SPI_MOSI_PIN			DDK_IOMUX_GPIO2_0
//#define	GPIO_SPI_MISO_PIN			DDK_IOMUX_GPIO2_1
//#define	GPIO_SPI_SCLK_PIN			DDK_IOMUX_GPIO2_2
//#define	GPIO_SPI_CS0N_PIN			DDK_IOMUX_GPIO0_17
//#define	GPIO_SPI_CS1N_PIN			DDK_IOMUX_GPIO0_21
//#define	GPIO_SPI_CS2N_PIN			DDK_IOMUX_GPIO0_28

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
//
// Function: OALSSP0SpiInit
//
// Function is called by PostInit to setup SSP0 pins for SPI
// SSP0_SCK / GPIO2_10 -> SPI_SCLK
// SSP0_CMD / GPIO2_8  -> SPI_MOSI
// SSP0_D0  / GPIO2_0  -> SPI_MISO
//
// GPMI_CE1N / SSP3_D3  / GPIO0_17 -> GPIO_SPI_CS0N -> UART7
// GPMI_RDY1 / SSP1_CMD / GPIO0_21 -> GPIO_SPI_CS1N -> UART8
// GPMI_RSTN / SSP3_CMD / GPIO0_28 -> GPIO_SPI_CS2N -> UART9
//
// PWM_7 / GPIO3_26	-> COMCLK = 12MHz
//
//------------------------------------------------------------------------------
void OALSSP0SpiInit(void)
{
	DWORD		dwValue;
	DWORD		dwIndex = 0;	// -> SSP0
    SSP_CTRL0	sControl0;
    SSP_CTRL1	sControl1;
	DWORD		dwCLOCK_DIVIDE;
	DWORD		dwCLOCK_RATE;

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
        OALMSG(1, (L"OALSSP0SpiInit:PWM_7 is NOT present in this chip!\r\n"));
	}

	//
	// step2: make PWM_7 output 12MHz => (24MHz main clock / 2)
	//
	HW_PWM_ACTIVEn_WR(7, 0x00010000);					// ACTIVE = 0, INACTIVE = 1
	dwValue = BF_PWM_PERIODn_MATT_SEL(1)											// select 24MHz main clock as source clock
		    | BF_PWM_PERIODn_CDIV(BV_PWM_PERIODn_CDIV__DIV_1)						// clock = mainclock / 1
		    | BF_PWM_PERIODn_INACTIVE_STATE(BV_PWM_PERIODn_INACTIVE_STATE__0)		// INACTIVE_STATE = 0 
            | BF_PWM_PERIODn_ACTIVE_STATE(BV_PWM_PERIODn_ACTIVE_STATE__1)			// ACTIVE_STATE = 1
		    | BF_PWM_PERIODn_PERIOD(0x0001);										// number of divided clock = 0x0001 + 1 => 2

	HW_PWM_PERIODn_WR(7, dwValue);						// select 12MHz clock onto PWM_7 pin
    // Enable PWM channel
	HW_PWM_CTRL_SET(BM_PWM_CTRL_PWM7_ENABLE);			//enable PWM_7
	// select PWM_7 onto chip pins 
    HW_PINCTRL_MUXSEL7_SET(3 << 20);	// switch to GPIO3_26 first -> muxmode = 2'b11
    HW_PINCTRL_MUXSEL7_CLR(1 << 21);    // then change to muxmode = 2'b01 -> PWM_7

	//
	// step3: config the pins of SSP0 in SPI mode
	//
	// select GPIO2_10 as SPI_SCLK
	// select GPIO2_8  as SPI_MOSI
	// select GPIO2_0  as SPI_MISO
	dwValue = (3 << 20)					// -> SSP0_SCK -> SPI_SCLK
		    | (3 << 16)					// -> SSP0_CMD -> SPI_MOSI
			| (3 <<  0);				// -> SSP0_D0  -> SPI_MISO
	HW_PINCTRL_MUXSEL4_CLR(dwValue);	// switch to SSP0 -> muxmode = 2'b00

	//Configure SSP pins data+clk+cmd for 8mA drive strength, 3 volts.
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D0,  DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);		//pull-up with 47K
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_CMD, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);		//pull-up with 10K
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_SCK, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

	// set GPIO2_1, GPIO2_2, GPIO2_9 as GPIO input mode -> interrupt input
	dwValue = (3 << (1 * 2))			// -> GPIO2_1 -> SPI_INT0N
		    | (3 << (9 * 2))			// -> GPIO2_9 -> SPI_INT1N
		    | (3 << (2 * 2));			// -> GPIO2_2 -> SPI_INT2N
	HW_PINCTRL_MUXSEL4_SET(dwValue);	// switch to GPIO -> muxmode = 2'b11
	dwValue = (3 << 1)					// -> GPIO2_1 -> SPI_INT0N
		    | (3 << 9)					// -> GPIO2_9 -> SPI_INT1N
		    | (3 << 2);					// -> GPIO2_2 -> SPI_INT2N
	HW_PINCTRL_DOE2_CLR(dwValue);		// GPIO2_1/9/2 outputs disable


	// select GPIO0_17(SPI_CS0N) as output with HIGH
	// select GPIO2_9 (SPI_CS1N) as output with HIGH
	// select GPIO0_28(SPI_CS2N) as output with HIGH
	dwValue = (3 << ((17 - 16) * 2))	// -> GPIO0_17 -> SPI_CS0N
		    | (3 << ((21 - 16) * 2))	// -> GPIO0_21 -> SPI_CS1N
		    | (3 << ((28 - 16) * 2));	// -> GPIO0_28 -> SPI_CS2N
	HW_PINCTRL_MUXSEL1_SET(dwValue);		// switch to GPIO0_17/21/28 -> muxmode = 2'b11

	//config drive strength of SPI_CSxN 
    DDKIomuxSetPadConfig(DDK_IOMUX_GPIO0_17, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_GPIO0_21, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_GPIO0_28, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

	dwValue = (1 << 17)					// -> GPIO0_17 -> SPI_CS0N
		    | (1 << 21)					// -> GPIO0_21 -> SPI_CS1N
		    | (1 << 28);				// -> GPIO0_28 -> SPI_CS2N
	HW_PINCTRL_DOUT0_SET(dwValue);		// set GPIO0_17/21/28 outputs value = 1
	HW_PINCTRL_DOE0_SET(dwValue);		// set GPIO0_17/21/28 outputs enable

	//
	// step4: config SSP0 to SPI mode with .....
	//
    //// Ungate the block
    //HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_CLKGATE);
    //// Reset the block    
    //HW_SSP_CTRL0_SET(dwIndex, BM_SSP_CTRL0_SFTRST);
    //// Release the Block from Reset and starts the clock
    //HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_SFTRST | BM_SSP_CTRL0_CLKGATE);

	// Clock Reset
    HW_CLKCTRL_SSP0_CLR(BM_CLKCTRL_SSP0_CLKGATE);
    while(HW_CLKCTRL_SSP0_RD() & BM_CLKCTRL_SSP0_CLKGATE);
    // SSP Reset
    HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_SFTRST);
    while(HW_SSP_CTRL0_RD(dwIndex) & BM_SSP_CTRL0_SFTRST);
	// ungate clock -> apply clock
    HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_CLKGATE);
    while(HW_SSP_CTRL0_RD(dwIndex) & BM_SSP_CTRL0_CLKGATE);


    // Configure SSP Control Register 0   
    sControl0.U = 0;                                                                                                      
                                                                                  
    sControl0.B.LOCK_CS      = FALSE;
    sControl0.B.IGNORE_CRC   = FALSE;
    sControl0.B.BUS_WIDTH    = 0;	       // 0: 1-bit bus, 1: 4 bit bus
    sControl0.B.WAIT_FOR_IRQ = FALSE;
    sControl0.B.LONG_RESP    = FALSE;
    sControl0.B.CHECK_RESP   = FALSE;
    sControl0.B.GET_RESP     = FALSE;
    sControl0.B.WAIT_FOR_CMD = FALSE;
    sControl0.B.DATA_XFER    = FALSE;		// Data Transfer Enable

	// Configure SSP Control Register 1
    sControl1.U = 0;

    sControl1.B.DMA_ENABLE        = FALSE;					// 0: dma disabled, 1: dma enabled
    sControl1.B.CEATA_CCS_ERR_EN  = FALSE;					// CEATA Unexpected CCS Error logic enable. 0: disabled, 1: enabled
    sControl1.B.SLAVE_OUT_DISABLE = FALSE;					// 0: SSP can drive MISO in slave mode, 1: SSP does not drive MISO
    sControl1.B.PHASE             = 1;						// supporting HT45B0F
    sControl1.B.POLARITY          = 1;						// supporting HT45B0F
    sControl1.B.WORD_LENGTH       = SSP_WORD_LENGTH_8BITS;
    sControl1.B.SLAVE_MODE        = FALSE;                  // 0: SSP is Master, 1: SSP is Slave
    sControl1.B.SSP_MODE          = SSP_MODE_SPI;

    // Write the SSP Control Register 0 and 1 values out to the interface
    HW_SSP_CTRL0_WR(dwIndex, sControl0.U);
    HW_SSP_CTRL1_WR(dwIndex, sControl1.U);
	OALMSG(1, (L"OALSSP0SpiInit: CTRL0 = 0x%X, CTRL1 =0x%X\r\n", sControl0.U, sControl1.U));

	//
	// step5: config SPI0_SCLK = 3MHz => (CLKI / 4) which is reuqired for HT45B0F
	//		  (SSP0CLK = REF_XTAL = 24MHz)
	//
    sControl1.U = HW_SSP_CTRL1_RD(dwIndex);

	dwCLOCK_DIVIDE = 2;
	dwCLOCK_RATE   = 3;
    
	dwValue = (DWORD)(0xFFFF << 16) | (dwCLOCK_DIVIDE << 8) | dwCLOCK_RATE;         

	OALMSG(1, (L"OALSSP0SpiInit: SPI_Timing = 0x%X, CLOCK_DIVIDE=%d, CLOCK_RATE=%d\r\n", dwValue, dwCLOCK_DIVIDE, dwCLOCK_RATE));
    HW_SSP_TIMING_WR(dwIndex, dwValue);
    HW_SSP_CTRL1_WR(dwIndex, sControl1.U);
}

void OALSSP0SpiReset(VOID)
{
	DWORD	dwIndex = 0;

    // Reset the block.

    // Prepare for soft-reset by making sure that SFTRST is not currently
    // asserted.
    HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond for SFTRST to deassert.
    HW_SSP_CTRL0_RD(dwIndex);
    HW_SSP_CTRL0_RD(dwIndex);
    
    // Also clear CLKGATE so we can wait for its assertion below.
    HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_CLKGATE);

    // Now soft-reset the hardware.
    HW_SSP_CTRL0_SET(dwIndex, BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond for SFTRST to deassert.
    HW_SSP_CTRL0_RD(dwIndex);
    HW_SSP_CTRL0_RD(dwIndex);

    // Deassert SFTRST.
    HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond .
    HW_SSP_CTRL0_RD(dwIndex);
    HW_SSP_CTRL0_RD(dwIndex);

    // Release the Block from Reset and starts the clock
    HW_SSP_CTRL0_CLR(dwIndex, BM_SSP_CTRL0_CLKGATE);

    // Wait at least a microsecond.
    HW_SSP_CTRL0_RD(dwIndex);
    HW_SSP_CTRL0_RD(dwIndex);
}

//------------------------------------------------------------------------------
// The basic routines for SPI timing
//------------------------------------------------------------------------------
void OALSSP0SpiEnable(DWORD dwCSNum)
{
	switch(dwCSNum)
	{
	case 0:
		HW_PINCTRL_DOUT0_SET((1 << 28) | (1 << 21));	// GPIO0_28 = GPIO0_21 = 1
		HW_PINCTRL_DOUT0_CLR(1 << 17);					// GPIO0_17 = 0;
		break;

	case 1:
		HW_PINCTRL_DOUT0_SET((1 << 17) | (1 << 28));	// GPIO0_17 = GPIO0_28 = 1
		HW_PINCTRL_DOUT0_CLR(1 << 21);					// GPIO0_21 = 0
		break;

	case 2:
		HW_PINCTRL_DOUT0_SET((1 << 21) | (1 << 17));	// GPIO0_21 = GPIO0_17 = 1
		HW_PINCTRL_DOUT0_CLR(1 << 28);					// GPIO0_28 = 0
		break;
	}
}

void OALSSP0SpiDisable(DWORD dwCSNum)
{
    UNREFERENCED_PARAMETER(dwCSNum);

	HW_PINCTRL_DOUT0_SET((1 << 28) | (1 << 21) | (1 << 17));		// set GPIO0_17/28 outputs value = 1
}

DWORD mxs_spi_cs(DWORD cs)
{
	return ((cs & 1) ? BM_SSP_CTRL0_WAIT_FOR_CMD : 0) |
	    ((cs & 2) ? BM_SSP_CTRL0_WAIT_FOR_IRQ : 0);
}


void mxs_spi_enable(unsigned int idx)
{
	//__raw_writel(BM_SSP_CTRL0_LOCK_CS, ss->regs + HW_SSP_CTRL0_SET);
	//__raw_writel(BM_SSP_CTRL0_IGNORE_CRC, ss->regs + HW_SSP_CTRL0_CLR);
	HW_SSP_CTRL0_SET(idx, BM_SSP_CTRL0_LOCK_CS);
	HW_SSP_CTRL0_CLR(idx, BM_SSP_CTRL0_IGNORE_CRC);
}

void mxs_spi_disable(unsigned int idx)
{
	//__raw_writel(BM_SSP_CTRL0_LOCK_CS, ss->regs + HW_SSP_CTRL0_CLR);
	//__raw_writel(BM_SSP_CTRL0_IGNORE_CRC, ss->regs + HW_SSP_CTRL0_SET);
	HW_SSP_CTRL0_CLR(idx, BM_SSP_CTRL0_LOCK_CS);
	HW_SSP_CTRL0_SET(idx, BM_SSP_CTRL0_IGNORE_CRC);
}

//static int mxs_spi_txrx_pio(struct mxs_spi *ss, int cs,
//			    unsigned char *buf, int len,
//			    int *first, int *last, int write)
int mxs_spi_txrx_pio(unsigned int idx, int cs, unsigned char *buf, int len, int write)
{
	int			count;
	static int	first = 1;

	if (first)
	{
		mxs_spi_enable(idx);
		first = 0;
	}

	//__raw_writel(mxs_spi_cs(cs), ss->regs + HW_SSP_CTRL0_SET);
	HW_SSP_CTRL0_CLR(idx, (BM_SSP_CTRL0_WAIT_FOR_IRQ | BM_SSP_CTRL0_WAIT_FOR_CMD));		//clear these two bits 
	HW_SSP_CTRL0_SET(idx, mxs_spi_cs(cs));												//set some bits if required

	while (len--) 
	{
		//if (*last && len == 0) {
		//	mxs_spi_disable(ss);
		//	*last = 0;
		//}

		//__raw_writel(BM_SSP_CTRL0_XFER_COUNT, ss->regs + HW_SSP_CTRL0_CLR);
		HW_SSP_CTRL0_CLR(idx, BM_SSP_CTRL0_XFER_COUNT);
		//__raw_writel(1, ss->regs + HW_SSP_CTRL0_SET);	// byte-by-byte 
		HW_SSP_CTRL0_SET(idx, 1);						// transfer data byte by byte

		if (write)
		{
			//__raw_writel(BM_SSP_CTRL0_READ, ss->regs + HW_SSP_CTRL0_CLR);
			HW_SSP_CTRL0_CLR(idx, BM_SSP_CTRL0_READ);	// clear READ flag
		}
		else
		{
			//__raw_writel(BM_SSP_CTRL0_READ, ss->regs + HW_SSP_CTRL0_SET);
			HW_SSP_CTRL0_SET(idx, BM_SSP_CTRL0_READ);	// set READ flag
		}

		//--------------------------------------
		// Run! 
		//--------------------------------------
		//__raw_writel(BM_SSP_CTRL0_RUN, ss->regs + HW_SSP_CTRL0_SET);
		HW_SSP_CTRL0_SET(idx, BM_SSP_CTRL0_RUN);
		count = 10000;
		//while (((__raw_readl(ss->regs + HW_SSP_CTRL0) & BM_SSP_CTRL0_RUN) == 0) && count--)
		while (((HW_SSP_CTRL0_RD(idx) & BM_SSP_CTRL0_RUN) == 0) && count--)
		{
			continue;
		}
		if (count <= 0) 
		{
			//printk(KERN_ERR "%c: timeout on line %s:%d\n", write ? 'W' : 'C', __func__, __LINE__);
			OALMSG(1, (L"mxs_spi_txrx_pio-%c: timeout on waiting CTRL0_RUN set\r\n", write ? 'W' : 'R'));
			break;
		}

		if (write)
		{
			//__raw_writel(*buf, ss->regs + HW_SSP_DATA);
			HW_SSP_DATA_WR(idx, *buf);
		}

		// Set TRANSFER 
		//__raw_writel(BM_SSP_CTRL0_DATA_XFER, ss->regs + HW_SSP_CTRL0_SET);
		HW_SSP_CTRL0_SET(idx, BM_SSP_CTRL0_DATA_XFER);

		if (!write) 
		{
			count = 10000;
			//while (count-- && (__raw_readl(ss->regs + HW_SSP_STATUS) & BM_SSP_STATUS_FIFO_EMPTY))
			while (count-- && (HW_SSP_STATUS_RD(idx) & BM_SSP_STATUS_FIFO_EMPTY))
			{
				continue;
			}
			if (count <= 0) 
			{
				//printk(KERN_ERR "%c: timeout on line %s:%d\n", write ? 'W' : 'C', __func__, __LINE__);
				OALMSG(1, (L"mxs_spi_txrx_pio-%c: timeout on waiting FIFO_EMPTY\r\n", write ? 'W' : 'R'));
				break;
			}
			//*buf = (__raw_readl(ss->regs + HW_SSP_DATA) & 0xFF);
			*buf = (UCHAR)(HW_SSP_DATA_RD(idx) & 0xFF);
		}

		count = 10000;
		//while ((__raw_readl(ss->regs + HW_SSP_CTRL0) & BM_SSP_CTRL0_RUN) && count--)
		while ((HW_SSP_CTRL0_RD(idx) & BM_SSP_CTRL0_RUN) && count--)
		{
			continue;
		}
		if (count <= 0) 
		{
			//printk(KERN_ERR "%c: timeout on line %s:%d\n", write ? 'W' : 'C', __func__, __LINE__);
			OALMSG(1, (L"mxs_spi_txrx_pio-%c: timeout on waiting CRTL0_RUN cleared\r\n", write ? 'W' : 'R'));
			break;
		}

		// advance to the next byte
		buf++;
	}

	//return len < 0 ? 0 : -ETIMEDOUT;
	if(len > 0)
	{
		return -12;		// timeout error
	}

	return 0;
}

//------------------------------------------------------------------------------
// The basic routines for SPI read / write
//------------------------------------------------------------------------------
BYTE OALSSP0SpiRead(DWORD dwCSNum, DWORD dwRegAddr)
{
	int		i1;
	DWORD	dwIndex = 0;
	UCHAR	uBuf[4];

	uBuf[0] = (UCHAR)dwRegAddr;
	//OALMSG(1, (L"->OALSSP0SpiRead: CS%dN, Addr=0x%02x\r\n", dwCSNum, uBuf[0]));

	// assert SPI_CSxN
	OALSSP0SpiEnable(dwCSNum);
	
	//int mxs_spi_txrx_pio(unsigned int idx, int cs, unsigned char *buf, int len, int write)
	//write address
	//i1 = mxs_spi_txrx_pio(dwIndex, dwCSNum, uBuf, 1, 1);
	i1 = mxs_spi_txrx_pio(dwIndex, 0, uBuf, 1, 1);
	//read data
	//i1 = mxs_spi_txrx_pio(dwIndex, dwCSNum, uBuf, 1, 0);
	i1 = mxs_spi_txrx_pio(dwIndex, 0, uBuf, 1, 0);

	// de-assert SPI_CSxN
	OALSSP0SpiDisable(dwCSNum);
	//OALMSG(1, (L"<-OALSSP0SpiRead: %d\r\n", i1));

	return uBuf[0];
}

void OALSSP0SpiWrite(DWORD dwCSNum, DWORD dwRegAddr, BYTE ucByteData)
{
	int		i1;
	DWORD	dwIndex = 0;
	UCHAR	uBuf[4];

	uBuf[0] = (UCHAR)dwRegAddr;
	uBuf[1] = (UCHAR)ucByteData;
	//OALMSG(1, (L"->OALSSP0SpiWrite: CS%dN, Addr=0x%02x, Data=0x%02x\r\n", dwCSNum, uBuf[0], uBuf[1]));

	// assert SPI_CSxN
	OALSSP0SpiEnable(dwCSNum);
	
	//int mxs_spi_txrx_pio(unsigned int idx, int cs, unsigned char *buf, int len, int write)
	//i1 = mxs_spi_txrx_pio(dwIndex, dwCSNum, uBuf, 2, 1);
	i1 = mxs_spi_txrx_pio(dwIndex, 0, uBuf, 2, 1);

	// de-assert SPI_CSxN
	OALSSP0SpiDisable(dwCSNum);
	//OALMSG(1, (L"<-OALSSP0SpiWrite: %d\r\n", i1));
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
		pInfo->pDataBuf[0] = OALSSP0SpiRead(pInfo->dwCSNum, pInfo->dwStartAddr);
		bRet = TRUE;
		break;

	case SPI_ACCESS_CODE_WRITEBYTE:
		OALSSP0SpiWrite(pInfo->dwCSNum, pInfo->dwStartAddr, *pInfo->pDataBuf);
		bRet = TRUE;
		break;

	case SPI_ACCESS_CODE_READBLOCK:
		for(i = 0; i < pInfo->dwDataLength; i++)
		{
			pInfo->pDataBuf[i] = OALSSP0SpiRead(pInfo->dwCSNum, (pInfo->dwStartAddr + i));
		}
		bRet = TRUE;
		break;

	case SPI_ACCESS_CODE_WRITEBLOCK:
		for(i = 0; i < pInfo->dwDataLength; i++)
		{
			OALSSP0SpiWrite(pInfo->dwCSNum, (pInfo->dwStartAddr + i), pInfo->pDataBuf[i]);
		}
		bRet = TRUE;
		break;

	default:
		OALMSG(1, (L"->OALSPI_Access::unknown access code!\r\n"));
	}

	return bRet;
}

/*
	// setup SPI bus for writing
	HW_SSP_XFER_SIZE_WR(dwIndex, 1);					// 8-bit per word in case of HT45B0F

    //// set client SPI bus configuration based
    //HW_SSP_CTRL0_WR(dwIndex, 0x09000001);

	for(i1 = 0; i1 < 2; i1++)
	{
		//start exchange
		HW_SSP_CTRL0_SET(dwIndex, BM_SSP_CTRL0_RUN);

		//wait FIFO is NOT full
		//while((HW_SSP_STATUS_RD(dwIndex) & BM_SSP_STATUS_FIFO_FULL));
		
		//fill data into FIFO
		HW_SSP_DATA_WR(dwIndex, dwTxBuf[i1]);
		//if((i1%2) == 0)
		//{
		//	dwTmp = (dwTxBuf[i1+1] << 8) | dwTxBuf[i1];
		//}
		//HW_SSP_DATA_WR(dwIndex, dwTmp);

		//wait unit SSP0 is NOT running
		while(HW_SSP_CTRL0_RD(dwIndex) & BM_SSP_CTRL0_RUN);

		// setup SPI bus for writing
		HW_SSP_XFER_SIZE_WR(dwIndex, 1);					// 8-bit per word in case of HT45B0F
	}

	//wait until all data sent out -> FIFO is empty
    //while(!(HW_SSP_STATUS_RD(dwIndex) & BM_SSP_STATUS_FIFO_EMPTY));

	//wait unit SSP0 is NOT running
	//while(HW_SSP_CTRL0_RD(dwIndex) & BM_SSP_CTRL0_RUN);
*/