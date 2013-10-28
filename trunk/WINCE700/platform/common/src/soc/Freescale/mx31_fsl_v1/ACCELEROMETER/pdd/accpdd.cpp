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
//
//  File: accelerometer.cpp
//
//
#include <windows.h>
#include <strsafe.h>
#include <acc_ddsi.h>
#include <accapi.h>
#include <acc_ioctl.h>
#include "accpdd.h"

#define CEDDK_PDD
#include <ceddk.h>
#include <acc.h>
#include <accmain.h>

extern bool  ACCInitialize(void);
extern void  ACCDeinitialize(void);
extern bool  ACC_SetMode(ACC_MODE eACCMode);
extern bool  ACC_SetGSel(ACC_GSEL eACCGSel);
extern bool  ACC_SetLDTH(INT8 iLDTH);
extern bool  ACC_SetPDTH(INT8 iPDTH);
extern bool  ACC_SetLDPOL(ACC_POL ePOL);
extern bool  ACC_SetPDPOL(ACC_POL ePOL);
extern bool  ACC_SetDFBW(ACC_DFBW eDFBW);
extern bool  ACC_SetTHOPT(ACC_THRESHOLD_OPT eTHOPT);
extern bool  ACC_SetDetection(BYTE byDetection);
extern bool  ACC_SetXOffset(INT16 iXOffset);
extern bool  ACC_SetYOffset(INT16 iYOffset);
extern bool  ACC_SetZOffset(INT16 iZOffset);
extern bool  ACC_SetLatencyTime(BYTE byLT);
extern bool  ACC_SetPulseDuration(BYTE byPD);
extern bool  ACC_SetTimeWindow(BYTE byTW);
extern bool  ACC_SetOutputWidth(ACC_OUTPUT_WIDTH eOutputWidth);
extern bool  ACC_GetMode(ACC_MODE* peMode);
extern bool  ACC_GetGSel(ACC_GSEL* peGSel);
extern bool  ACC_GetLDTH(INT8* piLDTH);
extern bool  ACC_GetPDTH(INT8* piPDTH);
extern bool  ACC_GetLDPOL(ACC_POL* pePOL);
extern bool  ACC_GetPDPOL(ACC_POL* pePOL);
extern bool  ACC_GetLatencyTime(PBYTE pbyLT);
extern bool  ACC_GetPulseDuration(PBYTE pbyPD);
extern bool  ACC_GetTimeWindow(PBYTE pbyTW);
extern bool  ACC_GetOutputWidth(ACC_OUTPUT_WIDTH* peOutputWidth);
extern bool  ACC_GetOutput(ACC_OUTPUT* pOutput);
extern bool  ACC_GetStatus(ACC_STATUS* pStatus);
extern bool  ACC_GetDFBW(ACC_DFBW* peDFBW);
extern bool  ACC_GetTHOPT(ACC_THRESHOLD_OPT* peTHOPT);
extern bool  ACC_GetXOffset(INT16* piXOffset);
extern bool  ACC_GetYOffset(INT16* piYOffset);
extern bool  ACC_GetZOffset(INT16* piZOffset);
extern bool  ACC_SetDRPD(ACC_DRPD eDRPD);

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
#ifndef SHIP_BUILD
DBGPARAM dpCurSettings = {
    TEXT("ACC"), {
        TEXT("Errors"),TEXT("Warnings"),TEXT("Init"),TEXT("Deinit"),
            TEXT("Open"),TEXT("Close"),TEXT("IOCtl"),TEXT("Thread"),
            TEXT("Function"),TEXT("Samples"),TEXT("Orientation"),TEXT("Undefined"),
            TEXT("Undefined"),TEXT("Undefined"),TEXT("Undefined"),TEXT("Verbose") },
    0x0001 | // ZONE_ERROR
    0x0002 | // ZONE_WARN 
    0x0000
};
#endif

typedef struct _ACC_PDD_CONTEXT
{
    DWORD dwMddContext;
    PFN_ACC_MDD_PROCESSSAMPLE pfnMddProcessSample;

    PFN_ACC_MDD_POWER_EVENT pfnMddPowerEvent;
    BOOL fPowerEventingEnabled;
    volatile BOOL fShutDown;
    HANDLE hPowerOn;

    HANDLE hNotifyThread;
    volatile DWORD dwIntervalMs;

    ACC_DATA curSample ;
    DWORD dwHwMode;
    DWORD dwCaps;
} ACC_PDD_CONTEXT;

//------------------------------------------------------------------------------
// file-scope variables

// Simulate the 3 Chassis Spec'd ranges +/-2, 4, & 8 G
// Choosing resolutions that decrease as the total range increases, but that are 
// still arbitrary
static const ACC_RANGE_RESOLUTION m_aRangeRes[] = 
{
    { (float) -8, (float) 8, (float) 0.0625 },
    { (float) -2, (float) 2, (float) 0.015625 },
    { (float) -4, (float) 4, (float) 0.03125 }
};

//------------------------------------------------------------------------------
//
//  Function:  AccPddPollingThread
//
//  Poll the device at a rate a rate specified by m_dwIntervalMs and send 
//  all samples to the MDD driver.
//
void AccPddPollingThread(LPVOID pPddContext)
{
    DWORD dwErr;
    static DWORD dwIdx = 0;
    DWORD dwSleepTime = 0;
    ACC_OUTPUT output;

    ACC_PDD_CONTEXT *pPdd = (ACC_PDD_CONTEXT*) pPddContext;

    for ( ; ; )
    {
        // Wait for device to either be powered on or the shutdown event is signalled.
        dwErr = WaitForSingleObject(pPdd->hPowerOn, INFINITE);
        if (dwErr != WAIT_OBJECT_0)
        {
            DEBUGMSG(ZONE_ERROR, (L"accpdd: WaitForSingleObject failed. GLE=0x%x\r\n", GetLastError()));
            break;
        }
        if (pPdd->fShutDown)
        {
            //shut down signalled
            break;
        }

        if(!ACC_GetOutput(&output))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("ACC_IOControl: IOCTL_ACC_GET_OUTPUT failed\r\n")));
        }

        pPdd->curSample.x = output.x;
        pPdd->curSample.y = output.y;
        pPdd->curSample.z = output.z; 
        pPdd->curSample.hdr.dwTimeStampMs = GetTickCount();

        pPdd->pfnMddProcessSample(pPdd->dwMddContext, &pPdd->curSample );

        
        // Calculate Sleep time: We know that tick count is running at a 1ms
        // interval. So we will use it as our synchronization source to trigger
        // the sampling of the sensor and pushing data up the stack. Because
        // the sensor access and time to push the sample up the stack is not 
        // fixed we need to recalculate the sleep time. By scheduling the 
        // sample on an even interval with tick count the sleep time is 
        // easy to recalculate.
        dwSleepTime = pPdd->dwIntervalMs - (GetTickCount() % pPdd->dwIntervalMs);
        DEBUGMSG(ZONE_STATE, (L"ACC Polling thread sleeping %ums\r\n",
            dwSleepTime));
        Sleep( dwSleepTime );
    }
}

BOOL PDDGetInfo(__out PDD_INFO *pPDDInfo)
{
    BOOL fOk;

    if (pPDDInfo == NULL || pPDDInfo->cbSize != sizeof(PDD_INFO))
    {
        fOk = FALSE;
    }
    else
    {
        pPDDInfo->dwPDDVersion = ACC_DRIVER_VERSION;
        pPDDInfo->dwInterfaceVersion = ACC_PDD_CURRENT_VERSION;
        wcsncpy_s(pPDDInfo->szChipset, _countof(pPDDInfo->szChipset), L"imx31", 6); 
        wcsncpy_s(pPDDInfo->szManufacturer, _countof(pPDDInfo->szManufacturer), L"FreeScale", 10); 

        fOk = TRUE;
    }
    return fOk;
}

BOOL PDDDeInit(DWORD dwContext)
{
    BOOL rc = FALSE;
    ACC_PDD_CONTEXT *pPdd = (ACC_PDD_CONTEXT*) dwContext;

    if (pPdd)
    {
        pPdd->fShutDown = TRUE;
        SetEvent(pPdd->hPowerOn);    //wake up the polling thread
        WaitForSingleObject( pPdd->hNotifyThread, INFINITE );
        CloseHandle(pPdd->hPowerOn);
        CloseHandle(pPdd->hNotifyThread);

        LocalFree(pPdd);
        pPdd = NULL;
        rc = TRUE;
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    ACCDeinitialize();
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  PDDIoControl
//
//  Required IOCTL stub.
//  If an IOCTL is implemented, PDD is required to validate and marshal user
//  buffers appropriately
//
BOOL WINAPI PDDIoControl (
                          __in DWORD PDDContext,
                          __in DWORD dwCode,
                          __inout PBYTE pBufIn,
                          __in DWORD dwLenIn,
                          __inout PBYTE pBufOut,
                          __in DWORD dwLenOut,
                          __inout PDWORD pdwActualOut
                          )
{
    INT8*              piLDTH;
    INT8*              piPDTH;
    BYTE*              pbyLT;
    BYTE*              pbyPD;
    BYTE*              pbyTW;
    BYTE*              pbyDection;
    INT16*             pOffset;
    ACC_MODE*          peMode;
    ACC_GSEL*          peGSel;
    ACC_OUTPUT*        pOutput;
    ACC_POL*           pACCPOL;
    ACC_DFBW*          peDFBW;
    ACC_OUTPUT_WIDTH*  peOutputWid;
    ACC_THRESHOLD_OPT* peTHOPT;
    ACC_STATUS*        pStatus;
    ACC_DRPD*          peDRPD;
    BOOL bRet = FALSE;

    switch (dwCode)
    {
    case IOCTL_ACC_SET_MODE:
        peMode = (ACC_MODE*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(ACC_MODE))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_MODE!\r\n")));
            break;
        }
        bRet = ACC_SetMode(*peMode);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_MODE %d failed\r\n"),*peMode));
        }

        break;
    case IOCTL_ACC_SET_GSEL:
        peGSel = (ACC_GSEL*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(ACC_GSEL))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_GSEL!\r\n")));
            break;
        }
        bRet = ACC_SetGSel(*peGSel);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_GSEL %d failed\r\n"),*peGSel));
        }

        break;
    case IOCTL_ACC_SET_LDTH:
        piLDTH = (INT8*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(INT8))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_LDTH!\r\n")));
            break;
        }
        bRet = ACC_SetLDTH(*piLDTH);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_LDTH %d failed\r\n"),*piLDTH));
        }

        break;
    case IOCTL_ACC_SET_PDTH:
        piPDTH = (INT8*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(INT8))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_PDTH!\r\n")));
            break;
        }
        bRet = ACC_SetPDTH(*piPDTH);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_PDTH %d failed\r\n"),*piPDTH));
        }

        break;
    case IOCTL_ACC_SET_LDPOL:
        pACCPOL = (ACC_POL*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(ACC_POL))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_LDPOL!\r\n")));
            break;
        }
        bRet = ACC_SetLDPOL(*pACCPOL);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_LDPOL %d failed\r\n"),*pACCPOL));
        }

        break;
    case IOCTL_ACC_SET_PDPOL:
        pACCPOL = (ACC_POL*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(ACC_POL))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_PDPOL!\r\n")));
            break;
        }
        bRet = ACC_SetPDPOL(*pACCPOL);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_PDPOL %d failed\r\n"),*pACCPOL));
        }

        break;
    case IOCTL_ACC_SET_DRPD:
        peDRPD = (ACC_DRPD*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(ACC_DRPD))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_DRPD!\r\n")));
            break;
        }
        bRet = ACC_SetDRPD(*peDRPD);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_DRPD %d failed\r\n"),*peDRPD));
        }

        break;                  
    case IOCTL_ACC_SET_OUTPUTWIDTH:
        peOutputWid = (ACC_OUTPUT_WIDTH*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(ACC_OUTPUT_WIDTH))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_OUTPUTWIDTH!\r\n")));
            break;
        }
        bRet = ACC_SetOutputWidth(*peOutputWid);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: ACC_SetOutputWidth %d failed\r\n"),*peOutputWid));
        }

        break;
    case IOCTL_ACC_SET_DFBW:
        peDFBW = (ACC_DFBW*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(ACC_DFBW))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_DFBW!\r\n")));
            break;
        }
        bRet = ACC_SetDFBW(*peDFBW);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_DFBW %d failed\r\n"),*peDFBW));
        }
        break;
    case IOCTL_ACC_SET_THOPT:
        peTHOPT = (ACC_THRESHOLD_OPT*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(ACC_THRESHOLD_OPT))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_THOPT!\r\n")));
            break;
        }
        bRet = ACC_SetTHOPT(*peTHOPT);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_THOPT %d failed\r\n"),*peTHOPT));
        }
        break;

    case IOCTL_ACC_SET_DETECTION:
        pbyDection = (BYTE*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(BYTE))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_XDETECTION!\r\n")));
            break;
        }
        bRet = ACC_SetDetection(*pbyDection);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_XDETECTION %d failed\r\n"),*pbyDection));
        }
        break;

    case IOCTL_ACC_SET_XOFFSET:
        pOffset = (INT16*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(INT16))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_XOFFSET!\r\n")));
            break;
        }
        bRet = ACC_SetXOffset(*pOffset);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_XOFFSET %d failed\r\n"),*pOffset));
        }
        break;
    case IOCTL_ACC_SET_YOFFSET:
        pOffset = (INT16*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(INT16))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_YOFFSET!\r\n")));
            break;
        }
        bRet = ACC_SetYOffset(*pOffset);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_YOFFSET %d failed\r\n"),*pOffset));
        }
        break;
    case IOCTL_ACC_SET_ZOFFSET:
        pOffset = (INT16*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(INT16))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_ZOFFSET!\r\n")));
            break;
        }
        bRet = ACC_SetZOffset(*pOffset);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_ZOFFSET %d failed\r\n"),*pOffset));
        }
        break;
    case IOCTL_ACC_SET_LATENCYTIME:
        pbyLT = (BYTE*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(BYTE))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_LATENCYTIME!\r\n")));
            break;
        }
        bRet = ACC_SetLatencyTime(*pbyLT);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_LATENCYTIME %d failed\r\n"),*pbyLT));
        }
        break;
    case IOCTL_ACC_SET_PULSEDURATION:
        pbyPD = (BYTE*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(BYTE))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_PULSEDURATION!\r\n")));
            break;
        }
        bRet = ACC_SetPulseDuration(*pbyPD);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_PULSEDURATION %d failed\r\n"),*pbyPD));
        }
        break;
    case IOCTL_ACC_SET_TIMEWINDOW:
        pbyTW = (BYTE*)pBufIn;
        if (pBufIn == NULL || dwLenIn < sizeof(BYTE))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_SET_TIMEWINDOW!\r\n")));
            break;
        }
        bRet = ACC_SetTimeWindow(*pbyTW);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_SET_TIMEWINDOW %d failed\r\n"),*pbyTW));
        }
        break;
    case IOCTL_ACC_GET_MODE:
        peMode = (ACC_MODE*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(ACC_MODE))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_MODE!\r\n")));
            break;
        }
        bRet = ACC_GetMode(peMode);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_MODE failed\r\n")));
        }

        break;
    case IOCTL_ACC_GET_GSEL:
        peGSel = (ACC_GSEL*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(ACC_GSEL))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_GSEL!\r\n")));
            break;
        }
        bRet = ACC_GetGSel(peGSel);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_GSEL failed\r\n")));
        }
        break;

    case IOCTL_ACC_GET_LDTH:
        piLDTH = (INT8*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(INT8))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_LDTH!\r\n")));
            break;
        }
        bRet = ACC_GetLDTH(piLDTH);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_LDTH failed\r\n")));
        }

        break;
    case IOCTL_ACC_GET_PDTH:
        piPDTH = (INT8*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(INT8))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_PDTH!\r\n")));
            break;
        }
        bRet = ACC_GetPDTH(piPDTH);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_PDTH failed\r\n")));
        }

        break;
    case IOCTL_ACC_GET_LDPOL:
        pACCPOL = (ACC_POL*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(ACC_POL))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_LDPOL!\r\n")));
            break;
        }
        bRet = ACC_GetLDPOL(pACCPOL);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_LDPOL failed\r\n")));
        }

        break;
    case IOCTL_ACC_GET_PDPOL:
        pACCPOL = (ACC_POL*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(ACC_POL))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_PDPOL!\r\n")));
            break;
        }
        bRet = ACC_GetPDPOL(pACCPOL);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_PDPOL failed\r\n")));
        }

        break;

    case IOCTL_ACC_GET_OUTPUTWIDTH:
        peOutputWid = (ACC_OUTPUT_WIDTH*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(ACC_OUTPUT_WIDTH))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_OUTPUTWIDTH!\r\n")));
            break;
        }
        bRet = ACC_GetOutputWidth(peOutputWid);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_OUTPUTWIDTH failed\r\n")));
        }

        break;
    case IOCTL_ACC_GET_DFBW:
        peDFBW = (ACC_DFBW*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(ACC_DFBW))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_DFBW!\r\n")));
            break;
        }
        bRet = ACC_GetDFBW(peDFBW);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_DFBW failed\r\n")));
        }
        break;

    case IOCTL_ACC_GET_THOPT:
        peTHOPT = (ACC_THRESHOLD_OPT*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(ACC_THRESHOLD_OPT))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_THOPT!\r\n")));
            break;
        }
        bRet = ACC_GetTHOPT(peTHOPT);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_THOPT failed\r\n")));
        }
        break;

    case IOCTL_ACC_GET_XOFFSET:
        pOffset = (INT16*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(INT16))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_XOFFSET!\r\n")));
            break;
        }
        bRet = ACC_GetXOffset(pOffset);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_XOFFSET failed\r\n")));
        }
        break;
    case IOCTL_ACC_GET_YOFFSET:
        pOffset = (INT16*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(INT16))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_YOFFSET!\r\n")));
            break;
        }
        bRet = ACC_GetYOffset(pOffset);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_YOFFSET failed\r\n")));
        }
        break;
    case IOCTL_ACC_GET_ZOFFSET:
        pOffset = (INT16*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(INT16))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_ZOFFSET!\r\n")));
            break;
        }
        bRet = ACC_GetZOffset(pOffset);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_ZOFFSET failed\r\n")));
        }
        break;
    case IOCTL_ACC_GET_STATUS:
        pStatus = (ACC_STATUS*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(ACC_STATUS))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_STATUS!\r\n")));
            break;
        }
        bRet = ACC_GetStatus(pStatus);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_STATUS failed\r\n")));
        }
        break;

    case IOCTL_ACC_GET_LATENCYTIME:
        pbyLT = (BYTE*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(BYTE))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_LATENCYTIME!\r\n")));
            break;
        }
        bRet = ACC_GetLatencyTime(pbyLT);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_LATENCYTIME failed\r\n")));
        }
        break;
    case IOCTL_ACC_GET_PULSEDURATION:
        pbyPD = (BYTE*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(BYTE))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_PULSEDURATION!\r\n")));
            break;
        }
        bRet = ACC_GetPulseDuration(pbyPD);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_PULSEDURATION failed\r\n")));
        }
        break;
    case IOCTL_ACC_GET_TIMEWINDOW:
        pbyTW = (BYTE*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(BYTE))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_TIMEWINDOW!\r\n")));
            break;
        }
        bRet = ACC_GetTimeWindow(pbyTW);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_TIMEWINDOW failed\r\n")));
        }
        break;
    case IOCTL_ACC_GET_OUTPUT:
        pOutput = (ACC_OUTPUT*)pBufOut;
        if (pBufOut == NULL || dwLenOut < sizeof(ACC_OUTPUT))
        {
            ERRORMSG(TRUE, (TEXT("Invalid parameter for IOCTL_ACC_GET_OUTPUT!\r\n")));
            break;
        }
        bRet = ACC_GetOutput(pOutput);
        if(!bRet)
        {
            DEBUGMSG(TRUE, (TEXT("ACC_IOControl: IOCTL_ACC_GET_OUTPUT failed\r\n")));
        }
        break;
    default:
        break;
    }
    return bRet;
}

//------------------------------------------------------------------------------
//
//  Function:  PDDCapabilitiesGet
//
//  Return the capabilites of the PDD/HW
//      ACC_PDD_CAPABILITIES_HWORIENTATION_DETECT: indicates the PDD can perform
//      orientation detection without streaming data to the MDD. When
//      a change in orientation is detected the PDD will send up a
//      MESSAGE_ACCELEROMETER_3D_DEVICE_ORIENTATION_CHANGE event.
//
DWORD PDDCapabilitiesGet(DWORD dwContext, __in PDWORD pdwCaps)
{
    ACC_PDD_CONTEXT *pPdd = (ACC_PDD_CONTEXT*) dwContext;

    if (pPdd == NULL || pdwCaps == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *pdwCaps = pPdd->dwCaps;
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  PDDPowerOn
//
//  Power on the device.  
//
DWORD PDDPowerOn(DWORD dwContext)
{
    ACC_PDD_CONTEXT *pPdd = (ACC_PDD_CONTEXT*) dwContext;

    if (pPdd == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    else
    {
        if (pPdd->dwHwMode == ACC_HW_MODE_STREAMING)
            ACC_SetMode(ACC_MODE_MEASUREMENT);

        SetEvent(pPdd->hPowerOn);

        // if enabling, log the current sensor state - in simple PDDs, only "on" and "off" are supported
        if(pPdd->fPowerEventingEnabled) {
            pPdd->pfnMddPowerEvent(pPdd->dwMddContext, GetTickCount(), 
                PDD_POWER_STATE_L0, SENSOR_MILLIWATTS_UNKNOWN);
        }
        return STATUS_SUCCESS;
    }
}

//------------------------------------------------------------------------------
//
//  Function:  PDDPowerOff
//
//  Power off the device
//
DWORD PDDPowerOff(DWORD dwContext)
{
    ACC_PDD_CONTEXT *pPdd = (ACC_PDD_CONTEXT*) dwContext;

    if (pPdd == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    else
    {
        ACC_SetMode(ACC_MODE_STANDBY);

        ResetEvent(pPdd->hPowerOn);

        // if enabling, log the current sensor state - in simple PDDs, only "on" and "off" are supported
        if(pPdd->fPowerEventingEnabled) {
            pPdd->pfnMddPowerEvent(pPdd->dwMddContext, GetTickCount(), 
                PDD_POWER_STATE_L4, SENSOR_MILLIWATTS_UNKNOWN);
        }
        return STATUS_SUCCESS;
    }
}

//------------------------------------------------------------------------------
//
//  Function:  PDDPowerEventingEnable
//
//  Called by the MDD to enable logging of internal sensor power states
//
//      dwContext: IN: driver stack/PDD context
// 
//      fEnable: TRUE Logging is to be turned on
//           FALSE: Logging is to be turned off
//
//  When enabled, immediately logs the internal power state of the sensor.
//
//  Returns:
//      ERROR_SUCCESS or error code
//
DWORD  PDDPowerEventingEnable(
                              DWORD dwContext, 
                              BOOL fEnable)
{
    DWORD dwRc = ERROR_INVALID_PARAMETER;
    ACC_PDD_CONTEXT *pPdd = (ACC_PDD_CONTEXT*) dwContext;

    DEBUGMSG(ZONE_FUNCTION, (L"PDDPowerEventingEnable+\r\n"));

    if (pPdd == NULL)
    {
        RETAILMSG(ZONE_ERROR, (L"ERROR! PDDPowerEventingEnable\r\n"));
        goto Error;
    }

    // enable or disable logging
    pPdd->fPowerEventingEnabled = fEnable;

    // if enabling, log the current sensor state - in simple PDDs, only "on" and "off" are supported
    if(pPdd->fPowerEventingEnabled) {
        // get the current power status
        BOOL fPowerOn = FALSE;
        if(WaitForSingleObject(pPdd->hPowerOn, 0) == WAIT_OBJECT_0) {
            fPowerOn = TRUE;
        }

        pPdd->pfnMddPowerEvent(pPdd->dwMddContext,
            GetTickCount(), fPowerOn ? PDD_POWER_STATE_L0 : PDD_POWER_STATE_L4, 
            SENSOR_MILLIWATTS_UNKNOWN);
    }

    dwRc = ERROR_SUCCESS;

Error:    
    DEBUGMSG(ZONE_FUNCTION, (L"PDDPowerEventingEnable - %u\r\n",dwRc));
    return dwRc;
}

//------------------------------------------------------------------------------
//  
//  Function:  PDDModeSet
//
//  This function updates the speed the PDD runs the harware at
//    ACC_HW_MODE_UNKNOWN: invalid value - 
//    ACC_HW_MODE_HWORIENTATION_DETECT: If pdd power is on, immediately change to 
//      orientation detect and stop streaming data to the MDD. If Pdd power is
//      off then wait for power on then start detecting orientation
//    ACC_HW_MODE_STREAMING: if PDD power is on, immediately change
//      to stream ACC samples to the MDD using the dwInvervalMs rate.
//      if PDD power is off, wait until the power is turned on.
//
//  ERROR_SUCCESS indicates success
//
DWORD PDDModeSet(DWORD dwContext, ACC_HW_MODE hwMode, DWORD dwIntervalMs)
{
    ACC_PDD_CONTEXT *pPdd = (ACC_PDD_CONTEXT*) dwContext;
    DWORD dwErr = ERROR_INVALID_PARAMETER;

    if (pPdd == NULL)
    {
        return dwErr;
    }

    // Only support streaming mode
    if (hwMode == ACC_HW_MODE_STREAMING && 
        (pPdd->dwCaps & ACC_HW_MODE_STREAMING) != 0)
    {
        pPdd->dwHwMode = ACC_MODE_MEASUREMENT;
        pPdd->dwIntervalMs = dwIntervalMs;
        dwErr = ERROR_SUCCESS;
    }

    return dwErr;
}


//------------------------------------------------------------------------------
//  
//  Function:  PDDRangeGet
//
//  This function returns a list of ranges & resolutions the PDD supports or 
//  the number of entries in the list so caller can allocate correct size
//
//  This is called by the MDD immediately after PDDInit
//
DWORD PDDRangeGet(
                  DWORD dwContext, 
                  __out_opt ACC_RANGE_RESOLUTION *paRange, 
                  __out DWORD *pcRange)
{
    ACC_PDD_CONTEXT *pPdd = (ACC_PDD_CONTEXT*) dwContext;

    if (pPdd == NULL || pcRange == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    else if (paRange == NULL)
    {
        *pcRange = _countof(m_aRangeRes);
    }
    else
    {
        DWORD cbSize = sizeof(m_aRangeRes);
        if (cbSize > (*pcRange * sizeof(ACC_RANGE_RESOLUTION)))
        {
            return ERROR_INVALID_PARAMETER;
        }
        memcpy(paRange, m_aRangeRes, cbSize);
        *pcRange = _countof(m_aRangeRes);
    }
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//  
//  Function:  PDDRangeSet
//
//  This function sets the Accelerometers range and resolution
//
//  ERROR_SUCCESS indicates success
//
DWORD PDDRangeSet(DWORD dwContext, __in ACC_RANGE_RESOLUTION *pRange)
{
    DWORD returnValue = ERROR_SUCCESS;
    BOOL fValidRange = FALSE;
    DWORD index = 0;

    ACC_PDD_CONTEXT *pPdd = (ACC_PDD_CONTEXT*) dwContext;

    if (pPdd == NULL || pRange == NULL)
    {
        returnValue = ERROR_INVALID_PARAMETER;
        goto Error;
    }

    for (index = 0; index < _countof(m_aRangeRes); index++)
    {
        if (memcmp(&m_aRangeRes[index], pRange, sizeof(ACC_RANGE_RESOLUTION)) == 0)
        {
            fValidRange = TRUE;
            break;
        }
    }

    if (fValidRange == TRUE)
    {
        if(!ACC_SetGSel((ACC_GSEL)index)) returnValue = ERROR_GEN_FAILURE;
    }
    else
    {
        returnValue = ERROR_INVALID_PARAMETER;
    }

Error:
    return returnValue;
}


//------------------------------------------------------------------------------
//  
//  Function:  PDDInit
//
//  This function is called by the MDD to initialize the PDD.
//
//  dwMDDContext: MDD Context
//  pPDDInterface: populated by the PDD the MDD uses this structure to call
//      PDD API's
//  pMDDCallbackInfo: used by the PDD to asynchronously call back into the MDD
//
//  Return: PDDContext
//      NULL -> failure
//      non-NULL -> success
//
DWORD PDDInit(DWORD dwMDDContext,
              __in_z WCHAR *pszPDDRegistryKey,
              __inout PDD_INTERFACE *pPDDInterface,
              __in MDD_CALLBACK_INFO* pMDDCallbackInfo)
{
    DWORD dwErr;
    ACCELEROMETER_PDD_FUNCTIONS* pDDInterface = 
        (ACCELEROMETER_PDD_FUNCTIONS*) pPDDInterface;
    ACCELEROMETER_CALLBACK_INFO* pCallbackInfo = 
        (ACCELEROMETER_CALLBACK_INFO*) pMDDCallbackInfo;

    ACC_PDD_CONTEXT *pPdd = 
        (ACC_PDD_CONTEXT*) LocalAlloc(LPTR, sizeof(ACC_PDD_CONTEXT));
    if (pPdd == NULL)
    {
        RETAILMSG(ZONE_ERROR, (L"ERROR! AccPdd: PDDInit OOM\r\n"));
        return NULL;
    }

    pPdd->hNotifyThread = INVALID_HANDLE_VALUE;
    pPdd->dwIntervalMs = MAXDWORD;
    pPdd->dwHwMode = ACC_HW_MODE_STREAMING;
    pPdd->dwCaps = ACC_HW_MODE_STREAMING;

    if (pDDInterface != NULL && pCallbackInfo != NULL &&
        pDDInterface->cbSize == sizeof(ACCELEROMETER_PDD_FUNCTIONS) &&
        pCallbackInfo->cbSize == sizeof(ACCELEROMETER_CALLBACK_INFO))
    {
        pDDInterface->pfnPDDGetInfo = PDDGetInfo;
        pDDInterface->pfnPDDDeInit = PDDDeInit;
        pDDInterface->pfnPDDIoControl = PDDIoControl;
        pDDInterface->pfnPddCapabilitiesGet = PDDCapabilitiesGet;
        pDDInterface->pfnPddPowerOn = PDDPowerOn;
        pDDInterface->pfnPddPowerOff = PDDPowerOff;
        pDDInterface->pfnPddModeSet = PDDModeSet;
        pDDInterface->pfnPddRangeGet = PDDRangeGet;
        pDDInterface->pfnPddRangeSet = PDDRangeSet;
        pDDInterface->pfnPddPowerEventingEnable = PDDPowerEventingEnable;

        pPdd->dwMddContext = dwMDDContext;

        pPdd->pfnMddProcessSample = pCallbackInfo->pfnAccMddProcessSample;
        pPdd->pfnMddPowerEvent = pCallbackInfo->pfnAccMddPowerEvent;
        pPdd->fShutDown = FALSE;

        pPdd->hPowerOn = CreateEvent(NULL, TRUE, FALSE, NULL);

        pPdd->hNotifyThread = CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)&AccPddPollingThread,
            pPdd,
            0,
            NULL);

        BOOL rc = ACCInitialize();

        if (pPdd->dwMddContext && pPdd->pfnMddProcessSample && pPdd->hPowerOn && pPdd->hNotifyThread && rc)
        {
            dwErr = ERROR_SUCCESS;
        }
        else
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
        dwErr = ERROR_INVALID_PARAMETER;
    }

    if (dwErr != ERROR_SUCCESS)
    {
        PDDDeInit((DWORD)pPdd);
        return NULL;
    }
    else
    {
        return (DWORD) pPdd;
    }
}
