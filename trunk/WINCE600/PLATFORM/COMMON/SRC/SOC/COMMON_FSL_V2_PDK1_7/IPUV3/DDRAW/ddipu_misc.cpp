//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddipu_misc.cpp
//
//  Implementation of DDIPU miscellaneous functions.
//
//------------------------------------------------------------------------------
#include "precomp.h"
#include <cmnintrin.h>

// Include GAPI support only for Windows Mobile builds.
#if defined(BSP_POCKETPC) || defined(BSP_SMARTPHONE)
#include <winuserm.h> // Needed for virtual keypad VK keycodes.
#include <gxinfo.h>   // Needed for GAPI data structure definitions.
#endif



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
extern BOOL VMSetAttributeEx(pVMSetAttributeExData pData, VM_ATTRIBUTE * pOrgAttr);


// Only include GAPI support for Windows Mobile builds.
#if defined(BSP_POCKETPC) || defined(BSP_SMARTPHONE)
//------------------------------------------------------------------------------
//
// Function: GetGameXInfo
//
// This routine returns the display driver information needed for GAPI support.
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
//     -1               Error, failed to return display driver settings.
//      0               Not supported.
//      1               Successfully returned display driver settings.
//
//------------------------------------------------------------------------------
int DDIPU::GetGameXInfo( ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut,
                             PVOID pvOut )
{
    int     RetVal = 0;     // Default not supported
    GXDeviceInfo * pgxoi;

    // We only provide GAPI support for 16-bit RGB565.
    //
    if ((cjOut >= sizeof(GXDeviceInfo)) && (pvOut != NULL)
        && (m_pMode->Bpp == 16))
    {
        if (((GXDeviceInfo *) pvOut)->idVersion == kidVersion100)
        {
            pgxoi = (GXDeviceInfo *) pvOut;

            // This is the only value defined in the GXDeviceInfo online help.
            //
            pgxoi->idVersion = kidVersion100;

            // Return framebuffer configuration information.
            //
            pgxoi->pvFrameBuffer = (void *)m_pPrimarySurface->Buffer();
            pgxoi->cbStride = m_pPrimarySurface->Stride();
            pgxoi->cxWidth = m_pPrimarySurface->Width();
            pgxoi->cyHeight = m_pPrimarySurface->Height();

            // For 16-bit RGB565 direct mapped (no palette) mode.
            //
            pgxoi->cBPP = 16;
            pgxoi->ffFormat= kfDirect | kfDirect565;

            // Define virtual keycodes for portrait orientation controls.
            // This is simply the normal orientation for the rocker switch.
            //
            pgxoi->vkButtonUpPortrait     = VK_UP;
            pgxoi->vkButtonDownPortrait   = VK_DOWN;
            pgxoi->vkButtonLeftPortrait   = VK_LEFT;
            pgxoi->vkButtonRightPortrait  = VK_RIGHT;

            // Define virtual keycodes for landscape orientation controls.
            // For the landscape orientation, just take both the normal
            // display and keypad orientation and rotate it 90 degrees
            // clockwise. This results in the following mapping for the
            // rocker switch.
            //
            pgxoi->vkButtonUpLandscape    = VK_LEFT;
            pgxoi->vkButtonDownLandscape  = VK_RIGHT;
            pgxoi->vkButtonLeftLandscape  = VK_DOWN;
            pgxoi->vkButtonRightLandscape = VK_UP;

            // Define A, B, C, and rotate display action keys on the keypad
            // for portrait mode.
            //
            pgxoi->vkButtonAPortrait      = VK_TSOFT1; // Softkey 1 button
            pgxoi->vkButtonBPortrait      = VK_TSOFT2; // Softkey 2 button
            pgxoi->vkButtonCPortrait      = VK_TSTAR;  // Asterisk button
            pgxoi->vkButtonStartPortrait  = VK_ACTION; // Send button

            // Define A, B, C, and rotate display action keys for landscape
            // mode. Unlike the rocker switch, we don't really want to remap
            // the action keys when changing the display orientation.
            //
            pgxoi->vkButtonALandscape     = VK_TSOFT1; // Softkey 1 button
            pgxoi->vkButtonBLandscape     = VK_TSOFT2; // Softkey 2 button
            pgxoi->vkButtonCLandscape     = VK_TSTAR;  // Asterisk button
            pgxoi->vkButtonStartLandscape = VK_ACTION; // Send button

            // Define display (X,Y) positions for all buttons. This assumes
            // that the default startup orientation is portrait mode. Note
            // that the following (X,Y) coordinates are given in screen
            // coordinates but do not map directly to the physical screen
            // display.
            //
            // Instead, imagine a "virtual display" that extends below the
            // physical LCD panel for another 240 pixels and with a width of
            // 240 pixels. Then the following coordinates approximately maps
            // the actual keypad to this extended virtual display.
            //
            pgxoi->ptButtonUp.x    = 100; // Position of rocker UP button.
            pgxoi->ptButtonUp.y    = 330;

            pgxoi->ptButtonDown.x  = 100; // Position of rocker DOWN button.
            pgxoi->ptButtonDown.y  = 390;

            pgxoi->ptButtonLeft.x  = 70;  // Position of rocker LEFT button.
            pgxoi->ptButtonLeft.y  = 360;

            pgxoi->ptButtonRight.x = 130; // Position of rocker RIGHT button.
            pgxoi->ptButtonRight.y = 360;

            pgxoi->ptButtonA.x = 50;      // Position of the KEY 1 button.
            pgxoi->ptButtonA.y = 330;

            pgxoi->ptButtonB.x = 150;     // Position of the KEY 2 button.
            pgxoi->ptButtonB.y = 330;

            pgxoi->ptButtonC.x = 50;      // Position of the ASTERISK button.
            pgxoi->ptButtonC.y = 440;

            pgxoi->ptButtonStart.x = 20;  // Position of the SEND button.
            pgxoi->ptButtonStart.y = 330;

            // These fields are currently unused.
            pgxoi->pvReserved1 = (void *) 0;
            pgxoi->pvReserved2 = (void *) 0;

            RetVal = 1;
        }
        else
        {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = -1;
        }
    }
    else
    {
        SetLastError (ERROR_INVALID_PARAMETER);
        RetVal = -1;
    }

    return(RetVal);
}
#endif // BSP_POCKETPC || BSP_SMARTPHONE

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
ULONG DDIPU::DrvEscapeGAPI(ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut,
                               PVOID pvOut)
{
    ULONG rc = (ULONG) ESC_FAILED;

#if defined(BSP_POCKETPC) || defined(BSP_SMARTPHONE)

    switch(iEsc)
    {
        case QUERYESCSUPPORT:

            DWORD EscapeFunction;
            EscapeFunction = *(DWORD *)pvIn;

            // GETGXINFO is the only additional GAPI-specific IOCTL code
            // that we need to support.
            if (EscapeFunction == GETGXINFO)
            {
                rc = ESC_SUCCESS;
            }
            break;

        case GETGXINFO:

            if (GetGameXInfo(iEsc, cjIn, pvIn, cjOut, pvOut) == 1)
            {
                rc = ESC_SUCCESS;
            }
            break;
    }
#else
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(iEsc);
    UNREFERENCED_PARAMETER(cjIn);
    UNREFERENCED_PARAMETER(pvIn);
    UNREFERENCED_PARAMETER(cjOut);
    UNREFERENCED_PARAMETER(pvOut);
#endif

    return rc;
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
ULONG DDIPU::DrvEscape(SURFOBJ * pso, ULONG iEsc, ULONG cjIn, PVOID pvIn, ULONG cjOut, PVOID pvOut)
{
    int retval = ESC_FAILED;
    PVIDEO_POWER_MANAGEMENT psPowerManagement;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pso);

    switch(iEsc)
    {
        case QUERYESCSUPPORT:
            if(pvIn != NULL && cjIn == sizeof(DWORD))
            {
                // Query DrvEscap support functions
                DWORD EscapeFunction;
                EscapeFunction = *(DWORD *)pvIn;
                if ((EscapeFunction == QUERYESCSUPPORT)          ||
                    (EscapeFunction == SETPOWERMANAGEMENT)       ||
                    (EscapeFunction == GETPOWERMANAGEMENT)       ||
                    (EscapeFunction == IOCTL_POWER_CAPABILITIES) ||
                    (EscapeFunction == IOCTL_POWER_QUERY)        ||
                    (EscapeFunction == IOCTL_POWER_SET)          ||
                    (EscapeFunction == IOCTL_POWER_GET)          ||
#if (UNDER_CE < 600)
// WinCE 6.0: Both GETVFRAMEPHYSICAL and GETVFRAMELEN are not defined.
                    (EscapeFunction == GETVFRAMEPHYSICAL)        ||
                    (EscapeFunction == GETVFRAMELEN)             ||
#endif
                    (EscapeFunction == DRVESC_GETSCREENROTATION) ||
                    (EscapeFunction == DRVESC_SETSCREENROTATION) ||
                    (EscapeFunction == DISPLAY_SET_PRIMARY)      ||
                    (EscapeFunction == DISPLAY_GET_OUTPUT_FREQUENCY)  ||
                    (EscapeFunction == DISPLAY_SET_OUTPUT_FREQUENCY)  ||
                    (EscapeFunction == CGMSA_GET_OUTPUT_MODE)    ||
                    (EscapeFunction == CGMSA_SET_OUTPUT_MODE)    ||
                    (EscapeFunction == DISPLAY_SET_GAMMA_VALUE)  ||
                    (EscapeFunction == DISPLAY_GET_GAMMA_VALUE)  ||                    
                    (EscapeFunction == DISPLAY_DLS_GET_CSC)      ||
                    (EscapeFunction == DISPLAY_DLS_SET_CSC)      ||
                    (EscapeFunction == DISPLAY_IS_VIDEO_INTERLACED) ||
                    (EscapeFunction == DISPLAY_ENABLE_DMI) ||
                    (EscapeFunction == DISPLAY_DISABLE_DMI) ||
                    (EscapeFunction == VM_SETATTEX) ||
                    (EscapeFunction == VF_GET_DISPLAY_INFO))
                {
                    retval = ESC_SUCCESS;
                }
                else
                {
                    retval = DrvEscapeGAPI(iEsc, cjIn, pvIn, cjOut, pvOut);
                }

                if(retval != ESC_SUCCESS)
                    SetLastError(ERROR_INVALID_PARAMETER);
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);
            break;

        case SETPOWERMANAGEMENT :
            DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DDIPU DrvEscape: SETPOWERMANAGEMENT\r\n")));
#if 0 // Remove-W4: Warning C4706 workaround
            if (psPowerManagement = (PVIDEO_POWER_MANAGEMENT) pvIn)
#else
            if ((psPowerManagement = (PVIDEO_POWER_MANAGEMENT) pvIn) != 0)
#endif
            {
                if (cjIn >= sizeof(VIDEO_POWER_MANAGEMENT))
                {
                        switch(psPowerManagement->PowerState)
                        {
                            case VideoPowerOff:
                                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DDIPU DrvEscape: BackLight Off for screen toggle\r\n")));
                                // turn on display
                                SetDisplayPower(D0);
                                retval = ESC_SUCCESS;
                                break;

                            case VideoPowerOn:
                                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DDIPU DrvEscape: BackLight ON for screen toggle\r\n")));
                                // turn off display
                                SetDisplayPower(D4);
                                retval = ESC_SUCCESS;
                                break;

                            default:
                                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DDIPU DrvEscape: SetPowerManagement : unsupported power state requested\r\n")));
                                break;
                        }
               }
                else
                       SetLastError(ERROR_INVALID_PARAMETER);
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);

            break;

        case GETPOWERMANAGEMENT:
            DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DDIPU DrvEscape: GETPOWERMANAGEMENT\r\n")));
#if 0 // Remove-W4: Warning C4706 workaround
            if (psPowerManagement = (PVIDEO_POWER_MANAGEMENT) pvOut)
#else
            if ((psPowerManagement = (PVIDEO_POWER_MANAGEMENT) pvOut) != 0)
#endif
            {
                if (cjOut >= sizeof(VIDEO_POWER_MANAGEMENT))
                {
                    psPowerManagement->Length = sizeof(VIDEO_POWER_MANAGEMENT);
                    psPowerManagement->DPMSVersion = 0x0100;
                    psPowerManagement->PowerState = PmToVideoPowerState(m_Dx);
                    retval = ESC_SUCCESS;
                }
                else
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                }
            }
            else
            {
                SetLastError(ERROR_INVALID_PARAMETER);
            }

            break;

        case IOCTL_POWER_CAPABILITIES:
            DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DDIPU DrvEscape: IOCTL_POWER_CAPABILITIES\r\n")));
            if(pvOut != NULL && cjOut == sizeof(POWER_CAPABILITIES))
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES)pvOut;
                memset(ppc, 0, sizeof(POWER_CAPABILITIES));
                ppc->DeviceDx = 0x11;    // support D0 and D4
                ppc->WakeFromDx = 0x00; // No wake capability
                ppc->InrushDx = 0x00;        // No in rush requirement
                ppc->Power[D0] = (DWORD) PwrDeviceUnspecified;
                ppc->Power[D1] = (DWORD) PwrDeviceUnspecified;
                ppc->Power[D2] = (DWORD) PwrDeviceUnspecified;
                ppc->Power[D3] = (DWORD) PwrDeviceUnspecified;
                ppc->Power[D4] = 0;
                ppc->Latency[D0] = 0;
                ppc->Latency[D1] = (DWORD) PwrDeviceUnspecified;
                ppc->Latency[D2] = (DWORD) PwrDeviceUnspecified;
                ppc->Latency[D3] = (DWORD) PwrDeviceUnspecified;
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
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DDIPU DrvEscape: IOCTL_POWER_QUERY %u %s\r\n"),
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
                DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DDIPU DrvEscape: %s IOCTL_POWER_GET: passing back %u\r\n"), m_szDevName, CurrentDx));
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
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DDIPU DrvEscape: %s IOCTL_POWER_SET %u: passing back %u\r\n"), m_szDevName, NewDx, m_Dx));
                }
                else
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DDIPU DrvEscape: IOCTL_POWER_SET: invalid state request %u\r\n"), NewDx));
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
                retval = SetRotation( m_pMode->modeId, cjIn );
            }
            else
            {
                retval = DISP_CHANGE_BADMODE;
            }
            break;
            
        case DISPLAY_SET_PRIMARY:
            //if the address is NULL, the original primary surface will be restored.
            if(pvIn && cjIn == sizeof(DISPLAY_PRIMARY))
            {
                PDISPLAY_PRIMARY DispPri = (PDISPLAY_PRIMARY)pvIn;
                if(DispPri->id == DISPLAY_ID_MAIN_DISPLAY)
                {
                    if(DispPri->phyAddr!= NULL)
                        DisplaySetSrcBuffer(DispPri->phyAddr);
                    else
                        DisplaySetSrcBuffer(m_pActiveSurface->OffsetInVideoMemory());
                }
                else
                {
                    if(!m_pPrimarySurface)
                    {
                        if(DispPri->phyAddr!= NULL)
                            DisplaySetSrcBuffer2(DispPri->phyAddr);
                        else
                            DisplaySetSrcBuffer2(m_pPrimarySurface2->OffsetInVideoMemory());  
                    }
                }
            }
            break;
            
        case DISPLAY_SET_OUTPUT_FREQUENCY:
            if(pvIn && cjIn == sizeof(DWORD))
            {
                int iDisplayFreq = *(int *)pvIn;

                if (iDisplayFreq != m_iDisplayFrequency)
                {
                    // We use ChangeDisplaySettingsEx() to change the display mode, here we keep
                    // the correct display frequency for using in DDIPU::SetMode() in ddipu.cpp
                    m_iDisplayFrequency = iDisplayFreq;
                }

                // WinCE only calls SetMode() when the width or height change.  If just the frequency changes,
                // SetMode() will not be called.  Since we may support different modes that share the
                // same resolution but different frequencies, we need to call SetMode manually
                // when the frequency changes.  If we need a mode change, SetMode will handle it.  If not,
                // SetMode will take no action.
                if (SetMode(m_nCurrentDisplayMode, NULL) == S_FALSE)
                {
                    DEBUGMSG(GPE_ZONE_WARNING, (TEXT("DDIPU DrvEscape: Unable to change display mode with new frequency\r\n")));
                }


                retval = ESC_SUCCESS;
            }
            break;

        case DISPLAY_GET_OUTPUT_FREQUENCY:
            if(pvOut != NULL && cjOut == sizeof(DWORD))
            {
                //Check if pointer initialized...
                //
                if( pvOut != NULL )
                {
                    DWORD *pDispFreq = (DWORD *)pvOut;
                    *pDispFreq = m_iDisplayFrequency;

                    cjOut = sizeof(DWORD);
                    retval = ESC_SUCCESS;
                }
                else
                {
                    SetLastError (ERROR_INVALID_PARAMETER);
                    retval = ESC_FAILED;
                }
            }
            break;

        case DISPLAY_SET_GAMMA_VALUE:
            if(pvIn && cjIn == sizeof(float))
            {
                float fGamma = *(float *)pvIn;

                if (fGamma != m_fGammaValue)
                {
                    if(DisplaySetGamma(fGamma) == FALSE)
                    {
                        retval = ESC_FAILED;    
                    }
                    else
                    {
                        m_fGammaValue = fGamma;
                        retval = ESC_SUCCESS;
                    }
                     
                }
            }

            break;

        case DISPLAY_GET_GAMMA_VALUE:
            if(pvOut != NULL && cjOut == sizeof(float))
            {
                //Check if pointer initialized...
                //
                if( pvOut != NULL )
                {
                    float *pGamma = (float *)pvOut;
                    *pGamma = m_fGammaValue;

                    cjOut = sizeof(float);
                    retval = ESC_SUCCESS;
                }
                else
                {
                    SetLastError (ERROR_INVALID_PARAMETER);
                    retval = ESC_FAILED;
                }
            }
            break;


        // XEC DLS
        case DISPLAY_DLS_GET_CSC:
            if(Display_GetCSC(cjOut, pvOut))
            {
                retval = ESC_SUCCESS;
            }
            else
            {
                retval = ESC_FAILED;
            }
            
            break;

        case DISPLAY_DLS_SET_CSC:
            if(Display_SetCSC(cjIn, pvIn))
            {
                retval = ESC_SUCCESS;
            }
            else
            {
                retval = ESC_FAILED;
            }
            
            break;

        case DISPLAY_IS_VIDEO_INTERLACED:
            if(pvIn && cjIn == sizeof(InterlacedVideoData))
            {
                PInterlacedVideoData pVdiData = (PInterlacedVideoData)pvIn;
                m_bVideoIsInterlaced = pVdiData->bIsInterlaced;
                m_TopField = pVdiData->topField;
                retval = ESC_SUCCESS;
            }
            
            break;

        case VF_GET_DISPLAY_INFO:

            //Check if pointer initialized...
            //
            if( pvOut != NULL )
            {
                PDISPLAY_CHARACTERISTIC pDisChar = (PDISPLAY_CHARACTERISTIC)pvOut;
                if(pvIn == NULL) //compatible with previous setting
                {
                    pDisChar->width  = m_nScreenWidthSave;
                    pDisChar->height = m_nScreenHeightSave;
                    pDisChar->bpp    = m_pMode->Bpp;
                }
                else if(*(PDISPLAY_ID)pvIn == DISPLAY_ID_MAIN_DISPLAY) //primary display device
                {
                    pDisChar->width  = m_nScreenWidthSave;
                    pDisChar->height = m_nScreenHeightSave;
                    pDisChar->bpp    = m_pMode->Bpp;
                }
                else //secondary display device
                {
                    if(m_pPrimarySurface2!= NULL)
                    {
                        pDisChar->width  = m_pPrimarySurface2->Width();
                        pDisChar->height = m_pPrimarySurface2->Height();
                        pDisChar->bpp    = m_pPrimarySurface2->Bpp();       
                    }
                    else
                    {
                        pDisChar->width  = 0;
                        pDisChar->height = 0;
                        pDisChar->bpp    = 0;                              
                    }
                }
                cjOut = sizeof(DISPLAY_CHARACTERISTIC);
                retval = ESC_SUCCESS;
            }
            else
            {
                SetLastError (ERROR_INVALID_PARAMETER);
                retval = ESC_FAILED;
            }
            break;

         case CGMSA_GET_OUTPUT_MODE:
            if(pvOut != NULL && cjOut == sizeof(DWORD))
            {
                //Check if pointer initialized...
                //
                if( pvOut != NULL )
                {
                    DWORD *pCgmsaMode = (DWORD *)pvOut;
                    *pCgmsaMode = m_iCgmsaMode;

                    cjOut = sizeof(DWORD);
                    retval = ESC_SUCCESS;
                }
                else
                {
                    SetLastError (ERROR_INVALID_PARAMETER);
                    retval = ESC_FAILED;
                }
            }
            break;
            
        case CGMSA_SET_OUTPUT_MODE:
            if(pvIn && cjIn == sizeof(DWORD))
            {
                int iCgmsaMode = *(int *)pvIn;

                if (iCgmsaMode != m_iCgmsaMode)
                { 
                    SetCGMSAMode(iCgmsaMode);
                    retval = ESC_SUCCESS;
                }
            }
            break; 
        case DISPLAY_ENABLE_DMI:
            //DMI can't be enabled when overlay is enabled, otherwise it will cause IPU hang.
            if(!m_bIsOverlayWindowRunning)
            {
                if(pvIn && cjIn == sizeof(DISPLAY_DMI))
                {
                    PDISPLAY_DMI pDMI = (PDISPLAY_DMI)pvIn;
                    if(DisplayDMIEnable(pDMI->eGPUID, pDMI->eColorFormat,
                                         pDMI->uiStride, pDMI->eBufferMode,
                                         pDMI->pBuffer1, pDMI->pBuffer2, pDMI->pBuffer3)==TRUE)
                    {
                       retval = ESC_SUCCESS; 
                    }
                }
                else
                {
                    SetLastError (ERROR_INVALID_PARAMETER);
                    retval = ESC_FAILED;
                }
                
            }
            else
            {
                SetLastError (ERROR_BUSY);
                retval = ESC_FAILED;
            }

            break;

        case DISPLAY_DISABLE_DMI:
            //DMI can't be disabled when overlay is enabled.
            if(!m_bIsOverlayWindowRunning)
            {
                if(DisplayDMIDisable()==TRUE)   
                    retval = ESC_SUCCESS;
            }
            else
            {
                SetLastError (ERROR_BUSY);
                retval = ESC_FAILED;
            }
            break;

        case VM_SETATTEX:
            pVMSetAttributeExData pSetData;
            VM_ATTRIBUTE orgAttr;
            pSetData = (pVMSetAttributeExData)pvIn;
            if(VMSetAttributeEx(pSetData,&orgAttr)) 
            {
                if( pvOut != NULL)
                {
                    *(VM_ATTRIBUTE *)pvOut = orgAttr;
                    cjOut = sizeof(VM_ATTRIBUTE);
                }
                retval = ESC_SUCCESS;
            }            
            break;
            
        default :
            retval = DrvEscapeGAPI(iEsc, cjIn, pvIn, cjOut, pvOut);
            break;
    }

    return (ULONG) retval;
}


//------------------------------------------------------------------------------
//
// Function: IsBusy
//
// Method to determine whether IPU ASync is available
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
int DDIPU::IsBusy(VOID)
{
    return FALSE;
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
int DDIPU::InVBlank(VOID)
{
    return (!DisplayIsBusy());
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
VOID DDIPU::WaitForNotBusy()
{
    //DisplayWaitForVSync();

    return;
}

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
VOID DDIPU::WaitForVBlank()
{
    //Only for primary display device
    DisplayWaitForVSync(FALSE);
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
DWORD DDIPU::WaitForVerticalBlank(LPDDHAL_WAITFORVERTICALBLANKDATA pd)
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
            DisplayWaitForVSync(FALSE);
            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;

        case DDWAITVB_BLOCKEND:
            // Returns when the vertical blank interval ends and display begins.
            pd->ddRVal = DDERR_NOVSYNCHW; // TODO:
            return DDHAL_DRIVER_HANDLED;
    }

    return DDHAL_DRIVER_NOTHANDLED;
}


//------------------------------------------------------------------------------
//
// Function: SetCGMSAMode
//
// Update the CGMS-A status (CGMSA_MODE_NTSC, CGMSA_MODE_PAL)
//
// Parameters:
//      iCgmsaMode 
//              [in] a CGMS-A status to set.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DDIPU::SetCGMSAMode(int iCgmsaMode)
{
    m_iCgmsaMode = iCgmsaMode;  
    return;
}



        
