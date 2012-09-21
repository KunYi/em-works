//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//
// Module Name:
//
//    sdhcbsp.cpp   
//
// Abstract:
//
//    Board-level SD implmentation.
//
// Notes:
//
///////////////////////////////////////////////////////////////////////////////

#include "bsp.h"
#include "sdhc.h"
#include "image_cfg.h"

// Define
#define BIT_CARD0_DETECT (0x1 << 9)
#define BIT_CARD1_DETECT (0x1 << 20)
#define _DUMP_SSP       (0)

// Local variables

PVOID pv_HWregPINCTRL = NULL;

//LQK:Jul-18-2012
static DWORD g_dwDetection;

// LQK Jul-19-2012: add EM9283 GPIO Pin Table
DDK_IOMUX_PIN g_EM9283_iMXGpioPin[32] =
{
	DDK_IOMUX_GPIO3_20,			// EM9283_GPIO0
	DDK_IOMUX_GPIO3_21,			// EM9283_GPIO1
	DDK_IOMUX_GPIO1_8,			// EM9283_GPIO2
	DDK_IOMUX_GPIO1_9,			// EM9283_GPIO3
	DDK_IOMUX_GPIO1_16,			// EM9283_GPIO4
	DDK_IOMUX_GPIO1_17,			// EM9283_GPIO5
	DDK_IOMUX_GPIO2_16,			// EM9283_GPIO6
	DDK_IOMUX_GPIO2_17,			// EM9283_GPIO7
	DDK_IOMUX_GPIO2_18,			// EM9283_GPIO8
	DDK_IOMUX_GPIO2_19,			// EM9283_GPIO9
	DDK_IOMUX_GPIO3_24,			// EM9283_GPIO10
	DDK_IOMUX_GPIO3_25,			// EM9283_GPIO11
	DDK_IOMUX_GPIO2_4,          // EM9283_GPIO12
	DDK_IOMUX_GPIO2_6,			// EM9283_GPIO13
	DDK_IOMUX_GPIO2_7,			// EM9283_GPIO14
	DDK_IOMUX_GPIO2_5,			// EM9283_GPIO15
	DDK_IOMUX_GPIO4_7,			// EM9283_GPIO16
	DDK_IOMUX_GPIO4_3,			// EM9283_GPIO17
	DDK_IOMUX_GPIO4_8,			// EM9283_GPIO18
	DDK_IOMUX_GPIO4_4,			// EM9283_GPIO19
	DDK_IOMUX_GPIO2_9,			// EM9283_GPIO20
	DDK_IOMUX_GPIO4_16,			// EM9283_GPIO21
	DDK_IOMUX_GPIO2_8,			// EM9283_GPIO22
	DDK_IOMUX_GPIO2_10,			// EM9283_GPIO23
	DDK_IOMUX_GPIO2_0,          // EM9283_GPIO24
	DDK_IOMUX_GPIO2_1,			// EM9283_GPIO25
	DDK_IOMUX_GPIO2_2,			// EM9283_GPIO26
	DDK_IOMUX_GPIO2_3,			// EM9283_GPIO27
	DDK_IOMUX_GPIO3_23,         // EM9283_GPIO28
	DDK_IOMUX_GPIO3_22,			// EM9283_GPIO29
	DDK_IOMUX_GPIO3_28,			// EM9283_GPIO30
	DDK_IOMUX_GPIO3_26,			// EM9283_GPIO31
};

static DDK_IOMUX_PIN DDK_SDPowerPin;

// External functions
extern void ProcessCardInsertion(void *pContext);
extern void ProcessCardRemoval(void *pContext);


VOID BSPConfigPinsToSDHCFunc(DWORD dwIndex)
{
    if (dwIndex == 0)
    {
        // CLK
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_SCK, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_SCK, 
                             DDK_IOMUX_PAD_DRIVE_12MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        // CMD
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_CMD, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_CMD, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
        
        // DATA0
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D0, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        // DATA1
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D1, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        // DATA2
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D2, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D2, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        // DATA3
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D3, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D3, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

#ifndef EM9283	//LQK:Jul-18-2012 data4 to sata7 as SPI interface in EM9283 
        // DATA4
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D4, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D4, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);   
                             
        // DATA5
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D5, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D5, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);                                
                                          
        // DATA6
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D6, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D6, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);   
                             
        // DATA7
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D7, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D7, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);     
#endif	//#ifndef EM9283
                                                                                                    
    }
    
    if(dwIndex == 1)
    {
        // CLK
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_SCK_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_SCK_0, 
                             DDK_IOMUX_PAD_DRIVE_12MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        // CMD
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_CMD_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_CMD_0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
        
        // DATA0
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D0_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_D0_0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        // DATA1
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D1, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_D1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        // DATA2
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D2, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_D2, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        // DATA3
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D3_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_D3_0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        // DATA4
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D4, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_D4, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3); 
                             
        // DATA5
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D5, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_D5, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);  
                             
        // DATA6
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D6, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_D6, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);  
                             
        // DATA7
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D7, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_D7, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);    
                                                                                                        
    }
}

VOID BSPConfigPinsToGPIOLowOutput(DWORD dwIndex)
{
    if (dwIndex == 0)
    {
        //CLK
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_SCK, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_SCK, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP0_SCK, 0);
        
        //CMD
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_CMD, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_CMD, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP0_CMD, 0);
        
        //DATA0
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D0, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_D0, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP0_D0, 0);
        
        //DATA1
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D1, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_D1, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP0_D1, 0);
        
        //DATA2
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D2, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_D2, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP0_D2, 0);
        
        //DATA3
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D3, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_D3, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP0_D3, 0);
#ifndef EM9283	//LQK:Jul-18-2012 data4 to sata7 as SPI interface in EM9283        
        //DATA4
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D4, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_D4, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP0_D4, 0);
        
        //DATA5
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D5, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_D5, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP0_D5, 0);
        
        //DATA6
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D6, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_D6, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP0_D6, 0);
        
        //DATA7
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D7, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_D7, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP0_D7, 0);
#endif	//#ifndef EM9283
    }
    if (dwIndex == 1)
    {
        //CLK
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_SCK_0, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_SCK_0, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_SCK_0, 0);
        
        //CMD
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_CMD_0, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_CMD_0, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_CMD_0, 0);
        
        //DATA0
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D0_0, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D0_0, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D0_0, 0);
        
        //DATA1
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D1, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D1, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D1, 0);
        
        //DATA2
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D2, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D2, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D2, 0);
        
        //DATA3
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D3_0, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D3_0, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D3_0, 0);
        
        //DATA4
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D4, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D4, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D4, 0);
        
        //DATA5
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D5, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D5, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D5, 0);
        
        //DATA6
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D6, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D6, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D6, 0);
        
        //DATA7
        DDKIomuxSetPinMux(DDK_IOMUX_SSP1_D7, DDK_IOMUX_MODE_GPIO);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP1_D7, 1);
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_D7, 0);
    }
}
   
///////////////////////////////////////////////////////////////////////////////
//  BSPSDHCIsCardPresent - check if the card present
//  Input:  
//    dwIndex: instanc index
//  Return: TRUE for present, FALSE for not present
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
BOOL BSPSDHCIsCardPresent(DWORD dwIndex)
{
    if (dwIndex == 0)
    {

#ifdef EM9283
		BOOL bResult;
		if(HW_PINCTRL_DIN2_RD() & BIT_CARD0_DETECT)
		{
			DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("NO SD card0.\r\n")));
			bResult =  FALSE;
		}
		else
		{
			DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("The SD card0 is present.\r\n")));
			bResult =  TRUE;
		}

		if( g_dwDetection )
		{
			bResult = !bResult;
		}

		return bResult;

#else
		// SSP1_DET (bank2 pin9)
        if(HW_PINCTRL_DIN2_RD() & BIT_CARD0_DETECT)
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("NO SD card0.\r\n")));
            return FALSE;
        }
        else
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("The SD card0 is present.\r\n")));
            return TRUE;
        }
#endif // #ifdef EM9283

    }
    else if(dwIndex == 1)
    {
        if(HW_PINCTRL_DIN0_RD() & BIT_CARD1_DETECT)
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("NO SD card1.\r\n")));
            return FALSE;
        }
        else
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("The SD card1 is present.\r\n")));
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}

///////////////////////////////////////////////////////////////////////////////
//  BSPSDHCIsWriteProtected - check if the card be protected
//  Input:  
//    dwIndex: instanc index
//  Return: TRUE for protected, FALSE for not protected
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
BOOL BSPSDHCIsWriteProtected(DWORD dwIndex)
{
    BOOL retVal = FALSE;

    if (dwIndex == 0)
    {
#ifdef EM9283		//lqk:Jul-18-2012 EM9283 not support write protection 
		retVal = FALSE;
#else
		DDKGpioReadDataPin(DDK_IOMUX_SSP1_SCK_1, (PUINT32) &retVal);
#endif
    }
    else if (dwIndex == 1)
    {
        DDKGpioReadDataPin(DDK_IOMUX_SSP3_CMD_0, (PUINT32) &retVal);
    }   
    return (retVal & 0x1);
}

///////////////////////////////////////////////////////////////////////////////
//  BSPGetCardDetectIRQ - get the card detect IRQ if the system support this function
//  Input:  
//    dwIndex: instanc index 
//  Output: 
//    pIRQ : card detect IRQ
//  Return: TRUE for support card detect function, FALSE for not support
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
BOOL BSPGetCardDetectIRQ(DWORD dwIndex, PDWORD pIRQ)
{
    if (dwIndex == 0)
    {    
        *pIRQ = IRQ_GPIO2_PIN9;
        return TRUE;
    }
    else if(dwIndex == 1)
    {
        *pIRQ = IRQ_GPIO0_PIN20;
        return TRUE;
    }
    else
        return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//  BSPSDHCSetIOMUX - config pin IOMUX
//  Input:  
//    dwIndex: instanc index
//  Return: TRUE for success
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
static BOOL BSPSDHCSetIOMUX(DWORD dwIndex)
{
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("BSPSDHCSetIOMUX Begin\r\n")));
    
    BOOL bRet = TRUE;
    
    switch (dwIndex)
    {
        case 0:
            
            // Detect
            DDKIomuxSetPinMux(DDK_IOMUX_SSP0_CARD_DETECT, DDK_IOMUX_MODE_GPIO);
            DDKGpioEnableDataPin(DDK_IOMUX_SSP0_CARD_DETECT, 0);

#ifdef EM9283		//LQK:Jul-18-2012
			HW_PINCTRL_IRQLEVEL2_CLR(BIT_CARD0_DETECT);	// Eage detection
			if( g_dwDetection )
			{
				HW_PINCTRL_IRQPOL2_SET(BIT_CARD0_DETECT);	// High or rising edge
				RETAILMSG(1, (TEXT("Rising edge detection\r\n")));
			}
			else
			{
				HW_PINCTRL_IRQPOL2_CLR(BIT_CARD0_DETECT);	// low or falling edge
				RETAILMSG(1, (TEXT("Falling edge detection\r\n")));
			}
			
			HW_PINCTRL_IRQSTAT2_CLR(BIT_CARD0_DETECT);	
			HW_PINCTRL_PIN2IRQ2_SET(BIT_CARD0_DETECT);
			
#else
            HW_PINCTRL_IRQLEVEL2_CLR(BIT_CARD0_DETECT);	// Eage detection
            HW_PINCTRL_IRQPOL2_CLR(BIT_CARD0_DETECT);	// low or falling edge
            HW_PINCTRL_IRQSTAT2_CLR(BIT_CARD0_DETECT);	
            HW_PINCTRL_PIN2IRQ2_SET(BIT_CARD0_DETECT);
#endif	

            BSPConfigPinsToGPIOLowOutput(dwIndex);

#ifdef EM9283	//Lqk:Jul-18-2012 for SD card power management
			if( DDK_SDPowerPin >= 0 || DDK_SDPowerPin <= 31 )
			{
				DDKIomuxSetPinMux(DDK_SDPowerPin, DDK_IOMUX_MODE_GPIO);
				DDKGpioEnableDataPin(DDK_SDPowerPin, 1);
				DDKGpioWriteDataPin(DDK_SDPowerPin, 1);
			}
#else
			// WP on SSP1_SCK as GPIO input on EVK
			DDKIomuxSetPinMux(DDK_IOMUX_SSP1_SCK_1, DDK_IOMUX_MODE_GPIO);
			DDKGpioEnableDataPin(DDK_IOMUX_SSP1_SCK_1, 0);        // make it an input

			// PWM3
            DDKIomuxSetPinMux(DDK_IOMUX_PWM3_1, DDK_IOMUX_MODE_GPIO);
            DDKGpioEnableDataPin(DDK_IOMUX_PWM3_1, 1);
            DDKGpioWriteDataPin(DDK_IOMUX_PWM3_1, 1);
#endif	//ifdef EM9283

            // Delay 1 ms (=1000 us) for reset as well as power supply ramp up (controlled by PWM3 gpio line)
            Sleep(1);


            break;
     case 1:
            
            // Detect
            DDKIomuxSetPinMux(DDK_IOMUX_SSP1_CARD_DETECT, DDK_IOMUX_MODE_GPIO);
            DDKGpioEnableDataPin(DDK_IOMUX_SSP1_CARD_DETECT, 0);
            HW_PINCTRL_IRQLEVEL0_CLR(BIT_CARD1_DETECT);
            HW_PINCTRL_IRQPOL0_CLR(BIT_CARD1_DETECT);
            HW_PINCTRL_IRQSTAT0_CLR(BIT_CARD1_DETECT);
            HW_PINCTRL_PIN2IRQ0_SET(BIT_CARD1_DETECT);

            BSPConfigPinsToGPIOLowOutput(dwIndex);

            // WP
            DDKIomuxSetPinMux(DDK_IOMUX_SSP3_CMD_0, DDK_IOMUX_MODE_GPIO);
            DDKGpioEnableDataPin(DDK_IOMUX_SSP3_CMD_0, 0);        // make it an input
            
            // PWM4
            DDKIomuxSetPinMux(DDK_IOMUX_PWM4_1, DDK_IOMUX_MODE_GPIO);
            DDKGpioEnableDataPin(DDK_IOMUX_PWM4_1, 1);
            DDKGpioWriteDataPin(DDK_IOMUX_PWM4_1, 1);

            // Delay 1 ms (=1000 us) for reset as well as power supply ramp up (controlled by PWM3 gpio line)
            Sleep(1);

            break;

     default:
            ERRORMSG(1,  (TEXT("BSPSDHCSetIOMUX : Invalid SDHC index %d\r\n"), dwIndex));
            bRet = FALSE;
            goto _EXIT;
            break;
    }
 
_EXIT:  
    DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("BSPSDHCSetIOMUX End\r\n")));
    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//  BSPSDHCCardDetectThread - routine for card detect
//  Input:  
//    pHardwareContext: Hardware context with the card instance
//  Return: TRUE for success
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
BOOL BSPSDHCCardDetectThread(void *pHardwareContext)
{
    PSDHC_HARDWARE_CONTEXT pContext = (PSDHC_HARDWARE_CONTEXT)pHardwareContext;
    DWORD dwIndex = RunContext.dwSSPIndex;

    if (dwIndex == 0)
    {
        // Avoid debouncing effect
        Sleep(100);

        // Mask the interrupt
        HW_PINCTRL_IRQEN2_CLR(BIT_CARD0_DETECT);
        
        // Check card present or not
        if(BSPSDHCIsCardPresent(dwIndex))
        {
            //DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("ProcessCardInsertion\r\n")));
			RETAILMSG(1, (TEXT("-->SD Card Insertion\r\n")));
            
            //1. enable power
#ifdef EM9283 //	LQK:Jul-18-2012
			if( DDK_SDPowerPin >= 0 || DDK_SDPowerPin <= 31 )
			{
				DDKGpioWriteDataPin(DDK_SDPowerPin, 0);
			}
#else
            DDKGpioWriteDataPin(DDK_IOMUX_PWM3_1, 0);
#endif	//#ifdef EM9283
            
            Sleep(100);
            
            //2. config pins to SDHC pins
            BSPConfigPinsToSDHCFunc(dwIndex);
            
            ProcessCardInsertion(pContext);

            
#ifdef EM9283	//LQK:Jul-19-2012
			if( g_dwDetection )
				// Select falling edge to detect the insertion
				HW_PINCTRL_IRQPOL2_CLR(BIT_CARD0_DETECT);
			else
				// Select rising edge to detect the removal			
				HW_PINCTRL_IRQPOL2_SET(BIT_CARD0_DETECT);

#else		
			// Select rising edge to detect the removal
			HW_PINCTRL_IRQPOL2_SET(BIT_CARD0_DETECT);
#endif	//ifdef EM9283
        }
        else
        {
            //DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("ProcessCardRemoval\r\n")));
			RETAILMSG(1, (TEXT("<--SD Card Removal\r\n")));
            ProcessCardRemoval(pContext);

#ifdef EM9283	//LQK:Jul-19-2012
			if( g_dwDetection )
				// Select rising edge to detect the removal			
				HW_PINCTRL_IRQPOL2_SET(BIT_CARD0_DETECT);
			else
				// Select falling edge to detect the insertion
				HW_PINCTRL_IRQPOL2_CLR(BIT_CARD0_DETECT);
#else	
            // Select falling edge to detect the insertion
            HW_PINCTRL_IRQPOL2_CLR(BIT_CARD0_DETECT);
#endif		// ifdef EM9283
            
            // 1. Config sdhc pins to gpio low output
            BSPConfigPinsToGPIOLowOutput(dwIndex);
            
            // 2. shutdown power
#ifdef EM9283 //	LQK:Jul-18-2012
			if( DDK_SDPowerPin >= 0 || DDK_SDPowerPin <= 31 )
			{
				DDKGpioWriteDataPin(DDK_SDPowerPin, 1);
			}
#else
            DDKGpioWriteDataPin(DDK_IOMUX_PWM3_1, 1);
#endif	//#ifdef EM9283
        }

        // Clear the interrupt
        HW_PINCTRL_IRQSTAT2_CLR(BIT_CARD0_DETECT);

        return TRUE;
    }
    else if(dwIndex == 1)
    {
        // Avoid debouncing effect
        Sleep(100);

        // Mask the interrupt
        HW_PINCTRL_IRQEN0_CLR(BIT_CARD1_DETECT);
        
        // Check card present or not
        if(BSPSDHCIsCardPresent(dwIndex))
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("ProcessCardInsertion\r\n")));
            
            //1. enable power
            DDKGpioWriteDataPin(DDK_IOMUX_PWM4_1, 0);
            
            Sleep(100);

            //2. config pins to SDHC pins
            BSPConfigPinsToSDHCFunc(dwIndex);
            
            ProcessCardInsertion(pContext);

            // Select rising edge to detect the removal
            HW_PINCTRL_IRQPOL0_SET(BIT_CARD1_DETECT);
        }
        else 
        {
            DEBUGMSG(SDCARD_ZONE_INFO, (TEXT("ProcessCardRemoval\r\n")));

            ProcessCardRemoval(pContext);

            // Select falling edge to detect the insertion
            HW_PINCTRL_IRQPOL0_CLR(BIT_CARD1_DETECT);
            
            // 1. Config sdhc pins to gpio low output
            BSPConfigPinsToGPIOLowOutput(dwIndex);
            
            // 2. shutdown power
            DDKGpioWriteDataPin(DDK_IOMUX_PWM4_1, 1);
        } 

        // Clear the interrupt
        HW_PINCTRL_IRQSTAT0_CLR(BIT_CARD1_DETECT);

        return TRUE;
    }
    else
        return TRUE;
        
}

///////////////////////////////////////////////////////////////////////////////
//  BSPSDHCInit - BSP init code
//  Input:  
//    pHardwareContext: Hardware context with the card instance
//  Return: TRUE for success
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
BOOL BSPSDHCInit(void *pContext)
{
    BOOL rc = FALSE;
    DWORD dwIndex = ((PSDHC_HARDWARE_CONTEXT)pContext)->runContext.dwSSPIndex;
	//LQK:Jul-18-2012
	g_dwDetection = ((PSDHC_HARDWARE_CONTEXT)pContext)->runContext.dwDetection;
	DDK_SDPowerPin = g_EM9283_iMXGpioPin[((PSDHC_HARDWARE_CONTEXT)pContext)->runContext.dwPowerPin];

    PHYSICAL_ADDRESS PhysAddr;
    PhysAddr.QuadPart = CSP_BASE_REG_PA_PINCTRL;
    pv_HWregPINCTRL = (PVOID)MmMapIoSpace(PhysAddr, 0x2000, FALSE);
    
    if(pv_HWregPINCTRL == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("BSPSDHCSetIOMux:: pv_HWregPINCTRL NULL\r\n")));
        goto Exit;
    }

    //DEBUGMSG(SDCARD_ZONE_INIT, (TEXT("SSPPinIntializer Begin\r\n")));

    // SSP Pin Initialization
    if(!BSPSDHCSetIOMUX(dwIndex))
    {
        goto Exit;
    }
    
	rc = TRUE;
    
Exit:
    return rc;    
}

///////////////////////////////////////////////////////////////////////////////
//  BSPSDHCSysIntrInit - init the sys intr for the slot instance
//  Input:  
//    dwIndex: instanc index 
//  Output: 
//    pdwSysIntr : the sys intr
//  Return: TRUE for support card detect function, FALSE for not support
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
BOOL BSPSDHCSysIntrInit(DWORD dwIndex, PDWORD pdwSysIntr)
{
    BOOL fRet = FALSE;
    DWORD dwIrqs[4]; // in case we need to map multiple IRQs, including DMA and SSP to one SYSINTR
    DWORD dwIrqsSize = sizeof(dwIrqs);

    if (dwIndex == 0)
    {
        // Using -1 indicates we are not using the legacy calling convention
        dwIrqs[0] = (DWORD) -1;
        // Flags: in this case we want the existing sysintr if it
        // has already been allocated, and a new sysintr otherwise.
        dwIrqs[1] = OAL_INTR_TRANSLATE;

        // Now, let's add the controller IRQ
        dwIrqs[2] = IRQ_SSP0_ERROR;
        // then add the DMA IRQ
        dwIrqs[3] = IRQ_SSP0_DMA;
    }
    else if(dwIndex == 1)
    {
        // Using -1 indicates we are not using the legacy calling convention
        dwIrqs[0] = (DWORD) -1;
        // Flags: in this case we want the existing sysintr if it
        // has already been allocated, and a new sysintr otherwise.
        dwIrqs[1] = OAL_INTR_TRANSLATE;

        // Now, let's add the controller IRQ
        dwIrqs[2] = IRQ_SSP1_ERROR;
        // then add the DMA IRQ
        dwIrqs[3] = IRQ_SSP1_DMA;
    }
    else
        return FALSE;

    // convert the hardware IRQs into a logical SYSINTR value
    if (KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, dwIrqs, dwIrqsSize, pdwSysIntr, sizeof(DWORD), NULL))
    {
        fRet = TRUE;
    }

    return fRet;
}

///////////////////////////////////////////////////////////////////////////////
//  BSPSDHCDeinit - BSP Deinit code
//  Input:  
//    pHardwareContext: Hardware context with the card instance
//  Return: None
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
void BSPSDHCDeinit(void *pContext)
{
    UNREFERENCED_PARAMETER(pContext);
    if(pv_HWregPINCTRL != NULL) 
    {
        MmUnmapIoSpace((PVOID)pv_HWregPINCTRL, 0x2000);
        pv_HWregPINCTRL = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
//  BSPSDHCGetDMADescAddress - get the DMA description address
//  Input:  
//    dwIndex: instanc index 
//    dwDescSize: idescription size 
//  Return: address for the DMA description
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
UINT32 BSPSDHCGetDMADescAddress(DWORD dwIndex, DWORD dwDescSize)
{
    if (dwIndex > 3)
        return 0;
    return IMAGE_WINCE_SDMMC_IRAM_PA_START + (dwIndex)*dwDescSize;
}

BOOL BSPSDHCSupport8Bit(DWORD dwIndex)
{
    UNREFERENCED_PARAMETER(dwIndex);
    return TRUE;   
}

BOOL BSPSDHCSupportDDRMode(DWORD dwIndex)
{
    UNREFERENCED_PARAMETER(dwIndex);
    return TRUE;   
}

VOID BSPSDHCEnableDDRMode(void *pContext)
{
    RETAILMSG(TRUE, (_T("ddr mode enable\r\n")));
    ((PSDHC_HARDWARE_CONTEXT)pContext)->runContext.bMMCDDRMode = TRUE;
    
}

VOID BSPSDHCDisableDDRMode(void *pContext)
{
    //RETAILMSG(TRUE, (_T("ddr mode disable\r\n")));
    ((PSDHC_HARDWARE_CONTEXT)pContext)->runContext.bMMCDDRMode = FALSE;
}