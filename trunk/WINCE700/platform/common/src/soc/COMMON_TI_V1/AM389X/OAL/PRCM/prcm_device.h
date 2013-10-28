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
//  File: prcm_device.h

#include "prcm_priv.h"

DeviceLookupEntry s_rgDeviceLookupTable[] =
{
//	{POWERDOMAIN                        FCLK             ICLK        ref count   CLKCTRL 
/*???*/	{PWR_ALWAYSON,  CLKDMN_UNKNOWN, CLKDMN_UNKNOWN,   0,  PRCM_OFS(CM_ALWON_CONTROL_CLKCTRL) }, // AM_DEVICE_CONTROL
	{PWR_ALWAYSON,  CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_GPIO_0_CLKCTRL)  }, // AM_DEVICE_GPIO0,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_GPIO_1_CLKCTRL)  }, // AM_DEVICE_GPIO1,
	{PWR_ALWAYSON,	CLKDMN_NULL,           CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_GPMC_CLKCTRL)    }, // AM_DEVICE_GPMC,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_I2C_0_CLKCTRL)   }, // AM_DEVICE_I2C0,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_I2C_1_CLKCTRL)   }, // AM_DEVICE_I2C1,
	{PWR_ALWAYSON,	CLKDMN_NULL,           CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_MAILBOX_CLKCTRL) }, // AM_DEVICE_MAILBOX,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_MCASP0_CLKCTRL)  }, // AM_DEVICE_MCASP0,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_MCASP1_CLKCTRL)  }, // AM_DEVICE_MCASP1,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_MCASP2_CLKCTRL)  }, // AM_DEVICE_MCASP2,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_MCBSP_CLKCTRL)   }, // AM_DEVICE_MCBSP,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_SDIO_CLKCTRL)    }, // AM_DEVICE_SDIO,              
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_SMARTCARD_0_CLKCTRL) }, // AM_DEVICE_SMARTCARD0,
 	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_SMARTCARD_1_CLKCTRL) }, // AM_DEVICE_SMARTCARD1,
	{PWR_ALWAYSON,	CLKDMN_NULL,           CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_SR_0_CLKCTRL)    }, // AM_DEVICE_SMARTREFLEX0,
	{PWR_ALWAYSON,	CLKDMN_NULL,           CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_SR_1_CLKCTRL)    }, // AM_DEVICE_SMARTREFLEX1,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_SPI_CLKCTRL)     }, // AM_DEVICE_SPI,
	{PWR_ALWAYSON,	CLKDMN_NULL,           CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_SPINBOX_CLKCTRL) }, // AM_DEVICE_SPINBOX, // SPINLOCK
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_TIMER_0_CLKCTRL) }, // AM_DEVICE_TIMER0, //READONLY
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_TIMER_1_CLKCTRL) }, // AM_DEVICE_TIMER1,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_TIMER_2_CLKCTRL) }, // AM_DEVICE_TIMER2,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_TIMER_3_CLKCTRL) }, // AM_DEVICE_TIMER3,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_TIMER_4_CLKCTRL) }, // AM_DEVICE_TIMER4,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_TIMER_5_CLKCTRL) }, // AM_DEVICE_TIMER5,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_TIMER_6_CLKCTRL) }, // AM_DEVICE_TIMER6,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_TIMER_7_CLKCTRL) }, // AM_DEVICE_TIMER7,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_UART_0_CLKCTRL)  }, // AM_DEVICE_UART0,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_UART_1_CLKCTRL)  }, // AM_DEVICE_UART1,
	{PWR_ALWAYSON,	CLKDMN_ALWON_L3_SLOW,  CLKDMN_SYSCLK6, 0,  PRCM_OFS(CM_ALWON_UART_2_CLKCTRL)  }, // AM_DEVICE_UART2,

/*???*/	{PWR_ALWAYSON,CLKDMN_UNKNOWN,CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_VLYNQ_CLKCTRL) }, // AM_DEVICE_VLYNQ,

	{PWR_ALWAYSON,	CLKDMN_ETHERNET, CLKDMN_SYSCLK5, 0, PRCM_OFS(CM_ALWON_ETHERNET_0_CLKCTRL) }, // AM_DEVICE_EMAC0,
	{PWR_ALWAYSON,	CLKDMN_ETHERNET, CLKDMN_SYSCLK5, 0, PRCM_OFS(CM_ALWON_ETHERNET_1_CLKCTRL) }, // AM_DEVICE_EMAC1,

	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_TPCC_CLKCTRL) }, // AM_DEVICE_TPCC,
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_TPTC0_CLKCTRL) }, // AM_DEVICE_TPTC0,
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_TPTC1_CLKCTRL) }, // AM_DEVICE_TPTC1,
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_TPTC2_CLKCTRL) }, // AM_DEVICE_TPTC2,
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_TPTC3_CLKCTRL) }, // AM_DEVICE_TPTC3,
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_SECSS_CLKCTRL) }, // AM_DEVICE_SECURITY_SS,  //READONLY
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_MMUCFG_CLKCTRL)}, // AM_DEVICE_MMUCFG,
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_MMUDATA_CLKCTRL) }, // AM_DEVICE_MMU, ????
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_MPU_CLKCTRL)   }, // AM_DEVICE_MPU, // READONLY
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_OCMC_0_CLKCTRL)}, // AM_DEVICE_OCMC_RAM0,
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_OCMC_1_CLKCTRL)}, // AM_DEVICE_OCMC_RAM1,
	{PWR_ALWAYSON,	CLKDMN_RTC,      CLKDMN_SYSCLK6, 0, PRCM_OFS(CM_ALWON_RTC_CLKCTRL)     }, // AM_DEVICE_RTC, //READONLY
	{PWR_ALWAYSON,	CLKDMN_RTC,      CLKDMN_SYSCLK6, 0, PRCM_OFS(CM_ALWON_WDTIMER_CLKCTRL) }, // AM_DEVICE_WDTIMER, //READONLY
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_L3_CLKCTRL)      }, // AM_DEVICE_L3,//READONLY
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_L4HS_CLKCTRL)    }, // AM_DEVICE_L4_HS, //READONLY
	{PWR_ALWAYSON,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ALWON_L4LS_CLKCTRL)    }, // AM_DEVICE_L4_LS,//READONLY
	//DEFAULT DOMAIN	
	{PWR_DEFAULT,	CLKDMN_NULL,     CLKDMN_DUCATI_INTR, 0, PRCM_OFS(CM_DEFAULT_DUCATI_CLKCTRL)}, // AM_DEVICE_MEDIA_CONTROLLER,
	{PWR_DEFAULT,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_DEFAULT_DMM_CLKCTRL)   }, // AM_DEVICE_DMM,
	{PWR_DEFAULT,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_DEFAULT_EMIF_0_CLKCTRL)}, // AM_DEVICE_EMIF4_0,
	{PWR_DEFAULT,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_DEFAULT_EMIF_1_CLKCTRL)}, // AM_DEVICE_EMIF4_1,
	{PWR_DEFAULT,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_DEFAULT_FW_CLKCTRL)    }, // AM_DEVICE_EMIF_FW,
	{PWR_DEFAULT,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_DEFAULT_SATA_CLKCTRL)  }, // AM_DEVICE_SATA,
	{PWR_DEFAULT,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_DEFAULT_PCI_CLKCTRL)   }, // AM_DEVICE_PCI,
	{PWR_DEFAULT,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_DEFAULT_TPPSS_CLKCTRL) }, // AM_DEVICE_TPP, //?????
	{PWR_DEFAULT,	CLKDMN_NULL,     CLKDMN_L3_SLOW_DEFAULT, 0, PRCM_OFS(CM_DEFAULT_USB_CLKCTRL)   }, // AM_DEVICE_USB,
	// ACTIVE DOMAIN
	{PWR_ACTIVE,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ACTIVE_GEM_CLKCTRL) }, // AM_DEVICE_C674X,
	{PWR_ACTIVE,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ACTIVE_HDMI_CLKCTRL) }, // AM_DEVICE_HDMI,
	{PWR_ACTIVE,	CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_ACTIVE_HDDSS_CLKCTRL) }, // AM_DEVICE_HD_DSS,
	//IVA and SGX
	{PWR_SGX,		CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_SGX_SGX_CLKCTRL) }, // AM_DEVICE_SGX,
	{PWR_IVA0,		CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_IVAHD0_IVAHD_CLKCTRL) }, // AM_DEVICE_IVA0,
	{PWR_IVA0,		CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_IVAHD0_SL2_CLKCTRL)   }, // AM_DEVICE_SL2_0,
	{PWR_IVA1,		CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_IVAHD1_IVAHD_CLKCTRL) }, // AM_DEVICE_IVA1,
	{PWR_IVA1,		CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_IVAHD1_SL2_CLKCTRL)   }, // AM_DEVICE_SL2_1,
	{PWR_IVA2,		CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_IVAHD2_IVAHD_CLKCTRL) }, // AM_DEVICE_IVA2,
	{PWR_IVA2,		CLKDMN_UNKNOWN,  CLKDMN_UNKNOWN, 0, PRCM_OFS(CM_IVAHD2_SL2_CLKCTRL)   }, // AM_DEVICE_SL2_2,
};

#if 0 // on those defices _CLKCTRL regs is readonly and always on
AM_DEVICE_TIMER0
AM_DEVICE_SECURITY_SS
AM_DEVICE_MPU
AM_DEVICE_RTC
AM_DEVICE_WDTIMER
AM_DEVICE_L3
AM_DEVICE_L4_HS
AM_DEVICE_L4_LS
#endif

//-----------------------------------------------------------------------------
static UINT s_rgActiveDomainDeviceCount[PWR_COUNT] = 
{
	0,	// PWR_ALWAYSON
	0,	// PWR_DEFAULT
	0,	// PWR_ACTIVE
	0,	// PWR_SGX       
	0,	// PWR_IVA0   
	0,	// PWR_IVA1
	0,	// PWR_IVA2
};

