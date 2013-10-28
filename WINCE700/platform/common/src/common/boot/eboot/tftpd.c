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
    tftpd.c

Abstract:
    This contains the TFTP server (daemon) routines for the
    bootloader.  For the most part, these operate on top of the udp.c
    module.  There are some global data structures that are shared with
    the tftp.c module.

Functions:


Notes:

--*/
#include <windows.h>
#include <udp.h>
#include <tftp.h>

TFtpdServerRegistryEntry TFtpdServerRegistry[MAX_TFTP_SERVER_PROCS];
BOOL  bDoLaunch = FALSE;

// Holds address of last TFTP download peer
EDBG_ADDR TftpPeerAddr = {0};

extern BYTE TFTPFrameBuffer[];

void EbootInitTFtpd( void ) {

    int i;

    for( i = 0; i < MAX_TFTP_SERVER_PROCS; i++ )
        TFtpdServerRegistry[i].fInUse = 0;
}


int __cdecl tolower(int c) {
    return ((c>='A') && (c<='Z')) ? (c-'A'+'a') : c;
}


// This function is called to associate a callback function with a given TFTP file name.
//  When a packet arrives that specifies the given file name, the callback will be called.
//  The requested operation is given, either TFTPD_OPEN, TFTPD_READ, TFTPD_WRITE, or TFTPD_DESTROY
//  and a pointer to a data buffer is given along with it.  The amount of valid data in the buffer is stored in
//  cwLength.  The amount of data cannot exceed 512 bytes, and in fact, must always be 512 bytes unless this is
//  the last packet that will be transferred over the link. If the number of data bytes is < 512 the link
//  is closing.
// The file name used for the associated link is also passed, allowing a single routine to act as a server for
//  multiple files.  The callback should not return until either all the data has been written to or taken from
//  the buffer because the buffer will later be destroyed.  It should then return 0 to indicate that there are no
//  errors and the link may continue.  If an error exists, the callback should return non-zero and the link will
//  be destroyed.
// When calling the TFtpServerRegister() routine, it will return 0 if the callback was successfully registered,
//  or non-zero if there are no more registration slots available.
UINT16 EbootTFtpdServerRegister( char *pszFileName, TFtpdCallBackFunc pfCallBack ) {

    int i;

    if ((strlen(pszFileName) + 1) > MAX_TFTP_FILENAME) return 1;

    for( i = 0; i < MAX_TFTP_SERVER_PROCS; i++ )
        if (TFtpdServerRegistry[i].fInUse == 0)
            break;
    if (i == MAX_TFTP_SERVER_PROCS)
        return 1;

    TFtpdServerRegistry[i].fInUse = 1;
    strcpy_s( TFtpdServerRegistry[i].szFileName, sizeof(TFtpdServerRegistry[i].szFileName), pszFileName );
    TFtpdServerRegistry[i].pfCallBack = pfCallBack;
    return 0;
}



// A server process can call this routine if it wants to remove itself as a server for the given file name
void EbootTFtpdServerUnRegister( char *pszFileName ) {

    int i;

    for( i = 0; i < MAX_TFTP_SERVER_PROCS; i++ )
        if (!strcmp( TFtpdServerRegistry[i].szFileName, pszFileName ))
            break;
    if (i != MAX_TFTP_SERVER_PROCS) {
        TFtpdServerRegistry[i].fInUse = 0;
        // Make sure to kill all links associated with this server
        for( i = 0; i < MAX_TFTP_LINKS; i++ )
            if (TFtpLinks[i].State != TFTP_STATE_IDLE && TFtpLinks[i].pfCallBack == TFtpdServerRegistry[i].pfCallBack)
                TFtpKillLink( (UINT16)i, TFTP_ERROR_FILENOTFOUND, "Server Process Has UnRegistered Itself" );
    }

}



//  This routine used to generate a pseudo-random source port, but for Vista compatibility we need to
//  use the same port for each download link as we used to form the initial connection.
UINT16 GenerateSrcPort( void ) {

    return htons(EDBG_DOWNLOAD_PORT);
}



// This routine is called when a packet arrives to form a new server link.  It will then attempt
//  to determine if there is a callback procedure associated with that file name, and
//  if so, it will then call that process with the TFTPD_OPEN command.  If all goes well, the
//  routine returns 0.  If there is an error, it will return non-zero.  The slot number that
//  is to be used for this link in the TFtpLinks[] table is passed in so that the callback
//  function pointer can be initialized for the link.  This is done to avoid the overhead of
//  looking up the callback address for every succeeding packet.
UINT16 TFtpdFormNewLink( EDBG_ADDR *pMyAddr, BYTE *pFrameBuffer, UINT16 iLinkSlot, UINT16 *pwMsg, UINT16 wMsgLength ) {
    char    *optionName, *optionValue;
    char    *pszParse, *StartOfPacketStringData,*EndOfPacketStringData;
    int     SubstringCount, NumberOfNameValuePairs, ServerProcessPointer;
    int     BlockSizeValue;


    StartOfPacketStringData = (char*)(pwMsg+1);         //After the first UINT16 there should exist the string data of the packet
                                                        //This includes the filename to transfer as well as optional options

    EndOfPacketStringData = (wMsgLength - sizeof(UINT16)) + StartOfPacketStringData;

    // Convert both strings to lower case so that the comparisons here will be case insensitive
    // Also perform simple check that string is valid
    //   Assume provided msglength is valid
    //   Count zeros in string -- this must be even and if divided by 2 provides # name/value pairs
    //   We require 1 name/value pair for NO OPTIONS, 2 name/value pairs allowed for 1 addl option
    KITLOutputDebugString("Packet has the following data:\r\n  ");

    SubstringCount=0;
    pszParse = StartOfPacketStringData;

    while(pszParse < EndOfPacketStringData)
    {
        *pszParse = (char)tolower(*pszParse);
        if(*pszParse == '\0')
            SubstringCount++;

        if(*pszParse)
        {
            KITLOutputDebugString("%c", *pszParse);
        }
        else
        {
            KITLOutputDebugString("[NULL]");
        }

        pszParse++;
    }

    KITLOutputDebugString("\r\n");

    if( (SubstringCount & 1) || (SubstringCount < 2) )
    {
        KITLOutputDebugString( "There must be an even number, and at least 2 substrings in packet.  Detected  %d\r\n", SubstringCount );
        return 1;
    }

    NumberOfNameValuePairs = SubstringCount / 2;            //Calculate number of name/value pairs

    //Number of options is equal to NumberOfNameValuePairs-1
    KITLOutputDebugString("TFTP packet could have %d name/value pairs\r\n",NumberOfNameValuePairs);


    // Make sure we have a server process registered for this file name
    for( ServerProcessPointer = 0; ServerProcessPointer < MAX_TFTP_SERVER_PROCS; ServerProcessPointer++ )
        if (!strcmp( TFtpdServerRegistry[ServerProcessPointer].szFileName, StartOfPacketStringData ))
            break;
    if (ServerProcessPointer == MAX_TFTP_SERVER_PROCS) {
        KITLOutputDebugString( "No Server Process Associated With File Name %s\r\n", (char *)(pwMsg + 1) );
        return 1;
        }
    else {
        // Go ahead and fill out the link record so that I can call TFtpKillLink() if I have too.
        nNumTFtpLinksInUse++;
        TFtpLinks[iLinkSlot].pfCallBack = TFtpdServerRegistry[ServerProcessPointer].pfCallBack;
        strcpy_s( TFtpLinks[iLinkSlot].szFileName, sizeof(TFtpLinks[iLinkSlot].szFileName), StartOfPacketStringData );
        TFtpLinks[iLinkSlot].wBlockNumber = 0;

        TFtpLinks[iLinkSlot].SrcAddr = *pMyAddr;
        TFtpLinks[iLinkSlot].SrcAddr.wPort = GenerateSrcPort();
        SrcAddrFromFrame( &(TFtpLinks[iLinkSlot].DestAddr), pFrameBuffer );
        // gmd - store address in global so we can filter out EDBG commands from
        // other hosts.
        memcpy(&TftpPeerAddr, &(TFtpLinks[iLinkSlot].DestAddr), sizeof(EDBG_ADDR));

        KITLOutputDebugString( "Locked Down Link %d\r\n", iLinkSlot );
        KITLOutputDebugString( "Src IP %s Port %H   ",
            inet_ntoa( TFtpLinks[iLinkSlot].SrcAddr.dwIP ), ntohs(TFtpLinks[iLinkSlot].SrcAddr.wPort));
        KITLOutputDebugString( "Dest IP %s Port %H\r\n",
            inet_ntoa( TFtpLinks[iLinkSlot].DestAddr.dwIP ), ntohs(TFtpLinks[iLinkSlot].DestAddr.wPort) );

        //We will store the block size in the link structure
        TFtpLinks[iLinkSlot].wMaxTftpData = DEFAULT_MAX_TFTP_DATA;
        KITLOutputDebugString( "Default TFTP block size set to: %d bytes\r\n", TFtpLinks[iLinkSlot].wMaxTftpData-4 );
        TFtpLinks[iLinkSlot].UsedBlkSizeOption = FALSE;  //Default to a normal TFTP packet

        // See if this is a read request
        if (ntohs(*((UINT16 *)pwMsg)) == 1) {
            TFtpLinks[iLinkSlot].DataDir = O2HLink;
            TFtpLinks[iLinkSlot].State = TFTP_STATE_XFER_BUSY;
        }
        // Otherwise it should be a write request
        else if (ntohs(*((UINT16 *)pwMsg)) == 2) {
            TFtpLinks[iLinkSlot].DataDir = H2OLink;
            TFtpLinks[iLinkSlot].State = TFTP_STATE_XFER_WAIT;
        }
        else {
            TFtpLinks[iLinkSlot].State = TFTP_STATE_XFER_WAIT;
            TFtpKillLink( iLinkSlot, TFTP_ERROR_ILLEGALOPERATION, "Illegal TFTP Operation - Read/Write Req Expected" );
            return 2;
        }

        // Make sure they are using binary transfer mode
        if (strcmp( "octet", StartOfPacketStringData + strlen(StartOfPacketStringData) + 1 )) {
            TFtpKillLink( iLinkSlot, TFTP_ERROR_ACCESSVIOLATION, "You must access files in binary mode." );
            return 3;
        }

        //This code is where we must pull out any options that exist
        //Only the buffersize option is supported for now
        if( NumberOfNameValuePairs > 1 )
        {
            KITLOutputDebugString("This TFTP packet contains %d options.\r\n",NumberOfNameValuePairs-1);
            KITLOutputDebugString("We currently only look at the first and expect it to be BLKSIZE.\r\n");

            //   Extract option name
            optionName = StartOfPacketStringData + strlen(StartOfPacketStringData) + 1 + 6;
            KITLOutputDebugString("The packet's option name is %s.\r\n",optionName);

            if(!strcmp( optionName, "blksize" ))    //   Ensure option is valid (only supporting BLKSIZE right now)
            {
                //   Extract option value
                optionValue = optionName + strlen(optionName)+1;
                KITLOutputDebugString("The option's value is %s\r\n",optionValue);    //  Determine if option is valid (name is 'blksize')

                BlockSizeValue = atoi(optionValue);                                 //  Get integer version of block size for comparisons

                TFtpLinks[iLinkSlot].UsedBlkSizeOption = TRUE;                  //  Since there was an option that was recognized

                if(BlockSizeValue < 512 || BlockSizeValue > 1024)                   //  Determine if its value is valid (in range 516-1028)
                {                                                                   //  if value is not valid
                    KITLOutputDebugString("with a value that is out of range so sticking with default of");
                }                                                                   //  end if
                else                                                                //  else option is  valid
                {
                    TFtpLinks[iLinkSlot].wMaxTftpData = (UINT16)(BlockSizeValue+4);
                    KITLOutputDebugString("with a value that is in range.\r\nBLKSIZE will be adjusted to");
                }
                KITLOutputDebugString(" %d.\r\n",TFtpLinks[iLinkSlot].wMaxTftpData-4);
            }//   end if
            else
            {
                TFtpKillLink( iLinkSlot, TFTP_ERROR_OPTIONSERROR, "Illegal TFTP Option - An option was detected but it was not the supported BLKSIZE option" );
                return 4;
            }
        }//end if
        else
        {
            KITLOutputDebugString("There were no options detected in the TFTP\r\n");
        }

        // Now we will inform the server process of the new link and send any appropriate data/acknowledge back.
        //  Also, errors can be generated down there that could terminate the link.
        TFtpdCallBack( iLinkSlot, TFTPD_OPEN );
    }

    return 0;
}



void TFtpdCallBack( UINT16 iLinkSlot, TFtpdCallBackOps Operation ) {

    char *pszErrorMsg;
    char *StartOfOptions;

    switch(Operation) {

        case TFTPD_OPEN:
            // Call open to give the callback a chance to initialize
            if (TFtpLinks[iLinkSlot].pfCallBack( TFtpLinks[iLinkSlot].szFileName, TFTPD_OPEN, NULL, NULL, &pszErrorMsg )) {
                TFtpKillLink( iLinkSlot, TFTP_ERROR_FILENOTFOUND, pszErrorMsg );
                return;
            }
            // If it's for a Read request we need to respond with data, recursively call ourselves
            if (TFtpLinks[iLinkSlot].DataDir == O2HLink) {
                TFtpdCallBack( iLinkSlot, TFTPD_READ );
                return;
            }
            // If it's for a Write request, we need to respond with an acknowledge
            // If it was an options enabled packet we should respond with OACK
            else {
                if(TFtpLinks[iLinkSlot].UsedBlkSizeOption == TRUE ) //Should we do OACK?
                {
                    //Respond with an OACK
                    //This OACK is nonstandard as I am including the blocknumber
                    KITLOutputDebugString("An OACK packet is the proper acknowledgement\r\n");
                    StartOfOptions = (char*)((UINT16 *)TFtpLinks[iLinkSlot].Buffer + 2);

                    *((UINT16 *)TFtpLinks[iLinkSlot].Buffer) = htons(TFTP_OPCODE_OACK); //Packet opcode
                    *((UINT16 *)TFtpLinks[iLinkSlot].Buffer + 1) = htons(TFtpLinks[iLinkSlot].wBlockNumber);
                    TFtpLinks[iLinkSlot].cwMsgLen = 4;

                    strcpy_s(StartOfOptions,sizeof(TFtpLinks[iLinkSlot].Buffer) - 2,"blksize");       //Block size option
                    _itoa_s(TFtpLinks[iLinkSlot].wMaxTftpData-4,StartOfOptions+8,sizeof(TFtpLinks[iLinkSlot].Buffer) - 10,10);     //Add the actual size

                    TFtpLinks[iLinkSlot].cwMsgLen += (UINT16)(8 + strlen(StartOfOptions+8) + 1);
                }
                else
                {
                    //Respond with traditional ACK

                    *((UINT16 *)TFtpLinks[iLinkSlot].Buffer) = htons(TFTP_OPCODE_ACK);
                    // This block number should be 0
                    *((UINT16 *)TFtpLinks[iLinkSlot].Buffer + 1) = htons(TFtpLinks[iLinkSlot].wBlockNumber);
                    TFtpLinks[iLinkSlot].cwMsgLen = 4;
                }
            }
            break;
        case TFTPD_READ:
            // We need to send a data packet back
            *((UINT16 *)TFtpLinks[iLinkSlot].Buffer) = htons(TFTP_OPCODE_DATA);
            TFtpLinks[iLinkSlot].wBlockNumber++;
            *((UINT16 *)TFtpLinks[iLinkSlot].Buffer + 1) = htons(TFtpLinks[iLinkSlot].wBlockNumber);
            if (TFtpLinks[iLinkSlot].pfCallBack( TFtpLinks[iLinkSlot].szFileName, TFTPD_READ,
            TFtpLinks[iLinkSlot].Buffer + 4, &(TFtpLinks[iLinkSlot].cwMsgLen), &pszErrorMsg ) ) {
                TFtpKillLink( iLinkSlot, TFTP_ERROR_ACCESSVIOLATION, pszErrorMsg );
                return;
            }
            // Add in length for the control words
            TFtpLinks[iLinkSlot].cwMsgLen += 4;
            TFtpLinks[iLinkSlot].State = TFTP_STATE_XFER_WAIT;
            break;
        case TFTPD_WRITE:
            // We need to process the data and send an acknowledge back
            *((UINT16 *)TFtpLinks[iLinkSlot].Buffer) = htons(TFTP_OPCODE_ACK);
            *((UINT16 *)TFtpLinks[iLinkSlot].Buffer + 1) = htons(TFtpLinks[iLinkSlot].wBlockNumber);
            TFtpLinks[iLinkSlot].cwMsgLen -= 4;
            if (TFtpLinks[iLinkSlot].pfCallBack( TFtpLinks[iLinkSlot].szFileName, TFTPD_WRITE,
            TFtpLinks[iLinkSlot].Buffer + 4, &(TFtpLinks[iLinkSlot].cwMsgLen), &pszErrorMsg ) ) {
                TFtpKillLink( iLinkSlot, TFTP_ERROR_ACCESSVIOLATION, pszErrorMsg );
                return;
            }
            // The acknowledge length is just the control words
            TFtpLinks[iLinkSlot].cwMsgLen = 4;
            TFtpLinks[iLinkSlot].State = TFTP_STATE_XFER_WAIT;
            break;
        case TFTPD_DESTROY:
            // Just tell them that it's dead and return
            TFtpLinks[iLinkSlot].pfCallBack( TFtpLinks[iLinkSlot].szFileName, TFTPD_DESTROY, NULL, NULL, &pszErrorMsg );
            return;

    } // switch(Operation)

    // Send the packet back to the host
    EbootSendUDP(TFTPFrameBuffer, &(TFtpLinks[iLinkSlot].DestAddr), &(TFtpLinks[iLinkSlot].SrcAddr),
                 TFtpLinks[iLinkSlot].Buffer, TFtpLinks[iLinkSlot].cwMsgLen );
    TFtpLinks[iLinkSlot].cwRetries = 0;
    TFtpLinks[iLinkSlot].tLastTransmit = OEMKitlGetSecs();

}
