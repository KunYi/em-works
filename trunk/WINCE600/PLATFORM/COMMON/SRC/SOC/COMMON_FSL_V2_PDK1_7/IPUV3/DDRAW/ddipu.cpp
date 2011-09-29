//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  DDIPU.cpp
//
//  Implementation of class DDIPU.
//
//------------------------------------------------------------------------------

#include "precomp.h"

#define dim(x)          (sizeof(x) / sizeof(x[0]))


//------------------------------------------------------------------------------
// External Functions
#if defined(USE_C2D_ROUTINES)
extern DWORD BSPGetC2DInfoFromRegistry();
#endif

// This prototype avoids problems exporting from .lib
BOOL APIENTRY GPEEnableDriver(ULONG engineVersion, ULONG cj,
                              DRVENABLEDATA *data,
                              PENGCALLBACKS  engineCallbacks);

//------------------------------------------------------------------------------
// External Variables

extern "C" PANEL_INFO  g_PanelArray[];


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables

static DDGPE *gGPE = (DDGPE *) NULL;

static DWORD g_dwMonitorsExpected = 1;
static TCHAR g_szBaseInstance[256] = {0};

INSTANTIATE_GPE_ZONES((GPE_ZONE_ERROR |
                       GPE_ZONE_INIT  |
                       GPE_ZONE_WARNING),
                      "IPU Display Driver", "unused1", "unused2");

static EGPEFormat eFormat[] =
{
    gpe8Bpp,
    gpe16Bpp,
    gpe24Bpp,
    gpe32Bpp,
};

static EDDGPEPixelFormat ePixelFormat[4] =
{
    ddgpePixelFormat_8bpp,
    ddgpePixelFormat_565,
    ddgpePixelFormat_8880,  // not supported, just for possible future expansion
    ddgpePixelFormat_8888,  // not supported, just for possible future expansion
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


BOOL APIENTRY DrvEnableDriver(ULONG engineVersion, ULONG cj, DRVENABLEDATA *data,
                              PENGCALLBACKS  engineCallbacks)
{
    pfnGetGPEPerCard = NULL; // We are not MULTIMON-compliant.
    return GPEEnableDriver(engineVersion, cj, data, engineCallbacks);
}


//------------------------------------------------------------------------------
// DisplayInit
//
// GWES will invoke this routine once prior to making any other calls into the
// driver.  This routine needs to save its instance path information and return
// TRUE.  If it returns FALSE, GWES will abort the display initialization.
//
// Inputs:        pszInstance = Instance name of SMI driver
//                g_dwNumMonitors = Number of monitors expected to be supported
//
// Returns:        See description above
//
//------------------------------------------------------------------------------
BOOL APIENTRY DisplayInit(LPCTSTR pszInstance, DWORD dwNumMonitors)
{
    BOOL result = FALSE;

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("DDIPU: Display instance '%s', ")
                             TEXT("Num monitors = %d\r\n"),
        pszInstance != NULL ? pszInstance : TEXT("<NULL>"), dwNumMonitors));

    // if we expect to be a PCI device, then there must be an instance
    if(pszInstance == NULL)
    {
        pszInstance = L"System\\GDI\\Drivers";
    }

    // save our first instance path (without instance specifier) along with
    // the number of monitors we expect to support
    StringCchCopy((STRSAFE_LPWSTR)g_szBaseInstance, dim(g_szBaseInstance),
                  pszInstance);

    g_dwMonitorsExpected = dwNumMonitors;
    if (!gGPE)
    {
        gGPE = new DDIPU(&result);

        if(!result)
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("%s: Failed to get GPE ")
                                      TEXT("instance!\r\n"), __WFUNCTION__));
            gGPE = NULL;
        }
    }

    return result;
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
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dhpdev);

    if (gGPE == NULL)
        return(BitMasks[0]);

    int nBPP = ((DDIPU *)gGPE)->GetBpp()/8 - 1;
    switch (((DDIPU *)gGPE)->GetBpp())
    {
        case 8:
        case 16:
        case 24:
        case 32:
            return(BitMasks[nBPP]);
            break;
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
    return gGPE;
}

//*********************************************************************
// CLASS MEMBER FUNCTIONS
//*********************************************************************
//------------------------------------------------------------------------------
//
// Function: DDIPU
//
// Constructor of DDIPU class.
//
// Parameters:
//      pbRet
//          [in] Pointer to return value.  TRUE if success, FALSE if failure.
//
// Returns:
//
//------------------------------------------------------------------------------
DDIPU::DDIPU(BOOL * pbRet)
{
    BOOL result = TRUE;

    // Initialize IPU class on software perspective
    if(!Init())
    {
        result = FALSE;
        goto _done;
    }

    // Setup video buffer
    if(!SetupVideoMemory(TRUE))
    {
        result = FALSE;
        goto _done;
    }

    m_pPrimarySurface->SetRotation(m_nScreenWidth, m_nScreenHeight, m_iRotate);
    DisplaySetScreenRotation(m_iRotate);

    // Set up IPU
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

_done:
    if(!result)
        Cleanup();

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("DDIPU: class construction %s!!\r\n"), result ? L"succeeds" : L"fails"));
    *pbRet = result;
}


//------------------------------------------------------------------------------
//
// Function: ~DDIPU
//
// Destructor of DDIPU class.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
DDIPU::~DDIPU()
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
SCODE DDIPU::SetMode(int modeNo, HPALETTE * pPalette)
{
    static BOOL bFirstTimeInit = TRUE;
    EnterCriticalSection(&m_csDrawLock);
    
    // m_pMode maybe released by DrvDisablePDEV().
    if(m_pMode == NULL)
    {
        m_pModeEx = &m_ModeInfoEx;
        m_pMode   = &m_ModeInfoEx.modeInfo;
        m_nCurrentDisplayMode = -1;
    }

    // Ensure that requested mode is supported by the platform
    if (modeNo > (int)m_nNumSupportedModes)
    {
        // Unsupported mode requested
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("%s: SetMode: TV modes unsupported\r\n"), __WFUNCTION__));
        LeaveCriticalSection(&m_csDrawLock);
        return S_FALSE;
    }

    if ((m_SupportedModes[modeNo].frequency == m_iDisplayFrequency) && (m_nCurrentDisplayMode == modeNo) && !bFirstTimeInit)
    {
        // Nothing has changed, so we don't need to actually make a mode change.  Just return w/ success.
        LeaveCriticalSection(&m_csDrawLock);
        return S_OK;
    }

    // TV 720P and 1080p/i modes have same resolution with different frequency. but the current 
    // ChangeDisplaySettingsEx() can't support "Frequency" parameter. We use ExtEscape() to set 
    // the exact desired frequency (stored in m_iDisplayFrequency).
    // Now we check if we need to adjust the modeNo according with the m_iDisplayFrequency and new mode resolution
    if (m_SupportedModes[modeNo].frequency != m_iDisplayFrequency)
    {
        int targetWidth, targetHeight;
        targetWidth = m_SupportedModes[modeNo].width;
        targetHeight = m_SupportedModes[modeNo].height;

        BOOL bFoundMatch = FALSE;

        // new mode frequency doesn't match frequency set via SET_OUTPUT_FREQUENCY.
        // We must iterate through supported modes to find the correct mode to use.
        for (DWORD i = 0; i < m_nNumSupportedModes; i++)
        {
            if ((m_SupportedModes[i].width == targetWidth) &&
               (m_SupportedModes[i].height == targetHeight) &&
               (m_SupportedModes[i].frequency == m_iDisplayFrequency))
            {
                // We found a match.  We will use the matched mode number to configure display
                DEBUGMSG(GPE_ZONE_INIT, (TEXT("%s: display mode is %d, adjusted display mode is %d!\r\n"), __WFUNCTION__, modeNo, m_iDisplayFrequency));
                modeNo = i;
                bFoundMatch = TRUE;
            }
        }

        if ((bFoundMatch != TRUE) && (m_nCurrentDisplayMode == modeNo))
        {
            // New frequency + current width + current height doesn't match any supported modes, 
            // plus mode number is not new,  so we don't need to actually make a mode change.  Just return w/ success.
            LeaveCriticalSection(&m_csDrawLock);
            return S_OK;
        }
    }

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("%s: Got display mode, %d!\r\n"), __WFUNCTION__, m_SupportedModes[modeNo].modeId));
    if(m_nCurrentDisplayMode != modeNo)
    {
        memcpy(m_pMode, &(m_SupportedModes[modeNo]), sizeof(GPEMode));
        m_nScreenHeightSave = m_pMode->height;
        m_nScreenWidthSave  = m_pMode->width;
        m_nScreenWidth  = m_pMode->width;
        m_nScreenHeight = m_pMode->height;
    }
    m_nScreenStride = ((m_nScreenWidthSave * (m_nScreenBpp / 8) + 3) >> 2) << 2;
    m_pModeEx->lPitch = m_nScreenStride;
    m_dwPhysicalModeID = m_pMode->modeId;

    int nBPP = m_nScreenBpp/8 - 1;
    if (pPalette)
    {
        switch (m_nScreenBpp)
        {
            case    8:
                // TODO:
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
        }
    }

    //Only run one time.
    InitHardware();
    
    // We have changed the display mode, so now we must query 
    // Display Interface Layer to update our info about primary and secondary modes
    m_nPrimaryDisplayMode = DisplayGetPrimaryMode();
        
    DisplaySetGamma(m_fGammaValue);  //initialzie gamma setting

    // Recreate primary if resolution changed
    BOOL bRecreatePrimary = ((m_SupportedModes[m_nCurrentDisplayMode].width == m_pMode->width) && (m_SupportedModes[m_nCurrentDisplayMode].height == m_pMode->height)) ? FALSE : TRUE;

    m_nCurrentDisplayMode = modeNo; 

    // We now support changing modes without changing the resolution (e.g. 800x600 DVI to 800x600 VGA).
    // We support these changes by going around the WinCE mode change software.  Therefore, we
    // don't want to delete and reallocate surfaces, since we will get out of sync with surface
    // pointers held by WinCE graphics layer software (leading to crashes).  So, if there is no
    // resolution change, call SetupVideoMemory with no primary surface recreation.
    SetupVideoMemory(bRecreatePrimary);

    //SetMode needs new stride value which will be set in function SetupVideoMemory().

    // IPUv3 doesn't have enough bandwidth to support dual display 
    // when one of panel's resolution is larger than 1024x768
    if(m_nScreenWidth > 1024) 
    {
        DisplaySetMode(modeNo,FALSE);
    }
    else
    {
        DisplaySetMode(modeNo,m_bEnableSPrimarySurface);
    }


    if(m_pPrimarySurface->OffsetInVideoMemory())
    {
        SetVisibleSurface((GPESurf *) m_pPrimarySurface);
    }

    //Rotation must be intialized, otherwise it will cause ddraw cetk failure.
    // Set rotation for the primary surface
    m_pPrimarySurface->SetRotation(m_pMode->width, m_pMode->height, m_iRotate);
    SetRotation(modeNo, m_iRotate);

    if (m_bEnableSPrimarySurface && (m_nCurrentDisplayMode != m_nPrimaryDisplayMode)
        && (m_nScreenWidth <= 1024))
    {
        m_pPrimarySurface2->SetRotation(m_SupportedModes[m_nPrimaryDisplayMode].width,
                                        m_SupportedModes[m_nPrimaryDisplayMode].height, 
                                        DMDO_0);
        DisplaySetSrcBuffer2(m_pPrimarySurface2->OffsetInVideoMemory());
    }

    LeaveCriticalSection(&m_csDrawLock);

    m_rcWorkRect.left   = 0;
    m_rcWorkRect.top    = 0;
    m_rcWorkRect.right  = m_nScreenWidth;
    m_rcWorkRect.bottom = m_nScreenHeight;

    //We can't call SystemParametersInfo() in SetMode(), as it will raise an redraw event which may cause GDI to 
    // draw something on a not ready surface and cause data abort during resolution change.

    bFirstTimeInit = FALSE;

    return S_OK;
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
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE DDIPU::GetModeInfo(GPEMode * pMode, int modeNo)
{
    if((modeNo >= NumModes())||modeNo < 0)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("IPU GetModeInfo: wrong mode (%d)!!\r\n"), modeNo));
        return E_INVALIDARG;
    }
    *pMode = m_SupportedModes[modeNo];

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
SCODE DDIPU::GetModeInfoEx(GPEModeEx * pModeEx, int modeNo)
{
    if(modeNo >= NumModes())
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("IPU GetModeInfoEx: wrong mode (%d)!!\r\n"), modeNo));
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
int DDIPU::NumModes(VOID)
{
    return m_nNumSupportedModes;
}


//------------------------------------------------------------------------------
//
// Function: GetVideoMemoryInfo
//
// This method retrieves the information of video memory.
//
// NOTE: The VIRTUAL address should be passed back as defined by Microsoft.
//
// Parameters:
//      pMemoryBase
//          [out]  Pointer to video memory base address.
//
//      pVideoMemorySize
//          [out]   Pointer to video memory size.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID DDIPU::GetVideoMemoryInfo(PULONG pMemoryBase, PULONG pVideoMemorySize)
{
    // The return value should be virtual address
    *pMemoryBase = m_nVideoMemoryBase;
    *pVideoMemorySize = m_nVideoMemorySize;
}


//----------------------------------------------------------------------
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
//----------------------------------------------------------------------
int DDIPU::GetBpp(VOID)
{
    return m_nScreenBpp;
}

//----------------------------------------------------------------------
//
// Function: Init
//
// Software perspective initialization of IPU class.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//----------------------------------------------------------------------
BOOL DDIPU::Init(VOID)
{
    BOOL result = TRUE;
    int  nBPP;

    // Pre-Initialization
    InitializeCriticalSection(&m_csDrawLock);

    InitializeCriticalSection(&m_csOverlayData);

    InitializeCriticalSection(&m_csOverlayShutdown);

    m_fGammaValue = 1;
    m_pPrimarySurface = NULL;
    m_pActiveSurface = NULL;
    m_pPrimarySurface2 = NULL;
    memset(&m_Overlay2SurfaceInfo,0,sizeof(overlaySurf_t));
    m_Overlay2SurfaceInfo.nBufPhysicalAddr = 0xffffffff;
    m_bSetMode        = FALSE;
    m_bOverlayDisableNotHandled = FALSE;
    m_bVideoIsInterlaced = FALSE;
    m_TopField = TopFieldSelect_Even;
 
    // Pre-setup display mode in DDGPE engine
    m_pModeEx = &m_ModeInfoEx;
    m_pMode   = &m_ModeInfoEx.modeInfo;
    memset(m_pModeEx, 0, sizeof(GPEModeEx));
    m_pModeEx->dwSize    = sizeof(GPEModeEx);
    m_pModeEx->dwVersion = GPEMODEEX_CURRENTVERSION;

    // Get number of supported modes
    m_nNumSupportedModes = DisplayGetNumSupportedModes();
    
    // Allocate Supported modes structure
    m_SupportedModes = (GPEMode *)LocalAlloc(LPTR, m_nNumSupportedModes * sizeof(GPEMode));

    m_nScreenBpp = DisplayGetPixelDepth(); 
    DisplayGetSupportedModes(m_SupportedModes);

    // Danny
    // Dump display modes reported
    for (DWORD i = 0; i < m_nNumSupportedModes; i++)
    {
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("%s: Display mode[%d]\r\nwidth = %d\r\nheight = %d\r\nfreq = %d\r\n\r\n"),
                             __WFUNCTION__, m_SupportedModes[i].modeId, m_SupportedModes[i].width,
                             m_SupportedModes[i].height, m_SupportedModes[i].frequency));
    }

    // Get initial display mode, determined by the primary mode from 
    // the Display Interface Layer.
    m_nPrimaryDisplayMode = DisplayGetPrimaryMode();
    m_nCurrentDisplayMode = DisplayGetPrimaryMode();

    m_SupportedModes[m_nCurrentDisplayMode].Bpp = m_nScreenBpp;

    m_iDisplayFrequency = m_SupportedModes[m_nCurrentDisplayMode].frequency;

    EGPEFormat tmpEGPEFormat = gpe16Bpp;
    switch (m_nScreenBpp)
    {
        case 32:
            tmpEGPEFormat = gpe32Bpp;
            break;
        case 24:
            tmpEGPEFormat = gpe24Bpp;
            break;
        case 16:
            tmpEGPEFormat = gpe16Bpp;
            break;
    }

    // Set current BPP to all supported display modes
    for (UINT16 displayMode = 0; displayMode < m_nNumSupportedModes; displayMode++)
    {
        m_SupportedModes[displayMode].format = tmpEGPEFormat;
    } 

    // Get the video pixel depth (independent of UI pixel depth)
    m_nVideoBpp = DisplayGetVideoPixelDepth();
    switch (m_nVideoBpp)
    {
        case 32:
            m_OutputFormat = ddgpePixelFormat_8888;
            break;
        case 24:
            m_OutputFormat = ddgpePixelFormat_8880;
            break;
        case 16:
            m_OutputFormat = ddgpePixelFormat_565;
            break;
    }

    // Always start using the LCD mode (never TV)
    memcpy(m_pMode, &(m_SupportedModes[m_nCurrentDisplayMode]),
           sizeof(GPEMode));

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("%s: Got display mode, %d!\r\n"),
                             __WFUNCTION__, m_pMode->modeId));

    // Determine if we can use a secondary primary surface
    m_bEnableSPrimarySurface = DisplayEanbleSPrimarySurface();
    
    if(m_SupportedModes[m_pMode->modeId].modeId != m_pMode->modeId)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("DDIPU Init: Unmatched ")
                                  TEXT("hardware init info!\r\n")));
        result = FALSE;
        goto _done;
    }

    // Setup surface alignment as required by the IPU for 4 bytes
    m_nSurfacePixelAlign = 4 / (m_pMode->Bpp / 8);
    
    // Sanity Check for screen width
    if(m_pMode->width & (m_nSurfacePixelAlign - 1))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("DDIPU Init: Screen width (%d) ")
                                  TEXT("is not aligned for %d bytes!")
                                  TEXT("\r\n"),
                                  m_pMode->width, m_nSurfacePixelAlign));
        result = FALSE;
        goto _done;
    }

    // Setup other DDGPE mode information
    nBPP = m_pMode->Bpp/8 - 1;
    switch (m_pMode->Bpp)
    {
        case    8:
        case    16:
        case    24:
        case    32:
            m_pModeEx->ePixelFormat = ePixelFormat[nBPP];
            m_pModeEx->lPitch = m_nScreenStride;
            m_pModeEx->dwRBitMask = BitMasks[nBPP][0];
            m_pModeEx->dwGBitMask = BitMasks[nBPP][1];
            m_pModeEx->dwBBitMask = BitMasks[nBPP][2];
            break;

        default:
            DEBUGMSG(GPE_ZONE_ERROR,(TEXT("IPU Init: Invalid BPP ")
                                     TEXT("value passed to driver ")
                                     TEXT("(bpp = %d)\r\n"),
                                     m_pMode->Bpp));
            m_pMode->format = gpeUndefined;
            result = FALSE;
            goto _done;
    }

    if (m_pMode->Bpp == 32)
    {
        m_pModeEx->dwAlphaBitMask = 0x000000FF;
    }

    // Setup Rotation
    m_bRotationSupported = DisplayIsRotationSupported();
    m_nScreenHeightSave = m_pMode->height;
    m_nScreenWidthSave  = m_pMode->width;
    m_iRotate           = DMDO_0; //THE init rotation angle must be 0 degree.
#if 0
    //If the initial rotation degree is not 0, the MSFT code can't handle screen rotation correctly.
    //The following code is temporarily commented for this rotation issue workaround.
    m_iRotate           = GetRotateModeFromReg();
    SetRotateParams();
#endif
    m_nScreenWidth  = m_pMode->width;
    m_nScreenHeight = m_pMode->height;

    // Initialize top overlay node to NULL
    m_iNumVisibleOverlays = 0;

    m_bIsOverlayWindowRunning = FALSE;

    m_pOverlaySurfaceInfo = NULL;

    m_ppActiveOverlays = (pOverlaySurf_t *)LocalAlloc(LPTR, sizeof(pOverlaySurf_t) * MAX_VISIBLE_OVERLAYS);
    if(!m_ppActiveOverlays)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("IPU Init: failed to allocate ")
                                  TEXT("memory for active overlays array, ")
                                  TEXT("error(%d)\r\n"),
                                  GetLastError()));
        result = FALSE;
        goto _done;
    }

    // Initialize Display Update mechanism
    DisplayUpdateInit(&m_csDrawLock);

    // Initialize the XEC
    Display_XECInit();

#if defined(USE_C2D_ROUTINES)

    m_c2dFlag = NULL;
    m_c2dCtx = NULL;

    // retrieve C2D info from BSP layer
    m_c2dFlag = BSPGetC2DInfoFromRegistry();

#if 0
   //&&&&&&&: force this value temporarily for debugging
//   0x1:  SolidColorFill
//   0x2:  PatternFill
//   0x4:  SRCCOPY
//   0x8:  Line
//   0x10: SRCCOPY with mask
    m_c2dFlag = 0x7;
#endif

    // create a C2D context now.
    C2D_STATUS status = c2dCreateContext(&m_c2dCtx);
    if(status != C2D_STATUS_OK)
    {
        m_c2dCtx = NULL;
        m_c2dFlag = NULL;
    }
#endif

_done:
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("IPU Init %s.\r\n"),
                             result ? L"succeeds" : L"fails"));
    return result;
}


//----------------------------------------------------------------------
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
//----------------------------------------------------------------------
BOOL DDIPU::InitHardware(VOID)
{
    static BOOL bIsDisplayInitialized = FALSE;
    BOOL result = TRUE;

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("+%s()\r\n"), __WFUNCTION__));

    //***************************************************
    // Next, init/deinit Display settings
    //***************************************************
    if (!bIsDisplayInitialized)
    {
        DisplayInitialize(m_nScreenBpp, (UINT32*)m_pPrimarySurface->OffsetInVideoMemory());
        bIsDisplayInitialized = TRUE;
    }

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return result;
}


//----------------------------------------------------------------------
//
// Function: SetupVideoMemory
//
// Setup the video memory buffer and other related buffers.
//
// Parameters:
//      bRecreatePrimary
//          [in] If TRUE, primary surface should be deleted and recreated
//               If FALSE, primary is left alone.  Used when display resolution
//                   does not change.
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//----------------------------------------------------------------------
BOOL DDIPU::SetupVideoMemory(BOOL bRecreatePrimary)
{
    SCODE sc;
    BOOL result = TRUE;
    static BOOL bInitialized = FALSE;

    //***********************************************************
    // One-time video memory initialization steps
    //***********************************************************
    if (!bInitialized)
    {
        // Only need to call this one time...video memory size
        // should be constant.
        m_nVideoMemorySize = DisplayGetVideoMemorySize();

        // Get the video memory base address, it will be used to map the frame buffer into 
        // a client application space.
        // Refer to http://msdn.microsoft.com/en-us/library/aa930859.aspx
        m_nVideoMemoryBase= (ULONG)DisplayGetVideoMemoryBase();

        bInitialized = TRUE;
    }


    //***********************************************************
    // Primary Surface Setup
    //***********************************************************
    if (bRecreatePrimary)
    {
        // We need to reallocate surfaces in case the dimensions have changed
        // with the new mode
        if (m_pPrimarySurface)
        {
            delete m_pPrimarySurface;
            m_pPrimarySurface = NULL;
        }

        // Create primary surface
        if (FAILED(sc = AllocSurface((DDGPESurf **) &m_pPrimarySurface, m_nScreenWidthSave,
                m_nScreenHeightSave, m_pMode->format, m_pModeEx->ePixelFormat,
                GPE_REQUIRE_VIDEO_MEMORY)))
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("IPU SetMode: Couldn't allocate primary surface\r\n")));
            return FALSE;
        }

        m_CursorAreaLock = FALSE;
        m_pActiveSurface = (DDIPUSurf *)m_pPrimarySurface;
        DisplaySetStride(FALSE, m_pPrimarySurface->Stride());
        
        //Suppose the primary surface isn't YV12 surface.
        memset(m_pPrimarySurface->Buffer(), 0xFF, 
               m_nScreenWidthSave*m_nScreenHeightSave*EGPEFormatToBpp[m_pMode->format]/8);
    }

    //***********************************************************
    // Secondary Surface Setup
    //***********************************************************
    if (m_pPrimarySurface2)
    {
        delete m_pPrimarySurface2;
        m_pPrimarySurface2 = NULL;
    }

    // Create the secondary primary surface if the feature is enabled.
    if (m_bEnableSPrimarySurface && (m_nCurrentDisplayMode != m_nPrimaryDisplayMode))
    {
        // Create second primary surface, here second primary surface always is the "primary" display, 
        // so we use m_nPrimaryDisplayMode to get information.
        if (FAILED(sc = AllocSurface((DDGPESurf **) &m_pPrimarySurface2, 
                    m_SupportedModes[m_nPrimaryDisplayMode].width,
                    m_SupportedModes[m_nPrimaryDisplayMode].height, 
                    m_SupportedModes[m_nPrimaryDisplayMode].format, 
                    m_pModeEx->ePixelFormat,
                    GPE_REQUIRE_VIDEO_MEMORY)))
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("IPU SetMode: Couldn't allocate second primary surface\r\n")));
            return FALSE;
        }

        DisplaySetStride(TRUE, m_pPrimarySurface2->Stride());
        
        //Suppose the primary surface isn't YV12 surface.
        memset(m_pPrimarySurface2->Buffer(), 0xFF, 
             m_SupportedModes[m_nPrimaryDisplayMode].width*
             m_SupportedModes[m_nPrimaryDisplayMode].height*
             EGPEFormatToBpp[m_SupportedModes[m_nPrimaryDisplayMode].format]/8);        
    }
    
    if(!result)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("IPU SetupVideoMemory: failed at VirtualCopy(), error[%d]\r\n"), m_nVideoMemorySize, GetLastError()));
        result = FALSE;
        goto _done;
    }

_done:

    if(result)
    {
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("IPU SetupVideoMemory: \r\n")));
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("\t\tSize:             0x%08x\r\n"), m_nVideoMemorySize));
    }

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
VOID DDIPU::Cleanup(VOID)
{
    pOverlaySurf_t pTempOverlayInfo, pTempOverlayInfoNext;

    // TODO:
    //Display_DisablePanels();
    //Display_DisableDisplayPorts();

    // Destroy surface objects
    if(m_pPrimarySurface)
        delete m_pPrimarySurface;
        
    if(m_pPrimarySurface2)
        delete m_pPrimarySurface2;

    DeleteCriticalSection(&m_csDrawLock);

    DeleteCriticalSection(&m_csOverlayData);

    DeleteCriticalSection(&m_csOverlayShutdown);


    // Release all overlay surface info structures
    if(m_pOverlaySurfaceInfo)
    {
        pTempOverlayInfo = m_pOverlaySurfaceInfo;
        while (pTempOverlayInfo != NULL)
        {
            // Store next pointer in temporary variable
            pTempOverlayInfoNext = pTempOverlayInfo->next;
            // Free current info structure
            free(pTempOverlayInfo);
            // Look at next info structure
            pTempOverlayInfo = pTempOverlayInfoNext;
        }

        m_pOverlaySurfaceInfo = NULL;
    }

#if defined(USE_C2D_ROUTINES)

    if(NULL != m_c2dFlag)
        c2dDestroyContext(&m_c2dCtx);
    m_c2dCtx = NULL;
    m_c2dFlag = NULL;
    
#endif    
}

//------------------------------------------------------------------------------
//
// Function: IsFlipBusy
//
// This method determines whether Flip is busy or not.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE    busy
//      FALSE   not busy
//
//------------------------------------------------------------------------------
BOOL DDIPU::IsFlipBusy(VOID)
{
    return Display_OverlayIsBusy();
}
