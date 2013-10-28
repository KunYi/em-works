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
#include "csp.h"
#include "CsiClass.h"
#include "PrpClass.h"

#ifndef _NULLPDD_H
#define _NULLPDD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef MMRESULT (WINAPI *FNTIMEKILLEVENT)(UINT);
typedef MMRESULT (WINAPI *FNTIMESETEVENT)(UINT, UINT, LPTIMECALLBACK, DWORD, UINT );

// LGN pin define
#define BUFFER_COUNT            4        // Increasing the buffer count from 3 to 4.

typedef class CamPddDevice
{
public:

    friend class CCameraDevice;

    CamPddDevice();

    ~CamPddDevice();
    DWORD Open( 
        PVOID MDDOpenContext ); 

    DWORD Close(
        PVOID MDDOpenContext );

    DWORD GetMetadata(
        DWORD dwPropId,
        PUCHAR pOutBuf,
        DWORD OutBufLen,
        PDWORD pdwBytesTransferred);
    HANDLE  m_hOpenContext;
    // Metadata
    PCS_PROPERTYITEM  m_pMetadata;

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

private:

    bool    m_bStillCapInProgress;
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
    // This is also used as a flag of weather the pin is initialized.
    LPVOID       *m_ppModeContext;

    // Currently selected video format for each pin
    PCS_DATARANGE_VIDEO m_pCurrentFormat;

    // LGN added properties and methods
public:

    BOOL
    CameraVfIST();
    BOOL
    CameraEncIST();

private:

    BOOL
    CameraPowerUp(BOOL);

    void
    CameraPowerDown(BOOL);

    BOOL
    AllocateDriverBuffers(ULONG ulModeType);

    BOOL
    DeAllocateDriverBuffers(ULONG ulModeType);

    BOOL
    CameraSensorConfig();

    BOOL
    CameraPrpConfig(ULONG iBufNum);

    void
    CameraSensorZoom(DWORD zoomVal);

    void CameraSensorFlip(BOOL, BOOL); // LG_NORTEL_MODIFY

    //BOOL
    //AdapterCompareFormat(ULONG ulPinId, const PCS_DATAFORMAT_VIDEOINFOHEADER pCsDataVIHToCompare, PCS_DATARANGE_VIDEO *ppCsDataRangeVideoMatched, BOOL fDetailedComparison);
    //
    //BOOL
    //AdapterCompareGUIDsAndFormatSize(const PCSDATARANGE DataRange1, const PCSDATARANGE DataRange2);

    //properties
    ULONG            m_StillPinInherited;
    CEDEVICE_POWER_STATE m_PowerState;
    ULONG               m_ulMaxNumOfBuffers;
    prpInputFormat   m_inFormat;                         // Input parameters, dependent on the camera sensor
    prpFrameSize     m_inFrameSize;                      // and configured one time.

    BOOL            m_bFlipVertical;
    BOOL            m_bFlipHorizontal;

    PrpClass *m_pPrp;
    CsiClass *m_pCsi;
    HANDLE m_hVfShutDown;
    HANDLE m_hEncShutDown;
    HANDLE m_hVfInterruptThread;
    HANDLE m_hEncInterruptThread;
} NULLPDD, * PNULLPDD;

DWORD CallVfThread(CamPddDevice *pCamPddDevice);
DWORD CallEncThread(CamPddDevice *pCamPddDevice);

#ifdef __cplusplus
}
#endif

#endif
