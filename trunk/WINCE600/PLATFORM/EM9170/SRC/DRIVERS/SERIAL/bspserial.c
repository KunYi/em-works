//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  bspserial.c
//
//  Provides BSP-specific configuration routines for the UART peripheral.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "bsp.h"
#include "common_uart.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

#define BSP_SDMA_SUPPORT_UART1     TRUE
#define BSP_SDMA_SUPPORT_UART2     TRUE
#define BSP_SDMA_SUPPORT_UART3     TRUE
#define BSP_SDMA_SUPPORT_UART4     TRUE
#define BSP_SDMA_SUPPORT_UART5     TRUE

#define UART_MAX_DIV            7

// UART3 mux selection
#define     UART3_KPP_IOMUX         TRUE
#define     UART3_CSPI1_IOMUX       FALSE

// UART4 mux selection
#define     UART4_GPIO_IOMUX        TRUE
#define     UART4_KPP_IOMUX         FALSE
#define     UART4_LD_IOMUX          FALSE

// UART5 mux selection
#define     UART5_CSI_IOMUX         TRUE
#define     UART5_LB_IOMUX          FALSE

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: BSPUartCalRFDIV
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
UCHAR BSPUartCalRFDIV(ULONG* pRefFreq)
{
    UCHAR dwDivisor; // the data rate divisor
    UINT32 freq;

    *pRefFreq = UART_REF_FREQ;

    DDKClockGetFreq(DDK_CLOCK_SIGNAL_PER_UART, &freq);

    dwDivisor =(UCHAR) (freq / *pRefFreq);

    if ( (dwDivisor != 0) && ((freq / dwDivisor) < UART_REF_FREQ))
    {
        dwDivisor--;
       if (dwDivisor < 0)
           dwDivisor = 0;
    }

    if (dwDivisor == 0)
    {
        dwDivisor = 1;
    }
    else if (dwDivisor > UART_MAX_DIV)
    {
        dwDivisor = UART_MAX_DIV;
    }

    *pRefFreq = freq /dwDivisor;

    return dwDivisor;
}

//-----------------------------------------------------------------------------
//
// Function: BSPUartGetType
//
// This is a private function to get the UART type with specified Uart
// IO address.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
//      pType
//          [out] Serial device type (DCE/DTE).
//
// Returns:
//      corresponding uart type.
//
//-----------------------------------------------------------------------------
BOOL BSPUartGetType(ULONG HWAddr, uartType_c * pType)
{
    switch (HWAddr)
    {
    case CSP_BASE_REG_PA_UART1:
        *pType = DCE;
        return TRUE;
    case CSP_BASE_REG_PA_UART2:
//#ifdef UART1_USED
        *pType = DTE;
//#else
//        *pType = DCE;
//#endif
        return TRUE;
    case CSP_BASE_REG_PA_UART3:
        *pType = DTE;
        return TRUE;
    case CSP_BASE_REG_PA_UART4:
        *pType = DTE;
        return TRUE;
    case CSP_BASE_REG_PA_UART5:
        *pType = DTE;
        return TRUE;
    default:
        return FALSE;
    }
}

//-----------------------------------------------------------------------------
//
// Function: BSPUartEnableClock
//
// This function is a wrapper for Uart to enable/disable its clock using a valid
// CRM handle.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
//      bEnable
//          [in] TRUE if Uart Clock is to be enabled. FALSE if Uart Clock is
//                to be disabled.
//
// Returns:
//      TRUE if successfully performed the required action.
//
//-----------------------------------------------------------------------------
BOOL BSPUartEnableClock(ULONG HWAddr, BOOL bEnable)
{
    BOOL result = FALSE;
    DDK_CLOCK_GATE_INDEX cgIndex;

    switch (HWAddr)
    {
    case CSP_BASE_REG_PA_UART1:
        cgIndex = DDK_CLOCK_GATE_INDEX_UART1;
        break;
    case CSP_BASE_REG_PA_UART2:
        cgIndex = DDK_CLOCK_GATE_INDEX_UART2;
        break;
    case CSP_BASE_REG_PA_UART3:
        cgIndex = DDK_CLOCK_GATE_INDEX_UART3;
        break;
    case CSP_BASE_REG_PA_UART4:
        cgIndex = DDK_CLOCK_GATE_INDEX_UART4;
        break;
    case CSP_BASE_REG_PA_UART5:
        cgIndex = DDK_CLOCK_GATE_INDEX_UART5;
        break;
    default:
        return result;
    }

    if (bEnable)
    {
        result = DDKClockSetGatingMode(cgIndex, DDK_CLOCK_GATE_MODE_ENABLED);
    }
    else
    {
    
        result = DDKClockSetGatingMode(cgIndex, DDK_CLOCK_GATE_MODE_DISABLED);  
    }
    return result;
}

//-----------------------------------------------------------------------------
//
// Function: BSPUartConfigTranceiver
//
// This function is used to configure the Tranceiver.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
//      bEnable
//          [in] TRUE if tranceiver is to be enabled,
//                FALSE if tranceiver is to be disabled.
//               
//
// Returns:
//      TRUE if successfully performed the required action.
//
//-----------------------------------------------------------------------------
VOID BSPUartConfigTranceiver(ULONG HWAddr,BOOL bEnable)
{
    if(bEnable)
    {
        switch (HWAddr)
        {
        case CSP_BASE_REG_PA_UART1:
            break;

        case CSP_BASE_REG_PA_UART2:
            break;
            
        case CSP_BASE_REG_PA_UART3:
            break;
            
        case CSP_BASE_REG_PA_UART4:
            break;
            
        case CSP_BASE_REG_PA_UART5:
            break;
        }
    }
    else
    {
        switch (HWAddr)
        {
        case CSP_BASE_REG_PA_UART1:
            break;

        case CSP_BASE_REG_PA_UART2:
            break;

        case CSP_BASE_REG_PA_UART3:
            break;
            
        case CSP_BASE_REG_PA_UART4:
            break;
            
        case CSP_BASE_REG_PA_UART5:
            break;
        }
    }

}

//-----------------------------------------------------------------------------
//
// Function: BSPUartConfigureGPIO
//
// This function is used to configure the GPIO.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
//
// Returns:
//      TRUE if successfully performed the required action.
//
//-----------------------------------------------------------------------------
BOOL BSPUartConfigureGPIO( ULONG HWAddr )
{
    BOOL result = FALSE;

    switch (HWAddr)
    {
    case CSP_BASE_REG_PA_UART1:
#ifdef		EM9170
		//
		// CS&ZHL JUN-2-2011: only use RXD / TXD as debug port in EM9170
		//
		RETAILMSG(1, (TEXT("CS&ZHL::config UART1 pins\r\n")));
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_UART1_RXD, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_UART1_TXD, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
#else		// ->iMX257PDK
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_UART1_RXD, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_UART1_TXD, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_UART1_RTS, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_UART1_CTS, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);

        // DCE signals
        // UART1 DTR
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW0, DDK_IOMUX_PIN_MUXMODE_ALT4,DDK_IOMUX_PIN_SION_REGULAR);
        // UART1 DSR
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW1, DDK_IOMUX_PIN_MUXMODE_ALT4,DDK_IOMUX_PIN_SION_REGULAR);
        // UART1 DCD
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW2, DDK_IOMUX_PIN_MUXMODE_ALT4,DDK_IOMUX_PIN_SION_REGULAR);
        // UART1 RI
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW3, DDK_IOMUX_PIN_MUXMODE_ALT4,DDK_IOMUX_PIN_SION_REGULAR);
#endif		//EM9170
        break;
        
    case CSP_BASE_REG_PA_UART2:
		// CS&ZHL JUN-2-2011: EM9170 has the same config with iMX257PDK's
		RETAILMSG(1, (TEXT("CS&ZHL::config UART2 pins\r\n")));
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_UART2_RXD, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_UART2_TXD, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_UART2_RTS, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_UART2_CTS, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);

#ifdef  CSR_BLUETOOTH
        // BlueTooth reset.
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_D10, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D10, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKGpioSetConfig(DDK_GPIO_PORT4, 10, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
        DDKGpioWriteDataPin(DDK_GPIO_PORT4, 10, 0);
        Sleep(50);
        DDKGpioWriteDataPin(DDK_GPIO_PORT4, 10, 1);
#endif
        break;

    case CSP_BASE_REG_PA_UART3:
#ifdef		EM9170
		RETAILMSG(1, (TEXT("CS&ZHL::config UART3 pins\r\n")));
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_MOSI, DDK_IOMUX_PIN_MUXMODE_ALT2,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_MISO, DDK_IOMUX_PIN_MUXMODE_ALT2,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART3_IPP_UART_RXD_MUX, 0x0);
#else		// ->iMX257PDK
#if UART3_CSPI1_IOMUX
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_MOSI, DDK_IOMUX_PIN_MUXMODE_ALT2,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_MISO, DDK_IOMUX_PIN_MUXMODE_ALT2,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_SS1, DDK_IOMUX_PIN_MUXMODE_ALT2,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSPI1_SCLK, DDK_IOMUX_PIN_MUXMODE_ALT2,DDK_IOMUX_PIN_SION_REGULAR);

        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART3_IPP_UART_RXD_MUX, 0x0);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART3_IPP_UART_RTS_B, 0x0);
#elif UART3_KPP_IOMUX
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW0, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW1, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW2, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW3, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);

        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART3_IPP_UART_RXD_MUX, 0x1);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART3_IPP_UART_RTS_B, 0x1);
#endif
#endif		//EM9170
        break;

    case CSP_BASE_REG_PA_UART4:
#ifdef		EM9170
		RETAILMSG(1, (TEXT("CS&ZHL::config UART4 pins\r\n")));
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL0, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL1, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART4_IPP_UART_RXD_MUX, 0x1);
#else		// -> iMX257PDK
#if UART4_LD_IOMUX
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD8, DDK_IOMUX_PIN_MUXMODE_ALT2,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD9, DDK_IOMUX_PIN_MUXMODE_ALT2,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD10, DDK_IOMUX_PIN_MUXMODE_ALT2,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD11, DDK_IOMUX_PIN_MUXMODE_ALT2,DDK_IOMUX_PIN_SION_REGULAR);

        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART4_IPP_UART_RXD_MUX, 0x0);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART4_IPP_UART_RTS_B, 0x0);
#elif UART4_KPP_IOMUX
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL0, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL1, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL2, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL3, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);

        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART4_IPP_UART_RXD_MUX, 0x1);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART4_IPP_UART_RTS_B, 0x1);
#elif UART4_GPIO_IOMUX
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_E, DDK_IOMUX_PIN_MUXMODE_ALT6,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_F, DDK_IOMUX_PIN_MUXMODE_ALT6,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_VSTBY_REQ, DDK_IOMUX_PIN_MUXMODE_ALT6,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_POWER_FAIL, DDK_IOMUX_PIN_MUXMODE_ALT6,DDK_IOMUX_PIN_SION_REGULAR);

        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART4_IPP_UART_RXD_MUX, 0x2);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART4_IPP_UART_RTS_B, 0x2);
#endif
#endif		//EM9170
        break;

    case CSP_BASE_REG_PA_UART5:
#ifdef	EM9170
		RETAILMSG(1, (TEXT("CS&ZHL::config UART5 pins\r\n")));
		// CS&ZHL JUN-1-2011: no RTS/CTS in EM9170
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_LBA, DDK_IOMUX_PIN_MUXMODE_ALT3,DDK_IOMUX_PIN_SION_REGULAR);		//UART5_RXD
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_ECB, DDK_IOMUX_PIN_MUXMODE_ALT3,DDK_IOMUX_PIN_SION_REGULAR);	//UART5_TXD
		// select RXD source ->
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART5_IPP_UART_RXD_MUX, 0x0);
#else	// ->iMX257PDK
#if UART5_LB_IOMUX
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_LBA, DDK_IOMUX_PIN_MUXMODE_ALT3,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_ECB, DDK_IOMUX_PIN_MUXMODE_ALT3,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CS5, DDK_IOMUX_PIN_MUXMODE_ALT3,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CS4, DDK_IOMUX_PIN_MUXMODE_ALT3,DDK_IOMUX_PIN_SION_REGULAR);

        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART5_IPP_UART_RXD_MUX, 0x0);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART5_IPP_UART_RTS_B, 0x0);
#elif UART5_CSI_IOMUX
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D2, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D3, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D4, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_CSI_D5, DDK_IOMUX_PIN_MUXMODE_ALT1,DDK_IOMUX_PIN_SION_REGULAR);

        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART5_IPP_UART_RXD_MUX, 0x1);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_UART5_IPP_UART_RTS_B, 0x1);
#endif
#endif	//EM9170
        break;

    default:
        return result;
        
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: BSPSerGetChannelPriority
//
// This function is a wrapper for Uart to find out the SDMA channel priority.
//
// Parameters:
//
// Returns:
//      SDMA channel priority for Serial.
//
//-----------------------------------------------------------------------------
UINT8 BSPSerGetChannelPriority()
{
    return BSP_SDMA_CHNPRI_SERIAL;
}

//-----------------------------------------------------------------------------
//
// Function: BSPSerGetDMAIsEnabled
//
// This function is a wrapper for Uart to find out if SDMA is to be used or not.
// The UARTs are identified based on the based address.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
// Returns:
//      TRUE if SDMA is to be used for this UART.
//
//-----------------------------------------------------------------------------
BOOL BSPSerGetDMAIsEnabled(ULONG HWAddr)
{
    BOOL useDMA = FALSE;
    switch (HWAddr) {
        case CSP_BASE_REG_PA_UART1:
#if BSP_SDMA_SUPPORT_UART1
            useDMA = TRUE;
#endif
            break;
        case CSP_BASE_REG_PA_UART2:
#if BSP_SDMA_SUPPORT_UART2
            useDMA = TRUE;
#endif
            break;
        case CSP_BASE_REG_PA_UART3:
#if BSP_SDMA_SUPPORT_UART3
            useDMA = TRUE;
#endif
            break;
        case CSP_BASE_REG_PA_UART4:
#if BSP_SDMA_SUPPORT_UART4
            useDMA = TRUE;
#endif
            break;
        case CSP_BASE_REG_PA_UART5:
#if BSP_SDMA_SUPPORT_UART5
            useDMA = TRUE;
#endif
            break;
        default:
            break;
    }
    return useDMA;
}

//-----------------------------------------------------------------------------
//
// Function: BSPSerGetDMARequest
//
// This function is a wrapper for Uart to find out the TX/RX SDMA request line.
// The UARTs are identified based on the based address.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
//      reqRx
//          [out] the RX SDMA request line to be used for this UART.
//      reqTx
//          [out] the TX SDMA request line to be used for this UART.
// Returns:
//      TRUE if SDMA is to be used for this UART.
//
//-----------------------------------------------------------------------------
BOOL BSPSerGetDMARequest(ULONG HWAddr, int* reqRx, int* reqTx)
{
    switch (HWAddr)
    {
    case CSP_BASE_REG_PA_UART1:
        *reqRx = DDK_DMA_REQ_UART1_RX;
        *reqTx = DDK_DMA_REQ_UART1_TX;
        break;
    case CSP_BASE_REG_PA_UART2:
        *reqRx = DDK_DMA_REQ_UART2_RX;
        *reqTx = DDK_DMA_REQ_UART2_TX;
        break;
    case CSP_BASE_REG_PA_UART3:
        *reqRx = DDK_DMA_REQ_UART3_RX;
        *reqTx = DDK_DMA_REQ_UART3_TX;
        break;
    case CSP_BASE_REG_PA_UART4:
        *reqRx = DDK_DMA_REQ_UART4_RX;
        *reqTx = DDK_DMA_REQ_UART4_TX;
        break;
    case CSP_BASE_REG_PA_UART5:
        *reqRx = DDK_DMA_REQ_UART5_RX;
        *reqTx = DDK_DMA_REQ_UART5_TX;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: BSPSerSetDMAReqGpr
//
// This function is a wrapper for Uart to acquire a muxed SDMA request line.
// The UARTs are identified based on the based address.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
// Returns:
//      TRUE if success.
//
//-----------------------------------------------------------------------------
BOOL BSPSerAcquireDMAReqGpr(ULONG HWAddr)
{
    BOOL bRet = TRUE;
    UNREFERENCED_PARAMETER(HWAddr);

    return bRet;
}

//-----------------------------------------------------------------------------
//
// Function: BSPSerRestoreDMAReqGpr
//
// This function is a wrapper for Uart to restore a muxed SDMA request line.
// The UARTs are identified based on the based address.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
// Returns:
//      TRUE if success.
//
//-----------------------------------------------------------------------------
BOOL BSPSerRestoreDMAReqGpr(ULONG HWAddr)
{
    BOOL bRet = TRUE;
    UNREFERENCED_PARAMETER(HWAddr);

    return bRet;
}

//-----------------------------------------------------------------------------
//
//  Function: GetSdmaChannelIRQ
//
//  This function returns the IRQ number of the specified SDMA channel.
//
//  Parameters:
//      chan
//          [in] The SDMA channel.
//
//  Returns:
//      IRQ number.
//
//-----------------------------------------------------------------------------
DWORD GetSdmaChannelIRQ(UINT32 chan)
{
    return (IRQ_SDMA_CH0 + chan);
}

//-----------------------------------------------------------------------------
//
// Function: BSPGetDMABuffSize
//
// This function is a wrapper for Uart to find out the TX/RX SDMA.
// The UARTs are identified based on the based address.
//
// Parameters:
//      buffRx
//          [out] the RX SDMA buffer size.
//
//      buffTx
//          [out] the RX SDMA buffer size 
// Returns:
//      
//
//-----------------------------------------------------------------------------
VOID BSPGetDMABuffSize(UINT16* buffRx, UINT16 * buffTx)
{
    *buffRx = SERIAL_SDMA_RX_BUFFER_SIZE;
    *buffTx = SERIAL_SDMA_TX_BUFFER_SIZE;
}
