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

    OPT 2   ; disable listing
    INCLUDE kxarm.h
    INCLUDE armmacros.s
    OPT 1   ; reenable listing
    OPT 128 ; disable listing of macro expansions


    TEXTAREA
        
; *************************************************************************
    LEAF_ENTRY OEMARMCacheMode
;++
; Routine Description:
;    Sets the C and B bits to be used to build page tables
;
; C and B bits are part of the page table entries and control write through vs. write back cache
; modes, cacheability, and write buffer use. Note that C and B bit functionality is processor 
; specific and different for the 720, 920, and SA1100. Consult the CPU hardware manual for the CPU 
; in question before altering these bit configurations!!
; This default configuration (C=B=1)works on all current ARM CPU's and gives the following behaviour
; ARM720: write through, write buffer enabled
; ARM920: write back cache mode
; SA1100: write back, write buffer enabled
;
; The four valid options are:
;   ARM_NoBits      0x00000000   ; do not use
;   ARM_CBit        0x00000008
;   ARM_BBit        0x00000004   ; do not use
;   ARM_CBBits      0x0000000C
;
; *Note:  XScale has altered the way the mini Dcache is configured.  It now uses
;         an additional bit, the 'X' bit, to designate a mini DCache configuration operation.
;
; Syntax:
;   DWORD OEMARMCacheMode(void);
;
; Arguments:
;   -- none --
;
; Return Value:
;   r0 must contain the desired C and B bit configuration. See description above for valid bit patterns.
;
; Caution: 
;   The value placed in r0 MUST be an immediate data value and NOT a predefined constant. This function
;   is called at a point in the boot cycle where the memory containing predefined constants has NOT been 
;   initialized yet. 
;--

    mov r0, #0x0000000C     ; C=1,B=1; write back, read allocate

    RETURN

    END

