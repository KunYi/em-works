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


//-----------------------------------------------------------------------------
// Defines
#define MMC_CMD_READ_EXTENDED_CSD (8)


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
SD_IMAGE_CFG            SDImageCfg;
SDCARD_CARD_REGISTERS   sdCardReg;                      // the card registers
CHAR                    extCSDBuffer[SDHC_BLK_LEN];     // extended CSD register
USHORT                  Card_rca = 0x0;                 // Relative Card Address
BOOL                    bHighDensityCard = FALSE;
UINT32                  mmcSpecVersion = 0;


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function: SDMMC_Init
//
//  This function initializes the SD interface.
//
//  Parameters:
//        None.
//
//  Returns:
//        None.
//
//-----------------------------------------------------------------------------
BOOL SDMMC_Init(void)
{  
    BOOL bInit = FALSE;
    
    OALMSG(OAL_FUNC, (_T("+ SDMMC_Init\r\n")));

    SDConfigPins();

    if(!SDInterface_Init())
    {
        OALMSG(OAL_VERBOSE, (_T("SDInterface_Init Failed\r\n")));
        return bInit;
    }

    SDMMC_card_software_reset();

    if(!SD_Init())
    {
        SDController.IsMMC = FALSE;
        OALMSG(OAL_INFO, (_T("INFO: Initialized SD Card\r\n")));
        bInit = TRUE;
    }
    else if(!MMC_Init())
    {
        SDController.IsMMC = TRUE;
        OALMSG(OAL_INFO, (_T("INFO: Initialized MMC Card\r\n")));
        bInit = TRUE;
    }

    // Get the Image Configuration
    memset(&SDImageCfg, 0, sizeof(SD_IMAGE_CFG));
    BSP_GetSDImageCfg(&SDImageCfg);
    
    OALMSG(OAL_FUNC, (_T("- SDMMC_Init\r\n")));

    return bInit;
}


//-----------------------------------------------------------------------------
//
//  Function: SDMMC_Deinit
//
//  This function de-initializes the flash chip.
//
//  Parameters:
//        None.
//
//  Returns:  
//        None.
//
//-----------------------------------------------------------------------------
BOOL SDMMC_Deinit (void)
{
    return(TRUE);
}


//-----------------------------------------------------------------------------
//
//  Function: SDMMC_get_cid
//
//  This function will return the CID value of card.
//
//  Parameters:
//        None.
//
//  Returns:
//        ESDHC_STATUS_PASS or ESDHC_STATUS_FAILURE
//
//-----------------------------------------------------------------------------
UINT32 SDMMC_get_cid (void)
{
    SD_BUS_REQUEST sdRequest;
    UINT32 cid_request = ESDHC_STATUS_FAILURE;
    PSD_COMMAND_RESPONSE response = &sdRequest.CommandResponse;
    
    OALMSG(OAL_FUNC, (_T("+ SDMMC_get_cid\r\n")));

    /* Configure CMD2 for card */
    /* No Argument is expected for CMD2 */
    SDCard_command_config(&sdRequest, SD_CMD_ALL_SEND_CID, 0, SD_COMMAND, ResponseR2);

    /* Issue CMD2 to card to determine CID contents */
    if(!SDHCSendCmdWaitResp(&sdRequest))
    {
        /* Read Command response  */
        response->ResponseType = ResponseR2;
        SDHCReadResponse (response);
        
        memcpy(sdCardReg.CID, response->ResponseBuffer, SD_CID_REGISTER_SIZE);        
        
        /* Assign cid_request as SUCCESS */
        cid_request = ESDHC_STATUS_PASS;
    }
    
    OALMSG(OAL_FUNC, (_T("- SDMMC_get_cid\r\n")));

    return cid_request;
}


//-----------------------------------------------------------------------------
//
//  Function: SDMMC_set_data_transfer_mode
//
//  This function will put card in transfer mode.
//
//  Parameters:
//        None.
//
//  Returns:
//        ESDHC_STATUS_PASS or ESDHC_STATUS_FAILURE
//
//-----------------------------------------------------------------------------
UINT32 SDMMC_set_data_transfer_mode (void)
{
    SD_BUS_REQUEST sdRequest;
    UINT32 transfer_status =ESDHC_STATUS_FAILURE;
    UINT32 card_address = (Card_rca << RCA_SHIFT);
    
    OALMSG(OAL_FUNC , (_T("+ SDMMC_set_data_transfer_mode\r\n")));

    /* Configure CMD7 for MMC card */
    /* 16bit card address is expected as Argument */
    SDCard_command_config(&sdRequest,SD_CMD_SELECT_DESELECT_CARD, card_address, 
                            SD_COMMAND, ResponseR1b);
    
    /* Sending the card from stand-by to transfer state
    */
    if(!SDHCSendCmdWaitResp(&sdRequest))
    {
        if (!SDMMC_R1b_busy_wait())
        {
            transfer_status = ESDHC_STATUS_PASS;
        }
    }
    
    OALMSG(OAL_FUNC , (_T("- SDMMC_set_data_transfer_mode\r\n")));

    return transfer_status;
    
}


//-----------------------------------------------------------------------------
//
//  Function: SDMMC_card_software_reset
//
//  This function will issue CMD0 to card. This gives software reset to card.
//
//  Parameters:
//        None.
//
//  Returns:
//        ESDHC_STATUS_PASS or ESDHC_STATUS_FAILURE
//
//-----------------------------------------------------------------------------
UINT32 SDMMC_card_software_reset(void)
{
    SD_BUS_REQUEST sdRequest;
    UINT32 response = ESDHC_STATUS_FAILURE;

    OALMSG(OAL_FUNC, (_T("+ SDMMC_card_software_reset\r\n")));
    
    /*Configure CMD0 for MMC/SD card*/
    /*CMD0 doesnt expect any response */
    SDCard_command_config(&sdRequest, SD_CMD_GO_IDLE_STATE, 0, SD_COMMAND, NoResponse);
    
    /*Issue CMD0 to MMC/SD card to put in active state */
    response = SDHCSendCmdWaitResp(&sdRequest);
    
    OALMSG(OAL_FUNC, (_T("- SDMMC_card_software_reset\r\n")));

    return response;
}


//------------------------------------------------------------------------------
//
// Function: SDMMC_card_send_appcmd
//
//    This function will issue an APP CMD (CMD55) to the card
//
// Parameters:
//        None
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------ 
UINT32 SDMMC_card_send_appcmd (void)
{
    SD_BUS_REQUEST sdRequest;
    UINT32 response = ESDHC_STATUS_FAILURE;
    UINT32 card_address = (Card_rca << RCA_SHIFT);
    
    OALMSG(OAL_FUNC, (_T("+ SDMMC_card_send_appcmd\r\n")));

    /*Configure CMD55 for MMC/SD card to be followed by an ACMD */
    SDCard_command_config (&sdRequest,SD_CMD_APP_CMD, card_address, SD_COMMAND, 
                            ResponseR1);
    
    /*Issue CMD55 to MMC/SD card */
    if(SDHCSendCmdWaitResp(&sdRequest) != ESDHC_STATUS_FAILURE)
    {
        response = ESDHC_STATUS_PASS;
    }

    OALMSG(OAL_FUNC, (_T("- SDMMC_card_send_appcmd\r\n")));

    return response;
}


//------------------------------------------------------------------------------
//
// Function: SDMMC_get_csd
//
//    This function will will read CSD register
//
// Parameters:
//        None
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------ 
UINT32  SDMMC_get_csd(void)
{
    SD_BUS_REQUEST sdRequest;
    PSD_COMMAND_RESPONSE response = &sdRequest.CommandResponse;
    UINT32 card_address = (Card_rca << RCA_SHIFT);    
    UINT32 csd_request=ESDHC_STATUS_FAILURE;    
    
    OALMSG(OAL_FUNC, (_T("+ SDMMC_get_csd\r\n")));

    /* Configure CMD9 for MMC card */
    /* 16bit card address is expected as Argument */
    SDCard_command_config(&sdRequest, SD_CMD_SEND_CSD, card_address, SD_COMMAND, ResponseR2);

    /* Issue Command CMD9 to Extrace CSD register contents
     */     
    if(SDHCSendCmdWaitResp(&sdRequest) != ESDHC_STATUS_FAILURE)
    {
        /* Read Command response */
        response->ResponseType = ResponseR2;
        SDHCReadResponse (response);
        
        /* Assign Response to CSD Strcuture */
        memcpy(sdCardReg.CSD, response->ResponseBuffer, SD_CSD_REGISTER_SIZE);
        csd_request = ESDHC_STATUS_PASS;
    }
    
    OALMSG(OAL_FUNC, (_T("- SDMMC_get_csd\r\n")));

    return csd_request;      
}


//------------------------------------------------------------------------------
//
// Function: SDMMC_get_ext_csd
//
//    This function will will read the extended-CSD register in an MMC card
//
// Parameters:
//        None
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------ 
UINT32 SDMMC_get_ext_csd(void)
{
    SD_BUS_REQUEST sdRequest;
    PSD_COMMAND_RESPONSE response = &sdRequest.CommandResponse;
    UINT32 ext_csd_request=ESDHC_STATUS_FAILURE;    
    
    OALMSG(OAL_FUNC, (_T("+ SDMMC_get_ext_csd\r\n")));

    /* Configure CMD8 for MMC card */
    SDCard_command_config(&sdRequest, MMC_CMD_READ_EXTENDED_CSD, 0x00000000, SD_READ, ResponseR1);

    // Ext CSD is 1 block of 512 bytes
    sdRequest.NumBlocks = 1;
    sdRequest.BlockSize = SDHC_BLK_LEN; 

    /* Issue Command CMD8 to Extrace EXT_CSD register contents
     */     
    if(SDHCSendCmdWaitResp(&sdRequest) != ESDHC_STATUS_FAILURE)
    {
        /* Read Command response */
        response->ResponseType = ResponseR1;
        SDHCReadResponse (response);

        if (SDHCDataRead((UINT32 *)&extCSDBuffer[0],SDHC_BLK_LEN) != ESDHC_STATUS_FAILURE)
        {
            ext_csd_request = ESDHC_STATUS_PASS;
        }
    }
 
    OALMSG(OAL_FUNC, (_T("- SDMMC_get_ext_csd\r\n")));

    return ext_csd_request;        
    
}


//------------------------------------------------------------------------------
//
// Function: SDMMC_GetCardCapacity
//
//    This function will will read SD/MMC card capacity
//
// Parameters:
//        None
//
// Returns:
//        Card capacity in number of sectors(each sector of ESDHC_SECTOR_SIZE)
//
//------------------------------------------------------------------------------ 
DWORD SDMMC_GetCardCapacity (void)
{
    UINT32 uCsize;
    USHORT uCmult;
    UINT32 uBlkLen;
    DWORD  dwCapacity = 0, dwTotalNumOfSectors = 0;

    if (bHighDensityCard)
    {
        if (SDController.IsMMC == TRUE)
        {
            if (SDMMC_get_ext_csd() == ESDHC_STATUS_PASS)
            {
                /* Extract Bytes 212-215 to get the high density card capacity in number of sectors */
                dwTotalNumOfSectors = (*(UINT32 *)&extCSDBuffer[212]);
                dwCapacity = dwTotalNumOfSectors >> 1; //(dwTotalNumOfSectors * ESDHC_SECTOR_SIZE) >> 10 = numsec * 2^9/2^10 = numsec  >> 1;
            }
            else
            {
                OALMSG(OAL_INFO, (_T("ERROR: Unable to read EXT_CSD from MMC card\r\n")));
            }
        }   
        else
        {
            /* Extract Bits 48-69 to get the C_SIZE */
            memcpy (&uCsize, &(sdCardReg.CSD[6]), sizeof (UINT32));
            uCsize = (uCsize & 0x3FFFFF);
            
            /* Memory Capacity = (C_SIZE + 1) * 512K byte */
            dwCapacity = (uCsize + 1) * ESDHC_SECTOR_SIZE;
            dwTotalNumOfSectors = dwCapacity*2;
        }
    }
    else
    {    
        /* Extract Bits 62-73 to get the C_SIZE */
        memcpy (&uCsize, &(sdCardReg.CSD[7]), sizeof (UINT32));
        uCsize = ((uCsize & 0x3FFC0) >> 6);
        
        /* Extract Bits 47-49 to get the C_MULT */
        memcpy (&uCmult, &(sdCardReg.CSD[5]), sizeof (USHORT));
        uCmult = ((uCmult & 0x380) >> 7);
        
        /* Extract Bits 80-83 to get the READ_BL_LEN */
        uBlkLen = 1 << (sdCardReg.CSD[10] & 0x0F);

        /* Memory Capacity calculation:
         * capacity = BLOCKNR * BLOCK_LEN
         * MULT = 2^(C_MULT + 2)
         * BLOCKNR = (C_SIZE + 1) * MULT
         * BLOCKLEN = 2^READ_BL_LEN
         */
        dwTotalNumOfSectors = (((uCsize + 1) << (uCmult + 2)) * uBlkLen)/ESDHC_SECTOR_SIZE;
        dwCapacity = dwTotalNumOfSectors >> 1;  // (numSectors * 512 / 1024) = KBytes
    }

    OALMSG(OAL_INFO, (_T("Card Capacity %d (Kbytes)\r\n"), dwCapacity));
    
    return (dwTotalNumOfSectors);
}


//------------------------------------------------------------------------------
//
// Function: SDMMC_set_blocklen
//
//    This function will will set the maximum read/write Block Length
//
// Parameters:
//        None
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------ 
UINT32 SDMMC_set_blocklen (UINT32 BlockLen)
{
    SD_BUS_REQUEST sdRequest;
    UINT32 status = ESDHC_STATUS_FAILURE;

    OALMSG(OAL_FUNC, (_T("+ SDMMC_set_blocklen\r\n")));

    if(BlockLen < SDHC_BLK_LEN)
    {
        BlockLen = SDHC_BLK_LEN;
    }
    
    /* Configure CMD16 to set block length as 512 bytes.*/
    SDCard_command_config(&sdRequest, SD_CMD_SET_BLOCKLEN, BlockLen, SD_COMMAND, ResponseR1);

    if(SDHCSendCmdWaitResp(&sdRequest) != ESDHC_STATUS_FAILURE)
    {
        status = ESDHC_STATUS_PASS;
    }
    
    OALMSG(OAL_FUNC, (_T("- SDMMC_set_blocklen\r\n")));

    return status;        
}


//------------------------------------------------------------------------------
//
// Function: SDMMC_send_rca
//
//    This function will send/set RCA for SD/MMC card..
//
// Parameters:
//        isMMC[in] - TRUE for MMC else FALSE for SD card
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------ 
UINT32 SDMMC_send_rca (BOOL isMMC)
{
    SD_BUS_REQUEST sdRequest;
    SD_CARD_STATUS CardStatus=0;
    UINT32 rca_request = ESDHC_STATUS_FAILURE;
    PSD_COMMAND_RESPONSE response = &sdRequest.CommandResponse;
    UINT32 card_address = 0;

    OALMSG(OAL_FUNC, (_T("+ SDMMC_send_rca\r\n")));
    if (isMMC)
    {
        Card_rca = 0x1;
        card_address = (Card_rca << RCA_SHIFT);    
    }
    
    /* Configure CMD3 for MMC card */
    /* 32bit card address is expected as Argument */
    SDCard_command_config(&sdRequest, SD_CMD_MMC_SET_RCA, card_address, SD_COMMAND, ResponseR1);
                                          
    /* Assigns relative address to the card
    */
    if(SDHCSendCmdWaitResp(&sdRequest) != ESDHC_STATUS_FAILURE)
    {
        /* Read Command response */
        response->ResponseType = ResponseR1;
        SDHCReadResponse (response);
        if (!isMMC)
        {
            // RCA is in bytes 3,4
            Card_rca = (SD_CARD_RCA)response->ResponseBuffer[3];
            Card_rca |= ((SD_CARD_RCA)response->ResponseBuffer[4]) << 8;
        }
        memcpy((&CardStatus), &response->ResponseBuffer[1], sizeof(SD_CARD_STATUS));
        CardStatus = SD_STATUS_CURRENT_STATE(CardStatus);
        
        if(CardStatus == SD_STATUS_CURRENT_STATE_IDENT)
            rca_request = ESDHC_STATUS_PASS;
    }
        
    OALMSG(OAL_FUNC, (_T("- SDMMC_send_rca\r\n")));

    return rca_request;
}


//------------------------------------------------------------------------------
//
// Function: SDMMC_R1b_busy_wait
//
//    This function will wait during a command with R1b response type till the
//    SD state changes back to normal
//
// Parameters:
//        isMMC[in] - TRUE for MMC else FALSE for SD card
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------ 
UINT32 SDMMC_R1b_busy_wait ()
{
    SD_BUS_REQUEST sdRequest;
    PSD_COMMAND_RESPONSE pResp = &sdRequest.CommandResponse;
    SD_CARD_STATUS CardStatus=0;
    UINT32 wait_request= ESDHC_STATUS_FAILURE;
    UINT32 card_address = (Card_rca << RCA_SHIFT);    
    SD_CARD_STATUS  CardState = SD_STATUS_CURRENT_STATE_TRAN;
    UINT32 StartTime = OEMEthGetSecs();

    OALMSG(OAL_FUNC, (_T("+ SDMMC_R1b_busy_wait\r\n")));

    do
    {
        /* Configure CMD13 to read status of the card becuase previous CMD has R1b response */
        SDCard_command_config(&sdRequest, SD_CMD_SEND_STATUS, card_address, SD_COMMAND ,ResponseR1);
        
        if(!SDHCSendCmdWaitResp(&sdRequest))
        {
            /* Read Command response */
            pResp->ResponseType = ResponseR1;
            SDHCReadResponse (pResp);
            memcpy((&CardStatus), &pResp->ResponseBuffer[1], sizeof(SD_CARD_STATUS));
        }
    } while ((SD_STATUS_CURRENT_STATE (CardStatus) != CardState) && 
              (OEMEthGetSecs() - StartTime < ESDHC_DELAY_TIMEOUT));

    if(SD_STATUS_CURRENT_STATE (CardStatus) == CardState)
        wait_request = ESDHC_STATUS_PASS;
    else
        OALMSG(OAL_INFO, (_T("R1b_busy_wait timed out State - %d\r\n"), SD_STATUS_CURRENT_STATE (CardStatus)));

    OALMSG(OAL_FUNC, (_T("- SDMMC_R1b_busy_wait\r\n")));
        
    return wait_request;
}


//------------------------------------------------------------------------------
//
// Function: SDCard_command_config
//
//    This function will configure the command paramteres for card.
//
// Parameters:
//        pReq[out] - structure required to configure and issue commands
//        CommandCode[in] - Command code to issue
//        argument[in] - argument to be passed for a particular command
//        transfer[in] - Transfer type of the command
//        respType[in] - Response type of the command
//
// Returns:
//        None
//
//------------------------------------------------------------------------------ 
void SDCard_command_config (PSD_BUS_REQUEST pReq,UCHAR CommandCode, DWORD argument,
                            SD_TRANSFER_CLASS transfer,SD_RESPONSE_TYPE respType)
{
    /* Configure Command index */    
    pReq->CommandCode = CommandCode;
    
    /* Configure Command Argument */
    pReq->CommandArgument = argument;
    
    /* Configure Data transfer type */
    pReq->TransferClass = transfer;
    
    /* Configure Response Format */
    pReq->CommandResponse.ResponseType = respType;
}


//-----------------------------------------------------------------------------
//
//  Function: SDMMC_GetInfo
//
//  This function determines the size characteristics for the SD memory 
//
//  Parameters:
//      pFlashInfo 
//          [out] A pointer to a structure that contains the size 
//          characteristics for the flash memory device.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL SDMMC_GetInfo(PFlashInfo pFlashInfo)
{
    if (!pFlashInfo)
        return(FALSE);

    pFlashInfo->flashType           = (FLASH_TYPE)SDMMC;
    pFlashInfo->wDataBytesPerSector = ESDHC_SECTOR_SIZE;
    pFlashInfo->dwNumBlocks         = ESDHC_BLOCK_CNT;
    pFlashInfo->wSectorsPerBlock    = ESDHC_SECTOR_CNT;
    pFlashInfo->dwBytesPerBlock     = (pFlashInfo->wSectorsPerBlock * pFlashInfo->wDataBytesPerSector);

    return(TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SDMMC_EraseBlock
//
// Function Erases the specified flash block. 
//
// Parameters:
//      blockID 
//          [in] block number to erase.
//
// Returns:  
//      TRUE/FALSE for success
//
//-----------------------------------------------------------------------------
BOOL SDMMC_EraseBlock(BLOCK_ID blockID, UINT32 num_blocks)
{
    SD_BUS_REQUEST sdRequest;
    BOOL erase_request = FALSE;    
    DWORD startErase = blockID, endErase = blockID + num_blocks - 1;
    UCHAR startEraseBlockCmd = SD_CMD_ERASE_WR_BLK_START;
    UCHAR endEraseBlockCmd = SD_CMD_ERASE_WR_BLK_END;
    
    OALMSG(OAL_FUNC, (_T("- SDMMC_EraseBlock\r\n")));

    // if card is write protected, return with failure
    if(SDHC_IsCardWriteProtected())
    {
        OALMSG(1, (L"ERROR: card is write-protected\r\n"));
        return FALSE;
    }

    if(!bHighDensityCard)
    {
        // convert sector numbers to byte addresses
        startErase = startErase * SDHC_BLK_LEN;
        endErase  = endErase * SDHC_BLK_LEN;
    }

    if((SDController.IsMMC) && (mmcSpecVersion >= MMC_SPEC_VER_4_0))
    {
        startEraseBlockCmd = MMC_CMD_ERASE_WR_BLK_START;
        endEraseBlockCmd = MMC_CMD_ERASE_WR_BLK_END;
    }

    /* Configure CMD32 to select the first block to erase */
    SDCard_command_config(&sdRequest, startEraseBlockCmd, startErase,
                            SD_COMMAND, ResponseR1);

    if(SDHCSendCmdWaitResp(&sdRequest) != ESDHC_STATUS_FAILURE)
    {
        /* Configure CMD33 to select the ending block to erase */
        SDCard_command_config(&sdRequest, endEraseBlockCmd, endErase,
                            SD_COMMAND, ResponseR1);

        if(SDHCSendCmdWaitResp(&sdRequest) != ESDHC_STATUS_FAILURE)
        {
            /* Configure CMD38 to start to erase */
            SDCard_command_config(&sdRequest, SD_CMD_ERASE, 0, SD_COMMAND, ResponseR1b);
            if(SDHCSendCmdWaitResp(&sdRequest) != ESDHC_STATUS_FAILURE)
            {
                if (!SDMMC_R1b_busy_wait())
                    erase_request=TRUE;
            }
        }
    }
    
    OALMSG(OAL_FUNC, (_T("- SDMMC_EraseBlock\r\n")));

    return erase_request;
}


//-----------------------------------------------------------------------------
//
// Function: SDMMC_ReadSector
//
// Function Reads the requested sector data and metadata from the flash media. 
//
// Parameters:
//      startSectorAddr 
//          [in] starting sector address.
//
//      pSectorBuff 
//          [in] Ptr to  buffer that contains sector data read from flash.
//                 Set to NULL if  data is not needed.
//
//      pSectorInfoBuff 
//          [in] Buffer for an array of sector information structures.
//                 One sector    information entry for every sector to be read. 
//                 Set to NULL if not needed.
//
//      dwNumSectors 
//          [in] Number of sectors to read.
//
// Returns:  
//      TRUE/FALSE for success
//
//-----------------------------------------------------------------------------
BOOL SDMMC_ReadSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff,
                      PSectorInfo pSectorInfoBuff, DWORD dwNumSectors)
{
    SD_BUS_REQUEST sdRequest;
    UINT32 status = ESDHC_STATUS_FAILURE;
    BOOL read_status = FALSE;
    DWORD dwReadArg = startSectorAddr;
    UCHAR cmdCode = SD_CMD_READ_SINGLE_BLOCK;
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pSectorInfoBuff);

    OALMSG(OAL_FUNC, (_T("+ SDMMC_ReadSector\r\n")));

    /* Configure interface block and number of blocks */
    sdRequest.BlockSize = SDHC_BLK_LEN;
    sdRequest.NumBlocks = dwNumSectors;
    
    if(!bHighDensityCard)
    {
        // convert sector number to byte address
        dwReadArg = startSectorAddr * SDHC_BLK_LEN;
    }
        
    if(dwNumSectors > 1)
    {
        cmdCode = SD_CMD_READ_MULTIPLE_BLOCK;
    }

    /* Comfigure command for single/multiple block read */    
    SDCard_command_config(&sdRequest, cmdCode, dwReadArg, SD_READ, ResponseR1);    
         
    if(!SDHCSendCmdWaitResp(&sdRequest))
    {
        /* Call interface Data read function */
        status = SDHCDataRead((UINT32 *)pSectorBuff, dwNumSectors * SDHC_BLK_LEN);

        if((status == ESDHC_STATUS_PASS) && (dwNumSectors > 1))
        {
            // In MX233, For multi-sector reads, we must issue CMD12
            if(SDImageCfg.dwSocId == 233)
            {
                SDCard_command_config(&sdRequest, SD_CMD_STOP_TRANSMISSION, 0, SD_COMMAND, ResponseR1b);
                
                if(!SDHCSendCmdWaitResp(&sdRequest))
                {
                    status = SDMMC_R1b_busy_wait ();
                }
            }
            // for other SOCs, the ESDHC controller will auto issue CMD 12 for multi-block transfer, just poll for busy state
            else
                status = SDMMC_R1b_busy_wait ();
            
        }

        if(status == ESDHC_STATUS_PASS)
        {
            read_status = TRUE;
        }
    }

    OALMSG(OAL_FUNC, (_T("- SDMMC_ReadSector\r\n")));

    return read_status;
}


//-----------------------------------------------------------------------------
//
// Function: SDMMC_WriteSector
//
// Function Writes the requested sector data and metadata to the flash media. 
//
// Parameters:
//      startSectorAddr 
//          [in] starting physical sector address.
//
//      pSectorBuff 
//          [in] Ptr to  buffer that contains sector data to write.
//                 Set to NULL if  data is not needed.
//
//      pSectorInfoBuff 
//          [in] Buffer for an array of sector information structures.
//                 One sector    information entry for every sector to be written.
//                 Set to NULL if not needed.
//
//      dwNumSectors 
//          [in] Number of sectors to write.
//
// Returns:  
//      TRUE/FALSE for success
//
//-----------------------------------------------------------------------------
BOOL SDMMC_WriteSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff,
                       PSectorInfo pSectorInfoBuff, DWORD dwNumSectors)
{
    SD_BUS_REQUEST sdRequest;
    UINT32 status = ESDHC_STATUS_FAILURE;
    BOOL write_status = FALSE;
    DWORD dwWriteArg = startSectorAddr;
    UCHAR cmdCode = SD_CMD_WRITE_BLOCK;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pSectorInfoBuff);

    OALMSG(OAL_FUNC, (_T("+ SDMMC_WriteSector\r\n")));

    // if card is write protected, return with failure
    if(SDHC_IsCardWriteProtected())
    {
        OALMSG(1, (L"ERROR: card is write-protected\r\n"));
        return FALSE;
    }
        
    /* Configure interface block and number of blocks */
    sdRequest.BlockSize = SDHC_BLK_LEN;
    sdRequest.NumBlocks = dwNumSectors;
    
    if(!bHighDensityCard)
    {
        // convert sector number to byte address
        dwWriteArg = startSectorAddr * SDHC_BLK_LEN;
    }
        
    if(dwNumSectors > 1)
    {
        cmdCode = SD_CMD_WRITE_MULTIPLE_BLOCK;
    }
    
    /* Configure command for single/multiple block write */    
    SDCard_command_config(&sdRequest, cmdCode, dwWriteArg, SD_WRITE, ResponseR1);    
 
    if(!SDHCSendCmdWaitResp(&sdRequest))
    { 
        /* Call interface Write read function */
        status = SDHCDataWrite((UINT32 *)pSectorBuff, dwNumSectors * SDHC_BLK_LEN); 
        if((status == ESDHC_STATUS_PASS) && (dwNumSectors > 1))
        {        
            // In MX233, For multi-sector reads, we must issue CMD12
            if(SDImageCfg.dwSocId == 233)
            {
                SDCard_command_config(&sdRequest, SD_CMD_STOP_TRANSMISSION, 0, SD_COMMAND, ResponseR1b);
                
                if(!SDHCSendCmdWaitResp(&sdRequest))
                {
                    status = SDMMC_R1b_busy_wait ();
                }
            }
            // for other SOCs, the ESDHC controller will auto issue CMD 12 for multi-block transfer, just poll for busy state
            else
                status = SDMMC_R1b_busy_wait (); 
        }

        if(status == ESDHC_STATUS_PASS)
        {
            write_status = TRUE;
        }
    }
    OALMSG(OAL_FUNC, (_T("- SDMMC_WriteSector\r\n")));

    return write_status;
}


