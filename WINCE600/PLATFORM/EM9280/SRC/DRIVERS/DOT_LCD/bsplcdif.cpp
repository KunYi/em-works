//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// FILE:    bsplcdif.cpp
//
//  This file contains BSP part of the LCDIF driver implementation.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
#include <windows.h>
#include <winddi.h>
#pragma warning(push)
#pragma warning(disable: 4201 4100 4127)
#include <ddgpe.h>
#include <svsutil.hxx>
#pragma warning(pop)
#include <pm.h>

#include "display_controller_factory.h"
#include "display_controller.h"
#include "bsp.h"

//------------------------------------------------------------------------------
// Defines
#define NUM_MODES 1   // DISPLAY_MODE_DEVICE       

#define VIDEO_REG_PATH                        TEXT("SYSTEM\\GDI\\DRIVERS")
#define VIDEO_MEM_SIZE                        TEXT("VIDEOMEMSIZE")
#define ALIGN_REG_PATH                        TEXT("SYSTEM\\GDI\\DRIVERS")
#define ALIGN_REG_VALUE                       TEXT("ALIGNMENT")
#define DEFAULT_VIDEO_MEM_SIZE                (6*1024*1024) // if can not get from registry, then return 3M bytes for default video RAM size
#define PIXEL_DEPTH                           TEXT("Bpp")

//------------------------------------------------------------------------------
// Local Variables
DisplayController * pLCDDisplay = NULL;
DWORD dwPixelDepth = 0;
DWORD dwDisplayMode = 0;
ULONG g_PhysBase = 0;
PVOID g_pVirtBase = NULL;

static EGPEFormat eFormat[] =
{
    gpe16Bpp,
    gpe24Bpp,       //PXP doesn't support packed 24bit RGB888 format as input
    gpe32Bpp,
};

static EDDGPEPixelFormat ePixelFormat[] =
{
    ddgpePixelFormat_565,
    ddgpePixelFormat_8880,      //PXP doesn't support packed 24bit RGB888 format as input
    ddgpePixelFormat_8888,
};

static ULONG RGBBitMask[][3] = {
    { 0xF800, 0x07E0, 0x001F },
    { 0xFF0000, 0x00FF00, 0x0000FF },      //PXP doesn't support packed 24bit RGB888 format as input        
    { 0x00FF0000, 0x0000FF00, 0x000000FF }
};

//--------------------------------------------------------------------------
//
// Function: BSPGetModeNum
//
// This function returns the number of display modes supported by a driver. 
//
// Parameters:
//      None.
//
// Returns:
//      Returns the number of display modes supported by a driver.
//
//--------------------------------------------------------------------------
ULONG BSPGetModeNum(void)
{
    return NUM_MODES;
}

//--------------------------------------------------------------------------
//
// Function: BSPLoadPixelDepthFromRegistry
//
// This function loads pixel depth from registry.
//
// Parameters:
//      None.
//
// Returns:
//      Pixel depth find in registry.
//
//--------------------------------------------------------------------------
DWORD BSPLoadPixelDepthFromRegistry()
{
    LONG  error;
    HKEY  hKey;
    DWORD dwSize;
    
    // Open registry key for display driver
    error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, VIDEO_REG_PATH, 0 , 0, &hKey);
    if (error != ERROR_SUCCESS)
    {
        ERRORMSG(1,(TEXT("LoadPixelDepthFromRegistry: Failed to open reg path:%s [Error:0x%x]\r\n"), VIDEO_REG_PATH, error));
        goto _donePixel;
    }
    
    dwSize = sizeof(dwPixelDepth);
    error = RegQueryValueEx(hKey, PIXEL_DEPTH, NULL, NULL, (LPBYTE)&dwPixelDepth, (LPDWORD)&dwSize);
    if (error != ERROR_SUCCESS)
    {
        ERRORMSG(1,(TEXT("LoadPixelDepthFromRegistry: Failed to get display pixel depth, Error 0x%X\r\n"), error));
        goto _donePixel;
    }
    
_donePixel:
    // Close registry key
    RegCloseKey(hKey);
    
    RETAILMSG(1, (TEXT("LoadPixelDepthFromRegistry: %s!\r\n"), (error == ERROR_SUCCESS) ? L"succeeds" : L"fails"));
    
    return dwPixelDepth;

}

//------------------------------------------------------------------------------
//
// Function: BSPGetVideoMemorySize
//
// This function can return a specified framebuffer size. 
//
// Parameters:
//      None
//
// Returns:
//      Return wanted memory size.
//
//------------------------------------------------------------------------------
DWORD BSPGetVideoMemorySize()
{
	return pLCDDisplay->GetVideoMemorySize();   
	/*BOOL result = TRUE;
    LONG  error;
    HKEY  hKey;
    DWORD dwSize;
    ULONG iAlignment;
    UINT32 nVideoMemorySize = DEFAULT_VIDEO_MEM_SIZE;

    // Open registry key for display driver
    error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, VIDEO_REG_PATH, 0 , 0, &hKey);
    if (error != ERROR_SUCCESS)
    {
        RETAILMSG(1, (TEXT("GetVideoMemorySize: Failed to open reg path:%s [Error:0x%x]\r\n"), VIDEO_REG_PATH, error));
        result = FALSE;
        goto _doneVMem;
    }

    // Retrieve Video Memory Size from registry
    dwSize = sizeof(int);
    error = RegQueryValueEx(hKey, VIDEO_MEM_SIZE, NULL, NULL,(LPBYTE)&nVideoMemorySize, (LPDWORD)&dwSize);
    if (error != ERROR_SUCCESS) 
    {
        RETAILMSG(1, (TEXT("GetVMemSizeFromRegistry: Failed to get the video RAM size [Error:0x%x].  Setting to default.\r\n"),error));
        result = FALSE;
        nVideoMemorySize = DEFAULT_VIDEO_MEM_SIZE;
        goto _doneVMem;
    }

_doneVMem:

    // Aligned to 64k bytes
    iAlignment = (1UL << 0xF) - 1;
    nVideoMemorySize = (nVideoMemorySize + iAlignment) & (~iAlignment);

    // Close registry key
    RegCloseKey(hKey);

    RETAILMSG(1, (TEXT("GetVMemSizeFromRegistry: %s, size is %d!\r\n"), result ? L"succeeds" : L"fails", nVideoMemorySize));

    return nVideoMemorySize;*/
}

//------------------------------------------------------------------------------
//
// Function: BSPIsValidMode
//
// This function judges whether the display mode is supported. 
//
// Parameters:
//      modeNumber
//          [in] Display mode number
//
// Returns:
//      TRUE    supported
//      FALSE   not supported
//
//------------------------------------------------------------------------------
BOOL BSPIsValidMode(DWORD modeNumber)
{
    BOOL rc = FALSE;

    switch (modeNumber)
    {
    case DISPLAY_MODE_DEVICE:
        rc = TRUE;
        break;
    }
    return rc;
}

//------------------------------------------------------------------------------
//
// Function: BSPGetDisplayGuid
//
// This function gets GUID Info for display panel from registry. 
//
// Parameters:
//      modeNumber
//          [in/out] Pointer to guid
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void BSPGetDisplayGuid(GUID* guid)
{
    HKEY hKey;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\GDI\\DRIVERS"),
        0,0,
        &hKey)) {
            WCHAR guid_sz[100];
            DWORD dwSize, dwType;

            dwSize = sizeof(WCHAR) * 100;

            memset(guid_sz, 0, dwSize);

            if (RegQueryValueEx(hKey,
                TEXT("GUID"),
                NULL,
                &dwType,
                (LPBYTE) &guid_sz[0],
                &dwSize) == ERROR_SUCCESS) {
                    svsutil_ReadGuidW(guid_sz, guid, TRUE);
            } //if

            RETAILMSG(1, (TEXT("GetDisplayGuid:  %s\r\n"), guid_sz));
            RegCloseKey(hKey);
    } //if
} //BSPGetDisplayGuid

//------------------------------------------------------------------------------
//
// Function: BSPSetDisplayController
//
// This function sets display controller for display driver. 
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void BSPSetDisplayController()
{
    GUID guid;
    BSPGetDisplayGuid(&guid);

    pLCDDisplay = DisplayControllerFactory::GetDisplayController(&guid);
    pLCDDisplay->BacklightEnable(FALSE);
    pLCDDisplay->InitDisplay();
    dwDisplayMode = DISPLAY_MODE_DEVICE;

}

//------------------------------------------------------------------------------
//
// Function: BSPGetWidth
//
// This function gets width info of display according to different display mode. 
//
// Parameters:
//      modeNumber
//          [in] Display mode number
//
// Returns:
//      Width info value of display
//
//------------------------------------------------------------------------------
DWORD BSPGetWidth(DWORD modeNumber)
{
       return pLCDDisplay->GetWidth();
}

//------------------------------------------------------------------------------
//
// Function: BSPGetHeight
//
// This function gets height info of display according to different display mode. 
//
// Parameters:
//      modeNumber
//          [in] Display mode number
//
// Returns:
//      Height info value of display
//
//------------------------------------------------------------------------------
DWORD BSPGetHeight(DWORD modeNumber)
{
	return  pLCDDisplay->GetHeight();
}


BOOL BSPSetContrast( DWORD dwContrastLevel )
{
	return pLCDDisplay->SetContrast( dwContrastLevel );
}

BOOL BSPGetContrast( DWORD* dwContrastLevel, DWORD dwFlag )
{
	return pLCDDisplay->GetContrast( dwContrastLevel, dwFlag);
}
//------------------------------------------------------------------------------
//
// Function: BSPGetModeInfoEx
//
// This function gets GPE mode info according different mode number. 
//
// Parameters:
//      pModeEx
//          [in/out] Pointer to GPEModeEx
//
//      modeNumber
//          [in] Display mode number
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void BSPGetModeInfoEx(GPEModeEx* pModeEx, int modeNumber)
{
    pModeEx->modeInfo.modeId = modeNumber;
    pModeEx->modeInfo.Bpp =  dwPixelDepth?dwPixelDepth:16;
    
    int nBPP = 0;
    nBPP = pModeEx->modeInfo.Bpp/8 - 2;
    pModeEx->modeInfo.format = eFormat[nBPP];
    pModeEx->ePixelFormat = ePixelFormat[nBPP];       
        
    pModeEx->modeInfo.frequency = (modeNumber==DISPLAY_MODE_PAL)?50:60;
    pModeEx->dwPixelFourCC = 0;
    pModeEx->dwPixelFormatData = 0;
    pModeEx->lPitch = ((pModeEx->modeInfo.width * (pModeEx->modeInfo.Bpp / 8) + 3) >> 2) << 2;

    pModeEx->dwRBitMask     = RGBBitMask[nBPP][0];
    pModeEx->dwGBitMask     = RGBBitMask[nBPP][1];
    pModeEx->dwBBitMask     = RGBBitMask[nBPP][2];
    pModeEx->dwAlphaBitMask = 0x00000000;

    if (pModeEx->modeInfo.Bpp == 32)
        pModeEx->dwAlphaBitMask = 0xFF000000;
    else
        pModeEx->dwAlphaBitMask = 0x00000000;            
        
    pModeEx->dwSize = sizeof(GPEModeEx);
    pModeEx->dwDriverSignature = 0xf000baaa;
    pModeEx->dwVersion =  GPEMODEEX_CURRENTVERSION;
    pModeEx->dwReserved0 =
    pModeEx->dwReserved1 =
    pModeEx->dwReserved2 =
    pModeEx->dwReserved3 = 0;

    pModeEx->modeInfo.height = BSPGetHeight(modeNumber);
    pModeEx->modeInfo.width = BSPGetWidth(modeNumber);

}

void BSPGetModeInfo(GPEMode* pMode, int modeNumber)
{
	pMode->height = BSPGetHeight(modeNumber);
	pMode->width = BSPGetWidth(modeNumber);
	pMode->modeId = 0;
	pMode->Bpp = 4;
	pMode->frequency = 60;		// not too important
	pMode->format = gpe4Bpp;
}

//------------------------------------------------------------------------------
//
// Function: IsTvMode
//
// This function judges whether current display mode is TV mode. 
//
// Parameters:
//      None
//
// Returns:
//      TRUE    TV mode
//      FALSE   Not TV mode
//
//------------------------------------------------------------------------------
BOOL IsTvMode()
{
    BOOL rc = FALSE;

    return rc;
}

//------------------------------------------------------------------------------
//
// Function: BSPSetMode
//
// This method requests a specific mode for the display.
//
// Parameters:
//      modeNumber
//          [in] Mode number to set.
//    
// Returns:
//      TRUE            successful
//      FALSE          failed
//
//------------------------------------------------------------------------------
BOOL BSPSetMode(DWORD modeNumber)
{

    BOOL rc = FALSE;

    switch (modeNumber)
    {
    case DISPLAY_MODE_DEVICE:

        // power off everything
        LCDIFPowerDown(TRUE);

        // Must update our mode member prior to calling the
        // power handler so that the TV power state is not changed

        dwDisplayMode = modeNumber;
        pLCDDisplay->DispDrvrPowerHandler(POWERON, TRUE, TRUE,TRUE);
        RETAILMSG(1, (TEXT("BSPSetMode: Inform BackLight driver goto D0 power state\r\n")));
        DevicePowerNotify(TEXT("BKL1:"),D0,POWER_NAME);
        rc = TRUE;
        break;
    }
    return rc;
    
}

//------------------------------------------------------------------------------
//
//  FUNCTION:     DispDrvrPowerHandler
//
//  DESCRIPTION:  
//  This function controls the power state of the LCD and TV out.
//
// Parameters:
//      bOn
//        [IN] bOn = FALSE ( Power down )
//             bOn = TRUE ( Power up )
//
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPDisplayPowerHandler(BOOL bOn)
{
    if(bOn)
    {
            pLCDDisplay->DispDrvrPowerHandler(POWERON, FALSE, TRUE,TRUE);            
    }
    else
    {
            pLCDDisplay->DispDrvrPowerHandler(POWEROFF, FALSE, TRUE,TRUE);  
    }
}

//------------------------------------------------------------------------------
//
//  FUNCTION:     BSPSetDisplayBuffer
//
//  DESCRIPTION:  
//  This function sets display framebuffer address.
//
// Parameters:
//      PhysBase
//        [IN] physical address for framebuffer
//      VirtBase
//        [IN] virtual address for framebuffer
//
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPSetDisplayBuffer(ULONG PhysBase, PVOID pVirtBase)
{
    pLCDDisplay->SetDisplayBuffer( PhysBase, pVirtBase );
}

//------------------------------------------------------------------------------
//
// Function: BSPGetCacheMode
//
// Return whether the Cache mode is cached, write-through
// or non-cached, bufferable.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if cache mode is cached, write-through.
//      FALSE if non-cached, bufferable.
//
//------------------------------------------------------------------------------
BOOL BSPGetCacheMode()
{
    return BSP_VID_MEM_CACHE_WRITETHROUGH;
}

//------------------------------------------------------------------------------
//
// Function: BSPGetOverlayAlign
//
// Return overlay alignment
//
// Parameters:
//      None.
//
// Returns:
//      Overlay alignment
//
//------------------------------------------------------------------------------
UINT32 BSPGetOverlayAlign()
{
    BOOL result = TRUE;
    LONG  error;
    HKEY  hKey;
    DWORD dwSize;
    UINT32 OverlayAlign = 8;
            
    // Open registry key for display driver
    error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ALIGN_REG_PATH, 0 , 0, &hKey);
    if (error != ERROR_SUCCESS)
    {
        RETAILMSG(1, (TEXT("GetBlockSize: Failed to open reg path:%s [Error:0x%x]\r\n"), ALIGN_REG_PATH, error));
        result = FALSE;
        goto _doneGetOverlayAlign;
    }
            
    // Retrieve alignment value from registry
    dwSize = sizeof(int);
    error = RegQueryValueEx(hKey, ALIGN_REG_VALUE, NULL, NULL,(LPBYTE)&OverlayAlign, (LPDWORD)&dwSize);
    if (error != ERROR_SUCCESS) 
    {
        RETAILMSG(1, (TEXT("GetBlockSize: Failed to get the alignment value [Error:0x%x].  Setting to default.\r\n"),error));
        result = FALSE;
        OverlayAlign = 8;
        goto _doneGetOverlayAlign;
    }
    
_doneGetOverlayAlign:
                    
    // Close registry key
    RegCloseKey(hKey);
            
    RETAILMSG(1, (TEXT("GetOverlayAlign: %s, size is %d!\r\n"), result ? L"succeeds" : L"fails", OverlayAlign));

    return OverlayAlign;

}

//------------------------------------------------------------------------------
//
// Function: BSPEnableBackLight
//
// Enable/Disable BackLight.
//
// Parameters:
//      Enable.
//          [in] TRUE.     Enable backlight
//                FALSE.    Disable backlight
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPBacklightEnable(BOOL Enable)
{
    pLCDDisplay->BacklightEnable(Enable);
}

BOOL BSPInitLCD(  )
{
	pLCDDisplay->InitLCD( );
	return TRUE;
}

void BSPFrameBufferUpdate( PVOID pSurface )
{
	pLCDDisplay->Update( pSurface );
}
DWORD BSPGetIRQ( )
{
	return IRQ_LCDIF;
}