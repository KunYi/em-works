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
//
//  Copyright (C) 2005, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  regs_regulator.h
//
//  This header file defines the Linear Regulator Register of MC13783 PMIC
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

// Register 10, Arbitration switches
#define MC13783_ARB_SW_SW1ASTBYAND_LSH                      0
#define MC13783_ARB_SW_SW1BSTBYAND_LSH                      1
#define MC13783_ARB_SW_SW2ASTBYAND_LSH                      2
#define MC13783_ARB_SW_SW2BSTBYAND_LSH                      3
#define MC13783_ARB_SW_SW3SEL0_LSH                               4 
#define MC13783_ARB_SW_SW1ABDVS_LSH                            5 
#define MC13783_ARB_SW_SW2ABDVS_LSH                            6 
#define MC13783_ARB_SW_SW1ASEL_LSH                               7 
#define MC13783_ARB_SW_SW1BSEL_LSH                               8 
#define MC13783_ARB_SW_SW2ASEL_LSH                               9 
#define MC13783_ARB_SW_SW2BSEL_LSH                               10 
#define MC13783_ARB_SW_SW3SEL1_LSH                               11 
#define MC13783_ARB_SW_PLLSEL_LSH                                   12
#define MC13783_ARB_SW_PWGT1SEL_LSH                             14 
#define MC13783_ARB_SW_PWGT2SEL_LSH                             15 

// Register 11, Arbitration Regulators 0
#define MC13783_ARB_REG0_VAUDIOSEL_LSH                      0
#define MC13783_ARB_REG0_VIOHISEL_LSH                         2
#define MC13783_ARB_REG0_VIOLOSEL_LSH                         4
#define MC13783_ARB_REG0_VDIGSEL_LSH                           6
#define MC13783_ARB_REG0_VGENSEL_LSH                          8
#define MC13783_ARB_REG0_VRFDIGSEL_LSH                      10
#define MC13783_ARB_REG0_VRFREFSEL_LSH                      12
#define MC13783_ARB_REG0_VRFCPSSEL_LSH                      14
#define MC13783_ARB_REG0_VSIMSEL_LSH                          16
#define MC13783_ARB_REG0_VESIMSEL_LSH                        18
#define MC13783_ARB_REG0_VCAMSEL_LSH                         20
#define MC13783_ARB_REG0_VRFBGSEL_LSH                        22

// Register 12, Arbitration Regulators 1
#define MC13783_ARB_REG1_VVIBSEL_LSH                         0
#define MC13783_ARB_REG1_VRF1SEL_LSH                         2
#define MC13783_ARB_REG1_VRF2SEL_LSH                         4
#define MC13783_ARB_REG1_VMMC1SEL_LSH                      6
#define MC13783_ARB_REG1_VMMC2SEL_LSH                      8
#define MC13783_ARB_REG1_VGPO1SEL_LSH                      14
#define MC13783_ARB_REG1_VGPO2SEL_LSH                       16
#define MC13783_ARB_REG1_VGPO3SEL_LSH                        18
#define MC13783_ARB_REG1_VGPO4SEL_LSH                         20

// Register 24, Switchers 0
#define MC13783_SW0_SW1A_LSH                  0
#define MC13783_SW0_SW1ADVS_LSH            6
#define MC13783_SW0_SW1ASTBY_LSH          12

// Register 25, Switchers 1
#define MC13783_SW1_SW1B_LSH                  0
#define MC13783_SW1_SW1BDVS_LSH            6
#define MC13783_SW1_SW1BSTBY_LSH          12

// Register 26, Switchers 2
#define MC13783_SW2_SW2A_LSH                  0
#define MC13783_SW2_SW2ADVS_LSH            6
#define MC13783_SW2_SW2ASTBY_LSH          12

// Register 27, Switchers 3
#define MC13783_SW3_SW2B_LSH                  0
#define MC13783_SW3_SW2BDVS_LSH            6
#define MC13783_SW3_SW2BSTBY_LSH          12

// Register 28, Switchers 4
#define MC13783_SW4_SW1AMODE_LSH                  0
#define MC13783_SW4_SW1ASTBYMODE_LSH          2
#define MC13783_SW4_SW1ADVSSPEED_LSH           6
#define MC13783_SW4_SW1APANIC_LSH                  8
#define MC13783_SW4_SW1ASFST_LSH                    9
#define MC13783_SW4_SW1BMODE_LSH                   10
#define MC13783_SW4_SW1BSTBYMODE_LSH           12
#define MC13783_SW4_SW1BDVSSPEED_LSH            14
#define MC13783_SW4_SW1BPANIC_LSH                  16
#define MC13783_SW4_SW1BSFST_LSH                     17
#define MC13783_SW4_PLLEN_LSH                            18
#define MC13783_SW4_PLLX_LSH                           19
#define MC13783_SW4_PLLDITH_LSH                  22

// Register 29, Switchers 5
#define MC13783_SW5_SW2AMODE_LSH                  0
#define MC13783_SW5_SW2ASTBYMODE_LSH          2
#define MC13783_SW5_SW2ADVSSPEED_LSH           6
#define MC13783_SW5_SW2APANIC_LSH                  8
#define MC13783_SW5_SW2ASFST_LSH                    9
#define MC13783_SW5_SW2BMODE_LSH                   10
#define MC13783_SW5_SW2BSTBYMODE_LSH           12
#define MC13783_SW5_SW2BDVSSPEED_LSH            14
#define MC13783_SW5_SW2BPANIC_LSH                  16
#define MC13783_SW5_SW2BSFST_LSH                     17
#define MC13783_SW5_SW3_LSH                            18
#define MC13783_SW5_SW3EN_LSH                           20
#define MC13783_SW5_SW3STBY_LSH                  21
#define MC13783_SW5_SW3MODE_LSH                  22

// Register 30, Regulator Setting 0
#define MC13783_REG_SET0_VIOLO_LSH                 2
#define MC13783_REG_SET0_VDIG_LSH                   4
#define MC13783_REG_SET0_VGEN_LSH                   6
#define MC13783_REG_SET0_VRFDIG_LSH               9
#define MC13783_REG_SET0_VRFREF_LSH               11
#define MC13783_REG_SET0_VRFCP_LSH                 13
#define MC13783_REG_SET0_VSIM_LSH                   14
#define MC13783_REG_SET0_VESIM_LSH                 15
#define MC13783_REG_SET0_VCAM_LSH                   16

// Register 31, Regulator Setting 1
#define MC13783_REG_SET1_VVIB_LSH                         0
#define MC13783_REG_SET1_VRF1_LSH                         2
#define MC13783_REG_SET1_VRF2_LSH                         4
#define MC13783_REG_SET1_VMMC1_LSH                      6
#define MC13783_REG_SET1_VMMC2_LSH                      9

// Register 32, Regulator Mode 0
#define MC13783_REG_MODE0_VAUDIOEN_LSH                         0
#define MC13783_REG_MODE0_VAUDIOSTBY_LSH                      1 
#define MC13783_REG_MODE0_VAUDIOMODE_LSH                     2 
#define MC13783_REG_MODE0_VIOHIEN_LSH                             3
#define MC13783_REG_MODE0_VIOHISTBY_LSH                          4
#define MC13783_REG_MODE0_VIOHIMODE_LSH                        5
#define MC13783_REG_MODE0_VIOLOEN_LSH                            6
#define MC13783_REG_MODE0_VIOLOSTBY_LSH                        7
#define MC13783_REG_MODE0_VIOLOMODE_LSH                      8
#define MC13783_REG_MODE0_VDIGEN_LSH                            9
#define MC13783_REG_MODE0_VDIGSTBY_LSH                        10
#define MC13783_REG_MODE0_VDIGMODE_LSH                         11
#define MC13783_REG_MODE0_VGENEN_LSH                             12
#define MC13783_REG_MODE0_VGENSTBY_LSH                          13
#define MC13783_REG_MODE0_VGENMODE_LSH                         14
#define MC13783_REG_MODE0_VRFDIGEN_LSH                            15
#define MC13783_REG_MODE0_VRFDIGSTBY_LSH                         16
#define MC13783_REG_MODE0_VRFDIGMODE_LSH                         17
#define MC13783_REG_MODE0_VRFREFEN_LSH                         18
#define MC13783_REG_MODE0_VRFREFSTBY_LSH                         19
#define MC13783_REG_MODE0_VRFREFMODE_LSH                         20
#define MC13783_REG_MODE0_VRFCPEN_LSH                                21   
#define MC13783_REG_MODE0_VRFCPSTBY_LSH                           22
#define MC13783_REG_MODE0_VRFCPMODE_LSH                          23

// Register 33, Regulator Mode 1
#define MC13783_REG_MODE1_VSIMEN_LSH                         0
#define MC13783_REG_MODE1_VSIMSTBY_LSH                     1
#define MC13783_REG_MODE1_VSIMMODE_LSH                     2
#define MC13783_REG_MODE1_VESIMEN_LSH                       3
#define MC13783_REG_MODE1_VESIMSTBY_LSH                   4
#define MC13783_REG_MODE1_VESIMMODE_LSH                  5
#define MC13783_REG_MODE1_VCAMEN_LSH                        6
#define MC13783_REG_MODE1_VCAMSTBY_LSH                    7
#define MC13783_REG_MODE1_VCAMMODE_LSH                   8
#define MC13783_REG_MODE1_VRFBGEN_LSH                      9
#define MC13783_REG_MODE1_VRFBGSTBY_LSH                  10
#define MC13783_REG_MODE1_VVIBEN_LSH                         11
#define MC13783_REG_MODE1_VRF1EN_LSH                         12
#define MC13783_REG_MODE1_VRF1STBY_LSH                     13
#define MC13783_REG_MODE1_VRF1MODE_LSH                    14
#define MC13783_REG_MODE1_VRF2EN_LSH                         15
#define MC13783_REG_MODE1_VRF2STBY_LSH                      16
#define MC13783_REG_MODE1_VRF2MODE_LSH                     17
#define MC13783_REG_MODE1_VMMC1EN_LSH                       18
#define MC13783_REG_MODE1_VMMC1STBY_LSH                    19
#define MC13783_REG_MODE1_VMMC1MODE_LSH                    20
#define MC13783_REG_MODE1_VMMC2EN_LSH                         21
#define MC13783_REG_MODE1_VMMC2STBY_LSH                     22
#define MC13783_REG_MODE1_VMMC2MODE_LSH                     23

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
// Register 10, Arbitration switches
#define MC13783_ARB_SW_SW1ASTBYAND_WID        1
#define MC13783_ARB_SW_SW1BSTBYAND_WID        1
#define MC13783_ARB_SW_SW2ASTBYAND_WID        1
#define MC13783_ARB_SW_SW2BSTBYAND_WID        1
#define MC13783_ARB_SW_SW3SEL0_WID            1
#define MC13783_ARB_SW_SW1ABDVS_WID           1
#define MC13783_ARB_SW_SW2ABDVS_WID           1
#define MC13783_ARB_SW_SW1ASEL_WID            1
#define MC13783_ARB_SW_SW1BSEL_WID            1
#define MC13783_ARB_SW_SW2ASEL_WID            1
#define MC13783_ARB_SW_SW2BSEL_WID            1
#define MC13783_ARB_SW_SW3SEL1_WID            1
#define MC13783_ARB_SW_PLLSEL_WID             1
#define MC13783_ARB_SW_PWGT1SEL_WID           1
#define MC13783_ARB_SW_PWGT2SEL_WID           1

// Register 11, Arbitration Regulators 0
#define MC13783_ARB_REG0_VAUDIOSEL_WID                      2
#define MC13783_ARB_REG0_VIOHISEL_WID                         2
#define MC13783_ARB_REG0_VIOLOSEL_WID                         2
#define MC13783_ARB_REG0_VDIGSEL_WID                           2
#define MC13783_ARB_REG0_VGENSEL_WID                          2
#define MC13783_ARB_REG0_VRFDIGSEL_WID                      2
#define MC13783_ARB_REG0_VRFREFSEL_WID                      2
#define MC13783_ARB_REG0_VRFCPSSEL_WID                      2
#define MC13783_ARB_REG0_VSIMSEL_WID                          2
#define MC13783_ARB_REG0_VESIMSEL_WID                        2
#define MC13783_ARB_REG0_VCAMSEL_WID                         2
#define MC13783_ARB_REG0_VRFBGSEL_WID                        2

// Register 12, Arbitration Regulators 1
#define MC13783_ARB_REG1_VVIBSEL_WID                         2
#define MC13783_ARB_REG1_VRF1SEL_WID                         2
#define MC13783_ARB_REG1_VRF2SEL_WID                         2
#define MC13783_ARB_REG1_VMMC1SEL_WID                      2
#define MC13783_ARB_REG1_VMMC2SEL_WID                      2
#define MC13783_ARB_REG1_VGPO1SEL_WID                      2
#define MC13783_ARB_REG1_VGPO2SEL_WID                       2
#define MC13783_ARB_REG1_VGPO3SEL_WID                       2
#define MC13783_ARB_REG1_VGPO4SEL_WID                        2

// Register 24, Switchers 0
#define MC13783_SW0_SW1A_WID                  6
#define MC13783_SW0_SW1ADVS_WID            6
#define MC13783_SW0_SW1ASTBY_WID          6

// Register 25, Switchers 1
#define MC13783_SW1_SW1B_WID                  6
#define MC13783_SW1_SW1BDVS_WID            6
#define MC13783_SW1_SW1BSTBY_WID          6

// Register 26, Switchers 2
#define MC13783_SW2_SW2A_WID                  6
#define MC13783_SW2_SW2ADVS_WID            6
#define MC13783_SW2_SW2ASTBY_WID          6

// Register 27, Switchers 3
#define MC13783_SW3_SW2B_WID                  6
#define MC13783_SW3_SW2BDVS_WID            6
#define MC13783_SW3_SW2BSTBY_WID          6

// Register 28, Switchers 4
#define MC13783_SW4_SW1AMODE_WID                  2
#define MC13783_SW4_SW1ASTBYMODE_WID          2
#define MC13783_SW4_SW1ADVSSPEED_WID           2
#define MC13783_SW4_SW1APANIC_WID                  1
#define MC13783_SW4_SW1ASFST_WID                    1
#define MC13783_SW4_SW1BMODE_WID                   2
#define MC13783_SW4_SW1BSTBYMODE_WID           2
#define MC13783_SW4_SW1BDVSSPEED_WID            2
#define MC13783_SW4_SW1BPANIC_WID                  1
#define MC13783_SW4_SW1BSFST_WID                     1
#define MC13783_SW4_PLLEN_WID                           1
#define MC13783_SW4_PLLX_WID                           3
#define MC13783_SW4_PLLDITH_WID                      2

// Register 29, Switchers 5
#define MC13783_SW5_SW2AMODE_WID                  2
#define MC13783_SW5_SW2ASTBYMODE_WID          2
#define MC13783_SW5_SW2ADVSSPEED_WID           2
#define MC13783_SW5_SW2APANIC_WID                  1
#define MC13783_SW5_SW2ASFST_WID                    1
#define MC13783_SW5_SW2BMODE_WID                   2
#define MC13783_SW5_SW2BSTBYMODE_WID           2
#define MC13783_SW5_SW2BDVSSPEED_WID            2
#define MC13783_SW5_SW2BPANIC_WID                  1
#define MC13783_SW5_SW2BSFST_WID                    1
#define MC13783_SW5_SW3_WID                              2
#define MC13783_SW5_SW3EN_WID                          1
#define MC13783_SW5_SW3STBY_WID                      1
#define MC13783_SW5_SW3MODE_WID                     1

// Register 30, Regulator Setting 0
#define MC13783_REG_SET0_VIOLO_WID                 2
#define MC13783_REG_SET0_VDIG_WID                   2
#define MC13783_REG_SET0_VGEN_WID                   3
#define MC13783_REG_SET0_VRFDIG_WID               2
#define MC13783_REG_SET0_VRFREF_WID               2
#define MC13783_REG_SET0_VRFCP_WID                 1
#define MC13783_REG_SET0_VSIM_WID                   1
#define MC13783_REG_SET0_VESIM_WID                 1
#define MC13783_REG_SET0_VCAM_WID                   3

// Register 31, Regulator Setting 1
#define MC13783_REG_SET1_VVIB_WID                         2
#define MC13783_REG_SET1_VRF1_WID                         2
#define MC13783_REG_SET1_VRF2_WID                         2
#define MC13783_REG_SET1_VMMC1_WID                      3
#define MC13783_REG_SET1_VMMC2_WID                      3

// Register 32, Regulator Mode 0
#define MC13783_REG_MODE0_VAUDIOEN_WID                          1
#define MC13783_REG_MODE0_VAUDIOSTBY_WID                      1 
#define MC13783_REG_MODE0_VAUDIOMODE_WID                     1 
#define MC13783_REG_MODE0_VIOHIEN_WID                             1
#define MC13783_REG_MODE0_VIOHISTBY_WID                         1
#define MC13783_REG_MODE0_VIOHIMODE_WID                        1
#define MC13783_REG_MODE0_VIOLOEN_WID                            1
#define MC13783_REG_MODE0_VIOLOSTBY_WID                        1
#define MC13783_REG_MODE0_VIOLOMODE_WID                      1
#define MC13783_REG_MODE0_VDIGEN_WID                            1
#define MC13783_REG_MODE0_VDIGSTBY_WID                        1
#define MC13783_REG_MODE0_VDIGMODE_WID                         1
#define MC13783_REG_MODE0_VGENEN_WID                             1
#define MC13783_REG_MODE0_VGENSTBY_WID                          1
#define MC13783_REG_MODE0_VGENMODE_WID                         1
#define MC13783_REG_MODE0_VRFDIGEN_WID                           1
#define MC13783_REG_MODE0_VRFDIGSTBY_WID                       1
#define MC13783_REG_MODE0_VRFDIGMODE_WID                      1
#define MC13783_REG_MODE0_VRFREFEN_WID                           1
#define MC13783_REG_MODE0_VRFREFSTBY_WID                        1
#define MC13783_REG_MODE0_VRFREFMODE_WID                        1
#define MC13783_REG_MODE0_VRFCPEN_WID                               1   
#define MC13783_REG_MODE0_VRFCPSTBY_WID                           1
#define MC13783_REG_MODE0_VRFCPMODE_WID                          1

// Register 33, Regulator Mode 1
#define MC13783_REG_MODE1_VSIMEN_WID                         1
#define MC13783_REG_MODE1_VSIMSTBY_WID                     1
#define MC13783_REG_MODE1_VSIMMODE_WID                    1
#define MC13783_REG_MODE1_VESIMEN_WID                       1
#define MC13783_REG_MODE1_VESIMSTBY_WID                   1
#define MC13783_REG_MODE1_VESIMMODE_WID                  1
#define MC13783_REG_MODE1_VCAMEN_WID                        1
#define MC13783_REG_MODE1_VCAMSTBY_WID                    1
#define MC13783_REG_MODE1_VCAMMODE_WID                   1
#define MC13783_REG_MODE1_VRFBGEN_WID                      1
#define MC13783_REG_MODE1_VRFBGSTBY_WID                  1
#define MC13783_REG_MODE1_VVIBEN_WID                         1
#define MC13783_REG_MODE1_VRF1EN_WID                         1
#define MC13783_REG_MODE1_VRF1STBY_WID                     1
#define MC13783_REG_MODE1_VRF1MODE_WID                    1
#define MC13783_REG_MODE1_VRF2EN_WID                         1
#define MC13783_REG_MODE1_VRF2STBY_WID                      1
#define MC13783_REG_MODE1_VRF2MODE_WID                     1
#define MC13783_REG_MODE1_VMMC1EN_WID                       1
#define MC13783_REG_MODE1_VMMC1STBY_WID                    1
#define MC13783_REG_MODE1_VMMC1MODE_WID                    1
#define MC13783_REG_MODE1_VMMC2EN_WID                        1 
#define MC13783_REG_MODE1_VMMC2STBY_WID                     1
#define MC13783_REG_MODE1_VMMC2MODE_WID                     1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define MC13783_SW_MAX                                    0x3F
#define MC13783_SWDVS_MAX                              0x3F
#define MC13783_SWSTBY_MAX                            0x3F

#define MC13783_SW_DVSSPEED_MAX                  3

#define MC13783_SW3_MAX                                  3
#define MC13783_SW5_SW3EN_ENABLE                1
#define MC13783_SW5_SW3EN_DISABLE                0


#ifdef __cplusplus
}
#endif

#endif // __REGS_REGULATOR_H__
