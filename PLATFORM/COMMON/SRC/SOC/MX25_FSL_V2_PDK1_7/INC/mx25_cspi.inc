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
;; Header:  mx25_cspi.inc
;;
;; Provides definitions for CSPI module on MX25.
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
CSPI_RXDATA_OFFSET            EQU     (0x00)
CSPI_TXDATA_OFFSET            EQU     (0x04)
CSPI_CONREG_OFFSET            EQU     (0x08)
CSPI_INTREG_OFFSET            EQU     (0x0C)
CSPI_DMAREG_OFFSET            EQU     (0x10)
CSPI_STATREG_OFFSET           EQU     (0x14)
CSPI_PERIODREG_OFFSET         EQU     (0x18)
CSPI_TESTREG_OFFSET           EQU     (0x1C)


;;------------------------------------------------------------------------------
;; REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
;;------------------------------------------------------------------------------



;;------------------------------------------------------------------------------
;; REGISTER BIT FIELD WIDTHS
;;------------------------------------------------------------------------------




;;------------------------------------------------------------------------------
;; REGISTER BIT WRITE VALUES
;;------------------------------------------------------------------------------
CSPI_CONREG_ENABLE          EQU     (1 << 0)
CSPI_CONREG_XCHG            EQU     (1 << 2)

CSPI_STATREG_TRANS_COMPLETE EQU     (1 << 7)
CSPI_STATREG_TXFULL         EQU     (1 << 2)
CSPI_STATREG_RXRDY          EQU     (1 << 3)

;;------------------------------------------------------------------------------
;; HELPER MACROS
;;------------------------------------------------------------------------------


    END
