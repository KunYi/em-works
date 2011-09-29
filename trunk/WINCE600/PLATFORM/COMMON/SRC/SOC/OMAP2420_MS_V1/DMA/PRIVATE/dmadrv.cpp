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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
#include <windows.h>
#include "dmasys.h"

static DMASystem *gpSys = NULL;

#ifdef DEBUG

DBGPARAM dpCurSettings = {
    L"DMA", {
        L"Errors",      L"Warnings",    L"Function",    L"Init",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0003
};

#endif

DWORD DMA_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
    if (gpSys)
        return DMADRVERR_ALREADYINIT;
    /* szContext has active key in registry */
    DMASystem * pSys = DMASystem::Create(szContext);
    if (!pSys)
        return 0;
    gpSys = pSys;
    gpSys->Lock();
    uint err;
    __try {
        err = gpSys->Init();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        err = DMADRVERR_EXCEPTION;
    }
    gpSys->Unlock();
    SetLastError(err);
    if (err)
        return 0;
    return (DWORD)gpSys;
}

BOOL DMA_Deinit(DWORD context)
{
    if ((!gpSys) || (context!=(DWORD)gpSys))
    {
        SetLastError(DMADRVERR_NOINIT);
        return FALSE;
    }
    gpSys->Lock();
    uint err;
    __try {
        err = gpSys->Deinit();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        err = DMADRVERR_EXCEPTION;
    }
    gpSys->Unlock();

    if (!err)
    {
        delete gpSys;
        gpSys = NULL;
        return TRUE;
    } 

    return FALSE;
}

DWORD DMA_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
    if ((!gpSys) || (context!=(DWORD)gpSys))
    {
        SetLastError(DMADRVERR_NOINIT);
        return 0;
    }
    gpSys->Lock();
    uint err;
    DWORD dwOpenContext = 0;
    __try {
        err = gpSys->Open(accessCode,shareMode,&dwOpenContext);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        dwOpenContext = 0;
        err = DMADRVERR_EXCEPTION;
    }
    gpSys->Unlock();

    SetLastError(err);
    return dwOpenContext;
}

BOOL DMA_Close(DWORD openContext)
{
    if (!gpSys)
    {
        SetLastError(DMADRVERR_NOINIT);
        return 0;
    }
    gpSys->Lock();
    uint err;
    __try {
        err = gpSys->Close(openContext);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        err = DMADRVERR_EXCEPTION;
    }
    gpSys->Unlock();

    SetLastError(err);
    return (err==0)?TRUE:FALSE;
}

DWORD DMA_Read(DWORD openContext, PVOID pBuffer, DWORD size)
{
    if (!gpSys)
    {
        SetLastError(DMADRVERR_NOINIT);
        return 0;
    }
    SetLastError(DMADRVERR_NOTSUPPORTED);
    return (DWORD)-1;
}

DWORD DMA_Write(DWORD openContext, PVOID pBuffer, DWORD size)
{
    if (!gpSys)
    {
        SetLastError(DMADRVERR_NOINIT);
        return 0;
    }
    SetLastError(DMADRVERR_NOTSUPPORTED);
    return (DWORD)-1;
}

BOOL DMA_IOControl(
    DWORD openContext, DWORD code, BYTE *pInBuffer, DWORD inSize, BYTE *pOutBuffer,
    DWORD outSize, DWORD *pOutSize) 
{
    if (!gpSys)
    {
        SetLastError(DMADRVERR_NOINIT);
        return 0;
    }
    gpSys->Lock();
    uint err;
    __try {
        err = gpSys->Command(openContext,code,pInBuffer,inSize,pOutBuffer,outSize,pOutSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        err = DMADRVERR_EXCEPTION;
    }
    gpSys->Unlock();

    SetLastError(err);
    return (err==0)?TRUE:FALSE;
}

VOID DMA_PowerUp(DWORD context)
{
    /* no functioning */
}

void DMA_PowerDown(DWORD context)
{
    /* no functioning */
}

