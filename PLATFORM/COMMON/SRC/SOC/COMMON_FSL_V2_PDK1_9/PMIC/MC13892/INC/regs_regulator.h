//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  regs_regulator.h
//
//  This header file defines the Linear Regulator Register of MC13892 PMIC
//
//------------------------------------------------------------------------------

#ifndef __REGS_REGULATOR_H__
#define __REGS_REGULATOR_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

// Register 24, Switchers 0
#define MC13892_SW0_SW1_LSH                  0
#define MC13892_SW0_SW1DVS_LSH               5
#define MC13892_SW0_SW1STBY_LSH              10
#define MC13892_SW0_SW1SIDMAX_LSH            15
#define MC13892_SW0_SW1SIDMIN_LSH            19
#define MC13892_SW0_SW1HI_LSH                23

// Register 25, Switchers 1
#define MC13892_SW1_SW2_LSH                  0
#define MC13892_SW1_SW2DVS_LSH               5
#define MC13892_SW1_SW2STBY_LSH              10
#define MC13892_SW1_SW2SIDMAX_LSH            15
#define MC13892_SW1_SW2SIDMIN_LSH            19
#define MC13892_SW1_SW2HI_LSH                23

// Register 26, Switchers 2
#define MC13892_SW2_SW3_LSH                  0
#define MC13892_SW2_SW3STBY_LSH              10
#define MC13892_SW2_SW3HI_LSH                23

// Register 27, Switchers 3
#define MC13892_SW3_SW4_LSH                  0
#define MC13892_SW3_SW4STBY_LSH              10
#define MC13892_SW3_SW4HI_LSH                23

// Register 28, Switchers 4
#define MC13892_SW4_SW1MODE_LSH              0
#define MC13892_SW4_SW1MHMODE_LSH            4
#define MC13892_SW4_SW1UOMODE_LSH            5
#define MC13892_SW4_SW1DVSSPEED_LSH          6
#define MC13892_SW4_SIDEN_LSH                8
#define MC13892_SW4_SW2MODE_LSH              10
#define MC13892_SW4_SW2MHMODE_LSH            14
#define MC13892_SW4_SW2UOMODE_LSH            15
#define MC13892_SW4_SW2DVSSPEED_LSH          16
#define MC13892_SW4_PLLEN_LSH                18
#define MC13892_SW4_PLLX_LSH                 19
#define MC13892_SW4_SWILIMB_LSH              22


// Register 29, Switchers 5
#define MC13892_SW5_SW3MODE_LSH              0
#define MC13892_SW5_SW3MHMODE_LSH            4
#define MC13892_SW5_SW3UOMODE_LSH            5
#define MC13892_SW5_SW4MODE_LSH              8
#define MC13892_SW5_SW4MHMODE_LSH            12
#define MC13892_SW5_SW4UOMODE_LSH            13
#define MC13892_SW5_SWBSTEN_LSH              20

// Register 30, Regulator Setting 0
#define MC13892_REG_SET0_VGEN1_LSH                 0
#define MC13892_REG_SET0_VDIG_LSH                  4
#define MC13892_REG_SET0_VGEN2_LSH                 6
#define MC13892_REG_SET0_VPLL_LSH                  9
#define MC13892_REG_SET0_VUSB2_LSH                 11
#define MC13892_REG_SET0_VGEN3_LSH                 14
#define MC13892_REG_SET0_VCAM_LSH                  16


// Register 31, Regulator Setting 1
#define MC13892_REG_SET1_VVIDEO_LSH                    2
#define MC13892_REG_SET1_VAUDIO_LSH                    4
#define MC13892_REG_SET1_VSD1_LSH                      6


// Register 32, Regulator Mode 0
#define MC13892_REG_MODE0_VGEN1EN_LSH                   0
#define MC13892_REG_MODE0_VGEN1STBY_LSH                 1 
#define MC13892_REG_MODE0_VGEN1MODE_LSH                 2 
#define MC13892_REG_MODE0_VIOHIEN_LSH                   3
#define MC13892_REG_MODE0_VIOHISTBY_LSH                 4
#define MC13892_REG_MODE0_VDIGEN_LSH                    9
#define MC13892_REG_MODE0_VDIGSTBY_LSH                  10
#define MC13892_REG_MODE0_VGEN2EN_LSH                   12
#define MC13892_REG_MODE0_VGEN2STBY_LSH                 13
#define MC13892_REG_MODE0_VGEN2MODE_LSH                 14
#define MC13892_REG_MODE0_VPLLEN_LSH                    15
#define MC13892_REG_MODE0_VPLLSTBY_LSH                  16
#define MC13892_REG_MODE0_VUSB2EN_LSH                   18
#define MC13892_REG_MODE0_VUSB2STBY_LSH                 19


// Register 33, Regulator Mode 1
#define MC13892_REG_MODE1_VGEN3EN_LSH                       0
#define MC13892_REG_MODE1_VGEN3STBY_LSH                     1
#define MC13892_REG_MODE1_VGEN3MODE_LSH                     2
#define MC13892_REG_MODE1_VGEN3CONFIG_LSH                   3
#define MC13892_REG_MODE1_VCAMEN_LSH                        6
#define MC13892_REG_MODE1_VCAMSTBY_LSH                      7
#define MC13892_REG_MODE1_VCAMMODE_LSH                      8
#define MC13892_REG_MODE1_VCAMCONFIG_LSH                    9
#define MC13892_REG_MODE1_VVIDEOEN_LSH                      12
#define MC13892_REG_MODE1_VVIDEOSTBY_LSH                    13
#define MC13892_REG_MODE1_VVIDEOMODE_LSH                    14
#define MC13892_REG_MODE1_VAUDIOEN_LSH                      15
#define MC13892_REG_MODE1_VAUDIOSTBY_LSH                    16
#define MC13892_REG_MODE1_VSDEN_LSH                         18
#define MC13892_REG_MODE1_VSDSTBY_LSH                       19
#define MC13892_REG_MODE1_VSDMODE_LSH                       20


// Register 34, Register 34, Power Miscellaneous
#define MC13892_REG_MISC_REGSCPEN_LSH                       0
#define MC13892_REG_MISC_GPO1EN_LSH                         6
#define MC13892_REG_MISC_GPO1STBY_LSH                       7
#define MC13892_REG_MISC_GPO2EN_LSH                         8
#define MC13892_REG_MISC_GPO2STBY_LSH                       9
#define MC13892_REG_MISC_GPO3EN_LSH                         10
#define MC13892_REG_MISC_GPO3STBY_LSH                       11
#define MC13892_REG_MISC_GPO4EN_LSH                         12
#define MC13892_REG_MISC_GPO4STBY_LSH                       13
#define MC13892_REG_MISC_PWGT1SPIEN_LSH                     15
#define MC13892_REG_MISC_PWGT2SPIEN_LSH                     16
#define MC13892_REG_MISC_GPO4ADIN_LSH                       21


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

// Register 24, Switchers 0
#define MC13892_SW0_SW1_WID                  5
#define MC13892_SW0_SW1DVS_WID               5
#define MC13892_SW0_SW1STBY_WID              5
#define MC13892_SW0_SW1SIDMAX_WID            4
#define MC13892_SW0_SW1SIDMIN_WID            4
#define MC13892_SW0_SW1HI_WID                1

// Register 25, Switchers 1
#define MC13892_SW1_SW2_WID                  5
#define MC13892_SW1_SW2DVS_WID               5
#define MC13892_SW1_SW2STBY_WID              5
#define MC13892_SW1_SW2SIDMAX_WID            4
#define MC13892_SW1_SW2SIDMIN_WID            4
#define MC13892_SW1_SW2HI_WID                1

// Register 26, Switchers 2
#define MC13892_SW2_SW3_WID                  5
#define MC13892_SW2_SW3STBY_WID              5
#define MC13892_SW2_SW3HI_WID                1

// Register 27, Switchers 3
#define MC13892_SW3_SW4_WID                  5
#define MC13892_SW3_SW4STBY_WID              5
#define MC13892_SW3_SW4HI_WID                1


// Register 28, Switchers 4
#define MC13892_SW4_SW1MODE_WID              4
#define MC13892_SW4_SW1MHMODE_WID            1
#define MC13892_SW4_SW1UOMODE_WID            1
#define MC13892_SW4_SW1DVSSPEED_WID          2
#define MC13892_SW4_SIDEN_WID                1
#define MC13892_SW4_SW2MODE_WID              4
#define MC13892_SW4_SW2MHMODE_WID            1
#define MC13892_SW4_SW2UOMODE_WID            1
#define MC13892_SW4_SW2DVSSPEED_WID          2
#define MC13892_SW4_PLLEN_WID                1
#define MC13892_SW4_PLLX_WID                 3
#define MC13892_SW4_SWILIMB_WID              1

// Register 29, Switchers 5
#define MC13892_SW5_SW3MODE_WID              4
#define MC13892_SW5_SW3MHMODE_WID            1
#define MC13892_SW5_SW3UOMODE_WID            1
#define MC13892_SW5_SW4MODE_WID              4
#define MC13892_SW5_SW4MHMODE_WID            1
#define MC13892_SW5_SW4UOMODE_WID            1
#define MC13892_SW5_SWBSTEN_WID              1

// Register 30, Regulator Setting 0
#define MC13892_REG_SET0_VGEN1_WID                 2
#define MC13892_REG_SET0_VDIG_WID                  2
#define MC13892_REG_SET0_VGEN2_WID                 3
#define MC13892_REG_SET0_VPLL_WID                  2
#define MC13892_REG_SET0_VUSB2_WID                 2
#define MC13892_REG_SET0_VGEN3_WID                 1
#define MC13892_REG_SET0_VCAM_WID                  2


// Register 31, Regulator Setting 1
#define MC13892_REG_SET1_VVIDEO_WID                    2
#define MC13892_REG_SET1_VAUDIO_WID                    2
#define MC13892_REG_SET1_VSD1_WID                      3


// Register 32, Regulator Mode 0
#define MC13892_REG_MODE0_VGEN1EN_WID                   1
#define MC13892_REG_MODE0_VGEN1STBY_WID                 1 
#define MC13892_REG_MODE0_VGEN1MODE_WID                 1 
#define MC13892_REG_MODE0_VIOHIEN_WID                   1
#define MC13892_REG_MODE0_VIOHISTBY_WID                 1
#define MC13892_REG_MODE0_VDIGEN_WID                    1
#define MC13892_REG_MODE0_VDIGSTBY_WID                  1
#define MC13892_REG_MODE0_VGEN2EN_WID                   1
#define MC13892_REG_MODE0_VGEN2STBY_WID                 1
#define MC13892_REG_MODE0_VGEN2MODE_WID                 1
#define MC13892_REG_MODE0_VPLLEN_WID                    1
#define MC13892_REG_MODE0_VPLLSTBY_WID                  1
#define MC13892_REG_MODE0_VUSB2EN_WID                   1
#define MC13892_REG_MODE0_VUSB2STBY_WID                 1


// Register 33, Regulator Mode 1
#define MC13892_REG_MODE1_VGEN3EN_WID                    1
#define MC13892_REG_MODE1_VGEN3STBY_WID                  1
#define MC13892_REG_MODE1_VGEN3MODE_WID                  1
#define MC13892_REG_MODE1_VGEN3CONFIG_WID                1
#define MC13892_REG_MODE1_VCAMEN_WID                     1
#define MC13892_REG_MODE1_VCAMSTBY_WID                   1
#define MC13892_REG_MODE1_VCAMMODE_WID                   1
#define MC13892_REG_MODE1_VCAMCONFIG_WID                 1
#define MC13892_REG_MODE1_VVIDEOEN_WID                   1
#define MC13892_REG_MODE1_VVIDEOSTBY_WID                 1
#define MC13892_REG_MODE1_VVIDEOMODE_WID                 1
#define MC13892_REG_MODE1_VAUDIOEN_WID                   1
#define MC13892_REG_MODE1_VAUDIOSTBY_WID                 1
#define MC13892_REG_MODE1_VSDEN_WID                      1
#define MC13892_REG_MODE1_VSDSTBY_WID                    1
#define MC13892_REG_MODE1_VSDMODE_WID                    1


// Register 34, Register 34, Power Miscellaneous
#define MC13892_REG_MISC_REGSCPEN_WID                    1
#define MC13892_REG_MISC_GPO1EN_WID                      1
#define MC13892_REG_MISC_GPO1STBY_WID                    1
#define MC13892_REG_MISC_GPO2EN_WID                      1
#define MC13892_REG_MISC_GPO2STBY_WID                    1
#define MC13892_REG_MISC_GPO3EN_WID                      1
#define MC13892_REG_MISC_GPO3STBY_WID                    1
#define MC13892_REG_MISC_GPO4EN_WID                      1
#define MC13892_REG_MISC_GPO4STBY_WID                    1
#define MC13892_REG_MISC_PWGT1SPIEN_WID                  1
#define MC13892_REG_MISC_PWGT2SPIEN_WID                  1
#define MC13892_REG_MISC_GPO4ADIN_WID                    1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define MC13892_SW_MAX                                  0x1F
#define MC13892_SWDVS_MAX                               0x1F
#define MC13892_SWSTBY_MAX                              0x1F

#define MC13892_SW_PLLMF_MAX                             7
#define MC13892_SW_DVSSPEED_MAX                          3
#define MC13892_SW_DIP_LEVEL                             16

#define MC13892_SW3_MAX                                  3
#define MC13892_SW5_SW3EN_ENABLE                         1
#define MC13892_SW5_SW3EN_DISABLE                        0


#ifdef __cplusplus
}
#endif

#endif // __REGS_REGULATOR_H__
