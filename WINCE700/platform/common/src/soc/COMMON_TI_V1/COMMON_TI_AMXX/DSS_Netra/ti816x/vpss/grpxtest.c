/*
 * linux/drivers/video/ti816x/vpss/grpx.c
 *
 * VPSS graphics driver for TI 816X
 *
 * Copyright (C) 2009 TI
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#define VPSS_SUBMODULE_NAME "GRPX "

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
#include "../inc/ProcMgrApp.h"

#include "core.h"
#include "vpss.h"
#include "am38xx_display_cfg.h"

#define ENABLE_M3_ACCESS    1
#define SPEED_TEST          0

extern int vps_init(struct platform_device *pdev);

extern struct vps_grpx_ctrl *gctrl;

extern void VpsUtils_grpxGenPattern(u8 *addr,
                             u32 df,
                             u32 width,
                             u32 height,
                             u8 reversed,
                             u8 alpha);

extern u32 VpsUtils_getPitch(u32 width, u32 bpp);

/*allocate buffer*/
void *vps_framebuf_alloc(size_t size, u32 *paddr)
{
    static u32 base_vaddr = 0;
    static u32 next_vaddr = 0;
    static u32 next_paddr = 0;
    u32 vaddr = 0;
    u32 temp_size;
    static BOOL  init = 0;
    PHYSICAL_ADDRESS pa;
    DWORD  dwDisplayBufferSize;
    DWORD  dwPhysicalDisplayAddr;

    if (!init)
    {
        Display_GetFrameBufMemory( &dwDisplayBufferSize, &dwPhysicalDisplayAddr );

        pa.HighPart = 0;
        pa.LowPart = dwPhysicalDisplayAddr;

        base_vaddr = (u32)MmMapIoSpace(pa, dwDisplayBufferSize,FALSE);
        next_vaddr = base_vaddr;
        next_paddr = (u32)dwPhysicalDisplayAddr;
        init = 1;
    }

    vaddr = next_vaddr;
    *paddr = next_paddr;

    /* Align the size to the next 32 byte boundary. */
    temp_size = ((size + 31)/32)*32;
    next_vaddr += temp_size;
    next_paddr += temp_size;

    RETAILMSG(1, (L"vps frame buf alloc. Paddr=0x%x, vaddr=0x%x, size=0x%x\n", 
                (u32)*paddr, vaddr, size));
    return (void *)vaddr; 
}

void grpx_display_test()
{
    u32  paddr;
    u32  buf_size;
    u32* framebuf_vaddr;

//    /* Download the M3 with the VPSS image and initialize IPC */
//    ProcMgrApp_startup ();

    Sleep(2000);

    vps_init(NULL);

    /* Allocate a buffer and fill it with a pattern */
    buf_size = REGION_WIDTH * REGION_HEIGHT * GRPX_BPP/8;
    framebuf_vaddr = vps_framebuf_alloc(buf_size, &paddr);

    VpsUtils_grpxGenPattern((u8*)framebuf_vaddr,
                            GRPX_FORMAT,
                            REGION_WIDTH,
                            REGION_HEIGHT,
                            0,
                            0);

    /* Set data format */
    gctrl->set_format(gctrl, GRPX_BPP, GRPX_FORMAT, VpsUtils_getPitch(REGION_WIDTH, GRPX_BPP));

    /* Set frame buffer */
    gctrl->set_buffer(gctrl, paddr);

    /* Start the gfx pipe */
    gctrl->start(gctrl);

    while (1)
    {
        /* Change frames every 5 seconds */
        Sleep(5000);

        VpsUtils_grpxGenPattern((u8*)framebuf_vaddr,
                                GRPX_FORMAT,
                                REGION_WIDTH,
                                REGION_HEIGHT,
                                0,
                                0);
    }
}

void grpx_flipbuf_test()
{
    u32  paddr, paddr1, paddr2, paddr3;
    u32  buf_size;
    u32* framebuf_vaddr;
    u32* framebuf_vaddr1;
    u32* framebuf_vaddr2;
    u32* framebuf_vaddr3;
    u32 bufnum = 1;
    u32 framenum = 0;

#if SPEED_TEST
    DWORD stime, etime;
    u32 td = 0;
#endif

    RETAILMSG(1, (L"grpx flip buffer test\n"));

    /* Download the M3 with the VPSS image and initialize IPC */
//    ProcMgrApp_startup ();

    Sleep(2000);

    vps_init(NULL);

    /* Allocate a buffer and fill it with a pattern */
    buf_size = REGION_WIDTH * REGION_HEIGHT * GRPX_BPP/8;
    framebuf_vaddr1 = vps_framebuf_alloc(buf_size, &paddr1);
    framebuf_vaddr2 = vps_framebuf_alloc(buf_size, &paddr2);
    framebuf_vaddr3 = vps_framebuf_alloc(buf_size, &paddr3);

    VpsUtils_grpxGenPattern((u8*)framebuf_vaddr1,
                            GRPX_FORMAT,
                            REGION_WIDTH,
                            REGION_HEIGHT,
                            0,
                            0);

    VpsUtils_grpxGenPattern((u8*)framebuf_vaddr2,
                            GRPX_FORMAT,
                            REGION_WIDTH,
                            REGION_HEIGHT,
                            0,
                            0);

    VpsUtils_grpxGenPattern((u8*)framebuf_vaddr3,
                            GRPX_FORMAT,
                            REGION_WIDTH,
                            REGION_HEIGHT,
                            0,
                            0);

    /* Set data format */
    gctrl->set_format(gctrl, GRPX_BPP, GRPX_FORMAT, VpsUtils_getPitch(REGION_WIDTH, GRPX_BPP));

    /* Set frame buffer */
    gctrl->set_buffer(gctrl, paddr1);

    /* Start the gfx pipe */
    gctrl->start(gctrl);
#if SPEED_TEST
    stime = GetTickCount();
#endif

    while (1)
    {
        switch (bufnum)
        {
            case 0:
                framebuf_vaddr = framebuf_vaddr1;
                paddr = paddr1;
                break;

            case 1:
                framebuf_vaddr = framebuf_vaddr2;
                paddr = paddr2;
                break;

            case 2:
                framebuf_vaddr = framebuf_vaddr3;
                paddr = paddr3;
                break;
        }

#if SPEED_TEST
        etime = GetTickCount();

        td = etime - stime;
        if (td >= 5000) 
        {
            RETAILMSG(1, (L"Flip buf test. Frames=%d, fps=%d\n", 
                        (u32)framenum, framenum/5));
            stime = GetTickCount();
            framenum = 0;
        }
#else
        /* Change frames every 5 seconds */
        Sleep(5000);
#endif
        gctrl->flip_buffer(gctrl, paddr);

        bufnum++;
        framenum++;
        if (bufnum == 3)
        {
            bufnum = 0;
        }
    }
}

void vpssm3_download()
{
#if ENABLE_M3_ACCESS
    /* Download the M3 with the VPSS image and initialize IPC */
    ProcMgrApp_startup ();
#else
#endif
}

void grpx_setup()
{
#if ENABLE_M3_ACCESS
    int r;

    /* With M3 code/data cached this delay seems to be enough. If it is not cached change this to 4000ms */
    Sleep(1000);

    r = vps_init(NULL);
    if (r)
    {
        RETAILMSG(1, (L"vps init failed\n"));
        return;
    }

    gctrl->set_format(gctrl, GRPX_BPP, GRPX_FORMAT, VpsUtils_getPitch(REGION_WIDTH, GRPX_BPP));
#else
    return;
#endif
}

void grpx_setbuf(u32 paddr)
{
#if ENABLE_M3_ACCESS
    if (gctrl)
    {
        /* Set frame buffer */
        gctrl->set_buffer(gctrl, paddr);
    }
#else
    return;
#endif
}

void grpx_start()
{
#if ENABLE_M3_ACCESS
    if (gctrl)
    {
        /* Start the gfx pipe */
        gctrl->start(gctrl);
    }
#else
    return;
#endif
}

void grpx_flipbuf(u32 paddr)
{
#if ENABLE_M3_ACCESS
    if (gctrl)
    {
        gctrl->flip_buffer(gctrl, paddr);
    }
#else
    return;
#endif
}
