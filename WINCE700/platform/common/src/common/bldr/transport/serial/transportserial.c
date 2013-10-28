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
#include <bootTransportSerial.h>
#include <bootTransportEdbgNotify.h>
#include <bootString.h>
#include <bootDownloadBinFormat.h>
#include <bootCore.h>
#include <bootMemory.h>
#include <bootLog.h>
#include "serial.h"

#define KTS_PASSIVE_MODE 0x40       // flag for passive kitl mode
#define KTS_KITL_TRANSPORT_MEDIA_MASK 0x3f


//------------------------------------------------------------------------------

#define SERIAL_DMA_BUFFER_SIZE        0x20000
#define SERIAL_JUMP_DELAY                15000    // 15 seconds
//------------------------------------------------------------------------------

typedef struct OSConfigInfoFromJump_t {
    bool_t bValid;      // Set true when the Jump Command is recieved 
                        //      and the rest of the structure is filled in
    bool_t bPassiveKitl;
    uint32_t KitlTransportMediaType; 
} OSConfigInfoFromJump_t;

typedef struct Transport_t {

    BootDriverVTable_t *pVTable;

    void*   pContext;
    
    BootSerialDriver_t serial;
    
    uint8_t* pDmaBuffer;
    uint32_t dmaBufferSize;

    uint32_t bootMeRetry;
    uint32_t bootMeDelay;
    uint32_t recvTimeout;

    uint8_t  packetBuffer[KITL_MTU];
    uint16_t dataLength;
    // the first DWORD in the local buffer is the block header which contains
    // the sequence number of the block received
    SERIAL_BLOCK_HEADER *pBlockHeader;
    uint8_t* pBlockData;

    uint16_t mac[3];

    char     name[KITL_MAX_DEV_NAMELEN+1];
    char     unused[3];
    
    bool_t   bConnected;
    uint32_t blockNumber;

    OSConfigInfoFromJump_t osConfigInfo;    
} Transport_t;


//------------------------------------------------------------------------------
//  Local functions

bool_t
BootTransportSerialDeinit(
    void *pContext
    );

bool_t
BootTransportSerialIoCtl(
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

static
bool_t
ReadJumpPkt(
    Transport_t *pTransport
    );
    
//------------------------------------------------------------------------------

static
BootDriverVTable_t
s_transportVTable = {
    BootTransportSerialDeinit,
    BootTransportSerialIoCtl
};

static
Transport_t
s_transport;

//------------------------------------------------------------------------------

void*
BootTransportSerialInit(
    void *pContext,
    const BootSerialDriver_t *pDriver,
    pfnBootTransportSerialName_t pfnName,
    uint8_t *pAddress,
    uint32_t offset,
    uint16_t mac[3],
    uint32_t ip
    )
{
    void *pTransport = NULL;
    
    UNREFERENCED_PARAMETER(pAddress);
    UNREFERENCED_PARAMETER(offset);
    UNREFERENCED_PARAMETER(ip);

    // This driver shouldn't be instantied twice...
    if (s_transport.pVTable != NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportSerialInit: "
            L"Driver already initialized!\r\n"
            ));
        goto cleanUp;
        }

    memset(&s_transport, 0, sizeof(s_transport));

    // Initialize hardware
    if (pDriver->pfnDmaInit != NULL)
        {
        s_transport.dmaBufferSize = SERIAL_DMA_BUFFER_SIZE;
        s_transport.pDmaBuffer = BootAlloc(s_transport.dmaBufferSize);
        if (s_transport.pDmaBuffer == NULL)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportSerialInit: "
                L"Failed allocate DMA buffer!\r\n"
                ));
            goto cleanUp;
            }
        if (!pDriver->pfnDmaInit(
                (uint32_t)s_transport.pDmaBuffer, s_transport.dmaBufferSize
                ))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportSerialInit: "
                L"Serial driver DMA initialization failed!\r\n"
                ));
            goto cleanUp;
            }
        }
    
    if (!pDriver->pfnInit(NULL))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportSerialInit: "
            L"Serial driver initialization failed!\r\n"
            ));
        goto cleanUp;
        }

    // Create device name
    if (!pfnName(pContext, mac, s_transport.name, dimof(s_transport.name)))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportSerialInit: "
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
    s_transport.recvTimeout = 5000;

    // the first DWORD in the local buffer is the block header which contains
    // the sequence number of the block received
    s_transport.pBlockHeader = (SERIAL_BLOCK_HEADER *)s_transport.packetBuffer;
    s_transport.pBlockData = s_transport.packetBuffer + sizeof(SERIAL_BLOCK_HEADER);

    // Save driver & macto transport context
    memcpy(&s_transport.serial, pDriver, sizeof(s_transport.serial));
    memcpy(s_transport.mac, mac, sizeof(s_transport.mac));

    // Done
    pTransport = &s_transport;

cleanUp:
    return pTransport;
}

//------------------------------------------------------------------------------

bool_t
BootTransportSerialDeinit(
    void *pContext
    )
{
    bool_t rc = false;
    Transport_t *pTransport = pContext;


    // Check driver handle
    if (pTransport != &s_transport)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportSerialDeinit: "
            L"Invalid driver handle!\r\n"
            ));
        goto cleanUp;
        }

    // Deinitialize Serial driver
    if (pTransport->serial.pfnDeinit != NULL) pTransport->serial.pfnDeinit();
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
BootTransportSerialIoCtl(
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
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportSerialIoCtl: "
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
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportSerialIoCtl: "
                    L"Invalid BOOT_TRANSPORT_IOCTL_READ parameter!\r\n"
                    ));
                break;
                }
            rc = Read(pTransport, pParams->pBuffer, pParams->size);
            }
            break;
        case BOOT_TRANSPORT_IOCTL_GET_OS_CONFIG_INFO:
            {
            BootTransportGetOsConfigInfoParams_t* pParams = pBuffer;
            if((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportSerialIoCtl: "
                    L"Invalid BOOT_TRANSPORT_IOCTL_GET_OS_CONFIG_INFO parameter!\r\n"
                    ));
                break;
                }
            rc = ReadOsConfig(pTransport, pParams->type, pParams->pBuffer, pParams->BufferSize);
            }
            break;
        case BOOT_TRANSPORT_SERIAL_IOCTL_OPTIONS:
            {
            BootTransportSerialOptionsParams_t* pParams = pBuffer;
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootTransportSerialIoCtl: "
                    L"Invalid BOOT_TRANSPORT_SERIAL_IOCTL_OPTIONS parameter!\r\n"
                    ));
                break;
                }
            if (pParams->bootMeDelay != -1)
                pTransport->bootMeDelay = pParams->bootMeDelay;
            if (pParams->bootMeRetry != -1)
                pTransport->bootMeRetry = pParams->bootMeRetry;
            if (pParams->recvTimeout != -1)
                pTransport->recvTimeout = pParams->recvTimeout;
            rc = TRUE;
            }
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
uint8_t
Sum(uint8_t *pBuf, uint16_t len)
//------------------------------------------------------------------------------
{
    uint16_t s = 0;
    uint8_t sum = 0;

    for(s = 0; s < len; s++)
        sum += *(pBuf + s);
    return sum;
}

//------------------------------------------------------------------------------

static
VOID
SendPacket(
    Transport_t *pTransport,
    uint8_t pktType,
    uint8_t *pBuffer,
    size_t length
    )
{
    SERIAL_PACKET_HEADER *pHeader;
    uint8_t *pData;

    if (length > KITL_MTU) goto cleanUp;

    pHeader = (SERIAL_PACKET_HEADER *)pBuffer;
    pData = pBuffer + sizeof(SERIAL_PACKET_HEADER);

    // Fill in Serial header
    memcpy(pHeader->headerSig, packetHeaderSig, HEADER_SIG_BYTES);
    pHeader->pktType = pktType;
    pHeader->payloadSize = (USHORT)length;
    pHeader->crcData = Sum((uint8_t *)pData, (uint16_t)length);
    pHeader->crcHdr = Sum((uint8_t *)pHeader, 
        sizeof(SERIAL_PACKET_HEADER) - sizeof(pHeader->crcHdr));

    length += sizeof(SERIAL_PACKET_HEADER);

    // Block until send is complete, no timeout
    while(!pTransport->serial.pfnSendFrame(pBuffer, (uint16_t)length));

cleanUp:
    return;
}

//------------------------------------------------------------------------------

static
VOID
SendBootMe(
    Transport_t *pTransport
    )
{
    uint8_t pBuffer[sizeof(SERIAL_PACKET_HEADER) + sizeof(SERIAL_BOOT_REQUEST)];
    SERIAL_BOOT_REQUEST *pData;

    pData = (SERIAL_BOOT_REQUEST *)(pBuffer + sizeof(SERIAL_PACKET_HEADER));

   // create boot request
    memset(pBuffer, 0, sizeof(SERIAL_PACKET_HEADER) + sizeof(SERIAL_BOOT_REQUEST));
    strncpy_s((char *)pData->PlatformId, KITL_MAX_DEV_NAMELEN+1, pTransport->name, KITL_MAX_DEV_NAMELEN);
    strncpy_s((char *)pData->DeviceName, KITL_MAX_DEV_NAMELEN+1, pTransport->name, KITL_MAX_DEV_NAMELEN);

    SendPacket(
        pTransport, 
        KS_PKT_DLREQ, 
        pBuffer,
        sizeof(SERIAL_BOOT_REQUEST)
        );
}

//------------------------------------------------------------------------------

static
VOID
SendBlockAck(
    Transport_t *pTransport
    )
{
    uint8_t pBuffer[sizeof(SERIAL_PACKET_HEADER) + sizeof(SERIAL_BLOCK_HEADER)];
    SERIAL_BLOCK_HEADER *pData;
    uint32_t blockAckNumber = pTransport->blockNumber;
                
    if (blockAckNumber > 0) blockAckNumber--; // we are acknowledging the previous block

    pData = (SERIAL_BLOCK_HEADER *)(pBuffer + sizeof(SERIAL_PACKET_HEADER));

    // create block ack
    memset(pBuffer, 0, sizeof(SERIAL_PACKET_HEADER) + sizeof(SERIAL_BLOCK_HEADER));
    memcpy(&pData->uBlockNum, &blockAckNumber, sizeof(SERIAL_BLOCK_HEADER));

    SendPacket(
        pTransport, 
        KS_PKT_DLACK, 
        pBuffer,
        sizeof(SERIAL_BLOCK_HEADER)
        );
}

//------------------------------------------------------------------------------

static
bool_t
RecvFrame(
    Transport_t *pTransport,
    uint8_t *pBuffer,
    uint16_t *pSize,
    bool_t bWaitInfinite
    )
{
    bool_t rc = FALSE;
    uint8_t *pReadBuffer = pBuffer;
    uint16_t recvSize = 0;
    uint16_t cbToRead = *pSize;
    uint32_t tStart = 0;

    if (!bWaitInfinite) tStart = OEMBootGetTickCount();

    *pSize = 0;
    do
    {
        // Call Serial driver to get packet   
        recvSize = pTransport->serial.pfnRecvFrame(pReadBuffer, cbToRead);
        if (recvSize) 
        {
            pReadBuffer += recvSize;
            cbToRead -= recvSize;
            *pSize += recvSize;
        }
    }
    while (cbToRead && 
          (bWaitInfinite || OEMBootGetTickCount() - tStart <= pTransport->recvTimeout));

    if (*pSize != 0)
        rc = TRUE;

    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
RecvHeader(
    Transport_t *pTransport,
    SERIAL_PACKET_HEADER *pHeader,
    bool_t bWaitInfinite
    )
{
    bool_t rc = FALSE;
    uint32_t i = 0;
    uint16_t length = sizeof(uint8_t);
    uint8_t *pHeaderBuf = (uint8_t *)pHeader;

    // hunt for packet header signature
    while (i < HEADER_SIG_BYTES) 
        {
        if (!RecvFrame(pTransport, &pHeaderBuf[i], &length, bWaitInfinite) || 
            sizeof(uint8_t) != length)
            {       
            goto cleanUp;
            }        
            
        if (pHeaderBuf[i] == packetHeaderSig[i])
            {
            i++;
            }
        else 
            {
            i = 0;
            }
        }
   
    // read the remaining header
    length = sizeof(SERIAL_PACKET_HEADER) - HEADER_SIG_BYTES;
    if (!RecvFrame(pTransport, pHeaderBuf + HEADER_SIG_BYTES, &length, bWaitInfinite) ||
        sizeof(SERIAL_PACKET_HEADER) - HEADER_SIG_BYTES != length)
        {
        BOOTMSG(ZONE_WARN, (L"WARN: RecvHeader: "
                    L"Failed to receive header data!\r\n"
                    ));
        goto cleanUp;
        }

    // verify the header checksum
    if (pHeader->crcHdr != Sum(pHeaderBuf, 
        sizeof(SERIAL_PACKET_HEADER) - sizeof(pHeader->crcHdr)))
        {
        BOOTMSG(ZONE_WARN, (L"WARN: RecvHeader: "
                    L"header checksum failure!\r\n"
                    ));
        goto cleanUp;
        }    

    rc = TRUE;
  
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
RecvPacket(
    Transport_t *pTransport,
    SERIAL_PACKET_HEADER *pHeader,
    uint8_t *pBuffer,
    uint16_t *pLength,
    bool_t bWaitInfinite
    )
{
    bool_t rc = FALSE;

    // receive header
    if(!RecvHeader(pTransport, pHeader, bWaitInfinite))
    {
        goto cleanUp;
    }

    // make sure sufficient buffer is provided
    if(KITL_MTU < pHeader->payloadSize)
    {
        BOOTMSG(ZONE_ERROR, (L"ERROR: insufficient buffer size; ignoring packet!\r\n"));
        goto cleanUp;
    }

    // read the rest of the packet
    *pLength = pHeader->payloadSize;
    if(!RecvFrame(pTransport, pBuffer, pLength, bWaitInfinite))       
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: RecvHeader: "
                    L"Failed to receive header data!\r\n"
                    ));
        goto cleanUp;
        }

    // verify packet type -- don't return any packet that is not
    // a type the bootloader expects to receive
    if(KS_PKT_DLPKT != pHeader->pktType &&
       KS_PKT_DLACK != pHeader->pktType &&
       KS_PKT_JUMP  != pHeader->pktType)
    {
        BOOTMSG(ZONE_WARN, (L"WARN: CheckFrame: "
            L"Received non-download packet type! (0x%02X)\r\n",pHeader->pktType
            ));
        goto cleanUp;
    }

    // verify data checksum
    if(pHeader->crcData != Sum(pBuffer, *pLength))
    {
        BOOTMSG(ZONE_WARN, (L"WARN: CheckFrame: "
            L"Invalid packet data checksum! %u\r\n",
            *pLength));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
WaitForBootAck(
    Transport_t *pTransport,
    bool_t *pbJump
    )
{
    bool_t rc = FALSE;
    enum_t count = 0;
    uint32_t tStart;
    SERIAL_PACKET_HEADER header;
    uint8_t buffer[KITL_MTU];
    uint16_t length;

    while (count++ < pTransport->bootMeRetry)
        {   
        BootNotifyTransportEdbgBootMe_t bootMe;
    
        tStart = OEMBootGetTickCount();

        // Send packet
        SendBootMe(pTransport);

        // Notify user
        bootMe.name = pTransport->name;
        bootMe.attempt = count;
        OEMBootNotify(
            pTransport->pContext, BOOT_NOTIFY_TRANSPORT_EDBG_BOOTME,
            &bootMe, sizeof(bootMe)
            );

        while ((OEMBootGetTickCount() - tStart) < pTransport->bootMeDelay)
            {
            // Get next packet
            if (RecvPacket(pTransport, &header, buffer, &length, FALSE)) 
                {
                if (KS_PKT_DLACK == header.pktType && 
                    sizeof(SERIAL_BOOT_ACK) == header.payloadSize)
                    {
                    BOOTMSG(ZONE_INFO, (L"INFO: Received boot ack\r\n:"));
                    *pbJump = ((SERIAL_BOOT_ACK *)buffer)->fJumping;

                    pTransport->blockNumber = 0;
                    SendBlockAck(pTransport);

                    rc = TRUE;
                    goto cleanUp;          
                    }   
                }            
            }
        }

cleanUp:
    return rc;
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
    bool_t bJump = FALSE;
    SERIAL_PACKET_HEADER header;
    uint16_t packetLength;

    // If there isn't desktop IP we should initialize connection
    if (!pTransport->bConnected)
        {
        if (!WaitForBootAck(pTransport, &bJump))
            {
            BOOTMSG(ZONE_INFO, (
                L"No download server found, exiting...\r\n"
                ));
            goto cleanUp;
            }
        
        if (bJump) goto cleanUp; // special jump only download

        pTransport->bConnected = TRUE;
        }

    // Get data
    while (bufferLength > 0)
        {
        while (0 == pTransport->dataLength)
            {
            packetLength = sizeof(pTransport->packetBuffer);
            if (RecvPacket(pTransport, &header, pTransport->packetBuffer, &packetLength, TRUE))
                {
                // ignore non-download packet types
                if(KS_PKT_DLPKT == header.pktType)
                    {                    
                    // make sure we received the correct block in the sequence
                    if(pTransport->blockNumber == pTransport->pBlockHeader->uBlockNum)
                        {
                        pTransport->dataLength = header.payloadSize - sizeof(SERIAL_BLOCK_HEADER);
                        pTransport->blockNumber++;
                        }
                    else
                        {
                        BOOTMSG(ZONE_WARN, (L"WARN: Received out of sequence block %u. Expected block %u\r\n", 
                            pTransport->pBlockHeader->uBlockNum, 
                            pTransport->blockNumber));
                        }
                    
                    // ack, or re-ack the sender
                    if(pTransport->blockNumber > 0)
                        {
                        SendBlockAck(pTransport);
                        }
                    }  
                }                
            }

        // copy from local buffer into output buffer

        // if there are more than the requested bytes, copy and shift
        // the local data buffer
        if (pTransport->dataLength > bufferLength)
            {   
            // copy requested bytes from local buffer into output buffer            
            memcpy((uint8_t *)pBuffer, pTransport->pBlockData, bufferLength);
            pTransport->dataLength -= (uint16_t)bufferLength;

            // shift the local buffer accordingly because not all data was used
            memmove(pTransport->pBlockData, pTransport->pBlockData + bufferLength, pTransport->dataLength);
            bufferLength = 0;            
            }
        else // length <= bufferLength
            {
            // copy all bytes in local buffer to output buffer
            memcpy((uint8_t *)pBuffer, pTransport->pBlockData, pTransport->dataLength);
            bufferLength -= pTransport->dataLength;
            pBuffer = (uint8_t *)pBuffer + pTransport->dataLength;
            pTransport->dataLength = 0;
            }
        }

    // When we don't have all data at this moment, something wrong happen
    if (bufferLength > 0)
        {
        goto cleanUp;
        }

    // Done
    rc = true;

cleanUp:

    // Special JUMP only download 
    if (bJump)
        {    
        // sizeof(BOOT_BIN_SIGNATURE_JUMP) includes the terminating NULL character, 
        // so subtract 1 from it.
        if (bufferLength < sizeof(BOOT_BIN_SIGNATURE_JUMP) - 1) 
            {
            BOOTMSG(ZONE_ERROR, 
                (L"ERROR: Insufficient buffer size for special jump header!\r\n"));
            }
        else
            {
            const uint8_t *pSignature = (const uint8_t *)BOOT_BIN_SIGNATURE_JUMP;
            memcpy(pBuffer, pSignature, sizeof(BOOT_BIN_SIGNATURE_JUMP) - 1);
            rc = TRUE;
            }
        }

    return rc;
}


//------------------------------------------------------------------------------


static
bool_t
ReadJumpPkt(
    Transport_t *pTransport
    )
{
    bool_t rc = false;
    SERIAL_PACKET_HEADER header;
    uint16_t packetLength;
    uint32_t tStart;
    
    tStart = OEMBootGetTickCount();
    
    // Wait until you get jump packet or timeout expires
    while ((OEMBootGetTickCount() - tStart) < SERIAL_JUMP_DELAY)
        {
        packetLength = sizeof(pTransport->packetBuffer);
        if (RecvPacket(pTransport, &header, pTransport->packetBuffer, &packetLength, TRUE))
            {
            // ignore non-jump packet types
            if (KS_PKT_JUMP == header.pktType && 
                    sizeof(SERIAL_JUMP_REQUEST) == header.payloadSize)
                {
                    // Get the KitlTransport options out of jump packet payload
                    // The rest of the OSConfig data isn't valid within an EDBG_CMD_JUMP packet      
                    SERIAL_JUMP_REQUEST *pJumpRequest = (SERIAL_JUMP_REQUEST *)(&(pTransport->packetBuffer));
                    if( pJumpRequest->dwKitlTransport & KTS_PASSIVE_MODE )
                        {
                        pTransport->osConfigInfo.bPassiveKitl = true;
                        }
                  
                    pTransport->osConfigInfo.KitlTransportMediaType = 
                        pJumpRequest->dwKitlTransport & KTS_KITL_TRANSPORT_MEDIA_MASK;
                    
                    pTransport->osConfigInfo.bValid = true;  // The OsConfigInfo is now valid
                
                    pTransport->blockNumber = 0;
                    SendBlockAck(pTransport);
                    rc = TRUE;
                    
                    goto cleanUp;
                }
            }   
        }

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
    bool_t rc = FALSE;
        
    // If the Os Config info isn't available yet - wait for it
    if( !pTransport->osConfigInfo.bValid ) ReadJumpPkt(pTransport);
       
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

    return rc;
}


//------------------------------------------------------------------------------
