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
;-------------------------------------------------------------------------------
;
;   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
;   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
;   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
;   PARTICULAR PURPOSE.
;
;-------------------------------------------------------------------------------
.486p

;
; for assembly constants
;
include kxx86.inc

OFFSET32 EQU <OFFSET FLAT:>


        ASSUME  CS: FLAT, DS: FLAT, SS: FLAT


;vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
_TEXT SEGMENT para public 'TEXT'
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

        public      _OEMAddressTable
        public      _g_oalAddressTable
        public      _dwOEMTotalRAM

extrn _IdentifyCpu:near
extrn _KernelStart:near

;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
_StartUp PROC NEAR PUBLIC
        cli

        ;
        ; The following strange code is used to get the Physical Address of _PAOfStart.
        ; The trick is that "call" will pushed the return address onto stack, which will
        ; be the the Physical address of _PaStart. We then pop the stack to get the Physical
        ; address.
        ;
        call    _IdentifyCpu
        mov     edi, eax    ; argument to _KernelInitialize, edi = cpu capability bits

        call    _PAOfStart
_PAOfStart:
        pop     esi         ; (esi) = PA of _PAStart

        add     esi, OFFSET32 _OEMAddressTable
        sub     esi, OFFSET32 _PAOfStart         ; (esi) = PA of OEMAddressTable

        ; argument to KernelInitialize:
        ; (esi) = Physical Address of OEMAddressTable
        ; (edi) = CPU capability bits
        ;
        jmp  _KernelStart

        align 4
_OEMAddressTable:

        ;
        ; Uncomment the following 'dd' line to use the new address table format.
        ;
        ; The 1st entry have the format of {CE_NEW_MAPPING_TABLE, OEMDeviceTable, 0}
        ; if new table format is used.
        ; 
        ; - CEPC doesn't have a device table, thus 2nd entry is NULL
        ; - When using new mapping table, NO UNCACHED ADDRESS WILL BE MAPPED.
        ;
IFDEF NEW_MAPPING_TABLE
        dd    CE_NEW_MAPPING_TABLE, 0, 0
ENDIF

_g_oalAddressTable:
        ; 
        ; OEMAddressTable defines the mapping between Physical and Virtual Address
        ;   o MUST be in a READONLY Section
        ;   o First Entry MUST be RAM, mapping from 0x80000000 -> 0x00000000
        ;   o each entry is of the format ( VA, PA, cbSize )
        ;   o cbSize must be multiple of 4M
        ;   o last entry must be (0, 0, 0)
        ;   o must have at least one non-zero entry

        ; RAM 0x80000000 -> 0x00000000, size 64M
        ;
        ; NOTE: the 1st entry is splitted into 2 lines, such that the size itself
        ;       can be referenced directly. This way we can use fixupvar to change
        ;       it at makeimg time.
        ;
        dd  80000000h,      0
_dwOEMTotalRAM:
        dd                              04000000h

        ; FLASH and other memory, if any
        ; dd  FlashVA,      FlashPA,    FlashSize

        ; Last entry, all zeros
        dd  0,              0,          0
_StartUp ENDP


_TEXT ENDS


        END

