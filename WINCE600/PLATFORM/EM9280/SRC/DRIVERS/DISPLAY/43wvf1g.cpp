//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  43WVF1G.cpp
//
//  Implementation of class DisplayController43WVF1G which for 43WVF1G-0 4.3
//  WQVGA panel.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)
#include "csp.h"
#include "43wvf1g.h"
#include "display_panel.h"			// CS&ZHL MAY-8-2012: support different types of LCD

//------------------------------------------------------------------------------
// CS&ZHL MAR-07-2012:Description of all supported mode for all supported panel
// CS&ZHL AUG-13-2012:update LCD parameters settings
//------------------------------------------------------------------------------
DISPLAY_PANEL_MODE PanelModeArray[] =
{
    // 320*240 -> 5.7", PCLK = 6.3MHz -> LQ057 
    {
		320, 240, 6300000, 16,	//Width, Height, PClockFreq, BPP
		// LCD panel specific settings
		320,					//dwHWidth          -> DOTCLK_H_ACTIVE;
		16,						//dwHSyncPulseWidth -> DOTCLK_H_PULSE_WIDTH; 
		32,						//dwHFrontPorch     -> DOTCLK_HF_PORCH;
		48,						//dwHBackPorch(including dwHSyncPulseWidth) -> DOTCLK_HB_PORCH; 
								//HTotal = DOTCLK_HB_PORCH + DOTCLK_H_ACTIVE + DOTCLK_HF_PORCH = 400
		240,					//dwVHeight         -> DOTCLK_V_ACTIVE;
		3,						//dwVSyncPulseWidth -> DOTCLK_V_PULSE_WIDTH;
		4,						//dwVFrontPorch     -> DOTCLK_VF_PORCH;
		18,						//dwVBackPorch(including dwVSyncPulseWidth) -> DOTCLK_VB_PORCH;
								//VTotal = DOTCLK_VB_PORCH + DOTCLK_V_ACTIVE + DOTCLK_VF_PORCH = 262
    },
    // 480*272 -> 4.3", PCLK = 9MHz -> LR430LC9001
    {
		480, 272, 9000000, 16,	//Width, Height, PClockFreq, BPP
		// LCD panel specific settings
		480,					//dwHWidth          -> DOTCLK_H_ACTIVE;
		41,						//dwHSyncPulseWidth -> DOTCLK_H_PULSE_WIDTH; 
		2,						//dwHFrontPorch     -> DOTCLK_HF_PORCH;
		43,						//dwHBackPorch(including dwHSyncPulseWidth) -> DOTCLK_HB_PORCH; 
								//HTotal = DOTCLK_HB_PORCH + DOTCLK_H_ACTIVE + DOTCLK_HF_PORCH = 525
		272,					//dwVHeight         -> DOTCLK_V_ACTIVE;
		10,						//dwVSyncPulseWidth -> DOTCLK_V_PULSE_WIDTH;
		2,						//dwVFrontPorch     -> DOTCLK_VF_PORCH;
		12,						//dwVBackPorch(including dwVSyncPulseWidth) -> DOTCLK_VB_PORCH;
								//VTotal = DOTCLK_VB_PORCH + DOTCLK_V_ACTIVE + DOTCLK_VF_PORCH = 286
    },
    // 640*480 -> 5.6", PCLK = 25MHz -> AT056TN52, 
    {
		640, 480, 25000000, 16,	//Width, Height, PClockFreq, BPP
		// LCD panel specific settings
		640,					//dwHWidth          -> DOTCLK_H_ACTIVE;
		10,						//dwHSyncPulseWidth -> DOTCLK_H_PULSE_WIDTH; (other config: 64, 40, 56)
		16,						//dwHFrontPorch     -> DOTCLK_HF_PORCH;
		144,					//dwHBackPorch(including dwHSyncPulseWidth) -> DOTCLK_HB_PORCH; 
								//HTotal = DOTCLK_HB_PORCH + DOTCLK_H_ACTIVE + DOTCLK_HF_PORCH = 800
		480,					//dwVHeight         -> DOTCLK_V_ACTIVE;
		2,						//dwVSyncPulseWidth -> DOTCLK_V_PULSE_WIDTH;
		32,						//dwVFrontPorch     -> DOTCLK_VF_PORCH;
		13,						//dwVBackPorch(including dwVSyncPulseWidth) -> DOTCLK_VB_PORCH;
								//VTotal = DOTCLK_VB_PORCH + DOTCLK_V_ACTIVE + DOTCLK_VF_PORCH = 525
    },
    // 800*480 -> 7.0", PCLK = 33.3MHz -> AT070TN83 V1
    {
		800, 480, 33300000, 16,	//Width, Height, PClockFreq, BPP
		// LCD panel specific settings
#if (defined EM9280) || (defined EM9283)
		800,					//dwHWidth          -> DOTCLK_H_ACTIVE;
		48,						//dwHSyncPulseWidth -> DOTCLK_H_PULSE_WIDTH;
		40,						//dwHFrontPorch     -> DOTCLK_HF_PORCH;
		88,						//dwHBackPorch(including dwHSyncPulseWidth) -> DOTCLK_HB_PORCH; 
								//HTotal = DOTCLK_HB_PORCH + DOTCLK_H_ACTIVE + DOTCLK_HF_PORCH = 928
		480,					//dwVHeight         -> DOTCLK_V_ACTIVE;
		3,						//dwVSyncPulseWidth -> DOTCLK_V_PULSE_WIDTH;
		13,						//dwVFrontPorch     -> DOTCLK_VF_PORCH;
		32,						//dwVBackPorch(including dwVSyncPulseWidth) -> DOTCLK_VB_PORCH;
								//VTotal = DOTCLK_VB_PORCH + DOTCLK_V_ACTIVE + DOTCLK_VF_PORCH = 525
#else	// -> iMX28EVK->Seiko 4.3" 43WVF1G-0
		800,					//dwHWidth          -> DOTCLK_H_ACTIVE;
		10,						//dwHSyncPulseWidth -> DOTCLK_H_PULSE_WIDTH;
		164,					//dwHFrontPorch     -> DOTCLK_HF_PORCH;
		89,						//dwHBackPorch      -> DOTCLK_HB_PORCH;
		480,					//dwVHeight         -> DOTCLK_V_ACTIVE;
		10,						//dwVSyncPulseWidth -> DOTCLK_V_PULSE_WIDTH;
		10,						//dwVFrontPorch     -> DOTCLK_VF_PORCH;
		23,						//dwVBackPorch      -> DOTCLK_VB_PORCH;
#endif	//EM9280 EM9283
    },
    // 800*600 -> 8.4" & 10.4", PCLK = 39.8MHz -> G084SN03_V1 & G104SN03
    {
		800, 600, 39800000, 16,	//Width, Height, PClockFreq, BPP
		// LCD panel specific settings
		800,					//dwHWidth          -> DOTCLK_H_ACTIVE;
		128,					//dwHSyncPulseWidth -> DOTCLK_H_PULSE_WIDTH;
		64,						//dwHFrontPorch     -> DOTCLK_HF_PORCH;
		192,					//dwHBackPorch(including dwHSyncPulseWidth) -> DOTCLK_HB_PORCH; 
								//HTotal = DOTCLK_HB_PORCH + DOTCLK_H_ACTIVE + DOTCLK_HF_PORCH = 1056
		600,					//dwVHeight         -> DOTCLK_V_ACTIVE;
		4,						//dwVSyncPulseWidth -> DOTCLK_V_PULSE_WIDTH;
		8,						//dwVFrontPorch     -> DOTCLK_VF_PORCH;
		20,						//dwVBackPorch(including dwVSyncPulseWidth) -> DOTCLK_VB_PORCH;
								//VTotal = DOTCLK_VB_PORCH + DOTCLK_V_ACTIVE + DOTCLK_VF_PORCH = 628
    }
};

// CS&ZHL MAY-8-2012: handler of LCD controller
static PVOID	pv_HWregLCDIF;


#define MSG_DUMP_REG 0

#define DOTCLK_H_ACTIVE				800
#define DOTCLK_H_PULSE_WIDTH		10
#define DOTCLK_HF_PORCH				164
#define DOTCLK_HB_PORCH				89
#define DOTCLK_H_WAIT_CNT			(DOTCLK_H_PULSE_WIDTH +  (DOTCLK_HB_PORCH) +2)
#define DOTCLK_H_PERIOD				((DOTCLK_HB_PORCH) + DOTCLK_HF_PORCH + DOTCLK_H_ACTIVE + DOTCLK_H_PULSE_WIDTH)

#define DOTCLK_V_ACTIVE             480
#define DOTCLK_V_PULSE_WIDTH		10
#define DOTCLK_VF_PORCH             10
#define DOTCLK_VB_PORCH             23
#define DOTCLK_V_WAIT_CNT			(DOTCLK_V_PULSE_WIDTH + DOTCLK_VB_PORCH)
#define DOTCLK_V_PERIOD				(DOTCLK_VF_PORCH + DOTCLK_VB_PORCH + DOTCLK_V_ACTIVE + DOTCLK_V_PULSE_WIDTH)

#define PIX_CLK						33500

extern DWORD BSPLoadPixelDepthFromRegistry();
DisplayController43WVF1G* DisplayController43WVF1G::SingletonController = NULL;


//------------------------------------------------------------------------------
//
// Function: ~DisplayController43WVF1G
//
// Destructor of DisplayController43WVF1G class.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
DisplayController43WVF1G::~DisplayController43WVF1G()
{
}

//------------------------------------------------------------------------------
//
// Function: GetInstance
//
// This function returns pointer of DisplayController43WVF1G.
//
// Parameters:
//      None.
//
// Returns:
//      Pointer of DisplayController43WVF1G.
//
//------------------------------------------------------------------------------
DisplayController43WVF1G* DisplayController43WVF1G::GetInstance()
{
    if (SingletonController == NULL)
        SingletonController = new DisplayController43WVF1G();

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
void DisplayController43WVF1G::InitBacklight(UINT32 u32Frequency,
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
BOOL DisplayController43WVF1G::DDKIomuxSetupLCDIFPins(BOOL bPoweroff)
{
    BOOL rc = FALSE;

    if(bPoweroff)
    {
#if (defined EM9280) || (defined EM9283)
		// nothing to do in EM9280
#else	// -> iMX28EVK
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
#endif	//EM9280 EM9283
    }
    else
    {
#if (defined EM9280) || (defined EM9283)
		// LCD_D0 & LCD_D1 are used for other purposes in EM9280
#else	// -> iMX28EVK
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D0,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D1,DDK_IOMUX_MODE_00);
#endif	//EM9280 EM9283

		DDKIomuxSetPinMux(DDK_IOMUX_LCD_D2,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D3,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D4,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D5,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D6,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D7,DDK_IOMUX_MODE_00);

#if (defined EM9280) || (defined EM9283)
		// LCD_D8 & LCD_D9 are used for other purposes in EM9280
#else	// -> iMX28EVK
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D8,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D9,DDK_IOMUX_MODE_00);
#endif	//EM9280  EM9283

        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D10,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D11,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D12,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D13,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D14,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D15,DDK_IOMUX_MODE_00);

#if (defined EM9280) || (defined EM9283)
		// LCD_D16 & LCD_D17 are used for other purposes in EM9280
#else	// -> iMX28EVK
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D16,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D17,DDK_IOMUX_MODE_00);
#endif	//EM9280 EM9283

        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D18,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D19,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D20,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D21,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D22,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D23,DDK_IOMUX_MODE_00);

        // setup the pin for LCDIF block
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_VSYNC_0,  DDK_IOMUX_MODE_01);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_ENABLE_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_DOTCLK_0, DDK_IOMUX_MODE_01);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_HSYNC_0,  DDK_IOMUX_MODE_01);

#if (defined EM9280) || (defined EM9283)
		// LCD_RESET is used as GPIO26(IRQ3) in EM9280
#else	// -> iMX28EVK
		DDKIomuxSetPinMux(DDK_IOMUX_LCD_RESET,  DDK_IOMUX_MODE_00);
#endif	//EM9280 EM9283

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
void DisplayController43WVF1G::BSPInitLCDIF(BOOL bReset)
{
    LCDIF_INIT			LcdifInit;
    LCDIFVSYNC			LCDIfVsync;
    LCDIFDOTCLK			sLcdifDotclk;
	PDISPLAY_PANEL_MODE pPanel = NULL;				// CS&ZHL MAR-7-2012: supporting multiple LCD panels
	PHYSICAL_ADDRESS	phyAddr;
	DWORD				dwTranferCountReg;


	//
	// CS&ZHL MAY-8-2012: get LCD type according to the resolution of splash screen
	//
	if (pv_HWregLCDIF == NULL)
	{
		//Mem maps the LCDIF module space for access
		phyAddr.QuadPart = CSP_BASE_REG_PA_LCDIF;
		pv_HWregLCDIF = (PVOID)MmMapIoSpace(phyAddr, 0x1000, FALSE);
		if(pv_HWregLCDIF == NULL)
		{
			ERRORMSG(1, (TEXT("BSPInitLCDIF:: pv_HWregLCDIF map failed!\r\n")));
			return;
		}
	}

	// get current state of LCDC Size Register
	dwTranferCountReg = HW_LCDIF_TRANSFER_COUNT_RD();
	m_dwWidth  = dwTranferCountReg & 0xFFFF;
	m_dwHeight = (dwTranferCountReg >> 16) & 0xFFFF;

	// CS&ZHL MAR-7-2012: get panel format index -> 
	//					  return = 0: no bmp 
	//					  return = 1: 320x240 
	//					  return = 2: 480x272 (4.3" LCD)
	//				      return = 3: 640x480 (default)
	//					  return = 4: 800x480 (7" LCD)
	//
	if((m_dwWidth == 320) && (m_dwHeight == 240))
	{
		RETAILMSG(1, (TEXT("BSPInitLCDIF:: -> 320*240\r\n")));
		pPanel = &PanelModeArray[0];
		//LCDIFSetupLCDIFClock(6400);						// zxw : 3.5" 6.4MHz => 6400KHz
		LCDIFSetupLCDIFClock(pPanel->frequency / 1000);		// CS&ZHL AUG-13-2012: use PCLK in table
	}
	else if((m_dwWidth == 480) && (m_dwHeight == 272))
	{
		RETAILMSG(1, (TEXT("BSPInitLCDIF:: -> 480*272\r\n")));
		pPanel = &PanelModeArray[1];
		//LCDIFSetupLCDIFClock(9000);						// zxw : 4.3" 9MHz => 9000KHz
		LCDIFSetupLCDIFClock(pPanel->frequency / 1000);		// CS&ZHL AUG-13-2012: use PCLK in table
	}
	else if((m_dwWidth == 640) && (m_dwHeight == 480))
	{
		RETAILMSG(1, (TEXT("BSPInitLCDIF:: -> 640*480\r\n")));
		pPanel = &PanelModeArray[2];
		//LCDIFSetupLCDIFClock(25000);						// zxw : 5.6" 25MHz => 25000KHz
		LCDIFSetupLCDIFClock(pPanel->frequency / 1000);		// CS&ZHL AUG-13-2012: use PCLK in table
	}
	else if((m_dwWidth == 800) && (m_dwHeight == 480))
	{
		RETAILMSG(1, (TEXT("BSPInitLCDIF:: -> 800*480\r\n")));
		pPanel = &PanelModeArray[3];
		//LCDIFSetupLCDIFClock(33300);						// zxw : 7", 68.3Hz FrameFreq => 33.3MHz => 33300KHz
		LCDIFSetupLCDIFClock(pPanel->frequency / 1000);		// CS&ZHL AUG-13-2012: use PCLK in table
	}
	else if((m_dwWidth == 800) && (m_dwHeight == 600))
	{
		RETAILMSG(1, (TEXT("BSPInitLCDIF:: -> 800*600\r\n")));
		pPanel = &PanelModeArray[4];
		LCDIFSetupLCDIFClock(pPanel->frequency / 1000);		// CS&ZHL AUG-13-2012: use PCLK in table
	}
	else
	{
		RETAILMSG(1, (TEXT("BSPInitLCDIF:: unkown dispaly format!\r\n")));
		pPanel = NULL;
		// use default display format 800*480
		m_dwWidth  = DOTCLK_H_ACTIVE;
		m_dwHeight = DOTCLK_V_ACTIVE;
		// Start the PIX clock and set frequency
		LCDIFSetupLCDIFClock(PIX_CLK);
	}

    //// Start the PIX clock and set frequency
    //LCDIFSetupLCDIFClock(PIX_CLK);
       
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
    LCDIFInit(&LcdifInit, bReset, FALSE);
    
    LCDIFSetBusMasterMode(TRUE);
    LCDIFSetIrqEnable(LCDIF_IRQ_FRAME_DONE);    
        
    LCDIFSetDataShift(DATA_SHIFT_RIGHT, 0);
    if(32 == m_Bpp )
        LCDIFSetBytePacking(7);
    else
        LCDIFSetBytePacking(0x0F);    

    LCDIfVsync.bOEB                 = FALSE;
    LCDIfVsync.ePolarity            = POLARITY_LOW;
    LCDIfVsync.eVSyncPeriodUnit     = VSYNC_UNIT_HORZONTAL_LINE;
    LCDIfVsync.eVSyncPulseWidthUnit = VSYNC_UNIT_HORZONTAL_LINE;
	//
	// CS&ZHL MAR-7-2012: supporting multiple LCD panels
	//
	if(pPanel != NULL)
	{
		LCDIfVsync.u32PulseWidth    = pPanel->dwVSyncPulseWidth;											//DOTCLK_V_PULSE_WIDTH;
		LCDIfVsync.u32Period        = pPanel->dwVFrontPorch + pPanel->dwVBackPorch + pPanel->dwVHeight;		//DOTCLK_V_PERIOD;
		LCDIfVsync.WaitCount        = pPanel->dwVBackPorch;													//DOTCLK_V_WAIT_CNT;
	}
	else
	{
		LCDIfVsync.u32PulseWidth    = DOTCLK_V_PULSE_WIDTH;
		LCDIfVsync.u32Period        = DOTCLK_V_PERIOD;
		LCDIfVsync.WaitCount        = DOTCLK_V_WAIT_CNT;
	}
    LCDIFSetupVsync(&LCDIfVsync);

    sLcdifDotclk.bEnablePresent  = TRUE;
    sLcdifDotclk.eHSyncPolarity  = POLARITY_LOW;
    sLcdifDotclk.eEnablePolarity = POLARITY_HIGH;
    sLcdifDotclk.eDotClkPolarity = POLARITY_HIGH;
	//
	// CS&ZHL MAR-7-2012: supporting multiple LCD panels
	//
	if(pPanel != NULL)
	{
		sLcdifDotclk.u32HsyncPulseWidth = pPanel->dwHSyncPulseWidth;											//DOTCLK_H_PULSE_WIDTH;
		sLcdifDotclk.u32HsyncPeriod     = pPanel->dwHBackPorch + pPanel->dwHFrontPorch + pPanel->dwHWidth;		//DOTCLK_H_PERIOD;
		sLcdifDotclk.u32HsyncWaitCount  = pPanel->dwHBackPorch;													//DOTCLK_H_WAIT_CNT;
		sLcdifDotclk.u32DotclkWaitCount = pPanel->dwHWidth;														//DOTCLK_H_ACTIVE;
	}
	else
	{
		sLcdifDotclk.u32HsyncPulseWidth = DOTCLK_H_PULSE_WIDTH;
		sLcdifDotclk.u32HsyncPeriod     = DOTCLK_H_PERIOD;
		sLcdifDotclk.u32HsyncWaitCount  = DOTCLK_H_WAIT_CNT;
		sLcdifDotclk.u32DotclkWaitCount = DOTCLK_H_ACTIVE;
	}
    LCDIFSetupDotclk(&sLcdifDotclk);

	//
	// CS&ZHL MAR-7-2012: supporting multiple LCD panels
	//
	if(pPanel != NULL)
	{
		LCDIFSetTransferCount(pPanel->dwHWidth, pPanel->dwVHeight);												//(800,480); 
	}
	else
	{
		LCDIFSetTransferCount(DOTCLK_H_ACTIVE, DOTCLK_V_ACTIVE);
	}

    LCDIFSetSyncSignals(TRUE);
    LCDIFSetDotclkMode(TRUE);

} //DisplayController43WVF1G

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
void DisplayController43WVF1G::BSPResetController()
{
    LCDIFResetController(LCDRESET_LOW);
    Sleep(170); //More than 170ms interval between DISP and Interface signal is required by 43WVF1G panel.
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
//             bPowerOff = TRUE  ( Need power downLCDIF compulsively  power manager )
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DisplayController43WVF1G::DispDrvrPowerHandler(BOOL bOn, BOOL bInit, BOOL bReset,BOOL bPowerOff)
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
// Function: DisplayController43WVF1G
//
// Constructor of DisplayController43WVF1G class.
//
// Parameters:
//      None   
// 
// Returns:
//
//------------------------------------------------------------------------------
DisplayController43WVF1G::DisplayController43WVF1G()
{
	// use default display format 800*480
	m_dwWidth  = DOTCLK_H_ACTIVE;
	m_dwHeight = DOTCLK_V_ACTIVE;
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
DWORD DisplayController43WVF1G::GetWidth()
{
    //return  DOTCLK_H_ACTIVE;
	return	m_dwWidth;
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
DWORD DisplayController43WVF1G::GetHeight()
{
	//return  DOTCLK_V_ACTIVE;
    return  m_dwHeight;
}

//------------------------------------------------------------------------------
//
// Function: BackLightEnable
//
// Enable/Disable BackLight for 43WVF1G panel.
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
void DisplayController43WVF1G::BacklightEnable(BOOL Enable)
{
#ifdef	EM9280
	// use GPIO1_0 as output for LCD_PWR
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D0, DDK_IOMUX_MODE_GPIO);
	DDKGpioEnableDataPin(DDK_IOMUX_LCD_D0, 1);
    if(Enable)
    {
		DDKGpioWriteDataPin(DDK_IOMUX_LCD_D0, 0);	// turn on LCD power, active low
    }
    else
    {
		DDKGpioWriteDataPin(DDK_IOMUX_LCD_D0, 1);	// turn off LCD power, active low
    }
#else
#ifdef EM9283                                      /* SEP20-2012: add by lqk*/
	// use GPIO1_0 as output for LCD_PWR
	DDKIomuxSetPinMux(DDK_IOMUX_LCD_RESET, DDK_IOMUX_MODE_GPIO);
	DDKGpioEnableDataPin(DDK_IOMUX_LCD_RESET, 1);
	if(Enable)
	{
		DDKGpioWriteDataPin(DDK_IOMUX_LCD_RESET, 1);	// turn on LCD power, active low
	}
	else
	{
		DDKGpioWriteDataPin(DDK_IOMUX_LCD_RESET, 0);	// turn off LCD power, active low
	}

#else	// ->iMX28EVK
    if(Enable)
    {
        Sleep(180); //When this panel starts to work, blank data is output for 10 frames first. Hence sleep for a while
        DDKIomuxSetPinMux(DDK_IOMUX_PWM2,DDK_IOMUX_MODE_00);//Set to PWM mode to output PWM, 
                                                            //frequency has been set in backlight
                                                            //driver already.
    }
    else
    {
        DDKIomuxSetPinMux(DDK_IOMUX_PWM2,DDK_IOMUX_MODE_GPIO);  //Set PWM pin to GPIO mode
        DDKGpioEnableDataPin(DDK_IOMUX_PWM2,1);   //Enable as output
        DDKGpioWriteDataPin(DDK_IOMUX_PWM2,0);  //Pull low to display PWM output    
    }
#endif	//EM9283
#endif	//EM9280
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
void DisplayController43WVF1G::InitDisplay()
{
    RETAILMSG(1, (L"Initializing 43WVF1G controller\r\n"));
    
    m_Bpp = BSPLoadPixelDepthFromRegistry();
    
    BSPInitLCDIF(TRUE);
}
