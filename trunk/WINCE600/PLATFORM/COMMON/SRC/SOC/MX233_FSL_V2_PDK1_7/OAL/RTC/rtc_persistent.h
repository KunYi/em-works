//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: rtc_persistent.h
//
//
//
//
//-----------------------------------------------------------------------------
#ifndef _RTC_PERSISTENT_H
#define _RTC_PERSISTENT_H

#if __cplusplus
extern "C" {
#endif

typedef enum
{
    //! \brief Sets the crystal oscillator source for the 32KHz clock domain
    //! used by the RTC analog domain circuits.
    //!
    //! Set to 1 for 32KHz crystal oscillator; set to 0 for 24MHz crystal oscillator.
    RTC_CLOCKSOURCE,
    //! \brief Powers the chip upon arrival of an alarm event.
    //!
    //! Set this bit to 1 to power up the chip upon the arrival of an alarm event.
    //! ALARM_EN must be set to 1 to enable the detection of an alarm event.
    //!
    //! \note This bit is reset by writing a 0 directly to the shadow register
    //! which causes the copy controller to move it across to the analog domain.
    RTC_ALARM_WAKE_EN,
    //!
    //! Set this bit to 1 to enable alarm event generation.
    //! \brief Enable generation of an alarm event.
    RTC_ALARM_EN,
    //! \brief Powers the chip upon arrival of an alarm event.
    //!
    //! Set this bit to 1 to power up the chip upon the arrival of an alarm event.
    //! ALARM_EN must be set to 1 to enable the detection of an alarm event.
    //!
    //! \note This bit is reset by writing a 0 directly to the shadow register
    //! which causes the copy controller to move it across to the analog domain.
    RTC_ALARM_WAKE,
    //! Setting this to one prevents the RTC seconds from being modified,
    //! or this bit from being modified.   Supported on the 37xx only.
    //! \brief protects the RTC from being changed
    RTC_LCK_SECS,
    //! Controls the resolution of the millisecond timer of the RTC. Acceptable values are 1,2,4,8,16.
    //! \brief Controls the resolution of the millisecond timer of the RTC
    //! \note only supported on the 37xx devices.
    RTC_MSEC_RES,
    //! Sets the time differential from the internal secure clock to the current local time.
    //! Input value is a 32 bit number of seconds from the internal secure clock.
    RTC_USER_CLOCK_OFFSET,

    //! \brief Powers down the 24MHz crystal oscillator.
    //!
    //! Set to 1 to power down the 24MHz crystal oscillator and its power domain, including the
    //! real time clock and persistence bits (default).  Set to zero to enable the crystal
    //! oscillator and its power domain to remain on while the rest of the chip is in the power
    //! down state.
    RTC_XTAL24MHZ_PDOWN,
    //! \brief the inversion of RTC_XTAL24MHZ_PDOWN
    //!
    //! This is the logical inversion of RTC_XTAL24MHZ_PDOWN.
    RTC_XTAL24MHZ_PWRUP,
    //! \brief Powers down the 32.768KHz crystal oscillator.
    //!
    //! Set to 1 to power down the 32.768KHz crystal oscillator and its power domain, including
    //! the real time clock and persistence bits (default).  Set to zero to enable the crystal
    //! oscillator and its power domain to remain on while the rest of the chip is in the power
    //! down state.
    RTC_XTAL32KHZ_PDOWN,
    //! \brief the inversion of RTC_XTAL32KHZ_PDOWN
    //!
    //! This is the logical inversion of RTC_XTAL32KHZ_PDOWN.
    RTC_XTAL32KHZ_PWRUP,
    //! \brief Disables the circuit that resets the chip if 24MHz frequency falls below 2MHz.
    //!
    //! Set to 1 to disable the circuit that resets the chip if 24MHz frequency falls below 2MHz.
    //! The circuit defaults to enabled and will power down the device if the 24MHz stop
    //! oscillating for any reason.
    //!
    //! \warning Freescale Proprietary! Contact Freescale before changing default value.
    RTC_DISABLE_XTALSTOP,
    //! \brief the inversion of RTC_DISABLE_XTALSTOP
    //!
    //! This is the logical inversion of RTC_DISABLE_XTALSTOP.
    RTC_DISABLE_XTALOK,
    //! \brief Sets the exact frequency of the 32K crystal.
    //!
    //! If the value set for this persistent register is RTC_XTAL32_32768, a 32768Hz crystal is being used.
    //! If the value set for this persistent register is RTC_XTAL32_32000, a 32KHz crystal is being used.
    //! All other values return an error.
    RTC_XTAL32_FREQ,
    //! \brief Reduces the bias current of the 24MHz crystal by 25% or 50%
    //!
    //! This persistant register controls the bias current reduction.  It can be one of RTC_LOWER_BIAS_NONE,
    //! RTC_LOWER_BIAS_25, or RTC_LOWER_BIAS_50.
    RTC_LOWER_BIAS,
    //! \brief Protects the PSWITCH from activating the chip unless the voltage rises above VDDxtal.
    //!
    //! Protects the PSWITCH from activating the chip unless the voltage rises above VDDxtal.  Only on the 37xx
    RTC_DISABLE_PSWITCH,
    //! \brief Enables the chip to automatically power up approximately 180ms after powering down.
    //!
    //! Set to 1 to enable the chip to automatically power up.
    //! \warning Freescale Proprietary! Contact Freescale before changing default value.
    RTC_AUTO_RESTART,
    //! \brief Enables 24MHz crystal to turn off when the battery falls below threshold
    //!
    //! Set to 1 to enable 24MHz crystal, in an RTC application, to turn off when the battery falls below threshold.
    //! The threshold is determined by LOWBAT_3P0.
    //! \note Only supported on the 36xx chip family.
    //! \warning Freescale Proprietary! Contact Freescale before changing default value.
    RTC_DETECT_LOBAT,
    //! \brief Changes Lithium Ion low battery threshold to 3.0V
    //!
    //! Set to 1 to change Lithium Ion low battery threshold to 3.0V. Set to 0 for 2.7V threshold.  ONly available on
    //! the 36xx chip family.
    //! \warning Freescale Proprietary! Contact Freescale before changing default value.
    RTC_LOWBAT_3PO,
    //! \brief Enables the self bias circuit to remain powered up when the device is powered down.
    //!
    //! Set to 1 to enable the self bias circuit.   Only available on the 36xx chip family
    //! \warning Freescale Proprietary! Contact Freescale before changing default value.
    RTC_SELFBIAS_PWRUP,
    //! \brief Enables startup using internal oscillator.
    //!
    //! Set to 1 to enable startup using internal oscillator until power up sequence is complete.
    //! Only supported by the 36xx chip family.
    //!
    //! \note Do not set unless specifically directed by Freescale!
    //! \warning Freescale Proprietary! Contact Freescale before changing default value.
    RTC_SD_PRESENT,
    //! \brief ETM enable bit (0 = disabled, 1 = enabled)
    //!
    //! Set to 1 and ROM will enable ETM @ 4ma drive.  Only available on the 36xx chip family.
    RTC_ETM_ENABLE,
    //! \brief Index into internal ROM table of SDRAM settings.
    //!
    //! \note Not generally recommended. Better to load a small program to customize SDRAM.
    //! \note Only available for the 36xx platform
    RTC_SDRAM_NDX,
    //! \brief Chip select (0-3) that ROM should use when enabling SDRAM
    //!
    //! \note Not generally recommended. Better to load a small program to customize SDRAM.
    //! \note Only available for the 36xx platform
    RTC_SDRAM_CS,
    //! \brief USB role.
    RTC_OTG_ATL_ROLE_BIT,
    //! \brief HNP has been requested if set to one.
    RTC_OTG_HNP_BIT,
    //! \brief Boot to player mode when connected to USB.
    RTC_USB_BOOT_PLAYER_MODE,
    //! \brief Enable SDRAM Warm Boot
    //!
    //! \note Not generally recommended. ROM brings SDRAM out of self-refresh and jumps to fixed addr.
    //! \note only available on the 36xx family of chips
    RTC_SDRAM_BOOT,
    //! \brief Forces recovery mode in the ROM
    RTC_FORCE_RECOVERY,
    //! \brief Informs the ROM to boot using the secondary copy of the boot code.
    //! Causes the ROM to use the secondary copy of the boot code.
    //! \note Only available on the 37xx family of devices
    RTC_NAND_SECONDARY_BOOT,
    //! \brief Lets the ROM inform the application that it needs to attend to the boot images.
    //! When the ROM detects a failed or failing boot image, this bit is set.  The application should
    //! perform maintenance on the boot images to ensure there isn't a catastrophic boot failure.
    //! \note Only implemented on the 37xx family of devices
    RTC_NAND_SDK_BLOCK_REWRITE,
    //! \brief For Freescale Use Only
    //! \internal
    //! Incremented by the RTC Device Driver upon initialization
    //! of the driver and set to 0 by the RTC Device Driver upon
    //! shutdown of the driver.  Values greater than 0 before initialization
    //! of the driver indicate a prior abnormal termination (i.e. a shutdown
    //! of the device without a call to the RTC Device Driver’s shutdown.)
    RTC_ABNORM_TERM,
    //! \brief For Freescale Use Only
    //! \internal
    //! Default is always set, to skip checkdisk. This bit is
    //! cleared on a write to media so that player on startup runs checkdisk.
    RTC_SKIP_CHECKDISK,
    //! \brief For Freescale Use Only
    //! \internal
    //! This bit by default is initialized to set (1) state,
    //! to indicate a clean database. Database is only maintained
    //! in MTP mode. For Mass Storage and Player modes, any write
    //! operation performed on media will result in clearing (0) of
    //! this bit to indicate that media contents are no longer in sync
    //! with MTP database. When device boots up in MTP mode it will
    //! check the status of this bit and if it is clear(0) it rebuilds the database.
    RTC_MTP_DB_CLEAN_FLAG,
    //! \brief For Freescale Use Only
    //! \internal
    //! Once database is rebuilt or changed this bit is set so
    //! that player updates all playlist and other information and clears it.
    RTC_DATABASE_CHANGED,
    //! \brief For Freescale Use Only
    //! \internal
    //! When set, this bit indicates that the NAND system drive read disturbance recovery
    //! mechanism has been activated and that a drive is currently being re-written.
    RTC_FIRMWARE_RECOVERY_IN_PROGRESS,
    //! \brief For Freescale Use Only
    //! \internal
    //! When set, this bit tells the NAND system drive read disturbance
    //! recovery code to read from the backup copy of the hostlink resource
    //! file. Once the original has been restored, this bit will be cleared.
    RTC_FIRMWARE_USE_BACKUP,
    //! \brief For Freescale Use Only
    //! \internal
    //! When set, forces the hostlink profile to enumerate in MSC recovery mode.
    RTC_FORCE_UPDATER,
    //! \brief For Freescale Use Only
    //! \internal
    //! Causes the bootmanager to load the update application when cleared.
    RTC_SKIP_FIRMWARE_UPDATE,
    //! \brief Used to flag when the LCD has been initialised.
    RTC_LCD_IS_INITIALIZED,
    //! \brief This bit is used to track whether the NAND mapper has been properly flushed.
    RTC_NAND_LOAD_ZONE_MAP_FROM_MEDIA
} Predefined_PersistentBits;

#define RTC_SOURCE_CHOICE_24MHZ  0
#define RTC_SOURCE_CHOICE_32KHZ  1

#define RTC_XTAL32_32000         0
#define RTC_XTAL32_32768         1

#define RTC_LOWER_BIAS_NONE      0
#define RTC_LOWER_BIAS_25        1,
#define RTC_LOWER_BIAS_50        3

#ifdef __cplusplus
}
#endif


#endif    // __RTC_PERSISTENT
