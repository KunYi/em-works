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
//
// Portions Copyright (c) Texas Instruments.  All rights reserved. 
//
//------------------------------------------------------------------------------
//
//  File:  omap2420_hdq.h
//
//  This header file is comprised of register information for HDQ
//

#ifndef __OMAP2420_HDQ_H
#define __OMAP2420_HDQ_H

//------------------------------------------------------------------------------

// HDQ/1-Wire Base Address : OMAP2420_HDQ_1WIRE_REGS_PA (0x480B2000)

typedef struct __HDQREGS__
{
	UINT32 ulHDQ_REVISION;		//offset 0x00, HDQ revision
	UINT32 ulHDQ_TX_DATA;		//offset 0x04, TX data
	UINT32 ulHDQ_RX_DATA;		//offset 0x08, RX data
	UINT32 ulHDQ_CTRL_STATUS;	//offset 0x0C, CTRL status
	UINT32 ulHDQ_INT_STATUS;	//offset 0x10, interrupt status
	UINT32 ulHDQ_SYSCONFIG;		//offset 0x14, system config
	UINT32 ulHDQ_SYSSTATUS;		//offset 0x18, system status
}
OMAP2420_HDQ_REGS, *POMAP2420_HDQ_REGS;

//------------------------------------------------------------------------------

// SYSCONFIG register fields

#define HDQ_SYSCONFIG_AUTOIDLE				(1 << 0)
#define HDQ_SYSCONFIG_SOFTRESET				(1 << 1)

// SYSSTATUS register fields

#define HDQ_SYSSTATUS_RESETDONE				(1 << 0)

// CTRL_STATUS register fields

#define HDQ_CTRL_STATUS_MODE				(1 << 0)
#define HDQ_CTRL_STATUS_DIR					(1 << 1)
#define HDQ_CTRL_STATUS_INITIALIZATION		(1 << 2)
#define HDQ_CTRL_STATUS_PRESENCEDETECT		(1 << 3)
#define HDQ_CTRL_STATUS_GO					(1 << 4)
#define HDQ_CTRL_STATUS_CLOCKENABLE			(1 << 5)
#define HDQ_CTRL_STATUS_INTERRUPTMASK		(1 << 6)
#define HDQ_CTRL_STATUS_1_WIRE_SINGLE_BIT	(1 << 7)

// INT_STATUS register fields

#define HDQ_INT_STATUS_TIMEOUT				(1 << 0)
#define HDQ_INT_STATUS_RXCOMPLETE			(1 << 1)
#define HDQ_INT_STATUS_TXCOMPLETE			(1 << 2)

#endif
