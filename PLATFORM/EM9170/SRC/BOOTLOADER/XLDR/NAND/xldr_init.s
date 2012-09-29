;------------------------------------------------------------------------------
;
;   Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;   FILE:   xldr_init.s
;   
;   First instruction to be executed is this jump instruction, control 
;   is passed to the entry point of xloader from this function
;   This object file should be located at first address of SD card
;------------------------------------------------------------------------------

CSF_SIZE        EQU     0x3FC

LOAD_ADDR       EQU     0x78001B00

    MACRO
    RESERVE_CSF        $size
        LCLA    Counter
Counter        SETA    $size
        WHILE   Counter > 0
Counter        SETA    Counter - 1
            DCB 0x00 
        WEND
    MEND

    OPT 2                               ; disable listing
    INCLUDE kxarm.h
    OPT 1                               ; reenable listing
   
    TEXTAREA
    IMPORT StartUp                      ; Entry point for Xloader

    AREA .asecure,ALIGN=4,CODE
    EXPORT XLDR_CSF_DATA
    LEAF_ENTRY StartUp_Init

    b StartUp                           ; Jump to the XLDR entry 

XLDR_CSF_DATA
    RESERVE_CSF  CSF_SIZE               ; Reserve space for CSF data

; The following header is required for internal boot mode and must be defined at offset 0x400 from start of application.
; The previous XLDER_CSF_DATA is of size 0x3FC. It ensures that the following header is loaded at 0x400.
; This header is only valid for HAB_TYPE = engineering. The XLDR will fail to load for all other HAB_TYPE settings
XLDR_FLASH_HDR
    DCD (StartUp_Init + LOAD_ADDR)      ; image entry point
    DCD 0xB1                            ; barker_code
    DCD 0x0                             ; pointer to CSF (NULL)
    DCD (XLDR_DCD_PTR + LOAD_ADDR)      ; pointer to DCD pointer 
    DCD 0x0                             ; ptr to SRK struct (NULL)

XLDR_DCD_PTR
    DCD (XLDR_DCD_DATA + LOAD_ADDR)

XLDR_DEST_PTR
    DCD LOAD_ADDR

XLDR_DCD_DATA
    DCD 0xB17219E9                      ; barker code for DCD
    DCD 0x0                               ; length of DCD data
    
    ; length of XLDR image (4K)
    DCD 0x1000
    
    END 
