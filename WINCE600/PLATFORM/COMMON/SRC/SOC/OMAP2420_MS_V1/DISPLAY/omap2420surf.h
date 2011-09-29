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
//  File:  omap2420surf.h
//
#ifndef __OMAP2420SURF_H
#define __OMAP2420SURF_H

//------------------------------------------------------------------------------

class OMAP2420Surf : public DDGPESurf
{

private:
    HANDLE m_hSurface;
    UCHAR *m_pSurface;

    // Surface requires discard on next DMA bitblit
    BOOL   m_inCache;

public:
    OMAP2420_VID_REGS   *m_pVidRegs;

    OMAP2420Surf(
        int,
        int,
        DWORD,
        VOID*,
        int,
        EGPEFormat
        );
    
    virtual
    ~OMAP2420Surf(
        );

    VOID
    WriteBack(
        );

    VOID
    Discard(
        );
    
    BOOL
    SurfaceOk(
        );
    
    VOID
    InCache(
        );
};

//------------------------------------------------------------------------------

#endif // OMAP2420Surf
