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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File: buses.h
//
#ifndef __BUSES_H
#define __BUSES_H

//------------------------------------------------------------------------------

#ifdef DEBUG

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INIT           DEBUGZONE(3)
#define ZONE_INFO           DEBUGZONE(4)
#define ZONE_IST            DEBUGZONE(5)
#define ZONE_POWER          DEBUGZONE(6)

#define ZONE_HDQ_1WIRE      DEBUGZONE(12)
#define ZONE_I2C            DEBUGZONE(13)
#define ZONE_GPIO           DEBUGZONE(14)
#define ZONE_CLK            DEBUGZONE(15)

#endif

//------------------------------------------------------------------------------

#endif
