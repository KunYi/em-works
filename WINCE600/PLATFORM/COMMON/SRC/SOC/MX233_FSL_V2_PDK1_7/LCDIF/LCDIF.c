//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  LCDIF.c
//
//
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#ifdef BOOTLOADER
#include <oal.h>
#endif
#pragma warning(pop)
#include "csp.h"
#include "LCDIF.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
static PVOID pv_HWregLCDIF;

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions
VOID LCDIFPowerDown()
{
    //Power down the LCDIF block
    HW_LCDIF_CTRL1.B.RESET  = LCDRESET_LOW;    //Set LCD_RESET output signal to low    

    Sleep(100);
    
    // Turn off the clock gate
    HW_LCDIF_CTRL_SET(BM_LCDIF_CTRL_CLKGATE);

    //Powerdown the LCDIF block
    while (HW_LCDIF_CTRL.B.SFTRST != 1)
        BF_SETV(LCDIF_CTRL,SFTRST,1);

}
static void LCDIFReset(BOOL bReset)
{

    if(bReset)
    {
        if (HW_LCDIF_CTRL.B.CLKGATE == 0)
        {
            // if clock is not gated, stop the block first
            // else the next DMA request can freeze the system
            HW_LCDIF_CTRL.B.RUN = 0;
            while (HW_LCDIF_CTRL.B.RUN == 1);
        }
    
        while (HW_LCDIF_CTRL.B.CLKGATE != 0)
            HW_LCDIF_CTRL.B.CLKGATE = 0;
    
        while (HW_LCDIF_CTRL.B.SFTRST != 1)
            HW_LCDIF_CTRL.B.SFTRST = 1;
    
        while (HW_LCDIF_CTRL.B.CLKGATE != 0)
            HW_LCDIF_CTRL.B.CLKGATE = 0;
    
        while (HW_LCDIF_CTRL.B.SFTRST != 0)
            HW_LCDIF_CTRL.B.SFTRST = 0;
    
        while (HW_LCDIF_CTRL.B.CLKGATE != 0)
            HW_LCDIF_CTRL.B.CLKGATE = 0;
    
        HW_LCDIF_CTRL_RD();
        HW_LCDIF_CTRL_RD();
        HW_LCDIF_CTRL_RD();
    
        while (HW_LCDIF_CTRL.B.CLKGATE != 0)
            HW_LCDIF_CTRL.B.CLKGATE = 0;
    }

    else
    {
        if(HW_LCDIF_CTRL.B.SFTRST == 0)
        {
            //having been reseted
            return;
        }
        
        RETAILMSG(1,(_T("LCDIFReset+++\r\n")));

        HW_LCDIF_CTRL.B.SFTRST = 0;
        HW_LCDIF_CTRL.B.CLKGATE = 0;
        
        HW_LCDIF_CTRL_RD();
        HW_LCDIF_CTRL_RD();
        HW_LCDIF_CTRL_RD();
        RETAILMSG(1,(_T("LCDIFReset---\r\n")));
    }
       
}
//-----------------------------------------------------------------------------
//
//  Function: LCDIFSetupIOMUXPin()
//
//  Parameters:
//
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL LCDIFSetupIOMUXPin()
{
    // Setup the PINMUX
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D00,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D01,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D02,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D03,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D04,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D05,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D06,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D07,DDK_IOMUX_MODE_00);

    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D08,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D09,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D10,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D11,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D12,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D13,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D14,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D15,DDK_IOMUX_MODE_00);

    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D16,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D17,DDK_IOMUX_MODE_00);

    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_LCD_D18,DDK_IOMUX_MODE_01);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_LCD_D19,DDK_IOMUX_MODE_01);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_LCD_D20,DDK_IOMUX_MODE_01);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_LCD_D21,DDK_IOMUX_MODE_01);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_LCD_D22,DDK_IOMUX_MODE_01);
    DDKIomuxSetPinMux(DDK_IOMUX_GPMI_LCD_D23,DDK_IOMUX_MODE_01);

    // setup the rest of the pin
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_RESET,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_DOTCLK,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_ENABLE,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_HSYNC,DDK_IOMUX_MODE_00);

    // Enable Vsyn at lcd_busy pin
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_VSYNC,DDK_IOMUX_MODE_00);

    // control LCD_RS, LCD_WR, and LCD_CS using GPIO mode
    // LCD_RS is bank 1  pin 19
    // LCD_WR is bank 1, pin 20
    // LCD_CS is bank 1, pin 21
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_RS,DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_WR,DDK_IOMUX_MODE_GPIO);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_CS,DDK_IOMUX_MODE_GPIO);

    // enable them as outputs
    DDKGpioWriteDataPin(DDK_IOMUX_LCD_RS, 0);
    DDKGpioWriteDataPin(DDK_IOMUX_LCD_WR, 0);
    DDKGpioWriteDataPin(DDK_IOMUX_LCD_CS, 1);

    DDKGpioEnableDataPin(DDK_IOMUX_LCD_RS,1);
    DDKGpioEnableDataPin(DDK_IOMUX_LCD_WR,1);
    DDKGpioEnableDataPin(DDK_IOMUX_LCD_CS,1);

    // Set pin drive to 8mA,enable pull up,3.3V
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D00, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D01, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D02, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D03, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D00, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D04, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D05, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D06, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D07, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D08, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D09, 
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
    
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_DOTCLK, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_ENABLE, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_VSYNC, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_LCD_HSYNC, 
             DDK_IOMUX_PAD_DRIVE_8MA, 
             DDK_IOMUX_PAD_PULL_ENABLE,
             DDK_IOMUX_PAD_VOLTAGE_3V3);

    return TRUE;
}
//-----------------------------------------------------------------------------
//
//  Function: LCDIFInit()
//
//    Initializes the LCDIF for use.
//    Takes the LCDIF block out of reset and ungates the clock source.  Sets up
//    the HW_LCDIF_CTRL and HW_LCDIF_TIMING registers with the given settings.
//
//  Parameters:
//           pLCDIFInit Pointer to init struct to use for settings
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL LCDIFInit(LCDIF_INIT *pLCDIFInit,BOOL bReset)
{
    BOOL rc = FALSE;

#ifndef BOOTLOADER

    // don't reallocate io space
    // if already allocated
    if (pv_HWregLCDIF == NULL)
    {
    PHYSICAL_ADDRESS phyAddr;

    //Mem maps the LCDIF module space for access
    phyAddr.QuadPart = CSP_BASE_REG_PA_LCDIF;
    pv_HWregLCDIF = (PVOID)MmMapIoSpace(phyAddr, 0x1000, FALSE);
    }
#else
    pv_HWregLCDIF = (PVOID)OALPAtoUA(CSP_BASE_REG_PA_LCDIF);
#endif


    if (pv_HWregLCDIF  ==  NULL)
    {
        ERRORMSG (1, (TEXT("LCDIFInit: VirtualAlloc failed!\r\n")) );
        rc = FALSE;
        goto CleanUp;
    }

    LCDIFReset(bReset);
  
 // RESET LCDIF and Turn ON Clock for LCDIF module
    
    
    HW_LCDIF_CTRL1.B.RESET  = 1;
    Sleep(1);
    HW_LCDIF_CTRL1.B.RESET  = 0;

    // dump version number
    DEBUGMSG(1, (L"LCDIF: version %d.%d.%d\r\n",
        HW_LCDIF_VERSION.B.MAJOR,
        HW_LCDIF_VERSION.B.MINOR,
        HW_LCDIF_VERSION.B.STEP ));

    // these bits are not cleared by SFTRST and
    // so must be manually configured to a known state
    // following SFTRST

    HW_LCDIF_PIN_SHARING_CTRL0.B.MUX_OVERRIDE = 0x03;
    HW_LCDIF_PIN_SHARING_CTRL0.B.PIN_SHARING_ENABLE = 0;
    HW_LCDIF_PIN_SHARING_CTRL1.B.THRESHOLD1 = 0;
    HW_LCDIF_PIN_SHARING_CTRL2.B.THRESHOLD2 = 0;
    HW_LCDIF_CTRL1.B.RESET  = pLCDIFInit->eReset;
 
    
    BF_CS2(LCDIF_CTRL1,
           MODE86, pLCDIFInit->eBusMode,
           BUSY_ENABLE, pLCDIFInit->bBusyEnable);


    BF_CS4(LCDIF_CTRL,
           WORD_LENGTH, pLCDIFInit->eWordLength,
           LCD_DATABUS_WIDTH, pLCDIFInit->eBusWidth,
           INPUT_DATA_SWIZZLE, pLCDIFInit->eDataSwizzle,
           CSC_DATA_SWIZZLE, pLCDIFInit->eCscSwizzle);

    BF_CS4(LCDIF_TIMING,
           DATA_SETUP, pLCDIFInit->Timing.BYTE.u8DataSetup,
           DATA_HOLD, pLCDIFInit->Timing.BYTE.u8DataHold,
           CMD_SETUP, pLCDIFInit->Timing.BYTE.u8CmdSetup,
           CMD_HOLD, pLCDIFInit->Timing.BYTE.u8CmdHold);

   
    BW_LCDIF_CSC_COEFF0_C0(0x41);    //0.257X256 = 65
    BW_LCDIF_CSC_COEFF0_CSC_SUBSAMPLE_FILTER(2);    //co-sited

    BW_LCDIF_CSC_COEFF1_C1(0x81);    //0.504x256 = 129
    BW_LCDIF_CSC_COEFF1_C2(0x19);    //0.098x256 = 25

    BW_LCDIF_CSC_COEFF2_C3(0x3DB);    //-0.148x256 = -37
    BW_LCDIF_CSC_COEFF2_C4(0x3B6);    //-0.291x256 = -74

    BW_LCDIF_CSC_COEFF3_C5(0x70);    //0.439x256 = 112
    BW_LCDIF_CSC_COEFF3_C6(0x70);    //0.439x256 = 112

    BW_LCDIF_CSC_COEFF4_C7(0x3A2);    //-0.368x256 = -94
    BW_LCDIF_CSC_COEFF4_C8(0x3EE);    //-0.071x256 = -18

    BW_LCDIF_CSC_OFFSET_CBCR_OFFSET(128);
    BW_LCDIF_CSC_OFFSET_Y_OFFSET(16);

    //limiting values to be applied in both YCBCR input and CSC
    BW_LCDIF_CSC_LIMIT_CBCR_MIN (16);
    BW_LCDIF_CSC_LIMIT_CBCR_MAX (240);
    BW_LCDIF_CSC_LIMIT_Y_MIN (16);
    BW_LCDIF_CSC_LIMIT_Y_MAX (235);

    HW_LCDIF_DVICTRL0.B.START_TRS           = 0           ;
    HW_LCDIF_DVICTRL0.B.H_ACTIVE_CNT        =0    ;
    HW_LCDIF_DVICTRL0.B.H_BLANKING_CNT      = 0   ;
    HW_LCDIF_DVICTRL0.B.V_LINES_CNT         = 0      ;
    HW_LCDIF_DVICTRL1.B.F1_START_LINE       = 0  ;
    HW_LCDIF_DVICTRL1.B.F1_END_LINE         = 0  ;
    HW_LCDIF_DVICTRL1.B.F2_START_LINE       = 0  ;
    HW_LCDIF_DVICTRL2.B.F2_END_LINE         = 0    ;
    HW_LCDIF_DVICTRL2.B.V1_BLANK_START_LINE =0 ;
    HW_LCDIF_DVICTRL2.B.V1_BLANK_END_LINE   = 0   ;
    HW_LCDIF_DVICTRL3.B.V2_BLANK_START_LINE = 0;
    HW_LCDIF_DVICTRL3.B.V2_BLANK_END_LINE   = 0   ;
    HW_LCDIF_DVICTRL4.B.Y_FILL_VALUE        = 0       ;
    HW_LCDIF_DVICTRL4.B.CB_FILL_VALUE       =0      ;
    HW_LCDIF_DVICTRL4.B.CR_FILL_VALUE       = 0      ;
    HW_LCDIF_DVICTRL4.B.H_FILL_CNT          = 0       ;
     
    

    rc = TRUE;

CleanUp:
    return (rc);
}

//-----------------------------------------------------------------------------
//
//  Function: LCDIFSetDataShift()
//
//
//
//
//
//  Parameters:
//
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
VOID LCDIFSetDataShift(LCDIF_DATASHIFTDIR dir, UINT8 NoBits)
{
    HW_LCDIF_CTRL.B.DATA_SHIFT_DIR = dir;
    HW_LCDIF_CTRL.B.SHIFT_NUM_BITS = NoBits & 0x1F;
}
//-----------------------------------------------------------------------------
//
//  Function:LCDIFSetBytePacking()
//
//
//
//
//
//  Parameters:
//
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void LCDIFSetBytePacking(UINT8 ValidBytes)
{
    HW_LCDIF_CTRL1.B.BYTE_PACKING_FORMAT = ValidBytes & 0x0F;
}
//-----------------------------------------------------------------------------
//
//  Function:LCDIFSetupVsync()
//  Setup VSYNC functionality within the LCDIF.  If VSYNC is setup as an
//  input then the only valid fields are the active polarity, and wait count.
//
//  Parameters:[in] Pointer to the LCDIFVSYNC configuration structure
//
//        pLCDIFVsync->bOEB[IN] :output_enable TRUE setup the VSYNC as an output.
//                                FALSE setup VSYNC as an input.
//
//        pLCDIFVsync->ePolarity[IN]: polarity Setup VSYNC as active HIGH or LOW.
//        pLCDIFVsync->eVSyncPulseWidthUnit[IN]:    VYSNC timing will be either in PIXCLK,
//                                                or in number of horizontal lines.
//        pLCDIFVsync->eVSyncPeriodUnit[IN]:  VYSNC timing will be either in PIXCLK, or
//                                            in number of horizontal lines.
//        pLCDIFVsync->u32PulseWidth[IN]:     Total number of PIXCLKs/horizontal lines the
//                                         active period will occur.
//        pLCDIFVsync->u32Period[IN]:  Total number of PIXCLKs/horizontal lines between
//                                             leading edge of active period to next leading edge
//                                     of active period.
//        pLCDIFVsync->WaitCount[IN]: Total number of PIXCLKs to wait before starting
//                                    DMA transfer after leading edge of active period.
//                                    Vertical Back Porch)
//
//  Returns:
//      Returns TRUE if All registers were set properly, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL LCDIFSetupVsync(LCDIFVSYNC *pLCDIFVsync)
{
    BOOL rc = FALSE;

    HW_LCDIF_VDCTRL0.B.VSYNC_OEB = pLCDIFVsync->bOEB;
    HW_LCDIF_VDCTRL0.B.VSYNC_POL = pLCDIFVsync->ePolarity;
    HW_LCDIF_VDCTRL0.B.VSYNC_PULSE_WIDTH_UNIT = pLCDIFVsync->eVSyncPulseWidthUnit;
    HW_LCDIF_VDCTRL0.B.VSYNC_PERIOD_UNIT = pLCDIFVsync->eVSyncPeriodUnit;
    HW_LCDIF_VDCTRL0.B.VSYNC_PULSE_WIDTH = pLCDIFVsync->u32PulseWidth;

    HW_LCDIF_VDCTRL1.B.VSYNC_PERIOD = pLCDIFVsync->u32Period;


    HW_LCDIF_VDCTRL3_CLR(BM_LCDIF_VDCTRL3_VERTICAL_WAIT_CNT);

    HW_LCDIF_VDCTRL3_SET(pLCDIFVsync->WaitCount); 
    rc = TRUE;

    return (rc);
}
//-----------------------------------------------------------------------------
//
//  Function:LCDIFSetupDotclk()
//
//
//
//
//
//  Parameters:
//
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL LCDIFSetupDotclk(LCDIFDOTCLK *pLCDIFDotclk )
{
    BOOL rc = FALSE;

    HW_LCDIF_VDCTRL0.B.ENABLE_PRESENT = pLCDIFDotclk->bEnablePresent;
    HW_LCDIF_VDCTRL0.B.HSYNC_POL = pLCDIFDotclk->eHSyncPolarity;
    HW_LCDIF_VDCTRL0.B.ENABLE_POL = pLCDIFDotclk->eEnablePolarity;
    HW_LCDIF_VDCTRL0.B.DOTCLK_POL = pLCDIFDotclk->eDotClkPolarity;

    HW_LCDIF_VDCTRL2_WR((BF_LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH(pLCDIFDotclk->u32HsyncPulseWidth) |
                         BF_LCDIF_VDCTRL2_HSYNC_PERIOD(pLCDIFDotclk->u32HsyncPeriod)));;

    HW_LCDIF_VDCTRL3_CLR(BM_LCDIF_VDCTRL3_HORIZONTAL_WAIT_CNT);

    HW_LCDIF_VDCTRL3_SET(pLCDIFDotclk->u32HsyncWaitCount  << 16); // 9

    HW_LCDIF_VDCTRL4.B.DOTCLK_H_VALID_DATA_CNT = pLCDIFDotclk->u32DotclkWaitCount;

    rc = TRUE;

    return (rc);
}
//-----------------------------------------------------------------------------
//  Function:LCDIFSetIrqEnable()
//
//  Enable, or disable, interrupts within the LCDIF block.  The interrupts
//  are bit-or'ed together.
//
//  Parameters:
//         [in] A bit-mask of all interrupts that should be enabled.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void LCDIFSetIrqEnable(UINT32 IrqMask)
{
    UINT32 u32IrqMask;
    u32IrqMask =  (IrqMask & 0x0F) << 12;

    HW_LCDIF_CTRL1_CLR(u32IrqMask);
    HW_LCDIF_CTRL1_SET(u32IrqMask);

    if(IrqMask & LCDIF_IRQ_BM_ERROR)
    {
        HW_LCDIF_CTRL1_SET(BM_LCDIF_CTRL1_BM_ERROR_IRQ_EN);
    }
}
//-----------------------------------------------------------------------------
//  Function:LCDIFActiveIrq()
//
//
//
//
//  Parameters:
//         [in]
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
UINT32 LCDIFActiveIrq(VOID)
{
    UINT32 irq_mask = HW_LCDIF_CTRL1_RD();

    irq_mask &= 0x0000FF00;
    irq_mask = ((irq_mask & 0x0000F000) >> 4) & (irq_mask & 0x00000F00);
    irq_mask >>= 8;

    return irq_mask;
}
//-----------------------------------------------------------------------------
//  Function:LCDIFActiveIrq()
//
//
//
//
//  Parameters:
//         [in]
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
VOID LCDIFClearIrq(UINT32 irq_mask)
{
    if (irq_mask != 0)
    {
        irq_mask = (irq_mask & 0x0F) << 8;
        HW_LCDIF_CTRL1_CLR(irq_mask);
    }
}

//-----------------------------------------------------------------------------
//  Function:LCDIFSetDotclkMode()
//
//
//
//
//  Parameters:
//         [in]
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void LCDIFSetDotclkMode(BOOL bEnable)
{
    HW_LCDIF_CTRL.B.DOTCLK_MODE   = bEnable;

    if (!bEnable) while (HW_LCDIF_CTRL.B.RUN == 1) ;

    HW_LCDIF_CTRL.B.BYPASS_COUNT = bEnable;

    if (bEnable)
    {
        BW_LCDIF_CTRL1_RECOVER_ON_UNDERFLOW(1);
    }
    else
    {
        BW_LCDIF_CTRL1_RECOVER_ON_UNDERFLOW(0);
    }

    //RETAILMSG(1, (TEXT("REG_HW_LCDIF_CTRL=0x%x\r\n"),    HW_LCDIF_CTRL_RD()));
    //RETAILMSG(1, (TEXT("REG_HW_LCDIF_CTRL1=0x%x\r\n"),   HW_LCDIF_CTRL1_RD()));
    //RETAILMSG(1, (TEXT("REG_HW_LCDIF_TIMING=0x%x\r\n"),  HW_LCDIF_TIMING_RD()));
    //RETAILMSG(1, (TEXT("REG_HW_LCDIF_VDCTRL0=0x%x\r\n"), HW_LCDIF_VDCTRL0_RD()));
    //RETAILMSG(1, (TEXT("REG_HW_LCDIF_VDCTRL1=0x%x\r\n"), HW_LCDIF_VDCTRL1_RD()));
    //RETAILMSG(1, (TEXT("REG_HW_LCDIF_VDCTRL2=0x%x\r\n"), HW_LCDIF_VDCTRL2_RD()));
    //RETAILMSG(1, (TEXT("REG_HW_LCDIF_VDCTRL3=0x%x\r\n"), HW_LCDIF_VDCTRL3_RD()));
    //RETAILMSG(1, (TEXT("LCDIFSetupDotclk:REG_HW_LCDIF_VDCTRL4=0x%x\r\n"), HW_LCDIF_VDCTRL4_RD()));

}
//-----------------------------------------------------------------------------
//  Function:LCDIFSetVsyncMode()
//
//  Parameters:
//         [in]
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
VOID LCDIFSetVsyncMode(BOOL bEnable)
{
    HW_LCDIF_CTRL.B.VSYNC_MODE = bEnable;

    if (!bEnable) while (HW_LCDIF_CTRL.B.RUN == 1) ;

    HW_LCDIF_CTRL.B.BYPASS_COUNT = bEnable;
}
//-----------------------------------------------------------------------------
//  Function:LCDIFSetDviMode()
//
//  This function enables/disables the DVImode of the block according to input.
//
//  Parameters:
//      [in]  bEnable - TRUE to enable, FALSE to disable
//
//  Returns:
//
//-----------------------------------------------------------------------------
VOID LCDIFSetDviMode(BOOL bEnable)
{
    HW_LCDIF_CTRL.B.DVI_MODE = bEnable ? 1:0;

    if (!bEnable)
    {
        while (HW_LCDIF_CTRL.B.RUN == 1)
        ;
    }
    HW_LCDIF_CTRL.B.BYPASS_COUNT = bEnable ? 1:0;
    BW_LCDIF_CTRL_RGB_TO_YCBCR422_CSC( bEnable ? 1:0);
    BW_LCDIF_CTRL_DVI_MODE( bEnable ? 1:0);
    BW_LCDIF_CTRL1_RECOVER_ON_UNDERFLOW(bEnable ? 1:0);
}
//-----------------------------------------------------------------------------
//  Function:LCDIFSetSyncSignals()
//
//  This function enables/disables syncsignals according to input.
//
//  Parameters:
//      param[in] bEnable
//
//  Returns:
//-----------------------------------------------------------------------------
VOID LCDIFSetSyncSignals(BOOL bEnable)
{
    HW_LCDIF_VDCTRL4.B.SYNC_SIGNALS_ON = bEnable;
}
void LCDIFSetTransferCount(UINT32 WidthInBytes, UINT32 Height)
{
    BF_CS2(LCDIF_TRANSFER_COUNT, V_COUNT, Height, H_COUNT, WidthInBytes);
}
VOID LCDIFSetBusMasterMode(BOOL bEnable)
{
    HW_LCDIF_CTRL.B.LCDIF_MASTER = bEnable;
}
BOOL LCDIFWriteDMA(const void* pData, UINT32 size)
{
    hw_lcdif_ctrl_t ctrl_reg;
    UINT32 physAddress;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(size);

    ctrl_reg.U = HW_LCDIF_CTRL_RD();
    physAddress = (UINT32) pData;

    if (ctrl_reg.B.DOTCLK_MODE || ctrl_reg.B.DVI_MODE) {
        HW_LCDIF_NEXT_BUF_WR(physAddress);

        if (ctrl_reg.B.RUN != 1)
            HW_LCDIF_CUR_BUF_WR(physAddress);
    } else {
        if (ctrl_reg.B.RUN == 1)
            while (HW_LCDIF_CTRL.B.RUN == 1) ;

        HW_LCDIF_CUR_BUF_WR(physAddress);
    } //if/else

    ctrl_reg.B.DATA_SELECT = DATA_MODE;
    ctrl_reg.B.WAIT_FOR_VSYNC_EDGE = ctrl_reg.B.VSYNC_MODE;
    ctrl_reg.B.RUN = 1;

    HW_LCDIF_CTRL_WR(ctrl_reg.U);

    return TRUE;
}

//-----------------------------------------------------------------------------
//  Function:LCDIFSetupDvi()
//
//  This function configures the DVI mode parameters of the LCDIF
//
//  Parameters:
//      param[in] p - pointer to LCDIFDVI struct
//
//  Returns:
//      TRUE
//-----------------------------------------------------------------------------
BOOL LCDIFSetupDvi(LCDIFDVI *p)
{
    //
    // reset LCD panel and hold in reset
    //
    //  HW_LCDIF_CTRL1.B.RESET  = 0;
    Sleep(1);
    //   HW_LCDIF_CTRL1.B.RESET  = 1;

    //
    // reconfigure pins for DVI mode
    //
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D00        ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D01        ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D02        ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D03        ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D04        ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D05        ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D06        ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_D07        ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_RESET      ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_RS         ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_WR         ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_CS         ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_DOTCLK     ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_ENABLE     ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_HSYNC      ,DDK_IOMUX_MODE_00);
    DDKIomuxSetPinMux(DDK_IOMUX_LCD_VSYNC      ,DDK_IOMUX_MODE_00);

    //
    // configure DVI settings from caller's struct
    //
    HW_LCDIF_DVICTRL0.B.START_TRS           = p->bStartTRS           ;
    HW_LCDIF_DVICTRL0.B.H_ACTIVE_CNT        = p->u32HactiveCount     ;
    HW_LCDIF_DVICTRL0.B.H_BLANKING_CNT      = p->u32HblankingCount   ;
    HW_LCDIF_DVICTRL0.B.V_LINES_CNT         = p->u32VlinesCount      ;
    HW_LCDIF_DVICTRL1.B.F1_START_LINE       = p->u32Field1StartLine  ;
    HW_LCDIF_DVICTRL1.B.F1_END_LINE         = p->u32Field1EndLine    ;
    HW_LCDIF_DVICTRL1.B.F2_START_LINE       = p->u32Field2StartLine  ;
    HW_LCDIF_DVICTRL2.B.F2_END_LINE         = p->u32Field2EndLine    ;
    HW_LCDIF_DVICTRL2.B.V1_BLANK_START_LINE = p->u32V1BlankStartLine ;
    HW_LCDIF_DVICTRL2.B.V1_BLANK_END_LINE   = p->u32V1BlankEndLine   ;
    HW_LCDIF_DVICTRL3.B.V2_BLANK_START_LINE = p->u32V2BlankStartLine ;
    HW_LCDIF_DVICTRL3.B.V2_BLANK_END_LINE   = p->u32V2BlankEndLine   ;
    HW_LCDIF_DVICTRL4.B.Y_FILL_VALUE        = (reg8_t)p->u32YFillValue       ;
    HW_LCDIF_DVICTRL4.B.CB_FILL_VALUE       = (reg8_t)p->u32CBFillValue      ;
    HW_LCDIF_DVICTRL4.B.CR_FILL_VALUE       = (reg8_t)p->u32CRFillValue      ;
    HW_LCDIF_DVICTRL4.B.H_FILL_CNT          = (reg8_t)p->u32HFillCount       ;

    return TRUE;
}

VOID LCDIFFlush(VOID)
{
    if (HW_LCDIF_CTRL.B.BYPASS_COUNT != 1)
    {
        while (HW_LCDIF_CTRL.B.RUN)
            ;
    }
}

VOID LCDIFSetInterlace(BOOL bEnable)
{
    HW_LCDIF_CTRL1.B.INTERLACE_FIELDS = bEnable ? 1: 0;
//    HW_LCDIF_CTRL1.B.START_INTERLACE_FROM_SECOND_FIELD = bEnable ? 1: 0;
}
