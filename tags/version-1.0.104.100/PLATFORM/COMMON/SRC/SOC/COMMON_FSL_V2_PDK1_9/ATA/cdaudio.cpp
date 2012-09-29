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

// pIOReq->pBytesReturned is safe (supplied by DSK_IOControl).  Exceptions are
// caught in CDisk::PerformIoctl.  And, we can map pInBuf and pOutBuf in-place,
// since they are copies setup by DSK_IOControl.
DWORD CMXDisk::ControlAudio(PIOREQ pIOReq)
{
    ATAPI_COMMAND_PACKET CmdPkt = {0};
    DWORD                dwError = ERROR_SUCCESS;

    switch (pIOReq->dwCode) {
        case IOCTL_CDROM_PLAY_AUDIO_MSF:
            {
                PCDROM_PLAY_AUDIO_MSF pCdromPlayAudioMsf = (PCDROM_PLAY_AUDIO_MSF)pIOReq->pInBuf;
                // Validate buffer.
                if (NULL == pCdromPlayAudioMsf) {
                    dwError = ERROR_INVALID_PARAMETER;
                    break;
                }
                // Validate size.
                if (sizeof(CDROM_PLAY_AUDIO_MSF) > pIOReq->dwInBufSize) {
                    dwError = ERROR_INVALID_PARAMETER;
                    break;
                }
                // Prepare IOCTL.
                CmdPkt.Opcode = ATAPI_PACKET_CMD_PLAY_MSF;
                CmdPkt.Byte_3 = pCdromPlayAudioMsf->StartingM;
                CmdPkt.Byte_4 = pCdromPlayAudioMsf->StartingS;
                CmdPkt.Byte_5 = pCdromPlayAudioMsf->StartingF;
                CmdPkt.Byte_6 = pCdromPlayAudioMsf->EndingM;
                CmdPkt.Byte_7 = pCdromPlayAudioMsf->EndingS;
                CmdPkt.Byte_8 = pCdromPlayAudioMsf->EndingF;
            }
            break;
        case IOCTL_CDROM_SEEK_AUDIO_MSF:
            {
                PCDROM_SEEK_AUDIO_MSF pCdromSeekMsf = (PCDROM_SEEK_AUDIO_MSF)pIOReq->pInBuf;
                DWORD                 dwLBA;
                // Validate buffer.
                if (NULL == pCdromSeekMsf) {
                    dwError = ERROR_INVALID_PARAMETER;
                    break;
                }
                // Validate size.
                if (sizeof(CDROM_SEEK_AUDIO_MSF) > pIOReq->dwInBufSize) {
                    dwError = ERROR_INVALID_PARAMETER;
                    break;
                }
                // Prepare IOCTL.
                dwLBA = CDROM_MSFCOMP_TO_LBA(pCdromSeekMsf->M, pCdromSeekMsf->S, pCdromSeekMsf->F);
                CmdPkt.Opcode = ATAPI_PACKET_CMD_SEEK;
                CmdPkt.Byte_2 = LBA_MSB(&dwLBA);
                CmdPkt.Byte_3 = LBA_3rdLSB(&dwLBA);
                CmdPkt.Byte_4 = LBA_2ndLSB(&dwLBA);
                CmdPkt.Byte_5 = LBA_LSB(&dwLBA);
            }
            break;
        case IOCTL_CDROM_SCAN_AUDIO:
            {
                PCDROM_SCAN_AUDIO pCdromScanAudio = (PCDROM_SCAN_AUDIO)pIOReq->pInBuf;
                // Validate buffer.
                if (NULL == pCdromScanAudio) {
                    dwError = ERROR_INVALID_PARAMETER;
                    break;
                }
                // Validate size.
                if (sizeof(CDROM_SCAN_AUDIO) > pIOReq->dwInBufSize) {
                    dwError = ERROR_INVALID_PARAMETER;
                    break;
                }
                // Prepare IOCTL.
                CmdPkt.Opcode = ATAPI_PACKET_CMD_SCAN_AUDIO;
                CmdPkt.Byte_1 = (pCdromScanAudio->Direction << 4) & 0x10;
                CmdPkt.Byte_2 = pCdromScanAudio->Address[0];
                CmdPkt.Byte_3 = pCdromScanAudio->Address[1];
                CmdPkt.Byte_4 = pCdromScanAudio->Address[2];
                CmdPkt.Byte_5 = pCdromScanAudio->Address[3];
                CmdPkt.Byte_9 = pCdromScanAudio->AddressType << 6;
            }
            break;
        case IOCTL_CDROM_STOP_AUDIO:
            CmdPkt.Opcode = ATAPI_PACKET_CMD_STOP_PLAY_SCAN;
            break;
        case IOCTL_CDROM_PAUSE_AUDIO:
            CmdPkt.Opcode = ATAPI_PACKET_CMD_PAUSE_RESUME;
            CmdPkt.Byte_8 = 0;
            break;
        case IOCTL_CDROM_RESUME_AUDIO:
            CmdPkt.Opcode = ATAPI_PACKET_CMD_PAUSE_RESUME;
            CmdPkt.Byte_8 = 1;
            break;
    }

    if (ERROR_SUCCESS == dwError) {
        SGX_BUF SgBuf;
        DWORD   cbRead;
        SgBuf.sb_len = 0;
        SgBuf.sb_buf = NULL;
        // Execute IOCTL.
        if (AtapiSendCommand(&CmdPkt)) {
            if (FALSE == AtapiReceiveData(&SgBuf, 1, &cbRead)) {
                DEBUGMSG(ZONE_ERROR|ZONE_FUNC, (TEXT("ATAPI!CMXDisk::ControlAudio> Failed to receive data (IOCTL %d)\r\n"), pIOReq->dwCode));
                dwError = ERROR_READ_FAULT;
            }
        }
        else {
          dwError = ERROR_GEN_FAILURE;
        }
    }

    return dwError;
}
