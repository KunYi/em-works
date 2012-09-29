;------------------------------------------------------------------------------
;
;   Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;   FILE:   usb.s
;   
;   Provides a function to write USB register to avoid Bus Arbitor bug found on MX28.
;
;------------------------------------------------------------------------------
    INCLUDE kxarm.h
    INCLUDE armmacros.s                                   ; reenable listing

    TEXTAREA

;******************************************************************************
;*
;* FUNCTION:    usb_reg_write
;*
;* DESCRIPTION: USB register write
;*
;* PARAMETERS:  r0 - register address  
;*              r1 - value be written to register
;*              r2 - to store the original value of register
;*
;* RETURNS:     None
;*
;******************************************************************************
    LEAF_ENTRY  usb_reg_write
    
    swp r2, r1, [r0]
    
    RETURN
    ENTRY_END  

    END

