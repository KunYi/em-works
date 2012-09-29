//------------------------------------------------------------------------------
//
// Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: enet.c
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <ethdbg.h>
#include <oal.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_enet.h"
#include "enet.h"

//------------------------------------------------------------------------------
// Defines
#define    ENET_RX_DMA_RING_SIZE    4
#define    ENET_TX_DMA_RING_SIZE    4
#define    ENET_DMA_RX_FRSIZE      2048
#define    ENET_DMA_TX_FRSIZE      2048  
#define    PKT_MAXBLR_SIZE         1520
#define    PKT_MAXBUF_SIZE         1518

// MAC instance
#define MAC0        0

//------------------------------------------------------------------------------
// Types
typedef struct
{   
    UINT32    MIIPhyAddr;

    // virtual address and physical address for BDs
    PBYTE          RingBase;
    UINT32         RingPhysicalBase;
    
    // virtual address for receive and transmit buffers
    PVOID          RxBufferBase;
    PVOID          TxBufferBase;
    
    PBUFFER_DESC    RxBufferDescBase;
    PBUFFER_DESC    TxBufferDescBase;
    
    PUCHAR    RxBuffAddr[ENET_RX_DMA_RING_SIZE];
    PUCHAR    TxBuffAddr[ENET_TX_DMA_RING_SIZE];
    
    PBUFFER_DESC    CurrentRx;
    PBUFFER_DESC    CurrentTx;
    
    UINT32   hash[2];
    UINT32   filter;

    BOOL     bUseRMII;
}ENET;

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables
static ENET g_ENET;
static PVOID pv_HWregENET0;
static PVOID pv_HWregENET1;

//------------------------------------------------------------------------------
// Local Functions
UINT16 ENETSendFrame(UINT8 *pData, UINT32 length);

//------------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregCLKCTRL;

//-----------------------------------------------------------------------------
// External Functions
extern void EdbgOutputDebugString (LPCSTR sz, ...);
extern void OALKitlENETBSPInit(void);
extern BOOL BSPENETusesRMII(void);
extern VOID BSPENETClockInit();

//------------------------------------------------------------------------------
//
// Function: MIIREAD
//
UINT16 MIIREAD(UINT32 Reg)
{
    UINT32 PhyData, MIIReg;
    UINT32 RegValue;
    volatile UINT32 IntStatues;

    // Compose command
    RegValue = MII_READ_COMMAND(Reg);

    // Send command
    HW_ENET_MAC_MMFR_WR(MAC0, RegValue);
 
    // Wait command complete status
    IntStatues = 0;
    while ((IntStatues & BM_ENET_MAC_EIR_MII) == 0)
    {
        IntStatues = HW_ENET_MAC_EIR_RD(MAC0);
    }
    
    // Clear status (w1c)
    BW_ENET_MAC_EIR_MII(MAC0, 1);
    
    // Get result
    MIIReg = HW_ENET_MAC_MMFR_RD(MAC0);
    if (((PhyData = (MIIReg & 0xffff)) != 0xffff) && (PhyData != 0))   
    {
        return (UINT16)PhyData;
    }
    else
    {
        // Read failed
        return 0;
    }
}

//------------------------------------------------------------------------------
//
// Function: MIIWRITE
//
VOID MIIWRITE(UINT32 Reg, UINT16 Data)
{
    UINT32 RegValue;
    volatile UINT32 IntStatues;

    // Compose command
    RegValue = MII_WRITE_COMMAND(Reg, Data);

    // Send command
    HW_ENET_MAC_MMFR_WR(MAC0, RegValue);
 
    // Wait command complete status
    IntStatues = 0;
    while ((IntStatues & BM_ENET_MAC_EIR_MII) == 0)
    {
        IntStatues = HW_ENET_MAC_EIR_RD(MAC0);
    }

    // Clear status (w1c)
    BW_ENET_MAC_EIR_MII(MAC0, 1);
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
UCHAR CalculateHashValue(UCHAR *pAddr)
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
// Function: ENETEnableInts
//------------------------------------------------------------------------------
VOID ENETEnableInts()
{
    // We are only interested in RX interrupts, Enable  RxF 
    HW_ENET_MAC_EIMR_SET(MAC0,BM_ENET_MAC_EIR_RXF);
}

//------------------------------------------------------------------------------
//
// Function: ENETDisableInts
//
//------------------------------------------------------------------------------
VOID ENETDisableInts()
{
    // Disable RxF interrupts
    HW_ENET_MAC_EIMR_CLR(MAC0,BM_ENET_MAC_EIR_RXF);
}

//------------------------------------------------------------------------------
//
// Function: ENETPowerOff
//
//------------------------------------------------------------------------------
VOID ENETPowerOff()
{   
    // Enable Phy suspend 
    MIIWRITE(MII_REG_CR, 0x800);
}

//------------------------------------------------------------------------------
//
// Function: ENETPowerOn
//
//------------------------------------------------------------------------------
VOID ENETPowerOn()
{
    int i;

    // Enable Restart Auto-Negotiation
    MIIWRITE(MII_REG_CR, 0x1200);
    for(i=0;i<5;i++)
    {
        OALStall(1000000);
    }
 
}
//------------------------------------------------------------------------------
//
//  Function:  ENETInitDMABuffer
//
//  This function is used to inform this library on where the buffer for
//  Tx/Rx DMA is. Driver needs  at least:
//    
//   
//------------------------------------------------------------------------------
BOOL ENETInitDMABuffer(UINT32 address, UINT32 size)
{
    BOOL rc = FALSE;
    volatile PBUFFER_DESC BufferDescPointer;
    PBUFFER_DESC DescriptorBase;
    UINT32 BufferOffset;
    UINT32 ph,i;
    
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+ENETInitDMABuffer(0x%X, 0x%X)\r\n", address, size));

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
        EdbgOutputDebugString("ENETInitDMABuffer DMA Buffer too small\r\n");
        OALMSGS(OAL_ERROR, (L"ERROR: ENETInitDMABuffer: DMA Buffer too small\r\n" ));
        return rc;
    }

    g_ENET.RingPhysicalBase = ph; 
    g_ENET.RingBase = (PBYTE)OALPAtoUA(ph); 
    
    DescriptorBase = (PBUFFER_DESC)g_ENET.RingBase;

    // Set receive and transmit descriptor base
    g_ENET.RxBufferDescBase = DescriptorBase;
    g_ENET.TxBufferDescBase = DescriptorBase + ENET_RX_DMA_RING_SIZE;    

    g_ENET.CurrentTx = g_ENET.TxBufferDescBase;
    g_ENET.CurrentRx = g_ENET.RxBufferDescBase;
    
    // allocate receive buffers and initialize the receive buffer descriptors
    BufferDescPointer = g_ENET.RxBufferDescBase;
    BufferOffset = sizeof(BUFFER_DESC) * (ENET_RX_DMA_RING_SIZE + ENET_TX_DMA_RING_SIZE);
    
    for(i = 0; i < ENET_RX_DMA_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = BD_ENET_RX_EMPTY;
        BufferDescPointer->BufferAddr = g_ENET.RingPhysicalBase +  BufferOffset   + i * ENET_DMA_RX_FRSIZE;
        g_ENET.RxBuffAddr[BufferDescPointer - g_ENET.RxBufferDescBase] = g_ENET.RingBase + BufferOffset +i*ENET_DMA_RX_FRSIZE;
        
        BufferDescPointer++;
    }
    
    // set the last buffer to wrap 
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;
    
    // allocate transmit buffers and initialize the transmit buffer descriptors
    BufferDescPointer = g_ENET.TxBufferDescBase;
    BufferOffset += ENET_RX_DMA_RING_SIZE * ENET_DMA_RX_FRSIZE;

    for(i = 0; i < ENET_TX_DMA_RING_SIZE; i++)
    {
        BufferDescPointer->ControlStatus = 0;
        BufferDescPointer->BufferAddr = g_ENET.RingPhysicalBase +  BufferOffset   + i * ENET_DMA_TX_FRSIZE;
        g_ENET.TxBuffAddr[BufferDescPointer - g_ENET.TxBufferDescBase] = g_ENET.RingBase + BufferOffset +i*ENET_DMA_TX_FRSIZE;
            
        BufferDescPointer++;
    }
    
    // set the last buffer to wrap
    BufferDescPointer--;
    BufferDescPointer->ControlStatus |= BD_SC_WRAP;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-ENETInitDMABuffer(rc = %d)\r\n", rc));
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: ENETInit
//
//------------------------------------------------------------------------------
BOOL ENETInit(UINT8 *pAddress, UINT32 offset, UINT16 mac[3])
{
    BOOL rc = TRUE;
    UINT32 MIIPhyId;
    UINT16 PhyType=0, RegVal=0;
    UINT32 i;
    UINT32 MACAdd;
    UINT16 HcdSpeed;
    BOOL fIs100MB;
    BOOL fIsHalfDup=FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(offset);
    
    EdbgOutputDebugString("ENETInit MAC ADDR %x:%x:%x:%x:%x:%x\r\n",
        mac[0]&0xFF, mac[0]>>8, mac[1]&0xFF, mac[1]>>8, mac[2]&0xFF, mac[2]>>8);
    OALMSGS(OAL_ETHER&&OAL_FUNC, (
        L"ENETInit(0x%08x, 0x%08x, %02x:%02x:%02x:%02x:%02x:%02x)\r\n",
        pAddress, offset, mac[0]&0xFF, mac[0]>>8, mac[1]&0xFF, mac[1]>>8,
        mac[2]&0xFF, mac[2]>>8  ));

    // Init globals
    pv_HWregENET0=pAddress;
    g_ENET.filter = PACKET_TYPE_DIRECTED | PACKET_TYPE_BROADCAST;
    g_ENET.MIIPhyAddr = 0;
    g_ENET.bUseRMII = BSPENETusesRMII();
 
    // Gate on ENET clock in CLKCTRL 
    // (This must be done before accessing ENET registers)
    BSPENETClockInit();

    // Reset MAC
    BW_ENET_MAC_ECR_RESET(MAC0, 1);
    while (HW_ENET_MAC_ECR_RD(MAC0) & BM_ENET_MAC_ECR_RESET); 

    // Set RMII mode if it is
    if (g_ENET.bUseRMII == FALSE)
    {
        BW_ENET_MAC_RCR_RMII_MODE(MAC0, 0);
    }
    else
    {
        BW_ENET_MAC_RCR_RMII_MODE(MAC0, 1);
    }
    // MII_MODE should always be 1
    // BW_ENET_MAC_RCR_MII_MODE(MAC0, 1);

    // The PHY requires minimum 400 ns cycle time,
    // which means MDC no faster than 2.5 MHz.
    BW_ENET_MAC_MSCR_MII_SPEED(MAC0, 40);

    // Set up PINCTRL and PHY for talking to PHY
    OALKitlENETBSPInit();

    // Get the Transciver Vendor ID
    PhyType = MIIREAD(MII_REG_PHYIR1); 
    while (PhyType == 0 || PhyType == 0xFFFF)
    {
        EdbgOutputDebugString("Bad MMI read, continue ...\r\n");
        OALStall(10000);
        PhyType = MIIREAD(MII_REG_PHYIR1); 
    }
    MIIPhyId = (UINT32) PhyType << 16;
    // Read rest part
    PhyType = MIIREAD(MII_REG_PHYIR2);
    if (PhyType == 0)
    {
        OALMSGS(OAL_ERROR, (L"MII_REG_PHYIR2 READ Failed \r\n"));
    }
    MIIPhyId |=  (PhyType & 0xffff);;
    // Validate ID
    for (i = 0; PhyInfo[i] != NULL; i++)
    {
        if (PhyInfo[i]->PhyId == MIIPhyId)
            break;

        if ((PhyInfo[i]->PhyId & 0xffff0) == (MIIPhyId & 0xffff0)) 
            break;
    }
    if (PhyInfo[i])
    {        
        OALMSGS(OAL_ETHER&&OAL_FUNC, (L"The name for the external PHY is %s\r\n", PhyInfo[i]->PhyName));
    }
    else
    {
        EdbgOutputDebugString( "Failed to get the external PHY\r\n");
        rc = FALSE;
        goto CleanUp;
    }

    // Enable/Restart Auto-Negotiation
    MIIWRITE(MII_REG_CR, 0x1200);

    // Waiting for Link success
    while (!(MIIREAD(MII_REG_SR) &  0x0004));

    // Get Link states
    fIs100MB = FALSE;
    if(Am79c874==i)
    {
        RegVal = MIIREAD(MII_AM79C874_DR);
        if(RegVal & 0x0400)
        {
            EdbgOutputDebugString( "ENET is linked, Speed  100Mbps \r\n");
            fIs100MB = TRUE;
            if(RegVal & 0x0800)
            {
                EdbgOutputDebugString( "Full Duplex\r\n");
            }
            else
            {
                EdbgOutputDebugString( "Half Duplex\r\n");
                fIsHalfDup=TRUE;
            }
        }
        else
        {
            EdbgOutputDebugString( "ENET is linked,  Speed  10Mbps \r\n");
            
            if(RegVal & 0x0800)
            {
                EdbgOutputDebugString( "Full Duplex\r\n");
            }
            else
            {
                EdbgOutputDebugString( "Half Duplex\r\n");
                fIsHalfDup=TRUE;
            }
        }
    }else if(LAN87xx==i)
    {
        RegVal = MIIREAD(MII_LAN87xx_SCSR);
        
        HcdSpeed=((RegVal & 0x001C)>>2);
        if(1==HcdSpeed)
        {
            EdbgOutputDebugString( "ENET is linked,  Speed  10Mbps , Half Duplex \r\n");
            fIsHalfDup=TRUE;
        }
        else if(5==HcdSpeed)
        {
            EdbgOutputDebugString( "ENET is linked,  Speed  10Mbps , Full Duplex \r\n");
        }       
        else if(2==HcdSpeed)
        {
            EdbgOutputDebugString( "ENET is linked,  Speed  100Mbps , Half Duplex \r\n");
            fIsHalfDup=TRUE;
            fIs100MB = TRUE;
        }
        else if(6==HcdSpeed)
        {
            fIs100MB = TRUE;
            EdbgOutputDebugString( "ENET is linked,  Speed  100Mbps , Full Duplex \r\n");
        }
     }
    else if(Dp83640==i)
    {
        RegVal = MIIREAD(MII_DP83640_PHYSTS);
        HcdSpeed=(((UINT16)RegVal & 0x0006)>>1);
        if(1 & HcdSpeed)
        {
            if(2 & HcdSpeed)
            {
                EdbgOutputDebugString( "ENET is linked,  Speed  10Mbps , Full Duplex \r\n");
            }
            else
            {
                EdbgOutputDebugString( "ENET is linked,  Speed  10Mbps , Half Duplex \r\n");
                fIsHalfDup=TRUE;
            }
        }
        else
        {
            if(2 & HcdSpeed)
            {
                EdbgOutputDebugString( "ENET is linked,  Speed  100Mbps , Full Duplex \r\n");
                fIs100MB = TRUE;
            }
            else
            {
                EdbgOutputDebugString( "ENET is linked,  Speed  100Mbps , Half Duplex \r\n");
                fIsHalfDup=TRUE;
                fIs100MB = TRUE;
            }    
        }
    }
    else 
    {
        //using standard registers
        RegVal = MIIREAD(MII_REG_SR);
        
        if(RegVal & PHY_STAT_10HDX)
        {
            EdbgOutputDebugString( "ENET is linked,  Speed  10Mbps , Half Duplex \r\n");
            fIsHalfDup=TRUE;
        }
        else if(RegVal & PHY_STAT_10FDX)
        {
            EdbgOutputDebugString( "ENET is linked,  Speed  10Mbps , Full Duplex \r\n");
        }       
        else if(RegVal & PHY_STAT_100HDX)
        {
            EdbgOutputDebugString( "ENET is linked,  Speed  100Mbps , Half Duplex \r\n");
            fIsHalfDup=TRUE;
            fIs100MB = TRUE;
        }
        else if(RegVal & PHY_STAT_100FDX)
        {
            fIs100MB = TRUE;
            EdbgOutputDebugString( "ENET is linked,  Speed  100Mbps , Full Duplex \r\n");
        }
    } 
    // If the phy only supports half duplex
    if (fIsHalfDup)
        BW_ENET_MAC_RCR_DRT(MAC0, 1);
        
    // Disable internal loopback
    BW_ENET_MAC_RCR_LOOP(MAC0, 0);

    // Enable flow control
    BW_ENET_MAC_RCR_FCE(MAC0, 1);              

    // Set RX buffer size
    BW_ENET_MAC_EMRBR_R_BUF_SIZE(MAC0, PKT_MAXBLR_SIZE/16);

    // Set the station address for the ENET Adapter
    MACAdd = mac[0] & 0xFF;
    MACAdd = (MACAdd << 8) | (mac[0] >> 8);
    MACAdd = (MACAdd << 8) | ( mac[1] & 0xFF);
    MACAdd = (MACAdd << 8) | ( mac[1] >> 8);
    HW_ENET_MAC_PALR_WR(MAC0, MACAdd);
    
    MACAdd = mac[2] & 0xFF;
    MACAdd = (MACAdd << 8) | (mac[2] >> 8);
    BW_ENET_MAC_PAUR_PADDR2(MAC0, MACAdd);

    // Enable MAC address for TX
    BW_ENET_MAC_TCR_TX_ADDR_INS(MAC0, 1);

    // Enable full duplex
    // BW_ENET_MAC_TCR_FEDN(MAC0, 1);
    
    // Set the receive and transmit BDs ring base to 
    // hardware registers (ERDSR & ETDSR)
    HW_ENET_MAC_ERDSR_WR(MAC0, g_ENET.RingPhysicalBase);
    HW_ENET_MAC_ETDSR_WR(MAC0, g_ENET.RingPhysicalBase + ENET_RX_DMA_RING_SIZE * sizeof(BUFFER_DESC));

    // Clear interrupt status
    HW_ENET_MAC_EIR_WR(MAC0, 0xFFFFFFFF);

    // Enable ENET RX interrupt
    ENETEnableInts();

    // Enable MAC0
    BW_ENET_MAC_ECR_ETHER_EN(MAC0, 1);

    // RX BD Ready (Must be done after ETHER_EN)
    BW_ENET_MAC_RDAR_RDAR(MAC0, 1);

CleanUp:
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-ENETInit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
// Function: CopyFrame
//
//------------------------------------------------------------------------------
VOID CopyFrame(UINT8 *pDes, UINT8 *pSrc, UINT32 cnt)
{
    long *plDes = (long *)pDes;
    long *plSrc = (long *)pSrc;

    cnt = cnt / sizeof(long);
    while (cnt--)
    {
        *plDes = htonl(*plSrc);
        plDes++;  plSrc++;
    }
}

//------------------------------------------------------------------------------
//
// Function: ENETSendFrame
//
//------------------------------------------------------------------------------
UINT16 ENETSendFrame(UINT8 *pData, UINT32 length)
{
    BOOL rc = FALSE;
    volatile PBUFFER_DESC BufferDescPointer;
    PUCHAR MemAddr;
    UINT32  index;
    UINT32 dwCounter = 0;

    BufferDescPointer = g_ENET.CurrentTx;

    for(;;)
    {
        if (!(BufferDescPointer->ControlStatus & BD_ENET_TX_READY))
        {
            break;
        }
        OALStall(10);
        if (dwCounter++ > 200)
        {
            OALMSGS(OAL_ERROR, (L"ERROR: ENETSendFrame: Send timeout\r\n"));
            return FALSE;
        }
    }
    
    // Clear all of the status flags
    BufferDescPointer->ControlStatus &= ~BD_ENET_TX_STATS; 

    index = BufferDescPointer - g_ENET.TxBufferDescBase;
    MemAddr =  g_ENET.TxBuffAddr[index];

    // Copy Send frame to TX DMA buffer
    // memcpy(MemAddr, pData, length);
    CopyFrame(MemAddr, pData, length + (4 - length%4));

    // Set up length
    BufferDescPointer->DataLen = (USHORT)length;

    // Set up the transmit buffer descriptor
    BufferDescPointer->ControlStatus |= (BD_ENET_TX_READY | BD_ENET_TX_LAST  | BD_ENET_TX_TC);
    
    // Trigger transmission start
    BW_ENET_MAC_TDAR_TDAR(MAC0, 1);

    // If this was the last BD in the ring, start at the begining again
    if(BufferDescPointer->ControlStatus & BD_ENET_TX_WRAP)
    {
        BufferDescPointer = g_ENET.TxBufferDescBase;
    }
    else
    {
        BufferDescPointer++;
    }

    g_ENET.CurrentTx = (PBUFFER_DESC)BufferDescPointer;
    
    rc = TRUE;

    return (UINT16)!rc;
}

//------------------------------------------------------------------------------
//
// Function: ENETGetFrame
//
//------------------------------------------------------------------------------
UINT16 ENETGetFrame(UINT8 *pData, UINT16 *pLength)
{
    USHORT PacketSize = 0;
    volatile PBUFFER_DESC BufferDescPointer;
    PUCHAR pReceiveBuffer;
    USHORT ControlStatus;

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (L"+ENETGetFrame(0x%08x, %d)\r\n", pData, *pLength));

    // Check RX BD
    BufferDescPointer = g_ENET.CurrentRx;
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
            // EdbgOutputDebugString("!!!ERROR Rx ControlStatus is = %X\r\n", BufferDescPointer->ControlStatus);    
        }
        else
        {
            // Assign a pointer to the receive buffer    
            pReceiveBuffer = g_ENET.RxBuffAddr[BufferDescPointer - g_ENET.RxBufferDescBase];
            // memcpy(pData, pReceiveBuffer, PacketSize);
            CopyFrame(pData, pReceiveBuffer, PacketSize + (4 - PacketSize%4));
        }
        
        // Clear the status flags for this BD
        BufferDescPointer->ControlStatus &= ~BD_ENET_RX_STATS;
        
        // Mark the buffer as empty
        BufferDescPointer->ControlStatus |= BD_ENET_RX_EMPTY;
        
        // Update BD pointer to next entry 
        if(BufferDescPointer->ControlStatus & BD_ENET_RX_WRAP)
            BufferDescPointer = g_ENET.RxBufferDescBase;
        else
            BufferDescPointer++;
        
        g_ENET.CurrentRx = (PBUFFER_DESC)BufferDescPointer;
       
        // RX BD Ready
        BW_ENET_MAC_RDAR_RDAR(MAC0,1);
        
        //clear for Rx interrupt
        HW_ENET_MAC_EIR_WR(MAC0,HW_ENET_MAC_EIR_RD(MAC0));
        
        if (PacketSize > 0) break;
    }

    *pLength = PacketSize;
    OALMSGS(OAL_ETHER&&OAL_VERBOSE, ( L"-ENETGetFrame(PacketSize = %d)\r\n", PacketSize));
    
    // Enable RX interrupt
    ENETEnableInts();

    // Return packet size
    return PacketSize;
}

//------------------------------------------------------------------------------
//
// Function: ENETCurrentPacketFilter
//
//------------------------------------------------------------------------------
VOID ENETCurrentPacketFilter(UINT32 filter)
{
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+ENETCurrentPacketFilter(0x%08x)\r\n", filter));

    // First set hash functions
    if ((filter & PACKET_TYPE_PROMISCUOUS) != 0 ||
        (filter & PACKET_TYPE_ALL_MULTICAST) != 0
    ) {
        HW_ENET_MAC_GALR_WR(MAC0,0xFFFFFFFF);
        HW_ENET_MAC_GAUR_WR(MAC0,0xFFFFFFFF);
    } 
    else if ((filter & PACKET_TYPE_MULTICAST) != 0)
    {
        HW_ENET_MAC_GAUR_WR(MAC0,g_ENET.hash[0]);
        HW_ENET_MAC_GALR_WR(MAC0,g_ENET.hash[1]);
    }

    // Save actual filter
    g_ENET.filter = filter;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-ENETCurrentPacketFilter\r\n"));
}

//------------------------------------------------------------------------------
//
// Function: ENETMulticastList
//
//------------------------------------------------------------------------------
BOOL ENETMulticastList(UINT8 *pAddresses, UINT32 count)
{
    UCHAR HashValue = 0;
    UINT32 i;
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+ENETMulticastList(0x%08x, %d)\r\n", pAddresses, count));

    memset(g_ENET.hash, 0, sizeof(g_ENET.hash));
    
    for (i = 0; i < count; i++) 
    {
        HashValue = CalculateHashValue( pAddresses );

        if( HashValue > 31 )
            g_ENET.hash[0] |= 1 << (HashValue-32);
        else
            g_ENET.hash[1] |= 1 << HashValue;
        pAddresses += 6;
    }

    // But update only Multicast mode is active
    if ((g_ENET.filter & PACKET_TYPE_MULTICAST) != 0 ) 
    {        
        HW_ENET_MAC_GAUR_WR(MAC0,g_ENET.hash[0]);
        HW_ENET_MAC_GALR_WR(MAC0,g_ENET.hash[1]);
    }
    else
    {
        HW_ENET_MAC_GAUR_WR(MAC0,0xFFFFFFFF);
        HW_ENET_MAC_GALR_WR(MAC0,0xFFFFFFFF);
    }

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-ENETMulticastList(rc = 1)\r\n"));
    return TRUE;
}

