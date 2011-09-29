;------------------------------------------------------------------------------
;
;   Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;   File:  K9LAG08U0M.inc
;
;   Contains definitions for K9LAG08U0M NAND 
;   flash memory device.
;
;------------------------------------------------------------------------------

CMD_READID              EQU     0x90        ; Read ID
CMD_READ                EQU     0x00        ; Read data field
CMD_READ2CYCLE          EQU     0x30        ; Read CMD second cycle
CMD_READ2               EQU     0x50        ; Read spare field
CMD_RESET               EQU     0xFF        ; Reset
CMD_ERASE               EQU     0x60        ; Erase setup
CMD_ERASE2              EQU     0xD0        ; Erase 
CMD_WRITE               EQU     0x80        ; Sequential data input
CMD_WRITE2              EQU     0x10        ; Program
CMD_STATUS              EQU     0x70        ; Read status

NAND_PAGE_CNT_LSH       EQU     (7)         ; 2^7
NAND_PAGE_SIZE_LSH      EQU     (11)        ; 2^11  
NAND_BLOCK_SIZE_LSH     EQU     (NAND_PAGE_CNT_LSH+NAND_PAGE_SIZE_LSH)        

NAND_PAGE_CNT           EQU     (1 << NAND_PAGE_CNT_LSH)       
NAND_PAGE_SIZE          EQU     (1 << NAND_PAGE_SIZE_LSH)      
NAND_BLOCK_SIZE         EQU     (1 << NAND_BLOCK_SIZE_LSH)
NAND_BLOCK_CNT          EQU     (8192) 
NAND_SPARE_SIZE         EQU     (64) 

BBI_PAGE_NUM            EQU     (1)                     ;NAND flash bad block information(BBI) page numbers
BBI_PAGE_ADDR_1         EQU     (NAND_PAGE_CNT - 1)     ;NAND flash bbi page address
BBI_PAGE_ADDR_2         EQU     (NAND_PAGE_CNT - 1)     ;NAND flash bbi page address

NUM_OF_NAND_DEVICES     EQU     1           ;  Number of NAND device
NUM_OF_NAND_DEVICES_LSH EQU     0           ;       

NAND_BUS_WIDTH          EQU     8           ; 8-bit bus

    END
