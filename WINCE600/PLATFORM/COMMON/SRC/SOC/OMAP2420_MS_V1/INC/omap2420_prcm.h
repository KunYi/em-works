//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//
// Portions Copyright (c) Texas Instruments.  All rights reserved. 
//
//------------------------------------------------------------------------------
//
//  File:  omap2420_prcm.h
//
//  This header file is comprised of PRCM module register details defined as 
//  structures and macros for configuring and controlling PRCM module.

#ifndef __OMAP2420_PRCM_H
#define __OMAP2420_PRCM_H

//------------------------------------------------------------------------------

// Base Address : OMAP2420_PRCM_REGS_PA (0x48008000)

typedef volatile struct {

   UINT32 ulPRCM_REVISION;              //offset 0x00,
   UINT32 ulRESERVED_1[3];
   UINT32 ulPRCM_SYSCONFIG;    //offset 0x10,
   UINT32 ulRESERVED_0x14;
   UINT32 ulPRCM_IRQSTATUS_MPU; //offset 0x18,
   UINT32 ulPRCM_IRQENABLE_MPU; //offset 0x1C,
   UINT32 ulRESERVED_2[12];
   UINT32 ulPRCM_VOLTCTRL;     //offset 0x50,
   UINT32 ulPRCM_VOLTST;       //offset 0x54,
   UINT32 ulRESERVED_3[2];
   UINT32 ulPRCM_CLKSRC_CTRL;  //offset 0x60,
   UINT32 ulRESERVED_4[3];
   UINT32 ulPRCM_CLKOUT_CTRL;  //offset 0x70,
   UINT32 ulRESERVED_5[1];
   UINT32 ulPRCM_CLKEMUL_CTRL; //offset 0x78,
   UINT32 ulRESERVED_0x7C;
   UINT32 ulPRCM_CLKCFG_CTRL;  //offset 0x80,
   UINT32 ulPRCM_CLKCFG_STATUS;//offset 0x84,
   UINT32 ulRESERVED_6[2];
   UINT32 ulPRCM_VOLTSETUP;    //offset 0x90,
   UINT32 ulPRCM_CLKSSETUP;    //offset 0x94,
   UINT32 ulPRCM_POLCTRL;      //offset 0x98,
   UINT32 ulRESERVED_7[5];
   UINT32 ulGENERAL_PURPOSE1;          //offset 0xB0,
   UINT32 ulGENERAL_PURPOSE2;          //offset 0xB4,
   UINT32 ulGENERAL_PURPOSE3;          //offset 0xB8,
   UINT32 ulGENERAL_PURPOSE4;          //offset 0xBC,
   UINT32 ulGENERAL_PURPOSE5;          //offset 0xC0,
   UINT32 ulGENERAL_PURPOSE6;          //offset 0xC4,
   UINT32 ulGENERAL_PURPOSE7;          //offset 0xC8,
   UINT32 ulGENERAL_PURPOSE8;          //offset 0xCC,
   UINT32 ulGENERAL_PURPOSE9;          //offset 0xD0,
   UINT32 ulGENERAL_PURPOSE10;         //offset 0xD4,
   UINT32 ulGENERAL_PURPOSE11;         //offset 0xD8,
   UINT32 ulGENERAL_PURPOSE12;         //offset 0xDC,
   UINT32 ulGENERAL_PURPOSE13;         //offset 0xE0,
   UINT32 ulGENERAL_PURPOSE14;         //offset 0xE4,
   UINT32 ulGENERAL_PURPOSE15;         //offset 0xE8,
   UINT32 ulGENERAL_PURPOSE16;         //offset 0xEC,
   UINT32 ulGENERAL_PURPOSE17;         //offset 0xF0,
   UINT32 ulGENERAL_PURPOSE18;         //offset 0xF4,
   UINT32 ulGENERAL_PURPOSE19;         //offset 0xF8,
   UINT32 ulGENERAL_PURPOSE20;         //offset 0xFC,
   UINT32 ulRESERVED_8[16];
   UINT32 ulCM_CLKSEL_MPU;     //offset 0x140,
   UINT32 ulRESERVED_0x144;
   UINT32 ulCM_CLKSTCTRL_MPU;  //offset 0x148,
   UINT32 ulRESERVED_9[3];
   UINT32 ulRM_RSTST_MPU;      //offset 0x158,
   UINT32 ulRESERVED_10[27];
   UINT32 ulPM_WKDEP_MPU;      //offset 0x1C8,
   UINT32 ulRESERVED_11[2];
   UINT32 ulPM_EVGENCTRL_MPU;   //offset 0x1D4,
   UINT32 ulPM_EVEGENONTIM_MPU; //offset 0x1D8,
   UINT32 ulPM_EVEGENOFFTIM_MPU;//offset 0x1DC,
   UINT32 ulPM_PWSTCTRL_MPU;    //offset 0x1E0,
   UINT32 ulPM_PWSTST_MPU;      //offset 0x1E4,
   UINT32 ulRESERVED_12[6];
   UINT32 ulCM_FCLKEN1_CORE;    //offset 0x200,
   UINT32 ulCM_FCLKEN2_CORE;    //offset 0x204,
   UINT32 ulRESERVED_13[2];
   UINT32 ulCM_ICLKEN1_CORE;    //offset 0x210,
   UINT32 ulCM_ICLKEN2_CORE;    //offset 0x214,
   UINT32 ulRESERVED_0x218;
   UINT32 ulCM_ICLKEN4_CORE;    //offset 0x21C,
   UINT32 ulCM_IDLEST1_CORE;    //offset 0x220,
   UINT32 ulCM_IDLEST2_CORE;    //offset 0x224,
   UINT32 ulRESERVED_0x228;
   UINT32 ulCM_IDLEST4_CORE;    //offset 0x22C,
   UINT32 ulCM_AUTOIDLE1_CORE;  //offset 0x230,
   UINT32 ulCM_AUTOIDLE2_CORE;  //offset 0x234,
   UINT32 ulCM_AUTOIDLE3_CORE;  //offset 0x238,
   UINT32 ulCM_AUTOIDLE4_CORE;  //offset 0x23C,
   UINT32 ulCM_CLKSEL1_CORE;    //offset 0x240,
   UINT32 ulCM_CLKSEL2_CORE;    //offset 0x244,
   UINT32 ulCM_CLKSTCTRL_CORE;  //offset 0x248,
   UINT32 ulRESERVED_14[21];
   UINT32 ulPM_WKEN1_CORE;      //offset 0x2A0,
   UINT32 ulPM_WKEN2_CORE;      //offset 0x2A4,
   UINT32 ulRESERVED_15[2];
   UINT32 ulPM_WKST1_CORE;      //offset 0x2B0,
   UINT32 ulPM_WKST2_CORE;      //offset 0x2B4,
   UINT32 ulRESERVED_16[4];
   UINT32 ulPM_WKDEP_CORE;      //offset 0x2C8,
   UINT32 ulRESERVED_17[5];
   UINT32 ulPM_PWSTCTRL_CORE;   //offset 0x2E0,
   UINT32 ulPM_PWSTST_CORE;     //offset 0x2E4,
   UINT32 ulRESERVED_18[6];
   UINT32 ulCM_FCLKEN_GFX;      //offset 0x300,
   UINT32 ulRESERVED_19[3];
   UINT32 ulCM_ICLKEN_GFX;      //offset 0x310,
   UINT32 ulRESERVED_20[3];
   UINT32 ulCM_IDLEST_GFX;      //offset 0x320,
   UINT32 ulREERVED_21[7];
   UINT32 ulCM_CLKSEL_GFX;      //offset 0x340,
   UINT32 ulRESERVED_0x344;
   UINT32 ulCM_CLKSTCTRL_GFX;   //offset 0x348,
   UINT32 ulRESERVED_0x34C;
   UINT32 ulRM_RSTCTRL_GFX;     //offset 0x350,
   UINT32 ulRESERVED_0x354;
   UINT32 ulRM_RSTST_GFX;       //offset 0x358,
   UINT32 ulRESERVED_22[27];
   UINT32 ulPM_WKDEP_GFX;       //offset 0x3C8,
   UINT32 ulRESERVED_23[5];
   UINT32 ulPM_PWSTCTRL_GFX;    //offset 0x3E0,
   UINT32 ulPM_PWSTST_GFX;      //offset 0x3E4,
   UINT32 ulRESERVED_24[6];
   UINT32 ulCM_FCLKEN_WKUP;     //offset 0x400,
   UINT32 ulRESERVED_25[3];
   UINT32 ulCM_ICLKEN_WKUP;     //offset 0x410,
   UINT32 ulRESERVED_26[3];
   UINT32 ulCM_IDLEST_WKUP;     //offset 0x420,
   UINT32 ulRESERVED_27[3];
   UINT32 ulCM_AUTOIDLE_WKUP;   //offset 0x430,
   UINT32 ulRESERVED_28[3];
   UINT32 ulCM_CLKSEL_WKUP;     //offset 0x440,
   UINT32 ulRESERVED_29[3];
   UINT32 ulRM_RSTCTRL_WKUP;    //offset 0x450,
   UINT32 ulRM_RSTTIME_WKUP;    //offset 0x454,
   UINT32 ulRM_RSTST_WKUP;      //offset 0x458,
   UINT32 ulRESERVED_30[17];
   UINT32 ulPM_WKEN_WKUP;       //offset 0x4A0,
   UINT32 ulRESERVED_31[3];
   UINT32 ulPM_WKST_WKUP;       //offset 0x4B0,
   UINT32 ulRESERVED_32[19];
   UINT32 ulCM_CLKEN_PLL;       //offset 0x500,
   UINT32 ulRESERVED_33[7];
   UINT32 ulCM_IDLEST_CKGEN;    //offset 0x520,
   UINT32 ulRESERVED_34[3];
   UINT32 ulCM_AUTOIDLE_PLL;    //offset 0x530,
   UINT32 ulRESERVED_35[3];
   UINT32 ulCM_CLKSEL1_PLL;     //offset 0x540,
   UINT32 ulCM_CLKSEL2_PLL;     //offset 0x544,
   UINT32 ulRESERVED_36[174];
   UINT32 ulCM_FCLKEN_DSP;      //offset 0x800,
   UINT32 ulRESERVED_37[3];
   UINT32 ulCM_ICLKEN_DSP;      //offset 0x810,
   UINT32 ulRESERVED_38[3];
   UINT32 ulCM_IDLEST_DSP;      //offset 0x820,
   UINT32 ulRESERVED_39[3];
   UINT32 ulCM_AUTOIDLE_DSP;    //offset 0x830,
   UINT32 ulRESERVED_40[3];
   UINT32 ulCM_CLKSEL_DSP;      //offset 0x840,
   UINT32 ulRESERVED_0x844;
   UINT32 ulCM_CLKSTCTRL_DSP;   //offset 0x848,
   UINT32 ulRESERVED_0x84C;
   UINT32 ulRM_RSTCTRL_DSP;     //offset 0x850,
   UINT32 ulRESERVED_0x854;
   UINT32 ulRM_RSTST_DSP;       //offset 0x858,
   UINT32 ulRESERVED_41[17];
   UINT32 ulPM_WKEN_DSP;        //offset 0x8A0,
   UINT32 ulRESERVED_42[9];
   UINT32 ulPM_WKDEP_DSP;       //offset 0x8C8,
   UINT32 ulRESERVED_43[5];
   UINT32 ulPM_PWSTCTRL_DSP;    //offset 0x8E0,
   UINT32 ulPM_PWSTST_DSP;      //offset 0x8E4,
   UINT32 ulRESERVED_44[2];
   UINT32 ulPRCM_IRQSTATUS_DSP; //offset 0x8F0,
   UINT32 ulPRCM_IRQENABLE_DSP; //offset 0x8F4,
   UINT32 ulPRCM_IRQSTATUS_IVA; //offset 0x8F8,
   UINT32 ulPRCM_IRQENABLE_IVA; //offset 0x8FC,
}
OMAP2420_PRCM_REGS;


#define OMAP2420_RM_RSTCTRL_RSTCMD 0x00000002

// CM_FCLKEN1_CORE register bits

#define PRCM_FCLKEN1_CORE_EN_DSS1		BIT0
#define PRCM_FCLKEN1_CORE_EN_DSS2		BIT1
#define PRCM_FCLKEN1_CORE_EN_TV			BIT2
#define PRCM_FCLKEN1_CORE_EN_VLYNQ		BIT3
#define PRCM_FCLKEN1_CORE_EN_GPT2		BIT4
#define PRCM_FCLKEN1_CORE_EN_GPT3		BIT5
#define PRCM_FCLKEN1_CORE_EN_GPT4		BIT6
#define PRCM_FCLKEN1_CORE_EN_GPT5		BIT7
#define PRCM_FCLKEN1_CORE_EN_GPT6		BIT8
#define PRCM_FCLKEN1_CORE_EN_GPT7		BIT9
#define PRCM_FCLKEN1_CORE_EN_GPT8		BIT10
#define PRCM_FCLKEN1_CORE_EN_GPT9		BIT11
#define PRCM_FCLKEN1_CORE_EN_GPT10		BIT12
#define PRCM_FCLKEN1_CORE_EN_GPT11		BIT13
#define PRCM_FCLKEN1_CORE_EN_GPT12		BIT14
#define PRCM_FCLKEN1_CORE_EN_MCBSP1		BIT15
#define PRCM_FCLKEN1_CORE_EN_MCBSP2		BIT16
#define PRCM_FCLKEN1_CORE_EN_MCSPI1		BIT17
#define PRCM_FCLKEN1_CORE_EN_MCSPI2		BIT18
#define PRCM_FCLKEN1_CORE_EN_I2C1		BIT19
#define PRCM_FCLKEN1_CORE_EN_I2C2		BIT20
#define PRCM_FCLKEN1_CORE_EN_UART1		BIT21
#define PRCM_FCLKEN1_CORE_EN_UART2		BIT22
#define PRCM_FCLKEN1_CORE_EN_HDQ		BIT23
#define PRCM_FCLKEN1_CORE_EN_EAC		BIT24
#define PRCM_FCLKEN1_CORE_EN_FAC		BIT25
#define PRCM_FCLKEN1_CORE_EN_MMC		BIT26
#define PRCM_FCLKEN1_CORE_EN_MSPRO		BIT27
#define PRCM_FCLKEN1_CORE_EN_WDT3		BIT28
#define PRCM_FCLKEN1_CORE_EN_WDT4		BIT29
#define PRCM_FCLKEN1_CORE_EN_CAM		BIT31

// CM_FCLKEN2_CORE register bits

#define PRCM_FCLKEN2_CORE_EN_USB		BIT0
#define PRCM_FCLKEN2_CORE_EN_SSI		BIT1
#define PRCM_FCLKEN2_CORE_EN_UART3		BIT2

// CM_FCLKEN1_CORE register bits

#define PRCM_ICLKEN1_CORE_EN_DSS		BIT0
#define PRCM_ICLKEN1_CORE_EN_VLYNQ		BIT3
#define PRCM_ICLKEN1_CORE_EN_GPT2		BIT4
#define PRCM_ICLKEN1_CORE_EN_GPT3		BIT5
#define PRCM_ICLKEN1_CORE_EN_GPT4		BIT6
#define PRCM_ICLKEN1_CORE_EN_GPT5		BIT7
#define PRCM_ICLKEN1_CORE_EN_GPT6		BIT8
#define PRCM_ICLKEN1_CORE_EN_GPT7		BIT9
#define PRCM_ICLKEN1_CORE_EN_GPT8		BIT10
#define PRCM_ICLKEN1_CORE_EN_GPT9		BIT11
#define PRCM_ICLKEN1_CORE_EN_GPT10		BIT12
#define PRCM_ICLKEN1_CORE_EN_GPT11		BIT13
#define PRCM_ICLKEN1_CORE_EN_GPT12		BIT14
#define PRCM_ICLKEN1_CORE_EN_MCBSP1		BIT15
#define PRCM_ICLKEN1_CORE_EN_MCBSP2		BIT16
#define PRCM_ICLKEN1_CORE_EN_MCSPI1		BIT17
#define PRCM_ICLKEN1_CORE_EN_MCSPI2		BIT18
#define PRCM_ICLKEN1_CORE_EN_I2C1		BIT19
#define PRCM_ICLKEN1_CORE_EN_I2C2		BIT20
#define PRCM_ICLKEN1_CORE_EN_UART1		BIT21
#define PRCM_ICLKEN1_CORE_EN_UART2		BIT22
#define PRCM_ICLKEN1_CORE_EN_HDQ		BIT23
#define PRCM_ICLKEN1_CORE_EN_EAC		BIT24
#define PRCM_ICLKEN1_CORE_EN_FAC		BIT25
#define PRCM_ICLKEN1_CORE_EN_MMC		BIT26
#define PRCM_ICLKEN1_CORE_EN_MSPRO		BIT27
#define PRCM_ICLKEN1_CORE_EN_WDT3		BIT28
#define PRCM_ICLKEN1_CORE_EN_WDT4		BIT29
#define PRCM_ICLKEN1_CORE_EN_MAILBOXES	BIT30
#define PRCM_ICLKEN1_CORE_EN_CAM		BIT31

// CM_ICLKEN2_CORE register bits
#define PRCM_ICLKEN2_CORE_EN_USB		BIT0
#define PRCM_ICLKEN2_CORE_EN_SSI		BIT1
#define PRCM_ICLKEN2_CORE_EN_UART3		BIT2

// CM_FCLKEN_WKUP
#define PRCM_FCLKEN_WKUP_EN_GPT1        BIT0
#define PRCM_FCLKEN_WKUP_EN_GPIOS       BIT2
#define PRCM_FCLKEN_WKUP_EN_MPU_WDT     BIT3

// CM_ICLKEN_WKUP
#define PRCM_ICLKEN_WKUP_EN_GPT1        BIT0
#define PRCM_ICLKEN_WKUP_EN_32KSYNC     BIT1
#define PRCM_ICLKEN_WKUP_EN_GPIOS       BIT2
#define PRCM_ICLKEN_WKUP_EN_MPU_WDT     BIT3
#define PRCM_ICLKEN_WKUP_EN_WDT1        BIT4
#define PRCM_ICLKEN_WKUP_EN_OMAPCTRL    BIT5

//PM_WKEN_WKUP
#define PRCM_PM_WKEN_WKUP_EN_GPT1       BIT0
#define PRCM_PM_WKEN_WKUP_EN_GPIOS      BIT2

//CM_AUTOIDLE_PLL
#define PRCM_CM_AUTOIDLE_PLL_AUTO_DPLL_EN   0x00000003
#define PRCM_CM_AUTOIDLE_PLL_AUTO_96M_EN    0x0000000C
#define PRCM_CM_AUTOIDLE_PLL_AUTO_54M_EN    0x000000C0

//PRCM_CLKSRC_CTRL
#define PRCM_CLKSRC_CTRL_OFF_WHEN_RETENTION_OR_OFF    0x00000018

//PM_PWSTCTRL_CORE
#define PRCM_PM_PWSTCTRL_CORE_MEM3RETSTATE_RETAIN   BIT5
#define PRCM_PM_PWSTCTRL_CORE_MEM2RETSTATE_RETAIN   BIT4
#define PRCM_PM_PWSTCTRL_CORE_MEM1RETSTATE_RETAIN   BIT3

//CM_IDLEST1_CORE
#define PRCM_CM_IDLEST1_CORE_ST_GPT3    BIT5


#endif
