//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  regs.h
//
//  This header file defines the registers of MC13783.
//
//------------------------------------------------------------------------------

#ifndef __MC13783_REGS_H__
#define __MC13783_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// REGISTER SPI ADDRESSES
//------------------------------------------------------------------------------
#define MC13783_INT_STAT0_ADDR          0x00
#define MC13783_INT_MSK0_ADDR           0x01
#define MC13783_INT_SEN0_ADDR           0x02
#define MC13783_INT_STAT1_ADDR          0x03
#define MC13783_INT_MSK1_ADDR           0x04
#define MC13783_INT_SEN1_ADDR           0x05
#define MC13783_PWR_MOD_ADDR            0x06
#define MC13783_REV_ADDR                0x07
#define MC13783_SMPH_ADDR               0x08
#define MC13783_ARB_AUD_ADDR            0x09
#define MC13783_ARB_SW_ADDR             0x0A
#define MC13783_ARB_REG0_ADDR           0x0B
#define MC13783_ARB_REG1_ADDR           0x0C
#define MC13783_PWR_CTL0_ADDR           0x0D
#define MC13783_PWR_CTL1_ADDR           0x0E
#define MC13783_PWR_CTL2_ADDR           0x0F
#define MC13783_REG_ASS_ADDR            0x10
#define MC13783_CTL_SPR_ADDR            0x11
#define MC13783_MEM_A_ADDR              0x12
#define MC13783_MEM_B_ADDR              0x13
#define MC13783_RTC_TM_ADDR             0x14
#define MC13783_RTC_ALM_ADDR            0x15
#define MC13783_RTC_DAY_ADDR            0x16
#define MC13783_RTC_DAY_ALM_ADDR        0x17
#define MC13783_SW0_ADDR                0x18
#define MC13783_SW1_ADDR                0x19
#define MC13783_SW2_ADDR                0x1A
#define MC13783_SW3_ADDR                0x1B
#define MC13783_SW4_ADDR                0x1C
#define MC13783_SW5_ADDR                0x1D
#define MC13783_REG_SET0_ADDR           0x1E
#define MC13783_REG_SET1_ADDR           0x1F
#define MC13783_REG_MOD0_ADDR           0x20
#define MC13783_REG_MOD1_ADDR           0x21
#define MC13783_PWR_MISC_ADDR           0x22
#define MC13783_PWR_SPR_ADDR            0x23
#define MC13783_AUD_RX0_ADDR            0x24
#define MC13783_AUD_RX1_ADDR            0x25
#define MC13783_AUD_TX_ADDR             0x26
#define MC13783_SSI_NW_ADDR             0x27
#define MC13783_AUD_CDC_ADDR            0x28
#define MC13783_AUD_STR_DAC_ADDR        0x29
#define MC13783_AUD_SPR_ADDR            0x2A
#define MC13783_ADC0_ADDR               0x2B
#define MC13783_ADC1_ADDR               0x2C
#define MC13783_ADC2_ADDR               0x2D
#define MC13783_ADC3_ADDR               0x2E
#define MC13783_ADC4_ADDR               0x2F
#define MC13783_CHG0_ADDR               0x30
#define MC13783_USB0_ADDR               0x31
#define MC13783_CHG_USB1_ADDR           0x32
#define MC13783_LED_CTL0_ADDR           0x33
#define MC13783_LED_CTL1_ADDR           0x34
#define MC13783_LED_CTL2_ADDR           0x35
#define MC13783_LED_CTL3_ADDR           0x36
#define MC13783_LED_CTL4_ADDR           0x37
#define MC13783_LED_CTL5_ADDR           0x38
#define MC13783_SPR_ADDR                0x39
#define MC13783_TRIM0_ADDR              0x3A
#define MC13783_TRIM1_ADDR              0x3B
#define MC13783_TST0_ADDR               0x3C
#define MC13783_TST1_ADDR               0x3D
#define MC13783_TST2_ADDR               0x3E
#define MC13783_TST3_ADDR               0x3F


#ifdef __cplusplus
}
#endif

#endif // __MC13783_REGS_H__
