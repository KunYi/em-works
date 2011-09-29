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
;++
;
; Module Name:
;
;    idle.s
;
; Abstract:
;
;    This module implements Bulverde cpuidle code.
;
; Environment:
;
; Revision History:
;
; Notes:
;

;-------------------------------------------------------------------------------
;
 
    ; Disable listing
    OPT    2

    INCLUDE kxarm.h
   
    ; Re-enable listing 
    OPT 1
  
    TEXTAREA
   
;*******************************************************************************
;
;   CPUEnterIdle() is called by OALCPUIdle to put the CPU in idle mode
;    
;*******************************************************************************

    LEAF_ENTRY CPUEnterIdle
    
; Enter idle mode
    ldr     r0, =0x01                               ; 1 = Idle Mode
    mcr   p14, 0, r0, c7, c0, 0               ; Enter Idle mode


  IF Interworking :LOR: Thumbing
    bx  lr
  ELSE
    mov  pc, lr          ; return to caller.
  ENDIF
 

    ENTRY_END
;-------------------------------------------------------------------------------

    LTORG                           ; insert a literal pool here.

    END

