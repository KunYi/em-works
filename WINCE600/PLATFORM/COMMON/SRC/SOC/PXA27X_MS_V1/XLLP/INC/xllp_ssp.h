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
#ifndef __XLLP_H__
#define __XLLP_H__
/* Copyright 1999,2000,2001 Intel Corp.  */


//SSCR0  Macro definition for port 0/1/2
#define XLLP_SSCR0_MOD                  (0x1<<31)         //0-- Normal SSP mode
#define XLLP_SSCR0_ACS                  (0x1<<30)         // 1 - Audio Clock, 0 -- decided by NCS and ECS
#define XLLP_SSCR0_TIM                  (0x1<<23)         //Transmit FIFO underrun Interrupt Mask, 0 -- TUR will generate SSP interrupt
#define XLLP_SSCR0_RIM                  (0x1<<22)         //Receive FIFO Over Run Interrupt Mask 0 -- will generate interrupt
#define XLLP_SSCR0_NCS                  (0x1<<21)        //Select network Clock 0-- ECS bit determine clock selection
#define XLLP_SSCR0_EDSS                 (0x1<<20)        //Extended data size select is used in conjunction with DSS to select the size of data transimitted
                                                        //1 --- one is pre-appended to the DSS value that sets DSS range from 17-32 bit.
#define XLLP_SSCR0_SCR_512K             (0x19<<8)        //SCR value is 25 based on system SSP clock is 13MHZ
#define XLLP_SSCR0_SCR_128K             (0x67<<8)
#define XLLP_SSCR0_SSE                  (0x1<<7)         // 1 -- SSP operation is enabled
#define XLLP_SSCR0_ECS                  (0x1<<6)         //  External Clock Select, 0 - on_chip clock used to produce the SSP port's serial clock
#define XLLP_SSCR0_FRF_TISSP            (0x1<<4)         // TI SSP
#define XLLP_SSCR0_FRF_PSP              (0x3<<4)         // PSP -- programmable serial protocol
#define XLLP_SSCR0_DSS_14BIT            0xD              // 14 bit data transfer
#define XLLP_SSCR0_DSS_32BIT            0xF              //32 bit data transfer
#define XLLP_SSCR0_DSS_8BIT             0x7              //8 bit data
#define XLLP_SSCR0_DSS_16BIT            0xF              //16 bit data EDSS = 0
//SSCR1

#define XLLP_SSCR1_THLD_8               (0x8<<10)       //threshold is 8 bytes, written to
#define XLLP_SSCR1_TTELP                (0x1<<31)       //1 -- TXD line will be tristated 1/2 clock after TXD is to be flopped
#define XLLP_SSCR1_TTE                  (0x1<<30)       //1 -- TXD line will not be tristated, 0 --- TXD line will be tristated when not transmitting data
#define XLLP_SSCR1_EBCE1                (0x1<<29)       //1 -- Interrupt due to a bit error is enabled. 00 -- Interrupt due to a bit count is disabled.
#define XLLP_SSCR1_SCFR                 (0x1<<28)       //1 -- clock iput to SSPSCLK is active only during trnasfers
#define XLLP_SSCR1_ECRA                 (0x1<<27)       //Enable Clock request A, 1 = Clcok request from other SSP port is enabled
#define XLLP_SSCR1_ECRB                 (0x1<<26)       //Enable Clock Request B
#define XLLP_SSCR1_SCLKDIR              (0x1<<25)       //0 -- Master Mode, the port generate SSPSCLK internally, acts as master and drive SSPSCLK
#define XLLP_SSCR1_SFRMDIR              (0x1<<24)       //SSP frame direction determine whether SSP port is the master or slave 0 -- Master
#define XLLP_SSCR1_RWOT                 (0x1<<23)       // 0 -- Transmit/Receive mode, 1 -- Receive with out Transmit mode
#define XLLP_SSCR1_TRAIL                (0x1<<22)       //Trailing byte is used to configure how trailing byte are handled, 0 -- Processor Based, Trailing byte are handled by processor
#define XXLP_SSCR1_TSRE                 (0x1<<21)       //Transmit Service Request Enables the transmit FIFO DMA service request.1 -- DMA service request is enabled
#define XXLP_SSCR1_RSRE                 (0x1<<20)      //Receive Service Request enables the Receive FIFO DMa Service Request
#define XXLP_SSCR1_TINTE                (0x1<<19)       //Receiver time-out interrupt enables the receiver time-out interrupt, 1 -- receive time-out interrupts are enabled, 1 -- peripheral trailing byte interrupt are enabled
#define XXLP_SSCR1_PINTE                (0x1<<18)       //Peripheral trailing byte interrupt enables the peripheral trailing byte interrupt
#define XXLP_SSCR1_STRF                 (0x1<<15)       //Select whether the transmit FIFO or receive FIFO is enabled for writes and reads (test mode)
#define XXLP_SSCR1_EFWR                 (0x1<<14)       //Enable FIFO Write/Read (Test mode bit) for the SSP port
#define XXLP_SSCR1_MWDS                 (0x1<<5)        //Mircowire Transmit Data size.1 = 16 bit command word is transmitted
#define XXLP_SSCR1_SPH                  (0x1<<4)        //SPI SSPSCLK phase setting  0 = SSPSCLK is inactive one cycle at the start of frame and 1/2 cycle at the end of frame
#define XLLP_SSCR1_SPO                  (0x1<<3)        //Motoroloa SPI SSPSCLK polarity setting selects the polarity of the inactive state of the SSPSCLK pins
#define XXLP_SSCR1_LBM                  (0x1<<2)         //Loop-back mode
#define XXLP_SSCR1_TIE                  (0x1<<1)        //0 = Transmit FIFO level interrupt is disabled.
#define XXLP_SSCR1_RIE                  0x1              //0 = Receive FIFO level interrupt is disabled. The interrupt is masked.


//SSPSP programmable Serial Protocol Register


//SSTO     SSP time-out register   0-23 bits R/W time-out interval = (TIMEOUT)/Peripheral Clock Frequency

//SSSP     SSP status Register
#define XXLP_SSSP_BCE                   (0x1<<23)       //Bit Count Error 1 -- the SSPSFRM signal has been asserted when the bit counter was not 0
#define XXLP_SSSP_CSS                   (0x1<<22)       //Clock Synchronous Status  1 -- The SSP is currently busy synchronizing slave mode operation.
#define XXLP_SSSP_TUR                   (0x1<<21)       //1 = Attempted read from the tramit FIFO when the FIFO was empty
#define XXLP_SSSP_EOC                   (0x1<<20)       // 1 -- DMA has singaled an end of chain condition, there is no more data to be processed
#define XXLP_SSSP_TINT                  (0x1<<19)       //1 = Receiver Time-out pending
#define XXLP_SSSP_PINT                  (0x1<<18)        // 1 = Peripheral Trailing Byte Interrupt Pending
#define XXLP_SSSP_RFL_MASK              (0xf<<12)        //Receive FIFO Level is the number of valid entries.
#define XXLP_SSSP_TFL_MASK              (0xf<<8)         //The Transmit FIFO level the number of valid entries currently in the transmit FIFO
#define XXLP_SSSP_ROR                   (0x1<<7)         //1 = attempted data write to full receive FIFO
#define XXLP_SSSP_RFS                   (0x1<<6)         // 1 = Receive FIFO level is at or above RFT tirgger
#define XXLP_SSSP_TFS                   (0x1<<5)          //1 = Transmit FIFO level is at or below TFT tigger threshold, request interrupt
#define XXLP_SSSP_BSY                   (0x1<<4)          // 1 = SSP port is currently transmitting or receiving a frame
#define XXLP_SSSP_RNE                   (0x1<<3)          //1 = Receive FIFO is not empty
#define XXLP_SSSP_TNF                   (0x1<<2)          // 1 = Transmit FIFO is not full


//
// SSP
//
typedef struct
{
    unsigned long    sscr0;                         //SSP control register 0
    unsigned long    sscr1;                         //SSP control register 1
    unsigned long    ssr;                           //SSP status register
    unsigned long    ssitr;                         //SSP interrupt test register
    unsigned long    ssdr;                          //SSP data read/write register
    unsigned long    reserved1;
    unsigned long    reserved2;
    unsigned long    reserved3;
    unsigned long    reserved4;
    unsigned long    reserved5;
    unsigned long    ssto;                         //Time out register
    unsigned long    sspsp;                        //SSP programmable serial protocol
    unsigned long    sstsa;                        //SSP2 TX time slot active
    unsigned long    ssrsa;                        //SSP2 RX time slot active

} XLLP_SSP_REGS, *XLLP_PSSP_REGS;




/*
XLLP_BOOL_T XllpInitSspLink(volatile XLLP_SSP_REGS *);
XLLP_BOOL_T XllpSspGPIOConfigure(volatile XLLP_GPIO_T *);
XLLP_BOOL_T XllpReadSspFiFO(volatile XLLP_SSP_REGS *,XLLP_UINT32_T *,XLLP_UINT32_T, XLLP_UINT32_T);
XLLP_BOOL_T XllpWriteSspFifo(volatile XLLP_SSP_REGS *,XLLP_UINT32_T *,XLLP_UINT32_T, XLLP_UINT32_T);
XLLP_BOOL_T XllpReadTSCodec(volatile XLLP_SSP_REGS *,XLLP_UINT32_T *);
XLLP_BOOL_T XllpWriteTSCodec(volatile XLLP_SSP_REGS *,volatile XLLP_BCR_T *,XLLP_UINT32_T *);
XLLP_BOOL_T XllpSetUpTSInterrupts(volatile XLLP_SSP_REGS *,volatile XLLP_BCR_T *);
XLLP_BOOL_T XllpGetPenStatus(volatile XLLP_BCR_T *);
*/
#endif
