//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  bspcspi.c
//
//  Provides BSP-specific configuration routines for the CSPI peripheral.
//
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
// Defines
#ifndef BSP_SDMA_CHNPRI_CSPI_TX    
//#define BSP_SDMA_CHNPRI_CSPI_TX    (SDMA_CHNPRI_CHNPRI_HIGHEST-3)
#define BSP_SDMA_CHNPRI_CSPI_TX    (SDMA_CHNPRI_CHNPRI_HIGHEST-1)
#endif

#ifndef BSP_SDMA_CHNPRI_CSPI_RX    
//#define BSP_SDMA_CHNPRI_CSPI_RX    (SDMA_CHNPRI_CHNPRI_HIGHEST-2)
#define BSP_SDMA_CHNPRI_CSPI_RX    (SDMA_CHNPRI_CHNPRI_HIGHEST)
#endif

#ifndef BSP_SDMA_SUPPORT_CSPI1
#define BSP_SDMA_SUPPORT_CSPI1        FALSE
#endif

#ifndef BSP_SDMA_SUPPORT_CSPI2
#define BSP_SDMA_SUPPORT_CSPI2        FALSE
#endif

#ifndef BSP_SDMA_SUPPORT_CSPI3
#define BSP_SDMA_SUPPORT_CSPI3        FALSE
#endif

#ifndef BSP_ALLOW_POLLING_CSPI1
#define BSP_ALLOW_POLLING_CSPI1       TRUE
#endif

#ifndef BSP_ALLOW_POLLING_CSPI2
#define BSP_ALLOW_POLLING_CSPI2       FALSE
#endif

#ifndef BSP_ALLOW_POLLING_CSPI3
#define BSP_ALLOW_POLLING_CSPI3       FALSE
#endif

#define CSPI_MAX_DIV_RATE             9 // 2^9= 512

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
//
// Function: CalculateDivRate
//
// This is a private function to calculate the data
// rate divider from input frequency.
//
// Parameters:
//      dwFrequency
//          [in] Frequency requested.
//
// Returns:
//      Data rate divisor for requested frequency.
//-----------------------------------------------------------------------------
UINT32 BSPCSPICalculateDivRate(UINT32 dwFrequency, UINT32 dwTolerance)
{
    UINT32 dwDivisor;
    UINT32 dwCspiClk;

    DDKClockGetFreq(DDK_CLOCK_SIGNAL_IPG, &dwCspiClk);

    for (dwDivisor = 2; dwDivisor < CSPI_MAX_DIV_RATE; dwDivisor++)
    {
        if ((dwCspiClk>>dwDivisor) <= dwFrequency+dwTolerance)
        {
            break;
        }
    }

    return (dwDivisor - 2);

}
//-----------------------------------------------------------------------------
//
// Function: SetIOMux
//
// This is a private function which request IOMUX for 
// the corresponding CSPI Bus.
//
// Parameters:
//      dwIndex
//          [in] cspi port to be configured.
//
// Returns:
//      TRUE if successful, FALSE otherwise.
//-----------------------------------------------------------------------------
BOOL BSPCSPISetIOMux(UINT32 dwIndex)
{
    BOOL rc = FALSE;

    if(dwIndex == 1)
    {
        // Configure IOMUX to request CSPI1 pins
        // SPI1_RDY
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_RDY, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSPI1_RDY, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

        // SPI1_SCLK
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_SCLK, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSPI1_SCLK, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

        // SPI1_SS1
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_SS1, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSPI1_SS1, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

        // SPI1_SS0
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_SS0, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSPI1_SS0, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

        // SPI1_MISO
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_MISO, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSPI1_MISO, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

        // SPI1_MOSI
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_MOSI, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CSPI1_MOSI, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

        rc = TRUE;
    }
    else if(dwIndex == 2)
    {
        // CSPI2 is not used on this board
        rc = FALSE;
    }
    else if(dwIndex == 3)
    {
		// Aug 2, 2011 _LQK
		// Configure IOMUX to request CSPI3 pins for EM9170
		// SPI3_RDY->GPIO8
		//DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D5, DDK_IOMUX_PIN_MUXMODE_ALT7, DDK_IOMUX_PIN_SION_REGULAR);
		//DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D5, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		//DDKIomuxSelectInput(  DDK_IOMUX_SELEIN_CSPI3_IPP_IND_DATAREADY_B, 1 );

		// SPI3_SCLK->GPIO2
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D4, DDK_IOMUX_PIN_MUXMODE_ALT7, DDK_IOMUX_PIN_SION_REGULAR);
		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D4, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKIomuxSelectInput( DDK_IOMUX_SELEIN_CSPI3_IPP_CSPI_CLK_IN, 1 );


		// SPI3_SS0->GPIO4
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D6, DDK_IOMUX_PIN_MUXMODE_ALT7, DDK_IOMUX_PIN_SION_REGULAR);
		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D6, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKIomuxSelectInput(DDK_IOMUX_SELEIN_CSPI3_IPP_IND_SS0_B ,1 );

		// SPI3_MOSI->GPIO3
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D2, DDK_IOMUX_PIN_MUXMODE_ALT7, DDK_IOMUX_PIN_SION_REGULAR);
		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D2, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKIomuxSelectInput( DDK_IOMUX_SELEIN_CSPI3_IPP_IND_MOSI, 1 );

		// SPI3_MISO->GPIO5
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D3, DDK_IOMUX_PIN_MUXMODE_ALT7, DDK_IOMUX_PIN_SION_REGULAR);
		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D3, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKIomuxSelectInput(  DDK_IOMUX_SELEIN_CSPI3_IPP_IND_MISO, 1 );
		
		rc = TRUE;
    }

    return rc;
}
//lqk Sep 13, 2011
//
void BSPCSPICSSet(UINT32 dwIndex, BOOL bVal)
{
	switch( dwIndex )
	{
	case 1: break;
	case 2: break;
	case 3: 
		if( bVal )
		{
			//high level
			DDKGpioWriteDataPin(DDK_GPIO_PORT1, 31, 1);
		}
		else
		{
			//low level
			DDKGpioWriteDataPin(DDK_GPIO_PORT1, 31, 0);
		}
		break;
	default:;
	}

}

void BSPCSPIRDYSet( UINT32 dwIndex, BOOL bVal )
{
	switch( dwIndex )
	{
	case 1: break;
	case 2: break;
	case 3: 
		if( bVal )
		{
			//high level
			DDKGpioWriteDataPin(DDK_GPIO_PORT1, 30, 1);
		}
		else
		{
			//low level
			DDKGpioWriteDataPin(DDK_GPIO_PORT1, 30, 0);
		}
		break;
	default:;
	}
}

void BSPCSPICS2IO( UINT32 dwIndex )
{
	switch( dwIndex )
	{
	case 1: break;
	case 2: break;
	case 3: 
		// assign SPI3_SS0(SPI_CS) as GPIO
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D6, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D6, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKGpioSetConfig( DDK_GPIO_PORT1, 31, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE );
		break;
	default:;
	}
}

void BAPCSPIRDY2IO( UINT32 dwIndex )
{
	switch( dwIndex )
	{
	case 1: break;
	case 2: break;
	case 3: 
		//define SPI_RDY  as Master/Slave select signal
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D5, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D5, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKGpioSetConfig( DDK_GPIO_PORT1, 30, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE );
		break;
	default:;
	}
}

//-----------------------------------------------------------------------------
//
// Function: ReleaseIOMux
//
// This is a private function which releases the 
// IOMUX pins selected for the CSPI bus.
//
// Parameters:
//      dwIndex
//          [in] cspi port to be released.
//
// Returns:
//      TRUE if successful, FALSE otherwise.
//-----------------------------------------------------------------------------
BOOL BSPCSPIReleaseIOMux(UINT32 dwIndex)
{
    // Remove-W4: Warning C4100 workaround
    //UNREFERENCED_PARAMETER(dwIndex);
	switch( dwIndex )
	{
	case 1: break;
	case 2: break;
	case 3:
		//define SPI_RDY  as Master/Slave select signal
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D5, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D5, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKGpioSetConfig( DDK_GPIO_PORT1, 30, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE );

		// SPI3_SCLK->ALT6->GPIO1->GPIO[29]
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D4, DDK_IOMUX_PIN_MUXMODE_ALT6, DDK_IOMUX_PIN_SION_REGULAR);
		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D4, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKGpioSetConfig( DDK_GPIO_PORT1, 29, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE );

		// SPI3_SS0->ALT5->GPIO1->GPIO[31]
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D6, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D6, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKGpioSetConfig( DDK_GPIO_PORT1, 31, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE );

		// SPI3_MOSI->ALT5->GPIO1->GPIO[27]
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D2, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D2, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKGpioSetConfig( DDK_GPIO_PORT1, 27, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE );

		// SPI3_MISO->ALT5->GPIO1->GPIO[28]
		DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D3, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_CSI_D3, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		DDKGpioSetConfig( DDK_GPIO_PORT1, 28, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE );
		break;
	default:;
	}

    return TRUE;    
}
//-----------------------------------------------------------------------------
//
// Function: EnableClock
//
// Provides the platform-specific SPI clock gating control to enable/disable 
// module clocks.
//
// Parameters:
//      Index
//          [in] cspi port to be configured.
//      bEnable
//          [in] Set to TRUE to enable CSPI clocks, set to FALSE to disable
//          CSPI clocks.
//
// Returns:
//      TRUE if successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
BOOL BSPCSPIEnableClock(UINT32 Index, BOOL bEnable)
{
    BOOL rc = FALSE;

    DDK_CLOCK_GATE_MODE mode = bEnable ? 
        DDK_CLOCK_GATE_MODE_ENABLED : DDK_CLOCK_GATE_MODE_DISABLED;
    switch(Index)
    {
        case 1: 
            rc = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_CSPI1, mode); 
            break;
        case 2: 
            rc = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_CSPI2, mode); 
            break;
        case 3: 
            rc = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_CSPI3, mode); 
            break;
        default:
            ERRORMSG(TRUE, (TEXT("Invalid Index\r\n"))); 
    }

    return rc;
}
//-----------------------------------------------------------------------------
//
// Function: BSPCspiGetChannelPriority
//
// This function returns the sdma priority for cspi
//
// Parameters:
//        None
//
// Returns:
//      The channel Priority.
//
//-----------------------------------------------------------------------------
BOOL BSPCspiGetChannelPriority(UINT8 (*priority)[2])
{
    (*priority)[0] =BSP_SDMA_CHNPRI_CSPI_TX;
    (*priority)[1] =BSP_SDMA_CHNPRI_CSPI_RX;
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: BSPCspiIsDMAEnabled
//
// This function returns whether sdma is 
// enabled or diabled for a given cspi index
//
// Parameters:
//        CSPI Index
//
// Returns:
//      Whether DMA is Enabled for a given CSPI index.
//
//-----------------------------------------------------------------------------
BOOL BSPCspiIsDMAEnabled(UINT8 Index)
{
    switch (Index)
    {
        case 1:
            return BSP_SDMA_SUPPORT_CSPI1;
        case 2:
            return BSP_SDMA_SUPPORT_CSPI2;
        case 3:
            return BSP_SDMA_SUPPORT_CSPI3;
        default:
            ERRORMSG(TRUE, (TEXT("Invalid Index\r\n"))); 
    }
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: BSPCspiAcquireGprBit
//
// This function is a wrapper for CSPI to acquire a muxed SDMA request line.
//
// Parameters:
//      Index
//          [in] Index of the CSPI 
// Returns:
//      TRUE if success.
//
//-----------------------------------------------------------------------------
BOOL BSPCspiAcquireGprBit(UINT8 Index)
{
    switch(Index)
    {
        case 1:
        case 2:
        case 3:
            return TRUE;
        default:
            ERRORMSG(TRUE, (TEXT("Invalid Index\r\n"))); 
    }
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: BSPCspiIsAllowPolling
//
// This function returns whether polling method is 
// allowed or not allowed for a given cspi index
//
// Parameters:
//        CSPI Index
//
// Returns:
//      Whether polling method is allowed for a given CSPI index.
//
//-----------------------------------------------------------------------------
BOOL BSPCspiIsAllowPolling(UINT8 Index)
{
    switch (Index)
    {
        case 1:
            return BSP_ALLOW_POLLING_CSPI1;
        case 2:
            return BSP_ALLOW_POLLING_CSPI2;
        case 3:
            return BSP_ALLOW_POLLING_CSPI3;
        default:
            ERRORMSG(TRUE, (TEXT("Invalid Index\r\n"))); 
    }
    return FALSE;
}


//-----------------------------------------------------------------------------
//
// Function: BSPCspiExchange
//
// This function exchanges eCSPI data
//
// Parameters:
//        
//
// Returns:
//      
//
//-----------------------------------------------------------------------------
BOOL BSPCspiExchange(VOID *lpInBuf, LPVOID pRxBuf, UINT32 nOutBufSize, LPDWORD BytesReturned)
{
    UINT32 *p;
    p=(PUINT32) lpInBuf;

    return KernelIoControl(IOCTL_HAL_SHARED_CSPI_TRANSFER, lpInBuf, 3*sizeof(UINT32), pRxBuf, nOutBufSize, BytesReturned);
}

//------------------------------------------------------------------------------
//
// Function: BSPCheckPort
//
//  This function checks if the cspi port is used for NIC card
//
// Parameters:
//      None.
//
//  Returns:
//      TRUE - The port is Marley eCSPI2 port
//
//      FALSE - The port is not Marley eCSPI2 port
//
//------------------------------------------------------------------------------
BOOL BSPCheckPort(UINT32 Index)
{
#ifdef	BSP_CPLD_CSPI
    if( BSP_CPLD_CSPI==Index )
    {
        //JJH please note that in case the debug board is not used, we do'nt really need this.
        // so maybe we could later replace if( BSP_CPLD_CSPI==Index ) with if( (BSP_CPLD_CSPI==Index ) && (g_fDebugBoardIsUsed))

        return TRUE;
    }
#else
    UNREFERENCED_PARAMETER(Index);
#endif	//BSP_CPLD_CSPI

    return FALSE;
}
