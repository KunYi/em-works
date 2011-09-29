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
#ifndef __MACROS_H_
#define __MACROS_H_

typedef volatile BYTE   *PVBYTE;
typedef volatile SHORT  *PVSHORT;
typedef volatile USHORT *PVUSHORT;
typedef volatile ULONG  *PVULONG;
typedef volatile DWORD  *PVDWORD;

#ifdef REG_ACCESS_WAIT

// convenience macros used in every driver 
#ifndef REG_WAIT
__inline static int RegWait() 
{
	int i;
	for(i=0;i<20;i++);
	return i;
}
#define REG_WAIT	RegWait()
#endif
#define READ_REGISTER_ULONG(reg)		(REG_WAIT,(*(volatile unsigned long * const)(reg)))
#define WRITE_REGISTER_ULONG(reg, val)	(REG_WAIT,(*(volatile unsigned long * const)(reg)) = (val))
#define READ_REGISTER_USHORT(reg)		(REG_WAIT,(*(volatile unsigned short * const)(reg)))
#define WRITE_REGISTER_USHORT(reg, val)	(REG_WAIT,(*(volatile unsigned short * const)(reg)) = (val))
#define READ_REGISTER_UCHAR(reg)		(REG_WAIT,(*(volatile unsigned char * const)(reg)))
#define WRITE_REGISTER_UCHAR(reg, val)	(REG_WAIT,(*(volatile unsigned char * const)(reg)) = (val))

#define MASK_REGISTER_ULONG(reg, val, mask) \
			WRITE_REGISTER_ULONG((reg),(READ_REGISTER_ULONG(reg) & (mask)) | (val) )
#define MASK_REGISTER_USHORT(reg, val, mask) \
			WRITE_REGISTER_USHORT((reg),(READ_REGISTER_USHORT(reg) & (mask)) | (val) )
#define MASK_REGISTER_UCHAR(reg, val, mask) \
			WRITE_REGISTER_UCHAR((reg),(READ_REGISTER_UCHAR(reg) & (mask)) | (val) )
#else
#define READ_REGISTER_ULONG(reg)		(*(volatile unsigned long * const)(reg))
#define WRITE_REGISTER_ULONG(reg, val)	(*(volatile unsigned long * const)(reg)) = (val)
#define READ_REGISTER_USHORT(reg)		(*(volatile unsigned short * const)(reg))
#define WRITE_REGISTER_USHORT(reg, val)	(*(volatile unsigned short * const)(reg)) = (val)
#define READ_REGISTER_UCHAR(reg)		(*(volatile unsigned char * const)(reg))
#define WRITE_REGISTER_UCHAR(reg, val)	(*(volatile unsigned char * const)(reg)) = (val)

#define MASK_REGISTER_ULONG(reg, val, mask) \
			WRITE_REGISTER_ULONG((reg),(READ_REGISTER_ULONG(reg) & (mask)) | (val) )
#define MASK_REGISTER_USHORT(reg, val, mask) \
			WRITE_REGISTER_USHORT((reg),(READ_REGISTER_USHORT(reg) & (mask)) | (val) )
#define MASK_REGISTER_UCHAR(reg, val, mask) \
			WRITE_REGISTER_UCHAR((reg),(READ_REGISTER_UCHAR(reg) & (mask)) | (val) )
#endif//REG_ACCESS_NORMAL

//  Be careful to do pointer arithmetic as chars.

#define REG32(base, id)					(*((PVULONG)(((char*)(base))+(id))))
#define REG16(base, id)					(*((PVUSHORT)(((char*)(base))+(id))))
#define REG8(base, id)					(*((PVBYTE)(((char*)(base))+(id))))


#ifdef	__cplusplus
extern "C" {
#endif

/****************************************************************************
* WARNING: Do not make an inline delay loop for BusyWait.
*          The delay loop must be quad byte aligned and the
*          inline assembler cannot do this.
******************************************************************************/

#define Delay( dwMicroSeconds ) \
	BusyWait(AdjustMicroSecondsToLoopCount( dwMicroSeconds ))

DWORD AdjustMicroSecondsToLoopCount( DWORD dwMicroSeconds );

DWORD BusyWait( DWORD dwLoopCount );

#ifdef __cplusplus
}

#endif


#endif
