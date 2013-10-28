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
//-----------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
//
//  Header:  regs_battery.h
//
//  This header file defines battery charger registers of
//  MC13783.
//
//------------------------------------------------------------------------------

#ifndef __REGS_BATTERY_H__
#define __REGS_BATTERY_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define MC13783_CHG0_VCHRG_LSH            0   //output voltage of charge regulator
#define MC13783_CHG0_ICHRG_LSH            3   //current to main charger DAC
#define MC13783_CHG0_ICHRGTR_LSH          7   //current of the trickle charger
#define MC13783_CHG0_FETOVRD_LSH          10  //BATTFET & BPFET control mode
#define MC13783_CHG0_FETCTRL_LSH          11  //BPFET & BATTFET high/low bit
#define MC13783_CHG0_RVRSMODE_LSH         13  //reverse mode en/disable
#define MC13783_CHG0_OVCTRL_LSH           15  //overvoltage threshold
#define MC13783_CHG0_UCHEN_LSH            17  //unregulated charge en/disable
#define MC13783_CHG0_CHRGLEDEN_LSH        18  //charge LED en/disable
#define MC13783_CHG0_CHRGRAWPDEN_LSH      19  //enables a 5k pulldown CHRGRAW

#define MC13783_PWR_MOD_CHRGMOD0_LSH      8   //read the charger mode pin0 state
#define MC13783_PWR_MOD_CHRGMOD1_LSH      10  //read the charger mode pin1 state

#define MC13783_PWR_CTL0_EOLSEL_LSH       18  //select EOL function instead of LOBAT
#define MC13783_PWR_CTL0_VCOIN_LSH        20  //coincell charger voltage setting
#define MC13783_PWR_CTL0_COINCHEN_LSH     23  //coincell enable/disable bit


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define MC13783_CHG0_VCHRG_WID            3
#define MC13783_CHG0_ICHRG_WID            4
#define MC13783_CHG0_ICHRGTR_WID          3
#define MC13783_CHG0_FETOVRD_WID          1
#define MC13783_CHG0_FETCTRL_WID          1 
#define MC13783_CHG0_RVRSMODE_WID         1
#define MC13783_CHG0_OVCTRL_WID           2
#define MC13783_CHG0_UCHEN_WID            1
#define MC13783_CHG0_CHRGLEDEN_WID        1
#define MC13783_CHG0_CHRGRAWPDEN_WID      1

#define MC13783_PWR_MOD_CHRGMOD0_WID		2
#define MC13783_PWR_MOD_CHRGMOD1_WID		2

#define MC13783_PWR_CTL0_EOLSEL_WID       1
#define MC13783_PWR_CTL0_VCOIN_WID        3
#define MC13783_PWR_CTL0_COINCHEN_WID     1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------


#define MC13783_CHG0_FETOVRD_DISABLED     0   //Control BATTFET and BPFET by hardware
#define MC13783_CHG0_FETOVRD_ENABLE       1   //Control BATTFET and BPFET by FETCTRL bit

#define MC13783_CHG0_FETCTRL_LOW          0   //BPFET is driven low
#define MC13783_CHG0_FETCTRL_HIGH         1   //BPFET is driven high

#define MC13783_CHG0_RVRSMODE_DISABLE     0   //Reversed mode disabled
#define MC13783_CHG0_RVRSMODE_ENABLE      1   //Reversed mode enabled

#define MC13783_CHG0_UCHEN_DISABLE        0   //Unregulated charge disabled
#define MC13783_CHG0_UCHEN_ENABLE         1   //Unregulated charge enabled

#define MC13783_CHG0_CHRGLEDEN_DISABLE    0   //Charge LED disabled
#define MC13783_CHG0_CHRGLEDEN_ENABLE     1   //Charge LED enabled

#define MC13783_CHG0_CHRGRAWPDEN_DISABLE  0   //disable 5k pulldown
#define MC13783_CHG0_CHRGRAWPDEN_ENABLE   1   //enables 5k pulldown

#define MC13783_PWR_CTL0_EOLSEL_DISABLE   0   //EOL function disabled
#define MC13783_PWR_CTL0_EOLSEL_ENABLE    1   //EOL function enabled

#define MC13783_PWR_CTL0_COINCHEN_DISABLE 0   //Coincell charger disabled
#define MC13783_PWR_CTL0_COINCHEN_ENABLE  1   //Coincell charger enabled

#ifdef __cplusplus
}
#endif

#endif // __REGS_BATTERY_H__
