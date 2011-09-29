//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//
//------------------------------------------------------------------------------
//
//  File:  rotaryhw.c
//
//   This file implements the device specific functions for rotary encoder.
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//   Includes and external references
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)
#include <rotaryhw.h>
#include "csp.h"


//-----------------------------------------------------------------------------
// Local Variables
PVOID pv_HWregTIMROT = NULL;
//-----------------------------------------------------------------------------
// Local Functions

// Sets/Clears RELATIVE bit of the rotary decoder control register.
static VOID RotDecoder_SetCounterRelative (BOOL Value);
// Read STATE bitfield of the rotary decoder control register.
static UINT32 RotDecoder_GetState (VOID);
// Sets the polarities of the rotary decoder inputs.
static VOID RotDecoder_SetPolarities (BOOL SelAPol, BOOL SelBPol);
// Selects the rotary decoder inputs.
static VOID RotDecoder_SetSelects (ROT_INPUT_SOURCE InputA, ROT_INPUT_SOURCE InputB);
// Configure the debounce control of the rotary decoder.
static VOID RotDecoder_SetDebounceParams(UINT32 Div, UINT32 OverSample);
//setting the IOmux for rotary
static VOID RotarySetIOmux();

VOID  RotarySetRelative(BOOL Value);
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Code
//-----------------------------------------------------------------------------
//
//  Function: RotaryInit
//
//  This function initializes the rotary decoder control register with the
//  input paramters.
//
//  Parameters:
//     stRotSetup
//          [IN] Pointer to a rotary setup structure.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL RotaryInit(ROT_SETUP *stRotSetup)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    //allocating memory for the Timer and rotary
    if (pv_HWregTIMROT == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_TIMROT;

        // Map peripheral physical address to virtual address
        pv_HWregTIMROT = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);

        // Check if virtual mapping failed
        if (pv_HWregTIMROT == NULL)
        {
            ERRORMSG(1, (_T("TimRotAlloc:  Virtual mapping failed!\r\n")));
            goto CleanUp;
        }
    } 

    //mux selection for rotary
    RotarySetIOmux();
    
    // Check if the rotary decoder is present in this product
    //
    if( (BF_RD(TIMROT_ROTCTRL, ROTARY_PRESENT)) == 0 )
    {
        ERRORMSG(1, (_T("RotaryInit: ROTARY_PRESENT is not set\r\n")));
        rc= FALSE;
    }
    //
    // Enable the timrot block and turn on the clock
    //  to make the block configurable.
    //
    RotarySetClkGate(FALSE);
    // Clears the STRST bit if it is set

    RotarySetRelative(TRUE);
    RotarySetSftRst();

    //
    // Debounce the rotary inputs
    //
    if ( stRotSetup->Divider > ROT_DEBOUNCE_DIVIDER_MAX)
    {
        stRotSetup->Divider=ROT_DEBOUNCE_DIVIDER;
        BF_SETV(TIMROT_ROTCTRL, DIVIDER, stRotSetup->Divider);
    }
    else
    {
        BF_SETV(TIMROT_ROTCTRL, DIVIDER, stRotSetup->Divider);
    }
    if ( stRotSetup->Oversample > ROT_DEBOUNCE_OVERSAMPLE_MAX)
    {
        stRotSetup->Oversample=ROT_DEBOUNCE_OVERSAMPLE;
        BF_SETV(TIMROT_ROTCTRL, OVERSAMPLE, stRotSetup->Oversample);
    }
    else
    {
        // Set the oversample rate
        BF_SETV(TIMROT_ROTCTRL, OVERSAMPLE, stRotSetup->Oversample);
    }
    // Select the rotary inputs and their polarities
    // Note: Swap SELECT_A/B to reverse the direction of the rotation.
    //       Or, inverse one of the polarities to inverse the direction of the rotation
    if( stRotSetup->SelectA >= ROT_SELECT_MAX || stRotSetup->SelectB >= ROT_SELECT_MAX)
    {
        ERRORMSG(1, (_T("RotaryInit: invalid values \r\n")));
        rc= FALSE;
    }
    else
    {
        BF_CS4(TIMROT_ROTCTRL,
               POLARITY_A, stRotSetup->PolA,
               POLARITY_B, stRotSetup->PolB,
               SELECT_A, stRotSetup->SelectA,
               SELECT_B, stRotSetup->SelectB);
    }

    // Set the operation mode of the updown counter
    BF_SETV(TIMROT_ROTCTRL, RELATIVE, stRotSetup->Relative);

    HW_TIMROT_TIMCTRLn_WR(2,0xc086);
    HW_TIMROT_TIMCOUNTn_WR(2,0x1);
    rc=TRUE;
CleanUp:
    return rc;
}
//-----------------------------------------------------------------------------
//  Function: RotaryDeinit()
//
//  This function Deinitializes the rotary decoder and frees any memory allocated.
//
//  Parameters:
//         None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL RotaryDeinit()
{
    // Turn off the clock
    RotarySetClkGate(TRUE);
    // Unmap peripheral address space
    if (pv_HWregTIMROT != NULL)
    {
        MmUnmapIoSpace(pv_HWregTIMROT, 0x1000);
        pv_HWregTIMROT = NULL;
    }

    return TRUE;
}
//-----------------------------------------------------------------------------
//
//  Function: RotaryGetUpdownCount
//
//  This function Read the rotary decoder up/down counter.
//
//  Parameters:
//         None
//
//  Returns:
//      Returns the value of the up/down count from the rotary count register
//
//-----------------------------------------------------------------------------
INT16  RotaryGetUpdownCount()
{
    return ((INT16)(BF_RD(TIMROT_ROTCOUNT, UPDOWN)));
}

//-----------------------------------------------------------------------------
//
//  Function: RotarySetSftRst
//
//  This function clears the  SFTRST bit if the SFTRST bit for
//  rotary block is set
//
//  Parameters:
//         None
//
//  Returns:
//         None
//
//-----------------------------------------------------------------------------

VOID RotarySetSftRst()
{
    BOOL Value;

    Value=BF_RD(TIMROT_ROTCTRL, SFTRST);

    if(Value)
    {
        // Clear the Soft Reset for normal operation
        BF_CLR(TIMROT_ROTCTRL, SFTRST);
    }
}

//-----------------------------------------------------------------------------
//
//  Function: RotarySetClkGate
//
//  Enable or disable the clock gate of the timer and rotary block.
//
//  Parameters:
//         Value
//            [IN] When set to one disables the clock and when value is 0
//                 rotary block is in normal operation.
//
//  Returns:
//         None
//
//-----------------------------------------------------------------------------
VOID RotarySetClkGate(BOOL Value)
{
    if(Value)
    {
        // Shutdown the clock gate
        BF_SET(TIMROT_ROTCTRL, CLKGATE);
    }
    else
    {
        // Clear the Clock Gate for normal operation
        BF_CLR(TIMROT_ROTCTRL, CLKGATE);
    }
}
//-----------------------------------------------------------------------------
//
//  Function: RotarySetRelative
//
//  This function Sets/Clears RELATIVE bit of the rotary decoder control register.
//
//  Parameters:
//       Value
//         [IN] When set to one, the updown counter will be automatically reset
//               to zero whenever it is read.
//  Returns:
//        None
//
//-----------------------------------------------------------------------------
VOID  RotarySetRelative(BOOL Value)
{
    if(Value)
    {
        // Reset the updown counter to zero on each read
        BF_SET(TIMROT_ROTCTRL, RELATIVE);      // use bitfield set macro
    }
    else
    {
        // Don't reset the updown counter on each read
        BF_CLR(TIMROT_ROTCTRL, RELATIVE);      // use bitfield clear macro
    }
}
//-----------------------------------------------------------------------------
//
//  Function: RotaryGetState
//
//  Read STATE bitfield of the rotary decoder control register.
//
//  Parameters:
//         None
//
//  Returns:
//        The current state of the rotary decoder state machine.
//-----------------------------------------------------------------------------
UINT32 RotaryGetState()
{
    //
    // For easy reference, here is the state machine transition table.
    //
    // Curr  | Input | Input | Input | Input |
    // State |  00   |  01   |  10   |  11   |  (Input order=ba)
    // ---------------------------------------
    //  00   |  00   |  01   |  10   |  err  |
    //  01   | 00,dec|  01   |  err  |  11   |
    //  10   | 00,inc|  err  |  10   |  11   |
    //  11   |  err  |  01   |  10   |  11   |
    //
    // err = invalid input.
    //
    return((UINT32)(BF_RD(TIMROT_ROTCTRL, STATE)));
}

//-----------------------------------------------------------------------------
//
//  Function: RotarySetTimer
//
//  ReEnable the Rotary for timer interrupt 
//
//  Parameters:
//         None
//
//  Returns:
//        None
//-----------------------------------------------------------------------------
void RotarySetTimer()
{   
    HW_TIMROT_TIMCTRLn_CLR(2,0x8000); 
    HW_TIMROT_TIMCTRLn_WR (2,0xc086);
    HW_TIMROT_TIMCOUNTn_WR(2,0x1);
    
    return ;
}

//-----------------------------------------------------------------------------
//
//  Function: RotarySetPolarities
//
//  Sets the polarities of the rotary decoder inputs.
//
//  Parameters:
//     SelAPol
//           [IN] Input value of POLARITY_A.
//     SelBPol
//           [IN]   Input value of POLARITY_B.
//  Returns:
//        None
//-----------------------------------------------------------------------------
VOID RotarySetPolarities(BOOL SelAPol, BOOL SelBPol)
{
    if(SelAPol)
    {
        // Inverse the inputA polarity
        BF_SET(TIMROT_ROTCTRL, POLARITY_A);      // use bitfield set macro
    }
    else
    {
        // Do not inverse the inputA polarity
        BF_CLR(TIMROT_ROTCTRL, POLARITY_A);      // use bitfield clear macro
    }

    if(SelBPol)
    {
        // Inverse the inputB polarity
        BF_SET(TIMROT_ROTCTRL, POLARITY_B);      // use bitfield set macro
    }
    else
    {
        // Do not inverse the inputB polarity
        BF_CLR(TIMROT_ROTCTRL, POLARITY_B);      // use bitfield clear macro
    }
}
//-----------------------------------------------------------------------------
//
//  Function: RotarySetSelects
//
//  Selects the rotary decoder inputs.
//
//  Parameters:
//      InputA
//          [IN]   Input value of SELECT_A..
//      InputB
//          [IN]   Input value of SELECT_B.
//  Returns:
//        None
//-----------------------------------------------------------------------------
VOID RotarySetSelects (ROT_INPUT_SOURCE InputA, ROT_INPUT_SOURCE InputB)
{
    BF_CS2(TIMROT_ROTCTRL,
           SELECT_A, InputA,   // Select the inputA for Select_A
           SELECT_B, InputB);  // Select the inputB for Select_B
}
//-----------------------------------------------------------------------------
//
//  Function: RotarySetDebounceParams
//
//  Configure the debounce control of the rotary decoder.
//
//  Parameters:
//      Divider
//          [IN]   Input value of DIVIDER.
//      OverSample
//          [IN]   Input value of OVERSAMPLE.
//  Returns:
//        None
//-----------------------------------------------------------------------------
VOID RotarySetDebounceParams(UINT32 Divider, UINT32 OverSample)
{
    // Sets the 1 KHz clock divider
    BF_SETV(TIMROT_ROTCTRL, DIVIDER, Divider);

    // Sets the oversample rate
    BF_SETV(TIMROT_ROTCTRL, OVERSAMPLE, OverSample);
}
//-----------------------------------------------------------------------------
//
//  Function: RotarySetIOmux
//
//  Configure the debounce control of the rotary decoder.
//
//  Parameters:
//        None
//  Returns:
//        None
//-----------------------------------------------------------------------------
VOID RotarySetIOmux()
{
    //mux selection for rotarya rotaryb
    DDKIomuxSetPinMux(DDK_IOMUX_TIMROT1_DUP1, DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_TIMROT2_DUP, DDK_IOMUX_MODE_00);
}
