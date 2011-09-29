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
//  File:  omap2420_usbd.h
//
#ifndef __OMAP2420_USBD_H
#define __OMAP2420_USBD_H

//------------------------------------------------------------------------------

#define USBD_EP_COUNT   16
#define USBD_TOTAL_EP_COUNT    USBD_EP_COUNT
#define USBD_NONZERO_EP_COUNT  (USBD_EP_COUNT - 1)  // Number of EPs not including EP0

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 REV;                             // 0000 - Revision
    UINT32 EP_NUM;                          // 0004 - Endpoint selection
    UINT32 DATA;                            // 0008 - Data
    UINT32 CTRL;                            // 000C - Control
    UINT32 STAT_FLG;                        // 0010 - Status
    UINT32 RXFSTAT;                         // 0014 - Receive FIFO status
    UINT32 SYSCON1;                         // 0018 - System configuration 1
    UINT32 SYSCON2;                         // 001C - System configuration 2
    UINT32 DEVSTAT;                         // 0020 - Device status
    UINT32 SOF;                             // 0024 - Start of frame
    UINT32 IRQ_EN;                          // 0028 - Interrupt enable
    UINT32 DMA_IRQ_EN;                      // 002C - DMA interrupt enable
    UINT32 IRQ_SRC;                         // 0030 - Interrupt source
    UINT32 EP_STAT;                         // 0034 - Non-ISO endpoint interrupt enable
    UINT32 DMA_STAT;                        // 0038 - Non-ISO DMA interrupt enable
    UINT32 RESERVED_003C;                   // 003C
    UINT32 RXDMA_CFG;                       // 0040 - DMA receive channels config
    UINT32 TXDMA_CFG;                       // 0044 - DMA transmit channels config
    UINT32 DATA_DMA;                        // 0048 - DMA FIFO data
    UINT32 RESERVED_004C;                   // 004C
    UINT32 TXDMA0;                          // 0050 - Transmit DMA control 0
    UINT32 TXDMA1;                          // 0054 - Transmit DMA control 1
    UINT32 TXDMA2;                          // 0058 - Transmit DMA control 2
    UINT32 RESERVED_005C;                   // 005C
    UINT32 RXDMA0;                          // 0060 - Receive DMA control 0
    UINT32 RXDMA1;                          // 0064 - Receive DMA control 0
    UINT32 RXDMA2;                          // 0068 - Receive DMA control 0
    UINT32 RESERVED_006C[5];                // 006C
    UINT32 EP0;                             // 0080 - Endpoint 0 configuration
    UINT32 EP_RX[USBD_NONZERO_EP_COUNT];    // 0084 - Receive endpoint 1..15 config
    UINT32 RESERVED_00C0;                   // 00C0
    UINT32 EP_TX[USBD_NONZERO_EP_COUNT];    // 00C4 - Transmit endpoint 1..15 config
} OMAP2420_USBD_REGS;

//------------------------------------------------------------------------------

#define USBD_EP_VALID               (1 << 15)       // EP is valid
#define USBD_EP_DB                  (1 << 14)       // Use double buffer
#define USBD_EP_ISO                 (1 << 11)       // EP is ISO

#define USBD_IRQ_EN_SOF             (1 << 7)        // SOF interrupt
#define USBD_IRQ_EN_EP_RX           (1 << 5)        // EPn receive interrupt
#define USBD_IRQ_EN_EP_TX           (1 << 4)        // EPn transmit interrupt
#define USBD_IRQ_EN_DS_CHG          (1 << 3)        // Device status changed
#define USBD_IRQ_EN_EP0             (1 << 0)        // EP0 interrupt

#define USBD_INT_TX_DONE            (1 << 10)       // DMA channel n Done
#define USBD_INT_RX_CNT             (1 << 9)        // DMA channel n Cnt
#define USBD_INT_RX_EOT             (1 << 8)        // DMA channel n EOT
#define USBD_INT_SOF                (1 << 7)        // SOF interrupt
#define USBD_INT_EP_RX              (1 << 5)        // EPn receive interrupt
#define USBD_INT_EP_TX              (1 << 4)        // EPn transmit interrupt
#define USBD_INT_DS_CHG             (1 << 3)        // Device status changed
#define USBD_INT_SETUP              (1 << 2)        // Set up
#define USBD_INT_EP0_RX             (1 << 1)        // EP0 receive
#define USBD_INT_EP0_TX             (1 << 0)        // EP0 transmit

#define USBD_EP_NUM_SETUP           (1 << 6)        // Setup selection
#define USBD_EP_NUM_SEL             (1 << 5)        // EP selection
#define USBD_EP_NUM_DIRIN           (1 << 4)        // EP direction - IN
#define USBD_EP_NUM                 0x000F          // EP number

#define USBD_CTRL_CLR_HALT          (1 << 7)        // Clear halt
#define USBD_CTRL_SET_HALT          (1 << 6)        // Set halt
#define USBD_CTRL_CLR_DATA_TOGGLE   (1 << 3)        // Clear data toggle bit
#define USBD_CTRL_FIFO_EN           (1 << 2)        // Enable FIFO
#define USBD_CTRL_CLR_EP            (1 << 1)        // Clear EP
#define USBD_CTRL_RESET_EP          (1 << 0)        // Reset EP

#define USBD_DEVSTAT_R_WK_OK        (1 << 6)        // Remote wakeup enabled
#define USBD_DEVSTAT_USB_RESET      (1 << 5)        // USB device reset
#define USBD_DEVSTAT_SUS            (1 << 4)        // Device suspended
#define USBD_DEVSTAT_CFG            (1 << 3)        // Device configured
#define USBD_DEVSTAT_ADD            (1 << 2)        // Device addresses
#define USBD_DEVSTAT_DEF            (1 << 1)        // Device defined
#define USBD_DEVSTAT_ATT            (1 << 0)        // Device attached

#define USBD_STAT_MISS_IN           (1 << 14)       // Missed IN token (ISO)
#define USBD_STAT_DATA_FLUSH        (1 << 13)       // Data flush (ISO)
#define USBD_STAT_ISO_ERR           (1 << 12)       // ISO error
#define USBD_STAT_ISO_FIFO_EMPTY    (1 << 9)        // ISO fifo empty
#define USBD_STAT_ISO_FIFO_FULL     (1 << 8)        // ISO fifo full
#define USBD_STAT_HALTED            (1 << 6)        // EP halted
#define USBD_STAT_STALL             (1 << 5)        // EP stalled
#define USBD_STAT_NAK               (1 << 4)        // NACK
#define USBD_STAT_ACK               (1 << 3)        // ACK
#define USBD_STAT_FIFO_EN           (1 << 2)        // FIFO enable
#define USBD_STAT_FIFO_EMPTY        (1 << 1)        // FIFO empty
#define USBD_STAT_FIFO_FULL         (1 << 0)        // FIFO full

#define USBD_RFXSTAT_COUNT          0x3FF           // FIFO count mask

#define USBD_SYSCON1_CFG_LOCK       (1 << 8)        // Configure locked
#define USBD_SYSCON1_NAK_EN         (1 << 4)        // NAK enable
#define USBD_SYSCON1_AUTODEC_DIS    (1 << 3)        // Autodecode disabled
#define USBD_SYSCON1_SELF_PWR       (1 << 2)        // Self powered
#define USBD_SYSCON1_SOFF_DIS       (1 << 1)        // Shutoff disabled
#define USBD_SYSCON1_PULLUP_EN      (1 << 0)        // Pull-up enable

#define USBD_SYSCON2_RMT_WKP        (1 << 6)        // Remote wakeup
#define USBD_SYSCON2_STALL_CMD      (1 << 5)        // Stall command
#define USBD_SYSCON2_DEV_CFG        (1 << 3)        // Device configured
#define USBD_SYSCON2_CLR_CFG        (1 << 2)        // Clear configured

//------------------------------------------------------------------------------

#endif
