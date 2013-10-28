//========================================================================
//   Copyright 2006 Mistral Software Pvt Ltd.
//
//   Use of this software is controlled by the terms and conditions found
//   in the license agreement under which this software has been supplied.
//========================================================================

//! \file Am389xEmacRegs.h
//! \brief AM389X EMAC Controller Register Defines Header File
//! 
//! This header File contains the #defines and typedefs
//! related to the EMAC Controller of the DM644x Davinci Chipset.
//! 
//! \version  1.00 May 22nd 2006 File Created 

#ifndef __AM389X_EMACREGS_H_INCLUDED__
#define __AM389X_EMACREGS_H_INCLUDED__

#include <windows.h>
#include "Constants.h"

#define EMAC_CTRL_OFFSET	0x0900
#define EMAC_MDIO_OFFSET	0x0800
#define EMAC_RAM_OFFSET		0x2000

//! \def EMAC_TOTAL_MEMORY
//! \brief Total EMAC related memory to be mapped
#define EMAC_TOTAL_MEMORY   (N16KB + N2KB)

//! \def EMAC_MAX_CHAN
//! \brief Maximum number of channels supported in EMAC
#define EMAC_MAX_CHAN               8

//! \def EMAC_STATS_REGS
//! \brief Maximum number of Statics registers in EMAC
#define EMAC_STATS_REGS             36

//! \def EMAC_MULTICAST_REGS
//! \brief Maximum number of Multicast entries in EMAC                         
#define EMAC_MAX_MCAST_ENTRIES      64

//! \def EMAC_RXMBPENABL 
//! \brief Bit defines for RXMBPENABL register
#define EMAC_RXMBPENABLE_RXCAFEN_ENABLE         BIT21
#define EMAC_RXMBPENABLE_RXBROADEN              BIT13
#define EMAC_RXMBPENABLE_RXMULTIEN              BIT5

//! \def EMAC_MACCONTROL 
//! \brief Bit defines for MACCONTROL register
#define EMAC_MACCONTROL_MIIEN_ENABLE            BIT5
#define EMAC_MACCONTROL_FULLDUPLEX_ENABLE       BIT0
#define EMAC_MACCONTROL_LOOPBACK_ENABLE         BIT1
#define EMAC_MACCONTROL_RXBUFFERFLOW_ENABLE     BIT3
#define EMAC_MACCONTROL_TXFLOW_EMABLE           BIT4
#define EMAC_MACCONTROL_GMII_ENABLE             BIT5
#define EMAC_MACCONTROL_TXPACING_ENABLE         BIT6
#define EMAC_MACCONTROL_TX_PRIO_ROUND_ROBIN     0x0
#define EMAC_MACCONTROL_TX_PRIO_FIXED           BIT9
#define EMAC_MACCONTROL_RXOFFLENBLOCK_ENABLE    BIT14

#define EMAC_MACCONTROL_GIG_ENABLE         		BIT7
#define EMAC_MACCONTROL_GIGFORCE         		BIT17


//! \def EMAC_MDIO
//! \brief Bit defines for EMAC_MDIO register
#define MDIO_USERACCESS0_GO                     BIT31
#define MDIO_USERACCESS0_WRITE_READ             0x0
#define MDIO_USERACCESS0_WRITE_WRITE            BIT30

//! \def MDIO_CONTROL
//! \brief Bit defines for MDIO_CONTROL register
#define MDIO_CONTROL_ENABLE                     BIT30
#define MDIO_CONTROL_FAULT                      BIT19


//! \def EMAC_DSC
//! \brief Various Emac descriptor flags 
#define EMAC_DSC_FLAG_SOP                   BIT31
#define EMAC_DSC_FLAG_EOP                   BIT30
#define EMAC_DSC_FLAG_OWNER                 BIT29
#define EMAC_DSC_FLAG_EOQ                   BIT28
#define EMAC_DSC_FLAG_TDOWNCMPLT            BIT27
#define EMAC_DSC_FLAG_PASSCRC               BIT26
#define EMAC_DSC_FLAG_JABBER                BIT25
#define EMAC_DSC_FLAG_OVERSIZE              BIT24
#define EMAC_DSC_FLAG_FRAGMENT              BIT23
#define EMAC_DSC_FLAG_UNDERSIZED            BIT22
#define EMAC_DSC_FLAG_CONTROL               BIT21
#define EMAC_DSC_FLAG_OVERRUN               BIT20
#define EMAC_DSC_FLAG_CODEERROR             BIT19
#define EMAC_DSC_FLAG_ALIGNERROR            BIT18
#define EMAC_DSC_FLAG_CRCERROR              BIT17
#define EMAC_DSC_FLAG_NOMATCH               BIT16
#define EMAC_DSC_RX_ERROR_FRAME             0x03FC0000

//! \def EMAC_MDIO_CLOCK_FREQ clock
//! \brief Emac MDIO clock defines
#define EMAC_MDIO_CLOCK_FREQ                MHZ(1)     /// 1 MHz

//! \def EMAC_PACKET
//! \brief Emac Ethernet packet defines
#define EMAC_MAX_DATA_SIZE               1500
#define EMAC_MIN_DATA_SIZE               46
#define EMAC_HEADER_SIZE                 14
#define EMAC_RX_MAX_LEN                  1520   
#define EMAC_MIN_ETHERNET_PKT_SIZE       (EMAC_MIN_DATA_SIZE + EMAC_HEADER_SIZE)
#define EMAC_MAX_ETHERNET_PKT_SIZE       (EMAC_MAX_DATA_SIZE + EMAC_HEADER_SIZE)
#define EMAC_PKT_ALIGN                   22      /// (1514 + 22 = 1536)packet aligned on 32 byte boundry
#define EMAC_MAX_PKT_BUFFER_SIZE         (EMAC_MAX_ETHERNET_PKT_SIZE + EMAC_PKT_ALIGN)

//! \def EMAC_RX/TX
//! \brief RX/TX descriptor related defines 
#define EMAC_RX_DESC_BASE           0x0
#define EMAC_TX_DESC_BASE           (EMAC_RX_DESC_BASE + N4KB)
#define EMAC_MAX_RX_BUFFERS         8

//! \def EMAC_MII
//! \brief MII Register numbers
#define MII_CONTROL_REG             0
#define MII_STATUS_REG              1
#define MII_PHY_CONFIG_REG          22
#define MII_PHY_STATUS_REG          26
#define PHY_TX_CLK_EN		     	BIT5

#define PHY_SPEED_1000BASET			0x0200
#define PHY_SPEED_MASK				0x0300
#define PHY_DUPLEX_MASK				0x0080
#define PHY_FULL_DUPLEX				0x0080


//! \def EMAC_PHY
//! \brief Bit defines for Intel PHY Registers
#define PHY_LINK_STATUS             BIT2
#define PHY_INC_DRIVE_STRENGTH      BIT11

//! \def EMAC_BUF
//! \brief EMAC buffer regions allocated by OEM
#ifdef EMAC_BUF_BASE
#undef EMAC_BUF_BASE
#endif
#define EMAC_BUF_BASE       (0x8F000000)
#define EMAC_RX_BUFS_SIZE   (N4KB + N8KB)
#define EMAC_TX_BUFS_BASE   (EMAC_BUF_BASE + EMAC_RX_BUFS_SIZE)

//***********************************
//
//! \typedef EMAC_DESC
//! \brief EMAC descriptor overlay structure
//
//************************************
typedef struct __EMAC_DESC__ 
{
    struct  __EMAC_DESC__*  m_pNext;        /// Pointer to next descriptor in chain 
    UINT8*                  m_pBuffer;      /// Pointer to data buffer 
    UINT32                  m_BufOffLen;    /// Buffer Offset(MSW) and Length(LSW) 
    UINT32                  m_PktFlgLen;    /// Packet Flags(MSW) and Length(LSW) 
    
} EMAC_DESC,*PEMAC_DESC;


//*************************************************************************
//                 EMAC Register Declarations
//*************************************************************************

//***********************************
//
//! \typedef EMAC_REGS
//! \brief EMAC_REGS Register Overlay Structure
//
//************************************

typedef struct __EMAC_REGS__
{
    volatile UINT32    m_Txidver    ;           /// Transmit Identification and Version Register
    volatile UINT32    m_Txcontrol  ;           /// Transmit Control Register
    volatile UINT32    m_Txteardown ;           /// Transmit Teardown Register
    volatile UINT8     RSVD0[4]     ;
    volatile UINT32    m_Rxidver    ;
    volatile UINT32    m_Rxcontrol  ;           /// Receive Control Register
    volatile UINT32    m_Rxteardown ;           /// Receive Teardown Register
    volatile UINT8     RSVD1[100]   ;           /// 001C -  007C – Reserved
    volatile UINT32    m_Txintstatraw;          /// Transmit Interrupt Status (Unmasked) Register
    volatile UINT32    m_Txintstatmasked;       /// Transmit Interrupt Status (Masked) Register
    volatile UINT32    m_Txintmaskset;          /// Transmit Interrupt Mask Set Register
    volatile UINT32    m_Txintmaskclear;        /// Transmit Interrupt Mask Clear Register
    volatile UINT32    m_Macinvector;           /// MAC Input Vector Register
    volatile UINT32    m_Maceoiector;           /// MAC EOI Vector Register
    volatile UINT8     RSVD2[8]    ;           /// 0098 009C – Reserved
    volatile UINT32    m_Rxintstatraw;          /// Receive Interrupt Status (Unmasked) Register
    volatile UINT32    m_Rxintstatmasked;       /// Receive Interrupt Status (Masked) Register
    volatile UINT32    m_Rxintmaskset;          /// Receive Interrupt Mask Set Register
    volatile UINT32    m_Rxintmaskclear;        /// Receive Interrupt Mask Clear Register
    volatile UINT32    m_Macintstatraw;         /// MAC Interrupt Status (Unmasked) Register
    volatile UINT32    m_Macintstatmasked;      /// MAC Interrupt Status (Masked) Register
    volatile UINT32    m_Macintmaskset;         /// MAC Interrupt Mask Set Register
    volatile UINT32    m_Macintmaskclear;       /// MAC Interrupt Mask Clear Register
    volatile UINT8     RSVD3[64]        ;       /// 00C0 00fc – Reserved
    volatile UINT32    m_Rxmbpenable;           /// Receive Multicast/Broadcast/Promiscuous Channel Enable Register
    volatile UINT32    m_Rxunicastset;          /// Receive Unicast Enable Set Register
    volatile UINT32    m_Rxunicastclear;        /// Receive Unicast Clear Register
    volatile UINT32    m_Rxmaxlen   ;           /// Receive Maximum Length Register
    volatile UINT32    m_Rxbufferoffset;        /// Receive Buffer Offset Register
    volatile UINT32    m_Rxfilterlowthresh;     /// Receive Filter Low Priority Frame Threshold Register
    volatile UINT8     RSVD4[8]          ;      /// 0118 011C – Reserved
    volatile UINT32    m_Rx0flowthresh;         /// Receive Channel 0 Flow Control Threshold Register
    volatile UINT32    m_Rx1flowthresh;         /// Receive Channel 1 Flow Control Threshold Register
    volatile UINT32    m_Rx2flowthresh;         /// Receive Channel 2 Flow Control Threshold Register
    volatile UINT32    m_Rx3flowthresh;         /// Receive Channel 3 Flow Control Threshold Register
    volatile UINT32    m_Rx4flowthresh;         /// Receive Channel 4 Flow Control Threshold Register
    volatile UINT32    m_Rx5flowthresh;         /// Receive Channel 5 Flow Control Threshold Register
    volatile UINT32    m_Rx6flowthresh;         /// Receive Channel 6 Flow Control Threshold Register
    volatile UINT32    m_Rx7flowthresh;         /// Receive Channel 7 Flow Control Threshold Register
    volatile UINT32    m_Rx0freebuffer;         /// Receive Channel 0 Free Buffer Count Register
    volatile UINT32    m_Rx1freebuffer;         /// Receive Channel 1 Free Buffer Count Register
    volatile UINT32    m_Rx2freebuffer;         /// Receive Channel 2 Free Buffer Count Register
    volatile UINT32    m_Rx3freebuffer;         /// Receive Channel 3 Free Buffer Count Register
    volatile UINT32    m_Rx4freebuffer;         /// Receive Channel 4 Free Buffer Count Register
    volatile UINT32    m_Rx5freebuffer;         /// Receive Channel 5 Free Buffer Count Register
    volatile UINT32    m_Rx6freebuffer;         /// Receive Channel 6 Free Buffer Count Register
    volatile UINT32    m_Rx7freebuffer;         /// Receive Channel 7 Free Buffer Count Register
    volatile UINT32    m_Maccontrol ;           /// MAC Control Register   0x0160
    volatile UINT32    m_Macstatus  ;           /// MAC Status Register
    volatile UINT32    m_Emcontrol  ;           /// Emulation Control Register
    volatile UINT32    m_Fifocontrol;           /// FIFO Control Register (Transmit and Receive)
    volatile UINT32    m_Macconfig  ;           /// MAC Configuration Register
    volatile UINT32    m_Softreset  ;           /// Soft Reset Register
    volatile UINT8     RSVD5[88]    ;           /// 0178 01CC – Reserved
    volatile UINT32    m_Macsrcaddrlo;          /// MAC Source Address Low Bytes Register (Lower 32-bits)
    volatile UINT32    m_Macsrcaddrhi;          /// MAC Source Address High Bytes Register (Upper 16-bits)
    volatile UINT32    m_Machash1   ;           /// MAC Hash Address Register 1
    volatile UINT32    m_Machash2   ;           /// MAC Hash Address Register 2
    volatile UINT32    m_Bofftest   ;           /// Back Off Test Register
    volatile UINT32    m_Tpacetest  ;           /// Transmit Pacing Algorithm Test Register
    volatile UINT32    m_Rxpause    ;           /// Receive Pause Timer Register
    volatile UINT32    m_Txpause    ;           /// Transmit Pause Timer Register
    volatile UINT8     RSVD6[16]    ;           /// 01F0 01FC – Reserved
    volatile UINT32    m_Rxgoodframes;          /// Good Receive Frames Register
    volatile UINT32    m_Rxbcastframes;         /// Broadcast Receive Frames Register (Total number of good broadcast frames received)
    volatile UINT32    m_Rxmcastframes;         /// Multicast Receive Frames Register (Total number of good multicast frames received)
    volatile UINT32    m_Rxpauseframes;         /// Pause Receive Frames Register
    volatile UINT32    m_Rxcrcerrors;           /// Receive CRC Errors Register (Total number of frames received with CRC errors)
    volatile UINT32    m_Rxaligncodeerrors;     /// Receive Alignment/Code Errors Register (Total number of frames received with alignment/code   
    volatile UINT32    m_Rxoversized      ;     /// Receive Alignment/Code Errors Register (Total number of frames received with alignment/code    volatile UINT32    m_Rxoversized;    /// Receive Oversized Frames Register (Total number of oversized frames received)
    volatile UINT32    m_Rxjabber   ;           /// Receive Jabber Frames Register (Total number of jabber frames received)
    volatile UINT32    m_Rxundersized;          /// Receive Undersized Frames Register (Total number of undersized frames received)
    volatile UINT32    m_Rxfragments;           /// Receive Frame Fragments Register
    volatile UINT32    m_Rxfiltered ;           /// Filtered Receive Frames Register
    volatile UINT32    m_Rxqosfiltered;         /// Received QOS Filtered Frames Register
    volatile UINT32    m_Rxoctets   ;           /// Receive Octet Frames Register (Total number of received bytes in good frames)
    volatile UINT32    m_Txgoodframes;          /// Good Transmit Frames Register (Total number of good frames transmitted)
    volatile UINT32    m_Txbcastframes;         /// Broadcast Transmit Frames Register
    volatile UINT32    m_Txmcastframes;         /// Multicast Transmit Frames Register
    volatile UINT32    m_Txpauseframes;         /// Pause Transmit Frames Register
    volatile UINT32    m_Txdeferred ;           /// Deferred Transmit Frames Register
    volatile UINT32    m_Txcollision;           /// Transmit Collision Frames Register
    volatile UINT32    m_Txsinglecoll;          /// Transmit Single Collision Frames Register
    volatile UINT32    m_Txmulticoll;           /// Transmit Multiple Collision Frames Register
    volatile UINT32    m_Txexcessivecoll;       /// Transmit Excessive Collision Frames Register
    volatile UINT32    m_Txlatecoll ;           /// Transmit Late Collision Frames Register
    volatile UINT32    m_Txunderrun ;           /// Transmit Underrun Error Register
    volatile UINT32    m_Txcarriersense;        /// Transmit Carrier Sense Errors Register
    volatile UINT32    m_Txoctets   ;           /// Transmit Octet Frames Register
    volatile UINT32    m_Frame64    ;           /// Transmit and Receive 64 Octet Frames Register
    volatile UINT32    m_Frame65t127;           /// Transmit and Receive 65 to 127 Octet Frames Register
    volatile UINT32    m_Frame128t255;          /// Transmit and Receive 128 to 255 Octet Frames Register
    volatile UINT32    m_Frame256t511;          /// Transmit and Receive 256 to 511 Octet Frames Register
    volatile UINT32    m_Frame512t1023;         /// Transmit and Receive 512 to 1023 Octet Frames Register
    volatile UINT32    m_Frame1024tup;          /// Transmit and Receive 1024 to 1518 Octet Frames Register
    volatile UINT32    m_Netoctets  ;           /// Network Octet Frames Register
    volatile UINT32    m_Rxsofoverruns;         /// Receive FIFO or DMA Start of Frame Overruns Register
    volatile UINT32    m_Rxmofoverruns;         /// Receive FIFO or DMA Middle of Frame Overruns Register
    volatile UINT32    m_Rxdmaoverruns;         /// Receive DMA Start of Frame and Middle of Frame Overruns Register
    volatile UINT8     RSVD7[624]     ;         /// 01C8 04FC – Reserved
    volatile UINT32    m_Macaddrlo  ;           /// MAC Address Low Bytes Register
    volatile UINT32    m_Macaddrhi  ;           /// MAC Address High Bytes Register
    volatile UINT32    m_Macindex   ;           /// MAC Index Register
    volatile UINT8     RSVD8[244]   ;           /// 051C 05FC – Reserved
    volatile UINT32    m_Tx0hdp     ;           /// Transmit Channel 0 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Tx1hdp     ;           /// Transmit Channel 1 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Tx2hdp     ;           /// Transmit Channel 2 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Tx3hdp     ;           /// Transmit Channel 3 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Tx4hdp     ;           /// Transmit Channel 4 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Tx5hdp     ;           /// Transmit Channel 5 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Tx6hdp     ;           /// Transmit Channel 6 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Tx7hdp     ;           /// Transmit Channel 7 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Rx0hdp     ;           /// Receive Channel 0 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Rx1hdp     ;           /// Receive Channel 1 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Rx2hdp     ;           /// Receive Channel 2 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Rx3hdp     ;           /// Receive Channel 3 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Rx4hdp     ;           /// Receive Channel 4 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Rx5hdp     ;           /// Receive Channel 5 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Rx6hdp     ;           /// Receive Channel 6 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Rx7hdp     ;           /// Receive Channel 7 DMA Head Descriptor Pointer Register
    volatile UINT32    m_Tx0cp      ;           /// Transmit Channel 0 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Tx1cp      ;           /// Transmit Channel 1 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Tx2cp      ;           /// Transmit Channel 2 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Tx3cp      ;           /// Transmit Channel 3 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Tx4cp      ;           /// Transmit Channel 4 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Tx5cp      ;           /// Transmit Channel 5 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Tx6cp      ;           /// Transmit Channel 6 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Tx7cp      ;           /// Transmit Channel 7 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Rx0cp      ;           /// Receive Channel 0 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Rx1cp      ;           /// Receive Channel 1 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Rx2cp      ;           /// Receive Channel 2 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Rx3cp      ;           /// Receive Channel 3 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Rx4cp      ;           /// Receive Channel 4 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Rx5cp      ;           /// Receive Channel 5 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Rx6cp      ;           /// Receive Channel 6 Completion Pointer (Interrupt Acknowledge) Register
    volatile UINT32    m_Rx7cp      ;           /// Receive Channel 7 Completion Pointer (Interrupt Acknowledge) Register

}EMAC_REGS, *PEMAC_REGS;


//*************************************************************************
//            EMAC_CTRL Register Declarations
//*************************************************************************

//***********************************
//
//! \typedef EMAC_CTRL_REGS
//! \brief EMAC_CTRL_REGS Register Overlay Structure 
//
//************************************

typedef struct __EMAC_CTRL_REGS__
{
	volatile UINT32		id_ver;
	volatile UINT32		soft_reset;
	volatile UINT32		control;
	volatile UINT32		int_control;
	volatile UINT32		c0_rx_thresh_en;
	volatile UINT32		c0_rx_en;
	volatile UINT32		c0_tx_en;
	volatile UINT32		c0_misc_en;
	volatile UINT32		c1_rx_thresh_en;
	volatile UINT32		c1_rx_en;
	volatile UINT32		c1_tx_en;
	volatile UINT32		c1_misc_en;
	volatile UINT32		c2_rx_thresh_en;
	volatile UINT32		c2_rx_en;
	volatile UINT32		c2_tx_en;
	volatile UINT32		c2_misc_en;
	volatile UINT32		c0_rx_thresh_stat;
	volatile UINT32		c0_rx_stat;
	volatile UINT32		c0_tx_stat;
	volatile UINT32		c0_misc_stat;
	volatile UINT32		c1_rx_thresh_stat;
	volatile UINT32		c1_rx_stat;
	volatile UINT32		c1_tx_stat;
	volatile UINT32		c1_misc_stat;
	volatile UINT32		c2_rx_thresh_stat;
	volatile UINT32		c2_rx_stat;
	volatile UINT32		c2_tx_stat;
	volatile UINT32		c2_misc_stat;
	volatile UINT32		c0_rx_imax;
	volatile UINT32		c0_tx_imax;
	volatile UINT32		c1_rx_imax;
	volatile UINT32		c1_tx_imax;
	volatile UINT32		c2_rx_imax;
	volatile UINT32		c2_tx_imax;
	UINT32				rsv_088_0fc[31];
}EMAC_CTRL_REGS, *PEMAC_CTRL_REGS;

//*************************************************************************
//            EMAC MDIO Register Declarations
//*************************************************************************

//***********************************
//
//! \typedef EMAC_MDIO_REGS
//! \brief EMAC_MDIO_REGS Register Overlay Structure 
//
//************************************

typedef struct __EMAC_MDIO_REGS__
{
    volatile UINT32    m_Version    ;       /// MDIO Version Register
    volatile UINT32    m_Control    ;       /// MDIO Control Register
    volatile UINT32    m_Alive      ;       /// MDIO PHY Alive Status Register
    volatile UINT32    m_Link       ;       /// MDIO PHY Link Status Register
    volatile UINT32    m_Linkintraw ;       /// MDIO Link Status Change Interrupt (Unmasked) Register
    volatile UINT32    m_Linkintmasked;     /// MDIO Link Status Change Interrupt (Masked) Register
    volatile UINT8     RSVD1[8]     ;       /// Reserved
    volatile UINT32    m_Userintraw ;       /// MDIO User Command Complete Interrupt (Unmasked) Register
    volatile UINT32    m_Userintmasked;     /// MDIO User Command Complete Interrupt (Masked) Register
    volatile UINT32    m_Userintmaskset;    /// MDIO User Command Complete Interrupt Mask Set Register
    volatile UINT32    m_Userintmaskclear;  /// MDIO User Command Complete Interrupt Mask Clear Register
    volatile UINT8     RSVD2[80]    ;       /// Reserved
    volatile UINT32    m_Useraccess0;       /// MDIO User Access Register 0
    volatile UINT32    m_Userphysel0;       /// MDIO User PHY Select Register 0
    volatile UINT32    m_Useraccess1;       /// MDIO User Access Register 1
    volatile UINT32    m_Userphysel1;       /// MDIO User PHY Select Register 1

}EMAC_MDIO_REGS, *PEMAC_MDIO_REGS;

#endif /* #ifndef __AM389X_EMACREGS_H_INCLUDED__ */
