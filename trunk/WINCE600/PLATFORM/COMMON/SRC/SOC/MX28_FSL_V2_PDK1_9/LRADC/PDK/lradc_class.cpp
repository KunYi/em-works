//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//
//-----------------------------------------------------------------------------
//
//  File:  lradc_class.cpp
//
//  Provides BSP-specific configuration routines for the LRADC channels.
//`
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "lradc_class.h"
#include "csp.h"
#include "regslradc.h"

//-----------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregLRADC;
//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
static HANDLE g_hLRADCChannelIntrEvent[8];                 // Interrupt Occurence Event (Shared)
static HANDLE m_hLRADCAppIntrEvent[8];
static HANDLE Test_Event;


//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
//
// Function:  IOControl
//
// This function Initializes the H/w of the LRADC Driver
// with LRADC.
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number to configure
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------

BOOL LRADCClass::IOControl(DWORD Handle,
                           DWORD dwIoControlCode,
                           PBYTE pBufIn,PBYTE pBufOut)
{

    BOOL fSuccess = true;

    LRADCClass* pLRADC = (LRADCClass*) Handle;

    if( !pLRADC )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    switch( dwIoControlCode )
    {
    case LRADC_IOCTL_INIT_CHANNEL:
        fSuccess = pLRADC->Init(( STLRADCCONFIGURE * )pBufIn);
        break;

    case LRADC_IOCTL_GET_CLKCGATE:
        fSuccess = pLRADC->GetClkGate();
        break;

    case LRADC_IOCTL_CONFIGURE_CHANNEL:
        if(!pBufIn )
            break;
        fSuccess = pLRADC->ConfigureChannel(( STLRADCCONFIGURE * )pBufIn);
        break;
    case LRADC_IOCTL_ENABLE_INTERRUPT:
    {
        STLRADCCONFIGURE stLRADCConfig = *((STLRADCCONFIGURE *)pBufIn);
        if(!pBufIn )
            break;
        fSuccess = pLRADC->EnableInterrupt(stLRADCConfig.eChannel,stLRADCConfig.bValue);
    }
    break;

    case LRADC_IOCTL_CLEAR_INTERRUPT:
    {
        STLRADCCONFIGURE stLRADCConfig = *(STLRADCCONFIGURE *)pBufIn;

        if(!pBufIn )
            break;
        fSuccess = pLRADC->ClearInterruptFlag(stLRADCConfig.eChannel);
    }
    break;

    case LRADC_IOCTL_SET_DELAY_TRIGGER:
    {
        STLRADCCONFIGURE stLRADCConfig = *( STLRADCCONFIGURE * )pBufIn;
        if(!pBufIn )
            break;
        fSuccess = pLRADC->SetDelayTrigger(&stLRADCConfig);
    }
    break;
    case LRADC_IOCTL_CLEAR_DELAY_CHANNEL:
    {
        STLRADCCONFIGURE stLRADCConfig = *( STLRADCCONFIGURE * )pBufIn;
        if(!pBufIn )
            break;
        fSuccess = pLRADC->CLearDelayChannel(stLRADCConfig.eDelayChannel);

    }
    break;
    case LRADC_IOCTL_SET_TRIGGER_KICK:
    {
        STLRADCCONFIGURE stLRADCConfig = *( STLRADCCONFIGURE * )pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->SetDelayTriggerKick(stLRADCConfig.bValue,stLRADCConfig.eDelayTrigger);
    }
    break;
    case LRADC_IOCTL_SET_ANALOG_POWER_UP:
    {
        STLRADCCONFIGURE stLRADCConfig = *( STLRADCCONFIGURE * )pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->SetAnalogPowerUp(stLRADCConfig.Use5WireTouch,stLRADCConfig.bValue);
    }
    break;
    case LRADC_IOCTL_GET_ACCUM_VALUE:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        LradcConfigure.u16AccumValue = pLRADC->GetAccumValue(LradcConfigure.eChannel);

        memcpy(pBufOut,&LradcConfigure.u16AccumValue,sizeof(UINT16));
    }
    break;
    case LRADC_IOCTL_ENABLE_BATT_MEASUREMENT:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        pLRADC->EnableBatteryMeasurement( LradcConfigure.eDelayTrigger,
                                          LradcConfigure.u16BattSampRate);
    }
    break;
    case LRADC_IOCTL_MEASUREVDD5V:
    {
        UINT32 Y;
        Y = pLRADC->MeasureVDD5V();
        memcpy(pBufOut,&Y,sizeof(UINT32));
    }
    break;    
    case LRADC_IOCTL_MEASUREDIETEMPERATURE:
    {
        //STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        UINT32 X;
        X = pLRADC->MeasureDieTemperature();
        memcpy(pBufOut,&X,sizeof(UINT32));
    }
    break;    
    case LRADC_IOCTL_READ_TOUCH_X:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        UINT16 X;

        if(!pBufIn )
            break;

        X = pLRADC->TouchReadX(LradcConfigure.eChannel);

        memcpy(pBufOut,&X,sizeof(UINT16));
    }
    break;

    case LRADC_IOCTL_READ_TOUCH_Y:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        UINT16 Y;

        if(!pBufIn )
            break;

        Y = pLRADC->TouchReadY(LradcConfigure.eChannel);

        memcpy(pBufOut,&Y,sizeof(UINT16));
    }
    break;

    case LRADC_IOCTL_GET_INTERRUPT_FLAG:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = (pLRADC->GetInterruptFlag(LradcConfigure.eChannel));
    }
    break;

    case LRADC_IOCTL_GET_TOGGLE_FLAG:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->GetToggleFlag(LradcConfigure.eChannel);
    }
    break;

    case LRADC_IOCTL_SCHEDULE_CHANNEL:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->ScheduleChannel(LradcConfigure.eChannel);
    }
    break;
    case LRADC_IOCTL_CLEAR_ACCUM:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->ClearAccum(LradcConfigure.eChannel);
    }
    break;
    case LRADC_IOCTL_CHANNEL_PRESENT:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->GetChannelPresent(LradcConfigure.eChannel);
    }
    break;
    ////Touch Related IOCTLS
    case LRADC_IOCTL_TOUCH_DETECT_PRESENT:
    {
        fSuccess = pLRADC->GetTouchDetectPresent();
    }
    break;
    case LRADC_IOCTL_ENABLE_TOUCH_DETECT:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->EnableTouchDetect(LradcConfigure.bValue);
    }
    break;
    case LRADC_IOCTL_GET_TOUCH_DETECT:
    {
        fSuccess = pLRADC->GetTouchDetect();
    }
    break;
    case LRADC_IOCTL_ENABLE_TOUCH_DETECT_IRQ:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->EnableTouchDetectInterrupt(LradcConfigure.bValue);
    }
    break;
    case LRADC_IOCTL_GET_TOUCH_DETECT_IRQ:
    {
        fSuccess = pLRADC->GetTouchDetectInterruptFlag();
    }
    break;
    case LRADC_IOCTL_CLR_TOUCH_DETECT:
    {
        fSuccess = pLRADC->ClearTouchDetectInterruptFlag();
    }
    break;
    case LRADC_IOCTL_ENABLE_X_DRIVE:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->EnableTouchDetectXDrive(LradcConfigure.Use5WireTouch,LradcConfigure.bValue);
    }
    break;
    case LRADC_IOCTL_ENABLE_Y_DRIVE:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->EnableTouchDetectYDrive(LradcConfigure.Use5WireTouch,LradcConfigure.bValue);
    }
    break;
    case LRADC_IOCTL_CLEAR_CHANNEL:
    {
        STLRADCCONFIGURE LradcConfigure = *(STLRADCCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->ClearChannel(LradcConfigure.eChannel);
    }
    break;
    case LRADC_IOCTL_ENABLE_BUTTON_DETECT:
    {
        LRADCBUTTONCONFIGURE ButtonConfigure = *(LRADCBUTTONCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->EnableButtonDetect(ButtonConfigure.button,ButtonConfigure.bValue);
    }
    break;
    case LRADC_IOCTL_ENABLE_BUTTON_DETECT_IRQ:
    {
        LRADCBUTTONCONFIGURE ButtonConfigure = *(LRADCBUTTONCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;

        fSuccess = pLRADC->EnableButtonDetectInterrupt(ButtonConfigure.button,ButtonConfigure.bValue);
    }
    case LRADC_IOCTL_CLR_BUTTON_DETECT:
    {
        LRADCBUTTONCONFIGURE ButtonConfigure = *(LRADCBUTTONCONFIGURE *)pBufIn;
        if(!pBufIn )
            break;
        
        fSuccess = pLRADC->ClearButtonDetectInterruptFlag(ButtonConfigure.button);
    }
    break;    
    case LRADC_IOCTL_DUMPREGISTER:
    {
        fSuccess = pLRADC->DumpRegister();
    }
    break;

    }
    return fSuccess;
}
//-----------------------------------------------------------------------------
//
// Function:  ConfigureChannel
//
// This function Configures the specified channel required for interaction
// with LRADC.
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number to configure
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::ConfigureChannel(STLRADCCONFIGURE *stLRADCConfig)
{
    RETAILMSG(0,(TEXT("Configure Channel ++ \r\n")));

	switch(stLRADCConfig->eChannel)
	{
	case LRADC_CH0:
		BF_WR(LRADC_CTRL4, LRADC0SELECT, LRADC_CH0);
		break;

	case LRADC_CH1:
		BF_WR(LRADC_CTRL4, LRADC1SELECT, LRADC_CH1);
		break;

	case LRADC_CH6:
		BF_WR(LRADC_CTRL4, LRADC6SELECT, LRADC_CH6);
		break;
	}

    //DumpRegister();
    if(stLRADCConfig->bEnableDivideByTwo){

        // Enable the divide-by-two of a LRADC channel
        BF_SETV(LRADC_CTRL2, DIVIDE_BY_TWO, (1 << stLRADCConfig->eChannel));
    }
    else
	{
        // Disable the divide-by-two of a LRADC channel
        BF_CLRV(LRADC_CTRL2, DIVIDE_BY_TWO, (1 << stLRADCConfig->eChannel));
    }
    // Clear the accumulator & NUM_SAMPLES
    HW_LRADC_CHn_CLR(stLRADCConfig->eChannel, 0xFFFFFFFF);

    // Sets NUM_SAMPLES bitfield of HW_LRADC_CHn register.
    BF_WRn(LRADC_CHn, stLRADCConfig->eChannel, NUM_SAMPLES, (stLRADCConfig->u8NumSamples & 0x1f));

    // Set ACCUMULATE bit of HW_LRADC_CHn register
    if(stLRADCConfig->bEnableAccum)
    {
        // Enable the accumulation of a LRADC channel
        BF_SETn(LRADC_CHn, stLRADCConfig->eChannel, ACCUMULATE);
    }
    else
    {
        // Disable the accumulation of a LRADC channel
        BF_CLRn(LRADC_CHn, stLRADCConfig->eChannel, ACCUMULATE);
    }
    //DumpRegister();
    RETAILMSG(0,(TEXT("Configure Channel -- \r\n")));
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  ClearInterruptFlag
//
// This function Clears the interrupt Flag of the specified channel
//
// Parameters:
//      //      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number to clear the Interrupt
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::ClearInterruptFlag(LRADC_CHANNEL eChannel)
{
    // Clears a LRADC channel interrupt flag
    switch (eChannel)
    {
    case LRADC_CH0:
        BF_CLR(LRADC_CTRL1, LRADC0_IRQ);
        break;
    case LRADC_CH1:
        BF_CLR(LRADC_CTRL1, LRADC1_IRQ);
        break;
    case LRADC_CH2:
        BF_CLR(LRADC_CTRL1, LRADC2_IRQ);
        break;
    case LRADC_CH3:
        BF_CLR(LRADC_CTRL1, LRADC3_IRQ);
        break;
    case LRADC_CH4:
        BF_CLR(LRADC_CTRL1, LRADC4_IRQ);
        break;
    case LRADC_CH5:
        BF_CLR(LRADC_CTRL1, LRADC5_IRQ);
        break;
    case LRADC_CH6:
        BF_CLR(LRADC_CTRL1, LRADC6_IRQ);
        break;
    case LRADC_CH7:
        BF_CLR(LRADC_CTRL1, LRADC7_IRQ);
        break;
    default:
        break;
    }
    //RETAILMSG(0,(TEXT(" Clear Interrupt HW_LRADC_CTRL1 = %08X"),HW_LRADC_CTRL1_RD()));
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  EnableInterrupt
//
// This function Enable or disable the interrupt Flag of the specified channel
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies
//      the channel number to Enable or Disable the Interrupt
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::EnableInterrupt(LRADC_CHANNEL Channel, BOOL bValue)
{
    RETAILMSG(0,(TEXT("EnableInterrupt ++ \r\n")));
    if(bValue)
    {
        ClearInterruptFlag(Channel);
        HW_LRADC_CTRL1_SET((1 << (Channel + BP_LRADC_CTRL1_LRADC0_IRQ_EN)));
    }
    else
    {
        HW_LRADC_CTRL1_CLR((1 << (Channel + BP_LRADC_CTRL1_LRADC0_IRQ_EN)));
        ClearInterruptFlag(Channel);
    }

	//for debug
	if((Channel == LRADC_CH0) || (Channel == LRADC_CH1))
	{
		DWORD	dwCTRL1;

		dwCTRL1 = HW_LRADC_CTRL1_RD();
		RETAILMSG(1, (TEXT("EnableInterrupt: HW_LRADC_CTRL1 = 0x%08x\r\n"), dwCTRL1));
	}

	RETAILMSG(0,(TEXT("EnableInterrupt -- \r\n")));
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  SetDelayTrigger
//
// This function sets the Delay_Trigger value of the specified channel
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number
//      to set the corresponding TRIGGER_LRADCS
//                               TRIGGER_DELAYS
//                               LOOP_COUNT
//                               DELAY bitfields
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
// Set the Delay register
BOOL LRADCClass::SetDelayTrigger(STLRADCCONFIGURE *stLRADCConfig)
{
    //Set the TRIGGER_LRADCS bitfield of HW_LRADC_DELAYn register
    BF_SETVn(LRADC_DELAYn, stLRADCConfig->eDelayTrigger, TRIGGER_LRADCS,  stLRADCConfig->u32TriggerLradcs);

    //Set the TRIGGER_DELAYS bitfield of HW_LRADC_DELAYn register
    BF_SETVn(LRADC_DELAYn, stLRADCConfig->eDelayTrigger, TRIGGER_DELAYS,  stLRADCConfig->u32DelayTriggers);

    //Write the LOOP_COUNT bitfield of HW_LRADC_DELAYn register
    BF_WRn(LRADC_DELAYn, stLRADCConfig->eDelayTrigger, LOOP_COUNT,  stLRADCConfig->u32LoopCount);

    //Write the DEALY bitfield of HW_LRADC_DELAYn register
    BF_WRn(LRADC_DELAYn, stLRADCConfig->eDelayTrigger, DELAY,  stLRADCConfig->u32DelayCount);

    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  SetDelayTriggerKick
//
// This function sets the Delay_Trigger Kick value of the specified channel
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number
//      to set the KICK bitfield of the Delay Control register
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::SetDelayTriggerKick(BOOL bValue,LRADC_DELAYTRIGGER DelayTrigger)
{
    if(bValue)
    {
        // Start the delay trigger
        BF_SETn(LRADC_DELAYn, DelayTrigger, KICK);
    }
    else
    {
        // Stop the delay trigger
        BF_CLRn(LRADC_DELAYn, DelayTrigger, KICK);
    }

    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  SetAnalogPowerUp
//
// This function gets Accumulated value of the specified channel
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number
//      to get the Accumulated value of the Channel.
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::SetAnalogPowerUp(BOOL Use5WireTouch,BOOL bValue)
{
    if(bValue)
    {
        // Enable Analog Power
        BF_CLR(LRADC_CTRL3, FORCE_ANALOG_PWDN);
        BF_SET(LRADC_CTRL3, FORCE_ANALOG_PWUP);
    }
    else
    {
        // Disable Analog Power
        BF_CLR(LRADC_CTRL3, FORCE_ANALOG_PWUP);
        BF_SET(LRADC_CTRL3, FORCE_ANALOG_PWDN);
    }
    BW_LRADC_CTRL0_TOUCH_SCREEN_TYPE(Use5WireTouch) ;//0 for 4 wire 1 for 5 wire

    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  CLearDelayChannel
//
// This function gets Accumulated value of the specified channel
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number
//      to get the Accumulated value of the Channel.
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::CLearDelayChannel(LRADC_DELAY_CHANNEL eDelayChannel)
{
    if(eDelayChannel >= 0 && eDelayChannel < 4)
        HW_LRADC_DELAYn_CLR(eDelayChannel, 0xFFFFFFFF);

    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  GetAccumValue
//
// This function gets Accumulated value of the specified channel
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number
//      to get the Accumulated value of the Channel.
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
UINT16 LRADCClass::GetAccumValue(LRADC_CHANNEL eChannel)
{
    UINT16 AccumVal;

    // Read the accumulator value of the channel
    AccumVal = (BF_RDn(LRADC_CHn, eChannel, VALUE));

    return (AccumVal);
}

//-----------------------------------------------------------------------------
//
// Function:  GetInterruptFlag
//
// This function gets Interrupt Status bit of the specified channel
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number
//      to get Interrupt status value of the specified Channel.
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::GetInterruptFlag(LRADC_CHANNEL eChannel)
{
    BOOL bRtn;

    // Returns LRADC interrupt flag
    switch (eChannel)
    {
    case LRADC_CH0:
        bRtn = BF_RD(LRADC_CTRL1, LRADC0_IRQ);
        break;
    case LRADC_CH1:
        bRtn = BF_RD(LRADC_CTRL1, LRADC1_IRQ);
        break;
    case LRADC_CH2:
        bRtn = BF_RD(LRADC_CTRL1, LRADC2_IRQ);
        break;
    case LRADC_CH3:
        bRtn = BF_RD(LRADC_CTRL1, LRADC3_IRQ);
        break;
    case LRADC_CH4:
        bRtn = BF_RD(LRADC_CTRL1, LRADC4_IRQ);
        break;
    case LRADC_CH5:
        bRtn = BF_RD(LRADC_CTRL1, LRADC5_IRQ);
        break;
    case LRADC_CH6:
        bRtn = BF_RD(LRADC_CTRL1, LRADC6_IRQ);
        break;
    case LRADC_CH7:
        bRtn = BF_RD(LRADC_CTRL1, LRADC7_IRQ);
        break;
    default:
        bRtn = 0;
    }
    return bRtn;
}

//-----------------------------------------------------------------------------
//
// Function:  GetToggleFlag
//
// This function gets Toggle Value Status of the specified channel
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number
//      to get Toggle Value of the specified Channel.
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::GetToggleFlag(LRADC_CHANNEL eChannel)
{

    // Return the TOGGLE flag of a LRADC channel
    return ((BOOL)(BF_RDn(LRADC_CHn, eChannel, TOGGLE)));
}

//-----------------------------------------------------------------------------
//
// Function:  ScheduleChannel
//
// This function Sets the Schedule bit of the specified channel
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number
//      to set the Schedule Bit.
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::ScheduleChannel(LRADC_CHANNEL eChannel)
{
    // Set the SCHEDULE bitfield of HW_LRADC_CTRL0 register
    BF_SETV(LRADC_CTRL0, SCHEDULE, (1 << eChannel));

	////for debug
	//if((eChannel == LRADC_CH0) || (eChannel == LRADC_CH1))
	//{
	//	DWORD	dwCTRL0;

	//	dwCTRL0 = HW_LRADC_CTRL0_RD();
	//	RETAILMSG(1, (TEXT("ScheduleChannel: HW_LRADC_CTRL0 = 0x%08x\r\n"), dwCTRL0));
	//}

	return TRUE;
}

// CS&ZHL JUN-6-2012: return schedule state of specified channel
BOOL LRADCClass::ScheduleState(LRADC_CHANNEL eChannel)
{
	DWORD	dwCTRL0;

	dwCTRL0 = HW_LRADC_CTRL0_RD();
	if(dwCTRL0 & (1 << eChannel))
	{
		return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function:  ClearAccum
//
// This function Clears the Accum Value of the specified channel
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which specifies the channel number
//      to Clear the VALUE Bit field.
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::ClearAccum(LRADC_CHANNEL eChannel)
{
    // Write zero to VALUE bitfield of HW_LRADC_CHn register
    BF_CLRn(LRADC_CHn, eChannel, VALUE);
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  ClearChannel
//
// This function Check if the lradc channel is present.
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which checks the LRADC status regsiter
//      to detect the channel field present.
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------

BOOL LRADCClass::ClearChannel(LRADC_CHANNEL eChannel)
{
    //Clear the channel
    HW_LRADC_CHn_CLR(eChannel, 0xFFFFFFFF);
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  GetChannelPresent
//
// This function Check if the lradc channel is present.
//
// Parameters:
//      stLRADCConfig: pointer to structure STLRADCCONFIGURE which checks the LRADC status regsiter
//      to detect the channel field present.
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------

BOOL LRADCClass::GetChannelPresent(LRADC_CHANNEL eChannel)
{
    BOOL bRtn;

    // Read a bit field of HW_LRADC_STATUS register
    switch (eChannel)
    {
    case LRADC_CH0:
        bRtn =  BF_RD(LRADC_STATUS, CHANNEL0_PRESENT);
        break;
    case LRADC_CH1:
        bRtn =  BF_RD(LRADC_STATUS, CHANNEL1_PRESENT);
        break;
    case LRADC_CH2:
        bRtn =  BF_RD(LRADC_STATUS, CHANNEL2_PRESENT);
        break;
    case LRADC_CH3:
        bRtn =  BF_RD(LRADC_STATUS, CHANNEL3_PRESENT);
        break;
    case LRADC_CH4:
        bRtn =  BF_RD(LRADC_STATUS, CHANNEL4_PRESENT);
        break;
    case LRADC_CH5:
        bRtn =  BF_RD(LRADC_STATUS, CHANNEL5_PRESENT);
        break;
    case LRADC_CH6:
        bRtn =  BF_RD(LRADC_STATUS, CHANNEL6_PRESENT);
        break;
    case LRADC_CH7:
        bRtn =  BF_RD(LRADC_STATUS, CHANNEL7_PRESENT);
        break;
    default:
        bRtn = 0;
    }
    return bRtn;
}


//Touch related functions
//-----------------------------------------------------------------------------
//
// Function:  GetTouchDetectPresent
//
// This function Check if the TOUCH_PANEL_PRESENT is set in LRADC STATUS register.
//
// Parameters:
// None
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------

BOOL LRADCClass::GetTouchDetectPresent(VOID)
{
    // Read the TOUCH_PANEL_PRESENT bit of HW_LRADC_STATUS register
    return (BF_RD(LRADC_STATUS, TOUCH_PANEL_PRESENT));
}


//-----------------------------------------------------------------------------
//
// Function:  EnableTouchDetect
//
// This function set or clear the  TOUCH_DETECT_ENABLE in HW_LRADC_CTRL0 Register.
//
// Parameters:
//  bvalue : Boolean value to configure for enable or disable of the TOUCH_DETECT_ENABLE in
//           LRADC control register 0
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::EnableTouchDetect(BOOL bValue)
{
    if(bValue)
    {   //Enable the touch detector
        BF_SET(LRADC_CTRL0, TOUCH_DETECT_ENABLE);
    }
    else
    {   // Disable the touch detector
        BF_CLR(LRADC_CTRL0, TOUCH_DETECT_ENABLE);
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  EnableTouchDetectInterrupt
//
// This function set or clear the  TOUCH_DETECT_IRQ_EN in HW_LRADC_CTRL1 Register.
//
// Parameters:
//  bvalue : Boolean value to configure for enable or disable of the TOUCH_DETECT_IRQ_EN in
//           LRADC control register 1
// Returns:
//      None
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::EnableTouchDetectInterrupt(BOOL bValue)
{
    if(bValue)
    {   //Enable the touch detector interrupt
        BF_SET(LRADC_CTRL1, TOUCH_DETECT_IRQ_EN);
    }
    else
    {   // Disable the touch detector interrupt
        BF_CLR(LRADC_CTRL1, TOUCH_DETECT_IRQ_EN);
    }
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  EnableButtonDetect
//
// This function set or clear the  BUTTON0_DETECT_ENABLE or BUTTON1_DETECT_ENABLE in HW_LRADC_CTRL0 Register.
//
// Parameters:
//  button
//          [in] which button you select
//  bvalue : Boolean value to configure for enable or disable of the TOUCH_DETECT_ENABLE in
//           LRADC control register 0
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::EnableButtonDetect(LRADC_BUTTON button,BOOL bValue)
{
    if(button==LRADC_BUTTON0)
    {
        if(bValue)
        {   //Enable the touch detector
            BF_SET(LRADC_CTRL0,BUTTON0_DETECT_ENABLE); 
        }
        else
        {   // Disable the touch detector
            BF_CLR(LRADC_CTRL0, BUTTON0_DETECT_ENABLE); 
        }
    }
    else
    {
        if(bValue)
        {   //Enable the touch detector
            BF_SET(LRADC_CTRL0,BUTTON1_DETECT_ENABLE); 
        }
        else
        {   // Disable the touch detector
            BF_CLR(LRADC_CTRL0, BUTTON1_DETECT_ENABLE); 
        }
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  EnableButtonDetectInterrupt
//
// This function set or clear the  BUTTON0_DETECT_IRQ or  BUTTON1_DETECT_IRQ in HW_LRADC_CTRL1 Register.
//
// Parameters:
//  button
//          [in] which button you select
//  bvalue : Boolean value to configure for enable or disable of the BUTTON0_DETECT_IRQ  in
//           LRADC control register 1
// Returns:
//      None
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::EnableButtonDetectInterrupt(LRADC_BUTTON button,BOOL bValue)
{
    if(button==LRADC_BUTTON0)
    {
        if(bValue)
        {   //Enable the touch detector interrupt
            BF_SET(LRADC_CTRL1,BUTTON0_DETECT_IRQ_EN); 
        }
        else
        {   // Disable the touch detector interrupt
            BF_CLR(LRADC_CTRL1,BUTTON0_DETECT_IRQ_EN); 
        }
    }
    else
    {
      if(bValue)
        {   //Enable the touch detector interrupt
            BF_SET(LRADC_CTRL1,BUTTON1_DETECT_IRQ_EN); 
        }
        else
        {   // Disable the touch detector interrupt
            BF_CLR(LRADC_CTRL1,BUTTON1_DETECT_IRQ_EN); 
        }
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  ClearButtonDetectInterruptFlag
//
// This function Clears the  BUTTON0_DETECT_IRQ or  BUTTON1_DETECT_IRQ in HW_LRADC_CTRL1 Register.
//
// Parameters:
//  button
//          [in] which button you select
// Returns:
// None
//-----------------------------------------------------------------------------
BOOL LRADCClass::ClearButtonDetectInterruptFlag(LRADC_BUTTON button)
{
    if(button==LRADC_BUTTON0)
        BF_CLR(LRADC_CTRL1,BUTTON0_DETECT_IRQ);
    else
        BF_CLR(LRADC_CTRL1,BUTTON1_DETECT_IRQ);    
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  GetTouchDetectInterruptFlag
//
// This function Get the  TOUCH_DETECT_IRQ status in HW_LRADC_CTRL1 Register.
//
// Parameters:
//  None
// Returns:
//      Returns TRUE or FALSE.
//
//-----------------------------------------------------------------------------
BOOL LRADCClass::GetTouchDetectInterruptFlag(VOID)
{
    return ((BOOL)(BF_RD(LRADC_CTRL1, TOUCH_DETECT_IRQ)));
}

//-----------------------------------------------------------------------------
//
// Function:  ClearTouchDetectInterruptFlag
//
// This function Clears the  TOUCH_DETECT_IRQ in HW_LRADC_CTRL1 Register.
//
// Parameters:
// None
// Returns:
// None
//-----------------------------------------------------------------------------
BOOL LRADCClass::ClearTouchDetectInterruptFlag(VOID)
{
    BF_CLR(LRADC_CTRL1, TOUCH_DETECT_IRQ);
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  EnableTouchDetectXDrive
//
// This function enable or disable the X Channels in  HW_LRADC_CTRL0 Register.
//
// Parameters:
// bValue : Boolean value to enable or disable the X Drive
// Returns:
// None
//-----------------------------------------------------------------------------
BOOL LRADCClass::EnableTouchDetectXDrive(BOOL Use5WireTouch,BOOL bValue)
{
   
   if(Use5WireTouch)
   {
       if(bValue)
        {   //Enable the X-Drive
            BF_WR(LRADC_CTRL0, XNURSW,1);//+
            BF_SET(LRADC_CTRL0, XPULSW);//+
            BF_SET(LRADC_CTRL0, YNLRSW);//-
            BF_WR(LRADC_CTRL0, YPLLSW,2);//-
        }
        else
        {   // Disable the X-Drive
            BF_CLR(LRADC_CTRL0,XNURSW);//-
            BF_CLR(LRADC_CTRL0, XPULSW);//-
            BF_CLR(LRADC_CTRL0, YNLRSW);//-
            BF_CLR(LRADC_CTRL0,YPLLSW);//-
        }
    }
    else
    {
         if(bValue)
        {   //Enable the X-Drive
            BF_WR(LRADC_CTRL0,XNURSW,2);
            BF_SET(LRADC_CTRL0, XPULSW);
        }
        else
        {   // Disable the X-Drive
            BF_CLRV(LRADC_CTRL0,XNURSW,2);
            BF_CLR(LRADC_CTRL0, XPULSW);
        }
    }
 
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  EnableTouchDetectYDrive
//
// This function enable or disable the Y Channels in  HW_LRADC_CTRL0 Register.
//
// Parameters:
// bValue : Boolean value to enable or disable the Y Drive
// Returns:
// None
//-----------------------------------------------------------------------------
BOOL LRADCClass::EnableTouchDetectYDrive(BOOL Use5WireTouch,BOOL bValue)
{

    if(Use5WireTouch)
    {
        if(bValue)
        {   //Enable the Y-Drive
            BF_WR(LRADC_CTRL0,XNURSW,2); //-
            BF_SET(LRADC_CTRL0, XPULSW);//+
            BF_SET(LRADC_CTRL0, YNLRSW);//-
            BF_WR(LRADC_CTRL0, YPLLSW,1);//+
        }
        else
        {   // Disable the Y-Drive
            BF_CLR(LRADC_CTRL0,XNURSW);//-
            BF_CLR(LRADC_CTRL0, XPULSW);//-
            BF_CLR(LRADC_CTRL0, YNLRSW);//-
            BF_CLR(LRADC_CTRL0,YPLLSW);//-
        }
    }
    else
    {
        if(bValue)
        {   //Enable the Y-Drive
            BF_SET(LRADC_CTRL0, YNLRSW);
            BF_WR(LRADC_CTRL0, YPLLSW,1);
        }
        else
        {   // Disable the Y-Drive
            BF_CLR(LRADC_CTRL0, YNLRSW);
            BF_CLRV(LRADC_CTRL0, YPLLSW,1);
        }
    }
    
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  GetTouchDetect
//
// This function Read the TOUCH_DETECT_RAW bit of HW_LRADC_STATUS register.
//
// Parameters:
// bValue : Boolean value if the value is found true
//          else FALSE
// Returns:
// None
//-----------------------------------------------------------------------------
BOOL LRADCClass::GetTouchDetect(VOID)
{
    // Read the TOUCH_DETECT_RAW bit of HW_LRADC_STATUS register
    return ((BOOL)(BF_RD(LRADC_STATUS, TOUCH_DETECT_RAW)));
}

//-----------------------------------------------------------------------------
//
// Function:  GetClkGate
//
// This function Get Clk Gate state.
//
// Parameters:
// bValue : Boolean value if the value is found true
//          else FALSE
// Returns:
//  returns TRUE  for Enable ,FALSE for Disable.
//-----------------------------------------------------------------------------
BOOL LRADCClass::GetClkGate(VOID)
{
    if ( (BF_RD(LRADC_CTRL0, SFTRST) != 0) || (BF_RD(LRADC_CTRL0, CLKGATE) != 0) )
        return FALSE;
    else
        return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: Init
//
//  This function  for LRADCClass constructor 
//
// Parameters:
//     None.
//
// Returns:
//     None.
//
//------------------------------------------------------------------------------
LRADCClass::LRADCClass()
{
    return;
}

//------------------------------------------------------------------------------
//
// Function: DeInit
//
//  This function  for LRADC deinit 
//
// Parameters:
//     None.
//
// Returns:
//     None.
//
//------------------------------------------------------------------------------
VOID LRADCClass::DeInit(VOID)
{
    return;
}

//------------------------------------------------------------------------------
//
// Function: LARDCIST
//
//  This function  for LARDCIST  thread
//
// Parameters:
//     None.
//
// Returns:
//     None.
//
//------------------------------------------------------------------------------
DWORD LRADCClass::LARDCIST()
{
    DWORD dwRetVal =0, ObjectID = 0;

    for(;;){
        ObjectID = WaitForSingleObject(Test_Event, INFINITE);

        if(ObjectID != WAIT_OBJECT_0 )
        {
            RETAILMSG(0,(TEXT("Test Event not  Triggered ")));
            break;
        }
        else
        {
            RETAILMSG(0,(TEXT("Test Event is Triggered ")));
        }
        //DumpRegister();
        ClearInterruptFlag(LRADC_CH0);
        InterruptDone(dwSysIntr);
    }

    return dwRetVal;
}

//------------------------------------------------------------------------------
//
// Function: Init
//
// This function for lradc touch init.
//
// Parameters:
//      STLRADCCONFIGURE
//          Lradc coonfig
//
// Returns:
//      returns TRUE  for success.
//
//------------------------------------------------------------------------------
BOOL LRADCClass::Init(STLRADCCONFIGURE *stLRADCConfig)
{

    RETAILMSG(0,(TEXT("LRADCClass::Init :: HW_LRADC_CTRL4 = %08X ++ \r\n"),HW_LRADC_CTRL4_RD()));
    //DumpRegister();

    // Clear the Soft Reset for normal operation
    BF_CLR(LRADC_CTRL0, SFTRST);

    // Clear the Clock Gate for normal operation
    BF_CLR(LRADC_CTRL0, CLKGATE);      // use bitfield clear macro

    // Set on-chip ground ref
    if(stLRADCConfig->bEnableOnChipGroundRef)
    {
        // Enable the onchip ground ref of LRADC conversions
        BF_SET( LRADC_CTRL0, ONCHIP_GROUNDREF);
    }
    else
    {
        // Disable the onchip ground ref of LRADC conversions
        BF_CLR( LRADC_CTRL0, ONCHIP_GROUNDREF);
    }

    // Set LRADC conversion clock frequency
    BF_WR(LRADC_CTRL3, CYCLE_TIME, stLRADCConfig->eFreq);

    // Make sure the LRADC channel-6 selected for VDDIO
    BF_WR(LRADC_CTRL4, LRADC6SELECT, VDDIO_VOLTAGE_CH);

    // Make sure the LRADC channel-7 selected for Battery
    BF_WR(LRADC_CTRL4, LRADC7SELECT, BATTERY_VOLTAGE_CH);

    RETAILMSG(0,(TEXT("LRADCClass::Init :: HW_LRADC_CTRL4 = %08X  -- \r\n"),HW_LRADC_CTRL4_RD()));
    //DumpRegister();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: TouchReadX
//
// This function reads touch screen sample.
//
// Parameters:
//      eChannel
//          [] channel
//
// Returns:
//      X-coordinate of the point.
//
//------------------------------------------------------------------------------
UINT16 LRADCClass::TouchReadX(LRADC_CHANNEL eChannel)
{
    UINT16 X = 0;

    // set the schedule bit
    BF_SETV(LRADC_CTRL0, SCHEDULE, (1 << eChannel));

    // with for the conversion finish
    while((HW_LRADC_CTRL0_RD() & BF_LRADC_CTRL0_SCHEDULE(1 << eChannel)) != 0)

        // read the ADC X sample value
        X = (BF_RDn(LRADC_CHn, eChannel, VALUE));

    // return the X sample value
    return (X);
}

//------------------------------------------------------------------------------
//
// Function: TouchReadY
//
// This function reads touch screen sample.
//
// Parameters:
//      eChannel
//          [] channel
//
// Returns:
//      Y-coordinate of the point.
//
//------------------------------------------------------------------------------
UINT16 LRADCClass::TouchReadY(LRADC_CHANNEL eChannel)
{
    UINT16 Y = 0;

    // set the schedule bit
    BF_SETV(LRADC_CTRL0, SCHEDULE, (1 << eChannel));

    // with for the conversion finish
    while((HW_LRADC_CTRL0_RD() & BF_LRADC_CTRL0_SCHEDULE(1 << eChannel)) != 0)

        // read the ADC X sample value
        Y = (BF_RDn(LRADC_CHn, eChannel, VALUE));

    // return the Y sample value
    return (Y);
}

//------------------------------------------------------------------------------
//
// Function: EnableBatteryMeasurement
//
// This function Enables the Battery measurement.
//
// Parameters:
//      eTrigger
//          [in] Specifies the Delay Trigger to use
//      SamplingInterval
//          [in] Specifies the smapling interval for the Battery value updation
//
// Returns:
//      returns 0 for success otherwise the error value.
//
//------------------------------------------------------------------------------
UINT32 LRADCClass::EnableBatteryMeasurement( LRADC_DELAYTRIGGER eTrigger,
                                             UINT16 SamplingInterval)
{
    STLRADCCONFIGURE stLRADCConfig;
    LRADC_CHANNEL eChannel = BATTERY_VOLTAGE_CH;

    //RETAILMSG(1, (TEXT("EnableBatteryMeasurement ++ \r\n")));
    // Check if the lradc channel is present in this product
    if( GetChannelPresent(eChannel) == 0 )
        return (1);

    // Disable the channel interrupt
    EnableInterrupt(eChannel, FALSE);

    ClearInterruptFlag(eChannel);

    // Configure the battery conversion register
    // Set the scale factor of battery A-to-D conversion

    BF_WR(LRADC_CONVERSION, SCALE_FACTOR, 2);

    // Enable the automatic update mode of BATT_VALUE field in HW_POWER_MONITOR register
    BF_SET(LRADC_CONVERSION, AUTOMATIC);

    memset(&stLRADCConfig, 0, sizeof(stLRADCConfig));

    //initiallize the Lradc config structure
    stLRADCConfig.eChannel = eChannel;
    stLRADCConfig.bEnableDivideByTwo = FALSE;
    stLRADCConfig.bEnableAccum = FALSE;
    stLRADCConfig.u8NumSamples = 0;

    ConfigureChannel(&stLRADCConfig );

    // schedule a conversion before the setting up of the delay channel
    // so the user can have a good value right after the function returns
    ScheduleChannel(eChannel);

    // Setup the trigger loop forever,
    stLRADCConfig.eDelayTrigger = eTrigger;
    stLRADCConfig.u32TriggerLradcs = (1 << eChannel);
    stLRADCConfig.u32DelayTriggers = (1 << eTrigger);
    stLRADCConfig.u32LoopCount = 0;
    stLRADCConfig.u32DelayCount = SamplingInterval;

    SetDelayTrigger( &stLRADCConfig);

    // Clear the accumulator & NUM_SAMPLES
    HW_LRADC_CHn_CLR(eChannel, 0xFFFFFFFF);

    // Kick off the LRADC battery measurement
    SetDelayTriggerKick(TRUE, eTrigger);

    //RETAILMSG(1, (TEXT("EnableBatteryMeasurement :: Before Wait for Interrupt\r\n")));

    // Wait for first measurement of battery.  Should occur in 13 LRADC clock
    // cycles from the time of channel kickoff.
    while(!GetInterruptFlag(eChannel)) ;

    //DumpRegister();

    //RETAILMSG(1, (TEXT("EnableBatteryMeasurement -- \r\n")));

    return 0;
}

//------------------------------------------------------------------------------
//
// Function: MeasureBatteryTemperature
//
// This function Enables the Battery measurement.
//
// Parameters:
//      BattTempChannel
//          [in] Specifies the LRADC channel to use for measuring temperature
//
// Returns:
//      returns measured temperaure from the Battery Temp Channel
//
//------------------------------------------------------------------------------
UINT16 LRADCClass::MeasureBatteryTemperature(LRADC_CHANNEL BattTempChannel)
{
    UINT32 u32HighCurrentReading = 0;
    UINT32 u32LowCurrentReading = 0;
    UINT16 u16Temp;
    UINT16 i;
    LRADC_TEMPSENSOR TempSensor;

	// RETAILMSG(TRUE,(TEXT("MeasureBatteryTemperature: use LRADC_CH%d\r\n"), BattTempChannel));
    // Check the channel.  Only channel 0 or 1 should be used since the others
    // are reserved or cannot induce current.
    if(BattTempChannel == LRADC_CH0)
    {
        TempSensor = TEMP_SENSOR0;
    }
    //else if(BattTempChannel == LRADC_CH1)
	// CS&ZHL MAY-23-2012: should be LRADC_CH6 for TEMP_SENSOR_IENABLE1 ?
    else if(BattTempChannel == LRADC_CH6)
    {
        TempSensor = TEMP_SENSOR1;
    }
    else
    {
		RETAILMSG(TRUE,(TEXT("MeasureBatteryTemperature: invalid parameters = %d\r\n"), BattTempChannel));
        return 0;
    }

    // Enable the temperature sensor.
    EnableTempSensorCurrent(TempSensor,TRUE);

    // Setup the temperature sensor for high current measurement.  Wait while
    // the current ramps up.
    SetTempSensorCurrent(TempSensor,TEMP_SENSOR_CURRENT_HIGH_READING);
    //hw_digctl_MicrosecondWait(TEMP_SENSOR_CURRENT_RAMP_DELAY);
    Sleep(10);

    // Set up a loop to take a specified number of samples.  Then take the
    // average of the samples.
    for(i=0; i<NUM_TEMP_READINGS_TO_AVG; i++)
    {
        // Schedule the measurement and wait for it to complete.
        ScheduleChannel(BattTempChannel);
        while(GetInterruptFlag(BattTempChannel) == 0) ;
        ClearInterruptFlag( BattTempChannel );

        // Read the measurement and then clear the accumulator to prepare
        // for the next reading.
        u32HighCurrentReading += GetAccumValue(BattTempChannel);
        ClearAccum( BattTempChannel );
		Sleep(10);
    }
    // Take the average the high current readings.
    u32HighCurrentReading /= NUM_TEMP_READINGS_TO_AVG;


    // Setup the temperature sensor for low current measurement.  Wait while
    // the current ramps up.
    SetTempSensorCurrent(TempSensor,TEMP_SENSOR_CURRENT_LOW_READING);
    //hw_digctl_MicrosecondWait(TEMP_SENSOR_CURRENT_RAMP_DELAY);
    Sleep(10);

    // Set up a loop to take a specified number of samples.  Then take the
    // average of the samples.
    for(i=0; i<NUM_TEMP_READINGS_TO_AVG; i++)
    {
        // Schedule the measurement and wait for it to complete.
        ScheduleChannel(BattTempChannel);
        while(GetInterruptFlag(BattTempChannel) == 0) ;
        ClearInterruptFlag( BattTempChannel );

        // Read the measurement and then clear the accumulator to prepare
        // for the next reading.
        u32LowCurrentReading += GetAccumValue(BattTempChannel);
        ClearAccum( BattTempChannel );
		Sleep(10);
    }
    // Take the average the low current readings.
    u32LowCurrentReading /= NUM_TEMP_READINGS_TO_AVG;

    // Turn off the current to the temperature sensor to save power, and disable
    // the temperature sensing block.
    SetTempSensorCurrent(TempSensor,TEMP_SENSOR_CURRENT_0UA);
    EnableTempSensor(FALSE);

    // Do any LRADC reading to temperature conversions here...
    // Then return the temperature.
    {
        // This is the conversion if using a 1N4148 diode without compensating
        // for any routing impedance to the diode.
        {
            u16Temp = (UINT16)(((u32HighCurrentReading - u32LowCurrentReading) * TEMP_SENSOR_CONVERSION_CONSTANT) / 1000);
        }
    }

    return u16Temp;
}

//------------------------------------------------------------------------------
//
// Function: MeasureDieTemperature
//
// This function get the die temperature.
//
// Parameters:
//
// Returns:
//      returns die temperature.
//
//------------------------------------------------------------------------------
UINT32 LRADCClass::MeasureDieTemperature()
{
    UINT32 u32HighCurrentReading = 0;
    UINT32 u32LowCurrentReading = 0;
    UINT32 u32Temp;
    UINT16 i;

    EnableTempSensor(TRUE);

	BF_WR(LRADC_CTRL4, LRADC0SELECT, 0x9);
    HW_LRADC_CTRL2_CLR(0x1000000);// Clear Channel0's DIVEDE_BY_TWO bit.

    if((HW_LRADC_CTRL1_RD() & BM_LRADC_CTRL1_LRADC0_IRQ_EN) == 0)
        BF_WR(LRADC_CTRL1, LRADC0_IRQ_EN, 0x1);

    BF_WRn(LRADC_CHn, LRADC_CH0, VALUE, 0);

    BF_WR(LRADC_CTRL4, LRADC6SELECT, 0x8);

    if((HW_LRADC_CTRL1_RD() & BM_LRADC_CTRL1_LRADC6_IRQ_EN) == 0)
        BF_WR(LRADC_CTRL1, LRADC6_IRQ_EN, 0x1);

    BF_WRn(LRADC_CHn, LRADC_CH6, VALUE,0);

    Sleep(10);
    // Set up a loop to take a specified number of samples.  Then take the
    // average of the samples.
    for(i = 0; i < NUM_TEMP_READINGS_TO_AVG; i++)
    {
        // Schedule the measurement and wait for it to complete.
        ScheduleChannel(LRADC_CH0);
        ScheduleChannel(LRADC_CH6);
        while((GetInterruptFlag(LRADC_CH0) == 0)||(GetInterruptFlag(LRADC_CH6) == 0));
        ClearInterruptFlag(LRADC_CH0);
        ClearInterruptFlag(LRADC_CH6);

        // Read the measurement and then clear the accumulator to prepare
        // for the next reading.
        u32HighCurrentReading += GetAccumValue(LRADC_CH0);
        u32LowCurrentReading += GetAccumValue(LRADC_CH6);
        ClearAccum(LRADC_CH0);
        ClearAccum(LRADC_CH6);
        Sleep(10);
    }
    // Take the average the low current readings.
    u32HighCurrentReading /= NUM_TEMP_READINGS_TO_AVG;
    u32LowCurrentReading /= NUM_TEMP_READINGS_TO_AVG;

    u32Temp = ((u32HighCurrentReading - u32LowCurrentReading) * DIE_TEMP_SENSOR_CONVERSION_CONSTANT)/4000;

    BF_WR(LRADC_CTRL4, LRADC0SELECT, LRADC_CH0);

    BF_WR(LRADC_CTRL4, LRADC6SELECT, LRADC_CH6);
    //RETAILMSG(TRUE,(TEXT("u32HighCurrentReading = %d\r\n"),u32HighCurrentReading));
    //RETAILMSG(TRUE,(TEXT("u32LowCurrentReading = %d\r\n"),u32LowCurrentReading));

    return u32Temp;
}


UINT32 LRADCClass::MeasureNormalChannel(LRADC_CHANNEL Channel)
{
    UINT32			u32Temp = (UINT32)(-1);
    UINT16			i;

	if((Channel != LRADC_CH0) && (Channel != LRADC_CH1))
	{
		RETAILMSG(TRUE,(TEXT("MeasureNormalChannel: not support LRADC_CH%d\r\n"), Channel));
		goto exit;
	}

	// setup channel
	if(Channel == LRADC_CH0)
	{
		BF_WR(LRADC_CTRL4, LRADC0SELECT, LRADC_CH0);					// LRADC_CH0 connect to external physical channel
		HW_LRADC_CTRL2_SET(0x1000000);									// set Channel0's DIVEDE_BY_TWO bit.
		HW_LRADC_CTRL2_CLR(BM_LRADC_CTRL2_TEMP_SENSOR_IENABLE1);		// Disable Temperature Sensor Current Source.
		HW_LRADC_CTRL2_CLR(BM_LRADC_CTRL2_TEMP_SENSOR_IENABLE0);		// Disable Temperature Sensor Current Source.
		if((HW_LRADC_CTRL1_RD() & BM_LRADC_CTRL1_LRADC0_IRQ_EN) == 0)	// enable Channel0's interrupt
		{
			BF_WR(LRADC_CTRL1, LRADC0_IRQ_EN, 0x1);
		}
		BF_WRn(LRADC_CHn, LRADC_CH0, VALUE, 0);							// clear accum
	}
	else		// -> LRADC_CH1
	{
		BF_WR(LRADC_CTRL4, LRADC1SELECT, LRADC_CH1);					// LRADC_CH0 connect to external physical channel
		HW_LRADC_CTRL2_SET(0x2000000);									// set Channel1's DIVEDE_BY_TWO bit.
		HW_LRADC_CTRL2_CLR(BM_LRADC_CTRL2_TEMP_SENSOR_IENABLE1);		// Disable Temperature Sensor Current Source.
		HW_LRADC_CTRL2_CLR(BM_LRADC_CTRL2_TEMP_SENSOR_IENABLE0);		// Disable Temperature Sensor Current Source.
		if((HW_LRADC_CTRL1_RD() & BM_LRADC_CTRL1_LRADC1_IRQ_EN) == 0)	// enable Channel0's interrupt
		{
			BF_WR(LRADC_CTRL1, LRADC1_IRQ_EN, 0x1);
		}
		BF_WRn(LRADC_CHn, LRADC_CH1, VALUE, 0);							// clear accum
	}

    Sleep(10);
	u32Temp = 0;
    for(i = 0; i < NUM_TEMP_READINGS_TO_AVG; i++)
    {
        // Schedule the measurement and wait for it to complete.
        ScheduleChannel(Channel);
        while((GetInterruptFlag(Channel) == 0));
		ClearInterruptFlag(Channel);
		//while(ScheduleState(Channel));		// wait until schedule bit => 0
		//if(GetInterruptFlag(Channel))
		//{
		//	ClearInterruptFlag(Channel);
		//}

        // Read the measurement and then clear the accumulator to prepare
        // for the next reading.
        u32Temp += GetAccumValue(Channel);
        ClearAccum(Channel);
        Sleep(10);
    }
    // Take the average the low current readings.
    u32Temp /= NUM_TEMP_READINGS_TO_AVG;

exit:
	return u32Temp;
}

//------------------------------------------------------------------------------
//
// Function: MeasureVDD5V
//
// This function get the VDD5V voltage.
//
// Parameters:
//
// Returns:
//      returns VDD5V voltage.
//
//------------------------------------------------------------------------------
UINT32 LRADCClass::MeasureVDD5V()
{
    UINT32 u32VDD5VReading = 0;
    UINT16 i;

    BF_WR(LRADC_CTRL4, LRADC0SELECT, 0xF);
    HW_LRADC_CTRL2_CLR(0x1000000);				// CS&ZHL JUN-6-2012: Clear Channel0's DIVEDE_BY_TWO bit.

    if((HW_LRADC_CTRL1_RD() & BM_LRADC_CTRL1_LRADC0_IRQ_EN) == 0)
        BF_WR(LRADC_CTRL1, LRADC0_IRQ_EN, 0x1);

    BF_WRn(LRADC_CHn, LRADC_CH0, VALUE,0);
    Sleep(10);
    // Set up a loop to take a specified number of samples.  Then take the
    // average of the samples.
    for(i = 0; i < NUM_VDD5V_READINGS_TO_AVG; i++)
    {
        // Schedule the measurement and wait for it to complete.
        ScheduleChannel(LRADC_CH0);

        while(GetInterruptFlag(LRADC_CH0) == 0);
        ClearInterruptFlag( LRADC_CH0 );

        // Read the measurement and then clear the accumulator to prepare
        // for the next reading.
        u32VDD5VReading += GetAccumValue(LRADC_CH0);
        ClearAccum( LRADC_CH0 );
        Sleep(10);
    }
    // Take the average the low current readings.
    u32VDD5VReading /= NUM_VDD5V_READINGS_TO_AVG;

    u32VDD5VReading = u32VDD5VReading * LRADC_VDD5V_SCALE_VALUE / LRADC_FULL_SCALE_VALUE;
    //RETAILMSG(TRUE,(TEXT("u32VDD5VReading = %d\r\n"),u32VDD5VReading));

    return u32VDD5VReading;
}

//------------------------------------------------------------------------------
//
// Function: EnableTempSensorCurrent
//
// This function enables the Temp sensor.
//
// Parameters:
//      None.
//
// Returns:
//      returns TRUE.
//
//------------------------------------------------------------------------------
VOID LRADCClass::EnableTempSensorCurrent(LRADC_TEMPSENSOR eSensor,
                                         BOOL bValue)
{
    switch (eSensor)
    {
    case TEMP_SENSOR0:
        if(bValue)
        {    // Enable the sensor current source
            BF_SET(LRADC_CTRL2, TEMP_SENSOR_IENABLE0);
        }
        else
        {    // Disable the sensor current source
            BF_CLR(LRADC_CTRL2, TEMP_SENSOR_IENABLE0);
        }
        break;
    case TEMP_SENSOR1:
        if(bValue)
        {    // Enable the sensor current source
            BF_SET(LRADC_CTRL2, TEMP_SENSOR_IENABLE1);
        }
        else
        {    // Disable the sensor current source
            BF_CLR(LRADC_CTRL2, TEMP_SENSOR_IENABLE1);
        }
        break;
    default:
        break;
    }
}

//------------------------------------------------------------------------------
//
// Function: SetTempSensorCurrent
//
// This function sets the Temp sensor current.
//
// Parameters:
//      None.
//
// Returns:
//      returns TRUE.
//
//------------------------------------------------------------------------------
VOID LRADCClass::SetTempSensorCurrent(LRADC_TEMPSENSOR TempSensor,
                                      LRADC_CURRENTMAGNITUDE Current)
{
    // Set channel 0 temperature sensor current.  Turn temp sensor on or off
    // depending on the current magnitude.
    if(TempSensor == TEMP_SENSOR0)
    {
        HW_LRADC_CTRL2.B.TEMP_ISRC0 = Current;

        if(Current > TEMP_SENSOR_CURRENT_0UA)
        {
            HW_LRADC_CTRL2.B.TEMP_SENSOR_IENABLE0 = 1;
        }
        else
        {
            HW_LRADC_CTRL2.B.TEMP_SENSOR_IENABLE0 = 0;
        }

    }

    // Set channel 1 temperature sensor current.  Turn temp sensor on or off
    // depending on the current magnitude.
    if(TempSensor == TEMP_SENSOR1)
    {
        HW_LRADC_CTRL2.B.TEMP_ISRC1 = Current;

        if(Current > TEMP_SENSOR_CURRENT_0UA)
        {
            HW_LRADC_CTRL2.B.TEMP_SENSOR_IENABLE1 = 1;
        }
        else
        {
            HW_LRADC_CTRL2.B.TEMP_SENSOR_IENABLE1 = 0;
        }
    }
}

//------------------------------------------------------------------------------
//
// Function: EnableTempSensor
//
// This function enables the TempSensor.
//
// Parameters:
//      bEnable
//          [in] TRUE for Enabling and FALSE for disabling
//
// Returns:
//      returns TRUE.
//
//------------------------------------------------------------------------------
VOID LRADCClass::EnableTempSensor(BOOL bEnable)
{
    HW_LRADC_CTRL2.B.TEMPSENSE_PWD = !bEnable;
}

//------------------------------------------------------------------------------
//
// Function: DumpRegister
//
// This function Dumps the LRADC registers, used for debugging.
//
// Parameters:
//      None.
//
// Returns:
//      returns TRUE.
//
//------------------------------------------------------------------------------
BOOL LRADCClass::DumpRegister(VOID)
{
    RETAILMSG(1, (TEXT("HW_LRADC_CTRL0=0x%08lx\r\n"),        HW_LRADC_CTRL0_RD()));
    RETAILMSG(1, (TEXT("HW_LRADC_CTRL1=0x%08lx\r\n"),        HW_LRADC_CTRL1_RD()));
    RETAILMSG(1, (TEXT("HW_LRADC_CTRL2=0x%08lx\r\n"),        HW_LRADC_CTRL2_RD()));
    RETAILMSG(1, (TEXT("HW_LRADC_CTRL3=0x%08lx\r\n"),        HW_LRADC_CTRL3_RD()));
    RETAILMSG(1, (TEXT("HW_LRADC_STATUS=0x%08lx\r\n"),       HW_LRADC_STATUS_RD()));
    RETAILMSG(1, (TEXT("HW_LRADC_CHn(0)=0x%08lx\r\n"),       HW_LRADC_CHn_RD(0)));
    RETAILMSG(1, (TEXT("HW_LRADC_CHn(1)=0x%08lx\r\n"),       HW_LRADC_CHn_RD(1)));
    RETAILMSG(1, (TEXT("HW_LRADC_CHn(2)=0x%08lx\r\n"),       HW_LRADC_CHn_RD(2)));
    RETAILMSG(1, (TEXT("HW_LRADC_CHn(3)=0x%08lx\r\n"),       HW_LRADC_CHn_RD(3)));
    RETAILMSG(1, (TEXT("HW_LRADC_CHn(4)=0x%08lx\r\n"),       HW_LRADC_CHn_RD(4)));
    RETAILMSG(1, (TEXT("HW_LRADC_CHn(5)=0x%08lx\r\n"),       HW_LRADC_CHn_RD(5)));
    RETAILMSG(1, (TEXT("HW_LRADC_CHn(6)=0x%08lx\r\n"),       HW_LRADC_CHn_RD(6)));
    RETAILMSG(1, (TEXT("HW_LRADC_CHn(7)=0x%08lx\r\n"),       HW_LRADC_CHn_RD(7)));
    RETAILMSG(1, (TEXT("HW_LRADC_DELAYn(0)=0x%08lx\r\n"), HW_LRADC_DELAYn_RD(0)));
    RETAILMSG(1, (TEXT("HW_LRADC_DELAYn(1)=0x%08lx\r\n"), HW_LRADC_DELAYn_RD(1)));
    RETAILMSG(1, (TEXT("HW_LRADC_DELAYn(2)=0x%08lx\r\n"), HW_LRADC_DELAYn_RD(2)));
    RETAILMSG(1, (TEXT("HW_LRADC_DELAYn(3)=0x%08lx\r\n"), HW_LRADC_DELAYn_RD(3)));
    RETAILMSG(1, (TEXT("HW_LRADC_CTRL4=0x%08lx\r\n"),        HW_LRADC_CTRL4_RD()));

    return TRUE;
}
