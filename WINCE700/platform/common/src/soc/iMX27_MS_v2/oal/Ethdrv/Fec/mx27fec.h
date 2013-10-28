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
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006,2007 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// Header: MX27FEC.h
//
// Provides Definitions for FEC.
//
//------------------------------------------------------------------------------
#ifndef __MX27FEC_H__
#define __MX27FEC_H__

#ifdef __cplusplus
extern "C" {
#endif

// Hash creation constants.
//
#define CRC_PRIME           0xFFFFFFFF
#define CRC_POLYNOMIAL      0xEDB88320
#define HASH_BITS           6

#define ETHER_ADDR_SIZE       6   // Size of Ethernet address
#define ETHER_HDR_SIZE        14  // Size of Ethernet header
//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
#define BD_SC_EMPTY     ((ushort)0x8000)        /* Recieve is empty */
#define BD_SC_READY     ((ushort)0x8000)        /* Transmit is ready */
#define BD_SC_WRAP      ((ushort)0x2000)        /* Last buffer descriptor */
#define BD_SC_INTRPT    ((ushort)0x1000)        /* Interrupt on change */
#define BD_SC_CM        ((ushort)0x0200)        /* Continous mode */
#define BD_SC_ID        ((ushort)0x0100)        /* Rec'd too many idles */
#define BD_SC_P         ((ushort)0x0100)        /* xmt preamble */
#define BD_SC_BR        ((ushort)0x0020)        /* Break received */
#define BD_SC_FR        ((ushort)0x0010)        /* Framing error */
#define BD_SC_PR        ((ushort)0x0008)        /* Parity error */
#define BD_SC_OV        ((ushort)0x0002)        /* Overrun */
#define BD_SC_CD        ((ushort)0x0001)        /* ?? */


// Buffer descriptor control/status used by Ethernet receive

#define BD_ENET_RX_EMPTY        ((USHORT)0x8000)
#define BD_ENET_RX_WRAP         ((USHORT)0x2000)
#define BD_ENET_RX_INTR         ((USHORT)0x1000)
#define BD_ENET_RX_LAST         ((USHORT)0x0800)
#define BD_ENET_RX_FIRST        ((USHORT)0x0400)
#define BD_ENET_RX_MISS         ((USHORT)0x0100)
#define BD_ENET_RX_LG           ((USHORT)0x0020)
#define BD_ENET_RX_NO           ((USHORT)0x0010)
#define BD_ENET_RX_SH           ((USHORT)0x0008)
#define BD_ENET_RX_CR           ((USHORT)0x0004)
#define BD_ENET_RX_OV           ((USHORT)0x0002)
#define BD_ENET_RX_CL           ((USHORT)0x0001)
#define BD_ENET_RX_STATS        ((USHORT)0x013f)        /* All status bits */

// Buffer descriptor control/status used by Ethernet transmit.

#define BD_ENET_TX_READY        ((USHORT)0x8000)
#define BD_ENET_TX_PAD          ((USHORT)0x4000)
#define BD_ENET_TX_WRAP         ((USHORT)0x2000)
#define BD_ENET_TX_INTR         ((USHORT)0x1000)
#define BD_ENET_TX_LAST         ((USHORT)0x0800)
#define BD_ENET_TX_TC           ((USHORT)0x0400)
#define BD_ENET_TX_DEF          ((USHORT)0x0200)
#define BD_ENET_TX_HB           ((USHORT)0x0100)
#define BD_ENET_TX_LC           ((USHORT)0x0080)
#define BD_ENET_TX_RL           ((USHORT)0x0040)
#define BD_ENET_TX_RCMASK       ((USHORT)0x003c)
#define BD_ENET_TX_UN           ((USHORT)0x0002)
#define BD_ENET_TX_CSL          ((USHORT)0x0001)
#define BD_ENET_TX_STATS        ((USHORT)0x03ff)        /* All status bits */

// the buffer descriptor structure for the FEC
typedef struct  _BUFFER_DESC
{
    USHORT  DataLen;
    USHORT  ControlStatus;
    ULONG   BufferAddr;
}BUFFER_DESC,  *PBUFFER_DESC;


typedef struct _FEC_PHY_INFO
{
    UINT PhyId;
    WCHAR PhyName[256];

}FEC_PHY_INFO, *PFEC_PHY_INFO;


// Make MII read/write commands for the external PHY
#define MII_READ_COMMAND(reg)           CSP_BITFVAL(FEC_MMFR_ST, FEC_MMFR_ST_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_OP, FEC_MMFR_OP_READ)|\
                                        CSP_BITFVAL(FEC_MMFR_TA, FEC_MMFR_TA_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_RA, reg & 0x1f)

#define MII_WRITE_COMMAND(reg, val)     CSP_BITFVAL(FEC_MMFR_ST, FEC_MMFR_ST_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_OP, FEC_MMFR_OP_WRITE)|\
                                        CSP_BITFVAL(FEC_MMFR_TA, FEC_MMFR_TA_VALUE)|\
                                        CSP_BITFVAL(FEC_MMFR_RA, reg & 0x1f)|\
                                        CSP_BITFVAL(FEC_MMFR_DATA, val & 0xffff)


// Register definitions for the PHY

#define MII_REG_CR          0  /* Control Register                         */
#define MII_REG_SR          1  /* Status Register                          */
#define MII_REG_PHYIR1      2  /* PHY Identification Register 1            */
#define MII_REG_PHYIR2      3  /* PHY Identification Register 2            */
#define MII_REG_ANAR        4  /* A-N Advertisement Register               */
#define MII_REG_ANLPAR      5  /* A-N Link Partner Ability Register        */
#define MII_REG_ANER        6  /* A-N Expansion Register                   */
#define MII_REG_ANNPTR      7  /* A-N Next Page Transmit Register          */
#define MII_REG_ANLPRNPR    8  /* A-N Link Partner Received Next Page Reg. */

// values for PHY status

#define PHY_MAX_COUNT   32

#define PHY_CONF_ANE    0x0001  /* 1 auto-negotiation enabled */
#define PHY_CONF_LOOP   0x0002  /* 1 loopback mode enabled */
#define PHY_CONF_SPMASK 0x00f0  /* mask for speed */
#define PHY_CONF_10HDX  0x0010  /* 10 Mbit half duplex supported */
#define PHY_CONF_10FDX  0x0020  /* 10 Mbit full duplex supported */
#define PHY_CONF_100HDX 0x0040  /* 100 Mbit half duplex supported */
#define PHY_CONF_100FDX 0x0080  /* 100 Mbit full duplex supported */

#define PHY_STAT_LINK   0x0100  /* 1 up - 0 down */
#define PHY_STAT_FAULT  0x0200  /* 1 remote fault */
#define PHY_STAT_ANC    0x0400  /* 1 auto-negotiation complete  */
#define PHY_STAT_SPMASK 0xf000  /* mask for speed */
#define PHY_STAT_10HDX  0x1000  /* 10 Mbit half duplex selected */
#define PHY_STAT_10FDX  0x2000  /* 10 Mbit full duplex selected */
#define PHY_STAT_100HDX 0x4000  /* 100 Mbit half duplex selected */
#define PHY_STAT_100FDX 0x8000  /* 100 Mbit full duplex selected */


// AMD AM79C874 phy

// Specific register definitions for the Am79c874
//#define MII_AM79C874_MFR       16  /* Miscellaneous Feature Register */
//#define MII_AM79C874_ICSR      17  /* Interrupt/Status Register      */
//#define MII_AM79C874_DR        18  /* Diagnostic Register            */
//#define MII_AM79C874_PMLR      19  /* Power and Loopback Register    */
//#define MII_AM79C874_MCR       21  /* ModeControl Register           */
//#define MII_AM79C874_DC        23  /* Disconnect Counter             */
//#define MII_AM79C874_REC       24  /* Recieve Error Counter          */


// SMCS LAN8700 phy

// Specific register definitions for the SMCS LAN8700
#define MII_LAN8700_SRR        16  /* Silicon Revision Register            */
#define MII_LAN8700_MCSR       17  /* Mode Control/Status Register         */
#define MII_LAN8700_SMR        18  /* Special Modes  Register              */
#define MII_LAN8700_SECR       26  /* Symbol Error Counter Register        */
#define MII_LAN8700_CSIR       27  /* Control/Status Indication Register   */
#define MII_LAN8700_SITC       28  /* Special Internal Testbility  Register*/
#define MII_LAN8700_ICR        29  /* Interrupt Sources Control  Register  */
#define MII_LAN8700_IMR        30  /* Interrupt Mark Register              */
#define MII_LAN8700_SCSR       31  /* PHY Special Control/Status Register  */

#define MIIPHYSPEED                 14
#define LAN8700_10MBPS_HALFDUPLEX   1
#define LAN8700_10MBPS_FULLDUPLEX   5
#define LAN8700_100MBPS_HALFDUPLEX  2
#define LAN8700_100MBPS_FULLDUPLEX  6
#define PHY_LINKSTATUS              0x0004
#define PHY_AUTONEGO_ENABLE         0x1200
#define MMI_DATA_MASK               0xFFFF
#define GALR_CLEAR_ALL              0xFFFFFFFF
#define GAUR_CLEAR_ALL              0xFFFFFFFF
#define FEC_FRAME_SEND_TIMEOUT      2000
#define FEC_DMA_BUFFER_SIZE         (16 * 1024)


//For LAN8700 Hardware with Revision number 3
FEC_PHY_INFO LAN8700Info = {
    0x7c0c3,
    TEXT("LAN8700"),

};

//For LAN8700 Hardware with Revision number 4
FEC_PHY_INFO LAN8700Info_Rev1 = {
    0x7c0c4,
    TEXT("LAN8700"),

};

// We support SMCS LAN8700
FEC_PHY_INFO *PhyInfo[] = {
    &LAN8700Info,
    &LAN8700Info_Rev1,
    NULL
};


typedef enum _FEC_PHY_ID {
    LAN8700            = 1,
    FEC_PHY_MAX_ID
} FEC_PHY_ID;

#ifdef __cplusplus
    }
#endif

#endif // __MX27FEC_H__
