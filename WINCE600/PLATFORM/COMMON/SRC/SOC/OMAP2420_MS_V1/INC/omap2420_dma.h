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
//  File:  omap2420_dma.h
//
//  This header file is comprised of DMA module register details defined as
//  structures and macros for configuring and controlling DMA module.
//

#ifndef __OMAP2420_DMA_H
#define __OMAP2420_DMA_H

//------------------------------------------------------------------------------

// Module      Base      
// Name        Address
// DDMA        IOMA + 0xC000 : see OMAP2420_DDMA_REGS_PA in omap2420_base_regs.h 
// CamDMA      0x48052800    : see OMAP2420_CAMDMA_REGS_PA in omap2420_base_regs.h 
// SDMA        0x48056000    : see OMAP2420_SDMA_REGS_PA in omap2420_base_regs.h 

typedef volatile struct {
   UINT32 DMA4_CCR;       //offset 0x0,chnl ctrl
   UINT32 DMA4_CLNK_CTRL; //offset 0x4,chnl link ctrl
   UINT32 DMA4_CICR;      //offset 0x8,chnl intr ctrl
   UINT32 DMA4_CSR;       //offset 0xC,chnl status
   UINT32 DMA4_CSDP;      //offset 0x10,chnl src,dest params
   UINT32 DMA4_CEN;       //offset 0x14,chnl element number
   UINT32 DMA4_CFN;       //offset 0x18,chnl frame number
   UINT32 DMA4_CSSA;      //offset 0x1C,chnl src start addr
   UINT32 DMA4_CDSA;      //offset 0x20,chnl dest start addr
   UINT32 DMA4_CSEI;      //offset 0x24,chnl src element index
   UINT32 DMA4_CSFI;      //offset 0x28,chnl src frame index
   UINT32 DMA4_CDEI;      //offset 0x2C,chnl destination element index
   UINT32 DMA4_CDFI;      //offset 0x30,chnl destination frame index
   UINT32 DMA4_CSAC;      //offset 0x34,chnl src address counter
   UINT32 DMA4_CDAC;      //offset 0x38,chnl dest address counter
   UINT32 DMA4_CCEN;      //offset 0x3C,chnl cur trans element no
   UINT32 DMA4_CCFN;      //offset 0x40,chnl cur trans frame no
   //DMA4_COLOR register is valid only for SDMA module. It is a reserved section
   //in other modules(CAMDMA and DDMA) 
   UINT32 DMA4_COLOR;     //offset 0x44,chnl DMA color key/solid color
   UINT32 ulRESERVED_1[6];           // 48-60 Reserved
}  
OMAP2420_DMA_REGS;

typedef volatile struct {
   UINT32 DMA4_REVISION;              //offset 0x0, Revision code
   UINT32 ulRESERVED_0x04;
   UINT32 DMA4_IRQSTATUS_L0; //offset 0x08,intr status over line L0
   UINT32 DMA4_IRQSTATUS_L1; //offset 0x0C,intr status over line L1
   UINT32 DMA4_IRQSTATUS_L2; //offset 0x10,intr status over line L2
   UINT32 DMA4_IRQSTATUS_L3; //offset 0x14,intr status over line L3
   UINT32 DMA4_IRQENABLE_L0; //offset 0x18,intr enable over line L0
   UINT32 DMA4_IRQENABLE_L1; //offset 0x1C,intr enable over line L1
   UINT32 DMA4_IRQENABLE_L2; //offset 0x20,intr enable over line L2
   UINT32 DMA4_IRQENABLE_L3; //offset 0x24,intr enable over line L3
   UINT32 DMA4_SYSSTATUS;    //offset 0x28,module status
   UINT32 DMA4_OCP_SYSCONFIG;//offset 0x2C,OCP i/f params control
   UINT32 ulRESERVED_0x30[13];
   UINT32 DMA4_CAPS_0;       //offset 0x64,DMA capabilities reg 0 LSW
   UINT32 ulRESERVED_0x68;
   UINT32 DMA4_CAPS_2;       //offset 0x6C,DMA capabilities reg 2
   UINT32 DMA4_CAPS_3;       //offset 0x70,DMA capabilities reg 3
   UINT32 DMA4_CAPS_4;       //offset 0x74,DMA capabilities reg 4
   UINT32 DMA4_GCR;          //offset 0x78,chnl DMA global register
   UINT32 ulRESERVED_0x7C;
   OMAP2420_DMA_REGS CHNL_CTRL[24];//offset 0x80-0x920,chnl
}
OMAP2420_DDMA_REGS;

typedef volatile struct {
   UINT32 DMA4_REVISION;              //offset 0x0, Revision code
   UINT32 ulRESERVED_0x04;
   UINT32 DMA4_IRQSTATUS_L0; //offset 0x08,intr status over line L0
   UINT32 DMA4_IRQSTATUS_L1; //offset 0x0C,intr status over line L1
   UINT32 DMA4_IRQSTATUS_L2; //offset 0x10,intr status over line L2
   UINT32 DMA4_IRQSTATUS_L3; //offset 0x14,intr status over line L3
   UINT32 DMA4_IRQENABLE_L0; //offset 0x18,intr enable over line L0
   UINT32 DMA4_IRQENABLE_L1; //offset 0x1C,intr enable over line L1
   UINT32 DMA4_IRQENABLE_L2; //offset 0x20,intr enable over line L2
   UINT32 DMA4_IRQENABLE_L3; //offset 0x24,intr enable over line L3
   UINT32 DMA4_SYSSTATUS;    //offset 0x28,module status
   UINT32 DMA4_OCP_SYSCONFIG;//offset 0x2C,OCP i/f params control
   UINT32 ulRESERVED_0x30[13];
   UINT32 DMA4_CAPS_0;       //offset 0x64,DMA capabilities reg 0 LSW
   UINT32 ulRESERVED_0x68;
   UINT32 DMA4_CAPS_2;       //offset 0x6C,DMA capabilities reg 2
   UINT32 DMA4_CAPS_3;       //offset 0x70,DMA capabilities reg 3
   UINT32 DMA4_CAPS_4;       //offset 0x74,DMA capabilities reg 4
   UINT32 DMA4_GCR;          //offset 0x78,chnl DMA global register
   UINT32 ulRESERVED_0x7C;
   OMAP2420_DMA_REGS CHNL_CTRL[32];//offset 0x80-0x920,chnl
}
OMAP2420_SDMA_REGS;

typedef volatile struct {
   UINT32 DMA4_REVISION;              //offset 0x0, Revision code
   UINT32 ulRESERVED_0x04;
   UINT32 DMA4_IRQSTATUS_L0; //offset 0x08,intr status over line L0
   UINT32 DMA4_IRQSTATUS_L1; //offset 0x0C,intr status over line L1
   UINT32 DMA4_IRQSTATUS_L2; //offset 0x10,intr status over line L2
   UINT32 DMA4_IRQSTATUS_L3; //offset 0x14,intr status over line L3
   UINT32 DMA4_IRQENABLE_L0; //offset 0x18,intr enable over line L0
   UINT32 DMA4_IRQENABLE_L1; //offset 0x1C,intr enable over line L1
   UINT32 DMA4_IRQENABLE_L2; //offset 0x20,intr enable over line L2
   UINT32 DMA4_IRQENABLE_L3; //offset 0x24,intr enable over line L3
   UINT32 DMA4_SYSSTATUS;    //offset 0x28,module status
   UINT32 DMA4_OCP_SYSCONFIG;//offset 0x2C,OCP i/f params control
   UINT32 ulRESERVED_0x30[13];
   UINT32 DMA4_CAPS_0;       //offset 0x64,DMA capabilities reg 0 LSW
   UINT32 DMA4_ulRESERVED_0x68;
   UINT32 DMA4_CAPS_2;       //offset 0x6C,DMA capabilities reg 2
   UINT32 DMA4_CAPS_3;       //offset 0x70,DMA capabilities reg 3
   UINT32 DMA4_CAPS_4;       //offset 0x74,DMA capabilities reg 4
   UINT32 DMA4_GCR;          //offset 0x78,chnl DMA global register
   UINT32 ulRESERVED_0x7C;
   OMAP2420_DMA_REGS CHNL_CTRL[4];//offset 0x80-0x920,chnl
}
OMAP2420_CAMDMA_REGS;


//------------------------------------------------------------------------------
// System DMA request mappings

#define SDMA_REQ_XTI					1
#define SDMA_REQ_EXT_DMA_REQ_0			2
#define SDMA_REQ_EXT_DMA_REQ_1			3
#define SDMA_REQ_GPMC					4
#define SDMA_REQ_GFX					5
#define SDMA_REQ_DSS					6
#define SDMA_REQ_VLYNQ_TX				7
#define SDMA_REQ_CWT					8
#define SDMA_REQ_AES_TX					9
#define SDMA_REQ_AES_RX					10
#define SDMA_REQ_DES_TX					11
#define SDMA_REQ_DES_RX					12
#define SDMA_REQ_SHA1MD5_RX				13
#define SDMA_REQ_UNCONNECTED_14			14
#define SDMA_REQ_UNCONNECTED_15			15
#define SDMA_REQ_UNCONNECTED_16			16
#define SDMA_REQ_EAC_AC_RD				17
#define SDMA_REQ_EAC_AC_WR				18
#define SDMA_REQ_EAC_MB_UL_RD			19
#define SDMA_REQ_EAC_MB_UL_WR			20
#define SDMA_REQ_EAC_MD_DL_RD			21
#define SDMA_REQ_EAC_MD_DL_WR			22
#define SDMA_REQ_EAC_BT_UL_RD			23
#define SDMA_REQ_EAC_BT_UL_WR			24
#define SDMA_REQ_EAC_BT_DL_RD			25
#define SDMA_REQ_EAC_BT_DL_WR			26
#define SDMA_REQ_I2C1_TX				27
#define SDMA_REQ_I2C1_RX				28
#define SDMA_REQ_I2C2_TX				29
#define SDMA_REQ_I2C2_RX				30
#define SDMA_REQ_MCBSP1_TX				31
#define SDMA_REQ_MCBSP1_RX				32
#define SDMA_REQ_MCBSP2_TX				33
#define SDMA_REQ_MCBSP2_RX				34
#define SDMA_REQ_SPI1_TX0				35
#define SDMA_REQ_SPI1_RX0				36
#define SDMA_REQ_SPI1_TX1				37
#define SDMA_REQ_SPI1_RX1				38
#define SDMA_REQ_SPI1_TX2				39
#define SDMA_REQ_SPI1_RX2				40
#define SDMA_REQ_SPI1_TX3				41
#define SDMA_REQ_SPI1_RX3				42
#define SDMA_REQ_SPI2_TX0				43
#define SDMA_REQ_SPI2_RX0				44
#define SDMA_REQ_SPI2_TX1				45
#define SDMA_REQ_SPI2_RX1				46
#define SDMA_REQ_UNCONNECTED_47			47
#define SDMA_REQ_UNCONNECTED_48			48
#define SDMA_REQ_UART1_TX				49
#define SDMA_REQ_UART1_RX				50
#define SDMA_REQ_UART2_TX				51
#define SDMA_REQ_UART2_RX				52
#define SDMA_REQ_UART3_TX				53
#define SDMA_REQ_UART3_RX				54
#define SDMA_REQ_USB0_TX0				55
#define SDMA_REQ_USB0_RX0				56
#define SDMA_REQ_USB0_TX1				57
#define SDMA_REQ_USB0_RX1				58
#define SDMA_REQ_USB0_TX2				59
#define SDMA_REQ_USB0_RX2				60
#define SDMA_REQ_MMC_TX					61
#define SDMA_REQ_MMC_RX					62
#define SDMA_REQ_MS						63
#define SDMA_REQ_UNCONNECTED_64			64

//------------------------------------------------------------------------------
// DSP DMA request mappings

#define DDMA_REQ_XTI					1
#define DDMA_REQ_CWT					2
#define DDMA_REQ_EAC_AC_RD				3
#define DDMA_REQ_EAC_AC_WR				4
#define DDMA_REQ_EAC_MB_UL_RD			5
#define DDMA_REQ_EAC_MB_UL_WR			6
#define DDMA_REQ_EAC_MD_DL_RD			7
#define DDMA_REQ_EAC_MD_DL_WR			8
#define DDMA_REQ_EAC_BT_UL_RD			9
#define DDMA_REQ_EAC_BT_UL_WR			10
#define DDMA_REQ_EAC_BT_DL_RD			11
#define DDMA_REQ_EAC_BT_DL_WR			12
#define DDMA_REQ_MCBSP1_TX				13
#define DDMA_REQ_MCBSP1_RX				14
#define DDMA_REQ_MCBSP2_TX				15
#define DDMA_REQ_MCBSP2_RX				16
#define DDMA_REQ_UART3_TX				17
#define DDMA_REQ_UART3_RX				18
#define DDMA_REQ_UNCONNECTED_19			19
#define DDMA_REQ_UNCONNECTED_20			20
#define DDMA_REQ_UNCONNECTED_21			21
#define DDMA_REQ_UNCONNECTED_22			22
#define DDMA_REQ_UNCONNECTED_23			23
#define DDMA_REQ_UNCONNECTED_24			24
#define DDMA_REQ_UNCONNECTED_25			25
#define DDMA_REQ_UNCONNECTED_26			26
#define DDMA_REQ_UNCONNECTED_27			27
#define DDMA_REQ_UNCONNECTED_28			28
#define DDMA_REQ_UNCONNECTED_29			29
#define DDMA_REQ_UNCONNECTED_30			30
#define DDMA_REQ_UNCONNECTED_31			31
#define DDMA_REQ_UNCONNECTED_32			32

//------------------------------------------------------------------------------
// CCR register fields

#define DMA_CCR_BUFFERING_DISABLE           (1 << 25)
#define DMA_CCR_SEL_SRC_DST_SYNCH           (1 << 24)
#define DMA_CCR_PREFETCH                    (1 << 23)
#define DMA_CCR_SUPERVISOR                  (1 << 22)
#define DMA_CCR_SECURE                      (1 << 21)
#define DMA_CCR_BS                          (1 << 18)
#define DMA_CCR_TRANSPARENT_COPY_ENABLE     (1 << 17)
#define DMA_CCR_CONST_FILL_ENABLE           (1 << 16)

#define DMA_CCR_DST_AMODE_MASK              (3 << 14)
#define DMA_CCR_DST_AMODE_DOUBLE            (3 << 14)
#define DMA_CCR_DST_AMODE_SINGLE            (2 << 14)
#define DMA_CCR_DST_AMODE_POST_INC          (1 << 14)
#define DMA_CCR_DST_AMODE_CONST             (0 << 14)

#define DMA_CCR_SRC_AMODE_MASK              (3 << 12)
#define DMA_CCR_SRC_AMODE_DOUBLE            (3 << 12)
#define DMA_CCR_SRC_AMODE_SINGLE            (2 << 12)
#define DMA_CCR_SRC_AMODE_POST_INC          (1 << 12)
#define DMA_CCR_SRC_AMODE_CONST             (0 << 12)

#define DMA_CCR_WR_ACTIVE                   (1 << 10)
#define DMA_CCR_RD_ACTIVE                   (1 << 9)
#define DMA_CCR_SUSPEND_SENSITIVE           (1 << 8)
#define DMA_CCR_ENABLE                      (1 << 7)
#define DMA_CCR_PRIO                        (1 << 6)
#define DMA_CCR_FS                          (1 << 5)

#define DMA_CCR_SYNC(req)                   (((req) & 0x1F) | (((DWORD)(req) & 0x60) << 14))

//------------------------------------------------------------------------------
// CICR register fields

#define DMA_CICR_MISALIGNED_ERR_IE          (1 << 11)
#define DMA_CICR_SUPERVISOR_ERR_IE          (1 << 10)
#define DMA_CICR_SECURE_ERR_IE              (1 << 9)
#define DMA_CICR_TRANS_ERR_IE               (1 << 8)
#define DMA_CICR_PKT_IE                     (1 << 7)
#define DMA_CICR_BLOCK_IE                   (1 << 5)
#define DMA_CICR_LAST_IE                    (1 << 4)
#define DMA_CICR_FRAME_IE                   (1 << 3)
#define DMA_CICR_HALF_IE                    (1 << 2)
#define DMA_CICR_DROP_IE                    (1 << 1)

//------------------------------------------------------------------------------
// CSR register fields

#define DMA_CSR_MISALIGNED_ERR              (1 << 11)
#define DMA_CSR_SUPERVISOR_ERR              (1 << 10)
#define DMA_CSR_SECURE_ERR                  (1 << 9)
#define DMA_CSR_TRANS_ERR                   (1 << 8)
#define DMA_CSR_PKT                         (1 << 7)
#define DMA_CSR_sync                        (1 << 6)
#define DMA_CSR_BLOCK                       (1 << 5)
#define DMA_CSR_LAST                        (1 << 4)
#define DMA_CSR_FRAME                       (1 << 3)
#define DMA_CSR_HALF                        (1 << 2)
#define DMA_CSR_DROP                        (1 << 1)

//------------------------------------------------------------------------------
// CSR register fields

#define DMA_CSDP_SRC_ENDIAN_BIG                 0x00200000
#define DMA_CSDP_SRC_ENDIAN_LOCK                0x00100000
#define DMA_CSDP_DST_ENDIAN_BIG                 0x00080000
#define DMA_CSDP_DST_ENDIAN_LOCK                0x00040000
#define DMA_CSDP_WRITE_MODE_MASK                0x00030000
#define DMA_CSDP_WRITE_MODE_NOPOST              0x00000000
#define DMA_CSDP_WRITE_MODE_POSTED              0x00010000
#define DMA_CSDP_WRITE_MODE_POSTED_EXCEPT       0x00020000
#define DMA_CSDP_DST_BURST_MASK                 0x0000C000
#define DMA_CSDP_DST_BURST_NONE                 0x00000000
#define DMA_CSDP_DST_BURST_16BYTES_4x32_2x64    0x00004000
#define DMA_CSDP_DST_BURST_32BYTES_8x32_4x64    0x00008000
#define DMA_CSDP_DST_BURST_64BYTES_16x32_8x64   0x0000C000
#define DMA_CSDP_DST_PACKED                     0x00002000
#define DMA_CSDP_WR_ADDR_TRSLT_MASK             0x00001E00
#define DMA_CSDP_SRC_BURST_MASK                 0x00000180
#define DMA_CSDP_SRC_BURST_NONE                 0x00000000
#define DMA_CSDP_SRC_BURST_16BYTES_4x32_2x64    0x00000080
#define DMA_CSDP_SRC_BURST_32BYTES_8x32_4x64    0x00000100
#define DMA_CSDP_SRC_BURST_64BYTES_16x32_8x64   0x00000180
#define DMA_CSDP_SRC_PACKED                     0x00000040
#define DMA_CSDP_RD_ADDR_TRSLT_MASK             0x0000003C
#define DMA_CSDP_DATATYPE_MASK                  0x00000003
#define DMA_CSDP_DATATYPE_8BIT                  0x00000000
#define DMA_CSDP_DATATYPE_16BIT                 0x00000001
#define DMA_CSDP_DATATYPE_32BIT                 0x00000002


//------------------------------------------------------------------------------
// CLNK_CTRL register fields

#define DMA_CLNK_CTRL_ENABLE_LINK           (1 << 15)

//------------------------------------------------------------------------------

#define DMA_LCD_CSDP_B2_BURST_MASK          (3 << 14)
#define DMA_LCD_CSDP_B2_BURST_4             (2 << 14)
#define DMA_LCD_CSDP_B2_BURST_NONE          (0 << 14)

#define DMA_LCD_CSDP_B2_PACK                (1 << 13)

#define DMA_LCD_CSDP_B2_DATA_TYPE_MASK      (3 << 11)
#define DMA_LCD_CSDP_B2_DATA_TYPE_S32       (2 << 11)
#define DMA_LCD_CSDP_B2_DATA_TYPE_S16       (1 << 11)
#define DMA_LCD_CSDP_B2_DATA_TYPE_S8        (0 << 11)

#define DMA_LCD_CSDP_B1_BURST_MASK          (3 << 7)
#define DMA_LCD_CSDP_B1_BURST_4             (2 << 7)
#define DMA_LCD_CSDP_B1_BURST_NONE          (0 << 7)

#define DMA_LCD_CSDP_B1_PACK                (1 << 6)

#define DMA_LCD_CSDP_B1_DATA_TYPE_MASK      (3 << 0)
#define DMA_LCD_CSDP_B1_DATA_TYPE_S32       (2 << 0)
#define DMA_LCD_CSDP_B1_DATA_TYPE_S16       (1 << 0)
#define DMA_LCD_CSDP_B1_DATA_TYPE_S8        (0 << 0)

//------------------------------------------------------------------------------

#define DMA_LCD_CCR_B2_AMODE_MASK           (3 << 14)
#define DMA_LCD_CCR_B2_AMODE_DOUBLE         (3 << 14)
#define DMA_LCD_CCR_B2_AMODE_SINGLE         (2 << 14)
#define DMA_LCD_CCR_B2_AMODE_INC            (1 << 14)

#define DMA_LCD_CCR_B1_AMODE_MASK           (3 << 12)
#define DMA_LCD_CCR_B1_AMODE_DOUBLE         (3 << 12)
#define DMA_LCD_CCR_B1_AMODE_SINGLE         (2 << 12)
#define DMA_LCD_CCR_B1_AMODE_INC            (1 << 12)

#define DMA_LCD_CCR_END_PROG                (1 << 11)
#define DMA_LCD_CCR_OMAP32                  (1 << 10)
#define DMA_LCD_CCR_REPEAT                  (1 << 9)
#define DMA_LCD_CCR_AUTO_INIT               (1 << 8)
#define DMA_LCD_CCR_EN                      (1 << 7)
#define DMA_LCD_CCR_PRIO                    (1 << 6)
#define DMA_LCD_CCR_SYNC_PR                 (1 << 4)

//------------------------------------------------------------------------------

#define DMA_LCD_CTRL_EXTERNAL_LCD           (1 << 8)

#define DMA_LCD_CTRL_SOURCE_PORT_MASK       (3 << 6)
#define DMA_LCD_CTRL_SOURCE_PORT_SDRAM      (0 << 6)
#define DMA_LCD_CTRL_SOURCE_PORT_OCP_T1     (1 << 6)
#define DMA_LCD_CTRL_SOURCE_PORT_OCP_T2     (2 << 6)

#define DMA_LCD_CTRL_BLOCK_MODE             (1 << 0)

//------------------------------------------------------------------------------

#define DMA_LCD_LCH_CTRL_TYPE_D             (4 << 0)



#endif
