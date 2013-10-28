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
//  Copyright (C) 2005-2006, Freescale Semiconductor, 2007, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// File:        /drivers/display/DDLCDC/DDLcdcMisc.cpp
// Purpose:     miscellaneous functions
//
//------------------------------------------------------------------------------

#include "precomp.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#define SPLASH_SCREEN_BACKGROUND    0xFF        // white


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


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

ULONG MX27DDLcdc::DrvEscapeGAPI(ULONG iEsc, ULONG cjIn, void * pvIn, ULONG cjOut, void * pvOut)
{
    return ESC_FAILED;
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
ULONG MX27DDLcdc::DrvEscape(SURFOBJ * pso, ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut)
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
                if ((EscapeFunction == SETPOWERMANAGEMENT)       ||
                    (EscapeFunction == GETPOWERMANAGEMENT)       ||
                    (EscapeFunction == IOCTL_POWER_CAPABILITIES) ||
                    (EscapeFunction == IOCTL_POWER_QUERY)        ||
                    (EscapeFunction == IOCTL_POWER_SET)          ||
                    (EscapeFunction == IOCTL_POWER_GET)          ||
                    (EscapeFunction == DRVESC_GETSCREENROTATION) ||
                    (EscapeFunction == DRVESC_SETSCREENROTATION)
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

        case SETPOWERMANAGEMENT :
            //DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc DrvEscape: SETPOWERMANAGEMENT\r\n")));
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
                                    DEBUGMSG(GPE_ZONE_WARNING,(TEXT("MX27DDLcdc DrvEscape: Requesting POWER_STATE_IDLE\r\n")));
                                    #if (!defined(ULDR))
                                    err = SetSystemPowerState(0, POWER_STATE_IDLE, POWER_FORCE);
                                    #endif
                                    INSREG32BF(&m_pLcdcReg->PCCR, LCDC_PCCR_PW, 0);
                                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc DrvEscape: BackLight Off for screen toggle\r\n")));
                                    break;

                            case VideoPowerOn:
                                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc DrvEscape: Requesting POWER_STATE_ON\r\n")));
                                    #if (!defined(ULDR))
                                    err = SetSystemPowerState(0, POWER_STATE_ON, POWER_FORCE);
                                    #endif
                                    INSREG32BF(&m_pLcdcReg->PCCR, LCDC_PCCR_PW, 100);
                                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc DrvEscape: BackLight ON for screen toggle\r\n")));
                                    break;

                            default:
                                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc DrvEscape: SetPowerManagement : unsupported power state requested\r\n")));
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
            //DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc DrvEscape: GETPOWERMANAGEMENT\r\n")));
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
            //DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc DrvEscape: IOCTL_POWER_CAPABILITIES\r\n")));
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
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc DrvEscape: IOCTL_POWER_QUERY %u %s\r\n"),
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
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc DrvEscape: %s IOCTL_POWER_GET: passing back %u\r\n"), m_szDevName, CurrentDx));
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
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc DrvEscape: %s IOCTL_POWER_SET %u: passing back %u\r\n"), m_szDevName, NewDx, m_Dx));
                }
                else
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc DrvEscape: IOCTL_POWER_SET: invalid state request %u\r\n"), NewDx));
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
                retval = DISP_CHANGE_BADMODE;
            }
            break;

        default :
            retval = DrvEscapeGAPI(iEsc, cjIn, pvIn,cjOut, pvOut);
            break;
    }

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: IsBusy
//
// Method to determine whether LCDC is available
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
int MX27DDLcdc::IsBusy(VOID)
{
    int result = FALSE;
    UINT32 iIntrStatus = 0;

    EnterCriticalSection(&m_CriticalSection);

    if (m_bFlipInProgress)
        result = TRUE;

    LeaveCriticalSection(&m_CriticalSection);

    return result;
}


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
int MX27DDLcdc::InVBlank(VOID)
{
    return (!IsBusy());
}


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
VOID MX27DDLcdc::WaitForNotBusy(VOID)
{
    UINT8 retry = MAX_BUSY_RETRY; // Wait for 5 milliseconds is long enough.

    while(IsBusy() && retry--)
    {
        Sleep(0);
    }
    return;
}


//------------------------------------------------------------------------------
//
// Function: WaitForVerticalBlank
//
// This callback function helps the application synchronize itself
// with the vertical blanking interval (VBI).
//
// Parameters:
//      pd
//          [in, out] Pointer to a DDHAL_WAITFORVERTICALBLANKDATA structure
//          that contains the vertical blank information.
//
// Returns:
//      DDHAL_DRIVER_HANDLED
//      DDHAL_DRIVER_NOTHANDLED
//
//------------------------------------------------------------------------------
DWORD MX27DDLcdc::WaitForVerticalBlank(LPDDHAL_WAITFORVERTICALBLANKDATA pd)
{
    /*
    typedef struct _DDHAL_WAITFORVERTICALBLANKDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL lpDD;               // driver struct
        DWORD                   dwFlags;            // flags
        DWORD                   bIsInVB;            // is in vertical blank
        DWORD                   hEvent;             // event
        HRESUL                  ddRVal;             // return value
        LPDDHAL_WAITFORVERTICALBLANK WaitForVerticalBlank;
                                                    // PRIVATE: ptr to callback
    } DDHAL_WAITFORVERTICALBLANKDATA;
    */

    switch(pd->dwFlags)
    {
        case DDWAITVB_I_TESTVB:
            // If TESTVB, it's just a request for the current vertical blank
            if(InVBlank())
                pd->bIsInVB = TRUE;
            else
                pd->bIsInVB = FALSE;

            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;

        case DDWAITVB_BLOCKBEGIN:
            // Returns when the vertical blank interval begins.
            WaitForNotBusy();
            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;

        case DDWAITVB_BLOCKEND:
            // Returns when the vertical blank interval ends and display begins.
            pd->ddRVal = DDERR_NOVSYNCHW;
            return DDHAL_DRIVER_HANDLED;
    }

    return DDHAL_DRIVER_NOTHANDLED;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     GetModeFromRegistry
//
//  DESCRIPTION:  Get display mode information from registry
//                requested by user
//
//  PARAMETERS:
//
//  RETURNS:
//                TRUE: successful
//                FALSE: failed
//
//------------------------------------------------------------------------------
/*
BOOL MX27DDLcdc::GetModeFromRegistry(VOID)
{
    BOOL result = TRUE;
    LONG  error;
    HKEY  hKey;
    DWORD dwSize;

    // Open registry key for display driver
    error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, DRIVER_REGISTRY_STRING, 0 , 0, &hKey);
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(GPE_ZONE_ERROR,(TEXT("MX27DDLcdc GetModeFromRegistry: Failed to open reg path:%s [Error:0x%x]\r\n"), DRIVER_REGISTRY_STRING, error));
        result = FALSE;
        goto _done;
    }

    // Retrieve screen width from registry
    dwSize = sizeof(int);
    error = RegQueryValueEx(hKey, DRIVER_REGISTRY_STRING_CX, NULL, NULL,(LPBYTE)&m_nScreenWidth, (LPDWORD)&dwSize);
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(GPE_ZONE_ERROR,(TEXT("MX27DDLcdc GetModeFromRegistry: Failed to get the screen width [Error:0x%x]\r\n"),error));
        result = FALSE;
        goto _done;
    }

    // Retrieve screen height from registry
    dwSize = sizeof(int);
    error = RegQueryValueEx(hKey, DRIVER_REGISTRY_STRING_CY, NULL, NULL,(LPBYTE)&m_nScreenHeight, (LPDWORD)&dwSize);
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(GPE_ZONE_ERROR,(TEXT("MX27DDLcdc GetModeFromRegistry: Failed to get the screen height [Error:0x%x]\r\n"),error));
        result = FALSE;
        goto _done;
    }

    // Retrieve color depth from registry
    dwSize = sizeof(int);
    error = RegQueryValueEx(hKey, DRIVER_REGISTRY_STRING_BPP, NULL, NULL,(LPBYTE)&m_nScreenBpp, (LPDWORD)&dwSize);
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(GPE_ZONE_ERROR,(TEXT("MX27DDLcdc GetModeFromRegistry: Failed to get the color depth [Error:0x%x]\r\n"),error));
        result = FALSE;
        goto _done;
    }

_done:
    // Close registry key
    RegCloseKey(hKey);

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MX27DDLcdc GetModeFromRegistry: %s!\r\n"), result ? L"succeeds" : L"fails"));

    return result;
}
*/

//------------------------------------------------------------------------------
//
//  FUNCTION:     GetVMemSizeFromRegistry
//
//  DESCRIPTION:  Get the size of video RAM requested by user from
//                registry.
//
//  PARAMETERS:
//
//  RETURNS:
//                TRUE: successful
//                FALSE: failed
//
//------------------------------------------------------------------------------
/*
BOOL MX27DDLcdc::GetVMemSizeFromRegistry(VOID)
{
    BOOL result = TRUE;
    LONG  error;
    HKEY  hKey;
    DWORD dwSize;

    // Open registry key for display driver
    error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, DRIVER_REGISTRY_STRING, 0 , 0, &hKey);
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(GPE_ZONE_ERROR,(TEXT("MX27DDLcdc GetVMemSizeFromRegistry: Failed to open reg path:%s [Error:0x%x]\r\n"), DRIVER_REGISTRY_STRING, error));
        result = FALSE;
        goto _done;
    }

    // Retrieve screen width from registry
    dwSize = sizeof(int);
    error = RegQueryValueEx(hKey, DRIVER_REGISTRY_STRING_MEM_SIZE, NULL, NULL,(LPBYTE)&m_nVideoMemorySize, (LPDWORD)&dwSize);
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(GPE_ZONE_ERROR,(TEXT("MX27DDLcdc GetVMemSizeFromRegistry: Failed to get the vidoe RAM size [Error:0x%x]\r\n"),error));
        result = FALSE;
        m_nVideoMemorySize = 0;
        goto _done;
    }

_done:
    // Close registry key
    RegCloseKey(hKey);

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MX27DDLcdc GetVMemSizeFromRegistry: %s, size is %d!\r\n"), result ? L"succeeds" : L"fails", m_nVideoMemorySize));

    return result;
}
*/

//------------------------------------------------------------------------------
//
//  FUNCTION:     LoadFreescaleLogo
//
//  DESCRIPTION:  Load the Freescale logon onto screen.
//
//  PARAMETERS:
//
//  RETURNS:
//
//------------------------------------------------------------------------------
/*
VOID MX27DDLcdc::LoadFreescaleLogo(GPESurf * pSurf)
{
    if(pSurf)
    {
        LoadFreescaleLogo(pSurf->Buffer());
    }
}

VOID MX27DDLcdc::LoadFreescaleLogo(VOID * pFramePointer)
{
    if(pFramePointer)
    {
        BYTE *pfb = (BYTE *)pFramePointer;
        int bmpWidth, bmpHeight;
        int h, w, pixelBytes;
        int i, j;

        pixelBytes = m_nScreenBpp/8;

        // Sainty check
        if(bitmap_width * bitmap_height * pixelBytes != sizeof(bitmap_data))
        {
            DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc LoadFreescaleLogo: un-matched logo, width %d, height %d, size %d\r\n"), bitmap_width, bitmap_height, sizeof(bitmap_data)));
            DEBUGMSG(GPE_ZONE_WARNING, (TEXT("MX27DDLcdc LoadFreescaleLogo: Splash screen is NOT loaded!\r\n")));
            return;
        }

        bmpWidth  = (bitmap_width  > m_nScreenWidthSave)  ? m_nScreenWidthSave  : bitmap_width;
        bmpHeight = (bitmap_height > m_nScreenHeightSave) ? m_nScreenHeightSave : bitmap_height;

        w = (m_nScreenWidthSave - bmpWidth) / 2;
        h = (m_nScreenHeightSave - bmpHeight) / 2;

        // Pre-set the whole screen as white color
        memset(pfb, SPLASH_SCREEN_BACKGROUND, m_nScreenStride * m_nScreenHeightSave);

        // Copy and centre splash screen in framebuffer.
        j = bmpWidth * pixelBytes;
        pfb += w * pixelBytes + h * m_nScreenStride;
        for(i = 0; i < bmpHeight; i++)
        {

            memcpy(pfb, &bitmap_data[i * bitmap_width], j);
            pfb += m_nScreenStride;
        }
    }
}
*/
