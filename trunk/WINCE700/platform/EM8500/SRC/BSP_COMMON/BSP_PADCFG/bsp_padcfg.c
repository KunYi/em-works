// All rights reserved ADENEO EMBEDDED 2010
/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File:  bsp_padcfg.c
//
#include "bsp.h"
#include "am33x_clocks.h"
//#include "oal_padcfg.h"
#include "bsp_padcfg.h"


//------------------------------------------------------------------------------
//  Pad configuration
//------------------------------------------------------------------------------

static PAD_INFO g_allowedPadCfg[] = ALL_ALLOWED_PADS;

const PAD_INFO ADCTSCPads[]    =   {ADCTSC_PADS END_OF_PAD_ARRAY};

const PAD_INFO BKLPads[]    =   {BKL_PADS END_OF_PAD_ARRAY};

const PAD_INFO I2C0Pads[]   =   {I2C0_PADS END_OF_PAD_ARRAY};

const PAD_INFO I2C1Pads[]   =   {I2C1_PADS END_OF_PAD_ARRAY};

const PAD_INFO LCDCPads[]   =   {LCDC_PADS END_OF_PAD_ARRAY};

//const PAD_INFO MII1Pads[]   =   {MII1_PADS END_OF_PAD_ARRAY};

const PAD_INFO MCASP1Pads[] =   {MCASP1_PADS END_OF_PAD_ARRAY};

const PAD_INFO MMC0Pads[]   =   {MMC0_PADS END_OF_PAD_ARRAY};

const PAD_INFO MMC0_NoCD_Pads[]    =   {MMC0_NO_CD_PADS END_OF_PAD_ARRAY};

const PAD_INFO MMC1Pads[]   =   {MMC1_PADS END_OF_PAD_ARRAY};

const PAD_INFO MMC2Pads[]    =   {MMC2_PADS END_OF_PAD_ARRAY};

const PAD_INFO NANDPads[]   =   {NAND_PADS END_OF_PAD_ARRAY};

const PAD_INFO NORPads[]    =   {NOR_PADS  END_OF_PAD_ARRAY};

const PAD_INFO RGMIIMultiplePhyPads[] =   {RGMII1_PADS RGMII2_PADS END_OF_PAD_ARRAY};

const PAD_INFO RGMIISinglePhyPads[] =   {RGMII1_PADS END_OF_PAD_ARRAY};

const PAD_INFO SPI0Pads[]   =   {SPI0_PADS END_OF_PAD_ARRAY};

//const PAD_INFO SPI1Pads[]   =   {SPI1_PADS END_OF_PAD_ARRAY};

const PAD_INFO UART0Pads[]  =   {UART0_PADS END_OF_PAD_ARRAY};

//const PAD_INFO UART3Pads[]  =   {UART3_PADS END_OF_PAD_ARRAY};

const PAD_INFO USB0Pads[]    =   {USB0_PADS USB1_PADS END_OF_PAD_ARRAY};

const PAD_INFO KeypadPads[]    = {KEYPAD_PADS END_OF_PAD_ARRAY};
 

/*
 * Update the structure with the modules present in the general purpose
 * board and the profiles in which the modules are present.
 * If the module is physically present but if it is not available
 * in any of the profile, then do not update it.
 * For eg, nand is avialable only in the profiles 0 and 1, whereas
 * UART0  is available in all the profiles.
 */

const EVM_PIN_MUX GP_BOARD_PIN_MUX[] = {
    { ADCTSCPads,           AM_DEVICE_ADC_TSC,      PROFILE_0 | PROFILE_1 | PROFILE_2 | PROFILE_7,  DEV_ON_DGHTR_BRD},
    { BKLPads,              AM_DEVICE_BKL,          PROFILE_0 | PROFILE_1 | PROFILE_2 | PROFILE_7,  DEV_ON_DGHTR_BRD},
    { I2C0Pads,             AM_DEVICE_I2C0,         PROFILE_ALL,                                    DEV_ON_BASEBOARD},
    { I2C1Pads,             AM_DEVICE_I2C1,         PROFILE_ALL & ~PROFILE_2 & ~PROFILE_4,          DEV_ON_BASEBOARD},
    { LCDCPads,             AM_DEVICE_LCDC,         PROFILE_0 | PROFILE_1 | PROFILE_2 | PROFILE_7,  DEV_ON_DGHTR_BRD}, //although its separate LCD board, use BaseBoard as location; will use profile info to know if LCD is valid or not    
    { MCASP1Pads,           AM_DEVICE_MCASP1,       PROFILE_0 | PROFILE_3,                          DEV_ON_DGHTR_BRD},
    { MMC0Pads,             AM_DEVICE_MMCHS0,       PROFILE_ALL & ~PROFILE_5,                       DEV_ON_BASEBOARD},
    { MMC0_NoCD_Pads,       AM_DEVICE_MMCHS0,       PROFILE_5,                                      DEV_ON_BASEBOARD},
    { MMC1Pads,             AM_DEVICE_MMCHS1,       PROFILE_2,                                      DEV_ON_DGHTR_BRD},
    { MMC2Pads,             AM_DEVICE_MMCHS2,       PROFILE_4,                                      DEV_ON_DGHTR_BRD},
    { NANDPads,             AM_DEVICE_NAND,         PROFILE_ALL & ~PROFILE_2 & ~PROFILE_3,          DEV_ON_DGHTR_BRD},
    { NORPads,              AM_DEVICE_NOR,          PROFILE_3,                                      DEV_ON_DGHTR_BRD},    
    { RGMIIMultiplePhyPads, AM_DEVICE_CPGMAC0,      PROFILE_1 | PROFILE_2 | PROFILE_4 | PROFILE_6,  DEV_ON_DGHTR_BRD},
    { RGMIISinglePhyPads,   AM_DEVICE_CPGMAC0,      PROFILE_0 | PROFILE_3 | PROFILE_5 | PROFILE_7,  DEV_ON_BASEBOARD},
    { SPI0Pads,             AM_DEVICE_MCSPI0,       PROFILE_2,                                      DEV_ON_DGHTR_BRD},
    { UART0Pads,            AM_DEVICE_UART0,        PROFILE_ALL,                                    DEV_ON_BASEBOARD},    
    { USB0Pads,             AM_DEVICE_USB0,         PROFILE_ALL,                                    DEV_ON_BASEBOARD},    
    {0},
};

const EVM_PIN_MUX * AM335x_EVM_PIN_MUX[] = {
	GP_BOARD_PIN_MUX,
	NULL,
	NULL,
	NULL,
};


PAD_INFO* BSPGetAllPadsInfo()
{    
    return g_allowedPadCfg;
}

const PAD_INFO* BSPGetDevicePadInfo(OMAP_DEVICE device)
{
    DWORD i=0;
    EVM_PIN_MUX * board_pin_mux = AM335x_EVM_PIN_MUX[g_dwBoardId];
    if (board_pin_mux == NULL) {
        //OALMSG(1,(L"BSPGetDevicePadInfo: Invalid board ID %d\r\n",g_dwBoardId));   
        return NULL;
    }

    for (i = 0; board_pin_mux[i].padInfo != 0; i++)  {
        //look for the device
        if (board_pin_mux[i].device != device)
            continue;     
        //found the device; now check the profile
		if ((g_dwBoardProfile == PROFILE_NONE) || (board_pin_mux[i].profile & g_dwBoardProfile)) {
            //profile match found; now check if device on daugter card and if daugter card present
			if (((board_pin_mux->device_on == DEV_ON_DGHTR_BRD) && (g_dwBoardHasDcard == TRUE)) ||
                  (board_pin_mux->device_on == DEV_ON_BASEBOARD))
				return board_pin_mux[i].padInfo;			
		}        
	}
    
    //OALMSG(1,(L"BSPGetDevicePadInfo: No Device Pad Info found for device %d\r\n",device));       
    return NULL;
}


