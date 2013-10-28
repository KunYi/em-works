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
//
//  Copyright (C) 2004-2006,2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>
#include <pm.h>

#include "Cs.h"
#include "Csmedia.h"

#include "mxarm11.h"
#pragma warning(pop)
#include <Devload.h>
#include <NKIntr.h>
#include <ceddk.h>
#include "ipu.h"
#include "PrpClass.h"
#include "CsiClass.h"

#include "CameraPDDProps.h"
#include "dstruct.h"
#include "cameradbg.h"
#include "camera.h"
#include "CameraDriver.h"
#include "SensorFormats.h"
#include "SensorProperties.h"

#if (UNDER_CE > 600)
#include "CameraPinDriver.h"
#else
#include "PinDriver.h"
#endif

#include "CameraPDD.h"
#include "SensorPdd.h"
#include "wchar.h"

#pragma warning(disable: 4245)
#pragma warning(disable: 4100)
#pragma warning(disable: 4189)
#pragma warning(disable: 4127)
// External Functions
extern void BSPGetSensorFormat(DWORD *);
extern void BSPGetSensorResolution(DWORD *);

#if (UNDER_CE > 600)
PDDFUNCTBL2 FuncTbl = {
    sizeof(PDDFUNCTBL2),
#else
PDDFUNCTBL FuncTbl = {
    sizeof(PDDFUNCTBL),
#endif
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
#if (UNDER_CE > 600)
    PDD_Open,
    PDD_Close,
    PDD_GetMetadata
#endif
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
    m_hTimerThread = NULL;
    m_bTimerThreadCreated = FALSE;
    m_bTimerThreadRunning = FALSE;

    m_pCsi = NULL;
    m_pPrp = NULL;
    m_bCameraEncConfig = FALSE;
    m_bCameraVfConfig = FALSE;

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
    memset( &m_hTimerEvents, 0x0, sizeof(m_hTimerEvents));
    memset( &m_CsState, 0x0, sizeof(m_CsState));
    memset( &m_SensorModeInfo, 0x0, sizeof(m_SensorModeInfo));
    memset( &m_SensorProps, 0x0, sizeof(m_SensorProps));
    memset( &PowerCaps, 0x0, sizeof(PowerCaps));

#if (UNDER_CE > 600)
    m_pMetadata = NULL;
#endif
}

CSensorPdd::~CSensorPdd()
{
    for( int i=0; i < NUM_SUPPORTED_PINS; i++ )
    {
       KillTimer(i);
    }

    //Terminate the Timer Thread
    SetEvent( m_hTimerEvents[THREAD_TERMINATE_EVENT] );

    //Wait 5 secs. until the Timer Event thread processes the terminate event and gracefully exits
    if((WaitForSingleObject(m_hTimerThread,5000)) == WAIT_TIMEOUT)
    {
      DEBUGMSG(ZONE_ERROR,  (TEXT("%s: Timer thread Terminate Failed! \r\n"), __WFUNCTION__));
      ASSERT(FALSE);
    }
    CloseHandle(m_hTimerThread);
    m_hTimerThread = NULL;
    m_bTimerThreadCreated = FALSE;
    m_bTimerThreadRunning = FALSE;

    //Create Timer related events
    for(int i=0;i<MAX_TIMER_EVENTS;i++)
    {
        CloseHandle(m_hTimerEvents[i]);
        m_hTimerEvents[i] = NULL;
    }

    if ( NULL != m_hTimerDll )
    {
        FreeLibrary( m_hTimerDll );
        m_hTimerDll = NULL;
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

DWORD CSensorPdd::PDDInit( PVOID MDDContext, PPDDFUNCTBL pPDDFuncTbl )
{
    m_hContext = (HANDLE)MDDContext;
    // Real drivers may want to create their context

    m_ulCTypes = NUM_SUPPORTED_PINS;

    // Read registry to override the default number of Sensor Modes.
    ReadMemoryModelFromRegistry();

#if (UNDER_CE > 600)

    if (pPDDFuncTbl->dwSize  < sizeof(PDDFUNCTBL))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    else if (pPDDFuncTbl->dwSize  < sizeof(PDDFUNCTBL2))
    {
        memcpy( pPDDFuncTbl, &FuncTbl, sizeof(PDDFUNCTBL));
        pPDDFuncTbl->dwSize = sizeof(PDDFUNCTBL);
    }
    else if (pPDDFuncTbl->dwSize  >= sizeof( PDDFUNCTBL2 ))
    {
        memcpy( pPDDFuncTbl, &FuncTbl, sizeof( PDDFUNCTBL2 ) );
    }
#else
    if( pPDDFuncTbl->dwSize  < sizeof( PDDFUNCTBL ) )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    memcpy( pPDDFuncTbl, &FuncTbl, sizeof( PDDFUNCTBL ) );
#endif


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
    m_pModeVideoFormat[CAPTURE].ulAvailFormats       = 10;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[CAPTURE].ulAvailFormats];

    if( NULL == m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

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

    m_pModeVideoFormat[STILL].categoryGUID           = PINNAME_VIDEO_STILL;
    m_pModeVideoFormat[STILL].ulAvailFormats         = 5;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[STILL].ulAvailFormats];

    if( NULL == m_pModeVideoFormat[STILL].pCsDataRangeVideo )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[0]   = &DCAM_StreamMode_RGB565_QQVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[1]   = &DCAM_StreamMode_RGB565_QCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[2]   = &DCAM_StreamMode_RGB565_QVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[3]   = &DCAM_StreamMode_RGB565_CIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[4]   = &DCAM_StreamMode_RGB565_VGA;

    if( MAX_SUPPORTED_PINS == m_ulCTypes )
    {
        m_pModeVideoFormat[PREVIEW].categoryGUID         = PINNAME_VIDEO_PREVIEW;
        m_pModeVideoFormat[PREVIEW].ulAvailFormats       = 5;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[PREVIEW].ulAvailFormats];

        if( NULL == m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo )
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[0] = &DCAM_StreamMode_RGB565_QQVGA;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[1] = &DCAM_StreamMode_RGB565_QCIF;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[2] = &DCAM_StreamMode_RGB565_QVGA;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[3] = &DCAM_StreamMode_RGB565_CIF;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[4] = &DCAM_StreamMode_RGB565_VGA;
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

    // Timer specific variables
    m_hTimerDll                 = NULL;
    m_pfnTimeSetEvent           = NULL;
    m_pfnTimeKillEvent          = NULL;
    memset( &m_TimerIdentifier, 0, NUM_SUPPORTED_PINS * sizeof(MMRESULT));

    //Create Timer related events
    for(int i=0;i<MAX_TIMER_EVENTS;i++)
    {
        m_hTimerEvents[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
    }

    m_SensorModeInfo[CAPTURE].MemoryModel = CLIENT_MEMORY_MODEL;
    m_SensorModeInfo[CAPTURE].MaxNumOfBuffers = MAX_NO_OF_BUFFERS;
    m_SensorModeInfo[CAPTURE].PossibleCount = 1;

    m_SensorModeInfo[STILL].MemoryModel = CLIENT_MEMORY_MODEL;
    m_SensorModeInfo[STILL].MaxNumOfBuffers = MAX_NO_OF_BUFFERS;
    m_SensorModeInfo[STILL].PossibleCount = 1;
    if( MAX_SUPPORTED_PINS == m_ulCTypes )
    {
        m_SensorModeInfo[PREVIEW].MemoryModel =    CLIENT_MEMORY_MODEL;
        m_SensorModeInfo[PREVIEW].MaxNumOfBuffers = MAX_NO_OF_BUFFERS;
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

#if(UNDER_CE > 600)
    m_pMetadata = (PCS_PROPERTYITEM)Metadata;
#endif

    m_pCurrentFormat = new CS_DATARANGE_VIDEO[m_ulCTypes];
    if( NULL == m_pCurrentFormat )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

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

    // As part of initialization, turn power on (set to D0).
    // SetCameraPower(D0);

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
   CAM_FUNCTION_ENTRY();

   switch (PowerState)
   {
    case D0:
    case D1:
    case D2:
        {
         if( D0 != PowerCaps.DeviceDx )
         {
                RETAILMSG (0, (TEXT("%s: Set D0.\r\n"), __WFUNCTION__));
                // For power management purposes, enable CSI and PRP
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
                RETAILMSG (0, (TEXT("%s: Set D4.\r\n"), __WFUNCTION__));
                // For power management purposes, disable CSI and PRP
                // when we are stop the channels.
                m_pPrp->PrpDisable();
                m_pCsi->CsiDisable(IC_CHANNEL_ENC);
                m_pCsi->CsiDisable(IC_CHANNEL_VF);

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

    if( !m_SensorConfigured )
    {
        SensorConfig();
        m_SensorConfigured = TRUE;
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

    if( count == m_ulCTypes )
    {  // If all pins are closed.
        //RETAILMSG(1,(TEXT("SetPowerState:D4")));
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

            Sleep(100);//Waiting for Prp finish handling current frame.
            // Kill the timer to conserve resources
            if ( NULL != m_TimerIdentifier[ulModeType] )
            {
                dwError = (KillTimer(ulModeType))? ERROR_SUCCESS : ERROR_INVALID_PARAMETER;
            }

            if( STILL == ulModeType )
            {
                m_bStillCapInProgress = false;
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

            Sleep(100);//Waiting for Prp finish handling current frame.

            if( STILL == ulModeType )
            {
                 if(m_TimerIdentifier[STILL])
                     KillTimer(STILL);
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

                   if( !SensorViewfindingConfig(m_ulNumOfBuffer) )
                   {
                       dwError = ERROR_ACCESS_DENIED;
                   }
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

                     if( !SensorEncodingConfig(ulModeType, m_ulNumOfBuffer) )
                     {
                        dwError = ERROR_ACCESS_DENIED;
                     }
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

            dwError = ( CreateTimer( ulModeType ) ? ERROR_SUCCESS : ERROR_NOT_READY );

            if( PREVIEW == ulModeType )
            {
                if ( !m_pPrp->PrpStartVfChannel() )
                {
                    break;
                }
            }
            else if ( CAPTURE == ulModeType )
            {
                if( !m_pPrp->PrpStartEncChannel() )
                {
                    break;
                }
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
    ULONG ulPinId       = CAPTURE;
    PUINT8 pbySrcBuffer = NULL, pbyDstBuffer = NULL;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PIN_Function(%08x): +FillBuffer\r\n"), this));


#if(UNDER_CE > 600)
        ulPinId = ulModeType;
#else
    if (STILL == ulModeType)
    {
        ulPinId = CAPTURE;
    }
    else
    {
        ulPinId = ulModeType;
    }
#endif

    while (1)
    {
        switch (ulPinId)
        {
            case PREVIEW:

                pbySrcBuffer = (UINT8 *)m_pPrp->PrpGetVfBufFilled();

                if (pbySrcBuffer == NULL)
                {
                    if (WaitForSingleObject(m_pPrp->m_hVfEOFEvent, WAIT_BUFFER_TIMEOUT) != WAIT_OBJECT_0)
                    {
                        ERRORMSG(ZONE_ERROR, (TEXT("+FillBuffer:WaitForSingleObject m_hPrPBuffFilled timeout!\r\n")));
                        //If we can not receive anything from PrP, some times PrP is blocked ,so just restart it.It is not a good way,but it works.
                        m_pPrp->PrpStopVfChannel();
                        Sleep(500);
                        m_pPrp->PrpStartVfChannel();
                        return 0;
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
                        return 0;
                    }
                    pbySrcBuffer = (UINT8 *) m_pPrp->PrpGetEncBufFilled();
                }
                else
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer: ENC filled buffer ready!\r\n")));
                }

                break;

            case STILL:
                DEBUGMSG(ZONE_ERROR,(_T("FillBuffer: Don't forget to Pause Preview or Capture pin before capture a still picture.\r\n"))) ;
#if(UNDER_CE > 600)

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
                        return 0;
                    }
                    pbySrcBuffer = (UINT8 *) m_pPrp->PrpGetEncBufFilled();
                }
                else
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer: ENC filled buffer ready!\r\n")));
                }

                if( m_pModeVideoCaps[STILL].CurrentVideoControlCaps & CS_VideoControlFlag_Sample_Scanned_Notification )
                {
                    CAM_NOTIFICATION_CONTEXT camNotificationCtx;
                    camNotificationCtx.Size = sizeof(CAM_NOTIFICATION_CONTEXT);
                    camNotificationCtx.Data = 0;    // MDD will populate this member
                    camNotificationCtx.Result = ERROR_SUCCESS;

                    if (ERROR_SUCCESS != MDD_HandleNotification( m_hOpenContext, STILL, PDD_NOTIFICATION_SAMPLE_SCANNED, &camNotificationCtx))      
                    {
                        DEBUGMSG(ZONE_ERROR, (_T(" FillBuffer: MDD_HandleNotification returned error!\r\n")));
                        return 0;
                    }
                }
#endif
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
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+FillBuffer:buffer filled success!\r\n")));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-FillBuffer\r\n")));

    // return the size of the image filled
    return(biSizeImage);
}


#if (UNDER_CE > 600)
DWORD CSensorPdd::Open( PVOID MDDOpenContext )
{
    if( MDDOpenContext == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    m_hOpenContext = (HANDLE)MDDOpenContext;
    return ERROR_SUCCESS;
}

DWORD CSensorPdd::Close( PVOID MDDOpenContext )
{
    if( m_hOpenContext != MDDOpenContext )
    {
        return ERROR_INVALID_PARAMETER;
    }

    m_hOpenContext = NULL;
    return ERROR_SUCCESS;
}

DWORD CSensorPdd::GetMetadata(DWORD dwPropId, PUCHAR pOutBuf, DWORD OutBufLen, PDWORD pdwBytesTransferred)
{
    if (CSPROPERTY_METADATA_ALL != dwPropId)
    {
        return ERROR_NOT_SUPPORTED;
    }

    if (NULL == m_pMetadata)
    {
        return ERROR_INTERNAL_ERROR;
    }

    DWORD dwBufSize = sizeof(CS_PROPERTYITEM);
    dwBufSize *= NUMBER_OF_PROPERTYITEMS;
    dwBufSize += sizeof(ULONG);

    for (int i = 0; i < NUMBER_OF_PROPERTYITEMS; ++i)
    {
        dwBufSize += m_pMetadata[i].ulLength;
    }

    if (NULL == pOutBuf)
    {
        if (NULL == pdwBytesTransferred)
        {
            return ERROR_INVALID_PARAMETER;
        }

        *pdwBytesTransferred = dwBufSize;
        return ERROR_SUCCESS;
    }

    if (OutBufLen < dwBufSize)
    {
        return ERROR_INVALID_PARAMETER;
    }

    PCSMETADATA_S pMetadata = (PCSMETADATA_S)pOutBuf;
    pMetadata->ulCount = NUMBER_OF_PROPERTYITEMS;
    PBYTE pCurData = (PBYTE)&pMetadata->rgitemMetadata[NUMBER_OF_PROPERTYITEMS];

    for (int i = 0; i < NUMBER_OF_PROPERTYITEMS; ++i)
    {
        pMetadata->rgitemMetadata[i] = m_pMetadata[i];
        pMetadata->rgitemMetadata[i].ulDataOffset = pCurData - (PBYTE)pMetadata;
        memcpy(pCurData, (PBYTE)m_pMetadata[i].ulDataOffset, m_pMetadata[i].ulLength);
        pCurData += m_pMetadata[i].ulLength;
    }

    if (NULL != pdwBytesTransferred)
    {
        *pdwBytesTransferred = dwBufSize;
    }

    return ERROR_SUCCESS;
}


#endif

BOOL CSensorPdd :: CreateTimer( ULONG ulModeType )
{
   if ( NULL == m_hTimerDll )
   {
        m_hTimerDll        = LoadLibrary( L"MMTimer.dll" );
        m_pfnTimeSetEvent  = (FNTIMESETEVENT)GetProcAddress( m_hTimerDll, L"timeSetEvent" );


        if ( NULL == m_pfnTimeSetEvent )
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

    if(!m_bTimerThreadCreated)
    {
        //check if a previous instance  of Thread handle was closed properly. If not close it now.
        if(m_hTimerThread)
        {
            CloseHandle(m_hTimerThread);
            m_hTimerThread = NULL;
        }
        //Create a Thread Terminate event First. If that fails, drop out and do not create any timer
        DEBUGMSG(ZONE_DEVICE, (_T("IOControl(%08x): Creating Timer thread terminate event.\r\n"), this));

        m_hTimerThread = CreateThread(NULL,0,CSensorPdd::HandleTimerEvents,(LPVOID)this,0,NULL);

        if(!m_hTimerThread)
        {
          return FALSE;
        }
        m_bTimerThreadCreated = TRUE;
    }

    ASSERT( m_pfnTimeSetEvent );

    if ( NULL == m_TimerIdentifier[ulModeType] )
    {
        DEBUGMSG(ZONE_DEVICE, (_T("IOControl(%08x): Creating new timer.\r\n"), this));

        m_TimerIdentifier[ulModeType] = m_pfnTimeSetEvent( (ULONG)m_pCurrentFormat[ulModeType].VideoInfoHeader.AvgTimePerFrame/10000, 10, (LPTIMECALLBACK)m_hTimerEvents[ulModeType], reinterpret_cast<DWORD>(this), TIME_PERIODIC|TIME_CALLBACK_EVENT_SET);

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

BOOL CSensorPdd :: KillTimer( ULONG ulModeType )
{
    if(!m_hTimerDll)
    {
        DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl(%08x): Timer DLL not loaded.\r\n"), this));
        return FALSE;
    }

    if(NULL == m_pfnTimeKillEvent)
    {
        m_pfnTimeKillEvent = (FNTIMEKILLEVENT)GetProcAddress( m_hTimerDll, L"timeKillEvent" );

        if(!m_pfnTimeKillEvent)
        {
                DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl(%08x): GetProcAddress Returned Null.\r\n"), this));
                return FALSE;
        }
    }

    if(NULL == m_TimerIdentifier[ulModeType])
    {
        DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (_T("IOControl(%08x): Error Killing Timer!\r\n"), this));
        return FALSE;
    }

    DEBUGMSG(ZONE_DEVICE, (_T("Killing Timer Event for PIN:0x%x"),ulModeType));

    //Atlast...Kill the timer
    m_pfnTimeKillEvent(m_TimerIdentifier[ulModeType]);
    m_TimerIdentifier[ulModeType] = NULL;

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
    PreStillPicture(STILL);

    MDD_HandleIO( m_ppModeContext[STILL], STILL );

    PostStillPicture(STILL);
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
// Function: SensorConfig
//
// This function configures the camera sensor.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorConfig()
{
    csiSensorOutputFormat sensorOutputFormat;
    csiSensorOutputResolution sensorOutputResolution;
    DWORD sensorFormat, sensorResolution;

    //----------------------------------------------------
    // Configure camera sensor and CSI
    //----------------------------------------------------
    BSPGetSensorFormat(&sensorFormat);
    sensorOutputFormat = (csiSensorOutputFormat) sensorFormat;

    BSPGetSensorResolution(&sensorResolution);
    sensorOutputResolution = (csiSensorOutputResolution) sensorResolution;
    if (!m_pCsi->CsiConfigureSensor(sensorOutputFormat, sensorOutputResolution))
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
           m_inFrameSize.width = 1600;
           m_inFrameSize.height = 1200;
           break;

        case csiSensorOutputResolution_SXGA:
            m_inFrameSize.width = 1280;
            m_inFrameSize.height = 1024;
            break;

        case csiSensorOutputResolution_XGA:
            m_inFrameSize.width = 1280;
            m_inFrameSize.height = 960;
            break;

        case csiSensorOutputResolution_SVGA:
            m_inFrameSize.width = 800;
            m_inFrameSize.height = 600;
            break;

        case csiSensorOutputResolution_VGA:
            m_inFrameSize.width = 640;
            m_inFrameSize.height = 480;
            break;

        case csiSensorOutputResolution_QVGA:
            m_inFrameSize.width = 320;
            m_inFrameSize.height = 240;
            break;

        case csiSensorOutputResolution_CIF:
            m_inFrameSize.width = 352;
            m_inFrameSize.height = 288;
            break;

        case csiSensorOutputResolution_QCIF:
            m_inFrameSize.width = 176;
            m_inFrameSize.height = 144;
            break;

        case csiSensorOutputResolution_QQVGA:
            m_inFrameSize.width = 160;
            m_inFrameSize.height = 120;
            break;

        default:
            break;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: ViewfindingConfig
//
// This function configures the camera sensor and preprocessing module.
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
BOOL CSensorPdd::SensorViewfindingConfig(UINT32 iBufNum)
{
    prpVfConfigData prpVfConfig;
    DWORD Format;
    ULONG ulBufSize;
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
        DEBUGMSG(ZONE_ERROR,(_T("%s: Prp Buffer allocation failed.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: EncodingConfig
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
BOOL CSensorPdd::SensorEncodingConfig(UINT32 iEncPin, UINT32 iBufNum)
{
    prpEncConfigData prpEncConfig;
    DWORD Format;
    WORD bitDepth;
    ULONG ulBufSize;
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr ;

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
            if (prpEncConfig.inputFormat == prpInputFormat_UYVY422)//prpInputFormat_YUV444IL)
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
        DEBUGMSG(ZONE_ERROR,(_T("%s: Prp Buffer allocation failed.\r\n"), __WFUNCTION__));
        return FALSE;
    }

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
            if (!m_pPrp->PrpDeleteVfBuffers())
            {
                return FALSE;
            }
            break;
        case CAPTURE:
            m_pPrp->PrpStopEncChannel();
            if (!m_pPrp->PrpDeleteEncBuffers())
            {
                return FALSE;
            }
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
//      None.
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
    }
    else if (ulPinId == CAPTURE)
    {
        // Assure that we will reconfigure IPU before starting.
        m_bCameraEncConfig = FALSE;
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
         DEBUGMSG(ZONE_ERROR, (_T("PostStillPicture(%08x): The pin id for PreStillPicture is error!\r\n"), this));
         return FALSE;
    }

    // Stop the channel before reconfig STIL pin
    if( !m_pPrp->PrpStopEncChannel() )
    {
         DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Stop encoding channel for still capture failed!\r\n"), this));
         return FALSE;
    }

    // Reconfigure for STILL image capture
    if( !SensorEncodingConfig(STILL, 1) )
    {
        DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Config camera for still failed!\r\n"), this));
        return FALSE;
    }

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
    if( !SensorEncodingConfig(CAPTURE, m_ulNumOfBuffer) )
    {
         DEBUGMSG(ZONE_ERROR, (_T("HandlePinIO(%08x): Config camera for capture failed!\r\n"), this));
         return FALSE;
    }

    CAM_FUNCTION_EXIT();
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: HandleTimerEvents
//
// This is the ThreadProc that handles MMTimer events.
// MMTimer sets an event whenever the timer expires after the set interval
// This handles timer events for different PINs and stop when no more timer is
// active
//
// Parameters:
//      LPVOID lpvParam
//
// Returns:
//      ERROR_SUCCESS or appropriate failure code.
//
//-----------------------------------------------------------------------------
DWORD WINAPI
CSensorPdd::HandleTimerEvents(LPVOID lpvParam)
{
    DWORD retval = ERROR_SUCCESS;
    BOOL bContinue = TRUE;
    CSensorPdd* pSensorPdd = (CSensorPdd*)lpvParam;
    DWORD event_return;
    BOOL bStill = FALSE, bPreview = FALSE;

    pSensorPdd->m_bTimerThreadRunning = TRUE;

    SetThreadPriority(GetCurrentThread(), TIMER_THREAD_PRIORITY);

    while(bContinue)
    {
        event_return = WaitForMultipleObjects(MAX_TIMER_EVENTS,pSensorPdd->m_hTimerEvents,FALSE,INFINITE);

        bStill = FALSE;
        bPreview = FALSE;

        /*The following piece of if block adresses a specific scenario, wherein both the PREVIEW and CAPTURE
          streams are running. The WaitForMultipleObjects reports only the first event in the event array that
          is signalled. if both PREVIEW and CAPTURE events are set, then we have to service them both, else 
          we cannot preview when capture is running. This is possible only if we walk through the event array 
          and check if each of the event is signalled. We are addressing a specifc combination of PREVIEW and CAPTURE being on. 
          The other combinations of PREVIEW+STILL and CAPTURE + STILL are trivial and wouldnt cause visual jitters.
        */
        if((pSensorPdd->m_TimerIdentifier[PREVIEW] != NULL) && (event_return == WAIT_OBJECT_0 + CAPTURE))
        {
            for (int i = event_return + 1; i < MAX_TIMER_EVENTS; i++)
            {
                if (WaitForSingleObject(pSensorPdd->m_hTimerEvents[i], 0) == WAIT_OBJECT_0)
                {
                   if(i == STILL)
                   {
                     bStill = TRUE;
                   }
                   else if(i == PREVIEW)
                   {
                     bPreview = TRUE;
                   }
                   else
                    break;
                }
            }
       }

        switch(event_return)
        {

            case WAIT_OBJECT_0 + CAPTURE:
                {
                    if(pSensorPdd->m_TimerIdentifier[CAPTURE])
                    {
                         pSensorPdd->CaptureTimerCallBack(pSensorPdd->m_TimerIdentifier[CAPTURE],0,(DWORD_PTR)pSensorPdd,NULL,NULL);
                    }

                    //handle a specific case when CAPTURE and PREVIEW are both set
                    if ((bPreview == TRUE) && (pSensorPdd->m_TimerIdentifier[PREVIEW]))
                    {
                         pSensorPdd->CaptureTimerCallBack(pSensorPdd->m_TimerIdentifier[PREVIEW],0,(DWORD_PTR)pSensorPdd,NULL,NULL);
                    }

                    //If STILL is also set
                    if((bStill == TRUE) && (pSensorPdd->m_TimerIdentifier[STILL]))
                    {
                        pSensorPdd->StillTimerCallBack(pSensorPdd->m_TimerIdentifier[STILL],0,(DWORD_PTR)pSensorPdd,NULL,NULL);
                    }                   
                 break;
                }

            case WAIT_OBJECT_0 + STILL:
                {
                    if(pSensorPdd->m_TimerIdentifier[STILL])
                    {
                        pSensorPdd->StillTimerCallBack(pSensorPdd->m_TimerIdentifier[STILL],0,(DWORD_PTR)pSensorPdd,NULL,NULL);
                    }
                 break;
                }


            case WAIT_OBJECT_0 + PREVIEW:
                {
                    if(pSensorPdd->m_TimerIdentifier[PREVIEW])
                    {
                        pSensorPdd->CaptureTimerCallBack(pSensorPdd->m_TimerIdentifier[PREVIEW],0,(DWORD_PTR)pSensorPdd,NULL,NULL);
                    }
                 break;
                }

            case WAIT_OBJECT_0 + THREAD_TERMINATE_EVENT:
                retval =  ERROR_SUCCESS;
                bContinue = FALSE;
                break;
            default:
                retval =  ERROR_TIMEOUT;
                pSensorPdd->m_bTimerThreadCreated = FALSE;
                pSensorPdd->m_bTimerThreadRunning = FALSE;
                bContinue = FALSE;
                break;
        }
    }
    return retval;
}

