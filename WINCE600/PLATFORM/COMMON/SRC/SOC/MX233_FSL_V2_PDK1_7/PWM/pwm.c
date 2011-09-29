//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pwm.c
//
//  This file contains a DDK interface for the CLK module.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"
#include "hw_pwm.h"
#include "hw_lradc.h"

#ifdef DEBUG
#define ZONE_FUNCTION 1
#else
#define ZONE_FUNCTION 0
#endif

//-----------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Local Variables
PVOID pv_HWRegPWM = NULL;
static CRITICAL_SECTION g_hPwmLock;

//-----------------------------------------------------------------------------
// Local Functions

static void CalcChannelConfig(UINT32 u32PercentActiveDutyCycle,
                              UINT32    *pu32PeriodClocks,
                              PWM_STATE *pActiveState,
                              PWM_STATE *pInactiveState,
                              UINT32    *pu32ActiveClocks);
//------------------------------------------------------------------------------
//
// Function: PwmReset
//
// This function will reset PWM.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PwmReset(void)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PwmReset\r\n")));

    EnterCriticalSection(&g_hPwmLock);

    BF_CLR(PWM_CTRL, SFTRST);
    HW_PWM_CTRL_RD(); // waist some cyles.
    BF_CLR(PWM_CTRL, CLKGATE);
    HW_PWM_CTRL_RD(); // waist some cyles.

    LeaveCriticalSection(&g_hPwmLock);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PwmReset\r\n")));
}
//-----------------------------------------------------------------------------
// Function:  PWMSetClkGate
//
// This Function Enables and disables the clock gating function for the PWM block
// using the CLKGATE.  When enabled, no clock signal is sent to the PWM block.
// Clock gating must be disabled for normal PWM operation.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE
//
//-----------------------------------------------------------------------------
BOOL PWMSetClockGating(BOOL bGating)
{
    if (bGating)
    {
        BF_SET(PWM_CTRL, CLKGATE);
    }
    else
    {    
        BF_CLR(PWM_CTRL, CLKGATE);
    }
    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: PwmInitialize
//
// This function will allocate mapping for PWM registers, initialize critical section,
// reset the PWM. 
//
// Parameters:
//      None
//
// Returns:
//      TRUE - If success
//
//      FALSE - If failure
//
//------------------------------------------------------------------------------
BOOL PwmInitialize(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;
    
    // Turn on the PWM clock
    //DDKClockSetPwmClkGate(FALSE);
    DDKClockSetGatingMode(DDK_CLOCK_GATE_PWM24M_CLK, FALSE);

    if (!pv_HWRegPWM)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_PWM;

        // Map peripheral physical address to virtual address
        pv_HWRegPWM = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWRegPWM == NULL)
        {
            ERRORMSG(1, (TEXT("PwmInitialize:  MmMapIoSpace failed!\r\n")));
            goto Error;
        }
    }

    // Initialize PWM critical section
    InitializeCriticalSection(&g_hPwmLock);

    if(PWMGetChannelPresentMask() != PWM_CHANNEL_ALL_BITS)
    {
        ERRORMSG(1, (_T("PwmInitialize::PWMGetChannelPresentMask =  0x%x\r\n"),PWMGetChannelPresentMask()));
        goto Error;
    }

    PwmReset();

    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PwmInitialize\r\n")));

    rc = TRUE;
    return rc;

Error:
    // Turn off the PWM clock
    //DDKClockSetPwmClkGate(TRUE);
    DDKClockSetGatingMode(DDK_CLOCK_GATE_PWM24M_CLK, TRUE);    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PwmInitialize\r\n")));
    return rc;
}
//------------------------------------------------------------------------------
//
// Function: PwmDeinit
//
// Frees up the register space and deletes critical
// section for deinitialization.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PwmDeinit(void)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++PwmDeinit\r\n")));
    // free the virtual space allocated for PWM memory map
    if (pv_HWRegPWM != NULL)
    {
        MmUnmapIoSpace(pv_HWRegPWM, 0x1000);
        pv_HWRegPWM = NULL;
    }
    // Turn off PWM clock
    PWMSetClockGating(TRUE);

    // Turn off the PWM clock
    //DDKClockSetPwmClkGate(TRUE);
    DDKClockSetGatingMode(DDK_CLOCK_GATE_PWM24M_CLK, TRUE);

    // Delete the critical section
    DeleteCriticalSection(&g_hPwmLock);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("--PwmDeinit\r\n")));
    return;
}

//-----------------------------------------------------------------------------
// Function:  PWMGetChannelPresentMask
//
// This Function Checks which PWM channels are present in hardware Block
//
// Parameters:
//      None.
//
// Returns:
//      Returns Set of PWM_CHANNELBIT bits indicating which channels are present
//
//-----------------------------------------------------------------------------

UINT32 PWMGetChannelPresentMask(VOID)
{
    // Read all the present bits, then shift it down to match the
    // hw_pwm_ChannelBit_t enum bits
    return ((HW_PWM_CTRL_RD() & (BM_PWM_CTRL_PWM4_PRESENT |
                                 BM_PWM_CTRL_PWM3_PRESENT |
                                 BM_PWM_CTRL_PWM2_PRESENT |
                                 BM_PWM_CTRL_PWM1_PRESENT |
                                 BM_PWM_CTRL_PWM0_PRESENT))
            >> BP_PWM_CTRL_PWM0_PRESENT);
}

//-----------------------------------------------------------------------------
// Function:  PWMChSetMultiChipMode
//
// This Function Sets the Multi-chip attachment mode bit for the given PWM channel
//
// Parameters:
//      [in]: Channel - Which channel to setup
//      [in]: bMultiChipMode - Enable/disable multi chip attachment mode
//
// Returns:
//      Returns TRUE
//
//-----------------------------------------------------------------------------
BOOL PWMChSetMultiChipMode(PWM_CHANNEL Channel,BOOL bMultiChipMode)
{
    BW_PWM_PERIODn_MATT(Channel, bMultiChipMode);
    return TRUE;
}

//-----------------------------------------------------------------------------
// Function:  PWMChChSetConfig
//
// This Function Sets PWM clock frequency, period and active/inactive states
// Each channel's config consists of an operating frequency which is a
// divided 24MHz clock.  Also the active and inactive state settings
// define the GPIO pin output during those states.  The period is defined
// by the number of divided clock ticks minus one.  So to set the period for
// 6 divided clocks, set the period to 5.  Note also that calling this
// function will automatically clear multi-chip attachment mode (MATT).
//
//  [in]  Channel - Channel to config
//  [in]  ActiveState - Output during active state
//  [in]  InactiveState - Output during inactive state
//  [in]  u32HzTimes100 - Desired output frequency times 100
//  [in]  u32PercentActiveDutyCycle -
//                 0 to 100 percent desired active duty.
//                 If the given duty cycle is over 100%,
//                 then it will be assumed to be 100%.
//                 If the given duty cycle is greater than 0% and less than 100%,
//                 then the output is guaranteed to oscillate between active and
//                 inactive states.
//
// Returns:
//      Returns TRUE
//
//-----------------------------------------------------------------------------
BOOL PWMChSetConfig(PWM_CHANNEL Channel,
                    PWM_STATE ActiveState,
                    PWM_STATE InactiveState,
                    UINT32 u32HzTimes100,
                    UINT32 u32PercentActiveDutyCycle)
{
    int PreScaler;
    BOOL rc = FALSE;

    // Limit the requested frequency to the highest/lowest possible frequency
    if ((u32HzTimes100 < PWM_MIN_FREQUENCY) ||
        (u32HzTimes100 > PWM_MAX_FREQUENCY))
    {
        goto Exit;
    }

    //Grab the lock
    EnterCriticalSection(&g_hPwmLock);

    /* The formula to determine the number of period (frequency) clocks
     * is:
     *
     * PERIOD = Fclk / (PRESCALAR * FREQUENCY)
     *        = Fclk / FREQUENCY / PRESCALAR
     *
     * Since PRESCALARS are always 2^x (where x is a positive real value)
     * we can subsitute a simple right shift.
     *
     * \note The x100Hz is to allow for 1/100 integer math.
     */
    u32HzTimes100 = PWM_INPUT_FREQ_TIMES_100 / u32HzTimes100;

    /* The PWM is most accurate when you have a large number of PERIOD clocks
     * specified in the registers.  Thus we start from the smallest prescalar
     * to try and fit in the max number of PERIOD clocks.  The register holds
     * a 16-bit (PERIOD - 1) value.  This means we need need to search for
     * a value less than or equal to MAX_SHORT + 1 (or 0x10000).
     *
     * Since we know the prescalar is always a 2^x value we can incrementally
     * divide the number of clocks down until we find the correct value (thus
     * saving some stack by reusing the period_clock variable.
     */

    for (PreScaler = 1; ((PreScaler <= PWM_FREQ_LAST) && (u32HzTimes100 > 0x10000)); PreScaler++)
    {
        u32HzTimes100 >>= reg2div[PreScaler];
    }

    PreScaler--;

    CalcChannelConfig(u32PercentActiveDutyCycle,
                      &u32HzTimes100,
                      &ActiveState,
                      &InactiveState,
                      &u32PercentActiveDutyCycle);

    // Make sure clock is enabled
    //DDKClockSetPwmClkGate(FALSE);
    DDKClockSetGatingMode(DDK_CLOCK_GATE_PWM24M_CLK, FALSE);    
    PWMSetClockGating(FALSE);

    // Seems that you _must_ set the ACTIVE register before the PERIOD
    // register for correct operation.
    BF_CS2n(PWM_ACTIVEn, Channel,
            INACTIVE, u32PercentActiveDutyCycle,
            ACTIVE, 0);

    BF_CS5n(PWM_PERIODn, Channel,
            MATT, 0,
            CDIV, (PWM_INPUTFREQ) PreScaler,
            INACTIVE_STATE, InactiveState,
            ACTIVE_STATE, ActiveState,
            PERIOD, (u32HzTimes100 - 1));

    rc = TRUE;

    LeaveCriticalSection(&g_hPwmLock);

Exit:
    return rc;
}
//-----------------------------------------------------------------------------
// Function: PWMChGetConfig
// This Function Gets the current PWM clock divisor, period and active/inactive states
//
//
// [in]   Channel - Channel to config
// [out]  *pbMultiChipMode - Indicates the channel is in
//                 the multi chip attachment mode.  When this is true, the
//                 channel will output the 24MHz input clock and ignore
//                 the other PWM settings.
// [out]  *pInputFreq - Divided clock frequency
// [out]  *pActiveState - Output during active state
// [out]  *pInactiveState - Output during inactive state
// [out]  *pu16PeriodClocksMinusOne - Number of divided clocks
//              minus 1 that will define the PWM period.
// [out]  *pu16ClocksBeforeActive - Number of clocks before the
//              output transitions from the inactive to active state
// [out]  *pu16ClocksBeforeInactive - Number of clocks before the
//              output transitions from the active to inactive state
//-----------------------------------------------------------------------------

BOOL   PWMChGetConfig(PWM_CHANNEL Channel,
                      UINT8 *pu8MultiChipMode,
                      PWM_INPUTFREQ *pInputFreq,
                      PWM_STATE *pActiveState,
                      PWM_STATE *pInactiveState,
                      UINT16 *pu16PeriodClocksMinusOne,
                      UINT16 *pu16ClocksBeforeActive,
                      UINT16 *pu16ClocksBeforeInactive)
{
    hw_pwm_periodn_t PeriodReg ={0};
    hw_pwm_activen_t ActiveReg ={0};

    PeriodReg.U = HW_PWM_PERIODn_RD(Channel);
    ActiveReg.U = HW_PWM_ACTIVEn_RD(Channel);

    if( pu8MultiChipMode )
    {
        *pu8MultiChipMode = (UINT8)PeriodReg.B.MATT;
    }
    if( pInputFreq != NULL){
        *pInputFreq = (PWM_INPUTFREQ)PeriodReg.B.CDIV;
    }

    if( pActiveState != NULL){
        *pActiveState = (PWM_STATE)PeriodReg.B.ACTIVE_STATE;
    }

    if( pInactiveState != NULL){
        *pInactiveState = (PWM_STATE)PeriodReg.B.INACTIVE_STATE;
    }

    if( pu16PeriodClocksMinusOne!= NULL ){
        *pu16PeriodClocksMinusOne = (UINT16)PeriodReg.B.PERIOD;
    }

    if( pu16ClocksBeforeActive != NULL ){
        *pu16ClocksBeforeActive = (UINT16)ActiveReg.B.ACTIVE;
    }

    if( pu16ClocksBeforeInactive != NULL){
        *pu16ClocksBeforeInactive = (UINT16)ActiveReg.B.INACTIVE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
// Function: PWMChSetDutyCycle
// This Function Modifies the duty cycle of a previously configured PWM
//
// Parameters :
// [in]   Channel - Channel to config
// [in]  ActiveState -   Active   state
// [in]  InactiveState - Inactive state
// [in]  u32PercentActiveDutyCycle - 0 to 100 percent desired active duty
//
// RETURN  :
//           TRUE  if success
//-----------------------------------------------------------------------------
BOOL PWMChSetDutyCycle(PWM_CHANNEL Channel,
                         PWM_STATE ActiveState,
                         PWM_STATE InactiveState,
                         UINT32 u32PercentActiveDutyCycle)
{

    BOOL rc = FALSE;
    UINT32 u32PeriodClocks = 0;
    UINT32 u32ActiveClocks = 0;
    UINT8 bMattMode;
    hw_pwm_periodn_t PeriodReg;

    //Grab the lock
    EnterCriticalSection(&g_hPwmLock);

    // Make sure clock is enabled
    //DDKClockSetPwmClkGate(FALSE);
    DDKClockSetGatingMode(DDK_CLOCK_GATE_PWM24M_CLK, FALSE);       
    PWMSetClockGating(FALSE);

    PWMChGetConfig(Channel, &bMattMode,
               NULL, NULL,
               NULL, (UINT16*) &u32PeriodClocks,
               NULL, NULL);

    /* The hw_pwm returns period_clocks - 1.  Thus we want to
     * increment back up so we can get a more accurate duty cycle.
     */
    u32PeriodClocks++;

    CalcChannelConfig(u32PercentActiveDutyCycle,
                      &u32PeriodClocks,
                      &ActiveState,
                      &InactiveState,
                      &u32ActiveClocks);

    BF_CS2n(PWM_ACTIVEn, Channel,
            ACTIVE, 0,
            INACTIVE, u32ActiveClocks);

    PeriodReg.U = HW_PWM_PERIODn_RD(Channel);
    PeriodReg.B.ACTIVE_STATE = ActiveState;
    PeriodReg.B.INACTIVE_STATE = InactiveState;
    HW_PWM_PERIODn_WR(Channel, PeriodReg.U);

    rc = TRUE;

    LeaveCriticalSection(&g_hPwmLock);
    return rc;
}
//-----------------------------------------------------------------------------
// Function: PWMChOutputEnable
// This Function Enables/Disables one or more PWM channels
//
// Parameters :
// [in]   u32ChannelMask Set of \c hw_pwm_ChannelBit_t bits
// [in]  bEnable        True to enable the channel(s), false to disable
//
// RETURN  :
//           TRUE  if success
//-----------------------------------------------------------------------------
BOOL PWMChOutputEnable(UINT32 u32ChannelMask, BOOL bEnable)
{
    if( bEnable )
        HW_PWM_CTRL_SET(1 << u32ChannelMask);
    else
        HW_PWM_CTRL_CLR(1 << u32ChannelMask);

    return TRUE;
}

//-----------------------------------------------------------------------------
// Function: PWMGetChannelOutputEnabledMask
// This Function returns a bit mask representing which channels are enabled
//
// Parameters :
//      None
// RETURN  :
//            Value of Bitmask indicating which channels are enabled
//-----------------------------------------------------------------------------
UINT32 PWMGetChannelOutputEnabledMask(VOID)
{
    return((HW_PWM_CTRL_RD() & (BM_PWM_CTRL_PWM4_ENABLE |
                                BM_PWM_CTRL_PWM3_ENABLE |
                                BM_PWM_CTRL_PWM2_ENABLE |
                                BM_PWM_CTRL_PWM1_ENABLE |
                                BM_PWM_CTRL_PWM0_ENABLE)));
}
//-----------------------------------------------------------------------------
// Function: PWMChSetAnalogFeedback
// This Function Enables/Disables the PWM2_ANA_CTRL_ENABLE bit of the Control Register.
//
// Parameters :
// [in]  Channel - Channel to config
// [in]  bEnable - True to enable the channel, false to disable
// RETURN  :
//            TRUE on Success
//-----------------------------------------------------------------------------
BOOL   PWMChSetAnalogFeedback(PWM_CHANNEL Channel, BOOL bEnable)
{
    BOOL bRet = FALSE;
    if (Channel != PWM_CHANNEL_2) {
        ERRORMSG(1, (_T("PWMChSetAnalogFeedback:: channel 2 has not configured !\r\n")));
        goto Exit;
    }

    if (bEnable){
        HW_PWM_CTRL_SET(BM_PWM_CTRL_PWM2_ENABLE);
        // Enable PWM output
        HW_PWM_CTRL_SET(BM_PWM_CTRL_PWM2_ANA_CTRL_ENABLE);
    }
    else{
        HW_PWM_CTRL_CLR(BM_PWM_CTRL_PWM2_ENABLE);
        HW_PWM_CTRL_CLR(1 << 5);
    }
    bRet = TRUE;
Exit:
    return bRet;
}
//-----------------------------------------------------------------------------
// Function: PWMChLock
// This Function lock the Channel bit of the Control Register.L
//
// Parameters :
// [in]  Channel - Channel to config
// RETURN  :
//            TRUE on Success
//-----------------------------------------------------------------------------

BOOL   PWMchSetIOMux(UINT32 u32Channel,DDK_IOMUX_PIN_MUXMODE muxmode)
{
    switch(u32Channel)
    {
    case 0:
        DDKIomuxSetPinMux(DDK_IOMUX_PWM0,muxmode);
        break;
    case 1:
        DDKIomuxSetPinMux(DDK_IOMUX_PWM1,muxmode);
        break;
    case 2:
        DDKIomuxSetPinMux(DDK_IOMUX_PWM2,muxmode);
        break;
    case 3:
        DDKIomuxSetPinMux(DDK_IOMUX_PWM3,muxmode);
        break;
    case 4:
        DDKIomuxSetPinMux(DDK_IOMUX_PWM4,muxmode);
        break;
    default:
        break;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
// Function: CalcChannelConfig
//  This Function Calculates the channel config given the duty cycle and period
//
// Using the given requested duty cycle and number of period clocks, this
// function calculates the number of active clocks to achieve the duty cycle.
// Also may modify the active and inactive state settings to reflect a 0%
// or 100% duty cycle.
//
// [in]  u32PercentActiveDutyCycle -
//                  0 to 100 percent desired active duty.
//                  If the given duty cycle is over 100%,
//                  then it will be assumed to be 100%.
//                  If the given duty cycle is greater than 0% and less than 100%,
//                  then the output is guaranteed to oscillate between active and
//                  inactive states.
//  [in,out] *pu32PeriodClocks - Number of clocks for the period
//  [in,out] *pActiveState - Active output state
//  [in,out] *pInactiveState - Inactive output state
//  note When the input duty cycle is 0%, the active state is set to the
//        inactive state to guarantee no active state cycles.
//  [out]    *pu32ActiveClocks - Number of clocks for the active state
//
//  \note When the input duty cycle is 100%, the inactive state is set to the
//       active state to guarantee no inactive state cycles.
//-----------------------------------------------------------------------------
static void CalcChannelConfig(UINT32 u32PercentActiveDutyCycle,
                              UINT32    *pu32PeriodClocks,
                              PWM_STATE *pActiveState,
                              PWM_STATE *pInactiveState,
                              UINT32    *pu32ActiveClocks)
{

    /* Make sure that we provide at least something back to the user */
    *pu32ActiveClocks = 0;

    // You'll need at least 2 clocks per period
    // to get at least 50% duty cycle
    if (*pu32PeriodClocks < 2)
        *pu32PeriodClocks = 2;

    // Check for calc error
    if (*pu32PeriodClocks > 0x10000)
        *pu32PeriodClocks = 0x10000;

    if (u32PercentActiveDutyCycle == 0) {
        // Set the active state to the inactive state
        // so that the channel is always in the inactive state (0% duty)
        *pActiveState = *pInactiveState;
    } else if (u32PercentActiveDutyCycle >= 100) {
        // Set the inactive state to the active state
        // so that the channel is always in the active state (100% duty)
        *pInactiveState = *pActiveState;
    } else {
        *pu32ActiveClocks = (u32PercentActiveDutyCycle *
                             *pu32PeriodClocks / 100);

        // If the duty cycle is between 0 and 100, guarantee oscillation
        if (!*pu32ActiveClocks && u32PercentActiveDutyCycle)
            *pu32ActiveClocks = 1;
    } //if/else
} //CalcChannelConfig


