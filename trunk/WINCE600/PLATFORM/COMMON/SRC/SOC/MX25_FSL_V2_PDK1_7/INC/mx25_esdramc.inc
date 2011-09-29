;;------------------------------------------------------------------------------
;;
;; Copyright (c) Microsoft Corporation.  All rights reserved.
;;
;;
;; Use of this source code is subject to the terms of the Microsoft end-user
;; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
;; If you did not accept the terms of the EULA, you are not authorized to use
;; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
;; install media.
;;
;;------------------------------------------------------------------------------
;;
;; Copyright (C) 2007-2008 Freescale Semiconductor, Inc. All Rights Reserved.
;; THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;; AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;;
;;------------------------------------------------------------------------------
;;
;; Header:  mx25_esdramc.inc
;;
;; Provides definitions for ESDRAMC module on MX25.
;;
;;------------------------------------------------------------------------------

;;------------------------------------------------------------------------------
;; GENERAL MODULE CONSTANTS
;;------------------------------------------------------------------------------


;;------------------------------------------------------------------------------
;; REGISTER LAYOUT
;;------------------------------------------------------------------------------


;;------------------------------------------------------------------------------
;; REGISTER OFFSETS
;;------------------------------------------------------------------------------
ESDRAMC_ESDCTL0_OFFSET          EQU         0x0000
ESDRAMC_ESDCFG0_OFFSET          EQU         0x0004
ESDRAMC_ESDCTL1_OFFSET          EQU         0x0008
ESDRAMC_ESDCFG1_OFFSET          EQU         0x000C
ESDRAMC_ESDMISC_OFFSET          EQU         0x0010
ESDRAMC_ESDCDLY1_OFFSET         EQU         0x0020
ESDRAMC_ESDCDLY2_OFFSET         EQU         0x0024
ESDRAMC_ESDCDLY3_OFFSET         EQU         0x0028
ESDRAMC_ESDCDLY4_OFFSET         EQU         0x002C
ESDRAMC_ESDCDLY5_OFFSET         EQU         0x0030
ESDRAMC_ESDCDLYL_OFFSET         EQU         0x0034



;;------------------------------------------------------------------------------
;; REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
;;------------------------------------------------------------------------------



;;------------------------------------------------------------------------------
;; REGISTER BIT FIELD WIDTHS
;;------------------------------------------------------------------------------




;;------------------------------------------------------------------------------
;; REGISTER BIT WRITE VALUES
;;------------------------------------------------------------------------------



;;------------------------------------------------------------------------------
;; HELPER MACROS
;;------------------------------------------------------------------------------


    END
