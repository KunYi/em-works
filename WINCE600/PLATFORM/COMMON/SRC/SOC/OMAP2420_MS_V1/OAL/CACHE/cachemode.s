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
;-------------------------------------------------------------------------------
;
;  File: cachemode.s
;
        INCLUDE kxarm.h
        INCLUDE armmacros.s

        TEXTAREA

;
; Routine Description:
;    Sets the C and B bits to be used to build page tables
;
; C and B bits are part of the page table entries and control write through vs.
; write back cache modes, cacheability, and write buffer use. Note that C 
; and B bit functionality is processor specific and different for the 720,
; 920, and SA1100. Consult the CPU hardware manual for the CPU
; in question before altering these bit configurations!!
; This default configuration (C=B=1)works on all current ARM CPU's and gives
; the following behaviour
; ARM720: write through, write buffer enabled
; ARM920: write back cache mode
; SA1100: write back, write buffer enabled
;
; The four valid options are:
;   ARM_NoBits      0x00000000
;   ARM_CBit        0x00000008
;   ARM_BBit        0x00000004
;   ARM_CBBits      0x0000000C
;
; Syntax:
;   DWORD OEMARMCacheMode(void);
;
; Arguments:
;   -- none --
;
; Return Value:
;  r0 must contain the desired C and B bit configuration. See description above
;  for valid bit patterns.
;
; Caution:
;  The value placed in r0 MUST be an immediate data value and NOT a predefined
;  constant. This function is called at a point in the boot cycle where the 
;  memory containing predefined constants has NOT been initialized yet.
;
;
        LEAF_ENTRY OEMARMCacheMode

        mov r0, #0x0c
        ;mov r0, #0x0   

        RETURN

        END
