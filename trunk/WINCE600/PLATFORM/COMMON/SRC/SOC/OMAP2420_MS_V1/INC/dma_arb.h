//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
#ifndef __DMA_ARB_H
#define __DMA_ARB_H

/*----------------------------------------------------------------------------------------*

DMA channel arbiter for omap2420
Kurt Kennett
Intrinsyc Software International, Inc.

There can be multiple DMA controllers on a system, each with individual unique properties.

This API simply allows a user of a DMA controller to acquire free channels on a controller
and to configure specific aspects of the controller in a controlled fashion.

A user of a DMA controller must itself map the controller's registers, but should only use
the assigned channel(s) on the controller that it aquires via this API.  To modify DMA controller
settings that are not specific to an allocated channel, the user should employ the
DMA_ControllerSet() function to guarantee controlled access.

The typical usage of this API is as follows:

DWORD dwSize=0;
DMA_ControllerEnum(NULL,&dwSize);
DMACONTROLLER *pEnum = (DMACONTROLLER *)malloc(dwSize);
DMA_ControllerEnum(pEnum,&dwSize);
<< find the controller you want in the pEnum array of controller id/name pairs >>

HANDLE hController;
DMA_ControllerOpen(<id of controller you want>, &hController);
DMA_ControllerAcquireChannels(hController, <# of channels desired>, <mask of *required* channels/return mask of channels acquired>);
DMA_ControllerFreeChannels(hController, <mask of channels being freed>);
DMA_ControllerSet(hController, <property id>, <property value>);
DMA_ControllerGet(hController, <property id>, <property value>);
DMA_ControllerClose(hController);

This allows full unrestricted low-level access to the DMA controllers, but provides a
marshalling entity for channel usage.

*-----------------------------------------------------------------------------------------*/

#include <windows.h>
#include "dma_arbdefs.h"

/* if using implicit linking, then don't do anything and link your driver to
   dmaarb.lib.  to link explictly, you need to define DMA_LINK_EXPLICIT before 
   including this file */
#define DMA_IMPORT
#ifndef DMA_C
#ifndef DMA_LINK_EXPLICIT
#undef DMA_IMPORT
#define DMA_IMPORT __declspec(dllimport)
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------------------*/
/*                                  error codes                                           */
/*----------------------------------------------------------------------------------------*/

#define DMA_FACILITY            0x47
#define DMA_ERROR               0x80000000
#define DMA_WARNING             0x40000000
#define DMA_MAKEERR(x)          ((DMA_ERROR | (DMA_FACILITY<<16)) | (x))
#define DMA_MAKEWARN(x)         ((DMA_WARNING | (DMA_FACILITY<<16)) | (x))
#define DMA_FAILED(x)           (((x)&DMA_ERROR)!=0)

#define DMAERR_NOTENOUGHSPACE           DMA_MAKEERR(0x0001)
    /* not enough space has been alocated to hold the return buffer */
#define DMAERR_CONTROLLERNOTFOUND       DMA_MAKEERR(0x0002)
    /* dma controller referenced by id # or handle was not found */
#define DMAERR_RESOURCES                DMA_MAKEERR(0x0003)
    /* not enough resources to complete the command */
#define DMAERR_PROPERTY_READONLY        DMA_MAKEERR(0x0004)
    /* the property specified is read-only.  you cannot change it */
#define DMAERR_BADVALUE                 DMA_MAKEERR(0x0005)
    /* value specified or its size is not valid for the selected property */
#define DMAERR_NODRIVER                 DMA_MAKEERR(0x0006)
    /* no dma driver found on system */
#define DMAERR_BADPOINTER               DMA_MAKEERR(0x0007)
    /* bad pointer passed to function */
#define DMAERR_BADCHANMASK              DMA_MAKEERR(0x0008)
    /* channels specified by mask are not valid or not correct */
#define DMAERR_BADNUMCHANNELS           DMA_MAKEERR(0x0009)
    /* # of channels to acquire is outside of allowable range */
#define DMAERR_BADPROPERTY              DMA_MAKEERR(0x000A)
    /* unknown or invalid property specified */


/*----------------------------------------------------------------------------------------*/
/*                                controller operations                                   */
/*----------------------------------------------------------------------------------------*/

DMA_IMPORT uint DMA_ControllerEnum(DMACONTROLLER *apRetArray, uint *apSizeBytes);
/* 
DMA_ControllerEnum()
returns: uint: error code or 0 for success.
parameters:
DMAController *apRetArray
    return buffer for DMACONTROLLER definition
uint *apSizeBytes
    input - size of "apRetArray" buffer in bytes
    output - size of data returned in "apRetArray" buffer, or size required if insufficient space provided
error codes:
    DMAERR_NOTENOUGHSPACE - not enough space available to hold enumeration
    DMAERR_BADPOINTER - a bad pointer address was used for an argument.
    DMAERR_NODRIVER - no dma driver found on system
remarks:
    To get the size required to return an array of all controllers on the system, pass NULL for
    the 'apRetArray' parameter.  This will fill in the argument indicated by 'apSizeBytes' with
    the size of buffer required to return the information.
*/

#define DMA_DEFAULT_CONTROLLER  0
DMA_IMPORT uint DMA_ControllerOpen(uint aSysId, HANDLE *apRetHandle);
/*
DMA_ControllerOpen()
returns: uint: error code or 0 for success.
parameters:
uint aSysId
    system Id of controller to open.  Use 0 for the system default controller, or use
    the DMA_ControllerEnum() function to get a list of the other controllers on the system.
HANDLE *apRetHandle
    used to return the handle to the controller when it has been successfully opened.
error codes:
    DMAERR_CONTROLLERNOTFOUND - controller referenced by id was not found.
    DMAERR_BADPOINTER - a bad pointer address was used for an argument.
    DMAERR_NODRIVER - no dma driver found on system
*/

DMA_IMPORT uint DMA_ControllerAcquireChannels(HANDLE aController, uint aNumChannels, uint *apChanIO);
/*
DMA_ControllerAcquireChannels()
returns: uint: error code or 0 for success
parameters:
HANDLE aController
    handle to controller opened with DMA_ControllerOpen() function
uint aNumChannels
    number of channels required to be allocated.
uint *apChanIO
    input - mask of *required* channel numbers.  if the channels are already in use
            then this acquisition will fail and no channels will be allocated (DMAERR_BADCHANMASK).
    output - mask of allocated channels (0-31).
error codes:
    DMAERR_CONTROLLERNOTFOUND - invalid 'aController' argument used
    DMAERR_PROPUNKNOWN - unknown or invalid property specified
    DMAERR_BADCHANMASK - channel mask is invalid (channels not owned by this user of DMA)
*/

DMA_IMPORT uint DMA_ControllerFreeChannels(HANDLE aController, uint aChanFreeMask);
/*
DMA_ControllerFreeChannels()
returns: uint: error code or 0 for success
parameters:
HANDLE aController
    handle to controller opened with DMA_ControllerOpen() function
uint aChanFreeMask
    mask of channels held by this driver/process that are to be freed.  the mask of
    channels for an acquisition is returned in the *apChanIO parameter when the user
    calls the DMA_ControllerAcquireChannels() function.
error codes:
    DMAERR_CONTROLLERNOTFOUND - invalid 'aController' argument used
    DMAERR_PROPUNKNOWN - unknown or invalid property specified
    DMAERR_BADCHANMASK - channel mask is invalid (channels not owned by this user of DMA)
*/

DMA_IMPORT uint DMA_ControllerSet(HANDLE aController, DMA_CONT_PROPERTY aProp, uint aValue32);
/*
DMA_ControllerSet()
returns: uint: error code or 0 for success.
parameters:
HANDLE aController
    handle to controller opened with DMA_ControllerOpen() function
DMA_CONT_PROPERTY aProp
    property to set the value of
uint aValue32
    32-bit value to set for the property (contents vary)
error codes:
    DMAERR_CONTROLLERNOTFOUND - invalid 'aController' argument used
    DMAERR_PROPUNKNOWN - unknown or invalid property specified
    DMAERR_BADPOINTER - a bad pointer address was used for an argument
*/

DMA_IMPORT uint DMA_ControllerGet(HANDLE aController, DMA_CONT_PROPERTY aProp, uint *apRetValue32);
/*
DMA_ControllerGet()
returns: uint: error code or 0 for success.
parameters:
HANDLE aController
    handle to controller opened with DMA_ControllerOpen() function
DMA_CONT_PROPERTY aProp
    property to retrieve the value of 
uint *apRetValue32
    storage to hold retrieved property value
error codes:
    DMAERR_CONTROLLERNOTFOUND - invalid 'aController' argument used
    DMAERR_PROPUNKNOWN - unknown or invalid property specified
    DMAERR_BADVALUE - value passed for property is not valid.
*/

DMA_IMPORT uint DMA_ControllerClose(HANDLE aController);
/*
DMA_ControllerClose()
returns: uint: error code or 0 for success.
parameters:
HANDLE aController
    handle to controller opened with DMA_ControllerOpen() function
error codes:
    DMAERR_CONTROLLERNOTFOUND - invalid 'aController' argument used
*/

#ifdef __cplusplus
};
#endif

#endif // __DMA_ARB_H
