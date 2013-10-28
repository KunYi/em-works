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

#ifndef __VPS_SBUF_H__
#define __VPS_SBUF_H__

#include "vpss.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VPSS_SHARED_MEM_ALIGN    128         // 128 byte alignment (arbitrary).

/* Defines the memory information shared between A8 and M3 for each submodule */
struct vps_payload_info {
    DWORD   paddr;
    void    *vaddr;
    DWORD   handle;
    DWORD   size;       /* Requested size */
    DWORD   allocsize;  /* allocated size. Always >= requested size */
};

/*shared buffer memory util functions*/
int vps_sbuf_init(void);
int vps_sbuf_deinit(void);
int vps_sbuf_alloc(struct vps_payload_info *pinfo);
int vps_sbuf_free (struct vps_payload_info *pinfo);

#ifdef __cplusplus
}
#endif

#endif __VPS_SBUF_H__
