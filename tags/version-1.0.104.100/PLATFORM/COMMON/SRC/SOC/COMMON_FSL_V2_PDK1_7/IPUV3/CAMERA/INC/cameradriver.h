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
//

#ifndef __CAMERADRIVER_H
#define __CAMERADRIVER_H
#pragma warning(disable: 28251)

#ifdef __cplusplus
extern "C" {
#endif

extern DBGPARAM dpCurSettings;


// CAPTURE and STILL are required pin types. PREVIEW is optional, so list it last.
enum
{
    CAPTURE = 0,
    STILL,
    PREVIEW
};


// DEFINES for PROPSETID_VIDCAP_VIDEOPROCAMP
typedef enum {
    // VideoProcAmp
    ENUM_BRIGHTNESS = 0,
    ENUM_CONTRAST,
    ENUM_HUE,
    ENUM_SATURATION,
    ENUM_SHARPNESS,
    ENUM_GAMMA,
    ENUM_COLORENABLE,
    ENUM_WHITEBALANCE,
    ENUM_BACKLIGHT_COMPENSATION,
    ENUM_GAIN,

    // CameraControl
    ENUM_PAN,
    ENUM_TILT,
    ENUM_ROLL,
    ENUM_ZOOM,
    ENUM_IRIS,
    ENUM_EXPOSURE,
    ENUM_FOCUS,
    ENUM_FLASH

} ENUM_DEV_PROP;


const size_t StandardSizeOfBasicValues   = sizeof(CSPROPERTY_DESCRIPTION) + sizeof(CSPROPERTY_MEMBERSHEADER) + sizeof(CSPROPERTY_STEPPING_LONG) ;
const size_t StandardSizeOfDefaultValues = sizeof(CSPROPERTY_DESCRIPTION) + sizeof(CSPROPERTY_MEMBERSHEADER) + sizeof(ULONG) ;


DWORD MDD_HandleIO( LPVOID ModeContext, ULONG ulModeType );

class CPinDevice;

typedef class CCameraDevice 
{
public:
    CCameraDevice( );

    ~CCameraDevice( );
    
    BOOL
    Initialize(
        PVOID context
        );

    BOOL
    BindApplicationProc(
        HANDLE
        );

    BOOL
    UnBindApplicationProc( );
    
    DWORD
    AdapterHandlePinRequests(
        __in_bcount(InBufLen) PUCHAR pInBuf,
        DWORD  InBufLen,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred
        );

    DWORD
    AdapterHandleVersion(
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred
        );        

    DWORD
    AdapterHandleVidProcAmpRequests(
        __in_bcount(InBufLen) PUCHAR pInBuf,
        DWORD  InBufLen,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred 
        );

    DWORD
    AdapterHandleCamControlRequests(
        __in_bcount(InBufLen) PUCHAR pInBuf,
        DWORD  InBufLen,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred
        );

    DWORD
    AdapterHandleVideoControlRequests(
        __in_bcount(InBufLen) PUCHAR pInBuf,
        DWORD  InBufLen,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred
        );
    
    DWORD
    AdapterHandleDroppedFramesRequests(
        __in_bcount(InBufLen) PUCHAR pInBuf,
        DWORD  InBufLen,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred
        );

    LONGLONG 
    AdapterHandleVideoCtrlActFrameRateSubRequests(
       ULONG ulPinId
       );
    
    DWORD
    AdapterHandlePowerRequests(
        DWORD  dwCode,
        __in_bcount(InBufLen) PUCHAR pInBuf,
        DWORD  InBufLen,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred
        );

    DWORD
    AdapterHandleCustomRequests(
        __in_bcount(InBufLen) PUCHAR pInBuf,
        DWORD  InBufLen,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred
        );
  
    LPVOID
    ValidateBuffer(
        __in_bcount(ulActualBufLen) LPVOID  lpBuff,
        ULONG   ulActualBufLen,
        ULONG   ulExpectedBuffLen,
        DWORD * dwError
        );
    
    BOOL
    AdapterCompareFormat(
        const ULONG                 ulPinId,
        const PCS_DATARANGE_VIDEO   pCsDataRangeVideoToCompare,
        PCS_DATARANGE_VIDEO       * ppCsDataRangeVideoMatched,
        BOOL                        fDetailedComparison
        );

    BOOL
    AdapterCompareFormat(
        const ULONG                            ulPinId,
        const PCS_DATAFORMAT_VIDEOINFOHEADER   pCsDataRangeVideoToCompare,
        PCS_DATARANGE_VIDEO                  * ppCsDataRangeVideoMatched,
        BOOL                                   fDetailedComparison
        );

    BOOL
    IsValidPin(
        ULONG ulPinId
        );

    BOOL
    GetPinFormat(
        ULONG                 ulPinId,
        ULONG                 ulIndex,
        PCS_DATARANGE_VIDEO * ppCsDataRangeVid
        );

    BOOL
    IncrCInstances(
        ULONG        ulPinId,
        CPinDevice * pPinDev
        );

    BOOL
    DecrCInstances(
        ULONG ulPinId
        );

    BOOL
    PauseCaptureAndPreview( );

    BOOL
    RevertCaptureAndPreviewState( );

    DWORD
    PDDClosePin( 
        ULONG ulPinId 
        );

    DWORD 
    PDDGetPinInfo( 
        ULONG ulPinId, 
        PSENSORMODEINFO pSensorModeInfo 
        );

    DWORD PDDSetPinState( 
        ULONG ulPinId, 
        CSSTATE State 
        );

    DWORD PDDFillPinBuffer( 
        ULONG ulPinId, 
        PUCHAR pImage 
        );
    DWORD PDDFillPinBufferEx( 
        ULONG ulPinId, 
        PUCHAR* ppImage 
        );

    DWORD PDDInitPin( 
        ULONG ulPinId, 
        CPinDevice *pPin 
        );

    DWORD PDDSetPinFormat(
        ULONG ulPinId,
        PCS_DATARANGE_VIDEO pCsDataRangeVideo
        );

    PVOID PDDAllocatePinBuffer( 
        ULONG ulPinId 
        );

    DWORD PDDDeAllocatePinBuffer( 
        ULONG ulPinId, 
        PVOID pBuffer
        );

    DWORD PDDRegisterClientBuffer(
        ULONG ulPinId,
        PVOID pBuffer 
        );

    DWORD PDDUnRegisterClientBuffer(
        ULONG ulPinId,
        PVOID pBuffer 
        );

    DWORD PDDEnqueueBuffer(
        ULONG ulPinId,
        PVOID pBuffer 
        );

    DWORD PDDHandlePinCustomProperties(
        ULONG ulPinId,
        __in_bcount(InBufLen) PUCHAR pInBuf,
        DWORD  InBufLen,
        __out_bcount(OutBufLen) PUCHAR pOutBuf,
        DWORD  OutBufLen,
        PDWORD pdwBytesTransferred
        );

    DWORD PDDPreStillPicture(
        ULONG ulPinId
        );
    
    DWORD PDDPostStillPicture(
        ULONG ulPinId
        );

private:

    BOOL
    GetPDDPinInfo();

    void
    GetBasicSupportInfo(
        __out_bcount(OutBufLen) PUCHAR        pOutBuf,
        DWORD         OutBufLen,
        PDWORD        pdwBytesTransferred,
        PSENSOR_PROPERTY pSensorProp,
        PDWORD        pdwError
        );

    void
    GetDefaultValues(
        __out_bcount(OutBufLen) PUCHAR        pOutBuf,
        DWORD         OutBufLen,
        PDWORD        pdwBytesTransferred,
        PSENSOR_PROPERTY pDevProp,
        PDWORD        pdwError
        );

    BOOL
    AdapterCompareGUIDsAndFormatSize(
        const PCSDATARANGE DataRange1,
        const PCSDATARANGE DataRange2
        );

    void 
    PowerDown();

    void
    PowerUp();

private:
    CRITICAL_SECTION m_csDevice;        

    HANDLE           m_hStream;                         // Handle to the corresponding stream sub-device
    HANDLE           m_hCallerProcess;                  // Handle of the process this driver is currently bound to.
    
    DWORD           m_dwVersion;
    CEDEVICE_POWER_STATE m_PowerState;
    STREAM_INSTANCES *m_pStrmInstances;
    ADAPTERINFO     m_AdapterInfo;
    PVOID           m_PDDContext;
    PDDFUNCTBL      m_PDDFuncTbl;

} CAMERADEVICE, * PCAMERADEVICE;

typedef struct CCameraOpenHandle
{
    PCAMERADEVICE pCamDevice;
} CAMERAOPENHANDLE, * PCAMERAOPENHANDLE;

#ifdef __cplusplus
}
#endif

#endif // __CAMERADRIVER_H
