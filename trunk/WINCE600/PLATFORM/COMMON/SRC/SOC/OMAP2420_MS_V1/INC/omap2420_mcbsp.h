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
//  File:  omap2420_McBSP.h
//
//  This header file is comprised of McBSP module register details defined as 
//  structures and macros for configuring and controlling McBSP module

#ifndef __OMAP2420_McBSP_H
#define __OMAP2420_McBSP_H

//------------------------------------------------------------------------------

// Base Address
// McBSP1 : 0x48074000
// McBSP2 : 0x48076000

typedef struct __McBSPREGS__
{
   volatile unsigned short usMCBSP_DRR2; //offset 0x00,data recv2 reg
   unsigned short usRESERVED_0x02;
   volatile unsigned short usMCBSP_DRR1; //offset 0x04,data recv1 reg
   unsigned short usRESERVED_0x06;
   volatile unsigned short usMCBSP_DXR2; //offset 0x08,data trans2 reg
   unsigned short usRESERVED_0x0A;
   volatile unsigned short usMCBSP_DXR1; //offset 0x0C,data trans1 reg
   unsigned short usRESERVED_0x0E;
   volatile unsigned short usMCBSP_SPCR2;//offset 0x10,serial port ctrl2
   unsigned short usRESERVED_0x12;
   volatile unsigned short usMCBSP_SPCR1;//offset 0x14,serial port ctrl1
   unsigned short usRESERVED_0x16;
   volatile unsigned short usMCBSP_RCR2;//offset 0x18,recv control 2
   unsigned short usRESERVED_0x1A;
   volatile unsigned short usMCBSP_RCR1;//offset 0x1C,recv control 1
   unsigned short usRESERVED_0x1E;
   volatile unsigned short usMCBSP_XCR2;//offset 0x20,trans control 2
   unsigned short usRESERVED_0x22;
   volatile unsigned short usMCBSP_XCR1;//offset 0x24,trans control 1
   unsigned short usRESERVED_0x26;
   volatile unsigned short usMCBSP_SRGR2;//offset 0x28,sample rate gen 2
   unsigned short usRESERVED_0x2A;
   volatile unsigned short usMCBSP_SRGR1;//offset 0x2C,sample rate gen 1
   unsigned short usRESERVED_0x2E;
   volatile unsigned short usMCBSP_MCR2;//offset 0x30,multichnl reg2
   unsigned short usRESERVED_0x32;
   volatile unsigned short usMCBSP_MCR1;//offset 0x34,multichnl reg1
   unsigned short usRESERVED_0x36;
   volatile unsigned short usMCBSP_RCERA;//offset 0x38,recv chn enbl partitionA
   unsigned short usRESERVED_0x3A;
   volatile unsigned short usMCBSP_RCERB;//offset 0x3C,recv chn enbl partitionB
   unsigned short usRESERVED_0x3E;
   volatile unsigned short usMCBSP_XCERA;//offset 0x40,tran chn enbl partitionA
   unsigned short usRESERVED_0x42;
   volatile unsigned short usMCBSP_XCERB;//offset 0x44,tran chn enbl partitionB
   unsigned short usRESERVED_0x46;
   volatile unsigned short usMCBSP_PCR;//offset 0x48,pin ctrl reg
   unsigned short usRESERVED_0x4A;
   volatile unsigned short usMCBSP_RCERC;//offset 0x4C,recv chnl enbl part C
   unsigned short usRESERVED_0x4E;
   volatile unsigned short usMCBSP_RCERD;//offset 0x50,recv chnl enbl part D
   unsigned short usRESERVED_0x52;
   volatile unsigned short usMCBSP_XCERC;//offset 0x54,tran chnl enbl part C
   unsigned short usRESERVED_0x56;
   volatile unsigned short usMCBSP_XCERD;//offset 0x58,tran chnl enbl part D
   unsigned short usRESERVED_0x5A;
   volatile unsigned short usMCBSP_RCERE;//offset 0x5C,recv chnl enbl part E
   unsigned short usRESERVED_0x5E;
   volatile unsigned short usMCBSP_RCERF;//offset 0x60,recv chnl enbl part F
   unsigned short usRESERVED_0x62;
   volatile unsigned short usMCBSP_XCERE;//offset 0x64,tran chnl enbl part E
   unsigned short usRESERVED_0x66;
   volatile unsigned short usMCBSP_XCERF;//offset 0x68,tran chnl enbl part F
   unsigned short usRESERVED_0x6A;
   volatile unsigned short usMCBSP_RCERG;//offset 0x6C,recv chnl enbl part G
   unsigned short usRESERVED_0x6E;
   volatile unsigned short usMCBSP_RCERH;//offset 0x70,recv chnl enbl part H
   unsigned short usRESERVED_0x72;
   volatile unsigned short usMCBSP_XCERG;//offset 0x74,tran chnl enbl part G
   unsigned short usRESERVED_0x76;
   volatile unsigned short usMCBSP_XCERH;//offset 0x78,tran chnl enbl part H
   unsigned short usRESERVED_0x7A;
   volatile unsigned short usMCBSP_REV;//offset 0x7C,revision number
}
OMAP2420_McBSP_REGS,*pMcBSPREGS;


///////////////////////////////////////////////////////////////////////////////
// OMAP McBSP offsets 
///////////////////////////////////////////////////////////////////////////////

#define MCBSP_DRR2_REG_OFFSET     0x00
#define MCBSP_DRR1_REG_OFFSET     0x04
#define MCBSP_DXR2_REG_OFFSET     0x08
#define MCBSP_DXR1_REG_OFFSET     0x0C
#define MCBSP_SPCR2_REG_OFFSET    0x10
#define MCBSP_SPCR1_REG_OFFSET    0x14
#define MCBSP_RCR2_REG_OFFSET     0x18
#define MCBSP_RCR1_REG_OFFSET     0x1C
#define MCBSP_XCR2_REG_OFFSET     0x20
#define MCBSP_XCR1_REG_OFFSET     0x24
#define MCBSP_SRGR2_REG_OFFSET    0x28
#define MCBSP_SRGR1_REG_OFFSET    0x2C
#define MCBSP_MCR2_REG_OFFSET     0x30
#define MCBSP_MCR1_REG_OFFSET     0x34
#define MCBSP_RCERA_REG_OFFSET    0x38
#define MCBSP_RCERB_REG_OFFSET    0x3C
#define MCBSP_XCERA_REG_OFFSET    0x40
#define MCBSP_XCERB_REG_OFFSET    0x44
#define MCBSP_PCR_REG_OFFSET      0x48
#define MCBSP_RCERC_REG_OFFSET    0x4C
#define MCBSP_RCERD_REG_OFFSET    0x50
#define MCBSP_XCERC_REG_OFFSET    0x54
#define MCBSP_XCERD_REG_OFFSET    0x58
#define MCBSP_RCERE_REG_OFFSET    0x5C
#define MCBSP_RCERF_REG_OFFSET    0x60
#define MCBSP_XCERE_REG_OFFSET    0x64
#define MCBSP_XCERF_REG_OFFSET    0x68
#define MCBSP_RCERG_REG_OFFSET    0x6C
#define MCBSP_RCERH_REG_OFFSET    0x70
#define MCBSP_XCERG_REG_OFFSET    0x74
#define MCBSP_XCERH_REG_OFFSET    0x78
#define MCBSP_REV_REG_OFFSET      0x7C

/*************************** Field Definitions *******************************/

#define MCBSP_BIT(ARG)      ((0x01)<<(ARG))

/* Field masks for Register SPCR1 */
#define MCBSP_DLB          MCBSP_BIT(15) /* ALB now in 2430 */
#define MCBSP_RJUST(ARG)   (((ARG) & 0x03) << 13)
#define MCBSP_CLKSTP(ARG)  (((ARG) & 0x03) << 11)
#define MCBSP_DXENA        MCBSP_BIT(7)
#define MCBSP_RINTM(ARG)   (((ARG) & 0x03) << 4)
#define MCBSP_RSYNCERR     MCBSP_BIT(3)
#define MCBSP_RFULL        MCBSP_BIT(2)
#define MCBSP_RRDY         MCBSP_BIT(1)
#define MCBSP_RRST         (0x01)

/* Field masks for Register SPCR2 */
#define MCBSP_FREE         MCBSP_BIT(9)
#define MCBSP_SOFT         MCBSP_BIT(8)
#define MCBSP_FRST         MCBSP_BIT(7)
#define MCBSP_GRST         MCBSP_BIT(6)
#define MCBSP_XINTM(ARG)   (((ARG) & 0x03) << 4)
#define MCBSP_XSYNCERR     MCBSP_BIT(3)
#define MCBSP_XEMPTY       MCBSP_BIT(2)
#define MCBSP_XRDY         MCBSP_BIT(1)
#define MCBSP_XRST         (0x01)

#define MCBSP_WORD_8		0
#define MCBSP_WORD_12		1
#define MCBSP_WORD_16		2
#define MCBSP_WORD_20		3
#define MCBSP_WORD_24		4
#define MCBSP_WORD_32		5

/* Field masks for Register RCR1 */
#define MCBSP_RFRLEN1(ARG)  (((ARG) & 0x7F) << 8)
#define MCBSP_RWDLEN1(ARG)  (((ARG) & 0x07) << 5)

/* Field masks for Register RCR2 */
#define MCBSP_RPHASE        MCBSP_BIT(15)
#define MCBSP_RFRLEN2(ARG)  (((ARG) & 0x7F) << 8)
#define MCBSP_RWDLEN2(ARG)  (((ARG) & 0x07) << 5)
#define MCBSP_RCOMPAND(ARG) (((ARG) & 0x03) << 3)
#define MCBSP_RFIG          MCBSP_BIT(2)
#define MCBSP_RDATDLY(ARG)  ((ARG) & 0x03)

/* Field masks for Register XCR1 */
#define MCBSP_XFRLEN1(ARG)  (((ARG) & 0x7F) << 8)
#define MCBSP_XWDLEN1(ARG)  (((ARG) & 0x07) << 5)

/* Field masks for Register XCR2 */
#define MCBSP_XPHASE        MCBSP_BIT(15)
#define MCBSP_XFRLEN2(ARG)  (((ARG) & 0x7F) << 8)
#define MCBSP_XWDLEN2(ARG)  (((ARG) & 0x07) << 5)
#define MCBSP_XCOMPAND(ARG) (((ARG) & 0x03) << 3)
#define MCBSP_XFIG          MCBSP_BIT(2)
#define MCBSP_XDATDLY(ARG)  ((ARG) & 0x03)

/* Field masks for Register SRGR1 */
#define MCBSP_FWID(ARG)    (((ARG) & 0xFF) << 8)
#define MCBSP_CLKGDV(ARG)  ((ARG) & 0xFF)

/* Field masks for Register SRGR2 */
#define MCBSP_GSYNC        MCBSP_BIT(15)
#define MCBSP_CLKSP        MCBSP_BIT(14)
#define MCBSP_CLKSM        MCBSP_BIT(13)
#define MCBSP_FSGM         MCBSP_BIT(12)
#define MCBSP_FPER(ARG)    ((ARG) & 0xFFF)

/* Field masks for Register MCR1 */

#define MCBSP_RMCME         MCBSP_BIT(9)
#define MCBSP_RPBBLK(ARG)   (((ARG) & 0x03) << 7)
#define MCBSP_RPABLK(ARG)   (((ARG) & 0x03) << 5)
#define MCBSP_RCBLK(ARG)    (((ARG) & 0x07) << 2)
#define MCBSP_RMCM          (0x01)

/* Field masks for Register MCR2 */
#define MCBSP_XMCME         MCBSP_BIT(9)
#define MCBSP_XPBBLK(ARG)   (((ARG) & 0x03) << 7)
#define MCBSP_XPABLK(ARG)   (((ARG) & 0x03) << 5)
#define MCBSP_XCBLK(ARG)    (((ARG) & 0x07) << 2)
#define MCBSP_XMCM(ARG)     ((ARG) & 0x03)

/* Field masks for Register PCR */
#define MCBSP_IDLEEN         MCBSP_BIT(14)
#define MCBSP_XIOEN          MCBSP_BIT(13)
#define MCBSP_RIOEN          MCBSP_BIT(12)
#define MCBSP_FSXM           MCBSP_BIT(11)
#define MCBSP_FSRM           MCBSP_BIT(10)
#define MCBSP_CLKXM          MCBSP_BIT(9)
#define MCBSP_CLKRM          MCBSP_BIT(8)
#define MCBSP_SCLKME         MCBSP_BIT(7)
#define MCBSP_CLKSSTAT       MCBSP_BIT(6)
#define MCBSP_DXSTAT         MCBSP_BIT(5)
#define MCBSP_DRSTAT         MCBSP_BIT(4)
#define MCBSP_FSXP           MCBSP_BIT(3)
#define MCBSP_FSRP           MCBSP_BIT(2)
#define MCBSP_CLKXP          MCBSP_BIT(1)
#define MCBSP_CLKRP          (0x01)

#endif
