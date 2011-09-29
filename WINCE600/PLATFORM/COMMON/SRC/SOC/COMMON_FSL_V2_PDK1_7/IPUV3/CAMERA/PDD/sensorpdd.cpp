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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#pragma warning(disable: 4100 4115 4127 4189 4201 4204 4214 4245 4702 4706 6244)

#include <windows.h>
#include <ceddk.h>
#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#include <winddi.h>
#include <ddgpe.h>
#define __DVP_INCLUDED__

#include <Msgqueue.h>
#include <pwinbase.h>
#include <pm.h>

#include "Cs.h"
#include "Csmedia.h"

#include "common_ipuv3.h"
#include "common_macros.h"

#include "IpuBuffer.h"
#include "IPU_base.h"
#include "ipu_common.h"

#include "CamBufferManager.h"
#include "tpm.h"
#include "pp.h"
#include "idmac.h"
#include "smfc.h"
#include "SMFCClass.h"
#include "CameraPPClass.h"
#include "csi.h"
#include "CsiClass.h"

#include "CameraPDDProps.h"
#include "dstruct.h"
#include "camera.h"
#include "CameraDriver.h"
#include "SensorFormats.h"
#include "SensorProperties.h"
#include "PinDriver.h"

#include "CameraPDD.h"
#include "SensorPdd.h"
#include "wchar.h"
#include "cameradbg.h"


// External Functions
extern void BSPGetSensorFormat(DWORD *);
extern void BSPGetSensorResolution(DWORD *);
extern void BSPEnableCamera(CSI_SELECT csi_sel);
extern void BSPDisableCamera(CSI_SELECT csi_sel);
extern void BSPSensorFlip(BOOL doFlip);
extern DWORD BSPGetSiVer();


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


CSensorPdd::CSensorPdd()
{
    m_dwSiVer = 0;
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
    m_ulNumOfBuffer = NUM_PIN_BUFFER_DRIVER;    

    m_pCsi0 = NULL;    
    m_pCsi1 = NULL;
    m_CSIInterface = CSI_SELECT_CSI0;
    m_pSMFC = NULL;
    m_pCameraPP = NULL;
    m_bCameraEncConfigRequest  = FALSE;
    m_bCameraVfConfigRequest = FALSE;
    m_bCameraEncConfig = FALSE;
    m_bCameraStillConfig = FALSE;
    m_bCameraVfConfig = FALSE;

    m_bDirectDisplay = FALSE;

    m_bFlipVertical = FALSE;
    m_bFlipHorizontal = FALSE;
    m_bRotate = FALSE;

    m_SensorConfigured = FALSE;
    m_bCameraPPEnable = TRUE;

    m_ulSMFCBufSize = 0;
    m_ulCameraPPBufSize = 0;
    m_bAllocateBufferForSMFC = FALSE;
    m_bAllocateBufferForPP = FALSE;
    m_bAllocateBufferForSTILL = FALSE;
    m_bSensorBufferAllocByDriver = FALSE;
    m_dwSensorFramerate = 15;

    memset(&m_PrtclInf, 0, sizeof(CSI_PROTOCOL_INF));
    memset(&m_SMFCConfig, 0, sizeof(SMFCConfigData));
    memset(&m_PPConfig, 0, sizeof(ppConfigData));

    memset( &m_TimerIdentifier, 0x0, sizeof(m_TimerIdentifier));    
    memset( &m_CsState, 0x0, sizeof(m_CsState));
    memset( &m_SensorModeInfo, 0x0, sizeof(m_SensorModeInfo));
    memset( &m_SensorProps, 0x0, sizeof(m_SensorProps));
    memset( &PowerCaps, 0x0, sizeof(PowerCaps));

    // for the issue: new buffer manager
    m_pPreviewBufferManager = NULL;
    m_pCaptureBufferManager = NULL;
    m_pStillBufferManager = NULL;

    m_hFslFilter = NULL;
    m_bCsiTstMode = FALSE;

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
        CeSetThreadPriority(m_hPinEncThread, 248);
        DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: create Enc ISR thread success\r\n"), __WFUNCTION__));
    }

    m_hPinVfThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PinVfWorkerThread, this, 0, NULL);
    if (m_hPinVfThread == NULL)
    {
        ERRORMSG(TRUE,(TEXT("%s: CreateThread failed for Pin Vf!\r\n"), __WFUNCTION__));
    }
    else
    {
        CeSetThreadPriority(m_hPinVfThread, 248);
        DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: create Vf ISR thread success\r\n"), __WFUNCTION__));
    }
}

CSensorPdd::~CSensorPdd()
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
        // Set SMFCIntrThread end
        SetEvent ( m_hPinEncEvent[1] ); 
        // Wait for PrpIntrThread end
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hPinEncThread,INFINITE))
            DEBUGMSG(ZONE_FUNCTION,(TEXT("Wait for PinEncWorkerThread end")));

        CloseHandle(m_hPinEncThread);
        m_hPinEncThread = NULL;    
    }

    if (m_hPinVfThread)
    {    
        // Set SMFCIntrThread end
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
    if( NULL != m_pSMFC )
    {
        delete m_pSMFC;
        m_pSMFC = NULL;
    }

    if( NULL != m_pCameraPP )
    {
        delete m_pCameraPP;
        m_pCameraPP = NULL;
    }

    if( NULL != m_pCsi0 )
    {
        delete m_pCsi0;
        m_pCsi0 = NULL;
    }

    if( NULL != m_pCsi1 )
    {
        delete m_pCsi1;
        m_pCsi1 = NULL;
    }

    // for the issue: new buffer manager
    if( NULL != m_pPreviewBufferManager)
    {
        delete m_pPreviewBufferManager;
        m_pPreviewBufferManager = NULL;
    }
    
    if( NULL != m_pCaptureBufferManager)
    {
        delete m_pCaptureBufferManager;
        m_pCaptureBufferManager = NULL;
    }
    
    if( NULL != m_pStillBufferManager)
    {
        delete m_pStillBufferManager;
        m_pStillBufferManager = NULL;
    }

    if( NULL != m_hFslFilter)
    {
        CloseHandle(m_hFslFilter);
        m_hFslFilter = NULL;
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
// This function is the EOF frame handler for the SMFC.
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
    DWORD dwRet;

    // loop here
    while( (result = WaitForMultipleObjects (2, m_hPinEncEvent, FALSE, INFINITE )) != WAIT_TIMEOUT )
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::PinEncWorkerLoop get event 0x%x\r\n"),result));
        switch(result)
        {
            case (WAIT_OBJECT_0 + 0):
                {
                    DEBUGMSG(ZONE_FUNCTION,(TEXT("CAPTURE pin start work\r\n")));
                    while ( m_CsState[CAPTURE] == CSSTATE_RUN )
                    {
                        dwRet = WaitForSingleObject(m_pSMFC->m_hCameraSMFCEOFEvent, 100);
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
                    }
                    DEBUGMSG(ZONE_FUNCTION,(TEXT("CAPTURE pin stop work\r\n")));
                }
                break;

            case (WAIT_OBJECT_0 + 1):
                // Program buffer into c parameter memory
                ExitThread(1);
                break;

            default:
                // Error
                break;
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
// This function is the EOF frame handler for the PP.
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
    DWORD dwRet;

    // loop here
    while( (result = WaitForMultipleObjects (2, m_hPinVfEvent, FALSE, INFINITE )) != WAIT_TIMEOUT)
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::PinVfWorkerLoop get event 0x%x\r\n"),result));
        switch(result)
        {
            case (WAIT_OBJECT_0 + 0):
                {
                    DEBUGMSG(ZONE_FUNCTION,(TEXT("PREVIEW pin start work\r\n")));

                    while ( m_CsState[PREVIEW] == CSSTATE_RUN )
                    {
                        dwRet = WaitForSingleObject(m_pCameraPP->m_hCameraPPEOFEvent, 100);
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
                    }

                    DEBUGMSG(ZONE_FUNCTION,(TEXT("PREVIEW pin stop work\r\n")));
                }
                break;

            case (WAIT_OBJECT_0 + 1):
                DEBUGMSG(ZONE_FUNCTION,(TEXT("PREVIEW thread killed\r\n")));
                ExitThread(1);
                break;

            default:
                DEBUGMSG(ZONE_FUNCTION,(TEXT("PREVIEW thread error\r\n")));
                break;
        }
    }

    return;
}


DWORD CSensorPdd::PDDInit( PVOID MDDContext, PPDDFUNCTBL pPDDFuncTbl )
{
    // Get snapshot of silicon rev
    m_dwSiVer = BSPGetSiVer();

    ///////////////////////////////////////////////////////////////////////////////
    //init IPU class
    ///////////////////////////////////////////////////////////////////////////////
    // Create (and initialize) SMFC class object
    if(m_pSMFC == NULL)
    {
        m_pSMFC = new SMFCClass;
    }
    else
    {
        ERRORMSG(TRUE, (TEXT("%s: Failed creating SMFCClass object!\r\n"), __WFUNCTION__));
    }

    // Create (and initialize) CameraPP class object
    if(m_pCameraPP == NULL)
    {
        m_pCameraPP = new CameraPPClass;
    }
    else
    {
        ERRORMSG(TRUE, (TEXT("%s: Failed creating CameraPPClass object!\r\n"), __WFUNCTION__));
    }

    // Create (and initialize) CSI class object
    // Get the CSI selected interface
    ReadCSIInterfaceFromRegistry();
    switch (m_CSIInterface)
    {
        case CSI_SELECT_CSI0:
            if(m_pCsi0 == NULL)
            {
                m_pCsi0 = new CsiClass(CSI_SELECT_CSI0);
            }
            else
            {
                ERRORMSG(TRUE, (TEXT("%s: Failed creating CsiClass for CSI0 object!\r\n"), __WFUNCTION__));
            }
            break;

        case CSI_SELECT_CSI1:
            if(m_pCsi1 == NULL)
            {
                m_pCsi1 = new CsiClass(CSI_SELECT_CSI1);
            }
            else
            {
                ERRORMSG(TRUE, (TEXT("%s: Failed creating CsiClass for CSI1 object!\r\n"), __WFUNCTION__));
            }
            break;

        case CSI_SELECT_CSIAll:
            RETAILMSG(1,(TEXT("Not support CSI0 and CSI1 simultaneously now!\r\n")));
            break;

        default:
           ERRORMSG(TRUE, (TEXT("%s: Failed creating CSI object.No CSI interface selected!\r\n"), __WFUNCTION__));
           break;
    }

    // for the issue: new buffer manager
    // Create (and initialize) CamBufferManager class object for Preview Pin
    if(m_pPreviewBufferManager == NULL)
    {
        m_pPreviewBufferManager = new CamBufferManager;
    }
    else
    {
        ERRORMSG(TRUE, (TEXT("%s: Failed creating CamBufferManager object for Preview Pin\r\n"),__WFUNCTION__));
    }
    
    // Create (and initialize) CamBufferManager class object for CAPTURE Pin
    if(m_pCaptureBufferManager == NULL)
    {
        m_pCaptureBufferManager = new CamBufferManager;
    }
    else
    {
        ERRORMSG(TRUE, (TEXT("%s: Failed creating CamBufferManager object for Capture Pin\r\n"),__WFUNCTION__));
    }
    
    // Create (and initialize) CamBufferManager class object for STILL Pin
    if(m_pStillBufferManager == NULL)
    {
        m_pStillBufferManager = new CamBufferManager;
    }
    else
    {
        ERRORMSG(TRUE, (TEXT("%s: Failed creating CamBufferManager object for Still Pin\r\n"),__WFUNCTION__));
    }

    //Create an event to flag filter's factory
    m_hFslFilter = CreateEvent(NULL, TRUE, FALSE, TEXT("Freescale codec filter"));

    if(m_hFslFilter == NULL)
    {
        ERRORMSG(TRUE, (TEXT("%s: Failed creating Event for hFslFilter\r\n"),__WFUNCTION__));
    }
    
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
    m_pModeVideoFormat[CAPTURE].ulAvailFormats       = 28;
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
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[5] = &DCAM_StreamMode_RGB565_D1_PAL;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[6] = &DCAM_StreamMode_RGB565_D1_NTSC;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[7] = &DCAM_StreamMode_YV12_QQVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[8] = &DCAM_StreamMode_YV12_QCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[9] = &DCAM_StreamMode_YV12_QVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[10] = &DCAM_StreamMode_YV12_CIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[11] = &DCAM_StreamMode_YV12_VGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[12] = &DCAM_StreamMode_YV12_D1_PAL;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[13] = &DCAM_StreamMode_YV12_D1_NTSC;    
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[14] = &DCAM_StreamMode_UYVY_QQVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[15] = &DCAM_StreamMode_UYVY_QCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[16] = &DCAM_StreamMode_UYVY_QVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[17] = &DCAM_StreamMode_UYVY_CIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[18] = &DCAM_StreamMode_UYVY_VGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[19] = &DCAM_StreamMode_UYVY_D1_PAL;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[20] = &DCAM_StreamMode_UYVY_D1_NTSC;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[21] = &DCAM_StreamMode_NV12_QQVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[22] = &DCAM_StreamMode_NV12_QCIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[23] = &DCAM_StreamMode_NV12_QVGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[24] = &DCAM_StreamMode_NV12_CIF;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[25] = &DCAM_StreamMode_NV12_VGA;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[26] = &DCAM_StreamMode_NV12_D1_PAL;
    m_pModeVideoFormat[CAPTURE].pCsDataRangeVideo[27] = &DCAM_StreamMode_NV12_D1_NTSC;

    
    m_pModeVideoFormat[STILL].categoryGUID           = PINNAME_VIDEO_STILL;
    m_pModeVideoFormat[STILL].ulAvailFormats         = 15;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo = new PCS_DATARANGE_VIDEO[m_pModeVideoFormat[STILL].ulAvailFormats];

    if( NULL == m_pModeVideoFormat[STILL].pCsDataRangeVideo )
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[0] = &DCAM_StreamMode_RGB565_QQVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[1] = &DCAM_StreamMode_RGB565_QCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[2] = &DCAM_StreamMode_RGB565_QVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[3] = &DCAM_StreamMode_RGB565_CIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[4] = &DCAM_StreamMode_RGB565_VGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[5] = &DCAM_StreamMode_YV12_QQVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[6] = &DCAM_StreamMode_YV12_QCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[7] = &DCAM_StreamMode_YV12_QVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[8] = &DCAM_StreamMode_YV12_CIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[9] = &DCAM_StreamMode_YV12_VGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[10] = &DCAM_StreamMode_UYVY_QQVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[11] = &DCAM_StreamMode_UYVY_QCIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[12] = &DCAM_StreamMode_UYVY_QVGA;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[13] = &DCAM_StreamMode_UYVY_CIF;
    m_pModeVideoFormat[STILL].pCsDataRangeVideo[14] = &DCAM_StreamMode_UYVY_VGA;

    if( MAX_SUPPORTED_PINS == m_ulCTypes )
    {
        m_pModeVideoFormat[PREVIEW].categoryGUID         = PINNAME_VIDEO_PREVIEW;
        m_pModeVideoFormat[PREVIEW].ulAvailFormats       = 15;
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

        if( m_dwSiVer >= 0x20 )
        {
            m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[5] = &DCAM_StreamMode_YV12_QQVGA;
            m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[6] = &DCAM_StreamMode_YV12_QCIF;
            m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[7] = &DCAM_StreamMode_YV12_QVGA;
            m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[8] = &DCAM_StreamMode_YV12_CIF;
            m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[9] = &DCAM_StreamMode_YV12_VGA;
        }
        else
        {
            m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[5] = &DCAM_StreamMode_NV12_QQVGA;
            m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[6] = &DCAM_StreamMode_NV12_QCIF;
            m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[7] = &DCAM_StreamMode_NV12_QVGA;
            m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[8] = &DCAM_StreamMode_NV12_CIF;
            m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[9] = &DCAM_StreamMode_NV12_VGA;
        }

        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[10] = &DCAM_StreamMode_UYVY_QQVGA;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[11] = &DCAM_StreamMode_UYVY_QCIF;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[12] = &DCAM_StreamMode_UYVY_QVGA;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[13] = &DCAM_StreamMode_UYVY_CIF;
        m_pModeVideoFormat[PREVIEW].pCsDataRangeVideo[14] = &DCAM_StreamMode_UYVY_VGA;
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

    // for the issue:cetk
    //m_SensorModeInfo[CAPTURE].MemoryModel = CSPROPERTY_BUFFER_DRIVER | CSPROPERTY_BUFFER_CLIENT_UNLIMITED;
    m_SensorModeInfo[CAPTURE].MemoryModel = CSPROPERTY_BUFFER_DRIVER;
    m_SensorModeInfo[CAPTURE].MaxNumOfBuffers = 1;
    m_SensorModeInfo[CAPTURE].PossibleCount = 1;

    m_SensorModeInfo[STILL].MemoryModel = CSPROPERTY_BUFFER_DRIVER;
    m_SensorModeInfo[STILL].MaxNumOfBuffers = 1;
    m_SensorModeInfo[STILL].PossibleCount = 1;
    if( MAX_SUPPORTED_PINS == m_ulCTypes )
    {
        // for the issue:cetk
        //m_SensorModeInfo[PREVIEW].MemoryModel =    CSPROPERTY_BUFFER_DRIVER | CSPROPERTY_BUFFER_CLIENT_UNLIMITED;
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
                m_bCameraVfConfigRequest = FALSE;
            }
        }
        else if (pDevProp->ulCurrentValue == 0)
        {
            if (m_bRotate)
            {
                m_bRotate = FALSE;
                // Assure that we will reconfigure IPU before starting.
                m_bCameraVfConfigRequest = FALSE;
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
            m_bCameraVfConfigRequest = FALSE;
     }

     if (ulCaps & CS_VideoControlFlag_FlipVertical)
     {
            // Flip turned from OFF to ON state
            m_bFlipVertical = m_bFlipVertical ? FALSE : TRUE;
            DEBUGMSG(ZONE_DEVICE, (_T("CAM_IOControl(%08x): Just toggled Vert Flip.\r\n"), this));
            // Assure that we will reconfigure IPU before starting.
            m_bCameraVfConfigRequest = FALSE;
     }
     
    return ERROR_SUCCESS;
}

DWORD CSensorPdd :: SetPowerState( CEDEVICE_POWER_STATE PowerState )
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
                #if 0
                    m_pSMFC->DumpSMFCRegs();
                    m_pCsi0->DumpCSIRegs();
                #endif
                
                // For power management purposes
                // Befor enable csi and smfc/pp,turn clock on for camera
                BSPEnableCamera(m_CSIInterface);

                // For Suspend,when IPU check all submodule disabled,
                // It will turn ipu clk off
                // For Resume,if there is no submodule enable,
                // Ipu clk will not turn on
                // Enable SMFC for turn on IPU clk
                m_pSMFC->SMFCEnable();

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
                #if 0
                    m_pSMFC->DumpSMFCRegs();
                    m_pCsi0->DumpCSIRegs();
                #endif

                // Disable CSI,SMFC/PP in SetSensorState(CSSTATE_STOP)
                // Turn sensor clock off
                BSPDisableCamera(m_CSIInterface);   
 
                // Suspend will lost the values for CSI/SMFC/PP registers for TO2.
                // So when Resume need reconfig these registers.
                m_bCameraEncConfig = FALSE;               

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
       DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:m_ppModeContext[%d] = %x , count = %x \r\n"),
                                __WFUNCTION__,i,m_ppModeContext[i],count));
    }

    if( count == m_ulCTypes )
    {  
        SetPowerState(D0);
    }

    ASSERT( ModeContext );
    m_ppModeContext[ulModeType] = ModeContext;
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:m_ppModeContext[%d] = %x , \r\n"),
                __WFUNCTION__,ulModeType,m_ppModeContext[ulModeType]));

    // Perform one-time configuration tasks
    // Get display characteristics for SMFCClass.
    // Only perform once.
    if( !displayInitialized )
    {
        m_pSMFC->SMFCInitDisplayCharacteristics();
        displayInitialized = TRUE;
    }

    // Only Initialize sensor once.
    if( !m_SensorConfigured )
    {
        if(m_pCsi0 != NULL)
            m_pCsi0->CsiConfigureSensor(m_dwSensorFramerate);
        else if(m_pCsi1 != NULL)
            m_pCsi1->CsiConfigureSensor(m_dwSensorFramerate);
        m_SensorConfigured = TRUE;
    }

    // Only preview pin need PP module
    if (ulModeType == PREVIEW)
    {
        // Create PP handle
        if(!m_pCameraPP->CameraPPOpenHandle())
        {
            ERRORMSG(TRUE,(TEXT("%s:Creating PP handle failed!\r\n"),__WFUNCTION__));
            return ERROR_INVALID_FUNCTION;
        }
    }
        
    return ERROR_SUCCESS;
}

DWORD CSensorPdd::DeInitSensorMode( ULONG ulModeType )
{
    DWORD i = 0;
    DWORD count = 0;    

    SensorClosePin(ulModeType);
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:m_ppModeContext[%d] = %x \r\n"),
                            __WFUNCTION__,ulModeType,m_ppModeContext[ulModeType]));

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

        m_SensorConfigured = FALSE;
        
        m_bCameraVfConfigRequest = FALSE;
        m_bCameraEncConfigRequest  = FALSE;
        
        // Close PP handle
        if(!m_pCameraPP->CameraPPCloseHandle())
        {
            ERRORMSG(TRUE,(TEXT("%s:Creating PP handle failed!\r\n"),__WFUNCTION__));
            return ERROR_INVALID_FUNCTION;
        }
    }

    return ERROR_SUCCESS;
}

DWORD CSensorPdd::SetSensorState( ULONG ulModeType, CSSTATE csState )
{
    DWORD dwError = ERROR_SUCCESS;
    LPVOID ModeContext=NULL;
    CSSTATE PreCsState = CSSTATE_STOP;
 
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
                // PREVIEW and CAPTURE not using timer
                // so not return the error
                #if 0
                dwError = ERROR_INVALID_PARAMETER;
                #endif
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

            // Now SMFC for capture pin, SMFC&PP for preview pin 
            // So when stop capture pin,if preview pin running it cannot stop all channel
            // Or when stop preview pin,if capture pin running it cannot stop all channel  
            if( ulModeType == PREVIEW )
            {
                if(!SensorStopChannel(PREVIEW))
                {
                    ERRORMSG(TRUE,(TEXT("SensorStopChannel preview error\r\n")));
                    break;
                }

                // if stop preview pin, we need to check capture pin not running
                if( (m_CsState[CAPTURE] != CSSTATE_PAUSE) &&
                    (m_CsState[CAPTURE] != CSSTATE_RUN) )
                {
                    // it means NO capture pin enable by client
                    // we need stop capture by ourselves
                    if(!SensorStopChannel(CAPTURE))
                    {
                        ERRORMSG(TRUE,(TEXT("SensorStopChannel capture error\r\n")));
                        break;
                    }
                }
            }
            else if( ulModeType == CAPTURE )
            {
                // if stop capture pin, we need to check preview pin not running
                if( (m_CsState[PREVIEW] != CSSTATE_PAUSE) &&
                    (m_CsState[PREVIEW] != CSSTATE_RUN) )
                {
                    if(!SensorStopChannel(CAPTURE))
                    {
                        ERRORMSG(TRUE,(TEXT("SensorStopChannel capture error\r\n")));
                        break;
                    }
                }  
            }

            #if 0 
            if( PREVIEW == ulModeType)
            {
                // Close PP handle
                if(!m_pCameraPP->CameraPPCloseHandle())
                {
                    ERRORMSG(TRUE,(TEXT("%s:Closing PP handle failed!\r\n"),__WFUNCTION__));
                    break;
                }
            }
            #endif
            
            break;

        case CSSTATE_PAUSE:

            PreCsState = m_CsState[ulModeType];
            m_CsState[ulModeType] = CSSTATE_PAUSE;
           
            if( STILL == ulModeType )
            {
                 dwError = ERROR_SUCCESS;
                 break;
            }

            if( PreCsState == CSSTATE_STOP)
            {
                if( PREVIEW == ulModeType )
                { 
                    #if 0 
                    //Only preview pin open PP handle
                    // Create PP handle
                    if(!m_pCameraPP->CameraPPOpenHandle())
                    {
                        ERRORMSG(TRUE,(TEXT("%s:Creating PP handle failed!\r\n"),__WFUNCTION__));
                        return FALSE;
                    }
                    #endif
    
                    // if CAPTURE pin is not in PAUSE or RUN state, it means user only enable PREVIEW pin
                    // in this case, we need config and allocate buffer for SMFC
                    if( (m_CsState[CAPTURE] != CSSTATE_PAUSE) &&
                        (m_CsState[CAPTURE] != CSSTATE_RUN) )
                    {
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("Capture PIN must start before PREVIEW if you want to enable both m_CsState[CAPTURE] = %d\r\n"),
                                                  m_CsState[CAPTURE]));
                        //using the default value to setting enc
                        // Calculate the SMFC and PP parameters
                        if( !SensorEncConfigRequest(CAPTURE, TRUE) )
                        {
                           ERRORMSG(TRUE,(TEXT("%s:SensorEncConfigRequest fail\r\n"),__WFUNCTION__));
                           dwError = ERROR_ACCESS_DENIED;
                        }
                    
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("PREVIEW config enc setting for CAPTURE\r\n")));
                        //CAPTURE pin must run before preview
                        // otherwise we need allocate buffer ourself
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("PREVIEW alloc smfc buffer for CAPTURE: m_bAllocateBufferForSMFC-%x,m_bSensorBufferAllocByDriver%x \r\n")
                                        ,m_bAllocateBufferForSMFC,m_bSensorBufferAllocByDriver));

                        if(!m_bSensorBufferAllocByDriver)
                        {
                            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: PREVIEW calls SMFCAllocateBuffers for CAPTURE, bufsize is %x, bufnum is %x\r\n"),__WFUNCTION__,m_ulSMFCBufSize,m_ulNumOfBuffer));

                            // If capture pin buffer was not allocated, they will be allocated forcely by preview pin here.
                            if(!m_bAllocateBufferForSMFC)
                            {
                                if (!m_pSMFC->SMFCAllocateBuffers(m_pCaptureBufferManager,m_ulNumOfBuffer, m_ulSMFCBufSize))
                                {
                                    ERRORMSG(TRUE,(_T("%s: SMFC Buffer allocation failed.\r\n"), __WFUNCTION__));
                                    return FALSE;
                                }
                            }
                            else
                            {
                                RETAILMSG(1,(TEXT("%s: Capture pin buffer has been allocated by capture pin.\r\n"),__WFUNCTION__));
                            }

                            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: PREVIEW calls SMFCAllocateBuffers for CAPTURE\r\n"),__WFUNCTION__));
                            m_bSensorBufferAllocByDriver = TRUE;
                            DEBUGMSG(ZONE_FUNCTION,(TEXT("PREVIEW allocate buffer for CAPTURE\r\n")));
                        }
                    }

                    // whatever, we need set PP input address
                    if (!m_pSMFC->SMFCGetAllocBufPhyAddr(m_pCaptureBufferManager,m_pCaptureBufferManager->GetCurrentBufferNum(), m_pSMFCAllocPhyAddr))
                    {
                        ERRORMSG(TRUE,(_T("%s: SMFC Buffer allocation failed.\r\n"), __WFUNCTION__));
                        return FALSE;
                    }

                    if (!m_pCameraPP->CameraPPSetInputBufPhyAddr(m_pCaptureBufferManager->GetCurrentBufferNum(), m_pSMFCAllocPhyAddr))
                    {
                        ERRORMSG(TRUE,(_T("%s: CameraPP set input Buffer address failed.\r\n"), __WFUNCTION__));
                        return FALSE;
                    }

                    //----------------------------------------------------
                    // Configure viewfinding path if Preview pin is used
                    // and configuration is needed.
                    // First time,rotate,flip,change resolution need config
                    //----------------------------------------------------
                    if( !m_bCameraVfConfigRequest )
                    {
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:PREVIEW CSSTATE_PAUSE Begin!\r\n"),__WFUNCTION__)); 

                        // Calculate the SMFC and PP parameters
                        if( !SensorVfConfigRequest(PREVIEW) )
                        {
                           ERRORMSG(TRUE,(TEXT("%s:SensorVfConfigRequest fail\r\n"),__WFUNCTION__));
                           dwError = ERROR_ACCESS_DENIED;
                        }

                        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:PREVIEW CSSTATE_PAUSE called SensorViewfindingConfig() Finished!\r\n"),__WFUNCTION__)); 
                    }          
                }
                else if ( CAPTURE == ulModeType )
                {
                    if( !m_bCameraEncConfigRequest  )
                    {
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:CAPTURE CSSTATE_PAUSE Begin!\r\n"),__WFUNCTION__)); 

                        // Calculate the SMFC parameters
                        if( !SensorEncConfigRequest(CAPTURE, FALSE) )
                        {
                            ERRORMSG(TRUE,(TEXT("%s:SensorEncConfigRequest fail\r\n"),__WFUNCTION__));
                            dwError = ERROR_ACCESS_DENIED;
                        }
                    
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:CAPTURE CSSTATE_PAUSE called SensorEncConfigRequest() Finished!\r\n"),__WFUNCTION__)); 
                    }
                }
            }
            else
            {
                #if 0
                // If CsCstate switch to PAUSE form RUN
                if( PREVIEW == ulModeType )
                {
                    if(!SensorStopChannel(PREVIEW))
                    {
                        ERRORMSG(TRUE,(TEXT("SensorStopChannel capture error\r\n")));
                        break;
                    }

                    if( (m_CsState[CAPTURE] == CSSTATE_STOP) && (m_CsState[STILL] == CSSTATE_STOP) )
                    {
                        if(!SensorStopChannel(CAPTURE))
                        {
                            ERRORMSG(TRUE,(TEXT("SensorStopChannel capture error\r\n")));
                            break;
                        }
                    }
                }
                else if ( CAPTURE == ulModeType )
                {
                    if( (m_CsState[PREVIEW] == CSSTATE_STOP) )
                    {
                        if(!SensorStopChannel(CAPTURE))
                        {
                            ERRORMSG(TRUE,(TEXT("SensorStopChannel capture error\r\n")));
                            break;
                        }
                    }
                }
                #endif
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
                 
            if( PREVIEW == ulModeType )
            {                          
                // Config SMFC and PP 
                if( m_CsState[CAPTURE] != CSSTATE_RUN )
                {
                    // Config SMFC
                    if (!SensorEncodingConfig())
                    {
                        ERRORMSG(TRUE,(TEXT("%s:SensorViewfindingConfig fail\r\n"),__WFUNCTION__)); 
                        dwError = ERROR_ACCESS_DENIED;
                    }
                    DEBUGMSG(ZONE_FUNCTION,(TEXT("SensorEncodingConfig for enc in preview\r\n")));
                }
                
                if (!SensorViewfindingConfig())
                {
                    ERRORMSG(TRUE,(TEXT("%s:SensorViewfindingConfig fail\r\n"),__WFUNCTION__)); 
                    dwError = ERROR_ACCESS_DENIED;
                }
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:PREVIEW CSSTATE_RUM called SensorViewfindingConfig() Finished\r\n"),__WFUNCTION__));                

                if( m_CsState[CAPTURE] != CSSTATE_RUN )
                {
                    // at this time, we need startchannel also
                    if ( !SensorStartChannel(CAPTURE) )
                    {
                        break;
                    }
                }

                // Data flow is Sensor->CSI->SMFC->PP
                // Enable PP
                if ( !SensorStartChannel(PREVIEW) )
                {
                    break;
                }
                
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:PREVIEW CSSTATE_RUM called SensorStartChannel() Finished\r\n"),__WFUNCTION__));

                SetEvent(m_hPinVfEvent[0]);
                DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::SensorStartChannel set PREVIEW begin 0x%x\r\n"),m_hPinVfEvent[0]));
                
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:PREVIEW CSSTATE_RUN Finished !\r\n"),__WFUNCTION__));
            }
            else if ( CAPTURE == ulModeType )
            {  
                // if preview pin is runing, capture chanel is running
                if( m_CsState[PREVIEW] != CSSTATE_RUN )
                {
                    // Config SMFC
                    if (!SensorEncodingConfig())
                    {
                        ERRORMSG(TRUE,(TEXT("%s:SensorViewfindingConfig fail\r\n"),__WFUNCTION__)); 
                        dwError = ERROR_ACCESS_DENIED;
                    }
                
                    // Enable SMFC/CSI
                    if ( !SensorStartChannel(CAPTURE) )
                    {
                        break;
                    }
                }
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:CAPTURE CSSTATE_RUM called SensorStartChannel() Finished\r\n"),__WFUNCTION__));

                SetEvent(m_hPinEncEvent[0]);            
                DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::SensorStartChannel set CAPTURE begin 0x%x\r\n"),m_hPinEncEvent[0]));

                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:CAPTURE CSSTATE_RUN\r\n"),__WFUNCTION__));
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
    DWORD result;

    pSensorModeInfo->MemoryModel = m_SensorModeInfo[ulModeType].MemoryModel;
    pSensorModeInfo->MaxNumOfBuffers = m_SensorModeInfo[ulModeType].MaxNumOfBuffers;
    pSensorModeInfo->PossibleCount = m_SensorModeInfo[ulModeType].PossibleCount;
    pSensorModeInfo->VideoCaps.DefaultVideoControlCaps = DefaultVideoControlCaps[ulModeType];
    pSensorModeInfo->VideoCaps.CurrentVideoControlCaps = m_pModeVideoCaps[ulModeType].CurrentVideoControlCaps;

    result = WaitForSingleObject(m_hFslFilter,1);
    
    if(result == WAIT_OBJECT_0)
    {   
        //if connected filter is  freescale filter, we will support all format
        if(ulModeType == CAPTURE)
        {
            m_pModeVideoFormat[CAPTURE].ulAvailFormats = 28;
        }

        if(ulModeType == STILL)
        {
            m_pModeVideoFormat[STILL].ulAvailFormats = 15;
        }
    }
    else
    {
        //if connected filter isn't  freescale filter, we only support limited format
        if(ulModeType == CAPTURE)
        {
            m_pModeVideoFormat[CAPTURE].ulAvailFormats = 14;
        }

        if(ulModeType == STILL)
        {
            m_pModeVideoFormat[STILL].ulAvailFormats = 5;
        }
    }

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

    // If Pin is running currently, we will caculate the real-time  framerate 
    if( m_CsState[ulModeType] == CSSTATE_RUN )
    {
        totalframeCount = lastframeCount = curframeCount = 0;
        dwTickCount = GetTickCount();

        maxframeCount = -1;

        if (ulModeType != STILL)
        {
            if(ulModeType == PREVIEW)
                curframeCount = m_pCameraPP->CameraPPGetFrameCount();
            else
                curframeCount = m_pSMFC->SMFCGetFrameCount();
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

            if(ulModeType == PREVIEW)
                curframeCount = m_pCameraPP->CameraPPGetFrameCount();
            else
                curframeCount = m_pSMFC->SMFCGetFrameCount();

        }

        return dwTickCount2;
    }
    // If Pin isn't running currently, we just return the default frame rate
    else
    {
        if (ulModeType != STILL)
        {
            return 10000000/m_dwSensorFramerate;
        }
        else
        {
            return 0;
        }
    }
}

DWORD CSensorPdd::SetSensorFrameRate(DWORD dwFramerate)
{
    if(dwFramerate == m_dwSensorFramerate)
    {
        return m_dwSensorFramerate;
    }

    if( (dwFramerate > 0) && (dwFramerate <=15) )
        m_dwSensorFramerate = 15;
    else if( (dwFramerate > 15) && (dwFramerate <=30) )
        m_dwSensorFramerate = 30;
    else
        m_dwSensorFramerate = 30;

    if( !m_SensorConfigured )
    {
        return m_dwSensorFramerate;
    }
    
    if(m_pCsi0 != NULL)
        m_pCsi0->CsiChangeFrameRate(m_dwSensorFramerate);  
    else if(m_pCsi1 != NULL)
        m_pCsi1->CsiChangeFrameRate(m_dwSensorFramerate);    

    m_bCameraEncConfig = FALSE;

    return m_dwSensorFramerate;
}

PVOID CSensorPdd::AllocateBuffer( ULONG ulModeType )
{  
    // Real PDD may want to save off this allocated pointer
    // in an array.
    ULONG ulFrameSize = abs(CS_DIBSIZE (m_pCurrentFormat[ulModeType].VideoInfoHeader.bmiHeader));
    // at the same time, we will inform the downstream module to allocate memory too.
    switch(ulModeType)
    {
        case PREVIEW:
            if(m_pCameraPP != NULL && !m_bAllocateBufferForPP)
            {
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: CameraPP AllocateBuffers:ulFrameSize %x \r\n"),
                                        __WFUNCTION__,ulFrameSize));
                // for the issue: new buffer manager
                if (!m_pCameraPP->CameraPPAllocateBuffers(m_pPreviewBufferManager, m_ulNumOfBuffer, ulFrameSize))
                {
                    ERRORMSG(TRUE,(_T("%s: CameraPP Buffer allocation failed.\r\n"), __WFUNCTION__));
                    return NULL;
                }
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: CameraPPAllocateBuffers finished!\r\n"),__WFUNCTION__));
                m_bAllocateBufferForPP = TRUE;
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Preview pin calls PPAllocateBuffers,m_pPreviewBufferManager is %x, bufsize is %x, bufnum is %x\r\n"),
                                        __WFUNCTION__,m_pPreviewBufferManager,ulFrameSize,m_ulNumOfBuffer));
            }
            break;
            
        case CAPTURE:
            if(!m_bAllocateBufferForSMFC)
            {
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: SMFC AllocateBuffers:m_ulSMFCBufSize %x \r\n"),
                                        __WFUNCTION__,ulFrameSize));

                // Check if buffer of capture pin has been allocated forcely by preview pin 
                if( !m_bSensorBufferAllocByDriver )
                {
                    if (!m_pSMFC->SMFCAllocateBuffers(m_pCaptureBufferManager, m_ulNumOfBuffer, ulFrameSize))
                    {
                        ERRORMSG(TRUE,(_T("%s: SMFC Buffer allocation failed.\r\n"), __WFUNCTION__));
                        return NULL;
                    }
                    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: SMFC AllocateBuffers finished!\r\n"),__WFUNCTION__));

                    if (!m_pSMFC->SMFCGetAllocBufPhyAddr(m_pCaptureBufferManager,m_ulNumOfBuffer, m_pSMFCAllocPhyAddr))
                    {
                        ERRORMSG(TRUE,(_T("%s: SMFC Buffer allocation failed.\r\n"), __WFUNCTION__));
                        return FALSE;
                    }

                    for (UINT32 i = 0; i < m_ulNumOfBuffer; i++)
                    {
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: m_pSMFCAllocPhyAddr[%d] = %x"),
                                            __WFUNCTION__,i,m_pSMFCAllocPhyAddr[i]));
                    }

                    if(m_bCameraPPEnable)
                    {
                        if (!m_pCameraPP->CameraPPSetInputBufPhyAddr(m_ulNumOfBuffer, m_pSMFCAllocPhyAddr))
                        {
                            ERRORMSG(TRUE,(_T("%s: CameraPP set input Buffer address failed.\r\n"), __WFUNCTION__));
                            return FALSE;
                        }
                    }

                }
                else
                {
                    RETAILMSG(1,(TEXT("%s: Capture pin buffer has been allocated forcely by preview pin.\r\n"),__WFUNCTION__));
                }
            
                m_bAllocateBufferForSMFC = TRUE;

                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: CAPTURE pin calls SMFCAllocateBuffers,m_pCaptureBufferManager is %x, bufsize is %x, bufnum is %x\r\n"),
                                        __WFUNCTION__,m_pCaptureBufferManager,ulFrameSize,m_ulNumOfBuffer));
            }
            break;

        case STILL:
            if(!m_bAllocateBufferForSTILL)
            {
                if (!m_pSMFC->SMFCAllocateBuffers(m_pStillBufferManager, m_ulNumOfBuffer, ulFrameSize))
                {
                    ERRORMSG(TRUE,(_T("%s: SMFC Buffer allocation failed.\r\n"), __WFUNCTION__));
                    return NULL;
                } 

                m_bAllocateBufferForSTILL = TRUE;

                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: STILL pin calls SMFCAllocateBuffers,m_pStillBufferManager is %x, bufsize is %x, bufnum is %x\r\n"),
                                        __WFUNCTION__,m_pStillBufferManager,ulFrameSize,m_ulNumOfBuffer));
            }    
            break;
    }
                
    return RemoteLocalAlloc( LPTR, ulFrameSize );
}

DWORD CSensorPdd::DeAllocateBuffer( ULONG ulModeType, PVOID pBuffer )
{
    // at the same time, we will inform the downstream module to free memory too
    switch(ulModeType)
    {
        case PREVIEW:
            if (m_bCameraPPEnable)
            {
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: CameraPP DeleteBuffers\r\n"),__WFUNCTION__));
                if (!m_pCameraPP->CameraPPDeleteBuffers(m_pPreviewBufferManager))
                {
                    ERRORMSG(TRUE,(_T("%s: CameraPP Buffer delete failed.\r\n"), __WFUNCTION__));
                    return ERROR_INVALID_PARAMETER;
                }
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: CameraPPDeleteBuffers finished!\r\n"),__WFUNCTION__));
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: PREVIEW pin calls PPDeleteBuffers\r\n"),__WFUNCTION__));
            }
            m_bAllocateBufferForPP = FALSE;

            // If capturen pin buffer was allocated by preview pin
            if(m_bSensorBufferAllocByDriver)
            {
                // if capture pin is enable, then this buffer will be continuely used by capture pin.
                if(!m_bAllocateBufferForSMFC)
                {
                    if (!m_pSMFC->SMFCDeleteBuffers(m_pCaptureBufferManager))
                    {
                        ERRORMSG(TRUE,(_T("%s: SMFC Buffer allocation failed.\r\n"), __WFUNCTION__));
                        return FALSE;
                    }
                }
                else
                {
                    RETAILMSG(1,(TEXT("%s: Capture pin buffer will be used continuely by capture pin.\r\n"),__WFUNCTION__));
                }

                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: SMFCDeleteBuffers finished!\r\n"),__WFUNCTION__));        
                m_bSensorBufferAllocByDriver = FALSE;
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: PREVIEW pin calls SMFCDeleteBuffers\r\n"),__WFUNCTION__));
            }
            break;
            
        case CAPTURE:
            if(m_bAllocateBufferForSMFC)
            {
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: SMFC SMFCDeleteBuffers\r\n"),__WFUNCTION__));

                // Check if buffer of capture pin will be continuely used by preview pin.
                if( !m_bSensorBufferAllocByDriver )
                {
                    if (!m_pSMFC->SMFCDeleteBuffers(m_pCaptureBufferManager))
                    {
                        ERRORMSG(TRUE,(_T("%s: SMFC Buffer delete failed.\r\n"), __WFUNCTION__));
                        return ERROR_INVALID_PARAMETER;
                    }
                }
                else
                {
                    RETAILMSG(1,(TEXT("%s: Capture pin buffer will be used by preview pin.\r\n"),__WFUNCTION__));
                }

                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: SMFC SMFCDeleteBuffers finished!\r\n"),__WFUNCTION__));
                m_bAllocateBufferForSMFC = FALSE;

                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: CAPTURE pin calls SMFCDeleteBuffers\r\n"),__WFUNCTION__));                    
            }
            break;

        case STILL:    
            if(m_bAllocateBufferForSTILL)
            {
                if (!m_pSMFC->SMFCDeleteBuffers(m_pStillBufferManager))
                {
                    ERRORMSG(TRUE,(_T("%s: SMFC Buffer delete failed for Still Pin.\r\n"),__WFUNCTION__));
                    return ERROR_INVALID_PARAMETER;
                }

                m_bAllocateBufferForSTILL = FALSE;

                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: STILL pin calls SMFCDeleteBuffers\r\n"),__WFUNCTION__));
            }
            break;
    }
    
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
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SensorPdd RegisterClientBuffer Get PHY address = 0x%x\r\n"),dwPhyAddr));
    DWORD dwRet = ERROR_INVALID_PARAMETER;

    // for the issue: new buffer manager
    switch(ulModeType)
    {
        case PREVIEW:
            dwRet = m_pCameraPP->RegisterBuffer(m_pPreviewBufferManager, pBuffer, dwPhyAddr)? ERROR_SUCCESS : ERROR_INVALID_PARAMETER ;
            break;

        case CAPTURE:
            dwRet = m_pSMFC->RegisterBuffer(m_pCaptureBufferManager, pBuffer, dwPhyAddr)? ERROR_SUCCESS : ERROR_INVALID_PARAMETER ;
            break;

        case STILL:
            dwRet = m_pSMFC->RegisterBuffer(m_pStillBufferManager, pBuffer, dwPhyAddr)? ERROR_SUCCESS : ERROR_INVALID_PARAMETER ;
            break;
    }

    return dwRet;
}

DWORD CSensorPdd::UnRegisterClientBuffer( ULONG ulModeType, PVOID pBuffer )
{
    // DShow is not going to use pBuffer (which was originally allocated by DShow) anymore. If the PDD
    // is keeping a cached pBuffer pointer (in RegisterClientBuffer()) then this is the right place to
    // stop using it and maybe set the cached pointer to NULL. 
    // Note: PDD must not delete this pointer as it will be deleted by DShow itself
    ULONG ulFrameSize = abs(CS_DIBSIZE (m_pCurrentFormat[ulModeType].VideoInfoHeader.bmiHeader));
    DWORD dwPhyAddr = GetPhysAddress(pBuffer,ulFrameSize);
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    
    DEBUGMSG(ZONE_FUNCTION,(TEXT("SensorPdd UnRegisterClientBuffer Get PHY address = 0x%x\r\n"),dwPhyAddr));
    
    // when we unregister client buffer, we need to check if we have allocate buffer for smfc 
    if(m_bSensorBufferAllocByDriver)
    {
        // if those buffer were not used by capture pin.
        if(!m_bAllocateBufferForSMFC)
        {
            if (!m_pSMFC->SMFCDeleteBuffers(m_pCaptureBufferManager))
            {
                ERRORMSG(TRUE,(_T("%s: SMFC Buffer allocation failed.\r\n"), __WFUNCTION__));
                return FALSE;
            }
        }
        else
        {
            RETAILMSG(1,(TEXT("%s: Capture pin buffer will be used continuely by capture pin.\r\n"),__WFUNCTION__));
        }

        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: SMFCDeleteBuffers finished!\r\n"),__WFUNCTION__));
        m_bSensorBufferAllocByDriver = FALSE;
    }

    // for the issue: new buffer manager
    //then unregister
    switch(ulModeType)
    {
        case PREVIEW:
            dwRet =  m_pCameraPP->UnregisterBuffer(m_pPreviewBufferManager, pBuffer, dwPhyAddr) ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER ;
            break;

        case CAPTURE:
            dwRet = m_pSMFC->UnregisterBuffer(m_pCaptureBufferManager, pBuffer, dwPhyAddr) ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER ;
            break;

        case STILL:
            dwRet = m_pSMFC->UnregisterBuffer(m_pStillBufferManager, pBuffer, dwPhyAddr) ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER ;
            break;
    }

    return dwRet;
}

DWORD CSensorPdd::EnqueueBuffer( ULONG ulModeType, PVOID pBuffer )
{
    // DShow is not going to use pBuffer (which was originally allocated by DShow) anymore. If the PDD
    // is keeping a cached pBuffer pointer (in RegisterClientBuffer()) then this is the right place to
    // stop using it and maybe set the cached pointer to NULL. 
    // Note: PDD must not delete this pointer as it will be deleted by DShow itself
    ULONG ulFrameSize = abs(CS_DIBSIZE (m_pCurrentFormat[ulModeType].VideoInfoHeader.bmiHeader));
    DWORD dwPhyAddr = GetPhysAddress(pBuffer,ulFrameSize);
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    
    // In fact, CMagna should manger some component, e.g. SMFC, PP...
    // Here it should according to ulModeType to judge it should call SMFC->enqueue or PP->enqueue.
    switch(ulModeType)
    {
        case PREVIEW:
            dwRet =  m_pCameraPP->Enqueue(m_pPreviewBufferManager) ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER ;
            break;

        case CAPTURE:
            dwRet =  m_pSMFC->Enqueue(m_pCaptureBufferManager) ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER ;
            break;

        case STILL:
            dwRet =  m_pSMFC->Enqueue(m_pStillBufferManager) ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER ;
            break;
    }

    return dwRet;
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
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("PIN_Function(%08x): +FillBuffer\r\n"), this));

    while (1)
    {
        switch (ulModeType)
        {
            case PREVIEW:
                pbySrcBuffer = (UINT8 *)m_pCameraPP->CameraPPGetBufFilled();

                if (pbySrcBuffer == NULL)
                {
                    if (WaitForSingleObject(m_pCameraPP->m_hCameraPPEOFEvent, 1000) != WAIT_OBJECT_0) 
                    {
                        ERRORMSG(TRUE, (TEXT("FillBuffer:WaitForSingleObject m_hCameraPPEOFEvent timeout! Maybe something wrong...\r\n")));

                        //We have found root cause about why CSI/SMFC/PP will be blocked, so disable this workaround
                        #if 0
                        // If we can not receive anything , some times CSI/SMFC/PP is blocked ,so just restart it.
                        // It is not a good way,but it works.
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Wait for Frame time out for PREVIEW,Reset SMFC/PP/CSI\r\n"),__WFUNCTION__));

                        SensorStopChannel(PREVIEW);
                        SensorStopChannel(CAPTURE);

                        // ReConfig SMFC and PP 
                        SensorViewfindingConfig();
                        // ReConfig SMFC
                        SensorEncodingConfig();
                        
                        Sleep(100);
                        SensorStartChannel(CAPTURE);
                        SensorStartChannel(PREVIEW);
                        #endif

                        return biSizeImage;

                    }
                    pbySrcBuffer = (UINT8 *) m_pCameraPP->CameraPPGetBufFilled();
                }
                else
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("FillBuffer: PREVIEW filled buffer ready!\r\n")));
                }

                break;
                
            case CAPTURE:

                pbySrcBuffer = (UINT8 *)m_pSMFC->SMFCGetBufFilled();

                if (pbySrcBuffer == NULL)
                {
                    if (WaitForSingleObject(m_pSMFC->m_hCameraSMFCEOFEvent, 1000) != WAIT_OBJECT_0) 
                    {
                        ERRORMSG(TRUE, (TEXT("FillBuffer:WaitForSingleObject m_hCameraSMFCEOFEvent timeout! Maybe something wrong...\r\n")));

                        //We have found root cause about why CSI/SMFC/PP will be blocked, so disable this workaround
                        #if 0
                        // If we can not receive anything , some times CSI/SMFC/PP is blocked ,so just restart it.
                        // It is not a good way,but it works.
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Wait for Frame time out for CAPTURE,Reset SMFC/CSI\r\n"),__WFUNCTION__));
                        SensorStopChannel(CAPTURE);

                        // ReConfig SMFC
                        SensorEncodingConfig();
                        
                        Sleep(100);

                        SensorStartChannel(CAPTURE);
                        #endif

                        return biSizeImage;
                            
                    }
                    pbySrcBuffer = (UINT8 *) m_pSMFC->SMFCGetBufFilled();
                }
                else
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("FillBuffer: CAPTURE filled buffer ready!\r\n")));
                }

                break;
            
            case STILL:

                pbySrcBuffer = (UINT8 *)m_pSMFC->SMFCGetBufFilled();

                if (pbySrcBuffer == NULL)
                {
                    if (WaitForSingleObject(m_pSMFC->m_hCameraSMFCEOFEvent, 1000) != WAIT_OBJECT_0) 
                    {
                        ERRORMSG(TRUE, (TEXT("FillBuffer:WaitForSingleObject m_hCameraSMFCEOFEvent timeout! Maybe something wrong...\r\n")));

                        //We have found root cause about why CSI/SMFC/PP will be blocked, so disable this workaround
                        #if 0
                        // If we can not receive anything , some times CSI/SMFC/PP is blocked ,so just restart it.
                        // It is not a good way,but it works.
                        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Wait for Frame time out for STILL,Reset SMFC/CSI\r\n"),__WFUNCTION__));
                        SensorStopChannel(STILL);

                        // ReConfig SMFC
                        SensorEncodingConfig();
                        
                        Sleep(100);

                        SensorStartChannel(STILL);
                        #endif

                        return biSizeImage;
                    }
                    pbySrcBuffer = (UINT8 *) m_pSMFC->SMFCGetBufFilled();
                }
                else
                {
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("FillBuffer: STILL filled buffer ready!\r\n")));
                }

                break;

            default:

                DEBUGMSG(ZONE_ERROR,(_T("FillBuffer: Invalid pin id.\r\n"))) ;

                return 0;
        }

        // If we got a buffer from SMFC or PP, exit loop and continue.
        // If there was no buffer returned, loop again and wait for one.
        DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::FillBuffer 0x%x\r\n"),pbySrcBuffer));
        if (pbySrcBuffer != NULL)
        {
            break;
        }
        
        DEBUGMSG(ZONE_FUNCTION, (TEXT("FillBuffer: pbySrcBuffer NULL!  Waiting again...\r\n")));
    }
   
    pbyDstBuffer =  reinterpret_cast<PUINT8>(pImage);

    memcpy(pbyDstBuffer, pbySrcBuffer, biSizeImage);
    
    //CacheRangeFlush(pbySrcBuffer,biSizeImage,CACHE_SYNC_DISCARD);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("FillBuffer:buffer filled success!\r\n")));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-FillBuffer\r\n")));

    // return the size of the image filled
    return(biSizeImage); 
}

DWORD CSensorPdd::FillBufferEx( ULONG ulModeType, PUCHAR* ppImage )
{
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr = &m_pCurrentFormat[ulModeType].VideoInfoHeader;
    UINT biSizeImage    = pCsVideoInfoHdr->bmiHeader.biSizeImage;
    ULONG ulPinId       = CAPTURE;
    
    RETAILMSG(0, (TEXT("CSensorPdd(%08x): +FillBufferEx\r\n"), this));

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
                *ppImage = (UINT8 *)m_pCameraPP->CameraPPGetBufFilled();

                break;
            case CAPTURE:
                *ppImage = (UINT8 *)m_pSMFC->SMFCGetBufFilled();

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
        RETAILMSG(0, (TEXT("+FillBuffer for %d:buffer filled success address = 0x%x\r\n"),ulPinId,*ppImage));
        return(biSizeImage);    
    }
    else
    {
        RETAILMSG(1, (TEXT("+FillBufferEx: pbySrcBuffer NULL!  something wrong...\r\n")));
        return 0;
    } 
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

BOOL CSensorPdd::ReadCSIInterfaceFromRegistry()
{
    HKEY  hKey = 0;
    DWORD dwType  = 0;
    DWORD dwSize  = sizeof ( DWORD );
    DWORD dwValue = -1;

    if( ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"Drivers\\BuiltIn\\Camera", 0, 0, &hKey ))
    {
        return FALSE;
    }

    if ( ERROR_SUCCESS == RegQueryValueEx( hKey, L"CSIInterface", 0, &dwType, (BYTE *)&dwValue, &dwSize ) )
    {
        if ( REG_DWORD == dwType && sizeof ( DWORD ) == dwSize)
        {
            switch(dwValue)
            {
                case 0:
                    m_CSIInterface = CSI_SELECT_CSI0;
                    break;

                case 1:
                    m_CSIInterface = CSI_SELECT_CSI1;
                    break;

                case 2:
                    //reserved for dual camera support
                    m_CSIInterface = CSI_SELECT_CSIAll;
                    break;
                    
                default:
                    RETAILMSG(1,(TEXT("Read CSIInterface Registry error in ReadCSIInterfaceFromRegistry! \r\n")));
                    return FALSE;
            }
        }
    }

    if ( ERROR_SUCCESS == RegQueryValueEx( hKey, L"CameraId", 0, &dwType, (BYTE *)&dwValue, &dwSize ) )
    {
        if ( REG_DWORD == dwType && sizeof ( DWORD ) == dwSize)
        {
            switch(dwValue)
            {
                case 0:
                case 1:
                case 2:
                case 3:
                    m_PrtclInf.mode = CSI_GATED_CLOCK_MODE;   //CSI work in GATE MODE
                    m_bCsiTstMode = FALSE;
                    break;

                case 4:
                case 5:
                    //reserved for TVin support
                    #if 0
                    //Tvin maybe interlace mode or progressive mode
                    m_PrtclInf.mode = CSI_CCIR_PROGRESSIVE_BT656_MODE,
                    m_PrtclInf.mode = CSI_CCIR_INTERLACE_BT656_MODE;
                    // Set the following parameters due to the interlace or progressive mode
                    m_PrtclInf.PreCmd;                        
                    m_PrtclInf.Field0FirstBlankStartCmd;      
                    m_PrtclInf.Field0FirstBlankEndCmd;        
                    m_PrtclInf.Field0SecondBlankStartCmd;     
                    m_PrtclInf.Field0SecondBlankEndCmd;       
                    m_PrtclInf.Field0ActiveStartCmd;          
                    m_PrtclInf.Field0ActiveEndCmd;
                    #endif
                    m_bCsiTstMode = FALSE;
                    break;

                case 9:
                    //reserved for CSI test mode
                    m_PrtclInf.mode = CSI_NONGATED_CLOCK_MODE;
                    m_bCsiTstMode = TRUE;
                    break;

                default:
                    RETAILMSG(1,(TEXT("Read CameraId Registry error in ReadCSIInterfaceFromRegistry! \r\n")));
                    return FALSE;
            }
        }
    }

    RegCloseKey( hKey );
    return TRUE;
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
// Function: SensorViewfindingConfig
//
// This function configures the viewfinding path of the PP module.
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
    if(m_bCameraVfConfig)
    {
        return TRUE;
        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Already set SensorViewfindingConfig\r\n"),__WFUNCTION__));
    }

    //----------------------------------------------------
    // Configure PP
    //----------------------------------------------------
    if (m_bCameraPPEnable)
    {
        if (!m_pCameraPP->CameraPPConfigure(&m_SMFCConfig,&m_PPConfig))
        {
            ERRORMSG(TRUE, (TEXT("%s: SMFC to PP configure failed.\r\n"), __WFUNCTION__));
            return FALSE;
        }

        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Config PP finish \r\n"),__WFUNCTION__));
    }

    m_bCameraVfConfig = TRUE;
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SensorEncodingConfig
//
// This function configures the encoding path of the CSI and SMFC module.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorEncodingConfig()
{
    if(m_bCameraEncConfig)
    {
        return TRUE;
        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Already set SensorEncodingConfig\r\n"),__WFUNCTION__));
    }
    
    /****************************************************************/
    // Configure CSI
    /****************************************************************/  
    if(m_pCsi0 != NULL)
    {
        if (!m_pCsi0->CsiConfigure(m_SensorOutputFormat, m_SensorOutputResolution, &m_PrtclInf,m_bCsiTstMode))
        {
            ERRORMSG(TRUE, 
                (TEXT("%s: Sensor configuration failed.  Aborting remaining configuration steps.\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }
    else if(m_pCsi1 != NULL)
    {
        if (!m_pCsi1->CsiConfigure(m_SensorOutputFormat, m_SensorOutputResolution, &m_PrtclInf, m_bCsiTstMode))
        {
            ERRORMSG(TRUE, 
                (TEXT("%s: Sensor configuration failed.  Aborting remaining configuration steps.\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Config CSI finish!\r\n"),__WFUNCTION__));
    
    //----------------------------------------------------
    // Configure SMFC to IDMAC
    //----------------------------------------------------
    switch(m_CSIInterface)
    {
        case CSI_SELECT_CSI0:
            if (!m_pSMFC->SMFCConfigureEncoding(IDMAC_CH_SMFC_CH0,CSI_SELECT_CSI0,CSI_SELECT_FRAME0,&m_SMFCConfig))
            {
                ERRORMSG(TRUE, (TEXT("%s: SMFC viewfinding configure failed.\r\n"), __WFUNCTION__));
                return FALSE;
            }
            break;

        case CSI_SELECT_CSI1:
            if (!m_pSMFC->SMFCConfigureEncoding(IDMAC_CH_SMFC_CH1,CSI_SELECT_CSI1,CSI_SELECT_FRAME0,&m_SMFCConfig))
            {
                ERRORMSG(TRUE, (TEXT("%s: SMFC viewfinding configure failed.\r\n"), __WFUNCTION__));
                return FALSE;
            }
            break;

        case CSI_SELECT_CSIAll:
            RETAILMSG(1,(TEXT("%s: Hardware cannot support CSI0 and CSI1 simultaneously!\r\n"),__WFUNCTION__));
            break;
            
        default:
            ERRORMSG(TRUE, (TEXT("%s: SMFC viewfinding configure failed.No CSI interface selected\r\n"), __WFUNCTION__));
            break;
    }

    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: Config SMFC finish!\r\n"),__WFUNCTION__));

    m_bCameraEncConfig = TRUE;
        
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SensorVfConfigRequest
//
// This function set the SMFC and PP module parameters for preview pin.
//
// Parameters:
//      ulPinId
//          [in] Specifies whether data format for PREVIEW pin or
//          STILL pin should be used to configure the viewfinding path.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorVfConfigRequest(ULONG ulPinId)
{
    DWORD dCameraPPFormat;
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr = &m_pCurrentFormat[ulPinId].VideoInfoHeader; 

    memset(&m_PPConfig, 0, sizeof(ppConfigData));
        
    /****************************************************************/
    // PP Setting
    /****************************************************************/
    dCameraPPFormat = pCsVideoInfoHdr->bmiHeader.biCompression;
    m_PPConfig.allowNopPP = TRUE;  

    if (m_bCameraPPEnable)
    {
        m_ulCameraPPBufSize = abs(CS_DIBSIZE(pCsVideoInfoHdr->bmiHeader)); // by bytes
            
        // Set PP output format    
        switch (dCameraPPFormat)
        {
            case (CS_BI_BITFIELDS|BI_SRCPREROTATE):
                m_PPConfig.outputIDMAChannel.FrameFormat = icFormat_RGB;
                m_PPConfig.outputIDMAChannel.DataWidth = icDataWidth_16BPP;
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Output Set icFormat_RGB\r\n"),__WFUNCTION__));
                break;

            case (CS_BI_RGB|BI_SRCPREROTATE):
                 m_PPConfig.outputIDMAChannel.FrameFormat = icFormat_RGB;
                 m_PPConfig.outputIDMAChannel.DataWidth = icDataWidth_24BPP;
                 break;

            case (FOURCC_YV12|BI_SRCPREROTATE):
                m_PPConfig.outputIDMAChannel.FrameFormat = icFormat_YUV420;
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Output Set icFormat_YUV420\r\n"),__WFUNCTION__));
                break;
                
            case (FOURCC_NV12|BI_SRCPREROTATE):
                m_PPConfig.outputIDMAChannel.FrameFormat = icFormat_YUV420P;
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Output Set icFormat_YUV420P\r\n"),__WFUNCTION__));
                break;  
                
            case (FOURCC_UYVY|BI_SRCPREROTATE):
                m_PPConfig.outputIDMAChannel.FrameFormat = icFormat_UYVY422;
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Output Set icFormat_UYVY422\r\n"),__WFUNCTION__));
                break;

            default:
                ERRORMSG(TRUE, (TEXT("%s: Invalid data format for preview.\r\n"), __WFUNCTION__));
                return FALSE;
        }

        // Support Rotation
        m_PPConfig.FlipRot.verticalFlip = m_bFlipVertical;
        m_PPConfig.FlipRot.horizontalFlip = m_bFlipHorizontal;
        m_PPConfig.FlipRot.rotate90 = m_bRotate;

        // Set PP output data size
        if (m_bRotate)
        {
            m_PPConfig.outputIDMAChannel.FrameSize.width = (UINT16)(abs(pCsVideoInfoHdr->bmiHeader.biHeight));
            m_PPConfig.outputIDMAChannel.FrameSize.height = (UINT16)pCsVideoInfoHdr->bmiHeader.biWidth;
        }
        else
        {
            m_PPConfig.outputIDMAChannel.FrameSize.width = (UINT16)pCsVideoInfoHdr->bmiHeader.biWidth;
            m_PPConfig.outputIDMAChannel.FrameSize.height = (UINT16)(abs(pCsVideoInfoHdr->bmiHeader.biHeight));            
        }
    }

    // Camera has been configured for viewfinding.
    m_bCameraVfConfigRequest = TRUE;
    m_bCameraVfConfig = FALSE;

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: SensorEncConfigRequest
//
// This function set the SMFC module parameters for CAPTURE pin or STILL pin.
//
// Parameters:
//      ulPinId
//          [in] Specifies whether data format for CAPTURE pin or
//          STILL pin should be used to configure the viewfinding path.
//      bDefault
//          [in] TRUE for only PREVIEW pin enable, need to get smfc parameters
//               FALSE for CAPTURE pin enable, to get enc parameters
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorEncConfigRequest(ULONG ulPinId, BOOL bDefault)
{
    DWORD dSMFCFormat = 0;
    DWORD sensorFormat, sensorResolution; 
    PCS_VIDEOINFOHEADER pCsVideoInfoHdr = &m_pCurrentFormat[ulPinId].VideoInfoHeader; 
    DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:ulPinId = %x, \npCsVideoInfoHdr = %x,  \nm_ulSMFCBufSize = %x, \nwidth = %x, \nheight = %x\r\n"),
                    __WFUNCTION__,ulPinId,pCsVideoInfoHdr,
                    abs(CS_DIBSIZE(pCsVideoInfoHdr->bmiHeader)),
                    (UINT16)pCsVideoInfoHdr->bmiHeader.biWidth,
                    (UINT16)(abs(pCsVideoInfoHdr->bmiHeader.biHeight))));  

    memset(&m_SMFCConfig, 0, sizeof(SMFCConfigData));

    // If height in bmiHeader is positive, the captured image should be bottom-up, so we vertically flip the image.
    if( abs(pCsVideoInfoHdr->bmiHeader.biHeight) == pCsVideoInfoHdr->bmiHeader.biHeight )
        BSPSensorFlip(TRUE);
    else
        BSPSensorFlip(FALSE);

    /****************************************************************/
    // SMFC Setting
    /****************************************************************/     
    if( !bDefault )//For capture pin enable
    {    
        dSMFCFormat = pCsVideoInfoHdr->bmiHeader.biCompression;
        m_ulSMFCBufSize = abs(CS_DIBSIZE(pCsVideoInfoHdr->bmiHeader)); // by bytes

        // Set SMFC output size
        m_SMFCConfig.outputSize.width =  (UINT16)pCsVideoInfoHdr->bmiHeader.biWidth;
        m_SMFCConfig.outputSize.height = (UINT16)(abs(pCsVideoInfoHdr->bmiHeader.biHeight));

        // Set SMFC output format 
        switch (dSMFCFormat)
        {
            case (CS_BI_BITFIELDS|BI_SRCPREROTATE):
                m_SMFCConfig.outputFormat = SMFCFormat_RGB565;
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Set SMFCFormat_RGB\r\n"),__WFUNCTION__));
                break;

            case (FOURCC_YV12|BI_SRCPREROTATE):
                m_SMFCConfig.outputFormat = SMFCFormat_YUV420;
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Set SMFCFormat_YUV420\r\n"),__WFUNCTION__));
                break;
                
            case (FOURCC_UYVY|BI_SRCPREROTATE):
                m_SMFCConfig.outputFormat = SMFCFormat_UYVY422;
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Set SMFCFormat_UYVY422\r\n"),__WFUNCTION__));
                break;
                
            case (FOURCC_NV12|BI_SRCPREROTATE):
                m_SMFCConfig.outputFormat = SMFCFormat_YUV420P;
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Set SMFCFormat_YUV420P\r\n"),__WFUNCTION__));
                break; 
                    
            default:
                ERRORMSG(TRUE, (TEXT("%s: Invalid data format for preview.\r\n"), __WFUNCTION__));
                return FALSE;
        } 
        
        /****************************************************************/
        // CSI Setting
        /****************************************************************/ 
        // Set CSI output format
        if (m_SMFCConfig.outputFormat == SMFCFormat_RGB565)
        {
            m_SensorOutputFormat = csiSensorOutputFormat_RGB565;
        }
        else if((m_SMFCConfig.outputFormat == SMFCFormat_YUV420)||
                (m_SMFCConfig.outputFormat == SMFCFormat_YUV420P)||
                (m_SMFCConfig.outputFormat == SMFCFormat_UYVY422))
        {
            m_SensorOutputFormat = csiSensorOutputFormat_YUV422_UYVY;
        }

        // Set CSI output resolution
        switch(m_SMFCConfig.outputSize.width)
        {
            case D1_Width:
                if (m_SMFCConfig.outputSize.height == D1_PAL_Height)
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
            case SVGA_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_SVGA;
                break;
            case XGA_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_XGA;
                break;
            case SXGA_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_SXGA;
                break;
            case UXGA_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_UXGA;
                break;
            case QXGA_Width:
                m_SensorOutputResolution = csiSensorOutputResolution_QXGA;
                break;

            default:
                return FALSE;
        }

        if(m_PrtclInf.mode == CSI_CCIR_INTERLACE_BT656_MODE      ||
           m_PrtclInf.mode == CSI_CCIR_INTERLACE_BT1120DDR_MODE  ||
           m_PrtclInf.mode == CSI_CCIR_INTERLACE_BT1120SDR_MODE)
        {
            //if use interlace mode, wu will downsize the image height
            m_SMFCConfig.outputSize.height = m_SMFCConfig.outputSize.height/2;
        }

    }
    else// For only preview pin enable,need to request SMFC parameters
    {
        /****************************************************************/
        // CSI Setting
        /****************************************************************/ 
        // Set CSI output format
        BSPGetSensorFormat(&sensorFormat);
        m_SensorOutputFormat = (csiSensorOutputFormat) sensorFormat;

        // Set CSI output resolution
        BSPGetSensorResolution(&sensorResolution);
        m_SensorOutputResolution = (csiSensorOutputResolution) sensorResolution;
       
        // Set SMFC output resolution
        switch((csiSensorOutputResolution) sensorResolution)
        {
            case csiSensorOutputResolution_CIF:
                m_SMFCConfig.outputSize.width =  352;
                m_SMFCConfig.outputSize.height = 288;
                break;
            case csiSensorOutputResolution_QVGA:
                m_SMFCConfig.outputSize.width =  320;
                m_SMFCConfig.outputSize.height = 240;
                break;
            case csiSensorOutputResolution_VGA:
                m_SMFCConfig.outputSize.width =  640;
                m_SMFCConfig.outputSize.height = 480;
                break;
            case csiSensorOutputResolution_XGA:
                m_SMFCConfig.outputSize.width =  1024;
                m_SMFCConfig.outputSize.height = 768;
                break;
            case csiSensorOutputResolution_D1_PAL:
                m_SMFCConfig.outputSize.width =  720;
                m_SMFCConfig.outputSize.height = 576;
                break;    
            case csiSensorOutputResolution_D1_NTSC:
                m_SMFCConfig.outputSize.width =  720;
                m_SMFCConfig.outputSize.height = 480;
                break;  
        }

        if(m_PrtclInf.mode == CSI_CCIR_INTERLACE_BT656_MODE      ||
           m_PrtclInf.mode == CSI_CCIR_INTERLACE_BT1120DDR_MODE  ||
           m_PrtclInf.mode == CSI_CCIR_INTERLACE_BT1120SDR_MODE)
        {
            //if use interlace mode, wu will downsize the image height
            m_SMFCConfig.outputSize.height = m_SMFCConfig.outputSize.height/2;
        }
        
        // Set SMFC output format
        switch((csiSensorOutputFormat)sensorFormat)
        {
            case csiSensorOutputFormat_RGB444:
            case csiSensorOutputFormat_RGB555:
            case csiSensorOutputFormat_RGB565:
            case csiSensorOutputFormat_RGB888:    
                m_SMFCConfig.outputFormat = SMFCFormat_RGB565; 
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Set SMFCFormat_RGB\r\n"),__WFUNCTION__));
                break;

            case csiSensorOutputFormat_YUV422_UYVY:
            case csiSensorOutputFormat_YUV422_YUYV:
            case csiSensorOutputFormat_YUV444:                
                m_SMFCConfig.outputFormat = SMFCFormat_UYVY422;
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:Set SMFCFormat_UYVY422\r\n"),__WFUNCTION__));
                break;            
        }

        if((m_SMFCConfig.outputFormat == SMFCFormat_YUV420)
            ||(m_SMFCConfig.outputFormat == SMFCFormat_YUV420P))
        {     
            m_ulSMFCBufSize = m_SMFCConfig.outputSize.width * m_SMFCConfig.outputSize.height * 12/8; 
        }
        else
            m_ulSMFCBufSize = m_SMFCConfig.outputSize.width * m_SMFCConfig.outputSize.height * 16/8; 

        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: m_ulSMFCBufSize = %x \r\n"),__WFUNCTION__,m_ulSMFCBufSize));
    }

    // Set direct display variable
    m_SMFCConfig.directDisplay = m_bDirectDisplay;
    
    // Set SMFC output DataWidth
    m_SMFCConfig.outputDataWidth = SMFCDataWidth_16BPP;//default
    // Set SMFC output pixel format
    switch(m_SMFCConfig.outputFormat)
    {
        case SMFCFormat_RGB565:
            m_SMFCConfig.outputPixelFormat.component0_offset = RGB565_COMPONENT0_OFFSET;
            m_SMFCConfig.outputPixelFormat.component1_offset = RGB565_COMPONENT1_OFFSET;
            m_SMFCConfig.outputPixelFormat.component2_offset = RGB565_COMPONENT2_OFFSET;
            m_SMFCConfig.outputPixelFormat.component3_offset = RGB565_COMPONENT3_OFFSET;
            m_SMFCConfig.outputPixelFormat.component0_width = RGB565_COMPONENT0_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component1_width = RGB565_COMPONENT1_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component2_width = RGB565_COMPONENT2_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component3_width = RGB565_COMPONENT3_WIDTH;            
            break;
        case SMFCFormat_RGB24:
            m_SMFCConfig.outputDataWidth = SMFCDataWidth_24BPP;
            m_SMFCConfig.outputPixelFormat.component0_offset = RGB888_COMPONENT0_OFFSET;
            m_SMFCConfig.outputPixelFormat.component1_offset = RGB888_COMPONENT1_OFFSET;
            m_SMFCConfig.outputPixelFormat.component2_offset = RGB888_COMPONENT2_OFFSET;
            m_SMFCConfig.outputPixelFormat.component3_offset = RGB888_COMPONENT3_OFFSET;
            m_SMFCConfig.outputPixelFormat.component0_width = RGB888_COMPONENT0_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component1_width = RGB888_COMPONENT1_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component2_width = RGB888_COMPONENT2_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component3_width = RGB888_COMPONENT3_WIDTH - 1;            
            break;
        case SMFCFormat_YUV420:
        case SMFCFormat_YUV420P:
            m_SMFCConfig.outputPixelFormat.component0_offset = YUV420_COMPONENT0_OFFSET;
            m_SMFCConfig.outputPixelFormat.component1_offset = YUV420_COMPONENT1_OFFSET;
            m_SMFCConfig.outputPixelFormat.component2_offset = YUV420_COMPONENT2_OFFSET;
            m_SMFCConfig.outputPixelFormat.component3_offset = YUV420_COMPONENT3_OFFSET;
            m_SMFCConfig.outputPixelFormat.component0_width = YUV420_COMPONENT0_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component1_width = YUV420_COMPONENT1_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component2_width = YUV420_COMPONENT2_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component3_width = YUV420_COMPONENT3_WIDTH - 1;            
            break;
        case SMFCFormat_UYVY422:
            m_SMFCConfig.outputPixelFormat.component0_offset = UYVY_COMPONENT0_OFFSET;
            m_SMFCConfig.outputPixelFormat.component1_offset = UYVY_COMPONENT1_OFFSET;
            m_SMFCConfig.outputPixelFormat.component2_offset = UYVY_COMPONENT2_OFFSET;
            m_SMFCConfig.outputPixelFormat.component3_offset = UYVY_COMPONENT3_OFFSET;
            m_SMFCConfig.outputPixelFormat.component0_width = UYVY_COMPONENT0_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component1_width = UYVY_COMPONENT1_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component2_width = UYVY_COMPONENT2_WIDTH - 1;
            m_SMFCConfig.outputPixelFormat.component3_width = UYVY_COMPONENT3_WIDTH - 1;
            break;
    }

    // Camera has been configured for encoding.
    m_bCameraEncConfigRequest  = TRUE;
    m_bCameraEncConfig = FALSE;
        
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SensorClosePin
//
// This function close a given pin.
//
// Parameters:
//      iPin
//          [in] Specifies pin (STILL, CAPTURE, or PREVIEW) for
//          which we can close.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorClosePin(UINT32 iPin)
{
    DEBUGMSG(ZONE_IOCTL, (_T("+SensorClosePin\r\n")));

    // Call down to SMFC to deallocate buffer list
    switch (iPin)
    {
        case PREVIEW:
        case CAPTURE:
            // Stop SMFC
            m_pSMFC->SMFCStopChannel();
            if (!m_pSMFC->SMFCDeleteBuffers(m_pCaptureBufferManager))
            {
                return FALSE;
            }

            // Stop PP
            m_pCameraPP->CameraPPStopChannel();
            if (!m_pCameraPP->CameraPPDeleteBuffers(m_pPreviewBufferManager))
            {
                return FALSE;
            }
            break;
        case STILL:            
            // No need to delete buffers in the SMFC, as those buffers
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
    return;
}


//-----------------------------------------------------------------------------
//
// Function: SensorMarkAsModified
//
// This function informs the camera driver that important
// configuration parameters have been modified.  This ensures
// that the camera will reconfigure the preprocessing before
// starting again.
//
// Parameters:
//      ulPinId
//          [in] Specifies whether for CAPTURE pin or PREVIEW pin to
//               modify configuration parameters.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CSensorPdd::SensorMarkAsModified(ULONG ulPinId)
{

    if (ulPinId == PREVIEW)
    {
        // Assure that we will reconfigure IPU before starting.
        m_bCameraVfConfigRequest = FALSE;
    }
    else if (ulPinId == CAPTURE)
    {
        // Assure that we will reconfigure IPU before starting.
        m_bCameraEncConfigRequest  = FALSE;
        m_bCameraVfConfigRequest = FALSE;
    }
    else if(ulPinId == STILL)
    {
        m_bCameraStillConfig  = FALSE;
    }

}


//-----------------------------------------------------------------------------
//
// Function: SensorStopChannel
//
// Stop the SMFC , PP and CSI channel
//
// Parameters:
// Parameters:
//      iPin
//          [in] Specifies pin (STILL, CAPTURE, or PREVIEW) for
//          which we can stop channels.
//
// Returns:
//      TRUE for SUCCESS,FALSE for falure.
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorStopChannel(UINT32 iPin)
{
    switch (iPin)
    {
        case PREVIEW:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:PREVIEW StopPPChannel begin\r\n"),__WFUNCTION__));
            // Stop the PP channel
            if (m_bCameraPPEnable)
            {
                if( !m_pCameraPP->CameraPPStopChannel() )
                {
                     RETAILMSG(1, (_T("%s: Stop PP channel failed!\r\n"), __WFUNCTION__));
                     return FALSE;
                }
            }
            RETAILMSG(0,(TEXT("%s:PREVIEW StopPPChannel finished\r\n"),__WFUNCTION__));
            break;
            
        case CAPTURE:
            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:CAPTURE StopSMFCChannel begin\r\n"),__WFUNCTION__));

            // Wait for SMFC not busy
            m_pSMFC->SMFCPreStopChannel();

            // Disable CSI:for frame synchronization
            if(m_pCsi0 != NULL)
                m_pCsi0->CsiDisable();
            else if(m_pCsi1 != NULL)
                m_pCsi1->CsiDisable();

            // Stop the SMFC channel 
            if( !m_pSMFC->SMFCStopChannel() )
            {
                 RETAILMSG(1, (_T("%s: Stop SMFC channel failed!\r\n"), __WFUNCTION__));
                 return FALSE;
            } 

            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:CAPTURE StopSMFCChannel finished\r\n"),__WFUNCTION__));
            break;
            
        case STILL:  
            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:STILL StopSMFCChannel begin\r\n"),__WFUNCTION__));

            // Wait for SMFC not busy
            m_pSMFC->SMFCPreStopChannel();

            // Disable CSI:for frame synchronization
            if(m_pCsi0 != NULL)
                m_pCsi0->CsiDisable();
            else if(m_pCsi1 != NULL)
                m_pCsi1->CsiDisable();

            // Stop the SMFC channel 
            if( !m_pSMFC->SMFCStopChannel() )
            {
                 RETAILMSG(1, (_T("%s: Stop SMFC channel failed!\r\n"), __WFUNCTION__));
                 return FALSE;
            }

            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:STILL StopSMFCChannel finished\r\n"),__WFUNCTION__));
            break;
            
        default:
            DEBUGMSG(ZONE_ERROR,(_T("SensorStopChannel: Invalid pin id.\r\n"))) ;
            return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: SensorStartChannel
//
// Start the SMFC , PP and CSI channel
//
// Parameters:
// Parameters:
//      iPin
//          [in] Specifies pin (STILL, CAPTURE, or PREVIEW) for
//          which we can start channel.
//
// Returns:
//      TRUE for SUCCESS,FALSE for falure.
//
//-----------------------------------------------------------------------------
BOOL CSensorPdd::SensorStartChannel(UINT32 iPin)
{
    switch (iPin)
    {
        case PREVIEW:
            // for the issue: new buffer manager
            m_pCameraPP->CameraPPSetBufferManager(m_pPreviewBufferManager);

            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:PREVIEW StartPPChannel begin\r\n"),__WFUNCTION__));
            // Start the PP channel
            if (m_bCameraPPEnable)
            {
                if( !m_pCameraPP->CameraPPStartChannel() )
                {
                     DEBUGMSG(ZONE_ERROR, (_T("%s: Start PP channel failed!\r\n"), __WFUNCTION__));
                     return FALSE;
                }
            }

            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:PREVIEW StartPPChannel finished\r\n"),__WFUNCTION__));
            break;
            
        case CAPTURE:
            // for the issue: new buffer manager
            m_pSMFC->SMFCSetBufferManager(m_pCaptureBufferManager);

            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:CAPTURE StartSMFCChannel begin\r\n"),__WFUNCTION__));

            // Start the SMFC channel 
            if( !m_pSMFC->SMFCStartChannel() )
            {
                 DEBUGMSG(ZONE_ERROR, (_T("%s: Start SMFC channel failed!\r\n"), __WFUNCTION__));
                 return FALSE;
            }

            // Enable CSI:for frame synchronization
            if(m_pCsi0 != NULL)
                m_pCsi0->CsiEnable();
            else if(m_pCsi1 != NULL)
                m_pCsi1->CsiEnable();

            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:CAPTURE StartSMFCChannel finished\r\n"),__WFUNCTION__));
            break;
            
        case STILL:            
            // for the issue: new buffer manager
            m_pSMFC->SMFCSetBufferManager(m_pStillBufferManager);

            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:STILL StartSMFCChannel begin\r\n"),__WFUNCTION__));

            // Start the SMFC channel 
            if( !m_pSMFC->SMFCStartChannel() )
            {
                 DEBUGMSG(ZONE_ERROR, (_T("%s: Start SMFC channel failed!\r\n"), __WFUNCTION__));
                 return FALSE;
            }

            // Enable CSI:for frame synchronization
            if(m_pCsi0 != NULL)
                m_pCsi0->CsiEnable();
            else if(m_pCsi1 != NULL)
                m_pCsi1->CsiEnable();

            DEBUGMSG(ZONE_FUNCTION,(TEXT("%s:STILL StartSMFCChannel finished\r\n"),__WFUNCTION__));
            
            break;
            
        default:
            DEBUGMSG(ZONE_ERROR,(_T("SensorStopChannel: Invalid pin id.\r\n"))) ;
            return FALSE;
    }

    return TRUE;
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
//      ulPinId  
//          [in] module id.
//
// Returns:
//      TRUE for SUCCESS,FALSE for falure.
//
//-----------------------------------------------------------------------------
DWORD CSensorPdd::PreStillPicture(ULONG ulPinId)
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::PreStillPicture Begin\r\n")));
    
    if (STILL != ulPinId)
    {
         DEBUGMSG(ZONE_ERROR, (_T("PostStillPicture(%08x): The pin id for PreStillPicture is error!\r\n"), this));
         return FALSE; 
    }

    //************************************************
    //******Stop the channel before reconfig STIL pin 
    //**********************************
    // CAPTURE pin and STILL pin share SMFC, so when running STILL Pin , first stop CAPTURE pin channel
    if( (m_CsState[CAPTURE] == CSSTATE_RUN) | (m_CsState[PREVIEW] == CSSTATE_RUN) )
    {
        if( m_CsState[PREVIEW] == CSSTATE_RUN )
        {
            SensorStopChannel(PREVIEW);
        }

        if( !SensorStopChannel(CAPTURE) )
        {
            DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Stop sensor channel for capture capture failed!\r\n"), this));
            return FALSE;
        }
    }

    // Reconfigure for STILL image capture    
    if (!SensorEncConfigRequest(STILL,FALSE))
    {
        ERRORMSG(TRUE,(TEXT("%s:SensorVfConfigRequest fail\r\n"),__WFUNCTION__)); 
        return FALSE;
    }
    if (!SensorEncodingConfig())
    {
        ERRORMSG(TRUE,(TEXT("%s:SensorViewfindingConfig fail\r\n"),__WFUNCTION__)); 
        return FALSE;
    }

    if( m_CsState[PREVIEW] == CSSTATE_RUN )
    {
        SensorVfConfigRequest(PREVIEW);
        SensorViewfindingConfig();
    }
    
    //************************************************
    //******Start the channel after reconfig STIL pin
    //**********************************
    if( !SensorStartChannel(STILL) )
    {
         DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Start SMFC channel for still capture failed!\r\n"), this));
         return FALSE;
    }

    if( m_CsState[PREVIEW] == CSSTATE_RUN )
    {
        SensorStartChannel(PREVIEW);
    }
    
    DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::PreStillPicture Finished\r\n")));
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
//      ulPinId  
//          [in] module id.
//
// Returns:
//      TRUE for SUCCESS,FALSE for falure.
//
//-----------------------------------------------------------------------------
DWORD CSensorPdd::PostStillPicture(ULONG ulPinId)
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::PostStillPicture begin\r\n")));
    
    if (STILL != ulPinId)
    {
         DEBUGMSG(ZONE_ERROR, (_T("PostStillPicture(%08x): The pin id for poststillpicture is error!\r\n"), this));
         return FALSE; 
    }
    
    // Stop the channel.   
    if( !SensorStopChannel(STILL) )
    {
        DEBUGMSG(ZONE_ERROR, (_T("PreStillPicture(%08x): Stop sensor channel for still capture failed!\r\n"), this));
        return FALSE;
    }

    m_bCameraEncConfigRequest  = FALSE;

    DEBUGMSG(ZONE_FUNCTION,(TEXT("CSensorPdd::PostStillPicture finished\r\n")));

    return TRUE;    
}
