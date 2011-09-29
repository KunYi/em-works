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
#define DMA_C 1
#include "dma_arb.h"
#include "..\private\dmadrv.h"

HANDLE ghDrv = NULL;

/* used to open the driver for the first time */
static CRITICAL_SECTION  sgUseSec;
static BOOL CheckDriver(void)
{
    HANDLE ret;
    EnterCriticalSection(&sgUseSec);
    if (ghDrv==NULL)
    {
        ret = CreateFile(TEXT("DMA1:"),
                         GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
        if (ret!=INVALID_HANDLE_VALUE)
        {
            ghDrv = ret;
        }
    }
    LeaveCriticalSection(&sgUseSec);
    return (ghDrv!=NULL)?TRUE:FALSE;
}

static void ShutDown(void)
{
    EnterCriticalSection(&sgUseSec);
    if (ghDrv!=NULL)
    {
        CloseHandle(ghDrv);
        ghDrv = NULL;
    }
    LeaveCriticalSection(&sgUseSec);
}

uint DMA_ControllerEnum(DMACONTROLLER *apRetArray, uint *apSizeBytes)
{
    DWORD enumBytes;
    DWORD retBytes;
    BOOL ioRet;

    if (!CheckDriver())
        return DMAERR_NODRIVER;

    if (IsBadWritePtr(apSizeBytes,sizeof(uint)))
        return DMAERR_BADPOINTER;

    enumBytes = 0;
    retBytes = 0;
    ioRet = DeviceIoControl(ghDrv,
                            DMA_IOCTL_ENUM_GETSIZE,
                            NULL,
                            0,
                            (LPVOID)&enumBytes,
                            sizeof(DWORD),
                            &retBytes,
                            NULL);

    if (!ioRet)
        return DMAERR_NODRIVER;

    /* enumBytes holds required size of enum now */
    ioRet = (*apSizeBytes<enumBytes);
    *apSizeBytes = enumBytes;
    if (ioRet)
        return DMAERR_NOTENOUGHSPACE;

    /* there is enough room in the target to hold the data */
    if (IsBadWritePtr(apRetArray,enumBytes))
        return DMAERR_BADPOINTER;

    ioRet = DeviceIoControl(ghDrv,
                            DMA_IOCTL_ENUM_GETDATA,
                            NULL,
                            0,
                            (LPVOID)apRetArray,
                            enumBytes,
                            &enumBytes,
                            NULL);
    if (!ioRet)
        return DMAERR_NODRIVER;

    /* enumeration came back ok */
    *apSizeBytes = enumBytes;
    return 0;
}

uint DMA_ControllerOpen(uint aSysId, HANDLE *apRetHandle)
{
    DMAIOCTL inIoctl,outIoctl;
    DWORD retBytes;
    BOOL ioRet;

    if (!CheckDriver())
        return DMAERR_NODRIVER;

    /* check arguments */
    if (IsBadWritePtr(apRetHandle,sizeof(HANDLE)))
        return DMAERR_BADPOINTER;

    inIoctl.mParam[0] = (uint)aSysId;
    inIoctl.mParam[1] = 0;
    inIoctl.mParam[2] = 0;
    inIoctl.mParam[3] = 0;

    ioRet = DeviceIoControl(ghDrv,
                            DMA_IOCTL_OPEN,
                            (LPVOID)&inIoctl,
                            sizeof(inIoctl),
                            (LPVOID)&outIoctl,
                            sizeof(outIoctl),
                            &retBytes,
                            NULL);
    if (!ioRet)
        return DMAERR_NODRIVER;

    if (outIoctl.mParam[3])
    {
        /* driver is returning error code */
        *apRetHandle = NULL;
        return outIoctl.mParam[3];
    }

    /* driver opened controller ok. */
    *apRetHandle = (HANDLE)outIoctl.mParam[0];
    return 0;
}

uint DMA_ControllerAcquireChannels(HANDLE aController, uint aNumChannels, uint *apChanIO)
{
    DMAIOCTL inIoctl,outIoctl;
    DWORD retBytes;
    BOOL ioRet;

    if (!CheckDriver())
        return DMAERR_NODRIVER;

    /* check arguments */
    if (!aController)
        return DMAERR_CONTROLLERNOTFOUND;
    if (IsBadWritePtr(apChanIO,sizeof(uint)))
        return DMAERR_BADPOINTER;
    if (aNumChannels>32)
        return DMAERR_BADNUMCHANNELS;

    inIoctl.mParam[0] = (uint)aController;
    inIoctl.mParam[1] = aNumChannels;
    inIoctl.mParam[2] = *apChanIO;
    inIoctl.mParam[3] = 0;

    ioRet = DeviceIoControl(ghDrv,
                            DMA_IOCTL_ACQ,
                            (LPVOID)&inIoctl,
                            sizeof(inIoctl),
                            (LPVOID)&outIoctl,
                            sizeof(outIoctl),
                            &retBytes,
                            NULL);
    if (!ioRet)
        return DMAERR_NODRIVER;

    if (outIoctl.mParam[3])
    {
        /* driver is returning error code */
        *apChanIO = 0;
        return outIoctl.mParam[3];
    }

    /* driver acquired channels ok */
    *apChanIO = outIoctl.mParam[2];
    return 0;
}

uint DMA_ControllerFreeChannels(HANDLE aController, uint aChanFreeMask)
{
    DMAIOCTL inIoctl,outIoctl;
    DWORD retBytes;
    BOOL ioRet;

    if (!CheckDriver())
        return DMAERR_NODRIVER;

    /* check arguments */
    if (!aController)
        return DMAERR_CONTROLLERNOTFOUND;
    if (!aChanFreeMask)
        return DMAERR_BADCHANMASK;

    inIoctl.mParam[0] = (uint)aController;
    inIoctl.mParam[1] = aChanFreeMask;
    inIoctl.mParam[2] = 0;
    inIoctl.mParam[3] = 0;

    ioRet = DeviceIoControl(ghDrv,
                            DMA_IOCTL_FREE,
                            (LPVOID)&inIoctl,
                            sizeof(inIoctl),
                            (LPVOID)&outIoctl,
                            sizeof(outIoctl),
                            &retBytes,
                            NULL);
    if (!ioRet)
        return DMAERR_NODRIVER;

    return outIoctl.mParam[3];
}

uint DMA_ControllerSet(HANDLE aController, DMA_CONT_PROPERTY aProp, uint aValue32)
{
    DMAIOCTL inIoctl,outIoctl;
    DWORD retBytes;
    BOOL ioRet;

    if (!CheckDriver())
        return DMAERR_NODRIVER;

    /* check arguments */
    if (!aController)
        return DMAERR_CONTROLLERNOTFOUND;
    if (!(uint)aProp)
        return DMAERR_BADPROPERTY;

    inIoctl.mParam[0] = (uint)aController;
    inIoctl.mParam[1] = (uint)aProp;
    inIoctl.mParam[2] = aValue32;
    inIoctl.mParam[3] = 0;

    ioRet = DeviceIoControl(ghDrv,
                            DMA_IOCTL_SET,
                            (LPVOID)&inIoctl,
                            sizeof(inIoctl),
                            (LPVOID)&outIoctl,
                            sizeof(outIoctl),
                            &retBytes,
                            NULL);
    if (!ioRet)
        return DMAERR_NODRIVER;

    return outIoctl.mParam[3];
}

uint DMA_ControllerGet(HANDLE aController, DMA_CONT_PROPERTY aProp, uint *apRetValue32)
{
    DMAIOCTL inIoctl,outIoctl;
    DWORD retBytes;
    BOOL ioRet;

    if (!CheckDriver())
        return DMAERR_NODRIVER;

    /* check arguments */
    if (!aController)
        return DMAERR_CONTROLLERNOTFOUND;
    if (!(uint)aProp)
        return DMAERR_BADPROPERTY;
    if (IsBadWritePtr(apRetValue32,sizeof(uint)))
        return DMAERR_BADPOINTER;

    inIoctl.mParam[0] = (uint)aController;
    inIoctl.mParam[1] = (uint)aProp;
    inIoctl.mParam[2] = 0;
    inIoctl.mParam[3] = 0;

    ioRet = DeviceIoControl(ghDrv,
                            DMA_IOCTL_GET,
                            (LPVOID)&inIoctl,
                            sizeof(inIoctl),
                            (LPVOID)&outIoctl,
                            sizeof(outIoctl),
                            &retBytes,
                            NULL);
    if (!ioRet)
        return DMAERR_NODRIVER;

    if (outIoctl.mParam[3])
    {
        /* driver is returning error code */
        *apRetValue32 = 0;
        return outIoctl.mParam[3];
    }

    /* driver retrieved property ok */
    *apRetValue32 = outIoctl.mParam[2];
    return 0;
}

uint DMA_ControllerClose(HANDLE aController)
{
    DMAIOCTL inIoctl,outIoctl;
    DWORD retBytes;
    BOOL ioRet;

    if (!CheckDriver())
        return DMAERR_NODRIVER;

    /* check arguments */
    if (!aController)
        return DMAERR_CONTROLLERNOTFOUND;

    inIoctl.mParam[0] = (uint)aController;
    inIoctl.mParam[1] = 0;
    inIoctl.mParam[2] = 0;
    inIoctl.mParam[3] = 0;

    ioRet = DeviceIoControl(ghDrv,
                            DMA_IOCTL_CLOSE,
                            (LPVOID)&inIoctl,
                            sizeof(inIoctl),
                            (LPVOID)&outIoctl,
                            sizeof(outIoctl),
                            &retBytes,
                            NULL);
    if (!ioRet)
        return DMAERR_NODRIVER;

    return outIoctl.mParam[3];
}

BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason==DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls((HMODULE)hDll);
        InitializeCriticalSection(&sgUseSec);
    }
    else if (dwReason==DLL_PROCESS_DETACH)
    {
        ShutDown();
        DeleteCriticalSection(&sgUseSec);
    }
    return TRUE;
}