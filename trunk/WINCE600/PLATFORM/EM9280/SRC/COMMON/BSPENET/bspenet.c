//-----------------------------------------------------------------------------
//
// Copyright (C) 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File: bspdnet.c
//
// BSP-specific support for Ethernet KITL.
//
//-----------------------------------------------------------------------------
#include "bsp.h"

extern PVOID pv_HWregCLKCTRL;

//------------------------------------------------------------------------------
// External Fuctions
VOID OALStall(UINT32 uSecs);


//------------------------------------------------------------------------------
//
//  Function:  OALKitlGetENETDmaBuffer(void)
//
//  This function is called by ENET debug functions to get
//  the physical memory for ENET DMA.
//
//  Parameters:
//      None.
//
//  Returns:
//      Physical address of the buffer used for ENET DMA
//
//------------------------------------------------------------------------------
DWORD OALKitlGetENETDmaBuffer(void)
{
    return  IMAGE_SHARE_ENET_RAM_PA_START;
}

//------------------------------------------------------------------------------
//
//  Function:  OALKitlENETBSPInit(void)
//
//  This function is called by ENET debug functions to initialize ENET 
//  board-specific I/O signals and clocking.
//
//  Parameters:
//      None.
//
//  Returns:
//      None
//
//------------------------------------------------------------------------------
void OALKitlENETBSPInit(void)
{
    // Config PinMux for ENET0
    DDKIomuxSetPinMux(DDK_IOMUX_ENET0_MDC, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_MDC, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_ENET0_MDIO, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_MDIO, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_ENET0_RX_EN, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_RX_EN, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_ENET0_RXD0, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_RXD0, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_ENET0_RXD1, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_RXD1, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_ENET0_TX_EN, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_TX_EN, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_ENET0_TXD0, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_TXD0, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_ENET0_TXD1, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_TXD1, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_CLKCTRL_ENET, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_CLKCTRL_ENET, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    // Power on PHY
    DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D3_1, DDK_IOMUX_MODE_GPIO);
    DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D3_1, 1);
    DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D3_1, 0);

    // Set up 50 MHz clock for PHY (This must be done before resetting PHY)
    // 
    // Power on ENET PLL
    BW_CLKCTRL_PLL2CTRL0_POWER(1);
    // Wait 10us
    OALStall(10);
    // Gate on ENET PLL
    BW_CLKCTRL_PLL2CTRL0_CLKGATE(0);
    // Enable ENET_CLK pad output
    BW_CLKCTRL_ENET_CLK_OUT_EN(1);

    // Reset PHY
    DDKIomuxSetPinMux(DDK_IOMUX_ENET0_RX_CLK, DDK_IOMUX_MODE_GPIO);
    DDKGpioEnableDataPin(DDK_IOMUX_ENET0_RX_CLK, 1);
    DDKGpioWriteDataPin(DDK_IOMUX_ENET0_RX_CLK, 0);
    OALStall(200);
    DDKGpioWriteDataPin(DDK_IOMUX_ENET0_RX_CLK, 1);
}

//------------------------------------------------------------------------------
//
// Function:    BSPENETClockInit()
//
//------------------------------------------------------------------------------
VOID BSPENETClockInit()
{
    // Gate on ENET clocks in CLKCTRL
    BW_CLKCTRL_ENET_SLEEP(0);
    BW_CLKCTRL_ENET_DISABLE(0);
}

//------------------------------------------------------------------------------
//
// Function:    BOOL BSPENETusesRMI()
//
//
// Return Value:
//        This fuction returns TRUE if RMII mode is used otherwise it returns FALSE
//
//------------------------------------------------------------------------------
BOOL BSPENETusesRMII()
{
    return TRUE;
}
