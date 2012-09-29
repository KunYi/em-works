//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  adc_sdk.cpp
//
//  The implementation of ADC driver SDK.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_types.h"
#include "common_macros.h"
#include "common_adc.h"
#include "adc_sdk.h"
#include "adc_ioctl.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT       0
#define ZONEID_INFO       12
#define ZONEID_FUNCTION   13
#define ZONEID_WARN       14
#define ZONEID_ERROR      15

// Debug zone masks
#define ZONEMASK_INIT     (1<<ZONEID_INIT)
#define ZONEMASK_INFO     (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN     (1<<ZONEID_WARN)
#define ZONEMASK_ERROR    (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT         DEBUGZONE(ZONEID_INIT)
#define ZONE_INFO         DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION     DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN         DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR        DEBUGZONE(ZONEID_ERROR)
#endif

#define ADC_GCQCR_DEFAULT   CSP_BITFVAL(ADC_GCQCR_QSM           ,ADC_GCQCR_QSM_FQS)        |\
                            CSP_BITFVAL(ADC_GCQCR_FQS           ,0)                        |\
                            CSP_BITFVAL(ADC_GCQCR_RPT           ,ADC_GCQCR_RPT_REPEAT)    |\
                            CSP_BITFVAL(ADC_GCQCR_LAST_ITEM_ID  ,0)                        |\
                            CSP_BITFVAL(ADC_GCQCR_FIFOWATERMARK ,7)                        |\
                            CSP_BITFVAL(ADC_GCQCR_REPEATWAIT    ,0)                        |\
                            CSP_BITFVAL(ADC_GCQCR_QRST          ,0)                        |\
                            CSP_BITFVAL(ADC_GCQCR_FRST          ,0)                        |\
                            CSP_BITFVAL(ADC_GCQCR_PDMSK         ,ADC_GCQCR_PDMASK_MASK) |\
                            CSP_BITFVAL(ADC_GCQCR_PDCFG         ,0)

#define ADC_DEFAULT_ACQ_TIMEOUT        100

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables

#ifdef DEBUG
extern DBGPARAM dpCurSettings =
{
    _T("ADC"),
    {
        _T("Init"), _T(""), _T(""), _T(""),
        _T(""), _T(""), _T(""), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN | ZONEMASK_INFO
};
#endif

//-----------------------------------------------------------------------------
// Local Variables

static HANDLE g_hADC = INVALID_HANDLE_VALUE;
static HANDLE g_hAdcMutex = NULL;

static T_IOCTL_CFG_ITEM_PARAM g_DefaultChanCfg[CHAN_NUM]=
{
    { GCC0, CSP_BITFVAL(ADC_GCC_SEL_IN,ADC_GCC_SEL_IN_INAUX0) | CSP_BITFVAL(ADC_GCC_SEL_REFN,ADC_GCC_SEL_REFN_AGND) },
    { GCC1, CSP_BITFVAL(ADC_GCC_SEL_IN,ADC_GCC_SEL_IN_INAUX1) | CSP_BITFVAL(ADC_GCC_SEL_REFN,ADC_GCC_SEL_REFN_AGND) },
    { GCC2, CSP_BITFVAL(ADC_GCC_SEL_IN,ADC_GCC_SEL_IN_INAUX2) | CSP_BITFVAL(ADC_GCC_SEL_REFN,ADC_GCC_SEL_REFN_AGND) },
    { GCC3, CSP_BITFVAL(ADC_GCC_SEL_IN,ADC_GCC_SEL_IN_WIPER)  | CSP_BITFVAL(ADC_GCC_SEL_REFN,ADC_GCC_SEL_REFN_AGND) }
};

static DWORD g_DefaultGeneralCfg[CHAN_NUM]=
{
    CSP_BITFVAL( ADC_GCQ_ITEM_0, ADC_GCQ_ITEM_GCC0 ),
    CSP_BITFVAL( ADC_GCQ_ITEM_0, ADC_GCQ_ITEM_GCC1 ),
    CSP_BITFVAL( ADC_GCQ_ITEM_0, ADC_GCQ_ITEM_GCC2 ),
    CSP_BITFVAL( ADC_GCQ_ITEM_0, ADC_GCQ_ITEM_GCC3 )
};

//-----------------------------------------------------------------------------
// Local Functions

static inline BOOL ValidateID(CHAN_ID id)
{
    if (id < sizeof(g_DefaultGeneralCfg)/sizeof(g_DefaultGeneralCfg[0]))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static inline BOOL ValidateRef(POS_REF ref)
{
    switch (ref)
    {
    case EXTREF:
    case INTREF:
        return TRUE;
    default:
        return FALSE;
    }    
}


BOOL AdcInit(void)
{
    // Retrieve an handle on the ADC driver
    g_hADC = CreateFile(TEXT("ADC1:"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); 
    if (g_hADC == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(1, (_T("AdcInit : Could not retrieve an handle on ADC1:\r\n")));
        goto error;
    }

    // Create Mutex to avoid simultaneous acquisitions
    g_hAdcMutex = CreateMutex(NULL, FALSE, L"MUTEX_ADC");
    if (g_hAdcMutex == NULL)
    {
        ERRORMSG(1, (_T("AdcInit : Failed while creating a mutex\r\n")));
        goto cleanup;
    }

    return TRUE;

cleanup:
    CloseHandle(g_hADC);
    g_hADC = INVALID_HANDLE_VALUE;

error:
    return FALSE;
}

void AdcDeinit(void)
{
    if (g_hADC != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hADC);
        g_hADC = INVALID_HANDLE_VALUE;
    }

    if (g_hAdcMutex != NULL)
    {
        CloseHandle(g_hAdcMutex);
        g_hAdcMutex = NULL;
    }
}

BOOL AdcConfigureChannel(CHAN_ID id, DWORD settlingTime, POS_REF pRef, DWORD numSamples)
{
    BOOL bRet = TRUE;

    if ((g_hADC == INVALID_HANDLE_VALUE) || (!ValidateRef(pRef)) || (!ValidateID(id)))
    {
        bRet = FALSE;
        goto exit;
    }

    WaitForSingleObject(g_hAdcMutex, INFINITE);

    // Configure Channel Item
    T_IOCTL_CFG_ITEM_PARAM params;
    params.eItemID        = g_DefaultChanCfg[id].eItemID;
    params.dwItemConfig    = g_DefaultChanCfg[id].dwItemConfig                                                                            |                          
                              CSP_BITFVAL( ADC_GCC_SEL_REFP,      (pRef==EXTREF) ? ADC_GCC_SEL_REFP_EXT_REF : ADC_GCC_SEL_REFP_INT_REF )|    
                              CSP_BITFVAL( ADC_GCC_NOS,              ADC_GCC_NOS(numSamples-1)                                               )|
                              CSP_BITFVAL( ADC_GCC_SETTLING_TIME, ADC_GCC_SETTLING_TIME(settlingTime)                                   );                                  

    bRet = DeviceIoControl(g_hADC,
                           IOCTL_CFG_ITEM,
                           &params,
                           sizeof(T_IOCTL_CFG_ITEM_PARAM),
                           NULL,0,
                           NULL,NULL); 
    if(!bRet)
    {
        ERRORMSG(1, (_T("AdcGetSamples : Failed while configuring the Channel\r\n")));
        goto mutex_release;
    }

mutex_release:
    ReleaseMutex(g_hAdcMutex);
exit:
    return bRet;
}

BOOL AdcGetSamples(CHAN_ID id, UINT16* pBuf, DWORD nbSamples)
{
    BOOL bRet = TRUE;
    DWORD dwOutBytes = 0;

    if ((g_hADC == INVALID_HANDLE_VALUE) || (pBuf == NULL) || (!ValidateID(id)))
    {
        bRet = FALSE;
        goto exit;
    }

    if (nbSamples == 0) 
    {
        goto exit;
    }

    WaitForSingleObject(g_hAdcMutex, INFINITE);

    // Configure ADC General Queue
    T_IOCTL_CFG_QUEUE_PARAM queueParams;
    queueParams.eQueueID      = GENERAL_QUEUE;
    queueParams.dwItem_7_0      = g_DefaultGeneralCfg[id];
    queueParams.dwItem_15_8      = 0;
    queueParams.dwQConfigRreg = ADC_GCQCR_DEFAULT;
    queueParams.dwQIntConfig  = 0xFFFFFFFF;

    bRet = DeviceIoControl(g_hADC,
                           IOCTL_CFG_QUEUE,
                           &queueParams,
                           sizeof(T_IOCTL_CFG_QUEUE_PARAM),
                           NULL,0,
                           NULL,NULL); 
    if(!bRet)
    {
        ERRORMSG(1, (_T("AdcGetSamples : Failed while configuring the General Queue\r\n")));
        goto mutex_release;
    }

    // Triggers ADC acquisition
    T_IOCTL_START_ACQUIRE_SNGL_PARAM startParams;
    startParams.eQueueID = GENERAL_QUEUE;

    bRet = DeviceIoControl(g_hADC,
                           IOCTL_START_ACQUIRE_SNGL,
                           &startParams,
                           sizeof(T_IOCTL_START_ACQUIRE_SNGL_PARAM),
                           NULL,0,
                           NULL,NULL); 
    if(!bRet)
    {
        ERRORMSG(1, (_T("AdcGetSamples : Failed while issuing acquisition start signal\r\n")));
        goto mutex_release;
    }

    // Acquiring samples
    T_IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS_PARAM dataParams;
    dataParams.eQueueID  = GENERAL_QUEUE;
    dataParams.dwTimeout = ADC_DEFAULT_ACQ_TIMEOUT;

    bRet = DeviceIoControl(g_hADC,
                           IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS,
                           &dataParams,
                           sizeof(T_IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS_PARAM),
                           pBuf,
                           nbSamples*sizeof(UINT16),
                           &dwOutBytes,NULL);
    if(!bRet)
    {
        ERRORMSG(1, (_T("Failed while retrieving samples from the ADC General queue.\r\n")));
        goto mutex_release;
    }

    // Stop ADC acquisition
    T_IOCTL_STOP_ACQUIRE_SNGL_PARAM stopParams;
    stopParams.eQueueID = GENERAL_QUEUE;

    bRet = DeviceIoControl(g_hADC,
                           IOCTL_STOP_ACQUIRE_SNGL,
                           &stopParams,
                           sizeof(T_IOCTL_STOP_ACQUIRE_SNGL_PARAM),
                           NULL,0,
                           NULL,NULL); 
    if(!bRet)
    {
        ERRORMSG(1, (_T("AdcGetSamples : Failed while issuing acquisition stop signal\r\n")));
        goto mutex_release;
    }

    if (dwOutBytes != (nbSamples*sizeof(UINT16)))
    {
        bRet = FALSE;
        goto mutex_release;
    }

mutex_release:
    ReleaseMutex(g_hAdcMutex);
exit:
    return bRet;
}

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the ADC SDK module.  This
// function is called when processed and threads attach and detach from this
// module.
//
// Parameters:
//      hInstDll
//           [in] The handle to this module.
//
//      dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//      lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the ADC is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
extern "C"
BOOL WINAPI
DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HMODULE)hInstDll);
            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS ATTACH TO ADC SDK *****\r\n")));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS DETACH FROM ADC SDK *****\r\n")));
            break;
    }

    // return TRUE for success
    return TRUE;
}
