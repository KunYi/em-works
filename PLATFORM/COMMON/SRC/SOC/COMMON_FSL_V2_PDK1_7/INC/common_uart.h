//-----------------------------------------------------------------------------
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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_uart.h
//
//  Provides definitions for the UART (Universal Asynchronous Receiver 
//  Transmitter) module that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------

#ifndef __COMMON_UART_H
#define __COMMON_UART_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define IRSC_BAUDRATE    38400
#define UART_BAUDRATE_DEFAULT    115200
#define UART_UBMR_MOD_DEFAULT    30000
#define UART_UBIR_INCREMENT(baud, UBMR, refFreq)   \
                                 ((UINT32)(((UINT64)baud * 16*UBMR / refFreq)))


#define SER_FIFO_DEPTH    32
#define SER_FIFO_TXTL    0x10
#define SER_FIFO_RXTL    0x10
#define UART_URXD_RX_DATA_MSK    0xFF

//------------------------------------------------------------------------------
// Types

typedef enum uartType {
    DCE,
    DTE
}uartType_c;

typedef enum irMode {
    SIR_MODE,
    MIR_MODE,
    FIR_MODE
}irMode_c;

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define UART_RXFIFO_DEPTH       32
#define UART_TXFIFO_DEPTH       32

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 URXD;
    UINT32 reserved0[15];
    UINT32 UTXD;
    UINT32 reserved1[15];
    UINT32 UCR1;
    UINT32 UCR2;
    UINT32 UCR3;
    UINT32 UCR4;
    UINT32 UFCR;
    UINT32 USR1;
    UINT32 USR2;
    UINT32 UESC;
    UINT32 UTIM;
    UINT32 UBIR;
    UINT32 UBMR;
    UINT32 UBRC;
    UINT32 ONEMS;
    UINT32 UTS;
} CSP_UART_REG, *PCSP_UART_REG;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define UART_URXD_OFFSET            0x0000
#define UART_UTXD_OFFSET            0x0040
#define UART_UCR1_OFFSET            0x0080
#define UART_UCR2_OFFSET            0x0084
#define UART_UCR3_OFFSET            0x0088
#define UART_UCR4_OFFSET            0x008C
#define UART_UFCR_OFFSET            0x0090
#define UART_USR1_OFFSET            0x0094
#define UART_USR2_OFFSET            0x0098
#define UART_UESC_OFFSET            0x009C
#define UART_UTIM_OFFSET            0x00A0
#define UART_UBIR_OFFSET            0x00A4
#define UART_UBMR_OFFSET            0x00A8
#define UART_UBRC_OFFSET            0x00AC
#define UART_ONEMS_OFFSET           0x00B0
#define UART_UTS_OFFSET             0x00B4

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define UART_URXD_RX_DATA_LSH       0
#define UART_URXD_PRERR_LSH         10
#define UART_URXD_BRK_LSH           11
#define UART_URXD_FRMERR_LSH        12
#define UART_URXD_OVRRUN_LSH        13
#define UART_URXD_ERR_LSH           14

#define UART_UTXD_TX_DATA_LSH       0

#define UART_UCR1_UARTEN_LSH        0
#define UART_UCR1_DOZE_LSH          1
#define UART_UCR1_ATDMAEN_LSH       2
#define UART_UCR1_TDMAEN_LSH        3
#define UART_UCR1_SNDBRK_LSH        4
#define UART_UCR1_RTSDEN_LSH        5
#define UART_UCR1_TXMPTYEN_LSH      6
#define UART_UCR1_IREN_LSH          7
#define UART_UCR1_RDMAEN_LSH        8
#define UART_UCR1_RRDYEN_LSH        9
#define UART_UCR1_ICD_LSH           10
#define UART_UCR1_IDEN_LSH          12
#define UART_UCR1_TRDYEN_LSH        13
#define UART_UCR1_ADBR_LSH          14
#define UART_UCR1_ADEN_LSH          15

#define UART_UCR2_SRST_LSH          0
#define UART_UCR2_RXEN_LSH          1
#define UART_UCR2_TXEN_LSH          2
#define UART_UCR2_ATEN_LSH          3
#define UART_UCR2_RTSEN_LSH         4
#define UART_UCR2_WS_LSH            5
#define UART_UCR2_STPB_LSH          6
#define UART_UCR2_PROE_LSH          7
#define UART_UCR2_PREN_LSH          8
#define UART_UCR2_RTEC_LSH          9
#define UART_UCR2_ESCEN_LSH         11
#define UART_UCR2_CTS_LSH           12
#define UART_UCR2_CTSC_LSH          13
#define UART_UCR2_IRTS_LSH          14
#define UART_UCR2_ESCI_LSH          15

#define UART_UCR3_ACIEN_LSH         0
#define UART_UCR3_INVT_LSH          1
#define UART_UCR3_RXDMUXSEL_LSH     2
#define UART_UCR3_DTRDEN_LSH        3
#define UART_UCR3_AWAKEN_LSH        4
#define UART_UCR3_AIRINTEN_LSH      5
#define UART_UCR3_RXDSEN_LSH        6
#define UART_UCR3_ADNIMP_LSH        7
#define UART_UCR3_RI_LSH            8
#define UART_UCR3_DCD_LSH           9
#define UART_UCR3_DSR_LSH           10
#define UART_UCR3_FRAERREN_LSH      11
#define UART_UCR3_PARERREN_LSH      12
#define UART_UCR3_DTREN_LSH         13
#define UART_UCR3_DPEC_LSH          14

#define UART_UCR4_DREN_LSH          0
#define UART_UCR4_OREN_LSH          1
#define UART_UCR4_BKEN_LSH          2
#define UART_UCR4_TCEN_LSH          3
#define UART_UCR4_LPBYP_LSH         4
#define UART_UCR4_IRSC_LSH          5
#define UART_UCR4_WKEN_LSH          7
#define UART_UCR4_ENIRI_LSH         8
#define UART_UCR4_INVR_LSH          9
#define UART_UCR4_CTSTL_LSH         10

#define UART_UFCR_RXTL_LSH          0
#define UART_UFCR_DCEDTE_LSH        6
#define UART_UFCR_RFDIV_LSH         7
#define UART_UFCR_TXTL_LSH          10

#define UART_USR1_AWAKE_LSH         4
#define UART_USR1_AIRINT_LSH        5
#define UART_USR1_RXDS_LSH          6
#define UART_USR1_DTRD_LSH          7
#define UART_USR1_AGTIM_LSH         8
#define UART_USR1_RRDY_LSH          9
#define UART_USR1_FRAMERR_LSH       10
#define UART_USR1_ESCF_LSH          11
#define UART_USR1_RTSD_LSH          12
#define UART_USR1_TRDY_LSH          13
#define UART_USR1_RTSS_LSH          14
#define UART_USR1_PARITYERR_LSH     15

#define UART_USR2_RDR_LSH           0
#define UART_USR2_ORE_LSH           1
#define UART_USR2_BRCD_LSH          2
#define UART_USR2_TXDC_LSH          3
#define UART_USR2_RTSF_LSH          4
#define UART_USR2_DCDIN_LSH         5
#define UART_USR2_DCDDELT_LSH       6
#define UART_USR2_WAKE_LSH          7
#define UART_USR2_IRINT_LSH         8
#define UART_USR2_RIIN_LSH          9
#define UART_USR2_RIDELT_LSH        10
#define UART_USR2_ACST_LSH          11
#define UART_USR2_IDLE_LSH          12
#define UART_USR2_DTRF_LSH          13
#define UART_USR2_TXFE_LSH          14
#define UART_USR2_ADET_LSH          15

#define UART_UESC_ESC_CHAR_LSH      0

#define UART_UTIM_TIM_LSH           0

#define UART_UBIR_INC_LSH           0

#define UART_UBMR_MOD_LSH           0

#define UART_UBRC_BCNT_LSH          0

#define UART_ONEMS_ONEMS_LSH        0

#define UART_UTS_SOFTRST_LSH        0
#define UART_UTS_RXFULL_LSH         3
#define UART_UTS_TXFULL_LSH         4
#define UART_UTS_RXEMPTY_LSH        5
#define UART_UTS_TXEMPTY_LSH        6
#define UART_UTS_RXDBG_LSH          9
#define UART_UTS_LOOPIR_LSH         10
#define UART_UTS_DBGEN_LSH          11
#define UART_UTS_LOOP_LSH           12
#define UART_UTS_FRCPERR_LSH        13


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define UART_URXD_RX_DATA_WID       8
#define UART_URXD_PRERR_WID         1
#define UART_URXD_BRK_WID           1
#define UART_URXD_FRMERR_WID        1
#define UART_URXD_OVRRUN_WID        1
#define UART_URXD_ERR_WID           1

#define UART_UTXD_TX_DATA_WID       8

#define UART_UCR1_UARTEN_WID        1
#define UART_UCR1_DOZE_WID          1
#define UART_UCR1_ATDMAEN_WID       1
#define UART_UCR1_TDMAEN_WID        1
#define UART_UCR1_SNDBRK_WID        1
#define UART_UCR1_RTSDEN_WID        1
#define UART_UCR1_TXMPTYEN_WID      1
#define UART_UCR1_IREN_WID          1
#define UART_UCR1_RDMAEN_WID        1
#define UART_UCR1_RRDYEN_WID        1
#define UART_UCR1_ICD_WID           2
#define UART_UCR1_IDEN_WID          1
#define UART_UCR1_TRDYEN_WID        1
#define UART_UCR1_ADBR_WID          1
#define UART_UCR1_ADEN_WID          1

#define UART_UCR2_SRST_WID          1
#define UART_UCR2_RXEN_WID          1
#define UART_UCR2_TXEN_WID          1
#define UART_UCR2_ATEN_WID          1
#define UART_UCR2_RTSEN_WID         1
#define UART_UCR2_WS_WID            1
#define UART_UCR2_STPB_WID          1
#define UART_UCR2_PROE_WID          1
#define UART_UCR2_PREN_WID          1
#define UART_UCR2_RTEC_LS           2
#define UART_UCR2_ESCEN_WID         1
#define UART_UCR2_CTS_WID           1
#define UART_UCR2_CTSC_WID          1
#define UART_UCR2_IRTS_WID          1
#define UART_UCR2_ESCI_WID          1

#define UART_UCR3_ACIEN_WID         1
#define UART_UCR3_INVT_WID          1
#define UART_UCR3_RXDMUXSEL_WID     1
#define UART_UCR3_DTRDEN_WID        1
#define UART_UCR3_AWAKEN_WID        1
#define UART_UCR3_AIRINTEN_WID      1
#define UART_UCR3_RXDSEN_WID        1
#define UART_UCR3_ADNIMP_WID        1
#define UART_UCR3_RI_WID            1
#define UART_UCR3_DCD_WID           1
#define UART_UCR3_DSR_WID           1
#define UART_UCR3_FRAERREN_WID      1
#define UART_UCR3_PARERREN_WID      1
#define UART_UCR3_DTREN_WID         1
#define UART_UCR3_DPEC_WID          2

#define UART_UCR4_DREN_WID          1
#define UART_UCR4_OREN_WID          1
#define UART_UCR4_BKEN_WID          1
#define UART_UCR4_TCEN_WID          1
#define UART_UCR4_LPBYP_WID         1
#define UART_UCR4_IRSC_WID          1
#define UART_UCR4_WKEN_WID          1
#define UART_UCR4_ENIRI_WID         1
#define UART_UCR4_INVR_WID          1
#define UART_UCR4_CTSTL_WID         6

#define UART_UFCR_RXTL_WID          6
#define UART_UFCR_DCEDTE_WID        1
#define UART_UFCR_RFDIV_WID         3
#define UART_UFCR_TXTL_WID          6

#define UART_USR1_AWAKE_WID         1
#define UART_USR1_AIRINT_WID        1
#define UART_USR1_RXDS_WID          1
#define UART_USR1_DTRD_WID          1
#define UART_USR1_AGTIM_WID         1
#define UART_USR1_RRDY_WID          1
#define UART_USR1_FRAMERR_WID       1
#define UART_USR1_ESCF_WID          1
#define UART_USR1_RTSD_WID          1
#define UART_USR1_TRDY_WID          1
#define UART_USR1_RTSS_WID          1
#define UART_USR1_PARITYERR_WID     1

#define UART_USR2_RDR_WID           1
#define UART_USR2_ORE_WID           1
#define UART_USR2_BRCD_WID          1
#define UART_USR2_TXDC_WID          1
#define UART_USR2_RTSF_WID          1
#define UART_USR2_DCDIN_WID         1
#define UART_USR2_DCDDELT_WID       1
#define UART_USR2_WAKE_WID          1
#define UART_USR2_IRINT_WID         1
#define UART_USR2_RIIN_WID          1
#define UART_USR2_RIDELT_WID        1
#define UART_USR2_ACST_WID          1
#define UART_USR2_IDLE_WID          1
#define UART_USR2_DTRF_WID          1
#define UART_USR2_TXFE_WID          1
#define UART_USR2_ADET_WID          1

#define UART_UESC_ESC_CHAR_WID      8

#define UART_UTIM_TIM_WID           12

#define UART_UBIR_INC_WID           16

#define UART_UBMR_MOD_WID           16

#define UART_UBRC_BCNT_WID          16

#define UART_ONEMS_ONEMS_WID        16

#define UART_UTS_SOFTRST_WID        1
#define UART_UTS_RXFULL_WID         1
#define UART_UTS_TXFULL_WID         1
#define UART_UTS_RXEMPTY_WID        1
#define UART_UTS_TXEMPTY_WID        1
#define UART_UTS_RXDBG_WID          1
#define UART_UTS_LOOPIR_WID         1
#define UART_UTS_DBGEN_WID          1
#define UART_UTS_LOOP_WID           1
#define UART_UTS_FRCPERR_WID        1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// URXD
#define UART_URXD_PRERR_NOERROR     0
#define UART_URXD_PRERR_ERROR       1

#define UART_URXD_BRK_NOBREAK       0
#define UART_URXD_BRK_BREAK         1
 
#define UART_URXD_FRMERR_NOERROR    0
#define UART_URXD_FRMERR_ERROR      1
 
#define UART_URXD_OVRRUN_NOOVRRUN   0
#define UART_URXD_OVRRUN_OVRRUN     1
 
#define UART_URXD_ERR_NOERROR       0
#define UART_URXD_ERR_ERROR         1


// UCR1
#define UART_UCR1_UARTEN_DISABLE    0
#define UART_UCR1_UARTEN_ENABLE     1

#define UART_UCR1_DOZE_ENABLE       0
#define UART_UCR1_DOZE_DISABLE      1

#define UART_UCR1_TXDMAEN_DISABLE   0
#define UART_UCR1_TXDMAEN_ENABLE    1

#define UART_UCR1_SNDBRK_NOBREAK    0
#define UART_UCR1_SNDBRK_BREAK      1

#define UART_UCR1_RTSDEN_DISABLE    0
#define UART_UCR1_RTSDEN_ENABLE     1

#define UART_UCR1_TXMPTYEN_DISABLE  0
#define UART_UCR1_TXMPTYEN_ENABLE   1

#define UART_UCR1_TXMPTYEN_DISABLE  0
#define UART_UCR1_TXMPTYEN_ENABLE   1

#define UART_UCR1_IREN_DISABLE      0
#define UART_UCR1_IREN_ENABLE       1

#define UART_UCR1_RXDMAEN_DISABLE   0
#define UART_UCR1_RXDMAEN_ENABLE    1

#define UART_UCR1_RRDYEN_DISABLE    0
#define UART_UCR1_RRDYEN_ENABLE     1

#define UART_UCR1_ICD_4FRAMES       0
#define UART_UCR1_ICD_8FRAMES       1
#define UART_UCR1_ICD_16FRAMES      2
#define UART_UCR1_ICD_32FRAMES      3

#define UART_UCR1_IDEN_DISABLE      0
#define UART_UCR1_IDEN_ENABLE       1

#define UART_UCR1_TRDYEN_DISABLE    0
#define UART_UCR1_TRDYEN_ENABLE     1

#define UART_UCR1_ADBR_DISABLE      0
#define UART_UCR1_ADBR_ENABLE       1

#define UART_UCR1_ADEN_DISABLE      0
#define UART_UCR1_ADEN_ENABLE       1


// UCR2
#define UART_UCR2_SRST_RESET        0
#define UART_UCR2_SRST_NORESET      1

#define UART_UCR2_RXEN_DISABLE      0
#define UART_UCR2_RXEN_ENABLE       1

#define UART_UCR2_TXEN_DISABLE      0
#define UART_UCR2_TXEN_ENABLE       1

#define UART_UCR2_ATEN_DISABLE      0
#define UART_UCR2_ATEN_ENABLE       1

#define UART_UCR2_RTSEN_DISABLE     0
#define UART_UCR2_RTSEN_ENABLE      1

#define UART_UCR2_WS_7BIT           0
#define UART_UCR2_WS_8BIT           1

#define UART_UCR2_STPB_1STOP        0
#define UART_UCR2_STPB_2STOP        1

#define UART_UCR2_PROE_EVEN         0
#define UART_UCR2_PROE_ODD          1

#define UART_UCR2_PREN_DISBLE       0
#define UART_UCR2_PREN_ENABLE       1

#define UART_UCR2_RTEC_RISEDGE      0
#define UART_UCR2_RTEC_FALLEDGE     1
#define UART_UCR2_RTEC_ANYEDGE      2

#define UART_UCR2_ESCEN_DISABLE     0
#define UART_UCR2_ESCEN_ENABLE      1

#define UART_UCR2_CTS_HIGH          0
#define UART_UCR2_CTS_LOW           1

#define UART_UCR2_CTSC_BITCTRL      0
#define UART_UCR2_CTSC_RXCTRL       1

#define UART_UCR2_IRTS_USERTS       0
#define UART_UCR2_IRTS_IGNORERTS    1

#define UART_UCR2_ESCI_DISABLE      0
#define UART_UCR2_ESCI_ENABLE       1


// UCR3
#define UART_UCR3_ACIEN_DISABLE     0
#define UART_UCR3_ACIEN_ENABLE      1

#define UART_UCR3_INVT_ACTIVELOW    0
#define UART_UCR3_INVT_ACTIVEHIGH   1

#define UART_UCR3_RXDMUXSEL_NOTMUX  0
#define UART_UCR3_RXDMUXSEL_MUX     1

#define UART_UCR3_DTRDEN_DISABLE    0
#define UART_UCR3_DTRDEN_ENABLE     1

#define UART_UCR3_AWAKEN_DISABLE    0
#define UART_UCR3_AWAKEN_ENABLE     1

#define UART_UCR3_AIRINTEN_DISABLE  0
#define UART_UCR3_AIRINTEN_ENABLE   1

#define UART_UCR3_RXDSEN_DISABLE    0
#define UART_UCR3_RXDSEN_ENABLE     1

#define UART_UCR3_ADNIMP_NEW        0
#define UART_UCR3_ADNIMP_OLD        1

#define UART_UCR3_RI_LOW            0   // DCE
#define UART_UCR3_RI_HIGH           1   // DCE
#define UART_UCR3_RI_DISABLE        0   // DTE
#define UART_UCR3_RI_ENABLE         1   // DTE

#define UART_UCR3_DCD_LOW           0   // DCE
#define UART_UCR3_DCD_HIGH          1   // DCE
#define UART_UCR3_DCD_DISABLE       0   // DTE
#define UART_UCR3_DCD_ENABLE        1   // DTE

#define UART_UCR3_DSR_LOW           0
#define UART_UCR3_DSR_HIGH          1

#define UART_UCR3_FRAERREN_DISABLE  0
#define UART_UCR3_FRAERREN_ENABLE   1

#define UART_UCR3_PARERREN_DISABLE  0
#define UART_UCR3_PARERREN_ENABLE   1

#define UART_UCR3_DTREN_DISABLE     0
#define UART_UCR3_DTREN_ENABLE      1

#define UART_UCR3_DPEC_RISEDGE      0
#define UART_UCR3_DPEC_FALLEDGE     1
#define UART_UCR3_DPEC_ANYEDGE      2


// UCR4
#define UART_UCR4_DREN_DISABLE      0
#define UART_UCR4_DREN_ENABLE       1

#define UART_UCR4_OREN_DISABLE      0
#define UART_UCR4_OREN_ENABLE       1

#define UART_UCR4_BKEN_DISABLE      0
#define UART_UCR4_BKEN_ENABLE       1

#define UART_UCR4_TCEN_DISABLE      0
#define UART_UCR4_TCEN_ENABLE       1

#define UART_UCR4_LPBYP_DISABLE     0
#define UART_UCR4_LPBYP_ENABLE      1

#define UART_UCR4_IRSC_SAMPCLK      0
#define UART_UCR4_IRSC_REFCLK       1

#define UART_UCR4_WKEN_DISABLE      0
#define UART_UCR4_WKEN_ENABLE       1

#define UART_UCR4_ENIRI_DISABLE     0
#define UART_UCR4_ENIRI_ENABLE      1

#define UART_UCR4_INVR_ACTIVELOW    0
#define UART_UCR4_INVR_ACTIVEHIGH   1


// UFCR
#define UART_UFCR_DCEDTE_DCE        0
#define UART_UFCR_DCEDTE_DTE        1

#define UART_UFCR_RFDIV_DIV6        0
#define UART_UFCR_RFDIV_DIV5        1
#define UART_UFCR_RFDIV_DIV4        2
#define UART_UFCR_RFDIV_DIV3        3
#define UART_UFCR_RFDIV_DIV2        4
#define UART_UFCR_RFDIV_DIV1        5
#define UART_UFCR_RFDIV_DIV7        6


// USR1
#define UART_USR1_AWAKE_SET         1
#define UART_USR1_AIRINT_SET        1
#define UART_USR1_RXDS_SET          1
#define UART_USR1_DTRD_SET          1
#define UART_USR1_AGTIM_SET         1
#define UART_USR1_RRDY_SET          1
#define UART_USR1_FRAMERR_SET       1
#define UART_USR1_ESCF_SET          1
#define UART_USR1_RTSD_SET          1
#define UART_USR1_TRDY_SET          1
#define UART_USR1_RTSS_SET          1
#define UART_USR1_PARITYERR_SET     1

// USR2
#define UART_USR2_RDR_SET           1
#define UART_USR2_ORE_SET           1
#define UART_USR2_BRCD_SET          1
#define UART_USR2_TXDC_SET          1
#define UART_USR2_RTSF_SET          1
#define UART_USR2_DCDIN_SET         1
#define UART_USR2_DCDDELT_SET       1
#define UART_USR2_WAKE_SET          1
#define UART_USR2_IRINT_SET         1
#define UART_USR2_RIIN_SET          1
#define UART_USR2_RIDELT_SET        1
#define UART_USR2_ACST_SET          1
#define UART_USR2_IDLE_SET          1
#define UART_USR2_DTRF_SET          1
#define UART_USR2_TXFE_SET          1
#define UART_USR2_ADET_SET          1

// UTS
#define UART_UTS_RXDBG_NOINCREMENT  0
#define UART_UTS_RXDBG_INCREMENT    1

#define UART_UTS_LOOPIR_NOLOOP      0
#define UART_UTS_LOOPIR_LOOP        1

#define UART_UTS_DBGEN_DEBUG        0
#define UART_UTS_DBGEN_NODEBUG      1

#define UART_UTS_LOOP_NOLOOP        0
#define UART_UTS_LOOP_LOOP          1

#define UART_UTS_FRCPERR_NOERROR    0
#define UART_UTS_FRCPERR_ERROR      1


#ifdef __cplusplus
}
#endif

#endif // __COMMON_UART_H
