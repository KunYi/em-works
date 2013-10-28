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

#include <windows.h>
#include <pm.h>

#include "Cs.h"
#include "Csmedia.h"

#include "CameraPDDProps.h"
#include "dstruct.h"
#include "CameraDebug.h"
#include <camera.h>
#include "CameraDriver.h"
#include "SensorFormats.h"
#include "SensorProperties.h"
#include "CameraPinDriver.h"

#include "csp.h"
#include "CsiClass.h"
#include "Prp.h"
#include "PrpClass.h"

#include "CameraPDD.h"
#include "CamPdddevice.h"
#include "wchar.h"

//#include "CameraProps.h"

#define WAIT_BUFFER_TIMEOUT     10000    //Timeout for receive a buffer from Prp module.

//------------------------------------------------------------------------------
// External Functions
extern void BSPGetSensorFormat(DWORD *);
extern void BSPGetSensorResolution(DWORD *);
extern BOOL BSPCameraIsTVin(void);

PDDFUNCTBL2 FuncTbl = {
    sizeof(PDDFUNCTBL2),
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
    PDD_Open,
    PDD_Close,
    PDD_GetMetadata
};


CamPddDevice::CamPddDevice()
{
    m_ulCTypes = 2;
    m_bStillCapInProgress = false;
    m_hContext = NULL;
    m_pModeVideoFormat = NULL;
    m_pModeVideoCaps = NULL;
    m_ppModeContext = NULL;
    m_pCurrentFormat = NULL;

    memset( &m_CsState, 0x0, sizeof(m_CsState));
    memset( &m_SensorModeInfo, 0x0, sizeof(m_SensorModeInfo));
    memset( &m_SensorProps, 0x0, sizeof(m_SensorProps));
    memset( &PowerCaps, 0x0, sizeof(PowerCaps));

    //LG specific
    m_PowerState = D0;
    m_ulMaxNumOfBuffers = BUFFER_COUNT;

    m_pCsi = NULL;
    m_pPrp = NULL;

    m_inFormat = prpInputFormat_RGB16;
    m_inFrameSize.height = 0;
    m_inFrameSize.width = 0;

    m_bFlipHorizontal = TRUE; // The value reflect default state of the camera
    m_bFlipVertical = FALSE; // The value reflect default state of the camera
    m_pMetadata = NULL;
}

CamPddDevice::~CamPddDevice()
{
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
        if (NULL != m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo)
        {
            delete [] m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo;
            m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo = NULL;
        }

        if (NULL != m_pModeVideoFormat[STILL].pCsDataRangeVideo)
        {
            delete [] m_pModeVideoFormat[STILL].pCsDataRangeVideo;
            m_pModeVideoFormat[STILL].pCsDataRangeVideo = NULL;
        }

        if (NULL != m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo)
        {
            delete [] m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo;
            m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo = NULL;
        }

        delete [] m_pModeVideoFormat;
        m_pModeVideoFormat = NULL;
    }

    if( NULL != m_ppModeContext )
    {
        delete [] m_ppModeContext;
        m_ppModeContext = NULL;
    }

    if ( NULL != m_pPrp )
    {
        if (!VirtualFree((void*)m_pPrp, 0, MEM_RELEASE))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s : m_pPrp Deallocation failed\r\n"), __WFUNCTION__));
        }
        m_pPrp = NULL;
    }

    if ( NULL != m_pCsi )
    {
        if (!VirtualFree((void*)m_pCsi, 0, MEM_RELEASE))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("$s : m_pCsi Deallocation failed\r\n"), __WFUNCTION__));
        }
        m_pCsi = NULL;
    }

    if ( NULL != m_hVfShutDown )
    {
        CloseHandle(m_hVfShutDown);
        m_hVfShutDown = NULL;
    }

    if ( NULL != m_hEncShutDown )
    {
        CloseHandle(m_hEncShutDown);
        m_hEncShutDown = NULL;
    }
}

DWORD CamPddDevice::PDDInit( PVOID MDDContext, PPDDFUNCTBL pPDDFuncTbl )
{
    // Create (and initialize) preprocessor class object
    if( NULL == m_pPrp )
    {
        m_pPrp = new PrpClass;
        if( NULL == m_pPrp )
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }

    // Create (and initialize) CSI class object
    if( NULL == m_pCsi )
    {
        m_pCsi = new CsiClass;
        if( NULL == m_pCsi )
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }

    if (!CameraSensorConfig())
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    m_pCsi->CsiDisable();

    m_PowerState = D4;
    m_hVfShutDown = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hEncShutDown = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hVfInterruptThread = NULL;
    m_hEncInterruptThread = NULL;

    m_hContext = (HANDLE)MDDContext;
    // Real drivers may want to create their context

    m_ulCTypes = 3; // Default number of Sensor Modes is 3

    if( pPDDFuncTbl->dwSize  < sizeof( PDDFUNCTBL2 ) )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    memcpy( pPDDFuncTbl, &FuncTbl, sizeof( PDDFUNCTBL2 ) );

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
    m_pModeVideoFormat[CAPTURE].ulAvailFormats       = 6;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[CAPTURE].ulAvailFormats];

    if( NULL == m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    // MX27 only support the above 6 format.
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[0] = &DCAM_StreamMode_YV12_QQCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[1] = &DCAM_StreamMode_YV12_QQVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[2] = &DCAM_StreamMode_YV12_QCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[3] = &DCAM_StreamMode_YV12_QVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[4] = &DCAM_StreamMode_YV12_CIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[5] = &DCAM_StreamMode_YV12_VGA;

    m_pModeVideoFormat[STILL].categoryGUID           = PINNAME_VIDEO_STILL;
    m_pModeVideoFormat[STILL].ulAvailFormats         = 6;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[STILL].ulAvailFormats];

    if( NULL == m_pModeVideoFormat[STILL].pCsDataRangeVideo )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    m_pModeVideoFormat[STILL].pCsDataRangeVideo[0]   = &DCAM_StreamMode_RGB565_QQCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[1]   = &DCAM_StreamMode_RGB565_QQVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[2]   = &DCAM_StreamMode_RGB565_QCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[3]   = &DCAM_StreamMode_RGB565_QVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[4]   = &DCAM_StreamMode_RGB565_CIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[5]   = &DCAM_StreamMode_RGB565_VGA;

    if( 3 == m_ulCTypes )
    {
        m_pModeVideoFormat[PREVIEW].categoryGUID         = PINNAME_VIDEO_PREVIEW;
        m_pModeVideoFormat[PREVIEW].ulAvailFormats       = 7;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[PREVIEW].ulAvailFormats];

        if( NULL == m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo )
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }

        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[0] = &DCAM_StreamMode_RGB565_QQCIF;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[1] = &DCAM_StreamMode_RGB565_QQVGA;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[2] = &DCAM_StreamMode_RGB565_QCIF;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[3] = &DCAM_StreamMode_RGB565_QVGA;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[4] = &DCAM_StreamMode_RGB565_CIF;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[5] = &DCAM_StreamMode_RGB565_VGA;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[6] = &DCAM_StreamMode_RGB565_QQQVGA;
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
    m_pModeVideoCaps[CAPTURE].CurrentVideoControlCaps     = DefaultVideoControlCaps[CAPTURE];
    m_pModeVideoCaps[STILL].DefaultVideoControlCaps       = DefaultVideoControlCaps[STILL];
    m_pModeVideoCaps[STILL].CurrentVideoControlCaps       = DefaultVideoControlCaps[STILL];
    if( 3 == m_ulCTypes )
    {
        // Note PREVIEW control caps are the same, so we don't differentiate
        m_pModeVideoCaps[PREVIEW].DefaultVideoControlCaps     = DefaultVideoControlCaps[PREVIEW];
        m_pModeVideoCaps[PREVIEW].CurrentVideoControlCaps     = DefaultVideoControlCaps[PREVIEW];
    }

    m_SensorModeInfo[CAPTURE].MemoryModel = CSPROPERTY_BUFFER_CLIENT_LIMITED;
    m_SensorModeInfo[CAPTURE].MaxNumOfBuffers = 4;
    m_SensorModeInfo[CAPTURE].PossibleCount = 1;
    m_SensorModeInfo[STILL].MemoryModel = CSPROPERTY_BUFFER_CLIENT_LIMITED;
    m_SensorModeInfo[STILL].MaxNumOfBuffers = 2;
    m_SensorModeInfo[STILL].PossibleCount = 1;
    if( 3 == m_ulCTypes )
    {
        m_SensorModeInfo[PREVIEW].MemoryModel = CSPROPERTY_BUFFER_CLIENT_LIMITED;
        m_SensorModeInfo[PREVIEW].MaxNumOfBuffers = 4;
        m_SensorModeInfo[PREVIEW].PossibleCount = 1;
    }

    m_ppModeContext = new LPVOID[m_ulCTypes];
    if ( NULL == m_ppModeContext )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    // zero m_ppModeContext for un-initialized pin
    for(UINT i = 0; i < m_ulCTypes; i++)
    {
        m_ppModeContext[i] = NULL;
    }

    m_pCurrentFormat = new CS_DATARANGE_VIDEO[m_ulCTypes];
    if( NULL == m_pCurrentFormat )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    m_pMetadata = (PCS_PROPERTYITEM)&Metadata;
    return ERROR_SUCCESS;
}


DWORD CamPddDevice::GetAdapterInfo( PADAPTERINFO pAdapterInfo )
{
    pAdapterInfo->ulCTypes = m_ulCTypes;
    pAdapterInfo->PowerCaps = PowerCaps;
    pAdapterInfo->ulVersionID = DRIVER_VERSION_2; //Camera MDD and DShow support DRIVER_VERSION and DRIVER_VERSION_2. Defined in camera.h
    memcpy( &pAdapterInfo->SensorProps, &m_SensorProps, sizeof(m_SensorProps));

    return ERROR_SUCCESS;

}

DWORD CamPddDevice::HandleVidProcAmpChanges( DWORD dwPropId, LONG lFlags, LONG lValue )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("HandleVidProcAmpChanges +\r\n")));
    PSENSOR_PROPERTY pDevProp = NULL;

    pDevProp = m_SensorProps + dwPropId;

    if( CSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL == lFlags )
    {
        pDevProp->ulCurrentValue = lValue;
    }

    pDevProp->ulFlags = lFlags;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("HandleVidProcAmpChanges -\r\n")));
    return ERROR_SUCCESS;
}

DWORD CamPddDevice::HandleCamControlChanges( DWORD dwPropId, LONG lFlags, LONG lValue )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("HandleCamControlChanges +\r\n")));
    PSENSOR_PROPERTY pDevProp = NULL;

    pDevProp = m_SensorProps + dwPropId;

    if( CSPROPERTY_CAMERACONTROL_FLAGS_MANUAL == lFlags )
    {
        pDevProp->ulCurrentValue = lValue;

        if ( CSPROPERTY_CAMERACONTROL_ZOOM == dwPropId)
        {
            // We support CSPROPERTY_CAMERACONTROL_ZOOM property only.
            CameraSensorZoom(pDevProp->ulCurrentValue);
        }
    }

    pDevProp->ulFlags = lFlags;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("HandleCamControlChanges -\r\n")));
    return ERROR_SUCCESS;
}

DWORD CamPddDevice::HandleVideoControlCapsChanges( LONG lModeType ,ULONG ulCaps )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("HandleVideoControlCapsChanges +\r\n")));
    m_pModeVideoCaps[lModeType].CurrentVideoControlCaps = ulCaps;

    if ( ulCaps & CS_VideoControlFlag_FlipHorizontal )
    {
        // Toggle Flip state
        m_bFlipHorizontal = !m_bFlipHorizontal;
        CameraSensorFlip(FALSE, m_bFlipHorizontal); // LG_NORTEL_MODIFY
        DEBUGMSG(ZONE_DEVICE, (_T("CAM_IOControl(%08x): Just toggled Horiz Flip.\r\n"), this));
        // Assure that we will reconfigure IPU before starting.
        //m_bCameraConfig = FALSE;
    }


    if ( ulCaps & CS_VideoControlFlag_FlipVertical )
    {
        // Flip turned from OFF to ON state
        m_bFlipVertical = !m_bFlipVertical;
        CameraSensorFlip(TRUE, m_bFlipVertical); // LG_NORTEL_MODIFY
        DEBUGMSG(ZONE_DEVICE, (_T("CAM_IOControl(%08x): Just toggled Vert Flip.\r\n"), this));
        // Assure that we will reconfigure IPU before starting.
        //m_bCameraConfig = FALSE;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("HandleVideoControlCapsChanges -\r\n")));
    return ERROR_SUCCESS;
}

DWORD CamPddDevice::SetPowerState( CEDEVICE_POWER_STATE PowerState )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SetPowerState +\r\n")));
    DWORD dwErr = ERROR_SUCCESS;

    switch (PowerState)
    {
    case D0:
        if (m_PowerState != D0 && m_PowerState != D2)
        {
            if ( CameraPowerUp(FALSE) )
            {
                m_PowerState = D0;
            }
            else
            {
                dwErr = ERROR_FUNCTION_FAILED;
            }
        }
        break;

    case D2:
        if (m_PowerState != D0 && m_PowerState != D2)
        {
            if ( CameraPowerUp(FALSE) )
            {
                m_PowerState = D2;
            }
            else
            {
                dwErr = ERROR_FUNCTION_FAILED;
            }
        }
        break;

    case D3:
        m_PowerState = D3;
        CameraPowerDown(FALSE);
        break;

    case D4:
        m_PowerState = D4;
        CameraPowerDown(TRUE);
        break;

    default:
        break;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SetPowerState -\r\n")));
    return dwErr;
}

DWORD CamPddDevice::HandleAdapterCustomProperties( PUCHAR pInBuf, DWORD  InBufLen, PUCHAR pOutBuf, DWORD  OutBufLen, PDWORD pdwBytesTransferred )
{
    //DEBUGMSG( ZONE_IOCTL, ( _T("IOControl Adapter PDD: Unsupported PropertySet Request\r\n")) );
    return ERROR_NOT_SUPPORTED;
}

DWORD CamPddDevice::InitSensorMode( ULONG ulModeType, LPVOID ModeContext )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("InitSensorMode +\r\n")));

    ASSERT( ModeContext );
    m_ppModeContext[ulModeType] = ModeContext;

    //Still pin used current opened pin. If no opened pin, used preview as default
    if( STILL == ulModeType )
    {
        m_StillPinInherited = PREVIEW;
    }

    CameraPowerUp(TRUE);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("InitSensorMode -\r\n")));
    return ERROR_SUCCESS;
}

DWORD CamPddDevice::DeInitSensorMode( ULONG ulModeType )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DeInitSensorMode +\r\n")));

    DeAllocateDriverBuffers(ulModeType);
    m_ppModeContext[ulModeType] = NULL;

    if( ( NULL == m_ppModeContext[CAPTURE] )
        && ( NULL == m_ppModeContext[STILL] )
        && ( NULL == m_ppModeContext[PREVIEW] ) )
        CameraPowerDown(TRUE);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("DeInitSensorMode -\r\n")));
    return ERROR_SUCCESS;
}

DWORD CamPddDevice::SetSensorState( ULONG lModeType, CSSTATE csState )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SetSensorState +\r\n")));

    DWORD dwError  = ERROR_SUCCESS;

    if (csState == m_CsState[lModeType])
    {
        return ERROR_SUCCESS;
    }

    switch ( csState )
    {
    case CSSTATE_STOP:
        DEBUGMSG(ZONE_IOCTL, (_T("SetSensorState(%d, CSSTATE_STOP)\r\n"), lModeType));
        m_CsState[lModeType] = CSSTATE_STOP;

        if ( CAPTURE == lModeType )
        {
            if (FALSE == m_pPrp->PrpStopEncChannel())
            {
                dwError = ERROR_FUNCTION_FAILED;
                break;
            }
        }
        else if ( PREVIEW == lModeType )
        {
            if (FALSE == m_pPrp->PrpStopVfChannel())
            {
                dwError = ERROR_FUNCTION_FAILED;
                break;
            }
        }
        else if( STILL == lModeType )
        {
            m_bStillCapInProgress = false;
            dwError = ERROR_INVALID_PARAMETER;
        }

        break;

    case CSSTATE_PAUSE:
        DEBUGMSG(ZONE_IOCTL, (_T("SetSensorState(%d, CSSTATE_PAUSE)\r\n"), lModeType));

        m_CsState[lModeType] = CSSTATE_PAUSE;

        break;

    case CSSTATE_RUN:
        DEBUGMSG(ZONE_IOCTL, (_T("SetSensorState(%d, CSSTATE_RUN)\r\n"), lModeType));

        if ( CSSTATE_STOP == m_CsState[lModeType] )
        {
            DEBUGMSG(ZONE_WARN, (_T("SetSensorState: transition from CSSTATE_STOP to CSSTATE_RUN.\r\n")));
        }

        m_CsState[lModeType] = CSSTATE_RUN;

        if ( PREVIEW == lModeType )
        {
            if ( FALSE == m_pPrp->PrpStartVfChannel() )
            {
                dwError = ERROR_FUNCTION_FAILED;
            }
        }
        else if ( CAPTURE == lModeType )
        {
            if ( FALSE == m_pPrp->PrpStartEncChannel() )
            {
                dwError = ERROR_FUNCTION_FAILED;
            }
        }
        else
        {// STILL mode should directly call TakeStillPicture
            dwError = ERROR_INVALID_STATE;
        }

        break;

    default:
        DEBUGMSG( ZONE_IOCTL|ZONE_ERROR, ( _T("IOControl(%08x): Incorrect State\r\n"), this ) );
        dwError = ERROR_INVALID_PARAMETER;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SetSensorState -\r\n")));
    return dwError;
}

DWORD CamPddDevice::TakeStillPicture( LPVOID pBurstModeInfo )
{
    //Ignore pBurstModeInfo
    DWORD dwError = ERROR_FUNCTION_FAILED;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TakeStillPicture +\r\n")));

    //The call can come faster than the actual frame rate.
    //Wait 300ms for the last still picture to finish before next one starts.
    //If longer than 300ms and last still picture is not done yet,
    //there must be something wrong.
    if(NULL==m_ppModeContext[PREVIEW])
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Preview pin is not running. TakeStillPicture Failed.\r\n")));
    }
    else
    {
        for (int i=0; i<10; i++)
        {
            if( FALSE == m_bStillCapInProgress )
            {
                //Save the inherited pin state in STill pin state
                m_CsState[STILL] = m_CsState[m_StillPinInherited];

                // If inherit pin is not in run state, set the inherit pin to run. Later revert it back to original state
                if ( CSSTATE_RUN != m_CsState[m_StillPinInherited] )
                {
                    SetSensorState( m_StillPinInherited, CSSTATE_RUN );
                }

                m_bStillCapInProgress = true;

                dwError = ERROR_SUCCESS;
                break;
            }

            //If last picture is still pending, wait for 30ms and check again.
            Sleep(30);
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("TakeStillPicture -\r\n")));
    return dwError;
}


DWORD CamPddDevice::GetSensorModeInfo( ULONG ulModeType, PSENSORMODEINFO pSensorModeInfo )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("GetSensorModeInfo +\r\n")));
    pSensorModeInfo->MemoryModel = m_SensorModeInfo[ulModeType].MemoryModel;
    pSensorModeInfo->MaxNumOfBuffers = m_SensorModeInfo[ulModeType].MaxNumOfBuffers;
    pSensorModeInfo->PossibleCount = m_SensorModeInfo[ulModeType].PossibleCount;
    pSensorModeInfo->VideoCaps.DefaultVideoControlCaps = DefaultVideoControlCaps[ulModeType];
    pSensorModeInfo->VideoCaps.CurrentVideoControlCaps = m_pModeVideoCaps[ulModeType].CurrentVideoControlCaps;
    pSensorModeInfo->pVideoFormat = &m_pModeVideoFormat[ulModeType];

    DEBUGMSG(ZONE_FUNCTION, (TEXT("GetSensorModeInfo -\r\n")));
    return ERROR_SUCCESS;
}

DWORD CamPddDevice::SetSensorModeFormat( ULONG ulModeType, PCS_DATARANGE_VIDEO pCsDataRangeVideo )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SetSensorModeFormat +\r\n")));
    DWORD dwError = ERROR_SUCCESS;

    memcpy( &m_pCurrentFormat[ulModeType], pCsDataRangeVideo, sizeof ( CS_DATARANGE_VIDEO ) );

    if ( STILL != ulModeType ) //No need to configure STILL pin's data format
    {
        DeAllocateDriverBuffers(ulModeType);

        if (!CameraPrpConfig(m_ulMaxNumOfBuffers))
        {
            dwError = ERROR_INSUFFICIENT_BUFFER;
        }

        if (!AllocateDriverBuffers(ulModeType))
        {
            dwError = ERROR_INSUFFICIENT_BUFFER;
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SetSensorModeFormat -\r\n")));
    return dwError;
}


PVOID CamPddDevice::AllocateBuffer( ULONG ulModeType )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("AllocateBuffer +\r\n")));
    //Used limited buffer model. Don't need to do anything.
    //Will change to other driver model for better performance
    DEBUGMSG(ZONE_FUNCTION, (TEXT("AllocateBuffer -\r\n")));
    return NULL;
}

DWORD CamPddDevice::DeAllocateBuffer( ULONG ulModeType, PVOID pBuffer )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DeAllocateBuffer +\r\n")));
    // In this PDD, we dont need it
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DeAllocateBuffer -\r\n")));
    return NULL;
}

DWORD CamPddDevice::RegisterClientBuffer( ULONG ulModeType, PVOID pBuffer )
{
    // In this PDD, we dont need it.
    return ERROR_SUCCESS;
}

DWORD CamPddDevice::UnRegisterClientBuffer( ULONG ulModeType, PVOID pBuffer )
{
    // DShow is not going to use pBuffer (which was originally allocated by DShow) anymore. If the PDD
    // is keeping a cached pBuffer pointer (in RegisterClientBuffer()) then this is the right place to
    // stop using it and maybe set the cached pointer to NULL.
    // Note: PDD must not delete this pointer as it will be deleted by DShow itself
    return ERROR_SUCCESS;
}

DWORD CamPddDevice::HandleSensorModeCustomProperties( ULONG ulModeType, PUCHAR pInBuf, DWORD  InBufLen, PUCHAR pOutBuf, DWORD  OutBufLen, PDWORD pdwBytesTransferred )
{
    //DEBUGMSG( ZONE_IOCTL, ( _T("IOControl: Unsupported PropertySet Request\r\n")) );
    return ERROR_NOT_SUPPORTED;
}


DWORD CamPddDevice::FillBuffer( ULONG ulModeType, PUCHAR pImage )
{
    // This function will be called in response to MDD_HandleIO
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FillBuffer+\r\n")));

    PUINT8 pbySrcBuffer, pbyDstBuffer;
    prpBufferData  prpBuffData;
    UINT retVal = 0;
    ULONG ulPrpChannel;

    PCS_VIDEOINFOHEADER pCsVideoInfoHdr = &m_pCurrentFormat[ulModeType].VideoInfoHeader;
    ASSERT(pCsVideoInfoHdr->bmiHeader.biSizeImage != 0);
    ULONG BufferSize = abs(CS__DIBSIZE (pCsVideoInfoHdr->bmiHeader));

    if ( STILL == ulModeType )
    {
        ulPrpChannel = m_StillPinInherited;

        if ( NULL != m_ppModeContext[m_StillPinInherited] )
        {   // Make sure the transfer size won't exceed both source and target buffer size.
            pCsVideoInfoHdr = &m_pCurrentFormat[m_StillPinInherited].VideoInfoHeader;
            ASSERT(pCsVideoInfoHdr->bmiHeader.biSizeImage != 0);
            ULONG vfBufferSize = abs(CS__DIBSIZE (pCsVideoInfoHdr->bmiHeader));

            if ( BufferSize > vfBufferSize )
            {
                BufferSize = vfBufferSize;
            }
        }
        else
        {   //If the preview pin has not been initialized, don't transfer data
            retVal = -1;
            goto EXIT;
        }
        if( m_pModeVideoCaps[STILL].CurrentVideoControlCaps & CS_VideoControlFlag_Sample_Scanned_Notification )
        {
            CAM_NOTIFICATION_CONTEXT camNotificationCtx;
            DWORD dwRet = ERROR_SUCCESS;
            camNotificationCtx.Size = sizeof(CAM_NOTIFICATION_CONTEXT);
            camNotificationCtx.Data = 0;    // MDD will populate this member
            camNotificationCtx.Result = ERROR_SUCCESS;
            dwRet = MDD_HandleNotification( m_hOpenContext, STILL, PDD_NOTIFICATION_SAMPLE_SCANNED, &camNotificationCtx );
            if (dwRet!=ERROR_SUCCESS)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT(" FillBuffer: MDD_HandleNotification returned error!\r\n")));
            }
        } 
    }
    else
    {
        ulPrpChannel = ulModeType;
    }

    if ( PREVIEW == ulPrpChannel )
    {
        if (FALSE == m_pPrp->PrpGetVfBufFilled(&prpBuffData))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT(" FillBuffer: Get viewfinding filled buffer error!\r\n")));
            retVal = -1;
            goto EXIT;
        }
    }
    else if ( CAPTURE == ulPrpChannel )
    {
        if (FALSE == m_pPrp->PrpGetEncBufFilled(&prpBuffData))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT(" FillBuffer: Get encoder filled buffer error!\r\n")));
            retVal = -1;
            goto EXIT;
        }
    }
    else
    {
        DEBUGMSG(ZONE_ERROR,(_T(" FillBuffer: Invalid pin id.\r\n"))) ;
        retVal = -1;
        goto EXIT;
    }

    pbyDstBuffer =  reinterpret_cast<PUINT8>(pImage);

    if ((prpBuffData.pPhysAddr != NULL) && (prpBuffData.pVirtAddr != NULL))
    {
        // Here, copy the whole frame buffer memory to application.
        pbySrcBuffer = (PUINT8) prpBuffData.pVirtAddr;
        memcpy(pbyDstBuffer, pbySrcBuffer, BufferSize);
        retVal = BufferSize;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT(" FillBuffer: pbySrcBuffer NULL!\r\n")));
        retVal = 0;
    }

    if ( PREVIEW == ulPrpChannel )
    {
        m_pPrp->PrpPutVfBufIdle(&prpBuffData);
    }
    else
    {
        m_pPrp->PrpPutEncBufIdle(&prpBuffData);
    }

EXIT:
    DEBUGMSG(ZONE_FUNCTION, (TEXT("FillBuffer-\r\n")));
    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function:           CameraPowerUp
//
// Description:        Power up the driver
//
// Parameters:
//      enableMode
//         [in]  If TRUE, enable the clock and other hardware related resource before enable this mode.
//                        doing this before open the device
//                If FALSE, enable the device clock only.
//                        doing this by power management.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CamPddDevice::CameraPowerUp(BOOL enableMode)
{
    BOOL bResult = TRUE;

    // Power Up CSI module. Enable clock & CSI module.
    m_pCsi->CsiEnable();

    if  ((TRUE == enableMode) && ((NULL == m_hVfInterruptThread) || (NULL == m_hEncInterruptThread)))    //The interrupt thread handle is also an indication if the prp started
    {
        // When: open the device.
        // Power Up Prp module. Initialize interrupt and event
        bResult = m_pPrp->PrpPowerUp();

        // If power up failure, it will release the resource automatically.
        if (FALSE == bResult)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s : m_pPrp->PrpPowerUp failed\r\n"), __WFUNCTION__));
            goto EXIT;
        }

        // Enable clock and interrupt;
        // If there is a module has opened PrP module or PrP enable failure, PrpEnable() return FALSE.
        bResult = m_pPrp->PrpEnable();

        if (FALSE == bResult)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s : m_pPrp->PrpEnable failed\r\n"), __WFUNCTION__));
            goto EXIT;
        }

        //create and start m_hVfInterruptThread
        m_hVfInterruptThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
            0,
            (LPTHREAD_START_ROUTINE)CallVfThread,
            this,
            0,
            NULL);
        if (!m_hVfInterruptThread)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s : Create m_hVfInterruptThread  failed\r\n"), __WFUNCTION__));
            bResult = FALSE;
            goto EXIT;
        }

        //create and start m_hEncInterruptThread
        m_hEncInterruptThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
            0,
            (LPTHREAD_START_ROUTINE)CallEncThread,
            this,
            0,
            NULL);
        if (!m_hEncInterruptThread)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s : Create m_hEncInterruptThread  failed\r\n"), __WFUNCTION__));
            bResult = FALSE;
            goto EXIT;
        }
    }

EXIT:
    if ( FALSE == bResult )
    {
        CloseHandle(m_hVfInterruptThread);
        m_hVfInterruptThread = NULL;
        CloseHandle(m_hEncInterruptThread);
        m_hEncInterruptThread= NULL;
        m_pPrp->PrpDisable();
        m_pPrp->PrpPowerDown();
    }
    return bResult;
}

//-----------------------------------------------------------------------------
//
// Function:           CameraPowerDown
//
// Description:        Power down the driver
//
// Parameters:
//      disableMode
//         [in]  If TRUE, disable the clock and other hardware related resource after disable this mode.
//                        doing this before close the device
//               If FALSE, disable the device clock only.
//                        doing this by power management.
//
// Returns:            None
//
//-----------------------------------------------------------------------------
void CamPddDevice::CameraPowerDown(BOOL disableMode)
{
    if ((TRUE == disableMode) && ((NULL != m_hVfInterruptThread) || (NULL != m_hEncInterruptThread)))    //The interrupt thread handle is also an indication if the prp started
    {
        // Signal the IST to shutdown
        if ( m_hVfInterruptThread )
        {
            SetEvent(m_hVfShutDown);
            CloseHandle(m_hVfInterruptThread);
            m_hVfInterruptThread = NULL;
        }

        if ( m_hEncInterruptThread )
        {
            SetEvent(m_hEncShutDown);
            CloseHandle(m_hEncInterruptThread);
            m_hEncInterruptThread = NULL;
        }

        // Disable clock and interrupt;
        m_pPrp->PrpDisable();
        // When: close the device.
        // Power Down Prp module. Deinitialize interrupt and event
        m_pPrp->PrpPowerDown();
    }

    // Power Down CSI module. Disable clock & CSI module.
    m_pCsi->CsiDisable();

}

//-----------------------------------------------------------------------------
//
// Function:            AllocateDriverBuffers
//
// Description:        Allocate the buffers for PRP Vf and Enc channel based on the current Vf and Enc format.
//
// Parameters:
//      ulModeType
//         [in]  Pin number. 0 - Capture, 1 - Still, 2 - Capture. Still pin will be ignored.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CamPddDevice::AllocateDriverBuffers(ULONG ulModeType)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("+AllocateDriverBuffers\r\n")));

    BOOL hr = FALSE;

    ULONG m_ulFrameSize = abs(CS__DIBSIZE(m_pCurrentFormat[ulModeType].VideoInfoHeader.bmiHeader)); // by bytes

    // Call down to Prp to allocate new buffers
    if( PREVIEW == ulModeType )
    {
        if ( m_pPrp->PrpAllocateVfBuffers(m_ulMaxNumOfBuffers, m_ulFrameSize) )
        {
            hr = TRUE;
        }
    }
    else if( CAPTURE == ulModeType )
    {
        if ( m_pPrp->PrpAllocateEncBuffers(m_ulMaxNumOfBuffers, m_ulFrameSize) )
        {
            hr = TRUE;
        }
    }
    else if( STILL == ulModeType )
    {
        // No need to allocate buffers in the Prp, as those buffers
        // will already have been allocated by the Still Pin's
        // channel (enc or vf).
        hr = TRUE;
    }

    DEBUGMSG(ZONE_FUNCTION, (_T("+AllocateDriverBuffers ( hr = %d )\r\n"), hr));

    return hr;
}


//-----------------------------------------------------------------------------
//
// Function:            DeAllocateDriverBuffers
//
// Description:        DeAllocate the buffers for PRP Vf and Enc channel.
//
// Parameters:
//      ulModeType
//         [in]  Pin number. 0 - Capture, 1 - Still, 2 - Capture. Still pin will be ignored.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CamPddDevice::DeAllocateDriverBuffers(ULONG ulModeType)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("+DeAllocateDriverBuffers\r\n")));

    BOOL hr = FALSE;

    // Call down to Prp to deallocate buffer list
    if( PREVIEW == ulModeType )
    {
        if ( m_pPrp->PrpDeleteVfBuffers() )
        {
            hr = TRUE;
        }
    }
    else if( CAPTURE == ulModeType )
    {
        if ( m_pPrp->PrpDeleteEncBuffers() )
        {
            hr = TRUE;
        }
    }
    else if( STILL == ulModeType )
    {
        // No need to delete buffers in the Prp, as those buffers
        // will be deleted by the Still Pin's channel (enc or vf).
        hr = TRUE;
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("DeAllocateDriverBuffers ( hr = %d )\r\n"), hr)) ;

    return hr;
}

//-----------------------------------------------------------------------------
//
// Function: CameraSensorConfig
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
BOOL CamPddDevice::CameraSensorConfig()
{
    csiSensorOutputFormat sensorOutputFormat;
    csiSensorOutputResolution sensorOutputResolution;
    DWORD sensorFormat,sensorResolution;

    //----------------------------------------------------
    // Configure camera sensor and CSI
    //----------------------------------------------------
    BSPGetSensorFormat(&sensorFormat);
    BSPGetSensorResolution(&sensorResolution);

    sensorOutputFormat = (csiSensorOutputFormat) sensorFormat;
    sensorOutputResolution = (csiSensorOutputResolution) sensorResolution;

    if (!m_pCsi->CsiConfigureSensor(sensorOutputFormat, sensorOutputResolution))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Sensor configuration failed.  Aborting remaining configuration steps.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    //----------------------------------------------------
    // Set up preprocessor input parameters common
    // to both encoding and viewfinding tasks
    //----------------------------------------------------
    if (sensorOutputFormat == csiSensorOutputFormat_RGB)
    {
        m_inFormat = prpInputFormat_RGB16; // Interlaced RGB888
    }
    else if ((sensorOutputFormat == csiSensorOutputFormat_YUV422) ||
        (sensorOutputFormat == csiSensorOutputFormat_YUV444))
    {
        m_inFormat = prpInputFormat_UYVY422; // Interlaced YUV444
    }
    else
    {
        //m_inFormat = prpInputFormat_Generic; // Bayer data
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Sensor configuration failed. Don't Support the sensor output format.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    //    m_inFormat = prpInputFormat_RGB; // Use this if we are
    // are using CSI test pattern.

    // Set up Preproc input width and height configuration
    // parameters from the output resolution.
    switch (sensorOutputResolution)
    {
    case csiSensorOutputResolution_SXGA:
        m_inFrameSize.width = 1280;
        m_inFrameSize.height = 960;
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

    case csiSensorOutputResolution_PAL:
        m_inFrameSize.width = 720;
        m_inFrameSize.height = 288;
        break;

    case csiSensorOutputResolution_NTSC:
        m_inFrameSize.width = 720;
        m_inFrameSize.height = 244;
        break;

    default:
        break;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CameraPrpConfig
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
BOOL CamPddDevice::CameraPrpConfig(ULONG iBufNum)
{
    DWORD Format;
    static BOOL bPrpOnceConfigured = FALSE;
    static  prpConfigData prpConfig;

    // Set input format and frame size config params
    prpConfig.inputFormat = m_inFormat;
    prpConfig.inputSize = m_inFrameSize;

    if (m_inFormat == prpInputFormat_RGB16)
    {
        prpConfig.CSCEquation = prpCSCR2Y_A0;
    }
    else if (m_inFormat == prpInputFormat_UYVY422)
    {
        prpConfig.CSCEquation = prpCSCY2R_A0;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Prp configuration failed. Don't Support the sensor output format.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    //
    // Preview pin configuration
    //
    Format = m_pCurrentFormat[PREVIEW].VideoInfoHeader.bmiHeader.biCompression & ~BI_SRCPREROTATE;

    switch (Format)
    {
        // Used by GSG in MX27 driver for RGB565
    case FOURCC_YUYV:
        prpConfig.outputVfFormat = prpVfOutputFormat_YUYV422;
        break;

    case CS_BI_BITFIELDS:
        {
            switch (m_pCurrentFormat[PREVIEW].VideoInfoHeader.bmiHeader.biBitCount)
            {
            case 8:
                prpConfig.outputVfFormat = prpVfOutputFormat_RGB8;
                break;

            case 16:
                prpConfig.outputVfFormat = prpVfOutputFormat_RGB16;
                break;

            case 32:
                prpConfig.outputVfFormat = prpVfOutputFormat_RGB32;
                break;

            default:
                DEBUGMSG(ZONE_ERROR, (TEXT("%s: Unsupported biBitCount for PREVIEW pin! \r\n"),__WFUNCTION__));
                return FALSE;
            }
        }
        break;

    default:
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Invalid data format for preview.\r\n"), __WFUNCTION__));
        prpConfig.outputVfFormat = prpVfOutputFormat_Disabled;
        break;
    }

    // Viewfinding channel output size
    prpConfig.outputVfSize.width = (UINT16)m_pCurrentFormat[PREVIEW].VideoInfoHeader.bmiHeader.biWidth;
    prpConfig.outputVfSize.height = (UINT16)abs(m_pCurrentFormat[PREVIEW].VideoInfoHeader.bmiHeader.biHeight);

    prpConfig.outputVfStride = 0;

    //
    // CAPTURE pin configuration
    //
    Format = m_pCurrentFormat[CAPTURE].VideoInfoHeader.bmiHeader.biCompression & ~BI_SRCPREROTATE;;

    switch (Format)
    {
    case FOURCC_YUYV:
        prpConfig.outputEncFormat = prpEncOutputFormat_YUV422;
        break;

    case FOURCC_Y444:
        prpConfig.outputEncFormat = prpEncOutputFormat_YUV444;
        break;

    case FOURCC_YV12:
        prpConfig.outputEncFormat = prpEncOutputFormat_YUV420;
        break;

    default:
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Invalid data format for Capture/Still.\r\n"), __WFUNCTION__));
        prpConfig.outputEncFormat = prpEncOutputFormat_Disabled;
        break;
    }

    // Viewfinding channel output size
    prpConfig.outputEncSize.width = (UINT16)m_pCurrentFormat[CAPTURE].VideoInfoHeader.bmiHeader.biWidth;
    prpConfig.outputEncSize.height = (UINT16)abs(m_pCurrentFormat[CAPTURE].VideoInfoHeader.bmiHeader.biHeight);

    //
    // The PrP features which only need configuring once
    //
    if (!bPrpOnceConfigured) {
        // PrP frame skip control
        if ( TRUE == BSPCameraIsTVin() )
        {
            prpConfig.CSIInputSkip = prpSkip_1of2;
        }
        else
        {
            prpConfig.CSIInputSkip = prpSkip_NoSkip;
        }

        prpConfig.VfOutputSkip = prpSkip_NoSkip;
        prpConfig.EncOutputSkip = prpSkip_NoSkip;

        // Windowing control
        prpConfig.bWindowing = FALSE;
        prpConfig.CSILineSkip = 0;
        prpConfig.inputStride = 0;

        bPrpOnceConfigured = TRUE;
    }

    // Configure pre-processor

    if (m_pPrp->PrpConfigure(&prpConfig) == FALSE) {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: Pre-processor configuration failed! \r\n"), __WFUNCTION__));

        return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CameraSensorZoom
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
void CamPddDevice::CameraSensorZoom(DWORD zoomVal)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("%+ CameraSensorZoom( %d )\r\n"), zoomVal));

    m_pCsi->CsiZoom(zoomVal);

    m_SensorProps[ENUM_ZOOM].ulCurrentValue = zoomVal;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("%- CameraSensorZoom( %d )\r\n"), zoomVal));
}

// LG_NORTEL_MODIFY
void CamPddDevice::CameraSensorFlip(BOOL bVertical, BOOL bFlippingOn)
{
    //m_pCsi->CsiFlip(bVertical, bFlippingOn);
}

BOOL CamPddDevice::CameraVfIST()
{
    CeSetThreadPriority(GetCurrentThread(), 1);

    HANDLE events[2];
    DWORD event_id = 0;
    events[0] = m_pPrp->m_hVfEOFEvent;
    events[1] = m_hVfShutDown;

    while((event_id - WAIT_OBJECT_0) != 1)
    {
        event_id = WaitForMultipleObjects(2, (CONST HANDLE*)events, FALSE, INFINITE);

        if ((event_id - WAIT_OBJECT_0) == 0)
        {
            if (( true == m_bStillCapInProgress ) && ( PREVIEW == m_StillPinInherited ))
            {// Identified this is a still picture capture
                MDD_HandleIO( m_ppModeContext[STILL], STILL );
                m_bStillCapInProgress = false;

                if ( CSSTATE_RUN != m_CsState[STILL] )
                {
                    SetSensorState( PREVIEW, m_CsState[STILL] ); //restore the PREVIEW pin state saved in m_CsState[STILL]
                }
            }
            else
            {
                MDD_HandleIO( m_ppModeContext[PREVIEW], PREVIEW );
            }
        }
    }

    ExitThread(0);
    return TRUE;
}

DWORD CallVfThread(CamPddDevice *pCamPddDevice)
{
    pCamPddDevice->CameraVfIST();
    return 0;
}


BOOL CamPddDevice::CameraEncIST()
{
    CeSetThreadPriority(GetCurrentThread(), 1);

    HANDLE events[2];
    DWORD event_id = 0;
    events[0] = m_pPrp->m_hEncEOFEvent;
    events[1] = m_hEncShutDown;

    while((event_id - WAIT_OBJECT_0) != 1)
    {
        event_id = WaitForMultipleObjects(2, (CONST HANDLE*)events, FALSE, INFINITE);

        if ((event_id - WAIT_OBJECT_0) == 0)
        {
            MDD_HandleIO( m_ppModeContext[CAPTURE], CAPTURE );
        }
    }

    ExitThread(0);
    return TRUE;
}

DWORD CamPddDevice::Open( PVOID MDDOpenContext )
{
    if( MDDOpenContext == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    m_hOpenContext = (HANDLE)MDDOpenContext;
    return ERROR_SUCCESS;
}

DWORD CamPddDevice::Close( PVOID MDDOpenContext )
{
    if( m_hOpenContext != MDDOpenContext )
    {
        return ERROR_INVALID_PARAMETER;
    }

    m_hOpenContext = NULL;
    return ERROR_SUCCESS;
}

DWORD CamPddDevice::GetMetadata(DWORD dwPropId, PUCHAR pOutBuf, DWORD OutBufLen, PDWORD pdwBytesTransferred)
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
DWORD CallEncThread(CamPddDevice *pCamPddDevice)
{
    pCamPddDevice->CameraEncIST();
    return 0;
}
