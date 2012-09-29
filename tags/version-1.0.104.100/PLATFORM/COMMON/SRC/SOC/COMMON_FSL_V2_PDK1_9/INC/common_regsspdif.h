//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: comon_regsspdif.h
//  PIO Registers for SPDIF interface
//------------------------------------------------------------------------------
// WARNING!  THIS FILE IS AUTOMATICALLY GENERATED FROM XML.
//                DO NOT MODIFY THIS FILE DIRECTLY.
//
//------------------------------------------------------------------------------
//
// The following naming conventions are followed in this file.
//      XX_<module>_<regname>_<field>
//
// XX specifies the define / macro class
//      HW pertains to a register
//      BM indicates a Bit Mask
//      BF indicates a Bit Field macro
//
// <module> is the hardware module name which can be any of the following...
//      USB20 (Note when there is more than one copy of a given module, the
//      module name includes a number starting from 0 for the first instance
//      of that module)
//
// <regname> is the specific register within that module
//
// <field> is the specific bitfield within that <module>_<register>
//
// We also define the following...
//      hw_<module>_<regname>_t is typedef of anonymous union
//
//------------------------------------------------------------------------------

#ifndef _COMMON_REGSSPDIF_H
#define _COMMON_REGSSPDIF_H  1

#ifndef REGS_SPDIF_BASE
#define REGS_SPDIF_BASE (DWORD)m_pSpdifReg
#endif


typedef struct 
{
    UINT32 CTRL;
    UINT32 STAT;
    UINT32 FRAMECTRL;
    UINT32 SRR;
    UINT32 DEBUGREG;
    UINT32 DATA;
    UINT32 VERSION;
} SPDIF_REG, *PSPDIF_REG;

//------------------------------------------------------------------------------
//  HW_SPDIF_CTRL - SPDIF Control Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned RUN                 :  1;
        unsigned FIFO_ERROR_IRQ_EN   :  1;
        unsigned FIFO_OVERFLOW_IRQ   :  1;
        unsigned FIFO_UNDERFLOW_IRQ  :  1;
        unsigned WORD_LENGTH         :  1;
        unsigned WAIT_END_XFER       :  1;
        unsigned RSRVD0              : 10;
        unsigned DMAWAIT_COUNT       :  5;
        unsigned RSRVD1              :  9;
        unsigned CLKGATE             :  1;
        unsigned SFTRST              :  1;
    } B;
} hw_spdif_ctrl_t;
#endif


// constants & macros for entire HW_SPDIF_CTRL register

#define HW_SPDIF_CTRL_ADDR            (REGS_SPDIF_BASE + 0x00000000)
#define HW_SPDIF_CTRL_SET_ADDR        (REGS_SPDIF_BASE + 0x00000004)
#define HW_SPDIF_CTRL_CLR_ADDR        (REGS_SPDIF_BASE + 0x00000008)
#define HW_SPDIF_CTRL_TOG_ADDR        (REGS_SPDIF_BASE + 0x0000000C)

#ifndef __LANGUAGE_ASM__
#define HW_SPDIF_CTRL                 (*(volatile hw_spdif_ctrl_t *) HW_SPDIF_CTRL_ADDR)
#define HW_SPDIF_CTRL_RD()            (HW_SPDIF_CTRL.U)
#define HW_SPDIF_CTRL_WR(v)           (HW_SPDIF_CTRL.U = (v))
#define HW_SPDIF_CTRL_SET(v)          ((*(volatile reg32_t *) HW_SPDIF_CTRL_SET_ADDR) = (v))
#define HW_SPDIF_CTRL_CLR(v)          ((*(volatile reg32_t *) HW_SPDIF_CTRL_CLR_ADDR) = (v))
#define HW_SPDIF_CTRL_TOG(v)          ((*(volatile reg32_t *) HW_SPDIF_CTRL_TOG_ADDR) = (v))
#endif


// constants & macros for individual HW_SPDIF_CTRL bitfields

// Register HW_SPDIF_CTRL, field SFTRST
#define BP_SPDIF_CTRL_SFTRST           31
#define BM_SPDIF_CTRL_SFTRST           0x80000000
#ifndef __LANGUAGE_ASM__
#define BF_SPDIF_CTRL_SFTRST(v)        ((((reg32_t) v) << 31) & BM_SPDIF_CTRL_SFTRST)
#else
#define BF_SPDIF_CTRL_SFTRST(v)        (((v) << 31) & BM_SPDIF_CTRL_SFTRST)
#endif

//  Register HW_SPDIF_CTRL, field CLKGATE
#define BP_SPDIF_CTRL_CLKGATE          30
#define BM_SPDIF_CTRL_CLKGATE          0x40000000
#define BF_SPDIF_CTRL_CLKGATE(v)       (((v) << 30) & BM_SPDIF_CTRL_CLKGATE)

//  Register HW_SPDIF_CTRL, field RSRVD1
#define BP_SPDIF_CTRL_RSRVD1           21
#define BM_SPDIF_CTRL_RSRVD1           0x3FE00000
#define BF_SPDIF_CTRL_RSRVD1(v)        (((v) << 21) & BM_SPDIF_CTRL_RSRVD1)

//  Register HW_SPDIF_CTRL, field DMAWAIT_COUNT
#define BP_SPDIF_CTRL_DMAWAIT_COUNT    16
#define BM_SPDIF_CTRL_DMAWAIT_COUNT    0x001F0000
#define BF_SPDIF_CTRL_DMAWAIT_COUNT(v) (((v) << 16) & BM_SPDIF_CTRL_DMAWAIT_COUNT)

//  Register HW_SPDIF_CTRL, field RSRVD0
#define BP_SPDIF_CTRL_RSRVD0           6
#define BM_SPDIF_CTRL_RSRVD0           0x0000FFC0
#define BF_SPDIF_CTRL_RSRVD0(v)        (((v) << 6) & BM_SPDIF_CTRL_RSRVD0)

//  Register HW_SPDIF_CTRL, field WAIT_END_XFER
#define BP_SPDIF_CTRL_WAIT_END_XFER    5
#define BM_SPDIF_CTRL_WAIT_END_XFER    0x00000020
#define BF_SPDIF_CTRL_WAIT_END_XFER(v) (((v) << 5) & BM_SPDIF_CTRL_WAIT_END_XFER)

//  Register HW_SPDIF_CTRL, field WORD_LENGTH
#define BP_SPDIF_CTRL_WORD_LENGTH      4
#define BM_SPDIF_CTRL_WORD_LENGTH      0x00000010
#define BF_SPDIF_CTRL_WORD_LENGTH(v)   (((v) << 4) & BM_SPDIF_CTRL_WORD_LENGTH)

//  Register HW_SPDIF_CTRL, field FIFO_UNDERFLOW_IRQ
#define BP_SPDIF_CTRL_FIFO_UNDERFLOW_IRQ    3
#define BM_SPDIF_CTRL_FIFO_UNDERFLOW_IRQ    0x00000008
#define BF_SPDIF_CTRL_FIFO_UNDERFLOW_IRQ(v) (((v) << 3) & BM_SPDIF_CTRL_FIFO_UNDERFLOW_IRQ)

//  Register HW_SPDIF_CTRL, field FIFO_OVERFLOW_IRQ
#define BP_SPDIF_CTRL_FIFO_OVERFLOW_IRQ     2
#define BM_SPDIF_CTRL_FIFO_OVERFLOW_IRQ     0x00000004
#define BF_SPDIF_CTRL_FIFO_OVERFLOW_IRQ(v)  (((v) << 2) & BM_SPDIF_CTRL_FIFO_OVERFLOW_IRQ)

//  Register HW_SPDIF_CTRL, field FIFO_ERROR_IRQ_EN
#define BP_SPDIF_CTRL_FIFO_ERROR_IRQ_EN     1
#define BM_SPDIF_CTRL_FIFO_ERROR_IRQ_EN     0x00000002
#define BF_SPDIF_CTRL_FIFO_ERROR_IRQ_EN(v)  (((v) << 1) & BM_SPDIF_CTRL_FIFO_ERROR_IRQ_EN)

//  Register HW_SPDIF_CTRL, field RUN
#define BP_SPDIF_CTRL_RUN                   0
#define BM_SPDIF_CTRL_RUN                   0x00000001
#define BF_SPDIF_CTRL_RUN(v)                (((v) << 0) & BM_SPDIF_CTRL_RUN)


//------------------------------------------------------------------------------
//  HW_SPDIF_STAT - SPDIF Status Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t U;
    struct
    {
        unsigned END_XFER  :  1;
        unsigned RSRVD1    : 30;
        unsigned PRESENT   :  1;
    } B;
} hw_spdif_stat_t;
#endif


// constants & macros for entire HW_SPDIF_STAT register
#define HW_SPDIF_STAT_ADDR          (REGS_SPDIF_BASE + 0x00000010)

#ifndef __LANGUAGE_ASM__
#define HW_SPDIF_STAT               (*(volatile hw_spdif_stat_t *) HW_SPDIF_STAT_ADDR)
#define HW_SPDIF_STAT_RD()          (HW_SPDIF_STAT.U)
#endif


// constants & macros for individual HW_SPDIF_STAT bitfields

//  Register HW_SPDIF_STAT, field PRESENT
#define BP_SPDIF_STAT_PRESENT       31
#define BM_SPDIF_STAT_PRESENT       0x80000000
#ifndef __LANGUAGE_ASM__
#define BF_SPDIF_STAT_PRESENT(v)    ((((reg32_t) v) << 31) & BM_SPDIF_STAT_PRESENT)
#else
#define BF_SPDIF_STAT_PRESENT(v)    (((v) << 31) & BM_SPDIF_STAT_PRESENT)
#endif

// --- Register HW_SPDIF_STAT, field RSRVD1
#define BP_SPDIF_STAT_RSRVD1        1
#define BM_SPDIF_STAT_RSRVD1        0x7FFFFFFE
#define BF_SPDIF_STAT_RSRVD1(v)     (((v) << 1) & BM_SPDIF_STAT_RSRVD1)

// --- Register HW_SPDIF_STAT, field END_XFER
#define BP_SPDIF_STAT_END_XFER      0
#define BM_SPDIF_STAT_END_XFER      0x00000001
#define BF_SPDIF_STAT_END_XFER(v)   (((v) << 0) & BM_SPDIF_STAT_END_XFER)


//------------------------------------------------------------------------------
//  HW_SPDIF_FRAMECTRL - SPDIF Frame Control Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned PRO        :  1;
        unsigned AUDIO      :  1;
        unsigned COPY       :  1;
        unsigned PRE        :  1;
        unsigned CC         :  7;
        unsigned RSRVD0     :  1;
        unsigned L          :  1;
        unsigned V          :  1;
        unsigned USER_DATA  :  1;
        unsigned RSRVD1     :  1;
        unsigned AUTO_MUTE  :  1;
        unsigned V_CONFIG   :  1;
        unsigned RSRVD2     : 14;
    } B;
} hw_spdif_framectrl_t;
#endif


// constants & macros for entire HW_SPDIF_FRAMECTRL register
#define HW_SPDIF_FRAMECTRL_ADDR         (REGS_SPDIF_BASE + 0x20)
#define HW_SPDIF_FRAMECTRL_SET_ADDR     (HW_SPDIF_FRAMECTRL_ADDR + 4)
#define HW_SPDIF_FRAMECTRL_CLR_ADDR     (HW_SPDIF_FRAMECTRL_ADDR + 8)
#define HW_SPDIF_FRAMECTRL_TOG_ADDR     (HW_SPDIF_FRAMECTRL_ADDR + 12)

#ifndef __LANGUAGE_ASM__
#define HW_SPDIF_FRAMECTRL              (*(volatile hw_spdif_framectrl_t *) HW_SPDIF_FRAMECTRL_ADDR)
#define HW_SPDIF_FRAMECTRL_RD()         (HW_SPDIF_FRAMECTRL.U)
#define HW_SPDIF_FRAMECTRL_WR(v)        (HW_SPDIF_FRAMECTRL.U = (v))
#define HW_SPDIF_FRAMECTRL_SET(v)       ((*(volatile reg32_t *) HW_SPDIF_FRAMECTRL_SET_ADDR) = (v))
#define HW_SPDIF_FRAMECTRL_CLR(v)       ((*(volatile reg32_t *) HW_SPDIF_FRAMECTRL_CLR_ADDR) = (v))
#define HW_SPDIF_FRAMECTRL_TOG(v)       ((*(volatile reg32_t *) HW_SPDIF_FRAMECTRL_TOG_ADDR) = (v))
#endif


// constants & macros for individual HW_SPDIF_FRAMECTRL bitfields

//  Register HW_SPDIF_FRAMECTRL, field V_CONFIG
#define BP_SPDIF_FRAMECTRL_V_CONFIG       17
#define BM_SPDIF_FRAMECTRL_V_CONFIG       0x00020000
#define BF_SPDIF_FRAMECTRL_V_CONFIG(v)    (((v) << 17) & BM_SPDIF_FRAMECTRL_V_CONFIG)

//  Register HW_SPDIF_FRAMECTRL, field AUTO_MUTE
#define BP_SPDIF_FRAMECTRL_AUTO_MUTE      16
#define BM_SPDIF_FRAMECTRL_AUTO_MUTE      0x00010000
#define BF_SPDIF_FRAMECTRL_AUTO_MUTE(v)   (((v) << 16) & BM_SPDIF_FRAMECTRL_AUTO_MUTE)

//  Register HW_SPDIF_FRAMECTRL, field RSRVD1
#define BP_SPDIF_FRAMECTRL_RSRVD1         15
#define BM_SPDIF_FRAMECTRL_RSRVD1         0x00008000
#define BF_SPDIF_FRAMECTRL_RSRVD1(v)      (((v) << 15) & BM_SPDIF_FRAMECTRL_RSRVD1)

//  Register HW_SPDIF_FRAMECTRL, field USER_DATA
#define BP_SPDIF_FRAMECTRL_USER_DATA      14
#define BM_SPDIF_FRAMECTRL_USER_DATA      0x00004000
#define BF_SPDIF_FRAMECTRL_USER_DATA(v)   (((v) << 14) & BM_SPDIF_FRAMECTRL_USER_DATA)

//  Register HW_SPDIF_FRAMECTRL, field V
#define BP_SPDIF_FRAMECTRL_V              13
#define BM_SPDIF_FRAMECTRL_V              0x00002000
#define BF_SPDIF_FRAMECTRL_V(v)           (((v) << 13) & BM_SPDIF_FRAMECTRL_V)

//  Register HW_SPDIF_FRAMECTRL, field L
#define BP_SPDIF_FRAMECTRL_L              12
#define BM_SPDIF_FRAMECTRL_L              0x00001000
#define BF_SPDIF_FRAMECTRL_L(v)           (((v) << 12) & BM_SPDIF_FRAMECTRL_L)

//  Register HW_SPDIF_FRAMECTRL, field RSRVD0
#define BP_SPDIF_FRAMECTRL_RSRVD0         11
#define BM_SPDIF_FRAMECTRL_RSRVD0         0x00000800
#define BF_SPDIF_FRAMECTRL_SRVD0(v)       (((v) << 11) & BM_SPDIF_FRAMECTRL_RSRVD0)

//  Register HW_SPDIF_FRAMECTRL, field CC
#define BP_SPDIF_FRAMECTRL_CC             4
#define BM_SPDIF_FRAMECTRL_CC             0x000007F0
#define BF_SPDIF_FRAMECTRL_CC(v)          (((v) << 4) & BM_SPDIF_FRAMECTRL_CC)

// Register HW_SPDIF_FRAMECTRL, field PRE
#define BP_SPDIF_FRAMECTRL_PRE            3
#define BM_SPDIF_FRAMECTRL_PRE            0x00000008
#define BF_SPDIF_FRAMECTRL_PRE(v)         (((v) << 3) & BM_SPDIF_FRAMECTRL_PRE)

//  Register HW_SPDIF_FRAMECTRL, field COPY
#define BP_SPDIF_FRAMECTRL_COPY           2
#define BM_SPDIF_FRAMECTRL_COPY           0x00000004
#define BF_SPDIF_FRAMECTRL_COPY(v)        (((v) << 2) & BM_SPDIF_FRAMECTRL_COPY)

//  Register HW_SPDIF_FRAMECTRL, field AUDIO
#define BP_SPDIF_FRAMECTRL_AUDIO          1
#define BM_SPDIF_FRAMECTRL_AUDIO          0x00000002
#define BF_SPDIF_FRAMECTRL_AUDIO(v)       (((v) << 1) & BM_SPDIF_FRAMECTRL_AUDIO)

//  Register HW_SPDIF_FRAMECTRL, field PRO
#define BP_SPDIF_FRAMECTRL_PRO            0
#define BM_SPDIF_FRAMECTRL_PRO            0x00000001
#define BF_SPDIF_FRAMECTRL_PRO(v)         (((v) << 0) & BM_SPDIF_FRAMECTRL_PRO)


//------------------------------------------------------------------------------
//  HW_SPDIF_SRR - SPDIF Sample Rate Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned RATE      : 20;
        unsigned RSRVD0    :  8;
        unsigned BASEMULT  :  3;
        unsigned RSRVD1    :  1;
    } B;
} hw_spdif_srr_t;
#endif


// constants & macros for entire HW_SPDIF_SRR register
#define HW_SPDIF_SRR_ADDR         (REGS_SPDIF_BASE + 0x30)
#define HW_SPDIF_SRR_SET_ADDR     (HW_SPDIF_SRR_ADDR + 4)
#define HW_SPDIF_SRR_CLR_ADDR     (HW_SPDIF_SRR_ADDR + 8)
#define HW_SPDIF_SRR_TOG_ADDR     (HW_SPDIF_SRR_ADDR + 12)

#ifndef __LANGUAGE_ASM__
#define HW_SPDIF_SRR              (*(volatile hw_spdif_srr_t *) HW_SPDIF_SRR_ADDR)
#define HW_SPDIF_SRR_RD()         (HW_SPDIF_SRR.U)
#define HW_SPDIF_SRR_WR(v)        (HW_SPDIF_SRR.U = (v))
#define HW_SPDIF_SRR_SET(v)       ((*(volatile reg32_t *) HW_SPDIF_SRR_SET_ADDR) = (v))
#define HW_SPDIF_SRR_CLR(v)       ((*(volatile reg32_t *) HW_SPDIF_SRR_CLR_ADDR) = (v))
#define HW_SPDIF_SRR_TOG(v)       ((*(volatile reg32_t *) HW_SPDIF_SRR_TOG_ADDR) = (v))
#endif


// constants & macros for individual HW_SPDIF_SRR bitfields
#define BP_SPDIF_SRR_RSRVD1       31
#define BM_SPDIF_SRR_RSRVD1       0x80000000
#ifndef __LANGUAGE_ASM__
#define BF_SPDIF_SRR_RSRVD1(v)    ((((reg32_t) v) << 31) & BM_SPDIF_SRR_RSRVD1)
#else
#define BF_SPDIF_SRR_RSRVD1(v)    (((v) << 31) & BM_SPDIF_SRR_RSRVD1)
#endif

//  Register HW_SPDIF_SRR, field BASEMULT
#define BP_SPDIF_SRR_BASEMULT     28
#define BM_SPDIF_SRR_BASEMULT     0x70000000
#define BF_SPDIF_SRR_BASEMULT(v)  (((v) << 28) & BM_SPDIF_SRR_BASEMULT)

//  Register HW_SPDIF_SRR, field RSRVD0
#define BP_SPDIF_SRR_RSRVD0       20
#define BM_SPDIF_SRR_RSRVD0       0x0FF00000
#define BF_SPDIF_SRR_RSRVD0(v)    (((v) << 20) & BM_SPDIF_SRR_RSRVD0)

// --- Register HW_SPDIF_SRR, field RATE
#define BP_SPDIF_SRR_RATE         0
#define BM_SPDIF_SRR_RATE         0x000FFFFF
#define BF_SPDIF_SRR_RATE(v)      (((v) << 0) & BM_SPDIF_SRR_RATE)


//------------------------------------------------------------------------------
//  HW_SPDIF_DEBUG - SPDIF Debug Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t U;
    struct
    {
        unsigned FIFO_STATUS  :  1;
        unsigned DMA_PREQ     :  1;
        unsigned RSRVD1       : 30;
    } B;
} hw_spdif_debug_t;
#endif

// constants & macros for entire HW_SPDIF_DEBUG register
#define HW_SPDIF_DEBUG_ADDR         (REGS_SPDIF_BASE + 0x40)

#ifndef __LANGUAGE_ASM__
#define HW_SPDIF_DEBUG              (*(volatile hw_spdif_debug_t *) HW_SPDIF_DEBUG_ADDR)
#define HW_SPDIF_DEBUG_RD()         (HW_SPDIF_DEBUG.U)
#endif

// constants & macros for individual HW_SPDIF_DEBUG bitfields

//  Register HW_SPDIF_DEBUG, field RSRVD1
#define BP_SPDIF_DEBUG_RSRVD1       2
#define BM_SPDIF_DEBUG_RSRVD1       0xFFFFFFFC
#ifndef __LANGUAGE_ASM__
#define BF_SPDIF_DEBUG_RSRVD1(v)    ((((reg32_t) v) << 2) & BM_SPDIF_DEBUG_RSRVD1)
#else
#define BF_SPDIF_DEBUG_RSRVD1(v)    (((v) << 2) & BM_SPDIF_DEBUG_RSRVD1)
#endif

//  Register HW_SPDIF_DEBUG, field DMA_PREQ
#define BP_SPDIF_DEBUG_DMA_PREQ     1
#define BM_SPDIF_DEBUG_DMA_PREQ     0x00000002
#define BF_SPDIF_DEBUG_DMA_PREQ(v)  (((v) << 1) & BM_SPDIF_DEBUG_DMA_PREQ)

//  Register HW_SPDIF_DEBUG, field FIFO_STATUS
#define BP_SPDIF_DEBUG_FIFO_STATUS  0
#define BM_SPDIF_DEBUG_FIFO_STATUS  0x00000001
#define BF_SPDIF_DEBUG_FIFO_STATUS(v) (((v) << 0) & BM_SPDIF_DEBUG_FIFO_STATUS)


//------------------------------------------------------------------------------
//  HW_SPDIF_DATA - SPDIF Write Data Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned LOW   : 16;
        unsigned HIGH  : 16;
    } B;
} hw_spdif_data_t;
#endif


// constants & macros for entire HW_SPDIF_DATA register
#define HW_SPDIF_DATA_ADDR         (REGS_SPDIF_BASE + 0x50)
#define HW_SPDIF_DATA_SET_ADDR     (HW_SPDIF_DATA_ADDR + 4)
#define HW_SPDIF_DATA_CLR_ADDR     (HW_SPDIF_DATA_ADDR + 8)
#define HW_SPDIF_DATA_TOG_ADDR     (HW_SPDIF_DATA_ADDR + 12)

#ifndef __LANGUAGE_ASM__
#define HW_SPDIF_DATA              (*(volatile hw_spdif_data_t *) HW_SPDIF_DATA_ADDR)
#define HW_SPDIF_DATA_RD()         (HW_SPDIF_DATA.U)
#define HW_SPDIF_DATA_WR(v)        (HW_SPDIF_DATA.U = (v))
#define HW_SPDIF_DATA_SET(v)       ((*(volatile reg32_t *) HW_SPDIF_DATA_SET_ADDR) = (v))
#define HW_SPDIF_DATA_CLR(v)       ((*(volatile reg32_t *) HW_SPDIF_DATA_CLR_ADDR) = (v))
#define HW_SPDIF_DATA_TOG(v)       ((*(volatile reg32_t *) HW_SPDIF_DATA_TOG_ADDR) = (v))
#endif


// constants & macros for individual HW_SPDIF_DATA bitfields

//  Register HW_SPDIF_DATA, field HIGH
#define BP_SPDIF_DATA_HIGH         16
#define BM_SPDIF_DATA_HIGH         0xFFFF0000
#ifndef __LANGUAGE_ASM__
#define BF_SPDIF_DATA_HIGH(v)      ((((reg32_t) v) << 16) & BM_SPDIF_DATA_HIGH)
#else
#define BF_SPDIF_DATA_HIGH(v)      (((v) << 16) & BM_SPDIF_DATA_HIGH)
#endif
#ifndef __LANGUAGE_ASM__
#define BW_SPDIF_DATA_HIGH(v)      (HW_SPDIF_DATA.B.HIGH = (v))
#endif

//--- Register HW_SPDIF_DATA, field LOW
#define BP_SPDIF_DATA_LOW          0
#define BM_SPDIF_DATA_LOW          0x0000FFFF
#define BF_SPDIF_DATA_LOW(v)       (((v) << 0) & BM_SPDIF_DATA_LOW)
#ifndef __LANGUAGE_ASM__
#define BW_SPDIF_DATA_LOW(v)       (HW_SPDIF_DATA.B.LOW = (v))
#endif


//------------------------------------------------------------------------------
//  HW_SPDIF_VERSION - SPDIF Version Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned STEP   : 16;
        unsigned MINOR  :  8;
        unsigned MAJOR  :  8;
    } B;
} hw_spdif_version_t;
#endif


// constants & macros for entire HW_SPDIF_VERSION register
#define HW_SPDIF_VERSION_ADDR      (REGS_SPDIF_BASE + 0x60)
#ifndef __LANGUAGE_ASM__
#define HW_SPDIF_VERSION           (*(volatile hw_spdif_version_t *) HW_SPDIF_VERSION_ADDR)
#define HW_SPDIF_VERSION_RD()      (HW_SPDIF_VERSION.U)
#endif


// constants & macros for individual HW_SPDIF_VERSION bitfields

//  Register HW_SPDIF_VERSION, field MAJOR
#define BP_SPDIF_VERSION_MAJOR      24
#define BM_SPDIF_VERSION_MAJOR      0xFF000000
#ifndef __LANGUAGE_ASM__
#define BF_SPDIF_VERSION_MAJOR(v)   ((((reg32_t) v) << 24) & BM_SPDIF_VERSION_MAJOR)
#else
#define BF_SPDIF_VERSION_MAJOR(v)   (((v) << 24) & BM_SPDIF_VERSION_MAJOR)
#endif

//  Register HW_SPDIF_VERSION, field MINOR
#define BP_SPDIF_VERSION_MINOR      16
#define BM_SPDIF_VERSION_MINOR      0x00FF0000
#define BF_SPDIF_VERSION_MINOR(v)   (((v) << 16) & BM_SPDIF_VERSION_MINOR)

//  Register HW_SPDIF_VERSION, field STEP
#define BP_SPDIF_VERSION_STEP       0
#define BM_SPDIF_VERSION_STEP       0x0000FFFF
#define BF_SPDIF_VERSION_STEP(v)    (((v) << 0) & BM_SPDIF_VERSION_STEP)


#endif // __COMMON _REGSSPDIF_H
