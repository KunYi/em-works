//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddipu.h
//
//  IPU DirectDrawdisplay class header
//
//------------------------------------------------------------------------------
#ifndef __DDIPU_H__
#define __DDIPU_H__

class DDIPUSurf;

//------------------------------------------------------------------------------
// Defines

#define MAX_BUSY_RETRY             (5)

// Additional surface allocation flag to request
// that a surface be allocated from the secondary
// video memory region.
//
// NOTE: This must be at least "one bigger" than what Microsoft has already
//       defined for the "GPE_" flags in PUBLIC\COMMON\OAK\INC\gpe.h.
//
// WINCE600: The last Microsoft-defined flag is GPE_BACK_BUFFER = 0x4 so we
//           can use 0x8.
#define GPE_REQUIRE_SECONDARY_VIDEO_MEMORY 0x00000008

#define DDSCAPS_PRIMARYSURFACE2 (DDSCAPS_PRIMARYSURFACE|DDSCAPS_VIDEOPORT)  //for second primary surface

//--------------------------------------------------------
// DrvEscape return codes
//--------------------------------------------------------
#define ESC_FAILED              (-1)
#define ESC_NOT_SUPPORTED       (0)
#define ESC_SUCCESS             (1)


//------------------------------------------------------------------------------
// Types

typedef enum displayMode
{
    DISPLAY_240x320x16bpp_default,   // QVGA display mode
    //DISPLAY_240x320x8bpp,          //currently not supported
    //DISPLAY_240x320x18bpp,         //currently not supported
    //DISPLAY_240x320x32bpp,         //currently not supported
    DISPLAY_640x480x16bpp,           // VGA display mode
    DISPLAY_720x480x16bpp,           // NTSC TV mode
    numModes,                        // This enum should be ended by numModes
}displayMode_c;


//------------------------------------------------------------------------------
// Functions

class DDIPU : public DDGPE
{
    protected:
        ULONG           m_nVideoMemorySize;     // Size in bytes of actual video RAM total
        ULONG           m_nVideoMemoryBase;     // video memory base address
        
        GPEModeEx       m_ModeInfoEx;           // local mode info

        EDDGPEPixelFormat m_OutputFormat;       // Format for output frames

        GPEMode        *m_SupportedModes;       // Array of supported modes
        RECT            m_rcWorkRect;

        BOOL            m_bSetMode;
        UINT            m_nScreenBpp;
        UINT            m_nVideoBpp;
        ULONG           m_nScreenStride;        // byte count of a single scan line
        ULONG           m_nSurfacePixelAlign;   // Surface Pixel alignments

        INT             m_nCurrentDisplayMode;  // The current display mode setting
        INT             m_nPrimaryDisplayMode;   // The primary display mode setting, needed
                                                // to configure the second primary for dual display mode
        DWORD           m_nNumSupportedModes;   // number of supported modes
        DDIPUSurf      *m_pPrimarySurface2;     // The primary surface of second display device.
        overlaySurf_t   m_Overlay2SurfaceInfo;

        float           m_fGammaValue;          // LCD panel gamma value
    private:
#if defined(USE_C2D_ROUTINES)
        C2D_CONTEXT     m_c2dCtx;   
        DWORD           m_c2dFlag;
        DDIPUSurf*     m_TempSurf;
#endif
        // For Graphic window (overlay)
        pOverlaySurf_t  m_pOverlaySurfaceInfo;  // Pointer to first overlay surface, for all overlay surfaces managed by DDRAW
        pOverlaySurf_t *m_ppActiveOverlays;     // Array of active overlays
        BOOL            m_bIsOverlayWindowRunning;

        // Synchronization when changing modes
        CRITICAL_SECTION m_csDrawLock;

        //For secondary display device
        BOOL            m_bEnableSPrimarySurface;
        // Pointer to surface that is currently being shown
        DDIPUSurf       *m_pActiveSurface;
        
        // For Display update
        HANDLE          m_hDisplayUpdateThread; // Handle to thread for Display Update

        // For TV-out mode
        int             m_iCgmsaMode;           // CGMS-A Mode (CGMSA_MODE_NTSC, CGMSA_MODE_PAL)
        int             m_iDisplayFrequency;    // Display Output Frequency needed to distinguish those modes with 
                                                // the same resolution but different frequency, like
                                                // 720P60, 720P50, 1080i30, 1080i25, etc.
        
        // Global rotation variable
        BOOL            m_bRotationSupported;

        // For software cursor
        BOOL            m_CursorDisabled;
        BOOL            m_CursorVisible;
        BOOL            m_CursorForcedOff ;
        RECTL           m_CursorRect;
        POINTL          m_CursorSize;
        POINTL          m_CursorHotspot;
        BOOL            m_CursorAreaLock;
        RECTL           m_LockRect;

        // allocate enough backing store for a 64x64 cursor on a 32bpp (4 bytes per pixel) screen
        UCHAR           m_CursorBackingStore[64 * 64 * 4];
        UCHAR           m_CursorXorShape[64 * 64];
        UCHAR           m_CursorAndShape[64 * 64];

        // For power handling
        CEDEVICE_POWER_STATE  m_Dx;                    // Current device state
        TCHAR                 m_szDevName[MAX_PATH];   // Device name
        TCHAR                 m_szGuidClass[MAX_PATH]; // Class GUID

        // For Overlay
        UINT32          m_iNumVisibleOverlays;  // Tracks the current number of overlays being shown
        UINT32          m_iLastOverlayUpdateTime; // Used to measure interval since overlay was updated
        CRITICAL_SECTION m_csOverlayShutdown;   // Synchronization for shutting down overlays
        CRITICAL_SECTION m_csOverlayData;       // Synchronization for the PrP overlay data.
        BOOL            m_bDropFramesQuietly;   // BSP configured BOOL to determine whether or not to drop frames quietly
        BOOL            m_bOverlayDisableNotHandled; // Used to handle special case where overlay becomes disabled
                                                     // while in D4 state.
        BOOL            m_bVideoIsInterlaced;    // DrvEscape-configurable interlacing information
        TopFieldSelect  m_TopField;              // DrvEscape-configurable top field for de-interlacing

    private:
        // ddipu_power.cpp
        VOID SetDisplayPower(CEDEVICE_POWER_STATE dx);                  // display hardware power control function
        VIDEO_POWER_STATE PmToVideoPowerState(CEDEVICE_POWER_STATE dx); // video power state to Power Manager device power state conversion functions
        CEDEVICE_POWER_STATE VideoToPmPowerState(VIDEO_POWER_STATE vps);
        BOOL ConvertStringToGuid(LPCTSTR pszGuid, GUID *pGuid);         // String to GUID conversion function
        BOOL AdvertisePowerInterface(HMODULE hInst);

        // ddipu_overlay.cpp
        void SetupOverlayPosition(pOverlaySurf_t pOverlaySurf, LONG x, LONG Y);
        UINT32 GetOverlayIndex(pOverlaySurf_t pOverlaySurf);

        // ddipu.cpp
        BOOL Init(VOID);
        BOOL InitHardware(VOID);
        BOOL SetupVideoMemory(BOOL bRecreatePrimary);
        VOID Cleanup(VOID);

#if defined(USE_C2D_ROUTINES)
        typedef SCODE (GPE::*GPEBltFn)(GPEBltParms*);
        typedef SCODE (GPE::*GPELineFn)(GPELineParms*);

        // BLT methods through C2D API
        SCODE VidmemToVidmemBlt(GPEBltParms *bltParms);
        SCODE SysmemToVidmemBlt(GPEBltParms* bltParms);
        SCODE SolidColorFill(GPEBltParms* bltParms);
        SCODE PatternFill(GPEBltParms* bltParms);
        SCODE SolidLine(GPELineParms* lineParms);
        SCODE DrawLineDiag(GPELineParms* lineParms);
        SCODE DrawLineHorzVert(GPELineParms* lineParms);
        SCODE DrawLine(GPELineParms* lineParms, 
                       C2D_POINT* startPoint, 
                       C2D_POINT* endPoint,
                       C2D_RECT*  lineRect);
        SCODE DrawLine(GPESurf*  dst,
                       RECTL*     prclClip,
                       COLOR      solidColor,
                       C2D_POINT* p0,
                       C2D_POINT* p1,
                       BOOL       bRotate);

        SCODE       CopyToVideoMemory(GPESurf* src, DDIPUSurf** dst);
        void        GetBrushOffset(GPEBltParms* bltParms, int& x, int& y);
        COLOR       GetConvertedColor(GPEBltParms* bltParms, COLOR index);
        
        void        RectlToC2DRect(RECTL* rect, C2D_RECT& c2dRect);
        bool        IsAMirroredRect(RECTL* rect);    
        bool        IsSimpleCopyBlt(GPEBltParms* bltParms);
        bool        IsBltSupported(GPEBltParms* bltParms);    
        void        AdvancedSourceCheck(GPEBltParms * pBltParms);

        C2D_STATUS  SetBltParms(GPEBltParms* bltParms);        
        C2D_STATUS  SetSurfRotation(GPESurf* src, GPESurf* dst);    
        C2D_STATUS  SetBltDestination(GPESurf* dst, PRECTL dstRect, PRECTL clipRect);
        C2D_STATUS  SetBltSource(GPESurf* src, PRECTL srcRect);
        
        SCODE C2DStatusToScode(C2D_STATUS status)
        {
            switch (status)
            {
                case C2D_STATUS_OK:             return S_OK;
                case C2D_STATUS_FAILURE:        return E_FAIL;
                case C2D_STATUS_NOT_SUPPORTED:  return E_NOTIMPL;
                case C2D_STATUS_OUT_OF_MEMORY:  return E_OUTOFMEMORY;
                case C2D_STATUS_INVALID_PARAM:  return E_INVALIDARG;
                default:                        return E_FAIL;
            }
        }

        SCODE DrawGradientRect(TRIVERTEX* upperLeft, 
                               TRIVERTEX* lowerRight,
                               ULONG mode,
                               GPESurf* dst,
                               PRECTL extentsRect,
                               POINTL* ditherOrg,
                               CLIPOBJ* clipObj);
    
        SCODE DrawGradientSimple(GPESurf* dst, 
                                PRECTL dstRect, 
                                PRECTL clipRect,
                                unsigned int fgColor, 
                                unsigned int bgColor, 
                                ULONG mode);

        SCODE DrawGradientClip(GPESurf* dst, 
                               PRECTL dstRect, 
                               PRECTL clipRect, 
                               unsigned int fgColor, 
                               unsigned int bgColor, 
                               ULONG mode);

        unsigned int InterpolateColor(int point,
                                      int start, 
                                      unsigned int startColor, 
                                      int end, 
                                      unsigned int endColor);
#endif //#if defined(USE_C2D_ROUTINES)

    public:
        // ddipu.cpp
        DDIPU(BOOL *pbRet);
        ~DDIPU(VOID);

        SCODE   SetMode(int modeId, HPALETTE *pPalette);
        SCODE   GetModeInfo(GPEMode *pMode, int modeNo);
        SCODE   GetModeInfoEx(GPEModeEx *pModeEx, int modeNo);
        int     NumModes(VOID);
        VOID    GetVideoMemoryInfo(PULONG pMemoryBase, PULONG pVideoMemorySize);
        int     GetBpp(VOID);
        BOOL    IsFlipBusy(VOID);

        // ddipu_line.cpp
        SCODE   Line(GPELineParms *pLineParms, EGPEPhase phase);
        SCODE   WrappedEmulatedLine(GPELineParms *pLineParms);

        // ddipu_blt.cpp
        SCODE   BltPrepare(GPEBltParms *pBltParms);
        SCODE   WrapEmulatedBlt(GPEBltParms *pBltParms);
        SCODE   BltComplete(GPEBltParms *pBltParms);

        // ddipu_surf.cpp
#if defined(USE_C2D_ROUTINES)
        SCODE   AllocSurface(GPESurf **ppSurf, int width, int height, int stride, unsigned int virtAddr, unsigned int physAddr, EGPEFormat format, C2D_COLORFORMAT c2dFormat);
#endif

        SCODE   AllocSurface(GPESurf **ppSurf, int width, int height, EGPEFormat format, int surfaceFlags);
        SCODE   AllocSurface(DDGPESurf **ppSurf, int width, int height, EGPEFormat format, EDDGPEPixelFormat pixelFormat, int surfaceFlags);
        SCODE   DetectPixelFormat(DWORD dwCaps, DDPIXELFORMAT* pDDPF, EGPEFormat* pFormat, EDDGPEPixelFormat* pPixelFormat);
        VOID    SetVisibleSurface( GPESurf *pSurf, BOOL bWaitForVBlank = FALSE);
        DWORD   Flip(LPDDHAL_FLIPDATA pd);
        DDIPUSurf *  PrimarySurface2();

        // ddipu_misc.cpp
        ULONG   DrvEscapeGAPI(ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut);
        ULONG   DrvEscape(SURFOBJ *pso, ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut);
        int     GetGameXInfo(ULONG iEsc, ULONG cjIn, void *pvIn, ULONG cjOut, void *pvOut);
        SCODE   SetPalette(const PALETTEENTRY *src,unsigned short firstEntry,unsigned short numEntries )
        {
            // Remove-W4: Warning C4100 workaround
            UNREFERENCED_PARAMETER(src);
            UNREFERENCED_PARAMETER(firstEntry);
            UNREFERENCED_PARAMETER(numEntries);
            return S_OK;
        }
        VOID    WaitForNotBusy();
        VOID    WaitForVBlank();
        int     IsBusy(VOID);
        int     InVBlank();
        DWORD   WaitForVerticalBlank(LPDDHAL_WAITFORVERTICALBLANKDATA pd);

        void    SetCGMSAMode(int iCgmsaMode);
        
        // ddipu_cursor.cpp
        SCODE   SetPointerShape(GPESurf *pMask, GPESurf *pColorSurf, int xHot, int yHot, int cx, int cy);
        SCODE   MovePointer(int x, int y);
        VOID    CursorOn(VOID);
        VOID    CursorOff(VOID);
        VOID    PointerBltPrepare(GPEBltParms *pBltParms);
        VOID    AreaLock(DDIPUSurf * pSurf, RECT* pRect);
        VOID    AreaUnLock(DDIPUSurf * pSurf);

        // ddipu_rotate.cpp
        int     GetRotateModeFromReg(VOID);
        VOID    SetRotateParams(VOID);
        int     SetRotation(DWORD dwMode, DWORD dwRotation);
        BOOL    IsRotated(VOID);

        // ddipu_overlay.cpp
        DWORD   UpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA pd);
        HRESULT SetVisibleSurfaceOverlay(DDGPESurf * pSurf,DDGPESurf * pSurfCurr,DWORD dwFlags);
        DWORD   SetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA pd);
        HRESULT EnableOverlay();
        HRESULT DisableOverlay(VOID);
        VOID    WaitForNotBusyOverlay(VOID);

        friend VOID buildDDHALInfo( LPDDHALINFO lpddhi, DWORD modeidx );
#if defined(USE_C2D_ROUTINES)
        bool        IsFormatSupported(EGPEFormat format)
        {

#if 0
            return ((format ==  gpe16Bpp) || (format == gpe32Bpp));
#else
    // trying 24bpp gdi acceleration via z430 c2d
            return ((format ==  gpe16Bpp) || (format == gpe32Bpp) || (format == gpe24Bpp));    
#endif
        }
#endif
};

class DDIPUSurf : public DDGPESurf
{
    private:
        IpuBuffer * m_pIpuBuffer;

    public:
        DDIPUSurf(int width, int height, ULONG offset, PVOID pBits, int stride,
                EGPEFormat format, EDDGPEPixelFormat pixelFormat,
                IpuBuffer * pIpuBuffer);

        ~DDIPUSurf(VOID);

        VM_ATTRIBUTE m_vmAttr;

#if defined(USE_C2D_ROUTINES)
        C2D_CONTEXT m_c2dCtx;
        C2D_SURFACE m_c2dSurf;

        bool AssignSurfData(GPESurf* sir);
        C2D_SURFACE GetC2DSurface()
        {
            return m_c2dSurf; 
        }
#endif

        IpuBuffer* GetIpuBuffer()
        {
            return m_pIpuBuffer;
        }
};

#endif //_DDIPU_H
