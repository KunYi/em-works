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
//  File:  bsp_base_regs.h
//
//  This header file defines the addresses of the base registers for
//  the board components.
//
//  The following abbreviations are used for different addressing type:
//
//                PA - physical address
//                UA - uncached virtual address
//                CA - cached virtual address
//                OA - offset address
//
//  The naming convention for CPU base registers is:
//
//                BSP_<SUBSYSTEM>_REGS_<ADDRTYPE>
//
#ifndef __BSP_BASE_REGS_H
#define __BSP_BASE_REGS_H

//------------------------------------------------------------------------------
//  CPLD Chip
//------------------------------------------------------------------------------

#define BSP_CPLD_REGS_PA            0x08000000

//------------------------------------------------------------------------------
//  SMSC91C96 Ethernet Chip
//------------------------------------------------------------------------------

#define BSP_SMSC91C96_REGS_PA       0x08000300

//------------------------------------------------------------------------------
//  NAND flash memory location
//------------------------------------------------------------------------------

#define BSP_NAND_REGS_PA            0x6800A07C  //Use GPMC CMD,ADDR,DATA &
                                                //STATUS register for NAND

//------------------------------------------------------------------------------

#endif
