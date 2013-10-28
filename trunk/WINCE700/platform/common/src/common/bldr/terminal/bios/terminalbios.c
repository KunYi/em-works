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
#include <bootTerminalBios.h>
#include <bootBios.h>
#include <bootLog.h>
#include <bootSerial.h>

//------------------------------------------------------------------------------

typedef struct Terminal_t {
    BootDriverVTable_t *pVTable;
    enum_t port;
    WCHAR buffer[512];
} Terminal_t;

//------------------------------------------------------------------------------
//  Local functions

bool_t
BootTerminalBiosDeinit(
    void *pContext
    );

bool_t
BootTerminalBiosIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    );

//------------------------------------------------------------------------------

static 
BootDriverVTable_t
s_terminalVTable = {
    BootTerminalBiosDeinit,
    BootTerminalBiosIoCtl
};

static 
Terminal_t
s_terminal;


//------------------------------------------------------------------------------

static
bool_t
WriteStringToDisplay(
    wcstring_t string
    )
{
    bool_t rc = false;
    uint32_t eax, ebx, ecx, edx, esi, edi;

    if (string == NULL) goto cleanUp;

    while (string[0] != L'\0')
        {
        eax = 0x0E00 | (uint8_t)string[0];
        ebx = 0x0007;
        BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
        string++;
        }

    rc = true;

cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------

static
wchar_t
ReadCharFromKeyboard(
    )
{
    wchar_t ch = L'\0';
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0, esi = 0, edi = 0;

    eax = 0x0100;
    BootBiosInt16(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if (eax != 0)
        {
        eax = 0x0000;
        BootBiosInt16(&eax, &ebx, &ecx, &edx, &esi, &edi);
        ch = (uint16_t)(eax & 0xFF);
        }
    return ch;
}

//------------------------------------------------------------------------------

static
bool_t
WriteStringToSerial(
    uint32_t port,
    wcstring_t string
    )
{
    bool_t rc = false;
    uint16_t ioPortBase;

    if (string == NULL) 
    {
        goto cleanUp;
    }

    switch (port)
    {
        case BOOT_TERMINAL_BIOS_PORT_COM1: 
            ioPortBase = COM1_BASE;
            break;
        case BOOT_TERMINAL_BIOS_PORT_COM2: 
            ioPortBase = COM2_BASE;
            break;
        case BOOT_TERMINAL_BIOS_PORT_COM3: 
            ioPortBase = COM3_BASE;
            break;
        case BOOT_TERMINAL_BIOS_PORT_COM4: 
            ioPortBase = COM4_BASE;
            break;
        default:
            ioPortBase = 0;
            break;
    }

    if (ioPortBase == 0) 
    {
        goto cleanUp;
    }

    while (*string != L'\0')
    {
        BootBiosWriteByteToSerialPort(ioPortBase, (uint8_t) (*string));
        string++;
    }
    
    rc = true;

cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------

static
wchar_t
ReadCharFromSerial(
    uint32_t port
    )
{
    uint16_t ioPortBase;
    uint8_t byte = (uint8_t)-1;

    switch (port)
    {
        case BOOT_TERMINAL_BIOS_PORT_COM1: 
            ioPortBase = COM1_BASE;
            break;
        case BOOT_TERMINAL_BIOS_PORT_COM2: 
            ioPortBase = COM2_BASE;
            break;
        case BOOT_TERMINAL_BIOS_PORT_COM3: 
            ioPortBase = COM3_BASE;
            break;
        case BOOT_TERMINAL_BIOS_PORT_COM4: 
            ioPortBase = COM4_BASE;
            break;
        default:
            ioPortBase = 0;
            break;
    }

    if (ioPortBase)
    {
        byte = BootBiosReadByteFromSerialPort(ioPortBase);
    }

    return byte;
}

//------------------------------------------------------------------------------

handle_t
BootTerminalBiosInit(
    enum_t port
    )
{
    uint16_t ioPortBase;

    s_terminal.pVTable = &s_terminalVTable;
    s_terminal.port = port;

    switch (port)
        {
        case BOOT_TERMINAL_BIOS_PORT_COM1:
            ioPortBase = COM1_BASE;
            break;
        case BOOT_TERMINAL_BIOS_PORT_COM2:
            ioPortBase = COM2_BASE;
            break;
        case BOOT_TERMINAL_BIOS_PORT_COM3:
            ioPortBase = COM3_BASE;
            break;
        case BOOT_TERMINAL_BIOS_PORT_COM4:
            ioPortBase = COM4_BASE;
            break;
        default:
            ioPortBase = 0;
            break;
        }
    if (ioPortBase)
    {
        BootBiosInitializeSerialPort(ioPortBase);
    }
    
    return &s_terminal;
}

//------------------------------------------------------------------------------

bool_t
BootTerminalBiosDeinit(
    void *pContext
    )
{
    bool_t rc = FALSE;
    Terminal_t *pTerminal = pContext;


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
BootTerminalBiosIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool_t rc = FALSE;
    Terminal_t *pTerminal = pContext;


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
            switch (pTerminal->port)
                {
                case BOOT_TERMINAL_BIOS_PORT_COM1:
                    rc = WriteStringToSerial(0, pParams->string);
                    break;
                case BOOT_TERMINAL_BIOS_PORT_COM2:
                    rc = WriteStringToSerial(1, pParams->string);
                    break;
                case BOOT_TERMINAL_BIOS_PORT_COM3:
                    rc = WriteStringToSerial(2, pParams->string);
                    break;
                case BOOT_TERMINAL_BIOS_PORT_COM4:
                    rc = WriteStringToSerial(3, pParams->string);
                    break;
                case BOOT_TERMINAL_BIOS_PORT_CONSOLE:
                    rc = WriteStringToDisplay(pParams->string);
                    break;
                }
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
            switch (pTerminal->port)
                {
                case BOOT_TERMINAL_BIOS_PORT_COM1:
                    pParams->ch = ReadCharFromSerial(0);
                    rc = true;
                    break;
                case BOOT_TERMINAL_BIOS_PORT_COM2:
                    pParams->ch = ReadCharFromSerial(1);
                    rc = true;
                    break;
                case BOOT_TERMINAL_BIOS_PORT_COM3:
                    pParams->ch = ReadCharFromSerial(3);
                    rc = true;
                    break;
                case BOOT_TERMINAL_BIOS_PORT_COM4:
                    pParams->ch = ReadCharFromSerial(4);
                    rc = true;
                    break;
                default:
                    pParams->ch = ReadCharFromKeyboard();
                    rc = true;
                    break;
                }
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
            BootLogVSPrintf(
                pTerminal->buffer, dimof(pTerminal->buffer), pParams->format,
                pParams->pArgList, TRUE
                );
            switch (pTerminal->port)
                {
                case BOOT_TERMINAL_BIOS_PORT_COM1:
                    rc = WriteStringToSerial(0, pTerminal->buffer);
                    break;
                case BOOT_TERMINAL_BIOS_PORT_COM2:
                    rc = WriteStringToSerial(1, pTerminal->buffer);
                    break;
                case BOOT_TERMINAL_BIOS_PORT_COM3:
                    rc = WriteStringToSerial(2, pTerminal->buffer);
                    break;
                case BOOT_TERMINAL_BIOS_PORT_COM4:
                    rc = WriteStringToSerial(3, pTerminal->buffer);
                    break;
                default:
                    rc = WriteStringToDisplay(pTerminal->buffer);
                    break;
                }
            }                
            break;
        }
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

