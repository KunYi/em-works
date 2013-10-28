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
//  Header:  regs_pwrctrl.h
//
//  This header file defines the Power Control and Power Cut Register of
//  MC13783 PMIC
//
//------------------------------------------------------------------------------

#ifndef __REGS_PWRCTRL_H__
#define __REGS_PWRCTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

// Power control and power cut register
// Register 13, Power Control 0
#define MC13783_PWRCTRL0_PCEN_LSH                           0           // power cut enable
#define MC13783_PWRCTRL0_PCCOUNTEN_LSH                1           // power cut counter enable
#define MC13783_PWRCTRL0_WARMEN_LSH                     2           // warm start enable
#define MC13783_PWRCTRL0_USEROFFSPI_LSH               3           // SPI command for entering user off modes
#define MC13783_PWRCTRL0_USEROFFPC_LSH                4            // Automatic transition to user off during power cut
#define MC13783_PWRCTRL0_USEROFFCLK_LSH               5           // SPI command for entering user off modes
#define MC13783_PWRCTRL0_CLK32KMCUEN_LSH             6           // Enables the CLK32KMCU
#define MC13783_PWRCTRL0_VBKUP1EN_LSH                   8          //Enables VBKUP1 regulator
#define MC13783_PWRCTRL0_VBKUP1AUTO_LSH               9         // Automatically enables VBKUP1 during power cut
#define MC13783_PWRCTRL0_VBKUP1_LSH                        10        // Sets VBKUP1 voltage 
#define MC13783_PWRCTRL0_VBKUP2EN_LSH                   12         // Enables VBKUP2 regulator
#define MC13783_PWRCTRL0_VBKUP2AUTO_LSH               13         // Automatically enables VBKUP2 during power cut
#define MC13783_PWRCTRL0_VBKUP2_LSH                        14         // Sets VBKUP2 voltage
#define MC13783_PWRCTRL0_BPDET_LSH                         16          // BP detection threshold setting
#define MC13783_PWRCTRL0_EOLSEL_LSH                         18         // Selects EOL function instead of LOBAT
#define MC13783_PWRCTRL0_VCOIN_LSH                          20          // Coincell charger voltage setting
#define MC13783_PWRCTRL0_COINCHEN_LSH                     23        // Coincell charger enable

// Register 14, Power Control 1
#define MC13783_PWRCTRL1_PCT_LSH                             0           // power cut timer
#define MC13783_PWRCTRL1_PCCOUNT_LSH                    8           // power cut counter
#define MC13783_PWRCTRL1_PCMAXCNT_LSH                  12           // Maximum allowed number of power cuts
#define MC13783_PWRCTRL1_MEMTMR_LSH                     16           // extended power cut timer (memory hold timer)
#define MC13783_PWRCTRL1_MEMALLON_LSH                 20           // extended power cut timer set to infinite

// Register 15, Power Control 2
#define MC13783_PWRCTRL2_RESTARTEN_LSH                 0          // Enables automatic restart after a system reset

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
// Register 13, Power Control 0
#define MC13783_PWRCTRL0_PCEN_WID                              1           
#define MC13783_PWRCTRL0_PCCOUNTEN_WID                      1           
#define MC13783_PWRCTRL0_WARMEN_WID                         1           
#define MC13783_PWRCTRL0_USEROFFSPI_WID                         1           
#define MC13783_PWRCTRL0_USEROFFPC_WID                      1           
#define MC13783_PWRCTRL0_USEROFFCLK_WID                  1           
#define MC13783_PWRCTRL0_CLK32KMCUEN_WID                 1           
#define MC13783_PWRCTRL0_VBKUP1EN_WID                       1           
#define MC13783_PWRCTRL0_VBKUP1AUTO_WID                  1           
#define MC13783_PWRCTRL0_VBKUP1_WID                                2           
#define MC13783_PWRCTRL0_VBKUP2EN_WID                       1           
#define MC13783_PWRCTRL0_VBKUP2AUTO_WID                  1           
#define MC13783_PWRCTRL0_VBKUP2_WID                                2           
#define MC13783_PWRCTRL0_BPDET_WID                             2           
#define MC13783_PWRCTRL0_EOLSEL_WID                                1           
#define MC13783_PWRCTRL0_VCOIN_WID                             3
#define MC13783_PWRCTRL0_COINCHEN_WID                       1           

// Register 14, Power Control 1
#define MC13783_PWRCTRL1_PCT_WID                             8           // power cut timer
#define MC13783_PWRCTRL1_PCCOUNT_WID                    4           // power cut counter
#define MC13783_PWRCTRL1_PCMAXCNT_WID                  4           // Maximum allowed number of power cuts
#define MC13783_PWRCTRL1_MEMTMR_WID                     4           // extended power cut timer (memory hold timer)
#define MC13783_PWRCTRL1_MEMALLON_WID                 1           // extended power cut timer set to infinite

#define MC13783_PWRCTRL_RESTARTEN_WID                 1          // Enables automatic restart after a system reset

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
// Register 13, Power Control 0
#define MC13783_PWRCTRL0_PCEN_DISABLE                    0           
#define MC13783_PWRCTRL0_PCEN_ENABLE                     1           
#define MC13783_PWRCTRL0_PCCOUNTEN_DISABLE     0
#define MC13783_PWRCTRL0_PCCOUNTEN_ENABLE      1
#define MC13783_PWRCTRL0_WARMEN_DISABLE                  0
#define MC13783_PWRCTRL0_WARMEN_ENABLE               1
#define MC13783_PWRCTRL0_USEROFFSPI_DISABLE           0
#define MC13783_PWRCTRL0_USEROFFSPI_ENABLE        1
#define MC13783_PWRCTRL0_CLK32KMCUEN_DISABLE           0
#define MC13783_PWRCTRL0_CLK32KMCUEN_ENABLE           1
#define MC13783_PWRCTRL0_VBKUP_MAX                                3           

// Register 14, Power Control 1
#define MC13783_PWRCTRL1_PCT_MAX                               255           
#define MC13783_PWRCTRL1_PCCOUNT_MAX                     15          
#define MC13783_PWRCTRL1_MEMTMR_MAX                      15
#define MC13783_PWRCTRL1_MEMALLON_DISABLE               0
#define MC13783_PWRCTRL1_MEMALLON_ENABLE                1

// Maxium number of power cuts allowed                 
#define MC13783_PWRCTRL_PCMAXCNT_MAX                   15        


#ifdef __cplusplus
}
#endif

#endif // __REGS_PWRCTRL_H__
