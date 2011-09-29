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
/// @file   PmicI2Cutil.h
/// @brief  Defines the interface to the I2C driver for the WM8350.
///
/// This file contains the interface for communicating with the WM8350 over
/// the 2-wire (I2C) interface.  This code is specifically for the PMIC
/// interface and should be run over a dedicated I2C bus.
///
/// @version $Id: pmi2cutil.h 298 2007-04-13 09:43:55Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
////////////////////////////////////////////////////////////////////////////////
#ifndef __PMI2CUTIL_H__
#define __PMI2CUTIL_H__

//
// Include files.
//
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define I2C_WRITE          1
#define I2C_READ           0

//------------------------------------------------------------------------------
// Functions

BOOL PmicI2CInitialize( int index, UINT32 dwFrequency );
VOID PmicI2CRelease( int index );
VOID PmicI2CPowerDown();
VOID PmicI2CSync();
BOOL PmicI2CLock();
VOID PmicI2CUnlock();
BOOL PmicI2CWriteRegister( BYTE devAddr, UINT32 regAddr, UINT32 regval );
BOOL PmicI2CReadRegister( BYTE devAddr, UINT32 addr, UINT32 *pRegval );
BOOL PmicI2CWriteData( BYTE devAddr, BYTE *pData, UINT nBytes );
BOOL PmicI2CReadData( BYTE devAddr,
                      BYTE *pWriteData,
                      UINT writeBytes,
                      BYTE *pReadData,
                      UINT readBytes
                    );


#ifdef __cplusplus
}
#endif

#endif // __PMI2CUTIL_H__
//////////////////////////////// END OF FILE ///////////////////////////////////
