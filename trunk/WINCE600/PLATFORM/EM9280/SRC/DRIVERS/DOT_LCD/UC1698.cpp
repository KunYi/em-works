//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  UC1698.cpp
//
//  Implementation of class DisplayControllerUC1698 which for UC1698 160*160
//  WQVGA panel.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)
#include "csp.h"
#include "uc1698.h"

#define MSG_DUMP_REG			0

#define DOTCLK_H_ACTIVE         162
#define DOTCLK_V_ACTIVE         160

#define  FRAMEBUFFERSZIE        0x10000
#define  LCD_CMD_DATA_OFFSET	FRAMEBUFFERSZIE - 0x400

#define PIX_CLK					10000

const	PRODUCTCODE	 =	0x80;
 
const	CTRBYTECOUNT =	20+4;

const unsigned char CTRBYTE[CTRBYTECOUNT]={
	0xe2, 0xeb, 0x81, 60, 0xaf, 0x70, 0xc4, 0xd5, 0x84, 0xf4, 0x00, 0xf5,
	0x00, 0xf6, 0x35, 0xf7, 0xa0, 0xf8, 0xd1, 0xd5,
	0x00, 0x10, 0x60, 0x70,
};


extern DWORD BSPLoadPixelDepthFromRegistry();

DisplayControllerUC1698* DisplayControllerUC1698::SingletonController = NULL;


//------------------------------------------------------------------------------
//
// Function: ~DisplayControllerUC1698
//
// Destructor of DisplayControllerUC1698 class.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
DisplayControllerUC1698::~DisplayControllerUC1698()
{
} //~DisplayControllerUC1698

//------------------------------------------------------------------------------
//
// Function: GetInstance
//
// This function returns pointer of DisplayControllerUC1698.
//
// Parameters:
//      None.
//
// Returns:
//      Pointer of DisplayControllerUC1698.
//
//------------------------------------------------------------------------------
DisplayControllerUC1698* DisplayControllerUC1698::GetInstance()
{
    if (SingletonController == NULL)
        SingletonController = new DisplayControllerUC1698();

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
void DisplayControllerUC1698::InitBacklight(UINT32 u32Frequency,
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
BOOL DisplayControllerUC1698::DDKIomuxSetupLCDIFPins(BOOL bPoweroff)
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

        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D2,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D3,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D4,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D5,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D6,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D7,DDK_IOMUX_MODE_00);

        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D10,DDK_IOMUX_MODE_00);
        DDKIomuxSetPinMux(DDK_IOMUX_LCD_D11,DDK_IOMUX_MODE_00);
//         DDKIomuxSetPinMux(DDK_IOMUX_LCD_D12,DDK_IOMUX_MODE_00);
//         DDKIomuxSetPinMux(DDK_IOMUX_LCD_D13,DDK_IOMUX_MODE_00);
//         DDKIomuxSetPinMux(DDK_IOMUX_LCD_D14,DDK_IOMUX_MODE_00);
//         DDKIomuxSetPinMux(DDK_IOMUX_LCD_D15,DDK_IOMUX_MODE_00);
// 
//         DDKIomuxSetPinMux(DDK_IOMUX_LCD_D18,DDK_IOMUX_MODE_00);
//         DDKIomuxSetPinMux(DDK_IOMUX_LCD_D19,DDK_IOMUX_MODE_00);
// 		DDKIomuxSetPinMux(DDK_IOMUX_LCD_D18,DDK_IOMUX_MODE_00);
// 		DDKIomuxSetPinMux(DDK_IOMUX_LCD_D19,DDK_IOMUX_MODE_00);

        // setup the pin for LCDIF block
		DDKIomuxSetPinMux(DDK_IOMUX_LCD_CS,   DDK_IOMUX_MODE_00);
		DDKIomuxSetPinMux(DDK_IOMUX_LCD_RS,   DDK_IOMUX_MODE_00);
		DDKIomuxSetPinMux(DDK_IOMUX_LCD_WR_RWN,   DDK_IOMUX_MODE_00);
		DDKIomuxSetPinMux(DDK_IOMUX_LCD_RD_E,   DDK_IOMUX_MODE_00);

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

        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D10, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D11, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
//         DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D12, 
//                  DDK_IOMUX_PAD_DRIVE_8MA, 
//                  DDK_IOMUX_PAD_PULL_ENABLE,
//                  DDK_IOMUX_PAD_VOLTAGE_3V3);
//         DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D13, 
//                  DDK_IOMUX_PAD_DRIVE_8MA, 
//                  DDK_IOMUX_PAD_PULL_ENABLE,
//                  DDK_IOMUX_PAD_VOLTAGE_3V3);
//         DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D14, 
//                  DDK_IOMUX_PAD_DRIVE_8MA, 
//                  DDK_IOMUX_PAD_PULL_ENABLE,
//                  DDK_IOMUX_PAD_VOLTAGE_3V3);
//         DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D15, 
//                  DDK_IOMUX_PAD_DRIVE_8MA, 
//                  DDK_IOMUX_PAD_PULL_ENABLE,
//                  DDK_IOMUX_PAD_VOLTAGE_3V3);
// 
// 		DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D18, 
//                  DDK_IOMUX_PAD_DRIVE_8MA, 
//                  DDK_IOMUX_PAD_PULL_ENABLE,
//                  DDK_IOMUX_PAD_VOLTAGE_3V3);
//         DDKIomuxSetPadConfig(DDK_IOMUX_LCD_D19, 
//                  DDK_IOMUX_PAD_DRIVE_8MA, 
//                  DDK_IOMUX_PAD_PULL_ENABLE,
//                  DDK_IOMUX_PAD_VOLTAGE_3V3);
   
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_CS, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_RS, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_WR_RWN, 
                 DDK_IOMUX_PAD_DRIVE_8MA, 
                 DDK_IOMUX_PAD_PULL_ENABLE,
                 DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSetPadConfig(DDK_IOMUX_LCD_RD_E, 
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
void DisplayControllerUC1698::BSPInitLCDIF(BOOL bReset)
{

    LCDIF_INIT LcdifInit;
//    LCDIFVSYNC LCDIfVsync;
//   LCDIFDOTCLK sLcdifDotclk;

    // Start the PIX clock and set frequency
    LCDIFSetupLCDIFClock(PIX_CLK);
       
    LcdifInit.bBusyEnable = FALSE;
    LcdifInit.eBusMode = BUSMODE_8080;
    LcdifInit.eReset = LCDRESET_LOW;
    LcdifInit.eDataSwizzle = NO_SWAP;
    LcdifInit.eCscSwizzle = NO_SWAP;

	LcdifInit.eWordLength = WORDLENGTH_16BITS;           
    LcdifInit.eBusWidth = LCDIF_BUS_WIDTH_16BIT;

    LcdifInit.Timing.BYTE.u8DataSetup = 3;
    LcdifInit.Timing.BYTE.u8DataHold  = 12;
    LcdifInit.Timing.BYTE.u8CmdSetup  = 10;
    LcdifInit.Timing.BYTE.u8CmdHold   = 10; 

    DDKIomuxSetupLCDIFPins(FALSE);    
    LCDIFInit(&LcdifInit, bReset,FALSE);
    
    LCDIFSetBusMasterMode(TRUE);
    //LCDIFSetIrqEnable(LCDIF_IRQ_FRAME_DONE);    
        
    LCDIFSetDataShift(DATA_SHIFT_RIGHT, 0);
	//only use 2 low bytes
	LCDIFSetBytePacking(0x3);    

    /*LCDIfVsync.bOEB = FALSE;
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
    
    LCDIFSetupDotclk(&sLcdifDotclk);*/
    LCDIFSetDviMode(FALSE);
	LCDIFSetDotclkMode(FALSE);
	LCDIFSetVsyncMode(FALSE);

    //LCDIFSetTransferCount(DOTCLK_H_ACTIVE, DOTCLK_V_ACTIVE);
    
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
void DisplayControllerUC1698::BSPResetController()
{
    LCDIFResetController(LCDRESET_LOW);
    Sleep(100); //More than 5 frames time interval between PON and Interface signal is required by UC1698 panel.
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
void DisplayControllerUC1698::DispDrvrPowerHandler(BOOL bOn, BOOL bInit, BOOL bReset,BOOL bPowerOff)
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
// Function: DisplayControllerUC1698
//
// Constructor of DisplayControllerUC1698 class.
//
// Parameters:
//      None   
// 
// Returns:
//
//------------------------------------------------------------------------------
DisplayControllerUC1698::DisplayControllerUC1698()
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
DWORD DisplayControllerUC1698::GetWidth()
{
    return  (DOTCLK_H_ACTIVE-2);
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
DWORD DisplayControllerUC1698::GetHeight()
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
void DisplayControllerUC1698::BacklightEnable(BOOL Enable)
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
void DisplayControllerUC1698::InitDisplay()
{
    //RETAILMSG(1, (L"Initializing LCDIF\r\n"));

    //m_Bpp = BSPLoadPixelDepthFromRegistry();

    BSPInitLCDIF(TRUE);
}


void DisplayControllerUC1698::InitLCD(  )
{
	RETAILMSG(1, (L"Initializing UC1698 controller\r\n"));
	int i;

	UINT32 CtrByte[ CTRBYTECOUNT ];

	for( i=0; i<CTRBYTECOUNT; i++ )
	{
		CtrByte[i] = CTRBYTE[i];
		CtrByte[i] <<= 10;
		CtrByte[i] |= CTRBYTE[i];
	}

	memcpy( (unsigned char *)m_pVirtBase+LCD_CMD_DATA_OFFSET,  (const BYTE*)CtrByte, sizeof(CtrByte));

	LCDIFSetTransferCount(1, 1);
	LCDIFDisplayFrameBufferEx( (const void *)(m_PhysBase+LCD_CMD_DATA_OFFSET), CMD_MODE );
	//waits for LCDIF transmit current frame
	LCDIFFlush();
	Sleep( 5 );

	// Then beginning of initialization 
	LCDIFSetTransferCount(CTRBYTECOUNT-1, 1);
	LCDIFDisplayFrameBufferEx( (const void *)(m_PhysBase+LCD_CMD_DATA_OFFSET+4 ), CMD_MODE );
	//waits for LCDIF transmit current frame
	LCDIFFlush();
	
	memset( m_pVirtBase, 0x00, FRAMEBUFFERSZIE-0x400 );

	// UC1698 controller is 3 bytes corresponding 6 pixel
	LCDIFSetTransferCount(81, DOTCLK_V_ACTIVE );

	LCDIFSetIrqEnable(LCDIF_IRQ_FRAME_DONE);    
	LCDIFDisplayFrameBufferEx( (const void *)(m_PhysBase), DATA_MODE );
}

void DisplayControllerUC1698::SetDisplayBuffer( ULONG PhysBase, PVOID VirtBase )
{
	m_PhysBase = PhysBase;
	m_pVirtBase = VirtBase;
}
DWORD DisplayControllerUC1698::GetVideoMemorySize()
{
	return FRAMEBUFFERSZIE;
}
void DisplayControllerUC1698::Update( PVOID pSurface )
{

	int FrameBufferIdx,SurfaceBufIdx, SurfaceX, SurfaceY, MaxSurfaceX;
	BYTE	*pSurfaceBuffer;
	UINT32	*pFrameBuffer;
	int var;

	pSurfaceBuffer = (BYTE *)pSurface;
	pFrameBuffer = (UINT32 *)m_pVirtBase;

	FrameBufferIdx = 0;
	SurfaceBufIdx = 0;
	
	MaxSurfaceX = 80;

	for( SurfaceY=0; SurfaceY<DOTCLK_V_ACTIVE;SurfaceY++ )
	{
		for( SurfaceX=0; SurfaceX<MaxSurfaceX;SurfaceX++  )
		{
			var = ~pSurfaceBuffer[SurfaceBufIdx++];
			pFrameBuffer[FrameBufferIdx] = var;
			//var = var&0x0c;
			pFrameBuffer[FrameBufferIdx] |= var <<10;
			FrameBufferIdx++;
		}
		pFrameBuffer[FrameBufferIdx] = pFrameBuffer[FrameBufferIdx-1];	
		FrameBufferIdx++;
	}

	LCDIFSetIrqEnable( FALSE );
	LCDIFSetTransferCount(4, 1);
	LCDIFDisplayFrameBufferEx( (const void *)(m_PhysBase+LCD_CMD_DATA_OFFSET+((CTRBYTECOUNT-4)<<2) ), CMD_MODE );
	LCDIFFlush();
	LCDIFSetTransferCount( 81, DOTCLK_V_ACTIVE );
	LCDIFSetIrqEnable(LCDIF_IRQ_FRAME_DONE);   
	LCDIFDisplayFrameBufferEx( (const void *)m_PhysBase, DATA_MODE );
	LCDIFClearIrq( LCDIF_IRQ_FRAME_DONE );
	
}
