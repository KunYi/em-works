//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  display_xec.cpp
//
//  Display Interface Layer functions providing interface between
//  high-level DirectDraw API implementation and low-level IPU register
//  access functions, specified for XEC related functions.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201)
#include <windows.h>
#include <types.h>
#include <winddi.h>
#include <ceddk.h>
#include <cmnintrin.h>
#include <NKIntr.h>
#include <winddi.h>
#include <ddgpe.h>
#pragma warning(pop)

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"
#include "display.h"
#include "dp.h"
#include "prp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables
extern PANEL_INFO *g_pDI0PanelInfo;
extern PANEL_INFO *g_pDI1PanelInfo;

//-----------------------------------------------------------------------------
// Defines
#define DLS_UI_ACTIVE TEXT("DLSUIActive")
#define DLS_VIDEO_OFF TEXT("DLSVideoOff")
#define DLS_ENABLE    TEXT("DLSEnable")
#define DLS_DISABLE   TEXT("DLSDisable")
#define DLS_FLIP      TEXT("DLSFlip")

#define UISATAT_TIME_OUT    (3000)
#define DLS_FRAME_STEP      (6)

//-----------------------------------------------------------------------------
// Types
typedef struct {
    UINT8   *pSourceFrameBuffer;
    int     iSrcSurfaceWidth;
    int     iSrcSurfaceHeight;
    RECT    srcRect;
} NewFrame_msg;

typedef struct {
    UINT16  A[3];
    UINT16  B[3];
}CSCParameters, *pCSCParameters;


//-----------------------------------------------------------------------------
// Global Variables
HANDLE hUIActive;
HANDLE hDLSUIActive;
HANDLE hDLSVideoLayerOff;

HANDLE hWriteFliptoDLSQueue; 
HANDLE hReadFliptoDLSQueue; 


//-----------------------------------------------------------------------------
// Local Variables
BOOL bDLSEnabled    = FALSE;    // FALSE : DLS Disabled
BOOL bUIActive      = TRUE;     // TRUE  : UI Active

DP_CH_TYPE DpChannel = DP_CHANNEL_SYNC;
dpCSCConfigData DpCSCConfigData;


//-----------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: Display_SetVideoOffEvent
//
// This function sets the VideoOff event.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_SetVideoOffEvent(void)
{
    if(bDLSEnabled)
    {
        RETAILMSG(1, (TEXT("Video Off IPUv3\r\n")));
        SetEvent(hDLSVideoLayerOff);
    }
}

//------------------------------------------------------------------------------
//
// Function: Display_SetUIEvent
//
// This function sets the UI event.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_SetUIEvent(void)
{
    if(bDLSEnabled)
    {
        SetEvent(hUIActive);

        SetEvent(hDLSUIActive);
    }
}


//------------------------------------------------------------------------------
//
// Function: DLSUIStateThread
//
// This function detects the UI state.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DLSUIStateThread(void)
{
    DWORD dwResult;
    
    for( ; ; )
    {
        dwResult = WaitForSingleObject(hUIActive, UISATAT_TIME_OUT);
        if(dwResult == WAIT_OBJECT_0)
        {
            if(!bUIActive)
            {
                bUIActive = TRUE;
            }
        }
        else if(dwResult == WAIT_TIMEOUT)
        {
            if(bUIActive)
            {
                bUIActive = FALSE;
            }
        }
    }
}

//------------------------------------------------------------------------------
//
// Function: DLSStateThread
//
// This function detects the DLS enabled or disabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DLSStateThread(void)
{
    DWORD   dwResult;
    HANDLE  hEvents[2];

    // Open DLS Enable or Disable Event
    HANDLE hDLSEnabled = OpenEvent(EVENT_ALL_ACCESS, FALSE, DLS_ENABLE);
    if(hDLSEnabled == NULL)
    {
        RETAILMSG(1, (TEXT("Open DLS_ENABLE Event Fail\r\n")));
        return;
    }

    HANDLE hDLSDisabled = OpenEvent(EVENT_ALL_ACCESS, FALSE, DLS_DISABLE);
    if(hDLSDisabled == NULL)
    {
        RETAILMSG(1, (TEXT("Open DLS_DISABLE Event Fail\r\n")));
        return;
    }

    hEvents[0] = hDLSEnabled;
    hEvents[1] = hDLSDisabled;
    
    for( ; ; )
    {
        dwResult = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
        if(dwResult == WAIT_OBJECT_0 + 0)
        {
            if(!bDLSEnabled)
            {
                bDLSEnabled = TRUE;
            }
        }
        else if(dwResult == WAIT_OBJECT_0 + 1)
        {
            if(bDLSEnabled)
            {
                bDLSEnabled = FALSE;
            }
        }
    }
}


//------------------------------------------------------------------------------
//
// Function: Display_XECInit
//
// This function loads the DLL and gets the XEC-related functions
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL Display_XECInit(void)
{    
    MSGQUEUEOPTIONS queueOptions;

    // Create the UI Event used in IPUv3
    hUIActive = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(hUIActive == NULL)
    {
       RETAILMSG(1, (TEXT("Create hUIActive Event Fail\r\n"))); 
       return FALSE;
    }

    // Open DLS UIActive Event used in XEC
    hDLSUIActive = OpenEvent(EVENT_ALL_ACCESS, FALSE, DLS_UI_ACTIVE);
    if(hDLSUIActive == NULL)
    {
        RETAILMSG(1, (TEXT("Open hDLSUIActive Event Fail\r\n")));
        return FALSE;
    }
    
    // Open DLS VideoOff Event used in XEC
    hDLSVideoLayerOff = OpenEvent(EVENT_ALL_ACCESS, FALSE, DLS_VIDEO_OFF);
    if(hDLSVideoLayerOff == NULL)
    {
        RETAILMSG(1, (TEXT("Open hDLSVideoLayerOff Event Fail\r\n")));
        return FALSE;
    }
    
    // MSG
    // Create queues for reading and writing messages
    queueOptions.dwSize        = sizeof(MSGQUEUEOPTIONS);
    queueOptions.dwFlags       = MSGQUEUE_ALLOW_BROKEN;
    queueOptions.dwMaxMessages = 10;
    queueOptions.cbMaxMessage  = sizeof(NewFrame_msg);
    queueOptions.bReadAccess   = TRUE;  // we need read-access to msgqueue

    // Create read handles for queues
    hReadFliptoDLSQueue = CreateMsgQueue(DLS_FLIP, &queueOptions);
    if (hReadFliptoDLSQueue == NULL)
    {
        RETAILMSG(1, (TEXT("Error creating hReadFliptoDLSQueue\r\n")));
        return FALSE;
    }

    queueOptions.bReadAccess = FALSE;   // we need write-access to msgqueue

    // Create write handles for queues
    hWriteFliptoDLSQueue = OpenMsgQueue(GetCurrentProcess(),
                                        hReadFliptoDLSQueue,
                                        &queueOptions);
    if (hWriteFliptoDLSQueue == NULL)
    {
        RETAILMSG(1, (TEXT("Error creating hWriteFliptoDLSQueue\r\n")));
        return FALSE;
    }

    HANDLE hUIState = CreateThread(NULL, 0,
                                    (LPTHREAD_START_ROUTINE)DLSUIStateThread,
                                    NULL, 0, NULL);
    if(hUIState == NULL)
    {
        RETAILMSG(1, (TEXT("Create DLSUIStateThread Fail\r\n")));
        return FALSE;
    } 
    
    HANDLE hState = CreateThread(NULL, 0,
                                (LPTHREAD_START_ROUTINE)DLSStateThread,
                                NULL, 0, NULL);
    if(hState == NULL)
    {
        RETAILMSG(1, (TEXT("Create DLSStateThread Fail\r\n")));
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: Display_SendVideoFrame
//
// This function sets the UI event.
//
// Parameters:
//      pSurf
//          [DDGPESurf *] class DDGPESurf
//      SrcRect
//          [RECT] Rect struct
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_SendVideoFrame(DDGPESurf * pSurf, RECT SrcRect)
{
    NewFrame_msg newFrameMsg;
    static DWORD dwFrameNumber = 0;
    
    if(bDLSEnabled && (!bUIActive))
    {
        if(((++dwFrameNumber) % (DLS_FRAME_STEP)) == 0)
        {
            newFrameMsg.iSrcSurfaceHeight = pSurf->Height();
            newFrameMsg.iSrcSurfaceWidth = pSurf->Width();
            newFrameMsg.srcRect = SrcRect;
            newFrameMsg.pSourceFrameBuffer = (UINT8 *)pSurf->Buffer();

            if(!WriteMsgQueue(hWriteFliptoDLSQueue, &newFrameMsg, sizeof(newFrameMsg), 0, 0))
            {
                RETAILMSG(1, (TEXT("Could not write to Flip-to-DLS queue\r\n")));
            }
        }
    }
    else
    {
        dwFrameNumber = 0;
    }
}


//------------------------------------------------------------------------------
//
// Function: Display_SetCSC
//
// This function sets DP CSC.
//
// Parameters:
//      cjIn
//          [ULONG] the length of the data
//
//      pvIn
//          [PVOID] pointer to the data
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL Display_SetCSC(ULONG cjIn, PVOID pvIn)
{
    BOOL rc = FALSE;
    pCSCParameters pCscParameters;
    
    if(pvIn && cjIn == sizeof(CSCParameters))
    {
        pCscParameters = (pCSCParameters)pvIn;
        
        DpCSCConfigData.CSCCoeffs.A[0] = pCscParameters->A[0];
        DpCSCConfigData.CSCCoeffs.A[3] = pCscParameters->A[1];
        DpCSCConfigData.CSCCoeffs.A[6] = pCscParameters->A[2];

        DpCSCConfigData.CSCCoeffs.B[0] = pCscParameters->B[0];
        DpCSCConfigData.CSCCoeffs.B[1] = pCscParameters->B[1];
        DpCSCConfigData.CSCCoeffs.B[2] = pCscParameters->B[2];

        DpCSCConfigData.CSCEquation = CSCCustom;
        
        DPCSCConfigure(DpChannel, (pDpCSCConfigData)&DpCSCConfigData);

        rc = TRUE;
    }

    return rc;
}


//------------------------------------------------------------------------------
//
// Function: Display_GetCSC
//
// This function sets DP CSC.
//
// Parameters:
//      cjOut
//          [ULONG] the length of the data
//
//      pvOut
//          [PVOID] pointer to the data
//
// Returns:
//      TRUE if success.
//      FALSE if failure.

//
//------------------------------------------------------------------------------
BOOL Display_GetCSC(ULONG cjOut, PVOID pvOut)
{
    BOOL rc = FALSE;
    pCSCParameters pCscParameters;
    
    // Add source codes here to get the DpChannel
    PANEL_INFO * pPanelInfo;
    if(g_pDI1PanelInfo->ISACTIVE)
    {
        pPanelInfo=g_pDI1PanelInfo; 
    }
    else
    {
        pPanelInfo=g_pDI0PanelInfo; 
    }

    if(pPanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
    {
        DpChannel = DP_CHANNEL_ASYNC0;
        RETAILMSG(1, (TEXT("DP_CHANNEL_ASYNC0\r\n")));
    }
    else
    {
        DpChannel = DP_CHANNEL_SYNC;
        RETAILMSG(1, (TEXT("DP_CHANNEL_SYNC\r\n")));
    }

    if(pvOut && cjOut == sizeof(CSCParameters))
    {
        pCscParameters = (pCSCParameters)pvOut;
        DPCSCGetCoeffs(DpChannel, (pDpCSCConfigData)&DpCSCConfigData);

        pCscParameters->A[0] = DpCSCConfigData.CSCCoeffs.A[0];
        pCscParameters->A[1] = DpCSCConfigData.CSCCoeffs.A[3];
        pCscParameters->A[2] = DpCSCConfigData.CSCCoeffs.A[6];

        pCscParameters->B[0] = DpCSCConfigData.CSCCoeffs.B[0];
        pCscParameters->B[1] = DpCSCConfigData.CSCCoeffs.B[1];
        pCscParameters->B[2] = DpCSCConfigData.CSCCoeffs.B[2];

        rc = TRUE;
    }

    return rc;
}

