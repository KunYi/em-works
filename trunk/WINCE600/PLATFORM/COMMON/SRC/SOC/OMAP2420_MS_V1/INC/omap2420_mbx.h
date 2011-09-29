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
//  File:  omap2420_mbx.h
//
//  This header file is comprised of registers for MailBox (MBX) 
//

#ifndef __OMAP2420_MBX_H
#define __OMAP2420_MBX_H

//------------------------------------------------------------------------------

typedef struct __MBXREGS__
{
   unsigned long ulMAILBOX_REVISION;           //offset 0x0, Revision;
   unsigned long ulRESERVED_1[3];
   volatile unsigned long ulMAILBOX_SYSCONFIG; //offset 0x10, System config
   volatile unsigned long ulMAILBOX_SYSSTATUS; //offset 0x14, System status
   unsigned long ulRESERVED_2[10];
   volatile unsigned long ulMAILBOX_MESSAGE_0; //offset 0x40, MBX0 Message 
   volatile unsigned long ulMAILBOX_MESSAGE_1; //offset 0x44, MBX1 Message
   volatile unsigned long ulMAILBOX_MESSAGE_2; //offset 0x48, MBX2 Message
   volatile unsigned long ulMAILBOX_MESSAGE_3; //offset 0x4C, MBX3 Message
   volatile unsigned long ulMAILBOX_MESSAGE_4; //offset 0x50, MBX4 Message
   volatile unsigned long ulMAILBOX_MESSAGE_5; //offset 0x54, MBX5 Message
   unsigned long ulRESERVED_3[10];
   volatile unsigned long ulMAILBOX_FIFOSTATUS_0;//offset 0x80, MBX0 FIFO status
   volatile unsigned long ulMAILBOX_FIFOSTATUS_1;//offset 0x84, MBX1 FIFO status    
   volatile unsigned long ulMAILBOX_FIFOSTATUS_2;//offset 0x88, MBX2 FIFO status
   volatile unsigned long ulMAILBOX_FIFOSTATUS_3;//offset 0x8C, MBX3 FIFO status**
   volatile unsigned long ulMAILBOX_FIFOSTATUS_4;//offset 0x90, MBX0 FIFO status
   volatile unsigned long ulMAILBOX_FIFOSTATUS_5;//offset 0x94, MBX0 FIFO status
   unsigned long ulRESERVED_4[10];
   volatile unsigned long ulMAILBOX_MSGSTATUS_0;//offset 0xC0, MBX0 MSG status
   volatile unsigned long ulMAILBOX_MSGSTATUS_1;//offset 0xC4, MBX1 MSG status
   volatile unsigned long ulMAILBOX_MSGSTATUS_2;//offset 0xC8, MBX2 MSG status
   volatile unsigned long ulMAILBOX_MSGSTATUS_3;//offset 0xCC, MBX3 MSG status**
   volatile unsigned long ulMAILBOX_MSGSTATUS_4;//offset 0xD0, MBX4 MSG status
   volatile unsigned long ulMAILBOX_MSGSTATUS_5;//offset 0xD4, MBX5 MSG status
   unsigned long ulRESERVED_5[10];
   volatile unsigned long ulMAILBOX_IRQSTATUS_0;//offset0x100,MPU(Usr0)IRQ stat
   volatile unsigned long ulMAILBOX_IRQENABLE_0;//offset0x104,MPU(Usr0)IRQ ENBL
   volatile unsigned long ulMAILBOX_IRQSTATUS_1;//offset0x108,DSP(Usr1)IRQ stat
   volatile unsigned long ulMAILBOX_IRQENABLE_1;//offset0x10C,DSP(Usr1)IRQ ENBL
   volatile unsigned long ulMAILBOX_IRQSTATUS_2;//offset0x110,IVA(Usr2)IRQ stat
   volatile unsigned long ulMAILBOX_IRQENABLE_2;//offset0x114,IVA(Usr2)IRQ ENBL
   volatile unsigned long ulMAILBOX_IRQSTATUS_3;//offset0x118,MPU(Usr3)IRQ stat
   volatile unsigned long ulMAILBOX_IRQENABLE_3;//offset0x11C,MPU(Usr3)IRQ ENBL
}
OMAP2420_MAILBOX_REGS, *pMBXREGS;

#endif
