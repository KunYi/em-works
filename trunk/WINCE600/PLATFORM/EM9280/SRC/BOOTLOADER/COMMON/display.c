//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  display.c
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include "display_panel.h"			// CS&ZHL MAR-7-2012: supporting multiple panels

//------------------------------------------------------------------------------
// Local Functions
static VOID PutPixelBuf(DWORD dwX, DWORD dwY, DWORD wColor, WORD *pwBuf);
static BOOL InitQVGA(PDISPLAY_PANEL_MODE pPanel);				// CS&ZHL MAR-7-2012: supporting multiple LCD panels (VOID);
static VOID EBOOT_SetupPIXClock();
static VOID ConfigurePanel(PDISPLAY_PANEL_MODE pPanel);			// CS&ZHL MAR-7-2012: supporting multiple LCD panels (VOID);
static VOID InitBacklight();
static VOID ShowBmp(PBYTE pBmpFile);
static VOID ClrScreen(VOID);
static BOOL LCDIFSetupIOMUXPin();
VOID TurnOffDisplay();

#define  VIDEO_MEMORY_SIZE      800*480*4
#define  PALETTE_SIZE           256
PALETTEENTRY StdPalette[PALETTE_SIZE];

//------------------------------------------------------------------------------
// CS&ZHL MAR-7-2012:Description of all supported mode for all supported panel
//------------------------------------------------------------------------------
DISPLAY_PANEL_MODE PanelModeArray[] =
{
    // 320*240 -> 3.5", PCLK = 6.4MHz -> LQ035 
    {
		320, 240, 60, 16,		//Width, Height, FrameFreq, BPP
		// LCD panel specific settings
		320,					//dwHWidth          -> DOTCLK_H_ACTIVE;
		16,						//dwHSyncPulseWidth -> DOTCLK_H_PULSE_WIDTH; 
		32,						//dwHFrontPorch     -> DOTCLK_HF_PORCH;
		32,						//dwHBackPorch      -> DOTCLK_HB_PORCH; -> HTotal = 320 + 16 + 32 + 32 = 400
		240,					//dwVHeight         -> DOTCLK_V_ACTIVE;
		3,						//dwVSyncPulseWidth -> DOTCLK_V_PULSE_WIDTH;
		4,						//dwVFrontPorch     -> DOTCLK_VF_PORCH;
		15,						//dwVBackPorch      -> DOTCLK_VB_PORCH; -> VTotal = 240 + 3 + 4 + 15 = 262
    },
    // 480*272 -> 4.3", PCLK = 9MHz -> LR430LC9001
    {
		480, 272, 60, 16,		//Width, Height, FrameFreq, BPP
		// LCD panel specific settings
		480,					//dwHWidth          -> DOTCLK_H_ACTIVE;
		37,						//dwHSyncPulseWidth -> DOTCLK_H_PULSE_WIDTH; 
		4,						//dwHFrontPorch     -> DOTCLK_HF_PORCH;
		4,						//dwHBackPorch      -> DOTCLK_HB_PORCH; -> HTotal = 480 + 37 + 4 + 4 = 525
		272,					//dwVHeight         -> DOTCLK_V_ACTIVE;
		10,						//dwVSyncPulseWidth -> DOTCLK_V_PULSE_WIDTH;
		2,						//dwVFrontPorch     -> DOTCLK_VF_PORCH;
		2,						//dwVBackPorch      -> DOTCLK_VB_PORCH; -> VTotal = 272 + 10 + 2 + 2 = 286
    },
    // 640*480 -> 5.6" -> AT050TN22, 
    {
		640, 480, 60, 16,		//Width, Height, FrameFreq, BPP
		// LCD panel specific settings
		640,					//dwHWidth          -> DOTCLK_H_ACTIVE;
		10,						//dwHSyncPulseWidth -> DOTCLK_H_PULSE_WIDTH; (other config: 64, 40, 56)
		16,						//dwHFrontPorch     -> DOTCLK_HF_PORCH;
		134,					//dwHBackPorch      -> DOTCLK_HB_PORCH; -> HTotal = 640 + 10 + 16 + 134 = 800
		480,					//dwVHeight         -> DOTCLK_V_ACTIVE;
		2,						//dwVSyncPulseWidth -> DOTCLK_V_PULSE_WIDTH;
		32,						//dwVFrontPorch     -> DOTCLK_VF_PORCH;
		11,						//dwVBackPorch      -> DOTCLK_VB_PORCH; -> VTotal = 480 + 2 + 32 + 11 = 525
    },
    // 800*480 -> 7.0", PCLK = 33.3MHz -> AT070TN83 V1
    {
		800, 480, 68, 16,		//Width, Height, FrameFreq, BPP
		// LCD panel specific settings
#ifdef	EM9280_LCD
		800,					//dwHWidth          -> DOTCLK_H_ACTIVE;
		48,						//dwHSyncPulseWidth -> DOTCLK_H_PULSE_WIDTH;
		40,						//dwHFrontPorch     -> DOTCLK_HF_PORCH;
		40,						//dwHBackPorch      -> DOTCLK_HB_PORCH; -> HTotal = 800 + 48 + 40 + 40 = 928
		480,					//dwVHeight         -> DOTCLK_V_ACTIVE;
		3,						//dwVSyncPulseWidth -> DOTCLK_V_PULSE_WIDTH;
		13,						//dwVFrontPorch     -> DOTCLK_VF_PORCH;
		29,						//dwVBackPorch      -> DOTCLK_VB_PORCH; -> VTotal = 480 + 3 + 13 + 29 = 525
#else	// -> iMX28EVK->Seiko 4.3" 43WVF1G-0
		800,					//dwHWidth          -> DOTCLK_H_ACTIVE;
		10,						//dwHSyncPulseWidth -> DOTCLK_H_PULSE_WIDTH;
		164,					//dwHFrontPorch     -> DOTCLK_HF_PORCH;
		89,						//dwHBackPorch      -> DOTCLK_HB_PORCH;
		480,					//dwVHeight         -> DOTCLK_V_ACTIVE;
		10,						//dwVSyncPulseWidth -> DOTCLK_V_PULSE_WIDTH;
		10,						//dwVFrontPorch     -> DOTCLK_VF_PORCH;
		23,						//dwVBackPorch      -> DOTCLK_VB_PORCH;
#endif	//EM9280
    }
};


//-----------------------------------------------------------------------------
// Global Variables
static PVOID	pv_HWregLCDIF;
static WORD*	g_pLCDBuffer = NULL;
static PVOID	pv_HWRegPWM = NULL;

//------------------------------------------------------------------------------------
// CS&ZHL JAN-12-2012: add display format = 0: default format -> 800x480 (4.3" LCD)
//										  = 1: 320x240 (3.5" LCD) 
//										  = 2: 480x272 (4.3" LCD)
//										  = 3: 640x480 (5.6", 5.7", 6.4", 10.4" LCD)
//										  = 4: 800x480 (7" LCD)
//										  = 5: 800x600 (8.4", 10.4")
//------------------------------------------------------------------------------------
static DWORD	g_dwDispFormat = 0;

//-----------------------------------------------------------------------------
// CS&ZHL JAN-12-2012: external routines for splash screen
extern BOOL NANDReadImage(PBYTE pBuf, DWORD dwStartLogBlock, DWORD dwLen);

//
// return > 0: file size in byte
//        = 0: bmp file not found!
// 
DWORD FindBMP();

//
// return = 0: no bmp 
// return = 1: 320x240 (3.5" LCD) 
// return = 2: 480x272 (4.3" LCD)
// return = 3: 640x480 (5.6" LCD)
// return = 4: 800x480 (7.0" LCD, default setting)
//
DWORD GetBMP(PBYTE pBmpFile, DWORD dwLen);

//-----------------------------------------------------------------------------
// Defines

#define ACTIVATE_BACKLIGHT

// 4.3 serial panel specific settings
#define DOTCLK_H_ACTIVE         800
#define DOTCLK_H_PULSE_WIDTH    10
#define DOTCLK_HF_PORCH         164
#define DOTCLK_HB_PORCH         89
#define DOTCLK_H_WAIT_CNT		DOTCLK_HB_PORCH
#define DOTCLK_H_PERIOD			(DOTCLK_HB_PORCH + DOTCLK_HF_PORCH + DOTCLK_H_ACTIVE )

#define DOTCLK_V_ACTIVE         480
#define DOTCLK_V_PULSE_WIDTH    10
#define DOTCLK_VF_PORCH         10
#define DOTCLK_VB_PORCH         23
#define DOTCLK_V_WAIT_CNT		DOTCLK_VB_PORCH
#define DOTCLK_V_PERIOD			(DOTCLK_VF_PORCH + DOTCLK_VB_PORCH + DOTCLK_V_ACTIVE)
 

// RGB colour setting
#define COLOR_R 16
#define COLOR_G 8
#define COLOR_B 0

//------------------------------------------------------------------------------
//
//  Function:  PutPixelBuf
//
//  Parameters:
//      DWORD dwX.    X position
//      DWORD dwY.    Y position
//      DWORD wcolor  Pixel data
//      DWORD pwBuf   Pointer to the display RAM bufer
//  Returns:
//      None.
//------------------------------------------------------------------------------
static void PutPixelBuf(DWORD dwX, DWORD dwY, DWORD dwColor, WORD *pwBuf)
{
    WORD *pwTmp;
    WORD wColor = 0;
    if ((dwX < 800) && (dwY < 480) )
    {
        pwTmp = (WORD*)pwBuf + dwY * 800 + dwX;
        wColor = (WORD)(((dwColor & 0xF80000) >> 8) | ((dwColor & 0xFC00) >> 5) | ((dwColor & 0xF8) >> 3));
        *pwTmp = wColor;
    }
    return; 
}
//------------------------------------------------------------------------------
//
//  Function:  InitQVGA
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE or FALSE
//------------------------------------------------------------------------------
//
// LCD hardware initialization routine.
// In this routine we will initialize first display (video) memory buffer
// After that we will intialize various hardware required for LCD display
// Like clock, GPIO pins, DMA, SPI to communicate with LCD board
// Clear the screen and start the DMA channel
//
// Function    -> InitQVGA
// Param    -> None
// Return    -> None
// Note        -> None
//static BOOL InitQVGA(VOID)
static BOOL InitQVGA(PDISPLAY_PANEL_MODE pPanel)				// CS&ZHL MAR-7-2012: supporting multiple LCD panels
{
    //KITLOutputDebugString ( "EBOOT: InitQVGA++ \n");

    /* Initalize the globle buffer this settings are defined in XXX.h file */
    g_pLCDBuffer  = (WORD*)OALPAtoUA(IMAGE_WINCE_DISPLAY_RAM_PA_START);;

    // Start the PIX clock and set frequency
    EBOOT_SetupPIXClock();

    /* LCD PIN setup for Serial Panel*/
    LCDIFSetupIOMUXPin();

    /* LCD PIN setup for Serial Panel*/
    //ConfigurePanel();
    ConfigurePanel(pPanel);				// CS&ZHL MAR-7-2012: supporting multiple LCD panels

    /* Setup 3.5 Serial Panel registers*/
	if(!g_dwDispFormat)		// for default format only
	{
		memset((PBYTE)g_pLCDBuffer, 0xff, 0xBB800); 
	}

    //DMA
    LCDIFDisplayFrameBuffer((const void*)IMAGE_WINCE_DISPLAY_RAM_PA_START);

    //ClrScreen();

    //KITLOutputDebugString ( "***** EBOOT: InitQVGA-- \n");

    return TRUE;
}
//------------------------------------------------------------------------------
//
//  Function:  EBOOT_SetupPIXClock
//  Initialize PIX clock for 4.3 inch serial panel Display. We are setting the
//  PIX frequency for 8MHZ. Caluclations are commented below in code.
//
//  Parameters:
//      None.
//
//  Returns:
//      NONE
//------------------------------------------------------------------------------
static void EBOOT_SetupPIXClock()
{
    PVOID pv_HWregCLKCTRL = NULL;
    if (pv_HWregCLKCTRL == NULL)
    {
        // Map peripheral physical address to virtual address
        pv_HWregCLKCTRL = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_CLKCTRL);

        // Check if virtual mapping failed
        if (pv_HWregCLKCTRL == NULL)
        {
            KITLOutputDebugString ( "EBOOT: EBOOT_SetupPIXClock pv_HWregCLKCTRL NULL \n");
            return;
        }
    }

    // Turn On CLKGATE
    HW_CLKCTRL_DIS_LCDIF_CLR(BM_CLKCTRL_DIS_LCDIF_CLKGATE);

    // Set Dot Clock to 8MHz,this clock must slower than HBUS,or the display will become abnormal.
    HW_CLKCTRL_DIS_LCDIF_CLR(BM_CLKCTRL_DIS_LCDIF_DIV);
       
    HW_CLKCTRL_DIS_LCDIF_SET(0x1);

    //HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF);

    // Wait till we are OK
    while((HW_CLKCTRL_DIS_LCDIF_RD() & BM_CLKCTRL_DIS_LCDIF_BUSY) != 0) ;

    // Start PWM clock
    HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_PWM_CLK24M_GATE);

    //KITLOutputDebugString ( "EBOOT: EBOOT_SetupPIXClock-- \n");
}
//------------------------------------------------------------------------------
//
//  Function:  ConfigurePanel
//             Reset the LCDIF and setup the LCDIF registers for 3.5 inch serial panel
//  display
//
//  Parameters:
//      None.
//
//  Returns:
//      NONE
//------------------------------------------------------------------------------
//static void ConfigurePanel(VOID)
static void ConfigurePanel(PDISPLAY_PANEL_MODE pPanel)
{
    //KITLOutputDebugString ( "EBOOT: ConfigureHX8238ASerialPanel++ \n");
    LCDIF_INIT LcdifInit;
    LCDIFVSYNC LCDIfVsync;
    LCDIFDOTCLK sLcdifDotclk;

    LcdifInit.bBusyEnable   = FALSE;
    LcdifInit.eBusMode      = BUSMODE_8080;
    LcdifInit.eReset        = LCDRESET_HIGH;
    LcdifInit.eDataSwizzle  = NO_SWAP;    
    LcdifInit.eCscSwizzle   = NO_SWAP;
    LcdifInit.eWordLength   = WORDLENGTH_16BITS;
    LcdifInit.eBusWidth     = LCDIF_BUS_WIDTH_24BIT;

    LcdifInit.Timing.BYTE.u8DataSetup = 1;
    LcdifInit.Timing.BYTE.u8DataHold  = 1;
    LcdifInit.Timing.BYTE.u8CmdSetup  = 1;
    LcdifInit.Timing.BYTE.u8CmdHold   = 1;

    LCDIFInit(&LcdifInit,TRUE,FALSE);

    LCDIFSetBusMasterMode(TRUE);
    LCDIFSetDataShift(DATA_SHIFT_RIGHT, 0);
    LCDIFSetBytePacking(0xF);

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
    sLcdifDotclk.bEnablePresent     = TRUE; 
    sLcdifDotclk.eHSyncPolarity     = POLARITY_LOW;
    sLcdifDotclk.eEnablePolarity    = POLARITY_HIGH;
    sLcdifDotclk.eDotClkPolarity    = POLARITY_HIGH;
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
		LCDIFSetTransferCount(800,480); 
	}
    LCDIFSetSyncSignals(TRUE);
    LCDIFSetDotclkMode(TRUE);
}

//------------------------------------------------------------------------------
//
//  Function:  InitBacklight
//
//  Parameters:
//      None.
//  Returns:
//      None.
//------------------------------------------------------------------------------
static void InitBacklight()
{
    if (pv_HWRegPWM == NULL)
    {
        // Map peripheral physical address to virtual address
        pv_HWRegPWM = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_PWM);

        // Check if virtual mapping failed
        if (pv_HWRegPWM == NULL)
        {
            KITLOutputDebugString ( "EBOOT: pv_HWRegPWM NULL \n");
            return;
        }
    }

    BF_CLR(PWM_CTRL, SFTRST);

    //Make sure that we are completely out of reset before continuing.
    while (HW_PWM_CTRL.B.SFTRST) ;

    BF_CLR(PWM_CTRL,CLKGATE);

    //Make sure that we are completely out of reset before continuing.
    while (HW_PWM_CTRL.B.CLKGATE) ;

    DDKIomuxSetPinMux(DDK_IOMUX_PWM2,DDK_IOMUX_MODE_00);
    DDKClockSetGatingMode(DDK_CLOCK_GATE_PWM24M_CLK, FALSE);

    BF_CS2n(PWM_ACTIVEn, 2,
            INACTIVE, 0x3e80,
            ACTIVE, 0);

    BF_CS5n(PWM_PERIODn, 2,
            MATT, 0,
            CDIV, 0x2,
            INACTIVE_STATE, 0x2,
            ACTIVE_STATE, 0x3,
            PERIOD, 0x9c3f);

    // Enable PWM channel
    HW_PWM_CTRL_SET(1 << 2);
}
//------------------------------------------------------------------------------
//
//  Function:  DisplayInit
//
//  Parameters:
//      None.
//  Returns:
//      None.
//------------------------------------------------------------------------------
void DisplayInit(VOID)
{
    PBYTE				pBmpFile = NULL;
    PVOID				pv_HWregPOWER = NULL;
	PDISPLAY_PANEL_MODE pPanel = NULL;				// CS&ZHL MAR-7-2012: supporting multiple LCD panels

    pv_HWregPOWER =(PVOID) OALPAtoUA(CSP_BASE_REG_PA_POWER);
    
    if(pv_HWregPOWER == NULL)
    {
        KITLOutputDebugString ( "INFO:eboot display, pv_HWregPOWER = NULL\r\n");
        return;
    }
#ifdef BSP_5V_FROM_VBUS     
    if((HW_POWER_5VCTRL.B.PWD_CHARGE_4P2 == 0) && ((HW_POWER_5VCTRL_RD() & BM_POWER_5VCTRL_CHARGE_4P2_ILIMIT) == 0x20000))
    {    
        KITLOutputDebugString ( "There is 100mA limitation from xldr,eboot splash screen will be disabled. \r\n");
        return;
    } 
#endif
 
	//
	// CS&ZHL JAN-12-2012: try to find bmp in specified address of NandFlash
	//
#ifdef	EM9280
	{
		DWORD	dwLen;
		DWORD	dwFormatIndex;

		dwLen = FindBMP();		// try to find BMP file
		if(dwLen > 0)
		{
			// BMP found, dump it to temp buffer
			pBmpFile = (PBYTE)OALPAtoUA(IMAGE_BOOT_NKIMAGE_RAM_PA_START);
			//if(!GetBMP(pBmpFile, dwLen))
			//
			// CS&ZHL MAR-7-2012: get panel format index -> 
			//					  return = 0: no bmp 
			//					  return = 1: 320x240 
			//					  return = 2: 480x272 (4.3" LCD)
			//				      return = 3: 640x480 (default)
			//					  return = 4: 800x480 (7" LCD)
			//
			dwFormatIndex = GetBMP(pBmpFile, dwLen);
			if(!dwFormatIndex)
			{
				KITLOutputDebugString( "EBOOT: the BMP format is not supported!\r\n");
				pBmpFile = NULL;
			}
			else
			{
				pPanel = &PanelModeArray[dwFormatIndex - 1];
			}
		}
	}
#endif	//EM9280

	if(!g_dwDispFormat)
	{
		// default format
		KITLOutputDebugString( "EBOOT: DisplayInit++ :BSP_DISPLAY_43WVF1G-0\r\n");
	}

    //InitQVGA();
	// CS&ZHL supporting multiple LCD panels
    InitQVGA(pPanel);
    if(!pBmpFile)
	{
		pBmpFile = (PBYTE)OALPAtoUA(IMAGE_BOOT_RSVD_RAM_PA_START);
	}
    ShowBmp(pBmpFile);
    OALStall(200000);
#ifdef ACTIVATE_BACKLIGHT
    InitBacklight();
#endif

    //KITLOutputDebugString ( "EBOOT: DisplayInit-- \n");
}
//------------------------------------------------------------------------------
//
//  Function:  ShowBmp
//
//  Parameters:
//      pBmpFile : BMP image pointer
//  Returns:
//      None.
//------------------------------------------------------------------------------
static void ShowBmp(PBYTE pBmpFile)
{
    PBITMAPFILEHEADER	pBmFH;
    PBITMAPINFOHEADER	pBmIH;
    RGBQUAD				*pPalette;
    PBYTE				pBitmap;
    DWORD				x, y;
    DWORD				index;
    COLORREF			dwPoint;
    DWORD				RedLsh, BlueLsh;

    RETAILMSG(1, (TEXT("pBmpFile 0x%x \r\n"), pBmpFile));

    // Read the BMP header
    pBmFH = (PBITMAPFILEHEADER) pBmpFile;
    pBmIH = (PBITMAPINFOHEADER) (pBmFH + 1);
    pBitmap = pBmpFile + pBmFH->bfOffBits;

    //RETAILMSG(1, (TEXT("pBmIH->biWidth=0x%x\r\n"),pBmIH->biWidth));
    //RETAILMSG(1, (TEXT("pBmIH->biHeight=0x%x\r\n"),pBmIH->biHeight));
    //RETAILMSG(1, (TEXT("pBmIH->biCompression=0x%x\r\n"),pBmIH->biCompression));
    //RETAILMSG(1, (TEXT("pBmIH->biBitCount=0x%x\r\n"),pBmIH->biBitCount));

    // If BMP signature not found generate error and return
    if (pBmFH->bfType != 0x4D42) {
        EdbgOutputDebugString ("ERROR: Not a bmp file. \r\n");
        return;
    }

    // get bmp palette data if needed
    if ( pBmIH->biBitCount  == 8 || pBmIH->biBitCount  == 4 )
    {
        pPalette = (RGBQUAD *)((LPSTR)pBmIH + (WORD)(pBmIH->biSize));

        /* read the palette information */
        for(index=0; index < 256; index++)
        {
            StdPalette[index].peRed   = pPalette[index].rgbRed;
            StdPalette[index].peGreen = pPalette[index].rgbGreen;
            StdPalette[index].peBlue  = pPalette[index].rgbBlue;
            StdPalette[index].peFlags = pPalette[index].rgbReserved;
        }
    }
    else
    {
        RETAILMSG(1, (TEXT("ERROR pBmIH->biBitCount=0x%x Expected: 0x0008\r\n"),
                      pBmIH->biBitCount));
        return;
    }

    RedLsh = COLOR_R;
    BlueLsh = COLOR_B;

    // Loop here for Height x Width to put each pixel on display
    for (y = (DWORD)(pBmIH->biHeight) + ((DOTCLK_V_ACTIVE - pBmIH->biHeight)/2); 
        ((int)(y)) > ((DOTCLK_V_ACTIVE - pBmIH->biHeight)/2); y--) 
    {
        for(x = ((DOTCLK_H_ACTIVE - pBmIH->biWidth)/2); 
            x < (DWORD)(pBmIH->biWidth)+((DOTCLK_H_ACTIVE - pBmIH->biWidth)/2); x++) 
        {
            // Clear color value
            dwPoint = 0;
            // Read 8 BPP BMP image color value.
            // Find out RGB value for that color from
            // 256 color standard pallete

            dwPoint = (StdPalette[*pBitmap].peRed    << RedLsh )  |
                      (StdPalette[*pBitmap].peGreen  << COLOR_G)  |
                      (StdPalette[*pBitmap].peBlue   << BlueLsh);

            // Point to the next pixel color
            pBitmap++;

            // Put color value on each pixel
            PutPixelBuf(x, y, dwPoint, g_pLCDBuffer);
        }
    }
}
//------------------------------------------------------------------------------
//
//  Function:  ClrScreen
//
//  Parameters:
//      None.
//  Returns:
//      None.
//------------------------------------------------------------------------------
static VOID ClrScreen(VOID)
{
    //KITLOutputDebugString ("EBOOT: ClrScreen++ \n");
    int index;
    for (index = 0; index < 800 * 480; index++) 
    {
        *(g_pLCDBuffer+index) = 0;
    }
    //KITLOutputDebugString ("EBOOT: ClrScreen-- \n");
}

//-----------------------------------------------------------------------------
//
//  Function: LCDIFSetupIOMUXPin()
//
//      Set IOMux for display panel
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL LCDIFSetupIOMUXPin()
{
    // Setup the PINMUX
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

    return TRUE;
}
//------------------------------------------------------------------------------
//
//  Function:  TurnOffDisplay
//
//  Parameters:
//      None.
//  Returns:
//      None.
//------------------------------------------------------------------------------
VOID TurnOffDisplay()
{
    if (pv_HWRegPWM == NULL)
    {
        // Map peripheral physical address to virtual address
        pv_HWRegPWM = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_PWM);

        // Check if virtual mapping failed
        if (pv_HWRegPWM == NULL)
        {
            KITLOutputDebugString ( "EBOOT: pv_HWRegPWM NULL \n");
            return;
        }
    }

    BF_CLR(PWM_CTRL, SFTRST);

    //Make sure that we are completely out of reset before continuing.
    while (HW_PWM_CTRL.B.SFTRST);

    BF_CLR(PWM_CTRL,CLKGATE);

    //Make sure that we are completely out of reset before continuing.
    while (HW_PWM_CTRL.B.CLKGATE) ;

    DDKIomuxSetPinMux(DDK_IOMUX_PWM2,DDK_IOMUX_MODE_00);
    DDKClockSetGatingMode(DDK_CLOCK_GATE_PWM24M_CLK, FALSE);

    // Enable PWM channel
    HW_PWM_CTRL_CLR(1 << 2);
    if (g_pLCDBuffer != NULL)
      ClrScreen();
}


//--------------------------------------------------------
// CS&ZHL JAN-12-2012: routines for splash screen
//--------------------------------------------------------

//
// return > 0: file size in byte
//        = 0: bmp file not found!
// 
DWORD FindBMP()
{
	DWORD				dwNandLogBlock;		// nandflash logical block where customer's bmp file is stored in
	PBYTE				pBMPBuffer;			// temp buffer for bmp file from nandflash
	BITMAPFILEHEADER*	pBmpFileHead;
	BITMAPINFOHEADER*	pBmpInfoHead;

	RETAILMSG(1, (TEXT("->FindBMP\r\n")));
	dwNandLogBlock = IMAGE_BOOT_SPLASH_NAND_OFFSET;		
	pBMPBuffer = OALPAtoVA((DWORD)IMAGE_BOOT_NKIMAGE_RAM_PA_START, FALSE);		// -> (DDR2_start_address + 2MB)
	RETAILMSG(1, (TEXT("FindBMP::NandLogBlock=0x%x, BMPBuffer=0x%x\r\n"), dwNandLogBlock, pBMPBuffer));

	//read the 1st 2KB data for check
	NANDReadImage(pBMPBuffer, dwNandLogBlock, 2048);
	pBmpFileHead = (BITMAPFILEHEADER*)&pBMPBuffer[0];

	if(pBmpFileHead->bfType != 0x4D42)		// => "MB"
	{
		RETAILMSG(1, (TEXT("FindBMP::no BMP file 0x%x\r\n"), pBmpFileHead->bfType));
		return 0;
	}

	pBmpInfoHead = (BITMAPINFOHEADER*)&pBMPBuffer[sizeof(BITMAPFILEHEADER)];
	if ((pBmpInfoHead->biWidth > 800) ||
		(pBmpInfoHead->biHeight > 600) ||
		(pBmpInfoHead->biBitCount != 8))
	{
		RETAILMSG(1, (TEXT("->FindBMP::only 8bpp, Width < 800, Height < 600\r\n")));
		return 0;
	}

	RETAILMSG(1, (TEXT("->FindBMP::%dx%d, bfOffBits=%d, bfSize=%d\r\n"),
		pBmpInfoHead->biWidth, pBmpInfoHead->biHeight, 
		pBmpFileHead->bfOffBits, pBmpFileHead->bfSize));

	return pBmpFileHead->bfSize;
}

//
// return = 0: no bmp 
// return = 1: 320x240 
// return = 2: 480x272 (4.3" LCD)
// return = 3: 640x480 (default)
// return = 4: 800x480 (7" LCD)
//
DWORD GetBMP(PBYTE pBmpFile, DWORD dwLen)
{
	DWORD				dwNandLogBlock;		// nandflash logical block where customer's bmp file is stored in
	BITMAPFILEHEADER*	pBmpFileHead;
	BITMAPINFOHEADER*	pBmpInfoHead;

	dwNandLogBlock = IMAGE_BOOT_SPLASH_NAND_OFFSET;		

	//read all file
	NANDReadImage(pBmpFile, dwNandLogBlock, dwLen);
	pBmpFileHead = (BITMAPFILEHEADER*)&pBmpFile[0];
	pBmpInfoHead = (BITMAPINFOHEADER*)&pBmpFile[sizeof(BITMAPFILEHEADER)];

	//get display format
	if((pBmpInfoHead->biWidth == 320) && (pBmpInfoHead->biHeight == 240))
	{
		g_dwDispFormat = 1;
	}
	else if((pBmpInfoHead->biWidth == 480) && (pBmpInfoHead->biHeight == 272))
	{
		g_dwDispFormat = 2;
	}
	else if((pBmpInfoHead->biWidth == 640) && (pBmpInfoHead->biHeight == 480))
	{
		g_dwDispFormat = 3;
	}
	else if((pBmpInfoHead->biWidth == 800) && (pBmpInfoHead->biHeight == 480))
	{
		g_dwDispFormat = 4;
	}
	else
	{
		g_dwDispFormat = 0;
	}

	/*
	if(dwDispFormat)
	{
		DWORD	i1, i2, i3;

		// get color table length
		i1 = pBmpFileHead->bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER);
		if(i1 > 1024)
		{
			i1 = 1024;
		}

		//copy color table
		memcpy(pScrDef->pLUT, pNKAddress + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), i1);

		//copy bmp data
		for(i1 = 0; i1 < (DWORD)pBmpInfoHead->biHeight; i1++)
		{
			i2 = (pBmpInfoHead->biHeight - i1 - 1) * pBmpInfoHead->biWidth;
			i3 = i1 * pBmpInfoHead->biWidth + pBmpFileHead->bfOffBits;
			memcpy(&pScrDef->pScreenBuffer[i2], &pNKAddress[i3], pBmpInfoHead->biWidth);
		}
	}
	*/

	return g_dwDispFormat;
}


