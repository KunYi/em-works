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
//  File:  sir.cpp
//
//   This file implements the device specific functions for iMX51 fir device.
//
//------------------------------------------------------------------------------
#include "IrFir.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables
extern UINT32           pFirVirtBufferAddr;


//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables
extern BOOL bFirOn;

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: DumpUart2Reg
//
// Dump Uart2 registers
//
// Parameters:
//     None.
//
// Returns:  
//     None.
//
//-----------------------------------------------------------------------------
VOID DumpUart2Reg(VOID)
{
    DEBUGMSG(ZONE_ERROR, (TEXT("DumpUart2Reg: UCR1 is 0x%08x\r\n"), INREG32(&g_pVSIRReg->UCR1)));
    DEBUGMSG(ZONE_ERROR, (TEXT("DumpUart2Reg: UCR2 is 0x%08x\r\n"), INREG32(&g_pVSIRReg->UCR2)));
    DEBUGMSG(ZONE_ERROR, (TEXT("DumpUart2Reg: UCR3 is 0x%08x\r\n"), INREG32(&g_pVSIRReg->UCR3)));
    DEBUGMSG(ZONE_ERROR, (TEXT("DumpUart2Reg: UCR4 is 0x%08x\r\n"), INREG32(&g_pVSIRReg->UCR4)));
    DEBUGMSG(ZONE_ERROR, (TEXT("DumpUart2Reg: UFCR is 0x%08x\r\n"), INREG32(&g_pVSIRReg->UFCR))); 
    DEBUGMSG(ZONE_ERROR, (TEXT("DumpUart2Reg: USR1 is 0x%08x\r\n"), INREG32(&g_pVSIRReg->USR1)));
    DEBUGMSG(ZONE_ERROR, (TEXT("DumpUart2Reg: USR2 is 0x%08x\r\n"), INREG32(&g_pVSIRReg->USR2))); 
    DEBUGMSG(ZONE_ERROR, (TEXT("DumpUart2Reg: UBIR is 0x%08x\r\n"), INREG32(&g_pVSIRReg->UBIR))); 
    DEBUGMSG(ZONE_ERROR, (TEXT("DumpUart2Reg: UBMR is 0x%08x\r\n"), INREG32(&g_pVSIRReg->UBMR))); 
}
//-----------------------------------------------------------------------------
//
// Function: SirHwWrite
//
// Step the send fsm to send a few more bytes of an IR frame.
//
// Parameters:
//          thisDev 
//          [in] pFirDevice_t.
//
// Returns:  
//          This function returns packet send status.
//
//-----------------------------------------------------------------------------
static BOOLEAN SirHwWrite(pFirDevice_t thisDev)
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    UINT i, bytesAtATime, startPos = thisDev->portInfo.sendBufPos;
#else
    UINT i, bytesAtATime;
#endif
    BOOLEAN result;
    UINT maxLoops;

    //  Ordinarily, we want to fill the send FIFO once per interrupt.
    //  However, at high speeds the interrupt latency is too slow and
    //  we need to poll inside the ISR to send the whole packet during
    //  the first interrupt.

    if(thisDev->newSpeed > 115200)
        maxLoops = REG_TIMEOUT_LOOPS;
    else 
        maxLoops = REG_POLL_LOOPS;

    // Temporarily disable receiver first to avoid receiving data from ourselves
    INSREG32BF(&g_pVSIRReg->UCR2, UART_UCR2_RXEN, UART_UCR2_RXEN_DISABLE);

    // Write databytes as long as we have them and the UART's FIFO hasn't filled up.
    while (thisDev->portInfo.sendBufPos < thisDev->writeBufLen) 
    {
        // send up to the FIFO size.
        bytesAtATime = MIN(SER_FIFO_DEPTH, (thisDev->writeBufLen - thisDev->portInfo.sendBufPos));

        // Wait for ready-to-send.
        while (!(INREG32(&g_pVSIRReg->USR2) & CSP_BITFMASK(UART_USR2_TXFE))) 
        Sleep(0);
        // Send the next byte or FIFO-volume of bytes.
        while (bytesAtATime)
        {
            DEBUGMSG(ZONE_SEND, (TEXT("Sir: Send 0x%x\r\n"), thisDev->writeBuf[thisDev->portInfo.sendBufPos]));
            OUTREG32(&g_pVSIRReg->UTXD, thisDev->writeBuf[thisDev->portInfo.sendBufPos++]);
            bytesAtATime--;
        }
    }

    // Wail until transmission complete
    i = 0;
    while ((!(INREG32(&g_pVSIRReg->USR2) & CSP_BITFMASK(UART_USR2_TXDC))) && (++i < maxLoops)) 
        Sleep(0);

    // Re-enable receiver once again.
    INSREG32BF(&g_pVSIRReg->UCR2, UART_UCR2_RXEN, UART_UCR2_RXEN_ENABLE);

    if (i >= maxLoops) 
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Sir: Wait for tansmitting complete timeout!!"), i));
        return FALSE;
    }

    if (thisDev->portInfo.sendBufPos >= thisDev->writeBufLen)
        result = TRUE;
    else
        result = FALSE;

    DEBUGMSG(ZONE_SEND, (TEXT("Sir: SirHwWrite wrote %d bytes (%s): LOOP TIMES %d"), (UINT)(thisDev->portInfo.sendBufPos-startPos), (PUCHAR)(result ? "Successful" : "Failed"), i));

    return result;
}

//-----------------------------------------------------------------------------
//
// Function: SirHwRead
//
//  Read up to maxBytes bytes from the UART's receive FIFO.
//
// Parameters:
//      thisDev 
//          [in] .
//      maxBytes
//          [in] .
//
// Returns:  
//    This function returns number of bytes read.
//
//-----------------------------------------------------------------------------
static UINT SirHwRead(UCHAR *data, UINT maxBytes)
{
    UINT bytesRead = 0;
    UINT32 tempData;

    while (bytesRead < maxBytes) 
    {
        //  Wait for data-ready
        if(!(INREG32(&g_pVSIRReg->USR2) & CSP_BITFVAL(UART_USR2_RDR, UART_USR2_RDR_SET)))
            break;

        tempData = INREG32(&g_pVSIRReg->URXD);

        //  The UART reports framing and break errors as the effected
        //  characters appear on the stack.  We drop these characters,
        //  which will probably result in a bad frame checksum.
        if (tempData & SIR_RCV_ERROR) 
        {
            return (UINT)-1;
        }
        else 
        {
            data[bytesRead] = (UCHAR)(tempData & UART_URXD_RX_DATA_MSK);
        }

        DEBUGMSG(ZONE_RECV, (TEXT("Sir: Receive 0x%x\r\n"), data[bytesRead]));
        bytesRead++;
    }

    return bytesRead;
}

//-----------------------------------------------------------------------------
//
// Function: NdisToSirPacket
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
BOOLEAN NdisToSirPacket( PNDIS_PACKET Packet, UCHAR *irPacketBuf,
    UINT irPacketBufLen, UINT *irPacketLen )
{
    static UCHAR contigPacketBuf[MAX_IRDA_DATA_SIZE];
    PNDIS_BUFFER ndisBuf;
    UINT i, ndisPacketBytes = 0, I_fieldBytes, totalBytes = 0;
    UINT ndisPacketLen, numExtraBOFs;
    SLOW_IR_FCS_TYPE fcs, tmpfcs;
    UCHAR fcsBuf[SLOW_IR_FCS_SIZE*2];
    UINT fcsLen=0;
    PNDIS_IRDA_PACKET_INFO packetInfo = GetPacketInfo(Packet);
    UCHAR nextChar;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: +NdisToSirPacket\r\n")));

    //  Get the packet's entire length and its first NDIS buffer
    NdisQueryPacket(Packet, NULL, NULL, &ndisBuf, &ndisPacketLen);

    //  Make sure that the packet is big enough to be legal.
    //  It consists of an A, C, and variable-length I field.
    if (ndisPacketLen < (IR_ADDR_SIZE + IR_CONTROL_SIZE) ) 
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: packet too short in NdisToSirPacket (%d bytes)\r\n"), ndisPacketLen));
        return FALSE;
    }
    else 
    {
        I_fieldBytes = ndisPacketLen - IR_ADDR_SIZE - IR_CONTROL_SIZE;
    }

    //  Make sure that we won't overwrite our contiguous buffer.
    //  Make sure that the passed-in buffer can accomodate this packet's
    //  data no matter how much it grows through adding ESC-sequences, etc.
    if ((ndisPacketLen > MAX_IRDA_DATA_SIZE) ||
    (MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(I_fieldBytes) > irPacketBufLen)) 
    {
        //  The packet is too large
        //  Tell the caller to retry with a packet size large
        //  enough to get past this stage next time.
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: Packet too large in NdisToSirPacket (%d=%xh bytes), MAX_IRDA_DATA_SIZE=%d, irPacketBufLen=%d.\r\n"),
            ndisPacketLen, ndisPacketLen, MAX_IRDA_DATA_SIZE, irPacketBufLen));
        *irPacketLen = ndisPacketLen;
        return FALSE;
    }

    //  First, read the NDIS packet into a contiguous buffer.
    //  We have to do this in two steps so that we can compute the
    //  FCS BEFORE applying escape-byte transparency.
    while (ndisBuf) 
    {
        UCHAR *bufData;
        UINT bufLen;

        NdisQueryBuffer(ndisBuf, (PVOID *)&bufData, &bufLen);

        if ((ndisPacketBytes + bufLen) > ndisPacketLen) 
        {
            //  Packet was corrupt -- it misreported its size.
            *irPacketLen = 0;
            return FALSE;
        }

        NdisMoveMemory((PVOID)(contigPacketBuf+ndisPacketBytes), (PVOID)bufData, bufLen);
        ndisPacketBytes += bufLen;

        NdisGetNextBuffer(ndisBuf, &ndisBuf);
    }

    //  Do a sanity check on the length of the packet.
    if (ndisPacketBytes != ndisPacketLen) 
    {
        //  Packet was corrupt -- it misreported its size.
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: acket corrupt in NdisToSirPacket (buffer lengths don't add up to packet length).\r\n")));
        *irPacketLen = 0;
        return FALSE;
    }

    //  Compute the FCS on the packet BEFORE applying transparency fixups.
    //  The FCS also must be sent using ESC-char transparency, so figure
    //  out how large the fcs will really be.
    fcs = ComputeFCS(contigPacketBuf, ndisPacketBytes);

    for (i = 0, tmpfcs = fcs, fcsLen = 0; i < SLOW_IR_FCS_SIZE; tmpfcs >>= 8, i++) 
    {
        UCHAR fcsbyte = (UCHAR)(tmpfcs & 0x00ff);
        switch (fcsbyte) 
        {
            case SLOW_IR_BOF:
            case SLOW_IR_EOF:
            case SLOW_IR_ESC:
                fcsBuf[fcsLen++] = SLOW_IR_ESC; 
                fcsBuf[fcsLen++] = fcsbyte ^ SLOW_IR_ESC_COMP;
                break;

            default:
                fcsBuf[fcsLen++] = fcsbyte; 
                break;
        }
    }

    //  Now begin building the IR frame.
    //
    //  This is the final format:
    //
    //    BOF (1)
    //    extra BOFs ...
    //    NdisMediumIrda packet (what we get from NDIS):
    //        Address (1)
    //        Control (1)
    //    FCS (2)
    //    EOF (1)

    //  Prepend BOFs (extra BOFs + 1 actual BOF)
    numExtraBOFs = packetInfo->ExtraBOFs;

    if (numExtraBOFs > MAX_NUM_EXTRA_BOFS) 
        numExtraBOFs = MAX_NUM_EXTRA_BOFS;

    for (i = totalBytes = 0; i < numExtraBOFs; i++) 
    {
        *(SLOW_IR_BOF_TYPE *)(irPacketBuf+totalBytes) = SLOW_IR_EXTRA_BOF;
        totalBytes += SLOW_IR_EXTRA_BOF_SIZE;
    }
    *(SLOW_IR_BOF_TYPE *)(irPacketBuf+totalBytes) = SLOW_IR_BOF;
    totalBytes += SLOW_IR_BOF_SIZE;

    //  Copy the NDIS packet from our contiguous buffer, 
    //  applying escape-char transparency.
    for (i = 0; i < ndisPacketBytes; i++) 
    {
        nextChar = contigPacketBuf[i];
        switch (nextChar)
        {
            case SLOW_IR_BOF: 
            case SLOW_IR_EOF: 
            case SLOW_IR_ESC: 
                irPacketBuf[totalBytes++] = SLOW_IR_ESC;
                irPacketBuf[totalBytes++] = nextChar ^ SLOW_IR_ESC_COMP;
                break;

            default:
                irPacketBuf[totalBytes++] = nextChar;
                break;
        }
    }

    //  Add FCS, EOF.
    NdisMoveMemory((PVOID)(irPacketBuf+totalBytes), (PVOID)fcsBuf, fcsLen);
    totalBytes += fcsLen;
    *(SLOW_IR_EOF_TYPE *)(irPacketBuf+totalBytes) = (UCHAR)SLOW_IR_EOF;
    totalBytes += SLOW_IR_EOF_SIZE;

    *irPacketLen = totalBytes;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: ... NdisToSirPacket converted %d-byte ndis pkt to %d-byte irda pkt:\r\n"),
    ndisPacketLen, *irPacketLen));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SirDoSend
//
//  This function setup send packet.
//
// Parameters:
//      thisDev 
//          [in] .
//      packetToSend
//          [in] .
//
// Returns:  
//    This function returns packet send status.
//
//-----------------------------------------------------------------------------
BOOLEAN SirDoSend(pFirDevice_t thisDev, PNDIS_PACKET packetToSend)
{
    BOOLEAN stat;
    DEBUGMSG(ZONE_SEND, (TEXT("Sir: +SirDoSend\r\n")));

    stat = NdisToSirPacket(packetToSend,
        (UCHAR *)thisDev->writeBuf, MAX_IRDA_DATA_SIZE, &thisDev->writeBufLen);


    if (stat) 
    {
        // Disable interrupts while setting up the send FSM.
        SirDisableInterrupt(thisDev);

        // Finish initializing the send FSM.
        thisDev->portInfo.sendBufPos = 0;
        thisDev->writePending = TRUE;
        thisDev->nowReceiving = FALSE;   

        // Enable Tx interupt
        INSREG32BF(&g_pVSIRReg->UCR1, UART_UCR1_TXMPTYEN, UART_UCR1_TXMPTYEN_ENABLE);
    } 
    else  
    {
        DEBUGMSG(ZONE_SEND, (TEXT("Sir: Couldn't convert packet in SirDoSend()\r\n")));
    }

    DEBUGMSG(ZONE_SEND, (TEXT("Sir: -SirDoSend\r\n")));

    return stat;
}
//-----------------------------------------------------------------------------
//
// Function: SirSetSpeed
//
// This function sets the Sir baudrate.
//
// Parameters:
//         thisDev 
//         [in] pFirDevice_t.
//
// Returns:  
//         This function returns the actually baudrate set.
//
//-----------------------------------------------------------------------------
baudRates SirSetSpeed(pFirDevice_t thisDev)
{
    baudRates rate = BAUDRATE_INVALID;
    ULONG bRefFreq = 0;
    ULONG speed = thisDev->newSpeed;
    UCHAR cRFDivVal = 0;

    DEBUGMSG(ZONE_SEND, (TEXT("Sir: SirSetSpeed 0x%x\r\n"), speed));

    if (speed <= MAX_SIR_SPEED) 
    {
        for (int i=0; i<NUM_BAUDRATES; i++)
        {
            if (speed == supportedBaudRateTable[i].bitsPerSec)
            {
                rate = supportedBaudRateTable[i].tableIndex;
                break;
            }
        }

        INSREG32BF(&g_pVSIRReg->UCR4, UART_UCR4_IRSC, UART_UCR4_IRSC_SAMPCLK);
        INSREG32BF(&g_pVSIRReg->UCR2, UART_UCR2_SRST, UART_UCR2_SRST_RESET);
        for (UINT i=0; i<4; i++); //Delay after restting the UART
        
        // Check whether IR special case is needed.
        if (speed < IRSC_BAUDRATE) 
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: SIR special case!")));
            INSREG32BF(&g_pVSIRReg->UCR4, UART_UCR4_IRSC, UART_UCR4_IRSC_REFCLK);
        }
        else 
        {
            INSREG32BF(&g_pVSIRReg->UCR4, UART_UCR4_IRSC, UART_UCR4_IRSC_SAMPCLK);
        }

        cRFDivVal = BSPUartCalRFDIV(&bRefFreq);
        INSREG32BF(&g_pVSIRReg->UFCR, UART_UFCR_RFDIV, RFDIV_VALUE(cRFDivVal));
        OUTREG32(&g_pVSIRReg->UBIR, UART_UBIR_INCREMENT(speed, UART_UBMR_MOD_DEFAULT, bRefFreq)-1);
        OUTREG32(&g_pVSIRReg->UBMR, UART_UBMR_MOD_DEFAULT -1);

        NdisMSleep(5000); // delay
    }
    else
        DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: SirSetSpeed, unsupported bitsPerSec: %d, NOT SIR\r\n"), speed));
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: -SirSetSpeed\r\n")));
    return rate;

}

//-----------------------------------------------------------------------------
//
// Function: SirEnableInterrupt
//
// This function is to enable the interrupts from UART2 module.
//
// Parameters:
//      thisDev 
//          [in] .
//
// Returns:  
//    None.
//
//-----------------------------------------------------------------------------
VOID SirEnableInterrupt(pFirDevice_t thisDev)
{
    if (thisDev->writePending) 
    {
        INSREG32BF(&g_pVSIRReg->UCR1, UART_UCR1_TXMPTYEN, UART_UCR1_TXMPTYEN_ENABLE); // Transmitter FIFO empty interrupt
    } 
    else 
    {
        INSREG32BF(&g_pVSIRReg->UCR4, UART_UCR4_DREN, UART_UCR4_DREN_ENABLE);         // Receive data Ready Interrupt
    }
}

//-----------------------------------------------------------------------------
//
// Function: SirDisableInterrupt
//
//  This function is to disable the interrupts from UART2 module.
//
// Parameters:
//      thisDev 
//          [in] .
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SirDisableInterrupt(pFirDevice_t thisDev)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(thisDev);

    INSREG32BF(&g_pVSIRReg->UCR1, UART_UCR1_TXMPTYEN, UART_UCR1_TXMPTYEN_DISABLE);
    INSREG32BF(&g_pVSIRReg->UCR4, UART_UCR4_DREN, UART_UCR4_DREN_DISABLE);
}


//-----------------------------------------------------------------------------
//
// Function: SirInitialize
//
// This function initializes the Sir device.
//
// Parameters:
//        thisDev 
//        [in] pFirDevice_t.
//
// Returns:  
//        This function returns initialization status.
//
//-----------------------------------------------------------------------------
BOOLEAN SirInitialize(pFirDevice_t thisDev)
{
    PHYSICAL_ADDRESS Addr = {thisDev->SirPhyAddr, 0};
    DEVICE_LOCATION devLoc;

    DEBUGMSG(ZONE_INIT, (TEXT("Sir: +SirInitialize\r\n")));
    // read from registry
    // Map peripheral physical address to virtual address
    if(g_pVSIRReg == NULL)
    {
        g_pVSIRReg = (PCSP_UART_REG) MmMapIoSpace(Addr, sizeof(CSP_UART_REG), FALSE); 
        
        // Check if virtual mapping failed
        if (g_pVSIRReg == NULL)
        {
            DEBUGMSG(0, (TEXT("Sir:  MmMapIoSpace failed!\r\n")));
            return FALSE;
        }
    }

#if 0
    if (!BSPUartGetIrq((ULONG)thisDev->SirPhyAddr, &thisDev->sysIntrSir))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Resource: ERROR: Failed to obtain uart info.\r\n")));
        return FALSE;
    }
#endif

    // Use kernel IOCTL to translate the UART base address into an IRQ since
    // the IRQ value differs based on the SoC. Note that DEVICE_LOCATION
    // fields except IfcType and LogicalLoc are ignored for internal SoC 
    // components.
    devLoc.IfcType = Internal;
    devLoc.LogicalLoc = (ULONG)thisDev->SirPhyAddr;

    if (!KernelIoControl(IOCTL_HAL_REQUEST_IRQ, &devLoc, sizeof(devLoc),
        &thisDev->sysIntrSir, sizeof(thisDev->sysIntrSir), NULL))
    {
        ERRORMSG(1, (_T("Cannot obtain UART IRQ!\r\n")));
        DEBUGMSG(ZONE_ERROR, (TEXT("Sir - Initialization failed!!\r\n")));
        return FALSE;
    }

    DEBUGMSG(ZONE_INIT, (TEXT("Sir: -SirInitialize\r\n")));
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SirDeInitialize
//
// This function deinitializes the Sir device.
//
// Parameters:
//       thisDev 
//       [in] pFirDevice_t.
//
// Returns:  
//       None.
//
//-----------------------------------------------------------------------------
VOID SirDeInitialize(pFirDevice_t thisDev)
{
    DEBUGMSG(ZONE_DEINIT, (TEXT("Sir: +SirDeInitialize\r\n")));

    // Free UART register
    if(g_pVSIRReg)
    {
        MmUnmapIoSpace(g_pVSIRReg, sizeof(CSP_UART_REG));
        g_pVSIRReg = NULL;
    }
    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &thisDev->sysIntrSir, sizeof(DWORD), NULL, 0, NULL);
    thisDev->sysIntrSir = (DWORD)SYSINTR_UNDEFINED;

    DEBUGMSG(ZONE_DEINIT, (TEXT("Sir: -SirDeInitialize\r\n")));
}


//-----------------------------------------------------------------------------
//
// Function: SirClose
//
// This function closes the Sir device.
//
// Parameters:
//       thisDev 
//       [in] pFirDevice_t.
//
// Returns:  
//       None.
//
//-----------------------------------------------------------------------------
VOID SirClose(pFirDevice_t thisDev)
{
    DEBUGMSG(ZONE_CLOSE, (TEXT("Sir: +SirClose\r\n")));

    if (thisDev->portInfo.readBuf) 
    {
        InsertTailList(&thisDev->rcvBufBuf, RCV_BUF_TO_LIST_ENTRY(thisDev->portInfo.readBuf));
        thisDev->portInfo.readBuf  = NULL;
    }

    thisDev->writeBuf = NULL;

    INSREG32BF(&g_pVSIRReg->UCR2, UART_UCR2_TXEN,   UART_UCR2_TXEN_DISABLE);
    INSREG32BF(&g_pVSIRReg->UCR2, UART_UCR2_RXEN,   UART_UCR2_RXEN_DISABLE);

    // Reset UART2
    OUTREG32(&g_pVSIRReg->UCR2, CSP_BITFVAL(UART_UCR2_SRST, UART_UCR2_SRST_RESET));

    // Disable extern IrDa pin
    BSPIrdaEnable(FALSE);
    BSPUartEnableClock(thisDev->SirPhyAddr, FALSE);

    DEBUGMSG(ZONE_CLOSE, (TEXT("Sir: -SirClose\r\n")));
}

//-----------------------------------------------------------------------------
//
// Function: SirOpen
//
// This function opens the Sir device.
//
// Parameters:
//         thisDev 
//         [in] pFirDevice_t.
//
// Returns:  
//        This function returns status of open device.
//
//-----------------------------------------------------------------------------
BOOLEAN SirOpen(pFirDevice_t thisDev)
{
    PLIST_ENTRY pListEntry;
    BOOLEAN result = TRUE;
    UINT i;
    ULONG bRefFreq = 0;
    UCHAR cRFDivVal = 0;

    DEBUGMSG(ZONE_OPEN, (TEXT("Sir: +SirOpen")));

    BSPIrdaEnable(TRUE);
    BSPFirSetSIRIOMUX();

    if (!BSPUartEnableClock(thisDev->SirPhyAddr, TRUE))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Fir:  BSPUartEnableClock failed!\r\n")));
        result = FALSE;
        goto done;
    }

    thisDev->writeBuf = (PUCHAR)pFirVirtBufferAddr;

    pListEntry = MyRemoveHeadList(&thisDev->rcvBufBuf);
    if (pListEntry) 
    {
        thisDev->portInfo.readBuf = (PUCHAR) LIST_ENTRY_TO_RCV_BUF(pListEntry);
    } 
    else 
    {
        result = FALSE;
        goto done;
    }

    thisDev->portInfo.rcvState = STATE_INIT;
    thisDev->writePending = FALSE;


    // Configure UART2 registers

    OUTREG32(&g_pVSIRReg->UCR1, CSP_BITFVAL(UART_UCR1_UARTEN, UART_UCR1_UARTEN_DISABLE)); // Disable UART
    OUTREG32(&g_pVSIRReg->UCR2, CSP_BITFVAL(UART_UCR2_SRST, UART_UCR2_SRST_RESET));       // Reset UART

    // wait for a while
    for (i = 0; i <= 4; i++);

    cRFDivVal = BSPUartCalRFDIV(&bRefFreq);
    INSREG32BF(&g_pVSIRReg->UFCR, UART_UFCR_RFDIV, RFDIV_VALUE(cRFDivVal));

    INSREG32(&g_pVSIRReg->UFCR, CSP_BITFMASK(UART_UFCR_RXTL), CSP_BITFVAL(UART_UFCR_RXTL,SER_FIFO_RXTL));
    INSREG32(&g_pVSIRReg->UFCR, CSP_BITFMASK(UART_UFCR_TXTL), CSP_BITFVAL(UART_UFCR_TXTL,SER_FIFO_TXTL));
    OUTREG32(&g_pVSIRReg->UBIR, UART_UBIR_INCREMENT(DEFAULT_BAUD_RATE, UART_UBMR_MOD_DEFAULT, bRefFreq)-1);
    OUTREG32(&g_pVSIRReg->UBMR, UART_UBMR_MOD_DEFAULT -1);
    OUTREG32(&g_pVSIRReg->ONEMS, (bRefFreq / 1000));

    g_pVSIRReg-> UFCR |= (1<<6);   //set DTE mode

    SETREG32(&g_pVSIRReg->UCR2,
        CSP_BITFVAL(UART_UCR2_RXEN,   UART_UCR2_RXEN_ENABLE)     | // enable receiver
        CSP_BITFVAL(UART_UCR2_TXEN,   UART_UCR2_TXEN_ENABLE)     | // enable transmitter
        CSP_BITFVAL(UART_UCR2_WS,     UART_UCR2_WS_8BIT)         | // 8 Bits
        CSP_BITFVAL(UART_UCR2_IRTS,   UART_UCR2_IRTS_IGNORERTS));  // ignore RTS pin, for "none flow control"

    CspSerialSetMux (g_pVSIRReg);
    CspSerialSetTxIvt (g_pVSIRReg);

    OUTREG32(&g_pVSIRReg->UCR4,
        CSP_BITFVAL(UART_UCR4_INVR,   UART_UCR4_INVR_ACTIVELOW)  | // receiver not inverted
        CSP_BITFVAL(UART_UCR4_CTSTL,  SER_FIFO_DEPTH));

    OUTREG32(&g_pVSIRReg->UCR1,
        CSP_BITFVAL(UART_UCR1_UARTEN, UART_UCR1_UARTEN_ENABLE)   | // Enable UART
        CSP_BITFVAL(UART_UCR1_ICD,    UART_UCR1_ICD_8FRAMES)     |
        CSP_BITFVAL(UART_UCR1_IREN,   UART_UCR1_IREN_ENABLE));     // Enable IR

    BSPIrdaSetMode(SIR_MODE);

    // We should call SetSpeed() only at initialization.
    // If this function is called from other functions, 
    // we only need setup UART speed but not all speed info.
    if (!thisDev->linkSpeedInfo)  
        SetSpeed(thisDev);
    else
        SirSetSpeed(thisDev);

done:

    DEBUGMSG(ZONE_OPEN, (TEXT("Sir: -SirOpen %s"), (CHAR *)(result ? "succeeded" : "failed")));

    if (result) 
    {
        return TRUE;
    } 
    else 
    {
        SirClose(thisDev);
        return FALSE;
    }
}


//-----------------------------------------------------------------------------
//
// Function: SirSendPacketQ
//
//  This function sends the packet from the send queue.
//
// Parameters:
//        thisDev 
//        [in] pFirDevice_t.
//        firstBufIsPending
//        [out] BOOLEAN.
//
// Returns:  
//        NDIS_STATUS.
//
//-----------------------------------------------------------------------------
NDIS_STATUS SirSendPacketQ(pFirDevice_t thisDev, BOOLEAN firstBufIsPending)
{
    NDIS_STATUS Result = NDIS_STATUS_FAILURE;
    BOOLEAN sendSucceeded;
    PLIST_ENTRY ListEntry;

    DEBUGMSG(ZONE_SEND, (TEXT("Sir: SirSendPacketQ(dev=0x%x, %hs)\r\n"), 
        (UINT)thisDev, (CHAR *)(firstBufIsPending ? "pend" : "not pend")));

    // Get packet from the head of SendQueue
    ListEntry = MyRemoveHeadList(&thisDev->SendQueue);
    if (ListEntry) 
    {
        PNDIS_PACKET packetToSend = CONTAINING_RECORD(ListEntry,
            NDIS_PACKET, MiniportReserved);
        PNDIS_IRDA_PACKET_INFO packetInfo;

        packetInfo = GetPacketInfo(packetToSend);

        //
        // NOTE: Don't use NdisStallExecution for turnaround delay since
        //       you shouldn't stall for more than 60 us. Calling
        //       NdisStallExecution for large times will degrade system
        //       performance.
        //

        DEBUGMSG(ZONE_SEND, (TEXT("Sir: TurnaroundTimer (%d).\r\n"), packetInfo->MinTurnAroundTime));
        if (packetInfo->MinTurnAroundTime) 
        {
            UINT usecToWait = packetInfo->MinTurnAroundTime;
            UINT msecToWait;

            // Reset the time, so that the packet gets sent out after timeout
            packetInfo->MinTurnAroundTime = 0;

            // Add it back to the head of the SendQueue
            InsertHeadList(&thisDev->SendQueue, (PLIST_ENTRY)packetToSend->MiniportReserved);

            // Ndis timer has a 1ms granularity (in theory).  Let's round off.
            msecToWait = (usecToWait < 1000) ? 1 : (usecToWait + 500) / 1000;

            DEBUGMSG(ZONE_SEND, (TEXT("Sir: TurnaroundTimer (%d).\r\n"), msecToWait));
            NdisMSetTimer(&thisDev->TurnaroundTimer, msecToWait);
            return NDIS_STATUS_PENDING; // Say we're successful.  We'll come back here.
        }

        // See if this was the last packet before we need to change speed.
        if (packetToSend == thisDev->lastPacketAtOldSpeed) 
        {
            thisDev->lastPacketAtOldSpeed = NULL;
            thisDev->setSpeedAfterCurrentSendPacket = TRUE;
        }

        //  Send one packet to the COM port.
        DEBUGMSG(ZONE_SEND, (TEXT("Sir: Sending packet (0x%x).\r\n"), (UINT)packetToSend));

        sendSucceeded = SirDoSend(thisDev, packetToSend);

        //  If the buffer we just sent was pending 
        //  (i.e. we returned NDIS_STATUS_PENDING for it in SirSend),
        //  then hand the sent packet back to the protocol.
        //  Otherwise, we're just delivering it synchronously from SirSend.
        if (firstBufIsPending)
        {
            DEBUGMSG(ZONE_SEND, (TEXT("Sir: Send Packet Pending!\r\n")));

            NdisMSendComplete(thisDev->ndisAdapterHandle, packetToSend, 
            (NDIS_STATUS)(sendSucceeded ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE));

            // If the packet was pended, it means this packet was not just
            // received in MiniportSend.  If we're being called from there,
            // we want them to pend the packet they just did receive, so we
            // return PENDING.
            Result = NDIS_STATUS_PENDING;
        } 
        else 
        {
            DEBUGMSG(ZONE_SEND, (TEXT("Sir: Send result!!!\r\n")));
            Result = sendSucceeded ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE;
        }
    }

    DEBUGMSG(ZONE_SEND, (TEXT("Sir: -SirSendPacketQ\r\n")));
    return Result;
}


//-----------------------------------------------------------------------------
//
// Function: SirSend
//
// This function inserts packet to send queue and setup device to send.
//
// Parameters:
//        thisDev 
//        [in] pFirDevice_t.
//        Packet
//        [in] PNDIS_PACKET.
//
// Returns:  
//        This function returns packet send status.
//
//-----------------------------------------------------------------------------
NDIS_STATUS SirSend(pFirDevice_t thisDev, PNDIS_PACKET Packet)
{
    NDIS_STATUS result;

    DEBUGMSG(ZONE_SEND, (TEXT("Sir: +SirSend(thisDev=0x%x)\r\n"), (UINT)thisDev));

    // Use MiniportReserved as a LIST_ENTRY.  First check so no one
    // ever changes the size of these the wrong way.
    ASSERT(sizeof(Packet->MiniportReserved)>=sizeof(LIST_ENTRY));
    InsertTailList(&thisDev->SendQueue, (PLIST_ENTRY)Packet->MiniportReserved);

    if(thisDev->writePending)
    {
        DEBUGMSG(ZONE_SEND, (TEXT("Sir: SirSend already pending!!!\r\n")));
        result = NDIS_STATUS_PENDING;
    }
    else
    {
        BOOLEAN firstBufIsPending = (BOOLEAN)(Packet != HEAD_SEND_PACKET(thisDev));
        result = SirSendPacketQ(thisDev, firstBufIsPending);
    }
    DEBUGMSG(ZONE_SEND, (TEXT("Sir: -SirSend [%s]\r\n"), DBG_NDIS_RESULT_STR(result)));
    return result;
}


//-----------------------------------------------------------------------------
//
// Function: SirReceive
//
// Setup for SIR receive.
//
// Parameters:
//        thisDev
//        [in] .
//
// Returns:  
//        None.
//
//-----------------------------------------------------------------------------
VOID SirReceive(pFirDevice_t thisDev)
{
    DEBUGMSG(ZONE_RECV, (TEXT("Sir: +SirReceive, 0x%0x8.\r\n"), thisDev));

    // Disable Transmitter and interrupt first
    SirDisableInterrupt(thisDev);

    thisDev->AdapterState = ADAPTER_RX;

    // Enable Receiver interrupt first
    INSREG32BF(&g_pVSIRReg->UCR4, UART_UCR4_DREN, UART_UCR4_DREN_ENABLE);

    //if FIR don't open in control panel, then we must close uart clock to save power.
    if(bFirOn == FALSE)
        BSPUartEnableClock(thisDev->SirPhyAddr, FALSE);

    DEBUGMSG(ZONE_RECV, (TEXT("Sir: -SirReceive 0x%x\r\n"), INREG32(&g_pVSIRReg->USR2)));
}


//-----------------------------------------------------------------------------
//
// Function: SirReceivePacket
//
//  This function receives and analyze the raw IR data.
//
// Parameters:
//        thisDev 
//        [in] pFirDevice_t.
//
// Returns:  
//        This function returns true or false.
//
//-----------------------------------------------------------------------------
static BOOLEAN SirReceivePacket(pFirDevice_t thisDev)
{
    UINT rawBufPos = 0, rawBytesRead = 0;
    BOOLEAN result;
    UCHAR thisch;
    PLIST_ENTRY pListEntry;

    DEBUGMSG(ZONE_RECV, (TEXT("Sir: +SirReceivePacket\r\n")));

    //  Read in and process groups of incoming bytes from the FIFO.
    //  NOTE:  We have to loop once more after getting MAX_RCV_DATA_SIZE
    //         bytes so that we can see the 'EOF'; hence <= and not <.
    while ((thisDev->portInfo.rcvState != STATE_SAW_EOF) && (thisDev->portInfo.rcvBufPos <= MAX_RCV_DATA_SIZE)) 
    {
        if (thisDev->portInfo.rcvState == STATE_CLEANUP) 
        {
            //  We returned a complete packet last time, but we had read some
            //  extra bytes, which we stored into the rawBuf after returning
            //  the previous complete buffer to the user.  
            //  So instead of calling DoRcvDirect in this first execution of this loop, 
            //  we just use these previously-read bytes.
            //  (This is typically only 1 or 2 bytes).
            rawBytesRead = thisDev->portInfo.rcvBufPos;
            thisDev->portInfo.rcvState = STATE_INIT;
            thisDev->portInfo.rcvBufPos = 0;
        } 
        else  
        {
            rawBytesRead = SirHwRead(thisDev->portInfo.rawBuf, SER_FIFO_DEPTH);
            if (rawBytesRead == (UINT)-1) 
            {
                // Error. Reset the state
                thisDev->portInfo.rcvState = STATE_INIT;
                thisDev->portInfo.rcvBufPos = 0;
                continue;
            } 
            else if (rawBytesRead == 0) 
            {
                DEBUGMSG(ZONE_RECV, (TEXT("Sir: 0 byte read, break\r\n")));
                break;
            }
        }

        //  Let the receive state machine process this group of characters
        //  we got from the FIFO.
        //  NOTE:  We have to loop once more after getting MAX_RCV_DATA_SIZE
        //         bytes so that we can see the 'EOF'; hence <= and not <.        
        for (rawBufPos = 0; 
            ((thisDev->portInfo.rcvState != STATE_SAW_EOF) && 
            (rawBufPos < rawBytesRead) && 
            (thisDev->portInfo.rcvBufPos <= MAX_RCV_DATA_SIZE)); 
            rawBufPos++) 
        {
            thisch = thisDev->portInfo.rawBuf[rawBufPos];
            DEBUGMSG(ZONE_RECV, (TEXT("Sir: cur state=%d, data=0x%x\r\n"),thisDev->portInfo.rcvState,thisch));
            switch (thisDev->portInfo.rcvState) 
            {
                case STATE_INIT:
                    switch (thisch) 
                    {
                        case SLOW_IR_BOF:
                            thisDev->portInfo.rcvState = STATE_GOT_BOF;
                            break;
                        case SLOW_IR_EOF:
                        case SLOW_IR_ESC:
                        default:
                        //  This is meaningless noise.  Scan past it.
                        break;
                    }
                    break;

                case STATE_GOT_BOF:
                    switch (thisch) 
                    {
                        case SLOW_IR_BOF:
                            break;
                        case SLOW_IR_EOF:
                            DEBUGMSG(ZONE_ERROR, (TEXT("EOF in absorbing-BOFs state in DoRcv")));
                            thisDev->portInfo.rcvState = STATE_INIT;
                            break;
                        case SLOW_IR_ESC:
                            thisDev->portInfo.rcvBufPos = 0;
                            thisDev->portInfo.rcvState = STATE_ESC_SEQUENCE;
                            break;
                        default:
                            thisDev->portInfo.readBuf[0] = thisch;
                            thisDev->portInfo.rcvBufPos = 1;
                            thisDev->portInfo.rcvState = STATE_ACCEPTING;
                            break;
                    }
                    break;

                case STATE_ACCEPTING:
                    switch (thisch) 
                    {
                        case SLOW_IR_BOF:  
                            DEBUGMSG(ZONE_WARN, (TEXT("WARNING: BOF during accepting state in DoRcv")));
                            thisDev->portInfo.rcvState = STATE_INIT;
                            thisDev->portInfo.rcvBufPos = 0;
                            break;

                        case SLOW_IR_EOF:
                            if (thisDev->portInfo.rcvBufPos < 
                                IR_ADDR_SIZE + IR_CONTROL_SIZE + SLOW_IR_FCS_SIZE) 
                            {
                                thisDev->portInfo.rcvState = STATE_INIT;
                                thisDev->portInfo.rcvBufPos = 0;
                            } 
                            else  
                            {
                                thisDev->portInfo.rcvState = STATE_SAW_EOF;
                            }
                            break;

                        case SLOW_IR_ESC:
                            thisDev->portInfo.rcvState = STATE_ESC_SEQUENCE;
                            break;

                        default:
                        thisDev->portInfo.readBuf[thisDev->portInfo.rcvBufPos++] = thisch;
                        break;
                    }
                    break;

                case STATE_ESC_SEQUENCE:
                    switch (thisch) 
                    {
                        case SLOW_IR_EOF:
                        case SLOW_IR_BOF:
                        case SLOW_IR_ESC:
                            //  ESC + {EOF|BOF|ESC} is an abort sequence
                            DEBUGMSG(ZONE_ERROR, (TEXT("DoRcv - abort sequence; ")
                                TEXT("ABORTING IR PACKET: (got following ")
                                TEXT("packet + ESC,%xh)"), 
                                (UINT)thisch));
                            thisDev->portInfo.rcvState = STATE_INIT;
                            thisDev->portInfo.rcvBufPos = 0;
                            break;

                        case SLOW_IR_EOF^SLOW_IR_ESC_COMP:
                        case SLOW_IR_BOF^SLOW_IR_ESC_COMP:
                        case SLOW_IR_ESC^SLOW_IR_ESC_COMP:
                            thisDev->portInfo.readBuf[thisDev->portInfo.rcvBufPos++] = thisch ^ SLOW_IR_ESC_COMP;
                            thisDev->portInfo.rcvState = STATE_ACCEPTING;
                            break;

                        default:
                            DEBUGMSG(ZONE_ERROR, (TEXT("Unnecessary escape sequence: (got following packet + ESC,%xh"), (UINT)thisch));
                            thisDev->portInfo.readBuf[thisDev->portInfo.rcvBufPos++] = thisch ^ SLOW_IR_ESC_COMP;
                            thisDev->portInfo.rcvState = STATE_ACCEPTING;
                            break;
                    }
                    break;

                case STATE_SAW_EOF:
                default:
                    DEBUGMSG(ZONE_ERROR, (TEXT("Illegal state in DoRcv")));
                    thisDev->portInfo.rcvBufPos = 0;
                    thisDev->portInfo.rcvState = STATE_INIT;
                    return 0;
            }
        }
    }



    //  Set result and do any post-cleanup.
    switch (thisDev->portInfo.rcvState) 
    {
        case STATE_SAW_EOF:
            //  We've read in the entire packet.
            //  Queue it and return TRUE.
            DEBUGMSG(ZONE_RECV, (TEXT("Sir:  *** DoRcv returning with COMPLETE packet, read %d bytes ***"), 
                thisDev->portInfo.rcvBufPos));

            ASSERT(!IsListEmpty(&thisDev->rcvBufBuf));

            //QueueReceivePacket(thisDev, thisDev->portInfo.readBuf, thisDev->portInfo.rcvBufPos, FALSE);
            QueueReceivePacket(thisDev, thisDev->portInfo.readBuf, thisDev->portInfo.rcvBufPos, FALSE);

            // The protocol has our buffer.  Get a new one.
            pListEntry = RemoveHeadList(&thisDev->rcvBufBuf);
            thisDev->portInfo.readBuf = (PUCHAR) LIST_ENTRY_TO_RCV_BUF(pListEntry);

            result = TRUE;

            if (rawBufPos < rawBytesRead) 
            {
                //  We have some more unprocessed bytes in the raw buffer.
                //  Move these to the beginning of the raw buffer
                //  go to the CLEANUP state, which indicates that these
                //  bytes be used up during the next call.
                //  (This is typically only 1 or 2 bytes).
                //  Note:  We can't just leave these in the raw buffer because
                //         we might be supporting connections to multiple COM ports.
                memcpy(thisDev->portInfo.rawBuf, &thisDev->portInfo.rawBuf[rawBufPos], rawBytesRead-rawBufPos);
                thisDev->portInfo.rcvBufPos = rawBytesRead-rawBufPos;
                thisDev->portInfo.rcvState = STATE_CLEANUP;
            } 
            else  
            {
                thisDev->portInfo.rcvState = STATE_INIT;
            }
            break;

        default:
            if (thisDev->portInfo.rcvBufPos > MAX_RCV_DATA_SIZE) 
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("Sir: Overrun in DoRcv : read %d=%xh bytes:"), 
                    thisDev->portInfo.rcvBufPos, thisDev->portInfo.rcvBufPos));
                thisDev->portInfo.rcvBufPos = 0;
                thisDev->portInfo.rcvState = STATE_INIT;
            } 
            else 
            {
                DEBUGMSG(ZONE_RECV, (TEXT("Sir: DoRcv returning with partial packet, read %d bytes"), 
                    thisDev->portInfo.rcvBufPos));
            }
            result = FALSE;
            break;
    }
    DEBUGMSG(ZONE_RECV, (TEXT("Sir: -SirReceivePacket\r\n")));
    return result;
}


//-----------------------------------------------------------------------------
//
// Function: SirInterruptHandler
//
// This function is the interrupt handler for Sir device.
//
// Parameters:
//        thisDev 
//        [in] pFirDevice_t.
//
// Returns:  
//        None.
//
//-----------------------------------------------------------------------------
// Remove-Prefast: Warning 28167 workaround
PREFAST_SUPPRESS(28167,"This Warning can be skipped!")
VOID SirInterruptHandler(pFirDevice_t thisDev)
{
    UINT32 tempRegData = 0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: +SirInterruptHandler, 0x%08x\r\n"), thisDev));
    // Get interrupt status
    tempRegData = INREG32(&g_pVSIRReg->USR2);

    if (tempRegData & (SIR_INTR_MASK)) 
    {
        if (thisDev->writePending) 
        {
            // Tx
            DEBUGMSG(ZONE_THREAD, (TEXT("Sir: COM INTERRUPT: xmit reg empty, 0x%08x."), tempRegData));

            if (tempRegData & CSP_BITFMASK(UART_USR2_TXFE)) 
            {
                SirHwWrite(thisDev);

                thisDev->writePending = FALSE;

                //  If we just sent the last frame to be sent at the old speed,
                //  set the hardware to the new speed.
                if (thisDev->setSpeedAfterCurrentSendPacket) 
                {
                    thisDev->setSpeedAfterCurrentSendPacket = FALSE;
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: set new speed\r\n")));
                    SetSpeed(thisDev);
                }
            }
            // Any more Tx packets?
            if (!IsListEmpty(&thisDev->SendQueue))  
            {
                // Kick off another Tx
                SirSendPacketQ(thisDev, TRUE);
            }
            else 
            {
                //  There are no more bytes to send; 
                //  reset interrupts for receive mode.
                SirReceive(thisDev);
            }
        } 
        else if(tempRegData & CSP_BITFMASK(UART_USR2_RDR)) 
        {
            // Rx
            DEBUGMSG(ZONE_THREAD, (TEXT("COM INTERRUPT: rcv data available! 0x%08x\r\n"), tempRegData));

            thisDev->nowReceiving = TRUE;

            if (!thisDev->mediaBusy) 
            {
                // If we have just started receiving a packet, indicate media-busy
                // to the protocol.
                thisDev->mediaBusy = TRUE;
                DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: HandleInterrupt indicating media busy\r\n")));

                NdisReleaseSpinLock(&thisDev->Lock);
                NdisMIndicateStatus(thisDev->ndisAdapterHandle, NDIS_STATUS_MEDIA_BUSY, NULL, 0);
                NdisMIndicateStatusComplete(thisDev->ndisAdapterHandle);
                NdisAcquireSpinLock(&thisDev->Lock);
            }

            if (SirReceivePacket(thisDev)) 
            {
                //  The receive engine has accumulated an entire frame.
                //  Request a deferred callback so we can deliver the frame
                //  when not in interrupt context.
                DeliverFullBuffers(thisDev);
            }
        }
        else 
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("Sir: COM INTERRUPT: dummy interrupt, 0x%08x, ignored.\r\n"), tempRegData));
        }
    }
    else 
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Sir: SirInterruptHandler: unexpected interrupt comes, 0x%08x.\r\n"), tempRegData));
        return;
    }
}
