//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  display_overlay.cpp
//
//  Display Interface Layer functions providing interface between
//  high-level DirectDraw API implementation and low-level IPU register
//  access functions, specified for overlay surface.
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
#include "dc.h"
#include "prp.h"
#include "vdi.h"
#include "cm.h"

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
extern HANDLE           g_hPrp;                // handle to Pre-processor driver class
extern HANDLE           g_hVDI;                // handle to De-interlacing driver class


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
BOOL             g_bOverlayUpdating;
BOOL             g_bUIUpdated;

UINT32           g_nOverlayBufferOffset[MAX_VISIBLE_OVERLAYS];
pOverlaySurf_t  *g_ppOverlays;
UINT32           g_iNumOverlays;

//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: Display_OverlayInit
//
// This function initializes global variables for overlay support.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void Display_OverlayInit()
{
    g_bOverlayUpdating = FALSE;
    g_bUIUpdated = FALSE;
    g_iNumOverlays = 0;

    Display_Top_OverlayInit();
    Display_Middle_OverlayInit();
}

//------------------------------------------------------------------------------
//
// Function: Display_OverlayDeInit
//
// This function clean up global variables for overlay support.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void Display_OverlayDeInit()
{
    Display_Top_OverlayDeInit();
    Display_Middle_OverlayDeInit();
}

//------------------------------------------------------------------------------
//
// Function: Display_SetOverlayBuffer
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
BOOL Display_SetOverlayBuffer(UINT32 overlayIndex, BOOL bWait)
{
    BOOL ret = TRUE;

    // Error check on overlayIndex
    if (overlayIndex >= g_iNumOverlays)
    {
        ERRORMSG(1,
                (TEXT("%s: Invalid overlayIndex greater than the number of overlays!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    if (g_iNumOverlays == 1)
    {
        EnterCriticalSection(&g_csBufferLock);
        ret = Display_Top_SetOverlayBuffer(0, bWait);
        LeaveCriticalSection(&g_csBufferLock);
    }
    else
    {
        if (overlayIndex < g_iNumOverlays-1)
        {
            // Bottom overlay - This overlay is combined using the IC
            // Update position for IC combine operation
            ret = Display_Middle_SetOverlayBuffer(NULL, overlayIndex, bWait);
        }
        else
        {
            // Top overlay - This overlay is combine using the DP
            if((g_iNumOverlays == 2)&&(g_ppOverlays[1]->SrcBpp == 32)&&(g_ppOverlays[0]->SrcBpp != 32))
            {
                //For this special case, the buffer is set to dp directly
                Display_Set_DPOverlay(g_ppOverlays[overlayIndex]->nBufPhysicalAddr + g_nOverlayBufferOffset[overlayIndex]);
            }
            else

            {
                EnterCriticalSection(&g_csBufferLock);
                ret = Display_Top_SetOverlayBuffer(overlayIndex, bWait);
                LeaveCriticalSection(&g_csBufferLock);
            }
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
//
// Function: Display_StopOverlay
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
void Display_StopOverlay()
{
    EnterCriticalSection(&g_csBufferLock);

    // Stop all hardware operation
    if((g_iNumOverlays == 2)&&(g_ppOverlays[1]->SrcBpp == 32)&&(g_ppOverlays[0]->SrcBpp != 32))
    {
        //In this special case, dp configuration code is outside prpstop(). So the code is slight different.
        PrPStop(g_hPrp);
        // Now, stop DP flow
        DPGraphicWindowEnable(DP_CHANNEL_SYNC,FALSE);
        //WAIT FOR dp_sf_end (Sync Flow End)
        UINT32 uCount = 0;
        IDMACChannelClearIntStatus(GENERAL_IPU_INTR_DP_SF_END, IPU_INTR_TYPE_GENERAL);
        while(!IDMACChannelGetIntStatus(GENERAL_IPU_INTR_DP_SF_END, IPU_INTR_TYPE_GENERAL))
        {
            if (uCount <= 200)
            {
                //..give up the remainder of time slice
                Sleep(1);
                uCount++;
            }
            else
            {
                //.. there is something wrong ....break out
                RETAILMSG(1,
                    (TEXT("%s(): Error in waiting DP_SF_END signal!\r\n"),__WFUNCTION__));
                break;
            }    
        }
        
        if(g_pDI1PanelInfo->ISACTIVE)
            DCWait4TripleBufEmpty(DI_SELECT_DI1);
        else
            DCWait4TripleBufEmpty(DI_SELECT_DI0);
        
        IDMACChannelDisable(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
        IDMACChannelDBMODE(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE,FALSE);

        uCount = 0;
        IDMACChannelClearIntStatus(GENERAL_IPU_INTR_DP_SF_END, IPU_INTR_TYPE_GENERAL);
        while(!IDMACChannelGetIntStatus(GENERAL_IPU_INTR_DP_SF_END, IPU_INTR_TYPE_GENERAL))
        {
            if (uCount <= 200)
            {
                //..give up the remainder of time slice
                Sleep(1);
                uCount++;
            }
            else
            {
                //.. there is something wrong ....break out
                RETAILMSG(1,
                    (TEXT("%s(): Error in waiting DP_SF_END signal!\r\n"),__WFUNCTION__));
                break;
            }    
        }
        if (g_hVDI)
        {
            VDIStop(g_hVDI);
        }
       
        // Stop IC combine flow if it is running
        Display_Middle_StopOverlay();
        g_bOverlayUpdating = FALSE;    
    }
    else
    {
        if (g_iNumOverlays > 1)
        {
            // Stop IC combine flow if it is running
            Display_Middle_StopOverlay();
        }
        // Now, stop DP flow
        Display_Top_StopOverlay();
    }
    LeaveCriticalSection(&g_csBufferLock);

}

//------------------------------------------------------------------------------
//
// Function: Display_DisableOverlay
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
BOOL Display_DisableOverlay()
{
    EnterCriticalSection(&g_csBufferLock);

    
    // Stop all hardware operation
    if((g_iNumOverlays == 2)&&(g_ppOverlays[1]->SrcBpp == 32)&&(g_ppOverlays[0]->SrcBpp != 32))
    {
        //In this special case, dp configuration code is outside prpstop(). So the code is slight different.
        PrPStop(g_hPrp);
        DPGraphicWindowEnable(DP_CHANNEL_SYNC,FALSE);
        //WAIT FOR dp_sf_end (Sync Flow End)
        UINT32 uCount = 0;
        IDMACChannelClearIntStatus(GENERAL_IPU_INTR_DP_SF_END, IPU_INTR_TYPE_GENERAL);
        while(!IDMACChannelGetIntStatus(GENERAL_IPU_INTR_DP_SF_END, IPU_INTR_TYPE_GENERAL))
        {
            if (uCount <= 200)
            {
                //..give up the remainder of time slice
                Sleep(1);
                uCount++;
            }
            else
            {
                //.. there is something wrong ....break out
                RETAILMSG(1,
                    (TEXT("%s(): Error in waiting DP_SF_END signal!\r\n"),__WFUNCTION__));
                break;
            }    
        }
        if(g_pDI1PanelInfo->ISACTIVE)
            DCWait4TripleBufEmpty(DI_SELECT_DI1);
        else
            DCWait4TripleBufEmpty(DI_SELECT_DI0);
        
        IDMACChannelDisable(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
        IDMACChannelDBMODE(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE,FALSE);

        uCount = 0;
        IDMACChannelClearIntStatus(GENERAL_IPU_INTR_DP_SF_END, IPU_INTR_TYPE_GENERAL);
        while(!IDMACChannelGetIntStatus(GENERAL_IPU_INTR_DP_SF_END, IPU_INTR_TYPE_GENERAL))
        {
            if (uCount <= 200)
            {
                //..give up the remainder of time slice
                Sleep(1);
                uCount++;
            }
            else
            {
                //.. there is something wrong ....break out
                RETAILMSG(1,
                    (TEXT("%s(): Error in waiting DP_SF_END signal!\r\n"),__WFUNCTION__));
                break;
            }    
        }
        if (g_hVDI)
        {
            VDIStop(g_hVDI);
        }
        PrPClearBuffers(g_hPrp);

        // Disable IC combine flow if it is running
        Display_Middle_DisableOverlay();
        g_bOverlayUpdating = FALSE;    
    }
    else 
    {
        if (g_iNumOverlays > 1)
        {
            // Disable IC combine flow if it is running
            Display_Middle_DisableOverlay();
        }

        Display_Top_DisableOverlay();    
        g_bOverlayUpdating = FALSE;
    }
    LeaveCriticalSection(&g_csBufferLock);

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: Display_GetOverlaySrcBufOffset
//
// This function stops hardware operation, meanwhile clean up buffers.
//
// Parameters:
//          None.
//
// Returns:
//          Overlay source buffer Offset.
//
//------------------------------------------------------------------------------
UINT32 Display_GetOverlaySrcBufOffset(UINT32 overlayIndex)
{
    return g_nOverlayBufferOffset[overlayIndex];
}

//------------------------------------------------------------------------------
//
// Function: Display_SetupOverlay
//
// This function translates ddraw parameter to hardware specific 
// parameter and sets up the hardware.
//
// Parameters:
//      ppOverlayInfo
//          [in] Array of overlay info structures, one for each
//          overlay to be shown.
//
//      numOverlays
//          [in] The number of overlays in the ppOverlayInfo array
//
// Returns:
//          True if succeed.
//
//------------------------------------------------------------------------------

void Display_SetupOverlay(pOverlaySurf_t *ppOverlayInfo, UINT32 numOverlays)
{
    // First, we check to see if we need to disable the second overlay. 
    // I.e., we have just been reduced from >1 to =1 overlays
    if ((g_iNumOverlays > 1) && (numOverlays == 1))
    {
        // Disable IC combine flow if it is running
        Display_Middle_DisableOverlay();
    }

    // Save off overlay array and number of overlays
    g_ppOverlays = ppOverlayInfo;
    g_iNumOverlays = numOverlays;

    // Stop all hardware operation
    if((g_iNumOverlays == 2)&&(g_ppOverlays[1]->SrcBpp == 32)&&(g_ppOverlays[0]->SrcBpp != 32))
    {
        Display_Setup2Overlay();
    }
    else
    {
        if (numOverlays > 1)
        {
            // TODO: Multiple Overlay support
        
            // First, set up IC combine
            Display_Middle_SetupOverlay(NULL); //for configure the second overlay
        }

        // Now set up DP combine
        Display_Top_SetupOverlay();
    }
}

//------------------------------------------------------------------------------
//
// Function: Display_StartOverlay
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
void Display_StartOverlay()
{
    EnterCriticalSection(&g_csBufferLock);
    if((g_iNumOverlays == 2)&&(g_ppOverlays[1]->SrcBpp == 32)&&(g_ppOverlays[0]->SrcBpp != 32))
    {
        Display_Start2Overlay();
    }
    else
    {
        if (g_iNumOverlays != 1)
        {
            // Start IC combine flow
            Display_Middle_StartOverlay(NULL,FALSE);
        }

        // Now start DP flow
        Display_Top_StartOverlay();
    }
    LeaveCriticalSection(&g_csBufferLock);

}

//------------------------------------------------------------------------------
//
// Function: Display_SetupOverlayWindowPos
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
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_SetupOverlayWindowPos(UINT32 overlayIndex, UINT16 x, UINT16 y)
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

    // Error check on overlayIndex
    if (overlayIndex >= g_iNumOverlays)
    {
        ERRORMSG(1,
                (TEXT("%s: Invalid overlayIndex greater than the number of overlays!\r\n"), __WFUNCTION__));
        return;
    }

    if (g_iNumOverlays == 1)
    {
        Display_Top_SetupOverlayWindowPos(x, y);
    }
    else
    {
        if (overlayIndex < g_iNumOverlays-1)
        {
            // Bottom overlay - This overlay is combined using the IC
        
            // Update position for IC combine operation
            Display_Middle_SetupOverlayWindowPos(overlayIndex, x, y, FALSE);
        }
        else
        {
            // Top overlay - This overlay is combine using the DP
            Display_Top_SetupOverlayWindowPos(x, y);
        }
    }

    return;
}


//------------------------------------------------------------------------------
//
// Function: Display_OverlayIsBusy
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
BOOL Display_OverlayIsBusy()
{
    if (Display_Top_OverlayIsBusy() || Display_Middle_OverlayIsBusy())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
//------------------------------------------------------------------------------
//
// Function: Display_Set_DPOverlay
//
// A function to set buffer to dp foreground channel directly
//
// Parameters:
//      phAddr
//          [in] the physical buffer address
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_Set_DPOverlay(UINT32 phAddr)
{
    EnterCriticalSection(&g_csBufferLock);
    //Double buffer mode use another registry to check status. 
    if(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
    {
        CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 0, (UINT32 *)phAddr);
        IDMACChannelBUF0SetReady(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
        LeaveCriticalSection(&g_csBufferLock);
        while(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
            Sleep(1);
    }
    else
    {
        CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, 1, (UINT32 *)phAddr);
        IDMACChannelBUF1SetReady(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE);
        LeaveCriticalSection(&g_csBufferLock);
        while(!IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
            Sleep(1);
    }
    
}

//------------------------------------------------------------------------------
//
// Function: Display_Reconfigure_Interlaced
//
// A function to reconfigure the VDI module enable/disable, we must stop the overlay flow 
// before reconfigure it, and start it again after configuration is finished.
//
// Parameters:
//      overlayIndex
//          [in] index into overlay array to identify overlay buffer.
//
//      isInterlaced
//          [in]   TRUE:  enable VDI in overlay flow.
//                  FALSE: disable VDI in overlay flow.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void Display_Reconfigure_Interlaced(UINT32 overlayIndex, BOOL isInterlaced)
{
    EnterCriticalSection(&g_csBufferLock);
    Display_Top_StopOverlay();
    g_ppOverlays[overlayIndex]->isInterlaced = isInterlaced;
    Display_SetupPrP4Overlay(overlayIndex);
    PrPStart(g_hPrp);
    LeaveCriticalSection(&g_csBufferLock);
    return;
}


