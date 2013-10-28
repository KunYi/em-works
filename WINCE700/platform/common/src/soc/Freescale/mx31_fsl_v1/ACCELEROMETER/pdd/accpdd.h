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
//

#pragma once

#include <windows.h>
#include <ehm.h>
#include "pm.h"

#define ACC_DRIVER_VERSION 1

#ifndef SHIP_BUILD
#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INIT           DEBUGZONE(3)
#define ZONE_IOCTL          DEBUGZONE(4)
#define ZONE_SENSOR         DEBUGZONE(5)
#define ZONE_STATE          DEBUGZONE(6)
#define ZONE_INFO           DEBUGZONE(7)
#define ZONE_DATA           DEBUGZONE(8)

extern DBGPARAM dpCurSettings;
#endif
