/*---------------------------------------------------------------------------
* Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
//
//  Header:  regs_battery.h
//
//  This header file defines battery charger registers of
//  MC13892.
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
#define MC13892_CHG0_VCHRG_LSH            0   //output voltage of charge regulator
#define MC13892_CHG0_ICHRG_LSH            3   //current to main charger DAC
#define MC13892_CHG0_TREN_LSH             7   //Enables the internal trickle charger current
#define MC13892_CHG0_ACKLPB_LSH           8   //Acknowledge Low Power Boot
#define MC13892_CHG0_THCHKB_LSH           9   //Battery thermistor check disable
#define MC13892_CHG0_FETOVRD_LSH          10  //BATTFET & BPFET control mode
#define MC13892_CHG0_FETCTRL_LSH          11  //BPFET & BATTFET high/low bit
#define MC13892_CHG0_RVRSMODE_LSH         13  //reverse mode en/disable
#define MC13892_CHG0_PLIM_LSH             15  //Power limiter setting
#define MC13892_CHG0_PLIMDIS_LSH          17  //Power limiter disable
#define MC13892_CHG0_CHRGLEDEN_LSH        18  //charge LED en/disable
#define MC13892_CHG0_CHGTMRRST_LSH        19  //Charge timer reset
#define MC13892_CHG0_CHGRESTART_LSH       20  //Restarts charger state machine
#define MC13892_CHG0_CHGAUTOB_LSH         21  //Avoids automatic charging while on
#define MC13892_CHG0_CYCLB_LSH            22  //Disables cycling
#define MC13892_CHG0_CHGAUTOVIB_LSH       23  //Allows V and I programming

#define MC13892_USB1_IDPUCNTRL_LSH        22  //UID pin pull up source select

#define MC13892_CHGUSB1_VUSBIN_LSH        0  //Slave or Host configuration for VBUS
#define MC13892_CHGUSB1_VUSBEN_LSH        3  //VUSB enable
#define MC13892_CHGUSB1_ID100KPU_LSH      8  //Switches in 100K UID pull-up
#define MC13892_CHGUSB1_OTGSWBSTEN_LSH    10  //Enable SWBST for USB OTG mode

#define MC13892_PWR_MOD_CHRGSSS_LSH       8   //Charger Serial/Single mode sense
#define MC13892_PWR_MOD_CHRGSE1BS_LSH     9   //CHRGSE1BS sense bit

#define MC13892_PWR_CTL0_DRM_LSH          4   //Keeps VSRTC and CLK32KMCU on
#define MC13892_PWR_CTL0_VCOIN_LSH        20  //coincell charger voltage setting
#define MC13892_PWR_CTL0_COINCHEN_LSH     23  //coincell enable/disable bit


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define MC13892_CHG0_VCHRG_WID            3   
#define MC13892_CHG0_ICHRG_WID            4 
#define MC13892_CHG0_TREN_WID             1   
#define MC13892_CHG0_ACKLPB_WID           1  
#define MC13892_CHG0_THCHKB_WID           1   
#define MC13892_CHG0_FETOVRD_WID          1 
#define MC13892_CHG0_FETCTRL_WID          1
#define MC13892_CHG0_RVRSMODE_WID         1 
#define MC13892_CHG0_PLIM_WID             2  
#define MC13892_CHG0_PLIMDIS_WID          1  
#define MC13892_CHG0_CHRGLEDEN_WID        1
#define MC13892_CHG0_CHGTMRRST_WID        1  
#define MC13892_CHG0_CHGRESTART_WID       1  
#define MC13892_CHG0_CHGAUTOB_WID         1  
#define MC13892_CHG0_CYCLB_WID            1 
#define MC13892_CHG0_CHGAUTOVIB_WID       1  

#define MC13892_USB1_IDPUCNTRL_WID        1 

#define MC13892_CHGUSB1_VUSBIN_WID        1  
#define MC13892_CHGUSB1_VUSBEN_WID        1 
#define MC13892_CHGUSB1_ID100KPU_WID      1  
#define MC13892_CHGUSB1_OTGSWBSTEN_WID    1  

#define MC13892_PWR_MOD_CHRGSSS_WID       1   
#define MC13892_PWR_MOD_CHRGSE1BS_WID     1   

#define MC13892_PWR_CTL0_DRM_WID          1
#define MC13892_PWR_CTL0_VCOIN_WID        3 
#define MC13892_PWR_CTL0_COINCHEN_WID     1

#ifdef __cplusplus
}
#endif

#endif // __REGS_BATTERY_H__
