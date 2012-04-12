//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  display.h
//
//  Header file for Display interface layer functions.
//
//------------------------------------------------------------------------------

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define MAX_VISIBLE_OVERLAYS 2

#define RGB_666_FORMAT              0
#define RGB_565_FORMAT              1
#define RGB_444_FORMAT              2

//--------------------------------------------------------
// Addtional FOURCC codes supported
//--------------------------------------------------------
// Define Custom pixel formats
enum EDDGPEPixelFormat_extension
{
    ddgpePixelFormat_NV12 = ddgpePixelFormat_CustomFormat,
};

//-----------------------------------------------------------------------------
// Types

typedef enum {
    DISPLAY_FLOW_ASYNC_UI = 1,
    DISPLAY_FLOW_ASYNC_VIDEO = 2,
    DISPLAY_FLOW_SYNC = 4,
    DISPLAY_FLOW_ALL = 7,
} DISPLAY_FLOW;

typedef struct overlaySurf{
    UINT32   nBufPhysicalAddr;
    BOOL     bIsShown;     // Is overlay currently being shown
    struct overlaySurf* next;   // Pointer to next overlaySurf element

    // Source overlay surface info
    UINT16   SrcWidth;
    UINT16   SrcHeight;
    UINT32   SrcLineStride;
    RECT     SrcRect;
    EDDGPEPixelFormat SrcPixelFormat;
    BOOL     isInterlaced;
    TopFieldSelect TopField;
    UINT16   SrcBpp;

    // Destination overlay surface info
    UINT16   DstWidth_Orig;
    UINT16   DstHeight_Orig;
    UINT16   DstWidth;
    UINT16   DstHeight;
    UINT32   DstLineStride;
    UINT16   DstBpp;
    UINT16   XOffset;
    UINT16   YOffset;

    // Screen orientation info
    UINT32   iRotate;
    BOOL     isUpsideDown;


    // Blending info
    UINT32   Transparency;  // 255 indicate totally transparent, ie. not displayed on LCD screen
                            // 0 indicate totally opaque, ie. overlay on LCD screen
    UINT32   ColorKeyMask;  // 0 indicate no color key is used
    UINT32   ColorKeyPlane;

    BOOL     bGlobalAlpha;   // Global alpha if TRUE, per-pixel alpha if FALSE
}overlaySurf_t, *pOverlaySurf_t;


//------------------------------------------------------------------------------
// Functions

// APIs for retrieving display info
DWORD   DisplayGetNumSupportedModes(void);
BOOL    DisplayGetSupportedModes(GPEMode *);
DWORD   DisplayGetPrimaryMode();
DWORD   DisplayGetPixelDepth(void);
DWORD   DisplayGetVideoPixelDepth(void);
DWORD   DisplayGetVideoMemorySize(void);
DWORD   DisplayGetVideoMemoryBase(void);
BOOL    DisplayIsRotationSupported(void);
BOOL    DisplayEanbleSPrimarySurface(void);


// APIs for enabling/disabling display
BOOL    DisplayInitialize(UINT32, UINT32 *);
void    DisplayEnablePanels(void);
void    DisplayDisablePanels(void);
void    DisplayEnableActiveFlows(BOOL bPowerOn);
void    DisplayDisableActiveFlows(BOOL bPowerOff);
void    DisplayEnableSingleFlow(DISPLAY_FLOW displayFlow);
void    DisplayDisableSingleFlow(DISPLAY_FLOW displayFlow);
void    DisplaySetMode(DWORD, BOOL);


// APIs for updating display
void    DisplaySetSrcBuffer(UINT32 phAddr);
void    DisplaySetSrcBuffer2(UINT32 phAddr);
void    DisplaySetScreenRotation(DWORD dwRotation);
BOOL    DisplaySetGamma(float fGamma);
void    DisplayUpdateInit(LPCRITICAL_SECTION);
void    DisplayUpdate(LPRECT);
void    DisplayAsyncUpdateThread(LPVOID lpParameter);
void    DisplaySecondaryUpdateThread(LPVOID lpParameter);
void    DisplaySetStride(BOOL bIsBuf2, UINT32 stride);


// Misc. APIs
void    DisplayAllocateBuffer(DWORD dwBufSize, IpuBuffer* pNewBuffer);
void    DisplayDeallocateBuffer(IpuBuffer* pIpuBuffer);
void    DisplayWaitForVSync(BOOL bIsSecondarydevice);
BOOL    DisplayIsTVSupported();
BOOL    DisplayIsBusy();
void    DisplayRegPush();
void    DisplayRegPop();
BOOL    DisplayDMIEnable(DISPLAY_GPUID eGPUID, DISPLAY_GPU_COLOR eColorFormat,
                            UINT32 uiStride, DISPLAY_GPU_BUFFERMODE eBufferMode, 
                            void* pBuffer1, void* pBuffer2, void* pBuffer3);
BOOL    DisplayDMIDisable();


// APIs for overlaysurface
void    Display_OverlayInit();
void    Display_OverlayDeInit();
//BOOL    Display_SetOverlayBuffer(UINT32 PhysicalBuf, BOOL bWait);
BOOL    Display_SetOverlayBuffer(UINT32 overlayIndex, BOOL bWait);
void    Display_StopOverlay();
BOOL    Display_DisableOverlay();
//void    Display_SetupOverlay(pOverlaySurfaceParam pOverlaySurfaceOp);
void    Display_SetupOverlay(pOverlaySurf_t *ppOverlayInfo, UINT32 iNumOverlays);
void    Display_StartOverlay();
void    Display_SetupOverlayWindowPos(UINT32 overlayIndex, UINT16 x, UINT16 y);
BOOL    Display_OverlayParamAlign(RECT *pSrcRect, RECT *pDstRect, int iRotate, EDDGPEPixelFormat PixelFormat, int RealWidth, BOOL bPerPixelAlpha);
BOOL    Display_OverlayIsBusy();
UINT32  Display_GetOverlay2SrcBufOffset();
void    Display_Set_DPOverlay(UINT32 phAddr);
void    Display_Reconfigure_Interlaced(UINT32 overlayIndex, BOOL isInterlaced);



// APIs for top-most overlay surface
void    Display_Top_OverlayInit();
void    Display_Top_OverlayDeInit();
BOOL    Display_Top_SetOverlayBuffer(UINT32 overlayIndex, BOOL bWait);
void    Display_Top_StopOverlay();
BOOL    Display_Top_DisableOverlay();
void    Display_Top_SetupOverlay();
void    Display_Top_StartOverlay();
void    Display_Top_SetupOverlayWindowPos(UINT16 x, UINT16 y);
BOOL    Display_Top_OverlayParamAlign(RECT *pSrcRect, RECT *pDstRect, int iRotate, EDDGPEPixelFormat PixelFormat, int RealWidth);
BOOL    Display_Top_OverlayIsBusy();
void    Display_SetupPrP4Overlay(UINT32 overlayIndex);
void    Display_SetupDP4Overlay(pOverlaySurf_t pOverlaySurfaceInfo);



//APIs for non-top overlay surfaces
void    Display_Middle_OverlayInit();
void    Display_Middle_OverlayDeInit();
BOOL    Display_Middle_SetOverlayBuffer(UINT32 iBuf, UINT32 overlayIndex, BOOL bWait);
void    Display_Middle_StopOverlay();
BOOL    Display_Middle_DisableOverlay();
void    Display_Middle_SetupOverlay(pOverlaySurf_t pOverlaySurfaceInfo);
void    Display_Middle_StartOverlay(UINT32 iBuf,BOOL b4SecDisp);
void    Display_Middle_SetupOverlayWindowPos(UINT32 overlayIndex, UINT16 x, UINT16 y, BOOL b4SecDisp);
BOOL    Display_Middle_OverlayIsBusy();
void    Display_Setup2Overlay();
void    Display_Start2Overlay();





// APIs for XEC
void    Display_SetVideoOffEvent(void);
void    Display_SetUIEvent(void);
BOOL    Display_XECInit(void);
void    Display_SendVideoFrame(DDGPESurf * pSurf, RECT SrcRect);
BOOL    Display_SetCSC(ULONG cjIn, PVOID pvIn);
BOOL    Display_GetCSC(ULONG cjOut, PVOID pvOut);

#ifdef __cplusplus
}
#endif

#endif // __DISPLAY_H__
