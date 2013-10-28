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
//---------------------------------------------------------------------------
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Header:  MX27_slcdc.h
//
//  Provides definitions for SLCDC module based on i.MX27.
//
//------------------------------------------------------------------------------
#ifndef __MX27_SLCDC_H
#define __MX27_SLCDC_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {
    /* 0x10022000 : SLCDC */
    REG32 DBBA;             /* Data Buffer Base Address */
    REG32 DBS;              /* Data Buffer Size */
    REG32 CBBA;             /* Command Buffer Base Address */
    REG32 CBS;              /* Command Buffer Size */
    REG32 CSS;              /* Command String Size */
    REG32 FIFOC;            /* FIFO Config */
    REG32 LCDC;             /* LCD Config */
    REG32 LCDTXC;           /* LCD Transfer Config */
    REG32 DMACCS;           /* DMAC Control Status */
    REG32 LCDCC;            /* LCD Clock Config */
    REG32 LCDWD;            /* LCD Write Data */
} CSP_SLCDC_REGS, *PCSP_SLCDC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define SLCDC_DBBA_OFFSET   0X0000
#define SLCDC_DBS_OFFSET    0X0004
#define SLCDC_CBBA_OFFSET   0X0008
#define SLCDC_CBS_OFFSET    0X000C
#define SLCDC_CSS_OFFSET    0X0010
#define SLCDC_FIFOC_OFFSET  0X0014
#define SLCDC_LCDC_OFFSET   0X0018
#define SLCDC_LCDTXC_OFFSET 0X001C
#define SLCDC_DMACCS_OFFSET 0X0020
#define SLCDC_LCDCC_OFFSET  0X0024 
#define SLCDC_LCDWD_OFFSET  0X0028                                                
        
//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
// DBBA : Data Base Address Register
#define SLCDC_DBBA_DATABASEADR_LSH      0//2

// DBS : Data Buffer Size Register
#define SLCDC_DBS_DATABUFSIZ_LSH        0

//CBBA : Command Buffer Address Register
#define SLCDC_CBBA_COMBASEADR_LSH       0//2

//CBS : Command Buffer Size Register
#define SLCDC_CBS_COMBUFSIZE_LSH        0

// CSS : Command String Size Register
#define SLCDC_CSS_COMSTRINGSIZ_LSH      0

//FIFOC : FIFO Configuration Register
#define SLCDC_FIFOC_BURST_LSH           0

//LCDCC : LCD Controller Configuration Register
#define SLCDC_LCDC_WORDPPAGE_LSH        0

//LCDTXC : LCD Transfer Configuration Reggister
#define SLCDC_LCDTXC_SCKPOL_LSH         0   
#define SLCDC_LCDTXC_CSPOL_LSH          1
#define SLCDC_LCDTXC_XFRMODE_LSH        2
#define SLCDC_LCDTXC_WORDDEFCOM_LSH     3
#define SLCDC_LCDTXC_WORDDEFDAT_LSH     4
#define SLCDC_LCDTXC_WORDDEFWRITE_LSH   5
#define SLCDC_LCDTXC_IMGEND_LSH         16

//DMACCS : SLCDC Control/Status Register
#define SLCDC_DMACCS_GO_LSH             0
#define SLCDC_DMACCS_ABORT_LSH          1
#define SLCDC_DMACCS_BUSY_LSH           2
#define SLCDC_DMACCS_TEA_LSH            4
#define SLCDC_DMACCS_UNDRFLOW_LSH       5
#define SLCDC_DMACCS_IRQ_LSH            6
#define SLCDC_DMACCS_IRQEN_LSH          7
#define SLCDC_DMACCS_PROT1_LSH          8
#define SLCDC_DMACCS_AUTOMODE_LSH       11

//LCDCC : LCD Clock Configuration Register
#define SLCDC_LCDCC_DIVIDE_LSH          0

//LCDWD : LCD Write Data Register 
#define SLCDC_LCDWD_LCDDAT_LSH          0
#define SLCDC_LCDWD_RS_LSH              16

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
// DBBA : Data Base Address Register
#define SLCDC_DBBA_DATABASEADR_WID      30

// DBS : Data Buffer Size Register
#define SLCDC_DBS_DATABUFSIZ_WID        17

//CBBA : Command Buffer Address Register
#define SLCDC_CBBA_COMBASEADR_WID       30

//CBS : Command Buffer Size Register
#define SLCDC_CBS_COMBUFSIZE_WID        17

// CSS : Command String Size Register
#define SLCDC_CSS_COMSTRINGSIZ_WID      8

//FIFOC : FIFO Configuration Register
#define SLCDC_FIFOC_BURST_WID           3

//LCDCC : LCD Controller Configuration Register
#define SLCDC_LCDC_WORDPPAGE_WID        13

//LCDTXC : LCD Transfer Configuration Reggister
#define SLCDC_LCDTXC_SCKPOL_WID         1   
#define SLCDC_LCDTXC_CSPOL_WID          1
#define SLCDC_LCDTXC_XFRMODE_WID        1
#define SLCDC_LCDTXC_WORDDEFCOM_WID     1
#define SLCDC_LCDTXC_WORDDEFDAT_WID     1
#define SLCDC_LCDTXC_WORDDEFWRITE_WID   1
#define SLCDC_LCDTXC_IMGEND_WID         2

//DMACCS : SLCDC Control/Status Register
#define SLCDC_DMACCS_GO_WID             1
#define SLCDC_DMACCS_ABORT_WID          1
#define SLCDC_DMACCS_BUSY_WID           1
#define SLCDC_DMACCS_TEA_WID            1
#define SLCDC_DMACCS_UNDRFLOW_WID       1
#define SLCDC_DMACCS_IRQ_WID            1
#define SLCDC_DMACCS_IRQEN_WID          1
#define SLCDC_DMACCS_PROT1_WID          1
#define SLCDC_DMACCS_AUTOMODE_WID       2

//LCDCC : LCD Clock Configuration Register
#define SLCDC_LCDCC_DIVIDE_WID          6

//LCDWD : LCD Write Data Register 
#define SLCDC_LCDWD_LCDDAT_WID          16
#define SLCDC_LCDWD_RS_WID              1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
//FIFOC : FIFO Configuration Register
#define SLCDC_FIFOC_BURST_1WORD             0 // Burst length set to 1 32-bit word
#define SLCDC_FIFOC_BURST_2WORD             1 // 2 32-bit words
#define SLCDC_FIFOC_BURST_3WORD             2 // 3 32-bit words
#define SLCDC_FIFOC_BURST_4WORD             3 // 4 32-bit words
#define SLCDC_FIFOC_BURST_5WORD             4 // 5 32-bit words
#define SLCDC_FIFOC_BURST_6WORD             5 // 6 32-bit words
#define SLCDC_FIFOC_BURST_7WORD             6 // 7 32-bit words
#define SLCDC_FIFOC_BURST_8WORD             7 // 8 32-bit words

//LCDTXC : LCD Transfer Configuration Reggister
#define SLCDC_LCDTXC_SCKPOL_RISING          0 // Seial data transfered on the rising redeg of the serial clock 
#define SLCDC_LCDTXC_SCKPOL_FALLING         1 // Seial data transfered on the falling redeg of the serial clock
 
#define SLCDC_LCDTXC_CSPOL_LOW              0 
#define SLCDC_LCDTXC_CSPOL_HIGH             1

#define SLCDC_LCDTXC_XFRMODE_SERIAL         0 // Word serial transfers
#define SLCDC_LCDTXC_XFRMODE_PARALLEL       1 // Word parallel transfers
    
#define SLCDC_LCDTXC_WORDDEFCOM_8BIT        0 // 8-bit command words
#define SLCDC_LCDTXC_WORDDEFCOM_16BIT       1 // 16-bit command words

#define SLCDC_LCDTXC_WORDDEFDAT_8BIT        0 // 8-bit data words
#define SLCDC_LCDTXC_WORDDEFDAT_16BIT       1 // 16-bit data words

#define SLCDC_LCDTXC_WORDDEFWRITE_8BIT      0 // 8-bit data words
#define SLCDC_LCDTXC_WORDDEFWRITE_16BIT     1 // 16-bit data words

//DMACCS : SLCDC Control/Status Register
#define SLCDC_DMACCS_GO_STOP                0 // Not start SLCDC transfer
#define SLCDC_DMACCS_GO_START               1 // Start SLCDC transfer

#define SLCDC_DMACCS_ABORT_DISABLE          0 // Do not abort current data tansfer
#define SLCDC_DMACCS_ABORT_ENABLE           1 // Abort current data tansfer

#define SLCDC_DMACCS_TEA_CLEAR              1 // Clear the TEA bit

#define SLCDC_DMACCS_UNDRFLOW_CLEAR         1 // Clear the UNDRFLOW bit

#define SLCDC_DMACCS_IRQ_CLEAR              1 // Clear the IRQ bit

#define SLCDC_DMACCS_IRQEN_DISABLE          0 // SLCDC interrupt disable 
#define SLCDC_DMACCS_IRQEN_ENABLE           1 // SLCDC interrupt enable

#define SLCDC_DMACCS_PROT1_USER             0 // User data accessing
#define SLCDC_DMACCS_PROT1_PRIVILEGE        1 // Privileged data accessing

#define SLCDC_DMACCS_AUTOMODE_NO_INSERT_RS0 0 // Transfer data buffer without inserting control strings, tie LCD_RS to 0
#define SLCDC_DMACCS_AUTOMODE_NO_INSERT_RS1 1 // Transfer data buffer without inserting control strings, tie LCD_RS to 1
#define SLCDC_DMACCS_AUTOMODE_INSERT_CTLSTR 2 // Transfer data buffer with inserting control strings
#define SLCDC_DMACCS_AUTOMODE_RESERVED      3 // Reserved

//LCDWD : LCD Write Data Register 
#define SLCDC_LCDWD_RS_CMDDAT               0 // LCDDAT is command data
#define SLCDC_LCDWD_RS_DISPDAT              1 // LCDDAT is display data


#ifdef __cplusplus
}
#endif

#endif // __MX27_LCDC_H
