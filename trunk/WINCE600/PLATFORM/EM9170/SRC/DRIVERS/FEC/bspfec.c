//------------------------------------------------------------------------------
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//------------------------------------------------------------------------------
//
//  File:  bspfec.c
//
//  Implementation of FEC Driver
//
//  This file implements the bsp level code for FEC.
//
//-----------------------------------------------------------------------------
#include "bsp.h"


//-----------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Local Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
void BSPPhyEnable(BOOL bEnable);
void BSPPhyReset();

//------------------------------------------------------------------------------
//
// Function: BSPFECIomuxConfig
//
// This fuction will enable/disable the FEC GPIO according to the parameter
// "Enable".
//
// Parameters:
//        Enable
//            [in] TRUE for enabling the FEC GPIO, FALSE for disabling the
//                 FEC GPIO.
//
// Return Value:
//        TRUE for success, FALSE if failure.
//
//------------------------------------------------------------------------------

BOOL BSPFECIomuxConfig(BOOL bEnable )
{
	RETAILMSG(1, (TEXT("CS&ZHL::BSPFECIomuxConfig %d\r\n"), (DWORD)bEnable));

    if(bEnable)
    {
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_MDC,    DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_MDIO,   DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_TDATA0, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_TDATA1, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_TX_EN,  DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_RDATA0, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_RDATA1, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_RX_DV,  DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_TX_CLK, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    }

    BSPPhyEnable(bEnable);
    
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: void BSPPhyReset()
//
// This function resets the FEC
//
// Return Value:
//        TRUE for success, FALSE if failure.
//
//------------------------------------------------------------------------------
void BSPPhyReset()
{
#ifdef	EM9170
	RETAILMSG(1, (TEXT("BSPPhyReset:: Reset DM9161A\r\n")));
	//
	// CS&ZHL MAY-28-2011: using GPIO4_2 as RESET# to PHY(DM9161CEP) 
	//
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CS0, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
	// not PAD control register for PIN_CS0

#else	// -> iMX257PDK
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D12, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D12, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                     DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
#endif	//EM9170

	//
	// CS&ZHL JUN-1-2011: config FEC_RESET_GPIO_PIN as output (dir = 1) without interrupt
	//
	DDKGpioSetConfig(FEC_RESET_GPIO_PORT, FEC_RESET_GPIO_PIN, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE); 

    DDKGpioWriteDataPin(FEC_RESET_GPIO_PORT, FEC_RESET_GPIO_PIN, 0);
    Sleep(100);

    //Disable the FEC reset
    DDKGpioWriteDataPin(FEC_RESET_GPIO_PORT, FEC_RESET_GPIO_PIN, 1);
    Sleep(100);
}


//------------------------------------------------------------------------------
//
// Function: void BSPPhyEnable(BOOL bEnable)
//
// This function enables the FEC through a GPIO
//
// Return Value:
//        TRUE for success, FALSE if failure.
//
//------------------------------------------------------------------------------
void BSPPhyEnable(BOOL bEnable)
{
#ifdef	EM9170
	RETAILMSG(1, (TEXT("BSPPhyEnable::EM9170\r\n")));
	//
	// CS&ZHL MAY-28-2011: using GPIO4_3 as PWRDWN to PHY(DM9161CEP) 
	//
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CS1, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
	// not PAD control register for PIN_CS1

	//
	// CS&ZHL JUN-1-2011: config FEC_ENABLE_GPIO_PIN as output without interrupt
	//
	DDKGpioSetConfig(FEC_RESET_GPIO_PORT, FEC_ENABLE_GPIO_PIN, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE); 

	if(bEnable)
    {
        //Disable EM9161A POWERDOWN
        DDKGpioWriteDataPin(FEC_ENABLE_GPIO_PORT, FEC_ENABLE_GPIO_PIN, 0);
        Sleep(100);
        BSPPhyReset();
    }
    else
    {
        //Enable EM9161A POWERDOWN
        DDKGpioWriteDataPin(FEC_ENABLE_GPIO_PORT, FEC_ENABLE_GPIO_PIN, 1);
        Sleep(100);
    }
#else
	RETAILMSG(1, (TEXT("BSPPhyEnable::iMX257PDK\r\n")));
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_A17, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_A17, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                     DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    if(bEnable)
    {
        //Turn on fec ENABLE
        DDKGpioWriteDataPin(FEC_ENABLE_GPIO_PORT, FEC_ENABLE_GPIO_PIN, 1);
        Sleep(100);
        BSPPhyReset();
    }
    else
    {
        //Turn off fec DISABLE
        DDKGpioWriteDataPin(FEC_ENABLE_GPIO_PORT, FEC_ENABLE_GPIO_PIN, 0);
        Sleep(100);
    }
#endif	//EM9170
}

//------------------------------------------------------------------------------
//
// Function:  BSPFECPowerEnable
//
// This function handle FEC power suspend/resume
// Parameters:
//        Enable
//          [in] TRUE for power resume ,FALSE for power suspend 
//                 
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void BSPFECPowerEnable(BOOL bEnable)
{
#ifdef	EM9170
	//
	// CS&ZHL MAY-28-2011: using GPIO4_3 as PWRDWN to PHY(DM9161CEP) 
	//
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CS1, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
	// not PAD control register for PIN_CS1

	//
	// CS&ZHL JUN-1-2011: config FEC_ENABLE_GPIO_PIN as output without interrupt
	//
	DDKGpioSetConfig(FEC_RESET_GPIO_PORT, FEC_ENABLE_GPIO_PIN, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE); 

	if(bEnable)
    {
        //Disable EM9161A POWERDOWN -> turn on DM9161(A/C)
        DDKGpioWriteDataPin(FEC_ENABLE_GPIO_PORT, FEC_ENABLE_GPIO_PIN, 0);
        RETAILMSG(1, (TEXT("BSPFECPowerEnable: Turn On DM9161(C).\r\n")));
    }
    else
    {
        //Enable EM9161A POWERDOWN
        DDKGpioWriteDataPin(FEC_ENABLE_GPIO_PORT, FEC_ENABLE_GPIO_PIN, 1);
        RETAILMSG(1, (TEXT("BSPFECPowerEnable: Turn Off DM9161(C).\r\n")));
    }
#else	// ->iMX257PDK
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_A17, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_A17, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                     DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    if(bEnable)
    {
        //Turn on fec ENABLE
        DDKGpioWriteDataPin(FEC_ENABLE_GPIO_PORT, FEC_ENABLE_GPIO_PIN, 1);
    }
    else
    {
        //Turn off fec DISABLE
        DDKGpioWriteDataPin(FEC_ENABLE_GPIO_PORT, FEC_ENABLE_GPIO_PIN, 0);
    }
#endif	//EM9170
}

//------------------------------------------------------------------------------
//
// Function: BSPFECClockConfig
//
// This fuction will enable/disable the FEC Clock according to the parameter
// "Enable".
//
// Parameters:
//        Enable
//            [in] TRUE for enabling the FEC clock, FALSE for disabling the
//                 FEC clock.
//
// Return Value:
//        TRUE for success, FALSE if failure.
//
//------------------------------------------------------------------------------

BOOL BSPFECClockConfig( IN BOOL Enable )
{
    BOOL cfgState;
    
    if(Enable)
    {
        cfgState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_FEC, DDK_CLOCK_GATE_MODE_ENABLED);
        if(cfgState == FALSE)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPFECClockConfig: Enable FEC IPG clock failed.\r\n")));
            return FALSE;
        }
        
        cfgState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_FEC, DDK_CLOCK_GATE_MODE_ENABLED);
        if(cfgState == FALSE)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPFECClockConfig: Enable FEC AHB clock failed.\r\n")));
            return FALSE;
        }
    }
    else
    {
        cfgState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_FEC, DDK_CLOCK_GATE_MODE_DISABLED);
        if(cfgState == FALSE)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPFECClockConfig: Disable FEC IPG clock failed.\r\n")));
            return FALSE;
        }
        
        cfgState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_FEC, DDK_CLOCK_GATE_MODE_DISABLED);
        if(cfgState == FALSE)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPFECClockConfig: Disable FEC AHB clock failed.\r\n")));
            return FALSE;
        }
    }
   
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function:    DWORD BSPFECGetIrq()
//
// This fuction returns the FEC_IRQ
//
// Return Value:
//        fec irq number.
//
//------------------------------------------------------------------------------
DWORD BSPFECGetIrq()
{
    return IRQ_FEC;
}


//------------------------------------------------------------------------------
//
// Function:    DWORD BSPFECBaseAddress()
//
// This fuction returns the base address of the fec
//
// Return Value:
//        fec base address
//
//------------------------------------------------------------------------------
DWORD BSPFECBaseAddress()
{
    return CSP_BASE_REG_PA_FEC;
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
