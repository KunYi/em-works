/*
 *   Copyright (c) Texas Instruments Incorporated 2009. All Rights Reserved.
 *
 *   Use of this software is controlled by the terms and conditions found 
 *   in the license agreement under which this software has been supplied.
 * 
 *   l137-lcdc.h
 */

#ifndef __LCDC_H__
#define __LCDC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define		MAX_PALETTE_SIZE		0x00001000

//DISPC_PIXELFORMAT (pixel formats for LCDC)

#define DISPC_PIXELFORMAT_BITMAP1               (0x0)
#define DISPC_PIXELFORMAT_BITMAP2               (0x1)
#define DISPC_PIXELFORMAT_BITMAP4               (0x2)
#define DISPC_PIXELFORMAT_BITMAP8               (0x3)
#define DISPC_PIXELFORMAT_RGB12                 (0x4)
#define DISPC_PIXELFORMAT_ARGB16                (0x5)
#define DISPC_PIXELFORMAT_RGB16                 (0x6)
#define DISPC_PIXELFORMAT_RGB32                 (0x8)
#define DISPC_PIXELFORMAT_RGB24                 (0x9)
#define DISPC_PIXELFORMAT_YUV2                  (0xA)
#define DISPC_PIXELFORMAT_UYVY                  (0xB)
#define DISPC_PIXELFORMAT_ARGB32                (0xC)
#define DISPC_PIXELFORMAT_RGBA32                (0xD)
#define DISPC_PIXELFORMAT_RGBx32                (0xE)

enum fb_update_mode {
	FB_UPDATE_DISABLED = 0,
	FB_AUTO_UPDATE,
};

enum panel_shade {
	MONOCHROME = 0,
	COLOR_ACTIVE,
	COLOR_PASSIVE,
};

#define LCDC_INV_VSYNC			0x0001  //20
#define LCDC_INV_HSYNC			0x0002  //21
#define LCDC_INV_PIX_CLOCK		0x0004  //22
#define LCDC_INV_OUTPUT_EN		0x0008  //23
#define LCDC_HSVS_FALLING_EDGE	0x0010  //24
#define LCDC_HSVS_CONTROL		0x0020  //25
#define LCDC_SIGNAL_MASK		0x003f 
#define LCDC_PANEL_TFT			0x0100

#define res_size(_r) (((_r)->end - (_r)->start) + 1)

struct lcd_panel {
	int				config;		/* TFT/STN, signal inversion */
	int				bpp;		/* Pixel format in fb mem */

	int				x_res, y_res;
	int				pixel_clock;	/* In kHz */
	int				hsw;		/* Horizontal synchronization
									pulse width */
	int				hfp;		/* Horizontal front porch */
	int				hbp;		/* Horizontal back porch */
	int				vsw;		/* Vertical synchronization
									pulse width */
	int				vfp;		/* Vertical front porch */
	int				vbp;		/* Vertical back porch */
	int				acb;		/* ac-bias pin frequency */
	int				mono_8bit_mode;
	int                         tft_alt_mode;
	enum  panel_shade			panel_shade;   

	int				(*init)		(void);
};

#define CTRL_DIV_MASK		0xff00
#define CTRL_RASTER_MODE	0x0001
#define DMA_BURST_LEN_SHIFT	4

enum lcdc_load_mode {
	LCDC_LOAD_PALETTE_AND_FRAME = 0,
	LCDC_LOAD_PALETTE,
	LCDC_LOAD_FRAME,
};

/* Register definitions */
/* IRQ Status register*/
#define LCDC_STAT_DONE			(1 << 0)
#define LCDC_STAT_SYNC_LOST	(1 << 2)
#define LCDC_STAT_AC_BIAS		(1 << 3)
#define LCDC_STAT_FUF			(1 << 5)
#define LCDC_STAT_PALETTE_LOADED	(1 << 6)
#define LCDC_STAT_FRAME_DONE0	(1 << 8)
#define LCDC_STAT_FRAME_DONE1	(1 << 9)

/* RASTER CTRL register */
#define LCDC_CTRL_LCD_EN		(1 << 0)
#define LCDC_CTRL_MONOCHROME_MODE	(1 << 1)
#define LCDC_CTRL_LCD_TFT		(1 << 7)
#define LCDC_CTRL_RASTER_ORDER	(1 << 8)
#define LCDC_CTRL_MONO_8BIT_MODE	(1 << 9)
#define LCDC_CTRL_LINE_IRQ_CLR_SEL	(1 << 10)
#define LCDC_CTRL_TFT_ALT_ENABLE	(1 << 23)
#define LCDC_CTRL_LCD_STN_565	(1 << 24)
#define LCDC_CTRL_LCD_TFT_24	(1 << 25)
#define LCDC_CTRL_TFT_24BPP_UNPACK	(1 << 26)

/* CLKC_ENABLE register */
#define LCDC_DMA_CLK_EN		(1 << 2)
#define LCDC_LIDD_CLK_EN		(1 << 1)
#define LCDC_CORE_CLK_EN		(1 << 0)

/* LCD DMA Control Register */
#define LCDC_DMA_BURST_SIZE(x)		((x) << 4)
#define LCDC_DMA_BURST_1			0x0
#define LCDC_DMA_BURST_2			0x1
#define LCDC_DMA_BURST_4			0x2
#define LCDC_DMA_BURST_8			0x3
#define LCDC_DMA_BURST_16			0x4
#define LCDC_DUAL_FRAME_BUFFER_ENABLE	(1<<0)

#define LCDC_AC_BIAS_TRANSITIONS_PER_INT(x)	((x) << 16)
#define LCDC_AC_BIAS_FREQUENCY(x)		((x) << 8)

/* LCD IRQ Set/Clear Register */
#define LCDC_END_OF_FRAME_INT_ENA		(1<<0)
#define LCDC_END_OF_FRAME0_INT_ENA	(1<<8)
#define LCDC_END_OF_FRAME1_INT_ENA	(1<<9)
#define LCDC_UNDERFLOW_INT_ENA			(1<<5)
#define LCDC_PALETTE_INT_ENA		(1 << 6)

typedef struct {
	volatile UINT32	revision;		//0x0
	volatile UINT32	control;			//0x4
	volatile UINT32	status;			//0x8, still available?
	UINT32   res1[7];					//0xc, LIDD related registers
	volatile UINT32	raster_control;	//0x28
	volatile UINT32	timing0;			//0x2c
	volatile UINT32	timing1;			//0x30
	volatile UINT32	timing2;			//0x34
	volatile UINT32	subpanel1;		//0x38
	volatile UINT32	subpanel2;		//0x3c
	volatile UINT32	dma_control;	//0x40	
	volatile UINT32	fb0_base;		//0x44
	volatile UINT32	fb0_ceiling;		//0x48
	volatile UINT32	fb1_base;		//0x4c
	volatile UINT32	fb1_ceiling;		//0x50
	volatile UINT32	sysconfig; 		//0x54
	volatile UINT32 	IRQstatus_raw;	//0x58
	volatile UINT32 	IRQstatus;		//0x5c
	volatile UINT32 	IRQEnable_set;	//0x60
	volatile UINT32 	IRQEnable_clear;	//0x64
	volatile UINT32 	IRQEoi_vector;	//0x68
	volatile UINT32 	clkc_enable;		//0x6c
	volatile UINT32 	clkc_reset;		//0x70
} LCDC_REGS;


struct lcdc {
	LCDC_REGS				*regs;     // virtual address of regs
	UINT32						phys_base; // phy address of regs
	UINT32						fb_pa;     // phy address of fb
	UINT32						*fb_va;    // phy address of fb
	UINT32						fb_cur_win_size; // size of the FB window for current mode

	struct lcd_panel       *panel;
	UINT32					    clk;
	void						*palette_virt;
	UINT32						palette_phys;
	int							palette_code;
	int							palette_size;
	UINT32						irq_mask;
	UINT32						reset_count;
	enum fb_update_mode	update_mode;
	enum lcdc_load_mode	load_mode;

	HANDLE						last_frame_complete;
	HANDLE						palette_load_complete;
	HANDLE						lcd_int_event;
	UINT32						lcdc_irq;
	DWORD						sys_interrupt;
	HANDLE						lcd_int_thread;

       int 							dma_burst_sz;
       int							dma_fifo_thresh;

	int 							currentBLLevel;
	int 							backlightOn;

	enum fb_pack_format          	pack_format;
	
	int							color_mode; //TODO ?????????????? move out of here
};

int lcdc_change_mode(struct lcdc* lcdc, int color_mode);

int lcdc_setcolreg(struct lcdc* lcdc, UINT16 regno, 
		UINT8 red, UINT8 green, UINT8 blue, int update_hw_pal);

int lcdc_set_update_mode(struct lcdc* lcdc,
		enum fb_update_mode mode);

enum fb_update_mode
lcdc_get_update_mode(struct lcdc* lcdc);

struct lcd_panel *get_panel(void);

BOOL LcdPdd_LCD_GetMode(
    DWORD   *pPixelFormat,
    DWORD   *pWidth,
    DWORD   *pHeight,
    DWORD   *pPixelClock
    );

BOOL LcdPdd_DVI_Enabled(void);
BOOL LcdPdd_LCD_Initialize(struct lcdc *lcdc);
BOOL LcdPdd_GetMemory(
    DWORD   *pVideoMemLen,
    DWORD   *pVideoMemAddr
    );
void LcdPdd_Handle_LostofSync_int(struct lcdc *lcdc);
void LcdPdd_Handle_EndofPalette_int(struct lcdc *lcdc);
BOOL LcdPdd_SetPowerLevel(
    struct lcdc *lcdc,
    DWORD   dwPowerLevel
    );

VOID LcdPdd_EnableLCDC(
    struct lcdc *lcdc,
    BOOL bEnable
    );

DWORD LcdPdd_Get_PixClkDiv(void);

BOOL SetBacklightLevel_GPIO(UINT level);

#ifdef __cplusplus
}
#endif

#endif
