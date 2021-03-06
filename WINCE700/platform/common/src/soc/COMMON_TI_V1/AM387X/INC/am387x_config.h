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

//------------------------------------------------------------------------------
//
//  File:  am387x_config.h
//
//  This header file is comprised of control pad configuration module register
//  details defined as structures and macros for configuring the pads.
//

#ifndef __AM387X_CONFIG_H
#define __AM387X_CONFIG_H

//------------------------------------------------------------------------------
// System PAD Configuration Registers
#include <omap_types.h>

typedef volatile struct {
    REG32	CONTROL_REVISION;		// offset 0x0000 
    REG32	CONTROL_HWINFO;			// offset 0x0000 
    UINT32	zzzReserved01[2];
    REG32	CONTROL_SYSCONFIG;		// offset 0x0010
    UINT32	zzzReserved02[11];		// 0x0014-0x003C
} AM387X_OCP_CONF_REGS;

typedef volatile struct {
	REG32	CONTROL_STATUS;			// 0x0040
	REG32	BOOTSTAS;				// 0x0044
	REG32	DSPBOOTADDR;			// 0x0048
	UINT32	res_004c_005c[5];
	REG32	MMR_LOCK0;				// 0x0060
	REG32	MMR_LOCK1;				// 0x0064
	REG32	MMR_LOCK2;				// 0x0068
	REG32	MMR_LOCK3;				// 0x006C
	REG32	MMR_LOCK4;				// 0x0070
	UINT32  res_0074_00fc[35];				// 0x004c-0x00fc
} AM387X_DEVICE_BOOT_REGS;

typedef volatile struct {			// 0x100-0x2fc
	UINT32	placeholder[128];		// TODO: make structure
} AM387X_SECnFUSE_KEY_REGS;

typedef volatile struct {			// 0x300-0x3fc
	UINT32	placeholder[64];		// TODO: make structure
} AM387X_SECURE_SM_REGS;

typedef volatile struct {
	UINT32	res_0400_0404[2];
	REG32	SGX_VBBLDO_CTRL;	// 0x408 SGX VBB LDO Control Register
	UINT32  res_040C_0410[2];
	REG32	IVA_VBBLDO_CTRL;	// 0x414  IVA VBB LDO Control Register
	REG32	GEM_VBBLDO_CTRL;	// 0x418 GEM VBB LDO Control Register
	REG32	CORTEX_VBBLDO_CTRL;	// 0x41C Cortex VBB LDO Control Register
	UINT32	res_0420_0424[2];
	REG32	RAMLDO_CTRL0;		// 0x428 RAM LDO Control Register 0
	REG32	RAMLDO_CTRL1;		// 0x42C RAM LDO Control Register 1
	REG32	RAMLDO_CTRL2;		// 0x430 RAM LDO Control Register 2
	REG32	RAMLDO_CTRL3;		// 0x434 RAM LDO Control Register 3
	REG32	RAMLDO_CTRL4;		// 0x438 RAM LDO Control Register 4
	REG32	RAMLDO_CTRL5;		// 0x43C RAM LDO Control Register 5
	REG32	REFCLK_LJCBLDO_CTRL;// 0x440 Serdes refclk LJCB LDO control
	UINT32	res_0444;
	REG32	BANDGAP0_CTRL;		// 0x448 Bandgap 0 Control Register
	REG32	BANDGAP0_TRIM;		// 0x44C Bandgap 0 trim Register
	REG32	BANDGAP1_CTRL;		// 0x450 Bandgap 1 Control Register
	REG32	BANDGAP1_TRIM;		// 0x454 Bandgap 1 trim Register
	UINT32	res_0458_464[4];
	REG32	OSC0_CTRL;			// 0x468 Oscillator 0 Control Register
	REG32	OSC1_CTRL;			// 0x46C Oscillator 1 Control Register
	UINT32	res_0470_047C[4];
	REG32	PCIE_CFG;			// 0x480 PCIE Configuration
	UINT32	res_0484_04FC[31];
	REG32	PE_SCRATCHPAD_0;	// 0x500 PE Scratch Register bits
	REG32	PE_SCRATCHPAD_1;	// 0x504 PE Scratch Register bits
	REG32	PE_SCRATCHPAD_2;	// 0x508 PE Scratch Register bits
	REG32	PE_SCRATCHPAD_3;	// 0x50C PE Scratch Register bits
	UINT32	res_0510_05FC[60];
    REG32	DEVICE_ID;				// 0x0600 
    REG32	DEVI_FATURE;			// 0x0604 
	REG32	INIT_PRIORITY_0;		// 0x0608
	REG32	INIT_PRIORITY_1;		// 0x060C
	REG32	MMU_CFG;				// 0x0610
	REG32	TPTC_CFG;				// 0x0614	
	UINT32	res_0618;				// 0x0618	
	REG32	DSP_IDLE_CFG;			// 0x061C	
	REG32	USB_CTRL0;				// 0x0620	
	REG32	USB_STS0;				// 0x0624
	REG32	USB_CTRL1;				// 0x0628	
	REG32	USB_STS1;				// 0x062C	
	REG32	MAC_ID0_LO;				// 0x0630	
	REG32	MAC_ID0_HI;				// 0x0634	
	REG32	MAC_ID1_LO;				// 0x0638	
	REG32	MAC_ID1_HI;				// 0x063C	
	REG32	SW_REVISION;			// 0x0640	
	REG32	DCAN_RAMINIT;			// 0x0644	
	REG32	res_0648;				// 0x0648	
	REG32	AUD_CTRL;				// 0x064C	
	REG32	GMII_SEL;				// 0x0650
	REG32	OCMEM_PWRDN;			// 0x0654	
	UINT32	res_0658;
	REG32	DUCATI_MEM_PWRDN;		// 0x065C	
	UINT32	res_0660_066C[4];
	REG32	SD_DAC_CTRL;			// 0x0670 SD DAC Control
	REG32	SD_DAC0_CAL;			// 0x0674 SD DAC 0 Calibration
	REG32	SD_DAC1_CAL;			// 0x0678 SD DAC 1 Calibration
	REG32	SD_DAC0_REGCTRL;		// 0x067C SD DAC 0 Register Control
	REG32	SD_DAC0_REGSTATUS;		// 0x0680 SD DAC 0 Register Status
	REG32	SD_DAC1_REGCTRL;		// 0x0684 SD DAC 1 Register Control
	REG32	SD_DAC1_REGSTATUS;		// 0x0688 SD DAC 1 Register Status
	UINT32	res_068C_0690[2];
	REG32	EMIF_CLK_GATE;	        // 0x0694Gate clock for EMIF0/1
	UINT32	res_0698;
	REG32	CONTROL_CAMERA_RX;		// 0x069CControl the CSI2 PHY
	REG32	SMRT_CTRL;				// 0x06A0SRsleep control of the SR analog
	REG32	MODENA_HW_DBG_SEL;		// 0x06A4Controls debug info exported from modena
	REG32	MODENA_HW_DBG_INFO;		// 0x06A8Debug info from Modena
	UINT32	res_06AC;
	REG32	PRCM_DEBUG_ALWON_DEFAULT;// 0x06B0PRCM debug registers
	REG32	PRCM_DEBUG_PD_DOMAIN_STATUS;// 0x06B4PRCM debug registers
	REG32	PM_SMA;					// 0x06B8PM SMA Register
	UINT32	res_06B8_06D4[8];
	REG32	PCIE_PLLCFG0;			// 0x6D8 PCIE SerDes PLL Configuration Register 0
	REG32	PCIE_PLLCFG1;			// 0x6DC PCIE SerDes PLL Configuration Register 1
	REG32	PCIE_PLL_CFG2;			// 0x6E0 PCIE SerDes PLL Configuration Register 2
	REG32	PCIE_PLLCFG3;			// 0x6E4 PCIE SerDes PLL Configuration Register 3
	REG32	PCIE_PLLCFG4;			// 0x6E8 PCIE SerDes PLL Configuration Register 4
	REG32	PCIE_PLLSTATUS;			// 0x6EC PCIE SerDes PLL Status Register
	REG32	PCIE_RXSTATUS;			// 0x6F0 PCIE SerDes Receive Status register
	REG32	PCIE_TXSTATUS;			// 0x6F4 PCIE SerDes Transmit Status register
	REG32	PCIE_TESTCFG;			// 0x6F8 PCIE SerDes Test Config register
	REG32	PCIE_MISCCFG;			// 0x6FC PCIE misc config for future use
	UINT32	res_0700_071C[8];
	REG32	SATA_PLLCFG0;		// 0x720 PCIE SerDes PLL Configuration Register 0
	REG32	SATA_PLLCFG1;		// 0x724 SATA SerDes PLL Configuration Register 1
	REG32	SATA_PLLCFG2;		// 0x728 SATA SerDes PLL Configuration Register 2
	REG32	SATA_PLLCFG3;		// 0x72C SATA SerDes PLL Configuration Register 3
	REG32	SATA_PLLCFG4;		// 0x730 SATA SerDes PLL Configuration Register 4
	REG32	SATA_PLLSTATUS;		// 0x734 SATA SerDes PLL Status Register
	REG32	SATA_RXSTATUS;		// 0x738 SATA SerDes Receive Status register
	REG32	SATA_TXSTATUS;		// 0x73C SATA SerDes Transmit Status register
	REG32	SATA_TESTCFG;		// 0x740 SATA SerDes Test Config register
	UINT32	res_0744_076C[10];
	REG32	VDD_MPU_OPP_050;	// 0x770 MPU Voltage domain OPP 25 Ntarget
	REG32	VDD_MPU_OPP_100;	// 0x774 MPU Voltage domain OPP 100 Ntarget
	REG32	VDD_MPU_OPP_119;	// 0x778 MPU Voltage domain OPP 119 Ntarget
	REG32	VDD_MPU_OPP_TURB0;	// 0x77C MPU Voltage domain OPP Turbo Ntarget
	UINT32	res_0780_0784[2];
	REG32	VDD_GEM_OPP_050;	// 0x788 GEM Voltage domain OPP 25 Ntarget
	REG32	VDD_GEM_OPP_100;	// 0x78C GEM Voltage domain OPP 100 Ntarget
	REG32	VDD_GEM_OPP_119;	// 0x790 GEM Voltage domain OPP 119 Ntarget
	REG32	VDD_GEM_OPP_TURB0;	// 0x794 GEM Voltage domain OPP Turbo Ntarget
	UINT32	res_0798_079C[2];
	REG32	VDD_IVA_OPP_050;	// 0x7A0 IVA Voltage domain OPP 25 Ntarget
	REG32	VDD_IVA_OPP_100;	// 0x7A4 IVA Voltage domain OPP 100 Ntarget
	REG32	VDD_IVA_OPP_119;	// 0x7A8 IVA Voltage domain OPP 119 Ntarget
	REG32	VDD_IVA_OPP_TURB0;	// 0x7AC IVA Voltage domain OPP Turbo Ntarget
	UINT32	res_07B0_07B4[2];
	REG32	VDD_CORE_OPP_050;	// 0x7B8 CORE Voltage domain OPP 25 Ntarget
	REG32	VDD_CORE_OPP_100;	// 0x7BC CORE Voltage domain OPP 100 Ntarget
	REG32	VDD_CORE_OPP_119;	// 0x7C0 CORE Voltage domain OPP 119 Ntarget
	REG32	VDD_CORE_OPP_TURB0;	// 0x7C4 CORE Voltage domain OPP Turbo Ntarget
	UINT32	res_07C8_07CC[2];
	REG32	BB_SCALE;			// 0x7D0 Bbias and Dynamic Core voltage scaling register
	UINT32	res_07CC_07F0[10];
	REG32	USB_VID_PID;		// 0x7F4 USB Vendor ID and Product ID
	REG32	PCIE_VID_PID;		// 0x7F8 PCIE Vendor ID and Product ID
	REG32	EFUSE_SMA;			// 0x7FC EFUSE SMA Register
} AM387X_DEVICE_CONF_REGS;


//#define SAFE_MODE                       (7)
//#define RELEASED_PAD_DEFAULT_CONFIG     (INPUT_ENABLED | PULL_RESISTOR_DISABLED | MUXMODE(SAFE_MODE))
//#define UNUSED_PAD_DEFAULT_CONFIG       RELEASED_PAD_DEFAULT_CONFIG
#define PAD_ID(x) (offsetof(AM387X_SYSC_PADCONFS_REGS,CONTROL_PADCONF_##x)/sizeof(UINT32))

// AM387X_SYSC_PADCONFS_REGS
typedef volatile struct {
    REG32 CONTROL_PADCONF_MMC1_CLK		;   // 0x0800	1
    REG32 CONTROL_PADCONF_MMC1_CMD		;   // 0x0804
    REG32 CONTROL_PADCONF_MMC1_DAT0		;   // 0x0808
    REG32 CONTROL_PADCONF_MMC1_DAT1		;   // 0x080C
    REG32 CONTROL_PADCONF_MMC1_DAT2		;   // 0x0810	5
    REG32 CONTROL_PADCONF_MMC1_DAT3		;   // 0x0814
    REG32 CONTROL_PADCONF_OSC_WAKE		;   // 0x0818
    REG32 CONTROL_PADCONF_MMC0_CLK		;   // 0x081C
    REG32 CONTROL_PADCONF_MMC0_CMD		;   // 0x0820
    REG32 CONTROL_PADCONF_MMC0_DAT0		;   // 0x0824	10
	REG32 CONTROL_PADCONF_MMC0_DAT1		;   // 0x0828
    REG32 CONTROL_PADCONF_MMC0_DAT2		;   // 0x082C
    REG32 CONTROL_PADCONF_MMC0_DAT3		;   // 0x0830
    REG32 CONTROL_PADCONF_XREF_CLK0		;   // 0x0834
    REG32 CONTROL_PADCONF_XREF_CLK1		;   // 0x0838	15
	REG32 CONTROL_PADCONF_XREF_CLK2		;   // 0x083C
    REG32 CONTROL_PADCONF_MCASP0_ACLKX	;   // 0x0840
    REG32 CONTROL_PADCONF_MCASP0_FSX	;   // 0x0844
    REG32 CONTROL_PADCONF_MCASP0_ACLKR	;   // 0x0848
    REG32 CONTROL_PADCONF_MCASP0_FSR	;   // 0x084C	20
	REG32 CONTROL_PADCONF_MCASP0_AXR0	;   // 0x0850
    REG32 CONTROL_PADCONF_MCASP0_AXR1	;   // 0x0854
    REG32 CONTROL_PADCONF_MCASP0_AXR2	;   // 0x0858
    REG32 CONTROL_PADCONF_MCASP0_AXR3	;   // 0x085C
    REG32 CONTROL_PADCONF_MCASP0_AXR4	;   // 0x0860	25
	REG32 CONTROL_PADCONF_MCASP0_AXR5	;   // 0x0864
    REG32 CONTROL_PADCONF_MCASP0_AXR6	;   // 0x0868
    REG32 CONTROL_PADCONF_MCASP0_AXR7	;   // 0x086C
    REG32 CONTROL_PADCONF_MCASP0_AXR8	;   // 0x0870
    REG32 CONTROL_PADCONF_MCASP0_AXR9	;   // 0x0874	30
	REG32 CONTROL_PADCONF_MCASP1_ACLKX	;   // 0x0878
    REG32 CONTROL_PADCONF_MCASP1_FSX	;   // 0x087C
    REG32 CONTROL_PADCONF_MCASP1_ACLKR	;   // 0x0880
    REG32 CONTROL_PADCONF_MCASP1_FSR	;   // 0x0884
    REG32 CONTROL_PADCONF_MCASP1_AXR0	;   // 0x0888	35
	REG32 CONTROL_PADCONF_MCASP1_AXR1	;   // 0x088C
    REG32 CONTROL_PADCONF_MCASP1_AXR2	;   // 0x0890
    REG32 CONTROL_PADCONF_MCASP1_AXR3	;   // 0x0894
	REG32 CONTROL_PADCONF_MCASP2_ACLKX	;   // 0x0898
    REG32 CONTROL_PADCONF_MCASP2_FSX	;   // 0x089C	40
    REG32 CONTROL_PADCONF_MCASP2_AXR0	;   // 0x08A0
    REG32 CONTROL_PADCONF_MCASP2_AXR1	;   // 0x08A4
    REG32 CONTROL_PADCONF_MCASP2_AXR2	;   // 0x08A8
	REG32 CONTROL_PADCONF_MCASP2_AXR3	;   // 0x08AC
	REG32 CONTROL_PADCONF_MCASP3_ACLKX	;   // 0x08B0	45
    REG32 CONTROL_PADCONF_MCASP3_FSX	;   // 0x08B4
    REG32 CONTROL_PADCONF_MCASP3_AXR0	;   // 0x08B8
    REG32 CONTROL_PADCONF_MCASP3_AXR1	;   // 0x08BC
    REG32 CONTROL_PADCONF_MCASP3_AXR2	;   // 0x08C0
    REG32 CONTROL_PADCONF_MCASP3_AXR3	;   // 0x08C4	50
	REG32 CONTROL_PADCONF_MCASP4_ACLKX	;   // 0x08C8
	REG32 CONTROL_PADCONF_MCASP4_FSX	;   // 0x08CC
	REG32 CONTROL_PADCONF_MCASP4_AXR0	;   // 0x08D0
	REG32 CONTROL_PADCONF_MCASP4_AXR1	;   // 0x08D4
	REG32 CONTROL_PADCONF_MCASP5_ACLKX	;   // 0x08D8	55
	REG32 CONTROL_PADCONF_MCASP5_FSX	;   // 0x08DC
	REG32 CONTROL_PADCONF_MCASP5_AXR0	;   // 0x08E0
	REG32 CONTROL_PADCONF_MCASP5_AXR1	;   // 0x08E4
	REG32 CONTROL_PADCONF_MLB_SIG		;   // 0x08E8
	REG32 CONTROL_PADCONF_MLB_DAT		;   // 0x08EC	60
	REG32 CONTROL_PADCONF_MLB_CLK		;   // 0x08F0
	REG32 CONTROL_PADCONF_MLBP_SIG_P	;   // 0x08F4
	REG32 CONTROL_PADCONF_MLBP_SIG_N	;   // 0x08F8
	REG32 CONTROL_PADCONF_MLBP_DAT_P	;   // 0x08FC
	REG32 CONTROL_PADCONF_MLBP_DAT_N	;   // 0x0900	65
	REG32 CONTROL_PADCONF_MLBP_CLK_P	;   // 0x0904
	REG32 CONTROL_PADCONF_MLBP_CLK_N	;   // 0x0908
	REG32 CONTROL_PADCONF_DCAN0_TX		;   // 0x090C
	REG32 CONTROL_PADCONF_DCAN0_RX		;   // 0x0910
	REG32 CONTROL_PADCONF_UART0_TXD		;   // 0x0914	70
	REG32 CONTROL_PADCONF_UART0_RXD		;	// 0x0918	
	REG32 CONTROL_PADCONF_UART0_CTSN	;	// 0x091C
	REG32 CONTROL_PADCONF_UART0_RTSN	;	// 0x0920
	REG32 CONTROL_PADCONF_UART0_DCDN	;	// 0x0924
	REG32 CONTROL_PADCONF_UART0_DSRN	;	// 0x0928	75
	REG32 CONTROL_PADCONF_UART0_DTRN	;	// 0x092C
	REG32 CONTROL_PADCONF_UART0_RIN		;	// 0x0900
	REG32 CONTROL_PADCONF_I2C1_CSL		;	// 0x0934
	REG32 CONTROL_PADCONF_I2C1_SDA		;	// 0x0938
	REG32 CONTROL_PADCONF_SPI0_CS1		;	// 0x093C	80
	REG32 CONTROL_PADCONF_SPI0_CS0		;	// 0x0940
	REG32 CONTROL_PADCONF_SPI0_SCLK		;	// 0x0944
	REG32 CONTROL_PADCONF_SPI0_D1		;	// 0x0948
	REG32 CONTROL_PADCONF_SPI0_D0		;	// 0x094C
	REG32 CONTROL_PADCONF_SPI1_CS0		;	// 0x0950	85
	REG32 CONTROL_PADCONF_SPI1_SCLK		;	// 0x0954
	REG32 CONTROL_PADCONF_SPI1_D1		;	// 0x0958
	REG32 CONTROL_PADCONF_SPI1_D0		;	// 0x095C
	REG32 CONTROL_PADCONF_GPMC_AD0		;	// 0x0960
	REG32 CONTROL_PADCONF_GPMC_AD1		;	// 0x0964	90
	REG32 CONTROL_PADCONF_GPMC_AD2		;	// 0x0968
	REG32 CONTROL_PADCONF_GPMC_AD3		;	// 0x096C
	REG32 CONTROL_PADCONF_GPMC_AD4		;	// 0x0970
	REG32 CONTROL_PADCONF_GPMC_AD5		;	// 0x0974
	REG32 CONTROL_PADCONF_GPMC_AD6		;	// 0x0978	95
	REG32 CONTROL_PADCONF_GPMC_AD7		;	// 0x097C
	REG32 CONTROL_PADCONF_GPMC_AD8		;	// 0x0980
	REG32 CONTROL_PADCONF_GPMC_AD9		;	// 0x0984
	REG32 CONTROL_PADCONF_GPMC_AD10		;	// 0x0988
	REG32 CONTROL_PADCONF_GPMC_AD11		;	// 0x098C	100
	REG32 CONTROL_PADCONF_GPMC_AD12		;	// 0x0990
	REG32 CONTROL_PADCONF_GPMC_AD13		;	// 0x0994
	REG32 CONTROL_PADCONF_GPMC_AD14		;	// 0x0998
	REG32 CONTROL_PADCONF_GPMC_AD15		;	// 0x099C
	REG32 CONTROL_PADCONF_GPMC_A16		;	// 0x09A0	105
	REG32 CONTROL_PADCONF_GPMC_A17		;	// 0x09A4
	REG32 CONTROL_PADCONF_GPMC_A18		;	// 0x09A8
	REG32 CONTROL_PADCONF_GPMC_A19		;	// 0x09AC
	REG32 CONTROL_PADCONF_GPMC_A20		;	// 0x09B0
	REG32 CONTROL_PADCONF_GPMC_A21		;	// 0x09B4	110
	REG32 CONTROL_PADCONF_GPMC_A22		;	// 0x09B8
	REG32 CONTROL_PADCONF_GPMC_A23		;	// 0x09BC
	REG32 CONTROL_PADCONF_MMC2_DAT7		;	// 0x09C0
	REG32 CONTROL_PADCONF_MMC2_DAT6		;	// 0x09C4
	REG32 CONTROL_PADCONF_MMC2_DAT5		;	// 0x09C8	115
	REG32 CONTROL_PADCONF_MMC2_DAT4		;	// 0x09CC
	REG32 CONTROL_PADCONF_MMC2_DAT3		;	// 0x09D0
	REG32 CONTROL_PADCONF_MMC2_DAT2		;	// 0x09D4
	REG32 CONTROL_PADCONF_MMC2_DAT1		;	// 0x09D8
	REG32 CONTROL_PADCONF_MMC2_DAT0		;	// 0x09DC	120
	REG32 CONTROL_PADCONF_MMC2_CLK		;	// 0x09E0
	REG32 CONTROL_PADCONF_GPMC_CS0		;	// 0x09E4
	REG32 CONTROL_PADCONF_GPMC_CS1		;	// 0x09E8
	REG32 CONTROL_PADCONF_GPMC_CS2		;	// 0x09EC
	REG32 CONTROL_PADCONF_GPMC_CS3		;	// 0x09F0	125
	REG32 CONTROL_PADCONF_GPMC_CS4		;	// 0x09F4
	REG32 CONTROL_PADCONF_GPMC_CLK		;	// 0x09F8
	REG32 CONTROL_PADCONF_GPMC_ADVN_ALE	;	// 0x0A0C
	REG32 CONTROL_PADCONF_GPMC_OEN_REN	;	// 0x0A00
	REG32 CONTROL_PADCONF_GPMC_WEN		;	// 0x0A04	130
	REG32 CONTROL_PADCONF_GPMC_BEN0		;	// 0x0A08
	REG32 CONTROL_PADCONF_GPMC_BEN1		;	// 0x0A0C
	REG32 CONTROL_PADCONF_GPMC_WAIT0	;	// 0x0A10
	REG32 CONTROL_PADCONF_VIN0_CLK1		;	// 0x0A14
	REG32 CONTROL_PADCONF_VIN0_DE0_MUX0	;	// 0x0A18	135
	REG32 CONTROL_PADCONF_VIN0_FLD0_MUX0;	// 0x0A1C
	REG32 CONTROL_PADCONF_VIN0_CLK0		;	// 0x0A20
	REG32 CONTROL_PADCONF_VIN0_HSYNC0	;	// 0x0A24
	REG32 CONTROL_PADCONF_VIN0_VSYNC0	;	// 0x0A28
	REG32 CONTROL_PADCONF_VIN0_D0		;	// 0x0A2C	140
	REG32 CONTROL_PADCONF_VIN0_D1		;	// 0x0A30
	REG32 CONTROL_PADCONF_VIN0_D2		;	// 0x0A34
	REG32 CONTROL_PADCONF_VIN0_D3		;	// 0x0A38
	REG32 CONTROL_PADCONF_VIN0_D4		;	// 0x0A3C
	REG32 CONTROL_PADCONF_VIN0_D5		;	// 0x0A40	145
	REG32 CONTROL_PADCONF_VIN0_D6		;	// 0x0A44
	REG32 CONTROL_PADCONF_VIN0_D7		;	// 0x0A48
	REG32 CONTROL_PADCONF_VIN0_D8		;	// 0x0A4C
	REG32 CONTROL_PADCONF_VIN0_D9		;	// 0x0A50
	REG32 CONTROL_PADCONF_VIN0_D10		;	// 0x0A54	150
	REG32 CONTROL_PADCONF_VIN0_D11		;	// 0x0A58
	REG32 CONTROL_PADCONF_VIN0_D12		;	// 0x0A5C
	REG32 CONTROL_PADCONF_VIN0_D13		;	// 0x0A60
	REG32 CONTROL_PADCONF_VIN0_D14		;	// 0x0A64
	REG32 CONTROL_PADCONF_VIN0_D15		;	// 0x0A68	155
	REG32 CONTROL_PADCONF_VIN0_D16		;	// 0x0A6C
	REG32 CONTROL_PADCONF_VIN0_D17		;	// 0x0A70
	REG32 CONTROL_PADCONF_VIN0_D18		;	// 0x0A74
	REG32 CONTROL_PADCONF_VIN0_D19		;	// 0x0A78
	REG32 CONTROL_PADCONF_VIN0_D20		;	// 0x0A7C	160
	REG32 CONTROL_PADCONF_VIN0_D21		;	// 0x0A80
	REG32 CONTROL_PADCONF_VIN0_D22		;	// 0x0A84
	REG32 CONTROL_PADCONF_VIN0_D23		;	// 0x0A88
	REG32 CONTROL_PADCONF_VIN0_DE0_MUX1	;	// 0x0A8C
	REG32 CONTROL_PADCONF_VIN0_DE1		;	// 0x0A90	165
	REG32 CONTROL_PADCONF_VIN0_FLD0_MUX1;	// 0x0A94
	REG32 CONTROL_PADCONF_VIN0_FLD1		;	// 0x0A98
	REG32 CONTROL_PADCONF_VOUT1_G_Y_YC1	;	// 0x0A9C
	REG32 CONTROL_PADCONF_VOUT1_G_Y_YC0	;	// 0x0AA0
	REG32 CONTROL_PADCONF_VOUT1_R_CR1	;	// 0x0AA4	170
	REG32 CONTROL_PADCONF_VOUT1_R_CR0	;	// 0x0AA8
	REG32 CONTROL_PADCONF_VOUT1_B_CB_C1	;	// 0x0AAC
	REG32 CONTROL_PADCONF_VOUT1_B_CB_C0	;	// 0x0AB0
	REG32 CONTROL_PADCONF_VOUT1_FID		;	// 0x0AB4
	REG32 CONTROL_PADCONF_VOUT0_FID		;	// 0x0AB8	175
	REG32 CONTROL_PADCONF_VOUT0_CLK		;	// 0x0ABC
	REG32 CONTROL_PADCONF_VOUT0_HSYNC	;	// 0x0AC0
	REG32 CONTROL_PADCONF_VOUT0_VSYNC	;	// 0x0AC4
	REG32 CONTROL_PADCONF_VOUT0_AVID	;	// 0x0AC8
	REG32 CONTROL_PADCONF_VOUT0_B_CB_C2	;	// 0x0ACC	180
	REG32 CONTROL_PADCONF_VOUT0_B_CB_C3	;	// 0x0AD0
	REG32 CONTROL_PADCONF_VOUT0_B_CB_C4	;	// 0x0AD4
	REG32 CONTROL_PADCONF_VOUT0_B_CB_C5	;	// 0x0AD8
	REG32 CONTROL_PADCONF_VOUT0_B_CB_C6	;	// 0x0ADC
	REG32 CONTROL_PADCONF_VOUT0_B_CB_C7	;	// 0x0AE0	185
	REG32 CONTROL_PADCONF_VOUT0_B_CB_C8	;	// 0x0AE4
	REG32 CONTROL_PADCONF_VOUT0_B_CB_C9	;	// 0x0AE8
	REG32 CONTROL_PADCONF_VOUT0_G_Y_YC2	;	// 0x0AEC
	REG32 CONTROL_PADCONF_VOUT0_G_Y_YC3	;	// 0x0AF0
	REG32 CONTROL_PADCONF_VOUT0_G_Y_YC4	;	// 0x0AF4	190
	REG32 CONTROL_PADCONF_VOUT0_G_Y_YC5	;	// 0x0AF8
	REG32 CONTROL_PADCONF_VOUT0_G_Y_YC6	;	// 0x0AFC
	REG32 CONTROL_PADCONF_VOUT0_G_Y_YC7	;	// 0x0B00
	REG32 CONTROL_PADCONF_VOUT0_G_Y_YC8	;	// 0x0B04
	REG32 CONTROL_PADCONF_VOUT0_G_Y_YC9	;	// 0x0B08	195
	REG32 CONTROL_PADCONF_VOUT0_R_CR2	;	// 0x0B0C
	REG32 CONTROL_PADCONF_VOUT0_R_CR3	;	// 0x0B10
	REG32 CONTROL_PADCONF_VOUT0_R_CR4	;	// 0x0B14
	REG32 CONTROL_PADCONF_VOUT0_R_CR5	;	// 0x0B18
	REG32 CONTROL_PADCONF_VOUT0_R_CR6	;	// 0x0B1C	200
	REG32 CONTROL_PADCONF_VOUT0_R_CR7	;	// 0x0B20
	REG32 CONTROL_PADCONF_VOUT0_R_CR8	;	// 0x0B24
	REG32 CONTROL_PADCONF_VOUT0_R_CR9	;	// 0x0B28
	REG32 CONTROL_PADCONF_VOUT1_CLK		;	// 0x0B2C
	REG32 CONTROL_PADCONF_VOUT1_HSYNC	;	// 0x0B30	205
	REG32 CONTROL_PADCONF_VOUT1_VSYNC	;	// 0x0B34
	REG32 CONTROL_PADCONF_VOUT1_AVID	;	// 0x0B38
	REG32 CONTROL_PADCONF_VOUT1_B_CB_C3	;	// 0x0B3C
	REG32 CONTROL_PADCONF_VOUT1_B_CB_C4	;	// 0x0B40
	REG32 CONTROL_PADCONF_VOUT1_B_CB_C5	;	// 0x0B44	210
	REG32 CONTROL_PADCONF_VOUT1_B_CB_C6	;	// 0x0B48
	REG32 CONTROL_PADCONF_VOUT1_B_CB_C7	;	// 0x0B4C
	REG32 CONTROL_PADCONF_VOUT1_B_CB_C8	;	// 0x0B50
	REG32 CONTROL_PADCONF_VOUT1_B_CB_C9	;	// 0x0B54
	REG32 CONTROL_PADCONF_VOUT1_G_Y_YC3	;	// 0x0B58	215
	REG32 CONTROL_PADCONF_VOUT1_G_Y_YC4	;	// 0x0B5C
	REG32 CONTROL_PADCONF_VOUT1_G_Y_YC5	;	// 0x0B60
	REG32 CONTROL_PADCONF_VOUT1_G_Y_YC6	;	// 0x0B64
	REG32 CONTROL_PADCONF_VOUT1_G_Y_YC7	;	// 0x0B68
	REG32 CONTROL_PADCONF_VOUT1_G_Y_YC8	;	// 0x0B6C	220
	REG32 CONTROL_PADCONF_VOUT1_G_Y_YC9	;	// 0x0B70
	REG32 CONTROL_PADCONF_VOUT1_R_CR4	;	// 0x0B74
	REG32 CONTROL_PADCONF_VOUT1_R_CR5	;	// 0x0B78
	REG32 CONTROL_PADCONF_VOUT1_R_CR6	;	// 0x0B7C
	REG32 CONTROL_PADCONF_VOUT1_R_CR7	;	// 0x0B80	225
	REG32 CONTROL_PADCONF_VOUT1_R_CR8	;	// 0x0B84
	REG32 CONTROL_PADCONF_VOUT1_R_CR9	;	// 0x0B88
	REG32 CONTROL_PADCONF_VOUT1_G_Y_YC2	;	// 0x0B8C
	REG32 CONTROL_PADCONF_VOUT1_R_CR3	;	// 0x0B90
	REG32 CONTROL_PADCONF_VOUT1_R_CR2	;	// 0x0B94	230
	REG32 CONTROL_PADCONF_VOUT1_B_CB_C2	;	// 0x0B98
	REG32 CONTROL_PADCONF_RMII_REFCLK	;	// 0x0B9C
	REG32 CONTROL_PADCONF_MDIO_MCLK		;	// 0x0BA0
	REG32 CONTROL_PADCONF_MDIO_D		;	// 0x0BA4
	REG32 CONTROL_PADCONF_GMII0_TXCLK	;	// 0x0BA8	235
	REG32 CONTROL_PADCONF_GMII0_COL		;	// 0x0BAC
	REG32 CONTROL_PADCONF_GMII0_CRS		;	// 0x0BB0
	REG32 CONTROL_PADCONF_GMII0_RXER	;	// 0x0BB4
	REG32 CONTROL_PADCONF_GMII0_RXCLK	;	// 0x0BB8
	REG32 CONTROL_PADCONF_GMII0_RXD0	;	// 0x0BBC	240
	REG32 CONTROL_PADCONF_GMII0_RXD1	;	// 0x0BC0
	REG32 CONTROL_PADCONF_GMII0_RXD2	;	// 0x0BC4
	REG32 CONTROL_PADCONF_GMII0_RXD3	;	// 0x0BC8
	REG32 CONTROL_PADCONF_GMII0_RXD4	;	// 0x0BCC
	REG32 CONTROL_PADCONF_GMII0_RXD5	;	// 0x0BD0	245
	REG32 CONTROL_PADCONF_GMII0_RXD6	;	// 0x0BD4
	REG32 CONTROL_PADCONF_GMII0_RXD7	;	// 0x0BD8
	REG32 CONTROL_PADCONF_GMII0_RXDV	;	// 0x0BDC
	REG32 CONTROL_PADCONF_GMII0_GTXCLK	;	// 0x0BE0
	REG32 CONTROL_PADCONF_GMII0_TXD0	;	// 0x0BE4	250
	REG32 CONTROL_PADCONF_GMII0_TXD1	;	// 0x0BE8
	REG32 CONTROL_PADCONF_GMII0_TXD2	;	// 0x0BEC
	REG32 CONTROL_PADCONF_GMII0_TXD3	;	// 0x0BF0
	REG32 CONTROL_PADCONF_GMII0_TXD4	;	// 0x0BF4
	REG32 CONTROL_PADCONF_GMII0_TXD5	;	// 0x0BF8	255
	REG32 CONTROL_PADCONF_GMII0_TXD6	;	// 0x0BFC
	REG32 CONTROL_PADCONF_GMII0_TXD7	;	// 0x0C00
	REG32 CONTROL_PADCONF_GMII0_TXEN	;	// 0x0C04
	REG32 CONTROL_PADCONF_CLKIN32		;	// 0x0C08
	REG32 CONTROL_PADCONF_RESETN		;	// 0x0C0C	260
	REG32 CONTROL_PADCONF_NMIN			;	// 0x0C10
	REG32 CONTROL_PADCONF_RSTOUTN		;	// 0x0C14
	REG32 CONTROL_PADCONF_I2C0_SCL		;	// 0x0C18
	REG32 CONTROL_PADCONF_I2C0_SDA		;	// 0x0C1C
	REG32 CONTROL_PADCONF_PINCNTL265	;	// 0x0C20	265
	REG32 CONTROL_PADCONF_PINCNTL266	;	// 0x0C24
	REG32 CONTROL_PADCONF_PINCNTL267	;	// 0x0C28
	REG32 CONTROL_PADCONF_PINCNTL268	;	// 0x0C2C
	REG32 CONTROL_PADCONF_PINCNTL269	;	// 0x0C30
	REG32 CONTROL_PADCONF_USB_DRVVBUS	;	// 0x0C34	270
	REG32 CONTROL_PADCONF_PINCNTL271	;	// 0x0C38
	UINT32	res_0C3C_0DFC[113];
} AM387X_SYSC_PADCONFS_REGS;



//------------------------------------------------------------------------------

#define MUX_MODE_0                      (1 << 0)
#define MUX_MODE_1                      (1 << 1)
#define MUX_MODE_2                      (1 << 2)
#define MUX_MODE_3                      (1 << 3)
#define MUX_MODE_4                      (1 << 4)
#define MUX_MODE_5                      (1 << 5)
#define MUX_MODE_6                      (1 << 6)
#define MUX_MODE_7                      (1 << 7)

#define MUX_SLEWCTRL_SLOW				(1 << 19)
#define MUX_SLEWCTRL_FAST				(0 << 19)
#define INPUT_ENABLE                    (1 << 18)
#define INPUT_DISABLE                   (0 << 18)
#define PULL_DOWN						(0 << 17)
#define PULL_UP							(1 << 17)
#define PULL_DISABLE                    (1 << 16)
#define PULL_ENABLE                     (0 << 16)


typedef volatile struct {
	REG32	CQDETECT;				// 0xE00  Status of IO voltage (1.8 or 3.3) of the IO voltage domain
	REG32	DDR0_IO_CTRL;		// 0xE04  IO control register
	REG32	DDR1_IO_CTRL;		// 0xE08  IO control register
	REG32	VTP0_CTRL;				// 0xE0C  VTP0 control register
	REG32	VTP1_CTRL;				// 0xE10  VTP1 control register
	REG32	VREF_CTRL;				// 0xE14  Vref control register
	REG32	MLBP_SIG_IO_CTRL;		// 0xE18  MLBP�s SIG IO control register
	REG32	MLBP_DAT_IO_CTRL;		// 0xE1C  MLBP�s DAT IO control register
	REG32	MLBP_CLK_BG_CTRL;		// 0xE20  MLBP�s CLK reviver and Bandgap control register
	REG32	SERDES_REFCLK_CTL;		// 0xE24  SerDes Refclk powerdown Control
	UINT32	res_0E28_0EFC[54];
} AM387X_SYSC_MISC_REGS;

typedef volatile struct {
	REG32	DSP_INTMUX_15_18;	// 0xF00  DSP Interrupt mux for Interrupt 15 to 18
	REG32	DSP_INTMUX_19_22;	// 0xF04  DSP Interrupt mux for Interrupt 19 to 22
	REG32	DSP_INTMUX_23_26;	// 0xF08  DSP Interrupt mux for Interrupt 23 to 26
	REG32	DSP_INTMUX_27_30;	// 0xF0C  DSP Interrupt mux for Interrupt 27 to 30
	REG32	DSP_INTMUX_31_34;	// 0xF10  DSP Interrupt mux for Interrupt 31 to 34
	REG32	DSP_INTMUX_35_38;	// 0xF14  DSP Interrupt mux for Interrupt 35 to 38
	REG32	DSP_INTMUX_39_42;	// 0xF18  DSP Interrupt mux for Interrupt 39 to 42
	REG32	DSP_INTMUX_43_46;	// 0xF1C  DSP Interrupt mux for Interrupt 43 to 46
	REG32	DSP_INTMUX_47_50;	// 0xF20  DSP Interrupt mux for Interrupt 47 to 50
	REG32	DSP_INTMUX_51_54;	// 0xF24  DSP Interrupt mux for Interrupt 51 to 54
	REG32	DSP_INTMUX_55_58;	// 0xF28  DSP Interrupt mux for Interrupt 55 to 58
	REG32	DSP_INTMUX_59_62;	// 0xF2C  DSP Interrupt mux for Interrupt 59 to 62
	REG32	DSP_INTMUX_63_66;	// 0xF30  DSP Interrupt mux for Interrupt 63 to 66
	REG32	DSP_INTMUX_67_70;	// 0xF34  DSP Interrupt mux for Interrupt 67 to 70
	REG32	DSP_INTMUX_71_74;	// 0xF38  DSP Interrupt mux for Interrupt 71 to 74
	REG32	DSP_INTMUX_75_78;	// 0xF3C  DSP Interrupt mux for Interrupt 75 to 78
	REG32	DSP_INTMUX_79_82;	// 0xF40  DSP Interrupt mux for Interrupt 79 to 82
	REG32	DSP_INTMUX_83_86;	// 0xF44  DSP Interrupt mux for Interrupt 83 to 86
	REG32	DSP_INTMUX_87_90;	// 0xF48  DSP Interrupt mux for Interrupt 87 to 90
	REG32	DSP_INTMUX_91_94;	// 0xF4C  DSP Interrupt mux for Interrupt 91 to 94
	REG32	DSP_INTMUX_95;		// 0xF50  DSP Interrupt mux for Interrupt 95
	REG32	DUCATI_INTMUX_0_3;	// 0xF54  Ducati Interrupt mux for Interrupt 0 to 3
	REG32	DUCATI_INTMUX_4_7;	// 0xF58  Ducati Interrupt mux for Interrupt 4 to 7
	REG32	DUCATI_INTMUX_8_11;	// 0xF5C  Ducati Interrupt mux for Interrupt 8 to 11
	REG32	DUCATI_INTMUX_12_15;// 0xF60  Ducati Interrupt mux for Interrupt 12 to 15
	REG32	DUCATI_INTMUX_16_19;// 0xF64  Ducati Interrupt mux for Interrupt 16 to 19
	REG32	DUCATI_INTMUX_20_23;// 0xF68  Ducati Interrupt mux for Interrupt 20 to 23
	REG32	DUCATI_INTMUX_24_27;// 0xF6C  Ducati Interrupt mux for Interrupt 24 to 27
	REG32	DUCATI_INTMUX_28_31;// 0xF70  Ducati Interrupt mux for Interrupt 28 to 31
	REG32	DUCATI_INTMUX_32_35;// 0xF74  Ducati Interrupt mux for Interrupt 32 to 35
	REG32	DUCATI_INTMUX_36_39;// 0xF78  Ducati Interrupt mux for Interrupt 36 to 39
	REG32	DUCATI_INTMUX_40_43;// 0xF7C  Ducati Interrupt mux for Interrupt 40 to 43
	REG32	DUCATI_INTMUX_44_47;// 0xF80  Ducati Interrupt mux for Interrupt 44 to 47
	REG32	DUCATI_INTMUX_48_51;// 0xF84  Ducati Interrupt mux for Interrupt 48 to 51
	REG32	DUCATI_INTMUX_52_55;// 0xF88  Ducati Interrupt mux for Interrupt 52 to 55
	REG32	DUCATI_INTMUX_56;	// 0xF8C  Ducati Interrupt mux for Interrupt 56
	REG32	TPCC_EVTMUX_0_3;	// 0xF90  TPCC Event mux for event no 0 to 3
	REG32	TPCC_EVTMUX_4_7;	// 0xF94  TPCC Event mux for event no 4 to 7
	REG32	TPCC_EVTMUX_8_11;	// 0xF98  TPCC Event mux for event no 8 to 11
	REG32	TPCC_EVTMUX_12_15;	// 0xF9C  TPCC Event mux for event no 12 to 15
	REG32	TPCC_EVTMUX_16_19;	// 0xFA0  TPCC Event mux for event no 16 to 19
	REG32	TPCC_EVTMUX_20_23;	// 0xFA4  TPCC Event mux for event no 20 to 23
	REG32	TPCC_EVTMUX_24_27;	// 0xFA8  TPCC Event mux for event no 24 to 27
	REG32	TPCC_EVTMUX_28_31;	// 0xFAC  TPCC Event mux for event no 28 to 31
	REG32	TPCC_EVTMUX_32_35;	// 0xFB0  TPCC Event mux for event no 32 to 35
	REG32	TPCC_EVTMUX_36_39;	// 0xFB4  TPCC Event mux for event no 36 to 39
	REG32	TPCC_EVTMUX_40_43;	// 0xFB8  TPCC Event mux for event no 40 to 43
	REG32	TPCC_EVTMUX_44_47;	// 0xFBC  TPCC Event mux for event no 44 to 47
	REG32	TPCC_EVTMUX_48_51;	// 0xFC0  TPCC Event mux for event no 48 to 51
	REG32	TPCC_EVTMUX_52_55;	// 0xFC4  TPCC Event mux for event no 52 to 55
	REG32	TPCC_EVTMUX_56_59;	// 0xFC8  TPCC Event mux for event no 56 to 59
	REG32	TPCC_EVTMUX_60_63;	// 0xFCC  TPCC Event mux for event no 60 to 63
	REG32	TIMER_EVTCAPT;		// 0xFD0  Timer 5/6/7 event capture mux
	REG32	GPIO_MUX;			// 0xFD4  GPIO1_0 to GPIO1_5 input mux
	UINT32	res_0FD8_0FFC[10];
} AM387X_SYSC_INTR_DMA_MUX_REGS;

typedef volatile struct {
	REG32	BYPASSCLK_HDMI;		// 0x1300  Bypass enable for debug on HDMI
	UINT32	res_1304_130C[3];
	REG32	DAC0_TRIM;			// 0x1310 DAC 0 Trim bits
	REG32	DAC1_TRIM;			// 0x1314 DAC 1 Trim bits
	REG32	SMA0;				// 0x1318 Selectively Modyfiable Attribute Register0
	REG32	SMA1;				// 0x131C Selectively Modyfiable Attribute Register1
	REG32	SMA2;				// 0x1320 Selectively Modyfiable Attribute Register2
	REG32	SMA3;				// 0x1324 Selectively Modyfiable Attribute Register3
	UINT32	res_1328_13FC[54];
} AM387X_SYSC_MISC2_REGS;


#endif // __AM387X_CONFIG_H

