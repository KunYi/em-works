;-----------------------------------------------------------------------------
;  Copyright (c) Microsoft Corporation.  All rights reserved.
;
;  Use of this source code is subject to the terms of the Microsoft end-user
;  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
;  If you did not accept the terms of the EULA, you are not authorized to use
;  this source code. For a copy of the EULA, please see the LICENSE.RTF on
;  your install media.
;
;-----------------------------------------------------------------------------
;
;  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
;  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;-----------------------------------------------------------------------------
; 
;   FILE:   xldr.s
;   
;   Provides support for booting from a NAND device connected to the 
;   NAND flash controller.
;
;------------------------------------------------------------------------------
    INCLUDE armmacros.s
    INCLUDE mx25_base_regs.inc
    INCLUDE mx25_base_mem.inc
    INCLUDE mx25_esdramc.inc    
    INCLUDE mx25_nandfc.inc

    INCLUDE image_cfg.inc


NANDFC_MAIN_BUFF_SIZE             EQU     (512)
NANDFC_SPARE_BUFF_SIZE            EQU     (64)

NANDFC_NFC_ECC_MODE_8BIT            EQU     0
NANDFC_NFC_ECC_MODE_4BIT            EQU     1
    INCLUDE nandchip.inc
; Destination
DST_LOAD_ADDR                       EQU     0x78001B00
DST_BBI_MAIN_SECTION                EQU     (NANDFC_MAIN_BUFF0_OFFSET + (NANDFC_MAIN_BUFF_SIZE) * (NUM_SEGMENT_NAND_USED + 3))
DST_SPARE_SECTION_HIGH              EQU     (NANDFC_SPARE_BUFF0_OFFSET + (NANDFC_SPARE_BUFF_SIZE) * (NUM_SEGMENT_NAND_USED + 3))
DST_BBI_COL_ADDR                    EQU     (400)


    OPT 2                                       ; disable listing
    INCLUDE kxarm.h
    OPT 1                                       ; reenable listing
   
    TEXTAREA

; romimage needs pTOC. give it one.
pTOC    DCD -1
    EXPORT pTOC


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
    ; SWAP the BBI because nfc automatically read first page out without BBI swap.
    ; The swap must be done at very beginning of the XLDR to avoid code mismatch.
    ldr     r1, =(NAND_SPARE_SIZE)  
    cmp     r1, #218
    bne     STD_SPARE_SIZE_Code    
         
SPARE_SIZE_218B_Code   
    ; Get main data in spare buffer and store it to r0
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NFC_SPARE_SEGMENT_HIGH)
    ldrh    r0, [r1]    
    and     r0, r0, #(0xFF00)
    mov     r0, r0, lsl #8    

    ;restore main data   
    ldr     r1, =(DST_LOAD_ADDR+NFC_BBI_MAIN_SEGMENT+NAND_BBI_COL_ADDR)    
    ldr     r4, [r1]
    and     r4, r4, #(0xFF00FFFF)
    orr     r4, r4, r0
    str     r4, [r1]
    b       Exit_SwapBBI_Code
    
STD_SPARE_SIZE_Code    
    ; Get main data in spare buffer and store it to r0
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC + NFC_SPARE_SEGMENT_HIGH)
    ldrh    r0, [r1]    
    and     r0, r0, #(0xFF00)
    mov     r0, r0, lsr #8    

    ; Get BBI in main buffer
    ldr     r1, =(DST_LOAD_ADDR + NFC_BBI_MAIN_SEGMENT + NAND_BBI_COL_ADDR)
    ldr     r4, [r1]
    and     r4, r4, #(0xFFFFFF00)
    orr     r4, r4, r0
    str     r4, [r1]
    
    ;if the nand flash is 2K Page Size, then we should do second page bbi swapping.
    mov     r0, #(NAND_PAGE_SIZE)
    cmp     r0, #2048
    bne     Exit_SwapBBI_Code

    ; Get main data in spare buffer and store it to r0
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC + DST_SPARE_SECTION_HIGH)
    ldrh    r0, [r1]    
    and     r0, r0, #(0xFF00)
    mov     r0, r0, lsr #8    

    ;restore main data 
    ldr     r1, =(DST_LOAD_ADDR + DST_BBI_MAIN_SECTION + DST_BBI_COL_ADDR)
    ldr     r4, [r1]
    and     r4, r4, #(0xFFFFFF00)
    orr     r4, r4, r0
    str     r4, [r1]

Exit_SwapBBI_Code    
    
    ;; now we do the external RAM init. it's located in another file because it's shared among all the XLDRs
    INCLUDE xldr_sdram_init.inc
    
nfc_init
    ;
    ; Configure NFC Internal Buffer Lock Control
    ;
    ;   BLS - Unlocked (2 << 0)                         = 0x0002
    ;                                                   --------
    ;                                                     0x0002
    ;
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_NFC_CONFIGURATION_OFFSET)
    ldr     r0, =0x0002
    strh    r0, [r1]
    
    ; Select NANDFC RAM buffer address
    ;
    ;   RBA - 1st internal RAM buffer (0 << 0)          = 0x0000
    ;                                                   --------
    ;                                                     0x0000
    ;
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_RAM_BUFFER_ADDRESS_OFFSET)
    ldr     r0, =0x0000
    strh    r0, [r1]
    
    ; Configure NANDFC operation
    ;   
    ;   FP_INT - Interrupt generated after whole page   = 0x0800
    ;   PPB - 128 pages per block                       = 0x0400
    ;   NF_CE - normal CE signal (0 << 7)               = 0x0000
    ;   NF_RST - no reset (0 << 6)                      = 0x0000
    ;   NF_BIG - little endian (0 << 5)                 = 0x0000
    ;   INT_MSK - mask interrupt (1 << 4)               = 0x0010
    ;   ECC_EN - enable ECC (1 << 3)                    = 0x0008
    ;   SP_EN - main and spare read/write (0 << 2)      = 0x0000
    ;   DMA_MODE - after page read                      = 0x0002
    ;   ECC_MODE - 4bit ecc                             = 0x0001  
    ;                                                   --------
    ;                                                     0x0C1A
    ;
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_NAND_FLASH_CONFIG1_OFFSET)
    ldr     r0, =(0x091A)
    
    ldr     r3, =(NAND_SPARE_SIZE)
    cmp     r3, #218
    bne     ECC_configure

    ; if 218B spare size
    mov     r3, #(NANDFC_NFC_ECC_MODE_8BIT)
    b       ECC_set_cfg    
    
ECC_configure
    mov     r3, #(NANDFC_NFC_ECC_MODE_4BIT)
ECC_set_cfg    
    add     r0, r0, r3        
    
    ; Page number per block
    ldr     r3, =(NAND_PAGE_CNT)
    cmp     r3, #256
    bne     page_size_configure

    ; if 256 pages per block
    mov     r3, #(0x11)
    b       page_size_set_cfg

page_size_configure
    mov     r3, #(NAND_PAGE_CNT>>6)
page_size_set_cfg
    mov     r3, r3, lsl #9
    add     r0, r0, r3     
    strh    r0, [r1]

    ; Configure NANDFC unlock start block
    ;
    ;   USBA - block #0 (0 << 0)                        = 0x0000
    ;                                                   --------
    ;                                                     0x0000
    ;
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_UNLOCK_START_BLK_ADD0_OFFSET)
    ldr     r0, =0x0000
    strh    r0, [r1]

    ; Configure NANDFC unlock end block
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_UNLOCK_END_BLK_ADD0_OFFSET)
    ldr     r0, =(NAND_BLOCK_CNT-1)
    strh    r0, [r1]

    ; Configure NANDFC write protection status
    ;
    ;   WPC - unlock specified range (4 << 0)           = 0x0004
    ;                                                   --------
    ;                                                     0x0004
    ;
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_NF_WR_PROT_OFFSET)
    ldr     r0, =0x0004
    strh    r0, [r1]

    ; Configure the Spare area size
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_SPAS_OFFSET)
    ldr     r0, =(NAND_SPARE_SIZE >> 1)
    strh    r0, [r1]
    
    ;We should find the physical block address according to logical block address. 
    ldr     r13, =0x0
    ldr     r8, =0x0
    ldr     r9, =(IMAGE_EBOOT_NAND_BLOCK_OFFSET+1)
    
getgoodphyblock 
    mov     r12, r13, lsl #(NAND_PAGE_CNT_LSH)
    bl      GetBlockStatus
    cmp     r0,  #0xFF
    bne     nextphyblock
    
    add     r8,  r8, #0x01
    cmp     r8,  r9
    bne     nextphyblock
    b       preloadeboot  
    
nextphyblock    
    add     r13, r13, #0x01  
    b       getgoodphyblock

preloadeboot    
    ; Current external RAM load address is kept in R3
    ldr     r3,  =(IMAGE_BOOT_BOOTIMAGE_RAM_PA_START)

    ; Current page address is kept in R12
    mov     r12, r13, lsl #(NAND_PAGE_CNT_LSH)    

load_eboot
    ;skip bad block
    bl      GetBlockStatus
    cmp     r0, #0xFF
    bne     next_block
    mov     r12, r13, lsl #(NAND_PAGE_CNT_LSH)
    
    ; Read one physical page of data from the NAND device into NFC internal buffer
    bl      NfcRd1PhyPage
    
    ; Advance page pointer to next page
    add     r12, r12, #0x01 
    
    ; Copy one physical page of data in the NANDFC buffers to RAM
next_page
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_MAIN_BUFF0_OFFSET)
    mov     r2, #NAND_PAGE_SIZE

copy_eboot
    ldmia   r1!, {r4 - r11}
    stmia   r3!, {r4 - r11}
    subs    r2, r2, #32
    bne     copy_eboot

    ; Check if we are done
    ldr     r1, =(IMAGE_BOOT_BOOTIMAGE_RAM_PA_START)
    subs    r1, r3, r1
    cmp     r1, #IMAGE_BOOT_BOOTIMAGE_RAM_SIZE
    beq     jump2eboot

    ; Check if we have hit a block boundary
    ldr     r1, =(NAND_BLOCK_SIZE-1)
    ands    r1, r1, r3
    beq     next_block
    
    ; Read another page
    bl      NfcRd1PhyPage
    
    ; Advance page pointer to next page
    add     r12, r12, #0x01 
       
    b       next_page    

next_block
    ; Advance to next NAND block
    add     r13, r13, #1

    b       load_eboot
    
jump2eboot

    ; Jump to EBOOT image copied to DRAM
    ldr     r1, =(IMAGE_BOOT_BOOTIMAGE_RAM_PA_START)
    mov     pc, r1
    nop
    nop
    nop
    nop

forever
    b       forever

;-------------------------------------------------------------------------------
;
;   Function:  GetBlockStatus
;
;   This function sends an address (single cycle) to the NAND device.  
;
;   Parameters:
;       None.
;
;   Returns:
;           [out] - r0: Bad block flag: 0x00 indicates bad block, 0xFF indicate good block
;-------------------------------------------------------------------------------
    LEAF_ENTRY GetBlockStatus
    
    ; Save return address
    mov     r7, r14    

    mov     r5, r12     
            
CheckBadBlock   
    ; Check bad block indicator in first BBI page
    add     r12, r12, #BBI_PAGE_ADDR_1 
    
CheckBBIPage            
    bl      NfcRd1PhyPage
    
    ; NANDFC_ECC_STATUS_RESULT_OFFSET
    ldr     r2, =(NAND_PAGE_SIZE)
    cmp     r2, #2048
    bne     PageSize4K    
    ldr     r1,  =(CSP_BASE_REG_PA_NANDFC+NANDFC_ECC_STATUS_RESULT1_OFFSET)
    b       ECCCheck     
       
PageSize4K
    ldr     r1,  =(CSP_BASE_REG_PA_NANDFC+NANDFC_ECC_STATUS_RESULT2_OFFSET)
    
ECCCheck
    ldrh    r0, [r1]

    ; NOSER1, 2, 3, 4, 
    ; r0 = 0, NOSER1, 0x000F
    ;      1, NOSER2, 0x00F0
    ;      2, NOSER3, 0x0F00
    ;      3, NOSER4, 0xF000
    
    ldr     r1, =(0xFFFF) 
    and     r0, r0, r1
    cmp     r0, #0x0
    beq     NoECCError
    
    ldr     r1, =(0x4) 
    
ECCCheck2
    and     r2, r0, #0xF
    cmp     r2, #0xF
    beq     BadBlockFound
    
    mov     r0, r0, lsr #4    
    sub     r1, r1, #1
    cmp     r1, #0x0
    bne     ECCCheck2
    
    ;Has the page been written before?
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_MAIN_BUFF0_OFFSET)
    add     r2, r1, #NAND_PAGE_SIZE 
       
CheckMainData    
    ldr     r0, [r1]
    cmp     r0, #0xFFFFFFFF
    bne     Check2ndBBIPage
        
    add     r1, r1, #4
    cmp     r1, r2
    bne     CheckMainData
    
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NFC_SPARE_SEGMENT_HIGH)
    ldr     r0, [r1]
    cmp     r0, #0xFFFFFFFF
    bne     Check2ndBBIPage    
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NFC_SPARE_SEGMENT_LOW)
    ldr     r0, [r1]
    cmp     r0, #0xFFFFFFFF
    bne     Check2ndBBIPage 
     
WrittenPage
    ;Disable ECC
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_NAND_FLASH_CONFIG1_OFFSET)
    ldrh    r0, [r1]
    ands    r0, r0, #(~0x8)
    strh    r0, [r1] 
    
    bl      NfcRd1PhyPage
    
    ;Enable ECC   
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_NAND_FLASH_CONFIG1_OFFSET)
    ldrh    r2, [r1]
    orr     r2, r2, #(0x8)
    strh    r2, [r1]

NoECCError              
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NFC_SPARE_SEGMENT_HIGH)
    ldrh    r0, [r1]
    mov     r0, r0, lsr #8
    and     r0, r0, #0xFF
    cmp     r0, #0xFF
    bne     BadBlockFound

Check2ndBBIPage    
    ; Check one page or two for BBI
    ldr     r2, =(BBI_PAGE_NUM)
    cmp     r2, #2
    bne     GoodBlockFound    

    ; Check bad block indicator in second BBI page  
    mov     r12, r5        
    add     r12, r12, #BBI_PAGE_ADDR_2
    b       CheckBBIPage

BadBlockFound
    ldr     r0,  =0x00
    b       Quit
    
GoodBlockFound
    ldr     r0,  =0xFF
        
Quit    
    ; load return address
    mov     r14, r7 
    
    RETURN


;-------------------------------------------------------------------------------
;
;   Function:  NfcCmd
;
;   This function issues the specified command to the NAND device.
;
;   Parameters:
;       command (r0)
;           [in] - Command to issue to the NAND device.
;
;   Returns:
;       None.
;-------------------------------------------------------------------------------
    LEAF_ENTRY NfcCmd

    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_NAND_FLASH_CMD_OFFSET)
    strh    r0, [r1]
    
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_NAND_FLASH_CONFIG2_OFFSET)
    ldr     r0, =(NANDFC_CONFIG2_FCMD)
    strh    r0, [r1]

    RETURN


;-------------------------------------------------------------------------------
;
;   Function:  NfcAddr
;
;   This function sends an address (single cycle) to the NAND device.  
;
;   Parameters:
;       address (r0)
;           [in] - Address to issue to the NAND device.
;
;   Returns:
;       None.
;-------------------------------------------------------------------------------
    LEAF_ENTRY NfcAddr

    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_NAND_FLASH_ADDR_OFFSET)
    strh    r0, [r1]
    
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_NAND_FLASH_CONFIG2_OFFSET)
    ldr     r0, =(NANDFC_CONFIG2_FADD)
    strh    r0, [r1]

    RETURN


;-------------------------------------------------------------------------------
;
;   Function:  NfcRead
;
;   This function reads a page of data from the NAND device.  
;
;   Parameters:
;       NFC buffer (r0)
;           [in] - NFC buffer (0-3) into which the page is read
;
;   Returns:
;       None.
;-------------------------------------------------------------------------------
    LEAF_ENTRY NfcRead
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_RAM_BUFFER_ADDRESS_OFFSET)
    strh    r0, [r1]

    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_NAND_FLASH_CONFIG2_OFFSET)
    ldr     r0, =(NANDFC_CONFIG2_FDO_PAGE)
    strh    r0, [r1]

    RETURN


;-------------------------------------------------------------------------------
;
;   Function:  NfcWait
;
;   This function waits for the current NAND device operation to complete.
;
;   Parameters:
;       None.
;
;   Returns:
;       None.
;-------------------------------------------------------------------------------
    LEAF_ENTRY NfcWait

    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NANDFC_NAND_FLASH_CONFIG2_OFFSET)
wait_loop
    ldrh    r0, [r1]
    ands    r0, r0, #NANDFC_CONFIG2_INT
    beq     wait_loop

    ; Clear INT status
    bic     r0, r0, #NANDFC_CONFIG2_INT
    strh    r0, [r1]

    RETURN

;-------------------------------------------------------------------------------
;
;   Function:  SwapBBI
;
;   This function swap BBI between main buffer and spare buffer.
;
;   Parameters:
;       None
;
;   Returns:
;       None.
;-------------------------------------------------------------------------------
    LEAF_ENTRY SwapBBI


    ldr     r1, =(NAND_SPARE_SIZE)  
    CMP     r1, #218
    bne     STD_SPARE_SIZE    
         
SPARE_SIZE_218B
    ;Get main data in spare buffer and store it to r0
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NFC_SPARE_SEGMENT_HIGH)
    ldr     r0, [r1]    
    and     r0, r0, #(0xFF00)
    mov     r0, r0, lsl #8    

    ;Get BBI in main buffer
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NFC_BBI_MAIN_SEGMENT+NAND_BBI_COL_ADDR)
    ldr     r2, [r1]
    and     r2, r2, #(0xFF0000)
    mov     r2, r2, lsr #8
    
    ;swap data
    ;restore main data
    ldr     r4, [r1]
    and     r4, r4, #(0xFF00FFFF)
    orr     r4, r4, r0
    str     r4, [r1]

    b       Exit_SwapBBI
    
STD_SPARE_SIZE    
    ;Get main data in spare buffer and store it to r0
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NFC_SPARE_SEGMENT_HIGH)
    ldrh    r0, [r1]    
    and     r0, r0, #(0xFF00)
    mov     r0, r0, lsr #8    

    ;Get BBI in main buffer
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NFC_BBI_MAIN_SEGMENT+NAND_BBI_COL_ADDR)
    ldrh    r2, [r1]
    and     r2, r2, #(0xFF)
    mov     r2, r2, lsl #8
    
    ;swap data
    ;restore main data
    ldr     r4, [r1]
    and     r4, r4, #(0xFFFFFF00)
    orr     r4, r4, r0
    str     r4, [r1]


Exit_SwapBBI

    ;restore spare data
    ldr     r1, =(CSP_BASE_REG_PA_NANDFC+NFC_SPARE_SEGMENT_HIGH)
    ldr     r4, [r1]
    and     r4, r4, #(0xFFFF00FF)
    orr     r4, r4, r2
    str     r4, [r1]


    RETURN
    
;-------------------------------------------------------------------------------
;
;   Function:  NfcRd1PhyPage
;
;   This function reads one physical page from Nand Flash device.
;
;   Parameters:
;       address (r12)
;           [in/out] - page address for NAND device.
;
;   Returns:
;       None.
;-------------------------------------------------------------------------------
    LEAF_ENTRY NfcRd1PhyPage

    ; Save return address
    mov     r11, r14

    ; Send page read command
    ldr     r0, =(CMD_READ)
    bl      NfcCmd
    bl      NfcWait

    ; Send column address (cycle 1)
    mov     r0, #0
    and     r0, r0, #(0xFF)
    bl      NfcAddr
    bl      NfcWait

    ; Send column address (cycle 2)
    mov     r0, #0
    and     r0, r0, #(0xFF)
    bl      NfcAddr
    bl      NfcWait

    ; Send row address (cycle 3)
    mov     r0, r12
    and     r0, r0, #(0xFF)
    bl      NfcAddr
    bl      NfcWait

    ; Send row address (cycle 4)
    mov     r0, r12, lsr #8
    and     r0, r0, #(0xFF)
    bl      NfcAddr
    bl      NfcWait

    ; Send row address (cycle 5)
    mov     r0, r12, lsr #16
    and     r0, r0, #(0xFF)
    bl      NfcAddr
    bl      NfcWait

    ; Send the second cycle of page read command
    ldr     r0, =(CMD_READ2CYCLE)
    bl      NfcCmd
    bl      NfcWait

    ; Read the page, and start from buffer 0
    mov     r0, #0 
    bl      NfcRead
    bl      NfcWait

    bl      SwapBBI
 
    ; Restore return address
    mov     r14, r11

    RETURN

    END

