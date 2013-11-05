// All rights reserved ADENEO EMBEDDED 2010
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
//  File:  platform.c
//
//  This file contains X-Loader configuration code for AM33X
//
#include "bsp.h"
#include "bsp_cfg.h"
#include "bsp_def.h"

#include "sdk_i2c.h"
#include "sdk_padcfg.h"
#include "oal_padcfg.h"
#include "oal_i2c.h"

#include "bsp_padcfg.h"
#include "omap_cpuver.h"

#include "am33x_config.h"
#include "am33x_base_regs.h"
#include "am3xx_gpmc.h"
#include "am33x_clocks.h"
#include "am33x_prcm.h"
#include "am33x_dmm.h"

#include "tps6591x.h"

#if defined(FMD_ONENAND) && defined(FMD_NAND)
    #error FMD_ONENAND and FMD_NAND cannot both be defined.
#endif

#define IN_DDR                 0
#define CONFIG_MMC
#define CONFIG_LCDC

extern BOOL detect_board_id_and_profile_info();


/****************************************************************/

/* Module Offsets */
#define CM_PER					(AM33X_PRCM_REGS_PA + 0x0)
#define CM_WKUP					(AM33X_PRCM_REGS_PA + 0x400)
#define CM_DPLL					(AM33X_PRCM_REGS_PA + 0x500)
#define CM_DEVICE				(AM33X_PRCM_REGS_PA + 0x0700)
#define CM_CEFUSE				(AM33X_PRCM_REGS_PA + 0x0A00)
#define PRM_DEVICE				(AM33X_PRCM_REGS_PA + 0x0F00)

/* Register Offsets */
/* Core PLL ADPLLS */
#define CM_CLKSEL_DPLL_CORE		(CM_WKUP + 0x68)
#define CM_CLKMODE_DPLL_CORE	(CM_WKUP + 0x90)

/* Core HSDIV */
#define CM_DIV_M4_DPLL_CORE		(CM_WKUP + 0x80)
#define CM_DIV_M5_DPLL_CORE		(CM_WKUP + 0x84)
#define CM_DIV_M6_DPLL_CORE		(CM_WKUP + 0xD8)
#define CM_IDLEST_DPLL_CORE		(CM_WKUP + 0x5c)

/* Peripheral PLL */
#define CM_CLKSEL_DPLL_PER		(CM_WKUP + 0x9c)
#define CM_CLKMODE_DPLL_PER		(CM_WKUP + 0x8c)
#define CM_DIV_M2_DPLL_PER		(CM_WKUP + 0xAC)
#define CM_IDLEST_DPLL_PER		(CM_WKUP + 0x70)

/* Display PLL */
#define CM_CLKSEL_DPLL_DISP		(CM_WKUP + 0x54)
#define CM_CLKMODE_DPLL_DISP	(CM_WKUP + 0x98)
#define CM_DIV_M2_DPLL_DISP		(CM_WKUP + 0xA4)
#define CM_IDLEST_DPLL_DISP		(CM_WKUP + 0x48)

/* DDR PLL */
#define CM_CLKSEL_DPLL_DDR		(CM_WKUP + 0x40)
#define CM_CLKMODE_DPLL_DDR		(CM_WKUP + 0x94)
#define CM_DIV_M2_DPLL_DDR		(CM_WKUP + 0xA0)
#define CM_IDLEST_DPLL_DDR		(CM_WKUP + 0x34)

/* MPU PLL */
#define CM_CLKSEL_DPLL_MPU		(CM_WKUP + 0x2c)
#define CM_CLKMODE_DPLL_MPU		(CM_WKUP + 0x88)
#define CM_DIV_M2_DPLL_MPU		(CM_WKUP + 0xA8)
#define CM_IDLEST_DPLL_MPU		(CM_WKUP + 0x20)

/* Domain Wake UP */
#define CM_WKUP_CLKSTCTRL		(CM_WKUP + 0)	/* UART0 */
#define CM_PER_L4LS_CLKSTCTRL	(CM_PER + 0x0)	/* TIMER2 */
#define CM_PER_L3_CLKSTCTRL		(CM_PER + 0x0c)	/* EMIF */
#define CM_PER_L4FW_CLKSTCTRL	(CM_PER + 0x08)	/* EMIF FW */
#define CM_PER_L3S_CLKSTCTRL	(CM_PER + 0x4)
#define CM_PER_L4HS_CLKSTCTRL	(CM_PER + 0x011c)
#define CM_CEFUSE_CLKSTCTRL		(CM_CEFUSE + 0x0)

#define PRCM_FORCE_WAKEUP		0x2

#define PRCM_EMIF_CLK_ACTIVITY	(0x1 << 2)
#define PRCM_L3_GCLK_ACTIVITY	(0x1 << 4)

#define PLL_BYPASS_MODE			0x4
#define PLL_LOCK_MODE			0x7
#define PLL_MULTIPLIER_SHIFT	8

#define LCDC_PIXEL_CLK_DISP_CLKOUTM2    0x0
#define LCDC_PIXEL_CLK_CORE_CLKOUTM5    0x1
#define LCDC_PIXEL_CLK_PER_CLKOUTM2     0x2

#define CM_CLKSEL_LCDC_PIXEL_CLK        (CM_PER + 0x34)


/*****************************************************************/

#define VTP0_CTRL_REG					0x44E10E0C
#define VTP1_CTRL_REG					0x48140E10

#define EMIF_MOD_ID_REV					(AM33X_EMIF4_0_CFG_REGS_PA + 0x00)
#define EMIF4_0_SDRAM_STATUS            (AM33X_EMIF4_0_CFG_REGS_PA + 0x04)
#define EMIF4_0_SDRAM_CONFIG            (AM33X_EMIF4_0_CFG_REGS_PA + 0x08)
#define EMIF4_0_SDRAM_CONFIG2           (AM33X_EMIF4_0_CFG_REGS_PA + 0x0C)
#define EMIF4_0_SDRAM_REF_CTRL          (AM33X_EMIF4_0_CFG_REGS_PA + 0x10)
#define EMIF4_0_SDRAM_REF_CTRL_SHADOW   (AM33X_EMIF4_0_CFG_REGS_PA + 0x14)
#define EMIF4_0_SDRAM_TIM_1             (AM33X_EMIF4_0_CFG_REGS_PA + 0x18)
#define EMIF4_0_SDRAM_TIM_1_SHADOW      (AM33X_EMIF4_0_CFG_REGS_PA + 0x1C)
#define EMIF4_0_SDRAM_TIM_2             (AM33X_EMIF4_0_CFG_REGS_PA + 0x20)
#define EMIF4_0_SDRAM_TIM_2_SHADOW      (AM33X_EMIF4_0_CFG_REGS_PA + 0x24)
#define EMIF4_0_SDRAM_TIM_3             (AM33X_EMIF4_0_CFG_REGS_PA + 0x28)
#define EMIF4_0_SDRAM_TIM_3_SHADOW      (AM33X_EMIF4_0_CFG_REGS_PA + 0x2C)
#define EMIF0_0_SDRAM_MGMT_CTRL         (AM33X_EMIF4_0_CFG_REGS_PA + 0x38)
#define EMIF0_0_SDRAM_MGMT_CTRL_SHD     (AM33X_EMIF4_0_CFG_REGS_PA + 0x3C)
#define EMIF4_0_DDR_PHY_CTRL_1          (AM33X_EMIF4_0_CFG_REGS_PA + 0xE4)
#define EMIF4_0_DDR_PHY_CTRL_1_SHADOW   (AM33X_EMIF4_0_CFG_REGS_PA + 0xE8)
#define EMIF4_0_DDR_PHY_CTRL_2          (AM33X_EMIF4_0_CFG_REGS_PA + 0xEC)
#define EMIF4_0_IODFT_TLGC              (AM33X_EMIF4_0_CFG_REGS_PA + 0x60)


/****************************************************************************/

//------------------------------------------------------------------------------
//  Function Prototypes
//


//-----------------------------------------
//
// Function: OALGetTickCount
//
// Stub routine
//
UINT32 OALGetTickCount()
{
    return 1;
}

static VOID xdelay(UINT32 loops)
{
    OALStall(loops);
    return;
}


/****************************************************************************/

/* MAIN PLL Fdll = 1 GHZ, */
#define MPUPLL_N	23	/* (n -1 ) */
#define MPUPLL_M2	1

/* Core PLL Fdll = 1 GHZ, */
#define COREPLL_M      1000     /* 125 * n */
#define COREPLL_N      23       /* (n -1 ) */

#define COREPLL_M4     10       /* CORE_CLKOUTM4 = 200 MHZ */
#define COREPLL_M5     8       /* CORE_CLKOUTM5 = 250 MHZ */
#define COREPLL_M6     4       /* CORE_CLKOUTM6 = 500 MHZ */

/*
 * USB PHY clock is 960 MHZ. Since, this comes directly from Fdll, Fdll
 * frequency needs to be set to 960 MHZ. Hence,
 * For clkout = 192 MHZ, Fdll = 960 MHZ, divider values are given below
 */
#define PERPLL_M		960      /* M = 40 * (N + 1) */
#define PERPLL_N		23
#define PERPLL_M2		5

/* DDR Freq is 266 MHZ */
/* Set Fdll = 400 MHZ , Fdll = M * 2 * CLKINP/ N + 1; clkout = Fdll /(2 * M2) */
#define DDRPLL_M		266	/* M/N + 1 = 25/3 */
#define DDRPLL_N		23
#define DDRPLL_M2		1

/* DISP Freq is 300 MHZ */
/* Set Fdll = 600 MHZ , Fdll = M * 2 * CLKINP/ N + 1; clkout = Fdll /(2 * M2) */
#define DISPPLL_M		25	
#define DISPPLL_N		1
#define DISPPLL_M2		1


//------------------------------------------------------------------------------
//  OPP mode related table
#define DEFAULT_OPP 1
#define VDD2_INIT   0x2E //1.1.375

typedef struct CPU_OPP_SETTINGS
{
    UINT32 MPUMult;
    UINT32 VDD1Init;
}CPU_OPP_Settings, *pCPU_OPP_Settings;

CPU_OPP_Settings AM33x_OPP_Table[AM33x_OPP_NUM]=
{
    // MPU[275Mhz @ 0.95V], 
    {275, 0x1f},
    // MPU[500Mhz @ 1.1V], 
    {500, 0x2b},
    // MPU[600Mhz @ 1.2V], 
    {600, 0x33},
    // MPU[720Mhz @ 1.26V], 
    {720, 0x38}     
};


void interface_clocks_enable(void)
{
	/* Enable all the Interconnect Modules */
    EnableDeviceClocks(AM_DEVICE_L3,TRUE);

    EnableDeviceClocks(AM_DEVICE_L4LS,TRUE);

    EnableDeviceClocks(AM_DEVICE_L4FW,TRUE);

    EnableDeviceClocks(AM_DEVICE_L4WKUP,TRUE);

    EnableDeviceClocks(AM_DEVICE_L3_INSTR,TRUE);

    EnableDeviceClocks(AM_DEVICE_L4_HS,TRUE);
}

void power_domain_transition_enable(void)
{
	/*
	 * Force power domain wake up transition
	 * Ensure that the corresponding interface clock is active before
	 * using the peripheral
	 */
	OUTREG32(CM_PER_L3_CLKSTCTRL,   PRCM_FORCE_WAKEUP);
	OUTREG32(CM_PER_L4LS_CLKSTCTRL, PRCM_FORCE_WAKEUP);
	OUTREG32(CM_WKUP_CLKSTCTRL,     PRCM_FORCE_WAKEUP);
	OUTREG32(CM_PER_L4FW_CLKSTCTRL, PRCM_FORCE_WAKEUP);
	OUTREG32(CM_PER_L3S_CLKSTCTRL,  PRCM_FORCE_WAKEUP);
}

/*
 * Enable the module clock and the power domain for required peripherals
 */
void per_clocks_enable(void)
{
	/* GPMC */
    EnableDeviceClocks(AM_DEVICE_GPMC,TRUE);

	/* ELM */
    EnableDeviceClocks(AM_DEVICE_ELM,TRUE);

	/* Ethernet */
    EnableDeviceClocks(AM_DEVICE_CPGMAC0, TRUE);

	/* Enable the control module though RBL would have done it*/
    EnableDeviceClocks(AM_DEVICE_CONTROL, TRUE);

}

void mpu_pll_config(pCPU_OPP_Settings opp_setting)
{
	UINT32 clkmode, clksel, div_m2;

	clkmode = INREG32(OALPAtoUA(CM_CLKMODE_DPLL_MPU));
	clksel = INREG32(OALPAtoUA(CM_CLKSEL_DPLL_MPU));
	div_m2 = INREG32(OALPAtoUA(CM_DIV_M2_DPLL_MPU));

	/* Set the PLL to bypass Mode */
	OUTREG32(CM_CLKMODE_DPLL_MPU, PLL_BYPASS_MODE);

	while(INREG32(OALPAtoUA(CM_IDLEST_DPLL_MPU)) != 0x00000100);

	clksel = clksel & (~0x7ffff);
	clksel = clksel | ((opp_setting->MPUMult<< 0x8) | MPUPLL_N);
	OUTREG32(CM_CLKSEL_DPLL_MPU, clksel);

	div_m2 = div_m2 & ~0x1f;
	div_m2 = div_m2 | MPUPLL_M2;
	OUTREG32(CM_DIV_M2_DPLL_MPU, div_m2);

	clkmode = clkmode | 0x7;
	OUTREG32(CM_CLKMODE_DPLL_MPU, clkmode);

	while(INREG32(OALPAtoUA(CM_IDLEST_DPLL_MPU)) != 0x1);
}

void core_pll_config(void)
{
	UINT32 clkmode, clksel, div_m4, div_m5, div_m6;

	clkmode = INREG32(OALPAtoUA(CM_CLKMODE_DPLL_CORE));
	clksel = INREG32(OALPAtoUA(CM_CLKSEL_DPLL_CORE));
	div_m4 = INREG32(OALPAtoUA(CM_DIV_M4_DPLL_CORE));
	div_m5 = INREG32(OALPAtoUA(CM_DIV_M5_DPLL_CORE));
	div_m6 = INREG32(OALPAtoUA(CM_DIV_M6_DPLL_CORE));

	/* Set the PLL to bypass Mode */
	OUTREG32(CM_CLKMODE_DPLL_CORE, PLL_BYPASS_MODE);

	while(INREG32(OALPAtoUA(CM_IDLEST_DPLL_CORE)) != 0x00000100);

	clksel = clksel & (~0x7ffff);
	clksel = clksel | ((COREPLL_M << 0x8) | COREPLL_N);
	OUTREG32(CM_CLKSEL_DPLL_CORE, clksel);

	div_m4 = div_m4 & ~0x1f;
	div_m4 = div_m4 | COREPLL_M4;
	OUTREG32(CM_DIV_M4_DPLL_CORE, div_m4);

	div_m5 = div_m5 & ~0x1f;
	div_m5 = div_m5 | COREPLL_M5;
	OUTREG32(CM_DIV_M5_DPLL_CORE, div_m5);

	div_m6 = div_m6 & ~0x1f;
	div_m6 = div_m6 | COREPLL_M6;
	OUTREG32(CM_DIV_M6_DPLL_CORE, div_m6);


	clkmode = clkmode | 0x7;
	OUTREG32(CM_CLKMODE_DPLL_CORE, clkmode);

	while(INREG32(OALPAtoUA(CM_IDLEST_DPLL_CORE)) != 0x1);
}

void per_pll_config(void)
{
	UINT32 clkmode, clksel, div_m2;

	clkmode = INREG32(OALPAtoUA(CM_CLKMODE_DPLL_PER));
	clksel = INREG32(OALPAtoUA(CM_CLKSEL_DPLL_PER));
	div_m2 = INREG32(OALPAtoUA(CM_DIV_M2_DPLL_PER));

	/* Set the PLL to bypass Mode */
	OUTREG32(CM_CLKMODE_DPLL_PER, PLL_BYPASS_MODE);

	while(INREG32(OALPAtoUA(CM_IDLEST_DPLL_PER)) != 0x00000100);

	clksel = clksel & (~0x7ffff);
	clksel = clksel | ((PERPLL_M << 0x8) | PERPLL_N);
	OUTREG32(CM_CLKSEL_DPLL_PER, clksel);

	div_m2 = div_m2 & ~0x7f;
	div_m2 = div_m2 | PERPLL_M2;
	OUTREG32(CM_DIV_M2_DPLL_PER, div_m2);

	clkmode = clkmode | 0x7;
	OUTREG32(CM_CLKMODE_DPLL_PER, clkmode);

	while(INREG32(OALPAtoUA(CM_IDLEST_DPLL_PER)) != 0x1);

}

void disp_pll_config(void)
{
	UINT32 clkmode, clksel, div_m2;

	clkmode = INREG32(OALPAtoUA(CM_CLKMODE_DPLL_DISP));
	clksel = INREG32(OALPAtoUA(CM_CLKSEL_DPLL_DISP));
	div_m2 = INREG32(OALPAtoUA(CM_DIV_M2_DPLL_DISP));

	/* Set the PLL to bypass Mode */
	OUTREG32(CM_CLKMODE_DPLL_DISP, PLL_BYPASS_MODE);

	while(INREG32(OALPAtoUA(CM_IDLEST_DPLL_DISP)) != 0x00000100);

	clksel = clksel & (~0x7ffff);
	clksel = clksel | ((DISPPLL_M << 0x8) | DISPPLL_N);
	OUTREG32(CM_CLKSEL_DPLL_DISP, clksel);

	clkmode = clkmode | 0x7;
	OUTREG32(CM_CLKMODE_DPLL_DISP, clkmode);

	while(INREG32(OALPAtoUA(CM_IDLEST_DPLL_DISP)) != 0x1);

	/* SELECT the MUX  line for LCDC PIXEL clock*/
	OUTREG32(CM_CLKSEL_LCDC_PIXEL_CLK, LCDC_PIXEL_CLK_DISP_CLKOUTM2);  //250MHz

}


void ddr_pll_config(void)
{
	UINT32 clkmode, clksel, div_m2;

	clkmode = INREG32(OALPAtoUA(CM_CLKMODE_DPLL_DDR));
	clksel = INREG32(OALPAtoUA(CM_CLKSEL_DPLL_DDR));
	div_m2 = INREG32(OALPAtoUA(CM_DIV_M2_DPLL_DDR));

	/* Set the PLL to bypass Mode */
	clkmode = (clkmode & 0xfffffff8) | 0x00000004;
	OUTREG32(CM_CLKMODE_DPLL_DDR, clkmode);

	while ((INREG32(OALPAtoUA(CM_IDLEST_DPLL_DDR)) & 0x00000100) != 0x00000100);

	clksel = clksel & (~0x7ffff);
	clksel = clksel | ((DDRPLL_M << 0x8) | DDRPLL_N);
	OUTREG32(CM_CLKSEL_DPLL_DDR, clksel);

	div_m2 = div_m2 & 0xFFFFFFE0;
	div_m2 = div_m2 | DDRPLL_M2;
	OUTREG32(CM_DIV_M2_DPLL_DDR, div_m2);

	clkmode = (clkmode & 0xfffffff8) | 0x7;
	OUTREG32(CM_CLKMODE_DPLL_DDR, clkmode);

	while ((INREG32(OALPAtoUA(CM_IDLEST_DPLL_DDR)) & 0x00000001) != 0x1);
}


/*
 * Configure the PLL/PRCM for necessary peripherals
 */
void pll_init()
{
    mpu_pll_config(&AM33x_OPP_Table[DEFAULT_OPP]);
	core_pll_config();
	per_pll_config();
	ddr_pll_config();
    disp_pll_config();	
	/* Enable the required interconnect clocks */
	interface_clocks_enable();
	/* Enable power domain transition */
	power_domain_transition_enable();
	/* Enable the required peripherals */
	per_clocks_enable();
}

/****************************************************************************/

/* DDR offsets */
#define	DDR_PHY_BASE_ADDR		0x44E12000
#define	DDR_IO_CTRL				0x44E10E04
#define	DDR_CKE_CTRL			0x44E1131C
#define	CONTROL_BASE_ADDR		AM33X_CONTROL_MODULE_REGS_PA

#define	DDR_CMD0_IOCTRL			(CONTROL_BASE_ADDR + 0x1404)
#define	DDR_CMD1_IOCTRL			(CONTROL_BASE_ADDR + 0x1408)
#define	DDR_CMD2_IOCTRL			(CONTROL_BASE_ADDR + 0x140C)
#define	DDR_DATA0_IOCTRL		(CONTROL_BASE_ADDR + 0x1440)
#define	DDR_DATA1_IOCTRL		(CONTROL_BASE_ADDR + 0x1444)

#define	CMD0_CTRL_SLAVE_RATIO_0		(DDR_PHY_BASE_ADDR + 0x01C)
#define	CMD0_CTRL_SLAVE_FORCE_0		(DDR_PHY_BASE_ADDR + 0x020)
#define	CMD0_CTRL_SLAVE_DELAY_0		(DDR_PHY_BASE_ADDR + 0x024)
#define	CMD0_DLL_LOCK_DIFF_0		(DDR_PHY_BASE_ADDR + 0x028)
#define	CMD0_INVERT_CLKOUT_0		(DDR_PHY_BASE_ADDR + 0x02C)

#define	CMD1_CTRL_SLAVE_RATIO_0		(DDR_PHY_BASE_ADDR + 0x050)
#define	CMD1_CTRL_SLAVE_FORCE_0		(DDR_PHY_BASE_ADDR + 0x054)
#define	CMD1_CTRL_SLAVE_DELAY_0		(DDR_PHY_BASE_ADDR + 0x058)
#define	CMD1_DLL_LOCK_DIFF_0		(DDR_PHY_BASE_ADDR + 0x05C)
#define	CMD1_INVERT_CLKOUT_0		(DDR_PHY_BASE_ADDR + 0x060)

#define	CMD2_CTRL_SLAVE_RATIO_0		(DDR_PHY_BASE_ADDR + 0x084)
#define	CMD2_CTRL_SLAVE_FORCE_0		(DDR_PHY_BASE_ADDR + 0x088)
#define	CMD2_CTRL_SLAVE_DELAY_0		(DDR_PHY_BASE_ADDR + 0x08C)
#define	CMD2_DLL_LOCK_DIFF_0		(DDR_PHY_BASE_ADDR + 0x090)
#define	CMD2_INVERT_CLKOUT_0		(DDR_PHY_BASE_ADDR + 0x094)

#define DATA0_RD_DQS_SLAVE_RATIO_0	(DDR_PHY_BASE_ADDR + 0x0C8)
#define DATA0_RD_DQS_SLAVE_RATIO_1	(DDR_PHY_BASE_ADDR + 0x0CC)
#define	DATA0_WR_DQS_SLAVE_RATIO_0	(DDR_PHY_BASE_ADDR + 0x0DC)

#define	DATA0_WR_DQS_SLAVE_RATIO_1	(DDR_PHY_BASE_ADDR + 0x0E0)
#define	DATA0_WRLVL_INIT_RATIO_0	(DDR_PHY_BASE_ADDR + 0x0F0)

#define	DATA0_WRLVL_INIT_RATIO_1	(DDR_PHY_BASE_ADDR + 0x0F4)
#define	DATA0_GATELVL_INIT_RATIO_0	(DDR_PHY_BASE_ADDR + 0x0FC)

#define	DATA0_GATELVL_INIT_RATIO_1	(DDR_PHY_BASE_ADDR + 0x100)
#define	DATA0_FIFO_WE_SLAVE_RATIO_0	(DDR_PHY_BASE_ADDR + 0x108)

#define	DATA0_FIFO_WE_SLAVE_RATIO_1	(DDR_PHY_BASE_ADDR + 0x10C)
#define	DATA0_WR_DATA_SLAVE_RATIO_0	(DDR_PHY_BASE_ADDR + 0x120)

#define	DATA0_WR_DATA_SLAVE_RATIO_1	(DDR_PHY_BASE_ADDR + 0x124)
#define DATA0_DLL_LOCK_DIFF_0		(DDR_PHY_BASE_ADDR + 0x138)

#define DATA0_RANK0_DELAYS_0		(DDR_PHY_BASE_ADDR + 0x134)
#define	DATA1_RANK0_DELAYS_0		(DDR_PHY_BASE_ADDR + 0x1D8)


/* AM335X EMIF Register values */
#define EMIF_SDMGT				0x80000000
#define EMIF_SDRAM				0x00004650
#define EMIF_PHYCFG				0x2
#define DDR_PHY_RESET			(0x1 << 10)
#define DDR_FUNCTIONAL_MODE_EN	0x1
#define DDR_PHY_READY			(0x1 << 2)
#define	VTP_CTRL_READY			(0x1 << 5)
#define VTP_CTRL_ENABLE			(0x1 << 6)
#define VTP_CTRL_LOCK_EN		(0x1 << 4)
#define VTP_CTRL_START_EN		(0x1)
#define DDR2_RATIO				0x80	/* for mDDR */
#define CMD_FORCE				0x00	/* common #def */
#define CMD_DELAY				0x00

#if	(CONFIG_AM335X_DDR_IS_MDDR == 1)
#error "*** mDDR NOT supported ***"
#define EMIF_READ_LATENCY	0x05
#define EMIF_TIM1			0x04446249
#define EMIF_TIM2			0x101731C0
#define EMIF_TIM3			0x137
#define EMIF_SDCFG			0x20004EA3
#define EMIF_SDREF			0x57c
#define	DDR2_DLL_LOCK_DIFF	0x4
#define DDR2_RD_DQS			0x40
#define DDR2_PHY_FIFO_WE	0x56
#else
#define EMIF_READ_LATENCY	0x04
#define EMIF_TIM1			0x0666B3D6
#define EMIF_TIM2			0x143731DA
#define	EMIF_TIM3			0x00000347
#define EMIF_SDCFG			0x43805332
#define EMIF_SDREF			0x0000081a
#define DDR2_DLL_LOCK_DIFF	0x0
#define DDR2_RD_DQS			0x12
#define DDR2_PHY_FIFO_WE	0x80
#endif

#define	DDR2_INVERT_CLKOUT	0x00
#define	DDR2_WR_DQS			0x00
#define	DDR2_PHY_WRLVL		0x00
#define	DDR2_PHY_GATELVL	0x00
#define	DDR2_PHY_WR_DATA	0x40
#define	PHY_RANK0_DELAY		0x01
#define PHY_DLL_LOCK_DIFF	0x0
#define DDR_IOCTRL_VALUE	0x18B


void enable_ddr_clocks(void)
{
	/* Enable the  EMIF_FW Functional clock */
    EnableDeviceClocks(AM_DEVICE_EMIF_FW, TRUE);

	/* Enable EMIF0 Clock */
    EnableDeviceClocks(AM_DEVICE_EMIF, TRUE);

	/* Poll for emif_gclk  & L3_G clock  are active */
	while ((INREG32(OALPAtoUA(CM_PER_L3_CLKSTCTRL)) & 
       (PRCM_EMIF_CLK_ACTIVITY | PRCM_L3_GCLK_ACTIVITY)) != 
       (PRCM_EMIF_CLK_ACTIVITY | PRCM_L3_GCLK_ACTIVITY));
}

static void config_vtp(void)
{
	OUTREG32(VTP0_CTRL_REG, INREG32(OALPAtoUA(VTP0_CTRL_REG)) | VTP_CTRL_ENABLE);
	OUTREG32(VTP0_CTRL_REG, INREG32(OALPAtoUA(VTP0_CTRL_REG)) & (~VTP_CTRL_START_EN));
	OUTREG32(VTP0_CTRL_REG, INREG32(OALPAtoUA(VTP0_CTRL_REG)) | VTP_CTRL_START_EN);

	/* Poll for READY */
	while ((INREG32(OALPAtoUA(VTP0_CTRL_REG)) & VTP_CTRL_READY) != VTP_CTRL_READY);
}

static void Cmd_Macro_Config(void)
{
	OUTREG32(OALPAtoUA(CMD0_CTRL_SLAVE_RATIO_0), DDR2_RATIO);
	OUTREG32(OALPAtoUA(CMD0_CTRL_SLAVE_FORCE_0), CMD_FORCE);
	OUTREG32(OALPAtoUA(CMD0_CTRL_SLAVE_DELAY_0), CMD_DELAY);
	OUTREG32(OALPAtoUA(CMD0_DLL_LOCK_DIFF_0),    DDR2_DLL_LOCK_DIFF);
	OUTREG32(OALPAtoUA(CMD0_INVERT_CLKOUT_0),    DDR2_INVERT_CLKOUT);

	OUTREG32(OALPAtoUA(CMD1_CTRL_SLAVE_RATIO_0), DDR2_RATIO);
	OUTREG32(OALPAtoUA(CMD1_CTRL_SLAVE_FORCE_0), CMD_FORCE);
	OUTREG32(OALPAtoUA(CMD1_CTRL_SLAVE_DELAY_0), CMD_DELAY);
	OUTREG32(OALPAtoUA(CMD1_DLL_LOCK_DIFF_0),    DDR2_DLL_LOCK_DIFF);
	OUTREG32(OALPAtoUA(CMD1_INVERT_CLKOUT_0),    DDR2_INVERT_CLKOUT);

	OUTREG32(OALPAtoUA(CMD2_CTRL_SLAVE_RATIO_0), DDR2_RATIO);
	OUTREG32(OALPAtoUA(CMD2_CTRL_SLAVE_FORCE_0), CMD_FORCE);
	OUTREG32(OALPAtoUA(CMD2_CTRL_SLAVE_DELAY_0), CMD_DELAY);
	OUTREG32(OALPAtoUA(CMD2_DLL_LOCK_DIFF_0),    DDR2_DLL_LOCK_DIFF);
	OUTREG32(OALPAtoUA(CMD2_INVERT_CLKOUT_0),    DDR2_INVERT_CLKOUT);
}

static void Data_Macro_Config(int dataMacroNum)
{
	UINT32 BaseAddrOffset = 0x00;

	if (dataMacroNum == 0)
		BaseAddrOffset = 0x00;
	else if (dataMacroNum == 1)
		BaseAddrOffset = 0xA4;

	OUTREG32(
		OALPAtoUA(DATA0_RD_DQS_SLAVE_RATIO_0 + BaseAddrOffset),
        ((DDR2_RD_DQS<<30)|(DDR2_RD_DQS<<20)|(DDR2_RD_DQS<<10)|(DDR2_RD_DQS<<0))
    );

	OUTREG32(
        OALPAtoUA(DATA0_RD_DQS_SLAVE_RATIO_1 + BaseAddrOffset),
        DDR2_RD_DQS>>2
    );

	OUTREG32(
        OALPAtoUA(DATA0_WR_DQS_SLAVE_RATIO_0 + BaseAddrOffset),
        ((DDR2_WR_DQS<<30)|(DDR2_WR_DQS<<20)|(DDR2_WR_DQS<<10)|(DDR2_WR_DQS<<0))
    );

	OUTREG32(
        OALPAtoUA(DATA0_WR_DQS_SLAVE_RATIO_1 + BaseAddrOffset),
        DDR2_WR_DQS>>2
    );

	OUTREG32(
        OALPAtoUA(DATA0_WRLVL_INIT_RATIO_0 + BaseAddrOffset),
        ((DDR2_PHY_WRLVL<<30)|(DDR2_PHY_WRLVL<<20)|(DDR2_PHY_WRLVL<<10)|(DDR2_PHY_WRLVL<<0))
    );

	OUTREG32(
        OALPAtoUA(DATA0_WRLVL_INIT_RATIO_1 + BaseAddrOffset),
        DDR2_PHY_WRLVL>>2
    );

	OUTREG32(
        OALPAtoUA(DATA0_GATELVL_INIT_RATIO_0 + BaseAddrOffset),
        ((DDR2_PHY_GATELVL<<30)|(DDR2_PHY_GATELVL<<20)|(DDR2_PHY_GATELVL<<10)|(DDR2_PHY_GATELVL<<0))
    );

	OUTREG32(
        OALPAtoUA(DATA0_GATELVL_INIT_RATIO_1 + BaseAddrOffset),
        DDR2_PHY_GATELVL>>2
    );

	OUTREG32(
        OALPAtoUA(DATA0_FIFO_WE_SLAVE_RATIO_0 + BaseAddrOffset),
        ((DDR2_PHY_FIFO_WE<<30)|(DDR2_PHY_FIFO_WE<<20)|(DDR2_PHY_FIFO_WE<<10)|(DDR2_PHY_FIFO_WE<<0))
    );

	OUTREG32(
        OALPAtoUA(DATA0_FIFO_WE_SLAVE_RATIO_1 + BaseAddrOffset),
        DDR2_PHY_FIFO_WE>>2
    );

	OUTREG32(
		OALPAtoUA(DATA0_WR_DATA_SLAVE_RATIO_0 + BaseAddrOffset),
        ((DDR2_PHY_WR_DATA<<30)|(DDR2_PHY_WR_DATA<<20)|(DDR2_PHY_WR_DATA<<10)|(DDR2_PHY_WR_DATA<<0))
    );

	OUTREG32(
        OALPAtoUA(DATA0_WR_DATA_SLAVE_RATIO_1 + BaseAddrOffset),
        DDR2_PHY_WR_DATA>>2
    );

	OUTREG32(
        OALPAtoUA(DATA0_DLL_LOCK_DIFF_0 + BaseAddrOffset), 
        PHY_DLL_LOCK_DIFF
    );
}

static void config_emif_ddr2(void)
{
	UINT32 i;

	/*Program EMIF0 CFG Registers*/
	OUTREG32(OALPAtoUA(EMIF4_0_DDR_PHY_CTRL_1),        EMIF_READ_LATENCY);
	OUTREG32(OALPAtoUA(EMIF4_0_DDR_PHY_CTRL_1_SHADOW), EMIF_READ_LATENCY);
	OUTREG32(OALPAtoUA(EMIF4_0_DDR_PHY_CTRL_2),        EMIF_READ_LATENCY);

	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_TIM_1),        EMIF_TIM1);
	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_TIM_1_SHADOW), EMIF_TIM1);

	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_TIM_2),        EMIF_TIM2);
	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_TIM_2_SHADOW), EMIF_TIM2);

	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_TIM_3),        EMIF_TIM3);
	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_TIM_3_SHADOW), EMIF_TIM3);

	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_CONFIG),  EMIF_SDCFG);
	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_CONFIG2), EMIF_SDCFG);

	/*
    OUTREG32(OALPAtoUA(EMIF0_0_SDRAM_MGMT_CTRL),     EMIF_SDMGT);
 	OUTREG32(OALPAtoUA(EMIF0_0_SDRAM_MGMT_CTRL_SHD), EMIF_SDMGT);
    */
	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_REF_CTRL),        0x00004650);
	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_REF_CTRL_SHADOW), 0x00004650);

	for (i = 0; i < 5000; i++) {

	}

	/* 
    OUTREG32(OALPAtoUA(EMIF0_0_SDRAM_MGMT_CTRL),     EMIF_SDMGT);
	OUTREG32(OALPAtoUA(EMIF0_0_SDRAM_MGMT_CTRL_SHD), EMIF_SDMGT); 
    */
	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_REF_CTRL), EMIF_SDREF);
	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_REF_CTRL_SHADOW), EMIF_SDREF);

	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_CONFIG), EMIF_SDCFG);
	OUTREG32(OALPAtoUA(EMIF4_0_SDRAM_CONFIG2), EMIF_SDCFG);
}

static void config_am33x_ddr2(void)
{
	int data_macro_0 = 0;
	int data_macro_1 = 1;

	enable_ddr_clocks();

    config_vtp();

	Cmd_Macro_Config();

	Data_Macro_Config(data_macro_0);
	Data_Macro_Config(data_macro_1);

	OUTREG32(OALPAtoUA(DATA0_RANK0_DELAYS_0), PHY_RANK0_DELAY);
	OUTREG32(OALPAtoUA(DATA1_RANK0_DELAYS_0), PHY_RANK0_DELAY);

	OUTREG32(OALPAtoUA(DDR_CMD0_IOCTRL),  DDR_IOCTRL_VALUE);
	OUTREG32(OALPAtoUA(DDR_CMD1_IOCTRL),  DDR_IOCTRL_VALUE);
	OUTREG32(OALPAtoUA(DDR_CMD2_IOCTRL),  DDR_IOCTRL_VALUE);

	OUTREG32(OALPAtoUA(DDR_DATA0_IOCTRL), DDR_IOCTRL_VALUE);
	OUTREG32(OALPAtoUA(DDR_DATA1_IOCTRL), DDR_IOCTRL_VALUE);

	OUTREG32(OALPAtoUA(DDR_IO_CTRL),  INREG32(OALPAtoUA(DDR_IO_CTRL))  & 0x0fffffff);
	OUTREG32(OALPAtoUA(DDR_CKE_CTRL), INREG32(OALPAtoUA(DDR_CKE_CTRL)) | 0x00000001);

	config_emif_ddr2();
}

/****************************************************************************/

void configure_evm_pin_mux()
{
    /* do nothing here since UART0, and boot device (ex: NAND, NOR, SPI, MMC) pinmux 
               will be enabled by bootrom. Except for I2C0, there shouldnt be any other device pinmux needed 
               in xldr. For all other devices needed by eboot and kernel, set the pinmux in respective places */    
}

void enable_i2c0_pin_mux(void)
{
    // since board and profile info is not available until we have I2C0 functional, 
    // we cannot use BSPGetDevicePadInfo  
    const PAD_INFO I2C0Pads[]   =   {I2C0_PADS END_OF_PAD_ARRAY};
	ConfigurePadArray(I2C0Pads);
}

/****************************************************************************/

/* RGMII mode define */
#define RGMII_MODE_ENABLE	0xA
#define GMII_SEL_REG        (AM33X_CONTROL_MODULE_REGS_PA+0x0650)

int board_eth_init(void)
{
	/* set mii mode to rgmii in in device configure register */
	if (g_dwBoardId != AM33X_BOARDID_IA_BOARD)
		OUTREG32(GMII_SEL_REG, RGMII_MODE_ENABLE); /* GMII_SEL */

    return 0;
}


/****************************************************************************/

#define CONFIG_CMD_NAND

#define PISMO1_NAND_BASE GPMC_NAND_BASE
#define PISMO1_NAND_SIZE GPMC_NAND_SIZE

static VOID sdelay(UINT32 loops)
{
    OALStall(loops);
    return;
}

#if defined(CONFIG_CMD_NAND)
static const UINT32 gpmc_m_nand[GPMC_MAX_REG] = {
	M_NAND_GPMC_CONFIG1,
	M_NAND_GPMC_CONFIG2,
	M_NAND_GPMC_CONFIG3,
	M_NAND_GPMC_CONFIG4,
	M_NAND_GPMC_CONFIG5,
	M_NAND_GPMC_CONFIG6, 0
};

#define GPMC_CS 0

void enable_gpmc_cs_config(
    const UINT32 *gpmc_config, AM3XX_GPMC_CS *cs, UINT32 base, UINT32 size
)
{
	OUTREG32(GMII_SEL_REG, RGMII_MODE_ENABLE); /* GMII_SEL */
	OUTREG32(&cs->GPMC_CONFIG7, 0);
	sdelay(1000);
	/* Delay for settling */
	OUTREG32(&cs->GPMC_CONFIG1, gpmc_config[0]);
	OUTREG32(&cs->GPMC_CONFIG2, gpmc_config[1]);
	OUTREG32(&cs->GPMC_CONFIG3, gpmc_config[2]);
	OUTREG32(&cs->GPMC_CONFIG4, gpmc_config[3]);
	OUTREG32(&cs->GPMC_CONFIG5, gpmc_config[4]);
	OUTREG32(&cs->GPMC_CONFIG6, gpmc_config[5]);
	/* Enable the config */
	OUTREG32(&cs->GPMC_CONFIG7, (((size & 0xF) << 8) | ((base >> 24) & 0x3F) | (1 << 6)));
	sdelay(2000);
}
#endif

/*****************************************************
 * gpmc_init(): init gpmc bus
 * Init GPMC for x16, MuxMode (SDRAM in x32).
 * This code can only be executed from SRAM or SDRAM.
 *****************************************************/
void gpmc_init(void)
{
	/* putting a blanket check on GPMC based on ZeBu for now */
//+++TODO
    AM3XX_GPMC_REGS *gpmc_cfg = (AM3XX_GPMC_REGS *)AM33X_GPMC_REGS_PA;

#ifdef CONFIG_NOR_BOOT
	/* env setup */
	boot_flash_base = CONFIG_SYS_FLASH_BASE;
	boot_flash_off = CONFIG_ENV_OFFSET;
	boot_flash_sec = NOR_SECT_SIZE;
	boot_flash_env_addr = boot_flash_base + boot_flash_off;
#else
#if defined(CONFIG_CMD_NAND) || defined(CONFIG_CMD_ONENAND)
	const UINT32 *gpmc_config = NULL;
	UINT32 base = 0;
	UINT32 size = 0;
#if defined(CONFIG_ENV_IS_IN_NAND) || defined(CONFIG_ENV_IS_IN_ONENAND)
	u32 f_off = CONFIG_SYS_MONITOR_LEN;
	u32 f_sec = 0;
#endif
#endif
	/* global settings */
	OUTREG32(&gpmc_cfg->GPMC_SYSCONFIG, 0x00000008);
	OUTREG32(&gpmc_cfg->GPMC_IRQSTATUS, 0x00000100);
	OUTREG32(&gpmc_cfg->GPMC_IRQENABLE, 0x00000200);
	OUTREG32(&gpmc_cfg->GPMC_CONFIG,    0x00000012);
	/*
	 * Disable the GPMC0 config set by ROM code
	 */
	OUTREG32(&gpmc_cfg->CS[0].GPMC_CONFIG7, 0);
	sdelay(1000);

#if defined(CONFIG_CMD_NAND)	/* CS 0 */
	gpmc_config = gpmc_m_nand;

	base = PISMO1_NAND_BASE;
	size = PISMO1_NAND_SIZE;
	enable_gpmc_cs_config(gpmc_config, &gpmc_cfg->CS[0], base, size);
#if defined(CONFIG_ENV_IS_IN_NAND)
	f_off = MNAND_ENV_OFFSET;
	f_sec = (128 << 10);	/* 128 KiB */
	/* env setup */
	boot_flash_base = base;
	boot_flash_off = f_off;
	boot_flash_sec = f_sec;
	boot_flash_env_addr = f_off;
#endif
#endif

#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
	/* env setup */
	boot_flash_base = 0x0;
	boot_flash_off = CONFIG_ENV_OFFSET;
	boot_flash_sec = CONFIG_ENV_SECT_SIZE;
	boot_flash_env_addr = CONFIG_ENV_OFFSET;
#endif

#endif
}

/****************************************************************************/

int board_init()
{
    unsigned char buf[4]	= {0};
    pCPU_OPP_Settings opp_setting = &AM33x_OPP_Table[BSP_OPM_SELECT-1];    
	
	/* Configure the i2c0 pin mux */
	enable_i2c0_pin_mux();
    OALI2CInit(AM_DEVICE_I2C0);

    TWLSetOPVoltage(kVdd2,VDD2_INIT);

    if (TWLSetOPVoltage(kVdd1,opp_setting->VDD1Init))
    {
        mpu_pll_config(opp_setting);   
    } 
    
    detect_board_id_and_profile_info(); 
        
	configure_evm_pin_mux();
	
	gpmc_init();
    
	return 0;
}

/****************************************************************************/

/* WDT related */
#define WDT_BASE		AM33X_WDT1_REGS_PA /*0x44E35000*/
#define WDT_WDSC		(WDT_BASE + 0x010)
#define WDT_WDST		(WDT_BASE + 0x014)
#define WDT_WISR		(WDT_BASE + 0x018)
#define WDT_WIER		(WDT_BASE + 0x01C)
#define WDT_WWER		(WDT_BASE + 0x020)
#define WDT_WCLR		(WDT_BASE + 0x024)
#define WDT_WCRR		(WDT_BASE + 0x028)
#define WDT_WLDR		(WDT_BASE + 0x02C)
#define WDT_WTGR		(WDT_BASE + 0x030)
#define WDT_WWPS		(WDT_BASE + 0x034)
#define WDT_WDLY		(WDT_BASE + 0x044)
#define WDT_WSPR		(WDT_BASE + 0x048)
#define WDT_WIRQEOI		(WDT_BASE + 0x050)
#define WDT_WIRQSTATRAW	(WDT_BASE + 0x054)
#define WDT_WIRQSTAT	(WDT_BASE + 0x058)
#define WDT_WIRQENSET	(WDT_BASE + 0x05C)
#define WDT_WIRQENCLR	(WDT_BASE + 0x060)

#define WDT_UNFREEZE	(AM33X_CONTROL_MODULE_REGS_PA + 0x100)


//------------------------------------------------------------------------------
//
//  Function:  PlatformSetup
//
//  Initializes platform settings.  Stack based initialization only - no 
//  global variables allowed.
//
VOID PlatformSetup()
{
	/* WDT1 is already running when the bootloader gets control
	 * Disable it to avoid "random" resets
	 */
	OUTREG32(WDT_WSPR, 0xAAAA);
	while(INREG32(OALPAtoUA(WDT_WWPS)) != 0x0);

	OUTREG32(WDT_WSPR, 0x5555);
	while(INREG32(OALPAtoUA(WDT_WWPS)) != 0x0);

    pll_init();

	if (!IN_DDR)
		config_am33x_ddr2();	/* Do DDR settings */

    board_init();

    board_eth_init();
}


