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
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  i2cbus.h
//
//  Header file for I2C bus driver.
//
//------------------------------------------------------------------------------
#ifndef __I2CBUS_H__
#define __I2CBUS_H__

#ifdef __cplusplus
extern "C" {
#endif

// I2C Bus Class
#define I2C_BUS_CAM                 1           // CAM Bus Index
#define I2C_BUS_AUD                 2           // AUD Bus Index

// I2C File Name
#define I2C_FID_CAM                 _T("I2C1:")     // CAM File Index
#define I2C_FID_AUD                 _T("I2C2:")     // AUD File Index

#ifdef __cplusplus
}
#endif

#endif  // __I2CBUS_H__

