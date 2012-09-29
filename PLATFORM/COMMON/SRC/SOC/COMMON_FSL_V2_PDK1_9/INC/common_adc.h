//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#ifndef __COMMON_ADC_H
#define __COMMON_ADC_H

typedef struct {
    REG32 QFIFO;
    REG32 QCR;
    REG32 QSR;
    REG32 QMR;
    unsigned char RESERVED2[0x10];
    REG32 Q_ITEM_7_0;
    REG32 Q_ITEM_15_8;
} T_ADC_QUEUE;
//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {
    REG32 TGCR;
    REG32 TGSR;
    REG32 TICR;
    unsigned char RESERVED1[0x3F4];
    union
    {
        struct {
            REG32 TCQFIFO;
            REG32 TCQCR;
            REG32 TCQSR;
            REG32 TCQMR;
            unsigned char RESERVED2[0x10];
            REG32 TCQ_ITEM_7_0;
            REG32 TCQ_ITEM_15_8;
        } s;
        T_ADC_QUEUE TC_QUEUE;
    }TC_QUEUE_REGS;


    unsigned char RESERVED3[0x18];
    union
    {
        struct {
            REG32 TCC0;
            REG32 TCC1;
            REG32 TCC2;
            REG32 TCC3;
            REG32 TCC4;
            REG32 TCC5;
            REG32 TCC6;
            REG32 TCC7;
        } s;
        REG32 TCC[8];
    }TCC_REGS;
    unsigned char RESERVED4[0x3A0];
    union
    {
        struct {
            REG32 GCQFIFO;
            REG32 GCQCR;
            REG32 GCQSR;
            REG32 GCQMR;
            unsigned char RESERVED5[0x10];
            REG32 GCQ_ITEM_7_0;
            REG32 GCQ_ITEM_15_8;        
        } s;
        T_ADC_QUEUE GC_QUEUE;
    } GC_QUEUE_REGS;

    unsigned char RESERVED6[0x18];
    union
    {
        struct {
            REG32 GCC0;
            REG32 GCC1;
            REG32 GCC2;
            REG32 GCC3;
            REG32 GCC4;
            REG32 GCC5;
            REG32 GCC6;
            REG32 GCC7;
        } s;
        REG32 GCC[8];
    } GCC_REGS;
}CSP_ADC_REGS, *PCSP_ADC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define ADC_TGCR_OFFSET             0x000
#define ADC_TGSR_OFFSET             0x004
#define ADC_TICR_OFFSET             0x008
#define ADC_TCQFIFO_OFFSET          0x400
#define ADC_TCQCR_OFFSET            0x404
#define ADC_TCQSR_OFFSET            0x408
#define ADC_TCQMR_OFFSET            0x40C
#define ADC_TCQ_ITEM_7_0_OFFSET     0x420
#define ADC_TCQ_ITEM_15_8_OFFSET    0x424
#define ADC_TCC0_OFFSET             0x440
#define ADC_TCC1_OFFSET             0x444
#define ADC_TCC2_OFFSET             0x448
#define ADC_TCC3_OFFSET             0x44C
#define ADC_TCC4_OFFSET             0x450
#define ADC_TCC5_OFFSET             0x454
#define ADC_TCC6_OFFSET             0x458
#define ADC_TCC7_OFFSET             0x45C
#define ADC_GCQFIFO_OFFSET          0x800
#define ADC_GCQCR_OFFSET            0x804
#define ADC_GCQSR_OFFSET            0x808
#define ADC_GCQMR_OFFSET            0x80C
#define ADC_GCQ_ITEM_7_0_OFFSET     0x820
#define ADC_GCQ_ITEM_15_8_OFFSET    0x824
#define ADC_GCC0_OFFSET             0x840
#define ADC_GCC1_OFFSET             0x844
#define ADC_GCC2_OFFSET             0x848
#define ADC_GCC3_OFFSET             0x84C
#define ADC_GCC4_OFFSET             0x850
#define ADC_GCC5_OFFSET             0x854
#define ADC_GCC6_OFFSET             0x858
#define ADC_GCC7_OFFSET             0x85C


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
//TGCR TSC General Config Register
#define ADC_TGCR_IPG_CLK_EN_LSH     0
#define ADC_TGCR_TSC_RST_LSH        1
#define ADC_TGCR_FUNC_RST_LSH       2
#define ADC_TGCR_SLPC_LSH           4
#define ADC_TGCR_STLC_LSH           5
#define ADC_TGCR_HSYNCEN_LSH        6
#define ADC_TGCR_HSYNCPOL_LSH       7
#define ADC_TGCR_POWERMODE_LSH      8
#define ADC_TGCR_INTREFEN_LSH       10
#define ADC_TGCR_ADCCLKCFG_LSH      16
#define ADC_TGCR_PDEN_LSH           23
#define ADC_TGCR_PDBEN_LSH          24
#define ADC_TGCR_PDBTIME_LSH        25

//TICR/TCCO/1/2/3 
#define ADC_TCC_PENIACK_LSH             1
#define ADC_TCC_SEL_REFN_LSH            2
#define ADC_TCC_SEL_IN_LSH              4
#define ADC_TCC_SEL_REFP_LSH            7
#define ADC_TCC_XPULSW_LSH              9
#define ADC_TCC_XNURSW_LSH              10
#define ADC_TCC_YPLLSW_LSH              12
#define ADC_TCC_YNLRSW_LSH              14
#define ADC_TCC_WIPERSW_LSH             15
#define ADC_TCC_NOS_LSH                 16
#define ADC_TCC_IGS_LSH                 20
#define ADC_TCC_SETTLING_TIME_LSH       24

//GCCO/1/2/3 
#define ADC_GCC_PENIACK_LSH             1
#define ADC_GCC_SEL_REFN_LSH            2
#define ADC_GCC_SEL_IN_LSH              4
#define ADC_GCC_SEL_REFP_LSH            7
#define ADC_GCC_XPULSW_LSH              9
#define ADC_GCC_XNURSW_LSH              10
#define ADC_GCC_YPLLSW_LSH              12
#define ADC_GCC_YNLRSW_LSH              14
#define ADC_GCC_WIPERSW_LSH             15
#define ADC_GCC_NOS_LSH                 16
#define ADC_GCC_IGS_LSH                 20
#define ADC_GCC_SETTLING_TIME_LSH       24

//TCQ_ITEM_7_0
#define ADC_TCQ_ITEM_0_LSH              0
#define ADC_TCQ_ITEM_1_LSH              4
#define ADC_TCQ_ITEM_2_LSH              8
#define ADC_TCQ_ITEM_3_LSH              12
#define ADC_TCQ_ITEM_4_LSH              16
#define ADC_TCQ_ITEM_5_LSH              20
#define ADC_TCQ_ITEM_6_LSH              24
#define ADC_TCQ_ITEM_7_LSH              28

//TCQ_ITEM_15_8
#define ADC_TCQ_ITEM_8_LSH              0
#define ADC_TCQ_ITEM_9_LSH              4
#define ADC_TCQ_ITEM_10_LSH             8
#define ADC_TCQ_ITEM_11_LSH             12
#define ADC_TCQ_ITEM_12_LSH             16
#define ADC_TCQ_ITEM_13_LSH             20
#define ADC_TCQ_ITEM_14_LSH             24
#define ADC_TCQ_ITEM_15_LSH             28

//GCQ_ITEM_7_0
#define ADC_GCQ_ITEM_0_LSH              0
#define ADC_GCQ_ITEM_1_LSH              4
#define ADC_GCQ_ITEM_2_LSH              8
#define ADC_GCQ_ITEM_3_LSH              12
#define ADC_GCQ_ITEM_4_LSH              16
#define ADC_GCQ_ITEM_5_LSH              20
#define ADC_GCQ_ITEM_6_LSH              24
#define ADC_GCQ_ITEM_7_LSH              28

//GCQ_ITEM_15_8
#define ADC_GCQ_ITEM_8_LSH              0
#define ADC_GCQ_ITEM_9_LSH              4
#define ADC_GCQ_ITEM_10_LSH             8
#define ADC_GCQ_ITEM_11_LSH             12
#define ADC_GCQ_ITEM_12_LSH             16
#define ADC_GCQ_ITEM_13_LSH             20
#define ADC_GCQ_ITEM_14_LSH             24
#define ADC_GCQ_ITEM_15_LSH             28

//TCQCR
#define ADC_TCQCR_QSM_LSH                   0
#define ADC_TCQCR_FQS_LSH                   2
#define ADC_TCQCR_RPT_LSH                   3
#define ADC_TCQCR_LAST_ITEM_ID_LSH          4
#define ADC_TCQCR_FIFOWATERMARK_LSH         8
#define ADC_TCQCR_REPEATWAIT_LSH            12
#define ADC_TCQCR_QRST_LSH                  16
#define ADC_TCQCR_FRST_LSH                  17
#define ADC_TCQCR_PDMSK_LSH                 18
#define ADC_TCQCR_PDCFG_LSH                 19

//GCQCR
#define ADC_GCQCR_QSM_LSH                   0
#define ADC_GCQCR_FQS_LSH                   2
#define ADC_GCQCR_RPT_LSH                   3
#define ADC_GCQCR_LAST_ITEM_ID_LSH          4
#define ADC_GCQCR_FIFOWATERMARK_LSH         8
#define ADC_GCQCR_REPEATWAIT_LSH            12
#define ADC_GCQCR_QRST_LSH                  16
#define ADC_GCQCR_FRST_LSH                  17
#define ADC_GCQCR_PDMSK_LSH                 18
#define ADC_GCQCR_PDCFG_LSH                 19

//TCQMR
#define ADC_TCQMR_PD_IRQ_LSH                0
#define ADC_TCQMR_EOQ_IRQ_LSH               1
#define ADC_TCQMR_FOR_IRQ_LSH               4
#define ADC_TCQMR_FUR_IRQ_LSH               5
#define ADC_TCQMR_FER_IRQ_LSH               6
#define ADC_TCQMR_FDRY_IRQ_LSH              15
#define ADC_TCQMR_PD_DMA_LSH                16
#define ADC_TCQMR_EOQ_DMA_LSH               17
#define ADC_TCQMR_FOR_DMA_LSH               20
#define ADC_TCQMR_FUR_DMA_LSH               21
#define ADC_TCQMR_FER_DMA_LSH               22
#define ADC_TCQMR_FDRY_DMA_LSH              31

//GCQMR
#define ADC_GCQMR_PD_IRQ_LSH                0
#define ADC_GCQMR_EOQ_IRQ_LSH               1
#define ADC_GCQMR_FOR_IRQ_LSH               4
#define ADC_GCQMR_FUR_IRQ_LSH               5
#define ADC_GCQMR_FER_IRQ_LSH               6
#define ADC_GCQMR_FDRY_IRQ_LSH              15
#define ADC_GCQMR_PD_DMA_LSH                16
#define ADC_GCQMR_EOQ_DMA_LSH               17
#define ADC_GCQMR_FOR_DMA_LSH               20
#define ADC_GCQMR_FUR_DMA_LSH               21
#define ADC_GCQMR_FER_DMA_LSH               22
#define ADC_GCQMR_FDRY_DMA_LSH              31

//TCQSR/GCQSR
#define ADC_TCQSR_PD_LSH                    0
#define ADC_TCQSR_EOQ_LSH                   1
#define ADC_TCQSR_FOR_LSH                   4
#define ADC_TCQSR_FUR_LSH                   5
#define ADC_TCQSR_FRR_LSH                   6
#define ADC_TCQSR_FDN_LSH                   8
#define ADC_TCQSR_EMPT_LSH                  13
#define ADC_TCQSR_FULL_LSH                  14
#define ADC_TCQSR_FDRY_LSH                  15
#define ADC_TCQSR_FRPTR_LSH                 16
#define ADC_TCQSR_FWPTR_LSH                 21

//TCQFIFO/GCQFIFO
#define ADC_TCQFIFO_ITEM_ID_LSH             0
#define ADC_TCQFIFO_DATAOUT_LSH             4

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define ADC_TGCR_IPG_CLK_EN_WID         1
#define ADC_TGCR_TSC_RST_WID            1
#define ADC_TGCR_FUNC_RST_WID           1
#define ADC_TGCR_SLPC_WID               1
#define ADC_TGCR_STLC_WID               1
#define ADC_TGCR_HSYNCEN_WID            1
#define ADC_TGCR_HSYNCPOL_WID           1
#define ADC_TGCR_POWERMODE_WID          2
#define ADC_TGCR_INTREFEN_WID           1
#define ADC_TGCR_ADCCLKCFG_WID          5
#define ADC_TGCR_PDEN_WID               1
#define ADC_TGCR_PDBEN_WID              1
#define ADC_TGCR_PDBTIME_WID            7

//TICR/TCCO/1/2/3
#define ADC_TCC_PENIACK_WID             1
#define ADC_TCC_SEL_REFN_WID            2
#define ADC_TCC_SEL_IN_WID              2
#define ADC_TCC_SEL_REFP_WID            2
#define ADC_TCC_XPULSW_WID              1
#define ADC_TCC_XNURSW_WID              2
#define ADC_TCC_YPLLSW_WID              2
#define ADC_TCC_YNLRSW_WID              1
#define ADC_TCC_WIPERSW_WID             1
#define ADC_TCC_NOS_WID                 4
#define ADC_TCC_IGS_WID                 1
#define ADC_TCC_SETTLING_TIME_WID       8

//GCCO/1/2/3
#define ADC_GCC_PENIACK_WID             1
#define ADC_GCC_SEL_REFN_WID            2
#define ADC_GCC_SEL_IN_WID              2
#define ADC_GCC_SEL_REFP_WID            2
#define ADC_GCC_XPULSW_WID              1
#define ADC_GCC_XNURSW_WID              2
#define ADC_GCC_YPLLSW_WID              2
#define ADC_GCC_YNLRSW_WID              1
#define ADC_GCC_WIPERSW_WID             1
#define ADC_GCC_NOS_WID                 4
#define ADC_GCC_IGS_WID                 1
#define ADC_GCC_SETTLING_TIME_WID       8

//TCQ_ITEM_7_0
#define ADC_TCQ_ITEM_0_WID              4
#define ADC_TCQ_ITEM_1_WID              4
#define ADC_TCQ_ITEM_2_WID              4
#define ADC_TCQ_ITEM_3_WID              4
#define ADC_TCQ_ITEM_4_WID              4
#define ADC_TCQ_ITEM_5_WID              4
#define ADC_TCQ_ITEM_6_WID              4
#define ADC_TCQ_ITEM_7_WID              4

//TCQ_ITEM_15_8
#define ADC_TCQ_ITEM_8_WID              4
#define ADC_TCQ_ITEM_9_WID              4
#define ADC_TCQ_ITEM_10_WID             4
#define ADC_TCQ_ITEM_11_WID             4
#define ADC_TCQ_ITEM_12_WID             4
#define ADC_TCQ_ITEM_13_WID             4
#define ADC_TCQ_ITEM_14_WID             4
#define ADC_TCQ_ITEM_15_WID             4

//GCQ_ITEM_7_0
#define ADC_GCQ_ITEM_0_WID              4
#define ADC_GCQ_ITEM_1_WID              4
#define ADC_GCQ_ITEM_2_WID              4
#define ADC_GCQ_ITEM_3_WID              4
#define ADC_GCQ_ITEM_4_WID              4
#define ADC_GCQ_ITEM_5_WID              4
#define ADC_GCQ_ITEM_6_WID              4
#define ADC_GCQ_ITEM_7_WID              4

//GCQ_ITEM_15_8
#define ADC_GCQ_ITEM_8_WID              4
#define ADC_GCQ_ITEM_9_WID              4
#define ADC_GCQ_ITEM_10_WID             4
#define ADC_GCQ_ITEM_11_WID             4
#define ADC_GCQ_ITEM_12_WID             4
#define ADC_GCQ_ITEM_13_WID             4
#define ADC_GCQ_ITEM_14_WID             4
#define ADC_GCQ_ITEM_15_WID             4

//TCQCR/GCQCR
#define ADC_TCQCR_QSM_WID                   2
#define ADC_TCQCR_FQS_WID                   1
#define ADC_TCQCR_RPT_WID                   1
#define ADC_TCQCR_LAST_ITEM_ID_WID          4
#define ADC_TCQCR_FIFOWATERMARK_WID         4
#define ADC_TCQCR_REPEATWAIT_WID            4
#define ADC_TCQCR_QRST_WID                  1
#define ADC_TCQCR_FRST_WID                  1
#define ADC_TCQCR_PDMSK_WID                 1
#define ADC_TCQCR_PDCFG_WID                 1

//GCQCR
#define ADC_GCQCR_QSM_WID                   2
#define ADC_GCQCR_FQS_WID                   1
#define ADC_GCQCR_RPT_WID                   1
#define ADC_GCQCR_LAST_ITEM_ID_WID          4
#define ADC_GCQCR_FIFOWATERMARK_WID         4
#define ADC_GCQCR_REPEATWAIT_WID            4
#define ADC_GCQCR_QRST_WID                  1
#define ADC_GCQCR_FRST_WID                  1
#define ADC_GCQCR_PDMSK_WID                 1
#define ADC_GCQCR_PDCFG_WID                 1

//TCQMR
#define ADC_TCQMR_PD_IRQ_WID                1
#define ADC_TCQMR_EOQ_IRQ_WID               1
#define ADC_TCQMR_FOR_IRQ_WID               1
#define ADC_TCQMR_FUR_IRQ_WID               1   
#define ADC_TCQMR_FER_IRQ_WID               1
#define ADC_TCQMR_FDRY_IRQ_WID              1
#define ADC_TCQMR_PD_DMA_WID                1
#define ADC_TCQMR_EOQ_DMA_WID               1
#define ADC_TCQMR_FOR_DMA_WID               1
#define ADC_TCQMR_FUR_DMA_WID               1
#define ADC_TCQMR_FER_DMA_WID               1
#define ADC_TCQMR_FDRY_DMA_WID              1

//GCQMR
#define ADC_GCQMR_PD_IRQ_WID                1
#define ADC_GCQMR_EOQ_IRQ_WID               1
#define ADC_GCQMR_FOR_IRQ_WID               1
#define ADC_GCQMR_FUR_IRQ_WID               1   
#define ADC_GCQMR_FER_IRQ_WID               1
#define ADC_GCQMR_FDRY_IRQ_WID              1
#define ADC_GCQMR_PD_DMA_WID                1
#define ADC_GCQMR_EOQ_DMA_WID               1
#define ADC_GCQMR_FOR_DMA_WID               1
#define ADC_GCQMR_FUR_DMA_WID               1
#define ADC_GCQMR_FER_DMA_WID               1
#define ADC_GCQMR_FDRY_DMA_WID              1

//TCQSR/GCQSR
#define ADC_TCQSR_PD_WID                    1
#define ADC_TCQSR_EOQ_WID                   1
#define ADC_TCQSR_FOR_WID                   1
#define ADC_TCQSR_FUR_WID                   1
#define ADC_TCQSR_FRR_WID                   1
#define ADC_TCQSR_FDN_WID                   5
#define ADC_TCQSR_EMPT_WID                  1
#define ADC_TCQSR_FULL_WID                  1
#define ADC_TCQSR_FDRY_WID                  1
#define ADC_TCQSR_FRPTR_WID                 5
#define ADC_TCQSR_FWPTR_WID                 5

//TCQFIFO/GCQFIFO
#define ADC_TCQFIFO_ITEM_ID_WID             4
#define ADC_TCQFIFO_DATAOUT_WID             12



//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
//TGCR TSC General Config Register
#define ADC_TGCR_IPG_ON                 1
#define ADC_TGCR_IPG_OFF                0
#define ADC_TGCR_TSC_RESET              1
#define ADC_TGCR_SETTLING_NOT_LACTHED   0
#define ADC_TGCR_SETTLING_LACTHED       1
#define ADC_TGCR_HSYNC_FILTER_OFF       0
#define ADC_TGCR_HSYNC_FILTER_ON        1
#define ADC_TGCR_HSYNC_POLARITY_HIGH    0
#define ADC_TGCR_HSYNC_POLARITY_LOW     1
#define ADC_TGCR_POWER_ALWAYS_OFF       0
#define ADC_TGCR_POWER_SAVING           1
#define ADC_TGCR_POWER_ALWAYS_ON        2
#define ADC_TGCR_FUNC_RESET             1
#define ADC_TGCR_PDEN_ENABLE            1
#define ADC_TGCR_PDBEN_ENABLE           1
#define ADC_TGCR_INTREFEN_ON            1
#define ADC_TGCR_INTREFEN_OFF           0

//TICR/TCCO/1/2/3
#define ADC_TCC_PENIACK_ENABLE          1
#define ADC_TCC_PENIACK_DISABLE         0
#define ADC_TCC_SEL_REFN_XNUR           0
#define ADC_TCC_SEL_REFN_YNLR           1
#define ADC_TCC_SEL_REFN_AGND           2
#define ADC_TCC_SEL_IN_XPUL             0
#define ADC_TCC_SEL_IN_YPLL             1
#define ADC_TCC_SEL_IN_XNUR             2
#define ADC_TCC_SEL_IN_YNLR             3
#define ADC_TCC_SEL_IN_WIPER            4
#define ADC_TCC_SEL_IN_INAUX0           5
#define ADC_TCC_SEL_IN_INAUX1           6
#define ADC_TCC_SEL_IN_INAUX2           7
#define ADC_TCC_SEL_REFP_YPLL           0
#define ADC_TCC_SEL_REFP_XPUL           1
#define ADC_TCC_SEL_REFP_EXT_REF        2
#define ADC_TCC_SEL_REFP_INT_REF        3
#define ADC_TCC_XPULSW_HIGH             0
#define ADC_TCC_XPULSW_OFF              1
#define ADC_TCC_XNURSW_HIGH             0
#define ADC_TCC_XNURSW_OFF              1
#define ADC_TCC_XNURSW_LOW              3
#define ADC_TCC_YPLLSW_HIGH             0
#define ADC_TCC_YPLLSW_OFF              1
#define ADC_TCC_YPLLSW_LOW              3
#define ADC_TCC_YNLRSW_OFF              0
#define ADC_TCC_YNLRSW_LOW              1
#define ADC_TCC_WIPERSW_OFF             0
#define ADC_TCC_WIPERSW_LOW             1
#define ADC_TCC_NOS(x)                  ((x <= 15) && (x >= 0))?(x):(0)
#define ADC_TCC_IGS_IGNORE_ON           1
#define ADC_TCC_IGS_IGNORE_OFF          0
#define ADC_TCC_SETTLING_TIME(x)        ((x <= 255) && (x >= 0))?(x):(0)

//GCCO/1/2/3
#define ADC_GCC_PENIACK_ENABLE          1
#define ADC_GCC_PENIACK_DISABLE         0
#define ADC_GCC_SEL_REFN_XNUR           0
#define ADC_GCC_SEL_REFN_YNLR           1
#define ADC_GCC_SEL_REFN_AGND           2
#define ADC_GCC_SEL_IN_XPUL             0
#define ADC_GCC_SEL_IN_YPLL             1
#define ADC_GCC_SEL_IN_XNUR             2
#define ADC_GCC_SEL_IN_YNLR             3
#define ADC_GCC_SEL_IN_WIPER            4
#define ADC_GCC_SEL_IN_INAUX0           5
#define ADC_GCC_SEL_IN_INAUX1           6
#define ADC_GCC_SEL_IN_INAUX2           7
#define ADC_GCC_SEL_REFP_YPLL           0
#define ADC_GCC_SEL_REFP_XPUL           1
#define ADC_GCC_SEL_REFP_EXT_REF        2
#define ADC_GCC_SEL_REFP_INT_REF        3
#define ADC_GCC_XPULSW_HIGH             0
#define ADC_GCC_XPULSW_OFF              1
#define ADC_GCC_XNURSW_HIGH             0
#define ADC_GCC_XNURSW_OFF              1
#define ADC_GCC_XNURSW_LOW              3
#define ADC_GCC_YPLLSW_HIGH             0
#define ADC_GCC_YPLLSW_OFF              1
#define ADC_GCC_YPLLSW_LOW              3
#define ADC_GCC_YNLRSW_OFF              0
#define ADC_GCC_YNLRSW_LOW              1
#define ADC_GCC_WIPERSW_OFF             0
#define ADC_GCC_WIPERSW_LOW             1
#define ADC_GCC_NOS(x)                  (((x) > 15) ? 15 : ((x) < 0) ? 0 :(x))
#define ADC_GCC_IGS_IGNORE_ON           1
#define ADC_GCC_IGS_IGNORE_OFF          0
#define ADC_GCC_SETTLING_TIME(x)        (((x) > 255) ? 255 : ((x) < 0) ? 0 :(x))
//TCQ_ITEM_7_0/TCQ_ITEM_15_8
#define ADC_TCQ_ITEM_TCC0               0
#define ADC_TCQ_ITEM_TCC1               1
#define ADC_TCQ_ITEM_TCC2               2
#define ADC_TCQ_ITEM_TCC3               3
#define ADC_TCQ_ITEM_TCC4               4
#define ADC_TCQ_ITEM_TCC5               5
#define ADC_TCQ_ITEM_TCC6               6
#define ADC_TCQ_ITEM_TCC7               7
#define ADC_TCQ_ITEM_GCC7               8
#define ADC_TCQ_ITEM_GCC6               9
#define ADC_TCQ_ITEM_GCC5               10
#define ADC_TCQ_ITEM_GCC4               11
#define ADC_TCQ_ITEM_GCC3               12
#define ADC_TCQ_ITEM_GCC2               13
#define ADC_TCQ_ITEM_GCC1               14
#define ADC_TCQ_ITEM_GCC0               15

//GCQ_ITEM_7_0/GCQ_ITEM_15_8
#define ADC_GCQ_ITEM_GCC0               0
#define ADC_GCQ_ITEM_GCC1               1
#define ADC_GCQ_ITEM_GCC2               2
#define ADC_GCQ_ITEM_GCC3               3
#define ADC_GCQ_ITEM_GCC4               4
#define ADC_GCQ_ITEM_GCC5               5
#define ADC_GCQ_ITEM_GCC6               6
#define ADC_GCQ_ITEM_GCC7               7
#define ADC_GCQ_ITEM_TCC7               8
#define ADC_GCQ_ITEM_TCC6               9
#define ADC_GCQ_ITEM_TCC5               10
#define ADC_GCQ_ITEM_TCC4               11
#define ADC_GCQ_ITEM_TCC3               12
#define ADC_GCQ_ITEM_TCC2               13
#define ADC_GCQ_ITEM_TCC1               14
#define ADC_GCQ_ITEM_TCC0               15


//TCQCR
#define ADC_TCQCR_QSM_STOP              0
#define ADC_TCQCR_QSM_PEN               1
#define ADC_TCQCR_QSM_FQS               2
#define ADC_TCQCR_QSM_PEN_FQS           3
#define ADC_TCQCR_FQS_START             1
#define ADC_TCQCR_RPT_REPEAT            1
#define ADC_TCQCR_QRST_QRESET           1
#define ADC_TCQCR_FRST_FRESET           1
#define ADC_TCQCR_PDMASK_MASK           1
#define ADC_TCQCR_PDMASK_UNMASK         0
#define ADC_TCQCR_PDCFG_LEVEL           1
#define ADC_TCQCR_PDCFG_EDGE            0

//GCQCR
#define ADC_GCQCR_QSM_STOP              0
#define ADC_GCQCR_QSM_PEN               1
#define ADC_GCQCR_QSM_FQS               2
#define ADC_GCQCR_QSM_PEN_FQS           3
#define ADC_GCQCR_FQS_START             1
#define ADC_GCQCR_RPT_REPEAT            1
#define ADC_GCQCR_QRST_QRESET           1
#define ADC_GCQCR_FRST_FRESET           1
#define ADC_GCQCR_PDMASK_MASK           1
#define ADC_GCQCR_PDMASK_UNMASK         0
#define ADC_GCQCR_PDCFG_LEVEL           1
#define ADC_GCQCR_PDCFG_EDGE            0

//TCQMR/GCQMR
#define ADC_TCQMR_INT_MASK              1
#define ADC_TCQMR_INT_UNMASK            0

#endif //__COMMON_ADC_H
