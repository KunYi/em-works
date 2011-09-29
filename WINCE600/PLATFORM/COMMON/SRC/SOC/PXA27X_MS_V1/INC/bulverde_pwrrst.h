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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  Header: bulverde_pwrrst.h
//
//  Defines the power and reset register layout and definitions.
//
#ifndef __BULVERDE_PWRRST_H
#define __BULVERDE_PWRRST_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: BULVERDE_PWRRST_REG
//
//  Power Management and reset registers.
//

typedef struct
{
    UINT32    pmcr;         // Power manager control register.
    UINT32    pssr;         // Power manager sleep status register.
    UINT32    pspr;         // Power manager scratch pad register.
    UINT32    pwer;         // Power manager wake-up enable register.
    UINT32    prer;         // Power manager GPIO rising edge detect enable register.
    UINT32    pfer;         // Power manager GPIO falling edge detect enable register.
    UINT32    pedr;         // Power manager GPIO edge detect status register.
    UINT32    pcfr;         // Power manager general configuration register.
    UINT32    pgsr0;        // Power manager GPIO sleep state register for GPIO 31:0.
    UINT32    pgsr1;        // Power manager GPIO sleep state register for GPIO 63:32.
    UINT32    pgsr2;        // Power manager GPIO sleep state register for GPIO 95:64.
    UINT32    pgsr3;        // Power manager GPIO sleep state register for GPIO 120:96.
    UINT32    rcsr;         // **Reset controller status register**.
    UINT32    pslr;         // Power manager Sleep Mode Config.
    UINT32    pstr;         // Power manager Standby Mode Config.
    UINT32    psnr;         // Power manager Sense Mode Config.
    UINT32    pvcr;         // Power manager Voltage Change Control.
    UINT32    rsvd6[3];
    UINT32    pkwr;         // Power manager keyboard wake-up enable register
    UINT32    pksr;         // Power manager keyboard level-detect status register
    UINT32    rsvd7[10];
    UINT32    pcmd[32];     // Power manager I2C Command.
    UINT32    rsvd1[32];       
    UINT32    pibmr;        // Power manager I2C Bus Monitor.
    UINT32    rsvd2;
    UINT32    pidbr;        // Power manager I2C Data Buffer.
    UINT32    rsvd3;
    UINT32    picr;         // Power manager I2C Control.
    UINT32    rsvd4;
    UINT32    pisr;         // Power manager I2C Status.
    UINT32    rsvd5;
    UINT32    pisar;        // Power manager I2C Slave Adx.

} BULVERDE_PWRRST_REG, *PBULVERDE_PWRRST_REG;

//------------------------------------------------------------------------------
//  Type: BULVERDE_PWER_REGBITS
//
//  PWER register
//

typedef struct
{
    unsigned gpio0      :1;    // enable/disable wake up due to gpio0 edge detect (standby, sleep or deep-sleep mode)
    unsigned gpio1      :1;    // enable/disable wake up due to gpio1 edge detect (standby, sleep or deep-sleep mode)
    unsigned rsvd2      :1;     
    unsigned gpio3      :1;    // enable/disable wake up due to gpio3 edge detect (standby, sleep or deep-sleep mode)
    unsigned gpio4      :1;    // enable/disable wake up due to gpio4 edge detect (standby, sleep or deep-sleep mode)
    unsigned rsvd8_5    :4;
    unsigned gpio9      :1;    // enable/disable wake up due to gpio9 edge detect (standby, sleep or deep-sleep mode)
    unsigned gpio10     :1;    //                   .....
    unsigned gpio11     :1;    //                   .....
    unsigned gpio12     :1;    //                   .....
    unsigned gpio13     :1;    //                   .....
    unsigned gpio14     :1;    //
    unsigned gpio15     :1;    // enable/disable wake up due to gpio15 edge detect (standby, sleep or deep-sleep mode) 
    unsigned gpio_wemux2   :3; //enable/disable wake up for rising or falling edge on GPIO<36>, GPIO<38>, GPIO<40>  and/or GPIO<53> (standby or sleep mode)
    unsigned gpio_wemux3   :2; //enable/disable wake up for rising or falling edge on GPIO<31> and/or GPIO<113> (standby or sleep mode)
    unsigned rsvd22_21  :2;
    unsigned usim       :1;    // enable/disable wake up for a rising or falling edge from UDET (GPIO<116>) (standby or sleep mode)
    unsigned gpio35     :1;    // enable/disable wake up due to gpio35 edge detect (standby or sleep mode) 
    unsigned msl        :1;    // enable/disable wake up for a rising edge from msl (GPIO <83>) (standby or sleep mode)
    unsigned usbc       :1;    // enable/disable wake up due to usb client port (standby or sleep mode)
    unsigned usbh0      :1;    // enable/disable wake up due to usb host port 1 (standby or sleep mode)
    unsigned usbh1      :1;    // enable/disable wake up due to usb host port 2 (standby or sleep mode)
    unsigned rsvd29     :1; 
    unsigned pi         :1;    // enable/disable wake up for PI power domain due to timer wake-up event  (standby or sleep mode)
    unsigned rtc_alarm  :1;    // enable/disable wake up due to rtc alarm (standby, sleep or deep-sleep mode)
}   BULVERDE_PWER_REGBITS, *PBULVERDE_PWER_REGBITS;

//------------------------------------------------------------------------------
//  Type: BULVERDE_PEDR_REGBITS
//
//  PEDR register
//
typedef struct
{
    unsigned gpio0      :1;    // wake up due to edge on gpio0 detected (standby, sleep or deep-sleep mode)
    unsigned gpio1      :1;    // wake up due to edge on gpio1 detected (standby, sleep or deep-sleep mode)
    unsigned rsvd2      :1;     
    unsigned gpio3      :1;    // wake up due to edge on gpio3 detected (standby, sleep or deep-sleep mode)
    unsigned gpio4      :1;    // wake up due to edge on gpio4 detected(standby, sleep or deep-sleep mode)
    unsigned rsvd8_5    :4;
    unsigned gpio9      :1;    // wake up due to edge on gpio9 detected (standby, sleep or deep-sleep mode)
    unsigned gpio10     :1;    //                   .....
    unsigned gpio11     :1;    //                   .....
    unsigned gpio12     :1;    //                   .....
    unsigned gpio13     :1;    //                   .....
    unsigned gpio14     :1;    //
    unsigned gpio15     :1;    // wake up due to edge on gpio15 detected (standby, sleep or deep-sleep mode) 
    unsigned rsvd16      :1;
    unsigned gpio_edmux2 :1;   // wake up due to edge on gpion detected  where n=value programmed in PWER[WEMUX2] (standby or sleep mode)
    unsigned rsvd19_18   :2;
    unsigned gpio_edmux3 :1;   // wake up due to edge on gpion detected  where n=value programmed in PWER[WEMUX3] (standby or sleep mode)
    unsigned rsvd23_21   :3;
    unsigned gpio35     :1;    // wake up due to edge on gpio35 detected  (standby or sleep mode)
    unsigned msl        :1;    // wake up due to msl detected             (standby or sleep mode)
    unsigned usbc       :1;    // wake up due to usb client port detected (standby or sleep mode)
    unsigned usbh0      :1;    // wake up due to usb host port 1 detected (standby or sleep mode)
    unsigned usbh1      :1;    // wake up due to usb host port 2 detected (standby or sleep mode)
    unsigned rsvd29     :1; 
    unsigned pi         :1;    // wake up due to PI power domain source detected  (standby or sleep mode)
    unsigned rtc_alarm  :1;    // wake up due to rtc source detected (standby, sleep or deep-sleep mode)
}   BULVERDE_PEDR_REGBITS, *PBULVERDE_PEDR_REGBITS;

#define     PGSR0_REG_RSVD_BITS    0x0;
#define     PGSR1_REG_RSVD_BITS    0x0;
#define     PGSR2_REG_RSVD_BITS    0x0;
#define     PGSR3_REG_RSVD_BITS    0xff800000;
#define     PEDR_REG_RSVD_BITS     0x31ff01e4;
#define     PRER_REG_RSVD_BITS     0xffff01e4
#define     PFER_REG_RSVD_BITS     0xffff01e4

#if __cplusplus
    }
#endif

#endif 
