//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  regs_adc.h
//
//  This header file defines ADC (includes the Touch Screen) registers of
//  MC13892.
//
//------------------------------------------------------------------------------

#ifndef __MC13892_REGS_ADC_H__
#define __MC13892_REGS_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

#define MC13892_ADC0_LICELLCON_LSH   0
#define MC13892_ADC0_CHRGICON_LSH    1
#define MC13892_ADC0_BATICON_LSH     2
#define MC13892_ADC0_BUFFEN_LSH      3
#define MC13892_ADC0_ADIN7SEL0_LSH   4
#define MC13892_ADC0_ADIN7SEL1_LSH   5
#define MC13892_ADC0_ADRESET_LSH     8
#define MC13892_ADC0_ADIN7DIV_LSH    9
#define MC13892_ADC0_TSREFEN_LSH     10
#define MC13892_ADC0_TSMOD_LSH       12
#define MC13892_ADC0_CHRGRAWDIV_LSH  15
#define MC13892_ADC0_ADINC1_LSH      16
#define MC13892_ADC0_ADINC2_LSH      17
#define MC13892_ADC0_ADCBIS0_LSH     23


#define MC13892_ADC1_ADEN_LSH        0
#define MC13892_ADC1_RAND_LSH        1
#define MC13892_ADC1_ADCCA_LSH       2
#define MC13892_ADC1_AD_SEL_LSH      3
#define MC13892_ADC1_TRIGMASK_LSH    4
#define MC13892_ADC1_ADA1_LSH        5
#define MC13892_ADC1_ADA2_LSH        8
#define MC13892_ADC1_ATO_LSH         11
#define MC13892_ADC1_ATOX_LSH        19
#define MC13892_ADC1_ASC_LSH         20
#define MC13892_ADC1_ADTRIGIGN_LSH   21
#define MC13892_ADC1_ADONESHOT_LSH   22
#define MC13892_ADC1_ADCBIS1_LSH     23

#define MC13892_ADC2_ADD1_LSH        2
#define MC13892_ADC2_ADD2_LSH        14

#define MC13892_ADC3_ICID_LSH        6

#define MC13892_ADC4_ADDBIS1_LSH     2
#define MC13892_ADC4_ADDBIS2_LSH     14

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

#define MC13892_ADC0_LICELLCON_WID   1
#define MC13892_ADC0_CHRGICON_WID    1
#define MC13892_ADC0_BATICON_WID     1
#define MC13892_ADC0_RTHEN_WID       1
#define MC13892_ADC0_DTHEN_WID       1
#define MC13892_ADC0_ADOUTEN_WID     1
#define MC13892_ADC0_ADOUTPER_WID    1
#define MC13892_ADC0_ADREFEN_WID     1
#define MC13892_ADC0_ADREFMOD_WID    1
#define MC13892_ADC0_TSREFEN_WID     1
#define MC13892_ADC0_TSMOD_WID       3
#define MC13892_ADC0_ADINC1_WID      1
#define MC13892_ADC0_ADINC2_WID      1
#define MC13892_ADC0_WCOMP_WID       1
#define MC13892_ADC0_ADCBIS0_WID     1

#define MC13892_ADC1_ADEN_WID        1
#define MC13892_ADC1_RAND_WID        1
#define MC13892_ADC1_AD_SEL_WID      1
#define MC13892_ADC1_ADA1_WID        3
#define MC13892_ADC1_ADA2_WID        3
#define MC13892_ADC1_ATO_WID         8
#define MC13892_ADC1_ATOX_WID        1
#define MC13892_ADC1_ASC_WID         1
#define MC13892_ADC1_ADTRIGIGN_WID   1
#define MC13892_ADC1_ADONESHOT_WID   1
#define MC13892_ADC1_ADCBIS1_WID     1

#define MC13892_ADC2_ADD1_WID        10
#define MC13892_ADC2_ADD2_WID        10

#define MC13892_ADC3_WHIGH_WID       6
#define MC13892_ADC3_ICID_WID        3
#define MC13892_ADC3_WLOW_WID        6
#define MC13892_ADC3_ADCBIS2_WID     1

#define MC13892_ADC4_ADDBIS1_WID     10
#define MC13892_ADC4_ADDBIS2_WID     10

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
// ADC0
#define MC13892_ADC0_ADINC_NO_INCR       0
#define MC13892_ADC0_ADINC_AUTO_INCR     1

#define MC13892_ADC0_DISABLE             0
#define MC13892_ADC0_ENABLE              1

// ADC1
#define MC13892_ADC1_ADEN_DISABLE        0
#define MC13892_ADC1_ADEN_ENABLE         1

#define MC13892_ADC1_RAND_8CHAN_1X       0       // 1 sample, 8 channel conversion
#define MC13892_ADC1_RAND_1CHAN_8X       1       // 8 sample, 1 channel conversion

#define MC13892_ADC1_AD_SEL_GROUP0       0       // Group 0 A/D channels
#define MC13892_ADC1_AD_SEL_GROUP1       1       // Group 1 A/D channels

#define MC13892_ADC1_ASC_ADC_IDLE        0       // A/D comversion is complete
#define MC13892_ADC1_ASC_START_ADC       1       // Start A/D conversions

#define MC13892_ADC1_ATOX_DELAY_FIRST    0       // Delay only before first conversion
#define MC13892_ADC1_ATOX_DELAY_EACH     1       // Delay before each conversion

#define MC13892_ADC1_ADTRIGIGN_USE       0       // not ignore the ADTRIG input
#define MC13892_ADC1_ADTRIGIGN_IGNORE    1       // Ignore the ADTRIG input


#define MC13892_ADC1_ADA_SEL0_BATT       0        // BATT battery voltage when AD_SEL = 0
#define MC13892_ADC1_ADA_SEL0_BATTISNS   1        // BATTISNS battery current when AD_SEL = 0
#define MC13892_ADC1_ADA_SEL0_BPSNS      2        // BPSNS application supply when AD_SEL = 0
#define MC13892_ADC1_ADA_SEL0_CHRGRAW    3        // CHRGRAW charger voltage when AD_SEL = 0
#define MC13892_ADC1_ADA_SEL0_CHRGISNS   4        // CHRGRAW charger current when AD_SEL = 0
#define MC13892_ADC1_ADA_SEL0_ADIN5      5        // ADIN5 battery pack thermistor when AD_SEL = 0
#define MC13892_ADC1_ADA_SEL0_ADIN6      6         // ADIN6  voltage when AD_SEL = 0
#define MC13892_ADC1_ADA_SEL0_ADIN7      7        // ADIN 7 USBID or Die temperature when AD_SEL = 0


#define MC13892_ADC1_ADA_SEL1_TSX1       0        // TSX1 A/D when AD_SEL = 1
#define MC13892_ADC1_ADA_SEL1_TSX2       1        // TSX2 A/D when AD_SEL = 1
#define MC13892_ADC1_ADA_SEL1_TSY1       2        // TSY1 A/D when AD_SEL = 1
#define MC13892_ADC1_ADA_SEL1_TSY2       3        // TSY2 A/D when AD_SEL = 1

// To specify a channel easily as a 1-bit of UINT16 by CSP_BITFVAL and CSP_BITFMASK macros, 
// we define the A/D Channels as follows:

#define MC13892_ADC_BATT_LSH        0
#define MC13892_ADC_BATTISNS_LSH    1
#define MC13892_ADC_BPSNS_LSH       2
#define MC13892_ADC_CHRGRAW_LSH     3
#define MC13892_ADC_CHRGISNS_LSH    4
#define MC13892_ADC_ADIN5_LSH       5
#define MC13892_ADC_ADIN6_LSH       6
#define MC13892_ADC_ADIN7_LSH       7
#define MC13892_ADC_TSX1_LSH        8
#define MC13892_ADC_TSX2_LSH        9
#define MC13892_ADC_TSY1_LSH        10
#define MC13892_ADC_TSY2_LSH        11

#define MC13892_ADC_BATT_WID        1
#define MC13892_ADC_BATTISNS_WID    1
#define MC13892_ADC_BPSNS_WID       1
#define MC13892_ADC_CHRGRAW_WID     1
#define MC13892_ADC_CHRGISNS_WID    1
#define MC13892_ADC_ADIN5_WID       1
#define MC13892_ADC_ADIN6_WID       1
#define MC13892_ADC_ADIN7_WID       1
#define MC13892_ADC_TSX1_WID        1
#define MC13892_ADC_TSX2_WID        1
#define MC13892_ADC_TSY1_WID        1
#define MC13892_ADC_TSY2_WID        1

#ifdef __cplusplus
}
#endif

#endif // __MC13892_REGS_ADC_H__
