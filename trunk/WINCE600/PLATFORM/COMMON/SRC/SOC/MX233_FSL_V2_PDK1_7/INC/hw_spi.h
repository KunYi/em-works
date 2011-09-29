//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: hw_spi.h
//------------------------------------------------------------------------------
#ifndef __HW_SPI__
#define __HW_SPI__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define CSPI_IOCTL_EXCHANGE             CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3030, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CSPI_IOCTL_ENABLE_LOOPBACK      CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3031, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CSPI_IOCTL_DISABLE_LOOPBACK     CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3032, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define CSPI_IOCTL_SSPCONFIGURE         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3033, METHOD_BUFFERED, FILE_ANY_ACCESS)    
#define CSPI_IOCTL_SSPRESETAFTERERROR   CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3034, METHOD_BUFFERED, FILE_ANY_ACCESS)    
#define CSPI_IOCTL_SSPCHECKERRORS       CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3035, METHOD_BUFFERED, FILE_ANY_ACCESS)    
#define CSPI_IOCTL_SSPGETIRQSTATUS      CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3036, METHOD_BUFFERED, FILE_ANY_ACCESS)    
#define CSPI_IOCTL_SSPCLEARIRQ          CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3037, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CSPI_IOCTL_SSPCONFIGTIMING      CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3038, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CSPI_IOCTL_SSP_EN_ERROR         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3039, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CSPI_IOCTL_SSP_DIS_ERROR        CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3040, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CSPI_IOCTL_SSP_CLR_LOCK_CS      CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3041, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CSPI_IOCTL_SSP_CLR_DET          CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3042, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CSPI_IOCTL_SSP_GET_RESP         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 3043, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define CSPI_MAXENQ_EVENT_NAME        64
//------------------------------------------------------------------------------
// Types
// Enumerated type representing the word length of data
typedef enum _SSP_WORDLENGTH
{
    SSP_WORD_LENGTH_4BITS=0x3,
    SSP_WORD_LENGTH_8BITS=0x7,
    SSP_WORD_LENGTH_16BITS=0xF
}SSP_WORDLENGTH;

// Enumerated type representing the SSP block modes of operation
typedef enum _SSP_MODE
{
    SSP_MODE_SPI=0x0,
    SSP_MODE_SSI=0x1,
    SSP_MODE_MICROWIRE=0x2,
    SSP_MODE_SD_MMC=0x3,
    SSP_MODE_MEMSTICK=0x4
}SSP_MODE;

// Enumerated type representing the SSP IRQs
typedef enum _SSP_IRQ
{
    SSP_IRQ_SDIO,
    SSP_IRQ_RESP_ERR,
    SSP_IRQ_RESP_TIMEOUT,
    SSP_IRQ_DATA_TIMEOUT,
    SSP_IRQ_DATA_CRC,
    SSP_IRQ_FIFO_UNDERRUN,
    SSP_IRQ_CEATA_CCS_ERR,
    SSP_IRQ_RECV_TIMEOUT,
    SSP_IRQ_FIFO_OVERRUN
}SSP_IRQ;


typedef enum _SPEEDENUM
{
    SPI_IDENTIFY_SPEED = 0,         //!< 400KHz
    SPI_TRANSFER_SPEED,             //!< 12MHz
    SPI_TRANSFER_HS_NORMAL_SPEED,   //!< 12MHz
    SPI_TRANSFER_HS_FAST_SPEED,     //!< 12MHz
    SPI_UNDEFINED_SPEED             //!< Before being initialized.
} SSP_SPEED;



//brief Data structure passed into the Initialization function.
typedef struct _SSP_INIT
{
    SSP_MODE  eMode;             // SSP mode of operation
    SSP_WORDLENGTH eLength;      // data word length
    BOOL bDmaEnable;                // 0: dma disabled, 1: dma enabled
    BOOL b_ceata_ccs_err_en;        // CEATA Unexpected CCS Error logic enable. 0: disabled, 1: enabled
    BOOL bPhase;                    // SPI, MemStick, and SD modes
    BOOL bPolarity;                 // SPI, MemStick, and SD modes
    BOOL bSlave;                    // 0: SSP is Master, 1: SSP is Slave
    BOOL bIgnoreCrc;                // Ignore the response CRC
    BOOL bBusWidth4;                // 0: 1-bit bus, 1: 4 bit bus
    
    // Memory stick: wait for Memory stick IRQ before switching to bus state 1
    // MMC/SD: wait for MMC ready before sending command.
    BOOL bWaitIrq;
    BOOL bLongResp;                 // Get long response from device
    BOOL bCheckResp;                // Check Response against reference to insure integrity of response
    BOOL bGetResp;                  // wait for a response
    UINT16 u16TransferCount;
    BOOL bEnable;                   // Command Transmit Enable.
    BOOL bWaitCmd;                  //
    BOOL bDataTransfr;              // Data Transfer Enable
    BOOL bRead;                     // Read Mode

    //name SPI MODE ONLY
    
    BOOL bSlaveOutDisable;          // 0: SSP can drive MISO in slave mode, 1: SSP does not drive MISO
    // 0: do not ignore receive data when transmitting,
    // 1: ignore receive data when transmitting and force transmit data=0 when receiving.
    BOOL bHalfDuplex;
    // 0: deassert chip select after RUN complete,
    // 1: continue to assert chip select after RUN
    BOOL bLockCs; 
    
}SSP_INIT , *PSSP_INIT;

typedef union
{
    UINT32  U;
    struct
    {
        unsigned XFER_COUNT      :  16;    
        unsigned ENABLE          :  1;
        unsigned GET_RESP        :  1;
        unsigned CHECK_RESP      :  1;
        unsigned LONG_RESP       :  1;
        unsigned WAIT_FOR_CMD    :  1;
        unsigned WAIT_FOR_IRQ    :  1;
        unsigned BUS_WIDTH       :  2;
        unsigned DATA_XFER       :  1;
        unsigned READ            :  1;
        unsigned IGNORE_CRC      :  1;
        unsigned LOCK_CS         :  1;
        unsigned SDIO_IRQ_CHECK  :  1;
        unsigned RUN             :  1;
        unsigned CLKGATE         :  1;
        unsigned SFTRST          :  1;
    } B;
} SSP_CTRL0;

typedef union
{
    UINT32 U;
    struct
    {
        unsigned CLOCK_RATE     :8;
        unsigned CLOCK_DIVIDE   :8;
        unsigned TIMEOUT        :16;
    } B;
} SSP_TIMING;


typedef union
{
    UINT32 U;
    struct
    {
        unsigned SSP_MODE              :  4;
        unsigned WORD_LENGTH           :  4;
        unsigned SLAVE_MODE            :  1;
        unsigned POLARITY              :  1;
        unsigned PHASE                 :  1;
        unsigned SLAVE_OUT_DISABLE     :  1;
        unsigned CEATA_CCS_ERR_EN      :  1;
        unsigned DMA_ENABLE            :  1;
        unsigned FIFO_OVERRUN_IRQ_EN   :  1;
        unsigned FIFO_OVERRUN_IRQ      :  1;
        unsigned RECV_TIMEOUT_IRQ_EN   :  1;
        unsigned RECV_TIMEOUT_IRQ      :  1;
        unsigned CEATA_CCS_ERR_IRQ_EN  :  1;
        unsigned CEATA_CCS_ERR_IRQ     :  1;
        unsigned FIFO_UNDERRUN_EN      :  1;
        unsigned FIFO_UNDERRUN_IRQ     :  1;
        unsigned DATA_CRC_IRQ_EN       :  1;
        unsigned DATA_CRC_IRQ          :  1;
        unsigned DATA_TIMEOUT_IRQ_EN   :  1;
        unsigned DATA_TIMEOUT_IRQ      :  1;
        unsigned RESP_TIMEOUT_IRQ_EN   :  1;
        unsigned RESP_TIMEOUT_IRQ      :  1;
        unsigned RESP_ERR_IRQ_EN       :  1;
        unsigned RESP_ERR_IRQ          :  1;
        unsigned SDIO_IRQ_EN           :  1;
        unsigned SDIO_IRQ              :  1;
    } B;
} SSP_CTRL1;

typedef union
{
    UINT32 U;
    struct
    {
        unsigned BUSY               :  1;
        unsigned RSVD0              :  1;
        unsigned DATA_BUSY          :  1;
        unsigned CMD_BUSY           :  1;
        unsigned FIFO_UNDRFLW       :  1;
        unsigned FIFO_EMPTY         :  1;
        unsigned RSVD1              :  2;
        unsigned FIFO_FULL          :  1;
        unsigned FIFO_OVRFLW        :  1;
        unsigned CEATA_CCS_ERR      :  1;
        unsigned RECV_TIMEOUT_STAT  :  1;
        unsigned TIMEOUT            :  1;
        unsigned DATA_CRC_ERR       :  1;
        unsigned RESP_TIMEOUT       :  1;
        unsigned RESP_ERR           :  1;
        unsigned RESP_CRC_ERR       :  1;
        unsigned SDIO_IRQ           :  1;
        unsigned DMAEND             :  1;
        unsigned DMAREQ             :  1;
        unsigned DMATERM            :  1;
        unsigned DMASENSE           :  1;
        unsigned RSVD3              :  6;
        unsigned CARD_DETECT        :  1;
        unsigned SD_PRESENT         :  1;
        unsigned MS_PRESENT         :  1;
        unsigned PRESENT            :  1;
    } B;
} SSP_STATUS;

typedef union
{
    UINT32  U;
    struct
    {
        unsigned CMD          :  8;
        unsigned BLOCK_COUNT  :  8;
        unsigned BLOCK_SIZE   :  4;
        unsigned APPEND_8CYC  :  1;
        unsigned RSVD0        : 11;
    } B;
} SSP_CMD0;

typedef union
{
    UINT32 U;
    struct
    {
        UINT32  CMD_ARG;
    } B;
} SSP_CMD1;

typedef struct _SSP_RESETCONFIG
{
    SSP_CTRL0 sCtrl0;   // SSP_CTRL0 reg value
    SSP_CTRL1 sCtrl1;   // SSP_CTRL1 reg value
    SSP_TIMING sTiming; // SSP_TIMING reg value
    BOOL bLowPower;     // 1 if low power, 0 otherwise
}SSP_RESETCONFIG;
//#ifdef POLLING_MODE
// CSPI bus configuration
typedef struct 
{
    SSP_CTRL0 SspCtrl0;
    SSP_CMD0  SspCmd;
    SSP_CMD1  SspArg;
    BOOL      usedma;
    BOOL      usepolling;
    UINT8     bitcount; 
    BOOL      bRead;
    BOOL      bCmd;
} CSPI_BUSCONFIG_T, *PCSPI_BUSCONFIG_T;

// CSPI exchange packet
typedef struct
{
    PCSPI_BUSCONFIG_T pBusCnfg;
    LPVOID pBuf;
    UINT32 xchCnt;
    LPWSTR xchEvent;
    UINT32 xchEventLength;
} CSPI_XCH_PKT_T, *PCSPI_XCH_PKT_T;
//#endif
#define SSP_CTRL1_HANDLED_ERRORS_MASK    (   \
        BM_SSP_CTRL1_FIFO_OVERRUN_IRQ_EN   |    \
        BM_SSP_CTRL1_RECV_TIMEOUT_IRQ_EN   |    \
        BM_SSP_CTRL1_FIFO_UNDERRUN_EN      |    \
        BM_SSP_CTRL1_DATA_CRC_IRQ_EN       |    \
        BM_SSP_CTRL1_DATA_TIMEOUT_IRQ_EN   |    \
        BM_SSP_CTRL1_RESP_TIMEOUT_IRQ_EN   |    \
        BM_SSP_CTRL1_RESP_ERR_IRQ_EN       |    \
        BM_SSP_CTRL1_SDIO_IRQ_EN                )

#define SSP_CTRL1_IRQS_MASK    (   \
        BM_SSP_CTRL1_FIFO_OVERRUN_IRQ_EN   |    \
        BM_SSP_CTRL1_FIFO_OVERRUN_IRQ      |    \
        BM_SSP_CTRL1_RECV_TIMEOUT_IRQ_EN   |    \
        BM_SSP_CTRL1_RECV_TIMEOUT_IRQ      |    \
        BM_SSP_CTRL1_CEATA_CCS_ERR_IRQ_EN  |    \
        BM_SSP_CTRL1_CEATA_CCS_ERR_IRQ     |    \
        BM_SSP_CTRL1_FIFO_UNDERRUN_EN      |    \
        BM_SSP_CTRL1_FIFO_UNDERRUN_IRQ     |    \
        BM_SSP_CTRL1_DATA_CRC_IRQ_EN       |    \
        BM_SSP_CTRL1_DATA_CRC_IRQ          |    \
        BM_SSP_CTRL1_DATA_TIMEOUT_IRQ_EN   |    \
        BM_SSP_CTRL1_DATA_TIMEOUT_IRQ      |    \
        BM_SSP_CTRL1_RESP_TIMEOUT_IRQ_EN   |    \
        BM_SSP_CTRL1_RESP_TIMEOUT_IRQ      |    \
        BM_SSP_CTRL1_RESP_ERR_IRQ_EN       |    \
        BM_SSP_CTRL1_RESP_ERR_IRQ          |    \
        BM_SSP_CTRL1_SDIO_IRQ_EN           |    \
        BM_SSP_CTRL1_SDIO_IRQ                   )


//------------------------------------------------------------------------------
// Functions
HANDLE CSPIOpenHandle(LPCWSTR lpDevName);
BOOL CSPICloseHandle(HANDLE hCSPI);
BOOL CSPIConfigure(HANDLE hCSPI,PSSP_INIT  pSspInit);
BOOL CSPIResetAfterError(HANDLE hCSPI,SSP_RESETCONFIG * sConfigparams);
BOOL CSPICheckErros(HANDLE hCSPI);
BOOL CSPIGetIRQStatus(HANDLE hCSPI,SSP_IRQ eIrq);
BOOL CSPIClearIRQ(HANDLE hCSPI,SSP_IRQ eIrq);
BOOL CSPIConfigTiming(HANDLE hCSPI,SSP_SPEED eSpeed);
BOOL CSPIEnableErrorIRQ(HANDLE hCSPI,BOOL bEnable);
BOOL CSPIDisableErrorIRQ(HANDLE hCSPI);
BOOL CSPIClearLOCKCS(HANDLE hCSPI);

BOOL CSPIExchange(HANDLE hCSPI, PCSPI_XCH_PKT_T pCspiXchPkt);
//BOOL CSPIDisableLoopback(HANDLE hCSPI);
BOOL CSPIEnableLoopback(HANDLE hCSPI);

#ifdef __cplusplus
}
#endif  /* __cplusplus */


#endif //__HW_SPI__
