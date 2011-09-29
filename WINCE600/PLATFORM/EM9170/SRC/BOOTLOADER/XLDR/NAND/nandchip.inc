;------------------------------------------------------------------------------
;
;   Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;   File:  nandchip.inc
;
;   Contains definitions of NAND flash for XLDR access.
;
;------------------------------------------------------------------------------

    ; Include definitions for selected NAND flash device
    IF :DEF: BSP_NAND_K9LBG08U0M
        INCLUDE K9LBG08U0M.inc
    ELSE
        IF :DEF: BSP_NAND_K9LBG08U0D
            INCLUDE K9LBG08U0D.inc
        ELSE
			IF :DEF: BSP_NAND_K9F2G08U0A
				INCLUDE K9F2G08U0A.inc
			ELSE
				IF :DEF: BSP_NAND_K9F1G08U0A
					INCLUDE K9F1G08U0A.inc
				ELSE
					INCLUDE K9LAG08U0M.inc
				ENDIF
			ENDIF
        ENDIF
    ENDIF   
        
IMAGE_XLDR_NAND_BLOCK_OFFSET		EQU     (0)
IMAGE_XLDR_NAND_BLOCK_SIZE				EQU     (IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET >> NAND_BLOCK_SIZE_LSH)
IMAGE_EBOOT_NAND_BLOCK_OFFSET		EQU     (IMAGE_XLDR_NAND_BLOCK_OFFSET+IMAGE_XLDR_NAND_BLOCK_SIZE) 
            
NUM_SEGMENT_NAND_USED	EQU     (NAND_PAGE_SIZE/NANDFC_MAIN_BUFF_SIZE)    
NFC_BBI_MAIN_SEGMENT			EQU     (NANDFC_MAIN_BUFF0_OFFSET + NAND_PAGE_SIZE-NANDFC_MAIN_BUFF_SIZE)
NFC_SPARE_SEGMENT_HIGH	EQU     (NANDFC_SPARE_BUFF0_OFFSET+(NUM_SEGMENT_NAND_USED-1)*NANDFC_SPARE_BUFF_SIZE)
NFC_SPARE_SEGMENT_LOW		EQU     (NFC_SPARE_SEGMENT_HIGH-NANDFC_SPARE_BUFF_SIZE)
NAND_BBI_COL_ADDR				EQU     (((NANDFC_MAIN_BUFF_SIZE-(NUM_SEGMENT_NAND_USED-1)*(NAND_SPARE_SIZE/(NUM_SEGMENT_NAND_USED*2)*2))/4)*4);NAND flash bbi column address

    END
