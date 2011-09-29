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
//
//  File:  DDLCDC.cpp
//
//  Implementation of class DDLCDC.
//
//------------------------------------------------------------------------------

#include "precomp.h"
#include <syspal.h>    // for 8Bpp we use the natural palette 

#define PALETTE_2BPP_SIZE               4
RGBQUAD _rgb2bpp[PALETTE_2BPP_SIZE] =
{
    { 0x00, 0x00, 0x00, 0 },    /* Black */      \
    { 0x80, 0x80, 0x80, 0 },    /* Dark Grey */  \
    { 0xc0, 0xc0, 0xc0, 0 },    /* Light Grey */ \
    { 0xff, 0xff, 0xff, 0 }     /* White */      \
};

#define PALETTE_4BPP_SIZE               16
#if 0
// Colored 4BPP color palette.
RGBQUAD _rgb4bpp[PALETTE_4BPP_SIZE] =
{
    { 0x00, 0x00, 0x00, 0 },    /* black */        \
    { 0x80, 0x00, 0x00, 0 },    /* darkred */     \
    { 0xFF, 0x00, 0x00, 0 },    /* red */     \
    { 0x00, 0x80, 0x00, 0 },    /* darkgreen */     \
    { 0x80, 0x80, 0x00, 0 },    /* darkyellow */     \
    { 0x00, 0xFF, 0x00, 0 },    /* green */     \
    { 0xFF, 0xFF, 0x00, 0 },    /* yellow */     \
    { 0x00, 0x00, 0x80, 0 },    /* dark blue */     \
    { 0x80, 0x00, 0x80, 0 },    /* dark pruple */     \
    { 0x00, 0x80, 0x80, 0 },    /* darkteal */     \
    { 0x80, 0x80, 0x80, 0 },    /* darkgray */     \
    { 0xC0, 0xC0, 0xC0, 0 },    /* ligthgray */     \
    { 0x00, 0x00, 0xFF, 0 },    /* blue */     \
    { 0xFF, 0x00, 0xFF, 0 },    /* purple */     \
    { 0x00, 0xFF, 0xFF, 0 },    /* teal */     \
    { 0xFF, 0xFF, 0xFF, 0 }     /* white */     \
};
#else
// Greyscalebpp palette. 
RGBQUAD _rgb4bpp[PALETTE_4BPP_SIZE] =
{
    { 0x00, 0x00, 0x00, 0 },   \
    { 0x10, 0x10, 0x10, 0 },   \
    { 0x20, 0x20, 0x20, 0 },   \
    { 0x30, 0x30, 0x30, 0 },   \
    { 0x40, 0x40, 0x40, 0 },   \
    { 0x50, 0x50, 0x50, 0 },   \
    { 0x60, 0x60, 0x60, 0 },   \
    { 0x70, 0x70, 0x70, 0 },   \
    { 0x80, 0x80, 0x80, 0 },   \
    { 0x90, 0x90, 0x90, 0 },   \
    { 0xa0, 0xa0, 0xa0, 0 },   \
    { 0xb0, 0xb0, 0xb0, 0 },   \
    { 0xc0, 0xc0, 0xc0, 0 },   \
    { 0xd0, 0xd0, 0xd0, 0 },   \
    { 0xe0, 0xe0, 0xe0, 0 },   \
    { 0xFF, 0xFF, 0xFF, 0 }    \
};
#endif


//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------
extern BOOL   BSPGetModeDescriptor(int modeid,PLCDC_MODE_DESCRIPTOR pContext);
extern BOOL   BSPInitLCDC(VOID);
extern BOOL   BSPDeinitLCDC(PLCDC_MODE_DESCRIPTOR pContext);
extern void   BSPTurnOnDisplay(PLCDC_MODE_DESCRIPTOR pContext);
extern void   BSPTurnOffDisplay(PLCDC_MODE_DESCRIPTOR pContext);

extern GPEMode * BSPGetModes(VOID);

extern ULONG  BSPGetModeNum(void);

extern UINT32 LCDCGetBaseRegAddr(void);
extern DWORD  LCDCGetIRQ(void);

extern BOOL   LCDCEnableClock(BOOL bEnable);

extern ULONG  LCDCGetRefClk(PLCDC_MODE_DESCRIPTOR pContext);

extern DWORD BSPGetVideoMemorySize(PLCDC_MODE_DESCRIPTOR pModeDesc);
extern BOOL BSPGetCacheMode();

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Exported functions
//------------------------------------------------------------------------------
// This prototype avoids problems exporting from .lib
BOOL APIENTRY GPEEnableDriver(ULONG engineVersion, ULONG cj, DRVENABLEDATA *data,
                              PENGCALLBACKS  engineCallbacks);

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

INSTANTIATE_GPE_ZONES((GPE_ZONE_ERROR|GPE_ZONE_INIT|GPE_ZONE_WARNING),"MXDDLcdc","unused1","unused2");

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
static DWORD g_dwlcdcSysintr;

static GPE *gGPE = (GPE *) NULL;

static EGPEFormat eFormat[] =
{
    gpe1Bpp,
    gpe2Bpp,
    gpe4Bpp,
    gpe8Bpp,
    gpe16Bpp,
    gpe24Bpp,
    gpe32Bpp,
};


static ULONG BitMasks[][3] =
{
    { 0, 0, 0 },
    { 0xF800, 0x07E0, 0x001F },
    { 0xFF0000, 0x00FF00, 0x0000FF },
    { 0x00FF0000, 0x0000FF00, 0x000000FF }
};



//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------
DWORD MXDDLcdcIntr(MXDDLcdc *pClass);



//------------------------------------------------------------------------------
// Functions

//------------------------------------------------------------------------------
//
// Function: DrvEnableDriver
//
// This function is the display driver.
//
//------------------------------------------------------------------------------
BOOL APIENTRY DrvEnableDriver(ULONG engineVersion, ULONG cj, DRVENABLEDATA *data,
                              PENGCALLBACKS  engineCallbacks)
{
    return GPEEnableDriver(engineVersion, cj, data, engineCallbacks);
}

//------------------------------------------------------------------------------
//
// Function: DrvGetMasks
//
// This function retrieves the color masks for the display device's current
// mode. The ClearType and anti-aliased font libraries use this function.
//
// Parameters:
//      dhpdev
//          [in] Handle to the display device for which to retrieve color
//          mask information.
//
// Returns:
//      A pointer to three consecutive ULONG values. Each ULONG represents
//      the mask for a particular color component, red, green, or blue. For
//      example, a RGB565 based display returns (0xf800, 0x07e0, 0x001f).
//
//------------------------------------------------------------------------------
PULONG APIENTRY DrvGetMasks(DHPDEV dhpdev)
{
    UNREFERENCED_PARAMETER(dhpdev);

    if (gGPE == NULL)
        return(BitMasks[0]);

    switch (((MXDDLcdc *)gGPE)->GetBpp())
    {
        // Only modes with BPP of 16 and above are using masks. all other modes are palletized.       
        case 16:
        case 24:
        case 32:
            {
            int nBPP = ((MXDDLcdc *)gGPE)->GetBpp()/8 - 1;
            return(BitMasks[nBPP]);
            }
    }

    return(BitMasks[0]);
}


//------------------------------------------------------------------------------
//
// Function: GetGPE
//
// Main entry point for a GPE-compliant driver.
//
// Parameters:
//      None.
//
// Returns:
//      GPE class pointer.
//
//------------------------------------------------------------------------------
GPE *GetGPE(VOID)
{
    if (!gGPE)
    {
        BOOL result = FALSE;
        gGPE = new MXDDLcdc(&result);

        if(!result)
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc: Failed to get GPE instance!\r\n")));
            if(gGPE)
            {
                delete gGPE;
                gGPE = NULL;
            }
        }
    }

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MXDDLcdc: GetGPE(0x%08x)\r\n"), gGPE));
    return gGPE;
}


//------------------------------------------------------------------------------
// CLASS MEMBER FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  FUNCTION:     MXDDLcdc
//
//  DESCRIPTION:  Constructor of MXDDLcdc class
//
// Parameters:
//      pbRet
//          [in] Pointer to return value.  TRUE if success, FALSE if failure.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
MXDDLcdc::MXDDLcdc(BOOL * pbRet)
{
    BOOL result = TRUE;

    // Initialze MXDDLcdc class on software perspective
    if(!Init())
    {
        result = FALSE;
        goto _done;
    }

    // Setup LCDC of i.MX processor
    if(!InitHardware())
    {
        result = FALSE;
        goto _done;
    }

    // Notify the system that we are a power-manageable device
    m_Dx = D0;
    if(!AdvertisePowerInterface(g_hmodDisplayDll))
    {
        result = FALSE;
        goto _done;
    }

    // Pre-initialize the graphic window operator for overlay
    m_pOverlaySurfaceOp = (pOverlaySurf_t)malloc(sizeof(overlaySurf_t));
    if(m_pOverlaySurfaceOp)
    {
        memset(m_pOverlaySurfaceOp, 0x00, sizeof(overlaySurf_t));
    }
    else
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc Init: failed to allocate memory for graphic windower operator, error(%d)\r\n"), GetLastError()));
        result = FALSE;
        goto _done;
    }

_done:
    if(!result)
        Cleanup();

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MXDDLcdc: class construction %s!!\r\n"), result ? L"succeeds" : L"fails"));
    *pbRet = result;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     ~MXDDLcdc
//
//  DESCRIPTION:  Destructor of ~MXDDLcdc
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
MXDDLcdc::~MXDDLcdc()
{
    Cleanup();
}

//------------------------------------------------------------------------------
//
// Function: SetMode
//
// This method executes to enable the device driver
// based on the Graphics Primitive Engine (GPE)
// and to request a specific mode for the display.
//
// Parameters:
//      modeID
//          [in] Mode number to set.
//
//      pPalette
//          [in/out] Handle of palette.
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE MXDDLcdc::SetMode(int modeNo, HPALETTE * pPalette)
{
    SCODE result = S_OK;
    BOOL  bModeGot = FALSE;

    // Pre-setup display mode in DDGPE engine
    m_pModeEx = &m_ModeInfoEx;
    m_pMode = &m_ModeInfoEx.modeInfo;
    memset(m_pModeEx, 0, sizeof(GPEModeEx));
    m_pModeEx->dwSize = sizeof(GPEModeEx);
    m_pModeEx->dwVersion = GPEMODEEX_CURRENTVERSION;

    // Destroy previous surface objects if any
    if(m_pPrimarySurface != NULL)
    {
        if(!FreeVideoMemory())
        {
            ERRORMSG(1, (L"Failed to FreeVideoMemory"));
            result = S_FALSE;
            goto _done;
        }
    }

    // Setup video buffer
    if(!AllocVideoMemory())
    {
        ERRORMSG(1, (L"Failed to AllocateVideoMemory"));
        result = S_FALSE;
        goto _done;
    }

    // Get the selected mode from specific layer
    bModeGot = BSPGetModeDescriptor(modeNo, &m_ModeDesc);

    // setup local mode info structure
    *m_pMode  = (m_ModeDesc.GPEModeInfo);

    if(!bModeGot)
    {
        ERRORMSG(1, (L"Failed to get the mode descriptor. Initialization FAILED."));
        result = S_FALSE;
        goto _done;
    }

    m_dwPhysicalModeID = m_pMode->modeId;

    // Setup surface alignment as required by the LCDC of i.MX for 4 bytes
    if (m_pMode->Bpp >= 8)
    {
        m_nSurfacePixelAlign = 4 / (m_pMode->Bpp / 8);
    }
    else
    {
        m_nSurfacePixelAlign = 4 * ( 8 / m_pMode->Bpp);
    }
    
    // Sanity Check for screen width
    if(m_pMode->width & (m_nSurfacePixelAlign - 1))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc Init: Screen width (%d) is not aligned for %d bytes!\r\n"), m_pMode->width, m_nSurfacePixelAlign));
        result = S_FALSE;
        goto _done;
    }

    // The line stride of i.MX LCDC is 32-bits (4 bytes) aligned
    m_nScreenStride = (  (ULONG)(((float)m_pMode->width * ((float)m_pMode->Bpp / 8.0f)) + 3) >> 2) << 2;

    // Configure palette
    int nBPP = m_pMode->Bpp/8 - 1;
    if (nBPP<0)
    {
        nBPP = 0;
    }

    if (pPalette)
    {
        switch (m_pMode->Bpp)
        {
            case    2:
                *pPalette = EngCreatePalette (PAL_INDEXED,
                                             PALETTE_2BPP_SIZE,
                                             (ULONG*)_rgb2bpp,
                                             0,
                                             0,
                                             0);
                break;
            case    4:
                *pPalette = EngCreatePalette (PAL_INDEXED,
                                             PALETTE_4BPP_SIZE,
                                             (ULONG*)_rgb4bpp,
                                             0,
                                             0,
                                             0);
                SetRGBQuadPalette(_rgb4bpp,0,sizeof(_rgb4bpp)/sizeof(_rgb4bpp[0]));
                break;
            case    8:
                *pPalette = EngCreatePalette (PAL_INDEXED,
                                             PALETTE_SIZE,
                                             (ULONG*)_rgbIdentity,
                                             0,
                                             0,
                                             0);
                break;

            case    16:
            case    24:
            case    32:
                *pPalette = EngCreatePalette (PAL_BITFIELDS,
                                             0,
                                             NULL,
                                             BitMasks[nBPP][0],
                                             BitMasks[nBPP][1],
                                             BitMasks[nBPP][2]);
                break;
            default:
                break;
        }
    }

    // Setup other DDGPE mode informations
    switch (m_pMode->Bpp)
    {
        case    1:
        case    2:
        case    4:
        case    8:
        case    16:
        case    24:
        case    32:
            m_pModeEx->ePixelFormat = EGPEFormatToEDDGPEPixelFormat[m_pMode->format];
            m_pModeEx->lPitch = m_nScreenStride;
            m_pModeEx->dwRBitMask = BitMasks[nBPP][0];
            m_pModeEx->dwGBitMask = BitMasks[nBPP][1];
            m_pModeEx->dwBBitMask = BitMasks[nBPP][2];
            break;

        default:
            DEBUGMSG(GPE_ZONE_ERROR,(TEXT("MXDDLcdc Init: Invalid BPP value passed to driver - Bpp = %d\r\n"), m_pMode->Bpp));
            m_pMode->format = gpeUndefined;
            result = S_FALSE;
            goto _done;
    }

    // Setup Rotation
    m_nScreenHeightSave = m_pMode->height;
    m_nScreenWidthSave = m_pMode->width;
    m_iRotate = GetRotateModeFromReg();
    SetRotateParams();

    m_nScreenWidth  = m_pMode->width;
    m_nScreenHeight = m_pMode->height;
    m_nScreenBpp    = m_pMode->Bpp;

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MXDDLcdc Init: Current display mode:\r\n")));
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("\t\tm_nScreenWidth:  %d\r\n"), m_nScreenWidth));
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("\t\tm_nScreenHeight: %d\r\n"), m_nScreenHeight));
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("\t\tm_nScreenStride: %d\r\n"), m_nScreenStride));
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("\t\tm_nScreenBpp:    %d\r\n"), m_nScreenBpp));

    // Start display driver!!
    // Create primary surface and blank surface
    if (!m_pPrimarySurface)
    {
        result = AllocSurface((DDGPESurf **) &m_pPrimarySurface, m_nScreenWidthSave,
                m_nScreenHeightSave, m_pMode->format, m_pModeEx->ePixelFormat,
                GPE_REQUIRE_VIDEO_MEMORY);
        
        if(result != S_OK)
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc SetMode: Couldn't allocate primary surface\r\n")));
            result = S_FALSE;
            goto _done;
        }
    }

    // Configure LCDC of i.MX processor
    if(!SetModeHardware(m_pMode->modeId))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc SetMode: fail to initialize hardware!\r\n"), GetLastError()));
        result = S_FALSE;
        goto _done;
    }

    DynRotate(m_iRotate);

    if(m_pPrimarySurface->OffsetInVideoMemory())
        SetVisibleSurface((GPESurf *) m_pPrimarySurface);

    m_pPrimarySurface->SetRotation(m_nScreenWidth, m_nScreenHeight, m_iRotate);

_done:
    return result;
}


//------------------------------------------------------------------------------
//
// Function: GetModeInfo
//
// This method populates a GPEMode structure with data
// for the requested mode.
//
// Parameters:
//      pMode
//          [out]  Pointer to a GPEMode structure.
//
//      modeNo
//          [in]   Integer specifying the mode to return information about.
//          This no is not the mode id.
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE MXDDLcdc::GetModeInfo(GPEMode * pMode, int modeNo)
{
    if(modeNo >= NumModes())
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc GetModeInfo: wrong mode (%d)!!\r\n"), modeNo));
        return E_INVALIDARG;
    }

    *pMode = m_ModesInfo[modeNo];

    return S_OK;
}


//------------------------------------------------------------------------------
//
// Function: GetModeInfoEx
//
// This method populates a GPEMode structure with data
// for the requested mode.
//
// Parameters:
//      pMode
//          [out]  Pointer to a GPEModeEx structure.
//
//      modeNo
//          [in]   Integer specifying the mode to return information about.
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE MXDDLcdc::GetModeInfoEx(GPEModeEx * pModeEx, int modeNo)
{
    if(modeNo >= NumModes())
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc GetModeInfoEx: wrong mode (%d)!!\r\n"), modeNo));
        return E_INVALIDARG;
    }

    *pModeEx = *m_pModeEx;

    return S_OK;
}


//------------------------------------------------------------------------------
//
// Function: NumModes
//
// This method returns the number of
// display modes supported by a driver.
//
// Parameters:
//      None.
//
// Returns:
//      The number of supported display modes
//
//------------------------------------------------------------------------------
int MXDDLcdc::NumModes(VOID)
{
    return BSPGetModeNum();
}


//------------------------------------------------------------------------------
//
// Function: GetPhysicalVideoMemory
//
// This method retrieves the information of video memory.
//
// NOTE: The VIRTUAL address should be passed back as defined by Microsoft.
//
// Parameters:
//      pPhysicalMemoryBase
//          [out]  Pointer to physical memory.
//
//      pVideoMemorySize
//          [out]   Pointer to video memory size.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID MXDDLcdc::GetPhysicalVideoMemory(PULONG pPhysicalMemoryBase, PULONG pVideoMemorySize)
{
    // The return value should be virtual address
    *pPhysicalMemoryBase = (ULONG) m_pLAW;
    *pVideoMemorySize = m_nVideoMemorySize; 
}


//------------------------------------------------------------------------------
//
// Function: GetBpp
//
// Return the Bpp of the current monitor.
//
// Parameters:
//      None.
//
// Returns:
//      BPP of the current monitor.
//
//------------------------------------------------------------------------------
int MXDDLcdc::GetBpp(VOID)
{
    return m_nScreenBpp;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     Init
//
//  DESCRIPTION:  Software perspective initialization of MXDDLcdc class
//
// Parameters:
//      None.
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//------------------------------------------------------------------------------
BOOL MXDDLcdc::Init(VOID)
{
    BOOL result = TRUE;
    BOOL bModeGot = FALSE;

    m_pPrimarySurface = NULL;
    m_pBlankSurface   = NULL;
    m_pVisibleOverlay = NULL;

    m_pVideoMemoryHeap = NULL;

    m_nPreIntrStatus = 0;
    m_nMask = 0;     // for flicker

    // Low level layer initialisation
    BSPInitLCDC();

    // Get all supported modes
    m_ModesInfo = BSPGetModes();

    // Pre-setup display mode in DDGPE engine
    m_pModeEx = &m_ModeInfoEx;
    m_pMode = &m_ModeInfoEx.modeInfo;
    memset(m_pModeEx, 0, sizeof(GPEModeEx));
    m_pModeEx->dwSize = sizeof(GPEModeEx);
    m_pModeEx->dwVersion = GPEMODEEX_CURRENTVERSION;

    // Get the selected mode from specific layer
    bModeGot = BSPGetModeDescriptor(m_pMode->modeId,&m_ModeDesc);

    if(!bModeGot)
    {
        // No suitable display mode found, return an error
        
        ERRORMSG(1, (TEXT("MXDDLcdc Init: Default display mode is used!\r\n")));
        result = FALSE;
    }
    else
    {
        // setup local mode info structure
        *m_pMode  = (m_ModeDesc.GPEModeInfo);
       
        m_nScreenWidth  = m_pMode->width;
        m_nScreenHeight = m_pMode->height;
        m_nScreenBpp    = m_pMode->Bpp;

        DEBUGMSG (1, (TEXT("mode 0x%x\r\n"), m_pMode->modeId));
    }

    return result;
}


BOOL MXDDLcdc::ReInit(VOID)
{
    BOOL bModeGot;
    BOOL result = TRUE;
    GPEMode* pNewMode;

    pNewMode = &(m_ModeDesc.GPEModeInfo);

    // If the new mode is the same as the old one, just leave the function
    if(pNewMode->modeId == m_pMode->modeId) return TRUE;

    // Get the selected mode from specific layer
    bModeGot = BSPGetModeDescriptor(pNewMode->modeId, &m_ModeDesc);

    BSPTurnOffDisplay(&m_ModeDesc);

    // If the new mode resolution is the same as the old one, just reconfigure the hardware
    if((pNewMode->height == m_pMode->height) &&
       (pNewMode->width  == m_pMode->width)  &&
       (pNewMode->Bpp    == m_pMode->Bpp))
    {
        SetModeHardware(pNewMode->modeId);
    }
    else // If the resolution is different, let the upper layers do it
    {
        DEVMODE dmDispMode;
        dmDispMode.dmSize = sizeof(DEVMODE);
        dmDispMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY | DM_BITSPERPEL;
        dmDispMode.dmPelsHeight = pNewMode->height;
        dmDispMode.dmPelsWidth = pNewMode->width;
        dmDispMode.dmDisplayFrequency = pNewMode->frequency;
        dmDispMode.dmDisplayFrequency = pNewMode->Bpp;
        ChangeDisplaySettingsEx(NULL,&dmDispMode,NULL,CDS_RESET,NULL);
    }

    // setup local mode info structure
    *m_pMode  = *pNewMode;

    RECT rcWorkRect;
    rcWorkRect.left   = 0;
    rcWorkRect.top    = 0;
    rcWorkRect.right  = m_pMode->width;
    rcWorkRect.bottom = m_pMode->height;

    // The following line informs the OS that the working rectangle has changed.
    // TVOUT and the LCD may have different working rectangles depending on the design of the hardware.
    // Without this call, the UI not will not be drawn correctly.
    SystemParametersInfo(SPI_SETWORKAREA, 0, &rcWorkRect, SPIF_SENDCHANGE);

    return result;
}

//------------------------------------------------------------------------------
//
// Function: InitHardware
//
// Initialize the hardware resources for the driver.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE           successful
//      FALSE          failed
//
//------------------------------------------------------------------------------
BOOL MXDDLcdc::InitHardware(VOID)
{
    BOOL result = TRUE;
    PHYSICAL_ADDRESS phyAddr;

    //Mem maps the LCDC module space for user access
    phyAddr.QuadPart = LCDCGetBaseRegAddr();
    m_pLcdcReg = (CSP_LCDC_REGS *)MmMapIoSpace(phyAddr, sizeof(CSP_LCDC_REGS), FALSE);
    if (m_pLcdcReg  ==  NULL)   
    {
        DEBUGMSG (GPE_ZONE_ERROR, (TEXT("LcdcClass: VirtualAlloc failed!\r\n")) );
        result = FALSE;
        goto _done;
    }

    // Initialze LCDC interrupt
    m_hSyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == m_hSyncEvent)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc InitHardware: failed to create the intr event, error[%d]!\r\n"), GetLastError()));
        result = FALSE;
        goto _done;
    }

    // Create an event for properly exiting the thread
    m_hExitSyncThread = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(!m_hExitSyncThread)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc InitHardware: failed to create event for exiting SyncThread, error[%d]\r\n"), GetLastError()));
        result = FALSE;
        goto _done;
    }

    // Get LCDC IRQ
    DWORD irq = LCDCGetIRQ();
    // Request an associated systintr
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
                        &g_dwlcdcSysintr, sizeof(DWORD), NULL)){
                DEBUGMSG(GPE_ZONE_ERROR, (_T("Failed to obtain sysintr value!\r\n")));
        return 0;
    }
    
    // Initialize interrupt
    if(!InterruptInitialize(g_dwlcdcSysintr, m_hSyncEvent, NULL, NULL))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc InitHardware: fail to initialize the interrupt, error[%d]!\r\n"), GetLastError()));
        result = FALSE;
        goto _done;
    }

    // Create the IST (thread)
    m_fStopIntrProc = FALSE;
    m_hSyncThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MXDDLcdcIntr, this, 0, NULL);
    if(NULL == m_hSyncThread)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc InitHardware: fail to initiate the interrupt thread, error[%d]!\r\n"), GetLastError()));
        result = FALSE;
        goto _done;
    }

    // Initialze a critical section to protect interrupt status flag update violation
    InitializeCriticalSection(&m_CriticalSection);

_done:

    return result;
}


//------------------------------------------------------------------------------
//
// Function: AllocVideoMemory
//
// Setup the video memory buffer and other related buffers.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//------------------------------------------------------------------------------
BOOL MXDDLcdc::AllocVideoMemory(VOID)
{
    //SCODE sc;
    BOOL result = TRUE;
    DWORD minMemorySize;
    BOOL bCachedWT;

    // Compute the framebuffer minimum size.
    // The video memory should be at least three frame size:
      //    - one for primary surface // primary
      //    - one for a blank surface used when OS goes to power suspension // blank
      //    - one for general use in DirectDraw applications
    minMemorySize = 3 * (DWORD)(m_ModeDesc.GPEModeInfo.width * m_ModeDesc.GPEModeInfo.height * ((float)m_nScreenBpp / 8.0f));

    // Get the BSP specfic value.
    // So if user want a biggest screen, it can use the m_ModeDesc.VideoMemorySize variable
    // to specified a size.
    m_nVideoMemorySize = BSPGetVideoMemorySize(&m_ModeDesc);

    // Take the biggest size between the min and the user specified
    if(minMemorySize > m_nVideoMemorySize)
    {
        m_nVideoMemorySize = minMemorySize;
    }


    if(!m_pVideoMemory)
    {
        // The buffer is 4Mbytes aligned because the LCDC controller need a framebuffer 
        // not sperated between two pages of 4MBytes
        // So we allocate a buffer with the correct size, with all needed write, 4MBytes aligned
        m_pVideoMemory = (PUCHAR)AllocPhysMem(m_nVideoMemorySize,       // Frame buffer size
                                              PAGE_EXECUTE_READWRITE,   // All needed rigth
                                              0x3FFFFF,                 // 4MByte aligned
                                              0,                        // Reserved
                                              (ULONG *) &m_nLAWPhysical);
    }

    // Check if virtual mapping failed
    if (!m_pVideoMemory)
    {
        DEBUGMSG(GPE_ZONE_ERROR,
                 (TEXT("%s(): MmMapIoSpace failed!\r\n"), __WFUNCTION__));
        result = FALSE;
        goto _done;
    }

    //Allocate Frame Buffer Space in shared memory
    m_hLAWMapping = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        m_nVideoMemorySize,
        NULL);

    if (m_hLAWMapping != NULL) 
    {
        m_pLAW = (unsigned char *)MapViewOfFile(
            m_hLAWMapping,
            FILE_MAP_WRITE,
            0,
            0,
            0);
    } 
    else 
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc AllocVideoMemory: failed at MapViewOfFile(), error[%d]\r\n"), m_nVideoMemorySize, GetLastError()));
        result = FALSE;
        goto _done;
    }

    result = VirtualCopy(
        m_pLAW,
        (LPVOID)(m_nLAWPhysical >> 8),
        m_nVideoMemorySize,
        PAGE_READWRITE|PAGE_NOCACHE|PAGE_PHYSICAL);

    if(!result)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc AllocVideoMemory: failed at VirtualCopy(), error[%d]\r\n"), m_nVideoMemorySize, GetLastError()));
        result = FALSE;
        goto _done;
    }

    // BSP_VID_MEM_CACHE_WRITETHROUGH defaults to TRUE,
    // i.e. Using cached, write-through mode
    // to provides performance benefit.
    bCachedWT = BSPGetCacheMode();

    if (bCachedWT)
    {
        // Update the Framebuffer to be cached, write-through
        VirtualSetAttributes(m_pLAW, m_nVideoMemorySize, 0x8, 0xC, NULL);
    }
    else
    {
        // Update the Framebuffer to be non-cached, bufferable
        VirtualSetAttributes(m_pLAW, m_nVideoMemorySize, 0x4, 0xC, NULL);
    }

    // Create video memory heap
    if (!m_pVideoMemoryHeap)
        m_pVideoMemoryHeap = new SurfaceHeap(m_nVideoMemorySize, NULL, NULL, NULL);

_done:
    
    if(result)
    {
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("MXDDLcdc AllocVideoMemory: \r\n")));
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("\tVirtual address mapped access window: 0x%08x\r\n"), m_pLAW));
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("\tVirtual address of video memory:      0x%08x\r\n"), m_pVideoMemory));
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("\tPhysical address of video memory:     0x%08x\r\n"), m_nLAWPhysical)); 
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("\tSize:                                 0x%08x\r\n"), m_nVideoMemorySize));
    }

    return result;
}

//------------------------------------------------------------------------------
//
// Function: FreeVideoMemory
//
// Free the video memory buffer and other related buffers.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//------------------------------------------------------------------------------
BOOL MXDDLcdc::FreeVideoMemory(VOID)
{
    // Destroy surface objects
    if(m_pPrimarySurface)
    {
        delete m_pPrimarySurface;
        m_pPrimarySurface = NULL;
    }

    // Release the shared video memory
    if (m_pLAW)
    {
        UnmapViewOfFile(m_pLAW);
        VirtualFree(m_pLAW, 0, MEM_RELEASE);
        m_pLAW = NULL;
    }

    if (m_hLAWMapping)
    {
        CloseHandle(m_hLAWMapping);
        m_hLAWMapping = NULL;
    }

    if(m_pVideoMemory)
    {
        FreePhysMem(m_pVideoMemory);
        m_pVideoMemory = NULL;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:   SetRGBQuadPalette
//
//  Setup the plalette in hardware controller
//
//  Returns:
//      TRUE: successful
//      FALSE: failed
//
//------------------------------------------------------------------------------
SCODE MXDDLcdc::SetRGBQuadPalette(const RGBQUAD *src,unsigned short firstEntry,unsigned short numEntries )
{
    int i;
    UINT32 * baseReg = (UINT32*)(((UINT8*)m_pLcdcReg) + 0x800);

    for(i=firstEntry;i<firstEntry+numEntries;i++)
    {
        WORD w;
        w = 0;
        w |= (src[i].rgbRed >> 2)   << 12;
        w |= (src[i].rgbGreen >> 2) << 6;
        w |= (src[i].rgbBlue >> 2)  << 0;
        baseReg[i] = w;        
    }
    return S_OK;
}

//------------------------------------------------------------------------------
//
//  Function:   SetRGBQuadPalette
//
//  Setup the plalette in hardware controller
//
//  Returns:
//      TRUE: successful
//      FALSE: failed
//
//------------------------------------------------------------------------------
SCODE MXDDLcdc::SetPalette(const PALETTEENTRY *src,unsigned short firstEntry,unsigned short numEntries )
{
    int i;
    UINT32 * baseReg = (UINT32*)(((UINT8*)m_pLcdcReg) + 0x800);

    for(i=firstEntry;i<firstEntry+numEntries;i++)
    {
        WORD w;
        w = 0;
        w |= (src[i].peRed >> 2)   << 12;
        w |= (src[i].peGreen >> 2) << 6;
        w |= (src[i].peBlue >> 2)  << 0;
        baseReg[i] = w;        
    }
    return S_OK;
}

//------------------------------------------------------------------------------
//
//  Functions:  SetModeHardware
//
//  Description:
//      Setup hardware LCDC for current display mode
//
//  Parameters:
//      modeNo
//          [in] The mode number.
//
//  Returns:
//      TRUE: successful
//      FALSE: failed
//
//------------------------------------------------------------------------------
BOOL MXDDLcdc::SetModeHardware(int modeNo)
{
    BOOL		result = FALSE;
    DWORD		dHtotal, dVtotal, dPCR;
    UINT32		uLCDRefClk, uLCDPixClk;
	UINT32		uTemp;
	UINT32		uPCD;

    UNREFERENCED_PARAMETER(modeNo);

    // Disable LCDC clock
    LCDCEnableClock(FALSE);

    // LCDC Refresh Mode Control Register
    CLRREG32(&m_pLcdcReg->RMCR, CSP_BITFMASK(LCDC_RMCR_SELF_REF));

    // LCDC Screen Start Address Register
    OUTREG32(&m_pLcdcReg->SSAR, 
              (m_nLAWPhysical));

    // LCDC Size Register
    OUTREG32(&m_pLcdcReg->SR,              
              (CSP_BITFVAL(LCDC_SR_XMAX, ((m_ModeDesc.GPEModeInfo).width / 16))|
              CSP_BITFVAL(LCDC_SR_YMAX, ((m_ModeDesc.GPEModeInfo).height))));

    // LCDC Virtual Page Width Register
    OUTREG32(&m_pLcdcReg->VPWR, 
        ((m_ModeDesc.GPEModeInfo).width / (32/(m_ModeDesc.GPEModeInfo).Bpp))); // the number of 32-bit words

    // LCDC Cursor Position Register
    // Default: disable cursor
    OUTREG32(&m_pLcdcReg->CPR, 
                (CSP_BITFVAL(LCDC_CPR_OP, LCDC_CPR_OP_DISABLE) |
                CSP_BITFVAL(LCDC_CPR_CC, LCDC_CPR_CC_DISABLED)));

    // LCDC Cursor Width Height and Blink Register
    // Default: disable cursor
    OUTREG32(&m_pLcdcReg->CWHBR, 
                  (CSP_BITFVAL(LCDC_CWHBR_BK_EN, LCDC_CWHBR_BK_EN_DISABLE) |
                  CSP_BITFVAL(LCDC_CWHBR_CW, LCDC_CWHBR_CW_CURSOR_DISABLED) |
                  CSP_BITFVAL(LCDC_CWHBR_CH, LCDC_CWHBR_CH_CURSOR_DISABLED) |
                  CSP_BITFVAL(LCDC_CWHBR_BD, LCDC_CWHBR_BD_MAX_DIV)));

    // LCDC Color Cursor Mapping Register
    OUTREG32(&m_pLcdcReg->CCMR, 0);

    // Get the LCD Reference clock
    // PGAL : Caution, for TV OUT mode the reference clock are 266400000 / PERDIV3
    uLCDRefClk = LCDCGetRefClk(&m_ModeDesc);

    dHtotal = (m_ModeDesc.GPEModeInfo).width + m_ModeDesc.Hwidth + m_ModeDesc.Hwait1
        + m_ModeDesc.Hwait2;
    dVtotal = (m_ModeDesc.GPEModeInfo).height + m_ModeDesc.Vwidth + m_ModeDesc.Vwait1
        + m_ModeDesc.Vwait2;
    uLCDPixClk = dHtotal * dVtotal * (m_ModeDesc.GPEModeInfo).frequency;

    // Configure the PCR register using panel specfic information fill in board specific layer
    dPCR = m_ModeDesc.PCR_CFG.uPCRCfg;

    // Configure pixel divider value
    // dPCR |= CSP_BITFVAL(LCDC_PCR_PCD, LCDC_PCD_VALUE(uLCDRefClk, uLCDPixClk));
	//
	// CS&ZHL JUN-29-2011: new algorithm
	//
	uPCD = uLCDRefClk / uLCDPixClk;
	if(uPCD > 1)
	{
		uTemp = (uLCDRefClk * 10) / uLCDPixClk;
		if((uTemp % 10) < 5)
		{
			uPCD--;
		}
	}
    dPCR |= CSP_BITFVAL(LCDC_PCR_PCD, uPCD);

    switch((m_ModeDesc.GPEModeInfo).Bpp)
    {
        case 1:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_1BPP);
            break;
        case 2:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_2BPP);
            break;
        case 4:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_4BPP);
            break;
        case 8:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_8BPP);
            break;
        case 12:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_12BPP);
            break;
        case 16:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_16BPP);
            break;
        case 18:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_18BPP);
            break;
        default:
            goto cleanup;
    }
    OUTREG32(&m_pLcdcReg->PCR, dPCR);

    // LCDC Horizontal Configuration Register
    OUTREG32(&m_pLcdcReg->HCR, 
              (CSP_BITFVAL(LCDC_HCR_H_WIDTH, (m_ModeDesc.Hwidth - 1))|
              CSP_BITFVAL(LCDC_HCR_H_WAIT_1, (m_ModeDesc.Hwait1 - 1))|
              CSP_BITFVAL(LCDC_HCR_H_WAIT_2, (m_ModeDesc.Hwait2 - 3))));

    // LCDC Vertical Configuration Register
    OUTREG32(&m_pLcdcReg->VCR, 
             (CSP_BITFVAL(LCDC_VCR_V_WIDTH, (m_ModeDesc.Vwidth))|
             CSP_BITFVAL(LCDC_VCR_V_WAIT_1, (m_ModeDesc.Vwait1))|
             CSP_BITFVAL(LCDC_VCR_V_WAIT_2, (m_ModeDesc.Vwait2))));

    
    // LCDC Panning Offset Register
    OUTREG32(&m_pLcdcReg->POR, 0);

    // LCDC Sharp Configuration Register
    OUTREG32(&m_pLcdcReg->SCR, 
          (CSP_BITFVAL(LCDC_SCR_GRAY1, 0) |
          CSP_BITFVAL(LCDC_SCR_GRAY2, 0) |
          CSP_BITFVAL(LCDC_SCR_REV_TOGGLE_DELAY, 3) |
          CSP_BITFVAL(LCDC_SCR_CLS_RISE_DELAY, 18) |
          CSP_BITFVAL(LCDC_SCR_PS_RISE_DELAY, 0))); // 0001 = 2 LSCLK period

    // Check if a value has already been assigned for the backlight
    // If pwm not already enabled by backligth driver, enabled it and set a default value
    if ((INREG32(&m_pLcdcReg->PCCR) & CSP_BITFMASK(LCDC_PCCR_CC_EN)) == 0)
    {
        // LCDC PWM Contrast Control Register
        OUTREG32(&m_pLcdcReg->PCCR,
              (CSP_BITFVAL(LCDC_PCCR_PW, (LCDC_PCCR_PW_MAX / 2)) |
               CSP_BITFVAL(LCDC_PCCR_CC_EN, LCDC_PCCR_CC_EN_ENABLE) |
               CSP_BITFVAL(LCDC_PCCR_SCR, LCDC_PCCR_SCR_LCDCLK) |
               CSP_BITFVAL(LCDC_PCCR_LDMSK, LCDC_PCCR_LDMSK_DISABLE) |
               CSP_BITFVAL(LCDC_PCCR_CLS_HI_WIDTH, 169)
              ));
    }

    // LCDC DMA Control Register
    OUTREG32(&m_pLcdcReg->DCR,
              (CSP_BITFVAL(LCDC_DCR_BURST, LCDC_DCR_BURST_DYNAMIC)|
              CSP_BITFVAL(LCDC_DCR_HM, (0x04))|   // DMA High Mark
              CSP_BITFVAL(LCDC_DCR_TM, (0x68)))); // DMA Trigger Mark

    // LCDC Interrupt Configuration Register
    OUTREG32(&m_pLcdcReg->ICR,
          (CSP_BITFVAL(LCDC_ICR_GW_INT_CON, LCDC_ICR_GW_INT_CON_END) |
          CSP_BITFVAL(LCDC_ICR_INTSYN, LCDC_ICR_INTSYN_PANEL) |
          CSP_BITFVAL(LCDC_ICR_INTCON, LCDC_ICR_INTCON_BOF)));

    // LCDC Interrupt Enable Register
    OUTREG32(&m_pLcdcReg->IER, 0);

    // LCDC Graphic Window

    // LCDC Graphic Window Start Address Register
    OUTREG32(&m_pLcdcReg->GWSAR, 
              (m_nLAWPhysical));

    // LCDC Graphic Window DMA Control Register
    OUTREG32(&m_pLcdcReg->GWDCR, 
              (CSP_BITFVAL(LCDC_GWDCR_GWBT, LCDC_GWDCR_GWBT_DYNAMIC)|
              CSP_BITFVAL(LCDC_GWDCR_GWHM, 0x02)|
              CSP_BITFVAL(LCDC_GWDCR_GWTM, 0x10)));


    // Graphic window at first time can only be enabled while the HCLK to the LCDC is disabled. 
    // Once enabled it can subsequently be disabled and enabled without turning off the HCLK.
    // So we need to enable and then disable the graphic window at hardware init part(configlcdc),
    // then at next time to enable graphic window, the HCLK to LCDC does not need to be disabled, and the flicker (due to disabling of HCLK to LCDC) is avoided.
    {
        // Enable graphic window
        // LCDC Graphic Window Size Register
        OUTREG32(&m_pLcdcReg->GWSR, 
                 (CSP_BITFVAL(LCDC_GWSR_GWW, ((m_ModeDesc.GPEModeInfo).width >> 4))|
                  CSP_BITFVAL(LCDC_GWSR_GWH, (m_ModeDesc.GPEModeInfo).height)));

        // LCDC Graphic Window Virtual Page Width Register
        OUTREG32(&m_pLcdcReg->GWVPWR, 
                ((m_ModeDesc.GPEModeInfo).width / (32/(m_ModeDesc.GPEModeInfo).Bpp))); // the number of 32-bit words

        // LCDC Graphic Window Position Register
        OUTREG32(&m_pLcdcReg->GWPR, 
                  (CSP_BITFVAL(LCDC_GWPR_GWXP, 0)|
                  CSP_BITFVAL(LCDC_GWPR_GWYP, 0)));

        // LCDC Graphic Window Panning Offset Register
        OUTREG32(&m_pLcdcReg->GWPOR, 
                  16);

        // LCDC Graphic Window Control Registers
        OUTREG32(&m_pLcdcReg->GWCR, 
                  (CSP_BITFVAL(LCDC_GWCR_GWAV, LCDC_GWCR_GWAV_OPAQUE)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKE, LCDC_GWCR_GWCKE_DISABLE)|
                  CSP_BITFVAL(LCDC_GWCR_GWE, LCDC_GWCR_GWE_ENABLE)|
                  CSP_BITFVAL(LCDC_GWCR_GW_RVS, LCDC_GWCR_GW_RVS_NORMAL)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKR, 0)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKG, 0)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKB, 0)));
  
        // Disable graphic window
        CLRREG32(&m_pLcdcReg->GWCR, CSP_BITFMASK(LCDC_GWCR_GWAV));

        // LCDC Graphic Window Size Register
        OUTREG32(&m_pLcdcReg->GWSR, 
                  (CSP_BITFVAL(LCDC_GWSR_GWW, (16 >> 4))|
                  CSP_BITFVAL(LCDC_GWSR_GWH, 16)));
        Sleep(100);

        CLRREG32(&m_pLcdcReg->GWCR, CSP_BITFMASK(LCDC_GWCR_GWE));
    }

    LCDCEnableClock(TRUE);

    BSPTurnOnDisplay(&m_ModeDesc);

    result = TRUE;

cleanup:
    return result;
}


//------------------------------------------------------------------------------
//
// Function: Cleanup
//
// Release all resouces for either failed initialization
// or driver unloading.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID MXDDLcdc::Cleanup(VOID)
{
    m_nMask = 0;   // for flicker

    BSPTurnOffDisplay(&m_ModeDesc);

    BSPDeinitLCDC(&m_ModeDesc);

    // Release LCDC register map
    if(m_pLcdcReg)
    {
        // disable LCDC
        INSREG32BF(&m_pLcdcReg->RMCR, LCDC_RMCR_SELF_REF, LCDC_RMCR_SELF_REF_DISABLE);

        MmUnmapIoSpace(m_pLcdcReg, sizeof(CSP_LCDC_REGS));
        m_pLcdcReg  =   NULL;
    }

    LCDCEnableClock(FALSE);

    if(m_pBlankSurface)
    {
        delete m_pBlankSurface;
        m_pBlankSurface = NULL;
    }

    // Release the overlay operator
    if(m_pOverlaySurfaceOp)
    {
        free(m_pOverlaySurfaceOp);
        m_pOverlaySurfaceOp = NULL;
    }

    // Release allocated Video memory
    FreeVideoMemory();

    // Disable interrupt source and release interrupt event
    InterruptDisable(g_dwlcdcSysintr);

    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &g_dwlcdcSysintr, sizeof(DWORD), NULL, 0, NULL);
    g_dwlcdcSysintr = (DWORD)SYSINTR_UNDEFINED;
    
    if(m_hSyncEvent)
    {
        CloseHandle(m_hSyncEvent);
        m_hSyncEvent= NULL;
    }

    // Terminate interrupt thread
    if(m_hSyncThread)
    {
        // Set SyncThread end
        SetEvent ( m_hExitSyncThread ); 
        // Wait for SyncThread end
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hSyncThread,INFINITE))
            RETAILMSG(0,(TEXT("Wait for SyncThread end")));

        CloseHandle(m_hSyncThread);
        m_hSyncThread = NULL;
    }

    CloseHandle(m_hExitSyncThread);
    m_hExitSyncThread = NULL;

    DeleteCriticalSection(&m_CriticalSection);
}


//------------------------------------------------------------------------------
//
//  Function:   IntrProc
//
//  This function is interrupt handler.
//
//  Returns:        
//      A double word value for ExitThread()
//
//------------------------------------------------------------------------------
DWORD MXDDLcdc::IntrProc(VOID)
{
    
    while(!m_fStopIntrProc)
    {
        if (WaitForSingleObject(m_hExitSyncThread, 0) != WAIT_TIMEOUT)
        {
            // Exiting thread
            ExitThread(1);
        }

        if (WaitForSingleObject(m_hSyncEvent, INFINITE) != WAIT_OBJECT_0)
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MXDDLcdc IntrProc: interrupt error [%d]\r\n"), GetLastError()));
            continue;
        }

        EnterCriticalSection(&m_CriticalSection);
        m_nPreIntrStatus = m_pLcdcReg->ISR;
        LeaveCriticalSection(&m_CriticalSection);

        InterruptDone(g_dwlcdcSysintr);
    }

    return 0;
}

//------------------------------------------------------------------------------
//
//  Function:       MXDDLcdcIntr
//
//  This function is entry point of the interrupt thread.
//
//  Parameters: 
//      pClass
//          [in] The MXDDLCDC class instance
//
//  Returns:
//      A double word value for ExitThread()
//
//------------------------------------------------------------------------------
DWORD MXDDLcdcIntr(MXDDLcdc * pClass)
{
    pClass->IntrProc();

    // We shouldn't come to here
    return 0;
}
