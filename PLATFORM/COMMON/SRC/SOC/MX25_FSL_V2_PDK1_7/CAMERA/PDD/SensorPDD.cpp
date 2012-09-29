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

#pragma warning(disable: 4100 4115 4127 4189 4201 4204 4214 4245 4702 4706 6244)

#include <windows.h>
#include <pm.h>
#include <Devload.h>
#include <NKIntr.h>
#include <ceddk.h>

#include "Cs.h"
#include "Csmedia.h"
#include "CameraPDDProps.h"
#include "dstruct.h"
#include "CamBufferManager.h"
#include "CsiClass.h"

//#define  CAMINTERFACE
#include "cameradbg.h"
#include "camera.h"
#include "CameraDriver.h"
#include "SensorFormats.h"
#include "SensorProperties.h"
#include "PinDriver.h"

#include "CameraPDD.h"
#include "SensorPDD.h"
#include "wchar.h"

extern void BSPSensorFlip(BOOL doFlip);

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
    PDD_EnqueueBuffer,
    PDD_FillBuffer,
    PDD_FillBufferEx,
    PDD_HandleModeCustomProperties,
    PDD_PreStillPicture,
    PDD_PostStillPicture
};

CSensorPdd::CSensorPdd( )
{    
    m_hContext = NULL;
    m_ulCTypes = 0;
    m_pCsi = NULL;
    m_pModeVideoFormat = NULL;
    m_pModeVideoCaps = NULL;
    m_pfnTimeSetEvent = NULL;
    m_pfnTimeKillEvent = NULL;
    m_hTimerDll = NULL;
    m_ppModeContext = NULL;
    m_pCurrentFormat = NULL;
    m_SensorConfigured = FALSE;
    m_bStillCapInProgress = FALSE;
    m_bCameraEncConfig = FALSE;
    m_bAllocateBufferForDMA = FALSE;
    m_ulNumOfBuffer = NUM_PIN_BUFFER; 
    m_bSensorChannelStarted = FALSE;
    m_dwSensorFramerate = 15;

    memset( &m_CsState, 0x0, sizeof(m_CsState));
    memset( &m_TimerIdentifier, 0x0, sizeof(m_TimerIdentifier));    
    memset( &m_SensorModeInfo, 0x0, sizeof(m_SensorModeInfo));
    memset( &m_SensorProps, 0x0, sizeof(m_SensorProps));
    memset( &PowerCaps, 0x0, sizeof(PowerCaps));

    m_hPinEncEvent[0] = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;// for Enc begin
    if(NULL == m_hPinEncEvent[0])
    {
        ERRORMSG(TRUE,
            (TEXT("%s: CreateEvent failed for PinEnc begin\r\n"), __WFUNCTION__));
    }

    m_hPinEncEvent[1] = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;// for Enc stop
    if(NULL == m_hPinEncEvent[1])
    {
        ERRORMSG(TRUE,
            (TEXT("%s: CreateEvent failed for PinEnc stop\r\n"), __WFUNCTION__));
    }

    m_hPinVfEvent[0] = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;// for Vf begin
    if(NULL == m_hPinVfEvent[0])
    {
        ERRORMSG(TRUE,
            (TEXT("%s: CreateEvent failed for PinVf begin\r\n"), __WFUNCTION__));
    }

    m_hPinVfEvent[1] = CreateEvent ( NULL,TRUE,FALSE,NULL ) ;// for Vf stop
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

CSensorPdd::~CSensorPdd( )
{
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
        // Set CaptureThread end
        SetEvent ( m_hPinEncEvent[1] ); 

        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hPinEncThread,INFINITE))
            DEBUGMSG(ZONE_FUNCTION,(TEXT("Wait for PinEncWorkerThread end")));

        CloseHandle(m_hPinEncThread);
        m_hPinEncThread = NULL;    
    }

    if (m_hPinVfThread)
    {    
        // Set PreviewThread end
        SetEvent ( m_hPinVfEvent[1] ); 

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
        delete m_pModeVideoCaps;
        m_pModeVideoCaps = NULL;
    }

    if( NULL != m_pModeVideoFormat )
    {
        for(DWORD i=0; i < m_ulCTypes; i++)
        {
            if(NULL != m_pModeVideoFormat[i].pCsDataRangeVideo)
            {
                delete m_pModeVideoFormat[i].pCsDataRangeVideo;
            }
        }

        delete m_pModeVideoFormat;
        m_pModeVideoFormat = NULL;
    }

    if( NULL != m_ppModeContext )
    {
        delete [] m_ppModeContext;
        m_ppModeContext = NULL;
    }

    if( NULL != m_pCurrentFormat )
    {
        delete [] m_pCurrentFormat;
        m_pCurrentFormat = NULL;
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
    DWORD dwRet;

    while(TRUE)
    {
        if ( WaitForSingleObject ( m_hPinEncEvent[1], 0 ) != WAIT_TIMEOUT )
        {
            ExitThread(1);
        }

        if( WaitForSingleObject ( m_hPinEncEvent[0], timeout ) == WAIT_OBJECT_0 )
        {
            dwRet = WaitForSingleObject(m_pCsi->m_hCSIEOFEvent, 100);
            if(dwRet != WAIT_TIMEOUT)
            {
                if( m_CsState[CAPTURE] == CSSTATE_RUN )
                {
                    DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::PinEncWorkerLoop call handlePINIO\r\n")));
                    MDD_HandleIO( m_ppModeContext[CAPTURE], CAPTURE );
                }
            }
        }
        else
        {
            ERRORMSG(TRUE, (TEXT("CSensorPdd::PinEncWorkerLoop Time out\r\n")));
        }
    }

    return;
}

//------------------------------------------------------------------------------
//
// Function: PinVfWorkerThread
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
void CSensorPdd::PinVfWorkerThread(LPVOID lpParameter)
{
    CSensorPdd *pDD = (CSensorPdd *)lpParameter;

    pDD->PinVfWorkerLoop(INFINITE);
}

//------------------------------------------------------------------------------
//
// Function: PinVfWorkerLoop
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
    DWORD dwRet;

    while(TRUE)
    {
        if ( WaitForSingleObject ( m_hPinVfEvent[1], 0 ) != WAIT_TIMEOUT )
        {
            ExitThread(1);
        }

        if( WaitForSingleObject ( m_hPinVfEvent[0], timeout ) == WAIT_OBJECT_0 )
        {
            dwRet = WaitForSingleObject(m_pCsi->m_hCSIEOFEvent, 100);
            if(dwRet != WAIT_TIMEOUT)
            {
                if( m_CsState[PREVIEW] == CSSTATE_RUN )
                {
                    DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::PinVfWorkerLoop call handlePINIO\r\n")));
                    MDD_HandleIO( m_ppModeContext[PREVIEW], PREVIEW );
                }
            }
        }
        else
        {
            ERRORMSG(TRUE, (TEXT("CSensorPdd::PinVfWorkerLoop Time out\r\n")));
        }
    }

    return;
}


DWORD CSensorPdd::PDDInit( PVOID MDDContext, PPDDFUNCTBL pPDDFuncTbl )
{
    // Create (and initialize) CSI class object
    if(m_pCsi == NULL)
    {
        m_pCsi = new CsiClass;
    }

    m_hContext = (HANDLE)MDDContext;

    m_ulCTypes = NUM_SUPPORTED_PINS ; 

    // Read registry to override the default number of Sensor Modes.
    ReadMemoryModelFromRegistry(); 

    if( pPDDFuncTbl->dwSize  > sizeof( PDDFUNCTBL ) )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    // Copy the PDD functions table
    memcpy( pPDDFuncTbl, &FuncTbl, sizeof( PDDFUNCTBL ) );

    // Reset the Sensor properties structuie
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
    m_pModeVideoFormat[CAPTURE].ulAvailFormats       = 36;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[CAPTURE].ulAvailFormats];

    if( NULL == m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[0]  = &DCAM_StreamMode_RGB565_QQCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[1]  = &DCAM_StreamMode_RGB565_QQVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[2]  = &DCAM_StreamMode_RGB565_QCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[3]  = &DCAM_StreamMode_RGB565_QVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[4]  = &DCAM_StreamMode_RGB565_CIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[5]  = &DCAM_StreamMode_RGB565_VGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[6]  = &DCAM_StreamMode_RGB565_SVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[7]  = &DCAM_StreamMode_RGB565_1024_800;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[8]  = &DCAM_StreamMode_RGB565_1280_960;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[9]  = &DCAM_StreamMode_YV12_QQCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[10] = &DCAM_StreamMode_YV12_QQVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[11] = &DCAM_StreamMode_YV12_QCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[12] = &DCAM_StreamMode_YV12_QVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[13] = &DCAM_StreamMode_YV12_CIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[14] = &DCAM_StreamMode_YV12_VGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[15] = &DCAM_StreamMode_YV12_SVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[16] = &DCAM_StreamMode_YV12_1024_800;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[17] = &DCAM_StreamMode_YV12_1280_960;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[18] = &DCAM_StreamMode_UYVY_QQCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[19] = &DCAM_StreamMode_UYVY_QQVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[20] = &DCAM_StreamMode_UYVY_QCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[21] = &DCAM_StreamMode_UYVY_QVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[22] = &DCAM_StreamMode_UYVY_CIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[23] = &DCAM_StreamMode_UYVY_VGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[24] = &DCAM_StreamMode_UYVY_SVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[25] = &DCAM_StreamMode_UYVY_1024_800;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[26] = &DCAM_StreamMode_UYVY_1280_960;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[27] = &DCAM_StreamMode_YUY2_QQCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[28] = &DCAM_StreamMode_YUY2_QQVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[29] = &DCAM_StreamMode_YUY2_QCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[30] = &DCAM_StreamMode_YUY2_QVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[31] = &DCAM_StreamMode_YUY2_CIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[32] = &DCAM_StreamMode_YUY2_VGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[33] = &DCAM_StreamMode_YUY2_SVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[34] = &DCAM_StreamMode_YUY2_1024_800;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[35] = &DCAM_StreamMode_YUY2_1280_960;

    m_pModeVideoFormat[STILL].categoryGUID           = PINNAME_VIDEO_STILL;
    m_pModeVideoFormat[STILL].ulAvailFormats         = 36;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[STILL].ulAvailFormats];

    if( NULL == m_pModeVideoFormat[STILL].pCsDataRangeVideo )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[0]  = &DCAM_StreamMode_RGB565_QQCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[1]  = &DCAM_StreamMode_RGB565_QQVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[2]  = &DCAM_StreamMode_RGB565_QCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[3]  = &DCAM_StreamMode_RGB565_QVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[4]  = &DCAM_StreamMode_RGB565_CIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[5]  = &DCAM_StreamMode_RGB565_VGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[6]  = &DCAM_StreamMode_RGB565_SVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[7]  = &DCAM_StreamMode_RGB565_1024_800;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[8]  = &DCAM_StreamMode_RGB565_1280_960;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[9]  = &DCAM_StreamMode_YV12_QQCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[10] = &DCAM_StreamMode_YV12_QQVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[11] = &DCAM_StreamMode_YV12_QCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[12] = &DCAM_StreamMode_YV12_QVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[13] = &DCAM_StreamMode_YV12_CIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[14] = &DCAM_StreamMode_YV12_VGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[15] = &DCAM_StreamMode_YV12_SVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[16] = &DCAM_StreamMode_YV12_1024_800;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[17] = &DCAM_StreamMode_YV12_1280_960;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[18] = &DCAM_StreamMode_UYVY_QQCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[19] = &DCAM_StreamMode_UYVY_QQVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[20] = &DCAM_StreamMode_UYVY_QCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[21] = &DCAM_StreamMode_UYVY_QVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[22] = &DCAM_StreamMode_UYVY_CIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[23] = &DCAM_StreamMode_UYVY_VGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[24] = &DCAM_StreamMode_UYVY_SVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[25] = &DCAM_StreamMode_UYVY_1024_800;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[26] = &DCAM_StreamMode_UYVY_1280_960;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[27] = &DCAM_StreamMode_YUY2_QQCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[28] = &DCAM_StreamMode_YUY2_QQVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[29] = &DCAM_StreamMode_YUY2_QCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[30] = &DCAM_StreamMode_YUY2_QVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[31] = &DCAM_StreamMode_YUY2_CIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[32] = &DCAM_StreamMode_YUY2_VGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[33] = &DCAM_StreamMode_YUY2_SVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[34] = &DCAM_StreamMode_YUY2_1024_800;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[35] = &DCAM_StreamMode_YUY2_1280_960;


    if( MAX_SUPPORTED_PINS == m_ulCTypes )
    {
        m_pModeVideoFormat[PREVIEW].categoryGUID         = PINNAME_VIDEO_PREVIEW;
        m_pModeVideoFormat[PREVIEW].ulAvailFormats       = 9;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[PREVIEW].ulAvailFormats];

        if( NULL == m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo )
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }

        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[0]  = &DCAM_StreamMode_RGB565_QQCIF;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[1]  = &DCAM_StreamMode_RGB565_QQVGA;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[2]  = &DCAM_StreamMode_RGB565_QCIF;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[3]  = &DCAM_StreamMode_RGB565_QVGA;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[4]  = &DCAM_StreamMode_RGB565_CIF;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[5]  = &DCAM_StreamMode_RGB565_VGA;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[6]  = &DCAM_StreamMode_RGB565_SVGA;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[7]  = &DCAM_StreamMode_RGB565_1024_800;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[8]  = &DCAM_StreamMode_RGB565_1280_960;
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
        m_pModeVideoCaps[PREVIEW].CurrentVideoControlCaps     = DefaultVideoControlCaps[PREVIEW];
    }

    // Timer specific variables. Only to be used in this NULL PDD
    m_hTimerDll                 = NULL;
    m_pfnTimeSetEvent           = NULL;
    m_pfnTimeKillEvent          = NULL;
    memset( &m_TimerIdentifier, 0, NUM_SUPPORTED_PINS * sizeof(MMRESULT));

    m_SensorModeInfo[CAPTURE].MemoryModel = CSPROPERTY_BUFFER_DRIVER;
    m_SensorModeInfo[CAPTURE].MaxNumOfBuffers = 2;
    m_SensorModeInfo[CAPTURE].PossibleCount = 1;

    m_SensorModeInfo[STILL].MemoryModel = CSPROPERTY_BUFFER_DRIVER;
    m_SensorModeInfo[STILL].MaxNumOfBuffers = 2;
    m_SensorModeInfo[STILL].PossibleCount = 1;
    if( MAX_SUPPORTED_PINS == m_ulCTypes )
    {
        m_SensorModeInfo[PREVIEW].MemoryModel = CSPROPERTY_BUFFER_DRIVER;
        m_SensorModeInfo[PREVIEW].MaxNumOfBuffers = 2;
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
    #if 0//Not support
    else if (dwPropId == (CSPROPERTY_CAMERACONTROL_TILT + NUM_VIDEOPROCAMP_ITEMS))
    {
        ERRORMSG(TRUE, (_T("CAM_IOControl(%08x): Rotation is not supported.\r\n"), this));
        return ERROR_INVALID_PARAMETER;
    }
    #endif

    DEBUGMSG(ZONE_FUNCTION, (_T("CAM_IOControl(%08x): HandleCamControlRequests successfully. ulCurrentValue = %d\r\n"), this, pDevProp->ulCurrentValue)) ;

    return ERROR_SUCCESS;
}

DWORD CSensorPdd::HandleVideoControlCapsChanges( LONG lModeType, ULONG ulCaps )
{
    m_pModeVideoCaps[lModeType].CurrentVideoControlCaps = ulCaps;

    #if 0//Not support
    if (ulCaps & CS_VideoControlFlag_FlipHorizontal)
    {
        ERRORMSG(TRUE, (_T("CAM_IOControl(%08x): Horizontal Flipping is not supported.\r\n"), this));
        return ERROR_INVALID_PARAMETER;
    }

    if (ulCaps & CS_VideoControlFlag_FlipVertical)
    {
        ERRORMSG(TRUE, (_T("CAM_IOControl(%08x): Vertical Flipping is not supported.\r\n"), this));
        return ERROR_INVALID_PARAMETER;
    }
    #endif
     
    return ERROR_SUCCESS;
}

DWORD CSensorPdd::SetPowerState( CEDEVICE_POWER_STATE PowerState )
{
   switch (PowerState)
   {
    case D0:
    case D1:
    case D2:
        {
         if( D0 != PowerCaps.DeviceDx )
         {
                DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: Set D0.\r\n"), __WFUNCTION__));

                // Enable CSI controller(Enable MCLK clk)
                m_pCsi->CsiEnable();
                
                // For power management purposes
                if(m_pCsi)
                {
                    m_pCsi->BSPEnableCamera();
                }

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

                // Disable CSI controller(Disable MCLK clk)
                m_pCsi->CsiDisable();

                // Turn sensor clock off
                if(m_pCsi)
                {
                    m_pCsi->BSPDisableCamera();
                }

                // Set the current Power status of the device.
                PowerCaps.DeviceDx = D4;
          }   
        }
        break;
        
     default:
         break;
   }

   return ERROR_SUCCESS;
}
    
DWORD CSensorPdd::HandleAdapterCustomProperties( PUCHAR pInBuf, DWORD  InBufLen, PUCHAR pOutBuf, DWORD  OutBufLen, PDWORD pdwBytesTransferred )
{
    DWORD dwError = ERROR_NOT_SUPPORTED;
    PCSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S pCsPropVideoControlActFrameRateOutput = NULL;
    PCSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S pCsPropVideoControlActFrameRateInput = NULL;
    PCSPROPERTY_VIDEOCONTROL_FRAME_RATES_S pCsPropVideoControlFrameRateOutput = NULL;
    PCSPROPERTY_VIDEOCONTROL_FRAME_RATES_S pCsPropVideoControlFrameRateInput = NULL;
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

        case CSPROPERTY_VIDEOCONTROL_FRAME_RATES:
         {
           if(NULL == (pCsPropVideoControlFrameRateInput = reinterpret_cast<PCSPROPERTY_VIDEOCONTROL_FRAME_RATES_S>(pInBuf)))
                break;

           switch( csProp.Flags )
            {
                case CSPROPERTY_TYPE_SET:
                    *pdwBytesTransferred = sizeof(CSPROPERTY_VIDEOCONTROL_FRAME_RATES_S);
                    if (NULL == (pCsPropVideoControlFrameRateOutput = reinterpret_cast<PCSPROPERTY_VIDEOCONTROL_FRAME_RATES_S>(pOutBuf)))
                    {
                        dwError = ERROR_MORE_DATA;
                        break;
                    }

                    memcpy( pCsPropVideoControlFrameRateOutput, pCsPropVideoControlFrameRateInput, sizeof(CSPROPERTY));

                    pCsPropVideoControlFrameRateOutput->StreamIndex = pCsPropVideoControlFrameRateInput->StreamIndex;
                    pCsPropVideoControlFrameRateOutput->RangeIndex = SetSensorFrameRate(pCsPropVideoControlFrameRateInput->RangeIndex);

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
       DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:m_ppModeContext[%d] = %x , count = %x \r\n"),__WFUNCTION__,
                    i,m_ppModeContext[i],count));
    }

    if( count == m_ulCTypes )
    {  
        SetPowerState(D0);
    }

    ASSERT( ModeContext );
    m_ppModeContext[ulModeType] = ModeContext;
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:m_ppModeContext[%d] = %x , \r\n"),__WFUNCTION__,
                            ulModeType,m_ppModeContext[ulModeType]));

    m_pCsi->CsiConfigureSensor(m_dwSensorFramerate);
    
    return ERROR_SUCCESS;
}
   
DWORD CSensorPdd::DeInitSensorMode( ULONG ulModeType )
{
    DWORD i = 0;
    DWORD count = 0;    

    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:m_ppModeContext[%d] = %x \r\n"),
                __WFUNCTION__,ulModeType,m_ppModeContext[ulModeType]));

#ifndef SENNA_FIX_MEM_ALLOC_ISSUE
    if(m_bAllocateBufferForDMA)
    {
        if (!m_pCsi->CsiStopChannel())
        {
            ERRORMSG(TRUE,(_T("%s: CsiStopChannel failed.\r\n"), __WFUNCTION__));
            return FALSE;
        }

        if (!m_pCsi->CsiDeleteBuffers())
        {
            ERRORMSG(TRUE, (TEXT("%s: Failed to delete CSI buffers\r\n"),__WFUNCTION__));
            return FALSE;
        }
        m_bAllocateBufferForDMA = FALSE;
    }
#endif

    m_ppModeContext[ulModeType] = NULL;

    for( i = 0; i < m_ulCTypes; i ++ )
    {
       if( NULL == m_ppModeContext[i] )
       {
          count ++;
       }
    }

    if( count == m_ulCTypes )
    {  
        // If all pins are closed.
        SetPowerState(D4);

        m_bCameraEncConfig = FALSE;
    }

    return ERROR_SUCCESS;
}

DWORD CSensorPdd::SetSensorState( ULONG lPinId, CSSTATE csState )
{
    DWORD dwError = ERROR_SUCCESS;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("SensorStopChannel - lPinId = %d, csState = %d\r\n"),lPinId,csState));

    switch ( csState )
    {
        case CSSTATE_STOP:

            m_CsState[lPinId] = CSSTATE_STOP;

             // Kill the timer to conserve resources
            if ( NULL != m_TimerIdentifier[lPinId] )            
            {
                ASSERT( m_pfnTimeKillEvent );
                m_pfnTimeKillEvent(m_TimerIdentifier[lPinId]);
                m_TimerIdentifier[lPinId] = NULL;
            }

            if ( STILL == lPinId )
            {
                m_bStillCapInProgress = FALSE;
            }
            else
            {
                if(lPinId == PREVIEW)
                    ResetEvent(m_hPinVfEvent[0]);
                
                if(lPinId == CAPTURE)
                    ResetEvent(m_hPinEncEvent[0]);

                if ((m_CsState[PREVIEW] != CSSTATE_RUN) && (m_CsState[CAPTURE] != CSSTATE_RUN))
                {
                    if ( !SensorStopChannel(lPinId) )
                    {
                        ERRORMSG(1,(TEXT("SensorStopChannel %d error\r\n"),lPinId));
                        break;
                    }
                }
            }

        break;

        case CSSTATE_PAUSE:

            m_CsState[lPinId] = CSSTATE_PAUSE;

            if( STILL == lPinId )
            {
                 dwError = ERROR_SUCCESS;
                 break;
            }
                
            if ((m_CsState[PREVIEW] != CSSTATE_RUN) && (m_CsState[CAPTURE] != CSSTATE_RUN))
            {
                if( !SensorStopChannel(lPinId) )
                {
                    ERRORMSG(1,(TEXT("SensorStopChannel %d error\r\n"),lPinId));
                    dwError = ERROR_ACCESS_DENIED;
                    break;
                }

                if( !m_bCameraEncConfig )
                {
                    // Calculate the CSI parameters
                    if( !SensorEncConfigureSetting(lPinId, FALSE) )
                    {
                       ERRORMSG(TRUE,(TEXT("%s:SensorEncConfigureSetting failed\r\n"),__WFUNCTION__));
                       dwError = ERROR_ACCESS_DENIED;
                       break;
                    }

                    // Configure CSI
                    if (!SensorEncodingConfig(lPinId))
                    {
                        ERRORMSG(TRUE,(TEXT("%s:SensorEncodingConfig failed\r\n"),__WFUNCTION__)); 
                        dwError = ERROR_ACCESS_DENIED;
                        break;
                    }
                }
            }
  
        break;

        case CSSTATE_RUN:

            // Check that PREVIEW and CAPTURE pins do not use different settings
            // for the sensor while running simultaneously
            if  (((CAPTURE == lPinId) && (m_CsState[PREVIEW] == CSSTATE_RUN))  ||
                 ((PREVIEW == lPinId) && (m_CsState[CAPTURE] == CSSTATE_RUN))  )
            {
                if ((m_pCurrentFormat[CAPTURE].VideoInfoHeader.bmiHeader.biSizeImage  != 
                     m_pCurrentFormat[PREVIEW].VideoInfoHeader.bmiHeader.biSizeImage) ||
                    (m_pCurrentFormat[CAPTURE].VideoInfoHeader.bmiHeader.biWidth      != 
                     m_pCurrentFormat[PREVIEW].VideoInfoHeader.bmiHeader.biWidth)       )
                {
                     ERRORMSG(TRUE, (TEXT("%s: Cannot use different sensor settings while running PREVIEW and CAPTURE simultaneously\r\n"), __WFUNCTION__));
                     dwError = ERROR_ACCESS_DENIED;
                     break;
                }

            }

            m_CsState[lPinId] = CSSTATE_RUN;

            if( STILL == lPinId )
            {
                dwError = ERROR_SUCCESS;
                break;
            }

            // Check that the sensor has been configured
            if( !m_bCameraEncConfig )
            {
                ERRORMSG(TRUE,(TEXT("%s:Sensor not configured\r\n"),__WFUNCTION__));
                dwError = ERROR_ACCESS_DENIED;
                break;
            }

            // Enable CSI DMA channel
            if ( !SensorStartChannel(lPinId) )
            {
                ERRORMSG(TRUE,(TEXT("%s:SensorStartChannel failed\r\n"),__WFUNCTION__));
                dwError = ERROR_ACCESS_DENIED;
                break;
            }

            if(lPinId == PREVIEW)
                SetEvent(m_hPinVfEvent[0]);

            if(lPinId == CAPTURE)
                SetEvent(m_hPinEncEvent[0]);

        break;

        default:
            ERRORMSG(TRUE, ( _T("IOControl(%08x): Incorrect State\r\n"), this ) );
            dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}

DWORD CSensorPdd::TakeStillPicture( LPVOID pBurstModeInfo )
{
    DWORD dwError = ERROR_SUCCESS;
    m_bStillCapInProgress = TRUE;

    m_CsState[STILL] = CSSTATE_RUN;

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

DWORD CSensorPdd::GetSensorFrameRate( ULONG ulModeType )
{
    DWORD dwTickCount, dwTickCount2, dwCurTickCount;
    DWORD curframeCount=0;
    DWORD lastframeCount=0;
    DWORD totalframeCount=0;
    DWORD maxframeCount=0;

    totalframeCount = lastframeCount = curframeCount = 0;
    dwTickCount = GetTickCount();

    maxframeCount = -1;

    if (ulModeType != STILL)
    {
        curframeCount = m_pCsi->CsiGetFrameCount();
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

        // get the frame count from the CSI controller
        if (ulModeType == PREVIEW)
        {
            curframeCount = m_pCsi->CsiGetFrameCount();
        }
    }

    return dwTickCount2;
}

DWORD CSensorPdd::SetSensorFrameRate(DWORD dwFramerate)
{
    if (dwFramerate == m_dwSensorFramerate)
    {
        return m_dwSensorFramerate;
    }

    m_dwSensorFramerate = dwFramerate;
    m_pCsi->CsiChangeFrameRate(m_dwSensorFramerate);  

    m_bCameraEncConfig = FALSE;

    return m_dwSensorFramerate;
}


DWORD CSensorPdd::SetSensorModeFormat( ULONG ulModeType, PCS_DATARANGE_VIDEO pCsDataRangeVideo )
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Format : ulModeType = %d, biSizeImage = 0x%08x, width = %d\r\n"),__WFUNCTION__,ulModeType,pCsDataRangeVideo->VideoInfoHeader.bmiHeader.biSizeImage,pCsDataRangeVideo->VideoInfoHeader.bmiHeader.biWidth));

    memcpy( &m_pCurrentFormat[ulModeType], pCsDataRangeVideo, sizeof ( CS_DATARANGE_VIDEO ) );

    // Assure that we will reconfigure CSI before starting.
    SensorMarkAsModified(ulModeType);
   
    return ERROR_SUCCESS;
}

PVOID CSensorPdd::AllocateBuffer( ULONG ulModeType )
{
    ULONG ulFrameSize = abs(CS_DIBSIZE (m_pCurrentFormat[ulModeType].VideoInfoHeader.bmiHeader));

    return RemoteLocalAlloc( LPTR, ulFrameSize );
}

DWORD CSensorPdd::DeAllocateBuffer( ULONG ulModeType, PVOID pBuffer )
{
    RemoteLocalFree( pBuffer );

    return ERROR_SUCCESS;
}

DWORD CSensorPdd::GetPhysAddress( PVOID lpUnMappedBuffer, ULONG ulSize)
{
    // Real PDD may want to save pBuffer which is a pointer to buffer that DShow created.
    // MDD give us a virtual memory address, it need to be marshall, after marshall, the responding
    // phy address can be retrieved.
    BOOL  blRet = FALSE;
    DWORD dwPhy = 0;
    DWORD * pPFNs = NULL;
    
    pPFNs = new DWORD[300];
    MarshalledBuffer_t MarshalledBuffer(lpUnMappedBuffer, ulSize, ARG_O_PTR, FALSE, TRUE);
    LPVOID lpMappedBuffer = MarshalledBuffer.ptr();
    
    blRet = LockPages(lpMappedBuffer, ulSize,
                     pPFNs, LOCKFLAG_WRITE);
    if(!blRet)
    {
        if (NULL != pPFNs)
        {
            delete [] pPFNs;
            pPFNs = NULL;
        }
        ERRORMSG(TRUE, (TEXT("LockPages error! error code = %d\r\n"),GetLastError()));
        return (DWORD)-1;
    }
    
    long shift = UserKInfo[KINX_PFN_SHIFT];
    if (NULL != pPFNs)
        dwPhy = (DWORD)(((DWORD)pPFNs[0]) << shift);
    DWORD dwOffset = (DWORD) lpUnMappedBuffer & (UserKInfo[KINX_PAGESIZE]-1);
    dwPhy = ((DWORD)dwPhy+dwOffset);

    blRet = UnlockPages(lpMappedBuffer, ulSize);
    if(!blRet)
    {
        if (NULL != pPFNs)
        {
            delete [] pPFNs;
            pPFNs = NULL;
        }
        ERRORMSG(TRUE, (TEXT("UnlockPages error! error code = %d\r\n"),GetLastError()));
        return (DWORD)-1;
    }

    if (NULL != pPFNs)
    {
        delete [] pPFNs;
        pPFNs = NULL;
    }
    return dwPhy;
}

DWORD CSensorPdd::RegisterClientBuffer( ULONG ulModeType, PVOID pBuffer )
{
    ULONG ulFrameSize = abs(CS_DIBSIZE (m_pCurrentFormat[ulModeType].VideoInfoHeader.bmiHeader));
    DWORD dwPhyAddr = GetPhysAddress(pBuffer,ulFrameSize);
    DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd RegisterClientBuffer Get PHY address = 0x%x\r\n"),dwPhyAddr));
    
    return (m_pCsi->RegisterBuffer(ulFrameSize, pBuffer, dwPhyAddr)? ERROR_SUCCESS : ERROR_INVALID_PARAMETER) ;
}

DWORD CSensorPdd::UnRegisterClientBuffer( ULONG ulModeType, PVOID pBuffer )
{
    // DShow is not going to use pBuffer (which was originally allocated by DShow) anymore. If the PDD
    // is keeping a cached pBuffer pointer (in RegisterClientBuffer()) then this is the right place to
    // stop using it and maybe set the cached pointer to NULL. 
    // Note: PDD must not delete this pointer as it will be deleted by DShow itself
    ULONG ulFrameSize = abs(CS_DIBSIZE (m_pCurrentFormat[ulModeType].VideoInfoHeader.bmiHeader));
    DWORD dwPhyAddr = GetPhysAddress(pBuffer,ulFrameSize);
    DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd UnRegisterClientBuffer Get PHY address = 0x%x\r\n"),dwPhyAddr));
     
    return (m_pCsi->UnregisterBuffer(pBuffer, dwPhyAddr) ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER);
}

DWORD CSensorPdd::EnqueueBuffer( ULONG ulModeType, PVOID pBuffer )
{
    // DShow is not going to use pBuffer (which was originally allocated by DShow) anymore. If the PDD
    // is keeping a cached pBuffer pointer (in RegisterClientBuffer()) then this is the right place to
    // stop using it and maybe set the cached pointer to NULL. 
    // Note: PDD must not delete this pointer as it will be deleted by DShow itself
    ULONG ulFrameSize = abs(CS_DIBSIZE (m_pCurrentFormat[ulModeType].VideoInfoHeader.bmiHeader));
    DWORD dwPhyAddr = GetPhysAddress(pBuffer,ulFrameSize);

    return (m_pCsi->Enqueue() ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER);
}

DWORD CSensorPdd::FillBuffer( ULONG ulModeType, PUCHAR pImage )
{
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr = &m_pCurrentFormat[ulModeType].VideoInfoHeader;
    UINT biSizeImage    = pCsVideoInfoHdr->bmiHeader.biSizeImage;
    ULONG ulPinId       = CAPTURE;
    PUINT8 pbySrcBuffer = NULL, pbyDstBuffer = NULL;
    UINT biWaitCount    = 0;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("PIN_Function(%08x): +FillBuffer\r\n"), this));

    if (STILL == ulModeType)
    {
        ulPinId = CAPTURE;
    }
    else
    {
        ulPinId = ulModeType;
    }

    while (1)
    {
        switch (ulPinId)
        {
            case PREVIEW:

                pbySrcBuffer = (UINT8 *)m_pCsi->GetBufFilled();

                if (pbySrcBuffer == NULL)
                {
                    if (WaitForSingleObject(m_pCsi->m_hCSIEOFEvent, 2000) != WAIT_OBJECT_0) 
                    {
                        DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer:WaitForSingleObject m_hCSIEOFEvent timeout!\r\n")));
                        return 0;
                    }
                    pbySrcBuffer = (UINT8 *) m_pCsi->GetBufFilled();
                }
                break;
                
            case CAPTURE:

                pbySrcBuffer = (UINT8 *)m_pCsi->GetBufFilled();

                if (pbySrcBuffer == NULL)
                {
                    if (WaitForSingleObject(m_pCsi->m_hCSIEOFEvent, 2000) != WAIT_OBJECT_0) 
                    {
                        DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer:WaitForSingleObject m_hCSIEOFEvent timeout!\r\n")));
                        return 0;
                    }
                    pbySrcBuffer = (UINT8 *) m_pCsi->GetBufFilled();
                }
                break;
            
            case STILL:
                DEBUGMSG(ZONE_ERROR,(_T("FillBuffer: Don't forget to Pause Preview or Capture pin before capturing a still picture.\r\n"))) ;
                return 0;

            default:
                DEBUGMSG(ZONE_ERROR,(_T("FillBuffer: Invalid pin id.\r\n"))) ;
                return 0;
        }

        // If we got a buffer from CSI, exit loop and continue.
        if (pbySrcBuffer != NULL)
        {
            break;
        }

        // If there was no buffer returned, loop again and wait for one.
        biWaitCount++;
        DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer: pbySrcBuffer NULL!  Waiting again...\r\n")));

        if(biWaitCount >= 3)
        {    
            RETAILMSG(1, (TEXT("FillBuffer: Waiting too many times, must something wrong...\r\n")));
            biWaitCount = 0;
            SensorStopChannel(ulModeType);
            Sleep(500);
            SensorStartChannel(ulModeType);
            Sleep(500);
        }
    }

    pbyDstBuffer =  reinterpret_cast<PUINT8>(pImage);

    if(pCsVideoInfoHdr->bmiHeader.biCompression == (FOURCC_YV12|BI_SRCPREROTATE))
    {
        PUINT8 data_offset;
        PUINT8 data_in = pbySrcBuffer;
        PUINT8 data_out = pbyDstBuffer;
        UINT HEIGHT = abs(pCsVideoInfoHdr->bmiHeader.biHeight);
        UINT WIDTH = abs(pCsVideoInfoHdr->bmiHeader.biWidth);
        UINT i=0,j=0,k=0,m=0;

        DEBUGMSG(ZONE_FUNCTION,(TEXT("FillBuffer : HEIGHT = %d WIDTH = %d\r\n"),HEIGHT,WIDTH));
        DEBUGMSG(ZONE_FUNCTION,(TEXT("data_in = 0x%x data_out = 0x%x\r\n"),data_in,data_out));

        for (i=0; i< HEIGHT ; i+=2) 
        {
            data_offset = data_in + i * WIDTH * 3 /2;
        
            for (j=0; j< WIDTH * 2; j+= 4) 
            { 
                *(PUINT8)(data_out + k) = *(PUINT8)(data_offset + j + 1);
                *(PUINT8)(data_out + m + WIDTH * HEIGHT) = *(PUINT8)(data_offset + j + 0);
                *(PUINT8)(data_out + k + 1) = *(PUINT8)(data_offset + j + 3);
                *(PUINT8)(data_out + m + WIDTH * HEIGHT * 5 /4) = *(PUINT8)(data_offset + j + 2);
                k +=2;
                m ++;
            }

            j = 0;
        
            memcpy(data_out + k, data_in + i * WIDTH * 3 / 2 + WIDTH * 2, WIDTH);
            k += WIDTH;
        }   

    }
    else
    {
        memcpy(pbyDstBuffer, pbySrcBuffer, biSizeImage);
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer:buffer filled successfully!\r\n")));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-FillBuffer\r\n")));

    // return the size of the image filled
    return(biSizeImage); 
}

DWORD CSensorPdd::FillBufferEx( ULONG ulModeType, PUCHAR* ppImage )
{
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr = &m_pCurrentFormat[ulModeType].VideoInfoHeader;
    UINT biSizeImage    = pCsVideoInfoHdr->bmiHeader.biSizeImage;
    ULONG ulPinId       = CAPTURE;

    if (STILL == ulModeType)
    {
        ulPinId = PREVIEW;
    }
    else
    {
        ulPinId = ulModeType;
    }

    {
        switch (ulPinId)
        {
            case PREVIEW:
            case CAPTURE:
                *ppImage = (UINT8 *)m_pCsi->GetBufFilled();
                break;            
            case STILL:
                DEBUGMSG(ZONE_ERROR,(_T("FillBuffer: Don't forget to Pause Preview or Capture pin before capture a still picture.\r\n"))) ;
                return 0;
            default:
                DEBUGMSG(ZONE_ERROR,(_T("FillBuffer: Invalid pin id.\r\n"))) ;
                return 0;
        }        
        DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer: pbySrcBuffer NULL!  Waiting again...\r\n")));
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-FillBuffer\r\n")));
    if (*ppImage != NULL)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer for %d:buffer filled success address = 0x%x\r\n"),ulPinId,*ppImage));
        return(biSizeImage);    
    }
    else
    {
        RETAILMSG(1, (TEXT("+FillBufferEx: pbySrcBuffer NULL!  something wrong...\r\n")));
        return 0;
    } 
}

DWORD CSensorPdd::HandleSensorModeCustomProperties( ULONG ulModeType, PUCHAR pInBuf, DWORD  InBufLen, PUCHAR pOutBuf, DWORD  OutBufLen, PDWORD pdwBytesTransferred )
{
    DEBUGMSG( ZONE_IOCTL, ( _T("IOControl Adapter PDD: Unsupported PropertySet Request\r\n")) );
    return ERROR_NOT_SUPPORTED;
}

void CSensorPdd::CaptureTimerCallBack( UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2 )
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

void CSensorPdd::StillTimerCallBack( UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2 )
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

DWORD CSensorPdd::PreStillPicture(ULONG ulPinId)
{
    if (STILL != ulPinId)
    {
         DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): The pin id for PreStillPicture is error!\r\n"), this));
         return FALSE; 
    }

    if( !SensorStopChannel(STILL) )
    {
         DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Stop sensor channel for still capture failed!\r\n"), this));
         return FALSE;
    }

    if( m_CsState[CAPTURE] == CSSTATE_RUN )
        ResetEvent(m_hPinVfEvent[0]);
                
    if( m_CsState[PREVIEW] == CSSTATE_RUN )
        ResetEvent(m_hPinEncEvent[0]);

    // Save current config for proper reconfiguration after the picture is taken
    m_SaveOutputFormat = m_SensorOutputFormat;
    m_SaveOutputResolution = m_SensorOutputResolution;

    // Reconfigure for STILL image capture    
    if (!SensorEncConfigureSetting(STILL,FALSE))
    {
        ERRORMSG(TRUE,(TEXT("%s:SensorEncConfigureSetting fail\r\n"),__WFUNCTION__)); 
        return FALSE;
    }
    if (!SensorEncodingConfig(STILL))
    {
        ERRORMSG(TRUE,(TEXT("%s:SensorEncodingConfig fail\r\n"),__WFUNCTION__)); 
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Sleeping a little bit for the sensor to be stable\r\n"),__WFUNCTION__)); 
    Sleep(1000);

    if( !SensorStartChannel(STILL) )
    {
         DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Start DMA channel for still capture failed!\r\n"), this));
         return FALSE;
    }

    return TRUE;  
}

DWORD CSensorPdd::PostStillPicture(ULONG ulPinId)
{
    if (STILL != ulPinId)
    {
         DEBUGMSG(ZONE_ERROR, (_T("PostStillPicture(%08x): The pin id for poststillpicture is error!\r\n"), this));
         return FALSE; 
    }
    
    if( !SensorStopChannel(STILL) )
    {
         DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Stop sensor channel for still capture failed!\r\n"), this));
         return FALSE;
    }

    // Retrieve saved config after the picture is taken
    m_SensorOutputFormat = m_SaveOutputFormat;
    m_SensorOutputResolution = m_SaveOutputResolution;

    // Reconfigure with saved settings  
    if (!SensorEncConfigureSetting(STILL,FALSE))
    {
        ERRORMSG(TRUE,(TEXT("%s:SensorEncConfigureSetting fail\r\n"),__WFUNCTION__)); 
        return FALSE;
    }
    if (!SensorEncodingConfig(ulPinId))
    {
        ERRORMSG(TRUE,(TEXT("%s:SensorEncodingConfig fail\r\n"),__WFUNCTION__)); 
        return FALSE;
    }

    // Restart channel if CAPTURE or PREVIEW pins are running
    if ((m_CsState[PREVIEW] == CSSTATE_RUN) || (m_CsState[CAPTURE] == CSSTATE_RUN))
    {
        if( !SensorStartChannel(STILL) )
        {
            ERRORMSG(TRUE, (_T("PostStillPicture(%08x): Start DMA channel for still capture failed!\r\n"), this));
            return FALSE;
        }
        
        if( m_CsState[CAPTURE] == CSSTATE_RUN )
            SetEvent(m_hPinVfEvent[0]);
                
        if( m_CsState[PREVIEW] == CSSTATE_RUN )
            SetEvent(m_hPinEncEvent[0]);
    }

    return TRUE;   
}

BOOL CSensorPdd::CreateTimer( ULONG ulModeType )
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
        DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl(%08x): Creating new timer for %x Mode type-0.Cap,1.Stl,2.Prv.\r\n"), this,ulModeType));

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

void CSensorPdd::HandleCaptureInterrupt( UINT uTimerID )
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

void CSensorPdd::HandleStillInterrupt( UINT uTimerID )
{
    MDD_HandleIO( m_ppModeContext[STILL], STILL );
    m_bStillCapInProgress = FALSE;
}

BOOL CSensorPdd::ReadMemoryModelFromRegistry( )
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
    
BOOL CSensorPdd::SensorEncodingConfig(ULONG ulPinId)
{
    ULONG ulFrameSize = abs(CS_DIBSIZE (m_pCurrentFormat[ulPinId].VideoInfoHeader.bmiHeader));

#ifndef SENNA_FIX_MEM_ALLOC_ISSUE
    // Delete CSI buffers if they have already been allocated
    if(m_bAllocateBufferForDMA)
    {
        if (!m_pCsi->CsiStopChannel())
        {
            ERRORMSG(TRUE,(_T("%s: CsiStopChannel failed.\r\n"), __WFUNCTION__));
            return FALSE;
        }

        if (!m_pCsi->CsiDeleteBuffers())
        {
            ERRORMSG(TRUE, (TEXT("%s: Failed to delete CSI buffers\r\n"),__WFUNCTION__));
            return FALSE;
        }
        m_bAllocateBufferForDMA = FALSE;
    }

    // Allocate CSI buffers for current PIN
    if (!m_pCsi->CsiAllocateBuffers(m_ulNumOfBuffer, ulFrameSize))
    {
        ERRORMSG(TRUE, (_T("%s: Camera Buffer allocation failed.\r\n"), __WFUNCTION__));
        return FALSE;
    }
    m_bAllocateBufferForDMA = TRUE;
#endif

    // Configure the CSI controller for current PIN
    if (!m_pCsi->CsiConfigure(m_SensorOutputFormat, m_SensorOutputResolution))
    {
        ERRORMSG(TRUE, 
            (TEXT("%s: Sensor configuration failed.  Aborting remaining configuration steps.\r\n"), __WFUNCTION__));
        return FALSE;
    }
    
    m_bCameraEncConfig = TRUE;
        
    return TRUE;
}

void CSensorPdd::SensorZoom( DWORD zoomVal )
{
    m_pCsi->CsiZoom(zoomVal);
}

void CSensorPdd::SensorMarkAsModified( ULONG ulPinId )
{
    // Assure that we will reconfigure CSI before starting.
    m_bCameraEncConfig = FALSE;
}

BOOL CSensorPdd::SensorStartChannel(UINT32 iPin)
{
    // We only have one channel for all pins, make sure we don't start it twice
    if(m_bSensorChannelStarted) 
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Channel already started - iPin = %d\r\n"),__WFUNCTION__,iPin));
        return TRUE;
    }
 
    if (m_pCsi->CsiStartChannel())
    {
        m_bSensorChannelStarted = TRUE;
    }
    else
    {
        ERRORMSG(TRUE,(TEXT("%s: Failed to start the channel iPin = %d\r\n"),__WFUNCTION__,iPin));
    }

    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Channel Started - iPin = %d\r\n"),__WFUNCTION__,iPin));

    return (TRUE == m_bSensorChannelStarted);
}

BOOL CSensorPdd::SensorStopChannel(UINT32 iPin)
{
    // We only have one channel for all pins, make sure we don't stop it twice
    if(!m_bSensorChannelStarted)
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Channel already stopped - iPin = %d\r\n"),__WFUNCTION__,iPin));
        return TRUE;
    }

    if (!m_pCsi->CsiStopChannel())
    {
        ERRORMSG(TRUE,(_T("%s: CsiStopChannel failed.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    m_bSensorChannelStarted = FALSE;

    return TRUE;
}

BOOL CSensorPdd::SensorEncConfigureSetting(ULONG ulPinId, BOOL bDefault)
{
    DWORD sensorFormat, sensorResolution;
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr = &m_pCurrentFormat[ulPinId].VideoInfoHeader; 
    
    if( !bDefault )
    {  
        // Set CSI output format
        switch (pCsVideoInfoHdr->bmiHeader.biCompression)
        {
            case (FOURCC_YUY2|BI_SRCPREROTATE):
            case (FOURCC_YUYV|BI_SRCPREROTATE):
                m_SensorOutputFormat = csiSensorOutputFormat_YUV422_YUY2;
                
                // PREVIEW does not support YUV, stopping it if running
                if (m_CsState[PREVIEW] == CSSTATE_RUN)
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: Stopping PREVIEW because the sensor will be set to YUV mode.\r\n"), __WFUNCTION__));
                    SetSensorState(PREVIEW, CSSTATE_STOP);
                }
            break;  

            case (FOURCC_UYVY|BI_SRCPREROTATE):
                m_SensorOutputFormat = csiSensorOutputFormat_YUV422_UYVY;
                
                // PREVIEW does not support YUV, stopping it if running
                if (m_CsState[PREVIEW] == CSSTATE_RUN)
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: Stopping PREVIEW because the sensor will be set to YUV mode.\r\n"), __WFUNCTION__));
                    SetSensorState(PREVIEW, CSSTATE_STOP);
                }
            break;  

            case (FOURCC_YV12|BI_SRCPREROTATE):
                m_SensorOutputFormat = csiSensorOutputFormat_YUV420;
                
                // PREVIEW does not support YUV, stopping it if running
                if (m_CsState[PREVIEW] == CSSTATE_RUN)
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: Stopping PREVIEW because the sensor will be set to YUV mode.\r\n"), __WFUNCTION__));
                    SetSensorState(PREVIEW, CSSTATE_STOP);
                }
            break;

            case (CS_BI_BITFIELDS|BI_SRCPREROTATE):
                m_SensorOutputFormat = csiSensorOutputFormat_RGB565;
            break; 

            default:
                 ERRORMSG(TRUE, (TEXT("%s: Invalid data format = 0x%08x.\r\n"), __WFUNCTION__,pCsVideoInfoHdr->bmiHeader.biCompression));
                 return FALSE;
        }

        // If height in bmiHeader is positive, the captured image should be bottom-up, so we vertically flip the image.
        if( abs(pCsVideoInfoHdr->bmiHeader.biHeight) == pCsVideoInfoHdr->bmiHeader.biHeight )
            BSPSensorFlip(TRUE);
        else
            BSPSensorFlip(FALSE);

        // Set CSI output resolution
        switch(pCsVideoInfoHdr->bmiHeader.biWidth)
        {
            case D1_Width:
                if (abs(pCsVideoInfoHdr->bmiHeader.biHeight) == D1_PAL_Height)
                    m_SensorOutputResolution = csiSensorOutputResolution_D1_PAL;
                else
                    m_SensorOutputResolution = csiSensorOutputResolution_D1_NTSC;
                break;
            case VGA_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_VGA;
                break;
            case QVGA_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_QVGA;
                break;
            case CIF_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_CIF;
                break;
            case QCIF_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_QCIF;
                break;
            case QQVGA_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_QQVGA;
                break;
            case QQCIF_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_QQCIF;
                break;
            case SVGA_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_SVGA;
                break;
             case XGA_Width:
                {
                    if (abs(pCsVideoInfoHdr->bmiHeader.biHeight) == SXGA_1024_800_Height)
                        m_SensorOutputResolution = csiSensorOutputResolution_1024_800;
                    else
                        m_SensorOutputResolution = csiSensorOutputResolution_XGA;
                }
                break;
            case SXGA_Width:
                {
                    if (abs(pCsVideoInfoHdr->bmiHeader.biHeight) == SXGA_1280_960_Height)
                        m_SensorOutputResolution = csiSensorOutputResolution_1280_960;
                    else
                        m_SensorOutputResolution = csiSensorOutputResolution_SXGA;
                }
                break;
            case UXGA_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_UXGA;
                break;
            case QXGA_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_QXGA;
                break;

            default:
                ERRORMSG(TRUE, (TEXT("%s: Invalid image size (width = %d).\r\n"), __WFUNCTION__,pCsVideoInfoHdr->bmiHeader.biWidth));
                return FALSE;
        }
    }   
    else
    {
        // Set CSI output format
        m_pCsi->BSPGetSensorFormat(&sensorFormat);
        m_SensorOutputFormat = (csiSensorOutputFormat) sensorFormat;

        // Set CSI output resolution
        m_pCsi->BSPGetSensorResolution(&sensorResolution);
        m_SensorOutputResolution = (csiSensorOutputResolution) sensorResolution;
    }
    
    return TRUE;
}
