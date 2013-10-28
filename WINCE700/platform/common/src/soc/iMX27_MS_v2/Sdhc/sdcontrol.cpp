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
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on
// your install media.
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  sdcontrol.cpp
//
//  Implementation of SDHC common Device Driver
//
//  This file implements common SDHC functions
//  This driver is still pending to PMIC integration.
//------------------------------------------------------------------------------

#include "sdcontrol.hpp"
#include "pmic_regulator.h"
#include "pmic_lla.h"   /* For the PMIC API interface. */

/*******************************************************************************
 GLOBAL OR STATIC VARIABLES
*******************************************************************************/

/*******************************************************************************
 STATIC FUNCTION PROTOTYPES
*******************************************************************************/

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
    PSDH_HARDWARE_CONTEXT pHardwareContext;       // hardware context
    pHardwareContext = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+SDInitialize: Initialize the SDHC\r\n")));
    InitializeCriticalSection(&pHardwareContext->ControllerCriticalSection);

    // Init  globals
    InitGlobals(pHardwareContext);

    // Map Virtual Address
    pHardwareContext->pSDMMCRegisters = (PCSP_SDHC_REG) MmMapIoSpace(pHardwareContext->phySDHCAddr, sizeof(CSP_SDHC_REG), FALSE);
    if (pHardwareContext->pSDMMCRegisters == NULL)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("SDInitialize:  MmMapIoSpace failed!\r\n")));
        goto exitInit;
    }

    //Turn off the voltage regulator to the slot
    BSPSlotVoltageOff(pHardwareContext->ControllerIndex);

    //Set slot voltage
    BSPSetVoltageSlot(pHardwareContext->ControllerIndex, 0);

    //Stopt clock gating mode
    BSPSdhcSetClockGatingMode(FALSE, pHardwareContext->ControllerIndex);

    //Setup GPIO
    BspSdhcSetGPIO(pHardwareContext->ControllerIndex);

    //Reset the controller
    SoftwareReset(pHardwareContext, TRUE);

#if DMA
    // Initialize DMA
    if (!InitDMA(pHardwareContext))
    {
        ERRORMSG(ZONE_ERROR, (TEXT("MMCSD_DMAInit: cannot init DMA!\r\n")));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }
#endif //DMA

    //Create interrupt event
    pHardwareContext->hControllerInterruptEvent = CreateEvent(NULL, FALSE, FALSE,NULL);
    if (NULL == pHardwareContext->hControllerInterruptEvent)
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

    // Create interrupt thread for card detection
    if (!SetupCardDetectIST(pHardwareContext))
    {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
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

    // clean up card detection IST and free card insertion interrupt
    CleanupCardDetectIST(pHardwareContext);

    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &(pHardwareContext->dwSysintrSDHC),
                    sizeof(DWORD), NULL, 0, NULL);
    pHardwareContext->dwSysintrSDHC = SYSINTR_UNDEFINED;

    // free controller interrupt event
    if (NULL != pHardwareContext->hControllerInterruptEvent)
    {
        CloseHandle(pHardwareContext->hControllerInterruptEvent);
        pHardwareContext->hControllerInterruptEvent = NULL;
    }

#if DMA
    // Release DMA Resources
    DeInitDMA(pHardwareContext);
#endif //DMA

    // free the virtual space allocated for SDHC1 memory map
    if (pHardwareContext->pSDMMCRegisters != NULL)
    {
        MmUnmapIoSpace(pHardwareContext->pSDMMCRegisters, sizeof(CSP_SDHC_REG));
        pHardwareContext->pSDMMCRegisters = NULL;
    }
    DeleteCriticalSection(&pHardwareContext->ControllerCriticalSection);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-SDDeinitialize: \r\n")));
    return SD_API_STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: SDControllerBusyWaitResponse
//
// Thread for SDHC/MMC Controller driver which busy waits for controller to be ready
// in order to invoke next commands
//
// Parameters:
//      pHCDevice[in] - the controller instance
//
// Returns:
//      Thread exit code
//
//------------------------------------------------------------------------------
DWORD SDControllerBusyWaitResponse(PSDH_HARDWARE_CONTEXT pHCDevice)
{
    DWORD               cmdatRegister=0, cmdArg = 0;   // CMDAT register
    PSD_BUS_REQUEST     pRequest;
    SD_API_STATUS       status = SD_API_STATUS_SUCCESS;
    LONG                ii;             // loop variable
    USHORT              responseBuffer[SDH_RESPONSE_FIFO_DEPTH - 5]; // response buffer
    SD_CARD_STATUS      CardStatus = 0, CardState = 0;
    PUCHAR              tempBuffer = NULL;
    CHAR                retryCount =0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+SDControllerBusyWaitResponse for SDHC/MMC Controller driver\r\n")));

    retryCount = DEFAULT_BUS_REQUEST_RETRY_COUNT;
    pRequest = SDHCDGetAndLockCurrentRequest(pHCDevice->pHCContext, 0);

    //cmdArg = (((DWORD)((SDDCGetClientDeviceFromHandle(pRequest->hDevice))->RelativeAddress)) << 16);
    //Modifying the access to support SDBUS2 architecture.
    CSDDevice& pDevice = ((CSDBusRequest *)pRequest)->GetDevice();
    DWORD dwRelativeAddress = 0;
    status = pDevice.SDCardInfoQuery_I(SD_INFO_REGISTER_RCA, &dwRelativeAddress, sizeof(SD_CARD_RCA));
    cmdArg = (dwRelativeAddress << 16);
    do
    {
        status = SD_API_STATUS_SUCCESS;
        if (CardState != CardStatus)
        {
            DEBUGMSG (ZONE_INFO,(TEXT("SDControllerBusyWaitResponse: Sleeping before issuing next Status cmd\r\n")));
            Sleep (0);
        }

        CardState = CardStatus;

        DEBUGMSG(ZONE_INFO, (TEXT("SDControllerBusyWaitResponse: Sending Status Command %d\r\n"), retryCount));

        CardState = SD_STATUS_CURRENT_STATE_TRAN;

        CSP_BITFINS(cmdatRegister, SDHC_CDC_FORMAT, 1);
        // check to see if we need to enable wide bus (4 bit) data transfer mode
        CSP_BITFINS(cmdatRegister, SDHC_CDC_BW, pHCDevice->BusWidthSetting);
        // clear all status
        OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, 0xFFFFFFFF);
        // set the command
        INSREG32BF(&pHCDevice->pSDMMCRegisters->CMD, SDHC_CMD_CMD, SD_CMD_SEND_STATUS);
        // set the argument
        //  INSREG32BF(&pHCDevice->pSDMMCRegisters->ARG, SDHC_ARG_ARG, cmdArg);
        OUTREG32(&pHCDevice->pSDMMCRegisters->ARG, cmdArg);

        // write the CMDAT register
        OUTREG32(&pHCDevice->pSDMMCRegisters->CMD_DAT_CONT, cmdatRegister);

        //start clock
        //SetClock(pHCDevice, TRUE);

        while (!EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_ECR))
        Sleep (0);

        // Clear interrupt status by writing 1 to the corresponsing bit only
        OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_ECR, 1));

        //Check for response time-out or crc error.If found, retry again
        if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_TORESP))
        {
            // Clear status by writing 1
            OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_TORESP, 1));
            retryCount --;
            status = SD_API_STATUS_RESPONSE_TIMEOUT;
            ERRORMSG(ZONE_ERROR, (TEXT("SDControllerBusyResponseThread: Status Command timeout\r\n")));
            continue;
        }

        if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_RSPCERR))
        {
            //Clear status by writing 1
            OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_RSPCERR, 1));
            retryCount --;
            status = SD_API_STATUS_CRC_ERROR;
            ERRORMSG(ZONE_ERROR, (TEXT("SDControllerBusyResponseThread:  response CRC error\r\n")));
            continue;
        }

        memset ((&responseBuffer), 0, (SDH_RESPONSE_FIFO_DEPTH - 5));

        // read in the response words from the response fifo.
        for (ii = 2; ii >= 0; ii--)
        {
             // read from the fifo
             responseBuffer[ii] =INREG16(&pHCDevice->pSDMMCRegisters->RES_FIFO);
             DEBUGMSG(ZONE_INFO, (TEXT("responseBuffer[%d]=0x%x\r\n"),ii,responseBuffer[ii]));
        }

        tempBuffer = (PUCHAR)responseBuffer;

        memcpy((&CardStatus), &tempBuffer[1], sizeof(SD_CARD_STATUS));
        DEBUGMSG(ZONE_INFO, (TEXT("CardStatus=0x%x\r\n"),SD_STATUS_CURRENT_STATE(CardStatus)));
    }while ( (SD_STATUS_CURRENT_STATE (CardStatus) != CardState) && (retryCount > 0) );

    CardStatus = CardState = 0;

    return status;
}


//------------------------------------------------------------------------------
//
// Function: SDHCancelIoHandler
//
// io cancel handler
//
// Parameters:
//        pHostContext[in] - host controller context
//        Slot[in]              - slot the request is going on
//        pRequest[in]    - the request to be cancelled
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
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+SDHCancelIoHandler \n"))) ;

    // for now, we should never get here because all requests are non-cancelable
    // the hardware supports timeouts so it is impossible for the controller to get stuck
    DEBUG_ASSERT(FALSE);

    // get our extension
    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    // complete the request with a cancelled status
    SDHCDIndicateBusRequestComplete(pHCContext,pRequest, SD_API_STATUS_CANCELED);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-SDHCancelIoHandler \n"))) ;
    return TRUE;
}

BOOL SDControllerISTHandler(PSDH_HARDWARE_CONTEXT pHCDevice);
SD_API_STATUS SDBusIssueRequest(PSDCARD_HC_CONTEXT pHCContext, DWORD Slot, PSD_BUS_REQUEST pRequest);
//------------------------------------------------------------------------------
//
// Function: SDHBusRequestHandler
//
// bus request handler. The request passed in is marked as uncancelable, this function
// has the option of making the outstanding request cancelable
//
// Parameters:
//      pHostContext[in] - host controller context
//      Slot[in]         - slot the request is going on
//      pRequest[in]     - the request to be cancelled
//
// Returns:
//      SD_API_STATUS Code
//
//------------------------------------------------------------------------------
SD_API_STATUS SDHBusRequestHandler(PSDCARD_HC_CONTEXT pHCContext,
                                   DWORD              Slot,
                                   PSD_BUS_REQUEST    pRequest)
{
    PSDH_HARDWARE_CONTEXT   pController;     // our controller
    DWORD                   cmdatRegister=0;   // CMDAT register
    SD_API_STATUS           status;
    static  BOOL fIntrMasked = FALSE;   // flag to indicate if controller's interrupt is masked

    // get our extension
    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+  SDHBusRequestHandler \r\n")));
    DEBUGMSG(ZONE_COMMAND, (TEXT("SDHBusRequestHandler -  CMD:%d ARG:0x%08X, TxClass: %d\r\n"),
                              pRequest->CommandCode, pRequest->CommandArgument, pRequest->TransferClass));

    ACQUIRE_LOCK(pController);
    if (pController->pCurrentRequest) { // We have outstand request.
        ASSERT(FALSE);
        IndicateBusRequestComplete(pController, pRequest, SD_API_STATUS_CANCELED);
        pController->pCurrentRequest = NULL;
    }
    pController->fCurrentRequestFastPath = FALSE;
    pController->pCurrentRequest = pRequest ;

    if ((pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE) &&
            !( SD_COMMAND != pRequest->TransferClass &&
            pRequest->NumBlocks * pRequest->BlockSize >  pController->dwPollingModeSize)) {   // We do fast path here.
        // if FAST PATH
        DEBUGMSG(ZONE_INFO, (TEXT("SDHBusRequestHandler - fast path:%d\r\n"), pRequest->CommandCode));
        pController->fCurrentRequestFastPath = TRUE;
        InterruptMask(pController->dwSysintrSDHC, TRUE);
        fIntrMasked = TRUE;
        status = SDBusIssueRequest(pHCContext, Slot, pRequest);
        if (status == SD_API_STATUS_PENDING) { // Polling for completion.
            while (pController->pCurrentRequest) {
                SDControllerISTHandler(pController);
            }
            status = pController->FastPathStatus;
            if (status == SD_API_STATUS_SUCCESS) {
                status = SD_API_STATUS_FAST_PATH_SUCCESS;
            }
        }
        InterruptMask(pController->dwSysintrSDHC, FALSE);
    }
    else {
        // normal path
        if ((fIntrMasked) && !(pController->fCurrentRequestFastPath)) {
            //unmask the interrupt if needed
            InterruptMask(pController->dwSysintrSDHC, FALSE);
            fIntrMasked = FALSE;
        }
       pRequest->SystemFlags &= ~SD_FAST_PATH_AVAILABLE ;
       status = SDBusIssueRequest(pHCContext, Slot, pRequest);
       if (status!=SD_API_STATUS_PENDING) { // This has been completed.
            pController->pCurrentRequest = NULL;
       }
    }
    RELEASE_LOCK(pController);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("- SDHBusRequestHandler\r\n")));

    return status;
}

    
///////////////////////////////////////////////////////////////////////////////
//  SDBusIssueRequest - bus request function
//  Input:  pHostContext - host controller context
//          Slot - slot the request is going on
//          pRequest - the request
//
//  Output:
//  Return: SD_API_STATUS Code
//  Notes:  The request passed in is marked as uncancelable, this function
//          has the option of making the outstanding request cancelable
//
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS SDBusIssueRequest(PSDCARD_HC_CONTEXT pHCContext, DWORD Slot, PSD_BUS_REQUEST pRequest)
{
    PSDH_HARDWARE_CONTEXT    pController;     // our controller
    DWORD                      cmdatRegister=0;   // CMDAT register
    SD_API_STATUS status;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+SDBusIssueRequest \r\n")));
    
    // get our extension 
    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

     //Wait for the Card to Return to "TRANSFER" state if the previous
    //cmd response is of type "R1b"(busy wait), irrespective of the current cmd issued.
    //This will ensure that the card returns to "TRANSFER" state before processing
    //the next command.
    if (pController->LastResponedR1b == ResponseR1b)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("SDBusIssueRequest: wait for busy!\r\n")));
        status = SDControllerBusyWaitResponse(pController);
        if(status != SD_API_STATUS_SUCCESS)
        {
            SDHCDIndicateBusRequestComplete(pController->pHCContext, pRequest, SD_API_STATUS_DEVICE_BUSY);
            return SD_API_STATUS_DEVICE_BUSY;
        }
    }

    // Handle for ACMD 42/23. Our SDHC expects no reponse but spec say R1.
    if ( (pRequest->CommandCode == SD_CMD_LOCK_UNLOCK && pController->fAppCommandSent == TRUE) ||
         (pRequest->CommandCode == SD_ACMD_SET_WR_BLOCK_ERASE_COUNT && pController->fAppCommandSent == TRUE))
    {
        pRequest->CommandResponse.ResponseType = NoResponse;
    }
    pController->LastResponedR1b = pRequest->CommandResponse.ResponseType;

    // set the command
    INSREG32BF(&pController->pSDMMCRegisters->CMD, SDHC_CMD_CMD, pRequest->CommandCode);

    // set the argument
    OUTREG32(&pController->pSDMMCRegisters->ARG, pRequest->CommandArgument);
    //Set the response type
    switch (pRequest->CommandResponse.ResponseType)
    {
    case NoResponse:
        CSP_BITFINS(cmdatRegister, SDHC_CDC_FORMAT, 0);
        break;
        //MX27 spec does not have busy bit register set.
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
        ERRORMSG(ZONE_ERROR, (TEXT("SDBusIssueRequest failed (Invalid parameter)\n")));
        return SD_API_STATUS_INVALID_PARAMETER;
    }

    // check for Command Only
    if ((SD_COMMAND != pRequest->TransferClass))
    {
        // its a command with a data phase
        CSP_BITFINS(cmdatRegister, SDHC_CDC_DE, 1);

        DWORD   m_dwNumBytesToTransfer ;
        m_dwNumBytesToTransfer = pRequest->NumBlocks * pRequest->BlockSize;

        //adjust tranfer mode for SDHC tranfer performamce
        if ( (pController->f4BitMode == TRUE && m_dwNumBytesToTransfer < 64 ) ||
             (pController->f4BitMode == FALSE && m_dwNumBytesToTransfer < 16))
        {
            pController->fDMATransfer = FALSE;
        } else
        {
            #if DMA
            pController->fDMATransfer = TRUE;
            #else
            pController->fDMATransfer = FALSE;
            #endif //DMA
        }

        // check for write
        if (SD_WRITE == pRequest->TransferClass)
        {
            //Indicate that it is write process
            CSP_BITFINS(cmdatRegister, SDHC_CDC_WR, 1);

            OUTREG32(&pController->pSDMMCRegisters->STATUS, 0xFFFFFFFF);
            // Enable Write operation Done interrupt
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_WODONE, 1);

            //check for DMA or ARM transfer
            if (pController->fDMATransfer == TRUE)
            {
                BOOL fNoException;
                //SD_SET_PROC_PERMISSIONS_FROM_REQUEST( pRequest ) {
                    fNoException = SDPerformSafeCopy( (void*)pController->DmaLogicalAddressTX, pRequest->pBlockBuffer, m_dwNumBytesToTransfer );
                //} SD_RESTORE_PROC_PERMISSIONS();

                if (fNoException == FALSE)
                {
                    ERRORMSG(ZONE_ERROR, (TEXT("Write buffer access violation ERROR\r\n")));
                    return SD_API_STATUS_ACCESS_VIOLATION;
                }

                DDKDmacSetTransCount(pController->DmaReqTxCH, m_dwNumBytesToTransfer);
                // Clear DMAC status regs in order to restart channel.
                DDKDmacClearChannelIntr(pController->DmaReqTxCH);
            }
        }
        else if (SD_READ == pRequest->TransferClass)
        {
            //Specify the Read Time Out value
            INSREG32BF(&pController->pSDMMCRegisters->READ_TO, SDHC_READTO_TO, pController->ulReadTimeout);

            OUTREG32(&pController->pSDMMCRegisters->STATUS, 0xFFFFFFFF);
            // Enable Read Operation Done interrupt
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_RODONE, 1);

            //check for DMA or ARM transfer
            if (pController->fDMATransfer == TRUE)
            {
                DDKDmacSetTransCount(pController->DmaReqRxCH, m_dwNumBytesToTransfer);
                // Clear DMAC status regs in order to restart channel.
                DDKDmacClearChannelIntr(pController->DmaReqRxCH);
            }
        }

        // set transfer length
        INSREG32BF(&pController->pSDMMCRegisters->BLK_LEN, SDHC_BL_BL, pRequest->BlockSize);

        // Set Number of Blocks
        INSREG32BF(&pController->pSDMMCRegisters->NOB, SDHC_NOB_NOB, pRequest->NumBlocks);
    }
    else
    {
        //no data associated with this command
        // set transfer length
        INSREG32BF(&pController->pSDMMCRegisters->BLK_LEN, SDHC_BL_BL, 0);

        // Set Number of Blocks
        INSREG32BF(&pController->pSDMMCRegisters->NOB, SDHC_NOB_NOB, 0);
    }

    //clear interrupt status
    OUTREG32(&pController->pSDMMCRegisters->STATUS, 0xFFFFFFFF);
    // Enable End Command Response interrupt
    INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_ECR, 1);

    // check to see if we need to append the 80 clocks (i.e. this is the first transaction)
    if (pController->SendInitClocks)
    {
        pController->SendInitClocks = FALSE;
        CSP_BITFINS(cmdatRegister, SDHC_CDC_INIT, 1);
    }

    // check to see if we need to enable wide bus (4 bit) data transfer mode
    CSP_BITFINS(cmdatRegister, SDHC_CDC_BW, pController->BusWidthSetting);

    DEBUGMSG(ZONE_INFO, (TEXT("SDBusIssueRequest - CMD_DAT_CONT: 0x%08X\r\n"), cmdatRegister));

    if(pController->pSDMMCRegisters->STATUS !=  0x30000100 )
    {
        DEBUGMSG(ZONE_INFO, (TEXT("Clear STATUS REG!- pSDMMCRegisters->STATUS :0x%08X\r\n"),pController->pSDMMCRegisters->STATUS));
        OUTREG32(&pController->pSDMMCRegisters->STATUS, 0xFFFFFFFF);
        OUTREG32(&pController->pSDMMCRegisters->STATUS, 0xFFFFFFFF);
    }

    pController->SDCommandStatus = SD_COMMAND_STATUS_CMD;
    // write the CMDAT register
    OUTREG32(&pController->pSDMMCRegisters->CMD_DAT_CONT, cmdatRegister);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("- SDBusIssueRequest - Request Sent\r\n")));

    return SD_API_STATUS_PENDING;
}

///////////////////////////////////////////////////////////////////////////////
//  SDControllerISTHandler - IST Handler for MMC Controller driver
//  Input:  pHCDevice - the controller instance
//  Output:
//  Return: Thread exit code
//  Notes:
///////////////////////////////////////////////////////////////////////////////
BOOL SDControllerISTHandler(PSDH_HARDWARE_CONTEXT pHCDevice)
{
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+ SDControllerISTHandler\r\n")));
    // Check for INT_CNTR because SDIO status bit will be always set until
    // the card deasserts the SDIO int.
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

            //Indicate to upper layer that SDIO is interrupting
            SDHCDIndicateSlotStateChange(pHCDevice->pHCContext, 0, DeviceInterrupting);

            DEBUGMSG(ZONE_INTERRUPT, (TEXT("SDControllerISTHandler:IO Int\r\n")));
        }
    }

    // Check End Command Response interrupt
    if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_ECR))
    {
        if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_ECR))
        {
            //Negate End Command Response interrupt
            INSREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_ECR, 0);

            // Clear interrupt status by writing 1 to the corresponsing bit only
            OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_ECR, 1));

            if(pHCDevice->SDCommandStatus == SD_COMMAND_STATUS_CMD)
                        HandleCommandComplete(pHCDevice);
        }
    }

    // Check write operation done interrupt
    if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_WODONE))
    {
        if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_WODONE) )
        {
            //Negate Write Operation Done interrupt
            INSREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_WODONE, 0);

            // Clear interrupt status by writing 1
            OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_WODONE, 1));

            if(pHCDevice->SDCommandStatus == SD_COMMAND_STATUS_WRITE)
                        HandleTransferDone(pHCDevice);
            DEBUGMSG(ZONE_INTERRUPT, (TEXT("SDControllerISTHandler:write done ...\r\n")));
        }
    }

    // Check read operation done interrupt
    if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_RODONE))
    {
        if (EXTREG32BF(&pHCDevice->pSDMMCRegisters->STATUS, SDHC_SR_RODONE))
        {
            //Negate Write Operation Done interrupt
            INSREG32BF(&pHCDevice->pSDMMCRegisters->INT_CNTR, SDHC_INT_RODONE, 0);

            // Clear interrupt status by writing 1
            OUTREG32(&pHCDevice->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_RODONE, 1));

            if(pHCDevice->SDCommandStatus == SD_COMMAND_STATUS_READ)
                        HandleTransferDone(pHCDevice);
            DEBUGMSG(ZONE_INTERRUPT, (TEXT("SDControllerISTHandler:read done ...\r\n")));
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("- SDControllerISTHandler\r\n")));

    return TRUE;
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
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;   // status
    PSDH_HARDWARE_CONTEXT    pController;         // the controller
    PSD_HOST_BLOCK_CAPABILITY  pBlockCaps;          // queried block capabilities

    // get our extension
    pController = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    switch (Option)
    {

    case SDHCDAckSDIOInterrupt:
    case SDHCDEnableSDIOInterrupts:
        if (!pData && OptionSize == 0)
        {
            DEBUGMSG(ZONE_INFO,
                     (TEXT("SDHSlotOptionHandler - called - SDHCDAckSDIOInterrupt & EnableSDIOInterrupts : on slot %d\r\n"),
                      SlotNumber));
            //Enable SDIO interrupt
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQ, 1);
            // Enable SDIO Wakeup interrupt before cutting off the controller clock
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQWKP, 1);
            pController->fSDIOEnabled = TRUE;
        } else
            status = SD_API_STATUS_INVALID_PARAMETER;
        break;

    case SDHCDDisableSDIOInterrupts:
        if (!pData && OptionSize == 0)
        {
            DEBUGMSG(ZONE_INFO,
                     (TEXT("SDHSlotOptionHandler - called - DisableSDIOInterrupts : on slot %d  \r\n"),
                      SlotNumber));
            // Disable SDIO interrupt
            INSREG32BF(&pController->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQ, 0);
            pController->fSDIOEnabled = FALSE;
        } else
            status = SD_API_STATUS_INVALID_PARAMETER;
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
                pController->Bytes_in_fifo  = 16 ;  //16bytes

            } else if (SD_INTERFACE_SD_4BIT == ((PSD_CARD_INTERFACE)pData)->InterfaceMode)
            {
                DEBUGMSG(ZONE_INFO,
                         (TEXT("SDHSlotOptionHandler - called - SetSlotInterface : setting for 4 bit mode \r\n")));
                pController->f4BitMode = TRUE;
                pController->BusWidthSetting = 2;
                pController->Units_in_fifo  = 4*4;
                pController->Bytes_in_fifo  = 64; //64 bytes
            }

            // set rate
            SetClock(pController, FALSE);
            SetRate(pController, &((PSD_CARD_INTERFACE)pData)->ClockRate, TRUE);
            SetClock(pController, TRUE);

            if (!SD_API_SUCCESS(status))
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("SDHSlotOptionHandler: Failed to set Card Interface\n")));
            }
        } else
            status = SD_API_STATUS_INVALID_PARAMETER;
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
            //BSPSetVoltageSlot(pController->ControllerIndex, dwVddSettingMask) ;

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
                DEBUGMSG(ZONE_INFO,(TEXT("SDHSlotOptionHandler - Support SDIO Wanke Up\r\n")));
            } else
            {
                //Indicate that SDIO wake up is not supported
                pController->fWakeOnSDIOInt = FALSE;
                DEBUGMSG(ZONE_INFO,(TEXT("SDHSlotOptionHandler - not Support SDIO Wanke Up\r\n")));
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
            if ( BSPSdhcIsCardWriteProtected(pController->pBspSpecificContext) )
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

            pBlockCaps->ReadBlocks = (USHORT)  (pController->DmaRxBufSize / SDH_MAX_BLOCK_SIZE );
            pBlockCaps->WriteBlocks = (USHORT)  (pController->DmaTxBufSize / SDH_MAX_BLOCK_SIZE) ;
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

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDPowerUp+\r\n")));
    pHCDevice = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    SetHardwarePowerState(pHCDevice, pHCDevice->PsAtPowerDown);

    //Since 1 controller support 1 SD slot, we use index 0
    SDHCDPowerUpDown(pHCContext, TRUE, TRUE, 0);

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

    pHCDevice = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    pHCDevice->PsAtPowerDown = pHCDevice->CurrentPowerState;

    if (pHCDevice->fWakeOnSDIOInt)
    {
        cps = D3;

    } else
    {
        cps = D4;
    }

    SetHardwarePowerState(pHCDevice, cps);

    // Do not allow client driver to continue submit request during power down.
    //Since 1 controller support 1 SD slot, we use index 0
    SDHCDPowerUpDown(pHCContext, FALSE, FALSE, 0);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDPowerDown-\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: SDSetPowerState
//
//
//
// Parameters:
//      pHostContext[in] - host controller context
//
// Returns:
//      none
//
//------------------------------------------------------------------------------
void SDSetPowerState(PSDCARD_HC_CONTEXT pHCContext, CEDEVICE_POWER_STATE ds)
{
    PSDH_HARDWARE_CONTEXT pHCDevice;
    CEDEVICE_POWER_STATE NewPowerState = ds;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDSetPowerState+\r\n")));
    pHCDevice = GetExtensionFromHCDContext(PSDH_HARDWARE_CONTEXT, pHCContext);

    SetHardwarePowerState(pHCDevice, NewPowerState);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDSetPowerState-\r\n")));

}
/*******************************************************************************
 PRIVATE FUNCTIONS
*******************************************************************************/
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
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+SDControllerIstThread: IST thread for MMC Controller driver\r\n")));

    if (!CeSetThreadPriority(GetCurrentThread(), pHCDevice->ControllerIstThreadPriority))
    {
        ERRORMSG(ZONE_ERROR, (TEXT("SDControllerIstThread: warning, failed to set CEThreadPriority \n")));
    }

    while (1)
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

        if (pHCDevice->fFakeCardRemoval)
        {
            pHCDevice->fFakeCardRemoval = FALSE;
            DEBUGMSG(ZONE_WARN, (TEXT("SDControllerIstThread:Fake card removal ... \r\n")));
            ProcessCardRemoval(pHCDevice);

            //Function ResetCard() will be called if hardware does not reset the card after waking up.
            //If during power down (D0 to D4) hardware has set slot voltage to 0 and waking up
            //(D4 to D0) hardware has set voltage to optimum voltage, then calling ResetCard() will
            //not be necessary
            //ResetCard(pHCDevice);

            // Check for card presence again
            ProcessCardInsertion(pHCDevice);
        }

        //Print STATUS and INT_CNT registers
        DEBUGMSG(ZONE_INTERRUPT, (TEXT("IST: STATUS 0x%08X INT_CNT 0x%08X\r\n"),
                              INREG32(&pHCDevice->pSDMMCRegisters->STATUS),
                              (INREG32(&pHCDevice->pSDMMCRegisters->INT_CNTR))));

        // Handle card insert/remove
        // Wait for cmds to complete cos SDHC don't take reset very well.
        // status register contents are not reset.
        if (pHCDevice->fDeviceStatusChange)
        {
            EnterCriticalSection(&pHCDevice->ControllerCriticalSection);
            DEBUGMSG(ZONE_INTERRUPT, (TEXT("SDControllerIstThread: Device present %d\r\n"), pHCDevice->fDevicePresent));
            if (pHCDevice->fDevicePresent)
                ProcessCardInsertion(pHCDevice);
            else
                ProcessCardRemoval(pHCDevice);
            pHCDevice->fDeviceStatusChange = FALSE;
            LeaveCriticalSection(&pHCDevice->ControllerCriticalSection);
            goto _done;
        }

        ACQUIRE_LOCK(pHCDevice);
        SDControllerISTHandler(pHCDevice);
        RELEASE_LOCK(pHCDevice);

_done:
        InterruptDone(pHCDevice->dwSysintrSDHC);
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

    if (pController && pController->pCurrentRequest == pRequest) {
        DEBUGMSG(ZONE_INFO, (TEXT("IndicateBusRequestComplete - pRequest = %x, status = %d\n"),pRequest,status));
        pController->pCurrentRequest = NULL;
        if (pController->fCurrentRequestFastPath ) {
            if (status == SD_API_STATUS_SUCCESS) {
                status = SD_API_STATUS_FAST_PATH_SUCCESS;
            }
            pController->FastPathStatus = status ;
        }
        else
        {
            SDHCDIndicateBusRequestComplete(pController->pHCContext,pRequest,status);
        }
    }
    
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
static BOOL HandleCommandComplete(PSDH_HARDWARE_CONTEXT pController)
{
    PSD_BUS_REQUEST pRequest;
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+HandleCommandComplete\r\n")));
    pRequest = SDHCDGetAndLockCurrentRequest(pController->pHCContext, 0);


    //Check for response time-out
    if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_TORESP))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT(" Cmd %d: Command Response timeout! Status 0x%08X\r\n"),
                              pRequest->CommandCode, (INREG32(&pController->pSDMMCRegisters->STATUS))));

        // Clear status by writing 1
        OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_TORESP, 1));
        IndicateBusRequestComplete(pController, pRequest, SD_API_STATUS_RESPONSE_TIMEOUT);
        return TRUE;
    }

    //Check for crc error
    if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RSPCERR))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Cmd %d: Command Response CRC error! Status 0x%08X\r\n"),
                                 pRequest->CommandCode, (INREG32(&pController->pSDMMCRegisters->STATUS))));

        // Clear status by writing 1
        OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_RSPCERR, 1));
        IndicateBusRequestComplete(pController, pRequest, SD_API_STATUS_CRC_ERROR);
        return TRUE;
    }

    if (pRequest)
    {
        LONG     ii;             // loop variable
        LONG     startingOffset;     // starting offset in response buffer
        USHORT   responseBuffer[SDH_RESPONSE_FIFO_DEPTH]; // response buffer

        if (NoResponse != pRequest->CommandResponse.ResponseType)
        {
            if (ResponseR2 == pRequest->CommandResponse.ResponseType)
            {
                // 8 words - 128 bits
                startingOffset = SDH_RESPONSE_FIFO_DEPTH - 1;
            } else
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

            memcpy(pRequest->CommandResponse.ResponseBuffer, responseBuffer, (sizeof(USHORT)) * (startingOffset + 1));
        }

        ////////////////////////////////////////////////////////////////////////////
        //If this is a data transfer, start the I/O operation; otherwise, finish the request -----
        ///////////////////////////////////////////////////////////////////////////
        if (pRequest->TransferClass == SD_COMMAND)
        {
            IndicateBusRequestComplete(pController, pRequest, status);
            return TRUE;
        }
        else if (pRequest->TransferClass == SD_WRITE)
        {

            DWORD   m_dwNumBytesToTransfer ;
            m_dwNumBytesToTransfer = pRequest->NumBlocks * pRequest->BlockSize;

            pController->SDCommandStatus = SD_COMMAND_STATUS_WRITE;
            //Check whether DMA or CPU transfer
            if (pController->fDMATransfer == TRUE)
            {
                DDKDmacSetTransCount(pController->DmaReqTxCH, m_dwNumBytesToTransfer);
                // Start the output DMA
                DDKDmacStartChan(pController->DmaReqTxCH);
            }
            else
            {
                BYTE* tempBuff = pRequest->pBlockBuffer ;
                DWORD no_of_fill_fifo, j, k, remainder, tempData ;

                DWORD byteCount = 0; //count the bytes written

                no_of_fill_fifo     = (m_dwNumBytesToTransfer)/(pController->Bytes_in_fifo);
                //remaining bytes which are less than FIFO size
                remainder = (m_dwNumBytesToTransfer)-(pController->Bytes_in_fifo)*no_of_fill_fifo;

                DEBUGMSG(ZONE_INFO, (TEXT("m_dwNumBytesToTransfer:%d  Bytes_in_fifo:%d no_of_fill_fifo:%d\r\n")
                                         ,m_dwNumBytesToTransfer, pController->Bytes_in_fifo, no_of_fill_fifo));

                for (j=0; j<no_of_fill_fifo; j++)
                {
                    //Wait till buffer ready
                    while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BWR))Sleep(0);
                    for (k=0; k<pController->Units_in_fifo; k++)
                    {
                        tempData = ((tempBuff[0]) | (tempBuff[1] << 8) | (tempBuff[2] << 16) | (tempBuff[3] << 24));
                        OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, tempData);
                        tempBuff+=4 ;
                        byteCount+=4; //count the bytes written
                    }
                }

                if (remainder)
                {
                    //Wait till buffer ready
                    while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BWR))Sleep(0);

                    for (j=0;j<remainder/4;j++)
                    {
                        tempData = ((tempBuff[0]) | (tempBuff[1] << 8) | (tempBuff[2] << 16) | (tempBuff[3] << 24));
                        OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, tempData);
                        tempBuff+=4 ;
                        byteCount+=4; //count the bytes written
                    }
                    if ((remainder%4)==1)
                    {
                        tempData = tempBuff[0];
                        OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, tempData);
                        byteCount++; //count the bytes written
                    }
                    if ((remainder%4)==2)
                    {
                        tempData = ((tempBuff[0]) | (tempBuff[1] << 8));
                        OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, tempData);
                        byteCount+=2; //count the bytes written
                    }
                    if ((remainder%4)==3)
                    {
                        tempData = (tempBuff[0] | (tempBuff[1]<<8) | (tempBuff[2]<<16));
                        OUTREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS, tempData);
                        byteCount+=3; //count the bytes written
                    }
                 }

                //Print the bytes no of bytes written
                DEBUGMSG(ZONE_INFO, (TEXT("m_dwNumBytesWritten:%d  \r\n"), byteCount));
            }

            return FALSE;
        }
        else if (pRequest->TransferClass == SD_READ)
        {
            DWORD   m_dwNumBytesToTransfer ;
            m_dwNumBytesToTransfer = pRequest->NumBlocks * pRequest->BlockSize;

            pController->SDCommandStatus = SD_COMMAND_STATUS_READ;
            //Check whether DMA or CPU transfer
            if (pController->fDMATransfer == TRUE)
            {
                 DDKDmacSetTransCount(pController->DmaReqRxCH, m_dwNumBytesToTransfer);
                 // Start the output DMA
                 DDKDmacStartChan(pController->DmaReqRxCH);
            }
            else
            {
                BYTE* tempBuff =pRequest->pBlockBuffer;
                DWORD  no_of_empty_fifo, j ,k,remainder, tempData;

                DWORD byteCount = 0; //byte count

                no_of_empty_fifo    = (m_dwNumBytesToTransfer)/(pController->Bytes_in_fifo);
                //remaining bytes which are less than FIFO size
                remainder = (m_dwNumBytesToTransfer)-(pController->Bytes_in_fifo)*no_of_empty_fifo;

                DEBUGMSG(ZONE_INFO, (TEXT("m_dwNumBytesToTransfer:%d  Bytes_in_fifo:%d no_of_empty_fifo:%d\r\n")
                                         ,m_dwNumBytesToTransfer, (pController->Bytes_in_fifo), no_of_empty_fifo));

                for (j=0; j<no_of_empty_fifo; j++)
                {
                    //Wait till buffer ready or read time out
                    while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BRR) &&
                           !EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_TOREAD) &&
                           !EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RODONE))   Sleep(0);

                    //A Read Time Out can happen only in the begining
                    if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_TOREAD))
                    {
                        //0 bytes read.
                        DEBUGMSG(ZONE_INFO, (TEXT("m_dwNumBytesRead:%d  \r\n"), byteCount));
                        //TODO: why FALSE???
                        return FALSE;
                    }

                    for (k=0; k<pController->Units_in_fifo; k++)
                    {
                        tempData = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                        tempBuff[0] = (BYTE)tempData;
                        tempBuff[1] = (BYTE)(tempData >> 8);
                        tempBuff[2] = (BYTE)(tempData >> 16);
                        tempBuff[3] = (BYTE)(tempData >> 24);
                        tempBuff+=4 ;
                        byteCount+=4; //count the bytes written
                    }
                }

                if (remainder)
                {
                    while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_BRR) &&
                           !EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RODONE))  Sleep(0);

                for (j=0;j<remainder/4;j++)
                {
                    tempData = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                    tempBuff[0] = (BYTE)tempData;
                    tempBuff[1] = (BYTE)(tempData >> 8);
                    tempBuff[2] = (BYTE)(tempData >> 16);
                    tempBuff[3] = (BYTE)(tempData >> 24);
                    tempBuff+=4 ;
                    byteCount+=4; //count the bytes written
                }

                if ((remainder%4)==1)
                {
                    tempData = (BYTE)INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                    tempBuff[0] = (BYTE)tempData;
                    byteCount++; //count the bytes written
                }

                if ((remainder%4)==2)
                {
                    tempData = (WORD)INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                    tempBuff[0] = (BYTE)tempData;
                    tempBuff[1] = (BYTE)(tempData >> 8);
                    byteCount+=2; //count the bytes written
                }

                if ((remainder%4)==3)
                {
                    tempData = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                    tempBuff[0] = (BYTE)tempData;
                    tempBuff[1] = (BYTE)(tempData >> 8);
                    tempBuff[2] = (BYTE)(tempData >> 16);
                    byteCount+=3; //count the bytes written
                }
                }

                //Print the bytes no of bytes written
                DEBUGMSG(ZONE_INFO, (TEXT("m_dwNumBytesRead:%d  \r\n"), byteCount));
            }

            return FALSE;
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("HandleCommandComplete()- request must have been canceled due to an error!\r\n")));
    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: HandleTransferDone
//
// Handles an data transfer done event
//
// Parameters:
//          pController - hardware context
//
// Returns:
//      returns TRUE is command is completed. FALSE otherwise
//
//------------------------------------------------------------------------------
static BOOL HandleTransferDone(PSDH_HARDWARE_CONTEXT pController)
{
    PSD_BUS_REQUEST pRequest;
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;
    BOOL fRet = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+HandleTransferDone\r\n")));
    pRequest = SDHCDGetAndLockCurrentRequest(pController->pHCContext, 0);
    if (pRequest)
    {
        if (pRequest->TransferClass == SD_READ)
        {
            DWORD   m_dwNumBytesToTransfer;
            DWORD   DmaTransferedData, DmaRemainderData = 0;
            m_dwNumBytesToTransfer = pRequest->NumBlocks * pRequest->BlockSize;

            //Check whether DMA or CPU transfer
            if (pController->fDMATransfer == TRUE)
            {
                // Kill the output DMA channel
                DDKDmacStopChan(pController->DmaReqRxCH);

                // During read operations, the SDHC generates DMA requests if one of its data buffer is full.
                DmaTransferedData = DDKDmacGetTransSize(pController->DmaReqRxCH);
                DEBUGMSG(ZONE_DMA, (TEXT("DMA Transfer Data count 0x%X, total data count 0x%X\r\n"), DmaTransferedData, m_dwNumBytesToTransfer));
                if(DmaTransferedData < m_dwNumBytesToTransfer)
                    DmaRemainderData = m_dwNumBytesToTransfer -DmaTransferedData;

                if(DmaRemainderData > 0)
                {
                    DWORD k, tempData;
                    BYTE* tempBuff = pController->DmaLogicalAddressRX + DmaTransferedData;
                    DWORD byteCount = 0; //byte count

                    for (k=0; k<(DmaRemainderData/4); k++)
                    {
                        tempData = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                        tempBuff[0] = (BYTE)tempData;
                        tempBuff[1] = (BYTE)(tempData >> 8);
                        tempBuff[2] = (BYTE)(tempData >> 16);
                        tempBuff[3] = (BYTE)(tempData >> 24);
                        tempBuff+=4 ;
                        byteCount+=4; //count the bytes written
                    }
                    if ((DmaRemainderData%4)==1)
                    {
                        tempData = (BYTE)INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                        tempBuff[0] = (BYTE)tempData;
                        byteCount++; //count the bytes written
                    }

                    if ((DmaRemainderData%4)==2)
                    {
                        tempData = (WORD)INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                        tempBuff[0] = (BYTE)tempData;
                        tempBuff[1] = (BYTE)(tempData >> 8);
                        byteCount+=2; //count the bytes written
                    }

                    if ((DmaRemainderData%4)==3)
                    {
                        tempData = INREG32(&pController->pSDMMCRegisters->BUFFER_ACCESS);
                        tempBuff[0] = (BYTE)tempData;
                        tempBuff[1] = (BYTE)(tempData >> 8);
                        tempBuff[2] = (BYTE)(tempData >> 16);
                        byteCount+=3; //count the bytes written
                    }
                }
            }
            else
            {
                //............nothing to do...................
            }

            // if read crc error
            if (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_RCERR))
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("Cmd %d: Read CRC Error! Arg 0x%08X Status 0x%08X\r\n"),
                                      pRequest->CommandCode, pRequest->CommandArgument,
                                      INREG32(&pController->pSDMMCRegisters->STATUS)));
                //Clear status by writing 1
                OUTREG32(&pController->pSDMMCRegisters->STATUS, CSP_BITFVAL(SDHC_SR_RCERR, 1));
                status = SD_API_STATUS_CRC_ERROR;
                goto _done;
            }
            // if Read Time Out
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

            //Check whether DMA or CPU transfer
            if (pController->fDMATransfer == TRUE)
            {
                BOOL fNoException;
                //SD_SET_PROC_PERMISSIONS_FROM_REQUEST( pRequest ) {
                    fNoException = SDPerformSafeCopy( pRequest->pBlockBuffer, (void*) pController->DmaLogicalAddressRX, m_dwNumBytesToTransfer );
                //} SD_RESTORE_PROC_PERMISSIONS();

                if (fNoException == FALSE)
                {
                    ERRORMSG(ZONE_ERROR, (TEXT("SD_API_STATUS_ACCESS_VIOLATION\r\n")));
                    status = SD_API_STATUS_ACCESS_VIOLATION;
                    goto _done;
                }
            }

            fRet = TRUE;
        }
        else if (pRequest->TransferClass == SD_WRITE)
        {
            DWORD   m_dwNumBytesToTransfer ;
            m_dwNumBytesToTransfer = pRequest->NumBlocks * pRequest->BlockSize;

            //Check whether DMA or CPU transfer
            if (pController->fDMATransfer == TRUE)
            {
                //Query DMA transfer status DMAC_TRANSFER_STATUS_NONE DMA indicates that the DMA cycle is ongoing
                while(DDKDmacGetTransStatus(pController->DmaReqTxCH) == DMAC_TRANSFER_STATUS_NONE)
                {
                    DEBUGMSG(ZONE_INFO,(TEXT("DDKDmacGetTransSize 0x%X   m_dwNumBytesToTransfer 0x%X\r\n"),
                    DDKDmacGetTransSize(pController->DmaReqTxCH), m_dwNumBytesToTransfer));
                    Sleep(0);
                }
                // Kill the output DMA channel
                DDKDmacStopChan(pController->DmaReqTxCH);
            }
            else
            {
                //.............nothing to do......................
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
            fRet = TRUE;
        }
    }
    _done:

    if (pRequest)
        IndicateBusRequestComplete(pController, pRequest, status);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-HandleTransferDone\r\n")));
    return fRet;
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
    DWORD initializationClock = SD_DEFAULT_CARD_ID_CLOCK_RATE; // 100KHz

    PSDH_HARDWARE_CONTEXT pHCDevice = (PSDH_HARDWARE_CONTEXT)pContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+ ProcessCardInsertion: Device Fully Inserted!\r\n")));

    //Enable the Transceiver for sending the commands to SDSlot
    BspSdhcEnableTransceiver(TRUE);

    //Turn on the voltage regulator to the slot
    BSPSlotVoltageOn(pHCDevice->ControllerIndex);

    //Set slot voltage
    BSPSetVoltageSlot(pHCDevice->ControllerIndex, SLOT_VOLTAGE_MAX_BITMASK) ;

    //enable SDHC clocks
    BSPSdhcSetClockGatingMode(TRUE, pHCDevice->ControllerIndex);

    SetClock(pHCDevice, TRUE);

    // flag that this is the first command sent
    pHCDevice->SendInitClocks = TRUE;

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

    // indicate the slot change
    //Since 1 controller support 1 SD slot only, we use index 0
    SDHCDIndicateSlotStateChange(pHCDevice->pHCContext, 0,DeviceEjected);

    //shut off clock first
    SetClock(pHCDevice, FALSE);
    //disable SDHC clocks
    BSPSdhcSetClockGatingMode(FALSE, pHCDevice->ControllerIndex);

    //Check whether DMA or CPU transfer
    if (pHCDevice->fDMATransfer == TRUE)
    {
        // Kill DMA channel
        DDKDmacStopChan(pHCDevice->DmaReqRxCH);
        DDKDmacStopChan(pHCDevice->DmaReqRxCH);
    }

    // get the current request & reject it if neccessary.
    PSD_BUS_REQUEST pRequest = SDHCDGetAndLockCurrentRequest(pHCDevice->pHCContext, 0);

    if (pRequest != NULL)
    {
        DEBUGMSG(ZONE_INFO,
                 (TEXT("Card Removal Detected - Canceling current request: 0x%08X, command: %d\n"),
                  pRequest, pRequest->CommandCode));
        IndicateBusRequestComplete(pHCDevice, pRequest, SD_API_STATUS_DEVICE_REMOVED);
    }

    InitGlobals(pHCDevice);

    //Reset the controller
    SoftwareReset(pHCDevice, TRUE);

    //Set slot voltage
    BSPSetVoltageSlot(pHCDevice->ControllerIndex, 0) ;

    //Turn off the voltage regulator to the slot
    BSPSlotVoltageOff(pHCDevice->ControllerIndex);

    //Disable the transceiver
    BspSdhcEnableTransceiver(FALSE);
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
    ULONG clk_div, prescaler, ulErr;
    ULONG ulCurErr, i, j;
    ULONG ulMaxClockRate = BSPGetSDHCCLK();//Get BSP specific clock frequency
    DEBUGMSG(ZONE_FUNCTION, (TEXT("SetRate()+\r\n")));
    ulClockRate = *pRate;

    if (ulClockRate <= SD_DEFAULT_CARD_ID_CLOCK_RATE)
        ulClockRate = SD_DEFAULT_CARD_ID_CLOCK_RATE;

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
        if (*pRate <= SD_DEFAULT_CARD_ID_CLOCK_RATE)
            pHc->ulReadTimeout = 0x2DB4;    // HW recommended value
        else
            pHc->ulReadTimeout = *pRate / 256;
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
    DWORD i = 0 ;
    if (Start)
    {
        do
        {
            OUTREG32(&pController->pSDMMCRegisters->STR_STP_CLK, 0x2) ;
        } while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_CBCR));
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SetClock: START!\r\n")));
    } else
    {
        do
        {
            OUTREG32(&pController->pSDMMCRegisters->STR_STP_CLK, 0x1) ;
        } while (EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_CBCR));
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SetClock: STOP!\r\n")));
    }
}
//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
BOOL Drivstrset(DWORD dwIndex)
{
    PHYSICAL_ADDRESS phyAddr;
    PCSP_SYSCTRL_REGS pSYSCTRL;
    BOOL rc= TRUE;

    phyAddr.QuadPart = CSP_BASE_REG_PA_SYSCTRL;

    // Map peripheral physical address to virtual address
    pSYSCTRL = (PCSP_SYSCTRL_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_SYSCTRL_REGS), FALSE);
    // Check if virtual mapping failed
    if (pSYSCTRL == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("MmMapIoSpace failed!\r\n")));
        rc = FALSE;
        goto cleanUp;
    }

    if(dwIndex == 1)
        INSREG32BF(&pSYSCTRL->DSCR1, SYSCTRL_DSCR1_DS_SLOW10_SDHC1_CSPI3, SYSCTRL_DSCR_DRIVING_STRENGTH_MAX );
        INSREG32BF(&pSYSCTRL->PSCR, SYSCTRL_PSCR_SD1_D3_PUENCR, SYSCTRL_PSCR_22K_PU );

    if(dwIndex == 2)
    {
        INSREG32BF(&pSYSCTRL->DSCR1, SYSCTRL_DSCR1_DS_SLOW2_SDHC2_MSHC, SYSCTRL_DSCR_DRIVING_STRENGTH_MAX );
        INSREG32BF(&pSYSCTRL->PSCR, SYSCTRL_PSCR_SD2_D0_PUENCR, SYSCTRL_PSCR_22K_PU );
        INSREG32BF(&pSYSCTRL->PSCR, SYSCTRL_PSCR_SD2_D1_PUENCR, SYSCTRL_PSCR_22K_PU );
        INSREG32BF(&pSYSCTRL->PSCR, SYSCTRL_PSCR_SD2_D2_PUENCR, SYSCTRL_PSCR_22K_PU );
        INSREG32BF(&pSYSCTRL->PSCR, SYSCTRL_PSCR_SD2_D3_PUENCR, SYSCTRL_PSCR_22K_PU );
        INSREG32BF(&pSYSCTRL->PSCR, SYSCTRL_PSCR_SD2_CLK_PUENCR, SYSCTRL_PSCR_22K_PU );
        INSREG32BF(&pSYSCTRL->PSCR, SYSCTRL_PSCR_SD2_CMD_PUENCR, SYSCTRL_PSCR_22K_PU );
     }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("DSCR1 0x%08x\r\n"), pSYSCTRL->DSCR1));

    MmUnmapIoSpace(pSYSCTRL, sizeof(CSP_SYSCTRL_REGS));

cleanUp:
    return rc;

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

    switch (cpsCurrent)
    {
    case D1:
    case D2:
    case D0:
        switch (cpsNew)
        {
        case D3:
            //Enable wake-up interrupt sources
            if (pHc->fWakeOnSDIOInt && pHc->fSDIOEnabled)
                INSREG32BF(&pHc->pSDMMCRegisters->INT_CNTR, SDHC_INT_SDIOIRQWKP, 1);
            break ;
        case D4:
            // Shut down clock
            SetClock(pHc, FALSE);
            //Set the slot voltage down
            BSPSetVoltageSlot(pHc->ControllerIndex, 0) ;
            break;
        default:
            break;
        }
        break;

    case D3:
    case D4:
        switch (cpsNew)
        {
        // Waking device
        case D2:
        case D1:
            cpsNew = D0;
        case D0:
            //Set the slot voltage to slot Vdd
            BSPSetVoltageSlot(pHc->ControllerIndex, pHc->dwVddSettingMask) ;
            pHc->CurrentPowerState = cpsNew;
            if (pHc->fDevicePresent == TRUE)
            {
                // Fake card removal for re-enumeration of card.
                pHc->fFakeCardRemoval = TRUE;
                SetInterruptEvent(pHc->dwSysintrSDHC);
            }
            break;
        default:
            break;
        }
        break;
    }
}

//------------------------------------------------------------------------------
//
// Function: ResetCard
//
// Sends cmd to reset card. This function may not be needed if hardware
// is able to reset the card after waking up. This reset card is used because we haven't
// completed integration with PMIC, which is supposed to do a hardware reset. As a result,
// it's a need to reset the card manually by sending CMD0/CMD52. Sends both CMD0 and CMD52
// (I/O reset) to card. Ignores any failure.
//
// Parameters:
//      pController[in] - hardware context
//
// Returns:
//
//------------------------------------------------------------------------------
static void ResetCard(SDH_HARDWARE_CONTEXT *pController)
{
    UINT32 u32;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("ResetCard()+\r\n")));

    // Send cmd52(I/O reset) to soft reset SDIO card
    SetClock(pController, FALSE);
    INSREG32BF(&pController->pSDMMCRegisters->CMD, SDHC_CMD_CMD, SD_CMD_IO_RW_DIRECT);

    u32 = BUILD_IO_RW_DIRECT_ARG(SD_IO_OP_WRITE,
                                 SD_IO_RW_NORMAL,
                                 0, // function 0 to access common registers
                                 SD_IO_REG_IO_ABORT,
                                 8 );    // Soft reset card
    OUTREG32(&pController->pSDMMCRegisters->ARG, u32);
    u32 = 0 ;
    CSP_BITFINS(u32, SDHC_CDC_FORMAT, 1);

    if (pController->f4BitMode)
    {
        CSP_BITFINS(u32, SDHC_CDC_BW, 2);
    } else
    {
        CSP_BITFINS(u32, SDHC_CDC_BW, 0);
    }
    OUTREG32(&pController->pSDMMCRegisters->CMD_DAT_CONT, u32);
    SetClock(pController, TRUE);

    while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_ECR))
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Wait for CMD13 to complete 0x%04x\r\n"), pController->pSDMMCRegisters->STATUS));
    }

    // Send cmd 0 to card
    SetClock(pController, FALSE);
    INSREG32BF(&pController->pSDMMCRegisters->CMD, SDHC_CMD_CMD, SD_CMD_GO_IDLE_STATE);
   // INSREG32BF(&pController->pSDMMCRegisters->ARG, SDHC_ARG_ARG, 0);
 OUTREG32(&pController->pSDMMCRegisters->ARG, 0);
    u32 = 0;
    if (pController->f4BitMode)
    {
        CSP_BITFINS(u32, SDHC_CDC_BW, 2);
    } else
    {
        CSP_BITFINS(u32, SDHC_CDC_BW, 0);
    }
    OUTREG32(&pController->pSDMMCRegisters->CMD_DAT_CONT, u32);

    SetClock(pController, TRUE);
    while (!EXTREG32BF(&pController->pSDMMCRegisters->STATUS, SDHC_SR_ECR))
    {
        DEBUGMSG(ZONE_INFO, (TEXT("Wait for CMD13 to complete 0x%04x\r\n"), pController->pSDMMCRegisters->STATUS));
    }
    SetClock(pController, FALSE);


    DEBUGMSG(ZONE_FUNCTION, (TEXT("ResetCard()-\r\n")));
}

#if DMA
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
    UINT8 u8DmaTxCh, u8DmaRxCh;
    UINT32 g_dma_bufferTX, g_dma_bufferRX;
    DWORD dwDmaTxbufSize, dwDmaRxbufSize;
    LPBYTE  lpBufStart;

    DMA_ADAPTER_OBJECT Adapter;
    DMAC_CHANNEL_CFG gDMAC_InputData;
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+InitDMA\r\n")));

    dwDmaTxbufSize = pController->DmaTxBufSize;
    dwDmaRxbufSize = pController->DmaTxBufSize;

    // Allocate  buffers for DMA Adapter
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    // Allocate a block of virtual memory (physically contiguous) for the DMA buffers.
    DEBUGMSG(ZONE_INFO, (TEXT("InitDMA: Allocate DMA buffers\r\n")));

    lpBufStart = (PBYTE)HalAllocateCommonBuffer(&Adapter, dwDmaTxbufSize + dwDmaRxbufSize,
                                &pController->PhysDMABufferAddr, FALSE);

    if (!lpBufStart)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("InitDMA: DMA buffers Alloc Failed! err=%d\r\n"), GetLastError()));
        goto _exit;
    }

    pController->DmaLogicalAddressTX = lpBufStart;
    pController->DmaLogicalAddressRX = lpBufStart + dwDmaTxbufSize;
    g_dma_bufferTX = (UINT32)pController->PhysDMABufferAddr.QuadPart;
    g_dma_bufferRX = g_dma_bufferTX + dwDmaTxbufSize;

    //
    // Configure DMA for SDHC RX
    //
    DEBUGMSG(ZONE_INFO, (TEXT("InitDMA: Request RX DMA channel\r\n")));

    // 1. Request Channel for RX
    u8DmaRxCh = DDKDmacRequestChan(pController->DmaReqRxCH) ;
    if ( u8DmaRxCh == DMAC_CHANNEL_INVALID )
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Request MMCSD channel failed!\r\n")));
        goto _exit;
    }
    pController->DmaReqRxCH = u8DmaRxCh;

    // 2. Configure channel for RX
    // //TODO: FIFO should be retrieved based on which SDHC is used from HW context
    gDMAC_InputData.SrcAddr         = (UINT32)(pController->phySDHCAddr.QuadPart + SDHC_BUFFER_ACCESS_OFFSET);
    gDMAC_InputData.DstAddr         = g_dma_bufferRX;
    gDMAC_InputData.DataSize            = dwDmaRxbufSize;
    gDMAC_InputData.DstMode         = DMAC_TRANSFER_MODE_LINEAR_MEMORY;
    gDMAC_InputData.SrcMode         = DMAC_TRANSFER_MODE_FIFO ;
    gDMAC_InputData.MemDirIncrease      = TRUE ;
    gDMAC_InputData.DstSize         = DMAC_TRANSFER_SIZE_32BIT ;
    gDMAC_InputData.SrcSize         = DMAC_TRANSFER_SIZE_32BIT ;
    gDMAC_InputData.RepeatType      = DMAC_REPEAT_DISABLED;
    gDMAC_InputData.ExtReqEnable        = TRUE ;
    gDMAC_InputData.ReqSrc          = pController->DmaReq;
    gDMAC_InputData.ReqTimeout      = FALSE ;

    if (pController->f4BitMode)
        gDMAC_InputData.BurstLength = 0;    // 64 byte burst length
    else
        gDMAC_InputData.BurstLength = 16;   // 16 byte burst length

    if (DDKDmacConfigureChan(u8DmaRxCh, &gDMAC_InputData) != u8DmaRxCh)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Bind MMCSD channel failed!\r\n")));
        goto _exit;
    }

    //
    // Configure DMA for SDHC TX
    //
    DEBUGMSG(ZONE_INFO, (TEXT("InitDMA: Request TX DMA channel\r\n")));

    // 1. Request Channel for TX
    u8DmaTxCh = DDKDmacRequestChan(pController->DmaReqTxCH) ;
    if (u8DmaTxCh == DMAC_CHANNEL_INVALID)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Request MMCSD channel failed!\r\n")));
        goto _exit;
    }
    pController->DmaReqTxCH = u8DmaTxCh;

    // 2. Configure DMA for TX
    //TODO: FIFO should be retrieved based on which SDHC is used from HW context
    gDMAC_InputData.DstAddr         =(UINT32)(pController->phySDHCAddr.QuadPart + SDHC_BUFFER_ACCESS_OFFSET);
    gDMAC_InputData.SrcAddr         = g_dma_bufferTX;
    gDMAC_InputData.DataSize            = dwDmaTxbufSize ;
    gDMAC_InputData.DstMode         = DMAC_TRANSFER_MODE_FIFO ;
    gDMAC_InputData.SrcMode         = DMAC_TRANSFER_MODE_LINEAR_MEMORY ;
    gDMAC_InputData.MemDirIncrease      = TRUE ;
    gDMAC_InputData.DstSize         = DMAC_TRANSFER_SIZE_32BIT ;
    gDMAC_InputData.SrcSize         = DMAC_TRANSFER_SIZE_32BIT ;
    gDMAC_InputData.RepeatType      = DMAC_REPEAT_DISABLED;
    gDMAC_InputData.ExtReqEnable        = TRUE ;
    gDMAC_InputData.ReqSrc          = pController->DmaReq ;
    gDMAC_InputData.ReqTimeout      = FALSE ;
    if (pController->f4BitMode)
        gDMAC_InputData.BurstLength = 0;    //64 byte burst length
    else
        gDMAC_InputData.BurstLength = 16;   //16 byte burst length

    if (DDKDmacConfigureChan(u8DmaTxCh, &gDMAC_InputData) != u8DmaTxCh)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Bind MMCSD channel failed!\r\n")));
        goto _exit;
    }

    // Clear intr
    DDKDmacClearChannelIntr(u8DmaTxCh);
    DDKDmacClearChannelIntr(u8DmaRxCh);

    rc = TRUE;

_exit:
    if (!rc) DeInitDMA(pController);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-InitDMA: (rc = %d)\r\n"), rc));
    return rc;
}


BOOL DeInitDMA(PSDH_HARDWARE_CONTEXT pController)
{
    DMA_ADAPTER_OBJECT Adapter;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+DMADeinit\r\n")));

    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    // Release DMA Resources
    if (pController->DmaLogicalAddressTX)
    {
        DDKDmacReleaseChan(pController->DmaReqTxCH);
        DDKDmacReleaseChan(pController->DmaReqRxCH);

        HalFreeCommonBuffer(&Adapter, 0, pController->PhysDMABufferAddr, pController->DmaLogicalAddressTX, FALSE);
        pController->DmaLogicalAddressTX = NULL;
        pController->DmaLogicalAddressRX = NULL;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-DMADeinit\r\n")));
    return TRUE;
}
#endif //DMA

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
static void InitGlobals(PSDH_HARDWARE_CONTEXT pHardwareContext)
{
    pHardwareContext->BusWidthSetting = 0;
    pHardwareContext->ulReadTimeout = 0x2DB4;
    //pController->ulReadTimeout = 0xFFFF; //set to max
    pHardwareContext->PsAtPowerDown = D0;
    pHardwareContext->fSDIOEnabled = FALSE;
    pHardwareContext->f4BitMode = FALSE;
    pHardwareContext->fWakeOnSDIOInt = FALSE;        //SDIO wake up is enabled by bus driver
    pHardwareContext->fFakeCardRemoval = FALSE;
#if DMA
    pHardwareContext->DmaRxBufSize = BSPSdhcGetRxDmaBufferSize();
    pHardwareContext->DmaTxBufSize = BSPSdhcGetTxDmaBufferSize();
    pHardwareContext->DmaReqTxCH = BSPSdhcGetTxDmaChannel(pHardwareContext->ControllerIndex) ;
    pHardwareContext->DmaReqRxCH = BSPSdhcGetRxDmaChannel(pHardwareContext->ControllerIndex) ;
#endif

    if (pHardwareContext->ControllerIndex == 1)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SDInitialize: Controller 1\r\n")));
        //SD1 PHY Addr
        pHardwareContext->phySDHCAddr.QuadPart = CSP_BASE_REG_PA_SDHC1 ;
        //Assign IRQ value for SDHC1
        pHardwareContext->dwIrqSDHC = IRQ_SDHC1 ;
#if DMA
        pHardwareContext->DmaReq = DMAC_REQUEST_SDHC1;
#endif
    }
    else if (pHardwareContext->ControllerIndex == 2)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SDInitialize: Controller 2\r\n")));
        //SD2 PHY Addr
        pHardwareContext->phySDHCAddr.QuadPart = CSP_BASE_REG_PA_SDHC2 ;
        //Assign IRQ value for SDHC2
        pHardwareContext->dwIrqSDHC = IRQ_SDHC2 ;
#if DMA
        pHardwareContext->DmaReq = DMAC_REQUEST_SDHC2;
#endif
    }
   else if (pHardwareContext->ControllerIndex == 3)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("SDInitialize: Controller 3\r\n")));
        //SD3 PHY Addr
        pHardwareContext->phySDHCAddr.QuadPart = CSP_BASE_REG_PA_SDHC3 ;
        //Assign IRQ value for SDHC3
        pHardwareContext->dwIrqSDHC = IRQ_SDHC3 ;

#if DMA
        pHardwareContext->DmaReq = DMAC_REQUEST_SDHC3;
#endif
    }
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
// Function: SDCardDetectIstThread
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
DWORD SDCardDetectIstThread(void *pContext)
{
    HANDLE hCardInsertInterruptEvent;
    PSDH_HARDWARE_CONTEXT pHCDevice;
    BOOL card_exist = FALSE;
    UINT16  count = 0, source = 0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("SDCardDetectIstThread+ \r\n")));

    pHCDevice = (PSDH_HARDWARE_CONTEXT)pContext;
    hCardInsertInterruptEvent = pHCDevice->hCardInsertInterruptEvent;

    if (!CeSetThreadPriority(GetCurrentThread(), pHCDevice->dwCardDetectIstThreadPriority))
    {
        DEBUGMSG(ZONE_WARN, (TEXT("TransferIstThread: warning, failed to set CEThreadPriority \n")));
    }

    pHCDevice->fDevicePresent = FALSE;

    // reading the signal state to know the card existence
    if (BspSdhcIsCardPresent(pHCDevice->pBspSpecificContext))
    {
        SetEvent( hCardInsertInterruptEvent );
    }

    while (1)
    {
        if (WaitForSingleObject(hCardInsertInterruptEvent, INFINITE) != WAIT_OBJECT_0)
        {
            ERRORMSG(ZONE_ERROR, (TEXT("SDCardDetectIstThread: Wait Failed!\r\n")));
            return 0;
        }

        // This portion of the code is to exit the CardDetectIstThread when the
        // driver is unloaded.
        if (pHCDevice->DriverShutdown)
        {
            DEBUGMSG(ZONE_INFO, (TEXT("SDCardDetectIstThread: Thread Exiting\r\n")));
            return 0;
        }

        card_exist = BspSdhcIsCardPresent(pHCDevice->pBspSpecificContext);

        if (card_exist)
        {
            pHCDevice->fDevicePresent = TRUE;
            pHCDevice->fDeviceStatusChange = TRUE;
            BspSdhcCardDetectInterruptType(pHCDevice->ControllerIndex, FALSE);
            SetEvent(pHCDevice->hControllerInterruptEvent);
        } else
        {
            pHCDevice->fDevicePresent = FALSE;
            pHCDevice->fDeviceStatusChange = TRUE;
            BspSdhcCardDetectInterruptType(pHCDevice->ControllerIndex, TRUE);
            SetEvent(pHCDevice->hControllerInterruptEvent);
        }
        InterruptDone(pHCDevice->dwSysIntrCardDetect);
    }
}
//------------------------------------------------------------------------------
//
// Function: CleanupCardDetectIST
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
static void CleanupCardDetectIST(PSDH_HARDWARE_CONTEXT pHardwareContext)
{
    //Deregister interrupt
    BspSdhcDeregisterCardInterrupt(pHardwareContext->pBspSpecificContext) ;

    //Set the event to run the card controller IST thread for shut down
    SetEvent(pHardwareContext->hCardInsertInterruptEvent);
    //Give some time for the ControllerISTThread to run and exit
    Sleep(1);

    if (NULL != pHardwareContext->hCardInsertInterruptEvent)
    {
        pHardwareContext->hCardInsertInterruptEvent = NULL; //free interrupt event
    }

    // clean up card insertion IST
    if (NULL != pHardwareContext->hCardInsertInterruptThread)
    {
        CloseHandle(pHardwareContext->hCardInsertInterruptThread);
        pHardwareContext->hCardInsertInterruptThread = NULL;
    }

    // Release GPIO and IOMUX pin
    BspSdhcCardDetectDeinitialize(pHardwareContext->pBspSpecificContext);

}
//------------------------------------------------------------------------------
//
// Function: SetupCardDetectIST
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
BOOL SetupCardDetectIST(PSDH_HARDWARE_CONTEXT pHardwareContext)
{

    HANDLE hCardInsertInterruptThread;    // card insert/remove interrupt event
    DWORD threadID;                         // thread ID
    PVOID pContext = NULL  ;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+SetupCardDetectIST\n")));

    //Setup  interrupt pin gpio,
    pContext = BspSdhcCardDetectInitialize(pHardwareContext->ControllerIndex);
    if (pContext == NULL)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("SdhcCardDetectSetup FAIL!!\r\n")));
        goto _errExit;
    } else
    {
        pHardwareContext->pBspSpecificContext = pContext ;
    }

    //Register card interrupt
    pHardwareContext->hCardInsertInterruptEvent =
    BspSdhcRegisterCardInterrupt(&(pHardwareContext->dwSysIntrCardDetect));

    if (pHardwareContext->hCardInsertInterruptEvent == NULL)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("BspSdhcRegisterCardInterrupt FAIL!!\r\n")));
        goto _errExit;
    }

    // create the interrupt thread to handle card insertion events
    hCardInsertInterruptThread =
    CreateThread(NULL,
                 0,
                 (LPTHREAD_START_ROUTINE)SDCardDetectIstThread,
                 pHardwareContext,
                 0,
                 &threadID);
    if (NULL == hCardInsertInterruptThread)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("SetupCardDetectIST: unable to create card detect thread!\r\n")));
        goto _errExit;
    }

    pHardwareContext->hCardInsertInterruptThread = hCardInsertInterruptThread;

    return TRUE;

    _errExit:
    CleanupCardDetectIST(pHardwareContext);
    return FALSE;
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
    //SetClock(pController, TRUE);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-SoftwareReset\r\n")));
}

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

/*******************************************************************************
 END OF FILE
*******************************************************************************/
// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE
