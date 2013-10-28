/*
 * linux/drivers/video/ti816x/vpss/fvid2.c
 *
 * VPSS FVID2 driver for TI 816X
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
#define VPSS_SUBMODULE_NAME "FVID2"

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

#include "../inc/core.h"
#include "../inc/system.h"
#include "../inc/syslinkapi.h"


/*3GRPX + 1 DISPCTRL +  SYSTEM + 5 display + 4 capature*/
#define VPS_FVID2_NUM   (5 + 5 + 4)

struct vps_fvid2_ctrl {
    bool                isused;
    u32                 firm_ver;
    u32                 fvid2handle;
    u32                 rmprocid;
    u32                 notifyno;
    u32                 lineid;
    struct vps_psrvfvid2createparams    *fcrprms;
    u32                 fcrprms_phy;
    struct vps_psrvfvid2deleteparams    *fdltprms;
    u32                 fdltprms_phy;
    struct vps_psrvfvid2controlparams   *fctrlprms;
    u32                 fctrlprms_phy;
    struct vps_psrvfvid2queueparams     *fqprms;
    u32                 fqprms_phy;
    struct vps_psrvfvid2dequeueparams   *fdqprms;
    u32                 fdqprms_phy;
    struct vps_psrvcallback         *cbprms;
    u32                 cbprms_phy;
    struct vps_psrvcommandstruct        *cmdprms;
    u32                 cmdprms_phy;
    struct vps_psrverrorcallback        *ecbprms;
    u32                 ecbprms_phy;
};

static struct vps_fvid2_ctrl  *fvid2_ctrl[VPS_FVID2_NUM];
static struct vps_payload_info *fvid2_payload_info;
static struct vps_psrvgetstatusvercmdparams *vps_verparams;
static u32    vps_verparams_phy;
static unsigned long    vps_timeout = 0u;

/*define the information used by the proxy running in M3*/
#define VPS_FVID2_RESERVED_NOTIFY       0x09
#define VPS_FVID2_M3_INIT_VALUE         (0xAAAAAAAA)
#define VPS_FVID2_PS_LINEID             0
#define CURRENT_VPS_FIRMWARE_VERSION    (0x1000126)

#define BUG_ON(cond) ASSERT(!(cond))

void schedule()
{
//    Sleep(1);
}

static inline u32 time_diff(DWORD stime, DWORD etime)
{
    return (etime - stime);
}

static void vps_callback(u16 procid,
          u16 lineid,
          u32 eventno,
          void *arg,
          u32 payload)
{
    struct vps_psrvcallback *appcb;
    struct vps_fvid2_ctrl *fctrl = (struct vps_fvid2_ctrl *)arg;

//    VPSSDBG("Received No. %d event at line ID %d from processor %d\n",
//        eventno, lineid, procid);
    /*perform all kinds of sanity check*/

    if (payload == 0) {
//        VPSSERR("Payload is empty.\n");
        return;
    }
    if (fctrl->cbprms_phy != payload) {
//        VPSSERR("payload not matched\n");
        return;
    }
    appcb = fctrl->cbprms;
    if (appcb->appcallback == NULL) {
//        VPSSERR("no callback registered\n");
        return;
    }

    if (arg == NULL) {
//        VPSSERR("empty callback arguments.\n");
        return ;
    }

    if (eventno != fctrl->notifyno) {
//        VPSSERR("received event id %d and expected id %d\n",
//            eventno, fctrl->notifyno);
        return;
    }

    if (lineid != fctrl->lineid) {
//        VPSSERR("received lineid %d and expected lineid %d\n",
//            lineid, fctrl->lineid);
        return;
    }

    if (procid != fctrl->rmprocid) {
//        VPSSERR("received from processor %d and  \
//                    expected from processor %d\n",
//            procid, fctrl->rmprocid);
        return;
    }
    /*dispatch the callback*/
    if (appcb->cbtype == VPS_FVID2_IO_CALLBACK)
        appcb->appcallback(arg, appcb->appdata, NULL);
}

static struct vps_fvid2_ctrl *vps_get_fvid2_ctrl(void)
{
    int i;
    for (i = 0; i < VPS_FVID2_NUM; i++) {
        if (false == fvid2_ctrl[i]->isused) {
            fvid2_ctrl[i]->isused = true;
            return fvid2_ctrl[i];
        }
   }

    return NULL;
}

static int vps_check_fvid2_ctrl(void *handle)
{
    int i;
    for (i = 0; i < VPS_FVID2_NUM; i++) {
        if (((u32)handle == (u32)fvid2_ctrl[i]) &&
            (fvid2_ctrl[i]->isused == true))
            return 0;
    }

    return 1;
}

 void *vps_fvid2_create(u32 drvid,
            u32 instanceid,
            void *createargs,
            void *createstatusargs,
            struct fvid2_cbparams *cbparams)
{

    struct vps_fvid2_ctrl *fctrl = NULL;
    int status;
    DWORD stime, etime;
    u32 td = 0;

    fctrl = vps_get_fvid2_ctrl();
    if (fctrl == NULL)
        return NULL;

    /*assembel the create parameter structure*/
    fctrl->fcrprms->command = VPS_FVID2_CREATE;
    fctrl->fcrprms->hosttaskinstance = VPS_FVID2_TASK_TYPE_1,
    fctrl->fcrprms->createargs = createargs;
    fctrl->fcrprms->createstatusargs = createstatusargs;
    fctrl->fcrprms->drvid = drvid;
    fctrl->fcrprms->cbparams = cbparams;
    fctrl->fcrprms->instanceid = instanceid;
    fctrl->fcrprms->fvid2handle = (void *)VPS_FVID2_M3_INIT_VALUE;


    if (cbparams == NULL) {
        fctrl->fcrprms->ioreqcb = NULL;
        fctrl->fcrprms->errcb = NULL;
    } else {
        fctrl->fcrprms->ioreqcb =
            (struct vps_psrvcallback *)fctrl->cbprms_phy;
        fctrl->fcrprms->errcb =
            (struct vps_psrverrorcallback *)fctrl->ecbprms_phy;
    }

    fctrl->cmdprms->cmdtype = VPS_FVID2_CMDTYPE_SIMPLEX;
    fctrl->cmdprms->simplexcmdarg = (void *)fctrl->fcrprms_phy;
    /*set the event to M3*/
    status = Notify_sendEvent(fctrl->rmprocid,
                  fctrl->lineid,
                  VPS_FVID2_RESERVED_NOTIFY,
                  fctrl->cmdprms_phy,
                  1);

    if (status < 0) {
        VPSSERR("send create event failed with status 0x%0x\n",
              status);
        fctrl->isused = false;
        return NULL;
    }
    stime = GetTickCount();
    etime = stime;
    while ((fctrl->fcrprms->fvid2handle ==
            (void *)VPS_FVID2_M3_INIT_VALUE)) {
        schedule();
        /*time out check*/
        if (vps_timeout) {
            etime = GetTickCount();
            td = time_diff(stime, etime);
            if (vps_timeout < td) {
                fctrl->isused = false;
                VPSSERR("create timeout\n");
                return NULL;
            }
        }
    }

    fctrl->notifyno = fctrl->fcrprms->syslnkntyno;
    fctrl->fvid2handle = (u32)fctrl->fcrprms->fvid2handle;
    VPSSDBG("Fvid2 handle 0x%08x with notifyno %d within %d ms\n",
        (u32)fctrl->fvid2handle,
        fctrl->notifyno,
        td);

    if (fctrl->fvid2handle == 0) {
        fctrl->isused = false;
        VPSSERR("create handle is NULL\n");
        return NULL;
    }
    /*register the callback if successfully*/
    if (cbparams != NULL) {
        status = Notify_registerEvent(fctrl->rmprocid,
                      fctrl->lineid,
                      fctrl->notifyno,
                      (Notify_FnNotifyCbck)vps_callback,
                      (void *)fctrl);
        if (status < 0) {
            VPSSERR("register event status 0x%08x\n", status);
            /*since the registration is failed,
                delete from M3 as well*/
            vps_fvid2_delete(fctrl, NULL);
            fctrl->isused = false;
            return NULL;
        }
    }
    return (void *)fctrl;
}
EXPORT_SYMBOL(vps_fvid2_create);

int vps_fvid2_delete(void *handle, void *deleteargs)
{

    struct vps_fvid2_ctrl *fctrl = (struct vps_fvid2_ctrl *)handle;
    int status;
    DWORD stime, etime;
    u32 td = 0;

    VPSSDBG("enter delete.\n");
    if (vps_check_fvid2_ctrl(handle))
        return -EINVAL;

    fctrl->fdltprms->command = VPS_FVID2_DELETE;
    fctrl->fdltprms->fvid2handle = (void *)fctrl->fvid2handle;
    fctrl->fdltprms->deleteargs = deleteargs;
    fctrl->fdltprms->returnvalue = VPS_FVID2_M3_INIT_VALUE;


    fctrl->cmdprms->cmdtype = VPS_FVID2_CMDTYPE_SIMPLEX;
    fctrl->cmdprms->simplexcmdarg = (void *)fctrl->fdltprms_phy;

    /*send event to proxy in M3*/
    status = Notify_sendEvent(fctrl->rmprocid,
                  fctrl->lineid,
                  fctrl->notifyno,
                  fctrl->cmdprms_phy,
                  1);

    if (status < 0) {
        VPSSERR("set delete event failed status 0x%08x\n", status);
        return -EINVAL;
    } else {
        stime = GetTickCount();
        etime = stime;
        while ((fctrl->fdltprms->returnvalue ==
            VPS_FVID2_M3_INIT_VALUE)) {
            schedule();
            /*time out check*/
            if (vps_timeout) {
                etime = GetTickCount();
                td = time_diff(stime, etime);
                if (vps_timeout < td) {
                    VPSSERR("delete time out\n");
                    return -ETIMEDOUT;
                }
            }
        }

        VPSSDBG("delete event return %d within %d ms\n",
             fctrl->fdltprms->returnvalue,
             td);

    }
    if (fctrl->fcrprms->cbparams != NULL) {
        status = Notify_unregisterEvent(
                    fctrl->rmprocid,
                    fctrl->lineid,
                    fctrl->notifyno,
                    (Notify_FnNotifyCbck)vps_callback,
                    (void *)fctrl);

        if (status < 0)
            VPSSERR("unregister Event status 0x%08x\n", status);
    }

    fctrl->isused = false;
    fctrl->fvid2handle = 0;
    return fctrl->fdltprms->returnvalue;

}
EXPORT_SYMBOL(vps_fvid2_delete);

int vps_fvid2_control(void *handle,
              u32 cmd,
              void *cmdargs,
              void *cmdstatusargs)
{
    struct vps_fvid2_ctrl *fctrl = (struct vps_fvid2_ctrl *)handle;
    int status;
    DWORD etime, stime;
    u32 td = 0;

    if (vps_check_fvid2_ctrl(handle))
        return -EINVAL;

    VPSSDBG("send control with cmd 0x%08x\n", cmd);
    /*assembel the structure*/
    fctrl->fctrlprms->command = VPS_FVID2_CONTROL;
    fctrl->fctrlprms->fvid2handle = (void *)fctrl->fvid2handle;
    fctrl->fctrlprms->cmd = cmd;
    fctrl->fctrlprms->cmdargs = cmdargs;
    fctrl->fctrlprms->cmdstatusargs = cmdstatusargs;
    fctrl->fctrlprms->returnvalue = VPS_FVID2_M3_INIT_VALUE;

    fctrl->cmdprms->cmdtype = VPS_FVID2_CMDTYPE_SIMPLEX;
    fctrl->cmdprms->simplexcmdarg = (void *)fctrl->fctrlprms_phy;

    /*send the event*/
    status = Notify_sendEvent(fctrl->rmprocid,
                  fctrl->lineid,
                  fctrl->notifyno,
                  fctrl->cmdprms_phy,
                  1);

    if (status < 0) {
        VPSSERR("send control with cmd 0x%08x status 0x%08x\n",
              cmd, status);
        return -EINVAL;
    } else {
        stime = GetTickCount();
        etime = stime;
        while (fctrl->fctrlprms->returnvalue ==
            VPS_FVID2_M3_INIT_VALUE) {

            schedule();
            if (vps_timeout) {
                etime = GetTickCount();
                td = time_diff(stime, etime);
                if (vps_timeout < td) {
                    VPSSERR("contrl event 0x%x timeout\n",
                        cmd);
                    return -ETIMEDOUT;
                }
            }
        }
    }

    VPSSDBG("control event 0x%x return %d within %d ms.\n",
        cmd,
        fctrl->fctrlprms->returnvalue,
        td);

    return fctrl->fctrlprms->returnvalue;

}
EXPORT_SYMBOL(vps_fvid2_control);

int vps_fvid2_queue(void *handle,
            struct fvid2_framelist *framelist,
            u32 streamid)
{
    struct vps_fvid2_ctrl *fctrl = (struct vps_fvid2_ctrl *)handle;
    int status;
    DWORD stime, etime;
    u32 td = 0;

    if (vps_check_fvid2_ctrl(handle)) {
        VPSSERR("Q handle 0x%08x\n", (u32)handle);
        return -EINVAL;
    }
    /*assemble the structure*/
    fctrl->fqprms->command = VPS_FVID2_QUEUE;
    fctrl->fqprms->fvid2handle = (void *)fctrl->fvid2handle;
    fctrl->fqprms->framelist = framelist;
    fctrl->fqprms->streamid = streamid;
    fctrl->fqprms->returnvalue = VPS_FVID2_M3_INIT_VALUE;

    fctrl->cmdprms->cmdtype = VPS_FVID2_CMDTYPE_SIMPLEX;
    fctrl->cmdprms->simplexcmdarg = (void *)fctrl->fqprms_phy;

    /* send event to proxy in M3*/
    status = Notify_sendEvent(fctrl->rmprocid,
                  fctrl->lineid,
                  fctrl->notifyno,
                  fctrl->cmdprms_phy,
                  1);

    if (status < 0) {
        VPSSERR("send Q event status 0x%08x\n", status);
        return -EINVAL;
    } else {
        stime = GetTickCount();
        etime = stime;
        while (fctrl->fqprms->returnvalue ==
            VPS_FVID2_M3_INIT_VALUE) {
            schedule();
            /*time out check*/
            if (vps_timeout) {
                etime = GetTickCount();
                td = time_diff(stime, etime);
                if (vps_timeout < td) {
                    VPSSERR("queue timeout\n");
                    return -ETIMEDOUT;
                }
            }

        }
    }

//    VPSSDBG("queue event return %d within %d ms.\n",
//        fctrl->fqprms->returnvalue,
//        td);

    return fctrl->fqprms->returnvalue;
}
EXPORT_SYMBOL(vps_fvid2_queue);

int vps_fvid2_dequeue(void *handle,
              struct fvid2_framelist *framelist,
              u32 stream_id,
              u32 timeout)
{
    struct vps_fvid2_ctrl *fctrl = (struct vps_fvid2_ctrl *)handle;
    int status;
    DWORD etime, stime;
    u32 td = 0;

    if (vps_check_fvid2_ctrl(handle)) {
        VPSSERR("DQ handle NULL.\n");
        return -EINVAL;
    }

    /*assembel the structure*/
    fctrl->fdqprms->command = VPS_FVID2_DEQUEUE;
    fctrl->fdqprms->framelist = framelist;
    fctrl->fdqprms->streamid = stream_id;
    fctrl->fdqprms->fvid2handle = (void *)fctrl->fvid2handle;
    fctrl->fdqprms->timeout = timeout;
    fctrl->fdqprms->returnvalue = VPS_FVID2_M3_INIT_VALUE;

    fctrl->cmdprms->cmdtype = VPS_FVID2_CMDTYPE_SIMPLEX;
    fctrl->cmdprms->simplexcmdarg = (void *)fctrl->fdqprms_phy;

    /* send event to proxy in M3*/
    status = Notify_sendEvent(fctrl->rmprocid,
                  fctrl->lineid,
                  fctrl->notifyno,
                  fctrl->cmdprms_phy,
                  1);

    if (status < 0) {
        VPSSERR("send DQ event status 0x%08x\n", status);
        return -EINVAL;
    } else {
        stime = GetTickCount();
        etime = stime;
        while (fctrl->fdqprms->returnvalue ==
            VPS_FVID2_M3_INIT_VALUE) {
            schedule();
            /*time out check*/
            if (vps_timeout) {
                etime = GetTickCount();
                td = time_diff(stime, etime);
                if (vps_timeout < td) {
                    VPSSDBG("dequeue timeout\n");
                    return -ETIMEDOUT;
                }
            }
        }
    }

//    VPSSDBG("dequeue event return %d within %d ms\n",
//        fctrl->fdqprms->returnvalue,
//        td);

    return fctrl->fdqprms->returnvalue;
}
EXPORT_SYMBOL(vps_fvid2_dequeue);


static int get_firmware_version(struct platform_device *pdev, u32 procid)
{
    struct vps_psrvcommandstruct  *cmdstruct;
    struct vps_payload_info  pload_info;
    int status;
    int r = -1;
    DWORD stime, etime;
    u32 td = 0;

    pload_info.size = PAGE_SIZE;
    vps_sbuf_alloc(&pload_info);

    /*get the M3 version number*/
    cmdstruct = (struct vps_psrvcommandstruct *)pload_info.vaddr;
    if (cmdstruct == NULL) {
        VPSSERR("failed to allocate version cmd struct\n");
        return -EINVAL;
    }

    /*init the structure*/
    cmdstruct->cmdtype = VPS_FVID2_CMDTYPE_SIMPLEX;
    cmdstruct->simplexcmdarg = (void *)vps_verparams_phy;

    vps_verparams->command = VPS_FVID2_GET_FIRMWARE_VERSION;
    vps_verparams->returnvalue = VPS_FVID2_M3_INIT_VALUE;

    status = Notify_sendEvent(procid,
             VPS_FVID2_PS_LINEID,
             VPS_FVID2_RESERVED_NOTIFY,
             pload_info.paddr,
             1);

    if (status < 0) {
        VPSSERR("Failed to send version command to M3 %#x.\n",
             status);
        r = -EINVAL;
        goto exit;
    } else {
    stime = GetTickCount();
    etime = stime;
        while (vps_verparams->returnvalue ==
            VPS_FVID2_M3_INIT_VALUE) {
            schedule();
            if (vps_timeout) {
                etime = GetTickCount();
                td = time_diff(stime, etime);
                if (vps_timeout  < td) {
                    VPSSERR("get version timeout\n");
                    r = -ETIMEDOUT;
                    goto exit;
                }
            }
        }
    }

    r = vps_verparams->returnvalue;
exit:
    /*release the memory*/
    vps_sbuf_free(&pload_info);
    return r;
}
static inline int get_payload_size(void)
{
    int size = 0;

    /*calculate the size of each FVID2 to allocate*/
    size  = sizeof(struct vps_psrvfvid2createparams);
    size += sizeof(struct vps_psrvfvid2deleteparams);
    size += sizeof(struct vps_psrvfvid2controlparams);
    size += sizeof(struct vps_psrvfvid2queueparams);
    size += sizeof(struct vps_psrvfvid2dequeueparams);
    size += sizeof(struct vps_psrvcallback);
    size += sizeof(struct vps_psrverrorcallback);
    size += sizeof(struct vps_psrvcommandstruct);
    size += sizeof(struct vps_psrvfvid2processframesparams);
    size += sizeof(struct vps_psrvfvid2getprocessedframesparams);
    /*size of whole parameters*/
    size *= VPS_FVID2_NUM;
    /* verion params is for whole driver not based on the FVID2*/
    size  += sizeof(struct vps_psrvgetstatusvercmdparams);

    return size;
}

static inline void assign_payload_addr(struct vps_fvid2_ctrl *fctrl,
                       struct vps_payload_info *pinfo,
                       u32 *buf_offset)
{

    /*assign the virt and phy address*/

    fctrl->fcrprms = (struct  vps_psrvfvid2createparams *)
            setaddr(pinfo,
                buf_offset,
                &fctrl->fcrprms_phy,
                sizeof(struct vps_psrvfvid2createparams));


    fctrl->fdltprms = (struct vps_psrvfvid2deleteparams *)
            setaddr(pinfo,
                buf_offset,
                &fctrl->fdltprms_phy,
                sizeof(struct vps_psrvfvid2deleteparams));

    fctrl->fctrlprms = (struct vps_psrvfvid2controlparams *)
            setaddr(pinfo,
                buf_offset,
                &fctrl->fctrlprms_phy,
                sizeof(struct vps_psrvfvid2controlparams));

    fctrl->fqprms = (struct vps_psrvfvid2queueparams *)
            setaddr(pinfo,
                buf_offset,
                &fctrl->fqprms_phy,
                sizeof(struct vps_psrvfvid2queueparams));

    fctrl->fdqprms = (struct vps_psrvfvid2dequeueparams *)
            setaddr(pinfo,
                buf_offset,
                &fctrl->fdqprms_phy,
                sizeof(struct vps_psrvfvid2dequeueparams));

    fctrl->cbprms = (struct vps_psrvcallback *)
            setaddr(pinfo,
                buf_offset,
                &fctrl->cbprms_phy,
                sizeof(struct vps_psrvcallback));

    fctrl->ecbprms = (struct vps_psrverrorcallback *)
            setaddr(pinfo,
                buf_offset,
                &fctrl->ecbprms_phy,
                sizeof(struct vps_psrverrorcallback));

    fctrl->cmdprms = (struct vps_psrvcommandstruct *)
            setaddr(pinfo,
                buf_offset,
                &fctrl->cmdprms_phy,
                sizeof(struct vps_psrvcommandstruct));

}


int vps_fvid2_init(struct platform_device *pdev, u32 timeout)
{
    int i, r;
    struct vps_fvid2_ctrl *fctrl;
    u32  procid;
    struct vps_payload_info *pinfo;
    u32 offset = 0;

    VPSSDBG("fvid2 init\n");
    vps_timeout = timeout;

    procid = MultiProc_getId("VPSS-M3");
    if (MultiProc_INVALIDID == procid) {
        VPSSERR("failed to get the VPSS-M3 processor ID.\n");
        return -EINVAL;
    }

    /*allocate payload info structure*/
    fvid2_payload_info = kzalloc(sizeof(struct vps_payload_info),
                     GFP_KERNEL);
    if (!fvid2_payload_info) {
        VPSSERR("failed to allocate payload info");
        return -ENOMEM;
    }
    pinfo = fvid2_payload_info;

    /*these buffer are shared between A8 and M3*/
    pinfo->size = get_payload_size();
    /*always on the page size*/
    pinfo->size = PAGE_ALIGN(pinfo->size);

    vps_sbuf_alloc(pinfo);
    if (pinfo->vaddr == NULL) {
        VPSSERR("alloc fvid2 shared buffer failed\n");
        r = -ENOMEM;
        goto exit;
    }

    /*init buffer to 0*/
    memset(pinfo->vaddr, 0, pinfo->size);

    vps_verparams = (struct vps_psrvgetstatusvercmdparams *)
            ((u32)pinfo->vaddr + offset);
    vps_verparams_phy = pinfo->paddr + offset;
    offset = sizeof(struct vps_psrvgetstatusvercmdparams);

    if (get_firmware_version(pdev, procid) == 0) {
        if (vps_verparams->version != CURRENT_VPS_FIRMWARE_VERSION) {
            if (vps_verparams->version <
                 CURRENT_VPS_FIRMWARE_VERSION) {
                VPSSERR("M3 firmware version 0x%x is old,"
                    "please update firmware version.\n",
                    vps_verparams->version);
                r = -EINVAL;
                goto exit;
            }

            if (vps_verparams->version >
                CURRENT_VPS_FIRMWARE_VERSION)
                VPSSDBG("M3 firmware version 0x%x is newer than 0x%x,"
                    "driver may not work properly.\n",
                    vps_verparams->version, CURRENT_VPS_FIRMWARE_VERSION);
        } else
            /* Not an error. We just want the M3 fw verson to be printed */
            VPSSERR("M3 firmware version 0x%08x.\n",
                vps_verparams->version);
    } else {
        r = -EINVAL;
        goto exit;
    }

    /*allocate the memory for various command structures*/
    for (i = 0; i < VPS_FVID2_NUM; i++) {

        fctrl = kzalloc(sizeof(struct vps_fvid2_ctrl), GFP_KERNEL);
        BUG_ON(fctrl == NULL);
        fvid2_ctrl[i] = fctrl;

        assign_payload_addr(fctrl, pinfo, &offset);

        fctrl->rmprocid = procid;
        fctrl->lineid = VPS_FVID2_PS_LINEID;
        fctrl->firm_ver = vps_verparams->version;
    }
    return 0;
exit:
    vps_fvid2_deinit(pdev);
    return r;
}

void vps_fvid2_deinit(struct platform_device *pdev)
{
    int i;
    struct vps_fvid2_ctrl *fctrl;

    VPSSDBG("fvid2 deinit\n");
    vps_timeout = 0;
    /*free shared buffer*/
    if (fvid2_payload_info->vaddr) {
        vps_sbuf_free(fvid2_payload_info);
        vps_verparams = NULL;
        vps_verparams_phy = 0;
    }

    /*free payload info*/
    kfree(fvid2_payload_info);
    fvid2_payload_info = NULL;

    /*free ctrl handle*/
    for (i = 0; i <  VPS_FVID2_NUM; i++) {
        fctrl = fvid2_ctrl[i];
        kfree(fctrl);
        fvid2_ctrl[i] = NULL;
    }
}
