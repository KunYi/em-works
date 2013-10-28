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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  Header: s3c6410.h
//
//  This header file defines the S3C6410 processor.
//
//  The s3C6410 is a System on Chip (SoC) part consisting of an ARM920T core.
//  This header file is comprised of component header files that define the
//  register layout of each component.
//
//------------------------------------------------------------------------------
#ifndef __S3C6410_H
#define __S3C6410_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

// Base Definitions
#include <s3c6410_base_regs.h>

// SoC Components
#include "s3c6410_ac97.h"
#include "s3c6410_adc.h"
#include "s3c6410_camif.h"
#include "s3c6410_cfcon.h"
#include "s3c6410_display.h"
#include "s3c6410_dma.h"
#include "s3c6410_dramcon.h"
#include "s3c6410_gpio.h"
#include "s3c6410_hsmmc.h"
#include "s3c6410_iic.h"
#include "s3c6410_iis.h"
#include "s3c6410_intr.h"
#include "s3c6410_keypad.h"
#include "s3c6410_msmif.h"
#include "s3c6410_nand.h"
#include "s3c6410_post.h"
#include "s3c6410_rotator.h"
#include "s3c6410_pwm.h"
#include "s3c6410_rtc.h"
#include "s3c6410_spi.h"
#include "s3c6410_sromcon.h"
#include "s3c6410_syscon.h"
#include "s3c6410_tvenc.h"
#include "s3c6410_tvsc.h"
#include "s3c6410_uart.h"
#include "s3c6410_usbotg.h"
#include "s3c6410_wdog.h"
#include "s3c6410_vintr.h"
#include "s3c6410_system.h"

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
