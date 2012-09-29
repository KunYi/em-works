//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_can.h
//
//  Provides definitions for the CAN (Controller Area Network )
//   module that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------

#ifndef __COMMON_CAN_H
#define __COMMON_CAN_H

#if __cplusplus
extern "C" {
#endif

#define CAN_NUMBER_OF_BUFFERS 64

// define CAN TIMING PARAMETERS PRESDIV=8, PROP_SEG=0, PSEG1=PSEG2=2, RJW=2
//#define CAN_TIMING_PARAMETERS 0x0892

// define CAN TIMING PARAMETERS PRESDIV=18, PROP_SEG=0, PSEG1=6 PSEG2=4, RJW=1
#define CAN_TIMING_PARAMETERS 0x1274        // 250kbps

// define CAN TIMING PARAMETERS PRESDIV=5, PROP_SEG=0, PSEG1=4 PSEG2=3, RJW=1
//#define CAN_TIMING_PARAMETERS 0x0563        // 1Mbps

// define CAN TIMING PARAMETERS PRESDIV=8, PROP_SEG=0, PSEG1=PSEG2=3 RJW=3

//#define CAN_TIMING_PARAMETERS 0x08fb

// define CAN TIMING PARAMETERS PRESDIV=1, PROP_SEG=0, PSEG1=1 PSEG2=2 RJW=1

//#define CAN_TIMING_PARAMETERS 0x014a


// define C&S of MBs to transmit 8 bytes of DATA
#define CAN_CS_8BYTES 0x0c680000

typedef struct
{
    UINT32 CS          ;      
    UINT32 ID          ;  
    UINT32 BYTE_0_3    ; 
    UINT32 BYTE_4_7    ; 
} MB_REG, *PMB_REG;

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 MCR       ;      
    UINT16 CTRL      ;  
    UINT16 CTRL_H    ; 
    UINT32 TIMER     ; 
    UINT32 TCR       ;
    UINT32 RXGMASK   ;
    UINT32 RX14MASK  ;
    UINT32 RX15MASK  ;
    UINT32 ECR       ;
    UINT32 ESR       ;
    UINT32 IMASK2    ;
    UINT32 IMASK1    ;
    UINT32 IFLAG2    ;
    UINT32 IFLAG1    ;
    UINT32 RESERVED1[19] ;
    MB_REG MB[64];
    UINT32 RESERVED2[256] ;
    UINT32 RXIMR[64] ;

} CSP_CAN_REG, *PCSP_CAN_REG;

/****************************************************************************/
/*  Bit level definitions and macros                                        */
/****************************************************************************/


/****************************************************************************/
/*  Bit level definitions and macros                                        */
/****************************************************************************/

// definitions for the bits in the MCR register
#define  CAN_MDIS_BIT_LSH				31 
#define  CAN_FRZ_BIT_LSH					30 
#define  CAN_FEN_BIT_LSH					29 
#define  CAN_HALT_BIT_LSH					28 
#define  CAN_NOT_RDY_BIT_LSH			27 
#define  CAN_WAK_MSK_BIT_LSH			26  
#define  CAN_SOFT_RST_BIT_LSH		25  
#define  CAN_FRZ_ACK_BIT_LSH			24 
#define  CAN_SUPV_BIT_LSH				23  
#define  CAN_SLF_WAK_BIT_LSH			22  
#define  CAN_WARN_EN_BIT_LSH			21  
#define  CAN_BIT_LPM_ACK_BIT_LSH	20
#define  CAN_WAK_SRC_BIT_LSH			19 
#define  CAN_DOZE_BIT_LSH				18 
#define  CAN_SRX_DIS_BIT_LSH			17  
#define  CAN_BCC_BIT_LSH					16
#define  CAN_BOFF_MSK_LSH				15 
#define  CAN_ERR_MSK_LSH					14 
#define  CAN_PEN_BIT_LSH					13  
#define  CAN_AEN_BIT_LSH					12 
#define  CAN_TWRN_MSK_LSH				11 
#define  CAN_RWRN_MSK_LSH				10 
#define  CAN_IDAM0_BIT_LSH				8 
#define  CAN_IDAM1_BIT_LSH				9  

// definitions for the bits in the CTRL register
#define  CAN_BOFF_MSK_BIT_LSH		15
#define  CAN_ERR_MSK_BIT_LSH			14
#define  CAN_CLK_SRC_BIT_LSH			13
#define  CAN_BIT_LPB_BIT_LSH			12
#define  CAN_TWRN_MSK_BIT_LSH		11
#define  CAN_RWRN_MSK_BIT_LSH		10
#define  CAN_SMP_BIT_LSH					7
#define  CAN_BOFF_REC_BIT_LSH		6
#define  CAN_TSYN_BIT_LSH				5
#define  CAN_BIT_LBUF_BIT_LSH			4
#define  CAN_BIT_LOM_BIT_LSH			3

// definitions for the bits in the TCR register
#define  CAN_TRD_BIT_LSH					10
#define  CAN_BIT_LSHCLS_BIT_LSH		9
#define  CAN_DSCACK_BIT_LSH			8

// definitions for the bits in the RX masks registers
#define  CAN_MI0_BIT_LSH			0
#define  CAN_MI1_BIT_LSH			1
#define  CAN_MI2_BIT_LSH          2
#define  CAN_MI3_BIT_LSH          3
#define  CAN_MI4_BIT_LSH          4
#define  CAN_MI5_BIT_LSH          5
#define  CAN_MI6_BIT_LSH          6
#define  CAN_MI7_BIT_LSH          7
#define  CAN_MI8_BIT_LSH          8
#define  CAN_MI9_BIT_LSH          9
#define  CAN_MI10_BIT_LSH         10
#define  CAN_MI11_BIT_LSH         11
#define  CAN_MI12_BIT_LSH         12
#define  CAN_MI13_BIT_LSH         13
#define  CAN_MI14_BIT_LSH         14
#define  CAN_MI15_BIT_LSH         15
#define  CAN_MI16_BIT_LSH         16
#define  CAN_MI17_BIT_LSH         17
#define  CAN_MI18_BIT_LSH         18
#define  CAN_MI19_BIT_LSH         19
#define  CAN_MI20_BIT_LSH         20
#define  CAN_MI21_BIT_LSH         21
#define  CAN_MI22_BIT_LSH         22
#define  CAN_MI23_BIT_LSH         23
#define  CAN_MI24_BIT_LSH         24
#define  CAN_MI25_BIT_LSH         25
#define  CAN_MI26_BIT_LSH         26
#define  CAN_MI27_BIT_LSH         27
#define  CAN_MI28_BIT_LSH         28
#define  CAN_MI29_BIT_LSH         19
#define  CAN_MI30_BIT_LSH         30
#define  CAN_MI31_BIT_LSH         31

// definitions for the bits/field in the ESR registers
#define  CAN_TWRN_INT_BIT_LSH		17
#define  CAN_RWRN_INT_BIT_LSH		16    
#define  CAN_BIT_LSH1_ERR_BIT_LSH 15
#define  CAN_BIT_LSH0_ERR_BIT_LSH 14
#define  CAN_ACK_ERR_BIT_LSH			13
#define  CAN_CRC_ERR_BIT_LSH			12
#define  CAN_FRM_ERR_BIT_LSH			11
#define  CAN_STF_ERR_BIT_LSH			10
#define  CAN_TX_WRN_BIT_LSH			9
#define  CAN_RX_WRN_BIT_LSH			8
#define  CAN_IDLE_BIT_LSH					7
#define  CAN_TXRX_BIT_LSH				6
#define  CAN_FLT_CONF_FIELD_LSH		4
#define  CAN_BOFF_INT_BIT_LSH			2
#define  CAN_ERR_INT_BIT_LSH			1
#define  CAN_WAK_INT_BIT_LSH			0

// definitions for the bits/field in the C/S Message Buffer Structure
#define  CAN_CS_RTR				20
#define  CAN_CS_IDE				21
#define  CAN_CS_SRR				22

// definitions for the bits in the IMASK registers
#define  CAN_BUF0M_BIT_LSH        0
#define  CAN_BUF1M_BIT_LSH        1
#define  CAN_BUF2M_BIT_LSH        2
#define  CAN_BUF3M_BIT_LSH        3
#define  CAN_BUF4M_BIT_LSH        4
#define  CAN_BUF5M_BIT_LSH        5
#define  CAN_BUF6M_BIT_LSH        6
#define  CAN_BUF7M_BIT_LSH        7
#define  CAN_BUF8M_BIT_LSH        8
#define  CAN_BUF9M_BIT_LSH        9
#define  CAN_BUF10M_BIT_LSH       10
#define  CAN_BUF11M_BIT_LSH       11
#define  CAN_BUF12M_BIT_LSH       12
#define  CAN_BUF13M_BIT_LSH       13
#define  CAN_BUF14M_BIT_LSH       14
#define  CAN_BUF15M_BIT_LSH       15
#define  CAN_BUF16M_BIT_LSH       16
#define  CAN_BUF17M_BIT_LSH       17
#define  CAN_BUF18M_BIT_LSH       18
#define  CAN_BUF19M_BIT_LSH       19
#define  CAN_BUF20M_BIT_LSH       20
#define  CAN_BUF21M_BIT_LSH       21
#define  CAN_BUF22M_BIT_LSH       22
#define  CAN_BUF23M_BIT_LSH       23
#define  CAN_BUF24M_BIT_LSH       24
#define  CAN_BUF25M_BIT_LSH       25
#define  CAN_BUF26M_BIT_LSH       26
#define  CAN_BUF27M_BIT_LSH       27
#define  CAN_BUF28M_BIT_LSH       28
#define  CAN_BUF29M_BIT_LSH       29
#define  CAN_BUF30M_BIT_LSH       30
#define  CAN_BUF31M_BIT_LSH       31

#define  CAN_BUF32M_BIT_LSH       0
#define  CAN_BUF33M_BIT_LSH       1
#define  CAN_BUF34M_BIT_LSH       2
#define  CAN_BUF35M_BIT_LSH       3
#define  CAN_BUF36M_BIT_LSH       4
#define  CAN_BUF37M_BIT_LSH       5
#define  CAN_BUF38M_BIT_LSH       6
#define  CAN_BUF39M_BIT_LSH       7
#define  CAN_BUF40M_BIT_LSH       8
#define  CAN_BUF41M_BIT_LSH       9
#define  CAN_BUF42M_BIT_LSH       10
#define  CAN_BUF43M_BIT_LSH       11
#define  CAN_BUF44M_BIT_LSH       12
#define  CAN_BUF45M_BIT_LSH       13
#define  CAN_BUF46M_BIT_LSH       14
#define  CAN_BUF47M_BIT_LSH       15
#define  CAN_BUF48M_BIT_LSH       16
#define  CAN_BUF49M_BIT_LSH       17
#define  CAN_BUF50M_BIT_LSH       18
#define  CAN_BUF51M_BIT_LSH       19
#define  CAN_BUF52M_BIT_LSH       20
#define  CAN_BUF53M_BIT_LSH       21
#define  CAN_BUF54M_BIT_LSH       22
#define  CAN_BUF55M_BIT_LSH       23
#define  CAN_BUF56M_BIT_LSH       24
#define  CAN_BUF57M_BIT_LSH       25
#define  CAN_BUF58M_BIT_LSH       26
#define  CAN_BUF59M_BIT_LSH       27
#define  CAN_BUF60M_BIT_LSH       28
#define  CAN_BUF61M_BIT_LSH       29
#define  CAN_BUF62M_BIT_LSH       30
#define  CAN_BUF63M_BIT_LSH       31

// definitions for the bits in the IFLAG registers
#define  CAN_BUF0I_BIT_LSH        0
#define  CAN_BUF1I_BIT_LSH        1
#define  CAN_BUF2I_BIT_LSH        2
#define  CAN_BUF3I_BIT_LSH        3
#define  CAN_BUF4I_BIT_LSH        4
#define  CAN_BUF5I_BIT_LSH        5
#define  CAN_BUF6I_BIT_LSH        6
#define  CAN_BUF7I_BIT_LSH        7
#define  CAN_BUF8I_BIT_LSH        8
#define  CAN_BUF9I_BIT_LSH        9
#define  CAN_BUF10I_BIT_LSH       10
#define  CAN_BUF11I_BIT_LSH       11
#define  CAN_BUF12I_BIT_LSH       12
#define  CAN_BUF13I_BIT_LSH       13
#define  CAN_BUF14I_BIT_LSH       14
#define  CAN_BUF15I_BIT_LSH       15
#define  CAN_BUF16I_BIT_LSH       16
#define  CAN_BUF17I_BIT_LSH       17
#define  CAN_BUF18I_BIT_LSH       18
#define  CAN_BUF19I_BIT_LSH       19
#define  CAN_BUF20I_BIT_LSH       20
#define  CAN_BUF21I_BIT_LSH       21
#define  CAN_BUF22I_BIT_LSH       22
#define  CAN_BUF23I_BIT_LSH       23
#define  CAN_BUF24I_BIT_LSH       24
#define  CAN_BUF25I_BIT_LSH       25
#define  CAN_BUF26I_BIT_LSH       26
#define  CAN_BUF27I_BIT_LSH       27
#define  CAN_BUF28I_BIT_LSH       28
#define  CAN_BUF29I_BIT_LSH       29
#define  CAN_BUF30I_BIT_LSH       30
#define  CAN_BUF31I_BIT_LSH       31

#define  CAN_BUF32I_BIT_LSH       0
#define  CAN_BUF33I_BIT_LSH       1
#define  CAN_BUF34I_BIT_LSH       2
#define  CAN_BUF35I_BIT_LSH       3
#define  CAN_BUF36I_BIT_LSH       4
#define  CAN_BUF37I_BIT_LSH       5
#define  CAN_BUF38I_BIT_LSH       6
#define  CAN_BUF39I_BIT_LSH       7
#define  CAN_BUF40I_BIT_LSH       8
#define  CAN_BUF41I_BIT_LSH       9
#define  CAN_BUF42I_BIT_LSH       10
#define  CAN_BUF43I_BIT_LSH       11
#define  CAN_BUF44I_BIT_LSH       12
#define  CAN_BUF45I_BIT_LSH       13
#define  CAN_BUF46I_BIT_LSH       14
#define  CAN_BUF47I_BIT_LSH       15
#define  CAN_BUF48I_BIT_LSH       16
#define  CAN_BUF49I_BIT_LSH       17
#define  CAN_BUF50I_BIT_LSH       18
#define  CAN_BUF51I_BIT_LSH       19
#define  CAN_BUF52I_BIT_LSH       20
#define  CAN_BUF53I_BIT_LSH       21
#define  CAN_BUF54I_BIT_LSH       22
#define  CAN_BUF55I_BIT_LSH       23
#define  CAN_BUF56I_BIT_LSH       24
#define  CAN_BUF57I_BIT_LSH       25
#define  CAN_BUF58I_BIT_LSH       26
#define  CAN_BUF59I_BIT_LSH       27
#define  CAN_BUF60I_BIT_LSH       28
#define  CAN_BUF61I_BIT_LSH       29
#define  CAN_BUF62I_BIT_LSH       30
#define  CAN_BUF63I_BIT_LSH       31

// definitions for the bits in the MCR register
#define  CAN_MDIS_BIT_WID				1 
#define  CAN_FRZ_BIT_WID					1
#define  CAN_FEN_BIT_WID					1 
#define  CAN_HALT_BIT_WID				1 
#define  CAN_NOT_RDY_BIT_WID			1 
#define  CAN_WAK_MSK_BIT_WID		1  
#define  CAN_SOFT_RST_BIT_WID		1  
#define  CAN_FRZ_ACK_BIT_WID			1 
#define  CAN_SUPV_BIT_WID				1  
#define  CAN_SLF_WAK_BIT_WID			1  
#define  CAN_WARN_EN_BIT_WID			1  
#define  CAN_BIT_LPM_ACK_BIT_WID	1
#define  CAN_WAK_SRC_BIT_WID			1 
#define  CAN_DOZE_BIT_WID				1 
#define  CAN_SRX_DIS_BIT_WID			1  
#define  CAN_BCC_BIT_WID					1  
#define  CAN_PEN_BIT_WID					1  
#define  CAN_AEN_BIT_WID					1 
#define  CAN_IDAM0_BIT_WID				1 
#define  CAN_IDAM1_BIT_WID				1  
#define  CAN_BOFF_MSK_WID				1 
#define  CAN_ERR_MSK_WID				1 
#define  CAN_TWRN_MSK_WID				1 
#define  CAN_RWRN_MSK_WID				1 

// definitions for the bits in the CTRL register
#define  CAN_BOFF_MSK_BIT_WID		1
#define  CAN_ERR_MSK_BIT_WID			1
#define  CAN_CLK_SRC_BIT_WID			1
#define  CAN_BIT_LPB_BIT_WID			1
#define  CAN_TWRN_MSK_BIT_WID		1
#define  CAN_RWRN_MSK_BIT_WID		1
#define  CAN_SMP_BIT_WID					1
#define  CAN_BOFF_REC_BIT_WID		1
#define  CAN_TSYN_BIT_WID				1
#define  CAN_BIT_LBUF_BIT_WID			1
#define  CAN_BIT_LOM_BIT_WID			1

// definitions for the bits in the TCR register
#define  CAN_TRD_BIT_WID          1
#define  CAN_BIT_WIDCLS_BIT_WID   1
#define  CAN_DSCACK_BIT_WID       1

// definitions for the bits in the RX masks registers
#define  CAN_MI0_BIT_WID          1
#define  CAN_MI1_BIT_WID          1
#define  CAN_MI2_BIT_WID          1
#define  CAN_MI3_BIT_WID          1
#define  CAN_MI4_BIT_WID          1
#define  CAN_MI5_BIT_WID          1
#define  CAN_MI6_BIT_WID          1
#define  CAN_MI7_BIT_WID          1
#define  CAN_MI8_BIT_WID          1
#define  CAN_MI9_BIT_WID          1
#define  CAN_MI10_BIT_WID         1
#define  CAN_MI11_BIT_WID         1
#define  CAN_MI12_BIT_WID         1
#define  CAN_MI13_BIT_WID         1
#define  CAN_MI14_BIT_WID         1
#define  CAN_MI15_BIT_WID         1
#define  CAN_MI16_BIT_WID         1
#define  CAN_MI17_BIT_WID         1
#define  CAN_MI18_BIT_WID         1
#define  CAN_MI19_BIT_WID         1
#define  CAN_MI20_BIT_WID         1
#define  CAN_MI21_BIT_WID         1
#define  CAN_MI22_BIT_WID         1
#define  CAN_MI23_BIT_WID         1
#define  CAN_MI24_BIT_WID         1
#define  CAN_MI25_BIT_WID         1
#define  CAN_MI26_BIT_WID         1
#define  CAN_MI27_BIT_WID         1
#define  CAN_MI28_BIT_WID         1
#define  CAN_MI29_BIT_WID         1
#define  CAN_MI30_BIT_WID         1
#define  CAN_MI31_BIT_WID         1

// definitions for the bits/field in the ESR registers
#define  CAN_TWRN_INT_BIT_WID     1
#define  CAN_RWRN_INT_BIT_WID     1   
#define  CAN_BIT_LSH1_ERR_BIT_WID 15
#define  CAN_BIT_LSH0_ERR_BIT_WID 14
#define  CAN_ACK_ERR_BIT_WID      1
#define  CAN_CRC_ERR_BIT_WID      1
#define  CAN_FRM_ERR_BIT_WID      1
#define  CAN_STF_ERR_BIT_WID      1
#define  CAN_TX_WRN_BIT_WID       1
#define  CAN_RX_WRN_BIT_WID       1
#define  CAN_IDLE_BIT_WID         1
#define  CAN_TXRX_BIT_WID         1
#define  CAN_FLT_CONF_FIELD_WID   2
#define  CAN_BOFF_INT_BIT_WID     1
#define  CAN_ERR_INT_BIT_WID      1
#define  CAN_WAK_INT_BIT_WID      1

// definitions for the bits in the IMASK registers
#define  CAN_BUF0M_BIT_WID        1
#define  CAN_BUF1M_BIT_WID        1
#define  CAN_BUF2M_BIT_WID        1
#define  CAN_BUF3M_BIT_WID        1
#define  CAN_BUF4M_BIT_WID        1
#define  CAN_BUF5M_BIT_WID        1
#define  CAN_BUF6M_BIT_WID        1
#define  CAN_BUF7M_BIT_WID        1
#define  CAN_BUF8M_BIT_WID        1
#define  CAN_BUF9M_BIT_WID        1
#define  CAN_BUF10M_BIT_WID       1
#define  CAN_BUF11M_BIT_WID       1
#define  CAN_BUF12M_BIT_WID       1
#define  CAN_BUF13M_BIT_WID       1
#define  CAN_BUF14M_BIT_WID       1
#define  CAN_BUF15M_BIT_WID       1
#define  CAN_BUF16M_BIT_WID       1
#define  CAN_BUF17M_BIT_WID       1
#define  CAN_BUF18M_BIT_WID       1
#define  CAN_BUF19M_BIT_WID       1
#define  CAN_BUF20M_BIT_WID       1
#define  CAN_BUF21M_BIT_WID       1
#define  CAN_BUF22M_BIT_WID       1
#define  CAN_BUF23M_BIT_WID       1
#define  CAN_BUF24M_BIT_WID       1
#define  CAN_BUF25M_BIT_WID       1
#define  CAN_BUF26M_BIT_WID       1
#define  CAN_BUF27M_BIT_WID       1
#define  CAN_BUF28M_BIT_WID       1
#define  CAN_BUF29M_BIT_WID       1
#define  CAN_BUF30M_BIT_WID       1
#define  CAN_BUF31M_BIT_WID       1

#define  CAN_BUF32M_BIT_WID       1
#define  CAN_BUF33M_BIT_WID       1
#define  CAN_BUF34M_BIT_WID       1
#define  CAN_BUF35M_BIT_WID       1
#define  CAN_BUF36M_BIT_WID       1
#define  CAN_BUF37M_BIT_WID       1
#define  CAN_BUF38M_BIT_WID       1
#define  CAN_BUF39M_BIT_WID       1
#define  CAN_BUF40M_BIT_WID       1
#define  CAN_BUF41M_BIT_WID       1
#define  CAN_BUF42M_BIT_WID       1
#define  CAN_BUF43M_BIT_WID       1
#define  CAN_BUF44M_BIT_WID       1
#define  CAN_BUF45M_BIT_WID       1
#define  CAN_BUF46M_BIT_WID       1
#define  CAN_BUF47M_BIT_WID       1
#define  CAN_BUF48M_BIT_WID       1
#define  CAN_BUF49M_BIT_WID       1
#define  CAN_BUF50M_BIT_WID       1
#define  CAN_BUF51M_BIT_WID       1
#define  CAN_BUF52M_BIT_WID       1
#define  CAN_BUF53M_BIT_WID       1
#define  CAN_BUF54M_BIT_WID       1
#define  CAN_BUF55M_BIT_WID       1
#define  CAN_BUF56M_BIT_WID       1
#define  CAN_BUF57M_BIT_WID       1
#define  CAN_BUF58M_BIT_WID       1
#define  CAN_BUF59M_BIT_WID       1
#define  CAN_BUF60M_BIT_WID       1
#define  CAN_BUF61M_BIT_WID       1
#define  CAN_BUF62M_BIT_WID       1
#define  CAN_BUF63M_BIT_WID       1

// definitions for the bits in the IFLAG registers
#define  CAN_BUF0I_BIT_WID        1
#define  CAN_BUF1I_BIT_WID        1
#define  CAN_BUF2I_BIT_WID        1
#define  CAN_BUF3I_BIT_WID        1
#define  CAN_BUF4I_BIT_WID        1
#define  CAN_BUF5I_BIT_WID        1
#define  CAN_BUF6I_BIT_WID        1
#define  CAN_BUF7I_BIT_WID        1
#define  CAN_BUF8I_BIT_WID        1
#define  CAN_BUF9I_BIT_WID        1
#define  CAN_BUF10I_BIT_WID       1
#define  CAN_BUF11I_BIT_WID       1
#define  CAN_BUF12I_BIT_WID       1
#define  CAN_BUF13I_BIT_WID       1
#define  CAN_BUF14I_BIT_WID       1
#define  CAN_BUF15I_BIT_WID       1
#define  CAN_BUF16I_BIT_WID       1
#define  CAN_BUF17I_BIT_WID       1
#define  CAN_BUF18I_BIT_WID       1
#define  CAN_BUF19I_BIT_WID       1
#define  CAN_BUF20I_BIT_WID       1
#define  CAN_BUF21I_BIT_WID       1
#define  CAN_BUF22I_BIT_WID       1
#define  CAN_BUF23I_BIT_WID       1
#define  CAN_BUF24I_BIT_WID       1
#define  CAN_BUF25I_BIT_WID       1
#define  CAN_BUF26I_BIT_WID       1
#define  CAN_BUF27I_BIT_WID       1
#define  CAN_BUF28I_BIT_WID       1
#define  CAN_BUF29I_BIT_WID       1
#define  CAN_BUF30I_BIT_WID       1
#define  CAN_BUF31I_BIT_WID       1

#define  CAN_BUF32I_BIT_WID       1
#define  CAN_BUF33I_BIT_WID       1
#define  CAN_BUF34I_BIT_WID       1
#define  CAN_BUF35I_BIT_WID       1
#define  CAN_BUF36I_BIT_WID       1
#define  CAN_BUF37I_BIT_WID       1
#define  CAN_BUF38I_BIT_WID       1
#define  CAN_BUF39I_BIT_WID       1
#define  CAN_BUF40I_BIT_WID       1
#define  CAN_BUF41I_BIT_WID       1
#define  CAN_BUF42I_BIT_WID       1
#define  CAN_BUF43I_BIT_WID       1
#define  CAN_BUF44I_BIT_WID       1
#define  CAN_BUF45I_BIT_WID       1
#define  CAN_BUF46I_BIT_WID       1
#define  CAN_BUF47I_BIT_WID       1
#define  CAN_BUF48I_BIT_WID       1
#define  CAN_BUF49I_BIT_WID       1
#define  CAN_BUF50I_BIT_WID       1
#define  CAN_BUF51I_BIT_WID       1
#define  CAN_BUF52I_BIT_WID       1
#define  CAN_BUF53I_BIT_WID       1
#define  CAN_BUF54I_BIT_WID       1
#define  CAN_BUF55I_BIT_WID       1
#define  CAN_BUF56I_BIT_WID       1
#define  CAN_BUF57I_BIT_WID       1
#define  CAN_BUF58I_BIT_WID       1
#define  CAN_BUF59I_BIT_WID       1
#define  CAN_BUF60I_BIT_WID       1
#define  CAN_BUF61I_BIT_WID       1
#define  CAN_BUF62I_BIT_WID       1
#define  CAN_BUF63I_BIT_WID       1

#define EXTENDEDMSK			0x1fffffff 
#define STANDARDMSK         0x1ffc0000
//#define STANDARDMSK         0x1fffC000


#define ENABLE  1
#define DISABLE 0

#define ERROCCURED     1
#define ERRNOTOCCURED  0

#define SPECINTOCCURED     1
#define SPECINTNOTOCCURED  0

#ifdef __cplusplus
}
#endif

#endif // __COMMON_UART_H

