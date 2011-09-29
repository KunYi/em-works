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
//  File:  omap5912_gpio.h
//
#ifndef __OMAP5912_GPIO_H
#define __OMAP5912_GPIO_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 REVISION;			//0000
    UINT8 RESERVED0000[0x0C];   //0004
    UINT32 SYSCONFIG;           //0010
    UINT32 SYSSTATUS;           //0014
    UINT32 IRQSTATUS1;          //0018
    UINT32 IRQENABLE1;          //001C
    UINT32 IRQSTATUS2;          //0020
    UINT32 IRQENABLE2;          //0024
    UINT32 WAKEUPENABLE;        //0028
    UINT32 DATAIN;              //002C
    UINT32 DATAOUT;             //0030
    UINT32 DIRECTION;           //0034
    UINT16 EDGE_CTRL1;          //0038
	UINT16 RESERVED0001;
    UINT16 EDGE_CTRL2;          //003C
	UINT8 RESERVED0002[0x5E];
    UINT32 CLEAR_IRQENABLE1;    //009C
	UINT32 RESERVED0003;
    UINT32 CLEAR_IRQENABLE2;    //00A4
    UINT32 CLEAR_WAKEUPENA;     //00A8
	UINT32 RESERVED0004;
    UINT32 CLEAR_DATAOUT;       //00B0
	UINT8 RESERVED0005[0x28];
    UINT32 SET_IRQENABLE1;    //00DC
	UINT32 RESERVED0006;
    UINT32 SET_IRQENABLE2;    //00E4
    UINT32 SET_WAKEUPENA;     //00E8
	UINT32 RESERVED0007;
    UINT32 SET_DATAOUT;       //00F0
} OMAP5912_GPIO_REGS;

//------------------------------------------------------------------------------

#endif // __OMAP5912_GPIO_H
