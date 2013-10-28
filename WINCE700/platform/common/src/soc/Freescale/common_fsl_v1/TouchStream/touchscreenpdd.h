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
#ifndef __TOUCHPDD_H
#define __TOUCHPDD_H

// undef here since these are already defined.
#undef ZONE_INIT
#undef ZONE_ERROR

#include <tchddsi.h>

//------------------------------------------------------------------------------
// Sample rate during polling.
#define DEFAULT_SAMPLE_RATE                 200               // Hz.
#define TOUCHPANEL_SAMPLE_RATE_LOW          DEFAULT_SAMPLE_RATE
#define TOUCHPANEL_SAMPLE_RATE_HIGH         DEFAULT_SAMPLE_RATE
#define DEFAULT_THREAD_PRIORITY             109
#define THREAD_EXIT_TIMEOUT                 3000 //3sec

// Number of samples to discard when pen is initially down
#define DEFAULT_INITIAL_SAMPLES_DROPPED    1

#define RK_HARDWARE_DEVICEMAP_TOUCH     (TEXT("HARDWARE\\DEVICEMAP\\TOUCH"))
#define RV_CALIBRATION_DATA             (TEXT("CalibrationData"))


#define FILTEREDSAMPLESIZE              3
#define SAMPLESIZE                      1

#define CAL_DELTA_RESET             20
#define CAL_HOLD_STEADY_TIME        1500
#define ADC_SAMPLE_MIN_VALUE                   0
#define ADC_SAMPLE_MAX_VALUE                   1023


//------------------------------------------------------------------------------

#define dimof(x)            (sizeof(x)/sizeof(x[0]))
#define offset(s, f)        FIELD_OFFSET(s, f)
#define fieldsize(s, f)     sizeof(((s*)0)->f)


//------------------------------------------------------------------------------
//
//  Function:  ReadRegistryParams
//
//  This function initializes driver default settings from registry based on
//  table passed as argument.
//
#define PARAM_DWORD             1
#define PARAM_STRING            2
#define PARAM_MULTIDWORD        3
#define PARAM_BIN               4


typedef struct {
    LPTSTR name;
    DWORD  type;
    BOOL   required;
    DWORD  offset;
    DWORD  size;
    PVOID  pDefault;
} DEVICE_REGISTRY_PARAM;

extern "C" DWORD GetDeviceRegistryParams(
    LPCWSTR szContext, VOID *pBase, DWORD count,
    const DEVICE_REGISTRY_PARAM params[]
);
static DWORD GetStringParam(
    HKEY hKey,
    void *pBase,
    const DEVICE_REGISTRY_PARAM *pParam
);
static DWORD GetDWordParam(
    HKEY hKey, VOID *pBase, const DEVICE_REGISTRY_PARAM *pParam
);
static DWORD GetMultiDWordParam(
    HKEY hKey, VOID *pBase, const DEVICE_REGISTRY_PARAM *pParam
);
static DWORD GetBinParam(
    HKEY hKey, VOID *pBase, const DEVICE_REGISTRY_PARAM *pParam
);






//------------------------------------------------------------------------------
// local data structures
//

typedef struct
{
    BOOL        bInitialized;
    DWORD       nSampleRate;
    DWORD       dwSysIntr;
    DWORD       dwSamplingTimeOut;
    BOOL        bTerminateIST;
    HANDLE      hTouchPanelEvent;
    DWORD       dwPowerState;
    DWORD       dwISTPriority;
}TOUCH_DEVICE;


//------------------------------------------------------------------------------
//  Device registry parameters
static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"SampleRate", PARAM_DWORD, FALSE, offset(TOUCH_DEVICE, nSampleRate),
        fieldsize(TOUCH_DEVICE, nSampleRate), (VOID*)DEFAULT_SAMPLE_RATE
    },
    {
        L"Priority256", PARAM_DWORD, FALSE, offset(TOUCH_DEVICE, dwISTPriority),
        fieldsize(TOUCH_DEVICE, dwISTPriority), (VOID*)DEFAULT_THREAD_PRIORITY
    },
    {
        L"SysIntr", PARAM_DWORD, FALSE, offset(TOUCH_DEVICE, dwSysIntr),
        fieldsize(TOUCH_DEVICE, dwSysIntr), (VOID*)SYSINTR_NOP
    }
};

//------------------------------------------------------------------------------
// global variables
//
static TOUCH_DEVICE s_TouchDevice =  {
    FALSE,                                          //bInitialized
    DEFAULT_SAMPLE_RATE,                            //nSampleRate
    SYSINTR_NOP,                                    //dwSysIntr
    0,                                              //dwSamplingTimeOut
    FALSE,                                          //bTerminateIST
    0,                                              //hTouchPanelEvent
    D0,                                             //dwPowerState
    DEFAULT_THREAD_PRIORITY                         //dwISTPriority
};

// Internal functions.
static HRESULT PDDCalibrationThread();
void PDDStartCalibrationThread();

BOOL PDDGetTouchData(UINT32 * pxPos, UINT32 * pyPos);
BOOL PDDGetRegistrySettings( PDWORD );
BOOL PDDInitializeHardware(LPCTSTR pszActiveKey );
VOID PDDDeinitializeHardware( VOID );
VOID  PDDTouchPanelDisable();
BOOL  PDDTouchPanelEnable();
ULONG PDDTouchIST(PVOID   reserved);
void PDDTouchPanelPowerHandler(CEDEVICE_POWER_STATE dx);



extern "C" void BSPTouchInterruptDisable(void);
extern "C" HANDLE BSPTouchAttach(void);
extern "C" BOOL BSPTouchInterruptEnable(void);
extern "C" void BSPTouchInterruptDone(void);
extern "C" TOUCH_PANEL_SAMPLE_FLAGS BSPTouchGetSample(INT *x, INT *y);
extern "C" void BSPTouchPowerHandler(BOOL boff);


//TCH PDD DDSI functions
extern "C" DWORD WINAPI TchPdd_Init(
    LPCTSTR pszActiveKey,
    TCH_MDD_INTERFACE_INFO* pMddIfc,
    TCH_PDD_INTERFACE_INFO* pPddIfc,
    DWORD hMddContext
    );

void WINAPI TchPdd_Deinit(DWORD hPddContext);
void WINAPI TchPdd_PowerUp(DWORD hPddContext);
void WINAPI TchPdd_PowerDown(DWORD hPddContext);
BOOL WINAPI TchPdd_Ioctl(
    DWORD hPddContext,
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut
    );

#endif
