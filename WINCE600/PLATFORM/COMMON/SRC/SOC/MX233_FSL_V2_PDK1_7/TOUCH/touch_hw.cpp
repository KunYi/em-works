//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  touch_hw.cpp
//
//  Provides the PDD code for the DDSI touch driver routines.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <tchddsi.h>
#pragma warning(pop)

#include "ceddk.h"
#include "touch_ddsi.h"
#include "touch_hw.h"
#include "hw_lradc.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

extern HANDLE g_hLRADCTch;
extern DWORD gIntrTouch;
extern DWORD gIntrTouchChanged;
extern BOOL IsPowerOff;
//-----------------------------------------------------------------------------
// Defines
#define ABS(x)  ((x) >= 0 ? (x) : (-(x)))

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables
static DWORD ConfirmRetires;
static BOOL fPenRemoved, fValidSample;
static _ST_TP_SAMPLES Sample;

//-----------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
// Include Files
//------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//
// Function: TriggerSampleX
//
// This function initializes the delay channel to schedule x  channel
//
// Parameters:
//      None.
//
// Returns:
//      Returns zero (0).
//
//-----------------------------------------------------------------------------
static void TriggerSampleX(void)
{
    // Enable YMINUS_ENABLE/YPLUS_ENABLE
    LRADCEnableTouchDetectYDrive(g_hLRADCTch,TRUE);

    // Enable Interrupt
    LRADCEnableInterrupt(g_hLRADCTch, LRADC_CH2, TRUE);
    LRADCEnableInterrupt(g_hLRADCTch, LRADC_CH3, TRUE);

    // Configure delayTrigger
    LRADCSetDelayTrigger( g_hLRADCTch,
                          LRADC_DELAY_TRIGGER1,                                                     // Trigger Index
                          (1 << LRADC_CH3)|(1 << LRADC_CH2),    // Lradc channels
                          (1 << LRADC_DELAY_TRIGGER1),                  // Restart the triggers
                          0,                                                                    // No loop count
                          1);                                                                   // Delay Count

    // Set KICK
    LRADCSetDelayTriggerKick(g_hLRADCTch,LRADC_DELAY_TRIGGER1, TRUE);

    // Make sure interrupt is enabled
    InterruptDone(gIntrTouchChanged);
}

//-----------------------------------------------------------------------------
//
// Function: TriggerSampleY
//
// This function initializes the delay channel to schedule Y  channel
//
// Parameters:
//      None.
//
// Returns:
//      Returns zero (0).
//
//-----------------------------------------------------------------------------
static void TriggerSampleY(void)
{
    // Enable XMINUS_ENABLE/XPLUS_ENABLE
    LRADCEnableTouchDetectXDrive(g_hLRADCTch,TRUE);

    // Enable Interrupt
    LRADCEnableInterrupt(g_hLRADCTch, LRADC_CH2, TRUE);
    LRADCEnableInterrupt(g_hLRADCTch, LRADC_CH3, TRUE);

    // Configure delayTrigger
    LRADCSetDelayTrigger( g_hLRADCTch,
                          LRADC_DELAY_TRIGGER1,                                                  // Trigger Index
                          (1 << LRADC_CH3)|(1 << LRADC_CH2), // Lradc channels
                          (1 << LRADC_DELAY_TRIGGER1),           // Restart the triggers
                          0,                                                             // No loop count
                          1);
    // Set KICK
    LRADCSetDelayTriggerKick(g_hLRADCTch,LRADC_DELAY_TRIGGER1, TRUE);

    // Make sure interrupt is enabled
    InterruptDone(gIntrTouchChanged);
}

//-----------------------------------------------------------------------------
//
// Function: ValidSamplePoint
//
//
//
// Parameters:
//      None.
//
// Returns:
//      Returns _EN_TP_INTERRUPT.
//
//-----------------------------------------------------------------------------
static _EN_TP_INTERRUPT ValidSamplePoint(void)
{
    DWORD TouchHit;
    static _ST_TP_SAMPLE_POINT tempdata[4];
    static DWORD index=0;
    int i;

    fValidSample=FALSE;
    // TOUCH_DETECT_ENABLE =1
    LRADCEnableTouchDetect(g_hLRADCTch,TRUE);

    for(i=0;i<10;i++)
    {
        StallExecution(1);
        Sleep(0);
    }

    TouchHit = LRADCGetTouchDetect(g_hLRADCTch);

    // TOUCH_DETECT_ENABLE =0
    LRADCEnableTouchDetect(g_hLRADCTch,FALSE);

    // Ok Pen is up return to default state matchine
    if((TouchHit & 1) == 0)
    {
        index=0;       
        return _TP_INTERRUPT_PEN_DOWN_;
    
    }
  
    if((ABS(Sample.Current.y-Sample.Previous.y) >= _AD_TORLANCE_Y_) ||
       (ABS(Sample.Current.x-Sample.Previous.x) >= _AD_TORLANCE_X_))
    {
        ConfirmRetires = 2;
    }
    else if(ConfirmRetires == 0)
    {
        tempdata[index].x=Sample.Current.x;
        tempdata[index].y=Sample.Current.y;
        index++;
       
    }
    else
    {
        ConfirmRetires--;
    }
  
    if(index==4)
    {
    
        Sample.Confirmed.x=(tempdata[0].x+tempdata[1].x+tempdata[2].x+tempdata[3].x)/4;
        Sample.Confirmed.y=(tempdata[0].y+tempdata[1].y+tempdata[2].y+tempdata[3].y)/4;
        index=0;
        fValidSample=TRUE;
        Sleep(3);
     
    }
    Sample.Previous.x = Sample.Current.x;
    Sample.Previous.y = Sample.Current.y;

    return _TP_INTERRUPT_PEN_UP_OR_TIMER_SAMPLE_X_;
}
//-----------------------------------------------------------------------------
//
// Function: TouchConfig
//
// This function Configures the LRADC and intializes the interrupts
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE/FALSE.
//
//-----------------------------------------------------------------------------
BOOL TouchConfig(void)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+TouchConfig()\r\n")));

    // Set Analog powerUP
    LRADCSetAnalogPowerUp(g_hLRADCTch, TRUE);

    // Enable interrupts for X+, X-, Y+, Y- drive
    LRADCEnableInterrupt(g_hLRADCTch, LRADC_CH2, TRUE);
    LRADCEnableInterrupt(g_hLRADCTch, LRADC_CH3, TRUE);
    LRADCEnableInterrupt(g_hLRADCTch, LRADC_CH4, TRUE);
    LRADCEnableInterrupt(g_hLRADCTch, LRADC_CH5, TRUE);

    // Enable Touch detect
    LRADCEnableTouchDetect(g_hLRADCTch,TRUE);

    // Enable Touch detect irq
    LRADCEnableTouchDetectInterrupt(g_hLRADCTch,TRUE);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-TouchConfig()\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: BSPTouchInterruptDone
//
// DESCRIPTION:
//      Interrupt done for touch pen down.
//
// Parameters:
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void TouchInterruptClear(void)
{
    LRADCClearTouchDetect(g_hLRADCTch);
    
    LRADCClearInterruptFlag(g_hLRADCTch,LRADC_CH3);

}

//-----------------------------------------------------------------------------
//
// Function: TouchInterruptCapture
//
// This function processes irq state machine and return the expect interrupt
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
_EN_TP_INTERRUPT TouchInterruptCapture(_EN_TP_INTERRUPT irqCurrent)
{
    _EN_TP_INTERRUPT irqNext = _TP_INTERRUPT_PEN_DOWN_;

    // Disable interruprs
    LRADCEnableInterrupt(g_hLRADCTch, LRADC_CH2, FALSE);
    LRADCEnableInterrupt(g_hLRADCTch, LRADC_CH3, FALSE);
    LRADCEnableTouchDetectInterrupt(g_hLRADCTch,FALSE);

    switch(irqCurrent)
    {
    case _TP_INTERRUPT_PEN_DOWN_:

        LRADCEnableTouchDetect(g_hLRADCTch,FALSE);

        LRADCEnableTouchDetectXDrive(g_hLRADCTch,FALSE);
        LRADCEnableTouchDetectYDrive(g_hLRADCTch,FALSE);

        Sample.Previous.x = 0xffff;
        Sample.Previous.y = 0xffff;

        ConfirmRetires = 2;

        fValidSample   = FALSE;
        fPenRemoved    = FALSE;

        irqNext = _TP_INTERRUPT_PEN_UP_OR_TIMER_SAMPLE_X_;

        TriggerSampleX();

        break;

    case _TP_INTERRUPT_PEN_UP_OR_TIMER_SAMPLE_X_:

        LRADCCLearDelayChannel(g_hLRADCTch, LRADC_DELAY_CH1);

        Sample.Current.x = LRADCGetAccumValue(g_hLRADCTch,LRADC_CH2);
        LRADCClearAccum(g_hLRADCTch,LRADC_CH2);

        LRADCEnableTouchDetectXDrive(g_hLRADCTch,FALSE);
        LRADCEnableTouchDetectYDrive(g_hLRADCTch,FALSE);

        LRADCEnableTouchDetect(g_hLRADCTch,FALSE);

        irqNext =_TP_INTERRUPT_PEN_UP_OR_TIMER_SAMPLE_Y_;

        TriggerSampleY();

        break;

    case _TP_INTERRUPT_PEN_UP_OR_TIMER_SAMPLE_Y_:

        LRADCCLearDelayChannel(g_hLRADCTch, LRADC_DELAY_CH1);

        LRADCEnableTouchDetectXDrive(g_hLRADCTch,FALSE);
        LRADCEnableTouchDetectYDrive(g_hLRADCTch,FALSE);
        LRADCEnableTouchDetect(g_hLRADCTch,FALSE);

        Sample.Current.y = LRADCGetAccumValue(g_hLRADCTch,LRADC_CH3);
        LRADCClearAccum(g_hLRADCTch,LRADC_CH3);

        irqNext = ValidSamplePoint();

        if(irqNext == _TP_INTERRUPT_PEN_UP_OR_TIMER_SAMPLE_X_)
        {
            TriggerSampleX();
        }
        break;
    }
    if(irqNext == _TP_INTERRUPT_PEN_DOWN_)
    {
        // Enable Touch detect
        LRADCEnableTouchDetect(g_hLRADCTch,TRUE);

        // Enable Touch detect irq
        LRADCEnableTouchDetectInterrupt(g_hLRADCTch,TRUE);

        // ReEnable the interrupt
        InterruptDone(gIntrTouch);
    }
    return irqNext;
}

//-----------------------------------------------------------------------------
//
// Function: TouchGetSample
//
// This function gets current sample point
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
//
TOUCH_PANEL_SAMPLE_FLAGS TouchGetSample(INT *x, INT *y)
{
    if(fPenRemoved != FALSE)
    {
        return TouchSampleValidFlag;
    }
    else if(fValidSample == FALSE)
    {
        return (TouchSampleIgnore | TouchSampleDownFlag); 
    }
    *x = Sample.Confirmed.x;
    *y = Sample.Confirmed.y;


    if(IsPowerOff)
    {
       return TouchSampleValidFlag;
    }
   
    return (TouchSampleValidFlag | TouchSampleDownFlag);
}

