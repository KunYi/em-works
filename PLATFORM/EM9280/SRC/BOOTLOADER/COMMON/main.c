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
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
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
#include "dbgserial.h"
#include "sdfmd.h"


//-----------------------------------------------------------------------------
// External Functions
extern BOOL NANDLoadIPL(VOID);
extern BOOL NANDLoadNK(VOID);
//extern BOOL NANDLoadNK(DWORD dwImageStart, DWORD dwImageLength);		// CS&ZHL MAY-23-2011: use BootCFG parameter
extern BOOL NANDCheckImageAddress(DWORD dwPhyAddr);						// CS&ZHL MAY-21-2011: add for BinFS
extern BOOL SDHCLoadNK(VOID);
extern VOID SDHCDisableBP(VOID);
extern BOOL FlashLoadBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize);
extern BOOL FlashStoreBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize);
extern void Launch(unsigned int uAddr);
extern BOOL BLMenu();
extern UINT32 OEMGetMagicNumber();
extern BOOL CPLDInit(void);
extern BOOL ApbhDmaInit(void);
extern BOOL ApbhDmaDeInit(void);
extern BOOL ApbhDmaAlloc(void);
extern BOOL ApbhDmaDealloc(void);

extern BOOL IomuxAlloc(void);
extern BOOL IomuxDealloc(void);

extern BOOL ClockAlloc(void);

extern BOOL  SerialSendBlockAck(DWORD uBlockNumber);
extern BOOL  SerialSendBootRequest(const char * platformString);
extern BOOL  SerialWaitForBootAck(BOOL *pfJump);
extern DWORD SerialWaitForJump(VOID);

// SDMMC Downloading
extern BOOL  SDMMCDownload(PCSTR pFileName);
extern void  NANDBootReserved();
extern BOOL NFC_SetClock(BOOL bEnabled);

//-----------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregPINCTRL;
extern PVOID pv_HWregCLKCTRL;

extern BOOT_CFG g_BootCFG;
extern IMAGE_TYPE ImageTypeBIN;


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
BOOL g_DownloadbyUSB = FALSE;
UCHAR *g_DefaultRamAddress;
BOOL g_bNandBootloader;
BOOL g_bNandExist;
BOOL g_bSDHCBootloader;
BOOL g_bSDHCExist;
BOOL g_bSPIBootloader;
DWORD g_dwBoardID;
PVOID pv_HWregRTC = NULL;
// Used to save information about downloaded DIO mage
BOOT_BINDIO_CONTEXT g_BinDIO;

//
// CS&ZHL MAY-19-2011: variables for BinFS
//
DWORD g_dwTotalBinNum;			// CS&ZHL MAY-21-2011: get from OEMMultiBINNotify()
DWORD g_dwCurrentBinNum;
DWORD g_dwBinAddress[16];		// CS&ZHL MAY-21-2011: Flash Physical Addresses
DWORD g_dwBinLength[16];		// Now we support 16 bin files for MultiBIN

//-----------------------------------------------------------------------------
// Local Variables
BOOLEAN g_SerialUSBDownload = FALSE;
BOOLEAN g_StorageSDDownload = FALSE;


//------------------------------------------------------------------------------
// Local Functions
//
BOOL LoadBootCFG(BOOT_CFG *BootCFG);
BOOL StoreBootCFG(BOOT_CFG *BootCFG);
void ConfigBootCFG(BOOT_CFG *pBootCFG);
void ResetDefaultBootCFG(BOOT_CFG *pBootCFG);
BOOL OEMVerifyMemory(DWORD dwStartAddr, DWORD dwLength);
BOOL OEMReportError (DWORD dwReason, DWORD dwReserved);
void OEMMultiBINNotify(const PMultiBINInfo pInfo);
BOOL CPUClkInit(VOID);
UINT32 MMC_Send_Switch_Cmd(UINT32 switch_arg);

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
    // Common boot loader (blcommon) main routine.
    BootloaderMain();

    // Should never get here.
    SpinForever();
}

//------------------------------------------------------------------------------
// CS&ZHL MAY-18-2012: move out from esdhc.c
//------------------------------------------------------------------------------
void SetupGPIOClock()
{
    // Clear SFTRST
    HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_SFTRST);
    while(HW_PINCTRL_CTRL_RD() & BM_PINCTRL_CTRL_SFTRST);

    // Clear CLKGATE
    HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_CLKGATE);
    while(HW_PINCTRL_CTRL_RD() & BM_PINCTRL_CTRL_CLKGATE);
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
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  CPUClkInit
//
//  Programs the CPU clock and HBus clock.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL CPUClkInit(VOID)
{
    PVOID  pv_HWregCLKCTRL = (PVOID)OALPAtoUA(CSP_BASE_REG_PA_CLKCTRL);
    PVOID  pv_HWregPOWER = (PVOID)OALPAtoUA(CSP_BASE_REG_PA_POWER);
    UINT32 dwRegFrac;
    UINT32 x;

    // let CPU sink the xtal clock
    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);

    // Turn on PLL
    HW_CLKCTRL_PLL0CTRL0_SET(BM_CLKCTRL_PLL0CTRL0_POWER);

    //Set VDDD to 1.45V
    HW_POWER_VDDDCTRL_WR(HW_POWER_VDDDCTRL_RD() | BM_POWER_VDDDCTRL_BO_OFFSET);
    HW_POWER_VDDDCTRL_WR((HW_POWER_VDDDCTRL_RD() & ~BM_POWER_VDDDCTRL_TRG) | 26);

    // need to wait more than 10 microsecond before the DC_OK is valid
    OALStall(15);

    // wait for DC_OK
    while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));

    // set ref.cpu 454MHZ
    HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATECPU);

    HW_CLKCTRL_FRAC0_WR((HW_CLKCTRL_FRAC0_RD() & ~BM_CLKCTRL_FRAC0_CPUFRAC) | \
                        BF_CLKCTRL_FRAC0_CPUFRAC(19));

    dwRegFrac = HW_CLKCTRL_FRAC0_RD() & BM_CLKCTRL_FRAC0_CPU_STABLE;
    for(; (HW_CLKCTRL_FRAC0_RD() ^ dwRegFrac) == 0; ) ;

    // config CLK_CPU driver for 454/1 MHz (fractional divides it down to 454MHz).
    HW_CLKCTRL_CPU_WR((BF_CLKCTRL_CPU_DIV_CPU(1)             | \
                       BF_CLKCTRL_CPU_DIV_CPU_FRAC_EN(0)     | \
                       BF_CLKCTRL_CPU_INTERRUPT_WAIT(1)      | \
                       BF_CLKCTRL_CPU_DIV_XTAL(1)            | \
                       BF_CLKCTRL_CPU_DIV_XTAL_FRAC_EN(0)));

    // config CLK_HBUS as CPU/3 (151MHz)
    HW_CLKCTRL_HBUS_WR((BF_CLKCTRL_HBUS_DIV(3)                    | \
                        BF_CLKCTRL_HBUS_DIV_FRAC_EN(0)            | \
                        BF_CLKCTRL_HBUS_SLOW_DIV(0)               | \
                        BF_CLKCTRL_HBUS_ASM_ENABLE(0)             | \
                        BF_CLKCTRL_HBUS_CPU_INSTR_AS_ENABLE(0)    | \
                        BF_CLKCTRL_HBUS_CPU_DATA_AS_ENABLE(0)     | \
                        BF_CLKCTRL_HBUS_TRAFFIC_AS_ENABLE(0)      | \
                        BF_CLKCTRL_HBUS_TRAFFIC_JAM_AS_ENABLE(0)  | \
                        BF_CLKCTRL_HBUS_APBXDMA_AS_ENABLE(0)      | \
                        BF_CLKCTRL_HBUS_APBHDMA_AS_ENABLE(0)));

    HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);

    for(x=0; x++ != 0x1000; ) ;
    return TRUE;
}

//------------------------------------------------------------------------------
// CS&ZHL MAY-29-2012: get debug state
//------------------------------------------------------------------------------
BOOL GetDebugMode(void)
{
#ifdef	EM9280
	{
		DWORD	i;
		UINT32	uData;

		// switch DUART_TXD pin to GPIO input mode
		DDKIomuxSetPinMux(DDK_IOMUX_DUART_TX_1, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_17, 0);	// output disable -> set as input
		// Set pin drive to 8mA,enable pull up,3.3V
		DDKIomuxSetPadConfig(DDK_IOMUX_GPIO3_17, 
			DDK_IOMUX_PAD_DRIVE_8MA, 
			DDK_IOMUX_PAD_PULL_ENABLE,
			DDK_IOMUX_PAD_VOLTAGE_3V3);

		// delay 10ms
	    OALStall(10000);

		// read state of DBGSLn
		for(i = 0; i < 10; i++)
		{
			DDKGpioReadDataPin(DDK_IOMUX_GPIO3_17, &uData);
			if(uData)
			{
				break;
			}
			// delay 100us
			OALStall(100);
		}

		// switch DUART_TXD pin back to DUART_TXD mode
		DDKIomuxSetPinMux(DDK_IOMUX_DUART_TX_1, DDK_IOMUX_MODE_02);

		// final judge
		if(i >= 10)
		{
			return TRUE;
		}

		return FALSE;
	}
#elif	EM9283
	{
		DWORD	i;
		UINT32	uData;

		// set JTAG_RTCK pin to GPIO input mode
		DDKIomuxSetPinMux(DDK_IOMUX_JTAG_RTCK, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO4_20, 0);	// output disable -> set as input
		// Set pin drive to 8mA,enable pull up,3.3V
		DDKIomuxSetPadConfig(DDK_IOMUX_GPIO4_20, 
			DDK_IOMUX_PAD_DRIVE_8MA, 
			DDK_IOMUX_PAD_PULL_ENABLE,
			DDK_IOMUX_PAD_VOLTAGE_3V3);

		// delay 10ms
	    OALStall(10000);

		// read state of DBGSLn
		for(i = 0; i < 10; i++)
		{
			DDKGpioReadDataPin(DDK_IOMUX_GPIO4_20, &uData);
			if(uData)
			{
				break;
			}
			// delay 100us
			OALStall(100);
		}

		// final judge
		if(i >= 10)
		{
			return TRUE;
		}

		return FALSE;
	}
#else	// -> iMX28EVK
	return TRUE;
#endif	//EM9280
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
    PBYTE pbOCRAM = (PBYTE) OALPAtoVA(IMAGE_WINCE_IRAM_PA_START, FALSE);
    //PVOID pv_HWregOTP = OALPAtoVA(CSP_BASE_REG_PA_OCOTP, FALSE);

    OEMBootInit();
    //CPUClkInit();
    //KITLOutputDebugString("INFO: Update CPU clock to 454MHz,Hclk to 151MHz.\r\n");
    // Initialize BSP_ARGS to get early clocking info
    //OALBspArgsInit();
    // Serial debug support is now active.  Print BSP_ARGS info.
    //OALBspArgsPrint();

    if(!ClockAlloc())
        ERRORMSG(1, (_T("ClockInit() failed!")));

    if (!ApbhDmaAlloc())
        ERRORMSG(1, (_T("ApbhDmaAlloc failed!")));
    
    if (!IomuxAlloc())
        ERRORMSG(1, (_T("IomuxAlloc failed!")));

    if (!ApbhDmaInit())
        ERRORMSG(1, (_T("ApbhDmaInit failed!")));
    
    NFC_SetClock(TRUE);
    
    
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

    // Initialize the BSP args structure.
    g_pBSPArgs = (BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START;

    if ((g_pBSPArgs->header.signature  != OAL_ARGS_SIGNATURE) ||
        (g_pBSPArgs->header.oalVersion != OAL_ARGS_VERSION)   ||
        (g_pBSPArgs->header.bspVersion != BSP_ARGS_VERSION))
    {
        // zero-out the unused protions of the BspArgs. Frequencies already setup in call to OalBspArgsInit()
        memset(g_pBSPArgs->deviceId, 0, sizeof(g_pBSPArgs->deviceId));
        memset(&g_pBSPArgs->kitl, 0, sizeof(g_pBSPArgs->kitl));
        g_pBSPArgs->header.signature       = OAL_ARGS_SIGNATURE;
        g_pBSPArgs->header.oalVersion      = OAL_ARGS_VERSION;
        g_pBSPArgs->header.bspVersion      = BSP_ARGS_VERSION;
        g_pBSPArgs->kitl.flags             = OAL_KITL_FLAGS_VMINI;
        g_pBSPArgs->kitl.devLoc.IfcType    = Internal;
        g_pBSPArgs->kitl.devLoc.BusNumber  = 0;
        g_pBSPArgs->updateMode = FALSE;
    }
    

    // Initialize timer for stall function
    BootTimerInit();

    // read fuse to determine whether this is an EVK (0x1) or dev board (0x0)
    //g_dwBoardID = (HW_OCOTP_CUSTCAP_RD() & OCOTP_CUSTCAP_BOARDID_MASK) >> OCOTP_CUSTCAP_BOARDID_SHIFT;
    // Force the board ID to be EVK,will update it after got the OTP fuse map.
    g_dwBoardID = BOARDID_EVKBOARD;
    // Attempt to initialize the SD/MMC driver

	// CS&ZHL MAY-18-2012: apply clock to PINCTRL module
	SetupGPIOClock();

#ifdef	EM9280
	// No SD interface in EM9280
    g_bSDHCExist = FALSE;
#else	// -> iMX28EVK or EM9283
    if (!SDMMC_Init())
    {
        KITLOutputDebugString("WARNING: OEMPlatformInit: Failed to initialize SDHC device.\r\n");
        g_bSDHCExist = FALSE;
    }
    else
    {
        g_bSDHCExist = TRUE;
        //switch to user partition
        if (SDController.IsMMC)
        {
            KITLOutputDebugString("OEMPlatformInit: SWITCH SDHC device to user partition.\r\n");
            MMC_Send_Switch_Cmd(EMMC_SWITCH_WRITE_BOOT_PART | (0x0 << EMMC_SWITCH_BOOT_VALUE_SHIFT));
            SDMMC_get_ext_csd();
        }
    }
#endif	//EM9280

	// CS&ZHL MAY-29-2012: get DBGSLn state
	g_pBSPArgs->bDebugFlag = GetDebugMode();
    KITLOutputDebugString("OEMPlatformInit: DEBGSLn = %d\r\n", g_pBSPArgs->bDebugFlag);

    // This will depend on the boot mode
    g_bNandBootloader = TRUE;
    g_bSDHCBootloader = FALSE;
    g_bSPIBootloader  = FALSE;    

    // check for SDBoot
    if ( ((*(PBYTE)((DWORD)pbOCRAM + OCRAM_BOOT_MODE_OFFSET)) & BOOT_MODE_MASK) == BOOT_MODE_SDMMC)
    {
        g_bSPIBootloader  = FALSE;    
        g_bNandBootloader = FALSE;
        g_bSDHCBootloader = TRUE;
        RETAILMSG(1, (L"INFO: Booted from SD/MMC\r\n"));
    }
    
    // check for SPIBoot
    if ( ((*(PBYTE)((DWORD)pbOCRAM + OCRAM_BOOT_MODE_OFFSET)) & BOOT_MODE_MASK) == BOOT_MODE_SPI)
    {
        g_bNandBootloader = FALSE;
        g_bSDHCBootloader = FALSE;
        g_bSPIBootloader  = TRUE;
        RETAILMSG(1, (L"INFO: Booted from SPI Flash\r\n"));
    }

    // Load eboot configuration
    if (!LoadBootCFG(&g_BootCFG))
    {
        // Load default bootloader configuration settings.
        KITLOutputDebugString("ERROR: flash initialization failed - loading bootloader defaults...\r\n");
        ResetDefaultBootCFG(&g_BootCFG);
    }

    //Config EbootCFG
    ConfigBootCFG(&g_BootCFG);

    memcpy(g_pBSPArgs->uuid, g_BootCFG.ExtConfig,16);

    // Set up optional bootloader function pointers.
    g_pOEMMultiBINNotify = OEMMultiBINNotify;
    g_pOEMVerifyMemory = OEMVerifyMemory;
    g_pOEMReportError = OEMReportError;

#if 0
{
  PUCHAR           pv_HWregDUMP = NULL;

  pv_HWregDUMP = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_CLKCTRL);
  if (pv_HWregDUMP == NULL) {
    RETAILMSG(1, (TEXT("Could not allocate dump registers\r\n")));
  }
  else {
    RETAILMSG(1,(TEXT("BOOT LOADER MODE\r\n")));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_PLLCTRL0: %x\r\n"), *(pv_HWregDUMP + 0x0)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_PLLCTRL1: %x\r\n"), *(pv_HWregDUMP + 0x10)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_CPU     : %x\r\n"), *(pv_HWregDUMP + 0x20)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_HBUS    : %x\r\n"), *(pv_HWregDUMP + 0x30)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_XBUS    : %x\r\n"), *(pv_HWregDUMP + 0x40)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_XTAL    : %x\r\n"), *(pv_HWregDUMP + 0x50)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_PIX     : %x\r\n"), *(pv_HWregDUMP + 0x60)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_SSP     : %x\r\n"), *(pv_HWregDUMP + 0x70)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_GPMI    : %x\r\n"), *(pv_HWregDUMP + 0x80)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_SPDIF   : %x\r\n"), *(pv_HWregDUMP + 0x90)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_EMI     : %x\r\n"), *(pv_HWregDUMP + 0xA0)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_IR      : %x\r\n"), *(pv_HWregDUMP + 0xB0)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_SAIF    : %x\r\n"), *(pv_HWregDUMP + 0xC0)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_TV      : %x\r\n"), *(pv_HWregDUMP + 0xD0)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_ETM     : %x\r\n"), *(pv_HWregDUMP + 0xE0)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_FRAC    : %x\r\n"), *(pv_HWregDUMP + 0xF0)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_FRAC1   : %x\r\n"), *(pv_HWregDUMP + 0x100)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_SEQ     : %x\r\n"), *(pv_HWregDUMP + 0x110)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_RESET   : %x\r\n"), *(pv_HWregDUMP + 0x120)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_STATUS  : %x\r\n"), *(pv_HWregDUMP + 0x130)));
    RETAILMSG(1,(TEXT("HW_CLKCTRL_VERSION : %x\r\n"), *(pv_HWregDUMP + 0x140)));
  }
    // Dump all the clock registers.
}
#endif
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
    BOOL fGotJumpImg = FALSE;
    const char * platformString = "iMX28";
    // User menu code...
    if (!BLMenu())
    {
        return rc;
    }

    // Create device name based on Ethernet address (this is how Platform Builder identifies this device).
    OALKitlCreateName(BSP_DEVICE_PREFIX, g_pBSPArgs->kitl.mac, (CHAR *)g_pBSPArgs->deviceId);
    KITLOutputDebugString("INFO: Using device name: '%s'\n", g_pBSPArgs->deviceId);

    if(g_SerialUSBDownload)
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
    }   // USB Downloading
    else if(g_StorageSDDownload)
    {
        CHAR pFileName[12] = { 0 };
        
        // Read DATA to RAM
        if(g_DownloadImage)
        {
            switch(ImageTypeBIN)
            {
                case IMAGE_TYPE_XLDR:
                    strcpy(pFileName, "xldr.bin");
                    KITLOutputDebugString("SDMMCDownload : xldr.bin\r\n");
                    break;
                    
                case IMAGE_TYPE_BOOT:
                    strcpy(pFileName, "eboot.bin");
                    KITLOutputDebugString("SDMMCDownload : eboot.bin\r\n");
                    break;
                    
                case IMAGE_TYPE_NK:
                    strcpy(pFileName, "nk.bin");
                    KITLOutputDebugString("SDMMCDownload : nk.bin\r\n");
                    break;
                    
                default:
                    return rc;
            }
            
            SDMMCDownload(pFileName);
        }
    }   // SDMMC Downloading
    else
    {
        fGotJumpImg = GetPreDownloadInfo (&g_BootCFG);
    }

    if (!g_DownloadImage || // this gets set in the BLMenu() function
        fGotJumpImg)        // this gets set in EbootInitEtherTransport
    {
        switch(g_BootCFG.autoDownloadImage)
        {
        case BOOT_CFG_AUTODOWNLOAD_NK_NOR:
            rc = BL_JUMP;
            break;

        case BOOT_CFG_AUTODOWNLOAD_NK_NAND:
			//
			// CS&ZHL MAY-23-2011: g_BootCFG.dwNandImageStart => virtual start address of OS image in NandFlash
			//                     g_BootCFG.dwNandImageLength => byte length of OS image in NandFlash
			//
            //if (NANDLoadNK(g_BootCFG.dwNandImageStart, g_BootCFG.dwNandImageLength))
            if (NANDLoadNK())
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
        
         g_pBSPArgs->bCleanBootFlag = TRUE;
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
    PVOID  pv_HWregRTC = (PVOID)OALPAtoUA(CSP_BASE_REG_PA_RTC);		// Lqk SEP-14-2012: supporting WDT

	// Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pRomHdr);

    switch(g_BootCFG.autoDownloadImage)
    {
        //case BOOT_CFG_AUTODOWNLOAD_NK_NOR:
        //    // Set launch address for NOR OS image.  OS executes-in-place from NOR.
        //    PhysAddress = IMAGE_BOOT_NKIMAGE_NOR_PA_START;
        //    break;
    
        case BOOT_CFG_AUTODOWNLOAD_NK_NAND:
        case BOOT_CFG_AUTODOWNLOAD_NK_SD:
            // Set launch address for NAND/SDHC OS image.  OS is copied into RAM for execution.
           
            PhysAddress = IMAGE_BOOT_NKIMAGE_RAM_PA_START;
            KITLOutputDebugString("OEMLaunch called PhysAddress 0x%x.\r\n",PhysAddress);
            break;
    
        //case BOOT_CFG_AUTODOWNLOAD_IPL_NAND:
        //    // Set launch address for NAND IPL image.  IPL is copied into RAM for execution.
        //    PhysAddress = IMAGE_BOOT_IPLIMAGE_RAM_START;
        //    break;
    
        default:
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
    if(g_DownloadImage)
    {
        if(g_SerialUSBDownload)
        {
            SerialWaitForJump();
        }
        else if(g_StorageSDDownload)
        {
        }
        else
        {
            GetLaunchInfo();
        }
    }

    // Jump to the image we just downloaded.
    //
    //KITLOutputDebugString("\r\n\r\n\r\n");
    //OEMWriteDebugLED(0, 0x00FF);
	
	// CS&ZHL SEP-14-2012: always enable WDT to prevent launch fail
	KITLOutputDebugString("\r\nAlways Enable WDT before Launch...\r\n\r\n");
	HW_RTC_WATCHDOG_WR( 10000 );
	HW_RTC_CTRL_SET(BM_RTC_CTRL_WATCHDOGEN);

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

    KITLOutputDebugString("INFO: OEMMultiBINNotify (dwNumRegions = %d, dwRegionStart = 0x%x).\r\n",
                          pInfo->dwNumRegions, pInfo->Region[0].dwRegionStart);

	// CS&ZHL MAY-21-2011: add for BinFS
	g_dwTotalBinNum = pInfo->dwNumRegions;
	g_dwCurrentBinNum = 0;

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

        // It is an EBOOT MSB image
        if ((strncmp(szFileName, EBOOT_MSB_FILE, EBOOT_MSB_FILE_LEN) == 0) ||
            (strncmp(szFileName, EBOOT_IVT_MSB_FILE, EBOOT_IVT_MSB_FILE_LEN) == 0))
        {
            // Remap the start address to the region specified by the user
            KITLOutputDebugString("Specify EBOOT destination [1 = NOR, 2 = NAND, 3 = SD/MMC]: ");
            do {
                key = OEMReadDebugByte();
            } while ((key != '1') && (key != '2') && (key != '3'));
            KITLOutputDebugString("\r\n");
            switch (key)
            {
            case '1':
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_BOOTIMAGE_NOR_PA_START);
                break;
                
            case '2':
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_BOOTIMAGE_NAND_PA_START);
                break;

            case '3':
                pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_BOOTIMAGE_SD_PA_START);
                break;

            default:
                return;
            }
        }    
        // Is an NK NB0 image
        else if (strncmp(szFileName, NK_NB0_FILE, NK_NB0_FILE_LEN) == 0)
        {
            // Remap the start address to the region specified by the user
            KITLOutputDebugString("Specify destination for NK NB0 [0 = RAM, 1 = NAND, 2 = SD/MMC]: ");
            do {
                key = OEMReadDebugByte();
            } while ((key != '0') && (key != '1') && (key != '2'));
            KITLOutputDebugString("\r\n");
            switch (key)
            {
                case '0':
                    pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_NKIMAGE_RAM_PA_START);
                    break;
            
                case '1':
                    pInfo->Region[0].dwRegionStart = (DWORD) OALPAtoCA(IMAGE_BOOT_NKIMAGE_NAND_PA_START);
                    break;

                case '2':
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

    /*if ((dwPhysVerifyStart >= IMAGE_BOOT_XLDRIMAGE_NAND_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_XLDRIMAGE_NAND_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading XLDR NAND image.\r\n");
        g_ImageType = IMAGE_TYPE_XLDR;
        g_ImageMemory = IMAGE_MEMORY_NAND;
    }*/
    if ((dwPhysVerifyStart >= IMAGE_BOOT_BOOTIMAGE_NAND_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_BOOTIMAGE_NAND_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading SB NAND image.\r\n");
        g_ImageType = IMAGE_TYPE_BOOT;
        g_ImageMemory = IMAGE_MEMORY_NAND;
    }
    //else if ((dwPhysVerifyStart >= IMAGE_BOOT_IPLIMAGE_NAND_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_IPLIMAGE_NAND_PA_END))
    //{
    //    KITLOutputDebugString("INFO: Downloading IPL NAND image.\r\n");
    //    g_ImageType = IMAGE_TYPE_IPL;
    //    g_ImageMemory = IMAGE_MEMORY_NAND;
    //}
    // DIO and NK share this start address
    else if((dwPhysVerifyStart >= IMAGE_BOOT_NKIMAGE_NAND_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_NKIMAGE_NAND_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading NK NAND image.\r\n");
        g_ImageType = IMAGE_TYPE_NK;
        g_ImageMemory = IMAGE_MEMORY_NAND;
    }
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_BOOTIMAGE_NOR_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_BOOTIMAGE_NOR_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading EBOOT/SBOOT NOR image.\r\n");
        g_ImageType = IMAGE_TYPE_BOOT;
        g_ImageMemory = IMAGE_MEMORY_NOR;
    }
    
    /*else if ((dwPhysVerifyStart >= IMAGE_BOOT_BOOTIMAGE_NOR_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_BOOTIMAGE_NOR_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading EBOOT/SBOOT NOR image.\r\n");
        g_ImageType = IMAGE_TYPE_BOOT;
        g_ImageMemory = IMAGE_MEMORY_NOR;
    }*/
    /*else if ((dwPhysVerifyStart >= IMAGE_BOOT_NKIMAGE_NOR_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_NKIMAGE_NOR_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading NK NOR image.\r\n");
        g_ImageType = IMAGE_TYPE_NK;
        g_ImageMemory = IMAGE_MEMORY_NOR;
    }*/
    /*else if ((dwPhysVerifyStart >= IMAGE_BOOT_NORDEV_NOR_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_NORDEV_NOR_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading NK NOR image.\r\n");
        KITLOutputDebugString("WARNING:  NK image will overwrite EBOOT reserved space \r\n");
        g_ImageType = IMAGE_TYPE_NK;
        g_ImageMemory = IMAGE_MEMORY_NOR;
    }*/
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_RAMDEV_RAM_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_RAMDEV_RAM_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading NK RAM image.\r\n");
        g_ImageType = IMAGE_TYPE_NK;
        g_ImageMemory = IMAGE_MEMORY_RAM;
    }
    /*else if ((dwPhysVerifyStart >= IMAGE_BOOT_XLDRIMAGE_SD_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_XLDRIMAGE_SD_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading XLDR SD/MMC image.\r\n");
        g_ImageType = IMAGE_TYPE_XLDR;
        g_ImageMemory = IMAGE_MEMORY_SD;
    }*/
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_BOOTIMAGE_SD_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_BOOTIMAGE_SD_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading SB SD/MMC image.\r\n");
        g_ImageType = IMAGE_TYPE_BOOT;
        g_ImageMemory = IMAGE_MEMORY_SD;
    }
    else if ((dwPhysVerifyStart >= IMAGE_BOOT_NKIMAGE_SD_PA_START) && (dwPhysVerifyEnd <= IMAGE_BOOT_NKIMAGE_SD_PA_END))
    {
        KITLOutputDebugString("INFO: Downloading NK SD/MMC image.\r\n");
        g_ImageType = IMAGE_TYPE_NK;
        g_ImageMemory = IMAGE_MEMORY_SD;
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
    if (!FlashLoadBootCFG((BYTE*) BootCfg, sizeof(BOOT_CFG)))
    {
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
    g_DefaultRamAddress = (PUCHAR)g_BootCFG.dwLaunchAddr;
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

    if (g_BootCFG.dwConfigFlags & CONFIG_FLAGS_USBKITL)
    {
        //indicate USB RNDIS
        g_pBSPArgs->kitl.devLoc.LogicalLoc = (DWORD) CSP_BASE_REG_PA_USBCTRL0;
        g_pBSPArgs->kitl.devLoc.PhysicalLoc = (PVOID) (CSP_BASE_REG_PA_USBCTRL0);
    }

}


