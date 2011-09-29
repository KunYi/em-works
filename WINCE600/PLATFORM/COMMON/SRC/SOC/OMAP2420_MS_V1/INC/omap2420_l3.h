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
//  File:  omap2420_L3.h
//
//  This header file is comprised of L3 module register details defined as 
//  structures and macros for configuring L3 module.
//

#ifndef __OMAP2420_L3_H
#define __OMAP2420_L3_H

//------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// L3 Interconnect
//////////////////////////////////////////////////////////////////////

//
// L3 LLRC Error logging Registers
//

// LLRC-OCM-RAM Base Address : OMAP2420_LLRC_OCM_RAM_PA (0x68005400)
// LLRC-OCM-ROM Base Address : OMAP2420_LLRC_OCM_ROM_PA (0x68005C00)
// LLRC-GPMC Base Address    : OMAP2420_LLRC_GPMC_PA    (0x68006400)
// LLRC-SMS Base Address     : OMAP2420_LLRC_SMS_PA     (0x68006C00)


typedef struct __LLRCREGS__
{
   unsigned long ulRESERVED_1[82];
   volatile unsigned long ulTAERRLOGA; //offset 0x148, Address of request with error  
   unsigned long ulRESERVED_0x14C;
   volatile unsigned long ulTAERRLOG;  //offset 0x150, LSB of TA error conditions
   unsigned long ulRESERVED_2[18];
   volatile unsigned long ulTASTATE;   //offset 0x19C, error log for OCM and GPMC
}
OMAP2420_LLRC_REGS, *pLLRCREGS;
                                              
//
// IM Registers
//

// L3IMREGS struct is common for all IMs (IM1-IM8) : see base address definitions below
typedef struct __L3IMREGS__
{
   unsigned long ulRESERVED_1[42];      //00-A4
   volatile unsigned long ulSBIMERRLOGA;  //offset 0xA8,SB addr of req with error
   unsigned long ulRESERVED_0xAC;
   volatile unsigned long ulSBIMERRLOG;   //offset 0xB0,IM error conditions log info
   unsigned long ulRESERVED_2[55];
   volatile unsigned long ulSBIMSTATE;    //offset 0x190,initiator agent state values access reg
   unsigned long ulRESERVED_0x194;
   volatile unsigned long ulSBTMSTATE;    //offset 0x198,status info & target agent ctrl values
   unsigned long ulRESERVED_3[3];
   volatile unsigned long ulSBIMCONFIG_L;   //offset 0x1A8,initiator agent LSBs operation config reg
   volatile unsigned long ulSBIMCONFIG_H;   //offset 0x1AC,initiator agent MSBs operation config reg
   unsigned long ulRESERVED_4[18];
   volatile unsigned long ulSBID;           //offset 0x1F8,identification reg
}
OMAP2420_L3IM_REGS, *pL3IMREGS;

//////////////////////////////////////////////////////////////////////
// IMMPU2SB Registers
//////////////////////////////////////////////////////////////////////

typedef struct __IMMPU2SBREGS__
{
   unsigned long ulRESERVED_1[42];       //00-A4
   volatile unsigned long ulSBIMERRLOGA; //offset 0xA8,SB addr of req with error
   unsigned long ulRESERVED_0xAC;
   volatile unsigned long ulSBIMERRLOG;  //offset 0xB0,IM error conditions log info
   unsigned long ulRESERVED_2[55];
   volatile unsigned long ulSBIMSTATE;   //offset 0x190,initiator agent state values access reg
   unsigned long ulRESERVED_0x194;
   volatile unsigned long ulSBTMSTATE_L; //offset 0x198,status info & target agent control values-LSB
   volatile unsigned long ulSBTMSTATE_H; //offset 0x19C,status info & target agent control values-MSB
   unsigned long ulRESERVED_3[2];
   volatile unsigned long ulSBIMCONFIG_L;//offset 0x1A8,configures operation of initiator agent LSBs
   volatile unsigned long ulSBIMCONFIG_H;//offset 0x1AC,configures operation of initiator agent MSBs
   unsigned long ulRESERVED_4[18];
   volatile unsigned long ulSBID_L;      //offset 0x1F8,Identification reg-lower 32 bits
   volatile unsigned long ulSBID_H;      //offset 0x1F8,Identification reg-upper 32 bits                                                                                         
}
OMAP2420_IMMPU2SB_REGS, *pIMMPU2SBREGS;
      
//////////////////////////////////////////////////////////////////////
// L3 Firewall Registers
// - see below for definitions for
// VLYNQFW Base Address  : OMAP2420_L3_VLYNQFW_PA (0x68001800)
// DSPFW Base Address    : OMAP2420_L3_DSPFW_PA (0x68002800)
// IVAFW Base Address    : OMAP2420_L3_IVAFW_PA (0x68003000)
// RAMFW Base Address    : OMAP2420_L3_RAMFW_PA (0x68005000)
// ROMFW Base Address    : OMAP2420_L3_ROMFW_PA (0x68005800)
// GPMCFW Base Address   : OMAP2420_L3_GPMCFW_PA (0x68006000)
//
// All the above firewalls have common register functions and so
// a single structure is defined for all the firewalls.

typedef struct __L3FWREGS__
{
  unsigned long ulRESERVED_1[8];        //0ffset 0x0 - 0x1F
  volatile unsigned long ulERRLOG;      //offset 0x20,logs info abt req that incur protection violation
  unsigned long ulRESERVED_2[9];         //offset 0x24-0x48
  volatile unsigned long ulREQINFOPERM0;//offset 0x48, request info permission conf for region 0
  unsigned long ulRESERVED_0x4C;
  volatile unsigned long ulREADPERM0;   //offset 0x50,read permission conf for region 0
  unsigned long ulRESERVED_0x54;
  volatile unsigned long ulWRITEPERM0;  //offset 0x58,write permission conf for region 0
  unsigned long ulRESERVED_0x5C;
  volatile unsigned long ulADDMATCH1;   //offset 0x60, address match conf for region1
  unsigned long ulRESERVED_0x64;
  volatile unsigned long ulREQINFOPERM1;//offset 0x68, request info permission conf for region 1
  unsigned long ulRESERVED_0x6C;
  volatile unsigned long ulREADPERM1;   //offset 0x70,read permission conf for region 1
  unsigned long ulRESERVED_0x74;
  volatile unsigned long ulWRITEPERM1;  //offset 0x78,write permission conf for region 1
  unsigned long ulRESERVED_0x7C;
  volatile unsigned long ulADDMATCH2;   //offset 0x80, address match conf for region2
  unsigned long ulRESERVED_0x84;        
  volatile unsigned long ulREQINFOPERM2;//offset 0x88, request info permission conf for region 2
  unsigned long ulRESERVED_0x8C;
  volatile unsigned long ulREADPERM2;   //offset 0x90,read permission conf for region 2
  unsigned long ulRESERVED_0x94;
  volatile unsigned long ulWRITEPERM2;  //offset 0x98,write permission conf for region 2
  unsigned long ulRESERVED_0x9C;
  volatile unsigned long ulADDMATCH3;   //offset 0xA0, address match conf for region3
  unsigned long ulRESERVED_0xA4;
  volatile unsigned long ulREQINFOPERM3;//offset 0xA8, request info permission conf for region 3
  unsigned long ulRESERVED_0xAC;
  volatile unsigned long ulREADPERM3;   //offset 0xB0,read permission conf for region 3
  unsigned long ulRESERVED_0xB4;
  volatile unsigned long ulWRITEPERM3;  //offset 0xB8,write permission conf for region 3
  unsigned long ulRESERVED_0xBC;
  volatile unsigned long ulADDMATCH4;   //offset 0xC0, address match conf for region4
  unsigned long ulRESERVED_0xC4;
  volatile unsigned long ulREQINFOPERM4;//offset 0xC8, request info permission conf for region 4
  unsigned long ulRESERVED_0xCC;
  volatile unsigned long ulREADPERM4;   //offset 0xD0,read permission conf for region 4
  unsigned long ulRESERVED_0xD4;
  volatile unsigned long ulWRITEPERM4;  //offset 0xD8,write permission conf for region 4
  unsigned long ulRESERVED_0xDC;
  volatile unsigned long ulADDMATCH5;   //offset 0xE0, address match conf for region 5
  unsigned long ulRESERVED_0xE4;
  volatile unsigned long ulREQINFOPERM5;//offset 0xE8, request info permission conf for region 5
  unsigned long ulRESERVED_0xEC;
  volatile unsigned long ulREADPERM5;   //offset 0xF0,read permission conf for region 5
  unsigned long ulRESERVED_0xF4;
  volatile unsigned long ulWRITEPERM5;  //offset 0xF8,write permission conf for region 5
  unsigned long ulRESERVED_0xFC;
  volatile unsigned long ulADDMATCH6;   //offset 0x100, address match conf for region 6
  unsigned long ulRESERVED_0x104;
  volatile unsigned long ulREQINFOPERM6;//offset 0x108, request info permission conf for region 6
  unsigned long ulRESERVED_0x10C;
  volatile unsigned long ulREADPERM6;   //offset 0x110,read permission conf for region 6
  unsigned long ulRESERVED_0x114;
  volatile unsigned long ulWRITEPERM6;  //offset 0x118,write permission conf for region 6    
}
OMAP_L3FW_REGS, *pL3FWREGS;
//
// IMTM1 Registers
// Base Address    : OMAP2420_L3IMTM1_PA (0x68001E00)

typedef struct __L3IMTM1REGS__
{
   unsigned long ulRESERVED_1[42];       
   volatile unsigned long ulSBIMERRLOGA; //offset 0xA8,SB addr of request with error
   unsigned long ulRESERVED_0xAC;
   volatile unsigned long ulSBIMERRLOG;  //offset 0xB0,logs info abt IM error conditions
   unsigned long ulRESERVED_2[37];
   volatile unsigned long ulSBTMERRLOGA; //offset 0x148,logs info abt TM err conditions
   unsigned long ulRESERVED_0x14C;
   volatile unsigned long ulSBTMERRLOG;  //offset 0x150,logs info abt TM err conditions
   unsigned long ulRESERVED_3[15];
   volatile unsigned long ulSBIMSTATE;   //offset 0x190,initiator agent state values access reg
   unsigned long ulRESERVED_0x194;
   volatile unsigned long ulSBTMSTATE_L; //offset 0x198,status info&target agent ctrl values-LSBs
   volatile unsigned long ulSBTMSTATE_H; //offset 0x19C,status info&target agent ctrl values-MSBs
   unsigned long ulRESERVED_4[2];
   volatile unsigned long ulSBIMCONFIG_L;//offset 0x1A8,configures operation of initiator agent LSBs
   volatile unsigned long ulSBIMCONFIG_H;//offset 0x1AC,configures operation of initiator agent MSBs
   unsigned long ulRESERVED_5[3];
   volatile unsigned long ulSBTMCONFIG;  //offset 0x1BC,clk generator config & target agent response flag usage
   unsigned long ulRESERVED_6[14];
   volatile unsigned long ulSBID;        //offset 0x1F8,identification
}
OMAP2420_L3IMTM1_REGS, *pL3IMTM1REGS;



// SMSFW Base Address    : 0x68008000
typedef struct __L3SMSFWREGS__
{
   unsigned long ulRESERVED_1[16];       
   volatile unsigned long ulSMS_RG_START0; //offset 0x40
   volatile unsigned long ulSMS_RG_END0; //offset 0x44
   volatile unsigned long ulSMS_RG_ATT0; //offset 0x48
   volatile unsigned long ulRESERVED_0x4C;
   volatile unsigned long ulSMS_RG_START1; //offset 0x50
   volatile unsigned long ulSMS_RG_END1; //offset 0x54
   volatile unsigned long ulSMS_RG_ATT1; //offset 0x58
   volatile unsigned long ulRESERVED_0x5C;
   volatile unsigned long ulSMS_RG_START2; //offset 0x60
   volatile unsigned long ulSMS_RG_END2; //offset 0x64
   volatile unsigned long ulSMS_RG_ATT2; //offset 0x68
   volatile unsigned long ulRESERVED_0x6C;
   volatile unsigned long ulSMS_RG_START3; //offset 0x70
   volatile unsigned long ulSMS_RG_END3; //offset 0x74
   volatile unsigned long ulSMS_RG_ATT3; //offset 0x78
   volatile unsigned long ulRESERVED_0x7C;
   volatile unsigned long ulSMS_RG_WRPERM0; //offset 0x80
   volatile unsigned long ulSMS_RG_RDPERM0; //offset 0x84
   volatile unsigned long ulRESERVED_0x88;
   volatile unsigned long ulRESERVED_0x8C;
   volatile unsigned long ulSMS_RG_WRPERM1; //offset 0x90
   volatile unsigned long ulSMS_RG_RDPERM1; //offset 0x94
   volatile unsigned long ulRESERVED_0x98;
   volatile unsigned long ulRESERVED_0x9C;
   volatile unsigned long ulSMS_RG_WRPERM2; //offset 0xA0
   volatile unsigned long ulSMS_RG_RDPERM2; //offset 0xA4
   volatile unsigned long ulRESERVED_0xA8;
   volatile unsigned long ulRESERVED_0xAC;
   volatile unsigned long ulSMS_RG_WRPERM3; //offset 0xB0
   volatile unsigned long ulSMS_RG_RDPERM3; //offset 0xB4
   volatile unsigned long ulRESERVED_0xB8;
   volatile unsigned long ulRESERVED_0xBC;
   volatile unsigned long ulSMS_REGS_SECURITY; // offset 0xC0
}
OMAP2420_L3SMSFW_REGS, *pL3SMSFWREGS;

// End of Firewall registers

//------------------------------------------------------------------------------
//
// Base Address macro definition should be included in this header file.
//
//------------------------------------------------------------------------------

//  L3 Interconnect registers
#define OMAP2420_LLRC_OCM_RAM_PA      0x68005400  // LLRC-OCM-RAM
#define OMAP2420_LLRC_OCM_ROM_PA      0x68005C00  // LLRC-OCM-ROM
#define OMAP2420_LLRC_GPMC_PA         0x68006400  // LLRC-GPMC
#define OMAP2420_LLRC_SMS_PA          0x68006C00  // LLRC-SMS

// L3 SB Registers
#define OMAP2420_L3SB_IM1_PA          0x68000400  // System DMA RD port
#define OMAP2420_L3SB_IM2_PA          0x68000600  // System DMA WR port
#define OMAP2420_L3SB_IM3_PA          0x68000800  // DSS port
#define OMAP2420_L3SB_IM7_PA          0x68000A00  // DSP2SB port
#define OMAP2420_L3SB_IMMPU2SB_PA     0x68000C00  // MPU2SB port
#define OMAP2420_L3SB_IM5_PA          0x68001000  // IVA I port
#define OMAP2420_L3SB_IM6_PA          0x68001200  // USB port
#define OMAP2420_L3SB_IM4_PA          0x68001400  // Camera port
#define OMAP2420_L3SB_VLYNQFW_PA      0x68001800  // VLYNQ firewall port
#define OMAP2420_L3SB_IMTM1_PA        0x68001E00  // VLYNQ port
#define OMAP2420_L3SB_IM8_PA          0x68002000  // SSI port
#define OMAP2420_L3SB_L4TA_PA         0x68002400  // L4 target agent port
#define OMAP2420_L3SB_DSPFW_PA        0x68002800  // DSP memory firewall port
#define OMAP2420_L3SB_TM4_PA          0x68002E00  // DSP memory port
#define OMAP2420_L3SB_IVAFW_PA        0x68003000  // IVA firewall port
#define OMAP2420_L3SB_TM7_PA          0x68003600  // IVA T port
#define OMAP2420_L3SB_TM5_PA          0x68003B00  // GFX subsystem port
#define OMAP2420_L3SB_TM6_PA          0x68003D00  // Command write port
#define OMAP2420_L3SB_TM1_PA          0x68004100  // SB2SMS port
#define OMAP2420_L3SB_TM3_PA          0x68004300  // SB2OCM port
#define OMAP2420_L3SB_TM2_PA          0x68004500  // SB2GPMC port
#define OMAP2420_L3SB_RAMFW_PA        0x68005000  // OCM RAM firewall port
#define OMAP2420_L3SB_ROMFW_PA        0x68005800  // OCM ROM firewall port
#define OMAP2420_L3SB_GPMCFW_PA       0x68006000  // GPMC firewall port
#define OMAP2420_L3SB_SMSFW_PA        0x68008000  // SMS firewall port

// L3 Firewall Registers
#define OMAP2420_L3FW_VLYNQFW_PA      0x68001800
#define OMAP2420_L3FW_DSPFW_PA        0x68002800
#define OMAP2420_L3FW_IVAFW_PA        0x68003000
#define OMAP2420_L3FW_RAMFW_PA        0x68005000
#define OMAP2420_L3FW_ROMFW_PA        0x68005800
#define OMAP2420_L3FW_GPMCFW_PA       0x68006000

// L3 IMTM1 base address
#define OMAP2420_L3IMTM1_PA           0x68001E00

// SMSFW base address
#define OMAP2420_L3SMSFW_PA           0x68008000



#endif
