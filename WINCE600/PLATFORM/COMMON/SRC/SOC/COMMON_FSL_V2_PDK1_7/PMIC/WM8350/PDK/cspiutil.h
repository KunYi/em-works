//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
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
/// @file   cspiutil.h
/// @brief  Defines the interface to the CSPI driver for the WM8350.
///
/// This file contains the interface for communicating with the WM8350 over
/// the 3-wire and 4-wire (SPI) interfaces.  This code is specifically for
/// the PMIC interface and should be run over a dedicated SPI bus.
///
/// @version $Id: cspiutil.h 298 2007-04-13 09:43:55Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __CSPIUTIL_H__
#define __CSPIUTIL_H__

//
// Include files.
//
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define CSPI_WRITE          1
#define CSPI_READ           0

//
// WM8350 CSPI word format is (highest  bit gets transmitted first):
//
//31  30      23       15       7       0
// +-+-------+--------+--------+--------+
// |R|       |        |                 |
// |W|       |  addr  |      data       |
// +-+-------+--------+--------+--------+
//
typedef struct {
        unsigned int data:16;
        unsigned int address:8;
        unsigned int null:7;
        unsigned int rw:1;
} CSPI_PACKET32_FIELDS;
#define CSPI_DATA_MASK  0xFFFF

//
// A union so we can handle it as an int as well.
//
typedef union
{
    CSPI_PACKET32_FIELDS reg;
    unsigned int         data;
} CSPI_PACKET32;

//------------------------------------------------------------------------------
// Functions

BOOL PmicCspiInitialize(int index, UINT32 dwFrequency);
VOID PmicCspiRelease(int index);
BOOL PmicCspiAddWritePacket(UINT32 addr, UINT32 data);
BOOL PmicCspiAddReadPacket(UINT32 addr);
BOOL PmicCspiDataExchange();
BOOL PmicCspiReceiveData(UINT32* data);
BOOL PmicCspiDiscardData();
VOID PmicCspiPowerDown(void);
VOID PmicCspiSync(void);
VOID PmicCspiLock(void);
VOID PmicCspiUnlock(void);


#ifdef __cplusplus
}
#endif

#endif // __CSPIUTIL_H__
//////////////////////////////// END OF FILE ///////////////////////////////////
