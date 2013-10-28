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

//Timer Event constants
#define STOP_THREAD 1
#define MAX_TIMER_EVENTS MAX_SUPPORTED_PINS + STOP_THREAD
#define THREAD_TERMINATE_EVENT (MAX_TIMER_EVENTS-1)
#define TIMER_THREAD_PRIORITY     THREAD_PRIORITY_TIME_CRITICAL

//Constants moved from platform MDD to enable linking to public MDD
#define NUM_SUPPORTED_PINS        3
#define NUM_PIN_BUFFER            3
#define WAIT_BUFFER_TIMEOUT       2000     // Timeout for receive a buffer from Prp module

#if (UNDER_CE > 600)
# define MAX_NO_OF_BUFFERS              4
# define CLIENT_MEMORY_MODEL        CSPROPERTY_BUFFER_CLIENT_LIMITED
#else
# define MAX_NO_OF_BUFFERS              1
# define CLIENT_MEMORY_MODEL        CSPROPERTY_BUFFER_DRIVER
#endif


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

#if (UNDER_CE > 600)
    DWORD Open( 
        PVOID MDDOpenContext ); 

    DWORD Close(
        PVOID MDDOpenContext );

    DWORD GetMetadata(
        DWORD dwPropId,
        PUCHAR pOutBuf,
        DWORD OutBufLen,
        PDWORD pdwBytesTransferred);
#endif

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

    BOOL
    KillTimer( ULONG ulModeType );

    //ThreadProc to handle timer events
    static DWORD WINAPI HandleTimerEvents(LPVOID lpvParam);

    void 
    HandleCaptureInterrupt( UINT uTimerID );

    void 
    HandleStillInterrupt( UINT uTimerID );

    BOOL 
    ReadMemoryModelFromRegistry();

    BOOL SensorConfig();
    BOOL SensorViewfindingConfig(UINT32 iBufNum);
    BOOL SensorEncodingConfig(UINT32 iEncPin, UINT32 iBufNum);
    void SensorSetDirectDisplay(BOOL bDirectDisplay);
    void SensorSetDirectCapture(BOOL bDirectCapture);
    BOOL SensorClosePin(UINT32 iPin);
    void SensorZoom(DWORD zoomVal);
    void SensorMarkAsModified(ULONG ulPinId);

    
private:

    BOOL    m_bStillCapInProgress;
    HANDLE m_hContext;
    
    CSSTATE                 m_CsState[MAX_SUPPORTED_PINS];
    SENSORMODEINFO          m_SensorModeInfo[MAX_SUPPORTED_PINS];

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
    HANDLE             m_hTimerThread;
    FNTIMESETEVENT     m_pfnTimeSetEvent; 
    FNTIMEKILLEVENT    m_pfnTimeKillEvent;
    HMODULE            m_hTimerDll;
    MMRESULT           m_TimerIdentifier[MAX_SUPPORTED_PINS];
    HANDLE             m_hTimerEvents[MAX_TIMER_EVENTS];
    BOOL               m_bTimerThreadCreated;
    BOOL               m_bTimerThreadRunning;
    
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
    BOOL             m_bCameraEncConfig, m_bCameraVfConfig;

    // If TRUE, use hardware support to send data on PREVIEW
    // pin directly to display.  Ignored for any other pins.
    BOOL             m_bDirectDisplay;

    // If TRUE, use hardware support to send sensor data directly
    // to memory without any preprocessing.  Ignored for PREVIEW pin.
    BOOL             m_bDirectCapture;

#if(UNDER_CE > 600)
    HANDLE  m_hOpenContext;
    // Metadata
    PCS_PROPERTYITEM  m_pMetadata;
#endif
} SENSORPDD, * PSENSORPDD;

#ifdef __cplusplus
}
#endif

#endif
