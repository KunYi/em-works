//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
#define SERIAL_SDMA_RX_BUFFER_SIZE  0x400
#define SERIAL_SDMA_TX_BUFFER_SIZE  0x200

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
BOOL BSPUartConfigureGPIO(ULONG HWAddr)
{
    BOOL ret = TRUE;

    switch (HWAddr)
    {
    // AUART0
    case CSP_BASE_REG_PA_UARTAPP0:
        DDKIomuxSetPinMux(DDK_IOMUX_AUART0_TX, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART1_TX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART0_RX, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART0_RX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART0_RTS, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART0_RTS, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART0_CTS, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART0_CTS, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        break;

    // AUART1
    case CSP_BASE_REG_PA_UARTAPP1:
        DDKIomuxSetPinMux(DDK_IOMUX_AUART1_TX, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART1_TX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_RESERVED);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART1_RX, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART1_RX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART1_RTS, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART1_RTS, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART1_CTS, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART1_CTS, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        break;

    // AUART2
    case CSP_BASE_REG_PA_UARTAPP2:
        DDKIomuxSetPinMux(DDK_IOMUX_AUART2_TX_1, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART2_TX_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_RESERVED);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART2_RX_1, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART2_RX_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART2_RTS, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART2_RTS, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART1_CTS, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART1_CTS, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        break;

    // AUART3
    case CSP_BASE_REG_PA_UARTAPP3:
        DDKIomuxSetPinMux(DDK_IOMUX_AUART3_TX_1, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART3_TX_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_RESERVED);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART3_RX_1, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART3_RX_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART3_RTS, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART3_RTS, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART3_CTS, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART3_CTS, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        break;

    default:
        ret=FALSE;
        break;
    }

    return ret;
}

//-----------------------------------------------------------------------------
//
// Function: BSPSerGetDMARequest
//
// This function is a wrapper for Uart to find out the TX/RX SDMA channel number.
// The UARTs are identified based on the based address.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
//      reqRx
//          [out] the RX SDMA channel number to be used for this UART.
//      reqTx
//          [out] the TX SDMA channel number to be used for this UART.
// Returns:
//      TRUE if SDMA is to be used for this UART.
//
//-----------------------------------------------------------------------------
BOOL BSPSerGetDMARequest(ULONG HWAddr, UINT8* reqRx, UINT8* reqTx)
{
    switch (HWAddr)
    {
    case CSP_BASE_REG_PA_UARTAPP0:
        *reqRx = APBX_CHANNEL_AUART0_RX;
        *reqTx = APBX_CHANNEL_AUART0_TX;
        break;
    case CSP_BASE_REG_PA_UARTAPP1:
        *reqRx = APBX_CHANNEL_AUART1_RX;
        *reqTx = APBX_CHANNEL_AUART1_TX;
        break;
    case CSP_BASE_REG_PA_UARTAPP2:
        *reqRx = APBX_CHANNEL_AUART2_RX;
        *reqTx = APBX_CHANNEL_AUART2_TX;
        break;
    case CSP_BASE_REG_PA_UARTAPP3:
        *reqRx = APBX_CHANNEL_AUART3_RX;
        *reqTx = APBX_CHANNEL_AUART3_TX;
        break;
    default:
        return FALSE;
    }
    return TRUE;
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

//-----------------------------------------------------------------------------
//
// Function: BSPUartGetDMAIrq
//
// This function is a wrapper for Uart to find out the TX/RX SDMA IRQ.
// The UARTs are identified based on the based address.
//
// Parameters:
//      HWAddr
//          [IN] Physical IO address.
//
//      txDMAIrq
//          [out] the TX DMA IRQ.
//
//      rxDMAIrq
//          [out] the RX DMA IRQ
// Returns:
//
//-----------------------------------------------------------------------------
BOOL BSPUartGetDMAIrq(ULONG HWAddr, UINT32* txDMAIrq, UINT32 *rxDMAIrq)
{
    switch (HWAddr)
    {
    case CSP_BASE_REG_PA_UARTAPP0:
        *rxDMAIrq = IRQ_AUART0_RX_DMA;
        *txDMAIrq = IRQ_AUART0_TX_DMA;
        break;
    case CSP_BASE_REG_PA_UARTAPP1:
        *rxDMAIrq = IRQ_AUART1_RX_DMA;
        *txDMAIrq = IRQ_AUART1_TX_DMA;
        break;
    case CSP_BASE_REG_PA_UARTAPP2:
        *rxDMAIrq = IRQ_AUART2_RX_DMA;
        *txDMAIrq = IRQ_AUART2_TX_DMA;
        break;
    case CSP_BASE_REG_PA_UARTAPP3:
        *rxDMAIrq = IRQ_AUART3_RX_DMA;
        *txDMAIrq = IRQ_AUART3_TX_DMA;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: BSPUartGetIndex
//
// This function return Application UART Index.
//
// Parameters:
//     HWAddr
//          [IN] Physical IO address.
//
// Returns:
//     Index of AUART.
//
//-----------------------------------------------------------------------------
DWORD BSPUartGetIndex(ULONG HWAddr)
{
    DWORD dwIndex;
    switch (HWAddr)
    {
    case CSP_BASE_REG_PA_UARTAPP0:
        dwIndex = 0;
        break;
    case CSP_BASE_REG_PA_UARTAPP1:
        dwIndex = 1;
        break;
    case CSP_BASE_REG_PA_UARTAPP2:
        dwIndex = 2;
        break;
    case CSP_BASE_REG_PA_UARTAPP3:
        dwIndex = 3;
        break;
    default:
        dwIndex = 0xFF;
    }

    return dwIndex;
}

