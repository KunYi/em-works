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
//  File:  bsp_cfg.h
//
#ifndef __BSP_CFG_H
#define __BSP_CFG_H

//------------------------------------------------------------------------------
//
//  Following include files contains configuration constants for different
//  system subsystems.
//
//#include "bsp_dma_cfg.h"
//#include "bsp_lcd_cfg.h"

//------------------------------------------------------------------------------
//
//  Define:  BSP_DEVICE_PREFIX
//
//  This define is used as device name prefix when KITL creates device name.
//
#define BSP_DEVICE_PREFIX       "H4-"

//------------------------------------------------------------------------------
//
//  Define:  BSP_UART_xxx
//
//  This constats are used to initialize serial debugger output UART.
//  On H4 board serial debugger uses 115200 Bd, 8 bits, 1 stop bit, no parity.
//
#define BSP_UART_LCR            0x03
#define BSP_UART_DSIUDLL        0x1A
#define BSP_UART_DSIUDLH        0

//------------------------------------------------------------------------------

#endif
