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
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/

//------------------------------------------------------------------------------
//
//  File:  omap35xx_gpmc.h
//
//  This header file is comprised of the registers of GPMC module
//

#ifndef __OMAP35XX_GPMC_H
#define __OMAP35XX_GPMC_H

//------------------------------------------------------------------------------

// OMAP_GPMC_REGS
//
typedef volatile struct {
  
    UINT32 GPMC_REVISION;             //offset 0x000, revision ID
    UINT32 zzzReserved0x04_0x0F[3];
    UINT32 GPMC_SYSCONFIG;            //offset 0x010, system config
    UINT32 GPMC_SYSSTATUS;            //offset 0x014, status
    UINT32 GPMC_IRQSTATUS;            //offset 0x018, IRQ status
    UINT32 GPMC_IRQENABLE;            //offset 0x01C, IRQ enable
    UINT32 zzzReserved0x20_0x3F[8];
    UINT32 GPMC_TIMEOUT_CONTROL;      //offset 0x040, start value of timeout counter
    UINT32 GPMC_ERR_ADDRESS;          //offset 0x044, addr of illegal access when error occurs
    UINT32 GPMC_ERR_TYPE;             //offset 0x048, type of error
    UINT32 zzzReserved0x4C;
    UINT32 GPMC_CONFIG;               //offset 0x050, global config of GPMC
    UINT32 GPMC_STATUS;               //offset 0x054, global status of GPMC
    UINT32 zzzReserved0x58_0x5F[2];
    
    UINT32 GPMC_CONFIG1_0;            //offset 0x060, signal ctrl params for CS0
    UINT32 GPMC_CONFIG2_0;            //offset 0x064, signal timing param configuration
    UINT32 GPMC_CONFIG3_0;            //offset 0x068, nADV signal timing
    UINT32 GPMC_CONFIG4_0;            //offset 0x06C, WE & OE signal timing
    UINT32 GPMC_CONFIG5_0;            //offset 0x070, access time and cycle time configuration
    UINT32 GPMC_CONFIG6_0;            //offset 0x074, cycle2cycle bus turnaround configuration
    UINT32 GPMC_CONFIG7_0;            //offset 0x078, addr mapping conf
    UINT32 GPMC_NAND_COMMAND_0;       //offset 0x07C, NAND cmd register
    UINT32 GPMC_NAND_ADDRESS_0;       //offset 0x080, NAND addr register
    UINT32 GPMC_NAND_DATA_0;          //offset 0x084, NAND data register
    UINT32 zzzReserved0x88_0x8F[2];

    UINT32 GPMC_CONFIG1_1;            //offset 0x090, signal ctrl params for CS1
    UINT32 GPMC_CONFIG2_1;            //offset 0x094, signal timing param configuration
    UINT32 GPMC_CONFIG3_1;            //offset 0x098, nADV signal timing
    UINT32 GPMC_CONFIG4_1;            //offset 0x09C, WE & OE signal timing
    UINT32 GPMC_CONFIG5_1;            //offset 0x0A0, access time and cycle time configuration
    UINT32 GPMC_CONFIG6_1;            //offset 0x0A4, cycle2cycle bus turnaround configuration
    UINT32 GPMC_CONFIG7_1;            //offset 0x0A8, addr mapping conf
    UINT32 GPMC_NAND_COMMAND_1;       //offset 0x0AC, NAND cmd register
    UINT32 GPMC_NAND_ADDRESS_1;       //offset 0x0B0, NAND addr register
    UINT32 GPMC_NAND_DATA_1;          //offset 0x0B4, NAND data register
    UINT32 zzzReserved0xB8_0xBF[2];

    UINT32 GPMC_CONFIG1_2;            //offset 0x0C0, signal ctrl params for CS2
    UINT32 GPMC_CONFIG2_2;            //offset 0x0C4, signal timing param configuration
    UINT32 GPMC_CONFIG3_2;            //offset 0x0C8, nADV signal timing
    UINT32 GPMC_CONFIG4_2;            //offset 0x0CC, WE & OE signal timing
    UINT32 GPMC_CONFIG5_2;            //offset 0x0D0, access time and cycle time configuration
    UINT32 GPMC_CONFIG6_2;            //offset 0x0D4, cycle2cycle bus turnaround configuration
    UINT32 GPMC_CONFIG7_2;            //offset 0x0D8, addr mapping conf
    UINT32 GPMC_NAND_COMMAND_2;       //offset 0x0DC, NAND cmd register
    UINT32 GPMC_NAND_ADDRESS_2;       //offset 0x0E0, NAND addr register
    UINT32 GPMC_NAND_DATA_2;          //offset 0x0E4, NAND data register
    UINT32 zzzReserved0xE8_0xEF[2];

    UINT32 GPMC_CONFIG1_3;            //offset 0x0F0, signal ctrl params for CS3
    UINT32 GPMC_CONFIG2_3;            //offset 0x0F4, signal timing param configuration
    UINT32 GPMC_CONFIG3_3;            //offset 0x0F8, nADV signal timing
    UINT32 GPMC_CONFIG4_3;            //offset 0x0FC, WE & OE signal timing
    UINT32 GPMC_CONFIG5_3;            //offset 0x100, access time and cycle time configuration
    UINT32 GPMC_CONFIG6_3;            //offset 0x104, cycle2cycle bus turnaround configuration
    UINT32 GPMC_CONFIG7_3;            //offset 0x108, addr mapping conf
    UINT32 GPMC_NAND_COMMAND_3;       //offset 0x10C, NAND cmd register
    UINT32 GPMC_NAND_ADDRESS_3;       //offset 0x110, NAND addr register
    UINT32 GPMC_NAND_DATA_3;          //offset 0x114, NAND data register
    UINT32 zzzReserved0x118_0x11F[2];

    UINT32 GPMC_CONFIG1_4;            //offset 0x120, signal ctrl params for CS4
    UINT32 GPMC_CONFIG2_4;            //offset 0x124, signal timing param configuration
    UINT32 GPMC_CONFIG3_4;            //offset 0x128, nADV signal timing
    UINT32 GPMC_CONFIG4_4;            //offset 0x12C, WE & OE signal timing
    UINT32 GPMC_CONFIG5_4;            //offset 0x130, access time and cycle time configuration
    UINT32 GPMC_CONFIG6_4;            //offset 0x134, cycle2cycle bus turnaround configuration
    UINT32 GPMC_CONFIG7_4;            //offset 0x138, addr mapping conf
    UINT32 GPMC_NAND_COMMAND_4;       //offset 0x13C, NAND cmd register
    UINT32 GPMC_NAND_ADDRESS_4;       //offset 0x140, NAND addr register
    UINT32 GPMC_NAND_DATA_4;          //offset 0x144, NAND data register
    UINT32 zzzReserved0x148_0x14F[2];

    UINT32 GPMC_CONFIG1_5;            //offset 0x150, signal ctrl params for CS4
    UINT32 GPMC_CONFIG2_5;            //offset 0x154, signal timing param configuration
    UINT32 GPMC_CONFIG3_5;            //offset 0x158, nADV signal timing
    UINT32 GPMC_CONFIG4_5;            //offset 0x15C, WE & OE signal timing
    UINT32 GPMC_CONFIG5_5;            //offset 0x160, access time and cycle time configuration
    UINT32 GPMC_CONFIG6_5;            //offset 0x164, cycle2cycle bus turnaround configuration
    UINT32 GPMC_CONFIG7_5;            //offset 0x168, addr mapping conf
    UINT32 GPMC_NAND_COMMAND_5;       //offset 0x16C, NAND cmd register
    UINT32 GPMC_NAND_ADDRESS_5;       //offset 0x170, NAND addr register
    UINT32 GPMC_NAND_DATA_5;          //offset 0x174, NAND data register
    UINT32 zzzReserved0x178_0x17F[2];

    UINT32 GPMC_CONFIG1_6;            //offset 0x180, signal ctrl params for CS4
    UINT32 GPMC_CONFIG2_6;            //offset 0x184, signal timing param configuration
    UINT32 GPMC_CONFIG3_6;            //offset 0x188, nADV signal timing
    UINT32 GPMC_CONFIG4_6;            //offset 0x18C, WE & OE signal timing
    UINT32 GPMC_CONFIG5_6;            //offset 0x190, access time and cycle time configuration
    UINT32 GPMC_CONFIG6_6;            //offset 0x194, cycle2cycle bus turnaround configuration
    UINT32 GPMC_CONFIG7_6;            //offset 0x198, addr mapping conf
    UINT32 GPMC_NAND_COMMAND_6;       //offset 0x19C, NAND cmd register
    UINT32 GPMC_NAND_ADDRESS_6;       //offset 0x1A0, NAND addr register
    UINT32 GPMC_NAND_DATA_6;          //offset 0x1A4, NAND data register
    UINT32 zzzReserved0x1A8_0x1AF[2];

    UINT32 GPMC_CONFIG1_7;            //offset 0x1B0, signal ctrl params for CS4
    UINT32 GPMC_CONFIG2_7;            //offset 0x1B4, signal timing param configuration
    UINT32 GPMC_CONFIG3_7;            //offset 0x1B8, nADV signal timing
    UINT32 GPMC_CONFIG4_7;            //offset 0x1BC, WE & OE signal timing
    UINT32 GPMC_CONFIG5_7;            //offset 0x1C0, access time and cycle time configuration
    UINT32 GPMC_CONFIG6_7;            //offset 0x1C4, cycle2cycle bus turnaround configuration
    UINT32 GPMC_CONFIG7_7;            //offset 0x1C8, addr mapping conf
    UINT32 GPMC_NAND_COMMAND_7;       //offset 0x1CC, NAND cmd register
    UINT32 GPMC_NAND_ADDRESS_7;       //offset 0x1D0, NAND addr register
    UINT32 GPMC_NAND_DATA_7;          //offset 0x1D4, NAND data register
    UINT32 zzzReserved0x1D8_0x1DF[2];

    UINT32 GPMC_PREFETCH_CONFIG1;     //offset 0x1E0,prefetch engine configuration - 1
    UINT32 GPMC_PREFETCH_CONFIG2;     //offset 0x1E4,prefetch engine configuration - 2
    UINT32 zzzReserved_0x1E8;
    UINT32 GPMC_PREFETCH_CONTROL;     //offset 0x1EC,prefetch engine control
    UINT32 GPMC_PREFETCH_STATUS;      //offset 0x1F0,prefetch engine status

    UINT32 GPMC_ECC_CONFIG;           //offset 0x1F4,ECC config
    UINT32 GPMC_ECC_CONTROL;          //offset 0x1F8,ECC control
    UINT32 GPMC_ECC_SIZE_CONFIG;      //offset 0x1FC,ECC size
    UINT32 GPMC_ECC1_RESULT;          //offset 0x200,ECC1 result
    UINT32 GPMC_ECC2_RESULT;          //offset 0x204,ECC2 result
    UINT32 GPMC_ECC3_RESULT;          //offset 0x208,ECC3 result
    UINT32 GPMC_ECC4_RESULT;          //offset 0x20C,ECC4 result
    UINT32 GPMC_ECC5_RESULT;          //offset 0x210,ECC5 result
    UINT32 GPMC_ECC6_RESULT;          //offset 0x214,ECC6 result
    UINT32 GPMC_ECC7_RESULT;          //offset 0x218,ECC7 result
    UINT32 GPMC_ECC8_RESULT;          //offset 0x21C,ECC8 result
    UINT32 GPMC_ECC9_RESULT;          //offset 0x220,ECC9 result


//  UINT32 zzzReserved0x224_0x23F[3];

//  UINT32 GPMC_TESTMODE_CTRL;        //offset 0x230,Test mode ctrl
//  UINT32 GPMC_PSA_LSB;              //offset 0x234,PSA-LSB
//  UINT32 GPMC_PSA_MSB;              //offset 0x238,PSA-MSB

} OMAP_GPMC_REGS;


//------------------------------------------------------------------------------

#define GPMC_IRQSTATUS_FIFOEVENT        (1 << 0)
#define GPMC_IRQSTATUS_TERMINALCOUNT    (1 << 1)
#define GPMC_IRQSTATUS_WAIT0_EDGEDETECT (1 << 8)
#define GPMC_IRQSTATUS_WAIT1_EDGEDETECT (1 << 9)
#define GPMC_IRQSTATUS_WAIT2_EDGEDETECT (1 << 10)
#define GPMC_IRQSTATUS_WAIT3_EDGEDETECT (1 << 11)

#define GPMC_IRQENABLE_FIFOEVENT        (1 << 0)
#define GPMC_IRQENABLE_TERMINALCOUNT    (1 << 1)
#define GPMC_IRQENABLE_WAIT0_EDGEDETECT (1 << 8)
#define GPMC_IRQENABLE_WAIT1_EDGEDETECT (1 << 9)
#define GPMC_IRQENABLE_WAIT2_EDGEDETECT (1 << 10)
#define GPMC_IRQENABLE_WAIT3_EDGEDETECT (1 << 11)

#define GPMC_CONFIG_NANDFORCEPOSTEDWRITE (1 << 0)
#define GPMC_CONFIG_LIMITEDADDRESS      (1 << 1)
#define GPMC_CONFIG_WRITEPROTECT        (1 << 4)
#define GPMC_CONFIG_WAIT0PINPOLARITY    (1 << 8)
#define GPMC_CONFIG_WAIT1PINPOLARITY    (1 << 9)
#define GPMC_CONFIG_WAIT2PINPOLARITY    (1 << 10)
#define GPMC_CONFIG_WAIT3PINPOLARITY    (1 << 11)

#define GPMC_STATUS_EMPTYWRITEBUFFER    (1 << 0)
#define GPMC_STATUS_WAIT0_ASSERTED      (1 << 8)
#define GPMC_STATUS_WAIT1_ASSERTED      (1 << 9)
#define GPMC_STATUS_WAIT2_ASSERTED      (1 << 10)
#define GPMC_STATUS_WAIT3_ASSERTED      (1 << 11)

#define GPMC_ECC_CONFIG_ENABLE          (1 << 0)
#define GPMC_ECC_CONFIG_CS0             (0 << 1)
#define GPMC_ECC_CONFIG_CS1             (1 << 1)
#define GPMC_ECC_CONFIG_CS2             (2 << 1)
#define GPMC_ECC_CONFIG_CS3             (3 << 1)
#define GPMC_ECC_CONFIG_CS4             (4 << 1)
#define GPMC_ECC_CONFIG_CS5             (5 << 1)
#define GPMC_ECC_CONFIG_CS6             (6 << 1)
#define GPMC_ECC_CONFIG_CS7             (7 << 1)
#define GPMC_ECC_CONFIG_8BIT            (0 << 7)
#define GPMC_ECC_CONFIG_16BIT           (1 << 7)

#define GPMC_ECC_CONTROL_POINTER1       (1 << 0)
#define GPMC_ECC_CONTROL_POINTER2       (2 << 0)
#define GPMC_ECC_CONTROL_POINTER3       (3 << 0)
#define GPMC_ECC_CONTROL_POINTER4       (4 << 0)
#define GPMC_ECC_CONTROL_POINTER5       (5 << 0)
#define GPMC_ECC_CONTROL_POINTER6       (6 << 0)
#define GPMC_ECC_CONTROL_POINTER7       (7 << 0)
#define GPMC_ECC_CONTROL_POINTER8       (8 << 0)
#define GPMC_ECC_CONTROL_POINTER9       (9 << 0)
#define GPMC_ECC_CONTROL_CLEAR          (1 << 8)

#define GPMC_PREFETCH_CONFIG_WRITEPOST              (1 << 0)
#define GPMC_PREFETCH_CONFIG_DMAMODE                (1 << 2)
#define GPMC_PREFETCH_CONFIG_SYNCHROMODE            (1 << 3)
#define GPMC_PREFETCH_CONFIG_ENABLEENGINE           (1 << 7)
#define GPMC_PREFETCH_CONFIG_PFPWENROUNDROBIN       (1 << 23)
#define GPMC_PREFETCH_CONFIG_ENABLEOPTIMIZEDACCESS  (1 << 27)
#define GPMC_PREFETCH_CONFIG_WAITPINSELECTOR_SHIFT  (4)
#define GPMC_PREFETCH_CONFIG_WAITPINSELECTOR_MASK   (0x3 << GPMC_PREFETCH_CONFIG_WAITPINSELECTOR_SHIFT)
#define GPMC_PREFETCH_CONFIG_WAITPINSELECTOR(x)     ((x << GPMC_PREFETCH_CONFIG_WAITPINSELECTOR_SHIFT) & \
                                                        GPMC_PREFETCH_CONFIG_WAITPINSELECTOR_MASK)
#define GPMC_PREFETCH_CONFIG_FIFOTHRESHOLD_SHIFT    (8)
#define GPMC_PREFETCH_CONFIG_FIFOTHRESHOLD_MASK     (0x7F << GPMC_PREFETCH_CONFIG_FIFOTHRESHOLD_SHIFT)
#define GPMC_PREFETCH_CONFIG_FIFOTHRESHOLD(x)       ((x << GPMC_PREFETCH_CONFIG_FIFOTHRESHOLD_SHIFT) & \
                                                        GPMC_PREFETCH_CONFIG_FIFOTHRESHOLD_MASK)
#define GPMC_PREFETCH_CONFIG_PFPWEIGHTEDPRIO_SHIFT  (16)
#define GPMC_PREFETCH_CONFIG_PFPWEIGHTEDPRIO_MASK   (0xF << GPMC_PREFETCH_CONFIG_PFPWEIGHTEDPRIO_SHIFT)
#define GPMC_PREFETCH_CONFIG_PFPWEIGHTEDPRIO(x)     ((x << GPMC_PREFETCH_CONFIG_PFPWEIGHTEDPRIO_SHIFT) & \
                                                        GPMC_PREFETCH_CONFIG_PFPWEIGHTEDPRIO_MASK)
#define GPMC_PREFETCH_CONFIG_ENGINECSSELECTOR_SHIFT (24)
#define GPMC_PREFETCH_CONFIG_ENGINECSSELECTOR_MASK  (0x7 << GPMC_PREFETCH_CONFIG_ENGINECSSELECTOR_SHIFT)
#define GPMC_PREFETCH_CONFIG_ENGINECSSELECTOR(x)    ((x << GPMC_PREFETCH_CONFIG_ENGINECSSELECTOR_SHIFT) & \
                                                        GPMC_PREFETCH_CONFIG_ENGINECSSELECTOR_MASK)
#define GPMC_PREFETCH_CONFIG_CYCLEOPTIMIZATION_SHIFT (28)
#define GPMC_PREFETCH_CONFIG_CYCLEOPTIMIZATION_MASK (0x7 << GPMC_PREFETCH_CONFIG_CYCLEOPTIMIZATION_SHIFT)
#define GPMC_PREFETCH_CONFIG_CYCLEOPTIMIZATION(x)   ((x << GPMC_PREFETCH_CONFIG_CYCLEOPTIMIZATION_SHIFT) & \
                                                        GPMC_PREFETCH_CONFIG_CYCLEOPTIMIZATION_MASK)

#define GPMC_PREFETCH_STATUS_FIFOSHIFT  (24)
#define GPMC_PREFETCH_STATUS_FIFOMASK   (0x7F << GPMC_PREFETCH_STATUS_FIFOSHIFT)

#define GPMC_PREFETCH_CONTROL_STARTENGINE (1 << 0)

#define GPMC_CSVALID            (1<<6)
#endif

