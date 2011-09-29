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
//  File:  bsp_menelaus.h
//
#ifndef __BSP_MENELAUS_H
#define __BSP_MENELAUS_H


//------------------------------------------------------------------------------

#define I2C_MENELAUS_ADDRESS     0x72

//------------------------------------------------------------------------------
// Register Offset
//

#define MENELAUS_REV_OFFSET              0x01
#define MENELAUS_VCORECTRL1_OFFSET		 0x02
#define MENELAUS_VCORECTRL2_OFFSET		 0x03
#define MENELAUS_VCORECTRL3_OFFSET		 0x04
#define MENELAUS_VCORECTRL4_OFFSET		 0x05
#define MENELAUS_VCORECTRL5_OFFSET		 0x06
#define MENELAUS_DCDCCTRL1_OFFSET		 0x07
#define MENELAUS_DCDCCTRL2_OFFSET		 0x08
#define MENELAUS_DCDCCTRL3_OFFSET		 0x09
#define MENELAUS_LD0CTRL1_OFFSET		 0x0A
#define MENELAUS_LD0CTRL2_OFFSET		 0x0B
#define MENELAUS_LD0CTRL3_OFFSET		 0x0C
#define MENELAUS_LD0CTRL4_OFFSET		 0x0D
#define MENELAUS_LD0CTRL5_OFFSET		 0x0E
#define MENELAUS_LD0CTRL6_OFFSET		 0x0F
#define MENELAUS_LD0CTRL7_OFFSET		 0x10
#define MENELAUS_LD0CTRL8_OFFSET		 0x11
#define MENELAUS_SLEEPCTRL1_OFFSET		 0x12
#define MENELAUS_SLEEPCTRL2_OFFSET		 0x13
#define MENELAUS_DEVICEOFF_OFFSET		 0x14
#define MENELAUS_OSCCTRL_OFFSET		     0x15
#define MENELAUS_DETECTCTRL_OFFSET		 0x16
#define MENELAUS_INTMASK1_OFFSET		 0x17
#define MENELAUS_INTMASK2_OFFSET		 0x18
#define MENELAUS_INTSTATUS1_OFFSET		 0x19
#define MENELAUS_INTSTATUS2_OFFSET		 0x1A
#define MENELAUS_INTACK1_OFFSET		     0x1B
#define MENELAUS_INTACK2_OFFSET		     0x1C
#define MENELAUS_GPIOCTRL_OFFSET		 0x1D
#define MENELAUS_GPIOIN_OFFSET		     0x1E
#define MENELAUS_GPIOOUT_OFFSET		     0x1F
#define MENELAUS_BBSMS_OFFSET		     0x20
#define MENELAUS_RTCCTRL_OFFSET		     0x21
#define MENELAUS_RTCUPDATE_OFFSET		 0x22
#define MENELAUS_RTCSEC_OFFSET		     0x23
#define MENELAUS_RTCMIN_OFFSET		     0x24
#define MENELAUS_RTCHR_OFFSET		     0x25
#define MENELAUS_RTCDAY_OFFSET		     0x26
#define MENELAUS_RTCMON_OFFSET		     0x27
#define MENELAUS_RTCYR_OFFSET		     0x28
#define MENELAUS_RTCWKDAY_OFFSET		 0x29
#define MENELAUS_RTCALSEC_OFFSET		 0x2A
#define MENELAUS_RTCALMIN_OFFSET		 0x2B
#define MENELAUS_RTCALHR_OFFSET		     0x2C
#define MENELAUS_RTCALDAY_OFFSET		 0x2D
#define MENELAUS_RTCALMON_OFFSET		 0x2E
#define MENELAUS_RTCALYR_OFFSET		     0x2F
#define MENELAUS_RTCCOMPMSB_OFFSET		 0x30
#define MENELAUS_RTCCOMPLSB_OFFSET		 0x31
#define MENELAUS_S1PULLEN_OFFSET		 0x32
#define MENELAUS_S1PULLDIR_OFFSET		 0x33
#define MENELAUS_S2PULLEN_OFFSET		 0x34
#define MENELAUS_S2PULLDIR_OFFSET		 0x35
#define MENELAUS_MCTCTRL1_OFFSET		 0x36
#define MENELAUS_MCTCTRL2_OFFSET		 0x37
#define MENELAUS_MCTCTRL3_OFFSET		 0x38
#define MENELAUS_MCTPINST_OFFSET		 0x39
#define MENELAUS_DEBOUNCE_OFFSET		 0x3A


#define DCDC3_VOLT15					(0 << 3)
#define DCDC3_VOLT18					(1 << 3)
#define DCDC3_VOLT20					(2 << 3)
#define DCDC3_VOLT22					(3 << 3)
#define DCDC3_VOLT24					(4 << 3)
#define DCDC3_VOLT28					(5 << 3)
#define DCDC3_VOLT30					(5 << 3)
#define DCDC3_VOLT32					(7 << 3)
#define DCDC3_VOLT						(7 << 3)
#define DCDC3_MODE_OFF					(0 << 0)
#define DCDC3_MODE_SLEEP				(1 << 0)
#define DCDC3_MODE_ONPWM				(3 << 0)
#define DCDC3_MODE						(7 << 0)
#define VMMC_VOLT18						(0 << 6)
#define VMMC_VOLT28						(1 << 6)
#define VMMC_VOLT30						(2 << 6)
#define VMMC_VOLT32						(3 << 6)
#define VMCC_VOLT						(3 << 6)
#define VMMC_MODE_OFF					(0 << 0)
#define VMMC_MODE_SLEEP					(1 << 0)
#define VMMC_MODE_ON					(1 << 1)
#define VMMC_MODE						(3 << 0)
#define S2D1_MSK						(1 << 3)		
#define S1D1_MSK						(1 << 2)		
#define S2CD_MSK						(1 << 1)		
#define S1CD_MSK						(1 << 0)		
#define S2D1							(1 << 3)		
#define S1D1							(1 << 2)		
#define S2CD							(1 << 1)		
#define S1CD							(1 << 0)		
#define S2D1_ACK						(1 << 3)		
#define S1D1_ACK						(1 << 2)		
#define S2CD_ACK						(1 << 1)		
#define S1CD_ACK						(1 << 0)		
#define SLOTSELEN						(1 << 5)
#define S1_APPCLKF_DLY					(7 << 5)
#define S1_CMD_EN						(1 << 4)
#define S1_DAT3_EN						(1 << 3)
#define S1_DAT2_EN						(1 << 2)
#define S1_DAT1_EN						(1 << 1)
#define S1_DAT0_EN						(1 << 0)
#define S1_CMD_UP						(1 << 4)
#define S1_DAT3_UP						(1 << 3)
#define S1_DAT2_UP						(1 << 2)
#define S1_DAT1_UP						(1 << 1)
#define S1_DAT0_UP						(1 << 0)
#define S2_APPCLKF_DLY					(7 << 5)
#define S2_CMD_EN						(1 << 4)
#define S2_DAT3_EN						(1 << 3)
#define S2_DAT2_EN						(1 << 2)
#define S2_DAT1_EN						(1 << 1)
#define S2_DAT0_EN						(1 << 0)
#define S2_CMD_UP						(1 << 4)
#define S2_DAT3_UP						(1 << 3)
#define S2_DAT2_UP						(1 << 2)
#define S2_DAT1_UP						(1 << 1)
#define S2_DAT0_UP						(1 << 0)
#define APBUFDRV						(1 << 6)
#define S2BUFDRV						(1 << 5)
#define S1BUFDRV						(1 << 4)
#define S2_CMD_OD						(1 << 3)
#define S1_CMD_OD						(1 << 2)
#define S2CD_SWNO						(1 << 1)
#define S1CD_SWNO						(1 << 0)
#define S2CD_DBEN						(1 << 7)
#define S1CD_DBEN						(1 << 6)
#define S2CD_BUFEN						(1 << 5)
#define S1CD_BUFEN						(1 << 4)
#define S2D1_BUFEN						(1 << 3)
#define S1D1_BUFEN						(1 << 2)
#define VS2_EXT							(1 << 1)
#define VS2_VAUX						(1 << 0)
#define VS2_DCDC3						(0 << 0)
#define S2_AUTO_EN						(1 << 3)
#define S1_AUTO_EN						(1 << 2)
#define SLOT2_EN						(1 << 1)
#define SLOT1_EN						(1 << 0)
#define S2_DAT1_ST						(1 << 3)
#define S1_DAT1_ST						(1 << 2)
#define S2_CD_ST						(1 << 1)
#define S1_CD_ST						(1 << 0)
#define REF05_EN						(1 << 1)
#define VADAC_EN						(1 << 0)
//------------------------------------------------------------------------------

#endif
