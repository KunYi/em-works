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

  dhcp.h

Abstract:

  This contains the dhcp.c specific data structure declarations.

Functions:


Notes:


Revision History:

--*/
#ifndef DHCP_H
#define DHCP_H

// These structures must be byte aligned so that they can be laid over real data
#include <pshpack1.h>

// This is the format of a the DHCP data contained in the UDP datagram
typedef struct DHCPMsgFormatTag {

    BYTE bOperation;
    BYTE bHardwareAddrType;
    BYTE bHardwareAddrLen;
    BYTE bHops;
    DWORD dwXID;
    WORD wSecs;
    WORD wFlags;
    DWORD dwCIADDR;         // Address to verify previously allocated IP in DHCPREQUEST
    DWORD dwYIADDR;         // Address to use for the client
    DWORD dwSIADDR;         // Address of server to use for next step in boot process
    DWORD dwGIADDR;         // Address of relay agent being used, if any
    UINT16 wCHADDR[8];      // Client hardware address
    char szSNAME[64];       // Optional server host name, NULL terminated string
    char szFILE[128];       // Optional boot file name, NULL terminated string
                            //  "generic" or NULL in DHCP_DISCOVER, fully qualified in DHCP_OFFER
    BYTE bOptions[312];     // Optional parameter fields

} DHCPMsgFormat;

#include <poppack.h>


// Well known DHCP ports for use with UDP
#define DHCP_SERVER_PORT 0x4300
#define DHCP_CLIENT_PORT 0x4400



// These are the various DHCP or BOOTP Option types that are used for DHCP (RFC 1533)
typedef enum {

    DHCP_SUBNET_MASK = 1,    // Client's subnet mask
    DHCP_HOSTNAME = 12,
    DHCP_IP_ADDR_REQ = 50,
    DHCP_LEASE_TIME = 51,
    DHCP_OPTION_OVERLOAD = 52,
    DHCP_MSGTYPE = 53,
    DHCP_SERVER_ID = 54,
    DHCP_CLIENT_ID = 61,
    DHCP_END = 255

} DHCPOptions;



// These are the codes for the various DHCP message types.  They are used with a Option type of 53.
typedef enum {
    DHCP_DISCOVER = 1,
    DHCP_OFFER = 2,
    DHCP_REQUEST = 3,
    DHCP_DECLINE = 4,
    DHCP_ACK = 5,
    DHCP_NAK = 6,
    DHCP_RELEASE = 7
} DHCPMsgTypes;



// These are the states that the DHCP negotiation process can be in.
typedef enum {
    DHCP_INIT,
    DHCP_SELECTING,
    DHCP_REQUESTING,
    DHCP_BOUND
} DHCPStates;



UINT16 ProcessDHCP( EDBG_ADDR *pMyAddr, DWORD *pSubnetMask, BYTE *pbData, WORD cwLength, WORD *pfwDHCPComplete );
UINT16 SendDHCP( BYTE fRequestLastIP, DHCPMsgTypes MsgType, EDBG_ADDR *pServerAddr, EDBG_ADDR *pMyAddr, DWORD *dwXID );
void DHCPBuildOps( DHCPOptions DHCPOption, DHCPMsgFormat *pDHCPMsg, WORD *pwOpOff, DWORD dwData );
BYTE *DHCPFindOption( DHCPOptions DHCPOption, DHCPMsgFormat *pDHCPMsg );
BYTE *DHCPParseField( DHCPOptions DHCPOption, BYTE *pbParse );
UINT16 DHCPRetransmit( EDBG_ADDR *pMyAddr, EDBG_ADDR *pSrcAddr, DHCPMsgFormat *pDHCPMsg );
WORD ReadSerialIP( EDBG_ADDR *pMyAddr, DWORD *pSubnetMask, WORD *pfwNoTimeOut );
void UpdateEEPROMData(DWORD dwIP, DWORD dwSubnetMask);


#endif
