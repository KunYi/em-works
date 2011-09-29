//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: mx233_rtc.h
//
//  Provides definitions for the RTC (Real-Time Clock)
//  module that are common to Freescale SoCs.
//
//-----------------------------------------------------------------------------
#ifndef __MX233_RTC_H
#define __MX233_RTC_H

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
#define RTC_ALARM_OFFSET                 0x0040
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
#define  RTC_CTRL_ALARM_IRQ_EN_LSH           0
#define  RTC_CTRL_ONEMSEC_IRQ_EN_LSH         1
#define  RTC_CTRL_ALARM_IRQ_LSH              2
#define  RTC_CTRL_ONEMSEC_IRQ_LSH            3
#define  RTC_CTRL_WATCHDOGEN_LSH             4
#define  RTC_CTRL_FORCE_UPDATE_LSH                       5
#define  RTC_CTRL_SUPPRESS_COPY2ANALOG_LSH   6
#define  RTC_CTRL_CLKGATE_LSH                            30
#define  RTC_CTRL_SFTRST_LSH                             31

#define  RTC_PERSISTENT0_CLOCKSOURCE_LSH      0
#define  RTC_PERSISTENT0_ALARM_WAKE_EN_LSH    1
#define  RTC_PERSISTENT0_ALARM_EN_LSH         2
#define  RTC_PERSISTENT0_LCK_SECS_LSH         3
#define  RTC_PERSISTENT0_XTAL24MHZ_PWRUP_LSH  4
#define  RTC_PERSISTENT0_XTAL32KHZ_PWRUP_LSH  5
#define  RTC_PERSISTENT0_XTAL32_FREQ_LSH          6
#define  RTC_PERSISTENT0_ALARM_WAKE_LSH           7
#define  RTC_PERSISTENT0_MSEC_RES_LSH             8
#define  RTC_PERSISTENT0_DISABLE_XTALOK_LSH       13
#define  RTC_PERSISTENT0_LOWERBIAS_LSH            14
#define  RTC_PERSISTENT0_DISABLE_PSWITCH_LSH  16
#define  RTC_PERSISTENT0_AUTO_RESTART_LSH         17
#define  RTC_PERSISTENT0_SPARE_ANALOG_LSH         18

#define  RTC_DEBUG_WATCHDOG_RESET_MASK_LSH        1

#define RTC_VERSION_STEP_LSH             0
#define RTC_VERSION_MINOR_LSH            16
#define RTC_VERSION_MAJOR_LSH            24

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define  RTC_CTRL_ALARM_IRQ_EN_WID           1
#define  RTC_CTRL_ONEMSEC_IRQ_EN_WID         1
#define  RTC_CTRL_ALARM_IRQ_WID              1
#define  RTC_CTRL_ONEMSEC_IRQ_WID            1
#define  RTC_CTRL_WATCHDOGEN_WID             1
#define  RTC_CTRL_FORCE_UPDATE_WID                       1
#define  RTC_CTRL_SUPPRESS_COPY2ANALOG_WID   1
#define  RTC_CTRL_CLKGATE_WID                            1
#define  RTC_CTRL_SFTRST_WID                             1

#define  RTC_PERSISTENT0_CLOCKSOURCE_WID      1
#define  RTC_PERSISTENT0_ALARM_WAKE_EN_WID    1
#define  RTC_PERSISTENT0_ALARM_EN_WID         1
#define  RTC_PERSISTENT0_LCK_SECS_WID         1
#define  RTC_PERSISTENT0_XTAL24MHZ_PWRUP_WID  1
#define  RTC_PERSISTENT0_XTAL32KHZ_PWRUP_WID  1
#define  RTC_PERSISTENT0_XTAL32_FREQ_WID          1
#define  RTC_PERSISTENT0_ALARM_WAKE_WID           1
#define  RTC_PERSISTENT0_MSEC_RES_WID             5
#define  RTC_PERSISTENT0_DISABLE_XTALOK_WID       1
#define  RTC_PERSISTENT0_LOWERBIAS_WID            2
#define  RTC_PERSISTENT0_DISABLE_PSWITCH_WID  1
#define  RTC_PERSISTENT0_AUTO_RESTART_WID         1
#define  RTC_PERSISTENT0_SPARE_ANALOG_WID         14

#define  RTC_DEBUG_WATCHDOG_RESET_MASK_WID        1

#define RTC_VERSION_STEP_WID                              0
#define RTC_VERSION_MINOR_WID                             16
#define RTC_VERSION_MAJOR_WID                             24

//------------------------------------------------------------------------------
// REGISTER BIT MASK VALUES
//------------------------------------------------------------------------------

// RTC_CTRL - Real-Time Clock Control Register

#define BP_RTC_CTRL_SFTRST                                      31
#define BM_RTC_CTRL_SFTRST                                      0x80000000
#define BF_RTC_CTRL_SFTRST(v)                           ((((UINT32) v) << 31) & BM_RTC_CTRL_SFTRST)

#define BP_RTC_CTRL_CLKGATE                                     30
#define BM_RTC_CTRL_CLKGATE                                     0x40000000
#define BF_RTC_CTRL_CLKGATE(v)                          (((v) << 30) & BM_RTC_CTRL_CLKGATE)

#define BP_RTC_CTRL_SUPPRESS_COPY2ANALOG        6
#define BM_RTC_CTRL_SUPPRESS_COPY2ANALOG        0x00000040
#define BF_RTC_CTRL_SUPPRESS_COPY2ANALOG(v) (((v) << 6) & BM_RTC_CTRL_SUPPRESS_COPY2ANALOG)

#define BP_RTC_CTRL_FORCE_UPDATE                        5
#define BM_RTC_CTRL_FORCE_UPDATE                        0x00000020
#define BF_RTC_CTRL_FORCE_UPDATE(v)                     (((v) << 5) & BM_RTC_CTRL_FORCE_UPDATE)

#define BP_RTC_CTRL_WATCHDOGEN                          4
#define BM_RTC_CTRL_WATCHDOGEN                          0x00000010
#define BF_RTC_CTRL_WATCHDOGEN(v)                       (((v) << 4) & BM_RTC_CTRL_WATCHDOGEN)

#define BP_RTC_CTRL_ONEMSEC_IRQ                         3
#define BM_RTC_CTRL_ONEMSEC_IRQ                         0x00000008
#define BF_RTC_CTRL_ONEMSEC_IRQ(v)                      (((v) << 3) & BM_RTC_CTRL_ONEMSEC_IRQ)

#define BP_RTC_CTRL_ALARM_IRQ                           2
#define BM_RTC_CTRL_ALARM_IRQ                           0x00000004
#define BF_RTC_CTRL_ALARM_IRQ(v)                        (((v) << 2) & BM_RTC_CTRL_ALARM_IRQ)

#define BP_RTC_CTRL_ONEMSEC_IRQ_EN                      1
#define BM_RTC_CTRL_ONEMSEC_IRQ_EN                      0x00000002
#define BF_RTC_CTRL_ONEMSEC_IRQ_EN(v)           (((v) << 1) & BM_RTC_CTRL_ONEMSEC_IRQ_EN)

#define BP_RTC_CTRL_ALARM_IRQ_EN                        0
#define BM_RTC_CTRL_ALARM_IRQ_EN                        0x00000001
#define BF_RTC_CTRL_ALARM_IRQ_EN(v)                     (((v) << 0) & BM_RTC_CTRL_ALARM_IRQ_EN)


// RTC_STAT - Real-Time Clock Status Register
#define BP_RTC_STAT_RTC_PRESENT                         31
#define BM_RTC_STAT_RTC_PRESENT                         0x80000000
#define BF_RTC_STAT_RTC_PRESENT(v)                      ((((UINT32) v) << 31) & BM_RTC_STAT_RTC_PRESENT)

#define BP_RTC_STAT_ALARM_PRESENT                       30
#define BM_RTC_STAT_ALARM_PRESENT                       0x40000000
#define BF_RTC_STAT_ALARM_PRESENT(v)            (((v) << 30) & BM_RTC_STAT_ALARM_PRESENT)

#define BP_RTC_STAT_WATCHDOG_PRESENT            29
#define BM_RTC_STAT_WATCHDOG_PRESENT            0x20000000
#define BF_RTC_STAT_WATCHDOG_PRESENT(v)         (((v) << 29) & BM_RTC_STAT_WATCHDOG_PRESENT)

#define BP_RTC_STAT_XTAL32000_PRESENT           28
#define BM_RTC_STAT_XTAL32000_PRESENT           0x10000000
#define BF_RTC_STAT_XTAL32000_PRESENT(v)        (((v) << 28) & BM_RTC_STAT_XTAL32000_PRESENT)

#define BP_RTC_STAT_XTAL32768_PRESENT           27
#define BM_RTC_STAT_XTAL32768_PRESENT           0x08000000
#define BF_RTC_STAT_XTAL32768_PRESENT(v)        (((v) << 27) & BM_RTC_STAT_XTAL32768_PRESENT)

#define BP_RTC_STAT_STALE_REGS                          16
#define BM_RTC_STAT_STALE_REGS                          0x00FF0000
#define BF_RTC_STAT_STALE_REGS(v)                       (((v) << 16) & BM_RTC_STAT_STALE_REGS)

#define BP_RTC_STAT_NEW_REGS                            8
#define BM_RTC_STAT_NEW_REGS                            0x0000FF00
#define BF_RTC_STAT_NEW_REGS(v)                         (((v) << 8) & BM_RTC_STAT_NEW_REGS)


// RTC_MILLISECONDS - Real-Time Clock Milliseconds Counter
#define BP_RTC_MILLISECONDS_COUNT                       0
#define BM_RTC_MILLISECONDS_COUNT                       0xFFFFFFFF
#define BF_RTC_MILLISECONDS_COUNT(v)            ((UINT32) v)

// RTC_SECONDS - Real-Time Clock Seconds Counter
#define BP_RTC_SECONDS_COUNT                            0
#define BM_RTC_SECONDS_COUNT                            0xFFFFFFFF
#define BF_RTC_SECONDS_COUNT(v)                         ((reg32_t) v)

// RTC_ALARM - Real-Time Clock Alarm Register
#define BP_RTC_ALARM_VALUE                                      0
#define BM_RTC_ALARM_VALUE                                      0xFFFFFFFF
#define BF_RTC_ALARM_VALUE(v)                           ((UINT32) v)

// RTC_WATCHDOG - Watchdog Timer Register
#define BP_RTC_WATCHDOG_COUNT                           0
#define BM_RTC_WATCHDOG_COUNT                           0xFFFFFFFF
#define BF_RTC_WATCHDOG_COUNT(v)                        ((UINT32) v)

// RTC_PERSISTENT0 - Persistent State Register 0
#define BP_RTC_PERSISTENT0_SPARE_ANALOG      18
#define BM_RTC_PERSISTENT0_SPARE_ANALOG      0xFFFC0000
#define BF_RTC_PERSISTENT0_SPARE_ANALOG(v)   ((((reg32_t) v) << 18) & BM_RTC_PERSISTENT0_SPARE_ANALOG)

#define BP_RTC_PERSISTENT0_AUTO_RESTART      17
#define BM_RTC_PERSISTENT0_AUTO_RESTART      0x00020000
#define BF_RTC_PERSISTENT0_AUTO_RESTART(v)   (((v) << 17) & BM_RTC_PERSISTENT0_AUTO_RESTART)

#define BP_RTC_PERSISTENT0_DISABLE_PSWITCH   16
#define BM_RTC_PERSISTENT0_DISABLE_PSWITCH   0x00010000
#define BF_RTC_PERSISTENT0_DISABLE_PSWITCH(v)(((v) << 16) & BM_RTC_PERSISTENT0_DISABLE_PSWITCH)

#define BP_RTC_PERSISTENT0_LOWERBIAS             14
#define BM_RTC_PERSISTENT0_LOWERBIAS             0x0000C000
#define BF_RTC_PERSISTENT0_LOWERBIAS(v)          (((v) << 14) & BM_RTC_PERSISTENT0_LOWERBIAS)

#define BP_RTC_PERSISTENT0_DISABLE_XTALOK    13
#define BM_RTC_PERSISTENT0_DISABLE_XTALOK    0x00002000
#define BF_RTC_PERSISTENT0_DISABLE_XTALOK(v) (((v) << 13) & BM_RTC_PERSISTENT0_DISABLE_XTALOK)

#define BP_RTC_PERSISTENT0_MSEC_RES                      8
#define BM_RTC_PERSISTENT0_MSEC_RES                      0x00001F00
#define BF_RTC_PERSISTENT0_MSEC_RES(v)           (((v) << 8) & BM_RTC_PERSISTENT0_MSEC_RES)

#define BP_RTC_PERSISTENT0_ALARM_WAKE            7
#define BM_RTC_PERSISTENT0_ALARM_WAKE            0x00000080
#define BF_RTC_PERSISTENT0_ALARM_WAKE(v)         (((v) << 7) & BM_RTC_PERSISTENT0_ALARM_WAKE)

#define BP_RTC_PERSISTENT0_XTAL32_FREQ           6
#define BM_RTC_PERSISTENT0_XTAL32_FREQ       0x00000040
#define BF_RTC_PERSISTENT0_XTAL32_FREQ(v)    (((v) << 6) & BM_RTC_PERSISTENT0_XTAL32_FREQ)

#define BP_RTC_PERSISTENT0_XTAL32KHZ_PWRUP   5
#define BM_RTC_PERSISTENT0_XTAL32KHZ_PWRUP   0x00000020
#define BF_RTC_PERSISTENT0_XTAL32KHZ_PWRUP(v) (((v) << 5) & BM_RTC_PERSISTENT0_XTAL32KHZ_PWRUP)

#define BP_RTC_PERSISTENT0_XTAL24MHZ_PWRUP   4
#define BM_RTC_PERSISTENT0_XTAL24MHZ_PWRUP   0x00000010
#define BF_RTC_PERSISTENT0_XTAL24MHZ_PWRUP(v)   (((v) << 4) & BM_RTC_PERSISTENT0_XTAL24MHZ_PWRUP)

#define BP_RTC_PERSISTENT0_LCK_SECS                      3
#define BM_RTC_PERSISTENT0_LCK_SECS                      0x00000008
#define BF_RTC_PERSISTENT0_LCK_SECS(v)           (((v) << 3) & BM_RTC_PERSISTENT0_LCK_SECS)

#define BP_RTC_PERSISTENT0_ALARM_EN                      2
#define BM_RTC_PERSISTENT0_ALARM_EN                      0x00000004
#define BF_RTC_PERSISTENT0_ALARM_EN(v)           (((v) << 2) & BM_RTC_PERSISTENT0_ALARM_EN)

#define BP_RTC_PERSISTENT0_ALARM_WAKE_EN     1
#define BM_RTC_PERSISTENT0_ALARM_WAKE_EN     0x00000002
#define BF_RTC_PERSISTENT0_ALARM_WAKE_EN(v)  (((v) << 1) & BM_RTC_PERSISTENT0_ALARM_WAKE_EN)

#define BP_RTC_PERSISTENT0_CLOCKSOURCE       0
#define BM_RTC_PERSISTENT0_CLOCKSOURCE       0x00000001
#define BF_RTC_PERSISTENT0_CLOCKSOURCE(v)    (((v) << 0) & BM_RTC_PERSISTENT0_CLOCKSOURCE)

// RTC_PERSISTENT1 - Persistent State Register 1

#define BP_RTC_PERSISTENT1_GENERAL      0
#define BM_RTC_PERSISTENT1_GENERAL      0xFFFFFFFF
#define BF_RTC_PERSISTENT1_GENERAL(v)   ((reg32_t) v)

#define BV_RTC_PERSISTENT1_GENERAL__ENUMERATE_500MA_TWICE  0x1000
#define BV_RTC_PERSISTENT1_GENERAL__USB_BOOT_PLAYER_MODE   0x0800
#define BV_RTC_PERSISTENT1_GENERAL__SKIP_CHECKDISK         0x0400
#define BV_RTC_PERSISTENT1_GENERAL__USB_LOW_POWER_MODE     0x0200
#define BV_RTC_PERSISTENT1_GENERAL__OTG_HNP_BIT            0x0100
#define BV_RTC_PERSISTENT1_GENERAL__OTG_ATL_ROLE_BIT       0x0080

//copied from rom code
#define BP_PERSIST1_NAND_SECONDARY_BOOT         1
#define BM_PERSIST1_NAND_SECONDARY_BOOT         0x00000002

// RTC_PERSISTENT2 - Persistent State Register 2
#define BP_RTC_PERSISTENT2_GENERAL      0
#define BM_RTC_PERSISTENT2_GENERAL      0xFFFFFFFF
#define BF_RTC_PERSISTENT2_GENERAL(v)   ((UINT32) v)

// RTC_PERSISTENT3 - Persistent State Register 3
#define BP_RTC_PERSISTENT3_GENERAL      0
#define BM_RTC_PERSISTENT3_GENERAL      0xFFFFFFFF
#define BF_RTC_PERSISTENT3_GENERAL(v)   ((UINT32) v)

// RTC_PERSISTENT4 - Persistent State Register 4
#define BP_RTC_PERSISTENT4_GENERAL      0
#define BM_RTC_PERSISTENT4_GENERAL      0xFFFFFFFF
#define BF_RTC_PERSISTENT4_GENERAL(v)   ((UINT32) v)

// RTC_PERSISTENT5 - Persistent State Register 5
#define BP_RTC_PERSISTENT5_GENERAL      0
#define BM_RTC_PERSISTENT5_GENERAL      0xFFFFFFFF
#define BF_RTC_PERSISTENT5_GENERAL(v)   ((reg32_t) v)

// RTC_DEBUG - Real-Time Clock Debug Register
#define BP_RTC_DEBUG_WATCHDOG_RESET_MASK      1
#define BM_RTC_DEBUG_WATCHDOG_RESET_MASK      0x00000002
#define BF_RTC_DEBUG_WATCHDOG_RESET_MASK(v)   (((v) << 1) & BM_RTC_DEBUG_WATCHDOG_RESET_MASK)

#define BP_RTC_DEBUG_WATCHDOG_RESET      0
#define BM_RTC_DEBUG_WATCHDOG_RESET      0x00000001
#define BF_RTC_DEBUG_WATCHDOG_RESET(v)   (((v) << 0) & BM_RTC_DEBUG_WATCHDOG_RESET)

// HW_RTC_VERSION - Real-Time Clock Version Register
#define BP_RTC_VERSION_MAJOR      24
#define BM_RTC_VERSION_MAJOR      0xFF000000
#define BF_RTC_VERSION_MAJOR(v)   ((((reg32_t) v) << 24) & BM_RTC_VERSION_MAJOR)

#define BP_RTC_VERSION_MINOR      16
#define BM_RTC_VERSION_MINOR      0x00FF0000
#define BF_RTC_VERSION_MINOR(v)   (((v) << 16) & BM_RTC_VERSION_MINOR)

#define BP_RTC_VERSION_STEP      0
#define BM_RTC_VERSION_STEP      0x0000FFFF
#define BF_RTC_VERSION_STEP(v)   (((v) << 0) & BM_RTC_VERSION_STEP)


#ifdef __cplusplus
}
#endif


#endif    // __MX233_RTC_H

