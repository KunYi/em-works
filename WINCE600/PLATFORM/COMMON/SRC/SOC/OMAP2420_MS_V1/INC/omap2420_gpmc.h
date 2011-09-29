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
//  File:  omap2420_gpmc.h
//
//  This header file is comprised of the registers of GPMC module
//

#ifndef __OMAP2420_GPMC_H
#define __OMAP2420_GPMC_H

//------------------------------------------------------------------------------
// Offsets from OMAP2420_GPMC_REGS_PA 0x6800A000

typedef volatile struct {
  
  UINT32 ulGPMC_REVISION;		   //offset 0x00, revision ID
  UINT32 ulRESERVED0x04_0x0F[3];
  UINT32 ulGPMC_SYSCONFIG; //offset 0x10, system config
  UINT32 ulGPMC_SYSSTATUS; //offset 0x14, status
  UINT32 ulGPMC_IRQSTATUS; //offset 0x18, IRQ status
  UINT32 ulGPMC_IRQENABLE; //offset 0x1C, IRQ enable
  UINT32 ulRESERVED0x20_0x3F[8];
  UINT32 ulGPMC_TIMEOUT_CONTROL;//offset 0x40,start value of
                                            //timeout counter
  UINT32 ulGPMC_ERR_ADDRESS;  //offset 0x44,addr of illegal access
                                           //when error occurs
  UINT32 ulGPMC_ERR_TYPE;  //offset 0x48,type of error
  UINT32 ulRESERVED0x4C;
  UINT32 ulGPMC_CONFIG;    //offset 0x50,global config of GPMC
  UINT32 ulGPMC_STATUS;    //offset 0x54,global status of GPMC
  UINT32 ulRESERVED0x58_0x5F[2];
  UINT32 ulGPMC_CONFIG1_0; //offset 0x60,signal ctrl params for
                                           //for CS0
  UINT32 ulGPMC_CONFIG2_0; //offset 0x64,signal timing param
                                           //configuration
  UINT32 ulGPMC_CONFIG3_0; //offset 0x68,nADV signal timing
  UINT32 ulGPMC_CONFIG4_0; //offset 0x6C,WE & OE signal timing
  UINT32 ulGPMC_CONFIG5_0; //offset 0x70,access time and
                                           //cycle time configuration
  UINT32 ulGPMC_CONFIG6_0; //offset 0x74,cycle2cycle
                                           // bus turnaround configuration
  UINT32 ulGPMC_CONFIG7_0; //offset 0x78,CS0 addr mapping conf
  UINT32 ulGPMC_NAND_COMMAND_0;  //offset 0x7C,NAND cmd register
  UINT32 ulGPMC_NAND_ADDRESS_0; //offset 0x80,NAND addr register
  UINT32 ulGPMC_NAND_DATA_0; //offset 0x84,NAND data register

  UINT32 ulRESERVED0x88_0x8F[2];
  UINT32 ulGPMC_CONFIG1_1; //offset 0x90,signal ctrl params for
                                           //for CS1
  UINT32 ulGPMC_CONFIG2_1; //offset 0x94,signal timing param
  UINT32 ulGPMC_CONFIG3_1; //offset 0x98,ADV signal timing
  UINT32 ulGPMC_CONFIG4_1; //offset 0x9C,WE & OE signal timing
  UINT32 ulGPMC_CONFIG5_1; //offset 0xA0,access time & cycle
                                           //time configuration
  UINT32 ulGPMC_CONFIG6_1; //offset 0xA4,cycle2cycle bus
                                           //turnaround configuration
  UINT32 ulGPMC_CONFIG7_1; //offset 0xA8,CS1 addr mapping conf
  UINT32 ulGPMC_NAND_COMMAND_1;//offset 0xAC,NAND cmd register
  UINT32 ulGPMC_NAND_ADDRESS_1; //offset 0xB0,NAND addr register
  UINT32 ulGPMC_NAND_DATA_1; //offset 0xB4,NAND data register
  UINT32 ulRESERVED0xB8_0xBF[2];
  UINT32 ulGPMC_CONFIG1_2; //offset 0xC0,signal ctrl params
                                           //for CS2
  UINT32 ulGPMC_CONFIG2_2; //offset 0xC4,signal timing param
  UINT32 ulGPMC_CONFIG3_2; //offset 0xC8,ADV signal timing
  UINT32 ulGPMC_CONFIG4_2; //offset 0xCC,WE & OE signal timing
  UINT32 ulGPMC_CONFIG5_2; //offset 0xD0,access time & cycle
                                           //time configuration
  UINT32 ulGPMC_CONFIG6_2; //offset 0xD4,cycle2cycle bus
                                           //turnaround configuration
  UINT32 ulGPMC_CONFIG7_2; //offset 0xD8,CS2 addr mapping conf
  UINT32 ulGPMC_NAND_COMMAND_2;//offset 0xDC,NAND cmd register
  UINT32 ulGPMC_NAND_ADDRESS_2; //offset 0xE0,NAND addr register
  UINT32 ulGPMC_NAND_DATA_2; //offset 0xE4,NAND data register
  UINT32 ulRESERVED0xE8_0xEF[2];
  UINT32 ulGPMC_CONFIG1_3; //offset 0xF0,signal ctrl params
                                           //for CS3
  UINT32 ulGPMC_CONFIG2_3; //offset 0xF4,signal timing param
  UINT32 ulGPMC_CONFIG3_3; //offset 0xF8,ADV signal timing
  UINT32 ulGPMC_CONFIG4_3; //offset 0xFC,WE & OE signal timing
  UINT32 ulGPMC_CONFIG5_3; //offset 0x100,access time & cycle
                                           //time configuration
  UINT32 ulGPMC_CONFIG6_3; //offset 0x104,cycle2cycle bus
                                           //turnaround configuration
  UINT32 ulGPMC_CONFIG7_3; //offset 0x108,CS3 addr mapping conf
  UINT32 ulGPMC_NAND_COMMAND_3;//offset 0x10C,NAND cmd register
  UINT32 ulGPMC_NAND_ADDRESS_3; //offset 0x110,NAND addr register
  UINT32 ulGPMC_NAND_DATA_3; //offset 0x114,NAND data register
  UINT32 ulRESERVED0x118_0x11F[2];
  UINT32 ulGPMC_CONFIG1_4; //offset 0x120,signal ctrl params
                                           //for CS4
  UINT32 ulGPMC_CONFIG2_4; //offset 0x124,signal timing param
  UINT32 ulGPMC_CONFIG3_4; //offset 0x128,ADV signal timing
  UINT32 ulGPMC_CONFIG4_4; //offset 0x12C,WE & OE signal timing
  UINT32 ulGPMC_CONFIG5_4; //offset 0x130,access time & cycle
                                           //time configuration
  UINT32 ulGPMC_CONFIG6_4; //offset 0x134,cycle2cycle bus
                                           //turnaround configuration
  UINT32 ulGPMC_CONFIG7_4; //offset 0x138,CS4 addr mapping conf
  UINT32 ulGPMC_NAND_COMMAND_4;//offset 0x13C,NAND cmd register
  UINT32 ulGPMC_NAND_ADDRESS_4;//offset 0x140,NAND addr register
  UINT32 ulGPMC_NAND_DATA_4; //offset 0x144,NAND data register
  UINT32 ulRESERVED0x148_0x14F[2];
  UINT32 ulGPMC_CONFIG1_5; //offset 0x150,signal ctrl parms for
                                           //CS5
  UINT32 ulGPMC_CONFIG2_5; //offset 0x154,signal timing params
  UINT32 ulGPMC_CONFIG3_5; //offset 0x158,ADV signal timing
  UINT32 ulGPMC_CONFIG4_5; //offset 0x15C,WE & OE signal timing
  UINT32 ulGPMC_CONFIG5_5; //offset 0x160,access time & cycle
                                           //time configuration
  UINT32 ulGPMC_CONFIG6_5; //offset 0x164,cycle2cycle bus
                                           //turnaround configuration
  UINT32 ulGPMC_CONFIG7_5; //offset 0x168,CS5 addr mapping conf
  UINT32 ulGPMC_NAND_COMMAND_5;//offset 0x16C,NAND cmd register
  UINT32 ulGPMC_NAND_ADDRESS_5; //offset 0x170,NAND addr register
  UINT32 ulGPMC_NAND_DATA_5; //offset 0x174,NAND data register
  UINT32 ulRESERVED0x178_0x17F[2];
  UINT32 ulGPMC_CONFIG1_6; //offset 0x180,signal ctrl params for
                                           //CS6
  UINT32 ulGPMC_CONFIG2_6; //offset 0x184,signal timing params
  UINT32 ulGPMC_CONFIG3_6; //offset 0x188,ADV signal timing
  UINT32 ulGPMC_CONFIG4_6; //offset 0x18C,WE & OE signal timing
  UINT32 ulGPMC_CONFIG5_6; //offset 0x190,access time & cycle
                                           //time configuration
  UINT32 ulGPMC_CONFIG6_6; //offset 0x194,cycle2cycle bus
                                           //turnaround configuration
  UINT32 ulGPMC_CONFIG7_6; //offset 0x198,CS6 addr mapping conf
  UINT32 ulGPMC_NAND_COMMAND_6;//offset 0x19C,NAND cmd register
  UINT32 ulGPMC_NAND_ADDRESS_6; //offset 0x1A0,NAND addr register
  UINT32 ulGPMC_NAND_DATA_6; //offset 0x1A4,NAND data register
  UINT32 ulRESERVED0x1A8_0x1AF[2];
  UINT32 ulGPMC_CONFIG1_7; //offset 0x1B0,signal ctrl params for
                                           //CS7
  UINT32 ulGPMC_CONFIG2_7; //offset 0x1B4,signal timing params
  UINT32 ulGPMC_CONFIG3_7; //offset 0x1B8,ADV signal timing
  UINT32 ulGPMC_CONFIG4_7; //offset 0x1BC,WE & OE signal timing
  UINT32 ulGPMC_CONFIG5_7; //offset 0x1C0,access time & cycle
                                           //time configuration
  UINT32 ulGPMC_CONFIG6_7; //offset 0x1C4,cycle2cycle bus
                                           //turnaround configuration
  UINT32 ulGPMC_CONFIG7_7; //offset 0x1C8,CS7 addr mapping conf
  UINT32 ulGPMC_NAND_COMMAND_7;//offset 0x1CC,NAND cmd register
  UINT32 ulGPMC_NAND_ADDRESS_7; //offset 0x1D0,NAND addr register
  UINT32 ulGPMC_NAND_DATA_7; //offset 0x1D4,NAND data register
  UINT32 ulRESERVED0x1D8_0x1DF[2];
  UINT32 ulGPMC_PREFETCH_CONFIG1; //offset 0x1E0,prefetch engine
                                                  //configuration - 1
  UINT32 ulGPMC_PREFETCH_CONFIG2; //offset 0x1E4,prefetch engine
                                                  //configuration - 2
  UINT32 ulRESERVED_0x1E8;
  UINT32 ulGPMC_PREFETCH_CONTROL; //offset 0x1EC,prefetch engine
                                                  //control
  UINT32 ulGPMC_PREFETCH_STATUS;  //offset 0x1F0,prefetch engine
                                                  //status
  UINT32 ulGPMC_ECC_CONFIG;       //offset 0x1F4,ECC config
  UINT32 ulGPMC_ECC_CONTROL;      //offset 0x1F8,ECC control
  UINT32 ulGPMC_ECC_SIZE_CONFIG;  //offset 0x1FC,ECC size
  UINT32 ulGPMC_ECC1_RESULT;      //offset 0x200,ECC1 result
  UINT32 ulGPMC_ECC2_RESULT;      //offset 0x204,ECC2 result
  UINT32 ulGPMC_ECC3_RESULT;      //offset 0x208,ECC3 result
  UINT32 ulGPMC_ECC4_RESULT;      //offset 0x20C,ECC4 result
  UINT32 ulGPMC_ECC5_RESULT;      //offset 0x210,ECC5 result
  UINT32 ulGPMC_ECC6_RESULT;      //offset 0x214,ECC6 result
  UINT32 ulGPMC_ECC7_RESULT;      //offset 0x218,ECC7 result
  UINT32 ulGPMC_ECC8_RESULT;      //offset 0x21C,ECC8 result
  UINT32 ulGPMC_ECC9_RESULT;      //offset 0x220,ECC9 result
  UINT32 ulRESERVED0x224_0x23F[3];
  UINT32 ulGPMC_TESTMODE_CTRL;    //offset 0x230,Test mode ctrl
  UINT32 ulGPMC_PSA_LSB;          //offset 0x234,PSA-LSB
  UINT32 ulGPMC_PSA_MSB;          //offset 0x238,PSA-MSB

}OMAP2420_GPMC_REGS;


#endif
