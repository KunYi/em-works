;------------------------------------------------------------------------------
;
;   Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;   File:  cspichip.inc
;
;   Contains definitions for XLDR access of the MX25L1605D SPI flash.
;
;------------------------------------------------------------------------------

    
CSPI_SECTOR_NUM_LSH     EQU     (4)                             ; 2^4
CSPI_SECTOR_SIZE_LSH    EQU     (12)                            ; 2^12  
CSPI_BLOCK_SIZE_LSH     EQU     (16)                            ; 2^16

CSPI_BLOCK_NUM          EQU     (32)                            ; 32 blocks
CSPI_SECTOR_NUM         EQU     (1 << CSPI_SECTOR_NUM_LSH)      ; Each Block has 16 Sectors
CSPI_SECTOR_SIZE        EQU     (1 << CSPI_SECTOR_SIZE_LSH)     ; Each Sector has 4096 Bytes    
CSPI_BLOCK_SIZE         EQU     (1 << CSPI_BLOCK_SIZE_LSH)      


CMD_READ                EQU     (0x03)                  ; Read data command

CMD_READ_LEN            EQU     (0x04)                  ; length of Read command + 24 bit address in bytes
    END
