//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: mx28_rtc.h
//
//  Provides definitions for the RTC (Real-Time Clock)
//  module that are common to Freescale SoCs.
//
//-----------------------------------------------------------------------------
#ifndef __MX28_RTC_H
#define __MX28_RTC_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 CTRL[4];             // 0x000 - 0x00C
    UINT32 STAT[4];         // 0x010 - 0x01C
    UINT32 MILLISECONDS[4]; // 0x020 - 0x02C
    UINT32 SECONDS[4];      // 0x030 - 0x03C
    UINT32 ALARM[4];        // 0x040 - 0x04C
    UINT32 WATCHDOG[4];     // 0x050 - 0x05C
    UINT32 PERSISTENT0[4];  // 0x060 - 0x06C
    UINT32 PERSISTENT1[4];  // 0x070 - 0x07C
    UINT32 PERSISTENT2[4];  // 0x080 - 0x08C
    UINT32 PERSISTENT3[4];  // 0x090 - 0x09C
    UINT32 PERSISTENT4[4];  // 0x0A0 - 0x0AC
    UINT32 PERSISTENT5[4];  // 0x0B0 - 0x0BC
    UINT32 DEBUGREG[4];        // 0x0C0 - 0x0CC
    UINT32 VERSION;                 // 0x0D0
} CSP_RTC_REGS, *PCSP_RTC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define RTC_CTRL_OFFSET              0x0000
#define RTC_STAT_OFFSET              0x0010
#define RTC_MILLISECONDS_OFFSET      0x0020
#define RTC_SECONDS_OFFSET           0x0030
#define RTC_ALARM_OFFSET             0x0040
#define RTC_WATCHDOG_OFFSET          0x0050
#define RTC_PERSISTENT0_OFFSET       0x0060
#define RTC_PERSISTENT1_OFFSET       0x0070
#define RTC_PERSISTENT2_OFFSET       0x0080
#define RTC_PERSISTENT3_OFFSET       0x0090
#define RTC_PERSISTENT4_OFFSET       0x00A0
#define RTC_PERSISTENT5_OFFSET       0x00B0
#define RTC_DEBUG_OFFSET             0x00C0
#define RTC_VERSION_OFFSET           0x00D0

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define  RTC_CTRL_ALARM_IRQ_EN_LSH                0
#define  RTC_CTRL_ONEMSEC_IRQ_EN_LSH              1
#define  RTC_CTRL_ALARM_IRQ_LSH                   2
#define  RTC_CTRL_ONEMSEC_IRQ_LSH                 3
#define  RTC_CTRL_WATCHDOGEN_LSH                  4
#define  RTC_CTRL_FORCE_UPDATE_LSH                5
#define  RTC_CTRL_SUPPRESS_COPY2ANALOG_LSH        6
#define  RTC_CTRL_CLKGATE_LSH                     30
#define  RTC_CTRL_SFTRST_LSH                      31

#define  RTC_PERSISTENT0_CLOCKSOURCE_LSH          0
#define  RTC_PERSISTENT0_ALARM_WAKE_EN_LSH        1
#define  RTC_PERSISTENT0_ALARM_EN_LSH             2
#define  RTC_PERSISTENT0_LCK_SECS_LSH             3
#define  RTC_PERSISTENT0_XTAL24MHZ_PWRUP_LSH      4
#define  RTC_PERSISTENT0_XTAL32KHZ_PWRUP_LSH      5
#define  RTC_PERSISTENT0_XTAL32_FREQ_LSH          6
#define  RTC_PERSISTENT0_ALARM_WAKE_LSH           7
#define  RTC_PERSISTENT0_MSEC_RES_LSH             8
#define  RTC_PERSISTENT0_DISABLE_XTALOK_LSH       13
#define  RTC_PERSISTENT0_LOWERBIAS_LSH            14
#define  RTC_PERSISTENT0_DISABLE_PSWITCH_LSH      16
#define  RTC_PERSISTENT0_AUTO_RESTART_LSH         17
#define  RTC_PERSISTENT0_ENABLE_LRADC_PWRUP_LSH   18
#define  RTC_PERSISTENT0_RELEASE_GND_LSH          19
#define  RTC_PERSISTENT0_THERMAL_RESET_LSH        20
#define  RTC_PERSISTENT0_EXTERNAL_RESET_LSH       21
#define  RTC_PERSISTENT0_SPARE_ANALOG_LSH         22
#define  RTC_PERSISTENT0_ADJ_POSLIMITBUCK_LSH     28


#define  RTC_DEBUG_WATCHDOG_RESET_MASK_LSH        1

#define RTC_VERSION_STEP_LSH                      0
#define RTC_VERSION_MINOR_LSH                     16
#define RTC_VERSION_MAJOR_LSH                     24

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define  RTC_CTRL_ALARM_IRQ_EN_WID                1
#define  RTC_CTRL_ONEMSEC_IRQ_EN_WID              1
#define  RTC_CTRL_ALARM_IRQ_WID                   1
#define  RTC_CTRL_ONEMSEC_IRQ_WID                 1
#define  RTC_CTRL_WATCHDOGEN_WID                  1
#define  RTC_CTRL_FORCE_UPDATE_WID                1
#define  RTC_CTRL_SUPPRESS_COPY2ANALOG_WID        1
#define  RTC_CTRL_CLKGATE_WID                     1
#define  RTC_CTRL_SFTRST_WID                      1

#define  RTC_PERSISTENT0_CLOCKSOURCE_WID          1
#define  RTC_PERSISTENT0_ALARM_WAKE_EN_WID        1
#define  RTC_PERSISTENT0_ALARM_EN_WID             1
#define  RTC_PERSISTENT0_LCK_SECS_WID             1
#define  RTC_PERSISTENT0_XTAL24MHZ_PWRUP_WID      1
#define  RTC_PERSISTENT0_XTAL32KHZ_PWRUP_WID      1
#define  RTC_PERSISTENT0_XTAL32_FREQ_WID          1
#define  RTC_PERSISTENT0_ALARM_WAKE_WID           1
#define  RTC_PERSISTENT0_MSEC_RES_WID             5
#define  RTC_PERSISTENT0_DISABLE_XTALOK_WID       1
#define  RTC_PERSISTENT0_LOWERBIAS_WID            2
#define  RTC_PERSISTENT0_DISABLE_PSWITCH_WID      1
#define  RTC_PERSISTENT0_AUTO_RESTART_WID         1
#define  RTC_PERSISTENT0_ENABLE_LRADC_PWRUP_WID   1
#define  RTC_PERSISTENT0_RELEASE_GND_WID          1
#define  RTC_PERSISTENT0_THERMAL_RESET_WID        1
#define  RTC_PERSISTENT0_EXTERNAL_RESET_WID       1
#define  RTC_PERSISTENT0_SPARE_ANALOG_WID         6
#define  RTC_PERSISTENT0_ADJ_POSLIMITBUCK_WID     4

#define  RTC_DEBUG_WATCHDOG_RESET_MASK_WID        1

#define RTC_VERSION_STEP_WID                      16
#define RTC_VERSION_MINOR_WID                     8
#define RTC_VERSION_MAJOR_WID                     8



//copied from rom code
#define BP_PERSIST1_NAND_SECONDARY_BOOT         1
#define BM_PERSIST1_NAND_SECONDARY_BOOT         0x00000002

#ifdef __cplusplus
}
#endif


#endif    // __MX28_RTC_H

