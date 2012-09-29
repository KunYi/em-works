//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  uce.h
//
//  Implements the update command engine.
//
//-----------------------------------------------------------------------------
#ifndef _UCE_H__
#define _UCE_H__


#define UCE_FAIL(code)      (0x80000000 | (code))
#define UCE_FAIL_BADCOMMAND 0xBAD20000
#define UCE_FAIL_TOOLARGE   0xBAD20001
#define UCE_FAIL_TIMEOUT    0xBAD20002
#define UCE_FAIL_NOPAYLOAD  0xBAD20003

#define UCE_IDLE                            0
#define UCE_GET_DEVICE_INFO                 1
#define UCE_BEGIN_WRITE_FIRMWARE            2
#define UCE_BEGIN_WRITE_FILE                3
#define UCE_GET_FILENAME                    4
#define UCE_SEND_FIRMWARE_DATA              5
#define UCE_SEND_FILE_DATA                  6
#define UCE_QUEUE_STORE_STATUS              7
#define UCE_STORE_READY                     8
#define UCE_OTP_PROGRAM                     9



typedef struct _UCE_CONT
{
    int busy;                       //!< Command busy status
    int result;                     //!< Result of last command
    DWORD cmdArg;                    //!< Pre-parsed command argument
    /*union
    {
        uce_media_cmd_t media;          //!< Media command context
        uce_drive_cmd_t drive;          //!< Drive command context
    }
    cmd; */                               //!< Command dependent context
    //void (*Payload)(utp_message_t *);   //!< Payload message handler
}UCE_CONT, *PUCE_CONT;

extern UTP_MSG_REPLAY g_UTPMsgReply;

void SetUTPMsgReply( 
    BYTE ReplyCode, 
    DWORD ReplyInfo
);

void UceCommandDeal(
    PUTP_MSG pUTPMsg,
    PBYTE pCmd,
    DWORD CmdLength
);

void UceTransData(
    PBYTE pbData,
    DWORD DataLength
);

DWORD UtpMessagePoll(
    PUTP_MSG pUTPMsg
);


#endif \\_UCE_H__


