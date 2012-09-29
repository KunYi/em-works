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
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#include "bsp.h"
#include "sdfmd.h"


//-----------------------------------------------------------------------------
// External Functions
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);

//-----------------------------------------------------------------------------
// External Variables
extern PCSP_IOMUX_REGS g_pIOMUX;
//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Variables
PCSP_ESDHC_REG          g_pESDHCReg;
PCSP_GPIO_REGS          g_pGPIO2;
DWORD                   dwVVN;
BOOL                    bLastCmdRead = FALSE;

//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
//  Function: SDConfigPins
//
//  This function configures the pins and enables the SD interface.
//
//  Parameters:
//        None.
//
//  Returns:
//        None.
//
//-----------------------------------------------------------------------------
void SDConfigPins (void)
{
    g_pESDHCReg = (PCSP_ESDHC_REG) OALPAtoUA(CSP_BASE_REG_PA_ESDHC1);
    g_pGPIO2 = (PCSP_GPIO_REGS)OALPAtoUA(CSP_BASE_REG_PA_GPIO2);

    OALMSG(1, (TEXT("SDConfigPins:  \r\n")));
    // - GPIO2[1]
    // - Function ALT 5 on A15 pio
    // - PIO already pull-up
    // - PIO on VDD_SD1_IO (1v8)
    OAL_IOMUX_SET_MUX(g_pIOMUX, DDK_IOMUX_PIN_A15, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(g_pIOMUX, DDK_IOMUX_PAD_A15, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                     DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_1V8);

    // Set GPIO2_1 Direction as Input
    OUTREG32 (&g_pGPIO2->GDIR, (INREG32(&g_pGPIO2->GDIR) & ~(0x1<<1)));

    // SD Card Write protection (A14)
    // - GPIO2[0]
    // - Function ALT 5 on A14 pio
    // - PIO already pull-up
    // - PIO on VDD_SD1_IO (3v3) (PGAL: We can find a remarks on the schematics)
    OAL_IOMUX_SET_MUX(g_pIOMUX, DDK_IOMUX_PIN_A14, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(g_pIOMUX, DDK_IOMUX_PAD_A14, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                     DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_1V8);

    // Set GPIO2_0 Direction as Input
    OUTREG32 (&g_pGPIO2->GDIR, (INREG32(&g_pGPIO2->GDIR) & ~(0x1<<0)));


    // SD1_CMD setup
    OAL_IOMUX_SET_MUX(g_pIOMUX, DDK_IOMUX_PIN_SD1_CMD, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_FORCE);
    OAL_IOMUX_SET_PAD(g_pIOMUX, DDK_IOMUX_PAD_SD1_CMD, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                                    DDK_IOMUX_PAD_PULL_UP_22K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
    
    // SD1_CLK setup
    OAL_IOMUX_SET_MUX(g_pIOMUX, DDK_IOMUX_PIN_SD1_CLK, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(g_pIOMUX, DDK_IOMUX_PAD_SD1_CLK, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                                    DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    // SD1_DATA0 setup
    OAL_IOMUX_SET_MUX(g_pIOMUX, DDK_IOMUX_PIN_SD1_DATA0, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(g_pIOMUX, DDK_IOMUX_PAD_SD1_DATA0, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                                    DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    // SD1_DATA1 setup
    OAL_IOMUX_SET_MUX(g_pIOMUX, DDK_IOMUX_PIN_SD1_DATA1, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(g_pIOMUX, DDK_IOMUX_PAD_SD1_DATA1, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                                    DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    // SD1_DATA2 setup
    OAL_IOMUX_SET_MUX(g_pIOMUX, DDK_IOMUX_PIN_SD1_DATA2, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(g_pIOMUX, DDK_IOMUX_PAD_SD1_DATA2, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                                    DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    // SD1_DATA3 setup
    OAL_IOMUX_SET_MUX(g_pIOMUX, DDK_IOMUX_PIN_SD1_DATA3, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(g_pIOMUX, DDK_IOMUX_PAD_SD1_DATA3, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                                    DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
}

//-----------------------------------------------------------------------------
//
//  Function: SDInterface_Init
//
//  This function configures and resets the SD controller.
//
//  Parameters:
//        None.
//
//  Returns:
//        TRUE for success/FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL SDInterface_Init (void)
{
    SDController.IsMMC = FALSE;
    SDController.BusWidthSetting = ESDHC_DTW_1BIT;
    SDController.Words_in_fifo  = ESDHC_MAX_DATA_BUFFER_SIZE >> 2;    
    SDController.Bytes_in_fifo  = ESDHC_MAX_DATA_BUFFER_SIZE; 
    SDController.SendInitClocks = TRUE;

    if (!SDHC_IsCardPresent ())
    {
        return FALSE; 
    }

    //turn on ipg_clock_esdhc1, 
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_ESDHC1, DDK_CLOCK_GATE_MODE_ENABLED); 
    //turn on ipg_per_esdhc1
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_ESDHC1, DDK_CLOCK_GATE_MODE_ENABLED); 
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_ESDHC1, DDK_CLOCK_GATE_MODE_ENABLED);

     // Reset the controller
    INSREG32BF(&g_pESDHCReg->SYSCTL, ESDHC_SYSCTL_RSTA, 1);

    // wait for reset to complete
    while (INREG32(&g_pESDHCReg->SYSCTL) & CSP_BITFMASK(ESDHC_SYSCTL_RSTA));

    // Mask all SDHC interrupts
    OUTREG32(&g_pESDHCReg->IRQSIGEN, 0);

    // Enable all status bits in the IRQSTAT register except CINS and CRM because we do not process changes in card status in eboot
    OUTREG32(&g_pESDHCReg->IRQSTATEN, 0xFFFFFF3F);

    SDController.dwClockRate = ESDHC_INIT_CLOCK_RATE;
    SetClockRate(&(SDController.dwClockRate)); // Identification Frequency

    dwVVN = CSP_BITFEXT(INREG32(&g_pESDHCReg->HOSTVER), ESDHC_HOSTVER_VVN);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: SDHC_IsCardPresent
//
//  This function detects the presence of SD/MMC card.
//
//  Parameters:
//        None.
//
//  Returns:
//        TRUE for success/FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL SDHC_IsCardPresent (void)
{
    BOOL bCardPresent = FALSE;

    // card is present if GPIO2_0 pad reads back as 0
    if (!(INREG32(&g_pGPIO2->DR) & (0x1<<1)) )
        bCardPresent = TRUE;
    
    return bCardPresent;
}

//-----------------------------------------------------------------------------
//
//  Function: SDHC_IsCardWriteProtected
//
//  This function detects whether WP switch on card is on or off
//
//  Parameters:
//        None.
//
//  Returns:
//        TRUE for write protect set, FALSE for not set.
//
//-----------------------------------------------------------------------------
BOOL SDHC_IsCardWriteProtected(void)
{
    if (INREG32(&g_pGPIO2->DR) & 0x00000001 )
        return TRUE;
    else
        return FALSE;

}

//------------------------------------------------------------------------------
//
// Function: SetRate
//
// Sets the desired SD/MMC clock frequency. Note: The closest frequecy
//            to the desired setting is chosen.
//
// Parameters:
//          dwRate[in] - desired clock rate in Hz
//
// Returns:
//        None
//
//------------------------------------------------------------------------------
void SetClockRate(PDWORD pdwRate)
{
    BSP_ARGS *pBspArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;
    ULONG ulMaxSDClk = min(ESDHC_MAX_CLOCK_RATE, pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_ESDHC1]);
    ULONG ulMinSDClk = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_ESDHC1] >> 12;  // BaseClk / 4096 is the minimum SD clock
    const ULONG ulMaxDivisor = 16;
    const ULONG ulMaxPrescaler = 256;
    ULONG ulDiv =1, ulPrescaler = 1, ulErr = 0xFFFFFFFF;
    ULONG ulCurrRate = 0, ulCurrErr = 0, i = 0, j = 0;
    
    DWORD rate = *pdwRate;

    if( rate > ulMaxSDClk)
    {
        rate = ulMaxSDClk;
    }
    else if (rate < ulMinSDClk)
    {
        rate = ulMinSDClk;
    }

    // find the best values for Divisor and Prescalar 
    for (i = 1; i <= ulMaxDivisor; i++)
    {
        for (j = 1; j < ulMaxPrescaler; j <<= 1)
        {
            ulCurrRate = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_ESDHC1] / (i * j);

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
    rate = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_ESDHC1] / (ulDiv * ulPrescaler);

    OALMSG(OAL_FUNC, (TEXT("SetClockRate - Requested Rate: %d Hz, Setting clock rate to %d Hz \n"), *pdwRate, rate ));

    // Map the divisors to their respective register bit values
    ulDiv--;
    ulPrescaler >>= 1;

    // clear SDCLKEN bit first before changing frequency
    CLRREG32(&g_pESDHCReg->SYSCTL, CSP_BITFMASK(ESDHC_SYSCTL_SDCLKEN));
    
    // set the clock rate
    INSREG32BF(&g_pESDHCReg->SYSCTL, ESDHC_SYSCTL_DVS, ulDiv);
    INSREG32BF(&g_pESDHCReg->SYSCTL, ESDHC_SYSCTL_SDCLKFS, ulPrescaler);

    // set the data timeout value to be maximum
    INSREG32BF(&g_pESDHCReg->SYSCTL, ESDHC_SYSCTL_DTOCV, ESDHC_MAX_DATA_TIMEOUT_COUNTER_VAL);

    // wait until new frequency is stabilized (SDSTB bit is only avaliable on VVN = 0x12 or later)
    if ( CSP_BITFEXT(INREG32(&g_pESDHCReg->HOSTVER), ESDHC_HOSTVER_VVN) >= ESDHC_VVN_FSL22 )
        while(! CSP_BITFEXT(INREG32(&g_pESDHCReg->PRSSTAT), ESDHC_PRSSTAT_SDSTB));

    // Enable clocks (including SDCLKEN)
    INSREG32(&g_pESDHCReg->SYSCTL, 0xF, 0xF);
    
    // return the actual frequency
    *pdwRate = rate;

    OALMSG(OAL_FUNC, (_T("- SetClockRate\r\n")));
}

//------------------------------------------------------------------------------
//
// Function: SDHCCmdConfig
//
// Configure ESDHC registers for sending a command to MMC/SD.
//
// Parameters:
//          pReq[in] - structure containing necesary fields to issue a command
//
// Returns:
//        None
//
//------------------------------------------------------------------------------
static void SDHCCmdConfig(PSD_BUS_REQUEST pReq)                                                           
{
    DWORD dwXferTypReg = 0;

    UCHAR ucCmd = pReq->CommandCode;
    UINT32 Arg = pReq->CommandArgument;
    UINT16 respType = pReq->CommandResponse.ResponseType;
    UINT16 TransferClass = pReq->TransferClass;
    DWORD dwPresStatReg = INREG32(&g_pESDHCReg->PRSSTAT);

    // Can't write to XFERTYP register while CIHB is set
    if (dwPresStatReg & CSP_BITFMASK(ESDHC_PRSSTAT_CIHB))
    {
        OALMSG(1, (L"SDHCCmdConfig: CIHB set, can't Send Cmd!!\r\n"));
        // reset the command portion of ESDHC        
        SETREG32(&g_pESDHCReg->SYSCTL, CSP_BITFMASK(ESDHC_SYSCTL_RSTC));
    }
    
    // For commands with data, can't write to XFERTYP while CDIHB is set
    if (TransferClass != SD_COMMAND && (dwPresStatReg & CSP_BITFMASK(ESDHC_PRSSTAT_CDIHB)))
    {
        OALMSG(OAL_VERBOSE, (L"SDHCCmdConfig: CDIHB set, can't Send Data Cmd!!\r\n"));

        // reset the data portion of ESDHC
        SETREG32(&g_pESDHCReg->SYSCTL, CSP_BITFMASK(ESDHC_SYSCTL_RSTD));
    }

    if (SDController.SendInitClocks) 
    {
        SDController.SendInitClocks = FALSE;

        // send the initialization clocks (80) before first command is sent
        SETREG32(&g_pESDHCReg->SYSCTL, CSP_BITFMASK(ESDHC_SYSCTL_INITA));
        // wait until done sending init clocks
        while(INREG32(&g_pESDHCReg->SYSCTL) & CSP_BITFMASK(ESDHC_SYSCTL_INITA));
    }

    // Set the command index
    dwXferTypReg = CSP_BITFVAL(ESDHC_XFERTYP_CMDINX, ucCmd);
    
    dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_CMDTYP, ESDHC_CMD_NORMAL);    

    // set DPSEL and block size for transfers that are not command-only
    if (TransferClass != SD_COMMAND)
    {

        // workaround for hw errata for ESDHC versions < 2.2 that writes following reads will result in DCE error
        if (dwVVN < ESDHC_VVN_FSL22 && TransferClass == SD_WRITE && bLastCmdRead)
            SETREG32(&g_pESDHCReg->SYSCTL, CSP_BITFMASK(ESDHC_SYSCTL_RSTD));
    
        dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_DPSEL, 1);

        // setup blocksize and number of blocks to transfer (= 1 for single transfer)
        OUTREG32(&g_pESDHCReg->BLKATTR, (CSP_BITFVAL(ESDHC_BLKATTR_BLKSIZE, pReq->BlockSize) | CSP_BITFVAL(ESDHC_BLKATTR_BLKCNT, pReq->NumBlocks)) );
        // setup data bus width
        if (SDController.BusWidthSetting == SD_INTERFACE_SD_MMC_1BIT)
        {
            INSREG32BF(&g_pESDHCReg->PROCTL, ESDHC_PROCTL_DTW, ESDHC_DTW_1BIT);
        }
        else if (SDController.BusWidthSetting == SD_INTERFACE_SD_4BIT)
        {
            INSREG32BF(&g_pESDHCReg->PROCTL, ESDHC_PROCTL_DTW, ESDHC_DTW_4BIT);
        }
        else
        {
            INSREG32BF(&g_pESDHCReg->PROCTL, ESDHC_PROCTL_DTW, ESDHC_DTW_8BIT);
        }

        // set watermark level (WML register units are 4-byte words, not bytes
        SDController.Bytes_in_fifo = min(pReq->BlockSize, ESDHC_MAX_DATA_BUFFER_SIZE);
        SDController.Words_in_fifo = SDController.Bytes_in_fifo >> 2;
        OUTREG32(&g_pESDHCReg->WML, CSP_BITFVAL(ESDHC_WML_RDWML, SDController.Words_in_fifo)
                                                      | CSP_BITFVAL(ESDHC_WML_WRWML, SDController.Words_in_fifo) );

        // Multi-block transfer? Set BCEN, MSBSEL bits
        if (pReq->NumBlocks > 1)
        {
            dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_MSBSEL, 1);
            dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_BCEN, 1);

            // Set AC12EN bit to have controller auto-send the CMD12 at the end of multi-block transaction
            dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_AC12EN, 1);

        }

        // set DTDSEL for reads from card
        if (TransferClass == SD_READ)
        {
            dwXferTypReg |= CSP_BITFVAL(ESDHC_XFERTYP_DTDSEL, 1);
            bLastCmdRead = TRUE;
        }
        else
        {
            bLastCmdRead = FALSE;
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
        break;
    }
    
    // Clear Interrupt Status Register (all bits) before sending the command
    OUTREG32(&g_pESDHCReg->IRQSTAT, 0xFFFFFFFF);
    
    // Write the command argument, and send the command
    OUTREG32(&g_pESDHCReg->CMDARG, Arg);
    OUTREG32(&g_pESDHCReg->XFERTYP, dwXferTypReg);
    
    return; 
}

//------------------------------------------------------------------------------
//
// Function: SDHCWaitEndCmdRespIntr
//
//    Wait a END_CMD_RESP interrupt by polling status register. The fields are checked 
//    to determine if an error has occurred.
//
// Parameters:
//        None
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
static UINT32 SDHCWaitEndCmdRespIntr()
{
    UINT32 sdhc_status = ESDHC_STATUS_FAILURE;
    DWORD status = 0;
    
    OALMSG(OAL_FUNC, (_T("+ SDHCWaitEndCmdRespIntr\r\n")));

    while (!( (status = INREG32(&g_pESDHCReg->IRQSTAT)) &  (CSP_BITFMASK(ESDHC_IRQSTAT_CC) | ESDHC_CMD_ERROR_BITS)));

    /* Check whether the interrupt is an END_CMD_RESP  
     * or an error 
     */         
    if((status & CSP_BITFMASK(ESDHC_IRQSTAT_CC)) &&  !(status & ESDHC_CMD_ERROR_BITS) )
    {
        sdhc_status = ESDHC_STATUS_PASS;
    }

    OALMSG(OAL_VERBOSE, (_T("SDHCWaitEndCmdRespIntr Status: %x\r\n"), status));

    OALMSG(OAL_FUNC, (_T("- SDHCWaitEndCmdRespIntr\r\n")));
     
    return sdhc_status;
}

//------------------------------------------------------------------------------
//
// Function: SDHCCheckDataStatus
//
// Parameters:
//        None
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
static UINT32 SDHCCheckDataStatus ()
{
    UINT32 sdhc_status = INREG32 (&g_pESDHCReg->IRQSTAT);

    OALMSG(OAL_FUNC, (_T("+ CheckDataStatus\r\n")));

    /* Check whether there is a data time out error, a data end bit error, or a CRC error  */
    if(sdhc_status & ESDHC_DAT_ERROR_BITS)
    {
        sdhc_status = ESDHC_STATUS_FAILURE;
    }
    else
    {
        sdhc_status = ESDHC_STATUS_PASS;
    }

    return sdhc_status;
}

//------------------------------------------------------------------------------
//
// Function: SDHCSendCmdWaitResp
//
//    Execute a command and wait for the response
//
// Parameters:
//        pReq[in] - SD structure
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 SDHCSendCmdWaitResp(PSD_BUS_REQUEST pReq)
{
    UINT32 sdhc_status = ESDHC_STATUS_FAILURE;

    /* Configure XFERTYP register to send the command */
    SDHCCmdConfig(pReq);

    /* Wait for interrupt end_command_resp */
    sdhc_status = SDHCWaitEndCmdRespIntr();

    /* Check if an error occured */
    return sdhc_status;

}

//------------------------------------------------------------------------------
//
// Function: SDHCReadResponse
//
//    Read the response returned by the card after a command 
//
// Parameters:
//        pResp[in] - structure to fill the response
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 SDHCReadResponse (PSD_COMMAND_RESPONSE pResp)
{
    UINT32 status = ESDHC_STATUS_FAILURE;
 
    // skip the CRC
    UNALIGNED PDWORD respBuff = (PDWORD)(pResp->ResponseBuffer + 1);
    DWORD dwRespTyp = pResp->ResponseType;
    
    if (dwRespTyp != NoResponse)
    {
        respBuff[0] = INREG32(&g_pESDHCReg->CMDRSP0);

        if (dwRespTyp == ResponseR2)
        {
            respBuff[1] = INREG32(&g_pESDHCReg->CMDRSP1);
            respBuff[2] = INREG32(&g_pESDHCReg->CMDRSP2);
            respBuff[3] = INREG32(&g_pESDHCReg->CMDRSP3);   
        }
    
        status = ESDHC_STATUS_PASS;
    }
    
    return status;

}

//------------------------------------------------------------------------------
//
// Function: SDHCDataRead
//
//    Read the data from the card.
//
// Parameters:
//        dest_ptr[out] - Destination memory address
//        transferSize[in] - number of bytes to transfer
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 SDHCDataRead (UINT32* dest_ptr, UINT32 transferSize) 
{
    // transferSize MUST be multiple of read watermark level   
    DWORD dwBytesRead = 0;
    DWORD dwBurstLen = EXTREG32BF(&g_pESDHCReg->WML, ESDHC_WML_RDWML); 
    
    DWORD dwDwordsRemaining = 0;
    UINT32 status = ESDHC_STATUS_FAILURE;

    OALMSG(OAL_FUNC, (_T("+ SDHCDataRead\r\n")));

    // we will read out burst length of data until we read out transferSize number of bytes
    while(dwBytesRead < transferSize)
    {
        // wait for Data Buffer to be ready
        while(! (INREG32(&g_pESDHCReg->IRQSTAT) & CSP_BITFMASK(ESDHC_IRQSTAT_BRR)) );   

        dwDwordsRemaining = dwBurstLen;
        while(dwDwordsRemaining--)
        {
            *(dest_ptr++) = INREG32(&g_pESDHCReg->DATPORT);
        }

        dwBytesRead += (dwBurstLen << 2);

        // clear the BRR bit
        OUTREG32(&g_pESDHCReg->IRQSTAT, CSP_BITFMASK(ESDHC_IRQSTAT_BRR));
    }

    // Wait for transfer complete operation interrupt
    while(! (INREG32(&g_pESDHCReg->IRQSTAT) & CSP_BITFMASK(ESDHC_IRQSTAT_TC)) );
        
    /* Check for status errors */    
    status = SDHCCheckDataStatus();

    OALMSG(OAL_FUNC, (_T("- SDHCDataRead\r\n")));
    
    return status;
    
}


//------------------------------------------------------------------------------
//
// Function: SDHCDataWrite
//
//    Read the data from the card
//
// Parameters:
//        src_ptr[out] - Source memory address
//        transferSize[in] - number of bytes to transfer
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 SDHCDataWrite (UINT32* src_ptr, UINT32 transferSize) 
{
    // transferSize MUST be multiple of write watermark level   
    DWORD dwBytesWritten = 0;
    DWORD dwBurstLen = EXTREG32BF(&g_pESDHCReg->WML, ESDHC_WML_WRWML);

    DWORD dwDwordsRemaining = 0;
    UINT32 status = ESDHC_STATUS_FAILURE;

    OALMSG(OAL_FUNC, (_T("+ SDHCDataWrite\r\n")));

    // we will read out burst length of data until we write out transferSize number of bytes
    while(dwBytesWritten < transferSize)
    {
        // wait for Data Buffer to be ready
        while(!(INREG32(&g_pESDHCReg->IRQSTAT) & CSP_BITFMASK(ESDHC_IRQSTAT_BWR)));

        dwDwordsRemaining = dwBurstLen;
        while(dwDwordsRemaining--)
        {
            OUTREG32(&g_pESDHCReg->DATPORT, *(src_ptr++));
        }

        dwBytesWritten += (dwBurstLen << 2);

        // clear the BWR bit
        OUTREG32(&g_pESDHCReg->IRQSTAT, CSP_BITFMASK(ESDHC_IRQSTAT_BWR));
        
    }

    // Wait for transfer complete operation interrupt
    while(!(INREG32(&g_pESDHCReg->IRQSTAT) & CSP_BITFMASK(ESDHC_IRQSTAT_TC)));
        
    /* Check for status errors */    
    status = SDHCCheckDataStatus();
    
    OALMSG(OAL_FUNC, (_T("- SDHCDataWrite %d\r\n"), status));
    
    return status;
}


//------------------------------------------------------------------------------
//
// Function: BSP_MMC4BitSupported
//
//      Whether the MMC 4bit mode supported or not
//
// Parameters:
//      None.
//
// Returns:
//      TRUE for MMC 4bit supported, FALSE for MMC 4bit not supported
//
//------------------------------------------------------------------------------
BOOL BSP_MMC4BitSupported()
{
    return FALSE;
}


//------------------------------------------------------------------------------
//
// Function: BSP_GetSDImageInfo
//
//    Get the image parameters
//
// Parameters:
//        pSDImageContext[out] - image parameters
//
// Returns:
//        the image parameters
//
//------------------------------------------------------------------------------
void BSP_GetSDImageCfg(PSD_IMAGE_CFG pSDImageCfg)
{
    pSDImageCfg->dwXldrOffset = IMAGE_BOOT_XLDRIMAGE_SD_OFFSET;
    pSDImageCfg->dwXldrSize = IMAGE_BOOT_XLDRIMAGE_SD_SIZE;

    pSDImageCfg->dwBootOffset = IMAGE_BOOT_BOOTIMAGE_SD_OFFSET;
    pSDImageCfg->dwBootSize = IMAGE_BOOT_BOOTIMAGE_SD_SIZE;

    pSDImageCfg->dwNkOffset = IMAGE_BOOT_NKIMAGE_SD_OFFSET;
    pSDImageCfg->dwNkSize = IMAGE_BOOT_NKIMAGE_SD_SIZE;

    pSDImageCfg->dwCfgOffset = IMAGE_BOOT_BOOTCFG_SD_OFFSET;
    pSDImageCfg->dwCfgSize = IMAGE_BOOT_BOOTCFG_SD_SIZE;

    pSDImageCfg->dwNkRAMOffset = IMAGE_BOOT_NKIMAGE_RAM_PA_START;
    pSDImageCfg->dwSdSize = IMAGE_BOOT_SDHCDEV_SD_SIZE;
}


