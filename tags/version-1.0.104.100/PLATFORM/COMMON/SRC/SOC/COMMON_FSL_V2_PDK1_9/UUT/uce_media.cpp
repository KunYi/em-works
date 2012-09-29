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
#include "menu.h"
#include "sdboot.h"

#define    MAX_BUF_ALLOC_SIZE      8*1024*1024  //the value should be devided by block size of nand flash

DWORD     g_MediaType = MEDIA_NAND;
HANDLE    ghStore=NULL;
DISK_INFO gdiDiskInfo;
NANDWrtImgInfo g_NANDWrtImgInfo;
fwType   g_fwType;

extern DWORD   g_MinBufSize;

DWORD FindNextDevice (
    DWORD i
    ) {

    DWORD dwRet;
    HANDLE hDsk;
    TCHAR szTemp[16];
    DISK_INFO diskInfo;
    DWORD cb;
    
    RETAILMSG(1, (_T("Poll the whole root dir to find all disks, i = 0x%x.\r\n"),i));
    // Validate input parameters
    if (i > 9) {
        // Input parameter is out of range
        return NULL;
    }
    RETAILMSG(1, (_T("Poll the whole root dir to find all disks, i = 0x%x.\r\n"),i));

    // Find the first available DSKx: device
    do {

        // Create the device name to try to open
        _stprintf( szTemp, TEXT("DSK%u:"), i );
        
        // 'i' is the number of the last found device. Start the search at 'i+1'
        i++;

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
        PREFAST_SUPPRESS(6001,"This Warning can be skipped!")            
            || 0 == diskInfo.di_total_sectors )
        {
            CloseHandle(hDsk);
            hDsk = INVALID_HANDLE_VALUE;
            continue;
        }
        
        RETAILMSG(TRUE, (TEXT("Find valid disk handle: %s:\r\n"), szTemp)); 
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

DWORD GetMinImgBufSize()
{
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
        //return MAX_BUF_ALLOC_SIZE/4;
    }
    else//SDMMC
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

BOOL WriteImage(LPBYTE pImage, DWORD dwLength)
{   
    if(g_MediaType == MEDIA_SDMMC)
    {
        switch(g_fwType)
        {
            case fwType_EB_NB:
            case fwType_EB_SB:
            case fwType_NK_SB:
                return SDWriteMBRModeEboot(pImage, dwLength);

            case fwType_NK_NB:
                return SDWriteMBRModeNK(pImage, dwLength);
        }
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
        return SDPrePartialWriteMBRModeImage(g_fwType, dwLength);
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
        switch(g_fwType)
        {
            case fwType_NK_SB:
                return SDPartialWriteRedundantImage(pImage, dwLength);

            default:
                return SDPartialWriteImage(pImage, dwLength);
        }
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
        RETAILMSG(TRUE, (L"GetFileAttributes Error: [file: %s, errno=%d]\r\n", ptchFileName, GetLastError()));
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
        RETAILMSG(TRUE, (L"CreateFile Error: [file: %s, errno=%d]\r\n", ptchFileName, GetLastError()));
        return 0;;
    }

    dwFileSize = GetFileSize(hFile, NULL);
    if(dwFileSize == 0xFFFFFFFF)
    {
        CloseHandle(hFile);
        RETAILMSG(TRUE, (L"GetFileSize Error: [file: %s, errno=%d]\r\n", ptchFileName, GetLastError()));
        return 0;;
    }

    while(dwTotalRead < dwFileSize)
    {
        if(!ReadFile(hFile, (LPVOID)pBuff, dwBuffSize, &dwReadSize, NULL))
        {
            RETAILMSG(TRUE, (L"ReadFile Error: [errno=%d]\r\n", GetLastError()));
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
        RETAILMSG(TRUE, (L"OpenFile Error: There is no %s file.\r\n", ptchFileName));
        return FALSE;
    }

    RETAILMSG(TRUE, (L"CreateFile %s file.\r\n", ptchFileName));
    // Create the specific file to write
    hFile = CreateFile(ptchFileName, GENERIC_READ|GENERIC_WRITE, 
                        FILE_SHARE_READ, NULL, CREATE_ALWAYS, 
                        FILE_FLAG_RANDOM_ACCESS, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {       
        RETAILMSG(TRUE, (L"CreateFile Error: [file: %s, errno=%d].\r\n", ptchFileName, GetLastError()));
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
            RETAILMSG(TRUE, (L"WriteFile Error: [errno=%d]\r", GetLastError()));
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

BOOL UcePreWriteRawData(TCHAR * pDiskName, DWORD startAddr, DWORD dwValidDataLength)
{   
    //RETAILMSG(1, (L"UcePreWriteRawData: Verify parameter.\r\n"));

    if((pDiskName == NULL))
    {
        return FALSE;
    }

    if(g_MediaType == MEDIA_SDMMC)
    {
		RETAILMSG(1, (L"UcePreWriteRawData: SDMMC Verify parameter.\r\n"));
        return SDPreSet(startAddr, dwValidDataLength);
    }
	else	// CS&ZHL JAN-9-2012: default Type is MEDIA_NAND
	{
        g_NANDWrtImgInfo.dwIndex = 0; 
		// choose IMAGE_EBOOTCFG or IMAGE_SPLASH according to startAddr & dwValidDataLength
		// size of EbootCFG < SectorSize(=2048 bytes)
		
		//if(dwValidDataLength < 2048)
		//{
		//	g_NANDWrtImgInfo.dwImgType = IMAGE_EBOOTCFG; 
		//	RETAILMSG(1, (L"UcePreWriteRawData::IMAGE_EBOOTCFG startaddr = 0x%x, length = 0x%x.\r\n", startAddr, dwValidDataLength));
		//}
		//else
		//{
		//	g_NANDWrtImgInfo.dwImgType = IMAGE_SPLASH; 
		//	RETAILMSG(1, (L"UcePreWriteRawData::IMAGE_SPLASH startaddr = 0x%x, length = 0x%x.\r\n", startAddr, dwValidDataLength));
		//}

		//
		// CS&ZHL MAR-30-2012: choose IMAGE_EBOOTCFG or IMAGE_SPLASH or IMAGE_MBR according to startAddr
		//
		RETAILMSG(1, (L"UcePreWriteRawData::startaddr = 0x%x, length = 0x%x.\r\n", startAddr, dwValidDataLength));
		if( startAddr == IMAGE_EBOOTCFG )
		{
			g_NANDWrtImgInfo.dwImgType = IMAGE_EBOOTCFG; 
			RETAILMSG(1, (L"UcePreWriteRawData::IMAGE_EBOOTCFG startaddr = 0x%x, length = 0x%x.\r\n", startAddr, dwValidDataLength));
		}
		else if( startAddr == IMAGE_SPLASH )
		{
			g_NANDWrtImgInfo.dwImgType = IMAGE_SPLASH; 
			RETAILMSG(1, (L"UcePreWriteRawData::IMAGE_SPLASH startaddr = 0x%x, length = 0x%x.\r\n", startAddr, dwValidDataLength));
		}
		else if( startAddr == IMAGE_MBR )
		{
			g_NANDWrtImgInfo.dwImgType = IMAGE_MBR; 
			RETAILMSG(1, (L"UcePreWriteRawData::IMAGE_MBR startaddr = 0x%x, length = 0x%x.\r\n", startAddr, dwValidDataLength));
		}
		else if( startAddr == IMAGE_UID )
		{
			g_NANDWrtImgInfo.dwImgType = IMAGE_UID; 
			RETAILMSG(1, (L"UcePreWriteRawData::IMAGE_UID startaddr = 0x%x, length = 0x%x.\r\n", startAddr, dwValidDataLength));
		}
	}
        
    return TRUE;
}

//
// CS&ZHL JAN-9-2012: Here the dwLength is valid image length, which may be less than NAND image unit -> BlockSize
//
BOOL NANDWriteRawData(LPBYTE pImage, DWORD dwLength)
{
    BOOL ret;

    //UNREFERENCED_PARAMETER(dwLength);
    ret = DeviceIoControl(
        ghStore,
        IOCTL_DISK_VENDOR_WRITE_IMAGE,
        &g_NANDWrtImgInfo,
        sizeof(NANDWrtImgInfo),
        pImage,
        dwLength,					//g_NANDWrtImgInfo.dwImgSizeUnit,
        NULL,
        NULL);        
    
    g_NANDWrtImgInfo.dwIndex++;

    return ret;
}

//
// CS&ZHL AUG-13-2012: code for EM9280 uce to format NandFlash
//
BOOL UceNandLowLevelFormat( )
{
    BOOL ret;

    ret = DeviceIoControl(
        ghStore,
        IOCTL_DISK_FORMAT,
        NULL,
        0,
        NULL,
        0,					
        NULL,
        NULL);        
    
    g_NANDWrtImgInfo.dwIndex++;

    return ret;
}

BOOL UceWriteRawData(PBYTE pbData, DWORD dwValidDataLength)
{      
	if(g_MediaType == MEDIA_SDMMC)
    {
        return SDWriteRawData(pbData, dwValidDataLength);
    }
	// CS&ZHL JAN-9-2012: supporting writeing raw data into NAND, -> EbootCFG and SplashScreen
	else if(g_MediaType == MEDIA_NAND)
	{
		//RETAILMSG(1, (L"UceWriteRawData::write raw data 0x%x bytes to NandFlash\r\n", dwValidDataLength));
		//// CS&ZHL MAY09-2012: EbootCFG -> chang mac with being readed from OTP
		//if( g_NANDWrtImgInfo.dwImgType == IMAGE_EBOOTCFG )
		//{
		//	PBOOT_CFG pCfg;
		//	
		//	// modify mac
		//	pCfg = (PBOOT_CFG)pbData;
		//	// read mac from OTP
		//	// rewrite new mac to BOOT_CFG;
		//}
        return NANDWriteRawData(pbData, dwValidDataLength);
	}

    return TRUE;
}

//
// CS&ZHL MAY11-2012: write security info to NandFlash & OTP
//
BOOL UceWriteSecurityInfo( PBYTE pbData, DWORD dwValidDataLength )
{
	if( g_MediaType == MEDIA_NAND )
	{
		g_NANDWrtImgInfo.dwImgType = IMAGE_VID;
		g_NANDWrtImgInfo.dwIndex = 0;
        return NANDWriteRawData(pbData, dwValidDataLength);
	}
	return TRUE;
}


//
// CS&ZHL JUN1-2012: Get OTP Info
//
BOOL UceGetOTPInfo(DWORD* pdwCUST, DWORD dwLen)
{
    BOOL ret;

    //UNREFERENCED_PARAMETER(dwLength);
    ret = DeviceIoControl(
        ghStore,
        IOCTL_DISK_GET_OTP_MAC,
        NULL,
        0,
        pdwCUST,
        dwLen,					
		NULL,
        NULL);        

    return ret;
}


BOOL UceCreatePartitions(INT iPartition, PARTITION_CONT partCont)
{
    if(g_MediaType == MEDIA_SDMMC)
    {
        if(iPartition == 2)//for MX
        return SDCreatePartitions(ghStore,iPartition,partCont);  
        else if(iPartition == 3)//for ST
        return SDWriteMBR(ghStore,iPartition,partCont);  
    }
    return TRUE;
}

BOOL GetFriendlyName(LPCTSTR pszStorageProfile, LPTSTR pszFriendlyName, UINT cFriendlyNameLen)
{
    TCHAR szRegPath[MAX_PATH];
    BOOL fRet = FALSE;
    HKEY hKey = NULL;
    DWORD dwRetVal, dwKeyType, dwLen;
    
    ASSERT(pszStorageProfile);
    if(pszStorageProfile)
    {
        StringCbPrintf(szRegPath, sizeof(szRegPath), STORAGE_PROFILE_FMT, pszStorageProfile);
        dwRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_ALL_ACCESS, &hKey);

        if(ERROR_SUCCESS == dwRetVal && NULL != hKey)
        {
            dwLen = cFriendlyNameLen * sizeof(*pszFriendlyName);
            dwRetVal = RegQueryValueEx(hKey, STORAGE_PROFILE_NAME, NULL, &dwKeyType,
                (PBYTE)pszFriendlyName, &dwLen);

            if(ERROR_SUCCESS == dwRetVal)
            {
                fRet = TRUE;
            }
            dwRetVal = RegCloseKey(hKey);
            ASSERT(ERROR_SUCCESS == dwRetVal);
        }
    }
    return fRet;
}

BOOL GetStoreList()
{
    STOREINFO si = {0};
    HANDLE hFind = INVALID_HANDLE_VALUE;
    si.cbSize = sizeof(STOREINFO);
    TCHAR* szFriendlyName=NULL;
    TCHAR* szDisplayName=NULL;

    szFriendlyName = (TCHAR*) LocalAlloc(LPTR, MAX_PATH);
    if(szFriendlyName == NULL)
        goto Exit;
    szDisplayName = (TCHAR*) LocalAlloc(LPTR, MAX_PATH);
    if(szDisplayName == NULL)
        goto Exit;
    
     // enumerate first store
    hFind = FindFirstStore(&si);
    
    if(INVALID_HANDLE_VALUE != hFind)
    {
        do
        {
            // add device name of store to the list
            if(GetFriendlyName(si.sdi.szProfile, szFriendlyName, MAX_PATH))
            {
                _stprintf(szDisplayName, _T("%s %s"), si.szDeviceName, szFriendlyName);
            }  
            else
            {
                _tcscpy(szDisplayName, si.szDeviceName);
            }
            RETAILMSG(1,(_T("GetStoreList:szDisplayName is %s\r\n"),szDisplayName));
        }
        while(FindNextStore(hFind, &si));
        FindClose(hFind);
    }
    else
    {
        RETAILMSG(1,(_T("INFO:There is no store Device find!\r\n")));
        goto Exit;
    }
    
    LocalFree(szFriendlyName);
    LocalFree(szDisplayName);
    return TRUE;

Exit:
    if(szFriendlyName != NULL)
        LocalFree(szFriendlyName);
    if(szDisplayName != NULL)
        LocalFree(szDisplayName);
    return FALSE;
}

BOOL GetDeviceHandle(TCHAR *pDiskName)
{  
    ghStore = OpenStore(pDiskName);
    if(ghStore == INVALID_HANDLE_VALUE)
    {
        RETAILMSG(1, (_T("Opening disk %s failed.Error code is %d\r\n"), pDiskName, GetLastError()));
        CloseHandle(ghStore);
        ghStore = NULL;
        return FALSE;
    }

    GetStoreList();

    if(g_MediaType == MEDIA_SDMMC)
    {
        SDGetHandle(ghStore);  
    }
    
    return TRUE;
}
