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
//-----------------------------------------------------------------------------
//
//  Header:  socarm_macros.h
//
//  Provides useful macro definitions SOC/BSP code development.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//-----------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// Copyright (C) 2007, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#ifndef __SOCARM_MACROS_H__
#define __SOCARM_MACROS_H__

// Bitfield macros that use rely on bitfield width/shift information
// defined in SOC header files
#define CSP_BITFMASK(bit) (((1U << (bit ## _WID)) - 1) << (bit ## _LSH))
#define CSP_BITFVAL(bit, val) ((val) << (bit ## _LSH))

// Undefine previous implementations of peripheral access macros since
// we want to "own" the definitions and avoid redefinition warnings
// resulting from source code that includes oal_io.h
#undef INREG8
#undef OUTREG8
#undef SETREG8
#undef CLRREG8
#undef INREG16
#undef OUTREG16
#undef SETREG16
#undef CLRREG16
#undef INREG32
#undef OUTREG32
#undef SETREG32
#undef CLRREG32

// Macros for accessing peripheral registers using DDK macros/functions
#define INREG8(x)           READ_REGISTER_UCHAR((UCHAR*)(x))
#define OUTREG8(x, y)       WRITE_REGISTER_UCHAR((UCHAR*)(x), (UCHAR)(y))
#define SETREG8(x, y)       OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)       OUTREG8(x, INREG8(x)&~(y))
#define INSREG8(addr, mask, val) OUTREG8(addr, ((INREG8(addr)&(~(mask))) | val))
#define EXTREG8(addr, mask, lsh) ((INREG8(addr) & mask) >> lsh)
#define EXTREG8BF(addr, bit) (EXTREG8(addr, CSP_BITFMASK(bit), (bit ## _LSH)))
#define INSREG8BF(addr, bit, val) (INSREG8(addr, CSP_BITFMASK(bit), CSP_BITFVAL(bit, val)))

#define INREG16(x)          READ_REGISTER_USHORT((USHORT*)(x))
#define OUTREG16(x, y)      WRITE_REGISTER_USHORT((USHORT*)(x),(USHORT)(y))
#define SETREG16(x, y)      OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)      OUTREG16(x, INREG16(x)&~(y))
#define INSREG16(addr, mask, val) OUTREG16(addr, ((INREG16(addr)&(~(mask))) | val))
#define EXTREG16(addr, mask, lsh) ((INREG16(addr) & mask) >> lsh)
#define EXTREG16BF(addr, bit) (EXTREG16(addr, CSP_BITFMASK(bit), (bit ## _LSH)))
#define INSREG16BF(addr, bit, val) (INSREG16(addr, CSP_BITFMASK(bit), CSP_BITFVAL(bit, val)))

#define INREG32(x)          READ_REGISTER_ULONG((ULONG*)(x))
#define OUTREG32(x, y)      WRITE_REGISTER_ULONG((ULONG*)(x), (ULONG)(y))
#define SETREG32(x, y)      OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)      OUTREG32(x, INREG32(x)&~(y))
#define INSREG32(addr, mask, val) OUTREG32(addr, ((INREG32(addr)&(~(mask))) | val))
#define EXTREG32(addr, mask, lsh) ((INREG32(addr) & mask) >> lsh)
#define EXTREG32BF(addr, bit) (EXTREG32(addr, CSP_BITFMASK(bit), (bit ## _LSH)))
#define INSREG32BF(addr, bit, val) (INSREG32(addr, CSP_BITFMASK(bit), CSP_BITFVAL(bit, val)))

// Macros for bitfield operations on data variables.  DO NOT use for peripheral
// register accesses.  Use the INREG/OUTREG based macros above for peripheral
// accesses.
#define CSP_BITFEXT(var, bit) ((var & CSP_BITFMASK(bit)) >> (bit ## _LSH))

#define CSP_BITFCLR(var, bit) (var &= (~CSP_BITFMASK(bit)))

#define CSP_BITFINS(var, bit, val) \
    (CSP_BITFCLR(var, bit)); (var |= CSP_BITFVAL(bit, val))

// Macros for generating 64-bit IRQ masks
#define CSP_IRQMASK(irq) (((ULONGLONG) 1)  << irq)

// Macros to create Unicode function name
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFUNCTION__ WIDEN(__FUNCTION__)

// Macros for function tracing (better to use debug zones as these 
// macros will be phases out)
#define CSP_FUNC_TRACE_ENTRY() \
    DEBUGMSG(1, (TEXT("+%s\r\n"), __WFUNCTION__))

#define CSP_FUNC_TRACE_EXIT() \
    DEBUGMSG(1, (TEXT("-%s\r\n"), __WFUNCTION__))

// Macros for importing/exporting DLL interface
#define DllExport __declspec( dllexport )


#endif // __SOCARM_MACROS_H__
