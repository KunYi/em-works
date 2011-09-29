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
;;------------------------------------------------------------------------------
;;
;;  Copyright(c) 1998,1999 Renesas Technology Corp.
;;
;;------------------------------------------------------------------------------
;;
;;	File:   sh4_dmac.inc
;;
;;------------------------------------------------------------------------------

;
; DMAC registers
;

DMAC_REGBASE                .equ    h'FFA00000
DMAC_REGSIZE                .equ    h'0044

DMAC_SAR0_OFFSET            .equ    h'0000
DMAC_DAR0_OFFSET            .equ    h'0004
DMAC_DMATCR0_OFFSET         .equ    h'0008
DMAC_CHCR0_OFFSET           .equ    h'000C

DMAC_SAR1_OFFSET            .equ    h'0010
DMAC_DAR1_OFFSET            .equ    h'0014
DMAC_DMATCR1_OFFSET         .equ    h'0018
DMAC_CHCR1_OFFSET           .equ    h'001C

DMAC_SAR2_OFFSET            .equ    h'0020
DMAC_DAR2_OFFSET            .equ    h'0024
DMAC_DMATCR2_OFFSET         .equ    h'0028
DMAC_CHCR2_OFFSET           .equ    h'002C

DMAC_SAR3_OFFSET            .equ    h'0030
DMAC_DAR3_OFFSET            .equ    h'0034
DMAC_DMATCR3_OFFSET         .equ    h'0038
DMAC_CHCR3_OFFSET           .equ    h'003C

DMAC_DMAOR_OFFSET           .equ    h'0040
                    
DMAC_SAR0                   .equ    (DMAC_REGBASE + DMAC_SAR0_OFFSET)
DMAC_DAR0                   .equ    (DMAC_REGBASE + DMAC_DAR0_OFFSET)
DMAC_DMATCR0                .equ    (DMAC_REGBASE + DMAC_DMATCR0_OFFSET)
DMAC_CHCR0                  .equ    (DMAC_REGBASE + DMAC_CHCR0_OFFSET)

DMAC_SAR1                   .equ    (DMAC_REGBASE + DMAC_SAR1_OFFSET)
DMAC_DAR1                   .equ    (DMAC_REGBASE + DMAC_DAR1_OFFSET)
DMAC_DMATCR1                .equ    (DMAC_REGBASE + DMAC_DMATCR1_OFFSET)
DMAC_CHCR1                  .equ    (DMAC_REGBASE + DMAC_CHCR1_OFFSET)

DMAC_SAR2                   .equ    (DMAC_REGBASE + DMAC_SAR2_OFFSET)
DMAC_DAR2                   .equ    (DMAC_REGBASE + DMAC_DAR2_OFFSET)
DMAC_DMATCR2                .equ    (DMAC_REGBASE + DMAC_DMATCR2_OFFSET)
DMAC_CHCR2                  .equ    (DMAC_REGBASE + DMAC_CHCR2_OFFSET)

DMAC_SAR3                   .equ    (DMAC_REGBASE + DMAC_SAR3_OFFSET)
DMAC_DAR3                   .equ    (DMAC_REGBASE + DMAC_DAR3_OFFSET)
DMAC_DMATCR3                .equ    (DMAC_REGBASE + DMAC_DMATCR3_OFFSET)
DMAC_CHCR3                  .equ    (DMAC_REGBASE + DMAC_CHCR3_OFFSET)

DMAC_DMAOR                  .equ    (DMAC_REGBASE + DMAC_DMAOR_OFFSET)

;    DMA channel control register (DMAC_CHCR0 to 3)

DMAC_CHCR_SSA_RESERVED      .equ    h'00000000
DMAC_CHCR_SSA_DYNAMIC_IO    .equ    h'20000000
DMAC_CHCR_SSA_8_IO          .equ    h'40000000
DMAC_CHCR_SSA_16_IO         .equ    h'60000000
DMAC_CHCR_SSA_8_COMM        .equ    h'80000000
DMAC_CHCR_SSA_16_COMM       .equ    h'A0000000
DMAC_CHCR_SSA_8_ATTR        .equ    h'C0000000
DMAC_CHCR_SSA_16_ATTR       .equ    h'E0000000

DMAC_CHCR_STC_CS5           .equ    h'00000000
DMAC_CHCR_STC_CS6           .equ    h'10000000

DMAC_CHCR_DSA_RESERVED      .equ    h'00000000
DMAC_CHCR_DSA_DYNAMIC_IO    .equ    h'02000000
DMAC_CHCR_DSA_8_IO          .equ    h'04000000
DMAC_CHCR_DSA_16_IO         .equ    h'06000000
DMAC_CHCR_DSA_8_COMM        .equ    h'08000000
DMAC_CHCR_DSA_16_COMM       .equ    h'0A000000
DMAC_CHCR_DSA_8_ATTR        .equ    h'0C000000
DMAC_CHCR_DSA_16_ATTR       .equ    h'0E000000

DMAC_CHCR_DTC_CS5           .equ    h'00000000
DMAC_CHCR_DTC_CS6           .equ    h'01000000

DMAC_CHCR_DS_LOW_LEVEL      .equ    h'00000000
DMAC_CHCR_DS_FALLING_EDGE   .equ    h'00080000

DMAC_CHCR_RL_ACTIVE_HIGH    .equ    h'00000000
DMAC_CHCR_RL_ACTIVE_LOW     .equ    h'00040000

DMAC_CHCR_AM_READ_CYCLE     .equ    h'00000000
DMAC_CHCR_AM_WRITE_CYCLE    .equ    h'00020000

DMAC_CHCR_AL_ACTIVE_HIGH    .equ    h'00000000
DMAC_CHCR_AL_ACTIVE_LOW     .equ    h'00010000

DMAC_CHCR_DM_FIXED          .equ    h'00000000
DMAC_CHCR_DM_INCREMENTED    .equ    h'00004000
DMAC_CHCR_DM_DECREMENTED    .equ    h'00008000

DMAC_CHCR_SM_FIXED          .equ    h'00000000
DMAC_CHCR_SM_INCREMENTED    .equ    h'00001000
DMAC_CHCR_SM_DECREMENTED    .equ    h'00002000

DMAC_CHCR_RS_EX_DAM         .equ    h'00000000
DMAC_CHCR_RS_EX_SAM_EAS_ED  .equ    h'00000200
DMAC_CHCR_RS_EX_SAM_ED_EAS  .equ    h'00000300
DMAC_CHCR_RS_AUTO_EAS_EAS   .equ    h'00000400
DMAC_CHCR_RS_AUTO_EAS_OCP   .equ    h'00000500
DMAC_CHCR_RS_AUTO_OCP_EAS   .equ    h'00000600
DMAC_CHCR_RS_SCI1_EMPTY     .equ    h'00000800
DMAC_CHCR_RS_SCI1_FULL      .equ    h'00000900
DMAC_CHCR_RS_SCI2_EMPTY     .equ    h'00000A00
DMAC_CHCR_RS_SCI2_FULL      .equ    h'00000B00
DMAC_CHCR_RS_TMU2_EAS_EAS   .equ    h'00000C00
DMAC_CHCR_RS_TMU2_EAS_OCP   .equ    h'00000D00
DMAC_CHCR_RS_TMU2_OCP_EAS   .equ    h'00000E00

DMAC_CHCR_TM_CYCLE_STEAL    .equ    h'00000000
DMAC_CHCR_TM_BURST          .equ    h'00000080

DMAC_CHCR_TS_64             .equ    h'00000000
DMAC_CHCR_TS_8              .equ    h'00000010
DMAC_CHCR_TS_16             .equ    h'00000020
DMAC_CHCR_TS_32             .equ    h'00000030
DMAC_CHCR_TS_32_BT          .equ    h'00000040

DMAC_CHCR_IE_NOT_GENARATED  .equ    h'00000000
DMAC_CHCR_IE_GENERATED      .equ    h'00000004

DMAC_CHCR_TE_DMATCR_INCOMP  .equ    h'00000000
DMAC_CHCR_TE_DMATCR_COMP    .equ    h'00000002

DMAC_CHCR_DE_DISABLED       .equ    h'00000000
DMAC_CHCR_DE_ENABLED        .equ    h'00000001

DMAC_DMAOR_DDT              .equ    h'00008000
DMAC_DMAOR_PR00             .equ    h'00000000
DMAC_DMAOR_PR01             .equ    h'00000100
DMAC_DMAOR_PR10             .equ    h'00000200
DMAC_DMAOR_PR11             .equ    h'00000300
DMAC_DMAOR_AE               .equ    h'00000004
DMAC_DMAOR_NMFI             .equ    h'00000002
DMAC_DMAOR_DME              .equ    h'00000001

