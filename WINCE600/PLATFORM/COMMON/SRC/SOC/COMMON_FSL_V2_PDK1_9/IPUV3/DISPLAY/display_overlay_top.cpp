//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  display_overlay_top.cpp
//
//  The functions in this file support basic overlaying.  When using one 
//  overlay, these functions are used to configure, start, and stop the
//  overlay operation.  These functions control the PRP and DP IPU modules
//  to perform the final set of processing steps for the overlay and 
//  framebuffer before being sent to the display.  When using multiple overlays,
//  these functions handle the final processing step before displaying the result.
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
#include "prp.h"
#include "cm.h"
#include "vdi.h"
#include "dmfc.h"

//-----------------------------------------------------------------------------
// External Functions
extern DWORD BSPGetPixelDepthFromRegistry(VOID);


//-----------------------------------------------------------------------------
// External Variables
extern PANEL_INFO *g_pDI0PanelInfo;
extern PANEL_INFO *g_pDI1PanelInfo;
extern DWORD g_dwCurBGChan;
extern BOOL g_bAsyncPanelActive;
// Synchronization when buffer update.
extern CRITICAL_SECTION g_csBufferLock;
extern HANDLE g_hIPUBase;
extern DWORD g_iCurrentMode;
extern GPEMode g_PrimaryMode;
extern GPEMode g_SecondaryMode;

extern BOOL g_bOverlayUpdating;
extern BOOL g_bUIUpdated;
extern UINT32 g_nOverlayBufferOffset[MAX_VISIBLE_OVERLAYS];
extern UINT32 g_nOverlayDstBufferOffset[MAX_VISIBLE_OVERLAYS];

extern pOverlaySurf_t  *g_ppOverlays;
extern UINT32 g_iNumOverlays;

//-----------------------------------------------------------------------------
// Defines
#define LOCK_PRP()      EnterCriticalSection(&g_csPrPLock)
#define UNLOCK_PRP()    LeaveCriticalSection(&g_csPrPLock)
#define PRPWIDTH(a)     (a - 1)

#define GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH 16
#define GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH 8
#define GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH 0

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
HANDLE           g_hPrp;                // handle to Pre-processor driver class
HANDLE           g_hVDI = NULL;         // handle to Pre-processor driver class
CRITICAL_SECTION g_csPrPLock;    // Critical section for PrP

vdiBuffer       g_PrevField = {0};

//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: Display_Top_OverlayInit
//
// This function initializes global variables for overlay support.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void Display_Top_OverlayInit()
{
    g_hPrp = PrPOpenHandle();
    g_hVDI = VDIOpenHandle();
    InitializeCriticalSection(&g_csPrPLock);    
}

//------------------------------------------------------------------------------
//
// Function: Display_Top_OverlayDeInit
//
// This function clean up global variables for overlay support.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void Display_Top_OverlayDeInit()
{
    PrPCloseHandle(g_hPrp);
    if (g_hVDI)
    {
        VDICloseHandle(g_hVDI);
    }
}

//------------------------------------------------------------------------------
//
// Function: Display_Top_SetOverlayBuffer
//
// A wrapper of PrPAddInputBuffer
//
// Parameters:
//      overlayIndex
//          [in] index into overlay array to identify overlay buffer.
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
BOOL Display_Top_SetOverlayBuffer(UINT32 overlayIndex, BOOL bWait)
{
    BOOL ret = TRUE;
    PANEL_INFO * pPanelInfo;
    UINT32 YBufLen,srcBufOffset;
    UINT32 V1BufOffset = 0;
    UINT32 U1BufOffset = 0;
    UINT32 V2BufOffset = 0;
    UINT32 U2BufOffset = 0;
    UINT32 PhysicalBuf = g_ppOverlays[overlayIndex]->nBufPhysicalAddr + g_nOverlayBufferOffset[overlayIndex];

    // Get variables for Y buffer start, line stride, and U buffer offset,
    // since we will re-use them in computations below
    UINT32 lineStride = g_ppOverlays[overlayIndex]->SrcLineStride;

    // Compute U and V buffer offsets for planar formats (YV12, NV12)
    // For other input formats, U and V offsets will not be used
    YBufLen = g_ppOverlays[overlayIndex]->SrcWidth * g_ppOverlays[overlayIndex]->SrcHeight;
    if (g_ppOverlays[overlayIndex]->SrcPixelFormat == ddgpePixelFormat_YV12)
    {
        // For YUV 4:2:0 case, compute U and V buffer offsets.
        //Compute u,v buffer offset from source rectangle, the base offset changed
        srcBufOffset = g_ppOverlays[overlayIndex]->SrcRect.top * g_ppOverlays[overlayIndex]->SrcWidth /4
                    + (g_ppOverlays[overlayIndex]->SrcRect.left /2) - g_nOverlayBufferOffset[overlayIndex];
        //V plane is in front of U plane.
        V1BufOffset = YBufLen + srcBufOffset;
        U1BufOffset = YBufLen + (YBufLen/4) + srcBufOffset;
        V2BufOffset = V1BufOffset - g_ppOverlays[overlayIndex]->SrcLineStride/2;
        U2BufOffset = U1BufOffset - g_ppOverlays[overlayIndex]->SrcLineStride/2;
        
    }
    else if (g_ppOverlays[overlayIndex]->SrcPixelFormat == ddgpePixelFormat_NV12)
    {
        // For partial interleaved YUV 4:2:0 case, compute U and V buffer offsets.
        // For this pixel format, hardware only care ubufoffset, the vbufferoffset will be ignored.
        V1BufOffset = YBufLen;
        U1BufOffset = V1BufOffset;
        V2BufOffset = YBufLen;
        U2BufOffset = V2BufOffset;
    }


    if(g_pDI1PanelInfo->ISACTIVE)
    {
        pPanelInfo=g_pDI1PanelInfo; 
    }
    else
    {
        pPanelInfo=g_pDI0PanelInfo; 
    }

    BOOL bIsYUVFormat;
    // Check overlay surface format...only YUV can be de-interlaced
    if ((g_ppOverlays[overlayIndex]->SrcPixelFormat == ddgpePixelFormat_YV12)
        || (g_ppOverlays[overlayIndex]->SrcPixelFormat == ddgpePixelFormat_NV12)
        || (g_ppOverlays[overlayIndex]->SrcPixelFormat == ddgpePixelFormat_YUYV)
        || (g_ppOverlays[overlayIndex]->SrcPixelFormat == ddgpePixelFormat_YUY2)
        || (g_ppOverlays[overlayIndex]->SrcPixelFormat == ddgpePixelFormat_UYVY))
    {
        bIsYUVFormat = TRUE;
    }
    else
    {
        bIsYUVFormat = FALSE;
    }

    // Case 1: Interlaced overlay data (Requires Mem->VDI->IC)
    if ((g_ppOverlays[overlayIndex]->isInterlaced)
        &&(g_ppOverlays[overlayIndex]->SrcWidth<=720)
        &&(g_ppOverlays[overlayIndex]->DstWidth<=1024) && bIsYUVFormat && g_hVDI)

    {
        vdiStartParams startParams;
        memset(&startParams,0, sizeof(vdiStartParams));

        // Given the following sequence of fields:
        // LF = Last Frame, CF = Current Frame, F0 = Field 0, F1 = Field 1
        // LF/F0, LF/F1, CF/F0, CF/F1
        // We de-interlace using LF/F1 as our previous field,
        //                       CF/F0 as our current field, and
        //                       CF/F1 as our next field.

        // Configure current and next field from the current frame
        if (g_ppOverlays[overlayIndex]->TopField == TopFieldSelect_Even)
        {
            startParams.curField.yBufPtr = (UINT32 *)PhysicalBuf;
            startParams.curField.uOffset = U1BufOffset;
            startParams.curField.vOffset = V1BufOffset;
            startParams.curField.dwField = IPU_VDI_C_VDI_TOP_FIELD_FIELD0;

            // For Field 1, the line stride is the Field 1 offset
            startParams.nextField.yBufPtr = (UINT32 *)(PhysicalBuf + lineStride);
            startParams.nextField.uOffset = U2BufOffset;
            startParams.nextField.vOffset = V2BufOffset;
            startParams.nextField.dwField = IPU_VDI_C_VDI_TOP_FIELD_FIELD1;
        }
        else
        {
            startParams.curField.yBufPtr = (UINT32 *)(PhysicalBuf + lineStride);
            startParams.curField.uOffset = U2BufOffset;
            startParams.curField.vOffset = V2BufOffset;
            startParams.curField.dwField = IPU_VDI_C_VDI_TOP_FIELD_FIELD1;

            // For Field 1, the line stride is the Field 1 offset
            startParams.nextField.yBufPtr = (UINT32 *)PhysicalBuf;
            startParams.nextField.uOffset = U1BufOffset;
            startParams.nextField.vOffset = V1BufOffset;
            startParams.nextField.dwField = IPU_VDI_C_VDI_TOP_FIELD_FIELD0;
        }

        // Configure previous field info last, since we may need to duplicate nextField
        // If this is our first frame, the previous field will be NULL,
        // in which case, we duplicate the current even frame
        if (g_PrevField.yBufPtr != NULL)
        {
            startParams.prevField.yBufPtr = g_PrevField.yBufPtr;
            startParams.prevField.uOffset = g_PrevField.uOffset;
            startParams.prevField.vOffset = g_PrevField.vOffset;
            startParams.prevField.dwField = g_PrevField.dwField;
        }
        else
        {
            startParams.prevField.yBufPtr = startParams.nextField.yBufPtr;
            startParams.prevField.uOffset = startParams.nextField.uOffset;
            startParams.prevField.vOffset = startParams.nextField.vOffset;
            startParams.prevField.dwField = startParams.nextField.dwField;
        }
        

        startParams.bAutomaticStop = 0;

        //DEBUGMSG(1, (TEXT("%s: --- VDI Start data ---\r\n Prev->YBufPtr = 0x%x\r\n Prev->UOffset = %d\r\n Prev->VOffset = %d\r\n Cur->YBufPtr = 0x%x\r\n Cur->UBufPtr = %d\r\n Cur->VBufPtr = %d\r\n"), 
        //        __WFUNCTION__, startParams.prevField.yBufPtr, startParams.prevField.uOffset, startParams.prevField.vOffset,
        //        startParams.curField.yBufPtr, startParams.curField.uOffset, startParams.curField.vOffset));

        //DEBUGMSG(1, (TEXT("Next->YBufPtr = 0x%x\r\n Next->UBufPtr = %d\r\n Next->VBufPtr = %d\r\n"), 
        //        startParams.nextField.yBufPtr, startParams.nextField.uOffset, startParams.nextField.vOffset));

        VDIStart(g_hVDI, &startParams);

        // Save current Field 1 as g_PrevField
        // The top line value will impact the prevfield setting.
        if(g_ppOverlays[overlayIndex]->SrcRect.top%2)
        {
            g_PrevField.yBufPtr = startParams.curField.yBufPtr;
            g_PrevField.uOffset = startParams.curField.uOffset;
            g_PrevField.vOffset = startParams.curField.vOffset;
            g_PrevField.dwField = startParams.curField.dwField;
        }
        else
        {
            g_PrevField.yBufPtr = startParams.nextField.yBufPtr;
            g_PrevField.uOffset = startParams.nextField.uOffset;
            g_PrevField.vOffset = startParams.nextField.vOffset;
            g_PrevField.dwField = startParams.nextField.dwField;
        }
    }
    // Case 2: Progressive overlay data (Mem->IC)
    else
    {
        if(pPanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
        {
            if(g_bUIUpdated)
            {
                while(DPIsBusy(DP_CHANNEL_ASYNC0))
                {
                    Sleep(1);
                }
                g_bUIUpdated = FALSE;
            }
            g_bOverlayUpdating = TRUE;
        }
        LOCK_PRP();
        ret = PrPAddInputBuffer(g_hPrp, PhysicalBuf, bWait, FIRSTMODULE_INTERRUPT);
        UNLOCK_PRP();
        
        // Save current Field 1 as g_PrevField
        // We save off progressive data since our next frame could be interlaced
        if(g_ppOverlays[overlayIndex]->SrcRect.top%2)
        {
            g_PrevField.yBufPtr = (UINT32 *)PhysicalBuf;
            g_PrevField.uOffset = U1BufOffset;
            g_PrevField.vOffset = V1BufOffset;
            g_PrevField.dwField = IPU_VDI_C_VDI_TOP_FIELD_FIELD0;
        }
        else
        {
            g_PrevField.yBufPtr = (UINT32 *)(PhysicalBuf + lineStride);
            g_PrevField.uOffset = U2BufOffset;
            g_PrevField.vOffset = V2BufOffset;
            g_PrevField.dwField = IPU_VDI_C_VDI_TOP_FIELD_FIELD1;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
//
// Function: Display_Top_StopOverlay
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
void Display_Top_StopOverlay()
{
    // Now, stop DP flow
    PrPStop(g_hPrp);
    if (g_hVDI)
    {
        VDIStop(g_hVDI);
    }
    g_bOverlayUpdating = FALSE;
}

//------------------------------------------------------------------------------
//
// Function: Display_Top_DisableOverlay
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
BOOL Display_Top_DisableOverlay()
{
    UINT16 uCount = 0;

    PrPStop(g_hPrp);
    if (g_hVDI)
    {
        VDIStop(g_hVDI);
    }
    PrPClearBuffers(g_hPrp);
    if (g_bAsyncPanelActive)
    {
        EnterCriticalSection(&g_csBufferLock);
        while(IDMACChannelIsBusy(g_dwCurBGChan))
        {
            if (uCount <= 1000)
            {
                //..give up the remainder of time slice
                Sleep(1);
                uCount++;
            }
            else
            {
                //.. there is something wrong ....break out
                RETAILMSG(1,
                    (TEXT("%s(): Error in stopping DP bg channel!\r\n"),__WFUNCTION__));
                break;
            }    
        }    
        // Disable the Async Video flow in Display Interface Layer
        DisplayDisableSingleFlow(DISPLAY_FLOW_ASYNC_VIDEO);
        g_dwCurBGChan = IDMAC_CH_DC_SYNC_ASYNC_FLOW;
        LeaveCriticalSection(&g_csBufferLock);

    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: Display_Top_SetupOverlay
//
// This function translates ddraw parameter to hardware specific parameter and setup hardware.
//
// Parameters:
//      pOverlaySurfaceOp
//          [in] structure for overlay surface parameter.
//
// Returns:
//          True if succeed.
//
//------------------------------------------------------------------------------
void Display_Top_SetupOverlay()
{
    // Set up DP combine

    // Ensure IPU clocks enabled before writing to IPU registers
    IPUV3EnableClocks(g_hIPUBase);

    // configure DP  task
    Display_SetupDP4Overlay(g_ppOverlays[g_iNumOverlays-1]);

    // Configure PrP task
    Display_SetupPrP4Overlay(g_iNumOverlays-1);

    // We can make this call to disable IPU HSP clock.  It won't
    // have any effect if other IPU submodules remain enabled.
    IPUV3DisableClocks(g_hIPUBase);
}

//------------------------------------------------------------------------------
//
// Function: Display_Top_StartOverlay
//
// This function starts hardware operation.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_Top_StartOverlay()
{
    PANEL_INFO * pPanelInfo;
    UINT32 topIndex = g_iNumOverlays - 1;
    UINT32 buf = g_ppOverlays[topIndex]->nBufPhysicalAddr + g_nOverlayBufferOffset[topIndex];
    int targetDispFreq;

    // Assumption: If DI1 is active, then it will always be the target for the overlay
    if(g_pDI1PanelInfo->ISACTIVE)
    {
        pPanelInfo=g_pDI1PanelInfo; 
    }
    else
    {
        pPanelInfo=g_pDI0PanelInfo; 
    }

    // Start DP flow

    // Enable Async flow to ensure IPU modules are on before starting the PrP module.
    // For Sync flow, IPU modules should already be on.
    if (g_bAsyncPanelActive&&(g_dwCurBGChan != IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE))
    {
        EnterCriticalSection(&g_csBufferLock);
        g_dwCurBGChan = IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE;
        DisplayEnableSingleFlow(DISPLAY_FLOW_ASYNC_VIDEO);
        LeaveCriticalSection(&g_csBufferLock);
    }

    EnterCriticalSection(&g_csBufferLock);
    PrPStart(g_hPrp);
    if(pPanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
    {
        g_bOverlayUpdating = TRUE;
        g_bUIUpdated = FALSE;
        PrPAddInputBuffer(g_hPrp,buf,TRUE,FRAME_INTERRUPT);
        Sleep(25);
        PrPAddInputBuffer(g_hPrp,buf,TRUE,FRAME_INTERRUPT);
        Sleep(25);
    }
    else
    {
        BOOL bIsYUVFormat;
        // Check overlay surface format...only YUV can be de-interlaced
        if ((g_ppOverlays[topIndex]->SrcPixelFormat == ddgpePixelFormat_YV12)
            || (g_ppOverlays[topIndex]->SrcPixelFormat == ddgpePixelFormat_NV12)
            || (g_ppOverlays[topIndex]->SrcPixelFormat == ddgpePixelFormat_YUYV)
            || (g_ppOverlays[topIndex]->SrcPixelFormat == ddgpePixelFormat_YUY2)
            || (g_ppOverlays[topIndex]->SrcPixelFormat == ddgpePixelFormat_UYVY))
        {
            bIsYUVFormat = TRUE;
        }
        else
        {
            bIsYUVFormat = FALSE;
        }
       
        // Case 1: Interlaced overlay data (Requires Mem->VDI->IC)
        if ((g_ppOverlays[topIndex]->isInterlaced)
            &&(g_ppOverlays[topIndex]->SrcWidth<=720)
            &&(g_ppOverlays[topIndex]->DstWidth<=1024) && bIsYUVFormat && g_hVDI)
        {
            vdiStartParams startParams;
            UINT32 YBufLen,srcBufOffset;
            UINT32 V1BufOffset = 0;
            UINT32 U1BufOffset = 0;
            UINT32 V2BufOffset = 0;
            UINT32 U2BufOffset = 0;

            YBufLen = g_ppOverlays[topIndex]->SrcWidth * g_ppOverlays[topIndex]->SrcHeight;
            if (g_ppOverlays[topIndex]->SrcPixelFormat == ddgpePixelFormat_YV12)
            {
                // For YUV 4:2:0 case, compute U and V buffer offsets.
                //Compute u,v buffer offset from source rectangle, the base offset changed
                srcBufOffset = g_ppOverlays[topIndex]->SrcRect.top * g_ppOverlays[topIndex]->SrcWidth /4
                            + (g_ppOverlays[topIndex]->SrcRect.left /2) - g_nOverlayBufferOffset[topIndex];
                //V plane is in front of U plane.
                V1BufOffset = YBufLen + srcBufOffset;
                U1BufOffset = YBufLen + (YBufLen/4) + srcBufOffset;
                // Subtract lineStride to compensate for yPtr offset
                V2BufOffset = V1BufOffset - g_ppOverlays[topIndex]->SrcLineStride/2;
                U2BufOffset = U1BufOffset - g_ppOverlays[topIndex]->SrcLineStride/2;
                
            }
            else if (g_ppOverlays[0]->SrcPixelFormat == ddgpePixelFormat_NV12)
            {
                // For partial interleaved YUV 4:2:0 case, compute U and V buffer offsets.
                //For this pixel format, hardware only care ubufoffset, the vbufferoffset will be ignored.
                V1BufOffset = YBufLen;
                U1BufOffset = V1BufOffset;
                V2BufOffset = YBufLen;
                U2BufOffset = V2BufOffset;
            }

            memset(&startParams,0, sizeof(vdiStartParams));
        
            // Given the following sequence of fields:
            // LF = Last Frame, CF = Current Frame, F0 = Field 0, F1 = Field 1
            // LF/F0, LF/F1, CF/F0, CF/F1
            // We de-interlace using LF/F1 as our previous field,
            //                       CF/F0 as our current field, and
            //                       CF/F1 as our next field.

            // Configure current and next field from the current frame
            if (g_ppOverlays[topIndex]->TopField == TopFieldSelect_Even)
            {
                startParams.curField.yBufPtr = (UINT32 *)buf;
                startParams.curField.uOffset = U1BufOffset;
                startParams.curField.vOffset = V1BufOffset;
                startParams.curField.dwField = IPU_VDI_C_VDI_TOP_FIELD_FIELD0;

                // For Field 1, the line stride is the Field 1 offset
                startParams.nextField.yBufPtr = (UINT32 *)(buf + g_ppOverlays[topIndex]->SrcLineStride);
                startParams.nextField.uOffset = U2BufOffset;
                startParams.nextField.vOffset = V2BufOffset;
                startParams.nextField.dwField = IPU_VDI_C_VDI_TOP_FIELD_FIELD1;
            }
            else
            {
                startParams.curField.yBufPtr = (UINT32 *)(buf + g_ppOverlays[topIndex]->SrcLineStride);
                startParams.curField.uOffset = U2BufOffset;
                startParams.curField.vOffset = V2BufOffset;
                startParams.curField.dwField = IPU_VDI_C_VDI_TOP_FIELD_FIELD1;

                // For Field 1, the line stride is the Field 1 offset
                startParams.nextField.yBufPtr = (UINT32 *)buf;
                startParams.nextField.uOffset = U1BufOffset;
                startParams.nextField.vOffset = V1BufOffset;
                startParams.nextField.dwField = IPU_VDI_C_VDI_TOP_FIELD_FIELD0;
            }

            // Duplicate next field as the previous field for the first frame only
            startParams.prevField.yBufPtr = startParams.nextField.yBufPtr;
            startParams.prevField.uOffset = startParams.nextField.uOffset;
            startParams.prevField.vOffset = startParams.nextField.vOffset;
            startParams.prevField.dwField = startParams.nextField.dwField;

            startParams.bAutomaticStop = 0;

            //DEBUGMSG(1, (TEXT("%s: --- VDI Start data ---\r\n Prev->YBufPtr = 0x%x\r\n Prev->UOffset = %d\r\n Prev->VOffset = %d\r\n Cur->YBufPtr = 0x%x\r\n Cur->UBufPtr = %d\r\n Cur->VBufPtr = %d\r\n"), 
            //        __WFUNCTION__, startParams.prevField.yBufPtr, startParams.prevField.uOffset, startParams.prevField.vOffset,
            //        startParams.curField.yBufPtr, startParams.curField.uOffset, startParams.curField.vOffset));

            //DEBUGMSG(1, (TEXT("Next->YBufPtr = 0x%x\r\n Next->UBufPtr = %d\r\n Next->VBufPtr = %d\r\n"), 
            //        startParams.nextField.yBufPtr, startParams.nextField.uOffset, startParams.nextField.vOffset));

            VDIStart(g_hVDI, &startParams);

            // Save our current "next" Field 1 as g_PrevField
            // The top value will impact the prevfield setting.
            memcpy(&g_PrevField, &startParams.nextField,sizeof(vdiBuffer));
        }
        // Case 2: Progressive overlay data (Mem->IC)
        else
        {
            PrPAddInputBuffer(g_hPrp,buf,TRUE,FRAME_INTERRUPT);
        }

        // Get mode info for overlay display target...we need to use
        // the target display frequency
        if (g_iCurrentMode == (DWORD)g_SecondaryMode.modeId)
        {
            // Grab frequency from secondary mode
            targetDispFreq = g_SecondaryMode.frequency;
        }
        else
        {
            // Grab frequency from primary mode
            targetDispFreq = g_PrimaryMode.frequency;
        }

        // Use target panel frequency to determine how long we should sleep
        if (targetDispFreq != 0)
        {
            Sleep(1000/targetDispFreq);
        }
        else
        {
            // default 60Hz
            Sleep(17);
        }
    }

    LeaveCriticalSection(&g_csBufferLock);

}

//------------------------------------------------------------------------------
//
// Function: Display_Top_SetupOverlayWindowPos
//
// This function is for seting up overlay window position.
//
// Parameters:
//      x
//          [in] x position.
//
//      y
//          [in] y position.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_Top_SetupOverlayWindowPos(UINT16 x, UINT16 y)
{
    PANEL_INFO * pPanelInfo;
    if(g_pDI1PanelInfo->ISACTIVE)
    {
        pPanelInfo=g_pDI1PanelInfo; 
    }
    else
    {
        pPanelInfo=g_pDI0PanelInfo; 
    }

    // With only one overlay, we use the standard flow...
    // Mem->IC(resize)->Mem->DP->Display

    if(pPanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
    {
        // Ensure IPU clocks enabled before writing to IPU registers
        IPUV3EnableClocks(g_hIPUBase);
        DPPartialPlanePosition(DP_CHANNEL_ASYNC0, x, y); 
        // We can make this call to disable IPU HSP clock.  It won't
        // have any effect if other IPU submodules remain enabled.
        IPUV3DisableClocks(g_hIPUBase);
    }
    else
    {
        if (pPanelInfo->ISINTERLACE)
        {
            //For interlace mode the y value should be divided by 2, 
            //hardware will double it automatically.
            DPPartialPlanePosition(DP_CHANNEL_SYNC, x, y/2);
        }
        else
        {
            // Progressive mode
            DPPartialPlanePosition(DP_CHANNEL_SYNC, x, y);
        }
    }

    return;
}

//------------------------------------------------------------------------------
//
// Function: Display_SetupDP4Overlay
//
// This function setup parameter for display processor.
//
// Parameters:
//      pOverlaySurfaceInfo
//          [in] structure for overlay surface parameter.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_SetupDP4Overlay(pOverlaySurf_t pOverlaySurfaceInfo)
{
    dpGraphicWindowConfigData dispConfigData;
    UINT32 tempColorKey;
    UINT32 RGBColorKey;
    //BOOL bYUVColorKey;

    PANEL_INFO * pPanelInfo;
    if(g_pDI1PanelInfo->ISACTIVE)
    {
        pPanelInfo=g_pDI1PanelInfo; 
    }
    else
    {
        pPanelInfo=g_pDI0PanelInfo; 
    }

    dispConfigData.iXpos = pOverlaySurfaceInfo->XOffset;
    dispConfigData.iYpos = pOverlaySurfaceInfo->YOffset;
    dispConfigData.alpha = (UINT8)pOverlaySurfaceInfo->Transparency;
    dispConfigData.bPartialPlane = pOverlaySurfaceInfo->ColorKeyPlane;
    dispConfigData.bGlobalAlpha = pOverlaySurfaceInfo->bGlobalAlpha;

    // TODO: Resolve how to determine if we are using YUV in combining
    //    bYUVColorKey = m_bTV ? TRUE: FALSE;
    //bYUVColorKey = FALSE;

    // Adjust Color Key so that we can program it into the IPU
    tempColorKey = pOverlaySurfaceInfo->ColorKeyMask;

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
            RGBColorKey = tempColorKey & 0x00ffffff;
            break;
        default:
            RGBColorKey = 0;
            break;

    }

    if(pPanelInfo->PIXEL_FMT ==IPU_PIX_FMT_YUV422)
    {
        switch(pOverlaySurfaceInfo->SrcPixelFormat)
        {
            case ddgpePixelFormat_YV12:
            case ddgpePixelFormat_NV12:
            case ddgpePixelFormat_YUYV:
            case ddgpePixelFormat_YUY2:
            case ddgpePixelFormat_UYVY:
                RGBColorKey = DPColorKeyConv_A1(RGBColorKey,TRUE);
                break;

            case ddgpePixelFormat_8888:
            case ddgpePixelFormat_8880:
            case ddgpePixelFormat_565:
            case ddgpePixelFormat_5550:
            case ddgpePixelFormat_5551:
            default:
                //csc is after combining ,we can use RGB colorkey directly.
                break;                
        }
        //RGBColorKey = 0x008080;


    }    

    // If the color key is 0xFFFFFFFF, make sure to propagate this value
    // since it indicates that color key should be turned off.
    if (pOverlaySurfaceInfo->ColorKeyMask == 0xFFFFFFFF)
    {
        dispConfigData.colorKey = 0xFFFFFFFF;
    }
    else
    {
        dispConfigData.colorKey = RGBColorKey;
    }
    if(pPanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
    {
        DPGraphicWindowConfigure(DP_CHANNEL_ASYNC0, &dispConfigData);
    }
    else
    {
        if (pPanelInfo->ISINTERLACE)
        {
            //For interlace mode the y value should be divided by 2, 
            //hardware will double it automatically.
            dispConfigData.iYpos = dispConfigData.iYpos/2;
        }
        DPGraphicWindowConfigure(DP_CHANNEL_SYNC, &dispConfigData);    
    }
    return;
}
//------------------------------------------------------------------------------
//
// Function: Display_OverlayParaCheck
//
// This function aligns the overlay surface parameter for source and destination rectangle 
// according to hardware limitation.
//
// Parameters:
//      pSrcRect
//          [in/out] sturcture for source rectangle.
//
//      pDstRect
//          [in/out] sturcture for destination rectangle
//
//      iRotate
//          [in] rotation flag.
//
//      PixelFormat
//          [in] the surface pixel format.
//
//      RealWidth
//          [in] the real width of the frame, it's used for checking cropping feature.
//
//      bPerPixelAlpha
//          [in] TRUE if surface contains per-pixel alpha data, FALSE if it does not.
//
// Returns:
//      TRUE if success, FALSE if failed.
//
//------------------------------------------------------------------------------
BOOL Display_OverlayParamAlign(RECT *pSrcRect, RECT *pDstRect,
                   int iRotate, EDDGPEPixelFormat PixelFormat, int RealWidth, BOOL bPerPixelAlpha)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(RealWidth);

    UINT16 srcWidth, srcHeight, dstWidth, dstHeight;

    //As the u,v offset must be divisible by 8(cpmem limitation), 
    //according to the code at the end of the function Display_SetupPrP4Overlay() in file display_overlay.cpp
    //We can get the limitation for src.top and src.left.
    if ((PixelFormat == ddgpePixelFormat_YV12)
       &&((pSrcRect->top & 0x3)||( pSrcRect->left & 0xf)))
    {
        RETAILMSG(1, (TEXT("For YV12 surface the src.top must be aligned with 4 ")
                       TEXT("and the src.left must be aligned with 16, ")
                       TEXT("Current value(t:%d, l:%d)\r\n"), 
                       pSrcRect->top,pSrcRect->left));
        pSrcRect->top  = pSrcRect->top & 0xFFFC;
        pSrcRect->left = pSrcRect->left & 0xFFF0;
    }
    else if(pSrcRect->left & 0x7)
    {
        //As the buffer offset must be aligned with 8.
        RETAILMSG(1, (TEXT("The src.left must be aligned with 8, ")
                       TEXT("Current value(t:%d, l:%d)\r\n"), 
                       pSrcRect->top,pSrcRect->left));
        pSrcRect->left = pSrcRect->left & 0xFFF8;    
    }

    srcWidth  = (UINT16) (pSrcRect->right - pSrcRect->left);
    srcHeight = (UINT16) (pSrcRect->bottom - pSrcRect->top);
    dstWidth  = (UINT16) (pDstRect->right - pDstRect->left);
    dstHeight = (UINT16) (pDstRect->bottom - pDstRect->top);

    //For oversized frame(width larger than 800), we will split them by 2, so the limitation of src and dst width will be aligned with 16.
    //Oversized frame is only support in no rotated case
    if(dstWidth > PRP_MAX_OUTPUT_WIDTH )
    {
        if(iRotate == DMDO_0)
        {
            RETAILMSG(1, (TEXT("Oversized frame: ")
                          TEXT("Current src width(%d)\r\n")
                          TEXT("Current dst width(%d)\r\n"), srcWidth, dstWidth));
            srcWidth  = srcWidth & 0xFFF0;
            dstWidth  = dstWidth & 0xFFF0;
            if(((pSrcRect->top!=0)||(pSrcRect->left!=0))&&(PixelFormat == ddgpePixelFormat_YV12))
            {
                RETAILMSG(1, (TEXT("YV12 oversized frame doesn't support up-left corner cropping.\r\n")));
                //If top, left is not equal to zero, the following u,v offset calculation for right half frame will be incorrect.
                return FALSE;
            }
        }
        else
        {
            ERRORMSG(1, (TEXT("Oversized frame doesn't support rotation! ")));
            return FALSE;        
        }
    }
    

    //destination width and height align
    //force the width to be multiples of 8.
    //The limitation from IDMAC, the minimize burst size of IDMA is 8.
    if(dstWidth & 0x7)
    {
        RETAILMSG(1, (TEXT("The dst width with must be aligned with 8, ")
                       TEXT("Current value(%d)\r\n"), dstWidth));
        dstWidth  = dstWidth & 0xFFF8;

    }
    

    // For rotation, we must force the height to be multiples of 8.
    if (iRotate && (dstHeight & 0x7))
    {
        RETAILMSG(1, (TEXT("For rotation, the dst height with must be aligned with 8, ")
                       TEXT("Current value(%d)\r\n"), dstHeight));
        dstHeight = dstHeight & 0xFFF8;
    }

    //overlay output height is restricted by the U, V offset calculation
    if(((PixelFormat == ddgpePixelFormat_YV12)||(PixelFormat == ddgpePixelFormat_NV12) ) 
                &&(dstHeight & 0x3))
    {
        //According to limitation from u offset calculation
        //if the width can't be divided by 16, the height must be divded by 4.
        RETAILMSG(1, (TEXT("For Interleave format, dst height must be aligned 2 ")
                       TEXT("and dst area must be aligned with 32 for YV12, ")
                       TEXT("Current value(w:%d, h:%d)\r\n"), 
                       dstWidth, dstHeight));
        if((dstWidth & 0xF)&&(PixelFormat == ddgpePixelFormat_YV12))
            dstHeight = dstHeight & 0xFFFC; 
        else
            dstHeight = dstHeight & 0xFFFE; 
    }

    //source width and height align
    //Force the width to be multiples of 8.
    //The limitation from IDMAC, the minimize burst size of IDMA is 8.
    //Application should do this check by itself, but driver can't confirm it.
    if (srcWidth & 0x7)
    {
        RETAILMSG(1, (TEXT("The src width with must be aligned with 8, ")
                       TEXT("Current value(%d)\r\n"), srcWidth));
        srcWidth  = srcWidth & 0xFFF8;
    }

    // Force height to be multiples of 8, if we are rotated.
    if (iRotate && (srcHeight & 0x7))
    {
        RETAILMSG(1, (TEXT("For rotation, the src height with must be aligned with 8, ")
                       TEXT("Current value(%d)\r\n"), srcHeight));
        srcHeight = srcHeight & 0xFFF8;
    }

    //overlay input height is restricted by the U, V offset calculation
    if(((PixelFormat == ddgpePixelFormat_YV12)||(PixelFormat == ddgpePixelFormat_NV12) )
                &&(srcHeight & 0x3))
    {
        //According to limitation from u offset calculation
        //if the width can't be divided by 16, the height must be divded by 4.
        RETAILMSG(1, (TEXT("For Interleave format, src height must be aligned 2 ")
                       TEXT("and src area must be aligned with 32 for YV12, ")
                       TEXT("Current value(w:%d, h:%d)\r\n"), 
                       srcWidth, srcHeight));
        if((srcWidth & 0xF)&&(PixelFormat == ddgpePixelFormat_YV12))
            srcHeight = srcHeight & 0xFFFC; 
        else
            srcHeight = srcHeight & 0xFFFE;
    }

    // If the overlay image is 32bpp and uses local alpha, it cannot be resized
    if (((PixelFormat == ddgpePixelFormat_8888) && bPerPixelAlpha)
        && ((srcWidth != dstWidth) || (srcHeight != dstHeight) || iRotate))
    {
        ERRORMSG(1, (TEXT("Overlay with per-pixel alpha cannot be resized or rotated (it will lose alpha data)!\r\n")));
        return FALSE;     
    }

    pSrcRect->right = pSrcRect->left + srcWidth;
    pSrcRect->bottom = pSrcRect->top + srcHeight;
    pDstRect->right = pDstRect->left + dstWidth;
    pDstRect->bottom = pDstRect->top + dstHeight;

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: Display_SetupPrP4Overlay
//
// This function configure the Pre-processor to perform rotation,
// color space conversion, and resizing for the video overlay.
//
// Parameters:
//      overlayIndex
//          [in] index into overlay array to identify overlay buffer. For second panel,
//                it only support one overlay, so the index should always be 0.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_SetupPrP4Overlay(UINT32 overlayIndex)
{
    EDDGPEPixelFormat inputPixFormat;
    UINT32 YBufLen;
    INT32 srcBufOffset;
    RECT * pSrcRect;
    prpConfigData   prpData_Overlay;
    BOOL bIsYUVFormat = FALSE;

    PANEL_INFO * pPanelInfo;
    GPEMode *pCurMode;

    pOverlaySurf_t pOverlaySurfaceInfo = g_ppOverlays[overlayIndex];
    
    if(g_pDI1PanelInfo->ISACTIVE)
    {
        pPanelInfo=g_pDI1PanelInfo; 
    }
    else
    {
        pPanelInfo=g_pDI0PanelInfo; 
    }

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


    pSrcRect = &pOverlaySurfaceInfo->SrcRect;
    inputPixFormat = pOverlaySurfaceInfo->SrcPixelFormat;

    memset(&prpData_Overlay,0, sizeof(prpConfigData));
    // Set up post-processing configuration data

    // Set up input format and data width

    if (inputPixFormat < MAX_EDDGPE_FORMATS)
    {
        prpData_Overlay.inputIDMAChannel.FrameFormat = EDDGPEFormatToIcFormat[inputPixFormat];
        prpData_Overlay.inputIDMAChannel.DataWidth = EDDGPEFormatToIcDataWidth[inputPixFormat];
    }
    else if(inputPixFormat == ddgpePixelFormat_CustomFormat)
    {
        //for NV12
        prpData_Overlay.inputIDMAChannel.FrameFormat = EDDGPEFormatToIcFormat[19];
        prpData_Overlay.inputIDMAChannel.DataWidth = EDDGPEFormatToIcDataWidth[19];
    }
    else
    {
        // Use RGB565 as a fallback default
        inputPixFormat = ddgpePixelFormat_565;
        prpData_Overlay.inputIDMAChannel.FrameFormat = icFormat_RGB;
        prpData_Overlay.inputIDMAChannel.DataWidth = icDataWidth_16BPP;
    }
    
    switch(inputPixFormat)
    {
        case ddgpePixelFormat_YV12:
        case ddgpePixelFormat_NV12:
            bIsYUVFormat = TRUE;
            break;

        case ddgpePixelFormat_YUYV:
        case ddgpePixelFormat_YUY2:
            bIsYUVFormat = TRUE;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_offset = 0;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_offset = 8;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_offset = 16;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_offset = 24;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_width = PRPWIDTH(8);
            break;

        case ddgpePixelFormat_UYVY:
            bIsYUVFormat = TRUE;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_offset = 0;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_offset = 8;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_offset = 16;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_offset = 24;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_width = PRPWIDTH(8);
            break;

        case ddgpePixelFormat_8888:
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_offset = 8;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_offset = 16;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_offset = 24;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_offset = 0;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_width = PRPWIDTH(8);
            break;
            
        case ddgpePixelFormat_8880:
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_offset = 0;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_offset = 8;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_offset = 16;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_offset = 24;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_width = PRPWIDTH(8);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_width = 0;
            break;
        case ddgpePixelFormat_565:
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_offset = 0;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_offset = 5;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_offset = 11;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_offset = 24; //no use, but must keep
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_width = PRPWIDTH(5);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_width = PRPWIDTH(6);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_width = PRPWIDTH(5);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_width = 0;
            break;
        case ddgpePixelFormat_5550:
        case ddgpePixelFormat_5551:
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_offset = 0;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_offset = 5;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_offset = 10;
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_offset = 24; //no use
            prpData_Overlay.inputIDMAChannel.PixelFormat.component0_width = PRPWIDTH(5);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component1_width = PRPWIDTH(5);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component2_width = PRPWIDTH(5);
            prpData_Overlay.inputIDMAChannel.PixelFormat.component3_width = 0;
            break;

        default:
            DEBUGMSG(GPE_ZONE_ERROR,  (TEXT("%s: Invalid PRP input format, %d !! \r\n"), 
                        __WFUNCTION__, prpData_Overlay.inputIDMAChannel.FrameFormat));
            break;
    }
    
    //Input parameters setting
    prpData_Overlay.inputIDMAChannel.FrameSize.height = (UINT16) (pOverlaySurfaceInfo->SrcRect.bottom - pOverlaySurfaceInfo->SrcRect.top);
    prpData_Overlay.inputIDMAChannel.FrameSize.width = (UINT16) (pOverlaySurfaceInfo->SrcRect.right - pOverlaySurfaceInfo->SrcRect.left);
    prpData_Overlay.inputIDMAChannel.LineStride = pOverlaySurfaceInfo->SrcLineStride;

    if (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV420)
    {
        // For YUV 4:2:0 case, compute U and V buffer offsets.
        YBufLen = pOverlaySurfaceInfo->SrcWidth * pOverlaySurfaceInfo->SrcHeight;
        //Compute Y buffer offset from source rectangle, it's the base offset.
        g_nOverlayBufferOffset[overlayIndex] = pSrcRect->top * pOverlaySurfaceInfo->SrcWidth + pSrcRect->left;
        //Compute u,v buffer offset from source rectangle, the base offset changed
        srcBufOffset = pSrcRect->top * pOverlaySurfaceInfo->SrcWidth /4 + (pSrcRect->left /2) - g_nOverlayBufferOffset[g_iNumOverlays-1];
        //V plane is in front of U plane.
        prpData_Overlay.inputIDMAChannel.VBufOffset = YBufLen + srcBufOffset;
        prpData_Overlay.inputIDMAChannel.UBufOffset = YBufLen + (YBufLen/4) + srcBufOffset;

    }
    else if(prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV420P)
    {
        // For patrial interleaved YUV 4:2:0 case, compute U and V buffer offsets.
        YBufLen = pOverlaySurfaceInfo->SrcWidth * pOverlaySurfaceInfo->SrcHeight;
        //Compute Y buffer offset from source rectangle, it's the base offset.
        g_nOverlayBufferOffset[overlayIndex] = pSrcRect->top * pOverlaySurfaceInfo->SrcWidth + pSrcRect->left;
        //For this pixel format, hardware only care ubufoffset, the vbufferoffset will be ignored.
        prpData_Overlay.inputIDMAChannel.VBufOffset = YBufLen;
        prpData_Overlay.inputIDMAChannel.UBufOffset = prpData_Overlay.inputIDMAChannel.VBufOffset;
    }
    else
    {
        // Compute buffer offset from source rectangle
        g_nOverlayBufferOffset[overlayIndex] = pSrcRect->top * pOverlaySurfaceInfo->SrcLineStride + pSrcRect->left * pOverlaySurfaceInfo->SrcBpp / 8;
    }
//    pOverlaySurfaceInfo->isInterlaced = TRUE;
    prpData_Overlay.bInputIsInterlaced = pOverlaySurfaceInfo->isInterlaced;

    // If our surface is YUV, configure the Video De-Interlacer,
    // which will be chained with the IC to perform other 
    // processing to the surface.
    if (pOverlaySurfaceInfo->isInterlaced && bIsYUVFormat && g_hVDI)
    {
        //-----------------------------
        // Configure VDI
        //-----------------------------
        vdiConfigData configData;

        configData.iHeight = pOverlaySurfaceInfo->SrcHeight;
        configData.iWidth = pOverlaySurfaceInfo->SrcWidth;
        configData.iStride = pOverlaySurfaceInfo->SrcLineStride;
        configData.Uoffset = prpData_Overlay.inputIDMAChannel.UBufOffset;
        configData.Voffset = prpData_Overlay.inputIDMAChannel.VBufOffset;

        // Only 2 possible formats as input to VDI - YV12 and NV12
        switch (inputPixFormat)
        {
            case ddgpePixelFormat_YV12:
            configData.dwPixelFormat = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420;
                break;
            case ddgpePixelFormat_NV12:
            configData.dwPixelFormat = IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420; 
                break;
            case ddgpePixelFormat_UYVY:
                configData.dwPixelFormat = IDMAC_INTERLEAVED_FORMAT_CODE_UYVY2;
                break;
            default:
                configData.dwPixelFormat = IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420;
                break;
        }

        configData.inputSrc = VDI_INPUT_SOURCE_MEM;
        configData.outputDst = VDI_OUTPUT_DEST_IC;
        configData.dwTopField = pOverlaySurfaceInfo->TopField;
//        configData.dwMotionSel = IPU_VDI_C_VDI_MOT_SEL_ROM1;
        configData.dwMotionSel = IPU_VDI_C_VDI_MOT_SEL_FULL_MOTION;

//        DEBUGMSG(1, (TEXT("%s: --- VDI Config data ---\r\n Width = %d\r\n Height = %d\r\n Stride = %d\r\n UOffset = %d\r\n VOffset = %d\r\n"), 
//                __WFUNCTION__, configData.iWidth, configData.iHeight, configData.iStride, configData.Uoffset, configData.Voffset));

        VDIConfigure(g_hVDI, &configData);
    }

//    prpData_Overlay.bVFEnable = !g_pDI1PanelInfo->ISACTIVE;
    prpData_Overlay.bVFEnable = TRUE;

    if(pPanelInfo->PIXEL_FMT == IPU_PIX_FMT_YUV422)
    {
        //outputVFIDMAChannel.FrameFormat is used for csc formula selection.
        prpData_Overlay.outputVFIDMAChannel.FrameFormat = icFormat_UYVY422;
        prpData_Overlay.outputVFIDMAChannel.DataWidth = icDataWidth_16BPP;
        prpData_Overlay.outputVFIDMAChannel.PixelFormat.component0_offset = 0;
        prpData_Overlay.outputVFIDMAChannel.PixelFormat.component1_offset = 8;
        prpData_Overlay.outputVFIDMAChannel.PixelFormat.component2_offset = 16;
        prpData_Overlay.outputVFIDMAChannel.PixelFormat.component3_offset = 24;
        prpData_Overlay.outputVFIDMAChannel.PixelFormat.component0_width = PRPWIDTH(8);
        prpData_Overlay.outputVFIDMAChannel.PixelFormat.component1_width = PRPWIDTH(8);
        prpData_Overlay.outputVFIDMAChannel.PixelFormat.component2_width = PRPWIDTH(8);
        prpData_Overlay.outputVFIDMAChannel.PixelFormat.component3_width = PRPWIDTH(8);
    }
    else
    {
        //For RGB output, outputVFIDMAChannel setting is not used in most use cases.
        //It's only used in Local alpha top overlay+VDI->prp secnondary overlay use case.
        //For this case, we use mode bpp to setup outputVFIDMAChannel instead.
        switch (pCurMode->Bpp)
        {
            case 32:
                 // We use 32bpp datawidth and offsets so we can preserve potential alpha data,
                 // and b/c 32bpp is more efficient to use (in bandwidth and performance terms)
                 prpData_Overlay.outputVFIDMAChannel.FrameFormat = icFormat_RGB;
                 prpData_Overlay.outputVFIDMAChannel.DataWidth = icDataWidth_32BPP;
                 prpData_Overlay.outputVFIDMAChannel.PixelFormat.component0_offset = 8;
                 prpData_Overlay.outputVFIDMAChannel.PixelFormat.component1_offset = 16;
                 prpData_Overlay.outputVFIDMAChannel.PixelFormat.component2_offset = 24;
                 prpData_Overlay.outputVFIDMAChannel.PixelFormat.component3_offset = 0;
                 prpData_Overlay.outputVFIDMAChannel.PixelFormat.component0_width = PRPWIDTH(8);
                 prpData_Overlay.outputVFIDMAChannel.PixelFormat.component1_width = PRPWIDTH(8);
                 prpData_Overlay.outputVFIDMAChannel.PixelFormat.component2_width = PRPWIDTH(8);
                 prpData_Overlay.outputVFIDMAChannel.PixelFormat.component3_width = PRPWIDTH(8);
            
                 break;


           case 24:
                // We use 32bpp datawidth and offsets so we can preserve potential alpha data,
                // and b/c 32bpp is more efficient to use (in bandwidth and performance terms)
                prpData_Overlay.outputVFIDMAChannel.FrameFormat = icFormat_RGB;
                prpData_Overlay.outputVFIDMAChannel.DataWidth = icDataWidth_24BPP;
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component0_offset = 0;
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component1_offset = 8;
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component2_offset = 16;
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component3_offset = 24;
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component0_width = PRPWIDTH(8);
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component1_width = PRPWIDTH(8);
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component2_width = PRPWIDTH(8);
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component3_width = PRPWIDTH(8);

                break;

            case 16:
                prpData_Overlay.outputVFIDMAChannel.FrameFormat = icFormat_RGB;
                prpData_Overlay.outputVFIDMAChannel.DataWidth = icDataWidth_16BPP;
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component0_offset = 0;
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component1_offset = 5;
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component2_offset = 11;
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component3_offset = 24; //no use
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component0_width = PRPWIDTH(5);
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component1_width = PRPWIDTH(6);
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component2_width = PRPWIDTH(5);
                prpData_Overlay.outputVFIDMAChannel.PixelFormat.component3_width = 0;
                break;
            default:
                DEBUGMSG(GPE_ZONE_ERROR, (TEXT("%s: Display output format invalid\r\n"), __WFUNCTION__));
                break;
        }
    }
    // Set up post-processing channel CSC parameters
    // based on input and output
    switch(prpData_Overlay.outputVFIDMAChannel.FrameFormat)
    {
        case icFormat_YUV444:
        case icFormat_YUV422:
        case icFormat_YUV420:
        case icFormat_YUV420P:
        case icFormat_YUV444IL:
        case icFormat_YUYV422:
        case icFormat_YVYU422:
        case icFormat_UYVY422:
        case icFormat_VYUY422:
            if ((prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_RGB) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_RGBA))
                prpData_Overlay.VFCSCEquation = CSCR2Y_A1;
            else
                prpData_Overlay.VFCSCEquation = CSCNoOp;
            break;
        case icFormat_RGB:
        case icFormat_RGBA:
            if ((prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV444IL) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV444) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV422) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV420) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV420P) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUYV422) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YVYU422) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_UYVY422) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_VYUY422))
                prpData_Overlay.VFCSCEquation = CSCY2R_A1;
            else
                prpData_Overlay.VFCSCEquation = CSCNoOp;
            break;
    }

    switch (pOverlaySurfaceInfo->iRotate)
    {
        case DMDO_0:
            prpData_Overlay.VFFlipRot.verticalFlip = FALSE;
            prpData_Overlay.VFFlipRot.horizontalFlip = FALSE;
            prpData_Overlay.VFFlipRot.rotate90 = FALSE;
            break;
        case DMDO_90:
            prpData_Overlay.VFFlipRot.verticalFlip = TRUE;
            prpData_Overlay.VFFlipRot.horizontalFlip = TRUE;
            prpData_Overlay.VFFlipRot.rotate90 = TRUE;
            break;
        case DMDO_180:
            prpData_Overlay.VFFlipRot.verticalFlip = TRUE;
            prpData_Overlay.VFFlipRot.horizontalFlip = TRUE;
            prpData_Overlay.VFFlipRot.rotate90 = FALSE;
            break;
        case DMDO_270:
            prpData_Overlay.VFFlipRot.verticalFlip = FALSE;
            prpData_Overlay.VFFlipRot.horizontalFlip = FALSE;
            prpData_Overlay.VFFlipRot.rotate90 = TRUE;
            break;
    }
#if 0
    if(bForceOverlayUnRotated)
    {
            prpData_Overlay.flipRot.verticalFlip = FALSE;
            prpData_Overlay.flipRot.horizontalFlip = FALSE;
            prpData_Overlay.flipRot.rotate90 = FALSE;
    }
#endif
    if(pOverlaySurfaceInfo->isUpsideDown)
        prpData_Overlay.VFFlipRot.verticalFlip = prpData_Overlay.VFFlipRot.verticalFlip ? FALSE :TRUE;   
        
    prpData_Overlay.outputVFIDMAChannel.FrameSize.height = pOverlaySurfaceInfo->DstHeight;
    prpData_Overlay.outputVFIDMAChannel.FrameSize.width = pOverlaySurfaceInfo->DstWidth; 
    prpData_Overlay.outputVFIDMAChannel.LineStride = pOverlaySurfaceInfo->DstLineStride;  
/*
        DEBUGMSG(GPE_ZONE_HW, (TEXT("%s: Height: %x\n Width: %x\n HeightHw: %x\n WidthHw: %x\n LineStride: %x\n myheight: %x\n mywidth: %x\n mystride: %x\n\r\n"),
            __WFUNCTION__, pOverlaySurfaceOp->Height, pOverlaySurfaceOp->Width,
            pOverlaySurfaceOp->HeightHw, pOverlaySurfaceOp->WidthHw,
            pOverlaySurfaceOp->LineStride, prpData_Overlay.outputSize.height,
            prpData_Overlay.outputSize.width, prpData_Overlay.outputStride));
*/

    if(pPanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
    {
        prpData_Overlay.bVFIsSyncFlow = FALSE; //FALSE:asynchronous flow dp async0 channel
    }
    else
    {
        prpData_Overlay.bVFIsSyncFlow = TRUE;
    }

    if (pPanelInfo->ISINTERLACE)
    {
        prpData_Overlay.bVFIsInterlaced = TRUE;// interlace
    }
    else
    {
        prpData_Overlay.bVFIsInterlaced = FALSE;// progressive
    }

    // We currently use only Pre-processor Viewfinding path, since the
    // VDI->PRP flow only uses viewfinder path.
    // This code was originally designed to support the use case where
    // the same overlay data is destined for multiple display panels.
#if 0 
    prpData_Overlay.bENCEnable = g_pDI1PanelInfo->ISACTIVE;
    // Set up output format and data width
    switch (g_pDI1PanelInfo->PIXEL_FMT)
    {
        // WinCE 6.0: UYVY422 has been renamed UYVY.
        // TODO: ddgpePixelFormat_UYVY for tv, not supported temporarily
        case IPU_PIX_FMT_YUV422:
            prpData_Overlay.outputENCIDMAChannel.FrameFormat = icFormat_UYVY422;
            prpData_Overlay.outputENCIDMAChannel.DataWidth = icDataWidth_16BPP;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component0_offset = 0;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component1_offset = 8;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component2_offset = 16;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component3_offset = 24;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component0_width = PRPWIDTH(8);
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component1_width = PRPWIDTH(8);
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component2_width = PRPWIDTH(8);
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component3_width = PRPWIDTH(8);
            break;

       case IPU_PIX_FMT_RGB24:
       case IPU_PIX_FMT_RGB666:
            // We use 32bpp datawidth and offsets so we can preserve potential alpha data,
            // and b/c 32bpp is more efficient to use (in bandwidth and performance terms)
            prpData_Overlay.outputENCIDMAChannel.FrameFormat = icFormat_RGB;
            prpData_Overlay.outputENCIDMAChannel.DataWidth = icDataWidth_32BPP;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component0_offset = 8;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component1_offset = 16;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component2_offset = 24;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component3_offset = 0;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component0_width = PRPWIDTH(8);
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component1_width = PRPWIDTH(8);
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component2_width = PRPWIDTH(8);
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component3_width = PRPWIDTH(8);

            break;

        case IPU_PIX_FMT_RGB565:
            prpData_Overlay.outputENCIDMAChannel.FrameFormat = icFormat_RGB;
            prpData_Overlay.outputENCIDMAChannel.DataWidth = icDataWidth_16BPP;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component0_offset = 0;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component1_offset = 5;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component2_offset = 11;
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component3_offset = 24; //no use
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component0_width = PRPWIDTH(5);
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component1_width = PRPWIDTH(6);
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component2_width = PRPWIDTH(5);
            prpData_Overlay.outputENCIDMAChannel.PixelFormat.component3_width = 0;
            break;
        default:
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("%s: Display output format invalid\r\n"), __WFUNCTION__));
            break;
    }

    // Set up post-processing channel CSC parameters
    // based on input and output
    switch(prpData_Overlay.outputENCIDMAChannel.FrameFormat)
    {
        case icFormat_YUV444:
        case icFormat_YUV422:
        case icFormat_YUV420:
        case icFormat_YUV420P:
        case icFormat_YUV444IL:
        case icFormat_YUYV422:
        case icFormat_YVYU422:
        case icFormat_UYVY422:
        case icFormat_VYUY422:
            if ((prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_RGB) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_RGBA))
                prpData_Overlay.ENCCSCEquation = CSCR2Y_A1;
            else
                prpData_Overlay.ENCCSCEquation = CSCNoOp;
            break;
        case icFormat_RGB:
        case icFormat_RGBA:
            if ((prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV444IL) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV444) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV422) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV420) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUV420P) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YUYV422) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_YVYU422) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_UYVY422) ||
                (prpData_Overlay.inputIDMAChannel.FrameFormat == icFormat_VYUY422))
                prpData_Overlay.ENCCSCEquation = CSCY2R_A1;
            else
                prpData_Overlay.ENCCSCEquation = CSCNoOp;
            break;
    }

    switch (pOverlaySurfaceInfo->iRotate)
    {
        case DMDO_0:
            prpData_Overlay.ENCFlipRot.verticalFlip = FALSE;
            prpData_Overlay.ENCFlipRot.horizontalFlip = FALSE;
            prpData_Overlay.ENCFlipRot.rotate90 = FALSE;
            break;
        case DMDO_90:
            prpData_Overlay.ENCFlipRot.verticalFlip = TRUE;
            prpData_Overlay.ENCFlipRot.horizontalFlip = TRUE;
            prpData_Overlay.ENCFlipRot.rotate90 = TRUE;
            break;
        case DMDO_180:
            prpData_Overlay.ENCFlipRot.verticalFlip = TRUE;
            prpData_Overlay.ENCFlipRot.horizontalFlip = TRUE;
            prpData_Overlay.ENCFlipRot.rotate90 = FALSE;
            break;
        case DMDO_270:
            prpData_Overlay.ENCFlipRot.verticalFlip = FALSE;
            prpData_Overlay.ENCFlipRot.horizontalFlip = FALSE;
            prpData_Overlay.ENCFlipRot.rotate90 = TRUE;
            break;
    }
#if 0
    if(bForceOverlayUnRotated)
    {
            prpData_Overlay.flipRot.verticalFlip = FALSE;
            prpData_Overlay.flipRot.horizontalFlip = FALSE;
            prpData_Overlay.flipRot.rotate90 = FALSE;
    }
#endif
    if(pOverlaySurfaceInfo->isUpsideDown)
        prpData_Overlay.ENCFlipRot.verticalFlip = prpData_Overlay.ENCFlipRot.verticalFlip ? FALSE :TRUE;   
        
    prpData_Overlay.outputENCIDMAChannel.FrameSize.height = pOverlaySurfaceInfo->DstHeight;
    prpData_Overlay.outputENCIDMAChannel.FrameSize.width = pOverlaySurfaceInfo->DstWidth; 
    prpData_Overlay.outputENCIDMAChannel.LineStride = pOverlaySurfaceInfo->DstLineStride;  //acctually pp didn't use this parameter

    if(g_pDI1PanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
    {
        prpData_Overlay.bENCIsSyncFlow = FALSE; //FALSE:asynchronous flow dp async0 channel
    }
    else
    {
        prpData_Overlay.bENCIsSyncFlow = TRUE;
    }
    
    if (g_pDI1PanelInfo->ISINTERLACE)
    {
        prpData_Overlay.bENCIsInterlaced = TRUE;//interlace
    }
    else
    {
         prpData_Overlay.bENCIsInterlaced = FALSE;// progressive
    }
#endif

    //Not support combining yet, for second ovelay surface
    prpData_Overlay.bCombineEnable = FALSE;
    if((g_iNumOverlays == 2)&&(g_ppOverlays[1]->SrcBpp == 32)&&(g_ppOverlays[0]->SrcBpp != 32))
    {
        prpData_Overlay.bNoDirectDP = TRUE;
        if(pPanelInfo->PIXEL_FMT == IPU_PIX_FMT_YUV422)
        {
            prpData_Overlay.outputVFIDMAChannel.LineStride = pCurMode->width*2;              
            pOverlaySurfaceInfo->XOffset=pOverlaySurfaceInfo->XOffset&0xfff8;  // 8 alignment
            g_nOverlayDstBufferOffset[overlayIndex] = prpData_Overlay.outputVFIDMAChannel.LineStride * pOverlaySurfaceInfo->YOffset 
                                    + pOverlaySurfaceInfo->XOffset*2;
        }
        else
        {
            //RGB use case
            switch (pCurMode->Bpp)
            {
               case 32:
                    prpData_Overlay.outputVFIDMAChannel.LineStride = pCurMode->width*4;  
                    pOverlaySurfaceInfo->XOffset=pOverlaySurfaceInfo->XOffset&0xfff8;  // 8 alignment
                    g_nOverlayDstBufferOffset[overlayIndex] = prpData_Overlay.outputVFIDMAChannel.LineStride * pOverlaySurfaceInfo->YOffset 
                                            + pOverlaySurfaceInfo->XOffset*4;
                    break;
                    
               case 24:
                    prpData_Overlay.outputVFIDMAChannel.LineStride = pCurMode->width*3;  
                    pOverlaySurfaceInfo->XOffset=pOverlaySurfaceInfo->XOffset&0xfff8;  // 8 alignment
                    g_nOverlayDstBufferOffset[overlayIndex] = prpData_Overlay.outputVFIDMAChannel.LineStride * pOverlaySurfaceInfo->YOffset 
                                            + pOverlaySurfaceInfo->XOffset*3;
                    break;

                case 16:
                    prpData_Overlay.outputVFIDMAChannel.LineStride = pCurMode->width*2;              
                    pOverlaySurfaceInfo->XOffset=pOverlaySurfaceInfo->XOffset&0xfff8;  // 8 alignment
                    g_nOverlayDstBufferOffset[overlayIndex] = prpData_Overlay.outputVFIDMAChannel.LineStride * pOverlaySurfaceInfo->YOffset 
                                            + pOverlaySurfaceInfo->XOffset*2;
                    break;
                default:
                    DEBUGMSG(GPE_ZONE_ERROR, (TEXT("%s: Display output format invalid\r\n"), __WFUNCTION__));
                    break;
            }
        }
        UINT32 tempColorKey,RGBColorKey;
        //Setup alpha blending
        prpData_Overlay.inputcombAlpha = (UINT8)pOverlaySurfaceInfo->Transparency; 
        
        //Setup colorkey
        if(pOverlaySurfaceInfo->ColorKeyPlane == DisplayPlane_0)
        {
            tempColorKey = pOverlaySurfaceInfo->ColorKeyMask;
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
            if (pOverlaySurfaceInfo->ColorKeyMask == 0xFFFFFFFF)
            {
                prpData_Overlay.inputcombColorkey= 0xFFFFFFFF;
            }
            else
            {
                prpData_Overlay.inputcombColorkey = RGBColorKey;
            }
        }
        else
        {
            prpData_Overlay.inputcombColorkey= 0xFFFFFFFF;
            DEBUGMSG(1,(TEXT("Source colorkey is not supported for second overlay.\r\n")));
        }
        
        // Determine which IC mode we should use.
        if (pOverlaySurfaceInfo->ColorKeyPlane == DisplayPlane_0)
        {
            //Default setting.
        }
        else if (pOverlaySurfaceInfo->ColorKeyPlane == DisplayPlane_1)
        {
            DEBUGMSG(1,(TEXT("Source colorkey is not supported for this overlay surface.\r\n")));
        }
        
        // Enable alpha if:
        // a) global alpha enabled (not equal to 0xff)
        // OR
        // b) color keying enabled (not equal to 0xffffffff)
        if((prpData_Overlay.inputcombAlpha != 0xff)||
           (prpData_Overlay.inputcombColorkey!= 0xFFFFFFFF))
        {
            prpData_Overlay.bCombineEnable = TRUE;
            memcpy(&prpData_Overlay.inputcombVFIDMAChannel,&prpData_Overlay.outputVFIDMAChannel,sizeof(idmaChannel));
        }
    }
    else
    {
        prpData_Overlay.bNoDirectDP = FALSE;
    }
    LOCK_PRP();
    // Configure PrP with given configuration data.
    PrPConfigure(g_hPrp, &prpData_Overlay);
    
    UNLOCK_PRP();

    return;
}


//------------------------------------------------------------------------------
//
// Function: Display_Top_OverlayIsBusy
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
BOOL Display_Top_OverlayIsBusy()
{
    icBufferStatus status;
    // Ensure IPU clocks enabled before writing to IPU registers
    IPUV3EnableClocks(g_hIPUBase);   
    PrPBufferStatus(g_hPrp, &status);
    // We can make this call to disable IPU HSP clock.  It won't
    // have any effect if other IPU submodules remain enabled.
    IPUV3DisableClocks(g_hIPUBase);

    if(status == BufferIdle)
        return FALSE;
    else
        return TRUE;
}
