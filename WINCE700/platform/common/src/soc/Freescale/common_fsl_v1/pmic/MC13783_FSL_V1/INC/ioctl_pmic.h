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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------

#ifndef _IOCTL_PMIC_H_
#define _IOCTL_PMIC_H_

#include <winioctl.h>

#define CSPI_LOCK   0x0401
#define CSPI_UNLOCK 0x0402

#define IOCTL_PMIC_CSPI_LOCK   CTL_CODE(FILE_DEVICE_UNKNOWN, CSPI_LOCK, \
                                        0, FILE_ANY_ACCESS)

#define IOCTL_PMIC_CSPI_UNLOCK CTL_CODE(FILE_DEVICE_UNKNOWN, CSPI_UNLOCK, \
                                        0, FILE_ANY_ACCESS)

#endif // _IOCTL_PMIC_H_
