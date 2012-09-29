//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  regs_led.h
//
//  This header file defines led (includes the backlight and Tri-color LED) registers of
//  MC13783.
//
//------------------------------------------------------------------------------

#ifndef __REGS_LED_H__
#define __REGS_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define MC13783_LED_CTL0_LEDEN_LSH            0
#define MC13783_LED_CTL0_LEDMDRAMPUP_LSH            1
#define MC13783_LED_CTL0_LEDADRAMPUP_LSH            2
#define MC13783_LED_CTL0_LEDKPRAMPUP_LSH            3
#define MC13783_LED_CTL0_LEDMDRAMPDOWN_LSH            4
#define MC13783_LED_CTL0_LEDADRAMPDOWN_LSH            5
#define MC13783_LED_CTL0_LEDKPRAMPDOWN_LSH            6
#define MC13783_LED_CTL0_TRIODEMD_LSH            7
#define MC13783_LED_CTL0_TRIODEAD_LSH            8
#define MC13783_LED_CTL0_TRIODEKP_LSH            9
#define MC13783_LED_CTL0_BOOSTEN_LSH             10
#define MC13783_LED_CTL0_BOOSTABMS_LSH       11
#define MC13783_LED_CTL0_BOOSTABR_LSH        14
#define MC13783_LED_CTL0_FLPATTERN_LSH            17
#define MC13783_LED_CTL0_FLBANK1_LSH            21
#define MC13783_LED_CTL0_FLBANK2_LSH            22
#define MC13783_LED_CTL0_FLBANK3_LSH            23

#define MC13783_LED_CTL1_LEDR1RAMPUP_LSH            0
#define MC13783_LED_CTL1_LEDG1RAMPUP_LSH            1
#define MC13783_LED_CTL1_LEDB1RAMPUP_LSH            2
#define MC13783_LED_CTL1_LEDR1RAMPDOWN_LSH            3
#define MC13783_LED_CTL1_LEDG1RAMPDOWN_LSH            4
#define MC13783_LED_CTL1_LEDB1RAMPDOWN_LSH            5
#define MC13783_LED_CTL1_LEDR2RAMPUP_LSH            6
#define MC13783_LED_CTL1_LEDG2RAMPUP_LSH            7
#define MC13783_LED_CTL1_LEDB2RAMPUP_LSH            8
#define MC13783_LED_CTL1_LEDR2RAMPDOWN_LSH            9
#define MC13783_LED_CTL1_LEDG2RAMPDOWN_LSH            10
#define MC13783_LED_CTL1_LEDB2RAMPDOWN_LSH            11
#define MC13783_LED_CTL1_LEDR3RAMPUP_LSH            12
#define MC13783_LED_CTL1_LEDG3RAMPUP_LSH            13
#define MC13783_LED_CTL1_LEDB3RAMPUP_LSH            14
#define MC13783_LED_CTL1_LEDR3RAMPDOWN_LSH            15
#define MC13783_LED_CTL1_LEDG3RAMPDOWN_LSH            16
#define MC13783_LED_CTL1_LEDB3RAMPDOWN_LSH            17
#define MC13783_LED_CTL1_TC1HALF_LSH            18
#define MC13783_LED_CTL1_SLEWLIMTC_LSH            23

#define MC13783_LED_CTL2_LEDMD_LSH            0
#define MC13783_LED_CTL2_LEDAD_LSH            3
#define MC13783_LED_CTL2_LEDKP_LSH            6
#define MC13783_LED_CTL2_LEDMDDC_LSH            9
#define MC13783_LED_CTL2_LEDADDC_LSH            13
#define MC13783_LED_CTL2_LEDKPDC_LSH            17
#define MC13783_LED_CTL2_BLPERIOD_LSH            21
#define MC13783_LED_CTL2_SLEWLIMBL_LSH            23

#define MC13783_LED_CTL3_LEDR1_LSH            0
#define MC13783_LED_CTL3_LEDG1_LSH            2
#define MC13783_LED_CTL3_LEDB1_LSH            4
#define MC13783_LED_CTL3_LEDR1DC_LSH            6
#define MC13783_LED_CTL3_LEDG1DC_LSH            11
#define MC13783_LED_CTL3_LEDB1DC_LSH            16
#define MC13783_LED_CTL3_TC1PERIOD_LSH            21
#define MC13783_LED_CTL3_TC1TRIODE_LSH            23

#define MC13783_LED_CTL4_LEDR2_LSH            0
#define MC13783_LED_CTL4_LEDG2_LSH            2
#define MC13783_LED_CTL4_LEDB2_LSH            4
#define MC13783_LED_CTL4_LEDR2DC_LSH            6
#define MC13783_LED_CTL4_LEDG2DC_LSH            11
#define MC13783_LED_CTL4_LEDB2DC_LSH            16
#define MC13783_LED_CTL4_TC2PERIOD_LSH            21
#define MC13783_LED_CTL4_TC2TRIODE_LSH            23

#define MC13783_LED_CTL5_LEDR3_LSH            0
#define MC13783_LED_CTL5_LEDG3_LSH            2
#define MC13783_LED_CTL5_LEDB3_LSH            4
#define MC13783_LED_CTL5_LEDR3DC_LSH            6
#define MC13783_LED_CTL5_LEDG3DC_LSH            11
#define MC13783_LED_CTL5_LEDB3DC_LSH            16
#define MC13783_LED_CTL5_TC3PERIOD_LSH            21
#define MC13783_LED_CTL5_TC3TRIODE_LSH            23


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define MC13783_LED_CTL0_LEDEN_WID            1
#define MC13783_LED_CTL0_LEDMDRAMPUP_WID            1
#define MC13783_LED_CTL0_LEDADRAMPUP_WID            1
#define MC13783_LED_CTL0_LEDKPRAMPUP_WID            1
#define MC13783_LED_CTL0_LEDMDRAMPDOWN_WID            1
#define MC13783_LED_CTL0_LEDADRAMPDOWN_WID            1
#define MC13783_LED_CTL0_LEDKPRAMPDOWN_WID            1
#define MC13783_LED_CTL0_TRIODEMD_WID            1
#define MC13783_LED_CTL0_TRIODEAD_WID            1
#define MC13783_LED_CTL0_TRIODEKP_WID            1
#define MC13783_LED_CTL0_BOOSTEN_WID          1
#define MC13783_LED_CTL0_BOOSTABMS_WID   3
#define MC13783_LED_CTL0_BOOSTABR_WID        2
#define MC13783_LED_CTL0_FLPATTERN_WID            4
#define MC13783_LED_CTL0_FLBANK1_WID            1
#define MC13783_LED_CTL0_FLBANK2_WID            1
#define MC13783_LED_CTL0_FLBANK3_WID            1

#define MC13783_LED_CTL1_LEDR1RAMPUP_WID            1
#define MC13783_LED_CTL1_LEDG1RAMPUP_WID            1
#define MC13783_LED_CTL1_LEDB1RAMPUP_WID            1
#define MC13783_LED_CTL1_LEDR1RAMPDOWN_WID            1
#define MC13783_LED_CTL1_LEDG1RAMPDOWN_WID            1
#define MC13783_LED_CTL1_LEDB1RAMPDOWN_WID            1
#define MC13783_LED_CTL1_LEDR2RAMPUP_WID            1
#define MC13783_LED_CTL1_LEDG2RAMPUP_WID            1
#define MC13783_LED_CTL1_LEDB2RAMPUP_WID            1
#define MC13783_LED_CTL1_LEDR2RAMPDOWN_WID            1
#define MC13783_LED_CTL1_LEDG2RAMPDOWN_WID            1
#define MC13783_LED_CTL1_LEDB2RAMPDOWN_WID            1
#define MC13783_LED_CTL1_LEDR3RAMPUP_WID            1
#define MC13783_LED_CTL1_LEDG3RAMPUP_WID            1
#define MC13783_LED_CTL1_LEDB3RAMPUP_WID            1
#define MC13783_LED_CTL1_LEDR3RAMPDOWN_WID            1
#define MC13783_LED_CTL1_LEDG3RAMPDOWN_WID            1
#define MC13783_LED_CTL1_LEDB3RAMPDOWN_WID            1
#define MC13783_LED_CTL1_TC1HALF_WID            1
#define MC13783_LED_CTL1_SLEWLIMTC_WID            1

#define MC13783_LED_CTL2_LEDMD_WID            3
#define MC13783_LED_CTL2_LEDAD_WID            3
#define MC13783_LED_CTL2_LEDKP_WID            3
#define MC13783_LED_CTL2_LEDMDDC_WID            4
#define MC13783_LED_CTL2_LEDADDC_WID            4
#define MC13783_LED_CTL2_LEDKPDC_WID            4
#define MC13783_LED_CTL2_BLPERIOD_WID            2
#define MC13783_LED_CTL2_SLEWLIMBL_WID            1

#define MC13783_LED_CTL3_LEDR1_WID            2
#define MC13783_LED_CTL3_LEDG1_WID            2
#define MC13783_LED_CTL3_LEDB1_WID            2
#define MC13783_LED_CTL3_LEDR1DC_WID            5
#define MC13783_LED_CTL3_LEDG1DC_WID            5
#define MC13783_LED_CTL3_LEDB1DC_WID            5
#define MC13783_LED_CTL3_TC1PERIOD_WID            2
#define MC13783_LED_CTL3_TC1TRIODE_WID            1

#define MC13783_LED_CTL4_LEDR2_WID            2
#define MC13783_LED_CTL4_LEDG2_WID            2
#define MC13783_LED_CTL4_LEDB2_WID            2
#define MC13783_LED_CTL4_LEDR2DC_WID            5
#define MC13783_LED_CTL4_LEDG2DC_WID            5
#define MC13783_LED_CTL4_LEDB2DC_WID            5
#define MC13783_LED_CTL4_TC2PERIOD_WID            2
#define MC13783_LED_CTL4_TC2TRIODE_WID            1

#define MC13783_LED_CTL5_LEDR3_WID            2
#define MC13783_LED_CTL5_LEDG3_WID            2
#define MC13783_LED_CTL5_LEDB3_WID            2
#define MC13783_LED_CTL5_LEDR3DC_WID            5
#define MC13783_LED_CTL5_LEDG3DC_WID            5
#define MC13783_LED_CTL5_LEDB3DC_WID            5
#define MC13783_LED_CTL5_TC3PERIOD_WID            2
#define MC13783_LED_CTL5_TC3TRIODE_WID            1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// LED_CTL0
#define MC13783_LED_CTL0_LEDEN_DISABLE            0 //master led disable
#define MC13783_LED_CTL0_LEDEN_ENABLE            1 //master led enable

#define MC13783_LED_CTL0_LEDRAMPUP_DISABLE            0 //ramp up backlight channel disable
#define MC13783_LED_CTL0_LEDRAMPUP_ENABLE            1 //ramp up backlight channel enable

#define MC13783_LED_CTL0_LEDRAMPDOWN_DISABLE            0 //ramp down backlight channel disable
#define MC13783_LED_CTL0_LEDRAMPDOWN_ENABLE            1 //ramp down backlight channel enable

#define MC13783_LED_CTL0_TRIODE_DISABLE            0 //triode mode disable
#define MC13783_LED_CTL0_TRIODE_ENABLE            1 //triode mode enable

#define MC13783_LED_CTL0_BOOST_DISABLE            0 //adaptive boost mode disable
#define MC13783_LED_CTL0_BOOST_ENABLE            1 //adaptive boost mode enable

#define MC13783_LED_CTL0_FLBANK_DISABLE            0 //Tri-Color bank disable
#define MC13783_LED_CTL0_FLBANK_ENABLE            1 //Tri-Color bank enable

// LED_CTL1
#define MC13783_LED_CTL1_LEDRAMPUP_DISABLE            0 //ramp up Tri-Color channel disable
#define MC13783_LED_CTL1_LEDRAMPUP_ENABLE            1 //ramp up Tri-Color channel enable

#define MC13783_LED_CTL1_LEDRAMPDOWN_DISABLE            0 //ramp down Tri-Color channel disable
#define MC13783_LED_CTL1_LEDRAMPDOWN_ENABLE            1 //ramp down Tri-Color channel enable

#define MC13783_LED_CTL1_T1HALF_DISABLE            0 //half current mode disable for Tri-Color 1 driver channels
#define MC13783_LED_CTL1_T1HALF_ENABLE            1 //half current mode enable for Tri-Color 1 driver channels

#define MC13783_LED_CTL1_SLEWLIMTC_DISABLE            0 //master disable for Tri-Color Analog Edge Slowing
#define MC13783_LED_CTL1_SLEWLIMTC_ENABLE            1 //master enable for Tri-Color Analog Edge Slowing

// LED_CTL2
#define MC13783_LED_CTL2_SLEWLIMBL_DISABLE            0 //master disable for backlight Analog Edge Slowing
#define MC13783_LED_CTL2_SLEWLIMBL_ENABLE            1 //master enable for backlight Analog Edge Slowing

// LED_CTL3
#define MC13783_LED_CTL3_TC1TRIODE_DISABLE            0 //triode mode for Tri-Color bank 1 channels disable
#define MC13783_LED_CTL3_TC1TRIODE_ENABLE            1 //triode mode for Tri-Color bank 1 channels enable

// LED_CTL4
#define MC13783_LED_CTL4_TC2TRIODE_DISABLE            0 //triode mode for Tri-Color bank 2 channels disable
#define MC13783_LED_CTL4_TC2TRIODE_ENABLE            1 //triode mode for Tri-Color bank 2 channels enable

// LED_CTL5
#define MC13783_LED_CTL5_TC3TRIODE_DISABLE            0 //triode mode for Tri-Color bank 3 channels disable
#define MC13783_LED_CTL5_TC3TRIODE_ENABLE            1 //triode mode for Tri-Color bank 3 channels enable


#ifdef __cplusplus
}
#endif

#endif // __REGS_LED_H__
