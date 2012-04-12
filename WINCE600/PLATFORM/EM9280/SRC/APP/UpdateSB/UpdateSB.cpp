//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
#include <windows.h>
#include <ceddk.h>
#include <storemgr.h>
#include "common_nandfmd.h"
#include "updatesb.h"
#include "ioctl_cfg.h"
#include "sdmmc.h"

#pragma warning(disable: 4701)

BootMode    g_bm;

void RebootSystem(BOOL FirstBoot)
{
    BootMode boot;
    
    boot.source = FirstBoot == TRUE ? First : Second;
    
    if(!KernelIoControl(IOCTL_HAL_SET_BOOT_SOURCE, NULL, 0, &boot, sizeof(BootMode), NULL))
    {
        ERRORMSG(1, (TEXT("IOCTL_HAL_SET_BOOT_SOURCE failed!\r\n")));
        return;
    }
    
    RETAILMSG(1, (_T("rebooting... ...\r\n")));
    Sleep(1000);
    KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);
}

HANDLE FindAndOpenDSK(LPTSTR Profile, DWORD dwSize, PSTOREINFO pSI)
{
    HANDLE hStore;
    BOOL bResult = TRUE;
    
    // 1. find the correct device
    pSI->cbSize = sizeof(STOREINFO);
    hStore = FindFirstStore(pSI);
    
    if(!hStore)
    {
        ERRORMSG(TRUE, (_T("Failed to find first store\r\n")));
        return NULL;
    }
    
    while(memcmp(CharUpper(pSI->sdi.szProfile), Profile, dwSize))
    {
        if(!FindNextStore(hStore, pSI))
        {
            bResult = FALSE;
            break;
        }
    }    
    
    if(!bResult)
    {
        CloseHandle(hStore);
        return NULL;
    }
    RETAILMSG(TRUE, (_T("find the target store: %s\r\n"), pSI->sdi.szProfile));
    RETAILMSG(TRUE, (_T("DeviceName=%s\r\n"), pSI->szDeviceName));
    RETAILMSG(TRUE, (_T("dwBytesPerSector=0x%x\r\n"), pSI->dwBytesPerSector));
    
    CloseHandle(hStore);
   
    // 2. Open the device
    return CreateFile(pSI->szDeviceName,            // name of device
                    GENERIC_READ|GENERIC_WRITE,         // desired access
                    FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
                    NULL,                               // security attributes (ignored)
                    OPEN_EXISTING,                      // creation disposition
                    FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
                    NULL);                              // template file (ignored)
}

void ShowMessage()
{
    MessageBox(NULL, L"Updating, please wait ... system will be rebooted automatically after updating!", L"Message", MB_TOPMOST);
}

BOOL CheckAndDelSign()
{
    DWORD sign = 0;
    
    if(!KernelIoControl(IOCTL_HAL_QUERY_UPDATE_SIG, NULL, 0, &sign, sizeof(sign), NULL))
    {
        ERRORMSG(1, (TEXT("IOCTL_HAL_QUERY_UPDATE_SIG failed!\r\n")));
        return FALSE;
    }
    
    if(sign == UPDATE_SIGN)
    {
        sign = 0;
        
        if(!KernelIoControl(IOCTL_HAL_SET_UPDATE_SIG, NULL, 0, &sign, sizeof(sign), NULL))
        {
            ERRORMSG(1, (TEXT("IOCTL_HAL_SET_UPDATE_SIG failed!\r\n")));
            return FALSE;
        }
    
        return TRUE;
    }
    
    return FALSE;
}

void SDMMCBootProcess()
{
    HANDLE hDSK;
    SG_REQ sq, sq1;
    STOREINFO si;
    int count = 0;
    
retry:    
    // 1. find the correct device
    hDSK = FindAndOpenDSK(L"SDMEMORY", sizeof(L"SDMEMORY"), &si);
    
    if(!hDSK)
    {
        hDSK = FindAndOpenDSK(L"MMC", sizeof(L"MMC"), &si);
        if(!hDSK)
        {
            count++;
            if(count != 10)
            {
                Sleep(1000);   
                goto retry;
            }
            ERRORMSG(TRUE, (_T("failed to find and open store\r\n")));
            return;
        }
    }
    
    // 2. read MBR from device
    memset(&sq, 0, sizeof(SG_REQ));
    memset(&sq1, 0, sizeof(SG_REQ));
    
    sq.sr_start = 0;
    sq.sr_num_sec = 1;
    sq.sr_num_sg = 1;
    sq.sr_sglist[0].sb_buf = (PBYTE)LocalAlloc(LMEM_ZEROINIT, BUFFER_SIZE);
    sq.sr_sglist[0].sb_len = BUFFER_SIZE;
    
    sq1.sr_start = 0;
    sq1.sr_num_sec = BUFFER_SIZE / si.dwBytesPerSector;
    sq1.sr_num_sg = 1;
    sq1.sr_sglist[0].sb_buf = (PBYTE)LocalAlloc(LMEM_ZEROINIT, BUFFER_SIZE);
    sq1.sr_sglist[0].sb_len = BUFFER_SIZE;
    
    if(!sq.sr_sglist[0].sb_buf || !sq1.sr_sglist[0].sb_buf)
    {
        ERRORMSG(TRUE, (_T("alloc memory failed\r\n")));   
        goto exit;   
    }
    
    if (!DeviceIoControl(hDSK, IOCTL_DISK_READ, &sq, sizeof(SG_REQ), NULL, 0, NULL, NULL))
    {
        ERRORMSG(TRUE, (_T("IOCTL_DISK_READ failed\r\n")));   
        goto exit;
    }
    
    sq.sr_num_sec = BUFFER_SIZE / si.dwBytesPerSector;
    
    // 3. Parse MBR
    PMASTERBOOT_RECORD pMBR = (PMASTERBOOT_RECORD)sq.sr_sglist[0].sb_buf;
    DWORD dwBootOffset, dwBootSize, i;
    
    if(pMBR->Signature == 0xAA55)
    {
        for(i = 0; i < MAX_PARTTABLE_ENTRIES; i++)
        {
            if(pMBR->Partition[i].SystemId == 'S')       //soc spec   
            {
                //it's boot partition
                break;
            }
        }
        
        if(i == MAX_PARTTABLE_ENTRIES)
        {
            ERRORMSG(TRUE, (_T("Not MBR format, skipping\r\n")));
            goto exit;
        }
        else
        {
            //skip first 4 sectors as BCB
            //and sector addr must be 4 aligned
            dwBootOffset = RoundToNearestDWORD(pMBR->Partition[i].RelativeSector + 4);
            dwBootSize = (pMBR->Partition[i].TotalSector - (dwBootOffset - pMBR->Partition[i].RelativeSector)) & ~7;
        }
    }
    
    // 4. Read out image and compare
    DWORD len = dwBootSize / 2;
    DWORD dwBoot1Start, dwBoot2Start;
    
    dwBoot1Start = dwBootOffset;
    dwBoot2Start = dwBootOffset + len;
    
    RETAILMSG(1, (_T("dwBoot1Start=0x%x, dwBoot2Start=0x%x, len=0x%x\r\n"),dwBoot1Start, dwBoot2Start, len));
    
    for(i = 0; i < len;)
    {
        sq.sr_start = dwBoot1Start + i;
        sq1.sr_start = dwBoot2Start + i;
        
        if(len < sq.sr_num_sec + i)
        {
            sq.sr_num_sec = sq1.sr_num_sec = len - i;
        }
        
        if (!DeviceIoControl(hDSK, IOCTL_DISK_READ, &sq, sizeof(SG_REQ), NULL, 0, NULL, NULL))
        {
            ERRORMSG(TRUE, (_T("IOCTL_DISK_READ failed\r\n")));   
            goto exit;
        }
        if (!DeviceIoControl(hDSK, IOCTL_DISK_READ, &sq1, sizeof(SG_REQ), NULL, 0, NULL, NULL))
        {
            ERRORMSG(TRUE, (_T("IOCTL_DISK_READ failed\r\n")));   
            goto exit;
        }
        if(memcmp(sq.sr_sglist[0].sb_buf, sq1.sr_sglist[0].sb_buf, sq.sr_num_sec * si.dwBytesPerSector))
        {
            ERRORMSG(TRUE, (_T("image content comparing error: %d\r\n"),i));   
            break;
        }
        i += sq.sr_num_sec;
    }
    
    // 5. continue update?
    if(i == len)
    {
        if(g_bm.source == First && CheckAndDelSign())
        {
            RETAILMSG(1, (_T("update successfully\r\n")));
            MessageBox(NULL, L"update successfully!", L"Message", MB_TOPMOST | MB_OK);
        }
        goto exit;
    }
        
    int Ret;
    
    if(g_bm.source == First)
    {
        RETAILMSG(1, (_T("Do you want to recover boot stream two?\r\n")));  
        Ret = MessageBox(NULL, L"Do you want to recover boot stream two?", L"Message", MB_YESNO | MB_TOPMOST | MB_APPLMODAL | MB_ICONQUESTION);
    }
    else
    {
        RETAILMSG(1, (_T("Do you want to continue updating boot stream one?\r\n"))); 
        Ret = MessageBox(NULL, L"Do you want to continue updating boot stream one?", L"Message", MB_YESNO | MB_TOPMOST | MB_APPLMODAL | MB_ICONQUESTION);
    }
    if(Ret == IDNO)
    {
        RETAILMSG(1, (_T("quitting\r\n"))); 
        goto exit;   
    }
    RETAILMSG(1, (_T("update the %s boot stream!\r\n"), g_bm.source == First ? L"second" : L"first"));  
    
    //need to add some input function to let user decide whether to continue update.
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowMessage, NULL, 0, NULL);
    
    // 6. update
    if(g_bm.source == First)
    {
        sq.sr_start = dwBoot1Start;
        sq1.sr_start = dwBoot2Start;
    }
    else
    {
        sq.sr_start = dwBoot2Start;
        sq1.sr_start = dwBoot1Start;
    }
    
    sq.sr_num_sec = BUFFER_SIZE / si.dwBytesPerSector;
    sq1.sr_num_sec = BUFFER_SIZE / si.dwBytesPerSector;
    
    for(i = 0; i < len;)
    {
        DWORD temp;
        
        if(len < sq.sr_num_sec + i)
        {
            sq.sr_num_sec = sq1.sr_num_sec = len - i;
        }
        
        temp = sq.sr_start;
        if (!DeviceIoControl(hDSK, IOCTL_DISK_READ, &sq, sizeof(SG_REQ), NULL, 0, NULL, NULL))
        {
            ERRORMSG(TRUE, (_T("IOCTL_DISK_READ failed\r\n")));   
            goto exit;
        }
        sq.sr_start = sq1.sr_start;
        if (!DeviceIoControl(hDSK, IOCTL_DISK_WRITE, &sq, sizeof(SG_REQ), NULL, 0, NULL, NULL))
        {
            ERRORMSG(TRUE, (_T("IOCTL_DISK_WRITE failed\r\n")));   
            goto exit;
        }
        sq.sr_start = temp;
        
        i += sq.sr_num_sec;
        sq.sr_start += sq.sr_num_sec;;
        sq1.sr_start += sq1.sr_num_sec;;
    }
    RETAILMSG(1, (_T("done\r\n")));
    
    // 7. reboot system
    RebootSystem(g_bm.source == First ? FALSE : TRUE);
    
exit:
    if(hDSK)
        CloseHandle(hDSK);     
    if(sq.sr_sglist[0].sb_buf)
        LocalFree(sq.sr_sglist[0].sb_buf);       
    if(sq1.sr_sglist[0].sb_buf)
        LocalFree(sq1.sr_sglist[0].sb_buf);                  
}

void NANDBootProcess()
{
    HANDLE hDSK;
    NANDImgInfo ImgInfo = {};
    STOREINFO si;
    PBYTE FirstBootImg, SecondBootImg;
    FirstBootImg = SecondBootImg = NULL;
    int count = 0;
    
retry:
    hDSK = FindAndOpenDSK(L"MSFLASH", sizeof(L"MSFLASH"), &si);
    
    if(!hDSK)
    {
        count++;
        if(count != 10)
        {
            Sleep(1000);   
            goto retry;
        }
        ERRORMSG(TRUE, (_T("failed to find and open store\r\n")));
        return;
    }

    if (!DeviceIoControl(hDSK,      // file handle to the driver
        IOCTL_DISK_VENDOR_GET_SBIMGINFO,  // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        &ImgInfo,                   // out buffer
        sizeof(NANDImgInfo),        // out buffer size
        NULL,                       // pointer to number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        ERRORMSG(1, (TEXT("IOCTL_DISK_VENDOR_GET_IMGLENTH failed!\r\n")));
        goto exit;
    }
    
    // prepare to read out two boot streams
    FirstBootImg = (PBYTE)LocalAlloc(LMEM_ZEROINIT, ImgInfo.dwSBLenUnit);
    SecondBootImg = (PBYTE)LocalAlloc(LMEM_ZEROINIT, ImgInfo.dwSBLenUnit);
    
    if(!FirstBootImg || !SecondBootImg)
    {
        ERRORMSG(TRUE, (_T("alloc memory for use failed\r\n")));
        goto exit;   
    }
    
    DWORD index;
    
    for(index = 0; index < ImgInfo.dwSBLen / ImgInfo.dwSBLenUnit; index++)
    {
        NANDIORequest request;
        
        request.dwIndex = index;
        request.SBPos = FirstBoot;
        
        if (!DeviceIoControl(hDSK,      // file handle to the driver
            IOCTL_DISK_VENDOR_READ_SB,  // I/O control code
            &request,                   // in buffer
            sizeof(request),            // in buffer size
            FirstBootImg,               // out buffer
            ImgInfo.dwSBLenUnit,        // out buffer size
            NULL,                       // pointer to number of bytes returned
            NULL))                      // ignored (=NULL)
        {
            ERRORMSG(1, (TEXT("IOCTL_DISK_VENDOR_READ_SB failed!\r\n")));
            goto exit;
        }
        
        request.SBPos = SecondBoot;
        
        if (!DeviceIoControl(hDSK,      // file handle to the driver
            IOCTL_DISK_VENDOR_READ_SB,  // I/O control code
            &request,                   // in buffer
            sizeof(request),            // in buffer size
            SecondBootImg,              // out buffer
            ImgInfo.dwSBLenUnit,        // out buffer size
            NULL,                       // pointer to number of bytes returned
            NULL))                      // ignored (=NULL)
        {
            ERRORMSG(1, (TEXT("IOCTL_DISK_VENDOR_READ_SB failed!\r\n")));
            goto exit;
        }
        if(memcmp(FirstBootImg, SecondBootImg, ImgInfo.dwSBLenUnit))
        {
            SBPosition BootPositionSrc, BootPositionDest;
            int Ret;
            
            if(g_bm.source == First)
            {
                BootPositionSrc = FirstBoot;
                BootPositionDest = SecondBoot;
                RETAILMSG(1, (_T("Do you want to recover boot stream two?\r\n")));  
                Ret = MessageBox(NULL, L"Do you want to recover boot stream two?", L"Message", MB_YESNO | MB_TOPMOST | MB_APPLMODAL | MB_ICONQUESTION);
            }
            else
            {
                BootPositionSrc = SecondBoot;
                BootPositionDest = FirstBoot;
                RETAILMSG(1, (_T("Do you want to continue updating boot stream one?\r\n"))); 
                Ret = MessageBox(NULL, L"Do you want to continue updating boot stream one?", L"Message", MB_YESNO | MB_TOPMOST | MB_APPLMODAL | MB_ICONQUESTION);
            }
            if(Ret == IDNO)
            {
                RETAILMSG(1, (_T("quitting\r\n"))); 
                goto exit;   
            }
            RETAILMSG(1, (_T("update the %s boot stream!\r\n"), g_bm.source == First ? L"second" : L"first"));  
            
            //need to add some input function to let user decide whether to continue update.
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowMessage, NULL, 0, NULL);
            if (!DeviceIoControl(hDSK,      // file handle to the driver
                IOCTL_DISK_COPYBACK,        // I/O control code
                &BootPositionSrc,                       // in buffer
                sizeof(SBPosition),                          // in buffer size
                &BootPositionDest,              // out buffer
                sizeof(SBPosition),       // out buffer size
                NULL,                       // pointer to number of bytes returned
                NULL))                      // ignored (=NULL)
            {
                ERRORMSG(1, (TEXT("IOCTL_DISK_COPYBACK failed!\r\n")));
                goto exit;
            }
            
            goto reboot;
        }
    }
    if(g_bm.source == First && CheckAndDelSign())
    {
        RETAILMSG(1, (_T("update successfully\r\n")));
        MessageBox(NULL, L"update successfully!", L"Message", MB_TOPMOST | MB_OK);
        goto exit;
    }
    goto exit;
    
reboot:
    RebootSystem(g_bm.source == First ? FALSE : TRUE);
    
exit:    

    if(FirstBootImg)
    {
        LocalFree(FirstBootImg);   
    }
    if(SecondBootImg)
    {
        LocalFree(SecondBootImg);   
    }
    CloseHandle(hDSK);
}


//-----------------------------------------------------------------------------
//
// The main function
//-----------------------------------------------------------------------------
int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPTSTR pCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);
    UNREFERENCED_PARAMETER(pCmdLine);
    
    HANDLE mutex;
    
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
    mutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
    
    if(mutex && GetLastError() == ERROR_ALREADY_EXISTS)
    {
        RETAILMSG(TRUE, (_T("process has already been running\r\n")));
        MessageBox(NULL, L"process has already been running!", L"Message", MB_OK);
        goto exit;
    }
    
    if(!KernelIoControl(IOCTL_HAL_QUERY_BOOT_MODE, NULL, 0, &g_bm, sizeof(g_bm), NULL))
    {
        ERRORMSG(1, (TEXT("IOCTL_HAL_QUERY_BOOT_MODE failed!\r\n")));
        goto exit;
    }
    
    switch(g_bm.media)
    {
        case NandBoot:
            RETAILMSG(1, (_T("booted from NAND Flash\r\n")));
            break;
        case SDMMCBoot:
            RETAILMSG(1, (_T("booted from SDMMCBoot\r\n")));
            break;
        case SPIBoot:
            RETAILMSG(1, (_T("booted from SPI, skip\r\n")));
            goto exit;
        default:
            RETAILMSG(1, (_T("booted from UNKNOWN, skip\r\n")));
            goto exit;  
    }
    RETAILMSG(1, (_T("booted from %s\r\n"), g_bm.source == First? L"First Boot" : L"Second Boot"));

    if(g_bm.media == SDMMCBoot)
    {
        SDMMCBootProcess();
    }
    
    if(g_bm.media == NandBoot)
    {
        NANDBootProcess();
    }
exit:    
    if(mutex)
        CloseHandle(mutex);
        
    return 0;
}
    