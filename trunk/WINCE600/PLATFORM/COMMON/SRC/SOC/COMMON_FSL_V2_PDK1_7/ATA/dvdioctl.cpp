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

#include <atamain.h>
#include <ata.h>

static
DWORD
rkret[] =
{
    sizeof (RKFMT_AGID) << 24,
    sizeof (RKFMT_CHLGKEY) << 24,
    sizeof (RKFMT_BUS) << 24,
    sizeof (RKFMT_BUS) << 24,
    sizeof (RKFMT_TITLE) << 24,
    0,
    0,
    0,
    sizeof (RKFMT_RPC) << 24,
};

DWORD
EndSwap(
    DWORD dwInput
    )
{
    register DWORD dwResult;
    dwResult = (dwInput << 24) | ((dwInput & 0xff00) << 8) | ((dwInput & 0xff0000) >> 8) | (dwInput >> 24);
    return(dwResult);
}

DWORD
DVDSetupReadTitleKey(
    ATAPI_COMMAND_PACKET *pCmdPkt,
    BYTE                  bKeyType,
    BYTE                  bAgid,
    DWORD                 dwLBA
    )
{
    DWORD dwRet;
    PRKCDB pcdb = (PRKCDB)(pCmdPkt);

    pcdb->OpCode = DVDOP_REPORT_KEY;
    pcdb->KeyFmt = (bAgid << 6) | bKeyType;
    pcdb->Lun = 0;

    if (bKeyType >= (sizeof(rkret) / sizeof(DWORD))) {
        dwRet = 0;
    }
    else {
        dwRet = rkret[bKeyType];
    }

    // use memcpy because pcdb->Reserved is not aligned.
    memcpy((BYTE*)&pcdb->Reserved, (BYTE*)&dwRet, sizeof(DWORD));

    pcdb->LBA = EndSwap(dwLBA);
    pcdb->NACA = 0;

    return (EndSwap(dwRet));
}

DWORD
DVDSetupReadDiscKey(
    ATAPI_COMMAND_PACKET *pCmdPkt,
    BYTE                  bAgid
    )
{
    PRDVDCDB pcdb = (PRDVDCDB)(pCmdPkt);
// compile error
    DWORD PCDBLen;
    
    PCDBLen = (sizeof (RKFMT_DISC) << 8 | sizeof (RKFMT_DISC) >>8);

    pcdb->OpCode = DVDOP_READ_DVD_STRUC;

    pcdb->Lun = 0;
    memset((BYTE*)&pcdb->RMDLBA, 0, sizeof(pcdb->RMDLBA));
    pcdb->Layer = 0;
    pcdb->Format = DVDSTRUC_FMT_DISCKEY;
// compile error
//    pcdb->Len = (USHORT)(sizeof (RKFMT_DISC) << 8 | sizeof (RKFMT_DISC) >>8);
    pcdb->Len = (USHORT)PCDBLen;
    pcdb->agid = bAgid;
    pcdb->NACA = 0;

    return (sizeof (RKFMT_DISC));
}

DWORD
DVDSetupReadKey(
    ATAPI_COMMAND_PACKET  *pCmdPkt,
    PDVD_COPY_PROTECT_KEY  pKey
    )
{
    DWORD dwRet = 0;

    switch (pKey->KeyType) {
    case DvdTitleKey:
        dwRet = DVDSetupReadTitleKey(pCmdPkt, (BYTE)(pKey->KeyType),
            (BYTE)pKey->SessionId, pKey->StartAddr + 1);
        break;
    case DvdDriveKey:
        dwRet = DVDSetupReadDiscKey(pCmdPkt, (BYTE)((pKey->SessionId) << 6));
        break;
    default:
        dwRet = DVDSetupReadTitleKey(pCmdPkt, (BYTE)(pKey->KeyType),
           (BYTE)pKey->SessionId, 0);
    }

    return dwRet;
}

DWORD
CopyDVDKey(
    DWORD                 dwKeyLength,
    PDVD_COPY_PROTECT_KEY pKey,
    PRKFMT_TITLE          pTitle)
{
    DWORD dwTmp;
    PBYTE psrc, pdest;
// compile error
    psrc = (PBYTE)&(pTitle->cgms);

    dwTmp = dwKeyLength - 4;

    pdest = (PBYTE)&(pKey->KeyData[0]);

    switch (pKey->KeyType) {
    case DvdTitleKey:
        pKey->KeyFlags = pTitle->cgms;
        dwTmp--;
        psrc = (PBYTE)&(pTitle->title[0]);
        pdest = (PBYTE)&(pKey->KeyData[0]);
        break;
    case DvdAGID:
        pKey->SessionId = ((PRKFMT_AGID)pTitle)->agid;
        dwTmp = 0;
        break;
    default:
        psrc = (PBYTE)&(pTitle->cgms);
        pdest = (PBYTE)&(pKey->KeyData[0]);
        break;
    }

    for (;dwTmp >0; dwTmp--) {
        *pdest++ = *psrc++;
    }

    return TRUE;
}

DWORD
CMXDisk::DVDReadKey(
    PIOREQ pIOReq
    )
{
    DWORD                 dwError = ERROR_SUCCESS;
    SGX_BUF               SgBuf;
    ATAPI_COMMAND_PACKET  CmdPkt;
    PDVD_COPY_PROTECT_KEY pKey = (PDVD_COPY_PROTECT_KEY)pIOReq->pInBuf;
    DWORD                 dwLength;
    RKFMT_DISC            *pkeyBuf;
    DWORD                 dwRet;

    pkeyBuf = (RKFMT_DISC*)LocalAlloc(LPTR, sizeof(RKFMT_DISC));

    memset(&CmdPkt, 0, sizeof(ATAPI_COMMAND_PACKET));

    if (IOCTL_DVD_START_SESSION == pIOReq->dwCode) {
        pKey->KeyType = DvdAGID;
        // TODO: Check region
        dwLength = DVDSetupReadTitleKey(&CmdPkt, DvdAGID, 0, 0);
    }
    else {
        dwLength = DVDSetupReadKey(&CmdPkt, pKey);
    }

    if (dwLength > pKey->KeyLength) {
        DEBUGMSG(ZONE_ERROR, (TEXT(
            "Atapi!CMXDisk::DVDReadKey> illegal key request\r\n"
            )));
        return ERROR_INVALID_PARAMETER;
    }

    SgBuf.sb_len = dwLength;
    SgBuf.sb_buf = (PBYTE) (pkeyBuf);

    if (AtapiSendCommand(&CmdPkt)) {
        if (AtapiReceiveData(&SgBuf, 1, &dwRet)) {
            CopyDVDKey(dwLength, pKey, (PRKFMT_TITLE)(pkeyBuf));
        }
        else {
            DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (TEXT(
                "Atapi!CMXDisk::DVDReadKey> failed to execute command %d\r\n"
                ), CmdPkt.Opcode));
            dwError = ERROR_READ_FAULT;
        }
    }
    else {
        dwError = ERROR_GEN_FAILURE;
    }

    return dwError;
}

/*
    CMXDisk::DVDGetRegion

        Return the Region Playback Control (RPC) setting.

    Return

        Win32 error.

        Populate the DVD_REGIONCE struct nested in the supplied IOREQ.  If
        the target drive is a virgin drive, i.e., its RPC Setting (Region) has
        never been set, then return 0xFF as its Region.

    Notes

        For more information on the REPORT KEY command, see SFF8090i v% R0.10,
        13.32.

        The REPORT KEY data format with (KEY format = 001000b, Key Class = 0)
        is of the following form:

        Byte 0 REPORT KEY Data Length (MSB)
        Byte 1 REPORT KEY Data Length (LSB)
        Byte 2 Reserved
        Byte 3 Reserved
        Byte 4 RPC State
                  bit
               |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
               |   Type Code   |Vendor Resets Remaining|User Controlled Changes|
        Byte 5 Region Mask
        Byte 6 RPC Scheme
        Byte 7 Reserved

        RPC State Type Code (Proper RPC State (4.13.3))
        -------------------
        00b | NONE; No drive region setting (virgin)
        01b | SET; Drive region is set
        10b | LAST CHANGE; Drive region is set, with additional restrictions
            | required to make a change
        11b | Drive region has been set permanently, but may be reset by the
            | vendor, if necessary
*/
#ifdef DEBUG
#define DUMP_REPORT_KEY(x) \
    if ((x.ResetCounts & 0xC0) == 0) { \
        DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (_T( \
            "Atapi!CMXDisk::DVDGetRegion> RPC State=NONE; No drive region setting\r\n" \
            ))); \
    } \
    else if ((x.ResetCounts & 0xC0) == 0x40) { \
        DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (_T( \
            "Atapi!CMXDisk::DVDGetRegion> RPC State=SET; Drive region set\r\n" \
            ))); \
    } \
    else if ((x.ResetCounts & 0xC0) == 0x80) { \
        DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (_T( \
            "Atapi!CMXDisk::DVDGetRegion> RPC State=LAST CHANCE; Drive region set--last chance to set region\r\n" \
            ))); \
    } \
    else if ((x.ResetCounts & 0xC0) == 0xC0) { \
        DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (_T( \
            "Atapi!CMXDisk::DVDGetRegion> RPC State=PERM; Drive region set permanently\r\n" \
            ))); \
    } \
    DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (_T( \
        "Atapi!CMXDisk::DVDGetRegion> %u vendor resets available\r\n" \
        ), (x.ResetCounts & 0x38) >> 3)); \
    DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (_T( \
        "Atapi!CMXDisk::DVDGetRegion> %u user-controlled changes available\r\n" \
        ), x.ResetCounts & 0x07)); \
    DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (_T( \
        "Atapi!CMXDisk::DVDGetRegion> Region mask=%u\r\n" \
        ), x.SystemRegion ^ 0xFF)); \
    DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (_T( \
        "Atapi!CMXDisk::DVDGetRegion> RPC Scheme=%u\r\n" \
        ), x.RPCVer));
#else
#define DUMP_REPORT_KEY(x)
#endif

DWORD
CMXDisk::DVDGetRegion(
    PIOREQ pIOReq
    )
{
    DWORD                dwError = ERROR_SUCCESS;
    DWORD                dwLength;
    SGX_BUF              SgBuf;
    DWORD                dwRet;
    RKFMT_RPC            rpcData;
    PDVD_REGIONCE        preg = NULL;
    ATAPI_COMMAND_PACKET CmdPkt;
    PBYTE                psrcbuf;

    memset(&CmdPkt, 0, sizeof(ATAPI_COMMAND_PACKET));

    // map address and check for security violation

    psrcbuf = pIOReq->pOutBuf;
    ASSERT(psrcbuf);
    ASSERT(pIOReq->dwOutBufSize >= sizeof(DVD_REGIONCE));

    if(FAILED(CeOpenCallerBuffer(
                (PVOID *)&preg,
                (PVOID)pIOReq->pOutBuf,
                sizeof(DVD_REGIONCE),
                ARG_O_PTR,
                FALSE))) {

        DEBUGMSG(ZONE_ERROR, (TEXT(
            "Atapi!CMXDisk::DVDGetRegion> Failed in CeOpenCallerBuffer\r\n"
            )));
        return ERROR_INVALID_PARAMETER;
    }

    // read disc copy system and region
    DVDGetCopySystem(&(preg->CopySystem), &(preg->RegionData));

    memset(&CmdPkt, 0, sizeof(ATAPI_COMMAND_PACKET));

    // create command packet to read RPC setting (report key)
    dwLength = DVDSetupReadTitleKey(&CmdPkt, DvdGetRPC, 0, 0);
    SgBuf.sb_len = sizeof(RKFMT_RPC);
    SgBuf.sb_buf = (PBYTE) &rpcData;

    // send command
    if (AtapiSendCommand(&CmdPkt)) {
        // fetch result
        if (AtapiReceiveData(&SgBuf, 1, &dwRet)) {
            DUMP_REPORT_KEY(rpcData);
            // is the drive in a non-NONE RPC state?
            if (rpcData.ResetCounts & 0xc0) {
                // the drive is in SET, LAST CHANCE or PERM, i.e., this drive
                // has had its region set already
                preg->SystemRegion = rpcData.SystemRegion ^ 0xFF;
                preg->ResetCount = rpcData.ResetCounts & 0x07;
            }
            else {
                // the drive is in NONE, i.e., this drive is a virgin
                DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (_T(
                    "Atapi!CMXDisk::DVDGetRegion> Region has never been set\r\n"
                    )));
                preg->SystemRegion = 0xFF;
                preg->ResetCount = 5;
            }
        }
        else {
            DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (TEXT(
                "Atapi!CMXDisk::DVDGetRegion> Failed to receive REPORT KEY response\r\n"
                )));
            dwError = ERROR_READ_FAULT;
        }
    }
    else {
        DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (TEXT(
            "Atapi!CMXDisk::DVDGetRegion> Failed to send REPORT KEY command\r\n"
            )));
        dwError = ERROR_READ_FAULT;
    }


    if(FAILED(CeCloseCallerBuffer(
                (PVOID)preg,
                (PVOID)psrcbuf,
                sizeof(DVD_REGIONCE),
                ARG_O_PTR))) {

        DEBUGMSG(ZONE_ERROR, (TEXT(
            "Atapi!CMXDisk::DVDGetRegion> Unexpected failure in CeCloseCallerBuffer\r\n"
            )));
        return dwError = ERROR_GEN_FAILURE;
    }
    return dwError;
}

DWORD
DVDSetupSendKey(
    ATAPI_COMMAND_PACKET  *pCmd,
    PDVD_COPY_PROTECT_KEY  pKey,
    RKFMT_CHLGKEY         *keyBuf
    )
{
    BYTE   KeyType;
    BYTE   agid;
    DWORD  dwRet;
    DWORD  dwTmp;
    PRKCDB pcdb = (PRKCDB)pCmd;
    PVOID  pdata = &(pKey->KeyData[0]);

    KeyType = (BYTE)pKey->KeyType;
    agid = (BYTE)pKey->SessionId;

    pcdb->OpCode = DVDOP_SEND_KEY;
    pcdb->Lun = 0;
    pcdb->KeyFmt = (agid << 6) | KeyType;

    if (
        !KeyType || KeyType > DvdBusKey2 ||
        (pKey->KeyLength < ((dwRet = rkret[KeyType]) >> 24))
    ) {
        return 0;
    }

    keyBuf->Len = (WORD)((dwRet >> 16) - (2 << 8));
    keyBuf->Reserved[0] = keyBuf->Reserved[1] = 0;

    if (KeyType == DvdBusKey2) {
        for (dwTmp = 0; dwTmp < 5; dwTmp++) {
            *((PBYTE)(&keyBuf->chlgkey) + dwTmp) = *((PBYTE)pdata + dwTmp);
        }

        for (;dwTmp < 8; dwTmp++) {
            *((PBYTE)(&keyBuf->chlgkey) + dwTmp) = 0;
        }
    }
    else {
        for (dwTmp = 0; dwTmp < ((dwRet >> 26) - 1); dwTmp++ ) {
            *((PDWORD)(&keyBuf->chlgkey) + dwTmp) = *((PDWORD)pdata + dwTmp);
        }
    }

    // use memcpy because pcdb->Reserved is not aligned.
    memcpy((BYTE*)&pcdb->Reserved, (BYTE*)&dwRet, sizeof(DWORD));

    pcdb->LBA = 0;
    pcdb->NACA = 0;

    return (EndSwap(dwRet));
}

DWORD
CMXDisk::DVDSendKey(
    PIOREQ pIOReq
    )
{
    DWORD                 dwError = ERROR_SUCCESS;
    ATAPI_COMMAND_PACKET  CmdPkt;
    PDVD_COPY_PROTECT_KEY pKey = (PDVD_COPY_PROTECT_KEY)pIOReq->pInBuf;
    DWORD                 dwLength;
    RKFMT_CHLGKEY         keyBuf;
    SGX_BUF               SgBuf;
    DWORD                 dwRet;

    memset(&CmdPkt, 0, sizeof(ATAPI_COMMAND_PACKET));

    if (IOCTL_DVD_END_SESSION == pIOReq->dwCode) {
        BYTE bAgid;
        pKey->KeyType = DvdAGID;
        bAgid= (BYTE)pKey->SessionId;
        if (bAgid == -1) {
            bAgid = 0;
        }
// compile error
//        dwLength = DVDSetupReadTitleKey(&CmdPkt, -1, bAgid, 0);
        dwLength = DVDSetupReadTitleKey(&CmdPkt, (BYTE)(-1), bAgid, 0);
    }
    else {
        dwLength = DVDSetupSendKey(&CmdPkt, pKey, &keyBuf);
    }

    SgBuf.sb_len = dwLength;
    SgBuf.sb_buf = (PBYTE) &keyBuf;

    if (AtapiSendCommand(&CmdPkt)) {
        if (!AtapiSendData(&SgBuf, 1, &dwRet)) {
            DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (TEXT(
                "Atapi!CMXDisk::DVDSendKey> Failed to execute command %d\r\n"
                ), CmdPkt.Opcode));
            dwError = ERROR_WRITE_FAULT;
        }
    }
    else {
      dwError = ERROR_GEN_FAILURE;
    }

    return dwError;
}

DWORD
CMXDisk::DVDSetRegion(
    PIOREQ pIOReq
    )
{
    PIOREQ tmppIOReq;
    tmppIOReq = pIOReq;
    
    return ERROR_NOT_SUPPORTED;
}

/*
    CMXDisk::DVDGetRegion

        Return a bitfield which describes the regions in which the mounted
        disc can be played.

    Return

        Success.

    Notes

        READ DVD STRUCTURE Data Format with Format field DVDSTRUCT_FMT_COPY
        returns the Region Management Information, which describes the regions
        in which the disc can be played.  Each bit represents one of eight
        regions.  If a bit is cleared in this field, the disc can be played in
        the corresponding region.  If a bit is set in this field, the disc can
        not be played in the corresponding region.

        We perform an XOR with this field.

        For more information on the READ DVD STRUCTURE command, see SFF8090i v5
        R0.10, 13.25.

        The READ DVD STRUCTURE with Format field = 0x01 return data is of the
        following form:

        Byte 0 DVD STRUCTURE Data Length (MSB)
        Byte 1 DVD STRUCTURE Data Length (LSB)
        Byte 2 Reserved
        Byte 3 Reserved
        Byte 4 Copyright Protection System Type
        Byte 5 Region Management Information
        Byte 6 Reserved
        Byte 7 Reserved
*/
BOOL
CMXDisk::DVDGetCopySystem(
    LPBYTE pbCopySystem,
    LPBYTE pbRegionManagement
    )
{
    BOOL                 fSuccess = FALSE;
    ATAPI_COMMAND_PACKET CmdPkt;
    PRDVDCDB             pCmd = (PRDVDCDB)&CmdPkt;
    RDVDFMT_Copy         fmtCopy;
    SGX_BUF              SgBuf;
    DWORD                dwRet;

    DEBUGCHK(NULL != pbCopySystem);
    DEBUGCHK(NULL != pbRegionManagement);

    *pbCopySystem = 0;
    *pbRegionManagement = 0;

    memset(&CmdPkt, 0, sizeof(ATAPI_COMMAND_PACKET));
    memset(&fmtCopy, 0, sizeof(RDVDFMT_Copy));

    pCmd->OpCode = DVDOP_READ_DVD_STRUC;
    pCmd->Format = DVDSTRUC_FMT_COPY;
    pCmd->Len = sizeof (RDVDFMT_Copy) << 8; // endian swap length
    // pCmd->Len = keyLength << 8;
    SgBuf.sb_len = sizeof(RDVDFMT_Copy);
    SgBuf.sb_buf = (PBYTE) &fmtCopy;

    m_dwDeviceFlags &= ~DFLAGS_MEDIA_ISDVD;

    // attempt to detect if media is present
    if (!AtapiIsUnitReady()) {
        DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (_T(
            "Atapi!CMXDisk::DVDGetCopySystem> DVD not present\r\n"
            )));
       goto exit;
    }

    // fmtCopy is a struct of two DWORDs; we need to return 4th and 5th bytes
    if (AtapiSendCommand(&CmdPkt)) {
        if (AtapiReceiveData(&SgBuf, 1, &dwRet)) {
            *pbCopySystem = (BYTE)(fmtCopy.Data & 0xFF);              // 4th byte
            *pbRegionManagement = (BYTE)((fmtCopy.Data >> 8) & 0xFF); // 5th byte
            *pbRegionManagement = *pbRegionManagement ^ 0xFF;         // supported regions are 1s
            fSuccess = TRUE;
        }
    }

exit:;

    return fSuccess;
}

