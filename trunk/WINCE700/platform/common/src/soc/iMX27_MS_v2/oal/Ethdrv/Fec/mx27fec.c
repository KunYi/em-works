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
//------------------------------------------------------------------------------
// Copyright (C) 2004-2006,2007 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: MX27FEC.c
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <ceddk.h>
#include <ethdbg.h>
#include <oal.h>
#include "csp.h"
#include "Mx27FEC.h"

//------------------------------------------------------------------------------
// Defines
#define    FEC_RX_DMA_RING_SIZE    4
#define    FEC_TX_DMA_RING_SIZE    4
#define    FEC_DMA_RX_FRSIZE       2048
#define    FEC_DMA_TX_FRSIZE       2048
#define    PKT_MAXBLR_SIZE         1520
#define    PKT_MAXBUF_SIZE         1518
//------------------------------------------------------------------------------
// Types
typedef struct
{
    PCSP_FEC_REGS REG;

    UINT32    MIIPhyAddr;

    // virtual address and physical address for BDs
    PBYTE         RingBase;
    UINT32        RingPhysicalBase;

    // virtual address for receive and transmit buffers
    PVOID               RxBufferBase;
    PVOID               TxBufferBase;

    PBUFFER_DESC    RxBufferDescBase;
    PBUFFER_DESC    TxBufferDescBase;

    PUCHAR  RxBuffAddr[FEC_RX_DMA_RING_SIZE];
    PUCHAR  TxBuffAddr[FEC_TX_DMA_RING_SIZE];

    PBUFFER_DESC    CurrentRx;
    PBUFFER_DESC    CurrentTx;

    UINT32   hash[2];
    UINT32   filter;

}FEC;
//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables
static FEC g_FEC;

//------------------------------------------------------------------------------
// Local Functions

// External Variables

//-----------------------------------------------------------------------------
// External Functions
extern void KITLOutputDebugString (LPCSTR sz, ...);
extern UINT32 OALGetTickCount(void);
extern void OALKitlFECBSPInit(void);
extern void InitFecPhysical(void);

//------------------------------------------------------------------------------
//
// Function: FECClockSetGatingMode
//
// Sets the clock gating mode of the peripheral.
//
// Parameters:
//      index
//          [in] Index for referencing peripheral clock gating control bits.
//      mode
//          [in] Requested clock gating mode for the peripheral.
//
// Returns:
//      Returns TRUE if the clock gating mode was set successfully, otherwise
//      returns FALSE.
//
//------------------------------------------------------------------------------
BOOL FECClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode)
{
    UINT32 pccrValOld, pccrValNew, pccrIndex, pccrShift, pccrMask;
    PCSP_PLLCRC_REGS pPLLCRC;

    //Map access to CRM
    pPLLCRC = (PCSP_PLLCRC_REGS)OALPAtoUA(CSP_BASE_REG_PA_CRM);

    // Divide clock gating index by 32 to determine PCCR register index
    pccrIndex = index >> 5;

    // Calculate shift for PCCR bits by using the index
    pccrShift = index % 32;

    // Calculate bitmask for PCCR bits
    pccrMask = PLLCRC_PCCR_CG_MASK  << pccrShift;

    // Keep trying until the shared register access to the PCCR succeeds
    do {
        pccrValOld = INREG32(&pPLLCRC->PCCR0 + pccrIndex);
        pccrValNew = (pccrValOld & (~pccrMask)) | (mode << pccrShift);
        OUTREG32(&pPLLCRC->PCCR0 + pccrIndex, pccrValNew);
    } while (INREG32(&pPLLCRC->PCCR0 + pccrIndex) != pccrValNew);

    return TRUE;

}


//------------------------------------------------------------------------------
//
// Function: MIIREAD
//
UINT16 MIIREAD(UINT32 Reg)
{
    UINT32 PhyData, MIIReg;
    UINT32 RegValue;
    volatile UINT32 IntStatues;

    RegValue  = MII_READ_COMMAND(Reg) | CSP_BITFVAL(FEC_MMFR_PA, g_FEC.MIIPhyAddr);

    OUTREG32(&g_FEC.REG->MMFR, RegValue);

    IntStatues = 0;
    while((IntStatues & CSP_BITFVAL(FEC_EIR_MII, 1)) == 0 )
    {
        IntStatues = INREG32(&g_FEC.REG->EIR) ;
    }

    OUTREG32(&g_FEC.REG->EIR, INREG32(&g_FEC.REG->EIR));

    MIIReg = INREG32(&g_FEC.REG->MMFR);

    if(((PhyData = (MIIReg & MMI_DATA_MASK)) != MMI_DATA_MASK) && (PhyData != 0))   
            return PhyData;

    //Read failed
    return 0;
}


//------------------------------------------------------------------------------
//
// Function: MIIWRITE
//
VOID MIIWRITE(UINT32 Reg, UINT16 Data)
{
    volatile UINT32 IntStatues;

    OUTREG32(&g_FEC.REG->MMFR, MII_WRITE_COMMAND(Reg, Data));

    IntStatues = 0;
    while((IntStatues & CSP_BITFVAL(FEC_EIR_MII, 1)) == 0 )
    {
        IntStatues = INREG32(&g_FEC.REG->EIR) ;
    }
}



//------------------------------------------------------------------------------
//
// Function: CalculateHashValue
//
// This function calculates the Hash value for multicasting.
//
// Parameters:
//      pAddr
//        [in] pointer to a ethernet address
//
// Returns:
//      Returns the calculated Hash value.
//
//------------------------------------------------------------------------------
UCHAR CalculateHashValue(const UCHAR *pAddr)
{
    ULONG CRC;
    UCHAR HashValue = 0;
    UCHAR AddrByte;
    int byte, bit;

    // calculate CRC32 value of MAC address
    CRC = CRC_PRIME;

    for(byte=0; byte < ETHER_ADDR_SIZE; byte++)
    {
        AddrByte = *pAddr++;

        for(bit = 0; bit < 8; bit++, AddrByte >>= 1)
        {
            CRC = (CRC >> 1) ^
                    (((CRC ^ AddrByte) & 1) ? CRC_POLYNOMIAL : 0);
        }
    }
    // only upper 6 bits (HASH_BITS) are used which point to specific
    // bit in the hash registers
    HashValue = (UCHAR)((CRC >> (32 - HASH_BITS)) & 0x3f);

    return HashValue;

}


//------------------------------------------------------------------------------
//
// Function: FECEnableInts
//------------------------------------------------------------------------------
VOID FECEnableInts()
{
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+FECEnableInts\r\n"));

    // We are only interested in RX interrupts, Enable  RxF
    OUTREG32(&g_FEC.REG->EIMR, CSP_BITFVAL(FEC_EIMR_RXF, FEC_EIMR_RXF_UNMASK) );

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-FECEnableInts\r\n"));
}

//------------------------------------------------------------------------------
//
// Function: FECDisableInts
//
//------------------------------------------------------------------------------
VOID FECDisableInts()
{
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+FECDisableInts\r\n"));

    // Disable RxF interrupts
    OUTREG32(&g_FEC.REG->EIMR, CSP_BITFVAL(FEC_EIMR_RXF, FEC_EIMR_RXF_MASK) );

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-FECDisableInts\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  FECInitDMABuffer
//
//  This function is used to inform this library on where the buffer for
//  Tx/Rx DMA is. Driver needs  at least:
//
//
//------------------------------------------------------------------------------
BOOL FECInitDMABuffer(UINT32 address, UINT32 size)
{
    BOOL rc = FALSE;
    volatile PBUFFER_DESC BufferDescPointer;
    PBUFFER_DESC DescriptorBase;
    UINT32 BufferOffset;
    UINT32 ph,i;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+FECInitDMABuffer(0x%X, 0x%X)\r\n", address, size));

    ph = OALVAtoPA((VOID*)address);
    //128bit align
    if ((ph & 0x0f) != 0)
    {
        size -= 16 - (ph & 0x0f);
        ph = (ph + 0x0f) & ~0x0f;
    }

    //Check DAM buffer size
    if(size < FEC_DMA_BUFFER_SIZE )
    {
        OALMSGS(OAL_ERROR, (L"ERROR: FECInitDMABuffer: DMA Buffer too small\r\n" ));
        return rc;
    }

    g_FEC.RingPhysicalBase = ph;
    g_FEC.RingBase = (PBYTE)OALPAtoUA(ph);

    DescriptorBase = (PBUFFER_DESC)g_FEC.RingBase;

    // Set receive and transmit descriptor base
    g_FEC.RxBufferDescBase = DescriptorBase;
    g_FEC.TxBufferDescBase = DescriptorBase + FEC_RX_DMA_RING_SIZE;    //128bit align

    g_FEC.CurrentTx = g_FEC.TxBufferDescBase;
    g_FEC.CurrentRx = g_FEC.RxBufferDescBase;

    // allocate receive buffers and initialize the receive buffer descriptors
    BufferDescPointer = g_FEC.RxBufferDescBase;
    BufferOffset = sizeof(BUFFER_DESC) * (FEC_RX_DMA_RING_SIZE + FEC_TX_DMA_RING_SIZE);

    for(i = 0; i < FEC_RX_DMA_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = BD_ENET_RX_EMPTY;
        BufferDescPointer->BufferAddr = g_FEC.RingPhysicalBase +  BufferOffset   + i * FEC_DMA_RX_FRSIZE;
        g_FEC.RxBuffAddr[BufferDescPointer - g_FEC.RxBufferDescBase] = g_FEC.RingBase + BufferOffset +i*FEC_DMA_RX_FRSIZE;

        BufferDescPointer++;
    }

    // set the last buffer to wrap
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;

    // allocate transmit buffers and initialize the transmit buffer descriptors
    BufferDescPointer = g_FEC.TxBufferDescBase;
    BufferOffset += FEC_TX_DMA_RING_SIZE * FEC_DMA_TX_FRSIZE;

    for(i = 0; i < FEC_TX_DMA_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = 0;
        BufferDescPointer->BufferAddr = g_FEC.RingPhysicalBase +  BufferOffset   + i * FEC_DMA_TX_FRSIZE;
        g_FEC.TxBuffAddr[BufferDescPointer - g_FEC.TxBufferDescBase] = g_FEC.RingBase + BufferOffset +i*FEC_DMA_TX_FRSIZE;

        BufferDescPointer++;
    }
    // set the last buffer to wrap
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-FECInitDMABuffer(rc = %d)\r\n", rc));
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: FECInit
//
//------------------------------------------------------------------------------
BOOL FECInit(UINT8 *pAddress, UINT32 offset, const UINT16 mac[3])
{
    BOOL rc = TRUE;
    UINT32 MIIPhyId;
    UINT16 PhyType, RegVal;
    UINT32 i;
    UINT32 MACAdd;
    UINT16 HcdSpeed;

    KITLOutputDebugString("FECInit MAC ADDR %x:%x:%x:%x:%x:%x\r\n",
        mac[0]&0xFF, mac[0]>>8, mac[1]&0xFF, mac[1]>>8, mac[2]&0xFF, mac[2]>>8);

    OALMSGS(OAL_ETHER&&OAL_FUNC, (
        L"FECInit(0x%08x, 0x%08x, %02x:%02x:%02x:%02x:%02x:%02x)\r\n",
        pAddress, offset, mac[0]&0xFF, mac[0]>>8, mac[1]&0xFF, mac[1]>>8,
        mac[2]&0xFF, mac[2]>>8  ));

    g_FEC.filter = PACKET_TYPE_DIRECTED | PACKET_TYPE_BROADCAST;

    g_FEC.REG = (PCSP_FEC_REGS)pAddress;
    g_FEC.MIIPhyAddr = 0;

    //Initialize FEC
    InitFecPhysical();
    OALKitlFECBSPInit();

    // enable clock for FEC hardware
    FECClockSetGatingMode(DDK_CLOCK_GATE_INDEX_FEC, DDK_CLOCK_GATE_MODE_ENABLE);
    FECClockSetGatingMode(DDK_CLOCK_GATE_INDEX_HCLK_FEC, DDK_CLOCK_GATE_MODE_ENABLE);

    // issue a reset to the FEC hardware
    INSREG32BF(&g_FEC.REG->ECR, FEC_ECR_RESET, FEC_ECR_RESET_RESET);
    // wait for the hareware reset to complete
    while(EXTREG32BF(&g_FEC.REG->ECR, FEC_ECR_RESET));

    // set the receive and transmit BDs ring base to
    // hardware registers(ERDSR & ETDSR)
    OUTREG32(&g_FEC.REG->ERDSR, g_FEC.RingPhysicalBase);
    OUTREG32(&g_FEC.REG->ETDSR,
            g_FEC.RingPhysicalBase + FEC_RX_DMA_RING_SIZE*sizeof(BUFFER_DESC));

    //  Enable MII interrupts
    OUTREG32(&g_FEC.REG->EIMR, CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_UNMASK));         // intr mask
    OUTREG32(&g_FEC.REG->EIR, FEC_EIR_CLEARALL_MASK);

    OUTREG32(&g_FEC.REG->TFWR, 0);  // TX FIFO watermark
    OUTREG32(&g_FEC.REG->IAUR, 0);
    OUTREG32(&g_FEC.REG->IALR, 0);
    OUTREG32(&g_FEC.REG->GAUR, 0);
    OUTREG32(&g_FEC.REG->GALR, 0);

    // Set the station address for the FEC Adapter
    MACAdd = mac[0]&0xFF;
    MACAdd = (MACAdd << 8) | (mac[0] >> 8);
    MACAdd = (MACAdd << 8) | ( mac[1]&0xFF);
    MACAdd = (MACAdd << 8) | ( mac[1]>>8);
    OUTREG32(&g_FEC.REG->PALR, MACAdd);

    MACAdd = mac[2]&0xFF;
    MACAdd = (MACAdd << 8) | (mac[2] >> 8);
    OUTREG32(&g_FEC.REG->PAUR, MACAdd << 16);

    // Receive Buffer Size
    OUTREG32(&g_FEC.REG->EMRBR, PKT_MAXBLR_SIZE);

    // setup MII interface
    // Set MII speed to 2.5MHz
     //MIIPhySpeed = 14;
     OUTREG32(&g_FEC.REG->MSCR,
                    CSP_BITFVAL(FEC_MSCR_MIISPEED, MIIPHYSPEED));

    // Enable Ethernet controller
    INSREG32BF(&g_FEC.REG->ECR, FEC_ECR_ETHEREN, FEC_ECR_ETHEREN_ENABLE);

    INSREG32BF(&g_FEC.REG->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);
    OUTREG32(&g_FEC.REG->RCR,
                    CSP_BITFVAL(FEC_RCR_MAXFL, PKT_MAXBUF_SIZE)|
                    CSP_BITFVAL(FEC_RCR_MIIMODE, FEC_RCR_MIIMODE_ENABLE));

    OUTREG32(&g_FEC.REG->TCR, 0);

    //Get the Transciver Vendor ID
    while(g_FEC.MIIPhyAddr < PHY_MAX_COUNT)
    {
        PhyType = MIIREAD(MII_REG_PHYIR1);
        if(PhyType == 0)
        {
            OALMSGS(OAL_ETHER&&OAL_FUNC, (L"MII_REG_PHYIR1 READ Failed \r\n"));
            g_FEC.MIIPhyAddr++ ;
        }
        else
            break;
    }
    MIIPhyId = PhyType << 16;

    PhyType = MIIREAD(MII_REG_PHYIR2);
    if(PhyType == 0)
    {
        KITLOutputDebugString( "MII_REG_PHYIR2 READ Failed \n\r" );
        OALMSGS(OAL_ETHER&&OAL_FUNC, (L"MII_REG_PHYIR2 READ Failed \r\n"));
    }
    MIIPhyId |=  (PhyType & MMI_DATA_MASK);;

    for(i = 0; PhyInfo[i] != NULL; i++)
    {
        if(PhyInfo[i]->PhyId == MIIPhyId)
            break;
    }

    if(PhyInfo[i])
    {
        OALMSGS(OAL_ETHER&&OAL_FUNC, (L"The name for the external PHY is %s\r\n", PhyInfo[i]->PhyName));
    }
    else
    {
        OALMSGS(OAL_ERROR, (L"Failed to get  the external PHY\r\n"));
        rc = FALSE;
        goto CleanUp;
    }

    //Enable/Restart Auto-Negotiation
    MIIWRITE(MII_REG_CR, PHY_AUTONEGO_ENABLE);

    //Wating for Link success
    while (! (MIIREAD(MII_REG_SR) &  PHY_LINKSTATUS));

    //Get Link states
    if(LAN8700==i)
    {
        RegVal = MIIREAD(MII_LAN8700_SCSR);

        HcdSpeed=((RegVal & 0x001C)>>2);
        switch(HcdSpeed)
        {
        case LAN8700_10MBPS_HALFDUPLEX:
            KITLOutputDebugString( "FEC is linked,  Speed  10Mbps , Half Duplex \r\n");
            break;
        case LAN8700_100MBPS_HALFDUPLEX:
            KITLOutputDebugString( "FEC is linked,  Speed  100Mbps , Half Duplex \r\n");
            break;
        case LAN8700_10MBPS_FULLDUPLEX:
            KITLOutputDebugString( "FEC is linked,  Speed  10Mbps , Full Duplex \r\n");
            break;
        case LAN8700_100MBPS_FULLDUPLEX:
            KITLOutputDebugString( "FEC is linked,  Speed  100Mbps , Full Duplex \r\n");
            break;
        }
    }
    //  Disable MII interrupts
    OUTREG32(&g_FEC.REG->EIMR, CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_MASK));

    //Enable FEC RX interrupt
    FECEnableInts();

CleanUp:
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-FECInit(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
// Function: FECSendFrame
//
//------------------------------------------------------------------------------
UINT16 FECSendFrame(const UINT8 *pData, UINT32 length) 
{
    BOOL rc = FALSE;
    volatile PBUFFER_DESC BufferDescPointer;
    PUCHAR MemAddr;
    UINT32  index;
    UINT32  start;

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, ( L"+S (0x%08x, %d)\r\n", pData, length));

    BufferDescPointer = g_FEC.CurrentTx;

    start = OALGetTickCount();
    while (BufferDescPointer->ControlStatus & BD_ENET_TX_READY)
    {
        if ((OALGetTickCount() - start) > FEC_FRAME_SEND_TIMEOUT) 
        {
            OALMSGS(OAL_ERROR, (L"ERROR: FECSendFrame: Send timeout\r\n"));
            return FALSE;
        }
    }

    // Clear all of the status flags
    BufferDescPointer->ControlStatus &= ~BD_ENET_TX_STATS;

    index = BufferDescPointer - g_FEC.TxBufferDescBase;
    MemAddr =  g_FEC.TxBuffAddr[index];

    //Copy Send frame to TA DMA buffer
    memcpy(MemAddr, pData, length);

    // set up the transmit buffer descriptor
    BufferDescPointer->ControlStatus |= (BD_ENET_TX_READY | BD_ENET_TX_INTR |
                                         BD_ENET_TX_LAST  | BD_ENET_TX_TC);

    BufferDescPointer->DataLen = (USHORT)length;

    // Trigger transmission start
    INSREG32BF(&g_FEC.REG->TDAR, FEC_TDAR_ACTIVE, FEC_TDAR_ACTIVE_ACTIVE);

    // If this was the last BD in the ring, start at the begining again
    if(BufferDescPointer->ControlStatus & BD_ENET_TX_WRAP)
    {
        BufferDescPointer = g_FEC.TxBufferDescBase;
    }
    else
    {
        BufferDescPointer++;
    }

    g_FEC.CurrentTx = (PBUFFER_DESC)BufferDescPointer;

    rc = TRUE;

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (L"-FECSendFrame(rc = %d)\r\n", !rc));
    return !rc;
}

//------------------------------------------------------------------------------
//
// Function: FECGetFrame
//
//------------------------------------------------------------------------------
UINT16 FECGetFrame(__out_ecount(*pLength)UINT8 *pData, UINT16 *pLength)
{
    USHORT PacketSize = 0;
    volatile PBUFFER_DESC BufferDescPointer;
    PUCHAR pReceiveBuffer;
    USHORT ControlStatus;

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (L"+FECGetFrame(0x%08x, %d)\r\n", pData, *pLength));

    BufferDescPointer = g_FEC.CurrentRx;
    ControlStatus = BufferDescPointer->ControlStatus;

    while(!(BufferDescPointer->ControlStatus & BD_ENET_RX_EMPTY))
    {
         // Get frame size of the received frame
        PacketSize = BufferDescPointer->DataLen;
        PacketSize -= 4;    //Removed CRC

        if((PacketSize > *pLength) ||
            (BufferDescPointer->ControlStatus & (BD_ENET_RX_LG | BD_ENET_RX_SH |
                                                BD_ENET_RX_NO | BD_ENET_RX_CR |
                                                BD_ENET_RX_OV | BD_ENET_RX_CL )))
        {
            PacketSize = 0;
        }
        else
        {
            // Assign a pointer to the receive buffer
            pReceiveBuffer = g_FEC.RxBuffAddr[BufferDescPointer - g_FEC.RxBufferDescBase];
            memcpy(pData, pReceiveBuffer, PacketSize);
        }

        // Clear the status flags for this BD
        BufferDescPointer->ControlStatus &= ~BD_ENET_RX_STATS;

        // Mark the buffer as empty
        BufferDescPointer->ControlStatus |= BD_ENET_RX_EMPTY;

        // Update BD pointer to next entry
        if(BufferDescPointer->ControlStatus & BD_ENET_RX_WRAP)
                BufferDescPointer = g_FEC.RxBufferDescBase;
        else
                BufferDescPointer++;

        g_FEC.CurrentRx = (PBUFFER_DESC)BufferDescPointer;

        INSREG32BF(&g_FEC.REG->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);
        //clear for Rx interrupt
        OUTREG32(&g_FEC.REG->EIR, INREG32(&g_FEC.REG->EIR));

        if(PacketSize > 0)      break;
    }

    *pLength = PacketSize;
    OALMSGS(OAL_ETHER&&OAL_VERBOSE, ( L"-FECGetFrame(PacketSize = %d)\r\n", PacketSize));

    FECEnableInts();

    // Return packet size
    return PacketSize;
}

//------------------------------------------------------------------------------
//
// Function: FECCurrentPacketFilter
//
//------------------------------------------------------------------------------
VOID FECCurrentPacketFilter(UINT32 filter)
{
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+FECCurrentPacketFilter(0x%08x)\r\n", filter));

    // First set hash functions
    if ((filter & PACKET_TYPE_PROMISCUOUS) != 0 ||
        (filter & PACKET_TYPE_ALL_MULTICAST) != 0)
    {
        OUTREG32(&g_FEC.REG->GALR,  GALR_CLEAR_ALL);
        OUTREG32(&g_FEC.REG->GAUR,  GAUR_CLEAR_ALL);
    }
    else if ((filter & PACKET_TYPE_MULTICAST) != 0)
    {
        OUTREG32(&g_FEC.REG->GAUR, g_FEC.hash[0]);
        OUTREG32(&g_FEC.REG->GALR, g_FEC.hash[1]);
    }

    // Save actual filter
    g_FEC.filter = filter;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-FECCurrentPacketFilter\r\n"));
}

//------------------------------------------------------------------------------
//
// Function: FECMulticastList
//
//------------------------------------------------------------------------------
BOOL FECMulticastList(__out_ecount(count) const UINT8 *pAddresses, UINT32 count)
{
    UCHAR HashValue = 0;
    UINT32 i;
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+FECMulticastList(0x%08x, %d)\r\n", pAddresses, count));

    memset(g_FEC.hash, 0, sizeof(g_FEC.hash));

    for (i = 0; i < count; i++)
    {
        HashValue = CalculateHashValue( pAddresses );

        if( HashValue > 31 )
            g_FEC.hash[0] |= 1 << (HashValue-32);
        else
            g_FEC.hash[1] |= 1 << HashValue;
        pAddresses += 6;
    }

    // But update only Multicast mode is active
    if ((g_FEC.filter & PACKET_TYPE_MULTICAST) != 0 )
    {
        OUTREG32(&g_FEC.REG->GAUR, g_FEC.hash[0]);
        OUTREG32(&g_FEC.REG->GALR, g_FEC.hash[1]);
    }
    else
    {
        CLRREG32(&g_FEC.REG->GAUR, GAUR_CLEAR_ALL);
        CLRREG32(&g_FEC.REG->GALR, GALR_CLEAR_ALL);
    }

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-FECMulticastList(rc = 1)\r\n"));
    return TRUE;
}

