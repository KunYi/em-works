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
// This driver encapsulates terminal functionality.
//
#ifndef __BOOT_TERMINAL_H
#define __BOOT_TERMINAL_H

#include <bootDriver.h>
#include <bootDriverClasses.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BOOT_TERMINAL_IOCTL(i)      BOOT_IOCTL(BOOT_DRIVER_CLASS_TERMINAL, i)
#define BOOT_TERMINAL_IOCTL_OEM(i)  BOOT_IOCTL(BOOT_DRIVER_CLASS_TERMINAL, 0x8000 + i)

enum BootTerminalIoCtl_e {
    BOOT_TERMINAL_IOCTL_WRITE_STRING    = BOOT_TERMINAL_IOCTL(1),
    BOOT_TERMINAL_IOCTL_READ_CHAR       = BOOT_TERMINAL_IOCTL(2),
    BOOT_TERMINAL_IOCTL_VPRINTF         = BOOT_TERMINAL_IOCTL(3)
};

typedef struct BootTerminalReadCharParams_t {
    wchar_t ch;
} BootTerminalReadCharParams_t;

typedef struct BootTerminalWriteStringParams_t {
    wcstring_t string;
} BootTerminalWriteStringParams_t;

typedef struct BootTerminalVPrintfParams_t {
    wcstring_t format;
    va_list pArgList;
} BootTerminalVPrintfParams_t;

//------------------------------------------------------------------------------

#define BootTerminalDeinit      BootDriverDeinit
#define BootTerminalIoCtl       BootDriverIoCtl

//------------------------------------------------------------------------------
//
//  Function:  BootTerminalRead
//
__inline
WCHAR
BootTerminalReadChar(
    __in handle_t hDriver
    )
{
    WCHAR ch = L'\0';
    BootTerminalReadCharParams_t params;

    if (BootDriverIoCtl(
            hDriver, BOOT_TERMINAL_IOCTL_READ_CHAR, &params, sizeof(params)
            ))
        {
        ch = params.ch;
        }
    return ch;
}

//------------------------------------------------------------------------------
//
//  Function:  BootTerminalWrite
//
__inline
bool_t
BootTerminalWriteString(
    __in handle_t hDriver,
    __in_z wcstring_t string
    )
{
    BootTerminalWriteStringParams_t params;

    params.string = string;
    return BootDriverIoCtl(
        hDriver, BOOT_TERMINAL_IOCTL_WRITE_STRING, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------
//
//  Function:  BootTerminalPrintf
//
__inline
bool_t
BootTerminalPrintf(
    __in handle_t hDriver,
    __format_string wcstring_t format,
    ...
    )
{
    BootTerminalVPrintfParams_t params;
    va_list pArgList;
    
    va_start(pArgList, format);
    params.format = format;
    params.pArgList = pArgList;
    return BootDriverIoCtl(
        hDriver, BOOT_TERMINAL_IOCTL_VPRINTF, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------
//
//  Function:  BootTerminalVPrintf
//
__inline
bool_t
BootTerminalVPrintf(
    __in handle_t hDriver,
    __format_string wcstring_t format,
    __in va_list pArgList
    )
{
    BootTerminalVPrintfParams_t params;
    
    params.format = format;
    params.pArgList = pArgList;
    return BootDriverIoCtl(
        hDriver, BOOT_TERMINAL_IOCTL_VPRINTF, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_TERMINAL_H
