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
//-----------------------------------------------------------------------------
//  Copyright (C) 2004-2005, MOTOROLA, INC. All Rights Reserved
//------------------------------------------------------------------------------
//---------------------------------------------------------------------------
//  Copyright (C) 2005-2006, 2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  DDLCDC.cpp
//
//  Implementation of class DDLCDC.
//
//------------------------------------------------------------------------------

#include "precomp.h"


//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------
extern BOOL InitializeLCDC(BOOL bTVModeActive, BOOL bTVNTSCOut, DWORD dwPanelType);

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// EXPORTED FUNCTIONS
//------------------------------------------------------------------------------

// This prototype avoids problems exporting from .lib
BOOL APIENTRY GPEEnableDriver(ULONG engineVersion, ULONG cj, DRVENABLEDATA *data,
                              PENGCALLBACKS  engineCallbacks);

BOOL APIENTRY DrvEnableDriver(ULONG engineVersion, ULONG cj, DRVENABLEDATA *data,
                              PENGCALLBACKS  engineCallbacks)
{
    return GPEEnableDriver(engineVersion, cj, data, engineCallbacks);
}

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

INSTANTIATE_GPE_ZONES((GPE_ZONE_ERROR|GPE_ZONE_INIT|GPE_ZONE_WARNING),"MX27DDLcdc","unused1","unused2");

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
static DWORD g_dwlcdcSysintr;

static GPE *gGPE = (GPE *) NULL;

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
    ddgpePixelFormat_8880,  // not supported by i.MX27 LCDC, just for possible future expansion
    ddgpePixelFormat_8888,  // not supported by i.MX27 LCDC, just for possible future expansion
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
static DWORD MX27DDLcdcIntr(MX27DDLcdc *pClass);

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
    if (gGPE == NULL)
        return(BitMasks[0]);

    int nBPP = ((MX27DDLcdc *)gGPE)->GetBpp()/8 - 1;
    switch (((MX27DDLcdc *)gGPE)->GetBpp())
    {
        case 8:
        case 16:
        case 24:
        case 32:
            return(BitMasks[nBPP]);
            break;
        default:
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
    if (!gGPE)
    {
        BOOL result;
        gGPE = new MX27DDLcdc(&result);

        if(!result)
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc: Failed to get GPE instance!\r\n")));
            if(gGPE)
            {
                delete gGPE;
                gGPE = NULL;
            }
        }
    }

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MX27DDLcdc: GetGPE(0x%08x)\r\n"), gGPE));
    return gGPE;
}


//------------------------------------------------------------------------------
// CLASS MEMBER FUNCTIONS
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  FUNCTION:     MX27DDLcdc
//
//  DESCRIPTION:  Constructor of MX27DDLcdc class
//
// Parameters:
//      pbRet
//          [in] Pointer to return value.  TRUE if success, FALSE if failure.
//
// Returns:
//
//------------------------------------------------------------------------------
MX27DDLcdc::MX27DDLcdc(BOOL * pbRet)
{
    BOOL result = TRUE;

    // Initialze MX27DDLcdc class on software perspective
    if(!Init())
    {
        result = FALSE;
        goto _done;
    }

    // Setup video buffer
    if(!SetupVideoMemory())
    {
        result = FALSE;
        goto _done;
    }

    // Setup LCDC of i.MX27 processor
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

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MX27DDLcdc: class construction %s!!\r\n"), result ? L"succeeds" : L"fails"));
    *pbRet = result;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     ~MX27DDLcdc
//
//  DESCRIPTION:  Destructor of ~MX27DDLcdc
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
MX27DDLcdc::~MX27DDLcdc()
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
SCODE MX27DDLcdc::SetMode(int modeNo, HPALETTE * pPalette)
{
    if(m_bSetMode)
        return S_OK;
    if(modeNo != m_pMode->modeId)
    {
        // WindowCE doesn't allow to change display mode at run time
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc SetMode: failed to setMode, desired[%d], current[%d]\r\n"), modeNo, m_pMode->modeId));
        return E_INVALIDARG;
    }

    m_bSetMode = TRUE;

    m_dwPhysicalModeID = m_pMode->modeId;

    int nBPP = m_nScreenBpp/8 - 1;
    if (pPalette)
    {
        switch (m_nScreenBpp)
        {
            case    8:
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

    m_nScreenHeightSave = m_pMode->height;
    m_nScreenWidthSave = m_pMode->width;
    DynRotate(m_iRotate);

    // Start display driver!!
    if(m_pPrimarySurface->OffsetInVideoMemory())
        SetVisibleSurface((GPESurf *) m_pPrimarySurface);

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
SCODE MX27DDLcdc::GetModeInfo(GPEMode * pMode, int modeNo)
{
    if(modeNo >= NumModes())
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc GetModeInfo: wrong mode (%d)!!\r\n"), modeNo));
        return E_INVALIDARG;
    }

    *pMode = *m_pMode;

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
SCODE MX27DDLcdc::GetModeInfoEx(GPEModeEx * pModeEx, int modeNo)
{
    if(modeNo >= NumModes())
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc GetModeInfoEx: wrong mode (%d)!!\r\n"), modeNo));
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
int MX27DDLcdc::NumModes(VOID)
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
VOID MX27DDLcdc::GetPhysicalVideoMemory(PULONG pPhysicalMemoryBase, PULONG pVideoMemorySize)
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
int MX27DDLcdc::GetBpp(VOID)
{
    return m_nScreenBpp;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     Init
//
//  DESCRIPTION:  Software perspective initialization of MX27DDLcdc class
//
// Parameters:
//      None.
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//------------------------------------------------------------------------------
BOOL MX27DDLcdc::Init(VOID)
{
    BOOL result = TRUE;
    UINT iLoop = 0;
    BOOL bModeGot = FALSE;
    int  nBPP;

    // Pre-Initialization
    m_bSetMode = FALSE;

    m_pPrimarySurface = NULL;
    m_pBlankSurface   = NULL;
    m_pVisibleOverlay = NULL;

    m_pVideoMemoryHeap = NULL;

    //Flip Status
    m_bFlipInProgress = FALSE;

    m_nPreIntrStatus = 0;
    m_nMask = 0;     // for flicker

    // Pre-setup display mode in DDGPE engine
    m_pModeEx = &m_ModeInfoEx;
    m_pMode = &m_ModeInfoEx.modeInfo;
    memset(m_pModeEx, 0, sizeof(GPEModeEx));
    m_pModeEx->dwSize = sizeof(GPEModeEx);
    m_pModeEx->dwVersion = GPEMODEEX_CURRENTVERSION;

    BSPInitLCDC(&m_LcdcCtx);
    // setup local mode info structure
    m_pMode  = (GPEMode*)(m_LcdcCtx.pGPEModeInfo);

    RETAILMSG (0, (TEXT("mode 0x%x\r\n"), m_pMode->modeId));
    if(m_pMode->modeId == DISPLAY_MODE_SHPQVGA)   // SHARP
    {
        m_bTVModeActive = FALSE;
        m_nPanelTYPE = 0;
    }
    if(m_pMode->modeId == DISPLAY_MODE_NECVGA)    // NEC
    {
        m_bTVModeActive = FALSE;
        m_nPanelTYPE = 1;
    }
    if(m_pMode->modeId == DISPLAY_MODE_PAL)
    {
        m_bTVModeActive = TRUE;
        m_bTVNTSCOut = FALSE;           //PAL
    }
    if(m_pMode->modeId == DISPLAY_MODE_NTSC)
    {
        m_bTVModeActive = TRUE;
        m_bTVNTSCOut = TRUE;            //NTSC
    }

    if(!bModeGot)
    {
        // No suitable display mode found, default display mode is used
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("MX27DDLcdc Init: Default display mode is used!\r\n")));
        m_nScreenWidth  = m_pMode->width;
        m_nScreenHeight = m_pMode->height;
        m_nScreenBpp    = m_pMode->Bpp;
    }

    // Setup surface alignment as required by the LCDC of i.MX27 for 4 bytes
    m_nSurfacePixelAlign = 4 / (m_pMode->Bpp / 8);

    // Sanity Check for screen width
    if(m_pMode->width & (m_nSurfacePixelAlign - 1))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc Init: Screen width (%d) is not aligned for %d bytes!\r\n"), m_pMode->width, m_nSurfacePixelAlign));
        result = FALSE;
        goto _done;
    }

    // The line stride of i.MX27 LCDC is 32-bits (4 bytes) aligned
    m_nScreenStride = ((m_nScreenWidth * (m_nScreenBpp / 8) + 3) >> 2) << 2;

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MX27DDLcdc Init: Current display mode:\r\n")));
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("\t\tm_nScreenWidth:  %d\r\n"), m_nScreenWidth));
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("\t\tm_nScreenHeight: %d\r\n"), m_nScreenHeight));
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("\t\tm_nScreenStride: %d\r\n"), m_nScreenStride));
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("\t\tm_nScreenBpp:    %d\r\n"), m_nScreenBpp));

    // Setup other DDGPE mode informations
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
            DEBUGMSG(GPE_ZONE_ERROR,(TEXT("MX27DDLcdc Init: Invalid BPP value passed to driver - Bpp = %d\r\n"), m_pMode->Bpp));
            m_pMode->format = gpeUndefined;
            result = FALSE;
            goto _done;
    }

    // Setup Rotation
    m_nScreenHeightSave = m_pMode->height;
    m_nScreenWidthSave = m_pMode->width;
    m_iRotate = GetRotateModeFromReg();
    SetRotateParams();

    m_nScreenWidth = m_pMode->width;
    m_nScreenHeight = m_pMode->height;


    // Pre-initialize the graphic window operator for overlay
    m_pOverlaySurfaceOp = (pOverlaySurf_t)malloc(sizeof(overlaySurf_t));
    if(m_pOverlaySurfaceOp)
    {
        memset(m_pOverlaySurfaceOp, 0x00, sizeof(overlaySurf_t));
    }
    else
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc Init: failed to allocate memory for graphic windower operator, error(%d)\r\n"), GetLastError()));
        result = FALSE;
        goto _done;
    }

_done:
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MX27DDLcdc Init %s.\r\n"), result ? L"succeeds" : L"fails"));
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
BOOL MX27DDLcdc::InitHardware(VOID)
{
    BOOL result = TRUE;
    PHYSICAL_ADDRESS phyAddr;

    //Mem maps the LCDC module space for user access
    phyAddr.QuadPart = CSP_BASE_REG_PA_LCDC;
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
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc InitHardware: fail to create the intr event, error[%d]!\r\n"), GetLastError()));
        result = FALSE;
        goto _done;
    }

    DWORD irq = IRQ_LCDC;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
                        &g_dwlcdcSysintr, sizeof(DWORD), NULL)){
                DEBUGMSG(GPE_ZONE_ERROR, (_T("Failed to obtain sysintr value!\r\n")));
        return 0;
    }

    if(!InterruptInitialize(g_dwlcdcSysintr, m_hSyncEvent, NULL, NULL))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc InitHardware: fail to initialize the interrupt, error[%d]!\r\n"), GetLastError()));
        result = FALSE;
        goto _done;
    }

    m_hSyncThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MX27DDLcdcIntr, this, 0, NULL);
    if(NULL == m_hSyncThread)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc InitHardware: fail to initiate the interrupt thread, error[%d]!\r\n"), GetLastError()));
        result = FALSE;
        goto _done;
    }

    // Initialze a critical section to protect interrupt status flag update violation
    InitializeCriticalSection(&m_CriticalSection);

    // Configure LCDC of i.MX27 processor
    if(!SetModeHardware(m_pMode->modeId))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc InitHardware: fail to initialize hardware!\r\n"), GetLastError()));
        result = FALSE;
        goto _done;
    }

_done:

    return result;
}


//------------------------------------------------------------------------------
//
// Function: SetupVideoMemory
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
BOOL MX27DDLcdc::SetupVideoMemory(VOID)
{
    SCODE sc;
    PHYSICAL_ADDRESS phyAddr;
    BOOL result = TRUE;
    DWORD iFrameSize = m_nScreenStride * m_nScreenHeightSave;

    // Allocate video memory
    /*
     * IMPORTANT: There is dedicated video memory for i.MX27
     *            processor, therefore, we should not share from
     *            system memory.
     */

   // The video memory should be more than three frame size:
      //    - one for primary surface // primary
      //    - one for a blank surface used when OS goes to power suspension // blank
      //    - one for general use in DirectDraw applications

    //iFrameSize = m_LcdcCtx.VideoFrameWidth * m_LcdcCtx.VideoFrameHight * BSPGetPixelSizeInByte();
    m_nVideoMemorySize = m_LcdcCtx.VideoMemorySize;
    //    m_nVideoMemorySize = max3(m_nVideoMemorySize, 3*iFrameSize, DEFAULT_VIDEO_MEM_SIZE);
    phyAddr.QuadPart = m_LcdcCtx.VideoMemoryPhyAdd;
    m_pVideoMemory = (UINT8 *)MmMapIoSpace(phyAddr, m_nVideoMemorySize, FALSE);
    if (m_pVideoMemory == NULL)
    {
        DEBUGMSG (GPE_ZONE_ERROR, (TEXT("Frame Buffer MmMapIoSpace failed!\r\n")) );
                result = FALSE;
                goto _done;
    }
    m_nLAWPhysical = m_LcdcCtx.VideoMemoryPhyAdd;

    m_pLAW = (PUCHAR)VirtualAlloc(NULL, m_nVideoMemorySize, MEM_RESERVE, PAGE_READWRITE);

    result = VirtualCopy(
        m_pLAW,
        (LPVOID)(m_nLAWPhysical >> 8),
        m_nVideoMemorySize,
        PAGE_READWRITE|PAGE_NOCACHE|PAGE_PHYSICAL);

    if(!result)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc SetupVideoMemory: failed at VirtualCopy(), error[%d]\r\n"), m_nVideoMemorySize, GetLastError()));
        result = FALSE;
        goto _done;
    }

    // Create video memory heap
    if (!m_pVideoMemoryHeap)
        m_pVideoMemoryHeap = new SurfaceHeap(m_nVideoMemorySize, NULL, NULL, NULL);

    // Create primary surface and blank surface
    if (!m_pPrimarySurface)
    {
        if (FAILED(sc = AllocSurface((DDGPESurf **) &m_pPrimarySurface, m_nScreenWidthSave,
                m_nScreenHeightSave, m_pMode->format, m_pModeEx->ePixelFormat,
                GPE_REQUIRE_VIDEO_MEMORY)))
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc SetMode: Couldn't allocate primary surface\r\n")));
            result = FALSE;
            goto _done;
        }
    }

    // Load freescale logo onto screen in case that the primary surface is not at the starting point of video memory.
    if(m_pPrimarySurface->OffsetInVideoMemory())
    {
        WaitForNotBusy();
        //LoadFreescaleLogo(m_pPrimarySurface);
    }

    m_pPrimarySurface->SetRotation(m_nScreenWidth, m_nScreenHeight, m_iRotate);

_done:

    if(result)
    {
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("MX27DDLcdc SetupVideoMemory: \r\n")));
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("\tVirtual address mapped access window: 0x%08x\r\n"), m_pLAW));
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("\tVirtual address of video memory:      0x%08x\r\n"), m_pVideoMemory));
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("\tPhysical address of video memory:     0x%08x\r\n"), m_LcdcCtx.VideoMemoryPhyAdd));
        DEBUGMSG(GPE_ZONE_INIT, (TEXT("\tSize:                                 0x%08x\r\n"), m_nVideoMemorySize));
    }

    return result;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     SetModeHardware
//
//  DESCRIPTION:  Setup hardware LCDC for current display mode
//
//  PARAMETERS:
//
//  RETURNS:
//                TRUE: successful
//                FALSE: failed
//
//------------------------------------------------------------------------------
BOOL MX27DDLcdc::SetModeHardware(int modeNo)
{
    BOOL result = FALSE;

    if(!InitializeLCDC(m_bTVModeActive, m_bTVNTSCOut, m_nPanelTYPE))
    {
        DEBUGMSG(1, (TEXT("InitializeLCDC Failed")));
        goto _done;
    }

    // Pre-setup framebuffer address

    // Load Freescale logo at the starting point of the video memory
    //LoadFreescaleLogo(m_pLAW);

    result = TRUE;

_done:
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
VOID MX27DDLcdc::Cleanup(VOID)
{
    DDK_GPIO_CFG cfg;

    m_nMask = 0;   // for flicker

    BSPTurnOffLCD();

    BSPDeinitLCDC(&m_LcdcCtx);

    // Release LCDC register map
    if(m_pLcdcReg)
    {
        // disable LCDC
        INSREG32BF(&m_pLcdcReg->RMCR, LCDC_RMCR_SELF_REF, LCDC_RMCR_SELF_REF_DISABLE);

        MmUnmapIoSpace(m_pLcdcReg, sizeof(CSP_LCDC_REGS));
        m_pLcdcReg  =   NULL;
    }

    // Disable LCDC module pins
    DDK_GPIO_SET_CONFIG(cfg, LCDC);
    DDKGpioDisable(&cfg);

    // Disable LCDC hw clocks
    DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_DISABLE);

    // Destroy surface objects
    if(m_pPrimarySurface)
    {
        delete m_pPrimarySurface;
        m_pPrimarySurface = NULL;
    }
    if(m_pBlankSurface)
    {
        delete m_pBlankSurface;
        m_pBlankSurface = NULL;
    }

    // Release the shared vidoe memory
    if (m_pLAW)
    {
        VirtualFree(m_pLAW, 0, MEM_RELEASE);
        m_pLAW = NULL;
    }

    //Not need release memory because of reserved physical memory
    /*
    // Release video memory
    if(m_pVideoMemory)
    {
        FreePhysMem(m_pVideoMemory);
        m_pVideoMemory = NULL;
    }
    */

    // Release the overlay operator
    if(m_pOverlaySurfaceOp)
    {
        free(m_pOverlaySurfaceOp);
        m_pOverlaySurfaceOp = NULL;
    }

    // Disable interrupt source and release interrupt event
    InterruptDisable(g_dwlcdcSysintr);

    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &g_dwlcdcSysintr, sizeof(DWORD), NULL, 0, NULL);
    g_dwlcdcSysintr = SYSINTR_UNDEFINED;

    if(m_hSyncEvent)
    {
        CloseHandle(m_hSyncEvent);
        m_hSyncEvent= NULL;
    }

    // Terminate interrupt thread
    if(m_hSyncThread)
    {
        TerminateThread(m_hSyncThread, 0);
        CloseHandle(m_hSyncThread);
        m_hSyncThread = NULL;
    }

    DeleteCriticalSection(&m_CriticalSection);
}


//------------------------------------------------------------------------------
//
//  FUNCTION:       IntrProc
//
//  DESCRIPTION:    This function is interrupt handler.
//
//  NOTE:
//
//  PARAMETERS:
//
//  RETURNS:
//                  A double word value for ExitThread()
//
//------------------------------------------------------------------------------
DWORD MX27DDLcdc::IntrProc(VOID)
{

    CeSetThreadPriority(GetCurrentThread(), DISPLAY_DRIVER_THREAD_PRIORITY);
    while(TRUE)
    {
        if (WaitForSingleObject(m_hSyncEvent, INFINITE) != WAIT_OBJECT_0)
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc IntrProc: interrupt error [%d]\r\n"), GetLastError()));
            continue;
        }

        EnterCriticalSection(&m_CriticalSection);
        m_nPreIntrStatus = m_pLcdcReg->ISR;

        // When EOF is reached it indicates the Flip/painting on to the LCD is completed.
        if (m_bFlipInProgress && (m_nPreIntrStatus & CSP_BITFVAL(LCDC_ISR_EOF, LCDC_ISR_EOF_INTERRUPT)))
        {
            m_bFlipInProgress = FALSE;
            // Turn off the interrupts since we donot need to continously
            // monitor the status, will enable again when a flip is done
            OUTREG32(&m_pLcdcReg->IER, 0);    
        }

        LeaveCriticalSection(&m_CriticalSection);

        InterruptDone(g_dwlcdcSysintr);
    }

    return 0;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:       MX27DDLcdcIntr
//
//  DESCRIPTION:    This function is entry point of the interrupt thread.
//
//  NOTE:
//
//  PARAMETERS:
//
//  RETURNS:
//                  A double word value for ExitThread()
//
//------------------------------------------------------------------------------
DWORD MX27DDLcdcIntr(MX27DDLcdc * pClass)
{
    pClass->IntrProc();

    // We shouldn't come to here
    DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdcIntr: Exiting interrupt thread... error[%d]\r\n"), GetLastError()));
    return 0;
}
