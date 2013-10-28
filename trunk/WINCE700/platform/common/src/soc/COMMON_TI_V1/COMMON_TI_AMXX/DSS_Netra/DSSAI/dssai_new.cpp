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
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/

#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <pm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <oal.h>
#include <oalex.h>

//#include "omap3530_clocks.h"

#include "omap.h"
#include "omap_dss_regs.h"
#include "omap_prcm_regs.h"
#include "dssai.h"
#include "ceddkex.h"
#include "_debug.h"
#include <oal_clock.h>
#include <sdk_padcfg.h>

#include <dssai.h>
#include <vpss.h>

#include <_debug.h>

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

//
//  Defines
//
#define DSS_REGS_SIZE           1024
#define GAMMA_BUFFER_SIZE       1024


#define FIFO_HIGHTHRESHOLD_NORMAL   (1024-1)
#define FIFO_HIGHTHRESHOLD_MERGED   (3072-1)

#define FIFO_LOWTHRESHOLD_NORMAL(x) (FIFO_HIGHTHRESHOLD_NORMAL - x * FIFO_NUM_BURSTS)
#define FIFO_LOWTHRESHOLD_MERGED(x) (FIFO_HIGHTHRESHOLD_MERGED - 3 * x * FIFO_NUM_BURSTS)

#define FIFO_NUM_BURSTS             (8)

#define FIFO_BURSTSIZE_4x32         (16)
#define FIFO_BURSTSIZE_8x32         (32)
#define FIFO_BURSTSIZE_16x32        (64)

#define HDMI_CLK                    148500000
#define DSI_PLL_FREQINT               2000000   // Fint = 2MHz
#define DSI_PLL_FREQSELVAL            0x07      // Fint select = 1.75MHz to 2.1MHz


#define CEIL_MULT(x, mult)   (x) = ( ( (x)/mult )+1) * mult;
#define FLOOR_MULT(x, mult)  (x) = ((x)/mult) * mult;

#define LCD_WIDTH                      (REGION_WIDTH)
#define LCD_HEIGHT                     (REGION_HEIGHT)

//
//  Structs
//

typedef struct OMAPPipelineConfig
{
    BOOL                    bEnabled;           // Enabled flag
    OMAP_DSS_DESTINATION    eDestination;       // Pipeline destination
    OMAPSurface*            pSurface;           // Surface to output
    OMAP_DSS_ROTATION       eRotation;          // Rotation angle of pipeline output
    BOOL                    bMirror;            // Mirror pipeline (left/right)
    DWORD                   dwDestWidth;        // Width of output
    DWORD                   dwDestHeight;       // Height of output
}
OMAPPipelineConfig;

typedef struct OMAPPipelineScaling
{
    DWORD                   dwHorzScaling;      // Horizontal surface decimation factor for pipeline
    DWORD                   dwVertScaling;      // Vertical surface decimation factor for pipeline
    DWORD                   dwInterlace;        // Interlace offset
}
OMAPPipelineScaling;


//
//  Globals
//
OMAPPipelineConfig  g_rgPipelineMapping[NUM_DSS_PIPELINES] =
{
    { FALSE, OMAP_DSS_DESTINATION_LCD, NULL, OMAP_DSS_ROTATION_0, FALSE, 0, 0 },    // GFX
    { FALSE, OMAP_DSS_DESTINATION_LCD, NULL, OMAP_DSS_ROTATION_0, FALSE, 0, 0 },    // VIDEO1
    { FALSE, OMAP_DSS_DESTINATION_LCD, NULL, OMAP_DSS_ROTATION_0, FALSE, 0, 0 },    // VIDEO2
};

OMAPPipelineScaling g_rgPipelineScaling[NUM_DSS_PIPELINES] =
{
    { 1, 1, 0 },    // GFX
    { 1, 1, 0 },    // VIDEO1
    { 1, 1, 0 },    // VIDEO2
};


DWORD   g_dwDestinationRefCnt[NUM_DSS_DESTINATION] =
{
    0,      // LCD
    0       // TV Out
};

//------------------------------------------------------------------------------
OMAPDisplayController::OMAPDisplayController()
{
    m_pDSSRegs = NULL;
    m_pDispRegs = NULL;
    m_pVencRegs = NULL;

    m_dwPowerLevel = D4;

    m_bTVEnable = FALSE;
    m_bHDMIEnable = FALSE;

#if 0
    if (LcdPdd_DVI_Enabled())
        m_bDVIEnable = TRUE;
    else
#endif
        m_bDVIEnable = FALSE;

    m_bGammaEnable = TRUE;
    m_dwEnableWaitForVerticalBlank = FALSE;

    m_dwContrastLevel = DEFAULT_CONTRAST_LEVEL;
    m_pGammaBufVirt = NULL;

    m_bDssIntThreadExit = FALSE;
    m_hDssIntEvent = NULL;
    m_hDssIntThread = NULL;
    m_dwDssSysIntr = 0;

    m_dwVsyncPeriod = 0;
    m_hVsyncEvent = NULL;
    m_hVsyncEventSGX = NULL;

    m_hScanLineEvent = NULL;

#if 0
    m_pColorSpaceCoeffs = g_dwColorSpaceCoeff_BT601_Limited;
#endif

    m_bLPREnable = FALSE;

    m_hSmartReflexPolicyAdapter = NULL;

    m_eLcdPixelFormat = OMAP_DSS_PIXELFORMAT_ARGB32;
    m_dwLcdWidth = LCD_WIDTH;
    m_dwLcdHeight = LCD_HEIGHT;
}

//------------------------------------------------------------------------------
OMAPDisplayController::~OMAPDisplayController()
{
    /* Close GRPX and DISP drivers */
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::InitController(BOOL bEnableGammaCorr, BOOL bEnableWaitForVerticalBlank)
{
    BOOL    bResult = FALSE;

    // Disable gamma correction based on registry
    if(!bEnableGammaCorr)
        m_bGammaEnable = FALSE;

    // Enable VSYNC code based on registry
    if (bEnableWaitForVerticalBlank)
        m_dwEnableWaitForVerticalBlank = TRUE;

    //  Allocate physical memory for gamma table buffer
    m_pGammaBufVirt = (DWORD*)AllocPhysMem(NUM_GAMMA_VALS*sizeof(DWORD), PAGE_READWRITE | PAGE_NOCACHE, 0, 0,&m_dwGammaBufPhys);
    if( m_pGammaBufVirt == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: COMAPDisplayController::InitController: "
            L"Failed allocate Gamma phys buffer\r\n"
            ));
        goto cleanUp;
        }

    //  Initialize power lock critical section
    InitializeCriticalSection( &m_csPowerLock );

    //  Lock access to power level
    EnterCriticalSection( &m_csPowerLock );

    //  Reset the DSS controller
    ResetDSS();

    /* Download the vpss m3 download as early as possible */
    vpssm3_download();

    //  Unlock access to power level
    LeaveCriticalSection( &m_csPowerLock );

    //  Success
    bResult = TRUE;

cleanUp:
    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::InitLCD()
{
    /* Add calls to initialize HDMI, DVO1, DVO2 etc */
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::InitInterrupts(DWORD irq, DWORD istPriority)
{
    BOOL rc = FALSE;

    //Create specific interrupt events
    m_hVsyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hVsyncEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%S: ERROR: Failed to create Vsync interrupt event object!\r\n"), __FUNCTION__));
        goto cleanUp;
    }

    //Create specific interrupt events
    m_hVsyncEventSGX = CreateEvent(NULL, FALSE, FALSE, VSYNC_EVENT_NAME);
    if (m_hVsyncEventSGX == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%S: ERROR: Failed to create Vsync interrupt event object for SGX!\r\n"), __FUNCTION__));
        goto cleanUp;
    }

    //Create specific interrupt events
    m_hScanLineEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hScanLineEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%S: ERROR: Failed to create ScanLine interrupt event object!\r\n"), __FUNCTION__));
        goto cleanUp;
    }

    // spawn thread
    m_hDssIntThread = CreateThread(NULL, 0, DssInterruptHandler, this, 0, NULL);
    if (!m_hDssIntThread)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%S: Failed to create interrupt thread\r\n"), __FUNCTION__));
        goto cleanUp;
    }

    // set thread priority
    CeSetThreadPriority(m_hDssIntThread, istPriority);

    rc = TRUE;

cleanUp:
    if (rc == FALSE) UninitInterrupts();
    return rc;
}

//------------------------------------------------------------------------------
void
OMAPDisplayController::UninitInterrupts()
{
    // stop thread
    if (m_hDssIntEvent != NULL)
    {
        if (m_hDssIntThread != NULL)
        {
            // Signal stop to thread
            m_bDssIntThreadExit = TRUE;

            // Set event to wake it
            SetEvent(m_hDssIntEvent);

            // Wait until thread exits
            WaitForSingleObject(m_hDssIntThread, INFINITE);

            // Close handle
            CloseHandle(m_hDssIntThread);

            // reinit
            m_hDssIntThread = NULL;
        }

        // close event handle
        CloseHandle(m_hDssIntEvent);
        m_hDssIntEvent = NULL;
    }

    if(m_hVsyncEvent != NULL)
    {
        CloseHandle(m_hVsyncEvent);
        m_hVsyncEvent = NULL;
    }

    if(m_hScanLineEvent != NULL)
    {
        CloseHandle(m_hScanLineEvent);
        m_hScanLineEvent = NULL;
    }
}

//------------------------------------------------------------------------------
DWORD
OMAPDisplayController::DssInterruptHandler( void* pData )
{
    return 1;
}
//------------------------------------------------------------------------------
VOID
OMAPDisplayController::DssProcessInterrupt()
{
    // Access the regs
    if( AccessRegs() == FALSE )
    {
        // failure will cause lockup because the interrupt will still be pending...
        DEBUGMSG(ZONE_ERROR,(L"AccessRegs failed in DssProcessInterrupt\r\n"));
        ASSERT(0);
        goto cleanUp;
    }

cleanUp:
    ReleaseRegs();
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::SetSurfaceMgr(
    OMAPSurfaceManager *pSurfMgr
    )
{
    //  Reference the given surface mamager
    m_pSurfaceMgr = pSurfMgr;

    //  Return result
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::SetPipelineAttribs(
    OMAP_DSS_PIPELINE       ePipeline,
    OMAP_DSS_DESTINATION    eDestination,
    OMAPSurface*            pSurface,
    DWORD                   dwPosX,
    DWORD                   dwPosY
    )
{
    BOOL    bResult = FALSE;
    OMAP_DSS_ROTATION   eRotation;
    BOOL                bMirror;
//    DWORD               dwVidRotation = 0;
//    DWORD               dwX, dwY;

    //  Get rotation and mirror settings for pipeline output
    eRotation = g_rgPipelineMapping[ePipeline].eRotation;
    bMirror   = g_rgPipelineMapping[ePipeline].bMirror;

    //  Configure the attributes of the selected pipeline

    //  GFX pipeline
    if( ePipeline == OMAP_DSS_PIPELINE_GFX )
    {
#if 0
        DISPC_GFX_ATTR_GFXFORMAT(pSurface->PixelFormat())
        DISPC_GFX_SIZE_GFXSIZEX(pSurface->Width(eRotation)) |
        DISPC_GFX_SIZE_GFXSIZEY(pSurface->Height(eRotation))
        DISPC_GFX_POS_GFXPOSX(dwX) |
        DISPC_GFX_POS_GFXPOSY(dwY)
        pSurface->PixelIncr(eRotation, bMirror) );
        pSurface->RowIncr(eRotation, bMirror) );

        pSurface->PhysicalAddr(eRotation, bMirror) );
        pSurface->PhysicalAddr(eRotation, bMirror) );
#else
        /* Initialize GRPX driver over FVID2 interface */
//        grpx_display_test();  // display test
//        grpx_flipbuf_test();  // Flip buffer test 
        grpx_setup();
#endif
    }

    //  Set mapping of pipeline to destination and surface
    g_rgPipelineMapping[ePipeline].eDestination = eDestination;
    g_rgPipelineMapping[ePipeline].pSurface     = pSurface;
    g_rgPipelineMapping[ePipeline].dwDestWidth  = pSurface->Width(eRotation);
    g_rgPipelineMapping[ePipeline].dwDestHeight = pSurface->Height(eRotation);

    //  Reset the scaling factors to 100% and no interlacing
    g_rgPipelineScaling[ePipeline].dwHorzScaling = 1;
    g_rgPipelineScaling[ePipeline].dwVertScaling = 1;
    g_rgPipelineScaling[ePipeline].dwInterlace   = 0;

    //  Result
    bResult = TRUE;

//cleanUp:
    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::SetScalingAttribs(
    OMAP_DSS_PIPELINE       ePipeline,
    DWORD                   dwWidth,
    DWORD                   dwHeight,
    DWORD                   dwPosX,
    DWORD                   dwPosY
    )
{
    RECT    srcRect,
            destRect;

    //  Check for surface
    if( g_rgPipelineMapping[ePipeline].pSurface == NULL )
        return FALSE;

    //  Create src and dest RECTs for this scaling setup
    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = g_rgPipelineMapping[ePipeline].pSurface->Width();
    srcRect.bottom = g_rgPipelineMapping[ePipeline].pSurface->Height();

    destRect.left = dwPosX;
    destRect.top = dwPosY;
    destRect.right = dwPosX + dwWidth;
    destRect.bottom = dwPosY + dwHeight;

    //  Call the RECT based scaling method
    return SetScalingAttribs( ePipeline, &srcRect, &destRect );
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::SetScalingAttribs(
    OMAP_DSS_PIPELINE       ePipeline,
    RECT*                   pSrcRect,
    RECT*                   pDestRect
    )
{
    BOOL                bResult = FALSE;
    OMAPSurface*        pSurface;
    DWORD               paddr;

    pSurface  = g_rgPipelineMapping[ePipeline].pSurface;
    paddr = pSurface->PhysicalAddr(OMAP_DSS_ROTATION_0, 0);
    grpx_setbuf(paddr);

#if 0
    OMAP_DSS_ROTATION   eRotation;
    BOOL                bMirror;
    DWORD   dwSrcWidth,
            dwSrcHeight,
            dwDestWidth,
            dwDestHeight;
    DWORD   dwHorzScale,
            dwVertScale;
    DWORD   dwX, dwY;
    DWORD   dwCurrAttribs;
    DWORD   dwScaleEnable = 0;
    DWORD               dwAccum0 = 0;
    DWORD               dwAccum1 = 0;
    DWORD               dwHorzDecimation = 1;
    DWORD               dwVertDecimation = 1;
    DWORD               dwInterlace = 0;
    DWORD*  pHorizCoeffs = NULL;
    DWORD*  pVertCoeffs = NULL;
    DWORD               dwPixelsPerLine;
    DWORD   i;
    BOOL                bYUVRotated = FALSE;


    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;


    //  Get rotation and mirror settings for pipeline output
    pSurface  = g_rgPipelineMapping[ePipeline].pSurface;
    eRotation = g_rgPipelineMapping[ePipeline].eRotation;
    bMirror   = g_rgPipelineMapping[ePipeline].bMirror;


    //  Compute horizontal and vertical scaling factors
    dwSrcWidth  = pSrcRect->right - pSrcRect->left;
    dwSrcHeight = pSrcRect->bottom - pSrcRect->top;

    dwDestWidth  = pDestRect->right - pDestRect->left;
    dwDestHeight = pDestRect->bottom - pDestRect->top;


    //  Check for odd destination RECT points
    if( (pDestRect->left % 2) == 1 )
    {
        pDestRect->left += 1;
        dwDestWidth -= 1;
    }

    if( (pDestRect->top % 2) == 1 )
    {
        pDestRect->top += 1;
        dwDestHeight -= 1;
    }

    //  Check for odd destination RECT values
    if( (dwDestWidth % 2) == 1 )
    {
        pDestRect->right -= 1;
        dwDestWidth -= 1;
    }

    if( (dwDestHeight % 2) == 1 )
    {
        pDestRect->bottom -= 1;
        dwDestHeight -= 1;
    }

    //  Compute scaling factors
    dwHorzScale = 1024 * dwSrcWidth / dwDestWidth;
    dwVertScale = 1024 * dwSrcHeight / dwDestHeight;

    if ((eRotation == OMAP_DSS_ROTATION_0)||(eRotation == OMAP_DSS_ROTATION_180))
        dwPixelsPerLine = GetLCDWidth();
    else
        dwPixelsPerLine = GetLCDHeight();

    // Limit the scaling to 1/3rd of the original size
    if ((dwHorzScale > 3072) && (dwSrcWidth > dwPixelsPerLine))
    {
        // Max scale factor feasible is 1/3rd
        // Based on 1/3rd scale factor, increase the dest rect size
        DWORD dstWidthOffset = 0;
        DWORD newDstWidth    = 0;
        newDstWidth    = dwSrcWidth/3;

        // Make sure to set the dst width >= 1/3 x srcwidth
        if (dwSrcWidth%3 != 0)
            newDstWidth += 1;

        // Check the size of the new dst width calculated.
        newDstWidth    = (newDstWidth > GetLCDWidth())? GetLCDWidth() : newDstWidth ;

        dstWidthOffset = (newDstWidth-dwDestWidth)/2;

        // adjust the Dest rect based on the new scale factor
        if ((DWORD)pDestRect->left > dstWidthOffset)
            pDestRect->left  -= dstWidthOffset;

        pDestRect->right += dstWidthOffset;
    }
    else
    {
        //  If playback is rotated and scaled and color converted, adjust clipping to avoid sync lost
        if( (eRotation == OMAP_DSS_ROTATION_90)||(eRotation == OMAP_DSS_ROTATION_270) )
        {
            if( (pSurface->PixelFormat() == OMAP_DSS_PIXELFORMAT_YUV2) ||
                (pSurface->PixelFormat() == OMAP_DSS_PIXELFORMAT_UYVY) )
            {
                //  Flag special case of YUV rotated
                bYUVRotated = TRUE;

                //  Rotated scaling with color conversion in these bounds has issues
                if( dwHorzScale > 1536 && dwHorzScale < 2048 )
                {
                    DWORD   dwOldSrcWidth = dwSrcWidth;

                    //  Clip src width
                    dwSrcWidth = dwSrcWidth * 1536 / dwHorzScale;
                    dwSrcWidth = ((dwSrcWidth % 2) == 0) ? dwSrcWidth : dwSrcWidth - 1;
                    pSrcRect->left  = pSrcRect->left + (dwOldSrcWidth - dwSrcWidth)/2;
                    pSrcRect->right = pSrcRect->left + dwSrcWidth;
                }

                //  Rotated scaling with color conversion in these bounds has issues
                if( dwVertScale > 1536 && dwVertScale < 2048 )
                {
                    DWORD   dwOldSrcHeight = dwSrcHeight;

                    //  Clip src height
                    dwSrcHeight = dwSrcHeight * 1536 / dwVertScale;
                    dwSrcHeight = ((dwSrcHeight % 2) == 0) ? dwSrcHeight : dwSrcHeight - 1;
                    pSrcRect->top    = pSrcRect->top + (dwOldSrcHeight - dwSrcHeight)/2;
                    pSrcRect->bottom = pSrcRect->top + dwSrcHeight;
                }
            }
        }
    }

    //  Set the clipping region for the surface
    g_rgPipelineMapping[ePipeline].pSurface->SetClipping( pSrcRect );

    //  Compute src and dest width/height
    dwSrcWidth  = pSrcRect->right - pSrcRect->left;
    dwSrcHeight = pSrcRect->bottom - pSrcRect->top;

    dwDestWidth  = pDestRect->right - pDestRect->left;
    dwDestHeight = pDestRect->bottom - pDestRect->top;

    //  Swap src width/height based on pipeline rotation angle
    switch( g_rgPipelineMapping[ePipeline].eRotation )
    {
        case OMAP_DSS_ROTATION_90:
            //  Settings for rotation angle 90
            i = dwSrcWidth;
            dwSrcWidth = dwSrcHeight;
            dwSrcHeight = i;
            break;

        case OMAP_DSS_ROTATION_270:
            //  Settings for rotation angle 270
            i = dwSrcWidth;
            dwSrcWidth = dwSrcHeight;
            dwSrcHeight = i;
            break;
    }

    //  Default origin
    dwX = pDestRect->left;
    dwY = pDestRect->top;


    //------------------------------------------------------------------------------
    //  Configure the scaling of the pipeline for LCD display
    //
    if( g_rgPipelineMapping[ePipeline].eDestination == OMAP_DSS_DESTINATION_LCD )
    {
        //  Compute new origin and swap destination width/height based on GFX pipeline rotation angle
        switch( g_rgPipelineMapping[OMAP_DSS_PIPELINE_GFX].eRotation )
        {
            case OMAP_DSS_ROTATION_0:
                //  Settings for rotation angle 0
                dwX = pDestRect->left;
                dwY = pDestRect->top;
                break;

            case OMAP_DSS_ROTATION_90:
                //  Settings for rotation angle 90
                dwX = pDestRect->top;
                dwY = GetLCDHeight() - dwDestWidth - pDestRect->left;

                i = dwDestWidth;
                dwDestWidth = dwDestHeight;
                dwDestHeight = i;
                break;

            case OMAP_DSS_ROTATION_180:
                //  Settings for rotation angle 180
                dwX = GetLCDWidth() - dwDestWidth - pDestRect->left;
                dwY = GetLCDHeight() - dwDestHeight - pDestRect->top;
                break;

            case OMAP_DSS_ROTATION_270:
                //  Settings for rotation angle 270
                dwX = GetLCDWidth() - dwDestHeight - pDestRect->top;
                dwY = pDestRect->left;

                i = dwDestWidth;
                dwDestWidth = dwDestHeight;
                dwDestHeight = i;
                break;
        }


        //  Compute horizontal and vertical scaling factors
        dwHorzScale = 1024 * dwSrcWidth / dwDestWidth;
        dwVertScale = 1024 * dwSrcHeight / dwDestHeight;

        //  Adjust the HW scaling factors by the decimation factor
        dwHorzScale = 1024 * (dwSrcWidth/dwHorzDecimation) / dwDestWidth;
        dwVertScale = 1024 * (dwSrcHeight/dwVertDecimation) / dwDestHeight;

        //  Set the decimation factors for the surface
        switch( eRotation )
        {
            case OMAP_DSS_ROTATION_0:
            case OMAP_DSS_ROTATION_180:
                //  Standard orientation
                pSurface->SetHorizontalScaling( dwHorzDecimation );
                pSurface->SetVerticalScaling( dwVertDecimation );
                break;

            case OMAP_DSS_ROTATION_90:
            case OMAP_DSS_ROTATION_270:
                //  Rotated orientation
                pSurface->SetHorizontalScaling( dwVertDecimation );
                pSurface->SetVerticalScaling( dwHorzDecimation );
                break;
        }

        //  Adjust the source width and height by the decimation factor
        dwSrcWidth  = dwSrcWidth / dwHorzDecimation;
        dwSrcHeight = dwSrcHeight / dwVertDecimation;

        //  If YUV rotated, check for any odd width or height values due to decimation
        if( bYUVRotated )
        {
            if( (dwSrcWidth % 2) == 1 )
            {
                dwSrcWidth -= 1;

                //  Adjust clipping; note rotation means adjust clipping height here
                pSrcRect->bottom -= dwHorzDecimation;
            }

            if( (dwSrcHeight % 2) == 1 )
            {
                dwSrcHeight -= 1;

                //  Adjust clipping; note rotation means adjust clipping width here
                pSrcRect->right -= dwVertDecimation;
            }

            //  Set the clipping region for the surface
            g_rgPipelineMapping[ePipeline].pSurface->SetClipping( pSrcRect );

            //  Recalculate the scale factors to account for adjustments made
            dwHorzScale = 1024 * dwSrcWidth / dwDestWidth;
            dwVertScale = 1024 * dwSrcHeight / dwDestHeight;
        }

        //  Based on scaling factor, determine which coeffs to use and if to enable scaling
        dwScaleEnable |= (dwHorzScale == 1024) ? 0 : DISPC_VID_ATTR_VIDRESIZE_HORIZONTAL;
        dwScaleEnable |= (dwVertScale == 1024) ? 0 : DISPC_VID_ATTR_VIDRESIZE_VERTICAL;

        //  Horizontal scaling
        if( dwHorzScale > 1024 )
        {
        }

        //  Vertical scaling
        if( dwVertScale > 2048 )
        {
        }

        // For Portrait mode, the Vertical down scale coeff has to be 5 tap to
        // prevent SYNCLOST. so ignore the scale factor and force the 5-tap.
        if ((( eRotation == OMAP_DSS_ROTATION_90 )||
            ( eRotation == OMAP_DSS_ROTATION_270 )) &&
            ( pVertCoeffs == g_dwScalingCoeff_Vert_Down_3_Taps ))
        {
            //clear existing scale option
            dwScaleEnable &= ~DISPC_VID_ATTR_VIDVERTICALTAPS_3;
            dwScaleEnable |= DISPC_VID_ATTR_VIDVERTICALTAPS_5;
            dwScaleEnable |= DISPC_VID_ATTR_VIDLINEBUFFERSPLIT;
            pVertCoeffs = g_dwScalingCoeff_Vert_Down_5_Taps;
        }
    }

    //------------------------------------------------------------------------------
    //  Configure the scaling of the pipeline for TV display
    //
    else if( g_rgPipelineMapping[ePipeline].eDestination == OMAP_DSS_DESTINATION_TVOUT )
    {
    }
    else
    {
        ASSERT(0);
        goto cleanUp;
    }

    DEBUGMSG(ZONE_WARNING, (L"INFO: OMAPDisplayController::SetScalingAttribs: "));
    DEBUGMSG(ZONE_WARNING, (L"  Src  RECT (%d,%d) (%d,%d)\r\n", pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom));
    DEBUGMSG(ZONE_WARNING, (L"  Dest RECT (%d,%d) (%d,%d)\r\n", pDestRect->left, pDestRect->top, pDestRect->right, pDestRect->bottom));
    DEBUGMSG(ZONE_WARNING, (L"  Computed Origin (%d,%d) for rotation angle %d\r\n", dwX, dwY, g_rgPipelineMapping[ePipeline].eRotation));
    DEBUGMSG(ZONE_WARNING, (L"  dwScaleEnable = 0x%08X  dwHorzScale = %d  dwVertScale = %d\r\n", dwScaleEnable, dwHorzScale, dwVertScale));
    DEBUGMSG(ZONE_WARNING, (L"  dwSrcWidth  = %d  dwSrcHeight  = %d\r\n", dwSrcWidth, dwSrcHeight));
    DEBUGMSG(ZONE_WARNING, (L"  dwDestWidth = %d  dwDestHeight = %d\r\n", dwDestWidth, dwDestHeight));
    DEBUGMSG(ZONE_WARNING, (L"  dwHorzDecimation = %d  dwVertDecimation = %d\r\n", dwHorzDecimation, dwVertDecimation));

    //  GFX pipeline
    if( ePipeline == OMAP_DSS_PIPELINE_GFX )
    {
        //  Scaling is not supported on the GFX plane
    }

    //  VIDEO1 pipeline
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO1 )
    {
        //  Get the current attribute settings
        dwCurrAttribs = INREG32( &m_pDispRegs->tDISPC_VID1.ATTRIBUTES );

        //  Mask off the scaling bits
        dwCurrAttribs &= ~(DISPC_VID_ATTR_VIDRESIZE_MASK|DISPC_VID_ATTR_VIDLINEBUFFERSPLIT|DISPC_VID_ATTR_VIDVERTICALTAPS_5);

        //  Enable video resizing by or'ing with scale enable attribs
        OUTREG32( &m_pDispRegs->tDISPC_VID1.ATTRIBUTES,
                    dwCurrAttribs | dwScaleEnable
                    );


        //  Size of resized output window and original picture size
        OUTREG32( &m_pDispRegs->tDISPC_VID1.SIZE,
                    DISPC_VID_SIZE_VIDSIZEX(dwDestWidth) |
                    DISPC_VID_SIZE_VIDSIZEY(dwDestHeight)
                    );

        OUTREG32( &m_pDispRegs->tDISPC_VID1.PICTURE_SIZE,
                    DISPC_VID_PICTURE_SIZE_VIDORGSIZEX(dwSrcWidth) |
                    DISPC_VID_PICTURE_SIZE_VIDORGSIZEY(dwSrcHeight)
                    );

        //  Position of window
        OUTREG32( &m_pDispRegs->tDISPC_VID1.POSITION,
                    DISPC_VID_POS_VIDPOSX(dwX) |
                    DISPC_VID_POS_VIDPOSY(dwY)
                    );

        //  DMA properties
        OUTREG32( &m_pDispRegs->tDISPC_VID1.PIXEL_INC, pSurface->PixelIncr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID1.ROW_INC, pSurface->RowIncr(eRotation, bMirror) );

        OUTREG32( &m_pDispRegs->tDISPC_VID1.BA0, pSurface->PhysicalAddr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID1.BA1, pSurface->PhysicalAddr(eRotation, bMirror) + dwInterlace );


        //  Initialize FIR accumulators
        OUTREG32( &m_pDispRegs->tDISPC_VID1.ACCU0,
                    dwAccum0
                    );

        OUTREG32( &m_pDispRegs->tDISPC_VID1.ACCU1,
                    dwAccum1
                    );

        //  Set FIR increment value and coeffs
        OUTREG32( &m_pDispRegs->tDISPC_VID1.FIR,
                    DISPC_VID_FIR_VIDFIRHINC(dwHorzScale) |
                    DISPC_VID_FIR_VIDFIRVINC(dwVertScale)
                    );

        for( i = 0; i < NUM_SCALING_PHASES; i++ )
        {
            //  OR the horiz and vert coeff values b/c some registers span both H and V coeffs
            OUTREG32( &m_pDispRegs->tDISPC_VID1.aFIR_COEF[i].ulH,
                        *pHorizCoeffs++ | *pVertCoeffs++
                        );

            OUTREG32( &m_pDispRegs->tDISPC_VID1.aFIR_COEF[i].ulHV,
                        *pHorizCoeffs++ | *pVertCoeffs++
                        );

            OUTREG32( &m_pDispRegs->DISPC_VID1_FIR_COEF_V[i],
                        *pHorizCoeffs++ | *pVertCoeffs++
                        );
        }

        Dump_DISPC_VID( &m_pDispRegs->tDISPC_VID1, (UINT32*) &m_pDispRegs->DISPC_VID1_FIR_COEF_V[0], 1 );
    }


    //  VIDEO2 pipeline
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO2 )
    {
        //  Get the current attribute settings
        dwCurrAttribs = INREG32( &m_pDispRegs->tDISPC_VID2.ATTRIBUTES );

        //  Mask off the scaling bits
        dwCurrAttribs &= ~(DISPC_VID_ATTR_VIDRESIZE_MASK|DISPC_VID_ATTR_VIDLINEBUFFERSPLIT|DISPC_VID_ATTR_VIDVERTICALTAPS_5);

        //  Enable video resizing by or'ing with scale enable attribs
        OUTREG32( &m_pDispRegs->tDISPC_VID2.ATTRIBUTES,
                    dwCurrAttribs | dwScaleEnable
                    );


        //  Size of resized output window; picture size was set by in SetPipelineAttribs()
        OUTREG32( &m_pDispRegs->tDISPC_VID2.SIZE,
                    DISPC_VID_SIZE_VIDSIZEX(dwDestWidth) |
                    DISPC_VID_SIZE_VIDSIZEY(dwDestHeight)
                    );

        OUTREG32( &m_pDispRegs->tDISPC_VID2.PICTURE_SIZE,
                    DISPC_VID_PICTURE_SIZE_VIDORGSIZEX(dwSrcWidth) |
                    DISPC_VID_PICTURE_SIZE_VIDORGSIZEY(dwSrcHeight)
                    );

        //  Position of window
        OUTREG32( &m_pDispRegs->tDISPC_VID2.POSITION,
                    DISPC_VID_POS_VIDPOSX(dwX) |
                    DISPC_VID_POS_VIDPOSY(dwY)
                    );

        //  DMA properties
        OUTREG32( &m_pDispRegs->tDISPC_VID2.PIXEL_INC, pSurface->PixelIncr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID2.ROW_INC, pSurface->RowIncr(eRotation, bMirror) );

        OUTREG32( &m_pDispRegs->tDISPC_VID2.BA0, pSurface->PhysicalAddr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID2.BA1, pSurface->PhysicalAddr(eRotation, bMirror) + dwInterlace );


        //  Initialize FIR accumulators
        OUTREG32( &m_pDispRegs->tDISPC_VID2.ACCU0,
                    dwAccum0
                    );

        OUTREG32( &m_pDispRegs->tDISPC_VID2.ACCU1,
                    dwAccum1
                    );

        //  Set FIR increment value and coeffs
        OUTREG32( &m_pDispRegs->tDISPC_VID2.FIR,
                    DISPC_VID_FIR_VIDFIRHINC(dwHorzScale) |
                    DISPC_VID_FIR_VIDFIRVINC(dwVertScale)
                    );

        for( i = 0; i < NUM_SCALING_PHASES; i++ )
        {
            //  OR the horiz and vert coeff values b/c some registers span both H and V coeffs
            OUTREG32( &m_pDispRegs->tDISPC_VID2.aFIR_COEF[i].ulH,
                        *pHorizCoeffs++ | *pVertCoeffs++
                        );

            OUTREG32( &m_pDispRegs->tDISPC_VID2.aFIR_COEF[i].ulHV,
                        *pHorizCoeffs++ | *pVertCoeffs++
                        );

            OUTREG32( &m_pDispRegs->DISPC_VID2_FIR_COEF_V[i],
                        *pHorizCoeffs++ | *pVertCoeffs++
                        );
        }

        Dump_DISPC_VID( &m_pDispRegs->tDISPC_VID2, (UINT32*) &m_pDispRegs->DISPC_VID2_FIR_COEF_V[0], 2 );
    }


    //  Update output width and height
    g_rgPipelineMapping[ePipeline].dwDestWidth  = dwDestWidth;
    g_rgPipelineMapping[ePipeline].dwDestHeight = dwDestHeight;

    //  Cache the decimation factors applied to the source surface
    g_rgPipelineScaling[ePipeline].dwHorzScaling = dwHorzDecimation;
    g_rgPipelineScaling[ePipeline].dwVertScaling = dwVertDecimation;
    g_rgPipelineScaling[ePipeline].dwInterlace   = dwInterlace;

    //  Set the decimation factors for the surface back to normal
    pSurface->SetHorizontalScaling( 1 );
    pSurface->SetVerticalScaling( 1 );


    //  Result
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::EnablePipeline(
    OMAP_DSS_PIPELINE       ePipeline
    )
{
    BOOL                    bResult = FALSE;

#if 1
    OMAPSurface*        pSurface;
    DWORD               paddr;

    pSurface  = g_rgPipelineMapping[ePipeline].pSurface;
    paddr = pSurface->PhysicalAddr(OMAP_DSS_ROTATION_0, 0);
    grpx_setbuf(paddr);

    grpx_start();
    bResult = TRUE;
#else
    OMAP_DSS_DESTINATION    eDest;
    DWORD                   dwNumPipelinesOn = 0;
    DWORD                   dwDestEnable,
                            dwDestGo;

    //  Check if pipeline is already enabled
    if( g_rgPipelineMapping[ePipeline].bEnabled == TRUE )
        return TRUE;

    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;


    //  Enable GFX pipeline
    if( ePipeline == OMAP_DSS_PIPELINE_GFX )
    {
        //  Enable the interrupt for reporting the GFX under flow error
        SETREG32( &m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_GFXFIFOUNDERFLOW);

        //  Enable the pipeline
        SETREG32( &m_pDispRegs->DISPC_GFX_ATTRIBUTES, DISPC_GFX_ATTR_GFXENABLE );
        g_rgPipelineMapping[ePipeline].bEnabled = TRUE;
    }

    //  Enable VID1 pipeline
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO1 )
    {
        // Enable the interrupt for reporting the VID1 under flow error
        SETREG32( &m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_VID1FIFOUNDERFLOW);

        //  Enable the pipeline
        SETREG32( &m_pDispRegs->tDISPC_VID1.ATTRIBUTES, DISPC_VID_ATTR_VIDENABLE );
        g_rgPipelineMapping[ePipeline].bEnabled = TRUE;
    }

    //  Enable VID2 pipeline
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO2 )
    {
        // Enable the interrupt for reporting the VID2 under flow error
        SETREG32( &m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_VID2FIFOUNDERFLOW);

        //  Enable the pipeline
        SETREG32( &m_pDispRegs->tDISPC_VID2.ATTRIBUTES, DISPC_VID_ATTR_VIDENABLE );
        g_rgPipelineMapping[ePipeline].bEnabled = TRUE;
    }


    //  Count the number of pipelines that will be on
    dwNumPipelinesOn += (g_rgPipelineMapping[OMAP_DSS_PIPELINE_GFX].bEnabled) ? 1 : 0;
    dwNumPipelinesOn += (g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO1].bEnabled) ? 1 : 0;
    dwNumPipelinesOn += (g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO2].bEnabled) ? 1 : 0;


    //  If there is only one pipeline enabled, use FIFO merge to make 1 large FIFO
    //  for better power management
    if( dwNumPipelinesOn == 1 )
    {
        //  Enable FIFO merge
        SETREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_FIFOMERGE );

        //  Adjust the FIFO high and low thresholds for all the enabled pipelines
        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_GFX].bEnabled )
        {
            OUTREG32( &m_pDispRegs->DISPC_GFX_FIFO_THRESHOLD,
                        DISPC_GFX_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_MERGED(FIFO_BURSTSIZE_16x32)) |
                        DISPC_GFX_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_MERGED)
                        );
        }

        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO1].bEnabled )
        {
            OUTREG32( &m_pDispRegs->tDISPC_VID1.FIFO_THRESHOLD,
                        DISPC_VID_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_MERGED(FIFO_BURSTSIZE_16x32)) |
                        DISPC_VID_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_MERGED)
                        );
        }

        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO2].bEnabled )
        {
            OUTREG32( &m_pDispRegs->tDISPC_VID2.FIFO_THRESHOLD,
                        DISPC_VID_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_MERGED(FIFO_BURSTSIZE_16x32)) |
                        DISPC_VID_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_MERGED)
                        );
        }
    }
    else
    {
        //  Disable FIFO merge
        CLRREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_FIFOMERGE );

        //  Adjust the FIFO high and low thresholds for all the enabled pipelines
        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_GFX].bEnabled )
        {
            OUTREG32( &m_pDispRegs->DISPC_GFX_FIFO_THRESHOLD,
                        DISPC_GFX_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_NORMAL(FIFO_BURSTSIZE_16x32)) |
                        DISPC_GFX_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_NORMAL)
                        );
        }

        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO1].bEnabled )
        {
            OUTREG32( &m_pDispRegs->tDISPC_VID1.FIFO_THRESHOLD,
                        DISPC_VID_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_NORMAL(FIFO_BURSTSIZE_16x32)) |
                        DISPC_VID_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_NORMAL)
                        );
        }

        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO2].bEnabled )
        {
            OUTREG32( &m_pDispRegs->tDISPC_VID2.FIFO_THRESHOLD,
                        DISPC_VID_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_NORMAL(FIFO_BURSTSIZE_16x32)) |
                        DISPC_VID_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_NORMAL)
                        );
        }
    }


    //  Get the destination for the pipeline
    eDest = g_rgPipelineMapping[ePipeline].eDestination;
    switch( eDest )
    {
        case OMAP_DSS_DESTINATION_LCD:
            //  Set enable and go bits for LCD
            dwDestEnable = DISPC_CONTROL_LCDENABLE;
            dwDestGo     = DISPC_CONTROL_GOLCD;
            break;

        case OMAP_DSS_DESTINATION_TVOUT:
            //  Set enable and go bits for TV Out
            dwDestEnable = (m_bTVEnable) ? DISPC_CONTROL_DIGITALENABLE : 0;
            dwDestGo     = DISPC_CONTROL_GODIGITAL;
            break;

        default:
            ASSERT(0);
            goto cleanUp;
    }

    //  Try enabling overlay optimization
    EnableOverlayOptimization( TRUE );

    //  Flush the shadow registers
    FlushRegs( dwDestGo );


    //  If the destination for pipeline is not enabled, enable it
    if( g_dwDestinationRefCnt[eDest]++ == 0 )
    {
        if (eDest == OMAP_DSS_DESTINATION_LCD)
        {
            SETREG32( &m_pDispRegs->DISPC_CONTROL, dwDestEnable );
        }
        else
        {
            // For TVOUT enable, the SYNCLOST_DIGITAL interrupt
            // has to be cleared at the 1st EVSYNC after DIGITALENABLE

            DWORD irqStatus, irqEnable;

            irqEnable = INREG32( &m_pDispRegs->DISPC_IRQENABLE );
            // Disable all the DSS interrupts
            OUTREG32( &m_pDispRegs->DISPC_IRQENABLE , 0 );
            // Clear the Existing IRQ status
            OUTREG32( &m_pDispRegs->DISPC_IRQSTATUS, 0xFFFFFFFF );

            // Enable the DIGITAL Path
            SETREG32( &m_pDispRegs->DISPC_CONTROL, DISPC_CONTROL_DIGITALENABLE );

            // Wait for E-VSYNC
            WaitForIRQ( DISPC_IRQSTATUS_EVSYNC_EVEN|DISPC_IRQSTATUS_EVSYNC_ODD );

            // Clear the pending interrupt status
            irqStatus = INREG32( &m_pDispRegs->DISPC_IRQSTATUS );
            OUTREG32( &m_pDispRegs->DISPC_IRQSTATUS,  irqStatus );

            // Re-enable the DSS interrupts
            OUTREG32( &m_pDispRegs->DISPC_IRQENABLE , irqEnable );
        }
    }

    // Configure the LPR mode based on active Pipeline(s)
    BOOL bEnable = ((dwNumPipelinesOn == 1) &&
                   (g_rgPipelineMapping[OMAP_DSS_PIPELINE_GFX].bEnabled)) ?
                   TRUE : FALSE;
    EnableLPR( bEnable );

    //  Result
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::DisablePipeline(
    OMAP_DSS_PIPELINE       ePipeline
    )
{
    BOOL                    bResult = FALSE;

#if 0
//    DWORD                   dwTimeOut = 30;
    OMAP_DSS_DESTINATION    eDest;
    DWORD                   dwNumPipelinesOn = 0;
    DWORD                   dwIntrStatus;
    DWORD                   dwDestEnable,
                            dwDestGo;
    BOOL                    bLPRState = FALSE;

    //  Check if pipeline is already disabled
    if( g_rgPipelineMapping[ePipeline].bEnabled == FALSE )
        return TRUE;

    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;

     // Clear GO_XXX bit if it current enabled. The attributes register for
    // the pipeline is currently modified and so it is required to turn off
    // the GOLCD/GODIGITAL bit during the configuration.
    if ((INREG32( &m_pDispRegs->DISPC_CONTROL) & DISPC_CONTROL_GOLCD ) != 0)
        CLRREG32( &m_pDispRegs->DISPC_CONTROL, DISPC_CONTROL_GOLCD );

    if ((INREG32( &m_pDispRegs->DISPC_CONTROL) & DISPC_CONTROL_GODIGITAL ) != 0)
        CLRREG32( &m_pDispRegs->DISPC_CONTROL, DISPC_CONTROL_GODIGITAL );

    //  Disable GFX pipeline
    if( ePipeline == OMAP_DSS_PIPELINE_GFX )
    {
        //  Disable the pipeline
        CLRREG32( &m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_GFXFIFOUNDERFLOW);
        CLRREG32( &m_pDispRegs->DISPC_GFX_ATTRIBUTES, DISPC_GFX_ATTR_GFXENABLE );
        g_rgPipelineMapping[ePipeline].bEnabled = FALSE;
        g_rgPipelineMapping[ePipeline].bMirror = FALSE;
    }

    //  Disable VID1 pipeline
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO1 )
    {
        //  Disable the pipeline
        CLRREG32( &m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_VID1FIFOUNDERFLOW);
        CLRREG32( &m_pDispRegs->tDISPC_VID1.ATTRIBUTES, DISPC_VID_ATTR_VIDENABLE );
        g_rgPipelineMapping[ePipeline].bEnabled = FALSE;
        g_rgPipelineMapping[ePipeline].bMirror = FALSE;
    }

    //  Disable VID2 pipeline
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO2 )
    {
        //  Disable the pipeline
        CLRREG32( &m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_VID2FIFOUNDERFLOW);
        CLRREG32( &m_pDispRegs->tDISPC_VID2.ATTRIBUTES, DISPC_VID_ATTR_VIDENABLE );
        g_rgPipelineMapping[ePipeline].bEnabled = FALSE;
        g_rgPipelineMapping[ePipeline].bMirror = FALSE;
    }

    //  Count the number of pipelines that will be on
    dwNumPipelinesOn += (g_rgPipelineMapping[OMAP_DSS_PIPELINE_GFX].bEnabled) ? 1 : 0;
    dwNumPipelinesOn += (g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO1].bEnabled) ? 1 : 0;
    dwNumPipelinesOn += (g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO2].bEnabled) ? 1 : 0;


    //  If there is only one pipeline enabled, use FIFO merge to make 1 large FIFO
    //  for better power management
    if( dwNumPipelinesOn == 1 )
    {
        //  Enable FIFO merge
        SETREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_FIFOMERGE );

        //  Adjust the FIFO high and low thresholds for all the enabled pipelines
        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_GFX].bEnabled )
        {
            OUTREG32( &m_pDispRegs->DISPC_GFX_FIFO_THRESHOLD,
                        DISPC_GFX_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_MERGED(FIFO_BURSTSIZE_16x32)) |
                        DISPC_GFX_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_MERGED)
                        );
            bLPRState = TRUE;
        }

        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO1].bEnabled )
        {
            OUTREG32( &m_pDispRegs->tDISPC_VID1.FIFO_THRESHOLD,
                        DISPC_VID_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_MERGED(FIFO_BURSTSIZE_16x32)) |
                        DISPC_VID_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_MERGED)
                        );
        }

        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO2].bEnabled )
        {
            OUTREG32( &m_pDispRegs->tDISPC_VID2.FIFO_THRESHOLD,
                        DISPC_VID_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_MERGED(FIFO_BURSTSIZE_16x32)) |
                        DISPC_VID_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_MERGED)
                        );
        }


    }
    else
    {

        //  Disable FIFO merge
        CLRREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_FIFOMERGE );

        //  Adjust the FIFO high and low thresholds for all the enabled pipelines
        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_GFX].bEnabled )
        {
            OUTREG32( &m_pDispRegs->DISPC_GFX_FIFO_THRESHOLD,
                        DISPC_GFX_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_NORMAL(FIFO_BURSTSIZE_16x32)) |
                        DISPC_GFX_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_NORMAL)
                        );
        }

        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO1].bEnabled )
        {
            OUTREG32( &m_pDispRegs->tDISPC_VID1.FIFO_THRESHOLD,
                        DISPC_VID_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_NORMAL(FIFO_BURSTSIZE_16x32)) |
                        DISPC_VID_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_NORMAL)
                        );
        }

        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO2].bEnabled )
        {
            OUTREG32( &m_pDispRegs->tDISPC_VID2.FIFO_THRESHOLD,
                        DISPC_VID_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_NORMAL(FIFO_BURSTSIZE_16x32)) |
                        DISPC_VID_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_NORMAL)
                        );
        }

    }

    //  Get the destination for the pipeline
    eDest = g_rgPipelineMapping[ePipeline].eDestination;
    switch( eDest )
    {
        case OMAP_DSS_DESTINATION_LCD:
            //  Set enable and go bits for LCD
            dwDestEnable = DISPC_CONTROL_LCDENABLE;
            dwDestGo     = DISPC_CONTROL_GOLCD;
            break;

        case OMAP_DSS_DESTINATION_TVOUT:
            //  Set enable and go bits for TV Out
            dwDestEnable = DISPC_CONTROL_DIGITALENABLE;
            dwDestGo     = DISPC_CONTROL_GODIGITAL;
            break;

        default:
            ASSERT(0);
            goto cleanUp;
    }

    //  Try enabling overlay optimization
    EnableOverlayOptimization( TRUE );

    // First turn on the GO bit corresponding to the pipeline
    // that has to be disabled and wait for GO bit to clear.
    FlushRegs( dwDestGo );
    WaitForFlushDone( dwDestGo );

    // Additional flush required, when pipeline that is disabled in this
    // function was connected to DIGITALPATH. There could be another pipeline
    // driving the LCD path and so the LCD path has to be flushed as well
    if ( (dwDestGo != DISPC_CONTROL_GOLCD) && (dwNumPipelinesOn > 0) )
        FlushRegs( DISPC_CONTROL_GOLCD );

    //  Update ref count on destination
    //  If ref count on destination is 0, disable output
    if( --g_dwDestinationRefCnt[eDest] == 0 )
        CLRREG32( &m_pDispRegs->DISPC_CONTROL, dwDestEnable );

    // clear any pending interrupts
    dwIntrStatus = INREG32 ( &m_pDispRegs->DISPC_IRQSTATUS );
    SETREG32( &m_pDispRegs->DISPC_IRQSTATUS, dwIntrStatus );

    // Configure the LPR mode based on active Pipeline(s)
    // If the pipeline count is 0, then LPR should be ON
    bLPRState = (dwNumPipelinesOn == 0) ? TRUE : bLPRState;
    EnableLPR( bLPRState );

    //  Result
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::FlipPipeline(
    OMAP_DSS_PIPELINE       ePipeline,
    OMAPSurface*            pSurface
    )
{
    BOOL                bResult = FALSE;
    DWORD               surf_phy_addr;

//    RETAILMSG(1, (L"OMAPDisplayController::FlipPipeline called\r\n"));

    /* Only GRPX0 pipeline supported currently */
    surf_phy_addr = pSurface->PhysicalAddr(OMAP_DSS_ROTATION_0, 0);
    grpx_flipbuf(surf_phy_addr);

#if 0
    DWORD   dwDestGo;
    DWORD               dwInterlace;
    DWORD               dwHorzDecimation = 1;
    DWORD               dwVertDecimation = 1;


    //  Check if pipeline is already enabled; if not, no reason to flip
    if( g_rgPipelineMapping[ePipeline].bEnabled == FALSE )
        return FALSE;

    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;


    //  Get rotation and mirror settings for pipeline output
    eRotation = g_rgPipelineMapping[ePipeline].eRotation;
    bMirror   = g_rgPipelineMapping[ePipeline].bMirror;
    dwInterlace = g_rgPipelineScaling[ePipeline].dwInterlace;

    //Update clipping rectangle
    pSurface->SetClipping(&(g_rgPipelineMapping[ePipeline].pSurface->GetClipping()));

    //  Update GFX pipeline display base address
    if( ePipeline == OMAP_DSS_PIPELINE_GFX )
    {
        OUTREG32( &m_pDispRegs->DISPC_GFX_BA0, pSurface->PhysicalAddr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->DISPC_GFX_BA1, pSurface->PhysicalAddr(eRotation, bMirror) + dwInterlace );
    }

    //  Update VID1 pipeline display base address
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO1 )
    {
        OUTREG32( &m_pDispRegs->tDISPC_VID1.BA0, pSurface->PhysicalAddr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID1.BA1, pSurface->PhysicalAddr(eRotation, bMirror) + dwInterlace );
    }

    //  Update VID2 pipeline display base address
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO2 )
    {
        OUTREG32( &m_pDispRegs->tDISPC_VID2.BA0, pSurface->PhysicalAddr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID2.BA1, pSurface->PhysicalAddr(eRotation, bMirror) + dwInterlace );
    }


    //  Get the destination for the pipeline
    switch( g_rgPipelineMapping[ePipeline].eDestination )
    {
        case OMAP_DSS_DESTINATION_LCD:
            //  Set go bit for LCD
            dwDestGo  = DISPC_CONTROL_GOLCD;
            break;

        case OMAP_DSS_DESTINATION_TVOUT:
            //  Set go bit for TV Out
            dwDestGo  = DISPC_CONTROL_GODIGITAL;
            break;

        default:
            ASSERT(0);
            goto cleanUp;
    }

    //Clear Vysnc since we are about to flip (to avoid false signaling Vsync event)
    SETREG32(&m_pDispRegs->DISPC_IRQSTATUS, DISPC_IRQSTATUS_VSYNC);
    if(m_bTVEnable == TRUE)
    {
        SETREG32(&m_pDispRegs->DISPC_IRQSTATUS, DISPC_IRQSTATUS_EVSYNC_EVEN);
        SETREG32(&m_pDispRegs->DISPC_IRQSTATUS, DISPC_IRQSTATUS_EVSYNC_ODD);
    }
    //  Flush the shadow registers
    FlushRegs( dwDestGo );

    //  Update mapping of pipeline surface
    g_rgPipelineMapping[ePipeline].pSurface  = pSurface;

    //  Set the decimation factors for the surface back to normal
    //  Leave the clipping setting as is
    pSurface->SetHorizontalScaling( 1 );
    pSurface->SetVerticalScaling( 1 );


    //  Result
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::IsPipelineFlipping(
    OMAP_DSS_PIPELINE       ePipeline,
    OMAPSurface*            pSurface
    )
{
    BOOL                bResult = FALSE;

    RETAILMSG(1, (L"OMAPDisplayController::IsPipelineFlipping called\r\n"));

#if 0
    OMAP_DSS_ROTATION   eRotation;
    BOOL                bMirror;
    DWORD               dwDestGo = DISPC_CONTROL_GOLCD;

     //  Check if pipeline is already enabled; if not, no reason to query flip status
    if( g_rgPipelineMapping[ePipeline].bEnabled == FALSE )
        return FALSE;

    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;

    //  Get rotation and mirror settings for pipeline output
    eRotation   = g_rgPipelineMapping[ePipeline].eRotation;
    bMirror     = g_rgPipelineMapping[ePipeline].bMirror;

    //  Get the destination for the pipeline
    switch( g_rgPipelineMapping[ePipeline].eDestination )
    {
        case OMAP_DSS_DESTINATION_LCD:
            //  Set go bit for LCD
            dwDestGo  = DISPC_CONTROL_GOLCD;
            break;

        case OMAP_DSS_DESTINATION_TVOUT:
            //  Set go bit for TV Out
            dwDestGo  = DISPC_CONTROL_GODIGITAL;
            break;
    }

    //Test if we have already flipped (destGo has been cleared)
    if( (INREG32(&m_pDispRegs->DISPC_CONTROL) & dwDestGo) == 0)
    {
        bResult = FALSE;
        goto cleanUp;
    }

    //See if we are flipping to the surface given
    if( ePipeline == OMAP_DSS_PIPELINE_GFX )
    {
        bResult = INREG32(&m_pDispRegs->DISPC_GFX_BA0) == pSurface->PhysicalAddr(eRotation, bMirror);
    }

    //  Update VID1 pipeline display base address
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO1 )
    {
        bResult = INREG32(&m_pDispRegs->tDISPC_VID1.BA0) == pSurface->PhysicalAddr(eRotation, bMirror);
    }

    //  Update VID2 pipeline display base address
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO2 )
    {
        bResult = INREG32(&m_pDispRegs->tDISPC_VID2.BA0) == pSurface->PhysicalAddr(eRotation, bMirror);
    }

    //  Result
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;

}
//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::MovePipeline(
    OMAP_DSS_PIPELINE       ePipeline,
    LONG                    lXPos,
    LONG                    lYPos
    )
{
    BOOL    bResult = FALSE;

    RETAILMSG(1, (L"OMAPDisplayController::MovePipeline called\r\n"));

#if 0
    DWORD   dwDestGo = DISPC_CONTROL_GOLCD;
    DWORD   dwX, dwY;


    //  Check if pipeline is enabled; ignore operation if not
    if( g_rgPipelineMapping[ePipeline].bEnabled == FALSE )
        return FALSE;

    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;

    //  Compute new origin based on pipeline rotation angle
    switch( g_rgPipelineMapping[ePipeline].eRotation )
    {
        case OMAP_DSS_ROTATION_0:
            dwX = lXPos;
            dwY = lYPos;
            break;

        case OMAP_DSS_ROTATION_90:
            dwX = lYPos;
            dwY = GetLCDHeight() - g_rgPipelineMapping[ePipeline].dwDestHeight - lXPos;
            break;

        case OMAP_DSS_ROTATION_180:
            dwX = GetLCDWidth() - g_rgPipelineMapping[ePipeline].dwDestWidth - lXPos;
            dwY = GetLCDHeight() - g_rgPipelineMapping[ePipeline].dwDestHeight - lYPos;
            break;

        case OMAP_DSS_ROTATION_270:
            dwX = GetLCDWidth() - g_rgPipelineMapping[ePipeline].dwDestWidth - lYPos;
            dwY = lXPos;
            break;

        default:
            ASSERT(0);
            goto cleanUp;
    }


    //  Update GFX pipeline display position
    if( ePipeline == OMAP_DSS_PIPELINE_GFX )
    {
        OUTREG32( &m_pDispRegs->DISPC_GFX_POSITION,
                    DISPC_GFX_POS_GFXPOSX(dwX) |
                    DISPC_GFX_POS_GFXPOSY(dwY)
                    );
    }

    //  Update VID1 pipeline display position
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO1 )
    {
        OUTREG32( &m_pDispRegs->tDISPC_VID1.POSITION,
                    DISPC_VID_POS_VIDPOSX(dwX) |
                    DISPC_VID_POS_VIDPOSY(dwY)
                    );
    }

    //  Update VID2 pipeline display position
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO2 )
    {
        OUTREG32( &m_pDispRegs->tDISPC_VID2.POSITION,
                    DISPC_VID_POS_VIDPOSX(dwX) |
                    DISPC_VID_POS_VIDPOSY(dwY)
                    );
    }


    //  Get the destination for the pipeline
    switch( g_rgPipelineMapping[ePipeline].eDestination )
    {
        case OMAP_DSS_DESTINATION_LCD:
            //  Set go bit for LCD
            dwDestGo  = DISPC_CONTROL_GOLCD;
            break;

        case OMAP_DSS_DESTINATION_TVOUT:
            //  Set go bit for TV Out
            dwDestGo  = DISPC_CONTROL_GODIGITAL;
            break;
    }


    //  Enable/update overlay optimization
    EnableOverlayOptimization( TRUE );

    //  Flush the shadow registers
    FlushRegs( dwDestGo );


    //  Result
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::RotatePipeline(
    OMAP_DSS_PIPELINE       ePipeline,
    OMAP_DSS_ROTATION       eRotation
    )
{
    BOOL    bResult = FALSE;

    RETAILMSG(1, (L"OMAPDisplayController::RotatePipeline called\r\n"));

#if 0
    OMAPSurface*        pSurface;
    BOOL                bMirror = FALSE;
    DWORD               dwVidRotation = 0;
    DWORD               dwHorzDecimation = 1;
    DWORD               dwVertDecimation = 1;
    DWORD               dwInterlace = 0;


    //  If no change in the rotation, do nothing
    if( g_rgPipelineMapping[ePipeline].eRotation == eRotation )
        return TRUE;

    //  If no associated pipeline, just set the default rotation of the pipeline
    if( g_rgPipelineMapping[ePipeline].pSurface == NULL )
    {
        g_rgPipelineMapping[ePipeline].eRotation = eRotation;
        return TRUE;
    }


    //  Get the surface being output
    pSurface = g_rgPipelineMapping[ePipeline].pSurface;
    bMirror = g_rgPipelineMapping[ePipeline].bMirror;

    //  Get the decimation settings for the surface
    dwHorzDecimation = g_rgPipelineScaling[ePipeline].dwHorzScaling;
    dwVertDecimation = g_rgPipelineScaling[ePipeline].dwVertScaling;
    dwInterlace      = g_rgPipelineScaling[ePipeline].dwInterlace;


    //  Set rotation attributes for video pipelines if pixel format is YUV
    if( pSurface->PixelFormat() == OMAP_DSS_PIXELFORMAT_YUV2 ||
        pSurface->PixelFormat() == OMAP_DSS_PIXELFORMAT_UYVY )
    {
        //  Depending on rotation and mirror settings, change the VID rotation attributes
        switch( eRotation )
        {
            case OMAP_DSS_ROTATION_0:
                //  Settings for rotation angle 0
                dwVidRotation = (bMirror) ? DISPC_VID_ATTR_VIDROTATION_180 : DISPC_VID_ATTR_VIDROTATION_0;

                //  Set the decimation for the surface
                pSurface->SetHorizontalScaling( dwHorzDecimation );
                pSurface->SetVerticalScaling( dwVertDecimation );
                break;

            case OMAP_DSS_ROTATION_90:
                //  Settings for rotation angle 90 (270 for DSS setting)
                dwVidRotation = DISPC_VID_ATTR_VIDROTATION_270 | DISPC_VID_ATTR_VIDROWREPEATENABLE;

                //  Set the decimation for the surface
                pSurface->SetHorizontalScaling( dwVertDecimation );
                pSurface->SetVerticalScaling( dwHorzDecimation );
                break;

            case OMAP_DSS_ROTATION_180:
                //  Settings for rotation angle 180
                dwVidRotation = (bMirror) ? DISPC_VID_ATTR_VIDROTATION_0 : DISPC_VID_ATTR_VIDROTATION_180;

                //  Set the decimation for the surface
                pSurface->SetHorizontalScaling( dwHorzDecimation );
                pSurface->SetVerticalScaling( dwVertDecimation );
                break;

            case OMAP_DSS_ROTATION_270:
                //  Settings for rotation angle 270 (90 for DSS setting)
                dwVidRotation = DISPC_VID_ATTR_VIDROTATION_90 | DISPC_VID_ATTR_VIDROWREPEATENABLE;

                //  Set the decimation for the surface
                pSurface->SetHorizontalScaling( dwVertDecimation );
                pSurface->SetVerticalScaling( dwHorzDecimation );
                break;

            default:
                ASSERT(0);
                return FALSE;
        }
    }


    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;


    //  Update GFX pipeline display base address
    if( ePipeline == OMAP_DSS_PIPELINE_GFX )
    {
        OUTREG32( &m_pDispRegs->DISPC_GFX_PIXEL_INC, pSurface->PixelIncr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->DISPC_GFX_ROW_INC, pSurface->RowIncr(eRotation, bMirror) );

        OUTREG32( &m_pDispRegs->DISPC_GFX_BA0, pSurface->PhysicalAddr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->DISPC_GFX_BA1, pSurface->PhysicalAddr(eRotation, bMirror) + dwInterlace );
    }

    //  Update VID1 pipeline display base address and attributes for rotation
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO1 )
    {
        CLRREG32( &m_pDispRegs->tDISPC_VID1.ATTRIBUTES, DISPC_VID_ATTR_VIDROTATION_MASK|DISPC_VID_ATTR_VIDROWREPEATENABLE );
        SETREG32( &m_pDispRegs->tDISPC_VID1.ATTRIBUTES, dwVidRotation );

        OUTREG32( &m_pDispRegs->tDISPC_VID1.PIXEL_INC, pSurface->PixelIncr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID1.ROW_INC, pSurface->RowIncr(eRotation, bMirror) );

        OUTREG32( &m_pDispRegs->tDISPC_VID1.BA0, pSurface->PhysicalAddr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID1.BA1, pSurface->PhysicalAddr(eRotation, bMirror) + dwInterlace );
    }

    //  Update VID2 pipeline display base address and attributes for rotation
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO2 )
    {
        CLRREG32( &m_pDispRegs->tDISPC_VID2.ATTRIBUTES, DISPC_VID_ATTR_VIDROTATION_MASK|DISPC_VID_ATTR_VIDROWREPEATENABLE );
        SETREG32( &m_pDispRegs->tDISPC_VID2.ATTRIBUTES, dwVidRotation );

        OUTREG32( &m_pDispRegs->tDISPC_VID2.PIXEL_INC, pSurface->PixelIncr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID2.ROW_INC, pSurface->RowIncr(eRotation, bMirror) );

        OUTREG32( &m_pDispRegs->tDISPC_VID2.BA0, pSurface->PhysicalAddr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID2.BA1, pSurface->PhysicalAddr(eRotation, bMirror) + dwInterlace );
    }


    //  Update pipeline output rotation
    g_rgPipelineMapping[ePipeline].eRotation = eRotation;

    //  Set the decimation factors for the surface back to normal
    pSurface->SetHorizontalScaling( 1 );
    pSurface->SetVerticalScaling( 1 );


    //  Result
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::MirrorPipeline(
    OMAP_DSS_PIPELINE       ePipeline,
    BOOL                    bMirror
    )
{
    BOOL    bResult = FALSE;

    RETAILMSG(1, (L"OMAPDisplayController::MirrorPipeline called\r\n"));

#if 0
    OMAPSurface*        pSurface;
    OMAP_DSS_ROTATION   eRotation;
    DWORD               dwHorzDecimation = 1;
    DWORD               dwVertDecimation = 1;
    DWORD               dwInterlace = 0;


    //  If no change in the mirror setting, do nothing
    if( g_rgPipelineMapping[ePipeline].bMirror == bMirror )
        return TRUE;

    //  If no associated pipeline, just set the default mirror setting of the pipeline
    if( g_rgPipelineMapping[ePipeline].pSurface == NULL )
    {
        g_rgPipelineMapping[ePipeline].bMirror = bMirror;
        return TRUE;
    }

    //  Get the surface being output
    pSurface = g_rgPipelineMapping[ePipeline].pSurface;
    eRotation = g_rgPipelineMapping[ePipeline].eRotation;

    //  Get the decimation settings for the surface
    dwHorzDecimation = g_rgPipelineScaling[ePipeline].dwHorzScaling;
    dwVertDecimation = g_rgPipelineScaling[ePipeline].dwVertScaling;
    dwInterlace      = g_rgPipelineScaling[ePipeline].dwInterlace;


    //  Depending on rotation settings, change the surface scaling attributes
    switch( eRotation )
    {
        case OMAP_DSS_ROTATION_0:
        case OMAP_DSS_ROTATION_180:
            //  Set the decimation for the surface
            pSurface->SetHorizontalScaling( dwHorzDecimation );
            pSurface->SetVerticalScaling( dwVertDecimation );
            break;

        case OMAP_DSS_ROTATION_90:
        case OMAP_DSS_ROTATION_270:
            //  Set the decimation for the surface
            pSurface->SetHorizontalScaling( dwVertDecimation );
            pSurface->SetVerticalScaling( dwHorzDecimation );
            break;

        default:
            ASSERT(0);
            return FALSE;
    }

    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;


    //  Update GFX pipeline for mirror setting
    if( ePipeline == OMAP_DSS_PIPELINE_GFX )
    {
        OUTREG32( &m_pDispRegs->DISPC_GFX_PIXEL_INC, pSurface->PixelIncr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->DISPC_GFX_ROW_INC, pSurface->RowIncr(eRotation, bMirror) );

        OUTREG32( &m_pDispRegs->DISPC_GFX_BA0, pSurface->PhysicalAddr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->DISPC_GFX_BA1, pSurface->PhysicalAddr(eRotation, bMirror) + dwInterlace );
    }

    //  Update VID1 pipeline for mirror setting
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO1 )
    {
        OUTREG32( &m_pDispRegs->tDISPC_VID1.PIXEL_INC, pSurface->PixelIncr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID1.ROW_INC, pSurface->RowIncr(eRotation, bMirror) );

        OUTREG32( &m_pDispRegs->tDISPC_VID1.BA0, pSurface->PhysicalAddr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID1.BA1, pSurface->PhysicalAddr(eRotation, bMirror) + dwInterlace );
    }

    //  Update VID2 pipeline for mirror setting
    if( ePipeline == OMAP_DSS_PIPELINE_VIDEO2 )
    {
        OUTREG32( &m_pDispRegs->tDISPC_VID2.PIXEL_INC, pSurface->PixelIncr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID2.ROW_INC, pSurface->RowIncr(eRotation, bMirror) );

        OUTREG32( &m_pDispRegs->tDISPC_VID2.BA0, pSurface->PhysicalAddr(eRotation, bMirror) );
        OUTREG32( &m_pDispRegs->tDISPC_VID2.BA1, pSurface->PhysicalAddr(eRotation, bMirror) + dwInterlace );
    }


    //  Update pipeline output mirror setting
    g_rgPipelineMapping[ePipeline].bMirror = bMirror;

    //  Set the decimation factors for the surface back to normal
    pSurface->SetHorizontalScaling( 1 );
    pSurface->SetVerticalScaling( 1 );


    //  Result
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::UpdateScalingAttribs(
    OMAP_DSS_PIPELINE       ePipeline,
    RECT*                   pSrcRect,
    RECT*                   pDestRect
    )
{
    BOOL    bResult = FALSE;

    RETAILMSG(1, (L"OMAPDisplayController::UpdateScalingAttribs called\r\n"));

    //  Check if pipeline is enabled; ignore operation if not
    if( g_rgPipelineMapping[ePipeline].bEnabled == FALSE )
        return FALSE;

    //  Update the scaling attributes
    bResult = SetScalingAttribs( ePipeline, pSrcRect, pDestRect );

#if 0
    if( bResult )
    {
        DWORD dwDestGo = 0;

        //  Get the destination for the pipeline
        switch( g_rgPipelineMapping[ePipeline].eDestination )
        {
            case OMAP_DSS_DESTINATION_LCD:
                //  Set go bit for LCD
                dwDestGo  = DISPC_CONTROL_GOLCD;
                break;

            case OMAP_DSS_DESTINATION_TVOUT:
                //  Set go bit for TV Out
                dwDestGo  = DISPC_CONTROL_GODIGITAL;
                break;

            default:
                ASSERT(0);
                goto cleanUp;
        }

        //  Enable/update overlay optimization
        EnableOverlayOptimization( TRUE );

        //  Flush the shadow registers
        FlushRegs( dwDestGo );
    }


cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::EnableColorKey(
    OMAP_DSS_COLORKEY       eColorKey,
    OMAP_DSS_DESTINATION    eDestination,
    DWORD                   dwColor
    )
{
    BOOL    bResult = FALSE;

    RETAILMSG(1, (L"OMAPDisplayController::EnableColorKey called\r\n"));

#if 0
    DWORD   dwCurrentColor = 0;
    DWORD   dwDestGo = 0;


    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;


    //  Enable color key for the LCD
    if( eDestination == OMAP_DSS_DESTINATION_LCD )
    {
        //  Set color key for LCD
        switch( eColorKey )
        {
            case OMAP_DSS_COLORKEY_TRANS_SOURCE:
            case OMAP_DSS_COLORKEY_TRANS_DEST:
                //  Set transparent color for LCD
                OUTREG32( &m_pDispRegs->DISPC_TRANS_COLOR0, dwColor );

                //  Enable LCD transparent color blender
                SETREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_TCKLCDENABLE );

                //  Select source or destination transparency
                if( eColorKey == OMAP_DSS_COLORKEY_TRANS_SOURCE )
                    SETREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_TCKLCDSELECTION );
                else
                    CLRREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_TCKLCDSELECTION );

                break;


            case OMAP_DSS_COLORKEY_GLOBAL_ALPHA_GFX:
            case OMAP_DSS_COLORKEY_GLOBAL_ALPHA_VIDEO2:
                //  Get current color (both LCD and VID2 are in this single register)
                dwCurrentColor = INREG32( &m_pDispRegs->DISPC_GLOBAL_ALPHA );

                //  Set global alpha color for LCD
                if( eColorKey == OMAP_DSS_COLORKEY_GLOBAL_ALPHA_GFX )
                    dwColor = (dwCurrentColor & 0xFFFF0000) | DISPC_GLOBAL_ALPHA_GFX(dwColor);
                else
                    dwColor = (dwCurrentColor & 0x0000FFFF) | DISPC_GLOBAL_ALPHA_VID2(dwColor);

                OUTREG32( &m_pDispRegs->DISPC_GLOBAL_ALPHA, dwColor );

                //  Enable LCD alpha blender
                SETREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_LCDALPHABLENDERENABLE );
                break;

            default:
                ASSERT(0);
                goto cleanUp;
        }

        //  Display overlay optimization
        EnableOverlayOptimization( FALSE );

        //  Set dest GO
        dwDestGo = DISPC_CONTROL_GOLCD;
    }


    //  Enable color key for TV out
    if( eDestination == OMAP_DSS_DESTINATION_TVOUT )
    {
        //  Set color key for TV out
        switch( eColorKey )
        {
            case OMAP_DSS_COLORKEY_TRANS_SOURCE:
            case OMAP_DSS_COLORKEY_TRANS_DEST:
                //  Set transparent color for DIG
                OUTREG32( &m_pDispRegs->DISPC_TRANS_COLOR1, dwColor );

                //  Enable DIG transparent color blender
                SETREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_TCKDIGENABLE );

                //  Select source or destination transparency
                if( eColorKey == OMAP_DSS_COLORKEY_TRANS_SOURCE )
                    SETREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_TCKDIGSELECTION );
                else
                    CLRREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_TCKDIGSELECTION );

                break;


            case OMAP_DSS_COLORKEY_GLOBAL_ALPHA_GFX:
            case OMAP_DSS_COLORKEY_GLOBAL_ALPHA_VIDEO2:
                //  Get current color (both LCD and VID2 are in this single register)
                dwCurrentColor = INREG32( &m_pDispRegs->DISPC_GLOBAL_ALPHA );

                //  Set global alpha color
                if( eColorKey == OMAP_DSS_COLORKEY_GLOBAL_ALPHA_GFX )
                    dwColor = (dwCurrentColor & 0xFFFF0000) | DISPC_GLOBAL_ALPHA_GFX(dwColor);
                else
                    dwColor = (dwCurrentColor & 0x0000FFFF) | DISPC_GLOBAL_ALPHA_VID2(dwColor);

                OUTREG32( &m_pDispRegs->DISPC_GLOBAL_ALPHA, dwColor );

                //  Enable DIG alpha blender
                SETREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_DIGALPHABLENDERENABLE );
                break;

            default:
                ASSERT(0);
                goto cleanUp;
        }

        //  Set dest GO
        dwDestGo = DISPC_CONTROL_GODIGITAL;
    }


    //  Flush the shadow registers
    FlushRegs( dwDestGo );


    //  Result
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::DisableColorKey(
    OMAP_DSS_COLORKEY       eColorKey,
    OMAP_DSS_DESTINATION    eDestination
    )
{
    BOOL    bResult = FALSE;

    RETAILMSG(1, (L"OMAPDisplayController::DisableColorKey called\r\n"));

#if 0
    DWORD   dwDestGo = 0;

    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;


    //  Enable color key for the LCD
    if( eDestination == OMAP_DSS_DESTINATION_LCD )
    {
        //  Set color key for LCD
        switch( eColorKey )
        {
            case OMAP_DSS_COLORKEY_TRANS_SOURCE:
            case OMAP_DSS_COLORKEY_TRANS_DEST:
                //  Disable LCD transparent color blender
                CLRREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_TCKLCDENABLE );
                break;


            case OMAP_DSS_COLORKEY_GLOBAL_ALPHA_GFX:
            case OMAP_DSS_COLORKEY_GLOBAL_ALPHA_VIDEO2:
                //  Disable LCD alpha blender
                CLRREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_LCDALPHABLENDERENABLE );
                break;

            default:
                ASSERT(0);
                goto cleanUp;
        }

        //  Set dest GO
        dwDestGo = DISPC_CONTROL_GOLCD;
    }


    //  Enable color key for TV out
    if( eDestination == OMAP_DSS_DESTINATION_TVOUT )
    {
        //  Set color key for TV out
        switch( eColorKey )
        {
            case OMAP_DSS_COLORKEY_TRANS_SOURCE:
            case OMAP_DSS_COLORKEY_TRANS_DEST:
                //  Disable DIG transparent color blender
                CLRREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_TCKDIGENABLE );
                break;


            case OMAP_DSS_COLORKEY_GLOBAL_ALPHA_GFX:
            case OMAP_DSS_COLORKEY_GLOBAL_ALPHA_VIDEO2:
                //  Disable DIG alpha blender
                CLRREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_DIGALPHABLENDERENABLE );
                break;

            default:
                ASSERT(0);
                goto cleanUp;
        }

        //  Set dest GO
        dwDestGo = DISPC_CONTROL_GODIGITAL;
    }


    //  Flush the shadow registers
    FlushRegs( dwDestGo );


    //  Result
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::SetContrastLevel(
    DWORD dwContrastLevel
    )
{
    //  Set contrast level by copying in new gamma correction curve
    m_dwContrastLevel = (dwContrastLevel < NUM_CONTRAST_LEVELS) ? dwContrastLevel : NUM_CONTRAST_LEVELS - 1;

#if 0
    //  Copy the selected table to the gamma physical memory location
    memcpy(m_pGammaBufVirt, &(g_dwGammaTable[(NUM_CONTRAST_LEVELS - 1) - m_dwContrastLevel][0]), NUM_GAMMA_VALS*sizeof(DWORD));
#endif
    return TRUE;
}


//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::SaveRegisters(
    OMAP_DSS_DESTINATION eDestination
    )
{
    BOOL    bResult = FALSE;

#if 0
    DWORD   i;
    OMAP_DISPC_REGS   *pDisplaySaveRestore = 0;

    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;

    // Choose the last active LCD context ( internal LCD/external LCD)
    if (!m_bHDMIEnable)
        pDisplaySaveRestore = &g_rgDisplaySaveRestore;
    else
        pDisplaySaveRestore = &g_rgDisplaySaveRestore_eLcd;

    if (pDisplaySaveRestore == NULL)
        goto cleanUp;

    // Save the DISPC common register contents
    pDisplaySaveRestore->DISPC_CONFIG = INREG32 ( &m_pDispRegs->DISPC_CONFIG );
    pDisplaySaveRestore->DISPC_GLOBAL_ALPHA = INREG32 (&m_pDispRegs->DISPC_GLOBAL_ALPHA );
    pDisplaySaveRestore->DISPC_IRQENABLE = INREG32( &m_pDispRegs->DISPC_IRQENABLE );
    pDisplaySaveRestore->DISPC_TRANS_COLOR0 = INREG32( &m_pDispRegs->DISPC_TRANS_COLOR0 );
    pDisplaySaveRestore->DISPC_TRANS_COLOR1 = INREG32( &m_pDispRegs->DISPC_TRANS_COLOR1 );

    //  Save the DSS and LCD registers
    if( eDestination == OMAP_DSS_DESTINATION_LCD )
    {
        //  Save off GFX plane registers if enabled
        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_GFX].bEnabled )
        {
            pDisplaySaveRestore->DISPC_GFX_BA0 = INREG32( &m_pDispRegs->DISPC_GFX_BA0 );
            pDisplaySaveRestore->DISPC_GFX_BA1 = INREG32( &m_pDispRegs->DISPC_GFX_BA1 );
            pDisplaySaveRestore->DISPC_GFX_POSITION = INREG32( &m_pDispRegs->DISPC_GFX_POSITION );
            pDisplaySaveRestore->DISPC_GFX_SIZE = INREG32( &m_pDispRegs->DISPC_GFX_SIZE );
            pDisplaySaveRestore->DISPC_GFX_ATTRIBUTES = INREG32( &m_pDispRegs->DISPC_GFX_ATTRIBUTES );
            pDisplaySaveRestore->DISPC_GFX_FIFO_THRESHOLD = INREG32( &m_pDispRegs->DISPC_GFX_FIFO_THRESHOLD );
            pDisplaySaveRestore->DISPC_GFX_ROW_INC = INREG32( &m_pDispRegs->DISPC_GFX_ROW_INC );
            pDisplaySaveRestore->DISPC_GFX_PIXEL_INC = INREG32( &m_pDispRegs->DISPC_GFX_PIXEL_INC );
            pDisplaySaveRestore->DISPC_GFX_WINDOW_SKIP = INREG32( &m_pDispRegs->DISPC_GFX_WINDOW_SKIP );
        }

        //  Save off VID1 plane registers if enabled
        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO1].bEnabled )
        {
            pDisplaySaveRestore->tDISPC_VID1.BA0 = INREG32( &m_pDispRegs->tDISPC_VID1.BA0 );
            pDisplaySaveRestore->tDISPC_VID1.BA1 = INREG32( &m_pDispRegs->tDISPC_VID1.BA1 );
            pDisplaySaveRestore->tDISPC_VID1.POSITION = INREG32( &m_pDispRegs->tDISPC_VID1.POSITION );
            pDisplaySaveRestore->tDISPC_VID1.SIZE = INREG32( &m_pDispRegs->tDISPC_VID1.SIZE );
            pDisplaySaveRestore->tDISPC_VID1.ATTRIBUTES = INREG32( &m_pDispRegs->tDISPC_VID1.ATTRIBUTES );
            pDisplaySaveRestore->tDISPC_VID1.FIFO_THRESHOLD = INREG32( &m_pDispRegs->tDISPC_VID1.FIFO_THRESHOLD );
            pDisplaySaveRestore->tDISPC_VID1.ROW_INC = INREG32( &m_pDispRegs->tDISPC_VID1.ROW_INC );
            pDisplaySaveRestore->tDISPC_VID1.PIXEL_INC = INREG32( &m_pDispRegs->tDISPC_VID1.PIXEL_INC );
            pDisplaySaveRestore->tDISPC_VID1.FIR = INREG32( &m_pDispRegs->tDISPC_VID1.FIR );
            pDisplaySaveRestore->tDISPC_VID1.PICTURE_SIZE = INREG32( &m_pDispRegs->tDISPC_VID1.PICTURE_SIZE );
            pDisplaySaveRestore->tDISPC_VID1.ACCU0 = INREG32( &m_pDispRegs->tDISPC_VID1.ACCU0 );
            pDisplaySaveRestore->tDISPC_VID1.ACCU1 = INREG32( &m_pDispRegs->tDISPC_VID1.ACCU1 );

            //  Scaling coefficients
            for( i = 0; i < NUM_SCALING_PHASES; i++ )
            {
                pDisplaySaveRestore->tDISPC_VID1.aFIR_COEF[i].ulH = INREG32( &m_pDispRegs->tDISPC_VID1.aFIR_COEF[i].ulH );
                pDisplaySaveRestore->tDISPC_VID1.aFIR_COEF[i].ulHV = INREG32( &m_pDispRegs->tDISPC_VID1.aFIR_COEF[i].ulHV );
                pDisplaySaveRestore->DISPC_VID1_FIR_COEF_V[i] = INREG32( &m_pDispRegs->DISPC_VID1_FIR_COEF_V[i] );
            }
        }

        //  Save off VID2 plane registers if enabled
        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO2].bEnabled )
        {
            pDisplaySaveRestore->tDISPC_VID2.BA0 = INREG32( &m_pDispRegs->tDISPC_VID2.BA0 );
            pDisplaySaveRestore->tDISPC_VID2.BA1 = INREG32( &m_pDispRegs->tDISPC_VID2.BA1 );
            pDisplaySaveRestore->tDISPC_VID2.POSITION = INREG32( &m_pDispRegs->tDISPC_VID2.POSITION );
            pDisplaySaveRestore->tDISPC_VID2.SIZE = INREG32( &m_pDispRegs->tDISPC_VID2.SIZE );
            pDisplaySaveRestore->tDISPC_VID2.ATTRIBUTES = INREG32( &m_pDispRegs->tDISPC_VID2.ATTRIBUTES );
            pDisplaySaveRestore->tDISPC_VID2.FIFO_THRESHOLD = INREG32( &m_pDispRegs->tDISPC_VID2.FIFO_THRESHOLD );
            pDisplaySaveRestore->tDISPC_VID2.ROW_INC = INREG32( &m_pDispRegs->tDISPC_VID2.ROW_INC );
            pDisplaySaveRestore->tDISPC_VID2.PIXEL_INC = INREG32( &m_pDispRegs->tDISPC_VID2.PIXEL_INC );
            pDisplaySaveRestore->tDISPC_VID2.FIR = INREG32( &m_pDispRegs->tDISPC_VID2.FIR );
            pDisplaySaveRestore->tDISPC_VID2.PICTURE_SIZE = INREG32( &m_pDispRegs->tDISPC_VID2.PICTURE_SIZE );
            pDisplaySaveRestore->tDISPC_VID2.ACCU0 = INREG32( &m_pDispRegs->tDISPC_VID2.ACCU0 );
            pDisplaySaveRestore->tDISPC_VID2.ACCU1 = INREG32( &m_pDispRegs->tDISPC_VID2.ACCU1 );

            //  Scaling coefficients
            for( i = 0; i < NUM_SCALING_PHASES; i++ )
            {
                pDisplaySaveRestore->tDISPC_VID2.aFIR_COEF[i].ulH = INREG32( &m_pDispRegs->tDISPC_VID2.aFIR_COEF[i].ulH );
                pDisplaySaveRestore->tDISPC_VID2.aFIR_COEF[i].ulHV = INREG32( &m_pDispRegs->tDISPC_VID2.aFIR_COEF[i].ulHV );
                pDisplaySaveRestore->DISPC_VID2_FIR_COEF_V[i] = INREG32( &m_pDispRegs->DISPC_VID2_FIR_COEF_V[i] );
            }
        }
    }


    //  Success
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::RestoreRegisters(
    OMAP_DSS_DESTINATION eDestination
    )
{
    BOOL    bResult = FALSE;

#if 0
    DWORD   i;
    OMAP_DISPC_REGS   *pDisplaySaveRestore = 0;

    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;

    // Choose the last active LCD context ( internal LCD/external LCD)
    if (!m_bHDMIEnable)
        pDisplaySaveRestore = &g_rgDisplaySaveRestore;
    else
        pDisplaySaveRestore = &g_rgDisplaySaveRestore_eLcd;

    if (pDisplaySaveRestore == NULL)
        goto cleanUp;


    //  Restore the DSS and LCD registers
    if( eDestination == OMAP_DSS_DESTINATION_LCD )
    {
        //  Configure interconnect parameters
        OUTREG32( &m_pDSSRegs->DSS_SYSCONFIG, DISPC_SYSCONFIG_AUTOIDLE );
        OUTREG32( &m_pDispRegs->DISPC_SYSCONFIG, DISPC_SYSCONFIG_AUTOIDLE|SYSCONFIG_NOIDLE|SYSCONFIG_NOSTANDBY );

        //  Not enabling any interrupts
        OUTREG32( &m_pDispRegs->DISPC_IRQENABLE , pDisplaySaveRestore->DISPC_IRQENABLE);

        //  Initialize the LCD by calling PDD
        LcdPdd_LCD_Initialize(
            m_pDSSRegs,
            m_pDispRegs,
            NULL,
            m_pVencRegs);

        OUTREG32( &m_pDispRegs->DISPC_CONFIG, pDisplaySaveRestore->DISPC_CONFIG );

        //Enable/Disable Gamma correction
        if(m_bGammaEnable)
            SETREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_PALETTEGAMMATABLE );
        else
            CLRREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_PALETTEGAMMATABLE );

        // Load Gamma Table
        OUTREG32( &m_pDispRegs->DISPC_GFX_TABLE_BA, m_dwGammaBufPhys);

        //  Restore global alpha value
        OUTREG32( &m_pDispRegs->DISPC_GLOBAL_ALPHA, pDisplaySaveRestore->DISPC_GLOBAL_ALPHA );

        // Restore transparency value
        OUTREG32( &m_pDispRegs->DISPC_TRANS_COLOR0, pDisplaySaveRestore->DISPC_TRANS_COLOR0 );
        OUTREG32( &m_pDispRegs->DISPC_TRANS_COLOR1, pDisplaySaveRestore->DISPC_TRANS_COLOR1 );

        //  Restore GFX plane registers if enabled
        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_GFX].bEnabled )
        {
            OUTREG32( &m_pDispRegs->DISPC_GFX_BA0, pDisplaySaveRestore->DISPC_GFX_BA0 );
            OUTREG32( &m_pDispRegs->DISPC_GFX_BA1, pDisplaySaveRestore->DISPC_GFX_BA1 );
            OUTREG32( &m_pDispRegs->DISPC_GFX_POSITION, pDisplaySaveRestore->DISPC_GFX_POSITION );
            OUTREG32( &m_pDispRegs->DISPC_GFX_SIZE, pDisplaySaveRestore->DISPC_GFX_SIZE );
            OUTREG32( &m_pDispRegs->DISPC_GFX_ATTRIBUTES, pDisplaySaveRestore->DISPC_GFX_ATTRIBUTES );
            OUTREG32( &m_pDispRegs->DISPC_GFX_FIFO_THRESHOLD, pDisplaySaveRestore->DISPC_GFX_FIFO_THRESHOLD );
            OUTREG32( &m_pDispRegs->DISPC_GFX_ROW_INC, pDisplaySaveRestore->DISPC_GFX_ROW_INC );
            OUTREG32( &m_pDispRegs->DISPC_GFX_PIXEL_INC, pDisplaySaveRestore->DISPC_GFX_PIXEL_INC );
            OUTREG32( &m_pDispRegs->DISPC_GFX_WINDOW_SKIP, pDisplaySaveRestore->DISPC_GFX_WINDOW_SKIP );
        }


        //  Restore VID1 plane registers if enabled
        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO1].bEnabled )
        {
            OUTREG32( &m_pDispRegs->tDISPC_VID1.BA0, pDisplaySaveRestore->tDISPC_VID1.BA0 );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.BA1, pDisplaySaveRestore->tDISPC_VID1.BA1 );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.POSITION, pDisplaySaveRestore->tDISPC_VID1.POSITION );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.SIZE, pDisplaySaveRestore->tDISPC_VID1.SIZE );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.ATTRIBUTES, pDisplaySaveRestore->tDISPC_VID1.ATTRIBUTES );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.FIFO_THRESHOLD, pDisplaySaveRestore->tDISPC_VID1.FIFO_THRESHOLD );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.ROW_INC, pDisplaySaveRestore->tDISPC_VID1.ROW_INC );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.PIXEL_INC, pDisplaySaveRestore->tDISPC_VID1.PIXEL_INC );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.FIR, pDisplaySaveRestore->tDISPC_VID1.FIR );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.PICTURE_SIZE, pDisplaySaveRestore->tDISPC_VID1.PICTURE_SIZE );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.ACCU0, pDisplaySaveRestore->tDISPC_VID1.ACCU0 );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.ACCU1, pDisplaySaveRestore->tDISPC_VID1.ACCU1 );

            //  Scaling coefficients
            for( i = 0; i < NUM_SCALING_PHASES; i++ )
            {
                OUTREG32( &m_pDispRegs->tDISPC_VID1.aFIR_COEF[i].ulH, pDisplaySaveRestore->tDISPC_VID1.aFIR_COEF[i].ulH );
                OUTREG32( &m_pDispRegs->tDISPC_VID1.aFIR_COEF[i].ulHV, pDisplaySaveRestore->tDISPC_VID1.aFIR_COEF[i].ulHV );
                OUTREG32( &m_pDispRegs->DISPC_VID1_FIR_COEF_V[i], pDisplaySaveRestore->DISPC_VID1_FIR_COEF_V[i] );
            }

            //  Color conversion coefficients
            OUTREG32( &m_pDispRegs->tDISPC_VID1.CONV_COEF0, m_pColorSpaceCoeffs[0] );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.CONV_COEF1, m_pColorSpaceCoeffs[1] );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.CONV_COEF2, m_pColorSpaceCoeffs[2] );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.CONV_COEF3, m_pColorSpaceCoeffs[3] );
            OUTREG32( &m_pDispRegs->tDISPC_VID1.CONV_COEF4, m_pColorSpaceCoeffs[4] );
        }

        //  Restore VID2 plane registers if enabled
        if( g_rgPipelineMapping[OMAP_DSS_PIPELINE_VIDEO2].bEnabled )
        {
            OUTREG32( &m_pDispRegs->tDISPC_VID2.BA0, pDisplaySaveRestore->tDISPC_VID2.BA0 );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.BA1, pDisplaySaveRestore->tDISPC_VID2.BA1 );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.POSITION, pDisplaySaveRestore->tDISPC_VID2.POSITION );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.SIZE, pDisplaySaveRestore->tDISPC_VID2.SIZE );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.ATTRIBUTES, pDisplaySaveRestore->tDISPC_VID2.ATTRIBUTES );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.FIFO_THRESHOLD, pDisplaySaveRestore->tDISPC_VID2.FIFO_THRESHOLD );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.ROW_INC, pDisplaySaveRestore->tDISPC_VID2.ROW_INC );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.PIXEL_INC, pDisplaySaveRestore->tDISPC_VID2.PIXEL_INC );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.FIR, pDisplaySaveRestore->tDISPC_VID2.FIR );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.PICTURE_SIZE, pDisplaySaveRestore->tDISPC_VID2.PICTURE_SIZE );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.ACCU0, pDisplaySaveRestore->tDISPC_VID2.ACCU0 );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.ACCU1, pDisplaySaveRestore->tDISPC_VID2.ACCU1 );

            //  Scaling coefficients
            for( i = 0; i < NUM_SCALING_PHASES; i++ )
            {
                OUTREG32( &m_pDispRegs->tDISPC_VID2.aFIR_COEF[i].ulH, pDisplaySaveRestore->tDISPC_VID2.aFIR_COEF[i].ulH );
                OUTREG32( &m_pDispRegs->tDISPC_VID2.aFIR_COEF[i].ulHV, pDisplaySaveRestore->tDISPC_VID2.aFIR_COEF[i].ulHV );
                OUTREG32( &m_pDispRegs->DISPC_VID2_FIR_COEF_V[i], pDisplaySaveRestore->DISPC_VID2_FIR_COEF_V[i] );
            }

            //  Color conversion coefficients
            OUTREG32( &m_pDispRegs->tDISPC_VID2.CONV_COEF0, m_pColorSpaceCoeffs[0] );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.CONV_COEF1, m_pColorSpaceCoeffs[1] );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.CONV_COEF2, m_pColorSpaceCoeffs[2] );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.CONV_COEF3, m_pColorSpaceCoeffs[3] );
            OUTREG32( &m_pDispRegs->tDISPC_VID2.CONV_COEF4, m_pColorSpaceCoeffs[4] );
        }
    }


    //  Success
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();
#endif

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::SetPowerLevel(
    DWORD dwPowerLevel
    )
{
    BOOL            bResult = TRUE;

#if 0
    DWORD   dwTimeout;

    //  Lock access to power level
    EnterCriticalSection( &m_csPowerLock );

    //  Check if there is a change in the power level
    if( m_dwPowerLevel == dwPowerLevel )
        goto cleanUp;

    //  Enable/disable devices based on power level
    switch( dwPowerLevel )
    {
        case D0:
        case D1:
        case D2:
            //  Check against current level
            if( m_dwPowerLevel == D3 || m_dwPowerLevel == D4 )
            {
                //  Set the new power level
                m_dwPowerLevel = dwPowerLevel;


                //  Enable device clocks
                RequestClock( m_dssinfo.DSSDevice );         

                //  Call PDD layer
                LcdPdd_SetPowerLevel( dwPowerLevel );

                //  Re-enable LCD outputs
                if( g_dwDestinationRefCnt[OMAP_DSS_DESTINATION_LCD] > 0 )
                {
                    // The HDMI uses DSI clock. At init time, DSS is turned
                    // ON and so HDMI cannot be configured at init time.
                    // The HDMI config seq is given below
                    // init -> InternalLCD(DSS) -> HDMI(DSI)

                    // Store the current HDMI state
                    BOOL bHdmiEnable = m_bHDMIEnable;
                    // Force HDMI to inactive state during init
                    m_bHDMIEnable = FALSE;

#if 0
					// Turn on the DSS2_ALWON_FCLK if the FCLK source is DSI clock
                    if ( m_eDssFclkSource == OMAP_DSS_FCLK_DSS2ALWON )
                    {
                        ULONG count = 2;
                        ULONG clockSrc[2] = {kDSS1_ALWON_FCLK, kDSS2_ALWON_FCLK};
                        SelectDSSSourceClocks( count, clockSrc);
                        InitDsiPll();
                    }
#endif
                    //  Restore LCD settings
                    RestoreRegisters(OMAP_DSS_DESTINATION_LCD);

                    // enable interrupt for reporting SYNCLOST errors
                    SETREG32( &m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_SYNCLOST);

                    LcdPdd_SetPowerLevel( dwPowerLevel );

                    // Check the FIFO threshold level and decide if LPR is required
                    DWORD dwFIFOThreshold =
                        (DISPC_GFX_FIFO_THRESHOLD_LOW(FIFO_LOWTHRESHOLD_MERGED(FIFO_BURSTSIZE_16x32))|
                         DISPC_GFX_FIFO_THRESHOLD_HIGH(FIFO_HIGHTHRESHOLD_MERGED));

                    BOOL bLPREnable = FALSE;

                    // Enable the LPR if the FIFO's are merged.
                    if ( INREG32( &m_pDispRegs->DISPC_GFX_FIFO_THRESHOLD) == dwFIFOThreshold )
                    {
                        SETREG32( &m_pDispRegs->DISPC_CONFIG, DISPC_CONFIG_FIFOMERGE);

                        // Make sure the LPR is disabled when HDMI is enabled
                        if (bHdmiEnable == FALSE)
                        {
                            // LPR should be turned ON
                            bLPREnable = TRUE;
                        }
                    }

                    // Toggle the LPR state
                    m_bLPREnable = ( m_bLPREnable == TRUE ) ? FALSE : TRUE;
                    // Turn on LPR and also switch to DSI clock (if clksource == DSI)
                    EnableLPR( bLPREnable );

                    //  Flush shadow registers
                    FlushRegs( DISPC_CONTROL_GOLCD );

                    if (bHdmiEnable)
                    {
                        WaitForFlushDone( DISPC_CONTROL_GOLCD );
                        // Restore the HDMI specific context
                        m_bHDMIEnable = TRUE;
                        RestoreRegisters( OMAP_DSS_DESTINATION_LCD );
                        // Configure and enable the Hdmi Panel
                        EnableHdmi( TRUE );
                    }
                }

                //  Re-enable TV out if it was enabled prior to display power change
                if( m_bTVEnable )
                {
                    //  Enable the video encoder clock
                    RequestClock( m_dssinfo.TVEncoderDevice );

                    //  Restore the TV out registers
                    RestoreRegisters( OMAP_DSS_DESTINATION_TVOUT );

                    //  Enable TV out if there is something to show
                if( g_dwDestinationRefCnt[OMAP_DSS_DESTINATION_TVOUT] > 0 )
                        {
                      // enable interrupt for reporting SYNCLOST errors
                        SETREG32( &m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_SYNCLOSTDIGITAL);
                        // enable the tvout path
                        SETREG32( &m_pDispRegs->DISPC_CONTROL, DISPC_CONTROL_DIGITALENABLE );
                }
                    }

                //  Wait for VSYNC
                dwTimeout = DISPLAY_TIMEOUT;
                OUTREG32(&m_pDispRegs->DISPC_IRQSTATUS, DISPC_IRQSTATUS_VSYNC);
                while (((INREG32(&m_pDispRegs->DISPC_IRQSTATUS) & DISPC_IRQSTATUS_VSYNC) == 0) && (dwTimeout-- > 0))
                {
                    Sleep(1);
                }

                //  Clear all DSS interrupts
                OUTREG32( &m_pDispRegs->DISPC_IRQSTATUS, 0xFFFFFFFF );
            }
			else
			    m_dwPowerLevel = dwPowerLevel;

            break;

        case D3:
        case D4:
            //  Check against current level
            if( m_dwPowerLevel == D0 || m_dwPowerLevel == D1 || m_dwPowerLevel == D2 )
            {

                //  Disable TV out
                if( g_dwDestinationRefCnt[OMAP_DSS_DESTINATION_TVOUT] > 0 )
                {
                    //  Disable TV out control
                    CLRREG32( &m_pDispRegs->DISPC_CONTROL, DISPC_CONTROL_DIGITALENABLE );

                    //  Wait for EVSYNC
                    WaitForIRQ(DISPC_IRQSTATUS_EVSYNC_EVEN|DISPC_IRQSTATUS_EVSYNC_ODD);

                    // clear all the pending interrupts
                    SETREG32( &m_pDispRegs->DISPC_IRQSTATUS, DISPC_IRQSTATUS_EVSYNC_EVEN|DISPC_IRQSTATUS_EVSYNC_ODD);

                    //  Save TV out settings
                    SaveRegisters(OMAP_DSS_DESTINATION_TVOUT);

                    //  Reset the video encoder
                    ResetVENC();

                    //  Release the clock
                    ReleaseClock( m_dssinfo.TVEncoderDevice );
                }

                //  Disable LCD
                if( g_dwDestinationRefCnt[OMAP_DSS_DESTINATION_LCD] > 0 )
                {
                    //  Save LCD settings
                    SaveRegisters(OMAP_DSS_DESTINATION_LCD);

                    //  Disable LCD
                    //CLRREG32( &m_pDispRegs->DISPC_CONTROL, DISPC_CONTROL_LCDENABLE );
                    LcdPdd_SetPowerLevel( dwPowerLevel );

                    //  Wait for the frame to complete
                    WaitForFrameDone();
                }

                //  Call PDD layer
                LcdPdd_SetPowerLevel( dwPowerLevel );


                //  Clear all DSS interrupts
                OUTREG32( &m_pDispRegs->DISPC_IRQSTATUS, 0xFFFFFFFF );

                //  Change interconnect parameters to disable controller
                OUTREG32( &m_pDispRegs->DISPC_SYSCONFIG, DISPC_SYSCONFIG_AUTOIDLE|SYSCONFIG_FORCEIDLE|SYSCONFIG_FORCESTANDBY );

#if 0
				if ( m_eDssFclkSource == OMAP_DSS_FCLK_DSS2ALWON )
                {
                    // De-init the DSI Pll and Power Down the DSI PLL
                    DeInitDsiPll();
                    // Set clock to DSS1
                    ULONG count = 1;
                    ULONG clockSrc = kDSS1_ALWON_FCLK;
                    SelectDSSSourceClocks( count, &clockSrc);
                }
#endif

				//  Disable device clocks 
                ReleaseClock( m_dssinfo.DSSDevice );         
			}	

                //  Set the new power level
                m_dwPowerLevel = dwPowerLevel;

            break;
    }

cleanUp:
    //  Unlock access to power level
    LeaveCriticalSection( &m_csPowerLock );
#endif
    //  Return result
    return bResult;
}

#if 0
//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::EnableHdmi(
    BOOL bEnable
    )
{
    BOOL    bResult = FALSE;

    BOOL    bLPRState   = TRUE;
    
    //  Access the regs
    if( AccessRegs() == FALSE )
        goto cleanUp;

    //  Enable or disable HDMI output
    if( bEnable )
    {
        DWORD   dwX = (1280 - m_dwLcdWidth)/2,
                dwY = (720 - m_dwLcdHeight)/2;

        m_dwPixelClock = OMAP_DSS_FCLKVALUE_HDMI/2;
        
        //Save the configuration of internal LCD
        SaveRegisters( OMAP_DSS_DESTINATION_LCD );

        // Issue Power Down command to Internal LCD Panel
        LcdPdd_SetPowerLevel(D4);
        
        // Disable LPR and configure DSI for HDMI clk
        bLPRState = FALSE;
        EnableLPR( bLPRState, TRUE );

		//
        //  Configure the HDMI timing parameters
        //

        // Timing logic for HSYNC signal
        OUTREG32( &m_pDispRegs->DISPC_TIMING_H,
                    DISPC_TIMING_H_HSW(40) |
                    DISPC_TIMING_H_HFP(110) |
                    DISPC_TIMING_H_HBP(220)
                    );

        // Timing logic for VSYNC signal
        OUTREG32( &m_pDispRegs->DISPC_TIMING_V,
                    DISPC_TIMING_V_VSW(5) |
                    DISPC_TIMING_V_VFP(5) |
                    DISPC_TIMING_V_VBP(20)
                    );

        // Signal configuration
        OUTREG32( &m_pDispRegs->DISPC_POL_FREQ,
                    0
                    );

        // Configures the divisor
        OUTREG32( &m_pDispRegs->DISPC_DIVISOR,
                    DISPC_DIVISOR_PCD(2) |
                    DISPC_DIVISOR_LCD(1)
                    );


        // Configures the panel size
        OUTREG32( &m_pDispRegs->DISPC_SIZE_LCD,
                    DISPC_SIZE_LCD_LPP(720) |
                    DISPC_SIZE_LCD_PPL(1280)
                    );


        //  Center the output
        OUTREG32( &m_pDispRegs->DISPC_GFX_POSITION,
                    DISPC_GFX_POS_GFXPOSX(dwX) |
                    DISPC_GFX_POS_GFXPOSY(dwY)
                    );

        m_bHDMIEnable = TRUE;
    }
    else
    {
        // Get the int LCD params
        LcdPdd_LCD_GetMode( (DWORD*) &m_eLcdPixelFormat,
                              &m_dwLcdWidth,
                              &m_dwLcdHeight,
                              &m_dwPixelClock
                              );
        
        // Clear the HDMI at the beginning.
        // Required for restoring IntLCD context
        m_bHDMIEnable = FALSE;

        // Change the power state to LCD panel
        LcdPdd_SetPowerLevel(D0);

        // Restore internal LCD configurations
        RestoreRegisters(OMAP_DSS_DESTINATION_LCD);

        // Enable the LPR if the FIFO's are merged.
        if ( (INREG32( &m_pDispRegs->DISPC_CONFIG) & DISPC_CONFIG_FIFOMERGE ) )
            bLPRState = TRUE;
        else
            bLPRState = FALSE;

        // Restore LPR if enabled and configure DSI to IntLCD FCLK
        EnableLPR( bLPRState, FALSE );
    }
    
    //  Flush shadow registers
    FlushRegs( DISPC_CONTROL_GOLCD );

    //  Enable the LCD
    SETREG32( &m_pDispRegs->DISPC_CONTROL, DISPC_CONTROL_LCDENABLE );

    //  Success
    bResult = TRUE;

cleanUp:
    //  Release regs
    ReleaseRegs();

    //  Return result
    return bResult;
}
#endif

//------------------------------------------------------------------------------
BOOL OMAPDisplayController::DVISelect(
    BOOL bSelectDVI
    )
{
#if 0
    LcdPdd_DVI_Select(bSelectDVI);
#endif
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::EnableDVI(
    BOOL bEnable
    )
{
    BOOL    bResult = FALSE;

#if 0
    //  Enable/disable DVI
    if ( bEnable )
    {
        //  Enable DVI
        LcdPdd_SetPowerLevel(D4);
        LcdPdd_DVI_Select(TRUE);
        LcdPdd_SetPowerLevel(D0);
        m_bDVIEnable = TRUE;
    }
    else
    {
        //  Disable DVI
        LcdPdd_SetPowerLevel(D4);
        LcdPdd_DVI_Select(FALSE);
        LcdPdd_SetPowerLevel(D0);
        m_bDVIEnable = FALSE;
    }
#endif

    //  Success
    bResult = TRUE;

    //  Return result
    return bResult;
}


//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::ResetDSS()
{
#if 0
    DWORD   dwTimeout;
    DWORD   dwVal;

    //  Need to enable DSS1, DSS2 and TVOUT to reset controller
    RequestClock( m_dssinfo.DSSDevice );         
    RequestClock( m_dssinfo.TVEncoderDevice );

    // check if digital output or the lcd output are enabled
    dwVal = INREG32(&m_pDispRegs->DISPC_CONTROL);

    if(dwVal & (DISPC_CONTROL_DIGITALENABLE | DISPC_CONTROL_LCDENABLE))
    {
        // disable the lcd output and digital output
        dwVal &= ~(DISPC_CONTROL_DIGITALENABLE | DISPC_CONTROL_LCDENABLE);
        OUTREG32(&m_pDispRegs->DISPC_CONTROL, dwVal);

        // wait until frame is done
        WaitForFrameDone(DISPLAY_TIMEOUT);
    }


    //  Reset the whole display subsystem
    SETREG32( &m_pDSSRegs->DSS_SYSCONFIG, DSS_SYSCONFIG_SOFTRESET );

    //  Wait until reset completes OR timeout occurs
    dwTimeout=DISPLAY_TIMEOUT;
    while(((INREG32(&m_pDSSRegs->DSS_SYSSTATUS) & DSS_SYSSTATUS_RESETDONE) == 0) && (dwTimeout-- > 0))
    {
        // delay
        Sleep(1);
    }

    if( dwTimeout == 0 )
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OMAPDisplayController::ResetDSS: "
             L"Failed reset DSS\r\n"
            ));
    }

    //  Release clocks
    ReleaseClock( m_dssinfo.TVEncoderDevice );
    ReleaseClock( m_dssinfo.DSSDevice );         
        
    //  Return result
    return (dwTimeout > 0);
#else
    return 1;
#endif
}

//------------------------------------------------------------------------------
VOID   
OMAPDisplayController::EnableOverlayOptimization(BOOL bEnable)
{
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::ResetDISPC()
{
#if 0
    DWORD   dwVal;
    DWORD   dwTimeout;


    //  Need to enable DSS1, DSS2 and TVOUT to reset controller
    RequestClock( m_dssinfo.DSSDevice );         

    // check if digital output or the lcd output are enabled
    dwVal = INREG32(&m_pDispRegs->DISPC_CONTROL);

    if(dwVal & (DISPC_CONTROL_DIGITALENABLE | DISPC_CONTROL_LCDENABLE))
    {
        // disable the lcd output and digital output
        dwVal &= ~(DISPC_CONTROL_DIGITALENABLE | DISPC_CONTROL_LCDENABLE);
        OUTREG32(&m_pDispRegs->DISPC_CONTROL, dwVal);

        // wait until frame is done
        WaitForFrameDone(DISPLAY_TIMEOUT);
    }


    //  Reset the controller
    SETREG32( &m_pDispRegs->DISPC_SYSCONFIG, DISPC_SYSCONFIG_SOFTRESET );

    //  Wait until reset completes OR timeout occurs
    dwTimeout=DISPLAY_TIMEOUT;
    while(((INREG32(&m_pDispRegs->DISPC_SYSSTATUS) & DISPC_SYSSTATUS_RESETDONE) == 0) && (dwTimeout-- > 0))
    {
        // delay
        Sleep(1);
    }

    if( dwTimeout == 0 )
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OMAPDisplayController::ResetDISPC: "
             L"Failed reset DISPC\r\n"
            ));
    }

    //  Release clocks
    ReleaseClock( m_dssinfo.DSSDevice );         

    //  Return result
    return (dwTimeout > 0);
#else
    return 1;
#endif
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::ResetVENC()
{
#if 0
    DWORD   dwTimeout;


    //  Need to enable DSS1, DSS2 and TVOUT to reset video encoder
    RequestClock( m_dssinfo.TVEncoderDevice );         


    //  Reset the video encoder
    SETREG32( &m_pVencRegs->VENC_F_CONTROL, VENC_F_CONTROL_RESET );

    //  Wait until reset completes OR timeout occurs
    dwTimeout=DISPLAY_TIMEOUT;
    while(((INREG32(&m_pVencRegs->VENC_F_CONTROL) & VENC_F_CONTROL_RESET) == 0) && (dwTimeout-- > 0))
    {
        // delay
        Sleep(1);
    }

    if( dwTimeout == 0 )
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OMAPDisplayController::ResetVENC: "
             L"Failed reset DSS\r\n"
            ));
    }


    //  Clear video encoder F-control and SYNC Control regsiters
    OUTREG32( &m_pVencRegs->VENC_F_CONTROL, 0 );
    OUTREG32( &m_pVencRegs->VENC_SYNC_CTRL, 0 );


    //  Release clocks
    ReleaseClock( m_dssinfo.TVEncoderDevice );         

    //  Return result
    return (dwTimeout > 0);
#else
    return 1;
#endif
}

//-------------------------------------------------------------------------------
VOID
OMAPDisplayController::EnableLPR(BOOL bEnable, BOOL bHdmiEnable)
{
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::AccessRegs()
{
    BOOL    bResult = FALSE;

    //  Ensures that DSS regs can be accessed at current power level
    //  Locks power level at current level until ReleaseRegs called
    //  Returns FALSE if power level is too low to access regs

    //  Lock access to power level
    EnterCriticalSection( &m_csPowerLock );

    //  Check power level
    switch( m_dwPowerLevel )
    {
        case D0:
        case D1:
        case D2:
            //  Clocks are on at this level
            bResult = TRUE;
            break;

        case D3:
        case D4:
            //  Clocks are off at this level
            bResult = FALSE;
            break;
    }

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::ReleaseRegs()
{
    //  Releases power lock
    LeaveCriticalSection( &m_csPowerLock );
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::FlushRegs(
    DWORD   dwDestGo
    )
{
    DWORD   dwTimeout = DISPLAY_TIMEOUT;

    //  Ensure that registers can be flushed
    while(((INREG32(&m_pDispRegs->DISPC_CONTROL) & dwDestGo) == dwDestGo) &&  (dwTimeout-- > 0))
    {
        // delay
        StallExecution(10);
    }

    if( dwTimeout == 0 )
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OMAPDisplayController::FlushRegs: "
             L"Failed to flush regs\r\n"
            ));
    }

    //  Flush the shadow registers
    SETREG32( &m_pDispRegs->DISPC_CONTROL, dwDestGo );


    //  Return result
    return (dwTimeout > 0);
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::WaitForFlushDone(
    DWORD   dwDestGo
    )
{
    DWORD dwTimeout = DISPLAY_TIMEOUT;

    //  Ensure that registers can be flushed
    while(((INREG32(&m_pDispRegs->DISPC_CONTROL) & dwDestGo) == dwDestGo) &&  (dwTimeout-- > 0))
    {
        // delay = 1ms
        Sleep(1);
    }

    if( dwTimeout == 0 )
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OMAPDisplayController::FlushRegs: "
             L"Failed to flush regs\r\n"
            ));
    }

    return (dwTimeout > 0);

}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::WaitForFrameDone(
    DWORD dwTimeout
    )
{
    //  Call WaitForIRQ for FRAMEDONE
    return WaitForIRQ(DISPC_IRQSTATUS_FRAMEDONE, dwTimeout);
}

//------------------------------------------------------------------------------
BOOL
OMAPDisplayController::WaitForIRQ(
    DWORD dwIRQ,
    DWORD dwTimeout
    )
{
    if(AccessRegs() == FALSE)
    {
        goto cleanup;
    }


    //  Wait for VYSNC status
    SETREG32( &m_pDispRegs->DISPC_IRQSTATUS, dwIRQ );
    while(((INREG32(&m_pDispRegs->DISPC_IRQSTATUS) & dwIRQ) == 0) && (dwTimeout-- > 0))
    {
        // delay
        Sleep(1);
    }

    if( dwTimeout == 0 )
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OMAPDisplayController::WaitForIRQ: "
             L"IRQ = 0x%X failed to happen before timeout\r\n", dwIRQ
            ));
    }

    //  Clear the status
    SETREG32( &m_pDispRegs->DISPC_IRQSTATUS, dwIRQ );

cleanup:

    ReleaseRegs();

    //  Status
    return TRUE;
}

//------------------------------------------------------------------------------
DWORD
OMAPDisplayController::PixelFormatToPixelSize(
    OMAP_DSS_PIXELFORMAT    ePixelFormat
)
{
    DWORD   dwResult = 1;

    //  Convert pixel format into bytes per pixel
    switch( ePixelFormat )
    {
        case OMAP_DSS_PIXELFORMAT_RGB16:
        case OMAP_DSS_PIXELFORMAT_ARGB16:
        case OMAP_DSS_PIXELFORMAT_YUV2:
        case OMAP_DSS_PIXELFORMAT_UYVY:
            //  2 bytes per pixel
            dwResult = 2;
            break;

        case OMAP_DSS_PIXELFORMAT_RGB32:
        case OMAP_DSS_PIXELFORMAT_ARGB32:
        case OMAP_DSS_PIXELFORMAT_RGBA32:
            //  4 bytes per pixel
            dwResult = 4;
            break;
    }

    //  Return result
    return dwResult;
}

//------------------------------------------------------------------------------
BOOL OMAPDisplayController::EnableVSyncInterruptEx()
{
    BOOL bInterruptAlreadyEnabled = FALSE;
    DWORD irqEnableStatus;

    if (!m_dwEnableWaitForVerticalBlank)
       return FALSE;

    if(AccessRegs() == FALSE)
    {
        goto cleanup;
    }

    irqEnableStatus = INREG32(&m_pDispRegs->DISPC_IRQENABLE);
    bInterruptAlreadyEnabled = ((irqEnableStatus & DISPC_IRQENABLE_VSYNC) == DISPC_IRQENABLE_VSYNC) ||
                                ((irqEnableStatus & DISPC_IRQSTATUS_EVSYNC_EVEN) == DISPC_IRQSTATUS_EVSYNC_EVEN) ||
                                ((irqEnableStatus & DISPC_IRQSTATUS_EVSYNC_ODD) == DISPC_IRQSTATUS_EVSYNC_ODD);



    SETREG32(&m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_VSYNC);
    if(m_bTVEnable == TRUE)
    {
        SETREG32(&m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_EVSYNC_EVEN | DISPC_IRQENABLE_EVSYNC_ODD);
    }


cleanup:

    ReleaseRegs();
    return bInterruptAlreadyEnabled;
}

//------------------------------------------------------------------------------
void OMAPDisplayController::EnableVSyncInterrupt()
{
    if(AccessRegs() == FALSE)
    {
        goto cleanup;
    }

    SETREG32(&m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_VSYNC);
    if(m_bTVEnable == TRUE)
    {
        SETREG32(&m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_EVSYNC_EVEN | DISPC_IRQENABLE_EVSYNC_ODD);
    }

cleanup:

    ReleaseRegs();
}

//------------------------------------------------------------------------------
VOID OMAPDisplayController::DisableVSyncInterrupt()
{
    if(AccessRegs() == FALSE)
    {
        goto cleanup;
    }

    CLRREG32(&m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_VSYNC);
    if(m_bTVEnable == TRUE)
    {
        CLRREG32(&m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_EVSYNC_EVEN | DISPC_IRQENABLE_EVSYNC_ODD);
    }

cleanup:

    ReleaseRegs();
}

//------------------------------------------------------------------------------
BOOL OMAPDisplayController::InVSync(BOOL bClearStatus)
{
    BOOL bInVSync = FALSE;
    DWORD irqStatus = 0;
    BOOL  lcdVsync = FALSE;
    //Alwasy set to true, in case tv-out is disabled.
    BOOL  tvVsync = TRUE;

    if (!m_dwEnableWaitForVerticalBlank)
        return TRUE;

    if(AccessRegs() == FALSE)
    {
        bInVSync = TRUE;
        goto cleanup;
    }

    irqStatus = INREG32(&m_pDispRegs->DISPC_IRQSTATUS);
    lcdVsync = (irqStatus & DISPC_IRQSTATUS_VSYNC) == DISPC_IRQSTATUS_VSYNC;
    if(m_bTVEnable == TRUE)
    {
        tvVsync = ((irqStatus & DISPC_IRQSTATUS_EVSYNC_EVEN) == DISPC_IRQSTATUS_EVSYNC_EVEN) ||
                  ((irqStatus & DISPC_IRQSTATUS_EVSYNC_ODD) == DISPC_IRQSTATUS_EVSYNC_ODD);

    }
    //If tv-out is enabled we also need to check of it's VSYNC signal. Once both have been asserted then
    //we can say that Vsync has occurred.
    if( lcdVsync && tvVsync)
    {
        bInVSync = TRUE;

        if(bClearStatus)
        {
            SETREG32(&m_pDispRegs->DISPC_IRQSTATUS, DISPC_IRQSTATUS_VSYNC);
            if(m_bTVEnable == TRUE)
            {
                SETREG32(&m_pDispRegs->DISPC_IRQSTATUS, DISPC_IRQSTATUS_EVSYNC_EVEN);
                SETREG32(&m_pDispRegs->DISPC_IRQSTATUS, DISPC_IRQSTATUS_EVSYNC_ODD);
            }
        }
    }

cleanup:

    ReleaseRegs();

    return bInVSync;
}

//------------------------------------------------------------------------------
VOID OMAPDisplayController::WaitForVsync()
{
//    BOOL bVsyncPreviouslyEnabled = FALSE;

    if (!m_dwEnableWaitForVerticalBlank)
        return;

#if 0
    if(!InVSync(TRUE))
    {
        bVsyncPreviouslyEnabled = EnableVSyncInterruptEx();
        WaitForSingleObject(m_hVsyncEvent, m_dwVsyncPeriod);
        //SGX may have turned on the vsync interrupt, keep it on if that's the case.
        if(!bVsyncPreviouslyEnabled)
        {
            DisableVSyncInterrupt();
        }
    }
#endif
}

//------------------------------------------------------------------------------
VOID OMAPDisplayController::EnableScanLineInterrupt(DWORD dwLineNumber)
{
    if (!m_dwEnableWaitForVerticalBlank)
        return;

    if(AccessRegs() == FALSE)
    {
        goto cleanup;
    }

    //  Program line number to interrupt on
    if(INREG32(&m_pDispRegs->DISPC_LINE_NUMBER) != dwLineNumber)
    {
        OUTREG32(&m_pDispRegs->DISPC_LINE_NUMBER, dwLineNumber);
        FlushRegs(DISPC_CONTROL_GOLCD);
    }

    //  Enable
    SETREG32(&m_pDispRegs->DISPC_IRQSTATUS, DISPC_IRQSTATUS_PROGRAMMEDLINENUMBER);
    SETREG32(&m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_PROGRAMMEDLINENUMBER);

cleanup:
    ReleaseRegs();
}

//------------------------------------------------------------------------------
VOID OMAPDisplayController::DisableScanLineInterrupt()
{
    if (!m_dwEnableWaitForVerticalBlank)
        return;

    if(AccessRegs() == FALSE)
    {
        goto cleanup;
    }

    //  Disable interrupt
    SETREG32(&m_pDispRegs->DISPC_IRQSTATUS, DISPC_IRQSTATUS_PROGRAMMEDLINENUMBER);
    CLRREG32(&m_pDispRegs->DISPC_IRQENABLE, DISPC_IRQENABLE_PROGRAMMEDLINENUMBER);

cleanup:
    ReleaseRegs();
}

//------------------------------------------------------------------------------
DWORD OMAPDisplayController::GetScanLine()
{
    DWORD scanLine = 0;
    if(AccessRegs() == FALSE)
    {
        goto cleanup;
    }

    //  Get current scanline value
    scanLine = INREG32(&m_pDispRegs->DISPC_LINE_STATUS);

cleanup:
    ReleaseRegs();
    return scanLine;
}

//------------------------------------------------------------------------------
VOID OMAPDisplayController::WaitForScanLine(DWORD dwLineNumber)
{
    if (!m_dwEnableWaitForVerticalBlank)
        return;

    //  Enable the scanline interrupt for the given line number and wait
    EnableScanLineInterrupt(dwLineNumber);
    WaitForSingleObject(m_hScanLineEvent, m_dwVsyncPeriod);
    DisableScanLineInterrupt();
}

//------------------------------------------------------------------------------
BOOL OMAPSurface::SetClipping(RECT* pClipRect )
{
    BOOL    bResult;
    RECT    rcSurf;

    //  Set the rect of the entire surface
    rcSurf.left = 0;
    rcSurf.top = 0;
    rcSurf.right = m_dwWidth;
    rcSurf.bottom = m_dwHeight;

    //  Set the clipping region of the surface
    if( pClipRect == NULL )
    {
        //  No clipping; use entire surface size
        m_rcClip = rcSurf;
        bResult = TRUE;
    }
    else
    {
        //  Find intersection of surface rect and clipping rect
        bResult = IntersectRect( &m_rcClip, &rcSurf, pClipRect );
    }

    UpdateClipping(pClipRect);

    //  Update the given clipping rect
    if( pClipRect )
        *pClipRect = m_rcClip;

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL OMAPSurface::UpdateClipping( RECT* pClipRect )
{
    BOOL bResult = FALSE;

    // Could change to to ensure rectangle alignment with different
    // scale and decimation factors...

    //Force the clipping rectangle to fall in a pack pixel boundary
    if (pClipRect != NULL)
    {
        AdjustClippingRect(&m_rcClip, 2, 2);
    }

    bResult = TRUE;
    return bResult;
}

//------------------------------------------------------------------------------
RECT OMAPSurface::GetClipping()
{
    return m_rcClip;
}

//------------------------------------------------------------------------------
BOOL OMAPSurface::AdjustClippingRect(RECT *srcRect, UINT8 horzValue, UINT8 vertValue)
{
    BOOL bResult = FALSE;

    if(srcRect == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%S: ERROR: Null rectangle passed!\r\n"), __FUNCTION__));
        return bResult;
    }
    if(vertValue > 1)
    {
        if( ((srcRect->top)%vertValue)!= 0)
            CEIL_MULT(srcRect->top, vertValue);
        if( ((srcRect->bottom)%vertValue)!= 0)
            FLOOR_MULT(srcRect->bottom, vertValue);
    }
    if(horzValue > 1)
    {
        if( ((srcRect->left)%horzValue)!= 0)
            CEIL_MULT(srcRect->left, horzValue);
        if( ((srcRect->right)%horzValue)!= 0)
            FLOOR_MULT(srcRect->right, horzValue);
    }

    bResult = TRUE;
    return bResult;

}

//------------------------------------------------------------------------------
BOOL OMAPSurface::SetHorizontalScaling(DWORD dwScaleFactor)
{
    BOOL    bResult;

    //  Validate scaling factor
    switch( dwScaleFactor )
    {
        case 1:
        case 2:
        case 4:
        case 8:
            //  Valid scaling factors
            m_dwHorizScale = dwScaleFactor;
            bResult = TRUE;
            break;

        default:
            //  Invalid
            bResult = FALSE;
            break;
    }

    //  Return result
    return bResult;
}

//------------------------------------------------------------------------------
BOOL OMAPSurface::SetVerticalScaling(DWORD dwScaleFactor)
{
    BOOL    bResult;

    //  Validate scaling factor
    switch( dwScaleFactor )
    {
        case 1:
        case 2:
        case 4:
        case 8:
            //  Valid scaling factors
            m_dwVertScale = dwScaleFactor;
            bResult = TRUE;
            break;

        default:
            //  Invalid
            bResult = FALSE;
            break;
    }

    //  Return result
    return bResult;
}


