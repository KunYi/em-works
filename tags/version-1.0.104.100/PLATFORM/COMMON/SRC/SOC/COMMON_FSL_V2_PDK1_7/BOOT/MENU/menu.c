//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  menu.c
//
//  Menu routines for the ethernet bootloader.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//Include Files
#pragma warning(push)
#pragma warning(disable: 4115 4201 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <winnls.h>
#pragma warning(pop)
#include "menu.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//Global Variables
BOOT_CFG g_BootCFG;
BOOLEAN bCFGChanged  = FALSE;
BLMenuPrintDataSoc PrintSoc = NULL;
//------------------------------------------------------------------------------
//Externel Variables
extern BOOL g_DownloadImage;
extern CHAR extCSDBuffer[];
extern DWORD eSDBootPartitionSize;
extern BOOL eSDBPEnabled;
extern UINT32 EMMCToggleBP(void);
extern UINT32 EMMCToggleBPSize(void);
extern UINT32  ESDToggleBP(void);
extern UINT32 ESDToggleBPSize(void);

//------------------------------------------------------------------------------
//Function
//------------------------------------------------------------------------------
//
//  Function:  BLMenuShow
//
//  Provides boot loader menu interface.
//
//  Parameters:
//      pMenu:BLMENU_ITEM Item.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL BLMenuShow(BLMENU_ITEM *pMenu)
{
    BLMENU_ITEM *aMenu = pMenu, *pItemMenu,*aSoc = pMenu;    
    WCHAR key;
    
    for(;;) 
    {
        OALLog(L"\r\n-----------------------------------------------------------------------------\r\n");
        OALLog(L"Freescale iMX SOC Menu Item   %s",(bCFGChanged ? L"(UNSAVED CHANGES)" : L""));
        OALLog(L"\r\n-----------------------------------------------------------------------------\r\n");

        // Print menu items
        for (pItemMenu = aMenu; pItemMenu->key != 0; pItemMenu++) 
        {
            if(pItemMenu->pParam2 == NULL)
                OALLog(L" [%c] %s \r\n", pItemMenu->key, pItemMenu->text);
            else 
            {
                OALLog(L" [%c] %s", pItemMenu->key, pItemMenu->text);
                BLMenuPrintData(pItemMenu->key,pItemMenu->pParam2,pItemMenu->pParam3);
            }
        }
        aSoc = pItemMenu+1;
        for (pItemMenu = aSoc; pItemMenu->key != 0; pItemMenu++) 
        {
            if(pItemMenu->pParam2 == NULL)
                OALLog(L" [%c] %s \r\n", pItemMenu->key, pItemMenu->text);
            else 
            {
                OALLog(L" [%c] %s", pItemMenu->key, pItemMenu->text);
                (*PrintSoc)(pItemMenu->key,pItemMenu->pParam2,pItemMenu->pParam3);
            }
        }

        OALLog(L"\r\n Selection: ");
        
        for(;;) 
        {
            // Get key
            key = (WCHAR)OEMReadDebugByte();
            // Look for key in menu
            for (pItemMenu = aMenu; pItemMenu->key != 0; pItemMenu++) 
            {
                if ((pItemMenu->key == key) || ((pItemMenu->key == key-32)&&(key>=97)&&(key<=122))) break;
            }
            // If we find it, break loop
            if (pItemMenu->key != 0) break;
            aSoc = pItemMenu+1;
            for (pItemMenu = aSoc; pItemMenu->key != 0; pItemMenu++) 
            {
                if ((pItemMenu->key == key) || ((pItemMenu->key == key-32)&&(key>=97)&&(key<=122))) break;
            }
            // If we find it, break loop
            if (pItemMenu->key != 0) break;
        }

        // Print out selection character
        OALLog(L"%c\r\n", key);

        // When action is NULL return back to parent menu
       
        if (pItemMenu->pfnAction == NULL) 
            break;
        else
            if(pItemMenu->pfnAction(pItemMenu) == TRUE) break;
                
        // Else call menu action

    } 
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  BLMenuEnableDisable
//
//  Provides Enabling or Disabling Parameter.
//
//  Parameters:
//      pMenu:BLMENU_ITEM Item.
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuEnableDisable(BLMENU_ITEM *pMenu)
{
    UINT32 *pFlags = pMenu->pParam2;
    UINT32 mask = (UINT32)pMenu->pParam3;
    BOOL flag;
    // First check input parameter   
    if (pFlags == NULL) 
    {
        OALMSG(OAL_ERROR, (L"ERROR: BLMenuEnable: Invalid parameter\r\n"));
        goto cleanUp;
    }
    if (mask == 0) mask = 0xFFFF;
    flag = (*pFlags & mask) != 0;
    flag = !flag;
    // Save value
    if (flag) 
    {
        *pFlags |= mask;
    } 
    else 
    {
        *pFlags &= ~mask;
    }
cleanUp:;
    bCFGChanged = TRUE;
    return FALSE;
    
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuSetMacAddress
//
//  Provides Setting the MAC Address.
//
//  Parameters:
//      pMenu:BLMENU_ITEM Item.
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuSetMacAddress(BLMENU_ITEM *pMenu)
{
    LPCWSTR title = pMenu->pParam1;
    UINT16 *pMac = pMenu->pParam2;
    UINT16 mac[3];
    WCHAR buffer[18];

    // First check input parameters    
    if (title == NULL || pMac == NULL) 
    {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALBLMenuSetMacAddress: Invalid parameter\r\n"
        ));
        goto cleanUp;
    }

    // Get actual setting
    mac[0] = pMac[0];
    mac[1] = pMac[1];
    mac[2] = pMac[2];

    // Print prompt
    OALLog(
        L" Enter %s (actual %s): ", title, OALKitlMACtoString(mac)
    );
    
    // Read input line
    if (BLMenuReadLine(buffer, dimof(buffer)) == 0) goto cleanUp;

    // Convert string to MAC address
    if (!OALKitlStringToMAC(buffer, mac)) 
    {
        OALLog(L" '%s' isn't valid MAC address\r\n", buffer);
        goto cleanUp;
    }

    // Print final MAC address
    OALLog(L" %s set to %s\r\n", title, OALKitlMACtoString(mac));        

    // Save new setting
    pMac[0] = mac[0];
    pMac[1] = mac[1];
    pMac[2] = mac[2];
    
cleanUp:;
    bCFGChanged = TRUE;
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuSetIpAddress
//
//  Provides Setting the IP Address.
//
//  Parameters:
//      pMenu:BLMENU_ITEM Item.
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuSetIpAddress(BLMENU_ITEM *pMenu)
{
    LPCWSTR title = pMenu->pParam1;
    UINT32 *pIp = pMenu->pParam2;
    UINT32 ip;
    WCHAR buffer[16];

    // First check input parameters    
    if (title == NULL || pIp == NULL) 
    {
        OALMSG(OAL_ERROR, (
            L"ERROR: BLMenuSetIpAddress: Invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    // Get actual value
    ip = *pIp;

    // Print prompt
    OALLog(L" Enter %s (actual %s): ", title, OALKitlIPtoString(ip));

    // Read input line
    if (BLMenuReadLine(buffer, dimof(buffer)) == 0) goto cleanUp;

    // Convert string to IP address
    ip = OALKitlStringToIP(buffer);
    if (ip == 0) 
    {
        OALLog(L" '%s' isn't valid IP address\r\n", buffer);
        goto cleanUp;
    }

    // Print final IP address
    OALLog(L" %s set to %s\r\n", title, OALKitlIPtoString(ip));

    // Save new setting
    *pIp = ip;
    
cleanUp:;
    bCFGChanged = TRUE;
    return FALSE;
    
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuSetIpMask
//
//  Provides Setting the IP Mask.
//
//  Parameters:
//      pMenu:BLMENU_ITEM Item.
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuSetIpMask(BLMENU_ITEM *pMenu)
{
    LPCWSTR title = pMenu->pParam1;
    UINT32 *pIp = pMenu->pParam2;
    UINT32 ip;
    WCHAR buffer[16];

    // First check input parameters    
    if (title == NULL || pIp == NULL) 
    {
        OALMSG(OAL_ERROR, (
            L"ERROR: BLMenuSetIpMask: Invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    // Get actual value
    ip = *pIp;

    // Print prompt
    OALLog(L" Enter %s (actual %s): ", title, OALKitlIPtoString(ip));

    // Read input line
    if (BLMenuReadLine(buffer, dimof(buffer)) == 0) goto cleanUp;

    // Convert string to MAC address
    ip = OALKitlStringToIP(buffer);
    if (ip == 0) 
    {
        OALLog(L" '%s' isn't valid IP mask\r\n", buffer);
        goto cleanUp;
    }

    // Print final IP mask
    OALLog(L" %s set to %s\r\n", title, OALKitlIPtoString(ip));

    // Save new setting
    *pIp = ip;
    
cleanUp:;
    bCFGChanged = TRUE;
    return FALSE;
    
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuReadLine
//
//  Provides Reading a Line of Data From UART.
//
//  Parameters:
//      szBuffer:
//      CharCount:
//  Returns:
//      
//
//------------------------------------------------------------------------------
UINT32 BLMenuReadLine(LPWSTR szBuffer, size_t CharCount)
{
    UINT32 count;
    WCHAR key;
    
    count = 0;
    while (count < CharCount) 
    {
        key = (WCHAR)OEMReadDebugByte();
        if (key == L'\r' || key == L'\n') 
        {
            OALLog(L"\r\n");
            break;
        } 
        if (key == L'\b' && count > 0) 
        {
            OALLog(L"\b \b");
            count--;
        } 
        else if (key >= L' ' && key < 128 && count < (CharCount - 1)) 
        {
            szBuffer[count++] = key;
            OALLog(L"%c", key);
        } 
    }
    szBuffer[count] = '\0';
    return count;
    
}


//------------------------------------------------------------------------------
//
//  Function:  BLMenuPrintData
//
//  Provides Printing the Menu Item Status.
//
//  Parameters:
//      key
//      dwValue
//      mask
//  Returns:
//      
//
//------------------------------------------------------------------------------
BOOL  BLMenuPrintData(WCHAR key,VOID * dwValue,VOID * mask)
{
    switch(key)
    {
        case L'0':
            KITLOutputDebugString ( " : %s\r\n",(UINT8 *)inet_ntoa(*((DWORD *)dwValue)));
            break;
        case L'1':
            KITLOutputDebugString ( " : %s\r\n",(UINT8 *)inet_ntoa(*((DWORD *)dwValue)));
            break;
        case L'2': 
            KITLOutputDebugString ( " : %d\r\n",*((DWORD *)dwValue));
            break;
        case L'3':
            KITLOutputDebugString ( " : %s\r\n", (*((DWORD *)dwValue) == FALSE ? "Disabled" : "Enabled"));
            break;
        case L'5':
            (*PrintSoc)(key,dwValue,mask);
            break;                
        case L'6':
            KITLOutputDebugString ( " : %x-%x-%x-%x-%x-%x\r\n", 
                *((WORD *)dwValue+0) & 0x00FF,  *((WORD *)dwValue+0) >> 8,
                *((WORD *)dwValue+1) & 0x00FF,  *((WORD *)dwValue+1) >> 8,
                *((WORD *)dwValue+2) & 0x00FF,  *((WORD *)dwValue+2) >> 8);
            break;  
        case L'E':
            (*PrintSoc)(key,dwValue,mask);
            break;              
        case L'I':
            KITLOutputDebugString ( " : %s\r\n", (*((DWORD *)dwValue) & (UINT32)mask? "Interrupt": "Polling"));
            break;  
        case L'K':
            KITLOutputDebugString ( " : %s\r\n", (*((DWORD *)dwValue) & (UINT32)mask? "Enable": "Disable"));
            break;              
        case L'P':
            KITLOutputDebugString ( " : %s\r\n", (*((DWORD *)dwValue) & (UINT32)mask? "Enable": "Disable"));
            break;  

        // eMMC 4.3 EXT_CSD byte 179 (BOOT_CONFIG)    
        case L'A':
            switch(*(PBYTE)dwValue)
            {
                default:
                case 0x0:
                KITLOutputDebugString (" : Disabled / Not Available\r\n");
                break;

                case 0x9:
                case 0x49:
                    KITLOutputDebugString (" : Boot and r/w part 1\r\n");
                    break;

                case 0x8:
                case 0x48:
                    KITLOutputDebugString (" : Boot part 1, r/w usr part\r\n");
                    break;

                case 0x52:
                case 0x12:
                    KITLOutputDebugString (" : Boot and r/w part 2\r\n");
                    break;

                case 0x50:
                case 0x10:
                    KITLOutputDebugString (" : Boot part 2, r/w usr part\r\n");
                    break;


            }
            break;            


        // eMMC 4.3 EXT_CSD bytes 226-227 (BOOT_SIZE_MULT)    
        case L'B':
            KITLOutputDebugString ( " : %d Kbytes\r\n",  (*((PWORD)dwValue)) << 7);
            break;

        // eSD2.1 boot partition enable/disable
        case L'S':

            if (*(PBOOL) dwValue)
            {
                KITLOutputDebugString (" : Enabled\r\n");
            }
            else
            {
                KITLOutputDebugString (" : Disabled / Not Available\r\n");
            }

            break;
            
        // eSD2.1 boot partition size (# of blocks * 512 bytes (<< 9) / 1024 bytes (>> 10) = Kbytes (>> 1))
        case L'D':
            KITLOutputDebugString ( " : %d Kbytes\r\n",  ((*(PDWORD)dwValue) >> 1));
            break;
            
        default:            
            KITLOutputDebugString ( "\r\n");            
            break;
    }
    return TRUE;
    
}


//------------------------------------------------------------------------------
//
//  Function:  BootShellMenuItem
//
//  Provides boot shell function.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuBootShell(BLMENU_ITEM *pMenu)
{
    UNREFERENCED_PARAMETER(pMenu);
    BspBootShell();
    return FALSE;
}


//------------------------------------------------------------------------------
//
//  Function:  BLMenuSaveSettings
//
//  Provides saving the settings.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuSaveSettings(BLMENU_ITEM *pMenu)
{
    StoreBootCFG(pMenu->pParam3); 
    ConfigBootCFG(pMenu->pParam3);
    bCFGChanged = FALSE;
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuLaunchFlashImage
//
//  Provides launching the flash image.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuLaunchFlashImage(BLMENU_ITEM *pMenu)
{
    UNREFERENCED_PARAMETER(pMenu);
    g_DownloadImage = FALSE;
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuBootDelay
//
//  Provides setting the boot delay time(secs).
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuBootDelay(BLMENU_ITEM *pMenu)
{
    SetDelay(pMenu->pParam3);
    bCFGChanged=TRUE;
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuResetConfiguration
//
//  Provides reset the configuration.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuResetConfiguration(BLMENU_ITEM *pMenu)
{
    ResetDefaultBootCFG(pMenu->pParam3);
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuDownloadImageNow
//
//  Provides Downloading the Image.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuDownloadImageNow(BLMENU_ITEM *pMenu)
{
    UNREFERENCED_PARAMETER(pMenu);
    g_DownloadImage = TRUE;
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuFormatNandOS
//
//  Provides Formating the OS.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuFormatNandOS(BLMENU_ITEM *pMenu)
{
    UINT32 Selection;
    UNREFERENCED_PARAMETER(pMenu);

    KITLOutputDebugString("\r\nWARNING:  Format of OS NAND region requested.\r\n");
    KITLOutputDebugString("Do you want to continue (y/n)? ");
    do {
        Selection = tolower(OEMReadDebugByte());
    } while ((Selection != 'y') && (Selection != 'n'));
    KITLOutputDebugString("\r\n");
    
    if (Selection == 'y') 
    {
        NANDFormatNK();
    }
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuFormatAllNand
//
//  Provides Formating the All Nand.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuFormatAllNand(BLMENU_ITEM *pMenu)
{
    UINT32 Selection;
    UNREFERENCED_PARAMETER(pMenu);

    KITLOutputDebugString("\r\nWARNING:  Format of all NAND regions requested.\r\n");
    KITLOutputDebugString("Boot loader and boot configuration regions will be erased!!!\r\n");
    KITLOutputDebugString("Do you want to continue (y/n)? ");
    do {
        Selection = tolower(OEMReadDebugByte());
    } while ((Selection != 'y') && (Selection != 'n'));
    KITLOutputDebugString("\r\n");
    
    if (Selection == 'y') 
    {
        NANDFormatAll();
    }
    return FALSE;
    
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuMMCSDUtilities
//
//  Provides utilities for SD/MMC cards.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuMMCSDUtilities(BLMENU_ITEM *pMenu)
{
    static BLMENU_ITEM mmcsdmenu[] = MMCSDSubMenu;
    UNREFERENCED_PARAMETER(pMenu);
    
    BLMenuShow(mmcsdmenu);

    return FALSE;

}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuEMMCToggleBP
//
//  Enable/disable booting from boot partition for eMMC 4.3 device
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuEMMCToggleBP(BLMENU_ITEM *pMenu)
{

    UNREFERENCED_PARAMETER(pMenu);

    if (EMMCToggleBP())
        KITLOutputDebugString("ERROR: Unable to read EXT_CSD from MMC card\r\n");

    return FALSE;

}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuEMMCBPSize
//
//  Allow toggling between Boot Partition size of 0 and 64 MBs
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuEMMCBPSize(BLMENU_ITEM *pMenu)
{
    UINT32 Selection;
    UNREFERENCED_PARAMETER(pMenu);

    KITLOutputDebugString("\r\nWARNING:  Change in boot partition size is requested\r\n");
    KITLOutputDebugString("Data on all partitions on the card will be erased!!!\r\n");
    KITLOutputDebugString("Do you want to continue (y/n)? ");
    do {
        Selection = tolower(OEMReadDebugByte());
    } while ((Selection != 'y') && (Selection != 'n'));
    KITLOutputDebugString("\r\n");
    
    if (Selection == 'y') 
    {
        if (EMMCToggleBPSize())
            KITLOutputDebugString("ERROR: Unable to read EXT_CSD from MMC card\r\n");
    } 

    return FALSE;

}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuESDBPEnable
//
//  Toggle between boot partition and user partition
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuESDBPEnable(BLMENU_ITEM *pMenu)
{
    UNREFERENCED_PARAMETER(pMenu);

    ESDToggleBP();

    return FALSE;

}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuESDBPSize
//
//  Allow toggling between Boot Partition size of 0 and 64 MBs
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuESDBPSize(BLMENU_ITEM *pMenu)
{
    UINT32 Selection;
    UNREFERENCED_PARAMETER(pMenu);

    KITLOutputDebugString("\r\nWARNING:  Change in boot partition size is requested\r\n");
    KITLOutputDebugString("Data on all partitions on the card will be erased!!!\r\n");
    KITLOutputDebugString("Do you want to continue (y/n)? ");
    do {
        Selection = tolower(OEMReadDebugByte());
    } while ((Selection != 'y') && (Selection != 'n'));
    KITLOutputDebugString("\r\n");
    
    if (Selection == 'y') 
    {
        if (ESDToggleBPSize())
            KITLOutputDebugString("ERROR: Unable to set partition attributes\r\n");
    } 

    return FALSE;

}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuFormatAllSDMMC
//
//  Provides Formating of SD/MMC card.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuFormatAllSDMMC(BLMENU_ITEM *pMenu)
{
    UINT32 Selection;
    UNREFERENCED_PARAMETER(pMenu);

    KITLOutputDebugString("\r\nWARNING:  Format of all SD/MMC regions requested.\r\n");
    KITLOutputDebugString("Boot loader, OS and boot configuration regions will be erased!!!\r\n");
    KITLOutputDebugString("Do you want to continue (y/n)? ");
    do {
        Selection = tolower(OEMReadDebugByte());
    } while ((Selection != 'y') && (Selection != 'n'));
    KITLOutputDebugString("\r\n");
    
    if (Selection == 'y') 
    {
        SDHCFormatAll();
    }
    return FALSE;

}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuCreateFileSystemOnSDMMC
//
//  Provides Creation of MBR with 2 partitions on card.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuCreateFileSystemOnSDMMC(BLMENU_ITEM *pMenu)
{
    UINT32 Selection;
    UNREFERENCED_PARAMETER(pMenu);

    KITLOutputDebugString("\r\nWARNING:  SD/MMC File System partition requested on boot card.\r\n");
    KITLOutputDebugString("All previous file system data on the card may be lost.\r\n\r\n");
    KITLOutputDebugString("Do you want to continue (y/n)? ");
    do {
        Selection = tolower(OEMReadDebugByte());
    } while ((Selection != 'y') && (Selection != 'n'));
    KITLOutputDebugString("\r\n");
    
    if (Selection == 'y') 
    {
        SDHCCreateFileSystemPartition();
        KITLOutputDebugString("\r\nIf partition creation was successful, please plug the card into a Windows PC to format the file system partition.\r\n\r\n");
        KITLOutputDebugString("Reboot the device manually...\r\n");
        SpinForever();
    }
    
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuReturn
//
//  Provides return to main menu from submenu
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL BLMenuReturn(BLMENU_ITEM *pMenu)
{
    UNREFERENCED_PARAMETER(pMenu);

    return TRUE;
}

