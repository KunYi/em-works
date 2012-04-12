//------------------------------------------------------------------------------
//
// Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//
// Header: ENET.h
//
// Provides Definitions for ENET.
//
//------------------------------------------------------------------------------
#ifndef __ENET_H__
#define __ENET_H__

#ifdef __cplusplus
extern "C" {
#endif

// Hash creation constants.
//
#define CRC_PRIME             0xFFFFFFFF
#define CRC_POLYNOMIAL        0xEDB88320
#define HASH_BITS             6

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

// the buffer descriptor structure for the ENET
typedef struct  _BUFFER_DESC
{
    USHORT  DataLen;
    USHORT  ControlStatus;
    ULONG   BufferAddr;
}BUFFER_DESC,  *PBUFFER_DESC;


typedef struct _ENET_PHY_INFO
{
    UINT  PhyId;
    WCHAR PhyName[256];
    
}ENET_PHY_INFO, *PENET_PHY_INFO;

// Make MII read/write commands for the external PHY
#define MII_READ_COMMAND(reg)           (ENET_MMFR_ST_VALUE << BP_ENET_MAC_MMFR_ST)|(ENET_MMFR_OP_READ  << BP_ENET_MAC_MMFR_OP)|(ENET_MMFR_TA_VALUE << BP_ENET_MAC_MMFR_TA)|((reg & 0x1f) << BP_ENET_MAC_MMFR_RA)


#define MII_WRITE_COMMAND(reg, val)     (ENET_MMFR_ST_VALUE << BP_ENET_MAC_MMFR_ST)|(ENET_MMFR_OP_WRITE << BP_ENET_MAC_MMFR_OP)|(ENET_MMFR_TA_VALUE << BP_ENET_MAC_MMFR_TA)|((reg & 0x1f) << BP_ENET_MAC_MMFR_RA)|(val & 0xffff)
                                        

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
#define MII_AM79C874_MFR       16  /* Miscellaneous Feature Register */
#define MII_AM79C874_ICSR      17  /* Interrupt/Status Register      */
#define MII_AM79C874_DR        18  /* Diagnostic Register            */
#define MII_AM79C874_PMLR      19  /* Power and Loopback Register    */
#define MII_AM79C874_MCR       21  /* ModeControl Register           */
#define MII_AM79C874_DC        23  /* Disconnect Counter             */
#define MII_AM79C874_REC       24  /* Recieve Error Counter          */


// SMCS LAN87xx phy

// Specific register definitions for the SMCS LAN87xx
#define MII_LAN87xx_SRR        16  /* Silicon Revision Register            */
#define MII_LAN87xx_MCSR       17  /* Mode Control/Status Register         */
#define MII_LAN87xx_SMR        18  /* Special Modes  Register              */
#define MII_LAN87xx_SECR       26  /* Symbol Error Counter Register        */
#define MII_LAN87xx_CSIR       27  /* Control/Status Indication Register   */
#define MII_LAN87xx_SITC       28  /* Special Internal Testbility  Register*/
#define MII_LAN87xx_ICR        29  /* Interrupt Sources Control  Register  */
#define MII_LAN87xx_IMR        30  /* Interrupt Mark Register              */
#define MII_LAN87xx_SCSR       31  /* PHY Special Control/Status Register  */


// DP83640 phy
// DP83640 phy
// Specific register definitions for the DP83640
#define MII_DP83640_PHYSTS          16  /* PHY Status Register                                      */
#define MII_DP83640_MICR            17  /* MII Interrupt Control Register                           */
#define MII_DP83640_MISR            18  /* MII Interrupt Status and Event Control Register          */
#define MII_DP83640_PAGESEL         19  /* Page Select Register                                     */
#define MII_DP83640_FCSR            20  /* False Carrier Sense Counter Register                     */
#define MII_DP83640_RECR            21  /* Receive Error Counter Register                           */
#define MII_DP83640_PCSR            22  /* PCS Sub-Layer Configuration and Status Register          */
#define MII_DP83640_RBR             23  /* RMII and Bypass Register                                 */
#define MII_DP83640_LEDCR           24  /* LED Direct Control Register                              */
#define MII_DP83640_PHYCR           25  /* PHY Control Register                                     */
#define MII_DP83640_10BTSCR         26  /* 10Base-T Status/Control Register                         */
#define MII_DP83640_CDCTRL1         27  /* CD Test Control Register and BIST Extensions Register    */
#define MII_DP83640_PHYCR2          28  /* PHY Control Register 2                                   */
#define MII_DP83640_EDCR            29  /* Energy Detect Control Register                           */
#define MII_DP83640_PCFCR           31  /* PHY Control Frames Configuration Register                */


ENET_PHY_INFO Am79c874Info = {
    0x0022561b,
    TEXT("AM79C874"),

};

ENET_PHY_INFO LAN87xxInfo = {
    0x7c0f1,
    TEXT("LAN87xx"),

};
ENET_PHY_INFO Dp83640Info = {
    0x20005ce1, 
    TEXT("DP83640"),

};

// Now we  support Am79c874 and SMCS LAN87xx and DP83640

ENET_PHY_INFO *PhyInfo[] = {
    &Am79c874Info,
    &LAN87xxInfo,
    &Dp83640Info,
    NULL
};


typedef enum _ENET_PHY_ID {
    Am79c874           = 0,
    LAN87xx            = 1,
    Dp83640            = 2,
    ENET_PHY_MAX_ID
} ENET_PHY_ID;

#ifdef __cplusplus
    }
#endif

#endif // __ENET_H__
