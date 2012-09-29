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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  eboot.c
//
//  Core routines for the Ethernet bootloader.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include <ethdbg.h>
#pragma warning(push)
#pragma warning(disable: 4115)
#include <fmd.h>
#pragma warning(pop)
#include "loader.h"


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables
extern BOOL g_DownloadImage;
extern BOOT_CFG g_BootCFG;
extern BOOT_BINDIO_CONTEXT g_BinDIO;
extern BSP_ARGS *g_pBSPArgs;

extern PFN_EDBG_INIT pfnEDbgInit;
extern PFN_EDBG_GET_FRAME pfnEDbgGetFrame;
extern PFN_EDBG_SEND_FRAME pfnEDbgSendFrame;
extern PFN_EDBG_ENABLE_INTS pfnEDbgEnableInts;
extern PFN_EDBG_DISABLE_INTS pfnEDbgDisableInts;
extern PFN_EDBG_GET_PENDING_INTS pfnEDbgGetPendingInts;
extern PFN_EDBG_READ_EEPROM pfnEDbgReadEEPROM;
extern PFN_EDBG_WRITE_EEPROM pfnEDbgWriteEEPROM;
extern PFN_EDBG_SET_OPTIONS pfnEDbgSetOptions;

extern BOOL SerialReadData(DWORD cbData, LPBYTE pbData);
extern BOOL SDMMCReadData(DWORD cbData, LPBYTE pbData);

//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
DWORD EdbgDebugZone;
EDBG_ADDR g_DeviceAddr; // NOTE: global used so it remains in scope throughout download process
                        // since eboot library code keeps a global pointer to the variable provided.

INT32 EthDevice = -1;  // To determine if Ethernet has been initailized for download/KITL usage

//-----------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
//
void ResetDefaultBootCFG(BOOT_CFG *pBootCFG);

//------------------------------------------------------------------------------
//
//  Function:  OEMBootInit
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//------------------------------------------------------------------------------
void OEMBootInit (void)
{
    KITLOutputDebugString("Microsoft Windows CE Ethernet Bootloader %d.%d for MX28 (%s %s)\r\n",
                          EBOOT_VERSION_MAJOR, EBOOT_VERSION_MINOR, __DATE__, __TIME__);

    return;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMGetMagicNumber
//
//  Parameters:
//      None.
//
//  Returns:
//      Magic number of EBOOT version.
//------------------------------------------------------------------------------
UINT32 OEMGetMagicNumber()
{
    return EBOOT_CFG_MAGIC_NUMBER;
}


//------------------------------------------------------------------------------
//
//  Function:  GetPreDownloadInfo
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE for Download/FALSE for jump to resident OS image
//------------------------------------------------------------------------------
BOOL GetPreDownloadInfo (PBOOT_CFG p_bootCfg)
{
    BOOL fGotJumpImg = FALSE;
    UINT32 DHCPLeaseTime = 0;
    UINT32 *pDHCPLeaseTime = &DHCPLeaseTime;
    UINT32 SubnetMask;
    UINT32 BootFlags = 0;


    // Check if the user wants to use DHCP
    //
    if (p_bootCfg->DHCPEnable == FALSE)
    {
        pDHCPLeaseTime             = NULL;
        g_pBSPArgs->kitl.ipAddress = p_bootCfg->IP;
        g_pBSPArgs->kitl.ipMask    = p_bootCfg->subnetMask;
        g_pBSPArgs->kitl.flags    &= ~OAL_KITL_FLAGS_DHCP;
    }

    // Initialize Ethernet transport if we are in active KITL mode or we
    // need to download the image
    if (g_DownloadImage)
    {
        // Initialize the TFTP transport.
        memcpy(g_DeviceAddr.wMAC, g_pBSPArgs->kitl.mac, (sizeof(UINT16) * 3));
        g_DeviceAddr.dwIP  = g_pBSPArgs->kitl.ipAddress;
        g_DeviceAddr.wPort = 0;
        SubnetMask         = g_pBSPArgs->kitl.ipMask;

        if (!EbootInitEtherTransport(&g_DeviceAddr,
                                     (LPDWORD)&SubnetMask,
                                     &fGotJumpImg,
                                     (DWORD *)pDHCPLeaseTime,
                                     EBOOT_VERSION_MAJOR,
                                     EBOOT_VERSION_MINOR,
                                     BSP_DEVICE_PREFIX,
                                     (CHAR *)g_pBSPArgs->deviceId,
                                     EDBG_CPU_ARM720,
                                     BootFlags))
        {
            return(FALSE);
        }

        // If the user wanted a DHCP address, save the values obtained in the init call above.
        //
        if (p_bootCfg->DHCPEnable)
        {
            g_pBSPArgs->kitl.ipAddress  = g_DeviceAddr.dwIP;
            g_pBSPArgs->kitl.ipMask     = SubnetMask;
            g_pBSPArgs->kitl.flags     |= OAL_KITL_FLAGS_DHCP;
        }
    }

    return(fGotJumpImg);
}


//------------------------------------------------------------------------------
//
//  Function:  GetLaunchInfo
//
//  Parameters:
//     None.
//
//  Returns:
//      None.
//------------------------------------------------------------------------------
void GetLaunchInfo (void)
{
    EDBG_OS_CONFIG_DATA *pCfgData;
    EDBG_ADDR EshellHostAddr;

    memset(&EshellHostAddr, 0, sizeof(EDBG_ADDR));

    KITLOutputDebugString("INFO: EbootWaitForHostConnect (IP = %s, MAC = %x-%x-%x-%x-%x-%x, Port = 0x%x)\r\n",
                          inet_ntoa(g_DeviceAddr.dwIP), g_DeviceAddr.wMAC[0] & 0x00FF, g_DeviceAddr.wMAC[0] >> 8,
                          g_DeviceAddr.wMAC[1] & 0x00FF, g_DeviceAddr.wMAC[1] >> 8,
                          g_DeviceAddr.wMAC[2] & 0x00FF, g_DeviceAddr.wMAC[2] >> 8,
                          g_DeviceAddr.wPort);

#if 0 // Remove-W4: Warning C4706 workaround
    if (!(pCfgData = EbootWaitForHostConnect(&g_DeviceAddr, &EshellHostAddr)))
#else
    if ((pCfgData = EbootWaitForHostConnect(&g_DeviceAddr, &EshellHostAddr)) == 0)
#endif
    {
        KITLOutputDebugString("ERROR: EbootWaitForHostConnect failed!\r\n");
        SpinForever();
    }

    // If the user selected "passive" KITL (i.e., don't connect to the target at boot time), set the
    // flag in the args structure so the OS image can honor it when it boots.
    //
    if (pCfgData && (pCfgData->KitlTransport & KTS_SERIAL))
    {
        //g_pBSPArgs->kitl.devLoc.PhysicalLoc = (PVOID)BSP_BASE_REG_PA_SERIALKITL;;
    }
    else if ( (pCfgData && (pCfgData->KitlTransport & KTS_ETHER)) && (EthDevice == -1) )
    {
        EthDevice = InitSpecifiedEthDevice(&g_pBSPArgs->kitl,  ETH_DEVICE_LAN911x);
        if (EthDevice == -1)
        {
            // No device was found ...
            //
            KITLOutputDebugString("ERROR: Failed to detect and initialize Ethernet controller.\r\n");
            return;
        }
    }
    if (pCfgData && (pCfgData->KitlTransport & KTS_PASSIVE_MODE))
    {
        g_pBSPArgs->kitl.flags = OAL_KITL_FLAGS_PASSIVE;
    }

    return;
}

//-----------------------------------------------------------------------------
//
//  Function:  OEMReadData
//
//  This function reads data from the transport during the download process.
//  It is called by the BLCOMMON framework.
//
//  Parameters:
//      cbData
//          [in] Amount of data, in bytes, to read.
//
//      pbData
//          [out] Pointer to read buffer.
//
//  Returns:
//      TRUE for success/FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL OEMReadData(DWORD cbData, LPBYTE pbData)
{
    // TODO: increment bytes read to track download progress.
    // Save read data size and location. It is used in workaround
    // for download BIN DIO images larger than RAM.
    g_BinDIO.readSize = cbData;
    g_BinDIO.pReadBuffer = pbData;

    if(g_SerialUSBDownload)
    {
       return(SerialReadData(cbData, pbData));
    }
    else if(g_StorageSDDownload)
    {
        return(SDMMCReadData(cbData, pbData));
    }
    else
    {
        // TODO: increment bytes read to track download progress.
        return(EbootEtherReadData(cbData, pbData));
    }

}


//------------------------------------------------------------------------------
//
//  Function:  ResetDefaultBootCFG
//
//  Resets the debug bootloader configuration information (menu settings, etc.).
//
//  Parameters:
//      BootCfg
//          [out] Points to bootloader configuration that will be filled with
//          default data.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
void ResetDefaultBootCFG(BOOT_CFG *pBootCFG)
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    BOOT_CFG BootCfg = {0};
#endif

    KITLOutputDebugString("\r\nResetting factory default configuration...\r\n");

#ifdef	EM9280
	pBootCFG->autoDownloadImage  = BOOT_CFG_AUTODOWNLOAD_NK_NAND;
#else
    pBootCFG->autoDownloadImage  = BOOT_CFG_AUTODOWNLOAD_NONE;
#endif	//EM9280

	// CS&ZHL APR-1-2012: set default NK image virtual start address and size
	//pBootCFG->dwNandImageStart   = IMAGE_BOOT_NKIMAGE_NAND_CA_START;	// CS&ZHL MAR-27-2012: virtual start address of OS image in NandFlash
    //pBootCFG->dwNandImageLength  = IMAGE_BOOT_NKIMAGE_NAND_SIZE;		// max size = 48MB
	// end of CS&ZHL APR-1-2012: set default NK image virtual start address and size

    pBootCFG->dwLaunchAddr       = (DWORD)OALPAtoCA(IMAGE_BOOT_NKIMAGE_RAM_PA_START);
    pBootCFG->dwPhysStart        = 0;
    pBootCFG->dwPhysLen          = 0;
    pBootCFG->mac[0]             = 0x9BD0;
    pBootCFG->mac[1]             = 0x0005;
    pBootCFG->mac[2]             = 0x0500;
    pBootCFG->ConfigMagicNumber  = EBOOT_CFG_MAGIC_NUMBER;
    pBootCFG->numBootMe          = 50;
    pBootCFG->delay              = 5;					//3;

    pBootCFG->IP                 = inet_addr("192.168.201.172");
    pBootCFG->subnetMask         = inet_addr("255.255.255.0");
    pBootCFG->DHCPEnable         = FALSE;										//TRUE;

    pBootCFG->dwConfigFlags      = CONFIG_FLAGS_KITL_INT;
    pBootCFG->EtherDevice        = ETH_DEVICE_ENET;

    // save it back to flash
    if (!FlashStoreBootCFG((BYTE*) pBootCFG, sizeof(BOOT_CFG)))
    {
        KITLOutputDebugString("ERROR: ResetDefaultBootCFG: failed to store configuration to flash.\r\n");
    }
#ifdef DEBUG
    else
    {
        KITLOutputDebugString("INFO: ResetDefaultBootCFG: factory default configuration saved to flash.\r\n");

        // DEBUG
        // read it back to verify
        if (!FlashLoadBootCFG((BYTE*) &BootCfg, sizeof(BOOT_CFG)))
        {
            KITLOutputDebugString("WARNING: ResetDefaultBootCFG: failed to load configuration for double check.\r\n");
        }
        else
        {
            if (0 != memcmp((const void *)&BootCfg, (const void*)pBootCFG, sizeof(BOOT_CFG)))
            {
                KITLOutputDebugString("WARNING: ResetDefaultBootCFG: saved and retrieved data not equal.\r\n");
            }
            else
            {
                KITLOutputDebugString("INFO: ResetDefaultBootCFG: factory default configuration verified in flash.\r\n");
            }
        }
        // END DEBUG
    }
#endif
}

//-----------------------------------------------------------------------------
//
// Function: OEMEthGetFrame
//
// Reads data from the Ethernet device.
//
// Parameters:
//      pData
//          [out] Ptr to the receive buffer.
//
//      pwLength
//          [in] Length of data in the receiving buffer.
//
// Returns:
//      FALSE if no frame has been received
//
//-----------------------------------------------------------------------------
BOOL OEMEthGetFrame(PUCHAR pData, PUSHORT pwLength)
{
    return(pfnEDbgGetFrame(pData, pwLength));
}


//-----------------------------------------------------------------------------
//
// Function: OEMEthGetFrame
//
// Function checks if a frame has been received, and if so copy it to buffer.
//
// Parameters:
//      pData
//          [in] Ptr to the send buffer.
//
//      pwLength
//          [in] Length of data to be sent.
//
// Returns:
//      FALSE on failure
//
//-----------------------------------------------------------------------------
BOOL OEMEthSendFrame(PUCHAR pData, DWORD dwLength)
{
    BYTE Retries = 0;

    while (Retries++ < 4)
    {
        if (!pfnEDbgSendFrame(pData, dwLength))
            return(TRUE);

        KITLOutputDebugString("INFO: OEMEthSendFrame: retrying send (%u)\r\n", Retries);
    }

    return(FALSE);
}

