#ifndef EBOOT_SCRCONF
#define EBOOT_SCRCONF

//
// CS&ZHL FEB-09-2009: EM9161 default display format: 640*480 8bpp
//                     VTOTAL = VFP + VPW + VBP + VLINE = 525 lines
//                     VTOTAL = 10  + 2   + 33  + 480   = 525 lines
//                  -------------------------------------------------
//                     HTOTAL = HFP + HPW + HBP + HLINE = 800 lines
//                     HTOTAL = 40  + 64  + 56  + 640   = 800 lines
//
#define SCREEN_WIDTH	0x0280
#define SCREEN_HEIGHT	0x01E0
#define MARGIN_UPPER	0x0020						// (33 - 1)
#define MARGIN_LOWER	0x0009						// (10 - 1)
#define MARGIN_LEFT		0x0037						// (HBP - 1) = (56 - 1) = 55 = 0x37
#define MARGIN_RIGHT		0x0026						// (HFP - 2) = (40 - 2) = 38 = 0x26
#define VSYNC					0x01							// (2 - 1)
#define HSYNC					0x3F							// (64 - 1)
#define PIXEL_CLOCK		0x018023D8			// = 25.175MHz		
#define	VHDELAY				0x01							// (2 - 1)
#define INVPCLK				0x01

//common parameters
#define SCREEN_RES		0x08
#define FORCED_RGB		0x00
#define CACHED_VRAM		0x00
#define VRAM_ADDRESS	0x23F00000
#define VRAM_WIDTH		0x00000280
#define VRAM_HEIGHT		0x000001E0

//
// CS&ZHL AUG-12-2010: copy from "LQK setdisplay_cmd"
//
#define LCD320240_WIDTH					320
#define LCD320240_HEIGHT					240
#define LCD320240_MARGIN_UPPER	0x07
#define LCD320240_MARGIN_LOWER	0x0d
#define LCD320240_MARGIN_LEFT		0x1f
#define LCD320240_MARGIN_RIGHT		0x1f
#define LCD320240_VSYNC					0x01
#define LCD320240_HSYNC					0x0f
#define LCD320240_PIXEL_CLOCK		0x602160
#define LCD320240_VHDELAY				1
#define LCD320240_INVPCLK				0

#define LCD480272_WIDTH					480
#define LCD480272_HEIGHT					272
#define LCD480272_MARGIN_LOWER	0x02			
#define LCD480272_VSYNC					0x09			
#define LCD480272_MARGIN_UPPER	0x02			
#define LCD480272_MARGIN_RIGHT		0x00			
#define LCD480272_HSYNC					0x28			
#define LCD480272_MARGIN_LEFT		0x01			
#define LCD480272_PIXEL_CLOCK		10000000		
#define LCD480272_VHDELAY				1
#define LCD480272_INVPCLK				0

#define LCD800480_WIDTH					800
#define LCD800480_HEIGHT					480
#define LCD800480_MARGIN_LOWER	0x0b			
#define LCD800480_VSYNC					0x02			
#define LCD800480_MARGIN_UPPER	0x1a			
#define LCD800480_MARGIN_RIGHT		0x26			
#define LCD800480_HSYNC					0x2f			
#define LCD800480_MARGIN_LEFT		0x27			
#define LCD800480_PIXEL_CLOCK		27000000		
#define LCD800480_VHDELAY				1
#define LCD800480_INVPCLK				0

#define LCD800600_WIDTH					800
#define LCD800600_HEIGHT					600
#define LCD800600_MARGIN_LOWER	0x01			
#define LCD800600_VSYNC					0x03			
#define LCD800600_MARGIN_UPPER	0x17			
#define LCD800600_MARGIN_RIGHT		0x66			
#define LCD800600_HSYNC					0x3f			
#define LCD800600_MARGIN_LEFT		0x57			
#define LCD800600_PIXEL_CLOCK		40000000		// 40MHz
#define LCD800600_VHDELAY				1
#define LCD800600_INVPCLK				0

typedef enum PanelType_c
{
    DISPLAY_CHUNGHWA_CLAA057VA01CT = 0, // VGA display

    // Add your display panel here
    // ...

    numPanels           // Define the numPanel so value automaticaly the number of panel
}PanelType;

#define PANEL_TYPE_DEFAULT          DISPLAY_CHUNGHWA_CLAA057VA01CT

#endif

