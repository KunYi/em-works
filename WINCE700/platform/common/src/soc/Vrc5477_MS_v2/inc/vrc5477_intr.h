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
#ifndef __VRC5477_INTR_H
#define __VRC5477_INTR_H

//------------------------------------------------------------------------------
//
//  Defines: IRQ_xxx
//
//  This constants define physical interrupt request numbers as provided
//  by hardware.
//
#define IRQ_CPCE                0
#define IRQ_CNTD                1
#define IRQ_I2C                 2
#define IRQ_DMA                 3
#define IRQ_UART0               4
#define IRQ_WDOG                5
#define IRQ_SPT1                6
#define IRQ_LBRTD               7
#define IRQ_INTA                8
#define IRQ_INTB                9
#define IRQ_INTC                10
#define IRQ_INTD                11
#define IRQ_INTE                12
#define IRQ_PCISERR             14
#define IRQ_PCIIERR             15
#define IRQ_IOINTA              16
#define IRQ_IOINTB              17
#define IRQ_IOINTC              18
#define IRQ_IOINTD              19
#define IRQ_UART1               20
#define IRQ_SPT0                21
#define IRQ_GPT0                22
#define IRQ_GPT1                23
#define IRQ_GPT2                24
#define IRQ_GPT3                25
#define IRQ_GPIO                26
#define IRQ_SIO0                27
#define IRQ_SIO1                28
#define IRQ_IOPCISERR           30
#define IRQ_IOPCIIERR           31

//------------------------------------------------------------------------------
//
//  Defines: VRC5477_INTRxxx
//
//  This constants are used to initialize interrupt controller. The platform
//  code can overwrite this settings if appropriate. Constants bellow
//  route all interrupts to MIPS interrupt 0 and disable them.
//
#define VRC5477_INTCTRL0        0x00000000
#define VRC5477_INTCTRL1        0x00000000
#define VRC5477_INTCTRL2        0x00000000
#define VRC5477_INTCTRL3        0x00000000

//------------------------------------------------------------------------------
//
//  Defines: VRC5477_INTPPESx
//
//  This constants are used to initialize PCI bus interrupt controller
//  registers. The platform code can overwrite this settings if appropriate.
//  Constant bellow sets level low interrupt for all both PCI buses.
//
#define VRC5477_INTPPES0        0x000003FF
#define VRC5477_INTPPES1        0x000000FF

//------------------------------------------------------------------------------

#endif
