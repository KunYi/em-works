// All rights reserved ADENEO EMBEDDED 2010
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  devicemap.c
//
//  Configuration file for the device list
//
#include "am387x.h"
#include "omap_devices_map.h"
#include "am387x_clocks.h"

const DEVICE_ADDRESS_MAP s_DeviceAddressMap[] = {
//  DeviceAddress    Device id
//------------------------------------------------------------------------------
    {        AM387X_I2C0_REGS_PA,         AM_DEVICE_I2C0           },
    {        AM387X_I2C1_REGS_PA,         AM_DEVICE_I2C1           },
    {        AM387X_I2C2_REGS_PA,         AM_DEVICE_I2C2           },
    {        AM387X_I2C3_REGS_PA,         AM_DEVICE_I2C3           },
    {        AM387X_UART0_REGS_PA,        AM_DEVICE_UART0          },
    {        AM387X_UART1_REGS_PA,        AM_DEVICE_UART1          },
    {        AM387X_UART2_REGS_PA,        AM_DEVICE_UART2          },
    {        AM387X_UART3_REGS_PA,        AM_DEVICE_UART3          },
    {        AM387X_UART4_REGS_PA,        AM_DEVICE_UART4          },
    {        AM387X_UART5_REGS_PA,        AM_DEVICE_UART5          },
    {        AM387X_GPIO0_REGS_PA,        AM_DEVICE_GPIO0		   },
    {        AM387X_GPIO1_REGS_PA,        AM_DEVICE_GPIO1          },
    {        AM387X_GPIO2_REGS_PA,        AM_DEVICE_GPIO2		   },
    {        AM387X_GPIO3_REGS_PA,        AM_DEVICE_GPIO3          },
    {        AM387X_MCSPI0_REGS_PA,       AM_DEVICE_MCSPI0		   },
    {        AM387X_MCSPI1_REGS_PA,       AM_DEVICE_MCSPI1		   },
    {        AM387X_MCSPI2_REGS_PA,       AM_DEVICE_MCSPI2		   },
    {        AM387X_MCSPI3_REGS_PA,       AM_DEVICE_MCSPI3         },
    {        AM387X_GPTIMER1_REGS_PA,     AM_DEVICE_TIMER1         },
    {        AM387X_GPTIMER2_REGS_PA,     AM_DEVICE_TIMER2         },
    

    /* Note: for Centaurus MMC, the actual base PA should be AM387X_MMCHS_REGS_PA.
             0x0100 is added before returning so that the struct OMAP_MMCHS_REGS
             defined in COMMON_TI\INC\omap_sdhc_regs.h and the sdhc.c source code 
             can be re-used without modifications.

             However the first three Centaurus SD/SDIO registers, 
                 SD_HL_REV       (0x0000  resp to AM387X_MMCHS_REGS_PA)
                 SD_HL_HWINFO    (0x0004)
                 SD_HL_SYSCONFIG (0x0010) 
             will not be covered by the OMAP_MMCHS_REGS structure.

             Also, OMAP_MMCHS_REGS does not include the following 4 Centaurus regs
                 SD_FE      (0x0250)
                 SD_ADMAES  (0x0254)
                 SD_ADMASAL (0x0258)
                 SD_ADMASAH (0x025C)

             Ref: uboot/mmc_host_def.h/OMAP_HSMMC_BASE
     */
    {        AM387X_MMCHS0_REGS_PA+0x0100,  AM_DEVICE_SDIO0         },
    {        AM387X_MMCHS1_REGS_PA+0x0100,  AM_DEVICE_SDIO1         },
    {        AM387X_WDT0_REGS_PA,  		    AM_DEVICE_WDT0          },
    {        0,                             OMAP_DEVICE_NONE        }
};

const DEVICE_IRQ_MAP s_DeviceIrqMap[]=
{
    // IRQ              Device              Extra
	//{40,	AM_DEVICE_EMAC0,	L"THRS"		},
	//{41,	AM_DEVICE_EMAC0,	L"RX"		},
	//{42,	AM_DEVICE_EMAC0,	L"TX"		},
	//{43,	AM_DEVICE_EMAC0,	L"MISC"		},
	{28,	AM_DEVICE_SDIO1,	NULL		},
	{44,	AM_DEVICE_UART3,	NULL		},
	{45,	AM_DEVICE_UART4,	NULL		},
	{46,	AM_DEVICE_UART5,	NULL		},

	{64,	AM_DEVICE_SDIO0,	NULL		},
	{65,	AM_DEVICE_MCSPI0,	NULL		},

    {67,	AM_DEVICE_TIMER1,	NULL		},
    {68,	AM_DEVICE_TIMER2,	NULL		},

	{72,	AM_DEVICE_UART0,	NULL		},
	{73,	AM_DEVICE_UART1,	NULL		},
	{74,	AM_DEVICE_UART2,	NULL		},

	{125,	AM_DEVICE_MCSPI1,	NULL		},
	{126,	AM_DEVICE_MCSPI2,	NULL		},
	{127,	AM_DEVICE_MCSPI3,	NULL		},
    {0,     OMAP_DEVICE_NONE,   NULL        }
};
