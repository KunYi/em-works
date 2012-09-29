//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  regs_led.h
//
//  This header file defines led (includes the backlight and Tri-color LED) registers of
//  MC13892.
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
#define MC13892_LED_CTL0_LEDMDHI_LSH                1
#define MC13892_LED_CTL0_LEDMDRAMP_LSH              2
#define MC13892_LED_CTL0_LEDMDDC_LSH                3
#define MC13892_LED_CTL0_LEDMD_LSH                  9
#define MC13892_LED_CTL0_LEDADHI_LSH                13
#define MC13892_LED_CTL0_LEDADRAMP_LSH              14
#define MC13892_LED_CTL0_LEDADDC_LSH                15
#define MC13892_LED_CTL0_LEDAD_LSH                  21

#define MC13892_LED_CTL1_LEDKPHI_LSH                1
#define MC13892_LED_CTL1_LEDKPRAMP_LSH              2
#define MC13892_LED_CTL1_LEDKPDC_LSH                3
#define MC13892_LED_CTL1_LEDKP_LSH                  9

#define MC13892_LED_CTL2_LEDRPER_LSH                0
#define MC13892_LED_CTL2_LEDRRAMP_LSH               2
#define MC13892_LED_CTL2_LEDRDC_LSH                 3
#define MC13892_LED_CTL2_LEDR_LSH                   9
#define MC13892_LED_CTL2_LEDGPER_LSH                12
#define MC13892_LED_CTL2_LEDGRAMP_LSH               14
#define MC13892_LED_CTL2_LEDGDC_LSH                 15
#define MC13892_LED_CTL2_LEDG_LSH                   21

#define MC13892_LED_CTL3_LEDBPER_LSH                0
#define MC13892_LED_CTL3_LEDBRAMP_LSH               2
#define MC13892_LED_CTL3_LEDBDC_LSH                 3
#define MC13892_LED_CTL3_LEDB_LSH                   9
#define MC13892_LED_CTL3_LEDSWBSTEN_LSH             12


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define MC13892_LED_CTL0_LEDMDHI_WID                1
#define MC13892_LED_CTL0_LEDMDRAMP_WID              1
#define MC13892_LED_CTL0_LEDMDDC_WID                6
#define MC13892_LED_CTL0_LEDMD_WID                  3
#define MC13892_LED_CTL0_LEDADHI_WID                1
#define MC13892_LED_CTL0_LEDADRAMP_WID              1
#define MC13892_LED_CTL0_LEDADDC_WID                6
#define MC13892_LED_CTL0_LEDAD_WID                  3

#define MC13892_LED_CTL1_LEDKPHI_WID                1
#define MC13892_LED_CTL1_LEDKPRAMP_WID              1
#define MC13892_LED_CTL1_LEDKPDC_WID                6
#define MC13892_LED_CTL1_LEDKP_WID                  3

#define MC13892_LED_CTL2_LEDRPER_WID                2
#define MC13892_LED_CTL2_LEDRRAMP_WID               1
#define MC13892_LED_CTL2_LEDRDC_WID                 6
#define MC13892_LED_CTL2_LEDR_WID                   3
#define MC13892_LED_CTL2_LEDGPER_WID                2
#define MC13892_LED_CTL2_LEDGRAMP_WID               1
#define MC13892_LED_CTL2_LEDGDC_WID                 6
#define MC13892_LED_CTL2_LEDG_WID                   3

#define MC13892_LED_CTL3_LEDBPER_WID                2
#define MC13892_LED_CTL3_LEDBRAMP_WID               1
#define MC13892_LED_CTL3_LEDBDC_WID                 6
#define MC13892_LED_CTL3_LEDB_WID                   3
#define MC13892_LED_CTL3_LEDSWBSTEN_WID             1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
// LED_CTL0

#define MC13892_LED_CTL0_LEDMDHI_DISABLE             0 
#define MC13892_LED_CTL0_LEDMDHI_ENABLE              1 

#define MC13892_LED_CTL0_LEDMDRAMP_DISABLE           0 
#define MC13892_LED_CTL0_LEDMDRAMP_ENABLE            1 

#define MC13892_LED_CTL0_LEDADHI_DISABLE             0 
#define MC13892_LED_CTL0_LEDADHI_ENABLE              1 

#define MC13892_LED_CTL0_LEDADRAMP_DISABLE           0 
#define MC13892_LED_CTL0_LEDADRAMP_ENABLE            1 

// LED_CTL1
#define MC13892_LED_CTL1_LEDKPHI_DISABLE             0 
#define MC13892_LED_CTL1_LEDKPHI_ENABLE              1 

#define MC13892_LED_CTL1_LEDKPRAMP_DISABLE           0 
#define MC13892_LED_CTL1_LEDKPRAMP_ENABLE            1 


// LED_CTL2
#define MC13892_LED_CTL2_LEDRRAMP_DISABLE            0
#define MC13892_LED_CTL2_LEDRRAMP_ENABLE             1 

#define MC13892_LED_CTL2_LEDGRAMP_DISABLE            0 
#define MC13892_LED_CTL2_LEDGRAMP_ENABLE             1 


// LED_CTL3
#define MC13892_LED_CTL3_LEDBRAMP_DISABLE            0 
#define MC13892_LED_CTL3_LEDBRAMP_ENABLE             1 
#define MC13892_LED_CTL3_LEDSWBSTEN_DISABLE          0 
#define MC13892_LED_CTL3_LEDSWBSTEN_ENABLE           1 


#ifdef __cplusplus
}
#endif

#endif // __REGS_LED_H__
