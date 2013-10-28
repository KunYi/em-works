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
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: mx27_ddk.h
//
// Contains MX27 definitions to assist with driver development.
//
//------------------------------------------------------------------------------
#ifndef __MX27_DDK_H__
#define __MX27_DDK_H__

//------------------------------------------------------------------------------
// GENERAL CONSTANTS
//
//------------------------------------------------------------------------------
#define DMAC_CHANNEL_INVALID         -1

//------------------------------------------------------------------------------
// Types
//
//-----------------------------------------------------------------------------
//
// Type: DDK_CLOCK_SIGNAL
//
// Clock signal name for querying/setting clock configuration.
//
//-----------------------------------------------------------------------------
typedef enum
{
    DDK_CLOCK_SIGNAL_MPLL               = 0,
    DDK_CLOCK_SIGNAL_SPLL               = 1,
    DDK_CLOCK_SIGNAL_ARM                = 2,
    DDK_CLOCK_SIGNAL_AHB                = 3,
    DDK_CLOCK_SIGNAL_IPG                = 4,
    DDK_CLOCK_SIGNAL_PERDIV1            = 5,
    DDK_CLOCK_SIGNAL_PERDIV2            = 6,
    DDK_CLOCK_SIGNAL_PERDIV3            = 7,
    DDK_CLOCK_SIGNAL_PERDIV4            = 8,
    DDK_CLOCK_SIGNAL_NFC                = 9,
    DDK_CLOCK_SIGNAL_USB                = 10,
    DDK_CLOCK_SIGNAL_SSI1               = 11,
    DDK_CLOCK_SIGNAL_SSI2               = 12,
    DDK_CLOCK_SIGNAL_H264               = 13,
    DDK_CLOCK_SIGNAL_MSHC               = 14,
    DDK_CLOCK_SIGNAL_CSI                = 15,
    DDK_CLOCK_SIGNAL_ENUM_END           = 16
    } DDK_CLOCK_SIGNAL;

//------------------------------------------------------------------------------
//
// Type: DDK_CLOCK_GATE_INDEX
//
// Index for referencing the corresponding clock gating control bits within
// the CCM.
//
//------------------------------------------------------------------------------
typedef enum 
{
    DDK_CLOCK_GATE_INDEX_SSI2           = 0,
    DDK_CLOCK_GATE_INDEX_SSI1           = 1,
    DDK_CLOCK_GATE_INDEX_SLCDC          = 2,
    DDK_CLOCK_GATE_INDEX_SDHC3          = 3,
    DDK_CLOCK_GATE_INDEX_SDHC2          = 4,
    DDK_CLOCK_GATE_INDEX_SDHC1          = 5,
    DDK_CLOCK_GATE_INDEX_SCC            = 6,
    DDK_CLOCK_GATE_INDEX_SAHARA         = 7,
    DDK_CLOCK_GATE_INDEX_RTIC           = 8,
    DDK_CLOCK_GATE_INDEX_RTC            = 9,
    DDK_CLOCK_GATE_INDEX_RESERVE1       = 10,
    DDK_CLOCK_GATE_INDEX_PWM            = 11,
    DDK_CLOCK_GATE_INDEX_OWIRE          = 12,
    DDK_CLOCK_GATE_INDEX_MSHC           = 13,
    DDK_CLOCK_GATE_INDEX_LCDC           = 14,
    DDK_CLOCK_GATE_INDEX_KPP            = 15,
    DDK_CLOCK_GATE_INDEX_IIM            = 16,
    DDK_CLOCK_GATE_INDEX_I2C2           = 17,
    DDK_CLOCK_GATE_INDEX_I2C1           = 18,
    DDK_CLOCK_GATE_INDEX_GPT6           = 19,
    DDK_CLOCK_GATE_INDEX_GPT5           = 20,
    DDK_CLOCK_GATE_INDEX_GPT4           = 21,
    DDK_CLOCK_GATE_INDEX_GPT3           = 22,
    DDK_CLOCK_GATE_INDEX_GPT2           = 23,
    DDK_CLOCK_GATE_INDEX_GPT1           = 24,
    DDK_CLOCK_GATE_INDEX_GPIO           = 25,
    DDK_CLOCK_GATE_INDEX_FEC            = 26,
    DDK_CLOCK_GATE_INDEX_EMMA           = 27,
    DDK_CLOCK_GATE_INDEX_DMA            = 28,
    DDK_CLOCK_GATE_INDEX_CSPI3          = 29,
    DDK_CLOCK_GATE_INDEX_CSPI2          = 30,
    DDK_CLOCK_GATE_INDEX_CSPI1          = 31,
    DDK_CLOCK_GATE_INDEX_RESERVE2       = 32,
    DDK_CLOCK_GATE_INDEX_RESERVE3       = 33,
    DDK_CLOCK_GATE_INDEX_MSHC_BAUD      = 34,
    DDK_CLOCK_GATE_INDEX_NFG_BAUD       = 35,
    DDK_CLOCK_GATE_INDEX_SSI2_BAUD      = 36,
    DDK_CLOCK_GATE_INDEX_SSI1_BAUD      = 37,
    DDK_CLOCK_GATE_INDEX_H264_BAUD      = 38,
    DDK_CLOCK_GATE_INDEX_PERCLK4        = 39,
    DDK_CLOCK_GATE_INDEX_PERCLK3        = 40,
    DDK_CLOCK_GATE_INDEX_PERCLK2        = 41,
    DDK_CLOCK_GATE_INDEX_PERCLK1        = 42,
    DDK_CLOCK_GATE_INDEX_HCLK_USB       = 43,
    DDK_CLOCK_GATE_INDEX_HCLK_SLCDC     = 44,
    DDK_CLOCK_GATE_INDEX_HCLK_SAHARA    = 45,
    DDK_CLOCK_GATE_INDEX_HCLK_RTIC      = 46,
    DDK_CLOCK_GATE_INDEX_HCLK_LCDC      = 47,
    DDK_CLOCK_GATE_INDEX_HCLK_H264      = 48,
    DDK_CLOCK_GATE_INDEX_HCLK_FEC       = 49,
    DDK_CLOCK_GATE_INDEX_HCLK_EMMA      = 50,
    DDK_CLOCK_GATE_INDEX_HCLK_EMI       = 51,
    DDK_CLOCK_GATE_INDEX_HCLK_DMA       = 52,
    DDK_CLOCK_GATE_INDEX_HCLK_CSI       = 53,
    DDK_CLOCK_GATE_INDEX_HCLK_BROM      = 54,
    DDK_CLOCK_GATE_INDEX_HCLK_ATA       = 55,
    DDK_CLOCK_GATE_INDEX_WDT            = 56,
    DDK_CLOCK_GATE_INDEX_USB            = 57,
    DDK_CLOCK_GATE_INDEX_UART6          = 58,
    DDK_CLOCK_GATE_INDEX_UART5          = 59,
    DDK_CLOCK_GATE_INDEX_UART4          = 60,
    DDK_CLOCK_GATE_INDEX_UART3          = 61,
    DDK_CLOCK_GATE_INDEX_UART2          = 62,
    DDK_CLOCK_GATE_INDEX_UART1          = 63,
} DDK_CLOCK_GATE_INDEX;

//------------------------------------------------------------------------------
//
// Type: DDK_CLOCK_GATE_MODE
//
// Clock gating modes supported by CCM clock gating registers.
//
//------------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_GATE_MODE_DISABLE         = 0,
    DDK_CLOCK_GATE_MODE_ENABLE          = 1,
} DDK_CLOCK_GATE_MODE;

//------------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_BAUD_SOURCE
//
//  Input source for baud clock generation.
//
//------------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_BAUD_SOURCE_SPLL        = 0,
    DDK_CLOCK_BAUD_SOURCE_MPLL        = 1,
} DDK_CLOCK_BAUD_SOURCE;

//------------------------------------------------------------------------------
//
// Type: DDK_CLOCK_CKO_SRC
//
// Clock output source (CKO) signal selections.
//
//------------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_CKO_SRC_CLK32             = 0,
    DDK_CLOCK_CKO_SRC_PREMCLK           = 1,
    DDK_CLOCK_CKO_SRC_CLK26M            = 2,
    DDK_CLOCK_CKO_SRC_MPLL_REF_CLK      = 3,
    DDK_CLOCK_CKO_SRC_SPLL_REF_CLK      = 4,
    DDK_CLOCK_CKO_SRC_MPLL_CLK          = 5,
    DDK_CLOCK_CKO_SRC_SPLL_CLK          = 6,
    DDK_CLOCK_CKO_SRC_FCLK              = 7,
    DDK_CLOCK_CKO_SRC_HCLK              = 8,
    DDK_CLOCK_CKO_SRC_IPGCLK            = 9,
    DDK_CLOCK_CKO_SRC_PERCLK1           = 10,
    DDK_CLOCK_CKO_SRC_PERCLK2           = 11,
    DDK_CLOCK_CKO_SRC_PERCLK3           = 12,
    DDK_CLOCK_CKO_SRC_PERCLK4           = 13,
    DDK_CLOCK_CKO_SRC_SSI1_BAUD         = 14,
    DDK_CLOCK_CKO_SRC_SSI2_BAUD         = 15,
    DDK_CLOCK_CKO_SRC_NFC_BAUD          = 16,
    DDK_CLOCK_CKO_SRC_MSHC_BAUD         = 17,
    DDK_CLOCK_CKO_SRC_H264_BAUD         = 18,
    DDK_CLOCK_CKO_SRC_CLK60M_ALWAYS     = 19,
    DDK_CLOCK_CKO_SRC_CLK32K_ALWAYS     = 20,
    DDK_CLOCK_CKO_SRC_CLK48M            = 21,
    DDK_CLOCK_CKO_SRC_DPTC_REF_CLK      = 22
} DDK_CLOCK_CKO_SRC;

//------------------------------------------------------------------------------
//
// Type: DDK_CLOCK_CKO_DIV
//
// Clock output source (CKO) divider selections.
//
//------------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_CKO_DIV_1                 = 0,
    DDK_CLOCK_CKO_DIV_2                 = 1,
    DDK_CLOCK_CKO_DIV_3                 = 2,
    DDK_CLOCK_CKO_DIV_4                 = 3,
    DDK_CLOCK_CKO_DIV_5                 = 4,
    DDK_CLOCK_CKO_DIV_6                 = 5,
    DDK_CLOCK_CKO_DIV_7                 = 6,
    DDK_CLOCK_CKO_DIV_8                 = 7,
} DDK_CLOCK_CKO_DIV;

//------------------------------------------------------------------------------
//
// Type: DMAC_REQUEST_SRC
//
// DMA request resources.
//
//------------------------------------------------------------------------------
typedef enum {
    DMAC_REQUEST_RESERVED1,
    DMAC_REQUEST_CSPI3_RX_FIFO,
    DMAC_REQUEST_CSPI3_TX_FIFO,
    DMAC_REQUEST_Ext_DMA_REQ,
    DMAC_REQUEST_MSHC,
    DMAC_REQUEST_RESERVED2,
    DMAC_REQUEST_SDHC2,
    DMAC_REQUEST_SDHC1,
    DMAC_REQUEST_SSI2_RX0_FIFO,
    DMAC_REQUEST_SSI2_TX0_FIFO,
    DMAC_REQUEST_SSI2_RX1_FIFO,
    DMAC_REQUEST_SSI2_TX1_FIFO,
    DMAC_REQUEST_SSI1_RX0_FIFO,
    DMAC_REQUEST_SSI1_TX0_FIFO,
    DMAC_REQUEST_SSI1_RX1_FIFO,
    DMAC_REQUEST_SSI1_TX1_FIFO,
    DMAC_REQUEST_CSPI2_RX_FIFO,
    DMAC_REQUEST_CSPI2_TX_FIFO,
    DMAC_REQUEST_CSPI1_RX_FIFO,
    DMAC_REQUEST_CSPI1_TX_FIFO,
    DMAC_REQUEST_UART4_RX_FIFO,
    DMAC_REQUEST_UART4_TX_FIFO,
    DMAC_REQUEST_UART3_RX_FIFO,
    DMAC_REQUEST_UART3_TX_FIFO,
    DMAC_REQUEST_UART2_RX_FIFO,
    DMAC_REQUEST_UART2_TX_FIFO,
    DMAC_REQUEST_UART1_RX_FIFO,
    DMAC_REQUEST_UART1_TX_FIFO,
    DMAC_REQUEST_ATA_TX_FIFO,
    DMAC_REQUEST_ATA_RCV_FIFO,
    DMAC_REQUEST_CSI_STAT_FIFO,
    DMAC_REQUEST_CSI_RX_FIFO,
    DMAC_REQUEST_UART5_TX_FIFO,
    DMAC_REQUEST_UART5_RX_FIFO,
    DMAC_REQUEST_UART6_TX_FIFO,
    DMAC_REQUEST_UART6_RX_FIFO,
    DMAC_REQUEST_SDHC3,
    DMAC_REQUEST_NFC,
    DMAC_REQUEST_MAX = 64
} DMAC_REQUEST_SRC;
//------------------------------------------------------------------------------
//
// Type: DMAC_TRANSFER_STATUS
//
// DMA transfer status.
//
//------------------------------------------------------------------------------
typedef enum {
    DMAC_TRANSFER_STATUS_NONE,
    DMAC_TRANSFER_STATUS_COMPLETE=0x01,
    DMAC_TRANSFER_STATUS_BURST_TIMEOUT=0x02,
    DMAC_TRANSFER_STATUS_REQ_TIMEOUT=0x04,
    DMAC_TRANSFER_STATUS_ERROR=0x08,
    DMAC_TRANSFER_STATUS_BUFFER_OVERFLOW=0x10,
} DMAC_TRANSFER_STATUS;

//------------------------------------------------------------------------------
//
// Type: DMAC_TRANSFER_MODE
//
// DMA transfer mode.
//
//------------------------------------------------------------------------------
typedef enum {
    DMAC_TRANSFER_MODE_LINEAR_MEMORY,
    DMAC_TRANSFER_MODE_2D_MEMORY,
    DMAC_TRANSFER_MODE_FIFO,
    DMAC_TRANSFER_MODE_EOBE
} DMAC_TRANSFER_MODE;

//------------------------------------------------------------------------------
//
// Type: DMAC_TRANSFER_SIZE
//
// DMA transfer size.
//
//------------------------------------------------------------------------------
typedef enum DMAC_TRANSFER_SIZE {
    DMAC_TRANSFER_SIZE_32BIT,
    DMAC_TRANSFER_SIZE_8BIT,
    DMAC_TRANSFER_SIZE_16BIT
} DMAC_TRANSFER_SIZE;

//------------------------------------------------------------------------------
//
// Type: DMAC_REPEAT_TYPE
//
// DMA repeat type for DMA chain.
//
//------------------------------------------------------------------------------
typedef enum DMAC_REPEAT_TYPE {
    DMAC_REPEAT_DISABLED,
    DMAC_REPEAT_ONCE,
    DMAC_REPEAT_FOREVER, // dma cycle does not stop until TransStop() is called
} DMAC_REPEAT_TYPE;

//------------------------------------------------------------------------------
//
// Type: DMAC_CHANNEL_CFG 
//
// struct for DMA channel configurations.
//
//------------------------------------------------------------------------------
typedef struct DMAC_CHANNEL_CFG {
    UINT32               SrcAddr;
    UINT32               DstAddr;
    UINT32               DataSize;
    DMAC_TRANSFER_MODE   DstMode;
    DMAC_TRANSFER_MODE   SrcMode;
    BOOL                 MemDirIncrease;
    DMAC_TRANSFER_SIZE   DstSize;
    DMAC_TRANSFER_SIZE   SrcSize;
    DMAC_REPEAT_TYPE     RepeatType;
    BOOL                 ExtReqEnable;
    DMAC_REQUEST_SRC     ReqSrc;
    UINT32               BurstLength;
    BOOL                 ReqTimeout;
    UINT16               ReqTOCounter;
    UINT16               BusClkCounter;
    UINT16               WSR;
    UINT16               XSR;
    UINT16               YSR;
    UINT16               _pad;
} DMAC_CHANNEL_CFG,  *PDMAC_CHANNEL_CFG;


//------------------------------------------------------------------------------
//
// Structure and Macro for GPIO IOMUX configuration function DDKGpioEnable() 
// and DDKGpioDisable().
//
//------------------------------------------------------------------------------
typedef struct  {
    GPIO_CFG_TYPE   ConfigType;
    union {
        struct {
            GPIO_PORT   Port;
            UINT32      PinMap;
            UINT32      PuenMap;
        } PriConfig;
        struct {
            GPIO_PORT   Port;
            UINT32      PinMap;
            UINT32      PuenMap;
        } AltConfig;
        struct {
            GPIO_PORT               Port;
            UINT32                  InputPinMap;
            GPIO_INPUT_DEST_TYPE    InputDest;
            UINT32                  OutputPinMap;
            GPIO_OUTPUT_SOURCE_TYPE OutputSource;
            UINT32                  PuenMap;
        } ModuleIOConfig;
        struct {
            GPIO_PORT       Port;
            UINT32          PinMap;
            GPIO_INT_TYPE   IntType;    // interrupt trigger type
            DWORD           SysIntr;    // System interrupt ID
        } IntrConfig;
        struct {
            GPIO_PORT   Port;
            UINT32      InputPinMap;
            UINT32      OutputPinMap;
            UINT32      PuenMap;
        } IOConfig;
    };          
} DDK_GPIO_CFG, *PDDK_GPIO_CFG;

// The macro filling in DDK_GPIO_CFG structure.
#define DDK_GPIO_SET_CONFIG(config, module) \
    GPIOSetCfg2(GPIOValueCfgType(module), config, module)


//------------------------------------------------------------------------------
// Functions
BOOL DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);
BOOL DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE *pMode);
BOOL DDKClockGetFreq(DDK_CLOCK_SIGNAL sig, UINT32 *freq);
BOOL DDKClockConfigBaud(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src, UINT32 div);
BOOL DDKClockSetCKO(BOOL bEnable, DDK_CLOCK_CKO_SRC src, DDK_CLOCK_CKO_DIV div);

BOOL DDKGpioEnable(DDK_GPIO_CFG *pGpioCfg);
BOOL DDKGpioDisable(DDK_GPIO_CFG *pGpioCfg);
BOOL DDKGpioWriteData(GPIO_PORT port, UINT32 portMask, UINT32 data);
BOOL DDKGpioWriteDataPin(GPIO_PORT port, UINT32 pin, UINT32 data);
BOOL DDKGpioReadData(GPIO_PORT port, UINT32 portMask, UINT32 *pData);
BOOL DDKGpioReadDataPin(GPIO_PORT port, UINT32 pin, UINT32 *pData);
BOOL DDKGpioReadIntr(GPIO_PORT port, UINT32 portMask, UINT32 *pStatus);
BOOL DDKGpioReadIntrPin(GPIO_PORT port, UINT32 pin, UINT32 *pStatus);
BOOL DDKGpioClearIntrPin(GPIO_PORT port, UINT32 pin);
BOOL DDKGpioSetIntrPin(GPIO_PORT port, UINT32 pin, BOOL bEnable);
BOOL DDKGpioSetIntrType(GPIO_PORT port, UINT32 pin, GPIO_INT_TYPE type);

UINT8 DDKDmacRequestChan(UINT8 chan);
BOOL DDKDmacReleaseChan(UINT8 chan);
UINT8 DDKDmacConfigureChan(UINT8 chan, DMAC_CHANNEL_CFG *pChannelCfg);
BOOL DDKDmacStartChan(UINT8 chan);
BOOL DDKDmacStopChan(UINT8 chan);
UINT32 DDKDmacGetTransStatus(UINT8 chan);
UINT32 DDKDmacGetTransSize(UINT8 chan);
void DDKDmacSetSrcAddress(UINT8 chan, UINT32 src);
void DDKDmacSetDestAddress(UINT8 chan, UINT32 dest);
void DDKDmacSetTransCount(UINT8 chan, UINT32 count);
void DDKDmacSetBurstLength(UINT8 chan, UINT32 burstLen);
void DDKDmacSetRepeatType(UINT8 chan, DMAC_REPEAT_TYPE repeatType);
BOOL DDKDmacIsChannelIntr(UINT8 chan);
void DDKDmacClearChannelIntr(UINT8 chan);
void DDKDmacEnableChannelIntr(UINT8 chan);
void DDKDmacDisableChannelIntr(UINT8 chan);

#endif // __MX27_DDK_H__

