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
//------------------------------------------------------------------------------
//
//  File:  omap5912_config.h
//
#ifndef __OMAP5912_CONFIG_H
#define __OMAP5912_CONFIG_H

typedef volatile struct {
	UINT32 FUNC_MUX_CTRL_0;		//0x00
	UINT32 FUNC_MUX_CTRL_1;		//0x04
	UINT32 FUNC_MUX_CTRL_2;		//0x08
	UINT32 COMP_MODE_CTRL_0;	//0x0C
	UINT32 FUNC_MUX_CTRL_3;		//0x10
	UINT32 FUNC_MUX_CTRL_4;		//0x14
	UINT32 FUNC_MUX_CTRL_5;		//0x18
	UINT32 FUNC_MUX_CTRL_6;		//0x1C
	UINT32 FUNC_MUX_CTRL_7;		//0x20
	UINT32 FUNC_MUX_CTRL_8;		//0x24
	UINT32 FUNC_MUX_CTRL_9;		//0x28
	UINT32 FUNC_MUX_CTRL_A;		//0x2C
	UINT32 FUNC_MUX_CTRL_B;		//0x30
	UINT32 FUNC_MUX_CTRL_C;		//0x34
	UINT32 FUNC_MUX_CTRL_D;		//0x38
		UINT32 DUMMY001;		//0x3C
	UINT32 PULL_DWN_CTRL_0;		//0x40
	UINT32 PULL_DWN_CTRL_1;		//0x44
	UINT32 PULL_DWN_CTRL_2;		//0x48
	UINT32 PULL_DWN_CTRL_3;		//0x4C
	UINT32 GATE_INH_CTRL_0;     //0x50
	    UINT32 DUMMY00;			//0x54
	UINT32 CONF_REV;			//0x58
		UINT32 DUMMY000;		//0x5C
	UINT32 VOLTAGE_CTRL_0;		//0x60
	UINT32 USB_TRANSCEIVER_CTRL;//0x64
	UINT32 LDO_PWRDN_CBTRK;		//0x68
		UINT32 DUMMY01;			//0x6C
	UINT32 TEST_DBG_CTRL_0;		//0x70
		UINT32 DUMMY02;			//0x74
		UINT32 DUMMY03;			//0x78
		UINT32 DUMMY04;			//0x7C
	UINT32 MOD_CONF_CTRL_0;		//0x80
		UINT32 DUMMY05;			//0x84
		UINT32 DUMMY06;			//0x88
		UINT32 DUMMY07;			//0x8C
	UINT32 FUNC_MUX_CTRL_E;		//0x90
	UINT32 FUNC_MUX_CTRL_F;		//0x94
	UINT32 FUNC_MUX_CTRL_10;    //0x98
	UINT32 FUNC_MUX_CTRL_11;    //0x9C
	UINT32 FUNC_MUX_CTRL_12;    //0xA0
		UINT32 DUMMY08;			//0xA4
		UINT32 DUMMY09;			//0xA8
	UINT32 PULL_DWN_CTRL_4;     //0xAC
		UINT32 DUMMY10;			//0xB0
	UINT32 PU_PD_SEL_0;         //0xB4
	UINT32 PU_PD_SEL_1;         //0xB8
	UINT32 PU_PD_SEL_2;         //0xBC
	UINT32 PU_PD_SEL_3;         //0xC0
	UINT32 PU_PD_SEL_4;         //0xC4
		UINT32 DUMMY11;			//0xC8
		UINT32 DUMMY12;			//0xCC
	UINT32 FUNC_MUX_DSP_DMA_A;  //0xD0
	UINT32 FUNC_MUX_DSP_DMA_B;  //0xD4
	UINT32 FUNC_MUX_DSP_DMA_C;  //0xD8
	UINT32 FUNC_MUX_DSP_DMA_D;  //0xDC
		UINT32 DUMMY13;			//0xE0
		UINT32 DUMMY14;			//0xE4
		UINT32 DUMMY15;			//0xE8
	UINT32 FUNC_MUX_ARM_DMA_A;  //0xEC
	UINT32 FUNC_MUX_ARM_DMA_B;  //0xF0  
	UINT32 FUNC_MUX_ARM_DMA_C;  //0xF4
	UINT32 FUNC_MUX_ARM_DMA_D;  //0xF8
	UINT32 FUNC_MUX_ARM_DMA_E;  //0xFC
	UINT32 FUNC_MUX_ARM_DMA_F;  //0x100
	UINT32 FUNC_MUX_ARM_DMA_G;  //0x104
		UINT32 DUMMY16;			//0x108
		UINT32 DUMMY17;			//0x10C
	UINT32 MOD_CONF_CTRL_1;     //0x110
		UINT32 DUMMY18;			//0x114
		UINT32 DUMMY19;			//0x118
		UINT32 DUMMY20;			//0x11C
	UINT32 SECCTRL;				//0x120
		UINT32 DUMMY22;			//0x124
		UINT32 DUMMY23;			//0x128
		UINT32 DUMMY24;			//0x12C
	UINT32 CONF_STATUS;         //0x130
		UINT32 DUMMY25;			//0x134
		UINT32 DUMMY26;			//0x138
		UINT32 DUMMY27;			//0x13C
	UINT32 RESET_CONTROL;       //0x140
		UINT32 DUMMY28;			//0x144
		UINT32 DUMMY29;			//0x148
		UINT32 DUMMY30;			//0x14C
	UINT32 CONF_5912_CTRL;      //0x150
} OMAP5912_CONFIG_REGS;

#define CONF_MOD_UART1_CLK_REQ	(1<<29)
#define CONF_MOD_MMCSD_CLK_REQ	(1<<23)

#define CONF_ARMIO_RESET_R	(1<<2)

#endif
