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
/// @brief  Public prototypes and types used for the PMIC Low-level access API.
///
/// This file contains the interface for low-level access to the WM8350 -
/// register access and interrupts.
///
/// @version $Id: pmic_lla.h 682 2007-06-30 16:43:46Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//------------------------------------------------------------------------------

#ifndef __PMIC_LLA_H__
#define __PMIC_LLA_H__

#include "pmic_basic_types.h"
#include "WMPmic.h"
#include "WM8350.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define PMIC_MC13783_MAX_CLK_FREQ         4000000    // 20 MHz

#define MC13783_ON1_BUTTON_MASK  (1 << (PMIC_MC13783_INT_ONOFD1I - 32))
#define MC13783_ON2_BUTTON_MASK  (1 << (PMIC_MC13783_INT_ONOFD2I - 32))
#define MC13783_ON3_BUTTON_MASK  (1 << (PMIC_MC13783_INT_ONOFD3I - 32))
#define MC13783_PWR_BUTTON_MASK  (MC13783_ON1_BUTTON_MASK | MC13783_ON2_BUTTON_MASK | MC13783_ON3_BUTTON_MASK)

#define MC13783_TODAM_MASK       (1 << (PMIC_MC13783_INT_TODAI - 32))
#define MC13783_RTCRSTI_MASK     (1 << (PMIC_MC13783_INT_RTCRSTI - 32))

//------------------------------------------------------------------------------
// Types

// Interrupt IDs - see %WINCEROOT%\COMMON\OAK\CSP\Wolfson\inc\WM8350.h for precise IDs.
typedef WM8350_INTERRUPT_ID PMIC_INT_ID;

//------------------------------------------------------------------------------
// Functions

// Register access
PMIC_STATUS PmicRegisterRead(unsigned char index, UINT32* reg);
PMIC_STATUS PmicRegisterWrite(unsigned char index, UINT32 reg, UINT32 mask);
PMIC_STATUS PmicDumpAllRegs();
WM_CHIPREV  PmicGetSiliconRev();
PMIC_STATUS WMStatusToPmicStatus( WM_STATUS status );

// Use these to block access to the control interface.
// NB: this will prevent any control of the PMIC or anything else on the same
// control bus, so should be used sparingly and with caution.  It is only
// needed if the bus state needs to be rigorously controlled for a short while.
//
// In particular, it is not needed during normal operation - the PMIC code
// handles all locking and unlocking internally.
PMIC_STATUS PmicBlockControlIF();
PMIC_STATUS PmicUnblockControlIF();

// Interrupt handling
PMIC_STATUS PmicInterruptRegister(PMIC_INT_ID int_id, LPTSTR event_name);
PMIC_STATUS PmicInterruptDeregister(PMIC_INT_ID int_id);
PMIC_STATUS PmicInterruptHandlingComplete(PMIC_INT_ID int_id);
PMIC_STATUS PmicInterruptDisable(PMIC_INT_ID int_id);
PMIC_STATUS PmicInterruptEnable(PMIC_INT_ID int_id);

#ifdef __cplusplus
}
#endif

#endif // __PMIC_LLA_H__
