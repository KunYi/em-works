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
#ifndef __ETH_H
#define __ETH_H

//------------------------------------------------------------------------------

#ifndef _WINSOCKAPI_

#define htons(x)    ((UINT16)(((0x00FF&((UINT16)x)) << 8)|(((UINT16)((UINT16)x)) >> 8)))
#define ntohs(x)    htons(x)
#define htonl(x)    ((((ULONG)x)<<24)|((0x0000FF00UL&((ULONG)x))<<8)|   \
                    ((0x00FF0000UL&((ULONG)x))>>8)|(((ULONG)x)>>24))
#define ntohl(x)    htonl(x)

#endif

//------------------------------------------------------------------------------

#define ARP_FRAME                       0x0806
#define IP_FRAME                        0x0800

#define ICMP_PROTOCOL                   1
#define UDP_PROTOCOL                    17

#define IP4_HEADER_FLAG_MF              (1<<13)
#define IP4_HEADER_FLAG_DF              (1<<14)

#define DHCP_CLIENT_PORT                0x0044
#define DHCP_SERVER_PORT                0x0043
#define DHCP_COOKIE                     0x82538263

#define KITL_CLIENT_PORT                981
#define KITL_SERVER_PORT                981

#define EDBG_DOWNLOAD_PORT              980
#define EDBG_SERVICE_PORT               981

//------------------------------------------------------------------------------

#pragma pack( push, 1 )

//------------------------------------------------------------------------------

typedef struct ETH_HEADER {
    UINT16 destmac[3];
    UINT16 srcmac[3];
    UINT16 ftype;
} ETH_HEADER;

//------------------------------------------------------------------------------

typedef struct IP4_HEADER {
    UINT8  verlen;
    UINT8  tos;
    UINT16 length;
    UINT16 id;
    UINT16 fragment;
    UINT8  ttl;
    UINT8  protocol;
    UINT16 sum;
    uint32_t srcip;
    uint32_t destip;
} IP4_HEADER;

//------------------------------------------------------------------------------

typedef struct UDP_HEADER {
    UINT16 srcPort;
    UINT16 dstPort;
    UINT16 length;
    UINT16 sum;
} UDP_HEADER;

//------------------------------------------------------------------------------

typedef struct ARP_MESSAGE {
    UINT16 htype;
    UINT16 ptype;
    UINT8  hsize;
    UINT8  psize;
    UINT16 op;
    UINT16 srcmac[3];
    uint32_t srcip;
    UINT16 destmac[3];
    uint32_t destip;
} ARP_MESSAGE;

typedef enum {
    ARP_REQUEST = 1,
    ARP_RESPONSE = 2
} ARP_MESSAGE_OP;

//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4200) //nonstandard extension used : zero-sized array in struct/union
typedef struct ICMP_HEADER {
    UINT8  op;
    UINT8  code;
    UINT16 sum;
    UINT8  data[];
} ICMP_HEADER;
#pragma warning(pop)

typedef enum {
    ICMP_ECHOREPLY = 0,
    ICMP_ECHOREQ = 8
} ICMP_MESSAGE_OP;

//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4200) //nonstandard extension used : zero-sized array in struct/union
typedef struct BOOTP_MESSAGE {
    UINT8  op;              // Op/message code
    UINT8  htype;           // Hardware address type
    UINT8  hlen;            // Hardware address length
    UINT8  hops;            //
    uint32_t xid;           // Transaction ID
    UINT16 secs;            // Time since renewal start
    UINT16 flags;           // Flags
    uint32_t ciaddr;        // Existing client address
    uint32_t yiaddr;        // Address to use for the client
    uint32_t siaddr;        // Address of server to use for next step
    uint32_t giaddr;        // Address of relay agent being used, if any
    UINT8  chaddr[16];      // Client hardware address
    CHAR   sname[64];       // Optional server host name
    CHAR   file[128];       // Optional boot file name
    UINT8  options[0];      // Optional parameter fields
} BOOTP_MESSAGE;
#pragma warning(pop)

typedef enum {
    DHCP_DISCOVER = 1,
    DHCP_OFFER = 2,
    DHCP_REQUEST = 3,
    DHCP_DECLINE = 4,
    DHCP_ACK = 5,
    DHCP_NAK = 6,
    DHCP_RELEASE = 7
} DHCP_MESSAGE_TYPE;

typedef enum {
    DHCP_SUBNET_MASK = 1,
    DHCP_HOSTNAME = 12,
    DHCP_IP_ADDR_REQ = 50,
    DHCP_LEASE_TIME = 51,
    DHCP_OPTION_OVERLOAD = 52,
    DHCP_MSGTYPE = 53,
    DHCP_SERVER_ID = 54,
    DHCP_RENEW_TIME = 58,
    DHCP_REBIND_TIME = 59,
    DHCP_CLIENT_ID = 61,
    DHCP_END = 255
} DHCP_OPTIONS;

typedef enum {
    DHCP_BOUND,
    DHCP_SELECTING,
    DHCP_REQUESTING
} DHCP_STATE;

//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4200) //nonstandard extension used : zero-sized array in struct/union
typedef struct TFTP_HEADER {
    UINT16 opcode;
    UINT8  data[0];
} TFTP_HEADER;
#pragma warning(pop)

enum TFTP_OPCODE {
    TFTP_OPCODE_RRQ = 1,
    TFTP_OPCODE_WRQ = 2,
    TFTP_OPCODE_DATA = 3,
    TFTP_OPCODE_ACK = 4,
    TFTP_OPCODE_ERROR = 5
};

enum TFTP_ERROR {
    TFTP_ERROR_NOT_DEFINED = 0,
    TFTP_ERROR_FILE_NOT_FOUND = 1,
    TFTP_ERROR_ACCESS_VIOLATION = 2,
    TFTP_ERROR_DISK_FULL = 3,
    TFTP_ERROR_ILLEGAL_OPERATION = 4,
    TFTP_ERROR_UNKNOWN_TRANSFER_ID = 5,
    TFTP_ERROR_FILE_EXISTS = 6,
    TFTP_ERROR_NO_SUCH_USER = 7
};    
    
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4200) //nonstandard extension used : zero-sized array in struct/union
typedef struct EDBG_HEADER {
    uint32_t id;                // Protocol identifier ("EDBG" on the wire)
    UINT8 service;              // Service identifier
    UINT8 flags;                // Flags (see defs below)
    UINT8 seqNum;               // For detection of dropped packets
    UINT8 cmd;                  // For administrative messages
    UINT8 data[0];              // Cmd specific data starts here
} EDBG_HEADER;
#pragma warning(pop)

#define EDBG_FLAG_FROM_DEVICE   0x01    // Set if message is from the device
#define EDBG_FLAG_NACK          0x02    // Set if frame is a nack
#define EDBG_FLAG_ACK           0x04    // Set if frame is an ack
#define EDBG_FLAG_SYNC          0x08    // Can be used to reset sequence # to 0
#define EDBG_FLAG_ADMIN_RESP    0x10    // Indicate an admin response

#define EDBG_CMD_BOOTME         0       // Initial bootup message from device
#define EDBG_CMD_JUMP           2

typedef struct EDBG_BOOTME {
    UINT8  versionMajor;        // Bootloader version
    UINT8  versionMinor;        // Bootloader version
    UINT16 mac[3];              // Ether address of device (net byte order)
    uint32_t ip;                // IP address of device (net byte order)
    CHAR   platformName[17];    // Platform name
    CHAR   deviceName[17];      // Device name
    UINT8  cpuId;               // CPU identifier (upper nibble = type)
    UINT8  bootmeVersion;       // BOOTME version (currently 2)
    uint32_t bootFlags;         // Boot flags
    UINT16 downloadPort;        // Download Port (0 -> EDBG_DOWNLOAD_PORT)
    USHORT servicePort;         // Service Port (0 -> EDBG_SVC_PORT)
} EDBG_BOOTME;

//------------------------------------------------------------------------------

#pragma pack( pop )

//------------------------------------------------------------------------------

#endif
