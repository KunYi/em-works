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
//  File:  omap2420_gpio.h
//
//  This header file is comprised of GPIO module register details defined as 
//  structures and macros for configuring and controlling GPIO module.

#ifndef __OMAP2420_GPIO_H
#define __OMAP2420_GPIO_H

//------------------------------------------------------------------------------


// All the GPIO modules(1-4) have a common structure GPIOREGS for 
// the various registers control.

typedef struct __GPIOREGS__
{
   unsigned long ulGPIO_REVISION;           //offset 0x0, IP revision code
   unsigned long ulRESERVED_1[3];
   volatile unsigned long ulGPIO_SYSCONFIG; //offset 0x10,L4 interconnect params ctrl
   volatile unsigned long ulGPIO_SYSSTAUS;  //offset 0x14,status info abt module
   volatile unsigned long ulGPIO_IRQSTATUS1;//offset 0x18,IRQ1 status info
   volatile unsigned long ulGPIO_IRQENABLE1;//offset 0x1C,IRQ1 enable info
   volatile unsigned long ulGPIO_WAKEUPENABLE;//offset 0x20,wake-up enable info
   unsigned long ulRESERVED_0x24;
   volatile unsigned long ulGPIO_IRQSTATUS2;//offset 0x28,IRQ2 status info
   volatile unsigned long ulGPIO_IRQENABLE2;//offset 0x2C,IRQ2 enable info
   volatile unsigned long ulGPIO_CTRL;      //offset 0x30,clock gating functionality
   volatile unsigned long ulGPIO_OE;        //offset 0x34,pads config/pin's o/p capabilities
   volatile unsigned long ulGPIO_DATAIN;    //offset 0x38,data read reg
   volatile unsigned long ulGPIO_DATAOUT;   //offset 0x3C,data write reg
   volatile unsigned long ulGPIO_LEVELDETECT0;//offset 0x40,low-level intr enable
   volatile unsigned long ulGPIO_LEVELDETECT1;//offset 0x44,high-level intr enable
   volatile unsigned long ulGPIO_RISINGDETECT;//offset 0x48,rising edge intr/wkup enable
   volatile unsigned long ulGPIO_FALLINGDETECT;//offset 0x4C,falling edge intr/wkup enable
   volatile unsigned long ulGPIO_DEBOUNCENABLE;  //offset 0x50,input debounce enable
   volatile unsigned long ulGPIO_DEBOUNCINGTIME;  //offset 0x54,input debounce time
   unsigned long ulRESERVED_0x58;
   unsigned long ulRESERVED_0x5C;
   volatile unsigned long ulGPIO_CLEARIRQENABLE1;//offset 0x60,clear interrupt1 enable
   volatile unsigned long ulGPIO_SETIRQENABLE1;//offset 0x64,set interrupt1 enable
   unsigned long ulRESERVED_0x68;
   unsigned long ulRESERVED_0x6C;
   volatile unsigned long ulGPIO_CLEARIRQENABLE2;//offset 0x70,clear interrupt 2 enable
   volatile unsigned long ulGPIO_SETIRQENABLE2;//offset 0x74,set interrupt 2 enable
   unsigned long ulRESERVED_0x78;
   unsigned long ulRESERVED_0x7C;
   volatile unsigned long ulGPIO_CLEARWKUENA;  //offset 0x80,clear wakeup enable
   volatile unsigned long ulGPIO_SETWKUENA;  //offset 0x84,set wakeup enable
   unsigned long ulRESERVED_0x88;
   unsigned long ulRESERVED_0x8C;
   volatile unsigned long ulGPIO_CLEARDATAOUT; //offset 0x90,clear data output
   volatile unsigned long ulGPIO_SETDATAOUT; //offset 0x94,set data output
}
OMAP2420_GPIO_REGS, *pGPIOREGS;




#endif
