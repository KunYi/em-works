//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_ssi.h
//
//  Provides definitions for the SSI (Synchronous Serial Interface) module
//  that are common to Freescale SoCs.
//
//-----------------------------------------------------------------------------
#ifndef __COMMON_SSI_H
#define __COMMON_SSI_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 STX0;
    UINT32 STX1;
    UINT32 SRX0;
    UINT32 SRX1;
    UINT32 SCR;
    UINT32 SISR;
    UINT32 SIER;
    UINT32 STCR;
    UINT32 SRCR;
    UINT32 STCCR;
    UINT32 SRCCR;
    UINT32 SFCSR;
    UINT32 STR;
    UINT32 SOR;
    UINT32 SACNT;
    UINT32 SACADD;
    UINT32 SACDAT;
    UINT32 SATAG;
    UINT32 STMSK;
    UINT32 SRMSK;
} CSP_SSI_REG, *PCSP_SSI_REG;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define SSI_STX0_OFFSET                 0x0000
#define SSI_STX1_OFFSET                 0x0004
#define SSI_SRX0_OFFSET                 0x0008
#define SSI_SRX1_OFFSET                 0x000C
#define SSI_SCR_OFFSET                  0x0010
#define SSI_SISR_OFFSET                 0x0014
#define SSI_SIER_OFFSET                 0x0018
#define SSI_STCR_OFFSET                 0x001C
#define SSI_SRCR_OFFSET                 0x0020
#define SSI_STCCR_OFFSET                0x0024
#define SSI_SRCCR_OFFSET                0x0028
#define SSI_SFCSR_OFFSET                0x002C
#define SSI_STR_OFFSET                  0x0030
#define SSI_SOR_OFFSET                  0x0034
#define SSI_SACNT_OFFSET                0x0038
#define SSI_SACADD_OFFSET               0x003C
#define SSI_SACDAT_OFFSET               0x0040
#define SSI_SATAG_OFFSET                0x0044
#define SSI_STMSK_OFFSET                0x0048
#define SSI_SRMSK_OFFSET                0x004C


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define SSI_SCR_SSIEN_LSH               0
#define SSI_SCR_TE_LSH                  1
#define SSI_SCR_RE_LSH                  2
#define SSI_SCR_NET_LSH                 3
#define SSI_SCR_SYN_LSH                 4
#define SSI_SCR_I2S_MODE_LSH            5
#define SSI_SCR_SYS_CLK_EN_LSH          7
#define SSI_SCR_TCH_EN_LSH              8
#define SSI_SCR_CLK_IST_LSH             9

#define SSI_SISR_TFE0_LSH               0
#define SSI_SISR_TFE1_LSH               1
#define SSI_SISR_RFF0_LSH               2
#define SSI_SISR_RFF1_LSH               3
#define SSI_SISR_RLS_LSH                4
#define SSI_SISR_TLS_LSH                5
#define SSI_SISR_RFS_LSH                6
#define SSI_SISR_TFS_LSH                7
#define SSI_SISR_TUE0_LSH               8
#define SSI_SISR_TUE1_LSH               9
#define SSI_SISR_ROE0_LSH               10
#define SSI_SISR_ROE1_LSH               11
#define SSI_SISR_TDE0_LSH               12
#define SSI_SISR_TDE1_LSH               13
#define SSI_SISR_RDR0_LSH               14
#define SSI_SISR_RDR1_LSH               15
#define SSI_SISR_RXT_LSH                16
#define SSI_SISR_CMDDU_LSH              17
#define SSI_SISR_CMDAU_LSH              18

#define SSI_SIER_TFE0_EN_LSH            0
#define SSI_SIER_TFE1_EN_LSH            1
#define SSI_SIER_RFF0_EN_LSH            2
#define SSI_SIER_RFF1_EN_LSH            3
#define SSI_SIER_RLS_EN_LSH             4
#define SSI_SIER_TLS_EN_LSH             5
#define SSI_SIER_RFS_EN_LSH             6
#define SSI_SIER_TFS_EN_LSH             7
#define SSI_SIER_TUE0_EN_LSH            8
#define SSI_SIER_TUE1_EN_LSH            9
#define SSI_SIER_ROE0_EN_LSH            10
#define SSI_SIER_ROE1_EN_LSH            11
#define SSI_SIER_TDE0_EN_LSH            12
#define SSI_SIER_TDE1_EN_LSH            13
#define SSI_SIER_RDR0_EN_LSH            14
#define SSI_SIER_RDR1_EN_LSH            15
#define SSI_SIER_RXT_EN_LSH             16
#define SSI_SIER_CMDDU_EN_LSH           17
#define SSI_SIER_CMDAU_EN_LSH           18
#define SSI_SIER_TIE_LSH                19
#define SSI_SIER_TDMAE_LSH              20
#define SSI_SIER_RIE_LSH                21
#define SSI_SIER_RDMAE_LSH              22

#define SSI_SRCR_REFS_LSH               0
#define SSI_SRCR_RFSL_LSH               1
#define SSI_SRCR_RFSI_LSH               2
#define SSI_SRCR_RSCKP_LSH              3
#define SSI_SRCR_RSHFD_LSH              4
#define SSI_SRCR_RXDIR_LSH              5
#define SSI_SRCR_RFDIR_LSH              6
#define SSI_SRCR_RFEN0_LSH              7
#define SSI_SRCR_RFEN1_LSH              8
#define SSI_SRCR_RXBIT0_LSH             9
#define SSI_SRCR_RXEXT_LSH              10

#define SSI_STCR_TEFS_LSH               0
#define SSI_STCR_TFSL_LSH               1
#define SSI_STCR_TFSI_LSH               2
#define SSI_STCR_TSCKP_LSH              3
#define SSI_STCR_TSHFD_LSH              4
#define SSI_STCR_TXDIR_LSH              5
#define SSI_STCR_TFDIR_LSH              6
#define SSI_STCR_TFEN0_LSH              7
#define SSI_STCR_TFEN1_LSH              8
#define SSI_STCR_TXBIT0_LSH             9

#define SSI_STCCR_PM_LSH                0
#define SSI_STCCR_DC_LSH                8
#define SSI_STCCR_WL_LSH                13
#define SSI_STCCR_PSR_LSH               17
#define SSI_STCCR_DIV2_LSH              18

#define SSI_SRCCR_PM_LSH                0
#define SSI_SRCCR_DC_LSH                8
#define SSI_SRCCR_WL_LSH                13
#define SSI_SRCCR_PSR_LSH               17
#define SSI_SRCCR_DIV2_LSH              18

#define SSI_SFCSR_TFWM0_LSH             0
#define SSI_SFCSR_RFWM0_LSH             4
#define SSI_SFCSR_TFCNT0_LSH            8
#define SSI_SFCSR_RFCNT0_LSH            12
#define SSI_SFCSR_TFWM1_LSH             16
#define SSI_SFCSR_RFWM1_LSH             20
#define SSI_SFCSR_TFCNT1_LSH            24
#define SSI_SFCSR_RFCNT1_LSH            28

#define SSI_SOR_SYNRST_LSH              0
#define SSI_SOR_WAIT_LSH                1
#define SSI_SOR_INIT_LSH                3
#define SSI_SOR_TX_CLR_LSH              4
#define SSI_SOR_RX_CLR_LSH              5
#define SSI_SOR_CLKOFF_LSH              6


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define SSI_SCR_SSIEN_WID               1
#define SSI_SCR_TE_WID                  1
#define SSI_SCR_RE_WID                  1
#define SSI_SCR_NET_WID                 1
#define SSI_SCR_SYN_WID                 1
#define SSI_SCR_I2S_MODE_WID            2
#define SSI_SCR_SYS_CLK_EN_WID          1
#define SSI_SCR_TCH_EN_WID              1
#define SSI_SCR_CLK_IST_WID             1

#define SSI_SISR_TFE0_WID               1
#define SSI_SISR_TFE1_WID               1
#define SSI_SISR_RFF0_WID               1
#define SSI_SISR_RFF1_WID               1
#define SSI_SISR_RLS_WID                1
#define SSI_SISR_TLS_WID                1
#define SSI_SISR_RFS_WID                1
#define SSI_SISR_TFS_WID                1
#define SSI_SISR_TUE0_WID               1
#define SSI_SISR_TUE1_WID               1
#define SSI_SISR_ROE0_WID               1
#define SSI_SISR_ROE1_WID               1
#define SSI_SISR_TDE0_WID               1
#define SSI_SISR_TDE1_WID               1
#define SSI_SISR_RDR0_WID               1
#define SSI_SISR_RDR1_WID               1
#define SSI_SISR_RXT_WID                1
#define SSI_SISR_CMDDU_WID              1
#define SSI_SISR_CMDAU_WID              1

#define SSI_SIER_TFE0_EN_WID            1
#define SSI_SIER_TFE1_EN_WID            1
#define SSI_SIER_RFF0_EN_WID            1
#define SSI_SIER_RFF1_EN_WID            1
#define SSI_SIER_RLS_EN_WID             1
#define SSI_SIER_TLS_EN_WID             1
#define SSI_SIER_RFS_EN_WID             1
#define SSI_SIER_TFS_EN_WID             1
#define SSI_SIER_TUE0_EN_WID            1
#define SSI_SIER_TUE1_EN_WID            1
#define SSI_SIER_ROE0_EN_WID            1
#define SSI_SIER_ROE1_EN_WID            1
#define SSI_SIER_TDE0_EN_WID            1
#define SSI_SIER_TDE1_EN_WID            1
#define SSI_SIER_RDR0_EN_WID            1
#define SSI_SIER_RDR1_EN_WID            1
#define SSI_SIER_RXT_EN_WID             1
#define SSI_SIER_CMDDU_EN_WID           1
#define SSI_SIER_CMDAU_EN_WID           1
#define SSI_SIER_TIE_WID                1
#define SSI_SIER_TDMAE_WID              1
#define SSI_SIER_RIE_WID                1
#define SSI_SIER_RDMAE_WID              1

#define SSI_SRCR_REFS_WID               1
#define SSI_SRCR_RFSL_WID               1
#define SSI_SRCR_RFSI_WID               1
#define SSI_SRCR_RSCKP_WID              1
#define SSI_SRCR_RSHFD_WID              1
#define SSI_SRCR_RXDIR_WID              1
#define SSI_SRCR_RFDIR_WID              1
#define SSI_SRCR_RFEN0_WID              1
#define SSI_SRCR_RFEN1_WID              1
#define SSI_SRCR_RXBIT0_WID             1
#define SSI_SRCR_RXEXT_WID              1

#define SSI_STCR_TEFS_WID               1
#define SSI_STCR_TFSL_WID               1
#define SSI_STCR_TFSI_WID               1
#define SSI_STCR_TSCKP_WID              1
#define SSI_STCR_TSHFD_WID              1
#define SSI_STCR_TXDIR_WID              1
#define SSI_STCR_TFDIR_WID              1
#define SSI_STCR_TFEN0_WID              1
#define SSI_STCR_TFEN1_WID              1
#define SSI_STCR_TXBIT0_WID             1

#define SSI_STCCR_PM_WID                8
#define SSI_STCCR_DC_WID                5
#define SSI_STCCR_WL_WID                4
#define SSI_STCCR_PSR_WID               1
#define SSI_STCCR_DIV2_WID              1

#define SSI_SRCCR_PM_WID                8
#define SSI_SRCCR_DC_WID                5
#define SSI_SRCCR_WL_WID                4
#define SSI_SRCCR_PSR_WID               1
#define SSI_SRCCR_DIV2_WID              1

#define SSI_SFCSR_TFWM0_WID             4
#define SSI_SFCSR_RFWM0_WID             4
#define SSI_SFCSR_TFCNT0_WID            4
#define SSI_SFCSR_RFCNT0_WID            4
#define SSI_SFCSR_TFWM1_WID             4
#define SSI_SFCSR_RFWM1_WID             4
#define SSI_SFCSR_TFCNT1_WID            4
#define SSI_SFCSR_RFCNT1_WID            4

#define SSI_SOR_SYNRST_WID              1
#define SSI_SOR_WAIT_WID                2
#define SSI_SOR_INIT_WID                1
#define SSI_SOR_TX_CLR_WID              1
#define SSI_SOR_RX_CLR_WID              1
#define SSI_SOR_CLKOFF_WID              1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// SCR
#define SSI_SCR_SSIEN_DISABLE           0       // Disable SSI
#define SSI_SCR_SSIEN_ENABLE            1       // Enable SSI

#define SSI_SCR_TE_DISABLE              0       // Disable transmit section
#define SSI_SCR_TE_ENABLE               1       // Enable transmit section

#define SSI_SCR_RE_DISABLE              0       // Disable receive section
#define SSI_SCR_RE_ENABLE               1       // Enable receive section

#define SSI_SCR_NET_DISABLE             0       // Network mode not selected
#define SSI_SCR_NET_ENABLE              1       // Network mode selected

#define SSI_SCR_SYN_ASYNC               0       // Asynchronous mode selected
#define SSI_SCR_SYN_SYNC                1       // Synchronous mode selected

#define SSI_SCR_I2S_MODE_NORMAL         0       // Not in I2S mode
#define SSI_SCR_I2S_MODE_MASTER         1       // I2S master mode
#define SSI_SCR_I2S_MODE_SLAVE          2       // I2S slave mode

#define SSI_SCR_SYS_CLK_EN_OFF          0       // SYS_CLK output on SRCK port
#define SSI_SCR_SYS_CLK_EN_ON           1       // SYS_CLK output on SRCK port

#define SSI_SCR_TCH_EN_2CHAN_OFF        0       // Two channel mode disabled
#define SSI_SCR_TCH_EN_2CHAN_ON         1       // Two channel mode enabled

#define SSI_SCR_CLK_IST_HIGH            0       // Clock idle state is 0
#define SSI_SCR_CLK_IST_LOW             1       // Clock idle state is 1

// SISR
#define SSI_SISR_TFE0_NOT_EMPTY         0       // Transmit FIFO0 has data
#define SSI_SISR_TFE0_EMPTY             1       // Transmit FIFO0 is empty

#define SSI_SISR_TFE1_NOT_EMPTY         0       // Transmit FIFO1 has data
#define SSI_SISR_TFE1_EMPTY             1       // Transmit FIFO1 is empty

#define SSI_SISR_RFF0_NOT_FULL          0       // Space available in RX FIFO0
#define SSI_SISR_RFF0_FULL              1       // RX FIFO0 is full

#define SSI_SISR_RFF1_NOT_FULL          0       // Space available in RX FIFO1
#define SSI_SISR_RFF1_FULL              1       // RX FIFO1 is full

#define SSI_SISR_RLS_NOT_LAST_SLOT      0       // RX slot is not last in frame
#define SSI_SISR_RLS_LAST_SLOT          1       // RX slot is last in frame

#define SSI_SISR_TLS_NOT_LAST_SLOT      0       // TX slot is not last in frame
#define SSI_SISR_TLS_LAST_SLOT          1       // TX slot is last in frame

#define SSI_SISR_RFS_FS_NOT_OCCUR       0       // No RX frame sync
#define SSI_SISR_RFS_FS_OCCUR           1       // RX frame sync occurred

#define SSI_SISR_TFS_FS_NOT_OCCUR       0       // No TX frame sync
#define SSI_SISR_TFS_FS_OCCUR           1       // TX frame sync occurred

#define SSI_SISR_TUE0_NO_UNDERRUN       0       // No TX FIFO0 underrun error
#define SSI_SISR_TUE0_UNDERRUN          1       // TX FIFO0 underrun error

#define SSI_SISR_TUE1_NO_UNDERRUN       0       // No TX FIFO1 underrun error
#define SSI_SISR_TUE1_UNDERRUN          1       // TX FIFO1 underrun error

#define SSI_SISR_ROE0_NO_OVERRUN        0       // No RX FIFO0 overrun error
#define SSI_SISR_ROE0_OVERRUN           1       // RX FIFO0 overrun error

#define SSI_SISR_ROE1_NO_OVERRUN        0       // No RX FIFO1 overrun error
#define SSI_SISR_ROE1_OVERRUN           1       // RX FIFO1 overrun error

#define SSI_SISR_TDE0_NOT_READY         0       // No slot available for TX
#define SSI_SISR_TDE0_READY             1       // Empty slot available for TX

#define SSI_SISR_TDE1_NOT_READY         0       // No slot available for TX
#define SSI_SISR_TDE1_READY             1       // Empty slot available for TX

#define SSI_SISR_RDR0_NOT_READY         0       // No new data ready to read
#define SSI_SISR_RDR0_READY             1       // New data ready to read

#define SSI_SISR_RDR1_NOT_READY         0       // No new data ready to read
#define SSI_SISR_RDR1_READY             1       // New data ready to read

#define SSI_SISR_RXT_SAME               0       // No change in SATAG register
#define SSI_SISR_RXT_CHANGED            1       // SATAG register has changed

#define SSI_SISR_CMDDU_SAME             0       // No change in SACDAT register
#define SSI_SISR_CMDDU_CHANGED          1       // SACDAT register has changed

#define SSI_SISR_CMDAU_SAME             0       // No change in SACADD register
#define SSI_SISR_CMDAU_CHANGED          1       // SACADD register has changed

// SRCR
#define SSI_SRCR_REFS_EARLY             1       // Frame sync 1 bit before data
#define SSI_SRCR_REFS_NORMAL            0       // Frame sync aligned with data

#define SSI_SRCR_RFSL_1WORD             0       // Frame sync 1 word in length
#define SSI_SRCR_RFSL_1BIT              1       // Frame sync 1 bit in length

#define SSI_SRCR_RFSI_ACTIVE_HIGH       0       // Frame sync active high
#define SSI_SRCR_RFSI_ACTIVE_LOW        1       // Frame sync active low

#define SSI_SRCR_RSCKP_RISING_EDGE      0       // Data clocked on rising edge
#define SSI_SRCR_RSCKP_FALLING_EDGE     1       // Data clocked on falling edge

#define SSI_SRCR_RSHFD_MSB_FIRST        0       // Data sent MSB first
#define SSI_SRCR_RSHFD_LSB_FIRST        1       // Data sent LSB first

#define SSI_SRCR_RXDIR_EXTERNAL         0       // RX clock is external
#define SSI_SRCR_RXDIR_INTERNAL         1       // RX clock is internal

#define SSI_SRCR_RFDIR_EXTERNAL         0       // Frame sync is external
#define SSI_SRCR_RFDIR_INTERNAL         1       // Frame sync is internal

#define SSI_SRCR_RFEN0_DISABLE          0       // RX FIFO0 disabled
#define SSI_SRCR_RFEN0_ENABLE           1       // RX FIFO0 enabled

#define SSI_SRCR_RFEN1_DISABLE          0       // RX FIFO1 disabled
#define SSI_SRCR_RFEN1_ENABLE           1       // RX FIFO1 enabled

#define SSI_SRCR_RXBIT0_MSB_ALIGNED     0       // RX data word MSB aligned
#define SSI_SRCR_RXBIT0_LSB_ALIGNED     1       // RX data word LSB aligned

#define SSI_SRCR_RXEXT_DISABLE          0       // Sign extension off
#define SSI_SRCR_RXEXT_ENABLE           1       // Sign extension on

// STCR
#define SSI_STCR_TEFS_EARLY             1       // Frame sync 1 bit before data
#define SSI_STCR_TEFS_NORMAL            0       // Frame sync aligned with data

#define SSI_STCR_TFSL_1WORD             0       // Frame sync 1 word in length
#define SSI_STCR_TFSL_1BIT              1       // Frame sync 1 bit in length

#define SSI_STCR_TFSI_ACTIVE_HIGH       0       // Frame sync active high
#define SSI_STCR_TFSI_ACTIVE_LOW        1       // Frame sync active low

#define SSI_STCR_TSCKP_RISING_EDGE      0       // Data clocked on rising edge
#define SSI_STCR_TSCKP_FALLING_EDGE     1       // Data clocked on falling edge

#define SSI_STCR_TSHFD_MSB_FIRST        0       // Data sent MSB first
#define SSI_STCR_TSHFD_LSB_FIRST        1       // Data sent LSB first

#define SSI_STCR_TXDIR_EXTERNAL         0       // TX clock is external
#define SSI_STCR_TXDIR_INTERNAL         1       // TX clock is internal

#define SSI_STCR_TFDIR_EXTERNAL         0       // Frame sync is external
#define SSI_STCR_TFDIR_INTERNAL         1       // Frame sync is internal

#define SSI_STCR_TFEN0_DISABLE          0       // TX FIFO0 disabled
#define SSI_STCR_TFEN0_ENABLE           1       // TX FIFO0 enabled

#define SSI_STCR_TFEN1_DISABLE          0       // TX FIFO1 disabled
#define SSI_STCR_TFEN1_ENABLE           1       // TX FIFO1 enabled

#define SSI_STCR_TXBIT0_MSB_ALIGNED     0       // TX data word MSB aligned
#define SSI_STCR_TXBIT0_LSB_ALIGNED     1       // TX data word LSB aligned

// STCCR
#define SSI_STCCR_WL_8BIT               0x3     // 8-bit data word
#define SSI_STCCR_WL_10BIT              0x4     // 10-bit data word
#define SSI_STCCR_WL_12BIT              0x5     // 12-bit data word
#define SSI_STCCR_WL_14BIT              0x6     // 14-bit data word
#define SSI_STCCR_WL_16BIT              0x7     // 16-bit data word
#define SSI_STCCR_WL_18BIT              0x8     // 18-bit data word
#define SSI_STCCR_WL_20BIT              0x9     // 20-bit data word
#define SSI_STCCR_WL_22BIT              0xA     // 22-bit data word
#define SSI_STCCR_WL_24BIT              0xB     // 24-bit data word

#define SSI_STCCR_PSR_DIV8_BYPASS       0       // Bypass /8 prescalar
#define SSI_STCCR_PSR_DIV8_ENABLE       1       // Use /8 prescalar

#define SSI_STCCR_DIV2_BYPASS           0       // Bypass /2 prescalar
#define SSI_STCCR_DIV2_ENABLE           1       // Use /2 prescalar

// SRCCR
#define SSI_SRCCR_WL_8BIT               0x3     // 8-bit data word
#define SSI_SRCCR_WL_10BIT              0x4     // 10-bit data word
#define SSI_SRCCR_WL_12BIT              0x5     // 12-bit data word
#define SSI_SRCCR_WL_14BIT              0x6     // 14-bit data word
#define SSI_SRCCR_WL_16BIT              0x7     // 16-bit data word
#define SSI_SRCCR_WL_18BIT              0x8     // 18-bit data word
#define SSI_SRCCR_WL_20BIT              0x9     // 20-bit data word
#define SSI_SRCCR_WL_22BIT              0xA     // 22-bit data word
#define SSI_SRCCR_WL_24BIT              0xB     // 24-bit data word

#define SSI_SRCCR_PSR_DIV8_BYPASS       0       // Bypass /8 prescalar
#define SSI_SRCCR_PSR_DIV8_ENABLE       1       // Use /8 prescalar

#define SSI_SRCCR_DIV2_BYPASS           0       // Bypass /2 prescalar
#define SSI_SRCCR_DIV2_ENABLE           1       // Use /2 prescalar

// SOR
#define SSI_SOR_SYNRST_NO_RESET         0       // No data reset
#define SSI_SOR_SYNRST_RESET            1       // Reset on next frame sync

#define SSI_SOR_INIT_STATE_NO_RESET     0       // No state machine reset
#define SSI_SOR_INIT_STATE_RESET        1       // Reset SSI state machine

#define SSI_SOR_TX_CLR_NO_FLUSH         0       // No flush of TX FIFOs
#define SSI_SOR_TX_CLR_FLUSH            1       // Flush TX FIFOs

#define SSI_SOR_RX_CLR_NO_FLUSH         0       // No flush of RX FIFOs
#define SSI_SOR_RX_CLR_FLUSH            1       // Flush RX FIFOs

#define SSI_SOR_CLKOFF_IPG_CLK_ON       0       // Turn on ipg_clk
#define SSI_SOR_CLKOFF_IPG_CLK_OFF      1       // Turn off ipg_clk


#ifdef __cplusplus
}
#endif

#endif // __COMMON_SSI_H
