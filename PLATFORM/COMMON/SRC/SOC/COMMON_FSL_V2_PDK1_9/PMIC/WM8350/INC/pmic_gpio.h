//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
///
/// Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
///
/// This software as well as any related documentation may only be used or
/// copied in accordance with the terms of the Wolfson Microelectronic plc
/// agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
///
/// The information in this file is furnished for informational use only,
/// is subject to change without notice, and should not be construed as a
/// commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
/// assumes no responsibility or liability for any errors or inaccuracies that
/// may appear in the software or any related documention.
///
/// Except as permitted by the agreement(s), no part of the software or any
/// related documention may be reproduced, stored in a retrieval system, or
/// transmitted in any form or by any means without the express written
/// consent of Wolfson Microelectronics plc.
///
/// @file   pmic_gpio.h
/// @brief  Public prototypes and types used for the PMIC GPIO API.
///
/// This file contains the interface for controlling the GPIOs on the WM8350.
///
/// @version $Id: pmic_gpio.h 650 2007-06-15 22:31:22Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//------------------------------------------------------------------------------

#ifndef __PMIC_GPIO_H__
#define __PMIC_GPIO_H__

#include "pmic_basic_types.h"
#include <WMGPIO.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Functions

// GPIOs
PMIC_STATUS PmicGpioSetLevels(WM_GPIOS GPIOs, WM_GPIOS levelMask);
PMIC_STATUS PmicGpioSetDir(WM_GPIOS GPIOs, WM_GPIO_DIR dir);
PMIC_STATUS PmicGpioSetPolarity(WM_GPIOS GPIOs, WM_GPIO_POLARITY polarity);
PMIC_STATUS PmicGpioSetAutoInvert(WM_GPIOS GPIOs, BOOL autoInvert);
PMIC_STATUS PmicGpioSetPinType(WM_GPIOS GPIOs, WM_GPIO_PIN_TYPE pinType);
PMIC_STATUS PmicGpioSetPullUp(WM_GPIOS GPIOs, BOOL pullUp);
PMIC_STATUS PmicGpioSetPullDown(WM_GPIOS GPIOs, BOOL pullDown);
PMIC_STATUS PmicGpioSetDebounce(WM_GPIOS GPIOs, BOOL debounce);
PMIC_STATUS PmicGpioSetFunction(WM_GPIO GPIO, WM_GPIO_ALTFN function);

PMIC_STATUS PmicGpioClearInt(WM_GPIOS GPIOs);

PMIC_STATUS PmicGpioGetLevels(WM_GPIOS *pHigh);
PMIC_STATUS PmicGpioGetInputs(WM_GPIOS *pInputs);
PMIC_STATUS PmicGpioGetOutputs(WM_GPIOS *pOutputs);
PMIC_STATUS PmicGpioGetPolarity(WM_GPIOS *pActiveHigh);
PMIC_STATUS PmicGpioGetAutoInvert(WM_GPIOS *pAutoInvert);
PMIC_STATUS PmicGpioGetPinType(WM_GPIOS *pOpenDrain);
PMIC_STATUS PmicGpioGetPullUp(WM_GPIOS *pPullUps);
PMIC_STATUS PmicGpioGetPullDown(WM_GPIOS *pPullDowns);
PMIC_STATUS PmicGpioGetDebounce(WM_GPIOS *pDebounce);
PMIC_STATUS PmicGpioGetFunction(WM_GPIO GPIO, WM_GPIO_ALTFN *pFunction);


#ifdef __cplusplus
}
#endif

#endif // __PMIC_REGULATOR_H__
