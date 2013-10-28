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
#ifndef __BOOT_TERMINAL_BIOS_H
#define __BOOT_TERMINAL_BIOS_H

#include <bootTerminal.h>
#include <bootBios.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

enum BootTerminalBiosPort_e {
    BOOT_TERMINAL_BIOS_PORT_COM1 = 0,
    BOOT_TERMINAL_BIOS_PORT_COM2,
    BOOT_TERMINAL_BIOS_PORT_COM3,
    BOOT_TERMINAL_BIOS_PORT_COM4,
    BOOT_TERMINAL_BIOS_PORT_CONSOLE = 0x1000
};

//------------------------------------------------------------------------------

handle_t
BootTerminalBiosInit(
    enum_t port
    );

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_TERMINAL_BIOS_H
