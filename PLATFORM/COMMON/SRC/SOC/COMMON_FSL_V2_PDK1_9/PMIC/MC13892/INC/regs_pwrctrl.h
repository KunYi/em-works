//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  regs_pwrctrl.h
//
//  This header file defines the Power Control and Power Cut Register of
//  MC13892 PMIC
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
#define MC13892_PWRCTRL0_PCEN_LSH                     0           // power cut enable
#define MC13892_PWRCTRL0_PCCOUNTEN_LSH                1           // power cut counter enable
#define MC13892_PWRCTRL0_WARMEN_LSH                   2           // warm start enable
#define MC13892_PWRCTRL0_USEROFFSPI_LSH               3           // SPI command for entering user off modes
#define MC13892_PWRCTRL0_DRM_LSH                      4            // Keeps VSRTC and CLK32KMCU on for all statescut
#define MC13892_PWRCTRL0_USEROFFCLK_LSH               5           // SPI command for entering user off modes
#define MC13892_PWRCTRL0_CLK32KMCUEN_LSH              6           // Enables the CLK32KMCU
#define MC13892_PWRCTRL0_THSEL_LSH                    8           // Thermal protection threshold select
#define MC13892_PWRCTRL0_PCUTEXPB_LSH                 9          // PCUTEXPB=1 at a startup event indicates that PCUT timer did not expire
#define MC13892_PWRCTRL0_BATTDETEN_LSH                19         // Selects EOL function instead of LOBAT
#define MC13892_PWRCTRL0_VCOIN_LSH                    20          // Coincell charger voltage setting
#define MC13892_PWRCTRL0_COINCHEN_LSH                 23        // Coincell charger enable

// Register 14, Power Control 1
#define MC13892_PWRCTRL1_PCT_LSH                      0           // power cut timer
#define MC13892_PWRCTRL1_PCCOUNT_LSH                  8           // power cut counter
#define MC13892_PWRCTRL1_PCMAXCNT_LSH                 12           // Maximum allowed number of power cuts

// Register 15, Power Control 2
#define MC13892_PWRCTRL2_RESTARTEN_LSH                 0          // Enables automatic restart after a system reset
#define MC13892_PWRCTRL2_ON1BRSTEN_LSH                 1         //Enables system reset on ON1B pin
#define MC13892_PWRCTRL2_ON2BRSTEN_LSH                 2         //Enables system reset on ON2B pin
#define MC13892_PWRCTRL2_ON3BRSTEN_LSH                 3        //Enables system reset on ON3B pin
#define MC13892_PWRCTRL2_ON1BDBNC_LSH                  4         //Sets debounce time on ON1B pin
#define MC13892_PWRCTRL2_ON2BDBNC_LSH                  6       //Sets debounce time on ON2B pin
#define MC13892_PWRCTRL2_ON3BDBNC_LSH                  8        //Sets debounce time on ON3B pin
#define MC13892_PWRCTRL2_STANDBYPRIINV_LSH            10       //If set then STANDBYPRI is interpreted as active low
#define MC13892_PWRCTRL2_STANDBYSECINV_LSH            11      //If set then STANDBYPRI is interpreted as active low
#define MC13892_PWRCTRL2_WDIRESET_LSH                 12       //Enables system reset through WDI
#define MC13892_PWRCTRL2_SPIDRV_LSH                   13      //SPI drive strength
#define MC13892_PWRCTRL2_CLK32KDRV_LSH                17       //CLK32K and CLK32KMCU drive strength (master control bits)
#define MC13892_PWRCTRL2_STBYDLY_LSH                  22      //Standby delay control

// Register 6, Power Up Mode Sense
#define MC13892_PWR_MOD_MODES_LSH                     0   //MODE sense decode
#define MC13892_PWR_MOD_PUMSS1_LSH                    2   //PUMS1 state
#define MC13892_PWR_MOD_PUMSS2_LSH                    4   //PUMS2 state
#define MC13892_PWR_MOD_I2CS_LSH                      17   //I2C Mode sense 



//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define MC13892_PWRCTRL0_PCEN_WID                      1           // power cut enable
#define MC13892_PWRCTRL0_PCCOUNTEN_WID                 1           // power cut counter enable
#define MC13892_PWRCTRL0_WARMEN_WID                    1           // warm start enable
#define MC13892_PWRCTRL0_USEROFFSPI_WID                1          // SPI command for entering user off modes
#define MC13892_PWRCTRL0_DRM_WID                       1          // Keeps VSRTC and CLK32KMCU on for all statescut
#define MC13892_PWRCTRL0_USEROFFCLK_WID                1          // SPI command for entering user off modes
#define MC13892_PWRCTRL0_CLK32KMCUEN_WID               1          // Enables the CLK32KMCU
#define MC13892_PWRCTRL0_THSEL_WID                     1          // Enables  Thermal protection threshold select
#define MC13892_PWRCTRL0_PCUTEXPB_WID                  1         // PCUTEXPB=1 at a startup event indicates that PCUT timer did not expire
#define MC13892_PWRCTRL0_BATTDETEN_WID                 1        // Selects EOL function instead of LOBAT
#define MC13892_PWRCTRL0_VCOIN_WID                     3         // Coincell charger voltage setting
#define MC13892_PWRCTRL0_COINCHEN_WID                  1       // Coincell charger enable

// Register 14, Power Control 1
#define MC13892_PWRCTRL1_PCT_WID                       8           // power cut timer
#define MC13892_PWRCTRL1_PCCOUNT_WID                   4           // power cut counter
#define MC13892_PWRCTRL1_PCMAXCNT_WID                  4          // Maximum allowed number of power cuts

// Register 15, Power Control 2
#define MC13892_PWRCTRL2_RESTARTEN_WID                 1          // Enables automatic restart after a system reset
#define MC13892_PWRCTRL2_ON1BRSTEN_WID                 1         //Enables system reset on ON1B pin
#define MC13892_PWRCTRL2_ON2BRSTEN_WID                 1         //Enables system reset on ON2B pin
#define MC13892_PWRCTRL2_ON3BRSTEN_WID                 1        //Enables system reset on ON3B pin
#define MC13892_PWRCTRL2_ON1BDBNC_WID                  2         //Sets debounce time on ON1B pin
#define MC13892_PWRCTRL2_ON2BDBNC_WID                  2       //Sets debounce time on ON2B pin
#define MC13892_PWRCTRL2_ON3BDBNC_WID                  2        //Sets debounce time on ON3B pin
#define MC13892_PWRCTRL2_STANDBYPRIINV_WID             1       //If set then STANDBYPRI is interpreted as active low
#define MC13892_PWRCTRL2_STANDBYSECINV_WID             1       //If set then STANDBYPRI is interpreted as active low
#define MC13892_PWRCTRL2_WDIRESET_WID                  1       //Enables system reset through WDI
#define MC13892_PWRCTRL2_SPIDRV_WID                    2     //SPI drive strength
#define MC13892_PWRCTRL2_CLK32KDRV_WID                 2       //CLK32K and CLK32KMCU drive strength (master control bits)
#define MC13892_PWRCTRL2_STBYDLY_WID                   2  //Standby delay control


// Register 6, Power Up Mode Sense
#define MC13892_PWR_MOD_MODES_WID                      2   //MODE sense decode
#define MC13892_PWR_MOD_PUMSS1_WID                     2   //PUMS1 state
#define MC13892_PWR_MOD_PUMSS2_WID                     2   //PUMS2 state
#define MC13892_PWR_MOD_I2CS_WID                       1  //I2C Mode sense

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define MC13892_PWRCTRL0_VBKUP_MAX                       3           

// Register 14, Power Control 1
#define MC13892_PWRCTRL1_PCT_MAX                         255           
#define MC13892_PWRCTRL1_PCCOUNT_MAX                     15          
#define MC13892_PWRCTRL1_MEMTMR_MAX                      15
#define MC13892_PWRCTRL1_MEMALLON_DISABLE                0
#define MC13892_PWRCTRL1_MEMALLON_ENABLE                 1

// Register 15, Power Control 2
#define MC13892_PWRCTRL2_ONXBDBNC_MAX                    3

// Maxium number of power cuts allowed                 
#define MC13892_PWRCTRL_PCMAXCNT_MAX                     15        


#ifdef __cplusplus
}
#endif

#endif // __REGS_PWRCTRL_H__
