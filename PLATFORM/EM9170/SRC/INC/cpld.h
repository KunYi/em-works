//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  Header: cpld.h
//
//  Provides definitions for the CPLD logic on the 3DS board.
//
//------------------------------------------------------------------------------
#ifndef __CPLD_H
#define __CPLD_H

#if    __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
// CPLD board-level IRQs
#define IRQ_CPLD_ETHER                  (IRQ_SOC_NUMBER + 0)
#define IRQ_CPLD_XUARTB                 (IRQ_SOC_NUMBER + 1)
#define IRQ_CPLD_XUARTA                 (IRQ_SOC_NUMBER + 2)
#define IRQ_CPLD_BTN1                   (IRQ_SOC_NUMBER + 3)
#define IRQ_CPLD_BTN2                   (IRQ_SOC_NUMBER + 4)

#define IRQ_CPLD_MIN                    IRQ_CPLD_ETHER
#define IRQ_CPLD_MAX                    IRQ_CPLD_BTN2

// IRQ line that CPLD connects to SoC
#define CPLD_IRQ_GPIO_PIN               26
#define CPLD_IRQ_GPIO_LINE              IRQ_GPIO1_PIN26

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define CPLD_LAN9217_OFFSET                  0x00000000
#define CPLD_SC16C652_PORTA_OFFSET           0x00008000
#define CPLD_SC16C652_PORTB_OFFSET           0x00010000

#define CPLD_LED_CTRL_OFFSET                 0x00020000
#define CPLD_DIP_BTN_OFFSET                  0x00020008
#define CPLD_INT_STATUS_OFFSET               0x00020010
#define CPLD_INT_RESET_OFFSET                0x00020020
#define CPLD_INT_MASK_OFFSET                 0x00020038
#define CPLD_RET_AAAA_OFFSET                 0x00020040
#define CPLD_RET_5555_OFFSET                 0x00020048
#define CPLD_CPLD_VER_OFFSET                 0x00020050
#define CPLD_RET_CAFE_OFFSET                 0x00020058
#define CPLD_SW_RESET_OFFSET                 0x00020060
#define CPLD_BOARD_ID_OFFSET                 0x00020068


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define CPLD_LED_CTRL_LED1_LSH               0
#define CPLD_LED_CTRL_LED2_LSH               1
#define CPLD_LED_CTRL_LED3_LSH               2
#define CPLD_LED_CTRL_LED4_LSH               3
#define CPLD_LED_CTRL_LED5_LSH               4
#define CPLD_LED_CTRL_LED6_LSH               5
#define CPLD_LED_CTRL_LED7_LSH               6
#define CPLD_LED_CTRL_LED8_LSH               7

#define CPLD_DIP_BTN_DIP1_LSH                0
#define CPLD_DIP_BTN_DIP2_LSH                1
#define CPLD_DIP_BTN_DIP3_LSH                2
#define CPLD_DIP_BTN_DIP4_LSH                3
#define CPLD_DIP_BTN_DIP5_LSH                4
#define CPLD_DIP_BTN_DIP6_LSH                5
#define CPLD_DIP_BTN_DIP7_LSH                6
#define CPLD_DIP_BTN_BTN1_LSH                7
#define CPLD_DIP_BTN_BTN2_LSH                8

#define CPLD_INT_STATUS_ETHER_LSH            0
#define CPLD_INT_STATUS_XUARTA_LSH           1
#define CPLD_INT_STATUS_XUARTB_LSH           2
#define CPLD_INT_STATUS_BTN1_LSH             3
#define CPLD_INT_STATUS_BTN2_LSH             4

#define CPLD_INT_RESET_ETHER_LSH             0
#define CPLD_INT_RESET_XUARTA_LSH            1
#define CPLD_INT_RESET_XUARTB_LSH            2
#define CPLD_INT_RESET_BTN1_LSH              3
#define CPLD_INT_RESET_BTN2_LSH              4

#define CPLD_INT_MASK_ETHER_LSH              0
#define CPLD_INT_MASK_XUARTA_LSH             1
#define CPLD_INT_MASK_XUARTB_LSH             2
#define CPLD_INT_MASK_BTN1_LSH               3
#define CPLD_INT_MASK_BTN2_LSH               4

#define CPLD_SW_RESET_XUART_LSH              0


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define CPLD_LED_CTRL_LED1_WID               1
#define CPLD_LED_CTRL_LED2_WID               1
#define CPLD_LED_CTRL_LED3_WID               1
#define CPLD_LED_CTRL_LED4_WID               1
#define CPLD_LED_CTRL_LED5_WID               1
#define CPLD_LED_CTRL_LED6_WID               1
#define CPLD_LED_CTRL_LED7_WID               1
#define CPLD_LED_CTRL_LED8_WID               1

#define CPLD_DIP_BTN_DIP1_WID                1
#define CPLD_DIP_BTN_DIP2_WID                1
#define CPLD_DIP_BTN_DIP3_WID                1
#define CPLD_DIP_BTN_DIP4_WID                1
#define CPLD_DIP_BTN_DIP5_WID                1
#define CPLD_DIP_BTN_DIP6_WID                1
#define CPLD_DIP_BTN_DIP7_WID                1
#define CPLD_DIP_BTN_BTN1_WID                1
#define CPLD_DIP_BTN_BTN1_WID                1

#define CPLD_INT_STATUS_ETHER_WID            1
#define CPLD_INT_STATUS_XUARTA_WID           1
#define CPLD_INT_STATUS_XUARTB_WID           1
#define CPLD_INT_STATUS_BTN1_WID             1
#define CPLD_INT_STATUS_BTN2_WID             1

#define CPLD_INT_RESET_ETHER_WID             1
#define CPLD_INT_RESET_XUARTA_WID            1
#define CPLD_INT_RESET_XUARTB_WID            1
#define CPLD_INT_RESET_BTN1_WID              1
#define CPLD_INT_RESET_BTN2_WID              1

#define CPLD_INT_MASK_ETHER_WID              1
#define CPLD_INT_MASK_XUARTA_WID             1
#define CPLD_INT_MASK_XUARTB_WID             1
#define CPLD_INT_MASK_BTN1_WID               1
#define CPLD_INT_MASK_BTN2_WID               1

#define CPLD_SW_RESET_XUART_WID              1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define CPLD_LED_CTRL_OFF                    0
#define CPLD_LED_CTRL_ON                     1

#define CPLD_DIP_BTN_DTP1_EXT_UART           0
#define CPLD_DIP_BTN_DTP1_INT_UART           1


//------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//------------------------------------------------------------------------------
UINT16 CPLDRead16(UINT32 u32Addr);
UINT32 CPLDReadFifo16(UINT32 u32Addr, UINT16 *pwBuf, UINT32 dwCount);
void CPLDWrite16(UINT32 u32Addr, UINT16 data);
UINT32 CPLDWriteFifo16(UINT32 u32Addr, UINT16 *pwBuf, UINT32 dwCount);
BOOL CPLDMap(void);
BOOL CPLDInit(void);

#ifdef __cplusplus
}
#endif

#endif // __CPLD_H
