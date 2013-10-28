/*
 * linux/drivers/video/ti816x/vpss/core.c
 *
 * VPSS Core driver for TI 816X
 *
 * Copyright (C) 2009 TI
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * Some code and ideas taken from drivers/video/omap2/ driver
 * by Tomi Valkeinen.
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

#define VPSS_SUBMODULE_NAME  "CORE "

#if 1
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <string.h>
#include <strsafe.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <oal.h>
#include <oalex.h>

#else

/* Standard headers */
#include <ti/syslink/Std.h>

/* Osal & utils headers */
#include <ti/syslink/utils/Trace.h>
#include <ti/syslink/utils/String.h>
#include <ti/syslink/utils/Gate.h>
#include <ti/syslink/utils/IGateProvider.h>
#include <ti/syslink/utils/GateSpinlock.h>
#include <ti/syslink/utils/Memory.h>
#include <ti/syslink/utils/List.h>
#include <ti/ipc/MultiProc.h>

#endif

#include "types_temp.h"
#include "fvid2.h"
#include "vps.h"
#include "vps_proxyserver.h"
#include "vps_displayctrl.h"
#include "dc.h"
#include "vps_graphics.h"
#include "grpx.h"
#include "core.h"


#define VPS_DRIVER_NAME  "vpss"



//#ifdef DEBUG
unsigned int vpss_debug = 1;
//#endif

/*module parameters*/
static char *def_mode;
static int  def_tiedvencs;
static char *def_clksrc;
/*time out value is 2 seconds*/
static u32  def_timeout = 2000;

int vps_init(struct platform_device *pdev)
{
    int r;

    VPSSDBG("Entered vps_init\n");

    r = vps_sbuf_init();
    if (r) {
        VPSSERR("failed to allocate share buffer\n");
        return r;
    }

    r = vps_fvid2_init(pdev, def_timeout);
    if (r) {
        VPSSERR("Failed to init fvid2 interface,\n");
        goto exit0;
    }

    r = vps_system_init(pdev);
    if (r) {
        VPSSERR("failed to init system\n");
        goto exit1;
    }

    r = vps_dc_init(pdev, def_mode, def_tiedvencs, def_clksrc);
    if (r) {
        VPSSERR("failed to int display controller.\n");
        goto exit2;
    }

    r = vps_grpx_init(pdev);
    if (r) {
        VPSSERR("failed to int graphics.\n");
        goto exit3;
    }

//    r = vps_video_init(pdev);
//    if (r) {
//        VPSSERR("failed to int video.\n");
//        goto exit4;
//    }
    return 0;

//exit4:
//    vps_grpx_deinit(pdev);

exit3:
    vps_dc_deinit(pdev);

exit2:
    vps_system_deinit(pdev);

exit1:
    vps_fvid2_deinit(pdev);

exit0:
    vps_sbuf_deinit();
    return r;
}

static int vps_remove(struct platform_device *pdev)
{
    int r;

//    vps_video_deinit(pdev);

    vps_grpx_deinit(pdev);
    r = vps_dc_deinit(pdev);
    if (r) {
        VPSSERR("failed to remove display controller.\n");
        return r;
    }
    vps_system_deinit(pdev);
    vps_fvid2_deinit(pdev);

    vps_sbuf_deinit();
    return 0;
}


#if 0

module_param_named(mode, def_mode, charp, S_IRUGO);
MODULE_PARM_DESC(def_mode,
    "Mode of VENCs to be set in the VPSS device");
module_param_named(tiedvencs, def_tiedvencs, int, S_IRUGO);
MODULE_PARM_DESC(def_tiedvencs,
    "tied vencs to be set in the VPSS device");
module_param_named(clksrc, def_clksrc, charp, S_IRUGO);
MODULE_PARM_DESC(def_clksrc,
    "VENC clock source to be set in the VPSS device");
module_param_named(sbufaddr, def_sbaddr, charp, S_IRUGO);
MODULE_PARM_DESC(def_pladdr,
    "sharing buffer address to be set in the VPSS device");
module_param_named(sbufsize, def_sbsize, charp, S_IRUGO);
MODULE_PARM_DESC(def_plsize,
    "sharing buffer size to be set in the VPSS device");
module_param_named(timeout, def_timeout, uint, S_IRUGO);
MODULE_PARM_DESC(def_timeout,
    "timeout value to be set in the VPSS device \
        for waiting the response from M3");

subsys_initcall(vps_init);
module_exit(vps_cleanup);


MODULE_AUTHOR("Yihe Hu <yihehu@ti.com");
MODULE_DESCRIPTION("TI81XX Video Processing Subsystem");
MODULE_LICENSE("GPL v2");
#endif
