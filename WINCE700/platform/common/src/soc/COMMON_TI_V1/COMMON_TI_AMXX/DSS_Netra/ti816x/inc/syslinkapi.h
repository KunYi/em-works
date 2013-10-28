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

#ifndef _SYSLINKAPI_H_
#define _SYSLINKAPI_H_


/* Syslink api */

#if defined (__cplusplus)
extern "C" {
#endif

#define MultiProc_INVALIDID             (0xFFFF)

/* =============================================================================
 *  APIs. The only apis that are used by the display driver are abstracted here.
 *  This helps avoid having to install packages such as IPC and SYSLINK to build
 *  display driver. 
 * =============================================================================
 */
/*  MultiProc apis. These have to match the corresponding apis in IPC multiproc.h */
unsigned short MultiProc_getId(char * name);

/* Notify APIs. These have to match the corresponding apis in IPC notify.h */

typedef void (*Notify_FnNotifyCbck)(unsigned short, unsigned short, unsigned int, void *, unsigned int);

int Notify_registerEvent(unsigned short procId, 
                         unsigned short lineId, 
                         unsigned int   eventId,
                         Notify_FnNotifyCbck fnNotifyCbck, 
                         void * cbckArg);

int Notify_unregisterEvent(unsigned short procId, unsigned short lineId, unsigned int eventId,
                           Notify_FnNotifyCbck fnNotifyCbck, void * cbckArg);

int Notify_sendEvent(unsigned short procId, 
                     unsigned short lineId,
                     unsigned int   eventId, 
                     unsigned int   payload,
                     bool waitClear);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* ifndef _SYSLINK_API_H_ */
