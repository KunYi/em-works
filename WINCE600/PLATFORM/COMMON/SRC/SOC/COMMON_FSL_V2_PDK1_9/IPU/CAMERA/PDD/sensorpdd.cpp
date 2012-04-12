//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
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

#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#pragma warning(disable: 6244)

#include <windows.h>
#include <pm.h>

#include "Cs.h"
#include "Csmedia.h"

#include "common_ipu.h"
#include "common_macros.h"

#pragma warning(pop)
#include <Devload.h>
#include <NKIntr.h>
#include <ceddk.h>
#include "ipu.h"
#include "display_vf.h"
#include "IpuModuleInterfaceClass.h"
#include "IpuBufferManager.h"
#include "PrpClass.h"
#include "CsiClass.h"

#include "CameraPDDProps.h"
#include "dstruct.h"
#include "cameradbg.h"
#include "camera.h"
#include "CameraDriver.h"
#include "SensorFormats.h"
#include "SensorProperties.h"
#include "PinDriver.h"

#include "CameraPDD.h"
#include "SensorPdd.h"
#include "wchar.h"

#pragma warning(disable: 4245)
#pragma warning(disable: 4100)
#pragma warning(disable: 4189)
#pragma warning(disable: 4127)
#pragma warning(disable: 4706)

//-------------------------------------------------------------------------
// External Functions
extern void BSPGetSensorFormat(DWORD *);
extern void BSPGetSensorResolution(DWORD *);
extern int BSPGetDefaultCameraFromRegistry(void);
//For Ringo TVIN+
extern BYTE BSPGetTVinType(void);
extern void BSPDisableCamera(BOOL powerstate);

//-------------------------------------------------------------------------
// External Variables
extern BYTE        gLastTVinType;


PDDFUNCTBL FuncTbl = {
    sizeof(PDDFUNCTBL),
    PDD_Init,
    PDD_DeInit,
    PDD_GetAdapterInfo,
    PDD_HandleVidProcAmpChanges,
    PDD_HandleCamControlChanges,
    PDD_HandleVideoControlCapsChanges,
    PDD_SetPowerState,
    PDD_HandleAdapterCustomProperties,
    PDD_InitSensorMode,
    PDD_DeInitSensorMode,
    PDD_SetSensorState,
    PDD_TakeStillPicture,
    PDD_GetSensorModeInfo,
    PDD_SetSensorModeFormat,
    PDD_AllocateBuffer,
    PDD_DeAllocateBuffer,
    PDD_RegisterClientBuffer,
    PDD_UnRegisterClientBuffer,
    PDD_FillBuffer,
    PDD_HandleModeCustomProperties,
    PDD_PreStillPicture,
    PDD_PostStillPicture
};


CSensorPdd::CSensorPdd()
{
    m_ulCTypes = NUM_SUPPORTED_PINS;
    m_bStillCapInProgress = FALSE;
    m_hContext = NULL;
    m_pModeVideoFormat = NULL;
    m_pModeVideoCaps = NULL;
    m_ppModeContext = NULL;
    m_pCurrentFormat = NULL;
    m_pfnTimeSetEvent = NULL;
    m_pfnTimeKillEvent = NULL;
    m_hTimerDll = NULL;
    m_ulNumOfBuffer = NUM_PIN_BUFFER;    

    m_pCsi = NULL;
    m_pPrp = NULL;
    m_bCameraEncConfig = FALSE;
    m_bCameraStillConfig = FALSE;
    m_bCameraVfConfig = FALSE;
    m_bCameraVfAllocBuf = FALSE;
    m_bCameraEncAllocBuf = FALSE;

    m_bDirectDisplay = FALSE;
    m_bDirectCapture = FALSE;

    m_bFlipVertical = FALSE;
    m_bFlipHorizontal = FALSE;
    m_bRotate = FALSE;

    m_SensorConfigured = FALSE;
    m_inFormat = prpInputFormat_RGB;
    m_inFrameSize.height = 0;
    m_inFrameSize.width = 0;
    
    memset( &m_TimerIdentifier, 0x0, sizeof(m_TimerIdentifier));    
    memset( &m_CsState, 0x0, sizeof(m_CsState));
    memset( &m_SensorModeInfo, 0x0, sizeof(m_SensorModeInfo));
    memset( &m_SensorProps, 0x0, sizeof(m_SensorProps));
    memset( &PowerCaps, 0x0, sizeof(PowerCaps));

    m_hPinEncEvent[0] = CreateEvent ( NULL,FALSE,FALSE,NULL ) ;// for Enc begin
    if(NULL == m_hPinEncEvent[0])
    {
        ERRORMSG(TRUE,
            (TEXT("%s: CreateEvent failed for PinEnc begin\r\n"), __WFUNCTION__));
    }

    m_hPinEncEvent[1] = CreateEvent ( NULL,FALSE,FALSE,NULL ) ;// for Enc stop
    if(NULL == m_hPinEncEvent[1])
    {
        ERRORMSG(TRUE,
            (TEXT("%s: CreateEvent failed for PinEnc stop\r\n"), __WFUNCTION__));
    }

    m_hPinVfEvent[0] = CreateEvent ( NULL,FALSE,FALSE,NULL ) ;// for Vf begin
    if(NULL == m_hPinVfEvent[0])
    {
        ERRORMSG(TRUE,
            (TEXT("%s: CreateEvent failed for PinVf begin\r\n"), __WFUNCTION__));
    }

    m_hPinVfEvent[1] = CreateEvent ( NULL,FALSE,FALSE,NULL ) ;// for Vf stop
    if(NULL == m_hPinVfEvent[1])
    {
        ERRORMSG(TRUE,
            (TEXT("%s: CreateEvent failed for PinVf stop\r\n"), __WFUNCTION__));
    }

    m_hPinEncThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PinEncWorkerThread, this, 0, NULL);
    if (m_hPinEncThread == NULL)
    {
        ERRORMSG(TRUE,(TEXT("%s: CreateThread failed for Pin Enc!\r\n"), __WFUNCTION__));
    }
    else
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: create Enc ISR thread success\r\n"), __WFUNCTION__));
    }

    m_hPinVfThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PinVfWorkerThread, this, 0, NULL);
    if (m_hPinVfThread == NULL)
    {
        ERRORMSG(TRUE,(TEXT("%s: CreateThread failed for Pin Vf!\r\n"), __WFUNCTION__));
    }
    else
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: create Vf ISR thread success\r\n"), __WFUNCTION__));
    }
}

CSensorPdd::~CSensorPdd()
{
    RETAILMSG (0,
            (TEXT("%s: Calling ~CSensorPdd();\r\n"), __WFUNCTION__));
    if( m_pfnTimeKillEvent )
    {
        for( int i=0; i < NUM_SUPPORTED_PINS; i++ )
        {
            if ( NULL != m_TimerIdentifier[i] )
            {
                m_pfnTimeKillEvent( m_TimerIdentifier[i] );
                m_TimerIdentifier[i] = NULL;
            }
        }
    }

    if ( NULL != m_hTimerDll )
    {
        FreeLibrary( m_hTimerDll );
        m_hTimerDll = NULL;
    }

    if (m_hPinEncThread)
    {
        SetEvent ( m_hPinEncEvent[1] ); 
        // Wait for PrpIntrThread end
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hPinEncThread,INFINITE))
            DEBUGMSG(ZONE_FUNCTION,(TEXT("Wait for PinEncWorkerThread end")));

        CloseHandle(m_hPinEncThread);
        m_hPinEncThread = NULL;    
    }

    if (m_hPinVfThread)
    {
        SetEvent ( m_hPinVfEvent[1] ); 
        // Wait for PrpIntrThread end
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hPinVfThread,INFINITE))
            DEBUGMSG(ZONE_FUNCTION,(TEXT("Wait for PinVfWorkerThread end")));

        CloseHandle(m_hPinVfThread);
        m_hPinVfThread = NULL;        
    }

    if (m_hPinEncEvent[0] != NULL)
    {
        CloseHandle(m_hPinEncEvent[0]);
        m_hPinEncEvent[0] = NULL;
    }
    if (m_hPinEncEvent[1] != NULL)
    {
        CloseHandle(m_hPinEncEvent[1]);
        m_hPinEncEvent[1] = NULL;
    }

    if (m_hPinVfEvent[0] != NULL)
    {
        CloseHandle(m_hPinVfEvent[0]);
        m_hPinVfEvent[0] = NULL;
    }
    if (m_hPinVfEvent[1] != NULL)
    {
        CloseHandle(m_hPinVfEvent[1]);
        m_hPinVfEvent[1] = NULL;
    }

    if( NULL != m_pModeVideoCaps )
    {
        delete [] m_pModeVideoCaps;
        m_pModeVideoCaps = NULL;
    }

    if( NULL != m_pCurrentFormat )
    {
        delete [] m_pCurrentFormat;
        m_pCurrentFormat = NULL;
    }

    if( NULL != m_pModeVideoFormat )
    {
        delete [] m_pModeVideoFormat;
        m_pModeVideoFormat = NULL;
    }

    if( NULL != m_ppModeContext )
    {
        delete [] m_ppModeContext;
        m_ppModeContext = NULL;
    }

    // even if Camera is closed.  When we create a preprocessor driver, 
    // there must be a mechanism for determining between these two 
    // drivers whether the IC is still in use.
    if( NULL != m_pPrp )
    {
        delete m_pPrp;
        m_pPrp = NULL;
    }

    if( NULL != m_pCsi )
    {
        delete m_pCsi;
        m_pCsi = NULL;
    }

}

//------------------------------------------------------------------------------
//
// Function: PinEncWorkerThread
//
// This function is the Enc worker thread.
//
// Parameters:
//      lpParameter
//          [in] PDD  handler
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void CSensorPdd::PinEncWorkerThread(LPVOID lpParameter)
{
    CSensorPdd *pDD = (CSensorPdd *)lpParameter;

    pDD->PinEncWorkerLoop(INFINITE);
}

//------------------------------------------------------------------------------
//
// Function: PinEncWorkerLoop
//
// This function is the EOF frame handler for the PRP Enc.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF event.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void CSensorPdd::PinEncWorkerLoop(UINT32 timeout)
{
    DWORD result;
    //DWORD dwRet;

    // loop here
    while( (result = WaitForMultipleObjects (2, m_hPinEncEvent, FALSE, INFINITE )) != WAIT_TIMEOUT )
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::PinEncWorkerLoop get event 0x%x\r\n"),result));

        if(result == WAIT_OBJECT_0 + 0)
        {
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CAPTURE pin start work\r\n")));
            while ( m_CsState[CAPTURE] == CSSTATE_RUN )
            {
                #if 0
                dwRet = WaitForSingleObject(m_pPrp->m_hEncEOFEvent, 100);
                if(dwRet != WAIT_TIMEOUT)
                {
                    if( m_CsState[CAPTURE] == CSSTATE_RUN )
                    {
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::PinEncWorkerLoop call handlePINIO\r\n")));
                        MDD_HandleIO( m_ppModeContext[CAPTURE], CAPTURE );
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    if( m_CsState[CAPTURE] != CSSTATE_RUN )
                    {
                        break;
                    }
                }
                #endif

                Sleep(33);

                if( m_CsState[CAPTURE] == CSSTATE_RUN )
                {
                    MDD_HandleIO( m_ppModeContext[CAPTURE], CAPTURE );
                }

            }
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CAPTURE pin stop work\r\n")));
        }

        if(result == WAIT_OBJECT_0 + 1)
        {
            break;
        }
    }

    return;
}

//------------------------------------------------------------------------------
//
// Function: PinVfWorkerThread
//
// This function is the Vf worker thread.
//
// Parameters:
//      lpParameter
//          [in] PDD  handler
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void CSensorPdd::PinVfWorkerThread(LPVOID lpParameter)
{
    CSensorPdd *pDD = (CSensorPdd *)lpParameter;

    pDD->PinVfWorkerLoop(INFINITE);
}

//------------------------------------------------------------------------------
//
// Function: PinVfWorkerLoop
//
// This function is the EOF frame handler for the PRP Vf.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF event.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void CSensorPdd::PinVfWorkerLoop(UINT32 timeout)
{
    DWORD result;
    //DWORD dwRet;

    // loop here
    while( (result = WaitForMultipleObjects (2, m_hPinVfEvent, FALSE, INFINITE )) != WAIT_TIMEOUT)
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::PinVfWorkerLoop get event 0x%x\r\n"),result));

        if(result == WAIT_OBJECT_0 + 0)
        {
            DEBUGMSG(ZONE_FUNCTION,(TEXT("PREVIEW pin start work\r\n")));

            while ( m_CsState[PREVIEW] == CSSTATE_RUN )
            {
                #if 0
                dwRet = WaitForSingleObject(m_pPrp->m_hVfEOFEvent, 100);
                if(dwRet != WAIT_TIMEOUT)
                {
                    if(m_CsState[PREVIEW] == CSSTATE_RUN)
                    {
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("PREVIEW pin call MDD_HandleIO\r\n")));
                        MDD_HandleIO( m_ppModeContext[PREVIEW], PREVIEW );
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    if(m_CsState[PREVIEW] != CSSTATE_RUN)
                    {
                        break;
                    }
                }
                #endif

                Sleep(33);

                if( m_CsState[PREVIEW] == CSSTATE_RUN )
                {
                    MDD_HandleIO( m_ppModeContext[PREVIEW], PREVIEW );
                }

            }

            DEBUGMSG(ZONE_FUNCTION,(TEXT("PREVIEW pin stop work\r\n")));
        }

        if(result == WAIT_OBJECT_0 + 1)
        {
            DEBUGMSG(ZONE_FUNCTION,(TEXT("PREVIEW thread killed\r\n")));
            break;
        }
    }

    return;
}

DWORD CSensorPdd::PDDInit( PVOID MDDContext, PPDDFUNCTBL pPDDFuncTbl )
{
    //init IPU class
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    // Create (and initialize) preprocessor class object
    if(m_pPrp == NULL)
    {
        m_pPrp = new PrpClass;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed creating PrpClass object!\r\n"), __WFUNCTION__));
    }

    // Create (and initialize) CSI class object
    if(m_pCsi == NULL)
    {
        m_pCsi = new CsiClass;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed creating CsiClass object!\r\n"), __WFUNCTION__));
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    
    m_hContext = (HANDLE)MDDContext;
    // Real drivers may want to create their context

    m_ulCTypes = NUM_SUPPORTED_PINS; 
    
    // Read registry to override the default number of Sensor Modes.
    ReadMemoryModelFromRegistry();  

    if( pPDDFuncTbl->dwSize  > sizeof( PDDFUNCTBL ) )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    memcpy( pPDDFuncTbl, &FuncTbl, sizeof( PDDFUNCTBL ) );

    memset( m_SensorProps, 0x0, sizeof(m_SensorProps) );

    PowerCaps.DeviceDx = 0x11;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Set all VideoProcAmp and CameraControl properties.
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //VideoProcAmp
    m_SensorProps[ENUM_BRIGHTNESS].ulCurrentValue     = BrightnessDefault;
    m_SensorProps[ENUM_BRIGHTNESS].ulDefaultValue     = BrightnessDefault;
    m_SensorProps[ENUM_BRIGHTNESS].pRangeNStep        = &BrightnessRangeAndStep[0];
    m_SensorProps[ENUM_BRIGHTNESS].ulFlags            = CSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    m_SensorProps[ENUM_BRIGHTNESS].ulCapabilities     = CSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL|CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_BRIGHTNESS].fSetSupported      = VideoProcAmpProperties[ENUM_BRIGHTNESS].SetSupported;
    m_SensorProps[ENUM_BRIGHTNESS].fGetSupported      = VideoProcAmpProperties[ENUM_BRIGHTNESS].GetSupported;
    m_SensorProps[ENUM_BRIGHTNESS].pCsPropValues      = &BrightnessValues;

    m_SensorProps[ENUM_CONTRAST].ulCurrentValue       = ContrastDefault;
    m_SensorProps[ENUM_CONTRAST].ulDefaultValue       = ContrastDefault;
    m_SensorProps[ENUM_CONTRAST].pRangeNStep          = &ContrastRangeAndStep[0];
    m_SensorProps[ENUM_CONTRAST].ulFlags              = CSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    m_SensorProps[ENUM_CONTRAST].ulCapabilities       = CSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL|CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_CONTRAST].fSetSupported        = VideoProcAmpProperties[ENUM_CONTRAST].SetSupported;
    m_SensorProps[ENUM_CONTRAST].fGetSupported        = VideoProcAmpProperties[ENUM_CONTRAST].GetSupported;
    m_SensorProps[ENUM_CONTRAST].pCsPropValues        = &ContrastValues;

    m_SensorProps[ENUM_HUE].ulCurrentValue            = HueDefault;
    m_SensorProps[ENUM_HUE].ulDefaultValue            = HueDefault;
    m_SensorProps[ENUM_HUE].pRangeNStep               = &HueRangeAndStep[0];
    m_SensorProps[ENUM_HUE].ulFlags                   = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_HUE].ulCapabilities            = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_HUE].fSetSupported             = VideoProcAmpProperties[ENUM_HUE].SetSupported;
    m_SensorProps[ENUM_HUE].fGetSupported             = VideoProcAmpProperties[ENUM_HUE].GetSupported;
    m_SensorProps[ENUM_HUE].pCsPropValues             = &HueValues;

    m_SensorProps[ENUM_SATURATION].ulCurrentValue     = SaturationDefault;
    m_SensorProps[ENUM_SATURATION].ulDefaultValue     = SaturationDefault;
    m_SensorProps[ENUM_SATURATION].pRangeNStep        = &SaturationRangeAndStep[0];
    m_SensorProps[ENUM_SATURATION].ulFlags            = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_SATURATION].ulCapabilities     = CSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL|CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_SATURATION].fSetSupported      = VideoProcAmpProperties[ENUM_SATURATION].SetSupported;
    m_SensorProps[ENUM_SATURATION].fGetSupported      = VideoProcAmpProperties[ENUM_SATURATION].GetSupported;
    m_SensorProps[ENUM_SATURATION].pCsPropValues      = &SaturationValues;

    m_SensorProps[ENUM_SHARPNESS].ulCurrentValue      = SharpnessDefault;
    m_SensorProps[ENUM_SHARPNESS].ulDefaultValue      = SharpnessDefault;
    m_SensorProps[ENUM_SHARPNESS].pRangeNStep         = &SharpnessRangeAndStep[0];
    m_SensorProps[ENUM_SHARPNESS].ulFlags             = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_SHARPNESS].ulCapabilities      = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_SHARPNESS].fSetSupported       = VideoProcAmpProperties[ENUM_SHARPNESS].SetSupported;
    m_SensorProps[ENUM_SHARPNESS].fGetSupported       = VideoProcAmpProperties[ENUM_SHARPNESS].GetSupported;
    m_SensorProps[ENUM_SHARPNESS].pCsPropValues       = &SharpnessValues;

    m_SensorProps[ENUM_GAMMA].ulCurrentValue          = GammaDefault;
    m_SensorProps[ENUM_GAMMA].ulDefaultValue          = GammaDefault;
    m_SensorProps[ENUM_GAMMA].pRangeNStep             = &GammaRangeAndStep[0];
    m_SensorProps[ENUM_GAMMA].ulFlags                 = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_GAMMA].ulCapabilities          = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_GAMMA].fSetSupported           = VideoProcAmpProperties[ENUM_GAMMA].SetSupported;
    m_SensorProps[ENUM_GAMMA].fGetSupported           = VideoProcAmpProperties[ENUM_GAMMA].GetSupported;
    m_SensorProps[ENUM_GAMMA].pCsPropValues           = &GammaValues;

    m_SensorProps[ENUM_COLORENABLE].ulCurrentValue    = ColorEnableDefault;
    m_SensorProps[ENUM_COLORENABLE].ulDefaultValue    = ColorEnableDefault;
    m_SensorProps[ENUM_COLORENABLE].pRangeNStep       = &ColorEnableRangeAndStep[0];
    m_SensorProps[ENUM_COLORENABLE].ulFlags           = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_COLORENABLE].ulCapabilities    = CSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL|CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_COLORENABLE].fSetSupported     = VideoProcAmpProperties[ENUM_COLORENABLE].SetSupported;
    m_SensorProps[ENUM_COLORENABLE].fGetSupported     = VideoProcAmpProperties[ENUM_COLORENABLE].GetSupported;
    m_SensorProps[ENUM_COLORENABLE].pCsPropValues     = &ColorEnableValues;

    m_SensorProps[ENUM_WHITEBALANCE].ulCurrentValue   = WhiteBalanceDefault;
    m_SensorProps[ENUM_WHITEBALANCE].ulDefaultValue   = WhiteBalanceDefault;
    m_SensorProps[ENUM_WHITEBALANCE].pRangeNStep      = &WhiteBalanceRangeAndStep[0];
    m_SensorProps[ENUM_WHITEBALANCE].ulFlags          = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_WHITEBALANCE].ulCapabilities   = CSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL|CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_WHITEBALANCE].fSetSupported    = VideoProcAmpProperties[ENUM_WHITEBALANCE].SetSupported;
    m_SensorProps[ENUM_WHITEBALANCE].fGetSupported    = VideoProcAmpProperties[ENUM_WHITEBALANCE].GetSupported;
    m_SensorProps[ENUM_WHITEBALANCE].pCsPropValues    = &WhiteBalanceValues;

    m_SensorProps[ENUM_BACKLIGHT_COMPENSATION].ulCurrentValue = BackLightCompensationDefault;
    m_SensorProps[ENUM_BACKLIGHT_COMPENSATION].ulDefaultValue = BackLightCompensationDefault;
    m_SensorProps[ENUM_BACKLIGHT_COMPENSATION].pRangeNStep    = &BackLightCompensationRangeAndStep[0];
    m_SensorProps[ENUM_BACKLIGHT_COMPENSATION].ulFlags        = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_BACKLIGHT_COMPENSATION].ulCapabilities = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_BACKLIGHT_COMPENSATION].fSetSupported  = VideoProcAmpProperties[ENUM_BACKLIGHT_COMPENSATION].SetSupported;
    m_SensorProps[ENUM_BACKLIGHT_COMPENSATION].fGetSupported  = VideoProcAmpProperties[ENUM_BACKLIGHT_COMPENSATION].GetSupported;
    m_SensorProps[ENUM_BACKLIGHT_COMPENSATION].pCsPropValues  = &BackLightCompensationValues;

    m_SensorProps[ENUM_GAIN].ulCurrentValue           = GainDefault;
    m_SensorProps[ENUM_GAIN].ulDefaultValue           = GainDefault;
    m_SensorProps[ENUM_GAIN].pRangeNStep              = &GainRangeAndStep[0];
    m_SensorProps[ENUM_GAIN].ulFlags                  = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_GAIN].ulCapabilities           = CSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    m_SensorProps[ENUM_GAIN].fSetSupported            = VideoProcAmpProperties[ENUM_GAIN].SetSupported;
    m_SensorProps[ENUM_GAIN].fGetSupported            = VideoProcAmpProperties[ENUM_GAIN].GetSupported;
    m_SensorProps[ENUM_GAIN].pCsPropValues            = &GainValues;

    //CameraControl
    m_SensorProps[ENUM_PAN].ulCurrentValue            = PanDefault;
    m_SensorProps[ENUM_PAN].ulDefaultValue            = PanDefault;
    m_SensorProps[ENUM_PAN].pRangeNStep               = &PanRangeAndStep[0];
    m_SensorProps[ENUM_PAN].ulFlags                   = CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_PAN].ulCapabilities            = CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL|CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_PAN].fSetSupported             = VideoProcAmpProperties[ENUM_PAN-NUM_VIDEOPROCAMP_ITEMS].SetSupported;
    m_SensorProps[ENUM_PAN].fGetSupported             = VideoProcAmpProperties[ENUM_PAN-NUM_VIDEOPROCAMP_ITEMS].GetSupported;
    m_SensorProps[ENUM_PAN].pCsPropValues             = &PanValues;

    m_SensorProps[ENUM_TILT].ulCurrentValue           = TiltDefault;
    m_SensorProps[ENUM_TILT].ulDefaultValue           = TiltDefault;
    m_SensorProps[ENUM_TILT].pRangeNStep              = &TiltRangeAndStep[0];
    m_SensorProps[ENUM_TILT].ulFlags                  = CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_TILT].ulCapabilities           = CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL|CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_TILT].fSetSupported            = VideoProcAmpProperties[ENUM_TILT-NUM_VIDEOPROCAMP_ITEMS].SetSupported;
    m_SensorProps[ENUM_TILT].fGetSupported            = VideoProcAmpProperties[ENUM_TILT-NUM_VIDEOPROCAMP_ITEMS].GetSupported;
    m_SensorProps[ENUM_TILT].pCsPropValues            = &TiltValues;

    m_SensorProps[ENUM_ROLL].ulCurrentValue           = RollDefault;
    m_SensorProps[ENUM_ROLL].ulDefaultValue           = RollDefault;
    m_SensorProps[ENUM_ROLL].pRangeNStep              = &RollRangeAndStep[0];
    m_SensorProps[ENUM_ROLL].ulFlags                  = CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_ROLL].ulCapabilities           = CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL|CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_ROLL].fSetSupported            = VideoProcAmpProperties[ENUM_ROLL-NUM_VIDEOPROCAMP_ITEMS].SetSupported;
    m_SensorProps[ENUM_ROLL].fGetSupported            = VideoProcAmpProperties[ENUM_ROLL-NUM_VIDEOPROCAMP_ITEMS].GetSupported;
    m_SensorProps[ENUM_ROLL].pCsPropValues            = &RollValues;

    m_SensorProps[ENUM_ZOOM].ulCurrentValue           = ZoomDefault;
    m_SensorProps[ENUM_ZOOM].ulDefaultValue           = ZoomDefault;
    m_SensorProps[ENUM_ZOOM].pRangeNStep              = &ZoomRangeAndStep[0];
    m_SensorProps[ENUM_ZOOM].ulFlags                  = CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_ZOOM].ulCapabilities           = CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL|CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_ZOOM].fSetSupported            = VideoProcAmpProperties[ENUM_ZOOM-NUM_VIDEOPROCAMP_ITEMS].SetSupported;
    m_SensorProps[ENUM_ZOOM].fGetSupported            = VideoProcAmpProperties[ENUM_ZOOM-NUM_VIDEOPROCAMP_ITEMS].GetSupported;
    m_SensorProps[ENUM_ZOOM].pCsPropValues            = &ZoomValues;

    m_SensorProps[ENUM_IRIS].ulCurrentValue           = IrisDefault;
    m_SensorProps[ENUM_IRIS].ulDefaultValue           = IrisDefault;
    m_SensorProps[ENUM_IRIS].pRangeNStep              = &IrisRangeAndStep[0];
    m_SensorProps[ENUM_IRIS].ulFlags                  = CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_IRIS].ulCapabilities           = CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL|CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_IRIS].fSetSupported            = VideoProcAmpProperties[ENUM_IRIS-NUM_VIDEOPROCAMP_ITEMS].SetSupported;
    m_SensorProps[ENUM_IRIS].fGetSupported            = VideoProcAmpProperties[ENUM_IRIS-NUM_VIDEOPROCAMP_ITEMS].GetSupported;
    m_SensorProps[ENUM_IRIS].pCsPropValues            = &IrisValues;

    m_SensorProps[ENUM_EXPOSURE].ulCurrentValue       = ExposureDefault;
    m_SensorProps[ENUM_EXPOSURE].ulDefaultValue       = ExposureDefault;
    m_SensorProps[ENUM_EXPOSURE].pRangeNStep          = &ExposureRangeAndStep[0];
    m_SensorProps[ENUM_EXPOSURE].ulFlags              = CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_EXPOSURE].ulCapabilities       = CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL|CSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    m_SensorProps[ENUM_EXPOSURE].fSetSupported        = VideoProcAmpProperties[ENUM_EXPOSURE-NUM_VIDEOPROCAMP_ITEMS].SetSupported;
    m_SensorProps[ENUM_EXPOSURE].fGetSupported        = VideoProcAmpProperties[ENUM_EXPOSURE-NUM_VIDEOPROCAMP_ITEMS].GetSupported;
    m_SensorProps[ENUM_EXPOSURE].pCsPropValues        = &ExposureValues;

    m_SensorProps[ENUM_FOCUS].ulCurrentValue          = FocusDefault;
    m_SensorProps[ENUM_FOCUS].ulDefaultValue          = FocusDefault;
    m_SensorProps[ENUM_FOCUS].pRangeNStep             = &FocusRangeAndStep[0];
    m_SensorProps[ENUM_FOCUS].ulFlags                 = CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
    m_SensorProps[ENUM_FOCUS].ulCapabilities          = CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
    m_SensorProps[ENUM_FOCUS].fSetSupported           = VideoProcAmpProperties[ENUM_FOCUS-NUM_VIDEOPROCAMP_ITEMS].SetSupported;
    m_SensorProps[ENUM_FOCUS].fGetSupported           = VideoProcAmpProperties[ENUM_FOCUS-NUM_VIDEOPROCAMP_ITEMS].GetSupported;
    m_SensorProps[ENUM_FOCUS].pCsPropValues           = &FocusValues;

    m_SensorProps[ENUM_FLASH].ulCurrentValue          = FlashDefault;
    m_SensorProps[ENUM_FLASH].ulDefaultValue          = FlashDefault;
    m_SensorProps[ENUM_FLASH].pRangeNStep             = &FlashRangeAndStep[0];
    m_SensorProps[ENUM_FLASH].ulFlags                 = CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
    m_SensorProps[ENUM_FLASH].ulCapabilities          = CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
    m_SensorProps[ENUM_FLASH].fSetSupported           = VideoProcAmpProperties[ENUM_FLASH-NUM_VIDEOPROCAMP_ITEMS].SetSupported;
    m_SensorProps[ENUM_FLASH].fGetSupported           = VideoProcAmpProperties[ENUM_FLASH-NUM_VIDEOPROCAMP_ITEMS].GetSupported;
    m_SensorProps[ENUM_FLASH].pCsPropValues           = &FlashValues;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    m_pModeVideoFormat = NULL;
    // Allocate Video Format specific array.
    m_pModeVideoFormat = new PINVIDEOFORMAT[m_ulCTypes];
    if( NULL == m_pModeVideoFormat )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    // Video Format initialization
    m_pModeVideoFormat[CAPTURE].categoryGUID         = PINNAME_VIDEO_CAPTURE;
    switch (m_pCsi->m_iCamType)
    {
        case 4://csiTVinId_ADV7180:
            m_pModeVideoFormat[CAPTURE].ulAvailFormats       = 1;
            break;
        case 3://csiSensorId_OV2640:
        default:
            m_pModeVideoFormat[CAPTURE].ulAvailFormats       = 10;
            break;
    }
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[CAPTURE].ulAvailFormats];

    if( NULL == m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    switch (m_pCsi->m_iCamType)
    {
        case 4://csiTVinId_ADV7180:
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[0] = &DCAM_StreamMode_RGB565_QQVGA;
            break;
        case 3://csiSensorId_OV2640:
        default:
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[0] = &DCAM_StreamMode_RGB565_QQVGA;
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[1] = &DCAM_StreamMode_RGB565_QCIF;
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[2] = &DCAM_StreamMode_RGB565_QVGA;
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[3] = &DCAM_StreamMode_RGB565_CIF;
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[4] = &DCAM_StreamMode_RGB565_VGA;
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[5] = &DCAM_StreamMode_YV12_QQVGA;
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[6] = &DCAM_StreamMode_YV12_QCIF;
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[7] = &DCAM_StreamMode_YV12_QVGA;
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[8] = &DCAM_StreamMode_YV12_CIF;
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[9] = &DCAM_StreamMode_YV12_VGA;
            break;
    }
    
    m_pModeVideoFormat[STILL].categoryGUID           = PINNAME_VIDEO_STILL;
    // For Ringo TVIN+
    switch (m_pCsi->m_iCamType)
    {
        case 4://csiTVinId_ADV7180:
            m_pModeVideoFormat[STILL].ulAvailFormats       = 1;
            break;
        case 3://csiSensorId_OV2640:
        default:
            m_pModeVideoFormat[STILL].ulAvailFormats       = 6;
            break;
    }
    DEBUGMSG(ZONE_FUNCTION,(TEXT("STILL: AvailFormats is %x"),m_pModeVideoFormat[STILL].ulAvailFormats));
    m_pModeVideoFormat[STILL].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[STILL].ulAvailFormats];

    if( NULL == m_pModeVideoFormat[STILL].pCsDataRangeVideo )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    switch (m_pCsi->m_iCamType)
    {
        case 4://csiTVinId_ADV7180:
            m_pModeVideoFormat[STILL].pCsDataRangeVideo[0]   = &DCAM_StreamMode_RGB565_QQVGA;
            break;
        case 3://csiSensorId_OV2640:
        default:
            m_pModeVideoFormat[STILL].pCsDataRangeVideo[0]   = &DCAM_StreamMode_RGB565_QQVGA;
            m_pModeVideoFormat[STILL].pCsDataRangeVideo[1]   = &DCAM_StreamMode_RGB565_QCIF;
            m_pModeVideoFormat[STILL].pCsDataRangeVideo[2]   = &DCAM_StreamMode_RGB565_QVGA;
            m_pModeVideoFormat[STILL].pCsDataRangeVideo[3]   = &DCAM_StreamMode_RGB565_CIF;
            m_pModeVideoFormat[STILL].pCsDataRangeVideo[4]   = &DCAM_StreamMode_RGB565_VGA;
            m_pModeVideoFormat[STILL].pCsDataRangeVideo[5]   = &DCAM_StreamMode_RGB565_SVGA;
            break;
    }

    if( MAX_SUPPORTED_PINS == m_ulCTypes )
    {
        m_pModeVideoFormat[PREVIEW].categoryGUID         = PINNAME_VIDEO_PREVIEW;
        switch (m_pCsi->m_iCamType)
        {
            case 4://csiTVinId_ADV7180:
                m_pModeVideoFormat[PREVIEW].ulAvailFormats = 8;
                break;
            case 3://csiSensorId_OV2640:
            default:
                m_pModeVideoFormat[PREVIEW].ulAvailFormats = 6;
                break;
        }
        RETAILMSG(0,(TEXT("PREVIEW: AvailFormats is %x"),m_pModeVideoFormat[PREVIEW].ulAvailFormats));
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[PREVIEW].ulAvailFormats];

        if( NULL == m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo )
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }
 
        switch (m_pCsi->m_iCamType)
        {
            case 4://csiTVinId_ADV7180:
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[0] = &DCAM_StreamMode_RGB565_QQVGA;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[1] = &DCAM_StreamMode_RGB565_QCIF;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[2] = &DCAM_StreamMode_RGB565_QVGA;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[3] = &DCAM_StreamMode_RGB565_CIF;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[4] = &DCAM_StreamMode_RGB565_VGA;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[5] = &DCAM_StreamMode_RGB565_NTSC;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[6] = &DCAM_StreamMode_RGB565_WVGA;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[7] = &DCAM_StreamMode_RGB565_PAL;
                break;
            case 3://csiSensorId_OV2640:
            default:
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[0] = &DCAM_StreamMode_RGB565_QQVGA;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[1] = &DCAM_StreamMode_RGB565_QCIF;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[2] = &DCAM_StreamMode_RGB565_QVGA;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[3] = &DCAM_StreamMode_RGB565_CIF;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[4] = &DCAM_StreamMode_RGB565_VGA;
                m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[5] = &DCAM_StreamMode_RGB565_SVGA;
                break;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    m_pModeVideoCaps = NULL;
    // Allocate Video Control Caps specific array.
    m_pModeVideoCaps = new VIDCONTROLCAPS[m_ulCTypes];
    if( NULL == m_pModeVideoCaps )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    // Video Control Caps

    m_pModeVideoCaps[CAPTURE].DefaultVideoControlCaps     = DefaultVideoControlCaps[CAPTURE];
    m_pModeVideoCaps[CAPTURE].CurrentVideoControlCaps     = DefaultVideoControlCaps[CAPTURE];;
    m_pModeVideoCaps[STILL].DefaultVideoControlCaps       = DefaultVideoControlCaps[STILL];
    m_pModeVideoCaps[STILL].CurrentVideoControlCaps       = DefaultVideoControlCaps[STILL];
    if( MAX_SUPPORTED_PINS == m_ulCTypes )
    {
        // Note PREVIEW control caps are the same, so we don't differentiate
        m_pModeVideoCaps[PREVIEW].DefaultVideoControlCaps     = DefaultVideoControlCaps[PREVIEW];
        m_pModeVideoCaps[PREVIEW].CurrentVideoControlCaps     = DefaultVideoControlCaps[PREVIEW];;
    }

    // Timer specific variables. Only to be used in this NULL PDD
    m_hTimerDll                 = NULL;
    m_pfnTimeSetEvent           = NULL;
    m_pfnTimeKillEvent          = NULL;
    memset( &m_TimerIdentifier, 0, NUM_SUPPORTED_PINS * sizeof(MMRESULT));

    m_SensorModeInfo[CAPTURE].MemoryModel = CSPROPERTY_BUFFER_DRIVER;
    m_SensorModeInfo[CAPTURE].MaxNumOfBuffers = 1;
    m_SensorModeInfo[CAPTURE].PossibleCount = 1;
    
    m_SensorModeInfo[STILL].MemoryModel = CSPROPERTY_BUFFER_DRIVER;
    m_SensorModeInfo[STILL].MaxNumOfBuffers = 1;
    m_SensorModeInfo[STILL].PossibleCount = 1;
    if( MAX_SUPPORTED_PINS == m_ulCTypes )
    {
        m_SensorModeInfo[PREVIEW].MemoryModel =    CSPROPERTY_BUFFER_DRIVER;
        m_SensorModeInfo[PREVIEW].MaxNumOfBuffers = 1;
        m_SensorModeInfo[PREVIEW].PossibleCount = 1;
    }

    m_ppModeContext = new LPVOID[m_ulCTypes];
    if ( NULL == m_ppModeContext )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    m_ppModeContext[CAPTURE] = NULL;    
    m_ppModeContext[STILL] = NULL;
    if( MAX_SUPPORTED_PINS == m_ulCTypes )
    {
         m_ppModeContext[PREVIEW] = NULL;    
    }

    m_pCurrentFormat = new CS_DATARANGE_VIDEO[m_ulCTypes];
    if( NULL == m_pCurrentFormat )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    return ERROR_SUCCESS;
}


DWORD CSensorPdd::GetAdapterInfo( PADAPTERINFO pAdapterInfo )
{
    pAdapterInfo->ulCTypes = m_ulCTypes;
    pAdapterInfo->PowerCaps = PowerCaps;
    pAdapterInfo->ulVersionID = DRIVER_VERSION_2; //Camera MDD and DShow support DRIVER_VERSION and DRIVER_VERSION_2. Defined in camera.h
    memcpy( &pAdapterInfo->SensorProps, &m_SensorProps, sizeof(m_SensorProps));

    return ERROR_SUCCESS;

}

DWORD CSensorPdd::HandleVidProcAmpChanges( DWORD dwPropId, LONG lFlags, LONG lValue )
{
    PSENSOR_PROPERTY pDevProp = NULL;

    pDevProp = m_SensorProps + dwPropId;
    
    if( CSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL == lFlags )
    {
        pDevProp->ulCurrentValue = lValue;
    }

    pDevProp->ulFlags = lFlags;
    return ERROR_SUCCESS;
}

DWORD CSensorPdd::HandleCamControlChanges( DWORD dwPropId, LONG lFlags, LONG lValue )
{
    PSENSOR_PROPERTY pDevProp = NULL;
    
    pDevProp = m_SensorProps + dwPropId;
    
    if( CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL == lFlags )
    {
        pDevProp->ulCurrentValue = lValue;
    }
    pDevProp->ulFlags = lFlags;

    if (dwPropId == (CSPROPERTY_CAMERACONTROL_ZOOM + NUM_VIDEOPROCAMP_ITEMS))
    {
        SensorZoom(pDevProp->ulCurrentValue);
    }
    else if (dwPropId == (CSPROPERTY_CAMERACONTROL_TILT + NUM_VIDEOPROCAMP_ITEMS))
    {
        if (pDevProp->ulCurrentValue == 90)
        {
            if (!m_bRotate)
            {
                // Rotation turned from OFF to ON state
                m_bRotate = TRUE;
                DEBUGMSG(ZONE_DEVICE, (_T("CAM_IOControl(%08x): Just set up for 90 degree Rotation.\r\n"), this));
                // Assure that we will reconfigure IPU before starting.
                m_bCameraEncConfig = FALSE;
                m_bCameraVfConfig = FALSE;
            }
        }
        else if (pDevProp->ulCurrentValue == 0)
        {
            if (m_bRotate)
            {
                m_bRotate = FALSE;
                // Assure that we will reconfigure IPU before starting.
                m_bCameraEncConfig = FALSE;
                m_bCameraVfConfig = FALSE;
            }
        }
        else
        {
            // Invalid rotation value
            DEBUGMSG(ZONE_ERROR, (_T("CAM_IOControl(%08x): Invalid value for tilt.\r\n"), this));
        }
    }
    DEBUGMSG(ZONE_FUNCTION, (_T("CAM_IOControl(%08x): HandleCamControlRequests successfully. ulCurrentValue = %d\r\n"), this, pDevProp->ulCurrentValue)) ;

    return ERROR_SUCCESS;
}

DWORD CSensorPdd::HandleVideoControlCapsChanges( LONG lModeType ,ULONG ulCaps )
{
    m_pModeVideoCaps[lModeType].CurrentVideoControlCaps = ulCaps;

    if (ulCaps & CS_VideoControlFlag_FlipHorizontal)
    {
        // Toggle Flip state
        m_bFlipHorizontal = m_bFlipHorizontal ? FALSE : TRUE;
        DEBUGMSG(ZONE_DEVICE, (_T("CAM_IOControl(%08x): Just toggled Horiz Flip.\r\n"), this));
        // Assure that we will reconfigure IPU before starting.
        m_bCameraEncConfig = FALSE;
        m_bCameraVfConfig = FALSE;
    }

    if (ulCaps & CS_VideoControlFlag_FlipVertical)
    {
        // Flip turned from OFF to ON state
        m_bFlipVertical = m_bFlipVertical ? FALSE : TRUE;
        DEBUGMSG(ZONE_DEVICE, (_T("CAM_IOControl(%08x): Just toggled Vert Flip.\r\n"), this));
        // Assure that we will reconfigure IPU before starting.
        m_bCameraEncConfig = FALSE;
        m_bCameraVfConfig = FALSE;
    }
     
    return ERROR_SUCCESS;
}

DWORD CSensorPdd :: SetPowerState( CEDEVICE_POWER_STATE PowerState )
{
   BOOL bRetEOF = TRUE; 
   CAM_FUNCTION_ENTRY();

   switch (PowerState)
   {
    case D0:
    case D1:
    case D2:
        {
            if( D0 != PowerCaps.DeviceDx )
            {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: Set D0.\r\n"), __WFUNCTION__));
                // For power management purposes, enable CSI and PRP
                //Befor enable csi and prp,turn clock on(only for TVIN clock)
                //for camera, enable csi is turn sensor clock on
                BSPDisableCamera(TRUE);

                // only when we are about to use them.
                m_pCsi->CsiEnable(IC_CHANNEL_VF);
                m_pCsi->CsiEnable(IC_CHANNEL_ENC);
                m_pPrp->PrpEnable();

                // Set the current Power status of the device.
                PowerCaps.DeviceDx = D0;
            }
        }
        break;

    case D3:
    case D4:
        {
            if( D4 != PowerCaps.DeviceDx )
            {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: Set D4.\r\n"), __WFUNCTION__));

                // For power management purposes, disable CSI and PRP
                // when we are stop the channels.
                m_pCsi->CsiDisable(IC_CHANNEL_ENC);
                m_pCsi->CsiDisable(IC_CHANNEL_VF);
                m_pPrp->PrpDisable();

                //After disable csi and prp,Turn clock off(only for TVIN clock)
                //for camera,enable csi is turn sensor clock on
                BSPDisableCamera(FALSE);

                // Set the current Power status of the device.
                PowerCaps.DeviceDx = D4;
            }   
        }
        break;
        
     default:
         break;
   }

   CAM_FUNCTION_EXIT();

   return ERROR_SUCCESS;
}

DWORD CSensorPdd::HandleAdapterCustomProperties( PUCHAR pInBuf, DWORD  InBufLen, PUCHAR pOutBuf, DWORD  OutBufLen, PDWORD pdwBytesTransferred )
{

    DWORD dwError = ERROR_NOT_SUPPORTED;
    PCSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S pCsPropVideoControlActFrameRateOutput = NULL;
    PCSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S pCsPropVideoControlActFrameRateInput = NULL;
    CSPROPERTY csProp = {0};

    if( !CeSafeCopyMemory( &csProp, pInBuf, sizeof( CSPROPERTY )))
    {
        return dwError;
    }

    *pdwBytesTransferred = 0;

    if (FALSE == IsEqualGUID( csProp.Set, PROPSETID_VIDCAP_VIDEOCONTROL ) )
         return dwError;
  
    switch( csProp.Id )
    {
        case CSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE:
         {
           if(NULL == (pCsPropVideoControlActFrameRateInput = reinterpret_cast<PCSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S>(pInBuf)))
                break;

           switch( csProp.Flags )
            {
                case CSPROPERTY_TYPE_GET:
                    *pdwBytesTransferred = sizeof(CSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S);
                    if (NULL == (pCsPropVideoControlActFrameRateOutput = reinterpret_cast<PCSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S>(pOutBuf)))
                    {
                        dwError = ERROR_MORE_DATA;
                        break;
                    }

                    //Copy the CSPROPERTY structure to the output buffer just in case!
                    memcpy( pCsPropVideoControlActFrameRateOutput, pCsPropVideoControlActFrameRateInput, sizeof(CSPROPERTY));

                    pCsPropVideoControlActFrameRateOutput->StreamIndex = pCsPropVideoControlActFrameRateInput->StreamIndex;
                    pCsPropVideoControlActFrameRateOutput->CurrentMaxAvailableFrameRate = 333000;
                    pCsPropVideoControlActFrameRateOutput->CurrentActualFrameRate = GetSensorFrameRate(pCsPropVideoControlActFrameRateInput->StreamIndex);

                    dwError = ERROR_SUCCESS;
                    break;
                    
                default:
                    DEBUGMSG(ZONE_IOCTL, (_T("CAM_IOControl: Invalid Request\r\n"))) ;
                    break;
            }
         
            break;
            }
         
        default:
           DEBUGMSG( ZONE_IOCTL, ( _T("IOControl Adapter PDD: Unsupported PropertySet Request\r\n")) );
           return dwError ;
    }     

    return dwError ;
}

DWORD CSensorPdd::InitSensorMode( ULONG ulModeType, LPVOID ModeContext )
{
    static BOOL displayInitialized = FALSE;
    DWORD i = 0;
    DWORD count = 0;
    
    for( i = 0; i < m_ulCTypes; i ++ )
    {
       if( NULL == m_ppModeContext[i] )
       {
         count ++;
       }
    }
    
    if( count == m_ulCTypes )
    {  
        SetPowerState(D0);
    }

    ASSERT( ModeContext );
    m_ppModeContext[ulModeType] = ModeContext;

    // Perform one-time configuration tasks
    // Get display characteristics for PrpClass.
    // Only perform once.
    if( !displayInitialized )
    {
        m_pPrp->PrpInitDisplayCharacteristics();
        displayInitialized = TRUE;
    }

    // Only Initialize sensor once.
    if( !m_SensorConfigured )
    {
        m_pCsi->CsiConfigureSensor();
        m_SensorConfigured = TRUE;
    }

    /****************************************************************/
    // Configure CSI
    /****************************************************************/ 
    if( count == m_ulCTypes )
    {  
        SensorConfigCSI();
    }

    return ERROR_SUCCESS;
}

DWORD CSensorPdd::DeInitSensorMode( ULONG ulModeType )
{
    DWORD i = 0;
    DWORD count = 0;    

    SensorClosePin(ulModeType);

    m_ppModeContext[ulModeType] = NULL;

    for( i = 0; i < m_ulCTypes; i ++ )
    {
       if( NULL == m_ppModeContext[i] )
       {
          count ++;
       }
    }

    // If disable CSI&PRP ,cannot suspend
    // But add the function CsiDetectEOF before csidiable(),it will remove this issue. 
    if( count == m_ulCTypes )
    {  // If all pins are closed.
        SetPowerState(D4);
    }

    return ERROR_SUCCESS;
}

DWORD CSensorPdd::SetSensorState( ULONG ulModeType, CSSTATE csState )
{
    DWORD dwError = ERROR_SUCCESS;
    LPVOID ModeContext=NULL;
 
    switch ( csState )
    {
        case CSSTATE_STOP:

            m_CsState[ulModeType] = CSSTATE_STOP;

            // Kill the timer to conserve resources
            if ( NULL != m_TimerIdentifier[ulModeType] )            
            {
                ASSERT( m_pfnTimeKillEvent );
                m_pfnTimeKillEvent(m_TimerIdentifier[ulModeType]);
                m_TimerIdentifier[ulModeType] = NULL;
                dwError = ERROR_INVALID_PARAMETER;
            }

            if( STILL == ulModeType )
            {
                m_bStillCapInProgress = false;

                if( m_bCameraStillConfig )
                {
                    PostStillPicture(STILL);
                    m_bCameraStillConfig = FALSE;
                }

                dwError = ERROR_SUCCESS;
                break;
            }

            if( PREVIEW == ulModeType )
            {
                if( !m_pPrp->PrpStopVfChannel() )
                {
                    break;
                }
            }
            else if ( CAPTURE == ulModeType )
            {
                if( !m_pPrp->PrpStopEncChannel() )
                {
                    break;
                }
            }

            break;

        case CSSTATE_PAUSE:

            m_CsState[ulModeType] = CSSTATE_PAUSE;
           
            if( STILL == ulModeType )
            {
                 dwError = ERROR_SUCCESS;
                 break;
            }

            if( PREVIEW == ulModeType )
            {
                if( !m_pPrp->PrpPauseViewfinding() )
                {
                   DEBUGMSG(ZONE_IOCTL, (_T("PIN_IOControl(%08x): Failed disabling viewfinding in display!\r\n"), this));
                   break;
                }

                //----------------------------------------------------
                // Configure viewfinding path if Preview pin is used
                // and configuration is needed.
                //----------------------------------------------------
                if( !m_bCameraVfConfig )
                {
                    if( !m_pPrp->PrpStopVfChannel() )
                    {
                        break;
                    }
                   
                    // Config Viewfinding path
                    if( !SensorViewfindingConfig() )
                    {
                        dwError = ERROR_ACCESS_DENIED;
                    }

                    #ifndef RINGO_FIX_MEM_ALLOC_ISSUE
                    // Allocate Viewfinding buffers 
                    if( !m_bCameraVfAllocBuf )
                    {
                        if( !SensorVfBufferAlloc(m_ulNumOfBuffer))
                        {
                            dwError = ERROR_ACCESS_DENIED;
                        }
                    }
                    #endif
                }
            }
            else if ( CAPTURE == ulModeType )
            {
                 //----------------------------------------------------
                 // Configure encoding path if Capture pin is used
                 // and configuration is needed.
                 //----------------------------------------------------
                 if( !m_bCameraEncConfig ) 
                 {
                     if( !m_pPrp->PrpStopEncChannel() )
                     {
                         break;
                     }
       
                     // Config Encoding path
                     if( !SensorEncodingConfig(ulModeType) )
                     {
                         dwError = ERROR_ACCESS_DENIED;
                     }

                     #ifndef RINGO_FIX_MEM_ALLOC_ISSUE
                     // Allocate Enc buffers
                     if( !m_bCameraEncAllocBuf )
                     {
                         if( !SensorEncBufferAlloc(ulModeType,m_ulNumOfBuffer) )
                         {
                             dwError = ERROR_ACCESS_DENIED;
                         }
                     }
                     #endif
                }
            }

            break;

        case CSSTATE_RUN:

            m_CsState[ulModeType] = CSSTATE_RUN;
            
            // For STILL pin, just change the state variable...
            // Image capture is configured in HandlePinIO.
            if( STILL == ulModeType )
            {
                dwError = ERROR_SUCCESS;
                break;
            }

            #if 0
            if( NULL == m_TimerIdentifier[ulModeType] )            
            {
                dwError = ( CreateTimer( ulModeType ) ? ERROR_SUCCESS : ERROR_NOT_READY );
            }
            #endif
            
            if( PREVIEW == ulModeType )
            {
                // Start Viewfinding path                                   
                if ( !m_pPrp->PrpStartVfChannel() )
                {
                    break;
                }

                SetEvent(m_hPinVfEvent[0]);
            }
            else if ( CAPTURE == ulModeType )
            {
                // Start Encoding path
                if( !m_pPrp->PrpStartEncChannel() )
                {
                    break;
                }

                SetEvent(m_hPinEncEvent[0]);
            }

           break;

        default:
            DEBUGMSG( ZONE_IOCTL|ZONE_ERROR, ( _T("IOControl(%08x): Incorrect State\r\n"), this ) );
            dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}

DWORD CSensorPdd::TakeStillPicture( LPVOID pBurstModeInfo )
{
    DWORD dwError = ERROR_SUCCESS;
    m_bStillCapInProgress = TRUE;
    //Ignore pBurstModeInfo
    m_CsState[STILL] = CSSTATE_RUN;

    if( !m_bCameraStillConfig )
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Still Pin configure\r\n")));
        PreStillPicture(STILL);
        m_bCameraStillConfig = TRUE;
    }
    
    if ( NULL == m_TimerIdentifier[STILL] )
    {
        dwError = ( CreateTimer( STILL ) ? ERROR_SUCCESS : ERROR_NOT_READY );
    }

    return dwError;
}


DWORD CSensorPdd::GetSensorModeInfo( ULONG ulModeType, PSENSORMODEINFO pSensorModeInfo )
{
    pSensorModeInfo->MemoryModel = m_SensorModeInfo[ulModeType].MemoryModel;
    pSensorModeInfo->MaxNumOfBuffers = m_SensorModeInfo[ulModeType].MaxNumOfBuffers;
    pSensorModeInfo->PossibleCount = m_SensorModeInfo[ulModeType].PossibleCount;
    pSensorModeInfo->VideoCaps.DefaultVideoControlCaps = DefaultVideoControlCaps[ulModeType];
    pSensorModeInfo->VideoCaps.CurrentVideoControlCaps = m_pModeVideoCaps[ulModeType].CurrentVideoControlCaps;
    pSensorModeInfo->pVideoFormat = &m_pModeVideoFormat[ulModeType];
    
    return ERROR_SUCCESS;
}

DWORD CSensorPdd::SetSensorModeFormat( ULONG ulModeType, PCS_DATARANGE_VIDEO pCsDataRangeVideo )
{
    memcpy( &m_pCurrentFormat[ulModeType], pCsDataRangeVideo, sizeof ( CS_DATARANGE_VIDEO ) );

    // Assure that we will reconfigure IPU before starting.
    SensorMarkAsModified(ulModeType);
   
    return ERROR_SUCCESS;
}

DWORD CSensorPdd::GetSensorFrameRate( ULONG ulModeType)
{
    DWORD dwTickCount, dwTickCount2, dwCurTickCount;
    DWORD curframeCount=0;
    DWORD lastframeCount=0;
    DWORD totalframeCount=0;
    DWORD maxframeCount=0;

    totalframeCount = lastframeCount = curframeCount = 0;
    dwTickCount = GetTickCount();

    maxframeCount = -1;

    if (ulModeType == PREVIEW)
    {
        curframeCount = m_pPrp->PrpGetVfFrameCount();
    }
    else if (ulModeType == CAPTURE)
    {
        curframeCount = m_pPrp->PrpGetEncFrameCount();
    }
    else
    {
        return 0;
    }
    lastframeCount = curframeCount;

    while(TRUE)
    {
        if(curframeCount < lastframeCount) 
        {
            // reach maximum, so add maxFrame
            totalframeCount += maxframeCount - lastframeCount + curframeCount;
        } 
        else 
        {
            totalframeCount += curframeCount - lastframeCount;
        }
        
        // save the current count
        lastframeCount = curframeCount;

        dwCurTickCount = GetTickCount() - dwTickCount;

        // If we have reached our target frame count, or if we have reached our timeout time,
        // we compute the frame rate and return.
        if( (totalframeCount >= 10)  || (dwCurTickCount > 3000))
        {
            if (totalframeCount > 0)
          dwTickCount2 = (dwCurTickCount) * 10000 / totalframeCount;
            else
                dwTickCount2 = 0;
            break;
        }
        
        Sleep(10);

        //get the frame count from the Prp
        if (ulModeType == PREVIEW)
        {
            curframeCount = m_pPrp->PrpGetVfFrameCount();
        }
        else
        {
            curframeCount = m_pPrp->PrpGetEncFrameCount();
        }
    }

    return dwTickCount2;
}

PVOID CSensorPdd::AllocateBuffer( ULONG ulModeType )
{  
    // Real PDD may want to save off this allocated pointer
    // in an array.
    ULONG ulFrameSize = abs(CS_DIBSIZE (m_pCurrentFormat[ulModeType].VideoInfoHeader.bmiHeader));

    return RemoteLocalAlloc( LPTR, ulFrameSize );
}

DWORD CSensorPdd::DeAllocateBuffer( ULONG ulModeType, PVOID pBuffer )
{
    RemoteLocalFree( pBuffer );
    return ERROR_SUCCESS;
}

DWORD CSensorPdd::RegisterClientBuffer( ULONG ulModeType, PVOID pBuffer )
{
    // Real PDD may want to save pBuffer which is a pointer to buffer that DShow created.

    return ERROR_SUCCESS;
}

DWORD CSensorPdd::UnRegisterClientBuffer( ULONG ulModeType, PVOID pBuffer )
{
    // DShow is not going to use pBuffer (which was originally allocated by DShow) anymore. If the PDD
    // is keeping a cached pBuffer pointer (in RegisterClientBuffer()) then this is the right place to
    // stop using it and maybe set the cached pointer to NULL. 
    // Note: PDD must not delete this pointer as it will be deleted by DShow itself
    return ERROR_SUCCESS;
}

DWORD CSensorPdd::HandleSensorModeCustomProperties( ULONG ulModeType, PUCHAR pInBuf, DWORD  InBufLen, PUCHAR pOutBuf, DWORD  OutBufLen, PDWORD pdwBytesTransferred )
{
    DEBUGMSG( ZONE_IOCTL, ( _T("IOControl Adapter PDD: Unsupported PropertySet Request\r\n")) );
    return ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The following code is only meant for this imagic chip cmos sensor pdd
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CSensorPdd::FillBuffer( ULONG ulModeType, PUCHAR pImage )
{
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr = &m_pCurrentFormat[ulModeType].VideoInfoHeader;
    UINT biSizeImage    = pCsVideoInfoHdr->bmiHeader.biSizeImage;
    PUINT8 pbySrcBuffer = NULL, pbyDstBuffer = NULL;
    BYTE byTVinType     = 0;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("PIN_Function(%08x): +FillBuffer\r\n"), this));

    while (1)
    {
        switch (ulModeType)
        {
            case PREVIEW:
                //For Ringo TVIN+:
                if(m_pCsi->m_iCamType == 4)// 4:csiTVinId_ADV7180
                {
                    // For Ringo TVIN+:only for NTSC change to PAL
                    if ( m_pPrp->m_bNTSCtoPAL )
                    {
                        m_pPrp->m_bNTSCtoPAL = FALSE;

                        //Reset CSI output size and prp input size
                        m_pPrp->PrpStopVfChannel();
                        Sleep(50);
                        SensorConfigCSI(); 
                        SensorViewfindingConfig();
                        Sleep(50);
                        m_pPrp->PrpStartVfChannel();
                        Sleep(100);
                    }

                    // For Ringo TVIN+:only for PAL change to NTSC
                    if ( gLastTVinType == 4 )//PAL
                    {
                        byTVinType = BSPGetTVinType();
                                
                        if (byTVinType != gLastTVinType)
                        {
                            
                            if (gLastTVinType == 0)
                                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: TVin Type is changing from NTSC to PAL.\r\n"), __WFUNCTION__));
                            else if (gLastTVinType == 4)
                                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: TVin Type is changing from PAL to NTSC.\r\n"), __WFUNCTION__));
                            gLastTVinType = byTVinType; 

                            // For Ringo TVIN+:only for PAL change to NTSC
                            // PAL is 720*576(scanline is 720*625) and csi output and prp input size is set to PAL
                            // when tv type changes to NTSC 720*480,the size is smaller than PAL,
                            // the DVD output size will be always smaller than prp input size,and cannot generate EOF interrupt  
                            m_pPrp->PrpStopVfChannel();
                            Sleep(100);
                            SetPowerState(D4);
                            Sleep(500);
                            SetPowerState(D0);
                            Sleep(50);
                            SensorConfigCSI(); 
                            SensorViewfindingConfig();
                            Sleep(50);
                            m_pPrp->PrpStartVfChannel();
                            Sleep(100);               
                        }
                    }
                }

                pbySrcBuffer = (UINT8 *)m_pPrp->PrpGetVfBufFilled();

                if (pbySrcBuffer == NULL)
                {
                    if (WaitForSingleObject(m_pPrp->m_hVfEOFEvent, WAIT_BUFFER_TIMEOUT) != WAIT_OBJECT_0) 
                    {
                        RETAILMSG(0,(TEXT("WAIT_BUFFER_TIMEOUT:%d"),WAIT_BUFFER_TIMEOUT));
                        ERRORMSG(ZONE_ERROR, (TEXT("+FillBuffer:WaitForSingleObject m_hPrPBuffFilled timeout!\r\n")));
                        if(m_pCsi->m_iCamType != 4)
                        {
                            //If we can not receive anything from PrP, some times PrP is blocked ,so just restart it.It is not a good way,but it works.                        
                            m_pPrp->PrpStopVfChannel();
                            Sleep(500);
                            m_pPrp->PrpStartVfChannel();
                        }
                        else// 4:csiTVinId_ADV7180
                        {
                            // For Ringo TVIN+:only for PAL change to NTSC
                            // PAL is 720*576(scanline is 720*625) and csi output and prp input size is set to PAL
                            // when tv type changes to NTSC 720*480,the size is smaller than PAL,
                            // the DVD output size will be always smaller than prp input size,and cannot generate EOF interrupt  
                            byTVinType = BSPGetTVinType();
                            if (byTVinType == 0)
                                RETAILMSG (0,(TEXT("%s: Get TVin Type is NTSC.\r\n"), __WFUNCTION__));
                            else
                                RETAILMSG (0,(TEXT("%s: Get TVin Type is PAL.\r\n"), __WFUNCTION__));    

                            if( byTVinType != gLastTVinType )
                            {
                                gLastTVinType = byTVinType; 
                            }
                            else
                            {
                                RETAILMSG (0,(TEXT("%s: *****************************.\r\n"), __WFUNCTION__));
                                m_pPrp->PrpReadInterruptRegisters();
                                RETAILMSG (0,(TEXT("%s: *****************************.\r\n"), __WFUNCTION__));
                            }
                            
                            m_pPrp->PrpStopVfChannel();
                            Sleep(100);
                            SetPowerState(D4);
                            Sleep(500);
                            SetPowerState(D0);
                            Sleep(50);
                            SensorConfigCSI(); 
                            SensorViewfindingConfig();
                            Sleep(50);
                            m_pPrp->PrpStartVfChannel();
                            Sleep(100);
                        }

                        return biSizeImage;
                    }

                    pbySrcBuffer = (UINT8 *)m_pPrp->PrpGetVfBufFilled();
                }
                else
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer: VF filled buffer ready!\r\n")));
                }

                break;

            case CAPTURE:

                pbySrcBuffer = (UINT8 *)m_pPrp->PrpGetEncBufFilled();

                if (pbySrcBuffer == NULL)
                {
                    if (WaitForSingleObject(m_pPrp->m_hEncEOFEvent, WAIT_BUFFER_TIMEOUT) != WAIT_OBJECT_0) 
                    {
                        DEBUGMSG(ZONE_ERROR, (TEXT("+FillBuffer:WaitForSingleObject m_hPrPBuffFilled timeout!\r\n")));
                        //If we can not receive anything from PrP, some times PrP is blocked ,so just restart it.It is not a good way,but it works.
                        m_pPrp->PrpStopEncChannel();
                        Sleep(500);
                        m_pPrp->PrpStartEncChannel();

                        return biSizeImage;
                    }

                    pbySrcBuffer = (UINT8 *) m_pPrp->PrpGetEncBufFilled();
                }
                else
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer: ENC filled buffer ready!\r\n")));
                }

                break;
            
            case STILL:
 
                 pbySrcBuffer = (UINT8 *)m_pPrp->PrpGetEncBufFilled();
 
                 if (pbySrcBuffer == NULL)
                 {
                     if (WaitForSingleObject(m_pPrp->m_hEncEOFEvent, WAIT_BUFFER_TIMEOUT) != WAIT_OBJECT_0) 
                     {
                         DEBUGMSG(ZONE_ERROR, (TEXT("+FillBuffer:WaitForSingleObject m_hPrPBuffFilled timeout!\r\n")));
                         //If we can not receive anything from PrP, some times PrP is blocked ,so just restart it.It is not a good way,but it works.
                         m_pPrp->PrpStopEncChannel();
                         Sleep(500);
                         m_pPrp->PrpStartEncChannel();

                         return biSizeImage;
                     }
                     
                     pbySrcBuffer = (UINT8 *) m_pPrp->PrpGetEncBufFilled();
                 }
                 else
                 {
                     DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer: ENC filled buffer ready!\r\n")));
                 }

                 break;

            default:
                DEBUGMSG(ZONE_ERROR,(_T("FillBuffer: Invalid pin id.\r\n"))) ;

                return 0;
        }

        // If we got a buffer from Prp, exit loop and continue.
        // If there was no buffer returned, loop again and wait for one.
        if (pbySrcBuffer != NULL)
        {
            break;
        }

        DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer: pbySrcBuffer NULL!  Waiting again...\r\n")));
    }

    pbyDstBuffer =  reinterpret_cast<PUINT8>(pImage);

    memcpy(pbyDstBuffer, pbySrcBuffer,biSizeImage);
    
    if (ulModeType != 2)
        DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: ulModeType: %x,biSizeImage: %x, phyDstBuffer: %x, pbySrcBuffer: %x\r\n"), 
        __WFUNCTION__, ulModeType,biSizeImage,pbyDstBuffer,pbySrcBuffer));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer:buffer filled success!\r\n")));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-FillBuffer\r\n")));

    // return the size of the image filled
    return(biSizeImage);    
}

BOOL CSensorPdd :: CreateTimer( ULONG ulModeType )
{
   if ( NULL == m_hTimerDll )
   {
        m_hTimerDll        = LoadLibrary( L"MMTimer.dll" );
        m_pfnTimeSetEvent  = (FNTIMESETEVENT)GetProcAddress( m_hTimerDll, L"timeSetEvent" );
        m_pfnTimeKillEvent = (FNTIMEKILLEVENT)GetProcAddress( m_hTimerDll, L"timeKillEvent" );

        if ( NULL == m_pfnTimeSetEvent || NULL == m_pfnTimeKillEvent )
        {
            DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl(%08x): GetProcAddress Returned Null.\r\n"), this));
            return FALSE;
        }
    }
    
    if ( NULL == m_hTimerDll )
    {
            DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl(%08x): LoadLibrary failedr.\r\n"), this));
            return FALSE;
    }

    ASSERT( m_pfnTimeSetEvent );

    if ( NULL == m_TimerIdentifier[ulModeType] )
    {
        DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl(%08x): Creating new timer.\r\n"), this));
        if( STILL == ulModeType )
        {
            m_TimerIdentifier[ulModeType] = m_pfnTimeSetEvent( (ULONG)m_pCurrentFormat[ulModeType].VideoInfoHeader.AvgTimePerFrame/10000, 10, CSensorPdd::StillTimerCallBack, reinterpret_cast<DWORD>(this), TIME_PERIODIC|TIME_CALLBACK_FUNCTION);
        }
        else
        {
            m_TimerIdentifier[ulModeType] = m_pfnTimeSetEvent( (ULONG)m_pCurrentFormat[ulModeType].VideoInfoHeader.AvgTimePerFrame/10000, 10, CSensorPdd::CaptureTimerCallBack, reinterpret_cast<DWORD>(this), TIME_PERIODIC|TIME_CALLBACK_FUNCTION);
        }

        if ( NULL == m_TimerIdentifier[ulModeType] )
        {
            DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl(%08x): Timer could not be created.\r\n"), this));
            return FALSE;
        }
    }
    else
    {
        DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl(%08x): Timer already created.\r\n"), this));
    }

    return TRUE;
}

void CSensorPdd :: CaptureTimerCallBack( UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2 )
{
    UNREFERENCED_PARAMETER(uTimerID) ;
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(dw1);
    UNREFERENCED_PARAMETER(dw2);

    CSensorPdd *pSensorPdd= reinterpret_cast<CSensorPdd *>(dwUser);
    if( NULL == pSensorPdd )
    {
        DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl: TimerCallBack pSensorPdd is NULL.\r\n"))) ;
    }
    else
    {
        __try
        {
            pSensorPdd->HandleCaptureInterrupt( uTimerID );
        }
        __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl: TimerCallBack, Access violation.\r\n"))) ;
        }
    }
}

void CSensorPdd :: StillTimerCallBack( UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2 )
{
    UNREFERENCED_PARAMETER(uTimerID) ;
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(dw1);
    UNREFERENCED_PARAMETER(dw2);

    CSensorPdd *pSensorPdd= reinterpret_cast<CSensorPdd *>(dwUser);
    if( NULL == pSensorPdd )
    {
        DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl: TimerCallBack pSensorPdd is NULL.\r\n"))) ;
    }
    else
    {
        __try
        {
            pSensorPdd->HandleStillInterrupt( uTimerID );
        }
        __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl: TimerCallBack, Access violation.\r\n"))) ;
        }
    }
}

void CSensorPdd :: HandleCaptureInterrupt( UINT uTimerID )
{
    ULONG ulModeType;

    if( m_bStillCapInProgress )
    {
        return;
    }
    
    if( m_TimerIdentifier[CAPTURE] == uTimerID )
    {
        ulModeType = CAPTURE;
    }
    else if ( m_ulCTypes == MAX_SUPPORTED_PINS && m_TimerIdentifier[PREVIEW] == uTimerID )
    {
        ulModeType = PREVIEW;
    }
    else
    {
        ASSERT(FALSE);
        return;
    }

    MDD_HandleIO( m_ppModeContext[ulModeType], ulModeType );
}


void CSensorPdd :: HandleStillInterrupt( UINT uTimerID )
{
    MDD_HandleIO( m_ppModeContext[STILL], STILL );
    m_bStillCapInProgress = FALSE;
}

BOOL CSensorPdd::ReadMemoryModelFromRegistry()
{
    HKEY  hKey = 0;
    DWORD dwType  = 0;
    DWORD dwSize  = sizeof ( DWORD );
    DWORD dwValue = -1;


    if( ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\DirectX\\DirectShow\\Capture", 0, 0, &hKey ))
    {
        return FALSE;
    }

    if( ERROR_SUCCESS == RegQueryValueEx( hKey, L"MemoryModel", 0, &dwType, (BYTE *)&dwValue, &dwSize ) )
    {
        if(   ( REG_DWORD == dwType ) 
           && ( sizeof( DWORD ) == dwSize ) 
           && (( dwValue == CSPROPERTY_BUFFER_DRIVER ) || ( dwValue == CSPROPERTY_BUFFER_CLIENT_LIMITED ) || ( dwValue == CSPROPERTY_BUFFER_CLIENT_UNLIMITED )))
        {
            for( int i=0; i<NUM_SUPPORTED_PINS ; i++ )
            {
                m_SensorModeInfo[i].MemoryModel = (CSPROPERTY_BUFFER_MODE) dwValue;
            }
        }
    }

    // Find out if we should be using some other number of supported modes. The only
    // valid options are 2 or 3. Default to 2.
    if ( ERROR_SUCCESS == RegQueryValueEx( hKey, L"PinCount", 0, &dwType, (BYTE *)&dwValue, &dwSize ) )
    {
        if ( REG_DWORD == dwType
             && sizeof ( DWORD ) == dwSize
             && MAX_SUPPORTED_PINS == dwValue )
        {
            m_ulCTypes = MAX_SUPPORTED_PINS;
        }
    }

    RegCloseKey( hKey );
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: SensorConfigCSI
//
// This function configures the CSI moduel.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorConfigCSI()
{
    csiSensorOutputFormat sensorOutputFormat;
    csiSensorOutputResolution sensorOutputResolution;

    //----------------------------------------------------
    // Configure CSI
    //----------------------------------------------------
    if (!m_pCsi->CsiConfigure(&sensorOutputFormat, &sensorOutputResolution))
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Sensor configuration failed.  Aborting remaining configuration steps.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    //----------------------------------------------------
    // Set up preprocessor input parameters common
    // to both encoding and viewfinding tasks
    //----------------------------------------------------
    if (sensorOutputFormat == csiSensorOutputFormat_RGB)
    {
        m_inFormat = prpInputFormat_RGB; // Interlaced RGB888
    }
    else if ((sensorOutputFormat == csiSensorOutputFormat_YUV422) || 
        (sensorOutputFormat == csiSensorOutputFormat_YUV444))
    {
        m_inFormat = prpInputFormat_UYVY422;
    }
    else
    {
        m_inFormat = prpInputFormat_Generic; // Bayer data
    }

    // Set up Preproc input width and height configuration
    // parameters from the output resolution.
    switch (sensorOutputResolution)
    {
        case csiSensorOutputResolution_UXGA:            
           m_inFrameSize.width = UXGA_Width;
           m_inFrameSize.height = UXGA_Height;
           break;

        case csiSensorOutputResolution_SXGA:
            m_inFrameSize.width = SXGA_Width;
            m_inFrameSize.height = SXGA_Height;
            break;
            
        case csiSensorOutputResolution_XGA:
            m_inFrameSize.width = XGA_Width;
            m_inFrameSize.height = XGA_Height;
            break;

        case csiSensorOutputResolution_SVGA:
            m_inFrameSize.width = SVGA_Width;
            m_inFrameSize.height = SVGA_Height;
            break;

        case csiSensorOutputResolution_VGA:
            m_inFrameSize.width = VGA_Width;
            m_inFrameSize.height = VGA_Height;
            break;

        case csiSensorOutputResolution_QVGA:
            m_inFrameSize.width = QVGA_Width;
            m_inFrameSize.height = QVGA_Height;
            break;

        case csiSensorOutputResolution_CIF:
            m_inFrameSize.width = CIF_Width;
            m_inFrameSize.height = CIF_Height;
            break;

        case csiSensorOutputResolution_QCIF:
            m_inFrameSize.width = QCIF_Width;
            m_inFrameSize.height = QCIF_Height;
            break;

        case csiSensorOutputResolution_QQVGA:
            m_inFrameSize.width = QQVGA_Width;
            m_inFrameSize.height = QQVGA_Height;
            break;

        //only For Ringo TVIN +
        case csiSensorOutputResolution_PAL:
            m_inFrameSize.width = PAL_Width;
            m_inFrameSize.height = PAL_Height/2;
            break;

        //only for Ringo TVIN +    
        //There is a white line in front of the field when Playing NTSC.
        //So skip some line to remove the white line.
        //when prp input size is smaller than prp output size,the image will generate the  mosaic
        //so need ensure input size is bigger than output size
        case csiSensorOutputResolution_NTSC:
            m_inFrameSize.width = NTSC_Width;
            m_inFrameSize.height = NTSC_Height/2 - SKIP_NTSC;
            break;   
            
        default:
            break;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SensorViewfindingConfig
//
// This function configures the viewfinding path of preprocessing module.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorViewfindingConfig()
{
    prpVfConfigData prpVfConfig;
    DWORD Format;

    /****************************************************************/
    // Configure prp viewfinding path
    /****************************************************************/ 
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr = &m_pCurrentFormat[PREVIEW].VideoInfoHeader;

    // Set input format and frame size config params
    prpVfConfig.inputFormat = m_inFormat;
    prpVfConfig.inputSize = m_inFrameSize;

    Format = pCsVideoInfoHdr->bmiHeader.biCompression;

    switch (Format)
    {
        case (CS_BI_BITFIELDS|BI_SRCPREROTATE):
            prpVfConfig.vfFormat = prpVfOutputFormat_RGB;
            break;

        case (CS_BI_RGB|BI_SRCPREROTATE):
             prpVfConfig.vfFormat = prpVfOutputFormat_RGB;
             break;

        default:
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Invalid data format for preview.\r\n"), __WFUNCTION__));
            return FALSE;
    }

    // Set up viewfinding channel CSC parameters
    // based on input.  We know output is RGB.
    if (prpVfConfig.inputFormat == prpInputFormat_UYVY422)
    {
        prpVfConfig.vfCSCEquation = prpCSCY2R_A1;
    }
    else
    {
        prpVfConfig.vfCSCEquation = prpCSCNoOp;
    }

    // TODO: Support Rotation
    prpVfConfig.vfFlipRot.verticalFlip = m_bFlipVertical;
    prpVfConfig.vfFlipRot.horizontalFlip = m_bFlipHorizontal;
    prpVfConfig.vfFlipRot.rotate90 = m_bRotate;

    // If height in bmiHeader is positive, the capture image should
    // be bottom-up, so we vertically flip the image.
    if (abs(pCsVideoInfoHdr->bmiHeader.biHeight) == pCsVideoInfoHdr->bmiHeader.biHeight)
    {
        prpVfConfig.vfFlipRot.verticalFlip = prpVfConfig.vfFlipRot.verticalFlip ? FALSE : TRUE;
    }

    // Set up viewfinding image output dimensions
    prpVfConfig.vfSize.width =  (UINT16)pCsVideoInfoHdr->bmiHeader.biWidth;
    prpVfConfig.vfSize.height = (UINT16)(abs(pCsVideoInfoHdr->bmiHeader.biHeight));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: Viewfinding output width: %x, height: %x\r\n"), 
                __WFUNCTION__, prpVfConfig.vfSize.width, prpVfConfig.vfSize.height));

    // Set direct display variable
    prpVfConfig.directDisplay = m_bDirectDisplay;

    // Set up viewfinding window offset
    prpVfConfig.vfOffset.x = (UINT16)pCsVideoInfoHdr->rcTarget.left;            
    prpVfConfig.vfOffset.y = (UINT16)pCsVideoInfoHdr->rcTarget.top;

    //----------------------------------------------------
    // Configure Preprocessor
    //----------------------------------------------------
    if (m_pPrp->PrpConfigureViewfinding(&prpVfConfig))
    {
        // Camera has been configured for viewfinding.
        m_bCameraVfConfig = TRUE;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Preprocessor viewfinding configure failed.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: SensorVfBufferAlloc
//
// This function Allocates the viewfinding path of  preprocessing module buffers.
//
// Parameters:
//      iBufNum
//          [in] Number of buffers to allocate for Pre-processing
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorVfBufferAlloc(UINT32 iBufNum)
{
    ULONG ulBufSize;
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr = &m_pCurrentFormat[PREVIEW].VideoInfoHeader;

    //----------------------------------------------------
    // Re-allocate Preprocessor Buffers
    //----------------------------------------------------
    if (!m_pPrp->PrpDeleteVfBuffers())
    {
        return FALSE;
    }

    ulBufSize = abs(CS_DIBSIZE(pCsVideoInfoHdr->bmiHeader)); // by bytes

    if (!m_pPrp->PrpAllocateVfBuffers(iBufNum, ulBufSize))
    {
        ERRORMSG(TRUE,(_T("%s: Prp Buffer allocation failed.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_bCameraVfAllocBuf = TRUE;

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: SensorEncodingConfig
//
// This function configures the encoding path of the preprocessing module.
//
// Parameters:
//      iEncPin
//          [in] Specifies whether data format for STILL pin or
//          CAPTURE pin should be used to configure the encoding path.
//
//      iBufNum
//          [in] Number of buffers to allocate for Pre-processing
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorEncodingConfig(UINT32 iEncPin)
{
    prpEncConfigData prpEncConfig;
    DWORD Format;
    WORD bitDepth;
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr ;

    /****************************************************************/
    // Configure prp viewfinding path
    /****************************************************************/ 
    pCsVideoInfoHdr = &m_pCurrentFormat[iEncPin].VideoInfoHeader;

    // Set input format and frame size config params
    prpEncConfig.inputFormat = m_inFormat;
    prpEncConfig.inputSize = m_inFrameSize;

    Format = pCsVideoInfoHdr->bmiHeader.biCompression;
    
    switch (Format)
    {
        case (FOURCC_YV12|BI_SRCPREROTATE):
            prpEncConfig.encFormat = prpEncOutputFormat_YUV420;
            break;

        case (FOURCC_YUYV|BI_SRCPREROTATE):
            prpEncConfig.encFormat = prpEncOutputFormat_YUYV422;
            break;
            
        case (FOURCC_UYVY|BI_SRCPREROTATE):
            prpEncConfig.encFormat = prpEncOutputFormat_UYVY422;
            break;

        case (CS_BI_BITFIELDS|BI_SRCPREROTATE):
        case (CS_BI_RGB|BI_SRCPREROTATE):
            prpEncConfig.encFormat = prpEncOutputFormat_RGB;
            break;

        default:
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Invalid data format for Capture/Still.\r\n"), __WFUNCTION__));
            return FALSE;
    }

    // Set up encoding channel CSC parameters
    // based on input and output
    switch(prpEncConfig.encFormat)
    {
        case prpEncOutputFormat_YUV444:
        case prpEncOutputFormat_YUV422:
        case prpEncOutputFormat_YUV420:
        case prpEncOutputFormat_YUV444IL:
        case prpEncOutputFormat_YUYV422:
        case prpEncOutputFormat_YVYU422:
        case prpEncOutputFormat_UYVY422:
        case prpEncOutputFormat_VYUY422:
            if (prpEncConfig.inputFormat == prpInputFormat_RGB)
                prpEncConfig.encCSCEquation = prpCSCR2Y_A1;
            else
                prpEncConfig.encCSCEquation = prpCSCNoOp;
            break;
        case prpEncOutputFormat_RGB:
        case prpEncOutputFormat_RGBA:
            if (prpEncConfig.inputFormat == prpInputFormat_UYVY422)
                prpEncConfig.encCSCEquation = prpCSCY2R_A1;
            else
                prpEncConfig.encCSCEquation = prpCSCNoOp;
            break;
    }

    // Set up encoding data width if format is RGB
    // If format is not RGB, width and offsets can be inferred
    if (prpEncConfig.encFormat == prpEncOutputFormat_RGB)
    {
        bitDepth = pCsVideoInfoHdr->bmiHeader.biBitCount;
        switch (bitDepth)
        {
            case 32:
                // R = 10, G = 12, B = 10 ????
                prpEncConfig.encDataWidth = prpDataWidth_32BPP;
                prpEncConfig.encRGBPixelFormat.component0_offset = 0;
                prpEncConfig.encRGBPixelFormat.component1_offset = 10;
                prpEncConfig.encRGBPixelFormat.component2_offset = 22;
                prpEncConfig.encRGBPixelFormat.component0_width = 10;
                prpEncConfig.encRGBPixelFormat.component1_width = 12;
                prpEncConfig.encRGBPixelFormat.component2_width = 10;
                break;
            case 24:
                prpEncConfig.encDataWidth = prpDataWidth_24BPP;
                prpEncConfig.encRGBPixelFormat.component0_offset = 0;
                prpEncConfig.encRGBPixelFormat.component1_offset = 8;
                prpEncConfig.encRGBPixelFormat.component2_offset = 16;
                prpEncConfig.encRGBPixelFormat.component0_width = 8;
                prpEncConfig.encRGBPixelFormat.component1_width = 8;
                prpEncConfig.encRGBPixelFormat.component2_width = 8;
                break;
            case 16:
                prpEncConfig.encDataWidth = prpDataWidth_16BPP;
                prpEncConfig.encRGBPixelFormat.component0_offset = 0;
                prpEncConfig.encRGBPixelFormat.component1_offset = 5;
                prpEncConfig.encRGBPixelFormat.component2_offset = 11;
                prpEncConfig.encRGBPixelFormat.component0_width = 5;
                prpEncConfig.encRGBPixelFormat.component1_width = 6;
                prpEncConfig.encRGBPixelFormat.component2_width = 5;
                break;
            default:
                DEBUGMSG(ZONE_ERROR, (TEXT("%s: Invalid bit depth 0x%x for Capture/Still image.\r\n"), 
                    __WFUNCTION__, bitDepth));
                return FALSE;
        }
    }

    // TODO: Support Rotation
    prpEncConfig.encFlipRot.verticalFlip = m_bFlipVertical;
    prpEncConfig.encFlipRot.horizontalFlip = m_bFlipHorizontal;
    prpEncConfig.encFlipRot.rotate90 = m_bRotate;

    // If height in bmiHeader is positive, the captured image should
    // be bottom-up, so we vertically flip the image.
    if (abs(pCsVideoInfoHdr->bmiHeader.biHeight) == pCsVideoInfoHdr->bmiHeader.biHeight)
    {
        prpEncConfig.encFlipRot.verticalFlip = prpEncConfig.encFlipRot.verticalFlip ? FALSE : TRUE;
    }
    // Set up encoding image output dimensions
    prpEncConfig.encSize.width = (UINT16)pCsVideoInfoHdr->bmiHeader.biWidth;
    prpEncConfig.encSize.height = (UINT16)(abs(pCsVideoInfoHdr->bmiHeader.biHeight));

    DEBUGMSG(ZONE_DEVICE, (TEXT("%s: Encoding output width: %x, height: %x\r\n"), 
        __WFUNCTION__, prpEncConfig.encSize.width, prpEncConfig.encSize.height));

    //----------------------------------------------------
    // Configure Preprocessor for encoding
    //----------------------------------------------------
    if (m_pPrp->PrpConfigureEncoding(&prpEncConfig))
    {
        // Camera has been configured.
        if( CAPTURE == iEncPin )        
             m_bCameraEncConfig = TRUE;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Preprocessor encoding configure failed.\r\n"), __WFUNCTION__));
        return FALSE;
    }
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: SensorEncodingConfig
//
// This function Allocates the buffers for the encoding path of the preprocessing module.
//
// Parameters:
//      iEncPin
//          [in] Specifies whether data format for STILL pin or
//          CAPTURE pin should be used to configure the encoding path.
//
//      iBufNum
//          [in] Number of buffers to allocate for Pre-processing
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorEncBufferAlloc(UINT32 iEncPin, UINT32 iBufNum)
{
    ULONG ulBufSize;
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr = &m_pCurrentFormat[iEncPin].VideoInfoHeader;

    //----------------------------------------------------
    // Re-allocate Preprocessor Buffers
    //----------------------------------------------------
    if (!m_pPrp->PrpDeleteEncBuffers())
    {
        return FALSE;
    }

    ulBufSize = abs(CS_DIBSIZE(pCsVideoInfoHdr->bmiHeader)); // by bytes

    if (!m_pPrp->PrpAllocateEncBuffers(iBufNum, ulBufSize))
    {
        ERRORMSG(TRUE,(_T("%s: Prp Buffer allocation failed.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_bCameraEncAllocBuf = TRUE;

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: SensorSetDirectDisplay
//
// This function turns Direct Display mode on or off.
//
// Parameters:
//      bDirectDisplay
//          [in] If TRUE, turn Direct Display mode on.
//               If FALSE, turn Direct Display mode off.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CSensorPdd::SensorSetDirectDisplay(BOOL bDirectDisplay)
{
    CAM_FUNCTION_ENTRY();

    // Set Direct Display variable
    m_bDirectDisplay = bDirectDisplay;

    // Assure that we will reconfigure IPU before starting.
    m_bCameraVfConfig = FALSE;

    CAM_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: SensorSetDirectCapture
//
// This function turns Direct Capture mode on or off.
//
// Parameters:
//      bDirectDisplay
//          [in] If TRUE, turn Direct Capture mode on.
//               If FALSE, turn Direct Capture mode off.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CSensorPdd::SensorSetDirectCapture(BOOL bDirectCapture)
{
    CAM_FUNCTION_ENTRY();

    // Set Direct Capture variable
    m_bDirectCapture = bDirectCapture;

    // Assure that we will reconfigure IPU before starting.
    m_bCameraEncConfig = FALSE;

    CAM_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: SensorClosePin
//
// This function de-allocates buffers for a given pin.
//
// Parameters:
//      iPin
//          [in] Specifies pin (STILL, CAPTURE, or PREVIEW) for
//          which we can de-allocate buffers.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorClosePin(UINT32 iPin)
{
    DEBUGMSG(ZONE_IOCTL, (_T("+SensorClosePin\r\n")));

    // Call down to Prp to deallocate buffer list
    switch (iPin)
    {
        case PREVIEW:
            m_pPrp->PrpStopVfChannel();

            #ifndef RINGO_FIX_MEM_ALLOC_ISSUE
            if (!m_pPrp->PrpDeleteVfBuffers())
            {
                return FALSE;
            }
            #endif

            break;
        case CAPTURE:
            m_pPrp->PrpStopEncChannel();

            #ifndef RINGO_FIX_MEM_ALLOC_ISSUE
            if (!m_pPrp->PrpDeleteEncBuffers())
            {
                return FALSE;
            }           
            #endif

            break;
        case STILL:            
            // No need to delete buffers in the Prp, as those buffers
            // will be deleted by the Capture Pin.
            break;
        default:
            DEBUGMSG(ZONE_ERROR,(_T("SensorClosePin: Invalid pin id.\r\n"))) ;
            return FALSE;
    }
   
    DEBUGMSG(ZONE_IOCTL, (_T("-SensorClosePin\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SensorZoom
//
// This function sets the camera zoom value.
//
// Parameters:
//      zoomVal
//          [in] zoom value.  
//                If 2, zoom by 2x.  If 1, zoom by 1x.  
//                All other values are invalid.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CSensorPdd::SensorZoom(DWORD zoomVal)
{
    CAM_FUNCTION_ENTRY();

    m_pCsi->CsiZoom(zoomVal);

    CAM_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: SensorMarkAsModified
//
// This function informs the camera driver that important
// configuration paramaters have been modified.  This ensures
// that the camera will reconfigure the preprocessing before
// starting again.
//
// Parameters:
//      ulPinId  module id.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CSensorPdd::SensorMarkAsModified(ULONG ulPinId)
{
    CAM_FUNCTION_ENTRY();

    if (ulPinId == PREVIEW)
    {
        // Assure that we will reconfigure IPU before starting.
        m_bCameraVfConfig = FALSE;        
        m_bCameraVfAllocBuf = FALSE;
    }
    else if (ulPinId == CAPTURE)
    {
        // Assure that we will reconfigure IPU before starting.
        m_bCameraEncConfig = FALSE;
        m_bCameraEncAllocBuf = FALSE;
    }

    CAM_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: PreStillPicture
//
// Currently, STILL and CAPTURE pin use the same channel. If the capture pin is 
// STOP status when we try to capture a still picture, we should enable the channel
// first.
//
// Parameters:
//      ulPinId  module id.
//
// Returns:
//      TRUE for SUCCESS,FALSE for falure.
//
//-----------------------------------------------------------------------------
DWORD CSensorPdd::PreStillPicture(ULONG ulPinId)
{
    CAM_FUNCTION_ENTRY();

    if( STILL != ulPinId )
    {
         DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): The pin id for PreStillPicture is error!\r\n"), this));
         return FALSE;         
    }

    // Stop the channel before reconfig STIL pin    
    if( !m_pPrp->PrpStopEncChannel() )
    {
         DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Stop encoding channel for still capture failed!\r\n"), this));
         return FALSE;            
    }
    
    // Reconfigure for STILL image capture
    if( !SensorEncodingConfig(STILL) )
    {
        DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Config camera for still failed!\r\n"), this));
        return FALSE;
    }

    #ifndef RINGO_FIX_MEM_ALLOC_ISSUE
    // Realloc buffer for still pin
    if( !SensorEncBufferAlloc(STILL,m_ulNumOfBuffer) )
    {
        DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Allocate buffer for still failed!\r\n"), this));
        return FALSE;
    }
    #endif

    // Start the channel after reconfig STIL pin
    if( !m_pPrp->PrpStartEncChannel() )
     {
         DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Start encoding channel for still capture failed!\r\n"), this));
         return FALSE;
    }
   
    CAM_FUNCTION_EXIT();
    return TRUE;    
}


//-----------------------------------------------------------------------------
//
// Function: PostStillPicture
//
// Currently, STILL and CAPTURE pin use the same channel. After we capture a still
// picture, we should stop the channel. the following procedure will revert CAPTURE
// pin to previous status.
//
// Parameters:
//      ulPinId  module id.
//
// Returns:
//      TRUE for SUCCESS,FALSE for falure.
//
//-----------------------------------------------------------------------------
DWORD CSensorPdd::PostStillPicture(ULONG ulPinId)
{
    CAM_FUNCTION_ENTRY();

    if( STILL != ulPinId )
    {
         DEBUGMSG(ZONE_ERROR, (_T("PostStillPicture(%08x): The pin id for poststillpicture is error!\r\n"), this));
         return FALSE;         
    }
    
    // Stop the channel before reconfig CAPTURE pin
    if( !m_pPrp->PrpStopEncChannel() )
    {
         DEBUGMSG(ZONE_ERROR, (_T("PostStillPicture(%08x): Stop encoding channel for still capture failed!\r\n"), this));
         return FALSE;         
    }

    // Reconfigure for CAPTURE
    if( !SensorEncodingConfig(CAPTURE) )
    {
         DEBUGMSG(ZONE_ERROR, (_T("PostStillPicture(%08x): Config camera for capture failed!\r\n"), this));
         return FALSE;
    }

    #ifndef RINGO_FIX_MEM_ALLOC_ISSUE
    // Realloc buffers for capture pin
    if( !SensorEncBufferAlloc(CAPTURE, m_ulNumOfBuffer) )
    {
         DEBUGMSG(ZONE_ERROR, (_T("PostStillPicture(%08x): Allocate buffers for capture failed!\r\n"), this));
         return FALSE;
    }
    #endif

    CAM_FUNCTION_EXIT();
    return TRUE;    
}
