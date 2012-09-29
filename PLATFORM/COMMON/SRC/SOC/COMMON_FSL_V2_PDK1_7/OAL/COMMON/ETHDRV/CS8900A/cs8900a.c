//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  cs8900a.c
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <ethdbg.h>
#include <oal.h>
#pragma warning(pop)

extern BOOL OALKitlSendEthSelfReset(void);
extern BOOL OALKitlInitEthIrq(void);
extern void OALKitlClearEthIrq(void);

//------------------------------------------------------------------------------

#define CS8900A_EISA_NUMBER         0x630e
#define RETRY_COUNT                 0x00100000

//------------------------------------------------------------------------------

typedef struct {
    UINT16 DATA0;
    UINT16 DATA1;
    UINT16 TXCMD;
    UINT16 TXLENGTH;
    UINT16 ISQ;
    UINT16 PAGEIX;
    UINT16 PAGE0;
    UINT16 PAGE1;
} CS8900A_REGS;

static CS8900A_REGS *g_pCS8900;
static UINT16 g_hash[4];
static UINT32 g_filter;

//
// allocate a temporary buffer to try and accept packets even when
// we are sending large chunks of data (therefore blocking recv)
//

#define CS8900A_RECV_MCU          1536
#define CS8900A_RECV_BUFFER_COUNT 32

typedef struct
{
    UINT32 len;
    UINT32 pdata[CS8900A_RECV_MCU / sizeof(UINT32)];
} CS8900A_RECV_PACKET;

static CS8900A_RECV_PACKET g_recvbuffer[CS8900A_RECV_BUFFER_COUNT];
static UINT32 g_recvfree = 0;
static UINT32 g_recvdata = 0;

//------------------------------------------------------------------------------

#define EISA_NUMBER                 0x0000
#define PRODUCT_ID_CODE             0x0002
#define IO_BASE_ADDRESS             0x0020
#define INTERRUPT_NUMBER            0x0022
#define DMA_CHANNEL_NUMBER          0x0024
#define DMA_START_OF_FRAME          0x0026
#define DMA_FRAME_COUNT             0x0028
#define RXDMA_BYTE_COUNT            0x002a
#define MEMORY_BASE_ADDR            0x002c
#define BOOT_PROM_BASE_ADDR         0x0030
#define BOOT_PROM_ADDR_MASK         0x0034
#define EEPROM_COMMAND              0x0040
#define EEPROM_DATA                 0x0042
#define RECEIVE_FRAME_BYTE_COUNT    0x0050

#define INT_SQ                      0x0120
#define RX_CFG                      0x0102
#define RX_EVENT                    0x0124
#define RX_CTL                      0x0104
#define TX_CFG                      0x0106
#define TX_EVENT                    0x0128
#define TX_CMD                      0x0108
#define BUF_CFG                     0x010A
#define BUF_EVENT                   0x012C
#define RX_MISS                     0x0130
#define TX_COL                      0x0132
#define LINE_CTL                    0x0112
#define LINE_ST                     0x0134
#define SELF_CTL                    0x0114
#define SELF_ST                     0x0136
#define BUS_CTL                     0x0116
#define BUS_ST                      0x0138
#define TEST_CTL                    0x0118
#define AUI_TIME_DOMAIN             0x013C
#define TX_CMD_REQUEST              0x0144
#define TX_CMD_LENGTH               0x0146

#define LOGICAL_ADDR_FILTER_BASE    0x0150
#define INDIVIDUAL_ADDRESS          0x0158

#define RX_STATUS                   0x0400
#define RX_LENGTH                   0x0402
#define RX_FRAME                    0x0404
#define TX_FRAME                    0x0a00

//------------------------------------------------------------------------------

#define ISQ_ID_MASK                 0x003F


#define SELF_CTL_RESET              (1 << 6)

#define SELF_ST_SIBUSY              (1 << 8)
#define SELF_ST_INITD               (1 << 7)

// EEPROM status bits
#ifdef CS8900_EEPROM_SUPPORT
#define SELF_ST_EEPROM_OK           (1 << 10)
#define SELF_ST_EEPROM_PRESENT      (1 << 9)
#endif

#define LINE_CTL_MOD_BACKOFF        (1 << 11)
#define LINE_CTL_AUI_ONLY           (1 << 8)
#define LINE_CTL_TX_ON              (1 << 7)
#define LINE_CTL_RX_ON              (1 << 6)

#define RX_CFG_RX_OK_IE             (1 << 8)
#define RX_CFG_SKIP_1               (1 << 6)

#define RX_CTL_BROADCAST            (1 << 11)
#define RX_CTL_INDIVIDUAL           (1 << 10)
#define RX_CTL_MULTICAST            (1 << 9)
#define RX_CTL_RX_OK                (1 << 8)
#define RX_CTL_PROMISCUOUS          (1 << 7)
#define RX_CTL_IAHASH               (1 << 6)

#define RX_EVENT_RX_OK              (1 << 8)
#define RX_EVENT_ID                 0x0004

#define TX_CMD_PAD_DIS              (1 << 13)
#define TX_CMD_INHIBIT_CRC          (1 << 12)
#define TX_CMD_ONECOLL              (1 << 9)
#define TX_CMD_FORCE                (1 << 8)
#define TX_CMD_START_5              (0 << 6)
#define TX_CMD_START_381            (1 << 6)
#define TX_CMD_START_1021           (2 << 6)
#define TX_CMD_START_ALL            (3 << 6)

#define BUS_ST_TX_RDY               (1 << 8)
#define BUS_ST_TX_BID_ERR           (1 << 7)

#define BUS_CTL_MEMORYE             (1 << 10)
#define BUS_CTL_ENABLE_IRQ          (1 << 15)

// supported EEPROM commands
#ifdef CS8900_EEPROM_SUPPORT
#define EEPROM_READ_REGISTER        0x0200
#define EEPROM_WRITE_REGISTER       0x0100
#define EEPROM_ERASE_REGISTER       0x0300
#define EEPROM_ERASE_ALL_REGISTER   0x00A0
#define EEPROM_ERASE_WRITE_ENABLE   0x00F0
#define EEPROM_ERASE_WRITE_DISABLE  0x0000
#define EEPROM_WRITE_ALL_REGISTER   0x0050
#endif // CS8900_EEPROM_SUPPORT

//------------------------------------------------------------------------------

static UINT16 IOReadPacketPage(UINT16 address);
static VOID IOWritePacketPage(UINT16 address, UINT16 data);
static UINT32 ComputeCRC(UINT8 *pBuffer, UINT32 length);

#ifdef CS8900_USE_MEM_MODE

// Default IO and memory base addresses
#define CS8900_DEFAULT_IO_BASE_ADDR         0x0300
#define CS8900_DEFAULT_MEM_BASE_ADDR        0x1000

static UINT32 g_dwCS8900MemBase;

#define MEM_READ_PACKET_PAGE(address)           ((UINT16)(*(volatile UINT16 *)(g_dwCS8900MemBase+(address))))
#define MEM_WRITE_PACKET_PAGE(address, data)    ((*(volatile UINT16 *)(g_dwCS8900MemBase+(address))) = (UINT16)(data))

// Redefine  functions for memory or IO mode
#define ReadPacketPage              MEM_READ_PACKET_PAGE
#define WritePacketPage             MEM_WRITE_PACKET_PAGE

#else
#define ReadPacketPage              IOReadPacketPage
#define WritePacketPage             IOWritePacketPage
#endif

//------------------------------------------------------------------------------
//
//  Function:  CS8900AInit
//
BOOL CS8900AInit(UINT8 *pAddress, UINT32 offset, UINT16 mac[3])
{
    BOOL rc = FALSE;
    UINT32 count;
    UINT32 i;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(offset);

    OALMSGS(OAL_ETHER&&OAL_FUNC, (
        L"+CS8900AInit(0x%08x, 0x%08x, %02x:%02x:%02x:%02x:%02x:%02x)\r\n",
        pAddress, offset, mac[0]&0xFF, mac[0]>>8, mac[1]&0xFF, mac[1]>>8,
        mac[2]&0xFF, mac[2]>>8
    ));

    // initialize ring buffer
    for(i = 0; i < CS8900A_RECV_BUFFER_COUNT; i++)
    {
        g_recvbuffer[i].len = 0;
    }

    g_recvfree = 0;
    g_recvdata = 0;

    // Save address
    g_pCS8900 = (CS8900A_REGS*)pAddress;

    // First check if there is chip
    if (IOReadPacketPage(EISA_NUMBER) != CS8900A_EISA_NUMBER) {
        OALMSGS(OAL_ERROR, (L"ERROR: CS8900AInit: Failed detect chip\r\n"));
        goto cleanUp;
    }

    OALMSGS(OAL_INFO, (L"INFO: CS8900AInit chip detected\r\n"));

    // Due to CPLD on some platforms, we cannot issue software reset
    // without confusing the CPLD.  Call down to platform code to
    // determine if we need to issue software self reset.

    if (OALKitlSendEthSelfReset())
    {
        IOWritePacketPage(SELF_CTL, SELF_CTL_RESET);
        count = RETRY_COUNT;
        while (count-- > 0) {
            if ((IOReadPacketPage(SELF_ST) & SELF_ST_INITD) != 0) break;
        }
        if (count == 0) {
            OALMSGS(OAL_ERROR, (L"ERROR: CS8900AInit: Failed to reset card\r\n"));
            goto cleanUp;
        }
        count = RETRY_COUNT;
        while (count-- > 0) {
            if ((IOReadPacketPage(SELF_ST) & SELF_ST_SIBUSY) != 0) break;
        }
        if (count == 0) {
            OALMSGS(OAL_ERROR, (L"ERROR: CS8900AInit: Failed to reset card\r\n"));
            goto cleanUp;
        }
    }

#ifdef CS8900_USE_MEM_MODE

    // Note: Mem base offset is hard coded as KITL allows no such
    // parameter to be passed in.
    g_dwCS8900MemBase = (UINT32)pAddress + CS8900_DEFAULT_MEM_BASE_ADDR -
                        CS8900_DEFAULT_IO_BASE_ADDR;

    // Enable memory mode
    WritePacketPage(MEMORY_BASE_ADDR, (UINT16)(CS8900_DEFAULT_MEM_BASE_ADDR));
    WritePacketPage(MEMORY_BASE_ADDR+2,
                    (UINT16)(CS8900_DEFAULT_MEM_BASE_ADDR >> 16));
    count = ReadPacketPage(BUS_CTL);
    WritePacketPage(BUS_CTL, count | BUS_CTL_MEMORYE);

    OALMSG(OAL_FUNC, (L"INFO: CS8900AInit: Memory mode memBase: 0x%08x\r\n",
                      g_dwCS8900MemBase));
    OALMSG(OAL_FUNC, (L"INFO: MEMORY_BASE_ADDR: 0x%04x\r\n",
                      IOReadPacketPage(MEMORY_BASE_ADDR)));
    OALMSG(OAL_FUNC, (L"INFO: MEMORY_BASE_ADDR+2: 0x%04x\r\n",
                      IOReadPacketPage(MEMORY_BASE_ADDR+2)));
    OALMSG(OAL_FUNC, (L"INFO: BUS_CTL: 0x%04x\r\n", IOReadPacketPage(BUS_CTL)));

#endif / CS8900_USE_MEM_MODE

    // Set MAC address
    WritePacketPage(INDIVIDUAL_ADDRESS + 0, mac[0]);
    WritePacketPage(INDIVIDUAL_ADDRESS + 2, mac[1]);
    WritePacketPage(INDIVIDUAL_ADDRESS + 4, mac[2]);

    // Enable receive
    WritePacketPage(RX_CTL, RX_CTL_RX_OK|RX_CTL_INDIVIDUAL|RX_CTL_BROADCAST);

    // Enable interrupt on receive
    WritePacketPage(RX_CFG, RX_CFG_RX_OK_IE);

    // Let assume that hardware is using INTRQ0
    WritePacketPage(INTERRUPT_NUMBER, 0);

    // Enable
    WritePacketPage(LINE_CTL, LINE_CTL_RX_ON|LINE_CTL_TX_ON);

    // Initialize IRQ line
    rc = OALKitlInitEthIrq();

cleanUp:
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-CS8900AInit(rc = %d)\r\n", rc));
    return rc;
}


VOID CS8900AQueueGetFrame()
{
    // queue a pending recive into a buffer
    UINT16 isq, length, status, count, data;
    PBYTE pData = NULL;

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (
        L"+CS8900AQueueGetFrame\r\n"));
    
    if(g_recvbuffer[g_recvfree].len != 0)
        return;

    length = 0;
    isq = INPORT16(&g_pCS8900->ISQ);
    if ((isq & ISQ_ID_MASK) == RX_EVENT_ID && (isq & RX_EVENT_RX_OK) != 0) {

        // Get RxStatus and length
        status = INPORT16(&g_pCS8900->DATA0);
        length = INPORT16(&g_pCS8900->DATA0);
        
        // check if free buffers are available
        if((length > 0) && (length <= CS8900A_RECV_MCU) )
        {

            // find the next free packet buffer
            pData = (PBYTE)g_recvbuffer[g_recvfree].pdata;

            // Read packet data
            count = length;
            while (count > 1) {
                data = INPORT16(&g_pCS8900->DATA0);
                *(UINT16*)pData = data;
                pData += sizeof(UINT16);
                count -= sizeof(UINT16);
            }

            // Read last one byte
            if (count > 0) {
                data = INPORT16(&g_pCS8900->DATA0);
                *pData = (UINT8)data;
            }
           
            g_recvbuffer[g_recvfree].len = length;
            g_recvfree++;
            if(g_recvfree >= CS8900A_RECV_BUFFER_COUNT)
                g_recvfree = 0;

        }
#if 0
        else
        {
            // If packet doesn't fit in buffer, skip it
            data = ReadPacketPage(RX_CFG);
            WritePacketPage(RX_CFG, data | RX_CFG_SKIP_1);   
        }
#endif

    }
    
    return;

}

//------------------------------------------------------------------------------
//
//  Function:  CS8900ASendFrame
//
UINT16 CS8900ASendFrame(UINT8 *pData, UINT32 length)
{
    BOOL rc = FALSE;
    UINT32 count;

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (
        L"+CS8900ASendFrame(0x%08x, %d)\r\n", pData, length
    ));

    // read any pending frames, so they don't get lost
    CS8900AQueueGetFrame();

    // Send Command
    OUTPORT16(&g_pCS8900->TXCMD, TX_CMD_START_ALL);
    OUTPORT16(&g_pCS8900->TXLENGTH, length);

    if (ReadPacketPage(BUS_ST) & BUS_ST_TX_BID_ERR)
    {
        OALMSGS(OAL_ERROR, (L"ERROR:  CS8900ASendFrame TX_BID_ERR\r\n"));
        goto cleanUp;
    }

    count = RETRY_COUNT;
    while (count-- > 0) {
        if ((ReadPacketPage(BUS_ST) & BUS_ST_TX_RDY) != 0) break;
    }
    if (count == 0) goto cleanUp;

    // Convert length from number of bytes to number of 16-bit words.
    length = (length + 1) >> 1;
    while (length-- > 0) {
        OUTPORT16(&g_pCS8900->DATA0, *(UINT16*)pData);
        pData += sizeof(UINT16);
    }

    rc = TRUE;

cleanUp:
    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (L"-CS8900ASendFrame(rc = %d)\r\n", !rc));
    return (UINT16)!rc;
}

//------------------------------------------------------------------------------
//
//  Function:  CS8900AGetFrame
//
UINT16 CS8900AGetFrame(UINT8 *pData, UINT16 *pLength)
{
    UINT16 length;

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (
        L"+CS8900AGetFrame(0x%08x, %d)\r\n", pData, *pLength
    ));

    // Clear the IRQ
    OALKitlClearEthIrq();

    // read a pending frame
    CS8900AQueueGetFrame();
    
    if(g_recvbuffer[g_recvdata].len == 0)
    {
        *pLength = 0;
        return 0;
    }

    length = (UINT16)g_recvbuffer[g_recvdata].len;

    // if we don't have enough buffer, skip the packet
    if ((length < (CS8900A_RECV_MCU / sizeof(UINT32))) && (length > *pLength))
            length = 0;

    if(length > 0)
    {
        memcpy(pData,g_recvbuffer[g_recvdata].pdata,length);
            }

    // release buffer
    g_recvbuffer[g_recvdata].len = 0;    
    g_recvdata++;
    if(g_recvdata >= CS8900A_RECV_BUFFER_COUNT)
        g_recvdata = 0;

    // Return packet size.
    *pLength = length;

    // read any pending frames, so they don't get lost
    CS8900AQueueGetFrame();

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (
        L"-CS8900AGetFrame(length = %d)\r\n", length
    ));

    return length;
}

//------------------------------------------------------------------------------
//
//  Function:  CS8900AEnableInts
//
VOID CS8900AEnableInts()
{
    UINT16 data;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+CS8900AEnableInts\r\n"));
    data = ReadPacketPage(BUS_CTL);
    WritePacketPage(BUS_CTL, data | BUS_CTL_ENABLE_IRQ);
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-CS8900AEnableInts\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  CS8900ADisableInts
//
VOID CS8900ADisableInts()
{
    UINT16 data;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+CS8900ADisableInts\r\n"));
    data = ReadPacketPage(BUS_CTL);
    WritePacketPage(BUS_CTL, data & ~BUS_CTL_ENABLE_IRQ);
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-CS8900ADisableInts\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  CS8900ACurrentPacketFilter
//
VOID CS8900ACurrentPacketFilter(UINT32 filter)
{
    UINT16 rxCtl;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (
        L"+CS8900ACurrentPacketFilter(0x%08x)\r\n", filter
    ));

    // Read current filter
    rxCtl = ReadPacketPage(RX_CTL);

    if ((filter & PACKET_TYPE_ALL_MULTICAST) != 0) {
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 0, 0xFFFF);
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 2, 0xFFFF);
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 4, 0xFFFF);
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 6, 0xFFFF);
    } else {
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 0, g_hash[0]);
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 2, g_hash[1]);
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 4, g_hash[2]);
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 6, g_hash[3]);
    }

    if (
        (filter & PACKET_TYPE_MULTICAST) != 0 ||
        (filter & PACKET_TYPE_ALL_MULTICAST) != 0
    ) {
        rxCtl |= RX_CTL_MULTICAST;
    } else {
        rxCtl &= ~RX_CTL_MULTICAST;
    }

    if ((filter & PACKET_TYPE_PROMISCUOUS) != 0) {
        rxCtl |= RX_CTL_PROMISCUOUS;
    } else {
        rxCtl &= ~RX_CTL_PROMISCUOUS;
    }

    WritePacketPage(RX_CTL, rxCtl);

    // Save actual filter
    g_filter = filter;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-CS8900ACurrentPacketFilter\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  CS8900AMulticastList
//
BOOL CS8900AMulticastList(UINT8 *pAddresses, UINT32 count)
{
    UINT32 i, j, crc, data, bit;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (
        L"+RTL8139MulticastList(0x%08x, %d)\r\n", pAddresses, count
    ));

    g_hash[0] = g_hash[1] = g_hash[2] = g_hash[3] = 0;
    for (i = 0; i < count; i++) {
        data = crc = ComputeCRC(pAddresses, 6);
        for (j = 0, bit = 0; j < 6; j++) {
            bit <<= 1;
            bit |= (data & 1);
            data >>= 1;
        }
        g_hash[bit >> 4] |= 1 << (bit & 0x0f);
        pAddresses += 6;
    }

    // But update only if all multicast mode isn't active
    if ((g_filter & PACKET_TYPE_ALL_MULTICAST) == 0) {
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 0, g_hash[0]);
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 2, g_hash[1]);
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 4, g_hash[2]);
        WritePacketPage(LOGICAL_ADDR_FILTER_BASE + 6, g_hash[3]);
    }

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-CS8900AMulticastList(rc = 1)\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------

static UINT16 IOReadPacketPage(UINT16 address)
{
    OUTREG16(&g_pCS8900->PAGEIX, address);
    return INREG16(&g_pCS8900->PAGE0);
}

//------------------------------------------------------------------------------

static VOID IOWritePacketPage(UINT16 address, UINT16 data)
{
    OUTREG16(&g_pCS8900->PAGEIX, address);
    OUTREG16(&g_pCS8900->PAGE0, data);
}

//------------------------------------------------------------------------------

static UINT32 ComputeCRC(UINT8 *pBuffer, UINT32 length)
{
    UINT32 crc, carry, i, j;
    UINT8 byte;

    crc = 0xffffffff;
    for (i = 0; i < length; i++) {
        byte = pBuffer[i];
        for (j = 0; j < 8; j++) {
            carry = ((crc & 0x80000000) ? 1 : 0) ^ (byte & 0x01);
            crc <<= 1;
            byte >>= 1;
            if (carry) crc = (crc ^ 0x04c11db6) | carry;
        }
    }
    return crc;
}

//------------------------------------------------------------------------------

#ifdef CS8900_EEPROM_SUPPORT
//-----------------------------------------------------------------------------
//
//  Function: CS8900EEPROMWait
//
//  The read and write EEPROM routines access the EEPROM that is attached to the
//  CS8900. There are extra locations within the EEPROM that can be used to
//  store other important information. The reset configuration accepted by the
//  CS8900 is given in the CS8900 spec.
//
//  Parameters:
//      timeOut
//          [in] - Specifies the number of seconds to wait before the EEPROM
//                 operation to complete.
//
//  Returns:
//      TRUE/FALSE for success.
//
//-----------------------------------------------------------------------------
static BOOL CS8900EEPROMWait(UINT32 timeOut)
{
    BOOL rc = TRUE;
    DWORD dwCurSec;

    // Start time for timeout
    dwCurSec = OEMEthGetSecs();

    // SIBUSY is set when EEPROM is currently busy
    while(IOReadPacketPage(SELF_ST) & SELF_ST_SIBUSY)
    {
        // Check if we have exceeded timeout
        if ((OEMEthGetSecs() - dwCurSec) > timeOut)
        {
            OALMSGS(OAL_ERROR, (
                TEXT("+CS8900EEPROMWait: CS8900 busy timeout.\r\n")));
            rc = FALSE;
            break;
        }

    }

    return rc;

}


//-----------------------------------------------------------------------------
//
// Function: CS8900ReadEEPROM
//
// The read and write EEPROM routines access the EEPROM that is attached to the
// CS8900. There are extra locations within the EEPROM that can be used to store
// other important information. The reset configuration accepted by the CS8900
// is given in the CS8900 spec. 
//
// Parameters:
//      EEPROMAddress
//      [in]    EEPROM word address
//
//      pwVal
//      [out]   storage for data read
//
// Returns:
//      TRUE/FALSE for success.
//
//-----------------------------------------------------------------------------
BOOL CS8900ReadEEPROM(UINT16 EEPROMAddress, UINT16 *pwVal)
{
    OALMSGS(OAL_ETHER&&OAL_FUNC, (
        L"+CS8900ReadEEPROM(0x%04x, 0x%08x)\r\n", EEPROMAddress, pwVal));

    if(pwVal == NULL || !(IOReadPacketPage(SELF_ST) & SELF_ST_EEPROM_PRESENT))
    {
        OALMSGS(OAL_ERROR, (TEXT("+CS8900ReadEEPROM: EEPROM not present or ")
                            TEXT("invalid param.\r\n")));
        return FALSE;
    }

    // Wait for pending EEPROM operations to complete
    if (!CS8900EEPROMWait(5)) return FALSE;

    // Issue read EEPROM command 
    IOWritePacketPage(EEPROM_COMMAND, (EEPROM_READ_REGISTER |
                                     (EEPROMAddress & 0xFF)));

    // Wait for pending EEPROM operations to complete
    if (!CS8900EEPROMWait(5)) return FALSE;

    *pwVal = IOReadPacketPage(EEPROM_DATA);

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (L"-CS8900ReadEEPROM(*pwVal = 0x%04x)\r\n",
                                     *pwVal));
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CS8900WriteEEPROM
//
// The read and write EEPROM routines access the EEPROM that is attached to the
// CS8900. There are extra locations within the EEPROM that can be used to store
// other important information. The reset configuration accepted by the CS8900
// is given in the CS8900 spec. 
//
// Parameters:
//      EEPROMAddress
//      [in]    EEPROM address
//
//      data
//      [in]    data to write
//
// Returns:
//      TRUE/FALSE for success.
//
//-----------------------------------------------------------------------------
BOOL CS8900WriteEEPROM(UINT16 EEPROMAddress, UINT16 data)
{

    OALMSGS(OAL_ETHER&&OAL_FUNC, (
        L"+CS8900WriteEEPROM(0x%04x, 0x%08x)\r\n", EEPROMAddress, data));

    if(!(IOReadPacketPage(SELF_ST) & SELF_ST_EEPROM_PRESENT))
    {
        OALMSGS(OAL_ERROR, (TEXT("+CS8900WriteEEPROM: EEPROM not present. ")
                            TEXT("SELF_ST(0x%x)\r\n"),
                            IOReadPacketPage(SELF_ST)));
        return FALSE;
    }

    // Wait for pending EEPROM operations to complete
    if (!CS8900EEPROMWait(5)) return FALSE;

    // Issue enable write enable EEPROM command & data
    IOWritePacketPage(EEPROM_COMMAND, EEPROM_ERASE_WRITE_ENABLE); 

    // Wait for pending EEPROM operations to complete
    if (!CS8900EEPROMWait(5)) return FALSE;

    IOWritePacketPage(EEPROM_DATA, data); 

    // Wait for pending EEPROM operations to complete
    if (!CS8900EEPROMWait(5)) return FALSE;

    // Issue write EEPROM command
    IOWritePacketPage(EEPROM_COMMAND, (EEPROM_WRITE_REGISTER |
                                      (EEPROMAddress & 0xFF)));

    // Wait for pending EEPROM operations to complete
    if (!CS8900EEPROMWait(5)) return FALSE;

    // Issue enable write disable EEPROM command & data
    IOWritePacketPage(EEPROM_COMMAND, EEPROM_ERASE_WRITE_DISABLE); 

    // Wait for pending EEPROM operations to complete
    if (!CS8900EEPROMWait(5)) return FALSE;

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (L"-CS8900WriteEEPROM\r\n"));
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CS8900EEPROMDetect
//
// Checks that the CS8900 and EEPROM chips are present. Note: MUST be called
// before any other EEPROM access functions.
//
// Parameters:
//      pAddress
//      [in]    CS8900 IO base address
//
// Returns:
//      FALSE for if no EEPROM or CS8900 device detected.
//
//-----------------------------------------------------------------------------
BOOL CS8900EEPROMDetect(BYTE *pAddress)
{
    OALMSGS(OAL_ETHER&&OAL_FUNC, (
        L"+CS8900EEPROMDetect(0x%08x)\r\n", pAddress));

    // Save address
    g_pCS8900 = (CS8900A_REGS*)pAddress;

    // Wait for the Ethernet controller to become ready
    if (!CS8900EEPROMWait(5)) 
    {
        OALMSG(OAL_INFO, (L"INFO: CS8900EEPROMDetect failed!\r\n"));
        return FALSE;
    }

    // First check if there is chip
    if(IOReadPacketPage(EISA_NUMBER) != CS8900A_EISA_NUMBER)
    {
        OALMSG(OAL_INFO, (L"INFO: Failed to detect CS8900 chip.\r\n"));
        return FALSE;
    }

    if(!(IOReadPacketPage(SELF_ST) & SELF_ST_EEPROM_PRESENT))
    {
        OALMSG(OAL_INFO, (L"INFO: No EEPROM detected with CS8900.\r\n"));
        return FALSE;
    }
    OALMSG(OAL_INFO, (L"INFO: EEPROM chip detected with CS8900. "
                      L"SELF_ST(0x%04x)\r\n", IOReadPacketPage(SELF_ST)));

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-CS8900EEPROMDetect\r\n"));
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CS8900EEPROMResetCfgStatus
//
// The function returns CS8900 reset configuration load status from EEPROM.
//
// Parameters:
//
//
// Returns:
//      TRUE if reset configuration is loaded successfully on chip reset.
//
//-----------------------------------------------------------------------------
BOOL CS8900EEPROMResetCfgValid(BOOL bReset)
{
    BOOL rc = FALSE;
    UINT32 count;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+CS8900EEPROMResetCfgValid\r\n"));

    // Reset the chip if neccessary
    if(bReset)
    {
        IOWritePacketPage(SELF_CTL, SELF_CTL_RESET);
        count = RETRY_COUNT;
        while (count-- > 0)
        {
            if ((IOReadPacketPage(SELF_ST) & SELF_ST_INITD) != 0)
                break;
        }
        if(count == 0)
        {
            OALMSGS(OAL_ERROR, (L"ERROR: CS8900EEPROMResetCfgValid: Failed to "
                                L"reset card\r\n"));
            goto cleanUp;
        }
        count = RETRY_COUNT;
        while(count-- > 0)
        {
            if ((IOReadPacketPage(SELF_ST) & SELF_ST_SIBUSY) != 0)
                break;
        }
        if(count == 0)
        {
            OALMSGS(OAL_ERROR, (L"ERROR: CS8900EEPROMResetCfgValid: Failed "
                                L"to reset card\r\n"));
            goto cleanUp;
        }
    }

    // Wait for pending EEPROM operations to complete
    if (!CS8900EEPROMWait(5)) return FALSE;

    // Check if EEPROM is present and status is OK
    if((IOReadPacketPage(SELF_ST) & (SELF_ST_EEPROM_PRESENT |
                                     SELF_ST_EEPROM_OK)) ==
       (SELF_ST_EEPROM_PRESENT | SELF_ST_EEPROM_OK))
    {
        OALMSGS(OAL_INFO, (L"CS8900EEPROMResetCfgValid: EEPROM present & "
                           L"OK.\r\n"));
        rc  = TRUE;
    }

cleanUp:
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-CS8900EEPROMResetCfgValid(rc = %d)\r\n",
                                  rc));
    return rc;
}

#endif    // CS8900_EEPROM_SUPPORT
