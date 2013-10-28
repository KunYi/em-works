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
   edbg.c

Abstract:
   Routines to communicate with eshell program on the desktop to get our
   configuration information (which services are configured over Ethernet,
   and what the host addresses are).

Functions:


Notes:

--*/

#include <windows.h>
#include <halether.h>
#include <ethdbg.h>

#include "udp.h"

/*
 * ProcessEDBG
 *
 *  Receive handler to process messages from eshell program.
 *
 *  Return value:
 *   Return TRUE if message was valid edbg msg, else FALSE
 */
BOOL
EbootProcessEDBG(
    EDBG_ADDR *pMyAddr,     // IN - Our IP/ethernet address
    EDBG_ADDR *pEshellHost, // IN - Host running eshell program, if known. We will only accept
                            //      commands from this host.
                            // OUT- If valid cmd received, filled in with eshell host addr
    BYTE   *pFrameBuffer,   // IN - Pointer to frame buffer (for extracting src IP)
    UINT16 *pwUDPData,      // IN - Recvd UDP data
    UINT16 cwUDPDataLength, // IN - Len of recvd data
    BOOL   *pfJump,         // OUT - Set to TRUE if JUMPIMG command received
    EDBG_OS_CONFIG_DATA **ppCfgData) // OUT - If pfJump is set, this will contain a pointer to the
                                     // configuration paramters passed from eshell
{
    ETH_DBG_HDR *pHdr = (ETH_DBG_HDR *)pwUDPData;
    EDBG_ADDR SrcAddr;

    UNREFERENCED_PARAMETER(cwUDPDataLength);

    *pfJump = FALSE;

    if (pHdr->Id != EDBG_ID)
        return FALSE;

    // Toss all except admin messages
    if (pHdr->Service != EDBG_SVC_ADMIN)
        return TRUE;

    // Check and see if anyone has started a download.  If so, don't accept
    // any commands from different hosts.
    SrcAddrFromFrame(&SrcAddr,pFrameBuffer);
    if (pEshellHost && pEshellHost->dwIP && (pEshellHost->dwIP != SrcAddr.dwIP)) {
        char PeerIPString[16];  // inet_ntoa uses a static buffer
        strncpy_s(PeerIPString, sizeof(PeerIPString), inet_ntoa(SrcAddr.dwIP), 15);
        PeerIPString[15] = '\0';
        KITLOutputDebugString("TFTP download started previously by host %s, ignoring cmd from %s\r\n",
                           inet_ntoa(pEshellHost->dwIP), PeerIPString);
        return TRUE;
    }

    switch (pHdr->Cmd)
    {
        case EDBG_CMD_JUMPIMG:
            KITLOutputDebugString("Got EDBG_CMD_JUMPIMG\r\n");
            *pfJump = TRUE;

            // Set field in driver globals indicating address of host that is starting us
            if (pEshellHost)
                memcpy(pEshellHost, &SrcAddr, sizeof(EDBG_ADDR));

            // Fall through (data field is same as CONFIG command)

        case EDBG_CMD_OS_CONFIG:
        {
            KITLOutputDebugString("Got EDBG_CMD_CONFIG, flags:0x%X\r\n",
                                  ((PEDBG_OS_CONFIG_DATA)pHdr->Data)->Flags);

            if (ppCfgData)
                *ppCfgData = (PEDBG_OS_CONFIG_DATA)pHdr->Data;
        }
        break;

        default:
            //KITLOutputDebugString("Unrecognized EDBG cmd: 0x%X\r\n",pHdr->Cmd);
            return TRUE;
    }

    // Respond with ack
    {
        BYTE AckBuf[100];
        ETH_DBG_HDR *pAckHdr = (ETH_DBG_HDR *)(AckBuf+UDP_DATA_FRAME_OFFSET);
        EDBG_ADDR HostAddr;
        UINT16 AckLen = EDBG_DATA_OFFSET;

        memcpy(pAckHdr, pwUDPData, EDBG_DATA_OFFSET);
        pAckHdr->Flags = EDBG_FL_FROM_DEV | EDBG_FL_ACK;

        // Copy the address from the received frame
        SrcAddrFromFrame(&HostAddr, pFrameBuffer);
        pMyAddr->wPort = HostAddr.wPort;

        if (!EbootSendUDP(AckBuf, &HostAddr, pMyAddr, (BYTE *)pAckHdr, AckLen))
            KITLOutputDebugString("Error in SendUDP\r\n");
    }

    return TRUE;
}


/*
 * SendBootme
 *
 *   Send a broadcast UDP packet to identify us as available to receive images
 *   Later, possibly investigate use of multicast instead, to reduce overhead
 *   for machines that don't care about these frames, and to extend the reach
 *   of these packets off the subnet.
 */
static UCHAR BootmeSeqNum = 0;

void
EbootSendBootme(EDBG_ADDR *pMyAddr, UCHAR VersionMajor, UCHAR VersionMinor, char *szPlatformString, char *szDeviceName, UCHAR CPUId, DWORD dwBootFlags)
{
    EDBG_ADDR DestAddr;
    UCHAR BootmeBuf[sizeof(ETH_DBG_HDR)+sizeof(EDBG_BOOTME_DATA)+UDP_DATA_FRAME_OFFSET];
    char DevNumBuf[6];
    ETH_DBG_HDR      *pHdr = (ETH_DBG_HDR *)(BootmeBuf + UDP_DATA_FRAME_OFFSET);
    EDBG_BOOTME_DATA *pData = (EDBG_BOOTME_DATA *)(pHdr->Data);

    // Format BOOTME message
    pHdr->Id = EDBG_ID;
    pHdr->Service = EDBG_SVC_ADMIN;
    pHdr->Flags = EDBG_FL_FROM_DEV;
    pHdr->SeqNum = BootmeSeqNum++;
    pHdr->Cmd = EDBG_CMD_BOOTME;

    memset(pData,0,sizeof(EDBG_BOOTME_DATA));
    pData->VersionMajor = VersionMajor;
    pData->VersionMinor = VersionMinor;
    pData->uBootmeVer = EDBG_CURRENT_BOOTME_VERSION;
    pData->dwBootFlags = dwBootFlags;
    pData->dwBootFlags |= EDBG_CAPS_TFTP_OPTIONS;  // This bootloader has TFTP options capability
    memcpy(&pData->MACAddr, pMyAddr->wMAC, sizeof(pData->MACAddr));
    memcpy(&pData->IPAddr, &pMyAddr->dwIP, sizeof(pData->IPAddr));
    strncpy_s((char *)pData->PlatformId, sizeof(pData->PlatformId), szPlatformString, sizeof(pData->PlatformId) - 1);
    pData->PlatformId[sizeof(pData->PlatformId) - 1] = '\0';

    if (szDeviceName) {
        strncpy_s((char *)pData->DeviceName, sizeof(pData->DeviceName), szDeviceName, sizeof(pData->DeviceName) - 1);
        pData->DeviceName[sizeof(pData->DeviceName) - 1] = '\0';
    } else {
        // Device name based on platform ID and last word of MAC address
        strncpy_s((char *)pData->DeviceName, sizeof(pData->DeviceName), szPlatformString, sizeof(pData->DeviceName) - 6);
        pData->DeviceName[sizeof(pData->DeviceName) - 6] = '\0';
        if (0 == _itoa_s(ntohs(pMyAddr->wMAC[2]), DevNumBuf, sizeof(DevNumBuf), 10))
        {
            strcat_s((char *)pData->DeviceName, sizeof(pData->DeviceName), DevNumBuf);
        }
    }

    pData->CPUId      = CPUId;

#if 0
    // Send as a broadcast UDP frame on our subnet
    memcpy(&DestAddr.dwIP, &pMyAddr->dwIP, sizeof(DestAddr.dwIP));
    DestAddr.dwIP |= ~SubnetMask;
#else
    // Use local broadcast for now
    memset(&DestAddr.dwIP, 0xFF, sizeof(DestAddr.dwIP));
#endif
    memset(&DestAddr.wMAC, 0xFF, sizeof(DestAddr.wMAC));
    DestAddr.wPort = htons(EDBG_DOWNLOAD_PORT);

    pMyAddr->wPort = htons(EDBG_DOWNLOAD_PORT);

    if (!EbootSendUDP(BootmeBuf, &DestAddr, pMyAddr, (BYTE *)pHdr,
                      EDBG_DATA_OFFSET+sizeof(EDBG_BOOTME_DATA)))
        KITLOutputDebugString("SendBootme()::Error on SendUDP() call\r\n");
    else
        KITLOutputDebugString("Sent BOOTME to %s\r\n", inet_ntoa(DestAddr.dwIP));
}

