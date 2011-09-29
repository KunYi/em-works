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
//  File:  vrc5477_base_regs.h
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
#ifndef __VRC5477_BASE_REGS_H
#define __VRC5477_BASE_REGS_H

//------------------------------------------------------------------------------
// VRC5477 Base Address
//------------------------------------------------------------------------------

#define VRC5477_REG_PA          0x1FA00000

//------------------------------------------------------------------------------

#endif
