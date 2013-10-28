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
#include "vbridge.h"
#include <nkintr.h>
#include <kitl.h>

//------------------------------------------------------------------------------
//
//  Useful defines..
//

#define PRINTMSG(cond, printf_exp)  ((void)((cond)?(KITLOutputDebugString printf_exp), 1 : 0))

#define PLUSPLUS(a, max)    \
    (((a+1)>=max) ? 0x00 : (a+1))

#define MAX_8023_PDU    1514

//
//  Buffer manipulations..
//

#define     TX_BUFFERS_DWORD    0x4000 / sizeof(DWORD)  
#define     RX_BUFFERS_DWORD    0x4000 / sizeof(DWORD)
#define     HEADER_SIGNATURE    0x12345678

#define     FLAG_PRODUCED       0x00000001  //  This buffer is produced..
#define     FLAG_CONSUMED       0x00000002  //  This buffer is consumed..
#define     FLAG_SKIP_BUFFER    0x00000004  //  No data skip to next one.

typedef struct _PACKET_HEADER
{
    DWORD   dwSignature;                    //  Signature of header..
    DWORD   dwFlag;                         //  Miscellaneous flags..
    UCHAR   *pucNextBuffer;                 //  The next chunk..
    DWORD   dwCurrentSize;                  //  Zero ? go to next chunk..

}   PACKET_HEADER, *PPACKET_HEADER;


PUCHAR      pucTxProducer;                  //  Manipulated by User   Code.
PUCHAR      pucTxConsumer;                  //  Manipulated by Kernel Code.
PUCHAR      pucRxProducer;                  //  Manipulated by Kernel Code.
PUCHAR      pucRxConsumer;                  //  Manipulated by User   Code.
PUCHAR      pucTxMax;                       //  For convenience..
PUCHAR      pucTxMin;
PUCHAR      pucRxMax;                       //  For convenience..
PUCHAR      pucRxMin;

//  Add extra entry, otherwise pucRxMax is incorrectly calculated in VBridgeInit()
ULONG       TxBuffers[TX_BUFFERS_DWORD + 1];
ULONG       RxBuffers[RX_BUFFERS_DWORD + 1];


DWORD   dwCurrentPacketFilter   = 0x00;     //  Current VMini filter mode..
UCHAR   EDBGMacAddress[6];                  //  Address that VMini should use..
BOOL    bValidAddress           = FALSE;    //  Whether EDBGMacAddress is valid.
BOOL    bWarnNoMacDone          = FALSE;    //  One time warning to user.
BOOL    g_bResetBuffer          = FALSE;    //  Set in IOCTL_VBRIDGE_WILD_CARD_RESET_BUFFER.
BOOL    g_bUOpened              = FALSE;    //  Set when client request/send packet

////////////////////////////////////////////////////////////////////////////////
//  Misc utility functions.
//

void VBridgeDisplayHex (BYTE data)
{    
    if (data < 0x10)
        PRINTMSG(1, ("0"));
    PRINTMSG (1, ("%x ", data));

}    // DisplayHex()


void VBridgeDumpMemory (PBYTE pSource, DWORD dwLength)
{
    int        i = 0;

    PRINTMSG (1, ("+---- MEM DUMP (%d bytes)----+\r\n", dwLength));
    PRINTMSG (1, ("0x%x: ", pSource));
    while (dwLength--)
    {    
        VBridgeDisplayHex (*pSource++);
        if (++i == 16)
        {
            i = 0;        
            PRINTMSG (1, ("\r\n0x%x: ", pSource));
        }        
    }
    
    PRINTMSG (1, ("\r\n\r\n"));

}    // DumpMemory()



////////////////////////////////////////////////////////////////////////////////
//  ProduceBuffer()
//
//  Routine Description:
//
//      This function will give a chunck of buffer for the caller.
//
//  Arguments:  
//
//      bIsTx   ::  Whether it is TX buffer or RX buffer.
//                  For Tx case, it will be called by User code.
//                  For Rx case, it will be called by Kernel code.
//
//      dwSize  ::  Buffer size requested..
//
//  Return Value:
//
//      Null if no buffer available, otherwise pointer to buffer to be used.
//

PUCHAR
ProduceBuffer(BOOL  bIsTx, DWORD dwSize)
{
    PUCHAR          *ppucProducer;
    PUCHAR          *ppucBufferStart;
    PUCHAR          *ppucBufferEnd;
    PUCHAR          *ppucConsumer;  
    PACKET_HEADER   *pPacketHeader;
    DWORD           dwTotalSize;    
    DWORD           dwAddThis;

    //
    //  Our buffers are always DWORD align.. 
    //

    dwTotalSize = dwSize + sizeof(PACKET_HEADER);   
    dwAddThis   = dwTotalSize % 4;
    if (dwAddThis)
        dwTotalSize +=  (4 - dwAddThis);

    if (bIsTx)
    {
        ppucProducer    =   &pucTxProducer;
        ppucBufferStart =   &pucTxMin;
        ppucBufferEnd   =   &pucTxMax;
        ppucConsumer    =   &pucTxConsumer;
    }
    else
    {
        ppucProducer    =   &pucRxProducer;
        ppucBufferStart =   &pucRxMin;
        ppucBufferEnd   =   &pucRxMax;
        ppucConsumer    =   &pucRxConsumer;
    }


    //
    //  First stop is to check we won't overshoot consumer..
    //

    if ((*ppucConsumer > *ppucProducer) && 
        (*ppucProducer + dwTotalSize >= *ppucConsumer))
    {
        return NULL;
    }


    //
    //  Make sure we don't wrap..
    //  And after this requested buffer, we have at least 
    //  enough space for PACKET_HEADER at the end of this buffer for SKIP
    //  BUFFER..
    //

    if ((*ppucProducer + dwTotalSize) > (*ppucBufferEnd - sizeof(PACKET_HEADER)))
    {
        PRINTMSG (0,
            ("Producer = [0x%x] : TotalSize = [%d] : BufferEnd[0x%x]\r\n",
            *ppucProducer,
            dwTotalSize,
            *ppucBufferEnd));
        

        //
        //  Nop, and if the consumer is not at the beginning of the 
        //  buffer, then it's safe for us to advance the producer there..
        //

        if (*ppucConsumer == *ppucBufferStart)
        {
            //
            //  Consumer is hogging at the beginning..
            //

            return NULL;
        }

        //
        //  Okay, consumer is not there, so it's safe to mark this as 
        //  SKIP buffer..
        //

        pPacketHeader = (PPACKET_HEADER) *ppucProducer;     

        pPacketHeader->dwSignature   = HEADER_SIGNATURE;
        pPacketHeader->dwFlag        = FLAG_SKIP_BUFFER;
        pPacketHeader->pucNextBuffer = *ppucBufferStart;


        //
        //  It's safe to move the producer pointer now..
        //

        PRINTMSG (0, ("SKIP produced [0x%x].\r\n",
            *ppucProducer));

        *ppucProducer = *ppucBufferStart;


        //
        //  Now make sure we can fit it without overshooting consumer..
        //

        if ((*ppucConsumer > *ppucProducer) && 
            (*ppucProducer + dwTotalSize >= *ppucConsumer))
        {
            //
            //  Too Bad.. not enough space..
            //

            PRINTMSG (0,
                ("-- Producer = [0x%x] : TotalSize = [%d] : BufferEnd[0x%x]\r\n",
                *ppucProducer,
                dwTotalSize,
                *ppucBufferEnd));

            return NULL;
        }
        
    }


    //
    //  By now we are guarantee that the buffer will not wrap.. 
    //  And we definitely have space..
    //  

    pPacketHeader   = (PPACKET_HEADER) *ppucProducer;

    pPacketHeader->dwSignature   = HEADER_SIGNATURE;

    //
    //  Packet not ready until the caller calls ProduceBufferDone().
    //

    pPacketHeader->dwFlag        = 0x00;    
    pPacketHeader->pucNextBuffer = *ppucProducer + dwTotalSize;
    pPacketHeader->dwCurrentSize = dwSize;


    //
    //  Okay, it's safe to move the producer pointer now..
    //  The owner of this buffer is on its own now, it will need to
    //  call ProduceBufferDone() to let the consumer know that it can now
    //  be consumed.   Failure to do so will block the flow!!!
    //

    *ppucProducer   = *ppucProducer + dwTotalSize;

    return ((PUCHAR)pPacketHeader + sizeof(PACKET_HEADER));
    

}   //  ProduceBuffer() 



////////////////////////////////////////////////////////////////////////////////
//  ProduceBufferDone()
//
//  Routine Description:
//
//      This function will mark the buffer to be ready for consumption.
//
//  Arguments:  
//
//      bIsTx       ::  Whether it is TX buffer or RX buffer.
//      pucBuffer   ::  Points to buffer obtained in ProduceBuffer()
//
//  Return Value:
//
//      None.
//

void
ProduceBufferDone(BOOL  bIsTx, PUCHAR pucBuffer)
{
    PACKET_HEADER   *pPacketHeader;

    UNREFERENCED_PARAMETER(bIsTx);

    pPacketHeader = (PPACKET_HEADER)(pucBuffer - sizeof(PACKET_HEADER));

    if (pPacketHeader->dwSignature != HEADER_SIGNATURE)
    {
        PRINTMSG (1,
            ("VBridge: Error! ProduceBufferDone() Corrupted header signature [0x%x]!",
            pPacketHeader->dwSignature));

        return;            
    }            

    //
    //  Off it goes..
    //

    pPacketHeader->dwFlag = FLAG_PRODUCED;

}   //  ProduceBufferDone() 



////////////////////////////////////////////////////////////////////////////////
//  ConsumeBuffer()
//
//  Routine Description:
//
//      This function will give a chunck of buffer for the caller.
//
//  Arguments:  
//
//      bIsTx   ::  Whether it is TX buffer or RX buffer.
//                  For Tx case, it will be called by Kernel code.
//                  For Rx case, it will be called by User code.
//
//      pdwSize ::  [o] size of the buffer returned..
//
//  Return Value:
//
//      Null if no buffer available, otherwise pointer to buffer to be used.
//

PUCHAR
ConsumeBuffer(BOOL  bIsTx, DWORD *pdwSize)
{
    PUCHAR          *ppucProducer;
    PUCHAR          *ppucBufferStart;
    PUCHAR          *ppucBufferEnd;
    PUCHAR          *ppucConsumer;  
    PACKET_HEADER   *pPacketHeader;

    if (bIsTx)
    {
        ppucProducer    =   &pucTxProducer;
        ppucConsumer    =   &pucTxConsumer;
        ppucBufferStart =   &pucTxMin;
        ppucBufferEnd   =   &pucTxMax;
        
    }
    else
    {
        ppucProducer    =   &pucRxProducer;
        ppucConsumer    =   &pucRxConsumer;
        ppucBufferStart =   &pucRxMin;
        ppucBufferEnd   =   &pucRxMax;      
    }


    //
    //  Anything at all ???
    //

    if (*ppucProducer == *ppucConsumer)
        return NULL;


    //
    //  Something avail..  SKIP if it's SKIP buffer..
    //

    pPacketHeader = (PACKET_HEADER *)*ppucConsumer;

    if (pPacketHeader->dwSignature != HEADER_SIGNATURE)
    {
        PRINTMSG (1,
            ("VBridge: Error! ConsumeBuffer() Corrupted header signature [0x%x]!",
            pPacketHeader->dwSignature));
            
        return NULL;
    }                

    if (pPacketHeader->dwFlag & FLAG_SKIP_BUFFER)
    {
        PRINTMSG (0, ("SKIP consumed [0x%x].\r\n",
            *ppucConsumer));

        *ppucConsumer = pPacketHeader->pucNextBuffer;       

        //
        //  Now, check if producer has produced after the SKIP buffer 
        //

        if (*ppucProducer == *ppucConsumer)
            return NULL;
    }

    //
    //  By now we may have data..
    //

    pPacketHeader = (PACKET_HEADER *)*ppucConsumer;

    if (pPacketHeader->dwSignature != HEADER_SIGNATURE)
    {
        PRINTMSG (1,
            ("VBridge: Error! ConsumeBuffer() Corrupted header signature [0x%x]!",
            pPacketHeader->dwSignature));
            
        return NULL;
    }
    

    //
    //  See if it is ready for consumption..
    //

    if (!(pPacketHeader->dwFlag & FLAG_PRODUCED))
    {
        //
        //  Buffer not yet cooked..
        //

        return NULL;
    }

    
    //
    //  This buffer is now in the process of being consumed..
    //

    pPacketHeader->dwFlag &= ~FLAG_PRODUCED;

    *pdwSize = pPacketHeader->dwCurrentSize;

    return ((PUCHAR)pPacketHeader + sizeof(PACKET_HEADER));

}   //  ConsumeBuffer() 



////////////////////////////////////////////////////////////////////////////////
//  ConsumeBufferDone()
//
//  Routine Description:
//
//      This function will mark the chunk of buffer ready for production usage..
//
//  Arguments:  
//
//      bIsTx       ::  Whether it is TX buffer or RX buffer.
//      pucBuffer   ::  Points to buffer obtained in ProduceBuffer()
//
//  Return Value:
//
//      None.
//

void
ConsumeBufferDone(BOOL  bIsTx, PUCHAR pucBuffer)
{
    PACKET_HEADER   *pPacketHeader;
    PUCHAR          *ppucConsumer;      

    if (bIsTx)
    {   
        ppucConsumer    =   &pucTxConsumer;         
    }
    else
    {       
        ppucConsumer    =   &pucRxConsumer; 
    }


    pPacketHeader = (PPACKET_HEADER)(pucBuffer - sizeof(PACKET_HEADER));

    //
    //  This means we require only one outstanding consumer at anyone time..
    //

    if (*ppucConsumer != (pucBuffer - sizeof(PACKET_HEADER)))
    {
        PRINTMSG (1,
            ("VBridge: Error! ConsumeBufferDone() incorrect pucBuffer [0x%x] passed in ppucConsumer is [0x%x]",
            pucBuffer,
            *ppucConsumer));
            
        return;

    }


    if (pPacketHeader->dwSignature != HEADER_SIGNATURE)
    {
        PRINTMSG (1,
            ("VBridge: Error! ConsumeBufferDone() Corrupted header signature [0x%x]!",
            pPacketHeader->dwSignature));
            
        return;
    }    

    pPacketHeader->dwFlag = FLAG_CONSUMED;
    
    *ppucConsumer = pPacketHeader->pucNextBuffer;

}   //  ConsumeBufferDone() 




/* -------------------------------------------------------------------------- */
/* Start of VBridge exported functions.                                       */
/* -------------------------------------------------------------------------- */



////////////////////////////////////////////////////////////////////////////////
//  VBridgeInit()
//
//  Routine Description:
//
//      Called to initialize all the VBridge related variables.
//      This should be called by OEMEthInit() code, and hence it will be running
//      in KERNEL mode.
//
//  Arguments:  
//
//      None.
//
//  Return Value:
//
//      TRUE    ::  Successful.
//      FALSE   ::  Otherwise..
//

BOOL
VBridgeInit()
{
    pucTxProducer = (PUCHAR)&TxBuffers[0];
    pucTxConsumer = (PUCHAR)&TxBuffers[0];
    pucRxProducer = (PUCHAR)&RxBuffers[0];
    pucRxConsumer = (PUCHAR)&RxBuffers[0];

    pucTxMax      = (PUCHAR)&TxBuffers[TX_BUFFERS_DWORD];
    pucTxMin      = pucTxProducer;

    pucRxMax      = (PUCHAR)&RxBuffers[RX_BUFFERS_DWORD];
    pucRxMin      = pucRxProducer;  

    PRINTMSG (1, ("VBridge:: built on [%s] time [%s]\r\n",
        __DATE__,
        __TIME__));

    PRINTMSG (1, ("VBridgeInit()...TX = [%d] bytes -- Rx = [%d] bytes\r\n",
        pucTxMax - pucTxProducer,
        pucRxMax - pucRxProducer));

    PRINTMSG (1, ("Tx buffer [0x%x] to [0x%x].\r\n",
        pucTxMin,
        pucTxMax));

    PRINTMSG (1, ("Rx buffer [0x%x] to [0x%x].\r\n",
        pucRxMin,
        pucRxMax));    

    g_bResetBuffer = FALSE;

    return TRUE;

}   //  VBridgeInit()



////////////////////////////////////////////////////////////////////////////////
//  VBridgeKGetOneTxBuffer()
//
//  Routine Description:
//
//      Called by kernel mode code to query if there is any packet to be sent.
//      This function will return the pointer to the buffer (containing ethernet
//      packet) and the length of the actual data.
//      
//  Arguments:  
//
//      ppucBuffer  ::  Points to the start of the ethernet packet.
//      puiLength   ::  Length of the packet..
//
//  Return Value:
//
//      TRUE    ::  There is data to be sent.
//      FALSE   ::  No data to be sent.
//

BOOL
VBridgeKGetOneTxBuffer(
    PUCHAR  *ppucBuffer,
    UINT    *puiLength)
{
    if (g_bResetBuffer)
        VBridgeInit();

    if (ppucBuffer == NULL || puiLength == NULL)
    {
        PRINTMSG (1, ("VBridgeKGetOneTxBuffer() Err! [0x%x] [0x%x]\r\n",
            ppucBuffer,
            puiLength));

        return FALSE;
    }

    *ppucBuffer = ConsumeBuffer(TRUE, (DWORD *)puiLength);

    if (*ppucBuffer)
    {
        PRINTMSG (0, ("> "));
        return TRUE;
    }
    else 
        return FALSE;

}   //  VBridgeKGetOneTxBuffer()



////////////////////////////////////////////////////////////////////////////////
//  VBridgeKGetOneTxBufferComplete()
//
//  Routine Description:
//
//      This must follow VBridgeKGetOneTxBuffer().
//
//  Arguments:
//
//      pucBuffer   ::  Pointer to the buffer that kernel is returning to 
//                      VBridge.
//
//  Return Values:
//
//      None.
//
//  Mode:
//
//      Kernel mode only.
//
//

void        
VBridgeKGetOneTxBufferComplete(PUCHAR pucBuffer)
{
    if (g_bResetBuffer)
        VBridgeInit();

    if (pucBuffer == NULL)
    {
        PRINTMSG (1, 
            ("VBridgeKGetOneTxBufferComplete(), NULL pucBuffer!\r\n"));
        return;
    }

    ConsumeBufferDone(TRUE, pucBuffer);

}   //  VBridgeKGetOneTxBufferComplete()



////////////////////////////////////////////////////////////////////////////////
//  VBridgeKIndicateOneRxBuffer()
//
//  Routine Description:
//
//      This is called by kernel code to insert an RX ethernet packet for 
//      VMini.
//
//  Arguments:
//
//      pBuffer     ::  Points to the the rx'ed ethernet buffer.
//      uiLength    ::  Length of the packet.
//      bSwappable  ::  IGNORED (no longer supported)..
//                      
//  Return Values:
//      
//      The original buffer pointer, contents are copied locally.
//
//  Mode:
//      Kernel Mode only.
//
PUCHAR
VBridgeKIndicateOneRxBuffer(PUCHAR pBuffer, UINT uiLength, BOOL bSwappable, BOOL *pbTaken)
{
    PUCHAR  pucKernelBuffer;    

    UNREFERENCED_PARAMETER(bSwappable);

    //  Return packet back to KITL/EDBG when there isn't a client
    if (!g_bUOpened) {
        *pbTaken = FALSE;
        return pBuffer;
    }
    
    //
    //  Platform may have bug not calling VBridgeKSetLocalMacAddress()...
    //  Pass all the packets to EDBG..
    //

    if (g_bResetBuffer)
        VBridgeInit();

    if (pBuffer == NULL || uiLength > MAX_8023_PDU || pbTaken == NULL)
    {
        PRINTMSG(1,
            ("VBridgeKIndicateOneRxBuffer() Err [0x%x] [0x%x] [0x%x]\r\n",
            pBuffer,
            uiLength,
            pbTaken));

        return FALSE;
    }

    if (!bValidAddress)
    {               
        if (!bWarnNoMacDone)
        {
            PRINTMSG (1, 
                ("** WARNING!!! ** Device MAC addr not set to VBridge..\r\n"));
            bWarnNoMacDone = TRUE;
        }

        *pbTaken = FALSE;
        return pBuffer;
    }

    //
    //  By now, the packet has to be passed up to VMINI.
    //  There is no further filtering needed here since the h/w should have
    //  been set to filter correctly.
    //  First, pop a buffer from kernel's free buffer.
    //  If none is free, then we simply toss away this packet.
    //
    
    pucKernelBuffer = ProduceBuffer (FALSE, uiLength);

    if (pucKernelBuffer == NULL)        
    {
        PRINTMSG (0,
            ("VBridgeKIndicateOneRxBuffer():: no mem!\r\n"));

        PRINTMSG (0,
            ("!"));

        return pBuffer;
    }
    
    memcpy (pucKernelBuffer, pBuffer, uiLength);
    
    ProduceBufferDone (FALSE, pucKernelBuffer); 

    PRINTMSG (0, 
        ("Rx [0x%x] [%d] bytes\r\n", 
        pucKernelBuffer,
        uiLength));

    //
    //  Trigger the RX event VMini is waiting for..
    //  
    
    NKSetInterruptEvent(SYSINTR_VMINI);
    return pBuffer;

}   //  VBridgeKIndicateOneRxBuffer()



////////////////////////////////////////////////////////////////////////////////
//  VBridgeKSetLocalMacAddress()    
//
//  Routine Description:
//
//      Kernel only, used to indicate the MAC address currently used by the EDBG.
//
//  Arguments:
//
//      pucMacAddress   ::  The MAC address passed in here.
//  
//  Return Values:
//
//      None.
//
//  Mode:
//
//      Kernel Mode only.
//

void        
VBridgeKSetLocalMacAddress(PUCHAR pucMacAddress)
{
    if (pucMacAddress == NULL)
    {
        PRINTMSG (1, 
            ("VBridgeKSetLocalMacAddress() Err!! Null pucMacAddress.\r\n"));
        return;
    }

    PRINTMSG (1, ("VBridge:: NK add MAC: [%x-%x-%x-%x-%x-%x]\r\n",
        pucMacAddress[0],
        pucMacAddress[1],
        pucMacAddress[2],
        pucMacAddress[3],
        pucMacAddress[4],
        pucMacAddress[5]));   

    memcpy(
        EDBGMacAddress,
        pucMacAddress,
        0x06);

    bValidAddress = TRUE;

}   //  VBridgeKSetLocalMacAddress()



////////////////////////////////////////////////////////////////////////////////
//  VBridgeUGetOneTxBuffer()
//
//  Routine Description:
//
//      Called by user mode code to obtain a buffer for TX.   
//
//  Arguments:
//
//      ppucBuffer  :: If successful, points to the free buffer that VMini
//                     can use to fill up its TX packet.
//      uiSize      :: Size of the buffer requested..
//
//  Return Values:
//
//      TRUE    ::  If there is free buffer available.
//      FALSE   ::  Otherwise.
//
//  Mode:
//
//      User Mode only.
//

BOOL        
VBridgeUGetOneTxPacket(PUCHAR *ppucBuffer, UINT uiSize)
{
    if (ppucBuffer == NULL || uiSize > MAX_8023_PDU)
    {
        PRINTMSG (1, 
            ("VBridgeUGetOneTxPacket() Err!! [0x%x] [0x%x].\r\n",
            ppucBuffer,
            uiSize));

        return FALSE;
    }

    *ppucBuffer = ProduceBuffer(TRUE, (DWORD)uiSize);

    if (*ppucBuffer == NULL)
    {
        PRINTMSG (0 , ("@ "));
        return FALSE;
    }
    else
        return TRUE;    

}   //  VBridgeUGetOneTxBuffer()



////////////////////////////////////////////////////////////////////////////////
//  VBridgeUGetOneTxBufferComplete()
//
//  Routine Description:
//      This function will complete the user's call to send a packet.
//      The user should have called VBridgeUGetOneTxBuffer() and then return 
//      the filled buffer here, ready to be sent.
//      VBridge will then insert the buffer to TX queue for kernel to look at.
//
//  Arguments:
//      pucBuffer   ::  Pointer to buffer returned previously by 
//                          VBridgeGetOneTxBuffer().
//      uiLength    ::  Length of the user packet to be sent.
//
//  Return Values:
//      None.
//
//  Mode:
//      User Mode only.
//
//  Notes:
//      It is important that caller makes sure that the 
//      VBridgeUGetOneTxBufferComplete() is called after each 
//      VBridgeUGetOneTxBuffer().   And the total number called has to 
//      match.
//

void
VBridgeUGetOneTxPacketComplete(PUCHAR pucBuffer, UINT uiLength)
{
    if (pucBuffer == NULL || uiLength > MAX_8023_PDU)
    {
        PRINTMSG (1, 
            ("VBridgeUGetOneTxPacketComplete() Err!! [0x%x] [0x%x].\r\n",
            pucBuffer,
            uiLength));
        return;
    }

    PRINTMSG (0, ("Tx: %x%x%x%x%x%x  %x%x%x%x%x%x[%d]\r\n",
        pucBuffer[6],
        pucBuffer[7],
        pucBuffer[8],
        pucBuffer[9],
        pucBuffer[10],
        pucBuffer[11],

        pucBuffer[0],
        pucBuffer[1],
        pucBuffer[2],
        pucBuffer[3],
        pucBuffer[4],
        pucBuffer[5],   
        uiLength));

    RETAILMSG (0, (TEXT("VBridgeUGetOneTxPacketComplete():: Sending to: %x-%x-%x-%x-%x-%x\r\n"),
        pucBuffer[0],
        pucBuffer[1],
        pucBuffer[2],
        pucBuffer[3],
        pucBuffer[4],
        pucBuffer[5]));

#if 0

    PRINTMSG (1, ("Tx: Sending: \r\n"));
    VBridgeDumpMemory(pucBuffer, uiLength);

#endif

    ProduceBufferDone(TRUE, pucBuffer); 

}   //  VBridgeUGetOneTxBufferComplete()



////////////////////////////////////////////////////////////////////////////////
//  VBridgeUGetOneRxPacket()
//
//  Routine Description:
//
//      Called by user mode code to retrieve an RX packet.
//
//  Arguments:
//  
//      pucBuffer   ::  Pointer to buffer 
//      uiLength    ::  Length of the user packet to be sent.
//
//  Return Values:
//
//      TRUE    ::  If there is a pending RX packet.
//      FALSE   ::  Otherwise.
//
//  Mode:
//
//      User Mode only.
//

BOOL        
VBridgeUGetOneRxPacket(PUCHAR *ppucBuffer, UINT *puiLength)
{
    if (ppucBuffer == NULL || puiLength == NULL)
    {
        PRINTMSG (1, 
            ("VBridgeUGetOneRxPacket() Err!! [0x%x] [0x%x].\r\n",
            ppucBuffer,
            puiLength));

        return FALSE;
    }

    *ppucBuffer = ConsumeBuffer(FALSE, (DWORD *)puiLength);  
    
    if (*ppucBuffer)
    {
        PRINTMSG (0,
            ("Rx consuming [0x%x][%d]\r\n",
            *ppucBuffer,
            *puiLength));
        return TRUE;
    }
    else 
    {
        PRINTMSG (0, ("-"));
        return FALSE;
    }

}   //  VBridgeUGetOneRxPacket()



////////////////////////////////////////////////////////////////////////////////
//  VBridgeUGetOneRxPacketComplete()
//
//  Routine Descriptions:
//
//      Called by user mode code to return the buffer retrieved via 
//      VBridgeUGetOneRxPacket()
//
//  Arguments:
//
//      pucBuffer   ::  Pointer to buffer 
//      uiLength    ::  Length of the user packet to be sent.
//
//  Return Values:
//
//      Always TRUE..
//
//  Mode:
//      User Mode only.
//
//  Note:
//      It is important that the caller match this call to the
//      VBridgeUGetOneRxPacket()
//

BOOL        
VBridgeUGetOneRxPacketComplete(PUCHAR pucBuffer)
{
    if (pucBuffer == NULL)
    {
        PRINTMSG (1, 
            ("VBridgeUGetOneRxPacketComplete() NULL pucBuffer"));
            
        return FALSE;
    }

    ConsumeBufferDone(FALSE, pucBuffer);

    PRINTMSG (0, 
        ("Rx consumed [0x%x]r\n",
        pucBuffer));

    return TRUE;

}   //  VBridgeUGetOneRxPacketComplete()




////////////////////////////////////////////////////////////////////////////////
//  VBridgeUWildCard()
//
//  Routine Descriptions:
//
//      *** Not Used ***    For future expansion..
//
//  Arguments:
//
//
//  Return Values:
//
//      Always TRUE..
//
//  Mode:
//      User Mode only.
//
//

BOOL
VBridgeUWildCard(
    LPVOID  lpInBuf,
    DWORD   nInBufSize,
    LPVOID  lpOutBuf,
    DWORD   nOutBufSize,
    LPDWORD lpBytesReturned)
{
    UNREFERENCED_PARAMETER(nInBufSize);
    switch ((DWORD)lpInBuf)
    {   
        case IOCTL_VBRIDGE_WILD_CARD_RESET_BUFFER:
        {
            PRINTMSG (1, ("VBridge:: RESET_BUFFER received.\r\n"));
            dwCurrentPacketFilter = 0x00;
            g_bResetBuffer = TRUE;
            break;
        }            

        case IOCTL_VBRIDGE_WILD_CARD_VB_INITIALIZED:
            if (nOutBufSize == sizeof(DWORD))
            {       
                PRINTMSG (1, ("VBridge:: VB_INITIALIZED returns [%d]\r\n",
                    bValidAddress));
                *(DWORD *)lpOutBuf = bValidAddress;

                if (lpBytesReturned)
                    *lpBytesReturned   = sizeof(DWORD);
            }
            break;

        default:
            break;
    }

    return TRUE;

}   //  VBridgeUWildCard()



////////////////////////////////////////////////////////////////////////////////
//  VBridgeUGetEdbgMac()
//
//  Routine Descriptions:
//
//      Called by the user mode to get the EDBG mac address (this should be the
//      normal case, EDBG and VMini sharing same MAC).
//
//  Arguments:
//
//      pucMacAddress   :: [out] the mac address we have learnt from 
//                               kernel mode code.
//
//  Mode:
//
//      User Mode only.
//
void
VBridgeUGetEDBGMac(PUCHAR pucMacAddress)
{
    if (pucMacAddress == NULL)
    {
        PRINTMSG (1,
            ("VBridgeUGetEDBGMac(), ERR! NULL pucMacAddress.\r\n"));
        return;
    }

    // Open bridge, there is client on user side
    g_bUOpened = TRUE;

    memcpy(
        pucMacAddress,
        EDBGMacAddress,
        6);

}   //  VBridgeUGetEDBGMac()



////////////////////////////////////////////////////////////////////////////////
//  VBridgeUCurrentPacketFilter()
//  
//  Routine Descriptions:
//
//      Called by user mode code informing us the current filtering mode
//      the hardware is being set to.
//
//  Arguments:
//
//      pdwFilter   :: point to the new filtering mode.
//
//  Return Value:
//
//      None.
//
//
void
VBridgeUCurrentPacketFilter(PDWORD  pdwFilter)
{
    if (pdwFilter == NULL)
    {
        PRINTMSG (1,
            ("VBridgeUCurrentPacketFilter(), ERR! NULL pdwFilter.\r\n"));

        return;
    }

    PRINTMSG (1, ("VBridge:: Current VMini packet filter = [0x%x]\r\n",
        *pdwFilter));

    dwCurrentPacketFilter = *pdwFilter; 

}   //  VBridgeUCurrentPacketFilter()

