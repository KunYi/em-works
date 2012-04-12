//-----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
