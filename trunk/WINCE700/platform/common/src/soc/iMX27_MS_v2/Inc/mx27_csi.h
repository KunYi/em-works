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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Header:  mx27_csi.h
//
//  Provides definitions for SLCDC module based on i.MX27.
//
//------------------------------------------------------------------------------
#ifndef __MX27_CSI_H
#define __MX27_CSI_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {
	REG32 CSICR1;
	REG32 CSICR2;
	REG32 CSISR;
	REG32 CSISTATFIFO;
	REG32 CSIRXFIFO;
	REG32 CSIRXCNT;
	REG32 CSIDBGREG;
	REG32 CSICR3;
} CSP_CSI_REGS, *PCSP_CSI_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define CSI_CSICR1_OFFSET	0x0000    
#define CSI_CSICR2_OFFSET	0x0004    
#define CSI_CSISR_OFFSET	0x0008
#define CSI_CSISTATFIFO_OFFSET	0x000C
#define CSI_CSIRXFIFO_OFFSET	0x0010
#define CSI_CSIRXCNT_OFFSET	0x0014
#define CSI_CSIDBGREG_OFFSET	0X0018
#define CSI_CSICR3_OFFSET	0X001C                                      
        
//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
// CSICR1 : CSI Control Register 1
#define CSI_CSICR1_REDGE_LSH		1
#define CSI_CSICR1_INV_PCLK_LSH		2
#define CSI_CSICR1_INV_DATA_LSH 	3
#define CSI_CSICR1_GCLK_MODE_LSH	4
#define CSI_CSICR1_CLR_RXFIFO_LSH	5
#define CSI_CSICR1_CLR_STATFIFO_LSH	6
#define CSI_CSICR1_PACK_DIR_LSH		7
#define CSI_CSICR1_FCC_LSH		8
#define CSI_CSICR1_MCLKEN_LSH		9
#define CSI_CSICR1_CCIR_EN_LSH		10
#define CSI_CSICR1_HSYNC_POL_LSH	11
#define CSI_CSICR1_MCLKDIV_LSH		12
#define CSI_CSICR1_SOF_INTEN_LSH	16
#define CSI_CSICR1_SOF_POL_LSH		17
#define CSI_CSICR1_RXFF_INTEN_LSH	18
#define CSI_CSICR1_RXFF_LEVEL_LSH	19
#define CSI_CSICR1_STATFF_INTEN_LSH	21
#define CSI_CSICR1_STATFF_LEVEL_LSH	22
#define CSI_CSICR1_RF_OR_INTEN_LSH	24
#define CSI_CSICR1_SF_OR_INTEN_LSH	25
#define CSI_CSICR1_COF_INT_EN_LSH	26
#define CSI_CSICR1_CCIR_MODE_LSH	27
#define CSI_CSICR1_PRP_IF_EN_LSH	28
#define CSI_CSICR1_EOF_INT_EN_LSH	29
#define CSI_CSICR1_EXT_VSYNC_LSH	30
#define CSI_CSICR1_SWAP16_EN_LSH	31


//CSICR2 : CSI Control Register 2
#define CSI_CSICR2_HSC_LSH		0
#define CSI_CSICR2_VSC_LSH		8
#define CSI_CSICR2_LVRM_LSH		16
#define CSI_CSICR2_BTS_LSH		19
#define CSI_CSICR2_SCE_LSH		23
#define CSI_CSICR2_AFS_LSH		24
#define CSI_CSICR2_DRM_LSH		26


//CSISR :  CSI Status Register
#define CSI_CSISR_DRDY_LSH		0
#define CSI_CSISR_ECC_INT_LSH		1
#define CSI_CSISR_COF_INT_LSH		13
#define CSI_CSISR_F1_INT_LSH		14
#define CSI_CSISR_F2_INT_LSH		15
#define CSI_CSISR_SOF_INT_LSH		16
#define CSI_CSISR_EOF_INT_LSH		17
#define CSI_CSISR_RXFF_INT_LSH		18
#define CSI_CSISR_STATFF_INT_LSH	21
#define CSI_CSISR_RF_OR_INT_LSH		24
#define CSI_CSISR_SF_OR_INT_LSH		25

//CSISTATFIFO : CSI Statistic FIFO Register
#define CSI_CSISTATFIFO_STAT_LSH	0

//CSIRXFIFO : CSI RxFIFO Register
#define CSI_CSIRXFIFO_IMAGE_LSH		0

//CSIRXCNT :  CSI RxFIFO Count Register
#define CSI_CSIRXCNT_RXCNT_LSH		0

//CSIDBGREG : CSI Debug Register
#define CSI_CSIDBGREG_DEBUG_SIGNAL_EN_LSH	0

//CSICR3 : CSI Control Register 3
#define CSI_CSICR3_ECC_AUTO_EN_LSH	0
#define CSI_CSICR3_ECC_INT_EN_LSH	1
#define CSI_CSICR3_ZERO_PACK_EN_LSH	2
#define CSI_CSICR3_CSI_SVR_LSH		3
#define CSI_CSICR3_FRMCNT_RST_LSH	15
#define CSI_CSICR3_FRMCNT_LSH		16

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
// CSICR1 : CSI Control Register 1
#define CSI_CSICR1_REDGE_WID		1
#define CSI_CSICR1_INV_PCLK_WID		1
#define CSI_CSICR1_INV_DATA_WID 	1
#define CSI_CSICR1_GCLK_MODE_WID	1
#define CSI_CSICR1_CLR_RXFIFO_WID	1
#define CSI_CSICR1_CLR_STATFIFO_WID	1
#define CSI_CSICR1_PACK_DIR_WID		1
#define CSI_CSICR1_FCC_WID		1
#define CSI_CSICR1_MCLKEN_WID		1
#define CSI_CSICR1_CCIR_EN_WID		1
#define CSI_CSICR1_HSYNC_POL_WID	1
#define CSI_CSICR1_MCLKDIV_WID		4
#define CSI_CSICR1_SOF_INTEN_WID	1
#define CSI_CSICR1_SOF_POL_WID		1
#define CSI_CSICR1_RXFF_INTEN_WID	1
#define CSI_CSICR1_RXFF_LEVEL_WID	2
#define CSI_CSICR1_STATFF_INTEN_WID	1
#define CSI_CSICR1_STATFF_LEVEL_WID	2
#define CSI_CSICR1_RF_OR_INTEN_WID	1
#define CSI_CSICR1_SF_OR_INTEN_WID	1
#define CSI_CSICR1_COF_INT_EN_WID	1
#define CSI_CSICR1_CCIR_MODE_WID	1
#define CSI_CSICR1_PRP_IF_EN_WID	1
#define CSI_CSICR1_EOF_INT_EN_WID	1
#define CSI_CSICR1_EXT_VSYNC_WID	1
#define CSI_CSICR1_SWAP16_EN_WID	1


//CSICR2 : CSI Control Register 2
#define CSI_CSICR2_HSC_WID		8
#define CSI_CSICR2_VSC_WID		8
#define CSI_CSICR2_LVRM_WID		3
#define CSI_CSICR2_BTS_WID		2
#define CSI_CSICR2_SCE_WID		1
#define CSI_CSICR2_AFS_WID		2
#define CSI_CSICR2_DRM_WID		1


//CSISR :  CSI Status Register
#define CSI_CSISR_DRDY_WID		1
#define CSI_CSISR_ECC_INT_WID		1
#define CSI_CSISR_COF_INT_WID		1
#define CSI_CSISR_F1_INT_WID		1
#define CSI_CSISR_F2_INT_WID		1
#define CSI_CSISR_SOF_INT_WID		1
#define CSI_CSISR_EOF_INT_WID		1
#define CSI_CSISR_RXFF_INT_WID		1
#define CSI_CSISR_STATFF_INT_WID	1
#define CSI_CSISR_RF_OR_INT_WID		1
#define CSI_CSISR_SF_OR_INT_WID		1

//CSISTATFIFO : CSI Statistic FIFO Register
#define CSI_CSISTATFIFO_STAT_WID	32

//CSIRXFIFO : CSI RxFIFO Register
#define CSI_CSIRXFIFO_IMAGE_WID		32

//CSIRXCNT :  CSI RxFIFO Count Register
#define CSI_CSIRXCNT_RXCNT_WID		22

//CSIDBGREG : CSI Debug Register
#define CSI_CSIDBGREG_DEBUG_SIGNAL_EN_WID	1

//CSICR3 : CSI Control Register 3
#define CSI_CSICR3_ECC_AUTO_EN_WID	1
#define CSI_CSICR3_ECC_INT_EN_WID	1
#define CSI_CSICR3_ZERO_PACK_EN_WID	1
#define CSI_CSICR3_CSI_SVR_WID		1
#define CSI_CSICR3_FRMCNT_RST_WID	1
#define CSI_CSICR3_FRMCNT_WID		16


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
// CSICR1 : CSI Control Register 1
#define CSI_CSICR1_REDGE_RISING		1
#define CSI_CSICR1_REDGE_FALLING	0

#define CSI_CSICR1_INV_PCLK_DIRECT	0
#define CSI_CSICR1_INV_PCLK_INVERT	1

#define CSI_CSICR1_INV_DATA_DIRECT	0
#define CSI_CSICR1_INV_DATA_INVERT	1

#define CSI_CSICR1_GCLK_MODE_NONGATED	0
#define CSI_CSICR1_GCLK_MODE_GATED	1

#define CSI_CSICR1_CLR_RXFIFO_CLEAR	1

#define CSI_CSICR1_CLR_STATFIFO_CLEAR	1

#define CSI_CSICR1_PACK_DIR_LSB		0
#define CSI_CSICR1_PACK_DIR_MSB		1

#define CSI_CSICR1_FCC_ASYNC		0
#define CSI_CSICR1_FCC_SYNC		1

#define CSI_CSICR1_MCLKEN_DISABLE	0
#define CSI_CSICR1_MCLKEN_ENABLE	1

#define CSI_CSICR1_CCIR_EN_TRADITIONAL	0
#define CSI_CSICR1_CCIR_EN_CCIR		1

#define CSI_CSICR1_HSYNC_POL_LOW	0
#define CSI_CSICR1_HSYNC_POL_HIGN	1

#define CSI_CSICR1_MCLKDIV_VALUE(X)		((X)/2-1)

#define CSI_CSICR1_SOF_INTEN_DISABLE	0
#define CSI_CSICR1_SOF_INTEN_ENABLE	1

#define CSI_CSICR1_SOF_POL_FALLING	0
#define CSI_CSICR1_SOF_POL_RISING	1

#define CSI_CSICR1_RXFF_INTEN_DISABLE	0
#define CSI_CSICR1_RXFF_INTEN_ENABLE	1

#define CSI_CSICR1_RXFF_LEVEL_4WORDS	0
#define CSI_CSICR1_RXFF_LEVEL_8WORDS	1
#define CSI_CSICR1_RXFF_LEVEL_16WORDS	2
#define CSI_CSICR1_RXFF_LEVEL_24WORDS	3

#define CSI_CSICR1_STATFF_INTEN_DISABLE	0
#define CSI_CSICR1_STATFF_INTEN_ENABLE	1

#define CSI_CSICR1_STATFF_LEVEL_4WORDS	0
#define CSI_CSICR1_STATFF_LEVEL_8WORDS	1
#define CSI_CSICR1_STATFF_LEVEL_12WORDS	2
#define CSI_CSICR1_STATFF_LEVEL_16WORDS	3

#define CSI_CSICR1_RF_OR_INTEN_DISABLE	0
#define CSI_CSICR1_RF_OR_INTEN_ENABLE	1

#define CSI_CSICR1_SF_OR_INTEN_DISABLE	0
#define CSI_CSICR1_SF_OR_INTEN_ENABLE	1

#define CSI_CSICR1_COF_INT_EN_DISABLE	0
#define CSI_CSICR1_COF_INT_EN_ENABLE	1

#define CSI_CSICR1_CCIR_MODE_PROGRESSIVE 0
#define CSI_CSICR1_CCIR_MODE_INTERLACE	 1

#define CSI_CSICR1_PRP_IF_EN_DISABLE	0
#define CSI_CSICR1_PRP_IF_EN_ENABLE	1

#define CSI_CSICR1_EOF_INT_EN_DISABLE	0
#define CSI_CSICR1_EOF_INT_EN_ENABLE	1

#define CSI_CSICR1_EXT_VSYNC_INTERNAL	0
#define CSI_CSICR1_EXT_VSYNC_EXTERNAL	1

#define CSI_CSICR1_SWAP16_EN_DISABLE	0
#define CSI_CSICR1_SWAP16_EN_ENABLE	1


//CSICR2 : CSI Control Register 2
#define CSI_CSICR2_HSC_VALUE(X)		(X)
#define CSI_CSICR2_VSC_VALUE(X)		(X)
#define CSI_CSICR2_LVRM_512X384		0
#define CSI_CSICR2_LVRM_448X336		1
#define CSI_CSICR2_LVRM_384X288		2
#define CSI_CSICR2_LVRM_384X256		3
#define CSI_CSICR2_LVRM_320X240		4
#define CSI_CSICR2_LVRM_288X216		5
#define CSI_CSICR2_LVRM_400X300		6

#define CSI_CSICR2_BTS_GR		0
#define CSI_CSICR2_BTS_RG		1
#define CSI_CSICR2_BTS_BG		2
#define CSI_CSICR2_BTS_GB		3

#define CSI_CSICR2_SCE_DISABLE		0
#define CSI_CSICR2_SCE_ENABLE		1

#define CSI_CSICR2_AFS_CONSECUTIVE	0
#define CSI_CSICR2_AFS_EVERYOTHER	1
#define CSI_CSICR2_AFS_EVERYFOUR	2 // 3 also OK

#define CSI_CSICR2_DRM_8X6		0
#define CSI_CSICR2_DRM_8X12		1


//CSISR :  CSI Status Register
#define CSI_CSISR_ECC_INT_W1L		1
#define CSI_CSISR_COF_INT_W1L		1
#define CSI_CSISR_SOF_INT_W1L		1
#define CSI_CSISR_EOF_INT_W1L		1
#define CSI_CSISR_RF_OR_INT_W1L		1
#define CSI_CSISR_SF_OR_INT_W1L		1

//CSISTATFIFO : CSI Statistic FIFO Register

//CSIRXFIFO : CSI RxFIFO Register

//CSIRXCNT :  CSI RxFIFO Count Register
#define CSI_CSIRXCNT_RXCNT_VALUE(X)		(X)

//CSIDBGREG : CSI Debug Register
#define CSI_CSIDBGREG_DEBUG_SIGNAL_EN_DISABLE	0
#define CSI_CSIDBGREG_DEBUG_SIGNAL_EN_ENABLE	1

//CSICR3 : CSI Control Register 3
#define CSI_CSICR3_ECC_AUTO_EN_DISABLE	0
#define CSI_CSICR3_ECC_AUTO_EN_ENABLE	1

#define CSI_CSICR3_ECC_INT_EN_DISABLE	0
#define CSI_CSICR3_ECC_INT_EN_ENABLE	1

#define CSI_CSICR3_CSI_SVR_ANYMODE		0
#define CSI_CSICR3_CSI_SVR_SVRMODE		1

#define CSI_CSICR3_ZERO_PACK_EN_DISABLE	0
#define CSI_CSICR3_ZERO_PACK_EN_ENABLE	1

#define CSI_CSICR3_FRMCNT_RST_RESET	1
#define CSI_CSICR3_FRMCNT_RST_UNRESET	0

#define CSI_CSICR3_FRMCNT_VALUE(X)	(X)

#ifdef __cplusplus
}
#endif

#endif // __MX27_CSI_H
