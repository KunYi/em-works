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
#include <bootTerminalUtils.h>

//------------------------------------------------------------------------------

void
BootTerminalReadEnable(
    handle_t hTerminal,
    bool_t *pEnable,
    wcstring_t prompt
    )
{
    wchar_t key;
    

    BootTerminalPrintf(
        hTerminal, L" %s %s (actually %s) [y/-]: ",
        *pEnable ? L"Disable" : L"Enable", prompt, 
        *pEnable ? L"enabled" : L"disabled"
        );

    do
        {
        key = BootTerminalReadChar(hTerminal);
        }
    while (key == '\0');
    BootTerminalPrintf(hTerminal, L"%c\r\n", key);

    if ((key == L'y') || (key == L'Y'))
        {
        *pEnable = !*pEnable;
        BootTerminalPrintf(
            hTerminal, L" %s %s\r\n", prompt,
            *pEnable ? L"enabled" : L"disabled"
            );
        }
    else 
        {
        BootTerminalPrintf(
            hTerminal, L" %s stays %s\r\n", prompt, 
            *pEnable ? L"enabled" : L"disabled"
            );
        }
}

//------------------------------------------------------------------------------

