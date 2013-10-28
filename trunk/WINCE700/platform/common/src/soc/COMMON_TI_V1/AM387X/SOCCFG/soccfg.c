// All rights reserved ADENEO EMBEDDED 2010
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  soccfg.c
//
//  Configuration file for the device list
//
#include "am387x.h"
#include "soc_cfg.h"
#include "omap_devices_map.h"
#include "am387x_clocks.h"
#include "omap_cpgmac_regs.h"

DWORD SOCGetGPIODeviceByBank(DWORD index)
{
    switch (index)
    {
    case 1: return AM_DEVICE_GPIO0;
    case 2: return AM_DEVICE_GPIO1;
    case 3: return AM_DEVICE_GPIO2;
    case 4: return AM_DEVICE_GPIO3;
    default: return OMAP_DEVICE_NONE;   
    }
}

DWORD SOCGetSDHCDeviceBySlot(DWORD index)
{
    switch (index)
    {
    	case 1: return AM_DEVICE_SDIO0;
    	case 2: return AM_DEVICE_SDIO1;
    	default: return OMAP_DEVICE_NONE;   
    }
}

DWORD SOCGetMCSPIDeviceByBus(DWORD index)
{
	switch(index)
	{
		case 1:	return AM_DEVICE_MCSPI0;
		case 2:	return AM_DEVICE_MCSPI1;
		case 3:	return AM_DEVICE_MCSPI2;
		case 4:	return AM_DEVICE_MCSPI3;
	}
	return OMAP_DEVICE_NONE;
}

DWORD SOCGetI2CDeviceByBus(DWORD index)
{
	switch(index)
	{
		case 1:	return AM_DEVICE_I2C0;
		case 2:	return AM_DEVICE_I2C1;
		case 3:	return AM_DEVICE_I2C2;
		case 4:	return AM_DEVICE_I2C3;
	}
	return OMAP_DEVICE_NONE;
}

DWORD SOCGetUartDeviceByIndex(DWORD index)
{
	switch(index)
	{
		case 1:	return AM_DEVICE_UART0;
		case 2:	return AM_DEVICE_UART1;
		case 3:	return AM_DEVICE_UART2;
		case 4:	return AM_DEVICE_UART3;
		case 5:	return AM_DEVICE_UART4;
		case 6:	return AM_DEVICE_UART5;
	}
	return OMAP_DEVICE_NONE;
}

DWORD SOCGetGPMCAddress(DWORD index)
{
    UNREFERENCED_PARAMETER(index);
    return AM387X_GPMC_REGS_PA;
}

//
//const EMAC_MEM_LAYOUT g_EmacMemLayout = {
//    0x0900,		// EMAC_CTRL_OFFSET
//    0x2000,		// EMAC_RAM_OFFSET
//    0x0800,		// EMAC_MDIO_OFFSET
//    0x0000,		// EMAC_REG_OFFSET,
//    0x4000,		// EMAC_TOTAL_MEMORY,
//    0x00000000	// EMAC_RAM_ABS_ADDR ????
//};
//
//DWORD SOCGetEMACDevice(DWORD index)
//{
//	switch (index){
//		case 0: return AM_DEVICE_EMAC0;
//		case 1: return AM_DEVICE_EMAC1;
//	}	
//	return AM_DEVICE_EMAC0;
//}
//
//
//DWORD SOCGetHdmiDeviceByIndex(DWORD index)
//{
//	switch(index)
//	{
//		case 1:	return AM_DEVICE_HDMI;
//	}
//	return OMAP_DEVICE_NONE;
//}
//

UINT32 SOCGetSysFreqKHz(void)
{
	return 20000;
}

