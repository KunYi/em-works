;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this sample source code is subject to the terms of the Microsoft
; license agreement under which you licensed this sample source code. If
; you did not accept the terms of the license agreement, you are not
; authorized to use this sample source code. For the terms of the license,
; please see the license agreement between you and Microsoft or, if applicable,
; see the LICENSE.RTF on your install media or the root of your tools installation.
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
;
;//
;//
;//
;// Use of this source code is subject to the terms of the Microsoft end-user
;// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
;// If you did not accept the terms of the EULA, you are not authorized to use
;// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
;// install media.
;//
;//------------------------------------------------------------------------------
;//
;//  File:  clk.h
;//
;//  This file specifies clock module interface. It controls CLKM and ULPD
;//  hardware modules.
;//
;//------------------------------------------------------------------------------

CLK_DEVICE_NAME         EQU L"CLK1:"

;//------------------------------------------------------------------------------
;//
;//  GUID:  DEVICE_IFC_CLK_GUID
;//

;//------------------------------------------------------------------------------
;//
;//  Type:  DEVICE_IFC_CLK
;//
;//  This structure is used to obtain GPIO interface funtion pointers used for
;//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
;//

;//------------------------------------------------------------------------------
;//
;//  Type:  DEVICE_CONTEXT_CLK
;//
;//  This structure is used to store CLK device context.
;//

;//------------------------------------------------------------------------------

IOCTL_CLK_ATTACH        EQU \

IOCTL_CLK_DETACH        EQU \

IOCTL_CLK_REQUEST       EQU \

IOCTL_CLK_RELEASE       EQU \

IOCTL_CLK_FREQUENCY     EQU \

;//------------------------------------------------------------------------------
;//
;//  Function:  ClkOpen
;//
;//  This function opens clock module context for given device. Device is
;//  specified by its location.
;//

;// Open device

;// Allocate memory for our handler...

;// Save device handle

;// Get function pointers, fail when IOCTL isn't supported...

;// Attach open context to device


;//------------------------------------------------------------------------------
;//
;//  Function:  ClkClose
;//

;// Deattach device from context
;// Close open instance
;// Deallocate wrapper context

;//------------------------------------------------------------------------------
;//
;//  Function:  ClkRequest
;//
;//  This function is called by device driver to request/enable clock for
;//  driven hardware. In case that hardware has multiple clock sources, this
;//  function should be called multiple times with diffent clock id. Id starts
;//  from 0.
;//

;//------------------------------------------------------------------------------
;//
;//  Function:  ClkRelease
;//
;//  This function is called by device driver to release clock request for
;//  driven hardware. In case that hardware has multiple clock sources, this
;//  function should be called multiple times with diffent clock id's.
;//

;//------------------------------------------------------------------------------
;//
;//  Function:  ClkGetFrequency
;//
;//  This function returns clock frequency delivered to device.
;//

;//------------------------------------------------------------------------------
    END
