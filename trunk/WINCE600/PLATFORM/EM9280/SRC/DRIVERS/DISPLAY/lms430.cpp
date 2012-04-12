//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  LMS430.cpp
//
//  Implementation of class DisplayControllerLMS430 which for LMS430HF02 4.3
//  WQVGA panel.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)
#include "csp.h"
#include "lms430.h"

#define MSG_DUMP_REG			0

#define DOTCLK_H_ACTIVE         480
#define DOTCLK_H_PULSE_WIDTH	41
#define DOTCLK_HF_PORCH         8
#define DOTCLK_HB_PORCH         4
#define DOTCLK_H_WAIT_CNT		(DOTCLK_H_PULSE_WIDTH + DOTCLK_HB_PORCH + 2)
#define DOTCLK_H_PERIOD			(DOTCLK_HB_PORCH + DOTCLK_HF_PORCH + DOTCLK_H_ACTIVE + DOTCLK_H_PULSE_WIDTH)

#define DOTCLK_V_ACTIVE         272
#define DOTCLK_V_PULSE_WIDTH    10
#define DOTCLK_VF_PORCH         4
#define DOTCLK_VB_PORCH         2
#define DOTCLK_V_WAIT_CNT		(DOTCLK_V_PULSE_WIDTH + DOTCLK_VB_PORCH)
#define DOTCLK_V_PERIOD			(DOTCLK_VF_PORCH + DOTCLK_VB_PORCH + DOTCLK_V_ACTIVE + DOTCLK_V_PULSE_WIDTH)

#define PIX_CLK					9200

extern DWORD BSPLoadPixelDepthFromRegistry();

DisplayControllerLMS430* DisplayControllerLMS430::SingletonController = NULL;


//------------------------------------------------------------------------------
//
// Function: ~DisplayControllerLMS430
//
// Destructor of DisplayControllerLMS430 class.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
DisplayControllerLMS430::~DisplayControllerLMS430()
{
} //~DisplayControllerLMS430

//------------------------------------------------------------------------------
//
// Function: GetInstance
//
// This function returns pointer of DisplayControllerLMS430.
//
// Parameters:
//      None.
//
// Returns:
//      Pointer of DisplayControllerLMS430.
//
//------------------------------------------------------------------------------
DisplayControllerLMS430* DisplayControllerLMS430::GetInstance()
{
    if (SingletonController == NULL)
        SingletonController = new DisplayControllerLMS430();

    return SingletonController;
} //GetInstance

//------------------------------------------------------------------------------
//
// Function: InitBacklight
//
// This function is used to Initialize black light. Since it is not handled here,
// thus do nothing
//
// Parameters:
//      u32Frequency
//          [in] Frequency of PWM for black light
//
//      u8DutyCycle
//          [in] Duty cycle of PWM for black light
// Returns:
//      None
//
//------------------------------------------------------------------------------
void DisplayControllerLMS430::InitBacklight(UINT32 u32Frequency,
                                             UINT8 u8DutyCycle)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(u32Frequency);
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(u8DutyCycle);

} //InitBacklight


//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSetupLCDIFPins
//  This function enables the PIN MUX for LCDIF block
//
//  Parameters:
//      [IN] BOOL bPowerOff if TRUE: configure all the LCDIF pin to GPIO mode and 
//                                   trisate.
//                             FALSE: Configure the LCDIF pin for normal use. 
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DisplayControllerLMS430::DDKIomuxSetupLCDIFPins(BOOL bPoweroff)
{
    BOOL rc = FALSE;

    if(bPoweroff)
    {


        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D16,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D17,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D18,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D19,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D20,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D21,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D22,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D23,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_DOTCLK_0, DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_HSYNC_0,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_VSYNC_0,DDK_IOMUX_MODE_GPIO);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_ENABLE_0, DDK_IOMUX_MODE_GPIO);
    }
    else
    {

        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D0,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D1,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D2,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D3,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D4,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D5,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D6,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D7,DDK_IOMUX_MODE_00);

        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D8,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D9,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D10,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D11,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D12,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D13,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D14,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D15,DDK_IOMUX_MODE_00);

        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D16,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D17,DDK_IOMUX_MODE_00);

        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D18,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D19,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D20,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D21,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D22,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D23,DDK_IOMUX_MODE_00);

        // setup the pin for LCDIF block
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_VSYNC_0,  DDK_IOMUX_MODE_01);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_ENABLE_0,   DDK_IOMUX_MODE_01);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_DOTCLK_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_HSYNC_0,  DDK_IOMUX_MODE_01);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_RESET,  DDK_IOMUX_MODE_00);

        // Set pin drive to 8mA,enable pull up,3.3V
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D0, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D1, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D2, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D3, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D4, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D5, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D6, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D7, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D8, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D9, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D10, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D11, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D12, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D13, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D14, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D15, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D16, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D17, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D17, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D18, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D19, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D20, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D21, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D22, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D23, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);        

        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_DOTCLK_0, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_ENABLE_0, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_VSYNC_0, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_HSYNC_0, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);                                                 
    }   

    rc = TRUE;
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPInitLCDIF
//  This function initializes LCDIF module
//
//  Parameters:
//      bReset
//        [IN] bReset = FALSE ( not need to reset LCDIF compulsively)
//             bReset = TRUE ( Need reset LCDIF compulsively, for mode change and power manager )
//
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
void DisplayControllerLMS430::BSPInitLCDIF(BOOL bReset)
{

    LCDIF_INIT LcdifInit;
    LCDIFVSYNC LCDIfVsync;
    LCDIFDOTCLK sLcdifDotclk;

    // Start the PIX clock and set frequency
    LCDIFSetupLCDIFClock(PIX_CLK);
       
    LcdifInit.bBusyEnable = FALSE;
    LcdifInit.eBusMode = BUSMODE_8080;
    LcdifInit.eReset = LCDRESET_HIGH;
    LcdifInit.eDataSwizzle = NO_SWAP;
    LcdifInit.eCscSwizzle = NO_SWAP;

    switch(m_Bpp)
    {
    case 16:
        LcdifInit.eWordLength = WORDLENGTH_16BITS;  
        break;
    case 24:
        LcdifInit.eWordLength = WORDLENGTH_24BITS;
        break;        
    case 32:    
        LcdifInit.eWordLength = WORDLENGTH_24BITS;           
        break;
    default:
        RETAILMSG(1,(_T("Pixel Width invalid, will use 16 bits as default")));
        m_Bpp = 16;        
        LcdifInit.eWordLength = WORDLENGTH_16BITS;       
        break;
    }

    LcdifInit.eBusWidth = LCDIF_BUS_WIDTH_24BIT;

    LcdifInit.Timing.BYTE.u8DataSetup = 1;
    LcdifInit.Timing.BYTE.u8DataHold  = 1;
    LcdifInit.Timing.BYTE.u8CmdSetup  = 1;
    LcdifInit.Timing.BYTE.u8CmdHold   = 1; 

    DDKIomuxSetupLCDIFPins(FALSE);    
    LCDIFInit(&LcdifInit, bReset,FALSE);
    
    LCDIFSetBusMasterMode(TRUE);
    LCDIFSetIrqEnable(LCDIF_IRQ_FRAME_DONE);    
        
    LCDIFSetDataShift(DATA_SHIFT_RIGHT, 0);
    if(32 == m_Bpp )
        LCDIFSetBytePacking(7);
    else
        LCDIFSetBytePacking(0x0F);    

    LCDIfVsync.bOEB = FALSE;
    LCDIfVsync.ePolarity = POLARITY_LOW;
    LCDIfVsync.eVSyncPeriodUnit = VSYNC_UNIT_HORZONTAL_LINE;
    LCDIfVsync.eVSyncPulseWidthUnit = VSYNC_UNIT_HORZONTAL_LINE;
    LCDIfVsync.u32PulseWidth = DOTCLK_V_PULSE_WIDTH;
    LCDIfVsync.u32Period    = DOTCLK_V_PERIOD;
    LCDIfVsync.WaitCount    = DOTCLK_V_WAIT_CNT;
    
    LCDIFSetupVsync(&LCDIfVsync);

    sLcdifDotclk.bEnablePresent     = TRUE;

    sLcdifDotclk.eHSyncPolarity     = POLARITY_LOW;
    sLcdifDotclk.eEnablePolarity    = POLARITY_HIGH;
    sLcdifDotclk.eDotClkPolarity    = POLARITY_HIGH;

    sLcdifDotclk.u32HsyncPulseWidth = DOTCLK_H_PULSE_WIDTH;
    sLcdifDotclk.u32HsyncPeriod     = DOTCLK_H_PERIOD;
    sLcdifDotclk.u32HsyncWaitCount  = DOTCLK_H_WAIT_CNT;
    sLcdifDotclk.u32DotclkWaitCount = DOTCLK_H_ACTIVE;
    
    LCDIFSetupDotclk(&sLcdifDotclk);
    
    LCDIFSetTransferCount(DOTCLK_H_ACTIVE, DOTCLK_V_ACTIVE);
    
    LCDIFSetSyncSignals(TRUE);
    LCDIFSetDotclkMode(TRUE);

}

//-----------------------------------------------------------------------------
//
//  Function: BSPResetController
//  This function resets display control panel according different panel
//
//  Parameters:
//      None.
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
void DisplayControllerLMS430::BSPResetController()
{
    LCDIFResetController(LCDRESET_LOW);
    Sleep(100); //More than 5 frames time interval between PON and Interface signal is required by LMS430 panel.
}

//------------------------------------------------------------------------------
//
//  FUNCTION:     DispDrvrPowerHandler
//
//  DESCRIPTION:  
//  This function controls the power state of the LCD. bInit and bReset is just used for 
//  QVGA panel. 
//  QVGA panel have to set the panel param while power up and down,  bInit indicates 
//  whether this function is called by power manager, cause using system call function may have 
//  problem in PM process. 
//  bReset is used to do the workaround for the issue that when NAND is initializing for the first 
//  time, reset LCDIF will lead to no display. bReset is just to indicate whether it is the case mentioned.
//  bPowerOff is used to do the workaround for the  TVOUT conversion stress test issue
//  FALSE for TVOUT conversion not power down LCDIF,TRUE for LCD Power Management. 
//
// Parameters:
//      bOn
//        [IN] bOn = FALSE ( Power down )
//             bOn = TRUE ( Power up )
//      bInit
//        [IN] bInit = FALSE ( Power manager call )
//             bInit = TRUE ( No power manager call )
//      bReset
//        [IN] bReset = FALSE ( not need to reset LCDIF compulsively)
//             bReset = TRUE ( Need reset LCDIF compulsively, for mode change and power manager )
//      bPowerOff
//        [IN] bPowerOff = FALSE ( not need to power down  LCDIF compulsively)
//             bPowerOff  = TRUE  ( Need power downLCDIF compulsively  power manager )
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DisplayControllerLMS430::DispDrvrPowerHandler(BOOL bOn, BOOL bInit, BOOL bReset,BOOL bPowerOff)
{
    UNREFERENCED_PARAMETER(bInit);

    if(bOn)
    {
        BSPInitLCDIF(bReset);
    }
    else
    {
        BSPResetController();
        LCDIFPowerDown(bPowerOff);
    }
}

//------------------------------------------------------------------------------
//
// Function: DisplayControllerLMS430
//
// Constructor of DisplayControllerLMS430 class.
//
// Parameters:
//      None   
// 
// Returns:
//
//------------------------------------------------------------------------------
DisplayControllerLMS430::DisplayControllerLMS430()
{
}

//------------------------------------------------------------------------------
//
// Function: GetWidth
//
// This function returns the number of horizontal pixels of display panel.
//
// Parameters:
//      None   
// 
// Returns:
//      Number of pixels
//
//------------------------------------------------------------------------------
DWORD DisplayControllerLMS430::GetWidth()
{
    return  DOTCLK_H_ACTIVE;
}

//------------------------------------------------------------------------------
//
// Function: GetHeight
//
// This function returns the number of vertical pixels of display panel.
//
// Parameters:
//      None   
// 
// Returns:
//      Number of pixels
//
//------------------------------------------------------------------------------
DWORD DisplayControllerLMS430::GetHeight()
{
    return  DOTCLK_V_ACTIVE;
}

//------------------------------------------------------------------------------
//
// Function: BackLightEnable
//
// Enable/Disable BackLight.
// Cause Backlight driver can handle this display panel well 
// in supend and resume case, nothing need to be done here.
//
// Parameters:
//      Enable.
//          [in] TRUE.     Enable backlight
//                FALSE.    Disable backlight
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DisplayControllerLMS430::BacklightEnable(BOOL Enable)
{
    UNREFERENCED_PARAMETER(Enable);
}

//------------------------------------------------------------------------------
//
// Function: InitDisplay
//
// This function initializes display controller. 
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void DisplayControllerLMS430::InitDisplay()
{
    RETAILMSG(1, (L"Initializing LMS430 controller\r\n"));

    m_Bpp = BSPLoadPixelDepthFromRegistry();

    BSPInitLCDIF(FALSE);
}

