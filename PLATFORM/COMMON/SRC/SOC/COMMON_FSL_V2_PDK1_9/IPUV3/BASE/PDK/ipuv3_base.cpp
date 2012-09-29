//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <cmnintrin.h>
#include <new.h>
#pragma warning(pop)

#include "common_macros.h"
#include "ipuv3_base_include.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"
#include "ipuv3_base_priv.h"



//------------------------------------------------------------------------------
// External Functions
extern "C" DWORD IPUGetIRQ();
extern "C" BOOL BSPIPUSetClockGatingMode(DWORD dwIPUEnableMask);
extern "C" DWORD CSPIPUGetBaseAddr();
extern "C" BOOL BSPGetCacheMode();

//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
PCSP_IPU_COMMON_REGS g_pIPU;
HANDLE g_hIpuIntrEvent;

// IDMAC interrupt events
HANDLE g_hPrpIntrEvent;
HANDLE g_hPpIntrEvent;
HANDLE g_hDPBGIntrEvent;
HANDLE g_hDPFGIntrEvent;
HANDLE g_hDCChannel0IntrEvent;
HANDLE g_hDCChannel1IntrEvent;
HANDLE g_hDCChannel2IntrEvent;
HANDLE g_hDCIntrEvent;
HANDLE g_hSmfcIntrEvent0;
HANDLE g_hSmfcIntrEvent1;
HANDLE g_hSmfcIntrEvent2;
HANDLE g_hSmfcIntrEvent3;
HANDLE g_hVDIIntrEvent;

// Non-IDMAC interrupt events
HANDLE g_hSnoopingIntrEvent;
HANDLE g_hDPFlowIntrEvent;
HANDLE g_hDCFrameCompleteIntrEvent;
HANDLE g_hDIVSyncPreIntrEvent;
HANDLE g_hDICounterIntrEvent;

// Error interrupt events
HANDLE g_hDCTearingErrorIntrEvent;
HANDLE g_hDISyncErrorIntrEvent;
HANDLE g_hDITimeoutErrorIntrEvent;
HANDLE g_hICFrameLostErrorIntrEvent;
HANDLE g_hAXIErrorIntrEvent;

DWORD g_ipuIntr;
HANDLE g_hIpuISRThread;
static HANDLE g_phIntrHdlr[2][IPU_INT_MAX_ID];
static DWORD g_pIntrGroupMask[2][IPU_INT_MAX_ID];
static DWORD g_pUpperRegsMask[IPU_INT_MAX_ID];
static BOOL g_bPPEnableIC;
static BOOL g_bPRPEnableIC;
static BOOL g_bPPEnableIRT;
static BOOL g_bPRPVFEnableIRT;
static BOOL g_bPRPENCEnableIRT;
static DWORD g_dwIPUEnableMask;
static CRITICAL_SECTION g_csIPUEnableMask;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
static void HandleIDMACInterrupts();
static void HandleNonIDMACInterrupts();
static void HandleErrorInterrupts();
static UINT32 GetControlReg(int);
static UINT32 GetStatusReg(int);
static void ClearControlReg(int, DWORD);


//------------------------------------------------------------------------------
//
// Function: InitIPU
//
// This function initializes common IPU components.
//
// Parameters:
//      none.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
BOOL InitIPU(void)
{
    BOOL bCachedWT;

    IPU_FUNCTION_ENTRY();

    g_bPPEnableIC = FALSE;
    g_bPRPEnableIC = FALSE;
    g_bPPEnableIRT = FALSE;
    g_bPRPVFEnableIRT = FALSE;
    g_bPRPENCEnableIRT = FALSE;

    // IPU modules start disabled
    g_dwIPUEnableMask = 0;

    // Initialize lock for IPUEnableMask
    InitializeCriticalSection(&g_csIPUEnableMask);

    // Allocate IPU driver data structures.
    if (!IpuAlloc())
    {
        DEBUGMSG(ZONE_ERROR, (_T("IPU common Init:  Allocation failed!\r\n")));
    }

    // Initialize Video Memory region
    bCachedWT = BSPGetCacheMode();
    if (!IpuBufferManagerInit(bCachedWT))
    {
        DEBUGMSG(ZONE_ERROR, (_T("IPU common Init:  Video Memory Init failed!\r\n")));
        goto Error;
    }

    // Initialize IPU interrupt, sub-module events, and interrupt masks
    if (!IpuIntrInit())
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: Interrupt initialization failed!\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Initialize thread for IPU Interrupt Handling ISR
    //      pThreadAttributes = NULL (must be NULL)
    //      dwStackSize = 0 => default stack size determined by linker
    //      lpStartAddress = CspiProcessQueue => thread entry point
    //      lpParameter = NULL => point to thread parameter
    //      dwCreationFlags = 0 => no flags
    //      lpThreadId = NULL => thread ID is not returned
    PREFAST_SUPPRESS(5451, "affects 64 bit windows only");
    g_hIpuISRThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)IpuIntrThread, NULL, 0, NULL);

    if (g_hIpuISRThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        goto Error;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create IPU ISR thread success\r\n"), __WFUNCTION__));
        CeSetThreadPriority(g_hIpuISRThread, 95);//THREAD_PRIORITY_TIME_CRITICAL);
    }

    // Initialize IMA_ADDR to 0, so that IPU submodule drivers can
    // access the IMA registers.
//    OUTREG32(&g_pIPU->IPU_IMA_ADDR, 0);

    return TRUE;

Error:
    DeinitIPU();
    return FALSE;
}


//------------------------------------------------------------------------------
//
// Function: DeinitIPU
//
// This function deinitializes PMIC subsystems.
//
// Parameters:
//          none.
// Returns:
//          none.
//------------------------------------------------------------------------------
void DeinitIPU(void)
{
    IPU_FUNCTION_ENTRY();

    // Unmap IPU register pointer
    IpuDealloc();

    if (g_hIpuIntrEvent != NULL)
    {
        CloseHandle(g_hIpuIntrEvent);
        g_hIpuIntrEvent = NULL;
    }

    CloseHandle(g_hPrpIntrEvent);
    g_hPrpIntrEvent = NULL;

    CloseHandle(g_hPpIntrEvent);
    g_hPpIntrEvent = NULL;

    if (g_hIpuISRThread)
    {
        CloseHandle(g_hIpuISRThread);
        g_hIpuISRThread = NULL;
    }

    IPU_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function:  IpuAlloc
//
// This function allocates the data structures required for interaction
// with the IPU hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL IpuAlloc(void)
{

    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if (g_pIPU == NULL)
    {
        phyAddr.QuadPart = CSPIPUGetBaseAddr() + CSP_IPUV3_IPU_COMMON_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPU = (PCSP_IPU_COMMON_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_COMMON_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPU == NULL)
        {
            DEBUGMSG(ZONE_ERROR,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:
    // If initialization failed, be sure to clean up
    if (!rc) IpuDealloc();

    IPU_FUNCTION_EXIT();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  PrpIpuDealloc
//
// This function deallocates the data structures required for interaction
// with the IPU hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void IpuDealloc(void)
{
    IPU_FUNCTION_ENTRY();

    // Unmap peripheral registers
    if (g_pIPU)
    {
        MmUnmapIoSpace(g_pIPU, sizeof(CSP_IPU_COMMON_REGS));
        g_pIPU = NULL;
    }

    IPU_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: IpuIntrInit
//
// This function initializes the g_pIntrGroupMask array with
// the appropriate interrupt group masks.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
BOOL IpuIntrInit(void)
{
    DWORD ipuIrq;
    BOOL ret = FALSE;

    IPU_FUNCTION_ENTRY();

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for IPU Interrupt\r\n"), __WFUNCTION__));
    g_hIpuIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_hIpuIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for IPU Interrupt failed\r\n"), __WFUNCTION__));
        return ret;
    }

    // IRQ is defined at SOC level
    ipuIrq = IPUGetIRQ();

    // Translate IRQ to SYSINTR
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &ipuIrq, sizeof(ipuIrq),
        &g_ipuIntr, sizeof(g_ipuIntr), NULL))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Request SYSINTR failed (IPU Sync Interrupt)\r\n"), __WFUNCTION__));
        return ret;
    }

    if(!InterruptInitialize(g_ipuIntr, g_hIpuIntrEvent, NULL, 0))
    {
        CloseHandle(g_hIpuIntrEvent);
        g_hIpuIntrEvent = NULL;
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Interrupt initialization failed! (IPU Interrupt)\r\n"), __WFUNCTION__));
        return ret;
    }

    // Create events to facilitate communication of
    // interrupt status with drivers for IPU sub-modules.
    // Event names are shared between the common IPU code and
    // the Prp and Pp drivers through event strings found in Ipu.h
    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Pre-processor Interrupt event\r\n"), __WFUNCTION__));
    g_hPrpIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_PRP_INTR_EVENT);
    if (g_hPrpIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for Pre-processor Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Post-processor Interrupt event\r\n"), __WFUNCTION__));
    g_hPpIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_PP_INTR_EVENT);
    if (g_hPpIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for Post-processor Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Processor Background Interrupt event\r\n"), __WFUNCTION__));
    g_hDPBGIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DPBG_INTR_EVENT);
    if (g_hDPBGIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for Display Processor Background Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }
    
    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Processor Foreground Interrupt event\r\n"), __WFUNCTION__));
    g_hDPFGIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DPFG_INTR_EVENT);
    if (g_hDPFGIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for Display Processor Foreground Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Controller Interrupt event\r\n"), __WFUNCTION__));
    g_hDCIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DC_INTR_EVENT);
    if (g_hDCIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for DC Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Controller Channel 0 Interrupt event\r\n"), __WFUNCTION__));
    g_hDCChannel0IntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DC_CH0_INTR_EVENT);
    if (g_hDCChannel0IntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for DC Channel 0 Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Controller Channel 1 Interrupt event\r\n"), __WFUNCTION__));
    g_hDCChannel1IntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DC_CH1_INTR_EVENT);
    if (g_hDCChannel1IntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for DC Channel 1 Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Controller Channel 2 Interrupt event\r\n"), __WFUNCTION__));
    g_hDCChannel2IntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DC_CH2_INTR_EVENT);
    if (g_hDCChannel2IntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for DC Channel 2 Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for SMFC Interrupt event\r\n"), __WFUNCTION__));
    g_hSmfcIntrEvent0 = CreateEvent(NULL, FALSE, FALSE, IPU_SMFC_INTR_EVENT0);
    if (g_hSmfcIntrEvent0 == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for SMFC Interrupt Event0 failed\r\n"), __WFUNCTION__));
        return ret;
    }

    g_hSmfcIntrEvent1 = CreateEvent(NULL, FALSE, FALSE, IPU_SMFC_INTR_EVENT1);
    if (g_hSmfcIntrEvent1 == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for SMFC Interrupt Event1 failed\r\n"), __WFUNCTION__));
        return ret;
    }

    g_hSmfcIntrEvent2 = CreateEvent(NULL, FALSE, FALSE, IPU_SMFC_INTR_EVENT2);
    if (g_hSmfcIntrEvent2 == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for SMFC Interrupt Event2 failed\r\n"), __WFUNCTION__));
        return ret;
    }

    g_hSmfcIntrEvent3 = CreateEvent(NULL, FALSE, FALSE, IPU_SMFC_INTR_EVENT3);
    if (g_hSmfcIntrEvent3 == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for SMFC Interrupt Event3 failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for VDI Interrupt event\r\n"), __WFUNCTION__));
    g_hVDIIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_VDI_INTR_EVENT);
    if (g_hVDIIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for VDI Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Snooping Interrupt event\r\n"), __WFUNCTION__));
    g_hSnoopingIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_SNOOPING_INTR_EVENT);
    if (g_hSnoopingIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for Snooping Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Processor Flow Interrupt event\r\n"), __WFUNCTION__));
    g_hDPFlowIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DP_FLOW_INTR_EVENT);
    if (g_hDPFlowIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for Display Processor Flow Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }


    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Controller Frame Complete Interrupt event\r\n"), __WFUNCTION__));
    g_hDCFrameCompleteIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DC_FRAME_COMPLETE_INTR_EVENT);
    if (g_hDCFrameCompleteIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for DC Frame Complete Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Interface VSync Pre Interrupt event\r\n"), __WFUNCTION__));
    g_hDIVSyncPreIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DI_VSYNC_PRE_INTR_EVENT);
    if (g_hDIVSyncPreIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for DI VSync Pre Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Interface Counter Interrupt event\r\n"), __WFUNCTION__));
    g_hDICounterIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DI_COUNTER_INTR_EVENT);
    if (g_hDICounterIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for DI Counter Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Controller Tearing Error Interrupt event\r\n"), __WFUNCTION__));
    g_hDCTearingErrorIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DC_TEARING_ERROR_INTR_EVENT);
    if (g_hDCTearingErrorIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for DC Tearing Error Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Interface Sync Error Interrupt event\r\n"), __WFUNCTION__));
    g_hDISyncErrorIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DI_SYNC_ERROR_INTR_EVENT);
    if (g_hDISyncErrorIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for DI Sync Error Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Display Interface Timeout Error Interrupt event\r\n"), __WFUNCTION__));
    g_hDITimeoutErrorIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_DI_TIMEOUT_ERROR_INTR_EVENT);
    if (g_hDITimeoutErrorIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for DI Timeout Error Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Image Converter Frame Lost Error Interrupt event\r\n"), __WFUNCTION__));
    g_hICFrameLostErrorIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_IC_FRAME_LOST_ERROR_INTR_EVENT);
    if (g_hICFrameLostErrorIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for IC Frame Lost Error Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }


    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for AXI Error Interrupt event\r\n"), __WFUNCTION__));
    g_hAXIErrorIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_AXI_ERROR_INTR_EVENT);
    if (g_hAXIErrorIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for AXI Error Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    g_phIntrHdlr[0][IPU_INT_IDMAC_0] = g_hSmfcIntrEvent0;
    g_phIntrHdlr[0][IPU_INT_IDMAC_1] = g_hSmfcIntrEvent1;
    g_phIntrHdlr[0][IPU_INT_IDMAC_2] = g_hSmfcIntrEvent2;
    g_phIntrHdlr[0][IPU_INT_IDMAC_3] = g_hSmfcIntrEvent3;
    g_phIntrHdlr[0][IPU_INT_IDMAC_4] = 0;
    g_phIntrHdlr[0][IPU_INT_IDMAC_5] = 0;
    g_phIntrHdlr[0][IPU_INT_IDMAC_6] = 0;
    g_phIntrHdlr[0][IPU_INT_IDMAC_7] = 0;
    g_phIntrHdlr[0][IPU_INT_IDMAC_8] = g_hVDIIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_9] = g_hVDIIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_10] = g_hVDIIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_11] = g_hPpIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_12] = g_hPrpIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_13] = g_hVDIIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_14] = g_hPrpIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_15] = g_hPpIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_16] = 0;
    g_phIntrHdlr[0][IPU_INT_IDMAC_17] = g_hPrpIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_18] = g_hPpIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_19] = 0;
    g_phIntrHdlr[0][IPU_INT_IDMAC_20] = g_hPrpIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_21] = g_hPrpIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_22] = g_hPpIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_23] = g_hDPBGIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_24] = g_hDPBGIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_25] = 0;
    g_phIntrHdlr[0][IPU_INT_IDMAC_26] = 0;
    g_phIntrHdlr[0][IPU_INT_IDMAC_27] = g_hDPFGIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_28] = g_hDCChannel1IntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_29] = g_hDPFGIntrEvent;
    g_phIntrHdlr[0][IPU_INT_IDMAC_30] = 0;
    g_phIntrHdlr[0][IPU_INT_IDMAC_31] = g_hDPFGIntrEvent;

    g_phIntrHdlr[1][IPU_INT_IDMAC_32] = 0;
    g_phIntrHdlr[1][IPU_INT_IDMAC_33] = g_hDPFGIntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_34] = 0;
    g_phIntrHdlr[1][IPU_INT_IDMAC_35] = 0;
    g_phIntrHdlr[1][IPU_INT_IDMAC_36] = 0;
    g_phIntrHdlr[1][IPU_INT_IDMAC_37] = 0;
    g_phIntrHdlr[1][IPU_INT_IDMAC_38] = 0;
    g_phIntrHdlr[1][IPU_INT_IDMAC_39] = 0;
    g_phIntrHdlr[1][IPU_INT_IDMAC_40] = g_hDCChannel0IntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_41] = g_hDCChannel2IntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_42] = g_hDCIntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_43] = g_hDCIntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_44] = g_hDCIntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_45] = g_hPrpIntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_46] = g_hPrpIntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_47] = g_hPpIntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_48] = g_hPrpIntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_49] = g_hPrpIntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_50] = g_hPpIntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_51] = g_hDPBGIntrEvent;
    g_phIntrHdlr[1][IPU_INT_IDMAC_52] = g_hDPBGIntrEvent;


    g_pIntrGroupMask[0][IPU_INT_IDMAC_0] = SMFC0_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_1] = SMFC1_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_2] = SMFC2_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_3] = SMFC3_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_4] = 0;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_5] = 0;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_6] = 0;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_7] = 0;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_8] = VDI_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_9] = VDI_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_10] = VDI_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_11] = PP_INPUT_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_12] = PRP_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_13] = VDI_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_14] = PRP_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_15] = PP_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_16] = 0;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_17] = PRP_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_18] = PP_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_19] = 0;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_20] = PRP_ENC_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_21] = PRP_VF_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_22] = PP_OUTPUT_DMA_CHA_MASK;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_23] = DP_DMA_CHA_MASK_LOWER;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_24] = DP_DMA_CHA_MASK_LOWER;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_25] = 0;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_26] = 0;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_27] = DP_DMA_CHA_MASK_LOWER;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_28] = DC_DMA_CHA_MASK_LOWER;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_29] = DP_DMA_CHA_MASK_LOWER;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_30] = 0;
    g_pIntrGroupMask[0][IPU_INT_IDMAC_31] = DP_DMA_CHA_MASK_LOWER;

    g_pIntrGroupMask[1][IPU_INT_IDMAC_32] = 0;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_33] = DP_DMA_CHA_MASK_UPPER;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_34] = 0;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_35] = 0;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_36] = 0;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_37] = 0;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_38] = 0;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_39] = 0;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_40] = DC_DMA_CHA_MASK_UPPER;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_41] = DC_DMA_CHA_MASK_UPPER;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_42] = DC_DMA_CHA_MASK_UPPER;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_43] = DC_DMA_CHA_MASK_UPPER;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_44] = DC_DMA_CHA_MASK_UPPER;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_45] = PRP_ENC_ROT_DMA_CHA_MASK;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_46] = PRP_VF_ROT_DMA_CHA_MASK;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_47] = PP_ROT_INPUT_DMA_CHA_MASK;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_48] = PRP_ENC_ROT_DMA_CHA_MASK;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_49] = PRP_VF_ROT_DMA_CHA_MASK;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_50] = PP_ROT_OUTPUT_DMA_CHA_MASK;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_51] = DP_DMA_CHA_MASK_UPPER;
    g_pIntrGroupMask[1][IPU_INT_IDMAC_52] = DP_DMA_CHA_MASK_UPPER;

    // Only a few of the lower registers, those related to either
    // DP or DC, have corresponding bits in the upper registers
    // that must be cleared when an interrupt occurs.
    memset(g_pUpperRegsMask, 0, sizeof(DWORD) * IPU_INT_MAX_ID);
    g_pUpperRegsMask[IPU_INT_IDMAC_23] = DP_DMA_CHA_MASK_UPPER;
    g_pUpperRegsMask[IPU_INT_IDMAC_24] = DP_DMA_CHA_MASK_UPPER;
    g_pUpperRegsMask[IPU_INT_IDMAC_27] = DP_DMA_CHA_MASK_UPPER;
    g_pUpperRegsMask[IPU_INT_IDMAC_28] = DC_DMA_CHA_MASK_UPPER;
    g_pUpperRegsMask[IPU_INT_IDMAC_29] = DP_DMA_CHA_MASK_UPPER;
    g_pUpperRegsMask[IPU_INT_IDMAC_31] = DP_DMA_CHA_MASK_UPPER;

    ret = TRUE;

    IPU_FUNCTION_EXIT();

    return ret;
}


//------------------------------------------------------------------------------
//
// Function: IpuModuleEnable
//
// This function tracks IPU modules being enabled, and
// turns on IPU clocks when any IPU modules is enabled.
//
// Parameters:
//      module
//          [in] Identifies the IPU module that is being enabled.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IpuModuleEnable(DWORD module)
{
    if (module == 0)
    {
        DEBUGMSG (ZONE_ERROR, (TEXT("%s: Bad parameter!  Aborting.\r\n"), __WFUNCTION__));
        return;
    }

    EnterCriticalSection(&g_csIPUEnableMask);

    g_dwIPUEnableMask |= module;

    BSPIPUSetClockGatingMode(g_dwIPUEnableMask);

    LeaveCriticalSection(&g_csIPUEnableMask);

    return;
}


//------------------------------------------------------------------------------
//
// Function: IpuModuleDisable
//
// This function tracks IPU modules being disabled, and
// turns off IPU clocks if all IPU modules are disabled.
//
// Parameters:
//      module
//          [in] Identifies the IPU module that is being disabled.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IpuModuleDisable(DWORD module)
{
    if (module == 0)
    {
        DEBUGMSG (ZONE_ERROR, (TEXT("%s: Bad parameter!  Aborting.\r\n"), __WFUNCTION__));
        return;
    }

    EnterCriticalSection(&g_csIPUEnableMask);

    g_dwIPUEnableMask &= ~module;

    BSPIPUSetClockGatingMode(g_dwIPUEnableMask);

    LeaveCriticalSection(&g_csIPUEnableMask);

    return;
}

//------------------------------------------------------------------------------
//
// Function: EnableCSI0
//
// This function enables the IPU Camera Sensor Interface (CSI0) module.
// If the module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableCSI0()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_CSI0);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the CSI0.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_CSI0_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_CSI0_EN, IPU_IPU_CONF_CSI0_EN_ENABLE);

    // Use interlocked function to enable the CSI0.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    RETAILMSG(0, (_T("Enabling CSI0\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableCSI0
//
// This function disables the IPU Camera Sensor Interface (CSI0) module.
// If another driver is using the CSI0, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableCSI0()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the CSI0.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_CSI0_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_CSI0_EN, IPU_IPU_CONF_CSI0_EN_DISABLE);

    // Use interlocked function to disable CSI0.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling CSI0\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_CSI0);
}

//------------------------------------------------------------------------------
//
// Function: EnableCSI1
//
// This function enables the IPU Camera Sensor Interface (CSI0) module.
// If the module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableCSI1()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_CSI1);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the CSI1.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_CSI1_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_CSI1_EN, IPU_IPU_CONF_CSI1_EN_ENABLE);

    // Use interlocked function to enable the CSI1.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling CSI1\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableCSI1
//
// This function disables the IPU Camera Sensor Interface (CSI1) module.
// If another driver is using the CSI1, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableCSI1()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the CSI1.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_CSI1_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_CSI1_EN, IPU_IPU_CONF_CSI1_EN_DISABLE);

    // Use interlocked function to disable CSI1.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling CSI1\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_CSI1);
}

//------------------------------------------------------------------------------
//
// Function: EnableIC
//
// This function enables the IPU Image Converter (IC) modules.
// If these modules are already enabled, no action is taken.
//
// Parameters:
//      driver
//          [in] Identifies whether the Pre-Processor (PRP) or
//          Post-Processor (PP) is enabling the IC.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableIC(IPU_DRIVER driver)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (driver == IPU_DRIVER_PP)
    {
        g_bPPEnableIC = TRUE;

        // If PRP has already enabled IC, we are done
        if (g_bPRPEnableIC)
        {
            return;
        }
    }
    else if ((driver == IPU_DRIVER_PRP_VF) || (driver == IPU_DRIVER_PRP_ENC))
    {
        g_bPRPEnableIC = TRUE;

        // If PP has already enabled IC, we are done
        if (g_bPPEnableIC)
        {
            return;
        }
    }

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_IC);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the IC and the rotation unit.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_IC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_IC_EN, IPU_IPU_CONF_IC_EN_ENABLE);

    // Use interlocked function to enable the IC and the rotation unit.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling IC\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableIC
//
// This function disables the IPU Image Converter (IC) modules.
// If another driver is using the IC, it will remain enabled.
//
// Parameters:
//      driver
//          [in] Identifies whether the Pre-Processor (PRP) or
//          Post-Processor (PP) is disabling the IC.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableIC(IPU_DRIVER driver)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (driver == IPU_DRIVER_PP)
    {
        g_bPPEnableIC = FALSE;

        // If PRP still has IC enabled, leave it enabled and return.
        if (g_bPRPEnableIC)
        {
            return;
        }
    }
    else if ((driver == IPU_DRIVER_PRP_VF) || (driver == IPU_DRIVER_PRP_ENC))
    {
        g_bPRPEnableIC = FALSE;

        // If PP still has IC enabled, leave it enabled and return.
        if (g_bPPEnableIC)
        {
            return;
        }
    }
    // Protect access to IPU_CONF register.
    // Set the bit to enable the IC and the rotation unit.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_IC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_IC_EN, IPU_IPU_CONF_IC_EN_DISABLE);

    // Use interlocked function to disable IC and the rotation unit.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling IC\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_IC);
}


//------------------------------------------------------------------------------
//
// Function: EnableIRT
//
// This function enables the IPU Image Rotation (IRT) modules.
// If this module is already enabled, no action is taken.
//
// Parameters:
//      driver
//          [in] Identifies whether the Pre-Processor (PRP) or
//          Post-Processor (PP) is enabling the IRT.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableIRT(IPU_DRIVER driver)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (driver == IPU_DRIVER_PP)
    {
        g_bPPEnableIRT = TRUE;

        // If PRP ENC or PRP VF has already enabled IRT, we are done
        if (g_bPRPVFEnableIRT || g_bPRPENCEnableIRT)
        {
            return;
        }
    }
    else if (driver == IPU_DRIVER_PRP_VF)
    {
        g_bPRPVFEnableIRT = TRUE;

        // If PP or PRP ENC has already enabled IRT, we are done
        if (g_bPPEnableIRT || g_bPRPENCEnableIRT)
        {
            return;
        }
    }
    else if (driver == IPU_DRIVER_PRP_ENC)
    {
        g_bPRPENCEnableIRT = TRUE;

        // If PP or PRP VF has already enabled IRT, we are done
        if (g_bPPEnableIRT || g_bPRPVFEnableIRT)
        {
            return;
        }
    }

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_IRT);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the IRT.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_IRT_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_IRT_EN, IPU_IPU_CONF_IRT_EN_ENABLE);

    // Use interlocked function to enable the IRT.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling IRT\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableIRT
//
// This function disables the IPU CMOS Sensor Interface (IRT) modules.
// If another driver is using the IRT, it will remain enabled.
//
// Parameters:
//      driver
//          [in] Identifies whether the Pre-Processor (PRP) or
//          Post-Processor (PP) is disabling the IRT.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableIRT(IPU_DRIVER driver)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (driver == IPU_DRIVER_PP)
    {
        g_bPPEnableIRT = FALSE;

        // If PRP ENC or PRP VF still has IC enabled, leave it enabled and return.
        if (g_bPRPENCEnableIRT || g_bPRPVFEnableIRT)
        {
            return;
        }
    }
    else if (driver == IPU_DRIVER_PRP_ENC)
    {
        g_bPRPENCEnableIRT = FALSE;

        // If PP or PRP VF still has IC enabled, leave it enabled and return.
        if (g_bPPEnableIRT || g_bPRPVFEnableIRT)
        {
            return;
        }
    }
    else if (driver == IPU_DRIVER_PRP_VF)
    {
        g_bPRPVFEnableIRT = FALSE;

        // If PP or PRP ENC still has IC enabled, leave it enabled and return.
        if (g_bPPEnableIRT || g_bPRPENCEnableIRT)
        {
            return;
        }
    }

    // Protect access to IPU_CONF register.
    // Set the bit to enable the IRT.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_IRT_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_IRT_EN, IPU_IPU_CONF_IRT_EN_DISABLE);

    // Use interlocked function to disable the IRT.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling IRT\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_IRT);
}


//------------------------------------------------------------------------------
//
// Function: EnableISP
//
// This function enables the IPU Image Signal Processor (ISP) module.
// If this module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableISP()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_ISP);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the ISP.

    // If customer wants to enable ISP, VDI must be disabled.
    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_ISP_EN)
            |CSP_BITFMASK(IPU_IPU_CONF_VDI_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_ISP_EN, IPU_IPU_CONF_ISP_EN_ENABLE)
            |CSP_BITFVAL(IPU_IPU_CONF_VDI_EN, IPU_IPU_CONF_VDI_EN_DISABLE);

    // Use interlocked function to enable the ISP.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling ISP\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableISP
//
// This function disables the Image Signal Processor (ISP) module.
// If another driver is using the ISP, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableISP()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the ISP.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_ISP_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_ISP_EN, IPU_IPU_CONF_ISP_EN_DISABLE);

    // Use interlocked function to disable ISP.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling ISP\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_ISP);
}

//------------------------------------------------------------------------------
//
// Function: EnableDP
//
// This function enables the IPU Synchronous Display Controller (DP) module.
// If this module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableDP()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_DP);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DP.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DP_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DP_EN, IPU_IPU_CONF_DP_EN_ENABLE);

    // Use interlocked function to enable the DP.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling DP\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableDP
//
// This function disables the Synchronous Display Controller (DP) module.
// If another driver is using the DP, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableDP()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DP.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DP_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DP_EN, IPU_IPU_CONF_DP_EN_DISABLE);

    // Use interlocked function to disable DP.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling DP\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_DP);
}

//------------------------------------------------------------------------------
//
// Function: EnableDI0
//
// This function enables the IPU Asynchronous Display Controller (DI0) module.
// If this module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableDI0()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_DI0);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DI0.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DI0_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DI0_EN, IPU_IPU_CONF_DI0_EN_ENABLE);

    // Use interlocked function to enable the SDC.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling DI0\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableDI0
//
// This function disables the Asynchronous Display Controller (DI0) module.
// If another driver is using the DI0, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableDI0()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DI0.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DI0_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DI0_EN, IPU_IPU_CONF_DI0_EN_DISABLE);

    // Use interlocked function to disable DI0.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling DI0\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_DI0);
}


//------------------------------------------------------------------------------
//
// Function: EnableDI1
//
// This function enables the IPU Display Interface (DI1) module.
// If the module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableDI1()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_DI1);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DI1.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DI1_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DI1_EN, IPU_IPU_CONF_DI1_EN_ENABLE);

    // Use interlocked function to enable the DI1.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling DI1\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableDI1
//
// This function disables the IPU Display Interface (DI1) module.
// If another driver is using the DI1, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableDI1()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DI1.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DI1_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DI1_EN, IPU_IPU_CONF_DI1_EN_DISABLE);

    // Use interlocked function to disable DI1.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling DI1\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_DI1);
}

//------------------------------------------------------------------------------
//
// Function: EnableDC
//
// This function enables the IPU Postfilter (DC) module.
// If the module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableDC()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_DC);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DC.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DC_EN, IPU_IPU_CONF_DC_EN_ENABLE);

    // Use interlocked function to enable the DC.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling DC\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableDC
//
// This function disables the IPU Postfilter (DC) module.
// If another driver is using the DI, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableDC()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DC.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DC_EN, IPU_IPU_CONF_DC_EN_DISABLE);

    // Use interlocked function to disable DC.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling DC\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_DC);
}

//------------------------------------------------------------------------------
//
// Function: EnableDMFC
//
// This function enables the IPU Postfilter (DMFC) module.
// If the module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableDMFC()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_DMFC);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DMFC.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DMFC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DMFC_EN, IPU_IPU_CONF_DMFC_EN_ENABLE);

    // Use interlocked function to enable the DMFC.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling DMFC\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableDMFC
//
// This function disables the IPU Postfilter (DMFC) module.
// If another driver is using the DI, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableDMFC()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DMFC.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DMFC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DMFC_EN, IPU_IPU_CONF_DMFC_EN_DISABLE);

    // Use interlocked function to disable DMFC.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling DMFC\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_DMFC);
}

//------------------------------------------------------------------------------
//
// Function: EnableSMFC
//
// This function enables the IPU SMFC module.
// If the module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableSMFC()
{
    UINT32 oldVal, newVal, iMask, iBitval;
    
    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_SMFC);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the SMFC.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_SMFC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_SMFC_EN, IPU_IPU_CONF_SMFC_EN_ENABLE);

    // Use interlocked function to enable the SMFC.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    RETAILMSG(0, (_T("Enabling SMFC\n")));

    return;
}


//------------------------------------------------------------------------------
//
// Function: DisableSMFC
//
// This function disables the IPU SMFC module.
// If another driver is using the SMFC, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableSMFC()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the SMFC.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_SMFC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_SMFC_EN, IPU_IPU_CONF_SMFC_EN_DISABLE);

    // Use interlocked function to disable DMFC.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling DMFC\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_SMFC);
}

//------------------------------------------------------------------------------
//
// Function: EnableSISG
//
// This function enables the IPU SISG module.
// If the module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableSISG()
{
    UINT32 oldVal, newVal, iMask, iBitval;
    
    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_SISG);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the SISG.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_SISG_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_SISG_EN, IPU_IPU_CONF_SISG_EN_ENABLE);

    // Use interlocked function to enable the SISG.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    RETAILMSG(0, (_T("Enabling SISG\n")));

    return;
}


//------------------------------------------------------------------------------
//
// Function: DisableSISG
//
// This function disables the IPU SISG module.
// If another driver is using the SISG, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableSISG()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the SISG.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_SISG_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_SISG_EN, IPU_IPU_CONF_SISG_EN_DISABLE);

    // Use interlocked function to disable SISG.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling SISG\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_SISG);
}


//------------------------------------------------------------------------------
//
// Function: EnableVDI
//
// This function enables the IPU VDI module.
// If the module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableVDI()
{
    UINT32 oldVal, newVal, iMask, iBitval;
    
    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_VDI);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the VDI.

    // If customer wants to enable VDI, ISP and IC_INPUT also need to be enabled.
    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_ISP_EN)
            |CSP_BITFMASK(IPU_IPU_CONF_VDI_EN)
            |CSP_BITFMASK(IPU_IPU_CONF_IC_INPUT);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_ISP_EN, IPU_IPU_CONF_ISP_EN_ENABLE)
            |CSP_BITFVAL(IPU_IPU_CONF_VDI_EN, IPU_IPU_CONF_VDI_EN_ENABLE)
            |CSP_BITFVAL(IPU_IPU_CONF_IC_INPUT, IPU_IPU_CONF_IC_INPUT_ISP);


    // Use interlocked function to enable the VDI.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    RETAILMSG(0, (_T("Enabling VDI\n")));

    return;
}


//------------------------------------------------------------------------------
//
// Function: DisableVDI
//
// This function disables the IPU VDI module.
// If another driver is using the VDI, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableVDI()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the VDI.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_ISP_EN)
            |CSP_BITFMASK(IPU_IPU_CONF_VDI_EN)
            |CSP_BITFMASK(IPU_IPU_CONF_IC_INPUT);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_ISP_EN, IPU_IPU_CONF_ISP_EN_DISABLE)
            |CSP_BITFVAL(IPU_IPU_CONF_VDI_EN, IPU_IPU_CONF_VDI_EN_DISABLE)
            |CSP_BITFVAL(IPU_IPU_CONF_IC_INPUT, IPU_IPU_CONF_IC_INPUT_CSI);

    // Use interlocked function to disable VDI.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF,
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling VDI\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_VDI);
}

//------------------------------------------------------------------------------
//
// Function: EnableHSPClock
//
// This function enables the IPU HSP clock.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void EnableHSPClock()
{
    // We treat a direct request to enable the HSP clock as if
    // we are enabling an IPU submodule.  If another submodule
    // is already enabled, the clocks are already on, and no 
    // action will be taken

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_DIRECT_REQUEST);

    DEBUGMSG(ZONE_INFO, (_T("Enabling HSP Clocks\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableHSPClock
//
// This function disables the IPU HSP clock.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DisableHSPClock()
{
    // We treat a direct request to disable the HSP clock as if
    // we are disabling a special IPU submodule.  If another submodule
    // remains enabled, the HSP clock will remain on after this call.  Only if
    // all IPU submodules are disabled will this call result in the HSP clocks
    // being disabled.

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_DIRECT_REQUEST);

    DEBUGMSG(ZONE_INFO, (_T("Disabling HSP Clocks\n")));

    return;
}


//------------------------------------------------------------------------------
//
// Function: IPUAllocateBuffer
//
// This function allocates a new IpuBuffer object.
//
// Parameters:
//      pIPUBufInfo
//          [in] Structure containing info for allocating IPU buffers.
//
//      pIPUBuffer
//          [in] IPU buffers class pointer.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IPUAllocateBuffer(pIPUBufferInfo pIPUBufInfo, IpuBuffer* pIPUBuffer)
{
    new(pIPUBuffer) IpuBuffer(pIPUBufInfo->dwBufferSize, pIPUBufInfo->MemType);
    return;
}


//------------------------------------------------------------------------------
//
// Function: IPUDeallocateBuffer
//
// This function deletes an IpuBuffer object.
//
// Parameters:
//      pIPUBuffer
//          [in] IPU buffers class pointer.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IPUDeallocateBuffer(IpuBuffer* pIPUBuffer)
{
    delete pIPUBuffer;
}


//------------------------------------------------------------------------------
//
// Function: IpuIntrThread
//
// This function is the IPU IST thread.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void IpuIntrThread(LPVOID lpParameter)
{
    IPU_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParameter);

    IpuISRLoop(INFINITE);

    IPU_FUNCTION_EXIT();
}



//-----------------------------------------------------------------------------
//
// Function: IpuISRLoop
//
// This function is the interrupt handler for the IPU events.
// It waits for an interrupt from the IPU (most commonly an IDMAC
// End-Of-Frame (EOF) interrupt), and signals the appropriate
// event associated with the interrupt.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void IpuISRLoop(UINT32 timeout)
{
    IPU_FUNCTION_ENTRY();

    // loop here
    for (;;)
    {
        DEBUGMSG (ZONE_INFO, (TEXT("%s: In the loop\r\n"), __WFUNCTION__));

        if (WaitForSingleObject(g_hIpuIntrEvent, timeout) == WAIT_OBJECT_0)
        {
            DEBUGMSG (ZONE_INFO, (TEXT("%s: IPU Sync Interrupt received\r\n"), __WFUNCTION__));

            // We have 3 phases of interrupt handling:
            // 1) Checking for IDMAC-related interrupts
            // 2) Checking for non-IDMAC-related interrupts (i.e., INT_REG_15)
            // 3) Checking for Error interrupts (i.e., INT_REG_10)

            HandleIDMACInterrupts();

            HandleNonIDMACInterrupts();

            HandleErrorInterrupts();

            // Kernel call to unmask the interrupt so that it can be signalled again
            InterruptDone(g_ipuIntr);
        }
        else
        {
            DEBUGMSG (ZONE_INFO, (TEXT("%s: Time out\r\n"), __WFUNCTION__));
        }

    }

    return;
}


//-----------------------------------------------------------------------------
//
// Function: HandleIDMACInterrupts
//
// This function is the interrupt handler for the IDMAC IPU events.
// It signals the appropriate event associated with the interrupt, and
// clears the interrupt control bit.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void HandleIDMACInterrupts()
{
    DWORD ctrlReg, statReg, clearCtrlBits, int_src;

    for (int i = 1; i <= 14; i += 2)
    {
        // Skip 9 (no IDMAC channel 9) and skip 10 (10 is error
        // channel, not sync channel)
        if (i == 9)
        {
             continue;
        }

        // We will check two registers at a time, since the 52 IDMAC channels
        // are distributed over 2 registers, for each type of interrupt
        // (e.g., EOF, NFACK)

        // First, check the lower registers (for IDMAC channels 11-31).
        // We perform the following steps in a loop:
        //      1) Read control and status for the lower registers
        //      2) If an interrupt is signalled, clear control bits
        //      3) Signal the interrupt event
        //      4) Exit loop when no more interrupts are left to handle
        do
        {
            ctrlReg = GetControlReg(i);
            statReg = GetStatusReg(i);

            clearCtrlBits = 0;

            int_src = 31 - _CountLeadingZeros(statReg & ctrlReg);

            // If interrupt source is valid
            if (int_src < IPU_INT_MAX_ID)
            {

                // Clear interrupt control bits so we do not
                // continue to receive this interrupt.  The signalled
                // driver is responsible for re-enabling these bits.
                clearCtrlBits = g_pIntrGroupMask[0][int_src];

                // Use interlocked function to update control registers
                ClearControlReg(i, clearCtrlBits);

                // Now, check to see if we need to clear bits in the upper
                // control registers
                clearCtrlBits = g_pUpperRegsMask[int_src];
                if (clearCtrlBits != 0)
                {
                    ClearControlReg(i + 1, clearCtrlBits);
                }

                // If client event installed
                if (g_phIntrHdlr[0][int_src] != NULL)
                {
                    if(int_src == 0)
                    {
                        RETAILMSG(0,(TEXT("%s:interrup for int_src :%x \r\n"),__WFUNCTION__));
                    }

                    // Signal registered event
                    // Note:  Rescheduling may occur and could result in more
                    //        interrupts pending.
                    SetEvent(g_phIntrHdlr[0][int_src]);
                }
                else
                {
                    DEBUGMSG(ZONE_ERROR, (_T("No IPU submodule event registered for interrupt source = %d\r\n"), int_src));
                }

           }
        }while (_CountLeadingZeros(statReg & ctrlReg) != 32);

        do
        {
            ctrlReg = GetControlReg(i + 1);
            statReg = GetStatusReg(i + 1);

            clearCtrlBits = 0;

            int_src = 31 - _CountLeadingZeros(statReg & ctrlReg);

            // If interrupt source is valid
            if (int_src < IPU_INT_MAX_ID)
            {

                // Clear interrupt control bits so we do not
                // continue to receive this interrupt.  The signalled
                // driver is responsible for re-enabling these bits.
                clearCtrlBits = g_pIntrGroupMask[1][int_src];

                // Use interlocked function to update control registers
                ClearControlReg(i + 1, clearCtrlBits);

                // If client event installed
                if (g_phIntrHdlr[1][int_src] != NULL)
                {
                    // Signal registered event
                    // Note:  Rescheduling may occur and could result in more
                    //        interrupts pending.
                    SetEvent(g_phIntrHdlr[1][int_src]);
                }
                else
                {
                    DEBUGMSG(ZONE_ERROR, (_T("No IPU submodule event registered for interrupt source = %d\r\n"), int_src));
                }

           }
       }while (_CountLeadingZeros(statReg & ctrlReg) != 32);

   }

}


//-----------------------------------------------------------------------------
//
// Function: HandleNonIDMACInterrupts
//
// This function is the interrupt handler for the IDMAC IPU events.
// It signals the appropriate event associated with the interrupt, and
// clears the interrupt control bit.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void HandleNonIDMACInterrupts()
{
    DWORD ctrlReg, statReg, clearCtrlBits;

    //-----------------------------------
    // Handle non-IDMAC status interrupts
    //-----------------------------------
    statReg = INREG32(&g_pIPU->IPU_INT_STAT_15);
    ctrlReg = INREG32(&g_pIPU->IPU_INT_CTRL_15);

    if (statReg & ctrlReg & SNOOPING_INTR_MASK)
    {
        // Clear interrupt control bits so we do not
        // continue to receive this interrupt.  The signalled
        // driver is responsible for re-enabling these bits.
        clearCtrlBits = SNOOPING_INTR_MASK;

        ClearControlReg(15, clearCtrlBits);

        SetEvent(g_hSnoopingIntrEvent);
    }

    if (statReg & ctrlReg & DP_FLOW_INTR_MASK)
    {
        // Clear interrupt control bits so we do not
        // continue to receive this interrupt.  The signalled
        // driver is responsible for re-enabling these bits.
        clearCtrlBits = DP_FLOW_INTR_MASK;

        ClearControlReg(15, clearCtrlBits);

        SetEvent(g_hDPFlowIntrEvent);
    }

    if (statReg & ctrlReg & DC_FRAME_COMPLETE_INTR_MASK)
    {
        // Clear interrupt control bits so we do not
        // continue to receive this interrupt.  The signalled
        // driver is responsible for re-enabling these bits.
        clearCtrlBits = DC_FRAME_COMPLETE_INTR_MASK;

        ClearControlReg(15, clearCtrlBits);

        SetEvent(g_hDCFrameCompleteIntrEvent);
    }


    if (statReg & ctrlReg & DI_VSYNC_PRE_INTR_MASK)
    {
        // Clear interrupt control bits so we do not
        // continue to receive this interrupt.  The signalled
        // driver is responsible for re-enabling these bits.
        clearCtrlBits = DI_VSYNC_PRE_INTR_MASK;

        ClearControlReg(15, clearCtrlBits);

        SetEvent(g_hDIVSyncPreIntrEvent);
    }

    if (statReg & ctrlReg & DI_COUNTER_INTR_MASK)
    {
        // Clear interrupt control bits so we do not
        // continue to receive this interrupt.  The signalled
        // driver is responsible for re-enabling these bits.
        clearCtrlBits = DI_COUNTER_INTR_MASK;

        ClearControlReg(15, clearCtrlBits);

        SetEvent(g_hDICounterIntrEvent);
    }

}



//-----------------------------------------------------------------------------
//
// Function: HandleErrorInterrupts
//
// This function is the interrupt handler for the IDMAC IPU events.
// It signals the appropriate event associated with the interrupt, and
// clears the interrupt control bit.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void HandleErrorInterrupts()
{
    DWORD ctrlReg, statReg, clearCtrlBits;

    //-----------------------------------
    // Handle error interrupts
    //-----------------------------------
    statReg = INREG32(&g_pIPU->IPU_INT_STAT_10);
    ctrlReg = INREG32(&g_pIPU->IPU_INT_CTRL_10);

    if (statReg & ctrlReg & DC_TEARING_ERROR_INTR_MASK)
    {
        // Clear interrupt control bits so we do not
        // continue to receive this interrupt.  The signalled
        // driver is responsible for re-enabling these bits.
        clearCtrlBits = DC_TEARING_ERROR_INTR_MASK;

        ClearControlReg(10, clearCtrlBits);

        SetEvent(g_hDCTearingErrorIntrEvent);
    }

    if (statReg & ctrlReg & DI_SYNC_ERROR_INTR_MASK)
    {
        // Clear interrupt control bits so we do not
        // continue to receive this interrupt.  The signalled
        // driver is responsible for re-enabling these bits.
        clearCtrlBits = DI_SYNC_ERROR_INTR_MASK;

        ClearControlReg(10, clearCtrlBits);

        SetEvent(g_hDISyncErrorIntrEvent);
    }

    if (statReg & ctrlReg & DI_TIMEOUT_ERROR_INTR_MASK)
    {
        // Clear interrupt control bits so we do not
        // continue to receive this interrupt.  The signalled
        // driver is responsible for re-enabling these bits.
        clearCtrlBits = DI_TIMEOUT_ERROR_INTR_MASK;

        ClearControlReg(10, clearCtrlBits);

        SetEvent(g_hDITimeoutErrorIntrEvent);
    }

    if (statReg & ctrlReg & IC_FRAME_LOST_ERROR_INTR_MASK)
    {
        // Clear interrupt control bits so we do not
        // continue to receive this interrupt.  The signalled
        // driver is responsible for re-enabling these bits.
        clearCtrlBits = IC_FRAME_LOST_ERROR_INTR_MASK;

        ClearControlReg(10, clearCtrlBits);

        SetEvent(g_hICFrameLostErrorIntrEvent);
    }

    if (statReg & ctrlReg & IC_AXI_ERROR_INTR_MASK)
    {
        // Clear interrupt control bits so we do not
        // continue to receive this interrupt.  The signalled
        // driver is responsible for re-enabling these bits.
        clearCtrlBits = IC_AXI_ERROR_INTR_MASK;

        ClearControlReg(10, clearCtrlBits);

        SetEvent(g_hAXIErrorIntrEvent);
    }
}

UINT32 GetControlReg(int idmacChNum)
{
    UINT32 ctrlRegVal;

    switch (idmacChNum)
    {
        case 1:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_1);
            break;
        case 2:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_2);
            break;
        case 3:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_3);
            break;
        case 4:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_4);
            break;
        case 5:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_5);
            break;
        case 6:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_6);
            break;
        case 7:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_7);
            break;
        case 8:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_8);
            break;
        case 10:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_10);
            break;
        case 11:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_11);
            break;
        case 12:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_12);
            break;
        case 13:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_13);
            break;
        case 14:
            ctrlRegVal = INREG32(&g_pIPU->IPU_INT_CTRL_14);
            break;
        default:
            ctrlRegVal = 0;
    }

    return ctrlRegVal;
}

UINT32 GetStatusReg(int idmacChNum)
{
    UINT32 statRegVal;

    switch (idmacChNum)
    {
        case 1:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_1);
            break;
        case 2:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_2);
            break;
        case 3:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_3);
            break;
        case 4:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_4);
            break;
        case 5:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_5);
            break;
        case 6:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_6);
            break;
        case 7:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_7);
            break;
        case 8:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_8);
            break;
        case 10:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_10);
            break;
        case 11:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_11);
            break;
        case 12:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_12);
            break;
        case 13:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_13);
            break;
        case 14:
            statRegVal = INREG32(&g_pIPU->IPU_INT_STAT_14);
            break;
        default:
            statRegVal = 0;
    }

    return statRegVal;
}

void ClearControlReg(int idmacChNum, DWORD clearBitMask)
{
    UINT32 oldCtrl, newCtrl;

    // Use interlocked function to update control registers
    switch (idmacChNum)
    {
        case 1:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_1);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_1,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 2:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_2);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_2,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 3:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_3);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_3,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 4:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_4);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_4,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 5:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_5);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_5,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 6:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_6);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_6,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 7:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_7);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_7,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 8:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_8);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_8,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 10:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_10);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_10,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 11:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_11);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_11,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 12:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_12);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_12,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 13:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_13);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_13,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 14:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_14);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_14,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
        case 15:
            do
            {
                oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_15);
                newCtrl = oldCtrl & ~clearBitMask;
            } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_15,
                       oldCtrl, newCtrl) != oldCtrl);
            break;
    }
}

