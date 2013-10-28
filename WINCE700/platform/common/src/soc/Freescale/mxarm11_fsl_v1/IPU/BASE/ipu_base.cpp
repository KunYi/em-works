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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
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
#include <NKIntr.h>
#pragma warning(pop)

#include "mxarm11.h"

#include "Ipu.h"



//------------------------------------------------------------------------------
// External Functions
extern "C" DWORD IPUGetIRQ();
extern "C" BOOL BSPIPUSetClockGatingMode(DWORD dwIPUEnableMask);

//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines

// Masks for IPU modules
#define IPU_MODULE_CSI          (CSP_BITFMASK(IPU_IPU_CONF_CSI_EN))
#define IPU_MODULE_IC           (CSP_BITFMASK(IPU_IPU_CONF_IC_EN))
#define IPU_MODULE_PF           (CSP_BITFMASK(IPU_IPU_CONF_PF_EN))
#define IPU_MODULE_SDC          (CSP_BITFMASK(IPU_IPU_CONF_SDC_EN))
#define IPU_MODULE_ADC          (CSP_BITFMASK(IPU_IPU_CONF_ADC_EN))
#define IPU_MODULE_DI           (CSP_BITFMASK(IPU_IPU_CONF_DI_EN))


// Macros for generating 64-bit IRQ masks
#define IPU_INTMASK(intr) (((ULONG) 1)  << intr)

// Masks for IPU submodule interrupt groups
#define PRP_ENC_DMA_CHA_MASK   IPU_INTMASK(IPU_INT_DMAIC_0)

#define PRP_VF_DMA_CHA_MASK    IPU_INTMASK(IPU_INT_DMAIC_1) | IPU_INTMASK(IPU_INT_DMAIC_3)

#define PRP_ENC_ROT_DMA_CHA_MASK IPU_INTMASK(IPU_INT_DMAIC_8) | IPU_INTMASK(IPU_INT_DMAIC_10)

#define PRP_VF_ROT_DMA_CHA_MASK  IPU_INTMASK(IPU_INT_DMAIC_9) | IPU_INTMASK(IPU_INT_DMAIC_11)

#define PP_DMA_CHA_MASK        IPU_INTMASK(IPU_INT_DMAIC_2) | IPU_INTMASK(IPU_INT_DMAIC_4) | \
                               IPU_INTMASK(IPU_INT_DMAIC_5)

#define PP_ROT_DMA_CHA_MASK    IPU_INTMASK(IPU_INT_DMAIC_12) | IPU_INTMASK(IPU_INT_DMAIC_13)

#define SDC_BG_DMA_CHA_MASK    IPU_INTMASK(IPU_INT_DMASDC_0)

#define SDC_FG_DMA_CHA_MASK    IPU_INTMASK(IPU_INT_DMASDC_1)

#define ADC_DMA_CHA_MASK       IPU_INTMASK(IPU_INT_DMAADC_2) | IPU_INTMASK(IPU_INT_DMAADC_3) | \
                               IPU_INTMASK(IPU_INT_DMAADC_4) | IPU_INTMASK(IPU_INT_DMAADC_5) | \
                               IPU_INTMASK(IPU_INT_DMAADC_6) | IPU_INTMASK(IPU_INT_DMAADC_7)

#define PF_DMA_CHA_MASK        IPU_INTMASK(IPU_INT_DMAPF_0) | IPU_INTMASK(IPU_INT_DMAPF_1) | \
                               IPU_INTMASK(IPU_INT_DMAPF_2) | IPU_INTMASK(IPU_INT_DMAPF_3) | \
                               IPU_INTMASK(IPU_INT_DMAPF_4) | IPU_INTMASK(IPU_INT_DMAPF_5) | \
                               IPU_INTMASK(IPU_INT_DMAPF_6) | IPU_INTMASK(IPU_INT_DMAPF_7)

#define IPU_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define IPU_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNCTION        3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNCTION      (1 << ZONEID_FUNCTION)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNCTION          DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)

DBGPARAM dpCurSettings = {
    _T("IPU"),
    {
        TEXT("Errors"), TEXT("Warnings"), TEXT("Init"), TEXT("Func"),
        TEXT("Info"), TEXT(""), TEXT(""), TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT("")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN // ulZoneMask
//0xffff
};

#endif  // DEBUG


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
PCSP_IPU_REGS g_pIPU;
HANDLE g_hIpuIntrEvent;
HANDLE g_hPrpIntrEvent;
HANDLE g_hPpIntrEvent;
HANDLE g_hPfIntrEvent;
HANDLE g_hSDCBGIntrEvent;
HANDLE g_hSDCFGIntrEvent;
HANDLE g_hADCIntrEvent;
DWORD g_ipuIntr;
HANDLE g_hIpuISRThread;
static HANDLE g_phIntrHdlr[IPU_INT_MAX_ID];
static DWORD g_pIntrGroupMask[IPU_INT_MAX_ID];
static BOOL g_bPPEnable;
static BOOL g_bPRPEnable;
static BOOL g_bSDCEnable;
static BOOL g_bADCEnable;
static DWORD g_dwIPUEnableMask;
static CRITICAL_SECTION g_csIPUEnableMask;
static HANDLE g_hDdkClkMutex;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
static BOOL InitIPU(void);
static void DeinitIPU(void);
static BOOL IpuAlloc(void);
static void IpuDealloc(void);
static BOOL IpuIntrInit(void);
static void IpuIntrThread(LPVOID);
static void IpuISRLoop(UINT32);
static void IpuModuleEnable(DWORD);
static void IpuModuleDisable(DWORD);
static void EnableIC(IPU_DRIVER);
static void DisableIC(IPU_DRIVER);
static void EnableCSI();
static void DisableCSI();
static void EnableSDC();
static void DisableSDC();
static void EnableADC();
static void DisableADC();
static void EnableDI(IPU_DRIVER);
static void DisableDI(IPU_DRIVER);
static void EnablePF();
static void DisablePF();

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the IPU common module.  This
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
//      TRUE if the PMIC is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
extern "C"
BOOL WINAPI DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER((HMODULE)hInstDll);
        DEBUGMSG(ZONE_INIT, (_T("***** DLL PROCESS ATTACH TO IPU_BASE DLL *****\r\n")));
        DisableThreadLibraryCalls((HMODULE) hInstDll);
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    // return TRUE for success
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  Init
//
// This function initializes the IPU Base common component.  Called by the
// Device Manager to initialize a device.
//
//      pContext
//          [in] Pointer to a string containing the registry path to the
//          active key for the stream interface driver.
//
// Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//-----------------------------------------------------------------------------
DWORD IPU_Init(LPCTSTR pContext, DWORD dwBusContext)
{
    DWORD rc = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(dwBusContext);

    if (!InitIPU())
    {
        DEBUGMSG(ZONE_INIT, (_T("InitIPU failed!")));
        goto cleanUp;
    }

    rc = 1;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  Deinit
//
// This function deinitializes the IPU Base common component.  Called by the 
// Device Manager to de-initialize a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL IPU_Deinit(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);

    DeinitIPU();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: Open
//
// This function opens a device for reading, writing, or both.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context. The XXX_Init function creates
//          and returns this handle.
//
//      AccessCode
//          [in] Access code for the device. The access is a combination of
//          read and write access from CreateFile.
//
//      ShareMode
//          [in] File share mode of the device. The share mode is a
//          combination of read and write access sharing from CreateFile.
//
// Returns:
//      This function returns a handle that identifies the open context of
//      the device to the calling application.
//
//------------------------------------------------------------------------------
DWORD IPU_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    IPU_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    IPU_FUNCTION_EXIT();
    return hDeviceContext;
}


//------------------------------------------------------------------------------
//
// Function: Close
//
// This function closes the PP for reading and writing.
//
// Parameters:
//      hOpenContext
//          [in] Handle returned by the XXX_Open function, used to identify
//          the open context of the device.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL IPU_Close(DWORD hOpenContext)
{
    IPU_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    IPU_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: IOControl
//
// This function sends a command to a device.
//
// Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//          function creates and returns this identifier.
//
//      dwCode
//          [in] I/O control operation to perform. These codes are
//          device-specific and are usually exposed to developers through
//          a header file.
//
//      pBufIn
//          [in] Pointer to the buffer containing data to transfer to the
//          device.
//
//      dwLenIn
//          [in] Number of bytes of data in the buffer specified for pBufIn.
//
//      pBufOut
//          [out] Pointer to the buffer used to transfer the output data
//          from the device.
//
//      dwLenOut
//          [in] Maximum number of bytes in the buffer specified by pBufOut.
//
//      pdwActualOut
//          [out] Pointer to the DWORD buffer that this function uses to
//          return the actual number of bytes received from the device.
//
// Returns:
//      The new data pointer for the device indicates success. A value of -1
//      indicates failure.
//
//------------------------------------------------------------------------------
BOOL IPU_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    BOOL bRet = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pdwActualOut);

    switch(dwCode)
    {
        case IPU_IOCTL_ENABLE_IC:
            EnableIC(*(IPU_DRIVER *)pBufIn);
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ENABLE_IC occurred\r\n")));
            break;

        case IPU_IOCTL_DISABLE_IC:
            DisableIC(*(IPU_DRIVER *)pBufIn);
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_DISABLE_IC occurred\r\n")));
            break;

        case IPU_IOCTL_ENABLE_CSI:
            EnableCSI();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ENABLE_CSI occurred\r\n")));
            break;

        case IPU_IOCTL_DISABLE_CSI:
            DisableCSI();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_DISABLE_CSI occurred\r\n")));
            break;

        case IPU_IOCTL_ENABLE_SDC:
            EnableSDC();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ENABLE_SDC occurred\r\n")));
            break;

        case IPU_IOCTL_DISABLE_SDC:
            DisableSDC();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_DISABLE_SDC occurred\r\n")));
            break;

        case IPU_IOCTL_ENABLE_ADC:
            EnableADC();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ENABLE_ADC occurred\r\n")));
            break;

        case IPU_IOCTL_DISABLE_ADC:
            DisableADC();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_DISABLE_ADC occurred\r\n")));
            break;

        case IPU_IOCTL_ENABLE_DI:
            EnableDI(*(IPU_DRIVER *)pBufIn);
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ENABLE_DI occurred\r\n")));
            break;

        case IPU_IOCTL_DISABLE_DI:
            DisableDI(*(IPU_DRIVER *)pBufIn);
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_DISABLE_DI occurred\r\n")));
            break;

        case IPU_IOCTL_ENABLE_PF:
            EnablePF();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ENABLE_PF occurred\r\n")));
            break;

        case IPU_IOCTL_DISABLE_PF:
            DisablePF();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_DISABLE_PF occurred\r\n")));
            break;

        default:
            DEBUGMSG(ZONE_WARN, (TEXT("IPU_IOControl: No matching IOCTL.\r\n")));
            break;
    }

    return bRet;
}

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
static BOOL InitIPU(void)
{
    IPU_FUNCTION_ENTRY();

    g_bPPEnable = FALSE;
    g_bPRPEnable = FALSE;
    g_bADCEnable = FALSE;
    g_bSDCEnable = FALSE;

    // IPU modules start disabled
    g_dwIPUEnableMask = 0;

    // Initialize lock for IPUEnableMask
    InitializeCriticalSection(&g_csIPUEnableMask);

    // Allocate IPU driver data structures.
    if (!IpuAlloc())
    {
        DEBUGMSG(ZONE_ERROR, (_T("IPU common Init:  Allocation failed!\r\n")));
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
    OUTREG32(&g_pIPU->IPU_IMA_ADDR, 0);

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
//      none.
//------------------------------------------------------------------------------
static void DeinitIPU(void)
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
static BOOL IpuAlloc(void)
{

    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if (g_pIPU == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_IPU;

        // Map peripheral physical address to virtual address
        g_pIPU = (PCSP_IPU_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPU == NULL)
        {
            DEBUGMSG(ZONE_ERROR,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }


    if (g_hDdkClkMutex == NULL)
    {
        // Create mutex to sync with CSPDDK clock control
        g_hDdkClkMutex = CreateMutex(NULL, FALSE, L"MUTEX_DDKCLK");

        // Check if mutex creation failed
        if (g_hDdkClkMutex == NULL)
        {
            DEBUGMSG(ZONE_ERROR,
                (_T("Init:  CreateMutex failed!\r\n")));
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
static void IpuDealloc(void)
{
    IPU_FUNCTION_ENTRY();

    // Unmap peripheral registers
    if (g_pIPU)
    {
        MmUnmapIoSpace(g_pIPU, sizeof(CSP_IPU_REGS));
        g_pIPU = NULL;
    }

    // Unmap CSPDDK clock control mutex
    if (g_hDdkClkMutex)
    {
        CloseHandle(g_hDdkClkMutex);
        g_hDdkClkMutex = NULL;
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
static BOOL IpuIntrInit(void)
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
            (TEXT("%s: Request SYSINTR failed (IPU Interrupt)\r\n"), __WFUNCTION__));
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

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for Post-filtering Interrupt event\r\n"), __WFUNCTION__));
    g_hPfIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_PF_INTR_EVENT);
    if (g_hPfIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for Post-filtering Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for SDC Background plane Interrupt event\r\n"), __WFUNCTION__));
    g_hSDCBGIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_SDC_BG_INTR_EVENT);
    if (g_hSDCBGIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for SDC Background plane Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for SDC Foreground plane Interrupt event\r\n"), __WFUNCTION__));
    g_hSDCFGIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_SDC_FG_INTR_EVENT);
    if (g_hSDCFGIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for SDC Foreground plane Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("%s: CreateEvent for ADC Interrupt event\r\n"), __WFUNCTION__));
    g_hADCIntrEvent = CreateEvent(NULL, FALSE, FALSE, IPU_ADC_INTR_EVENT);
    if (g_hADCIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent for ADC Interrupt Event failed\r\n"), __WFUNCTION__));
        return ret;
    }

    g_phIntrHdlr[IPU_INT_DMAIC_0] = g_hPrpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_1] = g_hPrpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_2] = g_hPpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_3] = g_hPrpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_4] = g_hPpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_5] = g_hPpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_6] = g_hPrpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_7] = g_hPrpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_8] = g_hPrpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_9] = g_hPrpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_10] = g_hPrpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_11] = g_hPrpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_12] = g_hPpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAIC_13] = g_hPpIntrEvent;
    g_phIntrHdlr[IPU_INT_DMASDC_0] = g_hSDCBGIntrEvent;
    g_phIntrHdlr[IPU_INT_DMASDC_1] = g_hSDCFGIntrEvent;
    g_phIntrHdlr[IPU_INT_DMASDC_2] = 0;
    g_phIntrHdlr[IPU_INT_DMASDC_3] = 0;
    g_phIntrHdlr[IPU_INT_DMAADC_2] = g_hADCIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAADC_3] = g_hADCIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAADC_4] = g_hADCIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAADC_5] = g_hADCIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAADC_6] = g_hADCIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAADC_7] = g_hADCIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAPF_0] = g_hPfIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAPF_1] = g_hPfIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAPF_2] = g_hPfIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAPF_3] = g_hPfIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAPF_4] = g_hPfIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAPF_5] = g_hPfIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAPF_6] = g_hPfIntrEvent;
    g_phIntrHdlr[IPU_INT_DMAPF_7] = g_hPfIntrEvent;

    g_pIntrGroupMask[IPU_INT_DMAIC_0] = PRP_ENC_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAIC_1] = PRP_VF_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAIC_2] = PP_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAIC_3] = PRP_VF_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAIC_4] = PP_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAIC_5] = PP_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAIC_6] = 0;
    g_pIntrGroupMask[IPU_INT_DMAIC_7] = 0;
    g_pIntrGroupMask[IPU_INT_DMAIC_8] = PRP_ENC_ROT_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAIC_9] = PRP_VF_ROT_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAIC_10] = PRP_ENC_ROT_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAIC_11] = PRP_VF_ROT_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAIC_12] = PP_ROT_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAIC_13] = PP_ROT_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMASDC_0] = SDC_BG_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMASDC_1] = SDC_FG_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMASDC_2] = 0;
    g_pIntrGroupMask[IPU_INT_DMASDC_3] = 0;
    g_pIntrGroupMask[IPU_INT_DMAADC_2] = ADC_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAADC_3] = ADC_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAADC_4] = ADC_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAADC_5] = ADC_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAADC_6] = ADC_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAADC_7] = ADC_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAPF_0] = PF_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAPF_1] = PF_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAPF_2] = PF_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAPF_3] = PF_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAPF_4] = PF_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAPF_5] = PF_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAPF_6] = PF_DMA_CHA_MASK;
    g_pIntrGroupMask[IPU_INT_DMAPF_7] = PF_DMA_CHA_MASK;

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
    
    // If all IPU modules are disabled, disable IPU clocks.
    if (g_dwIPUEnableMask == 0)
    {
        DEBUGMSG(ZONE_INFO, (_T("Disabling IPU Clocks\n")));

        // Clear all Buffer Ready bits (by writing all 0's to the register)
        OUTREG32(&g_pIPU->IPU_CHA_BUF0_RDY, 0);
        OUTREG32(&g_pIPU->IPU_CHA_BUF1_RDY, 0);
    }

    LeaveCriticalSection(&g_csIPUEnableMask);

    return;
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
static void EnableIC(IPU_DRIVER driver)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (driver == IPU_DRIVER_PP)
    {
        g_bPPEnable = TRUE;

        // If PRP has already enabled IC, we are done
        if (g_bPRPEnable)
        {
            return;
        }
    }
    else if (driver == IPU_DRIVER_PRP)
    {
        g_bPRPEnable = TRUE;

        // If PP has already enabled IC, we are done
        if (g_bPPEnable)
        {
            return;
        }
    }

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_IC);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the IC and the rotation unit.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_IC_EN)
        | CSP_BITFMASK(IPU_IPU_CONF_ROT_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_IC_EN, IPU_IPU_CONF_IC_EN_ENABLE)
        | CSP_BITFVAL(IPU_IPU_CONF_ROT_EN, IPU_IPU_CONF_ROT_EN_ENABLE);

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
//          Post-Processor (PP) is enabling the IC.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void DisableIC(IPU_DRIVER driver)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (driver == IPU_DRIVER_PP)
    {
        g_bPPEnable = FALSE;

        // If PRP still has IC enabled, leave it enabled and return.
        if (g_bPRPEnable)
        {
            return;
        }
    }
    else if (driver == IPU_DRIVER_PRP)
    {
        g_bPRPEnable = FALSE;

        // If PP still has IC enabled, leave it enabled and return.
        if (g_bPPEnable)
        {
            return;
        }
    }
    // Protect access to IPU_CONF register.
    // Set the bit to enable the IC and the rotation unit.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_IC_EN)
        | CSP_BITFMASK(IPU_IPU_CONF_ROT_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_IC_EN, IPU_IPU_CONF_IC_EN_DISABLE)
        | CSP_BITFVAL(IPU_IPU_CONF_ROT_EN, IPU_IPU_CONF_ROT_EN_DISABLE);

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
// Function: EnableCSI
//
// This function enables the IPU CMOS Sensor Interface (CSI) modules.
// If this module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void EnableCSI()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_CSI);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the CSI.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_CSI_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_CSI_EN, IPU_IPU_CONF_CSI_EN_ENABLE);

    // Use interlocked function to enable the CSI.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF, 
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling CSI\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableCSI
//
// This function disables the IPU CMOS Sensor Interface (CSI) modules.
// If another driver is using the CSI, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void DisableCSI()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the CSI.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_CSI_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_CSI_EN, IPU_IPU_CONF_CSI_EN_DISABLE);

    // Use interlocked function to disable the CSI.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF, 
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling CSI\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_CSI);
}



//------------------------------------------------------------------------------
//
// Function: EnableSDC
//
// This function enables the IPU Synchronous Display Controller (SDC) module.
// If this module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void EnableSDC()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_SDC);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the SDC.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_SDC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_SDC_EN, IPU_IPU_CONF_SDC_EN_ENABLE);

    // Use interlocked function to enable the SDC.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF, 
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling SDC\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableSDC
//
// This function disables the Synchronous Display Controller (SDC) module.
// If another driver is using the SDC, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void DisableSDC()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the SDC.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_SDC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_SDC_EN, IPU_IPU_CONF_SDC_EN_DISABLE);

    // Use interlocked function to disable SDC.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF, 
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling SDC\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_SDC);
}

//------------------------------------------------------------------------------
//
// Function: EnableADC
//
// This function enables the IPU Asynchronous Display Controller (ADC) module.
// If this module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void EnableADC()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_ADC);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the ADC.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_ADC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_ADC_EN, IPU_IPU_CONF_ADC_EN_ENABLE);

    // Use interlocked function to enable the SDC.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF, 
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling ADC\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableADC
//
// This function disables the Asynchronous Display Controller (ADC) module.
// If another driver is using the ADC, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void DisableADC()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the ADC.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_ADC_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_ADC_EN, IPU_IPU_CONF_ADC_EN_DISABLE);

    // Use interlocked function to disable ADC.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF, 
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling ADC\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_ADC);
}



//------------------------------------------------------------------------------
//
// Function: EnableDI
//
// This function enables the IPU Display Interface (DI) module.
// If the module is already enabled, no action is taken.
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
static void EnableDI(IPU_DRIVER driver)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (driver == IPU_DRIVER_SDC)
    {
        g_bSDCEnable = TRUE;

        // If ADC has already enabled DI, we are done
        if (g_bADCEnable)
        {
            return;
        }
    }
    else if (driver == IPU_DRIVER_ADC)
    {
        g_bADCEnable = TRUE;

        // If SDC has already enabled DI, we are done
        if (g_bSDCEnable)
        {
            return;
        }
    }

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_DI);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DI.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DI_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DI_EN, IPU_IPU_CONF_DI_EN_ENABLE);

    // Enable of DI must be synchronized with DVFC driver.  Acquire the
    // CSPDDK clock control mutex to prevent a hazard that can occur 
    // while the DVFC driver is scaling the HSP divider.
    WaitForSingleObject(g_hDdkClkMutex, INFINITE);
    
    // Use interlocked function to enable the DI.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF, 
                oldVal, newVal) != oldVal);

    ReleaseMutex(g_hDdkClkMutex);


    DEBUGMSG(ZONE_INFO, (_T("Enabling DI\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisableDI
//
// This function disables the IPU Display Interface (DI) module.
// If another driver is using the DI, it will remain enabled.
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
static void DisableDI(IPU_DRIVER driver)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (driver == IPU_DRIVER_SDC)
    {
        g_bSDCEnable = FALSE;

        // If ADC still has DI enabled, leave it enabled and return.
        if (g_bADCEnable)
        {
            return;
        }
    }
    else if (driver == IPU_DRIVER_ADC)
    {
        g_bADCEnable = FALSE;

        // If PP still has DI enabled, leave it enabled and return.
        if (g_bSDCEnable)
        {
            return;
        }
    }

    // Protect access to IPU_CONF register.
    // Set the bit to enable the DI.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_DI_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_DI_EN, IPU_IPU_CONF_DI_EN_DISABLE);

    // Disable of DI must be synchronized with DVFC driver.  Acquire the
    // CSPDDK clock control mutex to prevent a hazard that can occur 
    // while the DVFC driver is scaling the HSP divider.
    WaitForSingleObject(g_hDdkClkMutex, INFINITE);

    // Use interlocked function to disable DI.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF, 
                oldVal, newVal) != oldVal);

    ReleaseMutex(g_hDdkClkMutex);
    
    DEBUGMSG(ZONE_INFO, (_T("Disabling DI\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_DI);
}


//------------------------------------------------------------------------------
//
// Function: EnablePF
//
// This function enables the IPU Postfilter (PF) module.
// If the module is already enabled, no action is taken.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void EnablePF()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Enable IPU clocks if necessary
    IpuModuleEnable(IPU_MODULE_PF);

    // Protect access to IPU_CONF register.
    // Set the bit to enable the PF.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_PF_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_PF_EN, IPU_IPU_CONF_PF_EN_ENABLE);

    // Use interlocked function to enable the PF.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF, 
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Enabling PF\n")));

    return;
}

//------------------------------------------------------------------------------
//
// Function: DisablePF
//
// This function disables the IPU Postfilter (PF) module.
// If another driver is using the DI, it will remain enabled.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void DisablePF()
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // Protect access to IPU_CONF register.
    // Set the bit to enable the PF.

    // Compute bitmask and shifted bit value for IPU_CONF register
    iMask = CSP_BITFMASK(IPU_IPU_CONF_PF_EN);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_PF_EN, IPU_IPU_CONF_PF_EN_DISABLE);

    // Use interlocked function to disable PF.
    do
    {
        oldVal = INREG32(&g_pIPU->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPU->IPU_CONF, 
                oldVal, newVal) != oldVal);

    DEBUGMSG(ZONE_INFO, (_T("Disabling PF\n")));

    // Disable IPU clocks if necessary
    IpuModuleDisable(IPU_MODULE_PF);
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
static void IpuIntrThread(LPVOID lpParameter)
{
    IPU_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParameter);

    IpuISRLoop(INFINITE);

    // Commenting out to prevent Warning C4702 (unreachable code) - W4
    //IPU_FUNCTION_EXIT();
}



//-----------------------------------------------------------------------------
//
// Function: IpuISRLoop
//
// This function is the interrupt handler for the Preprocessor.
// It waits for the End-Of-Frame (EOF) interrupt, and signals
// the EOF event registered by the user of the preprocessor.
//
// Parameters:
//      timeout
//          [in] Timeout value while waiting for EOF interrupt.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
static void IpuISRLoop(UINT32 timeout)
{
    DWORD ctrlReg1, statReg1, ctrlReg3, statReg3, statReg5, clearCtrlBits, int_src;
    UINT32 oldCtrl, newCtrl;

    IPU_FUNCTION_ENTRY();

    // loop here
    for (;;)
    {
        DEBUGMSG (ZONE_INFO, (TEXT("%s: In the loop\r\n"), __WFUNCTION__));

        if (WaitForSingleObject(g_hIpuIntrEvent, timeout) == WAIT_OBJECT_0)
        {
            DEBUGMSG (ZONE_INFO, (TEXT("%s: Interrupt received\r\n"), __WFUNCTION__));

            ctrlReg1 = INREG32(&g_pIPU->IPU_INT_CTRL_1);
            statReg1 = INREG32(&g_pIPU->IPU_INT_STAT_1);
            ctrlReg3 = INREG32(&g_pIPU->IPU_INT_CTRL_3);
            statReg3 = INREG32(&g_pIPU->IPU_INT_STAT_3);
            statReg5 = INREG32(&g_pIPU->IPU_INT_STAT_5);
            clearCtrlBits = 0;

            int_src = 31 - _CountLeadingZeros(statReg1 & ctrlReg1);

            // If interrupt source is valid
            if (int_src < IPU_INT_MAX_ID)
            {

                // Clear interrupt control bits so we do not
                // continue to receive this interrupt.  The signalled
                // driver is responsible for re-enabling these bits.
                clearCtrlBits = g_pIntrGroupMask[int_src];

                // Use interlocked function to update control registers
                do
                {
                    oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_1);
                    newCtrl = oldCtrl & ~clearCtrlBits;
                } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_1, 
                            oldCtrl, newCtrl) != oldCtrl);

                // If client event installed
                if (g_phIntrHdlr[int_src] != NULL)
                {
                    // Signal registered event 
                    // Note:  Rescheduling may occur and could result in more
                    //        interrupts pending.
                    SetEvent(g_phIntrHdlr[int_src]);
                }
                else
                {
                    DEBUGMSG(ZONE_ERROR, (_T("No IPU submodule event registered for interrupt source = %d\r\n"), int_src));
                }

            }
            else if ((ctrlReg3 & statReg3) == 0x100000)
            {
                // Special Case: ADC Direct Channel from PP
                // Clear interrupt control and signal PP

                // Use interlocked function to update control registers
                do
                {
                    oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_3);
                    newCtrl = oldCtrl & ~0x100000;
                } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_3,
                            oldCtrl, newCtrl) != oldCtrl);

                SetEvent(g_hPpIntrEvent);
            }
            else if ((ctrlReg3 & statReg3) == 0x20000)
            {
                // Special Case: We have received VSYNC for ADC
                // Clear interrupt control and signal ADC

                // Use interlocked function to update control registers
                do
                {
                    oldCtrl = INREG32(&g_pIPU->IPU_INT_CTRL_3);
                    newCtrl = oldCtrl & ~0x20000;
                } while ((UINT32) InterlockedTestExchange((LPLONG) &g_pIPU->IPU_INT_CTRL_3,
                            oldCtrl, newCtrl) != oldCtrl);

                SetEvent(g_hADCIntrEvent);
            }
            else if (_CountLeadingZeros(statReg1 & ctrlReg1) == 32)
            {
                DEBUGMSG(ZONE_INFO, (_T("No interrupt source?\r\n")));
            }
            else
            {
                DEBUGMSG(ZONE_ERROR, (_T("Invalid IPU interrupt source (%d)\r\n"), int_src));
            }

            if (statReg5 & 0xFFFF)
            {
                // TODO: Properly Handle Error Cases
                DEBUGMSG (ZONE_INFO, (TEXT("%s: Error Interrupt received!\r\n"), __WFUNCTION__));
                if (INREG32(&g_pIPU->IPU_INT_STAT_5) & 0x3800)
                {
                    DEBUGMSG (ZONE_INFO, (TEXT("%s: Frame Lost.\r\n"), __WFUNCTION__));
                    // Clear frame drop interrupt registers
                    OUTREG32(&g_pIPU->IPU_INT_STAT_5, 0x3800);
                    DEBUGMSG (ZONE_INFO, (TEXT("%s: Cleared INT_STAT_5: %x\r\n"),
                            __WFUNCTION__, INREG32(&g_pIPU->IPU_INT_STAT_5)));
                }
                else
                {
                    DEBUGMSG (ZONE_INFO, (TEXT("%s: Other error.\r\n"), __WFUNCTION__));
                }
            }

            // Kernel call to unmask the interrupt so that it can be signalled again
            InterruptDone(g_ipuIntr);
        }
        else
        {
            DEBUGMSG (ZONE_INFO, (TEXT("%s: Time out\r\n"), __WFUNCTION__));
        }

    }

    // Commenting out to prevent Warning C4702 (unreachable code) - W4
    //IPU_FUNCTION_EXIT();
}

