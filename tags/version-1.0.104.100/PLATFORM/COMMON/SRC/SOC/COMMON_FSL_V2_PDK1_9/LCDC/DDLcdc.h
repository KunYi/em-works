//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:       DDLcdc.h
//  Purpose:    DirectDraw display driver class header
//  
//------------------------------------------------------------------------------

 
#ifndef _DRIVERS_DISPLAY_DDLCDC_DDLCDC_H
#define _DRIVERS_DISPLAY_DDLCDC_DDLCDC_H

//------------------------------------------------------------------------------
// MACRO DEFINITIONS 
//------------------------------------------------------------------------------
#define MAX_BUSY_RETRY                        (5)
#define LCDC_MIN_GRAPHICWINDOW_WIDTH          16      // 16-pixel is minimum width

//------------------------------------------------------------------------------
// ENUMERATIONS AND STRUCTURES 
//------------------------------------------------------------------------------
// Types
typedef struct overlaySurf{
    BOOL     isGraphicWindowRunning;

    UINT32   nBufPhysicalAddr;

    // Overlay features
    UINT16   Width;
    UINT16   Height;
    UINT16   WidthHw;
    UINT16   HeightHw;
    UINT16   LineStride;
    UINT16   XOffset;
    UINT16   YOffset;
    UINT16   Transparency;  // 255 indicate totally transparent, ie. not displayed on LCD screen
                            // 0 indicate totally opaque, ie. overlay on LCD screen
    BOOL     isUpsideDown;
    UINT32   ColorKeyMask;  // 0 indicate no color key is used  
}overlaySurf_t, *pOverlaySurf_t;

//------------------------------------------------------------------------------
// CLASS DEFINITIONS

class MXDDLcdcSurf;

class MXDDLcdc : public DDGPE
{
    protected:
        PUCHAR          m_pVideoMemory;         // Virtual Address of the video memory
        HANDLE          m_hLAWMapping;          // Handle for file mapping
        ULONG           m_nLAWPhysical;         // Physical Linear Access Window (LAW) address
        PUCHAR          m_pLAW;                 // Mapped Linear Access Window (LAW) address
        PCSP_LCDC_REGS  m_pLcdcReg;             // Mapped LCDC Register Access Window (REG) address
        LCDC_MODE_DESCRIPTOR        m_ModeDesc;
        ULONG           m_nVideoMemorySize;     // Size in bytes of actual video RAM total
        SurfaceHeap    *m_pVideoMemoryHeap;     // Base entry representing all video memory

        GPEModeEx       m_ModeInfoEx;           // local mode info
        GPEMode        *m_ModesInfo;

        DDGPESurf      *m_pBlankSurface;        // A blank surface used when OS goes to power suspension

        DDGPESurf      *m_pVisibleOverlay;      // Visible overlay surface

        BOOL            m_bSetMode;
        UINT            m_nScreenBpp;
        ULONG           m_nScreenStride;        // byte count of a single scan line
        ULONG           m_nSurfacePixelAlign;   // Surface Pixel alignments
        UINT            m_nMask;    

        BOOL            m_fStopIntrProc;        // Use to stop the IST
    private:
        // For Graphic window (overlay)
        pOverlaySurf_t  m_pOverlaySurfaceOp;
        UINT32          m_pOverlaySrcOffset;

        // For software cursor
        BOOL            m_CursorDisabled;
        BOOL            m_CursorVisible;
        BOOL            m_CursorForcedOff;
        RECTL           m_CursorRect;
        POINTL          m_CursorSize;
        POINTL          m_CursorHotspot;        

        // allocate enough backing store for a 64x64 cursor on a 32bpp (4 bytes per pixel) screen
        UCHAR           m_CursorBackingStore[64 * 64 * 4];
        UCHAR           m_CursorXorShape[64 * 64];
        UCHAR           m_CursorAndShape[64 * 64];

        // For interrupt event
        HANDLE                m_hSyncEvent;                 // Event for Sync
        HANDLE                m_hSyncThread;                // Thread for LCDC interrupt
        HANDLE                m_hExitSyncThread;            // Event for properly exiting the interrupt thread
        UINT32                m_nPreIntrStatus;             // The status for last interrupt
        CRITICAL_SECTION      m_CriticalSection;


        // For power handling
        CEDEVICE_POWER_STATE  m_Dx;                    // Current device state
        TCHAR                 m_szDevName[MAX_PATH];   // Device name
        TCHAR                 m_szGuidClass[MAX_PATH]; // Class GUID

    private:
        // DDLcdcPower.cpp
        VOID SetDisplayPower(CEDEVICE_POWER_STATE dx);                  // display hardware power control function
        VIDEO_POWER_STATE PmToVideoPowerState(CEDEVICE_POWER_STATE dx); // video power state to Power Manager device power state conversion functions
        CEDEVICE_POWER_STATE VideoToPmPowerState(VIDEO_POWER_STATE vps);
        BOOL ConvertStringToGuid(LPCTSTR pszGuid, GUID *pGuid);         // String to GUID conversion function
        BOOL AdvertisePowerInterface(HMODULE hInst);

        // DDLcdcMisc.cpp
        //VOID LoadFreescaleLogo(GPESurf * pSurf);
        //VOID LoadFreescaleLogo(VOID *pFramePointer);
        
        // DDLcdc.cpp
        BOOL    Init(VOID);
        BOOL    ReInit(VOID);
        BOOL    InitHardware(VOID);
        BOOL    AllocVideoMemory(VOID);
        BOOL    FreeVideoMemory(VOID);
        BOOL    SetModeHardware(int modeNo);
        DWORD   IntrProc(VOID);
        VOID    Cleanup(VOID);
        
    public:
        // DDLcdc.cpp
        MXDDLcdc(BOOL *pbRet);
        ~MXDDLcdc(VOID);
        
        SCODE   SetMode(int modeId, HPALETTE *pPalette);        
        SCODE   GetModeInfo(GPEMode *pMode, int modeNo);
        SCODE   GetModeInfoEx(GPEModeEx *pModeEx, int modeNo);
        int     NumModes(VOID);
        VOID    GetPhysicalVideoMemory(PULONG pPhysicalMemoryBase, PULONG pVideoMemorySize);
        int     GetBpp(VOID);

        // DDLcdcLine.cpp
        SCODE   Line(GPELineParms *pLineParms, EGPEPhase phase);        
        SCODE   WrappedEmulatedLine(GPELineParms *pLineParms);

        // DDLcdcBlt.cpp
        SCODE   BltPrepare(GPEBltParms *pBltParms);
        SCODE   BltComplete(GPEBltParms *pBltParms);

        // DDLcdcSurf.cpp
        SCODE   AllocSurface(GPESurf **ppSurf, int width, int height, EGPEFormat format, int surfaceFlags);
        SCODE   AllocSurface(DDGPESurf **ppSurf, int width, int height, EGPEFormat format, EDDGPEPixelFormat pixelFormat, int surfaceFlags);
        VOID    SetVisibleSurface( GPESurf *pSurf, BOOL bWaitForVBlank = FALSE);
        DWORD   Flip(LPDDHAL_FLIPDATA pd);

        // DDLcdcMisc.cpp
        ULONG   DrvEscapeGAPI(ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut);
        ULONG   DrvEscape(SURFOBJ *pso, ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut);
        SCODE   SetPalette(const PALETTEENTRY *src,unsigned short firstEntry,unsigned short numEntries );
        SCODE   SetRGBQuadPalette(const RGBQUAD *src,unsigned short firstEntry,unsigned short numEntries );
        ULONG   GetGraphicsCaps(VOID) {return GCAPS_GRAY16|GCAPS_CLEARTYPE;}  // Notify GDI we are supporting ClearType and anti-aliased fonts.
        VOID    WaitForNotBusy(VOID);
        int     IsBusy(VOID);
        int     InVBlank(VOID);
        DWORD   WaitForVerticalBlank(LPDDHAL_WAITFORVERTICALBLANKDATA pd);      

        // DDLcdcCursor.cpp
        SCODE   SetPointerShape(GPESurf *pMask, GPESurf *pColorSurf, int xHot, int yHot, int cx, int cy);
        SCODE   MovePointer(int x, int y);
        VOID    CursorOn(VOID);
        VOID    CursorOff(VOID);
        VOID    PointerBltPrepare(GPEBltParms *pBltParms);

        // DDLcdcRotate.cpp
        int     GetRotateModeFromReg(VOID);
        VOID    SetRotateParams(VOID);
        LONG    DynRotate(int angle);

        // DDLcdcOverlay.cpp
        DWORD   UpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA pd);
        DWORD   SetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA pd);
        VOID    SetVisibleSurfaceOverlay(DDGPESurf *pSurf);
        HRESULT EnableOverlay(VOID);
        HRESULT DisableOverlay(VOID);

        HRESULT GetOverlayAlphaValue(VOID);
        HRESULT SetOverlayAlphaValue(VOID);

        VOID    WaitForNotBusyOverlay(VOID);

        friend VOID buildDDHALInfo( LPDDHALINFO lpddhi, DWORD modeidx ); // Only used for Windows CE 5.00 but not Magneto.
        friend DWORD MXDDLcdcIntr(MXDDLcdc *pClass);
};

class MXDDLcdcSurf : public DDGPESurf
{
    private:
        SurfaceHeap * m_pHeap;

    public:
        MXDDLcdcSurf(int width, int height, ULONG offset, PVOID pBits, int stride,
                EGPEFormat format, EDDGPEPixelFormat pixelFormat,
                SurfaceHeap *pHeap);

        ~MXDDLcdcSurf(VOID);
};

#endif //_DRIVERS_DISPLAY_DIRECTDRAW_DDLCDC_H
