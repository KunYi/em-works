//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: oal_startup.c
//
// iMX28  board initialization code.
//
//------------------------------------------------------------------------------
#include <bsp.h>

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregPOWER;
extern PVOID pv_HWregCLKCTRL;

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
//
// Function: OALInitSysCtrl
//
// Function is called by StartUp to set up system control module. Physical
// addresses are to be used here.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void OALInitCpuHclkClock(void)
{
    UINT32 dwRegFrac;
    UINT32 x;
    BOOL bLimit = FALSE;

    if((HW_POWER_5VCTRL.B.PWD_CHARGE_4P2 == 0) && ((HW_POWER_5VCTRL_RD() & BM_POWER_5VCTRL_CHARGE_4P2_ILIMIT) == 0x20000))
    {
        bLimit = TRUE;      
    }
    // let CPU sink the xtal clock
    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);

    // Turn on PLL
    HW_CLKCTRL_PLL0CTRL0_SET(BM_CLKCTRL_PLL0CTRL0_POWER);

    //Set VDDD to 1.525V
    HW_POWER_VDDDCTRL_WR(HW_POWER_VDDDCTRL_RD() | BM_POWER_VDDDCTRL_BO_OFFSET);

    HW_POWER_VDDDCTRL_WR((HW_POWER_VDDDCTRL_RD() & ~BM_POWER_VDDDCTRL_TRG) | VDDDVolt2Reg(1525));
    
    // need to wait more than 10 microsecond before the DC_OK is valid
    OALStall(100);
    
    // set ref.cpu 454MHZ
    HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATECPU);

    HW_CLKCTRL_FRAC0_WR((HW_CLKCTRL_FRAC0_RD() & ~BM_CLKCTRL_FRAC0_CPUFRAC) | \
                        BF_CLKCTRL_FRAC0_CPUFRAC(19));

    dwRegFrac = HW_CLKCTRL_FRAC0_RD() & BM_CLKCTRL_FRAC0_CPU_STABLE;
    for(; (HW_CLKCTRL_FRAC0_RD() ^ dwRegFrac) == 0; ) ;

    if(bLimit)
    {
        HW_CLKCTRL_CPU_WR((BF_CLKCTRL_CPU_DIV_CPU(15)            | \
                           BF_CLKCTRL_CPU_DIV_CPU_FRAC_EN(0)     | \
                           BF_CLKCTRL_CPU_INTERRUPT_WAIT(1)      | \
                           BF_CLKCTRL_CPU_DIV_XTAL(1)            | \
                           BF_CLKCTRL_CPU_DIV_XTAL_FRAC_EN(0)));
        
        OALMSG(1, (L"OALInitCpuHclkClock:Update CPU clock to 30MHz!\r\n"));
    }
    else
    {
        // Config CLK_CPU driver for 454 MHz.
        HW_CLKCTRL_CPU_WR((BF_CLKCTRL_CPU_DIV_CPU(1)             | \
                           BF_CLKCTRL_CPU_DIV_CPU_FRAC_EN(0)     | \
                           BF_CLKCTRL_CPU_INTERRUPT_WAIT(1)      | \
                           BF_CLKCTRL_CPU_DIV_XTAL(1)            | \
                           BF_CLKCTRL_CPU_DIV_XTAL_FRAC_EN(0)));

        OALMSG(1, (L"OALInitCpuHclkClock:Update CPU clock to 454MHz!\r\n"));
    }

    // Config CLK_HBUS as CPU/3 (160MHz)
    HW_CLKCTRL_HBUS_WR((BF_CLKCTRL_HBUS_DIV(3)                    | \
                        BF_CLKCTRL_HBUS_DIV_FRAC_EN(0)            | \
                        BF_CLKCTRL_HBUS_SLOW_DIV(5)               | \
                        BF_CLKCTRL_HBUS_ASM_ENABLE(1)             | \
                        BF_CLKCTRL_HBUS_CPU_INSTR_AS_ENABLE(1)    | \
                        BF_CLKCTRL_HBUS_CPU_DATA_AS_ENABLE(1)     | \
                        BF_CLKCTRL_HBUS_TRAFFIC_AS_ENABLE(1)      | \
                        BF_CLKCTRL_HBUS_TRAFFIC_JAM_AS_ENABLE(1)  | \
                        BF_CLKCTRL_HBUS_APBXDMA_AS_ENABLE(1)      | \
                        BF_CLKCTRL_HBUS_APBHDMA_AS_ENABLE(1)      | \
                        BF_CLKCTRL_HBUS_ASM_EMIPORT_AS_ENABLE(1)  | \
                        BF_CLKCTRL_HBUS_PXP_AS_ENABLE(1)          | \
                        BF_CLKCTRL_HBUS_DCP_AS_ENABLE(1)));
        
    HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);

    for(x=0; x++ != 0x1000;);

}

//------------------------------------------------------------------------------
//
// Function: OALInitIOClock
//
// Function is called by StartUp to set up the BSP board level clock
// frequencies. Physical addresses are to be used here.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void OALInitIOClock(void)
{
    PDDK_CLK_CONFIG pDdkClkConfig  = (PVOID) OALPAtoUA(IMAGE_WINCE_DDKCLK_RAM_PA_START);

    // let i/o sink clock from xtal
    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_GPMI | \
                          BM_CLKCTRL_CLKSEQ_BYPASS_SSP0 | \
                          BM_CLKCTRL_CLKSEQ_BYPASS_SSP1 | \
                          BM_CLKCTRL_CLKSEQ_BYPASS_SSP2 | \
                          BM_CLKCTRL_CLKSEQ_BYPASS_SSP3);

    // set ref_IO0 ref_IO1 clock to 480MHz
    HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATEIO0);
    HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATEIO1);
    HW_CLKCTRL_FRAC0_WR((HW_CLKCTRL_FRAC0_RD() & ~BM_CLKCTRL_FRAC0_IO0FRAC) | \
                        (HW_CLKCTRL_FRAC0_RD() & ~BM_CLKCTRL_FRAC0_IO1FRAC) | \
                        BF_CLKCTRL_FRAC0_IO0FRAC(18) | \
                        BF_CLKCTRL_FRAC0_IO1FRAC(18));

    HW_CLKCTRL_FRAC0_SET(BM_CLKCTRL_FRAC0_CLKGATEIO0);
    HW_CLKCTRL_FRAC0_SET(BM_CLKCTRL_FRAC0_CLKGATEIO1);

    pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO0] = 480000000;
    pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO1] = 480000000;

}

//------------------------------------------------------------------------------
//
// Function: OALInitPIXClock
//
// Function is called by StartUp to set up the BSP board level clock
// frequencies. Physical addresses are to be used here.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void OALInitPIXClock(void)
{
    PDDK_CLK_CONFIG pDdkClkConfig  = (PVOID) OALPAtoUA(IMAGE_WINCE_DDKCLK_RAM_PA_START);    

#ifdef	UUT
    // let Pix sink clock from xtal
    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF);

    // set ref_PIX to 432MHz
    HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEPIX);
    HW_CLKCTRL_FRAC1_WR((HW_CLKCTRL_FRAC1_RD() & ~BM_CLKCTRL_FRAC1_PIXFRAC) | \
                       BF_CLKCTRL_FRAC1_PIXFRAC(20));   

    HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEPIX);

    pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_PIX] = 432000000;    

#else	//-> boot from Eboot
	DWORD			dwPixFrac;
	DWORD			dwRefPixInKHz;
	DWORD			dwEbootLCDIFDiv;
	DWORD			dwEbootPixClkInKHz;
	BOOL			bclkgate = FALSE;

	//
	// CS&ZHL JULY-2-2012: PixClk had been setup with ref_pix(=480MHz on power-up) in Eboot
	//                     it's required to keep PixClock unchanged to have a stable splash on screen during loading NK
	//
    // let Pix sink clock from ref_pix
	HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF);

	// get Eboot settings
	dwPixFrac = HW_CLKCTRL_FRAC1_RD() & BM_CLKCTRL_FRAC1_PIXFRAC;
	dwRefPixInKHz = 480000 * 18 / dwPixFrac;
	RETAILMSG(1,(TEXT("OALInitPIXClock: default Ref_PIX = %d.%dMHz\r\n"), (dwRefPixInKHz / 1000), (dwRefPixInKHz % 1000)));
	dwEbootLCDIFDiv = (HW_CLKCTRL_DIS_LCDIF.B.DIV) & 0x1FFF;
	dwEbootPixClkInKHz = dwRefPixInKHz / dwEbootLCDIFDiv;
	RETAILMSG(1,(TEXT("OALInitPIXClock: default PIX_CLK = %d.%dMHz\r\n"), (dwEbootPixClkInKHz / 1000), (dwEbootPixClkInKHz % 1000)));

    // let Pix sink clock from xtal
    //HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF);

    // set ref_PIX to 432MHz
    HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEPIX);					// ref_pix is off
    HW_CLKCTRL_FRAC1_WR((HW_CLKCTRL_FRAC1_RD() & ~BM_CLKCTRL_FRAC1_PIXFRAC) | \
                       BF_CLKCTRL_FRAC1_PIXFRAC(20));   

    HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEPIX);					//PIX fractional divider clock is enabled, MUST be enable!
    pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_PIX] = 432000000;    
	OALMSG(1, (L"OALInitPIXClock: Ref_PIX Update to 432MHz!\r\n"));

	// compute new divider, and new pixclock
	dwEbootLCDIFDiv = 432000 / dwEbootPixClkInKHz;
	dwEbootPixClkInKHz = 432000 / dwEbootLCDIFDiv;

    // Only change DIV when CLKGATE = 0
    if((HW_CLKCTRL_DIS_LCDIF_RD() & BM_CLKCTRL_DIS_LCDIF_CLKGATE) != 0)
    {
        HW_CLKCTRL_DIS_LCDIF_CLR(BM_CLKCTRL_DIS_LCDIF_CLKGATE); 
        bclkgate = TRUE;
    }  

    // Change divider
    // Always use integer divide for PIX clock
    HW_CLKCTRL_DIS_LCDIF_CLR(BM_CLKCTRL_DIS_LCDIF_DIV_FRAC_EN);
    // Set divider
    HW_CLKCTRL_DIS_LCDIF.B.DIV = dwEbootLCDIFDiv;
	RETAILMSG(1,(TEXT("OALInitPIXClock: New PIX_CLK = %d.%dMHz\r\n"), (dwEbootPixClkInKHz / 1000), (dwEbootPixClkInKHz % 1000)));

    //restore clock gating states
    if(bclkgate)
    {
        HW_CLKCTRL_DIS_LCDIF_SET(BM_CLKCTRL_DIS_LCDIF_CLKGATE);
    }
#endif	//UUT
}

//------------------------------------------------------------------------------
//
// Function: OALInitGpio
//
// The function is called by OALStartUp to initialise the GPIO pins to a known
// state. Physical addresses are to be used here.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void OALInitGpio(void)
{
}

//------------------------------------------------------------------------------
//
// Function: OALInitEsdramc
//
// This function initializes Enhanced SDRAM Controller for DDR.
// Phyiscal address is used only.
//
// Parameters:
//       None.
//
// Retruns:
//       None.
//
//------------------------------------------------------------------------------
void OALInitEsdramc(void)
{
}

//------------------------------------------------------------------------------
// CS&ZHL JUN-12-2012: 
// Function: OALInitHsadcClock
//
// Function is called by StartUp to set up the BSP board level clock
// frequencies. Physical addresses are to be used here.
//
// Parameters:
//       None.
//
// Retruns:
//       None.
//
//------------------------------------------------------------------------------
void OALInitHsadcClock(void)
{
    PDDK_CLK_CONFIG pDdkClkConfig  = (PVOID) OALPAtoUA(IMAGE_WINCE_DDKCLK_RAM_PA_START);    

    //OALMSG(1, (L"OALInitHsadcClock: set ref_HSADC to 480MHz\r\n"));
    // set ref_HSADC to 480MHz
    HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEHSADC);
    HW_CLKCTRL_FRAC1_WR((HW_CLKCTRL_FRAC1_RD() & ~BM_CLKCTRL_FRAC1_HSADCFRAC) | \
                       BF_CLKCTRL_FRAC1_HSADCFRAC(18));   

    HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEHSADC);

    pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_HSADC] = 480000000;    
}

//------------------------------------------------------------------------------
//
// Function: OALInitClock
//
// The function is called by OALStartUp to initialise the Clocks
// Physical addresses are to be used here.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void OALClockInit()
{
    OALInitCpuHclkClock();
    OALInitIOClock();
    OALInitPIXClock();
    OALInitEsdramc(); 
    OALInitGpio();

	// CS&ZHL JUN-12-2012: set ref_HSADC => 480MHz
#if	(defined EM9280 || defined EM9283)
	OALInitHsadcClock();
#endif	//EM9280 || EM9283
}


