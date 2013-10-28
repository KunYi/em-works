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
#include <bootTerminal.h>
#include <bootLog.h>

//------------------------------------------------------------------------------
//  External functions

WCHAR
OEMBootLogReadChar(
    );

//------------------------------------------------------------------------------

typedef struct TERMINAL {
    BootDriverVTable_t *pVTable;
    WCHAR buffer[512];
} TERMINAL;

//------------------------------------------------------------------------------
//  Local functions

bool_t
BootTerminalDebugDeinit(
    void *pContext
    );

bool_t
BootTerminalDebugIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    );

//------------------------------------------------------------------------------

static 
BootDriverVTable_t
s_terminalVTable = {
    BootTerminalDebugDeinit,
    BootTerminalDebugIoCtl
};

static 
TERMINAL
s_terminal;

//------------------------------------------------------------------------------

bool_t
TerminalWriteString(
    TERMINAL* pTerminal,
    LPCWSTR string
    )
{
    UNREFERENCED_PARAMETER(pTerminal);
    OEMBootLogWrite(string);
    return TRUE;
}

//------------------------------------------------------------------------------

WCHAR
TerminalReadChar(
    TERMINAL* pTerminal
    )
{
    UNREFERENCED_PARAMETER(pTerminal);
    return OEMBootLogReadChar();
}

//------------------------------------------------------------------------------

bool_t
TerminalVPrintf(
    TERMINAL* pTerminal,
    LPCWSTR format,
    va_list pArgList
    )
{
    BootLogVSPrintf(
        pTerminal->buffer, dimof(pTerminal->buffer), format, pArgList, TRUE
        );
    return TerminalWriteString(pTerminal, pTerminal->buffer);
}

//------------------------------------------------------------------------------

handle_t
BootTerminalDebugInit(
    )
{
    s_terminal.pVTable = &s_terminalVTable;
    return &s_terminal;
}

//------------------------------------------------------------------------------

bool_t
BootTerminalDebugDeinit(
    VOID *pContext
    )
{
    bool_t rc = FALSE;
    TERMINAL *pTerminal = pContext;


    // Check driver handle
    if (pTerminal != &s_terminal)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTerminalDebugDeinit: "
            L"Invalid terminal driver handle!\r\n"
            ));
        goto cleanUp;
        }

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootTerminalDebugIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool_t rc = FALSE;
    TERMINAL *pTerminal = pContext;


    // Check display handle
    if (pTerminal != &s_terminal)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTerminalDebugDeinit: "
            L"Invalid terminal driver handle!\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case BOOT_TERMINAL_IOCTL_WRITE_STRING:
            {
            BootTerminalWriteStringParams_t *pParams = pBuffer;
                
            // Check input parameter
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootTerminalDebugIoCtl: "
                    L"BootTerminalWriteStringParams_t - "
                    L"Invalid parameters!\r\n"
                    ));
                break;
                }
            rc = TerminalWriteString(pTerminal, pParams->string);
            }                
            break;
        case BOOT_TERMINAL_IOCTL_READ_CHAR:
            {
            BootTerminalReadCharParams_t *pParams = pBuffer;
                
            // Check input parameter
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootTerminalDebugIoCtl: "
                    L"BootTerminalReadCharParams_t - "
                    L"Invalid parameters!\r\n"
                    ));
                break;
                }
            pParams->ch = TerminalReadChar(pTerminal);
            rc = TRUE;
            }                
            break;
        case BOOT_TERMINAL_IOCTL_VPRINTF:
            {
            BootTerminalVPrintfParams_t *pParams = pBuffer;
                
            // Check input parameter
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootTerminalDebugIoCtl: "
                    L"BootTerminalVPrintfParams_t - Invalid parameters!\r\n"
                    ));
                break;
                }
            rc = TerminalVPrintf(pTerminal, pParams->format, pParams->pArgList);
            }                
            break;
        }
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

