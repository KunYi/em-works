//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  display_middle_overlay.cpp
//
//  The functions in this file support the multiple overlay use case.  When
//  displaying more than one overlay, the IPU IC block must be used to 
//  combine any overlays beyond the first one.  This function handles
//  combining of all non-top overlays which need the IC.  The top overlay 
//  is handled by functions in display_top_overlay.cpp
//  
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <types.h>
#include <winddi.h>
#include <ceddk.h>
#include <cmnintrin.h>
#include <NKIntr.h>
#include <winddi.h>
#include <ddgpe.h>
#pragma warning(pop)

#include "common_macros.h"

#include "idmac.h"
#include "cpmem.h"
#include "tpm.h"
#include "lut.h"
#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"
#include "display.h"
#include "dp.h"
#include "pp.h"
#include "cm.h"

#include "dmfc.h"

//-----------------------------------------------------------------------------
// External Functions
extern DWORD BSPGetPixelDepthFromRegistry(VOID);
extern BOOL BSPIsLCDMode(DWORD);
extern "C" void PrPStart(HANDLE hDeviceContext);
extern "C" BOOL PrPAddVFOutputBuffer(HANDLE hDeviceContext, UINT32 PhysicalBuf);
extern "C" BOOL PrPAddInputCombBuffer(HANDLE hDeviceContext, UINT32 PhysicalBuf);


//-----------------------------------------------------------------------------
// External Variables
extern DWORD g_dwCurBGChan;
extern BOOL g_bDoubleBufferMode;

// Synchronization when buffer update.
extern CRITICAL_SECTION g_csBufferLock;
extern HANDLE g_hIPUBase;
extern UINT32 g_PrimaryBufPhysAddr;
extern UINT32 g_Primary2BufPhysAddr;
extern GPEMode g_PrimaryMode;
extern GPEMode g_SecondaryMode;
extern DWORD g_iCurrentMode;
extern HANDLE g_hPrp;                // handle to Pre-processor driver class
extern CRITICAL_SECTION g_csPrPLock;    // Critical section for PrP


extern pOverlaySurf_t  *g_ppOverlays;
extern UINT32           g_iNumOverlays;
extern UINT32           g_nOverlayBufferOffset[MAX_VISIBLE_OVERLAYS];
//-----------------------------------------------------------------------------
// Defines

#define GET_PERF_TIME  0

#define PPWIDTH(a)     (a - 1)

#define GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH 16
#define GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH 8
#define GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH 0
#define MAX_EDDGPE_FORMATS          14

#define LOCK_PRP()      EnterCriticalSection(&g_csPrPLock)
#define UNLOCK_PRP()    LeaveCriticalSection(&g_csPrPLock)

//-----------------------------------------------------------------------------
// Types

typedef enum IC_MODEEnum
{
    IC_MODE_DESTKEY,           // Overlay is video input
    IC_MODE_SRCKEY_ONLY,       // Overlay is graphics input, no processing needed
    IC_MODE_SRCKEY_AND_PROCESSING, // 2 passes needed, one for processing, one for combining
} IC_MODE;


//-----------------------------------------------------------------------------
// Global Variables
static BOOL      g_bICUpdating,g_bICStep1Skip;
static IC_MODE   g_ICMode[MAX_VISIBLE_OVERLAYS];
UINT32    g_nOverlayDstBufferOffset[MAX_VISIBLE_OVERLAYS];
static ppConfigData g_ppData4ICCopy;
static ppConfigData g_ppData4ICCombine[MAX_VISIBLE_OVERLAYS];
static UINT32 CurrentOverlayBuf[MAX_VISIBLE_OVERLAYS];
static HANDLE g_hMiddleOverlayThread;
static HANDLE g_hMiddleOverlayUpdateEvent,g_hMiddleOverlayDoneEvent,g_hMiddleOverlayStopEvent;
HANDLE g_hMiddleOverlayUIUpdateEvent;

UINT32 g_nOverlayBuffer2Offset;


UINT32 time = 0;

//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions
void Display_SetupStep1_4Overlay(BOOL b4SecDisp);
void Display_SetupStep2_4Overlay(pOverlaySurf_t pOverlaySurfInfo, UINT32 order,BOOL b4SecDisp);
void Display_Middle_OverlayMainThread(LPVOID lpParameter);


//------------------------------------------------------------------------------
//
// Function: Display_Middle_OverlayInit
//
// This function initializes global variables for overlay support.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void Display_Middle_OverlayInit()
{
    g_bICUpdating = FALSE;
    g_bICStep1Skip = FALSE;
    g_hMiddleOverlayThread = NULL;
    g_hMiddleOverlayUIUpdateEvent = NULL;
    g_hMiddleOverlayUpdateEvent = NULL;
    g_hMiddleOverlayStopEvent = NULL;
    g_hMiddleOverlayDoneEvent = NULL;    
}

//------------------------------------------------------------------------------
//
// Function: Display_Middle_OverlayDeInit
//
// This function clean up global variables for overlay support.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void Display_Middle_OverlayDeInit()
{
}

//------------------------------------------------------------------------------
//
// Function: Display_Middle_SetOverlayBuffer
//
// A wrapper of PrPAddInputBuffer
//
// Parameters:
//      iBuf
//          [in] The base physical address of overlay surface, only for second panel overlay.
//                It should be NULL, when the overlay is on primary panel.
//
//      overlayIndex
//          [in] index into overlay array to identify overlay buffer. For second panel,
//                it only support one overlay, so the index should always be 0.
//
//      bWait
//          [in]   TRUE:  The function won't return until the first module finished processing.
//                  FALSE:The function returns immediately once the buffer is filled.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL Display_Middle_SetOverlayBuffer(UINT32 iBuf, UINT32 overlayIndex, BOOL bWait)
{
    UINT32 PhysicalBuf;

    if(iBuf)
        PhysicalBuf = iBuf + g_nOverlayBuffer2Offset;
    else
        PhysicalBuf = g_ppOverlays[overlayIndex]->nBufPhysicalAddr + g_nOverlayBufferOffset[overlayIndex];

    EnterCriticalSection(&g_csBufferLock);
    while(g_bICUpdating)
    {
        Sleep(1);
    }
    CurrentOverlayBuf[overlayIndex] = PhysicalBuf;
    if(g_hMiddleOverlayUpdateEvent)
    {
        ResetEvent(g_hMiddleOverlayDoneEvent);
        SetEvent(g_hMiddleOverlayUpdateEvent);
        RETAILMSG(GET_PERF_TIME, (TEXT("SetOverlayBuffer Set g_hMiddleOverlayUpdateEvent - %d \r\n"), GetTickCount()));
        if(bWait)
        {
            // TODO: The timeout value need to be justified accoridng to use case.
            if(WaitForSingleObject(g_hMiddleOverlayDoneEvent,100)==WAIT_TIMEOUT)
            {
                DEBUGMSG(1,(TEXT("Overlay 2 processing timeout.\r\n")));
            }
        }
    }
    LeaveCriticalSection(&g_csBufferLock);

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: Display_Middle_StopOverlay
//
// This function stops hardware operation.
//
// Parameters:
//          None.
//
// Returns:
//          None.
//
//------------------------------------------------------------------------------
void Display_Middle_StopOverlay()
{
    ResetEvent(g_hMiddleOverlayUIUpdateEvent);
    ResetEvent(g_hMiddleOverlayUpdateEvent);
    while(g_bICUpdating)
    {
        Sleep(1);
    }
}

//------------------------------------------------------------------------------
//
// Function: Display_Middle_DisableOverlay
//
// This function stops hardware operation, meanwhile clean up buffers.
//
// Parameters:
//          None.
//
// Returns:
//          True if succeed.
//
//------------------------------------------------------------------------------
BOOL Display_Middle_DisableOverlay()
{
    DWORD dwExitCode;
    if(g_hMiddleOverlayUIUpdateEvent)
    {
        ResetEvent(g_hMiddleOverlayUIUpdateEvent);
        ResetEvent(g_hMiddleOverlayUpdateEvent);
        SetEvent(g_hMiddleOverlayStopEvent);
        while(g_bICUpdating)
        {
            Sleep(1);
        }
        while (GetExitCodeThread(g_hMiddleOverlayThread, &dwExitCode))
        {
            if (dwExitCode != STILL_ACTIVE)
                break;
            Sleep(1);
        }
        CloseHandle(g_hMiddleOverlayThread);
        g_hMiddleOverlayThread = NULL;

        CloseHandle(g_hMiddleOverlayUIUpdateEvent);
        g_hMiddleOverlayUIUpdateEvent = NULL;
        CloseHandle(g_hMiddleOverlayUpdateEvent);
        g_hMiddleOverlayUpdateEvent = NULL;
        CloseHandle(g_hMiddleOverlayStopEvent);
        g_hMiddleOverlayStopEvent = NULL;
        CloseHandle(g_hMiddleOverlayDoneEvent);
        g_hMiddleOverlayDoneEvent = NULL;
    }
    return TRUE;

}


//------------------------------------------------------------------------------
//
// Function: Display_Middle_SetupOverlay
//
// This function translates ddraw parameter to hardware specific parameter and setup hardware.
//
// Parameters:
//      pOverlaySurfaceInfo
//          [in] structure for overlay surface parameter,only for second panel overlay.
//                It should be NULL, when the overlay is on primary panel.
//
// Returns:
//          True if succeed.
//
//------------------------------------------------------------------------------
void Display_Middle_SetupOverlay(pOverlaySurf_t pOverlaySurfaceInfo)
{
    GPEMode *pCurMode;
    if(pOverlaySurfaceInfo == NULL)
    {
        if (g_iCurrentMode == (DWORD)g_PrimaryMode.modeId)
        {
            // Basic LCD mode - Primary mode in use
            pCurMode = &g_PrimaryMode;
        }
        else
        {
            // TV Mode active - Secondary mode in use
            pCurMode = &g_SecondaryMode;
        }

        while(g_bICUpdating)
        {
            Sleep(1);
        }
        //If overlay surface is full screen and no alpha and colorkey we needn't
        //copy the primary surface, 
        //the UI combine data will from primary surface buffer 
        if((pCurMode->width != g_ppOverlays[0]->DstWidth)||
           (pCurMode->height!= g_ppOverlays[0]->DstHeight)||
            (g_ppOverlays[0]->Transparency!= 0xff)||
            (g_ppOverlays[0]->ColorKeyMask!= 0xFFFFFFFF)||
            (g_ppOverlays[0]->bGlobalAlpha != TRUE))

        {
            // configure pp for step 1
            Display_SetupStep1_4Overlay(FALSE);
            g_bICStep1Skip = FALSE;
        }
        else
        {
            g_bICStep1Skip = TRUE;
        }

        for (int i = 0; i < (int)(g_iNumOverlays-1); i++)
        {
            // Configure pp for step 2
            Display_SetupStep2_4Overlay(g_ppOverlays[i], i, FALSE);
        }
    }
    else
    {
        //Overlay support for secondary display device
        pCurMode = &g_PrimaryMode;

        g_iNumOverlays = 1;
        
        while(g_bICUpdating)
        {
            Sleep(1);
        }
        //If overlay surface is full screen and no alpha and colorkey we needn't
        //copy the primary surface, 
        //the UI combine data will from primary surface buffer 
        if((pCurMode->width != pOverlaySurfaceInfo->DstWidth)||
           (pCurMode->height!= pOverlaySurfaceInfo->DstHeight)|| 
            (pOverlaySurfaceInfo->Transparency!= 0xff)||
            (pOverlaySurfaceInfo->ColorKeyMask!= 0xFFFFFFFF)||
            (pOverlaySurfaceInfo->bGlobalAlpha != TRUE))

        {
            // configure pp for step 1
            Display_SetupStep1_4Overlay(TRUE);
            g_bICStep1Skip = FALSE;
        }
        else
        {
            g_bICStep1Skip = TRUE;
        }

        // Configure pp for step 2
        Display_SetupStep2_4Overlay(pOverlaySurfaceInfo, 0, TRUE);
    }
}

//------------------------------------------------------------------------------
//
// Function: Display_Start2Overlay
//
// This function starts hardware operation for special 2 overlay use case.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_Start2Overlay()
{
    //=============Start the middle overlay================//
    PrPStart(g_hPrp);
    if(!g_hMiddleOverlayThread)
    {
        g_hMiddleOverlayUIUpdateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(g_hMiddleOverlayUIUpdateEvent == NULL)
        {
            ERRORMSG(1, (TEXT("Create UI update event for overlay 2 failed!\r\n")));
        }
        g_hMiddleOverlayUpdateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(g_hMiddleOverlayUpdateEvent == NULL)
        {
            ERRORMSG(1, (TEXT("Create overlay update event for overlay 2 failed!\r\n")));
        }
        g_hMiddleOverlayStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(g_hMiddleOverlayStopEvent == NULL)
        {
            ERRORMSG(1, (TEXT("Create stop event for overlay 2 thread failed!\r\n")));
        }
        g_hMiddleOverlayDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(g_hMiddleOverlayDoneEvent == NULL)
        {
            ERRORMSG(1, (TEXT("Create overlay 2 operation done event failed!\r\n")));
        }

        PREFAST_SUPPRESS(5451, "affects 64 bit windows only");
        g_hMiddleOverlayThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Display_Middle_OverlayMainThread, 
                                    (LPVOID)0, 0, NULL);

        if (g_hMiddleOverlayThread == NULL)
        {
            ERRORMSG(1,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        }
        else
        {
            DEBUGMSG(1, (TEXT("%s: create second Overlay processing thread success\r\n"), __WFUNCTION__));
        }
    }
    ResetEvent(g_hMiddleOverlayDoneEvent);
    RETAILMSG(GET_PERF_TIME, (TEXT("StartOverlay Set g_hMiddleOverlayUpdateEvent - %d \r\n"), GetTickCount()));
    SetEvent(g_hMiddleOverlayUpdateEvent);

    if(WaitForSingleObject(g_hMiddleOverlayDoneEvent,30)==WAIT_TIMEOUT)
    {
        DEBUGMSG(1,(TEXT("Overlay 2 processing timeout.\r\n")));
    }
  

    //=============Start the top overlay================// 

    CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 0, (UINT32 *)(g_ppOverlays[1]->nBufPhysicalAddr + g_nOverlayBufferOffset[1]));
    CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 1, (UINT32 *)(g_ppOverlays[1]->nBufPhysicalAddr + g_nOverlayBufferOffset[1]));
    
    IDMACChannelDBMODE(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE,TRUE);

    IDMACChannelEnable(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);

    DPGraphicWindowEnable(DP_CHANNEL_SYNC,TRUE);
    
}

//------------------------------------------------------------------------------
//
// Function: Display_Setup2Overlay
//
// This function translates ddraw parameter to hardware specific parameter and 
// setup hardware.for special 2 overlay use case.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_Setup2Overlay()
{

    GPEMode *pCurMode;
    //Overlay support for secondary display device
    pCurMode = &g_PrimaryMode;
    dmfcConfigData dmfcData;
    CPMEMConfigData CPMEMData;
    CPMEMBufOffsets OffsetData;

    while(g_bICUpdating)
    {
        Sleep(1);
    }

    // Ensure IPU clocks enabled before writing to IPU registers
    IPUV3EnableClocks(g_hIPUBase);

    //=============Setup the middle overlay================//
    
    //If overlay surface is full screen and no alpha and colorkey we needn't
    //copy the primary surface, 
    //the UI combine data will from primary surface buffer 
    if((pCurMode->width != g_ppOverlays[0]->DstWidth)||
       (pCurMode->height!= g_ppOverlays[0]->DstHeight)||
        (g_ppOverlays[0]->Transparency!= 0xff)||
        (g_ppOverlays[0]->ColorKeyMask!= 0xFFFFFFFF)||
        (g_ppOverlays[0]->bGlobalAlpha != TRUE))
    
    {
        // configure pp for step 1
        Display_SetupStep1_4Overlay(FALSE);
        g_bICStep1Skip = FALSE;
    }
    else
    {
        g_bICStep1Skip = TRUE;
    }

    // Configure PrP task
    Display_SetupPrP4Overlay(0);


    //=============Setup the top overlay================//  
    //This surface only supports RGB type
   
    g_nOverlayBufferOffset[1] = g_ppOverlays[1]->SrcRect.top * g_ppOverlays[1]->SrcLineStride 
                                + g_ppOverlays[1]->SrcRect.left * g_ppOverlays[1]->SrcBpp / 8;
    // configure DP  task
    Display_SetupDP4Overlay(g_ppOverlays[1]);

    //Config DMFC    
    dmfcData.FrameWidth = g_ppOverlays[1]->SrcWidth;
    if(dmfcData.FrameWidth > 1024)
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_256x128;
    else if(dmfcData.FrameWidth*2 >= 512)
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_128x128;
    else if(dmfcData.FrameWidth*2 >= 256)
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_64x128;
    else if(dmfcData.FrameWidth*2 >= 128)
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_32x128;
    else if(dmfcData.FrameWidth*2 >= 64)
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_16x128;
    else if(dmfcData.FrameWidth*2 >= 32)
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_8x128;
    else 
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_4x128;

    dmfcData.FrameHeight = g_ppOverlays[1]->SrcHeight;
    dmfcData.BurstSize = IPU_DMFC_BURST_SIZE_0_4_WORDS;
    dmfcData.WaterMarkEnable = IPU_DMFC_WM_EN_DISABLE;
    
    // Don't care DMFC values
    dmfcData.WaterMarkClear = 1; // Make the value the same as default value.
    dmfcData.WaterMarkSet = 0; // Don't care if Watermark disabled
    dmfcData.DestChannel = 0; // Don't care unless using IC->DMFC path
    dmfcData.PixelPerWord = IPU_DMFC_PPW_C_8BPP; // Only care for DC Read Ch or IC->DMFC path
   
    dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_4;
    DMFCConfigure(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, &dmfcData);

    // Configure CPMEM
    //********************************
    // Special Mode parameters
    //********************************
    memset(&CPMEMData,0,sizeof(CPMEMData));
    CPMEMData.iAccessDimension = CPMEM_DIM_2D;
    CPMEMData.iScanOrder = CPMEM_SO_PROGRESSIVE;
    CPMEMData.iBandMode = CPMEM_BNDM_DISABLE;
    CPMEMData.iBlockMode = CPMEM_BM_DISABLE;
    CPMEMData.iThresholdEnable = CPMEM_THE_DISABLE;
    CPMEMData.iCondAccessEnable = CPMEM_CAE_COND_SKIP_DISABLE;
    CPMEMData.iAXI_Id = CPMEM_ID_0; //ID_0 has high priority to access memory, only for sync display channel

    //********************************
    // Format and dimensions
    //********************************
    CPMEMData.iPixelFormat = CPMEM_PFS_INTERLEAVED_RGB;
    CPMEMData.iHeight = (UINT16)g_ppOverlays[1]->SrcHeight - 1;
    CPMEMData.iWidth = (UINT16)g_ppOverlays[1]->SrcWidth - 1;

    CPMEMData.iLineStride = g_ppOverlays[1]->SrcLineStride - 1;
    OffsetData.interlaceOffset = 0;
    CPMEMWriteOffset(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, &OffsetData, TRUE);  // TRUE: interleaved RGB    

    switch (g_ppOverlays[1]->SrcBpp)
    {
        case 16:
            CPMEMData.iBitsPerPixel = CPMEM_BPP_16;
            CPMEMData.iPixelBurst = CPMEM_PIXEL_BURST_16;
            CPMEMData.pixelFormatData.component0_width = 5-1;
            CPMEMData.pixelFormatData.component1_width = 6-1;
            CPMEMData.pixelFormatData.component2_width = 5-1;
            CPMEMData.pixelFormatData.component3_width = 0;
            CPMEMData.pixelFormatData.component0_offset = 0;
            CPMEMData.pixelFormatData.component1_offset = 5;
            CPMEMData.pixelFormatData.component2_offset = 11;
            CPMEMData.pixelFormatData.component3_offset = 0;
            break;
        case 24:
            CPMEMData.iBitsPerPixel = CPMEM_BPP_24;
            CPMEMData.iPixelBurst = CPMEM_PIXEL_BURST_16;
            CPMEMData.pixelFormatData.component0_width = 8-1;
            CPMEMData.pixelFormatData.component1_width = 8-1;
            CPMEMData.pixelFormatData.component2_width = 8-1;
            CPMEMData.pixelFormatData.component3_width = 0;
            CPMEMData.pixelFormatData.component0_offset = 0;
            CPMEMData.pixelFormatData.component1_offset = 8;
            CPMEMData.pixelFormatData.component2_offset = 16;
            CPMEMData.pixelFormatData.component3_offset = 0;
            break;
        case 32:
            CPMEMData.iBitsPerPixel = CPMEM_BPP_32;
            CPMEMData.iPixelBurst = CPMEM_PIXEL_BURST_16;
            CPMEMData.pixelFormatData.component0_width = 8-1;
            CPMEMData.pixelFormatData.component1_width = 8-1;
            CPMEMData.pixelFormatData.component2_width = 8-1;
            CPMEMData.pixelFormatData.component3_width = 8-1;
            CPMEMData.pixelFormatData.component0_offset = 8;
            CPMEMData.pixelFormatData.component1_offset = 16;
            CPMEMData.pixelFormatData.component2_offset = 24;
            CPMEMData.pixelFormatData.component3_offset = 0;
            break;
        default:
            // error
            break;
    }
  
    // We now write CPMEM data into the CPMEM
    CPMEMWrite(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, &CPMEMData, TRUE);


    // Clear the bit to disable DMFC direct path.
    // CMSetPathIC2DP(FALSE, FALSE);

    //CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC1_SRC,
    //                        IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_MCU);
    
    // We can make this call to disable IPU HSP clock.  It won't
    // have any effect if other IPU submodules remain enabled.
    IPUV3DisableClocks(g_hIPUBase);
}


//------------------------------------------------------------------------------
//
// Function: Display_Middle_StartOverlay
//
// This function starts hardware operation.
//
// Parameters:
//      iBuf
//          [in] The base physical address of overlay surface, only for second panel overlay.
//                It should be NULL, when the overlay is on primary panel.
//
//      b4SecDisp
//          [in] The flag to figure out if for second panel overlay
//                TURE: For second panel overlay
//                FALSE: For pirmary panel overlay(multi-overlay)
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_Middle_StartOverlay(UINT32 iBuf,BOOL b4SecDisp)
{
    if(!g_hMiddleOverlayThread)
    {
        g_hMiddleOverlayUIUpdateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(g_hMiddleOverlayUIUpdateEvent == NULL)
        {
            ERRORMSG(1, (TEXT("Create UI update event for overlay 2 failed!\r\n")));
        }
        g_hMiddleOverlayUpdateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(g_hMiddleOverlayUpdateEvent == NULL)
        {
            ERRORMSG(1, (TEXT("Create overlay update event for overlay 2 failed!\r\n")));
        }
        g_hMiddleOverlayStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(g_hMiddleOverlayStopEvent == NULL)
        {
            ERRORMSG(1, (TEXT("Create stop event for overlay 2 thread failed!\r\n")));
        }
        g_hMiddleOverlayDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(g_hMiddleOverlayDoneEvent == NULL)
        {
            ERRORMSG(1, (TEXT("Create overlay 2 operation done event failed!\r\n")));
        }

        PREFAST_SUPPRESS(5451, "affects 64 bit windows only");
        g_hMiddleOverlayThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Display_Middle_OverlayMainThread, 
                                    (LPVOID)b4SecDisp, 0, NULL);

        if (g_hMiddleOverlayThread == NULL)
        {
            ERRORMSG(1,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        }
        else
        {
            DEBUGMSG(1, (TEXT("%s: create second Overlay processing thread success\r\n"), __WFUNCTION__));
        }
    }
    if(b4SecDisp)
        CurrentOverlayBuf[0] = iBuf  + g_nOverlayBuffer2Offset;
    else
        CurrentOverlayBuf[0] = g_ppOverlays[0]->nBufPhysicalAddr + g_nOverlayBufferOffset[0];

    ResetEvent(g_hMiddleOverlayDoneEvent);
    RETAILMSG(GET_PERF_TIME, (TEXT("StartOverlay Set g_hMiddleOverlayUpdateEvent - %d \r\n"), GetTickCount()));
    SetEvent(g_hMiddleOverlayUpdateEvent);

    if(WaitForSingleObject(g_hMiddleOverlayDoneEvent,30)==WAIT_TIMEOUT)
    {
        DEBUGMSG(1,(TEXT("Overlay 2 processing timeout.\r\n")));
    }        
}

//------------------------------------------------------------------------------
//
// Function: Display_Middle_SetupOverlayWindowPos
//
// This function is for seting up overlay window position.
//
// Parameters:
//      overlayIndex
//          [in] Index into the overlay info array, specifying which overlay
//          should have into position updated.
//
//      x
//          [in] x position.
//
//      y
//          [in] y position.
//
//      b4SecDisp
//          [in] The flag to figure out if for second panel overlay
//                TRUE: For second panel overlay
//                FALSE: For primary panel overlay(multi-overlay)
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_Middle_SetupOverlayWindowPos(UINT32 overlayIndex, UINT16 x, UINT16 y,BOOL b4SecDisp)
{
    UINT32 iLineStride;
    GPEMode *pCurMode;

    if (BSPIsLCDMode(g_iCurrentMode)||b4SecDisp)
    {
        // Basic LCD mode - Primary mode in use
        pCurMode = &g_PrimaryMode;
    }
    else
    {
        // TV Mode active - Secondary mode in use
        pCurMode = &g_SecondaryMode;
    }


    switch(pCurMode->Bpp)
    {
        case 32:
            x=x&0xfff8;  // 8 alignment
            iLineStride = pCurMode->width*4;
            g_nOverlayDstBufferOffset[overlayIndex] = iLineStride * y + x*4;
            break;
        case 24:
            x=x&0xfff8;  // 8 alignment
            iLineStride = pCurMode->width*3;
            g_nOverlayDstBufferOffset[overlayIndex] = iLineStride * y + x*3;
            break;
        case 16:
        default:
            x=x&0xfffc; // 4 alignment
            iLineStride = pCurMode->width*2;
            g_nOverlayDstBufferOffset[overlayIndex] = iLineStride * y + x*2;
            break;
    }
    
    //SetEvent to update
    if(g_hMiddleOverlayUpdateEvent)
    {
        RETAILMSG(GET_PERF_TIME, (TEXT("SetWindowPos Set g_hMiddleOverlayUpdateEvent - %d \r\n"), GetTickCount()));
        SetEvent(g_hMiddleOverlayUpdateEvent);
    }
    return;
}

//------------------------------------------------------------------------------
//
// Function: Display_SetupStep1_4Overlay
//
// This function sets up the PP configuration data for the framebuffer
// copy step, which precedes the IC Combining step.  We simply want to
// make a direct copy of the framebuffer, with no PP processing operations
// taking place.
//
// Parameters:
//      b4SecDisp
//          [in] The flag to figure out if for second panel overlay
//                TURE: For second panel overlay
//                FALSE: For pirmary panel overlay(multi-overlay)
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_SetupStep1_4Overlay(BOOL b4SecDisp)
{
    GPEMode *pCurMode;

    if (BSPIsLCDMode(g_iCurrentMode)||b4SecDisp)
    {
        // Basic LCD mode - Primary mode in use
        pCurMode = &g_PrimaryMode;
    }
    else
    {
        // TV Mode active - Secondary mode in use
        pCurMode = &g_SecondaryMode;
    }

    // Clear post-processing configuration data
    memset(&g_ppData4ICCopy, 0 , sizeof(ppConfigData));

    // Grab input width/height from current mode information
    g_ppData4ICCopy.inputIDMAChannel.FrameSize.height = (UINT16)pCurMode->height;
    g_ppData4ICCopy.inputIDMAChannel.FrameSize.width = (UINT16)pCurMode->width;

    // Framebuffer format is always RGB
    g_ppData4ICCopy.inputIDMAChannel.FrameFormat = icFormat_RGB;

    // Determine input and output pixel formats from current mode bpp info
    switch(pCurMode->Bpp)
    {
        case 32:
            g_ppData4ICCopy.inputIDMAChannel.DataWidth = icDataWidth_32BPP;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component0_offset = 8;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component1_offset = 16;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component2_offset = 24;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component3_offset = 0;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component0_width = PPWIDTH(8);
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component1_width = PPWIDTH(8);
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component2_width = PPWIDTH(8);
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component3_width = PPWIDTH(8);    
            g_ppData4ICCopy.inputIDMAChannel.LineStride = pCurMode->width*4;
            break;
        case 24:
            g_ppData4ICCopy.inputIDMAChannel.DataWidth = icDataWidth_24BPP;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component0_offset = 0;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component1_offset = 8;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component2_offset = 16;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component3_offset = 24;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component0_width = PPWIDTH(8);
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component1_width = PPWIDTH(8);
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component2_width = PPWIDTH(8);
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component3_width = PPWIDTH(8);    
            g_ppData4ICCopy.inputIDMAChannel.LineStride = pCurMode->width*3;
            break;
        case 16:
        default:
            g_ppData4ICCopy.inputIDMAChannel.DataWidth = icDataWidth_16BPP;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component0_offset = 0;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component1_offset = 5;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component2_offset = 11;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component3_offset = 24;
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component0_width = PPWIDTH(5);
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component1_width = PPWIDTH(6);
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component2_width = PPWIDTH(5);
            g_ppData4ICCopy.inputIDMAChannel.PixelFormat.component3_width = 0;    
            g_ppData4ICCopy.inputIDMAChannel.LineStride = pCurMode->width*2;
            break;

    }

    // Input and output data should be identical, so copy from input to output
    memcpy(&g_ppData4ICCopy.outputIDMAChannel, &g_ppData4ICCopy.inputIDMAChannel,sizeof(idmaChannel));

    // Set up post-processor to perform no processing tasks
    g_ppData4ICCopy.CSCEquation = CSCNoOp;
    g_ppData4ICCopy.FlipRot.verticalFlip = FALSE;
    g_ppData4ICCopy.FlipRot.horizontalFlip = FALSE;
    g_ppData4ICCopy.FlipRot.rotate90 = FALSE;
    g_ppData4ICCopy.bCombineEnable = FALSE;
    g_ppData4ICCopy.alphaType = icAlphaType_Global;

    // Force PP to perform no tasks
    g_ppData4ICCopy.allowNopPP = TRUE;

    return;
}

//------------------------------------------------------------------------------
//
// Function: Display_SetupStep2_4Overlay
//
// This function sets up the PP configuration data for the framebuffer
// combining step. It performs rotation, color space conversion, 
// and resizing for the overlay surfaces and than combine it we background data.
//
// Parameters:
//      pOverlaySurfInfo
//          [in] sturcture for overlay surface.
//
//      order
//          [in] Overlay surface Z-order. For second panel overlay, as it only supports
//                one overlay, so the order should always be 0.
//
//      b4SecDisp
//          [in] The flag to figure out if for second panel overlay
//                TURE: For second panel overlay
//                FALSE: For pirmary panel overlay(multi-overlay)
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_SetupStep2_4Overlay(pOverlaySurf_t pOverlaySurfInfo, UINT32 order,BOOL b4SecDisp)
{
    EDDGPEPixelFormat inputPixFormat;
    UINT32 YBufLen;
    INT32 srcBufOffset;
    RECT * pSrcRect;
    UINT32 tempColorKey;
    UINT32 RGBColorKey;
    GPEMode *pCurMode;

    if (BSPIsLCDMode(g_iCurrentMode)||b4SecDisp)
    {
        // Basic LCD mode - Primary mode in use
        pCurMode = &g_PrimaryMode;
    }
    else
    {
        // TV Mode active - Secondary mode in use
        pCurMode = &g_SecondaryMode;
    }

    
    pSrcRect = &pOverlaySurfInfo->SrcRect;
    inputPixFormat = pOverlaySurfInfo->SrcPixelFormat;
    
    memset(&g_ppData4ICCombine[order],0, sizeof(ppConfigData));
    // Set up post-processing configuration data

    // Set up input format and data width

    if (inputPixFormat < MAX_EDDGPE_FORMATS)
    {
        g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat = EDDGPEFormatToIcFormat[inputPixFormat];
        g_ppData4ICCombine[order].inputIDMAChannel.DataWidth = EDDGPEFormatToIcDataWidth[inputPixFormat];
    }
    else if(inputPixFormat == ddgpePixelFormat_CustomFormat)
    {
        //for NV12
        g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat = EDDGPEFormatToIcFormat[19];
        g_ppData4ICCombine[order].inputIDMAChannel.DataWidth = EDDGPEFormatToIcDataWidth[19];
    }
    else
    {
        // Use RGB565 as a fallback default
        inputPixFormat = ddgpePixelFormat_565;
        g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat = icFormat_RGB;
        g_ppData4ICCombine[order].inputIDMAChannel.DataWidth = icDataWidth_16BPP;
    }
    
    switch(inputPixFormat)
    {
        case ddgpePixelFormat_YV12:
        case ddgpePixelFormat_NV12:
            break;

        case ddgpePixelFormat_YUYV:
        case ddgpePixelFormat_YUY2:
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_offset = 0;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_offset = 8;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_offset = 16;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_offset = 24;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_width = PPWIDTH(8);
            break;

        case ddgpePixelFormat_UYVY:
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_offset = 0;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_offset = 8;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_offset = 16;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_offset = 24;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_width = PPWIDTH(8);
            break;

        case ddgpePixelFormat_8888:
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_offset = 8;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_offset = 16;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_offset = 24;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_offset = 0;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_width = PPWIDTH(8);
            break;
            
        case ddgpePixelFormat_8880:
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_offset = 0;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_offset = 8;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_offset = 16;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_offset = 24;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_width = PPWIDTH(8);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_width = 0;
            break;
        case ddgpePixelFormat_565:
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_offset = 0;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_offset = 5;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_offset = 11;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_offset = 24; //no use, but must keep
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_width = PPWIDTH(5);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_width = PPWIDTH(6);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_width = PPWIDTH(5);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_width = 0;
            break;
        case ddgpePixelFormat_5550:
        case ddgpePixelFormat_5551:
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_offset = 0;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_offset = 5;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_offset = 10;
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_offset = 24; //no use
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component0_width = PPWIDTH(5);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component1_width = PPWIDTH(5);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component2_width = PPWIDTH(5);
            g_ppData4ICCombine[order].inputIDMAChannel.PixelFormat.component3_width = 0;
            break;

        default:
            break;
    }
    
    //Input parameters setting
    g_ppData4ICCombine[order].inputIDMAChannel.FrameSize.height = (UINT16) (pOverlaySurfInfo->SrcRect.bottom - pOverlaySurfInfo->SrcRect.top);
    g_ppData4ICCombine[order].inputIDMAChannel.FrameSize.width = (UINT16) (pOverlaySurfInfo->SrcRect.right - pOverlaySurfInfo->SrcRect.left);
    g_ppData4ICCombine[order].inputIDMAChannel.LineStride = pOverlaySurfInfo->SrcLineStride;
    if(b4SecDisp)
    {
        if (g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_YUV420)
        {
            // For YUV 4:2:0 case, compute U and V buffer offsets.
            YBufLen = pOverlaySurfInfo->SrcWidth * pOverlaySurfInfo->SrcHeight;
            //Compute Y buffer offset from source rectangle, it's the base offset.
            g_nOverlayBuffer2Offset = pSrcRect->top * pOverlaySurfInfo->SrcWidth + pSrcRect->left;
            //Compute u,v buffer offset from source rectangle, the base offset changed
            srcBufOffset = pSrcRect->top * pOverlaySurfInfo->SrcWidth /4 + (pSrcRect->left /2) - g_nOverlayBuffer2Offset;
            //V plane is in front of U plane.
            g_ppData4ICCombine[order].inputIDMAChannel.VBufOffset = YBufLen + srcBufOffset;
            g_ppData4ICCombine[order].inputIDMAChannel.UBufOffset = YBufLen + (YBufLen/4) + srcBufOffset;

        }
        else if(g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_YUV420P)
        {
            // For patrial interleaved YUV 4:2:0 case, compute U and V buffer offsets.
            YBufLen = pOverlaySurfInfo->SrcWidth * pOverlaySurfInfo->SrcHeight;
            //Compute Y buffer offset from source rectangle, it's the base offset.
            g_nOverlayBuffer2Offset = pSrcRect->top * pOverlaySurfInfo->SrcWidth + pSrcRect->left;
            //For this pixel format, hardware only care ubufoffset, the vbufferoffset will be ignored.
            g_ppData4ICCombine[order].inputIDMAChannel.VBufOffset = YBufLen;
            g_ppData4ICCombine[order].inputIDMAChannel.UBufOffset = g_ppData4ICCombine[order].inputIDMAChannel.VBufOffset;
        }
        else
        {
            // Compute buffer offset from source rectangle
            g_nOverlayBuffer2Offset = pSrcRect->top * pOverlaySurfInfo->SrcLineStride + pSrcRect->left * pOverlaySurfInfo->SrcBpp / 8;
        }        
    }
    else
    {
        if (g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_YUV420)
        {
            // For YUV 4:2:0 case, compute U and V buffer offsets.
            YBufLen = pOverlaySurfInfo->SrcWidth * pOverlaySurfInfo->SrcHeight;
            //Compute Y buffer offset from source rectangle, it's the base offset.
            g_nOverlayBufferOffset[order] = pSrcRect->top * pOverlaySurfInfo->SrcWidth + pSrcRect->left;
            //Compute u,v buffer offset from source rectangle, the base offset changed
            srcBufOffset = pSrcRect->top * pOverlaySurfInfo->SrcWidth /4 + (pSrcRect->left /2) - g_nOverlayBufferOffset[0];
            //V plane is in front of U plane.
            g_ppData4ICCombine[order].inputIDMAChannel.VBufOffset = YBufLen + srcBufOffset;
            g_ppData4ICCombine[order].inputIDMAChannel.UBufOffset = YBufLen + (YBufLen/4) + srcBufOffset;

        }
        else if(g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_YUV420P)
        {
            // For patrial interleaved YUV 4:2:0 case, compute U and V buffer offsets.
            YBufLen = pOverlaySurfInfo->SrcWidth * pOverlaySurfInfo->SrcHeight;
            //Compute Y buffer offset from source rectangle, it's the base offset.
            g_nOverlayBufferOffset[order] = pSrcRect->top * pOverlaySurfInfo->SrcWidth + pSrcRect->left;
            //For this pixel format, hardware only care ubufoffset, the vbufferoffset will be ignored.
            g_ppData4ICCombine[order].inputIDMAChannel.VBufOffset = YBufLen;
            g_ppData4ICCombine[order].inputIDMAChannel.UBufOffset = g_ppData4ICCombine[order].inputIDMAChannel.VBufOffset;
        }
        else
        {
            // Compute buffer offset from source rectangle
            g_nOverlayBufferOffset[order] = pSrcRect->top * pOverlaySurfInfo->SrcLineStride + pSrcRect->left * pOverlaySurfInfo->SrcBpp / 8;
        }    
    }


    //Setup output channel
    
    g_ppData4ICCombine[order].outputIDMAChannel.FrameFormat = icFormat_RGB;
    g_ppData4ICCombine[order].outputIDMAChannel.FrameSize.height = pOverlaySurfInfo->DstHeight;
    g_ppData4ICCombine[order].outputIDMAChannel.FrameSize.width = pOverlaySurfInfo->DstWidth;
    switch(pCurMode->Bpp)
    {
        case 32:
            g_ppData4ICCombine[order].outputIDMAChannel.DataWidth = icDataWidth_32BPP;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component0_offset = 8;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component1_offset = 16;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component2_offset = 24;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component3_offset = 0;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component0_width = PPWIDTH(8);
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component1_width = PPWIDTH(8);
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component2_width = PPWIDTH(8);
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component3_width = PPWIDTH(8);    
            g_ppData4ICCombine[order].outputIDMAChannel.LineStride = pCurMode->width * 4;
            pOverlaySurfInfo->XOffset=pOverlaySurfInfo->XOffset&0xfff8;  // 8 alignment
            g_nOverlayDstBufferOffset[order] = g_ppData4ICCombine[order].outputIDMAChannel.LineStride * pOverlaySurfInfo->YOffset 
                                        + pOverlaySurfInfo->XOffset*4;
            break;
        case 24:
            g_ppData4ICCombine[order].outputIDMAChannel.DataWidth = icDataWidth_24BPP;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component0_offset = 0;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component1_offset = 8;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component2_offset = 16;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component3_offset = 24;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component0_width = PPWIDTH(8);
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component1_width = PPWIDTH(8);
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component2_width = PPWIDTH(8);
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component3_width = PPWIDTH(8);    
            g_ppData4ICCombine[order].outputIDMAChannel.LineStride = pCurMode->width * 3;
            pOverlaySurfInfo->XOffset=pOverlaySurfInfo->XOffset&0xfff8;  // 8 alignment
            g_nOverlayDstBufferOffset[order] = g_ppData4ICCombine[order].outputIDMAChannel.LineStride * pOverlaySurfInfo->YOffset 
                                        + pOverlaySurfInfo->XOffset*3;
            break;
        case 16:
        default:
            g_ppData4ICCombine[order].outputIDMAChannel.DataWidth = icDataWidth_16BPP;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component0_offset = 0;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component1_offset = 5;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component2_offset = 11;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component3_offset = 24;
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component0_width = PPWIDTH(5);
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component1_width = PPWIDTH(6);
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component2_width = PPWIDTH(5);
            g_ppData4ICCombine[order].outputIDMAChannel.PixelFormat.component3_width = 0;    
            g_ppData4ICCombine[order].outputIDMAChannel.LineStride = pCurMode->width * 2;
            pOverlaySurfInfo->XOffset=pOverlaySurfInfo->XOffset&0xfffC;  // 4 alignment
            g_nOverlayDstBufferOffset[order] = g_ppData4ICCombine[order].outputIDMAChannel.LineStride * pOverlaySurfInfo->YOffset 
                                        + pOverlaySurfInfo->XOffset*2;
            break;
    }


    // Set up post-processing channel CSC parameters
    // based on input and output
    if ((g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_YUV444IL) ||
        (g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_YUV444) ||
        (g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_YUV422) ||
        (g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_YUV420) ||
        (g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_YUV420P) ||
        (g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_YUYV422) ||
        (g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_YVYU422) ||
        (g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_UYVY422) ||
        (g_ppData4ICCombine[order].inputIDMAChannel.FrameFormat == icFormat_VYUY422))
        g_ppData4ICCombine[order].CSCEquation = CSCY2R_A1;
    else
        g_ppData4ICCombine[order].CSCEquation = CSCNoOp;

    switch (pOverlaySurfInfo->iRotate)
    {
        case DMDO_0:
            g_ppData4ICCombine[order].FlipRot.verticalFlip = FALSE;
            g_ppData4ICCombine[order].FlipRot.horizontalFlip = FALSE;
            g_ppData4ICCombine[order].FlipRot.rotate90 = FALSE;
            break;
        case DMDO_90:
            g_ppData4ICCombine[order].FlipRot.verticalFlip = TRUE;
            g_ppData4ICCombine[order].FlipRot.horizontalFlip = TRUE;
            g_ppData4ICCombine[order].FlipRot.rotate90 = TRUE;
            break;
        case DMDO_180:
            g_ppData4ICCombine[order].FlipRot.verticalFlip = TRUE;
            g_ppData4ICCombine[order].FlipRot.horizontalFlip = TRUE;
            g_ppData4ICCombine[order].FlipRot.rotate90 = FALSE;
            break;
        case DMDO_270:
            g_ppData4ICCombine[order].FlipRot.verticalFlip = FALSE;
            g_ppData4ICCombine[order].FlipRot.horizontalFlip = FALSE;
            g_ppData4ICCombine[order].FlipRot.rotate90 = TRUE;
            break;
    }

    if(pOverlaySurfInfo->isUpsideDown)
        g_ppData4ICCombine[order].FlipRot.verticalFlip = g_ppData4ICCombine[order].FlipRot.verticalFlip ? FALSE :TRUE;   

    //Setup alpha blending
    g_ppData4ICCombine[order].inputcombAlpha = (UINT8)pOverlaySurfInfo->Transparency; 

    //Setup colorkey
    if(pOverlaySurfInfo->ColorKeyPlane == DisplayPlane_0)
    {
        tempColorKey = pOverlaySurfInfo->ColorKeyMask;
        switch(BSPGetPixelDepthFromRegistry())
        {
            case 18:
                // TODO: NEED CHECK
                RGBColorKey =
                    (((tempColorKey & 0x3F000) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH-12+2)) |
                    ((tempColorKey & 0x00FC0) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH-6+2)) |
                    ((tempColorKey & 0x0003F) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH+2)));
                break;  

            case 16:
                // Convert 5-bit and 6-bit channels to 8-bit by shifting left.
                // Then duplicate the high bits into the low bits after shifting
                // to ensure that 0x1F and 0x3F become 0xFF for maximum accuracy.
                RGBColorKey =
                    ((tempColorKey & 0x00F800) << 8) | ((tempColorKey & 0x00E000) << 3) |
                    ((tempColorKey & 0x0007E0) << 5) | ((tempColorKey & 0x000600) >> 1) |
                    ((tempColorKey & 0x00001F) << 3) | ((tempColorKey & 0x00001C) >> 2);
                break;        
            case 24:
                RGBColorKey = tempColorKey;
                break;
            case 32:
                RGBColorKey = tempColorKey &0x00ffffff;
                break;
            default:
                RGBColorKey = 0;
                break;

        }

        // If the color key is 0xFFFFFFFF, make sure to propagate this value
        // since it indicates that color key should be turned off.
        if (pOverlaySurfInfo->ColorKeyMask == 0xFFFFFFFF)
        {
            g_ppData4ICCombine[order].inputcombColorkey= 0xFFFFFFFF;
        }
        else
        {
            g_ppData4ICCombine[order].inputcombColorkey = RGBColorKey;
        }
    }
    else
    {
        g_ppData4ICCombine[order].inputcombColorkey= 0xFFFFFFFF;
        DEBUGMSG(1,(TEXT("Source colorkey is not supported for second overlay.\r\n")));
    }

    // Determine which IC mode we should use.
    if (pOverlaySurfInfo->ColorKeyPlane == DisplayPlane_0)
    {
        // Destination color key mode
        g_ICMode[order] = IC_MODE_DESTKEY;
    }
    else if (pOverlaySurfInfo->ColorKeyPlane == DisplayPlane_1)
    {
        // Is processing needed for the overlay?
        if ((g_ppData4ICCombine[order].CSCEquation == CSCNoOp)
            && (g_ppData4ICCombine[order].outputIDMAChannel.FrameSize.height == g_ppData4ICCombine[order].inputIDMAChannel.FrameSize.height)
            && (g_ppData4ICCombine[order].outputIDMAChannel.FrameSize.width == g_ppData4ICCombine[order].inputIDMAChannel.FrameSize.width))
        {
            // No processing needed for overlay
            g_ICMode[order] = IC_MODE_SRCKEY_ONLY;
        }
        else
        {
            // Processing needed for overlay
            g_ICMode[order] = IC_MODE_SRCKEY_AND_PROCESSING;
        }
    }

    // Enable alpha if:
    // a) global alpha enabled (not equal to 0xff)
    // OR
    // b) color keying enabled (not equal to 0xffffffff)
    // OR
    // c) local alpha enabled
    if((g_ppData4ICCombine[order].inputcombAlpha != 0xff)||
       (g_ppData4ICCombine[order].inputcombColorkey!= 0xFFFFFFFF)||
       !pOverlaySurfInfo->bGlobalAlpha)
    {
        g_ppData4ICCombine[order].bCombineEnable = TRUE;
        g_ppData4ICCombine[order].alphaType = pOverlaySurfInfo->bGlobalAlpha ? icAlphaType_Global : icAlphaType_Local;

        // Fill in Combining Channel DMA data
        // * If per-pixel alpha enabled, shift IC input channel over to IC combining channel, and set
        // input channel equal to output channel.
        if (g_ppData4ICCombine[order].alphaType == icAlphaType_Local)
        {
            memcpy(&g_ppData4ICCombine[order].inputcombIDMAChannel,&g_ppData4ICCombine[order].inputIDMAChannel,sizeof(idmaChannel));
            memcpy(&g_ppData4ICCombine[order].inputIDMAChannel,&g_ppData4ICCombine[order].outputIDMAChannel,sizeof(idmaChannel));
        }
        else
        {
            memcpy(&g_ppData4ICCombine[order].inputcombIDMAChannel,&g_ppData4ICCombine[order].outputIDMAChannel,sizeof(idmaChannel));
        }
    }

    g_ppData4ICCombine[order].allowNopPP = TRUE;
    return;
}


//------------------------------------------------------------------------------
//
// Function: Display_Middle_OverlayIsBusy
//
// This function returns the status of first module of foreground channel flow.
//
//
// Parameters:
//      None
//
// Returns:
//      True if busy, FALSE if idle.
//------------------------------------------------------------------------------
BOOL Display_Middle_OverlayIsBusy()
{
    // TODO:
    return FALSE;
}



//------------------------------------------------------------------------------
//
// Function: Display_Middle_OverlayMainThread
//
// This function is the main thread for second overlay surface processing.
//
// Parameters:
//      lpParameter
//          [in] The flag to figure out if for second panel overlay
//                TURE: For second panel overlay
//                FALSE: For pirmary panel overlay(multi-overlay)
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void Display_Middle_OverlayMainThread(LPVOID lpParameter)
{
    HANDLE hPP;
    HANDLE hMiddleOverlayEvents[3];
    IPUBufferInfo IPUBufInfo;
    IpuBuffer * pOutputBuf;
    UINT32 OutPhyAddr[2];
    UINT32 OutputIndex = 0;
    UINT32 PrimaryPhyAddr;
    DWORD  dwEventReturn;
    GPEMode *pCurMode;
    BOOL b4SecDisp = (BOOL)lpParameter;
    INT8 iCurBuf;

    CeSetThreadPriority(GetCurrentThread(), 105);
    
    if (BSPIsLCDMode(g_iCurrentMode)||b4SecDisp)
    {
        // Basic LCD mode - Primary mode in use
        pCurMode = &g_PrimaryMode;
    }
    else
    {
        // TV Mode active - Secondary mode in use
        pCurMode = &g_SecondaryMode;
    }

    if(b4SecDisp)
        PrimaryPhyAddr = g_Primary2BufPhysAddr;
    else
        PrimaryPhyAddr = g_PrimaryBufPhysAddr;

    switch(pCurMode->Bpp)
    {
        case 32:
            IPUBufInfo.dwBufferSize = pCurMode->width * pCurMode->height * 4 * 2;
            break;
        case 24:
            IPUBufInfo.dwBufferSize = pCurMode->width * pCurMode->height * 3 * 2;
            break;
        case 16:
        default:
            IPUBufInfo.dwBufferSize = pCurMode->width * pCurMode->height * 2 * 2;
            break;
    }

    //Allocate output buffer
    IPUBufInfo.MemType = MEM_TYPE_IRAM;
    pOutputBuf = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
    IPUV3AllocateBuffer(g_hIPUBase, &IPUBufInfo, pOutputBuf);
    //Physical address needn't CEOpenCallerBuffer().
    if(pOutputBuf == NULL)
    {
        ERRORMSG(1,
                (TEXT("%s: Fail to allocate Outputbuffer ! \r\n"), __WFUNCTION__));    
        return;              
    }
    OutPhyAddr[0] = (UINT32)pOutputBuf->PhysAddr();
    OutPhyAddr[1] = (UINT32)pOutputBuf->PhysAddr()+ IPUBufInfo.dwBufferSize /2;

    //Open pp handle
    hPP = PPOpenHandle();
    // Bail if no PP handle
    if (hPP == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: Opening PP handle failed!\r\n"), __WFUNCTION__));
        IPUV3DeallocateBuffer(g_hIPUBase, pOutputBuf);    
        return ;
    }    

    hMiddleOverlayEvents[0] = g_hMiddleOverlayUIUpdateEvent;
    hMiddleOverlayEvents[1] = g_hMiddleOverlayUpdateEvent;
    hMiddleOverlayEvents[2] = g_hMiddleOverlayStopEvent;
    //main loop
    for(;;)
    {
        dwEventReturn= WaitForMultipleObjects(3, hMiddleOverlayEvents, FALSE, INFINITE );
        RETAILMSG(GET_PERF_TIME, (TEXT("Received Event - %d \r\n"), GetTickCount()));
        
        if(dwEventReturn == WAIT_OBJECT_0+2 ) //stop event
        {
            // Break out of loop to code that actually shuts
            // down the 2nd overlay flow.
            break;
        }
        else if((dwEventReturn == WAIT_OBJECT_0+0) //ui update
                ||(dwEventReturn == WAIT_OBJECT_0+1)) //MiddleOverlay update
        {
            ResetEvent(g_hMiddleOverlayUIUpdateEvent);
            ResetEvent(g_hMiddleOverlayUpdateEvent);

            g_bICUpdating = TRUE;
            if(b4SecDisp)
                PrimaryPhyAddr = g_Primary2BufPhysAddr;
            else
                PrimaryPhyAddr = g_PrimaryBufPhysAddr;  
            if(!g_bICStep1Skip)
            {
                //Step 1 copy
                if (!PPConfigure(hPP, &g_ppData4ICCopy))
                {
                    break;  
                } 

                PPStart(hPP);
                PPInterruptEnable(hPP, FRAME_INTERRUPT);
                PPAddInputBuffer(hPP, PrimaryPhyAddr);
                PPAddOutputBuffer(hPP,OutPhyAddr[OutputIndex]);
                PPWaitForNotBusy(hPP, FRAME_INTERRUPT);
                PPStop(hPP);
            }

            RETAILMSG(GET_PERF_TIME, (TEXT("Done copy, on to combine - %d \r\n"), GetTickCount()));

            if(b4SecDisp)
            {
                //For second panel overlay, only one overlay surface is supported.
                if ((g_ICMode[0] == IC_MODE_SRCKEY_AND_PROCESSING)
                    || (g_ICMode[0] == IC_MODE_SRCKEY_ONLY))
                {
                    g_ICMode[0] = IC_MODE_DESTKEY;
                }
                if (!PPConfigure(hPP, &g_ppData4ICCombine[0]))
                {
                    break;  
                } 
                if(CurrentOverlayBuf[0] != NULL) 
                {
                    PPStart(hPP);
                    PPInterruptEnable(hPP, FRAME_INTERRUPT);
                    PPAddInputCombBuffer(hPP, PrimaryPhyAddr+g_nOverlayDstBufferOffset[0]);
                    PPAddInputBuffer(hPP, CurrentOverlayBuf[0]);
                    PPAddOutputBuffer(hPP,OutPhyAddr[OutputIndex]+g_nOverlayDstBufferOffset[0]);
                    PPWaitForNotBusy(hPP, FRAME_INTERRUPT);
                    PPStop(hPP); 
                }
            }
            else
            {
                if((g_iNumOverlays == 2)&&(g_ppOverlays[1]->SrcBpp == 32)&&(g_ppOverlays[0]->SrcBpp != 32))
                {
                    //In this case prp will be used in stead of pp.
                    LOCK_PRP();
                    if(PrPAddVFOutputBuffer(g_hPrp, OutPhyAddr[OutputIndex]+g_nOverlayDstBufferOffset[0])
                     &&PrPAddInputCombBuffer(g_hPrp, OutPhyAddr[OutputIndex]+g_nOverlayDstBufferOffset[0]))
                    {
                        Display_Top_SetOverlayBuffer(0, TRUE);
                    }
                    else
                    {
                        //prp is stopped, skip this time.
                    }
                    UNLOCK_PRP();
                }
                else
                {
                    // We must combine iteratively for all non-top overlays
                    for (int i = 0; i < (INT)g_iNumOverlays - 1; i++)
                    {
                        // There are 3 cases we handle here, based on whether source
                        // or destination color keying is requested.  This is because color 
                        // key can only be applied to the graphics input, and processing
                        // can only be performed on the video input:
                        //
                        // 1) If destination color key (or no color key), the overlay
                        //    is the video input and UI is the graphics input. (1 pass)
                        // 2) If source color key requested, and no processing
                        //    is required for the overlay, we can make the overlay
                        //    the graphics input and the UI the video input. (1 pass)
                        // 3) If source color key requested, and processing is required
                        //    for the overlay, we must make 2 passes through the IC.
                        //    First, make overlay the video input and do one pass to
                        //    perform all processing.  Then, do another pass with
                        //    overlay as graphics input and UI as video input.

                        // TODO: Support source color keying
                        // For now, we only support destination color key and only
                        // make one pass throug IC - convert all modes to dest color key
                        if ((g_ICMode[i] == IC_MODE_SRCKEY_AND_PROCESSING)
                            || (g_ICMode[i] == IC_MODE_SRCKEY_ONLY))
                        {
                            g_ICMode[i] = IC_MODE_DESTKEY;
                        }

                        if (!PPConfigure(hPP, &g_ppData4ICCombine[i]))
                        {
                            break;  
                        } 
                        if(CurrentOverlayBuf[i] == NULL) 
                            continue; //Overlay buffer is not ready.
                        PPStart(hPP);
                        PPInterruptEnable(hPP, FRAME_INTERRUPT);

                        // If per-pixel alpha is enabled, swap the input and combining channels...
                        // We want overlay to be the combining input in this case, since the
                        // overlay contains the per-pixel alpha data.
                        if (g_ppData4ICCombine[i].bCombineEnable && (g_ppData4ICCombine[i].alphaType == icAlphaType_Local))
                        {
                            PPAddInputCombBuffer(hPP, CurrentOverlayBuf[i]);
                            PPAddInputBuffer(hPP, OutPhyAddr[OutputIndex]+g_nOverlayDstBufferOffset[i]);
                        }
                        else
                        {
                            PPAddInputCombBuffer(hPP,OutPhyAddr[OutputIndex]+g_nOverlayDstBufferOffset[i]);
                            PPAddInputBuffer(hPP, CurrentOverlayBuf[i]);
                        }

                        PPAddOutputBuffer(hPP,OutPhyAddr[OutputIndex]+g_nOverlayDstBufferOffset[i]);
                        PPWaitForNotBusy(hPP, FRAME_INTERRUPT);
                        PPStop(hPP);
                    }
                }
            }
            RETAILMSG(GET_PERF_TIME, (TEXT("SetEvent g_hMiddleOverlayDoneEvent - %d \r\n"), GetTickCount()));
            SetEvent(g_hMiddleOverlayDoneEvent);
            g_bICUpdating = FALSE;

            // Display the result, switching the DP Sync flow framebuffer
            // to the IC combining output buffer.
            if(b4SecDisp)
            {
                if(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DC_SYNC_ASYNC_FLOW))
                {
                    CPMEMWriteBuffer(IDMAC_CH_DC_SYNC_ASYNC_FLOW, 0, (UINT32 *)OutPhyAddr[OutputIndex]);
                    IDMACChannelBUF0SetReady(IDMAC_CH_DC_SYNC_ASYNC_FLOW);
                    while(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DC_SYNC_ASYNC_FLOW))
                        Sleep(1);

                }
                else
                {
                    CPMEMWriteBuffer(IDMAC_CH_DC_SYNC_ASYNC_FLOW, 1, (UINT32 *)OutPhyAddr[OutputIndex]);
                    IDMACChannelBUF1SetReady(IDMAC_CH_DC_SYNC_ASYNC_FLOW);
                    while(!IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DC_SYNC_ASYNC_FLOW))
                        Sleep(1);
                }
            }
            else
            {
                if(g_bDoubleBufferMode)
                {
                    iCurBuf = -1;
                }
                else
                {
                    iCurBuf = IDMACChannelQueryTripleCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
                }
                if(iCurBuf == 0)
                {
                    CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 1, (UINT32 *)OutPhyAddr[OutputIndex]);
                    IDMACChannelBUF1SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
                    while(IDMACChannelQueryTripleCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)!= 1)
                        Sleep(1);
                }
                else if(iCurBuf == 1)
                {
                    CPMEMWriteBuffer(IDMACChannelAltChIndex(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE), 0, (UINT32 *)OutPhyAddr[OutputIndex]);
                    IDMACChannelBUF2SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);                
                    while(IDMACChannelQueryTripleCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)!= 2)
                        Sleep(1);

                }
                else if(iCurBuf == 2)
                {
                    CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 0, (UINT32 *)OutPhyAddr[OutputIndex]);
                    IDMACChannelBUF0SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);                
                    while(IDMACChannelQueryTripleCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)!= 0)
                        Sleep(1);

                }
                else
                {
                    if(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE))
                    {
                        CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 0, (UINT32 *)OutPhyAddr[OutputIndex]);
                        IDMACChannelBUF0SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
                        while(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE))
                            Sleep(1);
                    }
                    else
                    {
                        CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 1, (UINT32 *)OutPhyAddr[OutputIndex]);
                        IDMACChannelBUF1SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
                        while(!IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE))
                            Sleep(1);
                    }               
                }

  
            }
            OutputIndex = (OutputIndex+1)%2;
        }
        else
        {
            ERRORMSG(1,(TEXT("Received an unknown event(0x%x)\r\n"),dwEventReturn));
            break;
        }
    }

    // We are shutting down the 2nd overlay.  Make sure to
    // restore the primary surface framebuffer as the source input
    // for the DP Sync flow
    if(b4SecDisp)
    {
        if(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DC_SYNC_ASYNC_FLOW))
        {
            CPMEMWriteBuffer(IDMAC_CH_DC_SYNC_ASYNC_FLOW, 0, (UINT32 *)g_Primary2BufPhysAddr);
            IDMACChannelBUF0SetReady(IDMAC_CH_DC_SYNC_ASYNC_FLOW);
            while(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DC_SYNC_ASYNC_FLOW))
                Sleep(1);
    
        }
        else
        {
            CPMEMWriteBuffer(IDMAC_CH_DC_SYNC_ASYNC_FLOW, 1, (UINT32 *)g_Primary2BufPhysAddr);
            IDMACChannelBUF1SetReady(IDMAC_CH_DC_SYNC_ASYNC_FLOW);
            while(!IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DC_SYNC_ASYNC_FLOW))
                Sleep(1);
        }
    }
    else
    {
        if(g_bDoubleBufferMode)
        {
            iCurBuf = -1;
        }
        else
        {
            iCurBuf = IDMACChannelQueryTripleCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
        }

        if(iCurBuf == 0)
        {
            CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 1, (UINT32 *)g_PrimaryBufPhysAddr);
            IDMACChannelBUF1SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
            while(IDMACChannelQueryTripleCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)!= 1)
                Sleep(1);
        }
        else if(iCurBuf == 1)
        {
            CPMEMWriteBuffer(IDMACChannelAltChIndex(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE), 0, (UINT32 *)g_PrimaryBufPhysAddr);
            IDMACChannelBUF2SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);                
            while(IDMACChannelQueryTripleCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)!= 2)
                Sleep(1);
        
        }
        else if(iCurBuf == 2)
        {
            CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 0, (UINT32 *)g_PrimaryBufPhysAddr);
            IDMACChannelBUF0SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);                
            while(IDMACChannelQueryTripleCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)!= 0)
                Sleep(1);
        
        }
        else
        {
            if(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE))
            {
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 0, (UINT32 *)g_PrimaryBufPhysAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
                while(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE))
                    Sleep(1);
            }
            else
            {
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 1, (UINT32 *)g_PrimaryBufPhysAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
                while(!IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE))
                    Sleep(1);
            }               
        }
    }

    IPUV3DeallocateBuffer(g_hIPUBase, pOutputBuf);
    PPClearBuffers(hPP);
    PPCloseHandle(hPP);
}
