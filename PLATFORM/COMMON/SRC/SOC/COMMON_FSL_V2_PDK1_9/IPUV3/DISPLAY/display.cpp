//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  display.cpp
//
//  Display Interface Layer functions providing interface between
//  high-level DirectDraw API implementation and low-level IPU register
//  access functions.
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
#include "dc.h"
#include "di.h"
#include "dmfc.h"
#include "dp.h"
#include "cm.h"
#include "dirtyrect.h"
#include "prp.h"


//------------------------------------------------------------------------------
// External Functions
extern "C" DWORD CSPIPUGetClk();
extern BOOL  BSPGetPanelInfo(DI_SELECT, PANEL_INFO**);
extern BOOL  BSPGetLCDModeInfo(UINT16 modeNo, PANEL_INFO **);
extern BOOL  BSPGetTVModeInfo(UINT16 modeNo, PANEL_INFO **);
extern BOOL  BSPIsEnabledOnBoot(DI_SELECT);
extern DWORD BSPGetPixelDepthFromRegistry(VOID);
extern DWORD BSPGetVideoPixelDepthFromRegistry(VOID);
extern DWORD BSPGetNumSupportedModes(VOID);
extern void  BSPGetSupportedModes(GPEMode *);
extern BOOL  BSPIsLCDMode(DWORD);
extern BOOL  BSPIsTVMode(DWORD);
extern BOOL  BSPIsRotationSupported(VOID);
extern BOOL  BSPEnableSecondaryPrimarySurface(VOID);
extern void  BSPDisplayIOMUXSetup(VOID);
extern void  BSPInitializePanel(PANEL_INFO*);
extern void  BSPEnablePanel(PANEL_INFO*);
extern void  BSPDisablePanel(PANEL_INFO*);
extern void  BSPEnableDisplayClock(PANEL_INFO*);
extern void  BSPDisableDisplayClock(PANEL_INFO*);
extern void  BSPInitializeDC(DI_SELECT, PANEL_INFO*);
extern void  BSPInitializeDI(DI_SELECT, PANEL_INFO*);
extern void  BSPEnableDCChannels(IPU_PANEL_SYNC_TYPE syncType);
extern void  BSPDisableDCChannels(IPU_PANEL_SYNC_TYPE syncType);
extern BOOL  BSPIsTVSupported(VOID); 
//------------------------------------------------------------------------------
// External Variables
extern HANDLE g_hPrp;                // handle to Pre-processor driver class
extern BOOL g_bOverlayUpdating;
extern BOOL g_bUIUpdated;
extern HANDLE g_hMiddleOverlayUIUpdateEvent;

//------------------------------------------------------------------------------
// Defines
#define DEBUG_DUMP_IPU_REGS   0
#define MIN_DIRTY_RECT_PIX_DIMS 16

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
HANDLE g_hIPUBase;
static UINT32 g_IPUClk = 0;
BOOL g_bDualDevice = FALSE;

// Current panel info
PANEL_INFO *g_pDI0PanelInfo, *g_pDI1PanelInfo;
BOOL g_bDI0Connected = FALSE;
BOOL g_bDI1Connected = FALSE;

// Current display mode descriptors
GPEMode g_PrimaryMode = {0};
GPEMode g_SecondaryMode = {0};
DWORD g_dwCurBGChan;
UINT32 g_PrimaryBufPhysAddr;
UINT32 g_Primary2BufPhysAddr;
UINT32 g_PrimaryBufStride = 0;
UINT32 g_Primary2BufStride = 0;
DWORD  g_iRotate = DMDO_0;
DWORD g_iCurrentMode = 0; // Default to 0, which is the default mode

// System state info
static BOOL g_bPowerOff = FALSE;
static DWORD g_dwFlowsEnabledStatus = 0;
static BOOL g_bDisplayPortsEnabled = FALSE;
static BOOL g_bDPEnabled = FALSE;
BOOL g_bDoubleBufferMode = FALSE;

// Async display update objects
BOOL g_bAsyncPanelActive = FALSE;
HANDLE g_hAsyncUpdateRequest;
DirtyRect *g_pAsyncDirtyRect;
UINT32 g_iNumAsyncDirtyRegions;
HANDLE g_hDisplayUpdateThread;
CRITICAL_SECTION g_csAsyncDirtyRect;

// Interrupt events
HANDLE g_hDCCh1IntrEvent;
HANDLE g_hDPBGIntrEvent;
HANDLE g_hDPFlowEvent;
HANDLE g_hDCFrameCompleteEvent;

// Synchronization when buffer update.
CRITICAL_SECTION g_csBufferLock;

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
static void DCSetup(DI_SELECT);
static void DISetup(DI_SELECT);
static void CPMEMSetup(DWORD dwIDMACChan, int width, int height, int bpp, int stride, BOOL bInterlaced);
static void DMFCInitialize();
static void DMFCSetup(DWORD dwIDMACChan, DI_SELECT di_sel, DWORD dwWidth);
static void GetUpdateRegion(RECT *pRct);
static void WaitForEOF(DWORD dwIDMACChan);
static void WaitForDPSyncFlowEnd();
static void WaitForDPAsyncFlowEnd();
static void WaitForDCFrameComplete();
static void DisplayConfigureSingle(DI_SELECT di_sel);
static void DisplayConfigureDual(DI_SELECT di_sel_main, DI_SELECT di_sel_aux);
static void DisplayCorrectPixelFormatFromRegistry(DI_SELECT di_sel);

#if DEBUG_DUMP_IPU_REGS
static void DumpIPURegs();
#endif


//------------------------------------------------------------------------------
//
// Function: DisplayGetNumSupportedModes
//
// This function retrieves the number of supported modes from the registry.
//
// Parameters:
//      None.
//
// Returns:
//      The number of supported modes.  0 if failure.
//------------------------------------------------------------------------------
DWORD DisplayGetNumSupportedModes()
{
    DWORD totalModes;

    //  Retrieve number of both LCD modes and supported TV modes
    totalModes = BSPGetNumSupportedModes();

    return totalModes;
}

//------------------------------------------------------------------------------
//
// Function: DisplayEanbleSPrimarySurface
//
// This function retrieves secondary primary surface should be enabled.
//
// Parameters:
//      None.
//
// Returns:
//      True if enable, False if disable.
//------------------------------------------------------------------------------
BOOL DisplayEanbleSPrimarySurface()
{
    return BSPEnableSecondaryPrimarySurface();
}


//------------------------------------------------------------------------------
//
// Function: DisplayGetSupportedModes
//
// This function retrieves an array of the supported display modes.
// This function must be called before calling DisplayInitialize()
// or any other Display Interface Layer functions that require
// panel info, as this function will build the PANEL_INFO structures
// for DI0 and DI1.
//
// Parameters:
//      pModeArray
//          [out] Pointer holding array of GPEModes for each possible
//          display mode.
//
// Returns:
//      TRUE if success, FALSE if failure
//------------------------------------------------------------------------------
BOOL DisplayGetSupportedModes(GPEMode* pModeArray)
{
    // Initialize DI0 and DI1 panel info pointers
    g_pDI0PanelInfo = 0;
    g_pDI1PanelInfo = 0;

    //The initial mode can't be 0, as 0 is a valid modeId.
    g_PrimaryMode.modeId = -1;
    g_SecondaryMode.modeId = -1;
    
    // Retrieve array of modes
    BSPGetSupportedModes(pModeArray);

    // Retrieve panel info for DI0 and DI1
    g_bDI0Connected = BSPGetPanelInfo(DI_SELECT_DI0, &g_pDI0PanelInfo);
    g_bDI1Connected = BSPGetPanelInfo(DI_SELECT_DI1, &g_pDI1PanelInfo);

    // Set DI0 panel as primary, if DI0 is connected
    if (g_bDI0Connected)
    { 
        // Correct the pixel format from registry if necessary
        DisplayCorrectPixelFormatFromRegistry(DI_SELECT_DI0);

        // If DI0 connected, the DI0 panel becomes primay mode
        g_PrimaryMode.modeId    = g_pDI0PanelInfo->MODEID;
        g_PrimaryMode.width     = g_pDI0PanelInfo->WIDTH;
        g_PrimaryMode.height    = g_pDI0PanelInfo->HEIGHT;
        g_PrimaryMode.frequency = g_pDI0PanelInfo->FREQUENCY;
    }
    else
    {
        // Allocate memory for DI0PanelInfo and set to 0's
        // This sets ISACTIVE, SYNC_TYPE, and other important vars to 0
        g_pDI0PanelInfo = (PANEL_INFO *)LocalAlloc(LPTR, sizeof(PANEL_INFO));
        memset(g_pDI0PanelInfo, 0, sizeof(PANEL_INFO));
    }

    // Set DI1 panel as primary if DI0 panel is NOT connected
    // Set DI1 panel as secondary if DI0 panel is primary
    if (g_bDI1Connected)
    {
        // Correct the pixel format from registry if necessary
        DisplayCorrectPixelFormatFromRegistry(DI_SELECT_DI1);
    
        if (!g_bDI0Connected)
        {
            // DI1 becomes primary mode since there is no DI0
            g_PrimaryMode.modeId    = g_pDI1PanelInfo->MODEID;
            g_PrimaryMode.width     = g_pDI1PanelInfo->WIDTH;
            g_PrimaryMode.height    = g_pDI1PanelInfo->HEIGHT;
            g_PrimaryMode.frequency = g_pDI1PanelInfo->FREQUENCY;
        }
        else
        {
            // DI1 becomes secondary mode; DI0 is primary mode
            g_SecondaryMode.modeId    = g_pDI1PanelInfo->MODEID;
            g_SecondaryMode.width     = g_pDI1PanelInfo->WIDTH;
            g_SecondaryMode.height    = g_pDI1PanelInfo->HEIGHT;
            g_SecondaryMode.frequency = g_pDI1PanelInfo->FREQUENCY;
        }
    }
    else
    {
        // Allocate memory for DI1PanelInfo and set to 0's
        // This sets ISACTIVE, SYNC_TYPE, and other important vars to 0
        g_pDI1PanelInfo = (PANEL_INFO *)LocalAlloc(LPTR, sizeof(PANEL_INFO));
        memset(g_pDI1PanelInfo, 0, sizeof(PANEL_INFO));

        if (g_bDI0Connected)
        {
            // ** Case 1 **
            // DI0 is primary mode; DI1 disconnected
        }
        else
        {
            ERRORMSG (1, (TEXT("%s: Can't have both DI0 and DI1 disconnected!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: DisplayGetPrimaryMode
//
// This function retrieves the primary mode for the display driver, which
// controls aspects of dual display support (resolution of second primary surf), 
// and therefore must be accessible by the DirectDraw code.
//
// Parameters:
//      None.
//
// Returns:
//      Primary display mode number
//------------------------------------------------------------------------------
DWORD DisplayGetPrimaryMode()
{
    return g_PrimaryMode.modeId;
}

//------------------------------------------------------------------------------
//
// Function: DisplayGetPixelDepth
//
// This function retrieves the UI pixel depth from the registry.
//
// Parameters:
//      None.
//
// Returns:
//      Pixel Depth Value.  0 if failure.
//------------------------------------------------------------------------------
DWORD DisplayGetPixelDepth()
{
    g_PrimaryMode.Bpp = BSPGetPixelDepthFromRegistry();
    g_SecondaryMode.Bpp = g_PrimaryMode.Bpp;
    return g_PrimaryMode.Bpp;
}


//------------------------------------------------------------------------------
//
// Function: DisplayGetPixelDepth
//
// This function retrieves the video pixel depth from the registry.
//
// Parameters:
//      None.
//
// Returns:
//      Pixel Depth Value.  0 if failure.
//------------------------------------------------------------------------------
DWORD DisplayGetVideoPixelDepth()
{
    return BSPGetVideoPixelDepthFromRegistry();
}


//------------------------------------------------------------------------------
//
// Function: DisplayGetVideoMemorySize
//
// This function retrieves the size of the video memory region (from IPU base)
//
// Parameters:
//      None
//
// Returns:
//      Size of video memory.
//------------------------------------------------------------------------------
DWORD DisplayGetVideoMemorySize()
{
    DWORD dwVidMemSize;

    // If first allocation, create IPU Base handle
    if (!g_hIPUBase)
    {
        // open handle to the IPU_BASE driver in order to enable IC module
        // First, create handle to IPU_BASE driver
        g_hIPUBase = IPUV3BaseOpenHandle();
        if (g_hIPUBase == INVALID_HANDLE_VALUE)
        {
            RETAILMSG(1,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
        }
    }

    dwVidMemSize = IPUV3GetVideoMemorySize(g_hIPUBase);

    return dwVidMemSize;
}


//------------------------------------------------------------------------------
//
// Function: DisplayGetVideoMemoryBase
//
// This function retrieves the video memory base (from IPU base)
//
// Parameters:
//      None
//
// Returns:
//      Video memory base.
//------------------------------------------------------------------------------
DWORD DisplayGetVideoMemoryBase()
{
    DWORD dwVidMemBase;

    // If first allocation, create IPU Base handle
    if (!g_hIPUBase)
    {
        // open handle to the IPU_BASE driver in order to enable IC module
        // First, create handle to IPU_BASE driver
        g_hIPUBase = IPUV3BaseOpenHandle();
        if (g_hIPUBase == INVALID_HANDLE_VALUE)
        {
            RETAILMSG(1,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
        }
    }

    dwVidMemBase = IPUV3GetVideoMemoryBase(g_hIPUBase);

    return dwVidMemBase;
}


//------------------------------------------------------------------------------
//
// Function: DisplayIsRotationSupported
//
// This function retrieves info about whether rotation
// is supported in the DirectDraw driver.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if rotation supported, FALSE if not.
//------------------------------------------------------------------------------
BOOL DisplayIsRotationSupported(void)
{
    return BSPIsRotationSupported();
}


//------------------------------------------------------------------------------
//
// Function: DisplayIsTVSupported
//
// This function retrieves info about whether TV
// is supported.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if TV is supported, FALSE if not.
//------------------------------------------------------------------------------
BOOL DisplayIsTVSupported(void)
{
   return BSPIsTVSupported();
}


//------------------------------------------------------------------------------
//
// Function: DisplayInitialize
//
// This function completes several initialization tasks:
//  1) Initializes structures needed to access IPUv3
//      submodule registers.
//  2) Initializes the IPUv3 registers to display to
//      a smart or dumb display device.
//  3) Initializes IOMUX settings for display-related pins.
//
// Parameters:
//      bpp
//          [in] Bits per pixel for the starting UI framebuffer.
//
//      pPhysAddr
//          [in] Physical address of the frame buffer, used to
//          initialize the buffer pointers for the display flows.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL DisplayInitialize(UINT32 bpp, UINT32 *pPhysAddr)
{
    BOOL retVal = TRUE;
    UNREFERENCED_PARAMETER(bpp);
    
    // If first allocation, create IPU Base handle
    if (!g_hIPUBase)
    {
        // open handle to the IPU_BASE driver in order to enable IC module
        // First, create handle to IPU_BASE driver
        g_hIPUBase = IPUV3BaseOpenHandle();
        if (g_hIPUBase == INVALID_HANDLE_VALUE)
        {
            RETAILMSG(1,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
        }
    }

    // Create event for IPU interrupt for DC Channel 1 (BG) EOF
    g_hDCCh1IntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DC_CH1_INTR_EVENT);
    if (g_hDCCh1IntrEvent == NULL)
    {
        ERRORMSG (1, (TEXT("%s: CreateEvent for IPU Interrupt failed!\r\n"), __WFUNCTION__));
        retVal = FALSE;
    }
    
    // Create event for IPU interrupt for DP BG EOF
    g_hDPBGIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DPBG_INTR_EVENT);
    if (g_hDPBGIntrEvent == NULL)
    {
        ERRORMSG (1, (TEXT("%s: CreateEvent for IPU Interrupt failed!\r\n"), __WFUNCTION__));
        retVal = FALSE;
    }

    // Call to turn on IPU clocks for register initializations
    IPUV3EnableClocks(g_hIPUBase);

    // Create event for IPU interrupt for DP Flow
    g_hDPFlowEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DP_FLOW_INTR_EVENT);
    if (g_hDPFlowEvent == NULL)
    {
        ERRORMSG (1, (TEXT("%s: CreateEvent for IPU DP Flow Interrupt failed!\r\n"), __WFUNCTION__));
        retVal = FALSE;
    }


    // Create event for IPU interrupt for DC Frame Complete
    g_hDCFrameCompleteEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DC_FRAME_COMPLETE_INTR_EVENT);
    if (g_hDCFrameCompleteEvent == NULL)
    {
        ERRORMSG (1, (TEXT("%s: CreateEvent for IPU DC FC Interrupt failed!\r\n"), __WFUNCTION__));
        retVal = FALSE;
    }

    // Perform one-time initialization of registers
    // for IPUv3 components that will be used
    if (!CMRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable CM Regs!\r\n"), __WFUNCTION__));
    }

    // Reset all IPU internal memories before 
    // initializing other IPU submodules
    DWORD dwResetFlags = IPU_RESET_MEM_SRM | IPU_RESET_MEM_ALPHA | IPU_RESET_MEM_CPMEM
                        | IPU_RESET_MEM_TPM | IPU_RESET_MEM_MPM | IPU_RESET_MEM_BM 
                        | IPU_RESET_MEM_RM | IPU_RESET_MEM_DSTM | IPU_RESET_MEM_DSOM
                        | IPU_RESET_MEM_LUT0 | IPU_RESET_MEM_LUT1 | IPU_RESET_MEM_DC_TEMPLATE
                        | IPU_RESET_MEM_DMFC_RD | IPU_RESET_MEM_DMFC_WR;
    CMResetIPUMemories(dwResetFlags);

    if (!IDMACRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable IDMAC Regs!\r\n"), __WFUNCTION__));
    }
    if (!DCRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable DC Regs!\r\n"), __WFUNCTION__));
    }
    if (!DIRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable DI Regs!\r\n"), __WFUNCTION__));
    }
    if (!CPMEMRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable CPMEM Regs!\r\n"), __WFUNCTION__));
    }
    if (!DMFCRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable DMFC Regs!\r\n"), __WFUNCTION__));
    }
    if (!DPRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable DP Regs!\r\n"), __WFUNCTION__));
    }
    if (!TPMRegsInit())
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable TPM Regs!\r\n"), __WFUNCTION__));
    }
    
    DMFCInitialize(); //Set a default value for all dmfc channel.

    Display_OverlayInit();

    BSPDisplayIOMUXSetup();

    IDMACChannelTRBMODE(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE,TRUE);
    //For async panel, we can't setup the double buffer mode for DC. 
    //Otherwise EOF signal may lost.
    if(g_pDI0PanelInfo->SYNC_TYPE != IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
    {
        IDMACChannelDBMODE(IDMAC_CH_DC_SYNC_ASYNC_FLOW,TRUE);
    }

    // Initialize CPMEM source buffer before starting
    CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 0, pPhysAddr);
    CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 1, pPhysAddr);
    CPMEMWriteBuffer(IDMACChannelAltChIndex(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE),0,pPhysAddr);
    // Initialize CPMEM source buffer before starting
    
    CPMEMWriteBuffer(IDMAC_CH_DC_SYNC_ASYNC_FLOW, 0, pPhysAddr);
    CPMEMWriteBuffer(IDMAC_CH_DC_SYNC_ASYNC_FLOW, 1, pPhysAddr);
    
    // Call to turn off IPU clocks - register initializations complete
    IPUV3DisableClocks(g_hIPUBase);

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: DisplayEnablePanels
//
// This function enables all active panels.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplayEnablePanels(void)
{
    // Only enable panel if it is active
    if (g_pDI0PanelInfo->ISACTIVE)
    {
        BSPEnablePanel(g_pDI0PanelInfo);
    }

    if (g_pDI1PanelInfo->ISACTIVE)
    {
        BSPEnablePanel(g_pDI1PanelInfo);
    }
}


//------------------------------------------------------------------------------
//
// Function: DisplayDisablePanels
//
// This function disables all actives display panels.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplayDisablePanels(void)
{
    // Only need to disable active panels
    if (g_pDI0PanelInfo->ISACTIVE)
    {
        BSPDisablePanel(g_pDI0PanelInfo);
    }

    if (g_pDI1PanelInfo->ISACTIVE)
    {
        BSPDisablePanel(g_pDI1PanelInfo);
    }
}


//------------------------------------------------------------------------------
//
// Function: DisplayEnableSingleDisplayFlow
//
// This function enables a single display flow.  If no display flows were previously
// enabled, IPU submodules will be enabled as needed.
//
// Parameters:
//      displayFlow
//          Identifies the display flow to enable.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplayEnableSingleFlow(DISPLAY_FLOW displayFlow)
{
    DWORD dwOldStatus;

    dwOldStatus = g_dwFlowsEnabledStatus;

    // Update flows enabled mask by adding newly enabled flow
    g_dwFlowsEnabledStatus |= displayFlow;

    // If any flows using DP are active, set DP flag
    if ((displayFlow == DISPLAY_FLOW_ASYNC_VIDEO) 
        || (displayFlow == DISPLAY_FLOW_SYNC))
    {
        g_bDPEnabled = TRUE;
        // Manually enable DP if other flows were previously enabled, since
        // this means that DisplayEnableActiveFlows may have been called 
        // previously and not enabled the DP.
        if(dwOldStatus != 0)
            DPEnable();
    }

    // If no flows were previously enabled, we need to enable active flows now
    if (dwOldStatus == 0)
    {
        DisplayEnableActiveFlows(FALSE);
    }
}

//------------------------------------------------------------------------------
//
// Function: DisplayDisableSingleFlow
//
// This function disables a single display flow.  If no display flows remain enabled,
// the display-related IPU submodules will be disabled.
//
// Parameters:
//      displayFlow
//          Identifies the display flow to disable
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplayDisableSingleFlow(DISPLAY_FLOW displayFlow)
{
    // Use temp flows status variable, so that we can reference
    // formerly active flows in DisplayDisableActiveFlows()
    DWORD tempFlowsStatus = g_dwFlowsEnabledStatus;

    // Update flows enabled mask by removing new disabled flow
    tempFlowsStatus &= ~displayFlow;

    // If now flows remain active, disable all flows
    if (tempFlowsStatus == 0)
    {
        DisplayDisableActiveFlows(FALSE);
    }

    // Update global flows status variable
    g_dwFlowsEnabledStatus = tempFlowsStatus;

    // If no flows using DP are left active, disable DP flag
    if (!((g_dwFlowsEnabledStatus & DISPLAY_FLOW_ASYNC_VIDEO) 
        || (g_dwFlowsEnabledStatus & DISPLAY_FLOW_SYNC)))
    {
        g_bDPEnabled = FALSE;
        // Manually disable DP if non-DP-dependent flows are still enabled, 
        // since a later call to DisplayDisableActiveFlows will not
        // disable the DP.
        if(g_dwFlowsEnabledStatus != 0)
            DPDisable();
    }
}


//------------------------------------------------------------------------------
//
// Function: DisplayEnableActiveDisplayFlows
//
// This function enables all active display flows.
//
// Parameters:
//      bPowerOn
//          Set to TRUE to indicate that the display driver is moving
//          to the power on state.  Only the display driver power management
//          code should call this function with bPowerOn set to TRUE.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplayEnableActiveFlows(BOOL bPowerOn)
{
    if(g_pDI0PanelInfo->ISACTIVE)
    {
        // Enable alternative display source clock (e.g., DI clk) if needed
        BSPEnableDisplayClock(g_pDI0PanelInfo);

        // Only enable DI modules that are active
        DIEnable(DI_SELECT_DI0);
    }

    if(g_pDI1PanelInfo->ISACTIVE)
    {
        // Enable alternative display source clock (e.g., DI clk) if needed
        BSPEnableDisplayClock(g_pDI1PanelInfo);

        // Only enable DI modules that are active
        DIEnable(DI_SELECT_DI1);
    }

    if(g_bDPEnabled)
    {
        DPEnable();
    }

    DCEnable();

    DMFCEnable();
    if(g_bPowerOff)
    {
        //When gointo stop mode, all idma channel must use single buffer mode
        if(g_dwCurBGChan == IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)
            IDMACChannelTRBMODE(g_dwCurBGChan,TRUE);
        else
            IDMACChannelDBMODE(g_dwCurBGChan,TRUE);
    }
    IDMACChannelEnable(g_dwCurBGChan);
    if(g_pDI0PanelInfo->ISACTIVE && g_pDI1PanelInfo->ISACTIVE)
    {
        if(g_bPowerOff)
        {
            //For async panel, we can't setup the double buffer mode for DC. 
            //Otherwise EOF signal may lost.
            if(g_pDI0PanelInfo->SYNC_TYPE != IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
            {
                IDMACChannelDBMODE(IDMAC_CH_DC_SYNC_ASYNC_FLOW,TRUE);
           }
        }
        IDMACChannelEnable(IDMAC_CH_DC_SYNC_ASYNC_FLOW);
    }
    if(g_pDI0PanelInfo->ISACTIVE)
    {
        // Enable DC channels
        BSPEnableDCChannels(g_pDI0PanelInfo->SYNC_TYPE);

        if (g_pDI0PanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_SYNCHRONOUS)
        {
            CMStartDICounters(DI_SELECT_DI0);        // Start counters
        }
    }
    
    if(g_pDI1PanelInfo->ISACTIVE)
    {
        // Enable DC channels
        BSPEnableDCChannels(g_pDI1PanelInfo->SYNC_TYPE);

        if (g_pDI1PanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_SYNCHRONOUS)
        {
            CMStartDICounters(DI_SELECT_DI1);        // Start counters
        }
    }

    // use bPowerOn to set global variable that controls 
    // operation of display update threads.
    if (bPowerOn)
    {
        g_bPowerOff = FALSE;
    }

    g_bDisplayPortsEnabled = TRUE;
}


//------------------------------------------------------------------------------
//
// Function: DisplayDisableActiveFlows
//
// This function disables all active display flows.
//
// Parameters:
//      bPowerOff
//          Set to TRUE to indicate that the display driver is moving
//          to the power off state.  This is distinct from disabling the 
//          display flows while the system display driver power is still on.
//          Note: Only the display driver power management code should
//          call this function with bPowerOff set to TRUE.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplayDisableActiveFlows(BOOL bPowerOff)
{
    // use bPowerOff to set global variable that controls 
    // operation of display update threads.
    if (bPowerOff)
    {
        g_bPowerOff = TRUE;
    }

    // If all modules are already disabled, we should not
    // disable them again (and don't need to), so bail out.
    // If we try to disable DC when it is already disabled,
    // the call to DCWait4TripleBufEmpty() will hang.
    if (!g_bDisplayPortsEnabled)
    {
        return;
    }

    EnterCriticalSection(&g_csBufferLock);

    //********************************************************************
    // New Disable Sequence(s)
    // -----------------------
    //
    // 1) a) For DP flow - Wait for DP Sync Flow or Async Flow End
    //       interrupt
    //    b) For DC flow - Wait for DC Frame Complete interrupt
    //    ** The purpose of this step is so that the following disable
    //       steps are completed within the vertical blank period for
    //       the flow.
    // 2) DI counter release
    // 3) DC Prog Type Disable
    // 4) IDMAC disable
    // 5) Submodules disable (DC, DI, DP, DMFC, etc.)
    //********************************************************************

    // If we are using Sync DP flow...
    if (g_dwFlowsEnabledStatus & DISPLAY_FLOW_SYNC)
    {
        // Step 1
        WaitForDPSyncFlowEnd();
    }
    // If we are using Async DP flow...
    else if (g_dwFlowsEnabledStatus & DISPLAY_FLOW_ASYNC_VIDEO)
    {
        // Step 1
        WaitForDPAsyncFlowEnd();
    }
    // If we are using DC flow...
    else if (g_dwFlowsEnabledStatus & DISPLAY_FLOW_ASYNC_UI)
    {
        // Step 1
        WaitForDCFrameComplete();
    }
    
    if (g_bDI0Connected)
    {
        // Steps 2 & 3
        CMStopDICounters(DI_SELECT_DI0);
        BSPDisableDCChannels(g_pDI0PanelInfo->SYNC_TYPE);
        DIDisable(DI_SELECT_DI0);
    }

    if (g_bDI1Connected)
    {
        // Steps 2 & 3
        CMStopDICounters(DI_SELECT_DI1);
        BSPDisableDCChannels(g_pDI1PanelInfo->SYNC_TYPE);
        DIDisable(DI_SELECT_DI1);
    }

    // Step 4
    IDMACChannelDisable(g_dwCurBGChan);
    if(g_bPowerOff)
    {
        //When gointo stop mode, all idma channel must use single buffer mode
        if(g_dwCurBGChan == IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)
            IDMACChannelTRBMODE(g_dwCurBGChan,FALSE);
        else
            IDMACChannelDBMODE(g_dwCurBGChan,FALSE);

    }

    if(g_pDI0PanelInfo->ISACTIVE && g_pDI1PanelInfo->ISACTIVE)
    {
        IDMACChannelDisable(IDMAC_CH_DC_SYNC_ASYNC_FLOW);
        if(g_bPowerOff)
        {
            //For async panel, we can't setup the double buffer mode for DC. 
            //Otherwise EOF signal may lost.
            if(g_pDI0PanelInfo->SYNC_TYPE != IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
            {
                IDMACChannelDBMODE(IDMAC_CH_DC_SYNC_ASYNC_FLOW,FALSE);
            }
        }
    }

    // Step 5
    DCDisable();
    if(g_bDPEnabled)
    {
        DPDisable();
    }
    DMFCDisable();

    // Disable alternative display source clock (e.g., DI clk)
    BSPDisableDisplayClock(g_pDI0PanelInfo);
    BSPDisableDisplayClock(g_pDI1PanelInfo);

    LeaveCriticalSection(&g_csBufferLock);

    g_bDisplayPortsEnabled = FALSE;
}


//------------------------------------------------------------------------------
//
// Function: DisplaySetMode
//
// This function uses the WinCE display mode to set active display panels.
//
// Parameters:
//      dispMode
//          [in] The current display mode.
//
//      bSecondaryOn
//          [in] True if need to keep the secondary display device on
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplaySetMode(DWORD dispMode, BOOL bSecondaryOn)
{

    //************************************************************************
    // Handle transition to new mode using the following steps:
    // 1) If special transition timing is required, synchronize as needed
    // 2) Disable panels associated with old mode if they should no longer
    //   be active.
    // 3) Configure IPU for new mode
    // 4) Enable panel for new mode 
    //************************************************************************
    // Handle LCD mode

    // TODO: If we are transition from TV+LCD mode, we must synchronize the
    // switch with the LCD VSYNC to minimize screen flicker during transition.
    // if (currentMode == DISPLAY_MODE_DEVICE_AND_NTSC)

    // We have to retrieve panel info for requested mode, so that we can try
    // to match the requested mode's port type against DI0 and DI1.
    PANEL_INFO *pNewModePanelInfo;
    if (BSPIsLCDMode(dispMode))
    {
        BSPGetLCDModeInfo((UINT16)dispMode, &pNewModePanelInfo);
    }
    else if (BSPIsTVMode(dispMode))
    {
        BSPGetTVModeInfo((UINT16)dispMode, &pNewModePanelInfo);
    }
    else
    {
        pNewModePanelInfo = NULL;
        RETAILMSG(1, (TEXT("%s: Invalid display mode %x requested!\r\n"), __WFUNCTION__, dispMode));
        return;
    }

    // DI0 gets priority - If DI0 uses same port as requested mode, we use it and ignore DI1
    if (g_bDI0Connected && (pNewModePanelInfo->PORTID == g_pDI0PanelInfo->PORTID))
    {
        // First step is to disable IPU and all flows
        // This is important for disabling TV flows, which require shutting down
        // IPU before shutting down TVE.

        // Call to turn on IPU clocks for register initializations
        IPUV3EnableClocks(g_hIPUBase);

        // Disable all flows so that we disable IPU modules while reconfiguring
        // We will re-enable the appropriate flows once configuration is complete.
        DisplayDisableSingleFlow(DISPLAY_FLOW_ALL);

        // Call to turn off IPU clocks for register initializations
        IPUV3DisableClocks(g_hIPUBase);

        // The DI0 port may support more than one mode. If the previous mode on DI0 uses the same port, but is 
        // not equal to newly configured mode, we need to update DI0 panel info with newly configured mode panel info.
        if (g_pDI0PanelInfo->MODEID != dispMode)
        {
            // Disable panel if it is currently enabled
            if (g_pDI0PanelInfo->ISACTIVE == TRUE)
            {
               BSPDisablePanel(g_pDI0PanelInfo);
            }

            // Set old panel info to inactive.
            // (This is a dynamic variable, so it will persist and should be in the 
            // proper state in case we switch back to this mode later)
            g_pDI0PanelInfo->ISACTIVE = FALSE;

            // Now update g_pDI0PanelInfo structure with panel info our new mode
            if (BSPIsLCDMode(dispMode))
            {
                BSPGetLCDModeInfo((UINT16)dispMode, &g_pDI0PanelInfo);
            }
            else
            {
                BSPGetTVModeInfo((UINT16)dispMode, &g_pDI0PanelInfo);
            }

            // Correct the pixel format from registry if necessary
            DisplayCorrectPixelFormatFromRegistry(DI_SELECT_DI0);
        
            // update the primary mode resolution
            g_PrimaryMode.height    = g_pDI0PanelInfo->HEIGHT;
            g_PrimaryMode.width     = g_pDI0PanelInfo->WIDTH;
            g_PrimaryMode.modeId    = g_pDI0PanelInfo->MODEID;
            g_PrimaryMode.frequency = g_pDI0PanelInfo->FREQUENCY;
        }

        // Disable DI1 panel if it was previously enabled
        if (g_bDI1Connected && (g_pDI1PanelInfo->MODEID == g_iCurrentMode))
        {
            BSPDisablePanel(g_pDI1PanelInfo);
            g_pDI1PanelInfo->ISACTIVE = FALSE;
        }

        // Make DI0 active
        g_pDI0PanelInfo->ISACTIVE = TRUE;

        // Configure system for LCD mode, from DI0
        DisplayConfigureSingle(DI_SELECT_DI0);
        BSPEnablePanel(g_pDI0PanelInfo);
    }
    // If DI0 didn't match but DI1 does, we are setting the mode on DI1.
    // For DI1, we must check for dual display mode.
    else if (g_bDI1Connected && (pNewModePanelInfo->PORTID == g_pDI1PanelInfo->PORTID))
    {
        // First step is to disable IPU and all flows
        // This is important for disabling TV flows, which require shutting down
        // IPU before shutting down TVE.

        // Call to turn on IPU clocks for register initializations
        IPUV3EnableClocks(g_hIPUBase);

        // Disable all flows so that we disable IPU modules while reconfiguring
        // We will re-enable the appropriate flows once configuration is complete.
        DisplayDisableSingleFlow(DISPLAY_FLOW_ALL);

        // Call to turn off IPU clocks for register initializations
        IPUV3DisableClocks(g_hIPUBase);


        // Disable DI0 panel if it was formerly enabled, we are changing to DI1 panel, and 
        // there is no no Secondary panel support        
        if (g_bDI0Connected && (g_pDI0PanelInfo->ISACTIVE == TRUE) 
            && (!bSecondaryOn))
        {
            BSPDisablePanel(g_pDI0PanelInfo);
            g_pDI0PanelInfo->ISACTIVE = FALSE;
        }

        // The DI1 port may support more than one mode. If the previous mode on DI1 uses the same port, but is 
        // not equal to newly configured mode, we need to update DI1 panel info with newly configured mode panel info.
        if (g_pDI1PanelInfo->MODEID != dispMode)
        {
            // Disable panel if it is currently enabled
            if (g_pDI1PanelInfo->ISACTIVE == TRUE)
            {
               BSPDisablePanel(g_pDI1PanelInfo);
            }

            // Set old panel info to inactive.
            // (This is a dynamic variable, so it will persist and should be in the 
            // proper state in case we switch back to this mode later)
            g_pDI1PanelInfo->ISACTIVE = FALSE;

            // Now update g_pDI1PanelInfo structure with panel info our new mode
            if (BSPIsLCDMode(dispMode))
            {
            BSPGetLCDModeInfo((UINT16)dispMode, &g_pDI1PanelInfo);
            }
            else
            {
                BSPGetTVModeInfo((UINT16)dispMode, &g_pDI1PanelInfo);
            }

            // Correct the pixel format from registry if necessary
            DisplayCorrectPixelFormatFromRegistry(DI_SELECT_DI1);

            // Determine if DI1 LCD panel is primary or secondary
            // based on whether or not DI0 is connected (i.e., DI0 is primary by default)
            if (g_bDI0Connected)
            {
                // update the secondary mode resolution
                g_SecondaryMode.height    = g_pDI1PanelInfo->HEIGHT;
                g_SecondaryMode.width     = g_pDI1PanelInfo->WIDTH;
                g_SecondaryMode.modeId    = g_pDI1PanelInfo->MODEID;
                g_SecondaryMode.frequency = g_pDI1PanelInfo->FREQUENCY;
            }
            else
            {
                // update the primary mode resolution
                g_PrimaryMode.height    = g_pDI1PanelInfo->HEIGHT;
                g_PrimaryMode.width     = g_pDI1PanelInfo->WIDTH;
                g_PrimaryMode.modeId    = g_pDI1PanelInfo->MODEID;
                g_PrimaryMode.frequency = g_pDI1PanelInfo->FREQUENCY;
            }
        }

        // Enable panel if not already enabled
        if (g_pDI1PanelInfo->ISACTIVE == FALSE)
        {
            g_pDI1PanelInfo->ISACTIVE = TRUE;

            // Configure system for LCD mode, from DI1
            if(bSecondaryOn && g_pDI0PanelInfo->ISACTIVE)
            {
                // Function is misnamed - We could have 2 LCD panels connected
                // to DI0 and DI1, so we use this to enable them both.
                DisplayConfigureDual(DI_SELECT_DI1, DI_SELECT_DI0);
            }
            else
            {
                DisplayConfigureSingle(DI_SELECT_DI1); 
            }
            BSPEnablePanel(g_pDI1PanelInfo);
        }

    }
    else
    {
        // Error: We have no displays for this mode
    }

    // Update g_iCurrentMode with newly configured mode
    g_iCurrentMode = dispMode;

    //************************************************************************
    // Update variables controlling update loops for Async panels
    // and for secondary panels
    //************************************************************************
    if (((g_pDI0PanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
        && g_pDI0PanelInfo->ISACTIVE)
        || 
        ((g_pDI1PanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
        && g_pDI1PanelInfo->ISACTIVE))
    {
        g_bAsyncPanelActive = TRUE;
    }
    else
    {
        g_bAsyncPanelActive = FALSE;
    }
}


//------------------------------------------------------------------------------
//
// Function: DisplayAllocateBuffer
//
// This function calls to allocate an IpuBuffer for use by the driver.
//
// Parameters:
//      dwBufSize
//          [in] Size of buffer in bytes
//
//      pNewBuffer
//          [in] Pointer to allocated IpuBuffer object
//
// Returns:
//      None
//------------------------------------------------------------------------------
void DisplayAllocateBuffer(DWORD dwBufSize, IpuBuffer * pNewBuffer)
{
    IPUBufferInfo ipuBufInfo;

    // If first allocation, create IPU Base handle
    if (!g_hIPUBase)
    {
        // open handle to the IPU_BASE driver in order to enable IC module
        // First, create handle to IPU_BASE driver
        g_hIPUBase = IPUV3BaseOpenHandle();
        if (g_hIPUBase == INVALID_HANDLE_VALUE)
        {
            RETAILMSG(1,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
        }
    }

    ipuBufInfo.dwBufferSize = dwBufSize;
    ipuBufInfo.MemType = MEM_TYPE_VIDEOMEMORY;

    IPUV3AllocateBuffer(g_hIPUBase, &ipuBufInfo, pNewBuffer);

    return;
}


//------------------------------------------------------------------------------
//
// Function: DisplayDeallocateBuffer
//
// This function calls to delete an IpuBuffer object.  This must be done
// to ensure that memory allocated from the IPU Base process is properly
// freed.
//
// Parameters:
//      pIpuBuffer
//          [in] Pointer to existing IpuBuffer object
//
// Returns:
//      None
//------------------------------------------------------------------------------
void DisplayDeallocateBuffer(IpuBuffer * pIpuBuffer)
{
    // If handle does not exist (it should), create IPU Base handle
    if (!g_hIPUBase)
    {
        // open handle to the IPU_BASE driver
        // First, create handle to IPU_BASE driver
        g_hIPUBase = IPUV3BaseOpenHandle();
        if (g_hIPUBase == INVALID_HANDLE_VALUE)
        {
            RETAILMSG(1,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
        }
    }

    IPUV3DeallocateBuffer(g_hIPUBase, pIpuBuffer);

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisplaySetSrcBuffer
//
// This function sets a new source buffer for sending image data to
// the display.
//
// Parameters:
//
//      phAddr
//          [in] New source address
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplaySetSrcBuffer(UINT32 phAddr)
{
    RECT fullRect;
    INT8 iCurBuf;

    g_PrimaryBufPhysAddr = phAddr;

    fullRect.left = 0;
    fullRect.top = 0;
    if (g_pDI1PanelInfo->ISACTIVE == TRUE)
    {
        fullRect.bottom = g_SecondaryMode.height;
        fullRect.right = g_SecondaryMode.width;
    }
    else
    {
        fullRect.bottom = g_PrimaryMode.height;
        fullRect.right = g_PrimaryMode.width;
    }

    // If we have 2 active overlays, the input framebuffer for the DP flow
    // becomes the output from IC combining.  We set that up in Display_Middle_OverlayMainThread.
    if(!g_hMiddleOverlayUIUpdateEvent)
    {
        if (g_bAsyncPanelActive &&(g_pDI1PanelInfo->ISACTIVE == FALSE))
        {   
            //For the case: Only smart panel is enabled
            CPMEMWriteBuffer(IDMAC_CH_DC_SYNC_ASYNC_FLOW, 0, (UINT32 *)phAddr);
            CPMEMWriteBuffer(IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE, 0, (UINT32 *)phAddr);
        }
        else //sync panel
        {
            //Get current buffer status
            if(g_bDoubleBufferMode)
            {
                //Set iCurBuf to -1 to declare double buffer mode is using.
                iCurBuf = -1;
            }
            else
            {
                iCurBuf = IDMACChannelQueryTripleCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
            }

            //Fill buffer to right place according to current buffer status.
            if(iCurBuf == 0)
            {
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 1, (UINT32 *)phAddr);
                IDMACChannelBUF1SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
            }
            else if(iCurBuf == 1)
            {
                CPMEMWriteBuffer(IDMACChannelAltChIndex(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE), 0, (UINT32 *)phAddr);
                IDMACChannelBUF2SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);                
            }
            else if(iCurBuf == 2)
            {
                CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 0, (UINT32 *)phAddr);
                IDMACChannelBUF0SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);   
            }
            else
            {
                //Double buffer mode use another registry to check status. 
                if(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE))
                {
                    CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 0, (UINT32 *)phAddr);
                    IDMACChannelBUF0SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
                }
                else
                {
                    CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 1, (UINT32 *)phAddr);
                    IDMACChannelBUF1SetReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
                }                 
            }
        }
    }
    DisplayUpdate(&fullRect);
}

//------------------------------------------------------------------------------
//
// Function: DisplaySetSrcBuffer2
//
// This function sets a new source buffer for sending image data to
// the second display device.
//
// Parameters:
//
//      phAddr
//          [in] New source address
//
// Returns:
//      None.
//------------------------------------------------------------------------------

void DisplaySetSrcBuffer2(UINT32 phAddr)
{
    //we don't support secondary display device for async panel yet.
    if(g_bAsyncPanelActive)
        return;
    g_Primary2BufPhysAddr = phAddr;
    if(IDMACChannelCurrentBufIsBuf1(IDMAC_CH_DC_SYNC_ASYNC_FLOW))
    {
        CPMEMWriteBuffer(IDMAC_CH_DC_SYNC_ASYNC_FLOW, 0, (UINT32 *)phAddr);
        IDMACChannelBUF0SetReady(IDMAC_CH_DC_SYNC_ASYNC_FLOW);
    }
    else
    {
        CPMEMWriteBuffer(IDMAC_CH_DC_SYNC_ASYNC_FLOW, 1, (UINT32 *)phAddr);
        IDMACChannelBUF1SetReady(IDMAC_CH_DC_SYNC_ASYNC_FLOW);
    }
}


//------------------------------------------------------------------------------
//
// Function: DisplaySetScreenRotation
//
// Set screen rotation.
//
// Parameters:
//      dwRotation
//          [in] Rotation setting.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplaySetScreenRotation(DWORD dwRotation)
{
    BOOL oldSettingRotated, newSettingRotated;
    DWORD dwNewWidth, dwNewHeight;

    oldSettingRotated = ((g_iRotate == DMDO_90) || (g_iRotate == DMDO_270)) ? TRUE : FALSE;
    newSettingRotated = ((dwRotation == DMDO_90) || (dwRotation == DMDO_270)) ? TRUE : FALSE;

    // If rotation is changing from rotated (90, 270)
    // to non-rotated (0, 180), we must reconstruct
    // dirty rectangle objects.
    if (newSettingRotated != oldSettingRotated)
    {
        if (g_bAsyncPanelActive)
        {
            EnterCriticalSection(&g_csAsyncDirtyRect);

            DEBUGCHK(g_pAsyncDirtyRect != NULL);
            
            if(g_pAsyncDirtyRect)
                delete g_pAsyncDirtyRect;

            dwNewWidth = newSettingRotated ? g_PrimaryMode.height : g_PrimaryMode.width;
            dwNewHeight = newSettingRotated ? g_PrimaryMode.width : g_PrimaryMode.height;

            // Initialize Dirty Rectangle object
            g_pAsyncDirtyRect = new DirtyRect(dwNewWidth, dwNewHeight, MIN_DIRTY_RECT_PIX_DIMS, MIN_DIRTY_RECT_PIX_DIMS);

            // Signal to ADC update thread that we can now be udpated.
            SetEvent(g_hAsyncUpdateRequest);

            LeaveCriticalSection(&g_csAsyncDirtyRect);
        }

        // XEC DLS : Set the UI Event
        Display_SetUIEvent();
    }

    g_iRotate = dwRotation;
}

//------------------------------------------------------------------------------
//
// Function: DisplayUpdateInit
//
// Initialize structures needed for updating display.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplayUpdateInit(LPCRITICAL_SECTION pcsActiveSurfaceLock)
{
    InitializeCriticalSection(&g_csBufferLock);   
    
    // Initialize event to signal request to update the Async display
    g_hAsyncUpdateRequest = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_hAsyncUpdateRequest == NULL)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("%s: CreateEvent failed ")
                                  TEXT("for Async update event\r\n"),
                                  __WFUNCTION__));
    }
       
    // Initialize display update thread if any connected displays
    // use an asynchronous interface
    if((g_pDI0PanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS) 
        || (g_pDI1PanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS))
    {
        // Initialize Dirty Rectangle objects
        g_pAsyncDirtyRect = new DirtyRect(g_PrimaryMode.width, g_PrimaryMode.height, MIN_DIRTY_RECT_PIX_DIMS, MIN_DIRTY_RECT_PIX_DIMS);
        g_iNumAsyncDirtyRegions = g_PrimaryMode.width/MIN_DIRTY_RECT_PIX_DIMS * g_PrimaryMode.height/MIN_DIRTY_RECT_PIX_DIMS;

        InitializeCriticalSection(&g_csAsyncDirtyRect);

        // Initialize Display update thread
        //
        //   pThreadAttributes = NULL (must be NULL)
        //   dwStackSize = 0 => default stack size determined by linker
        //   lpStartAddress = DisplayAsyncUpdateThread => thread entry point
        //   lpParameter = pcsActiveSurfaceLock => point to Critical Section
        //   dwCreationFlags = 0 => no flags
        //   lpThreadId = NULL => thread ID is not returned
        //
        PREFAST_SUPPRESS(5451, "affects 64 bit windows only");
        g_hDisplayUpdateThread = ::CreateThread(NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE)DisplayAsyncUpdateThread,
                                            pcsActiveSurfaceLock,
                                            0,
                                            NULL);

        if (g_hDisplayUpdateThread == NULL)
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("%s: CreateThread failed!")
                                      TEXT("\r\n"), __WFUNCTION__)); 
        }
        else
        {
            DEBUGMSG(GPE_ZONE_INIT, (TEXT("%s: create Display update ")
                                     TEXT("thread success\r\n"),
                                     __WFUNCTION__));
            CeSetThreadPriority(g_hDisplayUpdateThread, 101);
        }
    }

}

//------------------------------------------------------------------------------
//
// Function: DisplayUpdate
//
// This function sets a new source buffer for sending image data to
// the display.
//
// Parameters:
//      pUpdateRct
//          [in] Region of UI that has been modified and must
//          be updated to the display.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplayUpdate(LPRECT pUpdateRct)
{
    if (g_bAsyncPanelActive)
    {
        EnterCriticalSection(&g_csAsyncDirtyRect);

        DEBUGCHK(g_pAsyncDirtyRect != NULL);
        if(g_pAsyncDirtyRect)
            g_pAsyncDirtyRect->SetDirtyRegion(pUpdateRct);

        // Signal to Async update thread that we can now be udpated.
        SetEvent(g_hAsyncUpdateRequest);

        LeaveCriticalSection(&g_csAsyncDirtyRect);
    }
    else
    {
        // Send signal to 2nd overlay update thread (if active)
        // when display framebuffer has changed.
        
        if(g_hMiddleOverlayUIUpdateEvent)
        {
            //The event can not be set if we are stopping the overlay flow.
            EnterCriticalSection(&g_csBufferLock);
            SetEvent(g_hMiddleOverlayUIUpdateEvent);
            LeaveCriticalSection(&g_csBufferLock);
        }
#if DEBUG_DUMP_IPU_REGS
//        DumpIPURegs();
#endif
    }

    // XEC DLS : Set the UI Event
    Display_SetUIEvent();
}


//------------------------------------------------------------------------------
//
// Function: DisplayAsyncUpdateThread
//
// This method is a thread executed whenever the ASync is active.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplayAsyncUpdateThread(LPVOID lpParameter)
{
    UINT32 time, interval, startXYAddr, srcBufOffset, startPhysAddr;
    RECT curUpdateRect;
    DI_SELECT di_sel_async;

    di_sel_async = DI_SELECT_DI0;

    // Get DDIPU critical section to lock when drawing to screen
    LPCRITICAL_SECTION pcsDrawLock = (LPCRITICAL_SECTION)lpParameter;

    for (;;)
    {
        // Wait for the next UI update request
        WaitForSingleObject(g_hAsyncUpdateRequest, INFINITE);

        time = GetTickCount();
        DEBUGMSG(GPE_ZONE_PERF, (TEXT("%s: Screen Update request received at time %d:%d\r\n"), __WFUNCTION__, time/60, time%60));

        // Check to make sure that display ports are enabled, 
        // or else we will be unable to update Async display.
        if (g_bPowerOff)
        {
            // Reset event to re-synchronize updates to the display surface
            // with calls to update the Asynchronous display.
            ResetEvent(g_hAsyncUpdateRequest);
            continue;
        }

        EnterCriticalSection(&g_csBufferLock);

        if(g_dwCurBGChan == IDMAC_CH_28)
        {
            EnterCriticalSection(pcsDrawLock);

            // Enable Async flow in order to configure registers and perform update.
            DisplayEnableSingleFlow(DISPLAY_FLOW_ASYNC_UI);

            // Added to allow subsequent update to Write Channel Address
            // to propagate and allow correct X,Y start address to
            // be written out to the async panel.

            //********************************************************************
            // Disable Write Channel 1 (main channel to write pixels to display)
            //********************************************************************
            BSPDisableDCChannels(IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS);

            IDMACChannelDisable(g_dwCurBGChan);

            // Compute the smallest rectangle containing all regions
            // that need to be updated.
            GetUpdateRegion(&curUpdateRect);

            // Update Start Address register
            // Bits 16:8 are Y position, Bits 7:0 are X position
            startXYAddr = (curUpdateRect.top << 8) | curUpdateRect.left;

            // Update address register based on desired update region
            DCSetWriteChannelAddress(DC_CHANNEL_DC_SYNC_OR_ASYNC, startXYAddr);

            // Update CPMEM settings with new width/height/stride data
            CPMEMSetup(g_dwCurBGChan,                                 // IDMAC Channel to configure
                        curUpdateRect.right - curUpdateRect.left + 1, // rectangle width
                        curUpdateRect.bottom - curUpdateRect.top + 1, // rectangle height
                        g_PrimaryMode.Bpp,                            // UI bpp
                        g_PrimaryBufStride,                           // UI surface stride
                        FALSE);                                       // non-interleaved data

            IDMACChannelEnable(g_dwCurBGChan);

            DMFCSetup(g_dwCurBGChan, DI_SELECT_DI0, curUpdateRect.right - curUpdateRect.left + 1);
        
            // Added to allow subsequent update to Write Channel Address
            // to propagate and allow correct X,Y start address to
            // be written out to the async panel.
            BSPEnableDCChannels(IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS);

            // Compute buffer offset from source rectangle.
            srcBufOffset = (curUpdateRect.top * g_PrimaryMode.width + curUpdateRect.left) * g_PrimaryMode.Bpp / 8;
            startPhysAddr = g_PrimaryBufPhysAddr + srcBufOffset;
            CPMEMWriteBuffer(g_dwCurBGChan, 0, (UINT32 *)startPhysAddr);

            // This is the final step to initiate the update task
            IDMACChannelBUF0SetReady(g_dwCurBGChan);

            // Wait for update to complete before potentially starting a new one
            WaitForEOF(g_dwCurBGChan);
            LeaveCriticalSection(pcsDrawLock);

            // Update is complete.  Now we can shut down IPU modules (if they 
            // are not needed by other flows).
            DisplayDisableSingleFlow(DISPLAY_FLOW_ASYNC_UI);
        }
        //if the foreground is update, async0 background should be wait
        else if(g_dwCurBGChan == IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE)
        {
            //Workaround: Set DP MAIN PLANE ready will cause aux plane double buffer switch automatically.
            //It will cause video jitter seriously.
            //make sure these code won't be run while overlay surface is keeping on updating.
            if((!DPIsBusy(DP_CHANNEL_ASYNC0))&&(!PrPIsBusy(g_hPrp)))
            {
                if(g_bOverlayUpdating)
                {
                    g_bOverlayUpdating = FALSE;
                    LeaveCriticalSection(&g_csBufferLock);
                    Sleep(250);
                    EnterCriticalSection(&g_csBufferLock);
                }
                if((g_dwCurBGChan == IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE)&&
                    (!DPIsBusy(DP_CHANNEL_ASYNC0))&&
                    (!PrPIsBusy(g_hPrp))&&
                    (!g_bOverlayUpdating))
                {
                    g_bUIUpdated = TRUE;

                    CPMEMSwapBuffer(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE,FALSE);    
                    IDMACChannelBUF0SetReady(g_dwCurBGChan);

                    while(DPIsBusy(DP_CHANNEL_ASYNC0))
                    {Sleep(1);}

                    CPMEMSwapBuffer(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE,FALSE);
                    IDMACChannelBUF0SetReady(g_dwCurBGChan);
                }
                
            }
        }
    
        LeaveCriticalSection(&g_csBufferLock);
        

        // We don't want to update the screen any faster than 30FPS, so we sleep
        // to maintain a maximum of a 30FPS refresh rate.

        // Sleep such that we wait at least 30ms between updates (~30FPS).
        interval = GetTickCount() - time;
        DEBUGMSG(GPE_ZONE_PERF, (TEXT("%s: Interval since last update: %d\r\n"), __WFUNCTION__, interval));
        if (interval < 30)
        {
            Sleep(30 - interval);
        }
    }

    return;
}


//------------------------------------------------------------------------------
//
// Function: WaitForEOF
//
// Return once the specified channel receives an EOF interrupt.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
static void WaitForEOF(DWORD dwIDMACChan)
{
    HANDLE hEOFHandle;

    switch (dwIDMACChan)
    {
        case IDMAC_CH_28:
            hEOFHandle = g_hDCCh1IntrEvent;
            break;
        case IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE:
            hEOFHandle = g_hDPBGIntrEvent;
            break;            
        default:
            hEOFHandle = g_hDCCh1IntrEvent;
            break;
    }

    // Enable and wait for an EOF interrupt
    IDMACChannelIntCntrl(dwIDMACChan, IPU_INTR_TYPE_EOF, TRUE);

    if (WaitForSingleObject(hEOFHandle, 4000) == WAIT_TIMEOUT)
    {
        DEBUGMSG(1, (TEXT("%s(): Waiting for Channel %d EOF interrupt time out!\r\n"), __WFUNCTION__, dwIDMACChan));
    }

    IDMACChannelClearIntStatus(dwIDMACChan, IPU_INTR_TYPE_EOF);
}


//------------------------------------------------------------------------------
//
// Function: WaitForDPSyncFlowEnd
//
// Return once the DP Sync Flow end event occurs.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
static void WaitForDPSyncFlowEnd()
{
    // If either 
    //   a) BUF0_RDY and BUF1_RDY are not clear
    //     OR
    //   b) IDMAC channel is not busy
    // Then flow is busy and we must wait for end.
    if (IDMACChannelBUF0IsReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE) || 
        IDMACChannelBUF1IsReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE) ||
        IDMACChannelBUF2IsReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE) ||
        IDMACChannelIsBusy(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE))
    {
        // Clear status before waiting for interrupt
        CMDPFlowClearIntStatus(DP_INTR_TYPE_SF_END);

        // Enable and wait for a DP_SF_END interrupt
        CMDPFlowIntCntrl(DP_INTR_TYPE_SF_END, TRUE);

        if (WaitForSingleObject(g_hDPFlowEvent, 4000) == WAIT_TIMEOUT)
        {
            DEBUGMSG(1, (TEXT("%s(): Waiting for DP_SF_END interrupt time out!\r\n"), __WFUNCTION__));
        }

        CMDPFlowClearIntStatus(DP_INTR_TYPE_SF_END);
    }
    else
    {
        // 20ms ~ full refresh period larger than 50Hz
        Sleep(20);
    }
}


//------------------------------------------------------------------------------
//
// Function: WaitForDPAsyncFlowEnd
//
// Return once the DP Async Flow end event occurs.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
static void WaitForDPAsyncFlowEnd()
{
    // If either 
    //   a) BUF0_RDY and BUF1_RDY are not clear
    //     OR
    //   b) IDMAC channel is not busy
    // Then flow is busy and we must wait for end.
    if (IDMACChannelBUF0IsReady(IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE) || 
        IDMACChannelBUF1IsReady(IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE) ||
        IDMACChannelIsBusy(IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE))
    {
        // Clear status before waiting for interrupt
        CMDPFlowClearIntStatus(DP_INTR_TYPE_ASF_END);

        // Enable and wait for a DP_ASF_END interrupt
        CMDPFlowIntCntrl(DP_INTR_TYPE_ASF_END, TRUE);

        if (WaitForSingleObject(g_hDPFlowEvent, 4000) == WAIT_TIMEOUT)
        {
            DEBUGMSG(1, (TEXT("%s(): Waiting for DP_ASF_END interrupt time out!\r\n"), __WFUNCTION__));
        }

        CMDPFlowClearIntStatus(DP_INTR_TYPE_ASF_END);
    }
    else
    {
        // 20ms ~ full refresh period larger than 50Hz
        Sleep(20);
    }
}


//------------------------------------------------------------------------------
//
// Function: WaitForDCFrameComplete
//
// Return once the DP Async Flow end event occurs.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
static void WaitForDCFrameComplete()
{
    //**********************************************************************
    // IPU Designers recommend the following sequence:
    //  - IF flow is active, wait for end by using DC_FC_1 interrupt
    //  - ELSE flow is currently inactive, wait for a full refresh period, 
    //      in case data tail is active in DC and DI (no means available
    //      for checking status of data tail).
    //**********************************************************************

    // If either 
    //   a) BUF0_RDY and BUF1_RDY are not clear
    //     OR
    //   b) IDMAC channel is not busy
    // Then flow is busy and we must wait for end.
    if (IDMACChannelBUF0IsReady(IDMAC_CH_DC_SYNC_ASYNC_FLOW) || 
        IDMACChannelBUF1IsReady(IDMAC_CH_DC_SYNC_ASYNC_FLOW) ||
        IDMACChannelIsBusy(IDMAC_CH_DC_SYNC_ASYNC_FLOW))
    {
        // Clear status before waiting for interrupt
        CMDCFrameCompleteClearIntStatus(DC_CHANNEL_DC_SYNC_OR_ASYNC);

        // Enable and wait for a DC_FC_1 interrupt
        CMDCFrameCompleteIntCntrl(DC_CHANNEL_DC_SYNC_OR_ASYNC, TRUE);

        if (WaitForSingleObject(g_hDCFrameCompleteEvent, 4000) == WAIT_TIMEOUT)
        {
            DEBUGMSG(1, (TEXT("%s(): Waiting for DC_FC interrupt time out!\r\n"), __WFUNCTION__));
        }

        CMDCFrameCompleteClearIntStatus(DC_CHANNEL_DC_SYNC_OR_ASYNC);
    }
    else
    {
        // 20ms ~ full refresh period larger than 50Hz
        Sleep(20);
    }
}



//------------------------------------------------------------------------------
//
// Function: DisplayWaitForVSync
//
// Return once a VSync signal has been received.
//
// After adding the secondary display device's code, we need to check which vsync signal 
// needs to wait. If we need to wait for secondary display device's vsync, we just need to 
// check the panel connected to DI0 is sync panel (in current implementation, secondary display 
// device always is LCD which connect to DI0). If we need to wait for primary display device's 
// vsync, we may need to wait for TV's vsync(DI1) or dumb LCD's vsync(DI0). But when DI1 is 
// disabled (TV is off) and DI0's panel is async panel, we needn't wait for vsync.
//
// Parameters:
//      bIsSecondarydevice
//          [in] Will be set when need to wait secondary display device's vsync. 
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplayWaitForVSync(BOOL bIsSecondarydevice)
{
    if(bIsSecondarydevice)
    {
        //For async panel, we can't wait
        if(g_pDI0PanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
            return;
        //The secondary device is always DC channel.
        WaitForEOF(DC_CHANNEL_DC_SYNC_OR_ASYNC);
    }
    else
    {
        //Need check DI0 panel type when only DI0 is enable
        if((!g_pDI1PanelInfo->ISACTIVE)&&
            (g_pDI0PanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS))
            return;
        WaitForEOF(g_dwCurBGChan);
    }
}


//------------------------------------------------------------------------------
//
// Function: DCSetup
//
// This function initializes the DC based on the display properties.
//
// Parameters:
//      di_sel
//          [in] Selects DI0 or DI1 for setup.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
static void DCSetup(DI_SELECT di_sel)
{
    PANEL_INFO *pDIPanelInfo;

    if (di_sel == DI_SELECT_DI0)
    {
        pDIPanelInfo = g_pDI0PanelInfo;
    }
    else
    {
        pDIPanelInfo = g_pDI1PanelInfo;
    }

    BSPInitializeDC(di_sel, pDIPanelInfo);
}


//------------------------------------------------------------------------------
//
// Function: DISetup
//
// This function initializes the DI based on the display properties.
//
// Parameters:
//      di_sel
//          [in] Selects DI0 or DI1 for setup.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
static void DISetup(DI_SELECT di_sel)
{
    PANEL_INFO *pDIPanelInfo;

    if (di_sel == DI_SELECT_DI0)
    {
        pDIPanelInfo = g_pDI0PanelInfo;
    }
    else
    {
        pDIPanelInfo = g_pDI1PanelInfo;
    }

    // Retrieve HSP clock frequency value
    g_IPUClk = CSPIPUGetClk();
    DEBUGMSG (1, (TEXT("%s: IPU HSP clk = 0x%x\r\n"), __WFUNCTION__, g_IPUClk));

    DISetIPUClkFreq(g_IPUClk);
    DISetDIClkFreq(di_sel,g_IPUClk);

    BSPInitializeDI(di_sel, pDIPanelInfo);

}

//------------------------------------------------------------------------------
//
// Function: CPMEMSetup
//
// This function initializes the CPMEM based on the display properties.
//
// Parameters:
//      dwIDMACChan
//          [in] Selects IDMAC channel to configure
//
//      width
//          [in] Width of the frame.
//
//      height
//          [in] Height of the frame.
//
//      bpp
//          [in] Bits per pixel for the frame pixel data.
//
//      stride
//          [in] Stride value for the frame surface.
//
//      bInterlaced
//          [in] If TRUE, send frame as Interlaced data; If FALSE, 
//          frame is Progressive format.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
static void CPMEMSetup(DWORD dwIDMACChan, int width, int height, int bpp, int stride, BOOL bInterlaced)
{
    CPMEMConfigData CPMEMData;
    CPMEMBufOffsets OffsetData;
    
    //********************************
    // Special Mode parameters
    //********************************

    CPMEMData.iAccessDimension = CPMEM_DIM_2D;
    CPMEMData.iScanOrder = bInterlaced ? CPMEM_SO_INTERLACED : CPMEM_SO_PROGRESSIVE;
    CPMEMData.iBandMode = CPMEM_BNDM_DISABLE;
    CPMEMData.iBlockMode = CPMEM_BM_DISABLE;
    CPMEMData.iThresholdEnable = CPMEM_THE_DISABLE;
    CPMEMData.iCondAccessEnable = CPMEM_CAE_COND_SKIP_DISABLE;

    if((dwIDMACChan == IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)
      ||(dwIDMACChan == IDMAC_CH_DC_SYNC_ASYNC_FLOW))
        CPMEMData.iAXI_Id = CPMEM_ID_0; //ID_0 has high priority to access memory, only for sync display channel
    else
        CPMEMData.iAXI_Id = CPMEM_ID_1; 

    //********************************
    // Don't care parameters
    //********************************
    CPMEMData.iDecAddrSelect = CPMEM_DEC_SEL_ADDR_0_TO_15; // Don't care, only for 4BPP mode
    CPMEMData.iCondAccessPolarity = CPMEM_CAP_COND_SKIP_LOW;
    CPMEMData.iAlphaUsed = CPMEM_ALU_ALPHA_SAME_CHANNEL;

    //********************************
    // Flipping/Rotation parameters
    //********************************
    CPMEMData.iRotation90 = CPMEM_ROT_DISABLE;
    CPMEMData.iFlipHoriz = CPMEM_HF_DISABLE;
    CPMEMData.iFlipVert = CPMEM_VF_DISABLE;

    //********************************
    // Format and dimensions
    //********************************
    CPMEMData.iPixelFormat = CPMEM_PFS_INTERLEAVED_RGB;
    CPMEMData.iHeight = (UINT16)height - 1;
    CPMEMData.iWidth = (UINT16)width - 1;

    if( bInterlaced)
    {   
        CPMEMData.iLineStride = stride * 2 -1; 
        
        OffsetData.interlaceOffset = stride;
        CPMEMWriteOffset(dwIDMACChan, &OffsetData, TRUE);  // TRUE: interleaved RGB
    }
    else
    {
        CPMEMData.iLineStride = stride - 1;
        OffsetData.interlaceOffset = 0;
        CPMEMWriteOffset(dwIDMACChan, &OffsetData, TRUE);  // TRUE: interleaved RGB    
    }    

    switch (bpp)
    {
        case 4:
            CPMEMData.iBitsPerPixel = CPMEM_BPP_4;
            CPMEMData.iPixelBurst = CPMEM_PIXEL_BURST_16;
            CPMEMData.pixelFormatData.component0_width = 0;
            CPMEMData.pixelFormatData.component1_width = 0;
            CPMEMData.pixelFormatData.component2_width = 0;
            CPMEMData.pixelFormatData.component3_width = 0;
            CPMEMData.pixelFormatData.component0_offset = 4-1;
            CPMEMData.pixelFormatData.component1_offset = 0;
            CPMEMData.pixelFormatData.component2_offset = 0;
            CPMEMData.pixelFormatData.component3_offset = 0;
            CPMEMData.iDecAddrSelect = CPMEM_DEC_SEL_ADDR_0_TO_15;
            break;
        case 8:
            CPMEMData.iBitsPerPixel = CPMEM_BPP_8;
            CPMEMData.iPixelBurst = CPMEM_PIXEL_BURST_16;
            CPMEMData.pixelFormatData.component0_width = 0;
            CPMEMData.pixelFormatData.component1_width = 0;
            CPMEMData.pixelFormatData.component2_width = 0;
            CPMEMData.pixelFormatData.component3_width = 0;
            CPMEMData.pixelFormatData.component0_offset = 8-1;
            CPMEMData.pixelFormatData.component1_offset = 0;
            CPMEMData.pixelFormatData.component2_offset = 0;
            CPMEMData.pixelFormatData.component3_offset = 0;
            break;
        case 16:
            CPMEMData.iBitsPerPixel = CPMEM_BPP_16;
#if USE_LARGE_BURST_SIZE_BG
            CPMEMData.iPixelBurst = CPMEM_PIXEL_BURST_32; 
#else
            CPMEMData.iPixelBurst = CPMEM_PIXEL_BURST_16;
#endif
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
#if USE_LARGE_BURST_SIZE_BG
            CPMEMData.iPixelBurst = CPMEM_PIXEL_BURST_20; 
#else
            CPMEMData.iPixelBurst = CPMEM_PIXEL_BURST_16;
#endif
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
    CPMEMWrite(dwIDMACChan, &CPMEMData, TRUE);
}

//------------------------------------------------------------------------------
//
// Function: DMFCInitialization
//
// This function initialized all DMFC channel .
//
// Parameters:
//      None.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
static void DMFCInitialize()
{
    dmfcConfigData dmfcData;

    //Initialize all DMFC channels.
    //The burst size of all channel is 8x128bit
    //The FIFO size of all channel is 32X128 words
    dmfcData.PixelPerWord = IPU_DMFC_PPW_C_24BPP; // Only care for DC Read Ch or IC->DMFC path
    dmfcData.FrameWidth = 640;
    dmfcData.FrameHeight = 480;
    dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_32x128;
    dmfcData.BurstSize = IPU_DMFC_BURST_SIZE_0_4_WORDS;
    dmfcData.WaterMarkEnable = IPU_DMFC_WM_EN_DISABLE;
    dmfcData.WaterMarkClear = 1; // Keep it the same as default setting.
    dmfcData.WaterMarkSet = 0; // Don't care if Watermark disabled
    dmfcData.DestChannel = IDMAC_INVALID_CHANNEL; // Don't care unless using IC->DMFC path

    //channel 2, 5b, 6b, 2c  0~127  seg 0
    dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_0;
    DMFCConfigure(IDMAC_CH_DC_COMMAND_STREAM2, &dmfcData);  // 2c
    DMFCConfigure(IDMAC_CH_DC_ASYNC_FLOW, &dmfcData);  // 2
    DMFCConfigure(IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE, &dmfcData);  // 6b
    DMFCConfigure(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, &dmfcData);  // 5b
    

    //channel 1, 1c           128~255  seg 2
    dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_2;
    DMFCConfigure(IDMAC_CH_DC_COMMAND_STREAM1, &dmfcData);  // 1c
    DMFCConfigure(IDMAC_CH_DC_SYNC_ASYNC_FLOW, &dmfcData);  // 1

    //channel 5f                256~383   seg 4
    dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_4;
    DMFCConfigure(IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE, &dmfcData);  // 5f
    
    //channel 6f                384~511   seg 6
    dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_6;
    DMFCConfigure(IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE, &dmfcData);  // 6f

    //internal FIFO channel
    DMFCConfigure(IDMAC_CH_DC_READ, &dmfcData);  // 0
    DMFCConfigure(IDMAC_CH_DC_OUTPUT_MASK, &dmfcData);  // 9

    return;
}


//------------------------------------------------------------------------------
//
// Function: DMFCSetup
//
// This function initializes the DMFC based on the display properties.
//
// Parameters:
//      dwIDMACChan
//          [in] Selects IDMAC channel to configure in DMFC.
//
//      di_sel
//          [in] Selects DI0 or DI1 for setup.
//
//      dwWidth
//          [in] Frame width coming from IDMAC
//
//      bpp
//          [in] Bits per pixel for the frame pixel data.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
static void DMFCSetup(DWORD dwIDMACChan, DI_SELECT di_sel, DWORD dwWidth)
{
    dmfcConfigData dmfcData;
    UINT32 iScale;

    PANEL_INFO *pDIPanelInfo;

    if (di_sel == DI_SELECT_DI0)
    {
        pDIPanelInfo = g_pDI0PanelInfo;
    }
    else
    {
        pDIPanelInfo = g_pDI1PanelInfo;
    }

    //The muximum number of EOL indications of EOL is more than 1 for three IDMAC channel
    //here we use iScale parameter to declare it.
    //2010.4.26 Correct the scale setting from Mark's comments 
    if((dwIDMACChan == IDMAC_CH_DC_SYNC_ASYNC_FLOW)
     ||(dwIDMACChan == IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE))
    {
        iScale = 2;
    }
    else if(dwIDMACChan == IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE)
    {
        iScale = 3;
    }
    else
    {
        iScale = 1;
    }
    // There is a required relationship between the incoming frame's
    // width, and the FIFO size that must be configured for that frame
    if ((dwWidth * 32 * iScale) < (4 * 128))
    {
        DEBUGMSG(1, (TEXT("%s: IDMAC frame width is too small! DMFC FIFO size must be smaller than frame width * 2.\r\n"), __WFUNCTION__));
        // Use smallest setting and hope that it works.
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_4x128;
    }
    else if ((dwWidth * 32 * iScale) < (8 * 128))
    {
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_4x128;
    }
    else if ((dwWidth * 32 * iScale) < (16 * 128))
    {
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_8x128;
    }
    else if ((dwWidth * 32 * iScale) < (32 * 128))
    {
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_16x128;
    }
    else if ((dwWidth * 32 * iScale) < (64 * 128))
    {
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_32x128;
    }
    else if ((dwWidth * 32 * iScale) < (128 * 128))
    {
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_64x128;
    }
    else if (dwWidth > 1024)
    {
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_256x128;
    }
    else
    {
        dmfcData.FIFOSize = IPU_DMFC_CHAN_DMFC_FIFO_SIZE_128x128;    
    }

    dmfcData.FrameWidth = (UINT16)pDIPanelInfo->WIDTH;
    dmfcData.FrameHeight = (UINT16)pDIPanelInfo->HEIGHT;
#if USE_LARGE_BURST_SIZE_BG
    dmfcData.BurstSize = IPU_DMFC_BURST_SIZE_0_32_WORDS; 
#else
    dmfcData.BurstSize = IPU_DMFC_BURST_SIZE_0_4_WORDS;
#endif
    if(dwIDMACChan == IDMAC_CH_DC_SYNC_ASYNC_FLOW)
        dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_2;
    else
        dmfcData.StartSeg = IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_0;
    if((dwIDMACChan == IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)
        ||(dwIDMACChan == IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE))
    {
        dmfcData.WaterMarkEnable = IPU_DMFC_WM_EN_ENABLE;
        IDMACWaterMark(dwIDMACChan, TRUE);
    }
    else
        dmfcData.WaterMarkEnable = IPU_DMFC_WM_EN_DISABLE;

    //Only useful when watermarkenable is set.
    dmfcData.WaterMarkClear = 7; //When number of bursts in FIFO equal or less than 7 the watermark will clear. (decrease)
    dmfcData.WaterMarkSet = 5; //When number of bursts in FIFO equal or more than 5 the watermark will set.(increase) 
    // Don't care values
    dmfcData.DestChannel = 0; // Don't care unless using IC->DMFC path
    dmfcData.PixelPerWord = IPU_DMFC_PPW_C_8BPP; // Only care for DC Read Ch or IC->DMFC path

    if (!DMFCConfigure(dwIDMACChan, &dmfcData))
    {
        ERRORMSG (1, (TEXT("%s: Failed to enable IDMAC Regs!\r\n"), __WFUNCTION__));
    }
}


static void GetUpdateRegion(RECT *pRct)
{
    UINT32 iDirtyRectRegions;
    LPRECT pRectangles;
    LONG temp1, temp2;

    EnterCriticalSection(&g_csAsyncDirtyRect);

    iDirtyRectRegions = g_pAsyncDirtyRect->GetNumDirtyRegions();

    // If a) no dirty regions OR
    //    b) all regions dirty
    // We update the entire UI
    if ((iDirtyRectRegions == 0) || (iDirtyRectRegions == g_iNumAsyncDirtyRegions))
    {
        pRct->left = 0;
        pRct->top = 0;
        if((g_iRotate == DMDO_90)||(g_iRotate == DMDO_270))
        {
            pRct->right = g_PrimaryMode.height - 1;
            pRct->bottom = g_PrimaryMode.width - 1;
        }
        else
        {
            pRct->right = g_PrimaryMode.width - 1;
            pRct->bottom = g_PrimaryMode.height - 1;           
        }

        // If all regions dirty, we must call GetDirtyRegions to clear dirty regions
        if (iDirtyRectRegions == g_iNumAsyncDirtyRegions)
        {
            pRectangles = (LPRECT) LocalAlloc(LPTR, sizeof(RECT) * iDirtyRectRegions);
            if (pRectangles != NULL)
            {
                g_pAsyncDirtyRect->GetDirtyRegions(pRectangles, iDirtyRectRegions);
                LocalFree(pRectangles);
            }
            else
            {
                ERRORMSG(TRUE, (TEXT("%s: LocalAlloc() failed for pRectangles\r\n"),
                                __WFUNCTION__));
            }
        }
        LeaveCriticalSection(&g_csAsyncDirtyRect);
        return;
    }

    pRectangles = (LPRECT) LocalAlloc(LPTR, sizeof(RECT) * iDirtyRectRegions);

    if (pRectangles != NULL)
    {
        g_pAsyncDirtyRect->GetDirtyRegions(pRectangles, iDirtyRectRegions);

        // Reset event to re-synchronize updates to the display surface
        // with calls to update the ASync.
        ResetEvent(g_hAsyncUpdateRequest);

        LeaveCriticalSection(&g_csAsyncDirtyRect);

        // If some regions dirty, request the smallest rectangle
        // that encompasses all of these regions.  Note that this
        // includes the rectangle for the combined video region.
        g_pAsyncDirtyRect->GetFullDirtyRect(pRectangles, iDirtyRectRegions, pRct);

        // Convert from rectangle defined in rotated perspective to
        // rectangle from display panel perspective.
        switch (g_iRotate)
        {
            case DMDO_0:
                // No changes necessary...already in panel perspective
                break;
            case DMDO_90:
                temp1 = pRct->right;
                temp2 = pRct->left;
                pRct->left = pRct->top;
                pRct->right = pRct->bottom;
                pRct->top = g_PrimaryMode.height - 1  - temp1;
                pRct->bottom = g_PrimaryMode.height - 1 - temp2;
                break;
            case DMDO_270:
                temp1 = pRct->top;
                temp2 = pRct->bottom;
                pRct->top = pRct->left;
                pRct->bottom = pRct->right;
                pRct->left = g_PrimaryMode.width - 1  - temp2;
                pRct->right = g_PrimaryMode.width - 1 - temp1;
                break;
            case DMDO_180:
                temp1 = pRct->top;
                pRct->top = g_PrimaryMode.height - 1 - pRct->bottom;
                pRct->bottom = g_PrimaryMode.height - 1 - temp1;
                temp2 = pRct->left;
                pRct->left = g_PrimaryMode.width - 1 - pRct->right;
                pRct->right = g_PrimaryMode.width - 1 - temp2;
                break;
        }

        LocalFree(pRectangles);
    }
    else
    {
        ERRORMSG(TRUE, (TEXT("%s: LocalAlloc() failed for pRectangles\r\n"),
                        __WFUNCTION__));

        LeaveCriticalSection(&g_csAsyncDirtyRect);
    }

    return;
}


//------------------------------------------------------------------------------
//
// Function: DisplayConfigureSingle
//
// This function configures IPU flow settings for the single LCD
// display mode.
//
//
// Parameters:
//      di_sel
//          [in] Selects DI0 or DI1 to configure for single LCD mode
//
// Returns:
//      None
//------------------------------------------------------------------------------
static void DisplayConfigureSingle(DI_SELECT di_sel)
{
    INT32 bpp;
    PANEL_INFO *pDIPanelInfo;
    GPEMode *pActiveMode;

    // Choose the PANEL_INFO to use
    if (di_sel == DI_SELECT_DI0)
    {
        pDIPanelInfo = g_pDI0PanelInfo;
    }
    else
    {
        pDIPanelInfo = g_pDI1PanelInfo;
    }

    // Choose the mode info to use
    if (pDIPanelInfo->MODEID == (DWORD)g_SecondaryMode.modeId)
    {
        pActiveMode = &g_SecondaryMode;
    }
    else
    {
        pActiveMode = &g_PrimaryMode;
    }

    bpp = pActiveMode->Bpp;

    // Call to turn on IPU clocks for register initializations
    IPUV3EnableClocks(g_hIPUBase);

    // Set up IPUv3 DC
    DCSetup(di_sel);

    // Set up IPUv3 DI0
    DISetup(di_sel);

    if(pDIPanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS)
    {
        //************************************************************************
        // Configure Channel 28 for UI-only Async display operation
        //************************************************************************

        // Set up CPMEM
        CPMEMSetup(IDMAC_CH_28, pDIPanelInfo->WIDTH, pDIPanelInfo->HEIGHT, bpp, g_PrimaryBufStride, FALSE);

        // Set up IPUv3 DMFC
        DMFCSetup(IDMAC_CH_28, di_sel, pActiveMode->width);

        // Set high priority for display update channel
        IDMACChannelSetPriority(IDMAC_CH_28, IDMAC_CH_PRIORITY_HIGH);

        // Enable flow, which enables necessary IPU modules
        DisplayEnableSingleFlow(DISPLAY_FLOW_ASYNC_UI);

        // Call to turn off IPU clocks - should not actually disable IPU clocks, 
        // since previous call to enable display flows turns on IPU submodules and thus IPU clocks.
        IPUV3DisableClocks(g_hIPUBase);

        BSPInitializePanel(pDIPanelInfo);

        CMSetDispFlow(IPU_DISP_FLOW_DC1_SRC, 0 /* MCU is source */);

        //************************************************************************
        // Configure Channel 23 for UI+Video Async display operation
        //************************************************************************

        // DP main channel setup for async0
        // Set up CPMEM
        CPMEMSetup(IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE, pDIPanelInfo->WIDTH, pDIPanelInfo->HEIGHT, bpp, g_PrimaryBufStride, FALSE);

        // Set up IPUv3 DMFC
        DMFCSetup(IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE, di_sel, pActiveMode->width);

        // Set high priority for display update channel
        IDMACChannelSetPriority(IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE, IDMAC_CH_PRIORITY_HIGH);

        CMSetDispFlow(IPU_DISP_FLOW_DP_ASYNC1_SRC, 0 /* MCU is source */);

        // Start mode in UI-only operation...we may switch to UI+Video if overlays
        // are later activated.
        g_dwCurBGChan = IDMAC_CH_28;
        IDMACChannelEnable(g_dwCurBGChan);
    }
    else if(pDIPanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_SYNCHRONOUS)
    {
        //************************************************************************
        // Configure Channel 23 for Sync display operation (both UI-only and UI+Video)
        //************************************************************************

        // Set up CPMEM
        CPMEMSetup(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, pDIPanelInfo->WIDTH, pDIPanelInfo->HEIGHT, bpp, g_PrimaryBufStride, pDIPanelInfo->ISINTERLACE);
        CPMEMSetup(IDMACChannelAltChIndex(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE), pDIPanelInfo->WIDTH, pDIPanelInfo->HEIGHT, bpp, g_PrimaryBufStride, pDIPanelInfo->ISINTERLACE);

        // Set up IPUv3 DMFC
        DMFCSetup(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, di_sel, pActiveMode->width);

        // Set high priority for display update channel
        IDMACChannelSetPriority(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, IDMAC_CH_PRIORITY_HIGH);

        dpCSCConfigData DpCSCConfigData;
        DP_CH_TYPE DpChannel = DP_CHANNEL_SYNC;

        // Set up DP CSC configuration based on output pixel format
        if(pDIPanelInfo->PIXEL_FMT == IPU_PIX_FMT_YUV422)
        {
            // CSC from RGB to YUV422
            DPCSCGetCoeffs(DpChannel, &DpCSCConfigData);
            if (DpCSCConfigData.CSCPosition == DP_CSC_DISABLE)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_BG;
                DpCSCConfigData.CSCEquation = CSCR2Y_A1;  
            
                DpCSCConfigData.bGamutEnable = FALSE;
                DPCSCConfigure(DpChannel, &DpCSCConfigData);
            }
            else if (DpCSCConfigData.CSCPosition == DP_CSC_FG)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_BOTH;
                DPCSCConfigure(DpChannel, &DpCSCConfigData);
            }
        }
        else
        {
            // Ensure that DP CSC configuration is set for no BG conversion
            // (previous settings may convert from RGB->YUV)
            DPCSCGetCoeffs(DpChannel, &DpCSCConfigData);
            if (DpCSCConfigData.CSCPosition == DP_CSC_BOTH)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_FG;
                DpCSCConfigData.CSCEquation = CSCY2R_A1;  
            
                DpCSCConfigData.bGamutEnable = FALSE;
                DPCSCConfigure(DpChannel, &DpCSCConfigData);
            }
            else if (DpCSCConfigData.CSCPosition == DP_CSC_BG)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_DISABLE;
                DpCSCConfigData.CSCEquation = CSCY2R_A1;  
            
                DpCSCConfigData.bGamutEnable = FALSE;
                DPCSCConfigure(DpChannel, &DpCSCConfigData);
            }
        }

        // Enable flow, which enables necessary IPU modules
        DisplayEnableSingleFlow(DISPLAY_FLOW_SYNC);

        // Call to turn off IPU clocks - should not actually disable IPU clocks, 
        // since previous call to enable display flows turns on IPU submodules and thus IPU clocks.
        IPUV3DisableClocks(g_hIPUBase);

        BSPInitializePanel(pDIPanelInfo);

        CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC0_SRC, 0 /* MCU is source */);

        // Enable DP channel to begin Sync operation
        g_dwCurBGChan = IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE;
        IDMACChannelEnable(g_dwCurBGChan);     
    }
    else
    {
        // Call to turn off IPU clocks
        IPUV3DisableClocks(g_hIPUBase);
    }

}


//------------------------------------------------------------------------------
//
// Function: DisplayConfigureTVAndLCD
//
// This function configures IPU flow settings for the TV + LCD
// display mode. TV is the primary display device.
//
//
// Parameters:
//      di_sel_main
//          [in] Selects DI0 or DI1 to configure for the main (UI)
//          display panel in a dual configuration.
//
//      di_sel_aux
//          [in] Selects DI0 or DI1 to configure for the auxiliary
//          display panel in a dual configuration.
//
// Returns:
//      None
//------------------------------------------------------------------------------
static void DisplayConfigureDual(DI_SELECT di_sel_main, DI_SELECT di_sel_aux)
{
    UINT32 bpp;
    PANEL_INFO *pDIPanelInfoMain, *pDIPanelInfoAux;
    GPEMode *pModeMain, *pModeAux;

    if (di_sel_main == DI_SELECT_DI0)
    {
        pDIPanelInfoMain = g_pDI0PanelInfo;
    }
    else
    {
        pDIPanelInfoMain = g_pDI1PanelInfo;
    }

    if (di_sel_aux == DI_SELECT_DI0)
    {
        pDIPanelInfoAux = g_pDI0PanelInfo;
    }
    else
    {
        pDIPanelInfoAux = g_pDI1PanelInfo;
    }

    // Choose the mode info to use
    if (pDIPanelInfoMain->MODEID == (DWORD)g_SecondaryMode.modeId)
    {
        pModeMain = &g_SecondaryMode;
    }
    else
    {
        pModeMain = &g_PrimaryMode;
    }

    if (pDIPanelInfoAux->MODEID == (DWORD)g_SecondaryMode.modeId)
    {
        pModeAux = &g_SecondaryMode;
    }
    else
    {
        pModeAux = &g_PrimaryMode;
    }

    bpp = pModeMain->Bpp;

    // Call to turn on IPU clocks for register initializations
    IPUV3EnableClocks(g_hIPUBase);
    
    // Set up IPUv3 DC
    DCSetup(di_sel_main);
    
    // Set up IPUv3 DI1
    DISetup(di_sel_main);

    if (pDIPanelInfoMain->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_SYNCHRONOUS)
    {
        //************************************************************************
        // Configure Channel 23 for main display operation 
        //************************************************************************

        // Set up CPMEM for TV
        CPMEMSetup(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, pDIPanelInfoMain->WIDTH, pDIPanelInfoMain->HEIGHT, bpp, g_PrimaryBufStride, pDIPanelInfoMain->ISINTERLACE);
        CPMEMSetup(IDMACChannelAltChIndex(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE), pDIPanelInfoMain->WIDTH, pDIPanelInfoMain->HEIGHT, bpp, g_PrimaryBufStride, pDIPanelInfoMain->ISINTERLACE);

        // Set up IPUv3 DMFC
        DMFCSetup(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, di_sel_main, pModeMain->width);

        // Set high priority for display update channel
        IDMACChannelSetPriority(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, IDMAC_CH_PRIORITY_HIGH);
        if(pDIPanelInfoMain->PIXEL_FMT == IPU_PIX_FMT_YUV422)
        {
            // CSC from RGB to YUV422
            dpCSCConfigData DpCSCConfigData;
            DP_CH_TYPE DpChannel = DP_CHANNEL_SYNC;

            DPCSCGetCoeffs(DpChannel, &DpCSCConfigData);
            if (DpCSCConfigData.CSCPosition == DP_CSC_DISABLE)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_BG;
                DpCSCConfigData.CSCEquation = CSCR2Y_A1;  
            
                DpCSCConfigData.bGamutEnable = FALSE;
                DPCSCConfigure(DpChannel, &DpCSCConfigData);
            }
            else if (DpCSCConfigData.CSCPosition == DP_CSC_FG)
            {
                DpCSCConfigData.CSCPosition = DP_CSC_BOTH;
                DPCSCConfigure(DpChannel, &DpCSCConfigData);
            }
        }

        //************************************************************************
        // Configure Channel 28 for auxiliary display operation 
        //************************************************************************

        //For dumb panel DC channel
        // Set up CPMEM for LCD
        CPMEMSetup(IDMAC_CH_DC_SYNC_ASYNC_FLOW, pDIPanelInfoAux->WIDTH, pDIPanelInfoAux->HEIGHT, pModeAux->Bpp, g_Primary2BufStride, pDIPanelInfoAux->ISINTERLACE);

        // Set up IPUv3 DMFC
        DMFCSetup(IDMAC_CH_DC_SYNC_ASYNC_FLOW, di_sel_aux, pModeAux->width);

        // Set high priority for display update channel
        IDMACChannelSetPriority(IDMAC_CH_DC_SYNC_ASYNC_FLOW, IDMAC_CH_PRIORITY_HIGH);


        //************************************************************************
        // Configure Channel 28 for auxiliary display operation 
        //************************************************************************

        // Enable flow, which enables necessary IPU modules
        DisplayEnableSingleFlow(DISPLAY_FLOW_SYNC);

        // Call to turn off IPU clocks - should not actually disable IPU clocks, 
        // since previous call to enable display flows turns on IPU submodules and thus IPU clocks.
        IPUV3DisableClocks(g_hIPUBase);

        BSPInitializePanel(pDIPanelInfoMain);

        CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC0_SRC, 0 /* MCU is source */);

        //IDMAC enable dc at first
        g_dwCurBGChan = IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE;
        IDMACChannelEnable(g_dwCurBGChan);  

        //For dumb panel DC channel
        CMSetDispFlow(IPU_DISP_FLOW_DC1_SRC, 0 /* MCU is source */);
        //IDMAC enable dc at first
        IDMACChannelEnable(IDMAC_CH_DC_SYNC_ASYNC_FLOW); 
    }
    else
    {
        // Error - selected panel must be synchronous to support TV out
    }
}

//------------------------------------------------------------------------------
//
// Function: DisplayIsBusy
//
// This function returns the status of primary surface background channel.
//
//
// Parameters:
//      None
//
// Returns:
//      True if busy, FALSE if idle.
//------------------------------------------------------------------------------
BOOL DisplayIsBusy()
{
    //For dual display, this function need be modified.
    return IDMACChannelIsBusy(g_dwCurBGChan);
}

//------------------------------------------------------------------------------
//
// Function: DisplayCorrectPixelFormatFromRegistry
//
// This function corrects the pixel format from registry.
// display mode.
//
//
// Parameters:
//      di_sel
//          [in] Selects DI0 or DI1 to correct the pixel format
//
// Returns:
//      None
//------------------------------------------------------------------------------
static void DisplayCorrectPixelFormatFromRegistry(DI_SELECT di_sel)
{
    DWORD bpp = 0;
    PANEL_INFO *pDIPanelInfo;

    if (di_sel == DI_SELECT_DI0)
    {
        pDIPanelInfo = g_pDI0PanelInfo;
    }
    else
    {
        pDIPanelInfo = g_pDI1PanelInfo;
    }
    
    bpp = BSPGetPixelDepthFromRegistry();

    // correct the pixel format according to bpp setting in registry.
    if(pDIPanelInfo->PIXEL_FMT != IPU_PIX_FMT_YUV422)
    {
        if(bpp == 16)
        {
            pDIPanelInfo->PIXEL_FMT = IPU_PIX_FMT_RGB565;
        }
        else if(bpp == 18)
        {
            pDIPanelInfo->PIXEL_FMT = IPU_PIX_FMT_RGB666;
        }
        else if(bpp == 24)
        {
            pDIPanelInfo->PIXEL_FMT = IPU_PIX_FMT_RGB24;
        }
        else if(bpp == 32)
        {
            pDIPanelInfo->PIXEL_FMT = IPU_PIX_FMT_RGB24;
        }
    }  
}
//------------------------------------------------------------------------------
//
// Function: DisplaySetStride
//
// This function set the stride of primary buffer.
//
// Parameters:
//      bIsBuf2 : 
//          [in] TRUE: For primary buffer. FALSE: For primary buffer 2. 
//
//      stride : 
//          [in] The stride of certain primary buffer.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void DisplaySetStride(BOOL bIsBuf2, UINT32 stride)
{
    if(bIsBuf2)
        g_Primary2BufStride = stride; 
    else
        g_PrimaryBufStride = stride;
        
    return;
}


//------------------------------------------------------------------------------
//
// Function: DisplaySetGamma
//
// This function set the gamma value into DP sub-module.
// The gamma value setting only applies for primary dumb display device.
//
//
// Parameters:
//      fGamma : 
//          [in] The gamma value.
//
// Returns:
//      True if success, FALSE if failed.
//------------------------------------------------------------------------------
BOOL DisplaySetGamma(float fGamma)
{
    dpGammaConfigData GammaConfig;
    dpCSCConfigData DpCSCConfigData;
    DP_CH_TYPE DpChannel = DP_CHANNEL_SYNC;    
    if((fGamma < 0.1)||(fGamma > 10))
    {
        ERRORMSG(1,(TEXT("Acceptable gamma value range is 0.1~10(Current:%f).\r\n"),fGamma));
        return FALSE;
    }

    DPCSCGetCoeffs(DpChannel, &DpCSCConfigData);

    //Currently, only TVout need BG CSC. 
    //If BG CSC is enabled, the output of BG must be YUV.
    if((DpCSCConfigData.CSCPosition == DP_CSC_BOTH)
        ||(DpCSCConfigData.CSCPosition == DP_CSC_BG))
    {    
        GammaConfig.bYUVmode= TRUE;
    }
    else
    {
        GammaConfig.bYUVmode= FALSE;
    }
    GammaConfig.Gamma.fCenter =32;
    GammaConfig.Gamma.fWidth=32;
    //The value can be changed according to real requirement.
    GammaConfig.Gamma.fValue=fGamma;
    DPGammaConfigure(DpChannel, &GammaConfig); 
    return TRUE;
    
}

//------------------------------------------------------------------------------
//
// Function: DisplayDMIEnable
//
// This function enabled DMI feature to speedup the communication between IPU and GPU.
//
//
// Parameters:
//      eGPUID : 
//          [in] To determine connect with Z430 or Z160.
//
//      eColorFormat : 
//          [in] To declare the buffer color format provided by GPU.
//
//      uiStride : 
//          [in] To declare the stride of buffer provided by GPU.(byte)
//
//      eBufferMode : 
//          [in] To declare use double buffer mode or triple buffer mode.
//                Currently only triple buffer mode is supported.
//
//      pBuffer1 : 
//          [in] The physical address of first buffer.
//
//      pBuffer2 : 
//          [in] The physical address of second buffer.
//
//      pBuffer3 : 
//          [in] The physical address of third buffer.(Only valid for triple buffer mode)
//
// Returns:
//      True if success, FALSE if failed.
//------------------------------------------------------------------------------
BOOL DisplayDMIEnable(DISPLAY_GPUID eGPUID, DISPLAY_GPU_COLOR eColorFormat,
                            UINT32 uiStride, DISPLAY_GPU_BUFFERMODE eBufferMode, 
                            void* pBuffer1, void* pBuffer2, void* pBuffer3)
{
    PANEL_INFO *pDIPanelInfo;
    DI_SELECT di_sel;
    int bpp;

    if(eBufferMode != DMI_TRIPLE_BUFFERED)
    {
        g_bDoubleBufferMode = TRUE;
    }

    // Only IPUv3EX which supports triple buffer mode can support DMI. 
    // So here use tripleCurrentbufferr to validate DMI feature.
    if(IDMACChannelQueryTripleCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)==-1)
    {
        ERRORMSG(1,(TEXT("DMI mode is not supported in this chip.\r\n")));
        return FALSE;        
    }


    
    if (g_pDI1PanelInfo->ISACTIVE == TRUE)
    {
        pDIPanelInfo = g_pDI1PanelInfo;
        di_sel = DI_SELECT_DI1;
    }
    else
    {
        pDIPanelInfo = g_pDI0PanelInfo;
        di_sel = DI_SELECT_DI0;
    }

    
    // Call to turn on IPU clocks for register initializations
    IPUV3EnableClocks(g_hIPUBase);

    // Disable all flows so that we disable IPU modules while reconfiguring
    // We will re-enable the appropriate flows once configuration is complete.
    DisplayDisableSingleFlow(DISPLAY_FLOW_ALL);
    
    if(eColorFormat ==  DMI_RGB565)
    {
        bpp = 16;
    }
    else
    {
        bpp = 24;
    }

    if (pDIPanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_SYNCHRONOUS)
    {
        //************************************************************************
        // Configure Channel 23 for TV display operation (only UI+Video case)
        //************************************************************************
        IDMACChannelTRBMODE(g_dwCurBGChan,FALSE);
        //IDMACChannelClrCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
        IDMACChannelBUF0ClrReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
        IDMACChannelBUF1ClrReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
        IDMACChannelBUF2ClrReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
        if(!g_bDoubleBufferMode)
        {
            CPMEMWriteBuffer(IDMACChannelAltChIndex(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE),0,(UINT32 *)pBuffer3);
        }
        CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 0, (UINT32 *)pBuffer1);
        CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 1, (UINT32 *)pBuffer2);
        if(g_bDoubleBufferMode)
        {
            IDMACChannelDBMODE(g_dwCurBGChan,TRUE);
        }
        else
        {
            IDMACChannelTRBMODE(g_dwCurBGChan,TRUE);
        }
        
        // Set up CPMEM
        CPMEMSetup(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, pDIPanelInfo->WIDTH, pDIPanelInfo->HEIGHT, bpp, uiStride, pDIPanelInfo->ISINTERLACE);
        CPMEMSetup(IDMACChannelAltChIndex(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE), pDIPanelInfo->WIDTH, pDIPanelInfo->HEIGHT, bpp, uiStride, pDIPanelInfo->ISINTERLACE);
        
        // Enable flow, which enables necessary IPU modules
        DisplayEnableSingleFlow(DISPLAY_FLOW_SYNC);

        // Call to turn off IPU clocks - should not actually disable IPU clocks, 
        // since previous call to enable display flows turns on IPU submodules and thus IPU clocks.
        IPUV3DisableClocks(g_hIPUBase);

        if(eGPUID == DMI_GPU3D)
        {
            CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC0_SRC, 11 /* Z430 */);
            CMSetProcFlow(IPU_PROC_FLOW_EXT_SRC1_DEST, 1 /* dp sync0 */);
        }
        else
        {
            CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC0_SRC, 12 /* Z160 */);
            CMSetProcFlow(IPU_PROC_FLOW_EXT_SRC2_DEST, 1 /* dp sync0 */);
        }
        IDMACChannelEnable(g_dwCurBGChan);  
    }
    else
    {
        // Error 
        IPUV3DisableClocks(g_hIPUBase);
    }

    return TRUE;
    
}
//------------------------------------------------------------------------------
//
// Function: DisplayDMIDisable
//
// This function disabled DMI feature and return to normal mode.
//
//
// Parameters:
//      None.
//
// Returns:
//      True if success, FALSE if failed.
//------------------------------------------------------------------------------

BOOL DisplayDMIDisable()
{
    PANEL_INFO *pDIPanelInfo;
    DI_SELECT di_sel;
    int bpp;

    // Only IPUv3EX which supports triple buffer mode can support DMI. 
    // So here use tripleCurrentbufferr to validate DMI feature.
    if(IDMACChannelQueryTripleCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE)==-1)
    {
        ERRORMSG(1,(TEXT("DMI mode is not supported in this chip.\r\n")));
        return FALSE;        
    }

    if (g_pDI1PanelInfo->ISACTIVE == TRUE)
    {
        pDIPanelInfo = g_pDI1PanelInfo;
        di_sel = DI_SELECT_DI1;
        bpp = g_SecondaryMode.Bpp;
    }
    else
    {
        pDIPanelInfo = g_pDI0PanelInfo;
        di_sel = DI_SELECT_DI0;
        bpp = g_PrimaryMode.Bpp;
    }

    
    // Call to turn on IPU clocks for register initializations
    IPUV3EnableClocks(g_hIPUBase);

    // Disable all flows so that we disable IPU modules while reconfiguring
    // We will re-enable the appropriate flows once configuration is complete.
    DisplayDisableSingleFlow(DISPLAY_FLOW_ALL);

    if (pDIPanelInfo->SYNC_TYPE == IPU_PANEL_SYNC_TYPE_SYNCHRONOUS)
    {
        //************************************************************************
        // Configure Channel 23 for TV display operation (only UI+Video case)
        //************************************************************************
        if(g_bDoubleBufferMode)
        {
            g_bDoubleBufferMode = FALSE;
            IDMACChannelDBMODE(g_dwCurBGChan,FALSE);
        }
        else
        {
            IDMACChannelTRBMODE(g_dwCurBGChan,FALSE);
        }
        //IDMACChannelClrCurrentBuf(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
        IDMACChannelBUF0ClrReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
        IDMACChannelBUF1ClrReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
        IDMACChannelBUF2ClrReady(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE);
        CPMEMWriteBuffer(IDMACChannelAltChIndex(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE),0,(UINT32 *)g_PrimaryBufPhysAddr);
        CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 0, (UINT32 *)g_PrimaryBufPhysAddr);
        CPMEMWriteBuffer(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, 1, (UINT32 *)g_PrimaryBufPhysAddr);
        IDMACChannelTRBMODE(g_dwCurBGChan,TRUE);
        // Set up CPMEM
        CPMEMSetup(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE, pDIPanelInfo->WIDTH, pDIPanelInfo->HEIGHT, bpp, (pDIPanelInfo->WIDTH * bpp / 8), pDIPanelInfo->ISINTERLACE);
        CPMEMSetup(IDMACChannelAltChIndex(IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE), pDIPanelInfo->WIDTH, pDIPanelInfo->HEIGHT, bpp, (pDIPanelInfo->WIDTH * bpp / 8), pDIPanelInfo->ISINTERLACE);
       
        // Enable flow, which enables necessary IPU modules
        DisplayEnableSingleFlow(DISPLAY_FLOW_SYNC);

        // Call to turn off IPU clocks - should not actually disable IPU clocks, 
        // since previous call to enable display flows turns on IPU submodules and thus IPU clocks.
        IPUV3DisableClocks(g_hIPUBase);

        CMSetProcFlow(IPU_PROC_FLOW_EXT_SRC1_DEST, 0 /* MCU */);
        CMSetProcFlow(IPU_PROC_FLOW_EXT_SRC2_DEST, 0 /* MCU */);
        CMSetDispFlow(IPU_DISP_FLOW_DP_SYNC0_SRC, 0 /* MCU */);
        IDMACChannelEnable(g_dwCurBGChan);  
    }
    else
    {
        // Error 
        IPUV3DisableClocks(g_hIPUBase);
    }
    
    return TRUE;
    
}

//-----------------------------------------------------------------------------
//
// Function:  DisplayRegPush
//
// This function saves the IPU registers before power gating off.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisplayRegPush()
{
    IPUV3EnableClocks(g_hIPUBase);
    IDMACChannelBUFReadyPush();
    IPUV3DisableClocks(g_hIPUBase);
}

//-----------------------------------------------------------------------------
//
// Function:  DisplayRegPop
//
// This function restores the IPU registers after power gating on.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisplayRegPop()
{
    IPUV3EnableClocks(g_hIPUBase);
    IDMACChannelBUFReadyPop();
    IPUV3DisableClocks(g_hIPUBase);
}

#if DEBUG_DUMP_IPU_REGS
static void DumpIPURegs()
{
    CMDumpRegs();
    DIDumpRegs();
    DCDumpRegs();
    DMFCDumpRegs();
    IDMACDumpRegs();
}
#endif
