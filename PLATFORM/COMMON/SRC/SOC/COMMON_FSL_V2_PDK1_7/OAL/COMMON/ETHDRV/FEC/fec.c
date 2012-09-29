//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: fec.c
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
#include "common_fec.h"
#include "fec.h"

//------------------------------------------------------------------------------
// Defines
#define    FEC_RX_DMA_RING_SIZE    4
#define    FEC_TX_DMA_RING_SIZE    4
#define    FEC_DMA_RX_FRSIZE      2048
#define    FEC_DMA_TX_FRSIZE      2048  
#define    PKT_MAXBLR_SIZE        1520
#define    PKT_MAXBUF_SIZE        1518
//------------------------------------------------------------------------------
// Types
typedef struct
{
    PCSP_FEC_REGS REG;
    
    UINT32    MIIPhyAddr;
    
    // virtual address and physical address for BDs
    PBYTE          RingBase;
    UINT32         RingPhysicalBase;
    
    // virtual address for receive and transmit buffers
    PVOID               RxBufferBase;
    PVOID               TxBufferBase;
    
    PBUFFER_DESC    RxBufferDescBase;
    PBUFFER_DESC    TxBufferDescBase;
    
    PUCHAR    RxBuffAddr[FEC_RX_DMA_RING_SIZE];
    PUCHAR    TxBuffAddr[FEC_TX_DMA_RING_SIZE];
    
    PBUFFER_DESC    CurrentRx;
    PBUFFER_DESC    CurrentTx;
    
    UINT32   hash[2];
    UINT32   filter;

    BOOL     bUseRMII;
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
extern void EdbgOutputDebugString (LPCSTR sz, ...);


extern void OALKitlFECBSPInit(void);
extern BOOL BSPFECusesRMII(void);


//------------------------------------------------------------------------------
//
// Function: MIIREAD
//
UINT16 MIIREAD(UINT32 Reg)
{
    UINT32 PhyData, MIIReg;
    UINT32 RegValue;
    volatile UINT32 IntStatues;
    
    //EdbgOutputDebugString("+MIIREAD\r\n");    
    
    RegValue  = MII_READ_COMMAND(Reg) | CSP_BITFVAL(FEC_MMFR_PA, g_FEC.MIIPhyAddr);
    
    OUTREG32(&g_FEC.REG->MMFR, RegValue);
       
    IntStatues = 0;
    while((IntStatues & CSP_BITFVAL(FEC_EIR_MII, 1)) == 0 )
    {
        IntStatues = INREG32(&g_FEC.REG->EIR) ;
    }
    
    OUTREG32(&g_FEC.REG->EIR, INREG32(&g_FEC.REG->EIR));
    
    MIIReg = INREG32(&g_FEC.REG->MMFR);
    
    if(((PhyData = (MIIReg & 0xffff)) != 0xffff) && (PhyData != 0))   
            return (UINT16)PhyData;
    
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
    //EdbgOutputDebugString("+MIIWRITE\r\n");
    OUTREG32(&g_FEC.REG->MMFR, MII_WRITE_COMMAND(Reg, Data));

    IntStatues = 0;
    while((IntStatues & CSP_BITFVAL(FEC_EIR_MII, 1)) == 0 )
    {
        IntStatues = INREG32(&g_FEC.REG->EIR) ;
    }
    //EdbgOutputDebugString("-MIIWRITE\r\n");
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
    
    //EdbgOutputDebugString("+FECInitDMABuffer(0x%X, 0x%X)\r\n", address, size);
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+FECInitDMABuffer(0x%X, 0x%X)\r\n", address, size));

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
        EdbgOutputDebugString("FECInitDMABuffer DMA Buffer too small\r\n");
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
    BufferOffset += FEC_RX_DMA_RING_SIZE * FEC_DMA_RX_FRSIZE;

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
BOOL FECInit(UINT8 *pAddress, UINT32 offset, UINT16 mac[3])
{
    BOOL rc = TRUE;
    UINT32 MIIPhyId;
    UINT16 PhyType=0, RegVal=0;
    UINT32 i;
    UINT32 MACAdd;
    UINT16 HcdSpeed;
    BOOL fIs100MB;
    BOOL fIsHalfDup=FALSE;
	UINT16	CRVal;						// CS&ZHL MAY-31-2011: 

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(offset);

    //EdbgOutputDebugString("+FECInit pAddress 0x%x, offset %d\r\n", pAddress, offset);
    EdbgOutputDebugString("FECInit MAC ADDR %x:%x:%x:%x:%x:%x\r\n",
        mac[0]&0xFF, mac[0]>>8, mac[1]&0xFF, mac[1]>>8, mac[2]&0xFF, mac[2]>>8);
    OALMSGS(OAL_ETHER&&OAL_FUNC, (
        L"FECInit(0x%08x, 0x%08x, %02x:%02x:%02x:%02x:%02x:%02x)\r\n",
        pAddress, offset, mac[0]&0xFF, mac[0]>>8, mac[1]&0xFF, mac[1]>>8,
        mac[2]&0xFF, mac[2]>>8  ));

    g_FEC.filter = PACKET_TYPE_DIRECTED | PACKET_TYPE_BROADCAST;

    g_FEC.REG = (PCSP_FEC_REGS)pAddress;
    g_FEC.MIIPhyAddr = 0;

    OALKitlFECBSPInit();
    g_FEC.bUseRMII = BSPFECusesRMII();
    
    // issue a reset to the FEC hardware
    INSREG32BF(&g_FEC.REG->ECR, FEC_ECR_RESET, FEC_ECR_RESET_RESET);
    // wait for the hareware reset to complete
    while(EXTREG32BF(&g_FEC.REG->ECR, FEC_ECR_RESET));
    
    // set the receive and transmit BDs ring base to 
    // hardware registers(ERDSR & ETDSR)
    OUTREG32(&g_FEC.REG->ERDSR, g_FEC.RingPhysicalBase);
    OUTREG32(&g_FEC.REG->ETDSR, 
            g_FEC.RingPhysicalBase + FEC_RX_DMA_RING_SIZE*sizeof(BUFFER_DESC));
            
    // set other hardware registers
    OUTREG32(&g_FEC.REG->EIR, FEC_EIR_CLEARALL_MASK);

    OUTREG32(&g_FEC.REG->IAUR, 0);
    OUTREG32(&g_FEC.REG->IALR, 0);
    OUTREG32(&g_FEC.REG->GAUR, 0);
    OUTREG32(&g_FEC.REG->GALR, 0);
    
    OUTREG32(&g_FEC.REG->EMRBR, PKT_MAXBLR_SIZE);

    INSREG32BF(&g_FEC.REG->ECR, FEC_ECR_ETHEREN, FEC_ECR_ETHEREN_ENABLE);
    INSREG32BF(&g_FEC.REG->RDAR, FEC_RDAR_ACTIVE, FEC_RDAR_ACTIVE_ACTIVE);
    
    // Set the station address for the FEC Adapter
    MACAdd = mac[0]&0xFF;
    MACAdd = (MACAdd << 8) | (mac[0] >> 8);
    MACAdd = (MACAdd << 8) | ( mac[1]&0xFF);
    MACAdd = (MACAdd << 8) | ( mac[1]>>8);
    OUTREG32(&g_FEC.REG->PALR, MACAdd);

    MACAdd = mac[2]&0xFF;
    MACAdd = (MACAdd << 8) | (mac[2] >> 8);
    OUTREG32(&g_FEC.REG->PAUR, MACAdd << 16);

    OUTREG32(&g_FEC.REG->RCR,
                    CSP_BITFVAL(FEC_RCR_MAXFL, PKT_MAXBUF_SIZE)|
                    CSP_BITFVAL(FEC_RCR_MIIMODE, FEC_RCR_MIIMODE_ENABLE));
                    
    OUTREG32(&g_FEC.REG->TCR, 0);    

    // setup MII interface
    if (g_FEC.bUseRMII == FALSE)
    {
        // Set MII speed to 2.5MHz        
        OUTREG32(&g_FEC.REG->MSCR,
            CSP_BITFVAL(FEC_MSCR_MIISPEED, 14));
    }
    else
    {
        // Set MII speed 
        OUTREG32(&g_FEC.REG->MSCR,
            CSP_BITFVAL(FEC_MSCR_MIISPEED, 4));
    }
    //  Enable MII interrupts
    OUTREG32(&g_FEC.REG->EIMR, CSP_BITFVAL(FEC_EIMR_MII, FEC_EIMR_MII_UNMASK));

    //Get the Transciver Vendor ID
    while(g_FEC.MIIPhyAddr < PHY_MAX_COUNT)
    {
        PhyType = MIIREAD(MII_REG_PHYIR1); 
        if(PhyType == 0)
        {
            
            OALMSGS(OAL_ETHER&&OAL_FUNC, (L"MII_REG_PHYIR1 READ Failed \r\n"));
            OALStall(5000);
            g_FEC.MIIPhyAddr++ ;
        }   
        else
            break;
    }
    
    MIIPhyId =(UINT32) PhyType << 16;

    PhyType = MIIREAD(MII_REG_PHYIR2);
    if(PhyType == 0)
    {
        OALMSGS(OAL_ETHER&&OAL_FUNC, (L"MII_REG_PHYIR1 READ Failed \r\n"));
    }
    MIIPhyId |=  (PhyType & 0xffff);;
       
    for(i = 0; PhyInfo[i] != NULL; i++)
    {
        if(PhyInfo[i]->PhyId == MIIPhyId)
            break;

        if(((PhyInfo[i]->PhyId)&0xffff0) == (MIIPhyId&0xffff0)) 
            break;

    }
 
    if(PhyInfo[i])
    {        
        //OALMSGS(OAL_ETHER&&OAL_FUNC, (L"The name for the external PHY is %s\r\n", PhyInfo[i]->PhyName));
		// CS&ZHL MAY-30-2011: show the name of phy found
        OALMSGS(1, (L"The name for the external PHY is %s\r\n", PhyInfo[i]->PhyName));
    }
    else
    {
        EdbgOutputDebugString( "Failed to get the external PHY\r\n");
        rc = FALSE;
        goto CleanUp;
    }

    if (g_FEC.bUseRMII)
    {
        // Also switch the FEC in RMII mode    
        OUTREG32(&g_FEC.REG->MIIGSK_CFGR,FEC_MIIGSK_CFGR_IF_MODE_RMII);
    }

	// CS&ZHL MAY-30-2011: show MII_REG_CR and MII_REG_SR
    CRVal = MIIREAD(MII_REG_CR);
    EdbgOutputDebugString( "MII_REG_CR = 0x%x\r\n", CRVal);
    RegVal = MIIREAD(MII_REG_SR);
    EdbgOutputDebugString( "MII_REG_SR = 0x%x\r\n", RegVal);

	if(CRVal & PHY_CR_AUTONEGOCIATION_ENABLE)
	{
		EdbgOutputDebugString( "Auto-Negotiation is going......");
	}
	else
	{
		EdbgOutputDebugString( "Restart Auto-Negotiation......");
		//Enable/Restart Auto-Negotiation
		//MIIWRITE(MII_REG_CR, 0x1200);
		MIIWRITE(MII_REG_CR, (PHY_CR_AUTONEGOCIATION_ENABLE | PHY_CR_AUTONEGOCIATION_RESTART));
	}

	for(; ; )
	{
		EdbgOutputDebugString( "100ms...");
		OALStall(100000);		// wait 100ms
		RegVal = MIIREAD(MII_REG_SR);
		if(!(RegVal & PHY_STAT_AUTONEGOCIATION_COMPLETE))
		{
			continue;
		}
		if(!(RegVal & PHY_STAT_LINK_ESTABLISHED))
		{
			continue;
		}
		break;
	}

    //Wating for Link success
    //while (! (MIIREAD(MII_REG_SR) &  0x0004));
    EdbgOutputDebugString( "...Done\r\n");

	// CS&ZHL MAY-30-2011: show MII_REG_CR and MII_REG_SR
    CRVal = MIIREAD(MII_REG_CR);
    EdbgOutputDebugString( "MII_REG_CR = 0x%x\r\n", CRVal);
    RegVal = MIIREAD(MII_REG_SR);
    EdbgOutputDebugString( "MII_REG_SR = 0x%x\r\n", RegVal);


    fIs100MB = FALSE;
    //Get Link states
    if(Am79c874==i)
    {
        RegVal = MIIREAD(MII_AM79C874_DR);
        if(RegVal & 0x0400)
        {
            EdbgOutputDebugString( "FEC is linked, Speed  100Mbps \r\n");
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
            EdbgOutputDebugString( "FEC is linked,  Speed  10Mbps \r\n");
            
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
    }
	else if(LAN8700==i)
    {
        RegVal = MIIREAD(MII_LAN8700_SCSR);
        
        HcdSpeed=((RegVal & 0x001C)>>2);
        if(1==HcdSpeed)
        {
            EdbgOutputDebugString( "FEC is linked,  Speed  10Mbps , Half Duplex \r\n");
            fIsHalfDup=TRUE;
        }
        else if(5==HcdSpeed)
        {
            EdbgOutputDebugString( "FEC is linked,  Speed  10Mbps , Full Duplex \r\n");
        }       
        else if(2==HcdSpeed)
        {
            EdbgOutputDebugString( "FEC is linked,  Speed  100Mbps , Half Duplex \r\n");
            fIsHalfDup=TRUE;
            fIs100MB = TRUE;
        }
        else if(6==HcdSpeed)
        {
            fIs100MB = TRUE;
            EdbgOutputDebugString( "FEC is linked,  Speed  100Mbps , Full Duplex \r\n");
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
                EdbgOutputDebugString( "FEC is linked,  Speed  10Mbps , Full Duplex \r\n");
            }
            else
            {
                EdbgOutputDebugString( "FEC is linked,  Speed  10Mbps , Half Duplex \r\n");
                fIsHalfDup=TRUE;
            }
        }
        else
        {
            if(2 & HcdSpeed)
            {
                EdbgOutputDebugString( "FEC is linked,  Speed  100Mbps , Full Duplex \r\n");
                fIs100MB = TRUE;
            }
            else
            {
                EdbgOutputDebugString( "FEC is linked,  Speed  100Mbps , Half Duplex \r\n");
                fIsHalfDup=TRUE;
                fIs100MB = TRUE;
            }    
        }
    }
    //else if(DM9161A == i)		// CS&ZHL MAY-30-2011: supporting DM9161
	//{
	//}
    else 
    {
        //using standard registers
        RegVal = MIIREAD(MII_REG_SR);
        
		/*
        if(RegVal & PHY_STAT_10HDX)
        {
            EdbgOutputDebugString( "FEC is linked,  Speed  10Mbps , Half Duplex \r\n");
            fIsHalfDup=TRUE;
        }
        else if(RegVal & PHY_STAT_10FDX)
        {
            EdbgOutputDebugString( "FEC is linked,  Speed  10Mbps , Full Duplex \r\n");
        }       
        else if(RegVal & PHY_STAT_100HDX)
        {
            EdbgOutputDebugString( "FEC is linked,  Speed  100Mbps , Half Duplex \r\n");
            fIsHalfDup=TRUE;
            fIs100MB = TRUE;
        }
        else if(RegVal & PHY_STAT_100FDX)
        {
            fIs100MB = TRUE;
            EdbgOutputDebugString( "FEC is linked,  Speed  100Mbps , Full Duplex \r\n");
        }
		*/

		// CS&ZHL MAY-30-2011: check PHY config
		if(RegVal & (PHY_STAT_10HDX | PHY_STAT_100HDX))
		{
            fIsHalfDup=TRUE;
            EdbgOutputDebugString( "FEC is linked,  Half Duplex\r\n");
		}

		if(RegVal & (PHY_STAT_100FDX | PHY_STAT_100HDX))
		{
            fIs100MB = TRUE;
            EdbgOutputDebugString( "FEC is linked,  Speed  100Mbps\r\n");
		}
     }

    if (g_FEC.bUseRMII)
    {
        // select the speed of the RMII bock
        if (fIs100MB)
        {
            INSREG32BF(&g_FEC.REG->MIIGSK_CFGR,FEC_MIIGSK_CFGR_FRCONT,FEC_MIIGSK_CFGR_FRCONT_100);
        }
        else
        {
            INSREG32BF(&g_FEC.REG->MIIGSK_CFGR,FEC_MIIGSK_CFGR_FRCONT,FEC_MIIGSK_CFGR_FRCONT_10);
        }
    }

    if(TRUE==fIsHalfDup)
        INSREG32BF(&g_FEC.REG->RCR, FEC_RCR_DRT, 1);
    
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
UINT16 FECSendFrame(UINT8 *pData, UINT32 length)
{
    BOOL rc = FALSE;
    volatile PBUFFER_DESC BufferDescPointer;
    PUCHAR MemAddr;
    UINT32  index;
    UINT32 dwCounter = 0;
    
#if 0
   EdbgOutputDebugString( "+S %X %u \n\r",  pData, length  );
#endif
    OALMSGS(OAL_ETHER&&OAL_VERBOSE, ( L"+S (0x%08x, %d)\r\n", pData, length));

    BufferDescPointer = g_FEC.CurrentTx;

    
    for(;;)
    {
        if (!(BufferDescPointer->ControlStatus & BD_ENET_TX_READY))
        {
            break;
        }
        OALStall(10);
        if (dwCounter++ > 200)
        {
            OALMSGS(OAL_ERROR, (L"ERROR: FECSendFrame: Send timeout\r\n"));
            //EdbgOutputDebugString("!!!ERROR TX T buffers are full ControlStatus = %X\r\n", BufferDescPointer->ControlStatus);
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
    return (UINT16)!rc;
}

//------------------------------------------------------------------------------
//
// Function: FECGetFrame
//
//------------------------------------------------------------------------------
UINT16 FECGetFrame(UINT8 *pData, UINT16 *pLength)
{
    USHORT PacketSize = 0;
    volatile PBUFFER_DESC BufferDescPointer;
    PUCHAR pReceiveBuffer;
    USHORT ControlStatus;
//   UINT32  IntStatues;

#if 0
   EdbgOutputDebugString( "+FECGetFrame %X %u \n\r", pData, *pLength  );
#endif

    OALMSGS(OAL_ETHER&&OAL_VERBOSE, (L"+FECGetFrame(0x%08x, %d)\r\n", pData, *pLength));
    
    BufferDescPointer = g_FEC.CurrentRx;
    //EdbgOutputDebugString("The value of ControlStatus is = %X\r\n", BufferDescPointer->ControlStatus);
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
            //EdbgOutputDebugString("!!!ERROR Rx ControlStatus is = %X\r\n", BufferDescPointer->ControlStatus);    
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
        
        if(PacketSize > 0)        break;
    }

    *pLength = PacketSize;
    OALMSGS(OAL_ETHER&&OAL_VERBOSE, ( L"-FECGetFrame(PacketSize = %d)\r\n", PacketSize));
    
#if 0
if(PacketSize != 0)
{
    EdbgOutputDebugString( "-RX *pLength  %u \n\r",  *pLength  );
    EdbgOutputDebugString( "-RX ControlStatus  %u \n\r", ControlStatus  );
}
#endif

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
        (filter & PACKET_TYPE_ALL_MULTICAST) != 0
    ) {
        OUTREG32(&g_FEC.REG->GALR,  0xFFFFFFFF);
        OUTREG32(&g_FEC.REG->GAUR, 0xFFFFFFFF);
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
BOOL FECMulticastList(UINT8 *pAddresses, UINT32 count)
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
        CLRREG32(&g_FEC.REG->GAUR, 0xFFFFFFFF);
        CLRREG32(&g_FEC.REG->GALR, 0xFFFFFFFF);
    }

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-FECMulticastList(rc = 1)\r\n"));
    return TRUE;
}

