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
#include <bootTransportFileSys.h>
#include <bootFileSystem.h>
#include <bootCore.h>
#include <bootLog.h>

//------------------------------------------------------------------------------

typedef struct Transport_t {

    BootDriverVTable_t *pVTable;

    void*       pContext;
    handle_t    hFileSys;
    handle_t    hFile;
    
} Transport_t;

//------------------------------------------------------------------------------
//  Local functions

bool_t
BootTransportFileSystemDeinit(
    void *pContext
    );

bool_t
BootTransportFileSystemIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    );

static
bool_t
Read(
    Transport_t *pContext,
    void *pBuffer,
    size_t bufferLength
    );

//------------------------------------------------------------------------------

static
BootDriverVTable_t
s_transportVTable = {
    BootTransportFileSystemDeinit,
    BootTransportFileSystemIoCtl
};

static
Transport_t
s_transport;

//------------------------------------------------------------------------------

handle_t
BootTransportFileSystemInit(
    void *pContext,
    handle_t hFileSys,
    wcstring_t fileName
    )
{
    void *pTransport = NULL;
    
    // This driver shouldn't be instantied twice...
    if (s_transport.pVTable != NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportFileSystemInit: "
            L"Driver already initialized!\r\n"
            ));
        goto cleanUp;
        }

    memset(&s_transport, 0, sizeof(s_transport));

    s_transport.hFile = BootFileSystemOpen(
        hFileSys, fileName, BOOT_FILESYSTEM_ACCESS_READ, 
        BOOT_FILESYSTEM_ATTRIBUTE_NORMAL
        );
    if (s_transport.hFile == NULL) goto cleanUp;
    s_transport.hFileSys = hFileSys;
    s_transport.pContext = pContext;
        
    // Initialize virtual table & save context
    s_transport.pVTable = &s_transportVTable;
    s_transport.pContext = pContext;

    // Done
    pTransport = &s_transport;

cleanUp:
    return pTransport;
}

//------------------------------------------------------------------------------

bool_t
BootTransportFileSystemDeinit(
    void *pContext
    )
{
    bool_t rc = FALSE;
    Transport_t *pTransport = pContext;


    // Check driver handle
    if (pTransport != &s_transport)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportFileSystemDeinit: "
            L"Invalid driver handle!\r\n"
            ));
        goto cleanUp;
        }

    // Close file system handles
    BootFileSystemClose(pTransport->hFile);
    BootFileSystemDeinit(pTransport->hFileSys);

    // Clear context
    memset(pTransport, 0, sizeof(*pTransport));

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootTransportFileSystemIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool_t rc = FALSE;
    Transport_t *pTransport = pContext;

    
    // Check driver handle
    if (pTransport != &s_transport)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgIoCtl: "
            L"Invalid driver handle!\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case BOOT_TRANSPORT_IOCTL_READ:
            {
            BootTransportReadParams_t *pParams = pBuffer;
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgIoCtl: "
                    L"Invalid BOOT_TRANSPORT_IOCTL_READ parameter!\r\n"
                    ));
                break;
                }
            rc = Read(pTransport, pParams->pBuffer, pParams->size);
            }
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
Read(
    Transport_t *pTransport,
    VOID *pBuffer,
    size_t bufferLength
    )
{
    return BootFileSystemRead(pTransport->hFile, pBuffer, bufferLength);
}

//------------------------------------------------------------------------------

