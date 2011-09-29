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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File: bsp_cfg.h
//
// This file contains system constant specific for MX25 3DS board.
//
//------------------------------------------------------------------------------
#ifndef __BSP_CFG_H
#define __BSP_CFG_H

#ifdef	EM9170
#undef	BSP_CPLD_CSPI
#else	// ->iMX257PDK
#define BSP_CPLD_CSPI   1
#endif	//EM9170

//------------------------------------------------------------------------------
//
// Define: BSP_DEVICE_PREFIX
//
// Prefix used to generate device name for bootload/KITL
//
#define BSP_DEVICE_PREFIX       "MX25"                  // Device name prefix

//------------------------------------------------------------------------------
// CPU Configuration Settings
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Clock Configuration Settings
//------------------------------------------------------------------------------
#define RESCHED_PERIOD          1                       // Reschedule ms
#define BSP_EPIT_PRESCALAR      (4-1)                   // EPIT prescalar
#define BSP_EPIT_CLKSRC         EPIT_CR_CLKSRC_IPGCLK   // EPIT clock source


//------------------------------------------------------------------------------
// SDMA Configuration
//------------------------------------------------------------------------------
#define BSP_SDMA_MC0PTR                 IMAGE_WINCE_DDKSDMA_RAM_PA_START

#define BSP_SDMA_CHNPRI_AUDIO           (SDMA_CHNPRI_CHNPRI_HIGHEST)
#define BSP_SDMA_CHNPRI_SERIAL          (SDMA_CHNPRI_CHNPRI_HIGHEST-4)

//------------------------------------------------------------------------------
#define SYSINTR_USBOTG                  (SYSINTR_FIRMWARE+2)



//------------------------------------------------------------------------------
// LCD Panel Configuration to support IOCTL_HAL_QUERY_DISPLAYSETTINGS
//------------------------------------------------------------------------------
#define BSP_PREF_DISPLAY_WIDTH          640
#define BSP_PREF_DISPLAY_HEIGHT         480
#define BSP_PREF_DISPLAY_BPP            16

//------------------------------------------------------------------------------
// Video Memory memory attributes
// The video memory region may be configured as cacheable, write-through (WT)
// or as non-cacheable, bufferable (NCB). MX25 has no ARM errata (#399234) which
// affects ARM1136 core (r0p2,r1p0,r1p1,r1p2), so use WT mode to provides 
// performance benefit.
#define BSP_VID_MEM_CACHE_WRITETHROUGH  TRUE

//------------------------------------------------------------------------------
//
// Define: BSP_BASE_REG_PA_SERIALKITL
//
// Specifies physical address of serial port used for serial KITL transport.
//
#define BSP_BASE_REG_PA_SERIALKITL      CSP_BASE_REG_PA_UART1
#define BSP_UART_KITL_SERIAL_BAUD       115200


//------------------------------------------------------------------------------
// Debug OUTPUT
//
// DEBUG_PORT specifies which UART we use for debug serial port. It must be one
// of the following :
//     DBG_UART1
//     DBG_SC16C652_PORTA
//     DBG_SC16C652_PORTB
//------------------------------------------------------------------------------
#define DBG_UART1               1
#define DBG_SC16C652_PORTA      2
#define DBG_SC16C652_PORTB      3

// To disable debug port messages altogether, define DEBUG_PORT to 0
#define DEBUG_PORT              DBG_UART1

// UART Port
// Defines for UART Rx and Tx SDMA buffer size per SDMA buffer descriptor.  
//------------------------------------------------------------------------------
#define SERIAL_SDMA_RX_BUFFER_SIZE 0x200
#define SERIAL_SDMA_TX_BUFFER_SIZE 0x400

// SSI Port 2
// Defines for audio SDMA buffer starting location and total size  
//------------------------------------------------------------------------------
#define BSP_AUDIO_DMA_BUF_ADDR      IMAGE_WINCE_AUDIO_IRAM_PA_START
#define BSP_AUDIO_DMA_BUF_SIZE      IMAGE_WINCE_AUDIO_IRAM_SIZE

//------------------------------------------------------------------------------
// Defines the Maximum Baudrate and reference frequency for UART
//------------------------------------------------------------------------------
#define UART_MAX_BAUDRATE				4000000
#define UART_REF_FREQ						(16 * UART_MAX_BAUDRATE)

//------------------------------------------------------------------------------
// FEC Board Configuration
//------------------------------------------------------------------------------
#ifdef	EM9170
//------------------------------------------------------------------------------
// CS&ZHL MAY-31-2011: use GPIO4_2 as PHY_RESET#, 
//                                      use GPIO4_3 as PHY_PWRDWN
//------------------------------------------------------------------------------
#define FEC_ENABLE_GPIO_PORT			DDK_GPIO_PORT4
#define FEC_ENABLE_GPIO_PIN			3
#define FEC_RESET_GPIO_PORT			DDK_GPIO_PORT4
#define FEC_RESET_GPIO_PIN				2
#else	// -> iMX257PDK
#define FEC_ENABLE_GPIO_PORT			DDK_GPIO_PORT2
#define FEC_ENABLE_GPIO_PIN			3
#define FEC_RESET_GPIO_PORT			DDK_GPIO_PORT4
#define FEC_RESET_GPIO_PIN				8
#endif	//EM9170

//------------------------------------------------------------------------------
// ESAI Board Configuration
//------------------------------------------------------------------------------
#define ESAI_GPIO_GPIO_PORT         DDK_GPIO_PORT3
#define ESAI_GPIO_GPIO_PIN          15
#define ESAI_RST_GPIO_PORT          DDK_GPIO_PORT3
#define ESAI_RST_GPIO_PIN           16

//------------------------------------------------------------------------------
// Camera Board Configuration
//------------------------------------------------------------------------------
#define CAMERA_ENABLE_GPIO_PORT        DDK_GPIO_PORT2
#define CAMERA_ENABLE_GPIO_PIN         5
#define CAMERA_RESET_GPIO_PORT         DDK_GPIO_PORT2
#define CAMERA_RESET_GPIO_PIN          6

#endif
