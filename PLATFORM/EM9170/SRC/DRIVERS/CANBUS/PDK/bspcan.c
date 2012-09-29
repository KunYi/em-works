//------------------------------------------------------------------------------
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS 
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//------------------------------------------------------------------------------
//
//  File:  bspcan.c
//
//  Implementation of CAN Driver
//
//  This file implements the bsp level code for CANBUS driver.
//
//-----------------------------------------------------------------------------


/*********************************************************************
 GLOBAL DEFINITIONS  
*********************************************************************/
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define CAN_TRANSEIVER_ENABLE_GPIO_PORT     (DDK_GPIO_PORT4)
#define CAN_TRANSEIVER_ENABLE_GPIO_PIN      (6)

#define CAN_TRANSEIVER_ENABLE_GPIO_ENABLE      (0)
#define CAN_TRANSEIVER_ENABLE_GPIO_DISABLE     (1)
//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
typedef enum _CAN_IOMUX_MODE {  
    CAN1_FEC= 0 ,
    CAN1_GPIO   ,
    CAN2_FEC    ,
    CAN2_GPIO
} CAN_IOMUX_MODE;

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function:  CANConfigureGPIO
//
// This function makes the DDK call to configure the IOMUX
// 
//
// Parameters:
//      canmode
//          [in] can iomux index mode
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//-----------------------------------------------------------------------------

BOOL CANConfigureGPIO(CAN_IOMUX_MODE canmode)
{
    RETAILMSG(1, (TEXT("CANConfigureGPIO: CANMode = %d\r\n"), canmode));

#ifndef	EM9170
    // Configure PIO for enable/disable CAN transeiver
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D14, DDK_IOMUX_PIN_MUXMODE_ALT5,DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PIN_D14,DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_KEEPER,  DDK_IOMUX_PAD_HYSTERESIS_DISABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKGpioSetConfig(CAN_TRANSEIVER_ENABLE_GPIO_PORT, CAN_TRANSEIVER_ENABLE_GPIO_PIN, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    // Disable transeiver by default
    DDKGpioWriteDataPin(CAN_TRANSEIVER_ENABLE_GPIO_PORT, CAN_TRANSEIVER_ENABLE_GPIO_PIN, CAN_TRANSEIVER_ENABLE_GPIO_DISABLE);
#endif	//EM9170

    switch (canmode)
    {
           
    case CAN1_FEC:
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_TX_EN, DDK_IOMUX_PIN_MUXMODE_ALT4,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_RDATA0, DDK_IOMUX_PIN_MUXMODE_ALT4,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PIN_FEC_TX_EN,DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_KEEPER,  DDK_IOMUX_PAD_HYSTERESIS_DISABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_PIN_FEC_RDATA0, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_KEEPER,  DDK_IOMUX_PAD_HYSTERESIS_DISABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_CAN1_IPP_IND_CANRX, 0x0);
        break;

    case CAN1_GPIO:
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_A, DDK_IOMUX_PIN_MUXMODE_ALT6,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_B, DDK_IOMUX_PIN_MUXMODE_ALT6,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PIN_GPIO_A,DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_KEEPER,  DDK_IOMUX_PAD_HYSTERESIS_DISABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_PIN_GPIO_B, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_KEEPER,  DDK_IOMUX_PAD_HYSTERESIS_DISABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_CAN1_IPP_IND_CANRX, 0x1);
        break;

    case CAN2_FEC:
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_RDATA1, DDK_IOMUX_PIN_MUXMODE_ALT4,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_RX_DV, DDK_IOMUX_PIN_MUXMODE_ALT4,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PIN_FEC_RDATA1,DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_KEEPER,  DDK_IOMUX_PAD_HYSTERESIS_DISABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_PIN_FEC_RX_DV, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_KEEPER,  DDK_IOMUX_PAD_HYSTERESIS_DISABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_CAN2_IPP_IND_CANRX, 0x0);
        break;
        
    case CAN2_GPIO:
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_C, DDK_IOMUX_PIN_MUXMODE_ALT6,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_D, DDK_IOMUX_PIN_MUXMODE_ALT6,DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PIN_GPIO_C,DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_KEEPER,  DDK_IOMUX_PAD_HYSTERESIS_DISABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_PIN_GPIO_D, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE,  DDK_IOMUX_PAD_PULL_KEEPER,  DDK_IOMUX_PAD_HYSTERESIS_DISABLE,DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_CAN2_IPP_IND_CANRX, 0x1);
        break;

    default:
        break;
        
    }

   
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: BSPCANClockConfig
//
// This function is a wrapper for CAN  to enable/disable its clock using a valid
// CRM handle.
//
// Parameters:
//      index
//          [in]    Index specifying the CAN module.
//      bEnable
//          [in]    TRUE if CAN Clock is to be enabled. FALSE if CAN Clock is
//                  to be disabled.
//
// Returns:
//      TRUE if successfully performed the required action.
//
//-----------------------------------------------------------------------------

BOOL BSPCANClockConfig( DWORD index,IN BOOL Enable )
{
    BOOL cfgState;
    DDK_CLOCK_GATE_INDEX Ci;

    switch (index)
    {
        case 1:
           Ci=DDK_CLOCK_GATE_INDEX_CAN1;
           break;
        case 2:
           Ci=DDK_CLOCK_GATE_INDEX_CAN2;
            break;
       
        default:
            return  FALSE;
    } 
   
    if(Enable)
    {
        cfgState = DDKClockSetGatingMode(Ci, DDK_CLOCK_GATE_MODE_ENABLED);
        if(cfgState == FALSE)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPCANClockConfig: Enable CAN clock failed.\r\n")));
            return FALSE;
        }

    }
    else
    {
        cfgState = DDKClockSetGatingMode(Ci, DDK_CLOCK_GATE_MODE_DISABLED);
        if(cfgState == FALSE)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPCANClockConfig: Disable CAN clock failed.\r\n")));
            return FALSE;
        }

    }

return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  BSPCANConfigureGPIO
//
// This function makes the DDK call to configure the IOMUX
// pins required for the CAN.
//
// Parameters:
//      index
//          [in] Index of the CAN device requested.
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//-----------------------------------------------------------------------------

void  BSPCANConfigureGPIO(DWORD index)
{
    switch (index)
    {
        case 1:
            CANConfigureGPIO(CAN1_GPIO);
            break;

        case 2:
            CANConfigureGPIO(CAN2_GPIO);
            break;
       
        default:
            return ;
    } 
}

//-----------------------------------------------------------------------------
//
// Function:  CANGetBaseRegAddr
//
// This function returns the physical base address for the
// CAN registers based on the device index requested.
//
// Parameters:
//      index
//          [in] Index of the CAN device requested.
//
// Returns:
//      Physical base address for CAN registers, or 0 if an
//      invalid index was passed.
//
//-----------------------------------------------------------------------------


DWORD  BSPGetCANBaseRegAddr(DWORD index)
{
    switch (index)
    {
        case 1:
            return CSP_BASE_REG_PA_CAN1;
        case 2:
            return CSP_BASE_REG_PA_CAN2;
       
        default:
            return 0;
    } 
}

//-----------------------------------------------------------------------------
//
// Function:  BSPGetCANIRQ
//
// This function returns the physical base address for the
// CAN registers based on the device index requested.
//
// Parameters:
//      index
//          [in] Index of the CAN device requested.
//
// Returns:
//      Physical IRQ  for CAN  bus
//
//-----------------------------------------------------------------------------
DWORD  BSPGetCANIRQ(DWORD index)
{
    switch (index)
    {
        case 1:
            return IRQ_CAN1;
        case 2:
            return IRQ_CAN2;
        default:
            return 0;
    }

}

//-----------------------------------------------------------------------------
//
// Function:  BSPSetCanPower
//
// This function enabe can transceiver power for special canbus controller
//
// Parameters:
//      index
//          [in] Index of the CAN device requested.
//      bEnable
//
//          [in]    TRUE if CAN Power  is to be enabled. FALSE if CAN Power is
//                  to be disabled.
//
// Returns:
//     
//
//-----------------------------------------------------------------------------
void   BSPSetCanPowerEnable(DWORD index,BOOL Enable)
{
    
    if(2==index)
    {
        if(Enable)
        {
            if (DDKGpioWriteDataPin(CAN_TRANSEIVER_ENABLE_GPIO_PORT, CAN_TRANSEIVER_ENABLE_GPIO_PIN, CAN_TRANSEIVER_ENABLE_GPIO_ENABLE)==FALSE)
            {
                ERRORMSG(TRUE, (_T("BSPSetCanPowerEnable: DDKGpioWriteDataPin failed\r\n")));
            }
        }
        else
        {
            if (DDKGpioWriteDataPin(CAN_TRANSEIVER_ENABLE_GPIO_PORT, CAN_TRANSEIVER_ENABLE_GPIO_PIN, CAN_TRANSEIVER_ENABLE_GPIO_DISABLE)==FALSE)
            {
                ERRORMSG(TRUE, (_T("BSPSetCanPowerEnable: DDKGpioWriteDataPin failed\r\n")));
            }
        }
    }
    

}






