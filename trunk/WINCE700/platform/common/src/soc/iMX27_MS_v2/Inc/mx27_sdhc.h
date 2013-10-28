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
//  Copyright (C) 2004,	Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//--------------------------------------------------------------------------
//
//  Header: mx27_sd.h
//
//  Provides definitions forSDHC module based on Freescale MX27 chassis.
//
//------------------------------------------------------------------------------

#ifndef __MX27_SDHC_H
#define __MX27_SDHC_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {

    REG32 STR_STP_CLK;            	// 0x00 Clock control register
    REG32 STATUS	; 				// 0x04 Status register, read only 		
    REG32 CLK_RATE; 		     	// 0x08: Card clock rate register
    REG32 CMD_DAT_CONT;         	// 0x0C: Command data control register
    REG32 RESPONSE_TO;		    	// 0x10: Response Time-out register
    REG32 READ_TO;                	// 0x14: Read time-out register
    REG32 BLK_LEN;                 	// 0x18: Block length register
    REG32 NOB;              		// 0x1C: Number of block register
    REG32 REV_NO; 		        	// 0x20: Revision Number register, rad only
    REG32 INT_CNTR;               	// 0x24: Interrupt control register
    REG32 CMD;                		// 0x28: Command number register
    REG32 ARG;                		// 0x2C: Argument register
    REG32 pad;                      // 0x30: reserved register
    REG32 RES_FIFO;          		// 0x34: Command response FIFO aceess register
    REG32 BUFFER_ACCESS;         	// 0x38: Data buffer access register
    REG32 pad1;       				// 0x3C: reserved register
 //   REG32 REMAINING_NOB;         	// 0x40: Remaining number of blocks register
 //   REG32 REMAINING_BLK_SIZE;     	// 0x44: Remaining block bytes register
  
    } CSP_SDHC_REG, *PCSP_SDHC_REG;

//------------------------------------------------------------------------------
// INTERRUPT PROCESS STATUS
//-------------------------------------------------------------------------------
typedef enum{
    SD_COMMAND_STATUS_UNACTIVE = 0 ,
    SD_COMMAND_STATUS_CMD,
    SD_COMMAND_STATUS_READ,
    SD_COMMAND_STATUS_WRITE,
    }SD_COMMAND_STATUS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define   SDHC_STR_STP_CLK_OFFSET         	(0x0000)
#define   SDHC_STATUS_OFFSET         		(0x0004)
#define   SDHC_CLK_RATE_OFFSET         		(0x0008)
#define   SDHC_CMD_DAT_CONT_OFFSET        	(0x000C)
#define   SDHC_RESPONSE_TO_OFFSET         	(0x0010)
#define   SDHC_READ_TO_OFFSET        		(0x0014)
#define   SDHC_BLK_LEN_OFFSET         		(0x0018)
#define   SDHC_NOB_OFFSET         			(0x001C)
#define   SDHC_REV_NO_OFFSET         		(0x0020)
#define   SDHC_INT_CNTR_OFFSET        		(0x0024)
#define   SDHC_CMD_OFFSET         			(0x0028)
#define   SDHC_ARG_OFFSET        			(0x002C)
#define   SDHC_RES_FIFO_OFFSET        		(0x0034)
#define   SDHC_BUFFER_ACCESS_OFFSET      	(0x0038)
#define   SDHC_REMAINING_NOB_OFFSET        	(0x0040)
#define   SDHC_REMAINING_BLK_SIZE_OFFSET  	(0x0044)

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
//Start Stop clock register
#define SDHC_SSCR_STOP_LSH					0	//Stop  
#define SDHC_SSCR_START_LSH					1	//Start 
#define SDHC_SSCR_RESET_LSH					3	//Reset
#define SDHC_SSCR_PERCLKGATDIS_LSH			14	//Disable perclk gating
#define SDHC_SSCR_CLKGATDIS_LSH				15	//Disable clk gating
//Status register 
#define SDHC_SR_TOREAD_LSH 					0	//Read time out 	
#define SDHC_SR_TORESP_LSH 					1	//Response time out 
#define SDHC_SR_WCERR_LSH 					2	//Write CRC Error
#define SDHC_SR_RCERR_LSH 					3	//read CRC error
#define SDHC_SR_RSPCERR_LSH					5	//Response CRC error
#define SDHC_SR_BWR_LSH 					6	//Buffer write ready 
#define SDHC_SR_BRR_LSH						7	//Buffer read ready
#define SDHC_SR_CBCR_LSH					8	//Card bus clock run
#define SDHC_SR_WCERRCODE_LSH				9	//Write CRC error code
#define SDHC_SR_RODONE_LSH					11 	//Read operation done 
#define SDHC_SR_WODONE_LSH					12	//Write operation done
#define SDHC_SR_ECR_LSH						13 	//End command response 
#define SDHC_SR_SDIOINT_LSH					14	//SDIO interrupt active
#define SDHC_SR_BUFFOVL_LSH 				24	//Buffer overflow
#define SDHC_SR_BUFUNDRUN_LSH				25	//Buffer under run
#define SDHC_SR_XFULL_LSH					26	//X buffer full
#define SDHC_SR_YFULL_LSH					27 	//Y buffer full
#define SDHC_SR_XEMPTY_LSH					28	//X buffer empty
#define SDHC_SR_YEMPTY_LSH					29	//Y buffer empty
#define SDHC_SR_CARDRMV_LSH					30	//Card removal 
#define SDHC_SR_CARDINS_LSH					31 	//Card insertion 
//CLK_RATE register
#define SDHC_CRATE_DIV_LSH					0 	//Divider 
#define SDHC_CRATE_PRES_LSH					4 	//Prescaler
//CMD_DAT_CONT register
#define SDHC_CDC_FORMAT_LSH 				0 	//Format of response 
#define SDHC_CDC_DE_LSH 					3	//Data enable
#define SDHC_CDC_WR_LSH						4 	//Write/read
#define SDHC_CDC_INIT_LSH					7	//Init 80-clock prefix
#define SDHC_CDC_BW_LSH						8	//Bus width 
#define SDHC_CDC_STARTRW_LSH				10	//Start read wait 
#define SDHC_CDC_STOPRW_LSH					11	//Stop read wait 
#define SDHC_CDC_CRLO_LSH					12 	//CMD response long off 
#define SDHC_CDC_RESUME_LSH					15	//CMD resume 
//RESPONSE_TO register
#define SDHC_RESTO_TO_LSH 					0 	//Response time out value 
//READ_TO register
#define SDHC_READTO_TO_LSH 					0 	//Read time out value
//BLK_LEN register 
#define SDHC_BL_BL_LSH						0	//Block length 
//NOB register 
#define SDHC_NOB_NOB_LSH 					0 	//Number of blocks 
//REV_NO register 
#define SDHC_REVNO_REVNO_LSH 				0 	//Revision number 
//INTR_CNTR register 
#define SDHC_INT_RODONE_LSH 				0 	//read operation done 
#define SDHC_INT_WODONE_LSH 				1	//write operation done
#define SDHC_INT_ECR_LSH					2	//end command response  		
#define SDHC_INT_BWE_LSH					3	//Buffer write enable 
#define SDHC_INT_BRE_LSH					4	//Buffer read enable 
#define SDHC_INT_DAT0EN_LSH					12	//Dat0 enable 
#define SDHC_INT_SDIOIRQ_LSH				13	//SDIO IRQ enable 
#define SDHC_INT_CARDRMV_LSH				14	//Card removal enable 
#define SDHC_INT_CARDINS_LSH				15	//Card insertion enable 
#define SDHC_INT_CARDRMVWKP_LSH				16 	//Card removal wake-up enable 
#define SDHC_INT_CARDINSWKP_LSH				17	//Card insertion wake-up enable 
#define SDHC_INT_SDIOIRQWKP_LSH				18	//SDIO IRQ wake-up enable 
//CMD register 
#define SDHC_CMD_CMD_LSH 					0 	//Command number 
//ARG register 
#define SDHC_ARG_ARG_LSH 					0 	//Argument 
//RES_FIFO register 
#define SDHC_RESFIFO_CONTENT_LSH			0 	//Response FIFO content 
//BUFFER_ACCESS register 
#define SDHC_BA_CONTENT_LSH					0 	//Buffer access FIFO content 


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
//Start Stop clock register
#define SDHC_SSCR_STOP_WID					1	//Stop  
#define SDHC_SSCR_START_WID					1	//Start 
#define SDHC_SSCR_RESET_WID					1	//Reset
#define SDHC_SSCR_PERCLKGATDIS_WID			1	//Disable perclk gating
#define SDHC_SSCR_CLKGATDIS_WID				1	//Disable clk gating
//Status register 
#define SDHC_SR_TOREAD_WID 					1	//Read time out 	
#define SDHC_SR_TORESP_WID 					1	//Response time out 
#define SDHC_SR_WCERR_WID 					1	//Write CRC Error
#define SDHC_SR_RCERR_WID 					1	//read CRC error
#define SDHC_SR_RSPCERR_WID					1	//Response CRC error
#define SDHC_SR_BWR_WID 					1	//Buffer write ready 
#define SDHC_SR_BRR_WID						1	//Buffer read ready
#define SDHC_SR_CBCR_WID					1	//Card bus clock run
#define SDHC_SR_WCERRCODE_WID				2	//Write CRC error code
#define SDHC_SR_RODONE_WID					1 	//Read operation done 
#define SDHC_SR_WODONE_WID					1	//Write operation done
#define SDHC_SR_ECR_WID						1 	//End command response 
#define SDHC_SR_SDIOINT_WID					1	//SDIO interrupt active
#define SDHC_SR_BUFFOVL_WID 				1	//Buffer overflow
#define SDHC_SR_BUFUNDRUN_WID				1	//Buffer under run
#define SDHC_SR_XFULL_WID					1	//X buffer full
#define SDHC_SR_YFULL_WID					1 	//Y buffer full
#define SDHC_SR_XEMPTY_WID					1	//X buffer empty
#define SDHC_SR_YEMPTY_WID					1	//Y buffer empty
#define SDHC_SR_CARDRMV_WID					1	//Card removal 
#define SDHC_SR_CARDINS_WID					1 	//Card insertion 
//CLK_RATE register
#define SDHC_CRATE_DIV_WID					4 	//Divider 
#define SDHC_CRATE_PRES_WID					12 	//Prescaler
//CMD_DAT_CONT register
#define SDHC_CDC_FORMAT_WID 				3 	//Format of response 
#define SDHC_CDC_DE_WID 					1	//Data enable
#define SDHC_CDC_WR_WID						1 	//Write/read
#define SDHC_CDC_INIT_WID					1	//Init 80-clock prefix
#define SDHC_CDC_BW_WID						2	//Bus width 
#define SDHC_CDC_STARTRW_WID				1	//Start read wait 
#define SDHC_CDC_STOPRW_WID					1	//Stop read wait 
#define SDHC_CDC_CRLO_WID					1 	//CMD response long off 
#define SDHC_CDC_RESUME_WID					1	//CMD resume 
//RESPONSE_TO register
#define SDHC_RESTO_TO_WID 					8 	//Response time out value 
//READ_TO register
#define SDHC_READTO_TO_WID 					16 	//Read time out value
//BLK_LEN register 
#define SDHC_BL_BL_WID						12	//Block length 
//NOB register 
#define SDHC_NOB_NOB_WID 					16 	//Number of blocks 
//REV_NO register 
#define SDHC_REVNO_REVNO_WID 				16 	//Revision number 
//INTR_CNTR register 
#define SDHC_INT_RODONE_WID 				1 	//read operation done 
#define SDHC_INT_WODONE_WID 				1	//write operation done
#define SDHC_INT_ECR_WID					1	//end command response  		
#define SDHC_INT_BWE_WID					1	//Buffer write enable 
#define SDHC_INT_BRE_WID					1	//Buffer read enable 
#define SDHC_INT_DAT0EN_WID					1	//Dat0 enable 
#define SDHC_INT_SDIOIRQ_WID				1	//SDIO IRQ enable 
#define SDHC_INT_CARDRMV_WID				1	//Card removal enable 
#define SDHC_INT_CARDINS_WID				1	//Card insertion enable 
#define SDHC_INT_CARDRMVWKP_WID				1 	//Card removal wake-up enable 
#define SDHC_INT_CARDINSWKP_WID				1	//Card insertion wake-up enable 
#define SDHC_INT_SDIOIRQWKP_WID				1	//SDIO IRQ wake-up enable 
//CMD register 
#define SDHC_CMD_CMD_WID 					6 	//Command number 
//ARG register 
#define SDHC_ARG_ARG_WID 					32 	//Argument 
//RES_FIFO register 
#define SDHC_RESFIFO_CONTENT_WID			16 	//Response FIFO content 
//BUFFER_ACCESS register 
#define SDHC_BA_CONTENT_WID					32 	//Buffer access FIFO content 

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define CARD_DETECT_INSERTION	  			0
#define CARD_DETECT_REMOVAL  				1
#ifdef __cplusplus
}
#endif

#endif // __MX27_SDHC_H

