;------------------------------------------------------------------------------
;
;   Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;   FILE:   xldr.s
;   
;   Provides support for booting from a NAND device connected to the 
;   NAND flash controller.
;
;------------------------------------------------------------------------------
    OPT 2                                       ; disable listing
    INCLUDE kxarm.h
    OPT 1                                       ; reenable listing
   
    TEXTAREA

    IMPORT  start

;******************************************************************************
;*
;* FUNCTION:    StartUp
;*
;* DESCRIPTION: System bootstrap function
;*
;* PARAMETERS:  None
;*
;* RETURNS:     None
;*
;******************************************************************************
 
    STARTUPTEXT
    LEAF_ENTRY StartUp

    stmdb       sp!,{lr}
    bl    start
    ldmia       sp!,{lr}
    bx    lr
    
    END

