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
;  File:  omap5912_emif.inc
;
;  This file defines offset addresses for EMIF registers. 
;
OMAP5912_EMIF_OCPT1_PRIOR_REGS_OA               EQU     0x0000
OMAP5912_EMIF_EMIFS_LRUREG_REGS_OA              EQU     0x0004
OMAP5912_EMIF_EMIFF_PRIORITY_REGS_OA            EQU     0x0008
OMAP5912_EMIF_EMIFS_CONFIG_REGS_OA              EQU     0x000C
OMAP5912_EMIF_EMIFS_CFG_0_REGS_OA               EQU     0x0010
OMAP5912_EMIF_EMIFS_CFG_1_REGS_OA               EQU     0x0014
OMAP5912_EMIF_EMIFS_CFG_2_REGS_OA               EQU     0x0018
OMAP5912_EMIF_EMIFS_CFG_3_REGS_OA               EQU     0x001C
OMAP5912_EMIF_EMIFF_CONFIG_REGS_OA              EQU     0x0020
OMAP5912_EMIF_EMIFF_MRS_REGS_OA                 EQU     0x0024
OMAP5912_EMIF_EMIFS_TIMEOUT1_REGS_OA            EQU     0x0028
OMAP5912_EMIF_EMIFS_TIMEOUT2_REGS_OA            EQU     0x002C
OMAP5912_EMIF_EMIFS_TIMEOUT3_REGS_OA            EQU     0x0030
OMAP5912_EMIF_ENDIANISM_CONTROL_REGS_OA         EQU     0x0034
OMAP5912_EMIF_RESERVED_0038_REGS_OA             EQU     0x0038
OMAP5912_EMIF_EMIFF_CONFIG2_REGS_OA             EQU     0x003C
OMAP5912_EMIF_EMIFS_FL_CFG_DYN_WAIT_REGS_OA     EQU     0x0040
OMAP5912_EMIF_EMIFS_ABORT_ADDR_REGS_OA          EQU     0x0044
OMAP5912_EMIF_EMIFS_ABORT_TYPE_REGS_OA          EQU     0x0048
OMAP5912_EMIF_EMIFS_ABORT_TOUT_REGS_OA          EQU     0x004C
OMAP5912_EMIF_EMIFS_ACFG_0_I_REGS_OA            EQU     0x0050
OMAP5912_EMIF_EMIFS_ACFG_1_I_REGS_OA            EQU     0x0054
OMAP5912_EMIF_EMIFS_ACFG_2_I_REGS_OA            EQU     0x0058
OMAP5912_EMIF_EMIFS_ACFG_3_I_REGS_OA            EQU     0x005C
OMAP5912_EMIF_EMIFF_DOUBLER_EN_REGS_OA          EQU     0x0060
OMAP5912_EMIF_EMIFF_DLL_WRT_CONTROL_REGS_OA     EQU     0x0064
OMAP5912_EMIF_EMIFF_DLL_WRT_STATUS_REGS_OA      EQU     0x0068
OMAP5912_EMIF_RESERVED_006C_REGS_OA             EQU     0x006C
OMAP5912_EMIF_EMIFF_MRS_NEW_REGS_OA             EQU     0x0070
OMAP5912_EMIF_EMIFF_EMRS0_REGS_OA               EQU     0x0074
OMAP5912_EMIF_EMIFF_EMRS1_REGS_OA               EQU     0x0078
OMAP5912_EMIF_RESERVED_007C_REGS_OA             EQU     0x007C
OMAP5912_EMIF_EMIFF_OPERATION_REGS_OA           EQU     0x0080
OMAP5912_EMIF_EMIFF_MANUAL_CMD_REGS_OA          EQU     0x0084
OMAP5912_EMIF_RESERVED_0088_REGS_OA             EQU     0x0088
OMAP5912_EMIF_EMIFF_TIMEOUT1_REGS_OA            EQU     0x008C
OMAP5912_EMIF_EMIFF_TIMEOUT2_REGS_OA            EQU     0x0090
OMAP5912_EMIF_EMIFF_TIMEOUT3_REGS_OA            EQU     0x0094
OMAP5912_EMIF_EMIFF_ABORT_ADDRESS_REGS_OA       EQU     0x0098
OMAP5912_EMIF_EMIFF_ABORT_TYPE_REGS_OA          EQU     0x009C
OMAP5912_EMIF_OCPT1_PTOR1_REGS_OA               EQU     0x00A0
OMAP5912_EMIF_OCPT1_PTOR2_REGS_OA               EQU     0x00A4
OMAP5912_EMIF_OCPT1_PTOR3_REGS_OA               EQU     0x00A8
OMAP5912_EMIF_OCPT1_ATOR_REGS_OA                EQU     0x00AC
OMAP5912_EMIF_OCPT1_AADDR_REGS_OA               EQU     0x00B0
OMAP5912_EMIF_OCPT1_ATYPER_REGS_OA              EQU     0x00B4
OMAP5912_EMIF_OCPT_CONFIG_REGS_OA               EQU     0x00B8
OMAP5912_EMIF_EMIFF_DLL_LRD_STATUS_REGS_OA      EQU     0x00BC
OMAP5912_EMIF_EMIFF_DLL_URD_CONTROL_REGS_OA     EQU     0x00C0
OMAP5912_EMIF_EMIFF_DLL_URD_STATUS_REGS_OA      EQU     0x00C4
OMAP5912_EMIF_EMIFF_EMRS2_REGS_OA               EQU     0x00C8
OMAP5912_EMIF_EMIFF_DLL_LRD_CONTROL_REGS_OA     EQU     0x00CC
OMAP5912_EMIF_OCPT2_PRIOR_REGS_OA               EQU     0x00D0
OMAP5912_EMIF_OCPT2_PTOR1_REGS_OA               EQU     0x00D4
OMAP5912_EMIF_OCPT2_PTOR2_REGS_OA               EQU     0x00D8
OMAP5912_EMIF_OCPT2_PTOR3_REGS_OA               EQU     0x00DC
OMAP5912_EMIF_OCPT2_ATOR_REGS_OA                EQU     0x00E0
OMAP5912_EMIF_OCPT2_AADDR_REGS_OA               EQU     0x00E4
OMAP5912_EMIF_OCPT2_ATYPER_REGS_OA              EQU     0x00E8

;------------------------------------------------------------------------------

                                                END
