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
/******************************************************************************
**
**  COPYRIGHT (C) 2000, 2001 Intel Corporation.
**
**  This software as well as the software described in it is furnished under 
**  license and may only be used or copied in accordance with the terms of the 
**  license. The information in this file is furnished for informational use 
**  only, is subject to change without notice, and should not be construed as 
**  a commitment by Intel Corporation. Intel Corporation assumes no 
**  responsibility or liability for any errors or inaccuracies that may appear 
**  in this document or any software that may be provided in association with 
**  this document. 
**  Except as permitted by such license, no part of this document may be 
**  reproduced, stored in a retrieval system, or transmitted in any form or by 
**  any means without the express written consent of Intel Corporation. 
**
**  FILENAME:       xllp_ethernet.h
**
**  PURPOSE:        This is the main header file for the LAN91C111 Ethernet
**                  Controller.
**
**
******************************************************************************/

#ifndef _xllp_ethernet_h
#define _xllp_ethernet_h

/*
*******************************************************************************
*   HEADER FILES
*******************************************************************************
*/
#include "xllp_defs.h"
#include "xllp_reg_base_os_depend.h"

#if defined(POST_BUILD)
#include <stdio.h>
#include <string.h>
#elif defined(WinCE_BUILD)
#include <windows.h>
#include <halether.h>
#endif

/*
*******************************************************************************
*    Bank Select Field
*******************************************************************************
*/
#define XLLP_LAN91C111_BANK_SELECT	    14    // Byte Offset to Bank Select Register
#define XLLP_LAN91C111_BANK_SELECT_16	7     // Word Offset to Bank Select Register
#define XLLP_LAN91C111_BANK_SELECT_32   3     // Dword Offset to Bank Select Register
#define XLLP_LAN91C111_BANKSEL_MASK     (0x7U << 0)
#define XLLP_LAN91C111_BANKSEL_SHIFT    16
#define BANK0               0x00
#define BANK1               0x01
#define BANK2               0x02
#define BANK3               0x03
#define BANK7               0x07

/*
*******************************************************************************
*    EEPROM Addresses.
*******************************************************************************
*/
#define EEPROM_MAC_OFFSET_1    0x6020   // This value is used to read IA0-1 from EEPROM
#define EEPROM_MAC_OFFSET_2    0x6021   // This value is used to read IA2-3 from EEPROM
#define EEPROM_MAC_OFFSET_3    0x6022   // This value is used to read IA4-5 from EEPROM

#define EEPROM_MAC_OFFSET_1_W  0x4020   // This value is used to write IA0-1 to EEPROM
#define EEPROM_MAC_OFFSET_2_W  0x4021   // This value is used to write IA2-3 to EEPROM
#define EEPROM_MAC_OFFSET_3_W  0x4022   // This value is used to write IA4-5 to EEPROM

/*
*******************************************************************************
*    Bank 0 Register Map in I/O Space
*******************************************************************************
*/
#define XLLP_LAN91C111_TCR          0        // Transmit Control Register
#define XLLP_LAN91C111_EPH_STATUS   2        // EPH Status Register
#define XLLP_LAN91C111_RCR          4        // Receive Control Register
#define XLLP_LAN91C111_COUNTER      6        // Counter Register
#define XLLP_LAN91C111_MIR          8        // Memory Information Register
#define XLLP_LAN91C111_RPCR         10       // Receive/PHY Control Register

/*
*******************************************************************************
*    Transmit Control Register - Bank 0 - Offset 0
*******************************************************************************
*/
#define XLLP_LAN91C111_TCR_TXENA        (0x1U << 0)
#define XLLP_LAN91C111_TCR_LOOP         (0x1U << 1)
#define XLLP_LAN91C111_TCR_FORCOL       (0x1U << 2)
#define XLLP_LAN91C111_TCR_TXP_EN       (0x1U << 3)
#define XLLP_LAN91C111_TCR_PAD_EN       (0x1U << 7)
#define XLLP_LAN91C111_TCR_NOCRC        (0x1U << 8)
#define XLLP_LAN91C111_TCR_MON_CSN      (0x1U << 10)
#define XLLP_LAN91C111_TCR_FDUPLX       (0x1U << 11)
#define XLLP_LAN91C111_TCR_STP_SQET     (0x1U << 12)
#define XLLP_LAN91C111_TCR_EPH_LOOP     (0x1U << 13)
#define XLLP_LAN91C111_TCR_SWFDUP       (0x1U << 15)

/*
*******************************************************************************
*    EPH Status Register - Bank 0 - Offset 2
*******************************************************************************
*/
#define XLLP_LAN91C111_EPHSR_TX_SUC     (0x1U << 0)
#define XLLP_LAN91C111_EPHSR_SNGL_COL   (0x1U << 1)
#define XLLP_LAN91C111_EPHSR_MUL_COL    (0x1U << 2)
#define XLLP_LAN91C111_EPHSR_LTX_MULT   (0x1U << 3)
#define XLLP_LAN91C111_EPHSR_16COL      (0x1U << 4)
#define XLLP_LAN91C111_EPHSR_SQET       (0x1U << 5)
#define XLLP_LAN91C111_EPHSR_LTX_BRD    (0x1U << 6)
#define XLLP_LAN91C111_EPHSR_TX_DEFR    (0x1U << 7)
#define XLLP_LAN91C111_EPHSR_LATCOL     (0x1U << 9)
#define XLLP_LAN91C111_EPHSR_LOST_CARR  (0x1U << 10)
#define XLLP_LAN91C111_EPHSR_EXC_DEF    (0x1U << 11)
#define XLLP_LAN91C111_EPHSR_CTR_ROL    (0x1U << 12)
#define XLLP_LAN91C111_EPHSR_LINK_OK    (0x1U << 14)
#define XLLP_LAN91C111_EPHSR_TX_UNRN    (0x1U << 15)

#define XLLP_LAN91C111_EPHSR_ERRORS     (XLLP_LAN91C111_EPHSR_SNGL_COL |    \
                                   XLLP_LAN91C111_EPHSR_MUL_COL   |    \
                                   XLLP_LAN91C111_EPHSR_16COL     |    \
                                   XLLP_LAN91C111_EPHSR_SQET      |    \
                                   XLLP_LAN91C111_EPHSR_TX_DEFR   |    \
                                   XLLP_LAN91C111_EPHSR_LATCOL    |    \
                                   XLLP_LAN91C111_EPHSR_LOST_CARR |    \
                                   XLLP_LAN91C111_EPHSR_EXC_DEF   |    \
                                   XLLP_LAN91C111_EPHSR_LINK_OK   |    \
                                   XLLP_LAN91C111_EPHSR_TX_UNRN)

/*
*******************************************************************************
*    Receive Control Register - Bank 0 - Offset 4
*******************************************************************************
*/
#define XLLP_LAN91C111_RCR_RX_ABORT     (0x1U << 0)
#define XLLP_LAN91C111_RCR_PRMS         (0x1U << 1)
#define XLLP_LAN91C111_RCR_ALMUL        (0x1U << 2)
#define XLLP_LAN91C111_RCR_RXEN         (0x1U << 8)
#define XLLP_LAN91C111_RCR_STRIP_CRC    (0x1U << 9)
#define XLLP_LAN91C111_RCR_ABORT_ENB    (0x1U << 13)
#define XLLP_LAN91C111_RCR_FILT_CAR     (0x1U << 14)
#define XLLP_LAN91C111_RCR_SOFT_RST     (0x1U << 15)

/*
*******************************************************************************
*    Counter Register - Bank 0 - Offset 6
*******************************************************************************
*/
#define XLLP_LAN91C111_ECR_SNGL_COL     (0xFU << 0)
#define XLLP_LAN91C111_ECR_MULT_COL     (0xFU << 5)
#define XLLP_LAN91C111_ECR_DEF_TX       (0xFU << 8)
#define XLLP_LAN91C111_ECR_EXC_DEF_TX   (0xFU << 12)

/*
*******************************************************************************
*    Memory Information Register - Bank 0 - OFfset 8
*******************************************************************************
*/
#define XLLP_LAN91C111_MIR_SIZE        (0x1 << 2)    // 8K bytes

/*
*******************************************************************************
*    Receive/PHY Control Register - Bank 0 - Offset 10
*******************************************************************************
*/
#define XLLP_LAN91C111_RPCR_ANEG       (0x1U << 11)
#define XLLP_LAN91C111_RPCR_DPLX       (0x1U << 12)
#define XLLP_LAN91C111_RPCR_SPEED      (0x1U << 13)

#define XLLP_LAN91C111_RPCR_LSX0A      (0x0U << 5)   // Link detected (default)
#define XLLP_LAN91C111_RPCR_LSX2A      (0x2U << 5)   // 10Mbps Link detected
#define XLLP_LAN91C111_RPCR_LSX3A      (0x3U << 5)   // Full Duplex Mode Enabled
#define XLLP_LAN91C111_RPCR_LSX4A      (0x4U << 5)   // Transmit or Receive packet occured
#define XLLP_LAN91C111_RPCR_LSX5A      (0x5U << 5)   // 100Mbps Link detected
#define XLLP_LAN91C111_RPCR_LSX6A      (0x6U << 5)   // Receive packet occured
#define XLLP_LAN91C111_RPCR_LSX7A      (0x7U << 5)   // Transmit packet occured
#define XLLP_LAN91C111_RPCR_LSX0B      (0x0U << 2)   // Link detected (default)
#define XLLP_LAN91C111_RPCR_LSX2B      (0x2U << 2)   // 10Mbps Link detected
#define XLLP_LAN91C111_RPCR_LSX3B      (0x3U << 2)   // Full Duplex Mode Enabled
#define XLLP_LAN91C111_RPCR_LSX4B      (0x4U << 2)   // Transmit or Receive packet occured
#define XLLP_LAN91C111_RPCR_LSX5B      (0x5U << 2)   // 100Mbps Link detected
#define XLLP_LAN91C111_RPCR_LSX6B      (0x6U << 2)   // Receive packet occured
#define XLLP_LAN91C111_RPCR_LSX7B      (0x7U << 2)   // Transmit packet occured

/*
*******************************************************************************
*    Bank 1 Register Map in I/O Space
*******************************************************************************
*/
#define XLLP_LAN91C111_CONFIG       0        // Configuration Register
#define XLLP_LAN91C111_BASE         2        // Base Address Register
#define XLLP_LAN91C111_IA0          4        // Individual Address Register - 0
#define XLLP_LAN91C111_IA1          5        // Individual Address Register - 1
#define XLLP_LAN91C111_IA2          6        // Individual Address Register - 2
#define XLLP_LAN91C111_IA3          7        // Individual Address Register - 3
#define XLLP_LAN91C111_IA4          8        // Individual Address Register - 4
#define XLLP_LAN91C111_IA5          9        // Individual Address Register - 5
#define XLLP_LAN91C111_GEN_PURPOSE  10       // General Address Registers
#define XLLP_LAN91C111_CONTROL      12       // Control Register

/*
*******************************************************************************
*    Configuration Register - Bank 1 - Offset 0
********************************************************************************
*/
#define XLLP_LAN91C111_CR_EXT_PHY       (0x1U << 9)
#define XLLP_LAN91C111_CR_GPCNTRL       (0x1U << 10)
#define XLLP_LAN91C111_CR_NO_WAIT       (0x1U << 12)
#define XLLP_LAN91C111_CR_EPH_POWER_EN  (0x1U << 15)

/*
*******************************************************************************
*    Base Address Register - Bank 1 - Offset 2
*******************************************************************************
*/
#define XLLP_LAN91C111_BAR_A_BITS       (0xFFU << 8)

/*
*******************************************************************************
*    Control Register - Bank 1 - Offset 12
*******************************************************************************
*/
#define XLLP_LAN91C111_CTR_STORE        (0x1U << 0)
#define XLLP_LAN91C111_CTR_RELOAD       (0x1U << 1)
#define XLLP_LAN91C111_CTR_EEPROM       (0x1U << 2)
#define XLLP_LAN91C111_CTR_TE_ENABLE    (0x1U << 5)
#define XLLP_LAN91C111_CTR_CR_ENABLE    (0x1U << 6)
#define XLLP_LAN91C111_CTR_LE_ENABLE    (0x1U << 7)
#define XLLP_LAN91C111_CTR_AUTO_RELEASE (0x1U << 11)
#define XLLP_LAN91C111_CTR_RCV_BAD      (0x1U << 14)

/*
*******************************************************************************
*    Bank 2 Register Map in I/O Space
*******************************************************************************
*/
#define XLLP_LAN91C111_MMU            0      // MMU Command Register
#define XLLP_LAN91C111_PNR            2      // Packet Number Register
#define XLLP_LAN91C111_ARR            3      // Allocation Result Register
#define XLLP_LAN91C111_FIFO           4      // FIFO Ports Register
#define XLLP_LAN91C111_POINTER        6      // Pointer Register
#define XLLP_LAN91C111_DATA_HIGH      8      // Data High Register 
#define XLLP_LAN91C111_DATA_LOW       10     // Data Low Register
#define XLLP_LAN91C111_INT_STATS      12     // Interrupt Status Register - RO
#define XLLP_LAN91C111_INT_ACK        12     // Interrupt Acknowledge Register -WO
#define XLLP_LAN91C111_INT_MASK       13     // Interrupt Mask Register

/*
*******************************************************************************
*    MMU Command Register - Bank 2 - Offset 0
*******************************************************************************
*/
#define XLLP_LAN91C111_MMUCR_NO_BUSY    (0x1U << 0)
#define XLLP_LAN91C111_MMUCR_ALLOC_TX   (0x1U << 5)    // XYZ = 001
#define XLLP_LAN91C111_MMUCR_RESET_MMU  (0x2U << 5)    // XYZ = 010
#define XLLP_LAN91C111_MMUCR_REMOVE_RX  (0x3U << 5)    // XYZ = 011
#define XLLP_LAN91C111_MMUCR_REM_REL_RX (0x4U << 5)    // XYZ = 100
#define XLLP_LAN91C111_MMUCR_RELEASE_RX (0x5U << 5)    // XYZ = 101
#define XLLP_LAN91C111_MMUCR_RELEASE_TX (0x5U << 5)    // XYZ = 101
#define XLLP_LAN91C111_MMUCR_ENQUEUE    (0x6U << 5)    // XYZ = 110
#define XLLP_LAN91C111_MMUCR_RESET_TX   (0x7U << 5)    // XYZ = 111

/*
*******************************************************************************
*    Packet Number Register - Bank 2 - Offset 2
*******************************************************************************
*/
#define XLLP_LAN91C111_PNR_TX           (0x3FU << 0)

/*
*******************************************************************************
*    Allocation Result Register - Bank 2 - Offset 3
*******************************************************************************
*/
#define XLLP_LAN91C111_ARR_ALLOC_PN     (0x3FU << 0)
#define XLLP_LAN91C111_ARR_FAILED       (0x1U << 7)

/*
*******************************************************************************
*    FIFO Ports Register - Bank 2 - Offset 4
*******************************************************************************
*/
#define XLLP_LAN91C111_FIFO_TX_DONE_PN  (0x3FU << 0)
#define XLLP_LAN91C111_FIFO_TEMPTY      (0x1U << 7)
#define XLLP_LAN91C111_FIFO_RX_DONE_PN  (0x3FU << 8)
#define XLLP_LAN91C111_FIFO_RXEMPTY     (0x1U << 15)

/*
*******************************************************************************
*    Pointer Register - Bank 2 - Offset 6
*******************************************************************************
*/
#define XLLP_LAN91C111_PTR_LOW          (0xFFU << 0)
#define XLLP_LAN91C111_PTR_HIGH         (0x7U << 8)
#define XLLP_LAN91C111_PTR_NOT_EMPTY    (0x1U << 11)
#define XLLP_LAN91C111_PTR_ETEN         (0x1U << 12)
#define XLLP_LAN91C111_PTR_READ         (0x1U << 13)
#define XLLP_LAN91C111_PTR_AUTO_INCR    (0x1U << 14)
#define XLLP_LAN91C111_PTR_RCV          (0x1U << 15)

#define XLLP_LAN91C111_PTR_RX_FRAME     (XLLP_LAN91C111_PTR_RCV      |    \
                                   XLLP_LAN91C111_PTR_AUTO_INCR |    \
                                   XLLP_LAN91C111_PTR_READ)

#define XLLP_LAN91C111_PTR_RX_FRAME_NO_AUTO_INC  (XLLP_LAN91C111_PTR_RCV | XLLP_LAN91C111_PTR_READ)

/*
*******************************************************************************
*    Data Register - Bank 2 - Offset 8
*******************************************************************************
*/
#define XLLP_LAN91C111_CONTROL_CRC      (0x1U << 4)    // CRC bit
#define XLLP_LAN91C111_CONTROL_ODD      (0x1U << 5)    // ODD bit

/*
*******************************************************************************
*    Interrupt Status Register - Bank 2 - Offset 12
*******************************************************************************
*/
#define XLLP_LAN91C111_IST_RCV_INT      (0x1U << 0)
#define XLLP_LAN91C111_IST_TX_INT       (0x1U << 1)
#define XLLP_LAN91C111_IST_TX_EMPTY_INT (0x1U << 2)
#define XLLP_LAN91C111_IST_ALLOC_INT    (0x1U << 3)
#define XLLP_LAN91C111_IST_RX_OVRN_INT  (0x1U << 4)
#define XLLP_LAN91C111_IST_EPH_INT      (0x1U << 5)
#define XLLP_LAN91C111_IST_ERCV_INT     (0x1U << 6)
#define XLLP_LAN91C111_IST_MD_INT       (0x1U << 7)

/*
*******************************************************************************
*    Interrupt Acknowledge Register - Bank 2 - Offset 12
*******************************************************************************
*/
#define XLLP_LAN91C111_ACK_TX_INT       (0x1U << 1)
#define XLLP_LAN91C111_ACK_TX_EMPTY_INT (0x1U << 2)
#define XLLP_LAN91C111_ACK_RX_OVRN_INT  (0x1U << 4)
#define XLLP_LAN91C111_ACK_ERCV_INT     (0x1U << 6)
#define XLLP_LAN91C111_ACK_MD_INT       (0x1U << 7)

/*
*******************************************************************************
*    Interrupt Mask Register - Bank 2 - Offset 12
*******************************************************************************
*/
#define XLLP_LAN91C111_MSK_RCV_INT      (0x1U << 0)
#define XLLP_LAN91C111_MSK_TX_INT       (0x1U << 1)
#define XLLP_LAN91C111_MSK_TX_EMPTY_INT (0x1U << 2)
#define XLLP_LAN91C111_MSK_ALLOC_INT    (0x1U << 3)
#define XLLP_LAN91C111_MSK_RX_OVRN_INT  (0x1U << 4)
#define XLLP_LAN91C111_MSK_EPH_INT      (0x1U << 5)
#define XLLP_LAN91C111_MSK_ERCV_INT     (0x1U << 6)
#define XLLP_LAN91C111_MSK_MD_INT       (0x1U << 7)

/*
*******************************************************************************
*    Bank 3 Register Map in I/O Space
*******************************************************************************
*/
#define XLLP_LAN91C111_MULTICAST0     0      // Multicast Table 0
#define XLLP_LAN91C111_MULTICAST1     1      // Multicast Table 1
#define XLLP_LAN91C111_MULTICAST2     2      // Multicast Table 2
#define XLLP_LAN91C111_MULTICAST3     3      // Multicast Table 3
#define XLLP_LAN91C111_MULTICAST4     4      // Multicast Table 4
#define XLLP_LAN91C111_MULTICAST5     5      // Multicast Table 5
#define XLLP_LAN91C111_MULTICAST6     6      // Multicast Table 6
#define XLLP_LAN91C111_MULTICAST7     7      // Multicast Table 7
#define XLLP_LAN91C111_MGMT           8      // Management Interface
#define XLLP_LAN91C111_REVISION       10     // Revision Register
#define XLLP_LAN91C111_ERCV           12     // Early Rcv Register

/*
*******************************************************************************
*    Management Interface - Bank 3 -Offset 8
*******************************************************************************
*/
#define XLLP_LAN91C111_MGMT_MDO         (0x1U << 0)
#define XLLP_LAN91C111_MGMT_MDI         (0x1U << 1)
#define XLLP_LAN91C111_MGMT_MCLK        (0x1U << 2)
#define XLLP_LAN91C111_MGMT_MDOE        (0x1U << 3)
#define XLLP_LAN91C111_MGMT_MSK_CRS100  (0x3U << 14)

/*
*******************************************************************************
*    Revision Register - Bank 3 - Offset 10
*******************************************************************************
*/
#define XLLP_LAN91C111_REV_REVID        (0xFU << 0)
#define XLLP_LAN91C111_REV_CHIPID       (0xFU << 4)

/*
*******************************************************************************
*    Early RCV Register - Bank 3 - Offset 12
*******************************************************************************
*/
#define XLLP_LAN91C111_ERCV_THRESHOLD   (0x1FU << 0)
#define XLLP_LAN91C111_ERCV_RCV_DISCRD  (0x1U << 7)

/*
*******************************************************************************
*    PHY Registers 
*******************************************************************************
*/
#define XLLP_LAN91C111_CONTROL_MII_DIS           0x3000
#define XLLP_LAN91C111_CONTROL_LPBK              0x7000

/*
*******************************************************************************
*    Receive Frame Status Word
*******************************************************************************
*/
#define XLLP_LAN91C111_TOO_SHORT        (0x1U << 10)
#define XLLP_LAN91C111_TOO_LONG         (0x1U << 11)
#define XLLP_LAN91C111_ODD_FRM          (0x1U << 12)
#define XLLP_LAN91C111_BAD_CRC          (0x1U << 13)
#define XLLP_LAN91C111_BROD_CAST        (0x1U << 14)
#define XLLP_LAN91C111_ALGN_ERR         (0x1U << 15)

#ifdef NO_BROADCAST
#define XLLP_LAN91C111_FRAME_FILTER (XLLP_LAN91C111_TOO_SHORT | \
                                   XLLP_LAN91C111_TOO_LONG  | \
                                   XLLP_LAN91C111_BAD_CRC   | \
                                   XLLP_LAN91C111_BROD_CAST | \
                                   XLLP_LAN91C111_ALGN_ERR)
#else
#define XLLP_LAN91C111_FRAME_FILTER (XLLP_LAN91C111_TOO_SHORT | \
                                   XLLP_LAN91C111_TOO_LONG  | \
                                   XLLP_LAN91C111_BAD_CRC   | \
                                   XLLP_LAN91C111_ALGN_ERR)
#endif // NO_BROADCAST

/*
*******************************************************************************
*    Default I/O Signature - 0x33
*******************************************************************************
*/
#define XLLP_LAN91C111_LOW_SIGNATURE        (0x33U << 0)
#define XLLP_LAN91C111_HIGH_SIGNATURE       (0x33U << 8)
#define XLLP_LAN91C111_SIGNATURE (XLLP_LAN91C111_HIGH_SIGNATURE | XLLP_LAN91C111_LOW_SIGNATURE) 

/*
*******************************************************************************
*    LAN91C111 Address Space
*******************************************************************************
*/
#define XLLP_LAN91C111_REG_BASE_DEFAULT  0x300

/*
*******************************************************************************
*    Error codes.
*******************************************************************************
*/
#if defined(POST_BUILD) || defined(WinCE_BUILD)
#define ERR_L_LAN91C111         0x5u
#define ERR_T_TIMEOUT           0x003   // Timeout
#define ERR_T_NODEVICE          0x004   // A device is not present or cannot be initialized
#define ERR_T_BADRANGE          0x008   // bad range - some number or computation is out of range
#define ERR_T_NORECEIVE         0x009   // Some receiver cannot receive data
#define ERR_T_NOTRANSMIT        0x00A   // some transmitter cannot transmit data
#define ERR_T_ILLALIGN          0x00B   // Illegal alignment
#define ERR_T_UNEXPECTED        0x00E   // Unexpected result returned from device
#define ERR_T_CRC               0x01C   // CRC error.
#endif

/*
*******************************************************************************
*    Sub-location error codes.
*******************************************************************************
*/
#define ERR_S_INV_DEV           0x01
#define ERR_S_BAD_MAC           0x02
#define ERR_S_EEPROM            0x03
#define ERR_S_RESET             0x04
#define ERR_S_ALLOC             0x05
#define ERR_S_DEALLOC           0x06
#define ERR_S_TRANSMIT          0x07
#define ERR_S_RECEIVE           0x08
#define ERR_S_RELEASE           0x09
#define ERR_S_FILL_RAM          0x0A

#define ERR_TS_TXTEST           0x80     // Error codes in EthernetTransmitTest()
#define ERR_TS_LOOPTEST         0x81     // Error codes in PostLANControllerLoopBack()
#define ERR_TS_MACADDR          0x82     // Error codes in PostLANControllerLoopBack()

/*
*******************************************************************************
*    Timeout Definitions
*******************************************************************************
*/
#define XLLP_LAN91C111_TO_EEPROM     2        // Timeout for access the EEPROM
#define XLLP_LAN91C111_TO_ALLOC      2        // Timeout for buffer allocations
#define XLLP_LAN91C111_TO_TRANSMIT   2        // Timeout for transmit

/*
*******************************************************************************
*    Manifest Constants
*******************************************************************************
*/
#define XLLP_LAN91C111_MAX_PAGES         4        // Maximum number of 2K pages.
#define XLLP_LAN91C111_MIR_DEFAULT       0x0404
#define XLLP_LAN91C111_MIR_RELEASE       0x0004
#define XLLP_LAN91C111_FIFO_DEFAULT      0x8080
#define XLLP_LAN91C111_POINTER_WR        0x4000
#define XLLP_LAN91C111_POINTER_RD        0x6000

/*
************************************************************************************
*					DATA TYPES 
************************************************************************************
*/

typedef enum XllpEthernetLoopbackTypeE
{
	XLLP_ETHERNET_EPH_LOOPBACK = 1,
	XLLP_ETHERNET_PHY_HALF_LOOPBACK,
	XLLP_ETHERNET_PHY_FULL_LOOPBACK,
    XLLP_ETHERNET_PHY_EXT_LOOPBACK
}  XllpEthernetLoopbackTypeT;

typedef enum XllpEthernetLoopbackEnableE
{
	XLLP_ETHERNET_LOOPBACK_DISABLE,
	XLLP_ETHERNET_LOOPBACK_ENABLE
}  XllpEthernetLoopbackEnableT;

typedef struct XllpEthernetHeaderS
{
    XLLP_UINT16_T destMAC[3];
    XLLP_UINT16_T srcMAC[3];
    XLLP_UINT16_T frameType;
} XllpEthernetHeaderT;

/*
*******************************************************************************
*    Network definitions.
*******************************************************************************
*/
#define XLLP_ETHERNET_TYPE_IP           0x0800
#define XLLP_ETHERNET_TYPE_ARP          0x0806
#define XLLP_ETHERNET_TYPE_ARP_MSB      0x08
#define XLLP_ETHERNET_TYPE_ARP_LSB      0x06
#define XLLP_ETHERNET_MIN_LENGTH        60
#define XLLP_ETHERNET_MAX_LENGTH        2048
#define XLLP_ETHERNET_CRC_LEN           4

#if !defined(WinCE_BUILD)
#define ntohl(x) htonl(x)
#define ntohs(x) htons(x)

__inline XLLP_UINT16_T htons(XLLP_UINT16_T _val)
{
    return (_val << 8) | (_val >> 8);
}
__inline XLLP_UINT32_T htonl(XLLP_UINT32_T _val)
{
    return (_val << 24) | ((_val&0xff00) << 8) |
           ((_val&0xff0000) >> 8) | (_val >> 24);
}
#endif

/*
*******************************************************************************
*    External Functions
*******************************************************************************
*/

#if defined(POST_BUILD) || defined(WinCE_BUILD)
#define DM_CW_LAN91C111_0 	8
#define DM_CW_LAN91C111_1 	9
#endif

#if defined(POST_BUILD)

// Debug printf definitions
#define LOGERROR LogError

// Debug routines in POST
void DM_ControlWordsDebugPrintPutBit(XLLP_UINT32_T, XLLP_UINT32_T);
void DM_CwDbgPrintf(XLLP_UINT32_T debugFlag, char *, ...);

// Error log function defined in POST
extern XLLP_UINT32_T LogError(P_XLLP_UINT32_T logerror, XLLP_UINT32_T where,
                              XLLP_UINT32_T subwhere, XLLP_UINT32_T type,
                              XLLP_UINT32_T param1, XLLP_UINT32_T param2,
                              XLLP_UINT32_T param3);

#elif defined(WinCE_BUILD)
//void DM_CwDbgPrintf(XLLP_UINT_T contrlWord, P_XLLP_INT8_T fmt, ...);

// Debug printf definitions defined under WinCE
#define printf KITLOutputDebugString
#define DM_ControlWordsDebugPrintPutBit

// Error log function defined as NULL
#define LOGERROR(_logerr,_where,_sub_where,_type,_param1,_param2,_param3) 

// Debug printr for WinCE

__inline void DM_CwDbgPrintf(XLLP_UINT32_T contrlWord, P_XLLP_INT8_T fmt, ...)
{
/*
    XLLP_INT8_T pstring[100];
    va_list ap;
    void *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10, *p11, *p12;

    va_start(ap, fmt);
    p1 = va_arg(ap, void *);
    p2 = va_arg(ap, void *);
    p3 = va_arg(ap, void *);
    p4 = va_arg(ap, void *);
    p5 = va_arg(ap, void *);
    p6 = va_arg(ap, void *);
    p7 = va_arg(ap, void *);
    p8 = va_arg(ap, void *);
    p9 = va_arg(ap, void *);
    p10 = va_arg(ap, void *);
    p11 = va_arg(ap, void *);
    p12 = va_arg(ap, void *);
    va_end(ap);

    sprintf(pstring, fmt, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
    KITLOutputDebugString(pstring);
*/
}


#endif

/*
************************************************************************************
*					FUNCTION PROTOTYPES 
************************************************************************************
*/
XLLP_UINT16_T XllpEthernetReadPhyRegister(XLLP_UINT8_T phyaddr, XLLP_UINT8_T phyreg);
void XllpEthernetWritePhyRegister(XLLP_UINT8_T phyaddr, 
                                  XLLP_UINT8_T phyreg,
	                              XLLP_UINT16_T phydata);
void XllpEthernetVerifyRAMBuffer (XLLP_UINT32_T bufferSize);
void XllpEthernetDisplayTxStatus(void);
XLLP_UINT32_T XllpEthernetGetEEPROMMACAddress(P_XLLP_UINT16_T macAddressP);
XLLP_UINT32_T XllpEthernetStoreEEPROMMACAddress(P_XLLP_UINT16_T macAddressP);
void XllpEthernetGetMACAddress(P_XLLP_UINT16_T macAddressP);
void XllpEthernetStoreMACAddress(P_XLLP_UINT16_T macAddressP);
void XllpEthernetSetLoopBack(XllpEthernetLoopbackTypeT type, XllpEthernetLoopbackEnableT enable);
void XllpEthernetDumpFrame(P_XLLP_UINT8_T frame, XLLP_UINT16_T length);
XLLP_UINT32_T XllpEthernetTransmitPacket(P_XLLP_UINT16_T buffer, XLLP_UINT32_T length);
XLLP_UINT32_T XllpEthernetReceiveStatus(P_XLLP_UINT16_T rxIntStatusP);
XLLP_UINT32_T XllpEthernetReceivePacket(P_XLLP_UINT32_T buffer, P_XLLP_INT32_T rxCount);
void XllpEthernetSetMulticastList (P_XLLP_UINT8_T multicastAddrP, XLLP_UINT32_T addrCntr);
void XllpEthernetDumpRegisters(void);
XLLP_UINT32_T XllpEthernetHWSetup(P_XLLP_UINT16_T macAddressP,
                                  XLLP_BOOL_T autoRelease,
                                  XLLP_BOOL_T autoNegotiate,
                                  XllpEthernetLoopbackEnableT enable);
void XllpEthernetHWShutdown(void);
void XllpEthernetSWInit(void* ioRegsP);
void XllpEthernetGetRxIntFlag (P_XLLP_UINT16_T rxIntStatus);
void XllpEthernetTraceEnable(P_XLLP_INT8_T flag);
void XllpEthernetErrorEnable(P_XLLP_INT8_T flag);

void XllpEthernetEnableRxInterrupt(void);
void XllpEthernetDisableRxInterrupt(void);

#if defined(WinCE_BUILD)
XLLP_UINT32_T XllpEternetDeallocateTxPacket(void);
extern void XllpEthernetSelectRegisterBank (XLLP_UINT32_T bank);
extern XLLP_UINT8_T XllpEthernetReadByte(XLLP_UINT32_T offset);
void XllpEthernetWriteByte(XLLP_UINT8_T value, XLLP_UINT32_T offset);
#endif


#endif /* _xllp_ethernet_h */
