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
//  File:  omap2420_sdram.h
//
//  This header file is comprised of SDRAM module register structure and
//  definitions.

#ifndef __OMAP2420_SDRAM_H
#define __OMAP2420_SDRAM_H

//------------------------------------------------------------------------------


typedef struct __SMSREGS__
{
   unsigned long ulSMS_REVISION;              //offset 0x0
   unsigned long ulRESERVED_1[3];
   volatile unsigned long ulSMS_SYSCONFIG;    //offset 0x10, system config
   volatile unsigned long ulSMS_SYSSTATUS;    //offset 0x14, system status
   unsigned long ulRESERVED_0x18[10];
   volatile unsigned long ulSMS_RG_START0;    //offset 0x40, strt addr for reg0 
   volatile unsigned long ulSMS_RG_END0;      //offset 0x44, end addr for reg0 
   volatile unsigned long ulSMS_RG_ATT0;      //offset 0x48, prot attr for reg0
   unsigned long ulRESERVED_0x4C;
   volatile unsigned long ulSMS_RG_START1;    //offset 0x50, strt addr for reg1 
   volatile unsigned long ulSMS_RG_END1;      //offset 0x54, end addr for reg1 
   volatile unsigned long ulSMS_RG_ATT1;      //offset 0x58, prot attr for reg1
   unsigned long ulRESERVED_0x5C;
   volatile unsigned long ulSMS_RG_START2;    //offset 0x60, strt addr for reg2 
   volatile unsigned long ulSMS_RG_END2;      //offset 0x64, end addr for reg2 
   volatile unsigned long ulSMS_RG_ATT2;      //offset 0x68, prot attr for reg2
   unsigned long ulRESERVED_0x6C;
   volatile unsigned long ulSMS_RG_START3;    //offset 0x70, strt addr for reg3 
   volatile unsigned long ulSMS_RG_END3;      //offset 0x74, end addr for reg3 
   volatile unsigned long ulSMS_RG_ATT3;      //offset 0x78, prot attr for reg3
   unsigned long ulRESERVED_0x7C;
   volatile unsigned long ulSMS_RG_WRPERM0;   //offset 0x80,reg 0 initrs with 
                                              //write permission
   volatile unsigned long ulSMS_RG_RDPERM0;   //offset 0x84 reg 0 initrs with 
                                              //read permission
   unsigned long ulRESERVED_0x88[2];
   volatile unsigned long ulSMS_RG_WRPERM1;   //offset 0x90,reg 1 initiators 
                                              //with write permission
   volatile unsigned long ulSMS_RG_RDPERM1;   //offset 0x94,reg 1 initiators 
                                              //with read permission
   unsigned long ulRESERVED_0x98[2];
   volatile unsigned long ulSMS_RG_WRPERM2;   //offset 0xA0,reg 2 initiators 
                                              //with write permission
   volatile unsigned long ulSMS_RG_RDPERM2;   //offset 0xA4,reg 2 initiators 
                                              //with read permission
   unsigned long ulRESERVED_0xA8[2];
   volatile unsigned long ulSMS_RG_WRPERM3;   //offset 0xB0,reg 3 initiators 
                                              //with write permission
   volatile unsigned long ulSMS_RG_RDPERM3;   //offset 0xB4,reg 3 initiators 
                                              //with read permission
   unsigned long ulRESERVED_2[2];
   volatile unsigned long ulSMS_REGS_SECURITY; //offset 0xC0,security lvl to 
                                               //SMS registers
   unsigned long ulRESERVED_3[3];
   volatile unsigned long ulSMS_CLASS_ARBITER0; //offset 0xD0, arbitration params
   volatile unsigned long ulSMS_CLASS_ARBITER1; //offset 0xD4, arbitration params
   volatile unsigned long ulSMS_CLASS_ARBITER2; //offset 0xD8, arbitration params
   unsigned long ulRESERVED_0xDC;
   volatile unsigned long ulSMS_INTERCLASS_ARBITER; //offset 0xE0, arbit params
   volatile unsigned long ulSMS_CLASS_ROTATION0; //offset 0xE4,control number 
                                                 //of consecutice services
   volatile unsigned long ulSMS_CLASS_ROTATION1; //offset 0xE8,control number 
                                                 //of consecutice services 
   volatile unsigned long ulSMS_CLASS_ROTATION2; //offset 0xEC,control number 
                                                 //of consecutice services
   volatile unsigned long ulSMS_ERR_ADDR;     //offset 0xF0, Addr of an access
                                              //due to sec violation
   volatile unsigned long ulSMS_ERR_TYPE;     //offset 0xF4, information of an 
                                              //access due to sec violation
   volatile unsigned long ulSMS_POW_CTRL;     //offset 0xF8, power mgmt with 
                                              //OCP socket regs
   unsigned long ulRESERVED_0xFC;
   volatile unsigned long ulSMS_ROT_CONTROL0; //offset 0x100,virt rot frame buf
                                              //config for context0
   volatile unsigned long ulSMS_ROT_SIZE0;   //offset 0x104,bank org config 
                                              //for context 0
   volatile unsigned long ulSMS_ROT_PHYSICAL_BA0; //offset 0x108,phy base address0 
                                              //config for rot frame buf   
   unsigned long ulRESERVED_0x10C;
   volatile unsigned long ulSMS_ROT_CTRL1;    //offset 0x110,virt rot frame buf
                                              //config for context1
   volatile unsigned long ulSMS_ROT_SIZE1;   //offset 0x114,bank org config 
                                              //for context 1
   volatile unsigned long ulSMS_ROT_PHYSICAL_BA1; //offset 0x118,phy base addr1 
                                              //config
   unsigned long ulRESERVED_0x11C;
   volatile unsigned long ulSMS_ROT_CTRL2;    //offset 0x120,virt rot frame 
                                              //buf config for context2
   volatile unsigned long ulSMS_ROT_SIZE2;   //offset 0x124,bank org config 
                                              //for context 2
   volatile unsigned long ulSMS_ROT_PHYSICAL_BA2; //offset 0x128,phy base addr2 
                                              //config
   unsigned long ulRESERVED_0x12C;
   volatile unsigned long ulSMS_ROT_CTRL3;    //offset 0x130,virt rot frame buf
                                              //config for context3
   volatile unsigned long ulSMS_ROT_SIZE3;   //offset 0x134,bank org config 
                                              //for context 3
   volatile unsigned long ulSMS_ROT_PHYSICAL_BA3; //offset 0x138,phy base addr3 
                                              //config
}
OMAP2420_SMS_REGS, *pSMSREGS;

//
// SDRC
//

// Base Address : OMAP2420_SDRC_REGS_PA (0x68009000)

typedef struct __SDRCREGS__
{
   unsigned long ulSDRC_REVISION;             //offset 0x00,IP revision code
   unsigned long ulRESERVED_1[3];
   volatile unsigned long ulSDRC_SYSCONFIG;   //offset 0x10,OCP i/f control
   volatile unsigned long ulSDRC_STATUS;      //offset 0x14,module status
   unsigned long ulRESERVED_2[10];
   volatile unsigned long ulSDRC_CS_CFG;      //offset 0x40,config of start 
                                              //addr of cs1
   volatile unsigned long ulSDRC_SHARING;     //offset 0x44,controls GPMC 
                                              //access wrt SDRC
   volatile unsigned long ulSDRC_ERR_ADDR;    //offset 0x48,addr of last illegal
                                              //access received on OCP i/f.
   volatile unsigned long ulSDRC_ERR_TYPE;    //offset 0x4C,info abt last 
                                              //illegal access
   unsigned long ulRESERVED_3[4];
   volatile unsigned long ulSDRC_DLLA_CTRL;   //offset 0x60,controls SDRC DLL A
                                              //resrce for fine tunig a DDR I/F
   volatile unsigned long ulSDRC_DLLA_STATUS; //offset 0x64, reflects the 
                                              //current status of DLL A
   volatile unsigned long ulSDRC_DLLB_CTRL;   //offset 0x68,controls SDRC DLL B 
                                              //resrce for fine tuning a DDR I/F
   volatile unsigned long ulSDRC_DLLB_STATUS; //offset 0x6C,reflects the curr 
                                              //status of DLL B
   volatile unsigned long ulSDRC_POWER;       //offset 0x70, defn of global pwr 
                                              //mgmt policy
   unsigned long ulRESERVED_4[3];
   volatile unsigned long ulSDRC_MCFG_0;      //offset 0x80,memory config reg0
   volatile unsigned long ulSDRC_MR_0;        //offset 0x84,???
   volatile unsigned long ulSDRC_EMR1_0;      //offset 0x88, DDR1 EMR reg
   volatile unsigned long ulSDRC_EMR2_0;      //offset 0x8C, low-power EMR reg
   volatile unsigned long ulSDRC_EMR3_0;      //offset 0x90, ???.      
   volatile unsigned long ulSDRC_DCDL1_CTRL;  //offset 0x94, DCDL delay adj ctrl
   volatile unsigned long ulSDRC_DCDL2_CTRL;  //offset 0x98, DCDL delay adj ctrl
   volatile unsigned long ulSDRC_ACTIM_CTRLA_0;//offset 0x9C, sets the ac params 
                                               //values in the clock cycle unit 
                                               //to match memory characteristics
   volatile unsigned long ulSDRC_ACTIM_CTRLB_0;//offset 0xA0, sets the ac params  
                                               //values in the clock cycle unit 
                                               //to match memory characteristics
   volatile unsigned long ulSDRC_RFR_CTRL_0;   //offset 0xA4,SDRAM autorefresh 
                                               //control
   volatile unsigned long ulSDRC_MANUAL_0;     //offset 0xA8,cmd to ext mem devices
   unsigned long ulRESERVED_0xAC;
   volatile unsigned long ulSDRC_MCFG_1;       //offset 0xB0,memory config reg1
   volatile unsigned long ulSDRC_MR_1;         //offset 0xB4,SDRAM MR reg
   volatile unsigned long ulSDRC_EMR1_1;       //offset 0xB8, DDR1 EMR reg
   volatile unsigned long ulSDRC_EMR2_1;       //offset 0xBC, low-power EMR reg
   volatile unsigned long ulSDRC_EMR3_1;       //offset 0xC0, ???.      
   volatile unsigned long ulSDRC_ACTIM_CTRLA_1;//offset 0xC4, sets the ac params 
                                               //values in the clock cycle unit 
                                               //to match memory characteristics
   volatile unsigned long ulSDRC_ACTIM_CTRLB_1;//offset 0xC8, sets the ac params 
                                               //values in the clock cycle unit 
                                               //to match memory characteristics
   unsigned long ulRESERVED_5[2];
   volatile unsigned long ulSDRC_RFR_CTRL_1;   //offset 0xD4,SDRAM autorefresh 
                                               //control
   volatile unsigned long ulSDRC_MANUAL_1;     //offset 0xD8,cmd to ext mem devs
}
OMAP2420_SDRC_REGS, *pSDRCREGS;


#endif
