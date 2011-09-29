//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  utp.h
//
//  Implements the Update Transport Protocol (UTP) message handler.
//
//-----------------------------------------------------------------------------
#ifndef _UTP_H__
#define _UTP_H__

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define SCSI_UTP				0xF0

//Param and reply values for the POLL message
#define UTP_POLL_VERSION    1
#define UTP_INFO_VERSION    1

#define SCSI_ASC_VENDOR_SPECIFIC    	0x0080
#define	SENSE_DATA_INFORMATION_VALID	0
#define	SCSI_SENSE_KEY_VENDOR_SPECIFIC	0x09
#define	SENSE_DATA_ILI_CORRECT			0

//Error codes for the EXIT reply
#define UTP_FAIL_MSG_TYPE   0xBAD10000
#define UTP_FAIL_MSG_SEQ    0xBAD10001
#define UTP_FAIL_MSG_PARAM  0xBAD10002
#define UTP_FAIL_MSG_SIZE   0xBAD10003

//UTP message operation codes
#define UTP_MSG_OPCODE_POLL   0
#define UTP_MSG_OPCODE_EXEC   1
#define UTP_MSG_OPCODE_GET    2
#define UTP_MSG_OPCODE_PUT    3

// UTP message status codes
#define UTP_MSG_REPLAY_PASS   0
#define UTP_MSG_REPLAY_EXIT   1
#define UTP_MSG_REPLAY_BUSY   2
#define UTP_MSG_REPLAY_SIZE   3

#include <pshpack1.h>

//UTP context definition
typedef struct _utp_sanity
{
    DWORD SeqNum;  //The sequence number is used to confirm the order of payload messages. 
	    			//The host resets the sequence number to 0 when starting a new transaction. 
	    			//The number is incremented for each subsequent payload message.
    DWORD MsgTag;  // Expected GET/PUT message sequence number
}UTP_SANITY, *PUTP_SANITY;

//UTP context definition
typedef struct _utp_context
{
    DWORD SeqNum;  //The sequence number is used to confirm the order of payload messages. 
	    			//The host resets the sequence number to 0 when starting a new transaction. 
	    			//The number is incremented for each subsequent payload message.
    DWORD Reserved;  // Expected GET/PUT message sequence number
}UTP_CONT, *PUTP_CONT;

//UTP parameter definition
typedef union _UTP_PARAM
{
    UINT64   PayloadSize;
	UTP_CONT UTPCont;
}UTP_PARAM, *PUTP_PARAM;

//UTP message definition
typedef struct _UTP_MSG
{
	BYTE		OpCode;
	BYTE		MsgType;
    DWORD		MsgTag;        //Used to group messages belonging to the same UTP transaction.
    						//The host increments the tag when starting a new transaction. 
    						//It is included for sanity checking only
    UTP_PARAM	Param;      // Message parameter
    UINT16		Reserved;
}UTP_MSG, *PUTP_MSG;

//UTP reply definition
typedef struct _UTP_MSG_REPLAY
{
    BYTE  Reply;      // Status code
    UINT64 Info;          // Information field
}UTP_MSG_REPLAY, PUTP_MSG_REPLAY;

#include <poppack.h>


////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////
//void UtpSendBody(USBMSC_DEVICE * pDev, utp_message_t * pMsg);
//void UtpReceiveBody(USBMSC_DEVICE * pDev, utp_message_t * pMsg);
DWORD SwapULONG(DWORD value);


void
UtpMessagePreProc(
	PTRANSPORT_COMMAND ptcCommand,
	PBOOL pfDataStageRequired, 
	PDWORD pdwDirection, 
	PDWORD pdwDataSize
);

DWORD 
UtpMessageHandler(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
);

void 
UTPCmdResponse(
	PSENSE_DATA pSenseData
);

#endif // _UTP_H__
