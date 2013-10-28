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
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------


#ifndef _DRIVERS_DISPLAY_LCDC_LCDCCLASS_H
#define _DRIVERS_DISPLAY_LCDC_LCDCCLASS_H

#define USE_V1_CODE  // use v1 version of Line and BltPrepare

//------------------------------------------------------------------------------
// CLASS DEFINITIONS
//------------------------------------------------------------------------------
#ifdef __cplusplus

class LcdcSurf : public GPESurf
{
    private:
        Node2D          *m_pNode2D;
#ifdef OEM_ALLOC_MEM    
        void *      m_pLcdcSurfBuffer;
#endif  
        
    public:
                        LcdcSurf(int width, 
                                 int height, 
                                 int offset, 
                                 void *pBits, 
                                 int stride, 
                                 EGPEFormat format, 
                                 Node2D *pNode);
#ifdef OEM_ALLOC_MEM
                 LcdcSurf(int width,
                    int        height,
                    EGPEFormat format );
#endif
                        ~LcdcSurf(void);
        INT32           Top(void)  { return m_pNode2D->Top(); }
        INT32           Left(void) { return m_pNode2D->Left(); }
};


class LcdcClass : public GPE
{
    private:
        GPEMode       m_ModeInfo;                 //local mode info
        UINT32        m_nPhysicalVideoMem;
        UINT32        m_nVideoMemorySize;       //Size in bytes of video RAM total
        UINT8*        m_pLinearVideoMem;
        UINT32        m_nPhysicalBlankVideoMem; // physical add for blank frame buffer
        UINT8*      m_pBlankVideoMem;   // virtual address for blank frame buffer
        UINT32      m_VideoFrameSize;  // Vidio size of a frame
        LCDC_CTX    m_LcdcCtx;
        CSP_LCDC_REGS    *m_pLCDC;

        Node2D*       m_p2DVideoMemory;     //Base entry representing all video memory
        UINT32        m_nScreenStride;      //Stride of 2d memory in bytes
        UINT32        m_RMCRtemp;           //save RMCR status

        CEDEVICE_POWER_STATE  m_Dx; // Current device state
        TCHAR         m_szDevName[MAX_PATH];            // Device name
        TCHAR         m_szGuidClass[MAX_PATH];      // Class GUID
        
        HANDLE        m_hGraphicWindowHdl;
        BOOL          m_bGraphicWindowRunning;
        BOOL          m_bGraphicWindowFlipped;

        DWORD           m_colorDepth;
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



        ULONG LcdcDrvEscape(    SURFOBJ *pso,
                                ULONG    iEsc,
                                ULONG    cjIn,
                                PVOID    pvIn,
                                ULONG    cjOut,
                                PVOID    pvOut);

        // display hardware power control function
        void SetDisplayPower(CEDEVICE_POWER_STATE dx);

        // video power state to Power Manager device power state conversion functions
        VIDEO_POWER_STATE PmToVideoPowerState(CEDEVICE_POWER_STATE dx);
        CEDEVICE_POWER_STATE VideoToPmPowerState(VIDEO_POWER_STATE vps);
        
        // String to GUID conversion function
        BOOL ConvertStringToGuid(LPCTSTR pszGuid, GUID *pGuid);
        BOOL AdvertisePowerInterface(HMODULE hInst);

        // graphic window operation function
        BOOL EnableGraphicWindow(pLcdcGraphicWindowOP_t pOp);
        BOOL DisableGraphicWindow(pLcdcGraphicWindowOP_t pOp);
        BOOL FlipGraphicWindow(pLcdcGraphicWindowOP_t pOp);
        BOOL GetGWTransparency(pLcdcGraphicWindowOP_t pOp);
        BOOL SetGWTransparency(pLcdcGraphicWindowOP_t pOp);
        
        VOID DumpRegster(VOID);

    public:
        LcdcClass(void);
        ~LcdcClass(void);

        SCODE SetMode(int modeId, HPALETTE *pPalette);
        SCODE GetModeInfo(GPEMode *pMode, int modeNo );
        int   NumModes(void);
        void  PowerHandler(BOOL bOff);
        int   InVBlank(void);
        SCODE SetPalette(   const PALETTEENTRY *src, 
                            unsigned short firstEntry, 
                            unsigned short numEntries );
        BOOL  ContrastControl(  ULONG cmd,
                                ULONG *pValue);     
        SCODE SetPointerShape(GPESurf *pMask,
                              GPESurf *pColorSurf,
                              int xHot,
                              int yHot,
                              int cx,
                              int cy);
        SCODE MovePointer(  int x,
                            int y );
        SCODE SetCursorColor( UINT8 red, 
                              UINT8 green, 
                              UINT8 blue);
#ifdef USE_V1_CODE
        SCODE WrappedEmulatedLine(GPELineParms *lineParameters);
#endif // USE_V1_CODE
        SCODE Line( GPELineParms *pLineParms,
                    EGPEPhase phase = gpeSingle);
        SCODE BltPrepare (    GPEBltParms *pBltParms );
        SCODE BltComplete(    GPEBltParms *pBltParms );
        SCODE AllocSurface(   GPESurf **ppSurf,
                          int width,
                          int height,
                          EGPEFormat format,
                          int surfaceFlags );

    virtual ULONG DrvEscape(    SURFOBJ *pso,
                                    ULONG    iEsc,
                                    ULONG    cjIn,
                                    PVOID    pvIn,
                                    ULONG    cjOut,
                                    PVOID    pvOut);

    int GetRotateModeFromReg();
    void SetRotateParms();
    LONG DynRotate(int angle);

    VOID CursorOn(void);
    VOID CursorOff(void);

};

#endif


#endif  /* _DRIVERS_DISPLAY_LCDC_LCDCCLASS_H */

//------------------------------------------------------------------------------
// END OF FILE
//------------------------------------------------------------------------------
