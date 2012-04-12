//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  fir.cpp
//
//   This file implements the device specific functions for iMX51 fir device.
//
//------------------------------------------------------------------------------
#include "IrFir.h"

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

PHYSICAL_ADDRESS FirPhysBufferAddr;
UINT32           pFirVirtBufferAddr;

#ifdef FIR_SDMA_SUPPORT
// Variables used exclusively for SDMA
UINT8            FirDmaChanRx;      // Channel for Reception
UINT8            FirDmaChanTx;      // Channel for Transmission
#endif

//------------------------------------------------------------------------------
// Local Functions

#if 0 // Remove-W4: Warning C4505 workaround
//-----------------------------------------------------------------------------
//
// Function: FirDumpReg
//
// This function dump the value of registers of Fir device.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID FirDumpReg(VOID)
{
  DEBUGMSG(ZONE_FUNCTION, (TEXT("FIRITCR  0x%08x\r\n"), INREG32(&g_pVFiriReg->FIRI_TCR)));
  DEBUGMSG(ZONE_FUNCTION, (TEXT("FIRITCTR 0x%08x\r\n"), INREG32(&g_pVFiriReg->FIRI_TCTR)));
  DEBUGMSG(ZONE_FUNCTION, (TEXT("FIRIRCR  0x%08x\r\n"), INREG32(&g_pVFiriReg->FIRI_RCR)));
  DEBUGMSG(ZONE_FUNCTION, (TEXT("FIRITSR  0x%08x\r\n"), INREG32(&g_pVFiriReg->FIRI_TSR)));
  DEBUGMSG(ZONE_FUNCTION, (TEXT("FIRIRSR  0x%08x\r\n"), INREG32(&g_pVFiriReg->FIRI_RSR)));
  DEBUGMSG(ZONE_FUNCTION, (TEXT("FIRICR   0x%08x\r\n"), INREG32(&g_pVFiriReg->FIRI_CR)));
}
#endif

//-------------------------------------------------------------------------------
// Function: NdisToFirPacket
//
//  This function reads the NDIS packet into a contiguous buffer.
//
// Parameters:
//      Packet
//          [in] .
//      irPacketBuf
//          [in] .
//      irPacketBufLen
//          [in] .
//      irPacketLen
//          [in] .
//
// Returns:
//    This function returns TRUE if the packet check is successful.
//
//-----------------------------------------------------------------------------
BOOLEAN NdisToFirPacket( pFirDevice_t thisDev, PNDIS_PACKET Packet,
    UCHAR * irPacketBuf, UINT irPacketBufLen, UINT * irPacketLen )
{
    PNDIS_BUFFER ndisBuf;
    UINT ndisPacketBytes = 0;
    UINT ndisPacketLen;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(thisDev);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: +NdisToFirPacket(0x%x)\r\n"), (UINT) thisDev));

    // Get the packet's entire length and its first NDIS buffer
    NdisQueryPacket(Packet, NULL, NULL, &ndisBuf, &ndisPacketLen);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: NdisToFirPacket, number of bytes: %d\r\n"), ndisPacketLen));

    // Make sure that the packet is big enough to be legal.
    // It consists of an A, C, and variable-length I field.
    if (ndisPacketLen < IR_ADDR_SIZE + IR_CONTROL_SIZE)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: packet too short in NdisToFirPacket (%d bytes)\r\n"), ndisPacketLen));
        return FALSE;
    }

    // Make sure that we won't overwrite our contiguous buffer.
    if (ndisPacketLen > irPacketBufLen)
    {
        //  The packet is too large
        //  Tell the caller to retry with a packet size large
        //  enough to get past this stage next time.
        DEBUGMSG(ZONE_FUNCTION,
            (TEXT("Fir: Packet too large in NdisToIrPacket (%d=%xh bytes), MAX_IRDA_DATA_SIZE=%d, irPacketBufLen=%d\r\n"),
            ndisPacketLen, ndisPacketLen, MAX_IRDA_DATA_SIZE, irPacketBufLen));

        *irPacketLen = ndisPacketLen;
        return FALSE;
    }

    //  Read the NDIS packet into a contiguous buffer.
    //  We have to do this in two steps so that we can compute the
    //  FCS BEFORE applying escape-byte transparency.
    while (ndisBuf)
    {
        UCHAR *bufData;
        UINT bufLen;

        NdisQueryBuffer(ndisBuf, (PVOID *)&bufData, &bufLen);

        if (ndisPacketBytes + bufLen > ndisPacketLen)
        {
            // Packet was corrupt -- it misreported its size.
            *irPacketLen = 0;
            return FALSE;
        }

        NdisMoveMemory((PVOID)(irPacketBuf+ndisPacketBytes), (PVOID)bufData, (ULONG)bufLen);
        ndisPacketBytes += bufLen;
        NdisGetNextBuffer(ndisBuf, &ndisBuf);
    }

    // Do a validity check on the length of the packet
    if (ndisPacketBytes != ndisPacketLen)
    {
        // Packet was corrupt -- it misreported its size.
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: Packet corrupt in NdisToIrPacket\r\n")));
        *irPacketLen = 0;
        return FALSE;
    }

    *irPacketLen = ndisPacketBytes;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: -NdisToFirPacket\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: FirSendPacketComplete
//
//  This function stops Fir transmitter when transmission completed.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
// Remove-Prefast: Warning 28167 workaround
PREFAST_SUPPRESS(28167,"This Warning can be skipped!")
VOID FirSendPacketComplete( pFirDevice_t thisDev )
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    PLIST_ENTRY ListEntry;
    PNDIS_PACKET Packet;
    UINT32 tempRegVal;

    DEBUGMSG(ZONE_SEND, (TEXT("Fir: +FirSendPacketComplete(0x%x)\r\n"), (UINT) thisDev));
    
    ListEntry = MyRemoveHeadList(&thisDev->SendQueue);

    if (!ListEntry)
        ASSERT(0);
    else
    {
        Packet = CONTAINING_RECORD(ListEntry, NDIS_PACKET, MiniportReserved);

        // Check whether transmission error
        tempRegVal = INREG32(&g_pVFiriReg->FIRI_TSR);

        // Clear interrupt status
        OUTREG32(&g_pVFiriReg->FIRI_TSR, XMIT_INTR_MASK);

        // Disable FIRI transmitter
        INSREG32BF(&g_pVFiriReg->FIRI_TCR, FIRI_TCR_TE, FIRI_TCR_TE_DISABLE);

        if (tempRegVal & CSP_BITFMASK(FIRI_TSR_TFU)) 
        {
            status = NDIS_STATUS_SUCCESS; 
            // Disable and enable the RX, this will reset the RX FIFO controller
            INSREG32BF(&g_pVFiriReg->FIRI_RCR, FIRI_RCR_RE, FIRI_RCR_RE_DISABLE);
            INSREG32BF(&g_pVFiriReg->FIRI_RCR, FIRI_RCR_RE, FIRI_RCR_RE_ENABLE);
            //thisDev->HangChk = TRUE;
            DEBUGMSG(ZONE_SEND, (TEXT("Fir: FirSendPacketComplete: Transmit Underrun, 0x%08x, Data size is %d"), tempRegVal, thisDev->writeBufLen));
        } 
        else if (tempRegVal & (CSP_BITFMASK(FIRI_TSR_TPE) | CSP_BITFMASK(FIRI_TSR_TC))) 
        {
            status = NDIS_STATUS_SUCCESS;
            DEBUGMSG(ZONE_SEND, (TEXT("Fir: FirSendPacketComplete: Transfer completed!!! IntrStatus: 0x%08x"), tempRegVal));
        } 
        else 
        {
            status = NDIS_STATUS_FAILURE; 
            DEBUGMSG(ZONE_ERROR, (TEXT("Fir: FirSendPacketComplete: Unexpected interrupt, 0x%08x"), tempRegVal));
        }

        // Notify NDIS of TX complete
        NdisReleaseSpinLock(&thisDev->Lock);
        NdisMSendComplete(thisDev->ndisAdapterHandle, Packet, status);
        NdisAcquireSpinLock(&thisDev->Lock);
    }

    DEBUGMSG(ZONE_SEND, (TEXT("Fir: -FirSendPacketComplete\r\n")));
}

//-----------------------------------------------------------------------------
//
// Function: FirReceivePacket
//
//  This function queue the received packet.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    This function returns TRUE if the packet receiving is successful.
//
//-----------------------------------------------------------------------------

BOOLEAN FirReceivePacket( pFirDevice_t thisDev )
{
    const UINT fcsSize = FAST_IR_FCS_SIZE;

    DEBUGMSG(ZONE_RECV, (TEXT("Fir: +FirReceivePacket(0x%x)\r\n"), (UINT) thisDev));

    // Check interrupt status of FIRI
    UINT32 iIntrTemp = INREG32(&g_pVFiriReg->FIRI_RSR);

    // Clear Rx itnerrupt status
    OUTREG32(&g_pVFiriReg->FIRI_RSR, RCV_INTR_MASK);
    // check for all error conditions.
    if (iIntrTemp &
        (CSP_BITFVAL(FIRI_RSR_CRCE, FIRI_RSR_CRCE_FAILURE) |
         CSP_BITFVAL(FIRI_RSR_DDE, FIRI_RSR_DDE_ILLEGAL)   |
         CSP_BITFVAL(FIRI_RSR_RFO, FIRI_RSR_RFO_OVERRUN)))
    {
        DEBUGMSG(ZONE_RECV, (TEXT("Fir: FirReceivePacket Receive error , 0x%X \r\n"), iIntrTemp));
        return FALSE;
    }
#ifndef FIR_SDMA_SUPPORT

    UINT iDataInRxFIFO = g_pVFiriReg->FIRI_RSR>>8;
    thisDev->rcvDataLength = 0;  //Reset the receiver datelength

    while(iDataInRxFIFO--)
    {
        *(UINT8 *)((UINT32)thisDev->readBuf + thisDev->rcvDataLength++) = *(UINT8 *)(&(g_pVFiriReg->FIRI_RXFIFO));

        DEBUGMSG(ZONE_RECV, (TEXT("Fir: Receive 0x%x\r\n"), *(UINT8 *)((UINT32)thisDev->readBuf + thisDev->rcvDataLength - 1)));
    }
    
    // Disable FIRI receiver
    CSP_BITFINS(g_pVFiriReg->FIRI_RCR, FIRI_RCR_RE, FIRI_RCR_RE_DISABLE);
#endif

    ASSERT(thisDev->rcvDataLength < MAX_NDIS_DATA_SIZE);

    // Chop off FCS
    thisDev->rcvDataLength -= fcsSize; 

    if (thisDev->rcvDataLength <= MAX_NDIS_DATA_SIZE &&
        thisDev->rcvDataLength >= IR_ADDR_SIZE + IR_CONTROL_SIZE)
    {
#ifdef FIR_SDMA_SUPPORT
        // Queue this rcv packet.
        QueueReceivePacket(thisDev, thisDev->readBuf, thisDev->rcvDataLength, TRUE);
#else
        // Queue this rcv packet.
        QueueReceivePacket(thisDev, thisDev->readBuf, thisDev->rcvDataLength, TRUE);
#endif
    }
    else
        DEBUGMSG(ZONE_RECV, (TEXT("Fir: invalid packet size in FirReceivePacket; 0x%X > 0x%X\r\n"),
            thisDev->rcvDataLength, MAX_RCV_DATA_SIZE));

    DEBUGMSG(ZONE_RECV, (TEXT("Fir: -FirReceivePacket\r\n")));
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: FirClearInterrupt
//
//  This function clears Fir interrupts.
//
// Parameters:
//    None.
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID FirClearInterrupt( VOID )
{
    // Clear the interrupt status registers
    OUTREG32(&g_pVFiriReg->FIRI_TSR, XMIT_INTR_MASK);
    OUTREG32(&g_pVFiriReg->FIRI_RSR, RCV_INTR_MASK);
}


//-----------------------------------------------------------------------------
//
// Function: FirEnableInterrupt
//
//  This function denables Fir interrupts.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID FirEnableInterrupt( pFirDevice_t thisDev )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(thisDev);
    // Enabling the interrupts in the interrupt handler itself
}


//-----------------------------------------------------------------------------
//
// Function: FirDisableInterrupt
//
//  This function disables Fir interrupts.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID FirDisableInterrupt( pFirDevice_t thisDev )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(thisDev);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: +FirDisableInterrupt\r\n")));

    // Disable Interrupts from FIRI
    INSREG32BF(&g_pVFiriReg->FIRI_TCR,  FIRI_TCR_TCIE,  FIRI_TCR_TCIE_DISABLE);
    INSREG32BF(&g_pVFiriReg->FIRI_TCR,  FIRI_TCR_TPEIE, FIRI_TCR_TPEIE_DISABLE);

    INSREG32BF(&g_pVFiriReg->FIRI_RCR,  FIRI_RCR_RPEIE, FIRI_RCR_RPEIE_DISABLE);
    INSREG32BF(&g_pVFiriReg->FIRI_RCR,  FIRI_RCR_PAIE,  FIRI_RCR_PAIE_DISABLE);
    INSREG32BF(&g_pVFiriReg->FIRI_RCR,  FIRI_RCR_RFOIE, FIRI_RCR_RFOIE_DISABLE);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: -FirDisableInterrupt\r\n")));
}

#ifdef FIR_SDMA_SUPPORT
//-----------------------------------------------------------------------------
//
// Function: FirConfigSDMA
//
//  This function setup the common buffer
//  for the Fir SDMA transacation and
//  initializes the SDMA definitions for FIR
//
// Parameters:
//    None .
//
// Returns:
//    This function returns SDMA initialization status.
//
//-----------------------------------------------------------------------------
BOOLEAN FirConfigSDMA()
{
    BOOLEAN bRet = TRUE;    
    UINT8 FirDmaChPriority = BSPFirGetChannelPriority();

    DEBUGMSG(ZONE_INIT, (TEXT("Fir: +FirConfigSDMA\r\n")));
 
    // Configure Rx channel for FIRI
    // Open virtual DMA channels for Rx
    FirDmaChanRx = DDKSdmaOpenChan(DDK_DMA_REQ_FIRI_RX, FirDmaChPriority);
    if (!FirDmaChanRx)
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:FirConfigSDMA(Rx): SdmaOpenChan for input failed.\r\n")));
        bRet = FALSE;
        goto done;
    }
    DEBUGMSG(ZONE_FUNCTION,(_T("Channel Allocated(Rx) : %d\r\n"),FirDmaChanRx));

    // Allocate DMA chain buffer for Rx
    if (!DDKSdmaAllocChain(FirDmaChanRx, FIRI_MAX_RX_DESC_COUNT))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:FirConfigSDMA(Rx): DDKSdmaAllocChain for input failed.\r\n")));
        bRet = FALSE;
        goto done;
    }  

    // Initialize the chain and set the watermark level for Rx
    if (!DDKSdmaInitChain(FirDmaChanRx, FIRI_DMA_RX_WATERMARK))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:FirConfigSDMA(Rx): DDKSdmaInitChain failed.\r\n")));
        bRet = FALSE;
        goto done;
    }
    // Configure Tx Channel for FIRI
    // Open virtual DMA channels for Tx
    FirDmaChanTx = DDKSdmaOpenChan(DDK_DMA_REQ_FIRI_TX, FirDmaChPriority);
    if (!FirDmaChanTx)
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:FirConfigSDMA(Tx): SdmaOpenChan(Rx) for input failed.\r\n")));
        bRet = FALSE;
        goto done;
    }
    DEBUGMSG(ZONE_FUNCTION,(_T("Channel Allocated(Tx) : %d\r\n"),FirDmaChanTx));

    // Allocate DMA chain buffer for Tx
    if (!DDKSdmaAllocChain(FirDmaChanTx, FIRI_MAX_RX_DESC_COUNT))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:FirConfigSDMA(Tx): DDKSdmaAllocChain for input failed.\r\n")));
        bRet = FALSE;
        goto done;
    }  

    // Initialize the chain and set the watermark level for Tx 
    if (!DDKSdmaInitChain(FirDmaChanTx, FIRI_DMA_TX_WATERMARK))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:FirConfigSDMA(Tx): DDKSdmaInitChain failed.\r\n")));
        bRet = FALSE;
        goto done;
    }

done:

    DEBUGMSG(ZONE_INIT, (TEXT("Fir: -FirConfigSDMA\r\n")));
    return bRet; 
}

//-----------------------------------------------------------------------------
//
// Function: FirCloseSDMA
//
//  This function unmpas the common buffer
//  for the Fir SDMA transacation also
//  close all the open channels for SDMA
//
// Parameters:
//    None.
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID FirCloseSDMA()
{
    
    DEBUGMSG(ZONE_DEINIT, (TEXT("Fir: +FirDeInitDMA\r\n")));

    if (FirDmaChanTx)
    {
        DDKSdmaClearChainStatus(FirDmaChanTx); // preclear interrupt
        DDKSdmaCloseChan(FirDmaChanTx);
    }
    if (FirDmaChanRx)
    {
        DDKSdmaClearChainStatus(FirDmaChanRx); // preclear interrupt
        DDKSdmaCloseChan(FirDmaChanRx);
    }

    DEBUGMSG(ZONE_DEINIT, (TEXT("Fir: -FirDeInitDMA\r\n")));
}
#endif

//-----------------------------------------------------------------------------
//
// Function: FirInitialize
//
//  This function initializes the Fir device.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    This function returns initialization status.
//
//-----------------------------------------------------------------------------
BOOLEAN FirInitialize(pFirDevice_t thisDev)
{
    PHYSICAL_ADDRESS Addr = {thisDev->FirPhyAddr, 0};

    DEBUGMSG(ZONE_INIT, (TEXT("Fir: +FirInitialize\r\n")));
    
    // Map peripheral physical address to virtual address
    g_pVFiriReg = (PCSP_FIRI_REG) MmMapIoSpace(Addr, sizeof(CSP_FIRI_REG), FALSE);

    // Check if virtual mapping failed
    if (g_pVFiriReg == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Fir:  MmMapIoSpace failed!\r\n")));
        return FALSE;
    }

    // Use kernel IOCTL to translate the FIR base address into an IRQ since
    // the IRQ value differs based on the SoC. Note that DEVICE_LOCATION
    // fields except IfcType and LogicalLoc are ignored for internal SoC 
    // components.
    DEVICE_LOCATION devLoc;     
    devLoc.IfcType = Internal;
    devLoc.LogicalLoc = thisDev->FirPhyAddr;

    if (!KernelIoControl(IOCTL_HAL_REQUEST_IRQ, &devLoc, sizeof(devLoc),
        &thisDev->sysIntrFir, sizeof(thisDev->sysIntrFir), NULL))
    {
        DEBUGMSG(1, (_T("Cannot obtain FIR IRQ!\r\n")));
        return FALSE;
    }

    //Allocate buffer
    pFirVirtBufferAddr = NULL;

    pFirVirtBufferAddr = (UINT32)AllocPhysMem((MAX_IRDA_DATA_SIZE * 2), 
        PAGE_READWRITE | PAGE_NOCACHE, 0, 0, (PULONG)&(FirPhysBufferAddr));
    if (!pFirVirtBufferAddr) 
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:FirInitialize: Invalid DMA buffer physical address.\r\n")));
        return FALSE;
    }

#ifdef FIR_SDMA_SUPPORT
    if(!FirConfigSDMA())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Fir:  FirConfigSDMA failed!\r\n")));
        return FALSE;
    }
#endif

    DEBUGMSG(ZONE_INIT, (TEXT("Fir: -FirInitialize\r\n")));
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: FirDeinitialize
//
//  This function deinitializes the Fir device.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID FirDeinitialize( pFirDevice_t thisDev )
{
    DEBUGMSG(ZONE_DEINIT, (TEXT("Fir: +FirDeinitialize\r\n")));
    // Free FIRI register
    if (g_pVFiriReg)
    {
        MmUnmapIoSpace(g_pVFiriReg, sizeof(CSP_FIRI_REG));
        g_pVFiriReg = NULL;
    }
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &thisDev->sysIntrFir, sizeof(DWORD), NULL, 0, NULL);
    thisDev->sysIntrFir = (DWORD)SYSINTR_UNDEFINED;

    if (pFirVirtBufferAddr)
    {
        FreePhysMem((LPVOID)pFirVirtBufferAddr);
    }
    pFirVirtBufferAddr = NULL;

#ifdef FIR_SDMA_SUPPORT
    FirCloseSDMA();
#endif

    DEBUGMSG(ZONE_DEINIT, (TEXT("Fir: -FirDeinitialize\r\n")));
}


//-----------------------------------------------------------------------------
//
// Function: FirClose
//
//  This function closes the Fir device.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID FirClose( pFirDevice_t thisDev )
{
    DEBUGMSG(ZONE_CLOSE, (TEXT("Fir: +FirClose\r\n")));

#ifndef FIR_SDMA_SUPPORT
    // Flush the fifo
    DWORD dwDataInFifo = EXTREG32BF(&g_pVFiriReg->FIRI_RSR, FIRI_RSR_RFP);
    if (dwDataInFifo > 0)
    {
        UINT8 uTemp;
        for (UINT i = 0; i < dwDataInFifo; i++)
            uTemp = *(UINT8 *)(&(g_pVFiriReg->FIRI_RXFIFO));
    }

    while (EXTREG32BF(&g_pVFiriReg->FIRI_TSR, FIRI_TSR_TFP) > 0)
    Sleep(0);
#endif

    // Free the buffers
    OUTREG32(&g_pVFiriReg->FIRI_TSR, XMIT_INTR_MASK);
    OUTREG32(&g_pVFiriReg->FIRI_RSR, RCV_INTR_MASK);
    OUTREG32(&g_pVFiriReg->FIRI_CR, 0);
    OUTREG32(&g_pVFiriReg->FIRI_TCR, 0);
    OUTREG32(&g_pVFiriReg->FIRI_RCR, 0);

    // Free the buffers
    thisDev->writeBuf = NULL;
    thisDev->readBuf = NULL;

    // BSP level diabling
    BSPFirEnableClock(FALSE);  // Disable Fir Clock
    BSPFirSetSIRIOMUX();       // Set the IOMUX for SIR
    BSPIrdaEnable(FALSE);      // Disable IRDA  
    BSPIrdaSetMode(SIR_MODE);  // Set IRDA mode to SIR

#ifdef FIR_SDMA_SUPPORT
    // preclears the status 
    DDKSdmaClearChainStatus(FirDmaChanRx);  
    DDKSdmaClearChainStatus(FirDmaChanTx);

    // Stop the channels
    DDKSdmaStopChan(FirDmaChanRx, FALSE);
    DDKSdmaStopChan(FirDmaChanTx, FALSE);
#endif

    DEBUGMSG(ZONE_CLOSE, (TEXT("Fir: -FirClose\r\n")));
}


//-----------------------------------------------------------------------------
//
// Function: FirOpen
//
//  This function opens the Fir device.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    This function returns status of open device.
//
//-----------------------------------------------------------------------------
BOOLEAN FirOpen( pFirDevice_t thisDev )
{
    DEBUGMSG(ZONE_OPEN, (TEXT("Fir: +FirOpen\r\n")));

    // BSP Initialization code
    BSPFirEnableClock(TRUE);   // Enable Fir Clock
    BSPFirSetClock();          // Set Fir clock to 24 MHz
    BSPFirSetIOMUX();          // Set the IOMUX for FIR
    BSPIrdaEnable(TRUE);       // Enable IRDA   
    BSPIrdaSetMode(FIR_MODE);  // Set IRDA mode to FIR

    // Setup FIR speed
    FirSetSpeed(thisDev);

    thisDev->writeBuf = (PUCHAR)pFirVirtBufferAddr;
    thisDev->readBuf =  (PUCHAR)(pFirVirtBufferAddr + MAX_IRDA_DATA_SIZE);

    // Setup FIRICR
    INSREG32BF(&g_pVFiriReg->FIRI_CR, FIRI_CR_BL, BURST_LENGTH_DEFAULT);

    // Setup FIRITCR (transmitter)
#ifdef FIR_SDMA_SUPPORT
    OUTREG32(&g_pVFiriReg->FIRI_TCR, 
        CSP_BITFVAL(FIRI_TCR_TDT,   FIRI_TCR_TDT_16)        | //Set DMA request level as 16 byte
        CSP_BITFVAL(FIRI_TCR_TPP,   FIRI_TCR_TPP_NO_INVERT) | // Set the Transmitter Polarity to normal.
        CSP_BITFVAL(FIRI_TCR_TCIE,  FIRI_TCR_TCIE_ENABLE)   | // Transmit Complete Interrupt is enabled
        CSP_BITFVAL(FIRI_TCR_TPEIE, FIRI_TCR_TPEIE_ENABLE)  | // Transmitter Packet End Interrupt is enabled
        CSP_BITFVAL(FIRI_TCR_TM,    FIRI_TCR_TM_FIR)        | // FIR mode
        CSP_BITFVAL(FIRI_TCR_TE,    FIRI_TCR_TE_DISABLE));    // Disable Transmitter
#else
    OUTREG32(&g_pVFiriReg->FIRI_TCR,
        CSP_BITFVAL(FIRI_TCR_TDT,   FIRI_TCR_TDT_16)         | //Set DMA request level as 16 byte
        CSP_BITFVAL(FIRI_TCR_TPP,   FIRI_TCR_TPP_NO_INVERT)  | // Set the Transmitter Polarity to normal.
        CSP_BITFVAL(FIRI_TCR_TCIE,  FIRI_TCR_TCIE_ENABLE)    | // Transmit Complete Interrupt is enabled
        CSP_BITFVAL(FIRI_TCR_TPEIE, FIRI_TCR_TPEIE_ENABLE)   | // Transmitter Packet End Interrupt is enabled
        CSP_BITFVAL(FIRI_TCR_TFUIE, FIRI_TCR_TFUIE_ENABLE)   | // Transmitter FIFO Underrun Interrupt is enabled
        CSP_BITFVAL(FIRI_TCR_TM,    FIRI_TCR_TM_FIR));         // FIR mode
#endif
    // Clear FIRITCTR
    OUTREG32(&g_pVFiriReg->FIRI_TCTR, 0);

#ifdef FIR_SDMA_SUPPORT
    // Setup FIRIRCR (receiver)
    OUTREG32(&g_pVFiriReg->FIRI_RCR,
        CSP_BITFVAL(FIRI_RCR_RPEDE, FIRI_RCR_RPEDE_ENABLE) | // Receiver Packet End DMA Request is enabled
        CSP_BITFVAL(FIRI_RCR_RDT,   FIRI_RCR_RDT_16)       | // Set DMA request level as 16 byte
        CSP_BITFVAL(FIRI_RCR_RPP,   FIRI_RCR_RPP_INVERT)   | // RM RPP not inverted
        CSP_BITFVAL(FIRI_RCR_RPEIE, FIRI_RCR_RPEIE_ENABLE) | // Receiver Packet End Interrupt is enabled 
        CSP_BITFVAL(FIRI_RCR_RM,    FIRI_RCR_RM_FIR)       | // RM FIR
        CSP_BITFVAL(FIRI_RCR_RE,    FIRI_RCR_RE_DISABLE));   // Disable Transmitter
#else
    OUTREG32(&g_pVFiriReg->FIRI_RCR,
        CSP_BITFVAL(FIRI_RCR_RPEDE, FIRI_RCR_RPEDE_ENABLE) | // Receiver Packet End DMA Request is enabled
        CSP_BITFVAL(FIRI_RCR_RDT,   FIRI_RCR_RDT_16)       | // Set DMA request level as 16 byte
        CSP_BITFVAL(FIRI_RCR_RPP,   FIRI_RCR_RPP_INVERT)   | // RM RPP inverted
        CSP_BITFVAL(FIRI_RCR_RPEIE, FIRI_RCR_RPEIE_ENABLE) | // Receiver Packet End Interrupt is enabled
        CSP_BITFVAL(FIRI_RCR_PAIE,  FIRI_RCR_PAIE_ENABLE)  | // Packet Abort Interrupt is enable
        CSP_BITFVAL(FIRI_RCR_RFOIE, FIRI_RCR_RFOIE_ENABLE) | // Receiver FIFO Overrun Interrupt is enabled
        CSP_BITFVAL(FIRI_RCR_RM,    FIRI_RCR_RM_FIR));       // FIR mode
#endif
    // Clear FIRITSR and FIRIRSR
    OUTREG32(&g_pVFiriReg->FIRI_TSR, XMIT_INTR_MASK);
    OUTREG32(&g_pVFiriReg->FIRI_RSR, RCV_INTR_MASK);

    thisDev->rcvDataLength = 0;
    thisDev->writePending  = FALSE;

#ifdef FIR_SDMA_SUPPORT
    // Setup recv DMA channel and enable it
    DDKSdmaSetBufDesc(FirDmaChanRx, 0, 
        DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP, 
        (FirPhysBufferAddr.LowPart + MAX_IRDA_DATA_SIZE),
        0, DDK_DMA_ACCESS_8BIT, MAX_IRDA_DATA_SIZE);

    DDKSdmaStartChan(FirDmaChanRx);    
#endif
 
    DEBUGMSG(ZONE_OPEN, (TEXT("Fir: -FirOpen\r\n")));
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: FirSendPacketQ
//
//  This function sends the packet from the send queue.
//
// Parameters:
//      thisDev
//          [in] .
//      firstBufIsPending
//          [in] .
//
// Returns:
//    This function returns status of send packet.
//
//-----------------------------------------------------------------------------
NDIS_STATUS FirSendPacketQ( pFirDevice_t thisDev, BOOLEAN firstBufIsPending )
{
    NDIS_STATUS Result = NDIS_STATUS_PENDING;
    PNDIS_PACKET packetToSend;
    PNDIS_IRDA_PACKET_INFO packetInfo;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(firstBufIsPending);

    DEBUGMSG(ZONE_SEND, (TEXT("Fir: +FirSendPacketQ\r\n"), (UINT) thisDev));

    if (IsListEmpty(&thisDev->SendQueue))
    {
        // Don't remove this branch, I do see it goes to this branch
        // when hardware hang.
        thisDev->HangChk = TRUE;
        Result = NDIS_STATUS_FAILURE;
    }
    else
    {
        packetToSend = HEAD_SEND_PACKET(thisDev);

        // Check for min turnaround time set.
        packetInfo = GetPacketInfo(packetToSend);

        //
        // NOTE: Don't use NdisStallExecution for turnaround delay since
        //       you shouldn't stall for more than 60 us. Calling
        //       NdisStallExecution for large times will degrade system
        //       performance.
        //
        if (packetInfo->MinTurnAroundTime)
        {
            UINT usecToWait = packetInfo->MinTurnAroundTime;
            UINT msecToWait;
            packetInfo->MinTurnAroundTime = 0;

            // Ndis timer has a 1ms granularity (in theory).  Let's round off.

            msecToWait = (usecToWait<=1000) ? 1 : (usecToWait+500)/1000;
            NdisMSetTimer(&thisDev->TurnaroundTimer, msecToWait);

            DEBUGMSG(ZONE_SEND, (TEXT("Fir: FirSendPacketQ - do min TAT wait\r\n")));
            return NDIS_STATUS_PENDING; // Say we're successful.  We'll come back here.
        }

        NdisToFirPacket(thisDev, packetToSend, (UCHAR *) thisDev->writeBuf,
            MAX_IRDA_DATA_SIZE, &thisDev->writeBufLen);

        // Disable all interrupt first
        FirDisableInterrupt(thisDev);

        // Disable FIRI receiver
        INSREG32BF(&g_pVFiriReg->FIRI_RCR, FIRI_RCR_RE, FIRI_RCR_RE_DISABLE);

        thisDev->nowReceiving = FALSE;
        thisDev->writePending = TRUE;

        // Setup FIRI data size
        INSREG32BF(&g_pVFiriReg->FIRI_TCTR, FIRI_TCTR_TPL, TPL_VALUE(thisDev->writeBufLen));

#ifdef FIR_SDMA_SUPPORT
        // Enable FIRI transmitter 
        OUTREG32(&g_pVFiriReg->FIRI_TCR, 
            CSP_BITFVAL(FIRI_TCR_TDT,   FIRI_TCR_TDT_16)        | //Set DMA request level as 16 byte
            CSP_BITFVAL(FIRI_TCR_TCIE,  FIRI_TCR_TCIE_ENABLE)   | // Transmit Complete Interrupt is enabled
            CSP_BITFVAL(FIRI_TCR_TPEIE, FIRI_TCR_TPEIE_ENABLE)  | // Transmitter Packet End Interrupt is enabled
            CSP_BITFVAL(FIRI_TCR_TM,    FIRI_TCR_TM_FIR)        | // FIR mode
            CSP_BITFVAL(FIRI_TCR_TE,    FIRI_TCR_TE_ENABLE));
        
        CspFirConfig (g_pVFiriReg);

        DDKSdmaSetBufDesc(FirDmaChanTx, 0,
            DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP,
            FirPhysBufferAddr.LowPart,
            0, DDK_DMA_ACCESS_8BIT,
            (UINT16)thisDev->writeBufLen);
    
        // Start Channel
        DDKSdmaStartChan(FirDmaChanTx);
#else
        // Enable FIRI transmitter 
        OUTREG32(&g_pVFiriReg->FIRI_TCR,
            CSP_BITFVAL(FIRI_TCR_TCIE,  FIRI_TCR_TCIE_ENABLE)   |
            CSP_BITFVAL(FIRI_TCR_TPEIE, FIRI_TCR_TPEIE_ENABLE)  |
            CSP_BITFVAL(FIRI_TCR_TFUIE, FIRI_TCR_TFUIE_ENABLE)  |
            CSP_BITFVAL(FIRI_TCR_TM,    FIRI_TCR_TM_FIR)        | // FIR mode
            CSP_BITFVAL(FIRI_TCR_TE,    FIRI_TCR_TE_ENABLE));

        CspFirConfig (g_pVFiriReg);
        
        for(UINT i=0; i < thisDev->writeBufLen; i++)
            DEBUGMSG(ZONE_SEND, (TEXT("Fir: FirSendPacketQ sent : 0x%x \r\n"), *(thisDev->writeBuf + i)));

        for(UINT i=0; i < thisDev->writeBufLen; i++)
            *(UINT8 *)(&(g_pVFiriReg->FIRI_TXFIFO)) = *(thisDev->writeBuf + i);
#endif

        DEBUGMSG(ZONE_SEND, (TEXT("Fir: FirSendPacketQ sent 0x%x bytes of data.\r\n"), thisDev->writeBufLen));
    }

    DEBUGMSG(ZONE_SEND, (TEXT("Fir: -FirSendPacketQ\r\n")));
    return Result;
}


//-----------------------------------------------------------------------------
//
// Function: FirSend
//
//  This function inserts packet to send queue and setup device to send.
//
// Parameters:
//      thisDev
//          [in] .
//      Packet
//          [in] .
//
// Returns:
//    This function returns packet send status.
//
//-----------------------------------------------------------------------------
NDIS_STATUS FirSend( pFirDevice_t thisDev, PNDIS_PACKET Packet )
{
    NDIS_STATUS stat;

    DEBUGMSG(ZONE_SEND, (TEXT("Fir: +FirSend\r\n")));
    
    NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_PENDING);

    InsertTailList(&thisDev->SendQueue, (PLIST_ENTRY)Packet->MiniportReserved);

    if (thisDev->writePending)
    {
        stat = NDIS_STATUS_PENDING;
    }
    else
    {
        DEBUGMSG(ZONE_SEND, (TEXT("Fir: FirSend => TX idle, send\r\n")));
        stat = FirSendPacketQ(thisDev, TRUE);
    }

    DEBUGMSG(ZONE_SEND, (TEXT("Fir: -FirSend\r\n")));
    return stat;
}


//-----------------------------------------------------------------------------
//
// Function: FirReceive
//
//  This function setup device to receive data.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID FirReceive( pFirDevice_t thisDev )
{
    DEBUGMSG(ZONE_RECV, (TEXT("Fir: +FirReceive\r\n")));
    
    thisDev->nowReceiving = TRUE;

    // Enable FIRI receiver
#ifdef FIR_SDMA_SUPPORT
    INSREG32BF(&g_pVFiriReg->FIRI_RCR, FIRI_RCR_RE, FIRI_RCR_RE_ENABLE);
#else
    g_pVFiriReg->FIRI_RCR |=
        CSP_BITFVAL(FIRI_RCR_RPEIE, FIRI_RCR_RPEIE_ENABLE)  |
        CSP_BITFVAL(FIRI_RCR_PAIE,  FIRI_RCR_PAIE_ENABLE)   |
        CSP_BITFVAL(FIRI_RCR_RFOIE, FIRI_RCR_RFOIE_ENABLE)  |
        CSP_BITFVAL(FIRI_RCR_RE,    FIRI_RCR_RE_ENABLE);
#endif
    DEBUGMSG(ZONE_RECV, (TEXT("Fir: -FirReceive\r\n")));
}


//-----------------------------------------------------------------------------
//
// Function: FirSetSpeed
//
//  This function sets the Fir baudrate.
//
// Parameters:
//      speed
//          [in] .
//
// Returns:
//    This function returns the actually baudrate set.
//
//-----------------------------------------------------------------------------
baudRates FirSetSpeed( pFirDevice_t thisDev )
{
    baudRates rate = BAUDRATE_INVALID;
    UINT speed = thisDev->newSpeed;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: +FirSetSpeed %d\r\n"), speed ));

    if (speed > MAX_SIR_SPEED)
    {
        switch (speed)
        {
            case 4000000:
                INSREG32BF(&g_pVFiriReg->FIRI_CR,  FIRI_CR_OSF,  OSF_VALUE(3));
                INSREG32BF(&g_pVFiriReg->FIRI_TCR, FIRI_TCR_TM,  FIRI_TCR_TM_FIR);
                INSREG32BF(&g_pVFiriReg->FIRI_TCR, FIRI_TCR_TPP, FIRI_TCR_TPP_NO_INVERT);
                INSREG32BF(&g_pVFiriReg->FIRI_RCR, FIRI_RCR_RM,  FIRI_RCR_RM_FIR);
                INSREG32BF(&g_pVFiriReg->FIRI_RCR, FIRI_RCR_RPP, FIRI_RCR_RPP_INVERT);
                rate = BAUDRATE_4000000;
                break;

            case 1152000:
#if 1
                ERRORMSG(TRUE, (TEXT("Mir clock enable failed! Speed not supported\r\n")));
#else
                INSREG32BF(&g_pVFiriReg->FIRICR,  FIRI_CR_OSF, OSF_VALUE(2));
                INSREG32BF(&g_pVFiriReg->FIRITCR, FIRI_TCR_TM, FIRI_TCR_TM_1152_MIR);
                INSREG32BF(&g_pVFiriReg->FIRIRCR, FIRI_RCR_RM, FIRI_TCR_TM_1152_MIR);

                rate = BAUDRATE_1152000;
#endif
                break;

            case 576000:
#if 1
                ERRORMSG(TRUE, (TEXT("Mir clock enable failed! Speed not supported\r\n")));
#else
                INSREG32BF(&g_pVFiriReg->FIRI_CR,  FIRI_CR_OSF, OSF_VALUE(5));
                INSREG32BF(&g_pVFiriReg->FIRI_TCR, FIRI_TCR_TM, FIRI_TCR_TM_576_MIR);
                INSREG32BF(&g_pVFiriReg->FIRI_RCR, FIRI_RCR_RM, FIRI_RCR_RM_576_MIR);

                rate = BAUDRATE_576000;
#endif
                break;

            default:
                DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: FirSetSpeed, unsupported speed: %d\r\n"), speed));
        }
    }
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: FirSetSpeed, unsupported speed: %d, NOT FIR/MIR\r\n"), speed));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: -FirSetSpeed\r\n")));
    return rate;
}


//-----------------------------------------------------------------------------
//
// Function: FirInterruptHandler
//
//  This function is the interrupt handler for Fir device.
//
// Parameters:
//      thisDev
//          [in] .
//
// Returns:
//    None.
//
//-----------------------------------------------------------------------------
VOID FirInterruptHandler( pFirDevice_t thisDev )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: +FirInterruptHandler, 0x%08x\r\n"), thisDev));

#ifdef FIR_SDMA_SUPPORT
    UINT32 iStatus = 0;
#endif
    // if 1st packet not yet finished. continue to send
    if (thisDev->writePending)
    {
#ifdef FIR_SDMA_SUPPORT
        DDKSdmaGetBufDescStatus(FirDmaChanTx, 0, &iStatus);

        // check for DMA completion
        if (!(iStatus & DDK_DMA_FLAGS_BUSY))
        {
            // stall here till packet completion.
            while(! EXTREG32BF(&g_pVFiriReg->FIRI_TSR, FIRI_TSR_TC));

            FirSendPacketComplete(thisDev);
            thisDev->writePending = FALSE;

            if (thisDev->setSpeedAfterCurrentSendPacket)
            {
                thisDev->setSpeedAfterCurrentSendPacket = FALSE;
                SetSpeed(thisDev);

                // check return condition
                if (thisDev->linkSpeedInfo->bitsPerSec <= MAX_SIR_SPEED)
                return;
            }
            // Any more Tx packets?
            if (!IsListEmpty(&thisDev->SendQueue))
            {
                // Kick off another Tx
                FirSendPacketQ(thisDev, TRUE);
            }
            else
            {
                // Enable the receiver to receive other packets.
                g_pVFiriReg->FIRI_RCR |=
                    CSP_BITFVAL(FIRI_RCR_RPEIE, FIRI_RCR_RPEIE_ENABLE) |
                    CSP_BITFVAL(FIRI_RCR_RE,    FIRI_RCR_RE_ENABLE);
            }
        }
#else
        FirSendPacketComplete(thisDev);
        thisDev->writePending = FALSE;
        if (thisDev->setSpeedAfterCurrentSendPacket)
        {
            thisDev->setSpeedAfterCurrentSendPacket = FALSE;
            SetSpeed(thisDev);
            // check return condition
            if(thisDev->linkSpeedInfo->bitsPerSec <= MAX_SIR_SPEED)
                return;
        }

        // Any more Tx packets?
        if (!IsListEmpty(&thisDev->SendQueue))
        {
            // Kick off another Tx
            FirSendPacketQ(thisDev, TRUE);
        }
        else
        {
            // No more Tx packet, enable Rx hardware again.
            g_pVFiriReg->FIRI_RCR |=
                CSP_BITFVAL(FIRI_RCR_RPEIE,  FIRI_RCR_RPEIE_ENABLE) |
                CSP_BITFVAL(FIRI_RCR_PAIE,   FIRI_RCR_PAIE_ENABLE)  |
                CSP_BITFVAL(FIRI_RCR_RFOIE,  FIRI_RCR_RFOIE_ENABLE) |
                CSP_BITFVAL(FIRI_RCR_RE,     FIRI_RCR_RE_ENABLE);
        }
#endif
    }
    // Receive only
    else
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Fir: INTERRUPT: rcv packet end!\r\n")));
        
#ifdef FIR_SDMA_SUPPORT
        
        // get the status of the current buffer descriptor
        DDKSdmaGetBufDescStatus(FirDmaChanRx, 0, &iStatus);

        if (!(iStatus & DDK_DMA_FLAGS_BUSY))
        {
            // Update the count here.
            thisDev->rcvDataLength = iStatus & CSP_BITFMASK(SDMA_MODE_COUNT);
            FirReceivePacket(thisDev);
            DeliverFullBuffers(thisDev);

            // Setup recv DMA channel and enable it
            DDKSdmaSetBufDesc(FirDmaChanRx, 0, 
                DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP, 
                (FirPhysBufferAddr.LowPart + MAX_IRDA_DATA_SIZE),
                0, DDK_DMA_ACCESS_8BIT, MAX_IRDA_DATA_SIZE);

            DDKSdmaStartChan(FirDmaChanRx);
        }
#else
        if(FirReceivePacket(thisDev))
        {
            FirReceive(thisDev);
            DeliverFullBuffers(thisDev);
        }
#endif
    }
}
