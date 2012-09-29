//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mx28_timrot.h
//
//-----------------------------------------------------------------------------
#ifndef __MX28_TIMROT_H
#define __MX28_TIMROT_H

//
// Definitions  (Timers and Rotary Decoder)
//

#define TIMER_INTERRUPT_ENABLE          1
#define TIMER_INTERRUPT_DISABLE         0

#define TIMER_MATCH_MODE_ENABLE         1
#define TIMER_MATCH_MODE_DISABLE        0

// Only Timer3 support this mode
#define TIMER_DUTY_CYCLE_MODE_ON        1
#define TIMER_DUTY_CYCLE_MODE_OFF       0

#define TIMER_INPUT_POLARITY_INVERSE    1
#define TIMER_INPUT_POLARITY_NO_CHANGE  0

#define TIMER_UPDATE_RUNNING_COUNT_ON   1
#define TIMER_UPDATE_RUNNING_COUNT_OFF  0

#define TIMER_RELOAD_RUNNING_COUNT_ON   1
#define TIMER_RELOAD_RUNNING_COUNT_OFF  0


//! \brief Enumeration of timer Id
typedef enum _hw_timer_Id
{
    //! \brief TBD
    TIMER0,
    //! \brief TBD
    TIMER1,
    //! \brief TBD
    TIMER2,
    //! \brief TBD
    TIMER3,
    //! \brief TBD
    TIMER_ID_MAX,
    //! \brief TBD
    TIMER_ID_UNKNOWN
} hw_timer_Id_t;

//! \brief Enumeration of timer input clock prescale (PRESCALE)
typedef enum _hw_timer_Prescale
{
    //! \brief TBD
    DIVIDE_BY_1,
    //! \brief TBD
    DIVIDE_BY_2,
    //! \brief TBD
    DIVIDE_BY_4,
    //! \brief TBD
    DIVIDE_BY_8,
} hw_timer_Prescale_t;

//! \brief Enumeration of timer input sources (SELECT)
typedef enum _hw_timer_InputSource
{
    //! \brief TBD
    TIMER_SELECT_NEVER_TICK,
    //! \brief TBD
    TIMER_SELECT_PWM0,
    //! \brief TBD
    TIMER_SELECT_PWM1,
    //! \brief TBD
    TIMER_SELECT_PWM2,
    //! \brief TBD
    TIMER_SELECT_PWM3,
    //! \brief TBD
    TIMER_SELECT_PWM4,
    //! \brief TBD
    TIMER_SELECT_PWM5,
    //! \brief TBD
    TIMER_SELECT_PWM6,
    //! \brief TBD
    TIMER_SELECT_PWM7,
    //! \brief TBD
    TIMER_SELECT_ROTARYA,
    //! \brief TBD9
    TIMER_SELECT_ROTARYB,
    //! \brief 32KHz clock divided from 32KHz crystal
    TIMER_SELECT_32KHZ_XTAL,
    //! \brief 8KHz clock divided from 32KHz crystal
    TIMER_SELECT_8KHZ_XTAL,
    //! \brief 8KHz clock divided from 32KHz crystal
    TIMER_SELECT_4KHZ_XTAL,
    //! \brief 1KHz clock divided from 32KHz crystal
    TIMER_SELECT_1KHZ_XTAL,
    //! \brief xKHz clock prescaled from 24MHz crystal
    TIMER_SELECT_TICK_ALWAYS,
} hw_timer_InputSource_t;

//! Structure for the timer init
typedef struct _hw_timer_SetupStruct
{
    //! \brief Enable or disable the timer interrupt
    BOOL bIrqEnable;
    //! \brief Set input signal polarity
    BOOL bPolarity;
    //! \brief Set match mode
    BOOL bMatchMode;    
    //! \brief Only timer3 supports duty cycle mode;    
    BOOL bDutyCycleMode;
    //! \brief Update the running count when writing the fixed-count
    BOOL bUpdate;
    //! \brief Reload the running-count when it decrements to zero
    BOOL bReload;
    //! \brief Prescale the input clock
    hw_timer_Prescale_t ePrescale;
    //! \brief Select an input source for the timer
    hw_timer_InputSource_t eSelect;
    //! \brief Set the fixed count for the timer
    UINT32 u32FixedCount;
} hw_timer_SetupStruct_t;

#endif //__MX28_TIMROT_H

