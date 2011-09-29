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
//  File:  omap5912_dma.h
//
#ifndef __OMAP5912_DMA_H
#define __OMAP5912_DMA_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT16 GCR;             // 0000
	UINT16 UNUSED004;       // 0002
    UINT16 GSCR;            // 0004
    UINT16 UNUSED006;       // 0006
    UINT16 GRST;            // 0008
    UINT16 UNUSED00A[28];   // 000A
    UINT16 HW_ID;           // 0042
    UINT16 PCH2_ID;         // 0044
    UINT16 PCH0_ID;         // 0046
    UINT16 PCH1_ID;         // 0048
    UINT16 PCHG_ID;         // 004A
    UINT16 PCHD_ID;         // 004C
    UINT16 CAPS_0_U;        // 004E
    UINT16 CAPS_0_L;        // 0050
    UINT16 CAPS_1_U;        // 0052
    UINT16 CAPS_1_L;        // 0054
    UINT16 CAPS_2;          // 0056
    UINT16 CAPS_3;          // 0058
    UINT16 CAPS_4;          // 005A
	UINT16 UNUSED05C[2];    // 005C
    UINT16 PCH2_SR;         // 0060
    UINT16 UNUSED062[15];   // 0062
    UINT16 PCH0_SR;         // 0080
    UINT16 PCH1_SR;         // 0082
    UINT16 UNUSED084[30];   // 0084
    UINT16 PCHD_SR;         // 00C0
} OMAP5912_DMA_REGS;

typedef volatile struct {
    UINT16 CSDP;            // 0000
    UINT16 CCR;             // 0002
    UINT16 CICR;            // 0004
    UINT16 CSR;             // 0006
    UINT16 CSSA_L;          // 0008
    UINT16 CSSA_U;          // 000A
    UINT16 CDSA_L;          // 000C
    UINT16 CDSA_U;          // 000E
    UINT16 CEN;             // 0010
    UINT16 CFN;             // 0012
    UINT16 CSFI;            // 0014
    UINT16 CSEI;            // 0016
    UINT16 CSAC;            // 0018
    UINT16 CDAC;            // 001A
    UINT16 CDEI;            // 001C
    UINT16 CDFI;            // 001E
    UINT16 COLOR_L;         // 0020
    UINT16 COLOR_U;         // 0022
    UINT16 CCR2;            // 0024
    UINT16 UNUSED026;       // 0026
    UINT16 CLNK_CTRL;       // 0028
    UINT16 LCH_CTRL;        // 002A
    UINT16 UNUSED02C;       // 002C
    UINT16 UNUSED02E;       // 002E
    UINT16 UNUSED030;       // 0030
    UINT16 UNUSED032;       // 0032
    UINT16 CDDEN_L;         // 0034
    UINT16 CDDEN_U;         // 0036
    UINT16 CEXSFI_U;        // 0038
    UINT16 CEXSFI_L;        // 003A
    UINT16 CEXDFI_U;        // 003C
    UINT16 CEXDFI_L;        // 003E
} OMAP5912_DMA_LC_REGS;

typedef volatile struct {
    UINT16 CSDP;            // 0000
    UINT16 CCR;             // 0002
    UINT16 CTRL;            // 0004
    UINT16 UNUSED006;       // 0006
    UINT16 TOP_B1_L;        // 0008
    UINT16 TOP_B1_U;        // 000A
    UINT16 BOT_B1_L;        // 000C
    UINT16 BOT_B1_U;        // 000E
    UINT16 TOP_B2_L;        // 0010
    UINT16 TOP_B2_U;        // 0012
    UINT16 BOT_B2_L;        // 0014
    UINT16 BOT_B2_U;        // 0016
    UINT16 SRC_EI_B1;       // 0018
    UINT16 SRC_FI_B1_L;     // 001A
    UINT16 SRC_EI_B2;       // 001C
    UINT16 SRC_FI_B2_L;     // 001E
    UINT16 SRC_EN_B1;       // 0020
    UINT16 SRC_EN_B2;       // 0022
    UINT16 SRC_FN_B1;       // 0024
    UINT16 SRC_FN_B2;       // 0026
    UINT16 UNUSED028;       // 0028
    UINT16 LCH_CTRL;        // 002A
    UINT16 UNUSED02C[4];    // 002C
    UINT16 SRC_FI_B1_U;     // 0034
    UINT16 SRC_FI_B2_U;     // 0036
} OMAP5912_DMA_LCD_REGS;

typedef struct {
    OMAP5912_DMA_LC_REGS  Channel[16];
    OMAP5912_DMA_REGS     Global;
    UCHAR                Reserved[3902];
    OMAP5912_DMA_LCD_REGS LcdChannel;
}   OMAP5912_GDMA_REGS;

//------------------------------------------------------------------------------
// DMA request mapping.

#define OMAP5912_REQ_MCSI_BT_TX              1
#define OMAP5912_REQ_MCSI_BT_RX              2
#define OMAP5912_REQ_I2C_RX                  3
#define OMAP5912_REQ_I2C_TX                  4
#define OMAP5912_REQ_EXTREQ0                 5
#define OMAP5912_REQ_EXTREQ1                 6
#define OMAP5912_REQ_UWIRE_TX                7
#define OMAP5912_REQ_MCBSP1_TX               8
#define OMAP5912_REQ_MCBSP1_RX               9
#define OMAP5912_REQ_MCBSP3_TX               10

#define OMAP5912_REQ_MCBSP3_RX               11
#define OMAP5912_REQ_UART1_TX                12
#define OMAP5912_REQ_UART1_RX                13
#define OMAP5912_REQ_UART2_TX                14
#define OMAP5912_REQ_UART2_RX                15
#define OMAP5912_REQ_MCBSP2_TX               16
#define OMAP5912_REQ_MCBSP2_RX               17
#define OMAP5912_REQ_UART3_TX                18
#define OMAP5912_REQ_UART3_RX                19
#define OMAP5912_REQ_CAMERA_RX               20

#define OMAP5912_REQ_SDMMC_TX                21
#define OMAP5912_REQ_SDMMC_RX                22
#define OMAP5912_REQ_UNUSED_22               23
#define OMAP5912_REQ_LCD_LINE                24
#define OMAP5912_REQ_UNUSED_25               25
#define OMAP5912_REQ_USB_W2FC_RX0            26
#define OMAP5912_REQ_USB_W2FC_RX1            27
#define OMAP5912_REQ_USB_W2FC_RX2            28
#define OMAP5912_REQ_USB_W2FC_TX0            29
#define OMAP5912_REQ_USB_W2FC_TX1            30

#define OMAP5912_REQ_USB_W2FC_TX2            31
#define OMAP5912_REQ_UNUSED_31               32
#define OMAP5912_REQ_SPI_TX                  33
#define OMAP5912_REQ_SPI_RX                  34
#define OMAP5912_REQ_UNUSED_34               35
#define OMAP5912_REQ_UNUSED_35               36
#define OMAP5912_REQ_UNUSED_36               37
#define OMAP5912_REQ_CMT_APE_TX0             38
#define OMAP5912_REQ_CMT_APE_RV0             39
#define OMAP5912_REQ_CMT_APE_TX1             40

#define OMAP5912_REQ_CMT_APE_RV1             41
#define OMAP5912_REQ_CMT_APE_TX2             42
#define OMAP5912_REQ_CMT_APE_RV2             43
#define OMAP5912_REQ_CMT_APE_TX3             44
#define OMAP5912_REQ_CMT_APE_RV3             45
#define OMAP5912_REQ_CMT_APE_TX4             46
#define OMAP5912_REQ_CMT_APE_RV4             47
#define OMAP5912_REQ_CMT_APE_TX5             48
#define OMAP5912_REQ_CMT_APE_RV5             49
#define OMAP5912_REQ_CMT_APE_TX6             50

#define OMAP5912_REQ_CMT_APE_RV6             51
#define OMAP5912_REQ_CMT_APE_TX7             52
#define OMAP5912_REQ_CMT_APE_RV7             53
#define OMAP5912_REQ_SDMMC2_TX               54
#define OMAP5912_REQ_SDMMC2_RX               55
#define OMAP5912_REQ_UNUSED_55               56
#define OMAP5912_REQ_UNUSED_56               57
#define OMAP5912_REQ_UNUSED_57               58
#define OMAP5912_REQ_UNUSED_58               59
#define OMAP5912_REQ_UNUSED_59               60

#define OMAP5912_REQ_UNUSED_60               61
#define OMAP5912_REQ_UNUSED_61               62
#define OMAP5912_REQ_UNUSED_62               63
#define OMAP5912_REQ_UNUSED_63               64


//------------------------------------------------------------------------------

#define OMAP5912_CSDP_PORT_TYPE_EMIFF        0               
#define OMAP5912_CSDP_PORT_TYPE_EMIFS        1               
#define OMAP5912_CSDP_PORT_TYPE_OCP_T1       2               
#define OMAP5912_CSDP_PORT_TYPE_TIPB         3               
#define OMAP5912_CSDP_PORT_TYPE_OCP_T2       4               
#define OMAP5912_CSDP_PORT_TYPE_MPUI         5     

//------------------------------------------------------------------------------

#define DMA_CSDP_DST_BURST_MASK             (3 << 14)
#define DMA_CSDP_DST_BURST_8                (3 << 14)
#define DMA_CSDP_DST_BURST_4                (2 << 14)
#define DMA_CSDP_DST_BURST_NONE             (0 << 14)

#define DMA_CSDP_DST_PACK                   (1 << 13)

#define DMA_CSDP_DST_MASK                   (0xF << 9)
#define DMA_CSDP_DST_SDRAM                  (0 << 9)
#define DMA_CSDP_DST_EMIF                   (1 << 9)
#define DMA_CSDP_DST_IMIF                   (2 << 9)
#define DMA_CSDP_DST_TIPB                   (3 << 9)
#define DMA_CSDP_DST_LOCAL                  (4 << 9)
#define DMA_CSDP_DST_API                    (5 << 9)

#define DMA_CSDP_SRC_BURST_MASK             (3 << 7)
#define DMA_CSDP_SRC_BURST_8                (3 << 7)
#define DMA_CSDP_SRC_BURST_4                (2 << 7)
#define DMA_CSDP_SRC_BURST_NONE             (0 << 7)

#define DMA_CSDP_SRC_PACK                   (1 << 6)

#define DMA_CSDP_SRC_MASK                   (0xF << 2)
#define DMA_CSDP_SRC_SDRAM                  (0 << 2)
#define DMA_CSDP_SRC_EMIF                   (1 << 2)
#define DMA_CSDP_SRC_IMIF                   (2 << 2)
#define DMA_CSDP_SRC_TIPB                   (3 << 2)
#define DMA_CSDP_SRC_LOCAL                  (4 << 2)
#define DMA_CSDP_SRC_API                    (5 << 2)

#define DMA_CSDP_DATA_TYPE_MASK             (3 << 0)
#define DMA_CSDP_DATA_TYPE_S32              (2 << 0)
#define DMA_CSDP_DATA_TYPE_S16              (1 << 0)
#define DMA_CSDP_DATA_TYPE_S8               (0 << 0)

//------------------------------------------------------------------------------

#define DMA_CCR_DST_AMODE_MASK              (3 << 14)
#define DMA_CCR_DST_AMODE_DOUBLE            (3 << 14)
#define DMA_CCR_DST_AMODE_SINGLE            (2 << 14)
#define DMA_CCR_DST_AMODE_INC               (1 << 14)
#define DMA_CCR_DST_AMODE_CONST             (0 << 14)

#define DMA_CCR_SRC_AMODE_MASK              (3 << 12)
#define DMA_CCR_SRC_AMODE_DOUBLE            (3 << 12)
#define DMA_CCR_SRC_AMODE_SINGLE            (2 << 12)
#define DMA_CCR_SRC_AMODE_INC               (1 << 12)
#define DMA_CCR_SRC_AMODE_CONST             (0 << 12)

#define DMA_CCR_END_PROG                    (1 << 11)
#define DMA_CCR_FIFO_FLUSH                  (1 << 10)
#define DMA_CCR_REPEAT                      (1 << 9)
#define DMA_CCR_AUTO_INIT                   (1 << 8)
#define DMA_CCR_EN                          (1 << 7)
#define DMA_CCR_PRIO                        (1 << 6)
#define DMA_CCR_FS                          (1 << 5)
#define DMA_CCR_SYNC_PR                     (1 << 4)

#define DMA_CCR_SYNC_MASK                   (0x000F)

//------------------------------------------------------------------------------

#define DMA_CCR2_BS                         (1 << 2)
#define DMA_CCR2_TRANSPARENT_COPY           (1 << 1)
#define DMA_CCR2_CONSTANT_FILL              (1 << 0)

//------------------------------------------------------------------------------

#define DMA_CLNK_CTRL_ENABLE_LNK            (1 << 15)
#define DMA_CLNK_CTRL_STOP_LNK              (1 << 14)

#define DMA_LCH_CTRL_INTERLEAVE_DISABLE     (1 << 15)
#define DMA_LCH_CTRL_LCH_TYPE_2D            (0 << 0)
#define DMA_LCH_CTRL_LCH_TYPE_G             (1 << 0)
#define DMA_LCH_CTRL_LCH_TYPE_P             (2 << 0)
#define DMA_LCH_CTRL_LCH_TYPE_D             (4 << 0)
#define DMA_LCH_CTRL_LCH_TYPE_PD            (15 << 0)


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

//------------------------------------------------------------------------------

#endif // __OMAP5912_DMA_H
