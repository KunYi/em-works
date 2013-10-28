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
#include <wdm.h>
#include <oal_timer.h>
#include <oal_memory.h>

#define PRINTF(cond, printf_exp)        \
            ((void)((cond)?(KITLOutputDebugString printf_exp), 1 : 0))

#define NEXT(a, MAX)    \
            (++(a) >= MAX) ? (a = 0) : (0)

#define TO_REAL(Addr)   (OALVAtoPA((VOID *)(Addr)))
#define TO_VIRT(Addr)   (OALPAtoUA((UINT32)(Addr)))


//
//  Forward decl..
//

void
RTL8139HWSetMCRegs(PUCHAR   pucMulticastRegs, BOOL bAllMulticast);

BOOL
RTL8139MulticastList(PUCHAR pucMulticastAddresses, DWORD dwNoOfAddresses);


//
//  Multicast support ..
//

#define     MAX_MULTICAST_LIST      8
UCHAR       g_pucMulticastList[MAX_MULTICAST_LIST][6];
DWORD       g_dwMulticastListInUse;
DWORD       g_dwRCR;
DWORD       g_dwNdisFilter;


enum RTL8139Register
{
    RTL_IDR0_OFFSET                 = 0x00,
    RTL_IDR1_OFFSET                 = 0x01,
    RTL_IDR2_OFFSET                 = 0x02,
    RTL_IDR3_OFFSET                 = 0x03,
    RTL_IDR4_OFFSET                 = 0x04,
    RTL_IDR5_OFFSET                 = 0x05,

    RTL_MAR0_OFFSET                 = 0x08,
    RTL_MAR1_OFFSET                 = 0x09,
    RTL_MAR2_OFFSET                 = 0x0a,
    RTL_MAR3_OFFSET                 = 0x0b,
    RTL_MAR4_OFFSET                 = 0x0c,
    RTL_MAR5_OFFSET                 = 0x0d,
    RTL_MAR6_OFFSET                 = 0x0e,
    RTL_MAR7_OFFSET                 = 0x0f,

    RTL_TSD_BASE_OFFSET             = 0x10,
    RTL_TSAD_BASE_OFFSET            = 0x20,

    RTL_RBSTART_OFFSET              = 0x30,
    RTL_ERBCR_OFFSET                = 0x34,
    RTL_ERSR_OFFSET                 = 0x36,
    RTL_CR_OFFSET                   = 0x37,
    RTL_CAPR_OFFSET                 = 0x38,
    RTL_CBR_OFFSET                  = 0x3a,
    RTL_IMR_OFFSET                  = 0x3c,
    RTL_ISR_OFFSET                  = 0x3e,
    RTL_TCR_OFFSET                  = 0x40,
    RTL_RCR_OFFSET                  = 0x44,
    RTL_TCTR_OFFSET                 = 0x48,
    RTL_MPC_OFFSET                  = 0x4c,
    RTL_9346CR_OFFSET               = 0x50,
    RTL_CONFIG0_OFFSET              = 0x51,
    RTL_CONFIG1_OFFSET              = 0x52,

    RTL_TIMERINT_OFFSET             = 0x54,
    RTL_MSR_OFFSET                  = 0x58,
    RTL_CONFIG3_OFFSET              = 0x59,

    RTL_MULINT_OFFSET               = 0x5c,
    RTL_RERID_OFFSET                = 0x5e,
    RTL_TSAD_OFFSET                 = 0x60,
    RTL_BMCR_OFFSET                 = 0x62,
    RTL_BMSR_OFFSET                 = 0x64,
    RTL_ANAR_OFFSET                 = 0x66,
    RTL_ANLPAR_OFFSET               = 0x68,
    RTL_ANER_OFFSET                 = 0x6a,
    RTL_DIS_OFFSET                  = 0x6c,
    RTL_FCSC_OFFSET                 = 0x6e,
    RTL_NWAYTR_OFFSET               = 0x70,
    RTL_REC_OFFSET                  = 0x72,
    RTL_CSCR_OFFSET                 = 0x74,

    RTL_PHY1_PARAM_OFFSET           = 0x78,
    RTL_TW_PARAM_OFFSET             = 0x7c,
    RTL_PHY2_PARAM_OFFSET           = 0x80
};

enum RTL8139CommandRegister
{
    RTL8139_COMMAND_RESET           = 0x10,
    RTL8139_COMMAND_RX_ENABLE       = 0x08,
    RTL8139_COMMAND_TX_ENABLE       = 0x04,
    RTL8139_COMMAND_BUFFER_EMPTY    = 0x01
};

enum RTL8139BMCRRegister
{
    RTL_8139_PHY_REGISTERS_RESET    = 0x8000,
    RTL_100MBPS                     = 0x2000,
    RTL_AUTO_NEGOTIATE_ENABLE       = 0x1000,
    RTl_RESTART_AUTO                = 0x0200,
    RTL_FULL_DUPLEX                 = 0x0100
};


#define TX_DMA_BURST_SIZE       (0x007 << 8 )   //  2048 bytes. in TCR.
#define TX_CLEAR_ABORT          (0x01)          //  Clear abort in TCR.

#define NUM_TX_DESC             4
#define ONE_BUFFER_SIZE         1536


#define RX_DMA_BURST_SIZE       (0 << 8)
#define RX_ACCEPT_RUNT          (1 << 4)
#define RX_BROADCAST            (1 << 3)
#define RX_MULTICAST            (1 << 2)
#define RX_UNICAST              (1 << 1)
#define RX_PROMISCUOUS          (1 << 0)

#define INTR_TX_ERROR           (1 << 3)    // Tx error
#define INTR_TX_OK              (1 << 2)    // Tx OK
#define INTR_RX_ERROR           (1 << 1)    // Receive error
#define INTR_RX_OK              (1 << 0)    // Rx OK


#define RTL_AUTO_NEGOTIATION_COMPLETE   (1 << 5)


////////////////////////////////////////////////////////////////////////////////
//  The packet header in RX buffer.
//
#pragma warning(push)
#pragma warning(disable:4214 4201) 
typedef struct
{
    union
    {
        struct
        {
            USHORT  ROK : 1;    //  Receive OK.
            USHORT  FAE : 1;    //  Frame Alignment Error.
            USHORT  CRC : 1;    //  CRC Error.
            USHORT  LONG: 1;    //  Long packet.
            USHORT  RUNT: 1;    //  Runt packet.
            USHORT  ISE : 1;    //  Invalid Symbol Err.
            USHORT  resv: 7;
            USHORT  BAR : 1;    //  Broadcast packet.
            USHORT  PAM : 1;    //  Mathcing unicast address.
            USHORT  MAR : 1;    //  Multicast address.
        };

        USHORT  usPacketHeader;
    };

    USHORT  usPacketLength;

}   PACKET_HEADER, *PPACKET_HEADER;
#pragma warning(pop)


typedef struct RTL8139
{
    DWORD       dwBaseIO;
    DWORD       dwLogicalLocation;
    USHORT      usMacAddr[3];

    UCHAR       *pucTxBuffer;               //  Start of the TX buffers.
    DWORD       dwCurrentTxDescriptor;      //  TX desc for next send.
    DWORD       dwCurrentHwTxDescriptor;    //  TX desc to get back from hw.
    DWORD       dwFreeTxDescriptors;        //  Curr no of free TX desc.

    UCHAR       *pucRxBuffer;               //  Start of the RX buffer.
    UCHAR       *puLastRxAddress;           //  The last byte of RX buffer.
    DWORD       dwCurrentRxOffset;          //  Next RX packet read from here.



}   SRTL8139, *PSRTL8139;


////////////////////////////////////////////////////////////////////////////////
//  Useful constants..
//
#define RTL_IDR0            (PULONG) (sRTL8139.dwBaseIO + RTL_IDR0_OFFSET)
#define RTL_IDR1            (PULONG) (sRTL8139.dwBaseIO + RTL_IDR1_OFFSET)
#define RTL_IDR2            (PULONG) (sRTL8139.dwBaseIO + RTL_IDR2_OFFSET)
#define RTL_IDR3            (PULONG) (sRTL8139.dwBaseIO + RTL_IDR3_OFFSET)
#define RTL_IDR4            (PULONG) (sRTL8139.dwBaseIO + RTL_IDR4_OFFSET)
#define RTL_IDR5            (PULONG) (sRTL8139.dwBaseIO + RTL_IDR5_OFFSET)
#define RTL_MAR0            (PULONG) (sRTL8139.dwBaseIO + RTL_MAR0_OFFSET)
#define RTL_MAR1            (PULONG) (sRTL8139.dwBaseIO + RTL_MAR1_OFFSET)
#define RTL_MAR2            (PULONG) (sRTL8139.dwBaseIO + RTL_MAR2_OFFSET)
#define RTL_MAR3            (PULONG) (sRTL8139.dwBaseIO + RTL_MAR3_OFFSET)
#define RTL_MAR4            (PULONG) (sRTL8139.dwBaseIO + RTL_MAR4_OFFSET)
#define RTL_MAR5            (PULONG) (sRTL8139.dwBaseIO + RTL_MAR5_OFFSET)
#define RTL_MAR6            (PULONG) (sRTL8139.dwBaseIO + RTL_MAR6_OFFSET)
#define RTL_MAR7            (PULONG) (sRTL8139.dwBaseIO + RTL_MAR7_OFFSET)
#define RTL_RBSTART         (PULONG) (sRTL8139.dwBaseIO + RTL_RBSTART_OFFSET)
#define RTL_ERBCR           (PUSHORT)(sRTL8139.dwBaseIO + RTL_ERBCR_OFFSET)
#define RTL_ERSR            (PUCHAR) (sRTL8139.dwBaseIO + RTL_ERSR_OFFSET)
#define RTL_CR              (PUCHAR) (sRTL8139.dwBaseIO + RTL_CR_OFFSET)
#define RTL_CAPR            (PUSHORT)(sRTL8139.dwBaseIO + RTL_CAPR_OFFSET)
#define RTL_CBR             (PUSHORT)(sRTL8139.dwBaseIO + RTL_CBR_OFFSET)
#define RTL_IMR             (PUSHORT)(sRTL8139.dwBaseIO + RTL_IMR_OFFSET)
#define RTL_ISR             (PUSHORT)(sRTL8139.dwBaseIO + RTL_ISR_OFFSET)
#define RTL_TCR             (PULONG) (sRTL8139.dwBaseIO + RTL_TCR_OFFSET)
#define RTL_RCR             (PULONG) (sRTL8139.dwBaseIO + RTL_RCR_OFFSET)
#define RTL_TCTR            (PULONG) (sRTL8139.dwBaseIO + RTL_TCTR_OFFSET)
#define RTL_MPC             (PULONG) (sRTL8139.dwBaseIO + RTL_MPC_OFFSET)
#define RTL_9346CR          (PUCHAR) (sRTL8139.dwBaseIO + RTL_9346CR_OFFSET)
#define RTL_CONFIG0         (PUCHAR) (sRTL8139.dwBaseIO + RTL_CONFIG0_OFFSET)
#define RTL_CONFIG1         (PUCHAR) (sRTL8139.dwBaseIO + RTL_CONFIG1_OFFSET)
#define RTL_TIMERINT        (PULONG) (sRTL8139.dwBaseIO + RTL_TIMERINT_OFFSET)
#define RTL_MSR             (PUCHAR) (sRTL8139.dwBaseIO + RTL_MSR_OFFSET)
#define RTL_CONFIG3         (PUCHAR) (sRTL8139.dwBaseIO + RTL_CONFIG3_OFFSET)
#define RTL_MULINT          (PUSHORT)(sRTL8139.dwBaseIO + RTL_MULINT_OFFSET)
#define RTL_RERID           (PUSHORT)(sRTL8139.dwBaseIO + RTL_RERID_OFFSET)
#define RTL_TSAD            (PUSHORT)(sRTL8139.dwBaseIO + RTL_TSAD_OFFSET)
#define RTL_BMCR            (PUSHORT)(sRTL8139.dwBaseIO + RTL_BMCR_OFFSET)
#define RTL_BMSR            (PUSHORT)(sRTL8139.dwBaseIO + RTL_BMSR_OFFSET)
#define RTL_ANAR            (PUSHORT)(sRTL8139.dwBaseIO + RTL_ANAR_OFFSET)
#define RTL_ANLPAR          (PUSHORT)(sRTL8139.dwBaseIO + RTL_ANLPAR_OFFSET)
#define RTL_ANER            (PUSHORT)(sRTL8139.dwBaseIO + RTL_ANER_OFFSET)
#define RTL_DIS             (PUSHORT)(sRTL8139.dwBaseIO + RTL_DIS_OFFSET)
#define RTL_FCSC            (PUSHORT)(sRTL8139.dwBaseIO + RTL_FCSC_OFFSET)
#define RTL_NWAYTR          (PUSHORT)(sRTL8139.dwBaseIO + RTL_NWAYTR_OFFSET)
#define RTL_REC             (PUSHORT)(sRTL8139.dwBaseIO + RTL_REC_OFFSET)
#define RTL_CSCR            (PUSHORT)(sRTL8139.dwBaseIO + RTL_CSCR_OFFSET)
#define RTL_PHY1_PARM       (PULONG) (sRTL8139.dwBaseIO + RTL_PHY1_PARAM_OFFSET)
#define RTL_TW_PARM         (PULONG) (sRTL8139.dwBaseIO + RTL_TW_PARAM_OFFSET)
#define RTL_PHY2_PARM       (PUCHAR) (sRTL8139.dwBaseIO + RTL_PHY2_PARAM_OFFSET)

#define RTL_TSD_X(x)    (PULONG)(sRTL8139.dwBaseIO + RTL_TSD_BASE_OFFSET  + x * 4)
#define RTL_TSAD_X(x)   (PULONG)(sRTL8139.dwBaseIO + RTL_TSAD_BASE_OFFSET + x * 4)



////////////////////////////////////////////////////////////////////////////////
//  Global var..
//

//#define   BUFFER_START_ADDRESS    0x00200000  //  TX/RX DMA Buffer..
                                                //  *** WARNING ***
                                                //  THIS IS HARDCODED UNTIL EDBG
                                                //  IS CHANGED TO CONTROL THIS..
//#define   RX_BUFFER_LENGTH        (64 * 1024)
//#define   RX_BUFFER_LENGTH_BIT    (3 << 11)   //  64K + 16 bytes.

                                                //  SO WE NEED AROUND 70k OF
                                                //  BUFFER.. (64k + 6K for TX)

SRTL8139    sRTL8139;
DWORD       dwTxStartAddress  = 0x00;
DWORD       dwRxStartAddress  = 0x00;
DWORD       dwRxLength        = 0x00;
DWORD       dwRxLengthBit     = 0x00;



////////////////////////////////////////////////////////////////////////////////
//  Misc utility functions.
//
void DisplayHex (BYTE data)
{
    if (data < 0x10)
        PRINTF(1, ("0"));
    PRINTF (1, ("%x ", data));

}    // DisplayHex()


void DumpMemory (PBYTE pSource, DWORD dwLength)
{
    int        i = 0;

    PRINTF (1, ("+---- MEM DUMP (%d bytes)----+\r\n", dwLength));
    PRINTF (1, ("0x%x: ", pSource));
    while (dwLength--)
    {
        DisplayHex (*pSource++);
        if (++i == 16)
        {
            i = 0;
            PRINTF (1, ("\r\n0x%x: ", pSource));
        }
    }

    PRINTF (1, ("\r\n\r\n"));

}    // DumpMemory()


void
DumpAll8139Regs()
{
    PRINTF (1,
        ("+------------------------------------------------------------+\r\n"));

    PRINTF (1, ("IDR[0..5]  = 0x[%x - %x - %x - %x - %x - %x]\r\n",
        (UCHAR) READ_PORT_ULONG(RTL_IDR0),
        (UCHAR) READ_PORT_ULONG(RTL_IDR1),
        (UCHAR) READ_PORT_ULONG(RTL_IDR2),
        (UCHAR) READ_PORT_ULONG(RTL_IDR3),
        (UCHAR) READ_PORT_ULONG(RTL_IDR4),
        (UCHAR) READ_PORT_ULONG(RTL_IDR5)));

    PRINTF (1, ("MAR[0..7]  = 0x[%x - %x - %x - %x - %x - %x - %x - %x]\r\n",
        (UCHAR) READ_PORT_ULONG(RTL_MAR0),
        (UCHAR) READ_PORT_ULONG(RTL_MAR1),
        (UCHAR) READ_PORT_ULONG(RTL_MAR2),
        (UCHAR) READ_PORT_ULONG(RTL_MAR3),
        (UCHAR) READ_PORT_ULONG(RTL_MAR4),
        (UCHAR) READ_PORT_ULONG(RTL_MAR5),
        (UCHAR) READ_PORT_ULONG(RTL_MAR6),
        (UCHAR) READ_PORT_ULONG(RTL_MAR7)));

    PRINTF (1, ("TSD[0..3]  = 0x[%x - %x - %x - %x]\r\n",
        READ_PORT_ULONG(RTL_TSD_X(0)),
        READ_PORT_ULONG(RTL_TSD_X(1)),
        READ_PORT_ULONG(RTL_TSD_X(2)),
        READ_PORT_ULONG(RTL_TSD_X(3))));

    PRINTF (1, ("TSAD[0..3] = 0x[%x - %x - %x - %x]\r\n",
        READ_PORT_ULONG(RTL_TSAD_X(0)),
        READ_PORT_ULONG(RTL_TSAD_X(1)),
        READ_PORT_ULONG(RTL_TSAD_X(2)),
        READ_PORT_ULONG(RTL_TSAD_X(3))));

    PRINTF (1, ("RBSTART [0x%x] \t ERBCR   [0x%x] \t ERSR     [0x%x] \t CR       [0x%x]\r\n",
        READ_PORT_ULONG(RTL_RBSTART),
        READ_PORT_USHORT(RTL_ERBCR),
        READ_PORT_UCHAR(RTL_ERSR),
        READ_PORT_UCHAR(RTL_CR)));

    PRINTF (1, ("CAPR    [0x%x] \t CBR     [0x%x] \t IMR      [0x%x] \t ISR      [0x%x]\r\n",
        READ_PORT_USHORT(RTL_CAPR),
        READ_PORT_USHORT(RTL_CBR),
        READ_PORT_USHORT(RTL_IMR),
        READ_PORT_USHORT(RTL_ISR)));

    PRINTF (1, ("TCR     [0x%x] \t RCR     [0x%x] \t TCTR     [0x%x] \t MPC      [0x%x]\r\n",
        READ_PORT_ULONG(RTL_TCR),
        READ_PORT_ULONG(RTL_RCR),
        READ_PORT_ULONG(RTL_TCTR),
        READ_PORT_ULONG(RTL_MPC)));

    PRINTF (1, ("9346CR  [0x%x] \t CONFIG0 [0x%x] \t CONFIG1  [0x%x] \t TimerInt [0x%x]\r\n",
        READ_PORT_UCHAR(RTL_9346CR),
        READ_PORT_UCHAR(RTL_CONFIG0),
        READ_PORT_UCHAR(RTL_CONFIG1),
        READ_PORT_ULONG(RTL_TIMERINT)));

    PRINTF (1, ("MSR     [0x%x] \t CONFIG3 [0x%x] \t MULINT   [0x%x] \t RERID    [0x%x]\r\n",
        READ_PORT_UCHAR(RTL_MSR),
        READ_PORT_UCHAR(RTL_CONFIG3),
        READ_PORT_USHORT(RTL_MULINT),
        READ_PORT_USHORT(RTL_RERID)));

    PRINTF (1, ("TSAD    [0x%x] \t BMCR    [0x%x] \t BMSR     [0x%x] \t ANAR     [0x%x]\r\n",
        READ_PORT_USHORT(RTL_TSAD),
        READ_PORT_USHORT(RTL_BMCR),
        READ_PORT_USHORT(RTL_BMSR),
        READ_PORT_USHORT(RTL_ANAR)));

    PRINTF (1, ("ANLPAR  [0x%x] \t ANER    [0x%x] \t DIS      [0x%x] \t FCSC     [0x%x]\r\n",
        READ_PORT_USHORT(RTL_ANLPAR),
        READ_PORT_USHORT(RTL_ANER),
        READ_PORT_USHORT(RTL_DIS),
        READ_PORT_USHORT(RTL_FCSC)));

    PRINTF (1, ("NWAYTR  [0x%x] \t REC     [0x%x] \t CSCR     [0x%x] \t PHY1_PARM[0x%x]\r\n",
        READ_PORT_USHORT(RTL_NWAYTR),
        READ_PORT_USHORT(RTL_REC),
        READ_PORT_USHORT(RTL_CSCR),
        READ_PORT_ULONG(RTL_PHY1_PARM)));

    PRINTF (1, ("TW_PARM [0x%x] \t PHY2PARM[0x%x] \r\n\r\n",
        READ_PORT_ULONG(RTL_TW_PARM),
        READ_PORT_UCHAR(RTL_PHY2_PARM)));

}   // DumpAll8139Regs()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139InitDMABuffer()
//  This function is used to inform this library on where the buffer for
//  Tx/Rx DMA is.
//
BOOL
RTL8139InitDMABuffer(DWORD dwStartAddress, DWORD dwSize)
{
    DWORD   dwRxSize;
    DWORD   dwAlignedStart;

    ////////////////////////////////////////////////////////////////////////////
    //  We need at least:
    //  4 TX buffers (4 * ONE_BUFFER_SIZE) ~ 6k
    //  8K + 16 bytes of RX buffers.
    //  (64K Max) of RX buffer size is welcome!
    //  ** Note ** that ONE_BUFFER_SIZE is 1536 bytes which is in DWORD
    //  boundary!
    //
    #define MIN_DMA_SIZE    ((NUM_TX_DESC * ONE_BUFFER_SIZE) + (8 * 1024 + 16))

    dwAlignedStart = (dwStartAddress + 0x03) & 0xfffffffc;
    dwSize -= (dwAlignedStart - dwStartAddress);

    if (dwSize <= MIN_DMA_SIZE)
    {
        PRINTF (1, ("RTL8139InitDMABuffer():: DMA buffer is too small!!\r\n"));
        PRINTF (1, ("Given:[0x%x] :: Minimun required [0x%x]\r\n",
            dwSize,
            MIN_DMA_SIZE));

        return FALSE;
    }

    PRINTF (0,
        ("RTL8139InitDMABuffer():: Start[0x%x]-[0x%x] - Size[0x%x]\r\n",
        dwStartAddress,
        dwAlignedStart,
        dwSize));

    ////////////////////////////////////////////////////////////////////////////
    //  Okay, so we are in business..
    //  TX is fixed, so see how much RX we have.
    //

    dwTxStartAddress = dwAlignedStart;
    dwRxStartAddress = dwAlignedStart + (NUM_TX_DESC * ONE_BUFFER_SIZE);
    dwRxSize = dwSize - NUM_TX_DESC * ONE_BUFFER_SIZE;

    if (dwRxSize > (64 * 1024 + 16))
    {
        dwRxLength      = 64 * 1024;
        dwRxLengthBit   = 3 << 11;

    }
    else
    if (dwRxSize > (32 * 1024 + 16))
    {
        dwRxLength      = 32 * 1024;
        dwRxLengthBit   = 2 << 11;

    }
    else
    if (dwRxSize > (16 * 1024 + 16))
    {
        dwRxLength      = 16 * 1024;
        dwRxLengthBit   = 1 << 11;

    }
    else
    {
        dwRxLength      = 8 * 1024;
        dwRxLengthBit   = 0x00;
    }

    PRINTF (0, ("Tx[0x%x] - Rx[0x%x] - RxLength[0x%x] - RxLengthBit[0x%x]\r\n",
        dwTxStartAddress,
        dwRxStartAddress,
        dwRxLength,
        dwRxLengthBit));

    return TRUE;


}   //  RTL8139InitDMABuffer()


#define AUTO_NEGOTIATE  1

////////////////////////////////////////////////////////////////////////////////
//  RTL8139Init()
//
BOOL
RTL8139Init(UINT8 *pbBaseAddress, UINT32 dwLogicalLocation, UINT16 MacAddr[3])
{
    DWORD   dwScrap;
#ifndef AUTO_NEGOTIATE
    USHORT  usScrap;

#else
    DWORD   dwStart;
#endif

    DWORD   i;

    PRINTF(1,
        ("RTL8139:: built on [%s] [%s]\r\n",
        __DATE__,
        __TIME__));


    if (dwTxStartAddress == 0x00)
    {
        ////////////////////////////////////////////////////////////////////////
        //  Caller ** MUST ** call RTL8139InitDMABuffer() first!!
        //
        PRINTF (1,
            ("RTL8139():: Error!! RTL8139InitDMABuffer() was not called.\r\n"));
        return FALSE;
    }

    PRINTF (0, ("RTL8139Init():: BaseIO[0x%x] : LogicalLocation[0x%x]\r\n",
        pbBaseAddress,
        dwLogicalLocation));

    memset (&sRTL8139, 0x00, sizeof(SRTL8139));

    sRTL8139.dwBaseIO            = (DWORD)pbBaseAddress;
    sRTL8139.dwLogicalLocation   = dwLogicalLocation;
    sRTL8139.dwFreeTxDescriptors = NUM_TX_DESC;

    ////////////////////////////////////////////////////////////////////////////
    //  Caller ** MUST HAVE ** called RTL8139InitDMABuffer()
    //
    sRTL8139.pucTxBuffer     = (PUCHAR)(dwTxStartAddress);
    sRTL8139.pucRxBuffer     = (PUCHAR)(dwRxStartAddress);
    sRTL8139.puLastRxAddress = sRTL8139.pucRxBuffer + dwRxLength - 1;

    PRINTF (0, ("RTL8139Init():: TxBuff[0x%x] - RxBuff[0x%x] - LastRx[0x%x]\r\n",
        sRTL8139.pucTxBuffer,
        sRTL8139.pucRxBuffer,
        sRTL8139.puLastRxAddress));


    ////////////////////////////////////////////////////////////////////////////
    //  First stop, soft reset the NIC, and fill up the MacAddr.
    //
    if( NULL != MacAddr ) {
        WRITE_PORT_UCHAR(RTL_CR, RTL8139_COMMAND_RESET);
        while(READ_PORT_UCHAR(RTL_CR) & RTL8139_COMMAND_RESET)
            ;

        WRITE_PORT_UCHAR(
            RTL_CR,
            (RTL8139_COMMAND_TX_ENABLE | RTL8139_COMMAND_RX_ENABLE));

        dwScrap = READ_PORT_ULONG(RTL_IDR0);
        sRTL8139.usMacAddr[0] = (USHORT)(dwScrap);
        sRTL8139.usMacAddr[1] = (USHORT)(dwScrap >> 16);

        dwScrap = READ_PORT_ULONG(RTL_IDR4);
        sRTL8139.usMacAddr[2] = (USHORT)(dwScrap);

        PRINTF (1, ("RTL8139Init:: MAC = %02X-%02X-%02X-%02X-%02X-%02X\r\n",
            sRTL8139.usMacAddr[0] & 0x00FF,
            sRTL8139.usMacAddr[0] >> 8,
            sRTL8139.usMacAddr[1] & 0x00FF,
            sRTL8139.usMacAddr[1] >> 8,
            sRTL8139.usMacAddr[2] & 0x00FF,
            sRTL8139.usMacAddr[2] >> 8));

        memcpy(
            &(MacAddr[0]),
            &(sRTL8139.usMacAddr[0]),
            6);
    }

#ifndef AUTO_NEGOTIATE

    ////////////////////////////////////////////////////////////////////////////
    //  Currently hardcode to:
    //  -  10Mbps
    //  -  Auto negotiation disabled.
    //  -  Simplex
    //

    PRINTF (1, ("Hardcoded to 10Mbps..\r\n"));

    usScrap = READ_PORT_USHORT(RTL_BMCR);
    usScrap &= ~(RTL_100MBPS | RTL_FULL_DUPLEX | RTL_AUTO_NEGOTIATE_ENABLE);
    WRITE_PORT_USHORT(RTL_BMCR, usScrap);

#else

    //
    //  Auto Negotiate!!
    //

    PRINTF (0, ("RTL8139 ethdbg library: perform auto negotiate.\r\n"));

    WRITE_PORT_USHORT(RTL_ANAR, 0x01E1);

    dwStart = OALGetTickCount();

    while ((READ_PORT_USHORT(RTL_BMSR) & RTL_AUTO_NEGOTIATION_COMPLETE) == 0)
    {
        if (OALGetTickCount() - dwStart > 3000)
        {
            //
            //  Hardcode to 10Mbps, autoneg disabled, simplex.
            //
            PRINTF (1, ("Auto negotiation failed.   Hardcode to 10Mbps..\r\n"));

            WRITE_PORT_USHORT(RTL_BMCR, 0x0000);
#if 0

            PRINTF (1, ("BMSR   = [0x%x]\r\n", READ_PORT_USHORT(RTL_BMSR)));
            PRINTF (1, ("ANER   = [0x%x]\r\n", READ_PORT_USHORT(RTL_ANER)));
            PRINTF (1, ("ANLPAR = [0x%x]\r\n", READ_PORT_USHORT(RTL_ANLPAR)));
#endif

            break;
        }
    }

#endif


    ////////////////////////////////////////////////////////////////////////////
    //  Prepare the TX buffers & descriptors.
    //
    for (i = 0x00 ; i < NUM_TX_DESC ; i++)
    {
        WRITE_PORT_ULONG(
            RTL_TSAD_X(i),
            TO_REAL((DWORD)(sRTL8139.pucTxBuffer + i * ONE_BUFFER_SIZE)));
    }

    ////////////////////////////////////////////////////////////////////////////
    //  TX DMA burst size per TX DMA burst.
    //
    WRITE_PORT_ULONG(
        RTL_TCR,
        (TX_DMA_BURST_SIZE) | 0x03000000);


    //
    //  Init multicast registers to 0
    //

    memset(
        g_pucMulticastList,
        0x00,
        MAX_MULTICAST_LIST * 6);

    g_dwMulticastListInUse = 0x00;

    RTL8139HWSetMCRegs(NULL, FALSE);

    ////////////////////////////////////////////////////////////////////////////
    //  Now, prepare the RX buffers & descriptors and RX Mode.
    //  Hardcode to accept only broadcast and myself.
    //

    g_dwRCR = RX_BROADCAST      |
              RX_UNICAST        |
              dwRxLengthBit     |
              RX_DMA_BURST_SIZE |
              RX_ACCEPT_RUNT;

    WRITE_PORT_ULONG (RTL_RCR, g_dwRCR);

    WRITE_PORT_ULONG(
        RTL_RBSTART,
        TO_REAL((DWORD)(sRTL8139.pucRxBuffer)));

    WRITE_PORT_USHORT(
        RTL_ANAR,
        0x21);

    WRITE_PORT_USHORT(
        RTL_ISR,
        0xffff);

#if 0
    DumpAll8139Regs();
#endif

    return TRUE;

}   //  RTL8139Init()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139EnableInts()
//
void
RTL8139EnableInts()
{
    PRINTF (1, ("+RTL8139EnableInts()..\r\n"));

    ////////////////////////////////////////////////////////////////////////////
    //  We are only interested in RX interrupts.
    //  Also turn on TX interrupts for quick purge of TX descriptors..
    //
    WRITE_PORT_USHORT(
        RTL_IMR,
        0x05);


    PRINTF (1, ("-RTL8139EnableInts()..\r\n"));
}   // RTL8139EnableInts()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139DisableInts()
//
void
RTL8139DisableInts()
{
    PRINTF (0, ("RTL8139DisableInts()..\r\n"));

    WRITE_PORT_USHORT(
        RTL_IMR,
        0x00);


}   //  RTL8139DisableInts()



#define RX_ERR_ISE      (1 << 5)
#define RX_ERR_RUNT     (1 << 4)
#define RX_ERR_LONG     (1 << 3)
#define RX_ERR_CRC      (1 << 2)
#define RX_ERR_FAE      (1 << 1)

#define BAD_RX_PACKET       \
            (RX_ERR_ISE | RX_ERR_RUNT | RX_ERR_LONG | RX_ERR_CRC | RX_ERR_FAE)

////////////////////////////////////////////////////////////////////////////////
//  ExtractData()
//  This function will take the buffer pointer and it's start and end address.
//  It will extract dwSize amount of bytes and handle the wrap around case.
//
UCHAR *
ExtractData(
    UCHAR   *pucSource,
    UCHAR   *pucSourceHead,
    UCHAR   *pucSourceTail,
    UCHAR   *pucDest,
    DWORD   dwSize)
{

    PRINTF (0, ("Extracting fr [0x%x] - to [0x%x] - H[0x%x] - T[0x%x] - tot [%d]\r\n",
        pucSource,
        pucDest,
        pucSourceHead,
        pucSourceTail,
        dwSize));

    if (pucSource + dwSize <= pucSourceTail)
    {
        ////////////////////////////////////////////////////////////////////////
        //  No problem, straight copy.
        //
        memcpy(
            pucDest,
            pucSource,
            dwSize);

        PRINTF (0, ("Returning: [0x%x]\r\n",
            (pucSource + dwSize)));

        return (pucSource + dwSize);
    }
    else
    {
        DWORD   dwFirstCopySize;

        ////////////////////////////////////////////////////////////////////////
        //  Wrap around copy..
        //
        dwFirstCopySize = pucSourceTail - pucSource + 1;
        memcpy(
            pucDest,
            pucSource,
            dwFirstCopySize);

        if (dwSize > dwFirstCopySize)
        {
            memcpy(
                pucDest + dwFirstCopySize,
                pucSourceHead,
                dwSize - dwFirstCopySize);
        }            

        PRINTF (0, ("ReturningWrap: [0x%x]\r\n",
            (pucSource + (dwSize - dwFirstCopySize))));

        return (pucSourceHead + (dwSize - dwFirstCopySize));
    }


}   //  ExtractData()



DWORD
RTL8139GetPendingInts(void);


////////////////////////////////////////////////////////////////////////////////
//  RTL8139GetFrame()
//  Fill up pbData with the next availabe packet in the RX FIFO.
//
UINT16
RTL8139GetFrame(BYTE *pbData, UINT16 *pwLength)
{
    PACKET_HEADER   PacketHeader;
    UCHAR           *pucRxBuffer;

    UINT16  dwISR;

    *pwLength = 0x00;

    PRINTF (0, ("->\r\n"));

    //  GetPendingInts( ) is not called by KITL anymore, so call it here to
    //  fix other issues reported in interrupt.

    RTL8139GetPendingInts();

    // read interrupt
    dwISR = READ_PORT_USHORT(RTL_ISR);
    PRINTF (0, ("RGF(0x%x)..",dwISR));

    if (!(READ_PORT_UCHAR(RTL_CR) & RTL8139_COMMAND_BUFFER_EMPTY))
    {
        pucRxBuffer = sRTL8139.pucRxBuffer + sRTL8139.dwCurrentRxOffset;

        PRINTF (0, ("$ "));

        PRINTF (0, ("[a] [0x%x]\r\n", pucRxBuffer));


        ////////////////////////////////////////////////////////////////////////
        //  Now, the first 4 bytes form the PACKET_HEADER.
        //
        pucRxBuffer = ExtractData(
                        pucRxBuffer,
                        sRTL8139.pucRxBuffer,
                        sRTL8139.puLastRxAddress,
                        (PUCHAR)&PacketHeader,
                        sizeof(PACKET_HEADER));

        PRINTF (0, ("[b] [0x%x]\r\n", pucRxBuffer));
        PRINTF (0, (">RX L= [%d] bytes.\r\n",
                PacketHeader.usPacketLength));

        if (PacketHeader.usPacketHeader & BAD_RX_PACKET)
        {
            PRINTF (1, ("Bad RX packet [Header: 0x%x -- Length : [%d]]...\r\n",
                PacketHeader.usPacketHeader,
                PacketHeader.usPacketLength));

            // ack the interrupt so we can get interrupted again
            WRITE_PORT_USHORT(
                RTL_ISR,
                dwISR & INTR_RX_OK);

            // don't extract bad data
            //pucRxBuffer = ExtractData(
            //                pucRxBuffer,
            //                sRTL8139.pucRxBuffer,
            //               sRTL8139.puLastRxAddress,
            //                pbData,
            //                PacketHeader.usPacketLength);

            sRTL8139.dwCurrentRxOffset = READ_PORT_USHORT(RTL_CBR);
            sRTL8139.dwCurrentRxOffset = (sRTL8139.dwCurrentRxOffset + 3) & ~3;
            

            WRITE_PORT_USHORT(
                RTL_CAPR,
                (USHORT)(sRTL8139.dwCurrentRxOffset - 0x10));

            return 0x00;
        }
        else
        {
            pucRxBuffer = ExtractData(
                            pucRxBuffer,
                            sRTL8139.pucRxBuffer,
                            sRTL8139.puLastRxAddress,
                            pbData,
                            PacketHeader.usPacketLength);

            PRINTF (0, ("[c] [0x%x]\r\n", pucRxBuffer));

            *pwLength = (PacketHeader.usPacketLength - 4);


            //  DumpMemory(
            //      pbData,
            //      48);

            sRTL8139.dwCurrentRxOffset =
                (pucRxBuffer - sRTL8139.pucRxBuffer + 3) & ~3;

            PRINTF (0, ("[d] Offset = [0x%x]\r\n",
                sRTL8139.dwCurrentRxOffset));
        }

        WRITE_PORT_USHORT(
            RTL_CAPR,
            (USHORT)(sRTL8139.dwCurrentRxOffset - 0x10));


        PRINTF (0, ("[%d] \r\n", *pwLength));
        return *pwLength;
    }
    else
    {
        PRINTF (0, ("!# \r\n"));

        // ack the interrupt so we can get interrupted again
        WRITE_PORT_USHORT(
            RTL_ISR,
            dwISR & INTR_RX_OK);

        return 0x00;
    }


}    // RTL8139GetFrame()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139SendFrame()
//      This routine should be called with a pointer to the ethernet frame data.
//      It is the caller's responsibility to fill in all information including
//      the destination and source addresses and the frame type.
//      The length parameter gives the number of bytes in the ethernet frame.
//      The routine will return immediately regardless of whether transmission
//      has successfully been performed.
//


#define TX_OWN_BIT              (1 << 13)
#define TX_TOK_BIT              (1 << 15)
#define TX_ABORT                (1 << 30)
#define FREE_TX_DESCRIPTOR      (TX_TOK_BIT)
#define GET_TSD(x)  \
            (READ_PORT_ULONG(RTL_TSD_X(x)) & FREE_TX_DESCRIPTOR)

void
PurgeTxDescriptors()
{
    int     i     = 0x00;

    while((sRTL8139.dwFreeTxDescriptors < NUM_TX_DESC) &&
          (GET_TSD(sRTL8139.dwCurrentHwTxDescriptor) & FREE_TX_DESCRIPTOR))
    {
        i++;
        NEXT(sRTL8139.dwCurrentHwTxDescriptor, NUM_TX_DESC);
        sRTL8139.dwFreeTxDescriptors++;
    }

    ////////////////////////////////////////////////////////////////////////////
    //  If it was transmit abort in any of the TX desc, restart it.
    //
    if (!i)
    {
        for (i = 0 ; i < 4 ; i++)
        {
            if (READ_PORT_ULONG(RTL_TSD_X(i)) & TX_ABORT)
            {
                PRINTF (1, ("* "));
                WRITE_PORT_ULONG(
                    RTL_TCR,
                    TX_DMA_BURST_SIZE | TX_CLEAR_ABORT | 0x03000000);
                break;
            }
        }
    }


}   //  PurgeTxDescriptors()



UINT16
RTL8139SendFrame(BYTE *pbData, DWORD dwLength)
{
    DWORD   i;
    DWORD   dwWaitTime;


    PRINTF (0, ("<-\r\n"));

    PRINTF (0, ("RSF [0x%x].. ",
            READ_PORT_USHORT(RTL_ISR)));

    ////////////////////////////////////////////////////////////////////////////
    //  Check if the current TX descriptor is available.
    //  If not then we loop wait till it is available.
    //
    dwWaitTime = 100000;

    while (sRTL8139.dwFreeTxDescriptors == 0)
    {
        PurgeTxDescriptors();

        PRINTF (0, ("![%d, %x - %x - %x - %x] [0x%x]",
            sRTL8139.dwCurrentHwTxDescriptor,
            READ_PORT_ULONG(RTL_TSD_X(0)),
            READ_PORT_ULONG(RTL_TSD_X(1)),
            READ_PORT_ULONG(RTL_TSD_X(2)),
            READ_PORT_ULONG(RTL_TSD_X(3)),
            READ_PORT_USHORT(RTL_ISR)));

        ////////////////////////////////////////////////////////////////////////
        //  Oh well, chip is toasted... SOFT RESET it.
        //
        if (!--dwWaitTime)
        {
            USHORT  usDummy[3];

            PRINTF (1, ("@ "));
            dwWaitTime = 100000;

            PRINTF (1, ("< TX Reset >\r\n"));
            PRINTF (1, ("HW Descriptor [%d] :: SW Descriptor [%d] :: Free = [%d]",
                    sRTL8139.dwCurrentHwTxDescriptor,
                    sRTL8139.dwCurrentTxDescriptor,
                    sRTL8139.dwFreeTxDescriptors));
            DumpAll8139Regs();

            WRITE_PORT_UCHAR(
                RTL_CR,
                (RTL8139_COMMAND_RX_ENABLE));

            WRITE_PORT_UCHAR(
                RTL_CR,
                (RTL8139_COMMAND_TX_ENABLE | RTL8139_COMMAND_RX_ENABLE));

            RTL8139DisableInts();
            RTL8139Init(
                (PBYTE)sRTL8139.dwBaseIO,
                sRTL8139.dwLogicalLocation,
                usDummy);
            RTL8139EnableInts();
        }
    }

    sRTL8139.dwFreeTxDescriptors--;

    ////////////////////////////////////////////////////////////////////////////
    //  So we are definitely using this descriptor.
    //  Advance it for the next call.
    //
    i = sRTL8139.dwCurrentTxDescriptor;
    NEXT(sRTL8139.dwCurrentTxDescriptor, NUM_TX_DESC);

    PRINTF (0, (">RSF() [%d] S = [%d]\r\n",
        i,
        dwLength));


    ////////////////////////////////////////////////////////////////////////////
    //  Copy the user data and set it to go.
    //
    memcpy(
        sRTL8139.pucTxBuffer + i * ONE_BUFFER_SIZE,
        pbData,
        dwLength);

#if 0
    DumpMemory(
        pbData,
        dwLength);

    DumpMemory(
        sRTL8139.pucTxBuffer + i * ONE_BUFFER_SIZE,
        dwLength);
#endif


    WRITE_PORT_ULONG(
        RTL_TSAD_X(i),
        TO_REAL((DWORD)(sRTL8139.pucTxBuffer + i * ONE_BUFFER_SIZE)));


    ////////////////////////////////////////////////////////////////////////////
    //  Minimum packet size required by RTL
    //
    if (dwLength < 60)
    {
        memset(
          (sRTL8139.pucTxBuffer + i * ONE_BUFFER_SIZE) + dwLength,
            0x00,
            60 - dwLength);

        dwLength = 60;
    }


    ////////////////////////////////////////////////////////////////////////////
    //  Send it out now..
    //  8 bytes threshold.
    //
    WRITE_PORT_ULONG(
        RTL_TSD_X(i),
        (DWORD)(dwLength) | (0x00 << 16));

    return 0x00;

}    // RTL8139SendFrame()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139GetPendingInts()
//  EDBG only cares about INTR_RX_OK, we handle all others internally.
//
DWORD
RTL8139GetPendingInts(void)
{
    USHORT  dwISR;
    DWORD   dwReturn;

    PRINTF (0, ("$\r\n"));
    PRINTF (0, ("RTL8139GetPendingInts()..\r\n"));

    dwISR = READ_PORT_USHORT(RTL_ISR);

    if (dwISR & INTR_TX_OK)
        PurgeTxDescriptors();

    if (dwISR & INTR_RX_OK)
        dwReturn = 0x01;
    else
        dwReturn = 0x00;

    ////////////////////////////////////////////////////////////////////////////
    //  If RX FIFO overflow or RX Buffer overflow then reset the
    //  chip [???]   Too severe ???
    //
    if (dwISR & 0x0050)
    {
        USHORT  usDummy[3];

        PRINTF (1, ("ISR[0x%x] >> RX overflow.. Reset <<\r\n", dwISR));
        RTL8139DisableInts();
            RTL8139Init(
                (PBYTE)sRTL8139.dwBaseIO,
                sRTL8139.dwLogicalLocation,
                usDummy);
            RTL8139EnableInts();
        return 0x00;
    }


    WRITE_PORT_USHORT(
            RTL_ISR,
            dwISR & ~INTR_RX_OK);  // do not handle INTR_RX_OK here
    return dwReturn;

}    // RTL8139GetPendingInts()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139ReadEEPROM()
//
BOOL
RTL8139ReadEEPROM (UINT16 uiEEPROMAddress , UINT16 *pwVal)
{
    UNREFERENCED_PARAMETER(uiEEPROMAddress);
    UNREFERENCED_PARAMETER(pwVal);
    PRINTF (1, ("RTL8139ReadEEPROM() not implemented..\r\n"));
    return FALSE;

}    // RTL8139ReadEEPROM()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139WriteEEPROM()
//
BOOL
RTL8139WriteEEPROM(UINT16 uiEEPROMAddress, UINT16 uiData)
{
    UNREFERENCED_PARAMETER(uiEEPROMAddress);
    UNREFERENCED_PARAMETER(uiData);
    PRINTF (1, ("RTL8139WriteEEPROM() not implemented..\r\n"));
    return FALSE;

}   // RTL8139WriteEEPROM()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139SetOptions()
//
DWORD
RTL8139SetOptions(DWORD dwOptions)
{
    UNREFERENCED_PARAMETER(dwOptions);
    PRINTF (1, ("RTL8139SetOptions() not implemented..\r\n"));
    return 0x00;

}    // RTL8139SetOptions()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139HWSetMCRegs()
//
//  Description:
//
//      This function is used to set the multicast registers.
//
//  Arguments:
//
//      pucMulticastRegs :: If not null then use this instead of 0xFF or 0x00
//                             depending on bAllMulticast below.
//
//      bAllMulticast    :: TRUE == Set all to 0xff :: FALSE = Set all to 0x00
//
//  Return value:
//
//
void
RTL8139HWSetMCRegs(PUCHAR   pucMulticastRegs, BOOL bAllMulticast)
{
    DWORD   i;

    if (pucMulticastRegs)
    {
        DWORD   dwMulticast0To3 = 0x00;
        DWORD   dwMulticast4To7 = 0x00;

        KITLOutputDebugString(
            "RTL8139:: MulticastRegs set to : %x-%x-%x-%x-%x-%x-%x-%x\r\n",
            pucMulticastRegs[0],
            pucMulticastRegs[1],
            pucMulticastRegs[2],
            pucMulticastRegs[3],
            pucMulticastRegs[4],
            pucMulticastRegs[5],
            pucMulticastRegs[6],
            pucMulticastRegs[7]);

        for (i = 0 ; i < 4 ; i++)
            dwMulticast0To3 = dwMulticast0To3 +
                              (pucMulticastRegs[i] << (8 * i));

        for (i = 4 ; i < 8 ; i++)
            dwMulticast4To7 = dwMulticast4To7 +
                              (pucMulticastRegs[i] << (8 * (i - 4)));


        WRITE_PORT_ULONG(RTL_MAR0, dwMulticast0To3);
        WRITE_PORT_ULONG(RTL_MAR4, dwMulticast4To7);
    }
    else
    {
        DWORD   dwMulticastRegs;

        if (bAllMulticast)
        {
            dwMulticastRegs = 0xffffffff;
            KITLOutputDebugString("RTL8139:: RTL8139HWSetMCRegs():: Set all to 0xff\r\n");
        }
        else
        {
            dwMulticastRegs = 0x00;
            KITLOutputDebugString("RTL8139:: RTL8139HWSetMCRegs():: Set all to 0x00\r\n");
        }


        WRITE_PORT_ULONG(RTL_MAR0, dwMulticastRegs);
        WRITE_PORT_ULONG(RTL_MAR4, dwMulticastRegs);
    }


}   //  RTL8139HWSetMCRegs()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139CurrentPacketFilter()
//
//  Description:
//
//      This function is called to set the h/w filter.
//      We support:
//          PROMISCUOUS, ALL MULTICAST, BROADCAST, DIRECTED.
//
//  Arguments:
//
//      dwFilter::  The filter mode
//
//  Return value:
//
//      TRUE if successful, FALSE otherwise.
//
void
RTL8139CurrentPacketFilter(DWORD    dwFilter)
{
    if (dwFilter & (PACKET_TYPE_ALL_MULTICAST | PACKET_TYPE_PROMISCUOUS))
    {
        //
        //  Need all multicast..
        //
        RTL8139HWSetMCRegs(NULL, TRUE);
    }
    else
    {
        //
        //  No longer need "all multicast".
        //  When we do support MULTICAST LIST then we should copy the list
        //  to the h/w here..
        //

        UCHAR   pucMulticastList[MAX_MULTICAST_LIST][6];

        memcpy(
            pucMulticastList,
            &(g_pucMulticastList[0][0]),
            g_dwMulticastListInUse * 6);

        RTL8139MulticastList(
            &(pucMulticastList[0][0]),
            g_dwMulticastListInUse);
    }


    //
    //  The multicast bit in the RCR should be on if multicast/all multicast packets
    //  (or is promiscuous) is needed.
    //
    if(dwFilter &
        (PACKET_TYPE_ALL_MULTICAST | PACKET_TYPE_MULTICAST | PACKET_TYPE_PROMISCUOUS))
    {
        g_dwRCR |= RX_MULTICAST;
    }
    else
    {
        g_dwRCR &= ~RX_MULTICAST;
    }

    //
    //  The promiscuous physical bit in the RCR should be on if ANY
    //  protocol wants to be promiscuous.
    //
    if (dwFilter & PACKET_TYPE_PROMISCUOUS)
    {
        g_dwRCR |= RX_PROMISCUOUS;
    }
    else
    {
        g_dwRCR &= ~RX_PROMISCUOUS;
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
        g_dwRCR |= RX_BROADCAST;
    }
    else
    {
        g_dwRCR &= ~RX_BROADCAST;
    }


    //
    //  EDBG always receive directed and broadcast & directed as a minimum.
    //

    g_dwRCR |= (RX_BROADCAST | RX_UNICAST);

    g_dwNdisFilter = dwFilter;

    KITLOutputDebugString("RTL8139:: Set Ndis filter [0x%x] -> [0x%x]\r\n",
        dwFilter,
        g_dwRCR);


    //
    //  Burn it to h/w..
    //

    WRITE_PORT_ULONG (RTL_RCR, g_dwRCR);

}   //  RTL8139CurrentPacketFilter()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139ComputeCrc()
//
//  Description:
//
//      Runs the AUTODIN II CRC algorithm on buffer Buffer of length Length.
//
//  Arguments:
//
//      Buffer - the input buffer
//      Length - the length of Buffer
//
//  Return value:
//
//      The 32-bit CRC value.
//
//  Note:
//
//      This function is adopted from netcard\ne2000 miniport driver.
//
ULONG
RTL8139ComputeCrc(
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

}   //  RTL8139ComputeCrc()



////////////////////////////////////////////////////////////////////////////////
//  RTLGetMulticastBit()
//
//  Description:
//
//      For a given multicast address, returns the byte and bit in the card
//      multicast registers that it hashes to. Calls RTL8139ComputeCrc() to
//      determine the CRC value.
//
//  Arguments:
//
//      Address - the address
//      Byte    - the byte that it hashes to
//      Value   - will have a 1 in the relevant bit
//
//  Return value:
//
//      None.
//
//  Note:
//
//      This function is adopted from netcard\ne2000 miniport driver.
//
VOID
RTLGetMulticastBit(
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

    Crc = RTL8139ComputeCrc(Address, 6);


    //
    // The bit number is now in the 6 most significant bits of CRC.
    //

    BitNumber = (UINT)((Crc >> 26) & 0x3f);

    *Byte = (UCHAR)(BitNumber / 8);
    *Value = (UCHAR)((UCHAR)1 << (BitNumber % 8));

}   //  RTLGetMulticastBit()



////////////////////////////////////////////////////////////////////////////////
//  RTL8139MulticastList()
//
//  Description:
//
//      This function is used to insert multicast addresses to the h/w.
//
//  Arguments:
//
//      pucMulticastAddresses   :: The list of multicast addressses.
//      dwNoOfAddresses         :: The number of addresses in the list.
//
//  Return value:
//
//      TRUE if successful, FALSE otherwise.
//

BOOL
RTL8139MulticastList(PUCHAR pucMulticastAddresses, DWORD dwNoOfAddresses)
{
    UCHAR   NicMulticastRegs[8];        // contents of card multicast registers
    UINT    i;
    UCHAR   Byte, Bit;

#if 0
    KITLOutputDebugString(
        "RTL8139:: RTL8139MulticastList() set [%d] multicast list addrs.\r\n",
        dwNoOfAddresses);
#endif


    //
    //  Make sure it's below max list we can handle..
    //

    if (dwNoOfAddresses > MAX_MULTICAST_LIST)
    {
        KITLOutputDebugString(
            "RTL8139:: RTL8139MulticastList exceeds max list..\r\n");
        return FALSE;
    }


    //
    //  Remember these addresses as we may need them again when filter
    //  is switched from All Multicast to multicast.
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
            "RTL8139:: Multicast[%d]  = %x-%x-%x-%x-%x-%x\r\n",
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
    //  If we are already in Multicast or Promiscuous mode, then no
    //  need to set this..
    //

    if (g_dwNdisFilter & (PACKET_TYPE_PROMISCUOUS |
                          PACKET_TYPE_ALL_MULTICAST))
    {
        return TRUE;
    }


    //
    //  First turn all bits off.
    //

    for (i=0; i<8; i++)
        NicMulticastRegs[i] = 0;


    //
    // Now turn on the bit for each address in the multicast list.
    //

    for (i=0 ; i<dwNoOfAddresses; i++)
    {
        RTLGetMulticastBit(&pucMulticastAddresses[6*i], &Byte, &Bit);
        NicMulticastRegs[Byte] |= Bit;
    }

#if 0

    KITLOutputDebugString(
        "RTL8139:: MulticastRegs = %x-%x-%x-%x-%x-%x-%x-%x\r\n",
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
    //  Finally, burn that in the h/w..
    //

    RTL8139HWSetMCRegs(NicMulticastRegs, FALSE);

    return TRUE;

}   //  RTL8139MulticastList()


