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
;------------------------------------------------------------------------------
;
;  File:  omap5912_dma.inc
;
;  This file defines offset addresses for DMA registers. 
;
OMAP5912_DMA_GCR_REGS_OA                 EQU     0x0000
OMAP5912_DMA_GSCR_REGS_OA                EQU     0x0004
OMAP5912_DMA_GRST_REGS_OA                EQU     0x0008
OMAP5912_DMA_HW_ID_REGS_OA               EQU     0x0042
OMAP5912_DMA_PCH2_ID_REGS_OA             EQU     0x0044
OMAP5912_DMA_PCH0_ID_REGS_OA             EQU     0x0046
OMAP5912_DMA_PCH1_ID_REGS_OA             EQU     0x0048
OMAP5912_DMA_PCHG_ID_REGS_OA             EQU     0x004A
OMAP5912_DMA_PCHD_ID_REGS_OA             EQU     0x004C
OMAP5912_DMA_CAPS_0_U_REGS_OA            EQU     0x004E
OMAP5912_DMA_CAPS_0_L_REGS_OA            EQU     0x0050
OMAP5912_DMA_CAPS_1_U_REGS_OA            EQU     0x0052
OMAP5912_DMA_CAPS_1_L_REGS_OA            EQU     0x0054
OMAP5912_DMA_CAPS_2_REGS_OA              EQU     0x0056
OMAP5912_DMA_CAPS_3_REGS_OA              EQU     0x0058
OMAP5912_DMA_CAPS_4_REGS_OA              EQU     0x005A
OMAP5912_DMA_PCH2_SR_REGS_OA             EQU     0x0060
OMAP5912_DMA_PCH0_SR_REGS_OA             EQU     0x0080
OMAP5912_DMA_PCH1_SR_REGS_OA             EQU     0x0082
OMAP5912_DMA_PCHD_SR_REGS_OA             EQU     0x00C0

OMAP5912_DMA_LC_CSDP_REGS_OA             EQU     0x0000
OMAP5912_DMA_LC_CCR_REGS_OA              EQU     0x0002
OMAP5912_DMA_LC_CICR_REGS_OA             EQU     0x0004
OMAP5912_DMA_LC_CSR_REGS_OA              EQU     0x0006
OMAP5912_DMA_LC_CSSA_L_REGS_OA           EQU     0x0008
OMAP5912_DMA_LC_CSSA_U_REGS_OA           EQU     0x000A
OMAP5912_DMA_LC_CDSA_L_REGS_OA           EQU     0x000C
OMAP5912_DMA_LC_CDSA_U_REGS_OA           EQU     0x000E
OMAP5912_DMA_LC_CEN_REGS_OA              EQU     0x0010
OMAP5912_DMA_LC_CFN_REGS_OA              EQU     0x0012
OMAP5912_DMA_LC_CSFI_REGS_OA             EQU     0x0014
OMAP5912_DMA_LC_CSEI_REGS_OA             EQU     0x0016
OMAP5912_DMA_LC_CSAC_REGS_OA             EQU     0x0018
OMAP5912_DMA_LC_CDAC_REGS_OA             EQU     0x001A
OMAP5912_DMA_LC_CDEI_REGS_OA             EQU     0x001C
OMAP5912_DMA_LC_CDFI_REGS_OA             EQU     0x001E
OMAP5912_DMA_LC_COLOR_L_REGS_OA          EQU     0x0020
OMAP5912_DMA_LC_COLOR_U_REGS_OA          EQU     0x0022
OMAP5912_DMA_LC_CCR2_REGS_OA             EQU     0x0024
OMAP5912_DMA_LC_UNUSED026_REGS_OA        EQU     0x0026
OMAP5912_DMA_LC_CLNK_CTRL_REGS_OA        EQU     0x0028
OMAP5912_DMA_LC_LCH_CTRL_REGS_OA         EQU     0x002A

OMAP5912_DMA_LCD_CSDP_REGS_OA            EQU     0x0000
OMAP5912_DMA_LCD_CCR_REGS_OA             EQU     0x0002
OMAP5912_DMA_LCD_CTRL_REGS_OA            EQU     0x0004
OMAP5912_DMA_LCD_TOP_B1_L_REGS_OA        EQU     0x0008
OMAP5912_DMA_LCD_TOP_B1_U_REGS_OA        EQU     0x000A
OMAP5912_DMA_LCD_BOT_B1_L_REGS_OA        EQU     0x000C
OMAP5912_DMA_LCD_BOT_B1_U_REGS_OA        EQU     0x000E
OMAP5912_DMA_LCD_TOP_B2_L_REGS_OA        EQU     0x0010
OMAP5912_DMA_LCD_TOP_B2_U_REGS_OA        EQU     0x0012
OMAP5912_DMA_LCD_BOT_B2_L_REGS_OA        EQU     0x0014
OMAP5912_DMA_LCD_BOT_B2_U_REGS_OA        EQU     0x0016
OMAP5912_DMA_LCD_SRC_EI_B1_REGS_OA       EQU     0x0018
OMAP5912_DMA_LCD_SRC_FI_B1_L_REGS_OA     EQU     0x001A
OMAP5912_DMA_LCD_SRC_EI_B2_REGS_OA       EQU     0x001C
OMAP5912_DMA_LCD_SRC_FI_B2_L_REGS_OA     EQU     0x001E
OMAP5912_DMA_LCD_SRC_EN_B1_REGS_OA       EQU     0x0020
OMAP5912_DMA_LCD_SRC_EN_B2_REGS_OA       EQU     0x0022
OMAP5912_DMA_LCD_SRC_FN_B1_REGS_OA       EQU     0x0024
OMAP5912_DMA_LCD_SRC_FN_B2_REGS_OA       EQU     0x0026
OMAP5912_DMA_LCD_LCH_CTRL_REGS_OA        EQU     0x002A
OMAP5912_DMA_LCD_SRC_FI_B1_U_REGS_OA     EQU     0x0034
OMAP5912_DMA_LCD_SRC_FI_B2_U_REGS_OA     EQU     0x0036

;------------------------------------------------------------------------------

                                        END
