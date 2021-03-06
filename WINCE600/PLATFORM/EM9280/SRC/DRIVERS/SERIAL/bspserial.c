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
//-----------------------------------------------------
// LQK NOV-2-2012: supporting GPIO based RTSn
//-----------------------------------------------------
#ifdef EM9283
static DDK_IOMUX_PIN g_EM9283_iMXGpioPin[32] =
{
	DDK_IOMUX_GPIO3_20,			// EM9283_GPIO0
	DDK_IOMUX_GPIO3_21,			// EM9283_GPIO1
	DDK_IOMUX_GPIO1_8,			// EM9283_GPIO2
	DDK_IOMUX_GPIO1_9,			// EM9283_GPIO3
	DDK_IOMUX_GPIO1_16,			// EM9283_GPIO4
	DDK_IOMUX_GPIO1_17,			// EM9283_GPIO5
	DDK_IOMUX_GPIO2_16,			// EM9283_GPIO6
	DDK_IOMUX_GPIO2_17,			// EM9283_GPIO7
	DDK_IOMUX_GPIO2_18,			// EM9283_GPIO8
	DDK_IOMUX_GPIO2_19,			// EM9283_GPIO9
	DDK_IOMUX_GPIO3_24,			// EM9283_GPIO10
	DDK_IOMUX_GPIO3_25,			// EM9283_GPIO11
	DDK_IOMUX_GPIO2_4,          // EM9283_GPIO12
	DDK_IOMUX_GPIO2_6,			// EM9283_GPIO13
	DDK_IOMUX_GPIO2_7,			// EM9283_GPIO14
	DDK_IOMUX_GPIO2_5,			// EM9283_GPIO15
	DDK_IOMUX_GPIO4_7,			// EM9283_GPIO16
	DDK_IOMUX_GPIO4_3,			// EM9283_GPIO17
	DDK_IOMUX_GPIO4_8,			// EM9283_GPIO18
	DDK_IOMUX_GPIO4_4,			// EM9283_GPIO19
	DDK_IOMUX_GPIO2_9,			// EM9283_GPIO20
	DDK_IOMUX_GPIO4_16,			// EM9283_GPIO21
	DDK_IOMUX_GPIO2_8,			// EM9283_GPIO22
	DDK_IOMUX_GPIO2_10,			// EM9283_GPIO23
	DDK_IOMUX_GPIO2_0,          // EM9283_GPIO24
	DDK_IOMUX_GPIO2_1,			// EM9283_GPIO25
	DDK_IOMUX_GPIO2_2,			// EM9283_GPIO26
	DDK_IOMUX_GPIO2_3,			// EM9283_GPIO27
	DDK_IOMUX_GPIO3_23,         // EM9283_GPIO28 : SEP21-2012:DDK_IOMUX_GPIO3_22
	DDK_IOMUX_GPIO3_22,			// EM9283_GPIO29 : SEP21-2012:DDK_IOMUX_GPIO3_23
	DDK_IOMUX_GPIO3_28,			// EM9283_GPIO30
	DDK_IOMUX_GPIO3_26,			// EM9283_GPIO31 : SEP21-2012:DDK_IOMUX_GPIO3_29
};
#endif // #ifdef EM9283
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
		// CS&ZHL MAY-7-2012: UART0 as COM3
        DDKIomuxSetPinMux(DDK_IOMUX_AUART0_TX, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART0_TX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

		// GPIO3_0 output disable -> input only
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_0, 0);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART0_RX, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART0_RX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
#if (defined EM9280 || defined EM9283 )
		RETAILMSG(1, (TEXT("BSPUartConfigureGPIO:: setup UART0\r\n")));
		// NO RTS/CTS in EM9280
#else	// -> iMX28EVK
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
#endif	//EM9280
        break;

    // AUART1
    case CSP_BASE_REG_PA_UARTAPP1:
		// CS&ZHL MAY-7-2012: UART1 as COM4
        DDKIomuxSetPinMux(DDK_IOMUX_AUART1_TX, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART1_TX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

		// GPIO3_4 output disable -> input only
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_4, 0);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART1_RX, DDK_IOMUX_MODE_00);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART1_RX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

#if (defined EM9280 || defined EM9283 )
		RETAILMSG(1, (TEXT("BSPUartConfigureGPIO:: setup UART1\r\n")));
		// NO RTS/CTS in EM9280
#else	// -> iMX28EVK
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
#endif	//EM9280
        break;

    // AUART2
    case CSP_BASE_REG_PA_UARTAPP2:
		// CS&ZHL MAY-7-2012: UART2 as COM5
#if (defined EM9280 || defined EM9283 )
		RETAILMSG(1, (TEXT("BSPUartConfigureGPIO:: setup UART2\r\n")));
        DDKIomuxSetPinMux(DDK_IOMUX_AUART2_TX_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART2_TX_0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

		// GPIO2_16 output disable -> input only
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO2_16, 0);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART2_RX_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART2_RX_0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
#else	// -> iMX28EVK
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
#endif	//EM9280
        break;

    // AUART3
    case CSP_BASE_REG_PA_UARTAPP3:
		// CS&ZHL MAY-7-2012: UART3 as COM6
#if (defined EM9280 || defined EM9283 )
		RETAILMSG(1, (TEXT("BSPUartConfigureGPIO:: setup UART3\r\n")));
        DDKIomuxSetPinMux(DDK_IOMUX_AUART3_TX_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART3_TX_0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

		// GPIO2_18 output disable -> input only
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO2_18, 0);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART3_RX_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART3_RX_0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
#else	// -> iMX28EVK
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
#endif	//EM9280
        break;

	case CSP_BASE_REG_PA_UARTAPP4:
		// CS&ZHL MAY-7-2012: UART4 as COM2
#if (defined EM9280 || defined EM9283 )
		RETAILMSG(1, (TEXT("BSPUartConfigureGPIO:: setup UART4\r\n")));
        DDKIomuxSetPinMux(DDK_IOMUX_AUART4_TX_1, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART4_TX_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

		// GPIO3_2 output disable -> input only
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_2, 0);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART4_RX_1, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART4_RX_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
							 DDK_IOMUX_PAD_VOLTAGE_3V3);

        // CS&ZHL JUN-14-2012: RTS/CTS handshake pins is configured later if required
		//DDKIomuxSetPinMux(DDK_IOMUX_AUART4_RTS_1, DDK_IOMUX_MODE_02);
        //DDKIomuxSetPadConfig(DDK_IOMUX_AUART4_RTS_1, 
        //                     DDK_IOMUX_PAD_DRIVE_8MA, 
        //                     DDK_IOMUX_PAD_PULL_ENABLE,
        //                     DDK_IOMUX_PAD_VOLTAGE_3V3);

        //DDKIomuxSetPinMux(DDK_IOMUX_AUART4_CTS_1, DDK_IOMUX_MODE_02);
        //DDKIomuxSetPadConfig(DDK_IOMUX_AUART4_CTS_1, 
        //                     DDK_IOMUX_PAD_DRIVE_8MA, 
        //                     DDK_IOMUX_PAD_PULL_ENABLE,
        //                     DDK_IOMUX_PAD_VOLTAGE_3V3);
#else	// -> iMX28EVK
        ret=FALSE;
#endif	//EM9280
		break;

    default:
        ret=FALSE;
        break;
    }

    return ret;
}

//-----------------------------------------------------------------------------
// CS&ZHL JUN-13-2012: config UART pin into GPIO input mode with pull-up
//
// Function: BSPUart2GPIOInput
//
// This function is used to configure UART pin to GPIO input with pull-up.
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
//
// Returns:
//      TRUE if successfully performed the required action.
//
//-----------------------------------------------------------------------------
BOOL BSPUart2GPIOInput(ULONG HWAddr)
{
    BOOL ret = TRUE;

#if (defined EM9280 || defined EM9283 )
    switch (HWAddr)
    {
    case CSP_BASE_REG_PA_UARTAPP0:										// AUART0
		// CS&ZHL MAY-7-2012: UART0 as COM3
        DDKIomuxSetPinMux(DDK_IOMUX_AUART0_RX, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_0, 0);						// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART0_RX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

		DDKIomuxSetPinMux(DDK_IOMUX_AUART0_TX, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_1, 0);						// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART0_TX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
        break;

    case CSP_BASE_REG_PA_UARTAPP1:										// AUART1
		// CS&ZHL MAY-7-2012: UART1 as COM4
        DDKIomuxSetPinMux(DDK_IOMUX_AUART1_RX, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_4, 0);						// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART1_RX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

		DDKIomuxSetPinMux(DDK_IOMUX_AUART1_TX, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_5, 0);						// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART1_TX, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
        break;

    case CSP_BASE_REG_PA_UARTAPP2:										// AUART2
		// CS&ZHL MAY-7-2012: UART2 as COM5
        DDKIomuxSetPinMux(DDK_IOMUX_AUART2_RX_0, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO2_16, 0);					// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART2_RX_0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

		DDKIomuxSetPinMux(DDK_IOMUX_AUART2_TX_0, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO2_17, 0);					// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART2_TX_0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
        break;

    case CSP_BASE_REG_PA_UARTAPP3:										// AUART3
		// CS&ZHL MAY-7-2012: UART3 as COM6
        DDKIomuxSetPinMux(DDK_IOMUX_AUART3_RX_0, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO2_18, 0);					// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART3_RX_0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

		DDKIomuxSetPinMux(DDK_IOMUX_AUART3_TX_0, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO2_18, 0);					// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART3_TX_0, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
        break;

	case CSP_BASE_REG_PA_UARTAPP4:
		// CS&ZHL MAY-7-2012: UART4 as COM2
        DDKIomuxSetPinMux(DDK_IOMUX_AUART4_RX_1, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_2, 0);						// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART4_RX_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART4_TX_1, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_3, 0);						// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART4_TX_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART4_CTS_1, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_20, 0);					// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART4_CTS_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART4_RTS_1, DDK_IOMUX_MODE_GPIO);
		DDKGpioEnableDataPin(DDK_IOMUX_GPIO3_21, 0);					// output disable
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART4_RTS_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
		break;

    default:
        ret=FALSE;
    }
#endif	//EM9280

    return ret;
}

//-----------------------------------------------------------------------------
// CS&ZHL JUN-14-2012: config RTS/CTS handshake pins for some UART
//
// Function: BSPUartConfigureHandshake
//
// This function is used to configure RTS/CTS handshake pins for some UART
//
// Parameters:
//      HWAddr
//          [in] Physical IO address.
//
// Returns:
//      TRUE if successfully performed the required action.
//
//-----------------------------------------------------------------------------
BOOL BSPUartConfigureHandshake(ULONG HWAddr)
{
    BOOL ret = TRUE;

#if (defined EM9280 || defined EM9283 )
    switch (HWAddr)
    {
	case CSP_BASE_REG_PA_UARTAPP4:
		// CS&ZHL MAY-7-2012: UART4 as COM2
        DDKIomuxSetPinMux(DDK_IOMUX_AUART4_RTS_1, DDK_IOMUX_MODE_02);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART4_RTS_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_AUART4_CTS_1, DDK_IOMUX_MODE_02);
        DDKIomuxSetPadConfig(DDK_IOMUX_AUART4_CTS_1, 
                             DDK_IOMUX_PAD_DRIVE_8MA, 
                             DDK_IOMUX_PAD_PULL_ENABLE,
                             DDK_IOMUX_PAD_VOLTAGE_3V3);
		break;

    default:
		RETAILMSG(1, (TEXT("BSPUartConfigureHandshake: RTS/CTS Handshake is not supported for 0x%x\r\n"), HWAddr));
        ret = FALSE;
        break;
    }
#else	// -> iMX28EVK
    ret=FALSE;
#endif	//EM9280

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
	// CS&ZHL MAY-7-2012: support UART4
    case CSP_BASE_REG_PA_UARTAPP4:
        *reqRx = APBX_CHANNEL_AUART4_RX;
        *reqTx = APBX_CHANNEL_AUART4_TX;
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
	// CS&ZHL MAY-7-2012: support UART4
    case CSP_BASE_REG_PA_UARTAPP4:
        *rxDMAIrq = IRQ_AUART4_RX_DMA;
        *txDMAIrq = IRQ_AUART4_TX_DMA;
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
	// CS&ZHL MAY-7-2012: support UART4
    case CSP_BASE_REG_PA_UARTAPP4:
        dwIndex = 4;
        break;
    default:
        dwIndex = 0xFF;
    }

    return dwIndex;
}

// LQK NOV-2-2012: supporting GPIO based RTSn
BOOL BSPUartGPIO2RTS( DWORD dwGpio )
{
	DWORD dwRtsGpioPin = DDK_IOMUX_INVALID_PIN;

#ifdef EM9280
	switch(dwGpio)
	{
	case GPIO6:
		dwRtsGpioPin = DDK_IOMUX_GPIO3_22;
		break;

	case GPIO7:
		dwRtsGpioPin = DDK_IOMUX_GPIO3_23;
		break;

	case GPIO20:
		dwRtsGpioPin = DDK_IOMUX_GPIO3_28;
		break;

	case GPIO21:
		dwRtsGpioPin = DDK_IOMUX_GPIO3_29;
		break;

	case GPIO22:
		dwRtsGpioPin = DDK_IOMUX_GPIO3_25;
		break;

	case GPIO23:
		dwRtsGpioPin = DDK_IOMUX_GPIO3_24;
		break;

	case GPIO24:
		dwRtsGpioPin = DDK_IOMUX_GPIO2_3;
		break;

	case GPIO25:
		dwRtsGpioPin = DDK_IOMUX_GPIO2_20;
		break;

	case GPIO26:
		dwRtsGpioPin = DDK_IOMUX_GPIO3_30;
		break;

	case GPIO27:
		dwRtsGpioPin = DDK_IOMUX_GPIO4_20;
		break;

	case GPIO28:
		dwRtsGpioPin = DDK_IOMUX_GPIO2_4;
		break;

	case GPIO29:
		dwRtsGpioPin = DDK_IOMUX_GPIO2_6;
		break;

	case GPIO30:
		dwRtsGpioPin = DDK_IOMUX_GPIO2_7;
		break;

	case GPIO31:
		dwRtsGpioPin = DDK_IOMUX_GPIO2_5;
		break;
	default:;
	}

#else EM9283
	if( dwGpio >= 10 && dwGpio <= 31 )
		dwRtsGpioPin = g_EM9283_iMXGpioPin[dwGpio];
#endif //#ifdef EM9280

	return dwRtsGpioPin;
}


//-----------------------------------------------------------------------------
// LQK NOV-2-2012: supporting GPIO based RTSn
// Function: BSPUartConfigureRTS
//
// This function is used to configure the GPIO as RTSn.
//-----------------------------------------------------------------------------
BOOL BSPUartConfigureRTS( DWORD dwGpioRTS )
{
	if( dwGpioRTS == DDK_IOMUX_INVALID_PIN )
		return FALSE;

	DDKIomuxSetPinMux((DDK_IOMUX_PIN)dwGpioRTS, DDK_IOMUX_MODE_GPIO);	// config as GPIO
	DDKGpioWriteDataPin((DDK_IOMUX_PIN)dwGpioRTS, 1);					// DOUT -> High
	DDKGpioEnableDataPin((DDK_IOMUX_PIN)dwGpioRTS, 1);		
	return TRUE;
}

//-----------------------------------------------------------------------------
// LQK NOV-2-2012: supporting GPIO based RTSn
// Function: BSPUartSetGPIORTS
//
// This function is used to set GPIO based RTSn 
//
//-----------------------------------------------------------------------------
BOOL BSPUartSetGPIORTS(DWORD dwGpioRTS)
{
	if( dwGpioRTS == DDK_IOMUX_INVALID_PIN )
		return FALSE;

	DDKGpioWriteDataPin((DDK_IOMUX_PIN)dwGpioRTS, 0);		// DOUT -> Low, active low
	return TRUE;
}

//-----------------------------------------------------------------------------
// LQK NOV-2-2012: supporting GPIO based RTSn
// Function: BSPUartClearGPIORTS
//
// This function is used to clear GPIO based RTSn .
//-----------------------------------------------------------------------------
BOOL BSPUartClearGPIORTS(DWORD dwGpioRTS )
{
	if( dwGpioRTS == DDK_IOMUX_INVALID_PIN )
		return FALSE;

	DDKGpioWriteDataPin((DDK_IOMUX_PIN)dwGpioRTS, 1);		// DOUT -> High, active low
	return TRUE;
}