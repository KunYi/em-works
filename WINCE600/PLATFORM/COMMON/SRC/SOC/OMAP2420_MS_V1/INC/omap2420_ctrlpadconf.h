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
//  File:  omap2420_ctrlpadconf.h
//
//  This header file is comprised of control pad configuration module register
//  details defined as structures and macros for configuring the pads.
//

#ifndef __OMAP2420_CTRLPADCONF_H
#define __OMAP2420_CTRLPADCONF_H

//------------------------------------------------------------------------------

// SYSC1
// Register CONTROL_PADCONF_x
// Base Address : OMAP2420_SYSC1_REGS_PA+0x30 (0x48000000+0x30)

typedef struct {
   volatile unsigned long ulPADCONF_SDRC_A14; //offset 0x30
   volatile unsigned long ulPADCONF_SDRC_BA0; //offset 0x34
   volatile unsigned long ulPADCONF_SDRC_A8;  //offset 0x38
   volatile unsigned long ulPADCONF_SDRC_A4;  //offset 0x3C
   volatile unsigned long ulPADCONF_SDRC_A0;  //offset 0x40
   unsigned long ulRESERVED0x44_0x4F[3];
   volatile unsigned long ulPADCONF_SDRC_STK_D16; //offset 0x50
   volatile unsigned long ulPADCONF_SDRC_D28; //offset 0x54
   volatile unsigned long ulPADCONF_SDRC_D24; //offset 0x58
   volatile unsigned long ulPADCONF_SDRC_D20; //offset 0x5C
   volatile unsigned long ulPADCONF_SDRC_D16; //offset 0x60
   volatile unsigned long ulPADCONF_SDRC_D12; //offset 0x64
   volatile unsigned long ulPADCONF_SDRC_D8;  //offset 0x68
   volatile unsigned long ulPADCONF_SDRC_D4;  //offset 0x6C
   volatile unsigned long ulPADCONF_SDRC_D0;  //offset 0x70
   volatile unsigned long ulPADCONF_GPMC_A7;  //offset 0x74
   volatile unsigned long ulPADCONF_GPMC_A3;  //offset 0x78
   volatile unsigned long ulPADCONF_GPMC_D14; //offset 0x7C
   volatile unsigned long ulPADCONF_GPMC_D10; //offset 0x80
   volatile unsigned long ulPADCONF_GPMC_D6;  //offset 0x84
   volatile unsigned long ulPADCONF_GPMC_D2;  //offset 0x88
   volatile unsigned long ulPADCONF_GPMC_NCS0;//offset 0x8C
   volatile unsigned long ulPADCONF_GPMC_NCS4;//offset 0x90
   volatile unsigned long ulPADCONF_GPMC_NADV_ALE;//offset 0x94
   volatile unsigned long ulPADCONF_GPMC_NBE1;//offset 0x98
   volatile unsigned long ulPADCONF_GPMC_WAIT2;//offset 0x9C
   volatile unsigned long ulPADCONF_SDRC_NCS0;//offset 0xA0
   volatile unsigned long ulPADCONF_SDRC_NRAS;//offset 0xA4
   volatile unsigned long ulPADCONF_SDRC_DM1; //offset 0xA8
   volatile unsigned long ulPADCONF_SDRC_STK_DM1;//offset 0xAC
   volatile unsigned long ulPADCONF_SDRC_DQS1;//offset 0xB0
   volatile unsigned long ulPADCONF_DSS_D1;   //offset 0xB4
   volatile unsigned long ulPADCONF_DSS_D5;   //offset 0xB8
   volatile unsigned long ulPADCONF_DSS_D9;   //offset 0xBC
   volatile unsigned long ulPADCONF_DSS_D13;  //offset 0xC0
   volatile unsigned long ulPADCONF_DSS_D17;  //offset 0xC4
   volatile unsigned long ulPADCONF_UART1_RX; //offset 0xC8
   volatile unsigned long ulPADCONF_DSS_VSYNC;//offset 0xCC
   volatile unsigned long ulPADCONF_CAM_D8;   //offset 0xD0
   volatile unsigned long ulPADCONF_CAM_D4;   //offset 0xD4
   volatile unsigned long ulPADCONF_CAM_D0;   //offset 0xD8
   volatile unsigned long ulPADCONF_CAM_XCLK; //offset 0xDC
   volatile unsigned long ulPADCONF_GPIO_62;  //offset 0xE0
   volatile unsigned long ulPADCONF_SSI1_WAKE;//offset 0xE4
   volatile unsigned long ulPADCONF_VLYNQ_TX1;//offset 0xE8
   volatile unsigned long ulPADCONF_UART2_RTS;//offset 0xEC
   volatile unsigned long ulPADCONF_EAC_BT_FS;//offset 0xF0
   volatile unsigned long ulPADCONF_MMC_CMD;  //offset 0xF4
   volatile unsigned long ulPADCONF_MMC_DAT3; //offset 0xF8
   volatile unsigned long ulPADCONF_MMC_DAT_DIR3;//offset 0xFC
   volatile unsigned long ulPADCONF_SPI1_SIMO;  //offset 0x100
   volatile unsigned long ulPADCONF_SPI1_NCS2;  //offset 0x104
   volatile unsigned long ulPADCONF_SPI2_SOMI;  //offset 0x108
   volatile unsigned long ulPADCONF_MCBSP1_DX;  //offset 0x10C
   volatile unsigned long ulPADCONF_MCBSP1_CLKX;//offset 0x110
   volatile unsigned long ulPADCONF_I2C2_SDA;   //offset 0x114
   volatile unsigned long ulPADCONF_UART3_TX_IRTX;//offset 0x118
   volatile unsigned long ulPADCONF_TV_RREF;    //offset 0x11C
   volatile unsigned long ulPADCONF_USB0_RCV;   //offset 0x120
   volatile unsigned long ulPADCONF_EAC_AC_SCLK;//offset 0x124
   volatile unsigned long ulPADCONF_EAC_AC_MCLK;//offset 0x128
   volatile unsigned long ulPADCONF_SYS_NIRQ;   //offset 0x12C
   volatile unsigned long ulPADCONF_GPIO_121;   //offset 0x130
   volatile unsigned long ulPADCONF_SYS_XTALOUT;//offset 0x134
   volatile unsigned long ulPADCONF_GPIO_6;     //offset 0x138
   volatile unsigned long ulPADCONF_JTAG_EMU0;  //offset 0x13C
   volatile unsigned long ulPADCONF_JTAG_TMS;   //offset 0x140
}OMAP2420_CONTROL_PADCONF_REGS;

//
// Register CONTROL_REVISION
// Base Address : OMAP2420_SYSC1_REGS_PA (0x48000000)

typedef struct 
{
   unsigned long ulCONTROL_REVISION;                //offset 0x0,module revision
   unsigned long ulRESERVED_1[3];
   volatile unsigned long ulCONTROL_SYSCONFIG;      //offset 0x10,idle mode for control module
   unsigned long ulRESERVED_2[7];
   OMAP2420_CONTROL_PADCONF_REGS ulCONTROL_PADCONF;     //offset 0x30-0x140.
   unsigned long ulRESERVED0x144_0x270[75];      
   volatile unsigned long ulCONTROL_DEBOBS;         //offset 0x270,debug and observability reg
   volatile unsigned long ulCONTROL_DEVCONF;        //offset 0x274,static device conf reg
   unsigned long ulRESERVED_3[5];
   volatile unsigned long ulCONTROL_EMU_SUPPORT;    //offset 0x28C,emulation support reg
   volatile unsigned long ulCONTROL_MSUSPENDMUX_0;  //offset 0x290,MSUSPEND MUX0 control reg
   volatile unsigned long ulCONTROL_MSUSPENDMUX_1;  //offset 0x294,MSUSPEND MUX1 control reg
   volatile unsigned long ulCONTROL_MSUSPENDMUX_2;  //offset 0x298,MSUSPEND MUX2 control reg
   volatile unsigned long ulCONTROL_MSUSPENDMUX_3;  //offset 0x29C,MSUSPEND MUX3 control reg
   volatile unsigned long ulCONTROL_MSUSPENDMUX_4;  //offset 0x2A0,MSUSPEND MUX4 control reg
   volatile unsigned long ulCONTROL_MSUSPENDMUX_5;  //offset 0x2A4,MSUSPEND MUX5 control reg
   volatile unsigned long ulCONTROL_RESERVED;       //offset 0x2A8,    ???
   unsigned long ulRESERVED_0x2AC;
   volatile unsigned long ulCONTROL_SEC_CTRL;    //offset 0x2B0, security control reg
   volatile unsigned long ulCONTROL_SEC_TEST;    //offset 0x2B4, security test control reg
   volatile unsigned long ulCONTROL_PSA_CTRL;    //offset 0x2B8, MPU PSA operation control reg
   volatile unsigned long ulCONTROL_PSA_CMD;        //offset 0x2BC, PSA command reg
   volatile unsigned long ulCONTROL_PSA_VALUE;      //offset 0x2C0, PSA control value reg
   unsigned long ulRESERVED_4[3];
   volatile unsigned long ulCONTROL_SEC_EMU;        //offset 0x2D0, security emulation reg
   volatile unsigned long ulCONTROL_SEC_TAP;        //offset 0x2D4, security tap controller reg
   volatile unsigned long ulCONTROL_OCM_RAM_PERM;   //offset 0x2D8, security conf reg:
   volatile unsigned long ulCONTROL_OCM_PUB_RAM_ADD;//offset 0x2DC, security conf reg:OCM Public RAM addr
   volatile unsigned long ulCONTROL_EXT_SEC_RAM_START_ADD;//offset 0x2E0, security conf reg:Extended secure RAM start addr
   volatile unsigned long ulCONTROL_EXT_SEC_RAM_STOP_ADD; //offset 0x2E4, security conf reg:Extended secure RAM stop addr
   unsigned long ulRESERVED_5[2];
   volatile unsigned long ulCONTROL_SEC_STATUS;           //offset 0x2F0,security status reg
   volatile unsigned long ulCONTROL_SEC_ERR_STATUS;       //offset 0x2F4,security error status reg
   volatile unsigned long ulCONTROL_STATUS;               //offset 0x2F8,control module status reg
   volatile unsigned long ulCONTROL_GENERAL_PURPOSE_STATUS;//offset 0x2FC,status bits reflecting proc internal states
   volatile unsigned long ulCONTROL_RPUB_KEY_H_0;         //offset 0x300,Root_public_key_hash[31-0]
   volatile unsigned long ulCONTROL_RPUB_KEY_H_1;         //offset 0x304,Root_public_key_hash[63-32]
   volatile unsigned long ulCONTROL_RPUB_KEY_H_2;         //offset 0x308,Root_public_key_hash[95-64]
   volatile unsigned long ulCONTROL_RPUB_KEY_H_3;         //offset 0x30C,Root_public_key_hash[127-96]
   volatile unsigned long ulCONTROL_RAND_KEY_0;           //offset 0x310,Random_Key[31-0]Fuse Keys[159-128]
   volatile unsigned long ulCONTROL_RAND_KEY_1;           //offset 0x314,Random_Key[63-32]Fuse Keys[191-160]
   volatile unsigned long ulCONTROL_RAND_KEY_2;           //offset 0x318,Random_Key[95-64]Fuse Keys[223-192]
   volatile unsigned long ulCONTROL_RAND_KEY_3;           //offset 0x31C,Random_Key[127-96]Fuse Keys[255-224]
   volatile unsigned long ulCONTROL_CUST_KEY_0;           //offset 0x320,Customer_Key[31-0]Fuse Keys[287-256]
   volatile unsigned long ulCONTROL_CUST_KEY_1;           //offset 0x324,Customer_Key[63-32]Fuse Keys[319-288]
   unsigned long ulRESERVED_6[2];
   volatile unsigned long ulCONTROL_TEST_KEY_0;           //offset 0x330,Test_Key[31-0]
   volatile unsigned long ulCONTROL_TEST_KEY_1;           //offset 0x334,Test_Key[63-32]
   volatile unsigned long ulCONTROL_TEST_KEY_2;           //offset 0x338,Test_Key[95-64]
   volatile unsigned long ulCONTROL_TEST_KEY_3;           //offset 0x33C,Test_Key[127-96]
   volatile unsigned long ulCONTROL_TEST_KEY_4;           //offset 0x340,Test_Key[159-128]
   volatile unsigned long ulCONTROL_TEST_KEY_5;           //offset 0x344,Test_Key[191-160]
   volatile unsigned long ulCONTROL_TEST_KEY_6;           //offset 0x348,Test_Key[223-192]
   volatile unsigned long ulCONTROL_TEST_KEY_7;           //offset 0x34C,Test_Key[255-224]
   volatile unsigned long ulCONTROL_TEST_KEY_8;           //offset 0x350,Test_Key[287-256]
   volatile unsigned long ulCONTROL_TEST_KEY_9;           //offset 0x354,Test_Key[319-288]
} OMAP2420_SYSC1_REGS;

#endif
