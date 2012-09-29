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

#ifndef _SENSORPDD_H
#define _SENSORPDD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef MMRESULT (WINAPI *FNTIMEKILLEVENT)(UINT);
typedef MMRESULT (WINAPI *FNTIMESETEVENT)(UINT, UINT, LPTIMECALLBACK, DWORD, UINT );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// YUV specific buffer filler 

UINT YUVBufferFill( PUCHAR pImage, PCS_VIDEOINFOHEADER pCsVideoInfoHdr, BOOL FlipHorizontal, LPVOID lpParam );

// YUY2 specific defines
#define MACROPIXEL_RED 0xf0525a52
#define MACROPIXEL_GRN 0x22913691
#define MACROPIXEL_WHITE 0x80EB80EB
#define MACROPIXEL_BLACK 0x80108010

// YV12 specific defines
#define Y_WHITE 235
#define Y_BLACK 16
#define VU_BOTH 128

// Misc YUV filler flags
#define BOXWIDTHDIVIDER 8
#define BOXHEIGHTDIVIDER 8
#define LOCATIONWIDTHMASK 0xFF
#define LOCATIONHEIGHTMASK 0xFF
#define LOCATIONSHIFT 8
// divide the initial tick count by four to make the box move 2x faster.
#define SPEEDSHIFT 4

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

    DWORD FillBuffer( 
        ULONG ulModeType, 
        PUCHAR pImage );

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

    BOOL SensorConfigCSI();
    BOOL SensorViewfindingConfig();
    BOOL SensorVfBufferAlloc(UINT32 iBufNum);
    BOOL SensorEncodingConfig(UINT32 iEncPin);
    BOOL SensorEncBufferAlloc(UINT32 iEncPin, UINT32 iBufNum);
    void SensorSetDirectDisplay(BOOL bDirectDisplay);
    void SensorSetDirectCapture(BOOL bDirectCapture);
    BOOL SensorClosePin(UINT32 iPin);
    void SensorZoom(DWORD zoomVal);
    void SensorMarkAsModified(ULONG ulPinId);

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

    PrpClass   *m_pPrp;
    CsiClass   *m_pCsi;

    BOOL             m_SensorConfigured;
    prpInputFormat   m_inFormat;                         // Input parameters, dependent on the camera sensor
    prpFrameSize     m_inFrameSize;                      // and configured one time.

    BOOL             m_bFlipVertical;
    BOOL             m_bFlipHorizontal;
    BOOL             m_bRotate;

    // If TRUE, camera has been configured for encoding
    // or viewfinding task and may be started
    // without requiring reconfiguration.  If FALSE, camera
    // must be configured before starting.
    BOOL             m_bCameraEncConfig, m_bCameraVfConfig, m_bCameraStillConfig; 
    BOOL             m_bCameraEncAllocBuf, m_bCameraVfAllocBuf;

    // If TRUE, use hardware support to send data on PREVIEW
    // pin directly to display.  Ignored for any other pins.
    BOOL             m_bDirectDisplay;

    // If TRUE, use hardware support to send sensor data directly
    // to memory without any preprocessing.  Ignored for PREVIEW pin.
    BOOL             m_bDirectCapture;

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

} SENSORPDD, * PSENSORPDD;

#ifdef __cplusplus
}
#endif

#endif
