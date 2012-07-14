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
#include "security.h"
#pragma warning(pop)

#define    MAX_FILE_NAME_LENGTH    256
#define    MAX_STORE_NAME_LENGTH   20
#define    STORE_CHECK_LOOP_NUM    0x1000
#define    MIN_UTP_DATA_PACKAGE_SIZE  0x10000//The minimun UTP data package size.

    
////////////////////////////////////////////////////////////////////////////////
// Global variable
////////////////////////////////////////////////////////////////////////////////
UTP_MSG_REPLAY g_UTPMsgReply;
BYTE    g_UtpCmdState = UCE_IDLE;
BOOL    g_PartialTransfer = TRUE;
DWORD   g_RecvDataNum=0;
DWORD   g_Payloadsize, g_PayloadFinished, g_MinBufSize,g_StartAddr;
DWORD   g_lastPercentage = 0;
UceFileType g_FileType;
PBYTE   g_PayloadBuf, g_pCurPayloadBuf;
TCHAR   g_FileName[MAX_FILE_NAME_LENGTH];
TCHAR   g_FolderName[MAX_STORE_NAME_LENGTH];
TCHAR   g_DiskName[6];
HANDLE  g_hFile=INVALID_HANDLE_VALUE;

//
// CS&ZHL MAY09-2012: add security info
//
VENDOR_SECURITY_INFO  g_SecurityInfo;

static DWORD dwStartTime=0, dwElapsedTime=0;

extern DWORD   g_MediaType;
extern NANDWrtImgInfo g_NANDWrtImgInfo;
extern fwType  g_fwType;

extern BOOL BSPUceCmdDeal(char * pbCmd);
extern BOOL BSPUceTransDeal(PBYTE pbData,  DWORD DataLength);


////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

UINT32 Char2Int32(char *pBuff, DWORD BitCount)
{
    UINT32 ret = 0;
    char data[]="0123456789ABCDEFabcdef";
    for(UINT32 j=0; j<BitCount; j++)
    {  
        char temp = *(pBuff+j);
        if(strchr(data,temp) <0)
            return ret;
            
        if(temp <= '9')
        {
            temp -= '0'; 
        }
        else if(temp <= 'F')
        {
            temp -= 'A'-0xa;
        }
        else
        {
            temp -= 'a'-0xa;
        }                  
    
        ret <<= 4;                
        ret += (UINT32)temp;
    }    
    return ret;
} 


BOOL IsFolderReady()
{
    WIN32_FIND_DATA FindFileData;
        
    HANDLE pFFF =  FindFirstFile(
      (L"\\*"), 
      &FindFileData 
    ); 
        
    if(!wcsncmp(FindFileData.cFileName, g_FolderName, sizeof(g_FolderName))){
        goto FIND_STORE;
    }    
        
    
    while(FindNextFile(pFFF, &FindFileData )){
        if(!wcsncmp(FindFileData.cFileName, g_FolderName, sizeof(g_FolderName))){
            goto FIND_STORE;
        }
        else{
             RETAILMSG(1, (_T("Found %s\r\n"),FindFileData.cFileName));
        }        
    }

    RETAILMSG(1, (_T("%s can not access.\r\n"),g_FolderName));
    FindClose(pFFF);
    return FALSE;
        
FIND_STORE:

    RETAILMSG(1, (_T("%s disk is ready.\r\n"),FindFileData.cFileName));
    FindClose(pFFF);
    return TRUE;
}

BOOL Cmd_QueryFolderStatus(char * pbCmd,DWORD CmdLength)
{
    DWORD dwValidLength;
    char * temp;
    DWORD tempLength;
    TCHAR timeout[10];
    INT iTimeout;
    INT iTime;

    RETAILMSG(1, (_T("UTP command:Check if folder is ready.\r\n")));
    RETAILMSG(1, (_T("Please be patient to wait...\r\n")));
    
    //Store the Folder name
    memset(g_FolderName,0,MAX_STORE_NAME_LENGTH);
    temp = strstr(pbCmd,",");
    tempLength = temp - pbCmd;
    dwValidLength = ((tempLength - strlen("QueryFolderStatus:")) > MAX_STORE_NAME_LENGTH-2)? MAX_STORE_NAME_LENGTH-2: (tempLength - strlen("QueryFolderStatus:"));
    mbstowcs(g_FolderName, (pbCmd + strlen("QueryFolderStatus:")), dwValidLength);
    RETAILMSG(1, (_T("Send folder name: %s.\r\n"),g_FolderName));

    //Store the timeout
    memset(timeout,0,10);
    temp = strstr(pbCmd,"Timeout:");
    tempLength = (temp - pbCmd) + strlen("Timeout:");
    if(CmdLength <= tempLength)
        return FALSE;
    mbstowcs(timeout, (pbCmd + tempLength), CmdLength - tempLength);        
    iTimeout = _wtoi(timeout);
    RETAILMSG(1, (_T("Send time out: %d.\r\n"),iTimeout));
    
    g_UtpCmdState = UCE_QUERY_STORE_STATUS;
    iTime = 0;
    while(!IsFolderReady())
    {
        if(iTime == iTimeout)
            return FALSE;    
        iTime ++;
        Sleep(1000);//wait 1s for folder ready
    }
    g_UtpCmdState = UCE_STORE_READY; 
    return TRUE;
}

BOOL IsDeviceReady()
{
    if(!GetDeviceHandle(g_DiskName))
        return FALSE;
    
    RETAILMSG(1, (_T("%s is ready.\r\n"),g_DiskName));
    return TRUE;
}

BOOL Cmd_QueryStoreStatus(char * pbCmd,DWORD CmdLength)
{
    char * temp;
    DWORD tempLength;
    TCHAR timeout[10];
    INT iTimeout;
    INT iTime;
    
    RETAILMSG(1, (_T("UTP command:Check if disk is ready.\r\n")));
    RETAILMSG(1, (_T("Please be patient to wait...\r\n")));
    
    //Store the disk name, i.e. DSK1:
    memset(g_DiskName,0,6);
    temp = strstr(pbCmd,",");
    tempLength = temp - pbCmd;
    mbstowcs(g_DiskName, (pbCmd + strlen("QueryStoreStatus:")), tempLength - strlen("QueryStoreStatus:"));
    RETAILMSG(1, (_T("Send storage disk name: %s.\r\n"),g_DiskName));
    
    //Store the timeout
    memset(timeout,0,10);
    temp = strstr(pbCmd,"Timeout:");
    tempLength = (temp - pbCmd) + strlen("Timeout:");
    if(CmdLength <= tempLength)
        return FALSE;
    mbstowcs(timeout, (pbCmd + tempLength), CmdLength - tempLength);  
    iTimeout = _wtoi(timeout);
    RETAILMSG(1, (_T("Send time out: %d.\r\n"),iTimeout));

    g_UtpCmdState = UCE_QUERY_STORE_STATUS;
    iTime = 0;
    while(!IsDeviceReady())
    {
        DEBUGMSG(1, (_T("Wait for Disk %s ready...\r\n"),g_DiskName));
        if(iTime == iTimeout)
            return FALSE;    
        iTime++;
        Sleep(1000);//wait 1s for device ready
    }

    g_UtpCmdState = UCE_STORE_READY;
    return TRUE;
}

BOOL Cmd_QueryStoreName(char * pbCmd,DWORD CmdLength)
{
    char * temp;
    DWORD tempLength;
    TCHAR timeout[10];
    INT iTimeout=0;
    INT iTime=0;
    BOOL bFind = FALSE;
    STOREINFO si = {0};
    HANDLE hFind = INVALID_HANDLE_VALUE;
    si.cbSize = sizeof(STOREINFO);
    TCHAR* szFriendlyName=NULL;
        
    RETAILMSG(1, (_T("UTP command:Check if disk is ready.\r\n")));
    RETAILMSG(1, (_T("Please be patient to wait...\r\n")));

    //Store the Friendly Name, i.e. SD Memory Card or NAND FLASH Storage
    szFriendlyName = (TCHAR*) LocalAlloc(LPTR, MAX_PATH);
    if(szFriendlyName == NULL)
        goto Exit;
    memset(szFriendlyName,0,MAX_PATH);
    temp = strstr(pbCmd,",");
    tempLength = temp - pbCmd;
    mbstowcs(szFriendlyName, (pbCmd + strlen("QueryStoreName:")), tempLength - strlen("QueryStoreName:"));
    RETAILMSG(1, (_T("Send storage Friendly name: %s.\r\n"),szFriendlyName));
   
    //Store the timeout
    memset(timeout,0,10);
    temp = strstr(pbCmd,"Timeout:");
    tempLength = (temp - pbCmd) + strlen("Timeout:");
    if(CmdLength <= tempLength)
        goto Exit;
    mbstowcs(timeout, (pbCmd + tempLength), CmdLength - tempLength);  
    iTimeout = _wtoi(timeout);
    RETAILMSG(1, (_T("Send time out: %d.\r\n"),iTimeout));

    for(iTime=0;iTime<iTimeout;iTime++)
    {
        // enumerate first store
        hFind = FindFirstStore(&si);
        
        if(INVALID_HANDLE_VALUE != hFind)
        {
            do
            {
                RETAILMSG(1,(_T("Find Store Device Name is %s\r\n"),si.szDeviceName));
                RETAILMSG(1,(_T("Find Store Friendly Name is %s\r\n"),si.szStoreName));
                if(wcscmp(szFriendlyName,si.szStoreName)==0)
                {
                    bFind = TRUE;
                    break;
                }
            }
            while(FindNextStore(hFind, &si));
            FindClose(hFind);
        }

        if(bFind == TRUE)
        {
            RETAILMSG(1,(_T("Info:Find Store by Friendly Name %s\r\n"),si.szStoreName));
            break;
        }
    }

    if(bFind == FALSE)
    {
        RETAILMSG(1,(_T("INFO:There is no store Device find!\r\n")));
        goto Exit;
    }
    
    //Store the disk name, i.e. DSK1:
    memset(g_DiskName,0,6);
    wcscpy(g_DiskName, si.szDeviceName);
    RETAILMSG(1, (_T("Get Store disk name:%s by Friendly name %s.\r\n"),g_DiskName,szFriendlyName));

    g_UtpCmdState = UCE_QUERY_STORE_STATUS;
    if(!IsDeviceReady())
        goto Exit;
    
    if(szFriendlyName != NULL)
        LocalFree(szFriendlyName);
    g_UtpCmdState = UCE_STORE_READY;
    return TRUE;

Exit:
    if(szFriendlyName != NULL)
        LocalFree(szFriendlyName);
    return FALSE;
}

BOOL Cmd_Partitions(char * pbCmd,DWORD CmdLength)
{
    char *pCmd = NULL;
    char *cmd[4];//MBR at most define 4 partitions
    char *tmp;
    char *partSize = NULL;
    INT tmpLength;
    PARTITION_CONT partCont={0};
    INT iPartNum;
    
    RETAILMSG(1, (_T("UTP command:Creating partitions.\r\n")));
    RETAILMSG(1, (_T("Please be patient to wait...\r\n")));
    
    pCmd = (char*) LocalAlloc(LPTR, MAX_PATH);
    if(pCmd == NULL)
        goto Exit;
    partSize = (char*) LocalAlloc(LPTR, MAX_PATH);
    if(partSize == NULL)
        goto Exit;

    ZeroMemory(pCmd,MAX_PATH);
    if(CmdLength <= strlen("Partitions:"))
        goto Exit;
    strncpy(pCmd,pbCmd + strlen("Partitions:"),CmdLength - strlen("Partitions:"));
    pCmd[MAX_PATH-1]='\0';
    iPartNum = 0;
    cmd[iPartNum] = strtok(pCmd,",");
    while(cmd[iPartNum])
    {
        iPartNum++;
        cmd[iPartNum] = strtok(NULL,",");
    }
    RETAILMSG(1, (_T("There are %d partitions.\r\n"),iPartNum));
    for(INT i=0;i<iPartNum;i++)
    {
        ZeroMemory(partSize,MAX_PATH);
        if( strncmp(cmd[i],"Firmware:",strlen("Firmware:"))==0 )
        {
            tmp = strstr(cmd[i],"MB");
            tmpLength = tmp - cmd[i] - strlen("Firmware:");
            strncpy(partSize,cmd[i]+strlen("Firmware:"),tmpLength);
            partCont.iPartSize[i] = atoi(partSize);
            wcsncpy(partCont.PartName[i],_T("Firmware"),wcslen(_T("Firmware")));
        }
        else if( strncmp(cmd[i],("EBOOT:"),strlen("EBOOT:"))==0 )
        {
            tmp = strstr(cmd[i],"MB");
            tmpLength = tmp - cmd[i] - strlen("EBOOT:");
            strncpy(partSize,cmd[i]+strlen("EBOOT:"),tmpLength);
            partCont.iPartSize[i] = atoi(partSize);
            wcsncpy(partCont.PartName[i],_T("EBOOT"),wcslen(_T("EBOOT")));
        }
        else if( strncmp(cmd[i],("NK:"),strlen("NK:"))==0 )
        {
            tmp = strstr(cmd[i],"MB");
            tmpLength = tmp - cmd[i] - strlen("NK:");
            strncpy(partSize,cmd[i]+strlen("NK:"),tmpLength);
            partCont.iPartSize[i] = atoi(partSize);
            wcsncpy(partCont.PartName[i],_T("NK"),wcslen(_T("NK")));
        }
        else if( strncmp(cmd[i],("File"),strlen("File"))==0 ) 
        {  
            wcsncpy(partCont.PartName[i],_T("File"),wcslen(_T("File")));
        }
        else
        {
            RETAILMSG(1, (_T("ERROR:No the Partition name!\r\n")));
            goto Exit;
        }

        RETAILMSG(1, (_T("%s Partition %d:%dMB done!\r\n"),partCont.PartName[i],i,partCont.iPartSize[i]));
    }

    LocalFree(pCmd);
    LocalFree(partSize);
    UceCreatePartitions(iPartNum,partCont);
    return TRUE;

Exit:
    if(pCmd != NULL)
        LocalFree(pCmd);
    if(partSize != NULL)
        LocalFree(partSize);
    return FALSE;
}

BOOL Cmd_fwtype(char * pbCmd)
{
    if(strncmp(pbCmd+strlen("fwtype:"),"XL_NB",strlen("XL_NB")) == 0){//xldr.nb0
        RETAILMSG(1, (_T("UTP command:Image type: XLDR NB0.\r\n")));    
        g_NANDWrtImgInfo.dwImgType = IMAGE_XLDR;
        g_fwType = fwType_XL_NB;
    }
    else if(strncmp(pbCmd+strlen("fwtype:"),"EB_NB",strlen("EB_NB")) == 0){//eboot.nb0
        RETAILMSG(1, (_T("UTP command:Image type: EBOOT NB0.\r\n")));
        g_NANDWrtImgInfo.dwImgType = IMAGE_EBOOT;
        g_fwType = fwType_EB_NB;
    }
    else if(strncmp(pbCmd+strlen("fwtype:"),"NK_NB",strlen("NK_NB")) == 0){//nk.nb0
        RETAILMSG(1, (_T("UTP command:Image type: NK NB0.\r\n")));
        g_NANDWrtImgInfo.dwImgType = IMAGE_NK;
        g_fwType = fwType_NK_NB;
    }
    else if(strncmp(pbCmd+strlen("fwtype:"),"EB_SB",strlen("EB_SB")) == 0){//eboot.sb
        RETAILMSG(1, (_T("UTP command:Image type: EBOOT SB.\r\n")));
        g_NANDWrtImgInfo.dwImgType = IMAGE_EBOOT;
        g_fwType = fwType_EB_SB;
    }
    else if(strncmp(pbCmd+strlen("fwtype:"),"NK_SB",strlen("NK_SB")) == 0){//nk.sb
        RETAILMSG(1, (_T("UTP command:Image type: NK SB.\r\n")));
        g_NANDWrtImgInfo.dwImgType = IMAGE_NK;
        g_fwType = fwType_NK_SB;
    }
    else{
        RETAILMSG(1, (_T("UTP command:No the image type.\r\n")));
        return FALSE;
    }  

    return TRUE;
}

BOOL Cmd_filename(char * pbCmd,DWORD CmdLength)
{
    DWORD dwValidLength;
    
    RETAILMSG(1, (_T("UTP command:File name to be sent: ")));        
    g_UtpCmdState = UCE_GET_FILENAME;
    
    //Store the file name
    dwValidLength = ((CmdLength-9) > MAX_FILE_NAME_LENGTH-2)? MAX_FILE_NAME_LENGTH-2: (CmdLength-9);
    mbstowcs(g_FileName, (pbCmd+9), dwValidLength);

    //Add an end to the string
    g_FileName[dwValidLength] = '\0';
    
    RETAILMSG(1, (_T("%s.\r\n"),g_FileName));
    
    if (!UceOpenFile(g_FileName, g_hFile))
        return FALSE;

    return TRUE;
}


BOOL Cmd_send(PUTP_MSG pUTPMsg)
{
    RETAILMSG(1, (_T("UTP command:Send data.\r\n")));
        g_UtpCmdState = UCE_RECV_FILE_DATA;
    g_MinBufSize = GetMinImgBufSize();
    DEBUGMSG(1, (_T("Minimun image buffer size is : 0x%x.\r\n"),g_MinBufSize));  
    
    //Here we can get the whole data length.
    g_Payloadsize = (DWORD)pUTPMsg->Param.PayloadSize;        
    RETAILMSG(1, (_T("Whole data length to be sent: 0x%x.\r\n"),g_Payloadsize));
    
    if(g_FileType == RawData)
    {
        RETAILMSG(1, (_T("Prepare to receive raw data...\r\n")));
    
        if(!UcePreWriteRawData(g_DiskName, g_StartAddr, g_Payloadsize))
        {
            return FALSE;
        }
    }  
    
    if(g_FileType == Firmware || g_FileType == RawData)
    {
        g_PartialTransfer = FALSE;
        
        DEBUGMSG(1,(_T("UceCommandDeal: g_Payloadsize is 0x%x. g_MinBufSize is 0x%x. \r\n"),g_Payloadsize,g_MinBufSize));
        //We should judge if required space excceed the max buffer size can be allocated.
        if(g_Payloadsize > g_MinBufSize || g_MediaType == MEDIA_NAND)
        {
            
            //We have to spin off the whole transfer to several fragments to meet the limitation of buffer size.
            g_PartialTransfer = TRUE;
            
            if(g_FileType == Firmware)
            {
                //Some necessary preparation is necessary.
                PrePartialWriteImage(g_DiskName, g_Payloadsize);
            }
            g_PayloadBuf = (PBYTE) LocalAlloc(LPTR, g_MinBufSize);
        }
        else
        {
            g_PayloadBuf = (PBYTE) LocalAlloc(LPTR, g_Payloadsize);                
        }         

        if (g_PayloadBuf == NULL) 
        {
            ERRORMSG(1, (_T("Failed to alloc enough memory space! Memory required: 0x%x bytes.\r\n"), g_Payloadsize));    
        }    
        g_pCurPayloadBuf = g_PayloadBuf;    
        DEBUGMSG(1,(_T("UceCommandDeal: g_PayloadBuf = g_pCurPayloadBuf is 0x%x. \r\n"),g_pCurPayloadBuf));
        g_RecvDataNum = 0;                           
    }
    else if( (g_FileType == UserFile) || (g_FileType == SecurityFile) )
    {
        RETAILMSG(1, (_T("Prepare to receive file data...\r\n")));
        g_PayloadFinished = 0;
    }        
    g_lastPercentage = 0;

    return TRUE;
}


BOOL Cmd_save()
{
    if(g_FileType == Firmware || g_FileType == RawData)
    {

        if(!g_PartialTransfer)
        {
            if(g_FileType == Firmware)
            {
                WriteImage(g_PayloadBuf,g_Payloadsize);
            }
            else if(g_FileType == RawData)
            {
                UceWriteRawData(g_PayloadBuf, g_Payloadsize);
            }
        }
        else
        {
            if(g_pCurPayloadBuf > g_PayloadBuf)
            {
                DWORD PartialWriteLoop = (g_pCurPayloadBuf - g_PayloadBuf)/g_MinBufSize;
                DEBUGMSG(1, (_T("UceCommandDeal->Save:g_pCurPayloadBuf is 0x%x, g_PayloadBuf is 0x%x\r\n"),g_pCurPayloadBuf,g_PayloadBuf));
                DEBUGMSG(1, (_T("UCECommandDeal->PartialWriteLoop is : 0x%x.\r\n"),PartialWriteLoop));

                if(PartialWriteLoop)
                {
                    DWORD i;                
                    DEBUGMSG(1, (_T("Cmd_save:PartialWriteLoop is : 0x%x.\r\n"),PartialWriteLoop));                
                    for(i=0; i<PartialWriteLoop;i++){                            
                        if(g_FileType == Firmware){
                            PartialWriteImage(g_PayloadBuf + (i*g_MinBufSize),g_MinBufSize);
                        }
                        else if(g_FileType == RawData){
                            UceWriteRawData(g_PayloadBuf + (i*g_MinBufSize),g_MinBufSize);
                        }                            
                    }
                }         
                
                //There still exists some data which is less than g_MinBufSize.
                if(g_pCurPayloadBuf > (g_PayloadBuf + PartialWriteLoop*g_MinBufSize))
                {
                    DEBUGMSG(1, (_T("Cmd_save:the last data need be sent.\r\n"))); 
                    if(g_FileType == Firmware)
                    {
                        PartialWriteImage(g_PayloadBuf,g_pCurPayloadBuf - (g_PayloadBuf + PartialWriteLoop*g_MinBufSize));
                    }
                    else if(g_FileType == RawData)
                    {
                        UceWriteRawData(g_PayloadBuf,g_pCurPayloadBuf - (g_PayloadBuf + PartialWriteLoop*g_MinBufSize));
                    }                     
                }                        
            }
            
            if(g_FileType == Firmware)
            {
                EndPartialWriteImage(); 
            }
            RETAILMSG(TRUE, (L"Recieving data: 100%% finished.\r\n"));

        }
        
        if (g_PayloadBuf)
        { 
            LocalFree(g_PayloadBuf);            
        }           
    }
    else if(g_FileType == UserFile)
    {
        RETAILMSG(TRUE, (L"Writing File data: 100%% is finished.\r\n"));
        UceCloseFile(g_hFile);    
    }
	/* CS&ZHL MAY10-2012: add command for processing the file security.nb0 
	                      in the case, processing the security receiveing data */
	else if( g_FileType == SecurityFile )
	{
        RETAILMSG(TRUE, (L"Security File data: 100%% is finished.\r\n"));
		// write security info through OTP
		RETAILMSG(TRUE, (L"Recv MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n", g_SecurityInfo.mac[0]&0xff, (g_SecurityInfo.mac[0]>>8 )&0xff, 
			                                                             g_SecurityInfo.mac[1]&0xff, (g_SecurityInfo.mac[1]>>8 )&0xff,
																		 g_SecurityInfo.mac[2]&0xff, (g_SecurityInfo.mac[2]>>8 )&0xff));
        RETAILMSG(TRUE, (L"Time: %04d-%02d-%02d\r\n", g_SecurityInfo.dwYear, g_SecurityInfo.dwMonth, g_SecurityInfo.dwDay ));
		UceWriteSecurityInfo( (PBYTE)&g_SecurityInfo, sizeof(g_SecurityInfo) );
	}
    g_UtpCmdState = UCE_IDLE;  

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

// CS&ZHL JUN-1-2012: code for EM9280 uce to check OTP mac
void UceCmdSecuritySataus(PBYTE pBuf)
{
	DWORD	dwCUST[2];

	char *p = (char *)pBuf;
    
	dwCUST[0] = dwCUST[1] = 0;
	// CS&ZHL JUN-1-2012: do otp check
	UceGetOTPInfo( dwCUST, 8 );
	
	if( (dwCUST[0]==0)&&(dwCUST[1]==0) )
	{
		strcpy( p, "0\n" );
	}
	else
	{
		sprintf( p, "1 - MAC:0x%08X Time:0x%08X\n", dwCUST[0], dwCUST[1] );
	}
	// end of CS&ZHL JUN-1-2012: do otp check

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
        /*if(g_UtpCmdState == UCE_QUERY_STORE_STATUS){
            BOOL fRet = FALSE;    
            
            //To invoke IsFolderReady() per 5 seconds.
            dwElapsedTime = GetTickCount() - dwStartTime;
            if(dwElapsedTime > 5000){
                fRet = IsFolderReady();
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
        }*/
        if(!g_UTPMsgReply.Info){
            SetUTPMsgReply(UTP_MSG_REPLAY_BUSY, DWORD(--g_UTPMsgReply.Info));
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

    SetUTPMsgReply(UTP_MSG_REPLAY_PASS, 0);    
    
    if (strncmp(pbCmd, "?", 1) == 0) {
        RETAILMSG(1, (_T("UTP command:Get device info.\r\n")));
        g_UtpCmdState = UCE_GET_DEVICE_INFO;
        SetUTPMsgReply(UTP_MSG_REPLAY_SIZE, 0x7F);    
    }
    if (strncmp(pbCmd, "s?", 2) == 0) {
        RETAILMSG(1, (_T("UTP command:Get device mac info.\r\n")));
        g_UtpCmdState = UCE_GET_DEVICE_MAC;
        SetUTPMsgReply(UTP_MSG_REPLAY_SIZE, 0x7F);    
    }
    else if (strncmp(pbCmd, "!2", 2) == 0) {
        RETAILMSG(1, (_T("UTP command:Reboot device.\r\n")));      
        KernelIoControl(IOCTL_HAL_REBOOT, NULL,0, NULL, 0, NULL);
        RETAILMSG(1, (_T("Reboot checking.\r\n")));
    }
    else if (strncmp(pbCmd, "MediaType:", 10) == 0) {
        if(strncmp(pbCmd+10,"NAND",4) == 0){//NAND
            RETAILMSG(1, (_T("UTP command:Media type: NAND.\r\n")));    
            g_MediaType = MEDIA_NAND;
        }
        else{//SDMMC
            RETAILMSG(1, (_T("UTP command:Media type: SDMMC.\r\n")));
            g_MediaType = MEDIA_SDMMC;
        }
    }     
    else if (strncmp(pbCmd, "QueryStoreStatus:", strlen("QueryStoreStatus:")) == 0){

        if(!Cmd_QueryStoreStatus(pbCmd, CmdLength))
            goto Exit;
    }
    else if (strncmp(pbCmd, "QueryStoreName:", strlen("QueryStoreName:")) == 0){
        if(!Cmd_QueryStoreName(pbCmd, CmdLength))
            goto Exit;
    }
    else if (strncmp(pbCmd, "QueryFolderStatus:", strlen("QueryFolderStatus:")) == 0){
        
        if(!Cmd_QueryFolderStatus(pbCmd, CmdLength))
            goto Exit;
    } 
    else if (strncmp(pbCmd, "Partitions:", strlen("Partitions:")) == 0){
        
        if(!Cmd_Partitions(pbCmd, CmdLength))
            goto Exit;
    } 
    else if (strncmp(pbCmd,"wfw",3) == 0) {
        RETAILMSG(1, (_T("UTP command:Write image.\r\n")));  
        g_UtpCmdState = UCE_BEGIN_RECV_FILE;
        g_FileType = Firmware;
    }
    else if (strncmp(pbCmd,"fwtype:",strlen("fwtype:")) == 0) {    

        if(!Cmd_fwtype(pbCmd))
            goto Exit;
    }    
    else if (strncmp(pbCmd,"wfl",3) == 0) {
        RETAILMSG(1, (_T("UTP command:Write file.\r\n")));    
        g_UtpCmdState = UCE_BEGIN_RECV_FILE;
        g_FileType = UserFile;
    }  
    else if (strncmp(pbCmd,"wrd",3) == 0) {    
        g_UtpCmdState = UCE_BEGIN_RECV_FILE;
        g_FileType = RawData;
        g_StartAddr = Char2Int32(pbCmd+6,(CmdLength-6));
        RETAILMSG(1, (_T("UTP command:Write raw data, starting address is 0x%x.\r\n"),g_StartAddr));        
    }    
	/* CS&ZHL MAY10-2012: add command for processing the file security.nb0 */
    else if (strncmp(pbCmd,"scr",3) == 0) {    
        g_UtpCmdState = UCE_BEGIN_RECV_FILE;
        g_FileType = SecurityFile;
        RETAILMSG(1, (_T("UTP command:Get security data \r\n") ));        
    }    
    else if (strncmp(pbCmd,"filename:",9) == 0) {

        if(!Cmd_filename(pbCmd,CmdLength))
            goto Exit;
    }
    else if (strncmp(pbCmd,"send",4) == 0) {
        Cmd_send(pUTPMsg);                 
    }  
    else if (strncmp(pbCmd,"save",4) == 0) {
        Cmd_save();
    }
    else if (strncmp(pbCmd,"Done",4) == 0) {
        RETAILMSG(1, (L"Whole update work is finished successfully. Please power off the board.\r\n"));
        g_UtpCmdState = UCE_IDLE;                    
    }        
    else{
        BSPUceCmdDeal(pbCmd);
        g_UtpCmdState = UCE_IDLE;
    }
    return;
    
Exit:
    SetUTPMsgReply(UTP_MSG_REPLAY_EXIT, UCE_FAIL_TOOLARGE);    
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

		case UCE_GET_DEVICE_MAC: 
			UceCmdSecuritySataus( pbData );
            g_UtpCmdState = UCE_IDLE;
            
			break;           
            
        case UCE_RECV_FILE_DATA:{
            switch (g_FileType){
                case Firmware:
                case RawData:{
                    //Copy data to g_PayloadBuf, trans 64K/0x10000
                    DWORD dwRestDataLength = g_Payloadsize - g_RecvDataNum;                       
                    DWORD dwValidDataLength = (dwRestDataLength > DataLength)? DataLength : dwRestDataLength;
                    g_RecvDataNum += dwValidDataLength;
                    DEBUGMSG(1, (_T("UceTransData: DataLength is : 0x%x.\r\n"),DataLength));
                    DEBUGMSG(1, (_T("dwValidDataLength is : 0x%x.\r\n"),dwValidDataLength));
                    
                    memcpy(g_pCurPayloadBuf, pbData, dwValidDataLength);
                    g_pCurPayloadBuf += dwValidDataLength;
                    DEBUGMSG(1, (_T("g_pCurPayloadBuf is : 0x%x.\r\n"),g_pCurPayloadBuf));
                    DEBUGMSG(1, (_T("g_pCurPayloadBuf - g_PayloadBuf = 0x%x.\r\n"),g_pCurPayloadBuf- g_PayloadBuf));
                    if(g_PartialTransfer) 
                    {
                        if(g_Payloadsize > g_MinBufSize)
                        {
                            DWORD PartialWriteLoop = (g_pCurPayloadBuf - g_PayloadBuf)/g_MinBufSize;                
                            if(PartialWriteLoop){
                                DWORD i;                
                                DEBUGMSG(1, (_T("PartialWriteLoop is : 0x%x.\r\n"),PartialWriteLoop));                
                                for(i=0; i<PartialWriteLoop;i++){
                                    if(g_FileType == Firmware){
                                        PartialWriteImage(g_PayloadBuf + (i*g_MinBufSize),g_MinBufSize);
                                    }
                                    else if(g_FileType == RawData){
                                        UceWriteRawData(g_PayloadBuf + (i*g_MinBufSize),g_MinBufSize);
                                    }                                     
                                }                                                        
                                //Go back the start of the buffer to receive new data
                                g_pCurPayloadBuf = g_PayloadBuf;                     
                            }
                        }
                        else
                        {
                            DEBUGMSG(1, (_T("UceTransData: g_Payloadsize is less than g_MinBufSize.\r\n")));
                            if(g_pCurPayloadBuf >= (g_PayloadBuf + g_Payloadsize)){
                                if(g_FileType == Firmware){
                                    PartialWriteImage(g_PayloadBuf,g_Payloadsize);
                                }
                                else if(g_FileType == RawData){
                                    UceWriteRawData(g_PayloadBuf,g_Payloadsize);
                                }                                
                                
                                //Go back the start of the buffer to receive new data
                                g_pCurPayloadBuf = g_PayloadBuf; 
                            }
                        }
                    }
                    if((100- (dwRestDataLength*100/g_Payloadsize)) > g_lastPercentage){
                        g_lastPercentage = 100- (dwRestDataLength*100/g_Payloadsize);
                        RETAILMSG(TRUE, (L"Recieving data: %2d%% finished.\r", g_lastPercentage));                    
                    }
                    break; 
                    }            
                case UserFile:{ 

                    DWORD dwRestDataLength = g_Payloadsize - g_PayloadFinished;
                    DWORD dwValidDataLength = (dwRestDataLength > DataLength)? DataLength : dwRestDataLength;

                    UceWriteFile(g_hFile, pbData, dwValidDataLength);
                    g_PayloadFinished += dwValidDataLength;
                    
                    RETAILMSG(TRUE, (L"Writing File : %2d%% is finished.\r", 100- (dwRestDataLength*100/g_Payloadsize)));
                    break;
                    }
				/* CS&ZHL MAY10-2012: add command for processing the file security.nb0 
				                      in the case, only receiveing data */
				case SecurityFile:{
					DWORD dwRestDataLength = g_Payloadsize - g_PayloadFinished;
					if( DataLength > dwRestDataLength )
						break;
					char *p = (char*)&g_SecurityInfo;
					memcpy( &p[g_PayloadFinished], pbData, DataLength );
                    g_PayloadFinished += DataLength;
                    RETAILMSG(TRUE, (L"Recv Security data length: %d .\r\n", DataLength));
					break;          
					}
                default:
                    break;
                }
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
    DEBUGMSG(1, (_T("UceTransData: return pass to upper layer.\r\n")));
    SetUTPMsgReply(UTP_MSG_REPLAY_PASS, 0);    
}

