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
//  File:  omap2420_wdog.h
//
//  This file contains offset addresses for WatchDog registers.

#ifndef __OMAP2420_WDOG_H
#define __OMAP2420_WDOG_H

//------------------------------------------------------------------------------

// Watchdog Timer 3 Register Summary

typedef volatile struct {
    unsigned long ulWIDR;			// offset 0x0000, WIDR
    unsigned long ulRESERVED_1[3];
    unsigned long ulWD_SYSCONFIG;   // offset 0x0010, WD_SYSCONFIG
    unsigned long ulWD_SYSSTATUS;   // offset 0x0014, WD_SYSSTATUS
    unsigned long ulWISR;           // offset 0x0018, WISR
    unsigned long ulWIER;           // offset 0x001C, WIER
    unsigned long ulRESERVED_0x20;
    unsigned long ulWCLR;           // offset 0x0024, WCLR
    unsigned long ulWCRR;           // offset 0x0028, WCRR
    unsigned long ulWLDR;           // offset 0x002C, WLDR
    unsigned long ulWTGR;           // offset 0x0030, WTGR
    unsigned long ulWWPS;           // offset 0x0034, WLDR   
    unsigned long ulRESERVED_2[4];
    unsigned long ulWSPR;           // offset 0x0038, WSPR   
} OMAP2420_WDOG_REGS;

#define OMAP2420_WDOG_DISABLE_SEQ1  0x0000AAAA
#define OMAP2420_WDOG_DISABLE_SEQ2  0x00005555

#define OMAP2420_WDOG_ENABLE_SEQ1   0x0000BBBB
#define OMAP2420_WDOG_ENABLE_SEQ2   0x00004444

//------------------------------------------------------------------------------

#endif
