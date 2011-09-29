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
//  File:  omap2420_MMU.h
//
//  This header file is comprised of register details of 
//  MMU's(CAMERA, DSP & IVA)
//

#ifndef __OMAP2420_MMU_H
#define __OMAP2420_MMU_H

//------------------------------------------------------------------------------



typedef struct __DSPMMUREGS__
{
  unsigned long ulMMU_REVISION;           //offset 0x0,IP revision code
  unsigned long ulRESERVED_1[3];              
  volatile unsigned long ulMMU_SYSCONFIG; //offset 0x10,OCP I/F params config
  volatile unsigned long ulMMU_SYSSTATUS; //offset 0x14,module status info
  volatile unsigned long ulMMU_IRQSTATUS; //offset 0x18,intr status of module 
                                          //internal events
  volatile unsigned long ulMMU_IRQENABLE; //offset 0x1C,module internal source 
                                          //(intr) mask/unmask
  unsigned long ulRESERVED_2[8];
  volatile unsigned long ulMMU_WALKING_ST;//offset 0x40,status info abt TBL 
                                          //walking logic
  volatile unsigned long ulMMU_CNTL;      //offset 0x44,MMU features
  volatile unsigned long ulMMU_FAULT_AD;  //offset 0x48,virtual addr of access 
                                          //that generated a fault
  volatile unsigned long ulMMU_TTB;       //offset 0x4C,translation TBL base addr
  volatile unsigned long ulMMU_LOCK;      //offset 0x50,Locks TLB entries or 
                                          //specifies TLB entry to read
  volatile unsigned long ulMMU_LD_TLB;    //offset 0x54,Loads a TLB entry
  volatile unsigned long ulMMU_CAM;       //offset 0x58,Holds a CAM entry
  volatile unsigned long ulMMU_RAM;       //offset 0x5C,Holds a RAM entry
  volatile unsigned long ulMMU_GFLUSH;    //offset 0x60,Flushes the 
                                          //non-protected TLB entries
  volatile unsigned long ulMMU_FLUSH_ENTRY;//offset 0x64,Flushes the entry 
                                           //pointed to by CAM virtual addr
  volatile unsigned long ulMMU_READ_CAM;  //offset 0x68, Reads CAM data from 
                                          //CAM entry
  volatile unsigned long ulMMU_READ_RAM;  //offset 0x6C, Reads RAM data from 
                                          //RAM entry
  volatile unsigned long ulMMU_EMU_FAULT_AD;//offset 0x70, last virtual addr 
                                            //of a fault due to debugger
}
OMAP2420_DSP_MMU_REGS, *pDSPMMUREGS;

//
// Camera MMU
//

// Base Address : 0x48052C00

typedef struct __CAMMMUREGS__
{
  unsigned long ulMMU_REVISION;           //offset 0x0,IP revision code
  unsigned long ulRESERVED_1[3];              
  volatile unsigned long ulMMU_SYSCONFIG; //offset 0x10,OCP I/F params config
  volatile unsigned long ulMMU_SYSSTATUS; //offset 0x14,module status info
  volatile unsigned long ulMMU_IRQSTATUS; //offset 0x18,intr status of module 
                                          //internal events
  volatile unsigned long ulMMU_IRQENABLE; //offset 0x1C,module internal 
                                          //source (intr) mask/unmask
  unsigned long ulRESERVED_2[8];
  volatile unsigned long ulMMU_WALKING_ST;//offset 0x40,status info abt TBL 
                                          //walking logic
  volatile unsigned long ulMMU_CNTL;      //offset 0x44,MMU features
  volatile unsigned long ulMMU_FAULT_AD;  //offset 0x48,virtual addr of access
                                          //that generated a fault
  volatile unsigned long ulMMU_TTB;       //offset 0x4C,transl TBL base addr
  volatile unsigned long ulMMU_LOCK;      //offset 0x50,Locks TLB entries 
                                          //or specifies TLB entry to read
  volatile unsigned long ulMMU_LD_TLB;    //offset 0x54,Loads a TLB entry
  volatile unsigned long ulMMU_CAM;       //offset 0x58,Holds a CAM entry
  volatile unsigned long ulMMU_RAM;       //offset 0x5C,Holds a RAM entry
  volatile unsigned long ulMMU_GFLUSH;    //offset 0x60,Flushes the 
                                          //non-protected TLB entries
  volatile unsigned long ulMMU_FLUSH_ENTRY;//offset 0x64,Flushes the entry
                                           //pointed to by CAM virtual addr
  volatile unsigned long ulMMU_READ_CAM;  //offset 0x68, Reads CAM data from 
                                          //CAM entry
  volatile unsigned long ulMMU_READ_RAM;  //offset 0x6C, Reads RAM data from 
                                          //RAM entry
  volatile unsigned long ulMMU_EMU_FAULT_AD;//offset 0x70, last virtual addr 
                                            //of a fault due to debugger
}
OMAP2420_CAM_MMU_REGS, *pCAMMMUREGS;

//
// IVA MMU (OMAP 2420 only)
//

// Base Address : 0x5D000000

typedef struct __IVAMMUREGS__
{
  unsigned long ulMMU_REVISION;           //offset 0x0,IP revision code
  unsigned long ulRESERVED_1[3];              
  volatile unsigned long ulMMU_SYSCONFIG; //offset 0x10,OCP I/F params config
  volatile unsigned long ulMMU_SYSSTATUS; //offset 0x14,module status info
  volatile unsigned long ulMMU_IRQSTATUS; //offset 0x18,intr status of module 
                                          //internal events
  volatile unsigned long ulMMU_IRQENABLE; //offset 0x1C,module internal source 
                                          //(intr) mask/unmask
  unsigned long ulRESERVED_2[8];
  volatile unsigned long ulMMU_WALKING_ST;//offset 0x40,status info abt TBL 
                                          //walking logic
  volatile unsigned long ulMMU_CNTL;      //offset 0x44,MMU features
  volatile unsigned long ulMMU_FAULT_AD;  //offset 0x48,virtual addr of access 
                                          //that generated a fault
  volatile unsigned long ulMMU_TTB;       //offset 0x4C,translation TBL base addr
  volatile unsigned long ulMMU_LOCK;      //offset 0x50,Locks TLB entries or 
                                          //specifies TLB entry to read
  volatile unsigned long ulMMU_LD_TLB;    //offset 0x54,Loads a TLB entry
  volatile unsigned long ulMMU_CAM;       //offset 0x58,Holds a CAM entry
  volatile unsigned long ulMMU_RAM;       //offset 0x5C,Holds a RAM entry
  volatile unsigned long ulMMU_GFLUSH;    //offset 0x60,Flushes the 
                                          //non-protected TLB entries
  volatile unsigned long ulMMU_FLUSH_ENTRY;//offset 0x64,Flushes the entry 
                                           //pointed to by CAM virtual addr
  volatile unsigned long ulMMU_READ_CAM;  //offset 0x68, Reads CAM data from 
                                          //CAM entry
  volatile unsigned long ulMMU_READ_RAM;  //offset 0x6C, Reads RAM data from 
                                          //RAM entry
  volatile unsigned long ulMMU_EMU_FAULT_AD;//offset 0x70, last virtual addr 
                                            //of a fault due to debugger
}
OMAP2420_IVA_MMU_REGS, *pIVAMMUREGS;


#endif
