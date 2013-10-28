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

#ifndef _IOCTL_PMIC_TAB_H_
#define _IOCTL_PMIC_TAB_H_

// IOCTLs supporting exclusive CSPI access between OAL and all device drivers.
//
// Note that we must specify OAL_IOCTL_FLAG_NOCS to avoid using a critical
// section during the CSPI lock/unlock IOCTL calls. Otherwise, we can deadlock
// the system if we also happen to block while within the IOCTL call because
// we cannot immediately acquire the CSPI lock (which is another critical
// section).

{ IOCTL_PMIC_CSPI_LOCK,           OAL_IOCTL_FLAG_NOCS, OALPmicIoctlCspiLock   },
{ IOCTL_PMIC_CSPI_UNLOCK,         OAL_IOCTL_FLAG_NOCS, OALPmicIoctlCspiUnlock },

#endif // _IOCTL_PMIC_TAB_H_
