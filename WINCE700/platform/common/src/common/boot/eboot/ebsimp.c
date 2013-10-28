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
    ebsimp.c
Abstract:

    This contains the code to simplify bootloader development. It uses
    TFTP and DHCP functions. The intend is to hide all the ether details
    behind the scene for bootloader development. The bootloader can then
    be reduced to the following using the following routine.

      if (!EbootInitEtherTransport (....)) {
          while (1);  // spin forever
      }
      // main loop of download
      While (EbootEtherReadData (...)) {
          // process the data
          ProcessData (...);
      }

    For real implementation, please refer to sources under platform\ODO\eboot.

Functions:


Notes:

--*/
#include <windows.h>
#include "eboot.h"
#include "tftp.h"
#include "dhcp.h"
#include "udp.h"

#define MAX_DHCP_RETRY      3   // max retries on IP conflict

#define MAX_BOOTME_CNT     40   // Max number of BOOTMEs sent
#define BOOTME_INTERVAL     3   // send bootme every 3 seconds

#define DHCP_TIMEOUT      120   // DHCP default timeout in 120 seconds

static LPBYTE gpbData;
static UINT16 gwLength;
static BYTE gFrameBuffer[ETHER_MTU];
static EDBG_ADDR *gpEdbgAddr;
static EDBG_ADDR gHostAddr;
static BOOL fTftpLinked = FALSE;
static EDBG_OS_CONFIG_DATA *gpCfgData;

static UINT16 EbootSimpleCallback (char *pszFileName, TFtpdCallBackOps Operation, BYTE *pbData,
                                   UINT16 *cwLength, char **ppszErrorMsg )
{
    UNREFERENCED_PARAMETER(ppszErrorMsg);
    switch (Operation) {
    case TFTPD_OPEN:
        KITLOutputDebugString( "EthDown::TFTPD_OPEN::%s\r\n", pszFileName);
        fTftpLinked = TRUE;
        gpbData = NULL;
        gwLength = 0;
        break;
    case TFTPD_WRITE:
        gpbData = pbData;
        gwLength = *cwLength;
        break;
    default:
        KITLOutputDebugString( "EthDown::Illegal Operation Code %u\r\n", Operation );
        break;
    }
    return 0;
}

BOOL EbootInitTftpSimple (EDBG_ADDR *pEdbgAddr, UINT16 wOdoWKSP, UINT16 wHostWKSP, char *pszFileName)
{
    // Set up TFTP server.  Note that internally, we store port numbers in network byte order.
    EbootInitTFtp (wOdoWKSP, wHostWKSP);
    EbootInitTFtpd ();
    if (EbootTFtpdServerRegister (pszFileName, EbootSimpleCallback)) {
        KITLOutputDebugString( "Server Registration Failed\r\n" );
        return FALSE;
    }
    gpEdbgAddr = pEdbgAddr;
    return TRUE;
}

// Ethernet frame type is in 7th word of buffer
#define FRAMETYPE(x)    ntohs(*(USHORT *)((x) + 6*sizeof(USHORT)))

BOOL EbootEtherReadData (DWORD cbData, LPBYTE pbData)
{
    USHORT wCopied;
    USHORT wLen, wDestPort, wSrcPort, wUDPDataLen, *pwUDPData;

    if (!gpEdbgAddr) {
        KITLOutputDebugString ("EbootSimpleTFTP not initialized\r\n" );
        return FALSE;
    }
    while (cbData) {

        // wait for the next input
        while (!gwLength) {

            // get another frame and pass it to TFTP handler
            wLen = sizeof (gFrameBuffer);
            if (OEMEthGetFrame (gFrameBuffer, &wLen))       // frame available?
            {
                switch( FRAMETYPE(gFrameBuffer) ) {
                    case 0x0800:    // IP Packet
                        if (!EbootCheckUDP(gpEdbgAddr, gFrameBuffer, &wDestPort, &wSrcPort, &pwUDPData, &wUDPDataLen) &&
                             EbootTFtpReceiver(gpEdbgAddr, gFrameBuffer, wDestPort, wSrcPort, pwUDPData, wUDPDataLen))
                           {
                                // EbootTFtpReceiver returns 1 when a new link
                                // is formed - this really isn't an error but
                                // we shouldn't keep the data either.
                                gwLength = 0;
                           }
                        break;
                    case 0x0806:    // ARP Packet
                        EbootProcessARP( gpEdbgAddr, gFrameBuffer );
                        gwLength = 0;
                        break;
                    default:
                        break;
                }
            }

        }

        wCopied = (cbData > gwLength)? gwLength : (USHORT) cbData;
        memcpy (pbData, gpbData, wCopied);
        gpbData += wCopied;
        gwLength -= wCopied;
        pbData += wCopied;
        cbData -= wCopied;
    }
    return TRUE;
}


// check if anyone using the IP address we're using
BOOL EbootCheckIP (EDBG_ADDR *pEdbgAddr)
{
    DWORD dwCurSec;
    USHORT wLen;

    // Send a gratuitous ARP (an ARP of our new IP address) to verify that no other
    // station is using our IP address.
    if (EbootGratuitousARP (pEdbgAddr, gFrameBuffer)) {
        KITLOutputDebugString("EbootGratuitousARP failed\r\n");
        return FALSE;
    }

    // wait a while (ARP_RESPONSE_WAIT seconds) and see if anyone is responding
    // to the Gratuitous ARP
    dwCurSec = OEMKitlGetSecs ();
    while (OEMKitlGetSecs () - dwCurSec < ARP_RESPONSE_WAIT) {
        wLen = sizeof (gFrameBuffer);
        if (OEMEthGetFrame (gFrameBuffer, &wLen)        // frame available?
            && (0x0806 == FRAMETYPE (gFrameBuffer))     // ARP packet?
            && (EbootProcessARP (pEdbgAddr, gFrameBuffer) == PROCESS_ARP_RESPONSE)) { // to us?
            return FALSE;
        }
    }
    return TRUE;
}

BOOL EbootGetDHCPAddr (EDBG_ADDR *pEdbgAddr, DWORD *pdwSubnetMask, DWORD *pdwDHCPLeaseTime)
{
    USHORT wLen, wDestPort, wSrcPort, wUDPDataLen, *pwUDPData;
    BOOL fGotIP = FALSE;
    int nRetries;
    DWORD dwCurSec;

    // retry until we get a unique IP address from DHCP
    for (nRetries = 0; nRetries < MAX_DHCP_RETRY; nRetries ++) {
        // initialize DHCP, send DHCP request
        if (EbootInitDHCP (pEdbgAddr)) {
            KITLOutputDebugString( "Error On InitDHCP() Call\r\n" );
            return FALSE;
        }

        // Loop until we get an IP address or until DHCP server response timeout
        dwCurSec = OEMEthGetSecs ();
        while (!fGotIP) {
            // see if DHCP timed out
            if ((OEMEthGetSecs () - dwCurSec) > DHCP_TIMEOUT) {
                KITLOutputDebugString("DHCP server is not responding !!! Aborting.\r\n");
                return FALSE;
            }
            // try to re-transmit DHCP request
            EbootDHCPRetransmit (pEdbgAddr, NULL, NULL);

            // process DHCP packets
            wLen = sizeof (gFrameBuffer);
            if (OEMEthGetFrame (gFrameBuffer, &wLen)        // frame available?
                && (0x0800 == FRAMETYPE (gFrameBuffer))     // IP packet?
                && !EbootCheckUDP (pEdbgAddr, gFrameBuffer, &wDestPort, &wSrcPort, &pwUDPData, &wUDPDataLen)    // UDP?
                && (wDestPort == DHCP_CLIENT_PORT)          // DHCP src/dst port?
                && (wSrcPort == DHCP_SERVER_PORT)) {
                EbootProcessDHCP (pEdbgAddr,
                    pdwSubnetMask,
                    (BYTE *)pwUDPData,
                    wUDPDataLen,
                    pdwDHCPLeaseTime,
                    &fGotIP);
                KITLOutputDebugString ("Got Response from DHCP server, IP address: %s\r\n",
                                      inet_ntoa(pEdbgAddr->dwIP));
            }
        }

        // verify that we are the only one using this address
        if (EbootCheckIP (pEdbgAddr)) {
            KITLOutputDebugString ("No ARP response in %d seconds, assuming ownership of %s\r\n",
                                      ARP_RESPONSE_WAIT, inet_ntoa(pEdbgAddr->dwIP));
            return TRUE;
        }
        KITLOutputDebugString( "Some other station has IP Address: %s !!! Retrying.\r\n", inet_ntoa(pEdbgAddr->dwIP));
        fGotIP = FALSE;
    }

    KITLOutputDebugString ("Some other station has IP Address: %s !!! Aborting.\r\n", inet_ntoa(pEdbgAddr->dwIP));

    return FALSE;
}


BOOL EbootSendBootmeAndWaitForTftp (EDBG_ADDR *pEdbgAddr, UCHAR VersionMajor, UCHAR VersionMinor,
                             char *szPlatformString, char *szDeviceName,
                             UCHAR CPUId, DWORD dwBootFlags)
{
    DWORD dwCurSec = OEMKitlGetSecs () - BOOTME_INTERVAL;
    USHORT wLen, wDestPort, wSrcPort, wUDPDataLen, *pwUDPData;
    int nRetries = 0;
    KITLOutputDebugString ("+EbootSendBootmeAndWaitForTftp\r\n");
    while (!fTftpLinked) {
        if ((nRetries < MAX_BOOTME_CNT) && (OEMKitlGetSecs () - dwCurSec >= BOOTME_INTERVAL)) {
            nRetries ++;
            dwCurSec += BOOTME_INTERVAL;
            // send a bootme message
            EbootSendBootme (pEdbgAddr, VersionMajor, VersionMinor, szPlatformString, szDeviceName, CPUId, dwBootFlags);
        }
        // get another frame and pass it to TFTP handler
        wLen = sizeof (gFrameBuffer);
        if (OEMEthGetFrame (gFrameBuffer, &wLen)) {       // frame available?
            switch (FRAMETYPE (gFrameBuffer)) {
            case 0x0800:    // IP packet
                if (!EbootCheckUDP(pEdbgAddr, gFrameBuffer, &wDestPort, &wSrcPort, &pwUDPData, &wUDPDataLen)) {  // UDP?
                    // EDBG command? (should only occur if eshell asked us to jump to existing image)
                    if (!EbootProcessEDBG (pEdbgAddr, &gHostAddr, gFrameBuffer, pwUDPData, wUDPDataLen, &fTftpLinked, &gpCfgData)) {
                        // no, pass it to TFTP
                        EbootTFtpReceiver (pEdbgAddr, gFrameBuffer, wDestPort, wSrcPort, pwUDPData, wUDPDataLen);
                    }
                }
                break;
            case 0x0806:    // ARP packet
                if (EbootProcessARP (pEdbgAddr, gFrameBuffer) == PROCESS_ARP_RESPONSE) {
                    KITLOutputDebugString( "Some other station has IP Address: %s !!! Aborting.\r\n", inet_ntoa(pEdbgAddr->dwIP));
                    return FALSE;
                }
                break;
            default:
                break;
            }
        }
    }
    KITLOutputDebugString ("-EbootSendBootmeAndWaitForTftp\r\n");
    return TRUE;
}

EDBG_OS_CONFIG_DATA *EbootWaitForHostConnect (EDBG_ADDR *pDevAddr, EDBG_ADDR *pHostAddr)
{
    USHORT wLen, wDestPort, wSrcPort, wUDPDataLen, *pwUDPData;
    BOOL fGotJumpimg = FALSE;
    while (!gpCfgData) {
        wLen = sizeof (gFrameBuffer);
        if (OEMEthGetFrame (gFrameBuffer, &wLen)) {        // frame available?
            if ((0x0800 == FRAMETYPE (gFrameBuffer))     // IP packet?
                && !EbootCheckUDP (pDevAddr, gFrameBuffer, &wDestPort, &wSrcPort, &pwUDPData, &wUDPDataLen)) {    // UDP?
                // check EDBG for JUMP packet
                if (!EbootProcessEDBG(pDevAddr, &gHostAddr, gFrameBuffer, pwUDPData, wUDPDataLen, &fGotJumpimg, &gpCfgData))
                {
                    // workaround for case where it sends an empty data packet at the end of the download
                    EbootTFtpReceiver(pDevAddr, gFrameBuffer, wDestPort, wSrcPort, pwUDPData, wUDPDataLen);
                }
            }
            if (0x0806 == FRAMETYPE (gFrameBuffer)) {     // ARP packet?
                if (EbootProcessARP (pDevAddr, gFrameBuffer) == PROCESS_ARP_RESPONSE) {
                    KITLOutputDebugString( "WARNING: Some other station has IP Address: %s !!!\r\n", inet_ntoa(pDevAddr->dwIP));
                }
            }
        }
    }
    if (pHostAddr) {
        memcpy (pHostAddr, &gHostAddr, sizeof (EDBG_ADDR));
    }
    return gpCfgData;
}


BOOL EbootInitEtherTransport (EDBG_ADDR *pEdbgAddr, LPDWORD pdwSubnetMask,
                              BOOL *pfJumpImg,         // will be set to TRUE if eshell asked us to jump to existing image
                              DWORD *pdwDHCPLeaseTime, // this parameter is overloaded. pass NULL to indicate static IP
                              UCHAR VersionMajor, UCHAR VersionMinor,
                              char *szPlatformString, char *szDeviceName,
                              UCHAR CPUId, DWORD dwBootFlags)
{
    // simple check on arguments
    if (!pEdbgAddr || !pdwSubnetMask || !szPlatformString) {
        return FALSE;
    }

    // find out IP address and verify it
    if (pdwDHCPLeaseTime) {
        if (!EbootGetDHCPAddr (pEdbgAddr, pdwSubnetMask, pdwDHCPLeaseTime)) {
            return FALSE;
        }
    } else {
        if (!EbootCheckIP (pEdbgAddr)) {
            KITLOutputDebugString ("Some other station has IP Address: %s !!! Aborting.\r\n", inet_ntoa(pEdbgAddr->dwIP));
            return FALSE;
        }
        // This will tell CheckUDP() to only accept datagrams for our IP address
        ClearPromiscuousIP();
    }

    if (!EbootInitTftpSimple (pEdbgAddr, htons(EDBG_DOWNLOAD_PORT), htons(EDBG_DOWNLOAD_PORT), EDBG_DOWNLOAD_FILENAME)) {
        return FALSE;
    }
    if (!EbootSendBootmeAndWaitForTftp (pEdbgAddr, VersionMajor, VersionMinor, szPlatformString, szDeviceName, CPUId, dwBootFlags))
        return FALSE;

    *pfJumpImg = (NULL != gpCfgData);
    return TRUE;
}
