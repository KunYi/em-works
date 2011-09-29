//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  Enc28j60.c
//
//  This File contains an Ethernet interface for the ENC28j60 Module;
//  SPI Microchip ENC28J60 Ethernet controller
//  This mainly supports initialization and Sending/Receiving Ethernet packets. 
//  Minimum error handling is provided here...
// 
//  Code is meant to be used for ethernet download and debugging only and 
//  is expected to run in Kernel Mode...

//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#pragma warning(pop)

#include "csp.h"
#include "hw_spi.h"
#include "enc28j60.h"

//------------------------------------------------------------------------------
// Defines
#define SSPCLOCK            40000      // frequency in KHZ ( 40 MHZ)

// The frequency of the output signal bit clock SSP_SCK is defined as follows:
//                 SSPCLK
// SSP_SCK  =   -----------------
//               CLOCK_DIVIDE * (1+ CLOCK_RATE)
#define DATA_TIMEOUT  0xFFFF
#define CLOCK_DIVIDE  0x04     // SSPCLOCK/4 == 10 MHZ
#define CLOCK_RATE    0x00

#define ETHER_ADDR_SIZE     6   // Size of Ethernet address
#define ETHER_HDR_SIZE      14  // Size of Ethernet header
#define MCAST_LIST_SIZE     64  // number of multicast addresses supported
#define COMMAND_SIZE_BYTES  4   // Size of the ENC28J60 Command

//
//  ENC28J60 Buffer layout (8K size)
//
//  |------------------------|  
//  |                        | <-- 0x0000
//  |                        |
//  |      TX Buffer         |
//  |      (1536 bytes)      |
//  |                        | <-- 0x0600 
//  |------------------------|
//  |                        |          
//  |    256Kbyte (not used) |
//  |                        |
//  |------------------------|
//  |                        | <-- 0x0700
//  |                        |
//  |                        |
//  |                        |
//  |                        |
//  |                        |
//  |                        |
//  |                        |
//  |                        |
//  |      RX Buffer         |
//  |      (6399 bytes)      |
//  |                        |
//  |                        |
//  |                        |
//  |                        | <-- 0x1FFF
//  |------------------------|

#define MAX_ENC28J60_BUFFER_SIZE     0x2000 // 8K

// The rx buffers needs to be placed first according to a chip errata!
// http://ww1.microchip.com/downloads/en/DeviceDoc/80254e.pdf

#define RX_BUF_ADDR_START    0x0000
#define RX_BUF_ADDR_END      0x18FF  // Dont change this
#define RX_BUFFER_SIZE       (RX_BUF_ADDR_END - RX_BUF_ADDR_START)+1

#define TX_BUF_ADDR_START    (RX_BUF_ADDR_END + 0x100)
#define TX_BUF_ADDR_END      0x1FFF
#define TX_BUFFER_SIZE       (TX_BUF_ADDR_END - TX_BUF_ADDR_START)+1

#define ADDRMSK_H   0xFF00
#define ADDRMSK_L   0x00FF

#define  MAX_MULTICAST_LIST  8

//------------------------------------------------------------------------------
// Types
typedef struct _SPI_DMA_DESCRIPTOR
{
   struct _SPI_DMA_DESCRIPTOR      *pNext;
   union {
     UINT32 CommandBits;   // Union member to manipulate all bit fields at once.
     struct {
       // Note: The values for the command field can be found in : regsapbx.h
       //              BV_APBH_CHn_CMD_COMMAND__NO_DMA_XFER
       //              BV_APBH_CHn_CMD_COMMAND__DMA_WRITE
       //              BV_APBH_CHn_CMD_COMMAND__DMA_READ
       //
       unsigned Command    : 2;   // DMA command for the DMA enginer to execute.
       unsigned Chain      : 1;   // Another cmd structure follows this one?
       unsigned IRQ        : 1;   // Interrupt on command completion?
       unsigned NandLock   : 1;   // 
       unsigned NandWait4Rdy : 1; // 
       unsigned Semaphore  : 1;   // Semaphore decremented on cmd completion?
       unsigned WaitForEnd : 1;   // Cmd should wait for an end of cmd signal from the device?
       unsigned HaltonTerminate:1;
       unsigned Reserved2  : 3;   // Reserved
       unsigned CMDWords   : 4;   // Number of PIO words in this command.
       unsigned DMABytes   : 16;  // Number of bytes to xfer with this command.
        } B;
    } DMACMD;

    PVOID    pDMABuffer;
    SSP_CTRL0  SspCtrl0;
    SSP_CMD0   SspCmd;
    SSP_CMD1   SspArg;
} SPI_DMA_DESCRIPTOR ,*PSPI_DMA_DESCRIPTOR;

//-----------------------------------------------------------------------------
//  The packet header in RX buffer.
//
//  The packets are preceded by a six-byte header which
//  contains a next packet pointer, in addition to a receive
//  status vector which contains receive statistics, including
//  the packet’s size.
//
typedef struct 
{
    UINT8 NextPktPtr_LowByte;
    UINT8 NextPktPtr_HiByte;
    UINT8 RecvByteCount_LowByte;
    UINT8 RecvByteCount_HiByte;
    UINT8 Status_LowByte;
    UINT8 Status_HiByte;

}   PACKET_HEADER, *PPACKET_HEADER;

//------------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables
static PCSP_SSP_REGS REG = NULL;

// Virtual descriptor addresses
static PSPI_DMA_DESCRIPTOR  pVirtCMDDMADescAddr;
static PBYTE  pVirtDataDMABuffer;

// Physical descriptor addresses
static UINT32 PhyCMDDMADescAddr;
static UINT32 PhyDataDMADescAddr;

static BOOL WBM_Flag;
static BOOL IntrsEnabled;

UCHAR  g_pucMulticastList[MAX_MULTICAST_LIST][6];

// Save MacAddress
USHORT g_MacAddrSave[3];

//------------------------------------------------------------------------------
// Local Functions
static void DumpSSPRegisters(void);
static BOOL SetRegisterBank(UINT8 u8BankSel);

static UINT32 Enc28j60_RCR(UINT16 Addr);
static BOOL   Enc28j60_WCR(UINT16 addr, UINT8 val);
static BOOL   Enc28j60_RBM(UINT32 *pBuffer, UINT16 NumBytesToRead);
static BOOL   Enc28j60_BFS(UINT16 addr, UINT8 val);
static BOOL   Enc28j60_BFC(UINT16 addr, UINT8 val);
static BOOL   Enc28j60_SC(void);
static BOOL   Enc28j60_WBM(UINT32 *pBuffer, UINT16 NumBytesToWrite);
static UINT32 Enc28j60_PHY_Rd(UINT16 addr);
static BOOL   Enc28j60_PHY_Wr(UINT16 addr, UINT16 val);

static BOOL SPI_CheckErrors(void);

//-----------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------
extern void OALKitlENC28J60BSPInit(void);

//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Function: Init_Enc28j60_Mac
//
// This function will initialize the Enc28j60 MAC Module
//
// Parameters:
//      MacAddr --> [In]: Mac Address as Input.
//
// Returns:
//      TRUE if success, False if Failure .
//
//------------------------------------------------------------------------------
BOOL Init_Enc28j60_Mac(USHORT MacAddr[3])
{
    UINT32 temp = 0;
    BOOL Status = FALSE;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"Init_Enc28j60_Mac: MacAddr0:0x%x MacAddr1:0x%x MacAddr1:0x%x \r\n",MacAddr[0],MacAddr[1], MacAddr[2]));

    SetRegisterBank(BANK2);  // For access to MACON1-MACON4

    // 1. Clear the MARST bit in MACON2 to pull the MAC out of Reset.
    temp = Enc28j60_RCR(MACON2);
    temp &= ~BM_ENC28J60_MACON2_MARST;
    Enc28j60_WCR(MACON2,(UINT8)temp);

    // 2. Set the MARXEN bit in MACON1 to enable the MAC to receive frames. 
    // If using full duplex, most applications should also set TXPAUS and
    // RXPAUS to allow IEEE defined flow control to function.


    temp = Enc28j60_RCR(MACON1);
    temp |= BM_ENC28J60_MACON1_MARXEN;
    temp |= (BM_ENC28J60_MACON1_RXPAUS | BM_ENC28J60_MACON1_TXPAUS);
    temp &= ~BM_ENC28J60_MACON1_LOOPBK;
    temp &= ~BM_ENC28J60_MACON1_PASSALL;
    Enc28j60_WCR(MACON1,(UINT8)temp);

    // 3. Configure the PADCFG, TXCRCEN and FULDPX bits of MACON3. Most 
    // applications should enable automatic padding to at least
    // 60 bytes and always append a valid CRC. For convenience, many 
    // applications may wish to set the FRMLNEN bit as well to enable 
    // frame length status reporting. The FULDPX bit should be set
    // if the application will be connected to a full-duplex configured 
    // remote node; otherwise, it should be left clear.
      Enc28j60_WCR(MACON3, 0x71);

    // 5. Program the MAMXFL registers with the maximum
    // frame length to be permitted to be received
    // or transmitted. Normal network nodes are
    // designed to handle packets that are 1518 (0x5EE)bytes
    // or less.
      Enc28j60_WCR(MAMXFLL, 0xEE);
      Enc28j60_WCR(MAMXFLH, 0x05);

    // 6. Configure the Back-to-Back Inter-Packet Gap
    // register, MABBIPG. Most applications will program
    // this register with 15h when Full-Duplex
    // mode is used and 12h when Half-Duplex mode
    // is used.
      Enc28j60_WCR(MABBIPG, 0x15);

    // 7. Configure the Non-Back-to-Back Inter-Packet
    // Gap register low byte, MAIPGL. Most applications
    // will program this register with 12h.
      Enc28j60_WCR(MAIPGL, 0x12);

    //// 8. If half duplex is used, the Non-Back-to-Back
    //// Inter-Packet Gap register high byte, MAIPGH,
    //// should be programmed. Most applications will
    //// program this register to 0Ch.
      Enc28j60_WCR(MAIPGH, 0x0c);

    // 10. Program the local MAC address into the
    // MAADR0:MAADR5 registers.
    //
    // Note: Users of the ENC28J60 must generate a
    // unique MAC address for each controller used.
    //
    //
    SetRegisterBank(BANK3);
    Enc28j60_WCR(MAADR0, (UINT8)(MacAddr[2] >> 8));
    Enc28j60_WCR(MAADR1, (UINT8)(MacAddr[2] & 0x00FF));
    Enc28j60_WCR(MAADR2, (UINT8)(MacAddr[1] >> 8));
    Enc28j60_WCR(MAADR3, (UINT8)(MacAddr[1] & 0x00FF));
    Enc28j60_WCR(MAADR4, (UINT8)(MacAddr[0] >> 8));
    Enc28j60_WCR(MAADR5, (UINT8)(MacAddr[0] & 0x00FF));

    // EFLOCON
    //
    // When FULDPXS = 1:
    // 11 = Send one pause frame with a timer value then turn flow control off
    // 10 = Flow control on (pause frames will be automatically transmitted)
    // 01 = Reserved
    // 00 = Flow control off
    // When FULDPXS = 0:
    // 11 = Flow control on
    // 10 = Flow control off
    // 01 = Flow control on
    // 00 = Flow control off
    temp = Enc28j60_RCR(EFLOCON);
    temp &= ~BM_ENC28J60_EFLOCON_FCEN0;  // Flow control on
    temp |= BM_ENC28J60_EFLOCON_FCEN1;
    Enc28j60_WCR(EFLOCON, (UINT8)temp);


    //
    // Make sure to clear MIIRD and MIISCAN
    temp = Enc28j60_RCR(MICMD);
    temp &= ~BM_ENC28J60_MICMD_MIIRD;
    temp &= ~BM_ENC28J60_MICMD_MIISCAN; 
    Status = Enc28j60_WCR(MICMD,(UINT8)temp);

    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: Init_Enc28j60_Phy
//
// This function will initialize the Enc28j60 PHY Module
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success, False if Failure.
//
//------------------------------------------------------------------------------
BOOL Init_Enc28j60_Phy()
{
    UINT32 temp = 0;
    BOOL Status = FALSE;

    // Depending on the application, bits in three of the PHY
    // module’s registers may also require configuration.
    // The PHCON1.PDPXMD bit partially controls the
    // device’s half/full-duplex configuration. Normally, this bit
    // is initialized correctly by the external circuitry (see
    // Section 2.6 “LED Configuration?. If the external
    // circuitry is not present or incorrect, however, the host
    // controller must program the bit properly. Alternatively,
    // for an externally configurable system, the PDPXMD bit
    // may be read and the FULDPX bit be programmed to
    // match.
    // For proper duplex operation, the PHCON1.PDPXMD
    // bit must also match the value of the MACON3.FULDPX bit.
    temp   =  Enc28j60_PHY_Rd(PHCON1);
    temp  |= (BM_ENC28J60_PHCON1_PDPXMD);  // PHY operates in Full Duplex mode
    Status = Enc28j60_PHY_Wr(PHCON1, (UINT16)temp);

    // If using half duplex, the host controller may wish to set
    // the PHCON2.HDLDIS bit to prevent automatic
    // loopback of the data which is transmitted.
    // The PHY register, PHLCON, controls the outputs of
    // LEDA and LEDB. If an application requires a LED
    // configuration other than the default, PHLCON must be
    // altered to match the new requirements. The settings for
    // LED operation are discussed in Section 2.6 “LED
    // Configuration? The PHLCON register is shown in
    // Register 2-2 (page 9).

    Status = Enc28j60_PHY_Wr(PHCON2, (UINT16)0x100);
    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: DumpSSPRegisters
//
static void DumpSSPRegisters(void)
{   
    OALMSGS(1, (TEXT("DumpSSPRegisters ++ \r\n")));

    OALMSGS(1, (TEXT("SSP: SSP_CTRL0    = 0x%x\r\n"),INREG32(&REG->CTRL0)));
    OALMSGS(1, (TEXT("SSP: SSP_CTRL1    = 0x%x\r\n"),INREG32(&REG->CTRL1)));
    OALMSGS(1, (TEXT("SSP: SSP_CMD0     = 0x%x\r\n"),INREG32(&REG->CMD0)));
    OALMSGS(1, (TEXT("SSP: SSP_CMD1     = 0x%x\r\n"),INREG32(&REG->CMD1)));
    OALMSGS(1, (TEXT("SSP: SSP_COMPREF  = 0x%x\r\n"),INREG32(&REG->COMPREF)));
    OALMSGS(1, (TEXT("SSP: SSP_COMPMASK = 0x%x\r\n"),INREG32(&REG->COMPMASK)));
    OALMSGS(1, (TEXT("SSP: SSP_TIMING   = 0x%x\r\n"),INREG32(&REG->TIMING)));
    OALMSGS(1, (TEXT("SSP: SSP_DATA     = 0x%x\r\n"),INREG32(&REG->DATA)));
    OALMSGS(1, (TEXT("SSP: SSP_SDRESP0  = 0x%x\r\n"),INREG32(&REG->SDRESP0)));
    OALMSGS(1, (TEXT("SSP: SSP_SDRESP1  = 0x%x\r\n"),INREG32(&REG->SDRESP1)));
    OALMSGS(1, (TEXT("SSP: SSP_SDRESP2  = 0x%x\r\n"),INREG32(&REG->SDRESP2)));
    OALMSGS(1, (TEXT("SSP: SSP_SDRESP3  = 0x%x\r\n"),INREG32(&REG->SDRESP3)));
    OALMSGS(1, (TEXT("SSP: SSP_STATUS   = 0x%x\r\n"),INREG32(&REG->STATUS)));
    OALMSGS(1, (TEXT("SSP: SSP_DEBUG    = 0x%x\r\n"),INREG32(&REG->DEBUGREG)));

    OALMSGS(1, (TEXT("DumpSSPRegisters -- \r\n")));
}
//------------------------------------------------------------------------------
//
// Function: DumpAllEnc28j60Regs
//
static void DumpAllEnc28j60Regs()
{
    SetRegisterBank(BANK1);

    OALMSGS(1, (TEXT("ERXFCON = 0x%x\r\n"),  Enc28j60_RCR(ERXFCON)));
    OALMSGS(1, (TEXT("EPKTCNT = 0x%x\r\n"),  Enc28j60_RCR(EPKTCNT)));

    SetRegisterBank(BANK2);

    Enc28j60_RCR(MACON1);
    OALMSGS(1, (TEXT("MACON1  = 0x%x\r\n"), Enc28j60_RCR(MACON1)));

    Enc28j60_RCR(MACON2);
    OALMSGS(1, (TEXT("MACON2  = 0x%x\r\n"), Enc28j60_RCR(MACON2)));

    Enc28j60_RCR(MACON3);
    OALMSGS(1, (TEXT("MACON3  = 0x%x\r\n"), Enc28j60_RCR(MACON3)));

    Enc28j60_RCR(MACON4);
    OALMSGS(1, (TEXT("MACON4  = 0x%x\r\n"), Enc28j60_RCR(MACON4)));

    Enc28j60_RCR(MAMXFLL);
    OALMSGS(1, (TEXT("MAMXFLL = 0x%x\r\n"), Enc28j60_RCR(MAMXFLL)));

    Enc28j60_RCR(MAMXFLH);
    OALMSGS(1, (TEXT("MAMXFLH = 0x%x\r\n"), Enc28j60_RCR(MAMXFLH)));

    Enc28j60_RCR(MABBIPG);
    OALMSGS(1, (TEXT("MABBIPG = 0x%x\r\n"), Enc28j60_RCR(MABBIPG)));

    Enc28j60_RCR(MAIPGL);
    OALMSGS(1, (TEXT("MAIPGL  = 0x%x\r\n"), Enc28j60_RCR(MAIPGL)));

    Enc28j60_RCR(MAIPGH);
    OALMSGS(1, (TEXT("MAIPGH  = 0x%x\r\n"), Enc28j60_RCR(MAIPGH)));

    Enc28j60_RCR(MACLCON1);
    OALMSGS(1, (TEXT("MACLCON1= 0x%x\r\n"), Enc28j60_RCR(MACLCON1)));

    Enc28j60_RCR(MACLCON2);
    OALMSGS(1, (TEXT("MACLCON2= 0x%x\r\n"), Enc28j60_RCR(MACLCON2)));

    SetRegisterBank(BANK3);

    Enc28j60_RCR(MAADR0);
    OALMSGS(1, (TEXT("MAADR0  = 0x%x\r\n"), Enc28j60_RCR(MAADR0)));
    Enc28j60_RCR(MAADR1);
    OALMSGS(1, (TEXT("MAADR1  = 0x%x\r\n"), Enc28j60_RCR(MAADR1)));
    Enc28j60_RCR(MAADR2);
    OALMSGS(1, (TEXT("MAADR2  = 0x%x\r\n"), Enc28j60_RCR(MAADR2)));
    Enc28j60_RCR(MAADR3);
    OALMSGS(1, (TEXT("MAADR3  = 0x%x\r\n"), Enc28j60_RCR(MAADR3)));
    Enc28j60_RCR(MAADR4);
    OALMSGS(1, (TEXT("MAADR4  = 0x%x\r\n"), Enc28j60_RCR(MAADR4)));
    Enc28j60_RCR(MAADR5);
    OALMSGS(1, (TEXT("MAADR5  = 0x%x\r\n"), Enc28j60_RCR(MAADR5)));

    OALMSGS(1, (TEXT("EFLOCON = 0x%x\r\n"), Enc28j60_RCR(EFLOCON)));
    OALMSGS(1, (TEXT("EPAUSL  = 0x%x\r\n"), Enc28j60_RCR(EPAUSL)));
    OALMSGS(1, (TEXT("EPAUSH  = 0x%x\r\n"), Enc28j60_RCR(EPAUSH)));
    OALMSGS(1, (TEXT("PHCON1  = 0x%x\r\n"), Enc28j60_PHY_Rd(PHCON1)));
    OALMSGS(1, (TEXT("PHSTAT1 = 0x%x\r\n"), Enc28j60_PHY_Rd(PHSTAT1)));
    OALMSGS(1, (TEXT("PHID1   = 0x%x\r\n"), Enc28j60_PHY_Rd(PHID1)));
    OALMSGS(1, (TEXT("PHID2   = 0x%x\r\n"), Enc28j60_PHY_Rd(PHID2)));
    OALMSGS(1, (TEXT("PHCON2  = 0x%x\r\n"), Enc28j60_PHY_Rd(PHCON2)));
    OALMSGS(1, (TEXT("PHSTAT2 = 0x%x\r\n"), Enc28j60_PHY_Rd(PHSTAT2)));
    OALMSGS(1, (TEXT("PHIE    = 0x%x\r\n"), Enc28j60_PHY_Rd(PHIE)));
    OALMSGS(1, (TEXT("PHIR    = 0x%x\r\n"), Enc28j60_PHY_Rd(PHIR)));
    OALMSGS(1, (TEXT("PHLCON  = 0x%x\r\n"), Enc28j60_PHY_Rd(PHLCON)));

    SetRegisterBank(BANK0);

    OALMSGS(1, (TEXT("EIE   = 0x%x\r\n"),  Enc28j60_RCR(EIE)));
    OALMSGS(1, (TEXT("ECON1 = 0x%x\r\n"),  Enc28j60_RCR(ECON1)));
    OALMSGS(1, (TEXT("ECON2 = 0x%x\r\n"),  Enc28j60_RCR(ECON2)));

    OALMSGS(1, (TEXT("ETXSTL = 0x%x\r\n"), Enc28j60_RCR(ETXSTL)));
    OALMSGS(1, (TEXT("ETXSTH = 0x%x\r\n"), Enc28j60_RCR(ETXSTH)));
    OALMSGS(1, (TEXT("ETXNDL = 0x%x\r\n"), Enc28j60_RCR(ETXNDL)));
    OALMSGS(1, (TEXT("ETXNDH = 0x%x\r\n"), Enc28j60_RCR(ETXNDH)));
    OALMSGS(1, (TEXT("EWRPTL = 0x%x\r\n"), Enc28j60_RCR(EWRPTL)));
    OALMSGS(1, (TEXT("EWRPTH = 0x%x\r\n"), Enc28j60_RCR(EWRPTH)));
    OALMSGS(1, (TEXT("ERXSTL = 0x%x\r\n"), Enc28j60_RCR(ERXSTL)));
    OALMSGS(1, (TEXT("ERXSTH = 0x%x\r\n"), Enc28j60_RCR(ERXSTH)));
    OALMSGS(1, (TEXT("ERXNDL = 0x%x\r\n"), Enc28j60_RCR(ERXNDL)));
    OALMSGS(1, (TEXT("ERXNDH = 0x%x\r\n"), Enc28j60_RCR(ERXNDH)));
    OALMSGS(1, (TEXT("ERDPTL = 0x%x\r\n"), Enc28j60_RCR(ERDPTL)));
    OALMSGS(1, (TEXT("ERDPTH = 0x%x\r\n"), Enc28j60_RCR(ERDPTH)));
    OALMSGS(1, (TEXT("ERXWRPTL = 0x%x\r\n"),  Enc28j60_RCR(ERXWRPTL)));
    OALMSGS(1, (TEXT("ERXWRPTH = 0x%x\r\n"),  Enc28j60_RCR(ERXWRPTH)));
    OALMSGS(1, (TEXT("ERXRDPTL = 0x%x\r\n"),  Enc28j60_RCR(ERXRDPTL)));
    OALMSGS(1, (TEXT("ERXRDPTH = 0x%x\r\n"),  Enc28j60_RCR(ERXRDPTH)));
} 
//------------------------------------------------------------------------------
//
// Function: ResetReadBufferPointers
//
static void ResetReadBufferPointers()
{
    // First issue a system soft reset command to the chip
    if ( TRUE != Enc28j60_SC() )
    {
        OALMSGS(OAL_ERROR, (TEXT("ResetReadBufferPointers failed to detect SSP ENC28J60 Nic.\r\n")));
    }

    OALStall(1000); // Wait at least 1 millisecond

    SetRegisterBank(BANK0);

    // Need to receive and transmit logic is disabled 
    // Make sure ECON1:RXEN is cleared
    Enc28j60_BFC(ECON1, BM_ENC28J60_ECON1_RXEN);

    // Program Transmit Buffer address
    Enc28j60_WCR(ETXSTL, TX_BUF_ADDR_START&0x00FF);
    Enc28j60_WCR(ETXSTH, (TX_BUF_ADDR_START>>8));
    Enc28j60_WCR(ETXNDL, TX_BUF_ADDR_END&0x00FF);
    Enc28j60_WCR(ETXNDH, (TX_BUF_ADDR_END>>8));

    // Program write pointer
    Enc28j60_WCR(EWRPTL, (TX_BUF_ADDR_START&0x00FF));
    Enc28j60_WCR(EWRPTH, (TX_BUF_ADDR_START>>8));

    // Program Receive Buffer address
    Enc28j60_WCR(ERXSTL, (RX_BUF_ADDR_START&0x00FF));
    Enc28j60_WCR(ERXSTH, (RX_BUF_ADDR_START>>8));
    Enc28j60_WCR(ERXNDL, (RX_BUF_ADDR_END&0x00FF));
    Enc28j60_WCR(ERXNDH, (RX_BUF_ADDR_END>>8));

    // Program read pointer
    Enc28j60_WCR(ERDPTL, (RX_BUF_ADDR_START&0x00FF));
    Enc28j60_WCR(ERDPTH, (RX_BUF_ADDR_START>>8));

    // Make sure AUTOINC is set
    Enc28j60_BFS(ECON2, BM_ENC28J60_ECON2_AUTOINC); 

    // When programming the ERXST pointer, the ERXWRPT
    // registers will automatically be updated with the same
    // values. The address in ERXWRPT will be used as the
    // starting location when the receive hardware begins writing
    // received data.

    // For tracking purposes, the ERXRDPT
    // registers should additionally be programmed with the
    // same value.
    Enc28j60_WCR(ERXRDPTL, RX_BUF_ADDR_START&0x00FF);
    Enc28j60_WCR(ERXRDPTH, RX_BUF_ADDR_START>>8);

    // MAC Initialization Settings
    if ( TRUE != Init_Enc28j60_Mac(&g_MacAddrSave[0])) {
        OALMSGS(OAL_ERROR, (L"ERROR: ResetReadBufferPointers Failed to init MAC Settings.\r\n"));
    }

    // PHY Initiialization settings
    if ( TRUE != Init_Enc28j60_Phy()) {
        OALMSGS(OAL_ERROR, (L"ERROR: ResetReadBufferPointers Failed to init SPI Phy Settings.\r\n"));
    }
    // Set receive filters
    SetRegisterBank(BANK1);
    Enc28j60_BFC(ERXFCON, BM_ENC28J60_ERXFCON_ANDOR );

    Enc28j60_BFS(ERXFCON, BM_ENC28J60_ERXFCON_CRCEN |
                          BM_ENC28J60_ERXFCON_BCEN  |
                          BM_ENC28J60_ERXFCON_UCEN  |
                          BM_ENC28J60_ERXFCON_MCEN);

    // Enable reception by setting ECON1.RXEN.
    Enc28j60_BFS(ECON1, BM_ENC28J60_ECON1_RXEN);

}
//------------------------------------------------------------------------------
//
// Function: SPI_ENC28J60EnableInts
//------------------------------------------------------------------------------
VOID SPI_ENC28J60EnableInts()
{
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+SPI_ENC28J60EnableInts\r\n"));

    OALMSGS(1, (TEXT("Enabling Eth RX interrupts.\r\n")));
    // Clear it first.
    Enc28j60_BFC(EIE, BM_ENC28J60_EIE_PKTIE |
                 BM_ENC28J60_EIE_INTIE);

    // Only enable receive interrupts.
    Enc28j60_BFS(EIE, BM_ENC28J60_EIE_PKTIE |
                 BM_ENC28J60_EIE_INTIE);

    IntrsEnabled = TRUE;
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-SPI_ENC28J60EnableInts\r\n"));
}

//------------------------------------------------------------------------------
//
// Function: SPI_ENC28J60DisableInts
//
//------------------------------------------------------------------------------
VOID SPI_ENC28J60DisableInts()
{
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+SPI_ENC28J60DisableInts\r\n"));

    OALMSGS(1, (TEXT("Disabling Eth RX Interrupts.\r\n")));
    // Disable receive interrupts.
    Enc28j60_BFC(EIE, BM_ENC28J60_EIE_PKTIE |
                 BM_ENC28J60_EIE_INTIE);

    IntrsEnabled = FALSE;
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-SPI_ENC28J60DisableInts\r\n"));
}
//------------------------------------------------------------------------------
//
//  Function:  ETHInitDMABuffer
//
//  This function is used to inform this library on where the buffer for
//  Tx/Rx DMA is. Driver needs  at least:
//    
//   
//------------------------------------------------------------------------------
BOOL ETHInitDMABuffer(UINT32 address, UINT32 size)
{
    BOOL rc = FALSE;
    UINT32 BufferOffset;
    UINT32 ph;
    
    //EdbgOutputDebugString("+ETHInitDMABuffer(0x%X, 0x%X)\r\n", address, size);
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+ETHInitDMABuffer(0x%X, 0x%X)\r\n", address, size));

    ph = OALVAtoPA((VOID*)address);
    //128bit align
    if ((ph & 0x0f) != 0)
    {
        size -= 16 - (ph & 0x0f);
        ph = (ph + 0x0f) & ~0x0f;
    }

    // Check RAM buffer size
    if(size < 16 * 1024 )
    {
        EdbgOutputDebugString("ETHInitDMABuffer DMA Buffer too small\r\n");
        OALMSGS(OAL_ERROR, (L"ERROR: ETHInitDMABuffer: DMA Buffer too small\r\n" ));
        return rc;
    }

    PhyCMDDMADescAddr = ph; 
    pVirtCMDDMADescAddr = (PSPI_DMA_DESCRIPTOR)OALPAtoUA(ph); 

    BufferOffset = sizeof(SPI_DMA_DESCRIPTOR);
    
    PhyDataDMADescAddr = PhyCMDDMADescAddr +  BufferOffset;   

    pVirtCMDDMADescAddr->pDMABuffer = (PVOID)PhyDataDMADescAddr;

    pVirtDataDMABuffer = (PBYTE )((PBYTE)pVirtCMDDMADescAddr + BufferOffset);

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-ETHInitDMABuffer(0x%x),(0x%x),( 0x%x),(0x%x)\r\n",
                                     PhyCMDDMADescAddr,pVirtCMDDMADescAddr,
                                     PhyDataDMADescAddr,pVirtDataDMABuffer));

    rc = TRUE;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-ETHInitDMABuffer(rc = %d)\r\n", rc));
    return TRUE;
}
//------------------------------------------------------------------------------
// Function: SPI_Init
//
// This function initializes the SSP Block of the Hardware . The ETH
// hardware initialization process includes:
//
// Parameters:
//        pAddr
//            [in] the ETH driver context area allocated in function ETHInitialize
//
// Return Value:
//        returns TRUE if the initialization process is successful, otherwise 
//        returns FALSE.
//
//------------------------------------------------------------------------------
BOOL SPI_Init(UINT8 *pAddr)
{
    SSP_CTRL0 sControl0;
    SSP_CTRL1 sControl1;
    UINT32 u32Timing;
    UINT32 SSPFreq = SSPCLOCK;
    //BOOL bUseRefIo = FALSE;

    OALMSGS(1, (L"SPI_Init ++ 0x%x \r\n",pAddr));
   
    REG = (PCSP_SSP_REGS)pAddr;

    // Reset the SSP DMA Channel and Enable DMA Interrupt
    if(!DDKApbhDmaInitChan(DDK_APBH_CHANNEL_SSP1,TRUE))
    {
       OALMSGS(1, (L"ERROR: SPI_Init: Failed to  Init SSP DMA\r\n" ));
       return FALSE;
    }

    // configures the IOMUX pin for SSP1 used for SPI/Ethernet 
    OALKitlENC28J60BSPInit();

//------------------------------------------------------------------------
    // Reset the block.
    // Prepare for soft-reset by making sure that SFTRST is not currently
    // asserted.

    OUTREG32 (&REG->CTRL0[CLRREG],BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond for SFTRST to deassert.
    INREG32(&REG->CTRL0);
    INREG32(&REG->CTRL0);
    // Also clear CLKGATE so we can wait for its assertion below.
    OUTREG32 (&REG->CTRL0[CLRREG],BM_SSP_CTRL0_CLKGATE);

    // Now soft-reset the hardware.
    OUTREG32 ( &REG->CTRL0[SETREG], BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond for SFTRST to deassert.
    INREG32(&REG->CTRL0);
    INREG32(&REG->CTRL0);

    // Deassert SFTRST.
    OUTREG32 ( &REG->CTRL0[CLRREG], BM_SSP_CTRL0_SFTRST);

    // Wait at least a microsecond .
    INREG32(&REG->CTRL0);
    INREG32(&REG->CTRL0);

    // Release the Block from Reset and starts the clock
    OUTREG32 ( &REG->CTRL0[CLRREG], BM_SSP_CTRL0_CLKGATE);

    // Wait at least a microsecond.
    INREG32(&REG->CTRL0);
    INREG32(&REG->CTRL0);
//-----------------------------------------------------------------------

    // Configure SSP Control Register 1
    sControl1.U = 0;

    // Configure the SSP Control registers
    // SSP_CTRL1 Register values
    sControl1.B.DMA_ENABLE = TRUE;  // Enables DMA request and DMA Command End signals to be asserted.
    sControl1.B.CEATA_CCS_ERR_EN = FALSE;   // CEATA Unexpected CCS Error Enable.
    sControl1.B.SLAVE_OUT_DISABLE = FALSE;  // 0 means SSP can drive MISO in Slave Mode
    sControl1.B.PHASE = FALSE;              // Serial Clock Phase. For SPI mode only
    sControl1.B.POLARITY = FALSE;           // Command and TX data change after rising edge of SCK
    sControl1.B.WORD_LENGTH = SSP_WORD_LENGTH_8BITS;    // FALSE - SSP is in Master Mode
    sControl1.B.SLAVE_MODE = FALSE;                 // 8 Bits per word
    sControl1.B.SSP_MODE = SSP_MODE_SPI;            // SSP_MODE_SPI=0, Motorala SPI mode.

    // Configure SSP Control Register 0
    sControl0.U = 0;
    // NOte:  ENC28J60 only supports SPI Mode 0,0.
    // SSP_CTRL0 Register values
    sControl0.B.LOCK_CS = 0;   // 0= Deassert chip select when xfer is complete.
    sControl0.B.IGNORE_CRC   = 0;        // Ignores the Response CRC.
    sControl0.B.BUS_WIDTH    = 0;         // Data bus is 1-bit wide
    sControl0.B.WAIT_FOR_IRQ = FALSE;   
    sControl0.B.LONG_RESP    = FALSE;
    sControl0.B.CHECK_RESP   = FALSE;
    sControl0.B.GET_RESP     = FALSE;

    // Write the SSP Control Register 0 and 1 values out to the interface
    OUTREG32(&REG->CTRL0[BASEREG],sControl0.U);
    OUTREG32(&REG->CTRL1[BASEREG],sControl1.U);

   // Configure the SSP clock frequecy as 40 MHZ
/*    if(SSPFreq > 24000)
    {
         bUseRefIo = TRUE;
    }
*/
// Original clocking simply doesn't work with the display driver on my board.
SSPFreq = 24000;     
    //bUseRefIo = FALSE;  
    //DDKClockSetSspClk(&SSPFreq, bUseRefIo);

    DDKClockConfigBaud(DDK_CLOCK_SIGNAL_SSP_CLK, DDK_CLOCK_BAUD_SOURCE_REF_XTAL, 1 );    
    DDKClockSetGatingMode(DDK_CLOCK_GATE_SSP_CLK, FALSE);
    

    u32Timing = (UINT32)(DATA_TIMEOUT << 16) | CLOCK_DIVIDE << 8  | CLOCK_RATE;
    OUTREG32(&REG->TIMING[BASEREG],u32Timing);
    {
      UINT tmp;

      //tmp = DDKClockGetSspClk();
      DDKClockGetFreq(DDK_CLOCK_SIGNAL_SSP_CLK, &tmp);
      OALMSGS(1, (TEXT("SSP: CLOCK            = 0x%x\r\n"),tmp));
      tmp = INREG32(&REG->TIMING);
      OALMSGS(1, (TEXT("SSP: SSP_TIMING       = 0x%x\r\n"),tmp));
      if (tmp == 0) {
        OUTREG32(&REG->TIMING[BASEREG],u32Timing);
        OALMSGS(1, (TEXT("SSP: Retrying...\r\n")));
        tmp = INREG32(&REG->TIMING);
        OALMSGS(1, (TEXT("SSP: SSP_TIMING NOW   = 0x%x\r\n"),tmp));
        if (tmp == 0) {
          return(FALSE);
        }
      }
    }

    // Make certain that any changes we made to CLOCK_RATE take
    // effect, by writing to CTRL1 after TIMING (CQ PSGHW00000012) 
    OUTREG32(&REG->CTRL1[BASEREG],sControl1.U);

    DumpSSPRegisters();

    OALMSGS(1, (L"SPI_Init --\r\n"));

    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: SPI_ENC28J60Init
//
// This function Initializes SPI interfaced ENC28J60 Block.
//
// Parameters:
//      None
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
BOOL SPI_ENC28J60Init(UINT8 *pbBaseAddress, ULONG dwMemOffset, USHORT MacAddr[3])
{
    UNREFERENCED_PARAMETER(dwMemOffset);
    g_MacAddrSave[0] = MacAddr[0];
    g_MacAddrSave[1] = MacAddr[1];
    g_MacAddrSave[2] = MacAddr[2];

    OALMSGS(OAL_ETHER&&OAL_FUNC, (
        L"SPI_ENC28J60Init(0x%08x, 0x%08x, %02x:%02x:%02x:%02x:%02x:%02x)\r\n",
        pbBaseAddress, dwMemOffset, MacAddr[0]&0xFF, MacAddr[0]>>8, MacAddr[1]&0xFF, MacAddr[1]>>8,
        MacAddr[2]&0xFF, MacAddr[2]>>8  ));

    //
    // Initialize SPI hardware
    //
    if (SPI_Init(pbBaseAddress) == FALSE) {
      OALMSGS(OAL_ERROR, (L"ERROR: SPI_ENC28J60Init Could not reset SPI\r\n"));
      return FALSE;
    }

    WBM_Flag     = FALSE;
    IntrsEnabled = FALSE;

    // Read revision ID of Chip
    SetRegisterBank(BANK3);
    if ( !Enc28j60_RCR(EREVID))
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_ENC28J60Init Failed detect SPI Ethernet.\r\n"));
        return FALSE;
    }

    // First issue a system soft reset command to the chip
    if ( TRUE != Enc28j60_SC() )
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_ENC28J60Init system soft Reset Failed.\r\n"));
        return FALSE;
    }
    OALStall(1000);// Wait for at Least 1 Millisecond

    SetRegisterBank(BANK0);

    // Need to receive and transmit logic is disabled 
    // Make sure ECON1:RXEN is cleared
    Enc28j60_BFC(ECON1, BM_ENC28J60_ECON1_RXEN);

    //Program Transmit Buffer address
    Enc28j60_WCR(ETXSTL, TX_BUF_ADDR_START&0x00FF);
    Enc28j60_WCR(ETXSTH, (TX_BUF_ADDR_START>>8));
    Enc28j60_WCR(ETXNDL, TX_BUF_ADDR_END&0x00FF);
    Enc28j60_WCR(ETXNDH, (TX_BUF_ADDR_END>>8));

    // Program write pointer
    Enc28j60_WCR(EWRPTL, (TX_BUF_ADDR_START&0x00FF));
    Enc28j60_WCR(EWRPTH, (TX_BUF_ADDR_START>>8));

    // Program Receive Buffer address
    Enc28j60_WCR(ERXSTL, (RX_BUF_ADDR_START&0x00FF));
    Enc28j60_WCR(ERXSTH, (RX_BUF_ADDR_START>>8));
    Enc28j60_WCR(ERXNDL, (RX_BUF_ADDR_END&0x00FF));
    Enc28j60_WCR(ERXNDH, (RX_BUF_ADDR_END>>8));

    // Program read pointer
    Enc28j60_WCR(ERDPTL, (RX_BUF_ADDR_START&0x00FF));
    Enc28j60_WCR(ERDPTH, (RX_BUF_ADDR_START>>8));

    // Make sure AUTOINC is set
    Enc28j60_BFS(ECON2, BM_ENC28J60_ECON2_AUTOINC); 

    // When programming the ERXST pointer, the ERXWRPT
    // registers will automatically be updated with the same
    // values. The address in ERXWRPT will be used as the
    // starting location when the receive hardware begins writing
    // received data.

    // For tracking purposes, the ERXRDPT
    // registers should additionally be programmed with the
    // same value.
    Enc28j60_WCR(ERXRDPTL, RX_BUF_ADDR_START&0x00FF);
    Enc28j60_WCR(ERXRDPTH, RX_BUF_ADDR_START>>8);

    // MAC Initialization Settings
    if ( TRUE != Init_Enc28j60_Mac(&MacAddr[0]))
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_ENC28J60Init :Failed SPI MAC Settings.\r\n"));
        return FALSE;
    }

    // PHY Initiialization settings
    if ( TRUE != Init_Enc28j60_Phy() )
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_ENC28J60Init:Failed SPI Phy Settings.\r\n"));
        return FALSE;
    }
    // Set receive filters
    SetRegisterBank(BANK1);

    Enc28j60_BFC(ERXFCON, BM_ENC28J60_ERXFCON_ANDOR );
    Enc28j60_BFS(ERXFCON, BM_ENC28J60_ERXFCON_CRCEN |
                          BM_ENC28J60_ERXFCON_BCEN  |
                          BM_ENC28J60_ERXFCON_UCEN  |
                          BM_ENC28J60_ERXFCON_MCEN);

#if 0// For Debug use only
    DumpAllEnc28j60Regs();  
#endif

    // Enable reception by setting ECON1.RXEN.
    Enc28j60_BFS(ECON1,BM_ENC28J60_ECON1_RXEN);

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-ENC28J60Init\r\n"));
    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: SPI_ENC28J60SendFrame
//
//------------------------------------------------------------------------------
UINT16  SPI_ENC28J60SendFrame(BYTE *pbData, DWORD dwLength)
{
    UINT8 PacketControlByte = 0;
    UINT32 temp = 0;
    static int count = 0;

    if (dwLength > TX_BUFFER_SIZE)
    {
        OALMSGS(OAL_ERROR, (L"ERROR: ENC28J60SendFrame: Exceeds buffer max size 0x%x\r\n",dwLength));
        return FALSE;
    }

    // Make sure the transmission is stopped
    Enc28j60_BFC(ECON1, BM_ENC28J60_ECON1_TXRTS);
    PacketControlByte = (TX_PCRCEN | TX_PPADEN );

    // Program TX start and end address 
    // Program TX start address
    SetRegisterBank(BANK0);

    Enc28j60_WCR(ETXSTL,(TX_BUF_ADDR_START&0x00FF));
    Enc28j60_WCR(ETXSTH,(TX_BUF_ADDR_START>>8));

    // Program TX end address
    temp = TX_BUF_ADDR_START + dwLength ;
    if (temp & 0x0001)
        temp++;
    Enc28j60_WCR(ETXNDL, (reg8_t)(temp&0x00FF));
    Enc28j60_WCR(ETXNDH, (reg8_t)(temp>>8));

    // Set write buffer pointer
    Enc28j60_WCR(EWRPTL, (TX_BUF_ADDR_START&0x00FF));
    Enc28j60_WCR(EWRPTH, (TX_BUF_ADDR_START>>8));

    // Send the control byte first.
    Enc28j60_WBM((UINT32 *)&PacketControlByte, 1);

    // Send the data
    Enc28j60_WBM((UINT32 *)pbData, (UINT16)dwLength);

    // Start the transmission
    Enc28j60_BFS(ECON1, BM_ENC28J60_ECON1_TXRTS);

    // ECON1.TXRTS will clear when packet is done transmitting..
    while ( Enc28j60_RCR(ECON1) & BM_ENC28J60_ECON1_TXRTS) {
        OALStall(20);
    }

    if( Enc28j60_RCR(ESTAT) & BM_ENC28J60_ESTAT_TXABRT ) {
        OALMSGS(OAL_ERROR, (L"SPI_ENC28J60SendFrame: TX Packet Aborted  0x%x :\r\n",Enc28j60_RCR(ESTAT)));
    }

    // Make sure the transmission is stopped
    Enc28j60_BFC(ECON1, BM_ENC28J60_ECON1_TXRTS);
    Enc28j60_BFC(EIR,   BM_ENC28J60_EIR_TXIF);

    return 0; // Return 0 for success. 
}
//------------------------------------------------------------------------------
//
// Function: SPI_ENC28J60GetFrame
//
//------------------------------------------------------------------------------
UINT16  SPI_ENC28J60GetFrame(BYTE *pbData, UINT16 *pwLength)
{   
    static int count = 0;
    PACKET_HEADER PacketHdr;
    UINT32 PacKetCount;
    UINT32 i = 0;
    UINT16 NextPacketPtr = 0;
    UINT32 RecvByteCnt = 0;
    UINT32 erdpt = 0;
    static UINT32 erdpt_save = 0;

    count++;

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (L"+SPI_ENC28J60GetFrame(0x%08x, %d)\r\n", pbData, *pwLength));

    // check if a packet has been received and buffered 
    SetRegisterBank(BANK1);
    PacKetCount = Enc28j60_RCR(EPKTCNT);
    if (PacKetCount == 0) {

      // Please note there is an Errata for the ENC chip that claims
      // that the interrupt receive bit sometimes does NOT indicate that
      // a packet has been received when in actuality it was.
      if (IntrsEnabled == TRUE) {
        // Check to see if the interrupt bit is set, in which case
        // it won't be automatically cleared by processing the incoming
        // packets because there are none reported.
        if (Enc28j60_RCR(ESTAT) & BM_ENC28J60_ESTAT_INT) {
          if (Enc28j60_RCR(ESTAT) & BM_ENC28J60_ESTAT_BUFER) {
            // Receive overflow !
            Enc28j60_BFC(EIR, BM_ENC28J60_EIR_RXERIF);
            Enc28j60_BFC(ESTAT, BM_ENC28J60_ESTAT_BUFER);
          }
          // Re-read to avoid timing problem.
          SetRegisterBank(BANK1);
          PacKetCount = Enc28j60_RCR(EPKTCNT);
          if (PacKetCount == 0) {
            // Still no packets available so just return.
            *pwLength = 0;
            return 0;
          }
        }
        else {
          UINT32 tmp;

          // This case happens now and then. Interrupts are enabled,
          // an ARM GPIO interrupt has triggered but the Ethernet
          // controller doesn't report any interrupts.... clear the
          // GPIO interrupt if set.
          DDKGpioReadIntr(DDK_IOMUX_SSP1_D1, &tmp);
          if (tmp & 0x8) {
            DDKGpioIRQSTATCLR(DDK_IOMUX_SSP1_D1);
          }
        }
     }
     if (PacKetCount == 0) {
      // No packets available so just return a 0 back to the caller
      *pwLength = 0;
      return 0;
     }
    }

    memset(&PacketHdr, 0, sizeof(PacketHdr));

    // Get the 6 byte packet header
    Enc28j60_RBM((UINT32 *)&PacketHdr, sizeof(PacketHdr));
    RecvByteCnt   = ((PacketHdr.RecvByteCount_HiByte<<8) | PacketHdr.RecvByteCount_LowByte);
    NextPacketPtr = ((PacketHdr.NextPktPtr_HiByte<<8)    | PacketHdr.NextPktPtr_LowByte);

    OALMSGS(OAL_ETHER&&OAL_FUNC, (TEXT("NxtPckPtr: (0x%x) RcvByteCnt=0x%x%x(0x%x) [0x%x%x] pck = %d\r\n"),
                                  NextPacketPtr,PacketHdr.RecvByteCount_HiByte,PacketHdr.RecvByteCount_LowByte,
                                  RecvByteCnt,PacketHdr.Status_HiByte,PacketHdr.Status_LowByte,
                                  PacKetCount));

    // Perform some parameter checking..
    if ( RecvByteCnt > RX_BUFFER_SIZE )
    {
        OALMSGS(OAL_ERROR, (L"Error: Pckt=%d, Cnt=%d, RecvByteCnt(0x%x) > RX_BUFFER_SIZE(0x%x)\r\n",
                              PacKetCount, count, RecvByteCnt, RX_BUFFER_SIZE));

        ResetReadBufferPointers();
        RecvByteCnt = 0;
        *pwLength = (UINT16)RecvByteCnt;
        return *pwLength;  
    }

    // Check receive status
    if ( (PacketHdr.Status_LowByte & RCV_OK)) {
        ;
    }
    else {
        // Rcv packet not okay.. reject 
        OALMSGS(OAL_ERROR, (TEXT("Packet not ok. Stat = 0x%X, pckt=%d, cnt=%d\r\n"),
                            PacketHdr.Status_LowByte, PacKetCount, count ));

        //Dummy read to advance the pointer to point to beginning of next packet..
        Enc28j60_RBM((UINT32 *)pbData, (UINT16)RecvByteCnt);
        RecvByteCnt = 0;
        goto VERIFYRDBUFFERPOINTER;
    }

    if ( TRUE != Enc28j60_RBM((UINT32 *)pbData, (UINT16)RecvByteCnt) ) {
        OALMSGS(OAL_ERROR, (TEXT("ERROR: SPI_ENC28J60GetFrame RBM failed\r\n")));
        ResetReadBufferPointers();
        RecvByteCnt = 0;
        *pwLength = (UINT16)RecvByteCnt;
        return (UINT16)RecvByteCnt;  
    }

VERIFYRDBUFFERPOINTER:
    // At this point, ERDPT which is our current read buffer pointer that is advanced
    // automatically should be equal to "NextPacketPtr".
    SetRegisterBank(BANK0);
    erdpt = (Enc28j60_RCR(ERDPTH)<<8 | Enc28j60_RCR(ERDPTL));
    if  (  erdpt != NextPacketPtr ) {
        //Dummy read to increment ERDPT
        Enc28j60_RBM((UINT32 *)&i, 1);
    }

    // Advance the receive buffer read pointer (ERXRDP)
    // The ENC28J60 will always write up to, but not including,
    // the memory pointed to by the receive buffer read
    // pointer. If the ENC28J60 ever attempts to overwrite the
    // receive buffer read pointer location, the packet in
    // progress will be aborted,
    Enc28j60_WCR(ERXRDPTL, (reg8_t)(NextPacketPtr&0x00FF));
    Enc28j60_WCR(ERXRDPTH, (reg8_t) (NextPacketPtr>>8));

    // In addition to advancing the receive buffer read pointer,
    // after each packet is fully processed, the host controller
    // must write a ??to the ECON2.PKTDEC bit. Doing so
    // will cause the EPKTCNT register to decrement by 1.
    // After decrementing, if EPKTCNT is ?? the EIR.PKTIF
    // flag will automatically be cleared. Otherwise, it will
    // remain set, indicating that additional packets are in the
    // receive buffer and are waiting to be processed.
    // Attempts to decrement EPKTCNT below 0 are ignored.
    // Additionally, if the EPKTCNT register ever maximizes
    // at 255, all new packets which are received will be
    // aborted, even if buffer space is available. To indicate
    // the error, the EIR.RXERIF will be set and an interrupt
    // will be generated (if enabled). To prevent this condition,
    // the host controller must properly decrement the
    // counter whenever a packet is processed.
    Enc28j60_BFS(ECON2, BM_ENC28J60_ECON2_PKTDEC);

    // Return the number of bytes received.
    *pwLength = (UINT16)(RecvByteCnt);

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (L"ENC28J60GetFrame --\r\n"));

    // After the receive interrupt has been INDIRECTLY cleared by
    // processing the incoming packets, the GPIO interrupt must be
    // cleared.
    DDKGpioIRQSTATCLR(DDK_IOMUX_SSP1_D1); 
    return (UINT16)(RecvByteCnt);  
} 
//------------------------------------------------------------------------------
//
// Function: SPI_ENC28J60CurrentPacketFilter
//
//------------------------------------------------------------------------------
VOID SPI_ENC28J60CurrentPacketFilter(DWORD    dwFilter)
{    
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+SPI_ENC28J60CurrentPacketFilter(0x%08x)\r\n", dwFilter));

    SetRegisterBank(BANK1);
    if (dwFilter & (PACKET_TYPE_ALL_MULTICAST | PACKET_TYPE_PROMISCUOUS))
    {
        //
        //    Need all multicast..
        //
       Enc28j60_BFS(ERXFCON, BM_ENC28J60_ERXFCON_MCEN);
    }
    else        
    {
        Enc28j60_BFC(ERXFCON, BM_ENC28J60_ERXFCON_MCEN);
    }

    //    The multicast bit in the RCR should be on if multicast/all multicast packets 
    //    (or is promiscuous) is needed.
    if(dwFilter & (PACKET_TYPE_ALL_MULTICAST | PACKET_TYPE_MULTICAST | PACKET_TYPE_PROMISCUOUS))    
    {
        Enc28j60_BFS(ERXFCON, BM_ENC28J60_ERXFCON_MCEN);
    }
    else
    {
        Enc28j60_BFC(ERXFCON, BM_ENC28J60_ERXFCON_MCEN);
    }

    //
    //    The promiscuous physical bit in the RCR should be on if ANY
    //    protocol wants to be promiscuous.
    //
    // Note: The Enc28j60 device can enter Promiscuous mode and receive
    // all packets by clearing the ERXFCON register.
    //
    if (dwFilter & PACKET_TYPE_PROMISCUOUS)
    {
        Enc28j60_BFC(ERXFCON, BM_ENC28J60_ERXFCON_BCEN |
                                BM_ENC28J60_ERXFCON_MCEN  |
                                BM_ENC28J60_ERXFCON_HTEN  |
                                BM_ENC28J60_ERXFCON_MPEN  |
                                BM_ENC28J60_ERXFCON_PMEN  |
                                BM_ENC28J60_ERXFCON_CRCEN |
                                BM_ENC28J60_ERXFCON_ANDOR |
                                BM_ENC28J60_ERXFCON_UCEN  );
    }

    //
    // The broadcast bit in the RCR should be on if ANY protocol wants
    // broadcast packets (or is promiscuous).
    //
    if (dwFilter & (PACKET_TYPE_BROADCAST  | PACKET_TYPE_PROMISCUOUS )) {
      Enc28j60_BFS(ERXFCON, BM_ENC28J60_ERXFCON_BCEN);
    }
    else {
      Enc28j60_BFC(ERXFCON, BM_ENC28J60_ERXFCON_BCEN);
    }
}
//------------------------------------------------------------------------------
//
// Function: SPI_ENC28J60MulticastList
//
//------------------------------------------------------------------------------
BOOL SPI_ENC28J60MulticastList(PUCHAR pucMulticastAddresses, DWORD dwNoOfAddresses)
{
#ifdef SHIP_BUILD
    UNREFERENCED_PARAMETER(pucMulticastAddresses);
    UNREFERENCED_PARAMETER(dwNoOfAddresses);
#else
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+SPI_ENC28J60MulticastList(0x%08x) %(0x%08x)\r\n", pucMulticastAddresses,dwNoOfAddresses));
#endif
    return FALSE;
}
//
// ENC28J60 DMA  functions are implemented below.
//------------------------------------------------------------------------------
//
// Function: Ether_StartDma
//
//------------------------------------------------------------------------------
void Ether_StartDma(BOOL  bRead,       
                    BOOL  bCmd,     
                    UINT16  u16ByteCount,  // XFER_COUNT, # of bytes to transfer
                    UINT32  u32Cmd,        // 
                    UINT32  u32Arg,
                    UINT32 *p32Buffer)     // Pointer to buffer for DMA
{
   UINT8 Temp  = 0;

   OALMSGS(OAL_ETHER&&OAL_VERBOSE, (L"Ether_StartDma: ++ 0x%x 0x%x  0x%x 0x%x ",
                                            pVirtCMDDMADescAddr,pVirtDataDMABuffer,
                                            PhyCMDDMADescAddr,PhyDataDMADescAddr));

    // Set up the DMA Command Chain.
    pVirtCMDDMADescAddr->pNext = NULL;

    // First clear all the bits
    pVirtCMDDMADescAddr->DMACMD.CommandBits = 0;

    pVirtCMDDMADescAddr->DMACMD.B.DMABytes = u16ByteCount;
    pVirtCMDDMADescAddr->DMACMD.B.CMDWords = 1;
    pVirtCMDDMADescAddr->DMACMD.B.WaitForEnd = 1;
    pVirtCMDDMADescAddr->DMACMD.B.Semaphore = 1;
    pVirtCMDDMADescAddr->DMACMD.B.IRQ = 0;
    pVirtCMDDMADescAddr->DMACMD.B.Command = (bRead ? DDK_DMA_COMMAND_DMA_WRITE:
                                                     DDK_DMA_COMMAND_DMA_READ);

    memset( pVirtDataDMABuffer,0, u16ByteCount );
    
    if(bRead != TRUE)
    {
        memcpy(pVirtDataDMABuffer,p32Buffer,u16ByteCount);
    }
    pVirtCMDDMADescAddr->SspCtrl0.U = 0 ;

    pVirtCMDDMADescAddr->SspCtrl0.B.LOCK_CS =1 ;

    if((bRead != TRUE) && (u16ByteCount ==1))
    {
        Temp = (UINT8)*p32Buffer;
        Temp = Temp>>5;
        
        if((Temp == OP_WBM)||(Temp == OP_RBM))
        {
            WBM_Flag = TRUE;
            pVirtCMDDMADescAddr->SspCtrl0.B.IGNORE_CRC=0 ;
        }
        else if ((Temp == 0)&&(WBM_Flag != TRUE))
        {
            pVirtCMDDMADescAddr->SspCtrl0.B.IGNORE_CRC=0 ;
        }
        else
        {
           pVirtCMDDMADescAddr->SspCtrl0.B.IGNORE_CRC=1 ;
        }
    }
    else
    { 
        pVirtCMDDMADescAddr->SspCtrl0.B.IGNORE_CRC=1 ;
    }

    pVirtCMDDMADescAddr->SspCtrl0.B.READ=(bRead ? 1 : 0) ;
    pVirtCMDDMADescAddr->SspCtrl0.B.DATA_XFER =1 ;
    pVirtCMDDMADescAddr->SspCtrl0.B.BUS_WIDTH =0 ;
    pVirtCMDDMADescAddr->SspCtrl0.B.WAIT_FOR_IRQ = 0;
    pVirtCMDDMADescAddr->SspCtrl0.B.WAIT_FOR_CMD = 0;
    pVirtCMDDMADescAddr->SspCtrl0.B.GET_RESP =(bCmd ? 1 : 0) ;
    pVirtCMDDMADescAddr->SspCtrl0.B.ENABLE =(bCmd ? 1 : 0) ;
    pVirtCMDDMADescAddr->SspCtrl0.B.XFER_COUNT = u16ByteCount ;

    if(bCmd)
    {
        pVirtCMDDMADescAddr->SspCmd.U = u32Cmd;
        pVirtCMDDMADescAddr->SspArg.U = u32Arg;
    }

    // Clear the IRQ.
    // SSP1 DMA interrupts were not really being used, so no need to
    // clear the DMA IRQ.
    // DDKApbhDmaClearCommandCmpltIrq(DDK_APBH_CHANNEL_SSP1);
    DDKApbhStartDma(DDK_APBH_CHANNEL_SSP1,(PVOID)PhyCMDDMADescAddr,1);
}
//------------------------------------------------------------------------------
//
// Function: Ether_WaitDma
//
//------------------------------------------------------------------------------
BOOL Ether_WaitDma(void)
{
    UINT32 u32Sema;

    // SSP1 DMA interrupts were not really being used, so no need to
    // clear the DMA IRQ.
//  DDKApbhDmaClearCommandCmpltIrq(DDK_APBH_CHANNEL_SSP1);

    do
    {
      u32Sema = DDKApbhDmaGetPhore(DDK_APBH_CHANNEL_SSP1);
    }while (u32Sema != 0);

    // if timeout return error
    if (u32Sema != 0)
    {
        // abort dma by resetting dma channel
        DDKApbhDmaResetChan(DDK_APBH_CHANNEL_SSP1,TRUE);
        OALMSGS(ZONE_ERROR, (L"ETH: StartCmdDma  SSP_DMA_TIMEOUT u32Sema = 0x%x",u32Sema));
        return FALSE;
    }
    return TRUE; 
}

//
//    ENC28J60 COMMANDS functions are implemented below.

//------------------------------------------------------------------------------
//
// Function: Enc28j60_RCR
// Function to send an command to a device to retrieve a  a control register value
//------------------------------------------------------------------------------
static UINT32 Enc28j60_RCR(UINT16 addr)
{
    BOOL dStatus, eStatus = FALSE;
    UINT32 cmd = 0;
    UINT32 Value = 0;

    //
    // Strip out the bank select bits (bits 8-15) in "addr" that is passed 
    // into this function. All we want is the actual command.
    addr = (addr & ~BANKSEL_MASK);
    cmd = ((OP_RCR << 5) | addr);

// OALMSGS(1, (L"ETH: Enc28j60_RCR Called: 0x%x  0x%x  +\n\r",cmd , addr));

    Ether_StartDma(FALSE,  // bRead=FALSE -> Write
                   TRUE,   // bCmd=TRUE->Always set to TRUE
                   1,      // # of DMA bytes to transfer
                   0x0,    // Set to 0 always
                   0x0,    // Set to 0 always
                   &cmd);

    dStatus = Ether_WaitDma();
    if (dStatus != TRUE)
    {
        eStatus = SPI_CheckErrors();
        OALMSGS(OAL_ERROR, (L"RCR error = 0x%X\r\n",eStatus));
        return eStatus;
    }
    // Now read the data from from the Ethernet chip
    //
    Ether_StartDma(TRUE,   // bRead=TRUE -> Read
                   TRUE,   // bCmd=TRUE->Always set to TRUE
                   1,      // # of DMA bytes to transfer
                   0x0,    // Set to 0 always
                   0x0,    // Set to 0 always
                   &Value);

    dStatus = Ether_WaitDma();
    
    if (dStatus != TRUE)
    {
        eStatus = SPI_CheckErrors();
OALMSGS(1, (L"DMA ERROR: %x\n\r", eStatus));
        return eStatus;
    }
    memcpy(&Value,pVirtDataDMABuffer,1);

//OALMSGS(1, (L"ETH: Enc28j60_RCR return value : 0x%x  \n\r", Value));

    return Value;
}
//------------------------------------------------------------------------------
//
// Function: Enc28j60_WCR
// 
//------------------------------------------------------------------------------
static BOOL  Enc28j60_WCR(UINT16 addr, UINT8 val)
{

    BOOL dStatus, eStatus;
    UINT32 cmd = 0;

    //
    // Strip out the bank select bits (bits 8-15) in "addr" that is passed into this
    // function. All we want is the actual command.

    addr = (addr & ~BANKSEL_MASK);

    // Form the 1st byte  
    // OP_WCR = 2 = 0x2h = 010b
    //
    //   Byte 1    Byte 0
    // dddd.dddd.010a.aaaaa
    cmd = ((OP_WCR << 5) | addr | ((UINT32)val<<8));

    // OALMSGS(1, (L"ETH: Enc28j60_WCR Called: 0x%x +\n\r",cmd,val));

    Ether_StartDma(FALSE,   // bRead=FALSE -> Write
                   TRUE,    // bCmd=TRUE->Always set to TRUE
                   2,       // # of DMA bytes to transfer
                   0x0,     // Set to 0 always
                   0x0,     // Set to 0 always
                   &cmd);

    dStatus = Ether_WaitDma();
    if (dStatus != TRUE)
    {
        eStatus = SPI_CheckErrors();
        return eStatus;
    }
    return TRUE;
}

    // Read buffer Memory command
    // The Read Buffer Memory (RBM) command allows the
    // host controller to read bytes from the integrated 8-Kbyte
    // transmit and receive buffer memory.
    // If the AUTOINC bit in the ECON2 register is set, the
    // ERDPT pointer will automatically increment to point to
    // the next address after the last bit of each byte is read.
    // The next address will normally be the current address
    // incremented by one. However, if the last byte in the
    // receive buffer is read (ERDPT = ERXND), the ERDPT
    // pointer will change to the beginning of the receive
    // buffer (ERXST). This allows the host controller to read
    // packets from the receive buffer in a continuous stream
    // without keeping track of when a wraparound is needed.
    // If AUTOINC is set when address 1FFFh is read and
    // ERXND does not point to this address, the read pointer
    // will increment and wrap around to 0000h.
    // The RBM command is started by pulling the CS pin low.
    // The RBM opcode is then sent to the ENC28J60,
    // followed by the 5-bit constant 1Ah. After the RBM command
    // and constant are sent, the data stored in the
    // memory pointed to by ERDPT will be shifted out MSb
    // first on the SO pin. If the host controller continues to
    // provide clocks on the SCK pin, without raising CS, the
    // byte pointed to by ERDPT will again be shifted out MSb
    // first on the SO pin. In this manner, with AUTOINC
    // enabled, it is possible to continuously read sequential
    // bytes from the buffer memory without any extra SPI
    // command overhead. The RBM command is terminated
    // by raising the CS pin.
static BOOL Enc28j60_RBM(UINT32 *pBuffer, UINT16 NumBytesToRead)
{
    BOOL dStatus, eStatus = FALSE;
    UINT32 cmd = 0;

    OALMSGS(0, (L"ETH: Enc28j60_RBM Called: 0x%x   0x%x \r\n",pBuffer,NumBytesToRead));

    cmd = ((OP_RBM << 5) | 0x1A);  // 0x1A is part of the RBM command definition

    // Send the command out..
    Ether_StartDma(FALSE,  // bRead=FALSE -> Write
                   TRUE,   // bCmd=TRUE->Always set to TRUE
                   1,      // # of DMA bytes to transfer
                   0x0,  // Set to 0 always
                   0x0,  // Set to 0 always
                   &cmd);

    dStatus = Ether_WaitDma();

    if (dStatus != TRUE)
    {
        eStatus = SPI_CheckErrors();
    }

    WBM_Flag = FALSE;
    //
    // Now read the data from from the Ethernet chip
    //
    memset(pBuffer, 0x0, NumBytesToRead);

    Ether_StartDma(TRUE,  // bRead=TRUE -> Read
                   TRUE,   // bCmd=TRUE->Always set to TRUE
                   NumBytesToRead,  // # of DMA bytes to transfer
                   0x0,  // Set to 0 always
                   0x0,  // Set to 0 always
                   pBuffer);
    dStatus = Ether_WaitDma();
    
    if (dStatus != TRUE)
    {
        eStatus = SPI_CheckErrors();
        OALMSGS(OAL_ERROR, (L"ERROR:Enc28j60_RBM = 0x%X+\n\r",eStatus));
        return eStatus;
    }

    memcpy(pBuffer,pVirtDataDMABuffer,NumBytesToRead);

    return TRUE;
}

// BIT FIELD SET COMMAND
// The Bit Field Set (BFS) command is used to set up to
// 8 bits in any of the ETH Control registers. Note that this
// command cannot be used on the MAC registers, MII
// registers, PHY registers or buffer memory. The BFS command
// uses the provided data byte to perform a bit-wise
// OR operation on the addressed register contents.
// The BFS command is started by pulling the CS pin low.
// The BFS opcode is then sent, followed by a 5-bit
// address (A4 through A0). The 5-bit address identifies

static BOOL Enc28j60_BFS(UINT16 addr, UINT8 val)
{
    BOOL dStatus, eStatus = FALSE;
    UINT32 cmd = 0;
    //
    // Strip out the bank select bits (bits 8-15) in "addr" that is passed into this
    // function. All we want is the actual command.
    addr = (addr & ~BANKSEL_MASK);

    //
    //   Byte 1    Byte 0
    // dddd.dddd.010a.aaaaa
    cmd = ((OP_BFS << 5) | addr | ((UINT32)val<<8));

    //OALMSGS(1, (L"ETH: Enc28j60_BFS Called: 0x%x  +\n\r",cmd));

    Ether_StartDma(FALSE,   // bRead=FALSE -> Write
                   TRUE,    // bCmd=TRUE->Always set to TRUE
                   2,       // # of DMA bytes to transfer
                   0x0,     // Set to 0 always
                   0x0,     // Set to 0 always
                   &cmd);

    dStatus = Ether_WaitDma();
    if (dStatus != TRUE)
    {
        eStatus = SPI_CheckErrors();
        return eStatus;
    }
    return TRUE;
}
static BOOL Enc28j60_BFC(UINT16 addr, UINT8 val)
{
    BOOL dStatus, eStatus;
    UINT32 cmd = 0;
    
    //OALMSGS(1, (L"ETH: Enc28j60_BFC Called: +\n\r"));
    //
    // Strip out the bank select bits (bits 8-15) in "addr" that is passed into this
    // function. All we want is the actual command.
    addr = (addr & ~BANKSEL_MASK);

    //
    //   Byte 1    Byte 0
    // dddd.dddd.010a.aaaaa
    cmd = ((OP_BFC << 5) | addr | ((UINT32)val<<8));

    Ether_StartDma(FALSE,   // bRead=FALSE -> Write
                   TRUE,    // bCmd=TRUE->Always set to TRUE
                   2,       // # of DMA bytes to transfer
                   0x0,     // Set to 0 always
                   0x0,     // Set to 0 always
                   &cmd);

    dStatus = Ether_WaitDma();
    if (dStatus != TRUE)
    {
        eStatus = SPI_CheckErrors();
        return eStatus; 
    }
    return TRUE;
}

// SYSTEM COMMAND
// The System Command (SC) allows the host controller
// to issue a System Soft Reset command. Unlike other
// SPI commands, the SC is only a single-byte command
// and does not operate on any register.

static BOOL Enc28j60_SC(void)
{
    BOOL dStatus = FALSE; 
    UINT32 cmd = 0;

    //
    //   Byte 1    Byte 0
    // dddd.dddd.010a.aaaaa
    cmd = ((OP_SC << 5) | 0x1F);

    Ether_StartDma(FALSE,   // bRead=FALSE -> Write
                   TRUE,    // bCmd=TRUE->Always set to TRUE
                   1,       // # of DMA bytes to transfer
                   0x0,     // Set to 0 always
                   0x0,     // Set to 0 always
                   &cmd);

    dStatus = Ether_WaitDma();
    if (dStatus != TRUE)
    {
        OALMSGS(OAL_ERROR, (L"ERROR:Enc28j60_SC failed\r\n"));
        return dStatus;
    }
    return TRUE;
}

//      WRITE BUFFER MEMORY COMMAND
// The Write Buffer Memory (WBM) command allows the
// host controller to write bytes to the integrated 8-Kbyte
// transmit and receive buffer memory.
// If the AUTOINC bit in the ECON2 register is set, after
// the last bit of each byte is written, the EWRPT pointer
// will automatically be incremented to point to the next
// sequential address (current address + 1). If address
// 1FFFh is written with AUTOINC set, the write pointer
// will increment to 0000h.
// The WBM command is started by lowering the CS pin.
// The WBM opcode should then be sent to the 
// ENC28J60, followed by the 5-bit constant 1Ah. After
// the WBM command and constant are sent, the data to
// be stored in the memory pointed to by EWRPT should
// be shifted out MSb first to the ENC28J60. After 8 data
// bits are received, the write pointer will automatically
// increment if AUTOINC is set. The host controller can
// continue to provide clocks on the SCK pin and send
// data on the SI pin, without raising CS, to keep writing to
// the memory. In this manner, with AUTOINC enabled, it
// is possible to continuously write sequential bytes to the
// buffer memory without any extra SPI command overhead.
// The WBM command is terminated by bringing up the
// CS pin. Refer to Figure 4-6 for a detailed illustration of
// the write sequence.

static BOOL Enc28j60_WBM(UINT32 *pBuffer, UINT16 NumBytesToWrite)
{
    BOOL dStatus =FALSE;
    BOOL eStatus = FALSE;
    UINT32 cmd = 0;

    //OALMSGS(1, (L"ETH: Enc28j60_WBM Called: 0x%x 0x%x \r\n",pBuffer,NumBytesToWrite));

    if (NumBytesToWrite > TX_BUFFER_SIZE)
    {
        OALMSGS(OAL_ERROR, (L"WBM:  NumBytesToWrite (%d) > TX_BUFFER_SIZE(%d)\r\n",
                              NumBytesToWrite , TX_BUFFER_SIZE));
        return dStatus;
    }

    cmd = ((OP_WBM << 5) | 0x1A);  // 0x1A is part of the WBM command definition

    // Send the command out..
    Ether_StartDma(FALSE,           // bRead=FALSE -> Write
                   TRUE,    // bCmd=TRUE->Always set to TRUE
                   1,       // # of DMA bytes to transfer
                   0x0,     // Set to 0 always
                   0x0,     // Set to 0 always
                   &cmd);

    dStatus = Ether_WaitDma();
    
    if (dStatus != TRUE)
    {
        eStatus = SPI_CheckErrors();
    }
    Ether_StartDma(FALSE,  // bRead=FALSE -> Write
                   TRUE,   // bCmd=TRUE->Always set to TRUE
                   NumBytesToWrite,  // # of DMA bytes to transfer
                   0x0,  // Set to 0 always
                   0x0,  // Set to 0 always
                   pBuffer);

    dStatus = Ether_WaitDma();
    
    if (dStatus != TRUE)
    {
        eStatus = SPI_CheckErrors();
    }
    WBM_Flag = FALSE;
    return eStatus;
}

//
// Helper functions
//
static BOOL SetRegisterBank(UINT8 u8BankSel)
{
    /*OALMSGS(1, (L"SetRegisterBank ++ \r\n"));*/
    switch(u8BankSel)
    {
    case BANK0:
        Enc28j60_BFC(ECON1, (BM_ENC28J60_ECON1_BSEL0));
        Enc28j60_BFC(ECON1, (BM_ENC28J60_ECON1_BSEL1));
        break;
    case BANK1:
        Enc28j60_BFC(ECON1, (BM_ENC28J60_ECON1_BSEL1));
        Enc28j60_BFS(ECON1, (BM_ENC28J60_ECON1_BSEL0));
        break;
    case BANK2:
        Enc28j60_BFC(ECON1, (BM_ENC28J60_ECON1_BSEL0));
        Enc28j60_BFS(ECON1, (BM_ENC28J60_ECON1_BSEL1));
        break;
    case BANK3:
        Enc28j60_BFS(ECON1, (BM_ENC28J60_ECON1_BSEL0));
        Enc28j60_BFS(ECON1, (BM_ENC28J60_ECON1_BSEL1));
        break;
    default:
        OALMSGS(OAL_ETHER&&OAL_FUNC, (L"SetRegisterBank error. Using default\r\n"));
        Enc28j60_BFC(ECON1, (BM_ENC28J60_ECON1_BSEL0 | BM_ENC28J60_ECON1_BSEL1));
    }
    return TRUE;
}

// 3.3 PHY Registers
// The PHY registers provide configuration and control of
// the PHY module, as well as status information about its
// operation. All PHY registers are 16 bits in width. There
// are a total of 32 PHY addresses; however, only 9 locations
// are implemented. Writes to unimplemented
// locations are ignored and any attempts to read these
// locations will return ?? All reserved locations should be
// written as ?? their contents should be ignored when
// read.
// Unlike the ETH, MAC and MII control registers, or the
// buffer memory, the PHY registers are not directly
// accessible through the SPI control interface. Instead,
// access is accomplished through a special set of MAC
// control registers that implement a Media Independent
// Interface for Management (MIIM). These control registers
// are referred to as the MII registers. The registers
// that control access to the PHY registers are shown in
// Register 3-3 and Register 3-4.


//
// Command to Read from PHY register on the ENC28J60 ethernet chip
//

// 3.3.1 READING PHY REGISTERS
// When a PHY register is read, the entire 16 bits are
// obtained.
// To read from a PHY register:
// 1. Write the address of the PHY register to read
// from into the MIREGADR register.
// 2. Set the MICMD.MIIRD bit. The read operation
// begins and the MISTAT.BUSY bit is set.
// 3. Wait 10.24 ?s. Poll the MISTAT.BUSY bit to be
// certain that the operation is complete. While
// busy, the host controller should not start any
// MIISCAN operations or write to the MIWRH
// register.
// When the MAC has obtained the register
// contents, the BUSY bit will clear itself.
// 4. Clear the MICMD.MIIRD bit.
// 5. Read the desired data from the MIRDL and
// MIRDH registers. The order that these bytes are
// accessed is unimportant.

static UINT32 Enc28j60_PHY_Rd(UINT16 addr)
{
    UINT32 val = 0;
    UINT32 retry = 0;
    UINT8 temp = 0;
    UINT32 temp1 = 0;
    UINT32 temp2 = 0;
    BOOL bTimeOut = FALSE;

    if (addr>>8 != PHYS_REG_ID)
    {
        return FALSE;  
    }

    // Select Bank2
    SetRegisterBank(BANK2);

    temp = (UINT8)(addr & 0x00FF);
    // 1. Write the address of the PHY register to read
    // from into the MIREGADR register.
    Enc28j60_WCR(MIREGADR, temp);

    temp1 = Enc28j60_RCR(MICMD);
    temp1 &= ~BM_ENC28J60_MICMD_MIISCAN;  // Clear MIISCAN
    Enc28j60_WCR(MICMD, (UINT8)temp1);

    // 2. Set the MICMD.MIIRD bit. The read operation
    // begins and the MISTAT.BUSY bit is set.
    // Note: Have to do a read-write modify since BFS command can't be used
    // on MII registers.  Note: Bank2 is already set above.
    //temp1 = Enc28j60_RCR(MICMD);
    temp1 = Enc28j60_RCR(MICMD);
    temp1 |= BM_ENC28J60_MICMD_MIIRD;
    Enc28j60_WCR(MICMD, (UINT8)temp1);


    // 3. Wait 10.24 ?s. Poll the MISTAT.BUSY bit to be
    // certain that the operation is complete. While
    // busy, the host controller should not start any
    // MIISCAN operations or write to the MIWRH
    // register.
    // When the MAC has obtained the register
    // contents, the BUSY bit will clear itself.

    retry = 0x0;
    SetRegisterBank(BANK3);
    
    OALStall(11);
    while( (Enc28j60_RCR(MISTAT) & BM_ENC28J60_MISTAT_BUSY) )
    {
         OALStall(11); // Wait another ~10.24us
        if (retry++  > 5)  // We don't want to get stuck in the loop!!
        {
            bTimeOut = TRUE;
            break;
        }
    }

    //4. Clear the MICMD.MIIRD bit.
    SetRegisterBank(BANK2);
    temp1 = Enc28j60_RCR(MICMD);
    temp1 &= ~BM_ENC28J60_MICMD_MIIRD;
    Enc28j60_WCR(MICMD, (UINT8)temp1);

    // 5. Read the desired data from the MIRDL and
    // MIRDH registers. The order that these bytes are
    // accessed is unimportant.

    //
    // Note: Had to read MIRDL several times to get the value.
    // Could be issue with HW?
    //
    temp1 = Enc28j60_RCR(MIRDL);
    temp2 = Enc28j60_RCR(MIRDH);

    temp1 = Enc28j60_RCR(MIRDL);
    temp2 = Enc28j60_RCR(MIRDH);

    //temp1 = Enc28j60_RCR(MIRDL);
    //temp2 = Enc28j60_RCR(MIRDH);

    val = temp2;
    val |= ( temp1  << 8 );

    if (bTimeOut) val = 0;

    return val;
}


// When a PHY register is written to, the entire 16 bits is
// written at once; selective bit writes are not implemented.
// If it is necessary to reprogram only select bits
// in the register, the controller must first read the PHY
// register, modify the resulting data and then write the
// data back to the PHY register.


// To write to a PHY register:
// 1. Write the address of the PHY register to write to
// into the MIREGADR register.
// 2. Write the lower 8 bits of data to write into the
// MIWRL register.
// 3. Write the upper 8 bits of data to write into the
// MIWRH register. Writing to this register automatically
// begins the MII transaction, so it must
// be written to after MIWRL. The MISTAT.BUSY
// bit becomes set.
// The PHY register will be written after the MII operation
// completes, which takes 10.24 ?s. When the write
// operation has completed, the BUSY bit will clear itself.
// The host controller should not start any MIISCAN or
// MIIRD operations while busy.

static BOOL Enc28j60_PHY_Wr(UINT16 addr, UINT16 val)
{
    UINT32 retry = 0;
    UINT8 temp = 0;
    UINT32 temp1 = 0;
    BOOL Status = FALSE;

    if (addr>>8 != PHYS_REG_ID)
    {
        return Status; 
    }

    // Select Bank2
    SetRegisterBank(BANK2);

    //
    // Make sure MIIRD and MIISCAN are cleared
    temp1 = Enc28j60_RCR(MICMD);
    temp1 &= ~BM_ENC28J60_MICMD_MIIRD;
    temp1 &= ~BM_ENC28J60_MICMD_MIISCAN; 
    Enc28j60_WCR(MICMD, (UINT8)temp1);

    // 1. Write the address of the PHY register to write to into the MIREGADR register.
    temp = (UINT8)(addr & 0x00FF);
    Enc28j60_WCR(MIREGADR, temp);

    // 2. Write the lower 8 bits of data to write into the MIWRL register.
    Enc28j60_WCR(MIWRL, (UINT8)(val&0x00FF));

    // 3. Write the upper 8 bits of data to write into the MIWRH register. Writing to this register automatically
    // begins the MII transaction, so it must be written to after MIWRL. The MISTAT.BUSY bit becomes set.
    Enc28j60_WCR(MIWRH, (UINT8)(val>>8));


    // The PHY register will be written after the MII operation completes, which takes 10.24 ?s. When the write
    // operation has completed, the BUSY bit will clear itself. The host controller should not start any MIISCAN or
    // MIIRD operations while busy.
    retry = 0;
    SetRegisterBank(BANK3);
    Enc28j60_RCR(MISTAT);
    while( (Enc28j60_RCR(MISTAT) & BM_ENC28J60_MISTAT_BUSY) )
    {
        OALStall(10); 
        if (retry++  > 5)  // We don't want to get stuck in the loop!!
        {
            break;
        }
    }
    return Status;
}

//------------------------------------------------------------------------------
// Function: SPI_CheckErrors
//
// This function checks the  SSP Block for the errors. 
//
// Parameters:
//       None
//
// Return Value:
//        returns TRUE if the initialization process is successful, otherwise 
//        returns FALSE.
//
//------------------------------------------------------------------------------
static BOOL SPI_CheckErrors(void)
{
    SSP_CTRL1  sControl1;
    SSP_STATUS sStatus;

    // read in the value of the registers
    sControl1.U = INREG32(&REG->CTRL1);

    sStatus.U   = INREG32(&REG->STATUS);

    // Check CTRL1 and STATUS registers for errors
    if(sControl1.B.RESP_TIMEOUT_IRQ || sStatus.B.RESP_TIMEOUT)
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_CheckErrors: Response Timeout\r\n" ));
        return FALSE;
    }
    if(sStatus.B.RESP_CRC_ERR)
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_CheckErrors: Response CRC Error\r\n" ));
        return FALSE;
    }
    if(sControl1.B.RESP_ERR_IRQ || sStatus.B.RESP_ERR)
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_CheckErrors: Response Error\r\n" ));
        return FALSE;
    }
    if(sControl1.B.FIFO_OVERRUN_IRQ || sStatus.B.FIFO_OVRFLW)
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_CheckErrors: Receive Overflow\r\n" ));
        return FALSE;
    }
    if(sControl1.B.RECV_TIMEOUT_IRQ)
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_CheckErrors: Receive Timeout\r\n" ));
        return FALSE;
    }
    if(sControl1.B.DATA_CRC_IRQ || sStatus.B.DATA_CRC_ERR)
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_CheckErrors: Data CRC Error\r\n" ));
        return FALSE;
    }
    if(sControl1.B.DATA_TIMEOUT_IRQ)
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_CheckErrors: Data Timeout\r\n" ));
        return FALSE;
    }
    if(sStatus.B.TIMEOUT)
    {
        OALMSGS(OAL_ERROR, (L"ERROR: SPI_CheckErrors: Timeout\r\n" ));
        return FALSE;
    }
    // no error encountered
    return TRUE;
}
