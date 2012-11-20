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
//  File:  ddlcdif.cpp
//
//  Implementation of class DDLcdif.
//
//------------------------------------------------------------------------------

//#include <gxinfo.h>

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 6001 6385)
#include <stdio.h>

#include "precomp.h"
#include "dispperf.h"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 4100 4127 6001 6385)
#include <svsutil.hxx>
#pragma warning(pop)

#include "ddlcdif_escape.h"

//------------------------------------------------------------------------------
// Defines
#define DISPLAY_PHYS_MEM_ADDR IMAGE_WINCE_DISPLAY_RAM_PA_START
#define DISPLAY_PHYS_MEM_SZ   IMAGE_WINCE_DISPLAY_RAM_SIZE

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------
extern BOOL BSPGetCacheMode();
extern DWORD BSPGetWidth(DWORD modeNumber);
extern DWORD BSPGetHeight(DWORD modeNumber);
extern DWORD BSPGetVideoMemorySize();
extern ULONG BSPGetModeNum();
extern void BSPSetDisplayController();
extern void BSPDisplayPowerHandler(BOOL bOn);
extern BOOL BSPIsValidMode(DWORD modeNumber);
extern BOOL BSPSetMode(DWORD modeNumber);
extern void BSPGetModeInfoEx(GPEModeEx* pModeEx, int modeNumber);
extern void BSPSetDisplayBuffer(PVOID PhysBase, PVOID VirtBase);
extern UINT32 BSPGetOverlayAlign();
extern void BSPBacklightEnable(BOOL Enable);
//LQK SEP-19-2012
extern DWORD WINAPI ShutdownSplash(LPVOID lpParameter);

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------
extern CRITICAL_SECTION AllocCS;

BOOL bSuspended = FALSE;

//[RAKESH] Adding power management support
POWER_CAPABILITIES DisplayDrvPowerCaps =
{
    // DeviceDx:    Supported power states
    DX_MASK(D0) | DX_MASK(D2) | DX_MASK(D3) | DX_MASK(D4),

    // WakeFromDx
    0,

    // InrushDx:    No inrush of power
    0,

    // Power: Maximum milliwatts in each state
    {
        0x00000000, // D0
        0x00000000, // D1
        0x00000000, // D2
        0x00000000, // D3
        0x00000000  // D4
    },

    // Latency
    {
        0x00000000, // D0
        0x00000000, // D1
        0x00000000, // D2
        0x00000000, // D3
        0x00000000  // D4
    },

    // Flags: None
    0
};

HANDLE LCDIFInterruptEvent;
HANDLE m_hCombSurfNeedUdEvent;  //Handle for indicating Combine Surface needs to be updated
HANDLE m_hCombSurfUpdatedEvent;  //Handle for indicating Combine Surface has been updated
DWORD LCDIFSysintr;
HANDLE CombineSurface_thread = NULL;
bool CombineSurface_thread_running = false;

static ULONG RGBBitMask[][3] = {
    { 0xF800, 0x07E0, 0x001F },
    { 0xFF0000, 0x00FF00, 0x0000FF },      //PXP doesn't support packed 24bit RGB888 format as input        
    { 0x00FF0000, 0x0000FF00, 0x000000FF }
};

static HANDLE lcd_thread;
static bool lcd_thread_running = false;

DDLcdif* DDLcdif::SingletonDDGPE = NULL;
const UINT16 DDLcdif::colors[10] = {
    RGB565_COLOR_RED,
    RGB565_COLOR_WHITE,
    RGB565_COLOR_PURPLE,
    RGB565_COLOR_GREEN,
    RGB565_COLOR_BLUE,
    RGB565_COLOR_RED,
    RGB565_COLOR_GREEN,
    RGB565_COLOR_BLACK,
    RGB565_COLOR_PURPLE,
    RGB565_COLOR_BLUE,
};

// Start with errors and warnings
INSTANTIATE_GPE_ZONES(0x3,"LCDIF Driver","unused1","unused2")

// This prototype avoids problems exporting from .lib
BOOL APIENTRY GPEEnableDriver(ULONG engineVersion,
                              ULONG cj,
                              DRVENABLEDATA* data,
                              PENGCALLBACKS engineCallbacks);

BOOL APIENTRY DrvEnableDriver(ULONG engineVersion,
                              ULONG cj,
                              DRVENABLEDATA* data,
                              PENGCALLBACKS engineCallbacks)
{
    DEBUGMSG (1, (L"Display Driver Enable\r\n"));
    return GPEEnableDriver(engineVersion, cj, data, engineCallbacks);
} //DrvEnableDriver

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
ULONG* APIENTRY DrvGetMasks(DHPDEV dhpdev)
{
    ULONG* masks;
    GPEMode mode;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dhpdev);

    GetGPE()->GetModeInfo(&mode, 0);

    switch (mode.format) {
    case gpe16Bpp:
        masks = &RGBBitMask[0][0];
        break;
    case gpe24Bpp:
        masks = &RGBBitMask[1][0];
        break;
    case gpe32Bpp:
        masks = &RGBBitMask[2][0];
        break;
    default:
        masks = NULL;
        break;
    } //switch

    return masks;

} //DrvGetMasks

//------------------------------------------------------------------------------
//
// Function: CombineSurface_Update_thread
//
//      This function process PXP calling raised by Blt and Line operation.
//
// Parameters
//      lpParameter.
//          [in] Thread data passed to the function.
//
// Returns
//      Nonzero indicates success. Zero indicates failure.
//
//------------------------------------------------------------------------------
DWORD WINAPI CombineSurface_Update_thread(LPVOID lpParameter)
{
    DDLcdif* ddgpe = (DDLcdif*) lpParameter;

    DWORD result;

    CeSetThreadPriority(CombineSurface_thread, 100);

    CombineSurface_thread_running = true;

    while (CombineSurface_thread_running)
    {

        result = WaitForSingleObject(m_hCombSurfNeedUdEvent, INFINITE);
        
        if ( WAIT_OBJECT_0 == result)
        {
            result = WaitForSingleObject(m_hCombSurfUpdatedEvent, 100);
            if ( WAIT_TIMEOUT == result)
            {         
                if(ddgpe->m_bManualCombine && ddgpe->m_pCombinedSurface && ddgpe->m_hPXP &&(INVALID_HANDLE_VALUE != ddgpe->m_hPXP))
                {
                    EnterCriticalSection(&ddgpe->m_PxpOperateCS);
                    if(!ddgpe->m_hPXP)
                    {
                        LeaveCriticalSection(&ddgpe->m_PxpOperateCS);
                    }
                    else
                    {
                        PXPStartProcess(ddgpe->m_hPXP, FALSE);
                        LeaveCriticalSection(&ddgpe->m_PxpOperateCS);
                    }
                }                
            }            
            else if ( WAIT_OBJECT_0 != result)
                RETAILMSG(1,(TEXT(": Waiting for combine surface updated event error!\r\n")));
        }
        else if (WAIT_TIMEOUT != result)
            RETAILMSG(1,(TEXT(": Waiting for combine surface need update event error!\r\n")));
    }//while

    return 0;
}//CombineSurface_Update_thread

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
GPE* GetGPE()
{
    DEBUGMSG(1, (L"Display Driver GetGPE\r\n"));
    return (GPE*) DDLcdif::GetInstance();
} //GetGPE

//------------------------------------------------------------------------------
//
// Function: PowerHandler
//
// Power handler function for a GPE-compliant driver.
// We do not thing here since DrvEscape will handle for it.
//
// Parameters:
//      bOff.
//          [in] Power status
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID
DDLcdif::PowerHandler(
    BOOL bOff
    )
{
    // turn off the display if it is not already turned off
    // (turning on is controlled by DDLcdif::SetPmPowerState)
    if (bOff && !bSuspended) {
        DEBUGMSG(1, (TEXT("DDLcdif::PowerHandler: TurnOff Display\r\n")));
        bSuspended = TRUE;
    }
    else
    {
        DEBUGMSG(1, (TEXT("DDLcdif::PowerHandler: TurnOn Display\r\n")));
        bSuspended = FALSE;    
    }
}


//------------------------------------------------------------------------------
//
// Function: DDLcdif
//
// Constructor of DDLcdif class.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
DDLcdif::DDLcdif()
{
    m_bManualCombine = FALSE;
    m_bWaitPXPNotBusy = FALSE;

    BOOL result = TRUE;

    m_nOverlayAlign = 0;
    m_nDownScaleExp = 0;
    GetOverlayAlign();
    GetDownScaleFactorExp();
    m_nAlignMask = (UINT16)(~(m_nOverlayAlign - 1));

    m_hPXP = NULL;
    //  unsigned long PhysicalMemOffset;
    m_pUBufAdjVirtAddr = m_pVBufAdjVirtAddr = NULL;
    /*DWORD irq ;*/

    DEBUGMSG(1, (L"DDLcdif::DDLcdif\r\n"));

    fflush(stdout);

    InitializeCriticalSection(&AllocCS);

    BSPSetDisplayController();

    
    //
    // Allocate physical memory and surface heap
    // Initializes FrameBufferSize, PhysicalMemAddr,
    // VirtualMemAddr, and FrameBufferHeap
    //
    if (!AllocPhysicalMemory())
    {
        ERRORMSG(1, (L"DDLcdif: failed to allocate physical memory.\r\n"));
        result = FALSE;
        ASSERT(0);
    }

    GetModeInfoEx(&m_ModeInfoEx, DISPLAY_MODE_DEVICE);
    m_pMode = &m_ModeInfoEx.modeInfo;
    m_pModeEx = &m_ModeInfoEx;

    m_nScreenHeightSave = m_pMode->height;
    m_nScreenWidthSave = m_pMode->width;
	//
	//LQK OCT-10-2012: For correct rotation
	//
	//m_iRotate = GetRotateModeFromReg();
	m_iRotate = 0;
    SetRotateParams();
    m_nScreenWidth  = m_pMode->width;
    m_nScreenHeight = m_pMode->height;
    m_nScreenBpp = m_pMode->Bpp;
    m_nScreenStride = ((m_nScreenWidthSave * (m_nScreenBpp / 8) + 3) >> 2) << 2;
    m_pModeEx->lPitch = m_nScreenStride;
    m_dwPhysicalModeID = m_pMode->modeId;
    m_nCurrentDisplayMode = m_pMode->modeId;

    memset(&m_CursorXorShape, 0x0, 64*64);
    memset(&m_CursorRect, 0x0, sizeof(m_CursorRect));

    //
    // Allocate primary surfaces for LCD
    //
    if (!AllocPrimarySurfaces(DISPLAY_MODE_DEVICE))
    {
        ERRORMSG(1, (TEXT("DDLcdif: failed to create primary surfaces\r\n")));
        result = FALSE;
        ASSERT(0);
    }

    if(m_pPrimarySurface!=NULL)
        m_pPrimarySurface->SetRotation(m_nScreenWidth, m_nScreenHeight, m_iRotate);

    InitializeCriticalSection(&m_AllocCS);
    InitializeCriticalSection(&m_DrawCS);
    InitializeCriticalSection(&m_IntrCS);
    InitializeCriticalSection(&m_PxpOperateCS);

    // Notify the system that we are a power-manageable device.
    //
    m_Dx = D0;
    if(!AdvertisePowerInterface(g_hmodDisplayDll))
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("DDLcdif: failed to enable power management\r\n")));
        // non fatal
        // ASSERT(0);
    }


    m_pVisibleOverlay = NULL;
    m_pCombinedSurface = NULL;
    m_pRotatedSurface = NULL;
    m_iOrgRotSurfaceWidth = m_iOrgRotSurfaceHeight = 0;

    m_pOverlaySurfaceOp = (pOverlaySurf_t)malloc(sizeof(overlaySurf_t));
    if(m_pOverlaySurfaceOp)
    {
        memset(m_pOverlaySurfaceOp, 0x00, sizeof(overlaySurf_t));
    }
    else
    {
        ERRORMSG(1, (TEXT("DDLcdif Init: failed to allocate memory for graphic windower operator, error(%d)\r\n"), GetLastError()));
        result = FALSE;
        ASSERT(0);
       
    }

    // CreateThread for CombineSurface Update
    CombineSurface_thread = CreateThread(NULL, 0, &CombineSurface_Update_thread, this, 0, NULL);
    if (CombineSurface_thread == NULL)
        ERRORMSG(1, (TEXT("DDLcdif Init: failed to create thread for combine surface refresh, error(%d)\r\n"), GetLastError()));

    m_hCombSurfNeedUdEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hCombSurfNeedUdEvent == NULL)
        ERRORMSG(1,(TEXT("Create handle for combine surface need update event failed\r\n")));

    m_hCombSurfUpdatedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hCombSurfUpdatedEvent == NULL)
        ERRORMSG(1,(TEXT("Create handle for combine surface updated event failed\r\n")));

#ifdef USE_DCP
    dcp_Initialize();
#endif
    SetVisibleSurface(m_pBackgroundSurface);
    BSPBacklightEnable(TRUE);

	//LQK SEP-19-2012
	HANDLE ShutdownSplash_thread = CreateThread(NULL, 0, &ShutdownSplash, this, 0, NULL);
	if (ShutdownSplash_thread == NULL)
		ERRORMSG(1, (TEXT("DDLcdif Init: failed to create thread for shutdown splash, error(%d)\r\n"), GetLastError()));
	CloseHandle( ShutdownSplash_thread );
	//END LQK SEP-19-2012

    DEBUGMSG(1, (L"DDLcdif: ctor-\r\n"));

} //DDLcdif

//------------------------------------------------------------------------------
//
// Function: ~DDLcdif
//
// Destructor of DDLcdif class.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
DDLcdif::~DDLcdif()
{
    DEBUGMSG (1, (L"~DDLcdif enter\r\n"));

    if (VirtualMemAddr != NULL)
    {
        UnmapViewOfFile(VirtualMemAddr);
        VirtualFree(VirtualMemAddr, 0, MEM_RELEASE);
        VirtualMemAddr = NULL;
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

    if ((NULL != m_hPXP)&&(INVALID_HANDLE_VALUE != m_hPXP))
    {
        PXPCloseHandle(m_hPXP);
        m_hPXP = NULL;
    }

    // Release the overlay operator
    if(m_pOverlaySurfaceOp)
    {
        free(m_pOverlaySurfaceOp);
        m_pOverlaySurfaceOp = NULL;
    }

    FreePrimarySurfaces();

    if(m_pCombinedSurface)
    {
        delete m_pCombinedSurface;
        m_pCombinedSurface = NULL;
    }
    
    if(m_pVBufAdjVirtAddr)
    {
        FreePhysMem( m_pVBufAdjVirtAddr );
        m_pVBufAdjVirtAddr = NULL;
    }

    if(m_pUBufAdjVirtAddr)
    {
        FreePhysMem( m_pUBufAdjVirtAddr );
        m_pUBufAdjVirtAddr = NULL;
    }

    if(m_pRotatedSurface)
    {
        delete m_pRotatedSurface;
        m_pRotatedSurface = NULL;
    }

    DEBUGMSG (1, (L"~DDLcdif exit\r\n"));

    lcd_thread_running = false;
    CombineSurface_thread_running = false;    
    CombineSurface_thread = NULL;

    DeleteCriticalSection(&m_IntrCS);
    DeleteCriticalSection(&m_AllocCS);
    DeleteCriticalSection(&m_DrawCS);
    DeleteCriticalSection(&m_PxpOperateCS);

    SingletonDDGPE = NULL;
} //~DDLcdif
/* End DDLcdif Constructor/Destructor */

//------------------------------------------------------------------------------
//
// Function: GetInstance
//
// Start DDLcdif Public Routines.
//
// Parameters:
//      None.
//
// Returns:
//      DDLcdif pointer.
//
//------------------------------------------------------------------------------
DDLcdif* DDLcdif::GetInstance()
{
    if (SingletonDDGPE == NULL)
        SingletonDDGPE = new DDLcdif();

    return SingletonDDGPE;
} //GetInstance

//------------------------------------------------------------------------------
//
// Function: PoweroffLCDIF
//
//      This function turns off the LCD.
//
// Parameters
//      None.
//
// Returns
//      None.
//
//------------------------------------------------------------------------------
VOID DDLcdif::PoweroffLCDIF()
{
    BSPBacklightEnable(FALSE);
    BSPDisplayPowerHandler(POWEROFF);
}

//------------------------------------------------------------------------------
//
// Function: PowerOnLCDIF
//
//      This function turns on the LCD and starts the LCDIF running.
//
// Parameters
//      None.
//
// Returns
//      None.
//
//------------------------------------------------------------------------------
VOID DDLcdif::PowerOnLCDIF()
{
    BSPDisplayPowerHandler(POWERON);
    if(m_pCombinedSurface)
        SetVisibleSurface((GPESurf *) m_pCombinedSurface);
    else
        SetVisibleSurface((GPESurf *) m_pBackgroundSurface);

    BSPBacklightEnable(TRUE);
}

//------------------------------------------------------------------------------
//
// Function: AllocPhysicalMemory
//
// Allocate contiguous memory for the display driver.
// NOTE: Should be called only once at startup.
//
// Updates class members:
//      FrameBufferSize
//      PhysicalMemAddr
//      VirtualMemAddr
//      FrameBufferHeap
//
// Parameters:
//      none
//
// Returns:
//      TRUE for success, FALSE if fail.
//
//------------------------------------------------------------------------------
BOOL DDLcdif::AllocPhysicalMemory()
{


    BOOL rc = FALSE;

    FrameBufferSize = BSPGetVideoMemorySize();
    m_pVideoMemory = (PUCHAR)AllocPhysMem(FrameBufferSize,       // Frame buffer size
        PAGE_EXECUTE_READWRITE,   // All needed rigth
        0,                 // The default system alignment is used
        0,                        // Reserved
        (ULONG *) &m_nLAWPhysical);
    PhysicalMemOffset = m_nLAWPhysical >> 8;
    PhysicalMemAddr = (void *)m_nLAWPhysical;


    // Check if virtual mapping failed
    if (!m_pVideoMemory)
    {
        ERRORMSG(1,
            (TEXT("MmMapIoSpace failed!\r\n")));
         ASSERT(0);
         goto _AllocPhysMemdone;
    }


    //Allocate Frame Buffer Space in shared memory
    m_hLAWMapping = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        FrameBufferSize,
        NULL);

    if (m_hLAWMapping != NULL) 
    {
        VirtualMemAddr = (unsigned char *)MapViewOfFile(
            m_hLAWMapping,
            FILE_MAP_WRITE,
            0,
            0,
            0);
    } 
    else 
    {
        ERRORMSG(1, (TEXT("AllocPhysicalMemory: failed at MapViewOfFile(), error[%d]\r\n"), FrameBufferSize, GetLastError()));
        ASSERT(0);
        goto _AllocPhysMemdone;
    }

    //
    // VirtualCopy is called in order to create a page-table entry for the mapped
    // region. The physical address must be divided by 256 when the PAGE_PHYSICAL
    // flag is used.
    //
    // NOTE: Memory mapped with the PAGE_PHYSICAL flag cannot be freed by calling the
    // VirtualFree function. It remains allocated until the device is rebooted.
    //

    BOOL result = VirtualCopy(
                    VirtualMemAddr,
                    (LPVOID)PhysicalMemOffset,
                    FrameBufferSize,
                    PAGE_READWRITE|PAGE_NOCACHE|PAGE_PHYSICAL);


    if(!result)
    {
        RETAILMSG(1, (TEXT("AllocPhysicalMemory: failed at VirtualCopy(), error[%d]\r\n"), FrameBufferSize, GetLastError()));
        ASSERT(0);
        goto _AllocPhysMemdone;        
    }
    else
    {
        // BSP_VID_MEM_CACHE_WRITETHROUGH defaults to TRUE,
        // i.e. Using cached, write-through mode
        // to provides performance benefit.
        BOOL bCachedWT = BSPGetCacheMode();

        if (bCachedWT)
        {
            // Update the Framebuffer to be cached, write-through
            VirtualSetAttributes(VirtualMemAddr, FrameBufferSize, 0x8, 0xC, NULL);
        }
        else
        {
            // Update the Framebuffer to be non-cached, bufferable
            VirtualSetAttributes(VirtualMemAddr, FrameBufferSize, 0x4, 0xC, NULL);
        }
    }

    memset(VirtualMemAddr, 0, FrameBufferSize);

    FrameBufferHeap = new SurfaceHeap(FrameBufferSize,
                                      (ADDRESS) VirtualMemAddr,
                                      NULL, NULL);
    if (FrameBufferHeap == NULL) {
        ERRORMSG (1, (L"Couldn't allocate primary surface heap\n"));
        ASSERT(0);
        goto _AllocPhysMemdone;
    }

    rc = TRUE;

_AllocPhysMemdone:
    if(rc)
    {
        DEBUGMSG(1, (TEXT("DISPLAY: memory allocation OK\r\n")));
        DEBUGMSG(1, (TEXT("    Virtual address : 0x%08x\r\n"), (DWORD) VirtualMemAddr));
        DEBUGMSG(1, (TEXT("    Physical address: 0x%08x\r\n"), (DWORD) PhysicalMemAddr));
        DEBUGMSG(1, (TEXT("    Size            : 0x%08x\r\n"), (DWORD) FrameBufferSize));
        DEBUGMSG(1, (TEXT("    Heap address    : 0x%08x\r\n"), (DWORD) FrameBufferHeap));
    }
    return rc;


}


//------------------------------------------------------------------------------
//
// Function: AllocPrimarySurfaces
//
// This method allocate the primary surfaces.
//
// Parameters:
//      modeNumber
//          [in] Display mode.
//
// Returns:
//      TRUE        successful
//      FALSE       failed
//
//------------------------------------------------------------------------------
BOOL DDLcdif::AllocPrimarySurfaces(DWORD modeNumber)
{
    BOOL rc = FALSE;
    DWORD phys,virt,offset;

    // LCD surface
    //
    if(FAILED(AllocSurface((DDGPESurf **) &m_pPrimarySurface,
                            BSPGetWidth(modeNumber),
                            BSPGetHeight(modeNumber),
                            m_pMode->format,
                            m_pModeEx->ePixelFormat,
                            GPE_REQUIRE_VIDEO_MEMORY)))
    {
        ERRORMSG(1,(L"DDLcdif:: alloc primary surface failed\r\n"));
        ASSERT(0);
        goto _AllocSurfdone;
    }

    virt = (DWORD) m_pPrimarySurface->Buffer();
    offset = virt - (DWORD) VirtualMemAddr;
    phys =  (DWORD) PhysicalMemAddr + offset;
    BSPSetDisplayBuffer((PVOID)phys, (PVOID)virt);
    m_pBackgroundSurface = (DDGPESurf *)m_pPrimarySurface;

    rc = TRUE;  // success

_AllocSurfdone:
    return rc;
}

//------------------------------------------------------------------------------
//
// Function: FreePrimarySurfaces
//
// This method frees the primary surfaces if previously allocated, and sets
// the pointers to NULL.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DDLcdif::FreePrimarySurfaces()
{

    if (m_pPrimarySurface)
    {
        DEBUGMSG(1,(L"DDLcdif::FreePrimarySurfaces\r\n"));
        delete m_pPrimarySurface;
        m_pPrimarySurface = NULL;
    }

}

//------------------------------------------------------------------------------
//
// Function: SetMode
//
// This method requests a specific mode for the display.
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
SCODE DDLcdif::SetMode(int modeNo, HPALETTE * pPalette)
{
    if (!BSPIsValidMode(modeNo))
    {
        ERRORMSG(1, (L"DDLcdif::SetMode: invalid mode\r\n"));
        return E_INVALIDARG;
    }

    EnterCriticalSection(&m_DrawCS);

    // m_pMode maybe released by DrvDisablePDEV()
    // re-point to our mode info struct
    if(m_pMode == NULL)
    {
        m_pModeEx = &m_ModeInfoEx;
        m_pMode   = &m_ModeInfoEx.modeInfo;

        // force full refresh
        m_nCurrentDisplayMode = DISPLAY_MODE_NONE;
    }
    
    DEBUGMSG(1, (TEXT("DDLcdif::SetMode: requested mode %d\r\n"),modeNo));
    
    if (m_nCurrentDisplayMode != modeNo)
    {
        // Update the mode info struct based on the requested mode.
        GetModeInfoEx(&m_ModeInfoEx, modeNo);

        // Update members with the new values from mode info
        m_nScreenHeightSave = m_pMode->height;
        m_nScreenWidthSave  = m_pMode->width;
        m_nScreenHeight = m_pMode->height;
        m_nScreenWidth  = m_pMode->width;
        m_nScreenBpp = m_pMode->Bpp;
    }
    m_nScreenStride = ((m_nScreenWidthSave * (m_nScreenBpp / 8) + 3) >> 2) << 2;
    m_pModeEx->lPitch = m_nScreenStride;
    m_dwPhysicalModeID = m_pMode->modeId;

    if (pPalette)
    {
        *pPalette = EngCreatePalette(PAL_BITFIELDS,
                                    0,
                                    NULL,
                                    m_pModeEx->dwRBitMask,
                                    m_pModeEx->dwGBitMask,
                                    m_pModeEx->dwBBitMask);

        if (*pPalette == 0)
        {
            RETAILMSG(1, (L"DDLcdif::SetMode: create palette failed\r\n"));
            ASSERT(0);
        }
    }

    if(m_nCurrentDisplayMode != modeNo)
    {
        m_nCurrentDisplayMode = modeNo;
        m_dwPhysicalModeID = modeNo;

        // Because m_nCurrentDisplayMode != modeNo, we need to swap primary surface
        // from LCD to TV or vice-versa.

        // Release previous primary surface         
        FreePrimarySurfaces();
        
        // Alloc new primary surface
        AllocPrimarySurfaces(modeNo);

        // reconfigure hardware
        BSPSetMode(modeNo);

        // clear entire display
        memset((PUCHAR) m_pPrimarySurface->Buffer(), 0xff, m_nScreenStride * m_nScreenHeight);
        // start LCDIF
        SetVisibleSurface(m_pBackgroundSurface);

        m_pPrimarySurface->SetRotation(m_pMode->width, m_pMode->height, m_iRotate);
    }

    DEBUGMSG(1, (TEXT("DDLcdif::SetMode: mode %d selected\r\n"),m_nCurrentDisplayMode));
    LeaveCriticalSection(&m_DrawCS);
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
SCODE DDLcdif::GetModeInfo(GPEMode* mode, INT modeNumber)
{
    GPEModeEx info;
    SCODE ret;

    DEBUGMSG(1,((L"DDLcdif::GetModeInfo \r\n")));

    ret = GetModeInfoEx(&info, modeNumber);
    if (ret != S_OK) return ret;

    *mode = info.modeInfo;

    return S_OK;
} //GetModeInfo

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
int DDLcdif::NumModes()
{
    DEBUGMSG (1, ((L"DDLcdif::NumModes\r\n")));
    return BSPGetModeNum();
} //NumModes

//------------------------------------------------------------------------------
//
// Function: WaitForNotBusy
//
// Method to wait for the frame buffer is ready to update.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DDLcdif::WaitForNotBusy(void)
{
#ifdef USE_DCP
    dcp_WaitForComplete(blit_handle, INFINITE);
#endif
    //DEBUGMSG (0, ((L"DDLcdif::WaitForNotBusy\r\n")));
} //WaitForNotBusy

//------------------------------------------------------------------------------
//
// Function: WaitForVBlank
//
// Method to wait for vertical blank slot.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DDLcdif::WaitForVBlank(void)
{
    //DEBUGMSG (1, ((L"DDLcdif::WaitForVBlank\r\n")));
} //WaitForVBlank

//------------------------------------------------------------------------------
//
// Function: IsBusy
//
// Method to determine whether LCDIF ASync is available
// to transfer the frame buffer to display panel.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE            busy
//      FALSE           not busy
//
//------------------------------------------------------------------------------
int DDLcdif::IsBusy(void)
{
    DEBUGMSG (1, ((L"DDLcdif::IsBusy\r\n")));
#ifdef USE_DCP
    if (dcp_WaitForComplete(blit_handle, 0) != ERROR_SUCCESS) return 1;
#endif
    return 0;
} //IsBusy

//------------------------------------------------------------------------------
//
// Function: GetPhysicalVideoMemory
//
// Method to get physical video memory info.
//
// Parameters:
//      physicalMemoryBase
//          [in/out] Pointer to physical memory base address.
//      videoMemorySize
//          [in/out] Pointer to video memory size.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DDLcdif::GetPhysicalVideoMemory(unsigned long *physicalMemoryBase,
                                           unsigned long *videoMemorySize)
{
    DEBUGMSG (1, (L"DDLcdif::GetPhysicalVideoMemory enter\r\n"));

    *physicalMemoryBase = (unsigned long) VirtualMemAddr;
    *videoMemorySize = FrameBufferSize;

    DEBUGMSG (1, (L"DDLcdif::GetPhysicalVideoMemory exit\r\n"));
} //GetPhysicalVideoMemory

//------------------------------------------------------------------------------
//
// Function: InVBlank
//
// Timing method.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE            busy
//      FALSE           not busy
//
//------------------------------------------------------------------------------
INT DDLcdif::InVBlank(void)
{
    static BOOL value = FALSE;
    DEBUGMSG (1, ((L"DDLcdif::InVBlank\r\n")));
    value = !value;
    return value;
}

//------------------------------------------------------------------------------
//
// Function: SetPalette
//
// Palette method that a GPE-based driver implements.
//
// Parameters:
//      source.
//      firstEntry.
//      numEntries.
//
// Returns:
//      S_OK            successful
//      other           failed
//
//------------------------------------------------------------------------------
SCODE DDLcdif::SetPalette(const PALETTEENTRY *source,
                                USHORT firstEntry,
                                USHORT numEntries)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(source);
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(firstEntry);
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(numEntries);

    DEBUGMSG (1, (L"DDLcdif::SetPalette\r\n"));

    return S_OK;
}

//------------------------------------------------------------------------------
//
// Function: ConvertStringToGuid
//
// This converts a string into a GUID.
//
// Parameters:
//      pszGuid
//          [in] Pointer to GUID in string format.
//
//      pGuid
//          [out] Pointer to GUID struct for output
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//------------------------------------------------------------------------------
BOOL
DDLcdif::ConvertStringToGuid (LPCTSTR pszGuid, GUID *pGuid)
{
    UINT Data4[8];
    int  Count;
    BOOL fOk = FALSE;
    TCHAR *pszGuidFormat = _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

    DEBUGCHK(pGuid != NULL && pszGuid != NULL);
    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try
    {
        if (_stscanf(pszGuid, pszGuidFormat, &pGuid->Data1,
            &pGuid->Data2, &pGuid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3],
            &Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11)
        {
            for(Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++)
            {
                PREFAST_SUPPRESS(6385, "_stscanf is a banned API, must be guaranteed by ourselves.");
                pGuid->Data4[Count] = (UCHAR) Data4[Count];
            }
        }
        fOk = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        fOk = FALSE;
    }

    return fOk;
}
//------------------------------------------------------------------------------
//
// Function: DrvEscapeGAPI
//
// This routine handles the needed DrvEscape codes for GAPI. Note that GAPI
// is only supported for Windows Mobile.
//
// Parameters:
//      iEsc
//          [in] Query. The meaning of the other parameters depends on
//          this value. QUERYESCSUPPORT is the only predefined value; it
//          queries whether the driver supports a particular escape function.
//          In this case, pvIn points to an escape function number; cjOut and
//          pvOut are ignored. If the specified function is supported, the
//          return value is nonzero.
//
//      cjIn
//          [in] Size, in bytes, of the buffer pointed to by pvIn.
//
//      pvIn
//          [in] Pointer to the input data for the call. The format of the
//          input data depends on the query specified by the iEsc parameter.
//
//      cjOut
//          [in] Size, in bytes, of the buffer pointed to by pvOut.
//
//      pvOut
//          [out] Pointer to the output buffer. The format of the output data
//          depends on the query specified by the iEsc parameter.
//
// Returns:
//      ESC_SUCCESS    successful
//      ESC_FAILED     failed
//
//------------------------------------------------------------------------------
ULONG DDLcdif::DrvEscapeGAPI(ULONG iEsc, ULONG cjIn, void * pvIn, ULONG cjOut, void * pvOut)
{
    UNREFERENCED_PARAMETER(iEsc);
    UNREFERENCED_PARAMETER(cjIn);
    UNREFERENCED_PARAMETER(pvIn);
    UNREFERENCED_PARAMETER(cjOut);
    UNREFERENCED_PARAMETER(pvOut);

    return (ULONG)ESC_FAILED;
}

//------------------------------------------------------------------------------
//
// Function: AdvertisePowerInterface
//
// This routine notifies the OS that we support the
// Power Manager IOCTLs (through ExtEscape(), which
// calls DrvEscape()).
//
// Parameters:
//      hInst
//          [in] handle to module DLL
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//------------------------------------------------------------------------------
BOOL DDLcdif::AdvertisePowerInterface(HMODULE hInst)
{
    GUID gTemp;
    BOOL fOk = FALSE;
    HKEY hk;
    DWORD dwStatus;

    // assume we are advertising the default class
    _tcscpy(m_szGuidClass, PMCLASS_DISPLAY);
    m_szGuidClass[MAX_PATH-1] = 0;
    fOk = ConvertStringToGuid(m_szGuidClass, &gTemp);
    DEBUGCHK(fOk);

    // check for an override in the registry
    dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\GDI\\Drivers"), 0, 0, &hk);
    if(dwStatus == ERROR_SUCCESS)
    {
        DWORD dwType, dwSize;
        dwSize = sizeof(m_szGuidClass);
        dwStatus = RegQueryValueEx(hk, _T("DisplayPowerClass"), NULL, &dwType, (LPBYTE)m_szGuidClass, &dwSize);
        if(dwStatus == ERROR_SUCCESS && dwType == REG_SZ)
        {
            // got a guid string, convert it to a guid
            fOk = ConvertStringToGuid(m_szGuidClass, &gTemp);
            DEBUGCHK(fOk);
        }

        // release the registry key
        RegCloseKey(hk);
    }

    // figure out what device name to advertise
    if(fOk)
    {
        fOk = GetModuleFileName(hInst, m_szDevName, sizeof(m_szDevName) / sizeof(m_szDevName[0]));
        DEBUGCHK(fOk);
    }

    // now advertise the interface
    if(fOk)
    {
        fOk = AdvertiseInterface(&gTemp, m_szDevName, TRUE);
        DEBUGCHK(fOk);
    }

    return fOk;
}



//------------------------------------------------------------------------------
//
// Function: SetDisplayPower
//
// This function sets the display hardware according
// to the desired video power state. Also updates
// current device power state variable.
//
// Parameters:
//      dx
//          [in] Desired video power state.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID DDLcdif::SetDisplayPower(CEDEVICE_POWER_STATE dx)
{
    switch(dx)
    {
        case D0:
            if(m_Dx != dx)
            {
                PowerOnLCDIF();

                m_Dx = D0;
            }
            break;

        case D1:
        case D2:
        case D3:
        case D4:
            if(m_Dx != D4)
            {
                Sleep(200); // IMPORTANT: Wait at least one frame time!

                PoweroffLCDIF();

                m_Dx = D4;
            }
            break;

        default:
            break;
    }
}

//------------------------------------------------------------------------------
//
// Function: DrvEscape
//
// This routine handles the needed DrvEscape codes.
//
// Parameters:
//      pso
//          [in] Pointer to a SURFOBJ structure that describes the surface
//          to which the call is directed.
//
//      iEsc
//          [in] Query. The meaning of the other parameters depends on
//          this value. QUERYESCSUPPORT is the only predefined value; it
//          queries whether the driver supports a particular escape function.
//          In this case, pvIn points to an escape function number; cjOut and
//          pvOut are ignored. If the specified function is supported, the
//          return value is nonzero.
//
//      cjIn
//          [in] Size, in bytes, of the buffer pointed to by pvIn.
//
//      pvIn
//          [in] Pointer to the input data for the call. The format of the
//          input data depends on the query specified by the iEsc parameter.
//
//      cjOut
//          [in] Size, in bytes, of the buffer pointed to by pvOut.
//
//      pvOut
//          [out] Pointer to the output buffer. The format of the output data
//          depends on the query specified by the iEsc parameter.
//
// Returns:
//      TRUE            successful
//      FALSE          failed
//
//------------------------------------------------------------------------------
ULONG DDLcdif::DrvEscape(SURFOBJ * pso, ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut)
{
    ULONG retval = (ULONG)ESC_FAILED;
    //PVIDEO_POWER_MANAGEMENT psPowerManagement;

    UNREFERENCED_PARAMETER(pso);

    switch (iEsc)
    {
     case QUERYESCSUPPORT:
            if(pvIn != NULL && cjIn == sizeof(DWORD))
            {
                // Query DrvEscape support functions
                DWORD EscapeFunction;
                EscapeFunction = *(DWORD *)pvIn;
                if ((EscapeFunction == QUERYESCSUPPORT)          ||
                    (EscapeFunction == IOCTL_POWER_CAPABILITIES) ||
                    (EscapeFunction == IOCTL_POWER_QUERY)        ||
                    (EscapeFunction == IOCTL_POWER_SET)          ||
                    (EscapeFunction == IOCTL_POWER_GET)          ||
                    (EscapeFunction == DRVESC_GETSCREENROTATION) ||
                    (EscapeFunction == DRVESC_SETSCREENROTATION) ||
                    (EscapeFunction == DISPLAY_SET_OUTPUT_MODE)  ||
                    (EscapeFunction == DISPLAY_GET_OUTPUT_MODE)  ||
                    (EscapeFunction == VM_SETATTEX)
                    )
                    retval = ESC_SUCCESS;
                else
                    retval = DrvEscapeGAPI(iEsc, cjIn, pvIn, cjOut, pvOut);

                if(retval != ESC_SUCCESS)
                    SetLastError(ERROR_INVALID_PARAMETER);
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);
            break;

        case IOCTL_POWER_CAPABILITIES:
            //DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DrvEscape: IOCTL_POWER_CAPABILITIES\r\n")));
            if(pvOut != NULL && cjOut == sizeof(POWER_CAPABILITIES))
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES)pvOut;
                memset(ppc, 0, sizeof(POWER_CAPABILITIES));
                ppc->DeviceDx = 0x11;   // support D0 and D4
                ppc->WakeFromDx = 0x00; // No wake capability
                ppc->InrushDx = 0x00;       // No in rush requirement
                ppc->Power[D0] = 600;                   // 0.6W
                ppc->Power[D1] = (DWORD)PwrDeviceUnspecified;
                ppc->Power[D2] = (DWORD)PwrDeviceUnspecified;
                ppc->Power[D3] = (DWORD)PwrDeviceUnspecified;
                ppc->Power[D4] = 0;
                ppc->Latency[D0] = 0;
                ppc->Latency[D1] = (DWORD)PwrDeviceUnspecified;
                ppc->Latency[D2] = (DWORD)PwrDeviceUnspecified;
                ppc->Latency[D3] = (DWORD)PwrDeviceUnspecified;
                ppc->Latency[D4] = 0;
                ppc->Flags = 0;
                retval = ESC_SUCCESS;
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);
            break;

        case IOCTL_POWER_QUERY:
            if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
            {
                // return a good status on any valid query, since we are always ready to
                // change power states.
                CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pvOut;
                if(VALID_DX(NewDx))
                {
                    // this is a valid Dx state so return a good status
                    retval = ESC_SUCCESS;
                }
                else
                    SetLastError(ERROR_INVALID_PARAMETER);
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DrvEscape: IOCTL_POWER_QUERY %u %s\r\n"),
                                        NewDx, retval? L"succeeded" : L"failed"));
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);
            break;

        case IOCTL_POWER_GET:
            if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
            {
                CEDEVICE_POWER_STATE CurrentDx = m_Dx;
                *(PCEDEVICE_POWER_STATE)pvOut = CurrentDx;
                retval = ESC_SUCCESS;
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DrvEscape: %s IOCTL_POWER_GET: passing back %u\r\n"), m_szDevName, CurrentDx));
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);
            break;

        case IOCTL_POWER_SET:
            if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
            {
                CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pvOut;
                if(VALID_DX(NewDx))
                {
                    SetDisplayPower(NewDx);
                    retval = ESC_SUCCESS;
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DrvEscape: %s IOCTL_POWER_SET %u: passing back %u\r\n"), m_szDevName, NewDx, m_Dx));
                }
                else
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DrvEscape: IOCTL_POWER_SET: invalid state request %u\r\n"), NewDx));
                }
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);
            break;

        case DRVESC_GETSCREENROTATION:
            *(int *)pvOut = ((DMDO_0 | DMDO_90 | DMDO_180 | DMDO_270) << 8) | ((BYTE)m_iRotate);
            retval = DISP_CHANGE_SUCCESSFUL;
            break;

        case DRVESC_SETSCREENROTATION:
            if ((cjIn == DMDO_0)   ||
                (cjIn == DMDO_90)  ||
                (cjIn == DMDO_180) ||
                (cjIn == DMDO_270))
            {
                retval = DynRotate(cjIn);
            }
            else
            {
                retval = (ULONG)DISP_CHANGE_BADMODE;
            }
            break;

        case DISPLAY_SET_OUTPUT_MODE:
            if(pvIn != NULL && cjIn == sizeof(DWORD))
            {
                // TODO: call the DDGPE version of SetMode with 3rd parm=TRUE
                if (S_OK == SetMode(*((DWORD *)pvIn),0))
                    retval = ESC_SUCCESS;
            }
            break;

        case DISPLAY_GET_OUTPUT_MODE:
            if(pvOut != NULL && cjOut == sizeof(DWORD))
            {
                // TODO: get DDGPE mode # not physical mode ID
                *((DWORD*)pvOut) = GetPhysicalModeId();
                retval = ESC_SUCCESS;
            }
            break;

        case VM_SETATTEX:
            pLcdifVMSetAttributeExData pSetData;
            pSetData = (pLcdifVMSetAttributeExData)pvIn;

            if(LcdifVMSetAttributeEx(pSetData)) retval = ESC_SUCCESS;
            break;

        default :
            retval = DrvEscapeGAPI(iEsc, cjIn, pvIn,cjOut, pvOut);
            break;

    }
    return retval;
} //DrvEscape


//------------------------------------------------------------------------------
//
// Function: GetRotateModeFromReg
//
// This function is used to read the registry to get the initial
// rotation angle.
//
// Parameters:
//      None.
//
// Returns:
//      returns default rotation angle.
//
//------------------------------------------------------------------------------
int DDLcdif::GetRotateModeFromReg()
{
    HKEY hKey;
    int nRet = DMDO_0;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      TEXT("SYSTEM\\GDI\\ROTATION"),
                                      0,0,
                                      &hKey)) {
        DWORD dwSize, dwAngle, dwType = REG_DWORD;

        dwSize = sizeof(DWORD);

        if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                             TEXT("ANGLE"),
                                             NULL,
                                             &dwType,
                                             (LPBYTE)&dwAngle,
                                             &dwSize)) {
            switch (dwAngle) {
            case 90:
                nRet = DMDO_90;
                break;
            case 180:
                nRet = DMDO_180;
                break;
            case 270:
                nRet = DMDO_270;
                break;
            case 0:
            default:
                nRet = DMDO_0;
                break;
            } //switch
        } //if

        RegCloseKey(hKey);
    } //if

    return nRet;
} //GetRotateModeFromReg


//------------------------------------------------------------------------------
//
// Function: SetRotateParms
//
// This function is used to set up the screen width and height
// based on the current rotation angle.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID DDLcdif::SetRotateParams(VOID)
{
    switch(m_iRotate)
    {
        case DMDO_90:
        case DMDO_270:
            m_pMode->height = m_nScreenWidthSave;
            m_pMode->width = m_nScreenHeightSave;
            break;

        case DMDO_0:
        case DMDO_180:
        default:
            m_pMode->width = m_nScreenWidthSave;
            m_pMode->height = m_nScreenHeightSave;
            break;
    }

    return;
}

//------------------------------------------------------------------------------
//
// Function: DynRotate
//
// This function is used to process ratation.
//
// Parameters:
//      angle.
//          [in] rotation angle
//
// Returns:
//      DISP_CHANGE_SUCCESSFUL.     successful
//
//------------------------------------------------------------------------------
LONG DDLcdif::DynRotate(int angle)
{
    GPESurfRotate * pSurf = (GPESurfRotate*) m_pBackgroundSurface;

    if (angle == m_iRotate)
        return DISP_CHANGE_SUCCESSFUL;

    m_iRotate = angle;

    switch(m_iRotate) {
    case DMDO_0:
    case DMDO_180:
        m_nScreenHeight = m_nScreenHeightSave;
        m_nScreenWidth  = m_nScreenWidthSave;
        break;

    case DMDO_90:
    case DMDO_270:
        m_nScreenHeight = m_nScreenWidthSave;
        m_nScreenWidth  = m_nScreenHeightSave;
        break;
    } //switch

    m_pMode->width  = m_nScreenWidth;
    m_pMode->height = m_nScreenHeight;

    pSurf->SetRotation(m_nScreenWidth, m_nScreenHeight, angle);

    return DISP_CHANGE_SUCCESSFUL;
} //DynRotate

//------------------------------------------------------------------------------
//
// Function: fill_vertical_stripes
//
// This function is used to fill vertical stripe for display test.
//
// Parameters:
//      pBuff.
//          [in/out] buffer to fill
//      width
//          [in] width to fill
//      height
//          [in] height to fill
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void DDLcdif::fill_vertical_stripes(UINT16* pBuff,
                                          UINT32 width,
                                          UINT32 height)
{
    UINT32 idx, jdx, kdx;
    const UINT32 stripeWidth = 30;

    // make 1 frame, vertical stripes of red, white blue, etc.,
    // make 8 stripes, each stripe is 30 pixels wide
    // each line is one frame / 320.  each vertical is one pixel = 3 bytes.
    // make 8 stripes, 30 pixels wide by 320 high (1/2 reversed from the other)
    for (idx = 0; idx < height/2; idx++) {
        for (jdx = 0; jdx < (width/stripeWidth); jdx++)
            for (kdx = 0; kdx < stripeWidth; kdx++) *pBuff++ = colors[jdx];
    } //for

    for (idx = 0; idx < height/2; idx++) {
        for (jdx = (width/stripeWidth); jdx > 0; jdx--)
            for (kdx = 0; kdx < stripeWidth; kdx++) *pBuff++ = colors[jdx - 1];
    } //for
} //fill_vertical_stripes


//------------------------------------------------------------------------------
//
//  Function: GetPhysicalModeId
//
//  This function returns the physical mode ID.
//
//  Parameters:
//      None.
//
//  Returns:
//      Physical mode ID
//
//------------------------------------------------------------------------------
DWORD DDLcdif::GetPhysicalModeId()
{
    return m_dwPhysicalModeID;
}


//------------------------------------------------------------------------------
//
// Function: GetModeInfoEx
//
// This method populates a GPEMode structure with data for the specified mode.
//
// Parameters:
//      pMode
//          [out]  Pointer to a GPEModeEx structure.
//
//      modeNo
//          [in]   number of mode to get info about.
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE DDLcdif::GetModeInfoEx(GPEModeEx* pModeEx, int modeNumber)
{
    if(!BSPIsValidMode(modeNumber))
        return E_INVALIDARG;
    else
    {
        BSPGetModeInfoEx(pModeEx, modeNumber);
        return S_OK;
    }

}

//------------------------------------------------------------------------------
//
// Function: GetOverlayAlign
//
// This function get overlay alignment.
//
// Parameters:
//      None.
//
// Returns:
//      Overlay alignment.
//
//------------------------------------------------------------------------------
DWORD DDLcdif::GetOverlayAlign()
{
    if (!m_nOverlayAlign)
    {    
        m_nOverlayAlign = BSPGetOverlayAlign();

        m_hPXP = PXPOpenHandle();
        DEBUGMSG(1,(_T("m_hPXP = %0x\r\n"),m_hPXP));
        // Bail if no PXP handle
        if ((m_hPXP == NULL)||(m_hPXP == INVALID_HANDLE_VALUE))
        {
            RETAILMSG(1,(_T("No Pixel Pipeline handle available! Will use 8 as default block size.\r\n")));
            m_nOverlayAlign = 8;
        }
        else
        {
            PXPSetBlockSize(m_hPXP, &m_nOverlayAlign);  //Set block size value to PXP driver.
            DEBUGMSG(1,(_T("m_nOverlayAlign = %d\r\n"), m_nOverlayAlign));
            PXPCloseHandle(m_hPXP);
        }

    }

    return m_nOverlayAlign;
}

//------------------------------------------------------------------------------
//
// Function: GetDownScaleFactorExp
//
// This function get PXP down scaling factor exponent value.
//
// Parameters:
//      None.
//
// Returns:
//      PXP down scaling factor exponent value.
//
//------------------------------------------------------------------------------
DWORD DDLcdif::GetDownScaleFactorExp()
{
    if (!m_nDownScaleExp)
    {
        m_hPXP = PXPOpenHandle();
        DEBUGMSG(1,(_T("m_hPXP = %0x\r\n"),m_hPXP));
        // Bail if no PXP handle
        if ((m_hPXP == NULL)||(m_hPXP == INVALID_HANDLE_VALUE))
        {
            RETAILMSG(1,(_T("No Pixel Pipeline handle available! Will use 1 as default exponent value.\r\n")));
            m_nDownScaleExp = 1;
        }
        else
        {
            UINT32 Exponent;
            PXPGetDownScaleFactorExponent(m_hPXP, &Exponent);  //Get exponent value from PXP driver.
            m_nDownScaleExp = Exponent;
            DEBUGMSG(1,(_T("m_nDownScaleExp = %d\r\n"), m_nDownScaleExp));
            PXPCloseHandle(m_hPXP);
        }
    }

    return m_nDownScaleExp;
}
/* End DDLcdif Private Routines */


