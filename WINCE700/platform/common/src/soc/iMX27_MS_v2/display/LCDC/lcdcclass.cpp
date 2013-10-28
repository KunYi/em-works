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

//------------------------------------------------------------------------------
// INCLUDE FILES
//------------------------------------------------------------------------------
#include <windows.h>
#include <winddi.h>
#include <ceddk.h>
#include <gpe.h>
#include <emul.h>
#include <Winreg.h>
#include <Pwingdi.h>
#include <pm.h>

#include "csp.h"
#include "lcdc.h"
#include "LcdcClass.h"
//------------------------------------------------------------------------------
// GLOBAL DEFINITIONS
//------------------------------------------------------------------------------
// The dpCurSettings structure
/* Start with Errors, warnings, and temporary messages */
INSTANTIATE_GPE_ZONES((GPE_ZONE_ERROR|GPE_ZONE_INIT),"DDI Driver","unused1","unused2")

//------------------------------------------------------------------------------
// GLOBAL OR STATIC VARIABLES
//------------------------------------------------------------------------------
static GPE *gGPE  = (GPE *) NULL;

static DHPDEV (*pOldDrvEnablePDEV)(
      DEVMODEW *pdm,
      LPWSTR    pwszLogAddress,
      ULONG     cPat,
      HSURF    *phsurfPatterns,
      ULONG     cjCaps,
      ULONG    *pdevcaps,
      ULONG     cjDevInfo,
      DEVINFO  *pdi,
      HDEV      hdev,
      LPWSTR    pwszDeviceName,
      HANDLE    hDriver );


//------------------------------------------------------------------------------
// STATIC FUNCTION PROTOTYPES
//------------------------------------------------------------------------------
// This prototype avoids problems exporting from .lib
BOOL APIENTRY GPEEnableDriver( ULONG engineVersion,
                   ULONG cj,
                   DRVENABLEDATA *data,
                   PENGCALLBACKS  engineCallbacks);

//------------------------------------------------------------------------------
// EXPORTED FUNCTIONS
//------------------------------------------------------------------------------
//------------------------------------------------------------------------
// Function:  Tell GDI that the 256 Palettes are not settable, as
//            MC9328LCD is a fixed palette device.
//------------------------------------------------------------------------
DHPDEV LcdcDrvEnablePDEV
   (
      DEVMODEW *pdm,
      LPWSTR    pwszLogAddress,
      ULONG     cPat,
      HSURF    *phsurfPatterns,
      ULONG     cjCaps,
      ULONG    *pdevcaps,
      ULONG     cjDevInfo,
      DEVINFO  *pdi,
      HDEV      hdev,
      LPWSTR    pwszDeviceName,
      HANDLE    hDriver
   )
{
   DHPDEV   pPDEV;

   pPDEV=(*pOldDrvEnablePDEV) (pdm,
                        pwszLogAddress,
                        cPat,
                        phsurfPatterns,
                        cjCaps,
                        pdevcaps,
                        cjDevInfo,
                        pdi,
                        hdev,
                        pwszDeviceName,
                        hDriver );

   BSPSetDevCaps((GDIINFO *)pdevcaps);

   return (pPDEV);
}

//------------------------------------------------------------------------------
//
//  FUNCTION:     DrvEnableDriver
//
//  DESCRIPTION:  This function is the initial driver entry point exported
//        by the driver DLL for devices that link directly to GWES
//
//  PARAMETERS:
//
//  RETURNS:
//
//------------------------------------------------------------------------------

BOOL APIENTRY DrvEnableDriver( ULONG engineVersion,
                   ULONG cj,
                   DRVENABLEDATA *data,
                   PENGCALLBACKS  engineCallbacks)
{
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("LCDClass::DrvEnableDriver\r\n")));


    if(!GPEEnableDriver(engineVersion, cj, data, engineCallbacks))
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("GPEEnableDriver return FALSE!\r\n")));

    // Tell GDI palette is not settable.
    pOldDrvEnablePDEV = data->DrvEnablePDEV;
    data->DrvEnablePDEV = LcdcDrvEnablePDEV;

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:     DrvGetMasks
//
//  DESCRIPTION:
//
//  PARAMETERS:
//
//  RETURNS:
//
////------------------------------------------------------------------------------
ULONG *APIENTRY DrvGetMasks(DHPDEV dhpdev)
{
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("LCDClass::DrvGetMasks\r\n")));
    //return BitMasks;
    return BSPGetLCDCBitMasks();
}

//------------------------------------------------------------------------------
//
//  FUNCTION:     RegisterDDHALAPI
//
//  DESCRIPTION:
//
//  PARAMETERS:
//
//  RETURNS:
//
//------------------------------------------------------------------------------
void RegisterDDHALAPI(void)
{
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("LCDClass::RegisterDDHALAPI\r\n")));
    return; // no DDHAL support
}

//------------------------------------------------------------------------------
// PRIVATE FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  FUNCTION:     GetGPE
//
//  DESCRIPTION:  Main entry point for a GPE-compliant driver.
//
//  PARAMETERS:
//
//  RETURNS:
//        GPE class pointer
//
//------------------------------------------------------------------------------
GPE *GetGPE(void)
{
    if (!gGPE)
    {
        gGPE = new LcdcClass;
    }

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("LCDClass::GetGPE(0x%08x)\r\n"), gGPE));
    return gGPE;
}




//------------------------------------------------------------------------------
//
//  FUNCTION:     LcdcClass
//
//  DESCRIPTION:  Constructor of LcdcClass
//        Map the linear frame buffer
//        Map the the LCDC module space for user access
//
//  PARAMETERS:
//
//  RETURNS:
//------------------------------------------------------------------------------
LcdcClass::LcdcClass(void)
{
    BOOL initState;
    PHYSICAL_ADDRESS phyAddr;
    DDK_GPIO_CFG cfg;

    // Enable LCDC pins
    DDK_GPIO_SET_CONFIG(cfg, LCDC);
    initState = DDKGpioEnable(&cfg);
    if(initState != TRUE)
    {
        ERRORMSG(1, (TEXT("LCDC Class: LCDC pins enable failed!\r\n")));
        return;
    }

    // Enable LCDC hw clocks
    initState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_ENABLE);
    if(initState != TRUE)
    {
        ERRORMSG(1, (TEXT("LCDC Class: LCDC hw clock enable failed!\r\n")));
        return;
    }
    initState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_HCLK_LCDC, DDK_CLOCK_GATE_MODE_ENABLE);
    if(initState != TRUE)
    {
        ERRORMSG(1, (TEXT("LCDC Class: LCDC AHB hw clock enable failed!\r\n")));
        return;
    }

    BSPInitLCDC(&m_LcdcCtx);
    // setup local mode info structure
    m_ModeInfo  = *(GPEMode*)m_LcdcCtx.pGPEModeInfo;

    m_VideoFrameSize = m_LcdcCtx.VideoFrameWidth * m_LcdcCtx.VideoFrameHeight * BSPGetPixelSizeInByte();
    m_nVideoMemorySize = m_LcdcCtx.VideoMemorySize;
    phyAddr.QuadPart = m_LcdcCtx.VideoMemoryPhyAdd;
    m_pLinearVideoMem = (UINT8 *)MmMapIoSpace(phyAddr, m_nVideoMemorySize, FALSE);
    if (m_pLinearVideoMem == NULL)
    {
        DEBUGMSG (GPE_ZONE_ERROR, (TEXT("Frame Buffer MmMapIoSpace failed!\r\n")) );
        return;
    }

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("Actual memory detected: %d bytes\r\n"), m_nVideoMemorySize));

    // allocate blank frame buffer for backing up the frame buffer in power handler
    m_pBlankVideoMem = (UINT8*)malloc(m_VideoFrameSize);
    if(!m_pBlankVideoMem)
    {
        ERRORMSG(1, (TEXT(" buffers Alloc Failed!\r\n")));
        return;
    }

    // set blank frame buffer as white
    memset( (PVOID)m_pBlankVideoMem, 0xFF, m_VideoFrameSize);

    //Mem maps the LCDC module space for user access
    phyAddr.QuadPart = CSP_BASE_REG_PA_LCDC;
    m_pLCDC = (CSP_LCDC_REGS *)MmMapIoSpace(phyAddr, sizeof(CSP_LCDC_REGS), FALSE);
    if (m_pLCDC  ==  NULL)
    {
        DEBUGMSG (GPE_ZONE_ERROR, (TEXT("LcdcClass: VirtualAlloc failed!\r\n")) );
        return;
    }

    // Notify the system that we are a power-manageable device
    m_Dx = D0;
    AdvertisePowerInterface(g_hmodDisplayDll);

     //pass mode information to protected variables
    m_pMode      =   (GPEMode*)m_LcdcCtx.pGPEModeInfo;
    m_nScreenWidth   =    m_pMode->width;
    m_nScreenHeight  =    m_pMode->height;
    m_nScreenStride  =   (m_nScreenWidth * m_pMode->Bpp) / 8;

    m_iRotate = GetRotateModeFromReg();
    SetRotateParms();

    m_pMode->width = m_nScreenWidth;
    m_pMode->height = m_nScreenHeight;

    //create an instance of Node2D class to allocate memory as single rectangular blocks
    m_p2DVideoMemory = new Node2D(m_nScreenStride * 8UL / m_pMode->Bpp, m_nVideoMemorySize/m_nScreenStride);


    if(!m_p2DVideoMemory)
    {
        DEBUGMSG(GPE_ZONE_ERROR,
                 (TEXT("LcdcClass: error creating new Node2D!\r\n")));
        return;
    }

    // for the inserted software cursor
    m_colorDepth = m_ModeInfo.Bpp;

    if(FAILED(AllocSurface(&m_pPrimarySurface, m_nScreenWidthSave, m_nScreenHeightSave, m_pMode->format, GPE_REQUIRE_VIDEO_MEMORY)))
    {
       DEBUGMSG(GPE_ZONE_ERROR, (TEXT("Couldn't allocate primary surface\n")));
          return;
     }
    DEBUGMSG(GPE_ZONE_INIT,(TEXT("Screen Width  : %d\r\n"), m_nScreenWidth));
    DEBUGMSG(GPE_ZONE_INIT,(TEXT("Screen Height : %d\r\n"), m_nScreenHeight));
    DEBUGMSG(GPE_ZONE_INIT,(TEXT("Screen rotation : %x\r\n"), m_iRotate));

    ((GPESurf*)m_pPrimarySurface)->SetRotation(m_nScreenWidth,
                                                      m_nScreenHeight,
                                                      m_iRotate);

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("lcdcClass: Constructor completed!! \r\n")));
}

//------------------------------------------------------------------------------
//
//  FUNCTION:     ~LcdcClass
//
//  DESCRIPTION:  Destructor of LcdcClass
//        Free the linear frame buffer
//        Free the the LCDC module space for user access
//
//  PARAMETERS:
//
//  RETURNS:

//------------------------------------------------------------------------------

LcdcClass::~LcdcClass(void)
{
    DDK_GPIO_CFG cfg;

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("LCDClass::~LcdcClass\r\n")));

    // Free memory
    if(m_pPrimarySurface)
        delete m_pPrimarySurface;
    if(m_p2DVideoMemory)
        delete m_p2DVideoMemory;

    // free frame buffer
    if (m_pLinearVideoMem != NULL)
    {
        // blank display memory
        memset (m_pLinearVideoMem, 0xFF, m_VideoFrameSize);
        MmUnmapIoSpace(m_pLinearVideoMem, m_nVideoMemorySize);
        m_pLinearVideoMem = NULL;
    }

    // Free the blank frame buffer
    if(m_pBlankVideoMem)
    {
        free((void*)m_pBlankVideoMem);
        m_pBlankVideoMem = NULL;
    }

    //free mapped LCDC module space
    if (m_pLCDC != NULL)
    {
        // disable LCDC
        INSREG32BF(&m_pLCDC->RMCR, LCDC_RMCR_SELF_REF, LCDC_RMCR_SELF_REF_DISABLE);
        MmUnmapIoSpace(m_pLCDC, sizeof(CSP_LCDC_REGS));
        m_pLCDC  =   NULL;
    }

    // Disable LCDC hw clocks
    DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_DISABLE);


    // Disable LCDC module pins
    DDK_GPIO_SET_CONFIG(cfg, LCDC);
    DDKGpioDisable(&cfg);

    BSPDeinitLCDC(&m_LcdcCtx);
    //free all static objects
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("lcdcClass: Destructor completed!! \r\n")));
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     SetMode
//
//  DESCRIPTION:  This method executes to enable the device driver
//                based on the Graphics Primitive Engine (GPE)
//                and to request a specific mode for the display.
//
//  PARAMETERS:
//        modeID        [in] Mode number to set.
//        pPalette          Handle of palette
//
//  RETURNS:
//        S_OK              successful
//                others            failed
//
//------------------------------------------------------------------------------
SCODE LcdcClass::SetMode(int modeId, HPALETTE *pPalette)
{
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("Tahiti LcdcClass::SetMode(%d,0x%x)\r\n"), modeId, *pPalette));

    BSPSetMode(modeId, pPalette);

    return S_OK;                // Mode is inherently set
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     GetModeInfo
//
//  DESCRIPTION:  This method populates a GPEMode structure with data for the requested mode.
//
//  PARAMETERS:
//            pMode     [out]  Pointer to a GPEMode structure.
//        modeNo    [in]   Integer specifying the mode to return information about.
//
//  RETURNS:
//        S_OK              successful
//                others            failed
//
//------------------------------------------------------------------------------
SCODE LcdcClass::GetModeInfo(GPEMode * pMode, int modeNo)
{
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("LCDClass::GetModeInfo(%d)\r\n"), modeNo));

    if( modeNo<0 || modeNo >= NumModes() )
        return E_INVALIDARG;

    BSPGetModeInfo(pMode, modeNo);
    return S_OK;
}



//------------------------------------------------------------------------------
//
//  FUNCTION:     GetModeInfo
//
//  DESCRIPTION:  This method returns the number of
//                display modes supported by a driver.
//
//  PARAMETERS:
//
//  RETURNS:
//        The number of supported display modes
//
//------------------------------------------------------------------------------
int  LcdcClass::NumModes()
{
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("LCDClass::NumModes\r\n")));
    return BSPGetModeNum();
}



//------------------------------------------------------------------------------
//
//  FUNCTION:      PowerHandler
//
//  DESCRIPTION:   Turn on/off LCDC
//
//  PARAMETERS:
//         boff   switch status
//
//  RETURNS:
//
//------------------------------------------------------------------------------
VOID LcdcClass::PowerHandler(BOOL bOff)
{

}

#ifdef USE_V1_CODE

inline BOOL IsSurface_R5_G6_B5( GPESurf* pSurf )
{
    if ( ( pSurf == NULL ) ||
         ( pSurf->Format() != gpe16Bpp ) )
    {
        return FALSE;
    }

    GPEFormat* pFormat = pSurf->FormatPtr();

    if ( ( pFormat == NULL ) ||
        ( pFormat->m_PaletteEntries != 3 ) ||
        ( pFormat->m_pPalette[0] != 0xf800 ) ||
        ( pFormat->m_pPalette[1] != 0x07e0 ) ||
        ( pFormat->m_pPalette[2] != 0x001f ) )
    {
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
    CanUse_Blt_NoStretch_NoTrans_16To16

    Returns TRUE if it's okay to use the specified blt.

    Parameters:
    pParms: Blt parameters.
------------------------------------------------------------------------------*/
inline BOOL CanUse_Blt_NoStretch_NoTrans_16To16( GPEBltParms *pParms )
{
    // Do parameter validation
    if ( ( pParms->rop4 != 0xcccc ) ||
        ( pParms->solidColor == -1 ) ||
        ( pParms->pSrc == NULL ) ||
        ( pParms->bltFlags & (BLT_TRANSPARENT | BLT_STRETCH | BLT_ALPHABLEND) ) ||
        ( pParms->pBrush != NULL ) ||
        ( pParms->pMask != NULL ) ||
        ( pParms->pLookup != NULL ) ||
        ( pParms->pConvert != NULL ) ||
        ( pParms->pDst->Format() != gpe16Bpp ) ||
        ( pParms->pSrc->Format() != gpe16Bpp ) )
    {
        return FALSE;
    }

    // If the source and destination surface match, just fail.
    if ( pParms->pDst == pParms->pSrc )
    {
        return FALSE;
    }

    return TRUE;
}


/*------------------------------------------------------------------------------
    CanUse_Blt_NoStretch_Alpha_16To16

    Returns TRUE if it's okay to use the specified blt.

    Parameters:
    pParms: Blt parameters.
------------------------------------------------------------------------------*/
inline BOOL CanUse_Blt_NoStretch_Alpha_16To16( GPEBltParms *pParms )
{
    // Do simple parameter validation
    if ( ( pParms->rop4 != 0xcccc ) ||
        ( (pParms->bltFlags & (BLT_TRANSPARENT | BLT_STRETCH | BLT_ALPHABLEND)) != BLT_ALPHABLEND ) ||
        ( pParms->pSrc == NULL ) ||
        ( pParms->pDst == NULL ) ||
        ( pParms->pBrush != NULL ) ||
        ( pParms->pMask != NULL ) ||
        ( pParms->pLookup != NULL ) ||
        ( pParms->pConvert != NULL ) ||
        ( !IsSurface_R5_G6_B5(pParms->pDst) ) ||
        ( !IsSurface_R5_G6_B5(pParms->pSrc) ) )
    {
        return FALSE;
    }

    // If the source and destination surface match, just fail.
    // TODO: Allow for non-overlapping rectangles?
    if ( pParms->pDst == pParms->pSrc )
    {
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
    CanUse_Blt_NoStretch_Trans_16To16

    Returns TRUE if it's okay to use the specified blt.

    Parameters:
    pParms: Blt parameters.
------------------------------------------------------------------------------*/
inline BOOL CanUse_Blt_NoStretch_Trans_16To16( GPEBltParms *pParms )
{
    // Do parameter validation
    if ( ( pParms->rop4 != 0xcccc ) ||
        ( pParms->solidColor == -1 ) ||
        ( pParms->pSrc == NULL ) ||
        ( ( pParms->bltFlags & (BLT_TRANSPARENT | BLT_STRETCH | BLT_ALPHABLEND) ) != BLT_TRANSPARENT ) ||
        ( pParms->pBrush != NULL ) ||
        ( pParms->pMask != NULL ) ||
        ( pParms->pLookup != NULL ) ||
        ( pParms->pConvert != NULL ) ||
        ( pParms->pDst->Format() != gpe16Bpp ) ||
        ( pParms->pSrc->Format() != gpe16Bpp ) )
    {
        return FALSE;
    }

    // If the source and destination surface match, just fail.
    // TODO: Allow for non-overlapping rectangles?
    if ( pParms->pDst == pParms->pSrc )
    {
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
    CanUse_Blt_Stretch_Trans_16To16

    Returns TRUE if it's okay to use the specified blt.

    Parameters:
    pParms: Blt parameters.
------------------------------------------------------------------------------*/
inline BOOL CanUse_Blt_Stretch_Trans_16To16( GPEBltParms *pParms )
{
    // Do parameter validation
    if ( ( pParms->rop4 != 0xcccc ) ||
        ( pParms->pSrc == NULL ) ||
        ( pParms->iMode == BILINEAR ) ||
        ( pParms->bltFlags & BLT_ALPHABLEND ) ||
        ( !( pParms->bltFlags & ( BLT_TRANSPARENT | BLT_STRETCH ) ) ) ||
        ( pParms->pBrush != NULL ) ||
        ( pParms->pMask != NULL ) ||
        ( pParms->pLookup != NULL ) ||
        ( pParms->pConvert != NULL ) ||
        ( pParms->pDst->Format() != gpe16Bpp ) ||
        ( pParms->pSrc->Format() != gpe16Bpp ) )
    {
        return FALSE;
    }

    // If the source and destination surface match, just fail.
    // TODO: Allow for non-overlapping rectangles?
    if ( pParms->pDst == pParms->pSrc )
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL SelectOptimizedBlt(GPEBltParms* pBltParms)
{
    BOOL retval = TRUE;
    if (CanUse_Blt_NoStretch_NoTrans_16To16(pBltParms))
    {
        pBltParms->pBlt = ((SCODE (GPE::*)(GPEBltParms *))&Emulator::Blt_NoStretch_NoTrans_16To16);
    }
    else if (CanUse_Blt_NoStretch_Alpha_16To16(pBltParms))
    {
        pBltParms->pBlt = ((SCODE (GPE::*)(GPEBltParms *))&Emulator::Blt_NoStretch_Alpha_16To16);
    }
    else if (CanUse_Blt_NoStretch_Trans_16To16(pBltParms))
    {
        pBltParms->pBlt = ((SCODE (GPE::*)(GPEBltParms *))&Emulator::Blt_NoStretch_Trans_16To16);
    }
    else if (CanUse_Blt_Stretch_Trans_16To16(pBltParms))
    {
        pBltParms->pBlt = ((SCODE (GPE::*)(GPEBltParms *))&Emulator::Blt_Stretch_Trans_16To16);
    }
    else
    {
        retval = FALSE;
    }

    return retval;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:      BltPrepare
//
//  DESCRIPTION:   This method identifies the appropriate functions
//                 needed to perform individual blits.
//
//  PARAMETERS:
//
//  RETURNS:
//        S_OK              successful
//                others            failed
//
//------------------------------------------------------------------------------
SCODE LcdcClass::BltPrepare(GPEBltParms * pBltParms)
{
    RECTL   rectl;
    BOOL bRotate = FALSE;
    LONG   iSwapTmp;

    DEBUGMSG (GPE_ZONE_INIT, (TEXT("RGPEFlat::BltPrepare\r\n")));

    // default to base EmulatedBlt routine
    pBltParms->pBlt = &GPE::EmulatedBlt;

    // Can we do an optimized blt?
    // If so, this will set pBlt to use an optimized routine
    SelectOptimizedBlt(pBltParms);

    // see if we need to deal with cursor

        // check for destination overlap with cursor and turn off cursor if overlaps
    if (pBltParms->pDst == m_pPrimarySurface)   // only care if dest is main display surface
    {
        if (m_CursorVisible && !m_CursorDisabled)
        {
            if (pBltParms->prclDst != NULL)     // make sure there is a valid prclDst
            {
                rectl = *pBltParms->prclDst;        // if so, use it

                // There is no guarantee of a well ordered rect in blitParamters
                // due to flipping and mirroring.
                if(rectl.top > rectl.bottom)
                {
                    iSwapTmp     = rectl.top;
                    rectl.top    = rectl.bottom;
                    rectl.bottom = iSwapTmp;
                }

                if(rectl.left > rectl.right)
                {
                    iSwapTmp    = rectl.left;
                    rectl.left  = rectl.right;
                    rectl.right = iSwapTmp;
                }
            }
            else
            {
                rectl = m_CursorRect;                   // if not, use the Cursor rect - this forces the cursor to be turned off in this case
            }

            if (m_CursorRect.top < rectl.bottom && m_CursorRect.bottom > rectl.top &&
                m_CursorRect.left < rectl.right && m_CursorRect.right > rectl.left)
            {
                CursorOff();
                m_CursorForcedOff = TRUE;
            }
        }

        if (m_iRotate )
        {
            bRotate = TRUE;
        }
    }

        // check for source overlap with cursor and turn off cursor if overlaps
    if (pBltParms->pSrc == m_pPrimarySurface)   // only care if source is main display surface
    {
        if (m_CursorVisible && !m_CursorDisabled)
        {
            if (pBltParms->prclSrc != NULL)     // make sure there is a valid prclSrc
            {
                rectl = *pBltParms->prclSrc;        // if so, use it
            }
            else
            {
                rectl = m_CursorRect;                   // if not, use the CUrsor rect - this forces the cursor to be turned off in this case
            }
            if (m_CursorRect.top < rectl.bottom && m_CursorRect.bottom > rectl.top &&
                m_CursorRect.left < rectl.right && m_CursorRect.right > rectl.left)
            {
                CursorOff();
                m_CursorForcedOff = TRUE;
            }
        }


        if (m_iRotate) //if rotated
        {
            bRotate = TRUE;
        }
    }

    if (bRotate)
        pBltParms->pBlt = (SCODE (GPE::*)(GPEBltParms *))&GPE::EmulatedBltRotate;

    return S_OK;
}
#else // USE_V1_CODE

//------------------------------------------------------------------------------
//
//  FUNCTION:      BltPrepare
//
//  DESCRIPTION:   This method identifies the appropriate functions
//                 needed to perform individual blits.
//
//  PARAMETERS:
//
//  RETURNS:
//        S_OK              successful
//                others            failed
//
//------------------------------------------------------------------------------
SCODE LcdcClass::BltPrepare(GPEBltParms * pBltParms)
{
    RECTL   rectl;
    BOOL bRotate = FALSE;

    DEBUGMSG (GPE_ZONE_INIT, (TEXT("RGPEFlat::BltPrepare\r\n")));

    // default to base EmulatedBlt routine
    pBltParms->pBlt = &GPE::EmulatedBlt;

    // see if we need to deal with cursor

        // check for destination overlap with cursor and turn off cursor if overlaps
    if (pBltParms->pDst == m_pPrimarySurface)   // only care if dest is main display surface
    {
        if (m_CursorVisible && !m_CursorDisabled)
        {
            if (pBltParms->prclDst != NULL)     // make sure there is a valid prclDst
            {
                rectl = *pBltParms->prclDst;        // if so, use it
            }
            else
            {
                rectl = m_CursorRect;                   // if not, use the Cursor rect - this forces the cursor to be turned off in this case
            }

            if (m_CursorRect.top < rectl.bottom && m_CursorRect.bottom > rectl.top &&
                m_CursorRect.left < rectl.right && m_CursorRect.right > rectl.left)
            {
                CursorOff();
                m_CursorForcedOff = TRUE;
            }
        }

        if (m_iRotate )
        {
            bRotate = TRUE;
        }
    }

        // check for source overlap with cursor and turn off cursor if overlaps
    if (pBltParms->pSrc == m_pPrimarySurface)   // only care if source is main display surface
    {
        if (m_CursorVisible && !m_CursorDisabled)
        {
            if (pBltParms->prclSrc != NULL)     // make sure there is a valid prclSrc
            {
                rectl = *pBltParms->prclSrc;        // if so, use it
            }
            else
            {
                rectl = m_CursorRect;                   // if not, use the CUrsor rect - this forces the cursor to be turned off in this case
            }
            if (m_CursorRect.top < rectl.bottom && m_CursorRect.bottom > rectl.top &&
                m_CursorRect.left < rectl.right && m_CursorRect.right > rectl.left)
            {
                CursorOff();
                m_CursorForcedOff = TRUE;
            }
        }


        if (m_iRotate) //if rotated
        {
            bRotate = TRUE;
        }
    }

    if (bRotate)
        pBltParms->pBlt = (SCODE (GPE::*)(GPEBltParms *))(&GPE::EmulatedBltRotate);

    return S_OK;
}

#endif // USE_V1_CODE

//------------------------------------------------------------------------------
//
//  FUNCTION:      BltComplete
//
//  DESCRIPTION:   This method executes to complete
//                 a blit sequence initiated by GPE::BltPrepare
//
//  PARAMETERS:
//
//  RETURNS:
//        S_OK              successful
//                others            failed
//
//------------------------------------------------------------------------------
SCODE LcdcClass::BltComplete(GPEBltParms * pBltParms)
{
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("RGPEFlat::BltComplete\r\n")));

    // see if cursor was forced off because of overlap with source or destination and turn back on
    if (m_CursorForcedOff)
    {
        m_CursorForcedOff = FALSE;
        CursorOn();
    }

    return S_OK;
}




//------------------------------------------------------------------------------
//
//  FUNCTION:       AllocSurface
//
//  DESCRIPTION:    This method executes when the driver
//                  must allocate storage for surface pixels.
//
//  PARAMETERS:
//
//  RETURNS:
//        S_OK              successful
//                others            failed
//
//------------------------------------------------------------------------------
SCODE LcdcClass::AllocSurface( GPESurf **ppSurf,
                   int width,
                   int height,
                   EGPEFormat format,
                   int surfaceFlags)
{
    if ((surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY) || (format == m_pMode->format) && (surfaceFlags & GPE_PREFER_VIDEO_MEMORY))
    {
        if (!(format == m_pMode->format))
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("AllocSurface - Invalid format value\n")));
            return E_INVALIDARG;
        }

        // Attempt to allocate from video memory
        Node2D *pNode = m_p2DVideoMemory->Alloc(width, height);
        if (pNode)
        {
            ULONG offset = (m_nScreenStride * pNode->Top()) + ((pNode->Left()* EGPEFormatToBpp[format]) / 8);
            *ppSurf = new LcdcSurf(width, height, offset, (PVOID)(m_pLinearVideoMem + offset), m_nScreenStride, format, pNode);
            if (!(*ppSurf))
            {
                pNode->Free();
                DEBUGMSG(GPE_ZONE_ERROR, (TEXT("AllocSurface - Out of Memory 1\n")));
                return E_OUTOFMEMORY;
            }
            return S_OK;
        }

        if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
        {
            *ppSurf = (GPESurf *)NULL;
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("AllocSurface - Out of Memory 2\n")));
            return E_OUTOFMEMORY;
        }
    }

    if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
    {
        *ppSurf = (GPESurf *)NULL;
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("AllocSurface - Out of Memory 3\n")));
        return  E_OUTOFMEMORY;
    }

    // Allocate from system memory
    DEBUGMSG(GPE_ZONE_CREATE, (TEXT("Creating a GPESurf in system memory. EGPEFormat = %d\r\n"), (int) format));
#ifdef OEM_ALLOC_MEM
    *ppSurf = new LcdcSurf(width, height, format);
#else
    *ppSurf = new GPESurf(width, height, format);
#endif
    if (*ppSurf != NULL)
    {
        // check we allocated bits succesfully
        if (((*ppSurf)->Buffer()) == NULL)
        {
            delete *ppSurf;
        }
        else
        {
            return  S_OK;
        }
    }

    DEBUGMSG(GPE_ZONE_ERROR, (TEXT("AllocSurface - Out of Memory 4\n")));
    return E_OUTOFMEMORY;
}

#ifdef USE_V1_CODE

SCODE
LcdcClass::WrappedEmulatedLine(
    GPELineParms *lineParameters
    )
{
    SCODE retval;
    RECT  bounds;
    int   N_plus_1;                // Minor length of bounding rect + 1

    // calculate the bounding-rect to determine overlap with cursor
    if (lineParameters->dN)            // The line has a diagonal component (we'll refresh the bounding rect)
    {
        N_plus_1 = 2 + ((lineParameters->cPels * lineParameters->dN) / lineParameters->dM);
    }
    else
    {
        N_plus_1 = 1;
    }

    switch(lineParameters->iDir)
    {
        case 0:
            bounds.left = lineParameters->xStart;
            bounds.top = lineParameters->yStart;
            bounds.right = lineParameters->xStart + lineParameters->cPels + 1;
            bounds.bottom = bounds.top + N_plus_1;
            break;
        case 1:
            bounds.left = lineParameters->xStart;
            bounds.top = lineParameters->yStart;
            bounds.bottom = lineParameters->yStart + lineParameters->cPels + 1;
            bounds.right = bounds.left + N_plus_1;
            break;
        case 2:
            bounds.right = lineParameters->xStart + 1;
            bounds.top = lineParameters->yStart;
            bounds.bottom = lineParameters->yStart + lineParameters->cPels + 1;
            bounds.left = bounds.right - N_plus_1;
            break;
        case 3:
            bounds.right = lineParameters->xStart + 1;
            bounds.top = lineParameters->yStart;
            bounds.left = lineParameters->xStart - lineParameters->cPels;
            bounds.bottom = bounds.top + N_plus_1;
            break;
        case 4:
            bounds.right = lineParameters->xStart + 1;
            bounds.bottom = lineParameters->yStart + 1;
            bounds.left = lineParameters->xStart - lineParameters->cPels;
            bounds.top = bounds.bottom - N_plus_1;
            break;
        case 5:
            bounds.right = lineParameters->xStart + 1;
            bounds.bottom = lineParameters->yStart + 1;
            bounds.top = lineParameters->yStart - lineParameters->cPels;
            bounds.left = bounds.right - N_plus_1;
            break;
        case 6:
            bounds.left = lineParameters->xStart;
            bounds.bottom = lineParameters->yStart + 1;
            bounds.top = lineParameters->yStart - lineParameters->cPels;
            bounds.right = bounds.left + N_plus_1;
            break;
        case 7:
            bounds.left = lineParameters->xStart;
            bounds.bottom = lineParameters->yStart + 1;
            bounds.right = lineParameters->xStart + lineParameters->cPels + 1;
            bounds.top = bounds.bottom - N_plus_1;
            break;
        default:
            DEBUGMSG(GPE_ZONE_ERROR,(TEXT("Invalid direction: %d\r\n"), lineParameters->iDir));
            return E_INVALIDARG;
    }

    // check for line overlap with cursor and turn off cursor if overlaps
    RECTL cursorRect = m_CursorRect;
    RotateRectl (&cursorRect);

    if (m_CursorVisible && !m_CursorDisabled &&
        cursorRect.top < bounds.bottom && cursorRect.bottom > bounds.top &&
        cursorRect.left < bounds.right && cursorRect.right > bounds.left)
    {
        CursorOff();
        m_CursorForcedOff = TRUE;
    }

    // do emulated line
    retval = EmulatedLine (lineParameters);

    // see if cursor was forced off because of overlap with line bounds and turn back on
    if (m_CursorForcedOff)
    {
        m_CursorForcedOff = FALSE;
        CursorOn();
    }

    return    retval;

}

//------------------------------------------------------------------------------
//
//  FUNCTION:      Line
//
//  DESCRIPTION:   This method executes before and
//                 after a sequence of line segments.
//
//  PARAMETERS:
//
//  RETURNS:
//        S_OK              successful
//                others            failed
//
//------------------------------------------------------------------------------
SCODE LcdcClass::Line(GPELineParms * pLineParms, EGPEPhase phase)
{
    DEBUGMSG(GPE_ZONE_LINE, (TEXT("LcdcClass::Line\r\n")));

    if ((pLineParms->pDst != m_pPrimarySurface))
    {
        pLineParms->pLine = &GPE::EmulatedLine;
    }
    else
    {
        pLineParms->pLine = (SCODE (GPE::*)(struct GPELineParms *)) &LcdcClass::WrappedEmulatedLine;
    }

    return S_OK;
}

#else // USE_V1_CODE

//------------------------------------------------------------------------------
//
//  FUNCTION:      Line
//
//  DESCRIPTION:   This method executes before and
//                 after a sequence of line segments.
//
//  PARAMETERS:
//
//  RETURNS:
//        S_OK              successful
//                others            failed
//
//------------------------------------------------------------------------------
SCODE LcdcClass::Line(GPELineParms * pLineParms, EGPEPhase phase)
{
    DEBUGMSG(GPE_ZONE_LINE, (TEXT("LcdcClass::Line\r\n")));

    pLineParms->pLine = &GPE::EmulatedLine;

    return S_OK;
}

#endif // USE_V1_CODE

//------------------------------------------------------------------------------
//
//  FUNCTION:     SetPalette
//
//  DESCRIPTION:  Palette methods
//
//  PARAMETERS:
//
//  RETURNS:
//        S_OK              successful
//                others            failed
//
//------------------------------------------------------------------------------
SCODE LcdcClass::SetPalette(const PALETTEENTRY *src,
                    unsigned short firstEntry,
                unsigned short numEntries)
{
    //In 16bpp display mode,
    //the LCDC of Tahiti doesn't support palette change
    //put codes here in future expansion if 4bpp or 8bpp is supported

    return S_OK;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     InVBlank
//
//  DESCRIPTION:  Timing method
//
//  PARAMETERS:
//
//  RETURNS:
//        S_OK              successful
//                others            failed
//
//------------------------------------------------------------------------------
int LcdcClass::InVBlank(void)
{
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("LcdcClass::InVBlank()\r\n")));

    //hardware of Tahiti-Lite do not support this function
    return 1;
}




//------------------------------------------------------------------------------
//
//  FUNCTION:     ContrastControl
//
//  DESCRIPTION:  Sorfware contrast control method
//
//  PARAMETERS:
//                cmd      [in]      contrast control command
//                pValue   [in/out]  pointer to the parameter to be passed
//
//  RETURNS:
//        TRUE    successful
//                FALSE   failed
//
//------------------------------------------------------------------------------
BOOL LcdcClass::ContrastControl(ULONG cmd, ULONG *pValue)
{
    bool bRet = FALSE;

    return bRet;
}




//------------------------------------------------------------------------------
//
//  FUNCTION:     SetPointerShape
//
//  DESCRIPTION:   This method sets the shape of the pointer
//
//  PARAMETERS:
//
//  RETURNS:
//        S_OK              successful
//                others            failed
//
//------------------------------------------------------------------------------
SCODE LcdcClass::SetPointerShape(GPESurf * pMask,
                                 GPESurf * pColorSurf,
                                 int xHot,
                                 int yHot,
                                 int cx,
                                 int cy)
{
    UCHAR   *andPtr;        // input pointer
    UCHAR   *xorPtr;        // input pointer
    UCHAR   *andLine;       // output pointer
    UCHAR   *xorLine;       // output pointer
    char    bAnd;
    char    bXor;
    int     row;
    int     col;
    int     i;
    int     bitMask;

    DEBUGMSG(GPE_ZONE_CURSOR,(TEXT("RGPEFlat::SetPointerShape(0x%X, 0x%X, %d, %d, %d, %d)\r\n"),pMask, pColorSurf, xHot, yHot, cx, cy));

    // turn current cursor off
    CursorOff();

    // release memory associated with old cursor
    if (!pMask)                         // do we have a new cursor shape
    {
        m_CursorDisabled = TRUE;        // no, so tag as disabled
    }
    else
    {
        m_CursorDisabled = FALSE;       // yes, so tag as not disabled

        // store size and hotspot for new cursor
        m_CursorSize.x = cx;
        m_CursorSize.y = cy;
        m_CursorHotspot.x = xHot;
        m_CursorHotspot.y = yHot;

        andPtr = (UCHAR*)pMask->Buffer();
        xorPtr = (UCHAR*)pMask->Buffer() + (cy * pMask->Stride());

        // store OR and AND mask for new cursor
        for (row = 0; row < cy; row++)
        {
            andLine = &m_CursorAndShape[cx * row];
            xorLine = &m_CursorXorShape[cx * row];

            for (col = 0; col < cx / 8; col++)
            {
                bAnd = andPtr[row * pMask->Stride() + col];
                bXor = xorPtr[row * pMask->Stride() + col];

                for (bitMask = 0x0080, i = 0; i < 8; bitMask >>= 1, i++)
                {
                    andLine[(col * 8) + i] = bAnd & bitMask ? 0xFF : 0x00;
                    xorLine[(col * 8) + i] = bXor & bitMask ? 0xFF : 0x00;
                }
            }
        }
    }

    return  S_OK;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:     MovePointer
//
//  DESCRIPTION:   This method executes from applications either
//         to move the hot spot of the cursor to
//         a specific screen location or to hide the cursor.
//
//  PARAMETERS:
//       x  [in] Horizontal screen location to move the cursor to.
//                       Applications can pass a value of -1 to hide the cursor.
//               y  [in] Vertical screen location to move the cursor to.
//  RETURNS:
//       S_OK              successful
//               others            failed
//
//------------------------------------------------------------------------------

SCODE LcdcClass::MovePointer(int x, int y)
{
    DEBUGMSG(GPE_ZONE_CURSOR, (TEXT("RGPEFlat::MovePointer(%d, %d)\r\n"), x, y));

    CursorOff();

    if (x != -1 || y != -1)
    {
        // compute new cursor rect
        m_CursorRect.left = x - m_CursorHotspot.x;
        m_CursorRect.right = m_CursorRect.left + m_CursorSize.x;
        m_CursorRect.top = y - m_CursorHotspot.y;
        m_CursorRect.bottom = m_CursorRect.top + m_CursorSize.y;

        CursorOn();
    }

    return  S_OK;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:     SetCursorColor
//
//  DESCRIPTION:   This method set the color of cursor
//                 The color fomat is 18 bits
//
//  PARAMETERS:
//      red  [in]red component of the cursor color, in 6 bits format
//      green[in]green component of the cursor color, in 6 bits format
//      blue [in]blue component of the cursor color, in 6 bits format
//  RETURNS:
//      S_OK              successful
//              others            failed
//
//------------------------------------------------------------------------------
SCODE LcdcClass::SetCursorColor(UINT8 red, UINT8 green, UINT8 blue)
{
    if (  red   > 0x3f ||
          green > 0x3f ||
          blue  > 0x3f)
    {
        DEBUGMSG(GPE_ZONE_ERROR,(TEXT("LcdcClass::SetCursorColor: Bad color 0x%x  0x%x  0x%x\r\n"), red, green, blue));
        return   E_INVALIDARG;
    }

    // disable cursor
    INSREG32BF(&m_pLCDC->CPR, LCDC_CPR_OP, LCDC_CPR_OP_DISABLE);
    INSREG32BF(&m_pLCDC->CPR, LCDC_CPR_CC, LCDC_CPR_CC_DISABLED);

    //set cursor color
    m_pLCDC->CCMR  =  blue | green<<6 | red<<12;

    //enable cusor
    INSREG32BF(&m_pLCDC->CPR, LCDC_CPR_OP, LCDC_CPR_OP_ENABLE);
    INSREG32BF(&m_pLCDC->CPR, LCDC_CPR_CC, LCDC_CPR_CC_OR);

    DEBUGMSG(GPE_ZONE_CURSOR, (TEXT("lcdcClass SetCursorColor: the new cursor color is 0x%x 0x%x 0x%x! \r\n"),red, green, blue));

    return S_OK;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:       DrvEscape
//
//  DESCRIPTION:    This routine handles the needed DrvEscape codes.
//
//  PARAMETERS:     pso - pointer to SURFOBJ structure decribing desired surface
//                  iEsc - Escape codes
//                  pvIn - pointer to input buffer
//                  cjIn - size of input buffer
//                  cjOut - size of output buffer
//                  pvOut - pointer to output buffer
//
//  RETURNS:
//                  TRUE              successful
//                  FALSE             failed
//
//------------------------------------------------------------------------------
ULONG LcdcClass::DrvEscape(SURFOBJ *pso, ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut)
{
    ULONG retval = ESC_FAILED;
    PVIDEO_POWER_MANAGEMENT psPowerManagement;

    switch(iEsc)
    {
        case QUERYESCSUPPORT:
            if(pvIn != NULL && cjIn == sizeof(DWORD))
            {
                // Query DrvEscap support functions
                DWORD EscapeFunction;
                EscapeFunction = *(DWORD *)pvIn;
                if ((EscapeFunction ==  QUERYESCSUPPORT)          ||
                    (EscapeFunction == SETPOWERMANAGEMENT)        ||
                    (EscapeFunction == GETPOWERMANAGEMENT)        ||
                    (EscapeFunction == IOCTL_POWER_CAPABILITIES)  ||
                    (EscapeFunction == IOCTL_POWER_QUERY)         ||
                    (EscapeFunction == IOCTL_POWER_SET)           ||
                    (EscapeFunction == IOCTL_POWER_GET)           ||
                    (EscapeFunction == LCDC_ESC_REQUEST_WINDOW)   ||
                    (EscapeFunction == LCDC_ESC_RELEASE_WINDOW)   ||
                    (EscapeFunction == LCDC_ESC_ENABLE_WINDOW)    ||
                    (EscapeFunction == LCDC_ESC_DISABLE_WINDOW)   ||
                    (EscapeFunction == LCDC_ESC_FLIP_WINDOW)      ||
                    (EscapeFunction == LCDC_ESC_GET_TRANSPARENCY) ||
                    (EscapeFunction == LCDC_ESC_SET_TRANSPARENCY) )
                    retval = ESC_SUCCESS;
                else
                    retval = LcdcDrvEscape(pso, iEsc, cjIn, pvIn, cjOut, pvOut);
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);

            break;

        case SETPOWERMANAGEMENT :
            DEBUGMSG(GPE_ZONE_WARNING, (TEXT("SETPOWERMANAGEMENT\r\n")));
            if (psPowerManagement = (PVIDEO_POWER_MANAGEMENT) pvIn)
                {
                    if (cjIn >= sizeof(VIDEO_POWER_MANAGEMENT))
                   {
                            DWORD err = ERROR_SUCCESS;

                            //Ask Power Manager to update the system power state,
                            //PM will call us back with IOCTL_POWER_SET
                            switch(psPowerManagement->PowerState)
                            {
                                case VideoPowerOff:
                                    DEBUGMSG(GPE_ZONE_WARNING,(TEXT("Requesting POWER_STATE_IDLE\r\n")));

                                    err = SetSystemPowerState(0, POWER_STATE_IDLE, POWER_FORCE);
                                    break;

                                case VideoPowerOn:
                                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("Requesting POWER_STATE_ON\r\n")));

                                    err = SetSystemPowerState(0, POWER_STATE_ON, POWER_FORCE);
                                    break;

                                default:
                                        DEBUGMSG(GPE_ZONE_WARNING, (TEXT("SetPowerManagement : unsupported power state requested\r\n")));
                                        break;
                            }

                            if(ERROR_SUCCESS == err)
                                retval = ESC_SUCCESS;

                   }
                else
                    SetLastError(ERROR_INVALID_PARAMETER);
            }
               else
                SetLastError(ERROR_INVALID_PARAMETER);

            break;

        case GETPOWERMANAGEMENT:
            DEBUGMSG(GPE_ZONE_WARNING, (TEXT("GETPOWERMANAGEMENT\r\n")));
            if (psPowerManagement = (PVIDEO_POWER_MANAGEMENT) pvOut)
            {
                if (cjOut >= sizeof(VIDEO_POWER_MANAGEMENT))
                   {
                    psPowerManagement->Length = sizeof(VIDEO_POWER_MANAGEMENT);
                            psPowerManagement->DPMSVersion = 0x0100;
                            psPowerManagement->PowerState = PmToVideoPowerState(m_Dx);
                    retval = ESC_SUCCESS;
                    }
                    else
                        SetLastError(ERROR_INVALID_PARAMETER);
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);

            break;

        case IOCTL_POWER_CAPABILITIES:
            DEBUGMSG(GPE_ZONE_WARNING, (TEXT("IOCTL_POWER_CAPABILITIES\r\n")));
            if(pvOut != NULL && cjOut == sizeof(POWER_CAPABILITIES))
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES)pvOut;
                memset(ppc, 0, sizeof(POWER_CAPABILITIES));
                ppc->DeviceDx = 0x11;   // support D0 and D4
                ppc->WakeFromDx = 0x00; // No wake capability
                ppc->InrushDx = 0x00;       // No in rush requirement
                ppc->Power[D0] = 600;                   // 0.6W
                ppc->Power[D1] = PwrDeviceUnspecified;
                ppc->Power[D2] = PwrDeviceUnspecified;
                ppc->Power[D3] = PwrDeviceUnspecified;
                ppc->Power[D4] = 0;
                ppc->Latency[D0] = 0;
                ppc->Latency[D1] = PwrDeviceUnspecified;
                ppc->Latency[D2] = PwrDeviceUnspecified;
                ppc->Latency[D3] = PwrDeviceUnspecified;
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
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("IOCTL_POWER_QUERY %u %s\r\n"),
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
                DEBUGMSG(GPE_ZONE_WARNING, (L"%s IOCTL_POWER_GET: passing back %u\r\n", m_szDevName, CurrentDx));
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
                    DEBUGMSG(GPE_ZONE_WARNING, (L"%s IOCTL_POWER_SET %u: passing back %u\r\n", m_szDevName, NewDx, m_Dx));
                }
                else
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    DEBUGMSG(GPE_ZONE_WARNING, (L"IOCTL_POWER_SET: invalid state request %u\r\n", NewDx));
                }
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);
            break;

        case LCDC_ESC_REQUEST_WINDOW:
            if(pvIn != NULL && cjIn == sizeof(lcdcGraphicWindowOP_t))
            {
                pLcdcGraphicWindowOP_t pGWop = (pLcdcGraphicWindowOP_t)pvIn;

                if(m_hGraphicWindowHdl)
                {
                    // Graphic window is in use, reject the request
                    pGWop->Result = ESC_FAILED;
                    pGWop->BufHandle = NULL;
                }
                else
                {
                    // Graphic window is free, grant the request
                    UINT32 timeNow = GetTickCount();  // Get a pseudorandom number from OS time

                    if(timeNow)
                        m_hGraphicWindowHdl = (HANDLE)timeNow;
                    else
                        m_hGraphicWindowHdl = (HANDLE)(timeNow++);

                    pGWop->Result          = ESC_SUCCESS;
                    pGWop->BufHandle       = m_hGraphicWindowHdl;

                    m_bGraphicWindowFlipped = FALSE;
                    m_bGraphicWindowRunning = FALSE;

                    ClearGraphicWindowRegs(m_pLCDC);
                    retval = ESC_SUCCESS;
                }

            }
            else
            {
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_REQUEST_WINDOW, invalid parameter\r\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
            }
            break;

        case LCDC_ESC_RELEASE_WINDOW:
            if(pvIn != NULL && cjIn == sizeof(lcdcGraphicWindowOP_t))
            {
                pLcdcGraphicWindowOP_t pGWop = (pLcdcGraphicWindowOP_t)pvIn;

                if(pGWop->BufHandle && (pGWop->BufHandle == m_hGraphicWindowHdl))
                {
                    if(m_bGraphicWindowRunning)
                        DisableGraphicWindow(pGWop);

                    pGWop->Result = ESC_SUCCESS;
                    pGWop->BufHandle = NULL;

                    m_bGraphicWindowFlipped = FALSE;
                    m_bGraphicWindowRunning = FALSE;

                    m_hGraphicWindowHdl = NULL;

                    retval = ESC_SUCCESS;
                }
                else
                {
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_RELEASE_WINDOW, wrong request\r\n")));
                    pGWop->Result = ESC_FAILED;
                    SetLastError(ERROR_BUSY);
                }
            }
            else
            {
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_RELEASE_WINDOW, invalid parameter\r\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
            }
            break;

        case LCDC_ESC_ENABLE_WINDOW:
            if(pvIn != NULL && cjIn == sizeof(lcdcGraphicWindowOP_t))
            {
                if(EnableGraphicWindow((pLcdcGraphicWindowOP_t)pvIn))
                {
                    retval = ESC_SUCCESS;
                }
                else
                {
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_ENABLE_WINDOW, wrong request\r\n")));
                    SetLastError(ERROR_INVALID_SERVICE_CONTROL);
                }
            }
            else
            {
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_ENABLE_WINDOW, invalid parameter\r\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
            }
            break;

        case LCDC_ESC_DISABLE_WINDOW:
            if(pvIn != NULL && cjIn == sizeof(lcdcGraphicWindowOP_t))
            {
                if(DisableGraphicWindow((pLcdcGraphicWindowOP_t)pvIn))
                {
                    retval = ESC_SUCCESS;
                }
                else
                {
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_ENABLE_WINDOW, wrong request\r\n")));
                    SetLastError(ERROR_INVALID_SERVICE_CONTROL);
                }
            }
            else
            {
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_DISABLE_WINDOW, invalid parameter\r\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
            }

            break;

        case LCDC_ESC_FLIP_WINDOW:
            if(pvIn != NULL && cjIn == sizeof(lcdcGraphicWindowOP_t))
            {
                if(FlipGraphicWindow((pLcdcGraphicWindowOP_t)pvIn))
                {
                    retval = ESC_SUCCESS;
                }
                else
                {
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_ENABLE_WINDOW, wrong request\r\n")));
                    SetLastError(ERROR_INVALID_SERVICE_CONTROL);
                }
            }
            else
            {
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_FLIP_WINDOW, invalid parameter\r\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
            }
            break;

        case LCDC_ESC_GET_TRANSPARENCY:
            if(pvIn != NULL && cjIn == sizeof(lcdcGraphicWindowOP_t))
            {
                if(GetGWTransparency((pLcdcGraphicWindowOP_t)pvIn))
                {
                    retval = ESC_SUCCESS;
                }
                else
                {
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_GET_TRANSPARENCY, wrong request\r\n")));
                }
            }
            else
            {
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_GET_TRANSPARENCY, invalid parameter\r\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
            }
            break;

        case LCDC_ESC_SET_TRANSPARENCY:
            if(pvIn != NULL && cjIn == sizeof(lcdcGraphicWindowOP_t))
            {
                if(SetGWTransparency((pLcdcGraphicWindowOP_t)pvIn))
                {
                    retval = ESC_SUCCESS;
                }
                else
                {
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_SET_TRANSPARENCY, wrong request\r\n")));
                }
            }
            else
            {
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("LCDC: LCDC_ESC_SET_TRANSPARENCY, invalid parameter\r\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
            }
            break;

        case DRVESC_GETSCREENROTATION:
            *(int *)pvOut = ((DMDO_0 | DMDO_90 | DMDO_180 | DMDO_270) << 8) | ((BYTE)m_iRotate);
            retval=DISP_CHANGE_SUCCESSFUL;
            break;

        case DRVESC_SETSCREENROTATION:
            if ((cjIn == DMDO_0) ||
                (cjIn == DMDO_90) ||
                (cjIn == DMDO_180) ||
                (cjIn == DMDO_270) )
            {
                    return DynRotate(cjIn);
            }
            retval=DISP_CHANGE_BADMODE;
            break;

        default:
            retval = LcdcDrvEscape(pso, iEsc, cjIn, pvIn,cjOut, pvOut);
            break;
    }
    return retval;
}

//------------------------------------------------------------------------------
// PRIVATE FUNCTIONS
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  FUNCTION:     EnableGraphicWindow
//
//  DESCRIPTION:  This function is used to configure the Graphic Window
//                and enable it if requested.
//
//  PARAMETERS:
//
//  RETURNS:
//                TRUE           successful
//                FALSE          failed
//
//------------------------------------------------------------------------------
BOOL LcdcClass::EnableGraphicWindow(pLcdcGraphicWindowOP_t pOp)
{
    if(pOp->BufHandle && (pOp->BufHandle == m_hGraphicWindowHdl) && pOp->BufPhysicalAddr)
    {
        if(!m_bGraphicWindowRunning)
        {
            UINT32 tempGWCR = 0;
            BOOL initState;

            // Parameter validity check
            if((pOp->XOffset + pOp->Width > m_LcdcCtx.VideoFrameWidth) ||
                (pOp->YOffset + pOp->Height > m_LcdcCtx.VideoFrameHeight) ||
                (pOp->XOffset * BSPGetPixelSizeInByte() > pOp->LineStride) ||
                (pOp->Width < 16))
            {
                goto _cleanUp;
            }

            INSREG32BF(&m_pLCDC->GWSR, LCDC_GWSR_GWH, pOp->Height);
            INSREG32BF(&m_pLCDC->GWSR, LCDC_GWSR_GWW, LCDC_GW_SIZE_X(pOp->Width));
            INSREG32BF(&m_pLCDC->GWVPWR, LCDC_GWVPWR_GWVPW, LCDC_GW_SIZE_STRIDE(pOp->LineStride));
            INSREG32BF(&m_pLCDC->GWPR, LCDC_GWPR_GWXP, pOp->XOffset);
            INSREG32BF(&m_pLCDC->GWPR, LCDC_GWPR_GWYP, pOp->YOffset);


            INSREG32BF(&tempGWCR, LCDC_GWCR_GWAV, \
                    LCDC_GW_TRANSPARENCY(pOp->Transparency));

            if(pOp->isFlipWindow)
            {
                m_pLCDC->GWSAR = (UINT32)pOp->BufPhysicalAddr + pOp->LineStride * (pOp->Height - 1);
                INSREG32BF(&tempGWCR, LCDC_GWCR_GW_RVS, 1);
                m_bGraphicWindowFlipped = TRUE;
            }
            else
            {
                m_pLCDC->GWSAR = (UINT32)pOp->BufPhysicalAddr;
                m_bGraphicWindowFlipped = FALSE;
            }


            INSREG32BF(&tempGWCR, LCDC_GWCR_GWE, 1);
            m_bGraphicWindowRunning = TRUE;

            // Disable LCDC hw clocks
            DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_DISABLE);
            DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_HCLK_LCDC, DDK_CLOCK_GATE_MODE_DISABLE);

            m_pLCDC->GWCR = tempGWCR;

            // Re-enable LCDC hw clocks
            initState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_ENABLE);
            if(initState != TRUE)
            {
                ERRORMSG(1, (TEXT("LCDC Class: LCDC hw clock enable failed!\r\n")));
                goto _cleanUp;
            }
            initState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_HCLK_LCDC, DDK_CLOCK_GATE_MODE_ENABLE);
            if(initState != TRUE)
            {
                ERRORMSG(1, (TEXT("LCDC Class: LCDC AHB hw clock enable failed!\r\n")));
                goto _cleanUp;
            }
        }

        pOp->Result = ESC_SUCCESS;
    }
    else
    {
        goto _cleanUp;
    }

    return TRUE;

_cleanUp:

    pOp->Result = ESC_FAILED;
    ClearGraphicWindowRegs(m_pLCDC);
    return FALSE;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     DisableGraphicWindow
//
//  DESCRIPTION:  This function is used to disable the Graphic Window.
//
//  PARAMETERS:
//
//  RETURNS:
//                TRUE           successful
//                FALSE          failed
//
//------------------------------------------------------------------------------
BOOL LcdcClass::DisableGraphicWindow(pLcdcGraphicWindowOP_t pOp)
{
    if(pOp->BufHandle && (pOp->BufHandle == m_hGraphicWindowHdl))
    {
        if(m_bGraphicWindowRunning)
        {
            INSREG32BF(&m_pLCDC->GWCR, LCDC_GWCR_GWE, 0);
            ClearGraphicWindowRegs(m_pLCDC);
            m_bGraphicWindowRunning = FALSE;
        }
        pOp->Result = ESC_SUCCESS;
        return TRUE;
    }
    else
    {
        pOp->Result = ESC_FAILED;
        return FALSE;
    }
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     FlipGraphicWindow
//
//  DESCRIPTION:  This function is to flip the Graphic Window.
//
//  PARAMETERS:
//
//  RETURNS:
//                TRUE           successful
//                FALSE          failed
//
//------------------------------------------------------------------------------
BOOL LcdcClass::FlipGraphicWindow(pLcdcGraphicWindowOP_t pOp)
{
    if(pOp->BufHandle && (pOp->BufHandle == m_hGraphicWindowHdl) && pOp->BufPhysicalAddr)
    {
        if(m_bGraphicWindowFlipped)
        {
            INSREG32BF(&m_pLCDC->GWCR, LCDC_GWCR_GW_RVS, 0);
            OUTREG32(&m_pLCDC->GWSAR, (UINT32)pOp->BufPhysicalAddr);
            m_bGraphicWindowFlipped = FALSE;
        }
        else
        {
            INSREG32BF(&m_pLCDC->GWCR, LCDC_GWCR_GW_RVS, 1);
            OUTREG32(&m_pLCDC->GWSAR, (UINT32)pOp->BufPhysicalAddr + pOp->LineStride * (pOp->Height - 1));
            m_bGraphicWindowFlipped = TRUE;
        }

        pOp->Result = ESC_SUCCESS;

        return TRUE;
    }
    else
    {
        pOp->Result = ESC_FAILED;
        return FALSE;
    }
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     GetGWTransparency
//
//  DESCRIPTION:  This function is to get current transparency of the
//                graphic window.
//
//  PARAMETERS:
//
//  RETURNS:
//                TRUE           successful
//                FALSE          failed
//
//------------------------------------------------------------------------------
BOOL LcdcClass::GetGWTransparency(pLcdcGraphicWindowOP_t pOp)
{
    if(pOp->BufHandle && (pOp->BufHandle == m_hGraphicWindowHdl))
    {
        pOp->Transparency =
            LCDC_GW_TRANSPARENCY((UINT8)EXTREG32BF(&m_pLCDC->GWCR, LCDC_GWCR_GWAV));
        pOp->Result = ESC_SUCCESS;
        return TRUE;
    }
    else
    {
        pOp->Result = ESC_FAILED;
        return FALSE;
    }
}



//------------------------------------------------------------------------------
//
//  FUNCTION:     SetGWTransparency
//
//  DESCRIPTION:  This function is to set current transparency of the
//                graphic window.
//
//  PARAMETERS:
//
//  RETURNS:
//                TRUE           successful
//                FALSE          failed
//
//------------------------------------------------------------------------------
BOOL LcdcClass::SetGWTransparency(pLcdcGraphicWindowOP_t pOp)
{
    BOOL result = FALSE;
    if(pOp->BufHandle && (pOp->BufHandle == m_hGraphicWindowHdl))
    {
        if((pOp->Transparency >= 0) &&
            (pOp->Transparency <= 0xFF))
        {
            INSREG32BF(&m_pLCDC->GWCR, LCDC_GWCR_GWAV, \
                LCDC_GW_TRANSPARENCY(pOp->Transparency));

            pOp->Result = ESC_SUCCESS;
            result = TRUE;
        }
        else
        {
            pOp->Result = ESC_FAILED;
            SetLastError(ERROR_INVALID_PARAMETER);
        }
    }
    else
    {
        pOp->Result = ESC_FAILED;
    }

    return result;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:       AdvertisePowerInterface
//
//  DESCRIPTION:    This routine notifies the OS that we support the
//                  Power Manager IOCTLs (through ExtEscape(), which
//                  calls DrvEscape()).
//
//  PARAMETERS:     hInst - handle to module DLL
//
//  RETURNS:
//                  TRUE              successful
//                  FALSE             failed
//
//------------------------------------------------------------------------------
BOOL LcdcClass::AdvertisePowerInterface(HMODULE hInst)
{
    GUID gTemp;
    BOOL fOk = FALSE;
    HKEY hk;
    DWORD dwStatus;

    // assume we are advertising the default class
    _tcscpy_s(m_szGuidClass, _countof(m_szGuidClass), PMCLASS_DISPLAY);
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
    TCHAR szModuleName[MAX_PATH];
    if(fOk)
    {
        fOk = GetModuleFileName(hInst, szModuleName, _countof(szModuleName));
        DEBUGCHK(fOk);
    }

    // Obtain just the file name
    if(fOk)
    {
        const TCHAR* pszFileName = _tcsrchr(szModuleName, _T('\\'));
        if (pszFileName)
        {
            pszFileName++;
        }
        else
        {
            pszFileName = szModuleName;
        }

        if (FAILED(StringCchCopy(m_szDevName, _countof(m_szDevName), pszFileName)))
        {
            fOk = FALSE;
        }
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
//  FUNCTION:       SetDisplayPower
//
//  DESCRIPTION:    This function sets the display hardware according
//                  to the desired video power state. Also updates
//                  current device power state variable.
//
//  PARAMETERS:     vps - desired video power state
//
//  RETURNS:        None
//
//------------------------------------------------------------------------------
void LcdcClass::SetDisplayPower(CEDEVICE_POWER_STATE dx)
{
    switch(dx)
    {
        case D0:
            if(m_Dx != dx)
            {
                // Turn off graphic window first
                if(m_bGraphicWindowRunning)
                    INSREG32BF(&m_pLCDC->GWCR, LCDC_GWCR_GWE, 0);

                // wait for beginning of next frame
                m_pLCDC->IER |= CSP_BITFMASK(LCDC_IER_EOF_EN);
                while( !( m_pLCDC->ISR & CSP_BITFMASK(LCDC_ISR_EOF)) );

                // memcpy((PVOID)m_pLinearVideoMem, (PVOID)m_pBlankVideoMem, m_VideoFrameSize);
                // wait for screen refresh.
                while( !( m_pLCDC->ISR & CSP_BITFMASK(LCDC_ISR_EOF)) );
                while( !( m_pLCDC->ISR & CSP_BITFMASK(LCDC_ISR_EOF)) );
                while( !( m_pLCDC->ISR & CSP_BITFMASK(LCDC_ISR_EOF)) );

                m_pLCDC->IER &= ~CSP_BITFMASK(LCDC_IER_EOF_EN);

                // turn on lcd panel just in case
                BSPTurnOnLCD();
                m_Dx = D0;
            }
            break;

        case D1:
        case D2:
        case D3:
        case D4:
            if(m_Dx != D4)
            {
                m_pLCDC->IER |= CSP_BITFMASK(LCDC_IER_EOF_EN);

                // wait for beginning of next frame
                while( !( m_pLCDC->ISR & CSP_BITFMASK(LCDC_ISR_EOF)) );


                // backup the frame buffer to blank buffer
                // memcpy((PVOID)m_pBlankVideoMem, (PVOID)m_pLinearVideoMem, m_VideoFrameSize);
                // clear the frame buffer
                // memset((PVOID)m_pLinearVideoMem, 0xFF, m_VideoFrameSize);
                // Turn on graphic window
                if(m_bGraphicWindowRunning)
                    INSREG32BF(&m_pLCDC->GWCR, LCDC_GWCR_GWE, 1);

                // wait for screen refresh
                while( !( m_pLCDC->ISR & CSP_BITFMASK(LCDC_ISR_EOF)) );
                while( !( m_pLCDC->ISR & CSP_BITFMASK(LCDC_ISR_EOF)) );
                while( !( m_pLCDC->ISR & CSP_BITFMASK(LCDC_ISR_EOF)) );

                m_pLCDC->IER &= ~CSP_BITFMASK(LCDC_IER_EOF_EN);

                // turn off lcd panel
                BSPTurnOffLCD();
                m_Dx = D4;
            }
            break;

        default:
            break;
    }
}

//------------------------------------------------------------------------------
//
//  FUNCTION:       PmToVideoPowerState
//
//  DESCRIPTION:    This function maps between power manager power
//                  states and video ioctl power states.
//
//  PARAMETERS:     dx - desired power manager device power state
//
//  RETURNS:        corresponding video ioctl power state
//
//------------------------------------------------------------------------------
VIDEO_POWER_STATE LcdcClass::PmToVideoPowerState(CEDEVICE_POWER_STATE dx)
{
    VIDEO_POWER_STATE vps;

    switch(dx)
    {
        // turn the display on
        case D0:
            vps = VideoPowerOn;
            break;

        // if asked for a state we don't support, go to the next lower one
        case D1:
        case D2:
        case D3:
        case D4:
            vps = VideoPowerOff;
            break;

        default:
            DEBUGMSG(GPE_ZONE_ERROR, (L"PmToVideoPowerState: mapping unknown PM state %d to VideoPowerOn\r\n", dx));
            vps = VideoPowerOn;
            break;
    }
    return vps;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:       VideoToPmPowerState
//
//  DESCRIPTION:    This function maps between video power states
//                  to PM power states.
//
//  PARAMETERS:     vps - desired video power state
//
//  RETURNS:        corresponding PM device power state
//
//------------------------------------------------------------------------------
CEDEVICE_POWER_STATE LcdcClass::VideoToPmPowerState(VIDEO_POWER_STATE vps)
{
    CEDEVICE_POWER_STATE dx;

    switch(vps)
    {
        case VideoPowerOn:
            dx = D0;
            break;

        case VideoPowerStandBy:
        case VideoPowerSuspend:
        case VideoPowerOff:
            dx = D4;
            break;

        default:
            dx = D0;
            DEBUGMSG(GPE_ZONE_WARNING, (L"VideoToPmPowerState: mapping unknown video state %d to pm state %d\r\n",
                     vps, dx));
            break;
    }

    return dx;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:       ConvertStringToGuid
//
//  DESCRIPTION:    This converts a string into a GUID.
//
//  PARAMETERS:
//                  pszGuid - pointer to GUID in string format
//                  pGuid - pointer to GUID struct for output
//
//  RETURNS:        returns TRUE if the conversion was successful
//
//------------------------------------------------------------------------------
BOOL LcdcClass::ConvertStringToGuid(LPCTSTR pszGuid, GUID *pGuid)
{
    UINT Data4[8];
    int  Count;
    BOOL fOk = FALSE;
    TCHAR *pszGuidFormat = _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

    DEBUGCHK(pGuid != NULL && pszGuid != NULL);
    __try
    {
        if (_stscanf_s(pszGuid, pszGuidFormat, &pGuid->Data1,
            &pGuid->Data2, &pGuid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3],
            &Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11)
        {
            for(Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++)
            {
                        pGuid->Data4[Count] = (UCHAR) Data4[Count];
            }
        }
        fOk = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return fOk;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:       GetRotateModeFromReg
//
//  DESCRIPTION:    This function is used to read the registry to get the initial
//                  rotation angle.
//
//  PARAMETERS:
//
//
//  RETURNS:        returns default rotation angle
//
//------------------------------------------------------------------------------
int LcdcClass::GetRotateModeFromReg()
{
    HKEY hKey;
    int RotateMode = DMDO_0;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\GDI\\ROTATION"),0,0, &hKey))
    {
        DWORD dwSize, dwAngle, dwType = REG_DWORD;
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                               TEXT("ANGLE"),
                                               NULL,
                                               &dwType,
                                               (LPBYTE)&dwAngle,
                                               &dwSize))
        {
            switch (dwAngle)
            {
            case 0:
                RotateMode = DMDO_0;
                break;
            case 90:
                RotateMode = DMDO_90;
                break;
            case 180:
                RotateMode = DMDO_180;
                break;
            case 270:
                RotateMode = DMDO_270;
                break;
            default:
                RotateMode = DMDO_0;
                break;
            }
        }

        RegCloseKey(hKey);
    }

    return RotateMode;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:       SetRotateParms
//
//  DESCRIPTION:    This function is used to set up the screen width and height
//                  based on the current rotation angle.
//
//  PARAMETERS:
//
//  RETURNS:
//
//------------------------------------------------------------------------------
void LcdcClass::SetRotateParms()
{
    int iswap;

        switch(m_iRotate)
        {
            case DMDO_0:
            case DMDO_180:
            m_nScreenHeightSave = m_nScreenHeight;
            m_nScreenWidthSave = m_nScreenWidth;
            break;
        case DMDO_90:
        case DMDO_270:
            iswap = m_nScreenHeight;
            m_nScreenHeight = m_nScreenWidth;
            m_nScreenWidth = iswap;
                m_nScreenHeightSave = m_nScreenWidth;
                m_nScreenWidthSave = m_nScreenHeight;
            break;
        default:
            m_nScreenHeightSave = m_nScreenHeight;
            m_nScreenWidthSave = m_nScreenWidth;
            break;
        }

    return;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:       DynRotate
//
//  DESCRIPTION:    This function Rotates the screen when DrvEscape gets a
//                  DRVESC_SETSCREENROTATION message.
//
//  PARAMETERS:
//                  int angle - angle to rotate
//
//  RETURNS:
//                  DISP_CHANGE_SUCCESSFUL
//
//------------------------------------------------------------------------------
LONG LcdcClass::DynRotate(int angle)
{
    GPESurf *pSurf = (GPESurf*)m_pPrimarySurface;

    if (angle == m_iRotate)
        return DISP_CHANGE_SUCCESSFUL;

    m_iRotate = angle;

    switch(m_iRotate)
        {
            case DMDO_0:
            case DMDO_180:
               m_nScreenHeight = m_nScreenHeightSave;
               m_nScreenWidth = m_nScreenWidthSave;
               break;
            case DMDO_90:
            case DMDO_270:
               m_nScreenHeight = m_nScreenWidthSave;
               m_nScreenWidth = m_nScreenHeightSave;
               break;
           default:
               m_nScreenHeight = m_nScreenHeightSave;
               m_nScreenWidth = m_nScreenWidthSave;
               break;
        }

    m_pMode->width = m_nScreenWidth;
    m_pMode->height = m_nScreenHeight;
    pSurf->SetRotation(m_nScreenWidth, m_nScreenHeight, angle);

    return DISP_CHANGE_SUCCESSFUL;
}

//------------------------------------------------------------------------------
//
//  FUNCTION:       CursorOn
//
//  DESCRIPTION:    This function is used to support software cursor.
//
//  PARAMETERS:
//
//  RETURNS:
//
//------------------------------------------------------------------------------
VOID LcdcClass::CursorOn (void)
{
    UCHAR   *ptrScreen = (UCHAR*)m_pPrimarySurface->Buffer();
    UCHAR   *ptrLine;
    UCHAR   *cbsLine;
    int     x, y;

    if (!m_CursorForcedOff && !m_CursorDisabled && !m_CursorVisible)
    {
        RECTL cursorRectSave = m_CursorRect;
        int   iRotate;
        RotateRectl(&m_CursorRect);
        for (y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
        {
            if (y < 0)
            {
                continue;
            }
            if (y >= m_nScreenHeightSave)
            {
                break;
            }

            ptrLine = &ptrScreen[y * m_pPrimarySurface->Stride()];
            cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * (m_CursorSize.x * (m_colorDepth >> 3))];

            for (x = m_CursorRect.left; x < m_CursorRect.right; x++)
            {
                if (x < 0)
                {
                    continue;
                }
                if (x >= m_nScreenWidthSave)
                {
                    break;
                }

                // x' = x - m_CursorRect.left; y' = y - m_CursorRect.top;
                // Width = m_CursorSize.x;   Height = m_CursorSize.y;
                switch (m_iRotate)
                {
                    case DMDO_0:
                        iRotate = (y - m_CursorRect.top)*m_CursorSize.x + x - m_CursorRect.left;
                        break;
                    case DMDO_90:
                        iRotate = (x - m_CursorRect.left)*m_CursorSize.x + m_CursorSize.y - 1 - (y - m_CursorRect.top);
                        break;
                    case DMDO_180:
                        iRotate = (m_CursorSize.y - 1 - (y - m_CursorRect.top))*m_CursorSize.x + m_CursorSize.x - 1 - (x - m_CursorRect.left);
                        break;
                    case DMDO_270:
                        iRotate = (m_CursorSize.x -1 - (x - m_CursorRect.left))*m_CursorSize.x + y - m_CursorRect.top;
                        break;
                    default:
                        iRotate = (y - m_CursorRect.top)*m_CursorSize.x + x - m_CursorRect.left;
                        break;
                }
                cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3)] = ptrLine[x * (m_colorDepth >> 3)];
                ptrLine[x * (m_colorDepth >> 3)] &= m_CursorAndShape[iRotate];
                ptrLine[x * (m_colorDepth >> 3)] ^= m_CursorXorShape[iRotate];
                if (m_colorDepth > 8)
                {
                    cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 1] = ptrLine[x * (m_colorDepth >> 3) + 1];
                    ptrLine[x * (m_colorDepth >> 3) + 1] &= m_CursorAndShape[iRotate];
                    ptrLine[x * (m_colorDepth >> 3) + 1] ^= m_CursorXorShape[iRotate];
                    if (m_colorDepth > 16)
                    {
                        cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 2] = ptrLine[x * (m_colorDepth >> 3) + 2];
                        ptrLine[x * (m_colorDepth >> 3) + 2] &= m_CursorAndShape[iRotate];
                        ptrLine[x * (m_colorDepth >> 3) + 2] ^= m_CursorXorShape[iRotate];
                    }
                }
            }
        }
        m_CursorRect = cursorRectSave;
        m_CursorVisible = TRUE;
        }
}

//------------------------------------------------------------------------------
//
//  FUNCTION:       CursorOff
//
//  DESCRIPTION:    This function is used to support software cursor.
//
//  PARAMETERS:
//
//  RETURNS:
//
//------------------------------------------------------------------------------
VOID LcdcClass::CursorOff (void)
{
    UCHAR   *ptrScreen = (UCHAR*)m_pPrimarySurface->Buffer();
    UCHAR   *ptrLine;
    UCHAR   *cbsLine;
    int     x, y;

    if (!m_CursorForcedOff && !m_CursorDisabled && m_CursorVisible)
    {
        RECTL rSave = m_CursorRect;
        RotateRectl(&m_CursorRect);
        for (y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
        {
            // clip to displayable screen area (top/bottom)
            if (y < 0)
            {
                continue;
            }
            if (y >= m_nScreenHeightSave)
            {
                break;
            }

            ptrLine = &ptrScreen[y * m_pPrimarySurface->Stride()];
            cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * (m_CursorSize.x * (m_colorDepth >> 3))];

            for (x = m_CursorRect.left; x < m_CursorRect.right; x++)
            {
                // clip to displayable screen area (left/right)
                if (x < 0)
                {
                    continue;
                }
                if (x >= m_nScreenWidthSave)
                {
                    break;
                }

                ptrLine[x * (m_colorDepth >> 3)] = cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3)];
                if (m_colorDepth > 8)
                {
                    ptrLine[x * (m_colorDepth >> 3) + 1] = cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 1];
                    if (m_colorDepth > 16)
                    {
                        ptrLine[x * (m_colorDepth >> 3) + 2] = cbsLine[(x - m_CursorRect.left) * (m_colorDepth >> 3) + 2];
                    }
                }
            }
        }
        m_CursorRect = rSave;
        m_CursorVisible = FALSE;
    }
}
ULONG LcdcClass::LcdcDrvEscape(SURFOBJ *pso, ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut)
{
    // support nothing
    return ESC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
// END OF FILE
//------------------------------------------------------------------------------
