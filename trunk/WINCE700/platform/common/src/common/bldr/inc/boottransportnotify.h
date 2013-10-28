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
#ifndef __BOOT_TRANSPORT_NOTIFY_H
#define __BOOT_TRANSPORT_NOTIFY_H

#include <bootNotify.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BOOT_TRANSPORT_NOTIFY(i)    BOOT_NOTIFY(BOOT_DRIVER_CLASS_TRANSPORT, i)

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_TRANSPORT_NOTIFY_H
