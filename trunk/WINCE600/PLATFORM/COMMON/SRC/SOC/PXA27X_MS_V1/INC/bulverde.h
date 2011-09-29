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
//  Header: bulverde.h
//
//  This header file defines the Intel Bulverde processor.
//
//  The Bulverde is a System on Chip (SoC) part consisting of an ARM920T core. 
//  This header file is comprised of component header files that define the 
//  register layout of each component.
//  
//------------------------------------------------------------------------------
#ifndef _BULVERDE_H_
#define _BULVERDE_H_

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

// Base Definitions
//
#include "bulverde_base_regs.h"

// SoC Components
//
#include "bulverde_ac97.h"
#include "bulverde_clkmgr.h"
#include "bulverde_dma.h"
#include "bulverde_gpio.h"
#include "bulverde_iicbus.h"
#include "bulverde_iisbus.h"
#include "bulverde_intr.h"
#include "bulverde_keypad.h"
#include "bulverde_memctrl.h"
#include "bulverde_mmc.h"
#include "bulverde_ost.h"
#include "bulverde_pwrrst.h"
#include "bulverde_rtc.h"
#include "bulverde_ssp.h"
#include "bulverde_uart.h"
#include "bulverde_usbd.h"
#include "bulverde_camera.h"

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#include "bulverde_usbohci.h"

#endif 
