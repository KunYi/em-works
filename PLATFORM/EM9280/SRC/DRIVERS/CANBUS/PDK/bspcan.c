//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bspcan.c
//
//  Implementation of CAN Driver
//
//  This file implements the bsp level code for CANBUS driver.
//
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
typedef enum _CAN_IOMUX_MODE {  
    CAN1_GPMO= 0 ,
    CAN1_AUART   ,
    CAN2_GPMO    ,
    CAN2_AUART
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
   
    //Configure PIO for enable/disable CAN transeiver
    DDKIomuxSetPinMux(DDK_IOMUX_SSP1_CMD_1, DDK_IOMUX_MODE_GPIO);
    // Disable transeiver by default
    DDKGpioEnableDataPin(DDK_IOMUX_SSP1_CMD_1, 1);
    DDKGpioWriteDataPin(DDK_IOMUX_SSP1_CMD_1, 1);

    switch (canmode)
    {
           
    case CAN1_GPMO:     
        DDKIomuxSetPinMux(DDK_IOMUX_CAN0_TX_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_CAN0_TX_0, 
                        DDK_IOMUX_PAD_DRIVE_8MA, 
                        DDK_IOMUX_PAD_PULL_ENABLE,
                        DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_CAN0_RX_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_CAN0_RX_0, 
                        DDK_IOMUX_PAD_DRIVE_8MA, 
                        DDK_IOMUX_PAD_PULL_ENABLE,
                        DDK_IOMUX_PAD_VOLTAGE_3V3);

        break;

    case CAN1_AUART:
        DDKIomuxSetPinMux(DDK_IOMUX_CAN0_TX_1, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_CAN0_TX_1, 
                       DDK_IOMUX_PAD_DRIVE_8MA, 
                       DDK_IOMUX_PAD_PULL_ENABLE,
                       DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_CAN0_RX_1, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_CAN0_RX_1, 
                       DDK_IOMUX_PAD_DRIVE_8MA, 
                       DDK_IOMUX_PAD_PULL_ENABLE,
                       DDK_IOMUX_PAD_VOLTAGE_3V3);

        break;

    case CAN2_GPMO:
        DDKIomuxSetPinMux(DDK_IOMUX_CAN1_TX_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_CAN1_TX_0, 
                      DDK_IOMUX_PAD_DRIVE_8MA, 
                      DDK_IOMUX_PAD_PULL_ENABLE,
                      DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_CAN1_RX_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_CAN1_RX_0, 
                      DDK_IOMUX_PAD_DRIVE_8MA, 
                      DDK_IOMUX_PAD_PULL_ENABLE,
                      DDK_IOMUX_PAD_VOLTAGE_3V3);    
        break;
        
    case CAN2_AUART:   
        DDKIomuxSetPinMux(DDK_IOMUX_CAN1_TX_1, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_CAN1_TX_1, 
                      DDK_IOMUX_PAD_DRIVE_8MA, 
                      DDK_IOMUX_PAD_PULL_ENABLE,
                      DDK_IOMUX_PAD_VOLTAGE_3V3);

        DDKIomuxSetPinMux(DDK_IOMUX_CAN1_RX_1, DDK_IOMUX_MODE_01);
        DDKIomuxSetPadConfig(DDK_IOMUX_CAN1_RX_1, 
                      DDK_IOMUX_PAD_DRIVE_8MA, 
                      DDK_IOMUX_PAD_PULL_ENABLE,
                      DDK_IOMUX_PAD_VOLTAGE_3V3); 
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
           Ci=DDK_CLOCK_GATE_FLEXCAN0_CLK;
           break;
        case 2:
           Ci=DDK_CLOCK_GATE_FLEXCAN1_CLK;
            break;
       
        default:
            return  FALSE;
    } 
   
    if(Enable)
    {
        cfgState = DDKClockSetGatingMode(Ci, DDK_CLOCK_GATE_MODE_DISABLED);
        if(cfgState == FALSE)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("BSPCANClockConfig: Enable CAN clock failed.\r\n")));
            return FALSE;
        }

    }
    else
    {
        cfgState = DDKClockSetGatingMode(Ci, DDK_CLOCK_GATE_MODE_ENABLEDD );
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
            CANConfigureGPIO(CAN1_GPMO);
            break;

        case 2:
            CANConfigureGPIO(CAN2_GPMO);
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
           return CSP_BASE_REG_PA_CAN0;
       case 2:
           return CSP_BASE_REG_PA_CAN1;
      
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
             return IRQ_CAN0;
         case 2:
             return IRQ_CAN1;
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
    UNREFERENCED_PARAMETER(index);
    
    DDKIomuxSetPinMux(DDK_IOMUX_SSP1_CMD_1, DDK_IOMUX_MODE_GPIO);
    DDKGpioEnableDataPin(DDK_IOMUX_SSP1_CMD_1, 1);
    
    if(Enable)
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_CMD_1, 1); 
    else
        DDKGpioWriteDataPin(DDK_IOMUX_SSP1_CMD_1, 0); 
    
}






