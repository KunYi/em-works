//------------------------------------------------------------------------------
// Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//------------------------------------------------------------------------------
//
//  File:  bspenet.c
//
//  Implementation of ENET SWITCH Driver
//
//  This file implements the bsp level code for ENET driver.
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

#define MAC0 0
#define MAC1 1

//------------------------------------------------------------------------------
//
// Function: BSPENETIomuxConfig
//
// This fuction will enable/disable the ENET GPIO according to the parameter
// "Enable".
//
// Parameters:
//        index
//          [in]    Index specifying the ENET module.
//
//        Enable
//            [in] TRUE for enabling the ENET GPIO, FALSE for disabling the
//                 ENET GPIO.
//
// Return Value:
//        TRUE for success, FALSE if failure.
//
//------------------------------------------------------------------------------

BOOL BSPENETIomuxConfig(DWORD index,BOOL bEnable )
{    
   if(index==0)
   {
       if(bEnable)
       {
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
      
        }
    }
    else if(index==1)
    {   
        if(bEnable)
        {        
            DDKIomuxSetPinMux(DDK_IOMUX_ENET1_RX_EN, DDK_IOMUX_MODE_01);
            DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_RX_EN, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
           
            DDKIomuxSetPinMux(DDK_IOMUX_ENET1_RXD0, DDK_IOMUX_MODE_01);
            DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_RXD0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
           
            DDKIomuxSetPinMux(DDK_IOMUX_ENET1_RXD1, DDK_IOMUX_MODE_01);
            DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_RXD1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
           
            DDKIomuxSetPinMux(DDK_IOMUX_ENET1_TX_EN, DDK_IOMUX_MODE_01);
            DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_TX_EN, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
          
            DDKIomuxSetPinMux(DDK_IOMUX_ENET1_TXD0, DDK_IOMUX_MODE_01);
            DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_TXD0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
           
            DDKIomuxSetPinMux(DDK_IOMUX_ENET1_TXD1, DDK_IOMUX_MODE_01);
            DDKIomuxSetPadConfig(DDK_IOMUX_ENET0_TXD1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
        }
    }
    else
        RETAILMSG (1, (TEXT("%s error\r\n"), __WFUNCTION__));
        
    BSPPhyEnable(TRUE);
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: void BSPPhyReset()
//
// This function resets the ENET
//
// Return Value:
//          None.
//
//------------------------------------------------------------------------------
void BSPPhyReset()
{
    DDKIomuxSetPinMux(DDK_IOMUX_ENET0_RX_CLK, DDK_IOMUX_MODE_GPIO);
    DDKGpioEnableDataPin(DDK_IOMUX_ENET0_RX_CLK, 1);
    DDKGpioWriteDataPin(DDK_IOMUX_ENET0_RX_CLK, 0);
    Sleep(200);
    DDKGpioWriteDataPin(DDK_IOMUX_ENET0_RX_CLK, 1);

}

//------------------------------------------------------------------------------
//
// Function: void BSPPhyEnable(BOOL bEnable)
//
// This function enables the ENET through a GPIO
//        Enable
//          [in] TRUE for phy enable ,FALSE for disable 
// Return Value:
//          None.
//
//------------------------------------------------------------------------------
void BSPPhyEnable(BOOL bEnable)
{      
    if(bEnable)
    {
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D3_1, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D3_1, 1);
        // Disable transeiver by default
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D3_1, 0);
    }
    else
    {
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D3_1, DDK_IOMUX_MODE_GPIO);
        // Disable transeiver by default
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D3_1, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D3_1, 1);
    }
    
    BSPPhyReset();
}

//------------------------------------------------------------------------------
//
// Function:  BSPENETPowerEnable
//
// This function handle ENET power suspend/resume
// Parameters:
//        index
//          [in]    Index specifying the ENET module.
//        Enable
//          [in] TRUE for power resume ,FALSE for power suspend 
//                 
// Return Value:
//        None.
//
//------------------------------------------------------------------------------
void BSPENETPowerEnable(DWORD index,BOOL bEnable)
{
    if(index==MAC1)
        return;
    
    DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D3_1, DDK_IOMUX_MODE_GPIO);
    DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D3_1, 1);
    
    if(bEnable)
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D3_1, 0);
    else
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D3_1, 1);
}

//------------------------------------------------------------------------------
//
// Function: BSPENETClockConfig
//
// This fuction will enable/disable the ENET Clock according to the parameter
// "Enable".
//
// Parameters:
//      index
//          [in]    Index specifying the ENET module.
//        Enable
//           [in] TRUE for enabling the ENET clock, FALSE for disabling the
//                 ENET clock.
//
// Return Value:
//        TRUE for success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL BSPENETClockConfig(DWORD index, IN BOOL Enable )
{
  
    BOOL cfgState;

    if(index==MAC1)
        return TRUE;

    DDKClockWriteENetConfig(0x00200000|BM_CLKCTRL_ENET_CLK_OUT_EN);

    Sleep(100);
    
    if(Enable)
    {       
        cfgState = DDKClockSetGatingMode(DDK_CLOCK_GATE_ENET_CLK, DDK_CLOCK_GATE_MODE_DISABLED );
        if(cfgState == FALSE)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPENETClockConfig: Enable ENET AHB clock failed.\r\n")));
            return FALSE;
        }
    }
    else
    {
        cfgState = DDKClockSetGatingMode(DDK_CLOCK_GATE_ENET_CLK, DDK_CLOCK_GATE_MODE_ENABLEDD);
        if(cfgState == FALSE)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPENETClockConfig: Disable ENET IPG clock failed.\r\n")));
            return FALSE;
        }            
    }
    Sleep(100);
    
    DDKClockWriteENetConfig(0x00200000|BM_CLKCTRL_ENET_CLK_OUT_EN);
    
   return TRUE;
}

//------------------------------------------------------------------------------
//
// Function:    DWORD BSPENETGetIrq()
//
// This fuction returns the ENET_IRQ
//      index
//          [in]    Index specifying the ENET module.
//
// Return Value:
//        ENET irq number.
//
//------------------------------------------------------------------------------
DWORD BSPENETGetIrq(DWORD index)
{
    if(index==0)
        return IRQ_ENET_MAC0;
    else if(index==1)
        return IRQ_ENET_MAC1;
    else if(index==2)
        return IRQ_ENET_SWI;
    
    RETAILMSG (1, (TEXT("%s error\r\n"), __WFUNCTION__));
    return 0;
}

//------------------------------------------------------------------------------
//
// Function:   BSPENETBaseAddress()
//
// This fuction returns the base address of the ENET
//      index
//          [in]    Index specifying the ENET module.
//
// Return Value:
//        ENET base address
//
//------------------------------------------------------------------------------
DWORD BSPENETBaseAddress(DWORD index)
{
    if(index==0)
        return  CSP_BASE_REG_PA_ENET;
    else if(index==1)
        return (CSP_BASE_REG_PA_ENET+0x4000);
    else if(index==2)
        return (CSP_BASE_REG_PA_ENET+0x8000);
    else if(index==3)
        return (CSP_BASE_REG_PA_ENET+0xc000);

    RETAILMSG (1, (TEXT("%s error\r\n"), __WFUNCTION__));
    return 0;
}

//------------------------------------------------------------------------------
//
// Function:     BSPENETusesRMI
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

//------------------------------------------------------------------------------
//
// Function:     BSPENETusesKitlMode
//
//
// Return Value:
//        This fuction returns TRUE if kitl  mode is used otherwise it returns FALSE
//
//------------------------------------------------------------------------------

BOOL BSPENETusesKitlMode()
{
    PHYSICAL_ADDRESS phyAddr;
    BOOL mode  = TRUE;
    
    BSP_ARGS *pBspArgs = NULL;

    phyAddr.QuadPart = IMAGE_SHARE_ARGS_RAM_PA_START;
    pBspArgs = (BSP_ARGS *) MmMapIoSpace(phyAddr, sizeof(BSP_ARGS), FALSE);
    if (!pBspArgs)
    {
        ERRORMSG(TRUE, (TEXT("Error mapping BSP_ARGS.\r\n")));
        goto cleanUp;
    }
    
    if ((pBspArgs->kitl.devLoc.LogicalLoc == CSP_BASE_REG_PA_ENET) &&
        (pBspArgs->kitl.flags & OAL_KITL_FLAGS_ENABLED) &&
        (!(pBspArgs->kitl.flags & OAL_KITL_FLAGS_PASSIVE)))
    {
        RETAILMSG(TRUE, (TEXT("ENET already in use for KITL. Driver load failure!\r\n")));
        goto cleanUp;
    }

    mode  = FALSE;
    
cleanUp:
    if (pBspArgs) MmUnmapIoSpace(pBspArgs, sizeof(BSP_ARGS));
    
    return mode;
}

