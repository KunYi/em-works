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
//  Header:  omap5912_uart.h
//
#ifndef __OMAP5912_UART_H
#define __OMAP5912_UART_H


//------------------------------------------------------------------------------

#define		UART_MEMORY_ALIGNMENT							4

typedef volatile struct {

    union {                             // 0000
        UINT8 RHR;
        UINT8 THR;
        UINT8 DLL;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0000[UART_MEMORY_ALIGNMENT-1];
#endif

	union {                             // 0001
        UINT8 IER;
        UINT8 DLH;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0001[UART_MEMORY_ALIGNMENT-1];
#endif

    union {                             // 0002
        UINT8 IIR;
        UINT8 FCR;
        UINT8 EFR;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0002[UART_MEMORY_ALIGNMENT-1];
#endif

    UINT8 LCR;                          // 0003

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0003[UART_MEMORY_ALIGNMENT-1];
#endif

    union {                             // 0004
        UINT8 MCR;
        UINT8 XON1;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0004[UART_MEMORY_ALIGNMENT-1];
#endif

    union {                             // 0005
        UINT8 LSR;
        UINT8 XON2;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0005[UART_MEMORY_ALIGNMENT-1];
#endif

    union {                             // 0006
        UINT8 MSR;
        UINT8 TCR;
        UINT8 XOFF1;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0006[UART_MEMORY_ALIGNMENT-1];
#endif

    union {                             // 0007
        UINT8 SPR;
        UINT8 TLR;
        UINT8 XOFF2;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0007[UART_MEMORY_ALIGNMENT-1];
#endif

    UINT8 MDR1;                         // 0008

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0008[UART_MEMORY_ALIGNMENT-1];
#endif

    UINT8 MDR2;                         // 0009

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0009[UART_MEMORY_ALIGNMENT-1];
#endif

    union {                             // 000A
        UINT8 SFLSR;
        UINT8 TXFLL;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED000A[UART_MEMORY_ALIGNMENT-1];
#endif

    union {                             // 000B
        UINT8 RESUME;
        UINT8 TXFLH;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED000B[UART_MEMORY_ALIGNMENT-1];
#endif

    union {                             // 000C
        UINT8 SFREGL;
        UINT8 RXFLL;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED000C[UART_MEMORY_ALIGNMENT-1];
#endif

    union {                             // 000D
        UINT8 SFREGH;
        UINT8 RXFHL;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED000D[UART_MEMORY_ALIGNMENT-1];
#endif

    union {                             // 000E
        UINT8 BLR;
        UINT8 UASR;
    };

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED000E[UART_MEMORY_ALIGNMENT-1];
#endif

    UINT8 ACREG;                        // 000F

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED000F[UART_MEMORY_ALIGNMENT-1];
#endif

    UINT8 SCR;                          // 0010

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0010[UART_MEMORY_ALIGNMENT-1];
#endif

    UINT8 SSR;                          // 0011

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0011[UART_MEMORY_ALIGNMENT-1];
#endif

    UINT8 EBLR;                         // 0012

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0012[UART_MEMORY_ALIGNMENT-1];
#endif

#ifndef UART_MEMORY_ALIGNMENT
    UINT8 UNUSED0013;
#endif

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0013[UART_MEMORY_ALIGNMENT];
#endif

    UINT8 MVR;                          // 0014

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0014[UART_MEMORY_ALIGNMENT-1];
#endif

    UINT8 SYSC;                         // 0015

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0015[UART_MEMORY_ALIGNMENT-1];
#endif

    UINT8 SYSS;                         // 0016

#ifdef UART_MEMORY_ALIGNMENT	
	UCHAR UNUSED0016[UART_MEMORY_ALIGNMENT-1];
#endif

    UINT8 WER;                          // 0017
    
} OMAP5912_UART_REGS;

//------------------------------------------------------------------------------

#define UART_LCR_DLAB                           (1 << 7)
#define UART_LCR_DIV_EN                         (1 << 7)
#define UART_LCR_BREAK_EN                       (1 << 6)
#define UART_LCR_PARITY_TYPE_2                  (1 << 5)
#define UART_LCR_PARITY_TYPE_1                  (1 << 4)
#define UART_LCR_PARITY_FORCE_0                 (3 << 4)
#define UART_LCR_PARITY_FORCE_1                 (2 << 4)
#define UART_LCR_PARITY_EVEN                    (1 << 4)
#define UART_LCR_PARITY_ODD                     (0 << 4)
#define UART_LCR_PARITY_EN                      (1 << 3)
#define UART_LCR_NB_STOP                        (1 << 2)
#define UART_LCR_CHAR_LENGTH_8BIT               (3 << 0)
#define UART_LCR_CHAR_LENGTH_7BIT               (2 << 0)
#define UART_LCR_CHAR_LENGTH_6BIT               (1 << 0)
#define UART_LCR_CHAR_LENGTH_5BIT               (0 << 0)

#define UART_LSR_RX_ERROR                       0x1E

#define UART_LSR_RX_FIFO_STS                    (1 << 7)
#define UART_LSR_TX_SR_E                        (1 << 6)
#define UART_LSR_TX_FIFO_E                      (1 << 5)
#define UART_LSR_RX_BI                          (1 << 4)
#define UART_LSR_RX_FE                          (1 << 3)
#define UART_LSR_RX_PE                          (1 << 2)
#define UART_LSR_RX_OE                          (1 << 1)
#define UART_LSR_RX_FIFO_E                      (1 << 0)

#define UART_MCR_TCR_TLR                        (1 << 6)
#define UART_MCR_XON_EN                         (1 << 5)
#define UART_MCR_LOOPBACK_EN                    (1 << 4)
#define UART_MCR_CD_STS_CH                      (1 << 3)
#define UART_MCR_RI_STS_CH                      (1 << 2)
#define UART_MCR_RTS                            (1 << 1)
#define UART_MCR_DTR                            (1 << 0)

#define UART_TCR_RX_FIFO_TRIG_START_0           (0 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_4           (1 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_8           (2 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_12          (3 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_16          (4 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_20          (5 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_24          (6 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_28          (7 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_32          (8 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_36          (9 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_40          (10 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_44          (11 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_48          (12 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_52          (13 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_56          (14 << 4)
#define UART_TCR_RX_FIFO_TRIG_START_60          (15 << 4)
#define UART_TCR_RX_FIFO_TRIG_HALT_0            (0 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_4            (1 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_8            (2 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_12           (3 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_16           (4 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_20           (5 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_24           (6 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_28           (7 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_32           (8 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_36           (9 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_40           (10 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_44           (11 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_48           (12 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_52           (13 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_56           (14 << 0)
#define UART_TCR_RX_FIFO_TRIG_HALT_60           (15 << 0)

#define UART_TLR_RX_FIFO_TRIG_DMA_0             (0 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_4             (1 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_8             (2 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_12            (3 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_16            (4 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_20            (5 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_24            (6 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_28            (7 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_32            (8 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_36            (9 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_40            (10 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_44            (11 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_48            (12 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_52            (13 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_56            (14 << 4)
#define UART_TLR_RX_FIFO_TRIG_DMA_60            (15 << 4)
#define UART_TLR_TX_FIFO_TRIG_DMA_0             (0 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_4             (1 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_8             (2 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_12            (3 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_16            (4 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_20            (5 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_24            (6 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_28            (7 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_32            (8 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_36            (9 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_40            (10 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_44            (11 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_48            (12 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_52            (13 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_56            (14 << 0)
#define UART_TLR_TX_FIFO_TRIG_DMA_60            (15 << 0)

#define UART_MSR_NCD                            (1 << 7)
#define UART_MSR_NRI                            (1 << 6)
#define UART_MSR_NDSR                           (1 << 5)
#define UART_MSR_NCTS                           (1 << 4)
#define UART_MSR_DCD                            (1 << 3)
#define UART_MSR_RI                             (1 << 2)
#define UART_MSR_DSR                            (1 << 1)
#define UART_MSR_CTS                            (1 << 0)

#define UART_IER_CST                            (1 << 7)
#define UART_IER_RTS                            (1 << 6)
#define UART_IER_XOFF                           (1 << 5)
#define UART_IER_SPEEP_MODE                     (1 << 4)
#define UART_IER_MODEM                          (1 << 3)
#define UART_IER_LINE                           (1 << 2)
#define UART_IER_THR                            (1 << 1)
#define UART_IER_RHR                            (1 << 0)

#define UART_IER_IRDA_RHR                       (1 << 0)
#define UART_IER_IRDA_THR                       (1 << 1)
#define UART_IER_IRDA_RX_LAST_BYTE              (1 << 2)
#define UART_IER_IRDA_RX_OE                     (1 << 3)
#define UART_IER_IRDA_STS_FIFO                  (1 << 4)
#define UART_IER_IRDA_TX_STATUS                 (1 << 5)
#define UART_IER_IRDA_LINE_STATUS               (1 << 6)
#define UART_IER_IRDA_EOF                       (1 << 7)

#define UART_IIR_MODEM                          (0 << 1)
#define UART_IIR_THR                            (1 << 1)
#define UART_IIR_RHR                            (2 << 1)
#define UART_IIR_LINE                           (3 << 1)
#define UART_IIR_TO                             (6 << 1)
#define UART_IIR_XOFF                           (8 << 1)
#define UART_IIR_HW                             (16 << 1)
#define UART_IIR_IT_PENDING                     (1 << 0)

#define UART_IIR_IRDA_RHR                       (1 << 0)
#define UART_IIR_IRDA_THR                       (1 << 1)
#define UART_IIR_IRDA_RX_LAST_BYTE              (1 << 2)
#define UART_IIR_IRDA_RX_OE                     (1 << 3)
#define UART_IIR_IRDA_STS_FIFO                  (1 << 4)
#define UART_IIR_IRDA_TX_STATUS                 (1 << 5)
#define UART_IIR_IRDA_LINE_STATUS               (1 << 6)
#define UART_IIR_IRDA_EOF                       (1 << 7)

#define UART_FCR_RX_FIFO_TRIG_8                 (0 << 6)
#define UART_FCR_RX_FIFO_TRIG_16                (1 << 6)
#define UART_FCR_RX_FIFO_TRIG_56                (2 << 6)
#define UART_FCR_RX_FIFO_TRIG_60                (3 << 6)
#define UART_FCR_TX_FIFO_TRIG_8                 (0 << 4)
#define UART_FCR_TX_FIFO_TRIG_16                (1 << 4)
#define UART_FCR_TX_FIFO_TRIG_56                (2 << 4)
#define UART_FCR_TX_FIFO_TRIG_60                (3 << 4)
#define UART_FCR_DMA_MODE                       (1 << 3)
#define UART_FCR_TX_FIFO_CLEAR                  (1 << 2)
#define UART_FCR_RX_FIFO_CLEAR                  (1 << 1)
#define UART_FCR_FIFO_EN                        (1 << 0)

#define UART_SCR_RX_TRIG_GRANU1                 (1 << 7)
#define UART_SCR_TX_TRIG_GRANU1                 (1 << 6)
#define UART_SCR_DSR_IT                         (1 << 5)
#define UART_SCR_RX_CTS_DSR_WAKE_UP_ENABLE      (1 << 4)
#define UART_SCR_TX_EMPTY_CTL                   (1 << 3)
#define UART_SCR_DMA_MODE_2_MODE0               (0 << 1)
#define UART_SCR_DMA_MODE_2_MODE1               (1 << 1)
#define UART_SCR_DMA_MODE_2_MODE2               (2 << 1)
#define UART_SCR_DMA_MODE_2_MODE3               (3 << 1)
#define UART_SCR_DMA_MODE_CTL                   (1 << 0)

#define UART_SSR_TX_FIFO_FULL                   (1 << 0)

#define UART_SYSC_IDLE_FORCE                    (0 << 3)
#define UART_SYSC_IDLE_DISABLED                 (1 << 3)
#define UART_SYSC_IDLE_SMART                    (2 << 3)
#define UART_SYSC_WAKEUP_ENABLE                 (1 << 2)
#define UART_SYSC_RST                           (1 << 1)
#define UART_SYSC_AUTOIDLE                      (1 << 0)

#define UART_SYSS_RST_DONE                      (1 << 0)

#define UART_MDR1_UART16                        (0 << 0)
#define UART_MDR1_SIR                           (1 << 0)
#define UART_MDR1_UART16AUTO                    (2 << 0)
#define UART_MDR1_UART13                        (3 << 0)
#define UART_MDR1_MIR                           (4 << 0)
#define UART_MDR1_FIR                           (5 << 0)
#define UART_MDR1_DISABLE                       (7 << 0)

#define UART_EFR_AUTO_CTS_EN                    (1 << 7)
#define UART_EFR_AUTO_RTS_EN                    (1 << 6)
#define UART_EFR_SPECIAL_CHAR_DETECT            (1 << 5)
#define UART_EFR_ENHANCED_EN                    (1 << 4)
#define UART_EFR_SW_FLOW_CONTROL_TX_NONE        (0 << 2)
#define UART_EFR_SW_FLOW_CONTROL_TX_XONOFF1     (2 << 2)
#define UART_EFR_SW_FLOW_CONTROL_TX_XONOFF2     (1 << 2)
#define UART_EFR_SW_FLOW_CONTROL_TX_XONOFF12    (3 << 2)
#define UART_EFR_SW_FLOW_CONTROL_RX_NONE        (0 << 0)
#define UART_EFR_SW_FLOW_CONTROL_RX_XONOFF1     (2 << 0)
#define UART_EFR_SW_FLOW_CONTROL_RX_XONOFF2     (1 << 0)
#define UART_EFR_SW_FLOW_CONTROL_RX_XONOFF12    (3 << 0)



//------------------------------------------------------------------------------

#endif // __OMAP5912_UART_H
