//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//  Header:  Enc28j60.h
//  
// Provides Definitions for ENC28J60 Ethernet Controller.
//------------------------------------------------------------------------------

#ifndef _ENC28J60_H
#define _ENC28J60_H

#ifdef __cplusplus
extern "C" {
#endif

#define OP_RCR  0
#define OP_RBM  1
#define OP_WCR  2
#define OP_WBM  3
#define OP_BFS  4
#define OP_BFC  5
#define OP_SC   7
#define OP_MAX   OP_SC

#define BANK0       0x0
#define BANK1       0x1
#define BANK2       0x2
#define BANK3       0x3

//
// Use bits 12-15 to identify what registers
// belong to under Bank3.  Bank 3 contains
// MAC, MII and ETH register.  Bank0 and Bank1 
// contains all ETH. Bank2 contains MAC and MII
// registers only.
//
#define BANK3_MAC    (0xA)  //has to be nibble in size   
#define BANK3_MII    (0xB)  //has to be nibble in size   
#define BANK3_ETH    (0xC)  //has to be nibble in size   

#define BANKSEL_MASK  0x0000FF00
//
// Bank0 
//
// The first 8 bits are the register address.  The next
// 8 bits identifies which bank this register belongs to
//
#define ERDPTL      (0x00 | BANK0<<8)
#define ERDPTH      (0x01 | BANK0<<8)
#define EWRPTL      (0x02 | BANK0<<8)
#define EWRPTH      (0x03 | BANK0<<8)
#define ETXSTL      (0x04 | BANK0<<8)
#define ETXSTH      (0x05 | BANK0<<8)
#define ETXNDL      (0x06 | BANK0<<8)
#define ETXNDH      (0x07 | BANK0<<8)
#define ERXSTL      (0x08 | BANK0<<8)
#define ERXSTH      (0x09 | BANK0<<8)
#define ERXNDL      (0x0A | BANK0<<8)
#define ERXNDH      (0x0B | BANK0<<8)
#define ERXRDPTL    (0x0C | BANK0<<8)
#define ERXRDPTH    (0x0D | BANK0<<8)
#define ERXWRPTL    (0x0E | BANK0<<8)
#define ERXWRPTH    (0x0F | BANK0<<8)
#define EDMASTL     (0x10 | BANK0<<8)
#define EDMASTH     (0x11 | BANK0<<8)
#define EDMANDL     (0x12 | BANK0<<8)
#define EDMANDH     (0x13 | BANK0<<8)
#define EDMADSTL    (0x14 | BANK0<<8)
#define EDMADSTH    (0x15 | BANK0<<8)
#define EDMACSL     (0x16 | BANK0<<8)
#define EDMACSH     (0x17 | BANK0<<8)
//#define ?        (0x18 | BANK0<<8)
//#define ?        (0x19 | BANK0<<8)
//#define           (0x1A | BANK0<<8)


#define EIE         0x1B 
#define EIR         0x1C 
#define ESTAT       0x1D 
#define ECON2       0x1E
#define ECON1       0x1F


//
// Bank1
//
#define EHT0        (0x00 | BANK1<<8)
#define EHT1        (0x01 | BANK1<<8)
#define EHT2        (0x02 | BANK1<<8)
#define EHT3        (0x03 | BANK1<<8)
#define EHT4        (0x04 | BANK1<<8)
#define EHT5        (0x05 | BANK1<<8)
#define EHT6        (0x06 | BANK1<<8)
#define EHT7        (0x07 | BANK1<<8)
#define EPMM0       (0x08 | BANK1<<8)
#define EPMM1       (0x09 | BANK1<<8)
#define EPMM2       (0x0A | BANK1<<8)
#define EPMM3       (0x0B | BANK1<<8)
#define EPMM4       (0x0C | BANK1<<8)
#define EPMM5       (0x0D | BANK1<<8)
#define EPMM6       (0x0E | BANK1<<8)
#define EPMM7       (0x0F | BANK1<<8)
#define EPMCSL      (0x10 | BANK1<<8)
#define EPMCSH      (0x11 | BANK1<<8)
//#define ?        (0x12 | BANK1<<8)
//#define ?        (0x13 | BANK1<<8)
#define EPMOL       (0x14 | BANK1<<8)
#define EPMOH       (0x15 | BANK1<<8)
#define EWOLIE      (0x16 | BANK1<<8)
#define EWOLIR      (0x17 | BANK1<<8)
#define ERXFCON     (0x18 | BANK1<<8)
#define EPKTCNT     (0x19 | BANK1<<8)
//#define Reserved  (0x1A | BANK1<<8)


//
// Bank2
//
#define MACON1          (0x00 | BANK2<<8)
#define MACON2          (0x01 | BANK2<<8)
#define MACON3          (0x02 | BANK2<<8)
#define MACON4          (0x03 | BANK2<<8)
#define MABBIPG         (0x04 | BANK2<<8)
//#define ?            (0x05 | BANK2<<8)
#define MAIPGL          (0x06 | BANK2<<8)
#define MAIPGH          (0x07 | BANK2<<8)
#define MACLCON1        (0x08 | BANK2<<8)
#define MACLCON2        (0x09 | BANK2<<8)
#define MAMXFLL         (0x0A | BANK2<<8)
#define MAMXFLH         (0x0B | BANK2<<8)
//#define Reserved      (0x0C | BANK2<<8)
#define MAPHSUP         (0x0D | BANK2<<8)
//#define Reserved      (0x0E | BANK2<<8)
//#define ?            (0x0F | BANK2<<8)
//#define Reserved      (0x10 | BANK2<<8)
#define MICON           (0x11 | BANK2<<8)
#define MICMD           (0x12 | BANK2<<8)
//#define ?            (0x13 | BANK2<<8)
#define MIREGADR        (0x14 | BANK2<<8)
//#define Reserved      (0x15 | BANK2<<8)
#define MIWRL           (0x16 | BANK2<<8)
#define MIWRH           (0x17 | BANK2<<8)
#define MIRDL           (0x18 | BANK2<<8)
#define MIRDH           (0x19 | BANK2<<8)
//#define Reserved      (0x1A | BANK2<<8)


//
// Bank 3
//
#define MAADR1      (0x00 | BANK3<<8 | (BANK3_MAC << 12))
#define MAADR0      (0x01 | BANK3<<8 | (BANK3_MAC << 12))
#define MAADR3      (0x02 | BANK3<<8 | (BANK3_MAC << 12))
#define MAADR2      (0x03 | BANK3<<8 | (BANK3_MAC << 12))
#define MAADR5      (0x04 | BANK3<<8 | (BANK3_MAC << 12))
#define MAADR4      (0x05 | BANK3<<8 | (BANK3_MAC << 12))
#define EBSTSD      (0x06 | BANK3<<8 | (BANK3_ETH << 12))
#define EBSTCON     (0x07 | BANK3<<8 | (BANK3_ETH << 12))
#define EBSTCSL     (0x08 | BANK3<<8 | (BANK3_ETH << 12))
#define EBSTCSH     (0x09 | BANK3<<8 | (BANK3_ETH << 12))
#define MISTAT      (0x0A | BANK3<<8 | (BANK3_MII << 12))
//#define ?        (0x0B | BANK3<<8)
//#define ?        (0x0C | BANK3<<8)
//#define ?        (0x0D | BANK3<<8)
//#define ?        (0x0E | BANK3<<8)
//#define ?        (0x0F | BANK3<<8)
//#define ?        (0x10 | BANK3<<8)
//#define ?        (0x11 | BANK3<<8)
#define EREVID      (0x12 | BANK3<<8 | (BANK3_ETH << 12))
//#define ?        (0x13 | BANK3<<8)
//#define ?        (0x14 | BANK3<<8)
#define ECOCON      (0x15 | BANK3<<8 | (BANK3_ETH << 12))
//#define Reserved  (0x16 | BANK3<<8)
#define EFLOCON     (0x17 | BANK3<<8 | (BANK3_ETH << 12))
#define EPAUSL      (0x18 | BANK3<<8 | (BANK3_ETH << 12))
#define EPAUSH      (0x19 | BANK3<<8 | (BANK3_ETH << 12))
//#define Reserved  (0x1A | BANK3<<8)


// Recieve Status Vection - LowByte

#define RCV_LONG_EVENT_DROP_EVENT   (1<<0)
#define RCV_CARRIER_EVENT_PREV_SEEN (1<<2)
#define RCV_CRC_ERROR               (1<<4)
#define RCV_LENGTH_CHECK_ERROR      (1<<5)
#define RCV_LENGTH_OUT_OF_RANGE     (1<<6)
#define RCV_OK                      (1<<7)

// Recieve Status Vection - HiByte
#define RCV_MULTICAST_PACKET   (1<<0)
#define RCV_BROADCAST_PACKET   (1<<1)
#define RCV_DRIBBLE_NIBBLE      (1<<2)
#define RCV_CONTROL_FRAME       (1<<3)
#define RCV_PAUSE_CONTROL_FRAME (1<<4)
#define RCV_UNKNOWN_OPCODE      (1<<5)
#define RCV_VLAN_TYPE_DETECTED  (1<<6)


//
// Packet Transmit control byte
#define TX_POVERRIDE        (1<<0)
#define TX_PCRCEN           (1<<1)
#define TX_PPADEN           (1<<2)
#define TX_PHUGEEN          (1<<3)

// Transmit Status Vectors
#define TXMIT_CRC_ERROR     (1<<20)
#define TXMIT_LENGTH_CHECK_ERROR (1<<21)
#define TXMIT_LENGTH_OUT_OF_RANGE (1<<22)
#define TXMIT_DONE              (1<<23)
#define TXMIT_MULTICAST         (1<<24)
#define TXMIT_BROADCAST         (1<<25)
#define TXMIT_PACKET_DEFER          (1<<26)
#define TXMIT_EXCESSIVE_DEFER       (1<<27)
#define TXMIT_EXCESSIVE_COLL        (1<<28)
#define TXMIT_LATE_COLL             (1<<29)
#define TXMIT_GIANT                 (1<<30)
#define TXMIT_UNDERRUN              (1<<31)

#define TXMIT_CONTROL_FRAME         (1<<16)
#define TXMIT_PAUSE_CONTROL_FRAME   (1<<17)
#define TXMIT_BACKPRESSURE_APPLIED  (1<<18)
#define TXMIT_TRANS_VLAN_TAGGED_FRAME (1<<19)


//
// PHY Registers
//
#define PHYS_REG_ID (0xBB)

#define PHCON1      (0x00 | (PHYS_REG_ID << 8))
#define PHSTAT1     (0x01 | (PHYS_REG_ID << 8))
#define PHID1       (0x02 | (PHYS_REG_ID << 8))
#define PHID2       (0x03 | (PHYS_REG_ID << 8))
#define PHCON2      (0x10 | (PHYS_REG_ID << 8))
#define PHSTAT2     (0x11 | (PHYS_REG_ID << 8))
#define PHIE        (0x12 | (PHYS_REG_ID << 8))
#define PHIR        (0x13 | (PHYS_REG_ID << 8))
#define PHLCON      (0x14 | (PHYS_REG_ID << 8))

// PHCON1
#define    BM_ENC28J60_PHCON1_PDPXMD       (1<<8)
#define    BM_ENC28J60_PHCON1_PPWRSV       (1<<11)
#define    BM_ENC28J60_PHCON1_PLOOPBK      (1<<14)
#define    BM_ENC28J60_PHCON1_PRST         (1<<15)

// PHSTAT1
#define    BM_ENC28J60_PHSTAT1_JBRSTAT     (1<<1)
#define    BM_ENC28J60_PHSTAT1_LLSTAT      (1<<2)
#define    BM_ENC28J60_PHSTAT1_PHDPX       (1<<11)
#define    BM_ENC28J60_PHSTAT1_PFDPX       (1<<12)

// PHCON2
#define    BM_ENC28J60_PHCON2_HDLDIS       (1<<8)
#define    BM_ENC28J60_PHCON2_JABBER       (1<<10)
#define    BM_ENC28J60_PHCON2_TXDIS        (1<<13)
#define    BM_ENC28J60_PHCON2_FRCLNK       (1<<14)

// PHLCON

#define    LEDx_DISP_TRAN_ACT               0x1
#define    LEDx_DISP_RCV_ACT                0x2
#define    LEDx_DISP_COLL_ACT               0x3
#define    LEDx_DISP_LINK_STAT              0x4
#define    LEDx_DISP_DUPLX_STAT             0x5
#define    LEDx_DISP_TX_RX_ACT              0x7
#define    LEDx_DISP_ON                     0x8
#define    LEDx_DISP_OFF                    0x9
#define    LEDx_BLNK_FAST                   0xA
#define    LEDx_BLNK_SLOW                   0xB
#define    LEDx_DISP_LINK_STAT_RX_ACT       0xC
#define    LEDx_DISP_LINK_STAT_TXRX_ACT     0xD
#define    LEDx_DISP_DUPLX_STAT_COLL_ACT    0xE

#define    BM_ENC28J60_PHLCON_STRCH           (1<<1)

//
//- ECON1 
//
#define         BM_ENC28J60_ECON1_BSEL0       (1<<0)
#define         BM_ENC28J60_ECON1_BSEL1       (1<<1)
#define         BM_ENC28J60_ECON1_RXEN        (1<<2)
#define         BM_ENC28J60_ECON1_TXRTS       (1<<3)
#define         BM_ENC28J60_ECON1_CSUMEN      (1<<4)
#define         BM_ENC28J60_ECON1_DMAST       (1<<5)
#define         BM_ENC28J60_ECON1_RXRST       (1<<6)
#define         BM_ENC28J60_ECON1_TXRST       (1<<7)

// MACON1 - MAC Control Register 1
#define         BM_ENC28J60_MACON1_MARXEN    (1<<0)
#define         BM_ENC28J60_MACON1_PASSALL   (1<<1)
#define         BM_ENC28J60_MACON1_RXPAUS    (1<<2)
#define         BM_ENC28J60_MACON1_TXPAUS    (1<<3)
#define         BM_ENC28J60_MACON1_LOOPBK    (1<<4)


//
// MACON2 - MAC Control Register 2
#define         BM_ENC28J60_MACON2_TFUNRST    (1<<0)
#define         BM_ENC28J60_MACON2_MATXRST    (1<<1)
#define         BM_ENC28J60_MACON2_RFUNRST    (1<<2)
#define         BM_ENC28J60_MACON2_MARXRST    (1<<3)
#define         BM_ENC28J60_MACON2_RNDRST     (1<<6)
#define         BM_ENC28J60_MACON2_MARST      (1<<7)

// MACON3
#define         BM_ENC28J60_MACON3_FULDPX     (1<<0)
#define         BM_ENC28J60_MACON3_FRMLNEN    (1<<1)
#define         BM_ENC28J60_MACON3_HFRMEN     (1<<2)
#define         BM_ENC28J60_MACON3_PHDRLEN    (1<<3)
#define         BM_ENC28J60_MACON3_TXCRCEN    (1<<4)

#define    MACON3_SHRT_FRM_0_PAD                0x7
#define    MACON3_NO_AUTO_PAD                   0x6
#define    MACON3_MAC_AUTODECT_VLAN             0x5
#define    MACON3_NO_AUTO_PADDING               0x4

#define    MACON3_SHRT_FRM_0_PADDED             0x3
#define    MACON3_NO_AUTO_PAD_SHRT_FRM          0x2
#define    MACON3_SHRT_FRM_0_PADDEDD            0x1
#define    MACON3_NO_AUTO_PAD_SHRT_FRAM         0x0

/*
111 = All short frames will be zero padded to 64 bytes and a valid CRC will then be appended
110 = No automatic padding of short frames
101 = MAC will automatically detect VLAN Protocol frames which have a 8100h type field and
automatically pad to 64 bytes. If the frame is not a VLAN frame, it will be padded to
60 bytes. After padding, a valid CRC will be appended.
100 = No automatic padding of short frames

011 = All short frames will be zero padded to 64 bytes and a valid CRC will then be appended
010 = No automatic padding of short frames
001 = All short frames will be zero padded to 60 bytes and a valid CRC will then be appended
000 = No automatic padding of short frames
*/

// MACON4
#define    BM_ENC28J60_MACON4_PUREPRE    (1<<0)
#define    BM_ENC28J60_MACON4_LONGPRE    (1<<1)
#define    BM_ENC28J60_MACON4_NOBKOFF    (1<<4)
#define    BM_ENC28J60_MACON4_BPEN       (1<<5)
#define    BM_ENC28J60_MACON4_DEFER      (1<<6)


// ERXFCON (Default on reset 0xa1)
#define    BM_ENC28J60_ERXFCON_BCEN      (1<<0)
#define    BM_ENC28J60_ERXFCON_MCEN      (1<<1)
#define    BM_ENC28J60_ERXFCON_HTEN      (1<<2)
#define    BM_ENC28J60_ERXFCON_MPEN      (1<<3)
#define    BM_ENC28J60_ERXFCON_PMEN      (1<<4)
#define    BM_ENC28J60_ERXFCON_CRCEN     (1<<5)
#define    BM_ENC28J60_ERXFCON_ANDOR     (1<<6)
#define    BM_ENC28J60_ERXFCON_UCEN      (1<<7)


// EFLOCON
#define    BM_ENC28J60_EFLOCON_FCEN0      (1<<0)
#define    BM_ENC28J60_EFLOCON_FCEN1      (1<<1)
#define    BM_ENC28J60_EFLOCON_FULDPXS    (1<<2)


// EIE - Ethernet interrupt enable register
#define    BM_ENC28J60_EIE_RXERIE      (1<<0)
#define    BM_ENC28J60_EIE_TXERIE      (1<<1)
#define    BM_ENC28J60_EIE_WOLIE       (1<<2)
#define    BM_ENC28J60_EIE_TXIE        (1<<3)
#define    BM_ENC28J60_EIE_LINKIE      (1<<4)
#define    BM_ENC28J60_EIE_DMAIE       (1<<5)
#define    BM_ENC28J60_EIE_PKTIE       (1<<6)
#define    BM_ENC28J60_EIE_INTIE       (1<<7)

// 
//- EIR
//
#define         BM_ENC28J60_EIR_RXERIF      (1<<0)
#define         BM_ENC28J60_EIR_TXERIF      (1<<1)
#define         BM_ENC28J60_EIR_WOLIF       (1<<2)
#define         BM_ENC28J60_EIR_TXIF        (1<<3)
#define         BM_ENC28J60_EIR_LINKIF      (1<<4)
#define         BM_ENC28J60_EIR_DMAIF       (1<<5)
#define         BM_ENC28J60_EIR_PKTIF       (1<<6)

typedef union
{
    UINT8  U;
    struct
    {
        unsigned TXRST  : 1;
        unsigned RXRST  : 1;
        unsigned DMAST  : 1;
        unsigned CSUMEN : 1;
        unsigned TXRTS  : 1;
        unsigned RXEN   : 1;
        unsigned BSEL1  : 1;
        unsigned BSEL0  : 1;
    } B;
} ETHER_ECON1;

//
// ECON2
//
#define         BM_ENC28J60_ECON2_VRPS       (1<<3)
#define         BM_ENC28J60_ECON2_PWRSV      (1<<5)
#define         BM_ENC28J60_ECON2_PKTDEC     (1<<6)
#define         BM_ENC28J60_ECON2_AUTOINC    (1<<7)

//
// MICON: MII CONTROL REGISTER
#define         BM_ENC28J60_MICON_RSTMII     (1<<0)

//
// MICMD: MII COMMAND REGISTER
#define         BM_ENC28J60_MICMD_MIIRD      (1<<0)
#define         BM_ENC28J60_MICMD_MIISCAN    (1<<1)

//
// MISTAT: MII STATUS REGISTER
#define         BM_ENC28J60_MISTAT_BUSY     (1<<0)
#define         BM_ENC28J60_MISTAT_SCAN     (1<<1)
#define         BM_ENC28J60_MISTAT_NVALID   (1<<2)

// ESTAT - Ehternet status register
#define         BM_ENC28J60_ESTAT_CLKRDY   (1<<0)
#define         BM_ENC28J60_ESTAT_TXABRT   (1<<1)
#define         BM_ENC28J60_ESTAT_RXBUSY   (1<<2)
#define         BM_ENC28J60_ESTAT_LATECOL  (1<<4)
#define         BM_ENC28J60_ESTAT_BUFER    (1<<6)
#define         BM_ENC28J60_ESTAT_INT      (1<<7)

#ifdef __cplusplus
}
#endif
#endif // _ENC28J60_H

