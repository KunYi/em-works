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
#include "sdfmd.h"


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables
extern SDCARD_CARD_REGISTERS sdCardReg;
extern USHORT                Card_rca;      /* Relative Card Address */
extern BOOL                  bHighDensityCard;


//-----------------------------------------------------------------------------
// Defines
#define SD_OCR_VALUE                0x40FF8000
#define SD_R1_STATUS_APP_CMD_MSK    0x20
#define SD_BUS_WIDTH_OFFSET         6
#define BIT_4_MODE                  4
#define SD_STATUS_LEN               64


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
DWORD eSDBootPartitionSize = 0;


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions
static SD_INTERFACE_MODE SDGetBitMode (void);
static UINT32 SDSetBusWidth (SD_INTERFACE_MODE mode);


//------------------------------------------------------------------------------
//
// Function: SD_Init
//
//    This function will initialize the SD memory card.
//
// Parameters:
//        None
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 SD_Init(void)
{
    UINT32 status = ESDHC_STATUS_FAILURE;
    DWORD dwClockRate = SD_FULL_SPEED_RATE;
    
    OALMSG(OAL_FUNC, (_T("+ SD_Init\r\n")));

    if(SD_VoltageValidation()) // Not a SD Card
    {
        return status;
    }

    /* Get CID number of SD Memory Card */
    if(!SDMMC_get_cid())
    {
        /* Set RCA of the SD Card */
        if(!SDMMC_send_rca(FALSE))
        {
            if(!SDMMC_get_csd())
            {
                /*Enable operating frequency */
                SetClockRate(&dwClockRate);

                /*Put SD Card in Transfer State */
                if((!SDMMC_set_data_transfer_mode()) &&
                   (!SDMMC_set_blocklen (SDHC_BLK_LEN)))
                {
                    status = ESDHC_STATUS_PASS;

                    if(BSP_MMC4BitSupported() && (SDGetBitMode() == SD_INTERFACE_SD_4BIT))
                    {
                        /* Set Bus width of the device */
                        status = SDSetBusWidth(SD_INTERFACE_SD_4BIT);
                        if (status == ESDHC_STATUS_PASS)
                        {
                            SDController.BusWidthSetting = SD_INTERFACE_SD_4BIT; // 4 Bit Mode
                            OALMSG(OAL_INFO, (_T("SD: Switched to 4 bit mode\r\n")));
                        }
                        
                    }

                    // eSD2.1 initialization
                    // query partition list, and activate boot partition if present
                    eSDBootPartitionSize = ESDQueryPartitionSize();
                    if(eSDBootPartitionSize > 0)
                    {
                        ESDSetActivePartition(ESD_SET_BOOT_PARTITION1);
                    }
                }
            }
        }
    }
    
    OALMSG(OAL_FUNC, (_T("- SD_Init\r\n")));

    return status;
}


//------------------------------------------------------------------------------
//
// Function: SD_VoltageValidation
//
//    This function will validate the operating voltage range of SDMemory card.
//
// Parameters:
//        None
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 SD_VoltageValidation (void)
{
    SD_BUS_REQUEST sdRequest;
    PSD_COMMAND_RESPONSE response = &sdRequest.CommandResponse;
    UINT32 ocr_value=0;
    UINT32 voltage_validation = ESDHC_STATUS_FAILURE;
    UINT32 StartTime = OEMEthGetSecs();    

    OALMSG(OAL_FUNC, (_T("+ SD_VoltageValidation\r\n")));
   
    SDCard_command_config(&sdRequest, SD_CMD_SEND_IF_COND, 0x1AA, SD_COMMAND ,ResponseR1);
    SDHCSendCmdWaitResp(&sdRequest);

    while((OEMEthGetSecs() - StartTime < ESDHC_DELAY_TIMEOUT) && (voltage_validation != ESDHC_STATUS_PASS)) 
    {
        if(SDMMC_card_send_appcmd() == ESDHC_STATUS_FAILURE)
        {
            // Card won't go into APPCMD mode, not a valid card
            break;
        }
        else
        {
            /* Configure ACMD41 for SD card */
            /* This command expects operating voltage range as argument.*/
    
            ocr_value = ((UINT32)(SD_OCR_VALUE) & 0xFFFFFFFF);
        
            SDCard_command_config(&sdRequest, SD_ACMD_SD_SEND_OP_COND, ocr_value, SD_COMMAND ,ResponseR3);
        
            /* Issue ACMD41 to SD Memory card to determine OCR value */
            if(SDHCSendCmdWaitResp(&sdRequest) == ESDHC_STATUS_FAILURE)
            {
                // not an SD memory card
                break;
            }
            else
            {
                /* Read Response from CMDRSP0 Register */
                response->ResponseType = ResponseR3;
                SDHCReadResponse(response);
            
                /* Obtain OCR Values from the response */
                memcpy((&ocr_value), &response->ResponseBuffer[1], sizeof(UINT32));        
        
                OALMSG(OAL_VERBOSE, (_T("SD OCR Value %x\r\n"), ocr_value));

                /* Check if volatge lies in range or not*/
                if((ocr_value & MMC_OCR_VALUE_MASK) == MMC_OCR_VALUE_MASK)
                {
                    /* Check if card busy bit is cleared or not */
                    if(ocr_value & CARD_BUSY_BIT)
                    {
                        if((ocr_value & MMC_OCR_HC_RES) == MMC_OCR_HC_RES)
                        {  
                            bHighDensityCard = TRUE; 
                            OALMSG(OAL_INFO, (_T("SD High Density card\r\n")));
                        }
                        else if((ocr_value & MMC_OCR_LC_RES) == MMC_OCR_LC_RES)
                        {
                            bHighDensityCard = FALSE; 
                            OALMSG(OAL_INFO, (_T("SD Low Density card\r\n")));
                        }
                        voltage_validation = ESDHC_STATUS_PASS;
                    }
                }
            }
        }
    }

    OALMSG(OAL_FUNC, (_T("- SD_VoltageValidation\r\n")));
    
    return voltage_validation;
}


//------------------------------------------------------------------------------
//
// Function: SDGetBitMode
//
//    This function will read SD SCR register information and determine the 
//  bus width mode supported by the SD card.
//
// Parameters:
//        None
//
// Returns:
//    SD_INTERFACE_SD_MMC_1BIT for 1 bit/SD_INTERFACE_SD_4BIT for 4 bit mode
//
//------------------------------------------------------------------------------
static SD_INTERFACE_MODE SDGetBitMode (void)
{
    SD_BUS_REQUEST sdRequest;
    PSD_COMMAND_RESPONSE response = &sdRequest.CommandResponse;
    
    UCHAR rd_data_buff[512];
    UINT32 val, ii;
    
    OALMSG(OAL_FUNC, (_T("+ SDGetBitMode\r\n")));

    if (SDMMC_card_send_appcmd () == ESDHC_STATUS_PASS)
    {
        /* Read Command response */
        response->ResponseType = ResponseR1;
        SDHCReadResponse (response);
        memcpy((&val), &response->ResponseBuffer[1], sizeof(UINT32));                

        /* After giving ACMD Command, the R1 response should have
         * STATUS_APP_CMD set
         */
        if(val & SD_R1_STATUS_APP_CMD_MSK)
        {
            /* Configure ACMD51 for SD card */
            SDCard_command_config(&sdRequest, SD_ACMD_SEND_SCR, 0 , SD_READ, ResponseR1);
            sdRequest.BlockSize = SDHC_BLK_LEN;
            sdRequest.NumBlocks = 1;
        
            /* Issue ACMD51 to SD Memory card */
            if(SDHCSendCmdWaitResp(&sdRequest) != ESDHC_STATUS_FAILURE)
            {
                /* Read Response from e-SDHC buffer */
                SDHCDataRead ((UINT32 *)rd_data_buff, SDHC_BLK_LEN);
                for (ii = 0 ; ii < sizeof(sdCardReg.SCR); ii++) 
                {
                    sdCardReg.SCR[ii] = (UCHAR)rd_data_buff[(SD_SCR_REGISTER_SIZE - 1) - ii];
                }

                /* Check for bus width supported */
                if (sdCardReg.SCR[SD_BUS_WIDTH_OFFSET] & SD_SCR_BUS_WIDTH_4_BIT)
                {
                    return SD_INTERFACE_SD_4BIT;
                }
                
            }
        }
    }

    OALMSG(OAL_FUNC, (_T("- SDGetBitMode\r\n")));
    
    return SD_INTERFACE_SD_MMC_1BIT;
}


//------------------------------------------------------------------------------
//
// Function: SDSetBusWidth
//
//    This function will set bus width of SD Memory Card.
//
// Parameters:
//        mode[in] - SD_INTERFACE_SD_MMC_1BIT/SD_INTERFACE_SD_4BIT
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
static UINT32 SDSetBusWidth (SD_INTERFACE_MODE mode)
{
    SD_BUS_REQUEST sdRequest;
    PSD_COMMAND_RESPONSE response = &sdRequest.CommandResponse;
    UINT32 set_bus_width_status = ESDHC_STATUS_FAILURE;
    UINT32 val;

    OALMSG(OAL_FUNC, (_T("+ SDSetBusWidth\r\n")));
    
    /* Configure CMD55 for SD card */
    /* This command expects RCA as argument.*/
    if (SDMMC_card_send_appcmd () == ESDHC_STATUS_PASS)
    {
        /* Read Command response */
        response->ResponseType = ResponseR1;
        SDHCReadResponse (response);
        memcpy((&val), &response->ResponseBuffer[1], sizeof(UINT32));
            
        /* Afetr giving ACMD Command, the R1 response should have
         * STATUS_APP_CMD set
         */
        if(val & SD_R1_STATUS_APP_CMD_MSK)
        {                             
            /* Configure ACMD6 for SD card */
            /* This command expects argument 00 for 1 bit mode support.*/
            /* 10 for 4 bit mode support*/
            if (mode == SD_INTERFACE_SD_4BIT)
                val = 2;
            else
                val = 0;

            SDCard_command_config(&sdRequest, SD_ACMD_SET_BUS_WIDTH, val, SD_COMMAND, ResponseR1);
            
            /* Issue ACMD6 to SD Memory card*/
            if(SDHCSendCmdWaitResp(&sdRequest) == ESDHC_STATUS_PASS)
            {
                set_bus_width_status = ESDHC_STATUS_PASS;
            }
        }
    }
    
    OALMSG(OAL_FUNC, (_T("- SDSetBusWidth\r\n")));

    return set_bus_width_status;    
}


