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
/****************************************************************************** 
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
********************************************************************************/
#include "xllp_defs.h"
#include "xllp_serialization.h"
#include "xllp_ost.h"
#include "xllp_ci.h"
#include "xllp_clkmgr.h"
#include <stdlib.h>

//-------------------------------------------------------------------------------------------------------
//      Declarations
//-------------------------------------------------------------------------------------------------------
#define READ_REG(offset)         (*(unsigned int*)((unsigned)ci_reg_base + (offset)))
#define WRITE_REG(offset, value) *(unsigned int*)((unsigned)ci_reg_base + (offset)) = (value)

//-------------------------------------------------------------------------------------------------------
//      Configuration APIs
//-------------------------------------------------------------------------------------------------------
void XllpCISetFrameRate(unsigned int ci_reg_base, XLLP_CI_FRAME_CAPTURE_RATE frate)
{
    unsigned int value;
    
    // write cicr4
    value = READ_REG(XLLP_CICR4);
    value &= ~(XLLP_CI_CICR4_FR_RATE_SMASK << XLLP_CI_CICR4_FR_RATE_SHIFT);
    value |= (unsigned)frate << XLLP_CI_CICR4_FR_RATE_SHIFT;
    WRITE_REG(XLLP_CICR4, value);   
}

XLLP_CI_FRAME_CAPTURE_RATE XllpCIGetFrameRate(unsigned int ci_reg_base)
{
    unsigned int value;
    value = READ_REG(XLLP_CICR4);
    return (XLLP_CI_FRAME_CAPTURE_RATE)((value >> XLLP_CI_CICR4_FR_RATE_SHIFT) & XLLP_CI_CICR4_FR_RATE_SMASK);
}

void XllpCISetImageFormat(unsigned int ci_reg_base, XLLP_CI_IMAGE_FORMAT input_format, XLLP_CI_IMAGE_FORMAT output_format)
{
    unsigned int value, tbit, rgbt_conv, rgb_conv, rgb_f, ycbcr_f, rgb_bpp, raw_bpp, cspace;

    // write cicr1: preserve ppl value and data width value
    value = READ_REG(XLLP_CICR1);
    value &= ( (XLLP_CI_CICR1_PPL_SMASK << XLLP_CI_CICR1_PPL_SHIFT) | ((XLLP_CI_CICR1_DW_SMASK) << XLLP_CI_CICR1_DW_SHIFT));
    tbit = rgbt_conv = rgb_conv = rgb_f = ycbcr_f = rgb_bpp = raw_bpp = cspace = 0;
    switch(input_format) {
    case XLLP_CI_RAW8:
        cspace = 0;
        raw_bpp = 0;
        break;
    case XLLP_CI_RAW9:
        cspace = 0;
        raw_bpp = 1;
        break;
    case XLLP_CI_RAW10:
        cspace = 0;
        raw_bpp = 2;
        break;
    case XLLP_CI_YCBCR422:
    case XLLP_CI_YCBCR422_PLANAR:
        cspace = 2;
        if (output_format == XLLP_CI_YCBCR422_PLANAR) {
            ycbcr_f = 1;
        }
        break;
    case XLLP_CI_RGB444:
        cspace = 1;
        rgb_bpp = 0;
        break;  
    case XLLP_CI_RGB555:
        cspace = 1;
        rgb_bpp = 1;
        if (output_format == XLLP_CI_RGBT555_0) {
            rgbt_conv = 2;
            tbit = 0;
        } 
        else if (output_format == XLLP_CI_RGBT555_1) {
            rgbt_conv = 2;
            tbit = 1;
        }
        break;  
    case XLLP_CI_RGB565:
        cspace = 1;
        rgb_bpp = 2;
        rgb_f = 1;
        break;  
    case XLLP_CI_RGB666:
        cspace = 1;
        rgb_bpp = 3;
        if (output_format == XLLP_CI_RGB666_PACKED) {
            rgb_f = 1;
        }
        break;  
    case XLLP_CI_RGB888:
    case XLLP_CI_RGB888_PACKED:
        cspace = 1;
        rgb_bpp = 4;
        switch(output_format) {
        case XLLP_CI_RGB888_PACKED:
            rgb_f = 1;
            break;
        case XLLP_CI_RGBT888_0:
            rgbt_conv = 1;
            tbit = 0;
            break;
        case XLLP_CI_RGBT888_1:
            rgbt_conv = 1;
            tbit = 1;
            break;
        case XLLP_CI_RGB666:
            rgb_conv = 1;
            break;
        case XLLP_CI_RGB565:
            rgb_conv = 2;
            break;
        case XLLP_CI_RGB555:
            rgb_conv = 3;
            break;
        case XLLP_CI_RGB444:
            rgb_conv = 4;
            break;
        default:
            break;
        }
        break;  
    default:
        break;
    }
    value |= (tbit==1) ? XLLP_CI_CICR1_TBIT : 0;
    value |= rgbt_conv << XLLP_CI_CICR1_RGBT_CONV_SHIFT;
    value |= rgb_conv << XLLP_CI_CICR1_RGB_CONV_SHIFT;
    value |= (rgb_f==1) ? XLLP_CI_CICR1_RBG_F : 0;
    value |= (ycbcr_f==1) ? XLLP_CI_CICR1_YCBCR_F : 0;
    value |= rgb_bpp << XLLP_CI_CICR1_RGB_BPP_SHIFT;
    value |= raw_bpp << XLLP_CI_CICR1_RAW_BPP_SHIFT;
    value |= cspace << XLLP_CI_CICR1_COLOR_SP_SHIFT;
    WRITE_REG(XLLP_CICR1, value);   

    return; 
}

void XllpCISetMode(unsigned int ci_reg_base, XLLP_CI_MODE mode, XLLP_CI_DATA_WIDTH data_width)
{
    unsigned int value;

    // write mode field in cicr0    
    value = READ_REG(XLLP_CICR0);
    value &= ~(XLLP_CI_CICR0_SIM_SMASK << XLLP_CI_CICR0_SIM_SHIFT);
    value |= (unsigned int)mode << XLLP_CI_CICR0_SIM_SHIFT;
    WRITE_REG(XLLP_CICR0, value);   
    
    // write data width cicr1
    value = READ_REG(XLLP_CICR1);
    value &= ~(XLLP_CI_CICR1_DW_SMASK << XLLP_CI_CICR1_DW_SHIFT);
    value |= ((unsigned)data_width) << XLLP_CI_CICR1_DW_SHIFT;
    WRITE_REG(XLLP_CICR1, value);   
    return; 
}

void XllpCIConfigureMP(unsigned int ci_reg_base, unsigned int PPL, unsigned int LPF, XLLP_CI_MP_TIMING* timing)
{
    unsigned int value;
    
    // write ppl field in cicr1
    value = READ_REG(XLLP_CICR1);
    value &= ~(XLLP_CI_CICR1_PPL_SMASK << XLLP_CI_CICR1_PPL_SHIFT);
    value |= (PPL & XLLP_CI_CICR1_PPL_SMASK) << XLLP_CI_CICR1_PPL_SHIFT;
    WRITE_REG(XLLP_CICR1, value);   

    // write BLW, ELW in cicr2  
    value = READ_REG(XLLP_CICR2);
    value &= ~(XLLP_CI_CICR2_BLW_SMASK << XLLP_CI_CICR2_BLW_SHIFT | XLLP_CI_CICR2_ELW_SMASK << XLLP_CI_CICR2_ELW_SHIFT );
    value |= (timing->BLW & XLLP_CI_CICR2_BLW_SMASK) << XLLP_CI_CICR2_BLW_SHIFT;
    WRITE_REG(XLLP_CICR2, value);   
    
    // write BFW, LPF in cicr3
    value = READ_REG(XLLP_CICR3);
    value &= ~(XLLP_CI_CICR3_BFW_SMASK << XLLP_CI_CICR3_BFW_SHIFT | XLLP_CI_CICR3_LPF_SMASK << XLLP_CI_CICR3_LPF_SHIFT );
    value |= (timing->BFW & XLLP_CI_CICR3_BFW_SMASK) << XLLP_CI_CICR3_BFW_SHIFT;
    value |= (LPF & XLLP_CI_CICR3_LPF_SMASK) << XLLP_CI_CICR3_LPF_SHIFT;
    WRITE_REG(XLLP_CICR3, value);   
    return;
}

void XllpCIConfigureSP(unsigned int ci_reg_base, unsigned int PPL, unsigned int LPF, XLLP_CI_SP_TIMING* timing)
{
    unsigned int value;
    
    // write ppl field in cicr1
    value = READ_REG(XLLP_CICR1);
    value &= ~(XLLP_CI_CICR1_PPL_SMASK << XLLP_CI_CICR1_PPL_SHIFT);
    value |= (PPL & XLLP_CI_CICR1_PPL_SMASK) << XLLP_CI_CICR1_PPL_SHIFT;
    WRITE_REG(XLLP_CICR1, value);   

    // write cicr2
    value |= (timing->BLW & XLLP_CI_CICR2_BLW_SMASK) << XLLP_CI_CICR2_BLW_SHIFT;
    value |= (timing->ELW & XLLP_CI_CICR2_ELW_SMASK) << XLLP_CI_CICR2_ELW_SHIFT;
    value |= (timing->HSW & XLLP_CI_CICR2_HSW_SMASK) << XLLP_CI_CICR2_HSW_SHIFT;
    value |= (timing->BFPW & XLLP_CI_CICR2_BFPW_SMASK) << XLLP_CI_CICR2_BFPW_SHIFT;
    value |= (timing->FSW & XLLP_CI_CICR2_FSW_SMASK) << XLLP_CI_CICR2_FSW_SHIFT;
    WRITE_REG(XLLP_CICR2, value);   
    
    // write cicr3
    value |= (timing->BFW & XLLP_CI_CICR3_BFW_SMASK) << XLLP_CI_CICR3_BFW_SHIFT;
    value |= (timing->EFW & XLLP_CI_CICR3_EFW_SMASK) << XLLP_CI_CICR3_EFW_SHIFT;
    value |= (timing->VSW & XLLP_CI_CICR3_VSW_SMASK) << XLLP_CI_CICR3_VSW_SHIFT;
    value |= (LPF & XLLP_CI_CICR3_LPF_SMASK) << XLLP_CI_CICR3_LPF_SHIFT;
    WRITE_REG(XLLP_CICR3, value);   
    return;
}

void XllpCIConfigureMS(unsigned int ci_reg_base, unsigned int PPL, unsigned int LPF, XLLP_CI_MS_TIMING* timing)
{
    // the operation is same as Master-Parallel
    XllpCIConfigureMP(ci_reg_base, PPL, LPF, (XLLP_CI_MP_TIMING*)timing);
}
    
void XllpCIConfigureEP(unsigned int ci_reg_base, int parity_check)
{
    unsigned int value;

    // write parity_enable field in cicr0   
    value = READ_REG(XLLP_CICR0);
    if (parity_check) {
        value |= XLLP_CI_CICR0_PAR_EN;
    }
    else {
        value &= ~XLLP_CI_CICR0_PAR_EN;
    }
    WRITE_REG(XLLP_CICR0, value);   
    return; 
}

void XllpCIConfigureES(unsigned int ci_reg_base, int parity_check)
{
    // the operationi is same as Embedded-Parallel
    XllpCIConfigureEP(ci_reg_base, parity_check);
}

void XllpCISetClock(unsigned int ci_reg_base, unsigned int clk_regs_base, int pclk_enable, int mclk_enable, unsigned int mclk_mhz)
{
    unsigned int ciclk,  value, div, cccr_l;
    P_XLLP_CLKMGR_T pclk;
	float p;

    // determine the LCLK frequency programmed into the CCCR.
    pclk = (P_XLLP_CLKMGR_T)clk_regs_base;
    cccr_l = (pclk->cccr & 0x0000001F);

	if (cccr_l < 8) // L = [2 - 7]
		ciclk = (13 * cccr_l) * 100;
	else if (cccr_l < 17) // L = [8 - 16] 
		ciclk = ((13 * cccr_l) * 100) >> 1;
	else if (cccr_l < 32) // L = [17 - 31]
		ciclk = ((13 * cccr_l) * 100) >> 2;
	
	p = (float)((ciclk / mclk_mhz) - 2) / 2;

	div = (unsigned int) (ceil(p));

    // write cicr4
    value = READ_REG(XLLP_CICR4);
    value &= ~(XLLP_CI_CICR4_PCLK_EN | XLLP_CI_CICR4_MCLK_EN | XLLP_CI_CICR4_DIV_SMASK<<XLLP_CI_CICR4_DIV_SHIFT);
    value |= (pclk_enable) ? XLLP_CI_CICR4_PCLK_EN : 0;
    value |= (mclk_enable) ? XLLP_CI_CICR4_MCLK_EN : 0;
    value |= div << XLLP_CI_CICR4_DIV_SHIFT;
    WRITE_REG(XLLP_CICR4, value);   
    return; 
}

void XllpCISetPolarity(unsigned int ci_reg_base, int pclk_sample_falling, int hsync_active_low, int vsync_active_low)
{
    unsigned int value;

    // write cicr4
    value = READ_REG(XLLP_CICR4);
    value &= ~(XLLP_CI_CICR4_PCP | XLLP_CI_CICR4_HSP | XLLP_CI_CICR4_VSP);
    value |= (pclk_sample_falling)? XLLP_CI_CICR4_PCP : 0;
    value |= (hsync_active_low) ? XLLP_CI_CICR4_HSP : 0;
    value |= (vsync_active_low) ? XLLP_CI_CICR4_VSP : 0;
    WRITE_REG(XLLP_CICR4, value);   
    return; 
}

void XllpCISetFIFO(unsigned int ci_reg_base, unsigned int timeout, XLLP_CI_FIFO_THRESHOLD threshold, int fifo1_enable,
                   int fifo2_enable)
{
    unsigned int value;

    // write citor
    WRITE_REG(XLLP_CITOR, timeout); 
    
    // write cifr: always enable fifo 0! also reset input fifo 
    value = READ_REG(XLLP_CIFR);
    value &= ~(XLLP_CI_CIFR_FEN0 | XLLP_CI_CIFR_FEN1 | XLLP_CI_CIFR_FEN2 | XLLP_CI_CIFR_RESETF | 
                XLLP_CI_CIFR_THL_0_SMASK<<XLLP_CI_CIFR_THL_0_SHIFT);
    value |= (unsigned int)threshold << XLLP_CI_CIFR_THL_0_SHIFT;
    value |= (fifo1_enable) ? XLLP_CI_CIFR_FEN1 : 0;
    value |= (fifo2_enable) ? XLLP_CI_CIFR_FEN2 : 0;
    value |= XLLP_CI_CIFR_RESETF | XLLP_CI_CIFR_FEN0;
    WRITE_REG(XLLP_CIFR, value);    
    return; 
}

void XllpCIResetFIFO(unsigned int ci_reg_base)
{
    unsigned int value;
    value = READ_REG(XLLP_CIFR);
    value |= XLLP_CI_CIFR_RESETF;
    WRITE_REG(XLLP_CIFR, value);    
}

void XllpCISetInterruptMask(unsigned int ci_reg_base, unsigned int mask)
{
    unsigned int value;

    // write mask in cicr0  
    value = READ_REG(XLLP_CICR0);
    value &= ~XLLP_CI_CICR0_INTERRUPT_MASK;
    value |= (mask & XLLP_CI_CICR0_INTERRUPT_MASK);
    WRITE_REG(XLLP_CICR0, value);   
    return; 
}

unsigned int XllpCIGetInterruptMask(unsigned int ci_reg_base)
{
    unsigned int value;

    // write mask in cicr0  
    value = READ_REG(XLLP_CICR0);
    return (value & XLLP_CI_CICR0_INTERRUPT_MASK);
}

void XllpCIClearInterruptStatus(unsigned int ci_reg_base, unsigned int status)
{
    // write 1 to clear
    WRITE_REG(XLLP_CISR, status);
}

unsigned int XllpCIGetInterruptStatus(unsigned int ci_reg_base)
{
    return  READ_REG(XLLP_CISR);
}

void XllpCISetRegisterValue(unsigned int ci_reg_base, unsigned int reg_offset, unsigned int value)
{
	WRITE_REG(reg_offset, value);
}

//-------------------------------------------------------------------------------------------------------
//      Control APIs
//-------------------------------------------------------------------------------------------------------
void XllpCIInit(unsigned int ci_reg_base, unsigned int clk_regs_base)
{
    P_XLLP_CLKMGR_T pclk;

    // clear all CI registers
    WRITE_REG(XLLP_CICR0, 0x3FF);   // disable all interrupts
    WRITE_REG(XLLP_CICR1, 0);
    WRITE_REG(XLLP_CICR2, 0);
    WRITE_REG(XLLP_CICR3, 0);
    WRITE_REG(XLLP_CICR4, 0);
    WRITE_REG(XLLP_CISR, ~0);
    WRITE_REG(XLLP_CIFR,  0);
    WRITE_REG(XLLP_CITOR, 0);

    // enable CI clock
    XllpLock(CKEN);
    pclk = (P_XLLP_CLKMGR_T)clk_regs_base;
    pclk->cken |= XLLP_CLKEN_CAMERA;
    XllpUnlock(CKEN);
}

void XllpCIDeInit(unsigned int ci_reg_base, unsigned int clk_regs_base)
{
    P_XLLP_CLKMGR_T pclk;

    // disable CI clock
    XllpLock(CKEN);
    pclk = (P_XLLP_CLKMGR_T)clk_regs_base;
    pclk->cken &= ~XLLP_CLKEN_CAMERA;
    XllpUnlock(CKEN);
}

void XllpCIEnable(unsigned int ci_reg_base, int dma_en)
{
    unsigned int value;

    // write mask in cicr0  
    value = READ_REG(XLLP_CICR0);
    value |= XLLP_CI_CICR0_ENB;
    if (dma_en) {
        value |= XLLP_CI_CICR0_DMA_EN;
    }
    WRITE_REG(XLLP_CICR0, value);   
    return; 
}

void XllpCIDisableComplete(unsigned int ci_reg_base)
{
	unsigned int value;
	
	// Clear the disable control bit.
	value = READ_REG(XLLP_CICR0);
	value &= ~XLLP_CI_CICR0_DIS;
	WRITE_REG( XLLP_CICR0, value );
}

int XllpCIDisable(unsigned int ci_reg_base, unsigned int ost_reg_base, int quick, int wait_for_disable_complete )
{
    volatile unsigned int value, mask;
    int retry;
    
    // write control bit in cicr0   
    value = READ_REG(XLLP_CICR0);
    if (quick) {
        value &= ~XLLP_CI_CICR0_ENB;
        mask = XLLP_CI_CISR_CQD;
    }
    else {
        value |= XLLP_CI_CICR0_DIS;
        mask = XLLP_CI_CISR_CDD;
    }
    WRITE_REG(XLLP_CICR0, value);   

	if( wait_for_disable_complete )
	{
	    // wait shutdown complete
	    retry = 50;
	    while ( retry-- > 0 ) {
	        value = READ_REG(XLLP_CISR);        
	        if ( value & mask ) {
	            WRITE_REG(XLLP_CISR, mask);
	            return 0;
	        }
	        XllpOstDelayMilliSeconds((P_XLLP_OST_T)ost_reg_base, 10);
	    }
	}
	else
		return 0;
	
    return -1; 
}

void XllpCISlaveCaptureEnable(unsigned int ci_reg_base)
{
    unsigned int value;

    // write mask in cicr0  
    value = READ_REG(XLLP_CICR0);
    value |= XLLP_CI_CICR0_SL_CAP_EN;
    WRITE_REG(XLLP_CICR0, value);   
    return; 
}

void XllpCISlaveCaptureDisable(unsigned int ci_reg_base)
{
    unsigned int value;

    // write mask in cicr0  
    value = READ_REG(XLLP_CICR0);
    value &= ~XLLP_CI_CICR0_SL_CAP_EN;
    WRITE_REG(XLLP_CICR0, value);   
    return; 
}


