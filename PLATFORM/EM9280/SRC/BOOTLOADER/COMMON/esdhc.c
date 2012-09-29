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
//  Copyright (C) 2009-2010 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#include "bsp.h"
#include "sdfmd.h"
#include "sdmmc.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregPINCTRL;
extern PVOID pv_HWregCLKCTRL;


//-----------------------------------------------------------------------------
// Defines
// Card Detect
#define BIT_CARD_DETECT (0x1 << 9)

// Debug MSG
#define WARNING_MSG     (0)
#define DUMP_DATA       (0)

// CFG Offset and Size
#define CFG_SIZE        (512)

#define MAX_SDHC_FREQ  (12000000)

//-----------------------------------------------------------------------------
// Types

// Clock Rate Table
typedef struct _CLOCK_RATE_ENTRY {
    UINT32 Frequency;
    UINT32 ControlValue;
} CLOCK_RATE_ENTRY, *PCLOCK_RATE_ENTRY;

CLOCK_RATE_ENTRY SDClockTable[] =
{   {100000,   240},
    {400000,   60},
    {1000000,  24},
    {2000000,  12},
    {3000000,  8},
    {6000000,  4},
    {12000000, 2},   // 12 Mhz
    {24000000, 1},   // 24 Mhz
};

#define NUM_CLOCK_ENTRIES (sizeof(SDClockTable) / sizeof(CLOCK_RATE_ENTRY))


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
PVOID pv_HWregSSP0 = NULL;
PVOID pv_HWregSSP1 = NULL;
PVOID pv_HWregSSP2 = NULL;
PVOID pv_HWregSSP3 = NULL;
UINT32 dwSSPIndex = 0;

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
void SDConfigPins(void)
{
    //OALMSG(OAL_FUNC, (TEXT("SDConfigPins Begin\r\n")));
    OALMSG(1, (TEXT("SDConfigPins Begin\r\n")));
    
    //// Clear SFTRST
    //HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_SFTRST);
    //while(HW_PINCTRL_CTRL_RD() & BM_PINCTRL_CTRL_SFTRST);

    //// Clear CLKGATE
    //HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_CLKGATE);
    //while(HW_PINCTRL_CTRL_RD() & BM_PINCTRL_CTRL_CLKGATE);

	// PWM3
    DDKIomuxSetPinMux(DDK_IOMUX_PWM3_1, DDK_IOMUX_MODE_GPIO);
    DDKGpioEnableDataPin(DDK_IOMUX_PWM3_1, 1);
    DDKGpioWriteDataPin(DDK_IOMUX_PWM3_1, 0);

    // Delay 1 ms (=1000 us) for reset as well as power supply ramp up (controlled by PWM3 gpio line)
    OALStall(1000);

    // Detect
    DDKIomuxSetPinMux(DDK_IOMUX_SSP0_CARD_DETECT, DDK_IOMUX_MODE_GPIO);
    DDKGpioEnableDataPin(DDK_IOMUX_SSP0_CARD_DETECT, 0);
    HW_PINCTRL_IRQLEVEL2_CLR(BIT_CARD_DETECT);
    HW_PINCTRL_IRQPOL2_CLR(BIT_CARD_DETECT);
    HW_PINCTRL_IRQSTAT2_CLR(BIT_CARD_DETECT);
    HW_PINCTRL_PIN2IRQ2_SET(BIT_CARD_DETECT);

    // CLK
    DDKIomuxSetPinMux(DDK_IOMUX_SSP0_SCK, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_SCK, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    // CMD
    DDKIomuxSetPinMux(DDK_IOMUX_SSP0_CMD, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_CMD, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    
    // DATA0
    DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D0, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D0, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    // DATA1
    DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D1, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D1, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    // DATA2
    DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D2, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D2, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    // DATA3
    DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D3, DDK_IOMUX_MODE_00);
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D3, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    // WP on SSP1_SCK as GPIO input on EVK
    DDKIomuxSetPinMux(DDK_IOMUX_SSP1_SCK_1, DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP1_SCK_1, 
                         DDK_IOMUX_PAD_DRIVE_8MA, 
                         DDK_IOMUX_PAD_PULL_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKGpioEnableDataPin(DDK_IOMUX_SSP1_SCK_1, 0);        // make it an input
   
    
    OALMSG(OAL_FUNC, (TEXT("SDConfigPins End\r\n")));
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
BOOL SDInterface_Init(void)
{

    DWORD dwClockRate = ESDHC_INIT_CLOCK_RATE;

    OALMSG(1, (TEXT("->SDInterface_Init\r\n")));

    if(!SDHC_IsCardPresent())
    {
		OALMSG(1, (TEXT("SDInterface_Init: Card is not present\r\n")));
        return FALSE; 
    }

    pv_HWregSSP0 = (PVOID)OALPAtoUA(CSP_BASE_REG_PA_SSP0);
    dwSSPIndex = 0;
    
    // Clock Reset
    HW_CLKCTRL_SSP0_CLR(BM_CLKCTRL_SSP0_CLKGATE);
    while(HW_CLKCTRL_SSP0_RD() & BM_CLKCTRL_SSP0_CLKGATE)
    {
        ;
    }
    
    // SSP Reset
    HW_SSP_CTRL0_CLR(dwSSPIndex, BM_SSP_CTRL0_SFTRST);
    while(HW_SSP_CTRL0_RD(dwSSPIndex) & BM_SSP_CTRL0_SFTRST)
    {
        ;
    }

    HW_SSP_CTRL0_CLR(dwSSPIndex, BM_SSP_CTRL0_CLKGATE);
    while(HW_SSP_CTRL0_RD(dwSSPIndex) & BM_SSP_CTRL0_CLKGATE)
    {
        ;
    }

    // Initialize SSP for SD/MMC card
    HW_SSP_TIMING_WR(dwSSPIndex, 
                     (BF_SSP_TIMING_TIMEOUT(0xFFFF) |
                     BF_SSP_TIMING_CLOCK_DIVIDE(0xF0) |
                     BF_SSP_TIMING_CLOCK_RATE(0)));

    // 
    HW_SSP_CTRL1_WR(dwSSPIndex, 
                    BM_SSP_CTRL1_RESP_ERR_IRQ_EN |
                    BM_SSP_CTRL1_RESP_TIMEOUT_IRQ_EN |
                    BM_SSP_CTRL1_DATA_TIMEOUT_IRQ_EN |
                    BM_SSP_CTRL1_DATA_CRC_IRQ_EN |
                    BM_SSP_CTRL1_FIFO_UNDERRUN_EN |
                    BM_SSP_CTRL1_FIFO_OVERRUN_IRQ_EN |
                    BM_SSP_CTRL1_DMA_ENABLE |
                    BM_SSP_CTRL1_POLARITY |
                    BF_SSP_CTRL1_WORD_LENGTH(BV_SSP_CTRL1_WORD_LENGTH__EIGHT_BITS) |
                    BF_SSP_CTRL1_SSP_MODE(BV_SSP_CTRL1_SSP_MODE__SD_MMC));

    // Set Clock
    SetClockRate(&dwClockRate);

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
BOOL SDHC_IsCardPresent(void)
{
    // SSP0_DET (bank2 pin9)
    if(HW_PINCTRL_DIN2_RD() & BIT_CARD_DETECT)
    {
        OALMSG(OAL_INFO, (TEXT("SD/MMC card is not found.\r\n")));
        return FALSE;
    }
    else
    {
        OALMSG(OAL_FUNC, (TEXT("SD/MMC card is present.\r\n")));
        SDController.SendInitClocks = TRUE;

        // delay 250 ms for card ramp up time (SD spec)
        OALStall(250000);
        return TRUE;
    }
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
    BOOL retVal = FALSE;

    DDKGpioReadDataPin(DDK_IOMUX_SSP1_SCK_1, (PUINT32) &retVal);

    return (retVal & 0x1);

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
   
    UINT32 ControlValue, regTemp;
    DWORD i;
    DWORD rate = *pdwRate;

    // For higher performance, but have SD compatibility concern
    if(rate > MAX_SDHC_FREQ)
    {
        rate = MAX_SDHC_FREQ;
    }

    // check to see if the rate is below the first entry in the table
    if(rate <= SDClockTable[0].Frequency) 
    {
        i = 0;
    } 
    else 
    {
        // scan through the table looking for a frequency that
        // is close to the requested rate
        for(i = 0; i < (NUM_CLOCK_ENTRIES - 1); i++) 
        {
            if((rate >= SDClockTable[i].Frequency) &&
               (rate < SDClockTable[i+1].Frequency)) 
            {
                break;
            }
        }
    }

    OALMSG(OAL_FUNC, (TEXT("SetClockRate - Requested Rate: %d, Setting clock rate to %d Hz \r\n"),
           *pdwRate, SDClockTable[i].Frequency ));

    // return the actual fruency
    *pdwRate = SDClockTable[i].Frequency;
    ControlValue = SDClockTable[i].ControlValue;
    
    // set the clock rate
    regTemp = HW_SSP_TIMING_RD(dwSSPIndex);
    regTemp &= 0xFFFF0000;
    regTemp |= (ControlValue << 8);
    HW_SSP_TIMING_WR(dwSSPIndex, regTemp);

    OALMSG(OAL_FUNC, (TEXT("-SetClockRate : regTemp = 0x%X, HW_SSP_TIMING_RD = 0x%X\r\n"), regTemp, HW_SSP_TIMING_RD(dwSSPIndex)));
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
    UINT32 cmdatRegister = 0;
    
    while((HW_SSP_STATUS_RD(dwSSPIndex) & (BM_SSP_STATUS_BUSY | BM_SSP_STATUS_DATA_BUSY | BM_SSP_STATUS_CMD_BUSY)))
    {
        // delay 1 ms (=1000 us)
        OALStall(1000);        
        OALMSG(WARNING_MSG, (TEXT("SD Bus Busy : HW_SSP_STATUS = 0x%X\r\n"), HW_SSP_STATUS_RD(dwSSPIndex)));
    }

    if (SDController.SendInitClocks)
    {
        //send init clocks for 1ms
        SDController.SendInitClocks = FALSE;

        HW_SSP_CMD0_SET(dwSSPIndex, BM_SSP_CMD0_CONT_CLKING_EN);

        // delay 1 ms (=1000 us), 74 clock cyles (SD spec) at 100 khz will take 0.74 ms, but need to wait at least 1 ms
        OALStall(1000);
        
        HW_SSP_CMD0_CLR(dwSSPIndex, BM_SSP_CMD0_CONT_CLKING_EN);
    }
    
    // clear CTRL0 and COM0 register first
    HW_SSP_CTRL0_CLR(dwSSPIndex, 0xFFFFFFFF); // 0xEFFFFFFF
    HW_SSP_CMD0_CLR(dwSSPIndex, 0xFFFFFFFF);
    HW_SSP_XFER_SIZE_CLR(dwSSPIndex, 0xFFFFFFFF);
    HW_SSP_BLOCK_SIZE_CLR(dwSSPIndex, 0xFFFFFFF);
    
    // set the command
    HW_SSP_CMD0_SET(dwSSPIndex, BF_SSP_CMD0_CMD(pReq->CommandCode));
    if((pReq->CommandCode & 0xFF) == 52 ||
       (pReq->CommandCode & 0xFF) == 53)
    {
        HW_SSP_CMD0_SET(dwSSPIndex, BF_SSP_CMD0_APPEND_8CYC(1));
    }
    
    // set the argument
    HW_SSP_CMD1_WR(dwSSPIndex, pReq->CommandArgument);
    
    OALMSG(OAL_FUNC, (TEXT("+SDHCCmdConfig - ResponseType: 0x%x\r\n"), pReq->CommandResponse.ResponseType));
    
    switch (pReq->CommandResponse.ResponseType) 
    {
    case NoResponse:
        //No Setting here
        break;

    case ResponseR1b:
        // response1 with busy signalling
        cmdatRegister = BM_SSP_CTRL0_GET_RESP;
        break;

    case ResponseR1:
    case ResponseR5:
    case ResponseR6:
    case ResponseR7:
        // on an MMC controller R5 and R6 are really just an R1 response (CRC protected)
        cmdatRegister = BM_SSP_CTRL0_GET_RESP;
        break;

    case ResponseR2:
        cmdatRegister = BM_SSP_CTRL0_GET_RESP | BM_SSP_CTRL0_LONG_RESP;
        break;

    case ResponseR3:
    case ResponseR4:
        // R4 is really same as an R3 response on an MMC controller (non-CRC)
        cmdatRegister = BM_SSP_CTRL0_GET_RESP | BM_SSP_CTRL0_IGNORE_CRC;
        break;
    
    default:
        OALMSG(WARNING_MSG, (TEXT("SDHCCmdConfig : Invalid ResponseType\r\n")));
        break;
    }
    
    // check for Command Only
    if((SD_COMMAND == pReq->TransferClass)) 
    {
        OALMSG(OAL_FUNC, (TEXT("SDHCCmdConfig : SD_COMMAND\r\n")));

        // Set BLOCK_SIZE = 0 and BLOCK_COUNT = 0
        HW_SSP_CMD0_CLR(dwSSPIndex, 0xFFF00);
        HW_SSP_CMD0_SET(dwSSPIndex, BF_SSP_CMD0_APPEND_8CYC(1));
    } 
    else 
    {
        // command with a data phase
        cmdatRegister |= BM_SSP_CTRL0_DATA_XFER;
    
        OALMSG(OAL_FUNC, (TEXT( "SDHCCmdConfig : pRequest->BlockSize: %d, pRequest->NumBlocks: %d\r\n" ), pReq->BlockSize, pReq->NumBlocks));
        HW_SSP_CMD0_CLR(dwSSPIndex, 0xFFF00);
    
        if(pReq->NumBlocks > 1)
        {
            switch(pReq->BlockSize)
            {
            case 1:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (0) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;
                
            case 2:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (1) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;
                
            case 4:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (2) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;
                
            case 8:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (3) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;
                
            case 16:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (4) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;
                
            case 32:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (5) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;
                
            case 64:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (6) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));  
                break;
                
            case 128:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (7) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;
                
            case 256:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (8) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;
                
            case 512:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (9) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;
                
            case 1024:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (10) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;

            case 2048:
                 HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (11) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;

            case 4096:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (12) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;

            case 8192:
                HW_SSP_BLOCK_SIZE_SET(dwSSPIndex, BF_SSP_BLOCK_SIZE_BLOCK_SIZE (13) | BF_SSP_BLOCK_SIZE_BLOCK_COUNT(pReq->NumBlocks - 1));
                break;

            default:
                OALMSG(WARNING_MSG, (TEXT( "+SDHCCmdConfig: BLOCK_SIZE is not handled, pRequest->BlockSize: %d, pRequest->NumBlocks: %d\r\n" ), pReq->BlockSize, pReq->NumBlocks));
                break;
            }
        }   
        
        HW_SSP_XFER_SIZE_SET(dwSSPIndex, pReq->BlockSize * pReq->NumBlocks);
    
        // check for read
        if(SD_READ == pReq->TransferClass) 
        {
            cmdatRegister |= BM_SSP_CTRL0_READ;
        }
    
    }

    cmdatRegister |= (BM_SSP_CTRL0_ENABLE | BM_SSP_CTRL0_WAIT_FOR_IRQ);

    if (SDController.BusWidthSetting == SD_INTERFACE_SD_MMC_1BIT)
    {
        cmdatRegister |= BF_SSP_CTRL0_BUS_WIDTH(BV_SSP_CTRL0_BUS_WIDTH_ONE_BIT);
    }
    else if (SDController.BusWidthSetting == SD_INTERFACE_SD_4BIT)
    {
        cmdatRegister |= BF_SSP_CTRL0_BUS_WIDTH(BV_SSP_CTRL0_BUS_WIDTH_FOUR_BIT);
    }
    else
    {
        cmdatRegister |= BF_SSP_CTRL0_BUS_WIDTH(BV_SSP_CTRL0_BUS_WIDTH_EIGHT_BIT);
    }
    
    HW_SSP_CTRL0_WR(dwSSPIndex, cmdatRegister);
    
    OALMSG(OAL_FUNC, (TEXT("SDHCCmdConfig - CMDAT Reg: 0x%08X, CMD:%d \r\n"),
                 cmdatRegister, pReq->CommandCode));
    
    HW_SSP_CTRL1_CLR(dwSSPIndex, 0x2AA08000);
         
    // if((cmdatRegister & BM_SSP_CTRL0_DATA_XFER) == 0)
    {
        HW_SSP_CTRL0_SET(dwSSPIndex, BM_SSP_CTRL0_RUN);
    }

    OALMSG(OAL_FUNC, (TEXT("HW_SSP_STATUS = 0x%X\r\nHW_SSP_CMD0 = 0x%X\r\nHW_SSP_CMD1 = 0x%X\r\nHW_SSP_CTRL0 = 0x%X\r\nHW_SSP_CTRL1 = 0x%X\r\n"), 
                        HW_SSP_STATUS_RD(dwSSPIndex), HW_SSP_CMD0_RD(dwSSPIndex), HW_SSP_CMD1_RD(dwSSPIndex),
                        HW_SSP_CTRL0_RD(dwSSPIndex), HW_SSP_CTRL1_RD(dwSSPIndex)));

    // delay 1 ms
    OALStall(1000);
    
    // Wait for command and response to complete
    while(HW_SSP_STATUS_RD(dwSSPIndex) & BM_SSP_STATUS_CMD_BUSY)
    {
        OALMSG(WARNING_MSG, (TEXT("Wait for command and response to complete.\r\n")));
        // delay 1 ms
        OALStall(1000);
    }
}


//------------------------------------------------------------------------------
//
// Function: GetInterruptStatus
//
//    Get the interrupt.
//
// Parameters:
//        None
//
// Returns:
//        DWORD
//
//------------------------------------------------------------------------------
DWORD GetInterruptStatus()
{
    volatile DWORD dwRegValue;
    DWORD dwInterruptStatus;
    DWORD dwInterruptMask;

    dwRegValue = HW_SSP_CTRL1_RD(dwSSPIndex);
    dwInterruptStatus = (dwRegValue & 0xAAAA8000);
    dwInterruptMask = (dwRegValue & 0x55554000) << 1;

    OALMSG(OAL_FUNC, (TEXT("HW_SSP_CTRL1 = 0x%X\r\n"), dwRegValue));
    
    return (dwInterruptStatus & dwInterruptMask);

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

    DWORD dwIntrStatus = GetInterruptStatus();
    
    if(dwIntrStatus == 0)
    {
        sdhc_status = ESDHC_STATUS_PASS;
        OALMSG(OAL_FUNC, (TEXT("SDHCWaitEndCmdRespIntr Pass\r\n")));
    }
    else
    {
        OALMSG(WARNING_MSG, (TEXT("SDHCWaitEndCmdRespIntr Fail\r\n")));
        if(dwIntrStatus & BM_SSP_CTRL1_RESP_TIMEOUT_IRQ)
        {
            OALMSG(WARNING_MSG, (TEXT("RESP_TIMEOUT_IRQ\r\n")));
            HW_SSP_CTRL1_CLR(dwSSPIndex, BF_SSP_CTRL1_DATA_CRC_IRQ(1));
        }

        if(dwIntrStatus & BM_SSP_CTRL1_RESP_ERR_IRQ)
        {
            OALMSG(WARNING_MSG, (TEXT("RESP_ERR_IRQ\r\n")));
            HW_SSP_CTRL1_CLR(dwSSPIndex, BF_SSP_CTRL1_DATA_CRC_IRQ(1));
        }

        if(dwIntrStatus & BM_SSP_CTRL1_DATA_TIMEOUT_IRQ)
        {
            OALMSG(WARNING_MSG, (TEXT("DATA_TIMEOUT_IRQ\r\n")));
            HW_SSP_CTRL1_CLR(dwSSPIndex, BF_SSP_CTRL1_DATA_CRC_IRQ(1));
        }

        if(dwIntrStatus & BM_SSP_CTRL1_DATA_CRC_IRQ)
        {
            OALMSG(WARNING_MSG, (TEXT("DATA_CRC_IRQ\r\n")));
            HW_SSP_CTRL1_CLR(dwSSPIndex, BF_SSP_CTRL1_DATA_CRC_IRQ(1));
        }

        // FIFO_UNDERRUN_IRQ
        if(dwIntrStatus & BF_SSP_CTRL1_FIFO_UNDERRUN_IRQ(1))
        {
            OALMSG(WARNING_MSG, (TEXT("*****************FIFO_UNDERRUN_IRQ\r\n")));
            HW_SSP_CTRL1_CLR(dwSSPIndex, BF_SSP_CTRL1_FIFO_UNDERRUN_IRQ(1));
        }

        // FIFO_OVERRUN_IRQ
        if(dwIntrStatus & BF_SSP_CTRL1_FIFO_OVERRUN_IRQ(1))
        {
            OALMSG(WARNING_MSG, (TEXT("*****************FIFO_OVERRUN_IRQ\r\n")));
            HW_SSP_CTRL1_CLR(dwSSPIndex, BF_SSP_CTRL1_FIFO_OVERRUN_IRQ(1));
        }
    
    }
       
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
static UINT32 SDHCCheckDataStatus()
{
    return ESDHC_STATUS_PASS;
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

    OALMSG(OAL_FUNC, (TEXT("SDHCSendCmdWaitResp Begin\r\n")));
    
    /* Configure XFERTYP register to send the command */
    SDHCCmdConfig(pReq);

    /* Wait for interrupt end_command_resp */
    sdhc_status = SDHCWaitEndCmdRespIntr();

    OALMSG(OAL_FUNC, (TEXT("SDHCSendCmdWaitResp End\r\n")));

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
UINT32 SDHCReadResponse(PSD_COMMAND_RESPONSE pResp)
{

    UINT32 status = ESDHC_STATUS_FAILURE;
    
    volatile DWORD  dwStatus;
    DWORD           regValue;
    DWORD           dataTemp;
    
    OALMSG(OAL_FUNC, (TEXT("SDHCReadResponse Begin\r\n")));
    // get the status register
    dwStatus = HW_SSP_STATUS_RD(dwSSPIndex);

    if(dwStatus & BM_SSP_STATUS_RESP_CRC_ERR) 
    {
        regValue = (HW_SSP_CMD0_RD(dwSSPIndex) & BM_SSP_CMD0_CMD);

        OALMSG(WARNING_MSG, (TEXT("SDHCReadResponse: response for command %d , contains a CRC error \r\n"), regValue));

        return status;
    }

    if (NoResponse != pResp->ResponseType) 
    {
        // read in the response words from the response fifo.
        if (ResponseR2 == pResp->ResponseType)  
        {
            OALMSG(OAL_FUNC, (TEXT("SDHCReadResponse - ResponseType: %d\r\n"), pResp->ResponseType));

            dataTemp = HW_SSP_SDRESP0_RD(dwSSPIndex);
            pResp->ResponseBuffer[0] = (UCHAR)(( (dataTemp >> 0) & 0xFF));
            pResp->ResponseBuffer[1] = (UCHAR)(( (dataTemp >> 8) & 0xFF));
            pResp->ResponseBuffer[2] = (UCHAR)(( (dataTemp >> 16) & 0xFF));
            pResp->ResponseBuffer[3] = (UCHAR)(( (dataTemp >> 24) & 0xFF));

            dataTemp = HW_SSP_SDRESP1_RD(dwSSPIndex);
            pResp->ResponseBuffer[4] = (UCHAR)(( (dataTemp >> 0) & 0xFF));
            pResp->ResponseBuffer[5] = (UCHAR)(( (dataTemp >> 8) & 0xFF));
            pResp->ResponseBuffer[6] = (UCHAR)(( (dataTemp >> 16) & 0xFF));
            pResp->ResponseBuffer[7] = (UCHAR)(( (dataTemp >> 24) & 0xFF));

            dataTemp = HW_SSP_SDRESP2_RD(dwSSPIndex);
            pResp->ResponseBuffer[8] = (UCHAR)(( (dataTemp >> 0) & 0xFF));
            pResp->ResponseBuffer[9] = (UCHAR)(( (dataTemp >> 8) & 0xFF));
            pResp->ResponseBuffer[10] = (UCHAR)(( (dataTemp >> 16) & 0xFF));
            pResp->ResponseBuffer[11] = (UCHAR)(( (dataTemp >> 24) & 0xFF));


            dataTemp = HW_SSP_SDRESP3_RD(dwSSPIndex);
            pResp->ResponseBuffer[12] = (UCHAR)(( (dataTemp >> 0) & 0xFF));
            pResp->ResponseBuffer[13] = (UCHAR)(( (dataTemp >> 8) & 0xFF));
            pResp->ResponseBuffer[14] = (UCHAR)(( (dataTemp >> 16) & 0xFF));
            pResp->ResponseBuffer[15] = (UCHAR)(( (dataTemp >> 24) & 0xFF));

        } 
        else 
        {
            OALMSG(OAL_FUNC, (TEXT("SDHCReadResponse - ResponseType: %d\r\n"), pResp->ResponseType));

            dataTemp = HW_SSP_SDRESP1_RD(dwSSPIndex);
            pResp->ResponseBuffer[5] = (UCHAR)( dataTemp & 0x3F );
            dataTemp = HW_SSP_SDRESP0_RD(dwSSPIndex);
            pResp->ResponseBuffer[4] = (UCHAR)(( (dataTemp >> 24) & 0xFF));
            pResp->ResponseBuffer[3] = (UCHAR)(( (dataTemp >> 16) & 0xFF));
            pResp->ResponseBuffer[2] = (UCHAR)(( (dataTemp >> 8) & 0xFF));
            pResp->ResponseBuffer[1] = (UCHAR)(( (dataTemp >> 0) & 0xFF));

            pResp->ResponseBuffer[0] = (UCHAR)((0xFF));

        }

        status = ESDHC_STATUS_PASS;
    }

    OALMSG(OAL_FUNC, (TEXT("+SDHCReadResponse - HW_SSP_CTRL1: 0x%x\r\n"), HW_SSP_CTRL1_RD(dwSSPIndex)));

    OALMSG(OAL_FUNC, (TEXT("SSDHCReadResponse End\r\n")));
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
UINT32 SDHCDataRead(UINT32 *dest_ptr, UINT32 transferSize) 
{    
    UINT32 status = ESDHC_STATUS_FAILURE;
    UINT32 *dwPtr = dest_ptr;
#if DUMP_DATA
    UINT32 dwCount = 0;
    UINT32 dwTransferSize = transferSize;
#endif    
    OALMSG(OAL_FUNC, (TEXT("SDHCDataRead : transferSize = %d\r\n"), transferSize));

    OALMSG(OAL_FUNC, (TEXT("HW_SSP_STATUS = 0x%X\r\n"), HW_SSP_STATUS_RD(dwSSPIndex)));

    while(transferSize > 0)
    {
        if((HW_SSP_STATUS_RD(dwSSPIndex) & BM_SSP_STATUS_FIFO_EMPTY) == 0)
        {
            *(dwPtr++) = HW_SSP_DATA_RD(dwSSPIndex);
            transferSize -= 4;
        }
    }

    // Dump data 
#if DUMP_DATA
    for( ; dwCount < (dwTransferSize / sizeof(UINT32)); dwCount++)
    {
        OALMSG(1, (TEXT("0x%X "), *(dest_ptr + dwCount)));
    }
    OALMSG(1, (TEXT("\r\n")));
#endif
    
    while((HW_SSP_STATUS_RD(dwSSPIndex) & (BM_SSP_STATUS_BUSY | BM_SSP_STATUS_DATA_BUSY | BM_SSP_STATUS_CMD_BUSY) ) != 0) //read until bus is not busy
    {
        OALMSG(WARNING_MSG, (TEXT("SDHCDataRead: Should not get here: HW_SSP_STATUS=0x%08lx\r\n"), HW_SSP_STATUS_RD(dwSSPIndex)));
        OALMSG(WARNING_MSG, (TEXT("SDHCDataRead: Should not get here: HW_SSP_DATA=0x%08lx\r\n"), HW_SSP_DATA_RD(dwSSPIndex)));
    }

    // Check for status errors
    status = SDHCCheckDataStatus();
    
    OALMSG(OAL_FUNC, (TEXT("SDHCDataRead End\r\n")));
    
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
UINT32 SDHCDataWrite(UINT32 *src_ptr, UINT32 transferSize) 
{

    UINT32 status = ESDHC_STATUS_FAILURE;
    UINT32 *dwPtr = src_ptr;
    
    OALMSG(OAL_FUNC, (TEXT("SDHCDataWrite : size = %d\r\n"), transferSize));

    OALMSG(OAL_FUNC, (TEXT("HW_SSP_STATUS = 0x%X\r\n"), HW_SSP_STATUS_RD(dwSSPIndex)));

    while(transferSize > 0)
    {
        if((HW_SSP_STATUS_RD(dwSSPIndex) & BM_SSP_STATUS_FIFO_FULL) == 0)
        {
            HW_SSP_DATA_WR(dwSSPIndex, *(dwPtr++));
            transferSize -= 4;
        }
    }

    status = SDHCCheckDataStatus();
    
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
    return TRUE;
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
    char buf[512] = {0};
    PMASTERBOOT_RECORD pMBR = (PMASTERBOOT_RECORD)buf;

    if(!SDMMC_ReadSector(0, (LPBYTE)buf, NULL, 1))
    {
        OALMSG(OAL_ERROR, (_T("ERROR: Failed to read MBR from SDHC\r\n")));
        return;
    }
    
    // Boot configuration data will be at the last sector of the card
    pSDImageCfg->dwCfgOffset = SDMMC_GetCardCapacity() - 1;
    pSDImageCfg->dwCfgSize = CFG_SIZE;

    pSDImageCfg->dwNkRAMOffset = IMAGE_BOOT_NKIMAGE_RAM_PA_START;

    // NK.nb0 always store in Partition 2
    pSDImageCfg->dwNkOffset = (LONGLONG)pMBR->Partition[2].RelativeSector*ESDHC_SECTOR_SIZE;
    pSDImageCfg->dwNkSize = (LONGLONG)(pMBR->Partition[2].TotalSector - 1)*ESDHC_SECTOR_SIZE; //last one sector is reserved for cfg?

    pSDImageCfg->dwBootOffset = (LONGLONG)((pMBR->Partition[1].RelativeSector + 4 + 3) & ~3)*ESDHC_SECTOR_SIZE;    //sector addr must be 4 aligned
    pSDImageCfg->dwBootSize = (LONGLONG)((pMBR->Partition[1].TotalSector - \
                        (pSDImageCfg->dwBootOffset / ESDHC_SECTOR_SIZE - pMBR->Partition[1].RelativeSector)) & ~7) * ESDHC_SECTOR_SIZE; //make sure boot size is 8 byte aligned
    
    // setup SocId = 28 for i.MX28, used to make decisions in the common boot code
    pSDImageCfg->dwSocId = 28;
}


