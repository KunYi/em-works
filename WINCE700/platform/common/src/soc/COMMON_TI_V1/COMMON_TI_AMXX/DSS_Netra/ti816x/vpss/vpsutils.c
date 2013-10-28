/******************************************************************************
 **+-------------------------------------------------------------------------+**
 **|                            ****                                         |**
 **|                            ****                                         |**
 **|                            ******o***                                   |**
 **|                      ********_///_****                                  |**
 **|                      ***** /_//_/ ****                                  |**
 **|                       ** ** (__/ ****                                   |**
 **|                           *********                                     |**
 **|                            ****                                         |**
 **|                            ***                                          |**
 **|                                                                         |**
 **|         Copyright (c) 2008-2009 Texas Instruments Incorporated          |**
 **|                        ALL RIGHTS RESERVED                              |**
 **|                                                                         |**
 **| Permission is hereby granted to licensees of Texas Instruments          |**
 **| Incorporated (TI) products to use this computer program for the sole    |**
 **| purpose of implementing a licensee product based on TI products.        |**
 **| No other rights to reproduce, use, or disseminate this computer         |**
 **| program, whether in part or in whole, are granted.                      |**
 **|                                                                         |**
 **| TI makes no representation or warranties with respect to the            |**
 **| performance of this computer program, and specifically disclaims        |**
 **| any responsibility for any damages, special or consequential,           |**
 **| connected with the use of this program.                                 |**
 **|                                                                         |**
 **+-------------------------------------------------------------------------+**
 ******************************************************************************/

/**  \file      vpsutils_grpx.c
 *
 *   \brief     Code to generate color bar patter based on the format and dimension.
 *
 *   (C) Copyright 2010, Texas Instruments, Inc
 *
 *   \author    PSP
 *
 *
 */


/* ---------------------------------------------------------------------------
 *                             Include Files
 * ---------------------------------------------------------------------------*/
//#include <assert.h>
//#include <string.h>
//#include <xdc/std.h>
//#include <xdc/runtime/System.h>

#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <string.h>
#include <strsafe.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <oal.h>
#include <oalex.h>

#include "../inc/types_temp.h"
#include "../inc/fvid2.h"
#include "../inc/vps.h"
#include "../inc/vps_proxyserver.h"
#include "../inc/vps_displayctrl.h"
#include "../inc/dc.h"
#include "../inc/vps_graphics.h"
#include "../inc/grpx.h"

#include "../inc/core.h"
#include "../inc/system.h"

/* ---------------------------------------------------------------------------
 *                             Global variables
 * ---------------------------------------------------------------------------*/
/*used for ARGB444 or RGBA444,where alpha = 0;*/
static u16 rgb444[2][8] =
{
    {
        (0x0F << 8) | (0x0F << 4) | (0x0F),
        (0x00 << 8) | (0x00 << 4) | (0x00),
        (0x0F << 8) | (0x00 << 4) | (0x00),
        (0x00 << 8) | (0x0F << 4) | (0x00),
        (0x00 << 8) | (0x00 << 4) | (0x0F),
        (0x0F << 8) | (0x0F << 4) | (0x00),
        (0x0F << 8) | (0x00 << 4) | (0x0F),
        (0x00 << 8) | (0x0F << 4) | (0x0F),
    },
    {
        (0x00 << 8) | (0x0F << 4) | (0x0F),
        (0x0F << 8) | (0x00 << 4) | (0x0F),
        (0x0F << 8) | (0x0F << 4) | (0x00),
        (0x00 << 8) | (0x00 << 4) | (0x0F),
        (0x00 << 8) | (0x0F << 4) | (0x00),
        (0x0F << 8) | (0x00 << 4) | (0x00),
        (0x00 << 8) | (0x00 << 4) | (0x00),
        (0x0F << 8) | (0x0F << 4) | (0x0F),
    }
};
/*usedfor ARGB1555 or RGBA5551, where alpha = 0*/
static u32 rgb555[2][8] =
{
    {
        (0x1F << 10) | (0x1F << 5) | (0x1F),
        (0x00 << 10) | (0x00 << 5) | (0x00),
        (0x1F << 10) | (0x00 << 5) | (0x00),
        (0x00 << 10) | (0x1F << 5) | (0x00),
        (0x00 << 10) | (0x00 << 5) | (0x1F),
        (0x1F << 10) | (0x1F << 5) | (0x00),
        (0x1F << 10) | (0x00 << 5) | (0x1F),
        (0x00 << 10) | (0x1F << 5) | (0x1F),
    },
    {
        (0x00 << 10) | (0x1F << 5) | (0x1F),
        (0x1F << 10) | (0x00 << 5) | (0x1F),
        (0x1F << 10) | (0x1F << 5) | (0x00),
        (0x00 << 10) | (0x00 << 5) | (0x1F),
        (0x00 << 10) | (0x1F << 5) | (0x00),
        (0x1F << 10) | (0x00 << 5) | (0x00),
        (0x00 << 10) | (0x00 << 5) | (0x00),
        (0x1F << 10) | (0x1F << 5) | (0x1F),

    }
};
/*used for ARGB6666 or RGBA6666, where alpha = 0*/
static u32 rgb666[2][8] =
{
    {
        (0x3F << 12) | (0x3F << 6) | (0x3F),
        (0x00 << 12) | (0x00 << 6) | (0x00),
        (0x3F << 12) | (0x00 << 6) | (0x00),
        (0x00 << 12) | (0x3F << 6) | (0x00),
        (0x00 << 12) | (0x00 << 6) | (0x3F),
        (0x3F << 12) | (0x3F << 6) | (0x00),
        (0x3F << 12) | (0x00 << 6) | (0x3F),
        (0x00 << 12) | (0x3F << 6) | (0x3F),
    },
    {
        (0x00 << 12) | (0x3F << 6) | (0x3F),
        (0x3F << 12) | (0x00 << 6) | (0x3F),
        (0x3F << 12) | (0x3F << 6) | (0x00),
        (0x00 << 12) | (0x00 << 6) | (0x3F),
        (0x00 << 12) | (0x3F << 6) | (0x00),
        (0x3F << 12) | (0x00 << 6) | (0x00),
        (0x00 << 12) | (0x00 << 6) | (0x00),
        (0x3F << 12) | (0x3F << 6) | (0x3f),

    }
};
/*used for RGB565 format*/
static u16 rgb565[2][8] =
{
    {
        (0x1F << 11) | (0x3F << 5) | (0x1F),
        (0x00 << 11) | (0x00 << 5) | (0x00),
        (0x1F << 11) | (0x00 << 5) | (0x00),
        (0x00 << 11) | (0x3F << 5) | (0x00),
        (0x00 << 11) | (0x00 << 5) | (0x1F),
        (0x1F << 11) | (0x3F << 5) | (0x00),
        (0x1F << 11) | (0x00 << 5) | (0x1F),
        (0x00 << 11) | (0x3F << 5) | (0x1F),
    },
    {
        (0x00 << 11) | (0x3F << 5) | (0x1F),
        (0x1F << 11) | (0x00 << 5) | (0x1F),
        (0x1F << 11) | (0x3F << 5) | (0x00),
        (0x00 << 11) | (0x00 << 5) | (0x1F),
        (0x00 << 11) | (0x3F << 5) | (0x00),
        (0x1F << 11) | (0x00 << 5) | (0x00),
        (0x00 << 11) | (0x00 << 5) | (0x00),
        (0x1F << 11) | (0x3F << 5) | (0x1F),
     }
};
/*used for ARGB8888/RGBA8888/RGB888 format, where alpha = 0*/
static u32 rgb888[2][8] =
{
    {
        (0xFF << 16) | (0xFF << 8) | (0xFF),
        (0x00 << 16) | (0x00 << 8) | (0x00),
        (0xFF << 16) | (0x00 << 8) | (0x00),
        (0x00 << 16) | (0xFF << 8) | (0x00),
        (0x00 << 16) | (0x00 << 8) | (0xFF),
        (0xFF << 16) | (0xFF << 8) | (0x00),
        (0xFF << 16) | (0x00 << 8) | (0xFF),
        (0x00 << 16) | (0xFF << 8) | (0xFF),
    },
    {
         (0x00 << 16) | (0xFF << 8) | (0xFF),
         (0xFF << 16) | (0x00 << 8) | (0xFF),
         (0xFF << 16) | (0xFF << 8) | (0x00),
         (0x00 << 16) | (0x00 << 8) | (0xFF),
         (0x00 << 16) | (0xFF << 8) | (0x00),
         (0xFF << 16) | (0x00 << 8) | (0x00),
         (0x00 << 16) | (0x00 << 8) | (0x00),
         (0xFF << 16) | (0xFF << 8) | (0xFF),
    },
};


/* ---------------------------------------------------------------------------
 *                             Function implementation
 * ---------------------------------------------------------------------------*/
/*******************************************************************************
 ** Function name:     VpsUtils_getPitch
 **
 ** Description:       Get the ptich based on the width and bpp.
 ******************************************************************************/

/**
 * \brief VpsUtils_getPitch
 *
 *  Function to calculate the pitch
 */

u32 VpsUtils_getPitch(u32 width, u32 bpp)
{
    u32 pitch;

    pitch = (width * bpp >> 3);
    if (pitch & 0xF)
        pitch += 16 - (pitch & 0xF);

    return pitch;
}

/**
 * \brief VpsUtils_grpxGenPattern
 *
 *  Function to create color bar based on the format and dimension
 */

void VpsUtils_grpxGenPattern(u8 *addr,
                             u32 df,
                             u32 width,
                             u32 height,
                             u8 reversed,
                             u8 alpha)
{
    u32 i, j, k;
    u32 pitch;
    static u8 byte = 0;
    static u8 red = 0xff;
    static u8 green = 0x0;
    static u8 blue = 0x0;
    static u8 count = 0;

    if (count == 0)
    {
        red = 0xff;
        green = 0;
        blue = 0;
    }
    else if (count == 1)
    {
        red = 0;
        green = 0xff;
        blue = 0;
    }
    else if (count == 2)
    {
        red = 0;
        green = 0;
        blue = 0xff;
    }
    count++;
    if (count >= 3)
        count = 0;

    switch (df)
    {
        case FVID2_DF_ARGB32_8888:
        {
            u32 *start = (u32 *)addr;
            u32 pixel;

            pitch = VpsUtils_getPitch(width, 32);
            pixel = ((alpha << 24) | (red << 16) | (green << 8) | blue);

            for(i = 0 ; i < 8 ; i++)
            {
                for(j = 0 ; j < (height / 8) ; j++)
                {
#if 0
                    for(k = 0 ; k < (pitch/4); k++)
                    {
                        start[k] = rgb888[reversed][i] | (alpha << 24);
                    }
                    start += (pitch/4);
#else
                    for(k = 0 ; k < (pitch/4); k++)
                    {
                        start[k] = pixel;
                    }
                    start += (pitch/4);
#endif
                }
            }
            break;
        }
        case FVID2_DF_RGBA32_8888:
        {
            u32 *start = (u32 *)addr;
            pitch = VpsUtils_getPitch(width, 32);

            for(i = 0 ; i < 8 ; i++)
            {
                for(j = 0 ; j < (height / 8) ; j++)
                {
                        for(k = 0 ; k < (pitch/4); k++)
                        {
                            /*make the Alpha to zero at the lsb*/
                            start[k] = (rgb888[reversed][i] << 8) | alpha;
                        }
                        start += (pitch/4);
                }
            }
            break;
        }
        case FVID2_DF_RGB24_888:
        {
            u8 *start = (u8 *)addr;
//            u32 l;
            pitch = VpsUtils_getPitch(width, 24);

            for(i = 0 ; i < 8 ; i++)
            {
                for(j = 0 ; j < (height / 8) ; j++)
                {
                    for(k = 0 ; k < width; k++)
                    {
#if 0
                        for (l = 0; l < 3; l++)
                        {
//                            start[3*k + l] = (rgb888[reversed][i] >> (8 * l)) & 0xFF ;
                            start[3*k + l] = byte;
                            byte += 0x10;
                        }
#else
                        start[3*k + 0] = red;
                        start[3*k + 1] = green;
                        start[3*k + 2] = blue;
#endif
                    }
                    start += pitch;
                }
            }
            break;
        }

        case FVID2_DF_RGB16_565:
        {
            u16 *start = (u16 *) addr;
            pitch = VpsUtils_getPitch(width, 16);
            for (i = 0; i < 8; i++)
            {
                for(j = 0 ; j < (height / 8) ; j++)
                {
                    for (k = 0; k < (pitch/2); k++)
                    {
                        start[k] = rgb565[reversed][i];
                    }
                    start += (pitch/2);
                }
            }
            break;
        }
        case FVID2_DF_ARGB16_1555:
        {
            u16 *start = (u16 *)addr;
            pitch = VpsUtils_getPitch(width, 16);
            for (i = 0; i < 8; i++)
            {
                for(j = 0 ; j < (height / 8) ; j++)
                {
                    for (k = 0; k < (pitch/2); k++)
                    {
                        start[k] = rgb555[reversed][i] | ((alpha & 1) << 15);
                    }
                    start += (pitch/2);
                }
            }

            break;
        }
        case FVID2_DF_RGBA16_5551:
        {
            u16 *start = (u16 *)addr;
            pitch = VpsUtils_getPitch(width, 16);
            for (i = 0; i < 8; i++)
            {
                for(j = 0 ; j < (height / 8) ; j++)
                {
                    for (k = 0; k < (pitch/2); k++)
                    {
                        start[k] = (rgb555[reversed][i] << 1) | (alpha & 1);
                    }
                    start += (pitch/2);
                }
            }

            break;
        }

        case FVID2_DF_ARGB16_4444:
        {
            u16 *start = (u16 *) addr;
            pitch = VpsUtils_getPitch(width, 16);

            for (i = 0; i < 8; i++)
            {
                for(j = 0 ; j < (height / 8) ; j++)
                {
                    for (k = 0; k < (pitch/2); k++)
                    {
                        start[k] = rgb444[reversed][i] | ((alpha & 0xF) << 12);
                    }
                    start += (pitch/2);
                }
            }
            break;
        }
        case FVID2_DF_RGBA16_4444:
        {
            u16 *start = (u16 *) addr;
            pitch = VpsUtils_getPitch(width, 16);

            for (i = 0; i < 8; i++)
            {
                for(j = 0 ; j < (height / 8) ; j++)
                {
                    for (k = 0; k < (pitch/2); k++)
                    {
                        start[k] = (rgb444[reversed][i] << 4) | (alpha & 0xF);
                    }
                    start += (pitch/2);
                }
            }

            break;
        }

        case FVID2_DF_ARGB24_6666:
        {
            u8 *start = (u8 *)addr;
            int l;
            u32 temp;

            pitch = VpsUtils_getPitch(width, 24);
            for(i = 0 ; i < 8 ; i++)
            {
                for(j = 0 ; j < (height / 8) ; j++)
                {
                    for(k = 0 ; k < width; k++)
                    {
                        temp = rgb666[reversed][i] | ((alpha & 0x3F) << 18);
                        for (l = 0; l < 3; l++)
                        {

                            start[3 * k + l] = ((temp >> (8 * l)) & 0xFF);
                        }

                    }
                    start += pitch;
                }
            }
            break;
        }
        case FVID2_DF_RGBA24_6666:
        {
            u8 *start = (u8 *)addr;
            int l;
            u32 temp;

            pitch = VpsUtils_getPitch(width, 24);
            for(i = 0 ; i < 8 ; i++)
            {
                for(j = 0 ; j < (height / 8) ; j++)
                {
                    for(k = 0 ; k < width; k++)
                    {
                        temp = (rgb666[reversed][i] << 6) | (alpha & 0x3F);
                        for (l = 0; l < 3; l++)
                        {
                            start[k * 3 + l] = (temp >> (8 * l)) & 0xFF ;
                        }

                    }
                    start += pitch;
                }
            }
            break;
        }

}
}

