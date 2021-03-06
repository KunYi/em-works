//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: common_uartdbg.h
//
//  Provides definitions for the DEBUG UART (Universal Asynchronous Receiver 
//  Transmitter) module.
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

#ifndef _COMMON_UARTDBG_H
#define _COMMON_UARTDBG_H  1

#define REGS_UARTDBG_BASE (DWORD)pv_HWregUARTDbg

//------------------------------------------------------------------------------
//   HW_UARTDBGDR - UART Data Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned DATA         :  8;
        unsigned FE           :  1;
        unsigned PE           :  1;
        unsigned BE           :  1;
        unsigned OE           :  1;
        unsigned RESERVED     :  4;
        unsigned UNAVAILABLE  : 16;
    } B;
} hw_uartdbgdr_t;
#endif

//constants & macros for entire HW_UARTDBGDR register

#define HW_UARTDBGDR_ADDR         (REGS_UARTDBG_BASE + 0x0)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGDR           (*(volatile hw_uartdbgdr_t *) HW_UARTDBGDR_ADDR)
#define HW_UARTDBGDR_RD()      (HW_UARTDBGDR.U)
#define HW_UARTDBGDR_WR(v)     (HW_UARTDBGDR.U = (v))
#define HW_UARTDBGDR_SET(v)    (HW_UARTDBGDR_WR(HW_UARTDBGDR_RD() |  (v)))
#define HW_UARTDBGDR_CLR(v)    (HW_UARTDBGDR_WR(HW_UARTDBGDR_RD() & ~(v)))
#define HW_UARTDBGDR_TOG(v)    (HW_UARTDBGDR_WR(HW_UARTDBGDR_RD() ^  (v)))
#endif


//constants & macros for individual HW_UARTDBGDR bitfields

// Register HW_UARTDBGDR, field UNAVAILABLE
#define BP_UARTDBGDR_UNAVAILABLE      16
#define BM_UARTDBGDR_UNAVAILABLE      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGDR_UNAVAILABLE(v)   ((((reg32_t) v) << 16) & BM_UARTDBGDR_UNAVAILABLE)
#else
#define BF_UARTDBGDR_UNAVAILABLE(v)   (((v) << 16) & BM_UARTDBGDR_UNAVAILABLE)
#endif

//  Register HW_UARTDBGDR, field RESERVED 
#define BP_UARTDBGDR_RESERVED         12
#define BM_UARTDBGDR_RESERVED         0x0000F000

#define BF_UARTDBGDR_RESERVED(v)      (((v) << 12) & BM_UARTDBGDR_RESERVED)

//  Register HW_UARTDBGDR, field OE 
#define BP_UARTDBGDR_OE               11
#define BM_UARTDBGDR_OE               0x00000800

#define BF_UARTDBGDR_OE(v)            (((v) << 11) & BM_UARTDBGDR_OE)

//  Register HW_UARTDBGDR, field BE 
#define BP_UARTDBGDR_BE               10
#define BM_UARTDBGDR_BE               0x00000400

#define BF_UARTDBGDR_BE(v)            (((v) << 10) & BM_UARTDBGDR_BE)

//  Register HW_UARTDBGDR, field PE
#define BP_UARTDBGDR_PE               9
#define BM_UARTDBGDR_PE               0x00000200

#define BF_UARTDBGDR_PE(v)            (((v) << 9) & BM_UARTDBGDR_PE)

//  Register HW_UARTDBGDR, field FE
#define BP_UARTDBGDR_FE               8
#define BM_UARTDBGDR_FE               0x00000100

#define BF_UARTDBGDR_FE(v)            (((v) << 8) & BM_UARTDBGDR_FE)

//  Register HW_UARTDBGDR, field DATA
#define BP_UARTDBGDR_DATA             0
#define BM_UARTDBGDR_DATA             0x000000FF

#define BF_UARTDBGDR_DATA(v)          (((v) << 0) & BM_UARTDBGDR_DATA)
#ifndef __LANGUAGE_ASM__
#define BW_UARTDBGDR_DATA(v)          (HW_UARTDBGDR.B.DATA = (v))
#endif



//------------------------------------------------------------------------------
//  HW_UARTDBGRSR_ECR - UART Receive Status Register (Read) / Error Clear Register (Write)
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned FE           :  1;
        unsigned PE           :  1;
        unsigned BE           :  1;
        unsigned OE           :  1;
        unsigned EC           :  4;
        unsigned UNAVAILABLE  : 24;
    } B;
} hw_uartdbgrsr_ecr_t;
#endif


//  constants & macros for entire HW_UARTDBGRSR_ECR register

#define HW_UARTDBGRSR_ECR_ADDR         (REGS_UARTDBG_BASE + 0x4)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGRSR_ECR              (*(volatile hw_uartdbgrsr_ecr_t *) HW_UARTDBGRSR_ECR_ADDR)
#define HW_UARTDBGRSR_ECR_RD()         (HW_UARTDBGRSR_ECR.U)
#define HW_UARTDBGRSR_ECR_WR(v)        (HW_UARTDBGRSR_ECR.U = (v))
#define HW_UARTDBGRSR_ECR_SET(v)       (HW_UARTDBGRSR_ECR_WR(HW_UARTDBGRSR_ECR_RD() |  (v)))
#define HW_UARTDBGRSR_ECR_CLR(v)       (HW_UARTDBGRSR_ECR_WR(HW_UARTDBGRSR_ECR_RD() & ~(v)))
#define HW_UARTDBGRSR_ECR_TOG(v)       (HW_UARTDBGRSR_ECR_WR(HW_UARTDBGRSR_ECR_RD() ^  (v)))
#endif


//  constants & macros for individual HW_UARTDBGRSR_ECR bitfields

//  Register HW_UARTDBGRSR_ECR, field UNAVAILABLE
#define BP_UARTDBGRSR_ECR_UNAVAILABLE      8
#define BM_UARTDBGRSR_ECR_UNAVAILABLE      0xFFFFFF00

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGRSR_ECR_UNAVAILABLE(v)   ((((reg32_t) v) << 8) & BM_UARTDBGRSR_ECR_UNAVAILABLE)
#else
#define BF_UARTDBGRSR_ECR_UNAVAILABLE(v)   (((v) << 8) & BM_UARTDBGRSR_ECR_UNAVAILABLE)
#endif

//  Register HW_UARTDBGRSR_ECR, field EC 
#define BP_UARTDBGRSR_ECR_EC               4
#define BM_UARTDBGRSR_ECR_EC               0x000000F0

#define BF_UARTDBGRSR_ECR_EC(v)            (((v) << 4) & BM_UARTDBGRSR_ECR_EC)

//  Register HW_UARTDBGRSR_ECR, field OE
#define BP_UARTDBGRSR_ECR_OE               3
#define BM_UARTDBGRSR_ECR_OE               0x00000008

#define BF_UARTDBGRSR_ECR_OE(v)            (((v) << 3) & BM_UARTDBGRSR_ECR_OE)

//  Register HW_UARTDBGRSR_ECR, field BE
#define BP_UARTDBGRSR_ECR_BE               2
#define BM_UARTDBGRSR_ECR_BE               0x00000004

#define BF_UARTDBGRSR_ECR_BE(v)            (((v) << 2) & BM_UARTDBGRSR_ECR_BE)

//  Register HW_UARTDBGRSR_ECR, field PE
#define BP_UARTDBGRSR_ECR_PE               1
#define BM_UARTDBGRSR_ECR_PE               0x00000002

#define BF_UARTDBGRSR_ECR_PE(v)            (((v) << 1) & BM_UARTDBGRSR_ECR_PE)

//  Register HW_UARTDBGRSR_ECR, field FE
#define BP_UARTDBGRSR_ECR_FE               0
#define BM_UARTDBGRSR_ECR_FE               0x00000001

#define BF_UARTDBGRSR_ECR_FE(v)            (((v) << 0) & BM_UARTDBGRSR_ECR_FE)


//------------------------------------------------------------------------------
//  HW_UARTDBGFR - UART Flag Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned CTS          :  1;
        unsigned DSR          :  1;
        unsigned DCD          :  1;
        unsigned BUSY         :  1;
        unsigned RXFE         :  1;
        unsigned TXFF         :  1;
        unsigned RXFF         :  1;
        unsigned TXFE         :  1;
        unsigned RI           :  1;
        unsigned RESERVED     :  7;
        unsigned UNAVAILABLE  : 16;
    } B;
} hw_uartdbgfr_t;
#endif


//  constants & macros for entire HW_UARTDBGFR register

#define HW_UARTDBGFR_ADDR         (REGS_UARTDBG_BASE + 0x18)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGFR              (*(volatile hw_uartdbgfr_t *) HW_UARTDBGFR_ADDR)
#define HW_UARTDBGFR_RD()         (HW_UARTDBGFR.U)
#endif


//  constants & macros for individual HW_UARTDBGFR bitfields

//  Register HW_UARTDBGFR, field UNAVAILABLE 
#define BP_UARTDBGFR_UNAVAILABLE      16
#define BM_UARTDBGFR_UNAVAILABLE      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGFR_UNAVAILABLE(v)   ((((reg32_t) v) << 16) & BM_UARTDBGFR_UNAVAILABLE)
#else
#define BF_UARTDBGFR_UNAVAILABLE(v)   (((v) << 16) & BM_UARTDBGFR_UNAVAILABLE)
#endif
//  Register HW_UARTDBGFR, field RESERVED
#define BP_UARTDBGFR_RESERVED         9
#define BM_UARTDBGFR_RESERVED         0x0000FE00

#define BF_UARTDBGFR_RESERVED(v)      (((v) << 9) & BM_UARTDBGFR_RESERVED)

//  Register HW_UARTDBGFR, field RI
#define BP_UARTDBGFR_RI               8
#define BM_UARTDBGFR_RI               0x00000100

#define BF_UARTDBGFR_RI(v)            (((v) << 8) & BM_UARTDBGFR_RI)

//  Register HW_UARTDBGFR, field TXFE
#define BP_UARTDBGFR_TXFE             7
#define BM_UARTDBGFR_TXFE             0x00000080

#define BF_UARTDBGFR_TXFE(v)          (((v) << 7) & BM_UARTDBGFR_TXFE)

//  Register HW_UARTDBGFR, field RXFF
#define BP_UARTDBGFR_RXFF             6
#define BM_UARTDBGFR_RXFF             0x00000040

#define BF_UARTDBGFR_RXFF(v)          (((v) << 6) & BM_UARTDBGFR_RXFF)

//  Register HW_UARTDBGFR, field TXFF
#define BP_UARTDBGFR_TXFF             5
#define BM_UARTDBGFR_TXFF             0x00000020

#define BF_UARTDBGFR_TXFF(v)          (((v) << 5) & BM_UARTDBGFR_TXFF)

//  Register HW_UARTDBGFR, field RXFE
#define BP_UARTDBGFR_RXFE             4
#define BM_UARTDBGFR_RXFE             0x00000010

#define BF_UARTDBGFR_RXFE(v)          (((v) << 4) & BM_UARTDBGFR_RXFE)

//  Register HW_UARTDBGFR, field BUSY
#define BP_UARTDBGFR_BUSY             3
#define BM_UARTDBGFR_BUSY             0x00000008

#define BF_UARTDBGFR_BUSY(v)          (((v) << 3) & BM_UARTDBGFR_BUSY)

//  Register HW_UARTDBGFR, field DCD
#define BP_UARTDBGFR_DCD              2
#define BM_UARTDBGFR_DCD              0x00000004

#define BF_UARTDBGFR_DCD(v)           (((v) << 2) & BM_UARTDBGFR_DCD)

//  Register HW_UARTDBGFR, field DSR
#define BP_UARTDBGFR_DSR              1
#define BM_UARTDBGFR_DSR              0x00000002

#define BF_UARTDBGFR_DSR(v)           (((v) << 1) & BM_UARTDBGFR_DSR)

//  Register HW_UARTDBGFR, field CTS
#define BP_UARTDBGFR_CTS              0
#define BM_UARTDBGFR_CTS              0x00000001

#define BF_UARTDBGFR_CTS(v)           (((v) << 0) & BM_UARTDBGFR_CTS)


//------------------------------------------------------------------------------
//  HW_UARTDBGILPR - UART IrDA Low-Power Counter Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned ILPDVSR      :  8;
        unsigned UNAVAILABLE  : 24;
    } B;
} hw_uartdbgilpr_t;
#endif


//  constants & macros for entire HW_UARTDBGILPR register

#define HW_UARTDBGILPR_ADDR         (REGS_UARTDBG_BASE + 0x20)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGILPR              (*(volatile hw_uartdbgilpr_t *) HW_UARTDBGILPR_ADDR)
#define HW_UARTDBGILPR_RD()         (HW_UARTDBGILPR.U)
#define HW_UARTDBGILPR_WR(v)        (HW_UARTDBGILPR.U = (v))
#define HW_UARTDBGILPR_SET(v)       (HW_UARTDBGILPR_WR(HW_UARTDBGILPR_RD() |  (v)))
#define HW_UARTDBGILPR_CLR(v)       (HW_UARTDBGILPR_WR(HW_UARTDBGILPR_RD() & ~(v)))
#define HW_UARTDBGILPR_TOG(v)       (HW_UARTDBGILPR_WR(HW_UARTDBGILPR_RD() ^  (v)))
#endif


//  constants & macros for individual HW_UARTDBGILPR bitfields

//  Register HW_UARTDBGILPR, field UNAVAILABLE
#define BP_UARTDBGILPR_UNAVAILABLE      8
#define BM_UARTDBGILPR_UNAVAILABLE      0xFFFFFF00

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGILPR_UNAVAILABLE(v)   ((((reg32_t) v) << 8) & BM_UARTDBGILPR_UNAVAILABLE)
#else
#define BF_UARTDBGILPR_UNAVAILABLE(v)   (((v) << 8) & BM_UARTDBGILPR_UNAVAILABLE)
#endif

//  Register HW_UARTDBGILPR, field ILPDVSR
#define BP_UARTDBGILPR_ILPDVSR          0
#define BM_UARTDBGILPR_ILPDVSR          0x000000FF

#define BF_UARTDBGILPR_ILPDVSR(v)       (((v) << 0) & BM_UARTDBGILPR_ILPDVSR)
#ifndef __LANGUAGE_ASM__
#define BW_UARTDBGILPR_ILPDVSR(v)       (HW_UARTDBGILPR.B.ILPDVSR = (v))
#endif


//------------------------------------------------------------------------------
//  HW_UARTDBGIBRD - UART Integer Baud Rate Divisor Register
 //------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned BAUD_DIVINT  : 16;
        unsigned UNAVAILABLE  : 16;
    } B;
} hw_uartdbgibrd_t;
#endif


//  constants & macros for entire HW_UARTDBGIBRD register

#define HW_UARTDBGIBRD_ADDR         (REGS_UARTDBG_BASE + 0x24)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGIBRD              (*(volatile hw_uartdbgibrd_t *) HW_UARTDBGIBRD_ADDR)
#define HW_UARTDBGIBRD_RD()         (HW_UARTDBGIBRD.U)
#define HW_UARTDBGIBRD_WR(v)        (HW_UARTDBGIBRD.U = (v))
#define HW_UARTDBGIBRD_SET(v)       (HW_UARTDBGIBRD_WR(HW_UARTDBGIBRD_RD() |  (v)))
#define HW_UARTDBGIBRD_CLR(v)       (HW_UARTDBGIBRD_WR(HW_UARTDBGIBRD_RD() & ~(v)))
#define HW_UARTDBGIBRD_TOG(v)       (HW_UARTDBGIBRD_WR(HW_UARTDBGIBRD_RD() ^  (v)))
#endif


//------------------------------------------------------------------------------
//  constants & macros for individual HW_UARTDBGIBRD bitfields
//------------------------------------------------------------------------------

//  Register HW_UARTDBGIBRD, field UNAVAILABLE
#define BP_UARTDBGIBRD_UNAVAILABLE      16
#define BM_UARTDBGIBRD_UNAVAILABLE      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGIBRD_UNAVAILABLE(v)   ((((reg32_t) v) << 16) & BM_UARTDBGIBRD_UNAVAILABLE)
#else
#define BF_UARTDBGIBRD_UNAVAILABLE(v)   (((v) << 16) & BM_UARTDBGIBRD_UNAVAILABLE)
#endif

//  Register HW_UARTDBGIBRD, field BAUD_DIVINT
#define BP_UARTDBGIBRD_BAUD_DIVINT      0
#define BM_UARTDBGIBRD_BAUD_DIVINT      0x0000FFFF

#define BF_UARTDBGIBRD_BAUD_DIVINT(v)   (((v) << 0) & BM_UARTDBGIBRD_BAUD_DIVINT)
#ifndef __LANGUAGE_ASM__
#define BW_UARTDBGIBRD_BAUD_DIVINT(v)   (HW_UARTDBGIBRD.B.BAUD_DIVINT = (v))
#endif



//------------------------------------------------------------------------------
//  HW_UARTDBGFBRD - UART Fractional Baud Rate Divisor Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned BAUD_DIVFRAC  :  6;
        unsigned RESERVED      :  2;
        unsigned UNAVAILABLE   : 24;
    } B;
} hw_uartdbgfbrd_t;
#endif

//------------------------------------------------------------------------------
//  constants & macros for entire HW_UARTDBGFBRD register
//------------------------------------------------------------------------------

#define HW_UARTDBGFBRD_ADDR         (REGS_UARTDBG_BASE + 0x28)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGFBRD              (*(volatile hw_uartdbgfbrd_t *) HW_UARTDBGFBRD_ADDR)
#define HW_UARTDBGFBRD_RD()         (HW_UARTDBGFBRD.U)
#define HW_UARTDBGFBRD_WR(v)        (HW_UARTDBGFBRD.U = (v))
#define HW_UARTDBGFBRD_SET(v)       (HW_UARTDBGFBRD_WR(HW_UARTDBGFBRD_RD() |  (v)))
#define HW_UARTDBGFBRD_CLR(v)       (HW_UARTDBGFBRD_WR(HW_UARTDBGFBRD_RD() & ~(v)))
#define HW_UARTDBGFBRD_TOG(v)       (HW_UARTDBGFBRD_WR(HW_UARTDBGFBRD_RD() ^  (v)))
#endif


//  constants & macros for individual HW_UARTDBGFBRD bitfields

//  Register HW_UARTDBGFBRD, field UNAVAILABLE
#define BP_UARTDBGFBRD_UNAVAILABLE      8
#define BM_UARTDBGFBRD_UNAVAILABLE      0xFFFFFF00

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGFBRD_UNAVAILABLE(v)   ((((reg32_t) v) << 8) & BM_UARTDBGFBRD_UNAVAILABLE)
#else
#define BF_UARTDBGFBRD_UNAVAILABLE(v)   (((v) << 8) & BM_UARTDBGFBRD_UNAVAILABLE)
#endif

//  Register HW_UARTDBGFBRD, field RESERVED
#define BP_UARTDBGFBRD_RESERVED         6
#define BM_UARTDBGFBRD_RESERVED         0x000000C0

#define BF_UARTDBGFBRD_RESERVED(v)   (((v) << 6) & BM_UARTDBGFBRD_RESERVED)
//  Register HW_UARTDBGFBRD, field BAUD_DIVFRAC

#define BP_UARTDBGFBRD_BAUD_DIVFRAC     0
#define BM_UARTDBGFBRD_BAUD_DIVFRAC     0x0000003F

#define BF_UARTDBGFBRD_BAUD_DIVFRAC(v)  (((v) << 0) & BM_UARTDBGFBRD_BAUD_DIVFRAC)


//------------------------------------------------------------------------------
//  HW_UARTDBGLCR_H - UART Line Control Register, HIGH Byte
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned BRK          :  1;
        unsigned PEN          :  1;
        unsigned EPS          :  1;
        unsigned STP2         :  1;
        unsigned FEN          :  1;
        unsigned WLEN         :  2;
        unsigned SPS          :  1;
        unsigned RESERVED     :  8;
        unsigned UNAVAILABLE  : 16;
    } B;
} hw_uartdbglcr_h_t;
#endif


//  constants & macros for entire HW_UARTDBGLCR_H register

#define HW_UARTDBGLCR_H_ADDR         (REGS_UARTDBG_BASE + 0x2c)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGLCR_H              (*(volatile hw_uartdbglcr_h_t *) HW_UARTDBGLCR_H_ADDR)
#define HW_UARTDBGLCR_H_RD()         (HW_UARTDBGLCR_H.U)
#define HW_UARTDBGLCR_H_WR(v)        (HW_UARTDBGLCR_H.U = (v))
#define HW_UARTDBGLCR_H_SET(v)       (HW_UARTDBGLCR_H_WR(HW_UARTDBGLCR_H_RD() |  (v)))
#define HW_UARTDBGLCR_H_CLR(v)       (HW_UARTDBGLCR_H_WR(HW_UARTDBGLCR_H_RD() & ~(v)))
#define HW_UARTDBGLCR_H_TOG(v)       (HW_UARTDBGLCR_H_WR(HW_UARTDBGLCR_H_RD() ^  (v)))
#endif


//constants & macros for individual HW_UARTDBGLCR_H bitfields

//  Register HW_UARTDBGLCR_H, field UNAVAILABLE
#define BP_UARTDBGLCR_H_UNAVAILABLE      16
#define BM_UARTDBGLCR_H_UNAVAILABLE      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGLCR_H_UNAVAILABLE(v)   ((((reg32_t) v) << 16) & BM_UARTDBGLCR_H_UNAVAILABLE)
#else
#define BF_UARTDBGLCR_H_UNAVAILABLE(v)   (((v) << 16) & BM_UARTDBGLCR_H_UNAVAILABLE)
#endif

//  Register HW_UARTDBGLCR_H, field RESERVED
#define BP_UARTDBGLCR_H_RESERVED         8
#define BM_UARTDBGLCR_H_RESERVED         0x0000FF00

#define BF_UARTDBGLCR_H_RESERVED(v)      (((v) << 8) & BM_UARTDBGLCR_H_RESERVED)

//  Register HW_UARTDBGLCR_H, field SPS
#define BP_UARTDBGLCR_H_SPS              7
#define BM_UARTDBGLCR_H_SPS              0x00000080

#define BF_UARTDBGLCR_H_SPS(v)           (((v) << 7) & BM_UARTDBGLCR_H_SPS)

//  Register HW_UARTDBGLCR_H, field WLEN

#define BP_UARTDBGLCR_H_WLEN             5
#define BM_UARTDBGLCR_H_WLEN             0x00000060

#define BF_UARTDBGLCR_H_WLEN(v)          (((v) << 5) & BM_UARTDBGLCR_H_WLEN)

#define UARTDBG_LINECTRL_WLEN_5          0x00
#define UARTDBG_LINECTRL_WLEN_6          0x01
#define UARTDBG_LINECTRL_WLEN_7          0x02
#define UARTDBG_LINECTRL_WLEN_8          0x03

//  Register HW_UARTDBGLCR_H, field FEN
#define BP_UARTDBGLCR_H_FEN              4
#define BM_UARTDBGLCR_H_FEN              0x00000010

#define BF_UARTDBGLCR_H_FEN(v)           (((v) << 4) & BM_UARTDBGLCR_H_FEN)

//  Register HW_UARTDBGLCR_H, field STP2
#define BP_UARTDBGLCR_H_STP2             3
#define BM_UARTDBGLCR_H_STP2             0x00000008

#define BF_UARTDBGLCR_H_STP2(v)          (((v) << 3) & BM_UARTDBGLCR_H_STP2)

//  Register HW_UARTDBGLCR_H, field EPS
#define BP_UARTDBGLCR_H_EPS              2
#define BM_UARTDBGLCR_H_EPS              0x00000004

#define BF_UARTDBGLCR_H_EPS(v)           (((v) << 2) & BM_UARTDBGLCR_H_EPS)

//  Register HW_UARTDBGLCR_H, field PEN
#define BP_UARTDBGLCR_H_PEN              1
#define BM_UARTDBGLCR_H_PEN              0x00000002

#define BF_UARTDBGLCR_H_PEN(v)           (((v) << 1) & BM_UARTDBGLCR_H_PEN)

//  Register HW_UARTDBGLCR_H, field BRK
#define BP_UARTDBGLCR_H_BRK              0
#define BM_UARTDBGLCR_H_BRK              0x00000001

#define BF_UARTDBGLCR_H_BRK(v)           (((v) << 0) & BM_UARTDBGLCR_H_BRK)


//------------------------------------------------------------------------------
//  HW_UARTDBGCR - UART Control Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned UARTEN       :  1;
        unsigned SIREN        :  1;
        unsigned SIRLP        :  1;
        unsigned RESERVED     :  4;
        unsigned LBE          :  1;
        unsigned TXE          :  1;
        unsigned RXE          :  1;
        unsigned DTR          :  1;
        unsigned RTS          :  1;
        unsigned OUT1         :  1;
        unsigned OUT2         :  1;
        unsigned RTSEN        :  1;
        unsigned CTSEN        :  1;
        unsigned UNAVAILABLE  : 16;
    } B;
} hw_uartdbgcr_t;
#endif


// constants & macros for entire HW_UARTDBGCR register

#define HW_UARTDBGCR_ADDR         (REGS_UARTDBG_BASE + 0x30)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGCR              (*(volatile hw_uartdbgcr_t *) HW_UARTDBGCR_ADDR)
#define HW_UARTDBGCR_RD()         (HW_UARTDBGCR.U)
#define HW_UARTDBGCR_WR(v)        (HW_UARTDBGCR.U = (v))
#define HW_UARTDBGCR_SET(v)       (HW_UARTDBGCR_WR(HW_UARTDBGCR_RD() |  (v)))
#define HW_UARTDBGCR_CLR(v)       (HW_UARTDBGCR_WR(HW_UARTDBGCR_RD() & ~(v)))
#define HW_UARTDBGCR_TOG(v)       (HW_UARTDBGCR_WR(HW_UARTDBGCR_RD() ^  (v)))
#endif


//------------------------------------------------------------------------------
//  constants & macros for individual HW_UARTDBGCR bitfields
//------------------------------------------------------------------------------

//  Register HW_UARTDBGCR, field UNAVAILABLE
#define BP_UARTDBGCR_UNAVAILABLE      16
#define BM_UARTDBGCR_UNAVAILABLE      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGCR_UNAVAILABLE(v)   ((((reg32_t) v) << 16) & BM_UARTDBGCR_UNAVAILABLE)
#else
#define BF_UARTDBGCR_UNAVAILABLE(v)   (((v) << 16) & BM_UARTDBGCR_UNAVAILABLE)
#endif

//  Register HW_UARTDBGCR, field CTSEN
#define BP_UARTDBGCR_CTSEN            15
#define BM_UARTDBGCR_CTSEN            0x00008000

#define BF_UARTDBGCR_CTSEN(v)         (((v) << 15) & BM_UARTDBGCR_CTSEN)

//  Register HW_UARTDBGCR, field RTSEN
#define BP_UARTDBGCR_RTSEN            14
#define BM_UARTDBGCR_RTSEN            0x00004000

#define BF_UARTDBGCR_RTSEN(v)         (((v) << 14) & BM_UARTDBGCR_RTSEN)

//  Register HW_UARTDBGCR, field OUT2
#define BP_UARTDBGCR_OUT2             13
#define BM_UARTDBGCR_OUT2             0x00002000

#define BF_UARTDBGCR_OUT2(v)          (((v) << 13) & BM_UARTDBGCR_OUT2)

//  Register HW_UARTDBGCR, field OUT1
#define BP_UARTDBGCR_OUT1             12
#define BM_UARTDBGCR_OUT1             0x00001000

#define BF_UARTDBGCR_OUT1(v)          (((v) << 12) & BM_UARTDBGCR_OUT1)

//  Register HW_UARTDBGCR, field RTS
#define BP_UARTDBGCR_RTS              11
#define BM_UARTDBGCR_RTS              0x00000800

#define BF_UARTDBGCR_RTS(v)           (((v) << 11) & BM_UARTDBGCR_RTS)

//  Register HW_UARTDBGCR, field DTR
#define BP_UARTDBGCR_DTR              10
#define BM_UARTDBGCR_DTR              0x00000400

#define BF_UARTDBGCR_DTR(v)           (((v) << 10) & BM_UARTDBGCR_DTR)

//  Register HW_UARTDBGCR, field RXE
#define BP_UARTDBGCR_RXE              9
#define BM_UARTDBGCR_RXE              0x00000200

#define BF_UARTDBGCR_RXE(v)           (((v) << 9) & BM_UARTDBGCR_RXE)

//  Register HW_UARTDBGCR, field TXE
#define BP_UARTDBGCR_TXE              8
#define BM_UARTDBGCR_TXE              0x00000100

#define BF_UARTDBGCR_TXE(v)           (((v) << 8) & BM_UARTDBGCR_TXE)

//  Register HW_UARTDBGCR, field LBE
#define BP_UARTDBGCR_LBE              7
#define BM_UARTDBGCR_LBE              0x00000080

#define BF_UARTDBGCR_LBE(v)           (((v) << 7) & BM_UARTDBGCR_LBE)

//  Register HW_UARTDBGCR, field RESERVED
#define BP_UARTDBGCR_RESERVED         3
#define BM_UARTDBGCR_RESERVED         0x00000078

#define BF_UARTDBGCR_RESERVED(v)      (((v) << 3) & BM_UARTDBGCR_RESERVED)

//  Register HW_UARTDBGCR, field SIRLP

#define BP_UARTDBGCR_SIRLP            2
#define BM_UARTDBGCR_SIRLP            0x00000004

#define BF_UARTDBGCR_SIRLP(v)         (((v) << 2) & BM_UARTDBGCR_SIRLP)

//  Register HW_UARTDBGCR, field SIREN
#define BP_UARTDBGCR_SIREN            1
#define BM_UARTDBGCR_SIREN            0x00000002

#define BF_UARTDBGCR_SIREN(v)         (((v) << 1) & BM_UARTDBGCR_SIREN)

//  Register HW_UARTDBGCR, field UARTEN
#define BP_UARTDBGCR_UARTEN           0
#define BM_UARTDBGCR_UARTEN           0x00000001

#define BF_UARTDBGCR_UARTEN(v)        (((v) << 0) & BM_UARTDBGCR_UARTEN)


//------------------------------------------------------------------------------
//  HW_UARTDBGIFLS - UART Interrupt FIFO Level Select Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned TXIFLSEL     :  3;
        unsigned RXIFLSEL     :  3;
        unsigned RESERVED     : 10;
        unsigned UNAVAILABLE  : 16;
    } B;
} hw_uartdbgifls_t;
#endif


//  constants & macros for entire HW_UARTDBGIFLS register

#define HW_UARTDBGIFLS_ADDR         (REGS_UARTDBG_BASE + 0x34)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGIFLS              (*(volatile hw_uartdbgifls_t *) HW_UARTDBGIFLS_ADDR)
#define HW_UARTDBGIFLS_RD()         (HW_UARTDBGIFLS.U)
#define HW_UARTDBGIFLS_WR(v)        (HW_UARTDBGIFLS.U = (v))
#define HW_UARTDBGIFLS_SET(v)       (HW_UARTDBGIFLS_WR(HW_UARTDBGIFLS_RD() |  (v)))
#define HW_UARTDBGIFLS_CLR(v)       (HW_UARTDBGIFLS_WR(HW_UARTDBGIFLS_RD() & ~(v)))
#define HW_UARTDBGIFLS_TOG(v)       (HW_UARTDBGIFLS_WR(HW_UARTDBGIFLS_RD() ^  (v)))
#endif


//  constants & macros for individual HW_UARTDBGIFLS bitfields

//  Register HW_UARTDBGIFLS, field UNAVAILABLE
#define BP_UARTDBGIFLS_UNAVAILABLE      16
#define BM_UARTDBGIFLS_UNAVAILABLE      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGIFLS_UNAVAILABLE(v)   ((((reg32_t) v) << 16) & BM_UARTDBGIFLS_UNAVAILABLE)
#else
#define BF_UARTDBGIFLS_UNAVAILABLE(v)   (((v) << 16) & BM_UARTDBGIFLS_UNAVAILABLE)
#endif

//  Register HW_UARTDBGIFLS, field RESERVED
#define BP_UARTDBGIFLS_RESERVED         6
#define BM_UARTDBGIFLS_RESERVED         0x0000FFC0

#define BF_UARTDBGIFLS_RESERVED(v)      (((v) << 6) & BM_UARTDBGIFLS_RESERVED)

//  Register HW_UARTDBGIFLS, field RXIFLSEL
#define BP_UARTDBGIFLS_RXIFLSEL         3
#define BM_UARTDBGIFLS_RXIFLSEL         0x00000038

#define BF_UARTDBGIFLS_RXIFLSEL(v)      (((v) << 3) & BM_UARTDBGIFLS_RXIFLSEL)

#define BV_UARTDBGIFLS_RXIFLSEL__ONE_EIGHT       0x0
#define BV_UARTDBGIFLS_RXIFLSEL__ONE_QUARTER     0x1
#define BV_UARTDBGIFLS_RXIFLSEL__ONE_HALF        0x2
#define BV_UARTDBGIFLS_RXIFLSEL__THREE_QUARTERS  0x3
#define BV_UARTDBGIFLS_RXIFLSEL__SEVEN_EIGHTHS   0x4
#define BV_UARTDBGIFLS_RXIFLSEL__INVALID5        0x5
#define BV_UARTDBGIFLS_RXIFLSEL__INVALID6        0x6
#define BV_UARTDBGIFLS_RXIFLSEL__INVALID7        0x7

//  Register HW_UARTDBGIFLS, field TXIFLSEL
#define BP_UARTDBGIFLS_TXIFLSEL         0
#define BM_UARTDBGIFLS_TXIFLSEL         0x00000007

#define BF_UARTDBGIFLS_TXIFLSEL(v)      (((v) << 0) & BM_UARTDBGIFLS_TXIFLSEL)

#define BV_UARTDBGIFLS_TXIFLSEL__ONE_EIGHT       0x0
#define BV_UARTDBGIFLS_TXIFLSEL__ONE_QUARTER     0x1
#define BV_UARTDBGIFLS_TXIFLSEL__ONE_HALF        0x2
#define BV_UARTDBGIFLS_TXIFLSEL__THREE_QUARTERS  0x3
#define BV_UARTDBGIFLS_TXIFLSEL__SEVEN_EIGHTHS   0x4
#define BV_UARTDBGIFLS_TXIFLSEL__INVALID5        0x5
#define BV_UARTDBGIFLS_TXIFLSEL__INVALID6        0x6
#define BV_UARTDBGIFLS_TXIFLSEL__INVALID7        0x7


//------------------------------------------------------------------------------
//  HW_UARTDBGIMSC - UART Interrupt Mask Set/Clear Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned RIMIM        :  1;
        unsigned CTSMIM       :  1;
        unsigned DCDMIM       :  1;
        unsigned DSRMIM       :  1;
        unsigned RXIM         :  1;
        unsigned TXIM         :  1;
        unsigned RTIM         :  1;
        unsigned FEIM         :  1;
        unsigned PEIM         :  1;
        unsigned BEIM         :  1;
        unsigned OEIM         :  1;
        unsigned RESERVED     :  5;
        unsigned UNAVAILABLE  : 16;
    } B;
} hw_uartdbgimsc_t;
#endif


//  constants & macros for entire HW_UARTDBGIMSC register

#define HW_UARTDBGIMSC_ADDR         (REGS_UARTDBG_BASE + 0x38)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGIMSC              (*(volatile hw_uartdbgimsc_t *) HW_UARTDBGIMSC_ADDR)
#define HW_UARTDBGIMSC_RD()         (HW_UARTDBGIMSC.U)
#define HW_UARTDBGIMSC_WR(v)        (HW_UARTDBGIMSC.U = (v))
#define HW_UARTDBGIMSC_SET(v)       (HW_UARTDBGIMSC_WR(HW_UARTDBGIMSC_RD() |  (v)))
#define HW_UARTDBGIMSC_CLR(v)       (HW_UARTDBGIMSC_WR(HW_UARTDBGIMSC_RD() & ~(v)))
#define HW_UARTDBGIMSC_TOG(v)       (HW_UARTDBGIMSC_WR(HW_UARTDBGIMSC_RD() ^  (v)))
#endif


//  constants & macros for individual HW_UARTDBGIMSC bitfields

//  Register HW_UARTDBGIMSC, field UNAVAILABLE
#define BP_UARTDBGIMSC_UNAVAILABLE      16
#define BM_UARTDBGIMSC_UNAVAILABLE      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGIMSC_UNAVAILABLE(v)   ((((reg32_t) v) << 16) & BM_UARTDBGIMSC_UNAVAILABLE)
#else
#define BF_UARTDBGIMSC_UNAVAILABLE(v)   (((v) << 16) & BM_UARTDBGIMSC_UNAVAILABLE)
#endif

//  Register HW_UARTDBGIMSC, field RESERVED
#define BP_UARTDBGIMSC_RESERVED         11
#define BM_UARTDBGIMSC_RESERVED         0x0000F800

#define BF_UARTDBGIMSC_RESERVED(v)      (((v) << 11) & BM_UARTDBGIMSC_RESERVED)

//  Register HW_UARTDBGIMSC, field OEIM
#define BP_UARTDBGIMSC_OEIM             10
#define BM_UARTDBGIMSC_OEIM             0x00000400

#define BF_UARTDBGIMSC_OEIM(v)          (((v) << 10) & BM_UARTDBGIMSC_OEIM)

//  Register HW_UARTDBGIMSC, field BEIM
#define BP_UARTDBGIMSC_BEIM             9
#define BM_UARTDBGIMSC_BEIM             0x00000200

#define BF_UARTDBGIMSC_BEIM(v)          (((v) << 9) & BM_UARTDBGIMSC_BEIM)

//  Register HW_UARTDBGIMSC, field PEIM
#define BP_UARTDBGIMSC_PEIM             8
#define BM_UARTDBGIMSC_PEIM             0x00000100

#define BF_UARTDBGIMSC_PEIM(v)          (((v) << 8) & BM_UARTDBGIMSC_PEIM)

//  Register HW_UARTDBGIMSC, field FEIM
#define BP_UARTDBGIMSC_FEIM             7
#define BM_UARTDBGIMSC_FEIM             0x00000080

#define BF_UARTDBGIMSC_FEIM(v)          (((v) << 7) & BM_UARTDBGIMSC_FEIM)

//  Register HW_UARTDBGIMSC, field RTIM
#define BP_UARTDBGIMSC_RTIM             6
#define BM_UARTDBGIMSC_RTIM             0x00000040

#define BF_UARTDBGIMSC_RTIM(v)          (((v) << 6) & BM_UARTDBGIMSC_RTIM)

//  Register HW_UARTDBGIMSC, field TXIM
#define BP_UARTDBGIMSC_TXIM             5
#define BM_UARTDBGIMSC_TXIM             0x00000020

#define BF_UARTDBGIMSC_TXIM(v)          (((v) << 5) & BM_UARTDBGIMSC_TXIM)

//  Register HW_UARTDBGIMSC, field RXIM
#define BP_UARTDBGIMSC_RXIM             4
#define BM_UARTDBGIMSC_RXIM             0x00000010

#define BF_UARTDBGIMSC_RXIM(v)          (((v) << 4) & BM_UARTDBGIMSC_RXIM)

//  Register HW_UARTDBGIMSC, field DSRMIM
#define BP_UARTDBGIMSC_DSRMIM           3
#define BM_UARTDBGIMSC_DSRMIM           0x00000008

#define BF_UARTDBGIMSC_DSRMIM(v)        (((v) << 3) & BM_UARTDBGIMSC_DSRMIM)

//  Register HW_UARTDBGIMSC, field DCDMIM
#define BP_UARTDBGIMSC_DCDMIM           2
#define BM_UARTDBGIMSC_DCDMIM           0x00000004

#define BF_UARTDBGIMSC_DCDMIM(v)        (((v) << 2) & BM_UARTDBGIMSC_DCDMIM)

//  Register HW_UARTDBGIMSC, field CTSMIM
#define BP_UARTDBGIMSC_CTSMIM           1
#define BM_UARTDBGIMSC_CTSMIM           0x00000002

#define BF_UARTDBGIMSC_CTSMIM(v)        (((v) << 1) & BM_UARTDBGIMSC_CTSMIM)

//  Register HW_UARTDBGIMSC, field RIMIM
#define BP_UARTDBGIMSC_RIMIM            0
#define BM_UARTDBGIMSC_RIMIM            0x00000001

#define BF_UARTDBGIMSC_RIMIM(v)         (((v) << 0) & BM_UARTDBGIMSC_RIMIM)


//------------------------------------------------------------------------------
//  HW_UARTDBGRIS - UART Raw Interrupt Status Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned RIRMIS       :  1;
        unsigned CTSRMIS      :  1;
        unsigned DCDRMIS      :  1;
        unsigned DSRRMIS      :  1;
        unsigned RXRIS        :  1;
        unsigned TXRIS        :  1;
        unsigned RTRIS        :  1;
        unsigned FERIS        :  1;
        unsigned PERIS        :  1;
        unsigned BERIS        :  1;
        unsigned OERIS        :  1;
        unsigned RESERVED     :  5;
        unsigned UNAVAILABLE  : 16;
    } B;
} hw_uartdbgris_t;
#endif


//  constants & macros for entire HW_UARTDBGRIS register

#define HW_UARTDBGRIS_ADDR         (REGS_UARTDBG_BASE + 0x3c)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGRIS              (*(volatile hw_uartdbgris_t *) HW_UARTDBGRIS_ADDR)
#define HW_UARTDBGRIS_RD()         (HW_UARTDBGRIS.U)
#endif


//  constants & macros for individual HW_UARTDBGRIS bitfields

//  Register HW_UARTDBGRIS, field UNAVAILABLE
#define BP_UARTDBGRIS_UNAVAILABLE      16
#define BM_UARTDBGRIS_UNAVAILABLE      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGRIS_UNAVAILABLE(v)   ((((reg32_t) v) << 16) & BM_UARTDBGRIS_UNAVAILABLE)
#else
#define BF_UARTDBGRIS_UNAVAILABLE(v)   (((v) << 16) & BM_UARTDBGRIS_UNAVAILABLE)
#endif
//  Register HW_UARTDBGRIS, field RESERVED
#define BP_UARTDBGRIS_RESERVED         11
#define BM_UARTDBGRIS_RESERVED         0x0000F800

#define BF_UARTDBGRIS_RESERVED(v)      (((v) << 11) & BM_UARTDBGRIS_RESERVED)

//  Register HW_UARTDBGRIS, field OERIS
#define BP_UARTDBGRIS_OERIS            10
#define BM_UARTDBGRIS_OERIS            0x00000400

#define BF_UARTDBGRIS_OERIS(v)         (((v) << 10) & BM_UARTDBGRIS_OERIS)

//  Register HW_UARTDBGRIS, field BERIS
#define BP_UARTDBGRIS_BERIS            9
#define BM_UARTDBGRIS_BERIS            0x00000200

#define BF_UARTDBGRIS_BERIS(v)         (((v) << 9) & BM_UARTDBGRIS_BERIS)

//  Register HW_UARTDBGRIS, field PERIS
#define BP_UARTDBGRIS_PERIS            8
#define BM_UARTDBGRIS_PERIS            0x00000100

#define BF_UARTDBGRIS_PERIS(v)         (((v) << 8) & BM_UARTDBGRIS_PERIS)

//  Register HW_UARTDBGRIS, field FERIS
#define BP_UARTDBGRIS_FERIS            7
#define BM_UARTDBGRIS_FERIS            0x00000080

#define BF_UARTDBGRIS_FERIS(v)         (((v) << 7) & BM_UARTDBGRIS_FERIS)

//  Register HW_UARTDBGRIS, field RTRIS
#define BP_UARTDBGRIS_RTRIS            6
#define BM_UARTDBGRIS_RTRIS            0x00000040

#define BF_UARTDBGRIS_RTRIS(v)   (((v) << 6) & BM_UARTDBGRIS_RTRIS)

//  Register HW_UARTDBGRIS, field TXRIS
#define BP_UARTDBGRIS_TXRIS            5
#define BM_UARTDBGRIS_TXRIS            0x00000020

#define BF_UARTDBGRIS_TXRIS(v)         (((v) << 5) & BM_UARTDBGRIS_TXRIS)

//  Register HW_UARTDBGRIS, field RXRIS
#define BP_UARTDBGRIS_RXRIS            4
#define BM_UARTDBGRIS_RXRIS            0x00000010

#define BF_UARTDBGRIS_RXRIS(v)         (((v) << 4) & BM_UARTDBGRIS_RXRIS)

//  Register HW_UARTDBGRIS, field DSRRMIS
#define BP_UARTDBGRIS_DSRRMIS          3
#define BM_UARTDBGRIS_DSRRMIS          0x00000008

#define BF_UARTDBGRIS_DSRRMIS(v)       (((v) << 3) & BM_UARTDBGRIS_DSRRMIS)

//  Register HW_UARTDBGRIS, field DCDRMIS
#define BP_UARTDBGRIS_DCDRMIS          2
#define BM_UARTDBGRIS_DCDRMIS          0x00000004

#define BF_UARTDBGRIS_DCDRMIS(v)       (((v) << 2) & BM_UARTDBGRIS_DCDRMIS)

//  Register HW_UARTDBGRIS, field CTSRMIS
#define BP_UARTDBGRIS_CTSRMIS          1
#define BM_UARTDBGRIS_CTSRMIS          0x00000002

#define BF_UARTDBGRIS_CTSRMIS(v)       (((v) << 1) & BM_UARTDBGRIS_CTSRMIS)

//  Register HW_UARTDBGRIS, field RIRMIS
#define BP_UARTDBGRIS_RIRMIS           0
#define BM_UARTDBGRIS_RIRMIS           0x00000001

#define BF_UARTDBGRIS_RIRMIS(v)        (((v) << 0) & BM_UARTDBGRIS_RIRMIS)


//------------------------------------------------------------------------------
//  HW_UARTDBGMIS - UART Masked Interrupt Status Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned RIMMIS       :  1;
        unsigned CTSMMIS      :  1;
        unsigned DCDMMIS      :  1;
        unsigned DSRMMIS      :  1;
        unsigned RXMIS        :  1;
        unsigned TXMIS        :  1;
        unsigned RTMIS        :  1;
        unsigned FEMIS        :  1;
        unsigned PEMIS        :  1;
        unsigned BEMIS        :  1;
        unsigned OEMIS        :  1;
        unsigned RESERVED     :  5;
        unsigned UNAVAILABLE  : 16;
    } B;
} hw_uartdbgmis_t;
#endif


//  constants & macros for entire HW_UARTDBGMIS register

#define HW_UARTDBGMIS_ADDR         (REGS_UARTDBG_BASE + 0x40)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGMIS              (*(volatile hw_uartdbgmis_t *) HW_UARTDBGMIS_ADDR)
#define HW_UARTDBGMIS_RD()         (HW_UARTDBGMIS.U)
#endif


//  constants & macros for individual HW_UARTDBGMIS bitfields

//  Register HW_UARTDBGMIS, field UNAVAILABLE
#define BP_UARTDBGMIS_UNAVAILABLE      16
#define BM_UARTDBGMIS_UNAVAILABLE      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGMIS_UNAVAILABLE(v)   ((((reg32_t) v) << 16) & BM_UARTDBGMIS_UNAVAILABLE)
#else
#define BF_UARTDBGMIS_UNAVAILABLE(v)   (((v) << 16) & BM_UARTDBGMIS_UNAVAILABLE)
#endif

//  Register HW_UARTDBGMIS, field RESERVED
#define BP_UARTDBGMIS_RESERVED         11
#define BM_UARTDBGMIS_RESERVED         0x0000F800

#define BF_UARTDBGMIS_RESERVED(v)   (((v) << 11) & BM_UARTDBGMIS_RESERVED)

//  Register HW_UARTDBGMIS, field OEMIS
#define BP_UARTDBGMIS_OEMIS            10
#define BM_UARTDBGMIS_OEMIS            0x00000400

#define BF_UARTDBGMIS_OEMIS(v)         (((v) << 10) & BM_UARTDBGMIS_OEMIS)

//  Register HW_UARTDBGMIS, field BEMIS
#define BP_UARTDBGMIS_BEMIS            9
#define BM_UARTDBGMIS_BEMIS            0x00000200

#define BF_UARTDBGMIS_BEMIS(v)   (((v) << 9) & BM_UARTDBGMIS_BEMIS)

//  Register HW_UARTDBGMIS, field PEMIS
#define BP_UARTDBGMIS_PEMIS            8
#define BM_UARTDBGMIS_PEMIS            0x00000100

#define BF_UARTDBGMIS_PEMIS(v)         (((v) << 8) & BM_UARTDBGMIS_PEMIS)

//  Register HW_UARTDBGMIS, field FEMIS
#define BP_UARTDBGMIS_FEMIS            7
#define BM_UARTDBGMIS_FEMIS            0x00000080

#define BF_UARTDBGMIS_FEMIS(v)         (((v) << 7) & BM_UARTDBGMIS_FEMIS)

//  Register HW_UARTDBGMIS, field RTMIS
#define BP_UARTDBGMIS_RTMIS            6
#define BM_UARTDBGMIS_RTMIS            0x00000040

#define BF_UARTDBGMIS_RTMIS(v)         (((v) << 6) & BM_UARTDBGMIS_RTMIS)

//  Register HW_UARTDBGMIS, field TXMIS
#define BP_UARTDBGMIS_TXMIS            5
#define BM_UARTDBGMIS_TXMIS            0x00000020

#define BF_UARTDBGMIS_TXMIS(v)         (((v) << 5) & BM_UARTDBGMIS_TXMIS)

//  Register HW_UARTDBGMIS, field RXMIS
#define BP_UARTDBGMIS_RXMIS            4
#define BM_UARTDBGMIS_RXMIS            0x00000010

#define BF_UARTDBGMIS_RXMIS(v)         (((v) << 4) & BM_UARTDBGMIS_RXMIS)

//  Register HW_UARTDBGMIS, field DSRMMIS
#define BP_UARTDBGMIS_DSRMMIS          3
#define BM_UARTDBGMIS_DSRMMIS          0x00000008

#define BF_UARTDBGMIS_DSRMMIS(v)       (((v) << 3) & BM_UARTDBGMIS_DSRMMIS)

//  Register HW_UARTDBGMIS, field DCDMMIS
#define BP_UARTDBGMIS_DCDMMIS          2
#define BM_UARTDBGMIS_DCDMMIS          0x00000004

#define BF_UARTDBGMIS_DCDMMIS(v)       (((v) << 2) & BM_UARTDBGMIS_DCDMMIS)

//  Register HW_UARTDBGMIS, field CTSMMIS
#define BP_UARTDBGMIS_CTSMMIS          1
#define BM_UARTDBGMIS_CTSMMIS          0x00000002

#define BF_UARTDBGMIS_CTSMMIS(v)       (((v) << 1) & BM_UARTDBGMIS_CTSMMIS)

//  Register HW_UARTDBGMIS, field RIMMIS
#define BP_UARTDBGMIS_RIMMIS           0
#define BM_UARTDBGMIS_RIMMIS           0x00000001

#define BF_UARTDBGMIS_RIMMIS(v)        (((v) << 0) & BM_UARTDBGMIS_RIMMIS)


//------------------------------------------------------------------------------
//  HW_UARTDBGICR - UART Interrupt Clear Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned RIMIC        :  1;
        unsigned CTSMIC       :  1;
        unsigned DCDMIC       :  1;
        unsigned DSRMIC       :  1;
        unsigned RXIC         :  1;
        unsigned TXIC         :  1;
        unsigned RTIC         :  1;
        unsigned FEIC         :  1;
        unsigned PEIC         :  1;
        unsigned BEIC         :  1;
        unsigned OEIC         :  1;
        unsigned RESERVED     :  5;
        unsigned UNAVAILABLE  : 16;
    } B;
} hw_uartdbgicr_t;
#endif


//constants & macros for entire HW_UARTDBGICR register

#define HW_UARTDBGICR_ADDR         (REGS_UARTDBG_BASE + 0x44)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGICR              (*(volatile hw_uartdbgicr_t *) HW_UARTDBGICR_ADDR)
#define HW_UARTDBGICR_RD()         (HW_UARTDBGICR.U)
#define HW_UARTDBGICR_WR(v)        (HW_UARTDBGICR.U = (v))
#define HW_UARTDBGICR_SET(v)       (HW_UARTDBGICR_WR(HW_UARTDBGICR_RD() |  (v)))
#define HW_UARTDBGICR_CLR(v)       (HW_UARTDBGICR_WR(HW_UARTDBGICR_RD() & ~(v)))
#define HW_UARTDBGICR_TOG(v)       (HW_UARTDBGICR_WR(HW_UARTDBGICR_RD() ^  (v)))
#endif


//  constants & macros for individual HW_UARTDBGICR bitfields

//  Register HW_UARTDBGICR, field UNAVAILABLE
#define BP_UARTDBGICR_UNAVAILABLE      16
#define BM_UARTDBGICR_UNAVAILABLE      0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGICR_UNAVAILABLE(v)   ((((reg32_t) v) << 16) & BM_UARTDBGICR_UNAVAILABLE)
#else
#define BF_UARTDBGICR_UNAVAILABLE(v)   (((v) << 16) & BM_UARTDBGICR_UNAVAILABLE)
#endif
//  Register HW_UARTDBGICR, field RESERVED
#define BP_UARTDBGICR_RESERVED         11
#define BM_UARTDBGICR_RESERVED         0x0000F800

#define BF_UARTDBGICR_RESERVED(v)      (((v) << 11) & BM_UARTDBGICR_RESERVED)

//  Register HW_UARTDBGICR, field OEIC
#define BP_UARTDBGICR_OEIC             10
#define BM_UARTDBGICR_OEIC             0x00000400

#define BF_UARTDBGICR_OEIC(v)          (((v) << 10) & BM_UARTDBGICR_OEIC)

//  Register HW_UARTDBGICR, field BEIC
#define BP_UARTDBGICR_BEIC             9
#define BM_UARTDBGICR_BEIC             0x00000200

#define BF_UARTDBGICR_BEIC(v)          (((v) << 9) & BM_UARTDBGICR_BEIC)

//  Register HW_UARTDBGICR, field PEIC
#define BP_UARTDBGICR_PEIC             8
#define BM_UARTDBGICR_PEIC             0x00000100

#define BF_UARTDBGICR_PEIC(v)          (((v) << 8) & BM_UARTDBGICR_PEIC)

//  Register HW_UARTDBGICR, field FEIC
#define BP_UARTDBGICR_FEIC             7
#define BM_UARTDBGICR_FEIC             0x00000080

#define BF_UARTDBGICR_FEIC(v)          (((v) << 7) & BM_UARTDBGICR_FEIC)

//  Register HW_UARTDBGICR, field RTIC
#define BP_UARTDBGICR_RTIC             6
#define BM_UARTDBGICR_RTIC             0x00000040

#define BF_UARTDBGICR_RTIC(v)          (((v) << 6) & BM_UARTDBGICR_RTIC)

//  Register HW_UARTDBGICR, field TXIC
#define BP_UARTDBGICR_TXIC             5
#define BM_UARTDBGICR_TXIC             0x00000020

#define BF_UARTDBGICR_TXIC(v)          (((v) << 5) & BM_UARTDBGICR_TXIC)

//  Register HW_UARTDBGICR, field RXIC
#define BP_UARTDBGICR_RXIC             4
#define BM_UARTDBGICR_RXIC             0x00000010

#define BF_UARTDBGICR_RXIC(v)          (((v) << 4) & BM_UARTDBGICR_RXIC)

//  Register HW_UARTDBGICR, field DSRMIC
#define BP_UARTDBGICR_DSRMIC           3
#define BM_UARTDBGICR_DSRMIC           0x00000008

#define BF_UARTDBGICR_DSRMIC(v)        (((v) << 3) & BM_UARTDBGICR_DSRMIC)

//  Register HW_UARTDBGICR, field DCDMIC
#define BP_UARTDBGICR_DCDMIC           2
#define BM_UARTDBGICR_DCDMIC           0x00000004

#define BF_UARTDBGICR_DCDMIC(v)        (((v) << 2) & BM_UARTDBGICR_DCDMIC)

//  Register HW_UARTDBGICR, field CTSMIC
#define BP_UARTDBGICR_CTSMIC           1
#define BM_UARTDBGICR_CTSMIC           0x00000002

#define BF_UARTDBGICR_CTSMIC(v)        (((v) << 1) & BM_UARTDBGICR_CTSMIC)

//  Register HW_UARTDBGICR, field RIMIC
#define BP_UARTDBGICR_RIMIC            0
#define BM_UARTDBGICR_RIMIC            0x00000001

#define BF_UARTDBGICR_RIMIC(v)         (((v) << 0) & BM_UARTDBGICR_RIMIC)

// error status in DATA FIFO
#define FIFO_FRAME_ERROR               0x100
#define FIFO_PARITY_ERROR              0x200
#define FIFO_BREAK_ERROR               0x400
#define FIFO_OVERRUN_ERROR             0x800
#define FIFO_ERROR                     0xF00

#define UARTDBG_ALL_RX_ERROR_MASK     \
    ( BM_UARTDBGRSR_ECR_OE          | \
      BM_UARTDBGRSR_ECR_BE          | \
      BM_UARTDBGRSR_ECR_PE          | \
      BM_UARTDBGRSR_ECR_FE)

#define UARTDBG_ALL_INT_CLEAR         \
    ( BM_UARTDBGICR_OEIC            | \
      BM_UARTDBGICR_BEIC            | \
      BM_UARTDBGICR_PEIC            | \
      BM_UARTDBGICR_FEIC            | \
      BM_UARTDBGICR_RTIC            | \
      BM_UARTDBGICR_TXIC            | \
      BM_UARTDBGICR_RXIC            | \
      BM_UARTDBGICR_DSRMIC          | \
      BM_UARTDBGICR_DCDMIC          | \
      BM_UARTDBGICR_CTSMIC          | \
      BM_UARTDBGICR_RIMIC )

#define UARTDBG_ALL_INT_STATUS_MASK   \
    ( BM_UARTDBGIMSC_OEIM           | \
      BM_UARTDBGIMSC_BEIM           | \
      BM_UARTDBGIMSC_PEIM           | \
      BM_UARTDBGIMSC_FEIM           | \
      BM_UARTDBGIMSC_RTIM           | \
      BM_UARTDBGIMSC_TXIM           | \
      BM_UARTDBGIMSC_RXIM           | \
      BM_UARTDBGIMSC_DSRMIM         | \
      BM_UARTDBGIMSC_DCDMIM         | \
      BM_UARTDBGIMSC_CTSMIM         | \
      BM_UARTDBGIMSC_RIMIM)


//------------------------------------------------------------------------------
//  HW_UARTDBGDMACR - UART DMA Control Register
//------------------------------------------------------------------------------

#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned RXDMAE       :  1;
        unsigned TXDMAE       :  1;
        unsigned DMAONERR     :  1;
        unsigned RESERVED     : 13;
        unsigned UNAVAILABLE  : 16;
    } B;
} hw_uartdbgdmacr_t;
#endif


//  constants & macros for entire HW_UARTDBGDMACR register

#define HW_UARTDBGDMACR_ADDR         (REGS_UARTDBG_BASE + 0x48)

#ifndef __LANGUAGE_ASM__
#define HW_UARTDBGDMACR              (*(volatile hw_uartdbgdmacr_t *) HW_UARTDBGDMACR_ADDR)
#define HW_UARTDBGDMACR_RD()         (HW_UARTDBGDMACR.U)
#define HW_UARTDBGDMACR_WR(v)        (HW_UARTDBGDMACR.U = (v))
#define HW_UARTDBGDMACR_SET(v)       (HW_UARTDBGDMACR_WR(HW_UARTDBGDMACR_RD() |  (v)))
#define HW_UARTDBGDMACR_CLR(v)       (HW_UARTDBGDMACR_WR(HW_UARTDBGDMACR_RD() & ~(v)))
#define HW_UARTDBGDMACR_TOG(v)       (HW_UARTDBGDMACR_WR(HW_UARTDBGDMACR_RD() ^  (v)))
#endif


//  constants & macros for individual HW_UARTDBGDMACR bitfields

//  Register HW_UARTDBGDMACR, field UNAVAILABLE
#define BP_UARTDBGDMACR_UNAVAILABLE    16
#define BM_UARTDBGDMACR_UNAVAILABLE    0xFFFF0000

#ifndef __LANGUAGE_ASM__
#define BF_UARTDBGDMACR_UNAVAILABLE(v) ((((reg32_t) v) << 16) & BM_UARTDBGDMACR_UNAVAILABLE)
#else
#define BF_UARTDBGDMACR_UNAVAILABLE(v) (((v) << 16) & BM_UARTDBGDMACR_UNAVAILABLE)
#endif

//  Register HW_UARTDBGDMACR, field RESERVED
#define BP_UARTDBGDMACR_RESERVED       3
#define BM_UARTDBGDMACR_RESERVED       0x0000FFF8

#define BF_UARTDBGDMACR_RESERVED(v)    (((v) << 3) & BM_UARTDBGDMACR_RESERVED)

//  Register HW_UARTDBGDMACR, field DMAONERR
#define BP_UARTDBGDMACR_DMAONERR       2
#define BM_UARTDBGDMACR_DMAONERR       0x00000004

#define BF_UARTDBGDMACR_DMAONERR(v)    (((v) << 2) & BM_UARTDBGDMACR_DMAONERR)

//  Register HW_UARTDBGDMACR, field TXDMAE
#define BP_UARTDBGDMACR_TXDMAE         1
#define BM_UARTDBGDMACR_TXDMAE         0x00000002

#define BF_UARTDBGDMACR_TXDMAE(v)      (((v) << 1) & BM_UARTDBGDMACR_TXDMAE)

//  Register HW_UARTDBGDMACR, field RXDMAE
#define BP_UARTDBGDMACR_RXDMAE         0
#define BM_UARTDBGDMACR_RXDMAE         0x00000001

#define BF_UARTDBGDMACR_RXDMAE(v)      (((v) << 0) & BM_UARTDBGDMACR_RXDMAE)


#endif // COMMON_UARTDBG_H
