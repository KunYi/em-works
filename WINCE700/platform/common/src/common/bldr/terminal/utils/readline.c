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

size_t
BootTerminalReadLine(
    handle_t hTerminal,
    wchar_t *pBuffer,
    size_t bufferChars
    )
{
    size_t count = 0;
    wchar_t key;

    count = 0;
    while (count < bufferChars)
        {
        key = BootTerminalReadChar(hTerminal);
        if (key == L'\0') continue;
        if ((key == L'\r') || (key == L'\n'))
            {
            BootTerminalWriteString(hTerminal, L"\r\n");
            break;
            }
        if ((key == L'\b') && (count > 0))
            {
            BootTerminalWriteString(hTerminal, L"\b \b");
            count--;
            }
        else if ((key >= L' ') && (key < 128) && (count < (bufferChars - 1)))
            {
            pBuffer[count++] = key;
            BootTerminalPrintf(hTerminal, L"%c", key);
            }
        }
    pBuffer[count] = L'\0';

    return count;
}    

//------------------------------------------------------------------------------

