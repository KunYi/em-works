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
//------------------------------------------------------------------------------
//
//  File:  halsurf.cpp
//
#include "precomp.h"

//------------------------------------------------------------------------------

DWORD WINAPI HalLock(DDHAL_LOCKDATA *pd)
{
    DWORD       dwAddr;
    DDGPE       *pDDGPE = GetDDGPE();
    DDGPESurf   *pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);

    DEBUGMSG(GPE_ZONE_ENTER, (L"+HAlLock: %08X\r\n", pd->dwFlags));

    if (pd->dwFlags & DDLOCK_WAITNOTBUSY)
    {
        while (pDDGPE->SurfaceBusyFlipping(pSurf)) Sleep(0);
        pDDGPE->WaitForNotBusy();
    } 
    else if (pDDGPE->SurfaceBusyFlipping(pSurf) || pDDGPE->IsBusy())
    {
        pd->ddRVal = DDERR_WASSTILLDRAWING;
        goto cleanUp;
    }

    dwAddr = (DWORD)pSurf->Buffer();
    if (pd->bHasRect)
    {
        //dwAddr = (DWORD)pSurf->GetPtr(pd->rArea.left, pd->rArea.top);
        dwAddr += pd->rArea.top * pSurf->Stride();
        dwAddr += (pd->rArea.left * pSurf->Bpp()) >> 3;
    }

    pd->lpSurfData = (PVOID)dwAddr;
    pd->ddRVal = DD_OK;

cleanUp:
    
    DEBUGMSG(GPE_ZONE_ENTER, (L"-HAlLock: %08X\r\n", pd->ddRVal));
    return DDHAL_DRIVER_HANDLED;
}

//------------------------------------------------------------------------------

DWORD WINAPI HalUnlock(DDHAL_UNLOCKDATA *pd)
{
    DDGPESurf       *pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);
    OMAP2420Surf    *pOMAP2420Surf = static_cast<OMAP2420Surf*>(pSurf);

    DEBUGMSG(GPE_ZONE_ENTER, (L"+HAlUnlock\r\n"));

    // flush the surface memory from cache
    pOMAP2420Surf->WriteBack();
    pd->ddRVal = DD_OK;

    DEBUGMSG(GPE_ZONE_ENTER, (L"-HAlUnlock\r\n"));
    return DDHAL_DRIVER_HANDLED;
}

//------------------------------------------------------------------------------

DWORD WINAPI HalFlip(DDHAL_FLIPDATA *pd)
{
    DEBUGENTER(HalFlip);
    DDGPE* pDDGPE = GetDDGPE();
    DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpSurfTarg);

    // Wait for engine...
    if ((pd->dwFlags & DDFLIP_WAITNOTBUSY) != 0) {
        while (pDDGPE->SurfaceBusyFlipping(pSurf)) Sleep(0);
        pDDGPE->WaitForNotBusy();
    }
    else if (pDDGPE->SurfaceBusyFlipping(pSurf) || pDDGPE->IsBusy()) {
        pd->ddRVal = DDERR_WASSTILLDRAWING;
        goto cleanUp;
    }

    // Flip
    DDGPEFlip(pd);

cleanUp:
    DEBUGLEAVE(HalFlip);
    return DDHAL_DRIVER_HANDLED;
}

//------------------------------------------------------------------------------

DWORD WINAPI HalGetBltStatus(DDHAL_GETBLTSTATUSDATA *pd)
{
    DDGPE       *pDDGPE = GetDDGPE();
    DDGPESurf   *pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);

    DEBUGMSG(GPE_ZONE_ENTER, (L"+HalGetBltStatus\r\n"));

    pd->ddRVal = DD_OK;
    if (pd->dwFlags & DDGBS_CANBLT)
    {
        if (pDDGPE->SurfaceBusyFlipping(pSurf) || pDDGPE->IsBusy())
        {
            pd->ddRVal = DDERR_WASSTILLDRAWING;
        }            
    }
    else if (pd->dwFlags & DDGBS_ISBLTDONE)
    {
        if (pDDGPE->IsBusy())
        {
            pd->ddRVal = DDERR_WASSTILLDRAWING;
        }            
    }

    DEBUGMSG(GPE_ZONE_ENTER, (L"-HalGetBltStatus\r\n"));
    return DDHAL_DRIVER_HANDLED;
}

//------------------------------------------------------------------------------

DWORD WINAPI HalGetFlipStatus( LPDDHAL_GETFLIPSTATUSDATA pd )
{
    DEBUGENTER(HalGetFlipStatus);
    DDGPE* pDDGPE = GetDDGPE();
    DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);

    if (pDDGPE->SurfaceBusyFlipping(pSurf))
        {
        pd->ddRVal = DDERR_WASSTILLDRAWING;
        }
    else 
        {
        pd->ddRVal = DD_OK;
        }

    DEBUGLEAVE(HalGetFlipStatus);
    return DDHAL_DRIVER_HANDLED;
}

//------------------------------------------------------------------------------

DWORD WINAPI HalUpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA pUpdateOverlay)
{
    return ((OMAP2420GPE *)GetDDGPE())->UpdateOverlay(pUpdateOverlay);
}

//------------------------------------------------------------------------------

DWORD WINAPI HalSetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA pSetPos)
{
    return ((OMAP2420GPE *)GetDDGPE())->SetOverlayPosition(pSetPos);
}
