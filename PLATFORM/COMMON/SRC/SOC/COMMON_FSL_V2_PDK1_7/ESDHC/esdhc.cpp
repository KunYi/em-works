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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved. 
//

// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE

/*---------------------------------------------------------------------------
* Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
// Module Name:
//
//    esdhc.cpp
//
// Abstract:
//
//    Freescale ESDHC implementation
//
// Notes:
//
///////////////////////////////////////////////////////////////////////////////

#include "esdhc.h"

// Global Variables

//------------------------------------------------------------------------------
//  Local Structures

//Debug the Power IOCTL's
//#define PM_TRACE 0
#define SDCARD_ZONE_POWER          DEBUGZONE(0)

#define IndicateSlotStateChange(event) \
    SDHCDIndicateSlotStateChange(m_pHCContext, \
    (UCHAR) 0, (event))

#define GetAndLockCurrentRequest() \
    SDHCDGetAndLockCurrentRequest(m_pHCContext, (UCHAR) 0)    

#define GET_PCONTROLLER_FROM_HCD(pHCDContext) \
    GetExtensionFromHCDContext(PCESDHCBase, pHCDContext)

#define TRANSFER_SIZE(pRequest)            ((pRequest)->BlockSize * (pRequest)->NumBlocks)

#define SDHC_IOCTL_LOAD_CLIENT   CTL_CODE(FILE_DEVICE_DATALINK, 3024, METHOD_BUFFERED, FILE_ANY_ACCESS)

#ifdef DEBUG
// dump the current request info to the debugger
static 
VOID 
DumpRequest(
            PSD_BUS_REQUEST pRequest,
            DWORD dwZone
            )
{   
    PREFAST_DEBUGCHK(pRequest);

    if (dwZone) {
        DEBUGMSG(1, (TEXT("DumpCurrentRequest: 0x%08X\n"), pRequest)); 
        DEBUGMSG(1, (TEXT("\t Command: %d\n"),  pRequest->CommandCode));
        DEBUGMSG(1, (TEXT("\t Argument: 0x%08x\n"),  pRequest->CommandArgument));
        DEBUGMSG(1, (TEXT("\t ResponseType: %d\n"),  pRequest->CommandResponse.ResponseType)); 
        DEBUGMSG(1, (TEXT("\t NumBlocks: %d\n"),  pRequest->NumBlocks)); 
        DEBUGMSG(1, (TEXT("\t BlockSize: %d\n"),  pRequest->BlockSize)); 
        DEBUGMSG(1, (TEXT("\t HCParam: %d\n"),    pRequest->HCParam)); 
    }
}
#else
#define DumpRequest(ptr, dw)
#endif

CESDHCBase::CESDHCBase()
{
    m_fCommandCompleteOccurred = FALSE;
    m_fFirstTime = TRUE;
    m_hControllerISTEvent = NULL;
    m_htControllerIST = NULL;
    m_dwControllerSysIntr = (DWORD) SYSINTR_UNDEFINED;

    m_pESDHCReg = NULL;
    m_fCardPresent = FALSE;
    m_fSDIOInterruptsEnabled = FALSE;

    m_dwMaxTimeout = DEFAULT_TIMEOUT_VALUE;
    m_bReinsertTheCard = FALSE;
    m_fWakeupSource = FALSE;
    m_fHighSpeedSupport = FALSE;
    m_fAutoCMD12Success = FALSE;
    m_fUseDMA = FALSE;
    m_fADMASupport = FALSE;
    m_fDisableDMA = FALSE;
    m_fUseExternalDMA = FALSE;

    m_pCurrentRequest = NULL;
    m_fCurrentRequestFastPath = FALSE;    
    m_FastPathStatus = SD_API_STATUS_UNSUCCESSFUL;

    m_SlotDma = NULL;
    m_dwEDMAChanTx = 0;
    m_dwEDMAChanRx = 0;

    m_PowerState = D4;
    m_dwRCA = 0;
    m_fCardIsSDIO = FALSE;
    m_dwVendorVersion = 0;
    m_fSDIOInjectDelay = FALSE;
}

//  Reset the controller. 
VOID CESDHCBase::SoftwareReset( void)
{
    // Reset the controller
    INSREG32BF(&m_pESDHCReg->SYSCTL, ESDHC_SYSCTL_RSTA, 1);

    // wait for reset to complete
    while (INREG32(&m_pESDHCReg->SYSCTL) & CSP_BITFMASK(ESDHC_SYSCTL_RSTA));

}


//  Send CMD52 to reset SDIO. 
//  It assumes that the power state of controller has been set to D0
VOID CESDHCBase::SoftwareResetSDIO( void)
{
    DWORD  dwStatEn, dwSigEn;
    DWORD  dwXferTypReg, dwArgReg;

    if (m_fFirstTime) 
    {
        m_fFirstTime = FALSE;
        // send the initialization clocks (80) before first command is sent
        SETREG32(&m_pESDHCReg->SYSCTL, CSP_BITFMASK(ESDHC_SYSCTL_INITA));
        // wait until done sending init clocks
        while(INREG32(&m_pESDHCReg->SYSCTL) & CSP_BITFMASK(ESDHC_SYSCTL_INITA));
    }    

    // save status enable/signal enable regs
    dwStatEn = INREG32(&m_pESDHCReg->IRQSTATEN);
    dwSigEn = INREG32(&m_pESDHCReg->IRQSIGEN);

    // only keep CC and some error status enable bits
    OUTREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_CCSEN) | ESDHC_CMD_ERROR_BITS );
    OUTREG32(&m_pESDHCReg->IRQSIGEN, 0);

    // CMD 52
    dwXferTypReg = (CSP_BITFVAL(ESDHC_XFERTYP_CMDINX,  SD_CMD_IO_RW_DIRECT) |
                             CSP_BITFVAL(ESDHC_XFERTYP_CMDTYP, ESDHC_CMD_NORMAL) |  //ESDHC_CMD_ABORT
                             CSP_BITFVAL(ESDHC_XFERTYP_RSPTYP, ESDHC_RSPLEN_48));

    // write a 1 to the RES bit in the CCCR to reset SDIO                              
    dwArgReg = (DWORD)BUILD_IO_RW_DIRECT_ARG((UCHAR)SD_IO_WRITE, 0 , 0 ,SD_IO_REG_IO_ABORT , SD_IO_REG_IO_ABORT_RES);                      

    OUTREG32(&m_pESDHCReg->CMDARG, dwArgReg); 
    OUTREG32(&m_pESDHCReg->XFERTYP, dwXferTypReg);

    //wait for the cmd completed
    while(! (INREG32(&m_pESDHCReg->IRQSTAT) & (CSP_BITFMASK(ESDHC_IRQSTAT_CC) | ESDHC_CMD_ERROR_BITS)));
    if (!(INREG32(&m_pESDHCReg->IRQSTAT) & ESDHC_CMD_ERROR_BITS ))
    {
        DWORD respBuff[4];

        // poll on DAT[0] for busy signalling
        while(! (INREG32(&m_pESDHCReg->PRSSTAT) & CSP_BITFVAL(ESDHC_PRSSTAT_DLSL, 1)));
 
        respBuff[0] = INREG32(&m_pESDHCReg->CMDRSP0);
        respBuff[1] = INREG32(&m_pESDHCReg->CMDRSP1);
        respBuff[2] = INREG32(&m_pESDHCReg->CMDRSP2);
        respBuff[3] = INREG32(&m_pESDHCReg->CMDRSP3);   
 
    }

    //clear status
    OUTREG32(&m_pESDHCReg->IRQSTAT, (CSP_BITFMASK(ESDHC_IRQSTAT_CC) | ESDHC_CMD_ERROR_BITS));

    // restore status enable/signal enable regs
    OUTREG32(&m_pESDHCReg->IRQSTATEN, dwStatEn);
    OUTREG32(&m_pESDHCReg->IRQSIGEN, dwSigEn);
}

// Set up the controller according to the interface parameters.
// NOTE: Although ESDHC hardware supports 8-bit MMC, CE 6.0 (R2) stack does not support it, so 8-bit support not implemented.
VOID CESDHCBase::SetInterface(PSD_CARD_INTERFACE pInterface)
{            
    DEBUGCHK(pInterface);

    if (m_PowerState != D0)
    {
        BspESDHCSetClockGating(D0);
        m_PowerState = D0;
    }


    if (SD_INTERFACE_SD_MMC_1BIT == pInterface->InterfaceMode) 
    {
        DEBUGMSG(SDCARD_ZONE_INIT, (L"CESDHCBase::SetInterface: Setting for 1 bit mode\r\n"));

        INSREG32BF(&m_pESDHCReg->PROCTL, ESDHC_PROCTL_DTW, ESDHC_DTW_1BIT);
        DEBUGMSG(SDCARD_ZONE_INIT, (L"CESDHCBase::SetInterface: PROCTL value = 0x%X\r\n", INREG32(&m_pESDHCReg->PROCTL) ));
    } 
    else if (SD_INTERFACE_SD_4BIT == pInterface->InterfaceMode) 
    {
        DEBUGMSG(SDCARD_ZONE_INIT, (L"CESDHCBase::SetInterface: Setting for 4 bit mode \r\n"));

        INSREG32BF(&m_pESDHCReg->PROCTL, ESDHC_PROCTL_DTW, ESDHC_DTW_4BIT);
        DEBUGMSG(SDCARD_ZONE_INIT, (L"CESDHCBase::SetInterface: PROCTL value = 0x%X\r\n", INREG32(&m_pESDHCReg->PROCTL) ));
    } 
    else 
    {
        // no support in sdbus driver for 8-bit MMC even though ESDHC supports it.
        DEBUGCHK(FALSE);
    }

    SetClockRate(&pInterface->ClockRate);

    BspESDHCSetClockGating(D4);
    m_PowerState = D4;    
}


// Enable SDIO Interrupts.
VOID CESDHCBase::EnableSDIOInterrupts()
{
    DEBUGMSG(SHC_INTERRUPT_ZONE, (L"+CESDHCSlot::EnableSDIOInterrupts\r\n"));

    SETREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_CINTIEN));

    m_fSDIOInterruptsEnabled = TRUE;

}

// Disable SDIO Interrupts.
VOID CESDHCBase::DisableSDIOInterrupts()
{            
    DEBUGMSG(SHC_INTERRUPT_ZONE, (L"+CESDHCSlot::DisableSDIOInterrupts\r\n"));

    CLRREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_CINTIEN));

    m_fSDIOInterruptsEnabled = FALSE;

}

//  Set clock rate based on HC capability
VOID CESDHCBase::SetClockRate(PDWORD pdwRate)
{
    ULONG ulMaxSDClk = BspESDHCGetBaseClk();
    ULONG ulMinSDClk = ulMaxSDClk >> 12;  // BaseClk / 4096 is the minimum SD clock
    const ULONG ulMaxDivisor = 16;
    const ULONG ulMaxPrescaler = 256;
    ULONG ulDiv =1, ulPrescaler = 1, ulErr = 0xFFFFFFFF;
    ULONG ulCurrRate = 0, ulCurrErr = 0, i = 0, j = 0;

    DWORD rate = *pdwRate;

    if( rate > m_dwMaxClockRate)
        rate = m_dwMaxClockRate;

    else if (rate < ulMinSDClk)
        rate = ulMinSDClk;

    // find the best values for Divisor and Prescalar 
    for (i = 1; i <= ulMaxDivisor; i++)
    {
        for (j = 1; j < ulMaxPrescaler; j <<= 1)
        {
            ulCurrRate = ulMaxSDClk / (i * j);

            if (ulCurrRate <= rate)
            {
                ulCurrErr = rate - ulCurrRate;
                if (ulCurrErr <= ulErr)
                {
                    ulDiv = i;
                    ulPrescaler = j;
                    ulErr = ulCurrErr;
                }

                // we are done if we can generate the exact rate asked for
                if (ulErr == 0)
                    break;

            }

        }

        // we are done if we can generate the exact rate asked for
        if (ulErr == 0)
            break;

    }

    // Actual rate to be generated
    rate = ulMaxSDClk / (ulDiv * ulPrescaler);

    DEBUGMSG(SHC_CLOCK_ZONE, (TEXT("SetClockRate - Requested Rate: %d, Setting clock rate to %d Hz \n"), *pdwRate, rate ));

    // Map the divisors to their respective register bit values
    ulDiv--;
    ulPrescaler >>= 1;

    CLRREG32(&m_pESDHCReg->SYSCTL, CSP_BITFMASK(ESDHC_SYSCTL_SDCLKEN));

    // set the clock rate
    INSREG32BF(&m_pESDHCReg->SYSCTL, ESDHC_SYSCTL_DVS, ulDiv);
    INSREG32BF(&m_pESDHCReg->SYSCTL, ESDHC_SYSCTL_SDCLKFS, ulPrescaler);

    // set the data timeout value to be maximum
    INSREG32BF(&m_pESDHCReg->SYSCTL, ESDHC_SYSCTL_DTOCV, ESDHC_MAX_DATA_TIMEOUT_COUNTER_VAL);

    // wait for clock to stabilize (SDSTB and SDCLKEN bits only available in version 2.2 and later)
    if (m_dwVendorVersion >= ESDHC_VVN_FSL22)
        while(! CSP_BITFEXT(INREG32(&m_pESDHCReg->PRSSTAT), ESDHC_PRSSTAT_SDSTB));

    SETREG32(&m_pESDHCReg->SYSCTL, CSP_BITFMASK(ESDHC_SYSCTL_SDCLKEN));

    // return the actual frequency
    *pdwRate = rate;
}

//  Turn on clock gating: clocks off when no transaction pending or ongoing
VOID CESDHCBase::ClockGateOn()
{
    DWORD dwClockEnableBits = CSP_BITFMASK(ESDHC_SYSCTL_PEREN) | CSP_BITFMASK(ESDHC_SYSCTL_HCKEN) | CSP_BITFMASK(ESDHC_SYSCTL_IPGEN);

    // clear sys control reg bits for each clock source (PERCLK, HCLK, and IPG_CLK)
    // to enable clock gating
    CLRREG32(&m_pESDHCReg->SYSCTL, dwClockEnableBits);
}

//  Turn off clock gating: clocks on all times except when buffer overrun/underrun error may occur
VOID CESDHCBase::ClockGateOff()
{
    DWORD dwClockEnableBits = CSP_BITFMASK(ESDHC_SYSCTL_PEREN) | CSP_BITFMASK(ESDHC_SYSCTL_HCKEN) | CSP_BITFMASK(ESDHC_SYSCTL_IPGEN);

    // set  sys control reg bits for each clock source (PERCLK, HCLK, and IPG_CLK)
    // to disable clock gating
    SETREG32(&m_pESDHCReg->SYSCTL, dwClockEnableBits);

}


//------------------------------------------------------------------------------
//
// Function: TransferIsSDIOSuspend
//
//  Checks if request is an SDIO suspend 
//                  (CMD52, Function 0, )?
//
// Parameters:
//        Cmd[in] - Command index
//        Arg[in] - Command argument
//
//  Returns:
//        Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
inline BOOL CESDHCBase::TransferIsSDIOSuspend(UINT8 Cmd, UINT32 Arg)
{
    if (Cmd == SD_CMD_IO_RW_DIRECT)
    {
        if (IO_RW_DIRECT_ARG_FUNC(Arg) == 0)
        {
            if (IO_RW_DIRECT_ARG_ADDR(Arg) == SD_IO_REG_BUS_SUSPEND)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: TransferIsSDIOAbort
//
//  Checks if request is an SDIO abort 
//                  (CMD52, Function 0, I/O Abort Reg)?
//
// Parameters:
//        Cmd[in] - Command index
//        Arg[in] - Command argument
//
//  Returns:
//        Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
inline BOOL CESDHCBase::TransferIsSDIOAbort(UINT8 Cmd, UINT32 Arg)
{
    if (Cmd == SD_CMD_IO_RW_DIRECT)
    {
        if (IO_RW_DIRECT_ARG_FUNC(Arg) == 0)
        {
            if (IO_RW_DIRECT_ARG_ADDR(Arg) == SD_IO_REG_IO_ABORT)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}


// Issues the specified SDI command
SD_API_STATUS CESDHCBase::SendCommand( PSD_BUS_REQUEST pRequest )
{
    SD_API_STATUS status = SD_API_STATUS_PENDING;
    DWORD dwXferTypReg = 0;
    DWORD dwWML = 0;

    UCHAR ucCmd = pRequest->CommandCode;
    UINT32 Arg = pRequest->CommandArgument;
    UINT32 respType = pRequest->CommandResponse.ResponseType;
    UINT32 TransferClass = pRequest->TransferClass;

    if (m_PowerState != D0)
    {
        BspESDHCSetClockGating(D0);
        m_PowerState = D0;
    }

    DWORD dwPresStatReg = INREG32(&m_pESDHCReg->PRSSTAT);

    // Check for AutoCMD12 response
    if (ucCmd == SD_CMD_STOP_TRANSMISSION && m_fAutoCMD12Success)
    {
        DEBUGMSG(SHC_SEND_ZONE, (TEXT("SendCommand: AutoCMD12 Succeeded, bypass CMD12.\n")));
        // The response for Auto CMD12 is in a special area
        UNALIGNED DWORD *pdwResponseBuffer = 
            (PDWORD) (pRequest->CommandResponse.ResponseBuffer + 1); // Skip CRC
        *pdwResponseBuffer = INREG32(&m_pESDHCReg->CMDRSP3);
        IndicateBusRequestComplete(pRequest, SD_API_STATUS_SUCCESS);
        status = SD_API_STATUS_SUCCESS;

        goto Exit;
    }

    m_fAutoCMD12Success = FALSE;

    // Can't write to XFERTYP register while CIHB is set
    if (dwPresStatReg & CSP_BITFMASK(ESDHC_PRSSTAT_CIHB))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"SendCommand: CIHB set. Cannot send command.\r\n"));
        //DumpRequest(pRequest, SDCARD_ZONE_ERROR);
        status = SD_API_STATUS_DEVICE_NOT_RESPONDING;
        goto Exit;
    }

    // For commands with data, can't write to XFERTYP while CDIHB is set
    if ((TransferClass != SD_COMMAND || respType == ResponseR1b) && (dwPresStatReg & CSP_BITFMASK(ESDHC_PRSSTAT_CDIHB)))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"SendCommand: CDIHB set. Cannot send command.\r\n"));
        //DumpRequest(pRequest, SDCARD_ZONE_ERROR);
        status = SD_API_STATUS_DEVICE_NOT_RESPONDING;
        goto Exit;
    }

    DEBUGMSG(SHC_SEND_ZONE, (L"CESDHCBase::SendCommand: "
        L"Cmd = 0x%x Arg = 0x%x respType = 0x%x TransferClass = 0x%x, NumBlocks = %d, BlockSize = %d\r\n", ucCmd, Arg, respType, TransferClass, pRequest->NumBlocks, pRequest->BlockSize));

    // Workaround for random CRC errors on a write command: ENGR69016
    // required for SD/MMC write commands (CMD24, CMD25) as well as SDIO write commands (CMD52, CMD53)
    if (m_dwVendorVersion < ESDHC_VVN_FSL22 && TransferClass == SD_WRITE)
    {
        DWORD dwProctlReg = INREG32(&m_pESDHCReg->PROCTL);
        DWORD dwIrqStatEnReg = INREG32(&m_pESDHCReg->IRQSTATEN); 
        DWORD dwIrqSigEnReg = INREG32(&m_pESDHCReg->IRQSIGEN); 

        // reset the CMD portion of the controller
        INSREG32BF(&m_pESDHCReg->SYSCTL, ESDHC_SYSCTL_RSTC, 1);
        // reset the DATA portion of the controller
        INSREG32BF(&m_pESDHCReg->SYSCTL, ESDHC_SYSCTL_RSTD, 1);

        // restore registers that are affected by data reset
        OUTREG32(&m_pESDHCReg->PROCTL, dwProctlReg);
        OUTREG32(&m_pESDHCReg->IRQSTATEN, dwIrqStatEnReg);
        OUTREG32(&m_pESDHCReg->IRQSIGEN, dwIrqSigEnReg);

    }

    // Set the command index
    dwXferTypReg = CSP_BITFVAL(ESDHC_XFERTYP_CMDINX, ucCmd);

    // Is it a suspend command?
    if (TransferIsSDIOSuspend(ucCmd, Arg))
    {
        // Implement according to ESDHC spec ...
        // send cmd as normal, the check BS, if 0, send another cmd as suspend, save context
    }

    // Is it an abort command?
    else if (ucCmd == SD_CMD_STOP_TRANSMISSION || TransferIsSDIOAbort(ucCmd, Arg))
    {
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_CMDTYP, ESDHC_CMD_ABORT);
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_RSPTYP, ESDHC_RSPLEN_48B);
    }

    else
    {
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_CMDTYP, ESDHC_CMD_NORMAL);    
    }

    // Enable Command Complete and Transfer Complete interrupt
    SETREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_CCSEN));
    if (!m_fCurrentRequestFastPath)
        SETREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_CCIEN));


    SETREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_TCSEN));
    if (!m_fCurrentRequestFastPath)
        SETREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_TCIEN));


    // set DPSEL and block size for transfers that are not command-only
    if (TransferClass != SD_COMMAND)
    {    
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_DPSEL, 1);

        // Use DMA?: Only if current request is not a fast-path request and DMA object exists
        if (!m_fCurrentRequestFastPath && m_SlotDma &&  m_SlotDma->ArmDMA(*pRequest,TransferClass == SD_WRITE)) 
        {
            m_fUseDMA = TRUE;

            // internal DMA
            if (!m_fUseExternalDMA)
            {
                // Enable internal DMA interrupt
                SETREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_DINTSEN));
                SETREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_DINTIEN));

                INSREG32(&m_pESDHCReg->PROCTL, CSP_BITFMASK(ESDHC_PROCTL_DMAS),
                    CSP_BITFVAL(ESDHC_PROCTL_DMAS, m_SlotDma->DmaSelectBit()));
                // Enable DMA in XFERTYP register
                dwXferTypReg |= CSP_BITFMASK(ESDHC_XFERTYP_DMAEN);

                dwWML = min(pRequest->BlockSize, ESDHC_MAX_DATA_BUFFER_SIZE >> 1);
                OUTREG32(&m_pESDHCReg->WML, CSP_BITFVAL(ESDHC_WML_RDWML, dwWML >> 2) |
                    CSP_BITFVAL(ESDHC_WML_WRWML, dwWML >> 2) );

                // disable tc irq here when use internal dma as it cause some trouble??
                if (m_dwVendorVersion < ESDHC_VVN_FSL22)
                    CLRREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_TCIEN));

            }

        }

        // enable BRR and BWR interrupts, and set max WML when not using DMA
        else
        {
            if (TransferClass == SD_READ)
            {
                // enable Buffer for Read Ready interrupt
                SETREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_BRRSEN));
                if (!m_fCurrentRequestFastPath)
                    SETREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_BRRIEN));
            }

            else if (TransferClass == SD_WRITE)
            {
                // enable Buffer for Write Ready interrupt
                SETREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_BWRSEN));
                if (!m_fCurrentRequestFastPath)
                    SETREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_BWRIEN));
            }

            // set watermark level (WML register needs to be set to 4-byte words, not bytes)
            dwWML = min(pRequest->BlockSize, ESDHC_MAX_DATA_BUFFER_SIZE >> 1);
            // burstlength should be at least 4 bytes even if blocksize is less than 4
            dwWML = max(dwWML, 4);

            OUTREG32(&m_pESDHCReg->WML, CSP_BITFVAL(ESDHC_WML_RDWML, dwWML >> 2) |
                CSP_BITFVAL(ESDHC_WML_WRWML, dwWML >> 2) );
        }

        // setup blocksize and number of blocks to transfer (= 1 for single transfer)
        OUTREG32(&m_pESDHCReg->BLKATTR, CSP_BITFVAL(ESDHC_BLKATTR_BLKSIZE, pRequest->BlockSize) |
            CSP_BITFVAL(ESDHC_BLKATTR_BLKCNT, pRequest->NumBlocks)  );

        // Multi-block transfer? Set BCEN, and MSBSEL bits
        if (pRequest->NumBlocks > 1)
        {
            dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_MSBSEL, 1);
            dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_BCEN, 1);

            if (pRequest->Flags & SD_AUTO_ISSUE_CMD12)
            {
                dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_AC12EN, 1);
            }          
        }

        // set DTDSEL for reads from card
        if (TransferClass == SD_READ)
        {
            dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_DTDSEL, 1);
        }

    }

    switch( respType )
    {
    case NoResponse:
        // RSPTYP, CICEN, and CCCEN remain 0, nothing to do
        break;

    case ResponseR1:
    case ResponseR5:
    case ResponseR6:
    case ResponseR7:
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_RSPTYP, ESDHC_RSPLEN_48);
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_CICEN, 1);
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_CCCEN, 1);
        break;

    case ResponseR2:
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_RSPTYP, ESDHC_RSPLEN_136);
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_CCCEN, 1);
        break;

    case ResponseR3:
    case ResponseR4:
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_RSPTYP, ESDHC_RSPLEN_48);
        break;

        // Some R5 CMDs will be treated by ESDHC as R5b (need to define here), for eg. CMD52 for I/O Abort ...
    case ResponseR1b:                
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_RSPTYP, ESDHC_RSPLEN_48B);
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_CICEN, 1);
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_CCCEN, 1);
        break;

    default:
        status = SD_API_STATUS_INVALID_PARAMETER;
        goto Exit;
        break;
    }

    // workaround for ESDHC errata ENGcm05286 in non-1-bit SDIO mode.
    // if DAT1 is low (SDIO card is signaling an interrupt), ignore false DEBE error that occurs on write command
    if (m_fCardIsSDIO && (m_dwVendorVersion < ESDHC_VVN_FSL22) && TransferClass == SD_WRITE && 
        (CSP_BITFEXT(INREG32(&m_pESDHCReg->PROCTL), ESDHC_PROCTL_DTW) != ESDHC_DTW_1BIT) )
    {
        CLRREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_DEBESEN));
    }

    // Write the command argument, and send the command
    OUTREG32(&m_pESDHCReg->CMDARG, Arg);
    OUTREG32(&m_pESDHCReg->XFERTYP, dwXferTypReg);

    // save RCA for CMD13 later on (if necessary)
    if (ucCmd == SD_CMD_SELECT_DESELECT_CARD)
        m_dwRCA = Arg;

Exit:
    return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Remove the device instance in the slot
VOID CESDHCBase::HandleRemoval(BOOL fCancelRequest)
{    
    m_fCardPresent = FALSE;
    m_fCardIsSDIO = FALSE;    
    m_bReinsertTheCard = FALSE;

    // turn off SDIO interrupts
    if( m_fSDIOInterruptsEnabled )
    {
        DisableSDIOInterrupts();
    }

    if (fCancelRequest) {
        // get the current request  
        PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

        if (pRequest != NULL) {
            DEBUGMSG(SDCARD_ZONE_WARN, (L"CESDHCBase::HandleRemoval: "
                L"Card Removal Detected - Canceling current request: 0x%08X, command: %d\r\n", 
                pRequest, pRequest->CommandCode));

            // notify DMA engine to cancel current transfer
            if(m_fUseDMA && m_SlotDma)
                m_SlotDma->DMANotifyEvent(*pRequest, DMA_ERROR_OCCOR);     

            DumpRequest(pRequest, SDCARD_ZONE_WARN);
            IndicateBusRequestComplete(pRequest, SD_API_STATUS_DEVICE_REMOVED);       

        }

        // reset the controller in case of any errors that occured due to device removal

        DWORD dwProctlReg = INREG32(&m_pESDHCReg->PROCTL);
        // reset the CMD portion of the controller
        INSREG32BF(&m_pESDHCReg->SYSCTL, ESDHC_SYSCTL_RSTC, 1);
        // reset the DATA portion of the controller
        INSREG32BF(&m_pESDHCReg->SYSCTL, ESDHC_SYSCTL_RSTD, 1);

        // reintialize some of the registers affected by the reset
        OUTREG32(&m_pESDHCReg->PROCTL, dwProctlReg);

        OUTREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_DMAEIEN));
        OUTREG32(&m_pESDHCReg->IRQSTATEN, ESDHC_ERROR_BITS | CSP_BITFMASK(ESDHC_IRQSTATEN_CINTSEN));


    }

    IndicateSlotStateChange(DeviceEjected);

    // Gate clocks
    BspESDHCSetClockGating(D4);
    m_PowerState = D4;

    // Turn card power off
    m_dwCurrentPowerLevel = 0;
    BspESDHCSetSlotVoltage(m_dwCurrentPowerLevel);    

    // Enable Card Insert interrupt
    //SETREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_CINSSEN));  
    //SETREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_CINSIEN));    
    BspESDHCCardDetectInt(TRUE);

}


// Initialize the card
VOID CESDHCBase::HandleInsertion()
{
    DWORD dwClockRate = SD_DEFAULT_CARD_ID_CLOCK_RATE;

    m_fCardPresent = TRUE;

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CESDHCBase::HandleInsertion\r\n"));

    // turn power to the card on. For initialization, cards need ~ 3V
    m_dwCurrentPowerLevel = SD_VDD_WINDOW_3_0_TO_3_1;
    BspESDHCSetSlotVoltage(m_dwCurrentPowerLevel);

    // turn clocks on
    BspESDHCSetClockGating(D0);
    m_PowerState = D0;

    // Disable the Card Insert interrupt in IRQSIGEN to avoid repeated interrupts
    //CLRREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_CINSIEN));
    // W1C the Card Insert interrupt in IRQSTATUS to clear it
    //OUTREG32(&m_pESDHCReg->IRQSTAT, CSP_BITFMASK(ESDHC_IRQSTAT_CINS));
    BspESDHCCardDetectInt(FALSE);

    SetClockRate(&dwClockRate);

    // Send at least 74 clocks to the card over the course of at least 1 ms
    // with allowance for power supply ramp-up time. (SD Phys Layer 6.4)
    // Note that power supply ramp-up time occurs in SetVoltage().
    DWORD dwSleepMs = (74 / (dwClockRate / 1000)) + 1;
    Sleep(dwSleepMs);

    // turn off SDIO interrupts
    if( m_fSDIOInterruptsEnabled )
    {
        DisableSDIOInterrupts();
    }

    // This is a workaround to give a software reset for the sdio card at host level,
    // for some SDIO WIFI card will not be reset when suspend/resume or warm restart
    SoftwareResetSDIO();
    
    // indicate device arrival
    IndicateSlotStateChange(DeviceInserted);

    // Enable the card removal interrupt
    //SETREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_CRMSEN)); 
    //SETREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_CRMIEN)); 
}

VOID CESDHCBase::HandleCommandErrors()
{
    {
        SD_API_STATUS status = SD_API_STATUS_SUCCESS;

        // get the current request  
        PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

        DWORD dwErrorStatus = INREG32(&m_pESDHCReg->IRQSTAT) & ESDHC_CMD_ERROR_BITS;

        DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("HandleCommandErrors - STATUS ERROR BITS=0x%08X\n"), dwErrorStatus));

        if (pRequest) {
            DumpRequest(pRequest, SDCARD_ZONE_ERROR);
            DumpRegisters();
        }

        //DEBUGCHK( (dwErrorStatus & ERR_INT_STATUS_VENDOR) == 0 );

        if (dwErrorStatus) {
            if ( dwErrorStatus & CSP_BITFMASK(ESDHC_IRQSTAT_CTOE) ) {
                status = SD_API_STATUS_RESPONSE_TIMEOUT;
            }

            if ( dwErrorStatus & CSP_BITFMASK(ESDHC_IRQSTAT_CCE) ) {
                status = SD_API_STATUS_CRC_ERROR;
                if ( dwErrorStatus & CSP_BITFMASK(ESDHC_IRQSTAT_CTOE)  )
                    status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
            }

            if ( dwErrorStatus & CSP_BITFMASK(ESDHC_IRQSTAT_CEBE)  ) {
                status = SD_API_STATUS_RESPONSE_TIMEOUT;
            }

            if ( dwErrorStatus & CSP_BITFMASK(ESDHC_IRQSTAT_CIE)  ) {
                status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
            }

            if (dwErrorStatus & ESDHC_CMD_ERROR_BITS) {

                // Reset CMD line
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("HandleCommandErrors - Command line error (0x%x). Resetting CMD line.\r\n"), dwErrorStatus));

                // reset the CMD portion of the controller
                INSREG32BF(&m_pESDHCReg->SYSCTL, ESDHC_SYSCTL_RSTC, 1);

            }

            // clear all error status
            OUTREG32(&m_pESDHCReg->IRQSTAT, dwErrorStatus);

            // complete the request
            if (pRequest) {
                IndicateBusRequestComplete(pRequest, status);
            }
        }
    }

}
VOID CESDHCBase::HandleDataErrors()
{

    SD_API_STATUS status = SD_API_STATUS_SUCCESS;

    // get the current request  
    PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

    DWORD dwErrorStatus = (INREG32(&m_pESDHCReg->IRQSTAT) & (ESDHC_DAT_ERROR_BITS | CSP_BITFMASK(ESDHC_IRQSTAT_DMAE) |
        CSP_BITFMASK(ESDHC_IRQSTAT_AC12E) ));

    DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("HandleDataErrors - STATUS ERROR BITS=0x%08X\n"), dwErrorStatus));

    if (pRequest) {
        DumpRequest(pRequest, SDCARD_ZONE_ERROR);
        DumpRegisters();
    }

    if (dwErrorStatus)
    {
        if ( dwErrorStatus & CSP_BITFMASK(ESDHC_IRQSTAT_DTOE)  ) {
            status = SD_API_STATUS_DATA_TIMEOUT;
        }

        if ( dwErrorStatus & CSP_BITFMASK(ESDHC_IRQSTAT_DCE)  ) {
            status = SD_API_STATUS_CRC_ERROR;
        }

        if ( dwErrorStatus & CSP_BITFMASK(ESDHC_IRQSTAT_DEBE)  ) {
            status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
        }

        if ( dwErrorStatus & CSP_BITFMASK(ESDHC_IRQSTAT_AC12E)  ) {
            status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
        }

        if (dwErrorStatus & CSP_BITFMASK(ESDHC_IRQSTAT_DMAE)) { // internal DMA Error
            if (m_SlotDma && pRequest ) {
                m_SlotDma->DMANotifyEvent(*pRequest, DMA_ERROR_OCCOR );
            }
            else {
                DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("HandleDataErrors - Internal DMA Error without DMA Enabled (0x%x).\r\n"), dwErrorStatus));
            }
        }

        // notify external DMA engine to stop the channel
        if(m_fUseDMA && m_fUseExternalDMA && m_SlotDma)
            m_SlotDma->DMANotifyEvent(*pRequest, DMA_ERROR_OCCOR);

        if (dwErrorStatus & ESDHC_DAT_ERROR_BITS) {

            // Reset DAT line
            DEBUGMSG(SDCARD_ZONE_ERROR, (TEXT("HandleDataErrors - Data line error (0x%x). Resetting CMD & DAT line.\r\n"), dwErrorStatus));

            DWORD dwProctlReg = INREG32(&m_pESDHCReg->PROCTL);
            DWORD dwIrqStatEnReg = INREG32(&m_pESDHCReg->IRQSTATEN); 
            DWORD dwIrqSigEnReg = INREG32(&m_pESDHCReg->IRQSIGEN); 

            // reset the CMD portion of the controller
            INSREG32BF(&m_pESDHCReg->SYSCTL, ESDHC_SYSCTL_RSTC, 1);
            // reset the DATA portion of the controller
            INSREG32BF(&m_pESDHCReg->SYSCTL, ESDHC_SYSCTL_RSTD, 1);

            // restore registers that are affected by data reset
            OUTREG32(&m_pESDHCReg->PROCTL, dwProctlReg);
            OUTREG32(&m_pESDHCReg->IRQSTATEN, dwIrqStatEnReg);
            OUTREG32(&m_pESDHCReg->IRQSIGEN, dwIrqSigEnReg);

        }

        // complete the request
        if (pRequest) {
            IndicateBusRequestComplete(pRequest, status);
        }

    }

}
VOID CESDHCBase::HandleReadReady()
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CESDHCBase::HandleReadReady\r\n"));

    PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

    if(pRequest)
    {
        if (pRequest->TransferClass != SD_READ)
        {
            goto EXIT;
        }

        __try {
            PDWORD pdwUserBuffer = (PDWORD) &pRequest->pBlockBuffer[pRequest->HCParam];
            PDWORD pdwBuffer = pdwUserBuffer;
            HLOCAL rgdwIntermediateBuffer = NULL;   // interim buffer that may be used if provided buffer is not DWORD aligned
            DWORD dwIntBufSize = 0;    // interim buffer size in units of bytes
            BOOL   fUsingIntermediateBuffer = FALSE;
            DWORD dwBurstLen = EXTREG32BF(&m_pESDHCReg->WML, ESDHC_WML_RDWML);
            DWORD dwRemainderBlkBytes = pRequest->BlockSize - (pRequest->HCParam % pRequest->BlockSize);
            DWORD dwBytesRead = 0;

            if (((DWORD) pdwUserBuffer) % 4 != 0) {
                // Buffer is not DWORD aligned so we must use an intermediate buffer.

                dwIntBufSize = (pRequest->BlockSize < 4) ? 4 : ((pRequest->BlockSize >> 2) + 1) << 2  ; // extra DWORD at the end
                rgdwIntermediateBuffer = LocalAlloc(LMEM_ZEROINIT, dwIntBufSize);

                if (rgdwIntermediateBuffer == NULL)
                    goto EXIT;


                pdwBuffer = (PDWORD) rgdwIntermediateBuffer;
                fUsingIntermediateBuffer = TRUE;
            }

            // we will read out burst length of data when BRR is the result of WML being reached
            if (dwRemainderBlkBytes >= (dwBurstLen << 2))
            {
                dwBytesRead = dwBurstLen << 2;

                while(dwBurstLen--)
                {
                    *(pdwBuffer++) = INREG32(&m_pESDHCReg->DATPORT);
                }
            }

            // else we got BRR because of remainder bytes in the block (< WML).
            // internal WM of ESDHC will reduce to remainder bytes in the block.
            else
            {
                dwBytesRead = dwRemainderBlkBytes;

                while(dwRemainderBlkBytes > 3)
                {
                    *(pdwBuffer++) = INREG32(&m_pESDHCReg->DATPORT);
                    dwRemainderBlkBytes -= 4;
                }

                // get the last partial word and write it safely (without overstepping the buffer end)
                if (dwRemainderBlkBytes)
                {
                    DWORD dwLastWord = INREG32(&m_pESDHCReg->DATPORT);
                    SDPerformSafeCopy(pdwBuffer, &dwLastWord, dwRemainderBlkBytes);
                }
            }

            pRequest->HCParam += dwBytesRead;

            if (fUsingIntermediateBuffer) {
                SDPerformSafeCopy(pdwUserBuffer, rgdwIntermediateBuffer, dwBytesRead);
                LocalFree(rgdwIntermediateBuffer);
            }

        }
        __except(SDProcessException(GetExceptionInformation())) {
            DEBUGMSG(SDCARD_ZONE_ERROR, (_T("Exception reading from client buffer!\r\n")));
            IndicateBusRequestComplete(pRequest, SD_API_STATUS_ACCESS_VIOLATION);
        }
    }

EXIT:
    // clear BRR interrupt status
    OUTREG32(&m_pESDHCReg->IRQSTAT, CSP_BITFMASK(ESDHC_IRQSTAT_BRR));

}

VOID CESDHCBase::HandleWriteReady()
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CESDHCBase::HandleWriteReady\r\n"));

    PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();

    if(pRequest)
    {
        if (pRequest->TransferClass != SD_WRITE)
        {
            goto EXIT;
        }

        __try {
            PDWORD pdwUserBuffer = (PDWORD) &pRequest->pBlockBuffer[pRequest->HCParam];
            PDWORD pdwBuffer = pdwUserBuffer;
            HLOCAL rgdwIntermediateBuffer = NULL;   // interim buffer that may be used if provided buffer is not DWORD aligned
            DWORD dwIntBufSize = 0;    // interim buffer size in units of bytes
            DWORD dwBurstLen = EXTREG32BF(&m_pESDHCReg->WML, ESDHC_WML_WRWML);
            DWORD dwRemainderBlkBytes = pRequest->BlockSize - (pRequest->HCParam % pRequest->BlockSize);
            DWORD dwBytesWritten = 0;
            DWORD dwLastWord = 0;

            if (((DWORD) pdwUserBuffer) % 4 != 0) {
                // Buffer is not DWORD aligned so we must use an
                // intermediate buffer.

                dwIntBufSize = (pRequest->BlockSize < 4) ? 4 : ((pRequest->BlockSize >> 2) + 1) << 2  ; // extra DWORD at the end
                rgdwIntermediateBuffer = LocalAlloc(LMEM_ZEROINIT, dwIntBufSize);

                if (rgdwIntermediateBuffer == NULL)
                    goto EXIT;

                pdwBuffer = (PDWORD) rgdwIntermediateBuffer;
                DWORD dwBytesToCopy = (dwRemainderBlkBytes >= (dwBurstLen << 2)) ? (dwBurstLen << 2): dwRemainderBlkBytes;
                SDPerformSafeCopy(rgdwIntermediateBuffer, pdwUserBuffer, dwBytesToCopy);}

            // we will write out burst length of data if bytes left in the block is at least dwBurstLen in size
            if (dwRemainderBlkBytes >= (dwBurstLen << 2))
            {
                dwBytesWritten = dwBurstLen << 2;

                while(dwBurstLen--)
                {
                    OUTREG32(&m_pESDHCReg->DATPORT, *(pdwBuffer++));
                }
            }

            // else we got BWR because of remainder bytes in the block
            else
            {
                dwBytesWritten = dwRemainderBlkBytes;

                while(dwRemainderBlkBytes > 3)
                {
                    OUTREG32(&m_pESDHCReg->DATPORT, *(pdwBuffer++));
                    dwRemainderBlkBytes -= 4;
                }

                if (dwRemainderBlkBytes)
                {
                    SDPerformSafeCopy(&dwLastWord, pdwBuffer, dwRemainderBlkBytes);
                    OUTREG32(&m_pESDHCReg->DATPORT, dwLastWord);
                }

            }

            pRequest->HCParam += dwBytesWritten;

            if (rgdwIntermediateBuffer)
                LocalFree(rgdwIntermediateBuffer);
        }
        __except(SDProcessException(GetExceptionInformation())) {
            DEBUGMSG(SDCARD_ZONE_ERROR, (_T("Exception reading from client buffer!\r\n")));
            IndicateBusRequestComplete(pRequest, SD_API_STATUS_ACCESS_VIOLATION);
        }
    }

EXIT:
    // clear BWR interrupt status
    OUTREG32(&m_pESDHCReg->IRQSTAT, CSP_BITFMASK(ESDHC_IRQSTAT_BWR));
}

VOID CESDHCBase::HandleTransferComplete()
{
    PSD_BUS_REQUEST pRequest;
    SD_API_STATUS   status = SD_API_STATUS_SUCCESS;
    DWORD dwStatusReg;

    // check for any errors
    if ( (dwStatusReg = INREG32(&m_pESDHCReg->IRQSTAT)) & (ESDHC_DAT_ERROR_BITS | CSP_BITFMASK(ESDHC_IRQSTAT_DMAE) |
        CSP_BITFMASK(ESDHC_IRQSTAT_AC12E)) ) {

            HandleDataErrors();
            return;
    }

    // get the current request  
    pRequest = GetAndLockCurrentRequest();

    if (pRequest) {
        if (pRequest->HCParam != TRANSFER_SIZE(pRequest)) {
            if (m_dwVendorVersion < ESDHC_VVN_FSL22){
                // this is a workaround for MX35 (and MX51 TO1) as we got tc done in case transfer is
                // far from complete in polling mode, ignore it simply
                return;
            }    
            // This means that a Transfer Complete interrupt occurred before
            // a Buffer Ready interrupt. Hardware should not allow this. 
            DEBUGCHK(FALSE);
            status = SD_API_STATUS_DEVICE_RESPONSE_ERROR;
        }
        if(INREG32(&m_pESDHCReg->PRSSTAT) & CSP_BITFVAL(ESDHC_PRSSTAT_CDIHB, 1)){
            if (m_dwVendorVersion < ESDHC_VVN_FSL22){
                // this is a workaround for mx35 as we got un-expected tc  ignore it simply
                return;
            }
        }


        // DMA should have already been notified to stop processing when DMA complete interrupt occurred, but just in case
        if (pRequest->TransferClass != SD_COMMAND && m_fUseDMA) {
            if (m_SlotDma && (m_SlotDma->IsDMACompleted(pRequest->TransferClass == SD_WRITE) != NO_DMA)) {
                m_SlotDma->DMANotifyEvent(*pRequest, TRANSFER_COMPLETED);
            }
        }
        // complete the AutoCMD12 request
        if (pRequest->Flags & SD_AUTO_ISSUE_CMD12) {       
            m_fAutoCMD12Success = TRUE;
        }

        // we only do busy polling for memory cards write commands, not for SDIO write commands
        if (pRequest->CommandCode == SD_CMD_WRITE_BLOCK || 
            // for multi-block write, if no AutoCMD12, then SDBUS driver should send CMD12 next, so no need to do busy polling
            (pRequest->CommandCode == SD_CMD_WRITE_MULTIPLE_BLOCK && m_fAutoCMD12Success)) 
        {
            // send CMD13 until card is in TRAN state (instead of DAT0 polling, which seems unreliable)
            do
            {   
                // clear interrupt status bits, and send CMD13
                OUTREG32(&m_pESDHCReg->IRQSTAT, 0xFFFFFFFF);
                OUTREG32(&m_pESDHCReg->CMDARG, m_dwRCA);
                OUTREG32(&m_pESDHCReg->XFERTYP, 0x0D1A0000);                    

                while(! (INREG32(&m_pESDHCReg->IRQSTAT) & (CSP_BITFMASK(ESDHC_IRQSTAT_CC) | ESDHC_CMD_ERROR_BITS)));

                if (INREG32(&m_pESDHCReg->IRQSTAT) & ESDHC_CMD_ERROR_BITS)
                {
                    SETREG32(&m_pESDHCReg->SYSCTL, CSP_BITFMASK(ESDHC_SYSCTL_RSTC));
                    break;

                }                        
            }while(SD_STATUS_CURRENT_STATE(INREG32(&m_pESDHCReg->CMDRSP0)) != SD_STATUS_CURRENT_STATE_TRAN);
        }


        // re-enable DEBE status bit in case it was disabled to avoid false DEBE error due to errata ENGcm05286
        SETREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_DEBESEN));

        // indicate to bus driver that this request completed
        IndicateBusRequestComplete(pRequest, status);
    }

}

void CESDHCBase::HandleInterrupts()
{
    // capture all the interrupts from status register
    DWORD dwStatus = INREG32(&m_pESDHCReg->IRQSTAT);

    DEBUGMSG(SHC_INTERRUPT_ZONE, (L"+CESDHCSlot::HandleInterrupts\r\n"));


    // check for any card insertion/removal interrupts: but not during fast path as the check could take several 
    // additional cycles if gpio is used for card detection.
    // allow platform-specific code to handle card detection changes
    // because card detect line may be connected differently on different platforms
    if (!m_fCurrentRequestFastPath && (BspESDHCSlotStatusChanged() || m_bReinsertTheCard))
    {
        // was card inserted?
        if (BspESDHCIsCardPresent())
        {

            // maybe card was removed and new card was inserted while system was suspended
            if (m_bReinsertTheCard) {
                HandleRemoval(TRUE);
            }

            DEBUGMSG(SHC_INTERRUPT_ZONE, (L"CESDHCSlot::HandleInterrupts: Card is Inserted!\r\n"));

            if( m_fCardPresent == FALSE ) {
                m_fFirstTime = TRUE;
                HandleInsertion();
            }
        }

        // card was removed
        else if (m_fCardPresent)
        {
            DEBUGMSG(SHC_INTERRUPT_ZONE, (L"CESDHCSlot::HandleInterrupts: Card is Removed!\r\n"));
            HandleRemoval(TRUE);
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
    }

    if(dwStatus & CSP_BITFMASK(ESDHC_IRQSTAT_CC))
    {
        OUTREG32(&m_pESDHCReg->IRQSTAT, CSP_BITFMASK(ESDHC_IRQSTAT_CC));

        if (dwStatus & (ESDHC_CMD_ERROR_BITS)) {
            HandleCommandErrors();           
        }

        else
        {
            m_fCommandCompleteOccurred = TRUE;
            CommandCompleteHandler();          

        }
    }

    if (m_fCommandCompleteOccurred)
    {

        if (dwStatus & CSP_BITFMASK(ESDHC_IRQSTAT_BRR))
        {
            HandleReadReady();           
        }

        if (dwStatus & CSP_BITFMASK(ESDHC_IRQSTAT_BWR))
        {          
            HandleWriteReady();
        }

        // check internal DMA completion interrupt
        if (dwStatus & CSP_BITFMASK(ESDHC_IRQSTAT_DINT))
        {
            PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();
            if (m_SlotDma && pRequest) {
                CLRREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_DINTSEN));
                CLRREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_DINTIEN));
                OUTREG32(&m_pESDHCReg->IRQSTAT, CSP_BITFMASK(ESDHC_IRQSTAT_DINT));    
                m_SlotDma->DMANotifyEvent(*pRequest, DMA_COMPLETE);
                dwStatus = INREG32(&m_pESDHCReg->IRQSTAT); //update it
                //enable the tc intr (in case it was cleared for VVN < FSL22)
                SETREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_TCIEN));
            }    
            else {
                ASSERT(FALSE);
            }
        }

        // check external DMA completion or error if we are using DMA for this particular request
        else if (m_fUseExternalDMA && m_fUseDMA)
        {
            PSD_BUS_REQUEST pRequest = GetAndLockCurrentRequest();
            if (pRequest)
            {
                // check to see if any event occured, if so the EDMA methods will handle it
                DMAEVENT dmaEvent = m_SlotDma->IsDMACompleted(pRequest->TransferClass == SD_WRITE);

                if (dmaEvent != NO_DMA)
                {
                    m_SlotDma->DMANotifyEvent(*pRequest, dmaEvent);
                }
            }
        }

    }

    if ((dwStatus & CSP_BITFMASK(ESDHC_IRQSTAT_TC)) && (m_dwVendorVersion >= ESDHC_VVN_FSL22 || 
          (!m_fUseDMA || ((INREG32(&m_pESDHCReg->IRQSIGEN) & CSP_BITFMASK(ESDHC_IRQSIGEN_TCIEN)))))){

            OUTREG32(&m_pESDHCReg->IRQSTAT, CSP_BITFMASK(ESDHC_IRQSTAT_TC));    
            HandleTransferComplete();

            // For MX35, Sychip Wifi card needs a delay (of at least 22 microsec) for after CMD53 is completed
            if (m_fSDIOInjectDelay)
            {
                DWORD dwXferTyp = INREG32(&m_pESDHCReg->XFERTYP);
                if (m_fCurrentRequestFastPath && (CSP_BITFEXT(dwXferTyp,  ESDHC_XFERTYP_CMDINX) == SD_CMD_IO_RW_EXTENDED))
                    StallExecution(25);
            }
    }   

    if(!m_fCurrentRequestFastPath && m_fSDIOInterruptsEnabled && (dwStatus & CSP_BITFMASK(ESDHC_IRQSTAT_CINT)))
    {
        ASSERT( m_fSDIOInterruptsEnabled );

        // indicate that the card is interrupting
        DEBUGMSG(SHC_INTERRUPT_ZONE, (L"CESDHCBase::HandleInterrupts: Received SDIO interrupt!\r\n"));

        // disable the SDIO interrupt
        DisableSDIOInterrupts();

        OUTREG32(&m_pESDHCReg->IRQSTAT, CSP_BITFMASK(ESDHC_IRQSTAT_CINT));

        // notify the SDBusDriver of the SDIO interrupt
        IndicateSlotStateChange(DeviceInterrupting);
    }

}

#ifdef ENABLE_DEBUG

// Reads from SD Standard Host registers and writes them to the debugger.
VOID CESDHCBase::DumpRegisters()
{
    DEBUGMSG(SDCARD_ZONE_INIT, (L"+DumpESDHCRegs-------------------------\r\n"));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"DSADDR 0x%08X \r\n", INREG32(&m_pESDHCReg->DSADDR)));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"BLKATTR 0x%08X \r\n", INREG32(&m_pESDHCReg->BLKATTR)));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"CMDARG 0x%08X \r\n", INREG32(&m_pESDHCReg->CMDARG)));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"XFERTYP 0x%08X \r\n", INREG32(&m_pESDHCReg->XFERTYP)));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"CMDRSP0 0x%08X \r\n", INREG32(&m_pESDHCReg->CMDRSP0)));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"CMDRSP1 0x%08X \r\n", INREG32(&m_pESDHCReg->CMDRSP1)));   
    DEBUGMSG(SDCARD_ZONE_INIT, (L"CMDRSP2 0x%08X \r\n", INREG32(&m_pESDHCReg->CMDRSP2)));   
    DEBUGMSG(SDCARD_ZONE_INIT, (L"CMDRSP3 0x%08X \r\n", INREG32(&m_pESDHCReg->CMDRSP3)));   
    DEBUGMSG(SDCARD_ZONE_INIT, (L"PRSSTAT 0x%08X \r\n", INREG32(&m_pESDHCReg->PRSSTAT)));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"PROCTL 0x%08X \r\n", INREG32(&m_pESDHCReg->PROCTL)));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"SYSCTL 0x%08X \r\n", INREG32(&m_pESDHCReg->SYSCTL)));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"IRQSTAT 0x%08X \r\n", INREG32(&m_pESDHCReg->IRQSTAT)));        
    DEBUGMSG(SDCARD_ZONE_INIT, (L"AUTOC12ERR 0x%08X \r\n", INREG32(&m_pESDHCReg->AUTOC12ERR)));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"HOSTCAPBLT 0x%08X \r\n", INREG32(&m_pESDHCReg->HOSTCAPBLT)));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"WML 0x%08X \r\n", INREG32(&m_pESDHCReg->WML)));
    DEBUGMSG(SDCARD_ZONE_INIT, (L"HOSTVER 0x%08X \r\n", INREG32(&m_pESDHCReg->HOSTVER)));        
    DEBUGMSG(SDCARD_ZONE_INIT, (L"-DumpESDHCRegs-------------------------\r\n"));
}

#endif

CESDHCBase::~CESDHCBase()
{
    if (m_SlotDma)
        delete m_SlotDma;
}

BOOL CESDHCBase::Init( LPCTSTR pszActiveKey )
{
    SD_API_STATUS      status;              // SD status
    HKEY               hKeyDevice = NULL;   // device key
    CReg               regDevice;           // encapsulated device key
    DWORD              dwRet = 0;           // return value    
    BOOL               fRegisteredWithBusDriver = FALSE;
    BOOL               fHardwareInitialized = FALSE;
    PHYSICAL_ADDRESS   pa;

    DEBUGMSG(SDCARD_ZONE_INIT, (L"+CESDHCBase::Init: "
        L"Active RegPath: %s\r\n", pszActiveKey
        ));

    m_pCurrentRequest = NULL;
    m_pszActiveKey = pszActiveKey; //save the active key for power mgt

    hKeyDevice = OpenDeviceKey(pszActiveKey);
    if ( (hKeyDevice == NULL) || !regDevice.Open(hKeyDevice, NULL) ) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::Init: "
            L"Failed to open device key\r\n"
            ));
        goto EXIT;
    }

    // allocate the context - Parameter 1: support for multiple slots - BUT 1 instance of ESDHC class per slot
    status = SDHCDAllocateContext(1, &m_pHCContext);
    if (!SD_API_SUCCESS(status)) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::Init: "
            L"Failed to allocate context : 0x%08X\r\n",
            status
            ));
        goto EXIT;
    }

    // Set our extension
    m_pHCContext->pHCSpecificContext = this;

    // At a minimum, get the controller index from platform registry
    if( !BspGetRegistrySettings(&regDevice) )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::Init: "
            L"Error reading registry settings\r\n"
            ));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto EXIT;
    }

    // map hardware memory space
    pa.QuadPart = CspESDHCGetBaseAddr(m_dwControllerIndex);
    m_pESDHCReg = (PCSP_ESDHC_REG)MmMapIoSpace( pa, sizeof(CSP_ESDHC_REG), FALSE );
    if ( !m_pESDHCReg )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::Init: "
            L"Error allocating ESDHC registers\r\n"
            ));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto EXIT;
    }

    // turn on clocks here so that registers can be written to

    // turn on all clocks to ESDHC
    BspESDHCSetClockGating(D0);
    m_PowerState = D0;


    // Initialize the slot
    SoftwareReset();

    fHardwareInitialized = TRUE;

    // Read SD Host Controller Info from register.
    if (!InterpretCapabilities()) 
    {
        goto EXIT;
    }

    // now register the host controller 
    status = SDHCDRegisterHostController(m_pHCContext);

    if (!SD_API_SUCCESS(status)) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::Init: "
            L"Failed to register host controller: %0x08X\r\n", status
            ));
        goto EXIT;
    }

    fRegisteredWithBusDriver = TRUE;

    // return the controller context
    dwRet = (DWORD) this;

EXIT:

    // Turn card power off until insertion is detected
    m_dwCurrentPowerLevel = 0;
    BspESDHCSetSlotVoltage(m_dwCurrentPowerLevel); 

    
    if (hKeyDevice) RegCloseKey(hKeyDevice);

    if ( (dwRet == 0) && m_pHCContext ) {
        FreeHostContext( fRegisteredWithBusDriver, fHardwareInitialized );
    }

    DEBUGMSG(SDCARD_ZONE_INIT, (L"-CESDHCBase::Init\r\n"));

    return dwRet;
}


// Free the host context and associated resources.
VOID CESDHCBase::FreeHostContext( BOOL fRegisteredWithBusDriver, BOOL fHardwareInitialized )
{
    DEBUGCHK(m_pHCContext);

    if (fRegisteredWithBusDriver) {
        // deregister the host controller
        SDHCDDeregisterHostController(m_pHCContext);
    }

    // unmap hardware memory space

    if (fHardwareInitialized)
        BspESDHCDeinit();   

    // turn off all clocks to ESDHC
    BspESDHCSetClockGating(D4);
    m_PowerState = D4;

    // Turn card power off
    if (m_dwCurrentPowerLevel != 0)
    {
        m_dwCurrentPowerLevel = 0;
        BspESDHCSetSlotVoltage(m_dwCurrentPowerLevel); 
    }

    if (m_pESDHCReg) MmUnmapIoSpace((PVOID)m_pESDHCReg, sizeof(CSP_ESDHC_REG));



    // cleanup the host context
    SDHCDDeleteContext(m_pHCContext);
}    


BOOL CESDHCBase::IOControl(
                           DWORD dwCode, 
                           BYTE *pInBuffer, 
                           DWORD inSize, 
                           BYTE *pOutBuffer, 
                           DWORD outSize, 
                           DWORD *pOutSize)
{
    UNREFERENCED_PARAMETER(pInBuffer);
    UNREFERENCED_PARAMETER(inSize);

    BOOL bRetVal = FALSE;

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CESDHCBase::IOControl(0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        dwCode, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
        )); 

    switch (dwCode) {
        // Power management functions.
        // Return device specific power capabilities.
    case IOCTL_POWER_CAPABILITIES:
        {
            POWER_CAPABILITIES pc;

            // Check arguments.
            if ( pOutBuffer == NULL || outSize < sizeof(POWER_CAPABILITIES))
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::IOControl: "
                    L"IOCTL_POWER_CAPABILITIES Invalid parameter.\r\n"
                    ));
                break;
            }

            // Clear capabilities structure.
            memset(&pc, 0, sizeof(POWER_CAPABILITIES));

            // Set power capabilities to 0. Clock gating is managed internally, and 
            // power gating hooks are provided by SHC_PowerUp and SHC_PowerDown APIs
            pc.DeviceDx = 0;

            DEBUGMSG(SDCARD_ZONE_POWER, (L"CESDHCBase::IOControl: "
                L"IOCTL_POWER_CAPABILITIES = 0x%x\r\n", pc.DeviceDx
                ));

            if (CeSafeCopyMemory(pOutBuffer, &pc, sizeof(pc)) == 0)
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::IOControl: "
                    L"CeSafeCopyMemory Failed\r\n"
                    ));
                break;
            }

            // Update returned data size.
            if (pOutSize)
            {
                *pOutSize = sizeof(pc);
            }
            bRetVal = TRUE;
            break;
        }

        // Indicate if the device is ready to enter a new device power state.
    case IOCTL_POWER_QUERY:
        {
            DEBUGMSG(SDCARD_ZONE_POWER, (L"CESDHCBase::IOControl: "
                L"IOCTL_POWER_QUERY Deprecated Function Called\r\n"
                ));
            bRetVal = FALSE;
            break;
        }

        // This driver self-manages it's internal power state by controlling
        // functional and interface clocks as needed in between commands
        // rather than waiting for PM to tell it to save power
        // So the set calls below do nothing
    case IOCTL_POWER_SET:
        {
            bRetVal = TRUE;
            break;
        }

        // Return the current device power state.
    case IOCTL_POWER_GET:
        {
            // Check arguments.
            if (pOutBuffer == NULL || outSize < sizeof(CEDEVICE_POWER_STATE))
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::IOControl: "
                    L"IOCTL_POWER_GET Invalid parameter.\r\n"
                    ));
                break;
            }

            //Copy current state
            if (CeSafeCopyMemory(pOutBuffer, &this->m_PowerState, sizeof(this->m_PowerState)) == 0)
            {
                DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::IOControl: "
                    L"CeSafeCopyMemory Failed\r\n"
                    ));
                break;
            }

            // Update returned data size.
            if (pOutSize)
            {
                *pOutSize = sizeof(this->m_PowerState);
            }

            DEBUGMSG(SDCARD_ZONE_POWER, (L"CESDHCBase::IOControl: "
                L"IOCTL_POWER_GET: %d\r\n", this->m_PowerState
                ));
            bRetVal = TRUE;        
            break;
        }

    case SDHC_IOCTL_LOAD_CLIENT:
        m_dwCurrentPowerLevel = SD_VDD_WINDOW_3_0_TO_3_1;
        BspESDHCSetSlotVoltage(m_dwCurrentPowerLevel);
        SetEvent(m_hControllerISTEvent);
        break;

    default:
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::IOControl: "
            L"Unknown IOCTL_xxx(0x%0.8X)\r\n", dwCode
            ));
        break;

    }

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CESDHCBase::IOControl(rc = %d)\r\n", bRetVal));
    return bRetVal;    
}

// Process the capabilities register
BOOL CESDHCBase::InterpretCapabilities()
{
    BOOL fRet = TRUE;

    // set the host controller name
    SDHCDSetHCName(m_pHCContext, L"ESDHC");

    // set init handler
    SDHCDSetControllerInitHandler(m_pHCContext, CESDHCBase::SDHCInitialize);

    // set deinit handler    
    SDHCDSetControllerDeinitHandler(m_pHCContext, CESDHCBase::SDHCDeinitialize);

    // set the Send packet handler
    SDHCDSetBusRequestHandler(m_pHCContext, CESDHCBase::SDHCBusRequestHandler);

    // set the cancel I/O handler
    SDHCDSetCancelIOHandler(m_pHCContext, CESDHCBase::SDHCCancelIoHandler);

    // set the slot option handler
    SDHCDSetSlotOptionHandler(m_pHCContext, CESDHCBase::SDHCSlotOptionHandler);

    // set maximum block length
    m_usMaxBlockLen = ESDHC_MAX_BLK_LENGTH;
    m_fHighSpeedSupport = CSP_BITFEXT(INREG32(&m_pESDHCReg->HOSTCAPBLT), ESDHC_HOSTCAPBLT_HSS);
    m_fADMASupport = CSP_BITFEXT(INREG32(&m_pESDHCReg->HOSTCAPBLT), ESDHC_HOSTCAPBLT_ADMAS);
    m_dwVendorVersion = CSP_BITFEXT(INREG32(&m_pESDHCReg->HOSTVER), ESDHC_HOSTVER_VVN);

    return fRet;
}

SD_API_STATUS CESDHCBase::Stop()
{
    // mark for shutdown
    m_fDriverShutdown = TRUE;

    if (m_fInitialized) {
        if( m_dwControllerSysIntr != SYSINTR_UNDEFINED )
        {
            // disable wakeup
            if ( m_fWakeupSource)
            {
                KernelIoControl( IOCTL_HAL_DISABLE_WAKE,
                    &m_dwControllerSysIntr,
                    sizeof( m_dwControllerSysIntr ),
                    NULL,
                    0,
                    NULL );
            }

            // disable controller interrupt
            InterruptDisable(m_dwControllerSysIntr);

            // release the SYSINTR value
            KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_dwControllerSysIntr, sizeof(DWORD), NULL, 0, NULL);
            m_dwControllerSysIntr = (DWORD) SYSINTR_UNDEFINED;
        }

        if (m_fCardPresent) {
            // remove device
            HandleRemoval(FALSE);
        }

    }

    // clean up controller IST
    if (NULL != m_htControllerIST) {
        // wake up the IST
        SetEvent(m_hControllerISTEvent);
        // wait for the thread to exit
        WaitForSingleObject(m_htControllerIST, INFINITE); 
        CloseHandle(m_htControllerIST);
        m_htControllerIST = NULL;
    }

    // free controller interrupt event
    if (NULL != m_hControllerISTEvent) {
        CloseHandle(m_hControllerISTEvent);
        m_hControllerISTEvent = NULL;
    }

    if (m_SlotDma) {
        delete m_SlotDma;
        m_SlotDma = NULL;
    }

    // Turn card power off
    if (m_dwCurrentPowerLevel != 0)
    {
        m_dwCurrentPowerLevel = 0;
        BspESDHCSetSlotVoltage(m_dwCurrentPowerLevel); 
    }


    return SD_API_STATUS_SUCCESS;
}

SD_API_STATUS CESDHCBase::Start()
{
    SD_API_STATUS status = SD_API_STATUS_INSUFFICIENT_RESOURCES; // intermediate status
    DWORD dwProctlReg = 0;

    m_fDriverShutdown = FALSE;

    // --- Setup Protocol Control register with initial values ---
    // Data Bus width 1 bit - Little Endian - use LED for status (if available)
    dwProctlReg |= CSP_BITFVAL(ESDHC_PROCTL_EMODE, ESDHC_EMODE_LE);
    dwProctlReg |= CSP_BITFVAL(ESDHC_PROCTL_DTW, ESDHC_DTW_1BIT);
    dwProctlReg |= CSP_BITFVAL(ESDHC_PROCTL_LCTL, 1);

    // Enable wake up for card insertion/removal and SDIO interrupts
    dwProctlReg |= CSP_BITFVAL(ESDHC_PROCTL_WECINS, 1);
    dwProctlReg |= CSP_BITFVAL(ESDHC_PROCTL_WECRM, 1);
    dwProctlReg |= CSP_BITFVAL(ESDHC_PROCTL_WECINT, 1);

    OUTREG32(&m_pESDHCReg->PROCTL, dwProctlReg);
    DEBUGMSG(SDCARD_ZONE_INIT, (L"CESDHCBase::Start: "
        L"PROCTL set value = %X\r\n", dwProctlReg));

    if (!m_fDisableDMA) {

        // use internal DMA
        if (!m_fUseExternalDMA)
        {
            if (m_fADMASupport) {

                if (m_dwVendorVersion >= ESDHC_VVN_FSL22)
                    m_SlotDma = new CESDHCBase32BitADMA2(*this);
                else
                    m_SlotDma = new CESDHCBase32BitADMA(*this);
            }
            else {  // use Simple DMA
                m_SlotDma = new CESDHCBaseSDMA(*this);
            }
        } 

        // use external DMA engine
        else {
            m_SlotDma = new CESDHCBaseEDMA(*this);
        }

        if (m_SlotDma && !m_SlotDma->Init()) { // failed.
            delete m_SlotDma;
            m_SlotDma = NULL;
        }
    }


    // setup the IOMux
    if( !BspESDHCInit() )
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::Init: "
            L"Error with IOMux setup\r\n"
            ));
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }


    // allow BSP code to map more than 1 IRQ to the SysIntr
    if (!BspESDHCSysIntrSetup())
    {
        // KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR) call must have failed
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::SDHCInitialize: "
            L"Error obtaining ESDHC SYSINTR value!\r\n"
            ));
        m_dwControllerSysIntr = (DWORD) SYSINTR_UNDEFINED;
        status = SD_API_STATUS_UNSUCCESSFUL;
        goto exitInit;
    }

    // allocate the interrupt event for the SDIO/controller interrupt
    m_hControllerISTEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if (NULL == m_hControllerISTEvent) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    if ( !InterruptInitialize( m_dwControllerSysIntr, m_hControllerISTEvent, NULL, 0 ) ) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    // create the Controller IST thread
    m_htControllerIST = CreateThread(NULL,
        0,
        ISTStub,
        this,
        0,
        NULL);

    if (NULL == m_htControllerIST) {
        status = SD_API_STATUS_INSUFFICIENT_RESOURCES;
        goto exitInit;
    }

    // Enable command (& DMA) error interrupt signals and all error interrupt status. During execution, we will enable 
    // and disable other interrupts as needed.
    INSREG32BF(&m_pESDHCReg->IRQSIGEN, ESDHC_IRQSIGEN_DMAEIEN, 1); 
    OUTREG32(&m_pESDHCReg->IRQSTATEN, ESDHC_ERROR_BITS | CSP_BITFMASK(ESDHC_IRQSTATEN_CINTSEN));


    // Enable Card Insert interrupt upon initialization
    //SETREG32(&m_pESDHCReg->IRQSTATEN, CSP_BITFMASK(ESDHC_IRQSTATEN_CINSSEN));
    //SETREG32(&m_pESDHCReg->IRQSIGEN, CSP_BITFMASK(ESDHC_IRQSIGEN_CINSIEN)); 
    BspESDHCCardDetectInt(TRUE);


    m_fInitialized = TRUE;

    status = SD_API_STATUS_SUCCESS;

exitInit:

    if (!SD_API_SUCCESS(status)) {
        // just call the deinit handler directly to cleanup
        Stop();
    }

    // gate clocks again until card is inserted
    BspESDHCSetClockGating(D4);
    m_PowerState = D4;

    return status;

}

///////////////////////////////////////////////////////////////////////////////
//  CESDHCBase::SlotOptionHandler - handler for slot option changes
//  Input: Option
//           pData
//           OptionSize
//  Output: 
//  Return: SD_API_STATUS
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CESDHCBase::SlotOptionHandler( 
    SD_SLOT_OPTION_CODE Option, 
    PVOID pData,
    ULONG OptionSize )
{
    SD_API_STATUS status = SD_API_STATUS_SUCCESS;   // status

    SDHCDAcquireHCLock(m_pHCContext);

    switch (Option) {

        case SDHCDSetSlotPower:
            {
                DEBUGMSG(SDCARD_ZONE_INFO, (L"CESDHCBase::SDHCSlotOptionHandlerImpl: "
                    L"Called - SDHCDSetSlotPower\r\n"));

                m_dwCurrentPowerLevel = *(PDWORD) pData;
                BspESDHCSetSlotVoltage(m_dwCurrentPowerLevel);
            }
            break;

        case SDHCDSetSlotInterface:
            {
                PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;

                DEBUGMSG(SDCARD_ZONE_INFO, (L"CESDHCBase::SDHCSlotOptionHandlerImpl: "
                    L"Called - SetSlotInterface : Clock Setting: %d\r\n", pInterface->ClockRate
                    ));
                SetInterface(pInterface);
            }
            break;

        case SDHCDEnableSDIOInterrupts:
        case SDHCDAckSDIOInterrupt:            

            EnableSDIOInterrupts();
            break;

        case SDHCDDisableSDIOInterrupts:

            DisableSDIOInterrupts();
            break;            

        case SDHCDSetSlotPowerState:

            if( pData == NULL || OptionSize != sizeof(CEDEVICE_POWER_STATE) )
            {
                status = SD_API_STATUS_INVALID_PARAMETER;
            }
            else
            {

                // Request a change from one device power state to another
                // This driver self-manages it's internal power state by controlling
                // functional and interface clocks as needed in between commands
                // rather than waiting for Bus Driver to tell it to save power

                //PCEDEVICE_POWER_STATE pcps = (PCEDEVICE_POWER_STATE) pData;
                //this->m_PowerState = *pcps;
            }
            break;

        case SDHCDGetSlotPowerState:

            if( pData == NULL || OptionSize != sizeof(CEDEVICE_POWER_STATE) )
            {
                status = SD_API_STATUS_INVALID_PARAMETER;
            }
            else
            {
                PCEDEVICE_POWER_STATE pcps = (PCEDEVICE_POWER_STATE) pData;
                *pcps = this->m_PowerState;
            }
            break;

        case SDHCDGetWriteProtectStatus:
            {
                PSD_CARD_INTERFACE pInterface = (PSD_CARD_INTERFACE) pData;
                pInterface->WriteProtected = BspESDHCIsWriteProtected();
            }

            break;

        case SDHCDQueryBlockCapability:
            {
                PSD_HOST_BLOCK_CAPABILITY pBlockCaps = 
                    (PSD_HOST_BLOCK_CAPABILITY)pData;

                DEBUGMSG(SDCARD_ZONE_INFO, (L"CESDHCBase::SDHCSlotOptionHandler: "
                    L"Read Block Length: %d , Read Blocks: %d\r\n", pBlockCaps->ReadBlockSize, pBlockCaps->ReadBlocks
                    ));
                DEBUGMSG(SDCARD_ZONE_INFO, (L"CESDHCBase::SDHCSlotOptionHandler: "
                    L"Write Block Length: %d , Write Blocks: %d\r\n", pBlockCaps->WriteBlockSize, pBlockCaps->WriteBlocks
                    ));

                if (pBlockCaps->ReadBlockSize < ESDHC_MIN_BLK_LENGTH) {
                    pBlockCaps->ReadBlockSize = ESDHC_MIN_BLK_LENGTH;
                }

                if (pBlockCaps->ReadBlockSize > m_usMaxBlockLen) {
                    pBlockCaps->ReadBlockSize = m_usMaxBlockLen;
                }

                if (pBlockCaps->WriteBlockSize < ESDHC_MIN_BLK_LENGTH) {
                    pBlockCaps->WriteBlockSize = ESDHC_MIN_BLK_LENGTH;
                }

                if (pBlockCaps->WriteBlockSize > m_usMaxBlockLen) {
                    pBlockCaps->WriteBlockSize = m_usMaxBlockLen;
                }
            }
            break;

        case SDHCDGetSlotInfo:
            if( OptionSize != sizeof(SDCARD_HC_SLOT_INFO) || pData == NULL )
            {
                status = SD_API_STATUS_INVALID_PARAMETER;
            }
            else
            {
                PSDCARD_HC_SLOT_INFO pSlotInfo = (PSDCARD_HC_SLOT_INFO)pData;

                DWORD dwCaps = SD_SLOT_SD_1BIT_CAPABLE |
                    SD_SLOT_SD_4BIT_CAPABLE |
                    SD_SLOT_SDIO_CAPABLE      |
                    SD_SLOT_SDIO_INT_DETECT_4BIT_MULTI_BLOCK |
                    (m_fHighSpeedSupport? SD_SLOT_HIGH_SPEED_CAPABLE:0);

                // set the slot capabilities (ESDHC can detect interrupt in 4-bit mode during Block Gap)
                SDHCDSetSlotCapabilities( pSlotInfo, dwCaps);

                DEBUGMSG(SDCARD_ZONE_INIT, (L"SlotpOptionHandler: Caps = 0x%x, HSS = 0x%x\r\n", dwCaps, m_fHighSpeedSupport));

                SDHCDSetVoltageWindowMask(pSlotInfo, (SD_VDD_WINDOW_2_9_TO_3_0 | SD_VDD_WINDOW_3_0_TO_3_1 | SD_VDD_WINDOW_3_1_TO_3_2)); 


                // Set optimal voltage
                SDHCDSetDesiredSlotVoltage(pSlotInfo, SD_VDD_WINDOW_3_0_TO_3_1);     

                // Set maximum supported clock rate
                SDHCDSetMaxClockRate(pSlotInfo, m_dwMaxClockRate);

                // set power up delay
                SDHCDSetPowerUpDelay(pSlotInfo, ESDHC_MAX_POWER_SUPPLY_RAMP_UP); 
            }
            break;

        default:
            status = SD_API_STATUS_INVALID_PARAMETER;

    }

    SDHCDReleaseHCLock(m_pHCContext);
    return status;
}


///////////////////////////////////////////////////////////////////////////////
//  ControllerIstThread - SDIO/controller IST thread for driver
//  Input:  lpParameter - pController - controller instance
//  Output: 
//  Return: Thread exit status
//  Notes:
///////////////////////////////////////////////////////////////////////////////
DWORD CESDHCBase::ControllerIstThread()
{
    DWORD dwWaitTime    = INFINITE;

    if (!CeSetThreadPriority(GetCurrentThread(), m_dwSDIOPriority)) {
        DEBUGMSG(SDCARD_ZONE_WARN, (L"CESDHCBase::ControllerIstThread: "
            L"Warning, failed to set CEThreadPriority\r\n"
            ));
    }

    for(;;) {
        // wait for the SDIO/controller interrupt
        if (WAIT_OBJECT_0 != WaitForSingleObject(m_hControllerISTEvent, dwWaitTime)) 
        {
            DEBUGMSG(SDCARD_ZONE_WARN, (L"CESDHCBase::ControllerIstThread: "
                L"Wait Failed!\r\n"
                ));
            break;
        }

        if (m_fDriverShutdown) {
            DEBUGMSG(SDCARD_ZONE_WARN, (L"CESDHCBase::ControllerIstThread: "
                L"Thread exiting!\r\n"
                ));
            break;
        }

        SDHCDAcquireHCLock(m_pHCContext);

        HandleInterrupts();
        InterruptDone( m_dwControllerSysIntr );
        SDHCDReleaseHCLock(m_pHCContext);
    }

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CESDHCBase::ControllerIstThread\r\n"));
    return 0;

}


///////////////////////////////////////////////////////////////////////////////
//  CESDHCBase::BusRequestHandler - bus request handler 
//  Input:  pRequest - the request
//          
//  Output: 
//  Return: SD_API_STATUS
//  Notes:  The request passed in is marked as uncancelable, this function
//          has the option of making the outstanding request cancelable    
//          returns status pending
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CESDHCBase::BusRequestHandler( PSD_BUS_REQUEST pRequest ) 
{
    DEBUGCHK(pRequest);
    DEBUGCHK(!m_fCommandCompleteOccurred);

    SD_API_STATUS   status;

    DEBUGMSG(SHC_SEND_ZONE, (L"CESDHCBase::SDHCBusRequestHandler: "
        L"CMD: [%d]\r\n", pRequest->CommandCode
        ));

    // acquire the device lock to protect from device removal
    SDHCDAcquireHCLock(m_pHCContext);

    if (!m_fCardPresent)
    {
        status = SD_API_STATUS_DEVICE_REMOVED;
        goto EXIT;
    }    
    if ( m_pCurrentRequest) {
        IndicateBusRequestComplete(pRequest, SD_API_STATUS_CANCELED);
        m_pCurrentRequest = NULL;
    }

    m_fCurrentRequestFastPath = FALSE;
    m_pCurrentRequest = pRequest ;

    // if short transfer, use FAST PATH
    if (pRequest->SystemFlags & SD_FAST_PATH_AVAILABLE ){   // We do fast path here.
        m_fCurrentRequestFastPath = TRUE;

        if (m_dwVendorVersion < ESDHC_VVN_FSL22)
            InterruptMask(m_dwControllerSysIntr,TRUE);

        DWORD dwIrqSigen = INREG32(&m_pESDHCReg->IRQSIGEN);
        OUTREG32(&m_pESDHCReg->IRQSIGEN, 0);

        status = SendCommand(pRequest);
        if ( status == SD_API_STATUS_PENDING ) { // Polling for completion.
            while (m_pCurrentRequest) {
                HandleInterrupts();
            }               
            status = m_FastPathStatus;
        }

        if (m_dwVendorVersion < ESDHC_VVN_FSL22)
            InterruptMask(m_dwControllerSysIntr,FALSE);
        
        OUTREG32(&m_pESDHCReg->IRQSIGEN, dwIrqSigen);
        m_fCurrentRequestFastPath = FALSE;
    }

    else 
    {
        status = SendCommand(pRequest);
        if(!SD_API_SUCCESS(status))
        {
            DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::SDHCBusRequestHandler: "
                L"Error sending command:0x%02x\r\n", pRequest->CommandCode
                ));
            goto EXIT;      
        }
    }


EXIT:

    if (status != SD_API_STATUS_PENDING && m_pCurrentRequest) { 
        // if there is error case. We don't notify the callback function either So.
        m_fCurrentRequestFastPath = TRUE;
        IndicateBusRequestComplete(pRequest,status);
    }


    SDHCDReleaseHCLock(m_pHCContext);
    return status;
}

///////////////////////////////////////////////////////////////////////////////
//  CESDHCBase::CancelIoHandler - io cancel handler 
//  Input:  Slot - slot the request is going on
//          pRequest - the request to be cancelled
//          
//  Output: 
//  Return: TRUE if I/O was cancelled
//  Notes:  
//          the HC lock is taken before entering this cancel handler
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN CESDHCBase::CancelIoHandler( PSD_BUS_REQUEST pRequest)
{
    // for now, we should never get here because all requests are non-cancelable
    // the hardware supports timeouts so it is impossible for the controller to get stuck

    UNREFERENCED_PARAMETER(pRequest);

    return TRUE;
}


//  CommandCompleteHandler
//  Input:
//  Output: 
//  Notes:  
BOOL CESDHCBase::CommandCompleteHandler()
{
    PSD_BUS_REQUEST     pRequest = NULL;       // the request to complete
    BOOL fRet = FALSE;

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"CESDHCBase::CommandCompleteHandler\r\n"));

    // get and lock the current bus request
    if((pRequest = GetAndLockCurrentRequest()) == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::CommandCompleteHandler: "
            L"Unable to get/lock current request!\r\n"));
        goto EXIT;
    } 

    // poll on DAT[0] for busy signalling
    if (CSP_BITFEXT(INREG32(&m_pESDHCReg->XFERTYP), ESDHC_XFERTYP_RSPTYP) == ESDHC_RSPLEN_48B)
    {
        while(! (INREG32(&m_pESDHCReg->PRSSTAT) & CSP_BITFVAL(ESDHC_PRSSTAT_DLSL, 1)));
    }

    // skip the CRC
    UNALIGNED PDWORD respBuff = (PDWORD)(pRequest->CommandResponse.ResponseBuffer + 1);
    DWORD dwRespTyp = pRequest->CommandResponse.ResponseType;

    if (dwRespTyp != NoResponse)
    {
        respBuff[0] = INREG32(&m_pESDHCReg->CMDRSP0);

        if (dwRespTyp == ResponseR2)
        {
            respBuff[1] = INREG32(&m_pESDHCReg->CMDRSP1);
            respBuff[2] = INREG32(&m_pESDHCReg->CMDRSP2);
            respBuff[3] = INREG32(&m_pESDHCReg->CMDRSP3);   
        }

        DEBUGMSG(SHC_RESPONSE_ZONE, (L"CESDHCBase::GetCommandResponse: "
            L"Returned [%08x %08x %08x %08x]\r\n", respBuff[0], respBuff[1], respBuff[2], respBuff[3]));
    }

EXIT:

    if( pRequest != NULL )
    {
        // if command-only transfer, then we are done with this request
        if (pRequest->TransferClass == SD_COMMAND)
        {
            IndicateBusRequestComplete(pRequest, SD_API_STATUS_SUCCESS);
        }

        else
        {
            // data transfer will start. keep track of bytes transferred in this parameter (initially 0)
            pRequest->HCParam = 0;

            if (m_fUseExternalDMA && m_fUseDMA)
            {
                // allow external DMA to start the channel after cmd complete, if needed
                m_SlotDma->DMANotifyEvent(*pRequest, CMD_COMPLETE);

            }
        }

        fRet = TRUE;
    }

    return fRet;
}

// give derived class an opportunity to intercept call
VOID CESDHCBase::IndicateBusRequestComplete( 
    PSD_BUS_REQUEST     pRequest,
    SD_API_STATUS       Status)
{
    DEBUGMSG (SDCARD_ZONE_FUNC, (L"CESDHCBase::IndicateBusRequestComplete: pRequest = %x, Status = %d\r\n", pRequest,Status)); 
    DEBUGCHK(pRequest);

    // disable the following interrupts that may have been enable during SendCommand
    const DWORD c_dwTransferIntSources = 
        CSP_BITFMASK(ESDHC_IRQSTATEN_CCSEN) |
        CSP_BITFMASK(ESDHC_IRQSTATEN_TCSEN) |
        CSP_BITFMASK(ESDHC_IRQSTATEN_BWRSEN) |
        CSP_BITFMASK(ESDHC_IRQSTATEN_BRRSEN)  |
        CSP_BITFMASK(ESDHC_IRQSTATEN_DINTSEN);

    // clear those interrupt enable bits
    CLRREG32(&m_pESDHCReg->IRQSTATEN, c_dwTransferIntSources);
    CLRREG32(&m_pESDHCReg->IRQSIGEN, c_dwTransferIntSources);

    // Clear any of those remaining (spurious) interrupts.
    OUTREG32(&m_pESDHCReg->IRQSTAT, c_dwTransferIntSources);

    m_fCommandCompleteOccurred = FALSE;
    m_fUseDMA = FALSE;

    m_pCurrentRequest = NULL;

    if (m_fCurrentRequestFastPath ) {
        if (Status == SD_API_STATUS_SUCCESS) {
            Status = SD_API_STATUS_FAST_PATH_SUCCESS;
        }
        m_FastPathStatus = Status ;
    }

    // let SDBusDriver know of request completion (only for a non-fast path request)
    else
    {
        DEBUGMSG (SHC_SDBUS_INTERACT_ZONE, (L"CESDHCBase::IndicateBusRequestComplete: Non-fast path. Sending Stat = -0x%x. IRQSTAT = 0x%x\r\n", Status, INREG32(&m_pESDHCReg->IRQSTAT))); 
        SDHCDIndicateBusRequestComplete(m_pHCContext, pRequest, Status);
    }

    // if successful completion of CMD5, then the card is SDIO
    if (pRequest->CommandCode == SD_CMD_IO_OP_COND && SD_API_SUCCESS(Status))
        m_fCardIsSDIO = TRUE;

    BspESDHCSetClockGating(D4);
    m_PowerState = D4;

}

PVOID CESDHCBase::SlotAllocDMABuffer(ULONG Length,PPHYSICAL_ADDRESS  LogicalAddress,BOOLEAN CacheEnabled )
{
    CE_DMA_ADAPTER  dmaAdapter = {
        sizeof(CE_DMA_ADAPTER),
        ProcessorInternal,
        m_dwControllerIndex,
        0,0
    };
    dmaAdapter.BusMaster = TRUE;
    return OALDMAAllocBuffer(&dmaAdapter, Length, LogicalAddress,CacheEnabled);
}
BOOL CESDHCBase::SlotFreeDMABuffer(ULONG Length,PHYSICAL_ADDRESS  LogicalAddress,PVOID VirtualAddress,BOOLEAN CacheEnabled )
{
    CE_DMA_ADAPTER  dmaAdapter = {
        sizeof(CE_DMA_ADAPTER),
        ProcessorInternal,
        m_dwControllerIndex,
        0,0
    };
    dmaAdapter.BusMaster = TRUE;
    OALDMAFreeBuffer(&dmaAdapter,Length, LogicalAddress, VirtualAddress, CacheEnabled);
    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//  SDHCInitialize - Initialize the the controller
//  Input:  pHCContext -  host controller context
//          
//  Output: 
//  Return: SD_API_STATUS
//  Notes:  
//          
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CESDHCBase::SDHCInitialize(PSDCARD_HC_CONTEXT pHCContext)
{
    DEBUGMSG(SDCARD_ZONE_INIT,(TEXT("SDHCInitialize++\n")));

    PREFAST_DEBUGCHK(pHCContext);
    PCESDHCBase pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    PREFAST_DEBUGCHK(pController);
    SD_API_STATUS status = pController->Start();

    DEBUGMSG(SDCARD_ZONE_INIT,(TEXT("SDHCInitialize--\n")));

    return status;
}


///////////////////////////////////////////////////////////////////////////////
//  SDHCDeinitialize - Deinitialize the SDHC Controller
//  Input:  pHCContext - HC context
//          
//  Output: 
//  Return: SD_API_STATUS
//  Notes:  
//          
//
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CESDHCBase::SDHCDeinitialize(PSDCARD_HC_CONTEXT pHCContext)
{
    DEBUGMSG(SDCARD_ZONE_INIT,(TEXT("SDHCDeinitialize++\n")));

    PREFAST_DEBUGCHK(pHCContext);
    PCESDHCBase pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    PREFAST_DEBUGCHK(pController);
    SD_API_STATUS status = pController->Stop();

    DEBUGMSG(SDCARD_ZONE_INIT,(TEXT("SDHCDeinitialize--\n")));

    return status;
}


///////////////////////////////////////////////////////////////////////////////
//  SDHCSDCancelIoHandler - io cancel handler 
//  Input:  pHostContext - host controller context
//          dwSlot - slot the request is going on
//          pRequest - the request to be cancelled
//          
//  Output: 
//  Return: TRUE if I/O was cancelled
//  Notes:  
//          the HC lock is taken before entering this cancel handler
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN CESDHCBase::SDHCCancelIoHandler(
                                        PSDCARD_HC_CONTEXT  pHCContext,
                                        DWORD               dwSlot,
                                        PSD_BUS_REQUEST     pRequest
                                        )
{
    UNREFERENCED_PARAMETER(dwSlot);
    PREFAST_DEBUGCHK(pHCContext);
    PCESDHCBase pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    PREFAST_DEBUGCHK(pController);
    return pController->CancelIoHandler(pRequest);
}


///////////////////////////////////////////////////////////////////////////////
//  SDHCBusRequestHandler - bus request handler 
//  Input:  pHostContext - host controller context
//          dwSlot - slot the request is going to
//          pRequest - the request
//          
//  Output: 
//  Return: SD_API_STATUS
//  Notes:  The request passed in is marked as uncancelable, this function
//          has the option of making the outstanding request cancelable    
//          returns status pending
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CESDHCBase::SDHCBusRequestHandler(
    PSDCARD_HC_CONTEXT pHCContext, 
    DWORD              dwSlot, 
    PSD_BUS_REQUEST    pRequest
    ) 
{
    UNREFERENCED_PARAMETER(dwSlot);
    PREFAST_DEBUGCHK(pHCContext);
    PCESDHCBase pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    PREFAST_DEBUGCHK(pController);
    return pController->BusRequestHandler(pRequest);
}

///////////////////////////////////////////////////////////////////////////////
//  SDHCSlotOptionHandler - handler for slot option changes
//  Input:  pHostContext - host controller context
//          dwSlot       - the slot the change is being applied to
//          Option       - the option code
//          pData        - data associaSHC with the option
//  Output: 
//  Return: SD_API_STATUS
//  Notes:  
///////////////////////////////////////////////////////////////////////////////
SD_API_STATUS CESDHCBase::SDHCSlotOptionHandler(
    PSDCARD_HC_CONTEXT    pHCContext,
    DWORD                 dwSlot, 
    SD_SLOT_OPTION_CODE   sdOption, 
    PVOID                 pData,
    ULONG                 ulOptionSize
    )
{
    UNREFERENCED_PARAMETER(dwSlot);
    PCESDHCBase pController = GET_PCONTROLLER_FROM_HCD(pHCContext);
    PREFAST_DEBUGCHK(pController);
    return pController->SlotOptionHandler(sdOption, pData, ulOptionSize);
}


// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE

