//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  regs.h
//
//  This header file defines the registers of MC13892.
//
//------------------------------------------------------------------------------

#ifndef __MC13892_REGS_H__
#define __MC13892_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// REGISTER SPI ADDRESSES
//------------------------------------------------------------------------------
#define MC13892_INT_STAT0_ADDR          0x00
#define MC13892_INT_MSK0_ADDR           0x01
#define MC13892_INT_SEN0_ADDR           0x02
#define MC13892_INT_STAT1_ADDR          0x03
#define MC13892_INT_MSK1_ADDR           0x04
#define MC13892_INT_SEN1_ADDR           0x05
#define MC13892_PWR_MOD_ADDR            0x06
#define MC13892_REV_ADDR                0x07
#define MC13892_AAC1_ADDR               0x09
#define MC13892_AAC2_ADDR               0x0A
#define MC13892_PWR_CTL0_ADDR           0x0D
#define MC13892_PWR_CTL1_ADDR           0x0E
#define MC13892_PWR_CTL2_ADDR           0x0F
#define MC13892_MEM_A_ADDR              0x12
#define MC13892_MEM_B_ADDR              0x13
#define MC13892_RTC_TM_ADDR             0x14
#define MC13892_RTC_ALM_ADDR            0x15
#define MC13892_RTC_DAY_ADDR            0x16
#define MC13892_RTC_DAY_ALM_ADDR        0x17
#define MC13892_SW0_ADDR                0x18
#define MC13892_SW1_ADDR                0x19
#define MC13892_SW2_ADDR                0x1A
#define MC13892_SW3_ADDR                0x1B
#define MC13892_SW4_ADDR                0x1C
#define MC13892_SW5_ADDR                0x1D
#define MC13892_REG_SET0_ADDR           0x1E
#define MC13892_REG_SET1_ADDR           0x1F
#define MC13892_REG_MOD0_ADDR           0x20
#define MC13892_REG_MOD1_ADDR           0x21
#define MC13892_PWR_MISC_ADDR           0x22
#define MC13892_ADC0_ADDR               0x2B
#define MC13892_ADC1_ADDR               0x2C
#define MC13892_ADC2_ADDR               0x2D
#define MC13892_ADC3_ADDR               0x2E
#define MC13892_ADC4_ADDR               0x2F
#define MC13892_CHG0_ADDR               0x30
#define MC13892_USB0_ADDR               0x31
#define MC13892_CHG_USB1_ADDR           0x32
#define MC13892_LED_CTL0_ADDR           0x33
#define MC13892_LED_CTL1_ADDR           0x34
#define MC13892_LED_CTL2_ADDR           0x35
#define MC13892_LED_CTL3_ADDR           0x36
#define MC13892_TRIM0_ADDR              0x39
#define MC13892_TRIM1_ADDR              0x3A
#define MC13892_TST0_ADDR               0x3B
#define MC13892_TST1_ADDR               0x3C
#define MC13892_TST2_ADDR               0x3D
#define MC13892_TST3_ADDR               0x3E
#define MC13892_TST4_ADDR               0x3F

#ifdef __cplusplus
}
#endif

#endif // __MC13892_REGS_H__
