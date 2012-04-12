//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2009-2010 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: xldr.h
//
//  This header file defines the MX28 processor.
//
//  The MX28 is a System on Chip (SoC) part consisting of an ARM9 core.
//  This header file is comprised of component header files that define the
//  register layout of each component.
//
//-----------------------------------------------------------------------------
#ifndef __XLDR_H
#define __XLDR_H

#include "bsp.h"

#define pv_HWregDIGCTL     CSP_BASE_REG_PA_DIGCTL
#define pv_HWregCLKCTRL    CSP_BASE_REG_PA_CLKCTRL
#define pv_HWregPOWER      CSP_BASE_REG_PA_POWER
#define pv_HWregPINCTRL    CSP_BASE_REG_PA_PINCTRL
#define pv_HWregDRAM       CSP_BASE_REG_PA_DRAM
#define pv_HWregEMI        CSP_BASE_REG_PA_DRAM
#define pv_HWregUARTDbg    CSP_BASE_REG_PA_UARTDBG
#define pv_HWregUSBPhy0    CSP_BASE_REG_PA_USBPHY0
#define pv_HWregUSBPhy1    CSP_BASE_REG_PA_USBPHY1
#define pv_HWregLRADC      CSP_BASE_REG_PA_LRADC
#define pv_HWregOTP        CSP_BASE_REG_PA_OCOTP
#define pv_HWregRTC        CSP_BASE_REG_PA_RTC

#define BATTERY_VOLTAGE_CH     7
#define LRADC_DELAY_TRIGGER3   3
#define BATT_VOLTAGE_8_MV      8
#define BATTERY_LOW            2400
#define BATTERY_HIGH           4400
#define BATTERY_BAD            3500
#define BATTERY_BOOT           3600

// todo The following 5 defines should be configured by the application, not the driver.
#define EMI_PIN_DRIVE_ADDRESS          PIN_DRIVE_20mA
#define EMI_PIN_DRIVE_CONTROL          PIN_DRIVE_20mA
#define EMI_PIN_DRIVE_CLOCK            PIN_DRIVE_20mA
#define EMI_PIN_DRIVE_DATA_SLICE_0     PIN_DRIVE_20mA
#define EMI_PIN_DRIVE_DATA_SLICE_1     PIN_DRIVE_20mA

#define SSP0_PIN_DRIVE_12mA            0x2

typedef enum
{
    EMI_DEV_MOBILE_DDR,
    EMI_DEV_DDR2 ,
    EMI_DEV_LVDDR2
} EMI_MemType_t;

typedef enum
{
    PIN_VOLTAGE_1pt8V = 0,
    PIN_VOLTAGE_3pt3V = 1,
} TPinVoltage;

typedef enum
{
    PIN_DRIVE_5mA   = 0,
    PIN_DRIVE_10mA  = 1,
    PIN_DRIVE_20mA  = 2
} TPinDrive;

VOID InitSdram(EMI_MemType_t type);
VOID ConfigureEmiPins(TPinDrive pin_drive_addr,
    TPinDrive pin_drive_ctrl,TPinDrive pin_drive_clk,TPinDrive pin_drive_data_slice_0,TPinDrive pin_drive_data_slice_1);
VOID DDR2EmiController_EDE1116_200MHz(VOID);
VOID XLDRWriteDebugByte(UINT8 ch);
VOID InitDebugSerial();
VOID PrintHex(UINT32 value);
VOID PrintBatteryVoltage(UINT32 value);
VOID InitPower();
VOID PowerSetCharger(UINT32 current);
VOID PowerStopCharger();
VOID XLDRStall(UINT32 microSec);
VOID CPUClock2XTAL();
VOID CPUClock2PLL();
BOOL EnableBatteryMeasure();
BOOL IsBatteryGood();
BOOL IsUSBPlugin();
BOOL Is5VPresent();
VOID IsWall5V();
VOID BootFromBattery();
VOID BootFrom4P2();
VOID ChargeBattery2Boot();

#endif //__XLDR_H
