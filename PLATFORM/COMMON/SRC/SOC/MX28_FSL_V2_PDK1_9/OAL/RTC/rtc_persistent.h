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
    //! Setting this to one prevents the RTC seconds from being modified,
    //! or this bit from being modified.   Supported on the 37xx only.
    //! \brief protects the RTC from being changed
    RTC_LCK_SECS,    
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
    //! \brief Sets the exact frequency of the 32K crystal.
    //!
    //! If the value set for this persistent register is RTC_XTAL32_32768, a 32768Hz crystal is being used.
    //! If the value set for this persistent register is RTC_XTAL32_32000, a 32KHz crystal is being used.
    //! All other values return an error.
    RTC_XTAL32_FREQ,
    //! \brief Powers the chip upon arrival of an alarm event.
    //!
    //! Set this bit to 1 to power up the chip upon the arrival of an alarm event.
    //! ALARM_EN must be set to 1 to enable the detection of an alarm event.
    //!
    //! \note This bit is reset by writing a 0 directly to the shadow register
    //! which causes the copy controller to move it across to the analog domain.
    RTC_ALARM_WAKE,
    //! Controls the resolution of the millisecond timer of the RTC. Acceptable values are 1,2,4,8,16.
    //! \brief Controls the resolution of the millisecond timer of the RTC
    //! \note only supported on the 37xx devices.
    RTC_MSEC_RES,
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
    //! \brief Set to one to enable a low voltage on LRADC0 to powerup the
    //! chip. This is to support powerup via open drain pulldowns
    //! connected to LRADC0. Ths requires external circuitry and this
    //! bit should not be set unless the correct circuitry is present.
    //! Freescale will provide information on the external circuitry, if this
    //! functionality is required.
    RTC_ENABLE_LRADC_PWRUP,
    //! 
    RTC_RELEASE_GND,
    //! This bit is set by the analog hardware. On powerup, it indicate
    //! to software that that the chip had previously been powered
    //! down due to overheating. This bit must be cleared by software.
    //! This bit is analogous to the HW_POWER_STS_THERMAL_RESET bit only in persistent storage.
    RTC_THERMAL_RESET,
    //! This bit is set by the analog hardware. On powerup, it indicate
    //! to software that that the chip had previously been powered
    //! down due to a reset event on the reset pin by an external
    //! source. This bit must be cleared by software. This bit is
    //! analogous to the ?? bit only in persistent storage.
    RTC_EXTERNAL_RESET,
    //! Reserved for special uses.
    RTC_SPARE_ANALOG,
    //! This field can be used to allow dcdc startup with lower Liion
   //! battery voltages. With the default value of 0x0, the dcdc
   //! converter will not startup below 2.83V, and increasing this value
   //! will lower the Liion startup voltage. It is not recommended to set
   //! a value higher than 0x6. The value of this field effects the
   //! minimum startup voltage as follows: 0x0=2.83V, 0x1=2.78V,
   //! 0x2=2.73V, 0x3=2.68V, 0x4=2.62V, 0x5=2.57V, 0x6=2.52V,
   //! 0x7-0xF=2.48V.
   RTC_ADJ_POSLIMITBUCK
} Predefined_PersistentBits;

#define RTC_SOURCE_CHOICE_24MHZ  0
#define RTC_SOURCE_CHOICE_32KHZ  1

#define RTC_XTAL32_32000         0
#define RTC_XTAL32_32768         1

#define RTC_LOWER_BIAS_NONE      0
#define RTC_LOWER_BIAS_25        1
#define RTC_LOWER_BIAS_50        3

#ifdef __cplusplus
}
#endif


#endif    // __RTC_PERSISTENT
