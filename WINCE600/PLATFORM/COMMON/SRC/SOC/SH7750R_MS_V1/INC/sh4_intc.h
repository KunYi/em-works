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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  sh4_intc.h
//
//  This header file defines Interrupt Control register layout and associated constants
//  and types.
//
//  Note:  Unit base address is defined in sh4_base_regs.h header file.
//
#ifndef __SH4_INTC_H
#define __SH4_INTC_H

//------------------------------------------------------------------------------

typedef struct {
    UINT16  ICR;
    UINT16  UNUSED2[1];
    UINT16  IPRA;
    UINT16  UNUSED6[1];
    UINT16  IPRB;
    UINT16  UNUSEDA[1];
    UINT16  IPRC;
    UINT16  UNUSEDE[1];
} SH4_INTC_REGS, *PSH4_INTC_REGS;

//------------------------------------------------------------------------------
// Interrupt Control Register
#define INTC_ICR_IRL_MASK           0xFF7F
#define INTC_ICR_IRL_ENCODE         0x0000
#define INTC_ICR_IRL_INDEPENDENT    0x0080

//------------------------------------------------------------------------------
// Interrupt Priority level setting Register A
 
#define INTC_IPRA_TMU0_MASK         0x0FFF
#define INTC_IPRA_TMU1_MASK         0xF0FF
#define INTC_IPRA_TMU2_MASK         0xFF0F
#define INTC_IPRA_RTC_MASK          0xFFF0
#define INTC_IPRA_RTC_INT           0x000F

//------------------------------------------------------------------------------
// Interrupt Priority level setting Register B
 
#define INTC_IPRB_WDT_MASK          0x0FFF
#define INTC_IPRB_REF_MASK          0xF0FF
#define INTC_IPRB_SCI_MASK          0xFF0F

//------------------------------------------------------------------------------
// Interrupt Priority level setting Register C

#define INTC_IPRC_GPIO_MASK         0x0FFF
#define INTC_IPRC_DMAC_MASK         0xF0FF
#define INTC_IPRC_SCIF_MASK         0xFF0F
#define INTC_IPRC_JTAG_MASK         0xFFF0

//------------------------------------------------------------------------------

#define EXCEPTION_CODE_BASE         0x200
#define EXCEPTIONCODETOIRQ(x)       ((UINT32)((x - EXCEPTION_CODE_BASE) / 0x20))

//------------------------------------------------------------------------------
// Hardware Supported Exception Codes
//------------------------------------------------------------------------------
#define IRQ_0                       EXCEPTIONCODETOIRQ(0x200)
#define IRQ_1                       EXCEPTIONCODETOIRQ(0x220)
#define IRQ_2                       EXCEPTIONCODETOIRQ(0x240)
#define IRQ_3                       EXCEPTIONCODETOIRQ(0x260)
#define IRQ_4                       EXCEPTIONCODETOIRQ(0x280)
#define IRQ_5                       EXCEPTIONCODETOIRQ(0x2A0)
#define IRQ_6                       EXCEPTIONCODETOIRQ(0x2C0)
#define IRQ_7                       EXCEPTIONCODETOIRQ(0x2E0)
#define IRQ_8                       EXCEPTIONCODETOIRQ(0x300)
#define IRQ_9                       EXCEPTIONCODETOIRQ(0x320)
#define IRQ_10                      EXCEPTIONCODETOIRQ(0x340)
#define IRQ_11                      EXCEPTIONCODETOIRQ(0x360)
#define IRQ_12                      EXCEPTIONCODETOIRQ(0x380)
#define IRQ_13                      EXCEPTIONCODETOIRQ(0x3A0)
#define IRQ_14                      EXCEPTIONCODETOIRQ(0x3C0)
#define IRQ_NOINTR                  EXCEPTIONCODETOIRQ(0x3E0)

#define IRQTOIPL(x)                 (IRQ_NOINTR - x)
#define IPLTOIRQ(x)                 (IRQ_NOINTR - x)

#define IRQ_TMU0_TUNI0              EXCEPTIONCODETOIRQ(0x400)
#define IRQ_TMU1_TUNI1              EXCEPTIONCODETOIRQ(0x420)
#define IRQ_TMU2_TUNI2              EXCEPTIONCODETOIRQ(0x440)
#define IRQ_TMU2_TICPI2             EXCEPTIONCODETOIRQ(0x460)
#define IRQ_TMU3_TUNI3              EXCEPTIONCODETOIRQ(0xB00)
#define IRQ_TMU4_TUNI4              EXCEPTIONCODETOIRQ(0xB20)

#define IRQ_RTC_ATI                 EXCEPTIONCODETOIRQ(0x480)
#define IRQ_RTC_PRI                 EXCEPTIONCODETOIRQ(0x4A0)
#define IRQ_RTC_CUI                 EXCEPTIONCODETOIRQ(0x4C0)

#define IRQ_SCI_ERI                 EXCEPTIONCODETOIRQ(0x4E0)
#define IRQ_SCI_RXI                 EXCEPTIONCODETOIRQ(0x500)
#define IRQ_SCI_TXI                 EXCEPTIONCODETOIRQ(0x520)
#define IRQ_SCI_TEI                 EXCEPTIONCODETOIRQ(0x540)

#define IRQ_WDT_ITI                 EXCEPTIONCODETOIRQ(0x560)

#define IRQ_REF_RCMI                EXCEPTIONCODETOIRQ(0x580)
#define IRQ_REF_ROVI                EXCEPTIONCODETOIRQ(0x5A0)

#define IRQ_H_UDI                   EXCEPTIONCODETOIRQ(0x600)

#define IRQ_GPIO_GPIOI              EXCEPTIONCODETOIRQ(0x620)

#define IRQ_DMAC_DMTE0              EXCEPTIONCODETOIRQ(0x640)
#define IRQ_DMAC_DMTE1              EXCEPTIONCODETOIRQ(0x660)
#define IRQ_DMAC_DMTE2              EXCEPTIONCODETOIRQ(0x680)
#define IRQ_DMAC_DMTE3              EXCEPTIONCODETOIRQ(0x6A0)
#define IRQ_DMAC_DMTE4              EXCEPTIONCODETOIRQ(0x780)
#define IRQ_DMAC_DMTE5              EXCEPTIONCODETOIRQ(0x7A0)
#define IRQ_DMAC_DMTE6              EXCEPTIONCODETOIRQ(0x7C0)
#define IRQ_DMAC_DMTE7              EXCEPTIONCODETOIRQ(0x7E0)
#define IRQ_DMAC_DMAE               EXCEPTIONCODETOIRQ(0x6C0)

#define IRQ_SCIF_ERI                EXCEPTIONCODETOIRQ(0x700)
#define IRQ_SCIF_RXI                EXCEPTIONCODETOIRQ(0x720)
#define IRQ_SCIF_BRI                EXCEPTIONCODETOIRQ(0x740)
#define IRQ_SCIF_TXI                EXCEPTIONCODETOIRQ(0x760)

//------------------------------------------------------------------------------

#endif
