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
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
;

MPStartParam struc
    JumpInstr           dd ?            ; Jump Instruction
    PageDirPhysAddr     dd ?            ; Physical Address of Page directory
    tmpGDT              dd 6 dup(?)     ; temporary GDT - 3 64-bit words   
    Cr4Value            dd ?            ; value to set on CR4
    tmpGDTBase          dw 3 dup(?)     ; temporary GDTBase (3 words)
MPStartParam ends




