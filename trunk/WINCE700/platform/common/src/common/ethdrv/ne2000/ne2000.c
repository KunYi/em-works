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
#include <windows.h>
#include <halether.h>
#include <ceddk.h>
#include <oal_log.h>
#include "ne2000.h"
#include "ethdbg.h"


//------------------------------------------------------------------------------
//
//    List of multicast addresses currently set to h/w
//

#define     MAX_MULTICAST_LIST        8

static
UCHAR
g_pucMulticastList[MAX_MULTICAST_LIST][6];

static
DWORD
g_dwMulticastListInUse;

//------------------------------------------------------------------------------
//
//    Default fitler is Broadcast & directed.
//    Until this is changed via NE2000CurrentPacketFilter()
//
UCHAR
ucFilter = NIC_RECEIVE_BROADCAST;

DWORD
dwNdisFilter = 0x00;

//------------------------------------------------------------------------------

//#define DEBUG_SINGLE_CHAR   1

//
// Structure to track receive packets
//
#define RCV_Q_SIZE 32

typedef struct _RCV_ENTRY {
    BYTE re_state;
    BYTE re_next;       // next card receive buffer
    WORD re_len;        // packet length
    WORD re_ptr;        // location in card's buffer
} RCV_ENTRY, *PRCV_ENTRY;

//
// re_state values
//
#define RCV_FREE 0
#define RCV_USED 1

static RCV_ENTRY g_rcv_list[RCV_Q_SIZE];
static DWORD g_rcv_index;   // Next RCV_ENTRY to use
static DWORD g_rcv_next;    // Next RCV_ENTRY to indicate
static DWORD g_rcv_cnt;     // Number of packets to indicate

//
// Variables for remembering 8390 chip state
//
static BYTE command_reg;
static BYTE int_status;
static BYTE NextPage;
static BYTE overflowrestartflag;
static BYTE pstart;
static BYTE pstop;
static BYTE transmitting;
static DWORD srambase;
static DWORD sramsize;

static BYTE rcv_header[NIC_HEADER_LEN+16]; // NIC puts the current value of the Receive
#define rcv_status      rcv_header[0]     // Status Register in the frame header.
#define rcv_next_buff   rcv_header[1]   // Page number where next packet is/will be.
#define rcv_frame_len0  rcv_header[2]
#define rcv_frame_len1  rcv_header[3]   // This can be ignored since in high speed systems
                                        // it will be incorrect and can be calculated
                                        // using the page variables.
#define rcv_dest_addr0  rcv_header[4]
#define rcv_dest_addr1  rcv_header[5]

static BYTE new_current_page;

static BYTE tbb_start;
static WORD curr_xmit_len;

static BYTE erase_header[20];

// Mask to use for interrupts
static BYTE volatile *pbEthernetBase;
static BYTE ethernetaddr[6];

// Bitmask for configuration options passed to us from bootloader/kernel.
static DWORD dwConfigOptions;

static void InitRcvQueue(void);

#define WriteByte(Offset, Value)    WRITE_PORT_UCHAR((PUCHAR)(pbEthernetBase+Offset), Value)
#define ReadByte(Offset)            READ_PORT_UCHAR((PUCHAR)(pbEthernetBase+Offset))
#define WriteWord(Offset, Value)    WRITE_PORT_USHORT((PUSHORT)(pbEthernetBase+Offset), Value)
#define ReadWord(Offset)            READ_PORT_USHORT((PUSHORT)(pbEthernetBase+Offset))

#define WriteCmd(Cmd)   WriteByte(NIC_COMMAND, Cmd)
#define ReadCmd()       ReadByte(NIC_COMMAND)
#define UsePage0()      WriteCmd(NIC_Page0)
#define UsePage1()      WriteCmd(NIC_Page1)
#define UsePage1Stop()  WriteCmd(NIC_Page1Stop)

//#define NE2000_DUMP_FRAMES 1
#ifdef NE2000_DUMP_FRAMES
static void DumpEtherFrame( BYTE *pFrame, WORD cwFrameLength );
static void DumpNE2000Header(void);
#endif

BOOL
NE2000MulticastList(
    PUCHAR pucMulticastAddresses, 
    DWORD dwNoOfAddresses
    );

static void iodelay(int delay);
static void HWAccess(WORD addr, WORD count, PWORD mem, BYTE direction);
#define INSB(addr, count, mem) HWAccess(((WORD)(addr)), ((WORD)(count)), (WORD *)mem, NIC_RemoteDmaRd)
#define OUTSB(addr, count, mem) HWAccess(((WORD)(addr)), ((WORD)(count)), (WORD *)mem, NIC_RemoteDmaWr)


///////////////////////////////////////////////////////////////////////////
//
//   iodelay - Wait by polling the command register for "delay" iterations.
//
///////////////////////////////////////////////////////////////////////////
static void
iodelay(int delay)
{
    while (delay) {
        ReadCmd();
        delay--;
    }
}   // iodelay

///////////////////////////////////////////////////////////////////////////
//
//  insb/outsb - Read/Write bytes or words from/to adapter ram
//
//  In word mode, the caller must ensure that adapter ram access starts
//  on a word boundary.
//
///////////////////////////////////////////////////////////////////////////
static void
HWAccess(
    WORD addr,
    WORD count, 
    PWORD mem, 
    BYTE direction
    )
{
    WORD c;

    if (!count) {
        KITL_RETAILMSG(ZONE_ERROR, (
            "EDBG:NE2000:HWAccess - skipping 0 byte access!!!\r\n"
            ));
        return;     // Skip 0 length reads
    }
    
    WriteByte(NIC_INTR_STATUS, NIC_ISR_DMA_COMPLETE);

    // Set count, address, remote DMA address and direction,
    UsePage0();
    WriteByte(NIC_RMT_ADDR_LSB, (UCHAR)addr);
    WriteByte(NIC_RMT_ADDR_MSB, (UCHAR)(addr>>8));
    WriteByte(NIC_RMT_COUNT_LSB, (UCHAR)count);
    WriteByte(NIC_RMT_COUNT_MSB, (UCHAR)(count>>8));
    WriteCmd(direction);

    if (addr & 1) {     // Never start word IO on an odd adapter address
        KITL_RETAILMSG(ZONE_ERROR, (
            "EDBG:NE2000:HWAccess SKIPPING ODD ADAPTER ADDRESS ACCESS!!!\r\n"
            ));
        return;
    }

    c = count >> 1;    // word count.
    if (direction == NIC_RemoteDmaWr) {
        while (c) {
            WriteWord(NIC_RACK_NIC, *mem);
            mem++;
            c--;
        }
        if (count & 1) {    // trailing odd data byte will be on even adapter address
            WriteByte(NIC_RACK_NIC, *((PBYTE)mem));
        }
    } else {
        while (c) {
            *mem = ReadWord(NIC_RACK_NIC);
            mem++;
            c--;
        }
        if (count & 1) {
            *((PBYTE)mem) = ReadByte(NIC_RACK_NIC);
        }
    }

    c = 0;
    while ((++c) &&
      !(ReadByte(NIC_INTR_STATUS) & NIC_ISR_DMA_COMPLETE));
    return;
}   // HWAccess


/////////////////////////////////////////////////////////////////////////////
//
//  HWStartAdapter - initialize the data size (BYTE or WORD), set up the
//             receive configuration, and    unmask the receive and
//             transmit interrupts at the adapter.
//
/////////////////////////////////////////////////////////////////////////////
static void
HWStartAdapter(void)
{
    UsePage0();
    WriteByte(NIC_DATA_CONFIG, NIC_DCR_FIFO | NIC_DCR_NORMAL | NIC_DCR_WORD_XFER);
    WriteByte(NIC_XMIT_CONFIG, 0);  // Initialize transmit configuration for normal operation.
    
    //WriteByte(NIC_RCV_CONFIG, NIC_RECEIVE_BROADCAST);  // accept broadcast and directed packets.
    WriteByte(NIC_RCV_CONFIG, ucFilter); 

    WriteByte(NIC_INTR_STATUS, 0xff );                  // ack all interrupts
    WriteCmd(NIC_CMD_ABORT | NIC_CMD_START );           // Put NIC in START mode.
}   // HWStartAdapter


/////////////////////////////////////////////////////////////////////////////
//
//  HWStopAdapter - shut down the adapter and leave it in loopback mode
//              preparatory to a reset.
//
/////////////////////////////////////////////////////////////////////////////
static void
HWStopAdapter(void)
{
    WORD    closeTimeout;

    WriteCmd(NIC_CMD_PAGE0 | NIC_CMD_ABORT | NIC_CMD_STOP); // Stop NIC from receiving or transmitting.
    WriteByte(NIC_INTR_STATUS, 0 );                         // mask all adapter interrupts
    WriteByte(NIC_RMT_COUNT_LSB, 0 );                       // Clear the remote byte count registers.
    WriteByte(NIC_RMT_COUNT_MSB, 0 );

    // Poll the card ISR for the RST bit.
    closeTimeout = 0;
    while ((++closeTimeout) &&
      !(ReadByte(NIC_INTR_STATUS) & NIC_ISR_RESET_STATUS));
    WriteByte(NIC_XMIT_CONFIG, 2);                          // Place NIC in internal loopback mode.
    WriteCmd(NIC_CMD_PAGE0 | NIC_CMD_START );               // Issue a start command. The NIC is still in loopback mode.
}   // HWStopAdapter


static void
HWReset(void)
{
    HWStopAdapter();
    WriteByte(NIC_PAGE_START, pstart);
    WriteByte(NIC_BOUNDARY, (BYTE)(pstart+1));
    WriteByte(NIC_PAGE_STOP, pstop);
    UsePage1Stop();
    WriteByte(NIC_CURRENT, (BYTE)(pstart+1));
    WriteCmd(NIC_AbortDmaStop); // back to page0
    NextPage = pstart+1;
    HWStartAdapter();
}

// checkRam and ram_test are used at bind time.

#define RAMCHECK_SIZE    4    // RAM test pattern
static BYTE ramCheck[RAMCHECK_SIZE] = { 0xAA, 0x55, 0xCC, 0x66 };


//
//  HWCheckRam - test adapter RAM by writing a test pattern and reading it.
//
//  Inputs:
//    addr  - internal adapter RAM buffer address
//    count - number of bytes to move
//
static BOOL
HWCheckRam(DWORD addr, DWORD count)
{
    DWORD i,j;
    BYTE buff[RAMCHECK_SIZE];

    // write the test pattern in 4 byte chunks
    for (i = 0; i < count; i += RAMCHECK_SIZE) {
        OUTSB((addr+i), min(RAMCHECK_SIZE,count), ramCheck);
    }

    // read it back and compare against the checkRam pattern.
    for (i = 0; i < count; i += RAMCHECK_SIZE) {
        INSB((addr+i), min(RAMCHECK_SIZE,count), buff);
        for (j = 0; j < min(RAMCHECK_SIZE,count); j++) {
            if (buff[j] != ramCheck[j]) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

//
//  HWRamTest - figure out how much adapter RAM there is.
//
static BOOL
HWRamTest(void)
{
    DWORD ramEnd;
    BOOL bRet;

#define MAX_RAM_BASE 0x10000

    // find some RAM starting at 1K thru 64K. if it's greater then 64K, then
    // assume it isn't there.
    ramEnd = 0;
    HWStopAdapter();
    for (srambase = 1024; srambase < MAX_RAM_BASE; srambase += 1024) {
        if (HWCheckRam(srambase, 4)) {
            // found the starting point, now find out how much RAM there is.
            // assume it will increase in 1K chunks.
            for (ramEnd = srambase; !(ramEnd & 0xffff0000); ramEnd += 1024) {
                // RAM ends at the first location that fails the RAM test.
                if (!HWCheckRam(ramEnd, 4)) {
                    pstop = (BYTE)(ramEnd>>8);
                    break;
                }
            }
            break;
        }
    }

    // check for failure
    if ((srambase > MAX_RAM_BASE) || (srambase == ramEnd) || (ramEnd == 0)) {
        KITL_RETAILMSG(ZONE_ERROR, (
            "EDBG:NE2000:HWRamTest fail. srambase = 0x%X, ramEnd = 0x%X\r\n",
            srambase, ramEnd
            ));
        bRet = FALSE;
        goto hwrt_done;
    }

    // if ramEnd is >= 64K, then back off one 256 byte chunk to avoid the 16
    // bit wrap around.
    if (ramEnd & 0xffff0000) {
        pstop = 0xff;
    }

    //
    // Allocate 1 packet for transmit
    //
    tbb_start = (BYTE)(srambase>>8);
    pstart = tbb_start+6;   // Allocate 6 256 byte buffers for transmit

    // compute sramsize
    sramsize = ramEnd - srambase;

    // check the full RAM bank
    if (!HWCheckRam(srambase,sramsize)) {
        KITL_RETAILMSG(ZONE_ERROR, (
            "EDBG:NE2000:HWRamTest full RAM check failed.\r\n"
            ));
        bRet = FALSE;
        goto hwrt_done;
    }

    bRet = TRUE;

hwrt_done:
    HWStartAdapter();
    return bRet;
}   // HWRamTest


/*

// Obsolete, now replaced by HWSetMCRegs()

//
//  HWClearMCRegs - Clear the adapter's multicast address registers.
//
static void
HWClearMCRegs()
{
    DWORD j;
    UsePage1();
    j = NIC_MC_ADDR;

    // Now clear out all multicast address registers.
    while (j < NIC_MC_ADDR+8) {
        WriteByte(j, 0);
        j++;
    }
    UsePage0();
}  // HWClearMCRegs
*/

////////////////////////////////////////////////////////////////////////////////
//    HWSetMCRegs()
//
//    Description:
//
//        This function is used to set the multicast registers.
//
//    Arguments:
//
//        pucMulticastRegs :: If not null then use this instead of 0xFF or 0x00
//                               depending on bAllMulticast below.
//
//        bAllMulticast     :: TRUE == Set all to 0xff    :: FALSE = Set all to 0x00
//
//    Return value:
//
//
static void
HWSetMCRegs(PUCHAR    pucMulticastRegs, BOOL bAllMulticast)
{
    DWORD    j;
    UCHAR    ucData;
    
    UsePage1();    
    
    if (pucMulticastRegs)
    {
        KITL_RETAILMSG(TRUE, (
            "Ne2kDbg:: MulticastRegs set to : %x-%x-%x-%x-%x-%x-%x-%x\r\n",
            pucMulticastRegs[0],
            pucMulticastRegs[1],
            pucMulticastRegs[2],
            pucMulticastRegs[3],
            pucMulticastRegs[4],
            pucMulticastRegs[5],
            pucMulticastRegs[6],
            pucMulticastRegs[7]));

        for (j = 0 ; j < 8 ; j++)        
            WriteByte(NIC_MC_ADDR + j, pucMulticastRegs[j]);        
    }
    else
    {
        if (bAllMulticast)
            {
            KITL_RETAILMSG(TRUE, (
                "Ne2kDbg:: HWSetMCRegs():: Set all to 0xff\r\n"
                ));
            ucData = 0xFF;
            }
        else
            {
            KITL_RETAILMSG(TRUE, (
                "Ne2kDbg:: HWSetMCRegs():: Set all to 0x00\r\n"
                ));
            ucData = 0x00;
            }

        for (j = 0 ; j < 8 ; j++)
            WriteByte(NIC_MC_ADDR + j, ucData);    
    }

    UsePage0();

}    //    HWSetMCRegs()


//
//   HWInit - completely initialize the hardware and leave it in a
//          state that is ready to transmit and receive packets.
//
static BOOL
HWInit(void)
{
    int n;

    WriteCmd(NIC_AbortDmaStop); // Stop the NIC
    
    // Initialize Data Configuration register for auto remove.
    // This gets reset later on.
    WriteByte(NIC_DATA_CONFIG, NIC_DCR_FIFO|NIC_DCR_AUTO_INIT);

    // Bug workaround for SRAM memory protection.
    WriteByte(NIC_XMIT_START, 0xA0);
    WriteByte(NIC_XMIT_CONFIG, 0);
    WriteByte(NIC_RCV_CONFIG, NIC_RECEIVE_MONITOR_MODE);

    WriteByte(NIC_PAGE_START, 4);
    WriteByte(NIC_BOUNDARY,   4);
    WriteByte(NIC_PAGE_STOP, 0xff);
    WriteByte(NIC_XMIT_COUNT_LSB, 0x3c);
    WriteByte(NIC_XMIT_COUNT_MSB, 0);
    WriteByte(NIC_INTR_STATUS, 0xff);
    UsePage1Stop();
    WriteByte(NIC_CURRENT, 4);
    UsePage0();

    // If command register reflects the last command assume it is OK
    if (ReadCmd() != NIC_Page0) {
        KITL_RETAILMSG(ZONE_ERROR, (
            "EDBG:NE2000Init:HWInit failed to read cmd reg\r\n"
            ));
        goto hw_dead;
    }
    
    WriteByte(NIC_RMT_COUNT_LSB, 55); // initialization errata
    WriteCmd(NIC_RemoteDmaRd);

    HWStartAdapter();
    if (!HWRamTest()) { // see how much adapter RAM
        KITL_RETAILMSG(ZONE_ERROR, (
            "EDBG:NE2000Init:HWInit HWRamTest failed\r\n"
            ));
        goto hw_dead;
    }

    WriteCmd(NIC_AbortDmaStop);

    // Read the ethernet address
    // The NE2000 address PROM only decodes even addresses, so you
    // have to read WORDs and discard the high byte.
    WriteByte(NIC_RMT_COUNT_LSB, 12);
    WriteByte(NIC_RMT_COUNT_MSB, 0);
    WriteByte(NIC_RMT_ADDR_LSB, 0);
    WriteByte(NIC_RMT_ADDR_MSB, 0);
    WriteCmd(NIC_RemoteDmaRd);
    for (n = 0; n < 6; n++) {
        ethernetaddr[n] = (BYTE)ReadWord(NIC_RACK_NIC);
    }

    // copy node address to page1 physical address registers
    UsePage1Stop();
    for (n = 0; n < 6; n++) {
        WriteByte(NIC_PHYS_ADDR+n, ethernetaddr[n]);
    }
    
    //
    //    Init multicast registers to 0
    //

    memset(
        g_pucMulticastList,
        0x00,
        MAX_MULTICAST_LIST * 6);

    g_dwMulticastListInUse = 0x00;

    HWSetMCRegs(NULL, FALSE);                


    WriteCmd(NIC_AbortDmaStop); // back to page0

    // Initialize the page start, stop, and boundary registers based on the
    // results of ram_test().
    WriteByte(NIC_PAGE_START, pstart);
    WriteByte(NIC_BOUNDARY, (BYTE)(pstart+1));
    WriteByte(NIC_PAGE_STOP, pstop);
    UsePage1Stop();
    WriteByte(NIC_CURRENT, (BYTE)(pstart+1));
    WriteCmd(NIC_AbortDmaStop); // back to page0
    NextPage = pstart+1;
    tbb_start = (BYTE)(srambase >> 8);
    
    memset(erase_header, 0xff, sizeof(erase_header));
    return TRUE;

hw_dead:
    WriteCmd(NIC_AbortDmaStop); // stop NIC
    return FALSE;
} // HWinit


//------------------------------------------------------------------------------
//
// This is called to initialize the ethernet low level driver.  The base
// address of the ethernet hardware is passed into the routine.  The routine
// will return TRUE for a successful initialization.
//
BOOL
NE2000Init(
    BYTE *pbBaseAddress, 
    DWORD logicalLocation, 
    USHORT MacAddr[3]
    )
{
    BOOL bRet;
    int n;
    PBYTE pDst;
    PBYTE pSrc;

    UNREFERENCED_PARAMETER(logicalLocation);
    KITL_RETAILMSG(ZONE_KITL_ETHER, ("+EDBG:NE2000Init\r\n"));
    pbEthernetBase = pbBaseAddress;
    KITL_RETAILMSG(ZONE_KITL_ETHER, (
        "EDBG:NE2000Init using I/O range at 0x%X\r\n", pbBaseAddress
        ));
    InitRcvQueue();
    bRet = HWInit();
    if ( NULL != MacAddr ) {
        pDst = (PBYTE)MacAddr;
        pSrc = ethernetaddr;
        for (n = 6; n; n--) {
            *pDst++ = *pSrc++;
        }
    }
    KITL_RETAILMSG(ZONE_KITL_ETHER, ("-EDBG:NE2000Init\r\n"));
    return bRet;
}   // NE2000Init

//------------------------------------------------------------------------------

// Interrupts left disabled at init, call this function to turn them on
void
NE2000EnableInts(
    )
{
    //KITLOutputDebugString("NE2000EnableInts\n");
    UsePage0();
    WriteByte(
        NIC_INTR_MASK, IMR_RCV|IMR_XMIT|IMR_RCV_ERR|IMR_XMIT_ERR|IMR_OVERFLOW
        );
}

//------------------------------------------------------------------------------

void
NE2000DisableInts(
    )
{
    //KITLOutputDebugString("NE2000DisableInts\n");
    UsePage0();
    WriteByte(NIC_INTR_MASK, 0);
}

//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//    NE2000CurrentPacketFilter()
//    
//    Description:
//
//        This function is called to set the h/w filter.
//        We support:
//            PROMISCUOUS, ALL MULTICAST, BROADCAST, DIRECTED.
//
//    Arguments:
//
//        dwFilter::    The filter mode 
//
//    Return value:
//
//        TRUE if successful, FALSE otherwise.
//
void
Ne2000CurrentPacketFilter(DWORD    dwFilter)
{    
    DWORD    dwReceiveConfig = 0x00;


    if (dwFilter & (PACKET_TYPE_ALL_MULTICAST | PACKET_TYPE_PROMISCUOUS))
    {
        //
        //    Need all multicast..
        //
        HWSetMCRegs(NULL, TRUE);
    }
    else        
    {
        //
        //    No longer need "all multicast".        
        //    When we do support MULTICAST LIST then we should copy the list
        //    to the h/w here..
        //

        UCHAR    pucMulticastList[MAX_MULTICAST_LIST][6];

        memcpy(
            pucMulticastList,
            &(g_pucMulticastList[0][0]),
            g_dwMulticastListInUse * 6);
        
        NE2000MulticastList(
            &(pucMulticastList[0][0]),
            g_dwMulticastListInUse);
    }
    

    //
    //    The multicast bit in the RCR should be on if multicast/all multicast packets 
    //    (or is promiscuous) is needed.
    //
    if(dwFilter & 
        (PACKET_TYPE_ALL_MULTICAST | PACKET_TYPE_MULTICAST | PACKET_TYPE_PROMISCUOUS))    
    {
        dwReceiveConfig |= RCR_MULTICAST;
    }
    else
    {
        dwReceiveConfig &= ~RCR_MULTICAST;
    }

    //
    //    The promiscuous physical bit in the RCR should be on if ANY
    //    protocol wants to be promiscuous.
    //
    if (dwFilter & PACKET_TYPE_PROMISCUOUS)
    {
        dwReceiveConfig |= RCR_ALL_PHYS;
    }
    else
    {
        dwReceiveConfig &= ~RCR_ALL_PHYS;
    }

    //
    // The broadcast bit in the RCR should be on if ANY protocol wants
    // broadcast packets (or is promiscuous).
    //
    if
    (
        dwFilter & (PACKET_TYPE_BROADCAST |
                                 PACKET_TYPE_PROMISCUOUS)
    )
    {
        dwReceiveConfig |= RCR_BROADCAST;
    }
    else
    {
        dwReceiveConfig &= ~RCR_BROADCAST;
    }


    //
    //    EDBG always receive directed and broadcast as a minimum.
    //
    
    dwReceiveConfig |= RCR_BROADCAST;    
    
    ucFilter     = (BYTE)dwReceiveConfig;
    dwNdisFilter = dwFilter;

    KITL_RETAILMSG(ZONE_KITL_ETHER, (
        "NE2KDBG:: Set Ndis filter [0x%x] -> [0x%x]\r\n",
        dwFilter, dwReceiveConfig
        ));

    UsePage0();    
    WriteByte(NIC_RCV_CONFIG, (BYTE)dwReceiveConfig);

}    //    Ne2000CurrentPacketFilter()





////////////////////////////////////////////////////////////////////////////////
//    ComputeCrc()
//    
//    Description:
//
//        Runs the AUTODIN II CRC algorithm on buffer Buffer of length Length.
//
//    Arguments:
//
//        Buffer - the input buffer
//        Length - the length of Buffer
//
//    Return value:
//
//        The 32-bit CRC value.
//
//    Note:
//
//        This function is adopted from netcard\ne2000 miniport driver.
//        
ULONG
ComputeCrc(
    IN PUCHAR Buffer,
    IN UINT Length)
{
    ULONG Crc, Carry;
    UINT i, j;
    UCHAR CurByte;

    Crc = 0xffffffff;

    for (i = 0; i < Length; i++) {

        CurByte = Buffer[i];

        for (j = 0; j < 8; j++) {

            Carry = ((Crc & 0x80000000) ? 1 : 0) ^ (CurByte & 0x01);

            Crc <<= 1;

            CurByte >>= 1;

            if (Carry) {

                Crc = (Crc ^ 0x04c11db6) | Carry;

            }

        }

    }

    return Crc;

}    //    ComputeCrc()



////////////////////////////////////////////////////////////////////////////////
//    GetMulticastBit()
//    
//    Description:
//
//        For a given multicast address, returns the byte and bit in the card 
//        multicast registers that it hashes to. Calls CardComputeCrc() to 
//        determine the CRC value.
//
//    Arguments:
//
//        Address - the address
//        Byte    - the byte that it hashes to
//        Value   - will have a 1 in the relevant bit
//
//    Return value:
//
//        None.
//
//    Note:
//
//        This function is adopted from netcard\ne2000 miniport driver.
//
VOID
GetMulticastBit(
    IN UCHAR Address[6],
    OUT UCHAR * Byte,
    OUT UCHAR * Value
    )
{
    ULONG Crc;
    UINT BitNumber;

    //
    // First compute the CRC.
    //

    Crc = ComputeCrc(Address, 6);


    //
    // The bit number is now in the 6 most significant bits of CRC.
    //

    BitNumber = (UINT)((Crc >> 26) & 0x3f);

    *Byte = (UCHAR)(BitNumber / 8);
    *Value = (UCHAR)((UCHAR)1 << (BitNumber % 8));

}    //    GetMulticastBit()



////////////////////////////////////////////////////////////////////////////////
//    NE2000MulticastList()
//    
//    Description:
//
//        This function is used to insert multicast addresses to the h/w.
//
//    Arguments:
//
//        pucMulticastAddresses    :: The list of multicast addressses.
//        dwNoOfAddresses            :: The number of addresses in the list.
//
//    Return value:
//
//        TRUE if successful, FALSE otherwise.
//

BOOL
NE2000MulticastList(PUCHAR pucMulticastAddresses, DWORD dwNoOfAddresses)
{
    UCHAR    NicMulticastRegs[8];        // contents of card multicast registers
    UINT    i;
    UCHAR    Byte, Bit;

#if 0
    KITLOutputDebugString(
        "NE2KDBG:: NE2000MulticastList() set [%d] multicast list addrs.\r\n",
        dwNoOfAddresses);
#endif


    //
    //    Make sure it's below max list we can handle..
    //

    if (dwNoOfAddresses > MAX_MULTICAST_LIST)
    {
        KITL_RETAILMSG(ZONE_KITL_ETHER, (
            "Ne2kDbg:: NE2000MulticastList exceeds max list..\r\n"
            ));
        return FALSE;
    }


    //
    //    Remember these addresses as we may need them again when filter
    //    is switched from All Multicast to multicast.
    //
    
    memset(
        g_pucMulticastList,
        0x00,
        MAX_MULTICAST_LIST * 6);
    
    memcpy(
        g_pucMulticastList,
        pucMulticastAddresses,
        dwNoOfAddresses * 6);

    g_dwMulticastListInUse = dwNoOfAddresses;

#if 0

    for (i = 0 ; i < dwNoOfAddresses ; i++)
    {
        KITLOutputDebugString(
            "NE2KDBG:: Multicast[%d]  = %x-%x-%x-%x-%x-%x\r\n",
            i,
            pucMulticastAddresses[6*i + 0],
            pucMulticastAddresses[6*i + 1],
            pucMulticastAddresses[6*i + 2],
            pucMulticastAddresses[6*i + 3],
            pucMulticastAddresses[6*i + 4],
            pucMulticastAddresses[6*i + 5]);
    }

#endif


    //
    //    If we are already in Multicast or Promiscuous mode, then no
    //    need to set this..
    //

    if (dwNdisFilter & (PACKET_TYPE_PROMISCUOUS |
                        PACKET_TYPE_ALL_MULTICAST))
    {
        return TRUE;
    }


    //
    //    First turn all bits off.
    //

    for (i=0; i<8; i++) 
        NicMulticastRegs[i] = 0;
        

    //
    // Now turn on the bit for each address in the multicast list.
    //
    
    for (i=0 ; i<dwNoOfAddresses; i++) 
    {        
        GetMulticastBit(&pucMulticastAddresses[6*i], &Byte, &Bit);
        NicMulticastRegs[Byte] |= Bit;
    }

#if 0

    KITLOutputDebugString(
        "NE2KDBG:: MulticastRegs = %x-%x-%x-%x-%x-%x-%x-%x\r\n",        
        NicMulticastRegs[0],
        NicMulticastRegs[1],
        NicMulticastRegs[2],
        NicMulticastRegs[3],
        NicMulticastRegs[4],
        NicMulticastRegs[5],
        NicMulticastRegs[6],
        NicMulticastRegs[7]);
    
#endif


    //
    //    Finally, burn that in the h/w..
    //

    HWSetMCRegs(NicMulticastRegs, FALSE);
    
    return TRUE;

}    //    NE2000MulticastList()


//
//  HandleBufferOverflow - Perform the first 7 steps of the 11 step process to
//    recover from receive buffer overflow.  Step 8 is to remove packets from the
//  buffer which HandleReceive does. A flag is set to tell HandleReceive to do
//  steps 9-11.
//
//  Returns TRUE if current transmit needs resend.
// 
static BOOL
HandleBufferOverflow(void)
{
    BOOL bResend = FALSE;

#ifdef DEBUG_SINGLE_CHAR
    KITLOutputDebugString("V"); // overflow
#endif

#define STOP_LOOP_CNT 6000
                                        // 1. remember TXP bit (it's already in _command_reg)       
    WriteCmd(NIC_AbortDmaStop);         // 2. take the controller offline
    iodelay(STOP_LOOP_CNT);             // 3. Wait 1.6ms for NIC to stop    
    WriteByte(NIC_RMT_COUNT_LSB, 0);    // 4. clear the remote byte count
    WriteByte(NIC_RMT_COUNT_MSB, 0);

    // The transmit may have completed while waiting for the NIC to stop
    int_status |= ReadByte(NIC_INTR_STATUS);
    if (command_reg & NIC_CMD_XMIT) {
        if (int_status & (NIC_ISR_PACKET_XMITD|NIC_ISR_XMIT_ERR)) {
            bResend = TRUE;
        }
    }

    WriteByte(NIC_INTR_STATUS, int_status); // ack ints   
    WriteByte(NIC_XMIT_CONFIG, NIC_TCR_LOOPBACK1);  // 6. place NIC in loopback mode.    
    UsePage0();                                     // 7. restart in loopback mode
    overflowrestartflag = 1;
    return bResend;
}   // HandleBufferOverflow


//
//   alignframe - sometimes the frame gets shifted right by a WORD.
//         Fix up the frame header and return with BX=packet offset
//         Increment DI to track alignments done/undone.
//
//   input: called == 0 -> first time in, shift frame right
//          called == 1 -> second time in, shift frame left
//
//   side effects: packet header (at rcv_status) is shifted around.
//          first case:     123456 -> 561234
//          second case:    123456 -> 345612
//
static void
alignframe(int called)
{
    BYTE tmp0;
    BYTE tmp1;

    if (called) {
        // shift the receive header left by a WORD
        tmp0 = rcv_dest_addr0;
        tmp1 = rcv_dest_addr1;
        rcv_dest_addr0 = rcv_frame_len0;
        rcv_dest_addr1 = rcv_frame_len1;
        rcv_frame_len0 = rcv_status;
        rcv_frame_len1 = rcv_next_buff;
        rcv_status     = tmp0;
        rcv_next_buff  = tmp1;
    } else {
        // first time in for this packet, shift it right.
        tmp0 = rcv_status;
        tmp1 = rcv_next_buff;
        rcv_status     = rcv_frame_len0; 
        rcv_next_buff  = rcv_frame_len1;
        rcv_frame_len0 = rcv_dest_addr0;
        rcv_frame_len1 = rcv_dest_addr1;
        rcv_dest_addr0 = tmp0;
        rcv_dest_addr1 = tmp1;
    }    
}   // alignframe


//
//   check_rcv_header - check status and length fields and possibly call alignframe.
//
//  Return 0, 2, or 0xffff for failure
//
static WORD
check_rcv_header(void)
{
    int alignframecalls = 0;
    BYTE prev_len0;
    BYTE prev_len1;
    WORD len;

crh_restart:
    if (alignframecalls >= 2) {
        return 0xffff;
    }
    if (rcv_status & 0x5E) {
        // receive status bits look wrong
        //KITLOutputDebugString("check_rcv_header: rcv_status = %02X\r\n", rcv_status);
        alignframe(alignframecalls++);
    }

    // If it's a multicast or broadcast frame and we haven't realigned the
    // header, then see if the first byte of the destination address has its
    // lsb set. If it doesn't then it's in error.      
    if (rcv_status & NIC_RCV_STAT_MC_BC) {
        if (!alignframecalls) {
            if (!(rcv_dest_addr0 & 1)) {
                //KITLOutputDebugString("check_rcv_header: rcv_dest_addr0 = %02X\r\n", rcv_dest_addr0);
                alignframe(alignframecalls++);
                goto crh_restart;
            }
        }
    }

    prev_len0 = rcv_frame_len0;
    prev_len1 = rcv_frame_len1;
    if (rcv_frame_len0 == rcv_frame_len1) {
        // If the length bytes are the same, the low byte may have been copied into the high byte.
        // We will compute the packet length based on rcv_next_buff, NextPage and the low byte
        // Calculate high byte of pkt length:  (in high speed systems, the low
        // byte of the packet length gets copied into rcv_frame_len0 and rcv_frame_len1)
        rcv_frame_len1 = rcv_next_buff - NextPage - 1;
        if (rcv_frame_len1 & 0x80) {
             rcv_frame_len1 = (pstop - NextPage) + (rcv_next_buff - pstart) - 1;
        }
        if (rcv_frame_len0 > 0xFC) {
            rcv_frame_len1++;
            rcv_frame_len0 = 0;
        }
        len = (WORD)(rcv_frame_len0 + (rcv_frame_len1<<8));
        //KITLOutputDebugString("check_rcv_header: packet len = %d\r\n", len);
    }

    len = (WORD)(rcv_frame_len0 + (rcv_frame_len1<<8));
//    if ((len > MAX_FRAME_SIZE + NIC_HEADER_LEN) || (len < MIN_FRAME_SIZE + NIC_HEADER_LEN)) {
    if (len > MAX_FRAME_SIZE + NIC_HEADER_LEN) {
        rcv_frame_len0 = prev_len0;
        rcv_frame_len1 = prev_len1;
        //KITLOutputDebugString("check_rcv_header: packet len = %d.\r\n", len);
        alignframe(alignframecalls++);
        goto crh_restart;
    }

    if ((rcv_next_buff < pstart) || (rcv_next_buff > pstop)) {
        alignframe(alignframecalls++);
        goto crh_restart;
    }
    return (alignframecalls == 1) ? 2 : 0;
}   // check_rcv_header


static void
HWSetBoundary(BYTE next)
{
    if (next < pstart) {
        //KITLOutputDebugString("HWSetBoundary: next = %02X\r\n", next);
        next = pstart;
    }
    if (next > pstop) {
        //KITLOutputDebugString("HWSetBoundary: next = %02X\r\n", next);
        next = pstart;
    }
    WriteByte(NIC_BOUNDARY, (BYTE)((next == pstart) ? pstop-1 : next-1));
}

static void InitRcvQueue(void)
{
    DWORD n;

    for (n = 0; n < RCV_Q_SIZE; n++) {
        g_rcv_list[n].re_state = RCV_FREE;       
    }
    g_rcv_index = 0;
    g_rcv_next = 0;
    g_rcv_cnt = 0;
}

static BOOL
QueueRcv(
    WORD packetlen,
    WORD packetptr,
    BYTE nextbuff
    )
{
    PRCV_ENTRY prcv = &(g_rcv_list[g_rcv_index]);

    if (prcv->re_state != RCV_FREE) {
        //KITLOutputDebugString("QueueRcv: No free receives!\r\n");
        return FALSE;
    }

    prcv->re_state = RCV_USED;
    prcv->re_next = nextbuff;
    prcv->re_len = packetlen;
    prcv->re_ptr = packetptr;
    //KITLOutputDebugString("QueueRcv: Queued %d at 0x%x, len %d\r\n", g_rcv_index, packetptr, packetlen);
    g_rcv_cnt++;
    g_rcv_index++;
    if (g_rcv_index == RCV_Q_SIZE) {
        g_rcv_index = 0;
    }
    return TRUE;

} // QueueRcv


//
//   HandleReceive - Check that there is a packet in the receive buffer and
//            check its length and status.  Then queue the receive if
//            the packet is OK.
//
static void HandleReceive(void)
{
    BYTE rsr;   // receive status register
    WORD packetlen;
    WORD packetptr;
    int loopcnt;
    WORD adjust;

#define MAX_RCV_CNT 32

    loopcnt = MAX_RCV_CNT;

    //KITLOutputDebugString("+HandleReceive\r\n");

    // Loop looking for more receive packets
hr_more_packets:
    if (!loopcnt) {
        //KITLOutputDebugString("HandleReceive Skipped %d packets\r\n", MAX_RCV_CNT);
        goto hr_exit;
    }
    loopcnt--;

    // If current_page register is not equal to NextPage
    // then we have a packet in the ring buffer.
    UsePage1();
    new_current_page = ReadByte(NIC_CURRENT);
    UsePage0();

    if ((int_status & NIC_ISR_RCV_ERR) && (!overflowrestartflag)) {
        rsr = ReadByte(NIC_RCV_STATUS);
        if (!(rsr & RSR_PACKET_OK)) {
            //KITLOutputDebugString("HandleReceive NIC_ISR_RCV_ERR (rsr = %02X)\r\n", rsr);
            INSB((NextPage<<8), sizeof(rcv_header), rcv_header);
#ifdef NE2000_DUMP_FRAMES
            DumpNE2000Header();
#endif            
            /*
            if (rsr & RSR_CRC_ERROR) {
                KITLOutputDebugString("CRC Error\r\n");
            }
            if (rsr & RSR_MULTICAST) {
                KITLOutputDebugString("MULTICAST\r\n");
            }
            if (rsr & RSR_DISABLED) {
                KITLOutputDebugString("Receiver is disabled\r\n");
            }
            if (rsr & RSR_DEFERRING) {
                KITLOutputDebugString("Receiver is deferring\r\n");
            }
            */
#ifdef NE2000_DUMP_FRAMES
            DumpEtherFrame(&(rcv_dest_addr0), 14);
#endif
            InitRcvQueue();
            HWSetBoundary(new_current_page);
            NextPage = new_current_page;
            goto hr_exit;
        }
    }
    if (new_current_page == NextPage) {
        //KITLOutputDebugString("HandleReceive No new packets\r\n");
        goto hr_exit;
    }

    // There is another packet in the receive buffer.
    // Look at the packet's header
    INSB((NextPage<<8), sizeof(rcv_header), rcv_header);
    if ((adjust = check_rcv_header()) == 0xffff) {
        //KITLOutputDebugString("HandleReceive check_rcv_header() failed!\r\n");
        // Advance receive buffer pointer on packet errors 
        NextPage = new_current_page;
        goto hr_exit;
    }

    packetlen = (WORD)(rcv_frame_len1<<8)+(WORD)rcv_frame_len0 - NIC_HEADER_LEN;
    rcv_next_buff = NextPage + rcv_frame_len1 + 1;
    if (rcv_next_buff > pstop) {
        rcv_next_buff -= pstop - pstart;
    }

    //
    // Filter out non-ARP broadcast packets.
    //
    if (rcv_status & NIC_RCV_STAT_MC_BC) {
        if ((dwConfigOptions & OPT_BROADCAST_FILTERING) &&
            ((rcv_header[16] != 8) || (rcv_header[17] != 6))) {
            NextPage = rcv_next_buff;
            //KITLOutputDebugString("HandleReceive Dropping non-ARP broadcast packet!\r\n");
#ifdef NE2000_DUMP_FRAMES
            DumpEtherFrame(&(rcv_dest_addr0), 14);
#endif
            goto hr_more_packets;
        }
    }

    packetptr = (NextPage << 8) + NIC_HEADER_LEN + adjust;
    if (QueueRcv(packetlen, packetptr, rcv_next_buff)) {
        NextPage = rcv_next_buff;
        goto hr_more_packets;
    }

hr_exit:    // Done looking for receive packets

    if (!g_rcv_cnt) {
        //
        // We have caught up with receive packets, advance the boundary ptr
        //
        HWSetBoundary(NextPage);
    }

    if (overflowrestartflag) {
        // Do steps 9 and 10 of overflow recovery        
        WriteByte(NIC_INTR_STATUS, NIC_ISR_OVERWRITE);  // 9. reset the OVW bit        
        WriteByte(NIC_XMIT_CONFIG, NIC_TCR_NORMAL);     // 10. Get out of loopback mode.
        overflowrestartflag = 0;    
    }
    //KITLOutputDebugString("-HandleReceive\r\n");
}   // HandleReceive

static void HandleTransmit(void)
{
    // check for transmit errors
    if ((int_status & NIC_ISR_XMIT_ERR) || (!(ReadByte(NIC_XMIT_STATUS) & NIC_TSR_SUCCESS))) {
        iodelay(15);
        WriteByte(NIC_XMIT_CONFIG, NIC_TCR_LOOPBACK1);  // place NIC in loopback mode.
        UsePage0();                                     // start in loopback mode.
        ReadCmd();
        WriteByte(NIC_XMIT_CONFIG, NIC_TCR_NORMAL);     // get out of loopback mode.
        UsePage0();                                     // start the NIC
        ReadCmd();
    }
    transmitting = 0;
}

void NE2000_ISR(void)
{

#define ISR_LOOP_MAX 32

    int loopcnt = ISR_LOOP_MAX;

    command_reg = ReadCmd();
    UsePage0();
    int_status = ReadByte(NIC_INTR_STATUS);

    PREFAST_SUPPRESS(6316, "Not fixing PREfast warning for legacy code.");
    while ((int_status & NIC_ISR_MOREWORK) != 0) {
        WriteByte(NIC_INTR_STATUS, int_status); // ack the interrupts    
        //
        // Respond to the interrupt status register bits.
        //
        if (int_status & NIC_ISR_OVERWRITE) {
            HandleBufferOverflow();
        }
    
        //if (int_status & (NIC_ISR_PACKET_RCVD|NIC_ISR_RCV_ERR)) {
            HandleReceive();
        //}

        if (int_status & (NIC_ISR_PACKET_XMITD|NIC_ISR_XMIT_ERR)) {
            HandleTransmit();
        }

        if (!-- loopcnt)
            break;
            
        command_reg = ReadCmd();
        int_status = ReadByte(NIC_INTR_STATUS);
    }
}   // NE2000_ISR


//
// This routine is polled to find out if a frame has been received.  If there are no frames
//  in the RX FIFO, the routine will return 0.  If there was a frame that was received correctly,
//  it will be stored in pwData, otherwise it will be discarded.  
//
UINT16
NE2000GetFrame(BYTE *pbData, UINT16 *pwLength )
{
    PRCV_ENTRY prcv;
    WORD len;

    //KITLOutputDebugString("+NE2000GetFrame\r\n");

    len = *pwLength;
    *pwLength = 0;
    if (!g_rcv_cnt) {
        NE2000_ISR();
    }
    if (g_rcv_cnt) {
        prcv = &(g_rcv_list[g_rcv_next]);
        if (prcv->re_state == RCV_USED) {
            //KITLOutputDebugString("NE2000GetFrame: reading %d (ptr 0x%x, len %d)\r\n",
            //    g_rcv_next, prcv->re_ptr, prcv->re_len);
            if (prcv->re_len < len) {
                len = prcv->re_len;
            }
            INSB(prcv->re_ptr, len, pbData);
            HWSetBoundary(prcv->re_next);
            *pwLength = len;
            prcv->re_state = RCV_FREE;
            g_rcv_cnt--;
            g_rcv_next++;
            if (g_rcv_next == RCV_Q_SIZE) {
                g_rcv_next = 0;
            }
    }
    } else {
#ifdef DEBUG_SINGLE_CHAR
        KITLOutputDebugString("Z"); // Receive buffer empty
#endif
    }

    //KITLOutputDebugString("-NE2000GetFrame *pwLength = %d\r\n", *pwLength);
    return *pwLength;
} // NE2000GetFrame()


//
// This routine should be called with a pointer to the ethernet frame data.  It is the caller's
//  responsibility to fill in all information including the destination and source addresses and
//  the frame type.  The length parameter gives the number of bytes in the ethernet frame.
// The routine will not return until the frame has been transmitted or an error has occured.  If
//  the frame transmits successfully, 0 is returned.  If an error occured, a message is sent to
//  the serial port and the routine returns non-zero.
//
UINT16
NE2000SendFrame( BYTE *pbData, DWORD dwLength )
{
    int nIters;

    //KITLOutputDebugString("+NE2000SendFrame\r\n");
    //
    // Loop here until the request is satisfied, or we timeout
    //
    for (nIters = 0; transmitting; nIters ++) {
        iodelay(20);
        NE2000_ISR();
        if (nIters > 1000) {
            KITL_RETAILMSG(ZONE_ERROR, (
                "NE2000SendFrame Timed out waiting for transmit buffers\n"
                ));
            //
            // 2 seconds ought to be enough to transmit a packet!!! Reset for transmit errors
            // 
            iodelay(15);
            WriteByte(NIC_XMIT_CONFIG, NIC_TCR_LOOPBACK1);  // place NIC in loopback mode.
            UsePage0();                                     // start in loopback mode.
            ReadCmd();
            WriteByte(NIC_XMIT_CONFIG, NIC_TCR_NORMAL);     // get out of loopback mode.
            UsePage0();                                     // start the NIC
            ReadCmd();

            HWReset();
            transmitting = 0;
        }
    }

    curr_xmit_len = (WORD)dwLength;
    //KITLOutputDebugString("NE2000SendFrame %d byte packet\r\n", curr_xmit_len);
#ifdef DEBUG_SINGLE_CHAR
    KITLOutputDebugString("S"); // Send packet
#endif
    OUTSB(srambase, curr_xmit_len, pbData);      
    WriteByte(NIC_XMIT_START, tbb_start);
    if (curr_xmit_len < 60) {
        curr_xmit_len = 60;
    }
    if (curr_xmit_len > 1536) {
        curr_xmit_len = 1536;
    }
    WriteByte(NIC_XMIT_COUNT_LSB, (BYTE)curr_xmit_len);
    WriteByte(NIC_XMIT_COUNT_MSB, (BYTE)(curr_xmit_len>>8));
    transmitting = 1;
    WriteCmd(NIC_Transmit);
    iodelay(80);
    NE2000_ISR();
    //KITLOutputDebugString("-NE2000SendFrame\r\n");
    return 0;
} // NE2000SendFrame()


#ifdef NE2000_DUMP_FRAMES
static void
DumpEtherFrame( BYTE *pFrame, WORD cwFrameLength )
{
    int i,j;

    KITL_RETAILMSG(TRUE, ("Frame Buffer Address: 0x%X\r\n", pFrame ));
    KITL_RETAILMSG(TRUE, (
        "To: %x:%x:%x:%x:%x:%x  From: %x:%x:%x:%x:%x:%x  Type: 0x%x  Length: %u\r\n",
        pFrame[0], pFrame[1], pFrame[2], pFrame[3], pFrame[4], pFrame[5],
        pFrame[6], pFrame[7], pFrame[8], pFrame[9], pFrame[10], pFrame[11],
        ntohs(*((UINT16 *)(pFrame + 12))), cwFrameLength ));
    for( i = 0; i < cwFrameLength / 16; i++ ) {
        for( j = 0; j < 16; j++ )
            KITL_RETAILMSG(TRUE, (" %x", pFrame[i*16 + j] ));
        KITL_RETAILMSG(TRUE, ("\r\n" ));
    }
    for( j = 0; j < cwFrameLength % 16; j++ )
        KITL_RETAILMSG(TRUE, ( " %x", pFrame[i*16 + j] ));
    KITL_RETAILMSG(TRUE, ("\r\n" ));
}

static void
DumpNE2000Header(void)
{
    KITL_RETAILMSG(TRUE, (
        "NE2000 Frame Header at %x: Status: %x, Next Buffer: %x, Len0: %x, Len1: %x\r\n",
        NextPage, rcv_status, rcv_next_buff, rcv_frame_len0, rcv_frame_len1);
}
#endif  // NE2000_DUMP_FRAMES

