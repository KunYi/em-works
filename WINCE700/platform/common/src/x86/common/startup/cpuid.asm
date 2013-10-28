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


OFFSET32 EQU <OFFSET FLAT:>


        ASSUME  CS: FLAT, DS: FLAT, SS: FLAT



;vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
_TEXT SEGMENT para public 'TEXT'
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

        public      _IdentifyCpu


;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
_IdentifyCpu PROC NEAR                  ; COMDAT
        push    ebx
        cli

        xor     edx, edx                ; initialize return value to 0

        pushfd                          ; Save EFLAGS to stack
        pop     eax                     ; Store EFLAGS in EAX
        mov     ecx, eax                ; Save in ECX for testing later
        xor     eax, 00200000h          ; Switch bit 21
        push    eax                     ; Copy changed value to stack
        popfd                           ; Save changed EAX to EFLAGS
        pushfd                          ; Push EFLAGS to top of stack
        pop     eax                     ; Store EFLAGS in EAX
        cmp     eax, ecx                ; See if bit 21 has changed
        jz      cpuid_ret               ; If no change,no CPUID

        ; call cpuid with eax == 1 to get capability info
        mov     eax, 1
        DB      00fH
        DB      0a2H

cpuid_ret:
        mov     eax, edx
        pop     ebx
        ret     0
_IdentifyCpu ENDP



_TEXT ENDS


        END

