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

#ifndef __BLMENU_H
#define __BLMENU_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------


typedef struct MENU_ITEM {
    WCHAR key;
    LPCWSTR text;
    BOOL (*pfnAction)(struct MENU_ITEM *);
    VOID *pParam1;
    VOID *pParam2;
    VOID *pParam3;
} BLMENU_ITEM;

typedef enum
{
    ETH_DEVICE_LAN911x,
    ETH_DEVICE_FEC,
    ETH_DEVICE_ENET,
    ETH_DEVICE_ENC28J60,
    ETH_DEVICE_USB,
    SER_DEVICE_USB,
    STR_DEVICE_SDMMC
} ETH_DEVICE_TYPE;

typedef struct
{
    DWORD autoDownloadImage;
    DWORD IP;
    DWORD subnetMask;
    DWORD numBootMe;
    DWORD delay;
    DWORD DHCPEnable;
    DWORD dwPhysStart;
    DWORD dwPhysLen;
    DWORD dwLaunchAddr;					// CS&ZHL MAY-23-2011: physical start address of NK in RAM
    WORD  mac[4];						// Increase from 3 to 4 for pad purpose
    DWORD dwNumOfPort;					// CS&ZHL APR-10-2012: number of ethernet ports, default = 1
    DWORD dwYear;						// CS&ZHL APR-10-2012: start from 2012
    DWORD dwMonth;						// CS&ZHL APR-10-2012: 1 - 12
    DWORD dwDay;						// CS&ZHL APR-10-2012: 1 - 31
    DWORD ConfigMagicNumber;
    DWORD Channel;
    DWORD dwSerPhysAddr;
    DWORD dwConfigFlags;
    DWORD BaudRate;
    DWORD DataBits;
    DWORD Parity;
    DWORD StopBit;
    DWORD FlowCtrl;
    ETH_DEVICE_TYPE EtherDevice;
    WORD  ExtConfig[20];
} BOOT_CFG, *PBOOT_CFG;



typedef BOOL (*BLMenuPrintDataSoc)(WCHAR,VOID *,VOID *);


#define CONFIG_FLAGS_KITL_INT           (1 << 0)
#define CONFIG_FLAGS_KITL_PASSIVE       (1 << 1)
#define CONFIG_FLAGS_KITL_ENABLE        (1 << 2)

//------------------------------------------------------------------------------







//------------------------------------------------------------------------------
//Macro Define

#define dimof(x)                (sizeof(x)/sizeof(x[0]))

#define IP_Address {\
    L'0', L"IP Address", BLMenuSetIpAddress,L"IP Address", &g_BootCFG.IP, NULL\
    }
#define IP_Mask {\
    L'1', L"Set IP Mask", BLMenuSetIpMask,L"IP Mask", &g_BootCFG.subnetMask, NULL\
    }
#define Boot_Delay {\
    L'2', L"Boot Delay", BLMenuBootDelay,NULL, &g_BootCFG.delay, &g_BootCFG\
    }
#define DHCP {\
    L'3', L"DHCP", BLMenuEnableDisable,NULL, &g_BootCFG.DHCPEnable, NULL\
    }
#define Reset {\
    L'4', L"Reset to Factory Default Configuration", BLMenuResetConfiguration,NULL, NULL, &g_BootCFG\
    }
#define Select_Boot_Device {\
    L'5', L"Select Boot Device", SelectBootDevice,NULL, &g_BootCFG.autoDownloadImage, NULL\
    }
#define MAC {\
    L'6', L"Set MAC Address", BLMenuSetMacAddress,L"MAC Address", &g_BootCFG.mac, NULL\
    }
#define Format_OS {\
    L'7', L"Format OS NAND Region", BLMenuFormatNandOS,NULL, NULL, NULL\
    }
#define Format_All {\
    L'8', L"Format All NAND Regions", BLMenuFormatAllNand,NULL, NULL, NULL\
    }
#define Boot_Shell {\
    L'9', L"Bootloader Shell", BLMenuBootShell,NULL, NULL, NULL\
    }
#define KITL_Work_Mode {\
    L'I', L"KITL Work Mode", BLMenuEnableDisable,NULL, &g_BootCFG.dwConfigFlags, (VOID*)CONFIG_FLAGS_KITL_INT\
    }
#define KITL_Enable_Mode {\
    L'K', L"KITL Enable Mode", BLMenuEnableDisable,NULL, &g_BootCFG.dwConfigFlags,(VOID*)CONFIG_FLAGS_KITL_ENABLE\
    }
#define KITL_Passive_Mode {\
    L'P', L"KITL Passive Mode", BLMenuEnableDisable,NULL, &g_BootCFG.dwConfigFlags,(VOID*)CONFIG_FLAGS_KITL_PASSIVE\
    }
#define Save_Setting {\
    L'S', L"Save Settings", BLMenuSaveSettings,NULL, NULL, &g_BootCFG\
    }
#define Download_Image {\
    L'D', L"Download Image Now",BLMenuDownloadImageNow,NULL, NULL, &g_BootCFG\
    }
#define Launch_Flash_Image {\
    L'L', L"Launch Existing Flash Resident Image Now", BLMenuLaunchFlashImage,NULL, NULL, &g_BootCFG\
    }
#define Select_Ether_Device {\
    L'E', L"Select Ether Device", SelectEtherDevice,NULL, &g_BootCFG.EtherDevice, NULL,\
    }
#define MMC_SD_Utilities {\
    L'M', L"MMC and SD Utilities", BLMenuMMCSDUtilities,NULL, NULL, NULL\
    }
#define Null_Menu {\
    0, NULL, NULL,NULL, NULL, NULL\
    }

#define CommonMenu IP_Address,IP_Mask,Boot_Delay,DHCP,Reset,Select_Boot_Device,MAC,Format_OS,Format_All,\
Boot_Shell,KITL_Work_Mode,KITL_Enable_Mode,KITL_Passive_Mode,Save_Setting,Download_Image,Launch_Flash_Image,Select_Ether_Device,\
MMC_SD_Utilities,Null_Menu


#define eMMC_Toggle_Boot_Partition {\
    L'A', L"eMMC 4.3 Boot Partition", BLMenuEMMCToggleBP,NULL, &extCSDBuffer[179], NULL\
    }
#define eMMC_BP_Size {\
    L'B', L"eMMC 4.3 Boot Partition Size", BLMenuEMMCBPSize,NULL, &extCSDBuffer[226], NULL\
    }
#define eSD_BP_Activate {\
    L'S', L"eSD 2.1 Boot Partition", BLMenuESDBPEnable,NULL, &eSDBPEnabled, NULL\
    }
#define eSD_BP_Size {\
    L'D', L"eSD 2.1 Boot Partition Size", BLMenuESDBPSize,NULL, &eSDBootPartitionSize, NULL\
    }
#define Create_File_System_on_SD_MMC { \
    L'C', L"Create File System Partition on SD/MMC Boot Card", BLMenuCreateFileSystemOnSDMMC, NULL, NULL, NULL\
    }
#define Format_All_SD_MMC {\
    L'F', L"Format All SD/MMC Regions", BLMenuFormatAllSDMMC,NULL, NULL, NULL\
    }
#define Return_to_Main_Menu { \
    L'R', L"Return to Main Menu", BLMenuReturn, NULL, NULL, NULL\
    }

#define MMCSDSubMenu {eMMC_Toggle_Boot_Partition,eMMC_BP_Size,eSD_BP_Activate,eSD_BP_Size,Create_File_System_on_SD_MMC,\
    Format_All_SD_MMC,Return_to_Main_Menu,Null_Menu,Null_Menu}

BOOL BLMenuShow(BLMENU_ITEM *pMenu);
BOOL BLMenuEnableDisable(BLMENU_ITEM *pMenu);
BOOL BLMenuSetMacAddress(BLMENU_ITEM *pMenu);
BOOL BLMenuSetIpAddress(BLMENU_ITEM *pMenu);
BOOL BLMenuSetIpMask(BLMENU_ITEM *pMenu);
BOOL BLMenuBootShell(BLMENU_ITEM *pMenu);
BOOL BLMenuSaveSettings(BLMENU_ITEM *pMenu);
BOOL BLMenuLaunchFlashImage(BLMENU_ITEM *pMenu);
BOOL BLMenuBootDelay(BLMENU_ITEM *pMenu);
BOOL BLMenuResetConfiguration(BLMENU_ITEM *pMenu);
BOOL BLMenuDownloadImageNow(BLMENU_ITEM *pMenu);
BOOL BLMenuFormatNandOS(BLMENU_ITEM *pMenu);
BOOL BLMenuFormatAllNand(BLMENU_ITEM *pMenu);
BOOL BLMenuMMCSDUtilities(BLMENU_ITEM *pMenu);
BOOL BLMenuEMMCToggleBP(BLMENU_ITEM *pMenu);
BOOL BLMenuEMMCBPSize(BLMENU_ITEM *pMenu);
BOOL BLMenuESDBPEnable(BLMENU_ITEM *pMenu);
BOOL BLMenuESDBPSize(BLMENU_ITEM *pMenu);
BOOL BLMenuReturn(BLMENU_ITEM *pMenu);
BOOL BLMenuFormatAllSDMMC(BLMENU_ITEM *pMenu);
BOOL BLMenuCreateFileSystemOnSDMMC(BLMENU_ITEM *pMenu);

UINT32 BLMenuReadLine(LPWSTR buffer, UINT32 charCount);
BOOL   BLMenuPrintData(WCHAR key,VOID * dwValue,VOID * mask);
extern INT OEMReadDebugByte();
extern VOID BspBootShell();
extern void ResetDefaultBootCFG(BOOT_CFG *pBootCFG);
extern BOOL StoreBootCFG(BOOT_CFG *BootCFG);
extern void ConfigBootCFG(BOOT_CFG *pBootCFG);
extern BOOL NANDFormatNK(VOID);
extern BOOL NANDFormatAll(VOID);
extern BOOL SDHCFormatAll(VOID);
extern BOOL SDHCCreateFileSystemPartition(VOID);
extern VOID SetBootMe(BOOT_CFG *pBootCFG);
extern VOID SetDelay(BOOT_CFG *pBootCFG);
extern VOID SpinForever();


//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
