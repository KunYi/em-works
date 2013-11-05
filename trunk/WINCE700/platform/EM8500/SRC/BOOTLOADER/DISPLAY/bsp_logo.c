// All rights reserved ADENEO EMBEDDED 2010
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  bsp_logo.c
//


//------------------------------------------------------------------------------
//
// includes
//

#include "bsp.h"
#include "bsp_logo.h"
#include "oalex.h"
#include "eboot.h"
#include "lcdc.h"
#include "am33x_clocks.h"
#include "ceddkex.h"
#include "Image_Cfg.h"
#include <sdk_i2c.h>
#include "soc_cfg.h" 
#include <oal_clock.h>

//------------------------------------------------------------------------------
//
// prototypes
//
void reset_display_controller( void );
UINT32 enable_lcd_power( void );
void lcd_config(UINT32 framebuffer);
void lcd_shutdown(void);
UINT32 disable_lcd_power(void);

//------------------------------------------------------------------------------
//
// defines
//
#define LOGO_WIDTH                  800    // Logo bitmap image is RGB24 VGA Portrait bitmap
#define LOGO_HEIGHT                 480
#define BYTES_PER_PIXEL           	4
#define DELAY_COUNT                 100 

DWORD   g_dwLogoPosX;
DWORD   g_dwLogoPosY;

DWORD   g_dwLogoWidth;
DWORD   g_dwLogoHeight;

DWORD   g_dwPixelFormat;

struct lcdc lcdc_device;

//------------------------------------------------------------------------------
//
//  Function:  ShowLogo
//
//  This function shows the logo splash screen
//
VOID ShowLogo(UINT32 flashAddr, UINT32 offset)
{
    HANDLE  hFlash = NULL;
    DWORD  framebuffer;
    DWORD  framebufferPA;
    PUCHAR  pChar;
    ULONG   x, y;
    WORD    wSignature = 0;
    DWORD   dwOffset = 0;
    DWORD   dwLcdWidth,  dwLcdHeight;
    DWORD dwLength = 0;


    //  Get the LCD width and height
    LcdPdd_LCD_GetMode( &g_dwPixelFormat, &dwLcdWidth, &dwLcdHeight, NULL );

    dwLength = BYTES_PER_PIXEL * LOGO_WIDTH * LOGO_HEIGHT;
	
    //  Get the video memory
    framebufferPA = IMAGE_WINCE_DISPLAY_PA;
    framebuffer = (DWORD)OALPAtoUA(framebufferPA);
    pChar = (PUCHAR)framebuffer;
    
    if (flashAddr != -1)
    {
        // Open flash storage
        hFlash = OALFlashStoreOpen(flashAddr);
        if( hFlash != NULL )
        {
            //  The LOGO reserved NAND flash region contains the BMP file
            OALFlashStoreBufferedRead( hFlash, offset, (UCHAR*) &wSignature, sizeof(wSignature), FALSE );

            //  Check for 'BM' signature
            if( wSignature == 0x4D42 )  
            {
                //  Read the offset to the pixel data
                OALFlashStoreBufferedRead( hFlash, offset + 10, (UCHAR*) &dwOffset, sizeof(dwOffset), FALSE );

                //  Read the pixel data with the given offset
                OALFlashStoreBufferedRead( hFlash, offset + dwOffset, pChar, dwLength, FALSE );
            }
           
            //  Close store
            OALFlashStoreClose(hFlash);
        
            //  Compute position and size of logo image 
            g_dwLogoPosX   = (dwLcdWidth - LOGO_WIDTH)/2;
            g_dwLogoPosY   = (dwLcdHeight - LOGO_HEIGHT)/2;
            g_dwLogoWidth  = LOGO_WIDTH;
            g_dwLogoHeight = LOGO_HEIGHT;
            
            //As BMP are stored upside down, we need to flip the frame buffer's content
            //FlipFrameBuffer((PUCHAR)framebuffer,LOGO_HEIGHT,LOGO_WIDTH*BYTES_PER_PIXEL,(PUCHAR)framebuffer + dwLength);
        }
    }

    //  If bitmap signature is valid, display the logo, otherwise fill screen with pattern
    if( wSignature != 0x4D42 )
    {
        //  Adjust color bars to LCD size
        g_dwLogoPosX   = 0;
        g_dwLogoPosY   = 0;
        g_dwLogoWidth  = dwLcdWidth;
        g_dwLogoHeight = dwLcdHeight;
        
        for (y= 0; y < dwLcdHeight; y++)
        {
            for( x = 0; x < dwLcdWidth; x++ )
            {
                if( y < dwLcdHeight/2 )
                {
                    if( x < dwLcdWidth/2 )
                    {
                        *pChar++ = 0x00;    //  Blue
                        *pChar++ = 0x00;    //  Green
                        *pChar++ = 0xFF;    //  Red
                        if(g_dwPixelFormat == DISPC_PIXELFORMAT_RGB32 ||
							g_dwPixelFormat == 	DISPC_PIXELFORMAT_ARGB32 ||
							g_dwPixelFormat == DISPC_PIXELFORMAT_RGBA32)
                            *pChar++ = 0x00;    //  
							
                    }
                    else
                    {
                        *pChar++ = 0x00;    //  Blue
                        *pChar++ = 0xFF;    //  Green
                        *pChar++ = 0x00;    //  Red
                        if(g_dwPixelFormat == DISPC_PIXELFORMAT_RGB32 ||
							g_dwPixelFormat == 	DISPC_PIXELFORMAT_ARGB32 ||
							g_dwPixelFormat == DISPC_PIXELFORMAT_RGBA32)
                            *pChar++ = 0x00;    //  
                        
                    }
                }
                else
                {
                    if( x < dwLcdWidth/2 )
                    {
                        *pChar++ = 0xFF;    //  Blue
                        *pChar++ = 0x00;    //  Green
                        *pChar++ = 0x00;    //  Red
                        if(g_dwPixelFormat == DISPC_PIXELFORMAT_RGB32 ||
							g_dwPixelFormat == 	DISPC_PIXELFORMAT_ARGB32 ||
							g_dwPixelFormat == DISPC_PIXELFORMAT_RGBA32)
                            *pChar++ = 0x00;    //  
                        
                    }
                    else
                    {
                        *pChar++ = 0x00;    //  Blue
                        *pChar++ = 0xFF;    //  Green
                        *pChar++ = 0xFF;    //  Red
                        if(g_dwPixelFormat == DISPC_PIXELFORMAT_RGB32 ||
							g_dwPixelFormat == 	DISPC_PIXELFORMAT_ARGB32 ||
							g_dwPixelFormat == DISPC_PIXELFORMAT_RGBA32)
                            *pChar++ = 0x00;    //  
                        
                    }
                }
            }
        }
    }

    //  Fire up the LCD
    lcd_config(framebufferPA);       
}

//------------------------------------------------------------------------------
//
//  Function:   ShowSDLogo
//
//  This function is called to display the splaschreen bitmap from the SDCard
//
//
BOOL ShowSDLogo()
{
    DWORD framebuffer = 0;
    DWORD  framebufferPA = 0;
    DWORD dwLcdWidth = 0;
    DWORD dwLcdHeight = 0;
    DWORD dwLength = 0;

    // Get the LCD width and height
    LcdPdd_LCD_GetMode( &g_dwPixelFormat, &dwLcdWidth, &dwLcdHeight, NULL );

	// Get the frame buffer
    framebufferPA = IMAGE_WINCE_DISPLAY_PA;
    framebuffer = (DWORD) OALPAtoUA(framebufferPA);

    // Compute the size
    dwLength = BYTES_PER_PIXEL * LOGO_WIDTH * LOGO_HEIGHT;
    
    if (!BLSDCardReadLogo(L"Logo.bmp", (UCHAR*)framebuffer, dwLength))
    {
    	return FALSE;
    }

    //  Compute position and size of logo image 
    g_dwLogoPosX   = (dwLcdWidth - LOGO_WIDTH)/2;
    g_dwLogoPosY   = (dwLcdHeight - LOGO_HEIGHT)/2;
    g_dwLogoWidth  = LOGO_WIDTH;
    g_dwLogoHeight = LOGO_HEIGHT;

    //As BMP are stored upside down, we need to flip the frame buffer's content
    //FlipFrameBuffer((PUCHAR)framebuffer,LOGO_HEIGHT,LOGO_WIDTH*BYTES_PER_PIXEL,(PUCHAR)framebuffer + dwLength);

    //  Fire up the LCD
    lcd_config(framebufferPA);

	return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  HideLogo
//
//  This function hides the logo splash screen
//
VOID HideLogo(VOID)
{
    lcd_shutdown();
}

//------------------------------------------------------------------------------
//
//  Function:  reset_display_controller
//
//  This function resets the Display Sub System on omap24xx
//
void reset_display_controller( void )
{
    
}

//------------------------------------------------------------------------------
//
//  Function:  enable_lcd_power
//
//  This function enables the power for the LCD controller
//
UINT32 enable_lcd_power( void )
{
    EnableDeviceClocks(AM_DEVICE_LCDC, TRUE);

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  disable_lcd_power
//
//  This function disables the power for the LCD controller
//
UINT32 disable_lcd_power( void )
{
    EnableDeviceClocks(AM_DEVICE_LCDC, FALSE);

    return ERROR_SUCCESS;
}

void lcd_init(void)
{
	lcdc_device.regs = (LCDC_REGS*)OALPAtoUA(GetAddressByDevice(AM_DEVICE_LCDC));
	lcdc_device.clk = PrcmClockGetClockRate(LCD_PCLK); 
	LcdPdd_LCD_Initialize(&lcdc_device);
	LcdPdd_EnableLCDC(&lcdc_device, TRUE);	
}

void lcd_deinit(void)
{
	if(lcdc_device.regs)
	{
		lcdc_device.regs->raster_control = 0;
		lcdc_device.regs->dma_control = 0;
		lcdc_device.regs->fb0_base = 0;
		lcdc_device.regs->fb0_ceiling = 0;

              /* put in reset mode */
		lcdc_device.regs->clkc_reset = 0xC;
	}
}

//------------------------------------------------------------------------------
//
//  Function:  lcd_config
//
//  This function configures the LCD
//
void lcd_config(UINT32 framebuffer)
{   
	SetBacklightLevel_GPIO(0xFF);
	enable_lcd_power();	
	lcd_init();
}

//------------------------------------------------------------------------------
//
//  Function:  lcd_shutdown
//
//  This function disables the backlight and power of the LCD controller
//
void lcd_shutdown()
{
	SetBacklightLevel_GPIO(0);
	lcd_deinit();
	disable_lcd_power();
}   

//------------------------------------------------------------------------------
//
// end of bsp_logo.c
