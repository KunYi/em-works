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
#pragma warning(disable:  6287 6262 4201 4512 4100 4115 4214 6001)
#include <windows.h>
#pragma warning(pop)
#include <Storemgr.h>
#include "stdio.h"
#include <diskio.h>
#include "common_nandfmd.h"
#include "common_otp.h"
#include "uce_media.h"
#include "sdboot.h"

#define    MAX_BUF_ALLOC_SIZE      8*1024*1024  //the value should be devided by block size of nand flash

DWORD     g_MediaType = MEDIA_NAND;
HANDLE    ghStore;
DISK_INFO gdiDiskInfo;
NANDWrtImgInfo g_NANDWrtImgInfo;


extern DWORD   g_MinBufSize;

DWORD FindNextDevice (
    DWORD i
    ) {

    DWORD dwRet;
    HANDLE hDsk;
    TCHAR szTemp[16];
    DISK_INFO diskInfo;
    DWORD cb;
    
    // Validate input parameters
    if (i > 9) {
        // Input parameter is out of range
        return NULL;
    }

    // Find the first available DSKx: device
    do {

        // 'i' is the number of the last found device. Start the search at 'i+1'
        i++;
        
        // Create the device name to try to open
        _stprintf( szTemp, TEXT("DSK%u:"), i );

        // Attempt to open the device
        hDsk = CreateFile( szTemp,
            GENERIC_READ|GENERIC_WRITE,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            0 );

        if(INVALID_HANDLE_VALUE == hDsk)
        {
            continue;
        }

        // this control code will tell us if the DSK is R/W media or a CD
        if(!DeviceIoControl(hDsk, DISK_IOCTL_GETINFO, &diskInfo, sizeof(diskInfo), NULL, 0, &cb, NULL)
            || 0 == diskInfo.di_total_sectors )
        {
            CloseHandle(hDsk);
            hDsk = INVALID_HANDLE_VALUE;
            continue;
        }
        
        RETAILMSG(TRUE, (TEXT("Find valid disk handle: DSK%u:\r\n"), i)); 
    } while (i <= 9 );

    // Was the search successful?
    if ( hDsk == INVALID_HANDLE_VALUE ) {
        // Did not find a device
        dwRet = NULL;
    }
    else {
        // Found a device. Close this instance, return the number of the deivce
        CloseHandle( hDsk );
        dwRet = i;
    }

    return dwRet;
}

DWORD GetMinImgBufSize(TCHAR *pDiskName)
{
    ghStore = CreateFile( pDiskName,
        GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        0 );
    if(g_MediaType == MEDIA_NAND)
    {    
        DeviceIoControl(
            ghStore,
            IOCTL_DISK_VENDOR_GET_IMGINFO,
            NULL,
            NULL,
            &g_NANDWrtImgInfo,
            sizeof(NANDWrtImgInfo),
            NULL,
            NULL);
        return g_NANDWrtImgInfo.dwImgSizeUnit; 
    }
    else
    {
        return MAX_BUF_ALLOC_SIZE;
    }     
}

//Here the dwLength is valid image length, which may be less than NAND image unit
BOOL NANDPartialWriteImage(LPBYTE pImage, DWORD dwLength)
{
    BOOL ret;

    UNREFERENCED_PARAMETER(dwLength);
    ret = DeviceIoControl(
        ghStore,
        IOCTL_DISK_VENDOR_WRITE_IMAGE,
        &g_NANDWrtImgInfo,
        sizeof(NANDWrtImgInfo),
        pImage,
        g_NANDWrtImgInfo.dwImgSizeUnit,
        NULL,
        NULL);        
    
    g_NANDWrtImgInfo.dwIndex++;

    return ret;
}

BOOL NANDEndPartialWriteImage()
{
    return DeviceIoControl(
        ghStore,
        IOCTL_DISK_VENDOR_END_WRITE_IMAGE,
        &g_NANDWrtImgInfo,
        sizeof(NANDWrtImgInfo),
        NULL,
        NULL,
        NULL,
        NULL);          
}

BOOL WriteImage(TCHAR *pDiskName, LPBYTE pImage, DWORD dwLength)
{   
    ghStore = CreateFile( pDiskName,
        GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        0 );
    if(ghStore == INVALID_HANDLE_VALUE)
    {
        RETAILMSG(1, (_T("Opening disk %s failed.\r\n"), pDiskName));
        return FALSE;
    }
    if(g_MediaType == MEDIA_SDMMC)
    {
        return SDWriteImage(ghStore, pImage, dwLength);
    } 
    return TRUE;
}

BOOL PrePartialWriteImage(TCHAR *pDiskName, DWORD dwLength)
{
    ghStore = CreateFile( pDiskName,
        GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        0 );
    if(ghStore == INVALID_HANDLE_VALUE)
    {
        RETAILMSG(1, (_T("Opening disk %s failed.\r\n"), pDiskName));
        return FALSE;
    }
    if(g_MediaType == MEDIA_SDMMC)
    {
        return SDPrePartialWriteImage(ghStore, dwLength);
    }
    else
    {
        g_NANDWrtImgInfo.dwIndex = 0; 
    }
    return TRUE;
}

BOOL PartialWriteImage(LPBYTE pImage, DWORD dwLength)
{   
    if(g_MediaType == MEDIA_NAND)
    {
        return NANDPartialWriteImage(pImage, dwLength);
    }
    else
    {
        return SDPartialWriteImage(pImage, dwLength);
    }
}

BOOL EndPartialWriteImage()
{   
    if(g_MediaType == MEDIA_NAND)
    {
        return NANDEndPartialWriteImage();
    }
    else
    {
        return SDEndPartialWriteImage();
    }
}

DWORD UceMediaRead(TCHAR* ptchFileName, PBYTE pBuff, DWORD dwBuffSize)
{
    DWORD dwFileAttr=0, dwFileSize=0, dwReadSize=0, dwTotalRead=0;
    HANDLE    hFile;
    
    if((ptchFileName == NULL)||(dwBuffSize < 0))
    {
        return 0;;
    }

    dwFileAttr = GetFileAttributes(ptchFileName);
    if(dwFileAttr == 0xFFFFFFFF)
    {
        RETAILMSG(TRUE, (L"GetFileAttributes Error: [file: %s, errno=%d]", ptchFileName, GetLastError()));
        return 0;;
    }
    else if(dwFileAttr == FILE_ATTRIBUTE_DIRECTORY)
    {
        RETAILMSG(TRUE, (L"%s is a directory!",ptchFileName));
        return 0;;
    }

    // Open the specific file to read
    hFile = CreateFile(ptchFileName, GENERIC_READ|GENERIC_WRITE, 
                        FILE_SHARE_READ, NULL, OPEN_EXISTING, 
                        FILE_FLAG_NO_BUFFERING, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        RETAILMSG(TRUE, (L"CreateFile Error: [file: %s, errno=%d]", ptchFileName, GetLastError()));
        return 0;;
    }

    dwFileSize = GetFileSize(hFile, NULL);
    if(dwFileSize == 0xFFFFFFFF)
    {
        CloseHandle(hFile);
        RETAILMSG(TRUE, (L"GetFileSize Error: [file: %s, errno=%d]", ptchFileName, GetLastError()));
        return 0;;
    }

    while(dwTotalRead < dwFileSize)
    {
        if(!ReadFile(hFile, (LPVOID)pBuff, dwBuffSize, &dwReadSize, NULL))
        {
            RETAILMSG(TRUE, (L"ReadFile Error: [errno=%d]", GetLastError()));
            CloseHandle(hFile);
            return 0;;
        }

        dwTotalRead += dwReadSize;
        //dwReadSize = 0;
    }

    CloseHandle(hFile);
    for(DWORD i=0;i<20;i++)
        RETAILMSG(TRUE, (L"%c", *(pBuff++)));
    RETAILMSG(1, (L"\r\n"));    
    return dwReadSize;
}

BOOL UceOpenFile(TCHAR* ptchFileName, HANDLE &hFile)
{   
    if((ptchFileName == NULL))
    {
        return FALSE;
    }
    
    // Create the specific file to write
    hFile = CreateFile(ptchFileName, GENERIC_READ|GENERIC_WRITE, 
                        FILE_SHARE_READ, NULL, CREATE_ALWAYS, 
                        FILE_FLAG_RANDOM_ACCESS, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {       
        RETAILMSG(TRUE, (L"CreateFile Error: [file: %s, errno=%d]", ptchFileName, GetLastError()));
        return FALSE;
    }

    return TRUE;
}

BOOL UceWriteFile(HANDLE hFile, PBYTE pBuff, DWORD dwBuffSize)
{
    DWORD dwWriteSize=0, dwTotalWriten=0, dwSizeToWrite=0;
    DWORD dwTotalSize = dwBuffSize;
    
    while(dwTotalWriten < dwTotalSize)
    {
        if((dwTotalSize-dwTotalWriten)<dwBuffSize)
        {
            dwSizeToWrite = dwTotalSize-dwTotalWriten;
        }
        else
        {
            dwSizeToWrite = dwBuffSize;
        }

        if(!WriteFile(hFile, (LPVOID)pBuff, dwSizeToWrite, &dwWriteSize, NULL))
        {
            RETAILMSG(TRUE, (L"WriteFile Error: [errno=%d]", GetLastError()));
            CloseHandle(hFile);
            return FALSE;
        }
        else
        {
            //RETAILMSG(TRUE, (L"WriteFile data length: [%d]\r\n", dwWriteSize));
        }
        
        dwTotalWriten += dwWriteSize;
        dwWriteSize = 0;
    }
    
    return TRUE;
}

BOOL UceCloseFile(HANDLE hFile)
{   
    CloseHandle(hFile);
    
    
    RETAILMSG(1, (L"Close file.\r\n"));
    //RETAILMSG(1, (L"Verify data...\r\n"));
    //UceMediaRead(ptchFileName, pBuff, dwBuffSize);

    return TRUE;
}
