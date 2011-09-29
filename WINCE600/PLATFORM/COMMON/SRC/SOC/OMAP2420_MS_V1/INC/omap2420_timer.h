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
//  File:  omap2420.h
//
//  This header file is comprised of component header files that define 
//  the register layout of each System on Chip (SoC) component.

#ifndef __OMAP2420_GPTIMER_H
#define __OMAP2420_GPTIMER_H

//------------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////
// TIMERS 
//////////////////////////////////////////////////////////////////////

//
// General Purpose Timers.
//

typedef volatile struct{

    UINT32 TIDR;                 // offset 0x00 Identification register
    UINT32 ulRESERVED_0x4;
    UINT32 ulRESERVED_0x8;
    UINT32 ulRESERVED_0xC;
    UINT32 TIOCP_CFG;   // offset 0x10 L4 I/F config reg
    UINT32 TISTAT;      // offset 0x14 Timer sys status reg
    UINT32 TISR;        // offset 0x18 Timer interrupt stat
    UINT32 TIER;        // offset 0x1C Timer interrupt enable
    UINT32 TWER;        // offset 0x20 Timer wake-up enable
    UINT32 TCLR;        // offset 0x24 Timer control
    UINT32 TCRR;        // offset 0x28 Timer counter register
    UINT32 TLDR;        // offset 0x2C Timer load register
    UINT32 TTGR;        // offset 0x30 Timer trigger register
    UINT32 TWPS;        // offset 0x34 Timer write posted stat
    UINT32 TMAR;        // offset 0x38 Timer match value reg
    UINT32 TCAR1;       // offset 0x3C Timer counter1 capt reg
    UINT32 TSICR;       // offset 0x40 L4 I/F sync ctrl reg
    UINT32 TCAR2;       // offset 0x44 Timer counter2 capt reg

} OMAP2420_GPTIMER_REGS;

#define TIMER_PERIOD                      1
#define OMAP2420_GPTIMER1_COUNTS_PER_1MS  (32768/1000)

//
// 32 KHz Timer 
//

typedef struct __TIMER32K_REGS__
{
   unsigned long ul32KSYNCNT_REV;     //offset 0x0, Sync counter ident reg
   unsigned long ulRESERVED_0x4;
   unsigned long ulRESERVED_0x8;
   unsigned long ulRESERVED_0xC;
   volatile unsigned long CR;        //offset 0x10, Read counter 
}
OMAP2420_TIMER32K_REGS, *pTIMER32_REGS;

//
// Frame Adjustment Counter Register (FAC)
//

// Base address is OMAP2420_FAC_REGS_PA (0x48092000)

typedef struct __FACREGS__
{
   volatile unsigned short usFARC;     //offset 0x0, FAC reference count
   unsigned short usRESERVED_0x2;
   volatile unsigned short usFSC;      //offset 0x4, Frame start counter
   unsigned short usRESERVED_0x6;   
   volatile unsigned short usCTRL;     //offset 0x8, FAC control
   unsigned short usRESERVED_0xA;
   volatile unsigned short usSTATUS;   //offset 0xC, status
   unsigned short usRESERVED_0xE;
   volatile unsigned short usSYNC_CNT; //offset 0x10, Frame sync count
   unsigned short usRESERVED_0x12;
   volatile unsigned short usSTART_CNT; //offset 0x14, Frame start count
   unsigned short usRESERVED_0x16;
   volatile unsigned short usSYSCONFIG; //offset 0x18, FAC configuration
   unsigned short usRESERVED_0x1A;
   volatile unsigned short usREVISION;  //offset 0x1C, FAC revision
   unsigned short usRESERVED_0x1E;
   volatile unsigned short usSYSSTATUS; //offset 0x20, FAC Module status
}
OMAP2420_FAC_REGS, *pFACREGS;


#endif
