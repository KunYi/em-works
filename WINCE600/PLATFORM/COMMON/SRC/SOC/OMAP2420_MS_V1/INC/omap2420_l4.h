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
//  File:  omap2420_L4.h
//
//  This header file is comprised of L4 module register details defined as 
//  structures and macros for configuring L4 module.

#ifndef __OMAP2420_L4_H
#define __OMAP2420_L4_H

//------------------------------------------------------------------------------

//
// L4TA Registers
//

// Base Address     : 0x68002400

typedef struct __L4TAREGS__
{
   unsigned long ulRESERVED_1[54];
   volatile unsigned long ulSBTMPORTCONNID0; //offset 0xD8,**RESERVED**
   unsigned long ulRESERVED_2[7];
   volatile unsigned long ulSBTMPORTLOCK0;   //offset 0xF8,TM port lock status
   unsigned long ulRESERVED_3[19];
   volatile unsigned long ulSBTMERRLOGA;     //offset 0x148,logs info abt TM error conditions
   unsigned long ulRESERVED_0x14C;
   volatile unsigned long ulSBTMERRLOG;      //offset 0x150,logs info abt TM error conditions
   unsigned long ulRESERVED_4[17];
   volatile unsigned long ulSBTMSTATE_L;     //offset 0x198,status info & target agent ctrl -LSBs
   volatile unsigned long ulSBTMSTATE_H;     //offset 0x19C,status info & target agent ctrl -MSBs
   unsigned long ulRESERVED_5[7];   
   volatile unsigned long ulSBTMCONFIG;      //offset 0x1BC,clk generator conf & target agent response flg usage
   unsigned long ulRESERVED_6[14];
   volatile unsigned long ulSBID;            //offset 0x1F8,identification
}
OMAP2420_L4TA_REGS, *pL4TAREGS;

//
// TM Registers
//
// Module    Base         Port
// Name      Address
// TM4       0x68002E00   DSP Memory
// TM7       0x68003600   IVA T
// TM5       0x68003B00   GFX Subsystem
// TM6       0x68003D00   Command Write
// TM1       0x68004100   SB2SMS
// TM3       0x68004300   SB2OCM
// TM2       0x68004500   SB2GPMC

typedef struct __TMREGS__
{
   unsigned long ulRESERVED_1[81];
   volatile unsigned long ulSBTMERRLOGA;       //offset 0x148, logs info abt TM err conditions
   unsigned long ulRESERVED_0x14C;
   volatile unsigned long ulSBTMERRLOG;        //offset 0x150, logs info abt TM err conditions
   unsigned long ulRESERVED_2[17];
   volatile unsigned long ulSBTMSTATE_L;       //offset 0x198, status info & target agent ctrl -LSBs
   volatile unsigned long ulSBTMSTATE_H;       //offset 0x19C, status info & target agent ctrl -MSBs
   unsigned long ulRESERVED_3[7];
   volatile unsigned long ulSBTMCONFIG;        //offset 0xBC,clk generator conf & target agent response flg usage
   unsigned long ulRESERVED_4[14];
   volatile unsigned long ulSBID;              //offset 0xF8,Lower 32 bits indentification
}
OMAP2420_TM_REGS, *pTMREGS;

//////////////////////////////////////////////////////////////////////
// L4 Interconnect
//////////////////////////////////////////////////////////////////////


// 
// L4TAO
// 

// structure common to L4TAO1-L4TAO14 registers.
typedef struct __L4TAOREGS__
{
   volatile unsigned long ulCOMPONENT; //offset 0x0,logs info abt revision code & interconnect code
   unsigned long ulRESERVED_1[7];
   volatile unsigned long ulAGENT_CONTROL_L;//offset 0x20,agent control register bit allocation
   volatile unsigned long ulAGENT_CONTROL_H;//offset 0x24,status bit for pwr mgmt of TA
   volatile unsigned long ulAGENT_STATUS;//offset 0x28,records req_time-out and S error fields-LSBs
}
OMAP2420_L4TAO_REGS, *pL4TAOREGS;

//////////////////////////////////////////////////////////////////////
// L4 Address and Protection (L4AP)
/////////////////////////////////////////////////////////////////////

// Base Address : 0x48040000

typedef struct __L4APREGS__
{
   volatile unsigned long ulCOMPONENT;   //offset 0x00,
   unsigned long ulRESERVED_1[63];         //04-0x100
   volatile unsigned long ulSEGMENT_L_0; //offset 0x100,
   volatile unsigned long ulSEGMENT_H_0; //offset 0x104,
   volatile unsigned long ulSEGMENT_L_1; //offset 0x108,
   volatile unsigned long ulSEGMENT_H_1; //offset 0x10C,
   volatile unsigned long ulSEGMENT_L_2; //offset 0x110,
   volatile unsigned long ulSEGMENT_H_2; //offset 0x114,
   volatile unsigned long ulSEGMENT_L_3; //offset 0x118,
   volatile unsigned long ulSEGMENT_H_3; //offset 0x11C,
   volatile unsigned long ulSEGMENT_L_4; //offset 0x120,
   volatile unsigned long ulSEGMENT_H_4; //offset 0x124,
   volatile unsigned long ulSEGMENT_L_5; //offset 0x128,
   volatile unsigned long ulSEGMENT_H_5; //offset 0x12C,
   unsigned long ulRESERVED_2[52];
   volatile unsigned long ulPROTGROUP_0; //offset 0x200,
   volatile unsigned long ulPROTGROUP_0_H; //offset 0x204,
   volatile unsigned long ulPROTGROUP_1; //offset 0x208,
   volatile unsigned long ulPROTGROUP_2; //offset 0x210,
   volatile unsigned long ulPROTGROUP_3; //offset 0x218,
   volatile unsigned long ulPROTGROUP_4; //offset 0x220,
   volatile unsigned long ulPROTGROUP_5; //offset 0x228,
   volatile unsigned long ulPROTGROUP_6; //offset 0x230,
   volatile unsigned long ulPROTGROUP_7; //offset 0x238,
   unsigned long ulRESERVED_3[49];         //23C-300
   volatile unsigned long ulREGION_0_L;  //offset 0x300,
   volatile unsigned long ulREGION_0_H;  //offset 0x304,
   volatile unsigned long ulREGION_1_L;  //offset 0x308,
   volatile unsigned long ulREGION_1_H;  //offset 0x30C,
   volatile unsigned long ulREGION_2_L;  //offset 0x310,
   volatile unsigned long ulREGION_2_H;  //offset 0x314,
   volatile unsigned long ulREGION_3_L;  //offset 0x318,
   volatile unsigned long ulREGION_3_H;  //offset 0x31C,
   volatile unsigned long ulREGION_4_L;  //offset 0x320,
   volatile unsigned long ulREGION_4_H;  //offset 0x324,
   volatile unsigned long ulREGION_5_L;  //offset 0x328,
   volatile unsigned long ulREGION_5_H;  //offset 0x32C,
   volatile unsigned long ulREGION_6_L;  //offset 0x330,
   volatile unsigned long ulREGION_6_H;  //offset 0x334,
   volatile unsigned long ulREGION_7_L;  //offset 0x338,
   volatile unsigned long ulREGION_7_H;  //offset 0x33C,
   volatile unsigned long ulREGION_8_L;  //offset 0x340,
   volatile unsigned long ulREGION_8_H;  //offset 0x344,
   volatile unsigned long ulREGION_9_L;  //offset 0x348,
   volatile unsigned long ulREGION_9_H;  //offset 0x34C,
   volatile unsigned long ulREGION_10_L;  //offset 0x350,
   volatile unsigned long ulREGION_10_H;  //offset 0x354,
   volatile unsigned long ulREGION_11_L;  //offset 0x358,
   volatile unsigned long ulREGION_11_H;  //offset 0x35C,
   volatile unsigned long ulREGION_12_L;  //offset 0x360,
   volatile unsigned long ulREGION_12_H;  //offset 0x364,
   volatile unsigned long ulREGION_13_L;  //offset 0x368,
   volatile unsigned long ulREGION_13_H;  //offset 0x36C,
   volatile unsigned long ulREGION_14_L;  //offset 0x370,
   volatile unsigned long ulREGION_14_H;  //offset 0x374,
   volatile unsigned long ulREGION_15_L;  //offset 0x378,
   volatile unsigned long ulREGION_15_H;  //offset 0x37C,
   volatile unsigned long ulREGION_16_L;  //offset 0x380,
   volatile unsigned long ulREGION_16_H;  //offset 0x384,
   volatile unsigned long ulREGION_17_L;  //offset 0x388,
   volatile unsigned long ulREGION_17_H;  //offset 0x38C,
   volatile unsigned long ulREGION_18_L;  //offset 0x390,
   volatile unsigned long ulREGION_18_H;  //offset 0x394,
   volatile unsigned long ulREGION_19_L;  //offset 0x398,
   volatile unsigned long ulREGION_19_H;  //offset 0x39C,
   volatile unsigned long ulREGION_20_L;  //offset 0x3A0,
   volatile unsigned long ulREGION_20_H;  //offset 0x3A4,
   volatile unsigned long ulREGION_21_L;  //offset 0x3A8,
   volatile unsigned long ulREGION_21_H;  //offset 0x3AC,
   volatile unsigned long ulREGION_22_L;  //offset 0x3B0,
   volatile unsigned long ulREGION_22_H;  //offset 0x3B4,
   volatile unsigned long ulREGION_23_L;  //offset 0x3B8,
   volatile unsigned long ulREGION_23_H;  //offset 0x3BC,
   volatile unsigned long ulREGION_24_L;  //offset 0x3C0,
   volatile unsigned long ulREGION_24_H;  //offset 0x3C4,
   volatile unsigned long ulREGION_25_L;  //offset 0x3C8,
   volatile unsigned long ulREGION_25_H;  //offset 0x3CC,
   volatile unsigned long ulREGION_26_L;  //offset 0x3D0,
   volatile unsigned long ulREGION_26_H;  //offset 0x3D4,
   volatile unsigned long ulREGION_27_L;  //offset 0x3D8,
   volatile unsigned long ulREGION_27_H;  //offset 0x3DC,
   volatile unsigned long ulREGION_28_L;  //offset 0x3E0,
   volatile unsigned long ulREGION_28_H;  //offset 0x3E4,
   volatile unsigned long ulREGION_29_L;  //offset 0x3E8,
   volatile unsigned long ulREGION_29_H;  //offset 0x3EC,
   volatile unsigned long ulREGION_30_L;  //offset 0x3F0,
   volatile unsigned long ulREGION_30_H;  //offset 0x3F4,
   volatile unsigned long ulREGION_31_L;  //offset 0x3F8,
   volatile unsigned long ulREGION_31_H;  //offset 0x3FC,
   volatile unsigned long ulREGION_32_L;  //offset 0x400,
   volatile unsigned long ulREGION_32_H;  //offset 0x404,
   volatile unsigned long ulREGION_33_L;  //offset 0x408,
   volatile unsigned long ulREGION_33_H;  //offset 0x40C,
   volatile unsigned long ulREGION_34_L;  //offset 0x410,
   volatile unsigned long ulREGION_34_H;  //offset 0x414,
   volatile unsigned long ulREGION_35_L;  //offset 0x418,
   volatile unsigned long ulREGION_35_H;  //offset 0x41C,
   volatile unsigned long ulREGION_36_L;  //offset 0x420,
   volatile unsigned long ulREGION_36_H;  //offset 0x424,
   volatile unsigned long ulREGION_37_L;  //offset 0x428,
   volatile unsigned long ulREGION_37_H;  //offset 0x42C,
   volatile unsigned long ulREGION_38_L;  //offset 0x430,
   volatile unsigned long ulREGION_38_H;  //offset 0x434,
   volatile unsigned long ulREGION_39_L;  //offset 0x438,
   volatile unsigned long ulREGION_39_H;  //offset 0x43C,
   volatile unsigned long ulREGION_40_L;  //offset 0x440,
   volatile unsigned long ulREGION_40_H;  //offset 0x444,
   volatile unsigned long ulREGION_41_L;  //offset 0x448,
   volatile unsigned long ulREGION_41_H;  //offset 0x44C,
   volatile unsigned long ulREGION_42_L;  //offset 0x450,
   volatile unsigned long ulREGION_42_H;  //offset 0x454,
   volatile unsigned long ulREGION_43_L;  //offset 0x458,
   volatile unsigned long ulREGION_43_H;  //offset 0x45C,
   volatile unsigned long ulREGION_44_L;  //offset 0x460,
   volatile unsigned long ulREGION_44_H;  //offset 0x464,
   volatile unsigned long ulREGION_45_L;  //offset 0x468,
   volatile unsigned long ulREGION_45_H;  //offset 0x46C,
   volatile unsigned long ulREGION_46_L;  //offset 0x470,
   volatile unsigned long ulREGION_46_H;  //offset 0x474,
   volatile unsigned long ulREGION_47_L;  //offset 0x478,
   volatile unsigned long ulREGION_47_H;  //offset 0x47C,
   volatile unsigned long ulREGION_48_L;  //offset 0x480,
   volatile unsigned long ulREGION_48_H;  //offset 0x484,
   volatile unsigned long ulREGION_49_L;  //offset 0x488,
   volatile unsigned long ulREGION_49_H;  //offset 0x48C,
   volatile unsigned long ulREGION_50_L;  //offset 0x490,
   volatile unsigned long ulREGION_50_H;  //offset 0x494,
   volatile unsigned long ulREGION_51_L;  //offset 0x498,
   volatile unsigned long ulREGION_51_H;  //offset 0x49C,
   volatile unsigned long ulREGION_52_L;  //offset 0x4A0,
   volatile unsigned long ulREGION_52_H;  //offset 0x4A4,
   volatile unsigned long ulREGION_53_L;  //offset 0x4A8,
   volatile unsigned long ulREGION_53_H;  //offset 0x4AC,
   volatile unsigned long ulREGION_54_L;  //offset 0x4B0,
   volatile unsigned long ulREGION_54_H;  //offset 0x4B4,
   volatile unsigned long ulREGION_55_L;  //offset 0x4B8,
   volatile unsigned long ulREGION_55_H;  //offset 0x4BC,
   volatile unsigned long ulREGION_56_L;  //offset 0x4C0,
   volatile unsigned long ulREGION_56_H;  //offset 0x4C4,
   volatile unsigned long ulREGION_57_L;  //offset 0x4C8,
   volatile unsigned long ulREGION_57_H;  //offset 0x4CC,
   volatile unsigned long ulREGION_58_L;  //offset 0x4D0,
   volatile unsigned long ulREGION_58_H;  //offset 0x4D4,
   volatile unsigned long ulREGION_59_L;  //offset 0x4D8,
   volatile unsigned long ulREGION_59_H;  //offset 0x4DC,
   volatile unsigned long ulREGION_60_L;  //offset 0x4E0,
   volatile unsigned long ulREGION_60_H;  //offset 0x4E4,
   volatile unsigned long ulREGION_61_L;  //offset 0x4E8,
   volatile unsigned long ulREGION_61_H;  //offset 0x4EC,
   volatile unsigned long ulREGION_62_L;  //offset 0x4F0,
   volatile unsigned long ulREGION_62_H;  //offset 0x4F4,
   volatile unsigned long ulREGION_63_L;  //offset 0x4F8,
   volatile unsigned long ulREGION_63_H;  //offset 0x4FC,
   volatile unsigned long ulREGION_64_L;  //offset 0x500,
   volatile unsigned long ulREGION_64_H;  //offset 0x50C,
   volatile unsigned long ulREGION_65_L;  //offset 0x510,
   volatile unsigned long ulREGION_65_H;  //offset 0x514,
   volatile unsigned long ulREGION_66_L;  //offset 0x518,
   volatile unsigned long ulREGION_66_H;  //offset 0x51C,
   volatile unsigned long ulREGION_67_L;  //offset 0x520,
   volatile unsigned long ulREGION_67_H;  //offset 0x304,
   volatile unsigned long ulREGION_68_L;  //offset 0x300,
   volatile unsigned long ulREGION_68_H;  //offset 0x304,
   volatile unsigned long ulREGION_69_L;  //offset 0x300,
   volatile unsigned long ulREGION_69_H;  //offset 0x304,
   volatile unsigned long ulREGION_70_L;  //offset 0x300,
   volatile unsigned long ulREGION_70_H;  //offset 0x304,
   volatile unsigned long ulREGION_71_L;  //offset 0x300,
   volatile unsigned long ulREGION_71_H;  //offset 0x304,
   volatile unsigned long ulREGION_72_L;  //offset 0x308,
   volatile unsigned long ulREGION_72_H;  //offset 0x30C,
   volatile unsigned long ulREGION_73_L;  //offset 0x300,
   volatile unsigned long ulREGION_73_H;  //offset 0x304,
   volatile unsigned long ulREGION_74_L;  //offset 0x300,
   volatile unsigned long ulREGION_74_H;  //offset 0x304,
   volatile unsigned long ulREGION_75_L;  //offset 0x300,
   volatile unsigned long ulREGION_75_H;  //offset 0x304,
   volatile unsigned long ulREGION_76_L;  //offset 0x300,
   volatile unsigned long ulREGION_76_H;  //offset 0x304,
   volatile unsigned long ulREGION_77_L;  //offset 0x300,
   volatile unsigned long ulREGION_77_H;  //offset 0x304,
   volatile unsigned long ulREGION_78_L;  //offset 0x300,
   volatile unsigned long ulREGION_78_H;  //offset 0x304,
   volatile unsigned long ulREGION_79_L;  //offset 0x300,
   volatile unsigned long ulREGION_79_H;  //offset 0x304,
   volatile unsigned long ulREGION_80_L;  //offset 0x300,
   volatile unsigned long ulREGION_80_H;  //offset 0x304,
   volatile unsigned long ulREGION_81_L;  //offset 0x300,
   volatile unsigned long ulREGION_81_H;  //offset 0x304,
   volatile unsigned long ulREGION_82_L;  //offset 0x300,
   volatile unsigned long ulREGION_82_H;  //offset 0x304,
   volatile unsigned long ulREGION_83_L;  //offset 0x300,
   volatile unsigned long ulREGION_83_H;  //offset 0x304,
   volatile unsigned long ulREGION_84_L;  //offset 0x300,
   volatile unsigned long ulREGION_84_H;  //offset 0x304,
   volatile unsigned long ulREGION_85_L;  //offset 0x300,
   volatile unsigned long ulREGION_85_H;  //offset 0x304,
   volatile unsigned long ulREGION_86_L;  //offset 0x300,
   volatile unsigned long ulREGION_86_H;  //offset 0x304,
   volatile unsigned long ulREGION_87_L;  //offset 0x300,
   volatile unsigned long ulREGION_87_H;  //offset 0x304,
   volatile unsigned long ulREGION_88_L;  //offset 0x300,
   volatile unsigned long ulREGION_88_H;  //offset 0x304,
   volatile unsigned long ulREGION_89_L;  //offset 0x300,
   volatile unsigned long ulREGION_89_H;  //offset 0x304,
   volatile unsigned long ulREGION_90_L;  //offset 0x300,
   volatile unsigned long ulREGION_90_H;  //offset 0x304,
   volatile unsigned long ulREGION_91_L;  //offset 0x300,
   volatile unsigned long ulREGION_91_H;  //offset 0x304,
   volatile unsigned long ulREGION_92_L;  //offset 0x300,
   volatile unsigned long ulREGION_92_H;  //offset 0x304,
   volatile unsigned long ulREGION_93_L;  //offset 0x300,
   volatile unsigned long ulREGION_93_H;  //offset 0x304,
   volatile unsigned long ulREGION_94_L;  //offset 0x300,
   volatile unsigned long ulREGION_94_H;  //offset 0x304,
   volatile unsigned long ulREGION_95_L;  //offset 0x300,
   volatile unsigned long ulREGION_95_H;  //offset 0x304,
   volatile unsigned long ulREGION_96_L;  //offset 0x300,
   volatile unsigned long ulREGION_96_H;  //offset 0x304,
   volatile unsigned long ulREGION_97_L;  //offset 0x300,
   volatile unsigned long ulREGION_97_H;  //offset 0x304,
   volatile unsigned long ulREGION_98_L;  //offset 0x300,
   volatile unsigned long ulREGION_98_H;  //offset 0x304,
   volatile unsigned long ulREGION_99_L;  //offset 0x300,
   volatile unsigned long ulREGION_99_H;  //offset 0x304,
   volatile unsigned long ulREGION_100_L;  //offset 0x300,
   volatile unsigned long ulREGION_100_H;  //offset 0x304,
   volatile unsigned long ulREGION_101_L;  //offset 0x300,
   volatile unsigned long ulREGION_101_H;  //offset 0x304,
   volatile unsigned long ulREGION_102_L;  //offset 0x300,
   volatile unsigned long ulREGION_102_H;  //offset 0x304,
   volatile unsigned long ulREGION_103_L;  //offset 0x300,
   volatile unsigned long ulREGION_103_H;  //offset 0x304,
   volatile unsigned long ulREGION_104_L;  //offset 0x300,
   volatile unsigned long ulREGION_104_H;  //offset 0x304,
   volatile unsigned long ulREGION_105_L;  //offset 0x300,
   volatile unsigned long ulREGION_105_H;  //offset 0x304,
   volatile unsigned long ulREGION_106_L;  //offset 0x300,
   volatile unsigned long ulREGION_106_H;  //offset 0x304,
   volatile unsigned long ulREGION_107_L;  //offset 0x308,
   volatile unsigned long ulREGION_107_H;  //offset 0x30C,
   volatile unsigned long ulREGION_108_L;  //offset 0x300,
   volatile unsigned long ulREGION_108_H;  //offset 0x304,
   volatile unsigned long ulREGION_109_L;  //offset 0x300,
   volatile unsigned long ulREGION_109_H;  //offset 0x304,
   volatile unsigned long ulREGION_110_L;  //offset 0x300,
   volatile unsigned long ulREGION_110_H;  //offset 0x304,
   volatile unsigned long ulREGION_111_L;  //offset 0x300,
   volatile unsigned long ulREGION_111_H;  //offset 0x304,
   volatile unsigned long ulREGION_112_L;  //offset 0x300,
   volatile unsigned long ulREGION_112_H;  //offset 0x304,
   volatile unsigned long ulREGION_113_L;  //offset 0x300,
   volatile unsigned long ulREGION_113_H;  //offset 0x304,
   volatile unsigned long ulREGION_114_L;  //offset 0x300,
   volatile unsigned long ulREGION_114_H;  //offset 0x304,
   volatile unsigned long ulREGION_115_L;  //offset 0x300,
   volatile unsigned long ulREGION_115_H;  //offset 0x304,
   volatile unsigned long ulREGION_116_L;  //offset 0x300,
   volatile unsigned long ulREGION_116_H;  //offset 0x304,
   volatile unsigned long ulREGION_117_L;  //offset 0x300,
   volatile unsigned long ulREGION_117_H;  //offset 0x304,
   volatile unsigned long ulREGION_118_L;  //offset 0x300,
   volatile unsigned long ulREGION_118_H;  //offset 0x304,
   volatile unsigned long ulREGION_119_L;  //offset 0x300,
   volatile unsigned long ulREGION_119_H;  //offset 0x304,
   volatile unsigned long ulREGION_120_L;  //offset 0x300,
   volatile unsigned long ulREGION_120_H;  //offset 0x304,
   volatile unsigned long ulREGION_121_L;  //offset 0x300,
   volatile unsigned long ulREGION_121_H;  //offset 0x304,
   volatile unsigned long ulREGION_122_L;  //offset 0x300,
   volatile unsigned long ulREGION_122_H;  //offset 0x304,
   volatile unsigned long ulREGION_123_L;  //offset 0x300,
   volatile unsigned long ulREGION_123_H;  //offset 0x304,
   volatile unsigned long ulREGION_124_L;  //offset 0x300,
   volatile unsigned long ulREGION_124_H;  //offset 0x304,
}
OMAP2420_L4AP_REGS, *pL4APREGS;   

//
// L4IA Registers
//

typedef struct __L4IAREGS__
{
   volatile unsigned long ulCOMPONENT;      //offset 00,logs info abt revision code & interconnect code 
   unsigned long ulRESERVED_1[7];
   volatile unsigned long ulAGENT_CONTROL;  //offset 0x20,L4 IA agent control 
   unsigned long ulRESERVED_0x24;
   volatile unsigned long ulAGENT_STATUS;   //offset 0x28,agent status
   unsigned long ulRESERVED_2[11];
   volatile unsigned long ulERROR_LOG;       //offset 0x58,Error log
}
OMAP2420_L4IA_REGS, *pL4IAREGS;


//
// L4LA Registers
//

typedef struct __L4LAREGS__
{
   volatile unsigned long ulCOMPONENT;   //offset 0x0, intercnt code & rev code
   unsigned long ulRESERVED_1[4];
   volatile unsigned long ulNETWORK;     //offset 0x14,vendor code information
   volatile unsigned long ulINITIATOR_INFO_L; //offset 0x18,req info perm config for region0-LSBs
   volatile unsigned long ulINITIATOR_INFO_H; //offset 0x1C,read perm config for region0-MSBs
   volatile unsigned long ulNET_CONTROL_L;  //offset 0x20, network control information-LSBs
   volatile unsigned long ulNET_CONTROL_H;  //offset 0x24, network control information-MSBs
   unsigned long ulRESERVED_2[54];
   volatile unsigned long ulFLAG_MASK_L; //offset 0x100, mask LSBs
   volatile unsigned long ulFLAG_MASK_H; //offset 0x104, mask MSBs
   unsigned long ulRESERVED_3[2];
   volatile unsigned long ulFLAG_STATUS_L; //offset 0x110, status LSBs  
   volatile unsigned long ulFLAG_STATUS_H; //offset 0x114, status MSBs
}
OMAP2420_L4LA_REGS, *pL4LAREGS;

//
// Base Address macro definition should be included in this header file.
//

// L4TA base address
#define OMAP2420_L4TA_REGS_PA         0x68002400

// TM base addresses
#define OMAP2420_TM4_REGS_PA          0x68002E00  // DSP memory
#define OMAP2420_TM7_REGS_PA          0x68003600  // IVA T
#define OMAP2420_TM5_REGS_PA          0x68003B00  // GFX Subsystem
#define OMAP2420_TM6_REGS_PA          0x68003D00  // Command write
#define OMAP2420_TM1_REGS_PA          0x68004100  // SB2SMS
#define OMAP2420_TM3_REGS_PA          0x68004300  // SB2OCM
#define OMAP2420_TM2_REGS_PA          0x68004500  // SB2GPMC

// L4TAO base addresses
#define OMAP2420_L4TAO1_REGS_PA       0x48001000  // System control and pinout
#define OMAP2420_L4TAO2_REGS_PA       0x48005000  // 32K timer
#define OMAP2420_L4TAO3_REGS_PA       0x48009000  // PRCM
#define OMAP2420_L4TA1_REGS_PA        0x48013000  // Test BCM
#define OMAP2420_L4TA2_REGS_PA        0x48015000  // Test JTAG
#define OMAP2420_L4TA3_REGS_PA        0x4801B000  // Quad GPIO
#define OMAP2420_L4TA4_REGS_PA        0x48023000  // Dual WD timer (1/2)
#define OMAP2420_L4TA5_REGS_PA        0x48025000  // WD timer 3 (DSP)
#define OMAP2420_L4TA6_REGS_PA        0x48027000  // WD timer 4 (IVA)
#define OMAP2420_L4TA7_REGS_PA        0x48029000  // GP timer1
#define OMAP2420_L4TA8_REGS_PA        0x4802B000  // GP timer 2
#define OMAP2420_L4AP_REGS_PA         0x48040000  // Address and protection
#define OMAP2420_L4IA_REGS_PA         0x48040800  // Initiator agent
#define OMAP2420_L4LA_REGS_PA         0x48041000  // Link agent
#define OMAP2420_L4TA9_REGS_PA        0x4804A000  // MPU ETB Emulation/Test
#define OMAP2420_L4TA10_REGS_PA       0x48051000  // DSS
#define OMAP2420_L4TA11_REGS_PA       0x48053000  // Camera
#define OMAP2420_L4TA12_REGS_PA       0x48057000  // sDMA
#define OMAP2420_L4TA13_REGS_PA       0x4805C000  // SSI
#define OMAP2420_L4TAO4_REGS_PA       0x4805F000  // USB OTG
#define OMAP2420_L4TA14_REGS_PA       0x48061000  // Win Tracer Test
#define OMAP2420_L4TA15_REGS_PA       0x48063000  // Win Tracer Test
#define OMAP2420_L4TA16_REGS_PA       0x48065000  // Win Tracer Test
#define OMAP2420_L4TA17_REGS_PA       0x48067000  // Win Tracer Test
#define OMAP2420_L4TA18_REGS_PA       0x48069000  // XTI Test
#define OMAP2420_L4TA19_REGS_PA       0x4806B000  // UART1
#define OMAP2420_L4TA20_REGS_PA       0x4806D000  // UART2
#define OMAP2420_L4TA21_REGS_PA       0x4806F000  // UART3
#define OMAP2420_L4TAO5_REGS_PA       0x48071000  // I2C1
#define OMAP2420_L4TAO6_REGS_PA       0x48073000  // I2C2
#define OMAP2420_L4TAO7_REGS_PA       0x48075000  // McBSP1
#define OMAP2420_L4TAO8_REGS_PA       0x48077000  // McBSP2
#define OMAP2420_L4TA22_REGS_PA       0x48079000  // GP Timer3
#define OMAP2420_L4TA23_REGS_PA       0x4807B000  // GP Timer4
#define OMAP2420_L4TA24_REGS_PA       0x4807D000  // GP Timer5
#define OMAP2420_L4TA25_REGS_PA       0x4807F000  // GP Timer6
#define OMAP2420_L4TA26_REGS_PA       0x48081000  // GP Timer7
#define OMAP2420_L4TA27_REGS_PA       0x48083000  // GP Timer8
#define OMAP2420_L4TA28_REGS_PA       0x48085000  // GP Timer9
#define OMAP2420_L4TA29_REGS_PA       0x48087000  // GP Timer10
#define OMAP2420_L4TA30_REGS_PA       0x48089000  // GP Timer11
#define OMAP2420_L4TA31_REGS_PA       0x4808B000  // GP Timer12
#define OMAP2420_L4TA32_REGS_PA       0x48091000  // EAC
#define OMAP2420_L4TA33_REGS_PA       0x48093000  // FAC
#define OMAP2420_L4TA34_REGS_PA       0x48095000  // IPC
#define OMAP2420_L4TA35_REGS_PA       0x48099000  // SPI1
#define OMAP2420_L4TA36_REGS_PA       0x4809B000  // SPI2
#define OMAP2420_L4TAO9_REGS_PA       0x4809D000  // MMC SDIO
#define OMAP2420_L4TAO10_REGS_PA      0x4809F000  // Reserved
#define OMAP2420_L4TAO11_REGS_PA      0x480A1000  // RNG
#define OMAP2420_L4TAO12_REGS_PA      0x480A3000  // DES3DES
#define OMAP2420_L4TAO13_REGS_PA      0x480A5000  // SHA1MD5
#define OMAP2420_L4TA37_REGS_PA       0x480A7000  // AES
#define OMAP2420_L4TA38_REGS_PA       0x480AA000  // PKA
#define OMAP2420_L4TAO14_REGS_PA      0x480B1000  // Reserved
#define OMAP2420_L4TA39_REGS_PA       0x480B3000  // HDQ/1-Wire

#endif
