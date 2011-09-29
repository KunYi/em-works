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
;//  File:  ceddkex.h
;//
;//  This file contains OMAP2420 specific ceddk extensions.
;//
;//------------------------------------------------------------------------------

dimof(x)            EQU (sizeof(x)/sizeof(x[0]))
offset(s, EQU f)        FIELD_OFFSET(s, f)
fieldsize(s, EQU f)     sizeof(((s*)0)->f)

;//------------------------------------------------------------------------------
;//
;//  Define:  IN/OUT/SET/CLRREG
;//
INREG8(x)           EQU READ_REGISTER_UCHAR(x)
OUTREG8(x, EQU y)       WRITE_REGISTER_UCHAR(x, (UCHAR)(y))
SETREG8(x, EQU y)       OUTREG8(x, INREG8(x)|(y))
CLRREG8(x, EQU y)       OUTREG8(x, INREG8(x)&~(y))

INREG16(x)          EQU READ_REGISTER_USHORT((USHORT*)(x))
OUTREG16(x, EQU y)      WRITE_REGISTER_USHORT((USHORT*)(x),(USHORT)(y))
SETREG16(x, EQU y)      OUTREG16(x, INREG16(x)|(y))
CLRREG16(x, EQU y)      OUTREG16(x, INREG16(x)&~(y))

INREG32(x)          EQU READ_REGISTER_ULONG((ULONG*)(x))
OUTREG32(x, EQU y)      WRITE_REGISTER_ULONG((ULONG*)(x), (ULONG)(y))
SETREG32(x, EQU y)      OUTREG32(x, INREG32(x)|(y))
CLRREG32(x, EQU y)      OUTREG32(x, INREG32(x)&~(y))

;//------------------------------------------------------------------------------
;//
;//  Define:  IOCTL_DDK_GET_DRIVER_IFC
;//
;//  This IOCTL code is used to obtain device driver interface pointers used for
;//  in-process calls between drivers. The input driver can contain interface
;//  GUID. The driver should fill output buffer with interface structure based
;//  on interface GUID. When input buffer is empty driver should return base
;//  interface specified by CEDDK_DRIVER_PFN (see bellow).
;//
IOCTL_DDK_GET_DRIVER_IFC        EQU \

;//------------------------------------------------------------------------------
;//
;//  Type:  CEDDK_DRIVER_PFN
;//
;//  This structure is used to obtain device driver funtion pointers used for
;//  in-process calls via IOCTL_DDK_GET_DRIVER_PFN.
;//

;//------------------------------------------------------------------------------
;//
;//  Type:  CEDDK_DRIVER_CONTEXT
;//
;//  This structure is used internaly to store CEDDK_DRIVER_PFN and other
;//  context needed for operation implementation.
;//

;//------------------------------------------------------------------------------
;//
;//  Function:  CreateDevice
;//
;//  This function is similar to CreateFile function. It will call CreateFile
;//  and DeviceIoControl with code IOCTL_DDK_GET_DRIVER_PFN. If both calls
;//  succeed function will create internal structure and it returns its pointer
;//  as handler.
;//


;// Allocate memory for our handler...

;// Get function pointers, fail when IOCTL isn't supported...
;// Get function pointers, fail when IOCTL isn't supported...

;// Save device handle


;//------------------------------------------------------------------------------
;//
;//  Function:  CloseDevice
;//
;//  This function will call CloseHandle and deallocate internal structure.
;//


;//------------------------------------------------------------------------------
;//
;//  Function:  WriteDevice
;//
;//  This function will call directly XXX_Write function based on pointer and
;//  context obtained by CreateDevice.
;//


;//------------------------------------------------------------------------------
;//
;//  Function:  ReadDevice
;//
;//  This function will call directly XXX_Read function based on pointer and
;//  context obtained by CreateDevice.
;//


;//------------------------------------------------------------------------------
;//
;//  Function:  DeviceIoCtrl
;//
;//  This function will call directly XXX_IOControl function based on pointer
;//  and context obtained by CreateDevice.
;//


;//------------------------------------------------------------------------------
;//
;//  Function:  ReadRegistryParams
;//
;//  This function initializes driver default settings from registry based on
;//  table passed as argument. Implementation in form of inline function is
;//  used as long as there is only one function and there isn't way how to
;//  simply extend ceddk.
;//
PARAM_DWORD         EQU 1
PARAM_STRING        EQU 2



;// Open registry context to read parameters

;// We first need to get name

;// Close active key

;// Open registry context to read parameters

;// For all members of array
;// First find if value is there
;// Value isn't in registry, break or use default

;// Close key

;// Release allocated memory in case of failure


;//------------------------------------------------------------------------------

    END
