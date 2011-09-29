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
;// Portions Copyright (c) Texas Instruments.  All rights reserved.
;//
;//------------------------------------------------------------------------------
;//
;//  File:  omap2420.h
;//
;//  This header file is comprised of component header files that define
;//  the register layout of each System on Chip (SoC) component.

;//////////////////////////////////////////////////////////////////////
;// TIMERS
;//////////////////////////////////////////////////////////////////////

;//
;// General Purpose Timers.
;//

;// Offset of registers of general purpose timer.
OMAP2420_GP_TIMER_TIDR_OA        EQU 0x0
OMAP2420_GP_TIMER_TIOCP_CFG_OA   EQU 0x10
OMAP2420_GP_TIMER_TISTAT_OA      EQU 0x14
OMAP2420_GP_TIMER_TISR_OA        EQU 0x18
OMAP2420_GP_TIMER_TIER_OA        EQU 0x1C
OMAP2420_GP_TIMER_TWER_OA        EQU 0x20
OMAP2420_GP_TIMER_TCLR_OA        EQU 0x24
OMAP2420_GP_TIMER_TCRR_OA        EQU 0x28
OMAP2420_GP_TIMER_TLDR_OA        EQU 0x2C
OMAP2420_GP_TIMER_TTGR_OA        EQU 0x30
OMAP2420_GP_TIMER_TWPS_OA        EQU 0x34
OMAP2420_GP_TIMER_TMAR_OA        EQU 0x38
OMAP2420_GP_TIMER_TCAR1_OA       EQU 0x3C
OMAP2420_GP_TIMER_TSICR_OA       EQU 0x40
OMAP2420_GP_TIMER_TCAR2_OA       EQU 0x44

TIMER_PERIOD                      EQU 1
OMAP2420_GPTIMER1_COUNTS_PER_1MS  EQU 33


;// Watchdog timer register offsets.
OMAP2420_WDOG_WIDR_OA            EQU 0
OMAP2420_WDOG_SYSCONFIG_OA       EQU 0x10
OMAP2420_WDOG_SYSSTATUS_OA       EQU 0x14
OMAP2420_WDOG_WISR_OA            EQU 0x18
OMAP2420_WDOG_WIER_OA            EQU 0x1C
OMAP2420_WDOG_WCLR_OA            EQU 0x24
OMAP2420_WDOG_WCRR_OA            EQU 0x28
OMAP2420_WDOG_WLDR_OA            EQU 0x2C
OMAP2420_WDOG_WTGR_OA            EQU 0x30
OMAP2420_WDOG_WWPS_OA            EQU 0x34
OMAP2420_WDOG_WSPR_OA            EQU 0x48

OMAP2420_WDOG_DISABLE_SEQ1           EQU 0xAAAA
OMAP2420_WDOG_DISABLE_SEQ2           EQU 0x5555

;// 32-kHz Sync Timer register offsets.
OMAP2420_TIMER32KHz_32KSYNCNT_REV_OA EQU 0
OMAP2420_TIMER32KHz_CR_OA              EQU 0x10

;// Frame Adjustment Counter (FAC) register offsets.
OMAP2420_FAC_usFARC_OA           EQU 0x0
OMAP2420_FAC_usFSC_OA            EQU 0x4
OMAP2420_FAC_usCTRL_OA           EQU 0x8
OMAP2420_FAC_ususSTATUS_OA       EQU 0xC
OMAP2420_FAC_usSYNC_CNT_OA       EQU 0x10
OMAP2420_FAC_usSTART_CNT_OA      EQU 0x14
OMAP2420_FAC_usSYSCONFIG_OA      EQU 0x18
OMAP2420_FAC_usREVISION_OA       EQU 0x1C
OMAP2420_FAC_usSYSSTATUS_OA      EQU 0x20

    END
