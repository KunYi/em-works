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
#pragma once

#include <bootDriver.hpp>

//------------------------------------------------------------------------------

#define BOOT_DISPLAY_IOCTL(i)       BOOT_IOCTL(BOOT_DRIVER_CLASS_DISPLAY, i)

enum BOOT_DISPLAY_IOCTL {
    BOOT_DISPLAY_IOCTL_FILLRECT = BOOT_DISPLAY_IOCTL(0x0001),
    BOOT_DISPLAY_IOCTL_BLTRECT  = BOOT_DISPLAY_IOCTL(0x0002),
    BOOT_DISPLAY_IOCTL_SLEEP    = BOOT_DISPLAY_IOCTL(0x0003),
    BOOT_DISPLAY_IOCTL_AWAKE    = BOOT_DISPLAY_IOCTL(0x0004)
};

typedef struct BootDisplayFillRectParams_t {
    RECT *pRect;
    uint32_t color;
} BootDisplayFillRectParams_t;

typedef struct BootDisplayBltRectParams_t {
    RECT *pRect;
    void *pBuffer;
} BootDisplayBltRectParams_t;

//------------------------------------------------------------------------------

class BootDisplay_t : public BootDriver_t
{
public:

    virtual
    void        
    DeInit(
        );

    virtual
    bool_t        
    IoCtl(
        enum_t code,
        void *pBuffer,
        size_t size
        );        
            
    bool_t
    FillRect(
        RECT *pRect,
        uint32_t color
        ) {
        BootDisplayFillRectParams_t params;
    
        params.pRect = pRect;
        params.color = color;
        return IoCtl(BOOT_DISPLAY_IOCTL_FILLRECT, &params, sizeof(params));
        };

    bool_t
    BltRect(
        RECT* pRect,
        VOID* pBuffer
        ) {
        BootDisplayBltRectParams_t params;

        params.pRect = pRect;
        params.pBuffer = pBuffer;
        return IoCtl(BOOT_DISPLAY_IOCTL_BLTRECT, &params, sizeof(params));
        };

    bool_t
    Sleep(
        ) {
        return IoCtl(BOOT_DISPLAY_IOCTL_SLEEP, NULL, 0);
        };

    bool_t
    Awake(
        ) {
        return IoCtl(BOOT_DISPLAY_IOCTL_AWAKE, NULL, 0);
        };

};

//------------------------------------------------------------------------------

