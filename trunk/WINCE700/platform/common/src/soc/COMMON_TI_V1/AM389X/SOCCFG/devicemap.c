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
#include "am389x.h"
#include "omap_devices_map.h"
#include "am389x_clocks.h"

const DEVICE_ADDRESS_MAP s_DeviceAddressMap[] = {
//  DeviceAddress    Device id
//------------------------------------------------------------------------------
    {        AM389X_CPGMAC0_REGS_PA,      AM_DEVICE_EMAC0          },
    {        AM389X_CPGMAC1_REGS_PA,      AM_DEVICE_EMAC1          },
    {        AM389X_I2C0_REGS_PA,         AM_DEVICE_I2C0           },
    {        AM389X_I2C1_REGS_PA,         AM_DEVICE_I2C1           },
    {        AM389X_MCSPIOCP_REGS_PA,     AM_DEVICE_MCSPI          },
    {        AM389X_UART0_REGS_PA,        AM_DEVICE_UART0          },
    {        AM389X_UART1_REGS_PA,        AM_DEVICE_UART1          },
    {        AM389X_UART2_REGS_PA,        AM_DEVICE_UART2          },
    {        AM389X_GPIO0_REGS_PA,        AM_DEVICE_GPIO0		   },
    {        AM389X_GPIO1_REGS_PA,        AM_DEVICE_GPIO1          },

    /* Note: for Netra MMC, the actual base PA should be AM389X_MMCHS_REGS_PA.
             0x0100 is added before returning so that the struct OMAP_MMCHS_REGS
             defined in COMMON_TI\INC\omap_sdhc_regs.h and the sdhc.c source code 
             can be re-used without modifications.

             However the first three Netra SD/SDIO registers, 
                 SD_HL_REV       (0x0000  resp to AM389X_MMCHS_REGS_PA)
                 SD_HL_HWINFO    (0x0004)
                 SD_HL_SYSCONFIG (0x0010) 
             will not be covered by the OMAP_MMCHS_REGS structure.

             Also, OMAP_MMCHS_REGS does not include the following 4 Netra regs
                 SD_FE      (0x0250)
                 SD_ADMAES  (0x0254)
                 SD_ADMASAL (0x0258)
                 SD_ADMASAH (0x025C)
     */
    {        AM389X_MMCHS_REGS_PA+0x0100,  AM_DEVICE_SDIO          },
    {        0,                            OMAP_DEVICE_NONE        }
};

const DEVICE_IRQ_MAP s_DeviceIrqMap[]=
{
    // IRQ              Device              Extra
	{40,	AM_DEVICE_EMAC0,	L"THRS"		},
	{41,	AM_DEVICE_EMAC0,	L"RX"		},
	{42,	AM_DEVICE_EMAC0,	L"TX"		},
	{43,	AM_DEVICE_EMAC0,	L"MISC"		},
	{44,	AM_DEVICE_EMAC1,	L"THRS"		},
	{45,	AM_DEVICE_EMAC1,	L"RX"		},
	{46,	AM_DEVICE_EMAC1,	L"TX"		},
	{47,	AM_DEVICE_EMAC1,	L"MISC"		},

	{64,	AM_DEVICE_SDIO,		NULL		},
	{65,	AM_DEVICE_MCSPI,	NULL		},
	
	{70,	AM_DEVICE_I2C0,		NULL		},
	{71,	AM_DEVICE_I2C1, 	NULL		},
	{72,	AM_DEVICE_UART0,	NULL		},
	{73,	AM_DEVICE_UART1,	NULL		},
	{74,	AM_DEVICE_UART2,	NULL		},

    {0,     OMAP_DEVICE_NONE,   NULL        }
};
