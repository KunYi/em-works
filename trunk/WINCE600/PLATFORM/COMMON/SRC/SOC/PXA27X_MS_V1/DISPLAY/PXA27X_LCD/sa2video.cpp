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
/*
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.
** Title to the Material remains with Intel Corporation or its suppliers and licensors.
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
*/

#include <windows.h>
#include <types.h>
#include <nkintr.h>
#include <syspal.h>
#include <ceddk.h>
#include "memdefs.h"

#include "precomp.h"
#include "cursor.h"
#include "xllp_lcd.h"
#include "xllp_gpio.h"
#include "strsafe.h"

extern "C" PVOID VirtualAllocCopy(unsigned size,char *str,PVOID pVirtualAddress);
extern "C" void DispDrvrSetPalette(const PALETTEENTRY *,USHORT,USHORT);
extern "C" void *gFrameBuffer;
extern "C" volatile LCD_PALETTE *v_pPaletteBuffer;
extern "C" BOOL bDoRotation;
extern "C" void ScrollBuffer(int direction);
extern "C" void DispDrvrDirtyRectDump2(LPCRECT prc,DWORD color);
extern "C" void DispDrvrDirtyRectDump_rectfill(LPCRECT prc,DWORD color);
extern "C" BOOL gDrawCursorFlag;
extern "C" XLLP_LCD_T XllpLCD;
extern "C" volatile LCD_FRAME_DESCRIPTOR    *frameDescriptorCh2_YCbCr_Y;
extern "C" volatile LCD_FRAME_DESCRIPTOR    *frameDescriptorCh3_YCbCr_Cb;
extern "C" volatile LCD_FRAME_DESCRIPTOR    *frameDescriptorCh4_YCbCr_Cr;
extern "C" volatile unsigned int            *pOSCR;
extern "C" void ApplicationImageProcessingHandler(void);

extern "C" HANDLE hIntEventKnown;
extern "C" HANDLE hIntEvent;

extern "C" BOOL g_fDisableRotation;       //Enable/disable screen rotation

volatile LCDRegs     * v_pLcdRegs = NULL;
volatile XLLP_GPIO_T * v_pGPIORegs = NULL;

BOOL bSuspended = FALSE;
extern HMODULE g_hmodDisplayDll;
    
#define NUM_FRAME_BUFFERS 2

POWER_CAPABILITIES DisplayDrvPowerCaps =
{
     // DeviceDx:    Supported power states
    DX_MASK(D0) | DX_MASK(D2) | DX_MASK(D3) | DX_MASK(D4),

    // WakeFromDx
    0,

    // InrushDx:    No inrush of power
    0,

    // Power: Maximum milliwatts in each state
    {
        0x00000000, // D0
        0x00000000, // D1
        0x00000000, // D2
        0x00000000, // D3
        0x00000000  // D4
    },

    // Latency
    {
        0x00000000, // D0
        0x00000000, // D1
        0x00000000, // D2
        0x00000000, // D3
        0x00000000  // D4
    },

    // Flags: None
    0
};

DDGPE * pGPE = NULL;

// These masks are used to extract the color component for a 16 bit pixel value.  5 bits red, 6 bits green, 5 bits blue.
ulong BitMasks[] = {0xF800, 0x07E0, 0x001F};

#ifdef DEBUG
#define ZONE_PM               DEBUGZONE(14)
#endif

INSTANTIATE_GPE_ZONES(0x4003,"GDI Driver","Power Management","unused2")     /* Start with Errors, warnings, and pm */

BOOL APIENTRY
GPEEnableDriver(
    ULONG          iEngineVersion,
    ULONG          cj,
    DRVENABLEDATA *pded,
    PENGCALLBACKS  pEngCallbacks
    );

BOOL APIENTRY
DrvEnableDriver(
    ULONG          iEngineVersion,
    ULONG          cj,
    DRVENABLEDATA *data,
    PENGCALLBACKS  pEngineCallbacks
    )
{
    return GPEEnableDriver(iEngineVersion,cj,data,pEngineCallbacks);
}

#define DMA_CHANNEL_0  _T("DMA_CHANNEL_0")

////////////////////////////////////////////////////////////////////////////////////////////

// Main entry point for a GPE-compliant driver

GPE *
GetGPE()
{
    if (!pGPE)
    {
        pGPE = new SA2Video();
    }

    return pGPE;
}

VOID
SA2Video::PowerHandler(
    BOOL bOff
    )
{
    // turn off the display if it is not already turned off
    // (turning on is controlled by SA2Video::SetPmPowerState)
    if (bOff && !bSuspended) {
        DEBUGMSG(ZONE_PM, (TEXT("SA2Video::PowerHandler: TurnOff Display\r\n")));
        DispDrvrPowerHandler(TRUE);
        bSuspended = TRUE;
    }
}

SA2Video::SA2Video()
{
    DEBUGMSG(GPE_ZONE_INIT,(TEXT("SA2Video::SA2Video\r\n")));

    m_InDDraw = FALSE;

    // Determine the display type
    // Setup the LCD controller for that display
    // Power up and enable the display
    DispDrvrInitialize();

    // Advertise the power management interface for this driver.
    AdvertisePowerInterface();

    m_ModeInfo.modeId = 0;
    m_ModeInfo.width = m_nScreenWidth = DispDrvr_cxScreen;
    m_ModeInfo.height = m_nScreenHeight = DispDrvr_cyScreen;
    m_ModeInfo.Bpp = bpp;
    m_ModeInfo.frequency = 60;
    m_ModeInfo.format = bpp == 8 ? gpe8Bpp : gpe16Bpp;
    m_pMode = &m_ModeInfo;

    m_pModeEx.modeInfo.modeId = 0;
    m_pModeEx.modeInfo.width = m_nScreenWidth = DispDrvr_cxScreen;
    m_pModeEx.modeInfo.height = m_nScreenHeight = DispDrvr_cyScreen;
    m_pModeEx.modeInfo.Bpp = bpp;
    m_pModeEx.modeInfo.frequency = 60;
    m_pModeEx.modeInfo.format = bpp == 8 ? gpe8Bpp : gpe16Bpp;

    if (!g_fDisableRotation)
    {
        m_iRotate = GetRotateModeFromReg();
    }
    else
    {
        m_iRotate = DMDO_0;
    }

    SetRotateParams();

    m_pVirtualFrameBuffer = gFrameBuffer;

    m_pPrimarySurface = new DDGPESurf(m_nScreenWidth,m_nScreenHeight,m_pVirtualFrameBuffer,
         DispDrvr_cdwStride,m_ModeInfo.format);

    m_VirtualFrameBuffer = (ULONG)FRAME_BUFFER_0_BASE_VIRTUAL;

    if (m_pPrimarySurface)
    {
        ((GPESurf *)m_pPrimarySurface)->SetRotation(m_nScreenWidth, m_nScreenHeight, m_iRotate);
    }

    // Set up the software cursor state.
    m_CursorVisible   = FALSE;
    m_CursorDisabled  = TRUE;
    m_CursorForcedOff = FALSE;
}

SCODE
SA2Video::SetMode(
    int        modeId,
    HPALETTE * pPalette)
{
    SCODE Result = E_INVALIDARG;

    // Here, we use EngCreatePalette to create a palette that that MGDI will use as a
    // stock palette
    if(pPalette && 0 == modeId)
    {
        switch (bpp)
        {
        case 8:
            *pPalette = EngCreatePalette(PAL_INDEXED, PALETTE_SIZE, (ULONG*)_rgbIdentity, 0, 0, 0);
            Result    = S_OK;
            break;

        case 16:
            *pPalette = EngCreatePalette(PAL_BITFIELDS, 0, NULL, BitMasks[0], BitMasks[1], BitMasks[2]);
            Result    = S_OK;
            break;

        default:
            *pPalette = NULL;
            break;
        }
    }

    return Result;
}

SCODE
SA2Video::GetModeInfo(
    GPEMode * pMode,
    int       modeNumber
    )
{
    if(modeNumber != 0)
    {
        return E_INVALIDARG;
    }

    *pMode = m_ModeInfo;

    return S_OK;
}

int
SA2Video::NumModes()
{
    return 1;
}

SCODE
SA2Video::SetPointerShape(
    GPESurf * pMask,
    GPESurf * pColorSurf,
    int       xHot,
    int       yHot,
    int       cX,
    int       cY
    )
{
    int        row;
    int        colByte;
    BYTE     * pAND;
    BYTE     * pXOR;
    BYTE       bitMask;
    unsigned   i;

    if (!bDoRotation)
    {
        CursorOff();
    }

    if (!pMask)
    {   // Turn off the cursor.
        memset((BYTE *)gCursorMask,0xFF,sizeof(gCursorMask));
        memset((BYTE *)gCursorData,0x00,sizeof(gCursorData));

        m_CursorDisabled = TRUE;
    }
    else
    {
        i = 0;

        for (row = 0; row < cY; row++)
        {
            for (colByte = 0; colByte < (cX / 8); colByte++)
            {
                pAND = (unsigned char *)pMask->Buffer()+(row*pMask->Stride())+colByte;
                pXOR = pAND+(cY*pMask->Stride());

                for (bitMask = 0x0080; bitMask; bitMask >>= 1)
                {
                    gCursorMask[i] = (*pAND & bitMask) ? 0xFFFF : 0x0000;
                    gCursorData[i] = (*pXOR & bitMask) ? 0xFFFF : 0x0000;
                    i++;
                }
            }
        }

        m_CursorDisabled = FALSE;
        m_CursorSize.x   = cX;
        m_CursorSize.y   = cY;
        gxHot            = xHot;
        gyHot            = yHot;
    }

    return S_OK;
}

SCODE
SA2Video::MovePointer(
    int xPosition,
    int yPosition
    )
{
    if (!bDoRotation)
    {
        CursorOff();
    }

    if(xPosition != -1 || yPosition != -1)
    {
        // enable cursor
        DispDrvrMoveCursor(xPosition, yPosition);

        m_CursorRect.left   = xPosition - gxHot;
        m_CursorRect.right  = m_CursorRect.left + m_CursorSize.x;
        m_CursorRect.top    = yPosition - gyHot;
        m_CursorRect.bottom = m_CursorRect.top + m_CursorSize.y;

        if (!bDoRotation)
        {
            CursorOn();
        }
    }
    else
    {
        // disable cursor
        DispDrvrMoveCursor(DispDrvr_cxScreen, DispDrvr_cyScreen);
    }

    return S_OK;
}

void
SA2Video::WaitForNotBusy()
{
    return;
}

int
SA2Video::IsBusy()
{
    return 0;    // Never busy as there is no acceleration
}

void
SA2Video::GetPhysicalVideoMemory(
    unsigned long * pPhysicalMemoryBase,
    unsigned long * pVideoMemorySize
    )
{
    *pPhysicalMemoryBase = (ULONG)FRAME_BUFFER_0_BASE_VIRTUAL;
    *pVideoMemorySize    = DispDrvr_cdwStride * DispDrvr_cyScreen;
}

void
SA2Video::GetVirtualVideoMemory(
    unsigned long * pVirtualMemoryBase,
    unsigned long * pVideoMemorySize
    )
{
    *pVirtualMemoryBase = (unsigned)(m_pPrimarySurface->Buffer());
    *pVideoMemorySize   = DispDrvr_cdwStride * DispDrvr_cyScreen;
}

SCODE
SA2Video::AllocSurface(
    DDGPESurf         ** ppSurf,
    int                  width,
    int                  height,
    EGPEFormat           format,
    EDDGPEPixelFormat    pixelFormat,
    int                  surfaceFlags
    )
{
    if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
    {
        *ppSurf = NULL;

        return E_OUTOFMEMORY;
    }

    DWORD SurfBpp       = EGPEFormatToBpp[format];
    DWORD stride        = ((SurfBpp * width + 31) >> 5) << 2;
    DWORD nSurfaceBytes = stride * height;

    *ppSurf = new DDGPESurf(width, height, stride, format, pixelFormat);

    if (NULL != *ppSurf)
    {
        if (((*ppSurf)->Buffer()) == NULL)
        {
            delete *ppSurf;
            *ppSurf = NULL;
        }
        else
        {
            return S_OK;
        }
    }

    return E_OUTOFMEMORY;
}

SCODE
SA2Video::AllocSurface(
    GPESurf    ** surface,
    int           width,
    int           height,
    EGPEFormat    format,
    int           surfaceFlags
    )
{
    if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
    {
        // Can't allocate video-memory surfaces in the SA2Video environment
        return E_OUTOFMEMORY;
    }

    // Allocate from system memory
    *surface = new GPESurf(width,height,format);

    if (*surface != NULL)
    {
        // Check that the bits were allocated succesfully
        if (((*surface)->Buffer()) == NULL)
        {
            delete *surface;
        }
        else
        {
            return S_OK;
        }
    }

    return E_OUTOFMEMORY;
}

SCODE
SA2Video::WrappedEmulatedLine(
    GPELineParms *pParms
    )
{
    SCODE retval;
    RECT  bounds;
    int   N_plus_1;  // Minor length of bounding rect + 1

    // calculate the bounding-rect to determine overlap with cursor
    if (pParms->dN)   // The line has a diagonal component (we'll refresh the bounding rect)
    {
        N_plus_1 = 2 + ((pParms->cPels * pParms->dN) / pParms->dM);
    }
    else
    {
        N_plus_1 = 1;
    }

    switch(pParms->iDir)
    {
        case 0:
            bounds.left   = pParms->xStart;
            bounds.top    = pParms->yStart;
            bounds.right  = pParms->xStart + pParms->cPels + 1;
            bounds.bottom = bounds.top + N_plus_1;
            break;

        case 1:
            bounds.left   = pParms->xStart;
            bounds.top    = pParms->yStart;
            bounds.bottom = pParms->yStart + pParms->cPels + 1;
            bounds.right  = bounds.left + N_plus_1;
            break;

        case 2:
            bounds.right  = pParms->xStart + 1;
            bounds.top    = pParms->yStart;
            bounds.bottom = pParms->yStart + pParms->cPels + 1;
            bounds.left   = bounds.right - N_plus_1;
            break;

        case 3:
            bounds.right  = pParms->xStart + 1;
            bounds.top    = pParms->yStart;
            bounds.left   = pParms->xStart - pParms->cPels;
            bounds.bottom = bounds.top + N_plus_1;
            break;

        case 4:
            bounds.right  = pParms->xStart + 1;
            bounds.bottom = pParms->yStart + 1;
            bounds.left   = pParms->xStart - pParms->cPels;
            bounds.top    = bounds.bottom - N_plus_1;
            break;

        case 5:
            bounds.right  = pParms->xStart + 1;
            bounds.bottom = pParms->yStart + 1;
            bounds.top    = pParms->yStart - pParms->cPels;
            bounds.left   = bounds.right - N_plus_1;
            break;

        case 6:
            bounds.left   = pParms->xStart;
            bounds.bottom = pParms->yStart + 1;
            bounds.top    = pParms->yStart - pParms->cPels;
            bounds.right  = bounds.left + N_plus_1;
            break;

        case 7:
            bounds.left   = pParms->xStart;
            bounds.bottom = pParms->yStart + 1;
            bounds.right  = pParms->xStart + pParms->cPels + 1;
            bounds.top    = bounds.bottom - N_plus_1;
            break;

        default:
            DEBUGMSG(GPE_ZONE_ERROR,(TEXT("Invalid direction: %d\r\n"), pParms->iDir));
            return E_INVALIDARG;
    }

    // check for line overlap with cursor and turn off cursor if overlaps
    if (!bDoRotation)
    {
        if (m_CursorVisible && !m_CursorDisabled)
        {
            RotateRectl(&m_CursorRect);

            if (m_CursorRect.top < bounds.bottom && m_CursorRect.bottom > bounds.top &&
                m_CursorRect.left < bounds.right && m_CursorRect.right > bounds.left)
            {
                RotateRectlBack(&m_CursorRect);
                CursorOff();
                m_CursorForcedOff = TRUE;
            }
            else
            {
                RotateRectlBack(&m_CursorRect);
            }
        }
    }

    // do emulated line
    retval = EmulatedLine(pParms);

    // see if cursor was forced off because of overlap with line bouneds and turn back on.
    if (!bDoRotation)
    {
        if (m_CursorForcedOff)
        {
            m_CursorForcedOff = FALSE;
            CursorOn();
        }
    }

    return retval;
}

#undef SWAP
#define SWAP(a,b,type) { type tmp=a; a=b; b=tmp; }

int
EmulatedBltFill16_C(
    WORD         * pDst,
    WORD           color,
    int            width,
    int            height,
    unsigned int   step
    )
{
    step >>= 1; //in 16-bit
    step -= width;

    for (int r = 0; r < height; r++)
    {
        for (int w = 0; w < width; w++)
        {
            *pDst++ = color;
        }

        pDst += step;
    }

    return 0;
}
extern "C" SCODE EmulatedBltFill16ASM(WORD* pDst, WORD color, int width, int height, unsigned int step);

SCODE EmulatedBltSrcCopy0416( GPEBltParms *pParms );
SCODE EmulatedBltSrcCopy0116( GPEBltParms *pParms );
SCODE EmulatedBltSrcCopy0808( GPEBltParms *pParms );
SCODE EmulatedBltSrcCopy0408( GPEBltParms *pParms );
SCODE EmulatedBltSrcCopy0108( GPEBltParms *pParms );

__inline
SCODE
EmulatedBltFill16_Eml(
    GPEBltParms * pParms
    )
{
    DEBUGMSG(GPE_ZONE_BLT_HI,(TEXT("EmulatedBltFill16\r\n")));

    UINT32   iDstScanStride = pParms->pDst->Stride();
    BYTE   * pbDibBitsDst   = (BYTE *)pParms->pDst->Buffer();
    PRECTL   prcDst         = pParms->prclDst;
    DWORD    dwOnColorIndex = pParms->solidColor;
    int      iNumDstRows    = prcDst->bottom - prcDst->top;
    int      iNumDstCols    = prcDst->right  - prcDst->left;

    // Compute pointer to the starting rows in the dst bitmap
    WORD *pwDstScanLine = (WORD *)(pbDibBitsDst + prcDst->top * iDstScanStride + prcDst->left * 2);
    WORD  wColor = (WORD)dwOnColorIndex;

    EmulatedBltFill16ASM(pwDstScanLine, wColor, iNumDstCols, iNumDstRows, iDstScanStride);
    return S_OK;
}


#define BLOCK_OP(pbDst, pbSrc, cBytes)  memmove(pbDst, pbSrc, cBytes)
#define SCRCOPY_ASM

extern "C" void EmulatedBltSrcCopy1616ASM(WORD* pwScanLineSrc, WORD *pwScanLineDst, int width, int height, UINT32 iScanStrideSrc, UINT32 iScanStrideDst);

#define _MM_OPT_ASM
#ifdef _MM_OPT_ASM
extern "C"
{
    void Memmove1616_ASM_LE16(WORD* pSrc, WORD *pDst, int len) ;
    void Memmove1616_ASM_GT16(WORD* pSrc, WORD *pDst, int len) ;
}

#else
#include "ebcopy1616.h"
#endif

SCODE
EmulatedBltSrcCopy1616_Eml(
    GPEBltParms *pParms
    )
{
    // Source-related info
    PRECTL   prcSrc         = pParms->prclSrc;
    UINT32   iScanStrideSrc = pParms->pSrc->Stride()/sizeof(WORD);
    WORD   * pwScanLineSrc  = (WORD *)pParms->pSrc->Buffer() +
                              prcSrc->top * iScanStrideSrc      +
                              prcSrc->left;

    // Dest-related info
    PRECTL   prcDst         = pParms->prclDst;
    UINT32   iScanStrideDst = pParms->pDst->Stride()/sizeof(WORD);
    WORD   * pwScanLineDst  = (WORD *)pParms->pDst->Buffer() +
                              prcDst->top * iScanStrideDst      +
                              prcDst->left;

    int cRows = prcDst->bottom - prcDst->top;
    int cCols = prcDst->right  - prcDst->left;

    // Make sure to copy source before overwriting.
    if (!pParms->yPositive)
    {
        // Scan from end of memory, and negate stride
        pwScanLineSrc += iScanStrideSrc * (cRows - 1);
        pwScanLineDst += iScanStrideDst * (cRows - 1);

        iScanStrideSrc = (UINT32)-(INT32)iScanStrideSrc;
        iScanStrideDst = (UINT32)-(INT32)iScanStrideDst;
    }

#ifdef    SCRCOPY_ASM

    EmulatedBltSrcCopy1616ASM(pwScanLineSrc, pwScanLineDst, cCols, cRows,
                            iScanStrideSrc, iScanStrideDst);

#else
    //
    // Do the copy line by line.
    //
    //EmulatedBltSrcCopy1616_CO(pwScanLineSrc, pwScanLineDst, cCols, cRows,
    //                         iScanStrideSrc, iScanStrideDst, xPositive);


    for (int row = 0; row < cRows; row++)
    {
        //BLOCK_OP(pwScanLineDst, pwScanLineSrc, cCols*sizeof(WORD));
        if ( cCols < 8 ) {
#ifndef _MM_OPT_ASM
            Memmove1616_C_SIMPLE(pwScanLineSrc, pwScanLineDst, cCols);
#else
            Memmove1616_ASM_LE16(pwScanLineSrc, pwScanLineDst, cCols);
#endif
        } else {
#ifndef _MM_OPT_ASM
            Memmove1616_C(pwScanLineSrc, pwScanLineDst, cCols);
#else
            Memmove1616_ASM_GT16(pwScanLineSrc, pwScanLineDst, cCols);
#endif
        }

        pwScanLineSrc += iScanStrideSrc;
        pwScanLineDst += iScanStrideDst;
    }

#endif
        return S_OK;
}
#define SHIFT_ONE_MASK 0x7BEF
#define SHIFT_TWO_MASK 0x39E7

extern "C" void EmulatedBltText16ASM(BYTE* pSrcMask, WORD* pDstPixel, WORD color, int width, int height, unsigned int stepMask, unsigned int stepDst, int OffsetSrc);

__inline
SCODE
EmulatedBltAlphaText16_Eml(
    GPEBltParms *pParms
    )
{
    DEBUGMSG(GPE_ZONE_BLT_HI,(TEXT("EmulatedBltAlphaText16\r\n")));

    UINT32   iDstScanStride  = pParms->pDst->Stride();
    BYTE   * pDibBitsDst     = (BYTE *)pParms->pDst->Buffer();
    UINT32   iSrcScanStride  = pParms->pMask->Stride();
    BYTE   * pDibBitsSrc     = (BYTE *)pParms->pMask->Buffer();
    PRECTL   prcSrc          = pParms->prclMask;
    PRECTL   prcDst          = pParms->prclDst;
    WORD     wOnColor[5];
    int      iNumDstRows;
    int      iNumDstCols;
    BYTE   * pbSrcScanLine;
    BYTE   * pbDstScanLine;
    BYTE   * pbSrc;
    WORD   * pwDstPixel;
    BOOL     bOdd;
    BYTE     bSrc;

    // Caller assures a well-ordered, non-empty rect
    // compute size of destination rect
    iNumDstCols = prcDst->right  - prcDst->left;
    iNumDstRows = prcDst->bottom - prcDst->top;

    // compute pointers to the starting rows in the src and dst bitmaps
    pbSrcScanLine = pDibBitsSrc + prcSrc->top * iSrcScanStride + (prcSrc->left >> 1);
    pbDstScanLine = pDibBitsDst + prcDst->top * iDstScanStride + prcDst->left * 2;

    // Create pixel values with 0/4, 1/4, 2/4, 3/4 and 4/4 of the solid brush color
    wOnColor[0] = 0;
    wOnColor[4] = (WORD)pParms->solidColor;
    wOnColor[2] = (wOnColor[4] >> 1 ) & SHIFT_ONE_MASK;
    wOnColor[1] = (wOnColor[4] >> 2 ) & SHIFT_TWO_MASK;
    wOnColor[3] = wOnColor[1] + wOnColor[2];


    for (int i = 0; i < iNumDstRows; i++)
    {
        // set up pointers to first bytes on src and dst scanlines
        pbSrc      = pbSrcScanLine;
        pwDstPixel = (WORD *)pbDstScanLine;
        bOdd       = prcSrc->left & 1;

        for (int j = 0; j < iNumDstCols; j++ )
        {
            if (bOdd)
            {
                bSrc = *pbSrc++ & 0xF;
            }
            else
            {
                bSrc = *pbSrc >> 4;
            }

            bOdd = !bOdd;

            switch ( (bSrc + 1) >> 2 )  // src pixel in range 0...4
            {
            case 0:
                //      Leave destination untouched
                break;
            case 1:
                //      3/4 destination color and 1/4 brush color
                *pwDstPixel = ((*pwDstPixel >> 2) & SHIFT_TWO_MASK)
                            + ((*pwDstPixel >> 1) & SHIFT_ONE_MASK) + wOnColor[1];
                break;
            case 2:
                //      1/2 destination color and 1/2 brush color
                *pwDstPixel = ((*pwDstPixel >> 1) & SHIFT_ONE_MASK) + wOnColor[2];
                break;
            case 3:
                //      1/4 destination color and 3/4 brush color
                *pwDstPixel = ((*pwDstPixel >> 2) & SHIFT_TWO_MASK) + wOnColor[3];
                break;
            case 4:
                //      Fill with solid brush color
                *pwDstPixel = wOnColor[4];
                break;
            default:
                DebugBreak();
            }

            pwDstPixel++;
        }

        // advance to next scanline
        pbSrcScanLine += iSrcScanStride;
        pbDstScanLine += iDstScanStride;
    }

    return S_OK;

}

__inline
SCODE
EmulatedBltText16_Eml(
    GPEBltParms * pParms
    )
{
    DEBUGMSG(GPE_ZONE_BLT_HI,(TEXT("EmulatedBltText16\r\n")));

    UINT32   iDstScanStride = pParms->pDst->Stride();
    BYTE   * pDibBitsDst    = (BYTE *)pParms->pDst->Buffer();
    UINT32   iSrcScanStride = pParms->pMask->Stride();
    BYTE   * pDibBitsSrc    = (BYTE *)pParms->pMask->Buffer();
    PRECTL   prcSrc         = pParms->prclMask;
    PRECTL   prcDst         = pParms->prclDst;

    unsigned int wOnColor = (unsigned int)pParms->solidColor;

    int    iSrcBitOffset;
    int    iNumDstRows;
    int    iNumDstCols;
    BYTE * pbSrcScanLine;
    BYTE * pbDstScanLine;
    WORD * pwDstPixel;

    // Caller assures a well-ordered, non-empty rect
    // compute size of destination rect
    iNumDstCols = prcDst->right  - prcDst->left;
    iNumDstRows = prcDst->bottom - prcDst->top;

    // compute pointers to the starting rows in the src and dst bitmaps
    pbSrcScanLine = pDibBitsSrc + prcSrc->top * iSrcScanStride + (prcSrc->left >> 3);
    iSrcBitOffset = prcSrc->left & 0x07;
    pbDstScanLine = pDibBitsDst + prcDst->top * iDstScanStride + prcDst->left * 2;

    pwDstPixel = (WORD *)pbDstScanLine;

    EmulatedBltText16ASM(pbSrcScanLine,pwDstPixel, wOnColor, iNumDstCols, iNumDstRows, iSrcScanStride, iDstScanStride, iSrcBitOffset);

    return S_OK;

}

SCODE
SA2Video::Line(
    GPELineParms * pLineParms,
    EGPEPhase      phase
    )
{
    if(phase == gpeSingle || phase == gpePrepare)
    {
        if(pLineParms->pDst != m_pPrimarySurface)
        {
            pLineParms->pLine = &GPE::EmulatedLine;
        }
        else
        {
            pLineParms->pLine = (SCODE (GPE::*)(struct GPELineParms *))&SA2Video::WrappedEmulatedLine;
        }
    }

    return S_OK;
}

#undef SWAP
#define SWAP(type,a,b) { type tmp; tmp=a; a=b; b=tmp; }

SCODE
SA2Video::WrappedEmulatedBlt(
    GPEBltParms * pParms
    )
{
    SCODE code;
    RECT  bounds;

    // This function should only be called when using a virtual primary (ie.
    // bDoRotation is TRUE).
    ASSERT(bDoRotation);

    if ((pParms->pDst == m_pPrimarySurface && m_iRotate) || (pParms->pSrc == m_pPrimarySurface && m_iRotate))
    {
        code = EmulatedBltRotate(pParms);
    }
    else
    {
        if ( !( (pParms->bltFlags & (BLT_ALPHABLEND | BLT_TRANSPARENT | BLT_STRETCH)) || (pParms->pLookup) || (pParms->pConvert) )
             && pParms->pDst->Format() == gpe16Bpp )
        {
            if ( pParms->rop4 == 0xcccc )
            {
                if ( EGPEFormatToBpp[pParms->pSrc->Format()] == 16 )
                {
                    code = EmulatedBltSrcCopy1616_Eml(pParms);
                    goto contd;
                }
            }
            else if ( pParms->rop4 == 0xf0f0 )
            {
                if (pParms->solidColor != -1)
                {    // must be a solid colored brush
                     code = EmulatedBltFill16_Eml(pParms);
                     goto contd;
                }
            }
            else if ( pParms->rop4 == 0xaaf0 )
            {
                if (pParms->solidColor != -1)
                {
                    if (pParms->pMask->Format() == gpe1Bpp)
                    {
                        code = EmulatedBltText16_Eml(pParms);
                        goto contd;
                    }
                    else if (pParms->pMask->Format() == gpe4Bpp)
                    {
                        code = EmulatedBltAlphaText16_Eml(pParms);
                        goto contd;
                    }
                }
            }
        }

        code = EmulatedBlt(pParms);
    }

contd:

    if (bDoRotation)
    {
        bounds.left   = pParms->prclDst->left;
        bounds.top    = pParms->prclDst->top;
        bounds.right  = pParms->prclDst->right;
        bounds.bottom = pParms->prclDst->bottom;

        if(bounds.left > bounds.right)
        {
            SWAP(int,bounds.left,bounds.right)
        }

        if( bounds.top > bounds.bottom)
        {
            SWAP(int,bounds.top,bounds.bottom)
        }

        if (bounds.top == bounds.bottom-1)
        {
            if ( !( (pParms->bltFlags & (BLT_ALPHABLEND | BLT_TRANSPARENT | BLT_STRETCH)) || (pParms->pLookup) || (pParms->pConvert) ) )
            {
                if ( pParms->rop4 == 0xf0f0 )
                {
                    if ( EGPEFormatToBpp[pParms->pDst->Format()] == 16 )
                    {
                        if (pParms->solidColor != -1)
                        {    // must be a solid colored brush
                            if (!gDrawCursorFlag)
                            {
                                DispDrvrDirtyRectDump2((LPRECT)&bounds,pParms->solidColor);
                                return S_OK;
                            }
                        }
                    }
                }
            }
        }

        if ( !( (pParms->bltFlags & (BLT_ALPHABLEND | BLT_TRANSPARENT | BLT_STRETCH)) || (pParms->pLookup) || (pParms->pConvert) ) )
        {
            if ( pParms->rop4 == 0xf0f0 )
            {
                if ( EGPEFormatToBpp[pParms->pDst->Format()] == 16 )
                {
                    if (pParms->solidColor != -1)
                    {    // must be a solid colored brush
                        DispDrvrDirtyRectDump_rectfill((LPRECT)&bounds,pParms->solidColor);
                        return S_OK;
                    }
                }
            }
        }

        if(FAILED(code))
        {
            return code;
        }

        DispDrvrDirtyRectDump((LPRECT)&bounds);
    }

    return code;
}

SCODE
SA2Video::BltPrepare(
    GPEBltParms * pParms
    )
{
    DEBUGMSG(GPE_ZONE_LINE,(TEXT("SA2Video::BltPrepare\r\n")));

    RECTL rectl;

    pParms->pBlt = &GPE::EmulatedBlt;

    // Check to see if the software cursor should be disabled.
    if (!bDoRotation)
    {
        if (pParms->pDst == m_pPrimarySurface)  // only care if dest is main display surface
        {
            if (m_CursorVisible && !m_CursorDisabled)
            {
                if (pParms->prclDst != NULL)        // make sure there is a valid prclDst
                {
                    rectl = *pParms->prclDst;       // if so, use it

                    // There is no guarantee of a well ordered rect in pParms
                    // due to flipping and mirroring.
                    if(rectl.top > rectl.bottom)
                    {
                        int iSwapTmp     = rectl.top;
                        rectl.top    = rectl.bottom;
                        rectl.bottom = iSwapTmp;
                    }

                    if(rectl.left > rectl.right)
                    {
                        int iSwapTmp    = rectl.left;
                        rectl.left  = rectl.right;
                        rectl.right = iSwapTmp;
                    }
                }
                else
                {
                    rectl = m_CursorRect;                   // if not, use the Cursor rect - this forces the cursor to be turned off in this case.
                }

                if (m_CursorRect.top <= rectl.bottom && m_CursorRect.bottom >= rectl.top &&
                    m_CursorRect.left <= rectl.right && m_CursorRect.right >= rectl.left)
                {
                    CursorOff();
                    m_CursorForcedOff = TRUE;
                }
            }
        }

        // check for source overlap with cursor and turn off cursor if overlaps
        if (pParms->pSrc == m_pPrimarySurface)  // only care if source is main display surface
        {
            if (m_CursorVisible && !m_CursorDisabled)
            {
                if (pParms->prclSrc != NULL)        // make sure there is a valid prclSrc
                {
                    rectl = *pParms->prclSrc;       // if so, use it
                }
                else
                {
                    rectl = m_CursorRect;                   // if not, use the Cursor rect - this forces the cursor to be turned off in this case.
                }

                if (m_CursorRect.top < rectl.bottom && m_CursorRect.bottom > rectl.top &&
                    m_CursorRect.left < rectl.right && m_CursorRect.right > rectl.left)
                {
                    CursorOff();
                    m_CursorForcedOff = TRUE;
                }
            }
        }
    }

    if ((pParms->pDst == m_pPrimarySurface && m_iRotate) || (pParms->pSrc == m_pPrimarySurface && m_iRotate))
    {
        pParms->pBlt = &GPE::EmulatedBltRotate;
    }

    if (bDoRotation && pParms->pDst == m_pPrimarySurface)
    {
        pParms->pBlt = (SCODE (GPE::*)(GPEBltParms *))&SA2Video::WrappedEmulatedBlt;
    }

    return S_OK;
}

// This function would be used to undo the setting of clip registers etc

SCODE
SA2Video::BltComplete(
    GPEBltParms * pParms
    )
{
    if (!bDoRotation)
    {
        if (m_CursorForcedOff)
        {
            m_CursorForcedOff = FALSE;
            CursorOn();
        }
    }

    return S_OK;
}

int
SA2Video::InVBlank()
{
    static    BOOL    value = FALSE;
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("SA2Video::InVBlank\r\n")));
    value = !value;
    return value;
}

SCODE
SA2Video::SetPalette(
    const PALETTEENTRY * source,
    unsigned short       firstEntry,
    unsigned short       numEntries
    )
{
    if (bpp == 8)
    {
        if (firstEntry < 0 || firstEntry + numEntries > 256)
        {
            return E_INVALIDARG;
        }

        DispDrvrSetPalette(source,firstEntry,numEntries);
    }

    return S_OK;
}

ULONG *
APIENTRY
DrvGetMasks(
    DHPDEV dhpdev
    )
{
    return BitMasks;
}

// This routine maps between power manager power states and video
// ioctl power states.
ULONG
SA2Video::PmToVideoPowerState(CEDEVICE_POWER_STATE Dx)
{
    ULONG PowerState;

    switch( Dx )
    {
        case D0:
        case D1:
            PowerState = VideoPowerOn;
            break;

        case D2:
            PowerState = VideoPowerStandBy;
            break;

        case D3:
            PowerState = VideoPowerSuspend;
            break;

        case D4:
            PowerState = VideoPowerOff;
            break;

        default:
            PowerState = VideoPowerOn;
            break;
    }

    return(PowerState);
}

// This routine maps video power states to PM power states.
// Not currently used.
CEDEVICE_POWER_STATE
SA2Video::VideoToPmPowerState(ULONG PowerState)
{
    CEDEVICE_POWER_STATE Dx;

    switch( PowerState )
    {
        case VideoPowerOn:
            Dx = D0;
            break;

        case VideoPowerStandBy:
            Dx = D2;
            break;

        case VideoPowerSuspend:
            Dx = D3;
            break;

        case VideoPowerOff:
            Dx = D4;
            break;

        default:
            Dx = D0;
            break;
    }

    return Dx;
}

void SA2Video::SetPmPowerState(CEDEVICE_POWER_STATE PowerState)
{
    DEBUGMSG(ZONE_PM, (TEXT("SA2Video::SetPmPowerState: (D%d)\r\n"), PowerState));
    switch ( PowerState )
    {
        case D0:
        case D1:
            if (bSuspended)
            {
                DEBUGMSG(ZONE_PM, (TEXT("SA2Video::SetPmPowerState: TurnOn Display %d\r\n"), PowerState));
                DispDrvrPowerHandler(FALSE);
                bSuspended = FALSE;
            }
            break;

        case D2:
        case D3:
        case D4:
            if (!bSuspended)
            {
                DEBUGMSG(ZONE_PM, (TEXT("SA2Video::SetPmPowerState: TurnOff Display %d\r\n"), PowerState));
                DispDrvrPowerHandler(TRUE);
                bSuspended = TRUE;
            }
            break;
    }

    m_PmPowerState = PowerState;
}

CEDEVICE_POWER_STATE
SA2Video::GetPmPowerState(void)
{
    return(m_PmPowerState);
}

ULONG
SA2Video::GetVideoPowerState(void)
{
    return( PmToVideoPowerState(m_PmPowerState) );
}

// this routine converts a string into a GUID and returns TRUE if the
// conversion was successful.
BOOL
SA2Video::ConvertStringToGuid (LPCTSTR pszGuid, GUID *pGuid)
{
    UINT Data4[8];
    int  Count;
    BOOL fOk = FALSE;
    TCHAR *pszGuidFormat = _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

    DEBUGCHK(pGuid != NULL && pszGuid != NULL);
    __try
    {
        if (_stscanf(pszGuid, pszGuidFormat, &pGuid->Data1,
            &pGuid->Data2, &pGuid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3],
            &Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11)
        {
            for(Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++)
            {
                        pGuid->Data4[Count] = (UCHAR) Data4[Count];
            }
        }
        fOk = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return fOk;
}


// This routine notifies the OS that we support the Power Manager IOCTLs (through
// ExtEscape(), which calls DrvEscape()).
BOOL
SA2Video::AdvertisePowerInterface()
{
    BOOL fOk = FALSE;
    DWORD dwStatus;
    GUID gClass;
    TCHAR szModuleFileName[MAX_PATH];

    // PM assumes device is in power state D0 when it registers.
    m_PmPowerState    = D0;

    // assume we are advertising the default class
    ConvertStringToGuid(PMCLASS_DISPLAY, &gClass);
    DEBUGMSG(ZONE_PM,(TEXT("SA2Video::AdvertisePowerInterface: (%s)\r\n"), PMCLASS_DISPLAY));
    DEBUGMSG(ZONE_PM,(TEXT("SA2Video::AdvertisePowerInterface: 0x%x-0x%x-0x%x 0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x\r\n"),
                             gClass.Data1, gClass.Data2, gClass.Data3,
                             gClass.Data4[0], gClass.Data4[1], gClass.Data4[2], gClass.Data4[3],
                             gClass.Data4[4], gClass.Data4[5], gClass.Data4[6], gClass.Data4[7]));

    // Figure out what device name to advertise
    // Note - g_hmodDisplayDll is initialized in the DLL_PROCESS_ATTACH of DllMain()
    fOk = GetModuleFileName(g_hmodDisplayDll, szModuleFileName, sizeof(szModuleFileName) / sizeof(szModuleFileName[0]));
    if (!fOk) 
    {
        RETAILMSG(1,(TEXT("SA2Video::AdvertisePowerInterface: Failed to obtain DLL name. Driver is not power managed!\r\n")));
        return FALSE;
    }

    // Build the display device name for DevicePowerNotify().
    if( FAILED(StringCchCopy(m_DisplayDeviceName, sizeof(m_DisplayDeviceName)/sizeof(m_DisplayDeviceName[0]), PMCLASS_DISPLAY))||
        FAILED(StringCchCat(m_DisplayDeviceName, sizeof(m_DisplayDeviceName)/sizeof(m_DisplayDeviceName[0]), _T("\\"))) ||
        FAILED(StringCchCat(m_DisplayDeviceName, sizeof(m_DisplayDeviceName)/sizeof(m_DisplayDeviceName[0]), szModuleFileName))  )
    {
        RETAILMSG(1,(TEXT("SA2Video::AdvertisePowerInterface: Failed to construct unique name parameter. Driver is not power managed!\r\n")));
        return FALSE;
    }
    DEBUGMSG(ZONE_PM,(TEXT("SA2Video::AdvertisePowerInterface: m_DisplayDeviceName=%s\r\n"),m_DisplayDeviceName));

    // now advertise the interface
    fOk = AdvertiseInterface(&gClass, szModuleFileName, TRUE);

    if(fOk)
    {
        // Request initial power management state.
        dwStatus = DevicePowerNotify(m_DisplayDeviceName, m_PmPowerState, POWER_NAME);
        DEBUGMSG(ZONE_PM,(TEXT("SA2Video::AdvertisePowerInterface: dwStatus=0x%x\r\n"), dwStatus));
    }

    return fOk;
}

ULONG
SA2Video::DrvEscape(
    SURFOBJ * pso,
    ULONG     iEsc,
    ULONG     cjIn,
    PVOID     pvIn,
    ULONG     cjOut,
    PVOID     pvOut
    )
{
    int RetVal = 0; // default return value: "not supported"
    DWORD EscapeFunction;
    GXDeviceInfo *pgxoi;
    CEDEVICE_POWER_STATE NewDx;
    VIDEO_POWER_MANAGEMENT *pvpm;
    BOOL  bErr = TRUE;

    switch (iEsc)
    {
        case GETPOWERMANAGEMENT :

            if (!pvOut || (cjOut < sizeof(VIDEO_POWER_MANAGEMENT)))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return -1;
            }

            pvpm = (VIDEO_POWER_MANAGEMENT*)pvOut;
            pvpm->Length = sizeof(VIDEO_POWER_MANAGEMENT);
            pvpm->DPMSVersion = 0;
            pvpm->PowerState  = GetVideoPowerState();
            RetVal = 1;

            DEBUGMSG(ZONE_PM, (TEXT("SA2Video::GETPOWERMANAGEMENT: VidPowerState=0x%x\r\n"),pvpm->PowerState));

            break;

        case SETPOWERMANAGEMENT :
        {
            pvpm = (VIDEO_POWER_MANAGEMENT *)pvIn;

            if (!pvpm || (cjIn < sizeof(VIDEO_POWER_MANAGEMENT)))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return -1;
            }

            if (pvpm->Length < sizeof(VIDEO_POWER_MANAGEMENT))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return -1;
            }

            if (DevicePowerNotify(m_DisplayDeviceName, VideoToPmPowerState(pvpm->PowerState), POWER_NAME) )
            {
               DEBUGMSG(ZONE_PM, (TEXT("SA2Video::SetPowerManagement:DevicePowerNotify-success VPwrstate=%d (D%d)\r\n"),pvpm->PowerState, VideoToPmPowerState(pvpm->PowerState)));
            }
            else
            {
               RETAILMSG(1, (TEXT("SA2Video::SetPowerManagement:DevicePowerNotify failed\r\n")));
            }

            RetVal = 1;
            break;
        }
        case IOCTL_POWER_CAPABILITIES:
            if ( pvOut != NULL && cjOut == sizeof(POWER_CAPABILITIES) )
            {
                DEBUGMSG(ZONE_PM, (TEXT("SA2Video::SA2VIDEO:IOCTL_POWER_CAPABILITIES\r\n")));
                PPOWER_CAPABILITIES PowerCaps = (PPOWER_CAPABILITIES) pvOut;

                memcpy(PowerCaps, &DisplayDrvPowerCaps, sizeof(DisplayDrvPowerCaps));
                RetVal = 1;
            }
            else
            {
                SetLastError(MMSYSERR_INVALPARAM);
                RetVal = -1;
            }
            break;

        case IOCTL_POWER_QUERY:
            DEBUGMSG(ZONE_PM, (TEXT("SA2VIDEO::IOCTL_POWER_QUERY\r\n")));
            if ( pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE) )
            {
                // Return a good status on any valid query since
                // we are always ready to change power states.
                NewDx = *(PCEDEVICE_POWER_STATE) pvOut;

                if ( ! VALID_DX(NewDx) )
                {
                    DEBUGMSG(ZONE_PM, (TEXT("SA2VIDEO::IOCTL_POWER_QUERY-(D%d) success\r\n"),NewDx));
                    RetVal = 1;
                    bErr = FALSE;
                }
            }

            if (bErr)
            {
                RETAILMSG(1, (TEXT("SA2VIDEO::IOCTL_POWER_QUERY-(D%d) failed\r\n"),NewDx));
                SetLastError(MMSYSERR_INVALPARAM);
                RetVal = -1;
            }
            break;

        case IOCTL_POWER_SET:
            if ( pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE) )
            {
                NewDx = *(PCEDEVICE_POWER_STATE)pvOut;

                if ( VALID_DX(NewDx) )
                {
                    DEBUGMSG(ZONE_PM, (TEXT("SA2VIDEO::IOCTL_POWER_SET-(D%d) success\r\n"), NewDx));
                    SetPmPowerState(NewDx);
                    RetVal = 1;
                    bErr = FALSE;
                }
            }

            if (bErr)
            {
                RETAILMSG(1, (TEXT("SA2VIDEO::IOCTL_POWER_SET-(D%d) failed\r\n"), NewDx));
                SetLastError(MMSYSERR_INVALPARAM);
                RetVal = -1;
            }
            break;

        case IOCTL_POWER_GET:
            DEBUGMSG(ZONE_PM, (TEXT("SA2VIDEO::IOCTL_POWER_GET\r\n")));
            if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
            {
                *(PCEDEVICE_POWER_STATE) pvOut = GetPmPowerState();
                RetVal = 1;
            }
            break;

        case QUERYESCSUPPORT:
            EscapeFunction = *(DWORD *)pvIn;
            if ((EscapeFunction == VERTICALBLANKINTERRUPT)   ||
                (EscapeFunction == SCROLL)                   ||
                (EscapeFunction == SETPOWERMANAGEMENT)       ||
                (EscapeFunction == GETPOWERMANAGEMENT)       ||
                (EscapeFunction == IOCTL_POWER_CAPABILITIES) ||
                (EscapeFunction == IOCTL_POWER_QUERY) ||
                (EscapeFunction == IOCTL_POWER_SET) ||
                (EscapeFunction == IOCTL_POWER_GET) ||
                (EscapeFunction == GETGXINFO))
            {
                RetVal = 1;
            }
            else if ((!g_fDisableRotation) &&
                ((EscapeFunction == DRVESC_GETSCREENROTATION) ||
                (EscapeFunction == DRVESC_SETSCREENROTATION)))
            {
                RetVal = 1;
            }
            break;

        case VERTICALBLANKINTERRUPT:
            RetVal = 1;
            break;

        case SCROLL:
            ScrollBuffer(cjIn);
            break;

        case DRVESC_GETSCREENROTATION:
            if (!g_fDisableRotation)
            {
                *(int *)pvOut = ((DMDO_0 | DMDO_90 | DMDO_180 | DMDO_270) << 8) | ((BYTE)m_iRotate);
                return DISP_CHANGE_SUCCESSFUL;
            }
            else
            {
                return DISP_CHANGE_FAILED;
            }
            break;

        case DRVESC_SETSCREENROTATION:
            if ((!g_fDisableRotation) &&
                ((cjIn == DMDO_0)    ||
                (cjIn == DMDO_90)    ||
                (cjIn == DMDO_180)    ||
                (cjIn == DMDO_270)))
                {
                    return DynRotate(cjIn);
                }

            return DISP_CHANGE_BADMODE;
            break;

        case GETGXINFO:
            // Fill out the GAPI data structure.  Assumes 16bpp or 8bpp, with RGB565 format.
            // Must change cBPP and ffFormat fields to accomodate other formats.
            // All of the button data that follows must be filled out to match the specific OEM device.
            // The data that is used to fill out the data structure here is copied from
            // the Microsoft example.
            if ((cjOut >= sizeof(GXDeviceInfo)) && (pvOut != NULL) )
            {
                if (((GXDeviceInfo *) pvOut)->idVersion == kidVersion100)
                {
                    pgxoi = (GXDeviceInfo *) pvOut;
                    pgxoi->idVersion = kidVersion100;
                    pgxoi->pvFrameBuffer = (void *) FRAME_BUFFER_0_BASE_VIRTUAL;

                    // cbStride reflects the physical properties of the display regardless of orientation.
                    // Using a native portrait mode display, stride will always be 480.
                    // If using native landscape mode display, stride would be 640.
                    if (((DispDrvr_cxScreen == 240) && (DispDrvr_cyScreen == 320)) ||
                        ((DispDrvr_cxScreen == 320) && (DispDrvr_cyScreen == 240)) )
                    {
                        if (bpp == 16)
                        {
                            pgxoi->cbStride = 480;
                            pgxoi->cBPP = 16;
                        }
                        if (bpp == 8)
                        {
                            pgxoi->cbStride = 240;
                            pgxoi->cBPP = 16;
                        }
                    }

                    // Using a native landscape mode display, stride will always be 1280.
                    // If using native portrait mode display, stride would be 960.

                    if (((DispDrvr_cxScreen == 480) && (DispDrvr_cyScreen == 640)) ||
                        ((DispDrvr_cxScreen == 640) && (DispDrvr_cyScreen == 480)) )
                    {
                        if (bpp == 16)
                        {
                            pgxoi->cbStride = 1280;
                            pgxoi->cBPP = 16;
                        }
                        if (bpp == 8)
                        {
                            pgxoi->cbStride = 640;
                            pgxoi->cBPP = 16;
                        }
                    }


                    pgxoi->cxWidth = DispDrvr_cxScreen;
                    pgxoi->cyHeight = DispDrvr_cyScreen;

                    // Set kfLandscape only if the display orientation is not in its native format
                    pgxoi->ffFormat= kfDirect565;


                    pgxoi->vkButtonUpPortrait = VK_UP;
                    pgxoi->vkButtonUpLandscape = VK_LEFT;
                    pgxoi->ptButtonUp.x = 120;
                    pgxoi->ptButtonUp.y = 350;
                    pgxoi->vkButtonDownPortrait = VK_DOWN;
                    pgxoi->vkButtonDownLandscape = VK_RIGHT;
                    pgxoi->ptButtonDown.x = 120;
                    pgxoi->ptButtonDown.y = 390;
                    pgxoi->vkButtonLeftPortrait = VK_LEFT;
                    pgxoi->vkButtonLeftLandscape = VK_DOWN;
                    pgxoi->ptButtonLeft.x = 100;
                    pgxoi->ptButtonLeft.y = 370;
                    pgxoi->vkButtonRightPortrait = VK_RIGHT;
                    pgxoi->vkButtonRightLandscape = VK_UP;
                    pgxoi->ptButtonRight.x = 140;
                    pgxoi->ptButtonRight.y = 370;
                    pgxoi->vkButtonAPortrait = 'A';
                    pgxoi->vkButtonALandscape = 'A';
                    pgxoi->ptButtonA.x = 100;
                    pgxoi->ptButtonA.y = 410;
                    pgxoi->vkButtonBPortrait = 'B';
                    pgxoi->vkButtonBLandscape = 'B';
                    pgxoi->ptButtonB.x = 120;
                    pgxoi->ptButtonB.y = 410;
                    pgxoi->vkButtonCPortrait = 'C';
                    pgxoi->vkButtonCLandscape = 'C';
                    pgxoi->ptButtonC.x = 140;
                    pgxoi->ptButtonC.y = 410;
                    pgxoi->vkButtonStartPortrait = 'D';
                    pgxoi->vkButtonStartLandscape = 'D';
                    pgxoi->ptButtonStart.x = 160;
                    pgxoi->ptButtonStart.y = 410;
                    pgxoi->pvReserved1 = (void *) 0;
                    pgxoi->pvReserved2 = (void *) 0;
                    RetVal = 1;

                } else
                {
                    SetLastError (ERROR_INVALID_PARAMETER);
                    RetVal = -1;
                }
            } else {
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = -1;
            }
            break;

        default:
            RetVal = 0;
            break;
    }
    return RetVal;
}

int
SA2Video::GetRotateModeFromReg()
{
    HKEY hKey;
    int nRet = DMDO_0;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\GDI\\ROTATION"),0,0, &hKey))
    {
        DWORD dwSize, dwAngle, dwType = REG_DWORD;
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                               TEXT("ANGLE"),
                                               NULL,
                                               &dwType,
                                               (LPBYTE)&dwAngle,
                                               &dwSize))
        {
            switch (dwAngle)
            {
            case 90:
                nRet = DMDO_90;
                break;
            case 180:
                nRet = DMDO_180;
                break;
            case 270:
                nRet = DMDO_270;
                break;
            case 0:
                // fall through
            default:
                nRet = DMDO_0;
                break;
            }
        }

        RegCloseKey(hKey);
    }

    return nRet;
}

void
SA2Video::SetRotateParams()
{
    int iswap;
    switch(m_iRotate)
    {
    case DMDO_0:
        m_nScreenHeightSave = m_nScreenHeight;
        m_nScreenWidthSave = m_nScreenWidth;
        break;

    case DMDO_180:
        m_nScreenHeightSave = m_nScreenHeight;
        m_nScreenWidthSave = m_nScreenWidth;
        break;

    case DMDO_90:
    case DMDO_270:
        iswap = m_nScreenHeight;
        m_nScreenHeight = m_nScreenWidth;
        m_nScreenWidth = iswap;
        m_nScreenHeightSave = m_nScreenWidth;
        m_nScreenWidthSave = m_nScreenHeight;
        break;

    default:
        m_nScreenHeightSave = m_nScreenHeight;
        m_nScreenWidthSave = m_nScreenWidth;
        break;
    }

    return;
}

LONG
SA2Video::DynRotate(
    int angle
    )
{
    GPESurfRotate *pSurf = (GPESurfRotate *)m_pPrimarySurface;

    if (m_InDDraw)
    {
        return DISP_CHANGE_BADMODE;
    }

    if (angle == m_iRotate)
    {
        return DISP_CHANGE_SUCCESSFUL;
    }

    CursorOff();

    m_iRotate = angle;

    switch(m_iRotate)
    {
    case DMDO_0:
    case DMDO_180:
        m_nScreenHeight = m_nScreenHeightSave;
        m_nScreenWidth  = m_nScreenWidthSave;
        break;

    case DMDO_90:
    case DMDO_270:
        m_nScreenHeight = m_nScreenWidthSave;
        m_nScreenWidth  = m_nScreenHeightSave;
        break;
    }

    m_pMode->width  = m_nScreenWidth;
    m_pMode->height = m_nScreenHeight;
    pSurf->SetRotation(m_nScreenWidth, m_nScreenHeight, angle);

    CursorOn();

    return DISP_CHANGE_SUCCESSFUL;
}

void
SA2Video::CursorOn()
{
    USHORT * ptrScreen = (USHORT *)m_pPrimarySurface->Buffer();
    USHORT * ptrLine;
    USHORT * cbsLine;

    if (!m_CursorForcedOff && !m_CursorDisabled && !m_CursorVisible)
    {
        RECTL cursorRectSave = m_CursorRect;
        int   iRotate;

        RotateRectl(&m_CursorRect);
        for (int y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
        {
            if (y < 0)
            {
                continue;
            }
            if (y >= m_nScreenHeightSave)
            {
                break;
            }

            ptrLine = &ptrScreen[y * m_pPrimarySurface->Stride() / 2];
            cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * m_CursorSize.x];

            for (int x = m_CursorRect.left; x < m_CursorRect.right; x++)
            {
                if (x < 0)
                {
                    continue;
                }
                if (x >= m_nScreenWidthSave)
                {
                    break;
                }

                // x' = x - m_CursorRect.left; y' = y - m_CursorRect.top;
                // Width = m_CursorSize.x;   Height = m_CursorSize.y;
                switch (m_iRotate)
                {
                    case DMDO_0:
                        iRotate = (y - m_CursorRect.top)*m_CursorSize.x + x - m_CursorRect.left;
                        break;
                    case DMDO_90:
                        iRotate = (x - m_CursorRect.left)*m_CursorSize.x + m_CursorSize.y - 1 - (y - m_CursorRect.top);
                        break;
                    case DMDO_180:
                        iRotate = (m_CursorSize.y - 1 - (y - m_CursorRect.top))*m_CursorSize.x + m_CursorSize.x - 1 - (x - m_CursorRect.left);
                        break;
                    case DMDO_270:
                        iRotate = (m_CursorSize.x -1 - (x - m_CursorRect.left))*m_CursorSize.x + y - m_CursorRect.top;
                        break;
                    default:
                        iRotate = (y - m_CursorRect.top)*m_CursorSize.x + x - m_CursorRect.left;
                        break;
                }

                cbsLine[x - m_CursorRect.left] = ptrLine[x];
                ptrLine[x] &= gCursorMask[iRotate];
                ptrLine[x] ^= gCursorData[iRotate];
            }
        }

        m_CursorRect    = cursorRectSave;
        m_CursorVisible = TRUE;
    }
}

void
SA2Video::CursorOff()
{
    USHORT * ptrScreen = (USHORT*)m_pPrimarySurface->Buffer();
    USHORT * ptrLine;
    USHORT * cbsLine;

    if (!m_CursorForcedOff && !m_CursorDisabled && m_CursorVisible)
    {
        RECTL rSave = m_CursorRect;
        RotateRectl(&m_CursorRect);

        for (int y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
        {
            // clip to displayable screen area (top/bottom)
            if (y < 0)
            {
                continue;
            }
            if (y >= m_nScreenHeightSave)
            {
                break;
            }

            ptrLine = &ptrScreen[y * m_pPrimarySurface->Stride() / 2];
            cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * m_CursorSize.x];

            for (int x = m_CursorRect.left; x < m_CursorRect.right; x++)
            {
                // clip to displayable screen area (left/right)
                if (x < 0)
                {
                    continue;
                }
                if (x >= (int)m_nScreenWidthSave)
                {
                    break;
                }

                ptrLine[x] = cbsLine[x - m_CursorRect.left];
            }
        }

        m_CursorRect = rSave;
        m_CursorVisible = FALSE;
    }
}

