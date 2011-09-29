#ifndef _EBOOT_SPLASH
#define _EBOOT_SPLASH

//typedef unsigned char	BYTE;
//typedef unsigned short	USHORT;
//typedef unsigned long	ULONG;

// LCDC Content Struct
typedef struct {
  UINT32	width;
  UINT32	height;
  UINT32	frequency;
  UINT32	Bpp;
  UINT32   Hwidth;
  UINT32   Hwait1;
  UINT32   Hwait2;
  UINT32   Vwidth;
  UINT32   Vwait1;
  UINT32   Vwait2;
  union
  {
    PCR_CFG  PCRCfg;
    UINT32   uPCRCfg;
  } PCR_CFG;
} EBOOT_LCDC_MODE, *PEBOOT_LCDC_MODE;

typedef struct EBootScreenDesc
{
	/*
//	Custom Display parameters section
	USHORT	Width;
	USHORT	Height;
	BYTE			Bpp;
	BYTE			forceRGB;
	BYTE			Cached;
	ULONG		VRAMWidthInPixel;
	ULONG		VRAMHeightInPixel;
	ULONG		VRAMaddress;

	USHORT	UpperMargin;
	USHORT	LowerMargin;
	USHORT	LeftMargin;
	USHORT	RightMargin;

	BYTE			Vsync;
	BYTE			Hsync;
	ULONG		PixelClock;

	//
	// CS&ZHL FEB-10-2009: it's critical to the stability of TFT display
	//
	ULONG		VHDelay;
	ULONG		InvPClock;
	*/
	PEBOOT_LCDC_MODE	pCFG;

	// Driver variables section
	BYTE					*pScreenBuffer;
	BYTE					*pLUT;
	// 
	// = 0: no bmp 
	// = 1: 320x240 
	// = 2: 480x272 (4.3" LCD)
	// = 3: 640x480 (default)
	// = 4: 800x480 (7" LCD)
	// = 5: 800x600 (8.4", 10.4")
	//
	DWORD				dwBMPInstalled;

	// variances for hardware LCDC, clock, etc
	PCSP_LCDC_REGS	pLCDC;
	BOOL						bClockEnable;
    UINT32						uLCDRefClk;			//LCD ref clock
	ULONG						ulLAWPhysical;		//physical address of LCD display buffer
	DWORD						*pBGLUT;				//Back Ground LUT, 256*DWORD => 1KB
	DWORD						*pGWLUT;			//Graphic Window LUT, 256*DWORD => 1KB

} EBOOTSCREENDESC,*PEBOOTSCREENDESC;

void EBOOT_SplashScreen();

void Display_GetScreenConf(PEBOOTSCREENDESC pScrDef);

void Display_Init(PEBOOTSCREENDESC pScrDef);
void Display_Deinit(PEBOOTSCREENDESC pScrDef);
void Display_Show(PEBOOTSCREENDESC pScrDef);
void Display_On(PEBOOTSCREENDESC pScrDef);
void Display_Off(PEBOOTSCREENDESC pScrDef);

#endif

