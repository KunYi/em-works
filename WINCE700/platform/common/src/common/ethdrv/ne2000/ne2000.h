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
/*++


Module Name:

    ne2000hw.h

--*/

#ifndef _NE2000HARDWARE_
#define _NE2000HARDWARE_


//
// Offsets from Adapter->IoPAddr of the ports used to access
// the 8390 NIC registers.
//
// The names in parenthesis are the abbreviations by which
// the registers are referred to in the 8390 data sheet.
//
// Some of the offsets appear more than once
// because they have have relevant page 0 and page 1 values,
// or they are different registers when read than they are
// when written. The notation MSB indicates that only the
// MSB can be set for this register, the LSB is assumed 0.
//

#define NIC_COMMAND         0x0     // (CR)
#define NIC_PAGE_START      0x1     // (PSTART)   MSB, write-only
#define NIC_PHYS_ADDR       0x1     // (PAR0)     page 1
#define NIC_PAGE_STOP       0x2     // (PSTOP)    MSB, write-only
#define NIC_BOUNDARY        0x3     // (BNRY)     MSB
#define NIC_XMIT_START      0x4     // (TPSR)     MSB, write-only
#define NIC_XMIT_STATUS     0x4     // (TSR)      read-only
#define NIC_XMIT_COUNT_LSB  0x5     // (TBCR0)    write-only
#define NIC_XMIT_COUNT_MSB  0x6     // (TBCR1)    write-only
#define NIC_FIFO            0x6     // (FIFO)     read-only
#define NIC_INTR_STATUS     0x7     // (ISR)
#define NIC_CURRENT         0x7     // (CURR)     page 1
#define NIC_MC_ADDR         0x8     // (MAR0)     page 1
#define NIC_CRDA_LSB        0x8     // (CRDA0)
#define NIC_RMT_ADDR_LSB    0x8     // (RSAR0)
#define NIC_CRDA_MSB        0x9     // (CRDA1)
#define NIC_RMT_ADDR_MSB    0x9     // (RSAR1)
#define NIC_RMT_COUNT_LSB   0xa     // (RBCR0)    write-only
#define NIC_RMT_COUNT_MSB   0xb     // (RBCR1)    write-only
#define NIC_RCV_CONFIG      0xc     // (RCR)      write-only
#define NIC_RCV_STATUS      0xc     // (RSR)      read-only
#define NIC_XMIT_CONFIG     0xd     // (TCR)      write-only
#define NIC_FAE_ERR_CNTR    0xd     // (CNTR0)    read-only
#define NIC_DATA_CONFIG     0xe     // (DCR)      write-only
#define NIC_CRC_ERR_CNTR    0xe     // (CNTR1)    read-only
#define NIC_INTR_MASK       0xf     // (IMR)      write-only
#define NIC_MISSED_CNTR     0xf     // (CNTR2)    read-only
#define NIC_RACK_NIC        0x10    // Byte to read or write
#define NIC_RESET           0x1f    // (RESET)


//
// Constants for the NIC_COMMAND register.
//
// Start/stop the card, start transmissions, and select
// which page of registers was seen through the ports.
//

#define CR_STOP         (UCHAR)0x01        // reset the card
#define CR_START        (UCHAR)0x02        // start the card
#define CR_XMIT         (UCHAR)0x04        // begin transmission
#define CR_NO_DMA       (UCHAR)0x20        // stop remote DMA

#define CR_PS0          (UCHAR)0x40        // low bit of page number
#define CR_PS1          (UCHAR)0x80        // high bit of page number
#define CR_PAGE0        (UCHAR)0x00        // select page 0
#define CR_PAGE1        CR_PS0             // select page 1
#define CR_PAGE2        CR_PS1             // select page 2

#define CR_DMA_WRITE    (UCHAR)0x10        // Write
#define CR_DMA_READ     (UCHAR)0x08        // Read
#define CR_SEND         (UCHAR)0x18        // send


//
// Constants for the NIC_XMIT_STATUS register.
//
// Indicate the result of a packet transmission.
//

#define TSR_XMIT_OK     (UCHAR)0x01        // transmit with no errors
#define TSR_COLLISION   (UCHAR)0x04        // collided at least once
#define TSR_ABORTED     (UCHAR)0x08        // too many collisions
#define TSR_NO_CARRIER  (UCHAR)0x10        // carrier lost
#define TSR_NO_CDH      (UCHAR)0x40        // no collision detect heartbeat


//
// Constants for the NIC_INTR_STATUS register.
//
// Indicate the cause of an interrupt.
//

#define ISR_EMPTY       (UCHAR)0x00        // no bits set in ISR
#define ISR_RCV         (UCHAR)0x01        // packet received with no errors
#define ISR_XMIT        (UCHAR)0x02        // packet transmitted with no errors
#define ISR_RCV_ERR     (UCHAR)0x04        // error on packet reception
#define ISR_XMIT_ERR    (UCHAR)0x08        // error on packet transmission
#define ISR_OVERFLOW    (UCHAR)0x10        // receive buffer overflow
#define ISR_COUNTER     (UCHAR)0x20        // MSB set on tally counter
#define ISR_DMA_DONE    (UCHAR)0x40        // RDC
#define ISR_RESET       (UCHAR)0x80        // (not an interrupt) card is reset


//
// Constants for the NIC_RCV_CONFIG register.
//
// Configure what type of packets are received.
//

#define RCR_REJECT_ERR  (UCHAR)0x00        // reject error packets
#define RCR_BROADCAST   (UCHAR)0x04        // receive broadcast packets
#define RCR_MULTICAST   (UCHAR)0x08        // receive multicast packets
#define RCR_ALL_PHYS    (UCHAR)0x10        // receive ALL directed packets
#define RCR_MONITOR     (UCHAR)0x20        // don't collect packets


//
// Constants for the NIC_RCV_STATUS register.
//
// Indicate the status of a received packet.
//
// These are also used to interpret the status byte in the
// packet header of a received packet.
//

#define RSR_PACKET_OK   (UCHAR)0x01        // packet received with no errors
#define RSR_CRC_ERROR   (UCHAR)0x02        // packet received with CRC error
#define RSR_MULTICAST   (UCHAR)0x20        // packet received was multicast
#define RSR_DISABLED    (UCHAR)0x40        // received is disabled
#define RSR_DEFERRING   (UCHAR)0x80        // receiver is deferring


//
// Constants for the NIC_XMIT_CONFIG register.
//
// Configures how packets are transmitted.
//

#define TCR_NO_LOOPBACK (UCHAR)0x00        // normal operation
#define TCR_LOOPBACK    (UCHAR)0x02        // loopback (set when NIC is stopped)

#define TCR_INHIBIT_CRC (UCHAR)0x01        // inhibit appending of CRC

#define TCR_NIC_LBK     (UCHAR)0x02        // loopback through the NIC
#define TCR_SNI_LBK     (UCHAR)0x04        // loopback through the SNI
#define TCR_COAX_LBK    (UCHAR)0x06        // loopback to the coax


//
// Constants for the NIC_DATA_CONFIG register.
//
// Set data transfer sizes.
//

#define DCR_BYTE_WIDE   (UCHAR)0x00        // byte-wide DMA transfers
#define DCR_WORD_WIDE   (UCHAR)0x01        // word-wide DMA transfers

#define DCR_LOOPBACK    (UCHAR)0x00        // loopback mode (TCR must be set)
#define DCR_NORMAL      (UCHAR)0x08        // normal operation

#define DCR_FIFO_2_BYTE (UCHAR)0x00        // 2-byte FIFO threshhold
#define DCR_FIFO_4_BYTE (UCHAR)0x20        // 4-byte FIFO threshhold
#define DCR_FIFO_8_BYTE (UCHAR)0x40        // 8-byte FIFO threshhold
#define DCR_FIFO_12_BYTE (UCHAR)0x60       // 12-byte FIFO threshhold
#define DCR_AUTO_INIT   (UCHAR)0x10        // Auto-init to remove packets from ring


//
// Constants for the NIC_INTR_MASK register.
//
// Configure which ISR settings actually cause interrupts.
//
#define IMR_RCV         (UCHAR)0x01        // packet received with no errors
#define IMR_XMIT        (UCHAR)0x02        // packet transmitted with no errors
#define IMR_RCV_ERR     (UCHAR)0x04        // error on packet reception
#define IMR_XMIT_ERR    (UCHAR)0x08        // error on packet transmission
#define IMR_OVERFLOW    (UCHAR)0x10        // receive buffer overflow
#define IMR_COUNTER     (UCHAR)0x20        // MSB set on tally counter

////////////////////////////////////////////////////////////////////////////
//
//  8390 registers and other adapter specific constants
//
////////////////////////////////////////////////////////////////////////////

#define MAX_FRAME_SIZE 1514
#define MIN_FRAME_SIZE 60
#define RCV_BLK_SIZE 256
#define MAX_LOOKAHEAD_LENGTH 256
#define NIC_HEADER_LEN 4

// Network Interface Controller commands.
#define NIC_Transmit        0x26
#define NIC_MaskByte        0x00
#define NIC_UnMaskByte      0x1f
#define NIC_PacketReady     0x15
#define NIC_TxComplete      0x0a
#define NIC_Page0       0x22
#define NIC_Page1       0x62
#define NIC_Page1Stop       0x61
#define NIC_ReceiveConfigValue  0x0e
#define NIC_RemoteDmaComplete   0x40
#define NIC_RemoteDmaWr     0x12
#define NIC_AbortDmaWr      0x32
#define NIC_AbortDmaRd      0x2a
#define NIC_RemoteDmaRd     0x0a
#define NIC_AbortDmaStop    0x21
#define NIC_AbortDmaWarmStop    0x23


// Command Register
#define NIC_CMD_STOP        0x01
#define NIC_CMD_START       0x02
#define NIC_CMD_XMIT        0x04
#define NIC_CMD_READ        0x08
#define NIC_CMD_WRITE       0x10
#define NIC_CMD_SEND        0x18
#define NIC_CMD_ABORT       0x20
#define NIC_CMD_PAGE0       0x00
#define NIC_CMD_PAGE1       0x40
#define NIC_CMD_PAGE2       0x80

// Receive Configuration Register masks
#define NIC_RECEIVE_ERR_PACKETS 0x01
#define NIC_RECEIVE_RUNTS   0x02
#define NIC_RECEIVE_BROADCAST   0x04
#define NIC_RECEIVE_MULTICAST   0x08
#define NIC_RECEIVE_PHYS    0x10
#define NIC_RECEIVE_MONITOR_MODE 0x20

// Receive Status Register masks (also packet header receive status)
#define NIC_RCV_STAT_OK     0x01
#define NIC_RCV_STAT_CRC    0x02
#define NIC_RCV_STAT_ALIGN  0x04
#define NIC_RCV_STAT_FIFO   0x08
#define NIC_RCV_STAT_MISSED 0x10
#define NIC_RCV_STAT_MC_BC  0x20
#define NIC_RCV_STAT_DISABLED   0x40
#define NIC_RCV_STAT_DEFERRING  0x80

// Data Configuration Register masks
#define NIC_DCR_FIFO        0x40    // 8 bytes
// #define NIC_DCR_FIFO     0x00    // 2 bytes
// #define NIC_DCR_FIFO     0x60    // 12 bytes
#define NIC_DCR_AUTO_INIT   0x10
#define NIC_DCR_NORMAL      0x08
#define NIC_DCR_WORD_XFER   0x01

// Transmit Configuration Register masks
#define NIC_TCR_NORMAL      0x00
#define NIC_TCR_LOOPBACK1   0x02    // loopback mode 1

// Interrupt Status Register
#define NIC_ISR_PACKET_RCVD     0x01
#define NIC_ISR_PACKET_XMITD        0x02
#define NIC_ISR_RCV_ERR         0x04
#define NIC_ISR_XMIT_ERR        0x08
#define NIC_ISR_OVERWRITE       0x10
#define NIC_ISR_COUNTER_OVERFLOW    0x20
#define NIC_ISR_DMA_COMPLETE        0x40
#define NIC_ISR_RESET_STATUS        0x80
#define NIC_ISR_MOREWORK    NIC_ISR_PACKET_XMITD | NIC_ISR_PACKET_RCVD | \
                NIC_ISR_OVERWRITE | NIC_ISR_XMIT_ERR

// Transmit status register
#define NIC_TSR_SUCCESS     0x01
#define NIC_TSR_COLLIDED    0x04
#define NIC_TSR_ABORTED     0x08
#define NIC_TSR_NO_CARRIER  0x10    // carrier sense lost
#define NIC_TSR_UNDERRUN    0x20

#define NUM_MC_REG 8

#ifdef UNDER_CE

#define ADAPTER_OFFSET  (0x100/4)

#define LED_GETFRAME_ENTRY    (ADAPTER_OFFSET+1)
#define LED_GETFRAME_EXIT     (ADAPTER_OFFSET+2)
#define LED_SENDFRAME_ENTRY   (ADAPTER_OFFSET+3)
#define LED_SENDFRAME_EXIT    (ADAPTER_OFFSET+4)
#define LED_COPY              (ADAPTER_OFFSET+5)
#define LED_PKTLEN            (ADAPTER_OFFSET+6)
#define LED_OVERRUN           (ADAPTER_OFFSET+7)
#else
#define KITL_DEBUGLED(offset, value)
#endif // UNDER_CE


#endif // _NE2000HARDWARE_
