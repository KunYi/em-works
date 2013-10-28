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
//------------------------------------------------------------------------------
// Copyright (C) 2004-2005, MOTOROLA, INC. All Rights Reserved
//------------------------------------------------------------------------------
//---------------------------------------------------------------------------
//  Copyright (C) 2004-2006, 2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:       DDLcdc.h
//  Purpose:    MX27 DirectDraw display class header
//  
//------------------------------------------------------------------------------

 
#ifndef _DRIVERS_DISPLAY_DDLCDC_DDLCDC_H
#define _DRIVERS_DISPLAY_DDLCDC_DDLCDC_H

//------------------------------------------------------------------------------
// MACRO DEFINITIONS 
//------------------------------------------------------------------------------
#define DRIVER_REGISTRY_STRING                TEXT("Drivers\\Display\\DDLCDC")
#define DRIVER_REGISTRY_STRING_CX             TEXT("CxScreen")
#define DRIVER_REGISTRY_STRING_CY             TEXT("CyScreen")
#define DRIVER_REGISTRY_STRING_BPP            TEXT("Bpp")
#define DRIVER_REGISTRY_STRING_MEM_SIZE       TEXT("VideoMemSize")
#define DISPLAY_DRIVER_THREAD_PRIORITY        150  

#define DEFAULT_VIDEO_MEM_SIZE                (1024*1024)   // 1M bytes for default video RAM size

#define MAX_BUSY_RETRY                        (25)
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

class MX27DDLcdcSurf;

class MX27DDLcdc : public DDGPE
{
    protected:
        PUCHAR          m_pVideoMemory;         // Virtual Address of the video memory
        HANDLE          m_hLAWMapping;          // Handle for file mapping
        ULONG           m_nLAWPhysical;         // Physical Linear Access Window (LAW) address
        PUCHAR          m_pLAW;                 // Mapped Linear Access Window (LAW) address
        PCSP_LCDC_REGS  m_pLcdcReg;             // Mapped LCDC Register Access Window (REG) address
        LCDC_CTX        m_LcdcCtx;
        ULONG           m_nVideoMemorySize;     // Size in bytes of actual video RAM total
        SurfaceHeap    *m_pVideoMemoryHeap;     // Base entry representing all video memory

        GPEModeEx       m_ModeInfoEx;           // local mode info

        DDGPESurf      *m_pBlankSurface;        // A blank surface used when OS goes to power suspension

        DDGPESurf      *m_pVisibleOverlay;      // Visible overlay surface

        BOOL            m_bSetMode;
        UINT            m_nScreenBpp;
        ULONG           m_nScreenStride;        // byte count of a single scan line
        ULONG           m_nSurfacePixelAlign;   // Surface Pixel alignments
        UINT            m_nMask;    

    private:
        // For Graphic window (overlay)
        pOverlaySurf_t  m_pOverlaySurfaceOp;

        BOOL            m_bTVNTSCOut;
        BOOL            m_bTVModeActive;        // is TV out mode active?
        DWORD           m_nPanelTYPE;

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
        UINT32                m_nPreIntrStatus;             // The status for last interrupt
        CRITICAL_SECTION      m_CriticalSection;


        // For power handling
        CEDEVICE_POWER_STATE  m_Dx;                    // Current device state
        TCHAR                 m_szDevName[MAX_PATH];   // Device name
        TCHAR                 m_szGuidClass[MAX_PATH]; // Class GUID

        //For Flip Status Indication
        BOOL m_bFlipInProgress;
    private:
        // DDLcdcPower.cpp
        VOID SetDisplayPower(CEDEVICE_POWER_STATE dx);                  // display hardware power control function
        VIDEO_POWER_STATE PmToVideoPowerState(CEDEVICE_POWER_STATE dx); // video power state to Power Manager device power state conversion functions
        CEDEVICE_POWER_STATE VideoToPmPowerState(VIDEO_POWER_STATE vps);
        BOOL ConvertStringToGuid(LPCTSTR pszGuid, GUID *pGuid);         // String to GUID conversion function
        BOOL AdvertisePowerInterface(HMODULE hInst);

        // DDLcdcMisc.cpp
        //BOOL GetModeFromRegistry(VOID);
        //BOOL GetVMemSizeFromRegistry(VOID);
        //VOID LoadFreescaleLogo(GPESurf * pSurf);
        //VOID LoadFreescaleLogo(VOID *pFramePointer);
        
        // DDLcdc.cpp
        BOOL    Init(VOID);
        BOOL    InitHardware(VOID);
        BOOL    SetupVideoMemory(VOID);
        BOOL    SetModeHardware(int modeNo);
        DWORD   IntrProc(VOID);
        VOID    Cleanup(VOID);
        
    public:
        // DDLcdc.cpp
        MX27DDLcdc(BOOL *pbRet);
        ~MX27DDLcdc(VOID);
        
        SCODE   SetMode(int modeId, HPALETTE *pPalette);        
        SCODE   GetModeInfo(GPEMode *pMode, int modeNo);
        SCODE   GetModeInfoEx(GPEModeEx *pModeEx, int modeNo);
        int     NumModes(VOID);
        VOID    GetPhysicalVideoMemory(PULONG pPhysicalMemoryBase, PULONG pVideoMemorySize);
        int     GetBpp(VOID);

        // DDLcdcLine.cpp
        SCODE   Line(GPELineParms *pLineParms, EGPEPhase phase);        

        // DDLcdcBlt.cpp
        SCODE   BltPrepare(GPEBltParms *pBltParms);
        SCODE   BltComplete(GPEBltParms *pBltParms);

        // DDLcdcSurf.cpp
        SCODE   AllocSurface(GPESurf **ppSurf, int width, int height, EGPEFormat format, int surfaceFlags);
        SCODE   AllocSurface(DDGPESurf **ppSurf, int width, int height, EGPEFormat format, EDDGPEPixelFormat pixelFormat, int surfaceFlags);
        VOID    SetVisibleSurface( GPESurf *pSurf, BOOL bWaitForVBlank = FALSE);
        DWORD   Flip(LPDDHAL_FLIPDATA pd);
     DWORD  GetFlipStatus(LPDDHAL_GETFLIPSTATUSDATA pd);

        // DDLcdcMisc.cpp
        ULONG   DrvEscapeGAPI(ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut);
        ULONG   DrvEscape(SURFOBJ *pso, ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut);
        SCODE   SetPalette(const PALETTEENTRY *src,unsigned short firstEntry,unsigned short numEntries ) {return S_OK;}
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
#if 0   // These three function is for furture expansion (alpha blending and more efficient invert overlay)
        HRESULT InverOverlay(VOID);
        HRESULT GetOverlayAlphaValue(VOID);
        HRESULT SetOverlayAlphaValue(VOID);
#endif      
        VOID    WaitForNotBusyOverlay(VOID);

        friend VOID buildDDHALInfo( LPDDHALINFO lpddhi, DWORD modeidx ); // Only used for Windows CE 5.00 but not Magneto.
        friend DWORD MX27DDLcdcIntr(MX27DDLcdc *pClass);
};

class MX27DDLcdcSurf : public DDGPESurf
{
    private:
        SurfaceHeap * m_pHeap;

    public:
        MX27DDLcdcSurf(int width, int height, ULONG offset, PVOID pBits, int stride,
                EGPEFormat format, EDDGPEPixelFormat pixelFormat,
                SurfaceHeap *pHeap);

        ~MX27DDLcdcSurf(VOID);
};

#endif //_DRIVERS_DISPLAY_DIRECTDRAW_DDLCDC_H
