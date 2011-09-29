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
//  Header: bulverde_gpio.h
//
//  Defines the general-purpose I/O (GPIO) register layout and associated types 
//  and constants.
//
#ifndef __BULVERDE_GPIO_H
#define __BULVERDE_GPIO_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  BULVERDE_GPIO_REG
//
//  Defines the GPIO control register layout.
//

#include <xllp_gpio.h>

typedef XLLP_GPIO_T BULVERDE_GPIO_REG;
typedef XLLP_GPIO_T *PBULVERDE_GPIO_REG;

#define GPIO_GEDR0_VLD_MSK (~0x00000000u)
#define GPIO_GEDR1_VLD_MSK (~0x00000000u)
#define GPIO_GEDR2_VLD_MSK (~0x00000000u)
#define GPIO_GEDR3_VLD_MSK (~0xFE000000u)

#define GPIO_GFER3_VLD_MSK (~0xFE000000u)
#define GPIO_GRER3_VLD_MSK (~0xFE000000u)

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
