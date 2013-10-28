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
BootTerminalMenu(
    void *pContext,
    handle_t hTerminal,
    void *pActionContext
    )
{
    BootTerminalMenu_t *pMenu = pActionContext;
    BootTerminalMenuEntry_t *pEntry;
    wchar_t key, line[80];    
    enum_t ix;

    if ((hTerminal == NULL) || (pMenu == NULL)) goto cleanUp;
    
    for (;;)
        {

        // Print header
        BootTerminalPrintf(hTerminal, L"\r\n");
        for (ix = 0; ix < dimof(line) - 1; ix++) line[ix] = L'-';
        line[dimof(line) - 1] = L'\0';
        BootTerminalPrintf(hTerminal, L"%s\r\n", line);
        BootTerminalPrintf(hTerminal, L" %s\r\n", pMenu->title);
        BootTerminalPrintf(hTerminal, L"%s\r\n\r\n", line);

        // Print menu items
        for (pEntry = pMenu->entries; pEntry->key != 0; pEntry++)
            {
            BootTerminalPrintf(
                hTerminal, L" [%c] %s\r\n", pEntry->key, pEntry->text
                );
            }
        BootTerminalPrintf(hTerminal, L"\r\n Selection: ");

        for (;;)
            {
            // Get key
            key = BootTerminalReadChar(hTerminal);
            if (key == L'\0') continue;
        
            // Look for key in menu
            for (pEntry = pMenu->entries; pEntry->key != L'\0'; pEntry++)
                {
                if (pEntry->key == key) break;
                }

            // If we find it, break loop
            if (pEntry->key != L'\0') break;
            }
    
        // Print out selection character
        BootTerminalPrintf(hTerminal, L"%c\r\n", key);
    
        // When action is NULL return back to parent menu
        if ((pEntry->pfnAction == NULL) && (pEntry->pActionContext == NULL))
            break;

        if (pEntry->pfnAction == NULL)
            {
            BootTerminalMenu(pContext, hTerminal, pEntry->pActionContext);
            }
        else
            {
            pEntry->pfnAction(pContext, hTerminal, pEntry->pActionContext);
            }
        
        }            

cleanUp:
    return;
}

//------------------------------------------------------------------------------

