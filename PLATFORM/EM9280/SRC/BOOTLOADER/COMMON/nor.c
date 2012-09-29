//-----------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  nor.c
//
//  Contains BOOT NOR flash support functions.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include "loader.h"
#pragma warning(push)
#pragma warning(disable: 4115)
#include <fmd.h>
#include <hw_spi.h>
#pragma warning(pop)

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregDIGCTL;
//-----------------------------------------------------------------------------
// Defines
#define SPI                 3         // Flash Type
#define SECTOR_SIZE         4096      // Sector Size
#define BLOCK_CNT           16        // Max Number of Blocks
#define SECTOR_CNT          16        // Number of sectors to constitute a Block
#define NOR_PAGE_SIZE       256       // Page size in bytes
#define CONFIGBLOCK         8         // Config data store from block 8
#define BLOCKSIZE           SECTOR_CNT * SECTOR_SIZE
#define NORFlashID          0x13EF13EF

#define WREN_CMD            0x06
#define WRDI_CMD            0x04
#define RDSR_CMD            0x05
#define WRSR_CMD            0x01
#define BE_CMD              0xD8
#define PP_CMD              0x02
#define READ_CMD            0x03
#define CHER_CMD            0xC7
#define BLER_CMD            0xD8
#define RDID_CMD            0x90
#define RLPW_CMD            0xAB

#define DATA_TIMEOUT        0xFFFF
#define CLOCK_DIVIDE        0x8      // SSPCLOCK/8
#define CLOCK_RATE          0x0

#define SPI_NOR_SSP2        0x2

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
static PVOID pv_HWregSSP0 = NULL;
static PVOID pv_HWregSSP1 = NULL;
static PVOID pv_HWregSSP2 = NULL;
static PVOID pv_HWregSSP3 = NULL;
SSP_INIT spiinit;
UINT8 TxData8; 
UINT8 RxData8;
UINT16 TxData16; 
UINT16 RxData16;
UINT32 TxData32; 
UINT32 RxData32;
//UINT8  PageData[NOR_PAGE_SIZE]; 
//UINT32 len = 0;

SPI_BUSCONFIG_T buscnfgwrite8 =
{
    BM_SSP_CTRL0_LOCK_CS | BM_SSP_CTRL0_DATA_XFER,
    0,
    0,
    FALSE,
    TRUE, 
    8,
    FALSE,
    0,
};

SPI_BUSCONFIG_T buscnfgread8 =
{
    BM_SSP_CTRL0_IGNORE_CRC | BM_SSP_CTRL0_READ | BM_SSP_CTRL0_DATA_XFER,
    0,
    0,
    FALSE,
    TRUE,
    8,
    TRUE,
    0,
};

SPI_XCH_PKT_T xchpktwrite8 =
{
    &buscnfgwrite8,
    &TxData8,
    1,
    NULL,
    0
};

SPI_XCH_PKT_T xchpktread8 =
{
    &buscnfgread8,
    &RxData8,
    1,
    NULL,
    0
};

SPI_BUSCONFIG_T buscnfgwrite16 =
{
    BM_SSP_CTRL0_LOCK_CS | BM_SSP_CTRL0_DATA_XFER,
    0,
    0,
    FALSE,
    TRUE,
    16,
    FALSE,
    0,
};

SPI_BUSCONFIG_T buscnfgread16 =
{
    BM_SSP_CTRL0_IGNORE_CRC | BM_SSP_CTRL0_READ | BM_SSP_CTRL0_DATA_XFER,
    0,
    0,
    FALSE,
    TRUE,
    16,
    TRUE,
    0,
};

SPI_XCH_PKT_T xchpktwrite16 =
{
    &buscnfgwrite16,
    &TxData16,
    1,
    NULL,
    0
};

SPI_XCH_PKT_T xchpktread16 =
{
    &buscnfgread16,
    &RxData16,
    1,
    NULL,
    0
};

SPI_BUSCONFIG_T buscnfgwrite32 =
{
    BM_SSP_CTRL0_LOCK_CS | BM_SSP_CTRL0_DATA_XFER,
    0,
    0,
    FALSE,
    TRUE,
    32,
    FALSE,
    0,
};

SPI_BUSCONFIG_T buscnfgread32 =
{
    BM_SSP_CTRL0_IGNORE_CRC | BM_SSP_CTRL0_READ | BM_SSP_CTRL0_DATA_XFER,
    0,
    0,
    FALSE,
    TRUE,
    32,
    TRUE,
    0,
};

SPI_XCH_PKT_T xchpktwrite32 =
{
    &buscnfgwrite32,
    &TxData32,
    1,
    NULL,
    0
};

SPI_XCH_PKT_T xchpktread32 =
{
    &buscnfgread32,
    &RxData32,
    1,
    NULL,
    0
};


//-----------------------------------------------------------------------------
// Local Functions
BOOL SPIInit();
static BOOL DataExchange(PSPI_XCH_PKT_T pXchPkt,BOOL bRead);
static VOID DumpSSPRegisters(void);
static VOID SpiBufWrt8(LPVOID pBuf, UINT32 data);
static VOID SpiBufWrt16(LPVOID pBuf, UINT32 data);
static VOID SpiBufWrt32(LPVOID pBuf, UINT32 data);
static VOID UnlockBlock();
static VOID BytePro(UINT32 Addr);
static VOID WriteEnable();
static VOID WriteDisable();
static VOID WriteStatus(UINT8 status);
static VOID ChipErase();
static VOID PageProgram(UINT32 Addr,UINT8 * pData);
static VOID BlockErase(UINT32 Addr);
static VOID ReleasePower();
static UINT8 ReadData(UINT32 Addr);
static UINT8 ReadStatus();
static UINT32 SpiBufRd32(LPVOID pBuf);
static UINT32 SpiBufRd16(LPVOID pBuf);
static UINT32 SpiBufRd8(LPVOID pBuf);
static UINT32 ReadID();
BOOL NORStoreBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize);
BOOL NORLoadBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize);


//------------------------------------------------------------------------------
// Function: NORWrite
//
// Return Value:
//        TRUE if success,FALSE if fail.
//------------------------------------------------------------------------------
BOOL NORWrite(DWORD dwStartAddr, DWORD dwLength)
{
    BOOL ret = TRUE;    
    BOOL bVerify = TRUE;
    UINT32 i = 0;
    LPBYTE pImage; 
    UINT32 percentComplete = 0;
    UINT32 u32TotalBootPage = 0;
    UNREFERENCED_PARAMETER(dwStartAddr);
    u32TotalBootPage = (dwLength / NOR_PAGE_SIZE) + 1;

    //Init SPI controller.
    SPIInit();
    //Map Image addr.
    pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);
    //Make sure the NOR flash is available.
    
    if(ReadID() != NORFlashID)
    { 
        EdbgOutputDebugString("Can not find NOR flash!\r\n");    
        ret = FALSE;
        goto cleanUp;
    }
    ReleasePower();
    WriteEnable();
    //RETAILMSG(1,(TEXT("ReadStatus = 0x%x \r\n"),ReadStatus()));   

    for(i = 0;i < CONFIGBLOCK;i++)
        BlockErase(i * BLOCKSIZE);
    
    EdbgOutputDebugString("Waiting...\r\n");      
    for(i = 0;i < u32TotalBootPage;i++)
    {
        PageProgram((i * NOR_PAGE_SIZE),(pImage + i * NOR_PAGE_SIZE));
        //percentComplete =  100 * (i + 1) / u32TotalBootPage;
        //OEMWriteDebugByte('\r');
        //EdbgOutputDebugString("INFO: Programming image are %d%% completed.", percentComplete);      
    }
    //EdbgOutputDebugString("\r\n"); 

    percentComplete = 0;
    for(i = 0;i < dwLength;i++)
    {
        if(ReadData(i) != *(pImage + i))
        {
            EdbgOutputDebugString("INFO: Verifying image failed.\r\n");
            bVerify = FALSE;
        }
        if(i % NOR_PAGE_SIZE == 0)
        {
            percentComplete =  100 * (i + NOR_PAGE_SIZE) / dwLength;
            OEMWriteDebugByte('\r');
            EdbgOutputDebugString("INFO: Programming and Verifying image are %d%% completed.", percentComplete);              
        }
    }  
    //EdbgOutputDebugString("\r\nINFO: Verifying image succeed.\r\n");    
    if(bVerify == TRUE)
    {
        EdbgOutputDebugString("\r\nINFO: Updating of SB image completed successfully.\r\n");
    }
    else
    {
        EdbgOutputDebugString("\r\nINFO: Updating of SB image completed failed.\r\n");
    }
cleanUp:
    return ret;
}

//------------------------------------------------------------------------------
//
//  Function:  NORLoadBootCFG
//
//  Retrieves bootloader configuration information (menu settings, etc.) from
//  the NOR flash.
//
//  Parameters:
//      eBootCFG
//          [out] Points to bootloader configuration that will be filled with
//          loaded data.
//
//      cbBootCfgSize
//          [in] Size in bytes of the bootloader configuration.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NORLoadBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize)
{
    UINT32 u32Loop = 0;
    UINT32 Address = CONFIGBLOCK * SECTOR_CNT * SECTOR_SIZE;
    SPIInit();

    for(u32Loop = 0;u32Loop < cbBootCfgSize;u32Loop++)
        *(pBootCfg + u32Loop) = ReadData(Address + u32Loop);

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  NORStoreBootCFG
//
//  Stores bootloader configuration information (menu settings, etc.) to
//  the NOR flash.
//
//  Parameters:
//      eBootCFG
//          [out] Points to bootloader configuration that will be stored.
//
//      cbBootCfgSize
//          [in] Size in bytes of the bootloader configuration.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NORStoreBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize)
{
    UINT32 u32TotalPage = 0;
    UINT32 u32Loop = 0;
    UINT32 Address = CONFIGBLOCK * SECTOR_CNT * SECTOR_SIZE;
    SPIInit();
    BlockErase(Address);

    u32TotalPage = (cbBootCfgSize / NOR_PAGE_SIZE) + 1;

    for(u32Loop = 0;u32Loop < u32TotalPage;u32Loop++)
        PageProgram(((u32Loop * NOR_PAGE_SIZE) + CONFIGBLOCK * SECTOR_CNT * SECTOR_SIZE),(pBootCfg + u32Loop * NOR_PAGE_SIZE));

    return TRUE;
}

//------------------------------------------------------------------------------
// Function: SPI_Init
//
// This function initializes the SSP Block of the Hardware for NOR flash operation.
// Return Value:
//        returns TRUE if the initialization process is successful, otherwise 
//        returns FALSE.
//
//------------------------------------------------------------------------------
BOOL SPIInit()
{  
    SSP_CTRL0 sControl0;
    SSP_CTRL1 sControl1;
    UINT32 u32Timing;
    PVOID pv_HWregCLKCTRL = NULL;
    
    //RETAILMSG(1, (TEXT("SPI_Init ++\r\n")));
    // Map the pointer.
    pv_HWregSSP2 = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_SSP2, FALSE);
    
    if (pv_HWregCLKCTRL == NULL)
    {
        pv_HWregCLKCTRL = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_CLKCTRL, FALSE);
        if (pv_HWregCLKCTRL == NULL)
        {
            RETAILMSG(1, (TEXT("SPI_Init::OALPAtoVA map failed for pv_HWregCLKCTRL.\r\n")));
        }
    }
    
    if (pv_HWregDIGCTL == NULL)
    {
        pv_HWregDIGCTL = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_DIGCTL, FALSE);
        if (pv_HWregDIGCTL == NULL)
        {
            RETAILMSG(1, (TEXT("SPI_Init::OALPAtoVA map failed for pv_HWregDIGCTL.\r\n")));
        }
    }

    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_SSP2);     //SSP1 clock from 24MHz XTAL
    HW_CLKCTRL_SSP2_CLR(BM_CLKCTRL_SSP2_CLKGATE);  //Un-gate the SSP clock.

    // SSP Reset
    HW_SSP_CTRL0_CLR(SPI_NOR_SSP2,BM_SSP_CTRL0_SFTRST);
    while(HW_SSP_CTRL0_RD(SPI_NOR_SSP2) & BM_SSP_CTRL0_SFTRST);

    HW_SSP_CTRL0_CLR(SPI_NOR_SSP2,BM_SSP_CTRL0_CLKGATE);
    while(HW_SSP_CTRL0_RD(SPI_NOR_SSP2) & BM_SSP_CTRL0_CLKGATE);

    DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D3_1,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D0_1,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_SSP2_CMD_1,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_SSP2_SCK_1,DDK_IOMUX_MODE_00);
    //DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D2_1,DDK_IOMUX_MODE_00);
    //DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D3_1,DDK_IOMUX_MODE_00);
    
    //Configure SSP pins data+clk+cmd for 8mA drive strength, 3 volts.
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_CMD_1,DDK_IOMUX_PAD_DRIVE_8MA,0,1);
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_D0_1,DDK_IOMUX_PAD_DRIVE_8MA,0,1);
    //DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_D2_1,DDK_IOMUX_PAD_DRIVE_8MA,0,1);
    //DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_D3_1,DDK_IOMUX_PAD_DRIVE_8MA,0,1);
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_D3_1,DDK_IOMUX_PAD_DRIVE_8MA,0,1);
    DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_SCK_1,DDK_IOMUX_PAD_DRIVE_8MA,0,1);

    //Set PWM3 to GPIOs.
    //DDKIomuxSetPinMux(DDK_IOMUX_PWM3,DDK_IOMUX_MODE_GPIO);
    //Must set PWM3 to low to disable a SPI Ether buffer chip,or the NOR Flash can't be read.
    //DDKGpioWriteDataPin(DDK_IOMUX_PWM3, 1);
    //DDKGpioEnableDataPin(DDK_IOMUX_PWM3,1);
    //DDKGpioWriteDataPin(DDK_IOMUX_PWM3, 0);

    // Configure SSP Control SPIREGister 1
    sControl1.U = 0;

    // Configure the SSP Control SPIREGisters
    // SSP_CTRL1 SPIREGister values
    sControl1.B.DMA_ENABLE         = FALSE;   // Enables DMA request and DMA Command End signals to be asserted.
    sControl1.B.CEATA_CCS_ERR_EN   = FALSE;   // CEATA Unexpected CCS Error Enable.
    sControl1.B.SLAVE_OUT_DISABLE  = FALSE;   // 0 means SSP can drive MISO in Slave Mode
    sControl1.B.PHASE              = FALSE;   // Serial Clock Phase. For SPI mode only
    sControl1.B.POLARITY           = FALSE;   // Command and TX data change after rising edge of SCK
    sControl1.B.WORD_LENGTH        = SSP_WORD_LENGTH_8BITS; // FALSE - SSP is in Master Mode
    sControl1.B.SLAVE_MODE         = FALSE;                 // 8 Bits per word
    sControl1.B.SSP_MODE           = SSP_MODE_SPI;          // SSP_MODE_SPI=0, Motorala SPI mode.

    // Configure SSP Control SPIREGister 0
    sControl0.U = 0;
    // SSP_CTRL0 SPIREGister values
    sControl0.B.LOCK_CS            = 0;        // 0= Deassert chip select (CS) after transfer is complete.
    sControl0.B.IGNORE_CRC         = 0;        // Ignore CRC - SD/MMC, MS - Ignores the Response CRC.
    sControl0.B.BUS_WIDTH          = 0;        // Data bus is 1-bit wide
    sControl0.B.WAIT_FOR_IRQ       = FALSE;   
    sControl0.B.LONG_RESP          = FALSE;
    sControl0.B.CHECK_RESP         = FALSE;
    sControl0.B.GET_RESP           = FALSE;

    // Write the SSP Control SPIREGister 0 and 1 values out to the interface
    HW_SSP_CTRL0_WR(SPI_NOR_SSP2,sControl0.U);
    HW_SSP_CTRL1_WR(SPI_NOR_SSP2,sControl1.U);

    //while(HW_CLKCTRL_SSP_RD() & 0x20000000); //wait for the clock stable.

    u32Timing = (UINT32)(DATA_TIMEOUT << 16) | CLOCK_DIVIDE << 8  | CLOCK_RATE;
    HW_SSP_TIMING_WR(SPI_NOR_SSP2,u32Timing);

    //RETAILMSG(1, (TEXT("SSP: SSP_TIMING       = 0x%x\r\n"),INREG32(&SPIREG->TIMING)));

    // Make certain that any changes we made to CLOCK_RATE take
    // effect, by writing to CTRL1 after TIMING (CQ PSGHW00000012) 
    HW_SSP_CTRL1_WR(SPI_NOR_SSP2,sControl1.U);
    
    // Configure SSP Control Register 0                                                                                                                                                                       
   
    sControl0.B.IGNORE_CRC        = FALSE;
    sControl0.B.BUS_WIDTH         = FALSE;
    sControl0.B.WAIT_FOR_IRQ      = FALSE;
    sControl0.B.LONG_RESP         = FALSE;
    sControl0.B.CHECK_RESP        = FALSE;
    sControl0.B.GET_RESP          = FALSE;
    sControl0.B.WAIT_FOR_CMD      = FALSE;
    sControl0.B.DATA_XFER         = FALSE;
   
    // Configure SSP Control Register 1
   
    sControl1.B.DMA_ENABLE        = FALSE;
    sControl1.B.CEATA_CCS_ERR_EN  = FALSE;
    sControl1.B.SLAVE_OUT_DISABLE = FALSE;
    sControl1.B.PHASE             = FALSE;
    sControl1.B.POLARITY          = FALSE;
    sControl1.B.WORD_LENGTH       = SSP_WORD_LENGTH_8BITS;
    sControl1.B.SLAVE_MODE        = FALSE;
    sControl1.B.SSP_MODE          = SSP_MODE_SPI;
   
    // Configure SSP Control Register 0  
   
    // Write the SSP Control Register 0 and 1 values out to the interface
    HW_SSP_CTRL0_WR(SPI_NOR_SSP2,sControl0.U);
    HW_SSP_CTRL1_WR(SPI_NOR_SSP2,sControl1.U);
    //RETAILMSG(1, (TEXT("SPI_Init --\r\n")));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: DataExchange
//
//  This function performs the SPI transactions.
//
//  Parameters:
//        pbyTxBuf
//          [in] Pointer to transmit buffer
//        pbyRxBuf
//          [in] Pointer to receive buffer
//        dwMsgLen
//          [in] Length of TX/RX buffer in units of message words (in this case bytes)
//
//  Returns:
//        TRUE if transaction was  successful, else return FALSE
//
//-----------------------------------------------------------------------------
static BOOL  DataExchange(PSPI_XCH_PKT_T pXchPkt,BOOL bRead)
{
    PSPI_BUSCONFIG_T pBusCnfg = pXchPkt->pBusCnfg;
    LPVOID pBuf = pXchPkt->pBuf;
    volatile UINT32 tmp;
    BOOL bXchDone = FALSE;
    UINT32 xchCnt = 0;
    UINT32 (*pfnBufRd)(LPVOID);
    VOID (*pfnBufWrt)(LPVOID, UINT32);
    UINT8 bufIncr = 0;
    //RETAILMSG(1, (TEXT("CspiDataExchange ++\r\n")));

    if ((pBusCnfg->bitcount >= 1) && (pBusCnfg->bitcount <= 8))
    {
      
        // 8-bit access width
        pfnBufRd = SpiBufRd8;
        pfnBufWrt = SpiBufWrt8;
        bufIncr = sizeof(UINT8);
            // set client SPI bus configuration based
        if(bRead)
        {
            HW_SSP_CTRL0_WR(SPI_NOR_SSP2,buscnfgread8.SspCtrl0.U);
            HW_SSP_XFER_SIZE_WR(SPI_NOR_SSP2,1);
            if(buscnfgread8.bCmd)
            {
                HW_SSP_CMD0_WR(SPI_NOR_SSP2,buscnfgread8.SspCmd.U);
                HW_SSP_CMD0_WR(SPI_NOR_SSP2,buscnfgread8.SspArg.U);
            }
        }
        else
        {
            HW_SSP_CTRL0_WR(SPI_NOR_SSP2,buscnfgwrite8.SspCtrl0.U);
            HW_SSP_XFER_SIZE_WR(SPI_NOR_SSP2,1);
            if(buscnfgwrite8.bCmd)
            {
                HW_SSP_CMD0_WR(SPI_NOR_SSP2,buscnfgwrite8.SspCmd.U);
                HW_SSP_CMD1_WR(SPI_NOR_SSP2,buscnfgwrite8.SspArg.U);
            }
        }
    }
    else if ((pBusCnfg->bitcount >= 9) && (pBusCnfg->bitcount <= 16))
    {
        // 16-bit access width
        pfnBufRd = SpiBufRd16;
        pfnBufWrt = SpiBufWrt16;
        bufIncr = sizeof(UINT16);
            // set client SPI bus configuration based
        if(bRead)
        {
            HW_SSP_CTRL0_WR(SPI_NOR_SSP2,buscnfgread16.SspCtrl0.U);
            HW_SSP_XFER_SIZE_WR(SPI_NOR_SSP2,2);

            if(buscnfgread16.bCmd)
            {
                HW_SSP_CMD0_WR(SPI_NOR_SSP2,buscnfgread16.SspCmd.U);
                HW_SSP_CMD1_WR(SPI_NOR_SSP2,buscnfgread16.SspArg.U);
            }
        }
        else
        {
            HW_SSP_CTRL0_WR(SPI_NOR_SSP2,buscnfgwrite16.SspCtrl0.U);
            HW_SSP_XFER_SIZE_WR(SPI_NOR_SSP2,2);

            if(buscnfgwrite16.bCmd)
            {
                HW_SSP_CMD0_WR(SPI_NOR_SSP2,buscnfgwrite16.SspCmd.U);
                HW_SSP_CMD1_WR(SPI_NOR_SSP2,buscnfgwrite16.SspArg.U);
            }
        }
    }
    else if ((pBusCnfg->bitcount >= 17) && (pBusCnfg->bitcount <= 32))
    {
        // 32-bit access width
        pfnBufRd = SpiBufRd32;
        pfnBufWrt = SpiBufWrt32;
        bufIncr = sizeof(UINT32);
            // set client SPI bus configuration based
        if(bRead)
        {
            HW_SSP_CTRL0_WR(SPI_NOR_SSP2,buscnfgread32.SspCtrl0.U);
            HW_SSP_XFER_SIZE_WR(SPI_NOR_SSP2,4);

            if(buscnfgread32.bCmd)
            {
                HW_SSP_CMD0_WR(SPI_NOR_SSP2,buscnfgread32.SspCmd.U);
                HW_SSP_CMD1_WR(SPI_NOR_SSP2,buscnfgread32.SspArg.U);
            }
        }
        else
        {
            HW_SSP_CTRL0_WR(SPI_NOR_SSP2,buscnfgwrite32.SspCtrl0.U);
            HW_SSP_XFER_SIZE_WR(SPI_NOR_SSP2,4);
            
            if(buscnfgwrite32.bCmd)
            {
                HW_SSP_CMD0_WR(SPI_NOR_SSP2,buscnfgwrite32.SspCmd.U);
                HW_SSP_CMD1_WR(SPI_NOR_SSP2,buscnfgwrite32.SspArg.U);
            }
        }
    }
    else
    {
        // unsupported access width
        RETAILMSG(TRUE, (TEXT("CspiMasterDataExchange:  unsupported bitcount!\r\n")));
        return 0;
    }

    //DumpSSPRegisters();
    // until we are done with requested transfers
    while(!bXchDone)
    {
        if(pBuf != NULL)
        {
            // Process the SDMMC Data Buffer
            if(bRead == TRUE)
            {
                while (xchCnt < pXchPkt->xchCnt) 
                {
                    // start exchange
                    HW_SSP_CTRL0_SET(SPI_NOR_SSP2,BM_SSP_CTRL0_RUN);
                    while((HW_SSP_STATUS_RD(SPI_NOR_SSP2) & BM_SSP_STATUS_FIFO_EMPTY) != 0);

                    tmp = HW_SSP_DATA_RD(SPI_NOR_SSP2);
                   
                    // if receive data is not to be discarded
                    if (pBuf != NULL)
                    {
                        // get next Rx data from SPI FIFO
                        pfnBufWrt(pBuf, tmp);
                        // increment Rx Buffer to next data point
                        pBuf = (LPVOID) ((UINT) pBuf + bufIncr);
                    }

                    // increment Rx exchange counter
                    xchCnt++;
                        
                }
                // set flag to indicate requested exchange done
                bXchDone = TRUE;
            }
            else 
            {
                // load FIFO until full, or until we run out of data
                while (xchCnt < pXchPkt->xchCnt)
                {
                    HW_SSP_CTRL0_SET(SPI_NOR_SSP2,BM_SSP_CTRL0_RUN);
                    // put next Tx data into SPI FIFO
                    if((HW_SSP_STATUS_RD(SPI_NOR_SSP2) & BM_SSP_STATUS_FIFO_FULL) == 0)
                        HW_SSP_DATA_WR(SPI_NOR_SSP2,pfnBufRd(pBuf));

                    else
                        while((HW_SSP_STATUS_RD(SPI_NOR_SSP2) & BM_SSP_STATUS_FIFO_EMPTY) == 0);

                        // increment Tx Buffer to next data point
                        pBuf = (LPVOID) ((UINT) pBuf + bufIncr);
                        
                        //Sleep(10);
                        //tmp = HW_SSP_DATA_RD();
    
                        // increment exchange counter
                        xchCnt++;
                }
                    // set flag to indicate requested exchange done
                    bXchDone = TRUE;
            }
        }
        else
        {
              // Process the SDMMC Command
            if(pXchPkt->xchCnt == 0)
            {
                // start exchange
                HW_SSP_CTRL0_SET(SPI_NOR_SSP2,BM_SSP_CTRL0_RUN);
                bXchDone = TRUE;
            }
        }
    } // while(!bXchDone)
        
    //RETAILMSG(1, (TEXT("CspiDataExchange --\r\n")));
    return xchCnt;
}

//-----------------------------------------------------------------------------
//
// Function: CspiBufRd8
//
// This function is used to access a buffer as an array of 8-bit (UINT8) 
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer, promoted to UINT32.
//
//-----------------------------------------------------------------------------
UINT32 SpiBufRd8(LPVOID pBuf)
{
    UINT8 *p;
    p = (UINT8 *) pBuf;
    return *p;
}

//-----------------------------------------------------------------------------
//
// Function: CspiBufWrt8
//
// This function is used to access a buffer as an array of 8-bit (UINT8) 
// values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written demoted to UINT8.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SpiBufWrt8(LPVOID pBuf, UINT32 data)
{
    UINT8 *p;
    p = (UINT8 *) pBuf;
    *p = (UINT8) data;
}

//-----------------------------------------------------------------------------
//
// Function: CspiBufRd8
//
// This function is used to access a buffer as an array of 8-bit (UINT8) 
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer, promoted to UINT32.
//
//-----------------------------------------------------------------------------
UINT32 SpiBufRd16(LPVOID pBuf)
{
    UINT16 *p;
    p = (UINT16 *) pBuf;
    return *p;
}

//-----------------------------------------------------------------------------
//
// Function: CspiBufWrt8
//
// This function is used to access a buffer as an array of 8-bit (UINT8) 
// values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written demoted to UINT8.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SpiBufWrt16(LPVOID pBuf, UINT32 data)
{
    UINT16 *p;
    p = (UINT16 *) pBuf;
    *p = (UINT16) data;
}

//-----------------------------------------------------------------------------
//
// Function: CspiBufRd8
//
// This function is used to access a buffer as an array of 8-bit (UINT8) 
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer, promoted to UINT32.
//
//-----------------------------------------------------------------------------
UINT32 SpiBufRd32(LPVOID pBuf)
{
    UINT32 *p;
    p = (UINT32 *) pBuf;
    return *p;
}

//-----------------------------------------------------------------------------
//
// Function: CspiBufWrt8
//
// This function is used to access a buffer as an array of 8-bit (UINT8) 
// values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written demoted to UINT8.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SpiBufWrt32(LPVOID pBuf, UINT32 data)
{
    UINT32 *p;
    p = (UINT32 *) pBuf;
    *p = (UINT32) data;
}
//------------------------------------------------------------------------------
// Function: ReadID
//     Read ID from SPI NOR.
//
// Return Value:
//     ID read.
//------------------------------------------------------------------------------
UINT32 ReadID()
{
    TxData32 = RDID_CMD;
    //WaitForSingleObject(hEvent, INFINITE);
    
    if (!DataExchange(&xchpktwrite32,FALSE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }
    OALStall(50);
    
    if (!DataExchange(&xchpktread32,TRUE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }
    //RETAILMSG(TRUE,(TEXT("ID = 0x%x\r\n"),RxData32));
    OALStall(50);
    return RxData32;        
}
//------------------------------------------------------------------------------
// Function: WriteEnable
//     Enable write SPI NOR flash. 
//
// Return Value:
//     None.
//------------------------------------------------------------------------------
void WriteEnable()
{
    xchpktwrite8.pBusCnfg->SspCtrl0.U |= BM_SSP_CTRL0_IGNORE_CRC;
    TxData8 = WREN_CMD;
    
    if (!DataExchange(&xchpktwrite8,FALSE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }
    xchpktwrite8.pBusCnfg->SspCtrl0.U &= ~BM_SSP_CTRL0_IGNORE_CRC;
    OALStall(50);

}
//------------------------------------------------------------------------------
// Function: WriteDisable
//     Disable write SPI NOR flash. 
//
// Return Value:
//     None.
//------------------------------------------------------------------------------
void WriteDisable()
{
    xchpktwrite8.pBusCnfg->SspCtrl0.U |= BM_SSP_CTRL0_IGNORE_CRC;
    TxData8 = WRDI_CMD;
    
    if (!DataExchange(&xchpktwrite8,FALSE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }
    xchpktwrite8.pBusCnfg->SspCtrl0.U &= ~BM_SSP_CTRL0_IGNORE_CRC;
    OALStall(50);
}

//------------------------------------------------------------------------------
// Function: ReadStatus
//    This function is to read the status register of SPI NOR.
//
// Return Value:
//    Status read
//------------------------------------------------------------------------------
UINT8 ReadStatus()
{   
    TxData8 = RDSR_CMD;    
    if (!DataExchange(&xchpktwrite8,FALSE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }
    OALStall(50);
    if (!DataExchange(&xchpktread8,TRUE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }
    OALStall(50);
    //RETAILMSG(1,(TEXT("RxData32 = 0x%x\r\n"),RxData32));

    return RxData8;        
}
//------------------------------------------------------------------------------
// Function: WriteStatus
//    This function is to write status register of NOR Flash.
//
// Return Value:
//    None.
//------------------------------------------------------------------------------
VOID WriteStatus(UINT8 status)
{
    WriteEnable();
    TxData8 = WRSR_CMD;    
    if (!DataExchange(&xchpktwrite8,FALSE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }
    OALStall(50);

    TxData8 = status;    
    xchpktwrite8.pBusCnfg->SspCtrl0.U |= BM_SSP_CTRL0_IGNORE_CRC;
    if (!DataExchange(&xchpktwrite8,FALSE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }
    xchpktwrite8.pBusCnfg->SspCtrl0.U &= ~BM_SSP_CTRL0_IGNORE_CRC;
    OALStall(50);      
}

//------------------------------------------------------------------------------
// Function: WriteStatusEn
//    This function is to unlock the block of SPI NOR.
//
// Return Value:
//    None.
//------------------------------------------------------------------------------
VOID UnlockBlock()
{
    WriteStatus(0x2);
    //RETAILMSG(1,(TEXT("new status =  0x%x\r\n"),ReadStatus()));
}
//------------------------------------------------------------------------------
// Function: ChipErase
//    This function is to erase the whole chip of SPI NOR.
//
// Return Value:
//    None.
//------------------------------------------------------------------------------
VOID ChipErase()
{
    UINT32 status = 0;
    WriteEnable();
    //RETAILMSG(1,(TEXT("status =  0x%x\r\n"),ReadStatus()));
    
    TxData8 = CHER_CMD;
    xchpktwrite8.pBusCnfg->SspCtrl0.U |= BM_SSP_CTRL0_IGNORE_CRC;
    if (!DataExchange(&xchpktwrite8,FALSE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }
    xchpktwrite8.pBusCnfg->SspCtrl0.U &= ~BM_SSP_CTRL0_IGNORE_CRC;
    OALStall(50);
    do
    {
        status = ReadStatus();
        RETAILMSG(1,(TEXT("erasing...\r")));
        OALStall(50);
    } while(status & 0x1);

    RETAILMSG(1,(TEXT("done!\r\n")));
}
//------------------------------------------------------------------------------
// Function: BlockErase
//    This function is to erase a block of SPI NOR.
//
// Return Value:
//    None.
//------------------------------------------------------------------------------
VOID BlockErase(UINT32 Addr)
{
    UINT32 status = 0;
    WriteEnable();
    
    Addr = ((Addr & 0xFF) << 24) | ((Addr & 0xFF00) << 8) | ((Addr & 0xFF0000) >> 8);
    TxData32 = (Addr & 0xFFFFFF00) | BLER_CMD;

    xchpktwrite32.pBusCnfg->SspCtrl0.U |= BM_SSP_CTRL0_IGNORE_CRC;

    if (!DataExchange(&xchpktwrite32,FALSE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }

    xchpktwrite32.pBusCnfg->SspCtrl0.U &= ~BM_SSP_CTRL0_IGNORE_CRC;
    OALStall(50);
    do
    {
        status = ReadStatus();
        OALStall(50);
    } while(status & 0x1);

}

//------------------------------------------------------------------------------
// Function: ReadData
//    This function is to read a byte data from NOR Flash.
//
// Return Value:
//    The data read.
//------------------------------------------------------------------------------
UINT8 ReadData(UINT32 Addr)
{   
    Addr = ((Addr & 0xFF) << 24) | ((Addr & 0xFF00) << 8) | ((Addr & 0xFF0000) >> 8);
    TxData32 = (Addr & 0xFFFFFF00) | READ_CMD;

    if (!DataExchange(&xchpktwrite32,FALSE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }

    OALStall(50);

    if (!DataExchange(&xchpktread8,TRUE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }
       
    //OALStall(50);
    return RxData8;   
}

//------------------------------------------------------------------------------
// Function: PageProgram
//    This function is to program the NOR Flash by page.
//
// Return Value:
//    None. 
//------------------------------------------------------------------------------
VOID PageProgram(UINT32 Addr,UINT8 * pData)
{
    UINT32 u32Loop = 0;
    UINT32 status = 0;
    WriteEnable();
    Addr = ((Addr & 0xFF) << 24) | ((Addr & 0xFF00) << 8) | ((Addr & 0xFF0000) >> 8);
    TxData32 = (Addr & 0xFFFFFF00) | PP_CMD;

    xchpktwrite32.pBusCnfg->SspCtrl0.U &= ~BM_SSP_CTRL0_IGNORE_CRC;

    if (!DataExchange(&xchpktwrite32,FALSE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }
    OALStall(50);
    
    for(u32Loop = 0;u32Loop < 64;u32Loop++)
    {
        TxData32 = *(UINT32 *)(pData);
        //If it is last 4 bytes,set CS to high.
        if(u32Loop == 63)
            xchpktwrite32.pBusCnfg->SspCtrl0.U |= BM_SSP_CTRL0_IGNORE_CRC;
        if (!DataExchange(&xchpktwrite32,FALSE))
        {
            RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
        }
        OALStall(100);
        pData = pData + 4;
    }
    OALStall(50);
    xchpktwrite32.pBusCnfg->SspCtrl0.U &= ~BM_SSP_CTRL0_IGNORE_CRC;
    do
    {
        status = ReadStatus();
        OALStall(100);
    } while(status & 0x1);

}

//------------------------------------------------------------------------------
// Function: ReleasePower
//    This function is to release power control of SPI NOR.
//
// Return Value:
//              none.
//------------------------------------------------------------------------------
VOID ReleasePower()
{
    xchpktwrite8.pBusCnfg->SspCtrl0.U |= BM_SSP_CTRL0_IGNORE_CRC;
    TxData8 = RLPW_CMD;
    
    if (!DataExchange(&xchpktwrite8,FALSE))
    {
        RETAILMSG(1,(TEXT("SPIExchange failure.\r\n")));
    }    
    xchpktwrite8.pBusCnfg->SspCtrl0.U &= ~BM_SSP_CTRL0_IGNORE_CRC;
    OALStall(50); 
}

//------------------------------------------------------------------------------
// Function: DumpSSPRegisters
//     This function is to dump the SSP register.          
//         
// Return Value:
//              None.
//------------------------------------------------------------------------------
static void DumpSSPRegisters(void)
{   
    OALMSGS(1, (TEXT("DumpSSPRegisters ++ \r\n")));
    OALMSGS(1, (TEXT("SSP: SSP_CTRL0        = 0x%x\r\n"),  HW_SSP_CTRL0_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_CTRL1        = 0x%x\r\n"),  HW_SSP_CTRL1_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_CMD0         = 0x%x\r\n"),  HW_SSP_CMD0_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_CMD1         = 0x%x\r\n"),  HW_SSP_CMD1_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_COMPREF      = 0x%x\r\n"),  HW_SSP_COMPREF_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_COMPMASK     = 0x%x\r\n"),  HW_SSP_COMPMASK_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_TIMING       = 0x%x\r\n"),  HW_SSP_TIMING_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_DATA         = 0x%x\r\n"),  HW_SSP_DATA_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_SDRESP0      = 0x%x\r\n"),  HW_SSP_SDRESP0_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_SDRESP1      = 0x%x\r\n"),  HW_SSP_SDRESP1_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_SDRESP2      = 0x%x\r\n"),  HW_SSP_SDRESP2_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_SDRESP3      = 0x%x\r\n"),  HW_SSP_SDRESP3_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("SSP: SSP_STATUS       = 0x%x\r\n"),  HW_SSP_STATUS_RD(SPI_NOR_SSP2)));
    OALMSGS(1, (TEXT("DumpSSPRegisters -- \r\n")));
}
//------------------------------------------------------------------------------

