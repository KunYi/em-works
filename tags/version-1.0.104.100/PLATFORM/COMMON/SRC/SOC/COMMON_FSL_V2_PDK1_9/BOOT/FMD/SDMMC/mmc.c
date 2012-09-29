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
extern SDCARD_CARD_REGISTERS    sdCardReg;
extern CHAR                     extCSDBuffer[SDHC_BLK_LEN];
extern USHORT                   Card_rca;
extern BOOL                     bHighDensityCard;
extern UINT32                   mmcSpecVersion;


//-----------------------------------------------------------------------------
// Defines
#define MMC_SPEC_VER_MASK       0x3C
#define MMC_SPEC_VER_SHIFT      2

#define BUS_SIZE_SHIFT          2
#define BUS_WIDTH               0x03B70000

#define MMC_READ_BL_LEN         0xF0
#define MMC_READ_BL_LEN_SHIFT   4
#define READ_BL_LEN_512         9


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions
static UINT32 MMC_GetSpecVersion(void);
static UINT32 MMC_SetBusWidth(UINT32, UINT32);
static UINT32 MMC_VoltageValidation(void);
UINT32 MMC_Send_Switch_Cmd(UINT32 switch_arg);



//------------------------------------------------------------------------------
//
// Function: MMC_Init
//
//      This function will initialize the MMC memory card.
//
// Parameters:
//      None
//
// Returns:
//      ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 MMC_Init(void)
{
    UINT32 status = ESDHC_STATUS_FAILURE;
    UINT32 spec_version = 0;
    DWORD dwClockRate = MMC_FULL_SPEED_RATE;
    
    OALMSG(OAL_FUNC, (_T("+ MMC_Init\r\n")));

    SDMMC_card_software_reset();
    
    if(MMC_VoltageValidation()) // Not a MMC card
        return status;

    /* Get CID number of MMC Card */
    if(!SDMMC_get_cid())
    {
        /* Set RCA of the MMC Card */
        if(!SDMMC_send_rca(TRUE))
        {
            if(!SDMMC_get_csd())
            {
                /* Get Spec version supported by the card */
                spec_version = MMC_GetSpecVersion();
            }

            /*Enable operating frequency */
            SetClockRate (&dwClockRate);

            /*Put MMC in Transfer State */
            if(!SDMMC_set_data_transfer_mode ())
            {
                if(!SDMMC_set_blocklen(SDHC_BLK_LEN))
                {

                    if (SDMMC_get_ext_csd() == ESDHC_STATUS_FAILURE)
                    {
                        OALMSG(OAL_INFO, (L"MMC_Init: Unable to read EXT_CSD\r\n"));
                    }
                    else
                    {
                        if((BSP_MMC4BitSupported()) && (!MMC_SetBusWidth(spec_version, 4))) //MMC_SPEC_VER_4_0
                        {
                            SDController.BusWidthSetting = SD_INTERFACE_SD_4BIT; // 4 Bit Mode
                            OALMSG(OAL_INFO, (_T("MMC: Switched to 4 bit mode\r\n")));
                        }
                        switch(extCSDBuffer[MMC_EXT_CSD_BOOT_CONFIG])
                        {
                        // enable flashing to the boot part 1 that is already enabled for boot
                        case 0x8:  
                        case 0x48:
                            MMC_Send_Switch_Cmd(EMMC_SWITCH_WRITE_BOOT_PART | (0x49 << EMMC_SWITCH_BOOT_VALUE_SHIFT));
                            SDMMC_get_ext_csd();
                            break;

                        // enable flashing to the boot part 2 that is already enabled for boot
                        case 0x10:
                        case 0x50:
                            MMC_Send_Switch_Cmd(EMMC_SWITCH_WRITE_BOOT_PART | (0x52 << EMMC_SWITCH_BOOT_VALUE_SHIFT));
                            SDMMC_get_ext_csd();
                            break;                                
                        
                        case 0x49:
                        case 0x52:
                        default:
                            // do nothing because part 1 or part 2 is already enabled, or boot parts are disabled
                            break;        
                        }
                    }
                    
                    /* Set status variable as SUCESS */
                    status = ESDHC_STATUS_PASS; 
                }
            }
        }
    }
 
    OALMSG(OAL_FUNC, (_T("- MMC_Init\r\n")));

    return status;
}


//------------------------------------------------------------------------------
//
// Function: MMC_VoltageValidation
//
//    This function will validate the operating voltage range of MMCMemory card.
//
// Parameters:
//        None
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
static UINT32 MMC_VoltageValidation (void)
{
    SD_BUS_REQUEST sdRequest;
    PSD_COMMAND_RESPONSE response = &sdRequest.CommandResponse;
    UINT32 ocr_val = 0;
    UINT32 resp = 0;
    UINT32 voltage_validation = ESDHC_STATUS_FAILURE;
    UINT32 StartTime = OEMEthGetSecs();
    
    OALMSG(OAL_FUNC, (_T("+ MMC_VoltageValidation\r\n")));

    ocr_val = (UINT32)((MMC_OCR_VALUE) & 0xFFFFFFFF);
    
    while((OEMEthGetSecs() - StartTime < ESDHC_DELAY_TIMEOUT) && (voltage_validation != ESDHC_STATUS_PASS)) 
    {      
        /* Configure CMD1 for MMC card */
        /* Argument will be expected OCR value */
        SDCard_command_config(&sdRequest, SD_CMD_MMC_SEND_OPCOND, ocr_val, SD_COMMAND, ResponseR3);
                 
        /* Issue CMD1 to MMC card to determine OCR value */
        if(SDHCSendCmdWaitResp(&sdRequest) != ESDHC_STATUS_FAILURE)
        {
            /* Read Response */
            response->ResponseType = ResponseR3;
            SDHCReadResponse(response);
            memcpy((&resp), &response->ResponseBuffer[1], sizeof(UINT32));    
            
            /* Check if card busy bit is cleared or not */
            if(resp & CARD_BUSY_BIT)
            {
                if((resp & MMC_OCR_HC_RES) == MMC_OCR_HC_RES)
                {  
                    bHighDensityCard = TRUE; 
                    OALMSG(OAL_INFO, (_T("MMC High Density card\r\n")));
                }
                else if((resp & MMC_OCR_LC_RES) == MMC_OCR_LC_RES)
                {
                    bHighDensityCard = FALSE; 
                    OALMSG(OAL_INFO, (_T("MMC Low Density card\r\n")));
                }
                voltage_validation = ESDHC_STATUS_PASS;
            }
        }
        else
        {
            // Failure to respond to CMD1 means it is not a valid MMC card
            break;
        }
    }
    
    OALMSG(OAL_FUNC, (_T("- MMC_VoltageValidation\r\n")));

    return voltage_validation;
}


//------------------------------------------------------------------------------
//
// Function: MMC_GetSpecVersion
//
//    This function will return the MMC Spec version.
//
// Parameters:
//        None
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
static UINT32 MMC_GetSpecVersion(void)
{
    mmcSpecVersion = ((sdCardReg.CSD[15] & MMC_SPEC_VER_MASK) >> MMC_SPEC_VER_SHIFT);
    OALMSG(OAL_VERBOSE, (_T("+ MMC Spec Version %x\r\n"), mmcSpecVersion));
    
    return mmcSpecVersion;        
}


//------------------------------------------------------------------------------
//
// Function: MMC_SetBusWidth
//
//    This function will set bus width of MMC Card.
//
// Parameters:
//        spec_version[in] - MMC Specification version
//        bus_width[in] - Bus Width to be set
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
static UINT32 MMC_SetBusWidth (UINT32 spec_version,UINT32 bus_width)
{
    UINT32 bus_size;
    UINT32 set_bus_width_status = ESDHC_STATUS_FAILURE;

    OALMSG(OAL_FUNC, (_T("+ MMC_SetBusWidth\r\n")));
    
    if (((bus_width == 4 || bus_width == 8) && (spec_version >= MMC_SPEC_VER_4_0 )))
    {
        bus_size = (bus_width >> BUS_SIZE_SHIFT);
        set_bus_width_status = MMC_Send_Switch_Cmd((BUS_WIDTH)|(bus_size<<8));
    }
    
    OALMSG(OAL_FUNC, (_T("- MMC_SetBusWidth\r\n")));

    return set_bus_width_status;
}


//------------------------------------------------------------------------------
//
// Function: MMC_Send_Switch_Cmd
//
//    This function will send CMD6 with appropriate argument to modify EXT_CSD
//
// Parameters:
//        switch_arg[in] - Argument to CMD6
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 MMC_Send_Switch_Cmd(UINT32 switch_arg)
{
    SD_BUS_REQUEST sdRequest;
    UINT32 status = ESDHC_STATUS_FAILURE;

    /* Configure CMD6 to write to EXT_CSD register for BUS_WIDTH */
    SDCard_command_config(&sdRequest, SD_CMD_SWITCH_FUNCTION,
                        switch_arg, SD_COMMAND, ResponseR1b);        
    
    /* Sending the card from stand-by to transfer state
    */
    if(!SDHCSendCmdWaitResp(&sdRequest))
    {
        if (!SDMMC_R1b_busy_wait())
        {
            status = ESDHC_STATUS_PASS;
        }
    }

    return status;
}


