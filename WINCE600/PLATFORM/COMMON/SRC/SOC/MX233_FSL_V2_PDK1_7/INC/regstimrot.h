//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: regstimrot.h
//  PIO Registers for TIMROT interface
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

#ifndef _REGSTIMROT_H
#define _REGSTIMROT_H  1

#define REGS_TIMROT_BASE (DWORD) pv_HWregTIMROT

////////////////////////////////////////////////////////////////////////////////
//// HW_TIMROT_ROTCTRL - Rotary Decoder Control Register
////////////////////////////////////////////////////////////////////////////////

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t U;
    struct
    {
        unsigned SELECT_A        :  3;
        unsigned RSRVD1          :  1;
        unsigned SELECT_B        :  3;
        unsigned RSRVD2          :  1;
        unsigned POLARITY_A      :  1;
        unsigned POLARITY_B      :  1;
        unsigned OVERSAMPLE      :  2;
        unsigned RELATIVE        :  1;
        unsigned RSRVD3          :  3;
        unsigned DIVIDER         :  6;
        unsigned STATE           :  3;
        unsigned TIM0_PRESENT    :  1;
        unsigned TIM1_PRESENT    :  1;
        unsigned TIM2_PRESENT    :  1;
        unsigned TIM3_PRESENT    :  1;
        unsigned ROTARY_PRESENT  :  1;
        unsigned CLKGATE         :  1;
        unsigned SFTRST          :  1;
    } B;
} hw_timrot_rotctrl_t;
#endif


//
// constants & macros for entire HW_TIMROT_ROTCTRL register
//

#define HW_TIMROT_ROTCTRL_ADDR      (REGS_TIMROT_BASE + 0x00000000)
#define HW_TIMROT_ROTCTRL_SET_ADDR  (REGS_TIMROT_BASE + 0x00000004)
#define HW_TIMROT_ROTCTRL_CLR_ADDR  (REGS_TIMROT_BASE + 0x00000008)
#define HW_TIMROT_ROTCTRL_TOG_ADDR  (REGS_TIMROT_BASE + 0x0000000C)

#ifndef __LANGUAGE_ASM__
#define HW_TIMROT_ROTCTRL           (*(volatile hw_timrot_rotctrl_t *) HW_TIMROT_ROTCTRL_ADDR)
#define HW_TIMROT_ROTCTRL_RD()      (HW_TIMROT_ROTCTRL.U)
#define HW_TIMROT_ROTCTRL_WR(v)     (HW_TIMROT_ROTCTRL.U = (v))
#define HW_TIMROT_ROTCTRL_SET(v)    ((*(volatile reg32_t *) HW_TIMROT_ROTCTRL_SET_ADDR) = (v))
#define HW_TIMROT_ROTCTRL_CLR(v)    ((*(volatile reg32_t *) HW_TIMROT_ROTCTRL_CLR_ADDR) = (v))
#define HW_TIMROT_ROTCTRL_TOG(v)    ((*(volatile reg32_t *) HW_TIMROT_ROTCTRL_TOG_ADDR) = (v))
#endif


//
// constants & macros for individual HW_TIMROT_ROTCTRL bitfields
//

//--- Register HW_TIMROT_ROTCTRL, field SFTRST

#define BP_TIMROT_ROTCTRL_SFTRST      31
#define BM_TIMROT_ROTCTRL_SFTRST      0x80000000

#ifndef __LANGUAGE_ASM__
#define BF_TIMROT_ROTCTRL_SFTRST(v)   ((((reg32_t) v) << 31) & BM_TIMROT_ROTCTRL_SFTRST)
#else
#define BF_TIMROT_ROTCTRL_SFTRST(v)   (((v) << 31) & BM_TIMROT_ROTCTRL_SFTRST)
#endif

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_ROTCTRL_SFTRST(v)   BF_CS1(TIMROT_ROTCTRL, SFTRST, v)
#endif

//--- Register HW_TIMROT_ROTCTRL, field CLKGATE

#define BP_TIMROT_ROTCTRL_CLKGATE      30
#define BM_TIMROT_ROTCTRL_CLKGATE      0x40000000

#define BF_TIMROT_ROTCTRL_CLKGATE(v)   (((v) << 30) & BM_TIMROT_ROTCTRL_CLKGATE)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_ROTCTRL_CLKGATE(v)   BF_CS1(TIMROT_ROTCTRL, CLKGATE, v)
#endif

//--- Register HW_TIMROT_ROTCTRL, field ROTARY_PRESENT

#define BP_TIMROT_ROTCTRL_ROTARY_PRESENT      29
#define BM_TIMROT_ROTCTRL_ROTARY_PRESENT      0x20000000

#define BF_TIMROT_ROTCTRL_ROTARY_PRESENT(v)   (((v) << 29) & BM_TIMROT_ROTCTRL_ROTARY_PRESENT)

//--- Register HW_TIMROT_ROTCTRL, field TIM3_PRESENT

#define BP_TIMROT_ROTCTRL_TIM3_PRESENT      28
#define BM_TIMROT_ROTCTRL_TIM3_PRESENT      0x10000000

#define BF_TIMROT_ROTCTRL_TIM3_PRESENT(v)   (((v) << 28) & BM_TIMROT_ROTCTRL_TIM3_PRESENT)

//--- Register HW_TIMROT_ROTCTRL, field TIM2_PRESENT

#define BP_TIMROT_ROTCTRL_TIM2_PRESENT      27
#define BM_TIMROT_ROTCTRL_TIM2_PRESENT      0x08000000

#define BF_TIMROT_ROTCTRL_TIM2_PRESENT(v)   (((v) << 27) & BM_TIMROT_ROTCTRL_TIM2_PRESENT)

//--- Register HW_TIMROT_ROTCTRL, field TIM1_PRESENT

#define BP_TIMROT_ROTCTRL_TIM1_PRESENT      26
#define BM_TIMROT_ROTCTRL_TIM1_PRESENT      0x04000000

#define BF_TIMROT_ROTCTRL_TIM1_PRESENT(v)   (((v) << 26) & BM_TIMROT_ROTCTRL_TIM1_PRESENT)

//--- Register HW_TIMROT_ROTCTRL, field TIM0_PRESENT

#define BP_TIMROT_ROTCTRL_TIM0_PRESENT      25
#define BM_TIMROT_ROTCTRL_TIM0_PRESENT      0x02000000

#define BF_TIMROT_ROTCTRL_TIM0_PRESENT(v)   (((v) << 25) & BM_TIMROT_ROTCTRL_TIM0_PRESENT)

//--- Register HW_TIMROT_ROTCTRL, field STATE

#define BP_TIMROT_ROTCTRL_STATE      22
#define BM_TIMROT_ROTCTRL_STATE      0x01C00000

#define BF_TIMROT_ROTCTRL_STATE(v)   (((v) << 22) & BM_TIMROT_ROTCTRL_STATE)

//--- Register HW_TIMROT_ROTCTRL, field DIVIDER

#define BP_TIMROT_ROTCTRL_DIVIDER      16
#define BM_TIMROT_ROTCTRL_DIVIDER      0x003F0000

#define BF_TIMROT_ROTCTRL_DIVIDER(v)   (((v) << 16) & BM_TIMROT_ROTCTRL_DIVIDER)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_ROTCTRL_DIVIDER(v)   BF_CS1(TIMROT_ROTCTRL, DIVIDER, v)
#endif

//--- Register HW_TIMROT_ROTCTRL, field RELATIVE

#define BP_TIMROT_ROTCTRL_RELATIVE      12
#define BM_TIMROT_ROTCTRL_RELATIVE      0x00001000

#define BF_TIMROT_ROTCTRL_RELATIVE(v)   (((v) << 12) & BM_TIMROT_ROTCTRL_RELATIVE)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_ROTCTRL_RELATIVE(v)   BF_CS1(TIMROT_ROTCTRL, RELATIVE, v)
#endif

//--- Register HW_TIMROT_ROTCTRL, field OVERSAMPLE

#define BP_TIMROT_ROTCTRL_OVERSAMPLE      10
#define BM_TIMROT_ROTCTRL_OVERSAMPLE      0x00000C00

#define BF_TIMROT_ROTCTRL_OVERSAMPLE(v)   (((v) << 10) & BM_TIMROT_ROTCTRL_OVERSAMPLE)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_ROTCTRL_OVERSAMPLE(v)   BF_CS1(TIMROT_ROTCTRL, OVERSAMPLE, v)
#endif

#define BV_TIMROT_ROTCTRL_OVERSAMPLE__8X  0x0
#define BV_TIMROT_ROTCTRL_OVERSAMPLE__4X  0x1
#define BV_TIMROT_ROTCTRL_OVERSAMPLE__2X  0x2
#define BV_TIMROT_ROTCTRL_OVERSAMPLE__1X  0x3

//--- Register HW_TIMROT_ROTCTRL, field POLARITY_B

#define BP_TIMROT_ROTCTRL_POLARITY_B      9
#define BM_TIMROT_ROTCTRL_POLARITY_B      0x00000200

#define BF_TIMROT_ROTCTRL_POLARITY_B(v)   (((v) << 9) & BM_TIMROT_ROTCTRL_POLARITY_B)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_ROTCTRL_POLARITY_B(v)   BF_CS1(TIMROT_ROTCTRL, POLARITY_B, v)
#endif

//--- Register HW_TIMROT_ROTCTRL, field POLARITY_A

#define BP_TIMROT_ROTCTRL_POLARITY_A      8
#define BM_TIMROT_ROTCTRL_POLARITY_A      0x00000100

#define BF_TIMROT_ROTCTRL_POLARITY_A(v)   (((v) << 8) & BM_TIMROT_ROTCTRL_POLARITY_A)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_ROTCTRL_POLARITY_A(v)   BF_CS1(TIMROT_ROTCTRL, POLARITY_A, v)
#endif

//--- Register HW_TIMROT_ROTCTRL, field SELECT_B

#define BP_TIMROT_ROTCTRL_SELECT_B      4
#define BM_TIMROT_ROTCTRL_SELECT_B      0x00000070

#define BF_TIMROT_ROTCTRL_SELECT_B(v)   (((v) << 4) & BM_TIMROT_ROTCTRL_SELECT_B)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_ROTCTRL_SELECT_B(v)   BF_CS1(TIMROT_ROTCTRL, SELECT_B, v)
#endif

#define BV_TIMROT_ROTCTRL_SELECT_B__NEVER_TICK  0x0
#define BV_TIMROT_ROTCTRL_SELECT_B__PWM0        0x1
#define BV_TIMROT_ROTCTRL_SELECT_B__PWM1        0x2
#define BV_TIMROT_ROTCTRL_SELECT_B__PWM2        0x3
#define BV_TIMROT_ROTCTRL_SELECT_B__PWM3        0x4
#define BV_TIMROT_ROTCTRL_SELECT_B__PWM4        0x5
#define BV_TIMROT_ROTCTRL_SELECT_B__ROTARYA     0x6
#define BV_TIMROT_ROTCTRL_SELECT_B__ROTARYB     0x7

//--- Register HW_TIMROT_ROTCTRL, field SELECT_A

#define BP_TIMROT_ROTCTRL_SELECT_A      0
#define BM_TIMROT_ROTCTRL_SELECT_A      0x00000007

#define BF_TIMROT_ROTCTRL_SELECT_A(v)   (((v) << 0) & BM_TIMROT_ROTCTRL_SELECT_A)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_ROTCTRL_SELECT_A(v)   BF_CS1(TIMROT_ROTCTRL, SELECT_A, v)
#endif

#define BV_TIMROT_ROTCTRL_SELECT_A__NEVER_TICK  0x0
#define BV_TIMROT_ROTCTRL_SELECT_A__PWM0        0x1
#define BV_TIMROT_ROTCTRL_SELECT_A__PWM1        0x2
#define BV_TIMROT_ROTCTRL_SELECT_A__PWM2        0x3
#define BV_TIMROT_ROTCTRL_SELECT_A__PWM3        0x4
#define BV_TIMROT_ROTCTRL_SELECT_A__PWM4        0x5
#define BV_TIMROT_ROTCTRL_SELECT_A__ROTARYA     0x6
#define BV_TIMROT_ROTCTRL_SELECT_A__ROTARYB     0x7


////////////////////////////////////////////////////////////////////////////////
//// HW_TIMROT_ROTCOUNT - Rotary Decoder Up/Down Counter Register
////////////////////////////////////////////////////////////////////////////////

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t U;
    struct
    {
        unsigned UPDOWN  : 16;
        unsigned RSRVD1  : 16;
    } B;
} hw_timrot_rotcount_t;
#endif


//
// constants & macros for entire HW_TIMROT_ROTCOUNT register
//

#define HW_TIMROT_ROTCOUNT_ADDR      (REGS_TIMROT_BASE + 0x00000010)

#ifndef __LANGUAGE_ASM__
#define HW_TIMROT_ROTCOUNT           (*(volatile hw_timrot_rotcount_t *) HW_TIMROT_ROTCOUNT_ADDR)
#define HW_TIMROT_ROTCOUNT_RD()      (HW_TIMROT_ROTCOUNT.U)
#endif


//
// constants & macros for individual HW_TIMROT_ROTCOUNT bitfields
//

//--- Register HW_TIMROT_ROTCOUNT, field UPDOWN

#define BP_TIMROT_ROTCOUNT_UPDOWN      0
#define BM_TIMROT_ROTCOUNT_UPDOWN      0x0000FFFF

#define BF_TIMROT_ROTCOUNT_UPDOWN(v)   (((v) << 0) & BM_TIMROT_ROTCOUNT_UPDOWN)


////////////////////////////////////////////////////////////////////////////////
//// multi-register-define name HW_TIMROT_TIMCTRLn : base 0x80068020 : count 3 : offset 0x20
////////////////////////////////////////////////////////////////////////////////

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t U;
    struct
    {
        unsigned SELECT    :  4;
        unsigned PRESCALE  :  2;
        unsigned RELOAD    :  1;
        unsigned UPDATE    :  1;
        unsigned POLARITY  :  1;
        unsigned RSRVD1    :  5;
        unsigned IRQ_EN    :  1;
        unsigned IRQ       :  1;
        unsigned RSRVD2    : 16;
    } B;
} hw_timrot_timctrln_t;
#endif


//
// constants & macros for entire HW_TIMROT_TIMCTRLn multi-register
//

#define HW_TIMROT_TIMCTRLn_COUNT        3
#define HW_TIMROT_TIMCTRLn_ADDR(n)      (REGS_TIMROT_BASE + 0x00000020 + ((n) * 0x20))
#define HW_TIMROT_TIMCTRLn_SET_ADDR(n)  (REGS_TIMROT_BASE + 0x00000024 + ((n) * 0x20))
#define HW_TIMROT_TIMCTRLn_CLR_ADDR(n)  (REGS_TIMROT_BASE + 0x00000028 + ((n) * 0x20))
#define HW_TIMROT_TIMCTRLn_TOG_ADDR(n)  (REGS_TIMROT_BASE + 0x0000002C + ((n) * 0x20))

#ifndef __LANGUAGE_ASM__
#define HW_TIMROT_TIMCTRLn(n)           (*(volatile hw_timrot_timctrln_t *) HW_TIMROT_TIMCTRLn_ADDR(n))
#define HW_TIMROT_TIMCTRLn_RD(n)        (HW_TIMROT_TIMCTRLn(n).U)
#define HW_TIMROT_TIMCTRLn_WR(n, v)     (HW_TIMROT_TIMCTRLn(n).U = (v))
#define HW_TIMROT_TIMCTRLn_SET(n, v)    ((*(volatile reg32_t *) HW_TIMROT_TIMCTRLn_SET_ADDR(n)) = (v))
#define HW_TIMROT_TIMCTRLn_CLR(n, v)    ((*(volatile reg32_t *) HW_TIMROT_TIMCTRLn_CLR_ADDR(n)) = (v))
#define HW_TIMROT_TIMCTRLn_TOG(n, v)    ((*(volatile reg32_t *) HW_TIMROT_TIMCTRLn_TOG_ADDR(n)) = (v))
#endif


//
// constants & macros for individual HW_TIMROT_TIMCTRLn multi-register bitfields
//

//--- Register HW_TIMROT_TIMCTRLn, field IRQ

#define BP_TIMROT_TIMCTRLn_IRQ      15
#define BM_TIMROT_TIMCTRLn_IRQ      0x00008000

#define BF_TIMROT_TIMCTRLn_IRQ(v)   (((v) << 15) & BM_TIMROT_TIMCTRLn_IRQ)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRLn_IRQ(n, v)  BF_CS1n(TIMROT_TIMCTRLn, n, IRQ, v)
#endif

//--- Register HW_TIMROT_TIMCTRLn, field IRQ_EN

#define BP_TIMROT_TIMCTRLn_IRQ_EN      14
#define BM_TIMROT_TIMCTRLn_IRQ_EN      0x00004000

#define BF_TIMROT_TIMCTRLn_IRQ_EN(v)   (((v) << 14) & BM_TIMROT_TIMCTRLn_IRQ_EN)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRLn_IRQ_EN(n, v)  BF_CS1n(TIMROT_TIMCTRLn, n, IRQ_EN, v)
#endif

//--- Register HW_TIMROT_TIMCTRLn, field POLARITY

#define BP_TIMROT_TIMCTRLn_POLARITY      8
#define BM_TIMROT_TIMCTRLn_POLARITY      0x00000100

#define BF_TIMROT_TIMCTRLn_POLARITY(v)   (((v) << 8) & BM_TIMROT_TIMCTRLn_POLARITY)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRLn_POLARITY(n, v)  BF_CS1n(TIMROT_TIMCTRLn, n, POLARITY, v)
#endif

//--- Register HW_TIMROT_TIMCTRLn, field UPDATE

#define BP_TIMROT_TIMCTRLn_UPDATE      7
#define BM_TIMROT_TIMCTRLn_UPDATE      0x00000080

#define BF_TIMROT_TIMCTRLn_UPDATE(v)   (((v) << 7) & BM_TIMROT_TIMCTRLn_UPDATE)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRLn_UPDATE(n, v)  BF_CS1n(TIMROT_TIMCTRLn, n, UPDATE, v)
#endif

//--- Register HW_TIMROT_TIMCTRLn, field RELOAD

#define BP_TIMROT_TIMCTRLn_RELOAD      6
#define BM_TIMROT_TIMCTRLn_RELOAD      0x00000040

#define BF_TIMROT_TIMCTRLn_RELOAD(v)   (((v) << 6) & BM_TIMROT_TIMCTRLn_RELOAD)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRLn_RELOAD(n, v)  BF_CS1n(TIMROT_TIMCTRLn, n, RELOAD, v)
#endif

//--- Register HW_TIMROT_TIMCTRLn, field PRESCALE

#define BP_TIMROT_TIMCTRLn_PRESCALE      4
#define BM_TIMROT_TIMCTRLn_PRESCALE      0x00000030

#define BF_TIMROT_TIMCTRLn_PRESCALE(v)   (((v) << 4) & BM_TIMROT_TIMCTRLn_PRESCALE)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRLn_PRESCALE(n, v)  BF_CS1n(TIMROT_TIMCTRLn, n, PRESCALE, v)
#endif

#define BV_TIMROT_TIMCTRLn_PRESCALE__DIV_BY_1  0x0
#define BV_TIMROT_TIMCTRLn_PRESCALE__DIV_BY_2  0x1
#define BV_TIMROT_TIMCTRLn_PRESCALE__DIV_BY_4  0x2
#define BV_TIMROT_TIMCTRLn_PRESCALE__DIV_BY_8  0x3

//--- Register HW_TIMROT_TIMCTRLn, field SELECT

#define BP_TIMROT_TIMCTRLn_SELECT      0
#define BM_TIMROT_TIMCTRLn_SELECT      0x0000000F

#define BF_TIMROT_TIMCTRLn_SELECT(v)   (((v) << 0) & BM_TIMROT_TIMCTRLn_SELECT)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRLn_SELECT(n, v)  BF_CS1n(TIMROT_TIMCTRLn, n, SELECT, v)
#endif

#define BV_TIMROT_TIMCTRLn_SELECT__NEVER_TICK   0x0
#define BV_TIMROT_TIMCTRLn_SELECT__PWM0         0x1
#define BV_TIMROT_TIMCTRLn_SELECT__PWM1         0x2
#define BV_TIMROT_TIMCTRLn_SELECT__PWM2         0x3
#define BV_TIMROT_TIMCTRLn_SELECT__PWM3         0x4
#define BV_TIMROT_TIMCTRLn_SELECT__PWM4         0x5
#define BV_TIMROT_TIMCTRLn_SELECT__ROTARYA      0x6
#define BV_TIMROT_TIMCTRLn_SELECT__ROTARYB      0x7
#define BV_TIMROT_TIMCTRLn_SELECT__32KHZ_XTAL   0x8
#define BV_TIMROT_TIMCTRLn_SELECT__8KHZ_XTAL    0x9
#define BV_TIMROT_TIMCTRLn_SELECT__4KHZ_XTAL    0xA
#define BV_TIMROT_TIMCTRLn_SELECT__1KHZ_XTAL    0xB
#define BV_TIMROT_TIMCTRLn_SELECT__TICK_ALWAYS  0xC


////////////////////////////////////////////////////////////////////////////////
//// multi-register-define name HW_TIMROT_TIMCOUNTn : base 0x80068030 : count 3 : offset 0x20
////////////////////////////////////////////////////////////////////////////////

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t U;
    struct
    {
        unsigned FIXED_COUNT    : 16;
        unsigned RUNNING_COUNT  : 16;
    } B;
} hw_timrot_timcountn_t;
#endif


//
// constants & macros for entire HW_TIMROT_TIMCOUNTn multi-register
//

#define HW_TIMROT_TIMCOUNTn_COUNT        3
#define HW_TIMROT_TIMCOUNTn_ADDR(n)      (REGS_TIMROT_BASE + 0x00000030 + ((n) * 0x20))

#ifndef __LANGUAGE_ASM__
#define HW_TIMROT_TIMCOUNTn(n)           (*(volatile hw_timrot_timcountn_t *) HW_TIMROT_TIMCOUNTn_ADDR(n))
#define HW_TIMROT_TIMCOUNTn_RD(n)        (HW_TIMROT_TIMCOUNTn(n).U)
#define HW_TIMROT_TIMCOUNTn_WR(n, v)     (HW_TIMROT_TIMCOUNTn(n).U = (v))
#define HW_TIMROT_TIMCOUNTn_SET(n, v)    (HW_TIMROT_TIMCOUNTn_WR(n, HW_TIMROT_TIMCOUNTn_RD(n) |  (v)))
#define HW_TIMROT_TIMCOUNTn_CLR(n, v)    (HW_TIMROT_TIMCOUNTn_WR(n, HW_TIMROT_TIMCOUNTn_RD(n) & ~(v)))
#define HW_TIMROT_TIMCOUNTn_TOG(n, v)    (HW_TIMROT_TIMCOUNTn_WR(n, HW_TIMROT_TIMCOUNTn_RD(n) ^  (v)))
#endif


//
// constants & macros for individual HW_TIMROT_TIMCOUNTn multi-register bitfields
//

//--- Register HW_TIMROT_TIMCOUNTn, field RUNNING_COUNT

#define BP_TIMROT_TIMCOUNTn_RUNNING_COUNT      16
#define BM_TIMROT_TIMCOUNTn_RUNNING_COUNT      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_TIMROT_TIMCOUNTn_RUNNING_COUNT(v)   ((((reg32_t) v) << 16) & BM_TIMROT_TIMCOUNTn_RUNNING_COUNT)
#else
#define BF_TIMROT_TIMCOUNTn_RUNNING_COUNT(v)   (((v) << 16) & BM_TIMROT_TIMCOUNTn_RUNNING_COUNT)
#endif

//--- Register HW_TIMROT_TIMCOUNTn, field FIXED_COUNT

#define BP_TIMROT_TIMCOUNTn_FIXED_COUNT      0
#define BM_TIMROT_TIMCOUNTn_FIXED_COUNT      0x0000FFFF

#define BF_TIMROT_TIMCOUNTn_FIXED_COUNT(v)   (((v) << 0) & BM_TIMROT_TIMCOUNTn_FIXED_COUNT)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCOUNTn_FIXED_COUNT(n, v)  (HW_TIMROT_TIMCOUNTn(n).B.FIXED_COUNT = (v))
#endif


////////////////////////////////////////////////////////////////////////////////
//// HW_TIMROT_TIMCTRL3 - Timer 3 Control and Status Register
////////////////////////////////////////////////////////////////////////////////

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t U;
    struct
    {
        unsigned SELECT       :  4;
        unsigned PRESCALE     :  2;
        unsigned RELOAD       :  1;
        unsigned UPDATE       :  1;
        unsigned POLARITY     :  1;
        unsigned DUTY_CYCLE   :  1;
        unsigned DUTY_VALID   :  1;
        unsigned RSRVD1       :  3;
        unsigned IRQ_EN       :  1;
        unsigned IRQ          :  1;
        unsigned TEST_SIGNAL  :  4;
        unsigned RSRVD2       : 12;
    } B;
} hw_timrot_timctrl3_t;
#endif


//
// constants & macros for entire HW_TIMROT_TIMCTRL3 register
//

#define HW_TIMROT_TIMCTRL3_ADDR      (REGS_TIMROT_BASE + 0x00000080)
#define HW_TIMROT_TIMCTRL3_SET_ADDR  (REGS_TIMROT_BASE + 0x00000084)
#define HW_TIMROT_TIMCTRL3_CLR_ADDR  (REGS_TIMROT_BASE + 0x00000088)
#define HW_TIMROT_TIMCTRL3_TOG_ADDR  (REGS_TIMROT_BASE + 0x0000008C)

#ifndef __LANGUAGE_ASM__
#define HW_TIMROT_TIMCTRL3           (*(volatile hw_timrot_timctrl3_t *) HW_TIMROT_TIMCTRL3_ADDR)
#define HW_TIMROT_TIMCTRL3_RD()      (HW_TIMROT_TIMCTRL3.U)
#define HW_TIMROT_TIMCTRL3_WR(v)     (HW_TIMROT_TIMCTRL3.U = (v))
#define HW_TIMROT_TIMCTRL3_SET(v)    ((*(volatile reg32_t *) HW_TIMROT_TIMCTRL3_SET_ADDR) = (v))
#define HW_TIMROT_TIMCTRL3_CLR(v)    ((*(volatile reg32_t *) HW_TIMROT_TIMCTRL3_CLR_ADDR) = (v))
#define HW_TIMROT_TIMCTRL3_TOG(v)    ((*(volatile reg32_t *) HW_TIMROT_TIMCTRL3_TOG_ADDR) = (v))
#endif


//
// constants & macros for individual HW_TIMROT_TIMCTRL3 bitfields
//

//--- Register HW_TIMROT_TIMCTRL3, field TEST_SIGNAL

#define BP_TIMROT_TIMCTRL3_TEST_SIGNAL      16
#define BM_TIMROT_TIMCTRL3_TEST_SIGNAL      0x000F0000

#define BF_TIMROT_TIMCTRL3_TEST_SIGNAL(v)   (((v) << 16) & BM_TIMROT_TIMCTRL3_TEST_SIGNAL)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRL3_TEST_SIGNAL(v)   BF_CS1(TIMROT_TIMCTRL3, TEST_SIGNAL, v)
#endif

#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__NEVER_TICK   0x0
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__PWM0         0x1
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__PWM1         0x2
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__PWM2         0x3
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__PWM3         0x4
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__PWM4         0x5
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__ROTARYA      0x6
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__ROTARYB      0x7
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__32KHZ_XTAL   0x8
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__8KHZ_XTAL    0x9
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__4KHZ_XTAL    0xA
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__1KHZ_XTAL    0xB
#define BV_TIMROT_TIMCTRL3_TEST_SIGNAL__TICK_ALWAYS  0xC

//--- Register HW_TIMROT_TIMCTRL3, field IRQ

#define BP_TIMROT_TIMCTRL3_IRQ      15
#define BM_TIMROT_TIMCTRL3_IRQ      0x00008000

#define BF_TIMROT_TIMCTRL3_IRQ(v)   (((v) << 15) & BM_TIMROT_TIMCTRL3_IRQ)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRL3_IRQ(v)   BF_CS1(TIMROT_TIMCTRL3, IRQ, v)
#endif

//--- Register HW_TIMROT_TIMCTRL3, field IRQ_EN

#define BP_TIMROT_TIMCTRL3_IRQ_EN      14
#define BM_TIMROT_TIMCTRL3_IRQ_EN      0x00004000

#define BF_TIMROT_TIMCTRL3_IRQ_EN(v)   (((v) << 14) & BM_TIMROT_TIMCTRL3_IRQ_EN)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRL3_IRQ_EN(v)   BF_CS1(TIMROT_TIMCTRL3, IRQ_EN, v)
#endif

//--- Register HW_TIMROT_TIMCTRL3, field DUTY_VALID

#define BP_TIMROT_TIMCTRL3_DUTY_VALID      10
#define BM_TIMROT_TIMCTRL3_DUTY_VALID      0x00000400

#define BF_TIMROT_TIMCTRL3_DUTY_VALID(v)   (((v) << 10) & BM_TIMROT_TIMCTRL3_DUTY_VALID)

//--- Register HW_TIMROT_TIMCTRL3, field DUTY_CYCLE

#define BP_TIMROT_TIMCTRL3_DUTY_CYCLE      9
#define BM_TIMROT_TIMCTRL3_DUTY_CYCLE      0x00000200

#define BF_TIMROT_TIMCTRL3_DUTY_CYCLE(v)   (((v) << 9) & BM_TIMROT_TIMCTRL3_DUTY_CYCLE)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRL3_DUTY_CYCLE(v)   BF_CS1(TIMROT_TIMCTRL3, DUTY_CYCLE, v)
#endif

//--- Register HW_TIMROT_TIMCTRL3, field POLARITY

#define BP_TIMROT_TIMCTRL3_POLARITY      8
#define BM_TIMROT_TIMCTRL3_POLARITY      0x00000100

#define BF_TIMROT_TIMCTRL3_POLARITY(v)   (((v) << 8) & BM_TIMROT_TIMCTRL3_POLARITY)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRL3_POLARITY(v)   BF_CS1(TIMROT_TIMCTRL3, POLARITY, v)
#endif

//--- Register HW_TIMROT_TIMCTRL3, field UPDATE

#define BP_TIMROT_TIMCTRL3_UPDATE      7
#define BM_TIMROT_TIMCTRL3_UPDATE      0x00000080

#define BF_TIMROT_TIMCTRL3_UPDATE(v)   (((v) << 7) & BM_TIMROT_TIMCTRL3_UPDATE)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRL3_UPDATE(v)   BF_CS1(TIMROT_TIMCTRL3, UPDATE, v)
#endif

//--- Register HW_TIMROT_TIMCTRL3, field RELOAD

#define BP_TIMROT_TIMCTRL3_RELOAD      6
#define BM_TIMROT_TIMCTRL3_RELOAD      0x00000040

#define BF_TIMROT_TIMCTRL3_RELOAD(v)   (((v) << 6) & BM_TIMROT_TIMCTRL3_RELOAD)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRL3_RELOAD(v)   BF_CS1(TIMROT_TIMCTRL3, RELOAD, v)
#endif

//--- Register HW_TIMROT_TIMCTRL3, field PRESCALE

#define BP_TIMROT_TIMCTRL3_PRESCALE      4
#define BM_TIMROT_TIMCTRL3_PRESCALE      0x00000030

#define BF_TIMROT_TIMCTRL3_PRESCALE(v)   (((v) << 4) & BM_TIMROT_TIMCTRL3_PRESCALE)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRL3_PRESCALE(v)   BF_CS1(TIMROT_TIMCTRL3, PRESCALE, v)
#endif

#define BV_TIMROT_TIMCTRL3_PRESCALE__DIV_BY_1  0x0
#define BV_TIMROT_TIMCTRL3_PRESCALE__DIV_BY_2  0x1
#define BV_TIMROT_TIMCTRL3_PRESCALE__DIV_BY_4  0x2
#define BV_TIMROT_TIMCTRL3_PRESCALE__DIV_BY_8  0x3

//--- Register HW_TIMROT_TIMCTRL3, field SELECT

#define BP_TIMROT_TIMCTRL3_SELECT      0
#define BM_TIMROT_TIMCTRL3_SELECT      0x0000000F

#define BF_TIMROT_TIMCTRL3_SELECT(v)   (((v) << 0) & BM_TIMROT_TIMCTRL3_SELECT)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCTRL3_SELECT(v)   BF_CS1(TIMROT_TIMCTRL3, SELECT, v)
#endif

#define BV_TIMROT_TIMCTRL3_SELECT__NEVER_TICK   0x0
#define BV_TIMROT_TIMCTRL3_SELECT__PWM0         0x1
#define BV_TIMROT_TIMCTRL3_SELECT__PWM1         0x2
#define BV_TIMROT_TIMCTRL3_SELECT__PWM2         0x3
#define BV_TIMROT_TIMCTRL3_SELECT__PWM3         0x4
#define BV_TIMROT_TIMCTRL3_SELECT__PWM4         0x5
#define BV_TIMROT_TIMCTRL3_SELECT__ROTARYA      0x6
#define BV_TIMROT_TIMCTRL3_SELECT__ROTARYB      0x7
#define BV_TIMROT_TIMCTRL3_SELECT__32KHZ_XTAL   0x8
#define BV_TIMROT_TIMCTRL3_SELECT__8KHZ_XTAL    0x9
#define BV_TIMROT_TIMCTRL3_SELECT__4KHZ_XTAL    0xA
#define BV_TIMROT_TIMCTRL3_SELECT__1KHZ_XTAL    0xB
#define BV_TIMROT_TIMCTRL3_SELECT__TICK_ALWAYS  0xC


////////////////////////////////////////////////////////////////////////////////
//// HW_TIMROT_TIMCOUNT3 - Timer 3 Count Register
////////////////////////////////////////////////////////////////////////////////

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t U;
    struct
    {
        unsigned HIGH_FIXED_COUNT   : 16;
        unsigned LOW_RUNNING_COUNT  : 16;
    } B;
} hw_timrot_timcount3_t;
#endif


//
// constants & macros for entire HW_TIMROT_TIMCOUNT3 register
//

#define HW_TIMROT_TIMCOUNT3_ADDR      (REGS_TIMROT_BASE + 0x00000090)

#ifndef __LANGUAGE_ASM__
#define HW_TIMROT_TIMCOUNT3           (*(volatile hw_timrot_timcount3_t *) HW_TIMROT_TIMCOUNT3_ADDR)
#define HW_TIMROT_TIMCOUNT3_RD()      (HW_TIMROT_TIMCOUNT3.U)
#define HW_TIMROT_TIMCOUNT3_WR(v)     (HW_TIMROT_TIMCOUNT3.U = (v))
#define HW_TIMROT_TIMCOUNT3_SET(v)    (HW_TIMROT_TIMCOUNT3_WR(HW_TIMROT_TIMCOUNT3_RD() |  (v)))
#define HW_TIMROT_TIMCOUNT3_CLR(v)    (HW_TIMROT_TIMCOUNT3_WR(HW_TIMROT_TIMCOUNT3_RD() & ~(v)))
#define HW_TIMROT_TIMCOUNT3_TOG(v)    (HW_TIMROT_TIMCOUNT3_WR(HW_TIMROT_TIMCOUNT3_RD() ^  (v)))
#endif


//
// constants & macros for individual HW_TIMROT_TIMCOUNT3 bitfields
//

//--- Register HW_TIMROT_TIMCOUNT3, field LOW_RUNNING_COUNT

#define BP_TIMROT_TIMCOUNT3_LOW_RUNNING_COUNT      16
#define BM_TIMROT_TIMCOUNT3_LOW_RUNNING_COUNT      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_TIMROT_TIMCOUNT3_LOW_RUNNING_COUNT(v)   ((((reg32_t) v) << 16) & BM_TIMROT_TIMCOUNT3_LOW_RUNNING_COUNT)
#else
#define BF_TIMROT_TIMCOUNT3_LOW_RUNNING_COUNT(v)   (((v) << 16) & BM_TIMROT_TIMCOUNT3_LOW_RUNNING_COUNT)
#endif

//--- Register HW_TIMROT_TIMCOUNT3, field HIGH_FIXED_COUNT

#define BP_TIMROT_TIMCOUNT3_HIGH_FIXED_COUNT      0
#define BM_TIMROT_TIMCOUNT3_HIGH_FIXED_COUNT      0x0000FFFF

#define BF_TIMROT_TIMCOUNT3_HIGH_FIXED_COUNT(v)   (((v) << 0) & BM_TIMROT_TIMCOUNT3_HIGH_FIXED_COUNT)

#ifndef __LANGUAGE_ASM__
#define BW_TIMROT_TIMCOUNT3_HIGH_FIXED_COUNT(v)   (HW_TIMROT_TIMCOUNT3.B.HIGH_FIXED_COUNT = (v))
#endif


////////////////////////////////////////////////////////////////////////////////
//// HW_TIMROT_VERSION - TIMROT Version Register
////////////////////////////////////////////////////////////////////////////////

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t U;
    struct
    {
        unsigned STEP   : 16;
        unsigned MINOR  :  8;
        unsigned MAJOR  :  8;
    } B;
} hw_timrot_version_t;
#endif


//
// constants & macros for entire HW_TIMROT_VERSION register
//

#define HW_TIMROT_VERSION_ADDR      (REGS_TIMROT_BASE + 0x000000A0)

#ifndef __LANGUAGE_ASM__
#define HW_TIMROT_VERSION           (*(volatile hw_timrot_version_t *) HW_TIMROT_VERSION_ADDR)
#define HW_TIMROT_VERSION_RD()      (HW_TIMROT_VERSION.U)
#endif


//
// constants & macros for individual HW_TIMROT_VERSION bitfields
//

//--- Register HW_TIMROT_VERSION, field MAJOR

#define BP_TIMROT_VERSION_MAJOR      24
#define BM_TIMROT_VERSION_MAJOR      0xFF000000

#ifndef __LANGUAGE_ASM__
#define BF_TIMROT_VERSION_MAJOR(v)   ((((reg32_t) v) << 24) & BM_TIMROT_VERSION_MAJOR)
#else
#define BF_TIMROT_VERSION_MAJOR(v)   (((v) << 24) & BM_TIMROT_VERSION_MAJOR)
#endif

//--- Register HW_TIMROT_VERSION, field MINOR

#define BP_TIMROT_VERSION_MINOR      16
#define BM_TIMROT_VERSION_MINOR      0x00FF0000

#define BF_TIMROT_VERSION_MINOR(v)   (((v) << 16) & BM_TIMROT_VERSION_MINOR)

//--- Register HW_TIMROT_VERSION, field STEP

#define BP_TIMROT_VERSION_STEP      0
#define BM_TIMROT_VERSION_STEP      0x0000FFFF

#define BF_TIMROT_VERSION_STEP(v)   (((v) << 0) & BM_TIMROT_VERSION_STEP)


#endif // _REGSTIMROT_H

////////////////////////////////////////////////////////////////////////////////
