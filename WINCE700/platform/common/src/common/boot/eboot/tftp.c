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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:
    tftp.c
Abstract:

    This contains the TFTP client routines for the bootloader.
    For the most part, these operate on top of the udp.c module.
    There are some global data structures that are shared with
    the tftpd.c module.

Functions:


Notes:

--*/
#include <windows.h>
#include <udp.h>
#include <tftp.h>


// These are the UDP port numbers that are used for the TFTP servers on the Odo and Host sides
UINT16 wOdoWellKnownServerPort, wHostWellKnownServerPort;
TFtpLinkRec TFtpLinks[MAX_TFTP_LINKS];  // Data records for each of the possible TFTP links
UINT16 nNumTFtpLinksInUse;              // The number of active TFTP links

BYTE TFTPFrameBuffer[MAX_TFTP_DATA_BUFFERSIZE + UDP_DATA_FRAME_OFFSET];

void EbootInitTFtp( UINT16 wOdoWKSP, UINT16 wHostWKSP ) {

    int i;

    for( i = 0; i < MAX_TFTP_LINKS; i++ )
        TFtpLinks[i].State = TFTP_STATE_IDLE;

    wOdoWellKnownServerPort = wOdoWKSP;
    wHostWellKnownServerPort = wHostWKSP;

    nNumTFtpLinksInUse = 0;

}


// This routine handles the multiplexing of all active TFTP links.  It returns 0 if there was no valid link formed,
//  or 1 of there was.
WORD EbootTFtpReceiver( EDBG_ADDR *pMyAddr, BYTE *pFrameBuffer, UINT16 wDestPort, UINT16 wSrcPort, UINT16 *pwUDPData, UINT16 cwUDPDataLength ) {

    UINT16 i;
    UINT16 iLinkSlot = MAX_TFTP_LINKS;

    // Check to see if this is for a link that is currently in use
    for( i = 0; i < MAX_TFTP_LINKS; i++ ) {
        // Here I don't compare the destination (Odo) port, only the source (host) because
        //  this could be a repeat open packet, which we don't want to cause a second link.
        //  This could happen if a second open packet comes because we will have changed the
        //  destination port with the acknowledge.
        if (TFtpLinks[i].State != TFTP_STATE_IDLE && wSrcPort == TFtpLinks[i].DestAddr.wPort
            && wDestPort == TFtpLinks[i].SrcAddr.wPort) {
            iLinkSlot = i;
            break;
        }
        else if (TFtpLinks[i].State == TFTP_STATE_IDLE)
            iLinkSlot = i;
    }

    // If we broke out of the loop early, then the packet is for a link that is already open
    if (i < MAX_TFTP_LINKS) {
        // iLinkSlot is the index of the link this packet belongs too
        TFtpStateMachine( wSrcPort, iLinkSlot, pwUDPData, cwUDPDataLength );
    }
    // Check to see if someone is trying to start a new connection,
    // If so, guarantee that there are always TFTP_TX_LINKS link(s) available to transmit information
    else if (wDestPort == wOdoWellKnownServerPort && MAX_TFTP_LINKS - nNumTFtpLinksInUse > TFTP_TX_LINKS) {

        // iLinkSlot is the index of a link that is in the IDLE state, giving the number of a free link
        //  slot that can be used, the wDestTID and the TFTP Message
        TFtpdFormNewLink( pMyAddr, pFrameBuffer, iLinkSlot, pwUDPData, cwUDPDataLength );
        return 1;
    }
    else {

        for( i = 0; i < MAX_TFTP_LINKS; i++ ) {
            KITLOutputDebugString("TFTP link[%u]: State:%u, DestAddr.wPort: %u, SrcAddr.wPort: %u\r\n",
                               i,TFtpLinks[i].State, ntohs(TFtpLinks[i].DestAddr.wPort),
                               ntohs(TFtpLinks[i].SrcAddr.wPort));
            }
        KITLOutputDebugString("TftpReceiver, port: 0x%X, wkp: 0x%X\r\n",wDestPort,wOdoWellKnownServerPort);
    }

    return 0;

} // TFtpReceiver()



// This routine will go through and check all the links to perform retransmits and eventual timeouts.
// It uses the same retransmit strategy as DHCP, which is an exponential back off.  It starts with
//  4 seconds, then 8, 16 ... up to the last retry at 64 seconds for MAX_RETRIES = 5.
void TFTPRetransmit( void ) {

    DWORD dwCurrentTime;
    int i;

    dwCurrentTime = OEMKitlGetSecs();
    for( i = 0; i < MAX_TFTP_LINKS; i++ )
        if (TFtpLinks[i].State != TFTP_STATE_IDLE &&
            dwCurrentTime - TFtpLinks[i].tLastTransmit >= (4UL << TFtpLinks[i].cwRetries)) {
            // If there are still retries left, try again
            if (TFtpLinks[i].cwRetries <= MAX_RETRIES) {
                if (!EbootSendUDP(TFTPFrameBuffer, &(TFtpLinks[i].DestAddr), &(TFtpLinks[i].SrcAddr), TFtpLinks[i].Buffer, TFtpLinks[i].cwMsgLen ))
                    KITLOutputDebugString( "TFTPRetransmit()::Error On SendUDP() Call\r\n" );
                TFtpLinks[i].tLastTransmit = dwCurrentTime;
                TFtpLinks[i].cwRetries++;
            }
            // Otherwise, the link should be timed out, so send an error packet and then discard the link
            else
                TFtpKillLink( (UINT16)i, TFTP_ERROR_UNDEFINED, "Linked Timed Out By Odo" );
            KITLOutputDebugString( "TFTPRetransmit() Slot %u, wDestTID %u, Retry %u\r\n",
                i, TFtpLinks[i].DestAddr.wPort, TFtpLinks[i].cwRetries );
        } // If it's time to do a retry at sending the packet
}



// This will format the pBuffer to be a TFTP Error message of the ErrorCode type and with
//  the human readable error message.
void TFtpKillLink( UINT16 iLinkSlot, TFtpErrors ErrorCode, char *pszErrorMessage ) {

    if (TFtpLinks[iLinkSlot].State == TFTP_STATE_IDLE)
        return;

    // Only send a packet if the error was generated on this end.  If it was generated by the host
    //  then we need not send anything back.
    if (ErrorCode != TFTP_ERROR_HOSTERROR) {
        // Put in the code for an error packet
        *((UINT16 *)(TFtpLinks[iLinkSlot].Buffer)) = htons(TFTP_OPCODE_ERROR);
        // Put in the type of error
        *((UINT16 *)(TFtpLinks[iLinkSlot].Buffer) + 1) = htons(ErrorCode);
        // Put in the error message itself
        strcpy_s( (char *)(TFtpLinks[iLinkSlot].Buffer + 4), sizeof(TFtpLinks[iLinkSlot].Buffer) - 4, pszErrorMessage );
        TFtpLinks[iLinkSlot].cwMsgLen = (UINT16)(4 + strlen( pszErrorMessage ) + 1);

        EbootSendUDP(TFTPFrameBuffer, &(TFtpLinks[iLinkSlot].DestAddr), &(TFtpLinks[iLinkSlot].SrcAddr),
                     TFtpLinks[iLinkSlot].Buffer, TFtpLinks[iLinkSlot].cwMsgLen );
    }

    // If there is a callback function, inform it that the link is now dead
    if (TFtpLinks[iLinkSlot].pfCallBack != NULL) {
        TFtpdCallBack( iLinkSlot, TFTPD_DESTROY );
        TFtpLinks[iLinkSlot].State = TFTP_STATE_IDLE;
        nNumTFtpLinksInUse--;
    }
    // If this was an Odo client link, then we won't be able to inform the process of the error until it
    //  calls TFtpRead() or TFtpWrite() so transition to the TFTP_STATE_ERROR state until one of those
    //  routines is called
    else
        TFtpLinks[iLinkSlot].State = TFTP_STATE_ERROR;
}



// This routine will look at the current state of the link and at the data that is available and will
//  advance the link to the next state.  This may entail sending an acknowledge packet, requesting data
//  from a server process, closing the link, etc.
void TFtpStateMachine( WORD wSrcPort, UINT16 iLinkSlot, UINT16 *pwMsg, UINT16 cwMsgLen ) {

    // If this is an error packet, kill the link right now
    if (ntohs(*pwMsg) == 5) {
        TFtpKillLink( iLinkSlot, TFTP_ERROR_HOSTERROR, NULL );
        KITLOutputDebugString( "TFTP Error Received From Host %X - %s\r\n", *(pwMsg + 1), pwMsg + 2 );
        return;
    }

    switch( TFtpLinks[iLinkSlot].State ) {
        case TFTP_STATE_IDLE:
            // We shouldn't get called if the packet was for a link that's in the TFTP_STATE_IDLE state
            break;
        case TFTP_STATE_OPEN:
            // We will only get in this state if this is an H2OLink that we initiated, we're waiting
            //  for the host's write acknowledge packet
            if (ntohs(*pwMsg) == 4 && ntohs(*(pwMsg+1)) == 0)
                TFtpLinks[iLinkSlot].State = TFTP_STATE_XFER_BUSY;
            break;
        case TFTP_STATE_XFER_BUSY:
            // We are either waiting for the data to be taken out of the buffer by a read, or to be
            //  placed there by a write.  When one of those things happens, they will cause either an
            //  acknowledge or new data to be sent.  For now, just ignore this packet.
            break;
        case TFTP_STATE_XFER_WAIT:
            if (TFtpLinks[iLinkSlot].DataDir == H2OLink) {
                // If this is the data packet we've been waiting for
                if (ntohs(*pwMsg) == 3) {
                    if ((USHORT)(ntohs(*(pwMsg+1))) == (USHORT)(TFtpLinks[iLinkSlot].wBlockNumber+1)) {
                        // If this was a brand new link we have to copy the dest TID off of the first packet
                        if (TFtpLinks[iLinkSlot].wBlockNumber == 0)
                            TFtpLinks[iLinkSlot].DestAddr.wPort = wSrcPort;

                        // Copy the data out of the temporary buffer
                        TFtpLinks[iLinkSlot].cwMsgLen = cwMsgLen;
                        TFtpLinks[iLinkSlot].wBlockNumber++;
                        memcpy( TFtpLinks[iLinkSlot].Buffer, pwMsg, cwMsgLen );
                        // Flip the state back to BUSY so that TFtpRead() can succeed.
                        TFtpLinks[iLinkSlot].State = TFTP_STATE_XFER_BUSY;
                        if (TFtpLinks[iLinkSlot].pfCallBack != NULL)
                            TFtpdCallBack( iLinkSlot, TFTPD_WRITE );
                        // Check to see if this is the last packet, if so we close
                        if (cwMsgLen != TFtpLinks[iLinkSlot].wMaxTftpData) {
                            TFtpLinks[iLinkSlot].State = TFTP_STATE_IDLE;
                            nNumTFtpLinksInUse--;
                        }
                    } else if (ntohs(*(pwMsg+1)) == TFtpLinks[iLinkSlot].wBlockNumber) {
                        // desktop lost the ACK for previous write, ack it again
                        UINT16 ack[2] = {0};
                        BYTE buf[UDP_DATA_FRAME_OFFSET+sizeof(ack)];
                        ack[0] = htons(TFTP_OPCODE_ACK); 
                        ack[1] = htons(TFtpLinks[iLinkSlot].wBlockNumber);
                        KITLOutputDebugString ("TFTP: Desktop losing ACK, block number = %d, Ack again\r\n",
                            TFtpLinks[iLinkSlot].wBlockNumber);
                        EbootSendUDP(buf, &(TFtpLinks[iLinkSlot].DestAddr), &(TFtpLinks[iLinkSlot].SrcAddr),
                            (LPBYTE) ack, sizeof (ack));
                    }
                }
            }
            else {
                // See if this is the acknowledge packet we've been waiting for
                if (ntohs(*pwMsg) == 4 && ntohs(*(pwMsg+1)) == TFtpLinks[iLinkSlot].wBlockNumber) {
                    // Flip the state back to BUSY so that TFtpWrite() can succeed.
                    TFtpLinks[iLinkSlot].State = TFTP_STATE_XFER_BUSY;
                    if (TFtpLinks[iLinkSlot].pfCallBack != NULL)
                        TFtpdCallBack( iLinkSlot, TFTPD_READ );
                    // Check to see if this is the last packet, if so we close
                    if (TFtpLinks[iLinkSlot].cwMsgLen != TFtpLinks[iLinkSlot].wMaxTftpData)
                        TFtpLinks[iLinkSlot].State = TFTP_STATE_CLOSE;
                }
            }
            break;
        case TFTP_STATE_CLOSE:
            // This was an O2HLink (Write), and we're just waiting for that last data packet to be acknowledged.
            //  If it has been, we'll close the link and go back to IDLE
            if (ntohs(*pwMsg) == 4 && ntohs(*(pwMsg+1)) == TFtpLinks[iLinkSlot].wBlockNumber) {
                TFtpLinks[iLinkSlot].State = TFTP_STATE_IDLE;
                nNumTFtpLinksInUse--;
                KITLOutputDebugString( "TFtpStateMachine::TFTP_STATE_CLOSE Link Deleted\r\n" );
            }
            break;
    } // switch(State)

}



// This routine will open a new link to the host.  The DataDir parameter specifies if this is going to
//  be a request to read (H2OLink) or write (O2HLink) data.  The routine will return a non-zero link handle,
//  which is actually the iLinkSlot+1, if there is a link slot available.  Otherwise, it will return 0.
UINT16 TFtpOpen( EDBG_ADDR *pHostAddr, EDBG_ADDR *pMyAddr, char *pszFileName, TFtpLinkDir DataDir ) {

    int i, iLinkSlot = (MAX_TFTP_LINKS - 1);

    if (nNumTFtpLinksInUse >= MAX_TFTP_LINKS)
        return 0;

    if ((strlen(pszFileName) + 1) > MAX_TFTP_FILENAME) return 0;

    // Find a link that isn't currently in use
    for( i = 0; i < MAX_TFTP_LINKS; i++ ) {
        if (TFtpLinks[i].State == TFTP_STATE_IDLE) {
            iLinkSlot = i;
            break;
        }
    }

    TFtpLinks[iLinkSlot].DataDir = DataDir;
    TFtpLinks[iLinkSlot].State = (DataDir == H2OLink) ? TFTP_STATE_XFER_WAIT : TFTP_STATE_OPEN;
    TFtpLinks[iLinkSlot].pfCallBack = NULL;
    strcpy_s( TFtpLinks[iLinkSlot].szFileName, sizeof(TFtpLinks[iLinkSlot].szFileName), pszFileName );
    TFtpLinks[iLinkSlot].wBlockNumber = 0;

    // Put in Read Request or Write Request as appropriate
    *((UINT16*)(TFtpLinks[iLinkSlot].Buffer)) = htons((DataDir == H2OLink) ? 1 : 2);
    TFtpLinks[iLinkSlot].cwMsgLen = 2;
    // Put in the file name
    strcpy_s( (char *)(TFtpLinks[iLinkSlot].Buffer + 2), sizeof(TFtpLinks[iLinkSlot].Buffer) - 2, pszFileName );
    TFtpLinks[iLinkSlot].cwMsgLen += (UINT16)strlen( pszFileName );
    // Specify octet (binary) mode
    strcpy_s( (char *)(TFtpLinks[iLinkSlot].Buffer + TFtpLinks[iLinkSlot].cwMsgLen), sizeof(TFtpLinks[iLinkSlot].Buffer) - TFtpLinks[iLinkSlot].cwMsgLen, "octet" );
    TFtpLinks[iLinkSlot].cwMsgLen += 6;

    // Send the read/write request packet back to the host
    TFtpLinks[iLinkSlot].DestAddr = *pHostAddr;
    TFtpLinks[iLinkSlot].DestAddr.wPort = wHostWellKnownServerPort;
    TFtpLinks[iLinkSlot].SrcAddr = *pMyAddr;
    TFtpLinks[iLinkSlot].SrcAddr.wPort = GenerateSrcPort();
    EbootSendUDP(TFTPFrameBuffer, &(TFtpLinks[iLinkSlot].DestAddr), &(TFtpLinks[iLinkSlot].SrcAddr),
                 TFtpLinks[iLinkSlot].Buffer, TFtpLinks[iLinkSlot].cwMsgLen );
    TFtpLinks[iLinkSlot].tLastTransmit = OEMKitlGetSecs();
    TFtpLinks[iLinkSlot].cwRetries = 0;

    return (UINT16)(iLinkSlot+1);

}



// This routine is called by a process that wants to read data from an existing link.
//  The routine will return TFTP_WAIT if the read failed because no new data packet has arrived.
//  A TFTP_ERROR will be returned if an error has occured and the link has been terminated
//  It will return TFTP_SUCCESS if the read succeeded.  The number of data bytes read indicates
//  whether or not the link is closing.  If the number of data bytes is 512, then more
//  data is forthcoming.  If the number of data bytes is < 512, then this is the last
//  packet and the data link is now closed.
TFtpReturnCodes TFtpRead( UINT16 wLinkHandle, BYTE *pbData, UINT16 *cwDataLen ) {

    UINT16 iLinkSlot;

    iLinkSlot = wLinkHandle - 1;

    // For the sake of speed, I will assume that the wLinkHandle is valid

    if (TFtpLinks[iLinkSlot].State == TFTP_STATE_ERROR) {
        KITLOutputDebugString("!TftpReturnCodes, STATE_ERROR\r\n");

        TFtpLinks[iLinkSlot].State = TFTP_STATE_IDLE;
        nNumTFtpLinksInUse--;
        return TFTP_ERROR;
    }
    else if (TFtpLinks[iLinkSlot].State == TFTP_STATE_XFER_BUSY) {
        // Copy the data to the process' buffer
        *cwDataLen = TFtpLinks[iLinkSlot].cwMsgLen - 4;
        memcpy( TFtpLinks[iLinkSlot].Buffer + 4, pbData, *cwDataLen );

        // Send an acknowlege packet back to the host
        *((UINT16 *)TFtpLinks[iLinkSlot].Buffer) = htons(TFTP_OPCODE_ACK);
        *((UINT16 *)TFtpLinks[iLinkSlot].Buffer + 1) = htons(TFtpLinks[iLinkSlot].wBlockNumber);
        TFtpLinks[iLinkSlot].cwMsgLen = 4;
        EbootSendUDP(TFTPFrameBuffer, &(TFtpLinks[iLinkSlot].DestAddr), &(TFtpLinks[iLinkSlot].SrcAddr),
                     TFtpLinks[iLinkSlot].Buffer, TFtpLinks[iLinkSlot].cwMsgLen );
        TFtpLinks[iLinkSlot].tLastTransmit = OEMKitlGetSecs();
        TFtpLinks[iLinkSlot].cwRetries = 0;

        // I need to check to see if this is the last packet in the stream.  If so, I
        //  need to close the link
        if (TFtpLinks[iLinkSlot].cwMsgLen != TFtpLinks[iLinkSlot].wMaxTftpData) {
            TFtpLinks[iLinkSlot].State = TFTP_STATE_IDLE;
            nNumTFtpLinksInUse--;
        }
        // Otherwise, wait for the next data packet
        else
            TFtpLinks[iLinkSlot].State = TFTP_STATE_XFER_WAIT;
        return TFTP_SUCCESS;
    }

    return TFTP_WAIT;
}



// This routine is called by a process that wants to write data to an existing link.
//  The routine will return TFTP_WAIT if the write failed because the last data packet has not been
//  acknowledged.  A TFTP_ERROR will be returned if an error has occured and the link has been
//  terminated.  It will return TFTP_SUCCESS if the write succeeded.  The number of data bytes written
//  indicates whether or not the link is closing.  If the number of data bytes is 512, then more
//  data is forthcoming.  If the number of data bytes is < 512, then this is the last
//  packet and the data link is now closed.
TFtpReturnCodes TFtpWrite( UINT16 wLinkHandle, BYTE *pbData, UINT16 cwDataLen ) {

    UINT16 iLinkSlot;

    iLinkSlot = wLinkHandle - 1;

    // For the sake of speed, I will assume that the wLinkHandle is valid

    if (TFtpLinks[iLinkSlot].State == TFTP_STATE_ERROR) {
        KITLOutputDebugString("!!!! TFTP Errror\r\n");
        TFtpLinks[iLinkSlot].State = TFTP_STATE_IDLE;
        nNumTFtpLinksInUse--;
        return TFTP_ERROR;
    }
    else if (TFtpLinks[iLinkSlot].State == TFTP_STATE_XFER_BUSY) {
        // Send a new data packet to the host
        *((UINT16 *)TFtpLinks[iLinkSlot].Buffer) = htons(TFTP_OPCODE_DATA);
        TFtpLinks[iLinkSlot].wBlockNumber++;
        *((UINT16 *)TFtpLinks[iLinkSlot].Buffer + 1) = htons(TFtpLinks[iLinkSlot].wBlockNumber);
        memcpy( TFtpLinks[iLinkSlot].Buffer + 4, pbData, cwDataLen );
        TFtpLinks[iLinkSlot].cwMsgLen = cwDataLen + 4;

        EbootSendUDP(TFTPFrameBuffer, &(TFtpLinks[iLinkSlot].DestAddr), &(TFtpLinks[iLinkSlot].SrcAddr),
                     TFtpLinks[iLinkSlot].Buffer, TFtpLinks[iLinkSlot].cwMsgLen );
        TFtpLinks[iLinkSlot].tLastTransmit = OEMKitlGetSecs();
        TFtpLinks[iLinkSlot].cwRetries = 0;

        // I need to check to see if this is the last packet in the stream.  If so, I
        //  need go to TFTP_STATE_CLOSE so that we can wait for the last acknowledge packet.
        if (TFtpLinks[iLinkSlot].cwMsgLen != TFtpLinks[iLinkSlot].wMaxTftpData)
            TFtpLinks[iLinkSlot].State = TFTP_STATE_CLOSE;
        // Otherwise, wait for the next acknowledge packet
        else
            TFtpLinks[iLinkSlot].State = TFTP_STATE_XFER_WAIT;

        return TFTP_SUCCESS;
    }

    return TFTP_WAIT;
}



