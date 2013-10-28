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

static
bool_t
StringToIp4(
    wcstring_t szIp4,
    uint32_t *pIp4
    )
{
    bool_t rc = false;
    uint32_t ip4 = 0;
    wcstring_t psz = szIp4;
    enum_t count = 0, part = 0;
    
    // Replace the dots with NULL terminators
    while (count < 4)
        {
        if ((*psz == L'.') || (*psz == L'\0'))
            {
            ip4 |= part << (count << 3);
            part = 0;
            count++;
            }
        else if ((*psz >= L'0') && (*psz <= L'9'))
            {
            part = part * 10 + (*psz - L'0');
            if (part > 255) break;
            }
        else 
            {
            break;
            }
        if (*psz == L'\0') break;
        psz++;
        }

    rc = (count >= 4);
    if (rc) *pIp4 = ip4;
    return rc;
} 

//------------------------------------------------------------------------------

bool_t
BootTerminalReadIp4(
    handle_t hTerminal,
    uint32_t *pIp4,
    wcstring_t prompt
    )
{
    bool_t rc = false;
    uint32_t ip4 = *pIp4;
    wchar_t buffer[16];

    // Print prompt
    BootTerminalPrintf(
        hTerminal, L" Enter %s IP address (actual %d.%d.%d.%d): ", prompt,
        ((uint8_t*)&ip4)[0], ((uint8_t*)&ip4)[1], ((uint8_t*)&ip4)[2],
        ((uint8_t*)&ip4)[3]
        );

    // Read input line
    if (BootTerminalReadLine(hTerminal, buffer, dimof(buffer)) == 0)
        goto cleanUp;

    // Convert string to IP address
    if (!StringToIp4(buffer, &ip4))
        {
        BootTerminalPrintf(
            hTerminal, L" '%s' isn't valid IP address\r\n", buffer
            );
        goto cleanUp;
        }

    // Print final IP address
    BootTerminalPrintf(
        hTerminal, L" %s IP address set to %d.%d.%d.%d\r\n", prompt,
        ((uint8_t*)&ip4)[0], ((uint8_t*)&ip4)[1], ((uint8_t*)&ip4)[2],
        ((uint8_t*)&ip4)[3]
        );
    
    // Save new setting
    *pIp4 = ip4;
    rc = true;
    
cleanUp:
    return rc;
}    

//------------------------------------------------------------------------------

