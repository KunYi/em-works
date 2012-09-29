//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  uce.cpp
//
//  Implements the Update Transport Protocol (UTP) command.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:  6287 6262 4201 4512 4100 4115 4214)
#include <windows.h>
#include <Storemgr.h>
#include "winbase.h"
#include <devload.h>
#include "proxy.h"
#include "stdio.h"
#include "stdlib.h"
#include "scsi2.h"

#include "utp.h"
#include "uce.h"
#include "uce_media.h"
#include "common_nandfmd.h"
#pragma warning(pop)

#define    MAX_FILE_NAME_LENGTH    256
#define    MAX_STORE_NAME_LENGTH   20
#define    STORE_CHECK_LOOP_NUM    0x1000
#define    MAX_UTP_DATA_SIZE       0x10000
    
////////////////////////////////////////////////////////////////////////////////
// Global variable
////////////////////////////////////////////////////////////////////////////////
UTP_MSG_REPLAY g_UTPMsgReply;
BYTE    g_UtpCmdState = UCE_IDLE;
BOOL    g_PartialTransfer = TRUE;
DWORD   g_RecvDataNum=0;
DWORD   g_Payloadsize, g_PayloadFinished, g_MinBufSize;
PBYTE   g_PayloadBuf, g_pCurPayloadBuf;
TCHAR   g_FileName[MAX_FILE_NAME_LENGTH];
TCHAR   g_StoreName[MAX_STORE_NAME_LENGTH];
TCHAR   g_DiskName[6];
HANDLE  g_hFile=INVALID_HANDLE_VALUE;

static DWORD dwStartTime=0, dwElapsedTime=0;

extern DWORD   g_MediaType;
extern NANDWrtImgInfo g_NANDWrtImgInfo;
extern BOOL BSPUceCmdDeal(char * pbCmd);
extern BOOL BSPUceTransDeal(PBYTE pbData,  DWORD DataLength);


////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

BOOL IsStoreReady()
{
    WIN32_FIND_DATA FindFileData;
        
    HANDLE pFFF =  FindFirstFile(
      (L"\\*"), 
      &FindFileData 
    ); 
        
    if(!wcsncmp(FindFileData.cFileName, g_StoreName, sizeof(g_StoreName))){
        goto FIND_STORE;
    }    
        

    while(FindNextFile(pFFF, &FindFileData )){
        if(!wcsncmp(FindFileData.cFileName, g_StoreName, sizeof(g_StoreName))){
            goto FIND_STORE;
        }
    }
    
    FindClose(pFFF);
    return FALSE;
        
FIND_STORE:

    RETAILMSG(1, (_T("%s disk is ready.\r\n"),FindFileData.cFileName));
    FindClose(pFFF);
    return TRUE;
}

void SetUTPMsgReply( 
    BYTE ReplyCode, 
    DWORD ReplyInfo
)
{
    g_UTPMsgReply.Reply = ReplyCode; 
    g_UTPMsgReply.Info = ReplyInfo; 
} 

void UceCmdInfoPage(PBYTE pBuf)
{
    char *    utp_firmware_version = "WINCE6";
    char *    utp_sn = "000000000000";
    char *    utp_chipid = "370000A5";
    char *    utp_dce_version = "1.0";

    char *p = (char *)pBuf;

    //RETAILMSG(1, (_T("Sending device info+.\r\n")));
    sprintf(p, 
            "<DEVICE>\n"
            " <FW>%s</FW>\n"
            " <DCE>%s</DCE>\n"
            " <SN>%s</SN>"
            " <CID>%s</CID>"
            " <VID>%04X</VID>"
            " <PID>%04X</PID>"
            "</DEVICE>\n", 
            utp_firmware_version, utp_dce_version, utp_sn, utp_chipid, 0x66F, 0x37FF);

    SetUTPMsgReply(UTP_MSG_REPLAY_PASS, 0);    
}

// ----------------------------------------------------------------------------
// Function: UtpMessagePoll
//         Used to determine when an asynchronous 
//        device command is finished. Also used to query the UTP version. 
//
// Parameters:
//      pUTPMsg: UTP message need to be handled.
//        pg_UTPMsgReply: Reply for received UTP message 
// ----------------------------------------------------------------------------
DWORD UtpMessagePoll(
    PUTP_MSG pUTPMsg
)
{

    //If the type parameter for a Poll() message is 1, 
    //the device will return an EXIT reply with the result containing the UTP version.
    if (pUTPMsg->Param.UTPCont.SeqNum == UTP_POLL_VERSION)
    {
        SetUTPMsgReply(UTP_MSG_REPLAY_EXIT, UTP_INFO_VERSION);
    }
    else
    {
        if(g_UtpCmdState == UCE_QUEUE_STORE_STATUS){
            BOOL fRet = FALSE;    
            
            //To invoke IsStoreReady() per 5 seconds.
            dwElapsedTime = GetTickCount() - dwStartTime;
            if(dwElapsedTime > 5000){
                fRet = IsStoreReady();
                dwStartTime = dwElapsedTime;
            }
                
            if(fRet || !g_UTPMsgReply.Info){
                g_UtpCmdState = UCE_STORE_READY;
                SetUTPMsgReply(UTP_MSG_REPLAY_PASS, 0);
            }        
            else{
                SetUTPMsgReply(UTP_MSG_REPLAY_BUSY, DWORD(--g_UTPMsgReply.Info));
            }
        }
        else{
            SetUTPMsgReply(UTP_MSG_REPLAY_EXIT, 1);
        }
    }

    return EXECUTE_FAIL;
}

void UceCommandDeal(
    PUTP_MSG pUTPMsg,
    PBYTE pCmd,
    DWORD CmdLength
)
{
    char * pbCmd = (char *)pCmd;
    UNREFERENCED_PARAMETER(pUTPMsg);
    DWORD dwValidLength;
        
    SetUTPMsgReply(UTP_MSG_REPLAY_PASS, 0);    
    
    if (strncmp(pbCmd, "?", 1) == 0) {
        RETAILMSG(1, (_T("UTP command:Get device info.\r\n")));
        g_UtpCmdState = UCE_GET_DEVICE_INFO;
        SetUTPMsgReply(UTP_MSG_REPLAY_SIZE, 0x7F);    
    }
    else if (strncmp(pbCmd, "!2", 2) == 0) {
        RETAILMSG(1, (_T("UTP command:Reboot device.\r\n")));      
        KernelIoControl(IOCTL_HAL_REBOOT, NULL,0, NULL, 0, NULL);
        RETAILMSG(1, (_T("Reboot checking.\r\n")));
    }
    else if (strncmp(pbCmd, "MediaType:", 10) == 0) {
        if(strncmp(pbCmd+10,"NAND",4) == 0){
            RETAILMSG(1, (_T("UTP command:Media type: NAND.\r\n")));    
            g_MediaType = MEDIA_NAND;
        }
        else{//SDMMC
            RETAILMSG(1, (_T("UTP command:Media type: SDMMC.\r\n")));
            g_MediaType = MEDIA_SDMMC;
        }
    }    
    else if (strncmp(pbCmd, "StoreName:", 10) == 0) {
        //Store the disk name, i.e. DSK1:
        mbstowcs(g_DiskName, (pbCmd+10), 5);

        //Add an end to the string
        g_DiskName[5] = '\0';
        
        //Store the store name
        dwValidLength = ((CmdLength-15) > MAX_STORE_NAME_LENGTH-2)? MAX_STORE_NAME_LENGTH-2: (CmdLength-15);
        mbstowcs(g_StoreName, (pbCmd+15), dwValidLength);

        //Add an end to the string
        g_StoreName[dwValidLength] = '\0';
        
        RETAILMSG(1, (_T("UTP command:Send storage disk name: %s, folder name: %s.\r\n"),g_DiskName, g_StoreName));
    }    
    else if (strncmp(pbCmd, "QueueStoreStatus", 10) == 0) {
        RETAILMSG(1, (_T("UTP command:Check if disk is ready.\r\n")));
        RETAILMSG(1, (_T("Please be patient to wait...\r\n")));
        g_UtpCmdState = UCE_QUEUE_STORE_STATUS;
        if(IsStoreReady()){
            g_UtpCmdState = UCE_STORE_READY;
        }
        else{
            SetUTPMsgReply(UTP_MSG_REPLAY_BUSY, STORE_CHECK_LOOP_NUM);
            dwStartTime = GetTickCount();
        }
    }    
    else if (strncmp(pbCmd,"wfw",3) == 0) {
        RETAILMSG(1, (_T("UTP command:Write image.\r\n")));  
        g_UtpCmdState = UCE_BEGIN_WRITE_FIRMWARE;
    }
    else if (strncmp(pbCmd,"fwtype:",7) == 0) {    
        if(strncmp(pbCmd+7,"eb",2) == 0){
            RETAILMSG(1, (_T("UTP command:Image type: EBOOT.\r\n")));    
            g_NANDWrtImgInfo.dwImgType = IMAGE_EBOOT;
        }
        else if(strncmp(pbCmd+7,"nk",2) == 0){//NK
            RETAILMSG(1, (_T("UTP command:Image type: NK.\r\n")));
            g_NANDWrtImgInfo.dwImgType = IMAGE_NK;
        }
        else{
            RETAILMSG(1, (_T("UTP command:Image type: XLDR.\r\n")));
            g_NANDWrtImgInfo.dwImgType = IMAGE_XLDR;
        }        
    }    
    else if (strncmp(pbCmd,"wfl",3) == 0) {
        RETAILMSG(1, (_T("UTP command:Write file.\r\n")));    
        g_UtpCmdState = UCE_BEGIN_WRITE_FILE;
    }    
    else if (strncmp(pbCmd,"filename:",9) == 0) {
        RETAILMSG(1, (_T("UTP command:File name to be sent: ")));        
        g_UtpCmdState = UCE_GET_FILENAME;
        
        //Store the file name
        dwValidLength = ((CmdLength-9) > MAX_FILE_NAME_LENGTH-2)? MAX_FILE_NAME_LENGTH-2: (CmdLength-9);
        mbstowcs(g_FileName, (pbCmd+9), dwValidLength);

        //Add an end to the string
        g_FileName[dwValidLength] = '\0';
        
        RETAILMSG(1, (_T("%s.\r\n"),g_FileName));
        
        UceOpenFile(g_FileName, g_hFile);
    }
    else if (strncmp(pbCmd,"send",4) == 0) {
        RETAILMSG(1, (_T("UTP command:Send data.\r\n")));
        g_MinBufSize = GetMinImgBufSize(g_DiskName);
        if(g_MinBufSize%MAX_UTP_DATA_SIZE)
        {
            ERRORMSG(1, (_T("The mininum unit size of image is invalid, which should be divided by 0x%x.\r\n"),MAX_UTP_DATA_SIZE));
        } 
               
        //Here we can get the whole data length.
        g_Payloadsize = (DWORD)pUTPMsg->Param.PayloadSize;        
        RETAILMSG(1, (_T("Data length to be sent: 0x%x.\r\n"),g_Payloadsize));
            
        if(g_UtpCmdState == UCE_BEGIN_WRITE_FIRMWARE){
            //g_TotalDataLength = PreWriteImage(g_DiskName, g_Payloadsize);
            //RETAILMSG(1, (_T("Memory required: 0x%x bytes.\r\n"),g_TotalDataLength));
            g_PartialTransfer = FALSE;
            
            //We should judge if required space excceed the max buffer size can be allocated.
            if(g_Payloadsize > g_MinBufSize || g_MediaType == MEDIA_NAND){
                
                //We have to spin off the whole transfer to several fragments to meet the limitation of buffer size.
                g_PartialTransfer = TRUE;
                
                //Some necessary preparation is necessary.
                PrePartialWriteImage(g_DiskName, g_Payloadsize);
                g_PayloadBuf = (PBYTE) LocalAlloc(LPTR, g_MinBufSize);
            }
            else{
                g_PayloadBuf = (PBYTE) LocalAlloc(LPTR, g_Payloadsize);                
            }         

            if (g_PayloadBuf == NULL) {
                ERRORMSG(1, (_T("Failed to alloc enough memory space! Memory required: 0x%x bytes.\r\n"), g_Payloadsize));    
            }    
            g_pCurPayloadBuf = g_PayloadBuf;    
            g_RecvDataNum = 0;
                
            g_UtpCmdState = UCE_SEND_FIRMWARE_DATA;            
        }
        else{
            g_PayloadFinished = 0;        
            g_UtpCmdState = UCE_SEND_FILE_DATA;        
        }
    }
    else if (strncmp(pbCmd,"save",4) == 0) {
        if(g_UtpCmdState == UCE_SEND_FIRMWARE_DATA){
            RETAILMSG(1, (L"Receiveing Firmware data: 100%% is finished.\r\n"));
            
            if(!g_PartialTransfer){
                WriteImage(g_DiskName, g_PayloadBuf,g_Payloadsize);
            }
            else{
                if(g_pCurPayloadBuf - g_PayloadBuf){
                    PartialWriteImage(g_PayloadBuf,g_pCurPayloadBuf - g_PayloadBuf);
                }
                EndPartialWriteImage();
            }
            
            if (g_PayloadBuf){ 
                LocalFree(g_PayloadBuf);            
            }
        }
        else{
            RETAILMSG(TRUE, (L"Writing File data: 100%% is finished.\r\n"));
            UceCloseFile(g_hFile);    
        }    
        g_UtpCmdState = UCE_IDLE;                    
    }                  
    else if (strncmp(pbCmd,"Done",4) == 0) {
        RETAILMSG(1, (L"Whole update work is finished successfully. Please power off the board.\r\n"));
        g_UtpCmdState = UCE_IDLE;                    
    }        
    else{
        BSPUceCmdDeal(pbCmd);
        g_UtpCmdState = UCE_IDLE;
    }
}

void UceTransData(
    PBYTE pbData,
    DWORD DataLength
)
{
    // Call the appropriate command handler
    switch (g_UtpCmdState)
    {
        case UCE_GET_DEVICE_INFO: 
            UceCmdInfoPage(pbData);
            g_UtpCmdState = UCE_IDLE;
            break;           
            
        case UCE_SEND_FIRMWARE_DATA:{
            //Copy data to g_PayloadBuf
            DWORD dwRestDataLength = g_Payloadsize - g_RecvDataNum;
            DWORD dwValidDataLength = (dwRestDataLength > DataLength)? DataLength : dwRestDataLength;
            g_RecvDataNum += dwValidDataLength;
            
            memcpy(g_pCurPayloadBuf, pbData, dwValidDataLength);
            g_pCurPayloadBuf += dwValidDataLength;
            
            if(g_PartialTransfer && (g_pCurPayloadBuf == (g_MinBufSize + g_PayloadBuf))){
                //RETAILMSG(TRUE, (L"Do data burning...                           \r"));
                PartialWriteImage(g_PayloadBuf,g_MinBufSize);
                //Go back the start of the buffer to receive new data
                g_pCurPayloadBuf = g_PayloadBuf;
            }

            RETAILMSG(TRUE, (L"Receiveing Image data: %2d%% is finished.\r", 100- (dwRestDataLength*100/g_Payloadsize)));
            break; 
            }            
            
        case UCE_SEND_FILE_DATA:{ 

            DWORD dwRestDataLength = g_Payloadsize - g_PayloadFinished;
            DWORD dwValidDataLength = (dwRestDataLength > DataLength)? DataLength : dwRestDataLength;

            UceWriteFile(g_hFile, pbData, dwValidDataLength);
            g_PayloadFinished += dwValidDataLength;
            
            RETAILMSG(TRUE, (L"Writing File : %2d%% is finished.\r", 100- (dwRestDataLength*100/g_Payloadsize)));
            break;
            }

        /*case UCE_GET_FIRMWARE_DATA: 
        
            break;    
            
        case UCE_GET_FILE_DATA: 
            UceMediaRead(pbData,DataLength);
            break;*/                    

        default:
            SetUTPMsgReply(UTP_MSG_REPLAY_EXIT, UCE_FAIL_BADCOMMAND);                
            break;
    }
    BSPUceTransDeal(pbData,  DataLength);
    SetUTPMsgReply(UTP_MSG_REPLAY_PASS, 0);    
}

