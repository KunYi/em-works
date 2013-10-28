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

Module Name:    udp.c

Abstract:       This contains the UDP/IP and ARP routines for the
                bootloader.  This module operates on top of the
                pktdrv.c module.

Functions:


Notes:

--*/


// Include Files

#include <windows.h>
#include "udp.h"


// Local Variables

static  BYTE fPromiscuousIP;


// External Functions (header file?)

BOOL EbootDHCPDecline( EDBG_ADDR *pMyAddr); // dhcp.c


// Static Functions

static  void    DuplicateIPWarning( DWORD dwIP, BYTE * chMAC );
static  DWORD   UpperDWFromMAC( BYTE *pAddr );
static  UINT16  CRC( UINT16 *pwRegion1,
                     UINT16  wLength1,
                     UINT16 *pwRegion2,
                     UINT16  wLength2 );

//------------------------------------------------------------------------------
//
//  Function Name:  SrcAddrFromFrame( EDBG_ADDR *pAddr, BYTE *pFrameBuffer )
//  Description..:  This routine will set an EDBG_ADDR to point to the source
//                  of this IP frame
//  Inputs.......:  EDBG_ADDR       pntr to address
//                  BYTE *          pntr to frame buffer
//  Outputs......:  none
//
//------------------------------------------------------------------------------

void SrcAddrFromFrame( EDBG_ADDR *pAddr, BYTE *pFrameBuffer )
{
    EthernetFrameHeader *pFrameHeader;
    IPHeaderFormat      *pIPHeader;
    UDPHeaderFormat     *pUDPHeader;

    // Replace the old host information

    pFrameHeader = (EthernetFrameHeader *)pFrameBuffer;
    pAddr->wMAC[0] = pFrameHeader->wSrcMAC[0];
    pAddr->wMAC[1] = pFrameHeader->wSrcMAC[1];
    pAddr->wMAC[2] = pFrameHeader->wSrcMAC[2];
    pIPHeader = (IPHeaderFormat *)(pFrameBuffer + sizeof(EthernetFrameHeader));
    pAddr->dwIP = pIPHeader->dwSrcIP;
    pUDPHeader =
        (UDPHeaderFormat *)( (BYTE *)pIPHeader + sizeof(IPHeaderFormat) );
    pAddr->wPort = pUDPHeader->wSrcPort;
}

//------------------------------------------------------------------------------
//
//  Function Name:  EbootProcessARP( EDBG_ADDR *pMyAddr, BYTE *pFrameBuffer )
//  Description..:  This routine is called in response to receiving an
//                  ARP packet.  If a request is received, we send back a
//                  response and update the HostAddress to point to the
//                  machine that requested it. If a response is received, then
//                  most likely some other station thinks it owns our IP address.
//
//                  CAUTION: This routine WILL reformat the original frame into
//                  the format of the ARP response.
//
//  Inputs.......:  EDBG_ADDR *     pntr to address
//                  BYTE *          pntr to frame buffer
//  Outputs......:  ARP request status
//
//------------------------------------------------------------------------------

UINT16 EbootProcessARP( EDBG_ADDR *pMyAddr, BYTE *pFrameBuffer )
{
    EthernetFrameHeader *pFrameHeader;
    ARPPacketFormat     *pARP;

    // Access ARP packet

    pARP = (ARPPacketFormat *)(pFrameBuffer + sizeof(EthernetFrameHeader));

    // Check to see that they were requesting the ARP response from us

    if( pARP->dwDestIP != pMyAddr->dwIP )
    {
        return( PROCESS_ARP_IGNORE );
    }

    // Make sure this is an ARP request

    switch( htons(pARP->wOperation) )
    {
    case 1: // ARP request

        break;

    case 2: // ARP response

        // An ARP response destined for our station is NOT expected.
        // We expect to have a unique IP and we now need to re-issue
        // a DHCP request.

        DuplicateIPWarning( pARP->dwSrcIP, (BYTE *)pARP->wSrcMAC );
        EbootDHCPDecline( pMyAddr );
        return( PROCESS_ARP_RESPONSE );

    case 3: // RARP request
    case 4: // RARP response
    default:

        return( PROCESS_ARP_IGNORE );
    }

    // Fill in destination and source MAC addresses and the ARP frame type

    pFrameHeader = (EthernetFrameHeader *)pFrameBuffer;
    pFrameHeader->wDestMAC[0] = pARP->wSrcMAC[0];
    pFrameHeader->wDestMAC[1] = pARP->wSrcMAC[1];
    pFrameHeader->wDestMAC[2] = pARP->wSrcMAC[2];
    pFrameHeader->wSrcMAC[0]  = pMyAddr->wMAC[0];
    pFrameHeader->wSrcMAC[1]  = pMyAddr->wMAC[1];
    pFrameHeader->wSrcMAC[2]  = pMyAddr->wMAC[2];
    pFrameHeader->wFrameType  = htons(0x0806);

    // The field information comes from page 57 of
    // TCP/IP Illustrated by W. Richard Stevens.

    pARP->wHardwareType = htons(1);         // Ethernet
    pARP->wProtocolType = htons(0x0800);    // IP addresses are being mapped
    pARP->bHardwareAddrSize = 6;            // MAC addresses are 6 bytes long
    pARP->bProtocolAddrSize = 4;            // IP addresses are 4 bytes long
    pARP->wOperation = htons(2);            // Specify an ARP reply

    // Fill in the destination information

    pARP->wDestMAC[0] = pARP->wSrcMAC[0];
    pARP->wDestMAC[1] = pARP->wSrcMAC[1];
    pARP->wDestMAC[2] = pARP->wSrcMAC[2];
    pARP->dwDestIP    = pARP->dwSrcIP;

    // Fill in the source side information

    pARP->dwSrcIP    = pMyAddr->dwIP;
    pARP->wSrcMAC[0] = pMyAddr->wMAC[0];
    pARP->wSrcMAC[1] = pMyAddr->wMAC[1];
    pARP->wSrcMAC[2] = pMyAddr->wMAC[2];


    // Send the frame

    if( OEMEthSendFrame( pFrameBuffer,
        sizeof(EthernetFrameHeader) + sizeof(ARPPacketFormat)) )
    {
        return PROCESS_ARP_REQUEST;
    }
    else
    {
        return PROCESS_ARP_REQUEST_ERR;
    }
}

//------------------------------------------------------------------------------
//
//  Function Name:  EbootGratuitousARP( EDBG_ADDR *pMyAddr, BYTE *pFrameBuffer )
//  Description..:  This routine is called to verify that the station's IP
//                  address is unique on the net.  A gratuitous ARP is an ARP
//                  request for our own IP address. If there is a response,
//                  then some other station thinks it still owns our IP address.
//
//                  CAUTION: This routine WILL reformat the original frame.
//
//  Inputs.......:  EDBG_ADDR *     pntr to address
//                  BYTE *          pntr to frame buffer
//  Outputs......:  TRUE on success, else FALSE
//
//------------------------------------------------------------------------------

UINT16 EbootGratuitousARP( EDBG_ADDR *pMyAddr, BYTE *pFrameBuffer )
{
    EthernetFrameHeader *pFrameHeader;
    ARPPacketFormat *pARP;

    // Access the ARP packet

    pARP = (ARPPacketFormat *)(pFrameBuffer + sizeof(EthernetFrameHeader));

    // Fill in destination and source MAC addresses and the ARP frame type

    pFrameHeader = (EthernetFrameHeader *)pFrameBuffer;
    pFrameHeader->wDestMAC[0] = 0xFFFF;
    pFrameHeader->wDestMAC[1] = 0xFFFF;
    pFrameHeader->wDestMAC[2] = 0xFFFF;
    pFrameHeader->wSrcMAC[0] = pMyAddr->wMAC[0];
    pFrameHeader->wSrcMAC[1] = pMyAddr->wMAC[1];
    pFrameHeader->wSrcMAC[2] = pMyAddr->wMAC[2];
    pFrameHeader->wFrameType = htons(0x0806);

    // The field information comes from page 57 of TCP/IP
    // Illustrated by W. Richard Stevens.

    pARP->wHardwareType = htons(1);         // Ethernet
    pARP->wProtocolType = htons(0x0800);    // IP addresses are being mapped
    pARP->bHardwareAddrSize = 6;            // MAC addresses are 6 bytes long
    pARP->bProtocolAddrSize = 4;            // IP addresses are 4 bytes long
    pARP->wOperation = htons(1);            // Specify an ARP request

    // Fill in the destination information

    pARP->wDestMAC[0] = pMyAddr->wMAC[0];
    pARP->wDestMAC[1] = pMyAddr->wMAC[1];
    pARP->wDestMAC[2] = pMyAddr->wMAC[2];
    pARP->dwDestIP    = pMyAddr->dwIP;

    // Fill in the source side information

    pARP->dwSrcIP    = pMyAddr->dwIP;
    pARP->wSrcMAC[0] = pMyAddr->wMAC[0];
    pARP->wSrcMAC[1] = pMyAddr->wMAC[1];
    pARP->wSrcMAC[2] = pMyAddr->wMAC[2];

    // Send the frame

    if( OEMEthSendFrame( pFrameBuffer,
        sizeof(EthernetFrameHeader) + sizeof(ARPPacketFormat)) )
    {
        return( 0 );
    }
    else
    {
        return( 1 );
    }
}

//------------------------------------------------------------------------------
//
//  Function Name:  EbootSendUDP(...)
//  Description..:  Format Ethernet, IP, and UDP headers into supplied frame
//                  buffer, and send to peer.  Pointer to frame buffer to hold
//                  Ethernet frame must be at least 42 bytes + length of UDP
//                  data. Pointer to UDP data. Can, but doesn't have to, be
//                  within the frame buffer.
//  Inputs.......:  BYTE *          pntr to frame
//                  EDBG_ADDR       pntr to dst addr
//                  EDBG_ADDR       pntr to src addr
//                  BYTE *          pntr to UDP data
//                  UINT16          data length
//  Outputs......:  TRUE on success, else FALSE
//
//------------------------------------------------------------------------------

BOOL EbootSendUDP( BYTE        *pFrameBuffer,
                   EDBG_ADDR   *pDestAddr,
                   EDBG_ADDR   *pSrcAddr,
                   BYTE        *pUDPData,
                   UINT16       cwLength )
{
    EthernetFrameHeader     *pFrameHeader;
    IPHeaderFormat          *pIPHeader;
    UDPPseudoHeaderFormat    UDPPseudoHeader;
    UDPHeaderFormat         *pUDPHeader;

    static UINT16 wIndentification = 0;

    // Fill in destination and source MAC addresses and the IP frame type

    pFrameHeader = (EthernetFrameHeader *)pFrameBuffer;
    pFrameHeader->wDestMAC[0] = pDestAddr->wMAC[0];
    pFrameHeader->wDestMAC[1] = pDestAddr->wMAC[1];
    pFrameHeader->wDestMAC[2] = pDestAddr->wMAC[2];
    pFrameHeader->wSrcMAC[0]  = pSrcAddr->wMAC[0];
    pFrameHeader->wSrcMAC[1]  = pSrcAddr->wMAC[1];
    pFrameHeader->wSrcMAC[2]  = pSrcAddr->wMAC[2];
    pFrameHeader->wFrameType  = htons(0x0800);

    // The field information comes from page 34 of
    // TCP/IP Illustrated by W. Richard Stevens.

    pIPHeader = (IPHeaderFormat *)(pFrameBuffer + sizeof(EthernetFrameHeader));
    pIPHeader->bVersionLength = 0x45;   // IPv4, No options, 5 longs
    pIPHeader->bTypeOfService = 0;      // no special transport options

    // Total length of the IP datagram

    pIPHeader->cwTotalLength = htons(sizeof(IPHeaderFormat) +
                                   sizeof(UDPHeaderFormat) +
                                   cwLength);

    // Unique packet identification

    pIPHeader->wIdentification = wIndentification++;
    pIPHeader->wFragment = 0;           // no fragmentation
    pIPHeader->bTimeToLive = (char)64;  // we can go through 64 routers
    pIPHeader->bProtocol = (char)17;    // UDP protocol is 17
    pIPHeader->wCRC = 0;                // CRC is 0 until computed
    pIPHeader->dwSrcIP = pSrcAddr->dwIP;
    pIPHeader->dwDestIP = pDestAddr->dwIP;

    // Compute the CRC and we're done with the IP header

    pIPHeader->wCRC = CRC((UINT16 *)pIPHeader, sizeof(IPHeaderFormat), NULL, 0);

    // The field information comes from page 145 of
    // TCP/IP Illustrated by W. Richard Stevens.
    // Construct the UDP pseudo header

    UDPPseudoHeader.dwSrcIP = pSrcAddr->dwIP;
    UDPPseudoHeader.dwDestIP = pDestAddr->dwIP;
    UDPPseudoHeader.bZero = 0;
    UDPPseudoHeader.bProtocol = 17;     // UDP protocol is 17
    UDPPseudoHeader.cwTotalUDPLength = htons(sizeof(UDPHeaderFormat) +
                                           cwLength);
    // Construct the real UDP header

    pUDPHeader = (UDPHeaderFormat *)((BYTE *)pIPHeader+sizeof(IPHeaderFormat));
    pUDPHeader->wSrcPort  = pSrcAddr->wPort;
    pUDPHeader->wDestPort = pDestAddr->wPort;

    // The total length of the UDP datagram

    pUDPHeader->cwTotalUDPLength = UDPPseudoHeader.cwTotalUDPLength;
    pUDPHeader->wCRC = 0;               // CRC is 0 until computed

    // Copy user data if necessary

    if( (pUDPData - pFrameBuffer) != UDP_DATA_FRAME_OFFSET )
    {
        memcpy( (BYTE *)pUDPHeader + sizeof(UDPHeaderFormat),
            pUDPData,
            cwLength);
    }

    // Compute the CRC for the UDP datagram.  The pad 0 will
    // automatically be added if there are an odd number of bytes.

    pUDPHeader->wCRC = CRC( (UINT16 *)(&UDPPseudoHeader),
                            sizeof(UDPPseudoHeader),
                            (UINT16 *)pUDPHeader,
                            ntohs(pUDPHeader->cwTotalUDPLength) );

    if( pUDPHeader->wCRC == 0 )
    {
        pUDPHeader->wCRC = ~(pUDPHeader->wCRC);
    }

    // Indicate status of frame transmit

    return( OEMEthSendFrame( pFrameBuffer,
            sizeof( EthernetFrameHeader ) +
            ntohs( pIPHeader->cwTotalLength ) ) );
}

//------------------------------------------------------------------------------
//
//  Function Name:  SetPromiscuousIP( void )
//  Description..:  This functions sets the global PromiscuousIP flag. The flag
//                  is used to indicate when the CheckUDP routine should allow
//                  packets through the filtering system (set).
//  Inputs.......:  none
//  Outputs......:  none
//
//------------------------------------------------------------------------------

void SetPromiscuousIP( void )
{
    fPromiscuousIP = 1;
}

//------------------------------------------------------------------------------
//
//  Function Name:  ClearPromiscuousIP( void )
//  Description..:  This functions clears the global PromiscuousIP flag. The flag
//                  is used to indicate when the CheckUDP routine should only
//                  allow packets that match our address through.
//  Inputs.......:  none
//  Outputs......:  none
//
//------------------------------------------------------------------------------

void ClearPromiscuousIP( void )
{
    fPromiscuousIP = 0;
}

//------------------------------------------------------------------------------
//
//  Function Name:  EbootCheckUDP(...)
//  Description..:  This routine will check a UDP frame that has been received.
//                  It will make sure that it was for our IP address and that
//                  the checksums are right and that it's a UDP packet.  If
//                  something is wrong, the packet will be discarded and the
//                  routine will return non-zero.  If everything is right,
//                  then all the port and IP information will be filled out and
//                  the routine will return 0. Note that if we are doing the
//                  DHCP process, the DHCP server will send the OFFER packet to
//                  the IP address that it would like to give us.  We have to
//                  be able to accept a packet for any IP address in that
//                  case.  This condition is signaled to the routine by
//                  fPromiscuousIP == 1, which is set using SetPromiscuousIP()
//
//  Inputs.......:  EDBG_ADDR *         pntr to address
//                  BYTE *              pntr to frame
//                  UINT16 *            pntr to dst addr
//                  UINT16 *            pntr to src addr
//                  UINT16 **           pntr to pntr to data
//                  UINT16 *            pntr to data length
//  Outputs......:  0 on success, non zero on failure
//
//------------------------------------------------------------------------------

UINT16 EbootCheckUDP( EDBG_ADDR *pMyAddr,
                      BYTE      *pFrameBuffer,
                      UINT16    *wDestPort,
                      UINT16    *wSrcPort,
                      UINT16   **pwData,
                      UINT16    *cwLength )
{
    IPHeaderFormat          *pIPHeader;
    UDPPseudoHeaderFormat    UDPPseudoHeader;
    UDPHeaderFormat         *pUDPHeader;

    // Note that I don't do any checking that depends on a length field
    // in the packet until after the CRC is verified for that data.
    // This prevents the code from running past the end of buffers, etc.
    // when a bad packet is received.

    pIPHeader = (IPHeaderFormat *)(pFrameBuffer + sizeof(EthernetFrameHeader));

    // Make sure that it was for our IP address, unless we're
    // doing DHCP (indicated by fPromiscuousIP == 1)

    if( !fPromiscuousIP && pIPHeader->dwDestIP != pMyAddr->dwIP )
    {
//      KITLOutputDebugString(
//      "!CheckUDP: Not our IP (0x%X)\n",pIPHeader->dwDestIP);

        return( 1 );
    }

    // Make sure that it is a UDP packet

    if( pIPHeader->bProtocol != 17 )
    {
        KITLOutputDebugString( "!CheckUDP: Not UDP (proto = 0x%X)\r\n",
            pIPHeader->bProtocol );

        return( 2 );
    }

    // Check the IP header checksum

    if( CRC( (UINT16 *)pIPHeader, sizeof(IPHeaderFormat), NULL, 0 ) != 0 )
    {
        KITLOutputDebugString( "!CheckUDP: IP header checksum failure\r\n" );
        return( 3 );
    }

    // Build the UDP Pseudo Header

    UDPPseudoHeader.dwSrcIP   = pIPHeader->dwSrcIP;
    UDPPseudoHeader.dwDestIP  = pIPHeader->dwDestIP;
    UDPPseudoHeader.bZero     = 0;
    UDPPseudoHeader.bProtocol = 17;         // UDP Proto is 17
    UDPPseudoHeader.cwTotalUDPLength =
        htons( htons(pIPHeader->cwTotalLength) - sizeof( IPHeaderFormat ));

    // Check the UDP checksum, I'm using the cwTotalUDPLength
    // calculated from the IP header info because we know that
    // it's not corrupted and won't give an outrageous length for the packet

    pUDPHeader = (UDPHeaderFormat *)((BYTE *)pIPHeader+sizeof(IPHeaderFormat));

    if( pUDPHeader->wCRC != 0 &&
        CRC( (UINT16 *)&UDPPseudoHeader,
        sizeof(UDPPseudoHeader),
        (UINT16 *)pUDPHeader,
        ntohs(UDPPseudoHeader.cwTotalUDPLength) ) != 0 )
    {
        KITLOutputDebugString( "!CheckUDP: UDP header checksum failure\r\n" );
        return( 4 );
    }

    // Now we know we have a good packet, fill out the fields

    *wDestPort = pUDPHeader->wDestPort;
    *wSrcPort = pUDPHeader->wSrcPort;
    *pwData = (UINT16 *)((BYTE *)pUDPHeader + sizeof(UDPHeaderFormat));
    *cwLength = htons(pUDPHeader->cwTotalUDPLength) - sizeof(UDPHeaderFormat);

    // Indicate success

    return( 0 );
}

//------------------------------------------------------------------------------
//
//  Function Name:  DuplicateIPWarning( DWORD dwIP, BYTE *pchMAC )
//  Description..:  This function prints a warning when ProcessARP has
//                  detected an IP address collision.
//  Inputs.......:  DWORD       IP          ip address
//                  BYTE *      MAC         pntr to MAC address
//  Outputs......:  none
//
//------------------------------------------------------------------------------

static void DuplicateIPWarning( DWORD dwIP, BYTE *pchMAC )
{
    KITLOutputDebugString(
        "Duplicate IP Address Detected:\r\n"
        "-IP address %s in use by device with MAC address %B:%B:%B:%B:%B:%B.\r\n"
        "-Requesting new IP address via DHCP...\r\n",
        inet_ntoa( dwIP ),
        pchMAC[0], pchMAC[1], pchMAC[2], pchMAC[3], pchMAC[4], pchMAC[5] );
}

//------------------------------------------------------------------------------
//
//  Function Name:  CRC( ... )
//  Description..:  This routine will calculate the complemented 16-bit
//                  1's complement sum used to compute network CRCs.
//                  The CRC can be calculated over two different memory
//                  regions.  If only one region is desired, then the other's
//                  length can be set to 0.  Also, if an odd number of bytes
//                  are specified, the routine will convert the last byte to
//                  a word (w/o sign extension) and include that in the
//                  calculation.
//  Inputs.......:  UINT16 *        pntr to region 1
//                  UINT16          length of region 1
//                  UNIT16 *        pntr to region 2
//                  UINT16          length of region 2
//  Outputs......:  the calculated CRC
//
//------------------------------------------------------------------------------

static UINT16 CRC( UINT16 *pwRegion1,
                   UINT16  wLength1,
                   UINT16 *pwRegion2,
                   UINT16  wLength2 )
{
    DWORD dwSum;
    DWORD dwCarryOut;
    UINT16 wCRC;

    // There is no need to swap for network ordering during calculation
    // because of the end around carry used in 1's complement addition

    dwSum = 0;

    // Region 1

    while( wLength1 > sizeof(BYTE) )
    {
        dwSum += *pwRegion1++;
        wLength1 -= sizeof(UINT16);
    }

    if( wLength1 )
    {
        dwSum += ((UINT16)*(BYTE *)pwRegion1 & 0x00FF);
    }

    // Region 2

    while( wLength2 > sizeof(BYTE) )
    {
        dwSum += *pwRegion2++;
        wLength2 -= sizeof(UINT16);
    }

    if( wLength2 )
    {
        dwSum += ((UINT16)*(BYTE *)pwRegion2 & 0x00FF);
    }

    // Add back in all the carry out's from the lower
    // 16 bits because this is 1's complement.

    while( dwSum & 0xFFFF0000UL )
    {
        dwCarryOut = dwSum >> 16;
        dwSum &= 0x0000FFFFUL;
        dwSum += dwCarryOut;
    }

    wCRC = (UINT16)dwSum;

    // There is no need to flip for network byte order
    // because we did all the sums backwards already.

    wCRC = ~wCRC;

    return( wCRC );
}

//------------------------------------------------------------------------------
//
//  Function Name:  UpperDWFromMAC( BYTE *pAddr )
//  Description..:  This function extracts upper DWORD from MAC address.
//  Inputs.......:  BYTE *      pntr to address
//  Outputs......:  DWORD
//
//------------------------------------------------------------------------------

static DWORD UpperDWFromMAC( BYTE *pAddr )
{
    DWORD ret;

    // Given a hex ethernet address of 12 34 56 78 9a bc
    // the 4 byte return value should look like 0x00123456

    ret  = (DWORD)pAddr[0];
    ret  <<= 8;
    ret |= (DWORD)pAddr[1];
    ret  <<= 8;
    ret |= (DWORD)pAddr[2];
    return ret;
}
