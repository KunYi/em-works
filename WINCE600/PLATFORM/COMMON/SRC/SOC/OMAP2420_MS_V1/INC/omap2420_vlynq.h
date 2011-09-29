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
//  File:  omap2420_vlynq.h
//
//  This header file is comprised of vlynq module register details defined as 
//  structures and macros for controlling vlynq.

#ifndef __OMAP2420_VLYNQ_H
#define __OMAP2420_VLYNQ_H

//------------------------------------------------------------------------------

typedef struct __VLYNQFUNCREGS__
{
   unsigned long ulV2O_REVISION;              //offset 0x0,IP revision code
   unsigned long ulRESERVED_1[3];
   volatile unsigned long ulV2O_SYSCONFIG;    //offset 0x10,OCP i/f ctrl params
   volatile unsigned long ulV2O_SYSSTATUS;    //offset 0x14,module status info
   volatile unsigned long ulV2O_IRQSTATUS;    //offset 0x18,IRQ status of module 
                                              //intl events
   volatile unsigned long ulV2O_IRQENABLE;    //offset 0x1C,enable/disable for 
                                              //module interrupts
   unsigned long ulRESERVED_2[8];
   volatile unsigned long ulV2O_CONTROL;      //offset 0x40,endianess & pwr mgmt 
                                              //feature control
   volatile unsigned long ulV2O_DMAREQ;       //offset 0x44,DMA feature control
   volatile unsigned long ulV2O_ERROR;        //offset 0x48,master error info
   unsigned long ulRESERVED_3[45];
   volatile unsigned long ulVL_REVISION;      //offset 0x100,module revision 
                                              //and unique module ID
   volatile unsigned long ulVL_CONTROL;       //offset 0x104,VLYNQ module 
                                              //operation control
   volatile unsigned long ulVL_STATUS;        //offset 0x108, VLYNQ status
   volatile unsigned long ulVL_IRQ_PRTY_VEC_STS;//offset 0x10C,interrupt vector 
                                                //priority/status
   volatile unsigned long ulVL_IRQSTATUS;     //offset 0x110,interrupt 
                                              //status/clear for unmasked intr
   volatile unsigned long ulVL_IRQPENDING;    //offset 0x114,pending intr status
   volatile unsigned long ulVL_IRQPOINTER;    //offset 0x118,ptr to the addr of 
                                              //intr set reg for the dev
   volatile unsigned long ulVL_TX_ADDR_MAP;   //offset 0x11C,TX address map reg
   volatile unsigned long ulVL_RX_ADDR_MAP_SZ1;//offset 0x120,RX addr map size1
   volatile unsigned long ulVL_RX_ADDR_MAP_OS1;//offset 0x124,RX addr map 
                                               //offset reg
   volatile unsigned long ulVL_RX_ADDR_MAP_SZ2;//offset 0x128,RX address map
                                               //size2 reg
   volatile unsigned long ulVL_RX_ADDR_MAP_OS2;//offset 0x12C,RX address map 
                                               //offset2 reg
   volatile unsigned long ulVL_RX_ADDR_MAP_SZ3;//offset 0x130,RX address map 
                                               //size3 reg
   volatile unsigned long ulVL_RX_ADDR_MAP_OS3;//offset 0x134,RX address map 
                                               //offset3 reg
   volatile unsigned long ulVL_RX_ADDR_MAP_SZ4;//offset 0x138,RX address map 
                                               //size4 reg
   volatile unsigned long ulVL_RX_ADDR_MAP_OS4;//offset 0x13C,RX address map 
                                               //offset4 reg
   volatile unsigned long ulVL_CHIP_VERSION;   //offset 0x140,type and ver 
                                               //info of VLYNQ devices
   volatile unsigned long ulVL_AUTO_NEGOTIATION;//offset 0x144,auto negat reg
   volatile unsigned long ulVL_MAN_NEGOTIATION; //offset 0x148,RESERVED
   
   volatile unsigned long ulVL_NEG_STATUS;     //offset 0x14C, RESERVED
   unsigned long ulRESERVED_4[3];
   volatile unsigned long ulVL_ENDIAN;         //offset 0x15C,RESERVED
   volatile unsigned long ulVL_IRQ_VEC_3_0;    //offset 0x160,RESERVED
   volatile unsigned long ulVL_IRQ_VEC_7_4;    //offset 0x164,RESERVED
   unsigned long ulRESERVED_5[6];   
   volatile unsigned long ulVL_R_REVISION;     //offset 0x180,remote dev
                                               //rev id
   volatile unsigned long ulVL_R_CONTROL;      //offset 0x184,remote device
                                               //control reg
   volatile unsigned long ulVL_R_STATUS;       //offset 0x188,remote device 
                                               //status reg
   volatile unsigned long ulVL_R_IRQ_PRTY_VEC_STS;//offset 0x18C,remote device 
                                                  //IRQ priority vector/status
   volatile unsigned long ulVL_R_IRQSTATUS;    //offset 0x190,remote device 
                                               //IRQ status reg
   volatile unsigned long ulVL_R_IRQPENDING;   //offset 0x194,remote device 
                                               //IRQ pending info reg
   volatile unsigned long ulVL_R_IRQPOINTER;   //offset 0x198,remote device 
                                               //IRQ pointer reg
   volatile unsigned long ulVL_R_TX_ADDR_MAP;  //offset 0x19C,remote device 
                                               //TX address map reg
   volatile unsigned long ulVL_R_RX_ADDR_MAP_SZ1;//offset 0x1A0,remote device 
                                                 //RX address map size1 reg
   volatile unsigned long ulVL_R_RX_ADDR_MAP_OS1;//offset 0x1A4,remote device 
                                                 //RX address map offset1 reg
   volatile unsigned long ulVL_R_RX_ADDR_MAP_SZ2;//offset 0x1A8,remote device 
                                                 //RX address map size2 reg
   volatile unsigned long ulVL_R_RX_ADDR_MAP_OS2;//offset 0x1AC,remote device 
                                                 //RX address map offset2 reg
   volatile unsigned long ulVL_R_RX_ADDR_MAP_SZ3;//offset 0x1B0,remote device 
                                                 //RX address map size3 reg
   volatile unsigned long ulVL_R_RX_ADDR_MAP_OS3;//offset 0x1B4,remote device 
                                                 //RX address map offset3 reg
   volatile unsigned long ulVL_R_RX_ADDR_MAP_SZ4;//offset 0x1B8,remote device 
                                                 //RX address map size4 reg
   volatile unsigned long ulVL_R_RX_ADDR_MAP_OS4;//offset 0x1BC,remote device 
                                                 //RX address map offset4 reg
   volatile unsigned long ulVL_R_CHIP_VERSION;   //offset 0x1C0,remote device 
                                                 //type and ver info of VLYNQ
                                                 //devices
   volatile unsigned long ulVL_R_AUTO_NEGOTIATION;//offset 0x1C4,remote device 
                                                  //auto negatiation register
   volatile unsigned long ulVL_R_MAN_NEGOTIATION; //offset 0x1C8,remote device
   volatile unsigned long ulVL_R_NEG_STATUS;     //offset 0x1CC,remote device
   unsigned long ulRESERVED_6[3];
   volatile unsigned long ulVL_R_ENDIAN;         //offset 0x1DC,remote device
   volatile unsigned long ulVL_R_IRQ_VEC_3_0;    //offset 0x1E0,remote device 
                                                 //IRQ vectors 3-0 reg
   volatile unsigned long ulVL_R_IRQ_VEC_7_4;    //offset 0x1E4, remote device 
                                                 //IRQ vectors 7-4 reg
}
OMAP2420_VLYNQFUNC_REGS, *pVLYNQFUNCREGS;


#endif
