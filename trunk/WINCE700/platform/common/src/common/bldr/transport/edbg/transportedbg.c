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
#include <bootTransportEdbg.h>
#include <bootTransportEdbgNotify.h>
#include <bootString.h>
#include <bootDownloadBinFormat.h>
#include <bootCore.h>
#include <bootMemory.h>
#include <bootLog.h>
#include <ethdbg.h>
#include "eth.h"

#define KTS_PASSIVE_MODE 0x40       // flag for passive kitl mode
#define KTS_KITL_TRANSPORT_MEDIA_MASK 0x3f

//------------------------------------------------------------------------------

#define EDBG_DMA_BUFFER_SIZE        0x20000

//------------------------------------------------------------------------------

enum TFTP_STATE {
    TFTP_STATE_IDLE = 0,
    TFTP_STATE_WRITE,
    TFTP_STATE_WRITE_LAST,
    TFTP_STATE_JUMP
};

//------------------------------------------------------------------------------

typedef struct OSConfigInfoFromJump_t {
    bool_t bValid;      // Set true when the Jump Command is recieved and the 
                        //      rest of the structure is filled in
    bool_t bPassiveKitl;
    uint32_t KitlTransportMediaType; 
} OSConfigInfoFromJump_t;

typedef struct Transport_t {

    BootDriverVTable_t *pVTable;

    void*   pContext;
    
    BootEdbgDriver_t edbg;

    uint8_t* pDmaBuffer;
    uint32_t dmaBufferSize;
    
    uint32_t bootMeRetry;
    uint32_t bootMeDelay;
    uint32_t tftpTimeout;
    uint32_t dhcpRetry;
    uint32_t dhcpTimeout;
    uint32_t arpTimeout;

    uint16_t mac[3];
    uint32_t ip;

    char     name[17];
    char     unused[3];
    
    uint8_t  packet[2048];

    enum_t   tftpState;
    uint16_t tftpMac[3];
    uint32_t tftpIP;
    uint16_t tftpPort;
    uint16_t tftpBlock;
    size_t   tftpBlockLength;
    size_t   tftpOffset;
    size_t   tftpLength;

    enum_t   dhcpState;
    uint32_t dhcpServerIP;
    uint32_t dhcpXId;

    size_t readPos;
        
    OSConfigInfoFromJump_t osConfigInfo;
} Transport_t;


//------------------------------------------------------------------------------
//  Local functions

bool_t
BootTransportEdbgDeinit(
    void *pContext
    );

bool_t
BootTransportEdbgIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    );

static
bool_t
Read(
    Transport_t *pContext,
    void *pBuffer,
    size_t bufferLength
    );

static
bool_t
ReadOsConfig(
    Transport_t *pTransport,
    enum_t type,
    VOID *pBuffer,
    size_t bufferLength
    );


//------------------------------------------------------------------------------

static
BootDriverVTable_t
s_transportVTable = {
    BootTransportEdbgDeinit,
    BootTransportEdbgIoCtl
};

static
Transport_t
s_transport;

//------------------------------------------------------------------------------

void*
BootTransportEdbgInit(
    void *pContext,
    const BootEdbgDriver_t *pDriver,
    pfnBootTransportEdbgName_t pfnName,
    uint8_t *pAddress,
    uint32_t offset,
    uint16_t mac[3],
    uint32_t ip
    )
{
    void *pTransport = NULL;
    
    // This driver shouldn't be instantied twice...
    if (s_transport.pVTable != NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgInit: "
            L"Driver already initialized!\r\n"
            ));
        goto cleanUp;
        }

    memset(&s_transport, 0, sizeof(s_transport));

    // Initialize hardware
    if (pDriver->pfnDmaInit != NULL)
        {
        s_transport.dmaBufferSize = EDBG_DMA_BUFFER_SIZE;
        s_transport.pDmaBuffer = BootAlloc(s_transport.dmaBufferSize);
        if (s_transport.pDmaBuffer == NULL)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgInit: "
                L"Failed allocate DMA buffer!\r\n"
                ));
            goto cleanUp;
            }
        if (!pDriver->pfnDmaInit(
                (uint32_t)s_transport.pDmaBuffer, s_transport.dmaBufferSize
                ))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgInit: "
                L"Ethernet driver DMA initialization failed!\r\n"
                ));
            goto cleanUp;
            }
        }
    
    if (!pDriver->pfnInit(pAddress, offset, mac))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgInit: "
            L"Ethernet driver initialization failed!\r\n"
            ));
        goto cleanUp;
        }

    // Create device name
    if (!pfnName(pContext, mac, s_transport.name, dimof(s_transport.name)))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgInit: "
            L"Failed create device name!\r\n"
            ));
        goto cleanUp;
        }

    // Initialize virtual table & save context
    s_transport.pVTable = &s_transportVTable;
    s_transport.pContext = pContext;

    // Set default values
    s_transport.bootMeRetry = 60;
    s_transport.bootMeDelay = 2000;
    s_transport.tftpTimeout = 4000;
    s_transport.dhcpTimeout = 9000;
    s_transport.dhcpRetry   = 26; // Try for 4 minutes
    s_transport.arpTimeout  = 2000;

    // Save driver, mac & ip to transport context
    memcpy(&s_transport.edbg, pDriver, sizeof(s_transport.edbg));
    memcpy(s_transport.mac, mac, sizeof(s_transport.mac));
    s_transport.ip = ip;

    // Done
    pTransport = &s_transport;

cleanUp:
    return pTransport;
}

//------------------------------------------------------------------------------

bool_t
BootTransportEdbgDeinit(
    void *pContext
    )
{
    bool_t rc = false;
    Transport_t *pTransport = pContext;


    // Check driver handle
    if (pTransport != &s_transport)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgDeinit: "
            L"Invalid driver handle!\r\n"
            ));
        goto cleanUp;
        }

    // Deinitialize Ethernet driver
    if (pTransport->edbg.pfnDeinit != NULL) pTransport->edbg.pfnDeinit();
    if (pTransport->pDmaBuffer != NULL) BootFree(pTransport->pDmaBuffer);
    
    // Clear context
    memset(pTransport, 0, sizeof(*pTransport));

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootTransportEdbgIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool_t rc = false;
    Transport_t *pTransport = pContext;

    
    // Check driver handle
    if (pTransport != &s_transport)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgIoCtl: "
            L"Invalid driver handle!\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case BOOT_TRANSPORT_IOCTL_READ:
            {
            BootTransportReadParams_t *pParams = pBuffer;
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgIoCtl: "
                    L"Invalid BOOT_TRANSPORT_IOCTL_READ parameter!\r\n"
                    ));
                break;
                }
            rc = Read(pTransport, pParams->pBuffer, pParams->size);
            }
            break;
        case BOOT_TRANSPORT_EDBG_IOCTL_OPTIONS:
            {
            BootTransportEdbgOptionsParams_t* pParams = pBuffer;
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgIoCtl: "
                    L"Invalid BOOT_TRANSPORT_EDBG_IOCTL_OPTIONS parameter!\r\n"
                    ));
                break;
                }
            if (pParams->arpTimeout != -1)
                pTransport->arpTimeout = pParams->arpTimeout;
            if (pParams->dhcpTimeout != -1)
                pTransport->dhcpTimeout = pParams->dhcpTimeout;
            if (pParams->dhcpRetry != -1)
                pTransport->dhcpRetry = pParams->dhcpRetry;
            if (pParams->tftpTimeout != -1)
                pTransport->tftpTimeout = pParams->tftpTimeout;
            if (pParams->bootMeDelay != -1)
                pTransport->bootMeDelay = pParams->bootMeDelay;
            if (pParams->bootMeRetry != -1)
                pTransport->bootMeRetry = pParams->bootMeRetry;
            rc = TRUE;
            }
            break;
        case BOOT_TRANSPORT_IOCTL_GET_OS_CONFIG_INFO:
            {
            BootTransportGetOsConfigInfoParams_t* pParams = pBuffer;
            if((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportEdbgIoCtl: "
                    L"Invalid BOOT_TRANSPORT_EDBG_IOCTL_READ_OS_CONFIG parameter!\r\n"
                    ));
                break;
                }
            rc = ReadOsConfig(pTransport, pParams->type, pParams->pBuffer, pParams->BufferSize);
            }
            break;
   
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
UINT16
Sum(
    UINT16 seed,
    const VOID *pData,
    size_t dataSize
    )
{
    uint32_t sum;
    const uint8_t *p;


    // There is no need to swap for network ordering during calculation
    // because of the end around carry used in 1's complement addition
    sum = seed;
    p = pData;
    while (dataSize >= sizeof(UINT16))
        {
        sum += p[0] + (p[1] << 8);
        p += 2;
        dataSize -= 2;
        }
    if (dataSize > 0) sum += *((UINT8*)p);

    while ((sum & 0xFFFF0000) != 0)
        {
        sum = (sum & 0x0000FFFF) + (sum >> 16);
        }
    return (UINT16)sum;
}

//------------------------------------------------------------------------------

static
UINT8*
FindDHCPOption(
    BOOTP_MESSAGE *pDHCP,
    DHCP_OPTIONS option
    )
{
    uint8_t *p;


    p = &pDHCP->options[4];
    while (*p != DHCP_END)
        {
        if (*p == option) break;
        p += p[1] + 2;
        }
    if (*p == DHCP_END) p = NULL;
    return p;
}

//------------------------------------------------------------------------------

static
VOID
AddDHCPOption(
    BOOTP_MESSAGE *pDHCP,
    DHCP_OPTIONS option,
    uint32_t *pOffset,
    VOID *pData,
    uint32_t size
    )
{
    pDHCP->options[(*pOffset)++] = option;
    pDHCP->options[(*pOffset)++] = (UINT8)size;
    memcpy(&pDHCP->options[*pOffset], pData, size);
    *pOffset += size;
}

//------------------------------------------------------------------------------

static
LPCSTR
FindTFTPOption(
    Transport_t *pTransport,
    size_t offset,
    size_t length,
    cstring_t name
    )
{
    cstring_t p;


    p = (cstring_t)&pTransport->packet[offset];
    // Skip file name
    while ((length-- > 0) && (*p++ != '\0'));
    // Skip file type
    while ((length-- > 0) && (*p++ != '\0'));
    // Now compare    
    while (*p != '\0')
        {
        if (strcmp(p, name) != 0)
            {
            // Skip option name
            while ((length-- > 0) && (*p++ != '\0'));
            // Skip option value
            while ((length-- > 0) && (*p++ != '\0'));
            }
        else
            {
            // Skip option name
            while ((length-- > 0) && (*p++ != '\0'));
            // Pointer is now on option value
            break;
            }
        }

    if ((length == 0) || (*p == '\0')) p = NULL;
    return p;
}

//------------------------------------------------------------------------------

static
VOID
SendFrame(
    Transport_t *pTransport,
    size_t offset,
    size_t length,
    const uint16_t destmac[3],
    const uint16_t ftype
    )
{
    ETH_HEADER *pEth = (ETH_HEADER*)pTransport->packet;


    if (offset < sizeof(ETH_HEADER)) goto cleanUp;
    offset -= sizeof(ETH_HEADER);
    pEth = (ETH_HEADER*)(pTransport->packet + offset);

    // Fill in Ethernet header
    memcpy(pEth->destmac, destmac, sizeof(pEth->destmac));
    memcpy(pEth->srcmac, pTransport->mac, sizeof(pEth->srcmac));
    pEth->ftype = htons(ftype);
    length += sizeof(ETH_HEADER);

    // Send it to wire
    pTransport->edbg.pfnSendFrame((UINT8*)pEth, length);

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
SendARP(
    Transport_t *pTransport,
    const uint16_t destmac[3],
    const uint32_t destip,
    const uint16_t op
    )
{
    ARP_MESSAGE *pARP;
    size_t offset, length;


    offset = sizeof(ETH_HEADER);
    length = sizeof(ARP_MESSAGE);

    pARP = (ARP_MESSAGE*)(pTransport->packet + offset);
    pARP->htype = htons(1);             // Ethernet
    pARP->ptype = htons(IP_FRAME);      // IP4 addresses
    pARP->hsize = 6;                    // MAC address is 6 bytes long
    pARP->psize = 4;                    // IP4 addresses are 4 bytes long
    pARP->op = htons(op);               // Specify an ARP op

    // Fill in the source side information
    memcpy(pARP->srcmac, pTransport->mac, sizeof(pARP->srcmac));
    pARP->srcip = pTransport->ip;

    // Fill in the destination information
    memcpy(pARP->destmac, destmac, sizeof(pARP->destmac));
    pARP->destip = destip;

    // Send frame
    SendFrame(pTransport, offset, length, destmac, ARP_FRAME);
}

//------------------------------------------------------------------------------

static
VOID
SendIP(
    Transport_t *pTransport,
    size_t offset,
    size_t length,
    const uint16_t destmac[3],
    const uint32_t destip,
    const uint8_t protocol
    )
{
    static uint16_t ipId = 0;
    IP4_HEADER *pIP;


    // We must fit in buffer
    if (offset < sizeof(IP4_HEADER)) goto cleanUp;

    // Get IP header pointer
    offset -= sizeof(IP4_HEADER);
    pIP = (IP4_HEADER*)(pTransport->packet + offset);
    length += sizeof(IP4_HEADER);

    // Fill in IP4 header
    pIP->verlen = 0x45;
    pIP->tos = 0;
    pIP->length = htons(length);
    pIP->id = htons(ipId);
    pIP->fragment = 0;
    pIP->ttl = 64;
    pIP->protocol = protocol;
    pIP->sum = 0;
    pIP->srcip = pTransport->ip;
    pIP->destip = destip;

    // Get new id
    ipId++;

    // Compute IP4 header checksum
    pIP->sum = ~ Sum(0, pIP, sizeof(IP4_HEADER));

    // Send frame
    SendFrame(pTransport, offset, length, destmac, IP_FRAME);

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
SendUDP(
    Transport_t *pTransport,
    size_t offset,
    size_t length,
    const uint16_t destmac[3],
    const uint32_t destip,
    const uint16_t srcPort,
    const uint16_t destPort
    )
{
    UDP_HEADER *pUDP;
    uint16_t xsum;


    // We must fit in buffer
    if (offset < sizeof(UDP_HEADER)) goto cleanUp;
    offset -= sizeof(UDP_HEADER);
    pUDP = (UDP_HEADER*)(pTransport->packet + offset);

    // First calculate UDP length
    length += sizeof(UDP_HEADER);

    // Fill in UDP header
    pUDP->srcPort = htons(srcPort);
    pUDP->dstPort = htons(destPort);
    pUDP->length = htons(length);
    pUDP->sum = 0;

    // Compute UDP header checksum
    xsum = htons(UDP_PROTOCOL);
    xsum = Sum(xsum, &pTransport->ip, sizeof(uint32_t));
    xsum = Sum(xsum, &destip, sizeof(uint32_t));
    xsum = Sum(xsum, &pUDP->length, sizeof(uint32_t));
    xsum = Sum(xsum, pUDP, length);
    pUDP->sum = ~xsum;

    // And now do IP encode
    SendIP(pTransport, offset, length, destmac, destip, UDP_PROTOCOL);

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
SendDHCP(
    Transport_t *pTransport,
    DHCP_MESSAGE_TYPE msgType,
    uint32_t requestIP
    )
{
    BOOTP_MESSAGE *pDHCP;
    size_t offset, length;
    uint8_t clientId[7];
    uint16_t destmac[3];
    uint32_t destip;


    // Calculate BOOTP message position in buffer
    offset = sizeof(UDP_HEADER) + sizeof(IP4_HEADER) + sizeof(ETH_HEADER);
    pDHCP = (BOOTP_MESSAGE*)(pTransport->packet + offset);

    // Clear all fields
    memset(pDHCP, 0, sizeof(BOOTP_MESSAGE));

    // BOOTP header
    pDHCP->op = 1;                              // BOOTREQUEST
    pDHCP->htype = 1;                           // 10 Mbps Ethernet
    pDHCP->hlen = 6;                            // Ethernet address size
    pDHCP->secs = 0;                            // Who care?
    pDHCP->xid = pTransport->dhcpXId;

    // Device MAC address
    memcpy(pDHCP->chaddr, pTransport->mac, sizeof(pTransport->mac));

    // Start with options
    length = 0;

    // DHCP cookie
    pDHCP->options[length++] = 0x63;
    pDHCP->options[length++] = 0x82;
    pDHCP->options[length++] = 0x53;
    pDHCP->options[length++] = 0x63;

    // DHCP message type
    AddDHCPOption(pDHCP, DHCP_MSGTYPE, &length, &msgType, 1);

    // Add client id
    clientId[0] = 1;
    memcpy(&clientId[1], pTransport->mac, 6);
    AddDHCPOption(pDHCP, DHCP_CLIENT_ID, &length, clientId, 7);

    switch (msgType)
        {
        case DHCP_REQUEST:
            if (requestIP != 0)
                {
                AddDHCPOption(
                    pDHCP, DHCP_IP_ADDR_REQ, &length, &requestIP,
                    sizeof(requestIP)
                    );
                AddDHCPOption(
                    pDHCP, DHCP_SERVER_ID, &length, &pTransport->dhcpServerIP,
                    sizeof(pTransport->dhcpServerIP)
                    );
                }
            else
                {
                pDHCP->ciaddr = pTransport->ip;
                }
            break;
        case DHCP_DECLINE:
            AddDHCPOption(
                pDHCP, DHCP_IP_ADDR_REQ, &length, &requestIP, sizeof(requestIP)
                );
            AddDHCPOption(
                pDHCP, DHCP_SERVER_ID, &length, &pTransport->dhcpServerIP,
                sizeof(pTransport->dhcpServerIP)
                );
            break;
        }

    // DHCP message end
    pDHCP->options[length++] = DHCP_END;

    // Update length for BOOTP message
    length += sizeof(BOOTP_MESSAGE);

    // Set destination mac address
    destmac[0] = destmac[1] = destmac[2] = 0xFFFF;
    destip = 0xFFFFFFFF;

    // Send UDP packet
    SendUDP(
        pTransport, offset, length, destmac, destip,
        DHCP_CLIENT_PORT, DHCP_SERVER_PORT
        );
}

//------------------------------------------------------------------------------

static
VOID
SendBootMe(
    Transport_t *pTransport
    )
{
    static UINT8 bootMeSeqNum = 0;
    size_t offset, length;
    EDBG_HEADER *pEdbg;
    EDBG_BOOTME *pData;
    uint16_t destmac[3];
    uint32_t destip;


    offset = sizeof(UDP_HEADER) + sizeof(IP4_HEADER) + sizeof(ETH_HEADER);
    pEdbg = (EDBG_HEADER*)(pTransport->packet + offset);
    pData = (EDBG_BOOTME*)pEdbg->data;

    // Fill EDBG packet header
    pEdbg->id = 'GBDE';
    pEdbg->service = 0xFF;
    pEdbg->flags = EDBG_FLAG_FROM_DEVICE;
    pEdbg->seqNum = bootMeSeqNum++;
    pEdbg->cmd = EDBG_CMD_BOOTME;

    // Fill BOOTME data
    memset(pData, 0, sizeof(EDBG_BOOTME));
    pData->versionMajor = 1;
    pData->versionMinor = 0;
    memcpy(pData->mac, pTransport->mac, sizeof(pData->mac));
    pData->ip = pTransport->ip;
    memcpy(pData->platformName, pTransport->name, sizeof(pData->platformName));
    memcpy(pData->deviceName, pTransport->name, sizeof(pData->deviceName));
    pData->cpuId = 0;
    pData->bootmeVersion = 2;
    pData->bootFlags = EDBG_CAPS_TFTP_OPTIONS | EDBG_CAPS_PASSIVEKITL;

    pData->downloadPort = 0;
    pData->servicePort = 0;

    length = sizeof(EDBG_HEADER) + sizeof(EDBG_BOOTME);

    // Set destination mac address
    destmac[0] = destmac[1] = destmac[2] = 0xFFFF;
    destip = 0xFFFFFFFF;

    SendUDP(
        pTransport, offset, length, destmac, destip,
        EDBG_DOWNLOAD_PORT, EDBG_DOWNLOAD_PORT
        );
}

//------------------------------------------------------------------------------

static
VOID
SendTFTPAck(
    Transport_t *pTransport
    )
{
    size_t offset, length;
    TFTP_HEADER *pTFTP;


    offset = sizeof(UDP_HEADER) + sizeof(IP4_HEADER) + sizeof(ETH_HEADER);
    pTFTP = (TFTP_HEADER*)(pTransport->packet + offset);

    length = sizeof(TFTP_HEADER) + sizeof(UINT16);
    if ((sizeof(pTransport->packet) - offset) < length) goto cleanUp;

    pTFTP->opcode = htons(TFTP_OPCODE_ACK);
    *(UINT16*)pTFTP->data = htons(pTransport->tftpBlock);
    
    SendUDP(
        pTransport, offset, length, pTransport->tftpMac, pTransport->tftpIP, 
        EDBG_DOWNLOAD_PORT, pTransport->tftpPort
        );

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
SendTFTPError(
    Transport_t *pTransport,
    const uint16_t destmac[3],
    const uint32_t destip,
    const uint16_t destport,
    const uint16_t code
    )
{
    size_t offset, length;
    TFTP_HEADER *pTFTP;


    offset = sizeof(UDP_HEADER) + sizeof(IP4_HEADER) + sizeof(ETH_HEADER);
    pTFTP = (TFTP_HEADER*)(pTransport->packet + offset);

    length = sizeof(TFTP_HEADER) + sizeof(uint16_t) + 1;
    if ((sizeof(pTransport->packet) - offset) < length) goto cleanUp;
    
    pTFTP->opcode = htons(TFTP_OPCODE_ERROR);
    *(UINT16*)pTFTP->data = htons(code);
    pTFTP->data[sizeof(uint16_t) + 1] = '\0';
    
    SendUDP(
        pTransport, offset, length, destmac, destip, EDBG_DOWNLOAD_PORT,
        destport
        );

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
UINT16
GetOpARP(
    Transport_t *pTransport
    )
{
    uint16_t op = 0;
    ETH_HEADER *pEth = (ETH_HEADER*)pTransport->packet;
    ARP_MESSAGE *pARP = (ARP_MESSAGE*)((uint8_t *)pEth + sizeof(ETH_HEADER));


    if (pEth->ftype != htons(ARP_FRAME)) goto cleanUp;
    op = htons(pARP->op);

cleanUp:
    return op;
}

//------------------------------------------------------------------------------

static
VOID
DecodeEDBG(
    Transport_t *pTransport,
    size_t offset,
    size_t length,
    const uint16_t srcmac[3],
    uint32_t srcip,
    uint16_t srcport
    )
{
    EDBG_HEADER *pEdbg;
    PEDBG_OS_CONFIG_DATA pOSConfigData = NULL;


    if (length < sizeof(EDBG_HEADER)) goto cleanUp;
    pEdbg = (EDBG_HEADER*)(pTransport->packet + offset);

    switch (pEdbg->cmd)
        {
        case EDBG_CMD_JUMP:
            // If this is direct jump packet update server info
            if (pTransport->tftpIP == 0)
                {
                pTransport->tftpIP = srcip;
                pTransport->tftpPort = srcport;
                memcpy(pTransport->tftpMac, srcmac, sizeof(pTransport->tftpMac));
                }

            // Get the KitlTransport options out of the OSConfig information in the jump packet
            // The rest of the OSConfig data isn't valid within an EDBG_CMD_JUMP packet      
            pOSConfigData = (PEDBG_OS_CONFIG_DATA)(&(pEdbg->data));       
            if(pOSConfigData->KitlTransport & KTS_PASSIVE_MODE)
                {
                pTransport->osConfigInfo.bPassiveKitl = true;
                }

            pTransport->osConfigInfo.KitlTransportMediaType = 
                pOSConfigData->KitlTransport & KTS_KITL_TRANSPORT_MEDIA_MASK;
            
            pTransport->osConfigInfo.bValid = true;  // The OsConfigInfo is now valid
            
            // Update state
            pTransport->tftpState = TFTP_STATE_JUMP;
            // Update flags & send it back
            pEdbg->flags = EDBG_FLAG_FROM_DEVICE|EDBG_FLAG_ACK;
            length = sizeof(EDBG_HEADER);
            SendUDP(
                pTransport, offset, length, srcmac, srcip,
                EDBG_DOWNLOAD_PORT, srcport
                );
            break;
            
        }

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
DecodeTFTP(
    Transport_t *pTransport,
    size_t offset,
    size_t length,
    const uint16_t srcmac[3],
    uint32_t srcip,
    uint16_t srcport
    )
{
    TFTP_HEADER *pTFTP;
    uint16_t code, block;
    cstring_t pOption;
    long blksize;


    if (length < sizeof(TFTP_HEADER)) goto cleanUp;
    pTFTP = (TFTP_HEADER*)(pTransport->packet + offset);
    length -= sizeof(TFTP_HEADER);
    offset += sizeof(TFTP_HEADER);

    switch (pTFTP->opcode)
        {
        case htons(TFTP_OPCODE_WRQ):
            if (pTransport->tftpState != TFTP_STATE_IDLE)
            {
                // Old bootloader allowed multiple WRQ requests - drop this packet.
                break;
            }
            if (strcmp((const char *)pTFTP->data, "boot.bin") != 0)
                {
                code = TFTP_ERROR_ACCESS_VIOLATION;
                SendTFTPError(pTransport, srcmac, srcip, srcport, code);
                break;
                }
            pOption = FindTFTPOption(pTransport, offset, length, "blksize");
            blksize = 512;
            if (pOption != NULL) blksize = atol(pOption);
            if ((blksize < 512) || (blksize > 1024)) blksize = 512;
            pTransport->tftpState = TFTP_STATE_WRITE;
            memcpy(pTransport->tftpMac, srcmac, sizeof(pTransport->tftpMac));
            pTransport->tftpIP = srcip;
            pTransport->tftpPort = srcport;
            pTransport->tftpBlock = 0;
            pTransport->tftpBlockLength = blksize;
            pTransport->tftpOffset = 0;
            pTransport->tftpLength = 0;
            SendTFTPAck(pTransport);
            break;
        case htons(TFTP_OPCODE_DATA):
            if ((pTransport->tftpState == TFTP_STATE_IDLE) ||
                (pTransport->tftpIP != srcip) || 
                (pTransport->tftpPort != srcport))
                {
                code = TFTP_ERROR_ILLEGAL_OPERATION;
                SendTFTPError(pTransport, srcmac, srcip, srcport, code);
                break;
                }
            block = (pTFTP->data[0] << 8) + pTFTP->data[1];
            if (block == pTransport->tftpBlock)
                {
                SendTFTPAck(pTransport);
                break;
                }
            if (block != (uint16_t)(pTransport->tftpBlock + 1))
                {
                code = TFTP_ERROR_ILLEGAL_OPERATION;
                SendTFTPError(pTransport, srcmac, srcip, srcport, code);
                break;
                }
            pTransport->tftpBlock = block;
            pTransport->tftpOffset = offset + sizeof(uint16_t);
            pTransport->tftpLength = length - sizeof(uint16_t);
            if (pTransport->tftpLength < pTransport->tftpBlockLength)
                {
                pTransport->tftpState = TFTP_STATE_WRITE_LAST;
                }
            break;
        case htons(TFTP_OPCODE_RRQ):
        case htons(TFTP_OPCODE_ACK):            
        default:
            code = TFTP_ERROR_ILLEGAL_OPERATION;
            SendTFTPError(pTransport, srcmac, srcip, srcport, code);
            break;
        }
    
cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
DecodeDHCP(
    Transport_t *pTransport,
    size_t offset,
    size_t length,
    const uint16_t srcmac[3],
    uint32_t srcip
    )
{
    BOOTP_MESSAGE *pDHCP;
    const uint8_t *pOption;
    uint8_t msgType;

    UNREFERENCED_PARAMETER(srcmac);
    UNREFERENCED_PARAMETER(srcip);

    // Check & update offset and size
    if (length < sizeof(BOOTP_MESSAGE)) goto cleanUp;
    pDHCP = (BOOTP_MESSAGE*)(pTransport->packet + offset);

    // Check magic DHCP cookie & transaction id
    if ((pDHCP->options[0] != 0x63) || (pDHCP->options[1] != 0x82) ||
        (pDHCP->options[2] != 0x53) || (pDHCP->options[3] != 0x63) ||
        (pDHCP->xid != pTransport->dhcpXId))
        goto cleanUp;

    // Then find DHCP message type
    pOption = FindDHCPOption(pDHCP, DHCP_MSGTYPE);
    if (pOption == NULL) goto cleanUp;
    msgType = pOption[2];

    // Message processing depend on DHCP client state
    switch (pTransport->dhcpState)
        {
        case DHCP_SELECTING:
            // Ignore anything other then offer
            if (msgType != DHCP_OFFER) break;
            // Find server IP address
            pOption = FindDHCPOption(pDHCP, DHCP_SERVER_ID);
            if (pOption == NULL) break;
            memcpy(&pTransport->dhcpServerIP, &pOption[2], sizeof(uint32_t));
            // Request offered IP address
            SendDHCP(pTransport, DHCP_REQUEST, pDHCP->yiaddr);
            // We moved to new state
            pTransport->dhcpState = DHCP_REQUESTING;
            break;
        case DHCP_REQUESTING:
            if (msgType == DHCP_ACK)
                {
                // Set assigned address
                pTransport->ip = pDHCP->yiaddr;
                // We get address, let check it...
                pTransport->dhcpState = DHCP_BOUND;
                }
            else if (msgType == DHCP_NAK)
                {
                pTransport->ip = 0;
                // Discover DHCP servers
                SendDHCP(pTransport, DHCP_DISCOVER, 0);
                // Start with discover again
                pTransport->dhcpState = DHCP_SELECTING;
                }
            break;
        }

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
DecodeUDP(
    Transport_t *pTransport,
    size_t offset,
    size_t length,
    const uint16_t srcmac[3],
    uint32_t srcip,
    uint32_t destip
    )
{
    UDP_HEADER *pUDP;
    size_t udpLength;
    uint16_t xsum, srcport;
    uint32_t id;


    // Check & update offset and size
    if (length < sizeof(UDP_HEADER)) goto cleanUp;
    pUDP = (UDP_HEADER*)(pTransport->packet + offset);
    offset += sizeof(UDP_HEADER);

    // Check size
    udpLength = ntohs(pUDP->length);
    if (length < udpLength) goto cleanUp;
    
    // Packet checksum verification. The rules are as follows:
    //      IPv4: If pUDP->sum == 0 
    //              dont verify the checksum, and always accept the packet 
    //      IPv6: Always verify the checksum.
    // 
    // In KITL we dont support IPv6. So, skip if sum is zero.
    if(0x00 != pUDP->sum)
    {
        // Verify UDP header checksum
        xsum = htons(UDP_PROTOCOL);
        xsum = Sum(xsum, &srcip, sizeof(srcip));
        xsum = Sum(xsum, &destip, sizeof(destip));
        xsum = Sum(xsum, &pUDP->length, sizeof(uint32_t));
        xsum = Sum(xsum, pUDP, udpLength);
        if (xsum != pUDP->sum) goto cleanUp;
    }
        
    // Fix length
    length -= sizeof(UDP_HEADER);

    switch (pUDP->dstPort)
        {
        case htons(EDBG_DOWNLOAD_PORT):
            srcport = ntohs(pUDP->srcPort);
            memcpy(&id, &pUDP[1], sizeof(id));
            if (id == 'GBDE')
                {
                DecodeEDBG(pTransport, offset, length, srcmac, srcip, srcport);
                }
            else
                {
                DecodeTFTP(pTransport, offset, length, srcmac, srcip, srcport);
                }
            break;
        case htons(DHCP_CLIENT_PORT):
            DecodeDHCP(pTransport, offset, length, srcmac, srcip);
            break;
        }

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
DecodeICMP(
    Transport_t *pTransport,
    size_t offset,
    size_t length,
    const uint16_t srcmac[3],
    uint32_t srcip
    )
{
    ICMP_HEADER *pICMP;

    // Check length and checksum
    if (length < sizeof(ICMP_HEADER)) goto cleanUp;
    pICMP = (ICMP_HEADER*)(pTransport->packet + offset);
    if (Sum(0, pICMP, length) != 0xFFFF) goto cleanUp;

    // Reply to ping only
    if (pICMP->op != ICMP_ECHOREQ) goto cleanUp;

    // Encode reply & do checksum
    pICMP->op = 0;
    pICMP->code = 0;
    pICMP->sum = 0;
    pICMP->sum = ~ Sum(0, pICMP, length);

    // Add IP header & send
    SendIP(pTransport, offset, length, srcmac, srcip, ICMP_PROTOCOL);

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
DecodeIP(
    Transport_t *pTransport,
    size_t offset,
    size_t length,
    uint16_t srcmac[3]
    )
{
    IP4_HEADER *pIP;
    size_t ipLength;
    uint32_t srcip, destip;


    // Check & update offset and size
    if (length < sizeof(ARP_MESSAGE)) goto cleanUp;
    pIP = (IP4_HEADER*)(pTransport->packet + offset);
    offset += sizeof(IP4_HEADER);

    // We support only IP4 with standard IP4 header (no options...)
    if (pIP->verlen != 0x45) goto cleanUp;

    // First check if packet is for us
    if ((pIP->destip != pTransport->ip) &&
        (pIP->destip != 0xFFFFFFFF) && (pTransport->ip != 0))
        {
        goto cleanUp;
        }

    // Verify IP4 header checksum
    if (Sum(0, pIP, sizeof(IP4_HEADER)) != 0xFFFF) goto cleanUp;

    // We don't support IP packet fragmentation
    // Fragment if multiple fragments bit set or if fragment offset is nonzero
    if ((ntohs(pIP->fragment) & ~IP4_HEADER_FLAG_DF) != 0) 
        {
        //
        // IP addresses are still in network byte order
        //
        BOOTMSG(ZONE_WARN, (L"ERROR: DecodeIP: "
            L"Fragmented packet (%d.%d.%d.%d -> %d.%d.%d.%d) encountered!\r\n",
            ((uint8_t*)&pIP->srcip)[0],
            ((uint8_t*)&pIP->srcip)[1],
            ((uint8_t*)&pIP->srcip)[2],
            ((uint8_t*)&pIP->srcip)[3],
            ((uint8_t*)&pIP->destip)[0],
            ((uint8_t*)&pIP->destip)[1],
            ((uint8_t*)&pIP->destip)[2],
            ((uint8_t*)&pIP->destip)[3]
            ));
        goto cleanUp;
        }

    // Check length & update length for future processing
    ipLength = ntohs(pIP->length);
    if (length < ipLength) goto cleanUp;
    length = ipLength - sizeof(IP4_HEADER);

    // Then decode protocol
    switch (pIP->protocol)
        {
        case UDP_PROTOCOL:
            srcip = pIP->srcip;
            destip = pIP->destip;
            DecodeUDP(pTransport, offset, length, srcmac, srcip, destip);
            break;
        case ICMP_PROTOCOL:
            srcip = pIP->srcip;
            DecodeICMP(pTransport, offset, length, srcmac, srcip);
            break;
        }

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
DecodeARP(
    Transport_t *pTransport,
    size_t offset,
    size_t length,
    uint16_t srcmac[3]
    )
{
    ARP_MESSAGE *pARP;
    uint32_t srcip;


    // Check & update offset and size
    if (length < sizeof(ARP_MESSAGE)) goto cleanUp;
    pARP = (ARP_MESSAGE*)(pTransport->packet + offset);
    offset += sizeof(ARP_MESSAGE);
    length -= sizeof(ARP_MESSAGE);

    // Check to see that they were requesting the ARP response from us,
    // send reply to ARP request, but ignore other ops
    if (pARP->destip != pTransport->ip) goto cleanUp;

    switch (pARP->op)
        {
        case htons(ARP_REQUEST):
            srcip = pARP->srcip;
            SendARP(pTransport, srcmac, srcip, ARP_RESPONSE);
            break;
        }

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
DecodeFrame(
    Transport_t *pTransport,
    size_t offset,
    size_t length
    )
{
    ETH_HEADER *pEth = (ETH_HEADER*)(pTransport->packet + offset);
    uint16_t srcmac[3];


    // Check & update size and offset
    if (length < sizeof(ETH_HEADER)) goto cleanUp;
    length -= sizeof(ETH_HEADER);
    offset += sizeof(ETH_HEADER);

    // Process received packet
    switch (pEth->ftype)
        {
        case htons(ARP_FRAME):
            memcpy(srcmac, pEth->srcmac, sizeof(srcmac));
            DecodeARP(pTransport, offset, length, srcmac);
            break;
        case htons(IP_FRAME):
            memcpy(srcmac, pEth->srcmac, sizeof(srcmac));
            DecodeIP(pTransport, offset, length, srcmac);
            break;
        }

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
bool_t
RecvFrame(
    Transport_t *pTransport,
    size_t *pOffset,
    size_t *pSize
    )
{
    bool_t rc = false;
    uint32_t size;

    // Call Ethernet driver to get packet
    size = sizeof(pTransport->packet);
    if (!pTransport->edbg.pfnRecvFrame(pTransport->packet, &size))
        {
        *pOffset = 0;
        *pSize = 0;
        goto cleanUp;
        }

    *pOffset = 0;
    *pSize = size;

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
TryArpRequest(
    Transport_t *pTransport
    )
{
    bool_t rc = true;
    BootNotifyTransportEdbgArpRequest_t arpRequest;
    BootNotifyTransportEdbgArpResponse_t arpResponse;
    uint16_t mac[3];
    ETH_HEADER *pEth;
    ARP_MESSAGE *pARP = NULL;
    uint32_t startTime;


    // Send ARP request
    mac[0] = mac[1] = mac[2] = 0xFFFF;
    SendARP(pTransport, mac, pTransport->ip, ARP_REQUEST);

    arpRequest.ip = pTransport->ip;
    OEMBootNotify(
        pTransport->pContext, BOOT_NOTIFY_TRANSPORT_EDBG_ARP_REQUEST,
        &arpRequest, sizeof(arpRequest)
        );

    // Wait for resolve...
    startTime = OEMBootGetTickCount();
    while ((OEMBootGetTickCount() - startTime) < pTransport->arpTimeout)
        {
        size_t offset, length;

        if (!RecvFrame(pTransport, &offset, &length))
            {
            continue;
            }

        // Check if this is reply to request
        if (length < (sizeof(ETH_HEADER) + sizeof(ARP_MESSAGE)))
            {
            continue;
            }
        pEth = (ETH_HEADER*)(pTransport->packet + offset);
        if (pEth->ftype != htons(ARP_FRAME))
            {
            continue;
            }
        pARP = (ARP_MESSAGE*)((uint8_t *)pEth + sizeof(ETH_HEADER));
        if (pARP->op != htons(ARP_RESPONSE))
            {
            continue;
            }
        if (pARP->destip != pTransport->ip)
            {
            continue;
            }
        if ((pARP->srcmac[0] == pTransport->mac[0]) &&
            (pARP->srcmac[1] == pTransport->mac[1]) &&
            (pARP->srcmac[2] == pTransport->mac[2]))
            {
            continue;
            }

        // Oops, somebody is using assigned address...
        rc = false;
        break;
        }

    // Notify about result
    arpResponse.timeout = rc;
    arpResponse.ip = pTransport->ip;
    if (pARP)
        {
        memcpy(arpResponse.mac, pARP->destmac, sizeof(arpResponse.mac));
        }
    else
        {
        memset(arpResponse.mac, 0, sizeof(arpResponse.mac));
        }
    
    OEMBootNotify(
        pTransport->pContext, BOOT_NOTIFY_TRANSPORT_EDBG_ARP_RESPONSE,
        &arpResponse, sizeof(arpResponse)
        );
    
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
GetAddressByDHCP(
    Transport_t *pTransport
    )
{
    uint32_t startTime, attempts;


    // Allocate new IP address
    attempts = 0;
    pTransport->dhcpXId = OEMBootGetTickCount() ^ 0x17012511;

    while (attempts++ < pTransport->dhcpRetry)
        {
        BootNotifyTransportEdbgDhcpDiscover_t dhcpDiscover;
        BootNotifyTransportEdbgDhcpBound_t dhcpBound;
    
        // Reset device and server IP
        pTransport->ip = 0;
        pTransport->dhcpServerIP = 0;

        // Send DHCP discover message
        SendDHCP(pTransport, DHCP_DISCOVER, 0);
        pTransport->dhcpState = DHCP_SELECTING;

        // Let user now...
        dhcpDiscover.attempt = attempts;
        OEMBootNotify(
            pTransport->pContext, BOOT_NOTIFY_TRANSPORT_EDBG_DHCP_DISCOVER,
            &dhcpDiscover, sizeof(dhcpDiscover)
            );
        
        // Wait until DHCP gets address or timeout
        startTime = OEMBootGetTickCount();
        while ((OEMBootGetTickCount() - startTime) < pTransport->dhcpTimeout)
            {
            size_t offset, length;

            if (!RecvFrame(pTransport, &offset, &length)) continue;
            DecodeFrame(pTransport, offset, length);
            if (pTransport->dhcpState == DHCP_BOUND) break;
            }

        // If there was timeout try start DHCP againg with new transaction id
        if (pTransport->dhcpState != DHCP_BOUND)
            {
            pTransport->dhcpXId += 0x00080000;
            continue;
            }

        // Notify about obtained address
        dhcpBound.serverIp = pTransport->dhcpServerIP;
        dhcpBound.clientIp = pTransport->ip;
        OEMBootNotify(
            pTransport->pContext, BOOT_NOTIFY_TRANSPORT_EDBG_DHCP_BOUND,
            &dhcpBound, sizeof(dhcpBound)
            );
        
        // We get address, verify if it isn't used by somebody else...
        if (TryArpRequest(pTransport)) break;

        // Somebody else have it, decline address on server
        SendDHCP(pTransport, DHCP_DECLINE, pTransport->ip);

        // Try new DHCP transaction
        pTransport->dhcpXId += 0x01080000;

        }

#if 0
    // Apparently there ins't DHCP server, let fall back to auto-IP
    attempts = 0;
    while (attempts++ < 9)
        {
        }
#endif

    return (pTransport->ip != 0);
}

//------------------------------------------------------------------------------

static
bool_t
WaitForConnection(
    Transport_t *pTransport
    )
{
    enum_t count;
    uint32_t time;

    count = 0;
    while (count++ < pTransport->bootMeRetry)
        {
        BootNotifyTransportEdbgBootMe_t bootMe;
    
        time = OEMBootGetTickCount();

        // Send packet
        SendBootMe(pTransport);

        // Notify user
        bootMe.name = pTransport->name;
        bootMe.attempt = count;
        OEMBootNotify(
            pTransport->pContext, BOOT_NOTIFY_TRANSPORT_EDBG_BOOTME,
            &bootMe, sizeof(bootMe)
            );
        
        while ((OEMBootGetTickCount() - time) < pTransport->bootMeDelay)
            {
            size_t length, offset;

            // Get next packet
            if (!RecvFrame(pTransport, &offset, &length)) continue;
            // Decode received packet
            DecodeFrame(pTransport, offset, length);
            // Did TFTP transfer started?
            if (pTransport->tftpState == TFTP_STATE_IDLE) continue;
            // We are in different state
            break;
            }
        if (pTransport->tftpState != TFTP_STATE_IDLE) break;
        }

    return (pTransport->tftpState != TFTP_STATE_IDLE);
}

//------------------------------------------------------------------------------

static
bool_t
Read(
    Transport_t *pTransport,
    VOID *pBuffer,
    size_t bufferLength
    )
{
    bool_t rc = false;
    uint32_t time;


    // If there isn't device IP we should use DHCP to get it...
    if ((pTransport->ip == 0) && !GetAddressByDHCP(pTransport))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportPb: "
            L"Failed get IP address from DHCP server!\r\n"
            ));
        goto cleanUp;
        }

    // If there isn't desktop IP we should initialize connection
    if (pTransport->tftpIP == 0)
        {
        BootNotifyTransportEdbgServerAck_t serverAck;
        
        if (!WaitForConnection(pTransport))
            {
            BOOTMSG(ZONE_INFO, (
                L"No download server found, exiting...\r\n"
                ));
            goto cleanUp;
            }
        
        serverAck.serverIp = pTransport->tftpIP;
        serverAck.serverPort = pTransport->tftpPort;
        OEMBootNotify(
            pTransport->pContext, BOOT_NOTIFY_TRANSPORT_EDBG_SERVER_ACK,
            &serverAck, sizeof(serverAck)
            );
        
        }

    // Get data
    time = OEMBootGetTickCount();
    while (bufferLength > 0)
        {
        size_t length, offset;

        // Check for timeout
        if ((OEMBootGetTickCount() - time) >= pTransport->tftpTimeout)
        {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportPb: "
                L"TFTP timeout occured!\r\n"
                ));
            break;
        }

        // Copy existing data
        if (pTransport->tftpLength > 0)
            {
            length = min(pTransport->tftpLength, bufferLength);
            offset = pTransport->tftpOffset;
            memcpy(pBuffer, &pTransport->packet[offset], length);
            pBuffer = (UINT8*)pBuffer + length;
            bufferLength -= length;
            pTransport->tftpOffset += length;
            pTransport->tftpLength -= length;
            pTransport->readPos += length;
            if (pTransport->tftpLength > 0) continue;
            SendTFTPAck(pTransport);
            }

        // Wait for last packet
        if (pTransport->tftpState == TFTP_STATE_WRITE_LAST)
            {
            time = OEMBootGetTickCount();
            while (pTransport->tftpState != TFTP_STATE_JUMP)
                {
                // Check for timeout
                if ((OEMBootGetTickCount() - time) >= pTransport->tftpTimeout)
                {
                    BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportPb: "
                        L"TFTP timeout occured!\r\n"
                        ));
                    break;
                }
                // Get next packet
                if (!RecvFrame(pTransport, &offset, &length)) continue;
                // Decode received packet
                DecodeFrame(pTransport, offset, length);
                }            
            }
        else
            {
            // Get next packet
            if (!RecvFrame(pTransport, &offset, &length)) continue;
            // Decode received packet
            DecodeFrame(pTransport, offset, length);
            }

        // If we get data reset watchdog and send notification
        if (pTransport->tftpLength > 0)
            {
            BootNotifyTransportEdbgServerData_t serverData;

            serverData.data = pTransport->tftpLength;
            OEMBootNotify(
                pTransport->pContext, BOOT_NOTIFY_TRANSPORT_EDBG_SERVER_DATA,
                &serverData, sizeof(serverData)
                );
            
            time = OEMBootGetTickCount();
            }
        }

    // Special JUMP only download 
    if ((bufferLength > 0) && (pTransport->tftpState == TFTP_STATE_JUMP) &&
        (pTransport->readPos < (sizeof(BOOT_BIN_SIGNATURE_JUMP) - 1)))
        {
        const uint8_t *pSignature = (const uint8_t *)BOOT_BIN_SIGNATURE_JUMP;
        size_t length;

        length = sizeof(BOOT_BIN_SIGNATURE_JUMP) - 1 - pTransport->readPos;
        memcpy((uint8_t *)pBuffer, &pSignature[pTransport->readPos], length);
        pBuffer = (uint8_t *)pBuffer + length;
        bufferLength -= length;
        pTransport->readPos += length;
        }

    // When we don't have all data at this moment, something wrong happen
    if (bufferLength > 0)
        {
        goto cleanUp;
        }

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------


static
bool_t
ReadOsConfig(
    Transport_t *pTransport,
    enum_t type,
    VOID *pBuffer,
    size_t bufferLength
    )
{
    bool_t rc = false;

    // If we the Os Config info isn't available yet return an error
    if( !pTransport->osConfigInfo.bValid ) goto cleanUp;
       
    switch(type)
        {
        case BOOT_TRANSPORT_READ_OS_CONFIG_TYPE_PASSIVE_KITL:
            if( pBuffer != NULL &&
                    bufferLength == sizeof(pTransport->osConfigInfo.bPassiveKitl) )
                {
                // Return the value passed for Passive Kitl to the caller
                *((bool_t*)pBuffer) = pTransport->osConfigInfo.bPassiveKitl;
                rc = true;
                }
            break;
        case BOOT_TRANSPORT_READ_OS_CONFIG_TYPE_KITL_TRANSPORT:
            if( pBuffer != NULL &&
                bufferLength == sizeof(pTransport->osConfigInfo.KitlTransportMediaType))
                {
                *((uint32_t*)pBuffer) = pTransport->osConfigInfo.KitlTransportMediaType;
                rc = true;
                }
            break;
            
        default:
            rc = false;           
        }                 

cleanUp:
    return rc;
}


