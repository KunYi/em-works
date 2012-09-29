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

#ifndef _SENSORPDD_H
#define _SENSORPDD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef MMRESULT (WINAPI *FNTIMEKILLEVENT)(UINT);
typedef MMRESULT (WINAPI *FNTIMESETEVENT)(UINT, UINT, LPTIMECALLBACK, DWORD, UINT );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
typedef class CSensorPdd
{
public:
    
    friend class CCameraDevice;

    CSensorPdd();

    ~CSensorPdd();

    DWORD PDDInit( 
        PVOID MDDContext,
        PPDDFUNCTBL pPDDFuncTbl
        );

    DWORD GetAdapterInfo( 
        PADAPTERINFO pAdapterInfo 
        );

    DWORD HandleVidProcAmpChanges(
        DWORD dwPropId, 
        LONG lFlags, 
        LONG lValue
        );
    
    DWORD HandleCamControlChanges( 
        DWORD dwPropId, 
        LONG lFlags, 
        LONG lValue 
        );

    DWORD HandleVideoControlCapsChanges(
        LONG lModeType ,
        ULONG ulCaps 
        );

    DWORD SetPowerState(
        CEDEVICE_POWER_STATE PowerState 
        );
    
    DWORD HandleAdapterCustomProperties(
        PUCHAR pInBuf, 
        DWORD  InBufLen, 
        PUCHAR pOutBuf, 
        DWORD  OutBufLen, 
        PDWORD pdwBytesTransferred 
        );

    DWORD InitSensorMode(
        ULONG ulModeType, 
        LPVOID ModeContext
        );
    
    DWORD DeInitSensorMode( 
        ULONG ulModeType 
        );

    DWORD SetSensorState( 
        ULONG lPinId, 
        CSSTATE csState 
        );

    DWORD TakeStillPicture(
        LPVOID pBurstModeInfo );

    DWORD GetSensorModeInfo( 
        ULONG ulModeType, 
        PSENSORMODEINFO pSensorModeInfo 
        );

    DWORD GetSensorFrameRate( 
        ULONG ulModeType
        );

    // For change framerate
    DWORD SetSensorFrameRate( 
        DWORD dwFramerate
        );

    DWORD SetSensorModeFormat( 
        ULONG ulModeType, 
        PCS_DATARANGE_VIDEO pCsDataRangeVideo 
        );

    PVOID AllocateBuffer(
        ULONG ulModeType 
        );

    DWORD DeAllocateBuffer( 
        ULONG ulModeType, 
        PVOID pBuffer
        );

    DWORD RegisterClientBuffer(
        ULONG ulModeType, 
        PVOID pBuffer 
        );

    DWORD UnRegisterClientBuffer( 
        ULONG ulModeType, 
        PVOID pBuffer 
        );
    
    DWORD EnqueueBuffer(
        ULONG ulModeType, 
        PVOID pBuffer 
        );
    
    DWORD FillBuffer( 
        ULONG ulModeType, 
        PUCHAR pImage );
    
    DWORD FillBufferEx(
        ULONG ulModeType,
        PUCHAR *ppImage
        );

    DWORD HandleSensorModeCustomProperties( 
        ULONG ulModeType, 
        PUCHAR pInBuf, 
        DWORD  InBufLen, 
        PUCHAR pOutBuf, 
        DWORD  OutBufLen, 
        PDWORD pdwBytesTransferred 
        );

    static
    void
    CaptureTimerCallBack(
        UINT uTimerID,
        UINT uMsg,
        DWORD_PTR dwUser,
        DWORD_PTR dw1,
        DWORD_PTR dw2
        ); 

    static
    void
    StillTimerCallBack(
        UINT uTimerID,
        UINT uMsg,
        DWORD_PTR dwUser,
        DWORD_PTR dw1,
        DWORD_PTR dw2
        ); 
    DWORD PreStillPicture(ULONG ulPinId);
    DWORD PostStillPicture(ULONG ulPinId);
    
private:

    BOOL
    CreateTimer( ULONG ulModeType );

    void 
    HandleCaptureInterrupt( UINT uTimerID );

    void 
    HandleStillInterrupt( UINT uTimerID );

    BOOL 
    ReadMemoryModelFromRegistry();

    BOOL 
    ReadCSIInterfaceFromRegistry();

    BOOL SensorViewfindingConfig();
    BOOL SensorEncodingConfig();
    void SensorSetDirectDisplay(BOOL bDirectDisplay);
    void SensorSetDirectCapture(BOOL bDirectCapture);
    BOOL SensorClosePin(UINT32 iPin);
    void SensorZoom(DWORD zoomVal);
    void SensorMarkAsModified(ULONG ulPinId);
    BOOL SensorVfConfigRequest(ULONG ulPinId);
    BOOL SensorEncConfigRequest(ULONG ulPinId, BOOL bDefault);
    BOOL SensorStartChannel(UINT32 iPin);
    BOOL SensorStopChannel(UINT32 iPin);
    DWORD GetPhysAddress( PVOID lpUnMappedBuffer, ULONG ulSize);
    
private:

    BOOL    m_bStillCapInProgress;
    HANDLE m_hContext;
    
    CSSTATE                 m_CsState[MAX_SUPPORTED_PINS];
    SENSORMODEINFO           m_SensorModeInfo[MAX_SUPPORTED_PINS];

    // Total number of pins implemented by this camera
    ULONG m_ulCTypes;

    // Power Capabilities
    POWER_CAPABILITIES PowerCaps; 

    // All ProcAmp and CameraControl props
    SENSOR_PROPERTY     m_SensorProps[NUM_PROPERTY_ITEMS];

    // All the Video Formats supported by all the pins
    PPINVIDEOFORMAT   m_pModeVideoFormat;

    // VideoControl Caps corresponding to all the pins
    VIDCONTROLCAPS   *m_pModeVideoCaps;

    // Pointer to the MDD Pin Object corresponding to all the pins.
    // HandlePinIO() method of this object is then called whenever 
    // an image is ready. HandlePinIO() internally calls 
    // FillPinBuffer() of PDD interface.
    LPVOID       *m_ppModeContext;
    
    // Currently selected video format for each pin
    PCS_DATARANGE_VIDEO m_pCurrentFormat;

    // Timer specific
    FNTIMESETEVENT     m_pfnTimeSetEvent; 
    FNTIMEKILLEVENT    m_pfnTimeKillEvent;
    HMODULE            m_hTimerDll;
    MMRESULT           m_TimerIdentifier[MAX_SUPPORTED_PINS];
    
    ULONG   m_ulNumOfBuffer;

    SMFCClass  *m_pSMFC;
    CameraPPClass *m_pCameraPP;
    CsiClass   *m_pCsi0;
    CsiClass   *m_pCsi1;
    CSI_SELECT       m_CSIInterface;
    BOOL        m_bCsiTstMode;

    BOOL             m_SensorConfigured;

    BOOL             m_bFlipVertical;
    BOOL             m_bFlipHorizontal;
    BOOL             m_bRotate;

    // If TRUE, camera has been configured for encoding
    // or viewfinding task and may be started
    // without requiring reconfiguration.  If FALSE, camera
    // must be configured before starting.
    BOOL             m_bCameraEncConfig;
    BOOL             m_bCameraStillConfig;
    BOOL             m_bCameraVfConfig;
    BOOL             m_bCameraEncConfigRequest , m_bCameraVfConfigRequest;

    // If TRUE, use hardware support to send data on PREVIEW
    // pin directly to display.  Ignored for any other pins.
    BOOL             m_bDirectDisplay;

    //record CSI protocol mode and relative command
    CSI_PROTOCOL_INF      m_PrtclInf;

    csiSensorOutputFormat m_SensorOutputFormat;
    csiSensorOutputResolution m_SensorOutputResolution;
    SMFCConfigData m_SMFCConfig;
    ppConfigData   m_PPConfig;
    ULONG m_ulSMFCBufSize;
    ULONG m_ulCameraPPBufSize;
    BOOL  m_bSensorBufferAllocByDriver;
    BOOL  m_bAllocateBufferForSMFC;
    BOOL  m_bAllocateBufferForPP;
    BOOL  m_bAllocateBufferForSTILL;
    BOOL   m_bCameraPPEnable;
    UINT32*  m_pSMFCAllocPhyAddr[NUM_PIN_BUFFER_MAX];

    DWORD m_dwSensorFramerate;
    
    // add for removing timer
    // we will create 2 threads instead of 2 timer
    static void PinEncWorkerThread(LPVOID);
    static void PinVfWorkerThread(LPVOID);
    void PinEncWorkerLoop(UINT32);
    void PinVfWorkerLoop(UINT32);

    HANDLE             m_hPinEncEvent[2]; // 0 for begin 1 for stop
    HANDLE             m_hPinVfEvent[2];
    HANDLE             m_hPinEncThread;
    HANDLE             m_hPinVfThread;

    // for the issue: new buffer manager
    CamBufferManager *m_pPreviewBufferManager;
    CamBufferManager *m_pCaptureBufferManager;
    CamBufferManager *m_pStillBufferManager;

    //Event using to distermin the connected filter is freescale filter or not
    //When Event is set, connected filter is freescale filter, which can support all of the format
    //When Event is reset, the filter isn't freescale's, so we just report limited format
    HANDLE m_hFslFilter;

    // Get snapshot of silicon rev.  We use ROM ID to uniquely identify the silicon version:
    //
    //      ROM ID      Silicon Rev
    //      -----------------------
    //      0x01        TO1.0
    //      0x02        TO1.1
    //      0x10        TO2.0
    //      0x20        TO3.0
    //
    DWORD m_dwSiVer;
} SENSORPDD, * PSENSORPDD;

#ifdef __cplusplus
}
#endif

#endif
