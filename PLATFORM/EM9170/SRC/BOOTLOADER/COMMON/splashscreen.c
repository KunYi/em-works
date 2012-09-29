//
//  File:  splashscreen.c
//
//  do splash screen display.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#pragma warning(push)
#pragma warning(disable: 4115)
#include <fmd.h>
#pragma warning(pop)
//#include "loader.h"
#include "common_types.h"			//CS&ZHL MAY-5-2011: supporting splash screen
#include "common_lcdc.h"
#include "splashscreen.h"				//CS&ZHL MAY-5-2011: supporting splash screen
#include "screenconfig.h"				//CS&ZHL MAY-5-2011: supporting splash screen
#include "mc34704.h"					//CS&ZHL MAY-6-2011: supporting setup power supply

extern BSP_ARGS *g_pBSPArgs;

//-----------------------------------------------------------------------------
// External Functions
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);

//-----------------------------------------------------------------------------
//
//  Function:  BOOL EBOOT_ReadFlash
//
//  CS&ZHL MAY-5-2011: This function read data from NAND flash memory 
//                                     into specified RAM area
//
//  Parameters:
//      dwRAMAddressDst:  [in] pointer of RAM area
//      dwNANDAddressSrc:	[in] nand flash offset
//      dwLen:						[in] the length in byte which need to read
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
extern BOOL EBOOT_ReadFlash(DWORD dwRAMAddressDst, DWORD dwNANDAddressSrc, DWORD dwLen);

//
// CS&ZHL AUG-11-2010: allocat static ram for frame buffer
//
#define SPLASH_SCREEN_BUFFER_SIZE				0x75800		// = 470KB, supporting up to 800*600 LCD

unsigned char FrameBuffer[SPLASH_SCREEN_BUFFER_SIZE];

//------------------------------------------------------------------------------
// Description of all supported mode for all supported panel
EBOOT_LCDC_MODE ModeArray[] =
{
    // 320*240 -> 5.7", PCLK = 6.3MHz -> LQ057 
    {
		320, 240, 60, 8,										//Width, Height, FrameFreq, BPP
		//
		// CS&ZHL JUN-29-2011: Hwidth + 1 = 16;  
		//                                     Hwait1 + 1 = 32; -> horinzontal front porch (HFP)
		//                                     Hwait2 + 3 = 32; -> horinzontal back porch (HBP)
		//                                     Htotal = (Hwait1 + 1) + (Hwidth + 1) + (Hwait2 + 3) + Width = 400
		//                                     Vwidth = 2;  
		//                                     Vwait1 = 8;  
		//                                     Vwait2 = 12;  
		//                                     Vtotal = (Vwait1) + (Vwidth) + (Hwait2) + Height = 262
		//
        16, 32, 32, 2, 8, 12,			//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
        {
            0,															// Pixel clock divier compute by common driver layer
            LCDC_PCR_SHARP_DISABLE,					//LCDC_PCR_SHARP_ENABLE
            LCDC_PCR_SCLKSEL_ENABLE,				//LCDC_PCR_SCLKSEL_ENABLE
            0,															// Not used for TFT panel
            LCDC_PCR_ACDSEL_USE_LPHSYNC,    //LCDC_PCR_ACDSEL_USE_LPHSYNC
            LCDC_PCR_REV_VS_NORMAL,         //LCDC_PCR_REV_VS_NORMAL
            LCDC_PCR_SWAP_SEL_8BPP,        //LCDC_PCR_SWAP_SEL_16BPP
            LCDC_PCR_END_SEL_LITTLE_ENDIAN, //LCDC_PCR_END_SEL_LITTLE_ENDIAN
            LCDC_PCR_SCLKIDLE_ENABLE,      //LCDC_PCR_SCLKIDLE_DISABLE
            LCDC_PCR_OEPOL_ACTIVE_HIGH,     //LCDC_PCR_OEPOL_ACTIVE_HIGH
            LCDC_PCR_CLKPOL_POS_EDGE,       //LCDC_PCR_CLKPOL_NEG_EDGE
            LCDC_PCR_LPPOL_ACTIVE_LOW,   //LCDC_PCR_LPPOL_ACTIVE_LOW
            LCDC_PCR_FLMPOL_ACTIVE_LOW,  //LCDC_PCR_FLMPOL_ACTIVE_HIGH
            LCDC_PCR_PIXPOL_ACTIVE_HIGH,    //LCDC_PCR_PIXPOL_ACTIVE_HIGH
            0,
            LCDC_PCR_PBSIZ_1BIT,            //LCDC_PCR_PBSIZ_8BIT
            LCDC_PCR_COLOR_COLOR,           //LCDC_PCR_COLOR_COLOR
            LCDC_PCR_TFT_ACTIVE             //LCDC_PCR_TFT_ACTIVE
        }
    },
    // 480*272 -> 4.3", PCLK = 9MHz -> LR430LC9001
    {
		480, 272, 60, 8,										//Width, Height, FrameFreq, BPP
		//
		// CS&ZHL JUN-29-2011: Hwidth + 1 = 37;  
		//                                     Hwait1 + 1 = 4; -> horinzontal front porch (HFP)
		//                                     Hwait2 + 3 = 4; -> horinzontal back porch (HBP)
		//                                     Htotal = (Hwait1 + 1) + (Hwidth + 1) + (Hwait2 + 3) + Width = 525
		//                                     Vwidth = 10;  
		//                                     Vwait1 = 2;  
		//                                     Vwait2 = 2;  
		//                                     Vtotal = (Vwait1) + (Vwidth) + (Hwait2) + Height = 286
		//
        37, 4, 4, 10, 2, 2,				//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
        {
            0,															// Pixel clock divier compute by common driver layer
            LCDC_PCR_SHARP_DISABLE,					//LCDC_PCR_SHARP_ENABLE
            LCDC_PCR_SCLKSEL_ENABLE,				//LCDC_PCR_SCLKSEL_ENABLE
            0,															// Not used for TFT panel
            LCDC_PCR_ACDSEL_USE_LPHSYNC,		//LCDC_PCR_ACDSEL_USE_LPHSYNC
            LCDC_PCR_REV_VS_NORMAL,				//LCDC_PCR_REV_VS_NORMAL
            LCDC_PCR_SWAP_SEL_8BPP,				//LCDC_PCR_SWAP_SEL_16BPP
            LCDC_PCR_END_SEL_LITTLE_ENDIAN,	//LCDC_PCR_END_SEL_LITTLE_ENDIAN
            LCDC_PCR_SCLKIDLE_ENABLE,				//LCDC_PCR_SCLKIDLE_DISABLE
            LCDC_PCR_OEPOL_ACTIVE_HIGH,			//LCDC_PCR_OEPOL_ACTIVE_HIGH
            LCDC_PCR_CLKPOL_POS_EDGE,			//LCDC_PCR_CLKPOL_NEG_EDGE
            LCDC_PCR_LPPOL_ACTIVE_LOW,			//LCDC_PCR_LPPOL_ACTIVE_LOW
            LCDC_PCR_FLMPOL_ACTIVE_LOW,		//LCDC_PCR_FLMPOL_ACTIVE_HIGH
            LCDC_PCR_PIXPOL_ACTIVE_HIGH,		//LCDC_PCR_PIXPOL_ACTIVE_HIGH
            0,
            LCDC_PCR_PBSIZ_1BIT,						//LCDC_PCR_PBSIZ_8BIT
            LCDC_PCR_COLOR_COLOR,					//LCDC_PCR_COLOR_COLOR
            LCDC_PCR_TFT_ACTIVE						//LCDC_PCR_TFT_ACTIVE
        }
    },
    // 640*480
    {
		640, 480, 60, 8,									//Width, Height, FrameFreq, BPP
#ifdef	EM9170
		//
		// CS&ZHL JUN-1-2011: Hwidth + 1 = 64;  -> MAX VALUE = 64!!!
		//                                   Hwait1 + 1 = 40;  -> horinzontal front porch (HFP)
		//                                   Hwait2 + 3 = 56; -> horinzontal back porch (HBP)
		//                                   Htotal = (Hwait1 + 1) + (Hwidth + 1) + (Hwait2 + 3) + Width = 800
		//                                   Vwidth = 2;  
		//                                   Vwait1 = 10;  
		//                                   Vwait2 = 33;  
		//                                   Vtotal = (Vwait1) + (Vwidth) + (Hwait2) + Height = 525
		//
        64, 40, 56, 2, 10, 33,										//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
#else	// -> iMX257PDK
        20,41,6,10,9,7,										//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
#endif	//EM9170
        {
            0,                              // Pixel clock divier compute by common driver layer
            LCDC_PCR_SHARP_DISABLE,          //LCDC_PCR_SHARP_ENABLE
            LCDC_PCR_SCLKSEL_ENABLE,        //LCDC_PCR_SCLKSEL_ENABLE
            0,                              // Not used for TFT panel
            LCDC_PCR_ACDSEL_USE_LPHSYNC,    //LCDC_PCR_ACDSEL_USE_LPHSYNC
            LCDC_PCR_REV_VS_NORMAL,         //LCDC_PCR_REV_VS_NORMAL
            LCDC_PCR_SWAP_SEL_8BPP,        //LCDC_PCR_SWAP_SEL_16BPP
            LCDC_PCR_END_SEL_LITTLE_ENDIAN, //LCDC_PCR_END_SEL_LITTLE_ENDIAN
            LCDC_PCR_SCLKIDLE_ENABLE,      //LCDC_PCR_SCLKIDLE_DISABLE
            LCDC_PCR_OEPOL_ACTIVE_HIGH,     //LCDC_PCR_OEPOL_ACTIVE_HIGH
            LCDC_PCR_CLKPOL_POS_EDGE,       //LCDC_PCR_CLKPOL_NEG_EDGE
            LCDC_PCR_LPPOL_ACTIVE_LOW,   //LCDC_PCR_LPPOL_ACTIVE_LOW
            LCDC_PCR_FLMPOL_ACTIVE_LOW,  //LCDC_PCR_FLMPOL_ACTIVE_HIGH
            LCDC_PCR_PIXPOL_ACTIVE_HIGH,    //LCDC_PCR_PIXPOL_ACTIVE_HIGH
            0,
            LCDC_PCR_PBSIZ_1BIT,            //LCDC_PCR_PBSIZ_8BIT
            LCDC_PCR_COLOR_COLOR,           //LCDC_PCR_COLOR_COLOR
            LCDC_PCR_TFT_ACTIVE             //LCDC_PCR_TFT_ACTIVE
        }
    },
    // 800*480 -> 7.0", PCLK = 33.3MHz -> AT070TN83 V1
    {
		800, 480, 68, 8,							//Width, Height, FrameFreq, BPP
		//
		// CS&ZHL JUN-29-2011: Hwidth + 1 = 48;  
		//                                     Hwait1 + 1 = 40; -> horinzontal front porch (HFP)
		//                                     Hwait2 + 3 = 40; -> horinzontal back porch (HBP)
		//                                     Htotal = (Hwait1 + 1) + (Hwidth + 1) + (Hwait2 + 3) + Width = 928
		//                                     Vwidth = 3;  
		//                                     Vwait1 = 13;  
		//                                     Vwait2 = 29;  
		//                                     Vtotal = (Vwait1) + (Vwidth) + (Hwait2) + Height = 525
		//
        48, 40, 40, 3, 13, 29,		//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
        {
            0,                              // Pixel clock divier compute by common driver layer
            LCDC_PCR_SHARP_DISABLE,          //LCDC_PCR_SHARP_ENABLE
            LCDC_PCR_SCLKSEL_ENABLE,        //LCDC_PCR_SCLKSEL_ENABLE
            0,                              // Not used for TFT panel
            LCDC_PCR_ACDSEL_USE_LPHSYNC,    //LCDC_PCR_ACDSEL_USE_LPHSYNC
            LCDC_PCR_REV_VS_NORMAL,         //LCDC_PCR_REV_VS_NORMAL
            LCDC_PCR_SWAP_SEL_8BPP,        //LCDC_PCR_SWAP_SEL_16BPP
            LCDC_PCR_END_SEL_LITTLE_ENDIAN, //LCDC_PCR_END_SEL_LITTLE_ENDIAN
            LCDC_PCR_SCLKIDLE_ENABLE,      //LCDC_PCR_SCLKIDLE_DISABLE
            LCDC_PCR_OEPOL_ACTIVE_HIGH,     //LCDC_PCR_OEPOL_ACTIVE_HIGH
            LCDC_PCR_CLKPOL_POS_EDGE,       //LCDC_PCR_CLKPOL_NEG_EDGE
            LCDC_PCR_LPPOL_ACTIVE_LOW,   //LCDC_PCR_LPPOL_ACTIVE_LOW
            LCDC_PCR_FLMPOL_ACTIVE_LOW,  //LCDC_PCR_FLMPOL_ACTIVE_HIGH
            LCDC_PCR_PIXPOL_ACTIVE_HIGH,    //LCDC_PCR_PIXPOL_ACTIVE_HIGH
            0,
            LCDC_PCR_PBSIZ_1BIT,            //LCDC_PCR_PBSIZ_8BIT
            LCDC_PCR_COLOR_COLOR,           //LCDC_PCR_COLOR_COLOR
            LCDC_PCR_TFT_ACTIVE             //LCDC_PCR_TFT_ACTIVE
        }
    },
    // 800*600 -> 8.4", 10.4", PCLK = 33.3MHz -> G084SN03, G104SN03
    {
		800, 600, 50, 8,										//Width, Height, FrameFreq, BPP
		//
		// CS&ZHL JUN-29-2011: Hwidth + 1 = 64;  
		//                                     Hwait1 + 1 = 40; -> horinzontal front porch (HFP)
		//                                     Hwait2 + 3 = 40; -> horinzontal back porch (HBP)
		//                                     Htotal = (Hwait1 + 1) + (Hwidth + 1) + (Hwait2 + 3) + Width = 944
		//                                     Vwidth = 4;  
		//                                     Vwait1 = 1;  
		//                                     Vwait2 = 20;  
		//                                     Vtotal = (Vwait1) + (Vwidth) + (Hwait2) + Height = 625
		//
        64, 40, 40, 4, 1, 20,		//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
        {
            0,                              // Pixel clock divier compute by common driver layer
            LCDC_PCR_SHARP_DISABLE,          //LCDC_PCR_SHARP_ENABLE
            LCDC_PCR_SCLKSEL_ENABLE,        //LCDC_PCR_SCLKSEL_ENABLE
            0,                              // Not used for TFT panel
            LCDC_PCR_ACDSEL_USE_LPHSYNC,    //LCDC_PCR_ACDSEL_USE_LPHSYNC
            LCDC_PCR_REV_VS_NORMAL,         //LCDC_PCR_REV_VS_NORMAL
            LCDC_PCR_SWAP_SEL_8BPP,        //LCDC_PCR_SWAP_SEL_16BPP
            LCDC_PCR_END_SEL_LITTLE_ENDIAN, //LCDC_PCR_END_SEL_LITTLE_ENDIAN
            LCDC_PCR_SCLKIDLE_ENABLE,      //LCDC_PCR_SCLKIDLE_DISABLE
            LCDC_PCR_OEPOL_ACTIVE_HIGH,     //LCDC_PCR_OEPOL_ACTIVE_HIGH
            LCDC_PCR_CLKPOL_POS_EDGE,       //LCDC_PCR_CLKPOL_NEG_EDGE
            LCDC_PCR_LPPOL_ACTIVE_LOW,   //LCDC_PCR_LPPOL_ACTIVE_LOW
            LCDC_PCR_FLMPOL_ACTIVE_LOW,  //LCDC_PCR_FLMPOL_ACTIVE_HIGH
            LCDC_PCR_PIXPOL_ACTIVE_HIGH,    //LCDC_PCR_PIXPOL_ACTIVE_HIGH
            0,
            LCDC_PCR_PBSIZ_1BIT,            //LCDC_PCR_PBSIZ_8BIT
            LCDC_PCR_COLOR_COLOR,           //LCDC_PCR_COLOR_COLOR
            LCDC_PCR_TFT_ACTIVE             //LCDC_PCR_TFT_ACTIVE
        }
    }
};

//-----------------------------------------------------------------------------
//! \brief		void Display_On(PEBOOTSCREENDESC pScrDef)
//!
//! Turns backlight on
//-----------------------------------------------------------------------------
void Display_On(PEBOOTSCREENDESC pScrDef)
{
	PCSP_GPIO_REGS pGPIO3;

    UNREFERENCED_PARAMETER(pScrDef);

	RETAILMSG(1, (L"-> Display_On\r\n"));

	//
	// CS&ZHL JUN-1-2011: iMX257PDK has REG5_3V3 only
	//
//#ifndef	EM9170
	//setup REG5_3V3 -> VDD_LCDIO -> LCD_3V3 -> LCD_VCC
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_I2C1, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, DDK_CLOCK_GATE_MODE_ENABLED);
    
    PmicOpen(); 
    PmicEnable(TRUE);

    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_I2C1, DDK_CLOCK_GATE_MODE_DISABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, DDK_CLOCK_GATE_MODE_DISABLED);
//#endif	//EM9170

    pGPIO3 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO3);
    if (pGPIO3 == NULL)
    {
        OALMSG(TRUE, (L"Display_On: GPIO3 null pointer!\r\n"));
        return;
    }

#ifdef	EM9170
	//
	// CS&ZHL JUN-1-2011: LCD_PWR -> GPIO3_20 as output, active low
	//
    // Configure GPIO3_20 (LCDC_PWR) as an ouput    
    OUTREG32 (&pGPIO3->GDIR, (INREG32(&pGPIO3->GDIR) | (1 << 20)));
    // Clear GPIO3_20      
    OUTREG32 (&pGPIO3->DR, (INREG32(&pGPIO3->DR) & ~(1 << 20)));
#else	// iMX257PDK
	//
	// LCDC_EN -> select GPIO3[17], output, active low
	//

    // Configure GPIO3_17 (LCDC_EN) as an ouput    
    OUTREG32 (&pGPIO3->GDIR, (INREG32(&pGPIO3->GDIR) | (1 << 17)));
    // Clear GPIO3_17      
    OUTREG32 (&pGPIO3->DR, (INREG32(&pGPIO3->DR) & ~(1 << 17)));
#endif	//EM9170
}

//-----------------------------------------------------------------------------
//! \brief		void SetLut(PEBOOTSCREENDESC pScrDef,BYTE *pLUT)
//!
//!	This function takes the LUT table from GIMP and set it into the LCDC LUT
//-----------------------------------------------------------------------------
void SetLut(PEBOOTSCREENDESC pScrDef)
{
	int				i, i4;
	DWORD		r, g, b;
	DWORD		color;

	if(pScrDef->dwBMPInstalled)
	{
		for(i = 0; i < 256; i++)
		{
			i4 = i * 4;
			b = (DWORD)pScrDef->pLUT[i4] & 0xFF;
			g = (DWORD)pScrDef->pLUT[i4 + 1] & 0xFF;
			r = (DWORD)pScrDef->pLUT[i4 + 2] & 0xFF;
			color = ((r >> 2) << 12) | ((g >> 2) << 6) | (b >> 2);
			OUTREG32(&pScrDef->pBGLUT[i], color);
			OUTREG32(&pScrDef->pGWLUT[i], color);
		}
	}
	else
	{
		// default color = CYAN <=> default index = 0
		r = 0;
		g = 0xff;
		b = 0xff;
		color = ((r >> 2) << 12) | ((g >> 2) << 6) | (b >> 2);
		OUTREG32(&pScrDef->pBGLUT[0], color);
		OUTREG32(&pScrDef->pGWLUT[0], color);
	}
}

//-----------------------------------------------------------------------------
//! \brief		void SetBitmap(PEBOOTSCREENDESC pScrDef,USHORT Width,USHORT Height,BYTE *pPixmap)
//!
//!	This function takes the bitmap and copy it into memory
//! WARNING: the bitmap has to be the size of the screen or it will might be
//!			aligned.
//-----------------------------------------------------------------------------
void SetBitmap(PEBOOTSCREENDESC pScrDef)
{
	if(!pScrDef->dwBMPInstalled)
	{
		// set Frame Buffer to default color index = 0;
		memset(pScrDef->pScreenBuffer, 0, (pScrDef->pCFG->width * pScrDef->pCFG->height));
		RETAILMSG(1, (L"Clear Frame Buffer\r\n"));
	}
}


//-----------------------------------------------------------------------------
//! \brief		void Display_Show(PEBOOTSCREENDESC pScrDef,PEBITMAP pBmp)
//!
//! Print a picture on the initialized screen
//-----------------------------------------------------------------------------
void Display_Show(PEBOOTSCREENDESC pScrDef)
{
	RETAILMSG(1, (L"-> Display_Show\r\n"));

	SetLut(pScrDef);
	SetBitmap(pScrDef);
}

//------------------------------------------------------------------------------
//
// Function: LCDCEnableClock
//
// This function enable or disable Lcdc clock for LCD panel mode.
//
// Parameters:
//      bEnable
//          [in] This argument is enable or disable Lcdc clock for LCD panel mode.
//
// Returns:
//      Return operation status.
//      TRUE: Success
//      FALSE: Error
//
//------------------------------------------------------------------------------
void LCDCEnableClock(BOOL bEnable)
{
	if(bEnable)
    {
        // Enable LCDC
        OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_ENABLED);
        OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_LCDC, DDK_CLOCK_GATE_MODE_ENABLED);
        OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_LCDC, DDK_CLOCK_GATE_MODE_ENABLED);
    }
    else
    {
        // Disable LCDC
        OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_DISABLED);
        OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_LCDC, DDK_CLOCK_GATE_MODE_DISABLED);
        OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_LCDC, DDK_CLOCK_GATE_MODE_DISABLED);
     }
}


//-----------------------------------------------------------------------------
//! \brief		BOOL LCDC6xhw(PEBOOTSCREENDESC pScrDef)
//!
//! Setup LCDC hardware enviroment
//! return = FALSE: faile
//-----------------------------------------------------------------------------
BOOL LCDC6xhw(PEBOOTSCREENDESC pScrDef)
{
    BOOL		bState = TRUE;

	// Initialization
	pScrDef->pLCDC = (PCSP_LCDC_REGS)OALPAtoUA(CSP_BASE_REG_PA_LCDC);
	
	//Back Ground LUT, 256*DWORD => 1KB
	pScrDef->pBGLUT = (DWORD*)OALPAtoUA((DWORD)CSP_BASE_REG_PA_LCDC + 0x800);				
	//Graphic Window LUT, 256*DWORD => 1KB
	pScrDef->pGWLUT = (DWORD*)OALPAtoUA((DWORD)CSP_BASE_REG_PA_LCDC + 0xC00);
	
	// get LCDC ref clock
    //DDKClockGetFreq(DDK_CLOCK_SIGNAL_PER_LCDC, &freq);
    pScrDef->uLCDRefClk = g_pBSPArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_LCDC];
	RETAILMSG(1, (TEXT("LCDC6xhw::LCDRefClk = %d\r\n"), pScrDef->uLCDRefClk));
	
	pScrDef->ulLAWPhysical = OALVAtoPA(pScrDef->pScreenBuffer);

	return bState;
}

//-----------------------------------------------------------------------------
//! \brief		BOOL ChipConfig(PEBOOTSCREENDESC pScrDef)
//!
//! Internal Power up of the LCDC & DMA, Turn off the backlight, Set the clock,
//!	Set the default palette (all black)
//-----------------------------------------------------------------------------
BOOL ChipConfig(PEBOOTSCREENDESC pScrDef)
{
    BOOL							result = FALSE;
    DWORD							dHtotal, dVtotal, dPCR;
    UINT32							uLCDRefClk, uLCDPixClk;
	PCSP_LCDC_REGS		pLCDC = pScrDef->pLCDC;
	PEBOOT_LCDC_MODE	pCFG = pScrDef->pCFG;
	UINT32							uTemp;
	UINT32							uPCD;

    // Disable LCDC clock
    LCDCEnableClock(FALSE);

    // LCDC Refresh Mode Control Register
    CLRREG32(&pLCDC->RMCR, CSP_BITFMASK(LCDC_RMCR_SELF_REF));

    // LCDC Screen Start Address Register
    OUTREG32(&pLCDC->SSAR, pScrDef->ulLAWPhysical);

    // LCDC Size Register
    OUTREG32(&pLCDC->SR,              
              (CSP_BITFVAL(LCDC_SR_XMAX, (pCFG->width / 16))|
              CSP_BITFVAL(LCDC_SR_YMAX, (pCFG->height))));

    // LCDC Virtual Page Width Register
    OUTREG32(&pLCDC->VPWR, 
        (pCFG->width / (32/pCFG->Bpp))); // the number of 32-bit words

    // LCDC Cursor Position Register
    // Default: disable cursor
    OUTREG32(&pLCDC->CPR, 
                (CSP_BITFVAL(LCDC_CPR_OP, LCDC_CPR_OP_DISABLE) |
                CSP_BITFVAL(LCDC_CPR_CC, LCDC_CPR_CC_DISABLED)));

    // LCDC Cursor Width Height and Blink Register
    // Default: disable cursor
    OUTREG32(&pLCDC->CWHBR, 
                  (CSP_BITFVAL(LCDC_CWHBR_BK_EN, LCDC_CWHBR_BK_EN_DISABLE) |
                  CSP_BITFVAL(LCDC_CWHBR_CW, LCDC_CWHBR_CW_CURSOR_DISABLED) |
                  CSP_BITFVAL(LCDC_CWHBR_CH, LCDC_CWHBR_CH_CURSOR_DISABLED) |
                  CSP_BITFVAL(LCDC_CWHBR_BD, LCDC_CWHBR_BD_MAX_DIV)));

    // LCDC Color Cursor Mapping Register
    OUTREG32(&pLCDC->CCMR, 0);

    // Get the LCD Reference clock
    // PGAL : Caution, for TV OUT mode the reference clock are 266400000 / PERDIV3
    //uLCDRefClk = LCDCGetRefClk(&m_ModeDesc);
    uLCDRefClk = pScrDef->uLCDRefClk;

    dHtotal = pCFG->width + pCFG->Hwidth + pCFG->Hwait1 + pCFG->Hwait2;
    dVtotal = pCFG->height + pCFG->Vwidth + pCFG->Vwait1 + pCFG->Vwait2;
    uLCDPixClk = dHtotal * dVtotal * pCFG->frequency;

    // Configure the PCR register using panel specfic information fill in board specific layer
    dPCR = pCFG->PCR_CFG.uPCRCfg;

    // Configure pixel divider value
    //dPCR |= CSP_BITFVAL(LCDC_PCR_PCD, LCDC_PCD_VALUE(uLCDRefClk, uLCDPixClk));
	//
	// CS&ZHL JUN-29-2011: 
	//
	uPCD = uLCDRefClk / uLCDPixClk;
	if(uPCD > 1)
	{
		uTemp = (uLCDRefClk * 10) / uLCDPixClk;
		if((uTemp % 10) < 5)
		{
			uPCD--;
		}
	}
    dPCR |= CSP_BITFVAL(LCDC_PCR_PCD, uPCD);

    switch(pCFG->Bpp)
    {
        case 1:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_1BPP);
            break;
        case 2:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_2BPP);
            break;
        case 4:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_4BPP);
            break;
        case 8:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_8BPP);
            break;
        case 12:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_12BPP);
            break;
        case 16:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_16BPP);
            break;
        case 18:
            dPCR |= CSP_BITFVAL(LCDC_PCR_BPIX, LCDC_PCR_BPIX_18BPP);
            break;
        default:
            goto cleanup;
    }
    OUTREG32(&pLCDC->PCR, dPCR);

    // LCDC Horizontal Configuration Register
    OUTREG32(&pLCDC->HCR, 
              (CSP_BITFVAL(LCDC_HCR_H_WIDTH, (pCFG->Hwidth - 1))|
              CSP_BITFVAL(LCDC_HCR_H_WAIT_1, (pCFG->Hwait1 - 1))|
              CSP_BITFVAL(LCDC_HCR_H_WAIT_2, (pCFG->Hwait2 - 3))));

    // LCDC Vertical Configuration Register
    OUTREG32(&pLCDC->VCR, 
             (CSP_BITFVAL(LCDC_VCR_V_WIDTH, (pCFG->Vwidth))|
             CSP_BITFVAL(LCDC_VCR_V_WAIT_1, (pCFG->Vwait1))|
             CSP_BITFVAL(LCDC_VCR_V_WAIT_2, (pCFG->Vwait2))));

    
    // LCDC Panning Offset Register
    OUTREG32(&pLCDC->POR, 0);

    // LCDC Sharp Configuration Register
    OUTREG32(&pLCDC->SCR, 
          (CSP_BITFVAL(LCDC_SCR_GRAY1, 0) |
          CSP_BITFVAL(LCDC_SCR_GRAY2, 0) |
          CSP_BITFVAL(LCDC_SCR_REV_TOGGLE_DELAY, 3) |
          CSP_BITFVAL(LCDC_SCR_CLS_RISE_DELAY, 18) |
          CSP_BITFVAL(LCDC_SCR_PS_RISE_DELAY, 0))); // 0001 = 2 LSCLK period

    // Check if a value has already been assigned for the backlight
    // If pwm not already enabled by backligth driver, enabled it and set a default value
    if ((INREG32(&pLCDC->PCCR) & CSP_BITFMASK(LCDC_PCCR_CC_EN)) == 0)
    {
        // LCDC PWM Contrast Control Register
        OUTREG32(&pLCDC->PCCR,
              (CSP_BITFVAL(LCDC_PCCR_PW, (LCDC_PCCR_PW_MAX / 2)) |
               CSP_BITFVAL(LCDC_PCCR_CC_EN, LCDC_PCCR_CC_EN_ENABLE) |
               CSP_BITFVAL(LCDC_PCCR_SCR, LCDC_PCCR_SCR_LCDCLK) |
               CSP_BITFVAL(LCDC_PCCR_LDMSK, LCDC_PCCR_LDMSK_DISABLE) |
               CSP_BITFVAL(LCDC_PCCR_CLS_HI_WIDTH, 169)
              ));
    }

    // LCDC DMA Control Register
    OUTREG32(&pLCDC->DCR,
              (CSP_BITFVAL(LCDC_DCR_BURST, LCDC_DCR_BURST_DYNAMIC)|
              CSP_BITFVAL(LCDC_DCR_HM, (0x04))|   // DMA High Mark
              CSP_BITFVAL(LCDC_DCR_TM, (0x68)))); // DMA Trigger Mark

    // LCDC Interrupt Configuration Register
    OUTREG32(&pLCDC->ICR,
          (CSP_BITFVAL(LCDC_ICR_GW_INT_CON, LCDC_ICR_GW_INT_CON_END) |
          CSP_BITFVAL(LCDC_ICR_INTSYN, LCDC_ICR_INTSYN_PANEL) |
          CSP_BITFVAL(LCDC_ICR_INTCON, LCDC_ICR_INTCON_BOF)));

    // LCDC Interrupt Enable Register
    OUTREG32(&pLCDC->IER, 0);

    // LCDC Graphic Window

    // LCDC Graphic Window Start Address Register
    OUTREG32(&pLCDC->GWSAR, pScrDef->ulLAWPhysical);

    // LCDC Graphic Window DMA Control Register
    OUTREG32(&pLCDC->GWDCR, 
              (CSP_BITFVAL(LCDC_GWDCR_GWBT, LCDC_GWDCR_GWBT_DYNAMIC)|
              CSP_BITFVAL(LCDC_GWDCR_GWHM, 0x02)|
              CSP_BITFVAL(LCDC_GWDCR_GWTM, 0x10)));


    // Graphic window at first time can only be enabled while the HCLK to the LCDC is disabled. 
    // Once enabled it can subsequently be disabled and enabled without turning off the HCLK.
    // So we need to enable and then disable the graphic window at hardware init part(configlcdc),
    // then at next time to enable graphic window, the HCLK to LCDC does not need to be disabled, and the flicker (due to disabling of HCLK to LCDC) is avoided.
    {
        // Enable graphic window
        // LCDC Graphic Window Size Register
        OUTREG32(&pLCDC->GWSR, 
                 (CSP_BITFVAL(LCDC_GWSR_GWW, (pCFG->width >> 4))|
                  CSP_BITFVAL(LCDC_GWSR_GWH, pCFG->height)));

        // LCDC Graphic Window Virtual Page Width Register
        OUTREG32(&pLCDC->GWVPWR, 
                (pCFG->width / (32/pCFG->Bpp))); // the number of 32-bit words

        // LCDC Graphic Window Position Register
        OUTREG32(&pLCDC->GWPR, 
                  (CSP_BITFVAL(LCDC_GWPR_GWXP, 0)|
                  CSP_BITFVAL(LCDC_GWPR_GWYP, 0)));

        // LCDC Graphic Window Panning Offset Register
        OUTREG32(&pLCDC->GWPOR, 
                  16);

        // LCDC Graphic Window Control Registers
        OUTREG32(&pLCDC->GWCR, 
                  (CSP_BITFVAL(LCDC_GWCR_GWAV, LCDC_GWCR_GWAV_OPAQUE)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKE, LCDC_GWCR_GWCKE_DISABLE)|
                  CSP_BITFVAL(LCDC_GWCR_GWE, LCDC_GWCR_GWE_ENABLE)|
                  CSP_BITFVAL(LCDC_GWCR_GW_RVS, LCDC_GWCR_GW_RVS_NORMAL)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKR, 0)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKG, 0)|
                  CSP_BITFVAL(LCDC_GWCR_GWCKB, 0)));
  
        // Disable graphic window
        CLRREG32(&pLCDC->GWCR, CSP_BITFMASK(LCDC_GWCR_GWAV));

        // LCDC Graphic Window Size Register
        OUTREG32(&pLCDC->GWSR, 
                  (CSP_BITFVAL(LCDC_GWSR_GWW, (16 >> 4))|
                  CSP_BITFVAL(LCDC_GWSR_GWH, 16)));
        
		//Sleep(100);
        OALStall(100000);		// 100ms

        CLRREG32(&pLCDC->GWCR, CSP_BITFMASK(LCDC_GWCR_GWE));
    }

    LCDCEnableClock(TRUE);

    result = TRUE;

cleanup:
    return result;
}

//-----------------------------------------------------------------------------
//! \brief		void IOConfig(PEBOOTSCREENDESC pScrDef)
//!
//! Internal function, set PIO/Peripheral configuration based on the information
//!	given in ScrConf.h
//-----------------------------------------------------------------------------
void IOConfig(PEBOOTSCREENDESC pScrDef)
{
    PCSP_IOMUX_REGS pIOMUX;

	UNREFERENCED_PARAMETER(pScrDef);

    pIOMUX = (PCSP_IOMUX_REGS) OALPAtoUA(CSP_BASE_REG_PA_IOMUXC);
    if (pIOMUX == NULL)
    {
        OALMSG(TRUE, (L"IOConfig: IOMUXC mapping failed!\r\n"));
        return;
    }

	//Enable LCDC module pins
    //LCDC_D0
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD0, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD0, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D1
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD1, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD1, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D2
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD2, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD2, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D3
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD3, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD3, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D4
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD4, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD4, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D5
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD5, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD5, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D6
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD6, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD6, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D7
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD7, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD7, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D8
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD8, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD8, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D9
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD9, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD9, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D10
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD10, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD10, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D11
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD11, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD11, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D12
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD12, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD12, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D13
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD13, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD13, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D14
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD14, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD14, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D15
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LD15, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LD15, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_HSYNC
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_HSYNC, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_HSYNC, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_VSYNC
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_VSYNC, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_VSYNC, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_PCLK
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_LSCLK, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_LSCLK, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_DEN ??
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_OE_ACD, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_OE_ACD, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_CONTRAST
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_CONTRAST, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_CONTRAST, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

#ifdef	EM9170
    // CS&ZHL JUN-1-2011: LCDC_EN -> LCD_PWR: select GPIO3[20], output, active low
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_CS4, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_CS4, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_1V8);

    //LCDC_D16
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_D15, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR); // LCD_D16
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_D15, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D17
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_D14, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR); // LCD_D17
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_D14, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
#else	// ->iMX257PDK
    //LCDC_EN -> select GPIO3[17], output, active low
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_VSTBY_REQ, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_VSTBY_REQ, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

	//LCDC_D16
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_GPIO_E, DDK_IOMUX_PIN_MUXMODE_ALT2, DDK_IOMUX_PIN_SION_REGULAR); // LCD_D16
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_GPIO_E, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    //LCDC_D17
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_GPIO_F, DDK_IOMUX_PIN_MUXMODE_ALT2, DDK_IOMUX_PIN_SION_REGULAR); // LCD_D17
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_GPIO_F, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
#endif	//EM9170
}

//-----------------------------------------------------------------------------
//! \brief		void Display_Init(PEBOOTSCREENDESC pScrDef)
//!
//! Initalize the screen depending on the information contained in pScrDef.
//-----------------------------------------------------------------------------
void Display_Init(PEBOOTSCREENDESC pScrDef)
{
	RETAILMSG(1, (L"-> Display_Init\n\r"));
	
	//	Power On LCDC & Frame Buffer
	LCDC6xhw(pScrDef);

	//	LCDC Configuration
	ChipConfig(pScrDef);

	// Initialize IOs and peripherals
	IOConfig(pScrDef); 
}


//
// return > 0: file size in byte
//        = 0: bmp file not found!
// 
DWORD FindBMP()
{
	DWORD							dwNandAddress;		// nandflash address where customer's bmp file is stored in
	UCHAR*						pNKAddress;				// temp buffer for bmp file from nandflash
	BITMAPFILEHEADER*		pBmpFileHead;
	BITMAPINFOHEADER*	pBmpInfoHead;

	RETAILMSG(1, (TEXT("->FindBMP\r\n")));
	dwNandAddress = IMAGE_BOOT_SPLASH_NAND_OFFSET;		
	pNKAddress       = OALPAtoVA((DWORD)IMAGE_BOOT_NKIMAGE_RAM_PA_START, FALSE);		// -> (DDR2_start_address + 2MB)
	RETAILMSG(1, (TEXT("FindBMP::bmpNANDAddress=0x%08x\r\n"), dwNandAddress));
	RETAILMSG(1, (TEXT("FindBMP::bmpBufferAddress=0x%p\r\n"), pNKAddress));

	//read the 1st 2KB data for check
	EBOOT_ReadFlash((DWORD)pNKAddress, dwNandAddress, 2048);
	pBmpFileHead = (BITMAPFILEHEADER*)&pNKAddress[0];

	if(pBmpFileHead->bfType != 0x4D42)		// => "MB"
	{
		RETAILMSG(1, (TEXT("FindBMP::no BMP file 0x%x\r\n"), pBmpFileHead->bfType));
		return 0;
	}

	pBmpInfoHead = (BITMAPINFOHEADER*)&pNKAddress[sizeof(BITMAPFILEHEADER)];
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
// return = 5: 800x600 (8.4", 10.4")
//
DWORD GetBMP(PEBOOTSCREENDESC pScrDef, DWORD dwLen)
{
	DWORD							dwNandAddress;		// nandflash address where customer's bmp file is stored in
	UCHAR*						pNKAddress;				// temp buffer for bmp file from nandflash
	BITMAPFILEHEADER*		pBmpFileHead;
	BITMAPINFOHEADER*	pBmpInfoHead;
	DWORD							i1, i2, i3;
	DWORD							dwDispFormat = 0;

	dwNandAddress = IMAGE_BOOT_SPLASH_NAND_OFFSET;		
	pNKAddress       = OALPAtoVA((DWORD)IMAGE_BOOT_NKIMAGE_RAM_PA_START, FALSE);		// -> (DDR2_start_address + 2MB)

	//read all file
	EBOOT_ReadFlash((DWORD)pNKAddress, dwNandAddress, dwLen);
	pBmpFileHead = (BITMAPFILEHEADER*)&pNKAddress[0];
	pBmpInfoHead = (BITMAPINFOHEADER*)&pNKAddress[sizeof(BITMAPFILEHEADER)];

	//get display format
	//pScrDef->Width  = (USHORT)(pBmpInfoHead->biWidth);
	//pScrDef->Height = (USHORT)(pBmpInfoHead->biHeight);
	if((pBmpInfoHead->biWidth == 320) && (pBmpInfoHead->biHeight == 240))
	{
		dwDispFormat = 1;
	}
	else if((pBmpInfoHead->biWidth == 480) && (pBmpInfoHead->biHeight == 272))
	{
		dwDispFormat = 2;
	}
	else if((pBmpInfoHead->biWidth == 640) && (pBmpInfoHead->biHeight == 480))
	{
		dwDispFormat = 3;
	}
	else if((pBmpInfoHead->biWidth == 800) && (pBmpInfoHead->biHeight == 480))
	{
		dwDispFormat = 4;
	}
	else if((pBmpInfoHead->biWidth == 800) && (pBmpInfoHead->biHeight == 600))
	{
		dwDispFormat = 5;
	}
	else
	{
		dwDispFormat = 0;
	}

	if(dwDispFormat)
	{
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

	return dwDispFormat;
}


//-----------------------------------------------------------------------------
//! \brief		void Display_GetScreenConf(PEBOOTSCREENDESC pScrDef)
//!
//! Fill up the EBOOTSCREENDESC given as a parameter 
//-----------------------------------------------------------------------------
void Display_GetScreenConf(PEBOOTSCREENDESC pScrDef)
{
	DWORD		dwLen;

	RETAILMSG(1, (L"-> Display_GetScreenConf\r\n"));

	memset(pScrDef, 0, sizeof(EBOOTSCREENDESC));

	//
	// CS&ZHL AUG-11-2010: Frame buffer points to eboot ram area, and LUT is in the last 1KB
	//
	pScrDef->pScreenBuffer = FrameBuffer;
	//pScrDef->pScreenBuffer = OALPAtoUA(0x83F80000);			// last 512KB of SDRAM
	pScrDef->pLUT				 = pScrDef->pScreenBuffer + (SPLASH_SCREEN_BUFFER_SIZE - 0x400);

	//
	// CS&ZHL AUG-11-2010: try to read bmp image from nandflash
	//
	dwLen = FindBMP();
	if(dwLen)
	{
		pScrDef->dwBMPInstalled = GetBMP(pScrDef, dwLen);
	}

	//setup LCDC parameters ->
	switch(pScrDef->dwBMPInstalled)
	{
	case 1:		// case of 320*240
		RETAILMSG(1, (TEXT("Display Format = 320*240\r\n")));
		pScrDef->pCFG = (PEBOOT_LCDC_MODE)&ModeArray[0];
		break;

	case 2:		// case of 480*272
		RETAILMSG(1, (TEXT("Display Format = 480*272\r\n")));
		pScrDef->pCFG = (PEBOOT_LCDC_MODE)&ModeArray[1];
		break;

	case 3:		// case of 640*480
		RETAILMSG(1, (TEXT("Display Format = 640*480\r\n")));
		pScrDef->pCFG = (PEBOOT_LCDC_MODE)&ModeArray[2];
		break;

	case 4:		// case of 800*480
		RETAILMSG(1, (TEXT("Display Format = 800*480\r\n")));
		pScrDef->pCFG = (PEBOOT_LCDC_MODE)&ModeArray[3];
		break;

	case 5:		// case of 800*600
		RETAILMSG(1, (TEXT("Display Format = 800*600\r\n")));
		pScrDef->pCFG = (PEBOOT_LCDC_MODE)&ModeArray[4];
		break;

	default:	// default setting
		RETAILMSG(1, (TEXT("Display Format Not Set, default setting 640*480\r\n")));
		pScrDef->pCFG = (PEBOOT_LCDC_MODE)&ModeArray[2];
	}

	//set common parameters

	//Initializing hardware Variables later
}


//
// CS&ZHL MAY-4-2011: add slpash function
//
void Eboot_SplashScreen(VOID)
{
	EBOOTSCREENDESC	eScreenDesc;

	KITLOutputDebugString("-> Eboot Splash Screen\r\n");

	Display_GetScreenConf(&eScreenDesc);
	Display_Init(&eScreenDesc);
	Display_Show(&eScreenDesc);
	Display_On(&eScreenDesc);
}
