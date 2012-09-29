//---------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bspspi.c
//
//  Provides BSP-specific configuration routines for the SPI peripheral.
//  On Mx28,using SPI NOR flash as the client device,index = 2.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "bsp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
// Global Variables

// Defines

#define SPI_MAX_DIV_RATE             9 // 2^9= 512

#define DBGMSGON 0

//-----------------------------------------------------------------------------
// Local Variables
//-----------------------------------------------------------------------------
//
// Function: BSPSPISetIOMux
//
// This is a private function which request IOMUX for 
// the corresponding SPI Bus.
//
// Parameters:
//      Index
//          [in] spi port to be configured.
//
// Returns:
//      TRUE if successful, FALSE otherwise.
//-----------------------------------------------------------------------------
BOOL BSPSPISetIOMux(UINT32 Index)
{
    BOOL rc = FALSE;

	switch(Index)
	{
#ifdef	EM9280
	case 0:			// -> SSP0
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D0,  DDK_IOMUX_MODE_00);		//-> SPI0_MISO
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_CMD, DDK_IOMUX_MODE_00);		//-> SPI0_MOSI
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_SCK, DDK_IOMUX_MODE_00);		//-> SPI0_SCLK
        DDKIomuxSetPinMux(DDK_IOMUX_GPIO0_17, DDK_IOMUX_MODE_GPIO);		//-> SPI0_CS0N
        DDKIomuxSetPinMux(DDK_IOMUX_GPIO0_21, DDK_IOMUX_MODE_GPIO);		//-> SPI0_CS1N
        DDKIomuxSetPinMux(DDK_IOMUX_GPIO0_28, DDK_IOMUX_MODE_GPIO);		//-> SPI0_CS2N

        //Configure SSP pins data+clk+cmd for 8mA drive strength, 3 volts.
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_D0,  DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_CMD, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP0_SCK, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		//SPI_CSxN 
        DDKIomuxSetPadConfig(DDK_IOMUX_GPIO0_17, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_GPIO0_21, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_GPIO0_28, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

		DDKGpioEnableDataPin(DDK_IOMUX_GPIO0_17, 1);		// output enable
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO0_21, 1);		// output enable
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO0_28, 1);		// output enable

		DDKGpioWriteDataPin(DDK_IOMUX_GPIO0_17, 1);			// SPI_CS0N -> "1"
		DDKGpioWriteDataPin(DDK_IOMUX_GPIO0_21, 1);			// SPI_CS0N -> "1"
		DDKGpioWriteDataPin(DDK_IOMUX_GPIO0_28, 1);			// SPI_CS0N -> "1"
		rc = TRUE;
		break;
#endif	//EM9280

	case 2:			// -> SSP2
#ifdef EM9280 // zxw JUN05-2012
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D3_0,DDK_IOMUX_MODE_01);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D0_0,DDK_IOMUX_MODE_01);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_CMD_0,DDK_IOMUX_MODE_01);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_SCK_0,DDK_IOMUX_MODE_01);        
        //Configure SSP pins data+clk+cmd for 8mA drive strength, 3 volts.
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_CMD_0,DDK_IOMUX_PAD_DRIVE_8MA,DDK_IOMUX_PAD_PULL_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_D0_0,DDK_IOMUX_PAD_DRIVE_8MA,DDK_IOMUX_PAD_PULL_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_D3_0,DDK_IOMUX_PAD_DRIVE_8MA,DDK_IOMUX_PAD_PULL_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_SCK_0,DDK_IOMUX_PAD_DRIVE_8MA,DDK_IOMUX_PAD_PULL_ENABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        rc = TRUE;
#else // IMX28 EVB
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D3_1,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D0_1,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_CMD_1,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_SCK_1,DDK_IOMUX_MODE_00);        
        //Configure SSP pins data+clk+cmd for 8mA drive strength, 3 volts.
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_CMD_1,DDK_IOMUX_PAD_DRIVE_8MA,0,1);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_D0_1,DDK_IOMUX_PAD_DRIVE_8MA,0,1);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_D3_1,DDK_IOMUX_PAD_DRIVE_8MA,0,1);
        DDKIomuxSetPadConfig(DDK_IOMUX_SSP2_SCK_1,DDK_IOMUX_PAD_DRIVE_8MA,0,1);
        rc = TRUE;
#endif   //EM9280
		break;
	}
    
    return rc;
}
//-----------------------------------------------------------------------------
//
// Function: ReleaseIOMux
//
// This is a private function which releases the 
// IOMUX pins selected for the SPI bus.
//
// Parameters:
//      Index
//          [in] spi port to be released.
//
// Returns:
//      TRUE if successful, FALSE otherwise.
//-----------------------------------------------------------------------------
BOOL BSPSPIReleaseIOMux(UINT32 Index)
{
    BOOL rc = FALSE;

	switch(Index)
	{
#ifdef	EM9280
	case 0:			// -> SSP0
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_D0,  DDK_IOMUX_MODE_GPIO);		//-> SPI0_MISO
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_CMD, DDK_IOMUX_MODE_GPIO);		//-> SPI0_MOSI
        DDKIomuxSetPinMux(DDK_IOMUX_SSP0_SCK, DDK_IOMUX_MODE_GPIO);		//-> SPI0_SCLK

        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_D0,  0);					// output disable
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_CMD, 0);					// output disable
        DDKGpioEnableDataPin(DDK_IOMUX_SSP0_SCK, 0);					// output disable
		rc = TRUE;    
		break;
#endif	//EM9280

	case 2:			// -> SSP2
#ifdef EM9280 // zxw
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D3_0,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D0_0,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_CMD_0,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_SCK_0,DDK_IOMUX_MODE_GPIO);
        
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_D3_0,0);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_D0_0,0);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_CMD_0,0);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_SCK_0,0);
        rc = TRUE;  
#else // IMX28 EVK
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D3_1,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D0_1,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_CMD_1,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_SCK_1,DDK_IOMUX_MODE_GPIO);
        
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_D3_1,0);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_D0_1,0);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_CMD_1,0);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_SCK_1,0);
        rc = TRUE;    
#endif	// EM9280
		break;
    }
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: BSPSpiIsDMAEnabled
//
// This function returns whether sdma is 
// enabled or diabled for a given spi index
//
// Parameters:
//        SPI Index
//
// Returns:
//      Whether DMA is Enabled for a given SPI index.
//
//-----------------------------------------------------------------------------
BOOL BSPSpiIsDMAEnabled(UINT8 Index)
{
    UNREFERENCED_PARAMETER(Index);
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: BSPSpiIsAllowPolling
//
// This function returns whether polling method is 
// allowed or not allowed for a given spi index
//
// Parameters:
//        SPI Index
//
// Returns:
//      Whether polling method is allowed for a given SPI index.
//
//-----------------------------------------------------------------------------
BOOL BSPSpiIsAllowPolling(UINT8 Index)
{
    UNREFERENCED_PARAMETER(Index);
    return FALSE;
}


BOOL BSPSpiCSEnable(DWORD dwCSIndex)
{
	BOOL	bRet = TRUE;

	switch(dwCSIndex)
	{
	case 0:
		DDKGpioWriteDataPin(DDK_IOMUX_GPIO0_17, 0);			// SPI_CS0N -> "0"
		break;

	case 1:
		DDKGpioWriteDataPin(DDK_IOMUX_GPIO0_21, 0);			// SPI_CS0N -> "0"
		break;

	case 2:
		DDKGpioWriteDataPin(DDK_IOMUX_GPIO0_28, 0);			// SPI_CS0N -> "0"
		break;

	default:
		DEBUGMSG(1, (TEXT("BSPSpiCSEnable: unknown CS index\r\n")));
		bRet = FALSE;
	}

	return bRet;
}


BOOL BSPSpiCSDisable(DWORD dwCSIndex)
{
	BOOL	bRet = TRUE;

	switch(dwCSIndex)
	{
	case 0:
		DDKGpioWriteDataPin(DDK_IOMUX_GPIO0_17, 1);			// SPI_CS0N -> "1"
		break;

	case 1:
		DDKGpioWriteDataPin(DDK_IOMUX_GPIO0_21, 1);			// SPI_CS0N -> "1"
		break;

	case 2:
		DDKGpioWriteDataPin(DDK_IOMUX_GPIO0_28, 1);			// SPI_CS0N -> "1"
		break;

	default:
		DEBUGMSG(1, (TEXT("BSPSpiCSEnable: unknown CS index\r\n")));
		bRet = FALSE;
	}

	return bRet;
}

// zxw 2012-6-27
#ifdef EM9280

PBYTE BSPSPIGetDataBuffer(LPVOID pBuf, DWORD dwLength)//读取数据BUFF指针地址
{
	PSPI_INFO pSpiInfo = (PSPI_INFO)pBuf;

	if(!pSpiInfo || (dwLength != sizeof(SPI_INFO)))
	{
		return NULL;
	}

	return pSpiInfo->pDatBuf;
}

DWORD BSPSPIGetDataLength(LPVOID pBuf, DWORD dwLength)//读取数据操作长度
{
	PSPI_INFO pSpiInfo = (PSPI_INFO)pBuf;

	if(!pSpiInfo || (dwLength != sizeof(SPI_INFO)))
	{
		return 0;
	}

	return pSpiInfo->dwDatLen;
}

BOOL BSPSPIGetLCS(LPVOID pBuf, DWORD dwLength) // 读取CS控制命令
{
	PSPI_INFO pSpiInfo = (PSPI_INFO)pBuf;

	if(!pSpiInfo || (dwLength != sizeof(SPI_INFO)))
	{
		return 0;
	}

	return pSpiInfo->bLastTime;			//bLockCS;
}

BYTE BSPSPIGetBitDataLength(LPVOID pBuf, DWORD dwLength)//读取数据位长度
{
	PSPI_INFO pSpiInfo = (PSPI_INFO)pBuf;

	if(!pSpiInfo || (dwLength != sizeof(SPI_INFO)))
	{
		return 0;
	}

	return pSpiInfo->BitCount;
}

#endif