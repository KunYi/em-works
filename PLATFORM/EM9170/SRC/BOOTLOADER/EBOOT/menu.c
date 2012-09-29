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

#include "bsp.h"

//-----------------------------------------------------------------------------
// External Functions

extern BLMenuPrintDataSoc PrintSoc;
extern BOOL BLMenuShow(BLMENU_ITEM *pMenu);

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


//-----------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
//
static BOOL SelectBootDevice(BLMENU_ITEM *pMenu);
static BOOL SelectEtherDevice(BLMENU_ITEM *pMenu);
static BOOL PrintData(WCHAR key,VOID * dwValue,VOID * mask);

#define SocMenu Null_Menu
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
        case ETH_DEVICE_LAN911x:
            g_BootCFG.EtherDevice = ETH_DEVICE_FEC;
            break;
        case ETH_DEVICE_FEC:
            g_BootCFG.EtherDevice = ETH_DEVICE_USB;
            break;
        case ETH_DEVICE_USB:
            g_BootCFG.EtherDevice = SER_DEVICE_USB;
            break;
        case SER_DEVICE_USB:
            g_BootCFG.EtherDevice = ETH_DEVICE_LAN911x;
            break;
        default:
            g_BootCFG.EtherDevice = ETH_DEVICE_LAN911x;
            break;                       
    }
    bCFGChanged=TRUE;    
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
    bCFGChanged=TRUE;
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
                case ETH_DEVICE_LAN911x:
                    KITLOutputDebugString(" : LAN9217\r\n");
                    break;
                case ETH_DEVICE_FEC:
                    KITLOutputDebugString(" : FEC\r\n");
                    break;
                case ETH_DEVICE_USB:
                    KITLOutputDebugString(" : USB RNDIS\r\n");
                    break;
                case SER_DEVICE_USB:
                    KITLOutputDebugString(" : USB Serial\r\n");
                    break;
                default:
                    KITLOutputDebugString(" : Invalid Device\r\n");
                    break;
            }
            break;  
                       
        default:            
            KITLOutputDebugString ( "\r\n");            
            break;
    }
    return TRUE;
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
    UINT32 Selection;
	UINT32  nDBGSL = 1;
    PrintSoc = (BLMenuPrintDataSoc)PrintData;
    
    // If mac address has not been programmed, immediately drop into menu
    // so user can give us one.
    if (!g_BootCFG.mac[0] && !g_BootCFG.mac[1] && !g_BootCFG.mac[2])
    {
        EdbgOutputDebugString("WARNING:  Uninitialized MAC address.  Select valid address using menu.\r\n");
        Selection = 0x20;
    }
    else
    {
#ifdef EM9170
		//
		// CS&ZHL Jun 30-2011: check DBGSLn status
		//	
		PEM9K_CPLD_REGS		pCPLD;

		pCPLD = (PEM9K_CPLD_REGS)OALPAtoUA(CSP_BASE_MEM_PA_CS5);
		if(pCPLD == NULL)
		{
			OALMSG(1, (L"OALIoCtlHalBoardStateRead: CPLD pointer map failed\r\n"));
		}
		else
		{
			//read board state
			if( !(INREG8(&pCPLD->StateReg) & 0x80 ))
			{
				nDBGSL = 0;
			}
		}

#endif
		if( nDBGSL )    // for debug mode
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
		else       // for run mode
		{
			switch(g_BootCFG.autoDownloadImage)
			{
			case BOOT_CFG_AUTODOWNLOAD_NK_NAND:
			case BOOT_CFG_AUTODOWNLOAD_NK_SD:
				g_DownloadImage = FALSE;
				break;
			default:
				g_DownloadImage = TRUE;
				break;
			}
			Selection = 0x0d;               // just like [Enter] received
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

    if (bCFGChanged == TRUE)
    {
        StoreBootCFG(&g_BootCFG);
        ConfigBootCFG(&g_BootCFG);
    }    
    
    // EEPROM on ADS does not have proper contents to allow CS8900 to automatically
    // read MAC address during initialization.  We must provide MAC from the Boot
    // configuration parameters previously loaded.
    memcpy(g_pBSPArgs->kitl.mac, g_BootCFG.mac, 6);

    // Don't initialize any ethernet controller in case no image needs to be
    // downloaded but just USB KITL is enabled. Otherwise initialize the ethernet
    // controller selected by user.
    if ((!g_DownloadImage) && (g_BootCFG.dwConfigFlags & CONFIG_FLAGS_KITL_ENABLE) && (g_BootCFG.EtherDevice == ETH_DEVICE_USB))
    {
        g_pBSPArgs->kitl.devLoc.PhysicalLoc = (PVOID)CSP_BASE_REG_PA_USB;
        g_pBSPArgs->kitl.devLoc.LogicalLoc = (DWORD)g_pBSPArgs->kitl.devLoc.PhysicalLoc ;
        return(TRUE);
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



