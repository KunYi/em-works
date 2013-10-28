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
#include "am389x.h"
#include "soc_cfg.h"
#include "omap_devices_map.h"
#include "am389x_clocks.h"
#include "omap_cpgmac_regs.h"

DWORD SOCGetGPIODeviceByBank(DWORD index)
{
    switch (index)
    {
    case 1: return AM_DEVICE_GPIO0;
    case 2: return AM_DEVICE_GPIO1;
    default: return OMAP_DEVICE_NONE;   
    }
}

DWORD SOCGetSDHCDeviceBySlot(DWORD index)
{
    switch (index)
    {
    	case 1: return AM_DEVICE_SDIO;
    	default: return OMAP_DEVICE_NONE;   
    }
}

DWORD SOCGetMCSPIDeviceByBus(DWORD index)
{
	switch(index)
	{
		case 1:	return AM_DEVICE_MCSPI;
	}
	return OMAP_DEVICE_NONE;
}

DWORD SOCGetI2CDeviceByBus(DWORD index)
{
	switch(index)
	{
		case 1:	return AM_DEVICE_I2C0;
		case 2:	return AM_DEVICE_I2C1;
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
	}
	return OMAP_DEVICE_NONE;
}

DWORD SOCGetGPMCAddress(DWORD index)
{
    UNREFERENCED_PARAMETER(index);
    return AM389X_GPMC_REGS_PA;
}
#if 0

DWORD SOCGetIntCtrlAddr()
{
    return OMAP_INTC_MPU_REGS_PA;
}

void SOCGetDSSInfo(DSS_INFO* pInfo)
{
    pInfo->DSSDevice = OMAP_DEVICE_DSS;
    pInfo->TVEncoderDevice = OMAP_DEVICE_TVOUT;
    pInfo->DSS1_REGS_PA     = OMAP_DSS1_REGS_PA;
    pInfo->DISC1_REGS_PA    = OMAP_DISC1_REGS_PA; 
    pInfo->VENC1_REGS_PA    = OMAP_VENC1_REGS_PA;
    pInfo->DSI_REGS_PA      = OMAP_DSI_REGS_PA;
    pInfo->DSI_PLL_REGS_PA  = OMAP_DSI_PLL_REGS_PA;
}

DWORD SOCGetDMADevice(DWORD index)
{
    UNREFERENCED_PARAMETER(index);
    return OMAP_DEVICE_SDMA;
}

DWORD SOCGetVRFBDevice(DWORD index)
{
    UNREFERENCED_PARAMETER(index);
    return OMAP_DEVICE_VRFB;
}

DWORD SOCGetCANDevice(DWORD index)
{
    UNREFERENCED_PARAMETER(index);
    return OMAP_DEVICE_HECC;
}

void SOCGetUSBHInfo(USBH_INFO* pInfo)
{
	pInfo->EHCIDevice		= OMAP_DEVICE_EHCI;
	pInfo->TLLDevice		= OMAP_DEVICE_USBTLL;
	pInfo->Host1Device		= OMAP_DEVICE_USBHOST1;
	pInfo->Host2Device		= OMAP_DEVICE_USBHOST2;
	pInfo->Host3Device		= OMAP_DEVICE_USBHOST3;
	pInfo->UHH_REGS_PA		= OMAP_USBUHH_REGS_PA;
	pInfo->USBTLL_REGS_PA	= OMAP_USBTLL_REGS_PA;
	pInfo->EHCI_REGS_PA		= OMAP_USBEHCI_REGS_PA;
}

DWORD SOCGetUSBOTGAddress()
{
	return (OMAP_USBHS_REGS_PA + MENTOR_CORE_OFFSET);
}

DWORD SOCGetIDCodeAddress()
{
	return OMAP_IDCODE_REGS_PA;
}

DWORD SOCGetOTGDevice()
{
	return OMAP_DEVICE_HSOTGUSB;
}

#endif

//#define AM389X_EMAC_RAM_ABS_ADDR            0x01E00000
//As seen by the EMAC (not by the CPU, the CPU see this at addr 0x5c000000)

const EMAC_MEM_LAYOUT g_EmacMemLayout = {
    0x0900,		// EMAC_CTRL_OFFSET
    0x2000,		// EMAC_RAM_OFFSET
    0x0800,		// EMAC_MDIO_OFFSET
    0x0000,		// EMAC_REG_OFFSET,
    0x4000,		// EMAC_TOTAL_MEMORY,
    0x00000000	// EMAC_RAM_ABS_ADDR ????
};

DWORD SOCGetEMACDevice(DWORD index)
{
	switch (index){
		case 0: return AM_DEVICE_EMAC0;
		case 1: return AM_DEVICE_EMAC1;
	}	
	return AM_DEVICE_EMAC0;
}

#if 0

DWORD SOCGetHDQDevice(DWORD index)
{
    UNREFERENCED_PARAMETER(index);
	return OMAP_DEVICE_HDQ;
}
#endif

DWORD SOCGetHdmiDeviceByIndex(DWORD index)
{
	switch(index)
	{
		case 1:	return AM_DEVICE_HDMI;
	}
	return OMAP_DEVICE_NONE;
}

