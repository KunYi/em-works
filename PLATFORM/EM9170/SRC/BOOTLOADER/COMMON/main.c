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
//  File:  main.c
//
//  Common routines for the bootloader.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include <ethdbg.h>
#pragma warning(push)
#pragma warning(disable: 4115)
#include <fmd.h>
#pragma warning(pop)
#include "loader.h"
#include "sdfmd.h"
#include "cspifmd.h"

//-----------------------------------------------------------------------------
// External Functions
extern BOOL NANDLoadIPL(VOID);
//extern BOOL NANDLoadNK(VOID);
extern BOOL NANDLoadNK(DWORD dwImageStart, DWORD dwImageLength);		// CS&ZHL MAY-23-2011: use BootCFG parameter
extern BOOL NANDCheckImageAddress(DWORD dwPhyAddr);								// CS&ZHL MAY-21-2011: add for BinFS
extern BOOL SDHCLoadNK(VOID);
extern VOID SDHCDisableBP(VOID);
extern BOOL FlashLoadBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize);
extern BOOL FlashStoreBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize);
extern void ResetDefaultBootCFG(BOOT_CFG *pBootCFG);
extern void Launch(unsigned int uAddr);
extern BOOL BLMenu();
extern UINT32 OEMGetMagicNumber();
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);
extern BOOL  SerialSendBlockAck(DWORD uBlockNumber);
extern BOOL  SerialSendBootRequest(const char * platformString);
extern BOOL  SerialWaitForBootAck(BOOL *pfJump);
extern DWORD SerialWaitForJump(VOID);
extern void  NANDBootReserved();

//
// CS&ZHL MAY-4-2011: add slpash function
//
extern void Eboot_SplashScreen(VOID);
//
// CS&ZHL MAY-4-2011: EM9170 specified init
//
//extern void EM9170_IOConfig(void);

//-----------------------------------------------------------------------------
// External Variables
extern BOOT_CFG g_BootCFG;


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
BSP_ARGS *g_pBSPArgs;
IMAGE_TYPE g_ImageType;
IMAGE_MEMORY g_ImageMemory;

BOOL g_DownloadImage = TRUE;
UCHAR *g_DefaultRamAddress;
BOOL g_bNandBootloader = FALSE;
BOOL g_bNandExist = FALSE;
BOOL g_bSDHCBootloader = FALSE;
BOOL g_bSDHCExist = FALSE;
BOOL g_bSpiBootloader = FALSE;
BOOL g_bSpiExist = FALSE;

// Used to save information about downloaded DIO mage
BOOT_BINDIO_CONTEXT g_BinDIO;

//
// CS&ZHL MAY-19-2011: variables for BinFS
//
DWORD g_dwTotalBinNum;			// CS&ZHL MAY-21-2011: get from OEMMultiBINNotify()
DWORD g_dwCurrentBinNum;
DWORD g_dwBinAddress[16];		// CS&ZHL MAY-21-2011: Flash Physical Addresses
DWORD g_dwBinLength[16];			// Now we support 16 bin files for MultiBIN

//-----------------------------------------------------------------------------
// External Variables
extern PCSP_CRM_REGS g_pCRM;

//-----------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
//
BOOL LoadBootCFG(BOOT_CFG *BootCFG);
BOOL StoreBootCFG(BOOT_CFG *BootCFG);
void ConfigBootCFG(BOOT_CFG *pBootCFG);
BOOL OEMVerifyMemory(DWORD dwStartAddr, DWORD dwLength);
BOOL OEMReportError (DWORD dwReason, DWORD dwReserved);
void OEMMultiBINNotify(const PMultiBINInfo pInfo);

BOOLEAN             g_SerialUSBDownload = FALSE;

//------------------------------------------------------------------------------
//
//  Function:  main
//
//  Bootloader main routine.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//------------------------------------------------------------------------------
void main(void)
{
    //Turn off debug messages until the debug serial is ready
    OALLogSetZones(0);
  
    // Common boot loader (blcommon) main routine.
    //
    BootloaderMain();

    // Should never get here.
    //
    SpinForever();

}


//------------------------------------------------------------------------------
//
//  Function:  OEMDebugInit
//
//  This function is the first called by the BLCOMMON framework when a boot 
//  loader starts. This function initializes the debug transport, usually just 
//  initializing the debug serial universal asynchronous receiver-transmitter 
//  (UART).
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL OEMDebugInit (void)
{
    OEMInitDebugSerial();
    //OALLogSetZones(1<<0 | 1<<2 | 1<<15 | 1<<3);
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMPlatformInit
//
//  This function initializes the platform and is called by the BLCOMMON 
//  framework.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//------------------------------------------------------------------------------
BOOL OEMPlatformInit (void)
{
    PCSP_CRM_REGS pCRM;
    UINT32 rcsr, pmcr1;

    OEMBootInit ();
#ifdef PRINT_IMAGES_DEFINES
    RETAILMSG(1,(TEXT("IMAGE_BOOT_RAM_PA_START              0x%08x\r\n"),IMAGE_BOOT_RAM_PA_START             ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_IRAM_PA_START            0x%08x\r\n"),IMAGE_WINCE_IRAM_PA_START           ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_IRAM_SIZE                0x%08x\r\n"),IMAGE_WINCE_IRAM_SIZE               ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_DDKSDMA_IRAM_OFFSET      0x%08x\r\n"),IMAGE_WINCE_DDKSDMA_IRAM_OFFSET     ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_DDKSDMA_IRAM_PA_START    0x%08x\r\n"),IMAGE_WINCE_DDKSDMA_IRAM_PA_START   ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_DDKSDMA_IRAM_SIZE        0x%08x\r\n"),IMAGE_WINCE_DDKSDMA_IRAM_SIZE       ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_AUDIO_IRAM_OFFSET        0x%08x\r\n"),IMAGE_WINCE_AUDIO_IRAM_OFFSET       ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_AUDIO_IRAM_PA_START      0x%08x\r\n"),IMAGE_WINCE_AUDIO_IRAM_PA_START     ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_AUDIO_IRAM_SIZE          0x%08x\r\n"),IMAGE_WINCE_AUDIO_IRAM_SIZE         ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_RAMDEV_RAM_PA_START       0x%08x\r\n"),IMAGE_BOOT_RAMDEV_RAM_PA_START      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_RAMDEV_RAM_SIZE           0x%08x\r\n"),IMAGE_BOOT_RAMDEV_RAM_SIZE          ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_RAMDEV_RAM_PA_END         0x%08x\r\n"),IMAGE_BOOT_RAMDEV_RAM_PA_END        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTPT_RAM_OFFSET         0x%08x\r\n"),IMAGE_BOOT_BOOTPT_RAM_OFFSET        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTPT_RAM_PA_START       0x%08x\r\n"),IMAGE_BOOT_BOOTPT_RAM_PA_START      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTPT_RAM_SIZE           0x%08x\r\n"),IMAGE_BOOT_BOOTPT_RAM_SIZE          ));
    RETAILMSG(1,(TEXT("IMAGE_SHARE_ARGS_RAM_OFFSET          0x%08x\r\n"),IMAGE_SHARE_ARGS_RAM_OFFSET         ));
    RETAILMSG(1,(TEXT("IMAGE_SHARE_ARGS_RAM_PA_START        0x%08x\r\n"),IMAGE_SHARE_ARGS_RAM_PA_START       ));
    RETAILMSG(1,(TEXT("IMAGE_SHARE_ARGS_UA_START            0x%08x\r\n"),IMAGE_SHARE_ARGS_UA_START           ));
    RETAILMSG(1,(TEXT("IMAGE_SHARE_ARGS_RAM_SIZE            0x%08x\r\n"),IMAGE_SHARE_ARGS_RAM_SIZE           ));
    RETAILMSG(1,(TEXT("IMAGE_SHARE_FEC_RAM_OFFSET           0x%08x\r\n"),IMAGE_SHARE_FEC_RAM_OFFSET          ));
    RETAILMSG(1,(TEXT("IMAGE_SHARE_FEC_RAM_PA_START         0x%08x\r\n"),IMAGE_SHARE_FEC_RAM_PA_START        ));
    RETAILMSG(1,(TEXT("IMAGE_SHARE_FEC_RAM_SIZE             0x%08x\r\n"),IMAGE_SHARE_FEC_RAM_SIZE            ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_CSPDDK_RAM_OFFSET        0x%08x\r\n"),IMAGE_WINCE_CSPDDK_RAM_OFFSET       ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_CSPDDK_RAM_PA_START      0x%08x\r\n"),IMAGE_WINCE_CSPDDK_RAM_PA_START     ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_CSPDDK_RAM_SIZE          0x%08x\r\n"),IMAGE_WINCE_CSPDDK_RAM_SIZE         ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_DDKSDMA_RAM_PA_START     0x%08x\r\n"),IMAGE_WINCE_DDKSDMA_RAM_PA_START    ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_DDKSDMA_RAM_SIZE         0x%08x\r\n"),IMAGE_WINCE_DDKSDMA_RAM_SIZE        ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_DDKCLK_RAM_PA_START      0x%08x\r\n"),IMAGE_WINCE_DDKCLK_RAM_PA_START     ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_DDKCLK_RAM_UA_START      0x%08x\r\n"),IMAGE_WINCE_DDKCLK_RAM_UA_START     ));
    RETAILMSG(1,(TEXT("IMAGE_WINCE_DDKCLK_RAM_SIZE          0x%08x\r\n"),IMAGE_WINCE_DDKCLK_RAM_SIZE         ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_RAM_OFFSET      0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_RAM_OFFSET     ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_RAM_PA_START    0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_RAM_PA_START   ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_RAM_SIZE        0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_RAM_SIZE       ));
    RETAILMSG(1,(TEXT("IMAGE_USB_KITL_RAM_OFFSET            0x%08x\r\n"),IMAGE_USB_KITL_RAM_OFFSET           ));
    RETAILMSG(1,(TEXT("IMAGE_USB_KITL_RAM_PA_START          0x%08x\r\n"),IMAGE_USB_KITL_RAM_PA_START         ));
    RETAILMSG(1,(TEXT("IMAGE_USB_KITL_RAM_SIZE              0x%08x\r\n"),IMAGE_USB_KITL_RAM_SIZE             ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_NKIMAGE_RAM_OFFSET        0x%08x\r\n"),IMAGE_BOOT_NKIMAGE_RAM_OFFSET       ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_NKIMAGE_RAM_PA_START      0x%08x\r\n"),IMAGE_BOOT_NKIMAGE_RAM_PA_START     ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_NANDDEV_NAND_PA_START     0x%08x\r\n"),IMAGE_BOOT_NANDDEV_NAND_PA_START    ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_NAND_PA_START   0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_NAND_PA_START  ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_NAND_SIZE       0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_NAND_SIZE      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_NAND_PA_END     0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_NAND_PA_END    ));
    
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET     0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET    ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_NAND_PA_START   0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_NAND_PA_START  ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_NAND_SIZE       0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_NAND_SIZE      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_NAND_PA_END     0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_NAND_PA_END    ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_NKIMAGE_NAND_OFFSET       0x%08x\r\n"),IMAGE_BOOT_NKIMAGE_NAND_OFFSET      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_NKIMAGE_NAND_PA_START     0x%08x\r\n"),IMAGE_BOOT_NKIMAGE_NAND_PA_START    ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_NKIMAGE_NAND_SIZE         0x%08x\r\n"),IMAGE_BOOT_NKIMAGE_NAND_SIZE        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_NKIMAGE_NAND_PA_END       0x%08x\r\n"),IMAGE_BOOT_NKIMAGE_NAND_PA_END      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_NAND_OFFSET       0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_NAND_OFFSET      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_NAND_PA_START     0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_NAND_PA_START    ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_NAND_SIZE         0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_NAND_SIZE        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_NAND_PA_END       0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_NAND_PA_END      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_SDHCDEV_SD_PA_START       0x%08x\r\n"),IMAGE_BOOT_SDHCDEV_SD_PA_START      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_SDHCDEV_SD_SIZE           0x%08x\r\n"),IMAGE_BOOT_SDHCDEV_SD_SIZE          ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_SDHCDEV_SD_PA_END         0x%08x\r\n"),IMAGE_BOOT_SDHCDEV_SD_PA_END        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_SD_OFFSET       0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_SD_OFFSET      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_SD_PA_START     0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_SD_PA_START    ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_SD_SIZE         0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_SD_SIZE        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_SD_PA_END       0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_SD_PA_END      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_SD_OFFSET       0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_SD_OFFSET      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_SD_PA_START     0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_SD_PA_START    ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_SD_SIZE         0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_SD_SIZE        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_SD_PA_END       0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_SD_PA_END      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_SD_SIZE           0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_SD_SIZE          ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_SD_OFFSET         0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_SD_OFFSET        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_SD_PA_START       0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_SD_PA_START      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_SD_PA_END         0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_SD_PA_END        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_NKIMAGE_SD_OFFSET         0x%08x\r\n"),IMAGE_BOOT_NKIMAGE_SD_OFFSET        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_NKIMAGE_SD_PA_START       0x%08x\r\n"),IMAGE_BOOT_NKIMAGE_SD_PA_START      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_NKIMAGE_SD_SIZE           0x%08x\r\n"),IMAGE_BOOT_NKIMAGE_SD_SIZE          ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_NKIMAGE_SD_PA_END         0x%08x\r\n"),IMAGE_BOOT_NKIMAGE_SD_PA_END        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_I2CDEV_ROM_PA_START       0x%08x\r\n"),IMAGE_BOOT_I2CDEV_ROM_PA_START      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_I2CDEV_ROM_SIZE           0x%08x\r\n"),IMAGE_BOOT_I2CDEV_ROM_SIZE          ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_I2CDEV_ROM_PA_END         0x%08x\r\n"),IMAGE_BOOT_I2CDEV_ROM_PA_END        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_I2C_ROM_OFFSET  0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_I2C_ROM_OFFSET ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_I2C_ROM_PA_START 0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_I2C_ROM_PA_START));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_I2C_ROM_SIZE    0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_I2C_ROM_SIZE   ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_I2C_ROM_PA_END  0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_I2C_ROM_PA_END ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_SPIDEV_FLASH_PA_START     0x%08x\r\n"),IMAGE_BOOT_SPIDEV_FLASH_PA_START    ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_SPIDEV_FLASH_SIZE         0x%08x\r\n"),IMAGE_BOOT_SPIDEV_FLASH_SIZE        ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_SPIDEV_FLASH_PA_END       0x%08x\r\n"),IMAGE_BOOT_SPIDEV_FLASH_PA_END      ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_SPI_OFFSET      0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_SPI_OFFSET     ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_SPI_PA_START    0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_SPI_PA_START   ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_SPI_SIZE        0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_SPI_SIZE       ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_XLDRIMAGE_SPI_PA_END      0x%08x\r\n"),IMAGE_BOOT_XLDRIMAGE_SPI_PA_END     ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_SPI_OFFSET      0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_SPI_OFFSET     ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_SPI_PA_START    0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_SPI_PA_START   ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_SPI_SIZE        0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_SPI_SIZE       ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTIMAGE_SPI_PA_END      0x%08x\r\n"),IMAGE_BOOT_BOOTIMAGE_SPI_PA_END     ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_SPI_SIZE          0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_SPI_SIZE         ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_SPI_OFFSET        0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_SPI_OFFSET       ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_SPI_PA_START      0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_SPI_PA_START     ));
    RETAILMSG(1,(TEXT("IMAGE_BOOT_BOOTCFG_SPI_PA_END        0x%08x\r\n"),IMAGE_BOOT_BOOTCFG_SPI_PA_END       ));
#endif
    // Get reset status from CRM
    pCRM = (PCSP_CRM_REGS) OALPAtoUA(CSP_BASE_REG_PA_CRM);
    rcsr = INREG32(&pCRM->RCSR);
    pmcr1 = INREG32(&pCRM->PMCR1);


#if 0
    g_bNandBootloader = TRUE;
    KITLOutputDebugString("INFO: FAKING  Bootloader launched from NAND\r\n");
#else

    // Determine boot memory
    switch(CSP_BITFEXT(rcsr, CRM_RCSR_MEM_CTRL))
    {
//no NOR Flash on iMX25 PDK
/*
    case CRM_RCSR_MEM_CTRL_NOR:
        g_bNandBootloader = FALSE;
        KITLOutputDebugString("INFO:  Bootloader launched from NOR\r\n");
        break;
*/

    case CRM_RCSR_MEM_CTRL_NAND:
        g_bNandBootloader = TRUE;
        KITLOutputDebugString("INFO:  Bootloader launched from NAND\r\n");
        break;

    case CRM_RCSR_MEM_CTRL_EXPANSION_DEV:
        switch(CSP_BITFEXT(rcsr, CRM_RCSR_MEM_TYPE))
        {
        case CRM_RCSR_MEM_TYPE_EXPANSION_SD:
            g_bSDHCBootloader = TRUE;
            KITLOutputDebugString("INFO:  Bootloader launched from SDMMC\r\n");
            break;
        case CRM_RCSR_MEM_TYPE_EXPANSION_I2C_ROM:
            //default to boot from NAND since can't store EBOOT/EBOOT Config/NK
            g_bSDHCBootloader = TRUE;
            KITLOutputDebugString("INFO:  XLDR launched from I2C EEPROM. Eboot/Settings/NK are in SD/MMC\r\n");
            break;
        case CRM_RCSR_MEM_TYPE_EXPANSION_SPI_FLASH:
            g_bSpiBootloader = TRUE;
            KITLOutputDebugString("INFO:  Bootloader launched from SPI Flash\r\n");
            break;
        default:
            g_bNandBootloader = TRUE;
            KITLOutputDebugString("INFO: FAKING  Bootloader launched from NAND\r\n");
            break;
        }
        break;
    }
#endif


/* no NOR Flash on i.MX25 PDK
    // Check for image reflash flag from RVD
    // no RCSR GPF bit on MX25, so use PMCR1 DVGP
    if (CSP_BITFEXT(pmcr1, CRM_PMCR1_DVGP) == 0xF)
    {
        KITLOutputDebugString("Reflash request detected!\r\n");

        // Write out the image previously downloaded into SDRAM
        // by RVD
        OEMWriteFlash((DWORD) OALPAtoCA(IMAGE_BOOT_NORDEV_NOR_PA_START), IMAGE_BOOT_NORDEV_NOR_SIZE);

        // EBOOT download is unnecessary since RVD downloaded image via JTAG
        g_DownloadImage = FALSE;
            
        // Jump to OS image
        OEMLaunch(0, 0, (DWORD) OALPAtoCA(IMAGE_BOOT_NKIMAGE_NOR_PA_START), NULL);
    }
*/

    // Initialize the BSP args structure.
    //
    g_pBSPArgs = (BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START;
    if ((g_pBSPArgs->header.signature != OAL_ARGS_SIGNATURE) || 
        (g_pBSPArgs->header.oalVersion != OAL_ARGS_VERSION) || 
        (g_pBSPArgs->header.bspVersion != BSP_ARGS_VERSION))
    {
        memset((LPVOID)g_pBSPArgs, 0, sizeof(BSP_ARGS));
        g_pBSPArgs->header.signature  = OAL_ARGS_SIGNATURE;
        g_pBSPArgs->header.oalVersion = OAL_ARGS_VERSION;
        g_pBSPArgs->header.bspVersion = BSP_ARGS_VERSION;
        g_pBSPArgs->kitl.flags             =  OAL_KITL_FLAGS_VMINI;
        g_pBSPArgs->kitl.devLoc.IfcType    = Internal;
        g_pBSPArgs->kitl.devLoc.BusNumber  = 0;
        g_pBSPArgs->updateMode = FALSE;
    }

    // Initialize clockFreq for reference in bootloader
    OALBspArgsInit(g_pBSPArgs);

    // Initialize timer for stall function
    BootTimerInit();

	//
	// CS&ZHL MAY-31-2011: This CPLD is for iMX25PDK only
	//
#ifndef	EM9170
    // Initialize CPLD access
    CPLDInit();
#endif	//EM9170

    // Attempt to initialize the NAND flash driver
    if (!FMD_Init(NULL, NULL, NULL))
    {
        KITLOutputDebugString("WARNING: OEMPlatformInit: Failed to initialize NAND flash device.\r\n");
        g_bNandExist = FALSE;
    }
    else
    {
        NANDBootReserved();
        KITLOutputDebugString("INFO: OEMPlatformInit: Initialized NAND flash device.\r\n");
        g_bNandExist = TRUE;
    }

//#if 0
	//
	// CS&ZHL MAY-31-2011: This SDHC and SPIFMD are for iMX25PDK only
	//
#ifdef	EM9170
	EM9170_IOConfig();
	EM9170_ISABusSetup();
    g_bSDHCExist = FALSE;
    g_bSpiExist = FALSE;
#else
    // Attempt to initialize the SD/MMC driver
    if (!SDMMC_Init ())
    {
        KITLOutputDebugString("WARNING: OEMPlatformInit: Failed to initialize SDHC device.\r\n");
        g_bSDHCExist = FALSE;
    }
    else
    {
        g_bSDHCExist = TRUE;
    }

    // Attempt to initialize the SPI Flash driver
    if (!SPIFMD_Init ())
    {
        KITLOutputDebugString("WARNING: OEMPlatformInit: Failed to initialize SPI device.\r\n");
        g_bSpiExist = FALSE;
    }
    else
    {
        g_bSpiExist = TRUE;
    }
#endif	//EM9170
    
    // Load eboot configuration
    //
    if (!LoadBootCFG(&g_BootCFG)) 
    {

        // Load default bootloader configuration settings.
        KITLOutputDebugString("ERROR: could not find the boot settings - using bootloader defaults...\r\n");
        ResetDefaultBootCFG(&g_BootCFG);
    }

    //Config EbootCFG
    ConfigBootCFG(&g_BootCFG);

    // Set up optional bootloader function pointers.
    //
    g_pOEMMultiBINNotify = OEMMultiBINNotify;
    g_pOEMVerifyMemory = OEMVerifyMemory;
    g_pOEMReportError = OEMReportError;

	//
	// CS&ZHL MAY-4-2011: add Splash Screen here
	//
	Eboot_SplashScreen();

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMPreDownload
//
//  This function is called by the BLCOMMON framework prior to download and can
//  be customized to prompt for user feedback, such as obtaining a static IP 
//  address or skipping the download and jumping to a flash-resident run-time 
//  image.
//
//  Parameters:        
//      None.
//
//  Returns:
//      Possible return values for OEMPreDownload:
//      
//      Value               Description 
//      -----               -----------
//      BL_DOWNLOAD = 0     Download the OS image from the host machine. 
//      BL_JUMP = 1         Skip the download and jump to a resident OS image. 
//      BL_ERROR = -1       Image download is unsuccessful. 
//------------------------------------------------------------------------------
DWORD OEMPreDownload(void)
{
    UINT32 rc = (DWORD)BL_ERROR;
    BOOL  fGotJumpImg = FALSE;
    const char * platformString = "IMX25";

    // User menu code...
    //
    if (!BLMenu())
    {
        return rc;
    }

    // Create device name based on Ethernet address (this is how Platform Builder identifies this device).
    //
    OALKitlCreateName(BSP_DEVICE_PREFIX, g_pBSPArgs->kitl.mac, (CHAR *)g_pBSPArgs->deviceId);
    KITLOutputDebugString("INFO: Using device name: '%s'\n", g_pBSPArgs->deviceId);

    if( g_SerialUSBDownload && g_DownloadImage )
    {
        // Send boot requests indefinitely
        do
        {
            KITLOutputDebugString("Sending boot request...\r\n");
            if(!SerialSendBootRequest(platformString))
            {
                KITLOutputDebugString("Failed to send boot request\r\n");
                return (DWORD)BL_ERROR;
            }
        }
        while(!SerialWaitForBootAck(&fGotJumpImg));
        
        // Ack block zero to start the download
        SerialSendBlockAck(0);

        if( fGotJumpImg )
        {
            KITLOutputDebugString("Received boot request ack... jumping to image\r\n");
        }
        else
        {
            KITLOutputDebugString("Received boot request ack... starting download\r\n");
        }
        
    }
	else
    {
        fGotJumpImg = GetPreDownloadInfo (&g_BootCFG);
    }
     
    if (!g_DownloadImage || // this gets set in the BLMenu() function
        fGotJumpImg)        // this gets set in EbootInitEtherTransport
    {
        switch(g_BootCFG.autoDownloadImage)
        {
//no NOR Flash on iMX25 PDK
/*
        case BOOT_CFG_AUTODOWNLOAD_NK_NOR:
            rc = BL_JUMP;
            break;
*/

        case BOOT_CFG_AUTODOWNLOAD_NK_NAND:
            //if (NANDLoadNK())
			//
			// CS&ZHL MAY-23-2011: g_BootCFG.dwNandImageStart => virtual start address of OS image in NandFlash
			//                                      g_BootCFG.dwNandImageLength => byte length of OS image in NandFlash
			//
            if (NANDLoadNK(g_BootCFG.dwNandImageStart, g_BootCFG.dwNandImageLength))
            {
                rc = BL_JUMP;
            }
            else
            {
                KITLOutputDebugString("ERROR: Failed to load OS image from NAND.\r\n");
            }
            break;
            
        case BOOT_CFG_AUTODOWNLOAD_IPL_NAND:
            if (NANDLoadIPL())
            {
                rc = BL_JUMP;
            }
            else
            {
                KITLOutputDebugString("ERROR: Failed to load IPL image from NAND.\r\n");
            }
            break;

        case BOOT_CFG_AUTODOWNLOAD_NK_SD:
            if (SDHCLoadNK())
            {
                rc = BL_JUMP;
            }
            else
            {
                KITLOutputDebugString("ERROR: Failed to load OS image from SD/MMC.\r\n");
            }
            break;
        }

        // Set the clean boot flag so that OAL will let the kernel know that
        // it needs a clean boot
        //
        // g_pBSPArgs->bCleanBootFlag = TRUE;
    }
    else if (g_DownloadImage)
    {
        rc = BL_DOWNLOAD;
    }

    return(rc);
}


//------------------------------------------------------------------------------
//
//  Function:  OEMLaunch
//
//  This function launches the run-time image. It is the last one called by 
//  the BLCOMMON framework.
//
//  Parameters:        
//      dwImageStart 
//          [in] Starting address of OS image. 
//
//      dwImageLength 
//          [in] Length, in bytes, of the OS image. 
//
//      dwLaunchAddr 
//          [in] First instruction of the OS image.
//
//      pRomHdr 
//          [out] Pointer to the ROM header structure.
//
//  Returns:
//      None.
//------------------------------------------------------------------------------
void OEMLaunch (DWORD dwImageStart, DWORD dwImageLength, DWORD dwLaunchAddr, const ROMHDR *pRomHdr)
{
    UINT32 PhysAddress;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pRomHdr);

    switch(g_BootCFG.autoDownloadImage)
    {
//no NOR Flash on iMX25 PDK
/*
    case BOOT_CFG_AUTODOWNLOAD_NK_NOR:
        // Set launch address for NOR OS image.  OS executes-in-place from NOR.
        PhysAddress = IMAGE_BOOT_NKIMAGE_NOR_PA_START;
        break;
*/
    case BOOT_CFG_AUTODOWNLOAD_NK_NAND:
    case BOOT_CFG_AUTODOWNLOAD_NK_SD:
        // Set launch address for NAND/SDHC OS image.  OS is copied into RAM for execution.
        //PhysAddress = IMAGE_BOOT_NKIMAGE_RAM_PA_START;
		//KITLOutputDebugString("OEMLaunch::use constant physical address (0x%x) as LaunchAddr\r\n", PhysAddress);
        dwImageStart =g_BootCFG.dwNandImageStart;
        dwLaunchAddr = g_BootCFG.dwLaunchAddr;					// => IMAGE_BOOT_NKIMAGE_RAM_PA_START
        dwImageLength = g_BootCFG.dwNandImageLength;
		KITLOutputDebugString("OEMLaunch::use BootCFG -> dwImageStart = 0x%x, dwLaunchAddr = 0x%x, dwImageLength = 0x%x\r\n", 
												dwImageStart, dwLaunchAddr, dwImageLength);
		PhysAddress = (UINT32) OALVAtoPA((VOID *)dwLaunchAddr);
        break;
/*
    case BOOT_CFG_AUTODOWNLOAD_IPL_NAND:
        // Set launch address for NAND IPL image.  IPL is copied into RAM for execution.
        PhysAddress = IMAGE_BOOT_IPLIMAGE_RAM_START;
        break;
*/

    default:
		KITLOutputDebugString("OEMLaunch::use input virtual address (0x%x) as LaunchAddr\r\n", dwLaunchAddr);
        // If a launch address wasn't specified - use the last known good address.
        //
        if (!dwLaunchAddr)
        {
            dwLaunchAddr = g_BootCFG.dwLaunchAddr;
        }
        else
        {
            if (g_BootCFG.dwLaunchAddr != dwLaunchAddr ||
                g_BootCFG.dwPhysStart  != dwImageStart ||
                g_BootCFG.dwPhysLen    != dwImageLength)
            {
                g_BootCFG.dwLaunchAddr = dwLaunchAddr;
                g_BootCFG.dwPhysStart  = dwImageStart;
                g_BootCFG.dwPhysLen    = dwImageLength;

                StoreBootCFG(&g_BootCFG);
            }
        }

        // Translate the image start address (virtual address) to a physical address.
        //
        PhysAddress = (UINT32) OALVAtoPA((VOID *)dwLaunchAddr);
        break;
    }

    // disable boot partition in case there is eMMC or eSD in the system, so that OS file system does not overwrite boot partition
    if (g_bSDHCExist)
        SDHCDisableBP();
    
    KITLOutputDebugString("Download successful!  Jumping to image at 0x%x (physical 0x%x)...\r\n", dwLaunchAddr, PhysAddress);

    // Wait for PB connection...
    //
    if (g_DownloadImage)
    {
        if( g_SerialUSBDownload )
        {
            SerialWaitForJump();

        }else
        {
            GetLaunchInfo ();
        }
    }

    // Jump to the image we just downloaded.
    //
    KITLOutputDebugString("\r\n\r\n\r\n");
    OEMWriteDebugLED(0, 0x00FF);
    Launch(PhysAddress);

    // Should never get here...
    //
    SpinForever();

}


//------------------------------------------------------------------------------
//
//  Function:  OEMMultiBINNotify
//
//  Receives/processes download file manifest.
//
//  Parameters:
//      pInfo 
//          [in] Download manifiest provided by BLCOMMON framework.
//
//  Returns:
//             void
//------------------------------------------------------------------------------
void OEMMultiBINNotify(const PMultiBINInfo pInfo)
{
    CHAR szFileName[MAX_PATH];
    int i, key, fileExtPos;

    if (!pInfo) return;

    memset(szFileName, 0, sizeof(szFileName));

    KITLOutputDebugString("INFO: OEMMultiBINNotify (dwNumRegions = %d, dwRegionStart = 0x%x).\r\n", 
        pInfo->dwNumRegions, pInfo->Region[0].dwRegionStart);

	// CS&ZHL MAY-21-2011: add for BinFS
	g_dwTotalBinNum = pInfo->dwNumRegions;
	g_dwCurrentBinNum = 0;
    
    // Only inspect manifest if this is a monolithic .nb0 or diskimage file
    if (pInfo->dwNumRegions != 1) 
        return;

    //  NBO and DIO files will have start address of zero
    if (pInfo->Region[0].dwRegionStart == 0)
    {

        // Convert the file name to lower case
        i = 0;
        fileExtPos = 0;
        while ((pInfo->Region[0].szFileName[i] != '\0') && (i < MAX_PATH))
        {
            szFileName[i] = (CHAR)tolower(pInfo->Region[0].szFileName[i]);
            // Keep track of file extension position
            if (szFileName[i] == '.')
            {
                fileExtPos = i;
            }
            i++;
        }
        // Copy string terminator as well
        szFileName[i] = pInfo->Region[0].szFileName[i];
        
        // Check for NAND XLDR update
        if ((strncmp(szFileName, XLDR_NB0_FILE, XLDR_NB0_FILE_LEN) == 0))
        {
            // Remap the start address to the region specified by the user
            KITLOutputDebugString("Specify destination for XLDR NB0 [1 = NAND, 2 = SD/MMC 3 = SPI 4 = I2C]: ");
            do {
                key = OEMReadDebugByte();
            } while ((key != '1') && (key != '2') && (key != '3') && (key != '4'));
            KITLOutputDebugString("\r\n");
            switch (key)
            {
            case '1':
            // Remap the start address to the NAND XLDR region
            pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_XLDRIMAGE_NAND_PA_START);
                break;
            case '2':
             // Remap the start address to the SD XLDR region
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_XLDRIMAGE_SD_PA_START);
                break;
            case '3':
             // Remap the start address to the SPI XLDR region
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_XLDRIMAGE_SPI_PA_START);
                break;
            case '4':
                // Remap the start address to the I2C XLDR region
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_XLDRIMAGE_I2C_ROM_PA_START);
                break;
            default:
                return;
            }

            KITLOutputDebugString("INFO: XLDR NB0 remapped to 0x%x.\r\n", pInfo->Region[0].dwRegionStart);
        }

        // Check for EBOOT/SBOOT update
        else if  ( (strncmp(szFileName, EBOOT_NB0_FILE, EBOOT_NB0_FILE_LEN) == 0) ||
           (strncmp(szFileName, SBOOT_NB0_FILE, SBOOT_NB0_FILE_LEN) == 0) )
        {
            // Remap the start address to the region specified by the user
            KITLOutputDebugString("Specify destination for EBOOT/SBOOT NB0 [1 = NAND, 2 = SD/MMC, 3 = SPI]: ");
            do {
                key = OEMReadDebugByte();
            } while ((key != '1') && (key != '2') && (key != '3'));
            KITLOutputDebugString("\r\n");
            switch (key)
            {
            case '1':
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_BOOTIMAGE_NAND_PA_START);
                break;
            case '2':
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_BOOTIMAGE_SD_PA_START);
                break;
            case '3':
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_BOOTIMAGE_SPI_PA_START);
                break;
            default:
                return;
            }

            KITLOutputDebugString("\r\nINFO: EBOOT/SBOOT NB0 remapped to 0x%x.\r\n", pInfo->Region[0].dwRegionStart);
        }
        
        // If  file to be downloaded has an NB0 extension, assume it is an NK NB0 image
        else if (fileExtPos && (strncmp(&szFileName[fileExtPos], ".nb0", 4) == 0))
        {
            // Remap the start address to the region specified by the user
            KITLOutputDebugString("Specify destination for NK NB0 [1 = RAM, 2 = NAND, 3 = SD/MMC]: ");
            do {
                key = OEMReadDebugByte();
            } while ((key != '1') && (key != '2') && (key != '3'));
            KITLOutputDebugString("\r\n");
            switch (key)
            {
            case '1':
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_NKIMAGE_RAM_PA_START);
                break;

            case '2':
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_NKIMAGE_NAND_PA_START);
                break;

            case '3':
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_NKIMAGE_SD_PA_START);
                break;

            default:
                return;
            }

            KITLOutputDebugString("\r\nINFO: NK NB0 remapped to 0x%x.\r\n", pInfo->Region[0].dwRegionStart);
        }

        // Notify user of unsupported format
        else 
        {
            KITLOutputDebugString("\r\nWARNING: Unsupported binary format.  Image not remapped.\r\n");
        }
    }

    // If this is a Windows Mobile disk image BIN, then dwRegionStart will
    // be offset into NAND.
    else if (pInfo->Region[0].dwRegionStart < (DWORD) OALPAtoCA(IMAGE_BOOT_RAMDEV_RAM_PA_START))
    {
        g_ImageType = IMAGE_TYPE_BINDIO;
        g_ImageMemory = IMAGE_MEMORY_NAND;
        KITLOutputDebugString("\r\nINFO: DIO image with starting address 0x%x.\r\n", pInfo->Region[0].dwRegionStart);
    }
    
    return;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMReportError
//
//  Error reporting function provided by BLCOMMON framework.
//
//  Parameters:
//      dwReason 
//          [in] Reason for error
//
//      dwReserved 
//          [in] Reserved parameter
//
//  Returns:
//             void
//------------------------------------------------------------------------------
BOOL OEMReportError (DWORD dwReason, DWORD dwReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwReserved);

    KITLOutputDebugString("INFO: OEMReportError Reason 0x%x\r\n", dwReason);

    return TRUE;    
}

//-----------------------------------------------------------------------------
//
//  Function:  OEMVerifyMemory
//
//  This function verifies that the address provided is in valid memory,
//  and sets globals to describe the image region being updated.
//
//  Parameters:
//      dwStartAddr 
//          [in] Address to be verified. 
//
//      dwLength 
//          [in] Length of the address, in bytes.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL OEMVerifyMemory(DWORD dwStartAddr, DWORD dwLength)
{
    BOOL  rc = TRUE;
    DWORD dwPhysVerifyStart; 
    DWORD dwPhysVerifyEnd;

    // First check for DIO image flagged by OEMMultiBINNotify
    if (g_ImageType == IMAGE_TYPE_BINDIO)
    {
        KITLOutputDebugString("INFO: Downloading DIO NAND image.\r\n");
        return (TRUE);
    }

    dwPhysVerifyStart = (DWORD) OALVAtoPA((void *)dwStartAddr); 
    dwPhysVerifyEnd   = dwPhysVerifyStart + dwLength - 1;

    KITLOutputDebugString("INFO: OEMVerifyMemory (CA = 0x%x, PA = 0x%x, length = 0x%x)\r\n", 
        dwStartAddr, dwPhysVerifyStart, dwLength);

    if ((dwPhysVerifyStart >= IMAGE_BOOT_XLDRIMAGE_NAND_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_XLDRIMAGE_NAND_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading XLDR NAND image.\r\n");
        g_ImageType = IMAGE_TYPE_XLDR;
        g_ImageMemory = IMAGE_MEMORY_NAND;
    }
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_BOOTIMAGE_NAND_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_BOOTIMAGE_NAND_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading EBOOT/SBOOT NAND image.\r\n");
        g_ImageType = IMAGE_TYPE_BOOT;
        g_ImageMemory = IMAGE_MEMORY_NAND;
    }
/*
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_IPLIMAGE_NAND_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_IPLIMAGE_NAND_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading IPL NAND image.\r\n");
        g_ImageType = IMAGE_TYPE_IPL;
        g_ImageMemory = IMAGE_MEMORY_NAND;
    }
*/
    // DIO and NK share this start address
    //else if ((dwPhysVerifyStart >= IMAGE_BOOT_NKIMAGE_NAND_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_NKIMAGE_NAND_PA_END))
	//
	// CS&ZHL MAY-21-2011: Adjusts the BinFS NK address between...
	//                                      IMAGE_BOOT_MBR_NAND_PA_START = 0xBB200000
	//                                      IMAGE_BOOT_NANDDEV_NAND_PA_END = 0xBD800000
	else if ((dwPhysVerifyStart >= IMAGE_BOOT_MBR_NAND_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_NANDDEV_NAND_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading NK NAND image.\r\n");
        g_ImageType = IMAGE_TYPE_NK;
        g_ImageMemory = IMAGE_MEMORY_NAND;
    }
//no NOR Flash on iMX25 PDK
/*
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_BOOTIMAGE_NOR_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_BOOTIMAGE_NOR_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading EBOOT/SBOOT NOR image.\r\n");
        g_ImageType = IMAGE_TYPE_BOOT;
        g_ImageMemory = IMAGE_MEMORY_NOR;
    }
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_NKIMAGE_NOR_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_NKIMAGE_NOR_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading NK NOR image.\r\n");
        g_ImageType = IMAGE_TYPE_NK;
        g_ImageMemory = IMAGE_MEMORY_NOR;
    }
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_NORDEV_NOR_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_NORDEV_NOR_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading NK NOR image.\r\n");
        KITLOutputDebugString("WARNING:  NK image will overwrite EBOOT reserved space \r\n");
        g_ImageType = IMAGE_TYPE_NK;
        g_ImageMemory = IMAGE_MEMORY_NOR;
    }
*/
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_RAMDEV_RAM_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_RAMDEV_RAM_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading NK RAM image.\r\n");
        g_ImageType = IMAGE_TYPE_NK;
        g_ImageMemory = IMAGE_MEMORY_RAM;
    }
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_XLDRIMAGE_SD_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_XLDRIMAGE_SD_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading XLDR SD/MMC image.\r\n");
        g_ImageType = IMAGE_TYPE_XLDR;
        g_ImageMemory = IMAGE_MEMORY_SD;
    }
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_BOOTIMAGE_SD_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_BOOTIMAGE_SD_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading EBOOT/SBOOT for SD/MMC image.\r\n");
        g_ImageType = IMAGE_TYPE_BOOT;
        g_ImageMemory = IMAGE_MEMORY_SD;
    }
    // DIO and NK share this start address
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_NKIMAGE_SD_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_NKIMAGE_SD_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading NK SD/MMC image.\r\n");
        g_ImageType = IMAGE_TYPE_NK;
        g_ImageMemory = IMAGE_MEMORY_SD;
    }
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_XLDRIMAGE_I2C_ROM_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_XLDRIMAGE_I2C_ROM_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading XLDR I2C EEPROM image.\r\n");
        g_ImageType = IMAGE_TYPE_XLDR;
        g_ImageMemory = IMAGE_MEMORY_I2C;
    }
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_XLDRIMAGE_SPI_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_XLDRIMAGE_SPI_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading XLDR SPI image.\r\n");
        g_ImageType = IMAGE_TYPE_XLDR;
        g_ImageMemory = IMAGE_MEMORY_SPI;
    }
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_BOOTIMAGE_SPI_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_BOOTIMAGE_SPI_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading EBOOT/SBOOT for SPI image.\r\n");
        g_ImageType = IMAGE_TYPE_BOOT;
        g_ImageMemory = IMAGE_MEMORY_SPI;
    }
    else
    {
        KITLOutputDebugString("ERROR: Invalid address range.\r\n");
        rc = FALSE;
    }

	// CS&ZHL MAY-21-2011: add for BinFS
	if(g_ImageMemory == IMAGE_MEMORY_NAND)
	{
		if(!NANDCheckImageAddress(dwPhysVerifyStart))
		{
			KITLOutputDebugString("ERROR: Image not aligned in NAND block size.\r\n");
			return FALSE;
		}
	}

	g_dwBinAddress[g_dwCurrentBinNum] = dwPhysVerifyStart;
	g_dwBinLength[g_dwCurrentBinNum] = dwLength;
	g_dwCurrentBinNum++;

	// Check if it is the last bin
	if(g_dwCurrentBinNum == g_dwTotalBinNum)
	{
		// For OEMWriteFlash() using
		g_dwCurrentBinNum = 0;
	}
	// CS&ZHL MAY-21-2011: end of add for BinFS

    return(rc);

}


//------------------------------------------------------------------------------
//
//  Function:  StoreBootCFG
//
//  Stores bootloader configuration information (menu settings, etc.) in flash.
//
//  Parameters:
//      BootCfg 
//          [in] Points to bootloader configuration to be stored. 
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL StoreBootCFG(BOOT_CFG *BootCfg)
{

    if (!FlashStoreBootCFG((BYTE*) BootCfg, sizeof(BOOT_CFG)))
    {
        KITLOutputDebugString("ERROR: StoreBootCFG: failed to write configuration.\r\n");
        return(FALSE);
    }

    return(TRUE);
}


//------------------------------------------------------------------------------
//
//  Function:  LoadBootCFG
//
//  Retrieves bootloader configuration information (menu settings, etc.) from 
//  flash.
//
//  Parameters:
//      BootCfg 
//          [out] Points to bootloader configuration that will be filled with
//          loaded data. 
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL LoadBootCFG(BOOT_CFG *BootCfg)
{
    if (!FlashLoadBootCFG((BYTE*) BootCfg, sizeof(BOOT_CFG))) {
        KITLOutputDebugString("ERROR: LoadBootCFG: failed to load configuration.\r\n");
        return(FALSE);
    }

    // Is the CFG data valid?  Check for the magic number that was written the last time
    // the CFG block was updated.  If Eboot has never been run, there will be no configuration
    // information, so the magic number will likely not be found.  In this case, setup the 
    // factory defaults and store them into Flash.
    //
    if (BootCfg->ConfigMagicNumber != OEMGetMagicNumber())
    {
        KITLOutputDebugString("ERROR: LoadBootCFG: ConfigMagicNumber not correct. Expected = 0x%x ; Actual = 0x%x.\r\n", 
            OEMGetMagicNumber(), BootCfg->ConfigMagicNumber);
        ResetDefaultBootCFG(BootCfg);
    }

    g_DefaultRamAddress = (PUCHAR)BootCfg->dwLaunchAddr;

    return(TRUE);
}


//------------------------------------------------------------------------------
//
//  Function:  ConfigBootCFG
//
//  Update PCB ram and config system according eboot config
//
//  Parameters:
//      eBootCFG 
//          [in] Points to bootloader configuration
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void ConfigBootCFG(BOOT_CFG *pBootCFG)
{
    if(pBootCFG->dwConfigFlags & CONFIG_FLAGS_KITL_ENABLE)
    {
        g_pBSPArgs->kitl.flags  |= OAL_KITL_FLAGS_ENABLED;        
    } 
    else 
    {
        g_pBSPArgs->kitl.flags  &= ~OAL_KITL_FLAGS_ENABLED;        
    }

    if(pBootCFG->dwConfigFlags & CONFIG_FLAGS_KITL_PASSIVE)
    {
        g_pBSPArgs->kitl.flags  |= OAL_KITL_FLAGS_PASSIVE;        
    } 
    else 
    {
        g_pBSPArgs->kitl.flags  &= ~OAL_KITL_FLAGS_PASSIVE;        
    }

    // update bsp args kitl flag
    if(pBootCFG->dwConfigFlags & CONFIG_FLAGS_KITL_INT)
    {
        g_pBSPArgs->kitl.flags  &= ~OAL_KITL_FLAGS_POLL;        
    }
    else
    {
        g_pBSPArgs->kitl.flags  |= OAL_KITL_FLAGS_POLL;        
    }

}

