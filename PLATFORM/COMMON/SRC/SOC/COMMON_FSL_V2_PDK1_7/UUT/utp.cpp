//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  utp.cpp
//
//  Implements the Update Transport Protocol (UTP) message handler.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:  6287 6262 4201 4512 4100 4115 4214)
#include <windows.h>
#pragma warning(pop)

#include <devload.h>
#include "proxy.h"
#include "scsi2.h"
#include "utp.h"
#include "uce.h"

////////////////////////////////////////////////////////////////////////////////
// Data
////////////////////////////////////////////////////////////////////////////////
BOOL g_bUTPMsgReplyInfoNeeded = FALSE;

static UTP_SANITY g_UTPSanityInfo;

DWORD SwapULONG(DWORD value)
{
	DWORD SwappedValue;

	SwappedValue = ((value>>24) & 0xFF);
	SwappedValue += ((value>>16) & 0xFF)<<8;
	SwappedValue += ((value>>8) & 0xFF)<<16;	
	SwappedValue += (value & 0xFF)<<24;

	return SwappedValue;
	
}

UINT64 SwapUINT64(UINT64 value)
{
	UINT64 SwappedValue;

	SwappedValue = SwapULONG((DWORD)(value));
	SwappedValue = SwappedValue <<32;
	SwappedValue += SwapULONG((DWORD)(value>>32));

	return SwappedValue;
	
}


DWORD UtpMessageExec(
	PUTP_MSG pUTPMsg,
    PTRANSPORT_DATA ptdData
)
{
	PBYTE pbData = (PBYTE) ptdData->DataBlock;
	ptdData->TransferLength = ptdData->RequestLength;

	//We should refresh the sanity flag once a new utp EXEC message arrives. 
 	g_UTPSanityInfo.MsgTag = pUTPMsg->MsgTag;
	g_UTPSanityInfo.SeqNum = 0;//pUTPMsg->Param.UTPCont.SeqNum;
	//RETAILMSG(1, (_T("MsgTag = 0x%x. SeqNum = 0x%x.\r\n"), pUTPMsg->MsgTag, 0)); 
	
	UceCommandDeal(pUTPMsg,pbData,ptdData->TransferLength);

    if (UTP_MSG_REPLAY_PASS == g_UTPMsgReply.Reply)
    {
		return EXECUTE_PASS;
    }
	else{
		return EXECUTE_FAIL;		
	}
}

DWORD UtpMessageTransData(
	PUTP_MSG pUTPMsg,
    PTRANSPORT_DATA ptdData
)
{
    PBYTE pbData = (PBYTE) ptdData->DataBlock;	
	ptdData->TransferLength = ptdData->RequestLength;
	//RETAILMSG(1, (_T("Data length to be transferred:  %x.\r\n"), ptdData->RequestLength)); 	
	//RETAILMSG(1, (_T("Data transfer packet: MsgTag = 0x%x. SeqNum = 0x%x.\r\n"), pUTPMsg->MsgTag, pUTPMsg->Param.UTPCont.SeqNum));
    if ((pUTPMsg->MsgTag == g_UTPSanityInfo.MsgTag) && (pUTPMsg->Param.UTPCont.SeqNum == g_UTPSanityInfo.SeqNum))
    {
    	if(pUTPMsg->MsgType == UTP_MSG_OPCODE_GET){
			ZeroMemory(pbData, ptdData->RequestLength);
    	}
		
		UceTransData(pbData, ptdData->TransferLength);

		g_UTPSanityInfo.SeqNum++;
    }
    else
    {
    	RETAILMSG(1, (_T("UTP message mismatched!\r\n")));
		SetUTPMsgReply(UTP_MSG_REPLAY_EXIT, UTP_FAIL_MSG_SEQ);
    }
	return EXECUTE_PASS;	
}

////////////////////////////////////////////////////////////////////////////////
// \brief  Prepares a UTP message response.
//
// This
//
// \param[in]  pDev
// \param[in]  pMsg
////////////////////////////////////////////////////////////////////////////////
void 
UTPCmdResponse(
	PSENSE_DATA pSenseData
)
{
    // Map the UTP message response onto the SCSI sense data
    pSenseData->Valid = SENSE_DATA_INFORMATION_VALID;
    pSenseData->SenseKey = SCSI_SENSE_KEY_VENDOR_SPECIFIC;
	*(DWORD *)pSenseData->Information = SwapULONG((DWORD)g_UTPMsgReply.Info);	  
	*(DWORD *)pSenseData->CommandSpecificInformation = SwapULONG((DWORD)(g_UTPMsgReply.Info>>(sizeof(DWORD)*8)));	
	pSenseData->AdditionalSenseCode = 0x80;
	pSenseData->AdditionalSenseCodeQualifier = g_UTPMsgReply.Reply;		    
}

void
UtpMessagePreProc(
	PTRANSPORT_COMMAND ptcCommand,
	PBOOL pfDataStageRequired, 
	PDWORD pdwDirection, 
	PDWORD pdwDataSize
)
{
	PUTP_MSG pUtpMsg = (PUTP_MSG) ptcCommand->CommandBlock;    
    *pfDataStageRequired = FALSE;
    *pdwDirection = 0;
    *pdwDataSize = 0; 	
    
    switch (pUtpMsg->MsgType)
    {
		case UTP_MSG_OPCODE_POLL: 
			//RETAILMSG(1, (_T("Utp Poll Message received.\r\n")));
			break;
		case UTP_MSG_OPCODE_EXEC: 
			//RETAILMSG(1, (_T("Utp Exec Message received.\r\n")));
		    *pfDataStageRequired = TRUE;
		    *pdwDirection = DATA_OUT;			
			break;
		case UTP_MSG_OPCODE_GET: 
			//RETAILMSG(1, (_T("Utp Get Message received.\r\n")));
		    *pfDataStageRequired = TRUE;
		    *pdwDirection = DATA_IN;
			break;
		case UTP_MSG_OPCODE_PUT: 
			//RETAILMSG(1, (_T("Utp Put Message received.\r\n")));
		    *pfDataStageRequired = TRUE;
		    *pdwDirection = DATA_OUT;			
			break;
      default:
	        break;
    }
}
////////////////////////////////////////////////////////////////////////////////
// \brief  Implements UTP message handler.
//
// This
//
// \param[in]  ptcCommand    Indicates the updater device index
// \param[in]  pCdb            Pointer to buffer containing the SCSI CDB
////////////////////////////////////////////////////////////////////////////////
DWORD 
UtpMessageHandler(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
)
{
	//UNREFERENCED_PARAMETER(ptdData);
	PUTP_MSG pUtpMsg = (PUTP_MSG) ptcCommand->CommandBlock;
	
	DWORD fRet = EXECUTE_FAIL;
	
	pUtpMsg->MsgTag = SwapULONG(pUtpMsg->MsgTag);
	pUtpMsg->Param.PayloadSize= SwapUINT64(pUtpMsg->Param.PayloadSize);

	//RETAILMSG(1, (_T("Utp Message #%x:\r\n"), pUtpMsg->MsgTag));

    // Call appropriate message handler
    switch (pUtpMsg->MsgType)
    {
		case UTP_MSG_OPCODE_POLL: 
			fRet = UtpMessagePoll(pUtpMsg); 
			break;
			
		case UTP_MSG_OPCODE_EXEC: 
			fRet = UtpMessageExec(pUtpMsg, ptdData); 
			break;
			
		case UTP_MSG_OPCODE_GET: 
		case UTP_MSG_OPCODE_PUT: 
			fRet = UtpMessageTransData(pUtpMsg, ptdData); 
			break;
			
      default:
	        SetUTPMsgReply(UTP_MSG_REPLAY_EXIT, UTP_FAIL_MSG_TYPE);
	        break;
    }

    // Prepare a message response if needed
    if (UTP_MSG_REPLAY_PASS != g_UTPMsgReply.Reply)
    {
		g_bUTPMsgReplyInfoNeeded = TRUE;
    }
    return fRet;
}
