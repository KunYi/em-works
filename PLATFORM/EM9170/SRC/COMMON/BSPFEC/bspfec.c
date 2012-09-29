//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
////
// File: bspfec.c
//
// BSP-specific support for Ethernet KITL.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include "mc34704.h"
//------------------------------------------------------------------------------
// External Fuctions
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);

//VOID OALStall(UINT32 uSecs);
VOID OALGetRegister(BYTE addr, BYTE* content);

//------------------------------------------------------------------------------
// Local Functions
//

//------------------------------------------------------------------------------
//
//  Function:  OALStall
//
VOID OALStall(UINT32 uSecs)
{
    LARGE_INTEGER liStart, liEnd, liStallCount;
    static LARGE_INTEGER liFreq = {0, 0};
    static PCSP_EPIT_REG pEPIT = NULL;
    BSP_ARGS *pBspArgs;

    if (pEPIT == NULL)
    {
        // Map EPIT registers
        pEPIT = (PCSP_EPIT_REG) OALPAtoUA(CSP_BASE_REG_PA_EPIT1);
        if (pEPIT == NULL)
        {
            EdbgOutputDebugString("OALStall: EPIT mapping failed!\r\n");
            return;
        }

    }

    if (liFreq.QuadPart == 0)
    {
        pBspArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;
        
        switch(EXTREG32BF(&pEPIT->CR, EPIT_CR_CLKSRC))
        {
        case EPIT_CR_CLKSRC_CKIL:
            liFreq.QuadPart = BSP_CLK_CKIL_FREQ;
            break;

        case EPIT_CR_CLKSRC_IPGCLK:
            liFreq.QuadPart = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_EPIT];
            break;
        case EPIT_CR_CLKSRC_HIGHFREQ:
            liFreq.QuadPart = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB];
            break;
        }

        liFreq.QuadPart = liFreq.QuadPart / (EXTREG32BF(&pEPIT->CR, EPIT_CR_PRESCALAR) + 1);
    }


    liStallCount.QuadPart = ((liFreq.QuadPart * uSecs - 1) / 1000000) + 1;   // always round up
    
    liStart.QuadPart = INREG32(&pEPIT->CNT);
    do {
        liEnd.QuadPart = INREG32(&pEPIT->CNT);
    } while ( (liStart.QuadPart - liEnd.QuadPart) <= liStallCount.QuadPart);

}


//-----------------------------------------------------------------------------
//
//  Function: OEMEthGetSecs
//
//  This function returns the number of seconds that have passed since a 
//  certain fixed time.  This function handles time-outs while in polling 
//  mode. The origin of the count is unimportant as long as the count is 
//  incremental.
//
//  Parameters:
//      None.
//
// Returns:
//      Count of seconds that have passed since a certain fixed time.
//
//-----------------------------------------------------------------------------
DWORD OEMEthGetSecs(void)
{
#if 1
    static LARGE_INTEGER liFreq = {0, 0};
    static PCSP_EPIT_REG pEPIT = NULL;
    BSP_ARGS *pBspArgs;
    DWORD dwCount;
    DWORD freqHz;
    
    if (pEPIT == NULL)
    {
        pEPIT = (PCSP_EPIT_REG) OALPAtoUA(CSP_BASE_REG_PA_EPIT1);
        if (pEPIT == NULL)
        {
            EdbgOutputDebugString("OEMEthGetSecs: EPIT mapping failed!\r\n");
            return 0;
        }
    }
    dwCount = 0 - INREG32(&pEPIT->CNT);
    // Calculate the timer frequency
    if (liFreq.QuadPart == 0)
    {
        pBspArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;
        
        switch(EXTREG32BF(&pEPIT->CR, EPIT_CR_CLKSRC))
        {
        case EPIT_CR_CLKSRC_CKIL:
            liFreq.QuadPart = BSP_CLK_CKIL_FREQ;
            break;

        case EPIT_CR_CLKSRC_IPGCLK:
            liFreq.QuadPart = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_EPIT];
            break;

        case EPIT_CR_CLKSRC_HIGHFREQ:
            liFreq.QuadPart = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB];
            break;
        }
        
        liFreq.QuadPart = liFreq.QuadPart / (EXTREG32BF(&pEPIT->CR, EPIT_CR_PRESCALAR) + 1);        
    }

    freqHz = liFreq.LowPart;
    if (freqHz)
    {
        return dwCount/freqHz;
    }
    else
    {
        return 0;
    }

#else
    DWORD dwCounter;
    //using Dry Ice Timer instead of RTC since there isn't an RTC in i.MX25
    volatile PCSP_DRYICE_REGS pDryIce = (PCSP_DRYICE_REGS)OALPAtoUA(CSP_BASE_REG_PA_DRYICE);

    //Should we lock the Time Counter Registers so that the counters are read-only?

    //If Time Counter Reg is secure-only register (if we read back all 0's) then system may need to set the NSA bit in the Control
    //register to 1. (Datasheet did not say Time ctr register can only be accessed by secure SW only so for now assume Time Ctr 
    //reg is non-secure.
    
    //Time Counter MSB is a seconds timer
    dwCounter = INREG32(&pDryIce->DTCMR);
    //RETAILMSG(1,(TEXT("OEMEthGetSecs : %d 0x%x\r\n"),dwCounter,INREG32(&pDryIce->DSR)));
    return dwCounter;
#endif
}


//------------------------------------------------------------------------------
//
//  Function:  OALKitlGetFECDmaBuffer(void)
//
//  This function is called by FEC debug functions to get
//  the physical memory for FEC DMA.
//
//  Parameters:
//      None.
//
//  Returns:
//      Physical address of the buffer used for FEC DMA
//
//------------------------------------------------------------------------------
DWORD OALKitlGetFECDmaBuffer(void)
{
    return IMAGE_SHARE_FEC_RAM_PA_START;
}

//------------------------------------------------------------------------------
//
//  Function:  FECPmicInit(void)
//
//  This function is called by FEC debug functions to  Power up the PHY
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE
//
//------------------------------------------------------------------------------
BOOL FECPmicInit(void)
{
    BOOL rc = TRUE;

	//
	// CS&ZHL JUN-2-2011: iMX257PDK has a MC34704 as power managment IC, not EM9170
	//
#ifdef		IMX257PDK_MC34704
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_I2C1, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, DDK_CLOCK_GATE_MODE_ENABLED);
    
    PmicOpen(); 
    PmicEnable(TRUE);

    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_I2C1, DDK_CLOCK_GATE_MODE_DISABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, DDK_CLOCK_GATE_MODE_DISABLED);
#endif		//IMX257PDK_MC34704

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALKitlFECBSPInit(void)
//
//  This function is called by FEC debug functions to initialize external PHY and FEC pins
//
//  Parameters:
//      None.
//
//  Returns:
//      None
//
//------------------------------------------------------------------------------

void OALKitlFECBSPInit(void)
{
    PCSP_IOMUX_REGS pIOMUX;
    PCSP_GPIO_REGS pGPIO4;
    PCSP_GPIO_REGS pGPIO3;			// CS&ZHL MAY-30-2011: config DM9161A required
    PCSP_GPIO_REGS pGPIO2;

    pIOMUX = (PCSP_IOMUX_REGS) OALPAtoUA(CSP_BASE_REG_PA_IOMUXC);
    if (pIOMUX == NULL)
    {
        OALMSG(TRUE, (L"OALKitlFECBSPInit: IOMUXC mapping failed!\r\n"));
        return;
    }

    pGPIO4 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO4);
    if (pGPIO4 == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALKitlFECBSPInit: GPIO4 null pointer!\r\n"));
        return;
    }

	// CS&ZHL MAY-30-2011: config DM9161A required
    pGPIO3 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO3);
    if (pGPIO3 == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALKitlFECBSPInit: GPIO3 null pointer!\r\n"));
        return;
    }

	pGPIO2 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO2);
    if (pGPIO2 == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALKitlFECBSPInit: GPIO4 null pointer!\r\n"));
        return;
    }

    // Power up the PHY
    FECPmicInit();

#ifdef	EM9170
    OALMSG(1, (L"EM9170 setup PHY.\r\n"));
	//
	// CS&ZHL MAY-28-2011: using GPIO4_3 as PWRDWN to PHY(DM9161CEP) 
	//
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_CS1, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
	// not PAD control register for PIN_CS1
    // Configure fec_pwrden (GPIO4_3) as an ouput    
    OUTREG32 (&pGPIO4->GDIR, (INREG32(&pGPIO4->GDIR) | (1 << 3)));
    // Clear GPIO4_3 -> disable power down 
    OUTREG32 (&pGPIO4->DR, (INREG32(&pGPIO4->DR) & ~(1 << 3)));

	//
	// CS&ZHL MAY-28-2011: config PHY as node -> GPIO2_5 output 0
	//
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_A19, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_A19, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_1V8);
    // Configure fec_pwrden (GPIO2_5) as an ouput    
    OUTREG32 (&pGPIO2->GDIR, (INREG32(&pGPIO2->GDIR) | (1 << 5)));
    // Clear GPIO4_3 -> disable power down 
    OUTREG32 (&pGPIO2->DR, (INREG32(&pGPIO2->DR) & ~(1 << 5)));

	//
	// CS&ZHL MAY-30-2011: config PHY normal operation instead of test mode -> GPIO3_12 output 0
	//
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_RX_DV, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_FEC_RX_DV, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    // Configure fec_pwrden (GPIO2_5) as an ouput    
    OUTREG32 (&pGPIO3->GDIR, (INREG32(&pGPIO3->GDIR) | (1 << 12)));
    // Clear GPIO4_3 -> disable power down 
    OUTREG32 (&pGPIO3->DR, (INREG32(&pGPIO3->DR) & ~(1 << 12)));

	//
	// CS&ZHL MAY-28-2011: using GPIO4_2 as RESET# to PHY(DM9161CEP) 
	//
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_CS0, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
	// not PAD control register for PIN_CS0
    // Configure fec_reset_b (GPIO4_2) as an ouput    
    OUTREG32 (&pGPIO4->GDIR, (INREG32(&pGPIO4->GDIR) | (1 << 2)));
    // Clear GPIO4_2 and wait     
    OUTREG32 (&pGPIO4->DR, (INREG32(&pGPIO4->DR) & ~(1 << 2)));
    OALStall(100000);		// 100ms is enough
    // Set GPIO4_2
    OUTREG32 (&pGPIO4->DR, (INREG32(&pGPIO4->DR) | (1 << 2)));

#else	// -> iMX257PDK
    OALMSG(1, (L"iMX257PDK setup PHY.\r\n"));
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_A17, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_A17, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    // Configure fec_enable (GPIO2_3) as an ouput    
    OUTREG32 (&pGPIO2->GDIR, (INREG32(&pGPIO2->GDIR) | (1 << 3)));
    // Set  GPIO2_3   (FEC_ENABLE = 1 => FEC_3V3 = EXT_3V3)
    OUTREG32 (&pGPIO2->DR, (INREG32(&pGPIO2->DR) | (1 << 3)));
    
    //Reset the phy
    // Configure GPIO4_8 as a GPIO
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_D12, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_D12, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                     DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    // Configure fec_reset_b (GPIO4_8) as an ouput    
    OUTREG32 (&pGPIO4->GDIR, (INREG32(&pGPIO4->GDIR) | (1 << 8)));
    // Clear GPIO4_8 and wait     
    OUTREG32 (&pGPIO4->DR, (INREG32(&pGPIO4->DR) & ~(1 << 8)));
    OALStall(500000);
    // Set GPIO4_8
    OUTREG32 (&pGPIO4->DR, (INREG32(&pGPIO4->DR) | (1 << 8)));
#endif	//EM9170

    // Configure the FEC IO
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_MDC,    DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_MDIO,   DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_TDATA0, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_TDATA1, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_TX_EN,  DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_RDATA0, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_RDATA1, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_RX_DV,  DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_TX_CLK, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    
    // Enable clock
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_FEC, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_FEC, DDK_CLOCK_GATE_MODE_ENABLED);        
    

    return ;
}


//------------------------------------------------------------------------------
//
// Function:    BOOL BSPFECusesRMI()
//
//
// Return Value:
//        This fuction returns TRUE if RMII mode is used otherwise it returns FALSE
//
//------------------------------------------------------------------------------
BOOL BSPFECusesRMII()
{
    return TRUE;
}
