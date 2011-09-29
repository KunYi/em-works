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
////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2000 by Texas Instruments Incorporated. All rights reserved.
// Property of Texas Instruments Incorporated. Restricted rights to use,
// duplicate or disclose this code are granted through contract.
//
////////////////////////////////////////////////////////////////////////////////
//
// private touch screen IOCTL's and associated data structures.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __OMAP2420_TOUCH_H
#define __OMAP2420_TOUCH_H

#include <winioctl.h>

#define GET_TOUCHPOINT              0x0401
#define ENABLE_TOUCHPAD             0x0402
#define INIT_TOUCHPAD               0x0403
#define GET_TOUCHSTATE              0x0404

#define IOCTL_GET_TOUCHPOINT	CTL_CODE(FILE_DEVICE_UNKNOWN, GET_TOUCHPOINT, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_GET_TOUCHSTATE	CTL_CODE(FILE_DEVICE_UNKNOWN, GET_TOUCHSTATE, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_ENABLE_TOUCHPAD	CTL_CODE(FILE_DEVICE_UNKNOWN, ENABLE_TOUCHPAD, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_INIT_TOUCHPAD		CTL_CODE(FILE_DEVICE_UNKNOWN, INIT_TOUCHPAD, 0, FILE_ANY_ACCESS)

typedef struct _tagTOUCHPOINT {

    UINT32      xpos;
    UINT32      ypos;

} TOUCHPOINT;


typedef struct _tagTOUCHGPIOINFO {

    DWORD               gpioPen;
    DWORD               irqPen;

} TOUCHGPIOINFO;


#define GetTouchPoint(hPAD, pPoint, size) \
    DeviceIoControl(hPAD, IOCTL_GET_TOUCHPOINT, NULL, 0, \
        pPoint, size, NULL, NULL)

#define GetTouchState(hPAD, bVal, size) \
    DeviceIoControl(hPAD, IOCTL_GET_TOUCHSTATE, NULL, 0, \
        bVal, size, NULL, NULL)

#define EnableTouchPoint(hPAD) \
    DeviceIoControl(hPAD, IOCTL_ENABLE_TOUCHPAD, NULL, 0, \
        NULL, 0, NULL, NULL)

#define InitTouchPoint(hPAD, flag, pGpioInfo) \
    DeviceIoControl(hPAD, IOCTL_INIT_TOUCHPAD, flag, sizeof(DWORD), \
        pGpioInfo, sizeof(TOUCHGPIOINFO), NULL, NULL)

#endif // __OMAP2420_TOUCH_H
