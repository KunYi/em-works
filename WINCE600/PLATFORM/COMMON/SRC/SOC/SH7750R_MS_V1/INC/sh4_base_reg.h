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
//  File:  sh4_base_regs.h
//
//  This header file defines the addresses of the base registers for 
//  the System on Chip (SoC) components.
//
//  Note:  File is used for C/C++ and assembler code.
//
//  Naming:  The following abbreviations are used for different addressing type:
//
//                PA - physical address
//                UA - uncached virtual address
//                CA - cached virtual address
//                OA - offset address
//
//              The naming convention for CPU base registers is:
//
//                <SoC>_REG_<ADDRTYPE>_<SUBSYSTEM>
//
#ifndef __SH4_BASE_REGS_H
#define __SH4_BASE_REGS_H

//------------------------------------------------------------------------------
// Cache and TLB Controller 
//------------------------------------------------------------------------------

#define SH4_REG_PA_CCN                      0xFF000000

//------------------------------------------------------------------------------
// Bus State Controller
//------------------------------------------------------------------------------

#define SH4_REG_PA_BSC                      0xFF800000

//------------------------------------------------------------------------------
// Direct Memory Access Controller
//------------------------------------------------------------------------------

#define SH4_REG_PA_DMAC                     0xFFA00000
#define SH4_REG_PA_DMAC_EX                  0xFFA00050

//------------------------------------------------------------------------------
// Clock Pulse Generator
//------------------------------------------------------------------------------

#define SH4_REG_PA_CPG                      0xFFC00000

//------------------------------------------------------------------------------
// Real Time Clock Control
//------------------------------------------------------------------------------

#define SH4_REG_PA_RTC                      0xFFC80000
#define SH4_REG_PA_RTC_EX                   0xFFC80050

//------------------------------------------------------------------------------
// Interrupt Controller
//------------------------------------------------------------------------------

#define SH4_REG_PA_INTC                     0xFFD00000

//------------------------------------------------------------------------------
// Timer Unit
//------------------------------------------------------------------------------

#define SH4_REG_PA_TMU                      0xFFD80000
#define SH4_REG_PA_TMU_EX                   0xFE100000

//------------------------------------------------------------------------------
// Serial Communication Interface
//------------------------------------------------------------------------------

#define SH4_REG_PA_SCI                      0xFFE00000

//------------------------------------------------------------------------------
// Serial Communication Interface with FIFO
//------------------------------------------------------------------------------

#define SH4_REG_PA_SCIF                     0xFFE80000

//------------------------------------------------------------------------------

#endif

