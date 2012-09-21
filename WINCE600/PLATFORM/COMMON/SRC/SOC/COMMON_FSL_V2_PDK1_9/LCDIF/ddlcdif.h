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
/*++
   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Module Name:

   Abstract:

   Functions:

   Notes:

   --*/

 
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  ddlcdif.h
//
//  LCDIF DirectDrawdisplay class header
//
//------------------------------------------------------------------------------

#ifndef __DDGPE_LCDIF_H__
#define __DDGPE_LCDIF_H__

#include "common_pxp.h"
#include "common_lcdif.h"
#ifdef USE_DCP
#include "common_dcp.h"
#endif

// DrvEscape return codes
#define ESC_FAILED          (-1)
#define ESC_NOT_SUPPORTED   (0)
#define ESC_SUCCESS         (1)


#define RGB565_COLOR_WHITE  0xBDF7
#define RGB565_COLOR_PURPLE 0xB818
#define RGB565_COLOR_RED    0xB800
#define RGB565_COLOR_GREEN  0x05E0
#define RGB565_COLOR_BLUE   0x0017
#define RGB565_COLOR_BLACK  0x0000

#define ARGB8888_COLOR_WHITE  0x00FFFFFF
#define ARGB8888_COLOR_PURPLE 0x00BF00C0
#define ARGB8888_COLOR_ORANGE 0x00EFB11F
#define ARGB8888_COLOR_RED    0x00FF0000
#define ARGB8888_COLOR_GREEN  0x0000FF00
#define ARGB8888_COLOR_BLUE   0x000000FF
#define ARGB8888_COLOR_BLACK  0x00000000

#define SET_KEYSRC          0x1
#define SET_KEYDEST         0x2
#define SET_ALPHASRC        0x4
#define SET_ALPHADEST       0x8

//------------------------------------------------------------------------------
// ENUMERATIONS AND STRUCTURES 
//------------------------------------------------------------------------------
// Types
typedef union 
{
    UINT32 U;
    struct
    {
        unsigned KEYSRC_EN:1;
        unsigned KEYDEST_EN :1;
        unsigned ALPHASRC_EN  :1;
        unsigned ALPHACONSTANT_EN  :1;
        unsigned Dummy      :28;      // Dummy variable for alignment.
    } B;
} OverlaySetPara_t;

typedef struct overlaySurf{
    BOOL     isGraphicWindowRunning;

    UINT32   nBufPhysicalAddr;
    PBYTE   pBufVirtualAddr;
             
    // Overlay features
    UINT16   DstRecWidth;
    UINT16   DstRecHeight;
    UINT16   DstRecWidthHw;
    UINT16   DstRecHeightHw;
    UINT16   XOffset;
    UINT16   YOffset;
    UINT16   SrcXOffset;
    UINT16   SrcYOffset;
    UINT16   SrcRecWidth;
    UINT16   SrcRecHeight;
    UINT16   SrcSurWidth;
    UINT16   SrcSurHeight;
    UINT8    Transparency;  // 0 indicate totally transparent, ie. not displayed on LCD screen
    // 255 indicate totally opaque, ie. overlay on LCD screen

    // Screen orientation info
    UINT32   iRotate;
    BOOL     isUpsideDown;
    BOOL     isLeftsideRight;

    UINT32   SrcColorKeyLow;
    UINT32   SrcColorKeyHigh;
    UINT32   DestColorKeyLow;
    UINT32   DestColorKeyHigh;
    UINT32   SrcColKeyForRotation;
    pxpColorKey AlphaDestColKeyForRotation;
    OverlaySetPara_t OverlayPara;
    EDDGPEPixelFormat SrcPixelFormat;
    EDDGPEPixelFormat DestPixelFormat;
}overlaySurf_t, *pOverlaySurf_t;

class DDLcdifSurf;

class DDLcdif : public DDGPE
{
    // halcaps.cpp
    friend void buildDDHALInfo(LPDDHALINFO lpddhi, DWORD modeidx);
    friend DWORD WINAPI CombineSurface_Update_thread(LPVOID lpParameter);

public:
    virtual ~DDLcdif();

    static DDLcdif* GetInstance();

    virtual int NumModes();

    
    // Overrides SetMode for GPE and DDGPE.
    SCODE   SetMode(int modeNo, HPALETTE* pPalette);

    SCODE   GetModeInfo(GPEMode* pMode, int modeNumber);
    SCODE   GetModeInfoEx(GPEModeEx* pModeEx, int modeNumber);
    
  
   
    DWORD   GetPhysicalModeId();
    BOOL    AllocPhysicalMemory();
    BOOL    AllocPrimarySurfaces(DWORD modeNumber);
    VOID    FreePrimarySurfaces();

    virtual int InVBlank();

    virtual SCODE SetPalette(const PALETTEENTRY * source,
                             USHORT firstEntry,
                             USHORT numEntries);

 
    virtual VOID    PowerHandler(BOOL bOff);

    virtual void WaitForNotBusy();
    virtual void WaitForVBlank();

    virtual int IsBusy();

    virtual void GetPhysicalVideoMemory(unsigned long* physicalMemoryBase,
                                        unsigned long* videoMemorySize);

    virtual SCODE BltPrepare(GPEBltParms* blitParameters);
    virtual SCODE BltComplete(GPEBltParms* blitParameters);

    ULONG   DrvEscapeGAPI(ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut);

    virtual ULONG DrvEscape(SURFOBJ* pso,
                            ULONG iEsc,
                            ULONG cjIn,
                            void* pvIn,
                            ULONG cjOut,
                            void* pvOut);

    // surf.cpp
    virtual SCODE AllocSurface(GPESurf** surface,
                               int width,
                               int height,
                               EGPEFormat format,
                               int surfaceFlags);

    virtual SCODE AllocSurface(DDGPESurf** ppSurf,
                               int width,
                               int height,
                               EGPEFormat format,
                               EDDGPEPixelFormat pixelFormat,
                               int surfaceFlags);

    virtual void SetVisibleSurface(GPESurf* pSurf,
                                   BOOL bWaitForVBlank = FALSE );

    DWORD   Flip(LPDDHAL_FLIPDATA pd);
    VOID   SetVisibleSurfacePrimary(DDGPESurf* pSurf);    

    DWORD   UpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA pd);
    VOID    SetVisibleSurfaceOverlay(DDGPESurf *pSurf);
    DWORD   SetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA pd);
    HRESULT EnableOverlay(VOID);
    HRESULT DisableOverlay(VOID);
    DWORD SetColorKey(LPDDHAL_SETCOLORKEYDATA pd);

    VOID    WaitForNotBusyOverlay(VOID);

    DWORD GetOverlayAlign();       //Get overlay alignment
    DWORD GetDownScaleFactorExp();   //Get PXP down scaling factor exponent

    
    // DDLcdifCursor.cpp
    SCODE   SetPointerShape(GPESurf *pMask, GPESurf *pColorSurf, int xHot, int yHot, int cx, int cy);
    SCODE   MovePointer(int x, int y);
    VOID    CursorOn(VOID);
    VOID    CursorOff(VOID);
    VOID    PointerBltPrepare(GPEBltParms *pBltParms);

    // DDLcdifLine.cpp
    SCODE   Line(GPELineParms *pLineParms, EGPEPhase phase);
    SCODE   WrappedEmulatedLine(GPELineParms *pLineParms);    

	//
	//LQK SEP-19-2012
	//
	// DDLcdif_shutdown.cpp
	DDGPESurf	   *m_pSplashSurface; //Surface for shutdown image
	BOOL DDLcdif::AllocSplashSurfaces(DWORD modeNumber);
	// END LQK SEP-19-2012

private:
    DDLcdif();

    BOOL    ConvertStringToGuid (LPCTSTR pszGuid, GUID *pGuid);

    int GetRotateModeFromReg();

    void SetRotateParams();

    long DynRotate(int angle);

    void fill_vertical_stripes(UINT16* pBuff,
                               UINT32 width,
                               UINT32 height);
    
    VOID SetDisplayPower(CEDEVICE_POWER_STATE dx);
    BOOL AdvertisePowerInterface(HMODULE hInst);
    VOID PoweroffLCDIF();
    VOID PowerOnLCDIF();

    SCODE AcceleratedFillRect(struct GPEBltParms* BltParams);
    SCODE AcceleratedSrcCopy(struct GPEBltParms* BltParams);

private:
    static DDLcdif* SingletonDDGPE;
    static const UINT16 colors[];
protected:
    GPEModeEx       m_ModeInfoEx;           // mode info
    BOOL            m_fStopIntrProc;        // IST halt flag
    UINT            m_nScreenBpp;
    ULONG           m_nScreenStride;        // byte count of a single scan line


    GPEModeEx ModeInfoEx;
    SurfaceHeap* FrameBufferHeap;

    DWORD FrameBufferSize;

    void* PhysicalMemAddr;      //FrameBuffer Heap physical address      
    void* VirtualMemAddr;       //FrameBuffer Heap virtual address       
    void* m_pVideoMemory;
    HANDLE m_hLAWMapping;
    ULONG           m_nLAWPhysical;     //FrameBuffer Heap physical address     
    DWORD               PhysicalMemOffset;

    DDGPESurf      *m_pVisibleOverlay;      // Visible overlay surface

    pOverlaySurf_t  m_pOverlaySurfaceOp;    //Overlay parameters structure

    // ---------------------------------
    // for YUV
    void *  m_pVBufAdjVirtAddr;         //V image data virtual address
    ULONG   m_nVBufAdjPhysAddr;         //V image data physical address
    void *  m_pUBufAdjVirtAddr;         //U image data virtual address
    ULONG   m_nUBufAdjPhysAddr;         //U image data physical address

    // ---------------------------------
    HANDLE m_hPXP;      //Handle for PXP driver

    DDGPESurf      *m_pCombinedSurface;     //Surface for combination
    DDGPESurf      *m_pBackgroundSurface;     //Surface for background flipping while there is overlay surface 

    BOOL m_bManualCombine;      //Flag for PXP combination operation
    BOOL m_bWaitPXPNotBusy;     //PXP wait flag

    UINT32 m_nOverlayAlign;     //Overlay alignment
    UINT16 m_nAlignMask;    //Align mask due to overlay alignment
    DWORD m_nDownScaleExp;  //PXP down scaling factor exponent

    // ---------------------------------
    // for rotation
    DDGPESurf      *m_pRotatedSurface;      //Surface for rotation
    UINT16 m_iOrgRotSurfaceWidth;       //Original rotation surface width, in case  rotation surface needs to reallocate     
    UINT16 m_iOrgRotSurfaceHeight;       //Original rotation surface height, in case  rotation surface needs to reallocate


    int                 m_nCurrentDisplayMode;
    // ----------------------------------
    // for software cursor support
    BOOL                m_CursorDisabled;
    BOOL                m_CursorVisible;
    BOOL                m_CursorForcedOff;
    RECTL               m_CursorRect;
    POINTL              m_CursorSize;
    POINTL              m_CursorHotspot;

    // allocate enough backing store for a 64x64 cursor on a 32bpp (4 bytes per pixel) screen
    UCHAR           m_CursorBackingStore[64 * 64 * 4];
    UCHAR           m_CursorXorShape[64 * 64];
    UCHAR           m_CursorAndShape[64 * 64];


    // ----------------------------------
    // for interrupt handler
    DWORD               IntrProc(VOID);
    static DWORD        DDLcdcIntr(DDLcdif *pClass);// stub for IST
    DWORD               m_Sysintr;               // IRQ number
    HANDLE              m_hSyncEvent;            // IST event
    HANDLE              m_hSyncThread;           // IST handle
    UINT32              m_nPreIntrStatus;        // interrupt status
    // for power management
    CEDEVICE_POWER_STATE m_Dx;     // Current device state
    CEDEVICE_POWER_STATE m_PmPowerState;
    TCHAR m_szDevName[MAX_PATH];   // Device name
    TCHAR m_szGuidClass[MAX_PATH]; // Class GUID

    CRITICAL_SECTION    m_IntrCS;
    CRITICAL_SECTION    m_AllocCS;
    CRITICAL_SECTION    m_DrawCS;
    CRITICAL_SECTION    m_PxpOperateCS;
#ifdef USE_DCP
    DCPHandle_t blit_handle;    //DCP handler
#endif

}; //DDLcdif

class DDLcdifSurf : public DDGPESurf
{

public:
    DDLcdifSurf(int width, int height, ULONG offset,
        void* pBits, int stride,
        EGPEFormat format, EDDGPEPixelFormat pixelFormat,
        SurfaceHeap* pHeap);

    virtual ~DDLcdifSurf();

    DWORD GetSize() {
        return size;
    }                               //GetSize

private:
    SurfaceHeap* pHeap;

    DWORD size;

}; //DDLcdifSurf

#endif /* __DDGPE_LCDIF_H__ */

