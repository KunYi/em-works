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
//------------------------------------------------------------------------------
//
//  File:  omap2420gpe.cpp
//
//  This file implements the Omap2420 ddraw enabled display driver.
//
#include "precomp.h"
#include <devload.h>
#include <pmimpl.h>
#include <ceddkex.h>

//#undef DEBUGMSG
//#define DEBUGMSG(x,y) RETAILMSG(1,y)
//#define DEBUG

//------------------------------------------------------------------------------
// External functions

BOOL APIENTRY GPEEnableDriver(
    ULONG version, ULONG cj, DRVENABLEDATA *pData, ENGCALLBACKS *pCallbacks
);

//------------------------------------------------------------------------------
// Debug Zones

INSTANTIATE_GPE_ZONES(0x8003, "ddi_omap2420", "PDD", "Power")

#define GPE_ZONE_PDD        DEBUGZONE(14)
#define GPE_ZONE_PM         DEBUGZONE(15)

//------------------------------------------------------------------------------
#define I2C_DISPLAY_ENABLE_ADDRESS      0x20
#define I2C_DISPLAY_ENABLE_ADDRSIZE     7
#define LCD_ENVDD_BIT                   BIT7

#define GPE_DDGPE_SURF                  0x80000000

const GUID DEVICE_IFC_I2C_GUID;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
    {
        L"MemBase", PARAM_MULTIDWORD, TRUE, offset(OMAP2420GPE, m_memBase),
        fieldsize(OMAP2420GPE, m_memBase), NULL
    }, { 
        L"MemLen", PARAM_MULTIDWORD, TRUE, offset(OMAP2420GPE, m_memLen),
        fieldsize(OMAP2420GPE, m_memLen), NULL
    }, { 
        L"Bpp", PARAM_DWORD, FALSE, offset(OMAP2420GPE, m_bitsPerPixel),
        fieldsize(OMAP2420GPE, m_bitsPerPixel), (VOID*)16
    }, { 
        L"PowerDelay", PARAM_DWORD, FALSE, offset(OMAP2420GPE, m_powerDelay),
        fieldsize(OMAP2420GPE, m_powerDelay), (VOID*)INFINITE
    }, {
        L"Priority256", PARAM_DWORD, FALSE, offset(OMAP2420GPE, m_priority256),
        fieldsize(OMAP2420GPE, m_priority256), (VOID*)252
    }, { 
        L"DisplayPowerClass", PARAM_STRING, FALSE, 
        offset(OMAP2420GPE, m_szDisplayPowerClass), 0, PMCLASS_DISPLAY
    } 
           
};

#if defined(TVOUT)
//------------------------------------------------------------------------------
// video encoder initialization values

static const VENC_INIT g_Venc_Init[2] =
{
    // NTSC
    {
    0x0000, 0x1040, 0x0359, 0x020C, 0x0000, 0x0102, 0x016C, 0x012F, 0x0043,
    0x0038, 0x0000, 0x0001, 0x0038, 0x21F07C1F, 0x069300F4, 0x0016020C,
    0x00060107, 0x007E034E, 0x000F0359, 0x01A00000, 0x020901A0, 0x01AC0022,
    0x020D01AC, 0x0006, 0x03480078, 0x02060026, 0x0001008A, 0x01AC0106,
    0x01060006, 0x00140001, 0x00010001, 0x00FF0000
    },

    // PAL
    {
    0x0000, 0x1040, 0x035F, 0x0270, 0x0000, 0x0111, 0x0181, 0x0140, 0x003B,
    0x003B, 0x0000, 0x0002, 0x003F, 0x2A098ACB, 0x06A70108, 0x00180270,
    0x00040136, 0x00880358, 0x000F035F, 0x01A70000, 0x026F01A7, 0x01AF002E,
    0x027101AF, 0x0005, 0x03530083, 0x026E002E, 0x0005008A, 0x002E0138,
    0x01380005, 0x00140001, 0x00010001, 0x00FF0000
    }
};
#endif // defined(TVOUT)

//------------------------------------------------------------------------------
// variables

static DDGPE    *g_pGPE;
static ULONG    g_bitMasks[] = { 0, 0, 0 };

//------------------------------------------------------------------------------
//
//  Function:  DisplayInit
//
//  The GWES will invoke this routine once prior to making any other calls
//  into the driver. This routine needs to save its instance path information
//  and return TRUE.  If it returns FALSE, GWES will abort the display
//  initialization.
//
BOOL DisplayInit(LPCWSTR context, DWORD monitors)
{
    BOOL rc = FALSE;
    OMAP2420GPE *pGPE;

    DEBUGMSG(GPE_ZONE_PDD, (L"+DisplayInit(%s, %d)\r\n", context, monitors));

    pGPE = new OMAP2420GPE;
    if (context == NULL)
    {
        context = L"System\\GDI\\Drivers";
    }
    
    if (pGPE == NULL || !pGPE->Init(context))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: DisplayInit: "
            L"Failed create and initialize OMAP2420GPE object\r\n"
        ));
        goto cleanUp;
    }

    // Save instance to global variable
    g_pGPE = pGPE;

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(GPE_ZONE_PDD, (L"-DisplayInit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  DrvEnableDriver
//
//  This function must be exported from display driver. As long as we use
//  GPE class implementation we don't need do anything else than call
//  GPEEnableDriver library function.
//
BOOL DrvEnableDriver(ULONG version, ULONG cj, DRVENABLEDATA *pData, ENGCALLBACKS *pCallbacks)
{
    BOOL rc;

    DEBUGMSG(GPE_ZONE_PDD, (L"+DrvEnableDriver(0x%08x, %d, 0x%08x, 0x%08x)\r\n", 
        version, cj, pData, pCallbacks
    ));

    rc = GPEEnableDriver(version, cj, pData, pCallbacks);

    DEBUGMSG(GPE_ZONE_PDD, (L"-DrvEnableDriver(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  DrvGetMasks
//
ULONG *APIENTRY DrvGetMasks(DHPDEV dhpdev)
{
    return g_bitMasks;
}

//------------------------------------------------------------------------------
//
//  Function:  GetGPE
//
//  This function is called from GPE class library to get pointer to class
//  deriver from GPE.
//
GPE* GetGPE()
{
    return g_pGPE;
}

//-----------------------------------------------------------------------------
//
//  Function:  ConvertStringToGuid
//
static BOOL ConvertStringToGuid(LPCWSTR szGuid, GUID *pGuid)
{
    BOOL rc = FALSE;
    int idx, data4[8];
    const LPWSTR fmt = L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}";

    if (swscanf(
        szGuid, fmt, &pGuid->Data1, &pGuid->Data2, &pGuid->Data3, 
        &data4[0], &data4[1], &data4[2], &data4[3], 
        &data4[4], &data4[5], &data4[6], &data4[7]
    ) != 11) goto cleanUp;

    for (idx = 0; idx < dimof(data4); idx++) {
        pGuid->Data4[idx] = (UCHAR)data4[idx];
    }

    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Constructor
//
OMAP2420GPE::OMAP2420GPE()
{
    m_pVisibleSurface = NULL;
    m_pVisibleOverlay = NULL;
    m_currentDX = D4;
    m_externalDX = D0;
    InitializeCriticalSection(&m_powerCS);
}

//------------------------------------------------------------------------------
//
//  Destructor
//
OMAP2420GPE::~OMAP2420GPE()
{
    // Close power thread
    if (m_hPowerThread != NULL) {
        // Signal stop to thread
        m_powerThreadExit = TRUE;
        // Set event to wake it
        SetEvent(m_hPowerEvent);
        // Wait until thread exits
        WaitForSingleObject(m_hPowerThread, INFINITE);
    }

    // Close power event
    if (m_hPowerEvent != NULL)
    {
        CloseHandle(m_hPowerEvent);
    }

    // Delete critical section
//    DeleteCriticalSection(&m_powerCS);

    // Delete LCD object
    if (m_hDisplay != NULL) DisplayPddDeinit(m_hDisplay);

    // Remove power class interface
    if (m_szDisplayPowerClass != NULL) {
        LocalFree(m_szDisplayPowerClass);
    } else {        
        AdvertiseInterface(&m_powerClassGUID, L"DDI", FALSE);
    }

    // Unmap hardware registers & SRAM
    if (m_pDSSRegs != NULL)
    {
        MmUnmapIoSpace((VOID*)m_pDSSRegs, m_memLen[MEM_IDX_DSS]);
    }
    if (m_pDISPCRegs != NULL)
    {
        MmUnmapIoSpace((VOID*)m_pDISPCRegs, m_memLen[MEM_IDX_LCD]);
    }
    if (m_pVENCRegs != NULL)
    {
        MmUnmapIoSpace((VOID*)m_pVENCRegs, m_memLen[MEM_IDX_VENC]);
    }
    
    // As last delete critical section
    DeleteCriticalSection(&m_powerCS);
}

//------------------------------------------------------------------------------
//
//  Method:  Init
//
//  This method initialize OMAP2420GPE instance.
//
BOOL OMAP2420GPE::Init(LPCWSTR context)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS pa;
#if defined(TVOUT)
    I2C_SET_SLAVE_ADDRESS i2c;
#endif

    //DEBUGMSG(TRUE,(L"OMAP2420GPE::Init++\r\n"));
    // Read device parameters
    if (GetDeviceRegistryParams(context,
            this,
            dimof(g_deviceRegParams),
            g_deviceRegParams) != ERROR_SUCCESS)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::Init: "
            L"Failed read display driver registry parameters\r\n"));
        goto cleanUp;
    }
    ASSERT(m_bitsPerPixel == 16);

    // Convert display power class to GUID
    if (!ConvertStringToGuid(m_szDisplayPowerClass,
            &m_powerClassGUID))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::Init: "
            L"Failed convert display power class '%s' to GUID\r\n"));
        goto cleanUp;
    }

    // Advertise power class interface
    if (!AdvertiseInterface(&m_powerClassGUID,
            L"DDI",
            TRUE))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::Init: "
            L"Failed advertise display power class '%s'\r\n"
        ));
        goto cleanUp;
    }        

    // We don't need string GUID anymore
    delete m_szDisplayPowerClass;
    m_szDisplayPowerClass = NULL;

    // First initialize LCD display module connected to the controller
    if ((m_hDisplay = DisplayPddInit(context)) == NULL)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::Init: "
            L"Failed initialize LCD display object\r\n"));
        goto cleanUp;
    }

    // Map DSS controller registers
    pa.QuadPart = m_memBase[MEM_IDX_DSS];
    if ((m_pDSSRegs = (OMAP2420_DSS_REGS*)MmMapIoSpace(pa, m_memLen[MEM_IDX_DSS], FALSE)) == NULL)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::Init: "
            L"Failed to map DSS registers (pa = 0x%08x)\r\n", pa.LowPart));
        goto cleanUp;
    }

    // Map LCD controller registers
    pa.QuadPart = m_memBase[MEM_IDX_LCD];
    if ((m_pDISPCRegs = (OMAP2420_DISPC_REGS*)MmMapIoSpace(pa, m_memLen[MEM_IDX_LCD], FALSE)) == NULL)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::Init: "
            L"Failed to map LCD registers (pa = 0x%08x)\r\n", pa.LowPart));
        goto cleanUp;
    }

    // Map video encoder registers
    pa.QuadPart = m_memBase[MEM_IDX_VENC];
    if ((m_pVENCRegs = (OMAP2420_VENC_REGS*)MmMapIoSpace(pa, m_memLen[MEM_IDX_VENC], FALSE)) == NULL)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::Init: "
            L"Failed to map VENC registers (pa = 0x%08x)\r\n", pa.LowPart));
        goto cleanUp;
    }
   
#if defined(TVOUT)
    // open I2C bus
    m_hI2C = CreateFile(I2C_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, 0, NULL);
    if (m_hI2C == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::Init: "
            L"Failed (%08X) to open I2C device %s (%08X)\r\n", GetLastError(), I2C_DEVICE_NAME));
        goto cleanUp;
    }

    // set the I2C slave address
    i2c.address = I2C_MENELAUS_ADDRESS;
    i2c.size = I2C_MENELAUS_ADDRSIZE;
    if (!DeviceIoControl(m_hI2C, IOCTL_I2C_SET_SLAVE_ADDRESS, (PVOID)&i2c, sizeof(i2c), NULL, 0, NULL, NULL))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::Init: "
            L"Failed (%08X) to set I2C slave address\r\n", GetLastError()));
        CloseHandle(m_hI2C);
        goto cleanUp;
    }
#endif // defined(TVOUT)

    // Create power event
    if ((m_hPowerEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::Init: "
            L"Failed create power event\r\n"));
        goto cleanUp;
    }
    
    // Start power thread
    if ((m_hPowerThread = CreateThread(NULL, 0, PowerThreadStub, this, 0, NULL)) == NULL)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::Init: "
            L"Failed create power thread\r\n"));
        goto cleanUp;
    }

    // Set power thread priority
    CeSetThreadPriority(m_hPowerThread, m_priority256);

#if defined(TVOUT)
    // get the TV-Out registers pointer and set the initial TV-Out mode
    m_pTVVidRegs = &m_pDISPCRegs->tDISPC_VID2;
    m_dwTVOut = TVOUT_NONE;
#endif

    // Done
    rc = TRUE;

cleanUp:
    //DEBUGMSG(TRUE,(L"OMAP2420GPE::Init--\r\n"));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Method:  NumModes
//
//  This method returns number of modes supported by display. In our
//  implementation number of modes depends on display (and in most cases it
//  will be one). In theory we can add modes with different bpp...
//
int OMAP2420GPE::NumModes()
{
    int count;

    DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::NumModes()\r\n"));
    count = DisplayPddNumModes(m_hDisplay);
    DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::NumModes(count = %d)\r\n", count));
    return count;
}

//------------------------------------------------------------------------------
//
//  Method:  GetModeInfo
//
//  This method returns mode information for mode number of modes supported
//  by display.  On OMAP2420 mode info depend on LCD hardware, so ask it.
//
SCODE OMAP2420GPE::GetModeInfo(GPEMode* pMode, int modeNumber)
{
    SCODE sc = E_FAIL;
    OMAP2420_DISPLAY_MODE_INFO info;

    DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::GetModeInfo(0x%08x, %d, Bpp=%d)\r\n", pMode, modeNumber, m_bitsPerPixel));

    // Get hardware info
    if (!DisplayPddGetModeInfo(m_hDisplay, modeNumber, &info)) {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::GetModeInfo: "
            L"Failed get mode info from display for mode %d\r\n", modeNumber
        ));
        goto cleanUp;
    }

    // Now convert it to logical info
    pMode->modeId = modeNumber;
    pMode->width  = info.width;
    pMode->height = info.height;
    pMode->Bpp = m_bitsPerPixel;
    pMode->frequency = 60;
    switch (m_bitsPerPixel) {

    case 1:
        pMode->format = gpe1Bpp;
        break;
    case 2:
        pMode->format = gpe2Bpp;
        break;
    case 4:
        pMode->format = gpe4Bpp;
        break;
    case 8:
        pMode->format = gpe8Bpp;
        break;
    case 16:
        pMode->format = gpe16Bpp;
        break;
    default:
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::GetModeInfo: "
            L"Unsupported display mode with %d bits per pixel\r\n",
            m_bitsPerPixel
        ));
        goto cleanUp;
    }

    // Done
    sc = S_OK;

cleanUp:
    
    DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::GetModeInfo(sc = 0x%08x)\r\n", sc));
    return sc;
}

//------------------------------------------------------------------------------
//
//  Method:  SetMode
//
//  This method should set display hardware to given mode.
//
SCODE OMAP2420GPE::SetMode(int modeNumber, HPALETTE *phPalette)
{
    SCODE sc = E_FAIL;
    OMAP2420_DISPLAY_MODE_INFO info;
    GPEMode gpeMode;

    DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::SetMode.(%d, 0x%08x)\r\n", modeNumber, phPalette));

    // get the hardware info
    if (!DisplayPddGetModeInfo(m_hDisplay, modeNumber, &info))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::SetMode: "
            L"Failed get mode info from display for mode %d\r\n", modeNumber));
        goto cleanUp;
    }

    // get the mode info
    sc = GetModeInfo(&gpeMode, modeNumber);
    if (FAILED(sc))
    {
        goto cleanUp;
    }
    
    // Create palette depending on format
    switch (gpeMode.format) {
    case gpe16Bpp:
        *phPalette = EngCreatePalette(
            PAL_BITFIELDS, 0, NULL, info.red, info.green, info.blue
        );
        g_bitMasks[0] = info.red;
        g_bitMasks[1] = info.green;
        g_bitMasks[2] = info.blue;
        break;
    default:
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP730GPE::SetMode: "
            L"Unsupported format mode (%d)\r\n", gpeMode.format
        ));
        goto cleanUp;
    }
    
    if (*phPalette == NULL) {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP730GPE::SetMode: "
            L"Palette creation failed\r\n"
        ));
        sc = E_OUTOFMEMORY;
        goto cleanUp;
    }

    // We want set hardware to D0, but it can be
    // overpowered by external state. We have to kick 
    // power thread if there was real power change.
    m_updateFlag = TRUE;
    if (SetPower(D0))
    {
        SetEvent(m_hPowerEvent);
    }

    //----------------------------------------------------------------------
    //  Configure LCD hardware
    //----------------------------------------------------------------------

    // Set LCD controller to new hardware state
    if (!DisplayPddSetMode(m_hDisplay, modeNumber))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::SetMode: "
            L"LCD failed set mode %d\r\n", modeNumber));
        goto cleanUp;
    }

    //----------------------------------------------------------------------
    //  Create primary surface
    //----------------------------------------------------------------------

    // Set mode information
    m_gpeMode = gpeMode;
    m_pMode = &m_gpeMode;
    m_nScreenWidth  = m_pMode->width;
    m_nScreenHeight = m_pMode->height;
    
    // Allocate new primary surface
    delete m_pPrimarySurface;
    sc = AllocSurface(&m_pPrimarySurface, m_pMode->width, m_pMode->height, m_pMode->format, GPE_REQUIRE_VIDEO_MEMORY);

    // When it failed, there isn't so much we can do
    if (FAILED(sc)) {
        DEBUGMSG(GPE_ZONE_ERROR, (L"OMAP2420GPE::SetMode: "

            L"Couldn't allocate primary surface\r\n"));
        goto cleanUp;
    }

    // Set base rotation
    m_pPrimarySurface->SetRotation(m_nScreenWidth, m_nScreenHeight, DMDO_0);
    
    //----------------------------------------------------------------------
    //  Configure OMAP2420 DISP controller
    //----------------------------------------------------------------------

    // initialize the display registers
    InitController(&info);

    // make the primary surface visible and tell the controller to go with the new values
    SetVisibleSurface(m_pPrimarySurface, 0, FALSE);

    // reinitialize TV-Out
#if defined(TVOUT)
    SetTVOut(m_dwTVOut);
#endif

    // Done
    sc = S_OK;

cleanUp:

    DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::SetMode(sc = 0x%08x)\r\n", sc));
    return sc;
}

//------------------------------------------------------------------------------
//
//  Method:  AllocSurface
//
//  This method is called for all surface allocations from ddgpe and gpe
//
SCODE OMAP2420GPE::AllocSurface(GPESurf **ppSurf, int nWidth, int nHeight, EGPEFormat nFormat, int nFlags)
{
    SCODE sc = S_OK;
    DWORD pa;

    // Is video memory required?
    if ((nFlags & (GPE_REQUIRE_VIDEO_MEMORY|GPE_BACK_BUFFER)) != 0)
        {
        
        ULONG stride = (nWidth * EGPEFormatToBpp[nFormat] + 7) >> 3;
        ULONG size = MulDiv(stride, nHeight, 1);
        if (size == -1)
            {
            sc = E_OUTOFMEMORY;
            goto cleanUp;
            }

        VOID *pSurface = AllocPhysMem(size, PAGE_READWRITE, 0, 0, &pa);
        if (pSurface == NULL)
            {
            DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::AllocSurface: "
                L"Couldn't allocate video memory in SDRAM\r\n"
                ));
            sc = E_OUTOFMEMORY;
            goto cleanUp;
            }

        OMAP2420Surf *pSurf = new OMAP2420Surf(
            nWidth, nHeight, pa, pSurface, stride, nFormat
            );
        
        if (pSurf == NULL)
            {
            // When allocation failed, we are out of memory...
            FreePhysMem(pSurface);
            sc = E_OUTOFMEMORY;
            goto cleanUp;
            }

        // sanity check, cached mapping may have failed in constructor
        if (!pSurf->SurfaceOk())
            {
            // When allocation failed, we are out of memory...
            delete pSurf;
            sc = E_OUTOFMEMORY;
            goto cleanUp;
            }

        *ppSurf = pSurf;

        // We are done
        DEBUGMSG(GPE_ZONE_PDD, (L"OMAP2420GPE::AllocSurface: "
            L"Surface allocated in SDRAM memory (pa: 0x%08x, va: 0x%08x, "
            L"width: %d, height: %d, format %d)\r\n",
            pa, pSurface, nWidth, nHeight, nFormat
            )); 

        goto cleanUp;
        }

    if ((nFlags & GPE_DDGPE_SURF) != 0)
        {
        // Create DDGPE surface in memory
        *ppSurf = new DDGPESurf(nWidth, nHeight, nFormat);
        }
    else 
        {
        // Create GPE surface in memory
        *ppSurf = new GPESurf(nWidth, nHeight, nFormat);
        }
    
    // Check for failure
    if ((*ppSurf == NULL) || ((*ppSurf)->Buffer() == NULL))
        {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::AllocSurface: "
            L"Failed allocate surface (width: %d, height: %d, format %d)\r\n",
            nWidth, nHeight, nFormat
            ));
        delete *ppSurf;
        *ppSurf = NULL;
        sc = E_OUTOFMEMORY;
        goto cleanUp;
        }

    // We are done
    DEBUGMSG(GPE_ZONE_PDD, (L"OMAP2420GPE::AllocSurface: "
        L"Surface in memory (width: %d, height: %d, format %d, 0x%08x)\r\n",
        nWidth, nHeight, nFormat, (*ppSurf)->Buffer()
        ));

cleanUp:
    return sc;

}


//------------------------------------------------------------------------------
//
//  Method:  AllocSurface
//
//  This method is used to allocate DirectDraw enabled surfaces
//
//
SCODE
OMAP2420GPE::AllocSurface(DDGPESurf **ppSurf, int width, int height, EGPEFormat format, EDDGPEPixelFormat pixelFormat, int flags)
{
    return AllocSurface(
        (GPESurf **)ppSurf, width, height, format, flags | GPE_DDGPE_SURF
        );
}


//-----------------------------------------------------------------------------
//
//  SurfaceBusyFlipping
//
//  This function returns TRUE if a flip is still pending. Returning TRUE
//  prevents DirectDraw functions like Blt and Lock to use surfaces which
//  are still visible
//
BOOL OMAP2420GPE::SurfaceBusyFlipping(GPESurf *pSurf)
{
#if 0
    DEBUGMSG(TRUE,(L"Check for SurBusy\r\n"));
    // Check the DISPC_IRQSTATUS_GFXEndWindow bit.
    // If bit is set, it means all the data from GFX Window has been
    // fetched from memory and displayed on screen.
    // To clear the bit, write 1 to it.
//    while((INREG32(&m_pDISPCRegs->ulDISPC_IRQSTATUS) & 0x80)==0) Sleep(1);
    
    // Disable GFX window
//    CLRREG32(&m_pDISPCRegs->ulDISPC_GFX_ATTRIBUTES, DISPC_GFX_ATTR_GFXENABLE);

    // enable the LCD and wait for it to complete
//    SETREG32(&m_pDISPCRegs->ulDISPC_CONTROL, DISPC_CONTROL_LCDENABLE | DISPC_CONTROL_GOLCD);
//    while (INREG32(&m_pDISPCRegs->ulDISPC_CONTROL) & DISPC_CONTROL_GOLCD) Sleep(0);
   
    m_flipBusy = FALSE;

    // Clear the EOL Status interrupt.
//    SETREG32(&m_pDISPCRegs->ulDISPC_IRQSTATUS, 0x80);

    DEBUGMSG(TRUE,(L"Surface is free\r\n"));
#endif
    m_flipBusy = FALSE;
    return FALSE;
}

//-----------------------------------------------------------------------------
//
//  Method:  SetVisibleSurface
//
//  Makes a surface visible, either as the main graphics layer or as an overlay.
//
VOID OMAP2420GPE::SetVisibleSurface(GPESurf* pGPESurf, DWORD data, BOOL waitForVBlank)
{
    OMAP2420Surf *pSurf = (OMAP2420Surf *)pGPESurf;

    DEBUGMSG(GPE_ZONE_PDD, (L"OMAP2420GPE::SetVisibleSurface: %08X %d\r\n", pSurf, pSurf->IsOverlay()));

    // get the buffer address
    if (pSurf->OffsetInVideoMemory() == 0)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE::SetVisibleSurface: "
            L"Surface physical address is zero...\r\n"));
        return;
    }

    // We want set hardware to D0, but it can be
    // overpowered by external state. We have to kick 
    // power thread if there was real power change.
    m_updateFlag = TRUE;
    if (SetPower(D0))
    {
        SetEvent(m_hPowerEvent);
    }

    // update the visible pointer
    if (pSurf->IsOverlay())
    {
        if (m_pVisibleOverlay)
        {
            pSurf->m_pVidRegs = m_pVisibleOverlay->m_pVidRegs;
            m_pVisibleOverlay->m_pVidRegs = NULL;
        }
        else
        {
            pSurf->m_pVidRegs = &m_pDISPCRegs->tDISPC_VID1;
        }
        m_pVisibleOverlay = pSurf;
    }
    else
    {
        m_pVisibleSurface = pSurf;
    }

    // update the video addresses	
    SetControllerAddresses();
}

//------------------------------------------------------------------------------

DWORD OMAP2420GPE::UpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA pUpdate)
{
    DWORD               dwBA0, dwBA1, dwAttr, dwPixelInc, dwRowInc, dwBytesPerPixel, dwWidth;
    DDGPESurf           *pSrcSurf = DDGPESurf::GetDDGPESurf(pUpdate->lpDDSrcSurface);
    DDGPESurf           *pDestSurf = DDGPESurf::GetDDGPESurf(pUpdate->lpDDDestSurface);
    OMAP2420_VID_REGS   *pVidRegs = &m_pDISPCRegs->tDISPC_VID1;

    DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::UpdateOverlay: %08X %08X %08X %08X\r\n", pSrcSurf, pDestSurf, m_pVisibleOverlay, pUpdate->dwFlags));

    // hide the surface if requested
    if (pUpdate->dwFlags & DDOVER_HIDE)
    {
        if (pSrcSurf == m_pVisibleOverlay)
        {
            DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::UpdateOverlay: HIDE\r\n"));

            // disable the overlay
            CLRREG32(&pVidRegs->ulATTRIBUTES, DISPC_VID_ATTR_VIDENABLE);

            // commit the display controller changes
            SETREG32(&m_pDISPCRegs->ulDISPC_CONTROL, DISPC_CONTROL_LCDENABLE | DISPC_CONTROL_GOLCD);
            while (INREG32(&m_pDISPCRegs->ulDISPC_CONTROL) & DISPC_CONTROL_GOLCD);

            // clear the visible overlay pointer
            m_pVisibleOverlay->m_pVidRegs = NULL;
            m_pVisibleOverlay = NULL;
        }

        pUpdate->ddRVal = DD_OK;
        DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::UpdateOverlay: HIDDEN\r\n"));
        return DDHAL_DRIVER_HANDLED;
    }

    // if this isn't the visible overlay
    if (pSrcSurf != m_pVisibleOverlay)
    {
        // and we haven't been asked to make it visible, there's nothing to do
        if (!(pUpdate->dwFlags & DDOVER_SHOW))
        {
            pUpdate->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }
        // it's an error if some other overlay is already visible
        else if (m_pVisibleOverlay != NULL)
        {
            DEBUGMSG(1, (L"OMAP2420GPE::UpdateOverlay: "
                L"ERROR: Other overlay already visible!\r\n"));
            pUpdate->ddRVal = DDERR_OUTOFCAPS;
            return DDHAL_DRIVER_HANDLED;
        }
    }

    // enable/disable the color key
    if (pUpdate->dwFlags & DDOVER_KEYSRC)
    {
        OUTREG32(&m_pDISPCRegs->ulDISPC_TRANS_COLOR0, pSrcSurf->ColorKey());
        SETREG32(&m_pDISPCRegs->ulDISPC_CONFIG, DISPC_CONFIG_TCKLCDSELECTION);
        SETREG32(&m_pDISPCRegs->ulDISPC_CONFIG, DISPC_CONFIG_TCKLCDENABLE);
    }
    else if (pUpdate->dwFlags & DDOVER_KEYSRCOVERRIDE)
    {
        OUTREG32(&m_pDISPCRegs->ulDISPC_TRANS_COLOR0, pUpdate->overlayFX.dckSrcColorkey.dwColorSpaceLowValue);
        SETREG32(&m_pDISPCRegs->ulDISPC_CONFIG, DISPC_CONFIG_TCKLCDSELECTION);
        SETREG32(&m_pDISPCRegs->ulDISPC_CONFIG, DISPC_CONFIG_TCKLCDENABLE);
    }
    else if (pUpdate->dwFlags & DDOVER_KEYDEST)
    {
        OUTREG32(&m_pDISPCRegs->ulDISPC_TRANS_COLOR0, pDestSurf->ColorKey());
        CLRREG32(&m_pDISPCRegs->ulDISPC_CONFIG, DISPC_CONFIG_TCKLCDSELECTION);
        SETREG32(&m_pDISPCRegs->ulDISPC_CONFIG, DISPC_CONFIG_TCKLCDENABLE);
    }
    else if (pUpdate->dwFlags & DDOVER_KEYDESTOVERRIDE)
    {
        OUTREG32(&m_pDISPCRegs->ulDISPC_TRANS_COLOR0, pUpdate->overlayFX.dckDestColorkey.dwColorSpaceLowValue);
        CLRREG32(&m_pDISPCRegs->ulDISPC_CONFIG, DISPC_CONFIG_TCKLCDSELECTION);
        SETREG32(&m_pDISPCRegs->ulDISPC_CONFIG, DISPC_CONFIG_TCKLCDENABLE);
    }
    else
    {
        CLRREG32(&m_pDISPCRegs->ulDISPC_CONFIG, DISPC_CONFIG_TCKLCDENABLE);
    }

    // set the source address, rotation parameters, and attributes
    // based on the source format
    switch (pSrcSurf->Format())
    {
        // 16-bit RGB
        case gpe16Bpp:
            dwBytesPerPixel = EGPEFormatToBpp[pSrcSurf->Format()] / 8;
            dwAttr = DISPC_VID_ATTR_VIDFORMAT(6);
            switch (pSrcSurf->Rotate())
            {
                case DMDO_0:
                dwBA0 = pSrcSurf->OffsetInVideoMemory();
                dwPixelInc = 1;
                dwRowInc = 1;
                break;
                case DMDO_90:
                dwBA0 = pSrcSurf->OffsetInVideoMemory() + (pSrcSurf->Width() * (pSrcSurf->Height() - 1) * dwBytesPerPixel);
                dwPixelInc = -pSrcSurf->Width() * dwBytesPerPixel - (dwBytesPerPixel - 1);
                dwRowInc = (pSrcSurf->Width() * (pSrcSurf->Height() - 1)) * dwBytesPerPixel + 1;
                break;
                case DMDO_180:
                dwBA0 = pSrcSurf->OffsetInVideoMemory() + (pSrcSurf->Width() * pSrcSurf->Height() - 1) * dwBytesPerPixel;
                dwPixelInc = -(LONG)dwBytesPerPixel - (dwBytesPerPixel - 1);
                dwRowInc = -(LONG)dwBytesPerPixel - (dwBytesPerPixel - 1);
                break;
                case DMDO_270:
                dwBA0 = pSrcSurf->OffsetInVideoMemory() + (pSrcSurf->Width() - 1) * dwBytesPerPixel;
                dwPixelInc = (pSrcSurf->Width() - 1) * dwBytesPerPixel + 1;
                dwRowInc = -(pSrcSurf->Width() * (pSrcSurf->Height() - 1)) * dwBytesPerPixel - dwBytesPerPixel - (dwBytesPerPixel - 1);
                break;
            }
            dwBA1 = dwBA0;
        break;

        // 16-bit YCrCb (??)
        case gpe16YCrCb:
            dwBytesPerPixel = 4;
            dwWidth = pSrcSurf->Width() / 2;
            dwAttr = DISPC_VID_ATTR_VIDFORMAT(10); // ??? 10 (YUV2 4:2:2) or 11(UYVY 4:2:2)
            switch (pSrcSurf->Rotate())
            {
                case DMDO_0:
                dwBA0 = pSrcSurf->OffsetInVideoMemory();
                dwPixelInc = 1;
                dwRowInc = 1;
                break;
                case DMDO_90:
                dwBA0 = pSrcSurf->OffsetInVideoMemory() + (dwWidth * (pSrcSurf->Height() - 1) * dwBytesPerPixel);
                dwPixelInc = -(LONG)dwWidth * dwBytesPerPixel - (dwBytesPerPixel - 1);
                dwRowInc = (dwWidth * (pSrcSurf->Height() - 1)) * dwBytesPerPixel + 1;
                break;
                case DMDO_180:
                dwBA0 = pSrcSurf->OffsetInVideoMemory() + (dwWidth * pSrcSurf->Height() - 1) * dwBytesPerPixel;
                dwPixelInc = -(LONG)dwBytesPerPixel - (dwBytesPerPixel - 1);
                dwRowInc = -(LONG)dwBytesPerPixel - (dwBytesPerPixel - 1);
                break;
                case DMDO_270:
                dwBA0 = pSrcSurf->OffsetInVideoMemory() + (dwWidth - 1) * dwBytesPerPixel;
                dwPixelInc = (dwWidth - 1) * dwBytesPerPixel + 1;
                dwRowInc = -((LONG)dwWidth * (pSrcSurf->Height() - 1)) * dwBytesPerPixel - dwBytesPerPixel - (dwBytesPerPixel - 1);
                break;
            }
            dwBA1 = dwBA0;
        break;

        // all others are errors
        default:
            DEBUGMSG(GPE_ZONE_ERROR, (L"OMAP2420GPE::UpdateOverlay: "
                L"Invalid source format: %d!\r\n", pSrcSurf->Format()));
            pUpdate->ddRVal = DDERR_OUTOFCAPS;
            return DDHAL_DRIVER_HANDLED;
        break;
    }

    // set the source address, rotation parameters, and attributes (including format)
    OUTREG32(&pVidRegs->ulBA0, dwBA0);
    OUTREG32(&pVidRegs->ulBA1, dwBA1);
    OUTREG32(&pVidRegs->ulPIXEL_INC, dwPixelInc);
    OUTREG32(&pVidRegs->ulROW_INC, dwRowInc);
    OUTREG32(&pVidRegs->ulATTRIBUTES, (INREG32(&pVidRegs->ulATTRIBUTES) & DISPC_VID_ATTR_VIDENABLE) | dwAttr);

    // set the source size (position is 0,0)
    OUTREG32(&pVidRegs->ulPICTURE_SIZE, DISPC_VID_PICTURE_SIZE_VIDORGSIZEY(pUpdate->rSrc.bottom - pUpdate->rSrc.top) |
    DISPC_VID_PICTURE_SIZE_VIDORGSIZEX(pUpdate->rSrc.right - pUpdate->rSrc.left));

    // set the destination position and size
    OUTREG32(&pVidRegs->ulPOSITION, DISPC_VID_POS_VIDPOSY(pUpdate->rDest.top) | DISPC_VID_POS_VIDPOSX(pUpdate->rDest.left));
    OUTREG32(&pVidRegs->ulSIZE, DISPC_VID_SIZE_VIDSIZEY(pUpdate->rDest.bottom - pUpdate->rDest.top) |
    DISPC_VID_SIZE_VIDSIZEX(pUpdate->rDest.right - pUpdate->rDest.left));

    // set the overlay attributes
    OUTREG32(&pVidRegs->ulFIFO_THRESHOLD, BSP_TV_FIFO_THRESHOLD);

    // show the surface if requested
    if (pUpdate->dwFlags & DDOVER_SHOW)
    {
        DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::UpdateOverlay: SHOW\r\n"));

        // enable the overlay
        SETREG32(&pVidRegs->ulATTRIBUTES, DISPC_VID_ATTR_VIDENABLE);

        // set the visible overlay pointer
        m_pVisibleOverlay = (OMAP2420Surf *)pSrcSurf;
        m_pVisibleOverlay->m_pVidRegs = pVidRegs;
    }

    // commit the display controller changes
    SETREG32(&m_pDISPCRegs->ulDISPC_CONTROL, DISPC_CONTROL_LCDENABLE | DISPC_CONTROL_GOLCD);
    while (INREG32(&m_pDISPCRegs->ulDISPC_CONTROL) & DISPC_CONTROL_GOLCD);

#if defined(DEBUG)
    DumpRegs(L"UpdateOverlay");
#endif

    // indicate success
    DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::UpdateOverlay\r\n"));
    pUpdate->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

//------------------------------------------------------------------------------

DWORD OMAP2420GPE::SetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA pSetPos)
{
    DDGPESurf           *pSurf = DDGPESurf::GetDDGPESurf(pSetPos->lpDDSrcSurface);
    OMAP2420_VID_REGS   *pVidRegs = &m_pDISPCRegs->tDISPC_VID1;

    DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::SetOverlayPosition: %08X %d %d\r\n", pSurf, pSetPos->lXPos, pSetPos->lYPos));

    // must be the current surface
    if (pSurf != m_pVisibleOverlay)
    {
        DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::SetOverlayPosition: "
            L"NOT visible\r\n"));
        pSetPos->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }

    // set the overlay position
    OUTREG32(&pVidRegs->ulPOSITION, DISPC_VID_POS_VIDPOSY(pSetPos->lYPos) | DISPC_VID_POS_VIDPOSX(pSetPos->lXPos));

    // commit the display controller changes
    SETREG32(&m_pDISPCRegs->ulDISPC_CONTROL, DISPC_CONTROL_LCDENABLE | DISPC_CONTROL_GOLCD);
    while (INREG32(&m_pDISPCRegs->ulDISPC_CONTROL) & DISPC_CONTROL_GOLCD);

    // indicate success
    DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::SetOverlayPosition\r\n"));
    pSetPos->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

//------------------------------------------------------------------------------
//
//  Method: Line
//
//  This method executes before and after a sequence of line segments,
//  which are drawn as a path. It examines the line parameters to determine
//  whether the operation can be accelerated. It also places a pointer to
//  a function to execute once per line segment into the pLine member
//  of the GPELineParms structure.
//
SCODE OMAP2420GPE::Line(GPELineParms *pLineParms, EGPEPhase phase)
{
    DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::Line\r\n"));
    if (phase == gpeSingle || phase == gpePrepare)
    {
        // We want set hardware to D0, but it can be
        // overpowered by external state. We have to kick 
        // power thread if there was real power change.
        m_updateFlag = TRUE;
        if (SetPower(D0))
        {
            SetEvent(m_hPowerEvent);
        }
        // Use wrapper to make sure that we don't move to low power...
        pLineParms->pLine = (LINEFUNC)&OMAP2420GPE::WrappedEmulatedLine;
    }
    return S_OK;
}

//------------------------------------------------------------------------------
//
//  Method: WrapperEmulatedLine
//
SCODE OMAP2420GPE::WrappedEmulatedLine(GPELineParms *lineParameters)
{
    SCODE sc;
    DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::WrappedEmulatedLine\r\n"));
    // Do emulated line
    sc = EmulatedLine(lineParameters);
    // Set update flag
    m_updateFlag = TRUE;
    // Done   
    return sc;
}

//------------------------------------------------------------------------------
//
//  Method:  BltPrepare
//
//  This method identifies the appropriate functions needed to perform
//  individual blits. This function executes before a sequence of clipped blit
//  operations.
//
SCODE OMAP2420GPE::BltPrepare(GPEBltParms *pBltParms)
{
    // We want set hardware to D0, but it can be
    // overpowered by external state. We have to kick 
    // power thread/ if there was real power change.
    m_updateFlag = TRUE;
    if (SetPower(D0))
    {
        SetEvent(m_hPowerEvent);
    }
    
    pBltParms->pBlt = &GPE::EmulatedBlt;

#if 0    
    if (pBltParms->pDst->InVideoMemory()) {
        ASSERT( pBltParms->pDst->Format() == gpe16Bpp);

// removing this code fixes LTK test 4018, however disables hardware (DMA based) acceleration
/*
        switch(pBltParms->rop4) {
        case 0x0000:  // BLACKNESS
            pBltParms->solidColor = 0;
            pBltParms->pBlt = (BLTFUNC)HWFill16;
            break;

        case 0xFFFF:  // WHITENESS
            pBltParms->solidColor = 0x0000FFFF;
            pBltParms->pBlt = (BLTFUNC)HWFill16;
            break;

        case 0xF0F0:  // PATCOPY
            // Solid color only
            if (pBltParms->solidColor != -1) {
                pBltParms->pBlt = (BLTFUNC)HWFill16;
            }
            break;

        case 0xCCCC:  // SRCCOPY
            if( (pBltParms->pSrc->InVideoMemory()) &&        // only in SRAM
                !pBltParms->pLookup  &&                      // No color...
                !pBltParms->pConvert &&                      // ...conversions
                !(pBltParms->bltFlags & (BLT_STRETCH |       // No stretch
                                         BLT_TRANSPARENT |   // No transparency
                                         BLT_ALPHABLEND |    // No alpha
                                         BLT_DSTTRANSPARENT)))
            {
                ASSERT(pBltParms->pSrc->Format()==gpe16Bpp);
                pBltParms->pBlt = (BLTFUNC)HWSrcCopy1616;
            }
            break;

        }
*/

        if (m_flipBusy) {
            // SurfaceBusyFlipping only blocks the primary and
            // back buffer surface on a flip
            while (SurfaceBusyFlipping(pBltParms->pDst)) Sleep(1);
        }

        if (pBltParms->pBlt == EmulatedBlt)
        {
            static_cast<OMAP2420Surf*>(pBltParms->pDst)->InCache();
        }
    }
#endif

    // Set update flag
    m_updateFlag = TRUE;

    // Done
    return S_OK;
}

//------------------------------------------------------------------------------
//
//  Method:  BltComplete
//
//  This method executes to complete a blit sequence.
//
SCODE OMAP2420GPE::BltComplete(GPEBltParms *pBtlParms)
{
    // Set update flag
    m_updateFlag = TRUE;
    return S_OK;
}

//------------------------------------------------------------------------------
//
//  Method:  SetPointerShape
//
//  This method sets the shape of the pointer, the hot spot of the pointer,
//  and the colors to use for the cursor if the cursor is multicolored.
//
SCODE OMAP2420GPE::SetPointerShape(GPESurf* pMask, GPESurf* pColorSurf, int xHotspot, int yHotspot,
                                    int xSize, int ySize)
{
    DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::SetPointerShape\r\n"));
    DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::SetPointerShape\r\n"));
    return S_OK;
}

//------------------------------------------------------------------------------
//
//  Method:  MovePointer
//
//  This method executes from applications either to move the hot spot
//  of the cursor to a specific screen location or to hide the cursor (x == -1).
//
SCODE OMAP2420GPE::MovePointer(int x, int y)
{
    DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::MovePointer(%d, %d)\r\n", x, y));
    DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::MovePointer(sc = 0x%08x)\r\n", S_OK));
    return S_OK;
}

//------------------------------------------------------------------------------
//
//  Method:  InVBlank
//
int OMAP2420GPE::InVBlank()
{
    //DEBUGMSG(TRUE,(L"InVBlank++\r\n"));

    //To do set the required number of Vertical Blank pulses.
    //DEBUGMSG(TRUE,(L"InVBlank--\r\n"));

    return 1;
}

//------------------------------------------------------------------------------
//
//  Method:  PowerHandler
//
VOID OMAP2420GPE::PowerHandler(BOOL off)
{
}

//------------------------------------------------------------------------------
//
//  Method:  SetPower
//
//  This method sets device to given power state. It returns TRUE when
//  there was real change in power state.
//
BOOL OMAP2420GPE::SetPower(CEDEVICE_POWER_STATE dx)
{
    UCHAR   ucData;
    BOOL    bRetVal = FALSE;
    DWORD   regBit,cbRet;

//    DEBUGMSG(GPE_ZONE_PDD, (L"OMAP2420GPE::SetPower State - %d\r\n",dx));
    
    EnterCriticalSection(&m_powerCS);

    // Device can't be set to greater power state than external (ceiling)
    if (dx < m_externalDX)
    {
        dx = m_externalDX;
    }

    // only do something if the state has changed
    if (m_currentDX != dx)
    {
        // Let display hardware set LCD device 
        DisplayPddSetPower(m_hDisplay, dx);
            
        if (dx <= D2) 
        {        
            //Enable functional clocks
            regBit = PRCM_FCLKEN1_CORE_EN_DSS1;
            KernelIoControl(IOCTL_FCLK1_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
            regBit = PRCM_FCLKEN1_CORE_EN_DSS2;
            KernelIoControl(IOCTL_FCLK1_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
            regBit = PRCM_FCLKEN1_CORE_EN_TV;
            KernelIoControl(IOCTL_FCLK1_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);

            //Enable interface clocks
            regBit = PRCM_ICLKEN1_CORE_EN_DSS;
            KernelIoControl(IOCTL_ICLK1_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
    
            //Enable LCD - These should be a single bit-set operation in the I2C driver
            bRetVal = ReadI2CIO(I2C_DISPLAY_ENABLE_ADDRESS, I2C_DISPLAY_ENABLE_ADDRSIZE, &ucData);
            if (bRetVal)
            {
                //set the LCD_ENVDD bit
                ucData |= (1 << 7);

                // write the new value
                bRetVal = WriteI2CIO(I2C_DISPLAY_ENABLE_ADDRESS, I2C_DISPLAY_ENABLE_ADDRSIZE, ucData);
            }            
        }            
        else if (dx >= D3) 
        {        
            //Disable functional clocks
            regBit = PRCM_FCLKEN1_CORE_EN_DSS1;
            KernelIoControl(IOCTL_FCLK1_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
            regBit = PRCM_FCLKEN1_CORE_EN_DSS2;
            KernelIoControl(IOCTL_FCLK1_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
            regBit = PRCM_FCLKEN1_CORE_EN_TV;
            KernelIoControl(IOCTL_FCLK1_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
    
            //Disable interface clocks
            regBit = PRCM_ICLKEN1_CORE_EN_DSS;
            KernelIoControl(IOCTL_ICLK1_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
                
            //Disable LCD - These should be a single bit-clear operation in the I2C driver
            bRetVal = ReadI2CIO(I2C_DISPLAY_ENABLE_ADDRESS, I2C_DISPLAY_ENABLE_ADDRSIZE, &ucData);
            if (bRetVal)
            {
                //set the LCD_ENVDD bit
                ucData &= ~(1 << 7);

                // write the new value
                bRetVal = WriteI2CIO(I2C_DISPLAY_ENABLE_ADDRESS, I2C_DISPLAY_ENABLE_ADDRSIZE, ucData);
            }            
        }
    
        // Update current power state
        m_currentDX = dx;
        bRetVal = TRUE;
    }


    // If the power is on, there's a visible surface, and the SYNCLOST bug occurs, then reset everything
    // Temporary Black Screen bug fix - See OMAP2420 Silicon Errata - 1.9 and 1.10
    if (m_currentDX <= D2 && m_pVisibleSurface && INREG32(&m_pDISPCRegs->ulDISPC_IRQSTATUS) & DISPC_IRQSTATUS_SYNCLOST)
    {
        DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::SYNCLOST BUG IRQ: %x\r\n", INREG32(&m_pDISPCRegs->ulDISPC_IRQSTATUS)));

        // Get hardware info
        OMAP2420_DISPLAY_MODE_INFO info;
        DisplayPddGetModeInfo(m_hDisplay, 0, &info);

        //Need to reset DSS fully to clear the interrupt
        SETREG32(&m_pDSSRegs->ulDSS_SYSCONFIG, DSS_SYSCONFIG_SOFTRESET);
        while (!(INREG32(&m_pDSSRegs->ulDSS_SYSSTATUS) & DSS_SYSSTATUS_RESETDONE)) Sleep(0);

        //----------------------------------------------------------------------
        //  Configure OMAP2420 DISP controller
        //----------------------------------------------------------------------

        // initialize the display registers
        InitController(&info);

        // update the video addresses
        SetControllerAddresses();
    }

    LeaveCriticalSection(&m_powerCS);
//    DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::SetPower State - %d\r\n",dx));
    return bRetVal;
}

//------------------------------------------------------------------------------
//
//  Method:  InitController
//
//  Resets the display controller and initializes the shadow registers.
//  Must be followed by a call to SetControllerAddresses to commit the changed.
//

void OMAP2420GPE::InitController(OMAP2420_DISPLAY_MODE_INFO *pInfo)
{
    // reset the display controller and wait for it to complete
    OUTREG32(&m_pDISPCRegs->ulDISPC_SYSCONFIG, DISPC_SYSCONFIG_SOFTRESET); 
    while (!(INREG32(&m_pDISPCRegs->ulDISPC_SYSSTATUS) & DISPC_SYSSTATUS_RESETDONE)) Sleep(0);

    // disable the display controller and IRQ
    OUTREG32(&m_pDISPCRegs->ulDISPC_CONTROL, 0);
    OUTREG32(&m_pDISPCRegs->ulDISPC_IRQENABLE,0);
    OUTREG32(&m_pDISPCRegs->ulDISPC_IRQSTATUS, 0x7FFF);

    // set the display controller attributes
    OUTREG32(&m_pDISPCRegs->ulDISPC_SYSCONFIG, pInfo->sysconfig);
    OUTREG32(&m_pDISPCRegs->ulDISPC_CONTROL, pInfo->control);
    OUTREG32(&m_pDISPCRegs->ulDISPC_CONFIG, pInfo->config);

    // set the LCD attributes
    OUTREG32(&m_pDISPCRegs->ulDISPC_TIMING_H, pInfo->timing_H);
    OUTREG32(&m_pDISPCRegs->ulDISPC_TIMING_V, pInfo->timing_V);
    OUTREG32(&m_pDISPCRegs->ulDISPC_POL_FREQ, pInfo->pol_freq);
    OUTREG32(&m_pDISPCRegs->ulDISPC_DIVISOR, pInfo->divisor);
    OUTREG32(&m_pDISPCRegs->ulDISPC_SIZE_LCD, pInfo->size_LCD);
    OUTREG32(&m_pDISPCRegs->ulDISPC_SIZE_DIG, pInfo->size_DIG);
    OUTREG32(&m_pDISPCRegs->ulDISPC_DEFAULT_COLOR0, pInfo->background_LCD);
    OUTREG32(&m_pDISPCRegs->ulDISPC_DEFAULT_COLOR1, pInfo->background_DIG);

    // set the graphic attributes
    OUTREG32(&m_pDISPCRegs->ulDISPC_GFX_POSITION, pInfo->GFX_pos);
    OUTREG32(&m_pDISPCRegs->ulDISPC_GFX_SIZE, pInfo->GFX_size);
    OUTREG32(&m_pDISPCRegs->ulDISPC_GFX_ATTRIBUTES, pInfo->GFX_attr);
    OUTREG32(&m_pDISPCRegs->ulDISPC_GFX_FIFO_THRESHOLD, pInfo->GFX_FIFO_thres);
    OUTREG32(&m_pDISPCRegs->ulDISPC_GFX_ROW_INC, pInfo->GFX_row_inc);
    OUTREG32(&m_pDISPCRegs->ulDISPC_GFX_PIXEL_INC, pInfo->GFX_pixel_inc);
    OUTREG32(&m_pDISPCRegs->ulDISPC_GFX_WINDOW_SKIP, pInfo->GFX_window_skip);

#if defined(TVOUT)
    // set the video attributes for TV-Out
    OUTREG32(&m_pTVVidRegs->ulPOSITION, BSP_TV_POS);
    OUTREG32(&m_pTVVidRegs->ulSIZE, BSP_TV_SIZE);
    OUTREG32(&m_pTVVidRegs->ulPICTURE_SIZE, BSP_TV_PICTURE_SIZE);
    OUTREG32(&m_pTVVidRegs->ulATTRIBUTES, BSP_TV_ATTR);
    OUTREG32(&m_pTVVidRegs->ulFIFO_THRESHOLD, BSP_TV_FIFO_THRESHOLD);
    OUTREG32(&m_pTVVidRegs->ulROW_INC, BSP_TV_ROW_INC);
    OUTREG32(&m_pTVVidRegs->ulPIXEL_INC, BSP_TV_PIXEL_INC);
#endif
}

//------------------------------------------------------------------------------
//
//  Method:  SetControllerAddresses
//
//  This method sets device to given power state. It returns TRUE when
//  there was real change in power state.
//

void OMAP2420GPE::SetControllerAddresses(void)
{
    DEBUGMSG(TRUE,(L"OMAP2420GPE::SetControllerAddresses: %08X %08X\r\n", m_pVisibleSurface, m_pVisibleOverlay));

    // update the graphics addresses
    if (m_pVisibleSurface)
    {
        OUTREG32(&m_pDISPCRegs->ulDISPC_GFX_BA0, m_pVisibleSurface->OffsetInVideoMemory());
        OUTREG32(&m_pDISPCRegs->ulDISPC_GFX_BA1, m_pVisibleSurface->OffsetInVideoMemory());
#if defined(TVOUT)
        OUTREG32(&m_pTVVidRegs->ulBA0, m_pVisibleSurface->OffsetInVideoMemory());
        OUTREG32(&m_pTVVidRegs->ulBA1, m_pVisibleSurface->OffsetInVideoMemory());
#endif
    }

    // update the overlay addresses
    if (m_pVisibleOverlay)
    {
        OUTREG32(&m_pVisibleOverlay->m_pVidRegs->ulBA0, m_pVisibleOverlay->OffsetInVideoMemory());
        OUTREG32(&m_pVisibleOverlay->m_pVidRegs->ulBA1, m_pVisibleOverlay->OffsetInVideoMemory());
    }

    // commit the display controller changes
    SETREG32(&m_pDISPCRegs->ulDISPC_CONTROL, DISPC_CONTROL_LCDENABLE | DISPC_CONTROL_GOLCD);
    while (INREG32(&m_pDISPCRegs->ulDISPC_CONTROL) & DISPC_CONTROL_GOLCD);

    // Next blit needs to synchronize on this operation!
    m_flipBusy = TRUE;

    // Set update flag
    m_updateFlag = TRUE;
}

//------------------------------------------------------------------------------
//
//   GetGameXInfo
//
//   fill out GAPI information for DrvEscape call
//
int OMAP2420GPE::GetGameXInfo(ULONG code, ULONG inSize, VOID *pIn, ULONG outSize, VOID *pOut)
{
    int rc = 0;
    GXDeviceInfo *pInfo;

    DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::GetGameXInfo"));

    // GAPI only support P8, RGB444, RGB555, RGB565, and RGB888
    pInfo = (GXDeviceInfo *)pOut;
    if (
        pOut != NULL && outSize >= sizeof(GXDeviceInfo) && (
            m_pMode->Bpp == 8 || m_pMode->Bpp == 12 ||
            m_pMode->Bpp == 16 || m_pMode->Bpp == 24 ||
            m_pMode->Bpp == 32
        ) && pInfo->idVersion == kidVersion100
    )
    {
        pInfo->pvFrameBuffer = (UCHAR*)m_pPrimarySurface->Buffer();
        pInfo->cbStride = m_pPrimarySurface->Stride();
        pInfo->cxWidth  = m_pPrimarySurface->Width();
        pInfo->cyHeight = m_pPrimarySurface->Height();
        switch (m_pMode->Bpp) {
        case 8:
            pInfo->cBPP = 8;
            pInfo->ffFormat |= kfPalette;
            break;
        case 12:
            pInfo->cBPP = 12;
            pInfo->ffFormat |= kfDirect | kfDirect444;
            break;
        case 16:
            pInfo->cBPP = 16;
            pInfo->ffFormat |= kfDirect | kfDirect565;
            break;
        case 24:
            pInfo->cBPP = 24;
            pInfo->ffFormat |= kfDirect | kfDirect888;
        case 32:
            pInfo->cBPP = 32;
            pInfo->ffFormat |= kfDirect | kfDirect888;
        default:
            goto cleanUp;
        }

        // todo: get keys from registry
        pInfo->vkButtonUpPortrait = VK_UP;
        pInfo->vkButtonUpLandscape = VK_LEFT;
        pInfo->ptButtonUp.x = 88;
        pInfo->ptButtonUp.y = 250;
        pInfo->vkButtonDownPortrait = VK_DOWN;
        pInfo->vkButtonDownLandscape = VK_RIGHT;
        pInfo->ptButtonDown.x = 88;
        pInfo->ptButtonDown.y = 270;
        pInfo->vkButtonLeftPortrait = VK_LEFT;
        pInfo->vkButtonLeftLandscape = VK_DOWN;
        pInfo->ptButtonLeft.x = 78;
        pInfo->ptButtonLeft.y = 260;
        pInfo->vkButtonRightPortrait = VK_RIGHT;
        pInfo->vkButtonRightLandscape = VK_UP;
        pInfo->ptButtonRight.x = 98;
        pInfo->ptButtonRight.y = 260;

#if 1       // default to Smartphone settings for now
        pInfo->vkButtonAPortrait = VK_F1;   // Softkey 1
        pInfo->vkButtonALandscape = VK_F1;
        pInfo->ptButtonA.x = 10;
        pInfo->ptButtonA.y = 240;
        pInfo->vkButtonBPortrait = VK_F2;   // Softkey 2
        pInfo->vkButtonBLandscape = VK_F2;
        pInfo->ptButtonB.x = 166;
        pInfo->ptButtonB.y = 240;
        pInfo->vkButtonCPortrait = VK_F8;   // Asterisk on keypad
        pInfo->vkButtonCLandscape = VK_F8;
        pInfo->ptButtonC.x = 10;
        pInfo->ptButtonC.y = 320;
#else
        //  Added For GAPI to map Literal A, B, C
        pInfo->vkButtonAPortrait = VK_APP_LAUNCH1;
        pInfo->vkButtonALandscape = VK_APP_LAUNCH1;
        pInfo->ptButtonA.x = 10;
        pInfo->ptButtonA.y = 240;
        pInfo->vkButtonBPortrait = VK_APP_LAUNCH4;
        pInfo->vkButtonBLandscape = VK_APP_LAUNCH4;
        pInfo->ptButtonB.x = 166;
        pInfo->ptButtonB.y = 240;
        pInfo->vkButtonCPortrait = VK_APP_LAUNCH2;   // Asterisk on keypad
        pInfo->vkButtonCLandscape = VK_APP_LAUNCH2;
        pInfo->ptButtonC.x = 10;
        pInfo->ptButtonC.y = 320;
#endif
        pInfo->vkButtonStartPortrait  = '\r';
        pInfo->vkButtonStartLandscape = '\r';
        pInfo->ptButtonStart.x = 88;
        pInfo->ptButtonStart.y = 260;
        pInfo->pvReserved1 = (void *) 0;
        pInfo->pvReserved2 = (void *) 0;

        rc = 1;
    }
    else {
        SetLastError (ERROR_INVALID_PARAMETER);
        rc = -1;
    }

cleanUp:
    DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::GetGameXInfo: %d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  OMAP2420GPE::DrvEscape
//
ULONG OMAP2420GPE::DrvEscape(SURFOBJ* pso, ULONG code, ULONG inSize, PVOID pIn, ULONG outSize, PVOID pOut)
{

    ULONG rc = 0;
    VIDEO_POWER_MANAGEMENT *pvpm;
//    DEBUGMSG(GPE_ZONE_PDD, (L"+OMAP2420GPE::DrvEscape: %08X\r\n", code));

    switch (code) {
    case QUERYESCSUPPORT:
        if (pIn == NULL || inSize < sizeof(DWORD)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = ESC_FAILED;
            goto cleanUp;
        }
        switch (*((DWORD*)pIn)) {
        case GETPOWERMANAGEMENT:
        case SETPOWERMANAGEMENT:
        case IOCTL_POWER_CAPABILITIES:
        case IOCTL_POWER_QUERY:
        case IOCTL_POWER_SET:
        case IOCTL_POWER_GET:
        case GETGXINFO:
            rc = ESC_SUCCESS;
            break;
        case DRVESC_GETSCREENROTATION:
        case DRVESC_SETSCREENROTATION:
        default:
            rc = ESC_NOTIMPLEMENTED;
        }
        break;

    case GETGXINFO:
        rc = GetGameXInfo(code, inSize, pIn, outSize, pOut);
        break;

    case SETPOWERMANAGEMENT:
        DEBUGMSG(GPE_ZONE_PM, (L"DDI_OMAP2420GPE::DrvEsc: "
            L"SETPOWERMANAGEMENT\r\n"
        ));
        if (pIn == NULL || inSize < sizeof(VIDEO_POWER_MANAGEMENT)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = ESC_FAILED;
            goto cleanUp;
        }
        __try {
            pvpm = (VIDEO_POWER_MANAGEMENT *)pIn;
            if (pvpm->Length < sizeof(VIDEO_POWER_MANAGEMENT)) {
                SetLastError(ERROR_INVALID_PARAMETER);
                rc = ESC_FAILED;
                goto cleanUp;
            }
            switch (pvpm->PowerState) {
            case VideoPowerOn:
                m_externalDX = D0;
                if (m_currentDX < D0 && SetPower(D0)) SetEvent(m_hPowerEvent);
                DEBUGMSG(GPE_ZONE_PM, (L"DDI_OMAP2420GPE::DrvEsc: "
                    L"SETPOWERMANAGEMENT: Display PowerOn\r\n"
                ));
                rc = ESC_SUCCESS;
                break;
            case VideoPowerStandBy:
                m_externalDX = D2;
                if (m_currentDX < D2 && SetPower(D2)) SetEvent(m_hPowerEvent);
                DEBUGMSG(GPE_ZONE_PM, (L"DDI_OMAP2420GPE::DrvEsc: "
                    L"SETPOWERMANAGEMENT: Display StandBy\r\n"
                ));
                rc = ESC_SUCCESS;
                break;
            case VideoPowerSuspend:
                m_externalDX = D3;
                if (m_currentDX < D3 && SetPower(D3)) SetEvent(m_hPowerEvent);
                DEBUGMSG(GPE_ZONE_PM, (L"DDI_OMAP2420GPE::DrvEsc: "
                    L"SETPOWERMANAGEMENT: Display StandBy\r\n"
                ));
                rc = ESC_SUCCESS;
                break;
            case VideoPowerOff:
                m_externalDX = D4;
                if (m_currentDX < D4 && SetPower(D4)) SetEvent(m_hPowerEvent);
                DEBUGMSG(GPE_ZONE_PM, (L"DDI_OMAP2420GPE::DrvEsc: "
                    L"SETPOWERMANAGEMENT: display off\r\n"
                ));
                rc = ESC_SUCCESS;
                break;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: DDI_OMAP2420GPE::DrvEsc: "
                L"Exception in DrvEscape/SETPOWERMANAGEMENT\r\n"
            ));
            rc = ESC_FAILED;
        }
        break;

    case GETPOWERMANAGEMENT:
        DEBUGMSG(GPE_ZONE_PM, (L"DDI_OMAP2420GPE::DrvEsc: "
            L"GETPOWERMANAGEMENT\r\n"
        ));
        if (pOut == NULL || outSize < sizeof(VIDEO_POWER_MANAGEMENT)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = ESC_FAILED;
            goto cleanUp;
        }
        __try {
            pvpm = (VIDEO_POWER_MANAGEMENT*)pOut;
            pvpm->Length = sizeof(VIDEO_POWER_MANAGEMENT);
            pvpm->DPMSVersion = 0;
            switch (m_externalDX) {
            case D0:
            case D1:
                pvpm->PowerState = VideoPowerOn;
                break;
            case D2:
                pvpm->PowerState = VideoPowerStandBy;
                break;
            case D3:
                pvpm->PowerState = VideoPowerSuspend;
                break;
            case D4:                
                pvpm->PowerState = VideoPowerOff;
                break;
            }
            rc = ESC_SUCCESS;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: DDI_OMAP2420GPE::DrvEsc: "
                L"Exception in DrvEscape/GETPOWERMANAGEMENT\r\n"
            ));
            rc = ESC_FAILED;
        }
        break;

    case IOCTL_POWER_CAPABILITIES:
        DEBUGMSG(GPE_ZONE_PM, (L"DDI_OMAP2420GPE::DrvEsc: "
            L"IOCTL_POWER_CAPABILITIES\r\n"
        ));
        if (pOut == NULL || outSize < sizeof(POWER_CAPABILITIES)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = ESC_FAILED;
            goto cleanUp;
        }
        __try {
            POWER_CAPABILITIES *pPowerCaps = (POWER_CAPABILITIES*)pOut;
            memset(pPowerCaps, 0, sizeof(POWER_CAPABILITIES));
            pPowerCaps->DeviceDx |= DX_MASK(D0) | DX_MASK(D2);
            pPowerCaps->DeviceDx |= DX_MASK(D3) | DX_MASK(D4);
            rc = ESC_SUCCESS;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: DDI_OMAP2420GPE::DrvEsc: "
                L"Exception in DrvEscape/IOCTL_POWER_CAPABILITIES\r\n"
            ));
            rc = ESC_FAILED;
        }
        break;

    case IOCTL_POWER_QUERY:
        DEBUGMSG(GPE_ZONE_PM, (L"DDI_OMAP2420GPE::DrvEsc: "
            L"IOCTL_POWER_QUERY\r\n"
        ));
        if (pOut == NULL || outSize < sizeof(CEDEVICE_POWER_STATE)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = ESC_FAILED;
            goto cleanUp;
        }
        __try {
            CEDEVICE_POWER_STATE dx = *(CEDEVICE_POWER_STATE*)pOut;
            rc = VALID_DX(dx) ? ESC_SUCCESS : ESC_FAILED;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: DDI_OMAP2420GPE::DrvEsc: "
                L"Exception in DrvEscape/IOCTL_POWER_QUERY\r\n"
            ));
            rc = ESC_FAILED;
        }
        break;

    case IOCTL_POWER_SET:
        DEBUGMSG(GPE_ZONE_PM, (L"DDI_OMAP2420GPE::DrvEsc: "
            L"IOCTL_POWER_SET %d\r\n", *(CEDEVICE_POWER_STATE*)pOut
        ));
        if (pOut == NULL || outSize < sizeof(CEDEVICE_POWER_STATE)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = ESC_FAILED;
            goto cleanUp;
        }
        __try {
            CEDEVICE_POWER_STATE dx = *(CEDEVICE_POWER_STATE*)pOut;
            m_externalDX = dx;
            SetPower(dx);
            *(CEDEVICE_POWER_STATE*)pOut = m_externalDX;
            rc = ESC_SUCCESS;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: DDI_OMAP2420GPE::DrvEsc: "
                L"Exception in DrvEscape/IOCTL_POWER_SET\r\n"
            ));
            rc = ESC_FAILED;
        }
        break;

    case IOCTL_POWER_GET:
        DEBUGMSG(GPE_ZONE_PM, (L"DDI_OMAP2420GPE::DrvEsc: "
            L"IOCTL_POWER_GET\r\n"
        ));
        if (pOut == NULL || outSize < sizeof(CEDEVICE_POWER_STATE)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = ESC_FAILED;
            goto cleanUp;
        }
        __try {
            *(CEDEVICE_POWER_STATE*)pOut = m_externalDX;
            rc = ESC_SUCCESS;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: DDI_OMAP2420GPE::DrvEsc: "
                L"Exception in DrvEscape/IOCTL_POWER_GET\r\n"
            ));
            rc = ESC_FAILED;
        }
        break;

#if defined(TVOUT)
        // get the TV-Out mode
    case DRVESC_GET_TVOUT:
        DEBUGMSG(GPE_ZONE_PDD, (L"OMAP2420GPE::DrvEscape DRVESC_GET_TVOUT %d\r\n", m_dwTVOut));
        if (pOut == NULL || outSize < sizeof(DWORD))
        {
            DEBUGMSG(GPE_ZONE_ERROR, (L"OMAP2420GPE::DrvEscape GET_TVOUT FAILED: invalid parameter!!!\r\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = ESC_FAILED;
        }
        else
        {
            *(PDWORD)pOut = m_dwTVOut;
            rc = ESC_SUCCESS;
        }
        break;

    // set the TV-Out mode
    case DRVESC_SET_TVOUT:
        DEBUGMSG(GPE_ZONE_PDD, (L"OMAP2420GPE::DrvEscape DRVESC_SET_TVOUT: %d\r\n", *(PDWORD)pIn));
        if (pIn == NULL || inSize < sizeof(DWORD))
        {
            DEBUGMSG(GPE_ZONE_ERROR, (L"OMAP2420GPE::DrvEscape SET_TVOUT FAILED: invalid parameter!!!\r\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = ESC_FAILED;
        }
        else
        {
            DWORD dwTVOut = *(PDWORD)pIn;
            if (dwTVOut != TVOUT_NONE && dwTVOut != TVOUT_NTSC && dwTVOut != TVOUT_PAL)
            {
                DEBUGMSG(GPE_ZONE_ERROR, (L"OMAP2420GPE::DrvEscape SET_TVOUT FAILED: invalid mode!!!\r\n"));
                SetLastError(ERROR_INVALID_PARAMETER);
                rc = ESC_FAILED;
            }
            else
            {
                SetTVOut(dwTVOut);
                rc = ESC_SUCCESS;
            }
        }
    break;
#endif // defined(TVOUT)

    default:
        rc = GPE::DrvEscape(pso, code, inSize, pIn, outSize, pOut);
    }

cleanUp:
//    DEBUGMSG(GPE_ZONE_PDD, (L"-OMAP2420GPE::DrvEscape: %08X\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Static Method:  PowerThreadStub
//
DWORD OMAP2420GPE::PowerThreadStub(VOID *pContext)
{
    OMAP2420GPE *pOMAP2420GPE = (OMAP2420GPE *)pContext;
    return pOMAP2420GPE->PowerThread();
}

//------------------------------------------------------------------------------
//
//  Method:  PowerThread
//
//  This thread wait for power event handler timeout and set device to lower
//  power state. It should run on lower priority than other GWE threads to
//  avoid unnecessary context switches. This is also reason why draw function
//  call SetPower(D0) instead using this thread to do so (when m_hPowerEvent
//  is set).
//
DWORD OMAP2420GPE::PowerThread()
{
    CEDEVICE_POWER_STATE dx = D0;
    DWORD delay = m_powerDelay;
    BOOL updateFlag;
    
    DEBUGMSG(GPE_ZONE_PDD, (L"OMAP2420GPE::PowerThread++\r\n"));
    while (!m_powerThreadExit) {

        // Wait for event or timeout...
        WaitForSingleObject(m_hPowerEvent, delay);
        DEBUGMSG(GPE_ZONE_PDD, (L"OMAP2420GPE::PwrThread object set++, dx = %d\r\n",dx));
        // Get update flag
        updateFlag = (BOOL)InterlockedExchange(&m_updateFlag, FALSE);

        // If there was update, simply wait next period
        if (updateFlag != 0) {
            dx = D0;
            delay = m_powerDelay;
            continue;
        }

        // Depending on previous power thread state device
        // will be moved to D2 or D3 (there should be delay
        // to give LCD controller chance to finish frame).
        switch (dx) {
        case D0:            
            // We want to move display driver to D2.
            // This can be overpower by external power
            // state to higher state (e.g. D4)
            SetPower(D2);
            dx = D2;
            delay = m_powerDelay;
            break;
        case D2:
            SetPower(D3);
            dx = D3;
            delay = INFINITE;
            break;
        }            
    }    

    // All is great...
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//  Method:  ReadI2C
//
BOOL OMAP2420GPE::ReadI2C(DWORD dwAddress, DWORD dwAddressSize, UCHAR ucReg, UCHAR *pucData)
{
    DWORD                   dwNumBytes;
    I2C_SET_SLAVE_ADDRESS   sI2CAddress;
    I2CTRANS                sI2C_Transaction;

    // set the I2C slave address
    sI2CAddress.address = dwAddress;
    sI2CAddress.size = dwAddressSize;
    if (!DeviceIoControl(m_hI2C, IOCTL_I2C_SET_SLAVE_ADDRESS, (PVOID)&sI2CAddress, sizeof(sI2CAddress), NULL, 0, NULL, NULL))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE:ReadI2C: Failed (%08X) to set I2C slave address\r\n", GetLastError()));
        return FALSE;
    }

    ZeroMemory(&sI2C_Transaction,sizeof(sI2C_Transaction));
    sI2C_Transaction.mClk_HL_Divisor = I2C_CLOCK_DEFAULT;
    sI2C_Transaction.mOpCode[0] = I2C_OPCODE_WRITE;
    sI2C_Transaction.mBufferOffset[0] = 0;
    sI2C_Transaction.mBuffer[0] = ucReg;
    sI2C_Transaction.mTransLen[0] = 1;
    sI2C_Transaction.mOpCode[1] = I2C_OPCODE_READ;
    sI2C_Transaction.mBufferOffset[1] = 1;
    sI2C_Transaction.mTransLen[1] = 1;
    
    //Issue transaction
    if (!DeviceIoControl(m_hI2C, IOCTL_I2C_TRANSACT, NULL, 0, &sI2C_Transaction, sizeof(sI2C_Transaction), &dwNumBytes, NULL))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE:IOCTL_I2C_TRANSACT: Failed "));
        return FALSE;
    }

    if (sI2C_Transaction.mErrorCode)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"CMenelaus::ReadData failed with error code 0x%08X\r\n", sI2C_Transaction.mErrorCode));
        return FALSE;
    }

    *pucData = sI2C_Transaction.mBuffer[1];
    return TRUE;
}

//------------------------------------------------------------------------------
//  Method:  WriteI2C
//
BOOL OMAP2420GPE::WriteI2C(DWORD dwAddress, DWORD dwAddressSize, UCHAR ucReg, UCHAR ucData)
{
    DWORD                   dwNumBytes;
    I2C_SET_SLAVE_ADDRESS   sI2CAddress;
    I2CTRANS                sI2C_Transaction;

    // set the I2C slave address
    sI2CAddress.address = dwAddress;
    sI2CAddress.size = dwAddressSize;
    if (!DeviceIoControl(m_hI2C, IOCTL_I2C_SET_SLAVE_ADDRESS, (PVOID)&sI2CAddress, sizeof(sI2CAddress), NULL, 0, NULL, NULL))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE:ReadI2C: "
            L"Failed (%08X) to set I2C slave address\r\n", GetLastError()));
        return FALSE;
    }

    ZeroMemory(&sI2C_Transaction,sizeof(sI2C_Transaction));
    sI2C_Transaction.mClk_HL_Divisor = I2C_CLOCK_DEFAULT;
    sI2C_Transaction.mOpCode[0] = I2C_OPCODE_WRITE;
    sI2C_Transaction.mBufferOffset[0] = 0;
    sI2C_Transaction.mBuffer[0] = ucReg;
    sI2C_Transaction.mBuffer[1] = ucData;
    sI2C_Transaction.mTransLen[0] = 2;

    //Issue transaction
    if (!DeviceIoControl(m_hI2C, IOCTL_I2C_TRANSACT, NULL, 0, &sI2C_Transaction, sizeof(sI2C_Transaction), &dwNumBytes, NULL))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE:IOCTL_I2C_TRANSACT: Failed "));
        return FALSE;
    }

    if (sI2C_Transaction.mErrorCode)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"CMenelaus::ReadData failed with error code 0x%08X\r\n", sI2C_Transaction.mErrorCode));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//  Method:  ReadI2CIO - Used for direct reading IO Expanders
//
BOOL OMAP2420GPE::ReadI2CIO(DWORD dwAddress, DWORD dwAddressSize, UCHAR *pucData)
{
    DWORD                   dwNumBytes;
    I2C_SET_SLAVE_ADDRESS   sI2CAddress;
    I2CTRANS                sI2C_Transaction;

    // set the I2C slave address
    sI2CAddress.address = dwAddress;
    sI2CAddress.size = dwAddressSize;
    if (!DeviceIoControl(m_hI2C, IOCTL_I2C_SET_SLAVE_ADDRESS, (PVOID)&sI2CAddress, sizeof(sI2CAddress), NULL, 0, NULL, NULL))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE:ReadI2C: "
            L"Failed (%08X) to set I2C slave address\r\n", GetLastError()));
        return FALSE;
    }

    ZeroMemory(&sI2C_Transaction,sizeof(sI2C_Transaction));
    sI2C_Transaction.mClk_HL_Divisor  = I2C_CLOCK_DEFAULT;
    sI2C_Transaction.mOpCode[0]       = I2C_OPCODE_READ;
    sI2C_Transaction.mTransLen[0]     = 1;
    sI2C_Transaction.mBufferOffset[0] = 0;

    //Issue transaction
    if (!DeviceIoControl(m_hI2C, IOCTL_I2C_TRANSACT, NULL, 0, &sI2C_Transaction, sizeof(sI2C_Transaction), &dwNumBytes, NULL))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE:IOCTL_I2C_TRANSACT: Failed "));
        return FALSE;
    }

    if (sI2C_Transaction.mErrorCode)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"CMenelaus::ReadData failed with error code 0x%08X\r\n", sI2C_Transaction.mErrorCode));
        return FALSE;
    }

    *pucData = sI2C_Transaction.mBuffer[0];
    return TRUE;
}

//------------------------------------------------------------------------------
//  Method:  WriteI2CIO - Used for direct writing to IO Expanders
//
BOOL OMAP2420GPE::WriteI2CIO(DWORD dwAddress, DWORD dwAddressSize, UCHAR ucData)
{
    DWORD                   dwNumBytes;
    I2C_SET_SLAVE_ADDRESS   sI2CAddress;
    I2CTRANS                sI2C_Transaction;

    // set the I2C slave address
    sI2CAddress.address = dwAddress;
    sI2CAddress.size = dwAddressSize;
    if (!DeviceIoControl(m_hI2C, IOCTL_I2C_SET_SLAVE_ADDRESS, (PVOID)&sI2CAddress, sizeof(sI2CAddress), NULL, 0, NULL, NULL))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE:ReadI2C: "
            L"Failed (%08X) to set I2C slave address\r\n", GetLastError()));
        return FALSE;
    }

    ZeroMemory(&sI2C_Transaction,sizeof(sI2C_Transaction));
    sI2C_Transaction.mOpCode[0]       = I2C_OPCODE_WRITE;
    sI2C_Transaction.mTransLen[0]     = 1;
    sI2C_Transaction.mBufferOffset[0] = 0;
    sI2C_Transaction.mBuffer[0] = ucData;

    //Issue transaction
    if (!DeviceIoControl(m_hI2C, IOCTL_I2C_TRANSACT, NULL, 0, &sI2C_Transaction, sizeof(sI2C_Transaction), &dwNumBytes, NULL))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420GPE:IOCTL_I2C_TRANSACT: Failed "));
        return FALSE;
    }

    if (sI2C_Transaction.mErrorCode)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"CMenelaus::ReadData failed with error code 0x%08X\r\n", sI2C_Transaction.mErrorCode));
        return FALSE;
    }

    return TRUE;
}

#if defined(TVOUT)

//------------------------------------------------------------------------------
//  Method:  EnableVREF
//
BOOL OMAP2420GPE::EnableVREF(BOOL fEnable)
{
    BOOL    fRet;
    UCHAR   ucData;

    // read the current value
    fRet = ReadI2C(I2C_MENELAUS_ADDRESS, I2C_MENELAUS_ADDRSIZE, MENELAUS_LD0CTRL8_OFFSET, &ucData);

    if (fRet)
    {
        // set or clear the REF05 bit
        if (fEnable)
        {
            ucData |= REF05_EN | VADAC_EN;
        }
        else
        {
            ucData &= ~(REF05_EN | VADAC_EN);
        }

        // write the new value
        fRet = WriteI2C(I2C_MENELAUS_ADDRESS, I2C_MENELAUS_ADDRSIZE, MENELAUS_LD0CTRL8_OFFSET, ucData);
    }
    Sleep(100);

    // return the result
    return fRet;
}

//------------------------------------------------------------------------------
//
//  Method:  SetTVOut
//
//  Sets the TV-Out mode
//
void OMAP2420GPE::SetTVOut(DWORD dwTVOut)
{
//    DWORD           i, j;
    const VENC_INIT *pInit;

    m_dwTVOut = dwTVOut;
    if (m_dwTVOut == TVOUT_NONE)
    {
        // disable digital output
        CLRREG32(&m_pDSSRegs->ulDSS_CONTROL, DSS_CONTROL_DAC_DEMEN | DSS_CONTROL_VENC_CLOCK_4X_ENABLE);
        CLRREG32(&m_pDISPCRegs->ulDISPC_CONTROL, DISPC_CONTROL_DIGITALENABLE);

        // disable the video pipeline
        CLRREG32(&m_pTVVidRegs->ulATTRIBUTES, DISPC_VID_ATTR_VIDENABLE);

        // disable DAC output
        CLRREG32(&m_pVENCRegs->ulVENC_DAC_TST, DISPC_DAC_TST_DACX);

        // wait for the initialization to finish
        SETREG32(&m_pDISPCRegs->ulDISPC_CONTROL, DISPC_CONTROL_GOLCD);
        while (INREG32(&m_pDISPCRegs->ulDISPC_CONTROL) & DISPC_CONTROL_GOLCD) ;

        // turn off the DAC's reference voltage
        EnableVREF(FALSE);

#if defined(DEBUG)
        DumpRegs(L"SetTVOut NONE");
#endif
    }
    else
    {
        // turn on the DAC's reference voltage
        EnableVREF(TRUE);

        // set the DSS registers
        SETREG32(&m_pDSSRegs->ulDSS_CONTROL, DSS_CONTROL_DAC_DEMEN | DSS_CONTROL_VENC_CLOCK_4X_ENABLE);

        // set the VENC registers
        pInit = g_Venc_Init + m_dwTVOut - TVOUT_NTSC;
        OUTREG32(&m_pVENCRegs->ulVENC_F_CONTROL, pInit->ulVENC_F_CONTROL);
        OUTREG32(&m_pVENCRegs->ulVENC_SYNC_CTRL, pInit->ulVENC_SYNC_CTRL);
        OUTREG32(&m_pVENCRegs->ulVENC_LLEN, pInit->ulVENC_LLEN);
        OUTREG32(&m_pVENCRegs->ulVENC_FLENS, pInit->ulVENC_FLENS);
        OUTREG32(&m_pVENCRegs->ulVENC_HFLTR_CTRL, pInit->ulVENC_HFLTR_CTRL);
        OUTREG32(&m_pVENCRegs->ulVENC_GAIN_U, pInit->ulVENC_GAIN_U);
        OUTREG32(&m_pVENCRegs->ulVENC_GAIN_V, pInit->ulVENC_GAIN_V);
        OUTREG32(&m_pVENCRegs->ulVENC_GAIN_Y, pInit->ulVENC_GAIN_Y);
        OUTREG32(&m_pVENCRegs->ulVENC_BLACK_LEVEL, pInit->ulVENC_BLACK_LEVEL);
        OUTREG32(&m_pVENCRegs->ulVENC_BLANK_LEVEL, pInit->ulVENC_BLANK_LEVEL);
        OUTREG32(&m_pVENCRegs->ulVENC_X_COLOR, pInit->ulVENC_X_COLOR);
        OUTREG32(&m_pVENCRegs->ulVENC_M_CONTROL, pInit->ulVENC_M_CONTROL);
        OUTREG32(&m_pVENCRegs->ulVENC_BSTAMP_WSS_DATA, pInit->ulVENC_BSTAMP_WSS_DATA);
        OUTREG32(&m_pVENCRegs->ulVENC_S_CARR, pInit->ulVENC_S_CARR);
        OUTREG32(&m_pVENCRegs->ulVENC_SAVID__EAVID, pInit->ulVENC_SAVID__EAVID);
        OUTREG32(&m_pVENCRegs->ulVENC_FLEN__FAL, pInit->ulVENC_FLEN__FAL);
        OUTREG32(&m_pVENCRegs->ulVENC_LAL__PHASE_RESET, pInit->ulVENC_LAL__PHASE_RESET);
        OUTREG32(&m_pVENCRegs->ulVENC_HS_INT_START_STOP_X, pInit->ulVENC_HS_INT_START_STOP_X);
        OUTREG32(&m_pVENCRegs->ulVENC_HS_EXT_START_STOP_X, pInit->ulVENC_HS_EXT_START_STOP_X);
        OUTREG32(&m_pVENCRegs->ulVENC_VS_INT_START_X, pInit->ulVENC_VS_INT_START_X);
        OUTREG32(&m_pVENCRegs->ulVENC_VS_INT_STOP_X__VS_INT_START_Y, pInit->ulVENC_VS_INT_STOP_X__VS_INT_START_Y);
        OUTREG32(&m_pVENCRegs->ulVENC_VS_INT_STOP_Y__VS_EXT_START_X, pInit->ulVENC_VS_INT_STOP_Y__VS_EXT_START_X);
        OUTREG32(&m_pVENCRegs->ulVENC_VS_EXT_STOP_X__VS_EXT_START_Y, pInit->ulVENC_VS_EXT_STOP_X__VS_EXT_START_Y);
        OUTREG32(&m_pVENCRegs->ulVENC_VS_EXT_STOP_Y, pInit->ulVENC_VS_EXT_STOP_Y);
        OUTREG32(&m_pVENCRegs->ulVENC_AVID_START_STOP_X, pInit->ulVENC_AVID_START_STOP_X);
        OUTREG32(&m_pVENCRegs->ulVENC_AVID_START_STOP_Y, pInit->ulVENC_AVID_START_STOP_Y);
        OUTREG32(&m_pVENCRegs->ulVENC_FID_INT_START_X__FID_INT_START_Y, pInit->ulVENC_FID_INT_START_X__FID_INT_START_Y);
        OUTREG32(&m_pVENCRegs->ulVENC_FID_INT_OFFSET_Y__FID_EXT_START_X, pInit->ulVENC_FID_INT_OFFSET_Y__FID_EXT_START_X);
        OUTREG32(&m_pVENCRegs->ulVENC_FID_EXT_START_Y__FID_EXT_OFFSET_Y, pInit->ulVENC_FID_EXT_START_Y__FID_EXT_OFFSET_Y);
        OUTREG32(&m_pVENCRegs->ulVENC_TVDETGP_INT_START_STOP_X, pInit->ulVENC_TVDETGP_INT_START_STOP_X);
        OUTREG32(&m_pVENCRegs->ulVENC_TVDETGP_INT_START_STOP_Y, pInit->ulVENC_TVDETGP_INT_START_STOP_Y);
        OUTREG32(&m_pVENCRegs->ulVENC_GEN_CTRL, pInit->ulVENC_GEN_CTRL);

        // enable the video pipeline
        SETREG32(&m_pTVVidRegs->ulATTRIBUTES, DISPC_VID_ATTR_VIDENABLE);

        // configure and enable DAC output
        SETREG32(&m_pVENCRegs->ulVENC_DAC_TST, DISPC_DAC_TST_DACX);
        CLRREG32(&m_pVENCRegs->ulVENC_DAC_TST, DISPC_DAC_TST_DAC_INVT);

        // enable digital output and wait for the initialization to finish
        SETREG32(&m_pDISPCRegs->ulDISPC_CONTROL, DISPC_CONTROL_DIGITALENABLE | DISPC_CONTROL_GODIGITAL);
        while (INREG32(&m_pDISPCRegs->ulDISPC_CONTROL) & DISPC_CONTROL_GODIGITAL) ;

#if defined(DEBUG)
        DumpRegs((m_dwTVOut == TVOUT_NTSC) ? L"SetTVOut NTSC" : L"SetTVOut PAL");
#endif
    }
}
#endif  // defined(TVOUT)

#if defined(DEBUG)
//------------------------------------------------------------------------------
//
//  Method:  DumpVidRegs
//
//  Dumps a set of video register values out the debug port.
//
void OMAP2420GPE::DumpVidRegs(DWORD dwIndex)
{
    DWORD               i;
    OMAP2420_VID_REGS   *pVidRegs;

    pVidRegs = (dwIndex == 1) ? &m_pDISPCRegs->tDISPC_VID1 : &m_pDISPCRegs->tDISPC_VID2;
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_BA0:           %08X\r\n", dwIndex, INREG32(&pVidRegs->ulBA0)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_BA1:           %08X\r\n", dwIndex, INREG32(&pVidRegs->ulBA1)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_POSITION:      %08X\r\n", dwIndex, INREG32(&pVidRegs->ulPOSITION)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_SIZE:          %08X\r\n", dwIndex, INREG32(&pVidRegs->ulSIZE)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_ATTR:          %08X\r\n", dwIndex, INREG32(&pVidRegs->ulATTRIBUTES)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_FIFO_THR:      %08X\r\n", dwIndex, INREG32(&pVidRegs->ulFIFO_THRESHOLD)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_FIFO_SZST:     %08X\r\n", dwIndex, INREG32(&pVidRegs->ulFIFO_SIZE_STATUS)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_ROW_INC:       %08X\r\n", dwIndex, INREG32(&pVidRegs->ulROW_INC)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_PIXEL_INC:     %08X\r\n", dwIndex, INREG32(&pVidRegs->ulPIXEL_INC)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_FIR:           %08X\r\n", dwIndex, INREG32(&pVidRegs->ulFIR)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_PIC_SIZE:      %08X\r\n", dwIndex, INREG32(&pVidRegs->ulPICTURE_SIZE)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_ACCU0:         %08X\r\n", dwIndex, INREG32(&pVidRegs->ulACCU0)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_ACCU1:         %08X\r\n", dwIndex, INREG32(&pVidRegs->ulACCU1)));
    for (i = 0; i < 8; i++)
    {
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_FIR_COEF%u:     %08X    %08X\r\n", dwIndex,
    INREG32(&pVidRegs->aFIR_COEF[0].ulH), INREG32(&pVidRegs->aFIR_COEF[0].ulHV)));
    }
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_CONV_COEF0:    %08X\r\n", dwIndex, INREG32(&pVidRegs->ulCONV_COEF0)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_CONV_COEF1:    %08X\r\n", dwIndex, INREG32(&pVidRegs->ulCONV_COEF1)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_CONV_COEF2:    %08X\r\n", dwIndex, INREG32(&pVidRegs->ulCONV_COEF2)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_CONV_COEF3:    %08X\r\n", dwIndex, INREG32(&pVidRegs->ulCONV_COEF3)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_VID%d_CONV_COEF4:    %08X\r\n", dwIndex, INREG32(&pVidRegs->ulCONV_COEF4)));
}

//------------------------------------------------------------------------------
//
//  Method:  DumpRegs
//
//  Dumps the register values out the debug port.
//
void OMAP2420GPE::DumpRegs(LPTSTR szMsg)
{
#if defined(TVOUT)
    UCHAR   ucData;
#endif

    DEBUGMSG(GPE_ZONE_PDD, (L"%s Display Registers:\r\n", szMsg));

    DEBUGMSG(GPE_ZONE_PDD, (L"   DSS_REVISION:             %08X\r\n", INREG32(&m_pDSSRegs->ulDSS_REVISIONNUMBER)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DSS_SYSCONFIG:            %08X\r\n", INREG32(&m_pDSSRegs->ulDSS_SYSCONFIG)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DSS_SYSSTATUS:            %08X\r\n", INREG32(&m_pDSSRegs->ulDSS_SYSSTATUS)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DSS_CONTROL:              %08X\r\n", INREG32(&m_pDSSRegs->ulDSS_CONTROL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DSS_STATUS:               %08X\r\n", INREG32(&m_pDSSRegs->ulDSS_STATUS)));

    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_REVISION:           %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_REVISION)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_SYSCONFIG:          %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_SYSCONFIG)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_SYSSTATUS:          %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_SYSSTATUS)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_IRQSTATUS:          %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_IRQSTATUS)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_IRQENABLE:          %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_IRQENABLE)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_CONTROL:            %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_CONTROL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_CONFIG:             %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_CONFIG)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_CAPABLE:            %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_CAPABLE)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_DEF_COLOR0:         %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_DEFAULT_COLOR0)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_DEF_COLOR1:         %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_DEFAULT_COLOR1)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_TRANS_COLOR0:       %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_TRANS_COLOR0)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_TRANS_COLOR1:       %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_TRANS_COLOR1)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_LINE_STATUS:        %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_LINE_STATUS)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_LINE_NUMBER:        %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_LINE_NUMBER)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_TIMING_H:           %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_TIMING_H)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_TIMING_V:           %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_TIMING_V)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_POL_FREQ:           %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_POL_FREQ)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_DIVISOR:            %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_DIVISOR)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_SIZE_DIG:           %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_SIZE_DIG)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_SIZE_LCD:           %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_SIZE_LCD)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_GFX_BA0:            %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_GFX_BA0)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_GFX_BA1:            %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_GFX_BA1)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_GFX_POSITION:       %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_GFX_POSITION)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_GFX_SIZE:           %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_GFX_SIZE)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_GFX_ATTR:           %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_GFX_ATTRIBUTES)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_GFX_FIFO_THR:       %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_GFX_FIFO_THRESHOLD)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_GFX_FIFO_SZST:      %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_GFX_FIFO_SIZE_STATUS)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_GFX_ROW_INC:        %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_GFX_ROW_INC)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_GFX_PIXEL_INC:      %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_GFX_PIXEL_INC)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_GFX_WINDOW_SKIP:    %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_GFX_WINDOW_SKIP)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_GFX_TABLE_BA:       %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_GFX_TABLE_BA)));

    DumpVidRegs(1);
    DumpVidRegs(2);

#if defined(TVOUT)
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_DATA_CYCLE1:        %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_DATA_CYCLE1)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_DATA_CYCLE2:        %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_DATA_CYCLE2)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   DISPC_DATA_CYCLE3:        %08X\r\n", INREG32(&m_pDISPCRegs->ulDISPC_DATA_CYCLE3)));

    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_REV_ID:              %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_REV_ID)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_STATUS:              %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_STATUS)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_F_CONTROL:           %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_F_CONTROL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_VIDOUT_CTRL:         %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_VIDOUT_CTRL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_SYNC_CTRL:           %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_SYNC_CTRL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_LLEN:                %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_LLEN)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_FLENS:               %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_FLENS)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_HFLTR_CTRL:          %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_HFLTR_CTRL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_CC_CARR_WSS_CARR:    %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_CC_CARR_WSS_CARR)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_C_PHASE:             %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_C_PHASE)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_GAIN_U:              %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_GAIN_U)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_GAIN_V:              %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_GAIN_V)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_GAIN_Y:              %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_GAIN_Y)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_BLACK_LEVEL:         %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_BLACK_LEVEL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_BLANK_LEVEL:         %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_BLANK_LEVEL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_X_COLOR:             %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_X_COLOR)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_M_CONTROL:           %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_M_CONTROL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_BSTAMP_WSS_DATA:     %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_BSTAMP_WSS_DATA)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_S_CARR:              %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_S_CARR)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_LINE21:              %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_LINE21)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_LN_SEL:              %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_LN_SEL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_L21__WC_CTL:         %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_L21__WC_CTL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_HTRIG_VTRIG:         %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_HTRIGGER_VTRIGGER)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_SAVID__EAVID:        %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_SAVID__EAVID)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_FLEN__FAL:           %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_FLEN__FAL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_LAL__PHASE_RESET:    %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_LAL__PHASE_RESET)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_HS_INT_STRT_STP_X:   %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_HS_INT_START_STOP_X)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_HS_EXT_STRT_STP_X:   %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_HS_EXT_START_STOP_X)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_VS_INT_START_X:      %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_VS_INT_START_X)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_VS_ISTP_X_ISTRT_Y:   %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_VS_INT_STOP_X__VS_INT_START_Y)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_VS_ISTP_Y_ESTRT_X:   %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_VS_INT_STOP_Y__VS_EXT_START_X)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_VS_ESTP_X_ESTRT_Y:   %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_VS_EXT_STOP_X__VS_EXT_START_Y)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_VS_EXT_STOP_Y:       %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_VS_EXT_STOP_Y)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_AVID_START_STOP_X:   %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_AVID_START_STOP_X)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_AVID_START_STOP_Y:   %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_AVID_START_STOP_Y)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_FID_ISTRT_X_ISTRT_Y: %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_FID_INT_START_X__FID_INT_START_Y)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_FID_IOFF_Y_ESTRT_X:  %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_FID_INT_OFFSET_Y__FID_EXT_START_X)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_FID_ESTRT_Y_EOFF_Y:  %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_FID_EXT_START_Y__FID_EXT_OFFSET_Y)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_TVDETGP_ISS_X:       %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_TVDETGP_INT_START_STOP_X)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_TVDETGP_ISS_Y:       %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_TVDETGP_INT_START_STOP_Y)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_GEN_CTRL:            %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_GEN_CTRL)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_DAC_TST:             %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_DAC_TST)));
    DEBUGMSG(GPE_ZONE_PDD, (L"   VENC_DAC:                 %08X\r\n", INREG32(&m_pVENCRegs->ulVENC_DAC)));
    ReadI2C(I2C_MENELAUS_ADDRESS, I2C_MENELAUS_ADDRSIZE, MENELAUS_LD0CTRL8_OFFSET, &ucData);
    DEBUGMSG(GPE_ZONE_PDD, (L"   LD0CTRL8:                 %02X\r\n", ucData));
#endif // defined(TVOUT)
}
#endif // defined(DEBUG)
//------------------------------------------------------------------------------
