//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------ 
//------------------------------------------------------------------------------
//
//  File: registry.c
//
//  This file contains the functions USB Funcation PDD OS layer.

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <cebus.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <celog.h>
#include <oal.h>
#pragma warning(pop)

#pragma warning(disable: 4100)

#include "csp.h"
#include "usbd.h"

#include <common_usbname.h>
#include <common_usbcommon.h>
#include <common_usbfnioctl.h>

#pragma warning(disable: 4053)
//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
#ifdef DEBUG

#define ZONE_ERROR              DEBUGZONE(0)
#define ZONE_WARNING            DEBUGZONE(1)
#define ZONE_TRANSFER           DEBUGZONE(3)
#define ZONE_INTERRUPTS         DEBUGZONE(8)
#define ZONE_POWER              DEBUGZONE(9)
#define ZONE_FUNCTION           DEBUGZONE(12)
#define ZONE_PDD                DEBUGZONE(13)

extern DBGPARAM dpCurSettings = {
    L"UsbFn", {
            L"Error",       L"Warning",     L"<unused>",     L"<unused>",
            L"<unused>",    L"<unused>",    L"<unused>",     L"<unused>",
            L"Interrupts",  L"Power",       L"<unused>",     L"<unused>",
            L"Function",    L"PDD",    L"<unused>",     L"<unused>"
    },
    0x003 // Default to ZONE_ERROR | ZONE_WARNING
};

#endif
extern CRITICAL_SECTION g_csRegister;

#include "oscheckkitl.c"

extern void DumpUSBRegs(PUCHAR baseMem);
extern void BSPUsbPhyExit(void);

USBFN_PDD * FslUfnGetUsbFnPdd()
{
    return LocalAlloc(LPTR, sizeof(USBFN_PDD));
}

DWORD FslUfnRequestIrq(USBFN_PDD *pPdd)
{
    INT32 aIrqs[3];
    DWORD rc = ERROR_INVALID_PARAMETER;

    aIrqs[0] = -1;
#ifdef GENERAL_OTG
    // We now have 2 Sets of OTG mechanism
    // 1. traditional i.MX implementaion, where XVC, UFN and HCD driver are working simultaneously, blocked each other
    // by blocking mutex events. In this case we should use "OAL_INTR_TRANSLATE" to map USB IRQ to same SYSINTR in different module
    // 2. more general MSFT otg implementaion, where OTG driver is the father of UFN and HCD. The latter 2 are loaded by OTG dirver
    // and don't work simultaneously. In this implementaion a chain interrupt mechanism is used. Via giisr.dll, OTG driver only keeps
    // monitoring OTGSC related interrupt. If a USB IRQ happens not because of OTGSC, the SYSINTR is generated in a chainning method.
    // In this case we should use "OAL_INTR_FORCE_STATIC" in order for UFN/HCD getting different SYSINTR number so that interrupt chainning
    // can be acheieved through OTG and UFN/HCD
    // Select either "OAL_INTR_FORCE_STATIC" or "OAL_INTR_TRANSLATE" don't have impact on pure UFN/HCD configuration
    //
    // "GENERAL_OTG" is defined only in porject where the 2nd OTG mechanism is used

    aIrqs[1] = OAL_INTR_FORCE_STATIC;
#else
    aIrqs[1] = OAL_INTR_TRANSLATE;
#endif
    aIrqs[2] = pPdd->irq;  //retrived via "GetDeviceRegistryParams"
    KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, aIrqs, sizeof(aIrqs), &pPdd->sysIntr, sizeof(pPdd->sysIntr), NULL);

    // Create interrupt event
    pPdd->hIntrEvent = CreateEvent(0, FALSE, FALSE, NULL);   // manual_reset (false), ini_value (false)

    if (pPdd->hIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Error creating interrupt "
                              L"event\r\n"));
        goto clean;
    }

    // Initialize interrupt
    if (!InterruptInitialize(pPdd->sysIntr, pPdd->hIntrEvent, NULL, 0))
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Interrupt initialization "
                              L"failed\r\n"));
        goto clean;
    }

    DEBUGMSG(ZONE_INTERRUPTS, (TEXT("USBFNPDD SysIntr:0x%x, Irq:0x%x\r\n"),
                               pPdd->sysIntr, pPdd->irq));
    return ERROR_SUCCESS;

clean:
    return rc ;

}

void FslUfnGetDMABuffer(USBFN_PDD *pPdd)
{
    pPdd->qhbuffer = (USBFN_QH_BUF_T*)AllocPhysMem(sizeof(USBFN_QH_BUF_T),     // (2 * epNum Qh) + (2 * epNum Td) + (epNum prime flag)
        PAGE_READWRITE|PAGE_NOCACHE,
        0,        // is_this_correct                   // 4k alignment
        0,    // Reserved
        &pPdd->qhbuf_phy);

    // Allocate uncached memory to handle SDMA Cache allignment issue
    memset(&(pPdd->Adapter1), 0, sizeof(DMA_ADAPTER_OBJECT));
    pPdd->Adapter1.InterfaceType = Internal;
    pPdd->Adapter1.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    // allocate DMA compatible memory

    pPdd->Buffer_Td1 = HalAllocateCommonBuffer(
                        &(pPdd->Adapter1),          // [I] object
                        MAX_SIZE_PER_BP,            // [I] size
                        &(pPdd->phyAddr_TD1),       // [O] return logical start address of allocated buffer
                        FALSE);                     // [I] ignored, always uncached

    if( pPdd->Buffer_Td1 == NULL)
    {
        DEBUGMSG(ZONE_PDD, (TEXT("Allocate DMA memory for Buffer_Td1 failed\r\n")));
    }

    memset(&(pPdd->Adapter2), 0, sizeof(DMA_ADAPTER_OBJECT));
    pPdd->Adapter2.InterfaceType = Internal;
    pPdd->Adapter2.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    pPdd->Buffer_Td2 = HalAllocateCommonBuffer(
                        &(pPdd->Adapter2),
                        MAX_SIZE_PER_BP,
                        &(pPdd->phyAddr_TD2),
                        FALSE);

    if(pPdd->Buffer_Td2 == NULL)
    {
        DEBUGMSG(ZONE_PDD, (TEXT("Allocate DMA memory for Buffer_Td2 failed\r\n")));
    }
}

DWORD FslUfnCreateThread(USBFN_PDD *pPdd)
{
    pPdd->exitIntrThread = FALSE;
    pPdd->hIntrThread = CreateThread(NULL, 0, InterruptThread, pPdd, 0, NULL);
    if (pPdd->hIntrThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Interrupt thread "
                              L"creation failed\r\n"));
        return ERROR_INVALID_PARAMETER;
    }
    
    CeSetThreadPriority( pPdd->hIntrThread, pPdd->priority256 ) ;
    return ERROR_SUCCESS;

}

void FslUfnTrigIrq(USBFN_PDD *pPdd)
{
    SetEvent(pPdd->hIntrEvent);
}


void FslUfnDeinit(USBFN_PDD *pPdd)
{
    // Stop interrupt thread
    if (pPdd->hIntrThread != NULL)
    {
        pPdd->exitIntrThread = TRUE;
        FslUfnTrigIrq(pPdd);
        WaitForSingleObject(pPdd->hIntrThread, INFINITE);
        CloseHandle(pPdd->hIntrThread);
    }

    // disable Intr
    InterruptDisable(pPdd->sysIntr);
    // release irq 
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pPdd->sysIntr,
                    sizeof(pPdd->sysIntr), NULL, 0, NULL);


    // release resources in USBPHY module
    BSPUsbPhyExit();

    // Close interrupt handler
    if (pPdd->hIntrEvent != NULL)
    {
        CloseHandle(pPdd->hIntrEvent);
        pPdd->hIntrEvent = NULL;
    }

    // If parent bus is open, set hardware to D4 and close it
    if (pPdd->hParentBus != NULL)
    {
        SetDevicePowerState(pPdd->hParentBus, D4, NULL);
        CloseBusAccessHandle(pPdd->hParentBus);
        pPdd->hParentBus = NULL;
    }

    // Unmap USBD controller registers
    if (pPdd->pUSBDRegs != NULL)
    {
        MmUnmapIoSpace((VOID*)pPdd->pUSBDRegs, pPdd->memLen);
        pPdd->pUSBDRegs = NULL;
    }

    // Release interrupt
    if (pPdd->sysIntr != 0)
    {
        InterruptDisable(pPdd->sysIntr);
        // KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR,
        //                 &pPdd->sysIntr,
        //                 sizeof(pPdd->sysIntr),
        //                 NULL, 0, NULL
        //                );
        pPdd->sysIntr = 0;
    }

    // Delete critical section
    DeleteCriticalSection(&pPdd->epCS);

    DeleteCriticalSection(&g_csRegister);

    BSPUSBClockDeleteFileMapping();

    if ( pPdd->qhbuffer )
    {
        FreePhysMem(pPdd->qhbuffer);
        pPdd->qhbuffer = NULL;
    }

    HalFreeCommonBuffer(&(pPdd->Adapter1),
                        MAX_SIZE_PER_BP,
                        pPdd->phyAddr_TD1,
                        pPdd->Buffer_Td1,
                        FALSE);

    HalFreeCommonBuffer(&(pPdd->Adapter2),
                        MAX_SIZE_PER_BP,
                        pPdd->phyAddr_TD2,
                        pPdd->Buffer_Td2,
                        FALSE);

    // Free PDD context
    LocalFree(pPdd);
}

//------------------------------------------------------------------------------
//
//  Function:  InterruptThread
//
//  This is interrupt thread. It controls responsed to hardware interrupt. To
//  reduce code length it calls interrupt specific functions.
//
//  Parameter:
//
//      pPddContext - Pointer to USBFN_PDD
//
//  Return:
//
//      ERROR_SUCCESS
//
//------------------------------------------------------------------------------

DWORD InterruptThread(VOID *pPddContext)
{
    USBFN_PDD *pPdd = pPddContext;
    CSP_USB_REGS* pUSBDRegs = pPdd->pUSBDRegs;
    USB_USBSTS_T source;
    DWORD *temp;
    int i;
    HANDLE hFunction = NULL, hXcvr = NULL;
    ULONG WaitReturn;
    TCHAR szUSBFunctionObjectName[30];
    TCHAR szUSBXcvrObjectName[30];
    DWORD timeout = IDLE_TIMEOUT; // 3 sec
    DWORD StringSize;

    DEBUGMSG(ZONE_INTERRUPTS, (TEXT("IsOTGSupport? %d\r\n"), pPdd->IsOTGSupport));
    if (pPdd->IsOTGSupport)
    {
        StringSize = sizeof(szUSBFunctionObjectName) / sizeof(TCHAR);
        StringCchCopy(szUSBFunctionObjectName,StringSize,USBFunctionObjectName);
        StringCchCat(szUSBFunctionObjectName, StringSize, pPdd->szOTGGroup);

        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("USBFN: CreateEvent:%s\r\n"), szUSBFunctionObjectName));
        hFunction = CreateEvent(NULL, FALSE, FALSE, szUSBFunctionObjectName);
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Opened an existing Func Event\r\n")));
        }
        else
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Created a new Func Event\r\n")));
        }

        if (hFunction == NULL)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Create Event Failed for func!\r\n")));
        }

        StringSize = sizeof(szUSBXcvrObjectName) / sizeof(TCHAR);
        StringCchCopy(szUSBXcvrObjectName,StringSize,USBXcvrObjectName);
        StringCchCat(szUSBXcvrObjectName, StringSize, pPdd->szOTGGroup);
        
        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("USBFN: CreateEvent:%s\r\n"), szUSBXcvrObjectName));

        hXcvr = CreateEvent(NULL, FALSE, FALSE, szUSBXcvrObjectName);

        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Opened an existing XCVR Event\r\n")));
        }
        else
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Created a new XCVR Event\r\n")));
        }

        if (hXcvr == NULL)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Create Event Failed for xcvr!\r\n")));
        }

XCVR_SIG:
        pPdd->bInUSBFN = FALSE;
        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Waiting for signal from XCVR!!!\r\n")));
        WaitReturn = WaitForSingleObject(hFunction, INFINITE);               // OTG will block here, until XVC send signal

        pPdd->bInUSBFN = TRUE;

        //RETAILMSG(1, (TEXT("UFN: Function Driver in charge now!!!\r\n")));
        if (!InterruptInitialize(pPdd->sysIntr, pPdd->hIntrEvent, NULL, 0)) 
        {
            RETAILMSG(1, (L"ERROR: InterruptThread: Interrupt initialization failed\r\n"));
            return FALSE;
        }

        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UfnPdd_Start called\r\n")));
        UfnPdd_Start(pPdd);

        // Move this part to here so that it would only handle in case
        // of OTG support
        {
            USB_OTGSC_T otgsc_temp;
            DWORD       *t = (DWORD *)&otgsc_temp;

            *t = INREG32(&pUSBDRegs->OTG.OTGSC);
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("OTGSC IDIE(0x%x), IDIS(0x%x), (0x%x)\r\n"), otgsc_temp.IDIE, otgsc_temp.IDIS, *t));
            otgsc_temp.IDIE = 1;
            OUTREG32(&pUSBDRegs->OTG.OTGSC, *t);  //IDIE is enabled here
        }

        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("USBFNPDD SysIntr:0x%x, Irq:0x%x\r\n"), pPdd->sysIntr, pPdd->irq));

        // Handling of device attach
        temp=(DWORD *)&source;
        *temp = INREG32(&pUSBDRegs->OTG.USBSTS);
        // Clear source bit
        {
            do
            {
                if (INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT) & 1)
                {
                    SetupEvent(pPdd);
                }

                if (INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE))
                {
                    for (i=0;i<USBD_EP_COUNT;i++)
                    {
                        CheckEndpoint(pPdd, i);
                    }
                }
            } while (INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT) ||
                     INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE));
        }
    }
#if 0 
    /*This delay has moved to just before setup notification for suspend - resume operation*/
    {
        #define SLEEP_MACRO 2000
        RETAILMSG(1, (L"Sleep %d ms for ActiveSync Coldboot\r\n", SLEEP_MACRO));
        Sleep(2000);
    }
#endif
    // we must add a delay here, to make sure potential mass storage device
    // can be mounted.
    Sleep(1000);
    while (!pPdd->exitIntrThread)  // This is set in 2 place
                                   // 1. DeInit
                                   // 2. TestMode
    {
        DWORD dwErr = 0;
        DWORD ret ;

        if (pPdd->IsOTGSupport)
        {
            timeout = INFINITE;        // For non-otg, the timeout is always 3s, power consumption?
        }

        // Wait for interrupt from USB controller
        dwErr = WaitForSingleObject(pPdd->hIntrEvent, timeout);

        ret = InterruptHandle(pPdd, dwErr, &timeout);
#ifdef USBCV_FIX
        InterruptDone(pPdd->sysIntr);
#endif
        if( ret == IRQHANDLE_CONTINUE )
            continue;
        else if(ret == IRQHANDLE_BREAK)
        {
            //RETAILMSG(1, (TEXT("IRQHANDLE_BREAK\r\n")));
            break;
        }
        else if(ret == IRQHANDLE_GOTO_XVC)
        {
            InterruptDisable(pPdd->sysIntr);
            // wait for client function class and make sure everything is
            // disconnected this is not a very good design but since we
            // need to make sure we can get proper disconnect before we
            // switch back to XVR, we have to do that especially on
            DEBUGMSG(ZONE_FUNCTION, (TEXT("Leave Client back to XVR mode\r\n")));
            Sleep(500);
            SetEvent(hXcvr);
            goto XCVR_SIG;
        }
    }

    if (pPdd->bEnterTestMode)
    {
        RETAILMSG(1, (TEXT("USBOTG Client Test Mode set and loop forever\r\n")));
        pPdd->bEnterTestMode = FALSE;
        for (;;);
    }

    //Must add ExitThread to release signal
    ExitThread(0);
    return ERROR_SUCCESS;
}

extern DWORD FslUfnGetPageNumber(void *p, size_t size)
{
    return ADDRESS_AND_SIZE_TO_SPAN_PAGES(p,size);
}


DWORD FslUfnGetPageSize()
{
    return UserKInfo[KINX_PAGESIZE];
}


DWORD FslUfnGetPageShift()
{
    //RETAILMSG(1, (_T("KINX_PFN_SHIFT %d\n"),UserKInfo[KINX_PFN_SHIFT]));
    return UserKInfo[KINX_PFN_SHIFT];
}


void   FslInitializeCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
)
{
    InitializeCriticalSection(lpCriticalSection);
    return;
}

void FslEnterCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
)
{
    EnterCriticalSection(lpCriticalSection);
    return;
}

void  FslLeaveCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
)
{
    LeaveCriticalSection(lpCriticalSection);
    return;
}

VOID FslNKDbgPrintfW(LPCWSTR format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    NKvDbgPrintfW(format, arglist);
    va_end(arglist);
}
