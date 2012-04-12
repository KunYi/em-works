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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  menu.c
//
//  Menu routines for the ethernet bootloader.
//
//------------------------------------------------------------------------------
#include <bsp.h>
#include "loader.h"

//-----------------------------------------------------------------------------
// External Functions

extern BLMenuPrintDataSoc PrintSoc;
extern BOOL BLMenuShow(BLMENU_ITEM *pMenu);
extern BOOL NANDLowLevelFormat(void);
extern void DisplayInit();
extern VOID TurnOffDisplay();
//-----------------------------------------------------------------------------
// External Variables
extern BSP_ARGS *g_pBSPArgs;
extern BOOT_CFG g_BootCFG;
extern BOOL g_DownloadImage;
extern UINT32 EthDevice;  // To determine if Ethernet has been initailized for download/KITL usage
extern BOOLEAN bCFGChanged;
//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
IMAGE_TYPE  ImageTypeBIN;


//-----------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
//
static BOOL SelectBootDevice(BLMENU_ITEM *pMenu);
static BOOL SelectEtherDevice(BLMENU_ITEM *pMenu);
static BOOL PrintData(WCHAR key,VOID * dwValue,VOID * mask);
static BOOL BLMenuNANDLowLevelFormat(BLMENU_ITEM *pMenu);
static BOOL BLMenuSetUUID(BLMENU_ITEM *pMenu);


//Soc Menu Item Macro
#define NAND_Low_Level_Format {\
    L'F', L"NAND Low Level Format",BLMenuNANDLowLevelFormat, NULL, NULL, NULL\
    }
#define SET_UUID {\
    L'U', L"Set UUID",BLMenuSetUUID, L"UUID", g_BootCFG.ExtConfig, NULL\
    }

#define SocMenu NAND_Low_Level_Format,SET_UUID,Null_Menu
//#define SocMenu Null_Menu
#define MainMenu  { CommonMenu,SocMenu } 


// Local Variables
static BLMENU_ITEM g_menu[] = MainMenu;

//------------------------------------------------------------------------------
//
//  Function:  SelectEtherDevice
//
//  Provides selecting ethernet device.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL SelectEtherDevice(BLMENU_ITEM *pMenu) 
{   
    UNREFERENCED_PARAMETER(pMenu);      
    switch(g_BootCFG.EtherDevice) 
    {
        case ETH_DEVICE_ENET:
            g_BootCFG.EtherDevice = ETH_DEVICE_USB;
            break;
        case ETH_DEVICE_USB:
            g_BootCFG.EtherDevice = SER_DEVICE_USB;
            break;
        case SER_DEVICE_USB:
            g_BootCFG.EtherDevice = STR_DEVICE_SDMMC;
            break;
        case STR_DEVICE_SDMMC:
            g_BootCFG.EtherDevice = ETH_DEVICE_ENET;
            break;
        default:
            g_BootCFG.EtherDevice = SER_DEVICE_USB;
            break;                       
    }
    bCFGChanged = TRUE;    
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  SelectBootDevice
//
//  Provides selecting boot device.
//
//  Parameters:
//      pMenu
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL SelectBootDevice(BLMENU_ITEM *pMenu)
{
    UNREFERENCED_PARAMETER(pMenu);
    switch(g_BootCFG.autoDownloadImage) 
    {
        case BOOT_CFG_AUTODOWNLOAD_NONE:
            g_BootCFG.autoDownloadImage = BOOT_CFG_AUTODOWNLOAD_NK_NAND;
            break;
        case BOOT_CFG_AUTODOWNLOAD_NK_NAND:
            g_BootCFG.autoDownloadImage = BOOT_CFG_AUTODOWNLOAD_NK_SD;
            break;
        default:
            g_BootCFG.autoDownloadImage = BOOT_CFG_AUTODOWNLOAD_NONE;
            break;                       
    }
    bCFGChanged = TRUE;
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  PrintData
//
//  Provides Printing the SOC data.
//
//  Parameters:
//      key
//      dwValue
//      mask
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------
BOOL  PrintData(WCHAR key,VOID * dwValue,VOID * mask)
{
    UNREFERENCED_PARAMETER(dwValue);
    UNREFERENCED_PARAMETER(mask);

    switch(key) 
    {
        case L'5':
            switch(g_BootCFG.autoDownloadImage) 
            {
                case BOOT_CFG_AUTODOWNLOAD_NK_NAND:
                    KITLOutputDebugString(" : NK from NAND\r\n");
                    break;
                case BOOT_CFG_AUTODOWNLOAD_NK_SD:
                    KITLOutputDebugString(" : NK from SD/MMC\r\n");
                    break;
                default:
                    KITLOutputDebugString(" : Disabled\r\n");
                    break;
            }
            break;  
    
        case L'E':
            switch(g_BootCFG.EtherDevice) 
            {
                case ETH_DEVICE_ENET:
                    KITLOutputDebugString(" : ENET \r\n");
                    break;
                case ETH_DEVICE_USB:
                    KITLOutputDebugString(" : USB RNDIS\r\n");
                    break;
                case SER_DEVICE_USB:
                    KITLOutputDebugString(" : USB Serial\r\n");
                    break;
                case STR_DEVICE_SDMMC:
                    KITLOutputDebugString(" : SDMMC Storage\r\n");
                    break;
                default:
                    KITLOutputDebugString(" : Invalid Device\r\n");
                    break;
            }
            break;  
            
        case L'U':
            KITLOutputDebugString ( " : %x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x\r\n", 
                *((WORD *)dwValue) & 0x00FF,  *((WORD *)dwValue) >> 8,
                *((WORD *)dwValue + 1) & 0x00FF,  *((WORD *)dwValue + 1) >> 8,
                *((WORD *)dwValue + 2) & 0x00FF,  *((WORD *)dwValue + 2) >> 8,
                *((WORD *)dwValue + 3) & 0x00FF,  *((WORD *)dwValue + 3) >> 8,
                *((WORD *)dwValue + 4) & 0x00FF,  *((WORD *)dwValue + 4) >> 8,
                *((WORD *)dwValue + 5) & 0x00FF,  *((WORD *)dwValue + 5) >> 8,
                *((WORD *)dwValue + 6) & 0x00FF,  *((WORD *)dwValue + 6) >> 8,
                *((WORD *)dwValue + 7) & 0x00FF,  *((WORD *)dwValue + 7) >> 8);
            break;  
        
        default:            
            KITLOutputDebugString ( "\r\n");            
            break;
    }
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuNANDLowLevelFormat
//
//  Provides boot loader menu interface.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------

BOOL BLMenuNANDLowLevelFormat(BLMENU_ITEM *pMenu)
{
    volatile BOOL ret = TRUE;
    UNREFERENCED_PARAMETER(pMenu); 
    NANDLowLevelFormat();
    while(ret);
    SpinForever();
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenuSetUUID
//
//  Provides boot loader menu interface.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates exiting menu. FALSE indicates go back to menu.
//
//------------------------------------------------------------------------------

BOOL BLMenuSetUUID(BLMENU_ITEM *pMenu)
{
    WORD UUIDBuffer[8] = {0};
    WCHAR buffer[33] = {0};
    UINT32 i = 0;
    UNREFERENCED_PARAMETER(pMenu);
    
    OALLog(L" Enter UUID(MAX 32 numbers): ");

    // Read input line
    if (BLMenuReadLine(buffer, dimof(buffer)) == 0) 
        return FALSE;
    
    i = 0;
    while(i < 32)
    {
        if(!(((buffer[i] >= '0') && (buffer[i] <= '9')) || ((buffer[i] >= 'A') && (buffer[i] <= 'F')) || ((buffer[i] >= 'a') && (buffer[i] <= 'f'))))
        {
            OALLog(L" Invalid UUID number,please input 32 characters '0'-'F'!\r\n");  
            return FALSE;
        }
        i++;
    }
    
    for(i = 0;i < 3;i++)
    {
        // Convert string to MAC address
        OALKitlStringToMAC(buffer + i * 12, UUIDBuffer + i * 3 );
        // Print final MAC address
        OALLog(L" UUID part %d set to %s\r\n",i + 1,OALKitlMACtoString(UUIDBuffer + i * 3 ));        
    }
    // If it's a carriage return with an empty string, don't change anything.
    memcpy(g_BootCFG.ExtConfig,UUIDBuffer,16);
    bCFGChanged = TRUE;  

    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  BLMenu
//
//  Provides boot loader menu interface.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL BLMenu()
{
    UINT32 AutoBootDelay = 0;
    UINT32 StartTime, CurrTime, PrevTime;
    UINT32 Selection, ImageSelection;
    PrintSoc = (BLMenuPrintDataSoc)PrintData;
    
    EdbgOutputDebugString("INFO:  Initial Eboot Screen Display... \r\n");
    DisplayInit();

    // If mac address has not been programmed, immediately drop into menu
    // so user can give us one.
    if (!g_BootCFG.mac[0] && !g_BootCFG.mac[1] && !g_BootCFG.mac[2])
    {
        EdbgOutputDebugString("WARNING:  Uninitialized MAC address.  Select valid address using menu.\r\n");
        Selection = 0x20;
    }
    else
    {
        AutoBootDelay = g_BootCFG.delay;
        switch(g_BootCFG.autoDownloadImage)
        {
        case BOOT_CFG_AUTODOWNLOAD_NK_NAND:
            g_DownloadImage = FALSE;
            EdbgOutputDebugString("\r\nPress [ENTER] to launch image stored in NAND flash or [SPACE] to cancel.\r\n");
            EdbgOutputDebugString("\r\nInitiating image launch in %d seconds. ", AutoBootDelay--);
            break;

        case BOOT_CFG_AUTODOWNLOAD_NK_SD:
            g_DownloadImage = FALSE;
            EdbgOutputDebugString("\r\nPress [ENTER] to launch image stored in SD/MMC or [SPACE] to cancel.\r\n");
            EdbgOutputDebugString("\r\nInitiating image launch in %d seconds. ", AutoBootDelay--);
            break;

        default:
            g_DownloadImage = TRUE;
            EdbgOutputDebugString("\r\nPress [ENTER] to download now or [SPACE] to cancel.\r\n");
            EdbgOutputDebugString("\r\nInitiating image download in %d seconds. ", AutoBootDelay--);
            break;
        }

        // Get a snapshot of the RTC seconds count.
        //
        StartTime     = OEMEthGetSecs();
        PrevTime      = StartTime;
        CurrTime      = StartTime;
        Selection     = (UINT32)OEM_DEBUG_READ_NODATA;

        // Allow the user an amount of time to halt the auto boot/download process.
        // Count down to 0 before proceeding with default operation.
        //
        while ((CurrTime - StartTime) < g_BootCFG.delay)
        {
            UINT8 i=0;
            UINT8 j;

            Selection = OEMReadDebugByte(); 
            if ((Selection == 0x20) || (Selection == 0x0d))
            {
                break;
            }
            CurrTime = OEMEthGetSecs();   
            if (CurrTime > PrevTime)
            {
                PrevTime = CurrTime;
                if (AutoBootDelay < 9)
                    i = 11;
                else if (AutoBootDelay < 99)
                    i = 12;
                else if (AutoBootDelay < 999)
                    i = 13;

                for (j = 0; j < i; j++)
                {
                    OEMWriteDebugByte((BYTE)0x08); // print back space
                }

                KITLOutputDebugString ( "%d seconds. ", AutoBootDelay--);
            }
        }
    }

    switch (Selection)
    {
        case OEM_DEBUG_READ_NODATA: // fall through if nothing typed
        case 0x0d: // user canceled wait
        {
            if (g_BootCFG.autoDownloadImage)
            {
                KITLOutputDebugString ( "\r\nLaunching flash image  ... \r\n");
            }
            else
            {
                KITLOutputDebugString ( "\r\nStarting auto download ... \r\n");
            }
            break;
        }
        case 0x20:
        {            
            BLMenuShow(g_menu);
            break;
        }
        default:
            break;
    }

    // SD Downloading : Image Selection
    if((g_DownloadImage) && (g_BootCFG.EtherDevice == STR_DEVICE_SDMMC))
    {
        KITLOutputDebugString("\r\nImage Type Selection : 1. XLDR  2. EBOOT  3. NK\r\n");
        KITLOutputDebugString("Please select from the above three(1/2/3): ");
        do {
            ImageSelection = tolower(OEMReadDebugByte());
        } while ((ImageSelection != '1') && (ImageSelection != '2') && (ImageSelection != '3'));
        KITLOutputDebugString("%c\r\n\r\n", ImageSelection);
                        
        switch(ImageSelection)
        {
        case '1':
            ImageTypeBIN = IMAGE_TYPE_XLDR;
            break;
        case '2':
            ImageTypeBIN = IMAGE_TYPE_BOOT;
            break;
        case '3':
            ImageTypeBIN = IMAGE_TYPE_NK;
            break;
        default:
            ImageTypeBIN = IMAGE_TYPE_NK;
            break;
        }

        // Set SDMMC Downloading TRUE
        g_StorageSDDownload = TRUE;
    }
    
    if(bCFGChanged)
    {
        StoreBootCFG(&g_BootCFG);
        ConfigBootCFG(&g_BootCFG);
    }    

    if(g_BootCFG.EtherDevice == ETH_DEVICE_USB)
    {
        StoreBootCFG(&g_BootCFG);
        ConfigBootCFG(&g_BootCFG);
    }
    
    TurnOffDisplay();
    // configuration parameters previously loaded.
    memcpy(g_pBSPArgs->kitl.mac, g_BootCFG.mac, 6);

    // Don't initialize any ethernet controller in case no image needs to be
    // downloaded but just USB KITL is enabled. Otherwise initialize the ethernet
    // controller selected by user.
    if ((!g_DownloadImage) && (g_BootCFG.dwConfigFlags & CONFIG_FLAGS_KITL_ENABLE) && 
        ((g_BootCFG.EtherDevice == ETH_DEVICE_USB) || (g_BootCFG.EtherDevice == SER_DEVICE_USB)))
    {
        if (g_BootCFG.EtherDevice == ETH_DEVICE_USB)
        {
            g_pBSPArgs->kitl.devLoc.PhysicalLoc = (PVOID)CSP_BASE_REG_PA_USBCTRL0;
            g_pBSPArgs->kitl.devLoc.LogicalLoc = (DWORD)g_pBSPArgs->kitl.devLoc.PhysicalLoc ;
            return(TRUE);
        }
        if (g_BootCFG.EtherDevice == SER_DEVICE_USB)
        {
            g_pBSPArgs->kitl.devLoc.PhysicalLoc = (PVOID)(CSP_BASE_REG_PA_USBCTRL0 + 1);
            g_pBSPArgs->kitl.devLoc.LogicalLoc = (DWORD)g_pBSPArgs->kitl.devLoc.PhysicalLoc ;
            return(TRUE);
        }
    }

    // If active KITL is enabled at boot, or user selected the download option,
    // locate and initialize an Ethernet controller.
    //
    if (((g_BootCFG.dwConfigFlags & CONFIG_FLAGS_KITL_ENABLE) && ((g_BootCFG.dwConfigFlags & CONFIG_FLAGS_KITL_PASSIVE) == 0)) || g_DownloadImage)
    {
        EthDevice = InitSpecifiedEthDevice(&g_pBSPArgs->kitl, g_BootCFG.EtherDevice);
        if (EthDevice == -1)
        {
            // No device was found ... 
            //
            KITLOutputDebugString("ERROR: Failed to detect and initialize Ethernet controller.\r\n");
            return(FALSE);
        }

        // Make sure MAC address has been programmed.
        //
        if (!g_pBSPArgs->kitl.mac[0] && !g_pBSPArgs->kitl.mac[1] && !g_pBSPArgs->kitl.mac[2])
        {
            KITLOutputDebugString("ERROR: Invalid Ethernet address read from Ethernet controller.\n");
            return(FALSE);
        }

        KITLOutputDebugString("INFO: MAC address: %x-%x-%x-%x-%x-%x\r\n",
            g_pBSPArgs->kitl.mac[0] & 0x00FF, g_pBSPArgs->kitl.mac[0] >> 8,
            g_pBSPArgs->kitl.mac[1] & 0x00FF, g_pBSPArgs->kitl.mac[1] >> 8,
            g_pBSPArgs->kitl.mac[2] & 0x00FF, g_pBSPArgs->kitl.mac[2] >> 8);
    }

    KITLOutputDebugString("-BLMenu .\r\n");
    return(TRUE);
}



