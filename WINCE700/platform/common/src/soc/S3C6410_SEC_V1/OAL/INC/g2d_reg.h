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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
// Copyright (c) Samsung Electronics. Co. LTD.  All rights reserved.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
--*/
/**
*    @file        g2d_reg.h
*    @brief    Defines the FIMGSE-2D Graphics Accerlerator's register layout and definitions.
*    @author    Jiwon Kim
*    
*    @note This version is made for S3C6410
*    @note IP version is v2.0
*/

#ifndef __G2D_REG_H__
#define __G2D_REG_H__

#ifdef __cplusplus
extern "C"
{
#endif


//------------------------------------------------------------------------------
//  Type: G2D_REG
//
//  2D Graphics Acclerator registers. This register bank is located by the constant
//  <CPU>_BASE_REG_XX_2DGRAPHICS in the configuration file <cpu>_base_regs.h.
//

typedef struct _reg_g2d_v2
{
    // General Registers
    UINT32 CONTROL;                        //       (G2D_BASE+0x00)
    UINT32 INTEN;                            //    (G2D_BASE+0x04)
    UINT32 FIFO_INTC;            //    (G2D_BASE+0x08)
    UINT32 INTC_PEND;                    //        (G2D_BASE+0x0c)
    UINT32 FIFO_STATUS;        //    (G2D_BASE+0x10)
    UINT32 PAD1[0x3B];

    // Command Registers
    UINT32 CMDR0;                            //    (G2D_BASE+0x100)
    UINT32 CMDR1;                            //    (G2D_BASE+0x104)
    UINT32 CMDR2;                            //    (G2D_BASE+0x108)
    UINT32 CMDR3;                            //    (G2D_BASE+0x10c)
    UINT32 CMDR4;                            //    (G2D_BASE+0x110)
    UINT32 CMDR5;                            //    (G2D_BASE+0x114)
    UINT32 CMDR6;                            //    (G2D_BASE+0x118)
    UINT32 CMDR7;                            //    (G2D_BASE+0x11c)

    UINT32 PAD2[0x38];
    // Common Resource Registers
    UINT32 SRC_RES;                        //    (G2D_BASE+0x200)
    UINT32 SRC_HORI_RES;            //    (G2D_BASE+0x204)
    UINT32 SRC_VERT_RES;            //    (G2D_BASE+0x208)
    UINT32 PAD3[1];
    UINT32 SC_RES;                        //        (G2D_BASE+0x210)
    UINT32 SC_HORI_RES;                //    (G2D_BASE+0x214)
    UINT32 SC_VERT_RES;                //       (G2D_BASE+0x218)
    UINT32 PAD4[1];
    UINT32 CW_LEFT_TOP;                //    (G2D_BASE+0x220)
    UINT32 CW_LEFT_TOP_X;            //       (G2D_BASE+0x224)
    UINT32 CW_LEFT_TOP_Y;            //       (G2D_BASE+0x228)
    UINT32 PAD5[1];
    UINT32 CW_RIGHT_BOTTOM;        //    (G2D_BASE+0x230)
    UINT32 CW_RIGHT_BOTTOM_X;    //       (G2D_BASE+0x234)
    UINT32 CW_RIGHT_BOTTOM_Y; //       (G2D_BASE+0x238)
    UINT32 PAD6[0x31];
    UINT32 COORD0;                        //       (G2D_BASE+0x300)
    UINT32 COORD0_X;                    //    (G2D_BASE+0x304)
    UINT32 COORD0_Y;                    //    (G2D_BASE+0x308)
    UINT32 PAD7[1];
    UINT32 COORD1;                        //       (G2D_BASE+0x310)
    UINT32 COORD1_X;                    //    (G2D_BASE+0x314)
    UINT32 COORD1_Y;                    //    (G2D_BASE+0x318)
    UINT32 PAD8[1];
    UINT32 COORD2;                        //       (G2D_BASE+0x320)
    UINT32 COORD2_X;                    //    (G2D_BASE+0x324)
    UINT32 COORD2_Y;                    //    (G2D_BASE+0x328)
    UINT32 PAD9[1];
    UINT32 COORD3;                        //       (G2D_BASE+0x330)
    UINT32 COORD3_X;                    //    (G2D_BASE+0x334)
    UINT32 COORD3_Y;                    //    (G2D_BASE+0x338)
    UINT32 PAD10[1];
    UINT32 ROT_OC;                        //       (G2D_BASE+0x340)
    UINT32 ROT_OC_X;                    //    (G2D_BASE+0x344)
    UINT32 ROT_OC_Y;                    //    (G2D_BASE+0x348)
    UINT32 ROT_MODE;                    //    (G2D_BASE+0x34c)
    UINT32 ENDIAN;                        //       (G2D_BASE+0x350)
    UINT32 PAD11[0x2b];
    UINT32 X_INCR;                        //    (G2D_BASE+0x400)
    UINT32 Y_INCR;                        //    (G2D_BASE+0x404)
    UINT32 PAD12[2];
    UINT32 ROP;                                //       (G2D_BASE+0x410)
    UINT32 PAD13[3];
    UINT32 ALPHA;                            //    (G2D_BASE+0x420)
    UINT32 PAD14[0x37];

    UINT32 FG_COLOR;                    //    (G2D_BASE+0x500)
    UINT32 BG_COLOR;                    //    (G2D_BASE+0x504)
    UINT32 BS_COLOR;                    //    (G2D_BASE+0x508)
    UINT32 PAD141[0x1];    
    UINT32 SRC_COLOR_MODE;        //        (G2D_BASE+0x510)
    UINT32 DST_COLOR_MODE;        //        (G2D_BASE+0x514)
    UINT32 PAD15[0x3a];
    
    UINT32 PATTERN_ADDR[0x20];//    (G2D_BASE+0x600~0x67c)
    UINT32 PAD16[0x20];
    UINT32 PAT_OFF_XY;                //       (G2D_BASE+0x700)
    UINT32 PAT_OFF_X;                    //    (G2D_BASE+0x704)
    UINT32 PAT_OFF_Y;                    //    (G2D_BASE+0x708)                                                                                                                                                                                                      
    UINT32 PAD17[5];
    UINT32 COLORKEY_CNTL;            //    (G2D_BASE+0x720)    
    UINT32 COLORKEY_DR_MIN;        //    (G2D_BASE+0x724)
    UINT32 COLORKEY_DR_MAX;        //    (G2D_BASE+0x728)
    
    UINT32 PAD18[1];
    UINT32 SRC_BASE_ADDR;            //    (G2D_BASE+0x730)
    UINT32 DST_BASE_ADDR;            //    (G2D_BASE+0x734)    

} G2D_REG, *PG2D_REG;

 
#ifdef __cplusplus
}
#endif


#endif /*__G2D_REG_H__*/
