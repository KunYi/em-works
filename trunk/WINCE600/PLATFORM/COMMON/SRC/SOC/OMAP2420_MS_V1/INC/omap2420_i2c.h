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
//  File:  omap2420_i2c.h
//
//  This header file is comprised of I2C module register details defined as
//  structures and macros for controlling I2C.
//

#ifndef __OMAP2420_I2C_H
#define __OMAP2420_I2C_H

//------------------------------------------------------------------------------

//
// There are 2 I2C interface available in 2420.
// Both the interface have similar register sets available and so 
// the structure for the I2C will have the registers common and 
// user has to map to appropriate base address before using the 
// structure for accessing I2C1 and I2C2.



typedef volatile struct
{
   UINT16 I2C_REV;                 //offset 0x00, revision reg
   UINT16 usRESERVED_0x02;
   UINT16 I2C_IE;                  //offset 0x04, intr enable reg
   UINT16 usRESERVED_0x06;
   UINT16 I2C_STAT;                //offset 0x08, I2C status reg
   UINT16 usRESERVED_1[3];
   UINT16 I2C_SYSS;                //offset 0x10,module status info
   UINT16 usRESERVED_0x12;
   UINT16 I2C_BUF;                 //offset 0x14,RX DMA chnl disabled
   UINT16 usRESERVED_0x16;
   UINT16 I2C_CNT;                 //offset 0x18,no of bytes in I2C data payload
   UINT16 usRESERVED_0x1A;
   UINT16 I2C_DATA;                //offset 0x1C, data reg for I2C
   UINT16 usRESERVED_0x1E;
   UINT16 I2C_SYSC;                //offset 0x20, OCP i/f params
   UINT16 usRESERVED_0x22;
   UINT16 I2C_CON;                 //offset 0x24, func features ctrl
   UINT16 usRESERVED_0x26;
   UINT16 I2C_OA;                  //offset 0x28, 7/10-bit own addr
   UINT16 usRESERVED_0x2A;
   UINT16 I2C_SA;                  //offset 0x2C,7/10-bit slave addr
   UINT16 usRESERVED_0x2E;
   UINT16 I2C_PSC;                 //offset 0x30, prescale sampling
                               //clk divider value
   UINT16 usRESERVED_0x32;
   UINT16 I2C_SCLL;                //offset 0x34,SCL low time value
   UINT16 usRESERVED_0x36;
   UINT16 I2C_SCLH;                //offset 0x38,SCL high time value
   UINT16 usRESERVED_0x3A;
   UINT16 I2C_SYSTEST;             //offset 0x3C,sys lvl test bits
}OMAP2420_I2C_REGS;


//------------------------------------------------------------------------------

#define I2C_STAT_SBD            (1 << 15)
#define I2C_STAT_BB             (1 << 12)
#define I2C_STAT_ROVR           (1 << 11)
#define I2C_STAT_XUDF           (1 << 10)
#define I2C_STAT_AAS            (1 << 9)
#define I2C_STAT_GC             (1 << 5)
#define I2C_STAT_XRDY           (1 << 4)
#define I2C_STAT_RRDY           (1 << 3)
#define I2C_STAT_ARDY           (1 << 2)
#define I2C_STAT_NACK           (1 << 1)
#define I2C_STAT_AL             (1 << 0)

#define I2C_SYSS_RDONE          (1 << 0)

#define I2C_SYSC_SRST           (1 << 1)

#define I2C_CON_EN              (1 << 15)
#define I2C_CON_BE              (1 << 14)
#define I2C_CON_STB             (1 << 11)
#define I2C_CON_MST             (1 << 10)
#define I2C_CON_TRX             (1 << 9)
#define I2C_CON_XA              (1 << 8)
#define I2C_CON_STP             (1 << 1)
#define I2C_CON_STT             (1 << 0)

#endif
