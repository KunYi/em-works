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
;;------------------------------------------------------------------------------
;;
;;  Copyright(c) 1998,1999 Renesas Technology Corp.
;;
;;------------------------------------------------------------------------------
;;
;;	File:   sh4_rtc.inc
;;
;;------------------------------------------------------------------------------

;
; Real Time Clock control registers 
;

RTC_REGBASE                 .equ    h'FFC80000
RTC_REGSIZE                 .equ    h'0040

RTC_R64CNT_OFFSET           .equ    h'0000
RTC_RSECCNT_OFFSET          .equ    h'0004
RTC_RMINCNT_OFFSET          .equ    h'0008
RTC_RHRCNT_OFFSET           .equ    h'000C
RTC_RWKCNT_OFFSET           .equ    h'0010
RTC_RDAYCNT_OFFSET          .equ    h'0014
RTC_RMONCNT_OFFSET          .equ    h'0018
RTC_RYRCNT_OFFSET           .equ    h'001C
RTC_RSECAR_OFFSET           .equ    h'0020
RTC_RMINAR_OFFSET           .equ    h'0024
RTC_RHRAR_OFFSET            .equ    h'0028
RTC_RWKAR_OFFSET            .equ    h'002C
RTC_RDAYAR_OFFSET           .equ    h'0030
RTC_RMONAR_OFFSET           .equ    h'0034
RTC_RCR1_OFFSET             .equ    h'0038
RTC_RCR2_OFFSET             .equ    h'003c

RTC_R64CNT                  .equ    RTC_REGBASE + RTC_R64CNT_OFFSET
RTC_RSECCNT                 .equ    RTC_REGBASE + RTC_RSECCNT_OFFSET
RTC_RMINCNT                 .equ    RTC_REGBASE + RTC_RMINCNT_OFFSET
RTC_RHRCNT                  .equ    RTC_REGBASE + RTC_RHRCNT_OFFSET
RTC_RWKCNT                  .equ    RTC_REGBASE + RTC_RWKCNT_OFFSET
RTC_RDAYCNT                 .equ    RTC_REGBASE + RTC_RDAYCNT_OFFSET
RTC_RMONCNT                 .equ    RTC_REGBASE + RTC_RMONCNT_OFFSET
RTC_RYRCNT                  .equ    RTC_REGBASE + RTC_RYRCNT_OFFSET
RTC_RSECAR                  .equ    RTC_REGBASE + RTC_RSECAR_OFFSET
RTC_RMINAR                  .equ    RTC_REGBASE + RTC_RMINAR_OFFSET
RTC_RHRAR                   .equ    RTC_REGBASE + RTC_RHRAR_OFFSET
RTC_RWKAR                   .equ    RTC_REGBASE + RTC_RWKAR_OFFSET
RTC_RDAYAR                  .equ    RTC_REGBASE + RTC_RDAYAR_OFFSET
RTC_RMONAR                  .equ    RTC_REGBASE + RTC_RMONAR_OFFSET
RTC_RCR1                    .equ    RTC_REGBASE + RTC_RCR1_OFFSET
RTC_RCR2                    .equ    RTC_REGBASE + RTC_RCR2_OFFSET

; Second Alarm Register (RSECAR)

RTC_RSECAR_ENB              .equ    h'80

; Minute Alarm Register (RMINAR)

RTC_RMINAR_ENB              .equ    h'80

; Hour Alarm Register (RHRAR)

RTC_RHRAR_ENB               .equ    h'80

; Day of Week Alarm Register (RWKAR)

RTC_RWKAR_ENB               .equ    h'80

; Day Alarm Register (RDAYAR)

RTC_RDAYAR_ENB              .equ    h'80

; Month Alarm Register (RMONAR)

RTC_RMONAR_ENB              .equ    h'80

; RTC Control Register1 (RCR1)

RTC_RCR1_CF                 .equ    h'80
RTC_RCR1_CIE                .equ    h'10
RTC_RCR1_AIE                .equ    h'08
RTC_RCR1_AF                 .equ    h'01

; RTC Control Register2 (RCR2)

RTC_RCR2_PEF                .equ    h'80
RTC_RCR2_RES_2              .equ    h'70    ;Periodic interrupt generated every 2 second  
RTC_RCR2_RES_1              .equ    h'60    ;Periodic interrupt generated every 1 second  
RTC_RCR2_RES_1_2            .equ    h'50    ;Periodic interrupt generated every 1/2 second  
RTC_RCR2_RES_1_4            .equ    h'40    ;Periodic interrupt generated every 1/4 second  
RTC_RCR2_RES_1_16           .equ    h'30    ;Periodic interrupt generated every 1/16 second  
RTC_RCR2_RES_1_64           .equ    h'20    ;Periodic interrupt generated every 1/64 second  
RTC_RCR2_RES_1_256          .equ    h'10    ;Periodic interrupt generated every 1/256 second  
RTC_RCR2_RES_NO_PINT        .equ    h'00    ;NO periodic interrupt generation
RTC_RCR2_RTCEN              .equ    h'08
RTC_RCR2_ADJ                .equ    h'04
RTC_RCR2_RESET              .equ    h'02
RTC_RCR2_START              .equ    h'01

