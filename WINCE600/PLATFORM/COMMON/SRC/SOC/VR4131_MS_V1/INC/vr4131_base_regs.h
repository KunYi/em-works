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
//  File:  vr4131_base_regs.h
//
//  This header file defines the addresses of the base registers for
//  the System on Chip (SoC) components.
//
//  File is used for C/C++ and assembler code.
//
//  The following abbreviations are used for different addressing type:
//                PA - physical address
//                UA - uncached virtual address
//                CA - cached virtual address
//                OA - offset address
//
//              The naming convention for CPU base registers is:
//
//                <SoC>_REG_<ADDRTYPE>_<SUBSYSTEM>
//
#ifndef __VR4130_BASE_REGS_H
#define __VR4130_BASE_REGS_H

//------------------------------------------------------------------------------
// Bus Control Unit
//------------------------------------------------------------------------------

#define VR4131_REG_PA_BCU           0x0F000000

//------------------------------------------------------------------------------
// Clock Mask Unit
//------------------------------------------------------------------------------

#define VR4131_REG_PA_CMU           0x0F000060

//------------------------------------------------------------------------------
// Interrupt Control Unit
//------------------------------------------------------------------------------

#define VR4131_REG_PA_ICU           0x0F000080

//------------------------------------------------------------------------------
// Power Management Unit
//------------------------------------------------------------------------------

#define VR4131_REG_PA_PMU           0x0F0000C0

//------------------------------------------------------------------------------
// Realtime Clock Unit
//------------------------------------------------------------------------------

#define VR4131_REG_PA_RTCU          0x0F000100

//------------------------------------------------------------------------------
// General-Purpose I/O Unit
//------------------------------------------------------------------------------

#define VR4131_REG_PA_GIU           0x0F000140

//------------------------------------------------------------------------------
// SDRAM Control Unit
//------------------------------------------------------------------------------

#define VR4131_REG_PA_SDRAMU        0x0F000400

//------------------------------------------------------------------------------
// Serial Interface Unit
//------------------------------------------------------------------------------

#define VR4131_REG_PA_SIU           0x0F000800

//------------------------------------------------------------------------------
// Debug Serial Interface Unit
//------------------------------------------------------------------------------

#define VR4131_REG_PA_DSIU          0x0F000820

//------------------------------------------------------------------------------
// PCI Control Unit
//------------------------------------------------------------------------------

#define VR4131_REG_PA_PCIU          0x0F000C00

//------------------------------------------------------------------------------

#endif
