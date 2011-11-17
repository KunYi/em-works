;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this sample source code is subject to the terms of the Microsoft
; license agreement under which you licensed this sample source code. If
; you did not accept the terms of the license agreement, you are not
; authorized to use this sample source code. For the terms of the license,
; please see the license agreement between you and Microsoft or, if applicable,
; see the LICENSE.RTF on your install media or the root of your tools installation.
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
;
;//
;// Portions Copyright (c) Texas Instruments.  All rights reserved.
;//
;//------------------------------------------------------------------------------
;//
;//  File:  omap2420_prcm.h
;//
;//  This header file is comprised of PRCM module register details defined as
;//  structures and macros for configuring and controlling PRCM module.
;//------------------------------------------------------------------------------

;// Base Address : OMAP2420_PRCM_REGS_PA (0x48008000)

OMAP2420_PRCM_REVISION_OA              EQU 0x000
OMAP2420_PRCM_SYSCONFIG_OA             EQU 0x010
OMAP2420_PRCM_IRQSTATUS_MPU_OA         EQU 0x018
OMAP2420_PRCM_IRQENABLE_MPU_OA         EQU 0x01c
OMAP2420_PRCM_VOLTCTRL_OA              EQU 0x050
OMAP2420_PRCM_VOLTST_OA                EQU 0x054
OMAP2420_PRCM_CLKSRC_CTRL_OA           EQU 0x060
OMAP2420_PRCM_CLKOUT_CTRL_OA           EQU 0x070
OMAP2420_PRCM_CLKEMUL_CTRL_OA          EQU 0x078
OMAP2420_PRCM_CLKCFG_CTRL_OA           EQU 0x080
OMAP2420_PRCM_CLKCFG_STATUS_OA         EQU 0x084
OMAP2420_PRCM_VOLTSETUP_OA             EQU 0x090
OMAP2420_PRCM_CLKSSETUP_OA             EQU 0x094
OMAP2420_PRCM_POLCTRL_OA               EQU 0x098
OMAP2420_PRCM_GENERAL_PURPOSE1_OA      EQU 0x0B0
OMAP2420_PRCM_GENERAL_PURPOSE2_OA      EQU 0x0B4
OMAP2420_PRCM_GENERAL_PURPOSE3_OA      EQU 0x0B8
OMAP2420_PRCM_GENERAL_PURPOSE4_OA      EQU 0x0BC
OMAP2420_PRCM_GENERAL_PURPOSE5_OA      EQU 0x0C0
OMAP2420_PRCM_GENERAL_PURPOSE6_OA      EQU 0x0C4
OMAP2420_PRCM_GENERAL_PURPOSE7_OA      EQU 0x0C8
OMAP2420_PRCM_GENERAL_PURPOSE8_OA      EQU 0x0CC
OMAP2420_PRCM_GENERAL_PURPOSE9_OA      EQU 0x0D0
OMAP2420_PRCM_GENERAL_PURPOSE10_OA     EQU 0x0D4
OMAP2420_PRCM_GENERAL_PURPOSE11_OA     EQU 0x0D8
OMAP2420_PRCM_GENERAL_PURPOSE12_OA     EQU 0x0DC
OMAP2420_PRCM_GENERAL_PURPOSE13_OA     EQU 0x0E0
OMAP2420_PRCM_GENERAL_PURPOSE14_OA     EQU 0x0E4
OMAP2420_PRCM_GENERAL_PURPOSE15_OA     EQU 0x0E8
OMAP2420_PRCM_GENERAL_PURPOSE16_OA     EQU 0x0EC
OMAP2420_PRCM_GENERAL_PURPOSE17_OA     EQU 0x0F0
OMAP2420_PRCM_GENERAL_PURPOSE18_OA     EQU 0x0F4
OMAP2420_PRCM_GENERAL_PURPOSE19_OA     EQU 0x0F8
OMAP2420_PRCM_GENERAL_PURPOSE20_OA     EQU 0x0FC
OMAP2420_PRCM_CM_CLKSEL_MPU_OA         EQU 0x140
OMAP2420_PRCM_CLKSTCTRL_MPU_OA         EQU 0x148
OMAP2420_PRCM_RM_RSTST_MPU_OA          EQU 0x158
OMAP2420_PRCM_PM_WKDEP_MPU_OA          EQU 0x1C8
OMAP2420_PRCM_PM_EVGENCTRL_MPU_OA      EQU 0x1D4
OMAP2420_PRCM_PM_EVEGENONTIM_MPU_OA    EQU 0x1D8
OMAP2420_PRCM_PM_EVEGENOFFTIM_MPU_OA   EQU 0x1DC
OMAP2420_PRCM_PM_PWSTCTRL_MPU_OA       EQU 0x1E0
OMAP2420_PRCM_PM_PWSTST_MPU_OA         EQU 0x1E4
OMAP2420_PRCM_CM_FCLKEN1_CORE_OA       EQU 0x200
OMAP2420_PRCM_CM_FCLKEN2_CORE_OA       EQU 0x204
OMAP2420_PRCM_CM_ICLKEN1_CORE_OA       EQU 0x210
OMAP2420_PRCM_CM_ICLKEN2_CORE_OA       EQU 0x214
OMAP2420_PRCM_CM_ICLKEN4_CORE_OA       EQU 0x21C
OMAP2420_PRCM_CM_IDLEST1_CORE_OA       EQU 0x220
OMAP2420_PRCM_CM_IDLEST2_CORE_OA       EQU 0x224
OMAP2420_PRCM_CM_IDLEST4_CORE_OA       EQU 0x22C
OMAP2420_PRCM_CM_AUTOIDLE1_CORE_OA     EQU 0x230
OMAP2420_PRCM_CM_AUTOIDLE2_CORE_OA     EQU 0x234
OMAP2420_PRCM_CM_AUTOIDLE3_CORE_OA     EQU 0x238
OMAP2420_PRCM_CM_AUTOIDLE4_CORE_OA     EQU 0x23C
OMAP2420_PRCM_CM_CLKSEL1_CORE_OA       EQU 0x240
OMAP2420_PRCM_CM_CLKSEL2_CORE_OA       EQU 0x244
OMAP2420_PRCM_CM_CLKSTCTRL_CORE_OA     EQU 0x248
OMAP2420_PRCM_PM_WKEN1_CORE_OA         EQU 0x2A0
OMAP2420_PRCM_PM_WKEN2_CORE_OA         EQU 0x2A4
OMAP2420_PRCM_PM_WKST1_CORE_OA         EQU 0x2B0
OMAP2420_PRCM_PM_WKST2_CORE_OA         EQU 0x2B4
OMAP2420_PRCM_PM_WKDEP_CORE_OA         EQU 0x2C8
OMAP2420_PRCM_PM_PWSTCTRL_CORE_OA      EQU 0x2E0
OMAP2420_PRCM_PM_PWSTST_CORE_OA        EQU 0x2E4
OMAP2420_PRCM_CM_FCLKEN_GFX_OA         EQU 0x300
OMAP2420_PRCM_CM_ICLKEN_GFX_OA         EQU 0x310
OMAP2420_PRCM_CM_IDLEST_GFX_OA         EQU 0x320
OMAP2420_PRCM_CM_CLKSEL_GFX_OA         EQU 0x340
OMAP2420_PRCM_CM_CLKSTCTRL_GFX_OA      EQU 0x348
OMAP2420_PRCM_RM_RSTCTRL_GFX_OA        EQU 0x350
OMAP2420_PRCM_RM_RSTST_GFX_OA          EQU 0x358
OMAP2420_PRCM_PM_WKDEP_GFX_OA          EQU 0x3C8
OMAP2420_PRCM_PM_PWSTCTRL_GFX_OA       EQU 0x3E0
OMAP2420_PRCM_PM_PWSTST_GFX_OA         EQU 0x3E4
OMAP2420_PRCM_CM_FCLKEN_WKUP_OA        EQU 0x400
OMAP2420_PRCM_CM_ICLKEN_WKUP_OA        EQU 0x410
OMAP2420_PRCM_CM_IDLEST_WKUP_OA        EQU 0x420
OMAP2420_PRCM_CM_AUTOIDLE_WKUP_OA      EQU 0x430
OMAP2420_PRCM_CM_CLKSEL_WKUP_OA        EQU 0x440
OMAP2420_PRCM_RM_RSTCTRL_WKUP_OA       EQU 0x450
OMAP2420_PRCM_RM_RSTTIME_WKUP_OA       EQU 0x454
OMAP2420_PRCM_RM_RSTST_WKUP_OA         EQU 0x458
OMAP2420_PRCM_PM_WKEN_WKUP_OA          EQU 0x4A0
OMAP2420_PRCM_PM_WKST_WKUP_OA          EQU 0x4B0
OMAP2420_PRCM_CM_CLKEN_PLL_OA          EQU 0x500
OMAP2420_PRCM_CM_IDLEST_CKGEN_OA       EQU 0x520
OMAP2420_PRCM_CM_AUTOIDLE_PLL_OA       EQU 0x530
OMAP2420_PRCM_CM_CLKSEL1_PLL_OA        EQU 0x540
OMAP2420_PRCM_CM_CLKSEL2_PLL_OA        EQU 0x544
OMAP2420_PRCM_CM_FCLKEN_DSP_OA         EQU 0x800
OMAP2420_PRCM_CM_ICLKEN_DSP_OA         EQU 0x810
OMAP2420_PRCM_CM_IDLEST_DSP_OA         EQU 0x820
OMAP2420_PRCM_CM_AUTOIDLE_DSP_OA       EQU 0x830
OMAP2420_PRCM_CM_CLKSEL_DSP_OA         EQU 0x840
OMAP2420_PRCM_CM_CLKSTCTRL_DSP_OA      EQU 0x848
OMAP2420_PRCM_RM_RSTCTRL_DSP_OA        EQU 0x850
OMAP2420_PRCM_RM_RSTST_DSP_OA          EQU 0x858
OMAP2420_PRCM_PM_WKEN_DSP_OA           EQU 0x8A0
OMAP2420_PRCM_PM_WKDEP_DSP_OA          EQU 0x8C8
OMAP2420_PRCM_PM_PWSTCTRL_DSP_OA       EQU 0x8E0
OMAP2420_PRCM_PM_PWSTST_DSP_OA         EQU 0x8E4
OMAP2420_PRCM_PRCM_IRQSTATUS_DSP_OA    EQU 0x8F0
OMAP2420_PRCM_PRCM_IRQENABLE_DSP_OA    EQU 0x8F4
OMAP2420_PRCM_PRCM_IRQSTATUS_IVA_OA    EQU 0x8F8
OMAP2420_PRCM_IRQENABLE_IVA_OA         EQU 0x8FC

OMAP2420_RM_RSTCTRL_RSTCMD             EQU 0x00000002
    END