//-----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
