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
/*---------------------------------------------------------------------------
* Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
*--------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
//
//  File:  sdcontrol.cpp
//
//  Implementation of SDHC common Device Driver
//
//------------------------------------------------------------------------------

// Disable W4 warnings in header files that are outside the scope of this platform
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "mxarm11.h"
#include "sdhc.h"

#pragma warning(push)
#pragma warning(disable: 4100 4127 4189 4201 4512)
#pragma warning(disable: 4245)  // public\common\oak\inc\cmthread.h(118): C4245: conversion from 'int' to 'DWORD', signed/unsigned mismatch
#include "sdbus.hpp"
#pragma warning(pop)

/*******************************************************************************
 GLOBAL OR STATIC VARIABLES
*******************************************************************************/
/*******************************************************************************
 STATIC FUNCTION PROTOTYPES
*******************************************************************************/
static DWORD SDControllerIstThread(PSDH_HARDWARE_CONTEXT pHCDevice);
static SD_API_STATUS SDControllerBusyResponse(PSDH_HARDWARE_CONTEXT pHCDevice,
                                              PSD_BUS_REQUEST pRequest);
static void IndicateBusRequestComplete(PSDH_HARDWARE_CONTEXT pController,
                                       PSD_BUS_REQUEST pRequest,
                                       SD_API_STATUS status);
static void HandleCommandComplete(PSDH_HARDWARE_CONTEXT pController,
                                           PSD_BUS_REQUEST pRequest);
static void HandlePIODataTransfer(PSDH_HARDWARE_CONTEXT pController,
                                  PSD_BUS_REQUEST pRequest);
static void HandlePIOAlignedDataTransfer(PSDH_HARDWARE_CONTEXT pController,
                                         PSD_BUS_REQUEST pRequest);
static void HandleTransferDone(PSDH_HARDWARE_CONTEXT pController,
                               PSD_BUS_REQUEST pRequest);
static void WaitforFifoReady(PSDH_HARDWARE_CONTEXT pController,
                             PSD_BUS_REQUEST pRequest);
static void SetRate(PSDH_HARDWARE_CONTEXT pHc, PDWORD pRate, BOOL fSetNewRate);
static void SetClock(PSDH_HARDWARE_CONTEXT pController, BOOL Start) ;
static void SoftwareReset(PSDH_HARDWARE_CONTEXT pController, BOOL bResetClock);
static void SetHardwarePowerState(PSDH_HARDWARE_CONTEXT pHc, CEDEVICE_POWER_STATE ds);
static void ProcessCardInsertion(void *pContext);
static void ProcessCardRemoval(void *pContext);
static void InitGlobals(PSDH_HARDWARE_CONTEXT pController);
static inline BOOL TransferIsSDIOAbort(PSD_BUS_REQUEST pRequest);
static BOOL InitDMA(PSDH_HARDWARE_CONTEXT pController) ;
static BOOL DeInitDMA(PSDH_HARDWARE_CONTEXT pController) ;
static BOOL SetupCardPresenceDetect(PSDH_HARDWARE_CONTEXT pHardwareContext) ;
static void SDCardPresenceDetect(void *pContext);
static void CleanupCardPresenceDetect(PSDH_HARDWARE_CONTEXT pHardwareContext);
#if 0 // Remove-W4: Warning C4505 workaround
static void DumpRegisters(PSDH_HARDWARE_CONTEXT pHc);
#endif

/*******************************************************************************
 EXPORTED FUNCTIONS
*******************************************************************************/
//------------------------------------------------------------------------------
//
// Function: SDInitialize
//
// Initialize the the MMC Controller
//
// Parameters:
//       pHardwareContext[in] - newly allocated hardware context
//
// Returns:
//      SD_API_STATUS code
//
//------------------------------------------------------------------------------
SD_API_STATUS SDInitialize(PSDCARD_HC_CONTEXT pHCContext)
{
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;   // intermediate status
    PHYSICAL_ADDRESS phyAddr;
    PSDH_HARDWARE_CONTEXT pHardwareContext;       // hardware context
    pHardwareContext = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+SDInitialize: Initialize the SDHC\r\n")));
    InitializeCriticalSection(&pHardwareContext->ControllerCriticalSection);

    // Init  globals
    InitGlobals(pHardwareContext);

    if (pHardwareContext->ControllerIndex == 1)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SDInitialize: Controller 1\r\n")));
        //Virtual memory mapping
        phyAddr.QuadPart = CSP_BASE_REG_PA_SDHC1 ;
    }
    else if (pHardwareContext->ControllerIndex == 2)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SDInitialize: Controller 2\r\n")));
        //Virtual memory mapping
        phyAddr.QuadPart = CSP_BASE_REG_PA_SDHC2 ;
    }
    else
    {
        ERRORMSG(ZONE_ERROR, (TEXT("SDInitialize:  We only have 2 controllers!!\r\n")));
        goto exitInit;
    }

#if !VPMX31
    //Start SDHC BSP specific initialization
    if (!BSPSdhcInit(pHardwareContext->ControllerIndex))
    {
        ERRORMSG(ZONE_ERROR, (TEXT("BSPSdhcInit FAIL!\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }
#endif //VPMX31

    // Power off the card slot to save power
    BSPSetVoltageSlot(pHardwareContext->ControllerIndex, 0);
    // cut the clocks to the controller to save power
    BSPSdhcSetClockGatingMode(FALSE, pHardwareContext->ControllerIndex);

    //Setup IOMUX
    BspSdhcSetIOMux(pHardwareContext->ControllerIndex);

    //Get IRQ and DMA channels for SDHC
    pHardwareContext->dwIrqSDHC = BspSdhcGetIrq(pHardwareContext->ControllerIndex);
    pHardwareContext->DmaReqTx  = (DDK_DMA_REQ)BspSdhcGetSdmaChannelTx(pHardwareContext->ControllerIndex);
    pHardwareContext->DmaReqRx  = (DDK_DMA_REQ)BspSdhcGetSdmaChannelRx(pHardwareContext->ControllerIndex);

    // Map Virtual Address
    pHardwareContext->pSDMMCRegisters = (PCSP_SDHC_REG) MmMapIoSpace(phyAddr, sizeof(CSP_SDHC_REG), FALSE);
    if (pHardwareContext->pSDMMCRegisters == NULL)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("SDInitialize:  MmMapIoSpace failed!\r\n")));
        goto exitInit;
    }

    if (BspSdhcIsSdmaSupported(pHardwareContext->ControllerIndex) == TRUE)
    {
        // Initialize DMA
        if (!InitDMA(pHardwareContext))
        {
            ERRORMSG(ZONE_ERROR, (TEXT("MMCSD_DMAInit: cannot init DMA!\r\n")));
            status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
            goto exitInit;
        }
    }
    else
    {
        DEBUGMSG(ZONE_INFO, (TEXT("SDHC: DMA Support Disabled for Controller %d\r\n"), pHardwareContext->ControllerIndex));
    }

    // controller clock gating between cmds is supported only for memory cards. This decision is taken later.
    pHardwareContext->fClockGatingSupported = FALSE;
    pHardwareContext->fClockGatedOff = TRUE; // controller clocks are not enabled here

    // Setup card detection mechanism
    if (!SetupCardPresenceDetect(pHardwareContext))
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    //Create interrupt event
    pHardwareContext->hControllerInterruptEvent = CreateEvent(NULL, FALSE, FALSE,NULL);
    if (NULL == pHardwareContext->hControllerInterruptEvent)
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    //driver not shutting down
    pHardwareContext->DriverShutdown = FALSE;

    // Initialize SDControllerIstThread() to handle interrupt generated by Host Controller
    DWORD threadID;
    // create the interrupt thread for controller interrupts
    pHardwareContext->hControllerInterruptThread = CreateThread(NULL,
                                                                0,
                                                                (LPTHREAD_START_ROUTINE)SDControllerIstThread,
                                                                pHardwareContext,
                                                                0,
                                                                &threadID);

    if (NULL == pHardwareContext->hControllerInterruptThread)
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    // Translate IRQ to SYSINTR
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &(pHardwareContext->dwIrqSDHC),
                         sizeof(pHardwareContext->dwIrqSDHC), &(pHardwareContext->dwSysintrSDHC),
                         sizeof(pHardwareContext->dwSysintrSDHC), NULL))
    {
        ERRORMSG(ZONE_ERROR,(TEXT("%s: Request SYSINTR failed\r\n"), __WFUNCTION__));
        goto exitInit;
    }

    //Initialize interrupt
    if (!InterruptInitialize(pHardwareContext->dwSysintrSDHC,
                             pHardwareContext->hControllerInterruptEvent, NULL, 0))
    {
        ERRORMSG(ZONE_ERROR,(TEXT("%s: Interrupt initialization failed! \r\n"), __WFUNCTION__));
        goto exitInit;
    }


    exitInit:
    if (!SD_API_SUCCESS(status))
    {
        // just call the deinit handler directly to cleanup
        SDDeinitialize(pHCContext);
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-SDInitialize: Initialize the the MMC Controller\r\n")));
    return status;
}
//------------------------------------------------------------------------------
//
// Function: SDDeInitialize
//
// Deinitialize the the MMC Controller
//
// Parameters:
//       pHCContext[in] - Host controller context
//
// Returns:
//      SD_API_STATUS code
//
//------------------------------------------------------------------------------
SD_API_STATUS SDDeinitialize(PSDCARD_HC_CONTEXT pHCContext)
{
    PSDH_HARDWARE_CONTEXT pHardwareContext; // hardware context
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+SDDeinitialize: \r\n")));
    pHardwareContext = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    // make sure all interrupt sources are disabled
    InterruptDisable(pHardwareContext->dwSysintrSDHC);

    // Indicate shutdown to threads
    pHardwareContext->DriverShutdown = TRUE;

    // clean up controller IST
    if (NULL != pHardwareContext->hControllerInterruptThread)
    {
        // wake up the IST
        SetEvent(pHardwareContext->hControllerInterruptEvent);
        WaitForSingleObject(pHardwareContext->hControllerInterruptThread, INFINITE);
        CloseHandle(pHardwareContext->hControllerInterruptThread);
        pHardwareContext->hControllerInterruptThread = NULL;
    }
    PSD_BUS_REQUEST pRequest = SDHCDGetAndLockCurrentRequest(pHCContext, 0);

    if (pRequest != NULL)
    {
        DEBUGMSG(ZONE_INFO,
                 (TEXT("SDDeinitialize - Canceling current request: 0x%08X, command: %d\n"),
                  pRequest, pRequest->CommandCode));
    // for SD HIVE
    #ifndef BSP_HIVE_SDHC
        IndicateBusRequestComplete(pHardwareContext, pRequest, SD_API_STATUS_SHUT_DOWN);
    #endif
    }

    // clean up card detection IST and free card insertion interrupt
    CleanupCardPresenceDetect(pHardwareContext);

    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &(pHardwareContext->dwSysintrSDHC),
                    sizeof(DWORD), NULL, 0, NULL);
    pHardwareContext->dwSysintrSDHC = (DWORD)SYSINTR_UNDEFINED;

    // free controller interrupt event
    if (NULL != pHardwareContext->hControllerInterruptEvent)
    {
        CloseHandle(pHardwareContext->hControllerInterruptEvent);
        pHardwareContext->hControllerInterruptEvent = NULL;
    }

    if (BspSdhcIsSdmaSupported(pHardwareContext->ControllerIndex) == TRUE)
    {
        // Release DMA Resources
        DeInitDMA(pHardwareContext);
    }

    // free the virtual space allocated for SDHC memory map
    if (pHardwareContext->pSDMMCRegisters != NULL)
    {
        MmUnmapIoSpace(pHardwareContext->pSDMMCRegisters, sizeof(CSP_SDHC_REG));
        pHardwareContext->pSDMMCRegisters = NULL;
    }
    DeleteCriticalSection(&pHardwareContext->ControllerCriticalSection);

#ifndef BSP_HIVE_SDHC
    SDHCDIndicateSlotStateChange(pHCContext, 0,DeviceEjected);
#endif

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-SDDeinitialize: \r\n")));
    return SD_API_STATUS_SUCCESS;
}
//------------------------------------------------------------------------------
//
// Function: SDHCancelIoHandler
//
// io cancel handler
//
// Parameters:
//    pHostContext[in] - host controller context
//    Slot[in] - slot the request is going on
//    pRequest[in] - the request to be cancelled
//
// Returns:
//      TRUE if the request was cancelled
//
//------------------------------------------------------------------------------
BOOLEAN SDHCancelIoHandler(PSDCARD_HC_CONTEXT pHCContext,
                           DWORD              Slot,
                           PSD_BUS_REQUEST    pRequest)
{
    PSDH_HARDWARE_CONTEXT    pController;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Slot);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+SDHCancelIoHandler \n"))) ;

    // for now, we should never get here because all requests are non-cancelable
    // the hardware supports timeouts so it is impossible for the controller to get stuck
    DEBUG_ASSERT(FALSE);

    // get our extension
    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    // --- Stop hardware, cancel the request!

    // release the lock before we complete the request
    SDHCDReleaseHCLock(pHCContext);

    // complete the request with a cancelled status
    SDHCDIndicateBusRequestComplete(pHCContext,pRequest, SD_API_STATUS_CANCELED);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-SDHCancelIoHandler \n"))) ;
    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: SDHBusRequestHandler
//
// bus request handler. The request passed in is marked as uncancelable, this function
// has the option of making the outstanding request cancelable
//
// Parameters:
//    pHostContext[in] - host controller context
//    Slot[in] - slot the request is going on
//    pRequest[in] - the request to be cancelled
//
// Returns:
//      SD_API_STATUS Code
//
//------------------------------------------------------------------------------
SD_API_STATUS SDHBusRequestHandler(PSDCARD_HC_CONTEXT pHCContext,
                                   DWORD              Slot,
                                   PSD_BUS_REQUEST    pRequest)
{
    PSDH_HARDWARE_CONTEXT    pController;
    DWORD                    cmdatRegister = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Slot);

    // get our extension
    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);
    // HW access requires the use of the HostController CriticalSection
    SDHCDAcquireHCLock(pHCContext);

    DWORD  dwNumBytesToTransfer = pRequest->NumBlocks * pRequest->BlockSize;
    pController->dwNumBytesToTransfer = dwNumBytesToTransfer;
    pController->dwNumWordsToTransfer = (dwNumBytesToTransfer / 4);
    pController->dwMisalignedBytesToTransfer = (dwNumBytesToTransfer % 4);
    pController->pBuffer = pRequest->pBlockBuffer;

    // Get the Relative address from CMD 7, this is required to for CMD13 later
    if (pRequest->CommandCode == SD_CMD_SELECT_DESELECT_CARD)
        pController->RelativeAddress = pRequest->CommandArgument;
    // DMA is not used for comands
    // DMA is not used for small data transfers (DmaMinTransfer)
    // DMA is not used for transfer sizes that are not 32-bit aligned
    if ((SD_COMMAND == pRequest->TransferClass) ||
        (dwNumBytesToTransfer < (pController->DmaMinTransfer)) ||
        (dwNumBytesToTransfer & 0x3))
    {
        pController->fDMATransfer = FALSE;
    }
    // Allow platform code to specify if DMA is supported
    else
    {
        pController->fDMATransfer = BspSdhcIsSdmaSupported(pController->ControllerIndex);
    }

#ifdef SD_R1B_BUSYWAIT_WORKAROUND
    DWORD                   status;
    //check for card ready for next command
    if(pController->LastResponedR1b == ResponseR1b)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("SDHBusRequestHandler: wait for busy!\r\n")));
        status = SDControllerBusyResponse(pController, pRequest);
        if(status != SD_API_STATUS_SUCCESS)
        {
            SDHCDIndicateBusRequestComplete(pController->pHCContext, pRequest, SD_API_STATUS_DEVICE_BUSY);
            SDHCDReleaseHCLock(pHCContext);
            return SD_API_STATUS_DEVICE_BUSY;
        }
    }
#endif
    // Handle for ACMD 42/23. Our SDHC expects no reponse but spec say R1.
    //This is used during MX21 SDIO workaround. It might not be used during ARM11 hardware debugging
    if ( (pRequest->CommandCode == SD_CMD_LOCK_UNLOCK && pController->fAppCommandSent == TRUE) ||
         (pRequest->CommandCode == SD_ACMD_SET_WR_BLOCK_ERASE_COUNT && pController->fAppCommandSent == TRUE))
    {
        pRequest->CommandResponse.ResponseType = NoResponse;
    }
    pController->LastResponedR1b = pRequest->CommandResponse.ResponseType;

    //Set the response type
    switch (pRequest->CommandResponse.ResponseType)
    {
    case NoResponse:
        CSP_BITFINS(cmdatRegister, SDHC_CDC_FORMAT, 0);
        break;
        //ARM11 spec does not have busy bit register set.
        //Need to check busy bit Hw implementation and handling.This affects commands with
        //response R1b and for SDIOAbort transfers.
    case ResponseR1b:
    case ResponseR1:
    case ResponseR5:
    case ResponseR6:
    case ResponseR7:
        CSP_BITFINS(cmdatRegister, SDHC_CDC_FORMAT, 1);
        break;
    case ResponseR2:
        CSP_BITFINS(cmdatRegister, SDHC_CDC_FORMAT, 2);
        break;
    case ResponseR3:
    case ResponseR4:
        // R4 is really same as an R3 response on an MMC controller (non-CRC)
        // Note: sdbus send for R4 although specs say R3
        CSP_BITFINS(cmdatRegister, SDHC_CDC_FORMAT, 3);
        break;
    default:
        ERRORMSG(ZONE_ERROR, (TEXT("SDHBusRequestHandler failed (Invalid parameter)\n")));
        SDHCDReleaseHCLock(pHCContext);
        return SD_API_STATUS_INVALID_PARAMETER;
    }

    // check for Command Only
    if ((SD_COMMAND != pRequest->TransferClass))
    {
        // its a command with a data phase
        CSP_BITFINS(cmdatRegister, SDHC_CDC_DE, 1);
    }

    // check for write
    if (SD_WRITE == pRequest->TransferClass)
    {
        //Indicate that it is write process
        CSP_BITFINS(cmdatRegister, SDHC_CDC_WR, 1);

#ifdef SD_R1B_BUSYWAIT_WORKAROUND
        pController->CurrTransferReq = SD_WRITE;
#endif //SD_R1B_BUSYWAIT_WORKAROUND

        // If transfer can be supported by DMA
        if (pController->fDMATransfer == TRUE)
        {
            if (pController->CurrentDmaReq != pController->DmaReqTx)
            {
                //Update shared channel to TX
                DDKSdmaUpdateSharedChan(pController->ChanSDHC, pController->DmaReqTx);
                pController->CurrentDmaReq = pController->DmaReqTx ;

                // Set flag to update the DMA channel context
                pController->fDmaUpdateContext = TRUE;
            }
        }
    }
    else if (SD_READ == pRequest->TransferClass)
    {

#ifdef SD_R1B_BUSYWAIT_WORKAROUND
        pController->CurrTransferReq = SD_READ;
#endif //SD_R1B_BUSYWAIT_WORKAROUND

        // If transfer can be supported by DMA
        if (pController->fDMATransfer == TRUE)
        {

            if (pController->CurrentDmaReq != pController->DmaReqRx)
            {
                //Update shared channel to RX
                DDKSdmaUpdateSharedChan(pController->ChanSDHC, pController->DmaReqRx);
                pController->CurrentDmaReq = pController->DmaReqRx ;

                // Set flag to update the DMA channel context
                pController->fDmaUpdateContext = TRUE;
            }
        }
    }

    // Build up scatter-gather list for transfers supported by DMA
    if (pController->fDMATransfer == TRUE)
    {
        // Update DMA channel context if needed
        if (pController->fDmaUpdateContext == TRUE)
        {
            DDKSdmaInitChain(pController->ChanSDHC, pController->Bytes_in_fifo);
            pController->fDmaUpdateContext = FALSE;
        }

        // pBlockBuffer is already async'ly marshalled by SDBUS driver in WinCE 6.0, no need to map it, just use it
        LPBYTE pBuffer = pRequest->pBlockBuffer;
        if (pBuffer == NULL)
        {
            // Security violation
            ERRORMSG(ZONE_ERROR, (TEXT("SDHBusRequestHandler:  MapCaller pointer failed for block buffer\r\n")));
            SDHCDReleaseHCLock(pHCContext);
            return SD_API_STATUS_ACCESS_VIOLATION;
        }

        // Use CEDDK macro to calculate the number of physical pages spanned by the mapped buffer in virtual memory
        DWORD dwNumPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(pBuffer, dwNumBytesToTransfer);

        // Check if we have enough buffer desciptors
        if (dwNumPages > SDHC_DMA_BUF_DESC)
        {
            RETAILMSG(TRUE, (_T("WARNING: SDHBusRequestHandler:   Buffer descriptors exhausted.  Falling back to CPU transfers.\r\n")));
            pController->fDMATransfer = FALSE;
        }
        else
        {

            // Use LockPages to translate the mapped buffer in virtual memory into an array of physical pages.  These
            // physical pages will be used to build up a scatter-gather list for the DMA
            DWORD physPages[SDHC_DMA_BUF_DESC];
            int fOptions = ((SD_WRITE == pRequest->TransferClass) ? LOCKFLAG_READ : LOCKFLAG_WRITE);
            if (!LockPages(pBuffer, dwNumBytesToTransfer, physPages,  fOptions))
            {
                // Security violation
                ERRORMSG(ZONE_ERROR, (TEXT("SDHBusRequestHandler:  LockPages failed for block buffer\r\n")));
                SDHCDReleaseHCLock(pHCContext);
                return SD_API_STATUS_ACCESS_VIOLATION;
            }

            // Assume no bytes will be left stranded in the FIFO
            pController->DmaStrandedBytes = 0;

            DWORD dwTempPtr = (DWORD) pBuffer;
            DWORD dwTempLen = dwNumBytesToTransfer;
            DWORD currentDesc = 0;

            // Traverse the physical pages returned by LockPages and build up a scatter-gather list for the DMA
            for(DWORD i = 0; (i < dwNumPages) && (dwTempLen > 0) && pController->fDMATransfer; i++)
            {
                // Check for invaild physical pages
                DEBUGCHK(physPages[i] != 0);

                // Physical addresses returned from LockPages must be shifted to get start address of page
                DWORD dwPhys = physPages[i] << UserKInfo[KINX_PFN_SHIFT];

                // Offset into physical page is determined using lower bits of virtual address
                DWORD dwOffset = dwTempPtr & (UserKInfo[KINX_PAGESIZE] - 1);

                // Size for scatter-gather list item will be minimum of:
                //      - length of physical page minus buffer offset
                //      - remaining length of buffer
                DWORD dwSize = UserKInfo[KINX_PAGESIZE] - dwOffset;
                if(dwSize > dwTempLen)
                {
                    dwSize = dwTempLen;
                }

                // Calculate physical address for scatter-gather list item
                dwPhys += dwOffset;

                // Track progress of traversal through virtual buffer
                dwTempLen -= dwSize;
                dwTempPtr += dwSize;

                DWORD dwFlags;

                // If we are done creating the scatter-gather list,
                if(dwTempLen <= pController->Bytes_in_fifo)
                {
                    // calculate the non-DMA size in the last page that is DMAed.
                    DWORD uNonDMASize = dwSize % pController->Bytes_in_fifo;
                    // To avoid leaving data stranded in the SDHC FIFO, we must
                    // make the last buffer descriptor a multiple of the FIFO
                    // watermark (16 bytes for 1-bit, 64 bytes for 4-bit).
                    // We keep track of the number of bytes that would
                    // be stranded by the DMA and read them out
                    // with the CPU
                    pController->DmaStrandedBytes = dwTempLen + uNonDMASize;
                    dwSize -= uNonDMASize;
                    dwTempLen = 0;
                    dwFlags = DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP;
                }

                // Else we need more scatter-gather list entries, set the
                // buffer descriptor continue flag
                else
                {
                    dwFlags = DDK_DMA_FLAGS_CONT;
                }

                if (pRequest->TransferClass == SD_READ)
                {
                    // Reads require the DMA buffers to be cache line aligned to prevent
                    // coherency problems that may result from concurrent accesses to
                    // cache line data that is "shared" between the DMA and the CPU.
                #if 1
                    if ((dwSize & CACHE_LINE_SIZE_MASK) || (dwPhys & CACHE_LINE_SIZE_MASK))
                    {
                        pController->fDMATransfer = FALSE;
                        continue;
                    }
                #endif
                }
                else
                {
                    // Writes require the DMA buffers to be word aligned
                    if ((dwSize & 0x3) || (dwPhys & 0x3))
                    {
                        pController->fDMATransfer = FALSE;
                        continue;
                    }
                }

#ifdef SDHC_SDMA_VERBOSE
                if (SD_WRITE == pRequest->TransferClass)
                    RETAILMSG(TRUE, (_T("SD_WRITE:  DDKSdmaSetBufDesc-SG (BD[%d], flags = 0x%x, addr = 0x%x, size = %d\r\n"), currentDesc, dwFlags, dwPhys, dwSize));
                else
                    RETAILMSG(TRUE, (_T("SD_READ:  DDKSdmaSetBufDesc-SG (BD[%d], flags = 0x%x, addr = 0x%x, size = %d\r\n"), currentDesc, dwFlags, dwPhys, dwSize));
#endif //SDHC_SDMA_VERBOSE

                // Add new buffer descriptor to DMA chain
                DDKSdmaSetBufDesc(pController->ChanSDHC,
                                  currentDesc,
                                  dwFlags,
                                  dwPhys,
                                  0,
                                  DDK_DMA_ACCESS_32BIT,
                                  (UINT16) dwSize);

                ++currentDesc;

            }

            // If the transfer could not be supported by DMA
            if (pController->fDMATransfer == FALSE)
            {
                // Unlock the pages we previously locked to build the scatter-gather list
                UnlockPages(pBuffer, dwNumBytesToTransfer);
            }
            else
            {
                // Save size of the scatter-gather list we built
                pController->DmaChainSize = currentDesc;
#ifdef SDHC_SDMA_VERBOSE
                RETAILMSG(TRUE, (_T("Chain size: %d, stranded bytes %d, FIFO size = %d, transfer size = %d\r\n"),  pController->DmaChainSize, pController->DmaStrandedBytes, pController->Bytes_in_fifo, dwNumBytesToTransfer));
#endif //SDHC_SDMA_VERBOSE
            }

        }
    }

    // check to see if we need to append the 80 clocks (i.e. this is the first transaction)
    if (pController->SendInitClocks)
    {
        pController->SendInitClocks = FALSE;
        CSP_BITFINS(cmdatRegister, SDHC_CDC_INIT, 1);
    }

    // Check to see if we need to enable wide bus (4 bit) data transfer mode.
    CSP_BITFINS(cmdatRegister, SDHC_CDC_BW, pController->BusWidthSetting);

    DEBUGMSG(ZONE_INFO, (TEXT("SDHBusRequestHandler - CMDAT: 0x%08X, CMD:%d ARG:0x%08X, TxClass: %d\r\n"),
                             cmdatRegister, pRequest->CommandCode, pRequest->CommandArgument, pRequest->TransferClass));

    // Check if clock gating between commands is supported.
    if (pController->fClockGatingSupported == TRUE)
    {
        // Controller clocks needs to be enabled
        BSPSdhcSetClockGatingMode(TRUE, pController->ControllerIndex);
        // update this variable after enabling the clocks to
        // avoid possible race conditions
        pController->fClockGatedOff = FALSE; // controller clocks are enabled
    }

    // Clear all status.
    OUTREG32(&pController->pSDMMCRegisters->STATUS, 0xFFFFFFFF);

    // set the command
    INSREG32BF(&pController->pSDMMCRegisters->CMD, SDHC_CMD_CMD, pRequest->CommandCode);

    // set the argument
    OUTREG32(&pController->pSDMMCRegisters->ARG, pRequest->CommandArgument);

    switch (pRequest->TransferClass)
    {
        case SD_COMMAND:
            // No data associated with this command
            INSREG32BF(&pController->pSDMMCRegisters->BLK_LEN, SDHC_BL_BL, 0);
            INSREG32BF(&pController->pSDMMCRegisters->NOB, SDHC_NOB_NOB, 0);
            break;

        case SD_WRITE:
            // Set transfer length
            INSREG32BF(&pController->pSDMMCRegisters->BLK_LEN, SDHC_BL_BL, pRequest->BlockSize);
            // Set Number of Blocks
            INSREG32BF(&pController->pSDMMCRegisters->NOB, SDHC_NOB_NOB, pRequest->NumBlocks);
            break;

        case SD_READ:
            // Clear the read operation done flag
            pController->fDmaRdOpDone = FALSE;
            //Specify the Read Time Out value
            INSREG32BF(&pController->pSDMMCRegisters->READ_TO, SDHC_READTO_TO, pController->ulReadTimeout);
            // Set transfer length
            INSREG32BF(&pController->pSDMMCRegisters->BLK_LEN, SDHC_BL_BL, pRequest->BlockSize);
            // Set Number of Blocks
            INSREG32BF(&pController->pSDMMCRegisters->NOB, SDHC_NOB_NOB, pRequest->NumBlocks);
            break;
    }

    // write the CMDAT register
    OUTREG32(&pController->pSDMMCRegisters->CMD_DAT_CONT, cmdatRegister);

    /*
     * SDHC H/W Errata - DDTS TLSbo93469
     * Dont start the clock when Read command is issued in the CMD_DAT_CONT register
     */
    if (pRequest->TransferClass != SD_READ)
    {
        //start clock
        SetClock(pController, TRUE);
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("- SDHBusRequestHandler - Request Sent\r\n")));

    // Command transfer is done by polling, to reduce interrupts
    while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_ECR) &&
           !EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_TORESP) &&
           !EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RSPCERR))
    {
        Sleep (0);
        //It is observed that the Clock to the SD card is being disabled unexpectedly
        //at times and this causes an infinite while loop. To get out of this loop we re-enable
        //clock explicitly. This is only a workaround, a fix has to be made.
        if ( (pRequest->TransferClass != SD_READ) && (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_CBCR)))
        {
            SetClock(pController, TRUE);
            DEBUGMSG(ZONE_WARN, (TEXT("- SDHBusRequestHandler - Clock re-enabled, shouldn't have happened!\r\n")));
        }
    }

    SetEvent(pController->hControllerInterruptEvent);
    SDHCDReleaseHCLock(pHCContext);
    return SD_API_STATUS_PENDING;
}
//------------------------------------------------------------------------------
//
// Function: SDHSlotOptionHandler
//
// handler for slot option changes
//
// Parameters:
//      pHostContext[in] - host controller context
//      SlotNumber[in]   - the slot the change is being applied to
//      Option[in]       - the option code
//      pData[in]        - data associated with the option
//      OptionSize[in]   - size of option data
//
// Returns:
//      SD_API_STATUS Code
//
//------------------------------------------------------------------------------
SD_API_STATUS SDHSlotOptionHandler(PSDCARD_HC_CONTEXT    pHCContext,
                                   DWORD                 SlotNumber,
                                   SD_SLOT_OPTION_CODE   Option,
                                   PVOID                 pData,
                                   ULONG                 OptionSize)
{
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;  // status
    PSDH_HARDWARE_CONTEXT    pController;          // the controller
    PSD_HOST_BLOCK_CAPABILITY  pBlockCaps;         // queried block capabilities

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(SlotNumber);

    // get our extension
    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);
    SDHCDAcquireHCLock(pHCContext);

    switch (Option)
    {

    case SDHCDAckSDIOInterrupt:
    case SDHCDEnableSDIOInterrupts:
        if (!pData && OptionSize == 0)
        {
            DEBUGMSG(ZONE_INFO,
                     (TEXT("SDHSlotOptionHandler - called - Ack/EnableSDIOInterrupts : on slot %d\r\n"),
                      SlotNumber));

            // Check if clock gating between commands is supported
            if (pController->fClockGatingSupported == TRUE)
            {
                BSPSdhcSetClockGatingMode(TRUE, pController->ControllerIndex);

                // Enable SDIO Wakeup interrupt before cutting off the controller clock
                INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQWKP, 1);
                //Enable SDIO interrupt
                INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQ, 1);

                // update this variable before disabling the clocks to
                // avoid possible race conditions
                pController->fClockGatedOff = TRUE; // controller clocks are disabled
                SetClock(pController, FALSE);
                BSPSdhcSetClockGatingMode(FALSE, pController->ControllerIndex);
            }
            else
            {
                //Enable SDIO interrupt
                INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQ, 1);
            }
            pController->fSDIOEnabled = TRUE;
        }
        else
        {
            status = SD_API_STATUS_INVALID_PARAMETER;
        }
        break;

    case SDHCDDisableSDIOInterrupts:
        if (!pData && OptionSize == 0)
        {
            DEBUGMSG(ZONE_INFO,
                     (TEXT("SDHSlotOptionHandler - called - DisableSDIOInterrupts : on slot %d  \r\n"),
                      SlotNumber));

            // Check if clock gating between commands is supported
            if (pController->fClockGatingSupported == TRUE)
            {
                BSPSdhcSetClockGatingMode(TRUE, pController->ControllerIndex);

                // Disable SDIO interrupt
                INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQ, 0);

                pController->fClockGatedOff = TRUE; // controller clocks are disabled
                SetClock(pController, FALSE);
                BSPSdhcSetClockGatingMode(FALSE, pController->ControllerIndex);
            }
            else
            {
                // Disable SDIO interrupt
                INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQ, 0);
            }
            pController->fSDIOEnabled = FALSE;
        }
        else
        {
            status = SD_API_STATUS_INVALID_PARAMETER;
        }
        break;

    case SDHCDSetSlotInterface:
        if (pData && OptionSize == sizeof(SD_CARD_INTERFACE))
        {
            DEBUGMSG(ZONE_INFO,
                     (TEXT("SDHSlotOptionHandler - called - SetSlotInterface : Clock Setting: %d \r\n"),
                      ((PSD_CARD_INTERFACE)pData)->ClockRate));

            if (SD_INTERFACE_SD_MMC_1BIT ==
                ((PSD_CARD_INTERFACE)pData)->InterfaceMode)
            {
                DEBUGMSG(ZONE_INFO,
                         (TEXT("SDHSlotOptionHandler - called - SetSlotInterface : setting for 1 bit mode \r\n")));
                pController->f4BitMode = FALSE;
                pController->BusWidthSetting = 0;
                pController->Units_in_fifo  = 4;
                if (pController->Bytes_in_fifo  != 16)
                {
                    pController->Bytes_in_fifo  = 16 ;  //16bytes
                    pController->fDmaUpdateContext = TRUE;
                }

            } else if (SD_INTERFACE_SD_4BIT == ((PSD_CARD_INTERFACE)pData)->InterfaceMode)
            {
                DEBUGMSG(ZONE_INFO,
                         (TEXT("SDHSlotOptionHandler - called - SetSlotInterface : setting for 4 bit mode \r\n")));
                pController->f4BitMode = TRUE;
                pController->BusWidthSetting = 2;
                pController->Units_in_fifo  = 4*4;
                if (pController->Bytes_in_fifo  != 64)
                {
                    pController->Bytes_in_fifo  = 64; //64 bytes
                    pController->fDmaUpdateContext = TRUE;
                }
            }

            // Check if clock gating between commands is supported
            if (pController->fClockGatingSupported == TRUE)
            {
                // Controller clocks needs to be enabled
                BSPSdhcSetClockGatingMode(TRUE, pController->ControllerIndex);

                // set rate
                SetClock(pController, FALSE);
                SetRate(pController, &((PSD_CARD_INTERFACE)pData)->ClockRate, TRUE);

                pController->fClockGatedOff = TRUE; // controller clocks are disabled
                // disable controller clocks
                BSPSdhcSetClockGatingMode(FALSE, pController->ControllerIndex);
            }
            else
            {
                // set rate
                SetClock(pController, FALSE);
                SetRate(pController, &((PSD_CARD_INTERFACE)pData)->ClockRate, TRUE);
            }

            if (!SD_API_SUCCESS(status))
            {
                ERRORMSG(ZONE_ERROR, (TEXT("SDHSlotOptionHandler: Failed to set Card Interface\n")));
            }
        }
        else
        {
            status = SD_API_STATUS_INVALID_PARAMETER;
        }
        break;

    case SDHCDSetSlotPower:
        if (pData && OptionSize == sizeof(DWORD))
        {
            //takes a DWORD for the power bit mask
            UINT32 dwVddSettingMask = *(UINT32 *)pData;
            DEBUGMSG(ZONE_INFO,
                     (TEXT("SDHSlotOptionHandler - called - SDHCDSetSlotPower : 0x%X  \r\n"),
                      dwVddSettingMask));

            //Copy Slot Vdd voltage to hardware context. This will be used to set slot power
            //during wakeup
            pController->dwVddSettingMask = dwVddSettingMask ;

            // Vary voltage setting based on request.
            BSPSetVoltageSlot(pController->ControllerIndex, dwVddSettingMask) ;
        } else
            status = SD_API_STATUS_INVALID_PARAMETER;
        break;

    case SDHCDSetSlotPowerState:
        if (pData && OptionSize == sizeof(CEDEVICE_POWER_STATE))
        {
            CEDEVICE_POWER_STATE ps = *(CEDEVICE_POWER_STATE *)pData;
            DEBUGMSG(ZONE_INFO,
                     (TEXT("SDHSlotOptionHandler - called - SDHCDSetSlotPowerState : %d  \r\n"),
                      ps));
            SetHardwarePowerState(pController, ps);
        } else
            status = SD_API_STATUS_INVALID_PARAMETER;
        break;

    case SDHCDGetSlotPowerState:
        if (pData && OptionSize == sizeof(CEDEVICE_POWER_STATE))
        {
            *(CEDEVICE_POWER_STATE *)pData = pController->CurrentPowerState;
            DEBUGMSG(ZONE_INFO,
                     (TEXT("SDHSlotOptionHandler - called - GetSlotPower : %d  \r\n"),
                      *((CEDEVICE_POWER_STATE *)pData)));
        } else
            status = SD_API_STATUS_INVALID_PARAMETER;
        break;

    case SDHCDWakeOnSDIOInterrupts:
        if (pData && OptionSize == sizeof(BOOL))
        {
            // TODO: Enable wake on SDIO int
            BOOL fEnable = *(BOOL *)pData;
            if (fEnable)
            {
                //Indicate that SDIO wake up is supported
                pController->fWakeOnSDIOInt = TRUE;
            } else
            {
                //Indicate that SDIO wake up is not supported
                pController->fWakeOnSDIOInt = FALSE;
            }
        } else
            status = SD_API_STATUS_INVALID_PARAMETER;
        break;

    case SDHCDGetWriteProtectStatus:
        if (pData && OptionSize == sizeof(SD_CARD_INTERFACE))
        {
            DEBUGMSG(ZONE_INFO,
                     (TEXT("SDHSlotOptionHandler - called - SDHCDGetWriteProtectStatus : on slot %d  \r\n"),
                      SlotNumber));
            if ( BSPSdhcIsCardWriteProtected(pController->ControllerIndex) )
            {
                ((PSD_CARD_INTERFACE)pData)->WriteProtected = TRUE;
                DEBUGMSG(ZONE_INFO, (TEXT("SDHSlotOptionHandler - Card is write protected \r\n")));
            } else
            {
                ((PSD_CARD_INTERFACE)pData)->WriteProtected = FALSE;
                DEBUGMSG(ZONE_INFO, (TEXT("SDHSlotOptionHandler - Card is write enabled \r\n")));
            }
        } else
            status = SD_API_STATUS_INVALID_PARAMETER;
        break;

    case SDHCDQueryBlockCapability:
        if (pData && OptionSize == sizeof(SD_HOST_BLOCK_CAPABILITY))
        {
            pBlockCaps = (PSD_HOST_BLOCK_CAPABILITY)pData;
            DEBUGMSG(ZONE_INFO,
                     (TEXT("SDHSlotOptionHandler: Read Block Length: %d , Read Blocks: %d\r\n"),
                      pBlockCaps->ReadBlockSize,
                      pBlockCaps->ReadBlocks));
            DEBUGMSG(ZONE_INFO,
                     (TEXT("SDHSlotOptionHandler: Write Block Length: %d , Write Blocks: %d\r\n"),
                      pBlockCaps->WriteBlockSize,
                      pBlockCaps->WriteBlocks));

            if (pBlockCaps->ReadBlockSize > SDH_MAX_BLOCK_SIZE)
            {
                pBlockCaps->ReadBlockSize = SDH_MAX_BLOCK_SIZE;
            }

            if (pBlockCaps->ReadBlockSize < SDH_MIN_BLOCK_SIZE )
            {
                pBlockCaps->ReadBlockSize = SDH_MIN_BLOCK_SIZE;
            }

            if (pBlockCaps->WriteBlockSize > SDH_MAX_BLOCK_SIZE)
            {
                pBlockCaps->WriteBlockSize = SDH_MAX_BLOCK_SIZE;
            }

            if (pBlockCaps->WriteBlockSize < SDH_MIN_BLOCK_SIZE )
            {
                pBlockCaps->WriteBlockSize = SDH_MIN_BLOCK_SIZE;
            }

            // ARM11 is limited by DMA buffer size allocated for MMCSD
            pBlockCaps->ReadBlocks = SDHC_MAX_NUM_BLOCKS;
            pBlockCaps->WriteBlocks = SDHC_MAX_NUM_BLOCKS;
        } else
            status = SD_API_STATUS_INVALID_PARAMETER;
        break;

    case SDHCDGetSlotInfo:
        DEBUGMSG(ZONE_INFO,
                 (TEXT("SDHSlotOptionHandler - called - SDHCDGetSlotInfo   \r\n")));
        if ( OptionSize != sizeof(SDCARD_HC_SLOT_INFO) || pData == NULL )
        {
            status = SD_API_STATUS_INVALID_PARAMETER;
        } else
        {
            PSDCARD_HC_SLOT_INFO pSlotInfo = (PSDCARD_HC_SLOT_INFO)pData;
            // set the slot capabilities
            SDHCDSetSlotCapabilities(pSlotInfo, SD_SLOT_SD_1BIT_CAPABLE |
                                     SD_SLOT_SD_4BIT_CAPABLE |
                                     SD_SLOT_SDIO_CAPABLE |
                                     SD_SLOT_SDMEM_4BIT_CAPABLE |
                                     SD_SLOT_SDIO_4BIT_CAPABLE |
                                     SD_SLOT_SDIO_INT_DETECT_4BIT_MULTI_BLOCK);

            //TODO: Set all valid masks.
            //TODO: PMIC do not support ranges >3.00V.
            //Set Slot voltage window
            pController->dwVoltageWindowMask = (SD_VDD_WINDOW_2_9_TO_3_0 |
                                                SD_VDD_WINDOW_3_0_TO_3_1 |
                                                SD_VDD_WINDOW_3_1_TO_3_2);

            SDHCDSetVoltageWindowMask(pSlotInfo, pController->dwVoltageWindowMask);

            // Set optimal voltage
            //TODO : is this range supported
            pController->dwOptVoltageMask = SLOT_VOLTAGE_MAX_BITMASK;
            SDHCDSetDesiredSlotVoltage(pSlotInfo, pController->dwOptVoltageMask);

            // Controller may be able to clock at higher than the max SD rate,
            // but we should only report the highest rate in the range.
            DWORD dwMaxClockRateInSDRange = SD_FULL_SPEED_RATE;
            SetRate(pController, &dwMaxClockRateInSDRange, FALSE);
            SDHCDSetMaxClockRate(pSlotInfo, dwMaxClockRateInSDRange);

            // Set power up delay.
            pController->dwPowerUpDelay = SDHC_MAX_POWER_SUPPLY_RAMP_UP;
            SDHCDSetPowerUpDelay(pSlotInfo, pController->dwPowerUpDelay);
        }
        break;

    default:
        status = SD_API_STATUS_INVALID_PARAMETER;
        break;
    }
    SDHCDReleaseHCLock(pHCContext);
    return status;
}
//------------------------------------------------------------------------------
//
// Function: SDPowerUp
//
// Power up handler. To be called within XXX_PowerOn only
//
// Parameters:
//      pHostContext[in] - host controller context
//
// Returns:
//      none
//
//------------------------------------------------------------------------------
void SDPowerUp(PSDCARD_HC_CONTEXT pHCContext)
{
    PSDH_HARDWARE_CONTEXT pHCDevice;

    SDHCDAcquireHCLock(pHCContext);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDPowerUp+\r\n")));
    pHCDevice = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    SetHardwarePowerState(pHCDevice, pHCDevice->PsAtPowerDown);

    //Since 1 controller support 1 SD slot, we use index 0
    SDHCDPowerUpDown(pHCContext, TRUE, TRUE, 0);

    SDHCDReleaseHCLock(pHCContext);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDPowerUp-\r\n")));
}
//------------------------------------------------------------------------------
//
// Function: SDPowerDown
//
// Power down handler. To be called within XXX_PowerDown only
//
// Parameters:
//      pHostContext[in] - host controller context
//
// Returns:
//      none
//
//------------------------------------------------------------------------------
void SDPowerDown(PSDCARD_HC_CONTEXT pHCContext)
{
    PSDH_HARDWARE_CONTEXT pHCDevice;
    CEDEVICE_POWER_STATE cps;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDPowerDown+\r\n")));

    SDHCDAcquireHCLock(pHCContext);
    // Do not allow client driver to continue submit request during power down.
    //Since 1 controller support 1 SD slot, we use index 0
    SDHCDPowerUpDown(pHCContext, FALSE, FALSE, 0);

    pHCDevice = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    // ENGR00042582: Indicate device is ejected.
    // This is to inform the SDBusDriver to ignore any more requests
    // and flush them from its queue.
    // Also get the current request & reject it by indicating device is removed
    // This will be done by IST once again but it is done here to immediately
    // inform upper layers to disconnect and unbind. This is required for
    // Wireless SDIO cards which needs to be unbinded quickly
// for SD HIVE
#ifndef BSP_HIVE_SDHC
    SDHCDIndicateSlotStateChange(pHCContext, 0,DeviceEjected);
#endif

    PSD_BUS_REQUEST pRequest = SDHCDGetAndLockCurrentRequest(pHCContext, 0);

    if (pRequest != NULL)
    {
        DEBUGMSG(ZONE_INFO,
                 (TEXT("PowerDown - Canceling current request: 0x%08X, command: %d\n"),
                  pRequest, pRequest->CommandCode));
// for SD HIVE
#ifndef BSP_HIVE_SDHC
    IndicateBusRequestComplete(pHCDevice, pRequest, SD_API_STATUS_DEVICE_REMOVED);
#endif
    }

    // If DMA read is active
    if (pHCDevice->fDMATransfer == TRUE)
    {
        // Abrupt and stop the DMA transfer and report an error
        pHCDevice->fDmaError = TRUE;
        DDKSdmaStopChan(pHCDevice->ChanSDHC, FALSE);
        pHCDevice->fDMATransfer = FALSE;
    }
    pHCDevice->PsAtPowerDown = pHCDevice->CurrentPowerState;

    if (pHCDevice->fWakeOnSDIOInt)
    {
        cps = D3;
    } else
    {
        cps = D4;
    }

    SetHardwarePowerState(pHCDevice, cps);

    SDHCDReleaseHCLock(pHCContext);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDPowerDown-\r\n")));
}
/*******************************************************************************
 PRIVATE FUNCTIONS
*******************************************************************************/

#ifdef SD_R1B_BUSYWAIT_WORKAROUND
//------------------------------------------------------------------------------
//
// Function: SDControllerBusyResponse
//
//  busy waits for the card to be ready so that next write command can be issed
//
// Parameters:
//      pHCDevice[in] - the controller instance
//      pRequest[in]  - current request
//
// Returns:
//      SD_API_STATUS Code
//
//------------------------------------------------------------------------------
static SD_API_STATUS SDControllerBusyResponse(PSDH_HARDWARE_CONTEXT pHCDevice,
                                              PSD_BUS_REQUEST pRequest)
{
    DWORD           cmdatRegister = 0;
    DWORD           cmdArg = 0;
    SD_API_STATUS   status = SD_API_STATUS_SUCCESS;
    LONG            ii;
    USHORT          responseBuffer[SDH_RESPONSE_FIFO_DEPTH - 5];
    SD_CARD_STATUS  CardStatus = 0;
    SD_CARD_STATUS  CardState = 0;
    PUCHAR              tempBuffer = NULL;
    CHAR            retryCount =0;

    //W4 WARNING FIX: C4100
    UNREFERENCED_PARAMETER(pRequest);

        retryCount = DEFAULT_BUS_REQUEST_RETRY_COUNT;
    //cmdArg = (((DWORD)((SDDCGetClientDeviceFromHandle(pRequest->hDevice))->RelativeAddress)) << 16);
    cmdArg = pHCDevice->RelativeAddress;

        do
        {
            status = SD_API_STATUS_SUCCESS;
            if (CardState != CardStatus)
            {
            DEBUGMSG (ZONE_INFO,(TEXT("SDControllerBusyResponse: Sleeping before issuing next Status cmd\r\n")));
                Sleep (0);
            }

            CardState = CardStatus;

        DEBUGMSG(ZONE_INFO, (TEXT("SDControllerBusyResponse: Sending Status Command %d\r\n"), retryCount));

            CardState = SD_STATUS_CURRENT_STATE_TRAN;

        // Check if clock gating between commands is supported.
        if ((pHCDevice->fClockGatingSupported == TRUE) && (pHCDevice->fClockGatedOff == TRUE))
        {
            // Controller clocks needs to be enabled
            BSPSdhcSetClockGatingMode(TRUE, pHCDevice->ControllerIndex);
            // update this variable after enabling the clocks to
            // avoid possible race conditions
            pHCDevice->fClockGatedOff = FALSE; // controller clocks are enabled
        }

            CSP_BITFINS(cmdatRegister, SDHC_CDC_FORMAT, 1);

        // check to see if we need to enable wide bus (4 bit) data transfer mode
            CSP_BITFINS(cmdatRegister, SDHC_CDC_BW, pHCDevice->BusWidthSetting);
            // Clear all status.
            OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, 0xFFFFFFFF);
        // set the command
        INSREG32BF(&pHCDevice->pSDMMCRegisters->CMD, SDHC_CMD_CMD, SD_CMD_SEND_STATUS);
#pragma warning(push)
#pragma warning(disable: 4293)
        // set the argument
        INSREG32BF(&pHCDevice->pSDMMCRegisters->ARG, SDHC_ARG_ARG, cmdArg);
#pragma warning(pop)

            // write the CMDAT register
            OUTREG32(&pHCDevice->pSDMMCRegisters->CMD_DAT_CONT, cmdatRegister);

            //start clock
            SetClock(pHCDevice, TRUE);

            while (!EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_ECR))
            {
                Sleep (0);
            }

            // Clear interrupt status by writing 1 to the corresponsing bit only
        OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_ECR, 1));

            //Check for response time-out or crc error.If found, retry again
            if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_TORESP))
            {
                // Clear status by writing 1
                OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS,
                         CSP_BITFVAL(SDHC_SR_TORESP, 1));
                retryCount --;
                status = SD_API_STATUS_RESPONSE_TIMEOUT;
                DEBUGMSG(ZONE_ERROR, (TEXT("SDControllerBusyResponseThread: ")
                                      TEXT("Status Command timeout\r\n")));
                continue;
            }

            if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_RSPCERR))
            {
                    // Clear status by writing 1
                    OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_RSPCERR, 1));
                    retryCount --;
                    status = SD_API_STATUS_CRC_ERROR;
                    DEBUGMSG(ZONE_ERROR, (TEXT("SDControllerBusyResponseThread:  response CRC error\r\n")));
                    continue;
            }

        memset ((responseBuffer), 0, (SDH_RESPONSE_FIFO_DEPTH - 5));

            // read in the response words from the response fifo.
            for (ii = 2; ii >= 0; ii--)
            {
                 // read from the fifo
                 responseBuffer[ii] =INREG16(&pHCDevice->pSDMMCRegisters->RES_FIFO);
            DEBUGMSG(ZONE_INFO, (TEXT("SDControllerBusyResponse: ResponseBuffer[%d]=0x%x\r\n"),ii,responseBuffer[ii]));
            }

            tempBuffer = (PUCHAR)responseBuffer;

            memcpy((&CardStatus), &tempBuffer[1], sizeof(SD_CARD_STATUS));
            DEBUGMSG(ZONE_INFO, (TEXT("CardStatus=0x%x\r\n"),SD_STATUS_CURRENT_STATE(CardStatus)));
        }while ( (SD_STATUS_CURRENT_STATE (CardStatus) != CardState) && (retryCount > 0) );

    // update this variable before disabling the clocks to
    // avoid possible race conditions
    pHCDevice->fClockGatedOff = TRUE; // controller clocks are disabled
    SetClock(pHCDevice, FALSE);
    BSPSdhcSetClockGatingMode(FALSE, pHCDevice->ControllerIndex);
    return status;
    }
#endif //SD_R1B_BUSYWAIT_WORKAROUND

//------------------------------------------------------------------------------
//
// Function: SDControllerIstThread
//
// IST thread for MMC Controller driver
//
// Parameters:
//      pHCDevice[in] - the controller instance
//
// Returns:
//      Thread exit code
//
//------------------------------------------------------------------------------
DWORD SDControllerIstThread(PSDH_HARDWARE_CONTEXT pHCDevice)
{
    PSD_BUS_REQUEST pRequest;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+SDControllerIstThread: IST thread for MMC Controller driver\r\n")));
#if !VPMX31
    //cant use with virtio. It will cause thread synchronization problem
    if (!CeSetThreadPriority(GetCurrentThread(), pHCDevice->ControllerIstThreadPriority))
    {
        ERRORMSG(ZONE_ERROR, (TEXT("SDControllerIstThread: warning, failed to set CEThreadPriority \n")));
    }
#endif //VPMX31
    for (;;)
    {

        DEBUGMSG(ZONE_INTERRUPT, (TEXT("Wait For Interrupt ... \r\n")));
        if (WaitForSingleObject(pHCDevice->hControllerInterruptEvent, INFINITE) != WAIT_OBJECT_0)
        {
            // bail out
            ERRORMSG(ZONE_ERROR, (TEXT("SDControllerIstThread: Wait Failed!  \r\n")));
            return 0;
        }

        if (pHCDevice->DriverShutdown)
        {
            DEBUGMSG(ZONE_WARN, (TEXT("SDControllerIstThread: Thread Exiting\r\n")));
            return 0;
        }

        SDHCDAcquireHCLock(pHCDevice->pHCContext);

        // Handle card insert/remove
        SDCardPresenceDetect(pHCDevice);
        if (pHCDevice->fDeviceStatusChange)
        {
            DEBUGMSG(ZONE_INTERRUPT, (TEXT("SDControllerIstThread: Device present %d\r\n"), pHCDevice->fDevicePresent));
            if (pHCDevice->fDevicePresent)
            {
                ProcessCardInsertion(pHCDevice);
            }
            else
            {
                ProcessCardRemoval(pHCDevice);

#ifdef MSCDSK
                {
                    HANDLE hSDDisk = NULL;
                    BOOL bHandleCloseSD = FALSE;
                    DWORD dwErr = 0;
                    hSDDisk = CreateEvent(NULL, FALSE, FALSE, L"MSC_SDDISK");
                    if (GetLastError() == ERROR_ALREADY_EXISTS) bHandleCloseSD = TRUE;

                    dwErr = WaitForSingleObject(hSDDisk, 0);
                    if (dwErr != WAIT_TIMEOUT)
                    {
                        //means event is set
                        HANDLE hDetachEvent = NULL;
                        BOOL bHandleCloseDetach = FALSE;
                        hDetachEvent = CreateEvent(NULL, FALSE, FALSE, L"MSC_DETACH_EVENT");
                        if (GetLastError() == ERROR_ALREADY_EXISTS) bHandleCloseDetach = TRUE;

                        if (hSDDisk) ResetEvent(hSDDisk);
                        if (hDetachEvent) SetEvent(hDetachEvent);

                        if (hDetachEvent && bHandleCloseDetach) CloseHandle(hDetachEvent);

                    }
                    if (hSDDisk && bHandleCloseSD) CloseHandle(hSDDisk);
                }
#endif
            }

            goto _done;
        }

        if (pHCDevice->fFakeCardRemoval)
        {
            pHCDevice->fFakeCardRemoval = FALSE;
            DEBUGMSG(ZONE_WARN, (TEXT("SDControllerIstThread:Fake card removal ... \r\n")));
            ProcessCardRemoval(pHCDevice); // Upper layer should be notified of a card removal

            pHCDevice->fDevicePresent = TRUE; //Device is present
            ProcessCardInsertion(pHCDevice);
            goto _done;
        }

        // If we are in the IST only because of a SDIO card interrupt, clocks to
        // the controller could be cut off if clock gating between cmds is supported.
        // Inorder to read the controller registers, should enable clocks here.
        if ((pHCDevice->fClockGatedOff == TRUE) && (pHCDevice->fClockGatingSupported == TRUE))
        {
            // Controller clocks needs to be enabled
            BSPSdhcSetClockGatingMode(TRUE, pHCDevice->ControllerIndex);
            pHCDevice->fClockGatedOff = FALSE; // controller clocks are enabled
        }

        //Print STATUS and INT_CNT registers
        DEBUGMSG(ZONE_INFO, (TEXT("IST: STATUS 0x%08X INT_CNT 0x%08X\r\n"),
                                   INREG32(&pHCDevice->pSDMMCRegisters->STATUS),
                                   INREG32(&pHCDevice->pSDMMCRegisters->INT_CNTR)));

        // Get the current request
        pRequest = SDHCDGetAndLockCurrentRequest(pHCDevice->pHCContext, 0);

        if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_ECR) ||
            EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_TORESP) ||
            EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_RSPCERR))
        {
            // Clear interrupt status by writing 1 to the corresponsing bit only
            OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_ECR, 1));

            if (NULL != pRequest){
                HandleCommandComplete(pHCDevice, pRequest);
            }
            goto _done;
        }

        if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQ))
        {
            if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_SDIOINT))
            {
                //Disable the SDIO interrupt controller bit before servicing SDIO interrupt
                INSREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQ, 0);
                // Disable the wakeup intr
                INSREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQWKP, 0);

                pHCDevice->fSDIOEnabled = FALSE;
                //Clear SDIO interrupt status by writing 1
                OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_SDIOINT, 1));

                // We do not clock gate the controller here, because a CMD52 is going to follow
                // to get the card interrupt status register

                //Indicate to upper layer that SDIO is interrupting
                SDHCDIndicateSlotStateChange(pHCDevice->pHCContext, 0, DeviceInterrupting);
                goto _done;
            }
        }

        if (NULL == pRequest){
            goto _done;
        }
        // If DMA transfer is active
        if (pHCDevice->fDMATransfer == TRUE)
        {

            // Check DMA status for each buffer in the chain
            UINT32 dmaStatus;
            pHCDevice->fDmaBusy = FALSE;

            for (UINT32 i = 0; i < pHCDevice->DmaChainSize; i++)
            {
                DDKSdmaGetBufDescStatus(pHCDevice->ChanSDHC, i, &dmaStatus);

                if (dmaStatus & DDK_DMA_FLAGS_BUSY)
                {
                    pHCDevice->fDmaBusy = TRUE;
                }

                if (dmaStatus & DDK_DMA_FLAGS_ERROR)
                {
                    // there was an error during DMA transfer
                    pHCDevice->fDmaError = TRUE;
                }
            }

            HandleTransferDone(pHCDevice, pRequest);
            DEBUGMSG(ZONE_INFO, (TEXT("SDControllerIstThread: (DMA) Transfer Done...\r\n")));
            goto _done;
        }
        else
        {
            BOOL bReadBufferReadyStatus  = EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_BRR);
            BOOL bWriteBufferReadyStatus = EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_BWR);
            BOOL bReadBufferReadyIntr  = EXTREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_BRE);
            BOOL bWriteBufferReadyIntr = EXTREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_BWE);

            if ( (bReadBufferReadyStatus && bReadBufferReadyIntr) ||
                 (bWriteBufferReadyStatus && bWriteBufferReadyIntr) )
            {
                if (pRequest->TransferClass == SD_WRITE)
                {
                    INSREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_BWE, 0);
                    OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_BWR, 1));
                }
                else
                {
                    INSREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_BRE, 0);
                    OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_BRR, 1));
                }

                if ((UINT32)pHCDevice->pBuffer & 0x3)
                {
                    // in case of non word aligned buffers
                    HandlePIODataTransfer(pHCDevice, pRequest);
                }
                else
                {
                    // in case of word aligned buffers
                    HandlePIOAlignedDataTransfer(pHCDevice, pRequest);
                }
                goto _done;
            }

            // we will be here only at the end of a DMA transfer completion
            if ((EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_WODONE)) ||
                (EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_RODONE)))
            {
                    HandleTransferDone(pHCDevice, pRequest);
                    DEBUGMSG(ZONE_INTERRUPT, (TEXT("SDControllerIstThread: Transfer Done (DMA)...\r\n")));
                    goto _done;
            }
        }

        // Spurious interrupt. Print STATUS and INT_CNT registers
        DEBUGMSG(ZONE_INTERRUPT, (TEXT("SDHC%d IST: Spurious Interrupt??? CMD %d STATUS 0x%08X INT_CNT 0x%08X\r\n"),
                      pHCDevice->ControllerIndex,
                      ((pRequest != NULL) ? pRequest->CommandCode : 0),
                      INREG32(&pHCDevice->pSDMMCRegisters->STATUS),
                      INREG32(&pHCDevice->pSDMMCRegisters->INT_CNTR)));

        _done:
        InterruptDone(pHCDevice->dwSysintrSDHC);
        SDHCDReleaseHCLock(pHCDevice->pHCContext);
    }
}
//------------------------------------------------------------------------------
//
// Function: IndicateBusRequestComplete
//
// indicates to upper layer that command is done and its status
//
// Parameters:
//          pHardwareContext[in] - hardware context
//          pRequest[in] - the request
//          status[in] - the status
//
// Returns:
//      none
//
//------------------------------------------------------------------------------
static void IndicateBusRequestComplete(PSDH_HARDWARE_CONTEXT pController,
                                       PSD_BUS_REQUEST pRequest,
                                       SD_API_STATUS status)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+IndicateBusRequestComplete\r\n")));

    if (pRequest->CommandCode == SD_CMD_APP_CMD)
        pController->fAppCommandSent = TRUE;
    else
        pController->fAppCommandSent = FALSE;

    // Soft reset host on cmd response and data errors
    if (status != SD_API_STATUS_SUCCESS)
    {
        if (status == SD_API_STATUS_RESPONSE_TIMEOUT ||
            status == SD_API_STATUS_CRC_ERROR ||
            status == SD_API_STATUS_DATA_ERROR ||
            status == SD_API_STATUS_DATA_TIMEOUT )
        {
            // Soft reset but do not change any previous clock setting
            SoftwareReset(pController, FALSE);
        }
    }

    // Check whether clock gating between commands is supported
    if (pController->fClockGatingSupported == TRUE)
    {
        // Cut off clocks to the  controller
        SetClock(pController, FALSE);
        BSPSdhcSetClockGatingMode(FALSE, pController->ControllerIndex);
        pController->fClockGatedOff = TRUE; // controller clocks are disabled
    }

#if !VPMX31
    SDHCDIndicateBusRequestComplete(pController->pHCContext, pRequest, status);
#else
    if (pRequest->CommandCode == SD_CMD_IO_OP_COND)
    {
        SDHCDIndicateBusRequestComplete(pController->pHCContext, pRequest, SD_API_STATUS_RESPONSE_TIMEOUT);
    } else
    {
        SDHCDIndicateBusRequestComplete(pController->pHCContext, pRequest, status);
    }
#endif //VPMX31

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-IndicateBusRequestComplete\r\n")));
}
//------------------------------------------------------------------------------
//
// Function: HandleCommandComplete
//
// Handles an END_CMD_RESP event
//
// Parameters:
//          pController - hardware context
//
// Returns:
//      returns TRUE is command is completed. FALSE otherwise
//
//------------------------------------------------------------------------------
static void HandleCommandComplete(PSDH_HARDWARE_CONTEXT pController,
                                           PSD_BUS_REQUEST pRequest)
{
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+HandleCommandComplete\r\n")));

    //Check for response time-out
    if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_TORESP))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT(" Cmd %d: Command Response timeout! Status 0x%08X\r\n"),
                              pRequest->CommandCode, (INREG32(&pController->pSDMMCRegisters->STATUS))));
        // Clear status by writing 1
        OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_TORESP, 1));


        if (pRequest->CommandCode == SD_CMD_IO_OP_COND)
        {
            // Memory cards do not respond for CMD5.
            pController->fClockGatingSupported = BspSdhcIsClockGatingBetweenCmdsSupported(pController->ControllerIndex);
        }

        IndicateBusRequestComplete(pController, pRequest, SD_API_STATUS_RESPONSE_TIMEOUT);
        return;
    }
    else if (pRequest->CommandCode == SD_CMD_IO_OP_COND)
    {
        // Only SDIO cards will respond for CMD5
        pController->fClockGatingSupported = FALSE;
    }

    //Check for crc error
    if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RSPCERR))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Cmd %d: Command Response CRC error! Status 0x%08X\r\n"),
                              pRequest->CommandCode, (INREG32(&pController->pSDMMCRegisters->STATUS))));

        // Clear status by writing 1
        OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_RSPCERR, 1));
        IndicateBusRequestComplete(pController, pRequest, SD_API_STATUS_CRC_ERROR);
        return;
    }

    if (NoResponse != pRequest->CommandResponse.ResponseType)
    {
        LONG     ii;             // loop variable
        LONG     startingOffset;     // starting offset in response buffer
        USHORT   responseBuffer[SDH_RESPONSE_FIFO_DEPTH]; // response buffer

        if (ResponseR2 == pRequest->CommandResponse.ResponseType)
        {
            // 8 words - 128 bits
            startingOffset = SDH_RESPONSE_FIFO_DEPTH - 1;
        }
        else
        {
            // 3 WORDS - 48 bits
            startingOffset = 2;
        }

        // read in the response words from the response fifo.
        for (ii = startingOffset; ii >= 0; ii--)
        {
            // read from the fifo
            responseBuffer[ii] =INREG16(&pController->pSDMMCRegisters->RES_FIFO);
            DEBUGMSG(ZONE_INFO, (TEXT("responseBuffer[%d]=0x%x\r\n"),ii,responseBuffer[ii]));
        }

        SDPerformSafeCopy(pRequest->CommandResponse.ResponseBuffer, responseBuffer, (sizeof(USHORT)) * (startingOffset + 1));
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    //If this is a DMA data transfer, start the I/O operation; otherwise, finish the request -----
    //////////////////////////////////////////////////////////////////////////////////////////////
    if (pRequest->TransferClass == SD_COMMAND)
    {
        // there is no data associated with this command
        // finish the request

        // Need to make sure that the card has completed its internal operations
        // and is ready to accept next write command
        if ( (SD_CMD_STOP_TRANSMISSION == pRequest->CommandCode) &&
             (pController->CurrTransferReq == SD_WRITE) )
        {
#ifdef SD_BUSYWAIT_DETECT_BYPIN
        if(pController->ControllerIndex == 1)
        {
            while (BSPSdhcIsCardBusy())
                {
                    DEBUGMSG(ZONE_INFO, (TEXT("IndicateBusRequestComplete: Busy wait!\r\n")));
                    // ENGR45295: SDHC Clock Stops unexpectedly
                    // Kick start the clock if it stops in between
                    if (! EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_CBCR))
                    {
                        // Kick the clock
                        SetClock(pController, TRUE);
                        DEBUGMSG(ZONE_WARN, (TEXT("  **Kicked the SDHC clock back on**\r\n")));
                    }
                }
            }
        else
        {
         #ifdef SD_R1B_BUSYWAIT_WORKAROUND
            status = SDControllerBusyResponse(pController, pRequest);
         #endif //SD_R1B_BUSYWAIT_WORKAROUND

        }
#else
#ifdef SD_R1B_BUSYWAIT_WORKAROUND
            status = SDControllerBusyResponse(pController, pRequest);
#endif
#endif
        }


        IndicateBusRequestComplete(pController, pRequest, status);
        return;
    }

    DEBUGMSG(ZONE_INFO, (TEXT("NumBytesToTransfer: %d Bytes_in_fifo: %d\r\n"),
                         (pRequest->BlockSize*pRequest->NumBlocks),
                         pController->Bytes_in_fifo));

    // start data transfer
    if (pController->fDMATransfer == TRUE)
    {
        // Enable the read/write op done interrupts
        if (pRequest->TransferClass == SD_WRITE)
        {
            // Enable Write Operation Done interrupt
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_WODONE, 1);
        }
        else
        {
            // Enable Read Operation Done interrupt
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_RODONE, 1);
        }

        // Request cache sync to make physical memory coherent
        CacheSync(CACHE_SYNC_DISCARD);

        // Start the output DMA
        DDKSdmaStartChan(pController->ChanSDHC);
    }
    else
    {
        // Wait till fifo is ready
        WaitforFifoReady(pController, pRequest);

        if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_TOREAD))
        {
            // Read time-out error, end the transfer
            HandleTransferDone(pController,pRequest);
        }
        else
        {
            // Start PIO data transfer
            if ((UINT32)pController->pBuffer & 0x3)
        {
                // in case of non word aligned buffers
                HandlePIODataTransfer(pController, pRequest);
        }
        else
        {
                // in case of word aligned buffers
                HandlePIOAlignedDataTransfer(pController, pRequest);
            }
        }
    }
}
//------------------------------------------------------------------------------
//
// Function: HandlePIODataTransfer
//
// Handles data transfer in PIO mode for non-word aligned buffers
//
// Parameters:
//          pController - hardware context
//          pRequest    - current request
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
static void HandlePIODataTransfer(PSDH_HARDWARE_CONTEXT pController,
                                  PSD_BUS_REQUEST pRequest)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+HandlePIODataTransfer\r\n")));

    PUCHAR pBuffer             = pController->pBuffer;
    UINT32 uPollDataLength     = BSPPollDataLength();
    DWORD dwNumWordsToTransfer = min(pController->dwNumWordsToTransfer,
                                    (uPollDataLength / 4));
    DWORD dwFifoSizeInWords    = pController->Units_in_fifo;
    DWORD dwFifoFillCount      = (dwNumWordsToTransfer / dwFifoSizeInWords);
    DWORD dwLastWords          = (dwNumWordsToTransfer % dwFifoSizeInWords);
    DWORD dwMisalignedBytes    = pController->dwMisalignedBytesToTransfer;
    DWORD dwData               = 0;

    ///////////////////////////////////////////////////////////////////////////
    ///// this function is optimised for performance, not for code size ///////
    ///////////////////////////////////////////////////////////////////////////

    // this is a write operation
    if (pRequest->TransferClass == SD_WRITE)
    {
        // fill the fifo as many times possible
        for (DWORD j=0; j<dwFifoFillCount; j++)
        {
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BWR))
            {
                Sleep(0);
            }
            // fill the fifo completely
            for (DWORD k=0; k<dwFifoSizeInWords; k++)
            {
                SDPerformSafeCopy(&dwData, pBuffer, 4);
                OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, dwData);
                pBuffer+=4;
            }
        }

        // transfer the remaining words < fifo size
        if (dwLastWords)
        {
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BWR))
            {
                Sleep(0);
            }
            for (DWORD j=0; j<dwLastWords; j++)
        {
                SDPerformSafeCopy(&dwData, pBuffer, 4);
                OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, dwData);
                pBuffer+=4;
            }
        }

        // Update transfer status
        pController->pBuffer = pBuffer;
        pController->dwNumWordsToTransfer -= dwNumWordsToTransfer;

        if (!pController->dwNumWordsToTransfer)
        {
            // transfer the last few bytes that remain
            if (dwMisalignedBytes)
        {
                if (!dwLastWords)
            {
                    while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BWR))
                    {
                        Sleep(0);
                    }
                }
                SDPerformSafeCopy(&dwData, pBuffer, dwMisalignedBytes);
                OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, dwData);
                pController->dwMisalignedBytesToTransfer = 0;
            }
            // End of data, should wait for transfer done
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_WODONE))
            {
                Sleep(0);
            }
            // handle end of data
            HandleTransferDone(pController, pRequest);
        }
        else
        {
            // there is more data, enable fifo ready interrupt
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_BWE, 1);
        }
    }
    else // this is a read operation
    {
        // read the fifo as many times possible
        for (DWORD j=0; j<dwFifoFillCount; j++)
            {
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BRR))
            {
                Sleep(0);
            }
            // read the fifo completely
            for (DWORD k=0; k<dwFifoSizeInWords; k++)
            {
               dwData = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
               SDPerformSafeCopy(pBuffer, &dwData, 4);
               pBuffer+=4;
            }
            }

        // transfer the remaining words < fifo size
        if (dwLastWords)
        {
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BRR) &&
                   !EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RODONE))
            {
                Sleep(0);
            }
            for (DWORD j=0; j<dwLastWords; j++)
            {
                dwData = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                SDPerformSafeCopy(pBuffer, &dwData, 4);
                pBuffer+=4;
            }
        }

        // Update transfer status
        pController->pBuffer = pBuffer;
        pController->dwNumWordsToTransfer -= dwNumWordsToTransfer;

        if (!pController->dwNumWordsToTransfer)
        {
            // transfer the last few bytes that remain
            if (dwMisalignedBytes)
            {
                if (!dwLastWords)
                {
                    while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BRR) &&
                           !EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RODONE))
            {
                        Sleep(0);
                    }
            }
                dwData = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                SDPerformSafeCopy(pBuffer, &dwData, dwMisalignedBytes);
                pController->dwMisalignedBytesToTransfer = 0;
        }

            // End of data, should wait for transfer done
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RODONE))
            {
                Sleep(0);
            }
            // handle end of data
            HandleTransferDone(pController, pRequest);
    }
    else
    {
            // there is more data, enable fifo ready interrupt
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_BRE, 1);
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-HandlePIODataTransfer\r\n")));
}
//------------------------------------------------------------------------------
//
// Function: HandlePIOAlignedDataTransfer
//
// Handles data transfer in PIO mode for word aligned buffers
//
// Parameters:
//          pController - hardware context
//          pRequest    - current request
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
static void HandlePIOAlignedDataTransfer(PSDH_HARDWARE_CONTEXT pController,
                                         PSD_BUS_REQUEST pRequest)
        {
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+HandlePIODataTransfer\r\n")));

    PUINT32 pBuffer            = (PUINT32)pController->pBuffer;
    UINT32 uPollDataLength     = BSPPollDataLength();
    DWORD dwNumWordsToTransfer = min(pController->dwNumWordsToTransfer,
                                    (uPollDataLength / 4));
    DWORD dwFifoSizeInWords    = pController->Units_in_fifo;
    DWORD dwFifoFillCount      = (dwNumWordsToTransfer / dwFifoSizeInWords);
    DWORD dwLastWords          = (dwNumWordsToTransfer % dwFifoSizeInWords);
    DWORD dwMisalignedBytes    = pController->dwMisalignedBytesToTransfer;
    DWORD dwData               = 0;

    ///////////////////////////////////////////////////////////////////////////
    ///// this function is optimised for performance, not for code size ///////
    ///////////////////////////////////////////////////////////////////////////

    // this is a write operation
    if (pRequest->TransferClass == SD_WRITE)
    {
        // fill the fifo as many times possible
        for (DWORD j=0; j<dwFifoFillCount; j++)
        {
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BWR))
            {
                Sleep(0);
            }
            // fill the fifo completely
            for (DWORD k=0; k<dwFifoSizeInWords; k++)
            {
                OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, *pBuffer);
                pBuffer++;
            }
        }

        // transfer the remaining words < fifo size
        if (dwLastWords)
        {
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BWR))
            {
                Sleep(0);
            }
            for (DWORD j=0; j<dwLastWords; j++)
            {
                OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, *pBuffer);
                pBuffer++;
            }
        }

        // Update transfer status
        pController->pBuffer = (PUCHAR)pBuffer;
        pController->dwNumWordsToTransfer -= dwNumWordsToTransfer;

        if (!pController->dwNumWordsToTransfer)
        {
            // transfer the last few bytes that remain
            if (dwMisalignedBytes)
            {
                if (!dwLastWords)
        {
                    while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BWR))
            {
                        Sleep(0);
                    }
                }
                // need to make sure that we dont cross the buffer boundary
                SDPerformSafeCopy(&dwData, pBuffer, dwMisalignedBytes);
                OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, dwData);
                pController->dwMisalignedBytesToTransfer = 0;
            }

            // End of data, should wait for transfer done
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_WODONE))
            {
                Sleep(0);
            }
            // handle end of data
            HandleTransferDone(pController, pRequest);
        }
        else
        {
            // there is more data, enable fifo ready interrupt
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_BWE, 1);
        }
    }
    else // this is a read operation
    {
        // read the fifo as many times possible
        for (DWORD j=0; j<dwFifoFillCount; j++)
        {
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BRR))
            {
                if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RCERR) ||
                    EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_TOREAD)
          ){
                    HandleTransferDone(pController, pRequest);
                    return;
                }
                Sleep(0);
            }
            // read the fifo completely
            for (DWORD k=0; k<dwFifoSizeInWords; k++)
            {
               *pBuffer = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
               pBuffer++;
            }
        }

        // transfer the remaining words < fifo size
        if (dwLastWords)
        {
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BRR) &&
                   !EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RODONE))
            {
                Sleep(0);
            }
            for (DWORD j=0; j<dwLastWords; j++)
            {
                *pBuffer = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                pBuffer++;
            }
            }

        // Update transfer status
        pController->pBuffer = (PUCHAR)pBuffer;
        pController->dwNumWordsToTransfer -= dwNumWordsToTransfer;

        if (!pController->dwNumWordsToTransfer)
        {
            // transfer the last few bytes that remain
            if (dwMisalignedBytes)
            {
                if (!dwLastWords)
                {
                    while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BRR) &&
                           !EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RODONE))
                    {
                        Sleep(0);
            }
        }
                // need to make sure that we dont cross the buffer boundary
                dwData = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                SDPerformSafeCopy(pBuffer, &dwData, dwMisalignedBytes);
                pController->dwMisalignedBytesToTransfer = 0;
            }

            // End of data, should wait for transfer done
            while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RODONE))
            {
                Sleep(0);
            }
            // handle end of data
            HandleTransferDone(pController, pRequest);
        }
        else
        {
            // there is more data, enable fifo ready interrupt
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_BRE, 1);
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-HandlePIODataTransfer\r\n")));
}
//------------------------------------------------------------------------------
//
// Function: HandleTransferDone
//
// Handles an data transfer done event
//
// Parameters:
//          pController - hardware context
//          pRequest    - current request
//
// Returns:
//      returns TRUE is command is completed. FALSE otherwise
//
//------------------------------------------------------------------------------
static void HandleTransferDone(PSDH_HARDWARE_CONTEXT pController,
                               PSD_BUS_REQUEST pRequest)
{
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+HandleTransferDone\r\n")));

    // If DMA is involved, check for DMA errors first
    //Check whether DMA or CPU transfer
    if ((pController->fDMATransfer == TRUE) && (pController->fDmaError == TRUE))
    {
        // DMA transfer error
        pController->fDmaError = FALSE;
        status = SD_API_STATUS_DATA_ERROR;
        DEBUGMSG(ZONE_INFO, (TEXT("HandleTransferDone: Error in Dma transfer\r\n")));
        goto _done;
    }

    if (pRequest->TransferClass == SD_READ)
    {
        // If DMA read is active
        if (pController->fDMATransfer == TRUE)
        {
            // Check if read op done interrupt has been received
            if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RODONE))
            {
                // Disable read op done interrupt
                INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_RODONE, 0);

                // Clear read op done status
                OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_RODONE, 1));

                // Set flag to indicate we have received read op done
                pController->fDmaRdOpDone = TRUE;

                // Check for timeout during read operation
                if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_TOREAD))
                {
                    DEBUGMSG(ZONE_ERROR, (TEXT("Cmd %d: Read Time out! Arg 0x%08X Status 0x%08X\r\n"),
                                          pRequest->CommandCode, pRequest->CommandArgument,
                                          INREG32(&pController->pSDMMCRegisters->STATUS)));
                    //Clear status by writing 1
                    OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_TOREAD, 1));
                    status = SD_API_STATUS_DATA_TIMEOUT;
                    goto _done;
                }

            }

            // DMA transfer can be completed once all buffer desciptors
            // have been processed and read op done interrupt has
            // been received
            // If DMA read is complete
            if ((pController->fDmaBusy == TRUE) || (pController->fDmaRdOpDone == FALSE))
            {
                // Return FALSE to indicate command is not complete and wait
                return;
            }

            // Complete the DMA transfer
            DWORD dwTransferSize = pRequest->NumBlocks * pRequest->BlockSize;

            // Unlock the pages we previously locked to build the scatter-gather list
            UnlockPages(pRequest->pBlockBuffer, dwTransferSize);

            // If the DMA left bytes stranded in the SDHC FIFO
            if (pController->DmaStrandedBytes != 0)
            {
                PUINT32 pStranded = (PUINT32) (pRequest->pBlockBuffer + dwTransferSize - pController->DmaStrandedBytes);

                // Read out the remaining bytes using the CPU
                for (UINT32 i = 0; i < (pController->DmaStrandedBytes >> 2); i++)
                {
                    *pStranded = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                    pStranded++;
                }

            }

            // DMA transfer is now complete
            pController->fDMATransfer = FALSE;
        }
        else
        {
            // Disable read op done interrupt (this is required only for DMA)
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_RODONE, 0);
            // Clear interrupt status by writing 1
            OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_RODONE, 1));
        }

        // Check for CRC error
        if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RCERR))
        {
            if((pRequest->CommandCode == SD_ACMD_SEND_SCR)
              && (pController->fAppCommandSent == TRUE)){
              DEBUGMSG(ZONE_ERROR, (TEXT("Cmd %d: read SCR data crc error\r\n"),
                                  pRequest->CommandCode));
                      /* this is a workaround to make ATP 4G SDHC card work even the scr
                      reading failed */
              Sleep(10);
                  }
            DEBUGMSG(ZONE_ERROR, (TEXT("Cmd %d: Read CRC Error! Arg 0x%08X Status 0x%08X\r\n"),
                                  pRequest->CommandCode, pRequest->CommandArgument,
                                  INREG32(&pController->pSDMMCRegisters->STATUS)));
            //Clear status by writing 1
            OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_RCERR, 1));
            status = SD_API_STATUS_CRC_ERROR;
            goto _done;
        }
        // Check for timeout during read operation
        if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_TOREAD))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("Cmd %d: Read Time out! Arg 0x%08X Status 0x%08X\r\n"),
                                  pRequest->CommandCode, pRequest->CommandArgument,
                                  INREG32(&pController->pSDMMCRegisters->STATUS)));
            //Clear status by writing 1
            OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_TOREAD, 1));
            status = SD_API_STATUS_DATA_TIMEOUT;
            goto _done;
        }
    }
    else if (pRequest->TransferClass == SD_WRITE)
    {
        // If DMA write is active
        if (pController->fDMATransfer == TRUE)
        {
            DEBUGMSG(ZONE_INFO, (TEXT("Waiting for Write done status\r\n")));

            DWORD dwTransferSize = pRequest->NumBlocks * pRequest->BlockSize;

            // Unlock the pages we previously locked to build the scatter-gather list
            UnlockPages(pRequest->pBlockBuffer, dwTransferSize);

            // If the DMA left bytes stranded in the SDHC FIFO
            if (pController->DmaStrandedBytes != 0)
            {
                PUINT32 pStranded = (PUINT32) (pRequest->pBlockBuffer + dwTransferSize - pController->DmaStrandedBytes);

                //Wait till buffer ready
                while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BWR))
                {
                    Sleep(0);
                }

                // Write out the remaining bytes using the CPU
                for (UINT32 i = 0; i < (pController->DmaStrandedBytes >> 2); i++)
                {
                    OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, *pStranded);
                    pStranded++;
                }

            }

            // DMA transfer is complete
            pController->fDMATransfer = FALSE;

            // Return FALSE to indicate command is not complete and wait
            // for WRITE_OP_DONE interrupt
            return;

        }
        else
        {
            // Disable write op done interrupt (this is required only for DMA)
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_WODONE, 0);
            // Clear interrupt status by writing 1
            OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_WODONE, 1));
        }
        // if write crc error
        if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_WCERR) )
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("Cmd %d: Write CRC Error! Arg 0x%08X Status 0x%08X\r\n"),
                                  pRequest->CommandCode, pRequest->CommandArgument,
                                  INREG32(&pController->pSDMMCRegisters->STATUS)));
            //Clear status by writing 1
            OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_WCERR, 1));
            status = SD_API_STATUS_CRC_ERROR;
            goto _done;
        }
    }
    _done:
    // If we encountered a failure
    if (status != SD_API_STATUS_SUCCESS)
    {
        // Terminate any active DMA transfers
        DDKSdmaStopChan(pController->ChanSDHC, FALSE);
        pController->fDMATransfer = FALSE;
    }

    IndicateBusRequestComplete(pController, pRequest, status);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-HandleTransferDone\r\n")));
}
//------------------------------------------------------------------------------
//
// Function: ProcessCardInsertion
//
// Handles card insertion event
//
// Parameters:
//          pController - hardware context
//
// Returns:
//
//------------------------------------------------------------------------------
static void ProcessCardInsertion(void *pContext)
{
    PSDH_HARDWARE_CONTEXT pHCDevice = (PSDH_HARDWARE_CONTEXT)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+ ProcessCardInsertion: Device Fully Inserted!\r\n")));

    //Initialize the last response type to NoResponse.
    pHCDevice->LastResponedR1b = NoResponse;

    //enable SDHC clocks
    BSPSdhcSetClockGatingMode(TRUE, pHCDevice->ControllerIndex);
    pHCDevice->fClockGatedOff = FALSE; // controller clocks are enabled

    //Turn on the voltage regulator to the slot
    BSPSlotVoltageOn(pHCDevice->ControllerIndex);

    //Set slot voltage
    BSPSetVoltageSlot(pHCDevice->ControllerIndex, SLOT_VOLTAGE_MAX_BITMASK) ;
    // flag that this is the first command sent
    pHCDevice->SendInitClocks = TRUE;

    //initialise and configure the controller to start fresh with init clocks
    SoftwareReset(pHCDevice, TRUE);

    // indicate the slot change
    //Since 1 controller support 1 SD slot only, we use index 0
    SDHCDIndicateSlotStateChange(pHCDevice->pHCContext, 0, DeviceInserted);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("- ProcessCardInsertion()-\r\n")));
}
//------------------------------------------------------------------------------
//
// Function: ProcessCardRemoval
//
// Handles card removal event
//
// Parameters:
//          pController - hardware context
//
// Returns:
//
//------------------------------------------------------------------------------
static void ProcessCardRemoval(void *pContext)
{
    PSDH_HARDWARE_CONTEXT pHCDevice = (PSDH_HARDWARE_CONTEXT)pContext;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+ ProcessCardRemoval: Card Removal Detected!\r\n")));

    if (pHCDevice->fClockGatingSupported == TRUE)
    {
        // Enable clocks to controller
        BSPSdhcSetClockGatingMode(TRUE, pHCDevice->ControllerIndex);
        pHCDevice->fClockGatedOff = FALSE; // controller clocks are enabled
    }
    // Disable all interrupts from the controller
    OUTREG32(&pHCDevice->pSDMMCRegisters->INT_CNTR, 0);

    // Softreset the controller
    SoftwareReset(pHCDevice, TRUE);

    //disable SDHC clocks
    SetClock(pHCDevice, FALSE);
    BSPSdhcSetClockGatingMode(FALSE, pHCDevice->ControllerIndex);
    pHCDevice->fClockGatedOff = TRUE; // controller clocks are disabled

    //Set slot voltage to 0
    BSPSetVoltageSlot(pHCDevice->ControllerIndex, 0) ;

    // get the current request & reject it if neccessary.
    PSD_BUS_REQUEST pRequest = SDHCDGetAndLockCurrentRequest(pHCDevice->pHCContext, 0);

    if (pRequest != NULL)
    {
        DEBUGMSG(ZONE_INFO,
                 (TEXT("Card Removal Detected - Canceling current request: 0x%08X, command: %d\n"),
                  pRequest, pRequest->CommandCode));
// for SD HIVE
#ifndef BSP_HIVE_SDHC
    IndicateBusRequestComplete(pHCDevice, pRequest, SD_API_STATUS_DEVICE_REMOVED);
#endif
    }

// indicate the slot change
//Since 1 controller support 1 SD slot only, we use index 0
// for SD HIVE
#ifndef BSP_HIVE_SDHC
        SDHCDIndicateSlotStateChange(pHCDevice->pHCContext, 0,DeviceEjected);
#endif

    InitGlobals(pHCDevice);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("- ProcessCardRemoval()-\r\n")));
}
//------------------------------------------------------------------------------
//
// Function: SetRate
//
// Sets the desired MMC clock frequency. Note: The closest frequecy
//            to the desired setting is chosen.
//
// Parameters:
//          pHc[in] - hardware context
//          pRate[in] - pointer to desired clock rate in Hz
//          fSetNewRate[in] - set new rate in hardware if TRUE
//
// Returns:
//
//------------------------------------------------------------------------------
static void SetRate(PSDH_HARDWARE_CONTEXT pHc, PDWORD pRate, BOOL fSetNewRate)
{

    const ULONG ulMaxDivisior = 15;
    const ULONG ulMaxPrescaler = 0x0800;
    ULONG ulClockRate;
    ULONG ulCalRate;
    ULONG clk_div = 1, prescaler = 0, ulErr;
    ULONG ulCurErr, i, j;
    ULONG ulMaxClockRate = BSPGetSDHCCLK();//Get BSP specific clock frequency
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SetRate()+\r\n")));
    ulClockRate = *pRate;

    if (ulClockRate <= SD_DEFAULT_CARD_ID_CLOCK_RATE)
        ulClockRate = SD_DEFAULT_CARD_ID_CLOCK_RATE;

    if (ulClockRate >= BSPGetMaxCardCLK())
    {
        // If the requested bus clock cannot be supported
        ulClockRate = BSPGetMaxCardCLK();
    }

    ulErr = 0xFFFFFFFF;
    // Get best clk_div and prescaler
    for (i = 1; i <= ulMaxDivisior; i++)
    {
        for (j = 0; j <= ulMaxPrescaler;)
        {
            if (j == 0)
                ulCalRate = (ulMaxClockRate / (i + 1));
            else
                ulCalRate = (ulMaxClockRate / ((i + 1) * j * 2));

            ulCurErr = abs((ulCalRate - ulClockRate));
            if (ulCalRate <= ulClockRate)
            {
                if (ulCurErr < ulErr)
                {
                    clk_div = i;
                    prescaler = j;
                    ulErr = ulCurErr;
                }
            }

            if (j == 0)
                j = 1;
            else
                j <<= 1;
        }
    }

    // set the actual clock rate
    if (prescaler == 0)
        pHc->dwClockRate = ulMaxClockRate / (clk_div + 1);
    else
        pHc->dwClockRate = ulMaxClockRate / ((clk_div + 1) * (prescaler * 2));

    // set the new clock rate if requested
    if (fSetNewRate)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("SDSetRate - Requested Rate: %d, Setting clock rate to %d \r\n"),
                                 ulClockRate, pHc->dwClockRate ));
        DEBUGMSG(ZONE_INFO, (TEXT("SDSetRate - clk_div %d, prescaler %d \r\n"), clk_div, prescaler));

        INSREG32BF(&pHc->pSDMMCRegisters->CLK_RATE, SDHC_CRATE_DIV, clk_div);
        INSREG32BF(&pHc->pSDMMCRegisters->CLK_RATE, SDHC_CRATE_PRES, prescaler);

        // SDIO spec says card can take a max of 1 sec to complete register
        // read or write.
        if (ulClockRate <= SD_DEFAULT_CARD_ID_CLOCK_RATE)
            pHc->ulReadTimeout = 0x2DB4;    // HW recommended value
        else
            pHc->ulReadTimeout = ulClockRate / 256;
        if (pHc->ulReadTimeout > 0xFFFF)
            pHc->ulReadTimeout = 0xFFFF;
    }
    *pRate = pHc->dwClockRate;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SetRate()-\r\n")));
}
//------------------------------------------------------------------------------
//
// Function: SetClock
//
// Enable/disable MMC clock
//
// Parameters:
//          pController[in] - hardware context
//          Clock[in] - desired clock state
//
// Returns:
//
//------------------------------------------------------------------------------
void SetClock(PSDH_HARDWARE_CONTEXT pController, BOOL Start)
{
    if (Start)
    {
        do
        {
            SETREG32(&pController->pSDMMCRegisters->STR_STP_CLK, CSP_BITFMASK(SDHC_SSCR_START));
        } while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_CBCR));
    } else
    {
        do
        {
            /*
             * SDHC H/W Errata - DDTS TLSbo93469
             * Write 0 to STOP_CLK bit, then write 1 to stop the SDHC clock
             */
            INSREG32BF(&pController->pSDMMCRegisters->STR_STP_CLK, SDHC_SSCR_STOP, 0);
            SETREG32(&pController->pSDMMCRegisters->STR_STP_CLK, CSP_BITFMASK(SDHC_SSCR_STOP));
        } while (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_CBCR));
    }
}
//------------------------------------------------------------------------------
//
// Function: SetHardwarePowerState
//
// handles power state changes
//
// Parameters:
//          pHc[in] - hardware context
//          ds[in] - desired power state
//
// Returns:
//
//------------------------------------------------------------------------------
static void SetHardwarePowerState(PSDH_HARDWARE_CONTEXT pHc, CEDEVICE_POWER_STATE cpsNew)
{
    CEDEVICE_POWER_STATE cpsCurrent = pHc->CurrentPowerState;
    pHc->CurrentPowerState = cpsNew;

    // only D0 and D4 are supported
    if (cpsNew != D0)
    {
        cpsNew = D4;
    }

    if (cpsCurrent == D0)
    {
        //Switch to D4
        //Shut down clocks to the controller and card
        //turn off card clock if its on
        if (pHc->fClockGatedOff == FALSE)
        {
            SetClock(pHc, FALSE);
            // turn off the controller clock
        BSPSdhcSetClockGatingMode(FALSE, pHc->ControllerIndex);
        }
        //Switch off the slot voltage
        BSPSetVoltageSlot(pHc->ControllerIndex, 0) ;
    }
    else
    {
        //Move from D4 to D0
        pHc->CurrentPowerState = cpsNew;
        if (pHc->fDevicePresent == TRUE)
        {
            // Fake card removal for re-enumeration of card.
            pHc->fFakeCardRemoval = TRUE;
            SetInterruptEvent(pHc->dwSysintrSDHC);
        }

    }
}

//------------------------------------------------------------------------------
//
// Function: DeinitChannelDMA
//
//  This function deinitializes the DMA channel for output.
//
// Parameters:
//      pController[in] - hardware context
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL DeinitChannelDMA(PSDH_HARDWARE_CONTEXT pController)
{
    if (pController->ChanSDHC != 0)
    {
        DDKSdmaCloseChan(pController->ChanSDHC);
        pController->ChanSDHC = 0;
    }
    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: InitChannelDMA
//
//  This function initializes the DMA channel for output.
//
// Parameters:
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL InitChannelDMA(PSDH_HARDWARE_CONTEXT pController)
{
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION,(_T("SDHC: +InitChannelDMA\r\n")));

    // Open virtual DMA channels
    pController->ChanSDHC = DDKSdmaOpenChan(pController->DmaReqRx,
                                            BspSdhcGetSdmaChannelPriority(pController->ControllerIndex),
                                            NULL,
                                            pController->dwIrqSDHC);
    pController->CurrentDmaReq = pController->DmaReqRx ;
    DEBUGMSG(ZONE_FUNCTION,(_T("SDHC: Channel Allocated : %d\r\n"),pController->ChanSDHC));
    if (!pController->ChanSDHC)
    {
        ERRORMSG(ZONE_ERROR, (_T("SDHC: InitChannelDMA: SdmaOpenChan for input failed.\r\n")));
        goto cleanUp;
    }

    // Get DMA configuration from platform code
    pController->DmaMinTransfer = BspSdhcGetSdmaMinTransfer();

    // Allocate DMA chain buffer
    if (!DDKSdmaAllocChain(pController->ChanSDHC, SDHC_DMA_BUF_DESC))
    {
        ERRORMSG(ZONE_ERROR, (_T("SDHC: InitChannelDMA: DDKSdmaAllocChain for input failed.\r\n")));
        goto cleanUp;
    }

    pController->fDmaUpdateContext = TRUE;

    rc = TRUE;

    cleanUp:
    if (!rc)
    {
        DeinitChannelDMA(pController);
    }
    DEBUGMSG(ZONE_FUNCTION,(_T("SDHC: -InitChannelDMA\r\n")));
    return rc;
}
//------------------------------------------------------------------------------
//
// Function: InitDMA
//
//  Performs DMA channel intialization
//
// Parameters:
//      pController[in] - hardware context
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL InitDMA(PSDH_HARDWARE_CONTEXT pController)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+InitDMA!\r\n")));

    // Initialize the output DMA
    if (!InitChannelDMA(pController))
    {
        ERRORMSG(ZONE_ERROR, (TEXT("SDHC.DLL:InitDMA() - Failed to initialize output DMA.\r\n")));
        return FALSE;
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-InitDMA!\r\n")));
    return TRUE ;
}
//------------------------------------------------------------------------------
//
// Function: DeInitDMA
//
//  helper function which deinitialize DMA
//
// Parameters:
//      pController[in] - hardware context
//
//  Returns: TRUE  - successfully deinitialized the DMA interface to SDHC
//           FALSE - error in deinitializing the DMA interface to SDHC
//------------------------------------------------------------------------------
BOOL DeInitDMA(PSDH_HARDWARE_CONTEXT pController)
{
    if (!DeinitChannelDMA(pController))
    {
        ERRORMSG(ZONE_ERROR, (TEXT("SDHC.DLL:DeinitChannelDMA() - Failed to deinitialize DMA.\r\n")));
        return FALSE;
    }
    return TRUE ;
}
//------------------------------------------------------------------------------
//
// Function: InitGlobals
//
//  helper function to init all transaction globals
//
// Parameters:
//      pController[in] - hardware context
//
//  Returns:
//
//------------------------------------------------------------------------------
static void InitGlobals(PSDH_HARDWARE_CONTEXT pController)
{
    pController->BusWidthSetting = 0;
    pController->ulReadTimeout = 0x2DB4;
    pController->fDevicePresent = FALSE;
    pController->fDeviceStatusChange = FALSE;
    pController->PsAtPowerDown = D0;
    pController->fSDIOEnabled = FALSE;
    pController->f4BitMode = FALSE;
    pController->fWakeOnSDIOInt = FALSE;        //SDIO wake up is enabled by bus driver
    pController->fFakeCardRemoval = FALSE;
}
//------------------------------------------------------------------------------
//
// Function: TransferIsSDIOAbort
//
//  Checks if request is an SDIO abort
//                  (CMD52, Function 0, I/O Abort Reg)?
//
// Parameters:
//      pRequest[in] - request structure
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
inline BOOL TransferIsSDIOAbort(PSD_BUS_REQUEST pRequest)
{
    if (pRequest->CommandCode == SD_CMD_IO_RW_DIRECT)
    {
        if (IO_RW_DIRECT_ARG_FUNC(pRequest->CommandArgument) == 0)
        {
            if (IO_RW_DIRECT_ARG_ADDR(pRequest->CommandArgument) == SD_IO_REG_IO_ABORT)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: SDCardPresenceDetect
//
//  IST thread for card detect interrupts
//
// Parameters:
//      pContext[in] - pointer to hardware context
//
//  Returns:
//      thread exit code
//
//------------------------------------------------------------------------------
void SDCardPresenceDetect(void *pContext)
{
    PSDH_HARDWARE_CONTEXT pHCDevice;
    BOOL card_exist = FALSE;
    BOOL prevState;
    UINT8 checks;
    UINT8 check_count = 4;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDCardPresenceDetect+ \r\n")));

    pHCDevice = (PSDH_HARDWARE_CONTEXT)pContext;

    // check for card presence
    card_exist = BspSdhcIsCardPresent(pHCDevice->ControllerIndex);
    if (card_exist != pHCDevice->fDevicePresent)
    {
        if(pHCDevice->fDevicePresent)
        {
          check_count = 2;
        }else
        {
          check_count = 4;
        }
        // card presence status has changed
        // perform card insertion/removal debouncing
        checks = 0;
        prevState = card_exist;
        for (;;)
        {
            Sleep(SCL_DEBOUNCE_PERIOD);
            card_exist = BspSdhcIsCardPresent(pHCDevice->ControllerIndex);
            if (prevState != card_exist)
            {
                checks = 0;
                prevState = card_exist;
            } else
                checks++;
            //if (checks >= SCL_DEBOUNCE_CHECKS)
            if (checks >=  check_count)
                break;
        }

        if (card_exist)
        {
            if (pHCDevice->fDevicePresent != card_exist)
            {
            pHCDevice->fDevicePresent = TRUE;
            pHCDevice->fDeviceStatusChange = TRUE;
            BspSdhcSetcardDetectType(pHCDevice->ControllerIndex, CARD_DETECT_REMOVAL);
            }
            else
                pHCDevice->fDeviceStatusChange = FALSE;
        } else
        {
            if (pHCDevice->fDevicePresent != card_exist)
            {
            pHCDevice->fDevicePresent = FALSE;
            pHCDevice->fDeviceStatusChange = TRUE;
            BspSdhcSetcardDetectType(pHCDevice->ControllerIndex, CARD_DETECT_INSERTION);
            }
            else
                pHCDevice->fDeviceStatusChange = FALSE;
        }
    }
    else
    {
        // no change in card presence status
        pHCDevice->fDeviceStatusChange = FALSE;
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDCardPresenceDetect- \r\n")));
}
//------------------------------------------------------------------------------
//
// Function: CleanupCardPresenceDetect
//
// Cleans up the card detection thread and resources
//
// Parameters:
//      pHardwareContext[in] - pointer to the SDHC hardware context
//
//  Returns:
//      none
//
//------------------------------------------------------------------------------
static void CleanupCardPresenceDetect(PSDH_HARDWARE_CONTEXT pHardwareContext)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CleanupCardPresenceDetect+ \r\n")));
    // Release GPIO and IOMUX pin
    BspSdhcCardDetectDeinitialize(pHardwareContext->ControllerIndex);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CleanupCardPresenceDetect- \r\n")));
}
//------------------------------------------------------------------------------
//
// Function: SetupCardPresenceDetect
//
// Setup required resources for card detection
//
// Parameters:
//      pHardwareContext[in] - pointer to the SDHC hardware context
//
//  Returns:
//      TRUE on success
//
//------------------------------------------------------------------------------
BOOL SetupCardPresenceDetect(PSDH_HARDWARE_CONTEXT pHardwareContext)
{
    BOOL status;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SetupCardPresenceDetect+ \r\n")));

    //Setup card presence detection mechanism
    status = BspSdhcCardDetectInitialize(pHardwareContext->ControllerIndex);

    if (status == FALSE)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("BspSdhcCardDetectSetup FAIL!!\r\n")));
        CleanupCardPresenceDetect(pHardwareContext);
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SetupCardPresenceDetect- \r\n")));
    return status;
}

//------------------------------------------------------------------------------
//
//  SoftwareReset - Performs software reset on SDHC module.
//  Input:  pController - hardware context
//          bResetClock - restore clock to min initialization clock if TRUE.
//
//  Output:
//  Return:
//  Notes:
//  Restores SD/MMC clock setting to previous clock setting.
//  All interrupts are masked.
//
//------------------------------------------------------------------------------
static void SoftwareReset(PSDH_HARDWARE_CONTEXT pController, BOOL bResetClock)
{
    DWORD dwClockRate, ii;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+SoftwareReset\r\n")));

    // Software Reset
    OUTREG32(&pController->pSDMMCRegisters->STR_STP_CLK, CSP_BITFVAL(SDHC_SSCR_RESET, 1));
    OUTREG32(&pController->pSDMMCRegisters->STR_STP_CLK,
             CSP_BITFVAL(SDHC_SSCR_RESET, 1) | CSP_BITFVAL(SDHC_SSCR_STOP, 1));

    for (ii = 0; ii < 8; ii++)
    {
        OUTREG32(&pController->pSDMMCRegisters->STR_STP_CLK, CSP_BITFVAL(SDHC_SSCR_STOP, 1));
    }

    // Mask all SDHC interrupts
    OUTREG32(&pController->pSDMMCRegisters->INT_CNTR, 0);

    // Initialize registers
    OUTREG32(&pController->pSDMMCRegisters->CMD_DAT_CONT, 0);
    OUTREG32(&pController->pSDMMCRegisters->RESPONSE_TO, 0xFF); //max time-out value
    OUTREG32(&pController->pSDMMCRegisters->READ_TO, 0xFFFF);   //max time-out value

    // Restore previous clock rate settings if required
    if (bResetClock)
        dwClockRate = SD_DEFAULT_CARD_ID_CLOCK_RATE;
    else
        dwClockRate = pController->dwClockRate;

    SetRate(pController, &dwClockRate, TRUE);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-SoftwareReset\r\n")));
}

#if 0 // Remove-W4: Warning C4505 workaround
//------------------------------------------------------------------------------
//
//  DumpRegisters - helper function to display SDHC register contents
//  Input:  pHc - hardware context
//  Output:
//
//  Return:
//  Notes:
//
//------------------------------------------------------------------------------
static void DumpRegisters(PSDH_HARDWARE_CONTEXT pHc)
{
    DEBUGMSG(ZONE_INFO, (TEXT("STR_STP_CLK=0x%x\r\n"),INREG32(&pHc->pSDMMCRegisters->STR_STP_CLK)));
    DEBUGMSG(ZONE_INFO, (TEXT("STATUS=0x%x\r\n"),INREG32(&pHc->pSDMMCRegisters->STATUS)));
    DEBUGMSG(ZONE_INFO, (TEXT("CLK_RATE=0x%x\r\n"), INREG32(&pHc->pSDMMCRegisters->CLK_RATE)));
    DEBUGMSG(ZONE_INFO, (TEXT("CMD_DAT_CONT=0x%x\r\n"),INREG32(&pHc->pSDMMCRegisters->CMD_DAT_CONT)));
    DEBUGMSG(ZONE_INFO, (TEXT("RES_TO=0x%x\r\n"), INREG32(&pHc->pSDMMCRegisters->RESPONSE_TO)));
    DEBUGMSG(ZONE_INFO, (TEXT("READ_TO=0x%x\r\n"), INREG32(&pHc->pSDMMCRegisters->READ_TO)));
    DEBUGMSG(ZONE_INFO, (TEXT("BLK_LEN=0x%x\r\n"),INREG32(&pHc->pSDMMCRegisters->BLK_LEN)));
    DEBUGMSG(ZONE_INFO, (TEXT("NOB=0x%x\r\n"), INREG32(&pHc->pSDMMCRegisters->NOB)));
    DEBUGMSG(ZONE_INFO, (TEXT("INT_MASK=0x%x\r\n"), INREG32(&pHc->pSDMMCRegisters->INT_CNTR)));
    DEBUGMSG(ZONE_INFO, (TEXT("CMD=0x%x\r\n") , INREG32(&pHc->pSDMMCRegisters->CMD)));
    DEBUGMSG(ZONE_INFO, (TEXT("ARG=0x%x\r\n"), INREG32(&pHc->pSDMMCRegisters->ARG)));
}
#endif

//------------------------------------------------------------------------------
//
// Function: WaitforFifoReady
//
// Wait for the fifo be ready for a data transfer
//
// Parameters:
//          pController - hardware context
//         pRequest     - current request
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
static void WaitforFifoReady(PSDH_HARDWARE_CONTEXT pController,
                             PSD_BUS_REQUEST pRequest)
{
    if (pRequest->TransferClass == SD_WRITE)
    {
        while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BWR))
        {
            Sleep(0);
        }
    }
    else
    {
        while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BRR) &&
               !EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_TOREAD) &&
               !EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RODONE))
        {
            Sleep(0);
        }
    }
}
/*******************************************************************************
 END OF FILE
*******************************************************************************/
// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE
