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
#include <wdm.h>
#include <kitlprot.h>
#include <oal_memory.h>
#include "dec21140.h"

#define MIN_DMA_SIZE    0x10000     // Minimum DMA buffer possible.
#define DEC21140_SETUP_PERFECT_ENTRIES  16  // # of perfect match entries

#ifndef DEC21140_HAS_MII
#define DEC21140_HAS_MII 0
#endif

//  Forward declaration...
void    DEC21140ModeSetByAutoNegotiation(void);
BOOL    EEPROMReadMAC(WORD* wMAC);
void    DEC21140InitTxDescriptor (DWORD TxHead, DWORD TxBuffSize, BOOL bVirt);
void    DEC21140InitRxDescriptor (DWORD RxHead, DWORD RxBuffSize, BOOL bVirt);
DWORD   DEC21140QueryBufferSize (void);
DWORD   DEC21140QueryDescriptorSize (void);


/////////////////////////////////////////////////////////////////////////////////
//
//

#define __DEC21140_DUMP_TX_DESCRIPTOR__ 1
#ifdef  __DEC21140_DUMP_TX_DESCRIPTOR__
    static void DumpTxDescriptor (PTX_DESCRIPTOR_FORMAT pTxHead);
#endif


#define __DEC21140_DUMP_RX_DESCRIPTOR__ 1
#ifdef  __DEC21140_DUMP_RX_DESCRIPTOR__
    static void DumpRxDescriptor (PRX_DESCRIPTOR_FORMAT pRxHead);
#endif


#define __DEC21140_DUMP_MEMORY__    1
#ifdef  __DEC21140_DUMP_MEMORY__
    static void DumpMemory (PBYTE   pSource, DWORD  dwLength);
#endif


/////////////////////////////////////////////////////////////////////////////////
//  External Functions that must be there...
//

//extern void   localDEBUGMSG(const unsigned char *sz, ...);
#define localDEBUGMSG //KITLOutputDebugString




/////////////////////////////////////////////////////////////////////////////////
//  Misc defines
//
#define  CSR0_OFFSET     0x00
#define  CSR1_OFFSET     0x08
#define  CSR2_OFFSET     0x10
#define  CSR3_OFFSET     0x18
#define  CSR4_OFFSET     0x20
#define  CSR5_OFFSET     0x28
#define  CSR6_OFFSET     0x30
#define  CSR7_OFFSET     0x38
#define  CSR8_OFFSET     0x40
#define  CSR9_OFFSET     0x48
#define  CSR10_OFFSET    0x50
#define  CSR11_OFFSET    0x58
#define  CSR12_OFFSET    0x60
#define  CSR13_OFFSET    0x68
#define  CSR14_OFFSET    0x70
#define  CSR15_OFFSET    0x78

#define  CSR0_REG       (PULONG)(g_pIOBase + CSR0_OFFSET)
#define  CSR1_REG       (PULONG)(g_pIOBase + CSR1_OFFSET)
#define  CSR2_REG       (PULONG)(g_pIOBase + CSR2_OFFSET)
#define  CSR3_REG       (PULONG)(g_pIOBase + CSR3_OFFSET)
#define  CSR4_REG       (PULONG)(g_pIOBase + CSR4_OFFSET)
#define  CSR5_REG       (PULONG)(g_pIOBase + CSR5_OFFSET)
#define  CSR6_REG       (PULONG)(g_pIOBase + CSR6_OFFSET)
#define  CSR7_REG       (PULONG)(g_pIOBase + CSR7_OFFSET)
#define  CSR8_REG       (PULONG)(g_pIOBase + CSR8_OFFSET)
#define  CSR9_REG       (PULONG)(g_pIOBase + CSR9_OFFSET)
#define  CSR10_REG      (PULONG)(g_pIOBase + CSR10_OFFSET)
#define  CSR11_REG      (PULONG)(g_pIOBase + CSR11_OFFSET)
#define  CSR12_REG      (PULONG)(g_pIOBase + CSR12_OFFSET)
#define  CSR13_REG      (PULONG)(g_pIOBase + CSR13_OFFSET)
#define  CSR14_REG      (PULONG)(g_pIOBase + CSR14_OFFSET)
#define  CSR15_REG      (PULONG)(g_pIOBase + CSR15_OFFSET)

#ifdef MIPS
#define TO_REAL(Addr)   ((Addr & 0x1fffffff) + dwLogicalLocation)
#define TO_VIRT(Addr)   ((Addr | 0xA0000000) - dwLogicalLocation)
#else
#define TO_VIRT(addr)   (bEbootImage ? (ULONG)addr : ((ULONG)OALPAtoUA((UINT32)(addr))))
#define TO_REAL(addr)   (bEbootImage ? (ULONG)addr : ((ULONG)OALVAtoPA((VOID *)(addr))))
#endif

static BOOL bEbootImage = FALSE;

/////////////////////////////////////////////////////////////////////////////////
//  Local Variables...
//
static      BYTE                    pbEthernetAddr[6];      //  Local copy of MAC address
volatile    PCSR                    pCSR;                   //  pointer to 21140 Control and Status Register
PUCHAR g_pIOBase = NULL;

volatile    PRX_DESCRIPTOR_FORMAT   pRxDesc;                //  pointer to RX Descriptor head.
volatile    PRX_DESCRIPTOR_FORMAT   pCurrentRxDesc;         //  pointer to current RX Descriptor that Rx may use.

volatile    PTX_DESCRIPTOR_FORMAT   pTxDesc;                //  pointer to TX Descriptor head.
volatile    PTX_DESCRIPTOR_FORMAT   pCurrentTxDesc;         //  pointer to current TX Descriptor that Tx may use.

static      DWORD                   dwLogicalLocation;


//  These used to be constants.
//  To make this a true hardware independent library, user needs to provide these addresses...

volatile    DWORD   dwTRANSMIT_DESCRIPTORS_HEAD;
volatile    DWORD   dwRECEIVE_DESCRIPTORS_HEAD;
volatile    DWORD   dwTRANSMIT_BUFFER_START;
volatile    DWORD   dwRECEIVE_BUFFER_START;

//  Then we will calculate the following...

static  DWORD   dwTRANSMIT_RING_SIZE;
static  DWORD   dwRECEIVE_RING_SIZE;


//  Forward declaration...
BOOL    DEC21140SetupPerfectFilter (PUCHAR pucMulticastAddresses, DWORD dwNoOfAddresses);


/////////////////////////////////////////////////////////////////////////////////
//  Delay - Spin for the asked amount of time in MilliSecond.
//  Input: dwMilliSecond === Delay required in MilliSecond.
//  Oh well, simply spin...
//
void    Delay (DWORD dwMilliSecond)
{
    //  Need to find a better way...
    //  But should be okay for ether debug purposes...
    while (dwMilliSecond--)
        {
            READ_PORT_ULONG(CSR9_REG);
        }

}   // Delay()



/////////////////////////////////////////////////////////////////////////////////
//  MIIRead() -
//  On ICS1890, it's written:
//  Management Interface uses a serial bit stream with a specified frame
//  structure and protocol as defined:
//  -   Preamble    [32 ones]
//  -   SOF         [01]
//  -   OpCode      [10(read) ; 01(write)]
//  -   Address     [5 bits]
//  -   Register    [5 bits]
//  -   TA          [2 bits]
//  -   Data        [16 bits]
//  -   Idle        [at least 1 bit]
//
//
//  CSR9 controls:
//  +-----------------+-----------------+-----------------+-----------------+
//  | MII Management  | MII Management  | MII Management  | MII Management  |
//  | Data In         | Operation Mode  | Write Data      | Clock           |
//  +-----------------+-----------------+-----------------+-----------------+
//          19                 18                17               16
//

WORD MIIRead (BYTE bRegAddr)
{
    WORD    wValue;
    DWORD   wData;
    int     i;

    // Start with clock low and MDIO high
        WRITE_PORT_ULONG(CSR9_REG, 0x00020000);

    // 32 cycles with MDIO = '1'
    for( i = 0; i < 32; i++ )
    {
        WRITE_PORT_ULONG(CSR9_REG, 0x00030000);
        WRITE_PORT_ULONG(CSR9_REG, 0x00020000);
    }

    // Start of frame 01
    WRITE_PORT_ULONG(CSR9_REG, 0x00000000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00010000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00020000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00030000);


    // Read cycle 10
    WRITE_PORT_ULONG(CSR9_REG, 0x00020000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00030000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00000000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00010000);


    // Physical address of 00001.
    WRITE_PORT_ULONG(CSR9_REG, 0x00000000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00010000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00000000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00010000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00000000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00010000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00000000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00010000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00020000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00030000);


    // Register address
    for( i = 4; i >= 0; i-- )
    {
        if (bRegAddr & (1 << i))
        {
            WRITE_PORT_ULONG(CSR9_REG, 0x00020000);
            WRITE_PORT_ULONG(CSR9_REG, 0x00030000);
        }
        else
        {
            WRITE_PORT_ULONG(CSR9_REG, 0x00000000);
            WRITE_PORT_ULONG(CSR9_REG, 0x00010000);
        }
    }

    // Two cycle turnaround time
    WRITE_PORT_ULONG(CSR9_REG, 0x00040000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00050000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00040000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00050000);


    // Read out the data bits
    wValue = 0;

    for( i = 0; i < 16; i++ )
    {
        wValue <<= 1;

        WRITE_PORT_ULONG(CSR9_REG, 0x00040000);
        wData = READ_PORT_ULONG(CSR9_REG);
        WRITE_PORT_ULONG(CSR9_REG, 0x00050000);

        wData = (wData >> 19);
        wValue |= (WORD) (wData & 1);
    }

    // Two cycle at the end to make sure it's now Idle
    WRITE_PORT_ULONG(CSR9_REG, 0x00040000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00050000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00040000);
    WRITE_PORT_ULONG(CSR9_REG, 0x00050000);

    return wValue;
}




/////////////////////////////////////////////////////////////////////////////////
//  HWStopAdapter -   Stop the adapter by resetting the chip through CSR0[0]
//
static void
HWStopAdapter(void)
{
    CSR6_21140 CSR6;
    CSR0_21140 CSR0;

    CSR6.dwReg = READ_PORT_ULONG(CSR6_REG);
    CSR6.PortSelect = 1;                    //  Select MII/SYM port.
    WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);

    CSR0.dwReg = READ_PORT_ULONG(CSR0_REG);
    CSR0.dwReg |= SOFTWARE_RESET;
    WRITE_PORT_ULONG(CSR0_REG, CSR0.dwReg);

    Delay (12000);

    CSR6.dwReg = READ_PORT_ULONG(CSR6_REG);
    CSR6.TransmitThresholdMode = 1;
    WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);

}   // HWStopAdapter()



/////////////////////////////////////////////////////////////////////////////////
//  HWInit  -   completely initialize the hardware and leave it in a state that is ready to transmit and receive
//              packets.
//

static BOOL
HWInit(void)
{
    int     i;
    int     dwTxRingSize;
    int     dwRxRingSize;
    int     dwTxBufferStart;
    int     dwRxBufferStart;
    volatile    PTX_DESCRIPTOR_FORMAT   pTxHead;
    volatile    PRX_DESCRIPTOR_FORMAT   pRxHead;
    CSR0_21140  localCSR0;
    CSR6_21140  localCSR6;

    dwTxRingSize = dwTRANSMIT_RING_SIZE;
    dwRxRingSize = dwRECEIVE_RING_SIZE;
    dwTxBufferStart = TO_REAL(dwTRANSMIT_BUFFER_START);
    dwRxBufferStart = TO_REAL(dwRECEIVE_BUFFER_START);

    HWStopAdapter();

    // ==========================================================================
    //  The descriptors are linked by circular linked list
    //  They are basically one after another, and we use buffer address 2 as pointer to
    //  next descriptor.
    //  The last descriptor will link back to first descriptor.

    //  First stop, TX descriptors...
    //  Note: Initializing pTxDesc and pCurrentTxDesc for future global use...

    pTxHead = pTxDesc = pCurrentTxDesc = (PTX_DESCRIPTOR_FORMAT) dwTRANSMIT_DESCRIPTORS_HEAD;;
    memset (pTxHead, 0, (sizeof(TX_DESCRIPTOR_FORMAT) * dwTxRingSize));

    for (i = 0 ; i < dwTxRingSize ; i++)
    {
        //  For each descriptor, this is what we need to set:
        //  -   The descriptor is now owned by Host TDES0[31]
        //  -   TDES1[24] set indicating second address is a chained address
        //  -   TDES1[25] set for last descriptor indicating a loop back.
        //  -   TDES2 will point to the buffer this descriptor points to.
        //  -   The next descriptor address in TDES3

        pTxHead->TDES0.dwReg = pTxHead->TDES0.dwReg & DESC_OWNED_BY_HOST;
        pTxHead->TDES1.dwReg = pTxHead->TDES1.dwReg | SECOND_ADDRESS_CHAINED;


        pTxHead->TDES2 = dwTxBufferStart;
        dwTxBufferStart += MAX_BUFFER_SIZE;

        if (i == dwTxRingSize - 1)
        {
            //  Last descriptor points to first descriptor...
            //  And indicates that this is the end of the ring...
            pTxHead->TDES1.TransmitEndOfRing = 1;
            pTxHead->TDES3 = TO_REAL((DWORD) pTxDesc);
        }
        else
        {
            pTxHead->TDES3 = TO_REAL((DWORD) ((DWORD)pTxHead + sizeof(TX_DESCRIPTOR_FORMAT)));
            pTxHead = (PTX_DESCRIPTOR_FORMAT) TO_VIRT(pTxHead->TDES3);
        }

    }
    //DumpTxDescriptor (pTxDesc);


    // ==========================================================================
    //  Then, Rx descriptors...
    //  Note: Initializing pRxDesc for future global use...
    //

    pRxHead = pRxDesc = pCurrentRxDesc = (PRX_DESCRIPTOR_FORMAT) dwRECEIVE_DESCRIPTORS_HEAD;;
    memset (pRxHead, 0, (sizeof(RX_DESCRIPTOR_FORMAT) * dwRxRingSize));
    memset ((PBYTE)TO_VIRT(dwRxBufferStart), 0, dwRxRingSize * 1536);

    for (i = 0 ; i < dwRxRingSize ; i++)
    {
        //  For each descriptor, this is what we need to set:
        //  -   The descriptor is now owned by DEC21140 RDES0[31] = 1
        //  -   RDES1[24] set indicating second address is a chained address,
        //  -   RDES1[25] set for last descriptor indicating a loop back.
        //  -   RDES2 will point to the buffer this descriptor points to.
        //  -   The next descriptor address in TDES3

        pRxHead->RDES0.dwReg = pRxHead->RDES0.dwReg | DESC_OWNED_BY_DEC21140;
        pRxHead->RDES1.dwReg = pRxHead->RDES1.dwReg | SECOND_ADDRESS_CHAINED;
        pRxHead->RDES1.Buffer1Size = MAX_BUFFER_SIZE;

        pRxHead->RDES2 = dwRxBufferStart;

        dwRxBufferStart += MAX_BUFFER_SIZE;

        if (i == dwRxRingSize - 1)
        {
            //  Last descriptor points to first descriptor...
            //  And indicates that this is the end of the ring...
            pRxHead->RDES1.ReceiveEndOfRing = 1;
            pRxHead->RDES3 = TO_REAL((DWORD) pRxDesc);
        }
        else
        {
            pRxHead->RDES3 = TO_REAL((DWORD) ((DWORD)pRxHead + sizeof(RX_DESCRIPTOR_FORMAT)));
            pRxHead = (PRX_DESCRIPTOR_FORMAT) TO_VIRT(pRxHead->RDES3);
        }
    }
    // DumpRxDescriptor (pRxDesc);


    // ==========================================================================
    //  Set CSRs:
    //

    // enable the 21143 chip

    WRITE_PORT_ULONG(CSR13_REG, 0);        //pull it out of sleep  sudhakar
    WRITE_PORT_ULONG(CSR14_REG, 0xffff);
    WRITE_PORT_ULONG(CSR15_REG, 0x1000);
    WRITE_PORT_ULONG(CSR13_REG, 0xef01);

    Delay (1200);

     // CSR0
    localCSR0.dwReg = READ_PORT_ULONG(CSR0_REG);
    localCSR0.ProgrammableBurstLength = 0;      //  16 DWORD transfer in one DMA transaction.
    WRITE_PORT_ULONG(CSR0_REG, localCSR0.dwReg);
    localCSR0.CacheAlignment          = 1;      //  8 DWORD boundary alignment.
    WRITE_PORT_ULONG(CSR0_REG, localCSR0.dwReg);

    //  CSR 3 & CSR4
    WRITE_PORT_ULONG(CSR3_REG, TO_REAL ((DWORD)pRxDesc));
    WRITE_PORT_ULONG(CSR4_REG, TO_REAL ((DWORD)pTxDesc));


    //  CSR6
    localCSR6.dwReg = READ_PORT_ULONG(CSR6_REG);
    localCSR6.ReceiveAll = 1;                   //  Start off seeing everything...
    localCSR6.PromiscuousMode  = 1;
    localCSR6.PassAllMulticast = 0;             //  No multicast...
    localCSR6.FullDuplexMode   = 0;             //  Use half duplex.
    localCSR6.dwReg &= CSR6_MUST_AND;
    localCSR6.dwReg |= CSR6_MUST_OR;
    WRITE_PORT_ULONG(CSR6_REG, localCSR6.dwReg);

    DEC21140ModeSetByAutoNegotiation();//change for 5474
    DEC21140SetupPerfectFilter (NULL, 0);

    //  Turn on Receiver...
    localCSR6.dwReg = READ_PORT_ULONG(CSR6_REG);
    localCSR6.StartReceive = 1;
    WRITE_PORT_ULONG(CSR6_REG, localCSR6.dwReg);
    return TRUE;
}   // HWInit()



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//  Interfaces exported to caller starts here:
//  DEC21140Init()
//  DEC21140InitDMABuffer()
//  DEC21140EnableInts()
//  DEC21140DisableInts()
//  DEC21140ISR()
//  DEC21140GetFrame()
//  DEC21140SendFrame()
//
//

/////////////////////////////////////////////////////////////////////////////////
//  DEC21140Detect - Detect whether there is 21140 compatible chip.
//  Input:
//  -   pbBaseAddress... Caller can pass in either physical or virtual address here.
//                       But have to make sure that it's non cached (if it is virtual).
//
BOOL
DEC21140Detect(BYTE *pbBaseAddress)
{
    //  Need to do proper detection here...

    g_pIOBase = (PUCHAR)pbBaseAddress;

    localDEBUGMSG ("DEC21140Detect()::: returning TRUE\r\n");
    return  TRUE;
}   // DEC21140Detect()



/////////////////////////////////////////////////////////////////////////////////
//  DEC21140Init -  This is called to initialze the ethernet low level driver.
//                  The base address of the ethernet hardware is passed into
//                  the routine.  The routine will return TRUE for a successful
//                  initialization.
//  Initialization:
//  1.  Perform 21140 hardware initialization.
//  2.  Create transmit and receive descriptor lists.
//  3.  Start transmit and receive process on 21140.
//
BOOL
DEC21140Init( UINT8 *pbBaseAddress, UINT32 dwLogicalLocation, UINT16 MacAddr[3])
{
    int         n = 0;
    PBYTE       pDst = NULL;
    PBYTE       pSrc = NULL;


    localDEBUGMSG ("DEC21140::: Init using i/o address : 0x%x - logical location = 0x%x\r\n", pbBaseAddress, dwLogicalLocation);
    g_pIOBase = (PUCHAR)pbBaseAddress;

    // ** NOTE - the method for determining the MAC address is platform
    // ** dependent in the case of built-in controllers.  For plug-in NICs, we
    // ** generally assume the MAC address can be found in a common location.
    // ** Either way, the following code needs to obtain the MAC address to
    // ** be used with the controller.

    if ((PUSHORT)MacAddr != NULL)
    {
        if (!EEPROMReadMAC(&MacAddr[0]))
        {
            localDEBUGMSG("ERROR: Unable to get MAC address for DEC21140.\r\n");
            return(FALSE);
        }

        // Make a copy of the MAC address...
        pSrc = (PBYTE)MacAddr;
        pDst = pbEthernetAddr;
        for (n = 6; n ; n--)
        {
            *pDst++ = *pSrc++;
        }

        localDEBUGMSG ("MAC = %x-%x-%x", pbEthernetAddr[0], pbEthernetAddr[1], pbEthernetAddr[2]);
        localDEBUGMSG ("-%x-%x-%x\r\n",  pbEthernetAddr[3], pbEthernetAddr[4], pbEthernetAddr[5]);
    }
    else
    {
        //localDEBUGMSG("INFO: MAC address to be provided via call to DEC21140SetMACAddress().\r\n");
    }


    // HWInit will return false if hardware is not detected.

    if (!HWInit())
    {
        localDEBUGMSG ("DEC21140:::HWInit() failed...\r\n");
        return FALSE;
    }

    localDEBUGMSG ("DEC21140Init()::: Done.\r\n");
    return TRUE;

}   // DEC21140Init()



////////////////////////////////////////////////////////////////////////////////
//
//  InitTxDescriptor -
//
//  Caller tells us where the descriptors and buffers should live.  Furthermore,
//  the size of buffer is fixed at MAX_BUFFER_SIZE.  It is caller's
//  responsibility to allocate enough memory.  Note: Caller can use
//  DEC21140QueryBufferSize() to find out the size of buffer used for each
//  buffer and hence calculate the Total Buffer that can be allocated...
//  DEC21140QueryDescriptorSize() will return size of descriptor...
//
//  Input:
//  - dwStartAddress = Location where descriptors start.
//  - dwSize         = Size of buffer allocated by caller.
//
//  WARNING: IT IS CALLER RESPONSIBILITY TO GIVE PROPER DATA.
//       NO SANITY CHECK HERE !!!
//       THIS IS ONLY FOR INTERNAL USAGE...
//

void InitTxDescriptor(DWORD dwStartAddress, DWORD dwSize)
{

    // Easy... Simply use the provided addresses and size to fit the maximum
    // number of buffer and descriptors we can...
    // We must have been called from Bootloader or Ethdbg.

    BOOL   bDone = FALSE;
    int    i = 1;
    DWORD  dwCostOfOneBuffer = sizeof(TX_DESCRIPTOR_FORMAT) + MAX_BUFFER_SIZE;

    // Calculate the max number of descriptor + buffer that I can have for the
    // given size.
    while (!bDone)
    {
        if ((i * dwCostOfOneBuffer) > dwSize)
        {
            bDone = TRUE;
            i--;
        }
        else
            i++;
    }

    dwTRANSMIT_DESCRIPTORS_HEAD = dwStartAddress;
    dwTRANSMIT_BUFFER_START = dwStartAddress + i * sizeof(TX_DESCRIPTOR_FORMAT);
    dwTRANSMIT_RING_SIZE = i;

    localDEBUGMSG ("InitTxDescriptor::: dwTRANSMIT_DESCRIPTORS_HEAD = 0x%x...\r\n", dwTRANSMIT_DESCRIPTORS_HEAD);
    localDEBUGMSG ("InitTxDescriptor::: dwTRANSMIT_BUFFER_START     = 0x%x...\r\n", dwTRANSMIT_BUFFER_START);
    localDEBUGMSG ("InitTxDescriptor::: dwTRANSMIT_RING_SIZE        = 0x%x...\r\n", dwTRANSMIT_RING_SIZE);


} // InitTxDescriptor()



////////////////////////////////////////////////////////////////////////////////
//
//  InitRxDescriptor - See InitTxDescriptor()
//
//

void InitRxDescriptor(DWORD dwStartAddress, DWORD dwSize)
{
    // Easy... Simply use the provided addresses and size to fit the maximum
    // number of buffer and descriptors we can...
    // We must have been called from Bootloader or Ethdbg.

    BOOL   bDone = FALSE;
    int    i = 1;
    DWORD  dwCostOfOneBuffer = sizeof(RX_DESCRIPTOR_FORMAT) + MAX_BUFFER_SIZE;

    // Calculate the max number of descriptor + buffer that I can have for the
    // given size.
    while (!bDone)
    {
        if ((i * dwCostOfOneBuffer) > dwSize)
        {
            bDone = TRUE;
            i--;
        }
        else
            i++;
    }

    dwRECEIVE_DESCRIPTORS_HEAD = dwStartAddress;
    dwRECEIVE_BUFFER_START = dwStartAddress + i * sizeof(RX_DESCRIPTOR_FORMAT);
    dwRECEIVE_RING_SIZE = i;


    localDEBUGMSG ("InitRxDescriptor::: dwRECEIVE_DESCRIPTORS_HEAD   = 0x%x...\r\n", dwRECEIVE_DESCRIPTORS_HEAD);
    localDEBUGMSG ("InitRxDescriptor::: dwRECEIVE_BUFFER_START       = 0x%x...\r\n", dwRECEIVE_BUFFER_START);
    localDEBUGMSG ("InitRxDescriptor::: dwRECEIVE_RING_SIZE          = 0x%x...\r\n", dwRECEIVE_RING_SIZE);

}  // InitRxDescriptor()


BOOL DEC21140InitDMABuffer(DWORD dwStartAddress, DWORD dwSize)
{
    DWORD dwAlignedTxStart = 0;
    DWORD dwAlignedRxStart = 0;
    DWORD dwTxBufferSize   = 0;
    DWORD dwRxBufferSize   = 0;

    // We can be called by eboot code or by OAL code - decide who's calling
    // so we can initialize buffer chain with correct addresses.
    if (dwStartAddress & 0x80000000)
        bEbootImage = FALSE;
    else
        bEbootImage = TRUE;

    // Check for minimum buffer size.
    if (dwSize < MIN_DMA_SIZE)
    {
        localDEBUGMSG("ERROR: DMA buffer is too small.\r\n");
        return(FALSE);
    }

    // Determine aligned start address for DMA buffer.
    dwAlignedTxStart = (dwStartAddress + 0x03) & 0xFFFFFFFC;
    dwSize          -= (dwAlignedTxStart - dwStartAddress);

    // Roughly split buffer in two and init Tx and Rx regions.
    dwTxBufferSize   = dwSize / 2;
    dwSize          -= dwTxBufferSize;
    dwStartAddress  += dwTxBufferSize;

    dwAlignedRxStart = (dwStartAddress + 0x03) & 0xFFFFFFFC;
    dwSize          -= (dwAlignedRxStart - dwStartAddress);
    dwRxBufferSize   = dwSize;


    InitTxDescriptor(dwAlignedTxStart, dwTxBufferSize);
    InitRxDescriptor(dwAlignedRxStart, dwRxBufferSize);

    return(TRUE);
}


/////////////////////////////////////////////////////////////////////////////////
//  DEC21140EnableInts -    Interrupts left disabled at init, call this function
//                          to turn them on
//  For Ethernet debug, we only need Receive Interrupt.
//  Hence simply turn on CSR7[6] and CSR7[16]
//
void
DEC21140EnableInts()
{
    CSR7_21140  localCSR7;

    localDEBUGMSG ("DEC21140EnableInts::: Interrupt Enabled...\r\n");
    localCSR7.dwReg = READ_PORT_ULONG(CSR7_REG);
    localCSR7.ReceiveInterruptEnable    = 1;
    localCSR7.NormalIntrSummaryEnable   = 1;
    WRITE_PORT_ULONG(CSR7_REG, localCSR7.dwReg);
}   // DEC21140EnableInts()



/////////////////////////////////////////////////////////////////////////////////
//  DEC21140DisableInts -   Disable interrupt.
//  For etherent debug, we only worry about normal interrupt...
//
void
DEC21140DisableInts()
{
    CSR7_21140  CSR7;

    localDEBUGMSG ("+/- DEC21140DisableInts::: Interrupt Enabled...\r\n");
    CSR7.dwReg = READ_PORT_ULONG(CSR7_REG);
    CSR7.NormalIntrSummaryEnable    = 0;
    WRITE_PORT_ULONG(CSR7_REG, CSR7.dwReg);
}   //  DEC21140DisableInts()





/////////////////////////////////////////////////////////////////////////////////
//  DEC21140GetFrame -  This routine is used to find out if a frame has been
//                      received.  If there are no frames in the RX FIFO, the
//                      routine will return 0.
//                      If there was a frame that was received correctly,
//                      it will be stored in pwData, otherwise it will be discarded.
//
//  Check if the address is broadcast address...
BOOL IsBroadcast(PBYTE pHeader)
{
    int i = 0;
    for (;;)
    {
        if (pHeader[i++] != 0xff)
            break;
        if (i == 6)
            return TRUE;
    }

    return FALSE;
}   // IsBroadcast()


void PrintMAC (BYTE bData)
{
    if (bData < 10)
        localDEBUGMSG ("0");

    //localDEBUGMSG ("%x", bData);
    localDEBUGMSG ("%x", bData);
}   // PrintMAC



/////////////////////////////////////////////////////////////////////////////////
//  Not until SP1 that the problem in talking to DHCP server that does not reply
//  discovery packet with ARP broadcast is solved.
//  For altoona, the solution is to differentiate between a kernel call and
//  bootloader call.
//  We pass all broadcast packets up if it is bootloader.
//  Bootloader has to call this function with bBootLoaderCall == TRUE

UINT16
DEC21140GetFrame(BYTE *pbData, UINT16 *pwLength)
{
    RX_RDES0    localRDES0;
    BOOL        bMultiBufferDetected = FALSE;
    PBYTE       pHeader;
#if 0 
    PBYTE       pbOrig = pbData;
    BOOL        bBootLoaderCall = FALSE;
#endif
    CSR5_21140  CSR5;

    //  localDEBUGMSG ("+/- DEC2114GetFrame:::...\r\n");
    *pwLength = 0;

    //  Bail out immediately if the target descriptor is owned by DEC21140
    if (pCurrentRxDesc->RDES0.OwnBit == 1)
        return(0);

    // KITL doesn't call GetPendingInts to clear the interrupt at the card
    // therefore we need to do it here.
    //
    CSR5.dwReg = READ_PORT_ULONG(CSR5_REG);
    if (CSR5.ReceiveInterrupt)
    {
        //      Clear the interrupt...
        CSR5.ReceiveInterrupt = 1;
        WRITE_PORT_ULONG(CSR5_REG, CSR5.dwReg);
    }

    //  A chance of getting valid packet...
    //  Need to bail out if we wait too long for a frame completion...

    while (pCurrentRxDesc->RDES0.OwnBit == 0 || bMultiBufferDetected)
    {
        localRDES0 = pCurrentRxDesc->RDES0;
        //  localDEBUGMSG ("DEC21140GetFrame::: Looking at descriptor: %d... RDES0 = 0x%x\r\n",
        //      (pCurrentRxDesc - pRxDesc), localRDES0.dwReg);

        //  Discard Error Frame for now...   Donno what to do to them.
        //  Error frame detected if RDES0[15] and RDES0[8] are set.
        //  In this case, we simply clean up the Descriptor and release it back to 21140.
        if (localRDES0.ErrorSummary && localRDES0.LastDescriptor)
        {
            //  localDEBUGMSG ("DEC21140GetFrame::: Error Frame !!!   1 Buffer Tossed...\r\n");
            localRDES0.dwReg = 0x00 | DESC_OWNED_BY_DEC21140;
            pCurrentRxDesc->RDES0.dwReg = localRDES0.dwReg;

            //  Proceed to next descriptor...
            pCurrentRxDesc = (PRX_DESCRIPTOR_FORMAT) TO_VIRT (pCurrentRxDesc->RDES3);
            continue;
        }


        //  Okay, by now we must have a valid buffer.
        //  Copy data pointed to by this buffer to user buffer...

        //  localDEBUGMSG ("DEC21140GetFrame()::: Copying %d bytes\r\n", (UINT16)localRDES0.FrameLength);

        //  For Ethernet Debug and Bootloader, only ARP packet is accepted...
        //  Hence filter out non-ARP broadcast or Multicast packets...

        /////////////////////////////////////////////////////////////////////////
        //  Note: MULTICAST is ENABLED NOW. WE'LL PASS ALL FRAMES BACK

        pHeader = (PBYTE) TO_VIRT(pCurrentRxDesc->RDES2);
#if 0
        if (localRDES0.MulticastFrame || IsBroadcast(pHeader))
        {
            //  This is multicast or broadcast packet.
            //  Toss away if it is non-ARP (i.e Header[16] != 8  || Header[17] != 6)


            if (!bBootLoaderCall)
            {
                if (pHeader[12] != 0x08 && pHeader[13] != 0x06)
                {
                    //  The buffer is tossed !!!
                        //localDEBUGMSG ("DEC21140GetFrame()::: 1 non ARP broadcast/multicast buffer tossed.\r\n");
                        //localDEBUGMSG ("DEC21140GetFrame()::: RDES0 = 0x%x\r\n", localRDES0);
                    goto SkipCopy;
                }
            }
        }
#endif

        *pwLength = *pwLength + (UINT16)localRDES0.FrameLength;
        memcpy (pbData, (PBYTE) TO_VIRT(pCurrentRxDesc->RDES2), (UINT16)localRDES0.FrameLength);
        pbData += (UINT16)localRDES0.FrameLength;


//SkipCopy:
        //  Return descriptor back to 21140...
        pCurrentRxDesc->RDES0.dwReg = 0x00 | DESC_OWNED_BY_DEC21140;

        //  Proceed to next descriptor...
        pCurrentRxDesc = (PRX_DESCRIPTOR_FORMAT) TO_VIRT (pCurrentRxDesc->RDES3);

        //  Check the First and Last descriptor indications and take action accordingly...
        switch ((localRDES0.dwReg & 0x300) >> 8)
        {
            /////////////////////////////////////////////////////////////////////
            case 0x00:  // Not first and not last...
                //localDEBUGMSG ("DEC21140GetFrame()::: Middle buffer...\r\n");
                break;


            /////////////////////////////////////////////////////////////////////
            case 0x01:  // Last Descriptor...
                if (bMultiBufferDetected)
                {
                    //localDEBUGMSG ("DEC21140GetFrame()::: Last Descriptor found...length = %d\r\n", *pwLength);

                                        // Remove checksum from the length
                                        // returned...
                                        //
                                        if (*pwLength)
                                            *pwLength -= sizeof(ULONG);

                    return(*pwLength);
                }
                else
                {
                    //  Must be an error, toss the whole buffer..
                    return(0);
                }

            /////////////////////////////////////////////////////////////////////
            case 0x02:  //  First Descriptor...
                if (!bMultiBufferDetected)
                {
                    //localDEBUGMSG ("DEC21140GetFrame()::: First Descriptor detected...\r\n");
                    bMultiBufferDetected = TRUE;
                }
                else
                {
                    //  Another First Descriptor ???
                    //  Must be something wrong !!!
                    //  localDEBUGMSG ("DEC21140GetFrame()::: Multiple First Descriptor...Tossing all...\r\n");
                    return(0);
                }
                break;


            /////////////////////////////////////////////////////////////////////
            case 0x03:  //  One buffer frame...
                // localDEBUGMSG ("DEC21140GetFrame()::: One buffer frame (%d bytes)...\r\n", *pwLength);

                //  Let's see the destination MAC...
                //  Destination MAC starts from 8th byte.

                {
                    static  DWORD   dwTotalFrame = 0;

                    //localDEBUGMSG (" ... Total = %d\r\n", dwTotalFrame++);
                }

                               // Remove checksum from the length
                               // returned...
                               //
                               if (*pwLength)
                                   *pwLength -= sizeof(ULONG);
#if 0
                    if ((*pwLength > 0) && pbOrig[0] && (pbOrig[0] != 0xff))
                    {
                    localDEBUGMSG ("Dest MAC = ");
                    PrintMAC (pbOrig[0]);
                    PrintMAC (pbOrig[1]);
                    PrintMAC (pbOrig[2]);
                    PrintMAC (pbOrig[3]);
                    PrintMAC (pbOrig[4]);
                    PrintMAC (pbOrig[5]);
                    localDEBUGMSG ("\r\n");
                    }
#endif
                return(*pwLength);

            ///////////////////////////////////////////////////////
            default:
                //  Should NEVER get here..
                //  Just bail out...
                return(0);
        }
    }

    // Remove checksum from the length
    // returned...
    //
    if (*pwLength)
        *pwLength -= sizeof(ULONG);
#if 0
    if (*pwLength && (pbOrig[0] != 0xff) && (pbOrig[0])) {
        localDEBUGMSG ("Dest MAC2 = ");
        PrintMAC (pbOrig[0]);
        PrintMAC (pbOrig[1]);
        PrintMAC (pbOrig[2]);
        PrintMAC (pbOrig[3]);
        PrintMAC (pbOrig[4]);
        PrintMAC (pbOrig[5]);
        localDEBUGMSG ("\r\n");
        }
#endif
    return(*pwLength);

}   // DEC21140GetFrame()



/////////////////////////////////////////////////////////////////////////////////
//  DEC21140ModeSetByAutoNegotiation()
//  Set the DEC to the common denominator between what PHY (ICS1890) has negotiated
//  with "link partner" and capability of DEC.
//
void    DEC21140ModeSetByAutoNegotiation()
{
    WORD    wReg4;
    WORD    wReg5;
    WORD    wModes;
    DWORD   WaitCount;
    CSR6_21140  CSR6;
    CSR0_21140  CSR0;


#if DEC21140_HAS_MII
    WaitCount = 100000;
    while ((!(MIIRead(0x01) & 0x20)) && ( 0 != WaitCount-- ))
    {
        //  Wait till Autonegotiation completed...
        //  printf ("Autonegotiation not completed...\r\n");
    }
#endif

    //  Make sure TX and Rx are stopped.
    CSR6.dwReg = READ_PORT_ULONG(CSR6_REG);
    CSR6.StartTransmit = 0x0;
    CSR6.StartReceive  = 0x0;
    WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);

    //  Default to 10Base-TxHD.
    CSR6.TransmitThresholdMode = 0x01;      //  FIFO threshold to 10Mbps
    CSR6.FullDuplexMode    = 0x00;      //  Set half duplex mode.
    CSR6.StoreAndForward       = 0x01;      //  Store/Forward...
    WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);

    wModes = 0;

#if DEC21140_HAS_MII
    //  If autonegotiation was successful, get the communications
    //  that are supported by the local host and the remote host.
    if ( 0 != WaitCount )
    {
        wReg4 = MIIRead(0x04);
        wReg5 = MIIRead(0x05);

        //  Determine the common modes of operation.
        wModes = wReg4 & wReg5;
    }

    //  I know that ICS1890 does not support 100Base-T4
    //  Hence we start check for
    //  100Base-TxFD
    //  100Base-TxHD
    //  10Base-TxFD
    //  10Base-TxHD

    //  Enable 100Base-Tx if supported by both sides.
    if (( wModes & 100 ) || ( wModes & 0x80 ))
    {
        CSR0.dwReg = READ_PORT_ULONG(CSR0_REG);
        CSR0.ProgrammableBurstLength    = 32;
        WRITE_PORT_ULONG(CSR0_REG, CSR0.dwReg);

        CSR6.dwReg = READ_PORT_ULONG(CSR6_REG);
        CSR6.TransmitThresholdMode      = 0x00;     //  FIFO threshold to 100Mbps
        WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);

        //  Set bit 0 if full duplex is enabled.
        wModes >>= 2;
    }
    wModes >>= 6;

    //  Enable full duplex if supported by both sides.
    if ( wModes & 1 )
    {
        CSR6.dwReg = READ_PORT_ULONG(CSR6_REG);
        CSR6.FullDuplexMode             = 0x01;     //  Set Full duplex mode.
        WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);
    }
#else // The following code assumes a 100TX PHY.
    wReg4; // Unused.
    wReg5; // Unused.
    WaitCount; // Unused.
    CSR0.dwReg = READ_PORT_ULONG(CSR0_REG);
    CSR0.ProgrammableBurstLength    = 32;
    WRITE_PORT_ULONG(CSR0_REG, CSR0.dwReg);

    CSR6.dwReg = READ_PORT_ULONG(CSR6_REG);

    // PCS and scrambler enabled for 100TX
    CSR6.ScrambleMode = 1;
    CSR6.PCSfunction = 1;
    CSR6.TransmitThresholdMode      = 0;     //  FIFO threshold to 100Mbps
    CSR6.PortSelect = 1; // Use SYM port.
    CSR6.StoreAndForward = 1;
    CSR6.FullDuplexMode             = 0x01;     //  Set Full duplex mode.
    WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);
#endif
}


////////////////////////////////////////////////////////////////////////////////
//
// DEC21140SetupPerfectFilter (BYTE *pbPermittedAddress)
// This function creates a setup frame to be used to filter all incoming
// frames.  A perfect filter filters out everything except its own IP address
// and broadcast address.
//
BOOL DEC21140SetupPerfectFilter (PUCHAR pucMulticastAddresses, DWORD dwNoOfAddresses)
{
    // Here are the steps taken...
    // 1.  Prepare a zero-length buffer descriptor from current tx descriptor.
    // 2.  Setup the perfect filter frame with exactly 192 bytes data length.
    // 3.  First Segment = Last Segment = 0.
    // 4.  Finally, set TDES0[31] = 1 ... Adapter owned descriptor.
    //
    //     Zero length descriptor needs:
    //     TDES1[30] = 0 ... Last Segment Bit 0
    //     TDES1[29] = 0 ... First Segment Bit 0
    //     TDES1[21..11] = ... Transmit buffer 2 = 0
    //     TDES1[10..0]  = ... Transmit buffer 1 = 0
    //
    // 5.  Trigger transmitter, wait for packet 'eaten' by DEC.
    // 6.  Reset descriptor to become normal descriptor.


    PTX_DESCRIPTOR_FORMAT       pSetupDescriptor = NULL;
    PTX_DESCRIPTOR_FORMAT       pZeroLengthDescriptor = NULL;
    TX_TDES1                    ZeroLengthDescriptorOriginTDES1;
    TX_TDES1                    SetupDescriptorOriginTDES1;
    PBYTE                       pbBuffer = NULL;
    CSR6_21140                  CSR6;
    CSR0_21140                  CSR0;
    DWORD                       i;

    //
    // for multicast: first two entries reserved for Local MAC address and broadcast address
    //
    if (DEC21140_SETUP_PERFECT_ENTRIES-2 < dwNoOfAddresses) {
        return FALSE;
    }

    if (!pucMulticastAddresses) {
        //
        // Initial Setting -- make sure receiver is off and transmitter is on.
        //
        CSR6.dwReg = READ_PORT_ULONG(CSR6_REG);
        CSR6.StartReceive  = 0;
        CSR6.StartTransmit = 1;
        WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);

        CSR0.dwReg = READ_PORT_ULONG(CSR0_REG);
        CSR0.OnNow = 0;
        WRITE_PORT_ULONG(CSR0_REG, CSR0.dwReg);
    }

    // Setup zero length buffer descriptor.
    //
    pZeroLengthDescriptor  = pCurrentTxDesc;

    /* Not necessary, and causes problems for Virtual PC.
    pCurrentTxDesc     = (PTX_DESCRIPTOR_FORMAT) TO_VIRT(pCurrentTxDesc->TDES3);
    */
    pSetupDescriptor       = pCurrentTxDesc;
    pCurrentTxDesc     = (PTX_DESCRIPTOR_FORMAT) TO_VIRT(pCurrentTxDesc->TDES3);

    ZeroLengthDescriptorOriginTDES1.dwReg = pZeroLengthDescriptor->TDES1.dwReg;
    pZeroLengthDescriptor->TDES1.dwReg    = 0x09000000;
    pZeroLengthDescriptor->TDES0.OwnBit   = 1;

    /* Not necessary, and causes problems for Virtual PC.
    CSR6.dwReg = READ_PORT_ULONG(CSR6_REG);
    CSR6.StartTransmit = 1;
    WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);
    WRITE_PORT_ULONG(CSR1_REG, 0xffffffff); // Trigger transmitter...

    localDEBUGMSG ("Waiting for Zero Length Descriptor...\r\n");
    while ((pZeroLengthDescriptor->TDES0.OwnBit) == 1)
    {
        WRITE_PORT_ULONG(CSR1_REG, 0xffffffff);
    }
    */

    // Restore the original TDES1 descriptor.
    //
    pZeroLengthDescriptor->TDES1 = ZeroLengthDescriptorOriginTDES1;


    // Now, setup the perfect filter packet...
    //

    // Store original TDES1 value to restore later.
    //
    SetupDescriptorOriginTDES1.dwReg = pSetupDescriptor->TDES1.dwReg;

    //Store this into descriptor and it is ready to go...
    //
    pSetupDescriptor->TDES1.dwReg = (0x69000000 | 192);

    pbBuffer = (PVOID)TO_VIRT(pSetupDescriptor->TDES2);
    memset (pbBuffer, 0xff, 500);
    pbBuffer[0] = pbEthernetAddr[0];
    pbBuffer[1] = pbEthernetAddr[1];
    pbBuffer[4] = pbEthernetAddr[2];
    pbBuffer[5] = pbEthernetAddr[3];
    pbBuffer[8] = pbEthernetAddr[4];
    pbBuffer[9] = pbEthernetAddr[5];

    //
    // setup the rest of the addresses
    //
    for (i = 0; i < dwNoOfAddresses; i ++) {
        pbBuffer[(i+2)*12]   = pucMulticastAddresses[i*6];
        pbBuffer[(i+2)*12+1] = pucMulticastAddresses[i*6+1];
        pbBuffer[(i+2)*12+4] = pucMulticastAddresses[i*6+2];
        pbBuffer[(i+2)*12+5] = pucMulticastAddresses[i*6+3];
        pbBuffer[(i+2)*12+8] = pucMulticastAddresses[i*6+4];
        pbBuffer[(i+2)*12+9] = pucMulticastAddresses[i*6+5];
    }

    // We are ready to go...
    //
    pSetupDescriptor->TDES0.OwnBit = 1;

    WRITE_PORT_ULONG(CSR1_REG, 0xffffffff);

    // Now we just wait till it gobbles up the packets...
    //
    localDEBUGMSG ("Waiting for DEC21140 to perform perfect filtering...\r\n");

    while ((pSetupDescriptor->TDES0.OwnBit) == 1)
        WRITE_PORT_ULONG(CSR1_REG, 0xffffffff);


    // Finally, restore the original TDES1 for the setupdescriptor so that
    // next packet won't be treated as setup packet.
    //
    pSetupDescriptor->TDES1.dwReg = SetupDescriptorOriginTDES1.dwReg;

    if (!pucMulticastAddresses) {
        //
        // Initial setup: turn off promiscuous mode
        //
        CSR6.dwReg = READ_PORT_ULONG(CSR6_REG);
        CSR6.PromiscuousMode  = 0;
        CSR6.ReceiveAll = 0;
        WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);
    }
    return(TRUE);
}


/////////////////////////////////////////////////////////////////////////////////
//  DEC21140SendFrame - This routine should be called with a pointer to the
//                      ethernet frame data.  It is the caller's responsibility
//                      to fill in all information including the destination and
//                      source addresses and the frame type.
//                      The length parameter gives the number of bytes in
//                      the ethernet frame.
//                      The routine will return immediately regardless of
//                      whether transmission has successfully been performed by
//                      21140.
//  Error return:   Non zero.
//  No error:       Returns zero.
//

UINT16 DEC21140SendFrame( BYTE *pbData, DWORD dwLength )
{
    DWORD                   i = 0;
    DWORD                   dwTotalSent = 0;
    DWORD                   dwNumberOfDescriptorUsed = 0;
    DWORD                   dwToSend;
    CSR6_21140              CSR6;

    volatile PTX_DESCRIPTOR_FORMAT  pFirstTxDescriptor;
    volatile PTX_DESCRIPTOR_FORMAT  pRememberingTheVeryFirstDescriptor = NULL;


    pFirstTxDescriptor  = (PTX_DESCRIPTOR_FORMAT) NULL;

    //printf ("DEC21140SendFrame()::: Sending %d bytes \r\n", dwLength);

    //  localDEBUGMSG ("DEC21140SendFrame()::: Sending : %d bytes\r\n", dwLength);

    //  Need time out...   When all TX process of dec21140 STALL !!!
    while (dwTotalSent < dwLength)
    {
        if (pCurrentTxDesc->TDES0.dwReg & ~DESC_OWNED_BY_HOST)
        {
            //  Do something here to make sure the TX process get rid of all these packets...
            //  Then back to top of loop...

            continue;
        }

        if (pCurrentTxDesc == pFirstTxDescriptor)
        {
            //  This should NEVER happen if we have plenty of descriptors...
            //  We'll get into this loop only if we keep getting the descriptor in this while...

            //  localDEBUGMSG ("DEC21140SendFrame()::: Error Tx Loop Back...\r\n");

            //  Just forget about the whole sending and reset current descriptor to the first one...
            pCurrentTxDesc = pFirstTxDescriptor;
            return  DEC21140_ERROR_TX_LOOPBACK;
        }


        // Remember the first descriptor...
        if (pFirstTxDescriptor == (PTX_DESCRIPTOR_FORMAT) NULL)
            pFirstTxDescriptor = pCurrentTxDesc;

        dwNumberOfDescriptorUsed++;

        if ((dwLength - dwTotalSent) > MAX_BUFFER_SIZE)
            dwToSend = MAX_BUFFER_SIZE;
        else
            dwToSend = dwLength - dwTotalSent;


        //localDEBUGMSG ("DEC21140SendFrame()::: Obtained descriptor %d - Filling: %d bytes.\r\n",
        //  (pCurrentTxDesc - pTxDesc), dwToSend);

        memcpy ((PVOID)TO_VIRT(pCurrentTxDesc->TDES2), pbData + dwTotalSent, dwToSend);

        pCurrentTxDesc->TDES1.Buffer1Size = dwToSend;

        dwTotalSent += dwToSend;

        // Advance to next descriptor...
        pCurrentTxDesc = (PTX_DESCRIPTOR_FORMAT) TO_VIRT(pCurrentTxDesc->TDES3);
    }

    //printf ("NumberOfDescriptorUsed = %d\r\n", dwNumberOfDescriptorUsed);

    if (dwNumberOfDescriptorUsed < 1) {
        // No descriptors, return error
        return  DEC21140_ERROR_TX_LOOPBACK;
    }

    //  Now, let's update the descriptors...
    while (i++ < dwNumberOfDescriptorUsed)
    {
        //  If it is first segment, mark it, otherwise unmarked it.
        //  This descriptor may have been used in previous sending...
        if (i == 1)
            pFirstTxDescriptor->TDES1.FirstSegment = 1;
        else
            pFirstTxDescriptor->TDES1.FirstSegment = 0;

        //  Ditto for last segment.
        if (i == dwNumberOfDescriptorUsed)
            pFirstTxDescriptor->TDES1.LastSegment = 1;
        else
            pFirstTxDescriptor->TDES1.LastSegment = 0;


        //  Let 21140 owns it now...   It will reset this when the buffer has been
        //  successfully sent...
        if (i == 1)
        {
            //  Don't release the descriptor yet until the complete frame is done.
            pRememberingTheVeryFirstDescriptor = pFirstTxDescriptor;
        }
        else
            pFirstTxDescriptor->TDES0.dwReg |= ~DESC_OWNED_BY_HOST;

        // Advance to next descriptor...
        pFirstTxDescriptor = (PTX_DESCRIPTOR_FORMAT) TO_VIRT(pFirstTxDescriptor->TDES3);
    }


    //  Rock n Roll time...
    pRememberingTheVeryFirstDescriptor->TDES0.dwReg |= ~DESC_OWNED_BY_HOST;
    CSR6.dwReg = READ_PORT_ULONG(CSR6_REG);
    CSR6.StartTransmit = 1;
    WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);
    WRITE_PORT_ULONG(CSR1_REG, 0xffffffff);


    /*
    for (;;)
    {
        int i = 1000000;

        printf ("TDES0 = 0x%x \r\n", pRememberingTheVeryFirstDescriptor->TDES0.dwReg);
        while (i--)
            ;


    }
    */


    return 0x00;
}



/////////////////////////////////////////////////////////////////////////////////
//  DEC21140QueryBufferSize() & DEC21140QueryDescriptorSize()
//  Caller can use these functions to figure out how many descriptors and buffer
//  that can fit into its budget.
//  DEC21140QueryBufferSize() simply returns the constant used by this libary in
//  determining the size of each buffer.
//  DEC21140QueryDescriptorSize() will return size of TX_DESCRIPTOR_FORMAT.
//  These two dimensions can change, and caller should make sure that it uses these
//  functions in determining the number of descriptor that the library can use.
//

DWORD   DEC21140QueryBufferSize (void)
{
    return MAX_BUFFER_SIZE;
}   // DEC21140QueryBufferSize()



DWORD   DEC21140QueryDescriptorSize (void)
{
    return  sizeof(TX_DESCRIPTOR_FORMAT);
}   // DEC21140QueryDescriptorSize()




/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//  Misc Utility functions:
//  DumpTxDescriptor()
//  DumpRxDescriptor()
//  DumpMemory()
//

/////////////////////////////////////////////////////////////////////////////////
//  DumpTxDescriptor()
//
#ifdef  __DEC21140_DUMP_TX_DESCRIPTOR__
static void DumpTxDescriptor (PTX_DESCRIPTOR_FORMAT pTxHead)
{
    PTX_DESCRIPTOR_FORMAT   pTx;
    int                     dwCount = 0;

    pTx = pTxHead;


    localDEBUGMSG ("+--------  Dumping TX descriptor  --------+\r\n");
    localDEBUGMSG ("pTxHead = 0x%x \r\n", pTxHead);

    for (;;)
    {
        localDEBUGMSG ("%d ---> ", dwCount++);
        localDEBUGMSG ("Addr = 0x%x | ", pTx);
        localDEBUGMSG ("T0 = 0x%x |", pTx->TDES0.dwReg);
        localDEBUGMSG ("T1 = 0x%x |", pTx->TDES1.dwReg);
        localDEBUGMSG ("T2 = 0x%x |", pTx->TDES2);
        localDEBUGMSG ("T3 = 0x%x \r\n", pTx->TDES3);

        if (TO_VIRT(pTx->TDES3) == (DWORD) pTxHead)
        {
            //  This must be the last descriptor before we loop back...
            break;
        }
        else
            pTx = (PTX_DESCRIPTOR_FORMAT) TO_VIRT(pTx->TDES3);



    }
}
#endif // __DEC21140_DUMP_TX_DESCRIPTOR__



/////////////////////////////////////////////////////////////////////////////////
//  DumpRxDescriptor()
//
#ifdef  __DEC21140_DUMP_RX_DESCRIPTOR__
static void DumpRxDescriptor (PRX_DESCRIPTOR_FORMAT pRxHead)
{
    PRX_DESCRIPTOR_FORMAT   pRx;
    int                     dwCount = 0;

    pRx = pRxHead;


    localDEBUGMSG ("+--------  Dumping RX descriptor  --------+\r\n");
    localDEBUGMSG ("pRxHead = 0x%x \r\n", pRxHead);

    for (;;)
    {
        localDEBUGMSG ("%d ---> ", dwCount++);
        localDEBUGMSG ("Addr = 0x%x | ", pRx);
        localDEBUGMSG ("R0 = 0x%x |", pRx->RDES0.dwReg);
        localDEBUGMSG ("R1 = 0x%x |", pRx->RDES1.dwReg);
        localDEBUGMSG ("R2 = 0x%x |", pRx->RDES2);
        localDEBUGMSG ("R3 = 0x%x \r\n", pRx->RDES3);

        if (TO_VIRT(pRx->RDES3) == (DWORD) pRxHead)
        {
            //  This must be the last descriptor before we loop back...
            break;
        }
        else
            pRx = (PRX_DESCRIPTOR_FORMAT) TO_VIRT(pRx->RDES3);
    }
}   // DumpRxDescriptor()
#endif  //  __DEC21140_DUMP_RX_DEXCRIPTOR__


/////////////////////////////////////////////////////////////////////////////////
//  DumpMemory()
//
#ifdef  __DEC21140_DUMP_MEMORY__

//localDEBUGMSG ("0x%x\t", *pSource++);

static void DEC21140DisplayHex (BYTE data)
{
    localDEBUGMSG ("0x");
    if (data < 0x10)
        localDEBUGMSG ("0");
    localDEBUGMSG ("%x ", data);
}   // DEC21140DisplayHex()


void DumpMemory (PBYTE  pSource, DWORD  dwLength)
{
    int     i = 0;


    localDEBUGMSG ("+---- MEM DUMP ----+\r\n");
    localDEBUGMSG ("0x%x: ", pSource);
    while (dwLength--)
    {
        DEC21140DisplayHex (*pSource++);
        if (++i == 16)
        {
            i = 0;
            localDEBUGMSG ("\r\n0x%x: ", pSource);
        }

    }
    localDEBUGMSG ("\r\n\r\n");
}   // DumpMemory()
#endif  //  __DEC21140_DUMP_MEMORY__


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
WORD
EEPROMReadWord( BYTE bAddress )
{
    UCHAR nCnt = 0;
    CSR9_21140 localCSR9;
    USHORT nVal = 0;

    localCSR9.dwReg = 0;

    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);    // Clear.
    Delay(100);

    localCSR9.SerialRomSelect = 1;          // Read from SROM.
    localCSR9.ReadOperation   = 1;
    localCSR9.MiiManagementOperationMode = 1;

    localCSR9.BootRomDataOrSerialRomCtrl = 0x001;   // Setup.
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(100);

    localCSR9.BootRomDataOrSerialRomCtrl = 0x003;
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(1000);

    localCSR9.BootRomDataOrSerialRomCtrl = 0x001;
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(1000);

    localCSR9.BootRomDataOrSerialRomCtrl = 0x005;   // Command phase.
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(1000);

    localCSR9.BootRomDataOrSerialRomCtrl = 0x007;
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(1000);

    localCSR9.BootRomDataOrSerialRomCtrl = 0x005;
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(1000);

    localCSR9.BootRomDataOrSerialRomCtrl = 0x007;
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(1000);

    localCSR9.BootRomDataOrSerialRomCtrl = 0x005;
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(1000);

    localCSR9.BootRomDataOrSerialRomCtrl = 0x001;
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(1000);

    localCSR9.BootRomDataOrSerialRomCtrl = 0x003;
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(1000);

    localCSR9.BootRomDataOrSerialRomCtrl = 0x001;
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(1000);

    // Address phase.
    for (nCnt = 0 ; nCnt < 6 ; nCnt++)  // 6-bit address
    {
        nVal = (USHORT)(((bAddress << (nCnt + 2)) & 0x80) >> 5);


        localCSR9.BootRomDataOrSerialRomCtrl = (0x001 | nVal);
        WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
        Delay(1000);

        localCSR9.BootRomDataOrSerialRomCtrl = (0x003 | nVal);
        WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
        Delay(1000);

        localCSR9.BootRomDataOrSerialRomCtrl = (0x001 | nVal);
        WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
        Delay(1000);
    }


    Delay(1000);

    // Data phase.
    for (nCnt = 0, nVal = 0 ; nCnt < 16 ; nCnt++)  // Data is 16-bits wide.
    {

        localCSR9.BootRomDataOrSerialRomCtrl = 0x003;
        WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
        Delay(1000);

        nVal |= ((READ_PORT_ULONG(CSR9_REG) & 0x00000008) >> 3);
        if (nCnt < (16 - 1))
            nVal = nVal << 1;
        Delay(1000);

        localCSR9.BootRomDataOrSerialRomCtrl = 0x001;
        WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
        Delay(1000);
    }

    localCSR9.BootRomDataOrSerialRomCtrl = 0x000;  // Complete transaction.
    WRITE_PORT_ULONG(CSR9_REG, localCSR9.dwReg);
    Delay(1000);

localDEBUGMSG("EEPROMReadWord: Address=0x%x  Data=0x%x\r\n", bAddress, nVal);

    return(nVal);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOL
EEPROMReadMAC(WORD *wMAC)
{

    int i = 0;

    for (i = 0; i < 3; i++)
    {
        wMAC[i] = EEPROMReadWord((BYTE)(i + 10));  // MAC address is 20 bytes
                                                   // (10 words) into SROM.
    }

    return(TRUE);
}


BOOL
DEC21140SetMACAddress(PUSHORT pMacAddr)
{
    int n = 0;
    PBYTE pDst = NULL;
    PBYTE pSrc = NULL;

    // Make a copy of the MAC address...
    pSrc = (PBYTE)pMacAddr;
    pDst = pbEthernetAddr;
    for (n = 6; n ; n--)
    {
        *pDst++ = *pSrc++;
    }

        localDEBUGMSG ("INFO: Set DEC21140 MAC to %x-%x-%x", pbEthernetAddr[0], pbEthernetAddr[1], pbEthernetAddr[2]);
        localDEBUGMSG ("-%x-%x-%x\r\n",  pbEthernetAddr[3], pbEthernetAddr[4], pbEthernetAddr[5]);

    return(TRUE);
}

void DEC21140CurrentPacketFilter(DWORD dwFilter)
{
    CSR6_21140 CSR6;


    // read the original value of CSR6
    CSR6.dwReg = READ_PORT_ULONG(CSR6_REG);

    // update the bits of interest
    CSR6.PassAllMulticast = (dwFilter & PACKET_TYPE_ALL_MULTICAST)? 1 : 0;
    CSR6.PromiscuousMode = (dwFilter & PACKET_TYPE_PROMISCUOUS)? 1 : 0;
    // ignore other filter settings.

#if 0
    localDEBUGMSG ("\nINFO: dwFilter = %s | %s, CSR6 = 0x%x\n",
        (dwFilter & PACKET_TYPE_ALL_MULTICAST)? "ALLMULTICAST" : "NONE",
        (dwFilter & PACKET_TYPE_PROMISCUOUS)? "PROMISCUOUS" : "NONE",
        CSR6.dwReg
        );
#endif
    // update CSR6
    WRITE_PORT_ULONG(CSR6_REG, CSR6.dwReg);
}

BOOL DEC21140MulticastList (PUCHAR pucMulticastAddresses, DWORD dwNoOfAddresses)
{
#if 0
   localDEBUGMSG ("INFO: pucMulticastAddresses = 0x%x | dwNoOfAddresses = %d\n", pucMulticastAddresses, dwNoOfAddresses);
#endif
   return pucMulticastAddresses? DEC21140SetupPerfectFilter(pucMulticastAddresses, dwNoOfAddresses) : FALSE;
}


