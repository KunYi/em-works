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
//  File:  omap2420_McSPI.h
//
//  This header file is comprised of register details of Mutli Channel Serial
//  Port Interface module
//

#ifndef __OMAP2420_MCSPI_H
#define __OMAP2420_MCSPI_H

//------------------------------------------------------------------------------



typedef struct __McSPIREGS__
{
   unsigned long ulMCSPI_REVISION;           //offset 0x00, Revision
   unsigned long ulRESERVED_1[3];
   volatile unsigned long ulMCSPI_SYSCONFIG;	//offset 0x10, system config
   volatile unsigned long ulMCSPI_SYSSTATUS;	//offset 0x14, system status
   volatile unsigned long ulMCSPI_IRQSTATUS;	//offset 0x18, IRQ status
   volatile unsigned long ulMCSPI_IRQENABLE;	//offset 0x1C, IRQ enable
   volatile unsigned long ulMCSPI_WAKEUPENABLE;	//offset 0x20, WKUP enable
   volatile unsigned long ulMCSPI_SYST;			//offset 0x24, system test 
   volatile unsigned long ulMCSPI_MODULCTRL;	//offset 0x28, SPI config
   volatile unsigned long ulMCSPI_CHCONF0;		//offset 0x2C, channel 0 config
   volatile unsigned long ulMCSPI_CHSTATUS0;	//offset 0x30, channel 0 status
   volatile unsigned long ulMCSPI_CHCTRL0;		//offset 0x34, channel 0 control
   volatile unsigned long ulMCSPI_TX0;			//offset 0x38, Transmit0 register
   volatile unsigned long ulMCSPI_RX0;			//offset 0x3C, Receive0 register         
   volatile unsigned long ulMCSPI_CHCONF1;		//offset 0x40, channel 1 config
   volatile unsigned long ulMCSPI_CHSTATUS1;	//offset 0x44, channel 1 status
   volatile unsigned long ulMCSPI_CHCTRL1;		//offset 0x48, channel 1 control
   volatile unsigned long ulMCSPI_TX1;			//offset 0x4C, Transmit1 register
   volatile unsigned long ulMCSPI_RX1;			//offset 0x50, Receive 1 register
   volatile unsigned long ulMCSPI_CHCONF2;		//offset 0x54,
   volatile unsigned long ulMCSPI_CHSTATUS2;	//offset 0x58, channel 2 status
   volatile unsigned long ulMCSPI_CHCTRL2;		//offset 0x5C, channel 2 control
   volatile unsigned long ulMCSPI_TX2;			//offset 0x60, Transmit2 register
   volatile unsigned long ulMCSPI_RX2;			//offset 0x64, Receive2 register   
   volatile unsigned long ulMCSPI_CHCONF3;		//offset 0x68,
   volatile unsigned long ulMCSPI_CHSTATUS3;	//offset 0x6C, channel 3 status
   volatile unsigned long ulMCSPI_CHCTRL3;		//offset 0x70, channel 3 control
   volatile unsigned long ulMCSPI_TX3;			//offset 0x74, Transmit3 register
   volatile unsigned long ulMCSPI_RX3;			//offset 0x78, Receive3 register
}
OMAP2420_MCSPI_REGS, *pMcSPIREGS;

#define SPI_SYSC_SRST           (1 << 1)

#endif
