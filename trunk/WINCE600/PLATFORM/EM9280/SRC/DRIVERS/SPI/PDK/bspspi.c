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

    if(Index == 2)
    {
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

    if(Index == 2)
    {
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D3_1,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_D0_1,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_CMD_1,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_SSP2_SCK_1,DDK_IOMUX_MODE_GPIO);
        
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_D3_1,0);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_D0_1,0);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_CMD_1,0);
        DDKGpioEnableDataPin(DDK_IOMUX_SSP2_SCK_1,0);
        rc = TRUE;    
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
