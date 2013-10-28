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

Module Name:   dhcp.c

Abstract:
    This contains the DHCP protocol IP address resolution
    routines for the bootloader.  It also contains a routine at
    the bottom to allow IP address input via the serial port.
Functions:


Notes:

--*/

#include <windows.h>
#include <nkintr.h>
#include "udp.h"
#include "dhcp.h"
#include "tftp.h"

// This is used to broadcast things to the DHCP servers
static EDBG_ADDR BroadCastAddr = { 0xFFFFFFFFUL, { 0xFFFF, 0xFFFF, 0xFFFF }, DHCP_SERVER_PORT };

// For detecting loops in DHCP
static DWORD dwTimeLastDiscover = 0;

static DWORD dwPrevAddr = 0;

static EDBG_ADDR ServerAddr;
static DWORD dwXID;



// This will start the DHCP process by sending the first DISCOVER packet
UINT16
EbootInitDHCP( EDBG_ADDR *pMyAddr ) {

    // Calling ProcessDHCP() with the initialization parameters will generate the first DISCOVER packet
    KITLOutputDebugString( "InitDHCP():: Calling ProcessDHCP()\r\n" );
    if (EbootProcessDHCP( pMyAddr, NULL, NULL, 0, NULL, NULL)) {
        KITLOutputDebugString( "InitDHCP() Error - First DHCP Option isn't the message type\r\n" );
        return 1;
    }
    return 0;
}



// This is the main routine called when a DHCP packet is received.  It maintains the DHCP
//  state machine and sends appropriate responses to the DHCP hosts when they send packets.
//  When an IP address has been obtained, the flag pointed to by pfwDHCPComplete will be
//  set to true.  The routine can also be called with the pfwDHCPComplete set to NULL to
//  indicate that it should re-initialize and begin the DHCP IP address acquisition process
//  over again.
UINT16 EbootProcessDHCP(
    EDBG_ADDR *pMyAddr,
    DWORD *pSubnetMask,
    BYTE *pbData,
    WORD cwLength,
    DWORD *pLeaseDuration,
    BOOL *pfwDHCPComplete )
{

    static DHCPStates DHCPState;
    DHCPMsgFormat *pDHCPMsg = (DHCPMsgFormat *)pbData;
    BYTE *pbParse;
    DWORD DHCPLease;
    BYTE  DHCPMsgType = 0;

#ifndef DEBUG
    UNREFERENCED_PARAMETER(cwLength);
#endif
    KITL_DEBUGMSG(ZONE_DHCP,("+EbootProcessDHCP: State: %u, pMyAddr: %X, Length: %u\r\n",DHCPState,(ULONG)pMyAddr,cwLength));

    if (pLeaseDuration) {
        *pLeaseDuration = 0;
    }

    // If pfwDHCPComplete == NULL, then we are re-initializing the DHCP process.
    if (pfwDHCPComplete == NULL)
        DHCPState = DHCP_INIT;
    // Otherwise, there's a packet to process
    else {
        *pfwDHCPComplete = FALSE;
        // Make sure that this is a Server to Client BOOTRESPONSE
        if (pDHCPMsg->bOperation != 2)
            return 0;
        // Check for the "Magic Cookie" / sentinel
        if (pDHCPMsg->bOptions[0] != 99 || pDHCPMsg->bOptions[1] != 130 ||
            pDHCPMsg->bOptions[2] != 83 || pDHCPMsg->bOptions[3] != 99) {
            return 0;
        }
        // Check message formating
        else {
            pbParse = DHCPFindOption( DHCP_MSGTYPE, pDHCPMsg );
            if (NULL == pbParse) {
                KITLOutputDebugString( "ProcessDHCP() Error - No DHCP Message Type Option\r\n" );
                return 1;
            }
            DHCPMsgType = pbParse[2];
        }
    }

    switch( DHCPState ) {
        case DHCP_INIT:
            KITLOutputDebugString( "ProcessDHCP()::DHCP_INIT\r\n" );
            // Always try for a new address.
            // (Before, we would request a previously leased IP address on platforms that stored the IP address
            // on the netcard (i.e. ODO). This caused problems when the DHCP servers were switched.)
            pMyAddr->dwIP = 0;
            pMyAddr->wPort = DHCP_CLIENT_PORT;
            if (SendDHCP( 0, DHCP_DISCOVER, &ServerAddr, pMyAddr, &dwXID )) {
                KITLOutputDebugString( "ProcessDHCP()::DHCP_INIT::SendDHCP(DHCP_DISCOVER) Error\r\n" );
                return 1;
            }
            DHCPState = DHCP_SELECTING;
            //  This will indicate to CheckUDP() that it should accept any destination IP address.
            SetPromiscuousIP();
            break;
        case DHCP_SELECTING:
            if (pDHCPMsg->dwXID != dwXID)
                break;
            // We should only accept OFFER messages at this point, and we'll take the first one we get
            if (DHCPMsgType == DHCP_OFFER) {
                pMyAddr->dwIP = pDHCPMsg->dwYIADDR;
                pbParse = DHCPFindOption( DHCP_SERVER_ID, pDHCPMsg );
                if (pbParse == NULL) {
                    KITLOutputDebugString( "ProcessDHCP()::DHCP_SELECTING::DHCPFindOption() Got DHCP_OFFER without DHCP_SERVER_ID\r\n" );
                    return 1;
                }
                // Pull out the server IP address, remembering to skip over option code and length
                // I can't just retrieve a DWORD pointer from the buffer address because it might not be 4
                //  byte aligned.
                ServerAddr.dwIP =    ((DWORD)(pbParse[2]))        | (((DWORD)(pbParse[3])) <<  8) |
                                    (((DWORD)(pbParse[4])) << 16) | (((DWORD)(pbParse[5])) << 24);

                // Get our subnet mask
                pbParse = DHCPFindOption( DHCP_SUBNET_MASK, pDHCPMsg );
                if (pbParse == NULL) {
                    KITLOutputDebugString( "ProcessDHCP()::DHCP_SELECTING::DHCPFindOption() Got DHCP_OFFER without DHCP_SUBNET_MASK\r\n" );

                                        // We didn't get a subnet mask from the DHCP server - choose a good default.
                                        //
                                        *pSubnetMask = ntohl(0xffff0000);

                    return 1;
                }
                *pSubnetMask = ((DWORD)(pbParse[2]))          | (((DWORD)(pbParse[3])) <<  8) |
                                (((DWORD)(pbParse[4])) << 16) | (((DWORD)(pbParse[5])) << 24);

                if (SendDHCP( 0, DHCP_REQUEST, &ServerAddr, pMyAddr, &dwXID )) {
                    KITLOutputDebugString( "ProcessDHCP()::DHCP_SELECTING::SendDHCP() Error\r\n" );
                    return 1;
                }
                DHCPState = DHCP_REQUESTING;
            }
            // If the server says to discard the offer, start over
            else if (DHCPMsgType == DHCP_NAK) {
                DHCPState = DHCP_INIT;
                pMyAddr->dwIP = 0;
                dwTimeLastDiscover = 0; // Reset loop detection
                return EbootInitDHCP( pMyAddr );
            }
            break;
        case DHCP_REQUESTING:
            if (pDHCPMsg->dwXID != dwXID)
                break;
            if (DHCPMsgType == DHCP_ACK) {
                // This will tell CheckUDP() to only accept datagrams for our IP address
                ClearPromiscuousIP();

                // Signal that we've got a valid IP address
                *pfwDHCPComplete = TRUE;
                DHCPState = DHCP_BOUND;
                pbParse = DHCPFindOption(DHCP_LEASE_TIME, pDHCPMsg);
                if (pbParse == NULL) {
                    KITLOutputDebugString( "ProcessDHCP()::DHCP_REQUESTING::DHCPFindOption() Got DHCP_ACK without DHCP_LEASE_TIME\r\n" );
                    return 1;
                }
                DHCPLease =  ((DWORD)(pbParse[5]))        | (((DWORD)(pbParse[4])) <<  8) |
                            (((DWORD)(pbParse[3])) << 16) | (((DWORD)(pbParse[2])) << 24);
                if (pLeaseDuration) {
                    *pLeaseDuration = DHCPLease;
                }
                KITLOutputDebugString( "\r\nProcessDHCP()::DHCP IP Address Resolved as %s, ",
                                    inet_ntoa(pMyAddr->dwIP));
                KITLOutputDebugString("netmask: %s\r\n",inet_ntoa(*pSubnetMask));
                KITLOutputDebugString("Lease time: %d seconds\r\n",DHCPLease);

            }
            // If the server says to discard the offer, start over
            else if (DHCPMsgType == DHCP_NAK) {
                DHCPState = DHCP_INIT;
                pMyAddr->dwIP = 0;
                dwTimeLastDiscover = 0; // Reset loop detection
                return EbootInitDHCP( pMyAddr );
            }
            break;
        case DHCP_BOUND:
            break;

    } // switch( DHCPState )

    return 0;

} // ProcessDHCP



// This routine will form the DHCP messages that we have to send and send them out.  There are three different
//  messages that need to be sent: DHCP_DISCOVER and two types of DHCP_REQUEST.  If the fRequestLastIP flag is
//  set, then we are doing a request for a IP address that we've already got.  If fRequestLastIP is not true,
//  then we are requesting an IP address that we've just been OFFERed.
UINT16 SendDHCP( BYTE fRequestLastIP, DHCPMsgTypes MsgType, EDBG_ADDR *pServerAddr, EDBG_ADDR *pMyAddr, DWORD *dwXID ) {
    UCHAR FrameBuf[UDP_DATA_FRAME_OFFSET + sizeof(DHCPMsgFormat)];
    DHCPMsgFormat *pDHCPMsg = (DHCPMsgFormat *)(FrameBuf + UDP_DATA_FRAME_OFFSET);
    EDBG_ADDR ClientAddr;
    WORD wOpOff;

    KITL_DEBUGMSG(ZONE_DHCP,("+SendDHCP, MsgType: %u", MsgType));

    ClientAddr = *pMyAddr;

    // Start out by zeroing out the whole thing because most of it is supposed to be zero anyway
    memset(pDHCPMsg, 0, sizeof(DHCPMsgFormat));

    pDHCPMsg->bOperation = 1;                   // Client to Server messages are BOOTREQUEST
    pDHCPMsg->bHardwareAddrType = 1;            // Hardware type is 10Mbps Ethernet
    pDHCPMsg->bHardwareAddrLen = 6;         // Hardware address length
    // Fill in our MAC address
    pDHCPMsg->wCHADDR[0] = pMyAddr->wMAC[0];
    pDHCPMsg->wCHADDR[1] = pMyAddr->wMAC[1];
    pDHCPMsg->wCHADDR[2] = pMyAddr->wMAC[2];
    pDHCPMsg->wSecs = (WORD)OEMKitlGetSecs();       // Number of seconds elapsed since last reset

    // The flags field is zero.  Note that since the broadcast bit isn't set
    //  here, we must accept packets using our new IP address before we
    //  do the final confirmation.

    // Now fill in the option fields
    wOpOff = 0;

    // Fill in the "Magic Cookie"
    pDHCPMsg->bOptions[wOpOff++] = 99;      pDHCPMsg->bOptions[wOpOff++] = 130;
    pDHCPMsg->bOptions[wOpOff++] = 83;      pDHCPMsg->bOptions[wOpOff++] = 99;

    // Put in the message type field
    pDHCPMsg->bOptions[wOpOff++] = DHCP_MSGTYPE;    // This is the DHCP message type field
    pDHCPMsg->bOptions[wOpOff++] = 1;               // Length of 1 byte
    pDHCPMsg->bOptions[wOpOff++] = MsgType;         // Set the message type

    switch( MsgType ) {

        case DHCP_DISCOVER:
            dwTimeLastDiscover = OEMKitlGetSecs();

            // This is supposed to be a random number used to identify the link.
            //  This number should be more or less unique.
            *dwXID = pDHCPMsg->dwXID = (((DWORD)pDHCPMsg->wCHADDR[2]) << 16) | GenerateSrcPort();

            // Specify our host name
            DHCPBuildOps( DHCP_HOSTNAME, pDHCPMsg, &wOpOff, (DWORD)pMyAddr->wMAC);
            if (ClientAddr.dwIP)
                DHCPBuildOps( DHCP_IP_ADDR_REQ, pDHCPMsg, &wOpOff, ClientAddr.dwIP );
            DHCPBuildOps( DHCP_CLIENT_ID, pDHCPMsg, &wOpOff, (DWORD)pMyAddr->wMAC);
            // Indicate end of options
            DHCPBuildOps( DHCP_END, pDHCPMsg, &wOpOff, 0 );
            // The source IP for the DISCOVER is supposed to be 0;
            ClientAddr.dwIP = 0;
            break;

        case DHCP_REQUEST:
            // Specify our host name
            DHCPBuildOps( DHCP_HOSTNAME, pDHCPMsg, &wOpOff, (DWORD)pMyAddr->wMAC);
            // Request the new or old IP address
            DHCPBuildOps( DHCP_IP_ADDR_REQ, pDHCPMsg, &wOpOff, ClientAddr.dwIP );
            if (fRequestLastIP) {
                //  This number should be more or less unique.
                *dwXID = pDHCPMsg->dwXID = (((DWORD)pDHCPMsg->wCHADDR[2]) << 16) | GenerateSrcPort();
                pDHCPMsg->dwCIADDR = pMyAddr->dwIP;
            }
            else {
                pDHCPMsg->dwXID = *dwXID;
                DHCPBuildOps( DHCP_SERVER_ID, pDHCPMsg, &wOpOff, pServerAddr->dwIP );
            }
            DHCPBuildOps( DHCP_CLIENT_ID, pDHCPMsg, &wOpOff, (DWORD)pMyAddr->wMAC);
            // Indicate end of options
            DHCPBuildOps( DHCP_END, pDHCPMsg, &wOpOff, 0 );
            ClientAddr.dwIP = 0;
            break;

        case DHCP_DECLINE:
            DHCPBuildOps( DHCP_IP_ADDR_REQ, pDHCPMsg, &wOpOff, pMyAddr->dwIP);
            // fallthrough to DHCP_RELEASE case

        case DHCP_RELEASE:
            *dwXID = pDHCPMsg->dwXID = (((DWORD)pDHCPMsg->wCHADDR[2]) << 16) | GenerateSrcPort();
            pDHCPMsg->dwCIADDR = pMyAddr->dwIP;
            DHCPBuildOps( DHCP_CLIENT_ID, pDHCPMsg, &wOpOff, (DWORD)pMyAddr->wMAC);
            DHCPBuildOps( DHCP_SERVER_ID, pDHCPMsg, &wOpOff, pServerAddr->dwIP );
            DHCPBuildOps( DHCP_END, pDHCPMsg, &wOpOff, 0 );
            break;
    } // switch( MsgType )


    if (MsgType == DHCP_DECLINE) {
        if (!EbootSendUDP(FrameBuf, &BroadCastAddr, pMyAddr, (BYTE *)pDHCPMsg, sizeof(DHCPMsgFormat))) {
            KITLOutputDebugString( "SendDHCP()::Error On SendUDP() Call\r\n" );
            return 1;
        }
        return 0;
    }


    // We need to broadcast these messages to all the DHCP servers.
    if (EbootDHCPRetransmit( pMyAddr, &ClientAddr, FrameBuf)) {
        KITLOutputDebugString( "SendDHCP()::Error On DHCPRetransmit() Call\r\n" );
        return 1;
    }

    return 0;

} // SendDHCP()


static BYTE HexToChar(BYTE hex)
{
    switch (hex) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        return '0' + hex;

    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        return 'A' + hex - 10;
    }
    return 'X';
}

// Generate a string which is the concatenation of "CED" and the ethernet address
// ("CED" denotes the debug ethernet component of a CE device.)
static void FormatDHCPName(BYTE * pName, BYTE * pEthAddr)
{
    DWORD j;
    BYTE * pN;
    BYTE * pA;
    BYTE A;

    pName[0] = 'C';
    pName[1] = 'E';
    pName[2] = 'D';

    pN = pName + 3;
    pA = pEthAddr;

    for (j = 0; j < 6; j++, pA++, pN += 2) {
        A = *pA;
        pN[0] = HexToChar((BYTE)(A >> 4));
        pN[1] = HexToChar((BYTE)(A & 0x0f));
    }
    *pN = 0;
}   // FormatDHCPName


// This routine will add options to the DHCP Options field.  The option type is filled in
//  and the Option Offset (pwOpOff) is incremented to include the new option.  Some of the
//  options can be taken from the DHCP header itself, data for others is passed in through
//  the generic dwData.
void DHCPBuildOps( DHCPOptions DHCPOption, DHCPMsgFormat *pDHCPMsg, WORD *pwOpOff, DWORD dwData )
{
    BYTE * pName;

    switch( DHCPOption ) {
    case DHCP_SERVER_ID:
    case DHCP_IP_ADDR_REQ:
    case DHCP_HOSTNAME:
    case DHCP_CLIENT_ID:
    case DHCP_END:
        pDHCPMsg->bOptions[(*pwOpOff)++] = DHCPOption;
        break;
    } // switch

    switch( DHCPOption ) {
    case DHCP_SERVER_ID:
    case DHCP_IP_ADDR_REQ:
        // There are 4 data bytes
        pDHCPMsg->bOptions[(*pwOpOff)++] = 4;
        // IP address that we'd like to have
        pDHCPMsg->bOptions[(*pwOpOff)++] = (BYTE)dwData;
        pDHCPMsg->bOptions[(*pwOpOff)++] = (BYTE)(dwData >> 8);
        pDHCPMsg->bOptions[(*pwOpOff)++] = (BYTE)(dwData >> 16);
        pDHCPMsg->bOptions[(*pwOpOff)++] = (BYTE)(dwData >> 24);
        break;

    case DHCP_HOSTNAME:
        // The ethernet address is passed in dwData
        pName = &(pDHCPMsg->bOptions[*pwOpOff+1]);
        FormatDHCPName(pName, (BYTE *)dwData);

        // Fill out the host name length
        (*pwOpOff) += pDHCPMsg->bOptions[*pwOpOff] = (BYTE)(strlen((const char *)pName) + 1);
        (*pwOpOff)++;
        break;

    case DHCP_CLIENT_ID:
        pDHCPMsg->bOptions[(*pwOpOff)++] = 7;   // 1 byte of hw addr type and 6 bytes of ethernet addr
        pDHCPMsg->bOptions[(*pwOpOff)++] = 1;   // ethernet hw addr type
        memcpy(&(pDHCPMsg->bOptions[*pwOpOff]), (BYTE *) dwData, 6);  // ethernet addr
        *pwOpOff += 6;
        break;
    } // switch

} // DHCPBuildOps()



// This routine will parse through the DHCP message looking for the specified option.
//  Note that the options can extend beyond the Options field into the file and sname
//  fields.  They are done in a somewhat non-obvious order so that the Options field doesn't
//  simply spill over into the others.  Further, there is a defined parsing order that
//  doesn't make much sense.
BYTE *DHCPFindOption( DHCPOptions DHCPOption, DHCPMsgFormat *pDHCPMsg ) {

    BYTE *pbParse;
    BYTE fUseSname = 0;
    BYTE fUseFile = 0;

    // First determine if the file or sname fields have been used to include addtional options.
    // If they have, a DHCP_OPTION_OVERLOAD will appear in the normal Options field
    // Don't forget to skip over Magic Cookie in Options field
    pbParse = DHCPParseField( DHCP_OPTION_OVERLOAD, pDHCPMsg->bOptions + 4 );
    if (pbParse != NULL) {
        fUseFile = (*(pbParse + 2)) & 1;
        fUseSname = (*(pbParse + 2)) & 2;
    }

    // Now look for the option that we were called for, in the order specified by RFC1541
    // Don't forget to skip over Magic Cookie in Options field
    pbParse = DHCPParseField( DHCPOption, pDHCPMsg->bOptions + 4);
    if (pbParse != NULL)
        return pbParse;
    if (fUseFile) {
        pbParse = DHCPParseField( DHCPOption, (BYTE *)pDHCPMsg->szFILE );
        if (pbParse != NULL)
            return pbParse;
    }
    if (fUseSname)
        pbParse = DHCPParseField( DHCPOption, (BYTE *)pDHCPMsg->szSNAME );

    return pbParse;

} // DHCPFindOption()



// This routine will look for a DHCP option in the field that starts at the
//  location that pbParse points too.  Note that all DHCP fields MUST end in
//  a DHCP_END option, so that this search won't run away.
BYTE *DHCPParseField( DHCPOptions DHCPOption, BYTE *pbParse ) {

    while( *pbParse != DHCP_END ) {
        if (*pbParse == DHCPOption)
            return pbParse;
        pbParse += *(pbParse + 1) + 2;
    } // for every option code in this field

    return NULL;

} // DHCPParseField



// This routine is called in two instances.  When the SendDHCP() routine forms a packet, this
//  routine is called with the source address and the message to be sent.  That initilizes the
//  retry timers and causes the routine to save the message.  Then the routine is called
//  by the main Ethernet() loop so that the DHCP message can be retransmit as necessary.
// If there are too many retry attempts without a reply, the routine will call ProcessDHCP()
//  with the initialize flag set, which will cause the DHCP process to begin again.
// Note that the pMyAddr pointer must be valid on every call.
UINT16 EbootDHCPRetransmit( EDBG_ADDR *pMyAddr, EDBG_ADDR *pSrcAddr, BYTE *pFrame) {
    DHCPMsgFormat *pDHCPMsg;
    // These globals are used to store all the transmission information for the last DHCP packet
    //  that was sent.  The DHCPRetransmit() routine uses this info to do retries.
    static EDBG_ADDR SrcAddr;
    static UCHAR LastDHCPFrame[UDP_DATA_FRAME_OFFSET+sizeof(DHCPMsgFormat)];
    static WORD cRetries;
    static DWORD dwTimeOfLastRetry;

    // If this is the first call, then initialize everything and send the packet.
    if (pSrcAddr != NULL) {
        cRetries = 0;
        dwTimeOfLastRetry = OEMKitlGetSecs();
        KITL_DEBUGMSG(ZONE_DHCP,("+DHCPRetransmit: 1st transmission, time: %u\r\n",dwTimeOfLastRetry));
        pDHCPMsg = (DHCPMsgFormat *)(pFrame+UDP_DATA_FRAME_OFFSET);
        // Save last DHCP message sent, and source address
        memcpy(LastDHCPFrame+UDP_DATA_FRAME_OFFSET, (BYTE *)pDHCPMsg, sizeof(DHCPMsgFormat));
        SrcAddr = *pSrcAddr;
    }
    // If this is a retransmit call and we haven't exceeded the retry count, send it again
    // It's supposed to be a doubling backoff from 4 seconds to 64
    else if (OEMKitlGetSecs() - dwTimeOfLastRetry > (4UL << cRetries)) {
        dwTimeOfLastRetry = OEMKitlGetSecs();
        cRetries++;
        KITL_DEBUGMSG(ZONE_DHCP,("+DHCPRetransmit: Retry %u, time: %u\r\n",cRetries,dwTimeOfLastRetry));

        // If we've reached the maximum retry count (64 seconds), re-initialize the DHCP handler
        if (cRetries >= 5) {
            KITL_DEBUGMSG(ZONE_DHCP,("DHCPRetransmit, retry count exceeded, reinitializing...\r\n"));
            pMyAddr->dwIP = 0;
            return EbootInitDHCP( pMyAddr );
        }
        // Retransmit saved frame
        pFrame   = LastDHCPFrame;
        pDHCPMsg = (DHCPMsgFormat *)(pFrame+UDP_DATA_FRAME_OFFSET);
    }
    else
        return 0;

    if (!EbootSendUDP( pFrame, &BroadCastAddr, &SrcAddr, (BYTE *)pDHCPMsg, sizeof(DHCPMsgFormat))) {
        KITLOutputDebugString( "EbootDHCPRetransmit()::Error On SendUDP() Call\r\n" );
        return 1;
    }

    return 0;
}   // EbootDHCPRetransmit


//
// Send a DHCP DECLINE message and wait 10 seconds
//
BOOL
EbootDHCPDecline( EDBG_ADDR *pMyAddr) {
    BOOL bRet;
    DWORD dwSentTime;

    if (dwPrevAddr) {
        if (dwPrevAddr == pMyAddr->dwIP) {
            //
            // Server keeps sending us the same IP addr!
            // The bootloader code will continue the same retry algorithm as for ARP collisions
            //
        }
    } else {
        dwPrevAddr = pMyAddr->dwIP;
    }

    dwSentTime = OEMKitlGetSecs();
    bRet = SendDHCP(0, DHCP_DECLINE, &ServerAddr, pMyAddr, &dwXID ) ? FALSE : TRUE;
    if (bRet) {
        while (OEMKitlGetSecs() - dwSentTime < 10) {
        }
    }
    return bRet;
}   // EbootDHCPDecline


// This routine is called repeatedly during the DHCP IP acquisition process to scan for serial
//  input from the user.  It first sends a message out the serial port asking for an IP
//  address.  If the user enters anything, the DHCP process will be interrupted and the user
//  will be allowed to enter an IP address by hand.  The user's IP address will be returned
//  in pMyAddr and the routine will return non-zero if this happens.
BOOL
EbootReadSerialIP( EDBG_ADDR *pMyAddr, DWORD *pSubnetMask) {

    static char szDottedD[16];  // The string used to collect the dotted decimal IP address
    static WORD cwNumChars = 0;
    static BOOL fIPEntered = FALSE;

    UINT16 InChar;

    InChar = (UINT16)OEMReadDebugByte();
    if (InChar != OEM_DEBUG_COM_ERROR && InChar != OEM_DEBUG_READ_NODATA) {
        // If it's a number or a period, add it to the string
        if (InChar == '.' || (InChar >= '0' && InChar <= '9')) {
            if (cwNumChars < 16) {
                szDottedD[cwNumChars++] = (char)InChar;
                OEMWriteDebugByte((BYTE)InChar);
            }
        }
        // If it's a backspace, back up
        else if (InChar == 8) {
            if (cwNumChars > 0) {
                cwNumChars--;
                OEMWriteDebugByte((BYTE)InChar);
            }
        }
        // If it's a carriage return or line feed, send it back
        else if ((InChar == 0x0d) || (InChar == 0x0a)) {
            if (fIPEntered) {
                if (cwNumChars) {
                    szDottedD[cwNumChars] = '\0';
                    *pSubnetMask = inet_addr( szDottedD );
                }

                KITLOutputDebugString( "\r\nReadSerialIP()::Using IP Address %s, ",
                                    inet_ntoa(pMyAddr->dwIP));
                KITLOutputDebugString("netmask: %s\r\n", inet_ntoa(*pSubnetMask));

                // Now we've got our address, allow timeouts again
                return TRUE;
            }
            else {
                // If it's a carriage return with an empty string, use the default pMyAddr
                if (cwNumChars) {
                    szDottedD[cwNumChars] = '\0';
                    pMyAddr->dwIP = inet_addr( szDottedD );
                }
                fIPEntered = TRUE;
                KITLOutputDebugString("\r\nEnter new subnet mask or CR to use existing mask: ");
                cwNumChars = 0;

                // This will tell CheckUDP() to only accept datagrams for our IP address
                ClearPromiscuousIP();
            }
        }
    }

    return FALSE;

} // ReadSerialIP()

