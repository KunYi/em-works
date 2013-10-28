/*
 *   Copyright (c) Texas Instruments Incorporated 2009. All Rights Reserved.
 *
 *   Use of this software is controlled by the terms and conditions found 
 *   in the license agreement under which this software has been supplied.
 * 
 *   lcdc.c
 */

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <oal.h>
#include <oalex.h>
#include <nkintr.h>
#include "lcdc.h"
#include "am33x_clocks.h"
#include "ceddkex.h"
#include "Image_Cfg.h"
#include <sdk_i2c.h>
#include "bsp_def.h"
#include "sdk_gpio.h"

//
//  Defines
//
static BOOL bDVIEnabled = FALSE;

static void udelay(UINT32 delay)
{
	volatile UINT32 tmp;
	volatile UINT32 j;
	UINT32 i;
	
	for(i=0; i<delay; i++)
		for(j=0; j<10000; j++)
			tmp = j;
}

//------------------------------------------------------------------------------
DWORD PixelFormatToPixelSize( DWORD  ePixelFormat)
{
    DWORD   dwResult = 1;
    
    //  Convert pixel format into bytes per pixel
    switch( ePixelFormat )
    {
        case DISPC_PIXELFORMAT_RGB16:
        case DISPC_PIXELFORMAT_ARGB16:
        case DISPC_PIXELFORMAT_YUV2:
        case DISPC_PIXELFORMAT_UYVY:
            //  2 bytes per pixel
            dwResult = 2;
            break;

        case DISPC_PIXELFORMAT_RGB24:
            //  2 bytes per pixel
            dwResult = 3;
            break;

        case DISPC_PIXELFORMAT_RGB32:
        case DISPC_PIXELFORMAT_ARGB32:
        case DISPC_PIXELFORMAT_RGBA32:
            //  4 bytes per pixel
            dwResult = 4;
            break;
    }

    //  Return result
    return dwResult;
}

/* Enable the Raster Engine of the LCD Controller */
static __inline void lcdc_enable_raster(struct lcdc *lcdc)
{
	lcdc->regs->raster_control |= LCDC_CTRL_LCD_EN;
}

/* Disable the Raster Engine of the LCD Controller */
static __inline void lcdc_disable_raster(struct lcdc *lcdc)
{
	lcdc->regs->raster_control &= ~LCDC_CTRL_LCD_EN;
}

static void lcdc_blit(struct lcdc *lcdc, unsigned int load_mode)
{
	UINT32 start, end;
	UINT32 reg_ras, reg_dma, reg_int;

	/* init reg to clear PLM (loading mode) fields */
	reg_ras = lcdc->regs->raster_control;
	reg_ras &= ~(3 << 20);

	reg_dma  = lcdc->regs->dma_control;

	/* always enable underflow interrupt */
	reg_int = lcdc->regs->IRQEnable_set | LCDC_UNDERFLOW_INT_ENA; 

	if (load_mode == LCDC_LOAD_FRAME) {
		start = lcdc->fb_pa/* + 0x40000*/;
		end = start + lcdc->fb_cur_win_size -1;

	 	reg_int |= LCDC_END_OF_FRAME0_INT_ENA | LCDC_END_OF_FRAME1_INT_ENA;
		reg_dma |= LCDC_DUAL_FRAME_BUFFER_ENABLE;

              lcdc->regs->fb0_base = start;
              lcdc->regs->fb0_ceiling = end;
              lcdc->regs->fb1_base = start;
              lcdc->regs->fb1_ceiling = end;
			  
	} else if (load_mode == LCDC_LOAD_PALETTE) {
		start = lcdc->palette_phys;
		end = start + lcdc->palette_size -1;

	 	reg_int |= LCDC_PALETTE_INT_ENA;

              lcdc->regs->fb0_base = start;
              lcdc->regs->fb0_ceiling = end;
	}
	reg_ras |= load_mode <<20;
       lcdc->regs->IRQEnable_set = reg_int;
	lcdc->regs->dma_control = reg_dma;
	lcdc->regs->raster_control = reg_ras;

	/*
	 * The Raster enable bit must be set after all other control fields are
	 * set.
	 */
	lcdc_enable_raster(lcdc);
}

/* Configure the Burst Size and fifo threhold of DMA */
BOOL lcdc_cfg_dma(struct lcdc *lcdc)
{
	UINT32 tmp;

	lcdc->dma_fifo_thresh = 6; 
       tmp = lcdc->regs->dma_control & 0x00000001;
	switch (lcdc->dma_burst_sz) {
	case 1:
		tmp |= LCDC_DMA_BURST_SIZE(LCDC_DMA_BURST_1);
		break;
	case 2:
		tmp |= LCDC_DMA_BURST_SIZE(LCDC_DMA_BURST_2);
		break;
	case 4:
		tmp |= LCDC_DMA_BURST_SIZE(LCDC_DMA_BURST_4);
		break;
	case 8:
		tmp |= LCDC_DMA_BURST_SIZE(LCDC_DMA_BURST_8);
		break;
	case 16:
		tmp |= LCDC_DMA_BURST_SIZE(LCDC_DMA_BURST_16);
		break;
	default:
		return FALSE;
	}

	tmp |= (lcdc->dma_fifo_thresh << 8);

	lcdc->regs->dma_control = tmp;
	return TRUE;
}

static void lcdc_cfg_ac_bias(struct lcdc *lcdc, int period, int transitions_per_int)
{
	UINT32 tmp;

	/* Set the AC Bias Period and Number of Transisitons per Interrupt */
	
	tmp = lcdc->regs->timing2 & 0xFFF00000;
	tmp |= LCDC_AC_BIAS_FREQUENCY(period) |
		LCDC_AC_BIAS_TRANSITIONS_PER_INT(transitions_per_int);
	lcdc->regs->timing2 = tmp;
}

static void lcdc_cfg_horizontal_sync(struct lcdc *lcdc)
{
	UINT32 tmp;

	tmp = lcdc->regs->timing0 & 0x3F;
	tmp |= ((lcdc->panel->hbp & 0xff) << 24) |
		     ((lcdc->panel->hfp & 0xff) << 16) |
		     ((lcdc->panel->hsw & 0x3f) << 10) ;
	lcdc->regs->timing0 = tmp;
}

static void lcdc_cfg_vertical_sync(struct lcdc *lcdc)
{
	UINT32 tmp;

	tmp = lcdc->regs->timing1 & 0x3FF;
	tmp |= ((lcdc->panel->vbp & 0xff) << 24) |
		     ((lcdc->panel->vfp & 0xff) << 16) |
		     ((lcdc->panel->vsw & 0x3f) << 10) ;
	lcdc->regs->timing1 = tmp;
}

static void lcdc_cfg_display(struct lcdc *lcdc)
{
	UINT32 tmp;

       tmp = lcdc->regs->raster_control ;
	tmp &= ~(LCDC_CTRL_LCD_TFT | LCDC_CTRL_MONO_8BIT_MODE| LCDC_CTRL_MONOCHROME_MODE |LCDC_CTRL_LCD_TFT_24 |
		     LCDC_CTRL_TFT_24BPP_UNPACK| LCDC_CTRL_LCD_STN_565 | LCDC_CTRL_TFT_ALT_ENABLE);
	
       if(lcdc->panel->bpp == DISPC_PIXELFORMAT_RGB24 ||
	   lcdc->panel->bpp == DISPC_PIXELFORMAT_RGB32 ||
	   lcdc->panel->bpp == DISPC_PIXELFORMAT_ARGB32 ||
	   lcdc->panel->bpp == DISPC_PIXELFORMAT_RGBA32) 
	   	tmp |= LCDC_CTRL_LCD_TFT_24; 
	   
	if(lcdc->panel->bpp == DISPC_PIXELFORMAT_RGB32 ||
	   lcdc->panel->bpp == DISPC_PIXELFORMAT_ARGB32 ||
	   lcdc->panel->bpp == DISPC_PIXELFORMAT_RGBA32)
		tmp |= LCDC_CTRL_TFT_24BPP_UNPACK;
	   
	switch (lcdc->panel->panel_shade) {
        	case MONOCHROME:
        		tmp |= LCDC_CTRL_MONOCHROME_MODE;
        		if (lcdc->panel->mono_8bit_mode)
        			tmp |= LCDC_CTRL_MONO_8BIT_MODE;
        		break;
        	case COLOR_ACTIVE:
        		tmp |= LCDC_CTRL_LCD_TFT;
        		if (lcdc->panel->tft_alt_mode)
        			tmp |= LCDC_CTRL_TFT_ALT_ENABLE;
        		break;
        
        	case COLOR_PASSIVE:
        		if (lcdc->color_mode == DISPC_PIXELFORMAT_RGB16)
        			tmp |= LCDC_CTRL_LCD_STN_565;
        		break;
        
        	default:
        		return ;
	}

       /* reqdly = 0x80 lcd_clk */
	tmp |= 0x80 << 12;
	lcdc->regs->raster_control = tmp;

       tmp = lcdc->regs->timing2;
	tmp &= ~(LCDC_SIGNAL_MASK << 20);
	tmp |= ((lcdc->panel->config & LCDC_SIGNAL_MASK) << 20);
	lcdc->regs->timing2 = tmp;

}

static void lcdc_cfg_frame_buffer(struct lcdc *lcdc)
{
	UINT32 tmp;
       UINT32  width, height;
	   
	/* Set the Panel Width */
	/* Pixels per line = (PPL + 1)*16 */
	/*
	 * 0x7F in bits 4..10 gives max horizontal resolution = 2048
	 * pixels.
	 */

       tmp = lcdc->regs->timing0 & 0xfffffc00; 
	width = lcdc->panel->x_res & 0x7f0;
	width = (width >> 4) - 1;
	tmp |= ((width & 0x3f) << 4) | ((width & 0x40) >> 3); 
	lcdc->regs->timing0 = tmp;
	
	/* Set the Panel Height */
	/* Set bits 9:0 of Lines Per Pixel */
       tmp = lcdc->regs->timing1 & 0xfffffc00; 
	height = lcdc->panel->y_res;
	
	tmp |= ((height - 1) & 0x3ff) ;
	lcdc->regs->timing1 = tmp;
	
	/* Set bit 10 of Lines Per Pixel */
       tmp = lcdc->regs->timing2 ; 
	tmp |= ((height - 1) & 0x400) << 16;    //bit 26 in timing2 register
	lcdc->regs->timing2 = tmp;
	
	/* Set the Raster Order of the Frame Buffer */
       tmp = lcdc->regs->raster_control ;
	tmp |= (LCDC_CTRL_RASTER_ORDER | LCDC_LOAD_FRAME<<20);
	lcdc->regs->raster_control = tmp;
	
	return;
}

static void lcdc_config_clk_divider(struct lcdc* lcdc)
{
	UINT32 lcd_clk, div;
       UINT32 tmp;

	lcd_clk = lcdc->clk ; //in HZ
	div = lcd_clk / lcdc->panel->pixel_clock ;  //Tao: diagnostics code set it to 0x2
     
	/* Configure the LCD clock divisor. */
	tmp = lcdc->regs->control;
	tmp &= ~(CTRL_DIV_MASK);
	tmp |= div << 8 | CTRL_RASTER_MODE;
//	udelay(10);
	lcdc->regs->control = tmp;

	lcdc->regs->clkc_enable = LCDC_DMA_CLK_EN | LCDC_LIDD_CLK_EN |
				          LCDC_CORE_CLK_EN;

}

void lcdc_setup_regs(struct lcdc* lcdc) {

	lcdc_disable_raster(lcdc);

       lcdc_config_clk_divider(lcdc);
	lcdc_cfg_display(lcdc);

	/* Configure the AC bias properties. */
	//TODO: HOW TO DECIDE AC BIAS SETTINGS? data taken from Linux code
	lcdc_cfg_ac_bias(lcdc, 0xff, 0x0);

	/* Configure the vertical and horizontal sync properties. */
	lcdc_cfg_vertical_sync(lcdc);
	lcdc_cfg_horizontal_sync(lcdc);
	lcdc_cfg_frame_buffer(lcdc);	
	lcdc_cfg_dma(lcdc);
}

static __inline void lcdc_hw_reset(struct lcdc *lcdc) 
{
	lcdc->regs->raster_control  = 0;

	/* take module out of reset */
	lcdc->regs->clkc_reset = 0xC;
	udelay(5);
	lcdc->regs->clkc_reset = 0x0;
	
}
static __inline void lcdc_enable(struct lcdc *lcdc) {
	lcdc_hw_reset(lcdc);
	lcdc_setup_regs(lcdc);	
	lcdc_blit	(lcdc, lcdc->load_mode);
}

static __inline void lcdc_disable_async(struct lcdc *lcdc) {

	lcdc_disable_raster(lcdc); 

   	lcdc->regs->dma_control = 0x0;
	lcdc->regs->IRQEnable_set = 0x0;
}

static void lcdc_dumpregs(struct lcdc *lcdc) {
	RETAILMSG(1,(TEXT("LCD Controller Register Dump\r\n")));
	RETAILMSG(1,(TEXT("Revision ............... %08x\r\n"),lcdc->regs->revision));
	RETAILMSG(1,(TEXT("Control ................ %08x\r\n"),lcdc->regs->control));
	RETAILMSG(1,(TEXT("Status ................. %08x\r\n"),lcdc->regs->status));
	RETAILMSG(1,(TEXT("Raster Control ......... %08x\r\n"),lcdc->regs->raster_control));
	RETAILMSG(1,(TEXT("Raster Timing 0 ........ %08x\r\n"),lcdc->regs->timing0));
	RETAILMSG(1,(TEXT("Raster Timing 1 ........ %08x\r\n"),lcdc->regs->timing1)); 
	RETAILMSG(1,(TEXT("Raster Timing 2 ........ %08x\r\n"),lcdc->regs->timing2)); 
	RETAILMSG(1,(TEXT("Subpanel Display 1 ..... %08x\r\n"),lcdc->regs->subpanel1));
	RETAILMSG(1,(TEXT("Subpanel Display 2 ..... %08x\r\n"),lcdc->regs->subpanel2));
	RETAILMSG(1,(TEXT("DMA Control ............ %08x\r\n"),lcdc->regs->dma_control));
	RETAILMSG(1,(TEXT("Frame Buffer 0 Base .... %08x\r\n"),lcdc->regs->fb0_base));
	RETAILMSG(1,(TEXT("Frame Buffer 0 Ceiling . %08x\r\n"),lcdc->regs->fb0_ceiling));
	RETAILMSG(1,(TEXT("Frame Buffer 1 Base .... %08x\r\n"),lcdc->regs->fb1_base));
	RETAILMSG(1,(TEXT("Frame Buffer 1 Ceiling . %08x\r\n"),lcdc->regs->fb1_ceiling));
}


static void lcdc_reset(struct lcdc *lcdc, UINT32 status) {
	lcdc_disable_async(lcdc);
	lcdc->reset_count++;

	if (lcdc->reset_count == 1) {
		ERRORMSG(1,(TEXT( "resetting (status %x,reset count %lu)\r\n"),
			              status, lcdc->reset_count));
		lcdc_dumpregs(lcdc);
	}

	if (lcdc->reset_count < 100) {
		lcdc_enable_raster(lcdc);
	} else {
		lcdc->reset_count = 0;
		ERRORMSG(1,(TEXT("too many reset attempts, giving up.\r\n")));
		lcdc_dumpregs(lcdc);
	}
}

#define LCD_UPDN_GPIO_ON    (0x1<<0)   //GPIO4
#define LCD_SHLR_GPIO_ON     (0x1<<6)   //GPIO7 
#define TLC59108_MODE1   0x00
#define TLC59108_PWM2    0x04
#define TLC59108_LEDOUT0 0x0c
#define TLC59108_LEDOUT1 0x0d
#define TLC59108_MAX_BRIGHTNESS 0xFF

BOOL lcdc_Tlc59108_init(UINT level, BOOL up, BOOL left)
{
    DWORD rc = TRUE;
    UINT8 val, val_read;
    void* m_hI2CDevice=NULL;
	
    if(!m_hI2CDevice)
    {
    	
	m_hI2CDevice = I2COpen(AM_DEVICE_I2C0); //I2C0
	if (m_hI2CDevice == NULL)
	{
		RETAILMSG(1, (L"AM33x:Failed to open I2C driver"));
		return FALSE;
	}
	I2CSetSlaveAddress(m_hI2CDevice, (UINT16)0x40);
    }
	
    if (level )
    {
        // through TLC59108 
        val = 0x0; 
        I2CWrite(m_hI2CDevice, TLC59108_MODE1, &val, sizeof(val));
        I2CRead(m_hI2CDevice, TLC59108_MODE1, &val_read, sizeof(val));	
		
        val = 0x21; 
        I2CWrite(m_hI2CDevice, TLC59108_LEDOUT0, &val, sizeof(val));
        I2CRead(m_hI2CDevice, TLC59108_LEDOUT0, &val_read, sizeof(val));	

        val = 0;
	 if(up == TRUE) val |= LCD_UPDN_GPIO_ON;
	 if(left == TRUE) val |= LCD_SHLR_GPIO_ON;
        I2CWrite(m_hI2CDevice, TLC59108_LEDOUT1, &val, sizeof(val));
        I2CRead(m_hI2CDevice, TLC59108_LEDOUT1, &val_read, sizeof(val));	
		
        val = level & 0xff; 
        I2CWrite(m_hI2CDevice, TLC59108_PWM2, &val, sizeof(val));
        I2CRead(m_hI2CDevice, TLC59108_PWM2, &val_read, sizeof(val));	
		
    }
    else if (level == 0 )
    {
        // through TLC59108 
        val = 0x01; 
        I2CWrite(m_hI2CDevice, TLC59108_LEDOUT0, &val, sizeof(val));
    }
	
    if(m_hI2CDevice)
	I2CClose(m_hI2CDevice);
	
    return rc;
}

int lcdc_change_mode(struct lcdc* lcdc, int color_mode) {

	switch (color_mode) {
		case DISPC_PIXELFORMAT_BITMAP1:
			lcdc->palette_code = 0x0000;
			lcdc->palette_size = 32;
			break;
		case DISPC_PIXELFORMAT_BITMAP2:
			lcdc->palette_code = 0x1000;
			lcdc->palette_size = 32;
			break;
		case DISPC_PIXELFORMAT_BITMAP4:
			lcdc->palette_code = 0x2000;
			lcdc->palette_size = 32;
			break;
		case DISPC_PIXELFORMAT_BITMAP8:
			lcdc->palette_code = 0x3000;
			lcdc->palette_size = 512;
			break;
		case DISPC_PIXELFORMAT_RGB16:
			lcdc->palette_code = 0x4000;
			lcdc->palette_size = 32;
			break;
		case DISPC_PIXELFORMAT_RGB12:
			lcdc->palette_code = 0x4000;
			lcdc->palette_size = 32;
			break;
		case DISPC_PIXELFORMAT_RGB24:
		case DISPC_PIXELFORMAT_RGB32:
		case DISPC_PIXELFORMAT_ARGB32:
		case DISPC_PIXELFORMAT_RGBA32:
			lcdc->palette_code = 0x4000;
			lcdc->palette_size = 0;                 
			break;
		default:
			ERRORMSG(1,(TEXT("invalid color mode %d\n"),color_mode));
			return -1;
	}

	if (lcdc->update_mode == FB_AUTO_UPDATE) {
		lcdc_disable_raster(lcdc);
		lcdc_setup_regs(lcdc);
		lcdc_blit(lcdc, LCDC_LOAD_FRAME);
		lcdc_enable_raster(lcdc);
	}
	return 0;
}

static void lcdc_load_palette(struct lcdc* lcdc)
{
	UINT16	*palette;

	if (!lcdc->palette_size) return;

	palette = (UINT16 *)lcdc->palette_virt;

	*(UINT16 *)palette &= 0x0fff;
	*(UINT16 *)palette |= lcdc->palette_code;

	lcdc->load_mode = LCDC_LOAD_PALETTE;
	lcdc_blit(lcdc,LCDC_LOAD_PALETTE);
#ifdef DEVICE
	if (WaitForSingleObject(lcdc->palette_load_complete, 500) != WAIT_OBJECT_0)
		ERRORMSG(1,(TEXT("lcdc_load_palette: timeout waiting for palette load\r\n")));
#endif	
}

int lcdc_setcolreg(struct lcdc* lcdc, UINT16 regno, 
		UINT8 red, UINT8 green, UINT8 blue, int update_hw_pal) {
	UINT16 *palette;

	if (!lcdc->palette_size)
		return 0;

	if (regno > 255)
		return -1;

	palette = (UINT16 *)lcdc->palette_virt;

      /* TAO, handles 24 bit only ? */
	palette[regno] &= ~0x0fff;
	palette[regno] |= ((red >> 4) << 8) | ((green >> 4) << 4 ) |
			   (blue >> 4);

	if (update_hw_pal) {
		lcdc_disable_raster(lcdc);
		/* at this moment, is dma/display etc configured properly? */
		lcdc_load_palette(lcdc);
		lcdc->load_mode = LCDC_LOAD_FRAME;
		lcdc_blit(lcdc, LCDC_LOAD_FRAME);
	}

	return 0;
}

int lcdc_set_update_mode(struct lcdc* lcdc,
		enum fb_update_mode mode)
{
	if (mode != lcdc->update_mode) {
		switch (mode) {
		case FB_AUTO_UPDATE:
			lcdc_setup_regs(lcdc);
			lcdc_load_palette(lcdc);
			lcdc->load_mode = LCDC_LOAD_FRAME;
			lcdc_blit(lcdc, LCDC_LOAD_FRAME);
			lcdc->update_mode = mode;
			break;
		case FB_UPDATE_DISABLED:
			lcdc_disable_async(lcdc);
			lcdc->update_mode = mode;
			break;
		default:
			return -1;
		}
	}

	return 0;
}

enum fb_update_mode lcdc_get_update_mode(struct lcdc* lcdc) {
	return lcdc->update_mode;
}

#if 0 
void lcdc_suspend(struct lcdc* lcdc) {
	if (lcdc->update_mode == FB_AUTO_UPDATE)
		lcdc_disable(lcdc);

	EnableDeviceClocks(AM_DEVICE_LCDC, FALSE);

}

void lcdc_resume(struct lcdc* lcdc) {
	
	EnableDeviceClocks(AM_DEVICE_LCDC, TRUE);
	lcdc_hw_reset(lcdc);
	if (lcdc->update_mode == FB_AUTO_UPDATE) {
		lcdc_setup_regs(lcdc);
		lcdc_load_palette(lcdc);
		lcdc->load_mode = LCDC_LOAD_FRAME;
		lcdc_blit(lcdc, LCDC_LOAD_FRAME);
	}
}
#endif

BOOL SetBacklightLevel_GPIO(UINT level)
{
    DWORD rc = TRUE;
    HANDLE g_hGpio = NULL;
	
    if(!g_hGpio)
    {
       // Configure Backlight/Power pins as outputs
       g_hGpio = GPIOOpen();
 	
	if (g_hGpio == NULL)
	{
		RETAILMSG(1, (L"AM33x:Failed to open GPIO driver"));
		return FALSE;
	}
    }

    /* turn on backlight non-zero level */	
    if (level)
    {
        GPIOSetBit(g_hGpio, BSP_LCD_BACKLIGHT_GPIO);
        GPIOSetMode(g_hGpio, BSP_LCD_BACKLIGHT_GPIO, GPIO_DIR_OUTPUT );
		
    }
    else if (level == 0 )
    {
        GPIOClrBit(g_hGpio, BSP_LCD_BACKLIGHT_GPIO);
    }
	
    if(g_hGpio)
	GPIOClose(g_hGpio);
	
    return rc;
}

//------------------------------------------------------------------------------
BOOL
LcdPdd_LCD_GetMode(
    DWORD   *pPixelFormat,
    DWORD   *pWidth,
    DWORD   *pHeight,
    DWORD   *pPixelClock
    )
{
    struct lcd_panel       *panel;

    panel = get_panel();

    if( pPixelFormat )
        *pPixelFormat = panel->bpp;
	
    if( pWidth )
        *pWidth = panel->x_res;

    if( pHeight )
        *pHeight = panel->y_res;

//    if(pPixelClock)
//        *pPixelClock = lcdc->panel->pixel_clock;
		
    return TRUE;
}


//------------------------------------------------------------------------------
BOOL
LcdPdd_DVI_Enabled(void)
{
    return bDVIEnabled;
}

//------------------------------------------------------------------------------
BOOL
LcdPdd_LCD_Initialize(struct lcdc *lcdc)
{

    DWORD   dwLength;  

    lcdc->panel			= get_panel();

    // Compute the size
    dwLength = PixelFormatToPixelSize(lcdc->panel->bpp) * 
                      lcdc->panel->x_res *  
                      lcdc->panel->y_res;
	
    lcdc->fb_pa = IMAGE_WINCE_DISPLAY_PA; 
    lcdc->regs->raster_control = 0;
    lcdc_hw_reset(lcdc);
    lcdc->regs->dma_control = 0;
    lcdc->regs->fb0_base = IMAGE_WINCE_DISPLAY_PA;
    lcdc->regs->fb0_ceiling = IMAGE_WINCE_DISPLAY_PA + dwLength -1; 
    
    /* disable interrupt */
    lcdc->regs->IRQEnable_clear = 0x3FF;

    lcdc->clk *= 1000000;  /* change from MHz to Hz */
    lcdc->dma_burst_sz = 16;	
    lcdc_setup_regs(lcdc);
    lcdc_cfg_dma(lcdc);	

	SetBacklightLevel_GPIO(0xff);
#ifdef DEVICE
    lcdc_Tlc59108_init(0xff, TRUE, FALSE);
#endif
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL
LcdPdd_GetMemory(
    DWORD   *pVideoMemLen,
    DWORD   *pVideoMemAddr
    )
{
    //  Return video memory parameters
    if( pVideoMemLen )
        *pVideoMemLen = IMAGE_WINCE_DISPLAY_SIZE;

    if( pVideoMemAddr )
        *pVideoMemAddr = IMAGE_WINCE_DISPLAY_PA;

    return TRUE;
}

void LcdPdd_Handle_LostofSync_int(struct lcdc *lcdc)
{
    DWORD status = 0;

    status = lcdc->regs->IRQstatus;

    lcdc_disable_raster(lcdc);
    /* clear interrupt*/
    lcdc->regs->IRQstatus = status; 
    //lcdc_reset(lcdc, status);  // leo implementation
#ifdef DEVICE

#endif    
    lcdc_enable_raster(lcdc);

}

void LcdPdd_Handle_EndofPalette_int(struct lcdc *lcdc)
{
    DWORD status = 0;

    status = lcdc->regs->IRQstatus;

    /* Must disable raster before changing state of any control bit.
    * And also must be disabled before clearing the PL loading
    * interrupt via the following write to the status register. If
    * this is done after then one gets multiple PL done interrupts.
    */
    
    lcdc_disable_raster(lcdc);
    
    /* clear interrupt*/
    lcdc->regs->IRQstatus = status; 
    
    /* Disable PL completion inerrupt */
    lcdc->regs->IRQEnable_clear |= LCDC_PALETTE_INT_ENA;
    
    /* Setup and start data loading mode */
    lcdc_blit(lcdc, LCDC_LOAD_FRAME);

}

//------------------------------------------------------------------------------
BOOL
LcdPdd_SetPowerLevel(
    struct lcdc *lcdc,
    DWORD   dwPowerLevel
    )
{
        
    // Power display up/down
    switch(dwPowerLevel)
    {
        case D0:
        case D1:
        case D2:
          lcdc_enable_raster(lcdc);
//          lcdc_dumpregs(lcdc);
          break;        

        case D3:
        case D4:
//            lcdc_dumpregs(lcdc);
            lcdc_disable_raster(lcdc);
            break;
    }
        
    return TRUE;
}   

//------------------------------------------------------------------------------
DWORD
LcdPdd_Get_PixClkDiv(void)
{
    return 0;
}

//------------------------------------------------------------------------------
VOID LcdPdd_EnableLCDC(
    struct lcdc *lcdc,
    BOOL bEnable
    )
{
    if(bEnable) 
        lcdc_enable_raster(lcdc);
    else 
        lcdc_disable_raster(lcdc);
}
