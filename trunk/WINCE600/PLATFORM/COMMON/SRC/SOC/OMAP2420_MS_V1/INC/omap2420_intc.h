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
//  This header file is comprised of interrupt controller for MPU, DSP, IVA 
//  modules.


#ifndef __OMAP2420_INTC_H
#define __OMAP2420_INTC_H

//------------------------------------------------------------------------------

// Base Address : 0x480FE000

typedef volatile struct 
{
   UINT32 ulINTC_REVISION;             //offset 0x0, IP revision code
   UINT32 ulRESERVED_04;
   UINT32 ulRESERVED_08;
   UINT32 ulRESERVED_0C;
   UINT32 ulINTC_SYSCONFIG;   //offset 0x10,OCP i/f params ctrl
   UINT32 ulINTC_SYSSTATUS;   //offset 0x14,status info of mod
   UINT32 ulRESERVED_18;
   UINT32 ulRESERVED_1C;
   UINT32 ulRESERVED_20;
   UINT32 ulRESERVED_24;
   UINT32 ulRESERVED_28;
   UINT32 ulRESERVED_2C;
   UINT32 ulRESERVED_30;
   UINT32 ulRESERVED_34;
   UINT32 ulRESERVED_38;
   UINT32 ulRESERVED_3C;
   UINT32 ulINTC_SIR_IRQ;     //offset 0x40,supplies active IRQ
                              //intr number
   UINT32 ulINTC_SIR_FIQ;     //offset 0x44,supplies active FIQ
                              //intr number
   UINT32 ulINTC_CONTROL;     //offset 0x48,global mask and new
                              //intr agreement bits
   UINT32 ulINTC_PROTECTION;  //offset 0x4C,ctrls protection of
                              //other registers
   UINT32 ulINTC_IDLE;        //offset 0x50,func clock auto-idle
                              //disable register
   UINT32 ulRESERVED_3[11];
   UINT32 ulINTC_ITR0;        //offset 0x80,raw intr i/p status
                              //before masking
   UINT32 ulINTC_MIR0;        //offset 0x84,interrupt mask register
   UINT32 ulINTC_MIR_CLEAR0;  //offset 0x88,clear intr mask bits
   UINT32 ulINTC_MIR_SET0;    //offset 0x8C,set intr mask bits
   UINT32 ulINTC_ISR_SET0;    //offset 0x90,set SW intr bits
   UINT32 ulINTC_ISR_CLEAR0;  //offset 0x94,clear SW intr bits
   UINT32 ulINTC_PENDING_IRQ0;   //offset 0x98,IRQ stat aft masking
   UINT32 ulINTC_PENDING_FIQ0;   //offset 0x9C,FIQ stat aft masking
   UINT32 ulINTC_ITR1;        //offset 0xA0,raw intr i/p status
                              //before masking
   UINT32 ulINTC_MIR1;        //offset 0xA4,intr mask reg
   UINT32 ulINTC_MIR_CLEAR1;  //offset 0xA8,clr intr mask bits
   UINT32 ulINTC_MIR_SET1;    //offset 0xAC,set intr mask bits
   UINT32 ulINTC_ISR_SET1;    //offset 0xB0,set SW intr bits
   UINT32 ulINTC_ISR_CLEAR1;  //offset 0xB4,clear SW intr bits
   UINT32 ulINTC_PENDING_IRQ1;   //offset 0xB8,IRQ stat aft masking
   UINT32 ulINTC_PENDING_FIQ1;   //offset 0xBC,FIQ stat aft masking
   UINT32 ulINTC_ITR2;        //offset 0xC0,raw intr i/p status
                              //before masking
   UINT32 ulINTC_MIR2;        //offset 0xC4,intr mask register
   UINT32 ulINTC_MIR_CLEAR2;  //offset 0xC8,clr intr mask bits
   UINT32 ulINTC_MIR_SET2;    //offset 0xCC,set intr mask bits
   UINT32 ulINTC_ISR_SET2;    //offset 0xD0,set SW intr bits
   UINT32 ulINTC_ISR_CLEAR2;  //offset 0xD4,clr SW intr bits
   UINT32 ulINTC_PENDING_IRQ2;   //offset 0xD8,IRQ status after
                              //masking
   UINT32 ulINTC_PENDING_FIQ2;   //offset 0xDC,FIQ status after
                              //masking
   UINT32 ulRESERVED_4[8];
   UINT32 ulINTC_ILR[96];     //offset 0x100, IRQ/FIQ priority 0-96
}
OMAP2420_MPUINTC_REGS;

//
// DSP Interrupt Controller
//

// Base Address :  IOMA
typedef volatile struct
{
   UINT32 ulINTC_REVISION;    //offset 0x0, IP revision code
   UINT32 ulRESERVED_04;
   UINT32 ulRESERVED_08;
   UINT32 ulRESERVED_0C;
   UINT32 ulINTC_SYSCONFIG;   //offset 0x10,OCP i/f params ctrl
   UINT32 ulINTC_SYSSTATUS;   //offset 0x14,status info of mod
   UINT32 ulRESERVED_1[10];
   UINT32 ulINTC_SIR_IRQ;     //offset 0x40,supplies active IRQ
                              //intr number
   UINT32 ulINTC_SIR_FIQ;     //offset 0x44,supplies active FIQ
                              //intr number
   UINT32 ulINTC_CONTROL;     //offset 0x48,global mask and new
                              //intr agreement bits
   UINT32 ulINTC_PROTECTION;  //offset 0x4C,ctrls protection of
                              //other registers
   UINT32 ulINTC_IDLE;        //offset 0x50,func clock auto-idle
                              //disable register
   UINT32 ulRESERVED_2[11];
   UINT32 ulINTC_ITR0;        //offset 0x80,raw intr i/p status
                              //before masking
   UINT32 ulINTC_MIR0;        //offset 0x84,intr mask reg
   UINT32 ulINTC_MIR_CLEAR0;  //offset 0x88,clear intr mask bits
   UINT32 ulINTC_MIR_SET0;    //offset 0x8C,set intr mask bits
   UINT32 ulINTC_ISR_SET0;    //offset 0x90,set SW intr bits
   UINT32 ulINTC_ISR_CLEAR0;  //offset 0x94,clear SW intr bits
   UINT32 ulINTC_PENDING_IRQ0;   //offset 0x98,IRQ stat aft masking
   UINT32 ulINTC_PENDING_FIQ0;   //offset 0x9C,FIQ stat aft masking
   UINT32 ulRESERVED_3[25];
   UINT32 ulINTC_ILR[32];     //offset 0x100-0x17C
                              //IRQ/FIQ prio 0-32
}
OMAP2420_DSPINTC_REGS;

//
// IVA Interrupt Controller
//

// Base Address : OMAP2420_INTC_IVA_REGS_PA (0x40000000)
typedef volatile struct
{
   UINT32 ulINTC_REVISION;             //offset 0x0, IP revision code
   UINT32 ulINTC_SYSCONFIG;   //offset 0x10,OCP i/f params ctrl
   UINT32 ulINTC_SYSSTATUS;   //offset 0x14,status info of mod
   UINT32 ulRESERVED_1[10];
   UINT32 ulINTC_SIR_IRQ;     //offset 0x40,supplies active IRQ
                                              //intr number
   UINT32 ulINTC_SIR_FIQ;     //offset 0x44,supplies active FIQ
                                              //intr number
   UINT32 ulINTC_CONTROL;     //offset 0x48,global mask and new
                                              //intr agreement bits
   UINT32 ulINTC_PROTECTION;  //offset 0x4C,ctrls protection of
                                              //other registers
   UINT32 ulINTC_IDLE;        //offset 0x50,func clock auto-idle
                                              //disable register
   UINT32 ulRESERVED_2[11];
   UINT32 ulINTC_ITR0;        //offset 0x80,raw intr i/p status
                                              //before masking
   UINT32 ulINTC_MIR0;        //offset 0x84,interrupt mask reg
   UINT32 ulINTC_MIR_CLEAR0;  //offset 0x88,clear intr mask bits
   UINT32 ulINTC_MIR_SET0;    //offset 0x8C,set intr mask bits
   UINT32 ulINTC_ISR_SET0;    //offset 0x90,set SW intr bits
   UINT32 ulINTC_ISR_CLEAR0;  //offset 0x94,clear SW intr bits
   UINT32 ulINTC_PENDING_IRQ0;   //offset 0x98,IRQ stat aft masking
   UINT32 ulINTC_PENDING_FIQ0;   //offset 0x9C,FIQ stat aft masking
   UINT32 ulRESERVED_3[25];
   UINT32 ulINTC_ILR[32];     //offset 0x100-0x17C
                                              //IRQ/FIQ priority 0-32
}
OMAP2420_IVAINTC_REGS;

#define OMAP2420_MPUINTC_RESETBIT    0x00000002
#define OMAP2420_MPUINTC_RESETSTATUS 0x00000001
#define OMAP2420_MPUINTC_MASKALL     0xFFFFFFFF


#define ILR_IRQ                 (0 << 0)
#define ILR_FIQ                 (1 << 0)

#define ILR_PRI0                (0 << 2)
#define ILR_PRI1                (1 << 2)
#define ILR_PRI2                (2 << 2)
#define ILR_PRI3                (3 << 2)
#define ILR_PRI4                (4 << 2)
#define ILR_PRI5                (5 << 2)
#define ILR_PRI6                (6 << 2)
#define ILR_PRI7                (7 << 2)
#define ILR_PRI8                (8 << 2)
#define ILR_PRI9                (9 << 2)
#define ILR_PRI10               (10 << 2)
#define ILR_PRI11               (11 << 2)
#define ILR_PRI12               (12 << 2)
#define ILR_PRI13               (13 << 2)
#define ILR_PRI14               (14 << 2)
#define ILR_PRI15               (15 << 2)
#define ILR_PRI16               (16 << 2)
#define ILR_PRI17               (17 << 2)
#define ILR_PRI18               (18 << 2)
#define ILR_PRI19               (19 << 2)
#define ILR_PRI20               (20 << 2)
#define ILR_PRI21               (21 << 2)
#define ILR_PRI22               (22 << 2)
#define ILR_PRI23               (23 << 2)
#define ILR_PRI24               (24 << 2)
#define ILR_PRI25               (25 << 2)
#define ILR_PRI26               (26 << 2)
#define ILR_PRI27               (27 << 2)
#define ILR_PRI28               (28 << 2)
#define ILR_PRI29               (29 << 2)
#define ILR_PRI30               (30 << 2)
#define ILR_PRI31               (31 << 2)
 
//------------------------------------------------------------------------------

#define CNTL_NEW_IRQ            (1 << 0)
#define CNTL_NEW_FIQ            (1 << 1)


#endif
