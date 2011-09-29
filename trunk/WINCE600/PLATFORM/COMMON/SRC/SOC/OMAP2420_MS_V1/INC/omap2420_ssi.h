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
//  File:  omap2420_SSI.h
//
//  This header file is comprised of register details for the SSI module.
//

#ifndef __OMAP2420_SSI_H
#define __OMAP2420_SSI_H

//------------------------------------------------------------------------------

typedef struct __SSICTRLREGS__
{
   unsigned long ulSSI_REVISION;            //offset 0x00, SSI revision
   unsigned long ulRESERVED_1[3];
   volatile unsigned long ulSSI_SYSCONFIG;  //offset 0x10, system config
   volatile unsigned long ulSSI_SYSSTATUS;  //offset 0x14, system status
   unsigned long ulRESERVED_2[506];
   volatile unsigned long ulSSI_GDD_MPU_IRQ_STATUS; //0x800,GDD MPU IRQ stat
   volatile unsigned long ulSSI_GDD_MPU_IRQ_ENABLE; //0x804,GDD MPU IRQ enable
   volatile unsigned long ulSSI_P1_MPU_IRQ0_STATUS; //0x808, P1 MPU IRQ0 status
   volatile unsigned long ulSSI_P1_MPU_IRQ0_ENABLE; //0x80C, P1 MPU IRQ0 enable
   volatile unsigned long ulSSI_P1_MPU_IRQ1_STATUS; //0x810, P1 MPU IRQ1 status
   volatile unsigned long ulSSI_P1_MPU_IRQ1_ENABLE; //0x814, P1 MPU IRQ1 enable
   volatile unsigned long ulSSI_P2_MPU_IRQ0_STATUS; //0x818, P2 MPU IRQ0 status
   volatile unsigned long ulSSI_P2_MPU_IRQ0_ENABLE; //0x81C, P2 MPU IRQ0 enable
   volatile unsigned long ulSSI_P2_MPU_IRQ1_STATUS; //0x820, P2 MPU IRQ1 status
   volatile unsigned long ulSSI_P2_MPU_IRQ1_ENABLE; //0x824, P2 MPU IRQ1 enable
   volatile unsigned long ulSSI_GDD_DSP_IRQ_STATUS; //0x828, GDD-DSP MPU IRQ status
   volatile unsigned long ulSSI_GDD_DSP_IRQ_ENABLE; //0x82C, GDD-DSP MPU IRQ enable
   volatile unsigned long ulSSI_P1_DSP_IRQ0_STATUS; //0x830, P1-DSP MPU IRQ0 status
   volatile unsigned long ulSSI_P1_DSP_IRQ0_ENABLE; //0x834, P1-DSP MPU IRQ0 enable
   volatile unsigned long ulSSI_P1_DSP_IRQ1_STATUS; //0x838, P1-DSP MPU IRQ1 status
   volatile unsigned long ulSSI_P1_DSP_IRQ1_ENABLE; //0x83C, P1-DSP MPU IRQ1 enable
   volatile unsigned long ulSSI_P2_DSP_IRQ0_STATUS; //0x840, P2-DSP MPU IRQ0 status
   volatile unsigned long ulSSI_P2_DSP_IRQ0_ENABLE; //0x844, P2-DSP MPU IRQ0 enable
   volatile unsigned long ulSSI_P2_DSP_IRQ1_STATUS; //0x848, P2-DSP MPU IRQ1 status
   volatile unsigned long ulSSI_P2_DSP_IRQ1_ENABLE; //0x84C, P2-DSP MPU IRQ1 enable
   unsigned long ulRESERVED_3[236];
   volatile unsigned long ulSSI_P1_WAKE;          //0xC00, P1 wakeup 
   volatile unsigned long ulSSI_P1_CLEAR_WAKE;    //0xC04, P1 Clear wakeup
   volatile unsigned long ulSSI_P1_SET_WAKE;      //0xC08, P1 Set wakeup
   unsigned long ulRESERVED_0xC0C;
   volatile unsigned long ulSSI_P2_WAKE;          //0xC10, P2 wakeup 
   volatile unsigned long ulSSI_P2_CLEAR_WAKE;    //0xC14, P2 Clear wakeup
   volatile unsigned long ulSSI_P2_SET_WAKE;      //0xC18, P2 Set wakeup
}
OMAP2420_SSI_CTRL_REGS, *pSSICTRLREGS;

typedef struct __GDD1REGS__
{
   volatile unsigned long ulGDD_HW_ID;       //offset 0x00
   unsigned long ulRESERVED_1[3];
   volatile unsigned long ulGDD_PPORT_ID;    //offset 0x10
   volatile unsigned long ulGDD_MPORT_ID;    //offset 0x14
   unsigned long ulRESERVED_2[2];
   volatile unsigned long ulGDD_PPORT_SR;    //offset 0x20
   volatile unsigned long ulGDD_MPORT_SR;    //offset 0x24
   unsigned long ulRESERVED_3[6];
   volatile unsigned long ulGDD_TEST;        //offset 0x40
   unsigned long ulRESERVED_4[47];
   volatile unsigned long ulGDD_GCR;         //offset 0x100
   unsigned long ulRESERVED_5[63];
   volatile unsigned long ulGDD_GRST;        //offset 0x200
   unsigned long ulRESERVED_6[383];
   volatile unsigned short ulGDD_CSDP0;      //offset 0x800
   volatile unsigned short ulGDD_CCR0;       //offset 0x802
   volatile unsigned short ulGDD_CICR0;      //offset 0x804
   volatile unsigned short ulGDD_CSR0;       //offset 0x806
   volatile unsigned long ulGDD_CSSA0;       //offset 0x808
   volatile unsigned long ulGDD_CDSA0;       //offset 0x80C
   volatile unsigned short ulGDD_CEN0;       //offset 0x810
   unsigned long ulRESERVED_0x814;
   volatile unsigned short ulGDD_CSAC0;      //offset 0x818
   volatile unsigned short ulGDD_CDAC0;      //offset 0x81A
   unsigned short ulRESERVED_7[6];
   volatile unsigned short ulGDD_CLNK_CTRL0; //offset 0x828
   unsigned short ulRESERVED_8[11];
   volatile unsigned short ulGDD_CSDP1;      //offset 0x840
   volatile unsigned short ulGDD_CCR1;       //offset 0x842
   volatile unsigned short ulGDD_CICR1;      //offset 0x844
   volatile unsigned short ulGDD_CSR1;       //offset 0x846
   volatile unsigned long ulGDD_CSSA1;       //offset 0x848
   volatile unsigned long ulGDD_CDSA1;       //offset 0x84C
   volatile unsigned short ulGDD_CEN1;       //offset 0x850
   unsigned short ulRESERVED_9[3];
   volatile unsigned short ulGDD_CSAC1;      //offset 0x858
   volatile unsigned short ulGDD_CDAC1;      //offset 0x85A
   unsigned short ulRESERVED_10[6];
   volatile unsigned short ulGDD_CLNK_CTRL1; //offset 0x868
   unsigned short ulRESERVED_11[11];
   volatile unsigned short ulGDD_CSDP2;      //offset 0x880
   volatile unsigned short ulGDD_CCR2;       //offset 0x882
   volatile unsigned short ulGDD_CICR2;      //offset 0x884
   volatile unsigned short ulGDD_CSR2;       //offset 0x886
   volatile unsigned long ulGDD_CSSA2;       //offset 0x888
   volatile unsigned long ulGDD_CDSA2;       //offset 0x88C
   volatile unsigned short ulGDD_CEN2;       //offset 0x890
   unsigned short ulRESERVED_12[3];
   volatile unsigned short ulGDD_CSAC2;      //offset 0x898
   volatile unsigned short ulGDD_CDAC2;      //offset 0x89A
   unsigned short ulRESERVED_13[6];
   volatile unsigned short ulGDD_CLNK_CTRL2; //offset 0x8A8
   unsigned short ulRESERVED_14[11];
   volatile unsigned short ulGDD_CSDP3;      //offset 0x8C0
   volatile unsigned short ulGDD_CCR3;       //offset 0x8C2
   volatile unsigned short ulGDD_CICR3;      //offset 0x8C4
   volatile unsigned short ulGDD_CSR3;       //offset 0x8C6
   volatile unsigned long ulGDD_CSSA3;       //offset 0x8C8
   volatile unsigned long ulGDD_CDSA3;       //offset 0x8CC
   volatile unsigned short ulGDD_CEN3;       //offset 0x8D0
   unsigned short ulRESERVED_15[3];
   volatile unsigned short ulGDD_CSAC3;      //offset 0x8D8
   volatile unsigned short ulGDD_CDAC3;      //offset 0x8DA
   unsigned short ulRESERVED_16[6];
   volatile unsigned short ulGDD_CLNK_CTRL3; //offset 0x8E8
   unsigned short ulRESERVED_17[11];
   volatile unsigned short ulGDD_CSDP4;      //offset 0x900
   volatile unsigned short ulGDD_CCR4;       //offset 0x902
   volatile unsigned short ulGDD_CICR4;      //offset 0x904
   volatile unsigned short ulGDD_CSR4;       //offset 0x906
   volatile unsigned long ulGDD_CSSA4;       //offset 0x908
   volatile unsigned long ulGDD_CDSA4;       //offset 0x90C
   volatile unsigned short ulGDD_CEN4;       //offset 0x910
   unsigned short ulRESERVED_18[3];
   volatile unsigned short ulGDD_CSAC4;      //offset 0x918
   volatile unsigned short ulGDD_CDAC4;      //offset 0x91A
   unsigned short ulRESERVED_19[6];
   volatile unsigned short ulGDD_CLNK_CTRL4; //offset 0x928
   unsigned short ulRESERVED_20[11];
   volatile unsigned short ulGDD_CSDP5;      //offset 0x940
   volatile unsigned short ulGDD_CCR5;       //offset 0x942
   volatile unsigned short ulGDD_CICR5;      //offset 0x944
   volatile unsigned short ulGDD_CSR5;       //offset 0x946
   volatile unsigned long ulGDD_CSSA5;       //offset 0x948
   volatile unsigned long ulGDD_CDSA5;       //offset 0x94C
   volatile unsigned short ulGDD_CEN5;       //offset 0x950
   unsigned short ulRESERVED_21[3];
   volatile unsigned short ulGDD_CSAC5;      //offset 0x958
   volatile unsigned short ulGDD_CDAC5;      //offset 0x95A
   unsigned short ulRESERVED_22[6];
   volatile unsigned short ulGDD_CLNK_CTRL5; //offset 0x968
   unsigned short ulRESERVED_23[11];
   volatile unsigned short ulGDD_CSDP6;      //offset 0x980
   volatile unsigned short ulGDD_CCR6;       //offset 0x982
   volatile unsigned short ulGDD_CICR6;      //offset 0x984
   volatile unsigned short ulGDD_CSR6;       //offset 0x986
   volatile unsigned long ulGDD_CSSA6;       //offset 0x988
   volatile unsigned long ulGDD_CDSA6;       //offset 0x98C
   volatile unsigned short ulGDD_CEN6;       //offset 0x990
   unsigned short ulRESERVED_24[3];
   volatile unsigned short ulGDD_CSAC6;      //offset 0x998
   volatile unsigned short ulGDD_CDAC6;      //offset 0x99A
   unsigned short ulRESERVED_25[6];
   volatile unsigned short ulGDD_CLNK_CTRL6; //offset 0x9A8
   unsigned short ulRESERVED_26[11];
   volatile unsigned short ulGDD_CSDP7;      //offset 0x9C0
   volatile unsigned short ulGDD_CCR7;       //offset 0x9C2
   volatile unsigned short ulGDD_CICR7;      //offset 0x9C4
   volatile unsigned short ulGDD_CSR7;       //offset 0x9C6
   volatile unsigned long ulGDD_CSSA7;       //offset 0x9C8
   volatile unsigned long ulGDD_CDSA7;       //offset 0x9CC
   volatile unsigned short ulGDD_CEN7;       //offset 0x9D0
   unsigned short ulRESERVED_27[3];
   volatile unsigned short ulGDD_CSAC7;      //offset 0x9D8
   volatile unsigned short ulGDD_CDAC7;      //offset 0x9DA
   unsigned short ulRESERVED_28[6];
   volatile unsigned short ulGDD_CLNK_CTRL7; //offset 0x9E8
}
OMAP2420_GDD1_REGS, *pGDDREGS;

typedef struct __SSTREGS__
{
   volatile unsigned long ulSST_ID;             //offset 0x000
   volatile unsigned long ulSST_MODE;           //offset 0x004
   volatile unsigned long ulSST_FRAMESIZE;      //offset 0x008
   volatile unsigned long ulSST_TXSTATE;        //offset 0x00C
   volatile unsigned long ulSST_BUFSTATE;       //offset 0x010
   unsigned short ulRESERVED_0x014;
   volatile unsigned long ulSST_DIVISOR;        //offset 0x018
   unsigned short ulRESERVED_0x01C;
   volatile unsigned long ulSST_BREAK;          //offset 0x020
   volatile unsigned long ulSST_CHANNELS;       //offset 0x024
   volatile unsigned long ulSST_ARBMODE;        //offset 0x028
   unsigned short ulRESERVED_1[21];
   volatile unsigned long ulSST_BUFFER_CH0;     //offset 0x080
   volatile unsigned long ulSST_BUFFER_CH1;     //offset 0x084
   volatile unsigned long ulSST_BUFFER_CH2;     //offset 0x088
   volatile unsigned long ulSST_BUFFER_CH3;     //offset 0x08C
   volatile unsigned long ulSST_BUFFER_CH4;     //offset 0x090
   volatile unsigned long ulSST_BUFFER_CH5;     //offset 0x094
   volatile unsigned long ulSST_BUFFER_CH6;     //offset 0x098
   volatile unsigned long ulSST_BUFFER_CH7;     //offset 0x09C
   unsigned short ulRESERVED_2[8];
   volatile unsigned long ulSST_SWAPBUFFER_CH0; //offset 0x0C0
   volatile unsigned long ulSST_SWAPBUFFER_CH1; //offset 0x0C4
   volatile unsigned long ulSST_SWAPBUFFER_CH2; //offset 0x0C8
   volatile unsigned long ulSST_SWAPBUFFER_CH3; //offset 0x0CC
   volatile unsigned long ulSST_SWAPBUFFER_CH4; //offset 0x0D0
   volatile unsigned long ulSST_SWAPBUFFER_CH5; //offset 0x0D4
   volatile unsigned long ulSST_SWAPBUFFER_CH6; //offset 0x0D8
   volatile unsigned long ulSST_SWAPBUFFER_CH7; //offset 0x0DC
}
OMAP2420_SST_REGS, *pSSTREGS;

typedef struct __SSRREGS__
{
   volatile unsigned long ulSSR_ID;               //offset 0x00
   volatile unsigned long ulSSR_MODE;             //offset 0x04
   volatile unsigned long ulSSR_FRAMESIZE;        //offset 0x08
   volatile unsigned long ulSSR_RXSTATE;          //offset 0x0C
   volatile unsigned long ulSSR_BUFSTATE;         //offset 0x10
   unsigned short ulRESERVED_1[2];
   volatile unsigned long ulSSR_BREAK;            //offset 0x1C
   volatile unsigned long ulSSR_ERROR;            //offset 0x20
   volatile unsigned long ulSSR_ERRORACK;         //offset 0x24
   volatile unsigned long ulSSR_CHANNELS;         //offset 0x28
   volatile unsigned long ulSSR_OVERRUN;          //offset 0x2C
   volatile unsigned long ulSSR_OVERRUNACK;       //offset 0x30
   volatile unsigned long ulSSR_TIMEOUT;          //offset 0x34
   unsigned short ulRESERVED_2[18];
   volatile unsigned long ulSSR_BUFFER_CH0;       //offset 0x80
   volatile unsigned long ulSSR_BUFFER_CH1;       //offset 0x84
   volatile unsigned long ulSSR_BUFFER_CH2;       //offset 0x88
   volatile unsigned long ulSSR_BUFFER_CH3;       //offset 0x8C
   volatile unsigned long ulSSR_BUFFER_CH4;       //offset 0x90
   volatile unsigned long ulSSR_BUFFER_CH5;       //offset 0x94
   volatile unsigned long ulSSR_BUFFER_CH6;       //offset 0x98
   volatile unsigned long ulSSR_BUFFER_CH7;       //offset 0x9C
   unsigned short ulRESERVED_3[8];
   volatile unsigned long ulSSR_SWAPBUFFER_CH0;   //offset 0xC0
   volatile unsigned long ulSSR_SWAPBUFFER_CH1;   //offset 0xC4
   volatile unsigned long ulSSR_SWAPBUFFER_CH2;   //offset 0xC8
   volatile unsigned long ulSSR_SWAPBUFFER_CH3;   //offset 0xCC
   volatile unsigned long ulSSR_SWAPBUFFER_CH4;   //offset 0xD0
   volatile unsigned long ulSSR_SWAPBUFFER_CH5;   //offset 0xD4
   volatile unsigned long ulSSR_SWAPBUFFER_CH6;   //offset 0xD8
   volatile unsigned long ulSSR_SWAPBUFFER_CH7;   //offset 0xDC
}
OMAP2420_SSR_REGS, *pSSRREGS;



#endif
